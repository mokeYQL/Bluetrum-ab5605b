#include "include.h"
#include "port_ws2812.h"

#if RGB_WS2812_EN

/*
 * WS2812 SPI 驱动 — 使用 SPI1 G4 映射
 *
 * 原理：每个 WS2812 bit 用 3 个 SPI bit 编码
 *   WS2812 Bit 0 → SPI 100 (短高长低: H≈420ns, L≈840ns)
 *   WS2812 Bit 1 → SPI 110 (长高短低: H≈840ns, L≈420ns)
 *
 * SPI1 G4: CLK=PE6(悬空), DI=PE5(悬空), DO=PE7→WS2812 DIN
 *
 * SPI 波特率 = sysclk / (SPI1BAUD + 1)
 * 120MHz / 50 = 2.4MHz → 每 SPI bit ≈ 417ns
 * 3 SPI bit = 1.25us = WS2812 标准周期 ✓
 */

// SPI 编码缓冲（1 WS2812 byte → 3 SPI bytes，12 LED × 3色 × 3 = 108 bytes）
static u8 ws2812_spi_buf[WS2812_BUF_SIZE];

// RGB 原始数据缓冲（用户填充，GRB 顺序）
static u8 ws2812_rgb_buf[WS2812_NUM_LEDS * 3];

// UART命令覆盖模式: 0xFF=无覆盖, 0x00=关, 0x01=常亮, 0x02=呼吸, 0x03=音乐律动
u8 ws2812_override_mode = 0xFF;

/* 将 1 字节 WS2812 数据编码为 3 字节 SPI 数据
 * 每 bit WS2812 → 3 bit SPI:  0→100,  1→110
 * 8 bit → 24 bit SPI → 3 字节（大端，SPI 先发高位）
 */
static void ws2812_encode_byte(u8 data, u8 *out)
{
    u8 i;
    u32 spi_val = 0;

    for (i = 0; i < 8; i++)
    {
        spi_val <<= 3;
        spi_val |= (data & BIT(7 - i)) ? 0x06 : 0x04; // 1→110, 0→100
    }

    out[0] = (spi_val >> 16) & 0xFF;
    out[1] = (spi_val >> 8) & 0xFF;
    out[2] = spi_val & 0xFF;
}

/* 初始化 SPI1 G4: PE7(DO)→WS2812 DIN, PE6(CLK)悬空, PE5(DI)保留为普通IO */
void ws2812_init(void)
{
    // SPI1 G4: CLK=PE6, DO=PE7（DI=PE5 不初始化，留作普通IO）
    GPIOEDE  |= BIT(6) | BIT(7);             // 开启数字功能
    GPIOEFEN |= BIT(6) | BIT(7);             // 开启功能复用模式（SPI）
    GPIOEDIR &= ~BIT(6);                      // PE6(CLK) 输出
    GPIOEDIR &= ~BIT(7);                      // PE7(DO) 输出

    FUNCMCON1 = (0x0F << 12); // 清除 SPI1 映射
    FUNCMCON1 = SPI1MAP_G4;   // 设置 G4 映射

    // SPI1 波特率：120MHz / 50 = 2.4MHz
    // 每 SPI bit = 417ns, 3 SPI bit = 1.25us = WS2812 标准周期
    SPI1BAUD = 49; // 120MHz / (49+1) = 2.4MHz

    // 使能 SPI1 + 2线模式（仅 DO 输出，不需要 DI）
    SPI1CON = 0x01 | (1 << 2);

    // 清空缓冲
    memset(ws2812_spi_buf, 0, sizeof(ws2812_spi_buf));
    memset(ws2812_rgb_buf, 0, sizeof(ws2812_rgb_buf));

    printf("[WS2812] SPI1 G4 init ok (PE7→DIN, 2.4MHz)\n");
}

