#include "include.h"
#include "func.h"

bool power_off_check(void);
void lock_code_pwrsave(void);
void unlock_code_pwrsave(void);

#if PWR_RC_32K_EN
void param_rtccnt_write(u8 *param);
void param_rtccnt_read(u8 *param);
void param_rtc32clk_write(u8 *param);
void param_rtc32clk_read(u8 *param);
void param_rtc_pwroffhz_read(u8 *param);
void param_rtcnsec_read(u8 *param);
u32 sys_get_rc2m_clkhz(u32 type);
extern const char week_str[7][4];
#endif // PWR_RC_32K_EN

AT(.com_text.sleep)
void lowpwr_tout_ticks(void)
{
    if(sys_cb.sleep_delay != -1L && sys_cb.sleep_delay > 0) {
        sys_cb.sleep_delay--;
    }
    if(sys_cb.pwroff_delay != -1L && sys_cb.pwroff_delay > 0) {
        sys_cb.pwroff_delay--;
    }
}

AT(.com_text.sleep)
bool sys_sleep_check(u32 *sleep_time)
{
    if(*sleep_time > sys_cb.sleep_wakeup_time) {
        *sleep_time = sys_cb.sleep_wakeup_time;
        return true;
    }
    return false;
}

AT(.sleep_text.sleep)
void sys_sleep_cb(u8 lpclk_type)
{
    //注意！！！！！！！！！！！！！！！！！
    //此函数只能调用sleep_text或com_text函数

    //此处关掉影响功耗的模块
    u32 gpiogde = GPIOGDE;
    GPIOGDE = BIT(2) | BIT(4);                  //SPICS, SPICLK

    sys_enter_sleep(lpclk_type);                //enter sleep

    //唤醒后，恢复模块功能
    GPIOGDE = gpiogde;
}

//休眠定时器，500ms进一次
AT(.sleep_text.sleep)
uint32_t sleep_timer(void)
{
    uint32_t ret = 0;

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_detect(0);
    }
#endif // CHARGE_EN

    sys_cb.sleep_counter++;
    if (sys_cb.sleep_counter == 10) {
        sys_cb.sleep_counter = 0;
#if USER_TKEY_HW_EN
        if ((tk_cb.ch != 0xff) && !tkey_is_pressed()) {
            delay_5ms(2);                   //等待中断更新tkcnt
            TKBCNT = tkcnt_buf[tk_cb.ch];
        }
#endif
    }

    if (led_bt_sleep()) {
        sys_cb.sleep_wakeup_time = 50000/625;

#if VBAT_DETECT_EN
        bsp_saradc_init(adc_cb.channel);
        if ((xcfg_cb.lowpower_warning_en) && (sys_cb.vbat < ((u16)LPWR_WARNING_VBAT*100 + 2750))) {
            //低电需要唤醒sniff mode
            ret = 2;
        }
        saradc_exit(adc_cb.channel);
#endif // VBAT_DETECT_EN

#if CHARGE_EN
        if (!port_2led_is_sleep_en()) {
            ret = 2;
        }
#endif // CHARGE_EN

    } else {
        sys_cb.sleep_wakeup_time = -1L;
    }
    if(sys_cb.pwroff_delay != -1L) {
        if(sys_cb.pwroff_delay > 5) {
            sys_cb.pwroff_delay -= 5;
        } else {
            sys_cb.pwroff_delay = 0;
            return 1;
        }
    }

    if ((PWRKEY_2_HW_PWRON) && (!IS_PWRKEY_PRESS())){
        ret = 1;
    }
    return ret;
}

