#include "include.h"
#include "func.h"
#include "func_clock.h"

u32 sys_get_rc2m_clkhz(u32 type);
#if PWR_RC_32K_EN
void param_rtcalarm_write(u8 *param);
void param_rtcalarm_read(u8 *param);
void param_rtcalarmsta_write(u8 *param);
void param_rtcalarmsta_read(u8 *param);
void param_rtcnsec_write(u8 *param);
void param_rtccnt_write(u8 *param);
void param_rtc_pwroffhz_write(u8 *param);
#endif // PWR_RC_32K_EN

#if FUNC_CLOCK_EN
rtc_time_t rtc_tm AT(.buf.rtc.clock);
time_cb_t tm_cb AT(.buf.rtc.clock);

AT(.text.func.clock.tbl)
const char week_str[7][4] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
};

AT(.text.func.clock.init)
void rtc_clock_init(void)
{
    rtc_tm.tm_year = (2019-1970);
    rtc_tm.tm_mon  = 7;
    rtc_tm.tm_mday = 11;
    rtc_tm.tm_hour = 23;
    rtc_tm.tm_min  = 59;
    rtc_tm.tm_sec  = 0;
    rtc_tm.tm_wday = get_weekday(rtc_tm.tm_year, rtc_tm.tm_mon, rtc_tm.tm_mday);
    RTCCNT = rtc_tm_to_time(&rtc_tm);
}

//多少秒后闹钟响
AT(.text.func.clock)
void rtc_set_alarm_relative_time(u32 nsec)
{
    rtc_time_to_tm(RTCCNT, &rtc_tm);            //更新时间结构体
    RTCALM = rtc_tm_to_time(&rtc_tm) + nsec;    //设置闹钟相对于当前时间n秒后
#if PWR_RC_32K_EN
    sys_cb.rtc_nsec = rtc_tm_to_time(&rtc_tm) + nsec;;     //记录闹钟相对于当前时间n秒后
    param_rtcnsec_write((u8*)&sys_cb.rtc_nsec);
#endif // PWR_RC_32K_EN
}

//设置多少秒后闹钟唤醒
AT(.text.func.clock)
void rtc_set_alarm_wakeup(u32 nsec)
{
    uint rtccon3 = RTCCON3;

    RTCCPND = BIT(17);                          //clear RTC alarm pending
    RTCCON9 = BIT(0);                           //clear alarm pending
    rtc_set_alarm_relative_time(nsec);

    rtccon3 |= BIT(8);                          //RTC alarm wakeup enable
    RTCCON3 = rtccon3;
}

#if PWR_RC_32K_EN
//多少秒后闹钟唤醒开机
AT(.text.func.clock)
void rtc_rc_set_alarm_relative_time(u32 nsec)
{
    rtc_time_to_tm(RTCCNT, &rtc_tm);            //更新时间结构体
    RTCALM = rtc_tm_to_time(&rtc_tm) + nsec;    //设置闹钟相对于当前时间n秒后
}


