#include "include.h"
#include "func.h"
#include "func_bt.h"

#define MAX_DIG_VAL             32767
#define AEC_DIG_P0DB            (MAX_DIG_VAL * 1.000000)
#define AEC_DIG_P1DB            (MAX_DIG_VAL * 1.122018)
#define AEC_DIG_P2DB            (MAX_DIG_VAL * 1.258925)
#define AEC_DIG_P3DB            (MAX_DIG_VAL * 1.412538)
#define AEC_DIG_P4DB            (MAX_DIG_VAL * 1.584893)
#define AEC_DIG_P5DB            (MAX_DIG_VAL * 1.778279)
#define AEC_DIG_P6DB            (MAX_DIG_VAL * 1.995262)
#define AEC_DIG_P7DB            (MAX_DIG_VAL * 2.238721)
#define AEC_DIG_P8DB            (MAX_DIG_VAL * 2.511886)
#define AEC_DIG_P9DB            (MAX_DIG_VAL * 2.818383)
#define AEC_DIG_P10DB           (MAX_DIG_VAL * 3.162278)
#define AEC_DIG_P11DB           (MAX_DIG_VAL * 3.548134)
#define AEC_DIG_P12DB           (MAX_DIG_VAL * 3.981072)
#define AEC_DIG_P13DB           (MAX_DIG_VAL * 4.466836)
#define AEC_DIG_P14DB           (MAX_DIG_VAL * 5.011872)
#define AEC_DIG_P15DB           (MAX_DIG_VAL * 5.623413)

typedef struct {
    u8 aec_en;
    u8 wnr_en;
    u8 alc_en;
    u8 mic_eq_en;

    u8 aec_echo_level;
    u8 aec_far_offset;
    //降噪
    u16 noise_threshoid;

    u8 alc_fade_in_step;
    u8 alc_fade_out_step;
    u8 alc_fade_in_delay;
    u8 alc_fade_out_delay;
    s32 alc_far_voice_thr;
    u16 far_noise_threshoid;    //远端降噪阈值
    u8 far_nr_en;
    u8 ans_radio;
} bt_voice_cfg_t;

static bt_voice_cfg_t bt_voice_cfg AT(.sco_data);
int sfunc_bt_call_flag;
u32 cfg_post_mic_gain;
uint8_t cfg_bt_sco_dump = BT_SCO_DUMP_EN;

void bt_voice_init(bt_voice_cfg_t *p);
void bt_voice_exit(void);
void bt_sco_rec_exit(void);
void unlock_dcode(void);
void lock_code_bt_voice(void);
void unlock_code_bt_voice(void);
void magic_voice_set_level(u8 type, u8 level);
void lock_code_wsola(void);
void magic_voice_init(void *cfg);
void mav_kick_start(void);

AT(.rodata.mic_gain)
const int mic_gain_tbl[15] = {
    AEC_DIG_P1DB,
    AEC_DIG_P2DB,
    AEC_DIG_P3DB,
    AEC_DIG_P4DB,
    AEC_DIG_P5DB,
    AEC_DIG_P6DB,
    AEC_DIG_P7DB,
    AEC_DIG_P8DB,
    AEC_DIG_P9DB,
    AEC_DIG_P10DB,
    AEC_DIG_P11DB,
    AEC_DIG_P12DB,
    AEC_DIG_P13DB,
    AEC_DIG_P14DB,
    AEC_DIG_P15DB,
};

#if SCO_MAV_EN
#define MIC_CHCHE_LEN   256
#define SAMPLE_LEN      128

//魔音变量空间，蓝牙通话模式用
static s16 sco_buf_1[1536] AT(.sco_mav_buf);
static s16 sco_buf_2[128] AT(.sco_mav_cache);
static s16 sco_buf_3[128] AT(.sco_mav_cache);
static s16 sco_buf_4[512] AT(.sco_mav_cache);
static s16 sco_buf_5[64] AT(.sco_mav_cache);
static s16 sco_buf_6[64] AT(.sco_mav_cache);
static char sco_buf_7[384] AT(.sco_mav_buf);

typedef struct {
    u16 wptr;
    u16 rptr;
    u16 optr;
    u16 len;
    u16 olen;
    s16 buf[MIC_CHCHE_LEN];
}mic_mav_cache_t;

typedef struct {
    u16 res[2];
    s16 *buf_used;
    s16 *inbuf;
    s16 *outbuf;
    s16 *cache;
    s16 *mav_out;
    s16 *out1_buf;
    char *s8_fdata;
}mav_sco_cfg_t;

