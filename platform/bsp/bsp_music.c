#include "include.h"

#define FS_CRC_SEED         0xffff

uint calc_crc(void *buf, uint len, uint seed);
void mp3_res_play_kick(u32 addr, u32 len);
void mp3_res_switch_do(u32 addr, u32 len);
void wav_res_play_kick(u32 addr, u32 len);
void wav_res_dec_process(void);
bool wav_res_is_play(void);
bool wav_res_stop(void);
void mp3_res_play_exit(void);

#if FUNC_MUSIC_EN

uint8_t wma_lock_ram_en = BT_BACKSTAGE_EN;

#if BT_BACKSTAGE_EN
const u32 *res_addr[] = {
#if WARNING_FUNC_MUSIC
    &RES_BUF_MUSIC_MODE_MP3,
#endif
#if WARNING_USB_SD
    &RES_BUF_SDCARD_MODE_MP3,
    &RES_BUF_USB_MODE_MP3,
#endif
#if WARNING_FUNC_BT
    &RES_BUF_BT_MODE_MP3,
#endif
#if WARNING_FUNC_FMRX
    &RES_BUF_FM_MODE_MP3,
#endif
#if WARNING_FUNC_AUX
    &RES_BUF_AUX_MODE_MP3,
#endif
};
#endif

//扫描全盘文件
bool pf_scan_music(u8 new_dev)
{
    if (new_dev) {
#if UART_M_UPDATE
        func_uart_update();
#elif USB_SD_UPDATE_EN
        func_update();                                  //尝试升级
#endif // USB_SD_UPDATE_EN
    }

#if REC_FAST_PLAY
    f_msc.rec_scan = BIT(0);
    sys_cb.rec_num = 0;
    sys_cb.ftime = 0;
#endif // REC_FAST_PLAY

    f_msc.file_total = fs_get_total_files();
    if (!f_msc.file_total) {
        f_msc.dir_total = 0;
        return false;
    }

#if REC_FAST_PLAY
    f_msc.rec_scan = 0;
#endif // REC_FAST_PLAY

    if (new_dev) {
#if MUSIC_PLAYDEV_BOX_EN
        gui_box_show_playdev();
#endif // MUSIC_PLAYDEV_BOX_EN

#if WARNING_USB_SD
        if (sys_cb.cur_dev <= DEV_SDCARD1) {
            mp3_res_play(RES_BUF_SDCARD_MODE_MP3, RES_LEN_SDCARD_MODE_MP3);
        } else if(sys_cb.cur_dev <= DEV_UDISK){
            mp3_res_play(RES_BUF_USB_MODE_MP3, RES_LEN_USB_MODE_MP3);
        }
#endif // WARNING_USB_SD
    }

    f_msc.dir_total = fs_get_dirs_count();          //获取文件夹总数
    return true;
}

#if MUSIC_PLAYMODE_NUM
//music播放模式切换
void music_playmode_next(void)
{
    sys_cb.play_mode++;
    if (sys_cb.play_mode >= MUSIC_PLAYMODE_NUM) {
        sys_cb.play_mode = NORMAL_MODE;
    }
    gui_box_show_playmode();
    plugin_playmode_warning();
}
#endif // MUSIC_PLAYMODE_NUM

#if MUSIC_BREAKPOINT_EN
void bsp_music_breakpoint_clr(void)
{
    f_msc.brkpt.file_ptr = 0;
    f_msc.brkpt.frame_count = 0;
    param_msc_breakpoint_write();
    param_sync();
    //printf("%s\n", __func__);
}

void bsp_music_breakpoint_init(void)
{
    int clr_flag = 0;
    if (f_msc.brkpt_flag) {
        f_msc.brkpt_flag = 0;
        param_msc_breakpoint_read();
        //printf("restore: %d, %d, %04x\n", f_msc.brkpt.file_ptr, f_msc.brkpt.frame_count, f_msc.brkpt.fname_crc);
        if (calc_crc(f_msc.fname, 8, FS_CRC_SEED) == f_msc.brkpt.fname_crc) {
            music_set_jump(&f_msc.brkpt);
        } else {
            clr_flag = 1;
        }
    }
    f_msc.brkpt.fname_crc = calc_crc(f_msc.fname, 8, FS_CRC_SEED);
    f_msc.brkpt.file_ptr = 0;
    f_msc.brkpt.frame_count = 0;
    if (clr_flag) {
        param_msc_breakpoint_write();
    }
}

