#include "include.h"

extern u32 __comm_start, __comm_end;
extern u32 __bss_size, __bss_end, __bss_start;

xcfg_cb_t       xcfg_cb;
sys_cb_t sys_cb AT(.buf.bsp.sys_cb);
uint8_t         sdadc_buf_en = SYS_KARAOK_EN & MUSIC_WAV_SUPPORT;
volatile int    micl2gnd_flag;
volatile u32    ticks_50ms;

#if USB_DETECT_ONLY_CHECK_HOST
u8   usbchk_host_connect(void);
bool usb_detect_only_check_host_en(void)
{
    return true;
}
#endif // USB_DETECT_ONLY_CHECK_HOST

uint8_t cfg_spiflash_speed_up_en = SPIFLASH_SPEED_UP_EN; // SPI FLASH提速。部份FLASH不支持提速

extern volatile bool wav_output_en;
void                 wav_output_tick_start(void);
void                 sd_detect(void);
void                 tbox_uart_isr(void);
void                 testbox_init(void);
bool                 exspiflash_init(void);
void                 ledseg_6c6s_clr(void);
void                 dac_to_spi_enable(bool en);
#if AUDIO_STRETCH_EN
void bsp_stretch_init(void);
#endif

#if SD_SOFT_DETECT_EN
AT(.text.bsp.sys.init)
void sd_soft_detect_poweron_check(void) // 开机检测
{
    if (!SD_IS_SOFT_DETECT()) { // 配置工具中是否配置sd检测.
        return;
    }
    dev_delay_times(DEV_SDCARD, 1); // 检测到1次成功, 则认为SD在线.
    u8 i = 5;
    while (i--) {
        sd_soft_cmd_detect(0);
        if (dev_is_online(DEV_SDCARD)) {
            break;
        }
        delay_ms(10);
    }
    dev_delay_times(DEV_SDCARD, 3);
}

