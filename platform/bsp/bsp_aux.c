#include "include.h"

typedef struct {
    u8  anl2pa      :   1,          //AUX模拟直通
        dig2anl_dis :   1;          //是否关闭DAC digital to pa通路, 模拟直通时关闭
} bsp_aux_cb_t;
static bsp_aux_cb_t bsp_aux_cb AT(.buf.aux);

//AUX模拟直通start
AT(.text.bsp.aux)
void bsp_aux_anl2pa_start(u8 channel, u8 aux_anl_gain)
{
    bsp_aux_cb_t *s = &bsp_aux_cb;

    analog_aux2pa_start(channel);
    set_aux_analog_gain(aux_anl_gain, channel);         //set aux analog gain
    s->dig2anl_dis = 1;
#if SYS_KARAOK_EN
    analog_aux2pa_dig2anl_enable();           //KARAOK时不能关数字DAC
    s->dig2anl_dis = 0;
#endif
    s->anl2pa = 1;                                      //模拟直通start
}

//AUX模拟直通stop
AT(.text.bsp.aux)
void bsp_aux_anl2pa_stop(u8 channel)
{
    bsp_aux_cb_t *s = &bsp_aux_cb;

#if SYS_KARAOK_EN
    analog_aux2pa_exit_karaok(channel);
#else
    analog_aux2pa_exit(channel);
#endif // SYS_KARAOK_EN

    s->anl2pa = 0;
    s->dig2anl_dis = 0;
}


AT(.text.bsp.aux)
void bsp_aux_start(u8 channel, u8 aux_anl_gain, u8 aux2adc)
{
 //   printf("%s\n", __func__);
    dac_fade_out();
    dac_fade_wait();                                            //等待淡出完成
    u8 path = aux2adc & 0x3f;
    if (aux2adc & BIT(7)) {                                     //AUX to SDADC
        audio_path_init(path);
        audio_path_start(path);
    } else {
        bsp_aux_anl2pa_start(channel, aux_anl_gain);            //AUX直通
        if (aux2adc & BIT(6)) {                                 //模拟直通，需要同时开启SDADC用于录音
            audio_path_init(path);
            aux2pa_sdadc_enable();
        }
    }
    dac_fade_in();
}

AT(.text.bsp.aux)
void bsp_aux_stop(u8 channel, u8 aux2adc)
{
 //   printf("%s\n", __func__);
    dac_fade_out();
    dac_fade_wait();
    u8 path = aux2adc & 0x3f;
    if (aux2adc & BIT(7)) {
        audio_path_exit(path);
    } else {
        bsp_aux_anl2pa_stop(channel);
        if (aux2adc & BIT(6)) {
            audio_path_exit(path);
        }
    }
}

//处理AUX直通时播放WAV提示音
AT(.text.bsp.aux)
void bsp_aux_piano_start(void)
{
    bsp_aux_cb_t *s = &bsp_aux_cb;
    if (s->dig2anl_dis) {
        analog_aux2pa_dig2anl_enable();
    }
}

AT(.text.bsp.aux)
void bsp_aux_piano_exit(void)
{
    bsp_aux_cb_t *s = &bsp_aux_cb;
    if (s->dig2anl_dis) {
        analog_aux2pa_dig2anl_disable();
    }
}

AT(.text.bsp.aux)
void aux_var_init(void)
{
    memset(&bsp_aux_cb, 0, sizeof(bsp_aux_cb_t));
}

AT(.text.bsp.aux)
void micaux_analog_out_init(void)
{
    //printf("micaux_analog_out_init...\n");
    if (FUNC_AUX != func_cb.sta) {
        analog_aux2pa_start(CH_AUXL_PF0 | CH_AUXR_PF1);
        set_aux_analog_gain(xcfg_cb.aux_anl_gain, CH_AUXL_PF0 | CH_AUXR_PF1);
        analog_aux2pa_dig2anl_enable();
    }
    if (FUNC_SPEAKER != func_cb.sta) {
        aux_analog_channel_select(MICAUX_ANALOG_OUT_CH);                             //打通MIC到AUX通道
    }
    dac_fade_in();

}


