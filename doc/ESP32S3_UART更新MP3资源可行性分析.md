# ESP32S3 通过 UART 更新 AB5605B Flash MP3 资源 — 可行性分析

> **场景**: ESP32S3 作为上位机，通过 UART 串口更新 AB5605B 内置 NOR Flash 中的 MP3 语音提示文件
> **目标**: 无需重新烧录 DCF 固件，在线替换 MP3 资源

---

## 一、Flash 布局与资源寻址

### 1.1 NOR Flash 物理布局

```
NOR Flash 地址空间
┌──────────────────────────┐ 0x10000000
│                          │
│     app.bin              │  固件代码区 (~492KB max)
│   (.text/.rodata/.data)  │
│                          │
├──────────────────────────┤ 0x1007B000 (实际结束地址)
│     CM 参数区            │  20KB (配对信息/音量等)
├══════════════════════════┤ FLASH_SIZE (512KB)
│        (死区)            │
├══════════════════════════┤ 0x11000000
│                          │
│   资源索引表             │  每个条目 32 Bytes
│   ─ 文件名 (16B)        │
│   ─ BUF 地址 (4B)       │  ← 指向 MP3 数据
│   ─ LEN 长度 (4B)       │  ← 实际数据大小
│                          │
├──────────────────────────┤ 0x110008A0 (当前编译)
│                          │
│   MP3 数据区             │
│   max_vol.wav → 0x110008A0 │
│   poweron.mp3 → 0x11001860 │
│   ring.mp3    → 0x11005F80 │
│   ...                    │
│   39个英文提示音         │
│   EQ/DRC 配置            │
│                          │
└──────────────────────────┘
```

### 1.2 资源表条目结构（每条 32 字节）

```
Offset  Size  内容
0x00    16B   文件名 (ASCII, '\0' 填充)
0x10    8B    保留 (全 0)
0x18    4B    RES_BUF — 资源数据的 Flash 地址 (LE u32)
0x1C    4B    RES_LEN — 资源数据的实际长度 (LE u32)
```

对应 `res.h` 中的自动生成宏：

```c
// 条目 1: poweron.mp3
// 条目基址: 0x11000040
// BUF 在 0x11000058, LEN 在 0x1100005C
#define RES_BUF_POWERON_MP3    (*(u32 *)0x11000058)  // → 0x11001860
#define RES_LEN_POWERON_MP3    (*(u32 *)0x1100005c)  // → 0x4720 (18.2KB)
```

### 1.3 res.bin 原始数据验证

```hex
# 资源表条目 0: max_vol.wav
Offset 0x0020: 6D 61 78 5F 76 6F 6C 2E 77 61 76 00 00 00 00 00  "max_vol.wav....."
Offset 0x0030: 00 00 00 00 00 00 00 00 A0 08 00 11 C0 0F 00 00  "................"
              BUF = 0x110008A0 (LE: A0 08 00 11)
              LEN = 0x00000FC0 (LE: C0 0F 00 00) = 4032 bytes

# 资源表条目 1: poweron.mp3
Offset 0x0040: 70 6F 77 65 72 6F 6E 2E 6D 70 33 00 00 00 00 00  "poweron.mp3....."
Offset 0x0050: 00 00 00 00 00 00 00 00 60 18 00 11 20 47 00 00  "........`.. G.."
              BUF = 0x11001860 (LE: 60 18 00 11)
              LEN = 0x00004720 (LE: 20 47 00 00) = 18208 bytes
```

---

## 二、核心方案：固定槽位 + 定点写入

### 2.1 方案原理

将每个 MP3 文件在编译时**统一填充到相同大小**（如 20KB），使 xmaker 打包后每个资源的 Flash 地址固定不变。ESP32S3 只需硬编码目标地址，发送新 MP3 数据即可。

```
每个 MP3 = 20KB (0x5000)