AT(.com_text.detect)
void sd_soft_cmd_detect(u32 check_ms) // check_ms 时间间隔检测一次.  //主循环中执行检测.
{
    static u32 check_ticks = 0;
    if (!SD_IS_SOFT_DETECT()) { // 配置工具中是否配置sd检测.
        return;
    }
    if (tick_check_expire(check_ticks, check_ms) || (0 == check_ticks)) { // 每隔100ms才检测一次.
        check_ticks = tick_get();
    } else {
        return;
    }
    if (sd_soft_detect()) {
        if (dev_online_filter(DEV_SDCARD)) {
            sd_insert();
            msg_enqueue(EVT_SD_INSERT);
            my_printf("\nsd soft insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_SDCARD)) {
            sd_remove();
            msg_enqueue(EVT_SD_REMOVE);
            my_printf("\nsd soft remove\n");
        }
    }
}
#endif // SD_SOFT_DETECT_EN

#define SD_DETECT_DEBUG 0 // SD插拔打印
#if SD_DETECT_DEBUG
AT(.com_text.str_sddet)
const char str_sd_insert[] = "==>sd insert\n";
AT(.com_text.str_sddet)
const char str_sd_remove[] = "==>sd remove\n";
#endif

#if MUSIC_SDCARD_EN
AT(.com_text.detect)
void sd_detect(void)
{
    if ((!is_sd_support()) || (IS_DET_SD_BUSY())) {
        return;
    }
    #if SD_SOFT_DETECT_EN
    if (SD_IS_SOFT_DETECT()) {
        return;
    }
    #endif // SD_SOFT_DETECT_EN
    if (SD_IS_ONLINE()) {
        if (dev_online_filter(DEV_SDCARD)) {
            sd_insert();
            msg_enqueue(EVT_SD_INSERT);
    #if SD_DETECT_DEBUG
            printf(str_sd_insert);
    #endif
        }
    } else {
        if (dev_offline_filter(DEV_SDCARD)) {
            sd_remove();
            msg_enqueue(EVT_SD_REMOVE);
    #if SD_DETECT_DEBUG
            printf(str_sd_remove);
    #endif
        }
    }
}
#endif // MUSIC_SDCARD_EN

#if MUSIC_SDCARD1_EN
AT(.com_text.detect)
void sd1_detect(void)
{
    if ((!is_sd1_support()) || (IS_DET_SD1_BUSY())) {
        return;
    }
    if (SD1_IS_ONLINE()) {
        if (dev_online_filter(DEV_SDCARD1)) {
            sd1_insert();
            msg_enqueue(EVT_SD1_INSERT);
            //            printf("sd1 insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_SDCARD1)) {
            sd1_remove();
            msg_enqueue(EVT_SD1_REMOVE);
            //            printf("sd1 remove\n");
        }
    }
}
#endif // MUSIC_SDCARD_EN

#if SD_USB_MUX_IO_EN
static u8 usb_chk_sta = 0;
void      get_usb_chk_sta_convert(void)
{
    while (usb_chk_sta) {
        WDT_CLR();
    }
}
#endif

AT(.com_text.detect)
u8 get_usbtf_muxio(void)
{
#if SD_USB_MUX_IO_EN
    return 1;
#else
    return 0;
#endif
}

u8 cfg_usbchk_custom_en = USB_CUSTOM_DETECT;

#define USB_DETECT_DEBUG 0 // USB插拔打印
#if USB_DETECT_DEBUG
// AT(.com_text.str_usbdetdbg)
// const char str_usb_detect[] = "USB STA:%d\n";
AT(.com_text.str_usbdet)
const char str_udisk_insert[] = "==>udisk insert\n";
AT(.com_text.str_usbdet)
const char str_udisk_remove[] = "==>udisk remove\n";
AT(.com_text.str_usbdet)
const char str_pc_insert[] = "==>pc insert\n";
AT(.com_text.str_usbdet)
const char str_pc_remove[] = "==>pc remove\n";
#endif

#if USB_SUPPORT_EN
AT(.com_text.detect)
void usb_detect(void)
{
    if (!is_usb_support()) {
        return;
    }

    #if SD_USB_MUX_IO_EN
    if (is_det_sdcard_busy()) {
        return;
    }
        #if (SD0_MAPPING == SD0MAP_G6 || SD0_MAPPING == SD0MAP_G5 || SD0_MAPPING == SD0MAP_G4)
    usb_chk_sta = 1;
    FUNCMCON0   = 0x0f;              // 关SD0 Mapping
    USBCON1     = BIT(19) | BIT(17); // DM,DP pull down 15k enable
    SD_DAT_DIS_UP();
    delay_us(1);
        #endif
    #endif

    #if FUNC_USBDEV_EN
        #if USB_DETECT_ONLY_CHECK_HOST
    u8 usb_sta = usbchk_host_connect();
        #else
    u8 usb_sta = usbchk_connect(USBCHK_OTG);
        #endif // USB_DETECT_ONLY_CHECK_HOST
    #else
    u8 usb_sta = usbchk_connect(USBCHK_ONLY_HOST);
    #endif

    #if SD_USB_MUX_IO_EN && (SD0_MAPPING == SD0MAP_G6 || SD0_MAPPING == SD0MAP_G5 || SD0_MAPPING == SD0MAP_G4)

    if (sys_cb.cur_dev != DEV_UDISK) {
        SD_DAT_RES_UP();
        USBCON1 &= ~(BIT(17) | BIT(18) | BIT(19));
        FUNCMCON0 = SD0_MAPPING;
        delay_us(1);
    }
    usb_chk_sta = 0;

    #endif
    //	printf(str_usb_detect, usb_sta);

    if (usb_sta == USB_UDISK_CONNECTED) {
        if (dev_online_filter(DEV_UDISK)) {
            udisk_insert();
            msg_enqueue(EVT_UDISK_INSERT);
    #if USB_DETECT_DEBUG
            printf(str_udisk_insert);
    #endif
        }
    } else {
        if (dev_offline_filter(DEV_UDISK)) {
            udisk_remove();
            msg_enqueue(EVT_UDISK_REMOVE);
    #if USB_DETECT_DEBUG
            printf(str_udisk_remove);
    #endif
        }
    }
    #if FUNC_USBDEV_EN
    if (usb_sta == USB_PC_CONNECTED) {
        if (dev_online_filter(DEV_USBPC)) {
            msg_enqueue(EVT_PC_INSERT);
        #if USB_DETECT_DEBUG
            printf(str_pc_insert);
        #endif
        }
    } else {
        if (dev_offline_filter(DEV_USBPC)) {
            msg_enqueue(EVT_PC_REMOVE);
            pc_remove();
        #if USB_DETECT_DEBUG
            printf(str_pc_remove);
        #endif
        }
    }
    #endif
    #if USB_CUSTOM_DETECT
    if (usb_sta == USB_DP_GND_CONNECTED) {
        if (dev_online_filter(DEV_DP_GND)) {
            msg_enqueue(EVT_DP_GND_INSERT);
            // printf("USB_DP_GND insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_DP_GND)) {
            msg_enqueue(EVT_DP_GND_REMOVE);
            // printf("USB_DP_GND remove\n");
        }
    }

    if (usb_sta == USB_DPDM_SC_CONNECTED) {
        if (dev_online_filter(DEV_DPDM_SC)) {
            msg_enqueue(EVT_DPDM_SC_INSERT);
            // printf("USB_DPDM_SC insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_DPDM_SC)) {
            msg_enqueue(EVT_DPDM_SC_REMOVE);
            // printf("USB_DPDM_SC remove\n");
        }
    }
    #endif
}
#endif // USB_SUPPORT_EN

#if LINEIN_DETECT_EN
AT(.com_text.detect)
void linein_detect(void)
{
    if (IS_DET_LINEIN_BUSY()) {
        return;
    }
    if (LINEIN_IS_ONLINE()) {
        if (dev_online_filter(DEV_LINEIN)) {
            msg_enqueue(EVT_LINEIN_INSERT);
            //            printf("linein insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_LINEIN)) {
            msg_enqueue(EVT_LINEIN_REMOVE);
            //            printf("linein remove\n");
        }
    }
}
#endif // LINEIN_DETECT_EN

#if EARPHONE_DETECT_EN
AT(.com_text.detect)
void earphone_detect(void)
{
    if (IS_DET_EAR_BUSY()) {
        return;
    }
    if (EARPHONE_IS_ONLINE()) {
        if (dev_online_filter(DEV_EARPHONE)) {
        }
    } else {
        if (dev_offline_filter(DEV_EARPHONE)) {
        }
    }

    if (dev_is_online(DEV_EARPHONE)) {
        LOUDSPEAKER_MUTE();
    } else {
        if (!sys_cb.loudspeaker_mute) {
            LOUDSPEAKER_UNMUTE();
        }
    }
}
#endif // EARPHONE_DETECT_EN

#if MIC_DETECT_EN
AT(.com_text.detect)
void mic_detect(void)
{
    if (IS_DET_MIC_BUSY()) {
        return;
    }
    if (MIC_IS_ONLINE()) {
        if (dev_online_filter(DEV_MIC)) {
            msg_enqueue(EVT_MIC_INSERT);
            //            printf("mic insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_MIC)) {
            msg_enqueue(EVT_MIC_REMOVE);
            //            printf("mic remove\n");
        }
    }
}
#endif // EARPHONE_DETECT_EN

#if BT_PWRKEY_5S_DISCOVER_EN
AT(.com_text.detect)
void pwrkey_5s_on_check(void)
{
    if (!xcfg_cb.bt_pwrkey_nsec_discover) {
        return;
    }
    if (sys_cb.pwrkey_5s_check) {
        if (0 == (RTCCON & BIT(19))) {
            if (tick_check_expire(sys_cb.ms_ticks, 1000 * xcfg_cb.bt_pwrkey_nsec_discover)) {
                sys_cb.pwrkey_5s_flag  = 1;
                sys_cb.pwrkey_5s_check = 0;
            }
        } else {
            sys_cb.pwrkey_5s_check = 0;
        }
    }
}
#endif // BT_PWRKEY_5S_DISCOVER_EN

#if PWRKEY_2_HW_PWRON
// 软开机模拟硬开关，松开PWRKEY就关机。
AT(.com_text.detect)
void pwrkey_2_hw_pwroff_detect(void)
{
    static int off_cnt = 0;
    if (!PWRKEY_2_HW_PWRON) {
        return;
    }

    if (IS_PWRKEY_PRESS()) {
        off_cnt = 0;
    } else {
        if (off_cnt < 10) {
            off_cnt++;
        } else if (off_cnt == 10) {
            // pwrkey已松开，需要关机
            off_cnt               = 20;
            sys_cb.pwrdwn_hw_flag = 1;
            sys_cb.poweron_flag   = 0;
        }
    }
}
#endif // PWRKEY_2_HW_PWRON

#define DAC_FIFIO_CHECK 0

#if DAC_FIFIO_CHECK
AT(.com_text.str_dbg)
const char str_aufifo_sta[] = "FIFO_1S[%d, %d]\n";
AT(.com_text.str_dbg)
const char str_aufifo_empty[] = "!!!!!!!empty = %d,";
// timer tick interrupt(1ms)
AT(.com_text.timer)
void dacbuf_1ms_check(void)
{
    static u32 empty_cnt    = 0;
    static u32 ticks        = 0;
    static u16 min_fifo_cnt = 0;
    static u16 max_fifo_cnt = 0;
    u16        fifo_cnt     = (u16)AUBUFFIFOCNT;

    if (fifo_cnt < 10) { // DACBUF播空时,打印其值为3
        empty_cnt++;
    }
    if (max_fifo_cnt < fifo_cnt) {
        max_fifo_cnt = fifo_cnt;
    }

    if (min_fifo_cnt > fifo_cnt) {
        min_fifo_cnt = fifo_cnt;
    }
    if (tick_check_expire(ticks, 1000)) { // 统计1S内检测到有多少次dac_buf为空
        ticks = tick_get();
        if (empty_cnt) {
            printf(str_aufifo_empty, empty_cnt);
        }
        printf(str_aufifo_sta, min_fifo_cnt, max_fifo_cnt);

        min_fifo_cnt = 0xFFFF;
        max_fifo_cnt = 0;
        empty_cnt    = 0;
    }
}
#endif

// timer tick interrupt(1ms)
AT(.com_text.timer)
void usr_tmr1ms_isr(void)
{
#if DAC_FIFIO_CHECK
    dacbuf_1ms_check();
#endif

#if USER_TKEY_MUL_EN
    TKACON |= BIT(31); // TKEY kick start.
#endif
#if (GUI_SELECT & DISPLAY_LEDSEG)
    gui_scan(); // 7P屏按COM扫描时，1ms间隔
#endif

#if FUNC_FMRX_EN
    fmrx_tmr1ms_isr();
#endif // FUNC_FMRX_EN

#if LED_DISP_EN
    port_2led_scan();
#endif // LED_DISP_EN

    plugin_tmr1ms_isr();
#if USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif
}

// timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_isr(void)
{
    tmr5ms_cnt++;
    // 5ms timer process
    dac_fade_process();

#if WAV_SRC1_MIX_PLAY_EN
    wav_src1_mix_5ms_isr();
#endif

#if EXFLASH_MUSIC_WARNING
    if (wav_output_en) {
        wav_output_tick_start();
    }
#endif
#if USER_TKEY_MUL_EN
    void bsp_tkey_change_channel(void);
    bsp_tkey_change_channel();
#endif
#if !USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif
#if PWRKEY_2_HW_PWRON
    pwrkey_2_hw_pwroff_detect();
#endif

#if HDMI_DETECT_EN && FUNC_HDMI_EN
    hdmi_detect();
#endif
    plugin_tmr5ms_isr();

#if MUSIC_SDCARD_EN
    sd_detect();
#endif // MUSIC_SDCARD_EN

#if MUSIC_SDCARD1_EN
    sd1_detect();
#endif // MUSIC_SDCARD1_EN

#if USB_SUPPORT_EN
    usb_detect();
#endif // USB_SUPPORT_EN

#if LINEIN_DETECT_EN
    linein_detect();
#endif // LINEIN_DETECT_EN

#if EARPHONE_DETECT_EN
    earphone_detect();
#endif // EARPHONE_DETECT_EN

#if MIC_DETECT_EN
    mic_detect();
#endif // MIC_DETECT_EN

    // 20ms timer process
    if ((tmr5ms_cnt % 4) == 0) {
#if DAC_DNR_EN
        dac_dnr_detect();
#endif // DAC_DNR_EN
    } else {
        // dac_pcm_pow_calc 需要避免和 dac_dnr_detect 在同一时间调用 (两个函数用到同一硬件模块)，否则能量值不准
        // dac_pcm_pow_calc 可以在这里调用，硬件计算能量,很快,us级别
        // dac_pwr = dac_pcm_pow_calc(); 调用示例
    }

    // 50ms timer process
    if ((tmr5ms_cnt % 10) == 0) {
        ticks_50ms++;
#if BT_PWRKEY_5S_DISCOVER_EN
        pwrkey_5s_on_check();
#endif // BT_PWRKEY_5S_DISCOVER_EN

#if FMTX_DNR_EN
        fmtx_dnr_process();
#endif
    }

#if BT_TWS_EN
    // SYNC 50ms timer process
    if (bt_sync_tick()) {
        led_scan();
        led_sync();
    }
#endif

    // 100ms timer process
    if ((tmr5ms_cnt % 20) == 0) {
        lowpwr_tout_ticks();
#if UDE_HID_EN
        if (func_cb.sta == FUNC_USBDEV) {
            ude_tmr_isr();
        }
#endif                 // UDE_HID_EN
        gui_box_isr(); // 显示控件计数处理

        if (sys_cb.lpwr_cnt > 0) {
            sys_cb.lpwr_cnt++;
        }

        if (sys_cb.key2unmute_cnt) {
            sys_cb.key2unmute_cnt--;
            if (!sys_cb.key2unmute_cnt) {
                msg_enqueue(EVT_KEY_2_UNMUTE);
            }
        }
#if FUNC_TOM_CAT_EN
        if (f_tom.rec_type == 1) {
            f_tom.rec_time_ms += 100;
        }
#endif
    }

    // 500ms timer process
    if ((tmr5ms_cnt % 100) == 0) {
        sys_cb.cm_times++;
#if FUNC_CLOCK_EN
        msg_enqueue(MSG_SYS_500MS);
#endif // FUNC_CLOCK_EN
    }

    // 1s timer process
    if ((tmr5ms_cnt % 200) == 0) {
        msg_enqueue(MSG_SYS_1S);
        tmr5ms_cnt = 0;
        sys_cb.lpwr_warning_cnt++;
#if CHARGE_EN
        bsp_charge_bled_ctrl();
#endif
    }
}

#if ((GUI_SELECT == GUI_LEDSEG_7P7S) || (GUI_SELECT == GUI_LEDSEG_6C6S))
AT(.com_text.timer)
void timer1_isr(void)
{
    if (TMR1CON & BIT(9)) {
        TMR1CPND = BIT(9); // Clear Pending
        TMR1CON  = 0;
    #if (GUI_SELECT == GUI_LEDSEG_7P7S)
        ledseg_7p7s_clr(); // close display
    #elif (GUI_SELECT == GUI_LEDSEG_6C6S)
        ledseg_6c6s_clr();
    #endif
    }
}
#endif

AT(.com_text.bsp.sys)
void bsp_loudspeaker_mute(void)
{
    LOUDSPEAKER_MUTE();
    sys_cb.loudspeaker_mute = 1;
}

AT(.com_text.bsp.sys)
void bsp_loudspeaker_unmute(void)
{
    sys_cb.loudspeaker_mute = 0;
#if EARPHONE_DETECT_EN
    if (!dev_is_online(DEV_EARPHONE))
#endif // EARPHONE_DETECT_EN
    {
        LOUDSPEAKER_UNMUTE();
    }
}

AT(.text.bsp.sys)
void bsp_sys_mute(void)
{
    if (!sys_cb.mute) {
        sys_cb.mute = 1;
#if (GUI_SELECT != GUI_NO)
        gui_box_flicker_set(5, 0xff, 0xff);
#endif
        dac_fade_out();
        bsp_loudspeaker_mute();
#if DAC_DNR_EN
        dac_dnr_set_sta(0);
#endif
    }
}

AT(.text.bsp.sys)
void bsp_sys_unmute(void)
{
    if (sys_cb.mute) {
        sys_cb.mute = 0;
        bsp_loudspeaker_unmute();
        dac_fade_in();
#if DAC_DNR_EN
        dac_dnr_set_sta(sys_cb.dnr_sta);
#endif
    }
}

AT(.text.bsp.sys)
void bsp_clr_mute_sta(void)
{
    if (sys_cb.mute) {
        sys_cb.mute = 0;
#if DAC_DNR_EN
        dac_dnr_set_sta(sys_cb.dnr_sta);
#endif
    }
}

AT(.com_text.bsp.sys)
bool bsp_get_mute_sta(void)
{
    return sys_cb.mute;
}

////PWRKEY 引脚通过开关80K上拉/10K下拉, 来实现输出高低电平, 注意高电平为1.8V
// AT(.com_text.bsp.sys)
// void powerkey_out_set(bool high)  //里面调用到的函数都在公共区.
//{
//     if (high) {
//         RTCCON1 &= ~(BIT(1)| BIT(2) | BIT(3)) ;  //bit1: 10K PD DISABLE  //bit2,bit3 = 00, PU IS 80K
//         RTCCON1 |= BIT(4) ;                      //PU ENABLE,
//     } else {
//         RTCCON1 &= ~BIT(4) ;      //PU DISABLE
//         RTCCON1 |= BIT(1) ;       //10K PD ENABLE
//         if (!(RTCCON11 & BIT(4))){ //PD enable protect bit.
//             RTCCON11 |= BIT(4);
//         }
//     }
// }

AT(.text.bsp.sys.init)
static void rtc_32k_configure(void)
{
    u32 temp = RTCCON0;

#if EXT_32K_EN
    temp |= BIT(1);  // enable 32.768k osc
    temp &= ~BIT(6); // disable ext_32k
    RTCCON0 = temp;
    RTCCON2 &= ~BIT(6); // select 32.768k
#else
    temp &= ~BIT(1); // disable 32k osc
    temp |= BIT(8);  // rtc clk sel x26m div
    temp |= BIT(6);  // enable clk32k_rtc
    temp |= BIT(2);
    RTCCON0 = temp;
    RTCCON2 |= BIT(6); // select ext_32k
#endif // EXT_32K_EN
}

AT(.text.bsp.sys.init)
bool rtc_init(void)
{
    u32 temp;
    rtc_32k_configure();
    sys_cb.rtc_first_pwron = 0;
    temp                   = RTCCON0;
    if (temp & BIT(7)) {
        temp &= ~BIT(7); // clear first poweron flag
        RTCCON0                = temp;
        sys_cb.rtc_first_pwron = 1;
#if FUNC_CLOCK_EN
        rtc_clock_init();
#endif // FUNC_CLOCK_EN
        printf("rtc 1st pwrup\n");
        return false;
    }

    return true;
}

// UART0打印信息输出GPIO选择，UART0默认G1(PA7)
void uart0_mapping_sel(void)
{
    // 等待uart0发送完成
    if (UART0CON & BIT(0)) {
        while (!(UART0CON & BIT(8)))
            ;
    }

    GPIOADE &= ~BIT(7);
    GPIOAFEN &= ~BIT(7);
    GPIOAPU &= ~BIT(7);
    GPIOBFEN &= ~(BIT(2) | BIT(3));
    GPIOBPU &= ~(BIT(2) | BIT(3));
    FUNCMCON0 = (0xf << 12) | (0xf << 8); // clear uart0 mapping

#if (UART0_PRINTF_SEL == PRINTF_PA7)
    GPIOADE |= BIT(7);
    GPIOAPU |= BIT(7);
    GPIOADIR |= BIT(7);
    GPIOAFEN |= BIT(7);
    GPIOADRV |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA7; // RX0 Map To TX0, TX0 Map to G1
#elif (UART0_PRINTF_SEL == PRINTF_PB2)
    GPIOBDE |= BIT(2);
    GPIOBPU |= BIT(2);
    GPIOBDIR |= BIT(2);
    GPIOBFEN |= BIT(2);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB2; // RX0 Map To TX0, TX0 Map to G2
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    GPIOBDE |= BIT(3);
    GPIOBPU |= BIT(3);
    GPIOBDIR |= BIT(3);
    GPIOBFEN |= BIT(3);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB3; // RX0 Map To TX0, TX0 Map to G3
#elif (UART0_PRINTF_SEL == PRINTF_PE7)
    GPIOEDE |= BIT(7);
    GPIOEPU |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE7; // RX0 Map To TX0, TX0 Map to G4
#elif (UART0_PRINTF_SEL == PRINTF_PE0)
    GPIOEDE |= BIT(0);
    GPIOEPU |= BIT(0);
    GPIOEDIR |= BIT(0);
    GPIOEFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE0; // RX0 Map To TX0, TX0 Map to G5
#elif (UART0_PRINTF_SEL == PRINTF_PF0)
    GPIOFDE |= BIT(0);
    GPIOFPU |= BIT(0);
    GPIOFDIR |= BIT(0);
    GPIOFFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PF0; // RX0 Map To TX0, TX0 Map to G6
#elif (UART0_PRINTF_SEL == PRINTF_PF2)
    GPIOFDE |= BIT(0);
    GPIOFPU |= BIT(0);
    GPIOFDIR |= BIT(0);
    GPIOFFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PF2; // RX0 Map To TX0, TX0 Map to G7
#endif

#if UART0_CLK_SEL_X26M
    u32 baud_cfg = (26000000 / 2 + 1500000 / 2) / 1500000 - 1; // 1.5M
    CLKCON1 |= BIT(14);                                        // x26m to uart_inc
    UART0CON |= BIT(5);                                        // uart clksrc sel uart_inc
    UART0BAUD = (baud_cfg << 16) | baud_cfg;
#endif
}

#if UART0_CLK_SEL_X26M
// 可能用到UART0(使用26M时钟源)做通信,这里可选设置系统时钟 set_sys_clk 时不改波特率
void update_uart0baud_in_sysclk_set(u32 uart_baud)
{
}
#endif
AT(.rodata.vol)
const u8 maxvol_tbl[4] = {16, 30, 32, 50};

// 开user timer前初始化的内容
AT(.text.bsp.sys.init)
static void bsp_var_init(void)
{
    memset(&sys_cb, 0, sizeof(sys_cb));
    sys_cb.loudspeaker_mute = 1;
    sys_cb.ms_ticks         = tick_get();
    sys_cb.pwrkey_5s_check  = 1;
#if DYNAMIC_BASS_BOOST_EN
    xcfg_cb.vol_max = 0; // 该功能暂只支持16级音量
#endif

#if EXLIB_BT_MONO_XDRC_EN
    if (EQ_DBG_IN_UART && xcfg_cb.eq_dgb_uart_en) {
        xcfg_cb.bt_a2dp_vol_ctrl_en = 0; // EQ在线调试时关音量同步,避免音量变动影响调试
    }
#endif

    xcfg_cb.vol_max = maxvol_tbl[xcfg_cb.vol_max];

    sys_cb.hfp2sys_mul = (xcfg_cb.vol_max + 2) / 16;
    if (SYS_INIT_VOLUME > xcfg_cb.vol_max) {
        SYS_INIT_VOLUME = xcfg_cb.vol_max;
    }
    if (WARNING_VOLUME > xcfg_cb.vol_max) {
        WARNING_VOLUME = xcfg_cb.vol_max;
    }

#if (WORK_MODE == MODE_BQB_RF)
    xcfg_cb.sys_sleep_time = 0;
    xcfg_cb.sys_off_time   = 0;
    cfg_bt_tws_mode        = 0;
#endif

#if BT_A2DP_VOL_HID_CTRL_EN
    xcfg_cb.bt_hid_en = 1;
#endif

    sys_cb.sleep_time  = -1L;
    sys_cb.pwroff_time = -1L;
    if (xcfg_cb.sys_sleep_time != 0) {
        sys_cb.sleep_time = (u32)xcfg_cb.sys_sleep_time * 10; // 100ms为单位
    }
    if (xcfg_cb.sys_off_time != 0) {
        sys_cb.pwroff_time = (u32)xcfg_cb.sys_off_time * 10; // 100ms为单位
    }

    sys_cb.sleep_delay        = -1L;
    sys_cb.pwroff_delay       = -1L;
    sys_cb.sleep_en           = 0;
    sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
    sfunc_bt_call_flag        = 0;

    key_var_init();
    plugin_var_init();

    msg_queue_init();

    dev_init(((u8)is_sd_support()) | ((u8)is_usb_support() * 0x02));
#if SD_SOFT_DETECT_EN
    if (SD_IS_SOFT_DETECT()) {
        dev_delay_times(DEV_SDCARD, 3);
    }
#endif

#if SD_USB_MUX_IO_EN && (SD0_MAPPING == SD0MAP_G5 || SD0_MAPPING == SD0MAP_G4)
    sd_other_param_set(0x0F);
#endif

#if MUSIC_SDCARD_EN
    if ((xcfg_cb.sddet_iosel == IO_MUX_SDCLK) || (xcfg_cb.sddet_iosel == IO_MUX_SDCMD)) {
        dev_delay_offline_times(DEV_SDCARD, 3); // 复用时, 加快拔出检测. 这里拔出检测为3次.
    }
#endif

#if MUSIC_SDCARD1_EN
    if ((xcfg_cb.sd1det_iosel == IO_MUX_SDCLK) || (xcfg_cb.sd1det_iosel == IO_MUX_SDCMD)) {
        dev_delay_offline_times(DEV_SDCARD1, 3); // 复用时, 加快拔出检测. 这里拔出检测为3次.
    }
#endif

    aux_var_init();
    sdadc_var_init();
    mic_bias_var_init();
#if FUNC_REC_EN
    record_var_init();
#endif

#if (MUSIC_UDISK_EN || MUSIC_SDCARD_EN || MUSIC_SDCARD1_EN)
    fs_var_init();
#endif
    music_stream_var_init();

    dac_cb_init((DAC_CH_SEL & 0x0f) | ((u8)DAC_LDOH_SEL << 4) | (0x200 * DAC_FAST_SETUP_EN) | (0x400 * DAC_VCM_CAPLESS_EN) | (0x800 * DACVDD_BYPASS_EN));
}

AT(.text.bsp.sys.init)
static void bsp_io_init(void)
{
    GPIOADE = BIT(7); // UART
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F; // MCP FLASH

    uart0_mapping_sel(); // 调试UART IO选择或关闭

#if LINEIN_DETECT_EN
    LINEIN_DETECT_INIT();
#endif // LINEIN_DETECT_EN

#if MUSIC_SDCARD_EN
    SD_DETECT_INIT();
#endif // MUSIC_SDCARD_EN

#if MUSIC_SDCARD1_EN
    SD1_DETECT_INIT();
#endif // MUSIC_SDCARD1_EN

#if EARPHONE_DETECT_EN
    EARPHONE_DETECT_INIT();
#endif // EARPHONE_DETECT_EN

    LOUDSPEAKER_MUTE_INIT();
    AMPLIFIER_SEL_INIT();

#if MIC_DETECT_EN
    MIC_DETECT_INIT();
#endif
}

void bsp_get_xosc_xcfg(u8 *osci_cap, u8 *osco_cap, u8 *both_cap)
{
    *osci_cap = xcfg_cb.osci_cap;
    *osco_cap = xcfg_cb.osco_cap;
    *both_cap = xcfg_cb.osc_both_cap;
}

#if BT_TWS_EN
AT(.text.bsp.sys.init)
bool tws_pwrkey_820k_check(void) // TWS PWRKEY 820K接地为左声道
{
    u8   i, cnt;
    u16  adc_val;
    bool res = false;
    while (1) {
        if (RTCCON & BIT(19)) { // 等待PWRKEY 松开.
            break;
        }
        delay_ms(5);
        WDT_CLR();
    }

    i   = 10;
    cnt = 0;
    while (i--) {
        WDT_CLR();
        delay_ms(7);
        adc_val = ((u32)adc_cb.wko_val << 8) / adc_cb.vrtc_val;
        // pu= 90k  pd=820.000K adc=230    //max 255   //min 150k--159
        if ((adc_val > 200) && (adc_val < 245)) {
            cnt++;
        }
        // printf("[%d_%d_%d]\n",adc_val,adc_cb.vrtc_val,adc_cb.wko_val);
        if (cnt >= 5) {
            res = true;
            break;
        }
    }
    // printf("res = %d,i = %d, cnt = %d\n",res,i,cnt);
    return res;
}

static gpio_t tws_sel_left_gpio;
AT(.text.bsp.sys.init)
bool tws_gpiox_2_gnd_check(void) // TWS 特定GPIO 接地为左声道
{
    u8      i, cnt;
    bool    res    = false;
    gpio_t *g      = &tws_sel_left_gpio;
    u8      io_num = xcfg_cb.tws_sel_left_gpio_sel;
    // printf("io_num = %d\n",io_num);
    if ((!io_num) || (io_num > IO_PF3)) {
        return false;
    }
    bsp_gpio_cfg_init(&tws_sel_left_gpio, xcfg_cb.tws_sel_left_gpio_sel);
    // io sta backup
    u32 bk_de  = g->sfr[GPIOxDE];
    u32 bk_pu  = g->sfr[GPIOxPU];
    u32 bk_dir = g->sfr[GPIOxDIR];
    // 数字IO, 开上拉.
    g->sfr[GPIOxDE] |= BIT(g->num);
    g->sfr[GPIOxPU] |= BIT(g->num);
    g->sfr[GPIOxDIR] |= BIT(g->num);

    i   = 6;
    cnt = 0;
    while (i--) {
        delay_ms(5);
        if (!(g->sfr[GPIOx] & BIT(g->num))) { // to GND
            // printf("GND\n");
            cnt++;
        } else {
            // printf("!!!PU\n");
        }
        if (cnt >= 3) {
            res = true;
            break;
        }
    }
    // 检测完成, 恢复以前IO状态
    g->sfr[GPIOxDE]  = bk_de;
    g->sfr[GPIOxPU]  = bk_pu;
    g->sfr[GPIOxDIR] = bk_dir;
    // printf("res = %d,i = %d, cnt = %d\n",res,i,cnt);
    return res;
}

AT(.text.bsp.sys.init)
void tws_lr_xcfg_sel(void)
{
    static bool checked_flag = false; // 只检测一次.
    if ((!xcfg_cb.bt_tws_en) || (checked_flag) || xcfg_cb.bt_tws_lr_mode < 8) {
        return;
    }
    // printf("xcfg_cb.bt_tws_lr_mode = %d\n",xcfg_cb.bt_tws_lr_mode);
    if (8 == xcfg_cb.bt_tws_lr_mode) { // 有PWRKEY 820K接地则为左声道
        sys_cb.tws_left_channel = tws_pwrkey_820k_check();
    } else if (9 == xcfg_cb.bt_tws_lr_mode) { // 有GPIOx接地则为左声道
        sys_cb.tws_left_channel = tws_gpiox_2_gnd_check();
    } else if (10 == xcfg_cb.bt_tws_lr_mode) { // 配置选择为左声道
        sys_cb.tws_left_channel = 1;
    } else if (11 == xcfg_cb.bt_tws_lr_mode) { // 配置选择为右声道
        sys_cb.tws_left_channel = 0;
    } else {
        return;
    }
    checked_flag = true;
    bt_name_suffix_set();
}

void tws_get_lr_channel(u8 tws_status)
{
    bool first_role = bt_tws_get_first_role();

    if (xcfg_cb.bt_tws_lr_mode >= 8) { // 硬件选择（或配置选择）
        tws_lr_xcfg_sel();
    } else if (1 == xcfg_cb.bt_tws_lr_mode) { // 主右声道，副左声道
        sys_cb.tws_left_channel = first_role ? true : false;
    } else if (2 == xcfg_cb.bt_tws_lr_mode) { // 主左声道，副右声道
        sys_cb.tws_left_channel = first_role ? false : true;
    }
}
#endif

AT(.text.bsp.sys.init)
void bsp_update_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) { // 获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    bsp_io_init();

    // var init
    bsp_var_init();
    sys_cb.lang_id = 0;

    // peripheral init
    rtc_init();
    param_init(sys_cb.rtc_first_pwron);

    // 晶振配置
    xosc_init();

    plugin_init();
    sys_set_tmr_enable(1, 1);

    adpll_init(DAC_OUT_SPR);
    dac_init();
#if WARNING_UPDATE_DONE
    mp3_res_play(RES_BUF_UPDATE_DONE_MP3, RES_LEN_UPDATE_DONE_MP3);
#endif
}

