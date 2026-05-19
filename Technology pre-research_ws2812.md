## 1.预研方案说明



![](.\pic\SysFW.jpg)



### WS2812原理

#### 底层原理

WS2812 是一个单线串行通信的 RGB LED 灯珠，每颗灯珠内部有 IC，支持级联。

时序如下

![](.\pic\ws2812.png)



![](.\pic\3.png)

本预研解决方案：**SPI+DMA（数据发送无需CPU参与）  频率2.4MHZ  3bit代表bit0、bit1**

- SPI时序时间：~417ns

展开数据流程如下：

**WS2812 数据:   1        0        1        0       ...**
  **↓ 编码             ↓          ↓          ↓**
**SPI 比特流:     110      100    110      100      ...  （3倍膨胀）**

**SPI 寄存器:   一次发 8 个 SPI bit，拼成一个 SPI 字节**

#### RGB颜色说明

每个颜色值范围 0~255（8bit）一个灯珠：颜色 = (R值, G值, B值) ->3字节

 例如 (255, 0, 0)  → 纯红色/(0, 255, 0)  → 纯绿色/ (0, 0, 255)  → 纯蓝色

 组合256 * 256 * 256 = 16,777,216组合的色系



## 2.预研方案验证的目标

| 验证目标          | 数据/结论                                                    |
| ----------------- | ------------------------------------------------------------ |
| RAM占用           | ![image-20260515170005053](E:\Bluetrum-ab5605b\Bluetrum-ab5605b\pic\5.png)![image-20260515170158205](E:\Bluetrum-ab5605b\Bluetrum-ab5605b\pic\6.png)![image-20260515170257515](E:\Bluetrum-ab5605b\Bluetrum-ab5605b\pic\8.png)488 |
| FLASH占用         | ![image-20260515170324185](E:\Bluetrum-ab5605b\Bluetrum-ab5605b\pic\9.png)908K |
| 硬件影响          | 浪费SPI clk 口                                               |
| AUX模式的灯随乐动 | 可行！                                                       |
| BT模式灯随乐动    | 可行！                                                       |



## 3.预研验证的方案设计



### 灯效实现方案

利用 `dac_pcm_pow_calc()` 读取当前 DAC 音频能量值，驱动 WS2812 灯效：

**AUX 模式（音乐律动）**：40 颗灯珠从左到右逐级点亮，颜色呈绿→黄→红渐变。音量越大，亮灯越多，颜色越暖。

```c
// port_ws2812.c - AUX分支关键代码
if (func_cb.sta == FUNC_AUX) {
    energy = dac_pcm_pow_calc();          // 读取DAC音频能量
    // 能量 → 亮灯数映射（40灯，8级）
    if (energy < 300)         level = 0;      // 静音
    else if (energy < 1000)  level = 5;       // 小声 → 亮5颗
    else if (energy < 2500)  level = 10;      // 中声 → 亮10颗
    else if (energy < 5000)  level = 15;
    ...                        // 逐级递增
    else                     level = 40;      // 高潮 → 全亮
    // 从左到右填充，颜色绿→黄→红渐变
    for (i = 0; i < WS2812_NUM_LEDS; i++) {
        if (i < level)  ws2812_set_color(i, r, g, b);  // 亮
        else            ws2812_set_color(i, 0, 0, 0);   // 灭
    }
    ws2812_update();
}
```

**BT 模式（随声变色）**：40 颗灯珠全部显示同一颜色，颜色随能量值从蓝→绿→黄→红→紫渐变。音量越小偏冷色，音量越大偏暖色。

```c
// port_ws2812.c - BT分支关键代码
if (func_cb.sta == FUNC_BT) {
    energy = dac_pcm_pow_calc();          // 读取DAC音频能量
    // 能量 → 色相（0-255: 蓝→绿→黄→红→紫）
    u16 hue;
    if (energy < 300)   hue = 0;    // 蓝色
    else if (energy < 1200) hue = 32;
    else if (energy < 3000) hue = 64;  // 绿色
    ...                     // 逐级变化
    else                hue = 255;  // 紫色
    // 全灯显示同一颜色
    for (i = 0; i < WS2812_NUM_LEDS; i++) {
        ws2812_set_color(i, rr, gg, bb);
    }
    ws2812_update();
}
```

> 两种模式共享同一套 `dac_pcm_pow_calc()` 能量检测函数，音源不同但效果原理相同。刷新频率 80ms，人眼观感流畅。

![74052c3d451f3c48d3b029fe8fb7aee2](E:\Bluetrum-ab5605b\Bluetrum-ab5605b\pic\10.jpg)

## 结论

系统接入的ws2812灯带方案可行，**实现效果**方案需**项目经理和产品**重定义或一起定义。



## 附录1（AB5606B+WS2812编译后的数据）

| 项目               | 现在（正确）                        |
| ------------------ | ----------------------------------- |
| **Flash 总容量**   | 512KB                               |
| **Flash 编译占用** | **416KB** ✅（编译输出 `CODE SIZE`） |
| **Flash 剩余**     | **76KB**                            |
| **RAM 总容量**     | **128KB** ✅                         |
| **RAM 数据段**     | 21.2KB（16%）                       |
| **RAM 剩余**       | **~106KB**（充裕）                  |

