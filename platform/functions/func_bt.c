#include "include.h"
#include "func.h"
#include "func_bt.h"

void sbc_decode_exit(void);
void btrf_power_balance_exit(void);

func_bt_t f_bt;

ALIGNED(64)
u16 func_bt_chkclr_warning(u16 bits)
{
    u16 value;
    GLOBAL_INT_DISABLE();
    value = f_bt.warning_status & bits;
    if(value != 0) {
        f_bt.warning_status &= ~value;
        GLOBAL_INT_RESTORE();
        return value;
    }
    GLOBAL_INT_RESTORE();
    return value;
}


#if FUNC_BT_EN
#if BT_HFP_REC_EN
void bt_sco_rec_start(void);
void bt_sco_rec_stop(void);
void bt_sco_rec_mix_do(u8 *buf, u32 samples);
void bt_sco_rec_init(void);

AT(.com_text.bt_rec)
void bt_sco_rec_mix(u8 *buf, u32 samples)
{
    bt_sco_rec_mix_do(buf, samples);
}

AT(.text.func.bt)
void func_bt_sco_rec_init(void)
{
    rec_src.spr = SPR_8000;
    rec_src.nchannel = 0x01;
    rec_src.source_start = bt_sco_rec_start;
    rec_src.source_stop  = bt_sco_rec_stop;
    f_bt.rec_pause = 0;
    rec_cb.sco_flag = 1;
    bt_sco_rec_init();
}
#endif

#if BT_REC_EN
void bt_music_rec_start(void);
void bt_music_rec_stop(void);

AT(.text.func.bt)
void bt_music_rec_init(void)
{
    rec_src.spr = SPR_44100;
    if (DAC_OUT_SPR == DAC_OUT_48K) {
        rec_src.spr = SPR_48000;
    }
#if KARAOK_REC_EN
    rec_src.nchannel = 0x11;
#else
	rec_src.nchannel = 0x82;
#endif
    rec_src.source_start = bt_music_rec_start;
    rec_src.source_stop  = bt_music_rec_stop;
    f_bt.rec_pause = 0;
    rec_cb.sco_flag = 0;
}

AT(.text.func.bt)
bool bt_rec_status_process(void)
{
    if (func_cb.sta == FUNC_BT) {
        func_bt_status();
        if ((f_bt.disp_status > BT_STA_PLAYING) || (f_bt.disp_status < BT_STA_CONNECTED)) {
            if ((f_bt.disp_status > BT_STA_PLAYING) && rec_cb.sco_flag) {
                return true;
            }
            f_bt.rec_pause = 1;
            return false;       //结束录音
        }
    }
    return true;
}

AT(.text.func.bt)
void bt_music_rec_continue(void)
{
#if !BT_HFP_REC_EN
    if ((f_bt.rec_pause) && ((f_bt.disp_status == BT_STA_PLAYING) || (f_bt.disp_status == BT_STA_CONNECTED))) {
        msg_enqueue(KU_REC);    //继续录音
        f_bt.rec_pause = 0;
    }
#endif
}

#endif // BT_REC_EN

#if DAC_OFF_FOR_BT_CONN_EN
void func_bt_set_dac(u8 enable)
{
    if (DAC_OFF_FOR_BT_CONN_EN) {
        if (enable) {
            if (f_bt.dac_sta == 0) {
                f_bt.dac_sta = 1;
            }
            dac_switch_for_bt(f_bt.dac_sta);
        } else {
            if (f_bt.dac_sta == 1) {
                f_bt.dac_sta = 0;
            }
            dac_switch_for_bt(f_bt.dac_sta);
        }
    }
}
#endif // DAC_OFF_FOR_BT_CONN_EN

void func_bt_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }

#if BT_TWS_EN
    bool tws_res_is_busy(void);
    while(tws_res_is_busy()) {
        tws_res_proc();
    }
#endif

#if BT_REC_EN
    sfunc_record_pause();
#endif

#if DAC_OFF_FOR_BT_CONN_EN
    u8 dac_sta = f_bt.dac_sta;
    func_bt_set_dac(1);
#endif // DAC_OFF_FOR_BT_CONN_EN

    bt_audio_bypass();
    mp3_res_play(addr, len);
    bt_audio_enable();

#if DAC_OFF_FOR_BT_CONN_EN
    func_bt_set_dac(dac_sta);
#endif // DAC_OFF_FOR_BT_CONN_EN

#if BT_REC_EN
    sfunc_record_continue();
