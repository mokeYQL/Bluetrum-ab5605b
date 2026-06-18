#include "include.h"

func_cb_t func_cb AT(.buf.func_cb);
#if UART_S_UPDATE
bool deal_update_uart_msg(void);
#endif // UART_S_UPDATE
extern void sys_mic_set_eq(void);
#if VBAT_DETECT_EN
AT(.text.func.msg)
void lowpower_vbat_process(void)
{
    int lpwr_vbat_sta = is_lowpower_vbat_warning();

    if (lpwr_vbat_sta == 1) {
#if WARNING_LOW_BATTERY
        func_mp3_res_play(RES_BUF_LOW_BATTERY_MP3, RES_LEN_LOW_BATTERY_MP3);
#endif
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;     //低电，进入关机或省电模式
        return;
    }
    if ((func_cb.mp3_res_play == NULL) || (lpwr_vbat_sta != 2)) {
        if ((sys_cb.lowbat_flag) && (sys_cb.vbat > 3800)) {
            sys_cb.vbat_nor_cnt++;
            if (sys_cb.vbat_nor_cnt > 40) {
                sys_cb.lowbat_flag = 0;
                sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
                plugin_lowbat_vol_recover();    //离开低电, 恢复音乐音量
            }
        }
        return;
    }

    //低电提示音播放
    sys_cb.vbat_nor_cnt = 0;
    if (sys_cb.lpwr_warning_cnt > xcfg_cb.lpwr_warning_period) {
        sys_cb.lpwr_warning_cnt = 0;
        if (sys_cb.lpwr_warning_times) {        //低电语音提示次数
            if (RLED_LOWBAT_FOLLOW_EN) {
                led_lowbat_follow_warning();
            }

            sys_cb.lowbat_flag = 1;
#if WARNING_LOW_BATTERY
            func_mp3_res_play(RES_BUF_LOW_BATTERY_MP3, RES_LEN_LOW_BATTERY_MP3);
#endif
            plugin_lowbat_vol_reduce();         //低电降低音乐音量

            if (RLED_LOWBAT_FOLLOW_EN) {
                while (get_led_sta(1)) {        //等待红灯闪完
                    delay_5ms(2);
                }
                led_lowbat_recover();
            }

            if (sys_cb.lpwr_warning_times != 0xff) {
                sys_cb.lpwr_warning_times--;
            }
        }
    }
}
#endif // VBAT_DETECT_EN


AT(.text.func.bt)
void printf_vol(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        ticks = tick_get();

#if EXLIB_BT_MONO_XDRC_EN
        //printf("--------------->, sys_clk = %d\n",get_cur_sysclk());
		//printf("AUBUFSIZE = 0x%X, FIFOCNT = %d\n",AUBUFSIZE,  (u16)AUBUFFIFOCNT);
        print_xdrc_audio_cfg();
        xdrc_softvol_print();
        printf("xdrc_prevol = %d\n",xdrc_prevol_get());
#endif
        printf("DACVOLCON = 0x%X , AUANGCON3 = 0x%X,\n", DACVOLCON&0x7FFF,AUANGCON3&0x7F);    //DACVOLCON 总的数字音量   //AUANGCON3低7BIT为模拟音量
//        printf("SRC0VOLCON = 0x%X, SRC1VOLCON = 0x%X\n",SRC0VOLCON&0x7FFF,SRC1VOLCON&0x7FFF);  //SRC0VOLCON是音乐音量  //SRC1VOLCON是MIC音量
//        printf("AU0LMIXCOEF = 0x%X, AU0RMIXCOEF = 0x%X\n",AU0LMIXCOEF,AU0RMIXCOEF);  //DACDIGCON0的BIT(7)设置为1时该寄存器有效
//        printf("AU1LMIXCOEF = 0x%X, AU1RMIXCOEF = 0x%X\n",AU1LMIXCOEF,AU1RMIXCOEF);  //DACDIGCON0的BIT(8)设置为1时该寄存器有效
//        printf("DACDIGCON0 & (BIT(7)|BIT(8)) = 0x%X\n\n",DACDIGCON0 & (BIT(7)|BIT(8)));
    }
}

