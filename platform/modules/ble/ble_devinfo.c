#include "include.h"
#include "ble_devinfo.h"

#if LE_EN

/* ================================================================
 * Device Information Service 版本字符串
 * 根据实际产品修改
 * ================================================================ */
#define DIS_MANUFACTURER    "Bluetrum"
#define DIS_MODEL_NUM       "AB5605B-BTBOX"
#define DIS_HW_VERSION      "V1.0"
#define DIS_FW_VERSION      "V1.0.0"
#define DIS_SW_VERSION      "V1.0.0"

/* ================================================================
 * 读取回调
 * ================================================================ */
u8 ble_devinfo_att_read_callback(u16 handle, u8 *ptr, u8 len)
{
    switch (handle) {
    case HDL_DIS_MANU_VAL:      /* 0x001B — Manufacturer Name 0x2A29 */
        memcpy(ptr, DIS_MANUFACTURER, sizeof(DIS_MANUFACTURER));
        return sizeof(DIS_MANUFACTURER) - 1;  /* 不含结尾 \0 */

    case HDL_DIS_MODEL_VAL:     /* 0x001D — Model Number 0x2A24 */
        memcpy(ptr, DIS_MODEL_NUM, sizeof(DIS_MODEL_NUM));
        return sizeof(DIS_MODEL_NUM) - 1;

    case HDL_DIS_SERIAL_VAL:    /* 0x001F — Serial Number 0x2A25 */
        /* 用 BLE 地址作为序列号 */
        {
            u8 addr[6];
            ble_get_local_bd_addr(addr);
            /* 格式: "AB5605B-XXXXXXXXXXXX" */
            u8 tmp = ptr[0];
            ptr[0] = addr[0];
            (void)tmp;
            return 6;
        }

    case HDL_DIS_HW_VAL:        /* 0x0021 — Hardware Revision 0x2A27 */
        memcpy(ptr, DIS_HW_VERSION, sizeof(DIS_HW_VERSION));
        return sizeof(DIS_HW_VERSION) - 1;

    case HDL_DIS_FW_VAL:        /* 0x0023 — Firmware Revision 0x2A26 */
        memcpy(ptr, DIS_FW_VERSION, sizeof(DIS_FW_VERSION));
        return sizeof(DIS_FW_VERSION) - 1;

    case HDL_DIS_SW_VAL:        /* 0x0025 — Software Revision 0x2A28 */
        memcpy(ptr, DIS_SW_VERSION, sizeof(DIS_SW_VERSION));
        return sizeof(DIS_SW_VERSION) - 1;

    case HDL_DIS_SYSID_VAL:     /* 0x0027 — System ID 0x2A23 */
        /* System ID = 制造商ID(5字节) + 厂商唯一ID(3字节) */
        {
            u8 sysid[8];
            memset(sysid, 0, sizeof(sysid));
            sysid[0] = 0x00;  /* LSB of Manufacturer ID (Bluetooth SIG) */
            sysid[1] = 0x1B;  /* Bluetrum public company identifier */
            sysid[2] = 0x00;
            sysid[3] = 0x00;
            sysid[4] = 0x00;
            /* 后3字节 = MAC 地址后3字节 */
            u8 addr[6];
            bt_get_local_bd_addr(addr);
            sysid[5] = addr[3];
            sysid[6] = addr[4];
            sysid[7] = addr[5];
            memcpy(ptr, sysid, 8);
            return 8;
        }

    default:
        return 0;
    }
}

/* ================================================================
 * 初始化
 * ================================================================ */
void ble_devinfo_init(void)
{
    /* 无需特殊初始化 */
}

#endif // LE_EN
