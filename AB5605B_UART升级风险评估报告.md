# AB5605B UART 升级风险评估报告

> **一句话结论**：AB5605B 是单 Bank Flash 芯片，没有独立 Bootloader，升级时是"App 自己擦写自己"。**升级过程中断电 = 变砖**，只能拆机用烧录器恢复。这是硬件架构决定的，软件无法规避。

---

功能需求

✅走A2DP  手机APP可以播放音乐（  SBC解码）
✅AUX模式（外部语音也通过该芯片进行播放），切换AUX模式
✅硬件SPI支持驱动WB2812灯带
✅本地语音播放（MP3/WAV） 10条左右 /一个字1.8KB -估算
❌稳定的OTA （不要单分区 ，要有双备份 版本可回退）

硬件配置

- 串口1  PA7&PA6  
-  IO控制  PA5 
- PB3  下载&log（1500000）串口0 
- 音频输入（AUX模式）  PF2   PA5
- WS2812   PE7 SPIDO   PE6 SPICLK 
- 静音 PE0

## 一、为什么升级会变砖？

### 正常升级 vs 断电变砖

```mermaid
flowchart LR
    subgraph OK["✅ 正常升级"]
        A1["接收固件"] --> A2["擦除旧代码"] --> A3["写入新代码"] --> A4["全部完成"] --> A5["重启生效 ✓"]
    end
    subgraph BAD["💀 升级中断电"]
        B1["接收固件"] --> B2["擦除旧代码"] --> B3["写入一半..."] --> B4{"断电!"}
        B4 --> B5["Flash: 半新半空"]
        B5 --> B6["重启 → 残缺代码 → 无法启动"]
        B6 --> B7["变砖 💀 拆机才能救"]
    end
    style OK fill:#e8f5e9
    style BAD fill:#ffebee
```

### 根本原因：单 Bank，没有备份

```mermaid
flowchart TB
    subgraph AB["AB5605B 单 Bank（本项目）"]
        direction LR
        AB1["启动区<br/>512B"] --> AB2["代码区 492KB<br/>唯一，升级时直接覆写"]
    end
    subgraph ESP["ESP32 双 Bank（对比）"]
        direction LR
        E1["Bootloader<br/>受保护"] --> E2["App A<br/>运行中"] --> E3["App B<br/>备份"]
    end
    AB2 -.->|"断电 = 原地变废墟"| X["❌ 无法回滚"]
    E3 -.->|"断电 = B 废了, A 完好"| Y["✅ 自动回滚 A"]
    style AB2 fill:#ffcdd2
    style E1 fill:#c8e6c9
    style E3 fill:#c8e6c9
```

---

## 二、三大核心证据

### 证据 1：Flash 只有一份，没有备份区

**文件**：`projects/standard/config.h:51-52`

```c
#define FLASH_SIZE          FSIZE_512K   // 芯片内置 512KB
#define FLASH_CODE_SIZE     492K         // 程序使用空间（全部用完）
```

**文件**：`projects/standard/ram.ld:18-31`

```c
MEMORY {
    init   : org = __base,          len = 512        // 启动区
    flash  : org = __base + 512,    len = 492K       // ← 唯一代码区，无备份
    comm   : org = 0x14000,         len = 16k        // RAM（非 Flash）
}
```

> 512KB Flash 全部用完，没有空间放第二份固件。双 Bank 需要额外 492KB，物理上不可能。

---

### 证据 2：App 自己擦写自己，没有独立 Bootloader

**文件**：`platform/functions/func_update.c:15-48`

```c
AT(.text.func.update)              // ← 代码放在 Flash，不是 RAM
void func_update(void)
{
    updateproc();                  // ← App 自己擦写自己的 Flash
    while (1);                     // ← 等死，靠 WDT 复位
}
```

**文件**：`platform/modules/app/app_fota/app_fota.c:84-88`

```c
AT(.text.fot.cache)
void app_fota_write(void *buf, u32 addr, u32 len)
{
    fot_write(buf, addr, len);     // ← 直接擦写自身 Flash 代码区
}
```

> `updateproc()` 和 `fot_write()` 是 App 自己的函数，不是独立的 Bootloader。App 运行的同时擦写自己的代码区，一旦中断，Flash 中留下残缺代码。

---

### 证据 3：.map 文件证实升级代码在 Flash 运行



**文件**：`projects/standard/Output/bin/map.txt`

