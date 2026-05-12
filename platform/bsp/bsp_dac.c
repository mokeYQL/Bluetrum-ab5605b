#include "include.h"
#include "bsp_dac.h"

#if MICAUX_ANALOG_OUT_ALWAYS
u8 cfg_dac_fade_out_dis = true;   //打开屏蔽后dac_fade_out 无效,库中有同名的WEAK变量默认值为false
#endif

typedef struct {
	u8 eq_en;
	u8 eq_num;
	u8 eq_rfu[2];
	u8 *eq_coef;

	u8 drc_en;
	u8 drc_type;
	u8 drc_rfu[2];
	u8 *drc_coef;
} eq_drc_cfg_t;

const u8  *dac_avol_table;
const u16 *dac_dvol_table;

AT(.text.bsp.dac.table)
const u8 dac_vol_tbl_16[16 + 1] = {
    //取值范围：(54-54) ~ (54+5),共60个值;
    //说明：值(54-54)对应音量-54dB; 54对应音量0dB;
    54-54, 54-43, 54-32, 54-26, 54-22, 54-18, 54-15, 54-12,
    54-10, 54-8,  54-6,  54-4,  54-3,  54-2,  54-1,  54,  54+1
};

AT(.text.bsp.dac.table)
const u8 dac_vol_tbl_32[32 + 1] = {
    //取值范围：(54-54) ~ (54+5),共60个值;
    //说明：值(54-54)对应音量-54dB; 54对应音量0dB;
    54-54, 54-43, 54-38, 54-35, 54-32, 54-30, 54-28, 54-26,
    54-24, 54-22, 54-21, 54-20, 54-19, 54-18, 54-17, 54-16,
    54-15, 54-14, 54-13, 54-12, 54-11, 54-10, 54-9,  54-8,
    54-7,  54-6,  54-5,  54-4,  54-3,  54-2,  54-1,  54,
    54+1,
};

AT(.text.bsp.dac.table)
const u8 dac_vol_tbl_50[50 + 1] = {
    //取值范围：(54-54) ~ (54+5),共60个值;
    //说明：值(54-54)对应音量-54dB; 54对应音量0dB;
    54-54, 54-43, 54-39, 54-38, 54-37, 54-36, 54-35, 54-34,
    54-33, 54-32, 54-31, 54-30, 54-29, 54-28, 54-27, 54-26,
    54-25, 54-24, 54-23, 54-22, 54-21, 54-20, 54-19, 54-18,
    54-17, 54-16, 54-15, 54-14, 54-13, 54-12, 54-11, 54-10,
    54-9,  54-8,  54-7,  54-6,  54-6,  54-5,  54-5,  54-4,
    54-4,  54-3,  54-3,  54-2,  54-2,  54-1,  54-1,  54,
    54,    54+1,  54+1,
};

