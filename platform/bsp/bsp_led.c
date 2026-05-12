#include "include.h"
#include "bsp_led.h"

#if LED_DISP_EN

typedef struct {
    u8 rled_sta;   //红灯 亮-灭控制
    u8 bled_sta;   //蓝灯 亮-灭控制
    u8 unit;        //50ms单位 (范围：1~255)
    u8 period;      //间隔周期 (范围：0~255)


    u16 circle;     //一个周期(n-1)
    u16 cnt;        //已运行
    u8 wait;

    //u8 cnt;
    //u8 bcnt;
    //u8 space_cnt;

    u8 flag;
} led_cb_t;

led_cb_t led_cb AT(.buf.led);
led_cb_t led_bak AT(.buf.led);
void bt_set_sync_info(u8 *buf);

void m_led2_port_init(gpio_t *g, bool ext_res);
void m_port_2led_scan(gpio_t *g);
void led_cfg_port_init(gpio_t *g);
void led_cfg_set_onoff(gpio_t *g, bool onoff);

//50ms调用周期
AT(.com_text.led_disp)
void led_scan(void)
{
    uint bcnt;
    if (!is_led_scan_enable()) {
        return;
    }
    if (led_cb.wait) {
        led_cb.wait--;
        return;
    }

    //等待间隔时间
    bcnt = led_cb.cnt / led_cb.unit;
    if (bcnt <= 7) {
        if (led_cb.bled_sta & BIT(bcnt)) {
            bled_set_on();
        } else {
            bled_set_off();
        }
        if (led_cb.rled_sta & BIT(bcnt)) {
            rled_set_on();
        } else {
            rled_set_off();
        }
    }
    if (led_cb.cnt < 0xffff) {
        led_cb.cnt++;
        if (led_cb.cnt > led_cb.circle) {
            led_cb.cnt = 0;
        }
//        if (led_cb.cnt == 0xffff) {
//            //只亮一次
//            led_cb.bled_sta = 0;
//            led_cb.rled_sta = 0;
//            led_cb.period = 0;
//        }
    }
}

#if BT_TWS_EN
AT(.com_text.led_disp)
void led_sync(void)
{
    u8 info[8];
    u8 *p = info;
    memset(info, 0, 8);
    PUT_LE16(p, led_cb.cnt);
    bt_set_sync_info(info);
}

AT(.com_text.led_disp)
void led_sync_set(int cnt)
{
    if (cnt <  0 || cnt > led_cb.circle) {
        cnt = 0;                //最简单的方式处理异常
    }
    led_cb.cnt = cnt;
    led_cb.wait = 0;
}
#endif

//获取LED当前设置的状态
AT(.com_text.led_disp)
u8 get_led_sta(u32 type)
{
    if (type) {
        return led_cb.rled_sta;
    } else {
        return led_cb.bled_sta;
    }
}

AT(.com_text.led_disp)
void led_cfg_set_on(gpio_t *g)
{
    led_cfg_set_onoff(g, 1);
}

AT(.com_text.led_disp)
void led_cfg_set_off(gpio_t *g)
{
    led_cfg_set_onoff(g, 0);
}

#if USER_ADKEY_MUX_LED
AT(.text.led_disp)
void led_mux_cfg_port_init(gpio_t *g)
{
    if ((g == NULL) || (g->sfr == NULL)) {
        return;
    }
    g->type = 2;    //ADKEY_MUX
    led_cfg_port_init(g);
}
#endif // USER_ADKEY_MUX_LED

#if LED_PWR_EN
//一个IO口推两个LED灯
void led2_port_init(gpio_t *g)
{
    m_led2_port_init(g, !xcfg_cb.port_2led_resless_en);
}

//每ms扫描一次
AT(.com_text.led_disp)
void port_2led_scan(void)
{
    if ((!sys_cb.port2led_en)) {
        return;
    }
    m_port_2led_scan(&bled_gpio);
}

//1个IO推两个灯，充电状态下不进sleep，否则充电灯异常。
AT(.sleep_text.led)
bool port_2led_is_sleep_en(void)
{
    if ((sys_cb.port2led_en) && (CHARGE_DC_IN())) {
        return false;
    }
    return true;
}
#endif // LED_PWR_EN

AT(.com_text.led_disp)
void bled_set_off(void)
{
    if (!sys_cb.charge_bled_flag) {       //充满状态优先控制
        LED_SET_OFF();
    }
}

