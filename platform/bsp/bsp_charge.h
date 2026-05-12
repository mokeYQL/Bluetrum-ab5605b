#ifndef _BSP_CHARGE_H
#define _BSP_CHARGE_H

void bsp_charge_init(void);
void bsp_charge_off(void);
u8 charge_dc_detect(void);
void charge_detect(u8 mode);
void bsp_charge_sta(u8 sta);
void bsp_charge_set_stop_time(u16 stop_time);
void bsp_charge_bled_ctrl(void);
void internal_charge_disable(void);  //关闭内部充电,调用它关闭芯片内部充电.
#endif
