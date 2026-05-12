#include "include.h"
#include "func.h"
#include "func_fmrx.h"

fmrx_cb_t fmrx_cb AT(.buf.fmrx.cb);

#if FUNC_FMRX_EN


#define FMRX_INFO_PRINT    0

#if FMRX_INFO_PRINT
const char *str_cs_filter[4] = {"125K","75K","50K","25K"};
const char *str_src0_out[2] = {"44.1K", "48K"};
const char *str_src0_in[16] = {"48K", "44.1K", "38K", "32K", "24K","22.05K", "16K", "12K", "11.025K", "8K", "6K", "4K", "Err","Err","Err","Err"};
const char *str_fm_auch[2] = {"Mono", "Dual"};
AT(.text.fmrx)
void fmrx_info_print(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 2000) && fmrx_is_playing()) {
        ticks = tick_get();
        printf("\n===>fmrx 1S info print, freq = %d\n", fmrx_cb.freq);
        printf("rssi_powdb = %d ,fm cs_filter = %d, %s\n", fmrx_get_rssi_powdb(), fmrx_get_cs_filter(), str_cs_filter[fmrx_get_cs_filter()]);
        printf("audio_ch_set = %d(%s), fmrx_is_stereo = %d\n",fmrx_get_audio_channel(), str_fm_auch[fmrx_get_audio_channel()], fmrx_cur_freq_is_stereo());
        //printf("fmrx r_val = %d, z_val = %d, fz_val = %d, d_val = %d\n",fmrx_thd.r_val,fmrx_thd.z_val,fmrx_thd.fz_val,fmrx_thd.d_val);
    }
}
#endif
//先暂停FMRX再播放提示音
AT(.text.func.fmrx)
void func_fmrx_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }

    if (fmrx_cb.sta == FMRX_PAUSE) {
        mp3_res_play(addr, len);
    } else {
        dac_fade_out();
        dac_fade_wait();
        fmrx_digital_stop();
        mp3_res_play(addr, len);
        fmrx_digital_start();
        dac_fade_in();
    }
}

//FMRX播放暂停控制
AT(.text.func.fmrx)
void func_fmrx_pause_play(void)
{
    if (fmrx_cb.sta == FMRX_PLAY) {
        dac_fade_out();
        dac_fade_wait();
        fmrx_digital_stop();
        fmrx_cb.sta = FMRX_PAUSE;
        led_idle();
    } else if (fmrx_cb.sta == FMRX_PAUSE) {
        fmrx_digital_start();
        dac_fade_in();
        fmrx_cb.sta = FMRX_PLAY;
        led_fm_play();
    }
}

//调音量解除FMRX暂停
AT(.text.func.fmrx)
void func_fmrx_setvol_callback(u8 dir)
{
    if (fmrx_cb.sta == FMRX_PAUSE) {
        func_fmrx_pause_play();
    }
}


AT(.text.func.fmrx)
void save_ch_buf(u16 freq)
{
    unsigned char index, bit_pos;
    index = (freq - 875) / 16;
    bit_pos = (freq - 875) % 16;
    fmrx_cb.buf[index] |= BIT(bit_pos);
}

AT(.text.func.fmrx)
u16 get_ch_freq(u8 ch)
{
    u8 i, j;
    for (i = 0; i < 13; i++) {
        for (j = 0; j < 16; j++) {
            if (fmrx_cb.buf[i] & BIT(j)) {
                ch--;
                if (ch == 0) {
                    return ((j + i*16) + 875)*10;
                }
            }
        }
    }
    return FM_FREQ_MIN;
}

AT(.text.func.fmrx)
void reset_fm_cb(void)
{
    fmrx_cb.freq = FM_FREQ_MIN;
    fmrx_cb.ch_cnt = 0;
    fmrx_cb.ch_cur = 1;
    memset(fmrx_cb.buf, 0, 26);
}

static void fmrx_set_curch_freq(void)
{
    param_fmrx_chcur_write();
    param_sync();
    fmrx_cb.freq = get_ch_freq(fmrx_cb.ch_cur);
    gui_box_show_chan();
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
    printf("ch freq: %d,  %d\n", fmrx_cb.freq, fmrx_cb.ch_cur);
}

AT(.text.func.fmrx)
void fmrx_switch_freq(u8 dir)
{
    if (dir > 0) {
        fmrx_cb.freq += 10;
        if (fmrx_cb.freq > 10800) {
            fmrx_cb.freq = 8750;
        }
    } else {
        fmrx_cb.freq -= 10;
        if (fmrx_cb.freq < 8750) {
            fmrx_cb.freq = 10800;
        }
    }
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
    printf("switch freq: %d\n", fmrx_cb.freq);
}

