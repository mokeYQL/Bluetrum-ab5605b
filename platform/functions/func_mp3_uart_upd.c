/**
 * UART1 多语音批量升级协议 (TOC + MP3) v2
 *
 * 升级 voc 分区: NOR Flash 0x60000, 大小 0x1B000 (108KB)
 * 数据结构: [80B TOC头部] + [MP3数据...]
 *
 * 协议帧: [0x55][0xAA][CMD 1B][PAYLOAD N]
 *   CMD 0x01 ERASE : 擦除全部 voc 分区扇区
 *   CMD 0x02 DATA  : [seq 2B LE][xor 1B][256B data] → 写 Flash
 *   CMD 0x03 FINISH: [crc16_lo][crc_hi] → 读回Flash校验CRC16
 *
 * 响应: [0xAA][0x55][CMD_ECHO][RESULT]
 *   ERASE:  [AA 55 01 00]
 *   DATA:   [AA 55 02 seq_lo seq_hi]
 *   FINISH: [AA 55 03 result]  result=0x00成功, 0x01 CRC错误
 *
 * 接收方式: UART1 RX 中断 → FIFO → 主循环状态机处理
 */

#include "func_mp3_uart_upd.h"
#include "fifo.h"
#include "include.h"

// ---- 分区配置 ----
#define MP3_UPD_FLASH_ADDR  0x60000
#define MP3_UPD_PART_SIZE   0x1B000 // 108 KB
#define MP3_UPD_CHUNK_SIZE  256
#define MP3_UPD_SECTOR_SIZE 4096
#define MP3_UPD_BAUD        115200

// ---- 协议常量 ----
#define FRAME_HDR0          0x55
#define FRAME_HDR1          0xAA
#define CMD_ERASE           0x01
#define CMD_DATA            0x02
#define CMD_FINISH          0x03

extern void os_spiflash_erase(u32 addr);
extern void os_spiflash_program(void *buf, u32 addr, uint len);
extern uint os_spiflash_read(void *buf, u32 addr, uint len);

