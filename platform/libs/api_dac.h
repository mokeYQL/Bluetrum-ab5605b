#ifndef _API_DAC_H_
#define _API_DAC_H_

void dac_cb_init(u32 dac_cfg);
void dac_set_volume(u8 vol);
void dac_set_dvol(u16 vol);
u16  dac_cb_dvol_get(void);
void dac_fade_process(void);
void dac_aubuf_clr(void);
void dac_fade_out(void);
void dac_fade_in(void);
void dac_fade_wait(void);
void dac_analog_fade_in(void);
void dac_digital_enable(void);
void dac_digital_disable(void);
void dac_spr_set(uint spr);
void dac_src1_spr_set(uint spr);
void dac_obuf_init(void);
void dac_src1_init(void);
void dac_power_on(void);
void dac_restart(void);
void dac_power_off(void);
void dac_set_mono(void);
void dac_mono_init(bool dual_en, bool lr_sel);
void adpll_init(u8 out_spr);
bool adpll_spr_set(u8 out48k_flag);
void obuf_put_one_sample(u32 left, u32 right);
void obuf_put_samples(void *buf, uint len);
void dac_unmute_set_delay(u16 delay);
u16 dac_pcm_pow_calc(void);  //硬件计算能量,很快,us级别 //需要避免和 dac_dnr_detect 再同一时间调用 (两个函数用到同一硬件模块)，否则能量值不准
void sniff_dac_enable(void);
void sniff_dac_disable(void);
void dac_dnr_detect(void);
void music_src_set_volume(u16 vol);
void aux2mic_set_gain(u8 gain);                 //0~24共25级可调,0会断开通道,(-6~63DB) (step: 3db)
void dac_set_balance(u16 l_vol, u16 r_vol);     //调左右平衡，参数输入范围：0~0x7fff
void dac_channel_configure(void);
void dac_channel_exchange(void);                //DAC左右声道交换
void dac_power_on_fast(void);                   //快速打开dac电源   //dac_power_on之后调用才有效
void dac_power_off_fast(void);                  //快速关闭dac电源   //dac_power_on之后调用才有效
void dac_switch_for_bt(u8 enable);
//DRC
void dac_drc_init(void *cfg);
void dac_drc_exit(void);
void drc_set_param(const u32 *tbl);

//EQ
struct eq_coef_tbl_t {
    const int * coef_0;                         //tbl_alpha
    const int * coef_1;                         //tbl_cos_w0
};

void eq_var_init(void *cfg);

void music_set_eq_by_res(u32 *addr, u32 *len);
void music_set_eq_by_num(u8 num);
void music_eq_off(void);
void music_set_eq(u8 band_cnt, const u32 *eq_param);
void music_set_eq_gain(u32 gain);
bool music_set_eq_is_done(void);    //判断上一次设置EQ是否完成，1：已完成

void mic_set_eq(u8 band_cnt, const u32 *eq_param);
bool mic_set_eq_by_res(u32 *addr, u32 *len);
void mic_set_eq_gain(u32 gain);
bool mic_set_eq_is_done(void);      //判断上一次设置EQ是否完成，1：已完成

void bass_treble_coef_cal(void *eq_coef, int gain, int mode);   //gain:-12dB~12dB, mode:0(bass), 1(treble)
void eq_coef_cal(void *eq_coef, int gain);           //index:0~7（8条EQ）, gain:-12dB~12dB
void analog_mic2pa_start(u8 channel, u8 mic2lpf_en);
extern uint8_t cfg_dac_pcm_ext_en;
void dac_aubufsize_set(u8 flag);

void mp3_pitch_shift_play_set(bool en);  //MP3变调播放使能,目前只测试了SPEAKER下录音到SPIFALSH并变调播放,使用方法见宏 EXSPI_REC_PITCH_SHIFT_PLAY
bool mp3_pitch_shift_play_get(void);     //查看MP3变调播放是否使能

void wav_output_tick_start(void);

typedef struct _T_SOFT_VOL{
    s32 vol_cur;
    s32 vol_set;
    u8 step;       //淡入淡出步进
    u8 type;
    u8 fade_flag;  //以淡入或淡出方式设置音量
    u8 rsv;
}T_SOFT_VOL;

void soft_vol_set(T_SOFT_VOL *hd, s32 vol);
s32 soft_vol_get(T_SOFT_VOL *hd);
s32 soft_vol_proc(T_SOFT_VOL *hd, s32 pcm);
#endif