void bsp_music_breakpoint_save(void)
{
    music_get_breakpiont(&f_msc.brkpt);
    param_msc_breakpoint_write();
    param_sync();
    //printf("save: %d, %d, %04x\n", f_msc.brkpt.file_ptr, f_msc.brkpt.frame_count, f_msc.brkpt.fname_crc);
}
#endif // MUSIC_BREAKPOINT_EN

AT(.wavdec.code)
u32 wav_dec_sample_rate_limit(void)
{// 设置WAV支持的最高采样率，U盘时最高96k
    return 96000;
}
#endif // FUNC_MUSIC_EN

///提示音播放无缝切换
bool mp3_res_msg_switch(u16 msg)
{
//    if (msg == KU_PREV) {
//        mp3_res_switch_do(RES_BUF_EN_BT_MODE_MP3, RES_LEN_EN_BT_MODE_MP3);
//        return true;
//    } else if (msg == KU_NEXT) {
//        mp3_res_switch_do(RES_BUF_EN_CLOCK_MODE_MP3, RES_LEN_EN_CLOCK_MODE_MP3);
//        return true;
//    }
    return false;
}

void mp3_res_play_do(u32 addr, u32 len, bool sync)
{
    u16 msg;
    u8 mute_bak;
//    printf("%s: addr: %x, len: %x\n", __func__, addr, len);
    if (len == 0) {
        return;
    }

#if BT_BACKSTAGE_EN
    if (func_cb.sta_break != FUNC_NULL) {
        for (u32 i = 0; i < sizeof(res_addr) / sizeof(u32); i++) {
            if (*res_addr[i] == addr) {
                if (func_cb.sta != FUNC_BT) {
                    func_cb.sta_break = FUNC_NULL;
                }
                return;
            }
        }
    }
#endif

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if EXLIB_DRC_EN || EXLIB_SOFT_EQ_EN
    u8 pcm_ex_back = cfg_dac_pcm_ext_en;
    cfg_dac_pcm_ext_en = 0;
#endif

#if SYS_KARAOK_EN
    u8 voice_bak = 0, func_sta = func_cb.sta;
    if (karaok_get_voice_rm_sta()) {
        voice_bak = 1;
        karaok_voice_rm_disable();
    }
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif

#if EXLIB_BT_MONO_XDRC_EN
    sys_cb.xdrc_sysvol_en = is_xdrc_sysvol_en();
    xdrc_sysvol_en_set(0);
#endif

    mute_bak = sys_cb.mute;
    if (sys_cb.mute) {
        bsp_loudspeaker_unmute();
    }
    if(get_music_dec_sta() != MUSIC_STOP) { //避免来电响铃/报号未完成，影响get_music_dec_sta()状态
        music_control(MUSIC_MSG_STOP);
    }
    bsp_change_volume(WARNING_VOLUME);
    if (music_set_eq_is_done()) {
        music_set_eq_by_num(0); // EQ 设置为 normal
    }

    if(BT_TWS_EN && sync) {
        tws_mp3_res_play_kick(addr, len);
    } else {
        mp3_res_play_kick(addr, len);
    }

    while (get_music_dec_sta() != MUSIC_STOP) {
        WDT_CLR();
        msg = msg_dequeue();
        if (sys_cb.voice_evt_brk_en) {
            if (((msg == EVT_SD_INSERT) || (msg == EVT_UDISK_INSERT)) && (func_cb.sta != FUNC_MUSIC)) {
                func_message(msg);
                break;
            }
        }
		if ((PWRKEY_2_HW_PWRON) && (sys_cb.pwrdwn_hw_flag)) {//PWRKEY模拟硬件开关机快速关机
            break;
		}
		if (mp3_res_msg_switch(msg)) {
            continue;
		}
#if FUNC_HDMI_EN
        cec_frame_process();
#endif

#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)
        if ((msg == KU_REC) || (msg == KL_REC)) {
            break;
        }
#endif

#if LINEIN_DETECT_EN
        if ((msg == EVT_LINEIN_INSERT) && ((sys_cb.voice_evt_brk_en) || (LINEIN_2_PWRDOWN_EN))) {
            func_message(msg);
            break;
        }
#endif // LINEIN_DETECT_EN
        if ((msg != NO_MSG) && ((msg & KEY_TYPE_MASK) != KEY_HOLD)) {
            msg_enqueue(msg);       //还原未处理的消息
        }

        if(sys_cb.tws_res_brk) {
            sys_cb.tws_res_brk = 0;
            break;
        }
    }
    music_control(MUSIC_MSG_STOP);
	if (sys_cb.maxvol_fade) {
		msg_enqueue(KLU_NEXT_VOL_UP);     //防止music下播放最大提示音.mp3格式,DAC无声。
	}
#if EXLIB_BT_MONO_XDRC_EN
    xdrc_sysvol_en_set(sys_cb.xdrc_sysvol_en);
#endif
    bsp_change_volume(sys_cb.vol);

    if (music_set_eq_is_done()) {
        music_set_eq_by_num(sys_cb.eq_mode); // 恢复 EQ
    }
    mp3_res_play_exit();
    sys_cb.mute = mute_bak;
    if (sys_cb.mute) {
        bsp_loudspeaker_mute();
    }
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif

#if EXLIB_DRC_EN || EXLIB_SOFT_EQ_EN
    cfg_dac_pcm_ext_en = pcm_ex_back;
#endif

#if SYS_KARAOK_EN
    if (voice_bak) {
        karaok_voice_rm_enable();
    }
    bsp_karaok_init(AUDIO_PATH_KARAOK, func_sta);
#endif

}

