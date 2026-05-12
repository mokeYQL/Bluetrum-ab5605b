#include "include.h"

int s_abs(int x);

#if USER_TKEY
tk_cb_t tk_cb;

#if USER_TKEY_DEBUG_EN
typedef struct {
    u16 tk_buf[16];
    char spp_buf[128];
    u8  tk_idx;
    u8  ch;
volatile u8 tk_print_sta;
    u32 ticks;
} spp_tkcb_t;
spp_tkcb_t spp_tkcb;

//AT(.rodata.tkey)
//const char tkcnt_str[] = "[TK] State(%01x), TKBCNT(%03x):\n";
//AT(.rodata.tkey)
//const char tkbcnt_str[] = "TKBCNT: %x\n";
//AT(.rodata.tkey)
//const char tkpend_str[] = "TK ERR: %x\n";
#if USER_TKEY_MUL_EN
extern u16 tkbcnt[4];
extern scan_tkcb_t *pscan_tkcb;
#endif
//通过蓝牙SPP打印输出, 避免UART飞线干扰
void spp_tkcnt_tx(void)
{
#if USER_TKEY_HW_EN
    u16 tk_buf[8];
    int c_len = 0, i;

    if (spp_tkcb.tk_print_sta) {
        memset(spp_tkcb.spp_buf, 0, sizeof(spp_tkcb.spp_buf));
        memcpy(tk_buf, &spp_tkcb.tk_buf[(spp_tkcb.tk_print_sta - 1)*8], 16);
        spp_tkcb.tk_print_sta = 0;
        if (tk_cb.ch != 0xff) {
            c_len += sprintf(spp_tkcb.spp_buf + c_len, tkcnt_str, tkey_get_key(), (u16)(TKBCNT & 0xfff));
            for (i = 0; i < 8; i++) {
                c_len += sprintf(spp_tkcb.spp_buf + c_len, "%03x ", tk_buf[i]);
            }
            c_len += sprintf(spp_tkcb.spp_buf + c_len, "\n");
        }
        bt_spp_tx((uint8_t *)spp_tkcb.spp_buf, c_len);
    }
#elif USER_TKEY_MUL_EN
    char tk_buf[30];
    memset(tk_buf,0,sizeof(tk_buf));
    //打印触摸通道3数据
    snprintf(tk_buf, sizeof(tk_buf) - 1, "[TK]:%d %d %d %d\n", pscan_tkcb->tkcnt_val[3], pscan_tkcb->ttimes[3], pscan_tkcb->ttimes_cali[3], (int)tkbcnt[3]);
    bt_spp_tx((uint8_t *)tk_buf, 30);
#endif
}

//void spp_inpcon_tx(void)
//{
//    int c_len = 0;
//
//    if (tk_pnd_cb.flag & BIT(2)) {
//        tk_pnd_cb.flag &= ~BIT(2);
//        c_len += sprintf(spp_tkcb.spp_buf+c_len, tkpend_str, tk_pnd_cb.tk_exp);
//    }
//    if (tk_pnd_cb.flag & BIT(3)) {
//        tk_pnd_cb.flag &= ~BIT(3);
//        c_len += sprintf(spp_tkcb.spp_buf+c_len, tkbcnt_str, tk_pnd_cb.tkbcnt);
//    }
//    if (c_len) {
//        bt_spp_tx((uint8_t *)spp_tkcb.spp_buf, c_len);
//    }
//}

void bsp_tkey_spp_tx(void)
{
    spp_tkcnt_tx();                     //输出TKCNT
//    if (tick_check_expire(spp_tkcb.ticks, 100)) {
//        spp_tkcb.ticks = tick_get();
//        spp_inpcon_tx();                //输出一些状态
//    }
}

#endif // USER_TKEY_DEBUG_EN

#if USER_TKEY_HW_EN
u16 tkcnt_buf[4];
//AT(.com_rodata.tkey)
//const char tkey_timo_str[] = "->to: %x\n";
AT(.com_text.tkey.isr)
void tkey_timeout_calibration(tk_cb_t *s, u16 tkcnt)
{
    u16 tkbcnt = TKBCNT & 0xfff;
    if (s->anov_cnt) {
        if ((s->anov_cnt % s->to_cnt) == 0) {                 //8秒timeout
            if (s_abs((s16)tkcnt - (s16)tkbcnt) > 2) {
                TKBCNT = tkcnt;
//                printf(tkey_timo_str, tkcnt);
            }
        }
    }
}