#endif
}

//切换提示音语言
void func_bt_switch_voice_lang(void)
{
#if (LANG_SELECT == LANG_EN_ZH)
    if (xcfg_cb.lang_id >= LANG_EN_ZH) {
        sys_cb.lang_id = (sys_cb.lang_id) ? 0 : 1;
        multi_lang_init(sys_cb.lang_id);
        param_lang_id_write();
        param_sync();
        if (xcfg_cb.bt_tws_en) {
            bt_tws_sync_setting();                                              //同步语言
            tws_res_play(TWS_RES_LANGUAGE_EN + sys_cb.lang_id);                 //同步播放语言提示音
        } else {
            func_mp3_res_play(RES_BUF_LANGUAGE_MP3, RES_LEN_LANGUAGE_MP3);
        }
    }
#endif
}

#if BT_TWS_EN
static void func_bt_tws_set_channel(void)
{
    if(f_bt.tws_status & 0xc0) {   //对箱状态.
        tws_get_lr_channel(f_bt.tws_status);
        dac_mono_init(0, sys_cb.tws_left_channel);
    } else {
        dac_mono_init(1, 0);
    }
}

u8 func_bt_tws_get_channel(void)
{
    return sys_cb.tws_left_channel;
}
#endif

void func_bt_warning(void)
{
    u16 tws_warning;
#if BT_TWS_EN
    bool tws_res_is_busy(void);
    while(tws_res_is_busy()) {
        tws_res_proc();
    }
#endif

    tws_warning = func_bt_chkclr_warning(BT_WARN_TWS_DISCON | BT_WARN_TWS_CON);
    if(tws_warning) {
#if BT_TWS_EN
        if(xcfg_cb.bt_tws_en) {
            if(xcfg_cb.bt_tws_lr_mode != 0) {
                func_bt_tws_set_channel();
            }
        #if WARNING_BT_TWS_DISCON
            if (tws_warning == BT_WARN_TWS_DISCON) {
                func_mp3_res_play(RES_BUF_TWS_DISCON_MP3, RES_LEN_TWS_DISCON_MP3);
            }
        #endif
        }
#endif
    }

    if(func_bt_chkclr_warning(BT_WARN_DISCON)) {
#if WARNING_BT_DISCONNECT
        func_mp3_res_play(RES_BUF_DISCONNECT_MP3, RES_LEN_DISCONNECT_MP3);
#endif // WARNING_BT_DISCONNECT
#if WARNING_BT_WAIT_CONNECT
        func_mp3_res_play(RES_BUF_WAIT4CONN_MP3, RES_LEN_WAIT4CONN_MP3);
#endif // WARNING_BT_WAIT_CONNECT
        f_bt.autoplay = 0;
    }

	if(func_bt_chkclr_warning(BT_WARN_CON)) {
#if WARNING_BT_CONNECT
        func_mp3_res_play(RES_BUF_CONNECTED_MP3, RES_LEN_CONNECTED_MP3);
#endif
        f_bt.autoplay = 1;
    }

#if WARNING_BT_TWS_CONNECT
    if(xcfg_cb.bt_tws_en) {
        tws_warning = func_bt_chkclr_warning(BT_WARN_TWS_SCON | BT_WARN_TWS_MCON );
        if(tws_warning != 0) {
            f_bt.tws_had_pair = 1;
            if (xcfg_cb.bt_tws_lr_mode != 0) {
                func_bt_tws_set_channel();
            }
            ///固定声道方案，TWS连接后异步播放声道提示音。否则同步播放连接提示音
            if (xcfg_cb.bt_tws_lr_mode >= 8) {
                tws_get_lr_channel(tws_warning << 4);

                if(sys_cb.tws_left_channel) {
                    func_mp3_res_play(RES_BUF_LEFT_CH_MP3, RES_LEN_LEFT_CH_MP3);
                } else {
                    bt_audio_bypass();
                    delay_5ms(200);
                    func_mp3_res_play(RES_BUF_RIGHT_CH_MP3, RES_LEN_RIGHT_CH_MP3);
                    bt_audio_enable();
                }
            } else {
                if (tws_warning & BT_WARN_TWS_MCON) {
                    tws_res_play(TWS_RES_TWS_CONNECTED);
                }
            }
        }
    }
#endif
#if WARNING_BT_PAIR
    if(func_bt_chkclr_warning(BT_WARN_PAIRING)) {
        func_mp3_res_play(RES_BUF_PAIRING_MP3, RES_LEN_PAIRING_MP3);
    }
#endif
#if BT_HID_MENU_EN
    //按键手动断开HID Profile的提示音
    if (xcfg_cb.bt_hid_menu_en) {
    #if WARNING_BT_HID_MENU
        if (func_bt_chkclr_warning(BT_WARN_HID_CON)) {
            func_mp3_res_play(RES_BUF_CAMERA_ON_MP3, RES_LEN_CAMERA_ON_MP3);
        }
    #endif

    #if WARNING_BT_HID_MENU
        if (func_bt_chkclr_warning(BT_WARN_HID_DISCON)) {
            func_mp3_res_play(RES_BUF_CAMERA_OFF_MP3, RES_LEN_CAMERA_OFF_MP3);
        }
    #endif

    #if BT_HID_DISCON_DEFAULT_EN
        if (f_bt.hid_discon_flag) {
            if (bt_hid_is_ready_to_discon()) {
                f_bt.hid_discon_flag = 0;
                bt_hid_disconnect();
            }
        }
    #endif // BT_HID_DISCON_DEFAULT_EN
    }
#endif // BT_HID_MENU_EN
}

