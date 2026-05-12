#include "include.h"

extern u8 eq_rx_buf[EQ_BUFFER_LEN];
void sco_audio_set_param(u8 type, u16 value);

AT(.rodata.bsp.spp)
static const u16 bt_key_msg_tbl[] = {
    KU_PLAY,
    KU_PREV,
    KU_NEXT,
    KU_VOL_DOWN,
    KU_VOL_UP,
    KU_MODE,
};

#if BT_SCO_DBG_EN
void bsp_aec_ack_param(u8 type)
{
    u8 buf[11], size = 11;
    buf[0] = 'a';
    buf[1] = 'e';
    buf[2] = 'c';
    buf[3] = (u8)xcfg_cb.bt_anl_gain;
    buf[4] = (u8)xcfg_cb.bt_dig_gain;
    buf[5] = (u8)xcfg_cb.mic_post_gain;
    buf[6] = (u8)xcfg_cb.bt_noise_threshoid;
    buf[7] = (u8)((u16)xcfg_cb.bt_noise_threshoid >> 8);
    buf[8] = (u8)xcfg_cb.bt_echo_level;
    buf[9] = (u8)xcfg_cb.bt_far_offset;
    buf[10] = 0;

#if LE_EN
    if (type) {
        ble_send_packet(buf, size);
    } else
#endif // LE_EN
    {
        bt_spp_tx(buf, size);
    }
}

void bsp_aec_ack(u8 type)
{
    u8 buf[6];
    buf[0] = 'a';
    buf[1] = 'e';
    buf[2] = 'c';
    buf[3] = ' ';
    buf[4] = 'o';
    buf[5] = 'k';
#if LE_EN
    if (type) {
        ble_send_packet(buf, 6);
    } else
#endif // LE_EN
    {
        bt_spp_tx(buf, 6);
    }
}
#endif

void bt_app_cmd_process(u8 *ptr, u16 size, u8 type)
{
#if EQ_DBG_IN_SPP
    if (xcfg_cb.eq_dgb_spp_en) {
        eq_spp_cb_t *p = &eq_dbg_cb.eq_spp_cb;
        if ((ptr[0] == 'E' && ptr[1] == 'Q') || (ptr[0] == 'D' && ptr[1] == 'R')) {       //EQ消息
            if (ptr[2] == '?') {
                memcpy(eq_rx_buf, ptr, size);
                msg_enqueue(EVT_ONLINE_SET_EQ);
                return;
            }
            u32 rx_size = little_endian_read_16(ptr, 4) + 6;
            memcpy(eq_rx_buf, ptr, size);
            p->rx_size = rx_size;
            if (size < rx_size) {
                p->remain = 1;
                p->remian_ptr = size;
            } else {
                p->remain = 0;
                p->remian_ptr = 0;
                msg_enqueue(EVT_ONLINE_SET_EQ);
            }
            return;
        }
        if (p->remain) {
            memcpy(&eq_rx_buf[p->remian_ptr], ptr, size);
            p->remian_ptr += size;
            if (p->rx_size == p->remian_ptr) {
                msg_enqueue(EVT_ONLINE_SET_EQ);
                memset(p, 0, sizeof(eq_spp_cb_t));
            }
            return;
        }
    }
#endif

//    printf("SPP RX:");
//    print_r(ptr, size);

#if BT_SCO_DBG_EN
    if (ptr[0] == 'a' && ptr[1] == 'e' && ptr[2] == 'c') {
        u8 cnt, i;
        u8 args;

        if (ptr[3] == '?') {
            bsp_aec_ack_param(type);
            return;
        }
        cnt = 3;
        args = 6;
        if (size == 16) {
            args = 9;
        }
        for (i = 0; i < args; i++) {
            u16 value;
            if (i == AEC_PARAM_NOISE) {    //第4参数为2Byte
                value = ptr[cnt++];
                value |= ptr[cnt++] << 8;
            } else {
                value = ptr[cnt++];
            }
            sco_audio_set_param(i, value);
        }
        bsp_aec_ack(type);
        return;
    }
#endif

    switch (ptr[0]) {
        case 0x01:
            //system control
            if (size != 4) {
                return;     //2byte data + 2byte crc16
            }
            if ((ptr[1] > 0) && (ptr[1] < sizeof(bt_key_msg_tbl))) {
                msg_enqueue(bt_key_msg_tbl[ptr[1] - 1]);
            }
            break;

        case 0x02:
            //rgb leds control
            if (size != 7) {
                return;     //5byte data + 2byte crc16
            }
#if PWM_RGB_EN
            if (ptr[1] > 0) {
                pwm_rgb_write(ptr[2], ptr[3], ptr[4]);
            } else {
                pwm_rgb_close();
            }
#endif // PWM_RGB_EN
            break;

        case 0x03:
            if (ptr[1] == 'T') {
                RTCCNT = GET_LE32(ptr + 2);
                //printf("settime: %d\n", (u32)RTCCNT);
            }
            //ota control
            break;
    }
}

