"""
AB5605B UART 综合工具 v4
====================================
集成了 OTA 语音升级 + 控制协议（查询/复位/静音/音量/音频切换/蓝牙状态/LED灯效/广播名称）。

协议 v3 (0xA5 帧格式):
  帧格式: [0xA5][命令分类][子命令][LEN_H][LEN_L][数据段][FCS]
  命令分类 0x13 = ESP32S3 → AB5605B (下发)
  命令分类 0x12 = AB5605B → ESP32S3 (上报)

  控制命令:
    0x01 查询设备信息   → JSON 应答
    0x02 系统复位       → 无应答(立即复位)
    0x03 静音控制       → 0x00取消/0x01静音
    0x04 音量控制       → 0x00~0x10
    0x05 音频切换       → 0x01:BT / 0x02:AUX
    0x06 蓝牙状态查询   → 0x00~0x03
    0x08 LED灯效        → 0x00关/0x01亮/0x02呼吸/0x03律动
    0x09 广播名称修改   → UTF-8 字符串

  OTA命令:
    0x0A 擦除(含CRC)    0x0B 写数据    0x0C 完成校验

运行: python mp3_upgrade_tool.py
依赖: pip install pyserial
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import serial
import serial.tools.list_ports
import threading
import struct
import time
import os
import json

# ---------------- 协议常量 (0xA5 帧格式) ----------------
FRAME_HDR  = 0xA5
CMD_ESP    = 0x13    # ESP32S3 → AB5605B
CMD_AB     = 0x12    # AB5605B → ESP32S3

# 控制子命令
CMD_QUERY_INFO  = 0x01
CMD_RESET       = 0x02
CMD_MUTE        = 0x03
CMD_VOLUME      = 0x04
CMD_AUDIO_SW    = 0x05
CMD_BT_STATUS   = 0x06
CMD_OTA_STATUS  = 0x07   # AB主动上报
CMD_LED         = 0x08
CMD_BT_NAME     = 0x09

# OTA子命令
CMD_ERASE       = 0x0A
CMD_DATA        = 0x0B
CMD_FINISH      = 0x0C

# ---------------- voc 分区配置 ----------------
FLASH_ADDR     = 0x60000
PART_SIZE      = 0x1B000       # 108 KB
CHUNK_SIZE     = 256
SECTOR_SIZE    = 4096
HEADER_SIZE    = 80            # 10 条 × 8 字节
DATA_OFFSET    = FLASH_ADDR + HEADER_SIZE

MAX_ENTRIES = 10

# 蓝牙状态文本映射
BT_STATUS_TEXT = {
    0x00: "已断开",
    0x01: "连接中",
    0x02: "已连接",
    0x03: "播放中",
}


# ---------------- CRC16-CCITT ----------------
def crc16_ccitt(crc, data):
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def xor_checksum(data):
    v = 0
    for b in data:
        v ^= b
    return v


def calc_fcs(cmd_class, sub_cmd, data):
    """计算 0xA5 帧的 FCS = 命令分类 ^ 子命令 ^ LEN_H ^ LEN_L ^ DATA..."""
    fcs = cmd_class ^ sub_cmd
    length = len(data)
    fcs ^= (length >> 8) & 0xFF
    fcs ^= length & 0xFF
    for b in data:
        fcs ^= b
    return fcs


def build_frame(cmd_class, sub_cmd, data=b""):
    """构建 0xA5 协议帧"""
    length = len(data)
    fcs = calc_fcs(cmd_class, sub_cmd, data)
    return bytes([FRAME_HDR, cmd_class, sub_cmd,
                  (length >> 8) & 0xFF, length & 0xFF]) + data + bytes([fcs])


class AB5605BTool:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title("AB5605B 综合工具 v4")
        self.window.geometry("820x960")
        self.window.resizable(True, True)
        self.ser = None
        self.voc_folder = None
        self.voc_bin = None
        self._build_ui()
        self._scan_ports()

    def _build_ui(self):
        # ---- 串口设置 ----
        frm = ttk.LabelFrame(self.window, text="串口设置", padding=8)
        frm.pack(fill=tk.X, padx=10, pady=(10, 5))

        ttk.Label(frm, text="COM口:").grid(row=0, column=0, sticky=tk.W)
        self.combo_port = ttk.Combobox(frm, width=12, state="readonly")
        self.combo_port.grid(row=0, column=1, padx=5)
        ttk.Button(frm, text="刷新", width=6,
                   command=self._scan_ports).grid(row=0, column=2, padx=2)

        ttk.Label(frm, text="波特率:").grid(row=0, column=3, sticky=tk.W, padx=(15, 0))
        self.combo_baud = ttk.Combobox(frm, width=10,
                                        values=["115200", "57600", "19200", "9600"],
                                        state="readonly")
        self.combo_baud.current(0)
        self.combo_baud.grid(row=0, column=4, padx=5)

        ttk.Button(frm, text="打开串口", command=self._toggle_serial).grid(
            row=0, column=5, padx=(10, 0))
        self.lbl_serial = ttk.Label(frm, text="未连接", foreground="red")
        self.lbl_serial.grid(row=0, column=6, padx=10)

        # ==================== 控制功能区 ====================
        frm_ctrl = ttk.LabelFrame(self.window, text="控制功能", padding=8)
        frm_ctrl.pack(fill=tk.X, padx=10, pady=5)

        # 第一行: 查询设备信息 + 复位 + 查询蓝牙状态
        ttk.Button(frm_ctrl, text="查询设备信息",
                   command=lambda: self._exec_ctrl("query_info")).grid(
            row=0, column=0, padx=3, pady=2)
        ttk.Button(frm_ctrl, text="系统复位",
                   command=lambda: self._exec_ctrl("reset")).grid(
            row=0, column=1, padx=3, pady=2)
        ttk.Button(frm_ctrl, text="查询蓝牙状态",
                   command=lambda: self._exec_ctrl("bt_status")).grid(
            row=0, column=2, padx=3, pady=2)

        # 第二行: 静音控制
        ttk.Label(frm_ctrl, text="静音:").grid(row=1, column=0, sticky=tk.E, padx=2, pady=2)
        self.combo_mute = ttk.Combobox(frm_ctrl, width=8, state="readonly",
                                        values=["取消静音", "静音"])
        self.combo_mute.current(0)
        self.combo_mute.grid(row=1, column=1, padx=3, pady=2, sticky=tk.W)
        ttk.Button(frm_ctrl, text="设置静音",
                   command=lambda: self._exec_ctrl("mute")).grid(
            row=1, column=2, padx=3, pady=2)

        # 第三行: 音量控制
        ttk.Label(frm_ctrl, text="音量:").grid(row=2, column=0, sticky=tk.E, padx=2, pady=2)
        self.scale_vol = ttk.Scale(frm_ctrl, from_=0, to=16, orient=tk.HORIZONTAL,
                                    length=120, command=self._on_vol_change)
        self.scale_vol.set(8)
        self.scale_vol.grid(row=2, column=1, padx=3, pady=2, sticky=tk.W)
        self.lbl_vol = ttk.Label(frm_ctrl, text="8", width=3)
        self.lbl_vol.grid(row=2, column=2, padx=2, pady=2, sticky=tk.W)
        ttk.Button(frm_ctrl, text="设置音量",
                   command=lambda: self._exec_ctrl("volume")).grid(
            row=2, column=3, padx=3, pady=2)

        # 第四行: 音频切换
        ttk.Label(frm_ctrl, text="音频源:").grid(row=3, column=0, sticky=tk.E, padx=2, pady=2)
        self.combo_audio = ttk.Combobox(frm_ctrl, width=8, state="readonly",
                                         values=["蓝牙BT", "AUX"])
        self.combo_audio.current(0)
        self.combo_audio.grid(row=3, column=1, padx=3, pady=2, sticky=tk.W)
        ttk.Button(frm_ctrl, text="切换音频",
                   command=lambda: self._exec_ctrl("audio_sw")).grid(
            row=3, column=2, padx=3, pady=2)

        # 第五行: LED灯效
        ttk.Label(frm_ctrl, text="LED:").grid(row=4, column=0, sticky=tk.E, padx=2, pady=2)
        self.combo_led = ttk.Combobox(frm_ctrl, width=8, state="readonly",
                                       values=["关闭", "常亮", "呼吸", "律动"])
        self.combo_led.current(0)
        self.combo_led.grid(row=4, column=1, padx=3, pady=2, sticky=tk.W)
        ttk.Button(frm_ctrl, text="设置灯效",
                   command=lambda: self._exec_ctrl("led")).grid(
            row=4, column=2, padx=3, pady=2)

        # 第六行: 广播名称
        ttk.Label(frm_ctrl, text="广播名:").grid(row=5, column=0, sticky=tk.E, padx=2, pady=2)
        self.entry_btname = ttk.Entry(frm_ctrl, width=20)
        self.entry_btname.insert(0, "BT-BOX")
        self.entry_btname.grid(row=5, column=1, padx=3, pady=2, sticky=tk.W)
        ttk.Button(frm_ctrl, text="设置名称",
                   command=lambda: self._exec_ctrl("bt_name")).grid(
            row=5, column=2, padx=3, pady=2)

        # ==================== OTA 升级区 ====================
        frm_ota_sel = ttk.LabelFrame(self.window, text="语音文件夹 (按文件名排序)", padding=8)
        frm_ota_sel.pack(fill=tk.X, padx=10, pady=5)

        self.lbl_folder = ttk.Label(frm_ota_sel, text="未选择文件夹", foreground="gray")
        self.lbl_folder.pack(side=tk.LEFT, padx=5)
        ttk.Button(frm_ota_sel, text="选择文件夹", command=self._select_folder).pack(
            side=tk.RIGHT, padx=5)
        ttk.Button(frm_ota_sel, text="重新扫描", command=self._rescan).pack(
            side=tk.RIGHT, padx=2)

        # ---- 文件列表 ----
        frm3 = ttk.LabelFrame(self.window, text="文件列表", padding=5)
        frm3.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.tree = ttk.Treeview(frm3, columns=("size_kb",), show="tree headings",
                                 height=4)
        self.tree.heading("#0", text="索引")
        self.tree.heading("size_kb", text="大小 (KB)")
        self.tree.column("#0", width=40, stretch=False)
        self.tree.column("size_kb", width=100, anchor=tk.E)
        self.tree.pack(fill=tk.BOTH, expand=True)

        # ---- OTA功能列表 + 输入值框 ----
        frm_func = ttk.LabelFrame(self.window, text="OTA功能列表 & 输入参数", padding=8)
        frm_func.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(frm_func, text="功能:").grid(row=0, column=0, sticky=tk.W)
        self.combo_func = ttk.Combobox(frm_func, width=20, state="readonly", values=[
            "一键自动升级 (完整流程)",
            "1. 擦除分区 (发送CRC)",
            "2. 手动发送单包数据",
            "3. 完成校验 (触发复位)",
        ])
        self.combo_func.current(0)
        self.combo_func.grid(row=0, column=1, padx=5, sticky=tk.W)

        ttk.Label(frm_func, text="CRC16:").grid(row=0, column=2, sticky=tk.W, padx=(15, 0))
        self.entry_crc = ttk.Entry(frm_func, width=8)
        self.entry_crc.grid(row=0, column=3, padx=5)
        self.entry_crc.insert(0, "auto")

        ttk.Label(frm_func, text="包序号:").grid(row=1, column=0, sticky=tk.W, pady=(5, 0))
        self.entry_seq = ttk.Entry(frm_func, width=8)
        self.entry_seq.grid(row=1, column=1, padx=5, pady=(5, 0), sticky=tk.W)
        self.entry_seq.insert(0, "0")

        ttk.Label(frm_func, text="重试:").grid(row=1, column=2, sticky=tk.W, pady=(5, 0))
        self.entry_retry = ttk.Entry(frm_func, width=4)
        self.entry_retry.grid(row=1, column=3, padx=5, pady=(5, 0), sticky=tk.W)
        self.entry_retry.insert(0, "3")

        ttk.Button(frm_func, text="执行", command=self._start_func).grid(
            row=0, column=4, padx=(15, 0), rowspan=2)

        # ---- 进度 + 发送 ----
        frm4 = ttk.LabelFrame(self.window, text="升级进度", padding=8)
        frm4.pack(fill=tk.X, padx=10, pady=5)

        self.progress = ttk.Progressbar(frm4, mode="determinate")
        self.progress.pack(fill=tk.X, pady=(0, 5))

        self.lbl_status = ttk.Label(frm4, text="就绪")
        self.lbl_status.pack(side=tk.LEFT, padx=5)

        self.btn_send = ttk.Button(frm4, text="开始升级", command=self._start_upgrade,
                                   state=tk.DISABLED)
        self.btn_send.pack(side=tk.RIGHT, padx=5)

        # ---- 日志 ----
        frm5 = ttk.LabelFrame(self.window, text="日志", padding=5)
        frm5.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.log = scrolledtext.ScrolledText(frm5, height=18, state=tk.DISABLED,
                                             wrap=tk.WORD)
        self.log.pack(fill=tk.BOTH, expand=True)

    def _on_vol_change(self, val):
        self.lbl_vol.config(text=str(int(float(val))))

    # ---------------- 串口 ----------------
    def _scan_ports(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.combo_port["values"] = ports
        if ports:
            self.combo_port.current(0)

    def _toggle_serial(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.ser = None
            self.lbl_serial.config(text="未连接", foreground="red")
            self.btn_send.config(state=tk.DISABLED)
            self._log("串口已关闭")
        else:
            port = self.combo_port.get()
            if not port:
                messagebox.showwarning("提示", "请先选择 COM 口")
                return
            try:
                self.ser = serial.Serial(port, int(self.combo_baud.get()), timeout=2)
                self.lbl_serial.config(text=f"{port} 已连接", foreground="green")
                if self.voc_folder:
                    self.btn_send.config(state=tk.NORMAL)
                self._log(f"串口 {port} 打开成功")
            except Exception as e:
                messagebox.showerror("错误", f"打开串口失败:\n{e}")

    # ---------------- 控制功能执行 ----------------
    def _exec_ctrl(self, func_name):
        if not self.ser or not self.ser.is_open:
            messagebox.showwarning("提示", "请先打开串口")
            return
        threading.Thread(target=self._exec_ctrl_thread, args=(func_name,),
                         daemon=True).start()

    def _exec_ctrl_thread(self, func_name):
        try:
            if func_name == "query_info":
                self._ctrl_query_info()
            elif func_name == "reset":
                self._ctrl_reset()
            elif func_name == "mute":
                self._ctrl_mute()
            elif func_name == "volume":
                self._ctrl_volume()
            elif func_name == "audio_sw":
                self._ctrl_audio_sw()
            elif func_name == "bt_status":
                self._ctrl_bt_status()
            elif func_name == "led":
                self._ctrl_led()
            elif func_name == "bt_name":
                self._ctrl_bt_name()
        except Exception as e:
            self._log(f"错误: {e}")
            self._update_status(f"失败: {e}")

    def _send_cmd(self, sub_cmd, data=b""):
        """发送控制命令并返回应答数据段(含FCS校验), 失败返回None"""
        frame = build_frame(CMD_ESP, sub_cmd, data)
        self.ser.write(frame)
        return self._wait_response(sub_cmd, 3.0)

    def _ctrl_query_info(self):
        self._update_status("查询设备信息...")
        resp = self._send_cmd(CMD_QUERY_INFO)
        if resp is None:
            return
        try:
            text = resp.decode("utf-8")
            info = json.loads(text)
            self._log("=== 设备信息 ===")
            self._log(f"  产品名称: {info.get('pn', '?')}")
            self._log(f"  厂家名称: {info.get('fn', '?')}")
            self._log(f"  硬件版本: {info.get('hv', '?')}")
            self._log(f"  软件版本: {info.get('sv', '?')}")
            self._update_status(f"设备: {info.get('pn','?')} v{info.get('sv','?')}")
        except Exception as e:
            self._log(f"解析失败: {e}, 原始: {resp.hex(' ')}")
            self._update_status("解析设备信息失败")

    def _ctrl_reset(self):
        if not messagebox.askyesno("确认", "确定要复位AB5605B吗？"):
            return
        self._update_status("发送复位命令...")
        frame = build_frame(CMD_ESP, CMD_RESET, bytes([0x01]))
        self.ser.write(frame)
        self._log("复位命令已发送, 设备将立即复位")
        self._update_status("复位命令已发送")

    def _ctrl_mute(self):
        mute = 0x01 if self.combo_mute.get() == "静音" else 0x00
        self._update_status(f"设置{'静音' if mute else '取消静音'}...")
        resp = self._send_cmd(CMD_MUTE, bytes([mute]))
        if resp is not None and resp == b"\x00":
            self._update_status(f"{'静音' if mute else '取消静音'} 成功")
        else:
            self._update_status("静音设置失败")

    def _ctrl_volume(self):
        vol = int(float(self.scale_vol.get()))
        self._update_status(f"设置音量 {vol}...")
        resp = self._send_cmd(CMD_VOLUME, bytes([vol]))
        if resp is not None and resp == b"\x00":
            self._update_status(f"音量已设为 {vol}")
        else:
            self._update_status("音量设置失败")

    def _ctrl_audio_sw(self):
        src = 0x01 if self.combo_audio.get() == "蓝牙BT" else 0x02
        name = "蓝牙BT" if src == 0x01 else "AUX"
        self._update_status(f"切换到 {name}...")
        resp = self._send_cmd(CMD_AUDIO_SW, bytes([src]))
        if resp is not None and resp == b"\x00":
            self._update_status(f"已切换到 {name}")
        else:
            self._update_status("音频切换失败")

    def _ctrl_bt_status(self):
        self._update_status("查询蓝牙状态...")
        resp = self._send_cmd(CMD_BT_STATUS)
        if resp is None:
            return
        if len(resp) >= 1:
            st = resp[0]
            text = BT_STATUS_TEXT.get(st, f"未知(0x{st:02X})")
            self._log(f"蓝牙状态: {text}")
            self._update_status(f"蓝牙: {text}")
        else:
            self._update_status("蓝牙状态应答异常")

    def _ctrl_led(self):
        led = self.combo_led.current()
        names = ["关闭", "常亮", "呼吸", "律动"]
        self._update_status(f"设置LED {names[led]}...")
        resp = self._send_cmd(CMD_LED, bytes([led]))
        if resp is not None and resp == b"\x00":
            self._update_status(f"LED已设为 {names[led]}")
        else:
            self._update_status("LED设置失败")

    def _ctrl_bt_name(self):
        name = self.entry_btname.get().strip()
        if not name:
            messagebox.showwarning("提示", "请输入广播名称")
            return
        name_bytes = name.encode("utf-8")
        if len(name_bytes) > 32:
            messagebox.showwarning("提示", "名称过长(最多32字节)")
            return
        self._update_status(f"设置广播名称: {name}...")
        resp = self._send_cmd(CMD_BT_NAME, name_bytes)
        if resp is not None and resp == b"\x00":
            self._update_status(f"广播名称已设为 {name}")
        else:
            self._update_status("广播名称设置失败")

    # ---------------- 通用应答接收 (0xA5格式) ----------------
    def _wait_response(self, sub_cmd, timeout=3.0):
        """等待指定子命令的应答帧, 返回数据段bytes(已校验FCS), 失败返回None"""
        buf = bytearray()
        deadline = time.time() + timeout
        while time.time() < deadline:
            b = self.ser.read(1)
            if b:
                buf += b
                idx = buf.find(bytes([FRAME_HDR, CMD_AB, sub_cmd]))
                while idx >= 0:
                    buf = buf[idx:]
                    if len(buf) < 5:
                        break
                    data_len = (buf[3] << 8) | buf[4]
                    total = 5 + data_len + 1
                    if len(buf) < total:
                        break
                    recv_data = bytes(buf[5:5 + data_len])
                    recv_fcs = buf[5 + data_len]
                    calc = calc_fcs(CMD_AB, sub_cmd, recv_data)
                    if recv_fcs == calc:
                        return recv_data
                    else:
                        self._log(f"[WARN] FCS不匹配 calc=0x{calc:02X} recv=0x{recv_fcs:02X}")
                        buf = buf[total:]
                        idx = buf.find(bytes([FRAME_HDR, CMD_AB, sub_cmd]))
                if len(buf) > 512:
                    buf = buf[-256:]
            else:
                time.sleep(0.01)
        self._log(f"[FAIL] 等待子命令0x{sub_cmd:02X}应答超时")
        return None

    # ---------------- OTA: 文件夹选择 ----------------
    def _select_folder(self):
        path = filedialog.askdirectory(title="选择包含 .mp3 文件的文件夹")
        if path:
            self.voc_folder = path
            self._rescan()

    def _rescan(self):
        if not self.voc_folder:
            return
        for item in self.tree.get_children():
            self.tree.delete(item)

        all_files = sorted(os.listdir(self.voc_folder))
        mp3_files = [f for f in all_files if f.lower().endswith(".mp3")][:MAX_ENTRIES]

        if not mp3_files:
            self.lbl_folder.config(text="文件夹内无 .mp3 文件", foreground="red")
            self._log("未找到 .mp3 文件")
            return

        total_data = 0
        for i, fname in enumerate(mp3_files):
            fpath = os.path.join(self.voc_folder, fname)
            size  = os.path.getsize(fpath)
            total_data += size
            self.tree.insert("", tk.END, text=f"{i:02d}",
                           values=(f"{size/1024:.2f}",), tags=(fname,))

        total_size = HEADER_SIZE + total_data
        pct = total_size * 100.0 / PART_SIZE
        folder_name = os.path.basename(self.voc_folder)

        if total_size > PART_SIZE:
            over = total_size - PART_SIZE
            self.lbl_folder.config(
                text=f"{folder_name} ({len(mp3_files)}条) → 超限! {total_size}/{PART_SIZE}B",
                foreground="red")
            self.btn_send.config(state=tk.DISABLED)
            self._log(f"错误: voc.bin 超出分区限制 {over} 字节")
        else:
            self.lbl_folder.config(
                text=f"{folder_name} ({len(mp3_files)}条) → {total_size}/{PART_SIZE}B ({pct:.1f}%)",
                foreground="black")
            if self.ser and self.ser.is_open:
                self.btn_send.config(state=tk.NORMAL)
            self._log(f"扫描完成: {len(mp3_files)} 条语音, {total_size} 字节 ({pct:.1f}%)")

    # ---------------- OTA: 构建voc数据 ----------------
    def _build_voc_bin(self):
        if not self.voc_folder:
            messagebox.showwarning("提示", "请先选择语音文件夹")
            return None

        all_files = sorted(os.listdir(self.voc_folder))
        mp3_files = [f for f in all_files if f.lower().endswith(".mp3")][:MAX_ENTRIES]
        if not mp3_files:
            self._update_status("没有 .mp3 文件, 中止")
            return None

        header    = bytearray(HEADER_SIZE)
        data_blob = bytearray()

        self._log(f"正在构建 voc 数据 ({len(mp3_files)} 条语音)...")
        for i, fname in enumerate(mp3_files):
            fpath = os.path.join(self.voc_folder, fname)
            with open(fpath, "rb") as f:
                mp3_data = f.read()
            mp3_size = len(mp3_data)
            flash_addr = DATA_OFFSET + len(data_blob)
            struct.pack_into("<I", header, i * 8,     flash_addr)
            struct.pack_into("<I", header, i * 8 + 4, mp3_size)
            data_blob.extend(mp3_data)
            self._log(f"  [{i:02d}] {fname:30s} addr=0x{flash_addr:06X}  size={mp3_size}")

        voc_bin = bytes(header + data_blob)
        total_size = len(voc_bin)

        if total_size > PART_SIZE:
            self._update_status(f"大小超限 {total_size - PART_SIZE} 字节!")
            return None

        if total_size % CHUNK_SIZE != 0:
            pad_len = CHUNK_SIZE - (total_size % CHUNK_SIZE)
            voc_bin += b"\x00" * pad_len
            self._log(f"补零 {pad_len} 字节对齐, 最终 {len(voc_bin)} 字节")

        self.voc_bin = voc_bin
        return voc_bin

    # ---------------- OTA: 功能执行入口 ----------------
    def _start_func(self):
        if not self.ser or not self.ser.is_open:
            messagebox.showwarning("提示", "请先打开串口")
            return
        func = self.combo_func.get()
        threading.Thread(target=self._exec_func, args=(func,), daemon=True).start()

    def _exec_func(self, func):
        try:
            if "一键" in func:
                self._do_upgrade()
            elif "擦除" in func:
                self._do_erase_only()
            elif "手动发送" in func:
                self._do_send_single_chunk()
            elif "完成校验" in func:
                self._do_finish_only()
        except Exception as e:
            self._log(f"错误: {e}")
            self._update_status(f"失败: {e}")

    def _do_erase_only(self):
        voc_bin = self.voc_bin or self._build_voc_bin()
        if not voc_bin:
            return
        crc_str = self.entry_crc.get().strip()
        if crc_str.lower() == "auto" or not crc_str:
            pc_crc = crc16_ccitt(0, voc_bin)
        else:
            pc_crc = int(crc_str, 16)
        crc_lo = pc_crc & 0xFF
        crc_hi = (pc_crc >> 8) & 0xFF
        self._log(f"擦除: CRC16=0x{pc_crc:04X}")
        self._update_status("发送 ERASE...")
        frame = build_frame(CMD_ESP, CMD_ERASE, bytes([crc_lo, crc_hi]))
        self.ser.write(frame)
        resp = self._wait_response(CMD_ERASE)
        if resp is not None and resp == b"\x00":
            self._update_status("擦除完成")
        time.sleep(0.5)

    def _do_send_single_chunk(self):
        voc_bin = self.voc_bin or self._build_voc_bin()
        if not voc_bin:
            return
        seq = int(self.entry_seq.get().strip() or "0")
        offset = seq * CHUNK_SIZE
        if offset + CHUNK_SIZE > len(voc_bin):
            self._update_status(f"seq={seq} 超出范围")
            return
        chunk = voc_bin[offset:offset + CHUNK_SIZE]
        xor_val = xor_checksum(chunk)
        payload = struct.pack("<H", seq) + bytes([xor_val]) + chunk
        frame = build_frame(CMD_ESP, CMD_DATA, payload)
        seq_bytes = struct.pack("<H", seq)
        self._update_status(f"发送 DATA seq={seq}...")
        self.ser.write(frame)
        resp = self._wait_response(CMD_DATA)
        if resp is not None and resp == seq_bytes:
            self._update_status(f"seq={seq} 发送成功")

    def _do_finish_only(self):
        self._update_status("发送 FINISH...")
        frame = build_frame(CMD_ESP, CMD_FINISH)
        self.ser.write(frame)
        resp = self._wait_response(CMD_FINISH, 10.0)
        if resp is None:
            self._update_status("FINISH 超时!")
            return
        result = resp[0] if resp else 0xFF
        if result == 0x00:
            self._update_status("校验通过, 设备将自动复位")
        else:
            self._update_status(f"校验失败 result=0x{result:02X}")

    # ---------------- OTA: 完整升级流程 ----------------
    def _start_upgrade(self):
        if not self.voc_folder:
            messagebox.showwarning("提示", "请先选择语音文件夹")
            return
        if not self.ser or not self.ser.is_open:
            messagebox.showwarning("提示", "请先打开串口")
            return
        self.btn_send.config(state=tk.DISABLED)
        self.progress["value"] = 0
        threading.Thread(target=self._do_upgrade, daemon=True).start()

    def _do_upgrade(self):
        try:
            voc_bin = self._build_voc_bin()
            if not voc_bin:
                return
            total_chunks = len(voc_bin) // CHUNK_SIZE
            self.progress["maximum"] = total_chunks + 2
            self._log(f"总计 {len(voc_bin)} 字节 ({len(voc_bin)/1024:.1f} KB), "
                     f"{total_chunks} 个数据块")

            # 1. CRC
            crc_str = self.entry_crc.get().strip()
            if crc_str.lower() == "auto" or not crc_str:
                pc_crc = crc16_ccitt(0, voc_bin)
            else:
                pc_crc = int(crc_str, 16)
            crc_lo = pc_crc & 0xFF
            crc_hi = (pc_crc >> 8) & 0xFF
            self._log(f"整包 CRC16-CCITT = 0x{pc_crc:04X}")

            # 2. ERASE
            self._update_status("发送 ERASE (含CRC)...")
            frame = build_frame(CMD_ESP, CMD_ERASE, bytes([crc_lo, crc_hi]))
            self.ser.write(frame)
            resp = self._wait_response(CMD_ERASE)
            if resp is None or resp != b"\x00":
                self._update_status("ERASE 失败")
                return
            self._update_progress(1)
            time.sleep(0.5)

            # 3. DATA
            max_retry = int(self.entry_retry.get().strip() or "3")
            for seq in range(total_chunks):
                offset = seq * CHUNK_SIZE
                chunk  = voc_bin[offset:offset + CHUNK_SIZE]
                xor_val = xor_checksum(chunk)
                payload = struct.pack("<H", seq) + bytes([xor_val]) + chunk
                frame = build_frame(CMD_ESP, CMD_DATA, payload)
                seq_bytes = struct.pack("<H", seq)

                for retry in range(max_retry + 1):
                    self.ser.write(frame)
                    resp = self._wait_response(CMD_DATA)
                    if resp is not None and resp == seq_bytes:
                        break
                    if retry < max_retry:
                        time.sleep(0.020 * (retry + 1))
                        self._log(f"[RETRY] seq={seq} 第{retry+1}次重试")
                    else:
                        self._update_status(f"DATA seq={seq} 超时!")
                        return

                self._update_progress(seq + 2)
                time.sleep(0.010)
                if seq % 50 == 0:
                    self._update_status(f"发送中... {seq+1}/{total_chunks}")

            # 4. FINISH
            self._update_status("发送 FINISH...")
            frame = build_frame(CMD_ESP, CMD_FINISH)
            self.ser.write(frame)
            resp = self._wait_response(CMD_FINISH, 10.0)
            if resp is None:
                self._update_status("FINISH 超时!")
                return
            self._update_progress(total_chunks + 2)
            result = resp[0] if resp else 0xFF

            if result == 0x00:
                self._update_status("升级成功! 设备将自动复位")
                self.window.after(0, lambda: messagebox.showinfo(
                    "完成", f"升级成功!\n\n{total_chunks} 个数据块\nCRC16 校验通过"))
            else:
                self._update_status("CRC校验失败!")
                self.window.after(0, lambda: messagebox.showerror(
                    "失败", f"CRC16 校验失败! result=0x{result:02X}"))

        except Exception as e:
            self._log(f"错误: {e}")
            self._update_status(f"失败: {e}")
        finally:
            self.window.after(0, lambda: self.btn_send.config(
                state=tk.NORMAL if (self.ser and self.ser.is_open and self.voc_folder) else tk.DISABLED))

    # ---------------- UI 辅助 ----------------
    def _log(self, msg):
        self.window.after(0, lambda: self._append_log(msg))

    def _append_log(self, msg):
        self.log.config(state=tk.NORMAL)
        self.log.insert(tk.END, f"[{time.strftime('%H:%M:%S')}] {msg}\n")
        self.log.see(tk.END)
        self.log.config(state=tk.DISABLED)

    def _update_status(self, msg):
        self.window.after(0, lambda: self.lbl_status.config(text=msg))
        self._log(msg)

    def _update_progress(self, val):
        self.window.after(0, lambda: self.progress.configure(value=val))


if __name__ == "__main__":
    app = AB5605BTool()
    app.window.mainloop()