/* 将 RGB 缓冲编码为 SPI 缓冲并通过 DMA 发送 */
AT(.com_text.rgb)
void ws2812_update(void)
{
    u8 i;

    // 编码：每个 LED 的 G/R/B 各 1 字节 → 各 3 字节 SPI 数据
    for (i = 0; i < WS2812_NUM_LEDS; i++)
    {
        ws2812_encode_byte(ws2812_rgb_buf[i * 3 + 0], &ws2812_spi_buf[i * 9 + 0]); // G
        ws2812_encode_byte(ws2812_rgb_buf[i * 3 + 1], &ws2812_spi_buf[i * 9 + 3]); // R
        ws2812_encode_byte(ws2812_rgb_buf[i * 3 + 2], &ws2812_spi_buf[i * 9 + 6]); // B
    }

    // DMA 发送（SPI 硬件自动产生时序，无需关中断）
    SPI1DMAADR = DMA_ADR(ws2812_spi_buf);
    SPI1DMACNT = sizeof(ws2812_spi_buf);
}

/* 设置单个 LED 的 RGB 颜色 */
void ws2812_set_color(u8 led_idx, u8 r, u8 g, u8 b)
{
    if (led_idx >= WS2812_NUM_LEDS)
        return;
    ws2812_rgb_buf[led_idx * 3 + 0] = g; // WS2812 GRB 顺序
    ws2812_rgb_buf[led_idx * 3 + 1] = r;
    ws2812_rgb_buf[led_idx * 3 + 2] = b;
}

