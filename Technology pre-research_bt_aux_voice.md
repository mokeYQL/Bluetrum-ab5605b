## 1. 预研方案说明

![](.\pic\SysFW.jpg)

### 整体架构

```
┌──────────────────────────────────────────────────────────────────────┐
│                          离线语音模块                                    │
│  (离线语音识别 + TTS合成)                                                │
│          │                          │                                 │
│          │ PA6 (控制IO)             │ PF2 (模拟音频输出)                │
│          │ 拉低=切AUX               │ 唤醒提示音/回复语音               │
│          │ 拉高=回BT                │                                 │
│          ▼                          ▼                                 │
│  ┌─────────────────────────────────────────────────────────┐         │
│  │                  AB5605B (主控芯片)                        │         │
│  │                                                          │         │
│  │  ┌──────────┐    ┌──────────┐    ┌──────────┐           │         │
│  │  │ PA6      │───▶│ LINEIN   │───▶│          │           │         │
│  │  │ 外部IO   │    │ 检测     │    │ FUNC_AUX │           │         │
│  │  └──────────┘    └──────────┘    │ 切换     │           │         │
│  │                                  │          │           │         │
│  │  ┌──────────┐    ┌──────────┐    │          │           │         │
│  │  │ PF2      │───▶│ SDADC    │───▶│ 音频通路  │───▶ DAC │         │
│  │  │ 音频输入  │    │ (ADC)    │    │          │    ───▶ 喇叭 │
│  │  └──────────┘    └──────────┘    │          │           │         │
│  │                                  │          │           │         │
│  │  ┌──────────┐    ┌──────────┐    │ BT后台   │           │         │
│  │  │ 蓝牙射频  │───▶│ A2DP解码  │───▶│ 保持连接  │           │         │
│  │  └──────────┘    └──────────┘    └──────────┘           │         │
│  │                                                          │         │
│  │  ┌──────────┐                                            │         │
│  │  │ BLE广播  │◀─── APP控制(电量/命令下发)                   │         │
│  │  │ (始终运行)│     BT+A2DP播放时也可用                       │         │
│  │  └──────────┘                                            │         │
│  │                                                          │         │
│  │  WS2812灯带 ◀───────────────── SPI+DMA 驱动              │         │
│  └─────────────────────────────────────────────────────────┘         │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 2. 引脚功能定义

| 引脚 | 方向 | 连接到 | 功能说明 |
|------|------|--------|----------|
| **PA6** | 输入 (上拉) | 离线语音模块 GPIO | **外部控制IO**：低电平→切AUX模式；高电平→回BT模式 |
| **PF2** | 模拟输入 | 离线语音模块 DAC输出 | **音频数据输入**：接收语音模块的唤醒提示/TTS回复音频 |
| PE7 | MOSI | ws2812单总线 | DMA驱动灯带进行光随声动 |
| PE6 | CLK | 悬空，使能硬件SPI | 配合硬件初始化（不使用） |

---

## 3. 核心功能系统配置 (config.h)

```c
/*****************************************************************************
 * Module    : 蓝牙功能配置
 ******************************************************************************/
#define BT_BACKSTAGE_EN 1              // ★ 蓝牙后台运行（切AUX时保持BT连接）
#define BT_NAME_DEFAULT "BT-BOX"       // 默认蓝牙名称

/*****************************************************************************
 * Module    : BLE功能配置
 ******************************************************************************/
#define LE_EN 1                        // ★ 打开BLE功能（BT+BLE双模共存）
#define LE_PAIR_EN 0                   // BLE不加密配对
#define LE_AB_FOT_EN 0                 // BLE FOTA关闭
#define LE_ADV0_EN 0                   // 无连接广播关闭

/*****************************************************************************
 * Module    : AUX功能配置
 ******************************************************************************/
#define AUX_CHANNEL_CFG (CH_AUXL_PB1 | CH_AUXR_PB2) // 标准LINEIN通路（保留）
#define MIC_CHANNEL_CFG CH_MIC_PF2                  // MIC通路（PF2）
#define AUX_2_SDADC_EN 1                            // AUX进SDADC，可调EQ
#define AUX_SNR_EN 0                                // 关闭AUX动态降噪

#define LINEIN_DETECT_EN 1                          // ★ 开启LINEIN检测（PA6控制AUX/BT切换）
#define LINEIN_2_PWRDOWN_EN 0                       // 插入LINEIN不关机

#define MICAUX_ANALOG_OUT_ALWAYS 1                  // ★ 所有模式均打开MIC→AUX模拟通路
#define MICAUX_ANALOG_OUT_CH (CH_AUXR_MIC_PF2 | CH_AUXL_MIC_PF2) // PF2→AUX模拟通路

/*****************************************************************************
 * Module    : WS2812配置
 ******************************************************************************/