void print_cache_lock_map(void);
AT(.text.func.process)
void print_info(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        ticks = tick_get();
        //printf("[%d]",get_karaok_mic_maxpow(0));
        //printf("DACVOLCON = 0x%X , AUANGCON3 = 0x%X,\n", DACVOLCON&0x7FFF,AUANGCON3&0x7F);  //print DVOL,AVOL
        //my_printf("dac_dnr_get_sta = %d\n",dac_dnr_get_sta());
        //printf("--------------->PWRCON2 = 0x%X, %d\n",PWRCON2,PWRCON2&0x1F);
        //printf("CLKCON0~4: 0_0x%X,1_0x%X,2_0x%X,3_0x%X,4_0x%X\n",CLKCON0,CLKCON1,CLKCON2,CLKCON3,CLKCON4);
    }
}

AT(.text.func.process)
void func_process(void)
{
    WDT_CLR();
    //printf_vol();
    //print_info();
#if BT_2ACL_EN
/*一拖二 点击手机播放可切换对应的手机播放 还需注释 avdtp_acl2_sbc_compare_buf 函数*/
//    static u8 play_change_flag = 0;
//    if(bt_play_data_check(750)){
//        if(!play_change_flag){
//            play_change_flag = 1;
//            bt_2acl_switch_music_play();
//        }
//    }else{
//        play_change_flag = 0;
//    }
#endif // BT_2ACL_EN
#if VBAT_DETECT_EN
    lowpower_vbat_process();
#endif // VBAT_DETECT_EN

#if BT_BACKSTAGE_EN
    if (func_cb.sta != FUNC_BT) {
        func_bt_warning();
        uint status = bt_get_status();
#if BT_BACKSTAGE_PLAY_DETECT_EN
        if (status >= BT_STA_PLAYING) {
#else
        if (status > BT_STA_PLAYING) {
#endif
            if(!bt_is_silence()) {
                func_cb.sta_break = func_cb.sta;
                func_cb.sta = FUNC_BT;
            }
        }
    }
#endif

    //PWRKEY模拟硬开关关机处理
    if ((PWRKEY_2_HW_PWRON) && (sys_cb.pwrdwn_hw_flag)) {
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;
        sys_cb.pwrdwn_hw_flag = 0;
    }

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(1);
    }
#endif // CHARGE_EN

#if SYS_KARAOK_EN
    karaok_process();
#endif

#if FOT_EN
    bsp_fot_process();
#endif

#if LE_EN
    bsp_ble_process();
#endif

    mp3_uart_update_process();
}