// ===================== CRC16-CCITT =====================
static u16 crc16_ccitt(u16 crc, u8 *data, u32 len)
{
    while (len--) {
        crc ^= (u16)(*data++) << 8;
        for (u8 i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// ===================== 协议状态机 =====================
enum
{
    UPD_IDLE,
    UPD_GOT_55,
    UPD_GOT_AA,
    UPD_RX_PAYLOAD
};
static u8  upd_state;
static u8  upd_cmd;
static u16 upd_need;
static u16 upd_idx;
static u8  upd_payload[259]; // seq 2B + xor 1B + data 256B max
static u16 upd_max_seq;      // 记录最大 seq, 用于 FINISH 时计算读回大小

// ---- UART1 RX FIFO (ISR → 主循环) ----
static fifo_t g_upd_fifo;

// ===================== UART1 RX 中断 =====================
AT(.com_text.isr)
void mp3_upd_rx_isr(void)
{
    if (UART1CON & BIT(9)) {
        u8 ch = (u8)UART1DATA;
        UART1CPND |= BIT(9);
        fifo_put(&g_upd_fifo, ch);
    }
}

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
    u8 ack[8]    = {0xAA, 0x55, cmd, 0x00};
    u8 total_len = 4;
    if (extra && extra_len > 0 && extra_len <= 4) {
        for (u8 i = 0; i < extra_len; i++)
            ack[3 + i] = extra[i];
        total_len = 3 + extra_len;
    }
    _uart1_puts(ack, total_len);
}

// ===================== XOR 校验 =====================
static u8 xor_checksum(u8 *data, u32 len)
{
    u8 xor_val = 0;
    while (len--)
        xor_val ^= *data++;
    return xor_val;
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
    u16 seq      = upd_payload[0] | ((u16)upd_payload[1] << 8);
    u8  xor_recv = upd_payload[2];                                    // 接收到的 XOR 校验值
    u8  xor_calc = xor_checksum(&upd_payload[3], MP3_UPD_CHUNK_SIZE); // 本地计算

    if (xor_recv != xor_calc) {
        printf("[MP3_UPD] DATA seq=%u XOR mismatch!\n", seq);
        // 不写 Flash, 不回 ACK, Python 端会超时重试
        return;
    }

    u32 offset = (u32)seq * MP3_UPD_CHUNK_SIZE;
    if (offset + MP3_UPD_CHUNK_SIZE <= MP3_UPD_PART_SIZE) {
        WDT_CLR();
        os_spiflash_program(&upd_payload[3],
                            MP3_UPD_FLASH_ADDR + offset,
                            MP3_UPD_CHUNK_SIZE);
        WDT_CLR();

        if (seq > upd_max_seq)
            upd_max_seq = seq;

        u8 extra[2] = {upd_payload[0], upd_payload[1]};
        upd_ack(CMD_DATA, extra, 2);

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
    u16 pc_crc     = upd_payload[0] | ((u16)upd_payload[1] << 8);
    u32 total_size = ((u32)(upd_max_seq + 1)) * MP3_UPD_CHUNK_SIZE;

    printf("[MP3_UPD] Verify CRC16, read %u bytes...\n", (unsigned int)total_size);

    // 读回 Flash 数据计算本地 CRC16
    u16 local_crc = 0;
    u8  buf[256];

    for (u32 offset = 0; offset < total_size; offset += 256) {
        u32 read_len = 256;
        if (offset + read_len > total_size)
            read_len = total_size - offset;
        os_spiflash_read(buf, MP3_UPD_FLASH_ADDR + offset, read_len);
        WDT_CLR();
        local_crc = crc16_ccitt(local_crc, buf, read_len);
    }

    printf("[MP3_UPD] CRC: PC=0x%04X, Local=0x%04X\n", pc_crc, local_crc);

    if (pc_crc == local_crc) {
        printf("[MP3_UPD] CRC OK! Update complete\n");
        upd_ack(CMD_FINISH, NULL, 0); // result=0x00 = 成功
        delay_ms(200);
        WDT_RST(); // 复位生效
    } else {
        printf("[MP3_UPD] CRC FAIL!\n");
        u8 result = 0x01; // result=0x01 = CRC 不匹配
        upd_ack(CMD_FINISH, &result, 1);
    }
    upd_state = UPD_IDLE;
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
    UART1CON |= BIT(2); // 使能 RX 中断

    while (UART1CON & BIT(9)) {
        (void)UART1DATA;
        UART1CPND |= BIT(9);
    }

    fifo_init(&g_upd_fifo);
    sys_irq_init(IRQ_UART_VECTOR, 0, mp3_upd_rx_isr);
}

// ===================== 公共接口 =====================
void mp3_uart_update_init(void)
{
    mp3_upd_uart_init();
    upd_state   = UPD_IDLE;
    upd_max_seq = 0;
    printf("[MP3_UPD] UART1 @ %d, voc 0x%06X~0x%06X (%uKB)\n",
           MP3_UPD_BAUD,
           (unsigned int)MP3_UPD_FLASH_ADDR,
           (unsigned int)(MP3_UPD_FLASH_ADDR + MP3_UPD_PART_SIZE),
           (unsigned int)(MP3_UPD_PART_SIZE / 1024));
}

void mp3_uart_update_process(void)
{
    while (fifo_avail(&g_upd_fifo)) {
        u8 ch;
        fifo_get(&g_upd_fifo, &ch);
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
                upd_need  = 2 + 1 + MP3_UPD_CHUNK_SIZE; // seq 2B + xor 1B + data
                upd_idx   = 0;
                upd_state = UPD_RX_PAYLOAD;
            } else if (ch == CMD_FINISH) {
                upd_need  = 2; // CRC16 2B
                upd_idx   = 0;
                upd_state = UPD_RX_PAYLOAD;
            }
            break;

        case UPD_RX_PAYLOAD:
            upd_payload[upd_idx++] = ch;
            if (upd_idx >= upd_need) {
                upd_state = UPD_IDLE;
                if (upd_cmd == CMD_DATA) {
                    upd_do_data();
                } else if (upd_cmd == CMD_FINISH) {
                    upd_do_finish();
                }
            }
            break;
        }
    }
}
