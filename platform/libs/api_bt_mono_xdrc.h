#ifndef __API_MONO_SOFTEQ_LPHPDRC_H
#define __API_MONO_SOFTEQ_LPHPDRC_H

#define BIT_XDRC_PREVOL         BIT(0)
#define BIT_XDRC_DYEQ1          BIT(1)
#define BIT_XDRC_DYEQ2          BIT(2)
#define BIT_XDRC_PREEQ          BIT(3)
#define BIT_XDRC_LPHP           BIT(4)
#define BIT_XDRC_ALLDRC         BIT(5)
#define BIT_XDRC_POSTEQ         BIT(6)
#define BIT_XDRC_FILTER_EXP     BIT(8)
#define BIT_XDRC_FILTER_ORDER4  BIT(9)

#define BIT_XDRC_ALL_EFFECT    (BIT(8)|BIT(9) | 0x7F)
void xdrc_audio_en_cfg(u32 bit_cfg, bool en);

//-------------------------------------------------------------------------
//SOFTEQ DRC APIS
u32 exlib_softeq_cb_init(void *cb, u32 cb_len, u32 res_addr, u32 res_len);
bool exlib_softeq_coef_update(void *cb, u32 cb_len, u32 *coef, u32 band, bool pregain); //pregain = 1 , first u32 is pregain,  next is band coef  //pregain=0, all coef is band coef
s32 exlib_softeq_proc(void *cb, s32 input, u32 is_24bit);
s16 exlib_softeq_stereo_proc_s16(void *cb, s16 data);
s32 exlib_drc_process_gain(void *cb, s32 data_in);
s16 exlib_drc_get_pcm16_s(s32 data, int g);
s32 exlib_drc_get_pcm32_s(s32 data, int g);
u32 exlib_softeq_band_get(void *cb);
//drc apis
void exlib_drcv1_cb_init(void *cb, u32 cb_len, u32 res_addr, u32 res_len); //drc资源文件更新cb参数
bool exlib_drcv1_coef_update(void* cb, u32 *tbl);      //在线调试更新参数
s16 exlib_drc_process_s16(void *cb, s32 data_in);     //饱和到16 bit
s32 exlib_drc_process_s24(void *cb, s32 data_in);     //饱和到24 bit  //undo
s16 exlib_s_clip16(s32 x);
//-------------------------------------------------------------------------
//以下cb数组长度需要和库中一对应
//max eq band 10
extern s32 xdrc_preeq_cb[10*7 + 6] AT(.xdrc_ram);  //*coef, *zpara, band, pre_gain, coef(5*BAND), calc_buf((BAND+1)*2)
//lp_cb
extern s32 xdrc_lp_cb[1*7 + 6] AT(.xdrc_ram);
extern s32 xdrc_lp_cb2[1*7 + 6] AT(.xdrc_ram);   //分频点过两次
//hp_cb
extern s32 xdrc_hp_cb[1*7 + 6] AT(.xdrc_ram);
extern s32 xdrc_hp_cb2[1*7 + 6] AT(.xdrc_ram);   //分频点过两次
//lp_cb
extern s32 xdrc_lp_exp_cb[1*7 + 6] AT(.xdrc_ram);
//hp_cb
extern s32 xdrc_hp_exp_cb[1*7 + 6] AT(.xdrc_ram);
//drc_cb
extern u8 xdrc_drclp_cb[14 * 4] AT(.xdrc_ram);
//drc_cb
extern u8 xdrc_drchp_cb[14 * 4] AT(.xdrc_ram);
//drc_cb
extern u8 xdrc_drcall_cb[14 * 4] AT(.xdrc_ram);

extern u32 dynamic_eq_cb1[200/4];
extern u32 dynamic_eq_cb2[200/4];

void xdrc_ram_clear(void);
//-------------------------------------------------------------------------
void pcm_dual_2_mon(u8 *buf, u32 samples, u32 is24bit);
void pcm_mon_2_dual(u8 *buf, u32 samples, u32 is24bit);
void xdrc_mono_process(u8 *buf, u32 samples, u32 is24bit);   //输入为单声数据
//-------------------------------------------------------------------------
u8* get_xdrc_adj_rxbuf_addr(void);
u32 get_xdrc_adj_rxbuf_len(void);
void bt_mono_xdrc_param_init(void);
void mono_xdrc_onlie_adjust_process(void);
bool xdrc_huart_rx_done_callback(u8 *rx_buf);


void xdrc_softvol_init(void);
void xdrc_sysvol_set(s32 vol);
void xdrc_softvol_set(s32 vol);
void xdrc_sysvol_en_set(u8 en);
u8 is_xdrc_sysvol_en(void);
bool is_xdrc_sysvol_fade_finish(void);
void xdrc_softvol_print(void);

void xdrc_prevol_set(u32 vol);
u32 xdrc_prevol_get(void);

void xdrc_online_adj_init(void);
//dyeq_api
u32  dynamic_eq_cb_len_get(void);
u32 dynamic_eq_init(void *cb_dyeq, u32 cb_len);
void dynamic_eq_update(void *cb_dyeq, s32* coef_in);
void dynamic_eq_process(s32 *samples, void *cb_dyeq);

void bt_mono_xdrc_init(void);
void xdrc_huart_dump_init(void);
void print_xdrc_audio_cfg(void);
#endif