//func common message process
AT(.text.func.msg)
void func_message(u16 msg)
{
    switch (msg) {
        case KL_NEXT_VOL_UP:
        case KH_NEXT_VOL_UP:
        case KL_VOL_UP:
        case KH_VOL_UP:
            sys_cb.maxvol_fade = 1;
        case KU_VOL_UP_NEXT:
        case KU_VOL_UP:
        case KU_VOL_NEXT_PREV:
            if(bsp_bt_hid_vol_change(HID_KEY_VOL_UP)){
                printf("HID vol up\n");
            }else{
                if (msg == KU_VOL_NEXT_PREV) {
                    //循环音量加, 最大音量后提示下, 然后从0开始。
                    if (sys_cb.vol == VOL_MAX) {
                        bsp_set_volume(0);
                    } else {
                        bsp_set_volume(bsp_volume_inc(sys_cb.vol));
                    }
                } else {
                    bsp_set_volume(bsp_volume_inc(sys_cb.vol));
                }
                func_set_vol_callback(1);
                bsp_bt_vol_change();
            }
            printf("current volume: %d (DVOL=0x%X,AVOL=0x%X)\n", sys_cb.vol, DACVOLCON&0x7FFF,AUANGCON3&0x7F);
#if WARNING_MAX_VOLUME
            if (sys_cb.vol == VOL_MAX) {
                maxvol_tone_play();
            }
#endif // WARNING_MAX_VOLUME
            break;

        case KLU_VOL_UP:
        case KLU_NEXT_VOL_UP:
            if (sys_cb.maxvol_fade == 2) {
                dac_fade_in();
            }
            sys_cb.maxvol_fade = 0;
            break;

        case KL_PREV_VOL_DOWN:
        case KH_PREV_VOL_DOWN:
        case KU_VOL_DOWN_PREV:
        case KU_VOL_DOWN:
        case KL_VOL_DOWN:
        case KH_VOL_DOWN:
            if(bsp_bt_hid_vol_change(HID_KEY_VOL_DOWN)){
                printf("HID vol down\n");
            }else{
                bsp_set_volume(bsp_volume_dec(sys_cb.vol));
                func_set_vol_callback(0);
                bsp_bt_vol_change();
                printf("current volume: %d (DVOL=0x%X,AVOL=0x%X)\n", sys_cb.vol, DACVOLCON&0x7FFF,AUANGCON3&0x7F);
            }
#if WARNING_MIN_VOLUME
            if (sys_cb.vol == 0) {
                func_mp3_res_play(RES_BUF_MAX_VOL_MP3, RES_LEN_MAX_VOL_MP3);
            }
#endif // WARNING_MIN_VOLUME
            break;

#if SOFT_POWER_ON_OFF
        //长按PP/POWER软关机(通过PWROFF_PRESS_TIME控制长按时间)
        case KLH_PLAY_POWER:
        case KLH_MODE_POWER:
        case KLH_HSF_POWER:
        case KLH_POWER:
            sys_cb.pwrdwn_tone_en = 1;
            func_cb.sta = FUNC_PWROFF;
            break;
#endif // SOFT_POWER_ON_OFF

#if IRRX_HW_EN
        case KU_IR_POWER:
            func_cb.sta = FUNC_SLEEPMODE;
            break;
#endif

        case KU_MODE:
        case KU_MODE_POWER:
        case KL_PLAY_MODE:
            func_cb.sta = FUNC_NULL;
            break;

#if EQ_MODE_EN
        case KU_EQ:
            sys_set_eq();
//            sys_mic_set_eq();
            break;
#endif // EQ_MODE_EN

        case KU_MUTE:
            if (sys_cb.mute) {
                bsp_sys_unmute();
            } else {
                bsp_sys_mute();
            }
            break;

#if SYS_KARAOK_EN
#if KARAOK_RM_VOICE
        case KU_VOICE_RM:
            karaok_voice_rm_switch();
            printf("KU_VOICE_RM: %d\n",karaok_get_voice_rm_sta());
            break;
#endif
#if SYS_MAGIC_VOICE_EN
        case KL_VOICE_RM:
            magic_voice_switch();
            break;
#endif
#endif

        case MSG_SYS_500MS:
            break;

#if MUSIC_UDISK_EN
        case EVT_UDISK_INSERT:
            if (dev_is_online(DEV_UDISK)) {
#if SD_USB_MUX_IO_EN
                sys_cb.cur_dev = DEV_UDISK;
#endif // SD_USB_MUX_IO_EN
                if (dev_udisk_activation_try(0)) {
                    sys_cb.cur_dev = DEV_UDISK;
                    func_cb.sta = FUNC_MUSIC;
                }
            }
            break;
#endif // MUSIC_UDISK_EN

#if MUSIC_SDCARD_EN
        case EVT_SD_INSERT:
            if (dev_is_online(DEV_SDCARD)) {
                sys_cb.cur_dev = DEV_SDCARD;
                func_cb.sta = FUNC_MUSIC;
            }
            break;
#endif // MUSIC_SDCARD_EN

#if MUSIC_SDCARD1_EN
        case EVT_SD1_INSERT:
            if (dev_is_online(DEV_SDCARD1)) {
                sys_cb.cur_dev = DEV_SDCARD1;
                func_cb.sta = FUNC_MUSIC;
            }
            break;
#endif // MUSIC_SDCARD1_EN

#if FUNC_USBDEV_EN
        case EVT_PC_INSERT:
            if (dev_is_online(DEV_USBPC)) {
                func_cb.sta = FUNC_USBDEV;
            }
            break;
#endif // FUNC_USBDEV_EN

#if LINEIN_DETECT_EN
        case EVT_LINEIN_INSERT:
            if (dev_is_online(DEV_LINEIN)) {
#if LINEIN_2_PWRDOWN_EN
                sys_cb.pwrdwn_tone_en = LINEIN_2_PWRDOWN_TONE_EN;
                func_cb.sta = FUNC_PWROFF;
#else
                func_cb.sta = FUNC_AUX;
#endif // LINEIN_2_PWRDOWN_EN
            }
            break;
#endif // LINEIN_DETECT_EN


#if FUNC_HDMI_EN
        case EVT_HDMI_INSERT:
            if (dev_is_online(DEV_HDMI)) {
        		func_cb.sta = FUNC_HDMI;
        		bsp_hdmi_cec_init();
        		cec_rx_start();   //提前开始接收，及时回电视ack信息
            }
            break;

        case EVT_HDMI_REMOVE:
            break;
#endif

        case EVT_TWS_SET_VOL:
        case EVT_A2DP_SET_VOL:
            if(bt_get_status() != BT_STA_INCALL) {
                printf("A2DP SET VOL: %d\n", sys_cb.vol);
            #if SYS_KARAOK_EN
                a2dp_set_vol(sys_cb.vol);
            #else
                bsp_change_volume(sys_cb.vol);
            #endif
                gui_box_show_vol();
                param_sys_vol_write();
                sys_cb.cm_times = 0;
                sys_cb.cm_vol_change = 1;
            }
            break;

        case EVT_UDE_SET_VOL:
            bsp_change_volume(sys_cb.vol);
            gui_box_show_vol();
            param_sys_vol_write();
            sys_cb.cm_times = 0;
            sys_cb.cm_vol_change = 1;
            break;

        case EVT_BT_SCAN_START:
            if (bt_get_status() < BT_STA_SCANNING) {
                bt_scan_enable();
            }
            break;
#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
        case EVT_ONLINE_SET_EQ:
#if EXLIB_SOFT_EQ_DRC_EN
            sed_adj_parse_cmd_equalizer();
#elif EXLIB_BT_MONO_XDRC_EN
            mono_xdrc_onlie_adjust_process();
#else
            eq_parse_cmd();
#endif
            break;
#endif
#if SYS_KARAOK_EN
        case EVT_ECHO_LEVEL:
//            printf("echo level:%x\n", sys_cb.echo_level);
            bsp_echo_set_level();
            break;

        case EVT_MIC_VOL:
//            printf("mic vol:%x\n", sys_cb.mic_vol);
            bsp_karaok_set_mic_volume();
            break;

        case EVT_MUSIC_VOL:
//            printf("music vol:%x\n", sys_cb.music_vol);
            bsp_karaok_set_music_volume();
            break;
#endif
#if LANG_SELECT == LANG_EN_ZH
        case EVT_BT_SET_LANG_ID:
            param_lang_id_write();
            param_sync();
            break;
#endif

#if EQ_MODE_EN
        case EVT_BT_SET_EQ:
            music_set_eq_by_num(sys_cb.eq_mode);
            break;
#endif

#if MIC_DETECT_EN
        case EVT_MIC_INSERT:
            karaok_mic_unmute();
            break;

        case EVT_MIC_REMOVE:
            karaok_mic_mute();
            break;
#endif

#if UART_S_UPDATE
        case EVT_UART_UPDATE:
            if(deal_update_uart_msg()) {
                func_cb.sta = FUNC_NULL;
            }
            break;
#endif
    }

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 6)) {
        sys_cb.cm_vol_change = 0;
        cm_sync();
    }
