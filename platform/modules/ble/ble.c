#include "include.h"
#include "bsp_ble.h"
#include "ble_merach.h"
#include "ble_devinfo.h"

#define BLE_MAX_LOCAL_NAME      32

void bt_get_local_bd_addr(u8 *addr);

void ble_get_local_bd_addr(u8 *addr)
{
    bt_get_local_bd_addr(addr);
    addr[5] ^= 0x55;
}

u8 ble_get_local_addr_mode(void)
{
    return 1;   //0:public 1:random no resolvable 2:random resolvable
}


#if LE_EN

const bool cfg_ble_security_en = LE_PAIR_EN;

const uint8_t adv_data_const[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x02,
    // Name, 0xff-企业标识符
    0x09, 0xff, 'B', 'l', 'u', 'e', 't', 'r', 'u', 'm',
};

const uint8_t scan_data_const[] = {
    // Name
    0x08, 0x09, 'B', 'L', 'E', '-', 'B', 'O', 'X',
};

/* ================================================================
 * BLE Profile — 仅保留参考项目一致的服务:
 *   1. Merach 私有协议 (0x0001-0x0007)
 *   2. Device Information Service (0x0008-0x0016)
 * ================================================================ */
const uint8_t profile_data[] =
{
    MERACH_PROFILE_ENTRIES
    DIS_PROFILE_ENTRIES

    // END
    0x00, 0x00,
};

const struct att_hdl_t att_hdl_tbl[] = {
    [0]  = {HDL_MERACH_HEART_VAL,  1},   // Merach Heart Pack VALUE
    [1]  = {HDL_MERACH_SPP_VAL,    1},   // Merach SPP Data VALUE
    [2]  = {HDL_DIS_MANU_VAL,      0},   // DIS Manufacturer Name
    [3]  = {HDL_DIS_MODEL_VAL,     0},   // DIS Model Number
    [4]  = {HDL_DIS_SERIAL_VAL,    0},   // DIS Serial Number
    [5]  = {HDL_DIS_HW_VAL,        0},   // DIS Hardware Revision
    [6]  = {HDL_DIS_FW_VAL,        0},   // DIS Firmware Revision
    [7]  = {HDL_DIS_SW_VAL,        0},   // DIS Software Revision
    [8]  = {HDL_DIS_SYSID_VAL,     0},   // DIS System ID
};

const u8 *ble_get_profile_data(void)
{
    return profile_data;
}

u32 ble_get_scan_data(u8 *scan_buf, u32 buf_size)
{
    memset(scan_buf, 0, buf_size);
    u32 data_len = sizeof(scan_data_const);
    memcpy(scan_buf, scan_data_const, data_len);

    //读取BLE配置的蓝牙名称
    int len;
    len = strlen(xcfg_cb.le_name);
    if (len > 0) {
        memcpy(&scan_buf[2], xcfg_cb.le_name, len);
        data_len = 2 + len;
        scan_buf[0] = len + 1;
    }
    return data_len;
}

u32 ble_get_adv_data(u8 *adv_buf, u32 buf_size)
{
    memset(adv_buf, 0, buf_size);
    u32 data_len = sizeof(adv_data_const);
    memcpy(adv_buf, adv_data_const, data_len);
    return data_len;
}

static void bsp_ble_init(void);
void ble_init_att(void)
{
    u8 buffer[4];
    memset(buffer, 0, 4);
    for (int i = 0; i < (sizeof(att_hdl_tbl) / sizeof(struct att_hdl_t)); i++) {
        ble_init_att_do(i, att_hdl_tbl[i].hdl, att_hdl_tbl[i].cfg, buffer, 4);
    }
    ble_merach_init();
    ble_devinfo_init();
    bsp_ble_init();
}

#define BLE_CMD_BUF_LEN     4
#define BLE_CMD_BUF_MASK    (BLE_CMD_BUF_LEN - 1)
#define BLE_RX_BUF_LEN      220

struct ble_cmd_t{
    u8 len;
    u16 handle;
    u8 buf[BLE_RX_BUF_LEN];
};

AT(.ble_rx_buf)
struct ble_cb_t {
    struct ble_cmd_t cmd[BLE_CMD_BUF_LEN];
    u8 cmd_rptr;
    u8 cmd_wptr;
} ble_cb;

/* ================================================================
 * BLE 读回调
 * ================================================================ */
u8 ble_att_read_callback(u16 handle, u8 *ptr, u8 len)
{
    printf("BLE SLAVE Read hanlde:[%04x]\n",handle);
    u8 data_len = 0;

    // Merach
    data_len = ble_merach_att_read_callback(handle, ptr, len);
    if (data_len) return data_len;

    // DIS
    data_len = ble_devinfo_att_read_callback(handle, ptr, len);
    if (data_len) return data_len;

    return data_len;
}

/* ================================================================
 * BLE 写回调
 * ================================================================ */
u8 ble_att_write_callback(u16 handle, u8 *ptr, u8 len)
{
    // Merach
    if (ble_merach_att_write_callback(handle, ptr, len)) {
        return true;
    }

    printf("BLE RX [%d]: \n", len);
    print_r(ptr, len);

    u8 wptr = ble_cb.cmd_wptr & BLE_CMD_BUF_MASK;
    ble_cb.cmd_wptr++;
    if (len > BLE_RX_BUF_LEN) {
        len = BLE_RX_BUF_LEN;
    }
    memcpy(ble_cb.cmd[wptr].buf, ptr, len);
    ble_cb.cmd[wptr].len = len;
    ble_cb.cmd[wptr].handle = handle;
    return true;
}

/* ================================================================
 * BLE 轮询处理
 * ================================================================ */
void bsp_ble_process(void)
{
    ble_merach_process();
}

/* ================================================================
 * 兼容桩函数 — bsp_ble.h 声明的旧 API
 * ================================================================ */
bool ble_send_packet(u8 *buf, u8 len)
{
    /* 旧版 Test Service 已移除，此函数不再使用 */
    return false;
}

/* ================================================================
 * 内部初始化
 * ================================================================ */
static void bsp_ble_init(void)
{
    memset(&ble_cb, 0, sizeof(struct ble_cb_t));
}

void ble_conn_callback(void)
{
    printf("ble_conn\n");
#if LE_AB_FOT_EN
    fot_ble_connect_callback();
#endif
}

void ble_disconn_callback(void)
{
    printf("ble_disconn\n");
#if LE_AB_FOT_EN
    fot_ble_disconnect_callback();
#endif
}

void ble_advertising_report_callback(uint8_t addr_type, uint8_t *addr, uint8_t adv_data_len, const uint8_t *adv_data, char rssi)
{
    int offset = 0;
    uint8_t ad_len;
    uint8_t ad_type;
    uint8_t local_name[BLE_MAX_LOCAL_NAME];

    while (adv_data_len >= (offset + 2)) {
        ad_len = adv_data[offset];
        offset++;
        ad_type = adv_data[offset];
        offset++;
        if (ad_type == 0x09) {
            if ((ad_len > 1) && (ad_len <= BLE_MAX_LOCAL_NAME)) {
                memcpy(local_name, &adv_data[offset], ad_len - 1);
                local_name[ad_len - 1] = '\0';
                printf("[adv report]: [%s] addr type:%u, addr:0x%02x-%02x-%02x-%02x-%02x-%02x\n", local_name,
                       addr_type, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
            }
            break;
        }
        offset--;
        offset += ad_len;
    }
}

#endif