AT(.text.lowpwr.sleep)
static void sfunc_sleep(void)
{
    uint32_t status, ret;
    uint32_t usbcon0, usbcon1;
    u8 pa_de, pb_de, pe_de, pf_de, pg_de;
    u8 p111_en;
    u16 adc_ch;
    uint32_t sysclk;

#if VBAT_DETECT_EN
    if (is_lowpower_vbat_warning()) {   //低电不进sniff mode
        return;
    }
#endif // VBAT_DETECT_EN

    printf("%s\n", __func__);

#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
    bt_enter_sleep();

    gui_off();
    led_off();
    rled_off();

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

    if (SLEEP_DAC_OFF_EN) {
        dac_power_off();                        //dac power down
    } else {
        sniff_dac_disable();                    //不关DAC电源，保持VCMBUF有电
    }

    sys_set_tmr_enable(0, 0);

    saradc_exit(adc_cb.channel);                //close saradc及相关通路模拟
    adc_ch = adc_cb.channel;
    adc_cb.channel = BIT(VBAT2_ADCCH) | BIT(ADCCH_VRTC);

#if CHARGE_EN
    bsp_charge_set_stop_time(3600);
#endif

#if MUSIC_UDISK_EN
    if (dev_is_online(DEV_UDISK)) {
        udisk_insert();                         //udisk需要重新enum
    }
#endif

    usbcon0 = USBCON0;                          //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0 = BIT(5);
    USBCON1 = 0;

    sysclk = get_cur_sysclk();
    set_sys_clk(SYS_26M);
    DACDIGCON0 &= ~BIT(0);                      //disable digital dac
    dac_clk_source_sel(2);                      //dac_clkp select xosc26m_clk
    PLL0CON &= ~BIT(18);                        //PLL0 SDM Dis
    PLL0CON &= ~(BIT(6) | BIT(12));             //PLL0 Analog Dis, LDO DIS
    p111_en = pll1_disable();                   //close pll1
    PLL2CON &= ~BIT(18);                        //PLL2 SDM Dis
    PLL2CON &= ~(BIT(6) | BIT(12));             //PLL2 Analog Dis, LDO DIS

    //io analog input
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;
    GPIOADE = 0;
    GPIOBDE = 0;
#if CHARGE_EN
    if (CHARGE_DC_IN() && (xcfg_cb.rled_io_sel == IO_PE0)) {
        GPIOEDE = BIT(0);                       //PE0做充电指示灯，保持IO数字输出
    } else {
        GPIOEDE = 0;
    }
#else
    GPIOEDE = 0;
#endif // CHARGE_EN
    GPIOFDE = 0;
    GPIOGDE = 0x3F;                             //MCP FLASH

    wakeup_disable();
    sleep_wakeup_config();

    sys_cb.sleep_counter = 0;
    sys_cb.sleep_wakeup_time = -1L;
    while(bt_is_sleep()) {
        WDT_CLR();
        status = bt_sleep_proc();

        if(status == 1) {
            ret = sleep_timer();
            if(ret) {
                if (ret == 1) {
                    sys_cb.pwrdwn_tone_en = 1;
                    func_cb.sta = FUNC_PWROFF;
                }
                break;
            }
        }

        if (wakeup_get_status()) {
            break;
        }
#if USER_TKEY_HW_EN
        if (tkey_is_pressed()) {
            break;
        }
#endif
    }

    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;

    wakeup_disable();
    if (p111_en) {
        pll1_enable();
    }
    USBCON0 = usbcon0;
    USBCON1 = usbcon1;

    adpll_init(DAC_OUT_SPR);                    //enable adpll
    dac_clk_source_sel(1);                      //dac_clkp select adplldiv_clk_a

    adc_cb.channel = adc_ch;
    bsp_saradc_init(adc_cb.channel);
#if CHARGE_EN
    bsp_charge_set_stop_time(18000);
    bsp_charge_sta(sys_cb.charge_sta);          //update充灯状态
#endif // CHARGE_EN
    set_sys_clk(sysclk);
    sys_set_tmr_enable(1, 1);

#if GUI_LCD_EN
    gui_init();
    f_bt.draw_update = 1;
#endif // GUI_LCD_EN

    if (DAC_FAST_SETUP_EN) {
        bsp_loudspeaker_mute();
        delay_5ms(2);
    }

    if (SLEEP_DAC_OFF_EN) {
        dac_restart();
    } else {
        sniff_dac_enable();
    }

    bsp_change_volume(sys_cb.vol);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
    bt_exit_sleep();
#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
#endif
    printf("sleep_exit\n");
}

AT(.text.lowpwr.sleep)
bool sleep_process(is_sleep_func is_sleep)
{
    if ((*is_sleep)()) {
        if ((!sys_cb.sleep_en) || (!port_2led_is_sleep_en())) {
            reset_sleep_delay();
            return false;
        }
        if(sys_cb.sleep_delay == 0) {
            sfunc_sleep();
            reset_sleep_delay();
            reset_pwroff_delay();
            return true;
        }
    } else {
        reset_sleep_delay();
        reset_pwroff_delay();
    }
    return false;
}

//红外关机进入sleep mode
ALIGNED(512)
AT(.text.lowpwr.sleep)
void func_sleepmode(void)
{
    uint32_t usbcon0, usbcon1;
    u8 pa_de, pb_de, pe_de, pf_de, pg_de;
    u8 sys_clk, p111_en;
    printf("%s\n", __func__);

    gui_display(DISP_OFF);
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif
#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
    saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟
    dac_power_off();                    //dac power down

    gui_off();
    led_off();
    rled_off();
    sys_clk = get_cur_sysclk();
    set_sys_clk(SYS_26M);
    sys_set_tmr_enable(0, 0);

#if  MUSIC_UDISK_EN
    if (dev_is_online(DEV_UDISK)) {
        udisk_insert();                 //udisk需要重新enum
    }
#endif

    usbcon0 = USBCON0;                  //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0 = BIT(5);
    USBCON1 = 0;

    //close pll0
    PLL0CON &= ~BIT(18);                //pll10 sdm enable disable
    PLL0CON &= ~BIT(6);                 //pll0 analog diable
    PLL0CON &= ~BIT(12);                //pll0 ldo disable

    //close pll1
    p111_en = pll1_disable();

    //io analog input
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F;     //MCP FLASH

#if IRRX_HW_EN
    IR_CAPTURE_PORT();
#endif // IRRX_HW_EN
    wakeup_disable();
//    sleep_wakeup_config();                  //按键唤醒配置
    WDT_DIS();
#if CHARGE_EN
    if (xcfg_cb.charge_en) {    //配置sleep 1S唤醒一次检测充电状态
        RTCCPND = BIT(18);      //clear rtc wakeup sleep pending  (RTCCON_bit18)
        asm("nop"); asm("nop"); asm("nop");
        RTCCON  |= BIT(7);                         //rtc wakeup sleep enable
        RTCCON11 |= BIT(10);                       //rtc timer wakeup sleep enable
        RTCCON11 |= BIT(8)|BIT(9);                 //sleep time sel  1S
    }
#endif
    while(1) {
        LPMCON |= BIT(0);                   //Sleep mode enable
        asm("nop");asm("nop");asm("nop");

#if IRRX_HW_EN
        if (get_irkey() == KEY_IR_POWER) {
            ir_key_clr();
            break;
        }
#endif // IRRX_HW_EN

#if CHARGE_EN
        if (xcfg_cb.charge_en) {
            if (RTCCON & BIT(18)) {
                RTCCPND = BIT(18);       //clear rtc1s pending
            }
            charge_detect(0);
//            if((RTCCON >> 20) & 0x01) {  //DC_IN
//                break;  //DC 5V插入需要退出sleep时,可以在这里break
//            }
        }
#endif // CHARGE_EN

//        //io wakeup
//        if (wakeup_get_status()) {
//            break;
//        }
    }

    WDT_EN();

    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;

    wakeup_disable();

    //enable adpll
    PLL0CON |= BIT(12);                //PLL1 LDO enable
    delay_us(100);
    PLL0CON |= BIT(18);                //PLL1 SDM enable
    PLL0CON |= BIT(6);                 //PLL1 Analog enable

    set_sys_clk(sys_clk);              //还原系统时钟
    if (p111_en) {
        pll1_enable();
    }
    USBCON0 = usbcon0;
    USBCON1 = usbcon1;

    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1, 1);

    if (DAC_FAST_SETUP_EN) {
        bsp_loudspeaker_mute();
        delay_5ms(2);
    }
    dac_restart();
    bsp_change_volume(sys_cb.vol);
    gui_init();
    gui_display(DISP_POWERON);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