```
# 升级核心函数 updateproc() 的地址（map.txt:5557-5559）
.text.update   0x000000001000a884   0x982   ← 0x1000xxxxx 是 Flash 地址
               updateproc           0x1000af12   ← 升级核心，在 Flash 执行

# UART 中断在 RAM（map.txt:4637-4638）
.com_text.uart 0x000000000001467c   0xae     ← 0x000xxxxx 是 RAM 地址
               uart1_isr                     ← 中断服务，在 RAM 执行

# .comm 段：VMA≠LMA，证明从 Flash 拷贝到 RAM（map.txt:4587）
.comm          0x0000000000014000   0x2e00   load address 0x0000000010001600
```

| 组件 | 运行位置 | 擦 Flash 时 |
|------|---------|------------|
| UART 中断 `uart1_isr` | **RAM** 0x1467c | ✅ 安全 |
| 升级核心 `updateproc()` | **Flash** 0x1000af12 | ⚠️ 危险 |
| 数据缓冲 `.uart_upd_buf` | **RAM** | ✅ 安全 |

> 中断和数据缓冲在 RAM 中安全运行，但 `updateproc()` 自己也在 Flash 中。擦写到自己所在页时断电，连升级代码本身都会残缺。



---

## 三、升级失败时序图

```mermaid
sequenceDiagram
    participant ESP as ESP32S2 (主机)
    participant AB as AB5605B (从机)
    participant Flash as 片内 Flash

    Note over AB,Flash: App 从 Flash 执行中

    ESP->>AB: 握手 START_UPD
    AB->>ESP: 响应 OK
    AB->>AB: 关闭蓝牙/外设

    loop 每 512 字节
        ESP->>AB: 固件数据块
        AB->>Flash: 擦除页 + 写入新数据
        AB->>ESP: 状态响应
    end

    Note over AB,Flash: ⚡ 此刻断电！

    Note over Flash: 状态：前 N 页新代码，后 (492K-N) 页空白/旧代码

    Note over AB: 重新上电
    AB->>Flash: 从 0x0 取指令
    Flash-->>AB: 残缺代码 💀
    Note over AB: 无法启动 → 变砖
```

---

## 五、最终结论

| 问题 | 答案 |
|------|------|
| 支持 UART 升级？ | ✅ 支持 |
| 不断电时可靠？ | ✅ 可靠 |
| 升级中断电会变砖？ | ⚠️ **会**（单 Bank 无 Bootloader，App 自擦自写） |
| 变砖后能救吗？ | ❌ **不能**，只能拆机 ISP 烧录器 |
| 能否消除风险？ | ❌ **不能**，硬件架构决定 |

> **建议**：量产产品若要 OTA，必须保证升级期间供电稳定（电池备份 + VUSB 不掉线），并准备好 ISP 烧录器的售后维修预案。若要求"零变砖"，唯一方案是硬件引出 SPI Flash 编程引脚，由 ESP32 直接操作 Flash（AB5605B 不参与）。

---

## 附录：证据索引

| # | 文件 | 行号 | 证明内容 |
|---|------|------|---------|
| 1 | `config.h` | 51-52 | Flash 512K 全部用完 |
| 2 | `ram.ld` | 18-31 | 单 Flash 区，无备份分区 |
| 3 | `func_update.c` | 15-48 | `updateproc()` 自擦自写 |
| 4 | `app_fota.c` | 84-88 | `fot_write()` 直接写 Flash |
| 5 | `map.txt` | 4587 | `.comm` 段 VMA≠LMA（RAM 运行） |
| 6 | `map.txt` | 4637 | `uart1_isr` 在 RAM (0x1467c) |
| 7 | `map.txt` | 5557 | `updateproc` 在 Flash (0x1000af12) |
| 8 | `升级方案与协议详解.md` | 0.2/0.4/2 节 | 官方确认变砖风险 |

---

## 附：常见疑问解答

### Q1：升级代码在 Flash 执行，断电后上电重新跑升级、ESP32 重发不就行了吗？

**答：不行。升级是按地址从低到高擦写的，启动代码和 UART 中断的 Flash 副本会先被毁掉，重新上电后没有任何代码能接管。**

#### 升级擦写机制（修正版）

**Flash 硬件规则**：CPU 从页 A 执行时可以擦写页 B，但**不能擦写当前正在执行的页**（硬件死锁保护）。所以 `updateproc()` 自己所在的页要么最后擦（擦完立刻 `while(1)` 复位），要么不擦。

**真正的风险在于：擦写其他页时断电**

