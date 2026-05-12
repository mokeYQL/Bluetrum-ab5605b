#include "include.h"

#if CHARGE_EN

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DC_IN()                  ((RTCCON >> 20) & 0x01)            //VUSB Online state:    0->not online, 1->online

typedef struct {
    u8 chag_sta   : 3;          //充电状态
    u8 dcin_reset : 1;          //DC插入复位
    u8 const_curr : 4,          //恒流充电电流
       trick_curr : 4;          //涓流充电电流

    u8 stop_curr : 3,           //充电截止电流
       stop_volt : 2;           //充电截止电压
    u8 trick_curr_en    : 1;    //是否使能涓流充电
    u8 trick_stop_volt  : 1;    //涓流充电截止电压
    u8 bled_on_pr;
    volatile u16 stop_time;     //仅电压充满的超时时间
} charge_cfg_t;
charge_cfg_t charge_cfg;

void charge_stop(u8 mode);
void charge_init(charge_cfg_t *p);

//充满电亮蓝灯时间table表
AT(.rodata.charge)
const u8 ch_bled_timeout_tbl[8] = {0, 10, 20, 30, 60, 120, 180, 255};


//电池电压TRIM offset 返回0 = 4.2V，1 = 4.238V ， -1 是(4.2 - 0.038)，每一档0.038,以此类推
//范围-15 ~ 15
//AT(.text.voltage.trim.offset)
//int vbat_get_trim_offset(void)
//{
//    return 0;
//}

//充满电亮蓝灯时间控制
AT(.com_text.charge)
void bsp_charge_bled_ctrl(void)
{
    if (sys_cb.charge_bled_flag) {
        if (sys_cb.ch_bled_cnt) {
            if (sys_cb.ch_bled_cnt != 255) {
                sys_cb.ch_bled_cnt--;
            }
        } else {
            sys_cb.charge_bled_flag = 0;
            bled_set_off();
        }
    }
}

#if TRACE_EN
AT(.com_rodata.tkey)
const char charge_on_str[] = "Charge on\n";
AT(.com_rodata.tkey)
const char charge_off_str[] = "Charge off: %d\n";
#endif

AT(.text.charge_com.sta)
u8 charge_vddio_set(void)   //return 0:2.4V 1:2.5V  2:2.6V ------>12:3.8V   step = 0.1V
{
    if (sys_cb.vbat >= 2700) {
        return 8;
    } else
    return 0;
}

AT(.text.charge_com.sta)
void bsp_charge_sta(u8 sta)
{
    if (sta == 1) {
        //充电开启
        sys_cb.charge_sta = 1;
        sys_cb.charge_bled_flag = 0;
        charge_led_on();
        TRACE(charge_on_str);
    } else {
        //充电关闭
        TRACE(charge_off_str, sta);
        sys_cb.charge_sta = 0;
        sys_cb.charge_bled_flag = 0;
        charge_led_off();
        if (sta == 2) {
            sys_cb.charge_sta = 2;
            //充电满亮蓝灯
            if (BLED_CHARGE_FULL) {
                sys_cb.charge_bled_flag = 1;
                sys_cb.ch_bled_cnt = charge_cfg.bled_on_pr;
                charge_bled_on();
            }
        }
    }
}

void bsp_charge_off(void)
{
    if (charge_cfg.chag_sta >= 3) {
        charge_stop(0);
    }
}

AT(.text.charge_com.det)
u8 charge_dc_detect(void)
{
    if (!xcfg_cb.charge_en) {
        return 0;
    }
    return (RTCCON >> 20) & 0x01;
}

void bsp_charge_set_stop_time(u16 stop_time)
{
    charge_cfg.stop_time = stop_time;
}

void bsp_charge_init(void)
{
    charge_cfg_t *p = &charge_cfg;
    memset(p, 0, sizeof(charge_cfg_t));

    p->trick_curr_en = CHARGE_TRICK_EN;
    p->const_curr = CHARGE_CONSTANT_CURR & 0x0f;
    p->trick_curr = CHARGE_TRICKLE_CURR & 0x0f;
    p->stop_curr = CHARGE_STOP_CURR & 0x07;
    p->stop_volt = CHARGE_STOP_VOLT & 0x01;
    p->dcin_reset = CHARGE_DC_RESET & 0x01;
    p->trick_stop_volt = CHARGE_TRICK_STOP_VOLT;
    p->bled_on_pr = ch_bled_timeout_tbl[BLED_CHARGE_FULL];
    p->stop_time = 18000;                               //默认30分钟

    RTCCON3 &= ~BIT(12);                                //RTCCON3[12], INBOX Wakeup disable
    RTCCON7 &= ~0xf0;                                   //RTCCON7[7:4], PWRUPICH = 10mA
    RTCCON8 |= BIT(4);                                  //inbox voltage selection.  1'b1,  1.7v;   1'b0, 1.1v

    charge_init(p);

    if (SUPPORT_EXT_CHARGE_IC) {
        internal_charge_disable(); //支持外接充电IC时,外接充电IC和芯片充电引脚并连在一起,此时会关掉内部充电. "插入DC复位系统" 或 "插入DC禁止软开机" 则可以根据需要在配置工具中配置
    }
}
#endif // CHARGE_EN
