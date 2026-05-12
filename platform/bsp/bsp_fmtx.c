#include "include.h"
#include "bsp_fmtx.h"

#if FMTX_EN

typedef struct {
    u8 res[2];
    u16 dac_en      : 1;        //是否同时DAC输出
    u16 pud_out     : 1;        //是否上下拉方式FMTX输出
    u16 amp_en      : 1;        //是否放大器FMTX输出
    u16 drv_level   : 2;        //0: 2.6mA,  1: 5.25mA,  2: 8mA, 3: 24mA
    u16 lpf         : 2;
    u16 ch_mode     : 2;
} fmtx_cfg_t;

fmtx_cb_t fmtx_cb;

#define FMTX_USE_OTHER_IO       0
//关于FMTX_USE_OTHER_IO 宏配置说明(IO选择见FMTX_OTHER_IO_SEL)
//1:使用其它IO做FMTX(需要打开FMTX_EN),只有下面的 FMTX_OTHER_IO_POWER / FMTX_OTHER_IO_FIX_BT 优化选项可选,主要是优化对蓝牙的干扰 (config.h中的FMTX_POWER/FMTX_AMP_EN/FMTX_PUD_OUT会无效)
//0:使用PA6做FMTX会有更多的优化配置选项见config.h 中的 FMTX_POWER/FMTX_AMP_EN/FMTX_PUD_OUT

#if FMTX_USE_OTHER_IO           //FMTX选其它IO时的配置项
#define FMTX_OTHER_IO_SEL       IO_PB1    //可以使用的IO如下：IO_PA5, IO_PA6, IO_PB0, IO_PB1,IO_PE5, IO_PE6   //NOTE IO_PB1默认是SD卡引脚,需要把 MUSIC_SDCARD_EN 关掉。
#define FMTX_OTHER_IO_POWER     0         //1 大功率发射(FMTX距离远但对蓝牙干扰大些)  //0小功率发射(FM距离近些,对蓝牙干扰小些)
#define FMTX_OTHER_IO_FIX_BT    0         //1 对蓝牙干扰小但FM发射距离近一些  //0 FM发射距离远但对蓝牙干扰大些  //如果 FMTX_OTHER_IO_POWER配置成1还压不住对蓝牙的干扰,可以把 FMTX_OTHER_IO_FIX_BT设置为1进一步降低对蓝牙干扰

#define FMTX_OTHER_IO_CFG (FMTX_OTHER_IO_SEL | FMTX_OTHER_IO_POWER<<6 | FMTX_OTHER_IO_FIX_BT<<7)
//bool cfg_fmtx_some_freq_fixbt(void){return false;}  //true:少量频率强制使用FIX_BT降低蓝牙干扰,FMTX距离会近一些 //false:正常发射
#endif
fmtx_cfg_t fmtx_cfg = {
    .res[0]     = 0,
#if FMTX_USE_OTHER_IO
    .res[1]     = FMTX_OTHER_IO_CFG,
#else
    .res[1]     = 0,
#endif
    .dac_en     = FMTX_DAC_EN,
    .pud_out    = FMTX_PUD_OUT,
    .amp_en     = FMTX_AMP_EN,
    .drv_level  = FMTX_POWER,
    .lpf        = 2,
    .ch_mode    = FMTX_CHANNEL_MODE,
};

static const u16 fmtx_dig_vol_30[30 + 1] = {
    FMTX_DIG_N60DB,
    FMTX_DIG_N58DB, FMTX_DIG_N56DB, FMTX_DIG_N54DB, FMTX_DIG_N52DB, FMTX_DIG_N50DB,
    FMTX_DIG_N48DB, FMTX_DIG_N46DB, FMTX_DIG_N44DB, FMTX_DIG_N42DB, FMTX_DIG_N40DB,
    FMTX_DIG_N38DB, FMTX_DIG_N36DB, FMTX_DIG_N34DB, FMTX_DIG_N32DB, FMTX_DIG_N30DB,
    FMTX_DIG_N28DB, FMTX_DIG_N26DB, FMTX_DIG_N24DB, FMTX_DIG_N22DB, FMTX_DIG_N20DB,
    FMTX_DIG_N18DB, FMTX_DIG_N16DB, FMTX_DIG_N14DB, FMTX_DIG_N12DB, FMTX_DIG_N10DB,
    FMTX_DIG_N8DB,  FMTX_DIG_N6DB,  FMTX_DIG_N5DB,  FMTX_DIG_N4DB,  FMTX_DIG_N3DB,
};

static const u16 fmtx_dig_vol_16[16 + 1] = {
    FMTX_DIG_N60DB,
    FMTX_DIG_N43DB,  FMTX_DIG_N32DB,  FMTX_DIG_N26DB,  FMTX_DIG_N24DB,
    FMTX_DIG_N22DB,  FMTX_DIG_N20DB,  FMTX_DIG_N18DB,  FMTX_DIG_N16DB,
    FMTX_DIG_N14DB,  FMTX_DIG_N12DB,  FMTX_DIG_N10DB,  FMTX_DIG_N8DB,
    FMTX_DIG_N6DB,   FMTX_DIG_N5DB,   FMTX_DIG_N4DB,   FMTX_DIG_N3DB,
};

AT(.text.bsp.fmtx)
u16 bsp_fmtx_freq_inc(u16 freq)
{
    freq += 10;
    if (freq > FM_FREQ_MAX) {
        freq = FM_FREQ_MIN;
    }
    return freq;
}

AT(.text.bsp.fmtx)
u16 bsp_fmtx_freq_dec(u16 freq)
{
    freq -= 10;
    if (freq < FM_FREQ_MIN) {
        freq = FM_FREQ_MAX;
    }
    return freq;
}

AT(.text.bsp.fmtx)
void bsp_fmtx_set_vol(u8 vol)
{
    fmtx_set_vol(fmtx_cb.vol_table[vol]);
}

AT(.text.bsp.fmtx)
void bsp_fmtx_set_freq(u16 freq)
{
    if (fmtx_cb.freq == freq) {
        return;
    }
    printf("fmtx freq:%d\n", freq);
    fmtx_cb.freq = freq;
    fmtx_set_freq(fmtx_cb.freq);
    param_fmtx_freq_write();
}

AT(.text.bsp.fmtx)
void bsp_fmtx_init(void)
{
    memset(&fmtx_cb, 0, sizeof(fmtx_cb_t));
    if (!fmtx_init(&fmtx_cfg)) {
        printf("fmtx init fail\n");
        return;
    }
    if (!fmtx_cfg.dac_en) {
        dac_set_dvol(0x7fff);
    }

    if (VOL_MAX == 30) {
        fmtx_cb.vol_table = fmtx_dig_vol_30;
    } else if (VOL_MAX == 16) {
        fmtx_cb.vol_table = fmtx_dig_vol_16;
    } else {
        printf("fmtx vol err\n");
    }
    fmtx_vol_amp(FMTX_AMP_TIMES);
    param_fmtx_freq_read();
    fmtx_cb.freq = 8780;
    if (fmtx_cb.freq == 0xffff || fmtx_cb.freq == 0) {
        fmtx_cb.freq = FM_FREQ_MIN;
    }
    fmtx_set_freq(fmtx_cb.freq);
    param_fmtx_freq_write();
}
#endif
