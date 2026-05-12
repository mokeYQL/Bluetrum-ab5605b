#include "include.h"
#include "func.h"
#include "func_bt.h"

#if EXFLASH_MUSIC_WARNING
void wav_output_play(void);
#endif
AT(.text.func.bt.msg)
void func_bt_message(u16 msg)
{
#if BT_HID_SIMPLE_KEYBOARD
    if (simple_keyboard_message(msg)) {
        return;
    }
#endif

#if (BT_HID_EN && BT_HID_TYPE == HID_TYPE_DOUYIN)
    if (hid_message(msg)) {
        return;
    }
#endif

    switch (msg) {
    case KU_PLAY:
    case KU_PLAY_HSF:
    case KU_PLAY_POWER:
#if EXFLASH_MUSIC_WARNING
        wav_output_play();
        my_printf("play_bt\n");
        break;
#endif
        bt_music_play_pause();
        f_bt.pp_2_unmute = sys_cb.mute;
        break;

    case KU_PREV_VOL_DOWN:
    case KL_VOL_DOWN_PREV:
    case KU_PREV:
        bt_music_prev();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;

    case KU_NEXT_VOL_UP:
    case KL_VOL_UP_NEXT:
    case KU_NEXT:
        bt_music_next();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;
    case KL_PREV:
        bt_music_rewind();
        break;
    case KLU_PREV:
        bsp_clr_mute_sta();
        bt_music_rewind_end();
        break;
    case KL_NEXT:
        bt_music_fast_forward();
        break;
    case KLU_NEXT:
        bsp_clr_mute_sta();
        bt_music_fast_forward_end();
        break;
//    case KL_PLAY:
//        bsp_clr_mute_sta();
//        bt_disconnect(1);
//        break;

    case KU_HSF:
    case KD_PLAY:
	case KD_PLAY_HSF:
    case KD_PLAY_POWER:
    case KD_PLAY_MODE:
        bsp_clr_mute_sta();
        if (bt_get_status() >= BT_STA_CONNECTED) {
            bt_call_redial_last_number();           //回拨电话
#if WARNING_BT_CALLOUT
            func_mp3_res_play(RES_BUF_CALLOUT_MP3, RES_LEN_CALLOUT_MP3);
#endif
#if BT_TWS_EN
        } else if(xcfg_cb.bt_tws_en && xcfg_cb.bt_tws_pair_mode == 2) {
            if(bt_tws_is_connected()) {
                bt_tws_disconnect();
            } else {
                bt_tws_search_slave(15000);
            }
#endif
        } else {
            func_bt_switch_voice_lang();            //切换提示音语言
        }
        break;

#if BT_MAP_EN
    case KL_MODE:
        my_printf("---KL_MODE\n");
        if(bt_is_ios_device()){
            bt_hfp_send_at_cmd();
        }else{
            bt_map_start();
        }
        delay_5ms(10); //延迟一下，等它发送完毕
        break;
#elif BT_TWS_EN
    case KL_MODE:
        if(xcfg_cb.bt_tws_en && xcfg_cb.bt_tws_pair_mode == 3) {
            if(bt_tws_is_connected()) {
                bt_tws_disconnect();
            } else {
                bt_tws_search_slave(15000);
            }
        }
        break;
#endif

    case KD_VOL_UP_NEXT:
    case KD_NEXT_VOL_UP:
        bsp_clr_mute_sta();
#if BT_HID_MENU_EN
        if (xcfg_cb.bt_hid_menu_en) {
            bsp_bt_hid_tog_conn();
        }
#endif
        break;

    case KD_PREV_VOL_DOWN:
    case KD_VOL_DOWN_PREV:
        bsp_clr_mute_sta();
        bsp_bt_hid_photo();
        break;

    case MSG_SYS_500MS:
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
#if BT_CONNECTED_AUTO_PLAY_EN
        if (f_bt.autoplay > 0) {
            f_bt.autoplay++;
            if (f_bt.autoplay >= 3) {
                f_bt.autoplay = 0;
                if (bt_get_status() == BT_STA_CONNECTED) {
                    bt_music_play();
                    //printf("BT AutoPlay\n");
                }
            }
        }
#endif
        break;

    case EVT_A2DP_MUSIC_PLAY:
        dac_fade_in();
        if (f_bt.pp_2_unmute) {
            f_bt.pp_2_unmute = 0;
            bsp_clr_mute_sta();
        }
        break;

    case EVT_A2DP_MUSIC_STOP:
        dac_fade_out();
        break;

    case EVT_KEY_2_UNMUTE:
        bsp_clr_mute_sta();
        break;

#if BT_REC_EN
    case KU_REC:
        if ((!dev_is_online(DEV_SDCARD)) && (!dev_is_online(DEV_UDISK) && (!dev_is_online(DEV_SDCARD1)))) {
            break;
        }
        if (bt_is_connected()) {
            sfunc_record();
    #if REC_AUTO_PLAY
            if (rec_cb.flag_play) {
                rec_cb.flag_play = 0;
                bt_audio_bypass();
                sfunc_record_play();
                bt_audio_enable();
                dac_fade_in();
            }
    #endif
        }
        break;
#endif // BT_REC_EN

#if FUNC_BT_DUT_EN
    case KU_DUT_MODE:
    case KL_DUT_MODE:
    case KD_DUT_MODE:
        if (func_cb.sta == FUNC_BT_DUT) {
            func_cb.sta = FUNC_BT;
        } else {
            func_cb.sta = FUNC_BT_DUT;
        }
        break;
#endif

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.btring.msg)
void sfunc_bt_ring_message(u16 msg)
{
    switch (msg) {
    case KU_PLAY:
    case KU_HSF:                //接听
    case KU_PLAY_POWER:
    case KU_PLAY_MODE:
    case KU_PLAY_HSF:
        bsp_clr_mute_sta();
        bt_call_answer_incoming();
        break;

    case KL_PLAY:
    case KL_HSF:
    case KL_PLAY_HSF:
    case KL_PLAY_POWER:
    case KL_PLAY_MODE:
        bsp_clr_mute_sta();
        bt_call_terminate();    //挂断
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
        break;

    default:
        func_message(msg);
        break;
    }
}

void sfunc_bt_call_message(u16 msg)
{
    u8 call_status;
    switch (msg) {
    case KU_PLAY:
    case KU_HSF:
	case KU_PLAY_HSF:
    case KU_PLAY_POWER:
    case KU_PLAY_MODE:
        bsp_clr_mute_sta();
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incoming();  //接听第2路通话
        } else {
            bt_call_terminate();        //挂断当前通话
            func_mp3_res_play(RES_BUF_CALLOUT_MP3, RES_LEN_CALLOUT_MP3);
        }
        break;

    case KL_PLAY:
        bsp_clr_mute_sta();
#if BT_HFP_CALL_PRIVATE_EN
        if (xcfg_cb.bt_hfp_private_en) {
            bt_call_private_switch();
            func_mp3_res_play(RES_BUF_PRIV_CALL_MP3, RES_LEN_PRIV_CALL_MP3);
        }
#endif // BT_HFP_CALL_PRIVATE_EN
        break;

    case KL_HSF:
        bsp_clr_mute_sta();
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_terminate();        //拒接第2路通话
        } else if(call_status == BT_CALL_3WAY_CALL) {
            bt_call_swap();             //切换两路通话
        }
        break;

    case KL_NEXT_VOL_UP:
    case KH_NEXT_VOL_UP:
    case KU_VOL_UP_NEXT:
    case KU_VOL_UP:
    case KL_VOL_UP:
    case KH_VOL_UP:
        if (sys_cb.hfp_vol < 15) {
            sys_cb.hfp_vol++;
            bt_hfp_set_spk_gain();
            bsp_change_volume(sys_cb.hfp_vol * sys_cb.hfp2sys_mul);
        }
        gui_box_show_vol();
        break;

    case KL_PREV_VOL_DOWN:
    case KH_PREV_VOL_DOWN:
    case KU_VOL_DOWN_PREV:
    case KU_VOL_DOWN:
    case KL_VOL_DOWN:
    case KH_VOL_DOWN:
        if (sys_cb.hfp_vol > 0) {
            sys_cb.hfp_vol--;
            bt_hfp_set_spk_gain();
            bsp_change_volume(sys_cb.hfp_vol * sys_cb.hfp2sys_mul);
        }
        gui_box_show_vol();
        break;

#if BT_HFP_REC_EN
    case KU_REC:
        sfunc_record();
        break;
#endif

    case EVT_HFP_SET_VOL:
        bsp_change_volume(sys_cb.hfp_vol * sys_cb.hfp2sys_mul);
        gui_box_show_vol();
        break;

    case EVT_A2DP_MUSIC_PLAY:
        dac_fade_in();
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
        break;

    default:
        func_message(msg);
        break;
    }
}