void func_bt_disp_status(void)
{
    uint status = bt_get_disp_status();

    if(f_bt.disp_status != status) {
        f_bt.disp_status = status;
        f_bt.sta_update = 1;
        if(!bt_is_connected()) {
            en_auto_pwroff();
            sys_cb.sleep_en = BT_PAIR_SLEEP_EN;
        } else {
            dis_auto_pwroff();
            sys_cb.sleep_en = 1;
        }

        switch (f_bt.disp_status) {
        case BT_STA_CONNECTING:
            if (BT_RECONN_LED_EN) {
                led_bt_reconnect();
                break;
            }
        case BT_STA_INITING:
        case BT_STA_IDLE:
            led_bt_idle();
#if WARNING_BT_PAIR
            if(f_bt.need_pairing && f_bt.disp_status == BT_STA_IDLE) {
                f_bt.need_pairing = 0;
                if(xcfg_cb.warning_bt_pair && xcfg_cb.bt_tws_en) {
                    f_bt.warning_status |= BT_WARN_PAIRING;
                }
            }
#endif
            break;
        case BT_STA_SCANNING:
            led_bt_scan();
            break;

        case BT_STA_DISCONNECTING:
            led_bt_connected();
            break;

        case BT_STA_CONNECTED:
            led_bt_connected();
            break;
        case BT_STA_INCOMING:
            led_bt_ring();
            break;
        case BT_STA_PLAYING:
            led_bt_play();
            break;
        case BT_STA_OUTGOING:
        case BT_STA_INCALL:
            led_bt_call();
            break;
        }

        if(f_bt.disp_status >= BT_STA_CONNECTED) {
            f_bt.need_pairing = 1;
#if DAC_OFF_FOR_BT_CONN_EN
            func_bt_set_dac(1);
        } else {
            func_bt_set_dac(0);
#endif // DAC_OFF_FOR_BT_CONN_EN
        }
#if BT_BACKSTAGE_EN
        if (f_bt.disp_status < BT_STA_PLAYING && func_cb.sta_break != FUNC_NULL) {
            func_cb.sta = func_cb.sta_break;
        }
#endif
    }
}

AT(.text.func.bt)
void func_bt_status(void)
{
    func_bt_disp_status();

#if FUNC_BTHID_EN
    if(is_bthid_mode()) {
        func_bt_hid_warning();
    } else
#endif
    {
        func_bt_warning();
    }
}

AT(.text.func.bt)
void func_bt_sub_process(void)
{
    func_bt_status();
#if USER_TKEY_DEBUG_EN
    bsp_tkey_spp_tx();
#endif
}

AT(.text.func.bt)
void func_bt_process(void)
{
    func_process();
    func_bt_sub_process();

    if(f_bt.disp_status == BT_STA_INCOMING) {
#if BT_HFP_RING_NUMBER_EN
        sfunc_bt_ring();
#endif
        reset_sleep_delay();
        reset_pwroff_delay();
        f_bt.siri_kl_flag = 0;
        f_bt.user_kl_flag = 0;
#if BT_REC_EN
        bt_music_rec_continue();
#endif
    } else if(f_bt.disp_status == BT_STA_OTA) {

#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif

        sfunc_bt_ota();
        reset_sleep_delay();
        reset_pwroff_delay();
    } else if(f_bt.disp_status >= BT_STA_OUTGOING) {
        sfunc_bt_call();
        reset_sleep_delay();
        reset_pwroff_delay();
        f_bt.siri_kl_flag = 0;
        f_bt.user_kl_flag = 0;
#if BT_REC_EN
        bt_music_rec_continue();
#endif
    }

    if(sys_cb.pwroff_delay == 0) {
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;
        return;
    }
    if(sleep_process(bt_is_sleep)) {
        f_bt.disp_status = 0xff;
    }
}