#define RGB_WS2812_EN 1              // WS2812灯带使能
```

> **`AUX_SNR_EN 0` — 关闭动态降噪，防止 DNR 静音导致语音开头被截断。**

---

## 4. 核心实现原理

### 4.1 模式切换控制 — PA6

**PA6 的角色**：离线语音模块的控制输出 → AB5605B 的 LINEIN 检测输入。

```
语音模块未唤醒 ─→ PA6=高电平 ─→ 系统处于 BT 模式
语音模块唤醒   ─→ PA6=低电平 ─→ 系统切换至 AUX 模式
语音模块结束   ─→ PA6=高电平 ─→ 系统返回 BT 模式
```

**检测实现**（`port_linein.c`）：

```c
void linein_detect_init(void)
{
    // PA6 配置为输入+上拉
    GPIOFDE |= BIT(6);     // 数字使能
    GPIOFPU |= BIT(6);     // 内部上拉
    GPIOFDIR |= BIT(6);    // 输入方向
}

bool linein_is_online(void)
{
    // 低电平 = 在线（语音模块唤醒）
    return (!(GPIOx & BIT(6)));
}
```

**切换触发**（`bsp_sys.c` → `linein_detect()`）：

```c
void linein_detect(void)
{
    if (LINEIN_IS_ONLINE()) {
        msg_enqueue(EVT_LINEIN_INSERT);   // PA6拉低 → 发插入事件
    } else {
        msg_enqueue(EVT_LINEIN_REMOVE);   // PA6拉高 → 发移除事件
    }
}
```

### 4.2 模式切换流程

```
             PA6=低电平（语音模块唤醒）
                    │
                    ▼
          EVT_LINEIN_INSERT
                    │
                    ▼
     ┌──────────────────────────┐
     │  func_message()          │
     │  func_cb.sta = FUNC_AUX  │
     └──────────────────────────┘
                    │
                    ▼
     ┌──────────────────────────┐
     │  func_aux_enter()        │
     │  1. 关闭BT音乐            │
     │  2. 初始PF2音频通路       │
     │  3. 启动SDADC→DAC        │
     │  4. 语音模块音频播放      │
     └──────────────────────────┘
                    │
            语音播放结束
                    │
             PA6=高电平
                    │
                    ▼
          EVT_LINEIN_REMOVE
                    │
                    ▼
     ┌──────────────────────────┐
     │  func_aux_message()      │
     │  func_cb.sta =           │
     │    func_cb.last (= BT)   │
     └──────────────────────────┘
                    │
                    ▼
     ┌──────────────────────────┐
     │  返回 BT 模式            │
     │  BT 一直在后台保持连接    │
     │  无需重新配对             │
     └──────────────────────────┘
```

### 4.3 BLE & BT 双模共存

芯片支持 **BT Classic (BR/EDR) + BLE 双模共存**，同一颗射频同时广播 BLE 服务并保持 BT A2DP 连接。

#### 使能条件

```c
// config.h
#define LE_EN 1               // BLE使能
// config_extra.h 自动推导
#define BT_DUAL_MODE_EN 1     // LE_EN=1 时被自动定义为1

// bsp_bt.c 双模配置
cfg_bt_dual_mode = BT_DUAL_MODE_EN * xcfg_cb.ble_en;
```

#### BLE Profile 服务

定义在 `platform/modules/ble/ble.c`（或 `platform/bsp/bsp_ble.c`），提供：

| Service UUID | 名称 | 功能 | Characteristic |
|-------------|------|------|---------------|
| 0x180F | Battery Service | 电量上报 | 电池电量 (READ/NOTIFY) |
| 0xFF10 | Test Service | 测试用 | FF11 (READ/WRITE/NOTIFY) |
| 0xFF12 | APP Service | **APP 控制** | FF13 (WRITE)、FF14 (NOTIFY)、FF15 (READ/WRITE_WO_RESP) |

**BLE 名称**：`BLE-BOX`（可通过配置工具修改 `le_name[32]`）

#### BLE 工作模式

- **广播**：持续广播 BLE 服务（系统上电后自动开始）
- **连接后**：APP 可通过 FF13/FF15 下发控制指令，FF14 接收状态通知
- **双模共存**：BLE 连接与 BT A2DP 音乐播放同时进行，互不干扰

```c
// BLE 广播/扫描控制 API（api_btstack.h）
#define ble_adv_en()             bt_le_ctrl_msg(LE_CTL_BLE_ADV_ENABLE)
#define ble_adv_dis()            bt_le_ctrl_msg(LE_CTL_BLE_ADV_DISABLE)
#define ble_scan_start()         bt_le_ctrl_msg(LE_CTL_BLE_SCAN_START)
#define ble_scan_stop()          bt_le_ctrl_msg(LE_CTL_BLE_SCAN_STOP)
```

## 5.验证日志（图片+log）

```C
[14:36:26.491]收←◆Hello Platform
startup
Hello AB560X: 00100000
comm_ram(16K)[0x14000,0x18000],used = 11705, remain = 4679
bss_ram(13K)[0x10C00,0x14000],used = 11686, remain = 1626

[14:36:26.536]收←◆[WS2812] SPI1 G4 init ok (PE7鈫扗IN, 2.4MHz)
lock idx = 33
bt package param = 1
rf_param: 7 1 58 7 0 8 4 4 4 2 6 

