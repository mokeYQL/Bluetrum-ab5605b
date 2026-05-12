#ifndef _API_SYSCLK_H_
#define _API_SYSCLK_H_


void dac_clk_source_sel(u32 val);
u8 get_cur_sysclk(void);
void set_sys_clk(u32 clk_sel); //设置当前系统时钟,传入参数:SYS_2M ~ SYS_160M
u32 get_sys_clk(void);  //得到当前系统时钟，返回值:SYS_2M ~ SYS_160M
u8 pll1_disable(void);
void pll1_enable(void);
void adpll_2_usb_clk(void);
void adpll_freq_update(u8 flag);
#endif
