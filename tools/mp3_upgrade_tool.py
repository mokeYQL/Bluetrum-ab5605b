"""
AB5605B MP3 UART 升级工具 (POWERON_MP3)
=========================================
协议: [0x55][0xAA][CMD 1B][PAYLOAD N]
  CMD 0x01 = ERASE: 擦除5个扇区
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

# ---------------- 协议常量 ----------------
FRAME_HDR0 = 0x55
FRAME_HDR1 = 0xAA
CMD_ERASE  = 0x01
CMD_DATA   = 0x02
CMD_FINISH = 0x03

FLASH_ADDR   = 0x11002000
MAX_SIZE     = 20480       # 20KB
CHUNK_SIZE   = 256
TOTAL_CHUNKS = MAX_SIZE // CHUNK_SIZE  # 80

ACK_HDR0 = 0xAA
ACK_HDR1 = 0x55


class MP3UpgradeTool:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title("AB5605B MP3 Upgrade Tool")
        self.window.geometry("580x500")
        self.window.resizable(False, False)
        self.ser = None
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

        # ---- 文件选择 ----
        frm2 = ttk.LabelFrame(self.window, text="MP3 文件", padding=8)
        frm2.pack(fill=tk.X, padx=10, pady=5)

        self.lbl_file = ttk.Label(frm2, text="未选择文件", foreground="gray")
        self.lbl_file.pack(side=tk.LEFT, padx=5)
        ttk.Button(frm2, text="选择文件", command=self._select_file).pack(
            side=tk.RIGHT, padx=5)

        # ---- 进度 + 发送 ----
        frm3 = ttk.LabelFrame(self.window, text="升级进度", padding=8)
        frm3.pack(fill=tk.X, padx=10, pady=5)

        self.progress = ttk.Progressbar(frm3, mode="determinate", maximum=TOTAL_CHUNKS + 2)
        self.progress.pack(fill=tk.X, pady=(0, 5))

        self.lbl_status = ttk.Label(frm3, text="就绪")
        self.lbl_status.pack(side=tk.LEFT, padx=5)

        self.btn_send = ttk.Button(frm3, text="开始升级", command=self._start_upgrade,
                                   state=tk.DISABLED)
        self.btn_send.pack(side=tk.RIGHT, padx=5)

        # ---- 日志 ----
        frm4 = ttk.LabelFrame(self.window, text="日志", padding=5)
        frm4.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)

        self.log = scrolledtext.ScrolledText(frm4, height=10, state=tk.DISABLED, wrap=tk.WORD)
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
                self.btn_send.config(state=tk.NORMAL)
                self._log(f"串口 {port} 打开成功")
            except Exception as e:
                messagebox.showerror("错误", f"打开串口失败:\n{e}")

    # ---------------- 文件选择 ----------------
    def _select_file(self):
        path = filedialog.askopenfilename(
            title="选择 MP3 文件",
            filetypes=[("MP3 files", "*.mp3"), ("All files", "*.*")]
        )
        if path:
            size = os.path.getsize(path)
            if size > MAX_SIZE:
                messagebox.showwarning("提示",
                    f"文件大小 {size} 字节超过限制 {MAX_SIZE} 字节，将被截断")
            self.mp3_path = path
            self.lbl_file.config(text=os.path.basename(path), foreground="black")
            self._log(f"选择文件: {path}  ({size} bytes)")

    # ---------------- 升级流程 (后台线程) ----------------
    def _start_upgrade(self):
        if not hasattr(self, "mp3_path"):
            messagebox.showwarning("提示", "请先选择 MP3 文件")
            return
        if not self.ser or not self.ser.is_open:
            messagebox.showwarning("提示", "请先打开串口")
            return

        self.btn_send.config(state=tk.DISABLED)
        self.progress["value"] = 0
        threading.Thread(target=self._do_upgrade, daemon=True).start()

    def _do_upgrade(self):
        try:
            # 读文件, 补零到 20480
            with open(self.mp3_path, "rb") as f:
                data = bytearray(f.read(MAX_SIZE))
            if len(data) < MAX_SIZE:
                data += b"\x00" * (MAX_SIZE - len(data))
            chunks = [data[i:i + CHUNK_SIZE] for i in range(0, MAX_SIZE, CHUNK_SIZE)]
            self._update_status(f"文件加载完成: {len(chunks)} 个数据块")

            # 1. ERASE
            self._update_status("发送 ERASE 命令...")
            self.ser.write(bytes([FRAME_HDR0, FRAME_HDR1, CMD_ERASE]))
            if not self._wait_ack(CMD_ERASE, b"\x00", "ERASE"):
                return
            self._update_progress(1)
            time.sleep(0.1)

            # 2. DATA
            for seq, chunk in enumerate(chunks):
                frame = bytearray([FRAME_HDR0, FRAME_HDR1, CMD_DATA])
                frame += struct.pack("<H", seq)    # seq, 2B LE
                frame += chunk                     # 256B data
                self.ser.write(frame)

                if not self._wait_ack(CMD_DATA, struct.pack("<H", seq), f"DATA seq={seq}"):
                    return
                self._update_progress(seq + 2)

            # 3. FINISH
            self._update_status("发送 FINISH 命令...")
            self.ser.write(bytes([FRAME_HDR0, FRAME_HDR1, CMD_FINISH]))
            if not self._wait_ack(CMD_FINISH, b"\x00", "FINISH"):
                return
            self._update_progress(TOTAL_CHUNKS + 2)

            self._update_status("升级完成! 设备已重启.")
            messagebox.showinfo("完成", "POWERON_MP3 升级成功!")

        except Exception as e:
            self._log(f"错误: {e}")
            self._update_status(f"失败: {e}")
        finally:
            self.window.after(0, lambda: self.btn_send.config(state=tk.NORMAL))

    def _wait_ack(self, cmd, expected_tail, desc):
        """等待 [0xAA][0x55][cmd][expected_tail...]"""
        ack_len = 3 + len(expected_tail)  # AA 55 CMD + tail
        buf = bytearray()
        deadline = time.time() + 5.0
        while time.time() < deadline:
            b = self.ser.read(1)
            if b:
                buf += b
                if len(buf) >= ack_len:
                    # 找最近的 AA 55 CMD 序列
                    for i in range(len(buf) - ack_len + 1):
                        if (buf[i] == ACK_HDR0 and buf[i + 1] == ACK_HDR1
                                and buf[i + 2] == cmd
                                and buf[i + 3:i + ack_len] == expected_tail):
                            self._log(f"[OK] {desc}")
                            return True
                    # 不匹配, 继续读
                    if len(buf) > 128:
                        buf = buf[-64:]  # 防止缓冲区无限增大
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
    app = MP3UpgradeTool()
    app.window.mainloop()