mav_sco_cfg_t mav_cfg_te = {
    .res[0]         = 0,
    .res[1]         = 0,
    .buf_used           = sco_buf_1,
    .inbuf              = sco_buf_2,
    .outbuf             = sco_buf_3,
    .cache              = sco_buf_4,
    .mav_out            = sco_buf_5,
    .out1_buf           = sco_buf_6,
    .s8_fdata           = sco_buf_7,
};
mic_mav_cache_t mic_mav_cache AT(.sco_mav_buf);

///切换魔音
const u8 magic_table[4][2] = {
    {0, 5},
    {0, 2},
    {1, 1},
    {1, 4},
};

uint8_t cfg_sco_mav_en = 0;

void sco_magic_voice_switch(void)
{
    //设置魔音级别
    u8 magic_type,magic_level;
    static int magic_sel_ce = 0;
    magic_sel_ce++;
    if (magic_sel_ce > 4) {
        magic_sel_ce = 0;
    }

    if (!magic_sel_ce) {
        magic_type = 0;
        magic_level = 0;
    } else {
        magic_type = magic_table[magic_sel_ce - 1][0];
        magic_level = magic_table[magic_sel_ce - 1][1];
    }
    magic_voice_set_level(magic_type, magic_level);

}
AT(.text.my_proc)
void sco_mav_init(void)
{
    memset(&mic_mav_cache, 0, sizeof(mic_mav_cache_t));
    lock_code_wsola();
    magic_voice_init(&mav_cfg_te);
    cfg_sco_mav_en = 1;
}
AT(.text.my_proc)
void sco_mav_exit(void)
{
    cfg_sco_mav_en = 0;
    memset(&mic_mav_cache, 0, sizeof(mic_mav_cache_t));
}

//AT(.com_text.sco_proc1)
//const char cnt_str[] = "out_cnt:%d\n";
AT(.bt_voice.sco_proc)
void sco_mav_process(u8 *ptr, u32 samples, int ch_mode)
{
    u16 len = samples << 1;
//    static u16 out_cnt = 0;
//    static u32 ticks = 0;
    mic_mav_cache_t *p = &mic_mav_cache;
    if((MIC_CHCHE_LEN - p->wptr) >= samples){
        memcpy(&p->buf[p->wptr], ptr, len);
        p->wptr += samples;
        if(p->wptr >= MIC_CHCHE_LEN){
            p->wptr = 0;
        }
    } else {
        u16 remain = MIC_CHCHE_LEN - p->wptr;
        memcpy(&p->buf[p->wptr], ptr, remain * 2);
        memcpy(p->buf, &ptr[remain], (samples - remain) * 2);
        p->wptr = samples - remain;
    }
    p->len += samples;
    if(p->len >= 128){
        memcpy(mav_cfg_te.inbuf, &p->buf[p->rptr], 256);
        memcpy(&p->buf[p->rptr], mav_cfg_te.outbuf, 256);

//        magic_voice_process();
        mav_kick_start();

        p->rptr += 128;

        if (p->rptr >= MIC_CHCHE_LEN) {
            p->rptr = 0;
        }
        p->len -= 128;
        p->olen += 128;
    }
    if (p->olen >= samples) {
        if ((MIC_CHCHE_LEN - p->optr) >= samples) {
            memcpy(ptr, &p->buf[p->optr], len);
            p->optr += samples;
            if (p->optr >= MIC_CHCHE_LEN) {
                p->optr = 0;
            }
        } else {
            u16 remain = MIC_CHCHE_LEN - p->optr;
            memcpy(ptr, &p->buf[p->optr], remain * 2);
            memcpy(&ptr[remain], p->buf, (samples - remain) * 2);
            p->optr = samples - remain;
        }
        p->olen -= samples;
    }
//    if (tick_check_expire(ticks,1000)) {
//        ticks = tick_get();
////        my_printf(cnt_str, out_cnt);
//        out_cnt = 0;
//    }

}
#endif

////库调用，设置MIC的增益（算法之后）
AT(.bt_voice.aec)
int sco_set_mic_gain_after_aec(void)
{
    return cfg_post_mic_gain;
}