#if 0 // port_int_example
AT(.com_text)
const char strisr0[] = ">>[0x%X]_[0x%X]\n";
const char strisr1[] = "portisr->";
AT(.com_text.timer)
void port_isr(void)
{
    printf(strisr0,WKUPEDG,WKUPCPND);
    if (WKUPEDG & (BIT(6) << 16)) {
        WKUPCPND = (BIT(6) << 16);  //CLEAR PENDING
        printf(strisr1);
    }

}

void port_int_example(void)     //sys_set_tmr_enable(1, 1); 前调用 测试OK
{
    GPIOFDE |= BIT(0);  GPIOFDIR |= BIT(0); GPIOFFEN &= ~BIT(0);
    GPIOFPU |= BIT(0);
    sys_irq_init(IRQ_PORT_VECTOR,0, port_isr);
    PORTINTEN |= BIT(21);
    PORTINTEDG |= BIT(21);  //falling edge;
    WKUPEDG |= BIT(6);     //falling edge
    WKUPCON = BIT(6) | BIT(16);  //falling edge wake iput //wakeup int en

    printf("PORTINTEN = 0x%X, PORTINTEDG = 0x%X  WKUPEDG = 0x%X, WKUPCON = 0x%X\n", PORTINTEN, PORTINTEDG, WKUPEDG, WKUPCON);
    WDT_CLR();
    while(1) {
//       printf("WKUPEDG = 0x%X\n", WKUPEDG);
//       printf("GPIOF = 0x%X\n", GPIOF);
//       delay_ms(500);
    }
}
#endif