//省晶振，设置多少秒后闹钟唤醒开机
//nsec_alm：用户闹铃的RTCALM值，nsec_cal：rc闹钟唤醒校准时间
AT(.text.func.clock)
void rtc_rc_set_alarm_wakeup(u32 nsec_alm, u32 nsec_cal)
{
    uint rtccon3 = RTCCON3;

    RTCCPND = BIT(17);                             //clear RTC alarm pending
    RTCCON9 = BIT(0);                              //clear alarm pending

    sys_cb.pwroff_rtccnt = RTCCNT;
    param_rtccnt_write((u8*)&sys_cb.pwroff_rtccnt);

    u32 rtc_pwroffhz;
    u32 clk_cycle;
    if(nsec_alm > 0){                                              //有用户闹铃，要将用户闹铃拆分
        rtc_time_to_tm(RTCCNT, &rtc_tm);                           //更新时间结构体
        nsec_alm -= rtc_tm_to_time(&rtc_tm);                       //算出闹钟相对于当前时间n秒后
        u32 alarm_cnt = nsec_alm /nsec_cal;                        //将闹铃拆分成alarm_cnt个校准闹钟
        u32 alarm_remain = nsec_alm %nsec_cal;                     //alarm_cnt个校准闹钟后还要设置多长闹钟
        param_rtcalarm_read((u8*)&sys_cb.alarm_calcnt);
        if(sys_cb.alarm_calcnt != 0) {
            sys_cb.alarm_calcnt--;
        } else {
            sys_cb.alarm_calcnt = alarm_cnt;
        }
        if((alarm_cnt  == 0) || (sys_cb.alarm_calcnt == 0)) {      //用户闹铃小于10分钟校准闹钟
            rtc_pwroffhz = sys_get_rc2m_clkhz(1)/100;              //获取RC频率
            param_rtc_pwroffhz_write((u8*)&rtc_pwroffhz);
            clk_cycle = 31250000 / (rtc_pwroffhz / 96);            // (31200000) / (rtc_pwroffhz / 96)  96分频     //放大10W倍
            alarm_remain = ((alarm_remain * 100000) / clk_cycle);  //算出闹钟需要多少次RTCCNT计数来设置RTCALM才响闹铃
            rtc_rc_set_alarm_relative_time(alarm_remain);
            sys_cb.alarm_stat = 0;
            param_rtcalarmsta_write((u8*)&sys_cb.alarm_stat);
        } else if(sys_cb.alarm_calcnt){                            //有用户闹铃，设置10分钟校准闹钟
            rtc_pwroffhz = sys_get_rc2m_clkhz(1)/100;              //获取RC频率
            param_rtc_pwroffhz_write((u8*)&rtc_pwroffhz);
            clk_cycle = 31250000 / (rtc_pwroffhz / 96);            // (31200000) / (rtc_pwroffhz / 96)  96分频     //放大10W倍
            nsec_cal = ((nsec_cal * 100000) / clk_cycle);          //算出闹钟需要多少次RTCCNT计数来设置RTCALM才10分钟
            rtc_rc_set_alarm_relative_time(nsec_cal);
            if((sys_cb.alarm_calcnt == 1) && (alarm_remain == 0)) {
                sys_cb.alarm_stat = 0;
                sys_cb.alarm_calcnt--;
            } else {
                sys_cb.alarm_stat = 1;
            }
            param_rtcalarmsta_write((u8*)&sys_cb.alarm_stat);
        }
        param_rtcalarm_write((u8*)&sys_cb.alarm_calcnt);
    } else {                                                       //没有用户闹铃，设置10分钟的校准闹钟
        rtc_pwroffhz = sys_get_rc2m_clkhz(1)/100;                  //获取RC频率
        param_rtc_pwroffhz_write((u8*)&rtc_pwroffhz);
        clk_cycle = (31250000) / (rtc_pwroffhz / 96);              // (31200000) / (rtc_pwroffhz / 96)  96分频     //放大10W倍
        nsec_cal = ((nsec_cal * 100000) / clk_cycle);              //算出闹钟需要多少次RTCCNT计数来设置RTCALM才10分钟
        rtc_rc_set_alarm_relative_time(nsec_cal);
        sys_cb.alarm_stat = 1;
        param_rtcalarmsta_write((u8*)&sys_cb.alarm_stat);
    }

    rtccon3 |= BIT(8);                                             //RTC alarm wakeup enable
    RTCCON3 = rtccon3;
}
#endif //PWR_RC_32K_EN

#if LED_MATRIX_HUART_TX
extern u8 led_tx_buf[6];
AT(.text.func.clock)
void huart_tx_clock(void)
{
    led_tx_buf[0] = 'T';
    led_tx_buf[1] = 'I';
    led_tx_buf[2] = 'M';
    led_tx_buf[3] = tm_cb.index;
    led_tx_buf[4] = rtc_tm.tm_hour;
    led_tx_buf[5] = rtc_tm.tm_min;
    huart_putbuf(led_tx_buf, 6);
}
#endif

AT(.text.func.clock)
void func_clock_process(void)
{
    func_process();

//#if !GUI_LCD_EN
//    //通过ALARM每隔5秒打印一次时间，仅用于测试
//    if (RTCCON & BIT(17)) {
//        RTCCPND = BIT(17);                      //clear RTC alarm pending
//        RTCCON9 = BIT(0);                       //clear alarm pending
//        if (!tm_cb.setting) {
//            rtc_time_to_tm(RTCCNT, &rtc_tm);           //更新时间结构体
//            RTCALM = rtc_tm_to_time(&rtc_tm) + 4;                          //设置闹钟相对于当前时间5秒后
//            printf("time:  %d.%02d.%02d   %02d:%02d:%02d  %s\n", (rtc_tm.tm_year+1970), rtc_tm.tm_mon, rtc_tm.tm_mday,
//                       rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec, week_str[rtc_tm.tm_wday]);
//        }
//    }
//#endif
}

static void func_clock_enter(void)
{
    func_process();
    memset(&tm_cb, 0, sizeof(tm_cb));

#if LED_MATRIX_HUART_TX
    huart_tx_mode(2);
#endif

    func_cb.mp3_res_play = mp3_res_play;
    sys_cb.rtc_first_pwron = 0;
    rtc_time_to_tm(RTCCNT, &rtc_tm);         //更新时间结构体
    RTCALM = rtc_tm_to_time(&rtc_tm)+4;      //设置闹钟相对于当前时间5秒后
    func_clock_enter_display();
#if WARNING_FUNC_CLOCK
    mp3_res_play(RES_BUF_CLOCK_MODE_MP3, RES_LEN_CLOCK_MODE_MP3);
#endif // WARNING_FUNC_CLOCK

#if MICAUX_ANALOG_OUT_ALWAYS
    micaux_analog_out_init();
#endif // MICAUX_ANALOG_OUT_ALWAYS
}

static void func_clock_exit(void)
{
    func_clock_exit_display();
    func_cb.last = FUNC_CLOCK;
}

AT(.text.func.clock)
void func_clock(void)
{
    printf("%s\n", __func__);

    func_clock_enter();

    while (func_cb.sta == FUNC_CLOCK) {
        func_clock_process();
        func_clock_message(msg_dequeue());
        func_clock_display();
    }

    func_clock_exit();
}
#endif  //FUNC_CLOCK_EN