AT(.text.func.bt)
void func_bt_init(void)
{
    if (!f_bt.bt_is_inited) {
        msg_queue_clear();
        bsp_bt_init();
        f_bt.bt_is_inited = 1;
    }
}

AT(.text.func.bt)
void func_bt_chk_off(void)
{
    if ((func_cb.sta != FUNC_BT) && (f_bt.bt_is_inited)) {
#if BT_PWRKEY_5S_DISCOVER_EN
        bsp_bt_pwrkey5s_clr();
#endif
        bt_disconnect(1);
        bt_off();
        f_bt.bt_is_inited = 0;
    }
}

AT(.text.func.bt)
void func_bt_enter(void)
{
#if LED_MATRIX_HUART_TX
    huart_tx_mode(1);
#endif

#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_BTMIC);
#endif
    func_cb.mp3_res_play = func_bt_mp3_res_play;

#if WARNING_FUNC_BT
    mp3_res_play(RES_BUF_BT_MODE_MP3, RES_LEN_BT_MODE_MP3);
#endif // WARNING_FUNC_BT

#if WARNING_BT_WAIT_CONNECT
    mp3_res_play(RES_BUF_WAIT4CONN_MP3, RES_LEN_WAIT4CONN_MP3);
#endif // WARNING_BT_WAIT_CONNECT

    func_bt_enter_display();
    led_bt_init();
    dis_auto_pwroff();
    func_bt_init();
    f_bt.disp_status = 0xfe;
    f_bt.rec_pause = 0;
    f_bt.pp_2_unmute = 0;
    sys_cb.key2unmute_cnt = 0;

    bt_redial_init();
    bt_audio_enable();
#if DAC_DNR_EN
    dac_dnr_set_sta(1);
    sys_cb.dnr_sta = 1;
#endif

#if BT_PWRKEY_5S_DISCOVER_EN
    if(bsp_bt_pwrkey5s_check()) {
        f_bt.need_pairing = 0;  //已经播报了
        func_bt_disp_status();
#if WARNING_BT_PAIR
        func_mp3_res_play(RES_BUF_PAIRING_MP3, RES_LEN_PAIRING_MP3);
#endif
    } else {
        func_bt_disp_status();
#if WARNING_BT_PAIR
        if (xcfg_cb.warning_bt_pair && !xcfg_cb.bt_tws_en) {
            func_mp3_res_play(RES_BUF_PAIRING_MP3, RES_LEN_PAIRING_MP3);
        }
#endif // WARNING_BT_PAIR
    }
#endif

#if BT_REC_EN
    bt_music_rec_init();
#endif // BT_REC_EN

#if BT_TWS_EN
    if(xcfg_cb.bt_tws_pair_mode > 1) {
        bt_tws_set_scan(1, 1);
    }
#endif

#if BT_POWER_OPTIMIZE
    vddio_voltage_configure();  //该函数会回调getcfg_vddio_sel,在蓝牙下把VDDIO设置为2.7V左右
#endif

#if DAC_OFF_FOR_BT_CONN_EN
    f_bt.dac_sta = 1;
    func_bt_set_dac(0);
#endif // DAC_OFF_FOR_BT_CONN_EN

#if MICAUX_ANALOG_OUT_ALWAYS
    micaux_analog_out_init();
#endif // MICAUX_ANALOG_OUT_ALWAYS

#if SYS_KARAOK_EN
    if (f_bt.disp_status < BT_STA_INCOMING) {
        bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
    }
#endif

}