条目0: max_vol.wav  → BUF=0x110008A0  LEN=0x5000
条目1: poweron.mp3  → BUF=0x11000DA0  LEN=0x5000  ← 地址固定！
条目2: ring.mp3     → BUF=0x110012A0  LEN=0x5000  ← 地址固定！
条目3: update.mp3   → BUF=0x110017A0  LEN=0x5000  ← 地址固定！
...
```

### 2.2 方案优势

| 对比维度 | 动态协议方案 | 固定槽位方案 |
|----------|-------------|-------------|
| ESP32S3 需解析资源表 | ✅ 需要 | ❌ 不需要，硬编码地址 |
| AB5605B 需更新 BUF | ✅ 可能移位 | ❌ 不变 |
| AB5605B 需更新 LEN | ✅ 需要 | ✅ 需要 |
| NOR 扇区擦除范围 | 按实际大小计算 | 固定 5 个扇区 (20KB) |
| 新文件 ≤ 原文件 | 需处理碎片 | 直接覆盖，多余空间忽略 |
| 协议复杂度 | 高（需地址协商） | 极低（选文件 → 发数据） |
| ESP32S3 实现量 | ~500 行 | ~200 行 |
| AB5605B 实现量 | ~300 行 | ~80 行 |

### 2.3 限制条件

1. **新 MP3 必须 ≤ 槽位大小**（如 20KB），超出会覆盖下一个资源
2. **不能增删资源文件**，否则 xmaker 重新打包后地址会整体偏移
3. **文件顺序由 xmaker 按字典序扫描确定**，只要文件名不变，顺序不变

---

## 三、SDK 提供的 Flash 操作 API

### 3.1 内部 NOR Flash API（直接可用）

> 声明于 `func_exspiflash_music.c`，实现在预编译库 `libplatform.a`

```c
// 读取内部 NOR Flash
uint os_spiflash_read(void *buf, u32 addr, uint len);

// 写入内部 NOR Flash（Page Program，单次最多 256 字节）
void os_spiflash_program(void *buf, u32 addr, uint len);

// 擦除 4KB Sector（配置项 FLASH_ERASE_4K = 1）
void os_spiflash_erase(u32 addr);
```

> 这些函数已在录音功能 (`INTERNAL_FLASH_REC`) 中实际使用，经过验证可以直接操作内部 NOR Flash 任意地址。

### 3.2 CM 参数区 API（不适用于资源区）

```c
// api_cm.h — 绑定在参数区（FLASH_SIZE - 0x5000）
void cm_write(void *buf, u32 addr, uint len);  // 写缓存，需 cm_sync()
void cm_read(void *buf, u32 addr, uint len);
void cm_clear(u32 addr);                       // 清除一个 Page (250B)
```

> ⚠️ `cm_write` 绑定于 CM 参数区（0x10078000 附近），不一定能访问 0x11000000 资源区，不推荐使用。

### 3.3 外挂 SPI Flash API（不适用）

```c
// bsp_spiflash1.h — 第二颗外挂 SPI Flash
int  spiflash1_read(void *buf, u32 addr, u32 len);
void spiflash1_write(void *buf, u32 addr, u32 len);
void spiflash1_erase(u32 addr);
void spiflash1_erase_block(u32 addr);
```

---

## 四、Flash 写入底层约束

### 4.1 NOR Flash 写入特性

```
操作      粒度        耗时(典型)   说明
读取      1 Byte      ~100ns      可随机字节读取
写入(Program)  ≤256 Bytes  ~0.3ms     单次最多 256 字节，需按 Page 对齐
擦除      4KB          ~50ms       必须先擦除才能写入新数据
```

> `FLASH_ERASE_4K = 1` 表示支持 4KB 扇区擦除。

### 4.2 更新流程

```
对于一个 20KB 的槽位：

1. 擦除 5 个 4KB 扇区:
   os_spiflash_erase(addr + 0x0000);  // sector 0
   os_spiflash_erase(addr + 0x1000);  // sector 1
   os_spiflash_erase(addr + 0x2000);  // sector 2
   os_spiflash_erase(addr + 0x3000);  // sector 3
   os_spiflash_erase(addr + 0x4000);  // sector 4

2. 逐页写入新 MP3 (每页 ≤256 字节):
   for (off = 0; off < mp3_size; off += 256) {
       os_spiflash_program(buf + off, addr + off, min(256, mp3_size - off));
   }

3. 更新资源表 LEN 字段:
   读 4KB 扇区 → 修改 4 字节 → 擦除 → 写回
```

### 4.3 更新 LEN 字段的特殊处理

LEN 字段（如 `0x1100005c`）和相邻资源表条目在同一个 4KB 扇区，直接擦除会破坏相邻条目：

```
资源表扇区 0x11000000~0x11000FFF
┌──────────────────────────────────────┐
│ 条目0  条目1  条目2  条目3  条目4  ...│
│        ↑ LEN 在这里 (0x1100005c)     │
└──────────────────────────────────────┘