#if WARNING_WAVRES_PLAY

void wav_res_play_do(u32 addr, u32 len, bool sync)
{
    if (len == 0) {
        return;
    }

#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

#if FUNC_AUX_EN
    bsp_aux_piano_start();       //AUX直通时播放WAV提示音需要特殊处理下
#endif

    wav_res_play_kick(addr, len);
    while (wav_res_is_play()) {
        wav_res_dec_process();
        WDT_CLR();
    }
    wav_res_stop();

#if FUNC_AUX_EN
    bsp_aux_piano_exit();
#endif
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
}

void wav_res_play(u32 addr, u32 len)
{
    wav_res_play_do(addr, len, 0);
}

#endif

void tws_res_get_addr(u32 index, u32 *addr, u32 *len) {
    switch(index) {
//    case TWS_RES_NUM_0:
//        *addr = RES_BUF_NUM_0_MP3;
//        *len = RES_LEN_NUM_0_MP3;
//        break;
//    case TWS_RES_NUM_1:
//        *addr = RES_BUF_NUM_1_MP3;
//        *len = RES_LEN_NUM_1_MP3;
//        break;
//    case TWS_RES_NUM_2:
//        *addr = RES_BUF_NUM_2_MP3;
//        *len = RES_LEN_NUM_2_MP3;
//        break;
//    case TWS_RES_NUM_3:
//        *addr = RES_BUF_NUM_3_MP3;
//        *len = RES_LEN_NUM_3_MP3;
//        break;
//    case TWS_RES_NUM_4:
//        *addr = RES_BUF_NUM_4_MP3;
//        *len = RES_LEN_NUM_4_MP3;
//        break;
//    case TWS_RES_NUM_5:
//        *addr = RES_BUF_NUM_5_MP3;
//        *len = RES_LEN_NUM_5_MP3;
//        break;
//    case TWS_RES_NUM_6:
//        *addr = RES_BUF_NUM_6_MP3;
//        *len = RES_LEN_NUM_6_MP3;
//        break;
//    case TWS_RES_NUM_7:
//        *addr = RES_BUF_NUM_7_MP3;
//        *len = RES_LEN_NUM_7_MP3;
//        break;
//    case TWS_RES_NUM_8:
//        *addr = RES_BUF_NUM_8_MP3;
//        *len = RES_LEN_NUM_8_MP3;
//        break;
//    case TWS_RES_NUM_9:
//        *addr = RES_BUF_NUM_9_MP3;
//        *len = RES_LEN_NUM_9_MP3;
//        break;
//    case TWS_RES_RING:
//        *addr = RES_BUF_RING_MP3;
//        *len = RES_LEN_RING_MP3;
//        break;
#if (LANG_SELECT == LANG_EN_ZH)
    case TWS_RES_LANGUAGE_ZH:
        *addr = RES_BFUF_ZH_LANGUAGE_MP3;
        *len = RES_LEN_ZH_LANGUAGE_MP3;
        break;
    case TWS_RES_LANGUAGE_EN:
        *addr = RES_BUF_EN_LANGUAGE_MP3;
        *len = RES_LEN_EN_LANGUAGE_MP3;
        break;
#endif
//    case TWS_RES_LOW_BATTERY:
//        break;
#if WARNING_BT_CONNECT
    case TWS_RES_CONNECTED:
        *addr = RES_BUF_CONNECTED_MP3;
        *len = RES_LEN_CONNECTED_MP3;
        break;
#endif
#if WARNING_BT_TWS_CONNECT
    case TWS_RES_TWS_CONNECTED:
        *addr = RES_BUF_TWS_CONNECTED_MP3;
        *len = RES_LEN_TWS_CONNECTED_MP3;
        break;
#endif
//    case TWS_RES_DISCONNECT:
//        *addr = RES_BUF_DISCONNECT_MP3;
//        *len = RES_LEN_DISCONNECT_MP3;
//        break;
#if WARNING_MAX_VOLUME
    case TWS_RES_MAX_VOL:
        *addr = RES_BUF_MAX_VOL_MP3;
        *len = RES_LEN_MAX_VOL_MP3;
        break;
#endif
    case TWS_PIANO_MAX_VOL:
        *addr = (u32)bsp_piano_get_play_res(WARNING_TONE, TONE_MAX_VOL);
        *len  = WARNING_TONE;
        break;
    default :
        *addr = 0;
        *len = 0;
        break;
    }
}

