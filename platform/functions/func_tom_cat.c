#include "include.h"

#if FUNC_TOM_CAT_EN

func_tom_t f_tom;

//0:按键录音  1:声控录音
AT(.text.bsp.tom_cat)
void tom_cat_switch_cycfs_init(u8 rec_type)
{
    u32 cur;
    if (rec_type == 0) {
        param_spiflash_loop0_read((u8*)&cur);
        if (cur > TOM_CAT_KEY_REC_END_ADDR || cur < TOM_CAT_KEY_REC_START_ADDR) {
            cur = TOM_CAT_KEY_REC_START_ADDR;
        }
        os_spiflash_loop_init(TOM_CAT_KEY_REC_START_ADDR, TOM_CAT_KEY_REC_END_ADDR, cur);
        cycrec_fs_init(TOM_CAT_KEY_REC_START_ADDR, TOM_CAT_KEY_REC_END_ADDR, (void *)os_spiflash_read_loop, (void *)os_spiflash_program_loop, (void *)os_spiflash_erase_loop);
    } else if (rec_type == 1) {
        param_spiflash_loop1_read((u8*)&cur);
        if (cur > TOM_CAT_SPK_REC_END_ADDR || cur < TOM_CAT_SPK_REC_START_ADDR) {
            cur = TOM_CAT_SPK_REC_START_ADDR;
        }
        os_spiflash_loop_init(TOM_CAT_SPK_REC_START_ADDR, TOM_CAT_SPK_REC_END_ADDR, cur);
        cycrec_fs_init(TOM_CAT_SPK_REC_START_ADDR, TOM_CAT_SPK_REC_END_ADDR, (void *)os_spiflash_read_loop, (void *)os_spiflash_program_loop, (void *)os_spiflash_erase_loop);
    }
}


AT(.text.bsp.tom_cat)
void tom_rec_start(void)
{
    f_tom.rec_time_ms = 0;
    f_tom.rec_en = 1;
}

AT(.text.bsp.tom_cat)
void tom_rec_stop(void)
{
    f_tom.rec_en = 0;
}

AT(.text.bsp.tom_cat)
void tom_rec_init(void)
{
    rec_src.spr = SPR_16000;
    rec_src.nchannel = 1;
    rec_src.source_start = tom_rec_start;
    rec_src.source_stop  = tom_rec_stop;
    f_tom.rec_en = 0;
}

AT(.text.bsp.tom_cat)
void func_tom_cat_stop(void)
{
    audio_path_exit(AUDIO_PATH_TOM_CAT);
}

AT(.text.bsp.tom_cat)
void func_tom_cat_start(void)
{
    audio_path_init(AUDIO_PATH_TOM_CAT);
    audio_path_start(AUDIO_PATH_TOM_CAT);
}

AT(.text.bsp.tom_cat)
void func_tom_cat_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }
    sfunc_record_pause();
    func_tom_cat_stop();
    mp3_res_play(addr, len);
    func_tom_cat_start();
    sfunc_record_continue();
}

AT(.com_text.func.tom_cat)
void tom_cat_sdadc_process(u8 *ptr, u32 samples, int ch_mode)
{
    if (f_tom.rec_en) {
        puts_rec_obuf(ptr, (u16)(samples << (1 + ch_mode)));
    }
    sdadc_pcm_2_dac(ptr, samples, ch_mode);
}

AT(.text.func.tom_cat)
void func_tom_cat_setvol_callback(u8 dir)
{
}

AT(.text.func.tom_cat)
void func_tom_cat_process(void)
{
    static u32 tick = 0;
    u8 ret = tom_dnr_process();
    if (tick_check_expire(tick,10)) {
        tick = tick_get();
        if (ret == 1) {         //mic in
            f_tom.rec_type = 1;
            msg_enqueue(KU_REC);
        } else if (ret == 2) {  //mic out

        }
    }

    func_process();
}

static void func_tom_cat_enter(void)
{
    memset(&f_tom, 0, sizeof(f_tom));
    func_cb.mp3_res_play = func_tom_cat_mp3_res_play;
    func_cb.set_vol_callback = func_tom_cat_setvol_callback;
    tom_rec_init();
    aux_dnr_init(10, 30000, 300, 2000);     //有声检测次数，声音阈值，无声检测次数，无声阈值
    func_tom_cat_start();
}

static void func_tom_cat_exit(void)
{
    sfunc_record_stop();
    func_tom_cat_stop();
    func_cb.last = FUNC_TOM_CAT;
}

AT(.text.func.tom_cat)
void func_tom_cat(void)
{
    printf("%s\n", __func__);

    func_tom_cat_enter();

    while (func_cb.sta == FUNC_TOM_CAT) {
        func_tom_cat_process();
        func_tom_cat_message(msg_dequeue());
        func_tom_cat_display();
    }

    func_tom_cat_exit();
}

#endif // FUNC_TOM_CAT_EN

