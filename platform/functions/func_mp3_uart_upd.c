/**
 * UART1 多语音批量升级协议 (TOC + MP3) v4
 *
 * 升级 voc 分区: NOR Flash 0x60000, 大小 0x1B000 (108KB)
 * 数据结构: [80B TOC头部] + [MP3数据...]
 *
 * 协议帧格式 (0xA5): [0xA5][命令][LEN_H][LEN_L][数据段][FCS]
 *   命令 0x01~0x08 = 控制命令
 *   命令 0x09 ERASE  : [crc16_lo][crc_hi] → 擦除分区 + 保存CRC
 *   命令 0x0A DATA   : [seq 2B LE][xor 1B][256B data] → 写 Flash
 *   命令 0x0B FINISH : (无数据) → 读回Flash校验CRC16 (使用ERASE时收到的CRC)
 *
 * FCS = 命令 ^ LEN_H ^ LEN_L ^ DATA[0..N-1]
 *
 * 接收方式: UART1 RX 中断 → FIFO → 主循环状态机处理
 */
#include "include.h"

#include "fifo.h"
#include "func_mp3_uart_upd.h"

#include "port_ws2812.h"

// ---- 分区配置 ----
#define MP3_UPD_FLASH_ADDR  0x60000
#define MP3_UPD_PART_SIZE   0x1B000 // 108 KB
#define MP3_UPD_CHUNK_SIZE  256
#define MP3_UPD_SECTOR_SIZE 4096
#define MP3_UPD_BAUD        115200

// ---- 协议常量 (0xA5 帧格式) ----
#define FRAME_HDR           0xA5
#define CMD_ERASE           0x09 // 擦除 (携带总包CRC)
#define CMD_DATA            0x0A // 写数据
#define CMD_FINISH          0x0B // 完成校验

// ---- 设备信息宏定义 ----
#define DEV_INFO_PN         BT_NAME_DEFAULT // 产品名称
#define DEV_INFO_FN         "Bluetrum"      // 厂家名称
#define DEV_INFO_HV         "1.0.0"         // 硬件版本 (格式 x.x.x)
#define DEV_INFO_SV         "1.0.0"         // 软件版本 (格式 x.x.x)

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

// ===================== XOR 校验 =====================
static u8 xor_checksum(u8 *data, u32 len)
{
    u8 xor_val = 0;
    while (len--)
        xor_val ^= *data++;
    return xor_val;
}

// ===================== FCS 计算 =====================
static u8 calc_fcs(u8 cmd, u8 *data, u16 len)
{
    u8 fcs = cmd;
    fcs ^= (len >> 8) & 0xFF;
    fcs ^= len & 0xFF;
    for (u16 i = 0; i < len; i++)
        fcs ^= data[i];
    return fcs;
}

// ===================== 协议状态机 =====================
enum
{
    UPD_IDLE,       // 等待帧头 0xA5
    UPD_GOT_HDR,    // 收到 0xA5, 等待命令
    UPD_GOT_CMD,    // 收到命令, 等待 LEN_H
    UPD_RX_LEN_L,   // 收到 LEN_H, 等待 LEN_L
    UPD_RX_DATA,    // 接收数据段
    UPD_RX_FCS      // 接收 FCS
};

static u8  upd_state;
static u8  upd_cmd;        // 命令 (0x01~0x0C)
static u16 upd_data_len;   // 数据段长度
static u16 upd_idx;
static u8  upd_payload[259]; // seq 2B + xor 1B + data 256B max
static u8  upd_rx_fcs;       // 接收到的 FCS
static u16 upd_max_seq;      // 记录最大 seq
static u16 upd_flash_crc;    // ERASE 时收到的 CRC16, 供 FINISH 校验

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

// ===================== 发送 0xA5 应答帧 =====================
static void upd_send_frame(u8 cmd, u8 *data, u16 len)
{
    u8 hdr[4];
    u8 fcs = calc_fcs(cmd, data, len);

    hdr[0] = FRAME_HDR;
    hdr[1] = cmd;
    hdr[2] = (len >> 8) & 0xFF;
    hdr[3] = len & 0xFF;

    _uart1_puts(hdr, 4);
    if (len > 0)
        _uart1_puts(data, len);
    _uart1_putchar(fcs);
}