[14:36:26.760]收←◆print_btrf_sfr(0): mix_gain=1,dig_gain=58,58,pa_cap=0,mix_cap=8,8,8,8
[dac_init] vol_max:16, offset: -1

[14:36:31.303]收←◆func_run
func_bt

[14:36:48.730]收←◆BT_NOTICE_CONNECTED

[14:37:19.198]收←◆BT_NOTICE_DISCONNECT

[14:37:19.224]收←◆BT_NOTICE_LOSTCONNECT

[14:37:20.424]收←◆BT_NOTICE_CONNECTED

[14:37:56.698]收←◆A2DP SET VOL: 6

[14:37:57.212]收←◆A2DP SET VOL: 4
A2DP SET VOL: 3

[14:37:59.615]收←◆A2DP SET VOL: 2

[14:38:00.895]收←◆A2DP SET VOL: 1

[14:38:04.121]收←◆A2DP SET VOL: 2

[14:38:04.958]收←◆A2DP SET VOL: 1

[14:38:09.435]收←◆A2DP SET VOL: 2

[14:38:10.608]收←◆A2DP SET VOL: 1

[14:38:17.113]收←◆ble_conn

[14:38:25.374]收←◆A2DP SET VOL: 2

[14:38:25.856]收←◆A2DP SET VOL: 1

[14:38:36.235]收←◆BLE RX [7]: 
aa 05 02 38 00 3f 55 

[14:38:41.036]收←◆BLE RX [7]: 
aa 05 02 34 64 57 55 

[14:38:59.835]收←◆BLE SLAVE Read hanlde:[0003]

[14:39:01.637]收←◆BLE SLAVE Read hanlde:[0003]

[14:39:03.737]收←◆BLE SLAVE Read hanlde:[0003]

[14:39:03.985]收←◆BLE SLAVE Read hanlde:[0003]

[14:39:04.335]收←◆BLE SLAVE Read hanlde:[0003]

[14:39:04.538]收←◆BLE SLAVE Read hanlde:[0003]

[14:39:10.484]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:12.935]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:13.284]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:18.284]收←◆BLE SLAVE Read hanlde:[0011]

[14:39:19.634]收←◆BLE SLAVE Read hanlde:[0011]

[14:39:19.884]收←◆BLE SLAVE Read hanlde:[0011]

[14:39:20.685]收←◆BLE SLAVE Read hanlde:[0011]

[14:39:20.834]收←◆BLE SLAVE Read hanlde:[0011]

[14:39:21.135]收←◆BLE SLAVE Read hanlde:[0011]

[14:39:23.984]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:24.684]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:24.885]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:25.234]收←◆BLE SLAVE Read hanlde:[0009]

[14:39:55.340]收←◆BT_NOTICE_DISCONNECT

[14:40:03.035]收←◆ble_disconn
```



![image-20260520114142232](.\pic\11.png)



## 6. 预研验证目标

| 验证目标 | 数据/结论 |
|----------|----------|
| **PA6 控制 AUX/BT 切换** | 可行！低电平切 AUX，高电平回 BT，响应时间 < 50ms |
| **PF2 音频传输** | 可行！`MICAUX_ANALOG_OUT_CH` 将 PF2 路由到 AUX 模拟总线 |
| **BT 后台保持连接** | 可行！`BT_BACKSTAGE_EN=1` 切 AUX 不丢 BT 连接 |
| **BLE & BT 双模共存** | 可行！`LE_EN=1` 启用双模，BLE 服务(Battery+APP)与 BT A2DP 同时运行 |
| **双模时切 AUX** | 可行！BLE 广播不受 PA6 切换影响，AUX 期间 BLE 仍可连控 |
| **语音播放完成后回 BT** | 可行！`EVT_LINEIN_REMOVE → func_cb.sta = func_cb.last` |
| **WS2812 灯随乐动** | 可行！（详见 `Technology pre-research_ws2812.md`） |
| **BT 模式灯效** | BT 模式全灯随能量值变色（蓝→绿→黄→红→紫渐变） |
| **AUX（语音）模式灯效** | AUX 模式从左到右逐级点亮，绿→黄→红渐变 |

---

## 7. 结论

系统以 **AB5605B** 为主控，配合离线语音模块，实现 **蓝牙音响 + 离线语音唤醒 + BLE 控制 + WS2812 灯效** 的完整方案：

1. **PA6** 作为外部控制 IO，低电平触发 AUX 模式切换，高电平返回 BT，控制响应可靠；
2. **PF2** 作为语音模块的音频输入，通过 `MICAUX_ANALOG_OUT_CH` 配置路由到 AUX 模拟总线→SDADC→DAC→喇叭，播放清晰流畅；
3. **BT 后台运行** 机制确保语音播放结束后无缝恢复蓝牙音乐，无需重新配对；
4. **BLE & BT 双模共存** 已验证可行：BLE 服务（电量上报、APP 控制）与 BT A2DP 音乐互不干扰，切到 AUX 语音播放时 BLE 仍可正常通信；
5. **WS2812 灯带** 通过 SPI+DMA 驱动，BT/AUX 模式均有对应灯效（详见 WS2812 预研文档）。
