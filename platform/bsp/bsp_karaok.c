#include "include.h"

#if KARAOK_MIC_DRC
bool sdadc_drc_init(u8 *drc_addr, int drc_len);
uint8_t drc_mic_en = KARAOK_MIC_DRC;
uint8_t karaok_mic_gain_range = 20;                                  ///过完drc的后置数字增益，0~31,对应0~31db
#endif // KARAOK_MIC_DRC

#if AUDIO_STRETCH_EN && !SYS_MAGIC_VOICE_EN
static s16 buf_1[2048] AT(.speed_buf1);
static s16 buf_2[128] AT(.speed_buf1);
static s16 buf_3[128] AT(.speed_buf1);
static s16 buf_4[512] AT(.speed_buf1);
static s16 buf_5[64] AT(.speed_buf1);
static s16 buf_6[64] AT(.speed_buf1);
static char buf_7[512] AT(.speed_buf1); //AT(.mav_cache1);

//static s16 buf_11[1536] AT(.mav_cache1);

mav_cfg_t mav_cfg = {
    .res[0]         = 1,
    .res[1]         = 0,
    .buf_1          = buf_1,
    .buf_2          = buf_2,
    .buf_3          = buf_3,
    .buf_4          = buf_4,
    .buf_5          = buf_5,
    .buf_6          = buf_6,
    .buf_7          = buf_7,
};
#endif


#if SYS_KARAOK_EN

karaok_vol_t karaok_vol;

void lock_code_wsola(void);
void unlock_code_wsola(void);

static s16 echo_cache_0[0x1400] AT(.echo_buf);
/**/
#if SYS_MAGIC_VOICE_EN && !AUDIO_STRETCH_EN
static s16 buf_1[1536] AT(.mav_cache);
static s16 buf_2[128] AT(.mav_buf);
static s16 buf_3[128] AT(.mav_buf);
static s16 buf_4[512] AT(.magic_buf);
static s16 buf_5[64] AT(.magic_buf);
static s16 buf_6[64] AT(.magic_buf);
static char buf_7[384] AT(.magic_buf);

static s16 buf_11[1536] AT(.mav_cache_flash);

mav_cfg_t mav_cfg = {
    .res[0]         = 0,
    .res[1]         = 0,
    .buf_1          = buf_1,
    .buf_2          = buf_2,
    .buf_3          = buf_3,
    .buf_4          = buf_4,
    .buf_5          = buf_5,
    .buf_6          = buf_6,
    .buf_7          = buf_7,
};
#endif



