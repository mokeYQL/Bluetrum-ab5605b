#ifndef _API_SDADC_H
#define _API_SDADC_H

#define CHANNEL_L           0x0F
#define CHANNEL_R           0xF0

#define ADC2DAC_EN          0x01        //ADC-->DAC
#define ADC2SRC_EN          0x02        //ADC-->SRC

typedef void (*pcm_callback_t)(u8 *ptr, u32 samples, int ch_mode);

typedef struct {
    u8 channel;
    u8 sample_rate;
    u16 gain;                   //低5bit为sdadc digital gain, 其它bit为模拟gain控制
    u8 out_ctrl;
    u8 res;
    u16 samples;
    pcm_callback_t callback;
} sdadc_cfg_t;

enum {
    SPR_48000,
    SPR_44100,
    SPR_38000,
    SPR_32000,
    SPR_24000,
    SPR_22050,
    SPR_16000,
    SPR_12000,
    SPR_11025,
    SPR_8000,
};

void set_mic_analog_gain(u8 level, u8 channel);         //0~23(共24级), step 3DB (-6db ~ +63db)
void set_aux_analog_gain(u8 level, u8 channel);         //level: 0~4, 000(-6DB), 001(-3DB), 010(0DB), 011(+3DB), 100(+6DB)
void sdadc_dummy(u8 *ptr, u32 samples, int ch_mode);
void sdadc_pcm_2_dac(u8 *ptr, u32 samples, int ch_mode);
void sdadc_var_init(void);

int sdadc_init(const sdadc_cfg_t *p_cfg);
void change_aux_channel(u8 channel);
int sdadc_start(u8 channel);
int sdadc_exit(u8 channel);

//AUX直通PA
//AUX单声道直通，自动选择“单进单出”与“单进双出”配置。
int analog_aux2pa_start(u8 channel);
void analog_aux2pa_exit(u8 channel);
void analog_aux2pa_exit_karaok(u8 channel);
void analog_aux2pa_dig2anl_enable(void);
void analog_aux2pa_dig2anl_disable(void);
void aux2pa_sdadc_enable(void);     //AUX直通时打开SDADC
void aux_shorting_enable(void);
void aux_shorting_disable(void);
void aux_channel_mute(void);
void aux_channel_unmute(void);
void set_aux_analog_vol(u8 gain, u8 channel);

void mic_bias_var_init(void);
void mic_bias_trim(void);
bool mic_bias_trim_is_en(void);
u8 mic_bias_trim_sta(void);

void sdadc_pcm_peri_eq(u8 *ptr, u32 samples);
#endif //_API_SDADC_H

void aux_channel_exchange_start(u8 channel);   //aux 直通时，左右声道互换示例  //aux_channel_exchange_start(get_aux_channel_cfg());
void aux_channel_exchange_stop(u8 channel);