//计数实时方差
AT(.com_text.tkey.isr)
u32 tkey_tcnt_variance_calc(tk_cb_t *s, u16 tkcnt)
{
    u32 i;
    u32 sum = 0, dela, variance = 0;

    s->buf[s->cnt++] = tkcnt;
    if (s->cnt > 7) {
        s->cnt = 0;
    }
    for (i = 0; i < 8; i++) {
        sum += s->buf[i];
    }
    s->avg = sum >> 3;
    for (i = 0; i < 8; i++) {
        if (s->buf[i] > s->avg) {
            dela = s->buf[i] - s->avg;
        } else {
            dela = s->avg - s->buf[i];
        }
        variance += (dela * dela);
    }
    return variance;
}

//根据方差检测进行range exception校准
AT(.com_text.tkey.isr)
void tkey_bcnt_range_exception(tk_cb_t *s, u16 tkcnt)
{
    u16 bcnt;
    u32 variance = tkey_tcnt_variance_calc(s, tkcnt);
    if (variance < s->limit) {
        s->anov_cnt++;
        bcnt = TKBCNT & 0xfff;

        if ((s->avg > (bcnt+3)) && (s->avg < (bcnt + s->range_thresh))) {
            s->stable_cnt++;
            if (s->stable_cnt >= 20) {
                if (s->range_en) {                  //range cali is enable?
                    TKBCNT = bcnt + 3;
                }
                s->stable_cnt = 0;
            }
        } else {
            s->stable_cnt = 0;
        }
    } else {
        s->anov_cnt = 0;
        s->stable_cnt = 0;
    }
}

AT(.com_text.tkey)
bool tkey_is_pressed(void)
{
    if (tk_cb.ch == 0xff) {
        return false;
    }
    if (!tkey_get_key()) {
        return false;
    }
    return true;
}

AT(.com_text.bsp.tkey)
u8 bsp_tkey_scan(void)
{
    u8 key = NO_KEY;

    if (tkey_is_pressed()) {
        key = KEY_PLAY;
    }

    return key;
}
#endif // USER_TKEY_HW_EN

#if USER_TKEY_MUL_EN
extern scan_tkcb_t scan_tkcb;

AT(.com_text.bsp.tkey)
void bsp_tkey_change_channel(void)
{
    u32 tkacon = TKACON;
    TKACON = 0;
    tkacon &= ~(0x3<<14);
    while (1) {
        tk_cb.cur_ch_idx++;
        if (tk_cb.cur_ch_idx >= 4) {
            tk_cb.cur_ch_idx = 0;
        }
        if (tk_cb.tkey_channels[tk_cb.cur_ch_idx]) {
            tk_cb.ch=tk_cb.cur_ch_idx;
            break;
        }
    }
    tkacon |=(tk_cb.ch<<14);
    TKACON=tkacon;
}

AT(.com_text.str)
const char str_short2[] = "touch key[%d] short press up   \n";
AT(.com_text.str)
const char str_long2[] ="touch key[%d] long press up   \n";
AT(.com_text.str)
const char str_long22[] = "touch key[%d] long press hold  \n";
//AT(.com_text.xxx)
//char tkcnt_str2[]="tk_ch:%d tkcnt:%d cali:%d\n";
AT(.com_text.bsp.tkey)
u8 bsp_tkscan_get(void)
{
    u16 key = NO_KEY;
    u8 value;
    //printf(tkcnt_str2,tk_cb.ch,scan_tkcb.tkcnt_val[tk_cb.ch],scan_tkcb.ttimes_cali[tk_cb.ch]);
    for(u8 i = 0;i < 4;i++){                        //触摸扫描方式,扫描多路触摸通道
        value = tkey_get_scan(i);
        if (value == KSU_SHORT_PRESS) {             //短按抬起
            key = KU_PLAY;
            printf(str_short2,i);
        } else if (value == KLU_LONG_PRESS) {       //长按抬起
            key = KL_PLAY;
            printf(str_long2,i);
        }else if (value == KLH_LONG_PRESS) {        //持续长按
            key = KH_PLAY;
            printf(str_long22,i);
        }
        if (key != NO_KEY) {
            msg_enqueue(key);
            key = NO_KEY;
        }
    }
    return 0;
}
#endif // USER_TKEY_MUL_EN

