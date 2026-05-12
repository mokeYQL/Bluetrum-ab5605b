#ifndef _BSP_TKEY_H
#define _BSP_TKEY_H

#define TKC_MAX_BITS               2
#define TKC_MAX_SIZE              (1 << TKC_MAX_BITS)

typedef struct {
    u8  ch;                     //channel index
    u8  cnt;
    u8  limit;                  //方差阈值
    u8  stable_cnt;
    u8  te_flag;                //是否为入耳检测
    u8  range_thresh;           //rang校准的上限阈值
#if USER_TKEY_MUL_EN
    u8  cnts[4];
    u16 bufs[4][8];
    u8  stable_cnts[4];
    u8  tkey_pressed[4];
    u8  tkey_channels[4];
    u8  cur_ch_idx;
    u8  b_flags;
#endif
    u16 avg;                    //平均值
    u16 buf[8];
    u32 anov_cnt;               //满足方差条件计数
    u8  range_en;               //是否使能range校准
    u16 to_cnt;                 //定时校准时间
} tk_cb_t;
extern tk_cb_t tk_cb;
extern tk_cb_t te_cb;
extern u16 tkcnt_buf[4];

void bsp_tkey_init(void);
bool bsp_tkey_wakeup_en(void);
u8 tkey_get_scan(u8 ch);
u8 tkey_get_key(void);
u8 tkey_is_inear(void);     //入耳检测，0：无效，1：入耳，2：出耳
u8 tkey_wakeup_status(void);
void tkey_inear_disable(void);
void bsp_tkey_spp_tx(void);
void bsp_tkey_str_spp_tx(char *str);
u8 touchear_is_online(void);
bool touchear_is_wakeup(void);
bool tkey_is_pressed(void);
u8 bsp_tkey_scan(void);
u8 bsp_tkscan_get(void);

#endif