void tws_res_proc(void)
{
    u32 addr, len;
    u32 res = tws_res_sync(300);
    if(res == -1L) {
        return;
    }
    sys_cb.tws_res_brk = 0;
    tws_res_get_addr(res, &addr, &len);
    if (len) {
        if (res == TWS_PIANO_MAX_VOL) {
            piano_warning_play(len, (void *)addr, 1);
        } else {
            bt_audio_bypass();
            mp3_res_play_do(addr, len, 1);
            bt_audio_enable();
        }
    } else {
        tws_res_done();
    }
}

#if AUDIO_STRETCH_EN
int dac_music_speed_change_en(bool flag);
void stretch_voice_set_level(u8 type, u8 level);
#endif

void mp3_res_play(u32 addr, u32 len)
{
#if AUDIO_STRETCH_EN
	//stretch_voice_set_level(0,0);	// 非USB SD卡模式下更换模式使禁用变速
	if(addr == RES_BUF_SDCARD_MODE_MP3 || addr == RES_BUF_USB_MODE_MP3){
		dac_music_speed_change_en(1);
		stretch_voice_set_level(0,0);
	}else{
		dac_music_speed_change_en(0);
	}


#endif
    mp3_res_play_do(addr, len, 0);
}

//AT(.mp3dec.frame)
//u8 set_jump_cnt(void)               //部分音乐本身有错误，需要在这里选择跳过帧数，默认20
//{
//    return 20;
//}
#if EXFLASH_MUSIC_WARNING
#define FILE_HEAD_LEN       22          //wav文件需要跳过头数据(单位：点--字节*2)

volatile bool wav_output_en = false;
void obuf1_put_one_sample_w(u32 left, u32 right);
void dac_src1_init(void);
void dac_src1_spr_set(uint spr);

enum {
    WAV_SPR_8000  = 40,
    WAV_SPR_16000 = 80,
    WAV_SPR_24000 = 120,
    WAV_SPR_32000 = 160,
    WAV_SPR_48000 = 240,
};

AT(.text.res.wav)
void wav_output_init(void)              //调用一次初始化，播放一次提示音
{
    if (!RES_BUF_MAX_VOL_WAV || !RES_LEN_MAX_VOL_WAV) {
        return;
    }
    dac_src1_init();
    dac_src1_spr_set(SPR_16000);        //16K采样率
    SRC1VOLCON  = (0x7e00 | (0x02 << 16));
    SRC1VOLCON |= BIT(20);
    DACVOLCON   = 0x7e00 | BIT(18);
}

AT(.text.res.wav)
void wav_output_play(void)
{
    if (wav_output_en) {
        return;
    }
#if SYS_KARAOK_EN
    dac_fade_out();
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
    wav_output_init();
    wav_output_en = true;
    dac_fade_in();
}

