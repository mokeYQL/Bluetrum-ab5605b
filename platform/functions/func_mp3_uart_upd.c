/**
 * UART1 透传回环 + MP3升级协议
 *
 * 正常模式: PA6 RX → PA7 TX 回发 + PB3 HEX日志
 * 升级模式: 收到 [0x55][0xAA] 帧头后进入协议处理
 *
 * 协议帧: [0x55][0xAA][CMD 1B][PAYLOAD N]
 *   CMD 0x01 ERASE : 擦除 POWERON_MP3 占用的5个扇区(20KB)
 *   CMD 0x02 DATA  : [seq_lo][seq_hi][256B data] → 写 Flash
 *   CMD 0x03 FINISH: 打印完成信息
 *
 * 响应: [0xAA][0x55][CMD_ECHO][RESULT or SEQ]
 *   ERASE: [0xAA][0x55][0x01][0x00]
 *   DATA:  [0xAA][0x55][0x02][seq_lo][seq_hi]
 *   FINISH:[0xAA][0x55][0x03][0x00]
 */

#include "func_mp3_uart_upd.h"
#include "include.h"

#define MP3_UPD_FLASH_ADDR  0x11002000 // POWERON_MP3 数据地址
#define MP3_UPD_MAX_SIZE    20480      // 20KB = 5个扇区
#define MP3_UPD_CHUNK_SIZE  256        // 每包数据量
#define MP3_UPD_SECTOR_SIZE 4096
#define MP3_UPD_BAUD        115200

#define FRAME_HDR0          0x55
#define FRAME_HDR1          0xAA
#define CMD_ERASE           0x01
#define CMD_DATA            0x02
#define CMD_FINISH          0x03

extern void os_spiflash_erase(u32 addr);
extern void os_spiflash_program(void *buf, u32 addr, uint len);

// ===================== 协议状态机 =====================
enum
{
    UPD_IDLE,
    UPD_GOT_55,
    UPD_GOT_AA,
    UPD_GOT_CMD,
    UPD_RX_PAYLOAD
};
static u8  upd_state;
static u8  upd_cmd;
static u16 upd_need;
static u16 upd_idx;
static u8  upd_payload[260]; // seq 2B + data 256B = 258B

// ===================== UART1 发送 =====================

static void _uart1_putchar(u8 ch)
{
    while (!(UART1CON & BIT(8)))
        ;
    UART1DATA = ch;
}

static void _uart1_puts(u8 *buf, u16 len)
{
    for (u16 i = 0; i < len; i++)
        _uart1_putchar(buf[i]);
}

static void upd_ack(u8 cmd, u8 *extra, u8 extra_len)
{
    u8 ack[8] = {0xAA, 0x55, cmd};
    u8 total_len;
    ack[3] = 0x00;
    if (extra && extra_len > 0 && extra_len <= 4) {
        for (u8 i = 0; i < extra_len; i++)
            ack[3 + i] = extra[i];
        total_len = 3 + extra_len;
    } else {
        total_len = 4; // 至少发送4字节 (AA 55 CMD 00)
    }
    _uart1_puts(ack, total_len);
}

// ===================== 升级命令处理 =====================

static void upd_do_erase(void)
{
    printf("[MP3_UPD] Erasing 5 sectors @ 0x%08X...\n", (unsigned int)MP3_UPD_FLASH_ADDR);
    // for (u32 addr = MP3_UPD_FLASH_ADDR;
    //      addr < MP3_UPD_FLASH_ADDR + MP3_UPD_MAX_SIZE;
    //      addr += MP3_UPD_SECTOR_SIZE) {
    //     os_spiflash_erase(addr);
    //     WDT_CLR();
    // }
    os_spiflash_erase(MP3_UPD_FLASH_ADDR);
    delay_ms(1000);
    // printf("[MP3_UPD] Erase done, ready for data\n");
    // upd_ack(CMD_ERASE, NULL, 0);
}

static void upd_do_data(void)
{
    u16 seq    = upd_payload[0] | ((u16)upd_payload[1] << 8);
    u32 offset = (u32)seq * MP3_UPD_CHUNK_SIZE;

    if (offset + MP3_UPD_CHUNK_SIZE <= MP3_UPD_MAX_SIZE) {
        os_spiflash_program(&upd_payload[2],
                            MP3_UPD_FLASH_ADDR + offset,
                            MP3_UPD_CHUNK_SIZE);
        u8 extra[2] = {upd_payload[0], upd_payload[1]};
        upd_ack(CMD_DATA, extra, 2);
    } else {
        printf("[MP3_UPD] DATA seq=%u out of range!\n", seq);
    }
}

static void upd_do_finish(void)
{
    printf("[MP3_UPD] Upgrade complete! Reboot to take effect.\n");
    upd_ack(CMD_FINISH, NULL, 0);
}

