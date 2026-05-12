#ifndef _API_TKEY_H
#define _API_TKEY_H

enum{
    NO_PRESS,           //无按下
    KSU_SHORT_PRESS,    //短按抬起,手指抬起时才返回
    KLU_LONG_PRESS,     //长按抬起
    KLLU_LLONG_PRESS,   //超长按/连按抬起
    KS_SHORT_PRESS,     //短按状态,扫描次数一旦达到短按阈值就返回
    KL_LONG_PRESS,      //长按状态
    KLH_LONG_PRESS,     //持续长按
    KLL_LLONG_PRESS,    //超长按状态
};

typedef struct {
    u16 cdpr;
    u8 in_ear;                      //入耳检测
    u8 resv;
} tkey_ch_t;

typedef struct {
    //scanning method para
    u16 tkpress_thd;           //touch key press threshold
    u16 llong_press_times;     //超长按/连按时间设置(次数)
    u16 long_press_times;      //长按时间设置(次数)
    u16 short_press_times;     //短按时间设置(次数)
    u8 frq_long_press;         //长按返回灵敏度, 值越小, 相同时间内返回值越多,1为最灵敏,取值范围[1,255]
    u16 cail_tkcnt;            //校准计数延时,扫描按键时递减,减到5以下校准4个通道
    u16 cail_dwthd_left;       //低异常校准下限,tkcnt_val如果减小了,且减小的值在[cail_dwthd,cail_dwthd_right]范围内则被认为需要校准
    u16 cail_dwthd_right;      //低异常校准上限
    u16 cail_upthd_left;       //高异常校准下限,tkcnt_val如果增大了,且增加得值在[cail_upthd_left,cail_upthd_right]范围内则被认为需要校准
    u16 cail_upthd_right;      //高异常校准上限
    u16 cali_thd;              //ttimes_cali积累到该值则进行校准
    //scanning method result
    u16 tkcnt_val[4];          //各通道中断挂起后存储各通道的TKCNT值
    u16 ttimes[4];             //扫描到的按下次数积累
    u16 ttimes_cali[4];        //需要校准的次数积累
}scan_tkcb_t;

typedef struct {
    const tkey_ch_t *key[4];
    union {
        struct {
            u32 fil_low     :  4;
            u32 fil_high    :  4;
            u32 fil_except  :  8;
            u32 fil_val     :  4;
            u32 to_except   :  11;
        };
        u32 reg_tktmr;
    };
    union {
        struct {
            u32 tkpthd      :   12;     //touch key press threshold
            u32 resv0       :   4;
            u32 tkrthd      :   12;     //touch key release threshold
            u32 tkfathd     :   4;      //touch key fine adjust threshold
        };
        u32 reg_tkpthd;
    };
    union {
        struct {
            u32 tksthd      :   12;     //touch key smaller threshold
            u32 resv1       :   4;
            u32 tklthd      :   12;     //touch key larger threshold
            u32 resv2       :   4;
        };
        u32 reg_tkethd;
    };

    //in ear
    union {
        struct {
            u32 ear_fil_low     :  4;
            u32 ear_fil_high    :  4;
            u32 ear_fil_except  :  8;
            u32 ear_fil_val     :  4;
            u32 tkpwup          :  6;
        };
        u32 reg_tetmr;
    };
    union {
        struct {
            u32 tepthd      :   12;     //touch ear press threshold
            u32 resv3       :   4;
            u32 terthd      :   12;     //touch ear release threshold
            u32 tefathd     :   4;
        };
        u32 reg_tepthd;
    };
    union {
        struct {
            u32 testhd      :   12;     //touch ear smaller threshold
            u32 resv5       :   4;
            u32 telthd      :   12;     //touch ear larger threshold
            u32 resv6       :   4;
        };
        u32 reg_teethd;
    };
    //scanning method
    scan_tkcb_t *scan_tkcb;
} tkey_cfg_t;

typedef union {
    struct {
        u32 rtrim           : 2;
        u32 ctrim           : 6;
        u32 itrim           : 5;
        u32 resva           : 19;
    };
    u32 tka_trim;
} tkacon_trim_t;

///用于蓝牙SPP调试
typedef struct {
    u8  te_exp;                     //touch ear exception pending
    u8  tk_exp;                     //touch key exception pending
    u16 tebcnt;
    u16 tkbcnt;
volatile u32 flag;
} tk_pnd_cb_t;
extern tk_pnd_cb_t tk_pnd_cb;


int tkey_init(void *tkey_cfg, u32 first_pwron);
void tkey_tebcnt_set(u16 val);
void spp_inpcon_tx(void);

#endif // _API_TKEY_H