AT(.rodata.dac)
const u16 karaok_dvol_table_12[12 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N22DB,  DIG_N18DB,  DIG_N14DB,  DIG_N12DB,  DIG_N10DB,
    DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

AT(.rodata.dac)
const u16 karaok_dvol_table_16[16 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N24DB,  DIG_N22DB,  DIG_N20DB,  DIG_N18DB, DIG_N16DB,
    DIG_N14DB,  DIG_N12DB,  DIG_N10DB,  DIG_N8DB,   DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

///AUXL3(PF0), AUXR3(PF1)模拟增益表。karaok时最好使用PF0, PF1的AUX通路，AUX模拟增益档位比较多
AT(.rodata.dac)
const u8 karaok_aux3_table_12[12 + 1] = {
    AUX3_MUTE,   AUX3_N39DB,  AUX3_N30DB,  AUX3_N24DB,  AUX3_N18DB,  AUX3_N12DB,  AUX3_N9DB,  AUX3_N7DB,  AUX3_N5DB,
    AUX3_N3DB,   AUX3_N2DB,   AUX3_N1DB,   AUX3_N0DB,
};

AT(.rodata.dac)
const u8 karaok_aux3_table_16[16 + 1] = {
    AUX3_MUTE,   AUX3_N39DB,  AUX3_N30DB,  AUX3_N24DB,  AUX3_N18DB,  AUX3_N15DB,  AUX3_N12DB,  AUX3_N10DB, AUX3_N8DB,
    AUX3_N7DB,   AUX3_N6DB,   AUX3_N5DB,   AUX3_N4DB,   AUX3_N3DB,   AUX3_N2DB,   AUX3_N1DB,   AUX3_N0DB,
};

///其它AUX通路模拟增益表
AT(.rodata.dac)
const u8 karaok_aux_table_12[12 + 1] = {
    AUXG_MUTE,   AUXG_N39DB,  AUXG_N33DB,  AUXG_N27DB,  AUXG_N27DB,  AUXG_N21DB,  AUXG_N21DB,  AUXG_N15DB, AUXG_N15DB,
    AUXG_N9DB,   AUXG_N6DB,   AUXG_N3DB,   AUXG_N0DB,
};

AT(.rodata.dac)
const u8 karaok_aux_table_16[16 + 1] = {
    AUXG_MUTE,   AUXG_N39DB,  AUXG_N33DB,  AUXG_N27DB,  AUXG_N27DB,  AUXG_N21DB,  AUXG_N21DB,  AUXG_N15DB, AUXG_N15DB,
    AUXG_N9DB,   AUXG_N9DB,   AUXG_N6DB,   AUXG_N6DB,   AUXG_N3DB,   AUXG_N3DB,   AUXG_N0DB,   AUXG_N0DB,
};


//低通滤波器参数， 3KHz截至频率
const s32 echo_filter_coef[2][8] = {
    {   //44.1KHz 3K
        //分子
        FIX_BIT(0.006644395788557),
        FIX_BIT(0.019933187365671),
        FIX_BIT(0.019933187365671),
        FIX_BIT(0.006644395788557),
        //分母
        FIX_BIT(1.000000000000000),
        FIX_BIT(-2.150944711976045),
        FIX_BIT(1.626545208636282),
        FIX_BIT(-0.422445330351779),
    }, {//48KHz
        //分子
        FIX_BIT(0.005300409794526),
        FIX_BIT(0.015901229383577),
        FIX_BIT(0.015901229383577),
        FIX_BIT(0.005300409794526),

        //分母
        FIX_BIT(1.000000000000000),
        FIX_BIT(-2.219168618311665),
        FIX_BIT(1.715117830033402),
        FIX_BIT(-0.453545933365530),
    }
};

////低通滤波器参数， 4KHz截至频率
//const s32 echo_filter_coef[2][8] = {
//    {   //44.1KHz 4K
//        //分子
//        FIX_BIT(0.014099708769044),
//        FIX_BIT(0.042299126307133),
//        FIX_BIT(0.042299126307133),
//        FIX_BIT(0.014099708769044),
//        //分母
//        FIX_BIT(1.000000000000000),
//        FIX_BIT(-1.873027248422390),
//        FIX_BIT(1.300326954651049),
//        FIX_BIT(-0.314502036076304),
//    }, {//48KHz
//        //分子
//        FIX_BIT(0.011324865405187),
//        FIX_BIT(0.033974596215561),
//        FIX_BIT(0.033974596215561),
//        FIX_BIT(0.011324865405187),
//        //分母
//        FIX_BIT(1.000000000000000),
//        FIX_BIT(-1.962990915244728),
//        FIX_BIT(1.400000000000000),
//        FIX_BIT(-0.346410161513776),
//    }
//};


#if KARAOK_RM_VOICE
static const s32 rm_voice_filter_coef[8] = { //300Hz
	//分子
	FIX_BIT(0.0000093579292297363),
	FIX_BIT(0.0000280700623989105),
	FIX_BIT(0.0000280700623989105),
	FIX_BIT(0.0000093579292297363),
	//分母
	FIX_BIT( 1.000000000000000),
	FIX_BIT(-2.914520982652903),
	FIX_BIT( 2.832656301558018),
	FIX_BIT(-0.918060466647148),
};
#endif


//混响间隔：(echo_delay[0, 87] * 128 / 44100)ms
const u16 echo_delay_12[12 + 1] = {
    0,
    30,       35,        38,         40,
    45,       48,        55,         60,
    65,       70,        75,         79,
};

//混响间隔：(echo_delay[0, 87] * 128 / 44100)ms
const u16 echo_delay_16[16 + 1] = {
     0,
     3,          5,        10,        15,
    20,         25,        40,        48,
    55,         60,        65,        70,
    72,         74,        76,        79,
};


kara_cfg_t kara_cfg = {
    .lp_coef        = &echo_filter_coef,
#if SYS_MAGIC_VOICE_EN
    .mav            = &mav_cfg,
#endif
    .res[0]         = 0,
    .res[1]         = 0,
    .echo_en        = SYS_ECHO_EN,
    .mic_dnr        =  MIC_DNR_EN,
    .rec_en         = KARAOK_REC_EN,
    .magic_voice    = SYS_MAGIC_VOICE_EN,
//    .rec_no_echo_en = KARAOK_REC_NO_ECHO_EN,
    .howling_en     = SYS_HOWLING_EN,
    .pitch_shift    = SYS_PITCH_SHIFT,
};

#if REC_WHITOUT_KARAOK
u8 lcok_ram_karaok_ext(void){ return 2;} //多lock 2条，防止蓝牙模式下mp3_enc lock fail
#endif

AT(.text.bsp.karaok)
void bsp_echo_set_level(void)
{
    echo_set_level(karaok_vol.echo_level_gain[sys_cb.echo_level]);
}

AT(.text.bsp.karaok)
void bsp_echo_set_delay(void)
{
    echo_set_delay(karaok_vol.echo_delay[sys_cb.echo_delay]);
}

AT(.text.bsp.karaok)
void bsp_karaok_set_mic_volume(void)
{
    karaok_set_mic_volume(karaok_vol.micvol_table[sys_cb.mic_vol]);
}

//AT(.text.bsp.karaok1)
//const u8 aux2mic_gain_tbl[16 + 1] = {
//    0,  1,  3,  5,  8,  10, 12, 14, 16,
//    18, 19, 20, 21, 22, 23, 24, 25,
//};

AT(.text.bsp.karaok)
void bsp_karaok_set_music_volume(void)
{
    if (karaok_vol.func == FUNC_AUX) {
        //调节AUX模拟增益
        set_aux_analog_vol(karaok_vol.auxvol_table[sys_cb.music_vol], get_aux_channel_cfg());
    } else {
        karaok_set_music_volume(karaok_vol.dvol_table[sys_cb.music_vol]);
    }
}

void bsp_karaok_music_vol_adjust(u8 up)
{
    if (up) {
        if (sys_cb.music_vol <  USER_KEY_KNOB_LEVEL) {
            sys_cb.music_vol++;
        } else {
            sys_cb.music_vol = USER_KEY_KNOB_LEVEL;
        }
    } else {
        if (sys_cb.music_vol > 0) {
            sys_cb.music_vol--;
        }
    }
    printf("karaok music_vol = %d\n", sys_cb.music_vol);
    bsp_karaok_set_music_volume();
}

void bsp_karaok_mic_vol_adjust(u8 up)
{
    if (up) {
        if (sys_cb.mic_vol <  USER_KEY_KNOB_LEVEL) {
            sys_cb.mic_vol++;
        } else {
            sys_cb.mic_vol = USER_KEY_KNOB_LEVEL;
        }
    } else {
        if (sys_cb.mic_vol > 0) {
            sys_cb.mic_vol--;
        }
    }
    printf("karaok mic_vol = %d\n", sys_cb.mic_vol);
    bsp_karaok_set_mic_volume();
}

AT(.text.bsp.karaok)
void bsp_karaok_echo_reset_buf(u8 func)
{
    if (func == FUNC_NULL) {
        return;
    }
    if (func == FUNC_BT || func == FUNC_BTHID) {
#if KARAOK_REC_EN
        rec_cb_t *rec = &rec_cb;
        if (func == FUNC_BT && rec->sta == REC_STARTING) {
            kara_cfg.res[0] = (kara_cfg.res[0] & ~0x01) | BIT(0);
        } else {
            kara_cfg.res[0] = (kara_cfg.res[0] & ~0x01) | 0;
        }
#endif
        if (kara_cfg.echo_cache == NULL) {
            return;
        }
        kara_cfg.echo_cache = NULL;
    } else {
        if (kara_cfg.echo_cache == echo_cache_0) {
            return;
        }
        kara_cfg.echo_cache = echo_cache_0;
    }
#if SYS_MAGIC_VOICE_EN
    unlock_code_wsola();
#endif
#if SYS_PITCH_SHIFT
    unlock_code_pitch();
#endif
#if KARAOK_DENOISE_EN
    void unlock_code_karaok_dednr(void);
    unlock_code_karaok_dednr();
#endif
    echo_reset_buf(&kara_cfg);
#if SYS_MAGIC_VOICE_EN
    if (kara_cfg.magic_voice) lock_code_wsola();
#endif
#if SYS_PITCH_SHIFT
    if(kara_cfg.pitch_shift) lock_code_pitch();
#endif
#if KARAOK_DENOISE_EN
    void lock_code_karaok_dednr(void);
    lock_code_karaok_dednr();
#endif
    echo_set_delay(karaok_vol.echo_delay[sys_cb.echo_delay]);
    echo_set_level(karaok_vol.echo_level_gain[sys_cb.echo_level]);
}
void set_pitch_shift(u16 en)
{
    kara_cfg.pitch_shift = en;
}
void set_magic_voice(u16 en)
{
    kara_cfg.magic_voice = en;
}
AT(.text.bsp.karaok)
void bsp_karaok_init(u8 path, u8 func)
{
    u8 aux_channel;
    printf("%s %d\n", __func__, func);
    if (sys_cb.karaok_init) {
        return;
    }
    if (func == FUNC_NULL) {
        return;
    }

#if SYS_PITCH_SHIFT
    if (kara_cfg.pitch_shift) {
        kara_cfg.magic_voice = 0;
    }
    if (kara_cfg.magic_voice) {
        kara_cfg.pitch_shift = 0;
    }
#endif
    if ((func == FUNC_BT) || (func == FUNC_BTHID)) {
#if KARAOK_REC_EN
        rec_cb_t *rec = &rec_cb;
        if (func == FUNC_BT && rec->sta == REC_STARTING) {
            kara_cfg.res[0] = (kara_cfg.res[0] & ~0x01) | BIT(0);
        } else {
            kara_cfg.res[0] = (kara_cfg.res[0] & ~0x01) | 0;
        }
#endif
        #if SYS_MAGIC_VOICE_EN
        if (sys_cb.hfp_karaok_en) {
            mav_cfg.buf_1 = buf_11;
        } else {
            mav_cfg.buf_1 = buf_1;
        }
        #endif
        kara_cfg.echo_cache = NULL;
    } else {
        kara_cfg.echo_cache = echo_cache_0;
    }

    sys_cb.sysclk_bak = get_cur_sysclk();
    if (sys_cb.sysclk_bak < SYS_120M) {
        set_sys_clk(SYS_120M);
    }

    aux_channel = get_aux_channel_cfg();
    sys_cb.echo_delay = SYS_ECHO_DELAY;
    karaok_vol.func = func;
#if (USER_KEY_KNOB_LEVEL == 12)
    karaok_vol.echo_delay = echo_delay_12;
    karaok_vol.echo_level_gain = echo_level_gain_12;
    karaok_vol.dvol_table = karaok_dvol_table_12;
    karaok_vol.micvol_table = karaok_dvol_table_12;
    if (aux_channel == (CH_AUXL_PF0 | CH_AUXR_PF1) || aux_channel == CH_AUXR_PF1 || aux_channel == CH_AUXL_PF0) {
        karaok_vol.auxvol_table = karaok_aux3_table_12;
    } else {
        karaok_vol.auxvol_table = karaok_aux_table_12;
    }
#elif (USER_KEY_KNOB_LEVEL == 16)
    karaok_vol.echo_delay = echo_delay_16;
    karaok_vol.echo_level_gain = echo_level_gain_16;
    karaok_vol.dvol_table = karaok_dvol_table_16;
    karaok_vol.micvol_table = karaok_dvol_table_16;
    if (aux_channel == (CH_AUXL_PF0 | CH_AUXR_PF1) || aux_channel == CH_AUXR_PF1 || aux_channel == CH_AUXL_PF0) {
        karaok_vol.auxvol_table = karaok_aux3_table_16;
    } else {
        karaok_vol.auxvol_table = karaok_aux_table_16;
    }
#endif

#if MIC_EQ_EN
    mic_set_eq_by_res(&RES_BUF_EQ_MIC_NORMAL_EQ, &RES_LEN_EQ_MIC_NORMAL_EQ);
#endif

#if KARAOK_DENOISE_EN
    void unlock_code_karaok_dednr(void);
    unlock_code_karaok_dednr();
#endif //KARAOK_DENOISE_EN

    plugin_karaok_init();

    audio_path_init(path);
    karaok_init(&kara_cfg);
#if SYS_MAGIC_VOICE_EN
    if(kara_cfg.magic_voice) lock_code_wsola();
#endif
#if SYS_PITCH_SHIFT
    if(kara_cfg.pitch_shift) lock_code_pitch();
#endif
#if KARAOK_DENOISE_EN
    micdnr_level_init(KARAOK_DENOISE_LEVEL);
    void lock_code_karaok_dednr(void);
    lock_code_karaok_dednr();
#endif

#if KARAOK_MIC_DRC
    sdadc_drc_init((u8 *)RES_BUF_EQ_SDADC_DRC, RES_LEN_EQ_SDADC_DRC);
#endif // KARAOK_MIC_DRC

#if MIC_DNR_EN
    mic_dnr_init(2, 1000, 100, 800); //10ms检测一次 //连续超过2次大于能量 1000就认为有声，连续超过100次能量低于600 就认为无声. MIC实时能量值可以通过函数get_karaok_mic_maxpow(0)获取。
#endif
    //karaok_enable_mic_first(65535/10);  //(数字音量最大值为0xFFFF_65535)
    //mic_first_init(2, 700, 30, 1200);   //连续超过2次大于700 就开启mic_first，连续超过 100 次低于 1200 就关闭mic_first
    //karaok_mic_first_music_vol_fade_enable(100, 1000);   //淡入步进100, 淡出步进1000.   (数字音量最大值为0xFFFF_65535)

    echo_set_delay(karaok_vol.echo_delay[sys_cb.echo_delay]);
    echo_set_level(karaok_vol.echo_level_gain[sys_cb.echo_level]);
    karaok_set_mic_volume(karaok_vol.micvol_table[sys_cb.mic_vol]);
    bsp_karaok_set_music_volume();

#if MIC_DETECT_EN
    if (!dev_is_online(DEV_MIC)) {
        karaok_mic_mute();
    }
#endif
    audio_path_start(path);

#if SYS_MAGIC_VOICE_EN
    karaok_set_mav(sys_cb.magic_type, sys_cb.magic_level);
#endif

#if KARAOK_RM_VOICE
    karaok_set_voice_rm_type(1, rm_voice_filter_coef);   //type: 1 有过滤波器,消耗比较多RAM和算力  //0 简单左-右消人声
#endif

    sys_cb.karaok_init = 1;
#if SYS_PITCH_SHIFT
    if(kara_cfg.pitch_shift) pitch_shift2_init(44100, -30432);
#endif

#if SYS_PITCH_SHIFT
//    AUANGCON0 |= BIT(29);
//    AUANGCON0 |= BIT(28);
#endif
}


AT(.text.bsp.karaok)
void bsp_karaok_exit(u8 path)
{
    if (sys_cb.karaok_init == 0) {
        return;
    }
    printf("%s\n", __func__);
    karaok_exit();
#if SYS_MAGIC_VOICE_EN
    unlock_code_wsola();
#endif // SYS_MAGIC_VOICE_EN
#if SYS_PITCH_SHIFT
    unlock_code_pitch();
#endif

#if SYS_PITCH_SHIFT
//    AUANGCON0 &= ~BIT(29);
//    AUANGCON0 &= ~BIT(28);
#endif
    audio_path_exit(path);
    if (sys_cb.sysclk_bak) {
        set_sys_clk(sys_cb.sysclk_bak);
    }
    sys_cb.karaok_init = 0;
#if KARAOK_DENOISE_EN
    void unlock_code_karaok_dednr(void);
    unlock_code_karaok_dednr();
#endif // KARAOK_DENOISE_EN
}

#if KARAOK_REC_EN
AT(.text.bsp.karaok)
void bsp_karaok_rec_init(void)
{
    karaok_rec_init(rec_cb.src->nchannel & 0x0f);
}

AT(.text.bsp.karaok)
void bsp_karaok_rec_exit(void)
{
//    bsp_karaok_echo_reset_buf(func_cb.sta);
}
#endif
#endif

#if AUDIO_STRETCH_EN

int dac_music_speed_change_en(bool flag);
void magic_voice_init(void *cfg);
int speed_mono_dual_set(bool flag);	//设置真假立体声 flag:0,假立体；1，真立体
void magic_voice_set_level(u8 type, u8 level);


AT(.text.bsp.karaok)
void bsp_stretch_init(void)
{
	printf("<bsp_stretch_init>\n");
#if !SYS_MAGIC_VOICE_EN
	printf("<magic_voice_init>\n");
    lock_code_wsola();
	magic_voice_init(&mav_cfg);
#endif
	if(dac_music_speed_change_en(1) == -1)		//因为不能和魔音共用所以只能是1
		printf("dac_music_speed_set fail!\n");	//使能变速
	speed_mono_dual_set(1);						//1:真立体 0:假立体 不设置默认假立体
}

AT(.text.bsp.karaok)
void stretch_voice_set_level(u8 type, u8 level)
{
	magic_voice_set_level(type, level);
}


#endif



#if KARAOK_MIX_PLAY_WAV_EN
//-------------------------------------------------------------------------
//PCM(WAV)资源文件混到KARAOK_MIC数据流中一起播放出来(常用于开KARAOK时,该PCM提示音与KARAOK一起输出)
//提示音是11.025K的单声道WAV(PCM)数据, KARAOK_MIC是44.1K的,一个PCM数据需要混到4个KARAOK_MIC数据上
//需要播放提示音的地方,直接调用 kmix_play_wav就行
//kmix_play_wav(RES_BUF_MAX_VOL_WAV,RES_LEN_MAX_VOL_WAV);
enum  {
    KMIX_STOP = 0,
    KMIX_RUN,
};
typedef struct _KMIX_T {
    s16 *buf;
    u16 samples_total;
    u16 samples_idx;
    u8 div_cnt;
    volatile u8 sta;
} KMIX_T;
KMIX_T  kmix;
//WAV资源文件混到KARAOK_MIC数据流中一起播放出来
void kmix_play_wav(u32 addr, u32 len)
{
    //printf("pcm_len = %d\n",pcm_len);
    if (len < 44) {
        return;
    }
    len = len - 44;
    kmix.sta = KMIX_STOP;
    kmix.buf = (s16*)(addr + 44);            //wav 有44byte 头信息
    kmix.samples_total = ((u16)len-256)/2;   //资源文件中的数据以256BYTE对齐,减去最后256BYTE,防止取到无效的对齐数据
    kmix.samples_idx = 0;
    kmix.div_cnt = 0;
    kmix.sta  = KMIX_RUN;
}

AT(.com_text.karaok.proc)
s16 karaok_sdadc_weak(s16 ptr)  //库中回调函数,传入的时KARAOK_MIC数据,传出的是混合提示音后的数据
{
    s16 mic_data = ptr;
    if (KMIX_STOP == kmix.sta) {
        return mic_data;
    } else{   //KMIX_RUN
        kmix.div_cnt++;
        if (kmix.div_cnt > 3) {  //PCM提示音是11.025K的, KARAOK_MIC是44.1K的,一个PCM数据需要混到4个KARAOK_MIC数据上
            kmix.div_cnt = 0;
            kmix.samples_idx++;
            if (kmix.samples_idx >= kmix.samples_total) {
                kmix.sta = KMIX_STOP;
            }
        }
        s32 mix_data = mic_data + kmix.buf[kmix.samples_idx];
        if (mix_data > 32767) {
            mix_data = 32767;
        } else if (mix_data < -32768) {
            mix_data = -32768;
        }
        return (s16)mix_data;
    }
}
#endif