#if FUNC_BTHID_EN
AT(.text.func.bt.msg)
void func_bthid_message(u16 msg)
{
    switch (msg) {
    case KU_PLAY:
    case KU_PLAY_POWER:
    case KU_PLAY_MODE:
        if (bt_get_status() < BT_STA_DISCONNECTING) {
            bt_connect(BT_POWER_UP_RECONNECT_TIMES);
        } else {
            bsp_bt_hid_photo();
            mp3_res_play(RES_BUF_TAKE_PHOTO_MP3,RES_LEN_TAKE_PHOTO_MP3);
        }
        break;

    case KL_PLAY:
        bt_disconnect(1);
        break;

    default:
        func_message(msg);
        break;
    }
}
#endif


#if BT_HID_SIMPLE_KEYBOARD
AT(.text.func.bt.msg)
bool simple_keyboard_message(u16 msg)
{
    bool msg_processed = 1;
    switch (msg) {

        case KD_PLAY:    //回拨电话
            if (bt_is_ios_device()) {
                bt_call_redial_last_number();
            } else {
                btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY);
                btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY_UP);
//                btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY);
//                btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY_UP);
            }

        case KU_PLAY:
            printf("skb: play\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY);
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY_UP);
            break;

        case KU_PREV:
        case KL_PREV:
            printf("skb: prev\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PREV);
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_RELEASE);
            break;

        case KU_NEXT:
        case KL_NEXT:
            printf("skb: next\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_NEXT);
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_RELEASE);
            break;

        case KU_VOL_UP:
        case KL_VOL_UP:
        case KH_VOL_UP:
            printf("skb: vol+\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_VOL_UP);
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_RELEASE);
            break;

        case KU_VOL_DOWN:
        case KL_VOL_DOWN:
        case KH_VOL_DOWN:
            printf("skb: vol-\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_VOL_DOWN);
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_RELEASE);
            break;

        case KL_PLAY:  //长按触发语音助手
            printf("skb: voice assistant key press\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY);
            break;

        case KLU_PLAY:
            printf("skb: voice assistant key up\n");
            btstack_hid_simple_keyboard(SIMPLE_KEYBOARD_PLAY_UP);
            msg_queue_clear();
            break;

        default:
            msg_processed = 0;
            break;
    }
    return msg_processed;
}