/*
AT(.com_rodata.isr)
const char str_t3[] = "T3 ";

AT(.com_text.isr) FIQ
void timer3_isr(void)
{
    if (TMR3CON & BIT(16)) {
        TMR3CPND = BIT(16);              //Clear Pending
    }
    printk(str_t3);
}

void timer3_init(void)
{
    TMR3CON =  BIT(7);                  //Timer overflow interrupt enable
    TMR3CNT = 0;
    TMR3PR  = 1000000 / 2 - 1;          //500ms, select xosc26_div 1M clk
    TMR3CON |= BIT(2) | BIT(0);         //Timer works in Counter Mode
    sys_irq_init(IRQ_TMR3_VECTOR, 1, timer3_isr);
}
*/
// uart0用来升级，使用PA4进行Debug
#if 0 //(UART_UPD_PORT_SEL&UART_TX0_G3_PB3==UART_TX0_G3_PB3)&&UART_S_UPDATE
    #define UART_BAUD     1500000
    #define UART_BAUD_VAL (((26000000 + (UART_BAUD / 2)) / UART_BAUD) - 1)
void uart1_putchar(u8 ch);
AT(.comm)
void uart1_putchar_2(char ch)
{
    WDT_CLR();
    while (!(UART1CON & BIT(8)));
    UART1DATA = ch;
}
void my_printf_init(void (*putchar)(char));
void uart1_init(void)
{
    FUNCMCON0|=(0x0f<<24);
    GPIOADE |= BIT(4);                                         //Port Function EN
    UART1BAUD = (UART_BAUD_VAL << 16) | UART_BAUD_VAL;
    UART1CON = BIT(4) | BIT(0);                                 //2 Stop bits, Uart EN
    UART1CPND = BIT(17) | BIT(16);                              //CLR Pending
    FUNCMCON0 |= (2 << 24);                                      //TX1 Map to G1(PA4)
    GPIOADIR &= ~BIT(4);
    GPIOAFEN |=BIT(4);

}
#endif