#endif
    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;
    printf("out func_sleepmode\n");
}

//硬开关方案，低电时，进入省电状态
ALIGNED(512)
AT(.text.lowpwr.pwroff)
void sfunc_lowbat(void)
{
    WDT_DIS();

    USBCON0 = BIT(5);
    USBCON1 = 0;
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F; //MCP FLASH

    CLKCON0 |= BIT(0);                  //enable RC
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));  //system clk select RC

    PWRCON0 = (PWRCON0 & ~(0xf<<4)) | (0x0<<4);   //VDDIO   0:2.4V   1:2.5V

    PLL2CON &= ~(BIT(18)| BIT(12) | BIT(0));
    PLL1CON &= ~(BIT(18)| BIT(12) | BIT(0));
    PLL0CON &= ~BIT(18);                //PLL0 SDM Dis
    PLL0CON &= ~BIT(6);                 //PLL0 Analog Dis
    PLL0CON &= ~BIT(12);                //PLL0 LDO DIS
    PLL0CON1 &= ~BIT(15);               //PLL SDM LDO disable

    XO26MCON &= ~(0xf<<24);             //X26 output DIS

    XO26MCON &= ~BIT(10);               //X26 DIS
    XO26MCON &= ~BIT(9);                //X26 LDO DIS

    LPMCON |= BIT(0);                   //Sleep mode enable
    asm("nop");asm("nop");asm("nop");
    while(1);
}

///解决充满关机等待5V掉电，超时1分钟
static u8 sfunc_pwrdown_w4_vusb_offline(void)
{
    u32 timeout = 10000, off_cnt = 0;
    while (timeout--) {
        delay_us(330);                      //5ms
        if (!CHARGE_DC_IN()) {
            off_cnt++;
        } else {
            off_cnt = 0;
        }
        if (off_cnt > 5) {
            return 1;                       //VUSB已掉线，打开VUSB唤醒
        }
    }
    return 0;
}

//软开关方案，POWER键/低电时，进入关机状态
ALIGNED(512)
AT(.text.lowpwr.pwroff)
void sfunc_pwrdown(u8 vusb_wakeup_en)
{
    LOUDSPEAKER_MUTE_DIS();
    WDT_DIS();
    led_init();                                 //关闭LED灯显示。
#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        bsp_charge_off();
    }
#endif
//    rtc_set_alarm_wakeup(10);                 //32.768K外部晶振，可以alarm软关机唤醒
    RTCCON0 &= ~BIT(8);                         //CLK2M Select RTC 2M
#if PWR_RC_32K_EN
    printf("%s\n", __func__);
	RTCCON0 |= BIT(0);                          //enable RC
    RTCCON0 |= BIT(14);
    param_rtcnsec_read((u8*)&sys_cb.rtc_nsec);          //获取关机前设置闹铃的RTCALM，通过rtc_set_alarm_relative_time(u32 nsec)设置用户闹铃
    rtc_rc_set_alarm_wakeup(sys_cb.rtc_nsec, 600);      //32.768K省晶振，可以alarm软关机唤醒，600秒唤醒校准闹钟一次
#endif // PWR_RC_32K_EN
    PICCONCLR = BIT(0);                         //Globle IE disable
    CLKCON0 |= BIT(0);                          //enable RC
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));          //system clk select RC

    PLL2CON &= ~(BIT(18)| BIT(12) | BIT(0));
    PLL1CON &= ~(BIT(18)| BIT(12) | BIT(0));
    PLL0CON &= ~BIT(18);                        //PLL0 SDM Dis
    PLL0CON &= ~BIT(6);                         //PLL0 Analog Dis
    PLL0CON &= ~BIT(12);                        //PLL0 LDO DIS
    PLL0CON1 &= ~BIT(15);                       //PLL SDM LDO disable

    XO26MCON &= ~(0xf<<24);                     //X26 output DIS
    XO26MCON &= ~BIT(10);                       //X26 DIS
    XO26MCON &= ~BIT(9);                        //X26 LDO DIS
    RTCCON5 |= BIT(6);                          //vddbt enter lowpower ldo

    if (!vusb_wakeup_en) {
        RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1); //disable charger function
        vusb_wakeup_en = sfunc_pwrdown_w4_vusb_offline();
    }
    RTCCON1 &= ~(BIT(5) | BIT(7));		        //BIT(7): VRTC voltage for ADC, BIT(5):WK pin analog enable, output WKO voltage for ADC
    RTCCON11 = (RTCCON11 & ~0x03) | BIT(2);     //WK PIN filter select 8ms
    uint rtccon3 = RTCCON3 & ~BIT(11);
    uint rtccon1 = RTCCON1 & ~0x1f;
#if CHARGE_EN
    if ((xcfg_cb.charge_en) && (vusb_wakeup_en)) {
        rtccon3 |= BIT(11);                     //VUSB wakeup enable
    }
