#include "include.h"

void bt_aec_process(u8 *ptr, u32 samples, int ch_mode);
void bt_sco_process(u8 *ptr, u32 samples, int ch_mode);
void bt_alc_process(u8 *ptr, u32 samples, int ch_mode);
void aux_sdadc_process(u8 *ptr, u32 samples, int ch_mode);
void speaker_sdadc_process(u8 *ptr, u32 samples, int ch_mode);
void usbmic_sdadc_process(u8 *ptr, u32 samples, int ch_mode);
void karaok_sdadc_process(u8 *ptr, u32 samples, int ch_mode);
void mic_trim_sdadc_callback(u8 *ptr, u32 samples, int ch_mode);
void tom_cat_sdadc_process(u8 *ptr, u32 samples, int ch_mode);

#if FUNC_AUX_EN
    #define aux_sdadc_callback      aux_sdadc_process
#else
    #define aux_sdadc_callback      sdadc_dummy
#endif // FUNC_AUX_EN

#if FUNC_SPEAKER_EN
    #define speaker_sdadc_callback  speaker_sdadc_process
#else
    #define speaker_sdadc_callback  sdadc_dummy
#endif // FUNC_SPEAKER_EN

#if FUNC_TOM_CAT_EN
    #define tom_cat_sdadc_callback  tom_cat_sdadc_process
#else
    #define tom_cat_sdadc_callback  sdadc_dummy
#endif // FUNC_TOM_CAT_EN

#if UDE_MIC_EN
    #define usbmic_sdadc_callback   usbmic_sdadc_process
#else
    #define usbmic_sdadc_callback   sdadc_dummy
#endif // UDE_MIC_EN

#if BT_ALC_EN
    #define bt_sdadc_callback    bt_alc_process
#else
    #define bt_sdadc_callback    bt_sco_process
#endif

#if SYS_KARAOK_EN
    #define karaok_sdadc_callback   karaok_sdadc_process
#else
    #define karaok_sdadc_callback   sdadc_dummy
#endif


//AUX analog gain -42DB~+3DB
//MIC analog gain: 0~13(共14级), step 3DB (3db ~ +42db)
//adadc digital gain: 0~63, step 0.5 DB, 保存在gain的低6bit
const sdadc_cfg_t rec_cfg_tbl[] = {
/*   通道,              采样率,     增益,        通路控制,     保留,   样点数,   回调函数*/
    {AUX_CHANNEL_CFG,   SPR_44100,  (2 << 6),    ADC2DAC_EN,     0,      256,    aux_sdadc_callback},            /* AUX     */
    {MIC_CHANNEL_CFG,   SPR_16000,  (12 << 6),   ADC2DAC_EN,     0,      240,    speaker_sdadc_callback},        /* SPEAKER */
    {MIC_CHANNEL_CFG,   SPR_8000,   (12 << 6),   ADC2DAC_EN,     0,      128,    bt_sdadc_callback},             /* BTMIC   */
    {MIC_CHANNEL_CFG,   SPR_48000,  (12 << 6),   ADC2DAC_EN,     0,      128,    usbmic_sdadc_callback},         /* USBMIC  */
    {MIC_CHANNEL_CFG,   SPR_44100,  (12 << 6),   ADC2SRC_EN,     0,      256,    karaok_sdadc_callback},         /* KARAOK  */
    {MIC_CHANNEL_CFG,   SPR_44100,  (12 << 6),   ADC2DAC_EN,     0,      128,    mic_trim_sdadc_callback},
    {MIC_CHANNEL_CFG,   SPR_16000,  (12 << 6),   ADC2DAC_EN,     0,      256,    tom_cat_sdadc_callback},
};

const u8 btmic_ch_tbl[] = {0, CH_MIC_PF2, CH_MIC_PF3};
const u16 audio_spr_table[] = {48000,44100,38000,32000,24000,22050,16000,12000,11025,8000};

void audio_path_init(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel =  (xcfg_cb.auxr_sel << 4) | xcfg_cb.auxl_sel;
        cfg.gain =     ((u16)xcfg_cb.aux_anl_gain << 6) | xcfg_cb.aux_dig_gain;
    }
#endif // FUNC_AUX_EN

    if (path_idx == AUDIO_PATH_BTMIC || path_idx == AUDIO_PATH_KARAOK) {
        if (path_idx == AUDIO_PATH_BTMIC) {
            if (sys_cb.hfp_karaok_en) {
                memcpy(&cfg, &rec_cfg_tbl[AUDIO_PATH_KARAOK], sizeof(sdadc_cfg_t));
                cfg.sample_rate = SPR_48000;
            } else {
                #if BT_AEC_EN
                if (xcfg_cb.bt_aec_en) {
                    printf("-------->bt_aec_process,msbc = %d\n",BT_HFP_MSBC_EN);
                    cfg.callback = bt_aec_process;
                } else
                #endif
                #if BT_ALC_EN
                if (xcfg_cb.bt_alc_en) {
                    printf("-------->bt_alc_process,msbc = %d\n",BT_HFP_MSBC_EN);
                    cfg.callback = bt_alc_process;
                } else
                #endif
                {
                    cfg.callback = bt_sco_process;
                }
            }
        }

        cfg.channel = btmic_ch_tbl[xcfg_cb.bt_ch_mic];
        cfg.gain = ((u16)BT_ANL_GAIN << 6) | BT_DIG_GAIN;
    }
#if UDE_MIC_KARAOK_EN
    if (AUDIO_PATH_KARAOK == path_idx) {
        cfg.sample_rate = SPR_48000;
    }
#endif
    printf("cfg sadc sample_rate: %d\n",audio_spr_table[cfg.sample_rate]);
    sdadc_init(&cfg);
}

void audio_path_start(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = (xcfg_cb.auxr_sel << 4) | xcfg_cb.auxl_sel;
    }
#endif // FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_BTMIC || path_idx == AUDIO_PATH_KARAOK || path_idx == AUDIO_PATH_MICTRIM) {
        cfg.channel = btmic_ch_tbl[xcfg_cb.bt_ch_mic];
    }

    sdadc_start(cfg.channel);
}

void audio_path_exit(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = (xcfg_cb.auxr_sel << 4) | xcfg_cb.auxl_sel;
    }
#endif // FUNC_AUX_EN

    if (path_idx == AUDIO_PATH_BTMIC || path_idx == AUDIO_PATH_KARAOK || path_idx == AUDIO_PATH_MICTRIM) {
        cfg.channel = btmic_ch_tbl[xcfg_cb.bt_ch_mic];
    }

    sdadc_exit(cfg.channel);

    adpll_spr_set(DAC_OUT_SPR);
}

u8 get_aux_channel_cfg(void)
{
#if FUNC_AUX_EN
    return ((xcfg_cb.auxr_sel << 4) | xcfg_cb.auxl_sel);    //工具配置AUX通路
#else
    return 0;
#endif // FUNC_AUX_EN
}

u8 get_mic_dig_gain(void)
{
    return BT_DIG_GAIN;
}

void bsp_mic_bias_path_start(void)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[AUDIO_PATH_MICTRIM], sizeof(sdadc_cfg_t));
    cfg.channel = btmic_ch_tbl[xcfg_cb.bt_ch_mic];
    sdadc_init(&cfg);
    audio_path_start(AUDIO_PATH_MICTRIM);
}

void bsp_mic_bias_path_exit(void)
{
    audio_path_exit(AUDIO_PATH_MICTRIM);
}

u8 get_mic_bias_channel_cfg(void)
{
    return btmic_ch_tbl[xcfg_cb.bt_ch_mic];
}



