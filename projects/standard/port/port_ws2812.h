#ifndef __PORT_WS2812_H__
#define __PORT_WS2812_H__

#define WS2812_NUM_LEDS 40                    // 灯带灯珠数量
#define WS2812_BUF_SIZE (WS2812_NUM_LEDS * 9) // 每字节GRB=8bit,每bit编码为3 SPI bit = 24 SPI bit = 3 SPI bytes

#define WS2812_MODE_OFF    0x00  // 关闭
#define WS2812_MODE_ON     0x01  // 常亮
#define WS2812_MODE_BREATH 0x02  // 呼吸
#define WS2812_MODE_REACT  0x03  // 音乐律动(恢复自动)

extern u8 ws2812_override_mode;    // 0xFF=无覆盖, 其他=UART命令覆盖模式

void ws2812_init(void);
void ws2812_update(void);
void ws2812_set_color(u8 led_idx, u8 r, u8 g, u8 b);

#endif // __PORT_WS2812_H__
