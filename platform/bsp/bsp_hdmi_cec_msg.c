#include "include.h"
#include "bsp_hdmi.h"

#if FUNC_HDMI_EN
enum {
    WAIT_CEC_ACK,
    SEND_POWER_STA,
    RECV_POWER_STA,
    ACTIVE_TV,  //C0
 };

u8 wait_and_active_tv(void)
{
    CEC_FRAME_T f_recv;
    CEC_FRAME_T f_send;
    u32 retry_cnt = 0;
    u32 ticks;
//    ticks = tick_get();
//    while(1) {
//        WDT_CLR();
//        if (get_cec_frame(&f_recv)  && !tick_check_expire(ticks,300)) {//300ms内电视机有反应则直接返回
//            printf("phyaddr recv ack, continue\n");
//            //return 1;
//        }
//        if (tick_check_expire(ticks,1000)) {            //1000ms_TV not reponse,ready to send power sta
//            printf("phyaddr recv fail, ready to send power sta\n");
//            break;
//        }
//    }

    f_send.opcode     = CECMSG_SET_SYSTEM_AUDIO_MODE;
    f_send.src_dst    = AUD2TV;
    f_send.arg_len    = 1;
    f_send.arg_buf[0] = 1;
    cec_tx_frame(&f_send);

    f_send.opcode     = CECMSG_SET_SYSTEM_AUDIO_MODE;
    f_send.src_dst    = AUD2BC;
    f_send.arg_len    = 1;
    f_send.arg_buf[0] = 1;
    cec_tx_frame(&f_send);

    f_send.opcode     = CECMSG_GIVE_DEVICE_POWER_STATUS;
    f_send.src_dst    = AUD2TV;
    f_send.arg_len    = 0;
    cec_tx_frame(&f_send);
    printf("send power sta\n");
    ticks = tick_get();
    retry_cnt = 3;
    while(1) {
        WDT_CLR();
        if (0 == retry_cnt) {
            printf("send power sta , noack, return\n");
            return 0;
        }

        if (get_cec_frame(&f_recv)) {       //send sta succeed
            printf("recv power sta, continue to active TV : 0x%X\n",f_recv.arg_buf[0]);
            break;
        }

        if (tick_check_expire(ticks,300)) {
            ticks = tick_get();
            f_send.opcode     = CECMSG_GIVE_DEVICE_POWER_STATUS;
            f_send.src_dst    = AUD2TV;
            f_send.arg_len    = 0;
            cec_tx_frame(&f_send);
            retry_cnt--;
            printf("not recv power sta, return fail\n");
            continue;
        }
    }

    printf("send arc active\n");
    f_send.opcode     = CECMSG_ARC_INITIATE;
    f_send.src_dst    = AUD2TV;
    f_send.arg_len    = 0;
    cec_tx_frame(&f_send);
    printf("send arc init\n");
    ticks = tick_get();
    retry_cnt = 3;
    while(1) {
        WDT_CLR();
        if (0 == retry_cnt) {
            printf("send ARC_INI , noack, return\n");
            return 0;
        }

        if (get_cec_frame(&f_recv)) {
            printf("recv ARC_INI, continue to run : 0x%X\n",f_recv.arg_buf[0]);
            return 0;
        }

        if (tick_check_expire(ticks,300)) {
            ticks = tick_get();
            f_send.opcode     = CECMSG_ARC_INITIATE;
            f_send.src_dst    = AUD2TV;
            f_send.arg_len    = 0;
            cec_tx_frame(&f_send);
            retry_cnt--;
            printf("not recv ARC_INI, return fail\n");
            continue;
        }
    }
    return 0;
}


AT(.text.hdmi_lib)
void cec_frame_process(void)
{
    CEC_FRAME_T f_recv;
    CEC_FRAME_T f_send;
    if (get_cec_frame(&f_recv)) {
        switch (f_recv.opcode) {
            case CECMSG_GIVE_AUDIO_STATUS:
                printf("deal CECOP_GIVE_AUDIO_STATUS\n");
                f_send.opcode      =  CECMSG_REPORT_AUDIO_STATUS;
                f_send.src_dst     =  AUD2TV;
                f_send.arg_buf[0]  =  sys_cb.vol ;
                f_send.arg_len     = 1;
                cec_tx_frame(&f_send);
                break;

        	case CECMSG_USER_CONTROL_PRESSED:
                if(CECCTR_PRESSED_VOLUME_UP == f_recv.arg_buf[0]){
                    msg_enqueue(KU_VOL_UP);
                } else if(CECCTR_PRESSED_VOLUME_DOWN == f_recv.arg_buf[0]) {
                    msg_enqueue(KU_VOL_DOWN);
                } else if(CECCTR_PRESSED_MUTE == f_recv.arg_buf[0]) {
                    msg_enqueue(KU_MUTE);
                }
				break;

            case CECMSG_USER_CONTROL_RELEASED:
               break;

            case CECMSG_GIVE_DEVICE_VENDOR_ID:
                f_send.opcode        = CECMSG_DEVICE_VENDOR_ID;
                f_send.src_dst       = AUD2BC;
                f_send.arg_len       = 3;
                f_send.arg_buf[0]    = 0x55;
                f_send.arg_buf[1]    = 0x66;
                f_send.arg_buf[2]    = 0x77;
                cec_tx_frame(&f_send);
                break;

            case CECMSG_GIVE_OSD_NAME:
                f_send.opcode        = CECMSG_SET_OSD_NAME;
                f_send.src_dst       = AUD2TV;
                f_send.arg_len       = 6;
                f_send.arg_buf[0]    = 'B';  // later_add
                f_send.arg_buf[1]    = 'L';
                f_send.arg_buf[2]    = 'T';
                f_send.arg_buf[3]    = 'R';
                f_send.arg_buf[4]    = 'U';
                f_send.arg_buf[5]    = 'M';
                cec_tx_frame(&f_send);
                break;

            default:
                hdmi_cec_default_process(&f_recv);
                break;
        }
    }
}

#endif // FUNC_HDMI_EN