// ===================== 初始化 =====================

static void mp3_upd_uart_init(void)
{
    u32 baud_cfg;

    GPIOAFEN |= BIT(6) | BIT(7);
    GPIOADIR |= BIT(6);  // PA6 input (RX)
    GPIOADIR &= ~BIT(7); // PA7 output (TX)
    GPIOAPU |= BIT(6) | BIT(7);
    GPIOADE |= BIT(6) | BIT(7);

    FUNCMCON0 |= (0xF << 28) | (0xF << 24);
    FUNCMCON0 |= (0x1 << 28) | (0x1 << 24);

    CLKGAT0 |= BIT(21);
    CLKCON1 |= BIT(14);
    baud_cfg  = ((26000000 / 2 + MP3_UPD_BAUD / 2) / MP3_UPD_BAUD) - 1;
    UART1BAUD = (baud_cfg << 16) | baud_cfg;

    UART1CON = BIT(5) | BIT(7) | BIT(4) | BIT(0);

    while (UART1CON & BIT(9)) {
        (void)UART1DATA;
        UART1CPND |= BIT(9);
    }
}

// ===================== 公共接口 =====================

void mp3_uart_update_init(void)
{
    mp3_upd_uart_init();
    upd_state = UPD_IDLE;
    printf("[MP3_UPD] UART1 PA7/PA6 @ %d, loopback + upgrade protocol ready\n", MP3_UPD_BAUD);
    printf("[MP3_UPD] POWERON_MP3 @ 0x%08X, max %u bytes\n",
           (unsigned int)MP3_UPD_FLASH_ADDR, MP3_UPD_MAX_SIZE);
}

void mp3_uart_update_process(void)
{
    static u32  pkt_tick   = 0;
    static bool pkt_active = false;

    while (UART1CON & BIT(9)) {
        u8 ch = (u8)UART1DATA;
        UART1CPND |= BIT(9);
        WDT_CLR();

        switch (ch) {
        case 0x01:
            upd_do_erase();
            break;

        default:
            break;
        }

        // switch (upd_state) {
        // // ====== 状态机: 帧头检测 vs 普通回环 ======

        // switch (upd_state) {
        // // ====== 状态机: 帧头检测 vs 普通回环 ======
        // case UPD_IDLE:
        //         printf("%02X ", ch);
        //         pkt_active = true;
        //         pkt_tick   = tick_get();
        //     }
        //     break;

        // case UPD_GOT_55:
        //     if (ch == FRAME_HDR1) {
        //         upd_state = UPD_GOT_AA;
        //     } else {
        //         // 之前的 0x55 是普通数据, 补发
        //         _uart1_putchar(FRAME_HDR0);
        //         printf("55 ");
        //         if (ch == FRAME_HDR0) {
        //             upd_state = UPD_GOT_55;
        //         } else {
        //             _uart1_putchar(ch);
        //             printf("%02X ", ch);
        //             upd_state  = UPD_IDLE;
        //             pkt_active = true;
        //             pkt_tick   = tick_get();
        //         }
        //     }
        //     break;

        // // ====== 状态机: 协议帧 ======
        // case UPD_GOT_AA:
        //     upd_cmd = ch;
        //     if (ch == CMD_ERASE) {
        //         upd_state = UPD_IDLE;
        //         upd_do_erase();
        //     } else if (ch == CMD_DATA) {
        //         upd_need  = 2 + MP3_UPD_CHUNK_SIZE; // seq 2B + data 256B
        //         upd_idx   = 0;
        //         upd_state = UPD_RX_PAYLOAD;
        //     } else if (ch == CMD_FINISH) {
        //         upd_state = UPD_IDLE;
        //         upd_do_finish();
        //     } else {
        //         // 非法 CMD, 把 0x55/0xAA/CMD 当作普通数据回环
        //         _uart1_putchar(FRAME_HDR0);
        //         _uart1_putchar(FRAME_HDR1);
        //         _uart1_putchar(ch);
        //         printf("55 AA %02X ", ch);
        //         upd_state  = UPD_IDLE;
        //         pkt_active = true;
        //         pkt_tick   = tick_get();
        //     }
        //     break;

        // case UPD_RX_PAYLOAD:
        //     upd_payload[upd_idx++] = ch;
        //     if (upd_idx >= upd_need) {
        //         upd_state = UPD_IDLE;
        //         if (upd_cmd == CMD_DATA) {
        //             upd_do_data();
        //         }
        //     }
        //     break;
        // }
    }

    // 包边界换行(仅非协议模式)
    if (pkt_active && tick_check_expire(pkt_tick, 50)) {
        printf("\n");
        pkt_active = false;
    }
}