#if 0 // DEBUG_SPI_LOAD  //内部加载SPIFLASH代码，加载一次打印一次,用于评估系统切bank是否频繁
AT(.com_text*)
const char strload[] = "->";
AT(.com_text.debug_cache)
void cache_spi_load_callback(u32 epc, u32 addr) {printk(strload);}
#endif

void print_comm_info(void)
{
    u32 comm_size = (u32)&__comm_end - (u32)&__comm_start;
    comm_size     = comm_size;
    // comm(rx)        : org = __comm_vma,         len = 16k
    printf("comm_ram(16K)[0x%X,0x%X],used = %d, remain = %d\n", (u32)&__comm_start, (u32)&__comm_start + 1024 * 16, comm_size, 16 * 1024 - comm_size);
    // bss data : org = 0x10C00,            len = 13k
    printf("bss_ram(13K)[0x%X,0x%X],used = %d, remain = %d\n", (u32)&__bss_start, (u32)&__bss_start + 1024 * 13, (u32)&__bss_size, 13 * 1024 - (u32)&__bss_size);
}

#if PWRON_QUICK_ADC_PRINT
    // 上电快速检测ADC,需要把header_adc.bin修改成header.bin并重新编译
    // 目前支持PWRKEY,PB2,PB4,检测结果放在地址0x58400的RAM中供后面的程序使用，每条通路采16次ADC值
    #define PWRON_ADC_SAMPLES 16