#endif
    RTCCON3 = rtccon3 & ~BIT(10);               //关WK PIN，再打开，以清除Pending
    rtccon3 = RTCCON3 & ~0x17;                  //Disable VDDCORE VDDIO VDDBUCK, VDDXOEN
    rtccon3 |= BIT(6);                          //Core power down enable, VDDCORE short disable
    rtccon3 |= BIT(10);                         //WK pin wake up enable
    rtccon1 |= BIT(0) | BIT(2) | BIT(4);        //WK 90K pull up enable
    RTCCON |= BIT(5);                           //PowerDown Reset，如果有Pending，则马上Reset
    RTCCON1 = rtccon1;
    RTCCON3 = rtccon3;
    LPMCON |= BIT(0);
    asm("nop");asm("nop");asm("nop");
    while (1);
}

#if PWR_RC_32K_EN
bool pwroff_get_sta(void)
{
    return true;
}

//省晶振，闹钟唤醒后补偿RTCCNT来校准时间
AT(.text.bsp.power)
void pwroff_rtc_cal_by_rc(void)
{
    u32 temp = RTCCON0;
    temp &= ~BIT(8);                                     //RCT 2M
    temp |= BIT(0);
    temp |= BIT(14);
    RTCCON0 = temp;                                      //select RC

    param_rtc32clk_read((u8*)&sys_cb.pwroff_rtcclk);     //读取上一次的平均频率
    param_rtccnt_read((u8*)&sys_cb.pwroff_rtccnt);       //读取软关机时的rtccnt值
    u32 rtc_pwroffhz ;                                   //软关机时的频率
    param_rtc_pwroffhz_read((u8*)&rtc_pwroffhz);
    u32 rtccnt = RTCCNT - sys_cb.pwroff_rtccnt - 1;      //软关机到开机变化的rtccnt值
    u32 rtc_pwronhz = sys_get_rc2m_clkhz(1)/100;
    if(sys_cb.pwroff_rtcclk != 0) {
        sys_cb.pwroff_rtcclk = sys_cb.pwroff_rtcclk + rtc_pwronhz + rtc_pwroffhz;  //上一次平均频率+软开机测的频率+软关机测的频率
        sys_cb.pwroff_rtcclk /= 3 ;
    } else {
        sys_cb.pwroff_rtcclk = rtc_pwronhz + rtc_pwroffhz; //第一次进入闹钟唤醒补偿时间，上一次平均频率为0
        sys_cb.pwroff_rtcclk /= 2 ;
    }
    param_rtc32clk_write((u8*)&sys_cb.pwroff_rtcclk);    //将平均频率写入flash,供开机时读取

    //可以理解为计算RTCCNT计数一次是多少秒，//正常的话，如果sys_cb.pwroff_rtcclk为31.25K时RTCCNT计数一次为1S
    u32 clkhz = 31050000 / ((sys_cb.pwroff_rtcclk + 48) / 96);  // 31250000 / (sys_cb.pwroff_rtcclk / 96)   96分频    //放大10W倍
    if (clkhz < 100000) {                                //走快了,减回去
        RTCCNT -= rtccnt - clkhz * rtccnt / 100000;
    } else if (clkhz > 100000){                          //走慢了,加回去
        RTCCNT += clkhz * rtccnt / 100000 - rtccnt;
    }

    rtc_time_to_tm(RTCCNT, &rtc_tm);                     //更新时间结构体
    printf("time:  %d.%02d.%02d   %02d:%02d:%02d  %s\n", (rtc_tm.tm_year+1970), rtc_tm.tm_mon, rtc_tm.tm_mday,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec, week_str[rtc_tm.tm_wday]);

    RTCCON0 &= ~BIT(0);
    RTCCON0 |= BIT(8);                                  //rtc clk select x26m div
}
#endif //PWR_RC_32K_EN

AT(.text.lowpwr.pwroff)
void func_pwroff(int pwroff_tone_en)
{
    printf("%s\n", __func__);
    led_power_down();
#if WARNING_POWER_OFF
    if (SOFT_POWER_ON_OFF) {
        if (pwroff_tone_en == 1) {
            mp3_res_play(RES_BUF_POWEROFF_MP3, RES_LEN_POWEROFF_MP3);
        } else if (pwroff_tone_en == 2) {
#if WARNING_FUNC_AUX
            mp3_res_play(RES_BUF_AUX_MODE_MP3, RES_LEN_AUX_MODE_MP3);
#endif // WARNING_POWER_OFF
        }
    }
#endif // WARNING_POWER_OFF

    gui_off();

    if (SOFT_POWER_ON_OFF) {
        if (!PWRKEY_2_HW_PWRON) {
            while (IS_PWRKEY_PRESS()) { //等待PWRKWY松开
                delay_5ms(1);
                WDT_CLR();
            }
        }
#if SYS_KARAOK_EN
        bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
        dac_power_off();                    //dac power down
        if (CHARGE_DC_IN()) {
            if (power_off_check()) {
                return;
            }
        }
        saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟
        if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
            RTCCON1 |= BIT(6);              //WK PIN High level wakeup
        }
        sfunc_pwrdown(1);
    } else {
#if SYS_KARAOK_EN
        bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
        dac_power_off();                    //dac power down
        saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟

        sfunc_lowbat();                     //低电关机进入Sleep Mode
    }
}

void delay(uint cnt)
{
    uint i;
    for (i=0; i<cnt; i++) {
        asm("nop");
    }
}


