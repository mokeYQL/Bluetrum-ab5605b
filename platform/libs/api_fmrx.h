#ifndef _API_FMRX_H
#define _API_FMRX_H

#define FM_FREQ_MIN                         8750
#define FM_FREQ_MAX                         10800

typedef enum _FM_CS_FILTER{
    CS_FILTER_125K = 0,
    CS_FILTER_75K,
    CS_FILTER_50K,
    CS_FILTER_25K,
    CS_FILTER_MAX = CS_FILTER_25K,
}FM_CS_FILTER;

//内置收音判台阈值
typedef struct {
    u16 r_val;
    u16 z_val;
    u16 fz_val;
    u16 d_val;
} fmrx_thd_t;
extern fmrx_thd_t fmrx_thd;

void fmrx_dac_sync(void);
void fmrx_power_on(u32 val);
void fmrx_power_off(void);
void fmrx_analog_init(void);
void fmrx_analog_exit(void);
void fmrx_digital_stop(void);
void fmrx_digital_start(void);
void fmrx_set_freq(u16 freq);
u8 fmrx_check_freq(u16 freq);
void fmrx_logger_out(void);
s8 fmrx_rssi_pow_get(void);      //信号强度 //等同以前的 fmrx_rssi_dbu_get
s8 fmrx_get_rssi_powdb(void);     //db
void fmrx_set_audio_channel(u8 mono_flag);  //设置立体声或单声道解调输出  1:mono, 0:stereo
bool fmrx_cur_freq_is_stereo(void);         //检测当前收听台是不是立体声, 用于车机的ST标志显示(判断需要100ms)

void fmrx_rec_enable(void);
void fmrx_rec_start(void);
void fmrx_rec_stop(void);
void fmrx_rf_res_set(u32 res);     //天线阻抗匹配值,取值范围:0~3
//void fmrx_sel_cs_filter(u8 num);  //传用值为CS_FILTER_25K ~ CS_FILTER_125K

void fmrx_set_cs_filter(u8 num);   //取值范围见 T_FM_CS_FILTER:传入值为CS_FILTER_25K/50K/75K/125K
u8   fmrx_get_cs_filter(void);     //取值范围见 T_FM_CS_FILTER:

void fmrx_set_audio_channel_dual(u8 dual);  //设置解调输出配置: stereo(1) 或 mono(0)
u8   fmrx_get_audio_channel(void);          //获取解调输出配置: stereo(1) 或 mono(0)
#endif // _API_FMRX_H