AT(.text.func.fmrx)
void fmrx_switch_channel(u8 dir)
{
    sys_cb.mute = 0;
    if (fmrx_cb.ch_cnt == 0) {
        return;
    }

    if (dir > 0) {
        fmrx_cb.ch_cur++;
        if (fmrx_cb.ch_cur > fmrx_cb.ch_cnt) {
            fmrx_cb.ch_cur = 1;
        }
    } else {
        fmrx_cb.ch_cur--;
        if (fmrx_cb.ch_cur < 1) {
            fmrx_cb.ch_cur = fmrx_cb.ch_cnt;
        }
    }
    fmrx_set_curch_freq();
}


AT(.text.func.fmrx)
void fmrx_seek_start(void)
{
    printf("fmrx seek start\n");
    reset_fm_cb();
    fmrx_cb.seek_start = 1;
    fmrx_cb.sta = FMRX_SEEKING;
    dac_fade_out();
}

AT(.text.func.fmrx)
void fmrx_seek_stop(void)
{
    printf("fmrx seek stop\n");
    fmrx_cb.sta = FMRX_PLAY;
    fmrx_cb.freq = FM_FREQ_MIN;
    if (fmrx_cb.ch_cnt > 0) {
        fmrx_cb.ch_cur = 1;
        fmrx_cb.freq = get_ch_freq(fmrx_cb.ch_cur);
    }
    param_fmrx_chcur_write();
    param_fmrx_chcnt_write();
    param_fmrx_chbuf_write();
    param_sync();

    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
}

#if FMRX_HALF_SEEK_EN
AT(.text.func.fmrx)
void fmrx_half_seek_start(u8 dir)
{
    sys_cb.mute = 0;
    if (fmrx_cb.sta != FMRX_PLAY) {
        return;
    }
    fmrx_cb.seek_start = 2;
    fmrx_cb.hstep = -10;
    if (dir) {
        fmrx_cb.hstep = 10;
    }
    fmrx_cb.fcnt = 0;
    fmrx_cb.freq += fmrx_cb.hstep;
    if (fmrx_cb.freq > FM_FREQ_MAX) {
        fmrx_cb.freq = FM_FREQ_MIN;
    } else if (fmrx_cb.freq < FM_FREQ_MIN) {
        fmrx_cb.freq = FM_FREQ_MAX;
    }
    fmrx_cb.sta = FMRX_SEEKING_HALF;
    led_fm_scan();
    dac_fade_out();
}

AT(.text.func.fmrx)
void fmrx_half_seek_stop(void)
{
    fmrx_cb.seek_start = 0;
    fmrx_cb.sta = FMRX_PLAY;
//    led_fm_play();
//    bsp_fmrx_set_freq(fmrx_cb.freq);
//    dac_fade_in();
}
#endif // FMRX_HALF_SEEK_EN

static void func_fmrx_box_event(void)
{
#if IR_NUMKEY_EN
    if (box_cb.event) {
        sys_cb.mute = 0;
        box_cb.event = 0;
        if ((box_cb.number > 0) && (box_cb.number < fmrx_cb.ch_cnt)) {
            fmrx_cb.ch_cur = box_cb.number;
            fmrx_set_curch_freq();
        } else if ((box_cb.number >= FM_FREQ_MIN/10) && (box_cb.number <= FM_FREQ_MAX/10)) {
            fmrx_cb.freq = box_cb.number*10;
            bsp_fmrx_set_freq(fmrx_cb.freq);
        }
    }
#endif // IR_NUMKEY_EN
}
u8 count = 1;
AT(.text.func.fmrx)
void func_fmrx_process(void)
{
#if FMRX_INFO_PRINT
    fmrx_info_print();
#endif

    func_process();
    func_fmrx_box_event();
    switch (fmrx_cb.sta) {
        case FMRX_PLAY:
            break;

#if FMRX_HALF_SEEK_EN
        case FMRX_SEEKING_HALF:
            if(count)
            {
            //第一次清空一下
            reset_fm_cb();
            count--;
            }

            if (bsp_fmrx_check_freq(fmrx_cb.freq)) {
                fmrx_half_seek_stop();
                                fmrx_cb.ch_cnt++;
                save_ch_buf(fmrx_cb.freq / 10);
                printf("ch freq: %d\n", fmrx_cb.freq);
                fmrx_cb.ch_cur = fmrx_cb.ch_cnt;
                    param_fmrx_chcur_write();
                    param_fmrx_chcnt_write();
                    param_fmrx_chbuf_write();
                    param_sync();
                bsp_fmrx_set_freq(fmrx_cb.freq);
                dac_fade_in();

                break;
            }
            fmrx_cb.freq += fmrx_cb.hstep;
            if (fmrx_cb.freq > FM_FREQ_MAX) {
                fmrx_cb.freq = FM_FREQ_MIN;
            } else if (fmrx_cb.freq < FM_FREQ_MIN) {
                fmrx_cb.freq = FM_FREQ_MAX;
            }
            break;
#endif // FMRX_HALF_SEEK_EN

        case FMRX_SEEK_START:
            led_fm_scan();
            fmrx_seek_start();
            break;

        case FMRX_SEEKING:
            if (bsp_fmrx_check_freq(fmrx_cb.freq)) {
                fmrx_cb.ch_cnt++;
                save_ch_buf(fmrx_cb.freq / 10);
                plugin_fm_seek_chan();
//                printf("ch freq: %d\n", fmrx_cb.freq);
                fmrx_cb.ch_cur = fmrx_cb.ch_cnt;
                bsp_fmrx_set_freq(fmrx_cb.freq);
#if FMRX_SEEK_DISP_CH_EN
                gui_display(DISP_FQ_CHAN);
#endif // FMRX_SEEK_DISP_CH_EN
                dac_fade_in();
                delay_5ms(300);
                dac_fade_out();
                plugin_fm_seek_continue();
            }
            fmrx_cb.freq += 10;
            if (fmrx_cb.freq > FM_FREQ_MAX) {
                fmrx_cb.sta = FMRX_SEEK_STOP;
            }
            break;

        case FMRX_SEEK_STOP:
            led_fm_play();
            fmrx_seek_stop();
            break;
    }
}