AT(.com_text.lowpwr.sleep)
void func_poweroff_runrc(void)    //可在dac_init后调用函数函数验证,功耗在151uA左右
{
    u8 sys_clk, pa_de, pb_de, pe_de, pf_de, pg_de,gpioapu,gpiobpu,gpioepu,gpiofpu,p111_en;
    u32 pwrcon0,pwrcon2,fmrxangcon0,fmrxangcon1,fmrxangcon2,fmrxangcon3;
    u32 clk_gat0, clk_gat1,usbcon0, usbcon1;
    u32 auang_con1,auang_con2,auang_con3,auang_con4,auang_con5,auang_con6;
    u32 rtccon_buf[11];

    printf("\n===========>%s\n",__func__);
    //my_printf_init(iic_putchar);

	WDT_DIS();
    gui_display(DISP_OFF);
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
    dac_power_off();                    //dac power down
    saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟

#if  MUSIC_UDISK_EN
    if (dev_is_online(DEV_UDISK)) {
        udisk_insert();                 //udisk需要重新enum
    }
#endif

    gui_off();
    led_off();
    rled_off();
    sys_clk = get_cur_sysclk();
    set_sys_clk(SYS_26M);
    sys_set_tmr_enable(0, 0);         //关闭usr_tmr1ms_isr或usr_tmr5ms_isr,tmr0
    printf("CLKCON0 = 0x%X\n",CLKCON0);
    printf("sysclk run rc\n");

    GLOBAL_INT_DISABLE();   //关总中断
    CLKCON0 &= ~(BIT(2) | BIT(3));                  //sysclk sel rc2m
    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;
    gpioapu = GPIOAPU;
    gpiobpu = GPIOBPU;
    gpioepu = GPIOEPU;
    gpiofpu = GPIOFPU;
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOGDE = 0x3F;    //MCP FLASH
    GPIOAPU = 0;
    GPIOBPU = 0;
    GPIOEPU = 0;
    GPIOFPU = 0;

    usbcon0 = USBCON0;                  //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0  = (BIT(6)| BIT(5));        //IE disable
    USBCON1  = BIT(4);
    USBCON0 &= ~BIT(0);
    USBCON1 = 0;

    clk_gat0 = CLKGAT0;
    clk_gat1 = CLKGAT1;

    CLKGAT0 |= BIT(14); //usb_clk
    USBCON1 &= ~BIT(20); //usbdet dis
    CLKGAT0 &= ~BIT(14); //usb_clk

    CLKGAT1 |= BIT(31); //efuse_clk en
    EFCON0 |= (0xaa<<8); //efuse pgmen/avdden dis
    CLKGAT1 &= ~BIT(31); //efuse_clk dis

    CLKGAT1 |= BIT(17); //tk_clk   // touch key
    TKCON &= ~(BIT(14) | BIT(19)); //auto_en, phy_en
    TKACON &= ~(BIT(13) | BIT(26)); //tka_en_sw, tka_enchn
    CLKGAT1 &= ~BIT(17); //tk_clk

    auang_con1 = AUANGCON1;
    auang_con2 = AUANGCON2;
    auang_con3 = AUANGCON3;
    auang_con4 = AUANGCON4;
    auang_con5 = AUANGCON5;
    auang_con6 = AUANGCON6;
    AUANGCON1 = 0;
    AUANGCON2 = 0;
    AUANGCON3 = 0;
    AUANGCON4 = 0;
    AUANGCON5 = 0;
    AUANGCON6 = 0;

    fmrxangcon0 = FMRXANGCON0;
    fmrxangcon1 = FMRXANGCON1;
    fmrxangcon2 = FMRXANGCON2;
    fmrxangcon3 = FMRXANGCON3;
    FMRXANGCON0 = 0;
    FMRXANGCON1 = 0;
    FMRXANGCON2 = 0;
    FMRXANGCON3 = 0;

    PLL0CON &= ~BIT(18);                        //PLL0 SDM Dis
    PLL0CON &= ~(BIT(6) | BIT(12));             //PLL0 Analog Dis, LDO DIS
    p111_en = pll1_disable();                   //close pll1
    PLL2CON &= ~BIT(18);                        //PLL2 SDM Dis     //PLL2不用恢复
    PLL2CON &= ~(BIT(6) | BIT(12));             //PLL2 Analog Dis, LDO DIS


    rtccon_buf[0] = RTCCON0;
    rtccon_buf[1] = RTCCON1;
    rtccon_buf[2] = RTCCON2;
    rtccon_buf[3] = RTCCON3;
    rtccon_buf[4] = RTCCON4;
    rtccon_buf[5] = RTCCON5;
    rtccon_buf[6] = RTCCON6;
    rtccon_buf[7] = RTCCON7;
    rtccon_buf[8] = RTCCON8;
    rtccon_buf[9] = RTCCON9;
    rtccon_buf[10] = RTCCON10;
    rtccon_buf[11] = RTCCON11;

    RTCCON0 = 0;
    RTCCON1 = 0;
    RTCCON5 = 0xFF;
    RTCCON6 = 0;
    RTCCON7 = 0;
    RTCCON8 = 0;
    RTCCON9 = 0;
    RTCCON10 = 0;
    RTCCON11 = 0;

//    MEMCON2 |= (3<<6 | 3<<14);   //MASK clk dis  //实测电流没有变化
//    MEMCON2 = (MEMCON2 & ~(0x0F<<28)) | (0x0A<<28);

    //降VDDCORE电压或关CLKGAT,关晶振
    pwrcon0 = PWRCON0;     //VDDCORE, VDDIO SEL
    pwrcon2 = PWRCON2;
    PWRCON0 = (PWRCON0 & ~0xf) | (0x2);           //VDDCORE 0:0.75V  2:0.85v 4:0.95V
    PWRCON0 = (PWRCON0 & ~(0xf<<4)) | (0x0<<4);   //VDDIO   0:2.4V   1:2.5V

    PWRCON0 |= BIT(20); //pmu_normal
    PWRCON0 &=~BIT(23);
    PWRCON0 &=~BIT(30);
    PWRCON2 &= ~0x1F;        //vddbt lowpower

    XO26MCON &= ~(0xf<<24);             //X26 output DIS
    XO26MCON &= ~BIT(10);               //X26 DIS
    XO26MCON &= ~BIT(9);                //X26 LDO DIS
    PWRCON0 &=~BIT(11);                 //common bias

    CLKGAT0 &= ~(BIT(24)|BIT(25)|BIT(21)|BIT(18)|BIT(16)|BIT(15)|BIT(14)|BIT(12)|BIT(11)|BIT(10)|BIT(9)|BIT(8));//如果需要跑用户毫秒中断,不要关BIT(8)
    CLKGAT0 &= ~(BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(23)|BIT(22)|BIT(20)|BIT(16)|BIT(13));
    CLKGAT1 = 0;

#if 1 //RTC定时唤醒    //打开这里,可以让RTC定时唤醒SLEEP进行某些检测(如检测某个IO为低则退出sleep等)
    CLKGAT0 |= BIT(26);  //25 RTCC CLKGATE
    RTCCON0 |= BIT(0);   //CLK2M_RC EN
    RTCCON0 |= BIT(2);   //CLK2M to RTC32K en
    RTCCON0 |= BIT(6);   //use 32k
    RTCCON0 &= ~BIT(8);  //clock source sel CLK2M
    RTCCPND = BIT(18);      //clear rtc wakeup sleep pending  (RTCCON_bit18)
    asm("nop"); asm("nop"); asm("nop");
    RTCCON  |= BIT(7);                         //rtc wakeup sleep enable
    RTCCON11 |= BIT(10);                       //rtc timer wakeup sleep enable
    RTCCON11 = (RTCCON11 & ~(0x03 << 8)) | (0 << 8);   //wakeup time sel:  0 110ms, 1 220ms, 2 440ms, 3 880ms
#endif

//    //PA6 falling edge wake up config
//    wakeup_gpio_config(IO_PA6,0,1);   //PA6 开内部上拉,下降沿唤醒测试,从while主循环的wakeup_get_status判断中break出来

    //PWRKEY falling edge wakup config  //PRRKEY下降沿唤醒测试,从while主循环的wakeup_get_status判断中break出来
    RTCCON1 = (RTCCON1 & (0x03 << 2)) | (0x01 <<2);   //PWRKEY PU sel: 0_80K, 1_90K, 2_100K,3_400K
    RTCCON1 |= BIT(4);       //PWRKEY PU enable
    RTCCON3  |= BIT(10);     //PWRKEY wake up enable
    WKUPEDG |= BIT(5);       //PWRKEY falling edge to wakeup
    WKUPCPND = 0xff << 16;   //clear all pending
    WKUPCON |= BIT(5);       //wake up source sel PWRKEY

//    //示例: 如可打开 "RTC定时唤醒",检测到PB1为高则退出sleep
//    GPIOBFEN &= ~BIT(1);
//    GPIOBDE |= BIT(1);
//    GPIOBDIR |= BIT(1);
//    GPIOBPD |= BIT(1);

//    printf("run to while(1)\n");
    while(1) {
        //printf("-");
        LPMCON |= BIT(0);                   //Sleep mode enable
        asm("nop");asm("nop");asm("nop");
        //printf(">");

        if (RTCCON & BIT(18)) {
            RTCCPND = BIT(18);       //clear rtc1s pending
        }
        if (wakeup_get_status()) {
            WKUPCPND = 0xff << 16;   //clear all pending
            //printf("wko or other io wake up, break\n");
            break;
        }
        if(IS_PWRKEY_PRESS()){		//可选检测到PWRKEY为低也退出sleep
            //printf("PWRKEY low, break\n");
            break;
        }

//        if (GPIOB & BIT(1)) {   //PB1为高则退出SLEEP
//            //printf("GPIOB 1 HIGHT, break\n");
//            break;
//        }
    }
    //printf("recover PWRCON\n");
    PWRCON0 = pwrcon0;   //恢复VDDCORE, VDDIO等正常电压
    PWRCON2 = pwrcon2;
    delay_us(10);

    //printf("recover clkgat\n");
    CLKGAT0 = clk_gat0;  //恢复CLKGATE
    CLKGAT1 = clk_gat1;
    delay_us(10);

    //printf("recover rtccon\n");
    RTCCON0 = rtccon_buf[0];  //恢复RTC控制的相关电源
    RTCCON1 = rtccon_buf[1];
    RTCCON5 = rtccon_buf[5];
    RTCCON6 = rtccon_buf[6];
    RTCCON7 = rtccon_buf[7];
    RTCCON8 = rtccon_buf[8];
    RTCCON9 = rtccon_buf[9];
    RTCCON10 = rtccon_buf[10];
    RTCCON11 = rtccon_buf[11];

    //printf("recover x26m\n");
    XO26MCON |= BIT(9);                //X26 LDO en
    delay_us(10);
    XO26MCON |= BIT(10);               //X26 en
    XO26MCON |= (0xf<<24);             //X26 output en
    delay_us(50);

    //printf("recover fmrxang\n");
    FMRXANGCON0 = fmrxangcon0;
    FMRXANGCON1 = fmrxangcon1;
    FMRXANGCON2 = fmrxangcon2;
    FMRXANGCON3 = fmrxangcon3;

    //printf("recover usb and AUANGCONx\n");
    USBCON0 = usbcon0;
    USBCON1 = usbcon1;
    AUANGCON1 = auang_con1;
    AUANGCON2 = auang_con2;
    AUANGCON3 = auang_con3;
    AUANGCON4 = auang_con4;
    AUANGCON5 = auang_con5;
    AUANGCON6 = auang_con6;

    //printf("recover gpio\n");
    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;
    GPIOAPU = gpioapu;
    GPIOBPU = gpiobpu;
    GPIOEPU = gpioepu;
    GPIOFPU = gpiofpu;

    //enable adpll
    PLL0CON |= BIT(12);                //PLL1 LDO enable
    delay_us(100);
    PLL0CON |= BIT(18);                //PLL1 SDM enable
    PLL0CON |= BIT(6);                 //PLL1 Analog enable

    CLKCON0 |= BIT(3);                 //sysclk recover
	delay_us(100);
    GLOBAL_INT_RESTORE();
    printf("sysclk recover ok\n");
    set_sys_clk(sys_clk);              //还原系统时钟
    if (p111_en) {
        pll1_enable();
    }
    WDT_EN();
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1, 1);

    if (DAC_FAST_SETUP_EN) {
        bsp_loudspeaker_mute();
        delay_5ms(2);
    }
    dac_restart();
    adpll_init(DAC_OUT_SPR);		//恢复采样率