typedef struct TYPE_ADC_VAL
{
    u8 ad0[PWRON_ADC_SAMPLES]; // PWRKEY
    u8 ad1[PWRON_ADC_SAMPLES]; // PB2
    u8 ad2[PWRON_ADC_SAMPLES]; // PB4
} type_adc_val;
type_adc_val *pwron_adc_buf = (type_adc_val *)0x58400; // 0x58400

void print_pwron_adc_info(void)
{
    printf("\nprint_pwron_adc_info:\n");
    printf("ADC PWRKEY:\n");
    print_r(pwron_adc_buf->ad0, PWRON_ADC_SAMPLES);
    printf("ADC PB2:\n");
    print_r(pwron_adc_buf->ad1, PWRON_ADC_SAMPLES);
    printf("ADC PB4:\n");
    print_r(pwron_adc_buf->ad2, PWRON_ADC_SAMPLES);
}
#endif

AT(.text.bsp.sys.init)
void bsp_sys_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) { // 获取配置参数
        printf("xcfg init error\n");
    }
#if PWRON_ADC_QUICK_DETECT
    print_pwron_adc_info();
#endif
    print_comm_info();

    // io init
    bsp_io_init();

    // var init
    bsp_var_init();

    // power init
    pmu_init(0);

    // 低电复位电压设置,DAC大音量可能复位时,可以尝试调小LVD复位电压
    // LVDCON = (LVDCON & ~0x07) | 2;  //LVDCON[2:0] default is 2(2.2V)   //0(1.8V),1(2V),2(2.2V),3(2.4V),4(2.6V),5(2.8V),6(3V),7(3.2V)

    // clock init
    adpll_init(DAC_OUT_SPR);
    set_sys_clk(SYS_CLK_SEL);

    // peripheral init
    rtc_init();
    param_init(sys_cb.rtc_first_pwron);

    // 晶振配置
    xosc_init();
    plugin_init();

