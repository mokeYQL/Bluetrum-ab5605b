#ifndef _BSP_FMRX_H
#define _BSP_FMRX_H

void fmrx_tmr1ms_isr(void);
void bsp_fmrx_init(void);
void bsp_fmrx_exit(void);
u8 bsp_fmrx_check_freq(u16 freq);
void bsp_fmrx_set_freq(u16 freq);
void bsp_fmrx_logger_out(void);
bool fmrx_is_playing(void);
u8 fmrx_sysclk_config(void);

#if FMRX_OPTIMIZE_TRY   //FM 收台效果尝试优化,可以修改CLK控制等,需要实际样机去测试效果
void fmrx_optimize_try_set(void);
void fmrx_optimize_try_recover(void);
#endif

#if FMRX_TEST_CHANNEL   //FM 固定某些电台测试,可用于对比其它样机,定位到特定的一些台对比声音清晰度
u16 fmrx_test_first_channel_get(void);
void fmrx_test_channel_switch(u8 dir);
#endif


#endif // _BSP_FMRX_H
