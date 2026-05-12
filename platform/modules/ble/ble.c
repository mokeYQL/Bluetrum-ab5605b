#include "include.h"
#include "bsp_ble.h"

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

const uint8_t profile_data[] =
{
// 0x0001 PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x0f, 0x18,
    // 0x0002 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL-DYNAMIC | READ | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x12, 0x03, 0x00, 0x19, 0x2a,
    // 0x0003 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL-DYNAMIC | READ | NOTIFY-''
    // READ_ANYBODY
    0x08, 0x00, 0x02, 0x01, 0x03, 0x00, 0x19, 0x2a,
    // 0x0004 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0a, 0x01, 0x04, 0x00, 0x02, 0x29, 0x00, 0x00,

// Test Service
    // 0x0005 PRIMARY_SERVICE-FF10
    0x0a, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x28, 0x10, 0xff,
    // Test Characteristic, with read and notify
    // 0x0006 CHARACTERISTIC-FF11-READ | WRITE | NOTIFY | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x1a, 0x09, 0x00, 0xf1, 0xff,
    // 0x0007 VALUE-FF11-READ | WRITE | NOTIFY | DYNAMIC-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x09, 0x00, 0xf1, 0xff,
    // 0x0008 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
//    0x0a, 0x00, 0x0a, 0x01, 0x0a, 0x00, 0x02, 0x29, 0x00, 0x00,

    // APP
    // 0x000a PRIMARY_SERVICE-FF12
    0x0a, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x00, 0x28, 0x12, 0xff,
    // APP->BSP Characteristic, with write
    // 0x000b CHARACTERISTIC-FF13-WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x03, 0x28, 0x08, 0x0c, 0x00, 0x13, 0xff,
    // 0x000c VALUE-FF13-WRITE | DYNAMIC-''
    // WRITE_ANYBODY
    0x08, 0x00, 0x08, 0x01, 0x0c, 0x00, 0x13, 0xff,
    // BSP->APP Characteristic, with notify
    // 0x000d CHARACTERISTIC-FF14-NOTIFY | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x0d, 0x00, 0x03, 0x28, 0x10, 0x0e, 0x00, 0x14, 0xff,
    // 0x000e VALUE-FF14-NOTIFY | DYNAMIC-''
    0x08, 0x00, 0x00, 0x01, 0x0e, 0x00, 0x14, 0xff,
    // 0x000f CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0a, 0x01, 0x0f, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x0010 CHARACTERISTIC-FF15-READ | WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x10, 0x00, 0x03, 0x28, 0x06, 0x11, 0x00, 0x15, 0xff,
    // 0x0003 VALUE-FF15-READ | WRITE_WITHOUT_RESPONSE | DYNAMIC-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x06, 0x01, 0x11, 0x00, 0x15, 0xff,

    // END
    0x00, 0x00,
}; // total size 99 bytes

const struct att_hdl_t att_hdl_tbl[] = {
    [0]   = {0x0003,  1},
    [1]   = {0x0009,  1},
    [2]   = {0x000c,  0},
    [3]   = {0x000e,  1},
    [4]   = {0x0011,  1},
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

    //读取BLE配置的蓝牙名称
//    int len;
//    len = strlen(xcfg_cb.le_name);
//    if (len > 0) {
//        memcpy(&adv_buf[5], xcfg_cb.le_name, len);
//        data_len = 5 + len;
//        adv_buf[3] = len + 1;
//    }

    return data_len;
}

static void bsp_ble_init(void);
void ble_init_att(void)
{
    u8 buffer[4];
//    buffer[0] = ble_get_bat_level();
//    ble_init_att_do(BLE_IDX_BATTERY, att_hdl_tbl[BLE_IDX_BATTERY].hdl, att_hdl_tbl[BLE_IDX_BATTERY].cfg, buffer, 1);   //初始化电量
    memset(buffer, 0, 4);
    for (int i = 0; i < (sizeof(att_hdl_tbl) / sizeof(struct att_hdl_t)); i++) {
        ble_init_att_do(i, att_hdl_tbl[i].hdl, att_hdl_tbl[i].cfg, buffer, 4);
    }
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

u8 ble_att_read_callback(u16 handle, u8 *ptr, u8 len)
{
    printf("BLE SLAVE Read hanlde:[%04x]\n",handle);
    u8 data_len = 0;
    switch(handle)
    {
        case 0x0003: //att_hdl_tbl[BLE_IDX_BATTERY].hdl
            *ptr = 0x60;
            data_len = 1;
            break;
        case 0x00007: //att_hdl_tbl[BLE_IDX_OLD_APP].hdl
            memset(ptr,0x55,5);
            data_len = 5;
            break;
        ///...
        default:    //默认用ble_init_att_do初始化的数值
            data_len = 0;
            break;
    }

    return data_len;
}

u8 ble_att_write_callback(u16 handle, u8 *ptr, u8 len)
{
#if FOT_EN
    if(handle == 0x0011){
        if(fot_app_connect_auth(ptr, len)){
            fot_recv_proc(ptr, len);
            return true;
        }
    }
#endif

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

bool ble_send_packet(u8 *buf, u8 len)
{
    if (buf == NULL || len > ble_get_gatt_mtu()) {
        return false;
    }

    printf("BLE TX [%d]: \n", len);
    print_r(buf, len);

    return ble_tx_notify(1, buf, len);
}

#if LE_AB_FOT_EN
bool ble_fot_send_packet(u8 *buf, u8 len)
{
    return ble_tx_notify(3, buf, len);
}
#endif

void bsp_ble_tx_test(void)
{
    int i;
    u8 ble_test_buf[66];
    for (i = 1; i < 66; i++) {
        ble_test_buf[i-1] = i;
    }
    for (i = 1; i < (ble_get_gatt_mtu() + 1); i++) {
        if (ble_is_connect()) {
            ble_test_buf[0] = i;
            ble_send_packet(ble_test_buf, i);
            delay_5ms(8);
            WDT_CLR();
        }
    }
}

void bsp_ble_process(void)
{
    if (ble_cb.cmd_rptr == ble_cb.cmd_wptr) {
        return;
    }
    u8 rptr = ble_cb.cmd_rptr & BLE_CMD_BUF_MASK;
    ble_cb.cmd_rptr++;
    u8 *ptr = ble_cb.cmd[rptr].buf;
    u8 len = ble_cb.cmd[rptr].len;
    u16 handle = ble_cb.cmd[rptr].handle;
    if (handle == 0x0009) {             //兼容旧版APP
        if ((len == 4) && (ptr[0] == 0x01) && (ptr[1] == 0x06)) {
            bsp_ble_tx_test();
        } else {
            bt_app_cmd_process(ptr, len, 1);
        }
    }
}

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