#if (DAC_OUT_SPR == DAC_OUT_48K)
    DACDIGCON0 |= BIT(1);           //dac out sample 48K
#else
    DACDIGCON0 &= ~BIT(1);          //dac out sample 44.1K
#endif

    bsp_change_volume(sys_cb.vol);
    gui_init();
    gui_display(DISP_POWERON);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
#endif
    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;
}

//假关机，RTC跑26M晶振
//AT(.com_text.lowpwr.sleep)
void func_poweroff_runrc_rtc26m(void)
{
    uint32_t usbcon0, usbcon1;
    u8 pa_de, pb_de, pe_de, pf_de, pg_de,gpioapu,gpiobpu;
    u32 pll0con,pll1con,pll0con1,pll1con1,pll2con1,pwrcon0,fmrxangcon0;
    u32 rtccon1, rtccon3, rtccon4, rtccon5, clk_gat0, clk_gat1;
    u32 clkcon0;
    u32 rtccon3_write,auang_con1,auang_con2,auang_con3,auang_con4,auang_con5,auang_con6;
    u16 adc_ch;
    u8 sys_clk;
    clkcon0 = CLKCON0;
    printf("%s \n",__func__);

    //TEST CODE
//    u8 i;
//    i = 2;
//    while(i) {
//        static u32 time_old = 0;
////        u32 time_cur = irtc_time_read(RTCCNT_CMD);
//        u32 time_cur = RTCCNT;
//        if (time_old != time_cur) {
//            time_old = time_cur;
//            printf("1:[%d]", time_cur);
//            i--;
//        }
//        delay_ms(10);
//    }

    gui_display(DISP_OFF);
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
    dac_power_off();                    //dac power down
    saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟

    gui_off();
    led_off();
    rled_off();
    sys_clk = get_cur_sysclk();
    set_sys_clk(SYS_2M);
    sys_set_tmr_enable(0, 0);         //关闭usr_tmr1ms_isr或usr_tmr5ms_isr,tmr0
    adc_ch = adc_cb.channel;
    adc_cb.channel = BIT(ADCCH_VBAT) | BIT(ADCCH_BGOP);

#if  MUSIC_UDISK_EN
    if (dev_is_online(DEV_UDISK)) {
        udisk_insert();                 //udisk需要重新enum
    }
#endif
    usbcon0 = USBCON0;                  //需要先关中断再保存
    usbcon1 = USBCON1;
    USBCON0  = (BIT(6)| BIT(5));        //IE disable
    USBCON1  = BIT(4);
    USBCON0 &=~ BIT(0);
    USBCON1 = 0;
    //usbphy
    CLKGAT0 |= BIT(14); //usb_clk
    USBCON1 &= ~BIT(20); //usbdet dis
    CLKGAT0 &= ~BIT(14); //usb_clk

    CLKGAT1 |= BIT(31); //efuse_clk en
    EFCON0 |= (0xaa<<8); //efuse pgmen/avdden dis
    CLKGAT1 &= ~BIT(31); //efuse_clk dis

    auang_con1 = AUANGCON1;
    auang_con2 = AUANGCON2;
    auang_con3 = AUANGCON3;
    auang_con4 = AUANGCON4;
    auang_con5 = AUANGCON5;
    auang_con6 = AUANGCON6;
    AUANGCON1 = 0;
    AUANGCON2 = 0;
    AUANGCON3 = 0;
    AUANGCON4 = 0;
    AUANGCON5 = 0;
    AUANGCON6 = 0;

    clk_gat0 = CLKGAT0;
    clk_gat1 = CLKGAT1;

    pll0con = PLL0CON;
    pll1con = PLL1CON;
    pll0con1 = PLL0CON1;
    pll1con1 = PLL1CON1;
    pll2con1 = PLL2CON1;

    pwrcon0 = PWRCON0;

//    rtccon0 = RTCCON0;
    rtccon1 = RTCCON1;
    rtccon3 = RTCCON3;
    rtccon4 = RTCCON4;
    rtccon5 = RTCCON5;

    fmrxangcon0 = FMRXANGCON0;

    pa_de = GPIOADE;
    pb_de = GPIOBDE;
    pe_de = GPIOEDE;
    pf_de = GPIOFDE;
    pg_de = GPIOGDE;
    gpioapu = GPIOAPU;
    gpiobpu = GPIOBPU;
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F;    //MCP FLASH

    CLKCON0 |= BIT(0);                              //enable RC
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));              //sysclk sel rc2m

    CLKGAT1 = 0;
    CLKGAT0 &= ~(BIT(24)|BIT(25)|BIT(21)|BIT(18)|BIT(16)|BIT(15)|BIT(14)|BIT(12)|BIT(11)|BIT(10)|BIT(9)|BIT(8));//如果需要跑用户毫秒中断,不要关BIT(8)
    CLKGAT0 &= ~(BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(23)|BIT(22)|BIT(20)|BIT(16)|BIT(13));

    PLL0CON = 0;        //PLL0 Disable

    PLL1CON = 0;        //PLL1 Disable
    PLL1CON1= 0;
    PLL2CON = 0;
    PLL2CON1 = 0;
    PLL0CON1  = 0xe000;

    // touch key
    CLKGAT1 |= BIT(17); //tk_clk
    TKCON &= ~(BIT(14) | BIT(19)); //auto_en, phy_en
    TKACON &= ~(BIT(13) | BIT(26)); //tka_en_sw, tka_enchn
    CLKGAT1 &= ~BIT(17); //tk_clk

    CLKCON0 |= BIT(0); //rc2m en
    CLKCON0 = (CLKCON0 & ~(0x3<<2)); //sys clk select rc2m
    PLL0CON &= ~(BIT(6) | BIT(12) | BIT(7));
    PLL0CON1 &= ~(BIT(15) | BIT(11) | BIT(12));

    PLL0CON1 = 0;
    //不同唤醒电压可调的值不同 IO唤醒VDDCORE/VDDIO=2/0
    PWRCON0 = (PWRCON0 & ~0xf) | (0x0);           //VDDCORE 0:0.75V  2:0.85v 4:0.95V
    PWRCON0 = (PWRCON0 & ~(0xf<<4)) | (0x0<<4);   //VDDIO   0:2.4V   1:2.5V
    PWRCON0 |= BIT(20); //pmu_normal
    RTCCON3 &= ~BIT(4); //VDDXO_EN dis
    rtccon3_write = rtccon3;
    rtccon3_write = ((rtccon3_write & ~(0xffff)) | 0x6 ); //维持开启VDDCORE VDDIO

    FMRXANGCON0 = 0;
    FMRXANGCON1 = 0;
    FMRXANGCON2 = 0;
    FMRXANGCON3 = 0;