AT(.text.wav.flash)
void wav_output_process(void)           //kick 之后，5ms调一次
{
    s16* wav_ptr = (s16 *)RES_BUF_MAX_VOL_WAV;
    u16 samples = WAV_SPR_16000;
    static u16 write_len = FILE_HEAD_LEN;
    static u8 last_packet = 0;
    u8 i = 0;
    if (last_packet) {                  //最后一包数据
        samples = last_packet;
    }
    if (wav_output_en) {
        wav_ptr += write_len;
        while(samples--) {
            obuf1_put_one_sample_w(wav_ptr[i], wav_ptr[i]);
            i++;
        }
        if (last_packet) {
            write_len += last_packet;
        } else {
            write_len += WAV_SPR_16000;
        }
        if ((last_packet == 0) && (((RES_LEN_MAX_VOL_WAV/2) - write_len) < WAV_SPR_16000)) {//最后的数据不足一包
            last_packet = ((RES_LEN_MAX_VOL_WAV/2) - write_len);
        }
        if (write_len >= RES_LEN_MAX_VOL_WAV/2) {
            my_printf("write_len:%d, %d\n", write_len, RES_LEN_MAX_VOL_WAV);
            write_len = FILE_HEAD_LEN;
            wav_output_en = false;
            last_packet = 0;
            dac_fade_out();
#if SYS_KARAOK_EN
            bsp_karaok_init(AUDIO_PATH_KARAOK, func_cb.sta);
#endif
        }
    }
}

#endif


//---------------------------------------------------------------------
#if WAV_SRC1_MIX_PLAY_EN
//WAV(PCM,16K,单声)推SRC1 混合正常音乐SRC0播放 //不支持同时打开KARAOK(KARAOK MIC也会用到SRC1)
//使用方法,在需要的地方直接调用如 wav_res_src1_mix_play(RES_BUF_MAX_VOL_WAV,RES_LEN_MAX_VOL_WAV); //WAV(16K采样率,单声道)文件放到res目录下编译,在生成的res.h中会得到该函数的两个参数
enum {
    WAV_MIX_STOP = 0,
    WAV_MIX_RUN = 1,
};

typedef struct _WAV_MIX_T {
    u16 *buf;
    u16 samples_total;
    u16 samples_idx;
    volatile u8 sta;
    u8 res;
} WAV_MIX_T;
WAV_MIX_T  wavmix;
AT(.com_text.wavmix)
void wav_src1_mix_5ms_isr(void)
{
    if (WAV_MIX_RUN == wavmix.sta) {
        wav_output_tick_start();   //发消息给au线程,au线程会回调wav_output_process //相当于把推WAV声音放到au线程中处理
    }
}

AT(.text.wavmix)
void wav_output_process(void)
{
    while(WAV_MIX_RUN == wavmix.sta) {
        if ((AUBUF1CON & BIT(8)) == 0x00) {   //AUBUF1 NOT FULL
            u16 pcm = wavmix.buf[wavmix.samples_idx];
            AUBUF1DATA = ((u32)pcm << 16) | pcm;
            wavmix.samples_idx++;
            if ( wavmix.samples_idx >= wavmix.samples_total) {
                wavmix.sta = WAV_MIX_STOP;
            }
        } else {  //AUBUF1 FULL
            break;
        }
    }
}

AT(.text.wavmix)
void wav_res_src1_mix_play(u32 addr, u32 len)
{
    u16 pcm_len = (u16)len;
    printf("pcm_len = %d\n",pcm_len);
    if (pcm_len < 44) {
        return;
    }
    wavmix.sta = WAV_MIX_STOP;
    //SRC1 init
    dac_src1_init();
    dac_src1_spr_set(SPR_16000);        //16K采样率
    SRC1VOLCON  = (SRC1VOLCON & ~0xFFFF) | 0x7e00;  //低16位是SRC1音量(WAV音量),最大值为0x7FFF
    SRC1VOLCON |= BIT(20);
//    SRC0VOLCON  = (SRC0VOLCON & ~0xFFFF) | 0x100;  //低16位是SRC0音量(正常音乐音量),最大值为0x7FFF
//    SRC0VOLCON |= BIT(20);
    DACVOLCON =  (DACVOLCON & ~0xFFFF) | 0x7e00;  //低16位是SRC0/SRC1混合后经过的总数字音量,最大值为0x7FFF
    DACVOLCON |= BIT(20);

    dac_fade_in();
    //wav pcm control
    pcm_len = pcm_len - 44;    //wav 有44byte 头信息
    wavmix.buf = (u16*)(addr+44);
    wavmix.samples_total = (pcm_len-256)/2;   //资源文件中的数据以256BYTE对齐,减去最后256BYTE,防止取到无效的对齐数据
    wavmix.samples_idx = 0;
    wavmix.sta  = WAV_MIX_RUN;
}

#endif  //WAV_SRC1_MIX_PLAY_EN






