AT(.com_text.led_disp)
void bled_set_on(void)
{
   if (!sys_cb.charge_bled_flag) {        //充满状态优先控制
        LED_SET_ON();
    }
}

AT(.com_text.led_disp)
void rled_set_off(void)
{
    if (!sys_cb.charge_sta) {             //充电状态优先控制
        LED_PWR_SET_OFF();
    }
}

AT(.com_text.led_disp)
void rled_set_on(void)
{
   if (!sys_cb.charge_sta) {               //充电状态优先控制
        LED_PWR_SET_ON();
    }
}

AT(.text.led_disp)
void led_init(void)
{
    LED_INIT();
    LED_PWR_INIT();
    memset(&led_cb, 0, sizeof(led_cb));
}

//蓝灯灭
AT(.text.led_disp)
void led_off(void)
{
    led_cb.bled_sta = 0x00;
    bled_set_off();
}

//蓝灯亮
AT(.text.led_disp)
void led_on(void)
{
    led_cb.bled_sta = 0xff;
    bled_set_on();
}

AT(.text.led_disp)
void rled_on(void)
{
    led_cb.rled_sta = 0xff;
    rled_set_on();
}

AT(.text.led_disp)
void rled_off(void)
{
    led_cb.rled_sta = 0x00;
    rled_set_off();
}

//拔出充电红灯灭
AT(.com_text.led_disp)
void charge_led_off(void)
{
    LED_PWR_SET_OFF();
}

//充电红灯亮
AT(.com_text.led_disp)
void charge_led_on(void)
{
    LED_PWR_SET_ON();
}

//充满蓝灯常亮
void charge_bled_on(void)
{
    LED_SET_ON();
}

AT(.com_text.led_disp)
void led_set_sta(u8 rled_sta, u8 bled_sta, u8 unit, u8 period)
{
    led_cb_t *s = &led_cb;

    if (s->flag & LED_TOG_LBAT) {                   //低电优先闪灯
        s = &led_bak;
    }

    s->wait = 0xff;        //避免中断同时操作led_cb

    rled_set_off();
    bled_set_off();

    s->rled_sta = rled_sta;
    s->bled_sta = bled_sta;
    s->unit = unit;
    s->period = period;

    if (s->period == 0xff) {
        s->circle = 0xffff;
    } else {
        s->circle = s->unit * 8 + s->period;
        if (s->circle > 0) {
            s->circle--;
        }
    }

    s->cnt = 0;
#if BT_TWS_EN
    if (bt_tws_is_slave()) {
        s->wait = 20;                           //副耳等待同步时间：1s
    } else {
        s->wait = 0;
    }
    led_sync();
    bt_tws_sync_ledcnt();      //同步闪灯信息
#endif
}

AT(.com_text.led_disp)
void led_cfg_set_sta(led_cfg_t *cfg_cb)
{
    led_set_sta(cfg_cb->redpat, cfg_cb->bluepat, cfg_cb->unit, cfg_cb->cycle);

}

#if LED_LOWBAT_EN
AT(.text.led_disp)
void led_lowbat(void)
{
    if (!(led_cb.flag & LED_TOG_LBAT)) {
        memcpy(&led_bak, &led_cb, sizeof(led_cb_t));
        if (xcfg_cb.led_lowbat_config_en) {
            led_cfg_set_sta((led_cfg_t *)&xcfg_cb.led_lowbat);
        } else {
            led_set_sta(0xaa, 0x00, 6, 0);              //红灯300ms周期闪烁
        }
        led_cb.flag |= LED_TOG_LBAT;
    }
}

AT(.text.led_disp)
void led_lowbat_recover(void)
{
    if (led_cb.flag & LED_TOG_LBAT) {
        memcpy(&led_cb, &led_bak, sizeof(led_cb_t));
        led_cb.flag &= ~LED_TOG_LBAT;
    }
}
#endif // LED_LOWBAT_EN

void led_lowbat_follow_warning(void)
{
    if (!(led_cb.flag & LED_TOG_LBAT)) {
        memcpy(&led_bak, &led_cb, sizeof(led_cb_t));
        led_set_sta(0x0a, 0x00, 2, 0xff);              //红灯100ms周期闪2次
        led_cb.flag |= LED_TOG_LBAT;
    }
}

#endif
