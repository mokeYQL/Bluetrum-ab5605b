#include "include.h"
#include "port_ws2812.h"

#if RGB_WS2812_EN

// PE6 作为 WS2812 数据线（无 SPI 冲突）
#define WS2812_BIT      BIT(6)
#define WS2812_SET      GPIOESET = WS2812_BIT
#define WS2812_CLR      GPIOECLR = WS2812_BIT

// 内部 RGB 数据缓冲
static u8 ws2812_rgb_buf[WS2812_BUF_SIZE];

// 音频能量平滑处理
static u16 ws2812_energy_avg = 0;



/* 微秒级延迟 */
static void ws2812_delay(u32 n)
{
    while (n--) {
        __asm__ volatile("nop");
    }
}

/* 发送 1 个字节到 WS2812 */
static void ws2812_send_byte(u8 d)
{
    u8 mask = 0x80;
    while (mask) {
        if (d & mask) {
            // Bit 1: T1H~700ns, T1L~600ns
            WS2812_SET;
            ws2812_delay(13);   // ~80 cycles ≈ 670ns
            WS2812_CLR;
            ws2812_delay(10);   // ~60 cycles ≈ 500ns
        } else {
            // Bit 0: T0H~350ns, T0L~900ns
            WS2812_SET;
            ws2812_delay(6);    // ~40 cycles ≈ 330ns
            WS2812_CLR;
            ws2812_delay(16);   // ~100 cycles ≈ 830ns
        }
        mask >>= 1;
    }
}

/* 初始化 PE6 为 WS2812 数据线 */
void ws2812_init(void)
{
    GPIOEDE  |= WS2812_BIT;     // 开启数字功能（PE6）
    GPIOEDIR &= ~WS2812_BIT;    // 输出模式
    WS2812_CLR;                 // 初始低电平
}

/* 发送整帧 RGB 数据到灯带 — 在主循环调用，不关中断 */
void ws2812_update(u8 *rgb_data)
{
    u16 i;
    for (i = 0; i < WS2812_BUF_SIZE; i++) {
        ws2812_send_byte(rgb_data[i]);
    }
    // 复位脉冲 >50us
    WS2812_CLR;
    delay_us(60);
}

/*
 * 将 HSL 色值转换为 RGB
 * h: 0-360, s: 0-100, l: 0-100
 */
static void ws2812_hsl2rgb(u16 h, u8 s, u8 l, u8 *r, u8 *g, u8 *b)
{
    u8 rgb_max = (l > 50) ? (l + 10) : (l * 2);
    u8 rgb_min = (l > 50) ? (l - 10) : 0;
    if (h < 60)      { *r = rgb_max; *g = h * rgb_max / 60;         *b = rgb_min; }
    else if (h < 120) { *r = (120-h) * rgb_max / 60; *g = rgb_max; *b = rgb_min; }
    else if (h < 180) { *r = rgb_min; *g = rgb_max; *b = (h-120) * rgb_max / 60; }
    else if (h < 240) { *r = rgb_min; *g = (240-h) * rgb_max / 60; *b = rgb_max; }
    else if (h < 300) { *r = (h-240) * rgb_max / 60; *g = rgb_min; *b = rgb_max; }
    else              { *r = rgb_max; *g = rgb_min; *b = (360-h) * rgb_max / 60; }
}

/* 在主循环中调用：计算 + 发送灯带数据（每80ms刷新一次）*/
void ws2812_flush(void)
{
    static u32 last_tick = 0;
    u16 energy, level;
    u8 i, r, g, b;

    // 每80ms更新一次（用 tick 计时，不依赖 ISR）
    if (!tick_check_expire(last_tick, 16)) return;  // 16 tick ≈ 80ms
    last_tick = tick_get();

    // 1. 读音频能量
    energy = dac_pcm_pow_calc();
    ws2812_energy_avg = (ws2812_energy_avg * 3 + energy) / 4;

    // 2. 能量 → 灯数
    if (ws2812_energy_avg < 200)        level = 0;
    else if (ws2812_energy_avg < 500)   level = 1;
    else if (ws2812_energy_avg < 1000)  level = 3;
    else if (ws2812_energy_avg < 3000)  level = 5;
    else if (ws2812_energy_avg < 6000)  level = 7;
    else if (ws2812_energy_avg < 12000) level = 9;
    else                                level = WS2812_NUM_LEDS;

    // 3. 填充 RGB 缓冲
    for (i = 0; i < WS2812_NUM_LEDS; i++) {
        if (i < level) {
            u16 hue = 120 - (i * 120 / WS2812_NUM_LEDS);
            ws2812_hsl2rgb(hue, 100, 40, &r, &g, &b);
            ws2812_rgb_buf[i * 3 + 0] = g;
            ws2812_rgb_buf[i * 3 + 1] = r;
            ws2812_rgb_buf[i * 3 + 2] = b;
        } else {
            ws2812_rgb_buf[i * 3 + 0] = 0;
            ws2812_rgb_buf[i * 3 + 1] = 0;
            ws2812_rgb_buf[i * 3 + 2] = 0;
        }
    }

    // 4. 发送到灯带
    ws2812_update(ws2812_rgb_buf);
}

#endif // RGB_SERIAL_EN