#if BT_SCO_DBG_EN
void sdadc_set_digital_gain(u8 gain);
void sco_audio_set_param(u8 type, u16 value)
{
    //printf("set param[%d]:%d\n", type, value);
    if (type == AEC_PARAM_NOISE) {
        xcfg_cb.bt_noise_threshoid = value;
        bt_voice_cfg.noise_threshoid = value;
    } else if (type == AEC_PARAM_LEVEL) {
        xcfg_cb.bt_echo_level = value;
        bt_voice_cfg.aec_echo_level = value;
    } else if (type == AEC_PARAM_OFFSET) {
        xcfg_cb.bt_far_offset = value;
        bt_voice_cfg.aec_far_offset = value;
    } else if (type == AEC_PARAM_MIC_ANL_GAIN) {
        xcfg_cb.bt_anl_gain = value;
        set_mic_analog_gain(value, CH_MIC_PF2);
    } else if (type == AEC_PARAM_MIC_DIG_GAIN) {
        xcfg_cb.bt_dig_gain = value;
        sdadc_set_digital_gain(value & 0x3f);
    } else if (type == AEC_PARAM_MIC_POST_GAIN) {
        xcfg_cb.mic_post_gain = value & 0x0f;
    }
}
#endif // BT_SCO_DBG_EN

#if FUNC_BT_EN
void sco_audio_init(void)
{
    memset(&bt_voice_cfg, 0, sizeof(bt_voice_cfg_t));
    cfg_post_mic_gain = 0;
    if (xcfg_cb.mic_post_gain) {
        cfg_post_mic_gain = mic_gain_tbl[xcfg_cb.mic_post_gain - 1];
    }
    bt_voice_cfg.noise_threshoid = BT_NOISE_THRESHOID;
    bt_voice_cfg.ans_radio = xcfg_cb.bt_sco_ans_radio;
#if BT_AEC_EN
    if (xcfg_cb.bt_aec_en) {
        xcfg_cb.bt_alc_en = 0;
        bt_voice_cfg.aec_en = 1;
        bt_voice_cfg.aec_echo_level = BT_ECHO_LEVEL;
        bt_voice_cfg.aec_far_offset = BT_FAR_OFFSET;
    }
#endif

#if BT_ALC_EN
    if (xcfg_cb.bt_alc_en) {
        bt_voice_cfg.alc_en = 1;
        bt_voice_cfg.alc_fade_in_delay = BT_ALC_FADE_IN_DELAY;
        bt_voice_cfg.alc_fade_in_step = BT_ALC_FADE_IN_STEP;
        bt_voice_cfg.alc_fade_out_delay = BT_ALC_FADE_OUT_DELAY;
        bt_voice_cfg.alc_fade_out_step = BT_ALC_FADE_OUT_STEP;
        bt_voice_cfg.alc_far_voice_thr = BT_ALC_VOICE_THR;
        if (!bt_sco_is_msbc()) {
            bt_voice_cfg.alc_fade_in_delay >>= 1;
        }
    }
#endif

#if SCO_MAV_EN
    sco_mav_init();
    set_sys_clk(SYS_120M);
#endif

#if BT_HFP_REC_EN
    func_bt_sco_rec_init();
#endif
    if (mic_set_eq_by_res(&RES_BUF_EQ_BT_MIC_EQ, &RES_LEN_EQ_BT_MIC_EQ)) {
        bt_voice_cfg.mic_eq_en = 1;
    }
#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
    sys_cb.hfp_karaok_en = BT_HFP_CALL_KARAOK_EN;       //通话是否支持KARAOK
    plugin_hfp_karaok_configure();
    if (sys_cb.hfp_karaok_en) {
        bsp_karaok_init(AUDIO_PATH_BTMIC, FUNC_BT);
        kara_sco_start();
#if BT_TSCO_EN || BT_HFP_CALL_KARAOK_EN
        unlock_dcode();
        lock_code_bt_voice();
#endif
    } else
#endif
    {
        bt_voice_init(&bt_voice_cfg);
        dac_set_anl_offset(1);
        audio_path_init(AUDIO_PATH_BTMIC);
        audio_path_start(AUDIO_PATH_BTMIC);
#if EXLIB_BT_MONO_XDRC_EN
        sys_cb.xdrc_sysvol_en = is_xdrc_sysvol_en();
        xdrc_sysvol_en_set(0);
#endif
        bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
        dac_fade_in();
    }

#if BT_HID_SIMPLE_KEYBOARD
     bt_call_private_switch();     //通话切到音箱上面来
#endif
}

u8 bt_hfp_call_karaok_en(void)
{
    return BT_HFP_CALL_KARAOK_EN;
}

