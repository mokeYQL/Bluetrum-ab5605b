"""
AB5605B 多语音批量 UART 升级工具
===============================
选择包含 .mp3 文件的文件夹, 自动构建 TOC 头部并升级到设备。
支持最多 10 条语音, 总分区分区 108KB。

协议: [0x55][0xAA][CMD 1B][PAYLOAD N]
  CMD 0x01 = ERASE: 擦除 voc 分区
  CMD 0x02 = DATA:  [seq 2B LE][256B data]
  CMD 0x03 = FINISH

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
import sys

# ---------------- 协议常量 ----------------
FRAME_HDR0 = 0x55
FRAME_HDR1 = 0xAA
CMD_ERASE  = 0x01
CMD_DATA   = 0x02
CMD_FINISH = 0x03

# ---------------- voc 分区配置 (与 firmware / app.xm 保持一致) ----------------
FLASH_ADDR     = 0x60000       # NOR Flash 偏移
PART_SIZE      = 0x1B000       # 108 KB (110592 bytes)
CHUNK_SIZE     = 256        # 每包数据量
SECTOR_SIZE    = 4096
HEADER_SIZE    = 80            # 10 条 × 8 字节
DATA_OFFSET    = FLASH_ADDR + HEADER_SIZE   # 0x60050

ACK_HDR0 = 0xAA
ACK_HDR1 = 0x55

MAX_ENTRIES = 10


class VocUpgradeTool:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title("AB5605B VocMulti Voice Upgrade Tool")
        self.window.geometry("700x800")
        self.window.resizable(True, True)
        self.ser = None
        self.voc_folder = None
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
        self.combo_baud = ttk.Combobox(frm, width=10, values=["115200"], state="readonly")
        self.combo_baud.current(0)
        self.combo_baud.grid(row=0, column=4, padx=5)

        ttk.Button(frm, text="打开串口", command=self._toggle_serial).grid(
            row=0, column=5, padx=(10, 0))
        self.lbl_serial = ttk.Label(frm, text="未连接", foreground="red")
        self.lbl_serial.grid(row=0, column=6, padx=10)

        # ---- 文件夹选择 ----
        frm2 = ttk.LabelFrame(self.window, text="语音文件夹 (按文件名排序)", padding=8)
        frm2.pack(fill=tk.X, padx=10, pady=5)

        self.lbl_folder = ttk.Label(frm2, text="未选择文件夹", foreground="gray")
        self.lbl_folder.pack(side=tk.LEFT, padx=5)
        ttk.Button(frm2, text="选择文件夹", command=self._select_folder).pack(
            side=tk.RIGHT, padx=5)
        ttk.Button(frm2, text="重新扫描", command=self._rescan).pack(
            side=tk.RIGHT, padx=2)

        # ---- 文件列表 ----
        frm3 = ttk.LabelFrame(self.window, text="文件列表", padding=5)
        frm3.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.tree = ttk.Treeview(frm3, columns=("size_kb",), show="headings",
                                 height=5)
        self.tree.heading("#0", text="索引")
        self.tree.heading("size_kb", text="大小 (KB)")
        self.tree.column("#0", width=40, stretch=False)
        self.tree.column("size_kb", width=100, anchor=tk.E)
        self.tree.pack(fill=tk.BOTH, expand=True)

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

        self.log = scrolledtext.ScrolledText(frm5, height=30, state=tk.DISABLED,
                                             wrap=tk.WORD)
        self.log.pack(fill=tk.BOTH, expand=True)

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
                self.ser = serial.Serial(port, 115200, timeout=2)
                self.lbl_serial.config(text=f"{port} 已连接", foreground="green")
                if self.voc_folder:
                    self.btn_send.config(state=tk.NORMAL)
                self._log(f"串口 {port} 打开成功")
            except Exception as e:
                messagebox.showerror("错误", f"打开串口失败:\n{e}")

    # ---------------- 文件夹选择 ----------------
    def _select_folder(self):
        path = filedialog.askdirectory(title="选择包含 .mp3 文件的文件夹")
        if path:
            self.voc_folder = path
            self._rescan()

    def _rescan(self):
        if not self.voc_folder:
            return
        # 清空列表
        for item in self.tree.get_children():
            self.tree.delete(item)

        # 扫描并排序
        all_files = sorted(os.listdir(self.voc_folder))
        mp3_files = [f for f in all_files if f.lower().endswith(".mp3")][:MAX_ENTRIES]

        if not mp3_files:
            self.lbl_folder.config(text="文件夹内无 .mp3 文件", foreground="red")
            self._log("未找到 .mp3 文件")
            return

        # 计算大小
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
                text=f"{folder_name} ({len(mp3_files)}条) → 超限! {total_size}/{PART_SIZE}B ({over}B)",
                foreground="red")
            self.btn_send.config(state=tk.DISABLED)
            self._log(f"错误: voc.bin 超出分区限制 {over} 字节 (总计 {total_size}B / {PART_SIZE}B)")
            messagebox.showwarning("大小超限",
                f"语音总大小 {total_size} 字节超出 108KB 限制 {over} 字节!\n\n请减少文件或调整分区大小。")
        else:
            self.lbl_folder.config(
                text=f"{folder_name} ({len(mp3_files)}条) → {total_size}/{PART_SIZE}B ({pct:.1f}%), 剩余 {PART_SIZE-total_size}B",
                foreground="black")
            if self.ser and self.ser.is_open:
                self.btn_send.config(state=tk.NORMAL)
            self._log(f"扫描完成: {len(mp3_files)} 条语音, 总计 {total_size} 字节 ({pct:.1f}%)")

    # ---------------- 升级流程 (后台线程) ----------------
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
            # ---- 1. 扫描文件夹, 构建 voc.bin 同上 build_voc.py 逻辑 ----
            all_files = sorted(os.listdir(self.voc_folder))
            mp3_files = [f for f in all_files if f.lower().endswith(".mp3")][:MAX_ENTRIES]

            if not mp3_files:
                self._update_status("没有 .mp3 文件, 中止")
                return

            header    = bytearray(HEADER_SIZE)  # 全 0
            data_blob = bytearray()

            self._log(f"正在构建 voc 数据 ({len(mp3_files)} 条语音)...")
            for i, fname in enumerate(mp3_files):
                fpath = os.path.join(self.voc_folder, fname)
                with open(fpath, "rb") as f:
                    mp3_data = f.read()
                mp3_size = len(mp3_data)
                flash_addr = DATA_OFFSET + len(data_blob)
                struct.pack_into("<I", header, i * 8,       flash_addr)
                struct.pack_into("<I", header, i * 8 + 4,   mp3_size)
                data_blob.extend(mp3_data)
                self._log(f"  [{i:02d}] {fname:30s} addr=0x{flash_addr:06X}  size={mp3_size}")

            # 组装 voc.bin = header + data
            voc_bin = header + data_blob
            total_size = len(voc_bin)

            if total_size > PART_SIZE:
                over = total_size - PART_SIZE
                self._update_status(f"大小超限 {over} 字节!")
                self._log(f"错误: voc 数据 {total_size}B 超出分区 {PART_SIZE}B")
                return

            # 补零对齐到 256 字节边界
            if total_size % CHUNK_SIZE != 0:
                pad_len = CHUNK_SIZE - (total_size % CHUNK_SIZE)
                voc_bin += b"\x00" * pad_len
                self._log(f"补零 {pad_len} 字节对齐, 最终 {len(voc_bin)} 字节")

            total_chunks = len(voc_bin) // CHUNK_SIZE
            self.progress["maximum"] = total_chunks + 2  # ERASE + DATA 块数 + FINISH
            self._log(f"总计 {total_size} 字节 ({total_size/1024:.1f} KB), "
                     f"{(total_size*100.0/PART_SIZE):.1f}% 占用, {total_chunks} 个数据块")

            # ---- 2. ERASE ----
            self._update_status("发送 ERASE 命令...")
            self.ser.write(bytes([FRAME_HDR0, FRAME_HDR1, CMD_ERASE]))
            if not self._wait_ack(CMD_ERASE, b"\x00", "ERASE"):
                return
            self._update_progress(1)
            time.sleep(0.5)  # 等待擦除完成

            # ---- 3. DATA (带重试) ----
            MAX_RETRY = 3
            for seq in range(total_chunks):
                offset = seq * CHUNK_SIZE
                chunk  = voc_bin[offset:offset + CHUNK_SIZE]

                frame = bytearray([FRAME_HDR0, FRAME_HDR1, CMD_DATA])
                frame += struct.pack("<H", seq)
                frame += chunk

                seq_bytes = struct.pack("<H", seq)

                # 重试机制: 每帧最多重试 MAX_RETRY 次
                for retry in range(MAX_RETRY + 1):
                    self.ser.write(frame)
                    if self._wait_ack(CMD_DATA, seq_bytes, f"DATA seq={seq}"):
                        break  # 成功, 下一帧
                    if retry < MAX_RETRY:
                        time.sleep(0.020 * (retry + 1))  # 重试等待: 20ms, 40ms, 60ms
                        self._log(f"[RETRY] seq={seq} 第{retry+1}次重试")
                    else:
                        # 全部重试失败
                        self._update_status(f"DATA seq={seq} 超时! ({MAX_RETRY+1}次均失败)")
                        return

                self._update_progress(seq + 2)
                time.sleep(0.010)  # 10ms 间隙

                if seq % 50 == 0:
                    self._update_status(f"发送中... {seq+1}/{total_chunks}")

            # ---- 4. FINISH ----
            self._update_status("发送 FINISH 命令...")
            self.ser.write(bytes([FRAME_HDR0, FRAME_HDR1, CMD_FINISH]))
            if not self._wait_ack(CMD_FINISH, b"\x00", "FINISH"):
                return
            self._update_progress(total_chunks + 2)

            self._update_status("升级完成! 共 {} 条语音.".format(len(mp3_files)))
            self.window.after(0, lambda: messagebox.showinfo(
                "完成", f"升级成功!\n\n{len(mp3_files)} 条语音, {total_size/1024:.1f} KB"))

        except Exception as e:
            self._log(f"错误: {e}")
            self._update_status(f"失败: {e}")
        finally:
            self.window.after(0, lambda: self.btn_send.config(
                state=tk.NORMAL if (self.ser and self.ser.is_open and self.voc_folder) else tk.DISABLED))

    def _wait_ack(self, cmd, expected_tail, desc):
        """等待 [0xAA][0x55][cmd][expected_tail...]"""
        ack_len = 3 + len(expected_tail)
        buf = bytearray()
        deadline = time.time() + 3.0    # 单次等待 3 秒 (有重试机制)
        while time.time() < deadline:
            b = self.ser.read(1)
            if b:
                buf += b
                if len(buf) >= ack_len:
                    for i in range(len(buf) - ack_len + 1):
                        if (buf[i] == ACK_HDR0 and buf[i + 1] == ACK_HDR1
                                and buf[i + 2] == cmd
                                and buf[i + 3:i + ack_len] == expected_tail):
                            self._log(f"[OK] {desc}")
                            return True
                    if len(buf) > 256:
                        buf = buf[-128:]
            else:
                time.sleep(0.01)

        self._log(f"[FAIL] {desc} 超时, 收到: {buf.hex(' ')}")
        self._update_status(f"{desc} 超时!")
        return False

    # ---------------- UI 辅助 (线程安全) ----------------
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
    app = VocUpgradeTool()
    app.window.mainloop()
