/**
 * UART1 多语音批量升级协议 (TOC + MP3)
 *
 * 升级 voc 分区: NOR Flash 0x60000, 大小 0x1B000 (108KB)
 * 数据结构: [80B TOC头部] + [MP3数据...]
 *
 * 协议帧: [0x55][0xAA][CMD 1B][PAYLOAD N]
 *   CMD 0x01 ERASE : 擦除全部 voc 分区扇区
 *   CMD 0x02 DATA  : [seq 2B LE][256B data] → 写 Flash
 *   CMD 0x03 FINISH: 完成
 *
 * 响应: [0xAA][0x55][CMD_ECHO][RESULT or SEQ]
 *
 * 注意: printf 与 ACK 共用 UART1 TX, 仅在关键节点打印, 避免干扰协议解析
 */

#include "func_mp3_uart_upd.h"
#include "include.h"

// ---- 分区配置 (与 app.xm setuserbin 保持一致) ----
#define MP3_UPD_FLASH_ADDR   0x60000    // voc.bin NOR Flash 偏移
#define MP3_UPD_PART_SIZE    0x1B000    // 108 KB (110592 bytes)
#define MP3_UPD_CHUNK_SIZE   256        // 每包数据量
#define MP3_UPD_SECTOR_SIZE  4096
#define MP3_UPD_BAUD         115200

// ---- 协议常量 ----
#define FRAME_HDR0           0x55
#define FRAME_HDR1           0xAA
#define CMD_ERASE            0x01
#define CMD_DATA             0x02
#define CMD_FINISH           0x03

extern void os_spiflash_erase(u32 addr);
extern void os_spiflash_program(void *buf, u32 addr, uint len);

// ===================== 协议状态机 =====================
enum {
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
static u8  upd_payload[258];  // seq 2B + data 256B max

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
    u8 ack[8] = {0xAA, 0x55, cmd, 0x00};
    u8 total_len = 4;
    if (extra && extra_len > 0 && extra_len <= 4) {
        for (u8 i = 0; i < extra_len; i++)
            ack[3 + i] = extra[i];
        total_len = 3 + extra_len;
    }
    _uart1_puts(ack, total_len);
}

// ===================== 升级命令处理 =====================
static void upd_do_erase(void)
{
    u32 sectors = MP3_UPD_PART_SIZE / MP3_UPD_SECTOR_SIZE;
    printf("[MP3_UPD] Erase %u sectors...\n", (unsigned int)sectors);

    for (u32 addr = MP3_UPD_FLASH_ADDR;
         addr < MP3_UPD_FLASH_ADDR + MP3_UPD_PART_SIZE;
         addr += MP3_UPD_SECTOR_SIZE) {
        os_spiflash_erase(addr);
        WDT_CLR();
    }
    printf("[MP3_UPD] Erase done\n");
    upd_ack(CMD_ERASE, NULL, 0);
    delay_ms(200);
}

static void upd_do_data(void)
{
    u16 seq    = upd_payload[0] | ((u16)upd_payload[1] << 8);
    u32 offset = (u32)seq * MP3_UPD_CHUNK_SIZE;

    if (offset + MP3_UPD_CHUNK_SIZE <= MP3_UPD_PART_SIZE) {
        WDT_CLR();
        os_spiflash_program(&upd_payload[2],
                            MP3_UPD_FLASH_ADDR + offset,
                            MP3_UPD_CHUNK_SIZE);
        WDT_CLR();
        u8 extra[2] = {upd_payload[0], upd_payload[1]};
        upd_ack(CMD_DATA, extra, 2);
        // 每 50 帧打印一次进度, 减少对 UART1 ACK 的干扰
        if ((seq % 50) == 0) {
            printf("[MP3_UPD] WR %u @0x%06X\n", seq,
                   (unsigned int)(MP3_UPD_FLASH_ADDR + offset));
        }
    } else {
        printf("[MP3_UPD] DATA %u out of range!\n", seq);
    }
}

static void upd_do_finish(void)
{
    printf("[MP3_UPD] Finish, update complete\n");
    upd_ack(CMD_FINISH, NULL, 0);
    delay_ms(200);  // 等 ACK 完全发出后再复位
    WDT_RST();
}

// ===================== 初始化 =====================
static void mp3_upd_uart_init(void)
{
    u32 baud_cfg;

    GPIOAFEN |= BIT(6) | BIT(7);
    GPIOADIR |= BIT(6);   // PA6 input (RX)
    GPIOADIR &= ~BIT(7);  // PA7 output (TX)
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
    printf("[MP3_UPD] UART1 @ %d, voc 0x%06X~0x%06X (%uKB)\n",
           MP3_UPD_BAUD,
           (unsigned int)MP3_UPD_FLASH_ADDR,
           (unsigned int)(MP3_UPD_FLASH_ADDR + MP3_UPD_PART_SIZE),
           (unsigned int)(MP3_UPD_PART_SIZE / 1024));
}

void mp3_uart_update_process(void)
{
    while (UART1CON & BIT(9)) {
        u8 ch = (u8)UART1DATA;
        UART1CPND |= BIT(9);
        WDT_CLR();

        switch (upd_state) {

        case UPD_IDLE:
            if (ch == FRAME_HDR0) {
                upd_state = UPD_GOT_55;
            }
            break;

        case UPD_GOT_55:
            if (ch == FRAME_HDR1) {
                upd_state = UPD_GOT_AA;
            } else if (ch == FRAME_HDR0) {
                upd_state = UPD_GOT_55;
            } else {
                upd_state = UPD_IDLE;
            }
            break;

        case UPD_GOT_AA:
            upd_cmd   = ch;
            upd_state = UPD_IDLE;

            if (ch == CMD_ERASE) {
                upd_do_erase();
            } else if (ch == CMD_DATA) {
                upd_need  = 2 + MP3_UPD_CHUNK_SIZE;  // seq 2B + 256B
                upd_idx   = 0;
                upd_state = UPD_RX_PAYLOAD;
            } else if (ch == CMD_FINISH) {
                upd_do_finish();
            }
            break;

        case UPD_RX_PAYLOAD:
            upd_payload[upd_idx++] = ch;
            if (upd_idx >= upd_need) {
                upd_state = UPD_IDLE;
                if (upd_cmd == CMD_DATA) {
                    upd_do_data();
                }
            }
            break;
        }
    }
}