安全更新 LEN 步骤:
1. u8 buf[4096];
2. os_spiflash_read(buf, 0x11000000, 4096);   // 读整个扇区
3. *(u32 *)(buf + 0x5c) = new_len;            // 修改 LEN
4. os_spiflash_erase(0x11000000);              // 擦除扇区
5. os_spiflash_program(buf, 0x11000000, 4096); // 整扇写回
```

---

## 五、UART 通信协议设计

### 5.1 数据包格式

```
┌────────┬────────┬──────────┬──────────┬──────────┬────────┐
│ Magic  │ CMD    │ FileID   │ Addr     │ Len      │ Data   │
│ 2B     │ 1B     │ 1B       │ 4B       │ 2B       │ N Bytes│
│ 0x55AA │        │ 0~N      │ Flash地址│ 数据长度 │ MP3数据│
└────────┴────────┴──────────┴──────────┴──────────┴────────┘

命令定义:
  0x01  查询/握手        无数据
  0x02  擦除扇区         数据区 = 擦除的 Flash 地址 (4B)
  0x03  写入数据块        数据区 = MP3 数据 (≤256B)
  0x04  更新 LEN         数据区 = 新的 LEN 值 (4B)
  0x05  完成/重启         无数据
  0xFF  NACK/错误         数据区 = 错误码 (1B)
```

### 5.2 ESP32S3 发送流程

```python
# 伪代码
def update_mp3(file_id, mp3_data):
    addr = RES_ADDR_TABLE[file_id]   # 预编译的地址表
    len_addr = RES_LEN_ADDR_TABLE[file_id]

    # 1. 握手
    uart_send_cmd(0x01, 0, b'')

    # 2. 擦除 20KB 槽位 (5 个扇区)
    for i in range(5):
        uart_send_cmd(0x02, 0, struct.pack('<I', addr + i * 0x1000))
        wait_ack()

    # 3. 写入 MP3 数据 (逐页 256 字节)
    for off in range(0, len(mp3_data), 256):
        chunk = mp3_data[off:off+256]
        uart_send_cmd(0x03, 0, struct.pack('<IH', addr + off, len(chunk)) + chunk)
        wait_ack()

    # 4. 更新 LEN
    uart_send_cmd(0x04, 0, struct.pack('<I', len(mp3_data)))
    wait_ack()

    # 5. 完成
    uart_send_cmd(0x05, 0, b'')
```

### 5.3 固定地址映射表（编译后确定）

```c
// ESP32S3 侧硬编码（从 res.h 提取）
const flash_addr_map_t res_map[] = {
    {0, 0x110008A0, 0x1100003c, "max_vol.wav"},
    {1, 0x11000DA0, 0x1100005c, "poweron.mp3"},   // 若 20KB 槽位
    {2, 0x110012A0, 0x1100007c, "ring.mp3"},
    {3, 0x110017A0, 0x1100009c, "update.mp3"},
    {4, 0x11001CA0, 0x110000bc, "update_done.mp3"},
    // ... 39 个英文提示音
};
```

---

## 六、AB5605B 侧接收代码框架

```c
// func_mp3_uart_upd.c — 约 80 行

#include "include.h"

#define MP3_UPD_SLOT_SIZE    0x5000   // 20KB 槽位
#define MP3_UPD_PAGE_SIZE    256      // Page Program 最大 256 字节

static u8 rx_buf[MP3_UPD_PAGE_SIZE + 8];  // 接收缓冲区

// 扇区备份缓冲 (用于安全更新资源表)
static u8 sector_buf[4096] AT(.upd_buf);

typedef struct {
    u16 magic;      // 0x55AA
    u8  cmd;
    u8  file_id;
} mp3_upd_hdr_t;

void mp3_upd_update_len(u32 len_addr, u32 new_len)
{
    // 读扇区 → 改 LEN → 擦除 → 写回
    u32 sector_addr = len_addr & ~0xFFF;  // 4KB 对齐
    os_spiflash_read(sector_buf, sector_addr, 4096);
    *(u32 *)(sector_buf + (len_addr & 0xFFF)) = new_len;
    os_spiflash_erase(sector_addr);
    os_spiflash_program(sector_buf, sector_addr, 4096);
}