//hid可能用到的函数
//bt_hid_disconnect();
//bt_nor_delete_link_info();//清除配对信息
//bsp_bt_init();//重新初始化时要连接的协议在cfg_bt_support_profile中选
#endif


#if BT_HID_TYPE == HID_TYPE_DOUYIN
AT(.text.func.bt.msg)
bool hid_message(u16 msg)
{
    bool msg_processed = 1;
    switch (msg) {

        case KU_PLAY:
            printf("skb: play\n");
            bsp_bt_hid_keyboard(HID_PLAY_PAUSE);
            delay_5ms(15);
            bsp_bt_hid_keyboard(HID_KBD_RELEASE);
            break;

        case KU_PREV:
            printf("skb: prev\n");
            bsp_bt_hid_keyboard(HID_PREV_TRACK);
            delay_5ms(15);
            bsp_bt_hid_keyboard(HID_KBD_RELEASE);
            break;

        case KD_PREV:
            printf("skb: finger down slide \n");
            user_finger_down();
            break;

        case KL_PREV:
            printf("skb: down slide \n");
            hid_mouse_handler(HID_MOUSE_DOWN_SLIDE);
            break;


        case KU_NEXT:
            printf("skb: prev\n");
            bsp_bt_hid_keyboard(HID_NEXT_TRACK);
            delay_5ms(15);
            bsp_bt_hid_keyboard(HID_KBD_RELEASE);
            break;

        case KD_NEXT:
            printf("skb: finger up slide \n");
            user_finger_up();
            break;

        case KL_NEXT:
            printf("skb: up slide \n");
            hid_mouse_handler(HID_MOUSE_UP_SLIDE);
            break;

        case KU_VOL_UP:
        case KL_VOL_UP:
        case KH_VOL_UP:
            printf("skb: vol+\n");
            bsp_bt_hid_keyboard(HID_VOL_UP);
            delay_5ms(15);
            bsp_bt_hid_keyboard(HID_KBD_RELEASE);
            break;

        case KU_VOL_DOWN:
        case KL_VOL_DOWN:
        case KH_VOL_DOWN:
            printf("skb: vol-\n");
            bsp_bt_hid_keyboard(HID_VOL_DOWN);
            delay_5ms(15);
            bsp_bt_hid_keyboard(HID_KBD_RELEASE);
            break;

        case KL_PLAY:  //长按触发语音助手
            printf("skb: voice assistant key press\n");
            bsp_bt_hid_keyboard(HID_PLAY_PAUSE);
            break;

        case KLU_PLAY:
            printf("skb: voice assistant key up\n");
            bsp_bt_hid_keyboard(HID_KBD_RELEASE);
            msg_queue_clear();
            break;

        default:
            msg_processed = 0;
            break;
    }
    return msg_processed;
}
#endif
