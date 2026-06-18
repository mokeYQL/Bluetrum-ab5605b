/**
 * UART1 透传回环 (PA6 RX -> PA7 TX)
 * 初始化完全照搬 bsp_uart.c:uart1_g1_init(), 去掉 ISR 注册, 用轮询
 * 不在热循环中调 printf (避免与BT中断冲突导致复位)
 */

#include "func_mp3_uart_upd.h"
#include "include.h"

#define MP3_UPD_BAUD 115200

// 统计计数
static volatile u32 rx_total_cnt = 0;
static volatile u32 rx_byte_cnt  = 0;
static u32          stat_tick    = 0;

// ===================== UART1 发送 =====================

static void _uart1_putchar(u8 ch)
{
    while (!(UART1CON & BIT(8)))
        ;
    UART1DATA = ch;
}

// ===================== 初始化 =====================
// 参考 SDK func_uart_upd.c:uart_upd_init() + bsp_uart.c:uart1_g1_init()
// UART1CON = BIT(5)|BIT(7)|BIT(4)|BIT(0)
//   BIT(5)=时钟选uart_inc(13MHz), BIT(7)=16x过采样
//   BIT(4)=RX硬件使能, BIT(0)=UART使能
// CLKGAT0 |= BIT(21)  ← UART1时钟门控 (官方SDK confirmed)

static void mp3_upd_uart_init(void)
{
    u32 baud_cfg;

    // --- GPIO & Mapping (照搬 uart1_g1_init) ---
    GPIOAFEN |= BIT(6) | BIT(7);
    GPIOADIR |= BIT(6);  // PA6 input (RX)
    GPIOADIR &= ~BIT(7); // PA7 output (TX)
    GPIOAPU |= BIT(6) | BIT(7);
    GPIOADE |= BIT(6) | BIT(7);

    FUNCMCON0 |= (0xF << 28) | (0xF << 24);
    FUNCMCON0 |= (0x1 << 28) | (0x1 << 24); // G1 mapping

    // --- UART1 时钟 & 波特率 (照搬 uart_upd_init) ---
    CLKGAT0 |= BIT(21); // UART1 时钟门控 (官方SDK confirmed)
    CLKCON1 |= BIT(14); // uart_inc <- x26m_clkdiv2
    baud_cfg  = ((26000000 / 2 + MP3_UPD_BAUD / 2) / MP3_UPD_BAUD) - 1;
    UART1BAUD = (baud_cfg << 16) | baud_cfg;

    // --- 使能 UART: BIT(5)=时钟源, BIT(7)=16x, BIT(4)=RX硬件, BIT(0)=EN ---
    UART1CON = BIT(5) | BIT(7) | BIT(4) | BIT(0);

    // --- 清空初始化过程中可能产生的 RX 残留数据 ---
    while (UART1CON & BIT(9)) {
        (void)UART1DATA;
        UART1CPND |= BIT(9);
    }
}

// ===================== 公共接口 =====================

void mp3_uart_update_init(void)
{
    mp3_upd_uart_init();
    rx_total_cnt = 0;
    rx_byte_cnt  = 0;
    stat_tick    = tick_get();
    printf("[MP3_UPD] UART1 PA7(TX)/PA6(RX) @ %d, poll mode\n", MP3_UPD_BAUD);
}

void mp3_uart_update_process(void)
{
    static u32 pkt_tick = 0;
    static bool pkt_active = false;

    while (UART1CON & BIT(9)) {
        u8 ch = (u8)UART1DATA;
        UART1CPND |= BIT(9);

        _uart1_putchar(ch);         // PA7 回发

        printf("%02X ", ch);        // PB3 实时打印 HEX+空格

        pkt_active = true;
        pkt_tick = tick_get();
        rx_byte_cnt++;
        rx_total_cnt++;
        WDT_CLR();
    }

    // 50ms 无数据 → 换行 (包边界)
    if (pkt_active && tick_check_expire(pkt_tick, 50)) {
        printf("\n");
        pkt_active = false;
    }
}