void spp_rx_callback(uint8_t *packet, uint16_t size)
{
#if FOT_EN
    if(fot_app_connect_auth(packet, size)){
        fot_recv_proc(packet, size);
        return;
    }
#endif

#if BT_SPP_EN
    bt_app_cmd_process(packet, size, 0);
#endif // BT_SPP_EN
}

void spp_connect_callback(void)
{
    printf("-->spp_connect_callback\n");

#if FOT_EN
    fot_spp_connect_callback();
#endif
}

void spp_disconnect_callback(void)
{
    printf("-->spp_disconnect_callback\n");

#if FOT_EN
    fot_spp_disconnect_callback();
#endif
}

#if BT_CARPLAY_EN
//android auto uuid
const uint8_t sdp_spp_custom_service_record[] = {
//private static final UUID MY_UUID_INSECURE =UUID.fromString("4de17a00-52cb-11e6-bdf4-0800200c9a66");
    0x36, 0x00, 0x4e, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x08, 0x09,
    0x00, 0x01, 0x35, 0x11, 0x1c,
    0x4d, 0xe1, 0x7a, 0x00, 0x52, 0xcb, 0x11, 0xe6, 0xbd, 0xf4, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66,
    0x09, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x7f, 0xde, 0x09, 0x00, 0x04,
    0x35, 0x0c, 0x35, 0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00, 0x03, 0x08, 0x01, 0x09, 0x00,
    0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x01, 0x09, 0x01, 0x02, 0x09, 0x01, 0x00, 0x25, 0x05,
    0x53, 0x70, 0x70, 0x31, 0x00
};

//ios
const uint8_t sdp_spp_carplay_service_record[] = {
    0x36, 0x00, 0x66, 0x09, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x09, 0x09, 0x00, 0x01, 0x35, 0x11,
    0x1C, 0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA,
    0xFF, 0x09, 0x00, 0x04, 0x35, 0x0C, 0x35, 0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00, 0x03,
    0x08, 0x01, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x35, 0x09, 0x09,
    0x65, 0x6E, 0x09, 0x00, 0x6A, 0x09, 0x01, 0x00, 0x09, 0x00, 0x08, 0x08, 0xFF, 0x09, 0x00, 0x09,
    0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x01, 0x09, 0x01, 0x00, 0x09, 0x01, 0x00, 0x25, 0x0A, 0x43,
    0x75, 0x73, 0x74, 0x6F, 0x6D, 0x65, 0x72, 0x63, 0x00,
};

/////////////////////////////////////////////////////////////////////
//设置 EIR UUID
u8 bt_inquiry_res_eir_uuid(u8* prt)
{
    u8 uuid[16] = {0xFF, 0xCA, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xDE, 0xDE, 0xFA, 0xCA, 0xDE, 0x00, 0x00, 0x00, 0x00};
//	u8 uuid1[16] = {0xEC, 0x88, 0x43, 0x48, 0xCD, 0x41, 0x40, 0xA2, 0x97, 0x27, 0x57, 0x5D, 0x50, 0xBF, 0x1F, 0xD3};
	u8 uuid1[16] = {0xD3, 0x1F, 0xBF, 0x50, 0x5D, 0x57, 0x27, 0x97, 0xA2, 0x40, 0x41, 0xCD, 0x48, 0x43, 0x88, 0xEC};
    memcpy(prt,uuid,16);
    memcpy(prt+16,uuid1,16);
    return 32;
}

const u8 user_search_uuid128[16]={0x00,0x00,0x11,0x01, 0x00,0x00,  0x10,0x00,  0x80,0x00,  0x00,0x80,0x5f,0x9b,0x34,0xFb}; ///test uuid
const u8 uuid_mfi[16] = {0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFE};
//发起SPP连接之前先修改以下函数UUID，即可按此UUID 连接
void bt_spp_user_uuid(u8 *uuid)
{
    memcpy(uuid,user_search_uuid128,16);
}

#endif