/* 在 func_bt_process() 主循环中调用 */
void ws2812_flush(void)
{
    u8 i;

    /* ---- UART命令覆盖模式 ---- */
    if (ws2812_override_mode != 0xFF) {
        if (ws2812_override_mode == WS2812_MODE_OFF) {
            // 全灭
            for (i = 0; i < WS2812_NUM_LEDS; i++)
                ws2812_set_color(i, 0, 0, 0);
            ws2812_update();
        } else if (ws2812_override_mode == WS2812_MODE_ON) {
            // 常亮蓝色
            for (i = 0; i < WS2812_NUM_LEDS; i++)
                ws2812_set_color(i, 0, 0, 64);
            ws2812_update();
        } else if (ws2812_override_mode == WS2812_MODE_BREATH) {
            // 呼吸效果: 用整数三角波查表替代浮点sin
            static u8  breath_phase = 0;
            static u32 breath_tick  = 0;
            if (tick_check_expire(breath_tick, 20)) {
                breath_tick = tick_get();
                breath_phase++;
            }
            // 0~127递增 + 128~255递减, 产生平滑呼吸波形
            u8 b;
            if (breath_phase < 128)
                b = breath_phase * 2;
            else
                b = (255 - breath_phase) * 2;
            for (i = 0; i < WS2812_NUM_LEDS; i++)
                ws2812_set_color(i, 0, 0, b);
            ws2812_update();
        } else {
            // WS2812_MODE_REACT = 恢复自动模式
            ws2812_override_mode = 0xFF;
        }
        return;
    }

    /* ---- 原有自动模式 (BT/AUX音乐律动) ---- */
    static u32 last_tick = 0;
    u16 energy;
    u8 level, r, g, b;

    if (!tick_check_expire(last_tick, 80))
        return; // 每80ms刷新
    last_tick = tick_get();

#if FUNC_AUX_EN
    if (func_cb.sta == FUNC_AUX)
    {
        /* ===== AUX 模式：音乐律动 ===== */
        energy = dac_pcm_pow_calc();

        {
            static u16 log_cnt = 0;
            if (++log_cnt >= 30)
            { // 每30次≈2.4秒打印一次
                log_cnt = 0;
                printf("[WS2812] AUX energy=%d\n", energy);
            }
        }

        // 能量 → 灯数映射（40灯，8级均匀覆盖）
        if (energy < 300)
            level = 0;
        else if (energy < 1000)
            level = 5;
        else if (energy < 2500)
            level = 10;
        else if (energy < 5000)
            level = 15;
        else if (energy < 8000)
            level = 20;
        else if (energy < 12000)
            level = 25;
        else if (energy < 18000)
            level = 30;
        else if (energy < 25000)
            level = 35;
        else
            level = WS2812_NUM_LEDS;

        // 填充灯带
        for (i = 0; i < WS2812_NUM_LEDS; i++)
        {
            if (i < level)
            {
                u16 pos = (i + 1) * 256 / WS2812_NUM_LEDS;
                if (pos < 85)
                {
                    r = 0;
                    g = pos * 3;
                    b = 0;
                }
                else if (pos < 170)
                {
                    r = (pos - 85) * 3;
                    g = 255;
                    b = 0;
                }
                else
                {
                    r = 255;
                    g = 255 - (pos - 170) * 3;
                    b = 0;
                }
                ws2812_set_color(i, r, g, b);
            }
            else
            {
                ws2812_set_color(i, 0, 0, 0);
            }
        }

        ws2812_update();
        return;
    }
#endif

#if FUNC_BT_EN
    if ((func_cb.sta == FUNC_BT) && (bt_get_status() >= BT_STA_CONNECTED))
    {
        /* ===== BT 模式：从中心向两边扩散 =====
         * 中心 LED 19/20，左右对称扩散
         * level: 亮灯层数（0~20），20=全亮
         * 颜色随 energy 渐变：蓝→绿→黄→红→紫
         */
        s16  left_idx, right_idx;
        u16 hue;
        u8  rr = 0, gg = 0, bb = 0;

        energy = dac_pcm_pow_calc();

        // 能量 → 扩散层数（0~20 层，每层左右各 2 颗 = 共 40 颗）
        // 阈值整体偏高，小音量只亮中心几颗
        if      (energy < 300)    level = 0;
        else if (energy < 4000)   level = 2;   // 中心4颗
        else if (energy < 8000)   level = 5;   // 10颗
        else if (energy < 12000)  level = 8;   // 16颗
        else if (energy < 18000)  level = 11;  // 22颗
        else if (energy < 25000)  level = 14;  // 28颗
        else if (energy < 35000)  level = 17;  // 34颗
        else                      level = 20;  // 全亮

        // 能量 → 色相（0-255: 蓝→绿→黄→红→紫）
        if      (energy < 1200)   hue = 32;
        else if (energy < 3000)   hue = 64;
        else if (energy < 6000)   hue = 96;
        else if (energy < 10000)  hue = 128;
        else if (energy < 16000)  hue = 160;
        else if (energy < 25000)  hue = 192;
        else if (energy < 38000)  hue = 224;
        else                      hue = 255;

        // 色相→RGB
        if (hue < 85)
        {
            rr = 0;
            gg = hue * 3;
            bb = 255 - hue * 3;
        }
        else if (hue < 170)
        {
            rr = (hue - 85) * 3;
            gg = 255;
            bb = 0;
        }
        else
        {
            rr = 255;
            gg = (255 - hue) * 3;
            bb = (hue - 170) * 3;
        }

        // 先全灭
        for (i = 0; i < WS2812_NUM_LEDS; i++)
            ws2812_set_color(i, 0, 0, 0);

        // 从中心向两边点亮（LED编号 0~39，中心=19/20）
        for (i = 0; i < level; i++)
        {
            left_idx  = 19 - i;
            right_idx = 20 + i;
            if (left_idx >= 0)
                ws2812_set_color((u8)left_idx, rr, gg, bb);
            if (right_idx < WS2812_NUM_LEDS)
                ws2812_set_color((u8)right_idx, rr, gg, bb);
        }

        ws2812_update();
        return;
    }
#endif

    // /* ===== 非播放状态（空闲等）：淡蓝常亮（柔和） ===== */
    // for (i = 0; i < WS2812_NUM_LEDS; i++) {
    //     ws2812_set_color(i, 0, 0, 5);  // 微蓝
    // }
    // ws2812_update();
}

#endif // RGB_WS2812_EN