//中断调用
AT(.com_text.tkey.isr)
void tkey_tkcnt_isr(u32 tk_ch, u16 tkcnt)
{
#if USER_TKEY_MUL_EN
    if(tkcnt != 0)
    scan_tkcb.tkcnt_val[tk_cb.ch]=tkcnt;
#elif USER_TKEY_HW_EN
    tkcnt_buf[tk_ch] = tkcnt;
    if (tk_ch == tk_cb.ch) {
        tkey_bcnt_range_exception(&tk_cb, tkcnt);
        tkey_timeout_calibration(&tk_cb, tkcnt);
    #if USER_TKEY_DEBUG_EN
        spp_tkcb.tk_buf[spp_tkcb.tk_idx] = tkcnt;
    #endif
    }
#endif // USER_TKEY_MUL_EN

#if USER_TKEY_DEBUG_EN
    if (tk_ch == spp_tkcb.ch) {
        spp_tkcb.tk_idx++;
        if (spp_tkcb.tk_idx == 8) {
            spp_tkcb.tk_print_sta = 1;
        } else if (spp_tkcb.tk_idx >= 16) {
            spp_tkcb.tk_print_sta = 2;
            spp_tkcb.tk_idx = 0;
        }
    }
#endif
}

void bsp_tkey_var_init(void)
{
    memset(&tk_cb, 0, sizeof(tk_cb));
    tk_cb.limit = 18;
    tk_cb.range_thresh = TKRTHRESH-1;
    tk_cb.ch = TKEY_CH;
    tk_cb.to_cnt = 1000;                                //8秒
    tk_cb.range_en = 1;
#if USER_TKEY_MUL_EN
    tk_cb.tkey_channels[0] = TKEY_CH0_EN;
    tk_cb.tkey_channels[1] = TKEY_CH1_EN;
    tk_cb.tkey_channels[2] = TKEY_CH2_EN;
    tk_cb.tkey_channels[3] = TKEY_CH3_EN;
#endif

#if USER_TKEY_DEBUG_EN
    memset(&spp_tkcb, 0, sizeof(spp_tkcb));
    if (tk_cb.ch != 0xff) {
        spp_tkcb.ch = tk_cb.ch;
    }
#endif
}

void bsp_tkey_init(void)
{
    tkey_cfg_t tkey_var;
    bsp_tkey_var_init();
    if (tk_cb.ch != 0xff){
        memcpy(&tkey_var, &tkey_cfg, sizeof(tkey_cfg_t));

#if USER_TKEY_HW_EN
        tkey_var.key[tk_cb.ch] = &tkey0;
#endif // USER_TKEY_HW_EN

#if USER_TKEY_MUL_EN
        tkey_var.key[0] = &tkey0;
    #if TKEY_CH1_EN
        tkey_var.key[1] = &tkey0;
    #endif
    #if TKEY_CH2_EN
        tkey_var.key[2] = &tkey0;
    #endif
    #if TKEY_CH3_EN
        tkey_var.key[3] = &tkey0;
    #endif
    tkey_var.scan_tkcb = &scan_tkcb;
    bsp_tkey_change_channel();              //切换成初始通道
#endif // USER_TKEY_MUL_EN

        tkey_init((void *)&tkey_var, sys_cb.rtc_first_pwron);
#if USER_TKEY_HW_EN
        delay_5ms(4);
        TKBCNT = tkcnt_buf[tk_cb.ch];       //reset TKBCNT
#endif
    } else {
        RTCCON0 |= BIT(4);                  //TKITF_EN
        TKCPND = BIT(0);                    //inpwr_swrst
    }
}
#endif // USER_TKEY


