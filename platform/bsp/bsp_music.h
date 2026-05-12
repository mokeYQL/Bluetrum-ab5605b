#ifndef _BSP_MUSIC_H
#define _BSP_MUSIC_H

enum {
    NORMAL_MODE,
    SINGLE_MODE,
    FLODER_MODE,
    RANDOM_MODE,
};

bool pf_scan_music(u8 new_dev);
void music_playmode_next(void);
void mp3_res_play(u32 addr, u32 len);
void wav_res_play(u32 addr, u32 len);
void wav_res_src1_mix_play(u32 addr, u32 len);
void kmix_play_wav(u32 addr, u32 len);

void wav_src1_mix_5ms_isr(void);

#if MUSIC_BREAKPOINT_EN
void bsp_music_breakpoint_init(void);
void bsp_music_breakpoint_save(void);
void bsp_music_breakpoint_clr(void);

#define music_breakpoint_init()   bsp_music_breakpoint_init()
#define music_breakpoint_save()   bsp_music_breakpoint_save()
#define music_breakpoint_clr()    bsp_music_breakpoint_clr()
#else
#define music_breakpoint_init()
#define music_breakpoint_save()
#define music_breakpoint_clr()
#endif // MUSIC_BREAKPOINT_EN

#endif //_BSP_MUSIC_H
