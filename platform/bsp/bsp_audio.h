#ifndef __BSP_AUDIO_H__
#define __BSP_AUDIO_H__

#define AUDIO_PATH_AUX             0
#define AUDIO_PATH_SPEAKER         1
#define AUDIO_PATH_BTMIC           2
#define AUDIO_PATH_USBMIC          3
#define AUDIO_PATH_KARAOK          4
#define AUDIO_PATH_MICTRIM         5
#define AUDIO_PATH_TOM_CAT         6

void audio_path_init(u8 path_idx);
void audio_path_exit(u8 path_idx);
void audio_path_start(u8 path_idx);
u8 get_aux_channel_cfg(void);
void aux2mic_path_init(u8 enable);  //AUX2MIC 直通配置


//以下API的使用,需要增加外部独立DRC及软件EQ库
void exlib_drc_v1_init(u32 para);    //0x00:(L+R)/2->DRC   //0x01:L->DRC,R->DRC
void exlib_drc_v1_set_by_res(u32 *addr, u32* len);
void exlib_drc_v1_coef_set(u32 *tbl);  //drc online debug
void exlib_drc_v1_stereo_process(s32 *ldata, s32 *rdata);
void exlib_drc_v1_mono_process(s32 *data);

void exlib_soft_eq_init(u8 total_band);     //最大支持的eq band数量, 软件EQ一般不建议超过7个band
void music_set_soft_eq_gain(u32 gain);      //0DB 32767
u8 exlib_soft_eq_total_band_get(void);      //得到当前软件EQ支持多少个band
void exlib_soft_eq_total_band_set(u8 total_band); //设置最大能支持多少个band，最大不超过10 band
bool exlib_soft_eq_set_by_res(u32 *addr, u32 *len);
void exlib_soft_eq_band_set(u8 band_cnt,const u32 *eq_coef_buf);  //online eq debug
void exlib_soft_eq_stereo_proc_s16(s16 *ldata, s16 *rdata);
void exlib_soft_eq_stereo_proc_s32(s32 *ldata, s32 *rdata);
void exlib_soft_eq_mono_proc_s16(s16 *ldata);
void exlib_soft_eq_mono_proc_s32(s32 *ldata);

#endif //__BSP_AUDIO_H__

