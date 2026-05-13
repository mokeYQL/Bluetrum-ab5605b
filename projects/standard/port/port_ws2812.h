#ifndef __PORT_WS2812_H__
#define __PORT_WS2812_H__

#define WS2812_NUM_LEDS 40 // 灯带灯珠数量
#define WS2812_BUF_SIZE (WS2812_NUM_LEDS * 3)

void ws2812_init(void);
void ws2812_flush(void); // 在主循环调用：计算音频能量 + 更新灯带

#endif // __PORT_WS2812_H__