void mp3_upd_write_data(u32 addr, u8 *data, u16 len)
{
    os_spiflash_program(data, addr, len);
}

void mp3_upd_erase_sectors(u32 addr, u32 size)
{
    for (u32 off = 0; off < size; off += 4096) {
        os_spiflash_erase(addr + off);
    }
}

// 主处理函数 — 在消息循环中调用
void mp3_uart_update_process(void)
{
    mp3_upd_hdr_t *hdr = (mp3_upd_hdr_t *)rx_buf;

    if (!uart_recv_packet(rx_buf)) return;

    switch (hdr->cmd) {
    case 0x01: // 握手
        uart_send_ack(0);
        break;

    case 0x02: { // 擦除
        u32 addr = *(u32 *)(rx_buf + 4);
        mp3_upd_erase_sectors(addr, MP3_UPD_SLOT_SIZE);
        uart_send_ack(0);
        break;
    }

    case 0x03: { // 写入数据块
        u32 addr = *(u32 *)(rx_buf + 4);
        u16 len  = *(u16 *)(rx_buf + 8);
        mp3_upd_write_data(addr, rx_buf + 10, len);
        uart_send_ack(0);
        break;
    }

    case 0x04: { // 更新 LEN
        u32 new_len = *(u32 *)(rx_buf + 4);
        u32 len_addr = res_len_table[hdr->file_id];
        mp3_upd_update_len(len_addr, new_len);
        uart_send_ack(0);
        break;
    }

    case 0x05: // 完成
        uart_send_ack(0);
        // 可选：软重启使新资源生效
        break;
    }
}
```

---

## 七、风险与注意事项

| 风险 | 等级 | 说明 | 缓解措施 |
|------|------|------|----------|
| **Flash 擦写期间中断** | 🟡 低 | 资源区擦写时不影响代码执行（代码在 0x10000000），只要不播放 MP3 即可 | 更新前停止所有音频播放 |
| **断电导致资源表损坏** | 🔴 中 | 更新 LEN 需要读-改-写扇区，中途断电会导致整扇区丢失 | 1. 先用备份扇区 `cm_write` 2. 校验后再写正式扇区 |
| **新 MP3 超出槽位大小** | 🟡 低 | 超出部分覆盖下一个文件 | ESP32S3 侧做长度检查 |
| **槽位顺序变化** | 🟡 低 | 增删 res/ 目录文件会导致 xmaker 重新排序 | 锁定 res/ 下的文件列表 |
| **Flash 寿命** | 🟢 极低 | NOR Flash 10万次擦写寿命，偶尔更新无影响 | 无需特别处理 |
| **MP3 格式兼容** | 🟡 低 | 非标准 MP3 帧可能导致解码器异常 | 使用固定编码参数（128kbps CBR, 44100Hz） |

---

## 八、实施步骤总结

```
Phase 1: 编译期准备
  1. 将所有 MP3 文件统一填充到 20KB（尾部加静音帧）
  2. 编译 → 从 res.h 提取固定地址表
  3. 将地址表硬编码到 ESP32S3 固件

Phase 2: AB5605B 侧
  4. 新增 func_mp3_uart_upd.c（约 80 行）
  5. 注册 UART 消息处理
  6. 接收指令 → os_spiflash_erase → os_spiflash_program → 更新 LEN

Phase 3: ESP32S3 侧
  7. 实现 UART 命令发送（约 200 行）
  8. 按固定地址表写入 MP3 数据
  9. 校验完成后发送更新 LEN 命令

Phase 4: 测试验证
  10. 更新一个 MP3 → 触发播放 → 确认语音正确
  11. 断电重启 → 确认语音保持
  12. 压力测试连续更新 5 个文件
```

---

## 九、总结

> **可行性**: ✅ **完全可行**
>
> **核心原理**: 将 MP3 文件编译时统一为固定大小（20KB），使 xmaker 打包后每个资源的 Flash 地址固定不变。ESP32S3 硬编码地址表，通过 UART 发送原始 MP3 数据，AB5605B 使用 SDK 提供的 `os_spiflash_erase` + `os_spiflash_program` API 直接写入内部 NOR Flash。
>
> **工作量**: AB5605B 侧 ~80 行 + ESP32S3 侧 ~200 行
>
> **关键依赖**: `os_spiflash_erase` / `os_spiflash_program` (SDK 预编译库已提供) + `FLASH_ERASE_4K` (已配置)