#if FUNC_HDMI_EN && HDMI_DETECT_EN
    hdmi_detect_io_init();
    dev_delay_online_times(DEV_HDMI, 3);
#endif

#if IRRX_SW_EN
    irrx_sw_init();
#endif // IRRX_SW_EN

#if IRRX_HW_EN
    irrx_hw_init();
#endif // IRRX_HW_EN

    if (POWKEY_10S_RESET) {
        RTCCON12 = 0; // 10S Reset Enable
    } else {
        RTCCON12 = 0x0a; // 10S Reset Disable
    }

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        bsp_charge_init();
    }
#endif // CHARGE_EN

    led_init();
    key_init();

    gui_init();
#if PWM_RGB_EN
    pwm_init();
#endif // PWM_RGB_EN

#if EXLIB_BT_MONO_XDRC_EN
    xdrc_ram_clear();
#endif

    /// enable user timer for display & dac
    sys_set_tmr_enable(1, 1);
    led_power_up();
    gui_display(DISP_POWERON);

    bt_init();
    if (xcfg_cb.spk_mute_en * LOUDSPEAKER_MUTE_EN) {
#if !SYS_KARAOK_EN && FUNC_BT_EN && PWRON_ENTER_BTMODE_EN
        func_bt_init(); // 提前初始化蓝牙,可以让蓝牙芯片更快回连上手机
#endif
    }
