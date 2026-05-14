#ifndef __PORT_WS2812_H__
#define __PORT_WS2812_H__

#define WS2812_NUM_LEDS 40                    // 灯带灯珠数量
#define WS2812_BUF_SIZE (WS2812_NUM_LEDS * 9) // 每字节GRB=8bit,每bit编码为3 SPI bit = 24 SPI bit = 3 SPI bytes

void ws2812_init(void);
void ws2812_flush(void);
void ws2812_set_color(u8 led_idx, u8 r, u8 g, u8 b);

#endif // __PORT_WS2812_H__