void sco_audio_exit(void)
{
#if BT_HFP_REC_EN
    sfunc_record_stop();
    bt_sco_rec_exit();
#endif
#if SYS_KARAOK_EN
    if (sys_cb.hfp_karaok_en) {
        kara_sco_stop();
        bsp_karaok_exit(AUDIO_PATH_BTMIC);
#if BT_TSCO_EN || BT_HFP_CALL_KARAOK_EN
        unlock_code_bt_voice();
#endif
        sys_cb.hfp_karaok_en = 0;
    } else
#endif
    {
        dac_fade_out();
        dac_aubuf_clr();
        audio_path_exit(AUDIO_PATH_BTMIC);
        bt_voice_exit();
        dac_set_anl_offset(0);
        bsp_change_volume(sys_cb.vol);
    }

#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, FUNC_BT);
#endif
#if SCO_MAV_EN
    sco_mav_exit();
#endif
#if BT_REC_EN && BT_HFP_REC_EN
    bt_music_rec_init();
#endif
}

static void sfunc_bt_call_process(void)
{
    func_process();
    func_bt_sub_process();
    if(sco_is_connected() && (sfunc_bt_call_flag == 1)){
        sfunc_bt_call_flag = 2;
        bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
        dac_fade_in();
    }
}

#if BT_ALC_DUMP_EN
//file_0(双声道): L(远端数据(提取远端能量进行仲裁) + R 近端最终处理完后的数据
//file_1(双声道): L(MIC降噪前的数据) + R(MIC降噪后的数据)
//file_2(单声道): ALC 仲裁后的MIC近端数据
u8 sco_dump_dma_buf[14+256 + 14+256 + 14+128];
AT(.bt_voice.alc)
void alc_dump(void *buf, int idx)
{
     u16 *pcml,*pcmr;
     u32 *pcm_dual;
    if(0 == idx) {          //0: 远端数据(提取远端能量进行仲裁)
         memcpy(&sco_dump_dma_buf[14+128],buf,128);
    } else if (1 == idx) {  //1: 近端最终处理完后的数据
        //0和1合并成双声道一起导出，方便查看远端数据能量 对近端MIC进入淡入淡出的效果
        pcml = (u16*)&sco_dump_dma_buf[14+128];
        pcmr = (u16*)buf;
        pcm_dual = (u32*)&sco_dump_dma_buf[14];
        for(int i = 0; i < 64; i++) {
            pcm_dual[i] = ((u32)pcmr[i] << 16) | pcml[i];
        }
        dump_put2ram(&sco_dump_dma_buf[0],pcm_dual,256,0);             //file_0(双声道): L(远端数据(提取远端能量进行仲裁) + R 近端最终处理完后的数据
        dump_dma_wait();
        dump_dma_kick(sco_dump_dma_buf,14+256);  //0/1一起KICK导出
    } else if (2 == idx) {     //2: MIC原始数据
        memcpy(&sco_dump_dma_buf[14+256 + 14+128],buf,128);
    } else if (3 == idx) {     //3: MIC降噪后的数据
        //2和3合并成双声道一起导出,方便查看降噪效果
        pcml = (u16*)&sco_dump_dma_buf[14+256 + 14+128];
        pcmr = (u16*)buf;
        pcm_dual = (u32*)&sco_dump_dma_buf[14+256 + 14];
        for(int i = 0; i < 64; i++) {
            pcm_dual[i] = ((u32)pcmr[i] << 16) | pcml[i];
        }
        dump_put2ram(&sco_dump_dma_buf[14+256],pcm_dual,256,1);        //file_1(双声道): L(MIC降噪前的数据) + R(MIC降噪后的数据)
    } else if (4 == idx) {  //4: MIC ALC仲裁后的数据 (后面还会增加MIC后置增益,在idx=1中导出)
        dump_put2ram(&sco_dump_dma_buf[(14+256)*2], buf, 128, 2);      //file_2(单声道): ALC 仲裁后的MIC近端数据
        dump_dma_wait();
        dump_dma_kick(&sco_dump_dma_buf[14+256], (14+256 + 14+128));   //2/3/4 一起KICK导出
    }
};
#endif