AT(.text.func.fmrx)
void func_fmrx_enter(void)
{
#if MICAUX_ANALOG_OUT_ALWAYS
    micaux_analog_out_init();
#endif // MICAUX_ANALOG_OUT_ALWAYS

    memset(&fmrx_cb, 0, sizeof(fmrx_cb));
    if (!is_func_fmrx_en()) {
        func_cb.sta = FUNC_NULL;
        return;
    }
    func_cb.mp3_res_play = func_fmrx_mp3_res_play;
    func_cb.set_vol_callback = func_fmrx_setvol_callback;

    param_fmrx_chcur_read();
    param_fmrx_chcnt_read();
    param_fmrx_chbuf_read();
    fmrx_cb.freq = get_ch_freq(fmrx_cb.ch_cur);
    msg_queue_clear();

    led_fm_play();
    func_fmrx_enter_display();
    AMPLIFIER_SEL_AB();
#if (EXLIB_SOFT_EQ_DRC_EN || EXLIB_BT_MONO_XDRC_EN)
    fmrx_cb.pcm_ext = cfg_dac_pcm_ext_en;
    cfg_dac_pcm_ext_en = 0;
#endif

#if WARNING_FUNC_FMRX
    mp3_res_play(RES_BUF_FM_MODE_MP3, RES_LEN_FM_MODE_MP3);
#endif // WARNING_FUNC_FMRX

    set_sys_clk(fmrx_sysclk_config());

    bsp_fmrx_init();

#if FMRX_OPTIMIZE_TRY
    fmrx_rf_res_set(0);        //内部天线阻抗匹配值,取值范围:0~3  //default 0
    fmrx_optimize_try_set();   //尝试时钟控制,CLK分频等
    printf("FMRXANGCON0 = 0x%X\n",FMRXANGCON0);
#endif

    MEMCON &= ~0x3ff;                   // ram & rom memory always work
    led_fm_play();

    fmrx_cb.sta = FMRX_PLAY;

#if FMRX_TEST_CHANNEL
    fmrx_cb.freq = fmrx_test_first_channel_get();
#endif
//    fmrx_cb.freq = 9510;   //8750  //8770  //9150
    bsp_fmrx_set_freq(fmrx_cb.freq);
    dac_fade_in();
    printf("%s: %d\n", __func__, fmrx_cb.freq);
}

AT(.text.func.fmrx)
void func_fmrx_exit(void)
{
    dac_fade_out();
    func_fmrx_exit_display();
    if (fmrx_cb.sta > FMRX_IDLE) {
        bsp_fmrx_exit();
    }
#if FMRX_OPTIMIZE_TRY
    fmrx_optimize_try_recover();
#endif

#if (EXLIB_SOFT_EQ_DRC_EN || EXLIB_BT_MONO_XDRC_EN)
    cfg_dac_pcm_ext_en = fmrx_cb.pcm_ext;
#endif
    set_sys_clk(SYS_CLK_SEL);
    adpll_spr_set(DAC_OUT_SPR);
    MEMCON |= 0x3ff;                   // ram & rom memory standby when no peripheral access
    AMPLIFIER_SEL_D();
    func_cb.last = FUNC_FMRX;
}

AT(.text.func.fmrx)
void func_fmrx(void)
{
    printf("%s\n", __func__);
    func_fmrx_enter();
    while (func_cb.sta == FUNC_FMRX) {
        func_fmrx_process();
        func_fmrx_message(msg_dequeue());
        func_fmrx_display();
    }

    func_fmrx_exit();
}
#endif // FUNC_FMRX_EN
