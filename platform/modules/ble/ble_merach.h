#ifndef __BLE_MERACH_H__
#define __BLE_MERACH_H__

#include "include.h"

/*
 * Merach 私有协议 BLE 服务
 *
 * 128-bit UUID (MERACH + 0x8888 + 0x6666 + index + YULU):
 *   Service UUID:     index = 0x0080
 *   Heart Pack Char:  index = 0x0000
 *   SPP Data Char:    index = 0x0100
 */

/* 16字节 128-bit UUID */
#define MERACH_SVC_UUID  \
    0x48,0x43,0x41,0x52, 0x45,0x4D, 0x88,0x88, 0x66,0x66, 0x00,0x80, 0x55,0x4C,0x55,0x59
#define MERACH_HEART_UUID \
    0x48,0x43,0x41,0x52, 0x45,0x4D, 0x88,0x88, 0x66,0x66, 0x00,0x00, 0x55,0x4C,0x55,0x59
#define MERACH_SPP_UUID   \
    0x48,0x43,0x41,0x52, 0x45,0x4D, 0x88,0x88, 0x66,0x66, 0x01,0x00, 0x55,0x4C,0x55,0x59

/* profile_data handle 定义 */
#define HDL_MERACH_SVC       0x0012
#define HDL_MERACH_HEART_DEC 0x0013
#define HDL_MERACH_HEART_VAL 0x0014
#define HDL_MERACH_HEART_CCC 0x0015
#define HDL_MERACH_SPP_DEC   0x0016
#define HDL_MERACH_SPP_VAL   0x0017
#define HDL_MERACH_SPP_CCC   0x0018

/* Merach profile_data 条目（插入到 ble.c 的 profile_data[] 中） */
#define MERACH_PROFILE_ENTRIES \
    /* 0x0012: PRIMARY_SERVICE (128-bit UUID) */       \
    0x18, 0x00, 0x02, 0x00, 0x12, 0x00, 0x00, 0x28,   \
    MERACH_SVC_UUID,                                    \
                                                        \
    /* 0x0013: HEART PACK CHAR DECL */                  \
    0x1B, 0x00, 0x02, 0x00, 0x13, 0x00, 0x03, 0x28,   \
    0x28,                  /* props: WRITE | INDICATE */\
    0x14, 0x00,            /* value_handle = 0x0014 */  \
    MERACH_HEART_UUID,                                  \
                                                        \
    /* 0x0014: HEART PACK VALUE (DYNAMIC | WRITE) */    \
    0x16, 0x00, 0x08, 0x01, 0x14, 0x00,               \
    MERACH_HEART_UUID,                                  \
                                                        \
    /* 0x0015: HEART PACK CCCD */                       \
    0x0A, 0x00, 0x0A, 0x01, 0x15, 0x00, 0x02, 0x29,   \
    0x00, 0x00,                                         \
                                                        \
    /* 0x0016: SPP DATA CHAR DECL */                    \
    0x1B, 0x00, 0x02, 0x00, 0x16, 0x00, 0x03, 0x28,   \
    0x18,                  /* props: WRITE | NOTIFY */  \
    0x17, 0x00,            /* value_handle = 0x0017 */  \
    MERACH_SPP_UUID,                                    \
                                                        \
    /* 0x0017: SPP DATA VALUE (DYNAMIC | WRITE) */      \
    0x16, 0x00, 0x08, 0x01, 0x17, 0x00,               \
    MERACH_SPP_UUID,                                    \
                                                        \
    /* 0x0018: SPP DATA CCCD */                         \
    0x0A, 0x00, 0x0A, 0x01, 0x18, 0x00, 0x02, 0x29,   \
    0x00, 0x00,                                         \

/* att_hdl_tbl 索引 */
#define MERACH_ATT_IDX_HEART    5
#define MERACH_ATT_IDX_SPP      6

/* Merach 协议命令定义 */
#define MERACH_CMD_HEARTBEAT_REQ    0x01    // 心跳请求
#define MERACH_CMD_HEARTBEAT_RSP    0x01    // 心跳响应

/* Merach 数据包格式:
 *   [0] 包头:   0xAA
 *   [1] 长度:   包体长度（不含头尾校验）
 *   [2] 命令:   命令ID
 *   [3..N-2]   参数
 *   [N-1] 校验: 从[1]到[N-2]的累加和
 *   [N]   包尾: 0x55
 */
#define MERACH_FRAME_HEAD    0xAA
#define MERACH_FRAME_TAIL    0x55

/* API 函数 */
void ble_merach_heartbeat_rsp(void);
bool ble_merach_send_data(u8 *buf, u8 len);

/* 由 ble.c 调用的内部接口 */
u8   ble_merach_att_read_callback(u16 handle, u8 *ptr, u8 len);
u8   ble_merach_att_write_callback(u16 handle, u8 *ptr, u8 len);
void ble_merach_process(void);
void ble_merach_init(void);

#endif // __BLE_MERACH_H__