```mermaid
flowchart TB
    subgraph flash["Flash 布局（地址从低到高）"]
        F1["0x10000000 init 启动向量 512B"]
        F2["0x10000200 启动代码 bsp_sys_init"]
        F3["0x10001600 .comm Flash 副本<br/>← UART ISR 的源数据！"]
        F4["0x10004400 主代码区<br/>main / func_update 等"]
        F5["0x1000a800 升级代码区 updateproc<br/>← 最后擦或不擦"]
        F1 --> F2 --> F3 --> F4 --> F5
    end

    subgraph order["实际擦写顺序"]
        W1["① 擦 0x10000200<br/>启动代码没了"]
        W2["② 擦 0x10001600<br/>.comm Flash 副本没了"]
        W3["③ 擦 0x10004400<br/>主代码区没了"]
        W4["④ 最后擦 0x1000a800<br/>updateproc 自己<br/>擦完立刻 while1 复位"]
        W1 --> W2 --> W3 --> W4
    end

    F2 -.-> W1
    F3 -.-> W2
    F4 -.-> W3
    F5 -.-> W4

    style W1 fill:#ffcdd2
    style W2 fill:#ffcdd2
    style W3 fill:#ffcdd2
    style W4 fill:#fff9c4
```

> **关键修正**：`updateproc()` 所在页（0x1000a800）是**最后擦**的（擦完立刻复位），不会"擦自己正在执行的代码"。但 ①②③ 步擦的是启动代码、`.comm` Flash 副本、主代码区——这些被擦后断电，重新上电就起不来。

#### 重新上电后会发生什么？

```mermaid
flowchart TD
    P["重新上电"] --> B1["Boot ROM 从 0x0 读启动向量"]
    B1 --> B2{"init 区完整？"}
    B2 -->|可能被擦| FAIL1["❌ 启动向量损坏，直接死"]
    B2 -->|没被擦| B3["拷贝 .comm 从 Flash 0x10001600 到 RAM"]
    B3 --> B4{"Flash 0x10001600 完整？"}
    B4 -->|已被新固件覆盖| FAIL2["❌ UART ISR 加载的是垃圾<br/>串口收不到任何数据"]
    B4 -->|碰巧没被擦| B5["跳转到 main 执行"]
    B5 --> B6{"main 代码完整？"}
    B6 -->|新旧代码混合| FAIL3["❌ 代码不兼容，崩溃<br/>无法进入升级流程"]
    B6 -->|碰巧完整| B7["能运行，但概率极低"]

    style FAIL1 fill:#ffcdd2
    style FAIL2 fill:#ffcdd2
    style FAIL3 fill:#ffcdd2
```

#### 三个致命点

| 致命点 | 说明 |
|--------|------|
| ① 启动代码先被毁 | 升级先擦低地址，`bsp_sys_init()` 在 0x10000200，最早被覆盖 |
| ② .comm Flash 副本被毁 | UART ISR 虽在 RAM 运行，但 RAM 数据是从 Flash 0x10001600 拷来的，Flash 副本没了，RAM 里加载的就是垃圾 |
| ③ 主代码区被毁 | `main()` / `func_update()` / 事件处理等都在 0x10004400+，被擦后无法触发 `EVT_UART_UPDATE` 进入升级流程 |

> **注意**：`updateproc()` 自己所在的页（0x1000a800）是最后擦的，擦完立刻复位——这不是变砖的原因。变砖的原因是 ①②③ 步擦毁了启动和通信所需的代码，断电后没有任何代码能接管。

#### 对比：为什么 ESP32 断电后能重试？

```mermaid
flowchart LR
    subgraph ab["AB5605B"]
        A1["断电"] --> A2["上电"] --> A3{"启动代码在？"}
        A3 -->|已被擦| A4["💀 死<br/>没有代码能接管"]
    end
    subgraph esp["ESP32（有 Bootloader）"]
        E1["断电"] --> E2["上电"] --> E3["Bootloader 运行<br/>受保护，永远在"]
        E3 --> E4["检测 App 损坏"] --> E5["进入升级模式<br/>重新接收固件 ✓"]
    end
    style A4 fill:#ffcdd2
    style E5 fill:#c8e6c9
```

> **一句话**：ESP32 断电后能重试，是因为 Bootloader 在受保护区，永远活着。AB5605B 没有 Bootloader，升级把启动代码自己擦了，断电后**没有任何代码能接管**，连串口都收不了数据。