//  RTCCON0 = 0;              //bit 8 是选RC或者XOSC
//  RTCCON1 = 0;              //不能清除，有WKUP的一些设置
//  RTCCON3 = rtccon3_write;  //设置了会导致26M晶振关了
    RTCCON4 = 0;
//  RTCCON5 = 0x3c;           //设置了会导致26M晶振关了
    RTCCON6 = 0;
    RTCCON7 = 0;
    RTCCON8 = 0;
    RTCCON9 = 0;
    RTCCON10 = 0;
    RTCCON11 = 0;

    //可以导出RC，SYS_CLK，XOSC出来看看
//    GPIOEDIR &= ~BIT(6);
//    GPIOEDE |= BIT(6);
//    GPIOEFEN |= BIT(6);
//
//    CLKCON0 &= ~(0xf<<13);
//    CLKCON0 |= 1 << 13;
//    CLKCON2 &= ~(0x1f<<17);
//    CLKCON2 |= 9 << 17;
//    FUNCMCON1 &= ~0xf;
//    FUNCMCON1 |= 6;

    WDT_DIS();

    //WKO WAKE UP
    WKUPEDG |= BIT(5);
    WKUPCPND = 0xff << 16;
    WKUPCON |= BIT(5);
    u32 temp;
    temp = RTCCON3;
    temp |= BIT(10);
    RTCCON3 = temp;

    //假关机, 进入sleep, 等待唤WKO唤醒
    while(1) {
        LPMCON |= BIT(0);
        delay_us(10000);
        if (wakeup_get_status()) {   //WKO唤醒
            break;
        }
    }
    delay_us(10);

    PWRCON0 = pwrcon0;
    delay_us(20);
    RTCCON1 = rtccon1;
    RTCCON3 = rtccon3;
    RTCCON4 = rtccon4;
    RTCCON5 = rtccon5;
    delay_us(20);

    FMRXANGCON0 = fmrxangcon0;

    CLKGAT0 = clk_gat0;
    CLKGAT1 = clk_gat1;

    delay_us(10);
    PLL0CON = pll0con;
    PLL1CON = pll1con;
    PLL0CON1 = pll0con1;
    PLL1CON1 = pll1con1;
    PLL2CON  = pll1con;
    PLL2CON1 = pll2con1;
    delay_us(10);

    USBCON0 = usbcon0;
    USBCON1 = usbcon1;
    AUANGCON1 = auang_con1;
    AUANGCON2 = auang_con2;
    AUANGCON3 = auang_con3;
    AUANGCON4 = auang_con4;
    AUANGCON5 = auang_con5;
    AUANGCON6 = auang_con6;
    GPIOADE = pa_de;
    GPIOBDE = pb_de;
    GPIOEDE = pe_de;
    GPIOFDE = pf_de;
    GPIOGDE = pg_de;
    GPIOAPU = gpioapu;
    GPIOBPU = gpiobpu;

    WDT_EN();
    CLKCON0 = clkcon0;
    set_sys_clk(sys_clk);           //还原系统时钟
    printf("wake up,continue run\n");

    adc_cb.channel = adc_ch;
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1, 1);

    if (DAC_FAST_SETUP_EN) {
        bsp_loudspeaker_mute();
        delay_5ms(2);
    }
    dac_restart();
    adpll_init(DAC_OUT_SPR);		//恢复采样率
#if (DAC_OUT_SPR == DAC_OUT_48K)
    DACDIGCON0 |= BIT(1);           //dac out sample 48K
#else
    DACDIGCON0 &= ~BIT(1);          //dac out sample 44.1K
#endif

    bsp_change_volume(sys_cb.vol);
    gui_init();
    gui_display(DISP_POWERON);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
#endif
    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;

    //TEST RTC CLOCK
//    while(1) {
//        static u32 time_old = 0;
//        u32 time_cur = RTCCNT;
//        if (time_old != time_cur) {
//            time_old = time_cur;
//            printf("e:[%d]", time_cur);
//        }
//    }
}