AT(.text.func.bt)
void func_bt_exit(void)
{
#if BT_REC_EN
    sfunc_record_stop();
#endif // BT_REC_EN

    dac_fade_out();
#if DAC_DNR_EN
    dac_dnr_set_sta(0);
    sys_cb.dnr_sta = 0;
#endif
#if BT_PWRKEY_5S_DISCOVER_EN
    bsp_bt_pwrkey5s_clr();
#endif
    func_bt_exit_display();
    bt_audio_bypass();
#if BT_TWS_EN
    dac_mono_init(1, 0);
#endif
#if !BT_BACKSTAGE_EN
    bt_disconnect(1);
    bt_off();
    f_bt.bt_is_inited = 0;
#else
    if (bt_get_status() == BT_STA_PLAYING && !bt_is_testmode()) {        //蓝牙退出停掉音乐
        delay_5ms(10);
        if(bt_get_status() == BT_STA_PLAYING) {     //再次确认play状态
            u32 timeout = 850; //8.5s
            bt_music_pause();
            while (bt_get_status() == BT_STA_PLAYING && timeout > 0) {
                timeout--;
                delay_5ms(2);
            }
        }
    }
#endif

#if BT_RF_POWER_BALANCE_EN
    btrf_power_balance_exit();
#endif
    f_bt.rec_pause = 0;
    f_bt.pp_2_unmute = 0;
    sys_cb.key2unmute_cnt = 0;
#if BT_POWER_OPTIMIZE
    vddio_voltage_configure();  //该函数会回调getcfg_vddio_sel,退出蓝牙时，恢复正常配置中的VDDIO电压
#endif
    func_cb.last = FUNC_BT;
#if DAC_OFF_FOR_BT_CONN_EN
    func_bt_set_dac(1);
#endif // DAC_OFF_FOR_BT_CONN_EN
}

AT(.text.func.bt)
void func_bt(void)
{
    printf("%s\n", __func__);

    func_bt_enter();

    while (func_cb.sta == FUNC_BT) {
        func_bt_process();
        func_bt_message(msg_dequeue());
        func_bt_display();
    }

    func_bt_exit();
}
#if 0
#define test_put    0
#if test_put

#define SCLK_GPIOCLR    GPIOBCLR
#define SCLK_GPIOSET    GPIOBSET
#define SCLK_BIT        BIT(1)

#define SDAT_GPIOCLR    GPIOBCLR
#define SDAT_GPIOSET    GPIOBSET
#define SDAT_BIT        BIT(0)
void my_spi_init_rec(void)
{
    GPIOBDE |=BIT(0)|BIT(1)|BIT(2);
    GPIOBFEN &= ~(BIT(0)|BIT(1)|BIT(2));
    GPIOBDIR &= ~(BIT(0)|BIT(1)|BIT(2));
    GPIOBSET = BIT(0)|BIT(1)|BIT(2);

//    GPIOADE |=BIT(3)|BIT(4);
//    GPIOAFEN &= ~(BIT(3)|BIT(4));
//    GPIOADIR &= ~(BIT(3)|BIT(4));
//    GPIOASET = BIT(3)|BIT(4);

//    GPIOFDE  |= SDAT_BIT|SCLK_BIT;
//    GPIOFFEN &= ~(SDAT_BIT|SCLK_BIT);
//    GPIOFDIR &= ~(SDAT_BIT|SCLK_BIT);
//    GPIOFSET  = SDAT_BIT|SCLK_BIT;
}
AT(.com_text.putc)
void my_spi_putc_rec(u8 ch)
{
    GLOBAL_INT_DISABLE();
    u8 i;
    for(i=0; i<8; i++) {
        SCLK_GPIOCLR = SCLK_BIT;
//        GPIOBCLR = BIT(0);
        if(ch & 0x80) {
            SDAT_GPIOSET = SDAT_BIT;
//            GPIOBSET = BIT(1);
        } else {
            SDAT_GPIOCLR = SDAT_BIT;
//            GPIOBCLR = BIT(1);
        }
        ch <<= 1;
        SCLK_GPIOSET = SCLK_BIT;
//        GPIOBSET = BIT(0);
    }
    GLOBAL_INT_RESTORE();
}
AT(.com_text.dac)
void put_spi_buf(u8 *ptr, u16 len)
{
    GPIOBCLR = BIT(2);

    for (u16 i = 0; i < len; i++) {
        my_spi_putc_rec(ptr[i]);
    }

    GPIOBSET = BIT(2);
}
#else
void my_spi_init_rec(void){}
void put_spi_buf(u8 *ptr, u16 len){}
#endif

//AT(.com_text.dac)
//void dac_src0_dma_out_proc(void *buf, uint len)           //用户定义DMA输出（使用DMA则不用one byte发送）
//{
    //put_spi_buf(buf, 128);
//}
#endif // 0
#endif //FUNC_BT_EN