#if BT_AEC_DUMP_EN
//file_0(双声道): L(远端数据) + R (近端未处理的数据)
//file_1(双声道): L(MIC的远端数据) + R(MIC降噪消音后的近端数据)
u8 sco_aec_dump_dma_buf[14+256 + 14+256];
AT(.bt_voice.aec)
void aec_dump(void *buf, int idx)
{
    u16 *pcml,*pcmr;
    u32 *pcm_dual;
    if(0 == idx) {          //0: 远端数据(提取远端能量进行仲裁)
         memcpy(&sco_aec_dump_dma_buf[14+128],buf,128);
    } else if (1 == idx) {  //1: 近端数据
        //0和1合并成双声道一起导出，方便查看远端数据能量 对近端MIC进入淡入淡出的效果
        pcml = (u16*)&sco_aec_dump_dma_buf[14+128];
        pcmr = (u16*)buf;
        pcm_dual = (u32*)&sco_aec_dump_dma_buf[14];
        for(int i = 0; i < 64; i++) {
            pcm_dual[i] = ((u32)pcmr[i] << 16) | pcml[i];
        }
        dump_put2ram(&sco_aec_dump_dma_buf[0],pcm_dual,256,0);             //file_0(双声道): L(远端数据) + R (近端未处理的数据)
        dump_dma_wait();
        dump_dma_kick(sco_aec_dump_dma_buf,14+256);  //0/1一起KICK导出
    } else if (2 == idx) {     //2: 远端数据(与0相同，与3一起导出方便观察)
        memcpy(&sco_aec_dump_dma_buf[14+256 + 14+128],buf,128);
    } else if (3 == idx) {     //3: AEC近端降噪消音后数据
        //2和3合并成双声道一起导出,方便查看降噪效果
        pcml = (u16*)&sco_aec_dump_dma_buf[14+256 + 14+128];   //远端
        pcmr = (u16*)buf;                                  //近端
        pcm_dual = (u32*)&sco_aec_dump_dma_buf[14+256 + 14];
        for(int i = 0; i < 64; i++) {
            pcm_dual[i] = ((u32)pcmr[i] << 16) | pcml[i];
        }
        dump_put2ram(&sco_aec_dump_dma_buf[14+256],pcm_dual,256,1);        //file_1(双声道): L(MIC的远端数据) + R(MIC降噪消音后的近端数据)
        dump_dma_wait();
        dump_dma_kick(&sco_aec_dump_dma_buf[14+256], 14+256);   //2/3 一起KICK导出
    }
}
#endif // BT_AEC_DUMP_EN

static void sfunc_bt_call_enter(void)
{
#if BT_AEC_DUMP_EN || BT_ALC_DUMP_EN
//    GPIOAFEN &= ~0x70;  //GPIOA 4/5/6 FOR TEST
//    GPIOADE |= 0x70;
//    GPIOADIR &= ~0x70;
//    GPIOA &= ~0x70;
    dump_buf_huart_init();
#endif
    if(get_cur_sysclk() < SYS_120M) {  //通话抬高主频
        set_sys_clk(SYS_120M);
    }
    sfunc_bt_call_flag = 1;
    //修正ALC通话，MIC选外部电容电阻时，MIC声音串到DAC问题 (AUANGCON0的BIT6置1)
    //或者在配置界面中，VDDIO/VDDDAC电压分别改成(3.2/3.0)也能解决该问题, 测试另外几组也可(3.1/3.0)或(3.1/2.9)或(2.9/2.9)或(2.9/2.7)
//    if(0 == xcfg_cb.mic_bias_method) {//配置界面中mic偏置选外部电容电阻
//        AUANGCON0 |= BIT(6);
//    }
}

static void sfunc_bt_call_exit(void)
{
#if EXLIB_BT_MONO_XDRC_EN
    xdrc_sysvol_en_set(sys_cb.xdrc_sysvol_en);
#endif

    bsp_change_volume(sys_cb.vol);
    sfunc_bt_call_flag = 0;
#if SYS_KARAOK_EN && BT_HFP_CALL_PRIVATE_FORCE_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, FUNC_BT);
#endif // SYS_KARAOK_EN
    AUANGCON0 &= ~BIT(6);
    set_sys_clk(SYS_CLK_SEL);
}

AT(.text.func.bt)
void sfunc_bt_call(void)
{
    printf("%s\n", __func__);

    sfunc_bt_call_enter();
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

    while ((f_bt.disp_status >= BT_STA_OUTGOING) && (func_cb.sta == FUNC_BT)) {
        sfunc_bt_call_process();
        sfunc_bt_call_message(msg_dequeue());
        func_bt_display();
    }

#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
    sfunc_bt_call_exit();
}
#else

void sco_audio_init(void){}
void sco_audio_exit(void){}

#endif //FUNC_BT_EN