AT(.text.bsp.dac.table)
const u16 dac_dvol_tbl_16[16 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N24DB,  DIG_N22DB,  DIG_N20DB,  DIG_N18DB, DIG_N16DB,
    DIG_N14DB,  DIG_N12DB,  DIG_N10DB,  DIG_N8DB,   DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

AT(.text.bsp.dac.table)
const u16 dac_dvol_tbl_32[32 + 1] = {
    DIG_N60DB,  DIG_N50DB,  DIG_N43DB,  DIG_N38DB,  DIG_N35DB,  DIG_N30DB,  DIG_N28DB,  DIG_N26DB,
    DIG_N24DB,  DIG_N22DB,  DIG_N21DB,  DIG_N20DB,  DIG_N19DB,  DIG_N18DB,  DIG_N17DB,  DIG_N16DB,
    DIG_N16DB,  DIG_N15DB,  DIG_N14DB,  DIG_N13DB,  DIG_N12DB,  DIG_N11DB,  DIG_N10DB,  DIG_N9DB,
    DIG_N8DB,   DIG_N7DB,   DIG_N6DB,   DIG_N5DB,   DIG_N4DB,   DIG_N3DB,   DIG_N2DB,   DIG_N1DB,   DIG_N0DB,
};

AT(.text.bsp.dac.table)
const u16 dac_dvol_tbl_50[50 + 1] = {
    DIG_N60DB,  DIG_N56DB,  DIG_N52DB,  DIG_N48DB,  DIG_N44DB,  DIG_N42DB,  DIG_N40DB,  DIG_N38DB,
    DIG_N37DB,  DIG_N36DB,  DIG_N35DB,  DIG_N34DB,  DIG_N33DB,  DIG_N32DB,  DIG_N31DB,  DIG_N30DB,
    DIG_N29DB,  DIG_N28DB,  DIG_N28DB,  DIG_N27DB,  DIG_N26DB,  DIG_N25DB,  DIG_N25DB,  DIG_N24DB,
    DIG_N23DB,  DIG_N22DB,  DIG_N21DB,  DIG_N20DB,  DIG_N20DB,  DIG_N19DB,  DIG_N18DB,  DIG_N18DB,
    DIG_N17DB,  DIG_N16DB,  DIG_N16DB,  DIG_N15DB,  DIG_N14DB,  DIG_N13DB,  DIG_N12DB,  DIG_N11DB,
    DIG_N10DB,  DIG_N9DB,   DIG_N8DB,   DIG_N7DB,   DIG_N6DB,   DIG_N5DB,   DIG_N4DB,   DIG_N3DB,
    DIG_N2DB,   DIG_N1DB,   DIG_N0DB,
};

AT(.text.bsp.aucar.table)
const u16 aucar_bal_vol_table[16 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N24DB,  DIG_N22DB,  DIG_N20DB,  DIG_N18DB, DIG_N16DB,
    DIG_N14DB,  DIG_N12DB,  DIG_N10DB,  DIG_N8DB,   DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

//dacr_not_poweron 使用场景：如DACR和PF3绑定在一起
//若此函数返回1, DACR在DAC初始化时进入低驱动状态，此时该绑定引脚电平可由PF3决定
//AT(.text.dac)
//u8 dacr_not_poweron(void)
//{
//    return 1;
//}

AT(.text.bsp.dac)
void bsp_change_volume(u8 vol)
{
#if DYNAMIC_BASS_BOOST_EN
    music_bass_boost_vol_set(vol);
    return;
#endif

#if FMTX_EN
    bsp_fmtx_set_vol(vol);
#endif

#if !SYS_ADJ_DIGVOL_EN
    ///sys adjust dac analog volume
    u8 anl_vol = 0;
    if (vol <= VOL_MAX) {
        if (vol > 0) {
            anl_vol = dac_avol_table[vol] + sys_cb.anl_gain_offset;
        }
        dac_set_volume(anl_vol);
        if(anl_vol == 0)
        {
            dac_set_dvol(0);
        }
        else{
            dac_set_dvol(0x7ca0);
        }
    }
#else
    ///sys adjust dac digital volume
    u16 dig_vol = 0;
    if (vol <= VOL_MAX) {
        dig_vol = dac_dvol_table[vol];
#if EXLIB_SOFT_EQ_DRC_EN
    softeq_drc_vol_set(dig_vol);
#elif EXLIB_BT_MONO_XDRC_EN
    xdrc_sysvol_set(dig_vol);
#else
    dac_set_dvol(dig_vol);
#endif
    }
#endif
}

AT(.text.bsp.dac)
bool bsp_set_volume(u8 vol)
{
    bsp_change_volume(vol);
    if (vol == sys_cb.vol) {
        gui_box_show_vol();
        return false;
    }

    if (vol <= VOL_MAX) {
        sys_cb.vol = vol;
        gui_box_show_vol();
        param_sys_vol_write();
        sys_cb.cm_times = 0;
        sys_cb.cm_vol_change = 1;
    }
    return true;
}

//左右音量平衡控制
AT(.text.bsp.aucar)
void bsp_aucar_set_bal_vol(u8 l_vol, u8 r_vol)
{
//    printf("bal_vol: %d, %d\n", l_vol, r_vol);
    dac_set_balance(aucar_bal_vol_table[l_vol], aucar_bal_vol_table[r_vol]);
}

//开机控制DAC电容放电等待时间
AT(.text.dac)
void dac_pull_down_delay(void)
{
    delay_5ms(DAC_PULL_DOWN_DELAY);
}

AT(.com_text.dac)
void dac_set_mute_callback(u8 mute_flag)
{
    if (mute_flag) {
        bsp_loudspeaker_mute();
    } else {
        if (!sys_cb.mute) {
            bsp_loudspeaker_unmute();
#if EARPHONE_DETECT_EN
            if (!dev_is_online(DEV_EARPHONE))
#endif
            {
                //DAC延时淡入，防止UNMUTE时间太短导致喇叭声音不全的问题
                dac_unmute_set_delay(LOUDSPEAKER_UNMUTE_DELAY);
            }
        }
    }
}

AT(.text.bsp.dac)
u8 bsp_volume_inc(u8 vol)
{
    vol++;
    if(vol > VOL_MAX)
        vol = VOL_MAX;
    return vol;
}

AT(.text.bsp.dac)
u8 bsp_volume_dec(u8 vol)
{
    if(vol > 0)
        vol--;
    return vol;
}

AT(.text.bsp.dac)
void dac_set_anl_offset(u8 bt_call_flag)
{
    if (bt_call_flag) {
        sys_cb.anl_gain_offset = 54 - 9 + BT_CALL_MAX_GAIN - dac_avol_table[VOL_MAX];
    } else {
        sys_cb.anl_gain_offset = 54 - 9 + DAC_MAX_GAIN - dac_avol_table[VOL_MAX];
    }
}

AT(.text.bsp.dac)
void dac_set_vol_table(u8 vol_max)
{
    if (vol_max == 16) {
        dac_avol_table = dac_vol_tbl_16;
        dac_dvol_table = dac_dvol_tbl_16;
    } else if (vol_max <= 32) {
        dac_avol_table = dac_vol_tbl_32;
        dac_dvol_table = dac_dvol_tbl_32;
    } else {
        dac_avol_table = dac_vol_tbl_50;
        dac_dvol_table = dac_dvol_tbl_50;
    }
    dac_set_anl_offset(0);
}



//AT(.text.dac.hiz)
//void dac_high_impedance(u8 enable)
//{
//    static u32 auangcon0 = 0;
//    static u32 auangcon3 = 0;
//    if (enable) {   //需要先调用一下enable,保存一下以前的dac配置值。
//        auangcon0 = AUANGCON0;
//        auangcon3 = AUANGCON3;
//        AUANGCON0 &= ~(0x03 << 7);
//        AUANGCON3 &= ~(0x07 << 27);
//    } else {
//        AUANGCON0 = auangcon0;
//        AUANGCON3 = auangcon3;
//    }
//}

AT(.text.bsp.dac)
void dac_init(void)
{
    dac_set_vol_table(xcfg_cb.vol_max);
    printf("[%s] vol_max:%d, offset: %d\n", __func__, xcfg_cb.vol_max, sys_cb.anl_gain_offset);

    dac_obuf_init();
    dac_power_on();

#if (DAC_OUT_SPR == DAC_OUT_48K)
    DACDIGCON0 |= BIT(1);           //dac out sample 48K
#endif
    dac_digital_enable();

    plugin_music_eq();

#if SYS_ADJ_DIGVOL_EN || DYNAMIC_BASS_BOOST_EN
    dac_set_volume(54);             //analog volume设置0DB
#endif // SYS_ADJ_DIGVOL_EN

#if DAC_DNR_EN
    dac_dnr_init(2, 0x10, 88, 0x10);
#endif

#if EXLIB_SOFT_EQ_DRC_EN
    audio_softeq_drc_param_init();
#endif


}









