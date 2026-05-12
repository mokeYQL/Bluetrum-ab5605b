#include "include.h"
#include "func.h"

f_spdif_t f_spdif;
spd_cfg_t spd_cfg;

#if FUNC_SPDIF_EN

static void func_spdif_process(void)
{
    func_process();
    spdif_process();
    if (!spdif_is_online()) {
        func_cb.sta = FUNC_NULL;
    }
}

static void func_spdif_enter(void)
{
#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
    memset(&spd_cfg, 0, sizeof(spd_cfg_t));
    memset(&f_spdif, 0, sizeof(f_spdif_t));
    spd_cfg.channel = SPDIF_IN_CH;

#if SYS_CLK_SEL == SYS_160M
    set_sys_clk(SYS_120M);
#endif
    spdif_init(&spd_cfg);

    if (!spdif_is_online()) {
        func_cb.sta = FUNC_NULL;
        return;
    }
    adpll_spr_set(1);
    func_spdif_enter_display();

#if WARNING_FUNC_SPDIF
    spdif_exit();
    mp3_res_play(RES_BUF_SPDIF_MODE_MP3, RES_LEN_SPDIF_MODE_MP3);
    spdif_init(&spd_cfg);
#endif // WARNING_FUNC_SPDIF

#if SPDIF_IN_SHAPE
    spdif_shaping(1);
#endif
#if FUNC_SPDIF_TX_EN
    spdif_tx_en(0);
#endif
#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, FUNC_SPDIF);
#endif
    spdif_start();
}

static void func_spdif_exit(void)
{
    dac_fade_out();
    func_spdif_exit_display();
    spdif_exit();
    adpll_spr_set(DAC_OUT_SPR);
#if SPDIF_IN_SHAPE
    spdif_shaping(0);           //关闭PB5整形输出
#endif
#if SYS_CLK_SEL == SYS_160M
    set_sys_clk(SYS_160M);
#endif
#if FUNC_SPDIF_TX_EN
    spdif_tx_en(1);
#endif
    func_cb.last = FUNC_SPDIF;
}

AT(.text.func.spdif)
void func_spdif(void)
{
    printf("%s\n", __func__);

    func_spdif_enter();

    while (func_cb.sta == FUNC_SPDIF) {
        func_spdif_process();
        func_spdif_message(msg_dequeue());
        func_spdif_display();
    }

    func_spdif_exit();
}

#endif

#if FUNC_SPDIF_TX_EN
void func_spdif_tx_init(void)
{
    memset(&spd_cfg, 0, sizeof(spd_cfg_t));
    spd_cfg.channel = SPDIF_TX_CH;

    spdif_tx_init(&spd_cfg);
}
#endif