#if SD_SOFT_DETECT_EN
    sd_soft_cmd_detect(120);
#endif
}

///进入一个功能的总入口
AT(.text.func)
void func_enter(void)
{
    gui_box_clear();
    param_sync();
    reset_sleep_delay();
    reset_pwroff_delay();
    func_cb.mp3_res_play = NULL;
    func_cb.set_vol_callback = NULL;
    bsp_clr_mute_sta();
    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。
    AMPLIFIER_SEL_D();
#if SYS_KARAOK_EN
    karaok_voice_rm_disable();
    bsp_karaok_echo_reset_buf(func_cb.sta);
#endif
#if VUSB_ADCCH
    ADCCH_VUSB_EN();
#endif
}

AT(.text.func)
void func_exit(void)
{
#if UART_S_UPDATE
    void uart_s_update(void);
    uart_s_update();
#endif // UART_S_UPDATE
    u8 func_num;
    u8 funcs_total = get_funcs_total();

    for (func_num = 0; func_num != funcs_total; func_num++) {
        if (func_cb.last == func_sort_table[func_num]) {
            break;
        }
    }
    func_num++;                                     //切换到下一个任务
    if (func_num >= funcs_total) {
        func_num = 0;
    }
    func_cb.sta = func_sort_table[func_num];        //新的任务
#if SYS_MODE_BREAKPOINT_EN
    param_sys_mode_write(func_cb.sta);
#endif // SYS_MODE_BREAKPOINT_EN
}

AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);
//    func_cb.sta = FUNC_SPEAKER;
#if !BT_BACKSTAGE_EN && FUNC_BT_EN
    func_bt_chk_off();
#endif
    while (1) {
        func_enter();
        switch (func_cb.sta) {
#if FUNC_MUSIC_EN
        case FUNC_MUSIC:
            func_music();
            break;
#endif // FUNC_MUSIC_EN

#if FUNC_FMRX_EN
        case FUNC_FMRX:
            func_fmrx();
            break;
#endif // FUNC_FMRX_EN

#if EX_SPIFLASH_SUPPORT && !FUNC_TOM_CAT_EN
        case FUNC_EXSPIFLASH_MUSIC:
            func_exspifalsh_music();
            break;
#endif

#if FUNC_TOM_CAT_EN
        case FUNC_TOM_CAT:
            func_tom_cat();
            break;
#endif

#if FUNC_CLOCK_EN
        case FUNC_CLOCK:
            func_clock();
            break;
#endif // FUNC_CLOCK_EN

#if FUNC_BT_EN
        case FUNC_BT:
            func_bt();
            break;
#endif

#if FUNC_BTHID_EN
        case FUNC_BTHID:
            func_bthid();
            break;
#endif // FUNC_BTHID_EN

#if FUNC_BT_DUT_EN
        case FUNC_BT_DUT:
            func_bt_dut();
            break;
#endif
#if FUNC_BT_FCC_EN
        case FUNC_BT_FCC:
            func_bt_fcc();
            break;
#endif

#if FUNC_AUX_EN
        case FUNC_AUX:
            func_aux();
            break;
#endif // FUNC_AUX_EN

#if FUNC_USBDEV_EN
        case FUNC_USBDEV:
            func_usbdev();
            break;
#endif

#if FUNC_SPDIF_EN
        case FUNC_SPDIF:
            func_spdif();
            break;
#endif

#if FUNC_HDMI_EN
        case FUNC_HDMI:
            func_hdmi();
			break;
#endif

#if FUNC_SPEAKER_EN
        case FUNC_SPEAKER:
            func_speaker();
            break;
#endif // FUNC_SPEAKER_EN

#if FUNC_I2S_EN
        case FUNC_I2S:
            func_i2s();
            break;
#endif

#if FUNC_HUART_EN
        case FUNC_HUART:
            func_huart();
            break;
#endif

#if FUNC_IDLE_EN
        case FUNC_IDLE:
            func_idle();
            break;
#endif // FUNC_IDLE_EN

        case FUNC_PWROFF:
            func_pwroff(sys_cb.pwrdwn_tone_en);
            break;

        case FUNC_SLEEPMODE:
            func_sleepmode();
            break;

        default:
            func_exit();
            break;
        }
    }
}
