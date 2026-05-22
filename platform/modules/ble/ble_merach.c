#include "include.h"
#include "ble_merach.h"

#if LE_EN

/* ================================================================
 * 本地变量
 * ================================================================ */
static u8  s_merach_heart_ccc[2] = {0, 0};   // Heart Pack CCCD 缓存
static u8  s_merach_spp_ccc[2]   = {0, 0};   // SPP Data CCCD 缓存

/* 心跳包响应数据 */
static const u8 s_heartbeat_rsp[] = {0xAA, 0x01, 0x00, 0x01, 0x55};

/* ================================================================
 * 辅助函数：发送 Indicate
 * 使用 ble_tx_notify(index, buf, len) 发送，底层根据 CCCD 自动选择通知/指示
 * ================================================================ */
static bool merach_tx_heart(u8 *buf, u8 len)
{
    if (!ble_is_connect()) return false;
    return ble_tx_notify(MERACH_ATT_IDX_HEART, buf, len);
}

static bool merach_tx_spp(u8 *buf, u8 len)
{
    if (!ble_is_connect()) return false;
    return ble_tx_notify(MERACH_ATT_IDX_SPP, buf, len);
}

/* ================================================================
 * 校验和计算
 * ================================================================ */
static u8 merach_checksum(u8 *buf, u8 len)
{
    u8 sum = 0;
    for (u8 i = 0; i < len; i++) sum += buf[i];
    return sum;
}

/* ================================================================
 * API: 心跳响应
 * ================================================================ */
void ble_merach_heartbeat_rsp(void)
{
    merach_tx_heart((u8 *)s_heartbeat_rsp, sizeof(s_heartbeat_rsp));
}

/* ================================================================
 * API: 通过 SPP 通道发送数据
 * 自动添加包头的帧封装，调用者只需提供 payload
 * ================================================================ */
bool ble_merach_send_data(u8 *buf, u8 len)
{
    /* 最大 payload: MTU-6 (头1+长1+校验1+尾1  +  可能的分段开销) */
    if (len > 120) return false;
    if (!ble_is_connect()) return false;

    /* 构建 Merach 帧 */
    u8 frame[128];
    u8 idx = 0;
    frame[idx++] = MERACH_FRAME_HEAD;              // 包头
    frame[idx++] = len;                             // 包体长度
    memcpy(&frame[idx], buf, len);                  // 包体
    idx += len;
    frame[idx++] = merach_checksum(&frame[1], len+1); // 校验和（从长度字段开始）
    frame[idx++] = MERACH_FRAME_TAIL;               // 包尾

    return merach_tx_spp(frame, idx);
}

/* ================================================================
 * 读取回调
 * ================================================================ */
u8 ble_merach_att_read_callback(u16 handle, u8 *ptr, u8 len)
{
    if (handle == HDL_MERACH_HEART_CCC) {
        /* 返回 CCCD 缓存 */
        *ptr = s_merach_heart_ccc[0];
        return 1;
    }
    if (handle == HDL_MERACH_SPP_CCC) {
        *ptr = s_merach_spp_ccc[0];
        return 1;
    }
    return 0;  /* 未处理 */
}

/* ================================================================
 * 写入回调
 * ================================================================ */
u8 ble_merach_att_write_callback(u16 handle, u8 *ptr, u8 len)
{
    if (handle == HDL_MERACH_HEART_VAL) {
        /* Heart Pack 通道写入 → 心跳检测 */
        if (len == 5 && ptr[0] == 0xAA && ptr[1] == 0x01
            && ptr[2] == 0x00 && ptr[4] == 0x55) {
            /* 是心跳请求，回复心跳响应 */
            ble_merach_heartbeat_rsp();
            printf("[MERACH] heartbeat\n");
        } else {
            printf("[MERACH] heart unknown: ");
            print_r(ptr, len);
        }
        return true;
    }

    if (handle == HDL_MERACH_HEART_CCC) {
        s_merach_heart_ccc[0] = ptr[0];
        s_merach_heart_ccc[1] = (len > 1) ? ptr[1] : 0;
        return true;
    }

    if (handle == HDL_MERACH_SPP_VAL) {
        /* SPP Data 通道写入 → 透传数据 */
        printf("[MERACH] SPP rx[%d]: ", len);
        print_r(ptr, len);
        /* 在这里添加你的数据解包处理逻辑 */
        /* 例如: COMM_DataUnpack(ptr, len); */
        return true;
    }

    if (handle == HDL_MERACH_SPP_CCC) {
        s_merach_spp_ccc[0] = ptr[0];
        s_merach_spp_ccc[1] = (len > 1) ? ptr[1] : 0;
        return true;
    }

    return false;
}

/* ================================================================
 * 轮询处理（暂空，留给应用层扩展）
 * ================================================================ */
void ble_merach_process(void)
{
    /* nothing yet */
}

/* ================================================================
 * 初始化
 * ================================================================ */
void ble_merach_init(void)
{
    s_merach_heart_ccc[0] = 0;
    s_merach_heart_ccc[1] = 0;
    s_merach_spp_ccc[0] = 0;
    s_merach_spp_ccc[1] = 0;
    printf("[MERACH] init ok\n");
}

#endif // LE_EN