// ===================== 升级命令处理 =====================
static void upd_do_erase(void)
{
    // 保存 CRC16 (小端: lo, hi)
    upd_flash_crc = upd_payload[0] | ((u16)upd_payload[1] << 8);

    u32 sectors = MP3_UPD_PART_SIZE / MP3_UPD_SECTOR_SIZE;
    printf("[MP3_UPD] Erase %u sectors, CRC=0x%04X\n",
           (unsigned int)sectors, upd_flash_crc);

    for (u32 addr = MP3_UPD_FLASH_ADDR;
         addr < MP3_UPD_FLASH_ADDR + MP3_UPD_PART_SIZE;
         addr += MP3_UPD_SECTOR_SIZE) {
        os_spiflash_erase(addr);
        WDT_CLR();
    }
    printf("[MP3_UPD] Erase done\n");

    u8 result = 0x00; // 成功
    upd_send_frame(CMD_ERASE, &result, 1);
    delay_ms(200);
}

static void upd_do_data(void)
{
    u16 seq      = upd_payload[0] | ((u16)upd_payload[1] << 8);
    u8  xor_recv = upd_payload[2];
    u8  xor_calc = xor_checksum(&upd_payload[3], MP3_UPD_CHUNK_SIZE);

    if (xor_recv != xor_calc) {
        printf("[MP3_UPD] DATA seq=%u XOR mismatch!\n", seq);
        // 不写 Flash, 不回 ACK, 上位机超时重试
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

        // 应答: 回显 seq (2B 小端)
        upd_send_frame(CMD_DATA, upd_payload, 2);

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
    u32 total_size = ((u32)(upd_max_seq + 1)) * MP3_UPD_CHUNK_SIZE;

    printf("[MP3_UPD] Verify CRC16, read %u bytes...\n", (unsigned int)total_size);
    printf("[MP3_UPD] Expected CRC=0x%04X\n", upd_flash_crc);

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

    printf("[MP3_UPD] CRC: Expected=0x%04X, Local=0x%04X\n", upd_flash_crc, local_crc);

    u8 result;
    if (upd_flash_crc == local_crc) {
        printf("[MP3_UPD] CRC OK! Update complete\n");
        result = 0x00; // 成功
        upd_send_frame(CMD_FINISH, &result, 1);
        delay_ms(200);
        WDT_RST(); // 复位生效
    } else {
        printf("[MP3_UPD] CRC FAIL!\n");
        result = 0x01; // CRC 不匹配
        upd_send_frame(CMD_FINISH, &result, 1);
    }
}

// ===================== 帧处理 (校验FCS后分发) =====================
static void upd_process_frame(void)
{
    // 校验 FCS
    u8 calc = calc_fcs(upd_cmd, upd_payload, upd_data_len);
    if (calc != upd_rx_fcs) {
        printf("[MP3_UPD] FCS mismatch! calc=0x%02X recv=0x%02X\n", calc, upd_rx_fcs);
        return;
    }

    // ---- 控制命令分发 ----
    if (upd_cmd == 0x01) {
        // 查询设备信息 → 返回JSON
        char json[128];
        snprintf(json, sizeof(json),
                 "{\"pn\":\"%s\",\"fn\":\"%s\",\"hv\":\"%s\",\"sv\":\"%s\"}",
                 DEV_INFO_PN, DEV_INFO_FN, DEV_INFO_HV, DEV_INFO_SV);
        upd_send_frame(0x01, (u8 *)json, strlen(json));
        printf("[UART_CTL] Query info: %s\n", json);
        return;
    }
    if (upd_cmd == 0x02) {
        // 系统复位 → 先应答再复位
        u8 result = 0x00;
        upd_send_frame(0x02, &result, 1);
        printf("[UART_CTL] System reset\n");
        delay_ms(50);
        WDT_RST();
        return;
    }
    if (upd_cmd == 0x03) {
        u8 result = 0x00;
        if (upd_payload[0] == 0x01) {
            bsp_sys_mute();
            printf("[UART_CTL] Mute ON\n");
        } else {
            bsp_sys_unmute();
            printf("[UART_CTL] Mute OFF\n");
        }
        upd_send_frame(0x03, &result, 1);
        return;
    }
    if (upd_cmd == 0x04) {
        u8 vol    = upd_payload[0];
        u8 result = 0x00;
        bsp_set_volume(vol);
        cm_sync(); // 强制刷入Flash持久化
        printf("[UART_CTL] Volume set: %u\n", vol);
        upd_send_frame(0x04, &result, 1);
        return;
    }
    if (upd_cmd == 0x05) {
        u8 src    = upd_payload[0];
        u8 result = 0x00;
        if (src == 0x01) {
            func_cb.sta = FUNC_BT;
            printf("[UART_CTL] Audio switch: BT\n");
        } else {
            func_cb.sta = FUNC_AUX;
            printf("[UART_CTL] Audio switch: AUX\n");
        }
        upd_send_frame(0x05, &result, 1);
        return;
    }
    if (upd_cmd == 0x06) {
        uint bt_sta = bt_get_status();
        u8   result;
        if (bt_sta == BT_STA_CONNECTED)
            result = 0x02; // 已连接
        else if (bt_sta == BT_STA_PLAYING)
            result = 0x03; // 播放中
        else if (bt_sta == BT_STA_CONNECTING || bt_sta == BT_STA_SCANNING || bt_sta == BT_STA_INITING)
            result = 0x01; // 连接中
        else
            result = 0x00; // 已断开
        printf("[UART_CTL] BT status: %u (raw=%u)\n", result, bt_sta);
        upd_send_frame(0x06, &result, 1);
        return;
    }
    if (upd_cmd == 0x07) {
        u8 mode   = upd_payload[0];
        u8 result = 0x00;
#if RGB_WS2812_EN
        ws2812_override_mode = mode;
        printf("[UART_CTL] LED mode: %u\n", mode);
#else
        printf("[UART_CTL] LED mode: %u (WS2812 disabled)\n", mode);
#endif
        upd_send_frame(0x07, &result, 1);
        return;
    }
    if (upd_cmd == 0x08) {
        u8 result = 0x00;
        if (upd_data_len > 0 && upd_data_len < sizeof(upd_payload)) {
            upd_payload[upd_data_len] = '\0';
            char *name                = (char *)upd_payload;
            // 1. 更新运行时名称
            updata_bt_name(name);
            memset(xcfg_cb.bt_name, 0, sizeof(xcfg_cb.bt_name));
            strncpy(xcfg_cb.bt_name, name, sizeof(xcfg_cb.bt_name) - 1);
            // 2. 写入Flash持久化到BT_NAME_UPDATA地址
            u8 flash_buf[36];
            u8 len = upd_data_len + 4; // ## + name + ##
            if (len > 36)
                len = 36;
            memset(flash_buf, 0, sizeof(flash_buf));
            flash_buf[0] = 0x23;
            flash_buf[1] = 0x23;
            memcpy(&flash_buf[2], name, upd_data_len);
            flash_buf[2 + upd_data_len]     = 0x23;
            flash_buf[2 + upd_data_len + 1] = 0x23;
            cm_write(flash_buf, PAGE0(BT_NAME_UPDATA), len);
            cm_sync();
            printf("[UART_CTL] BT name saved: %s\n", name);
        }
        upd_send_frame(0x08, &result, 1);
        return;
    }
    // ---- OTA命令分发 ----
    if (upd_cmd == CMD_ERASE) {
        upd_do_erase();
    } else if (upd_cmd == CMD_DATA) {
        upd_do_data();
    } else if (upd_cmd == CMD_FINISH) {
        upd_do_finish();
    } else {
        printf("[MP3_UPD] Unknown cmd 0x%02X\n", upd_cmd);
    }
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
    upd_state     = UPD_IDLE;
    upd_max_seq   = 0;
    upd_flash_crc = 0;
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
            if (ch == FRAME_HDR) {
                upd_state = UPD_GOT_HDR;
            }
            break;

        case UPD_GOT_HDR:       // 收到帧头, 读命令
            upd_cmd   = ch;
            upd_state = UPD_GOT_CMD;
            break;

        case UPD_GOT_CMD:       // 收到命令, 读 LEN_H
            upd_data_len = (u16)ch << 8;
            upd_state    = UPD_RX_LEN_L;
            break;

        case UPD_RX_LEN_L:
            upd_data_len |= ch;
            upd_idx = 0;
            if (upd_data_len > 0) {
                upd_state = UPD_RX_DATA;
            } else {
                upd_state = UPD_RX_FCS;
            }
            break;

        case UPD_RX_DATA:
            if (upd_idx < sizeof(upd_payload)) {
                upd_payload[upd_idx++] = ch;
            }
            if (upd_idx >= upd_data_len) {
                upd_state = UPD_RX_FCS;
            }
            break;

        case UPD_RX_FCS:
            upd_rx_fcs = ch;
            upd_state  = UPD_IDLE;
            upd_process_frame();
            break;
        }
    }
}
