#ifndef _FUNC_SPDIF_H
#define _FUNC_SPDIF_H

typedef struct {
    u8  disp_update :   1;
} f_spdif_t;
extern f_spdif_t f_spdif;

typedef struct {
    u8 channel;
    u8 res[3];
}spd_cfg_t;

extern spd_cfg_t spd_cfg;

void func_spdif_message(u16 msg);
bool spdif_is_online(void);
void spdif_init(void *cfg);
void spdif_exit(void);
void spdif_stop(void);
void spdif_start(void);
void spdif_play_pause(void);
void spdif_process(void);
void spdif_shaping(bool shape_en);

void spdif_detect_init(u8 channel, u8 detsel, u8 dettmr);  //detsel,dettmr检测参数范围0~3，检测不稳定可将这2个参数相应调大
bool spdif_is_online_detect(void);  // 其他模式下SPDIF检测函数，开启spdif_detect_init有用
bool spdif_is_pcm_signal(void);
u32 get_spdif_baud(void);
s16* spdif_pcm_buf_get(void);  //buf中的数据是s16格式, 左右左右左右排列下去  //spdif接收循环buf,长度为4K //可以通过它计算收到的SPDIF信号能量值(比如间隔20ms计算一次,一次取其中200个样点进行计算),由于SPDIF收到信号后会直接推DAC,在SPDIF模式可以直接用dac_pcm_pow_calc获取的能量值即可视为SPDIF信号的能量值

void spdif_tx_init(void *cfg);
void spdif_tx_en(bool tx_en);
void func_spdif_tx_init(void);

#if (GUI_SELECT != GUI_NO)
void func_spdif_display(void);
void func_spdif_enter_display(void);
void func_spdif_exit_display(void);
#else
#define func_spdif_display()
#define func_spdif_enter_display()
#define func_spdif_exit_display()
#endif

#endif // _FUNC_SPDIF_H