#if FUNC_SPDIF_TX_EN
    func_spdif_tx_init();
#endif
    dac_init();
#if FMTX_EN
    bsp_fmtx_init();
#endif

#if SPI1_AUDIO_EN
    spi1_play();
#endif
    // sys_cb.vol = VOL_MAX;
    bsp_change_volume(sys_cb.vol);

#if EX_SPIFLASH_SUPPORT
    exspiflash_init();
#endif

#if I2S_EN && (I2S_MODE_SEL == 0) // I2S Master
    bsp_i2s_init();
#endif

#if WARNING_POWER_ON
    if (SD_INSERT_EXCEPTION_RESET && is_sd_insert_reset()) {
        printf("sd insert exception reset, NO need to play power_on.mp3\n");
    } else {
        // mp3_res_play(RES_BUF_POWERON_MP3, RES_LEN_POWERON_MP3);
        extern uint os_spiflash_read(void *buf, u32 addr, uint len);
        register_spi_read_function((void *)os_spiflash_read);
        mp3_res_play(0x76000, 13155);
        register_spi_read_function(NULL);
    }

#endif // WARNING_POWER_ON

    mic_bias_trim();

    if (PWRON_ENTER_BTMODE_EN) {
        func_cb.sta = FUNC_BT;
        if (dev_is_online(DEV_UDISK)) {
            sys_cb.cur_dev = DEV_UDISK;
        } else {
            sys_cb.cur_dev = DEV_SDCARD;
        }
    } else {
#if SD_SOFT_DETECT_EN
        sd_soft_detect_poweron_check();
#endif
        if (dev_is_online(DEV_USBPC)) {
            func_cb.sta = FUNC_USBDEV;
        } else if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_UDISK) || dev_is_online(DEV_SDCARD1)) {
            func_cb.sta = FUNC_MUSIC;
            if (dev_is_online(DEV_UDISK)) {
                sys_cb.cur_dev = DEV_UDISK;
            } else if (dev_is_online(DEV_SDCARD1)) {
                sys_cb.cur_dev = DEV_SDCARD1;
            } else {
                sys_cb.cur_dev = DEV_SDCARD;
            }
        } else {
#if FUNC_AUX_EN
            if (dev_is_online(DEV_LINEIN)) {
                func_cb.sta = FUNC_AUX;
            } else
#endif // FUNC_AUX_EN
            {
#if FUNC_HDMI_EN
                if (dev_is_online(DEV_HDMI)) {
                    func_cb.sta = FUNC_HDMI;
                    bsp_hdmi_cec_init();
                    cec_rx_start(); // 提前开始接收，及时回电视ack信息
                } else
#endif
                    func_cb.sta = FUNC_BT;
            }
        }
    }

#if SYS_MODE_BREAKPOINT_EN
    u8 sta = param_sys_mode_read();
    if (sta != 0 && sta != 0xff) {
        func_cb.sta = sta & 0xf;
        if (func_cb.sta == FUNC_MUSIC) {
            sys_cb.cur_dev = sta >> 4;
        }
    }
#endif // SYS_MODE_BREAKPOINT_EN

#if LINEIN_2_PWRDOWN_EN
    if (dev_is_online(DEV_LINEIN)) {
        sys_cb.pwrdwn_tone_en = LINEIN_2_PWRDOWN_TONE_EN;
        func_cb.sta           = FUNC_PWROFF;
    }
#endif // LINEIN_2_PWRDOWN_EN

#if SD_INSERT_EXCEPTION_RESET
    if (is_sd_insert_reset()) {
        func_cb.sta = FUNC_MUSIC;
    }
#endif

#if BT_BACKSTAGE_EN
    func_bt_init();
#endif

#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
    eq_dbg_init();
#endif // EQ_DBG_IN_UART

#if PLUGIN_SYS_INIT_FINISH_CALLBACK
    plugin_sys_init_finish_callback(); // 初始化完成, 各方案可能还有些不同参数需要初始化,预留接口到各方案
#endif

#if SYS_KARAOK_EN
    if (func_cb.sta != FUNC_BT) { // 蓝牙模式先不初始化karaok
        bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
    }
#endif

#if AUDIO_STRETCH_EN
    bsp_stretch_init(); // 变速不变调初始化
#endif

#if UDE_STOAGE_FLASH_EN
    void spiflash1_init_usb(void);
    spiflash1_init_usb();
    if (spiflash1_manu_id_read()) {
        func_cb.sta = FUNC_MUSIC;
    }
#endif // UDE_STOAGE_FLASH_EN

#if UART_M_UPDATE || UART_S_UPDATE
    #if 0  //(UART_UPD_PORT_SEL&UART_TX0_G3_PB3==UART_TX0_G3_PB3)&&UART_S_UPDATE
    CLKGAT0 |= BIT(21);
    uart1_init();
    my_printf_init(uart1_putchar_2);                              //使用PA4进行Debug
    #endif // UART_UPD_PORT_SEL
    uart_upd_init(UART_UPD_PORT_SEL, UART_UPD_BAUD, SYS_CLK_SEL);
    #if UART_S_UPDATE
    uart_upd_isr_init();
    #endif // UART_S_UPDATE
#endif

#if LED_MATRIX_HUART_TX
    huart_module_init();
#endif

#if EXLIB_BT_MONO_XDRC_EN
    bt_mono_xdrc_init();
#endif

    mp3_uart_update_init();
}
