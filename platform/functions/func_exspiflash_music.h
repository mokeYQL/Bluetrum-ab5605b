#ifndef FUNC_EXSPIFALSH_MUSIC_H
#define FUNC_EXSPIFALSH_MUSIC_H

#include "bsp_spiflash1.h"
#include "api_spiflash_ex.h"

typedef struct {
    u16 total_num;
    u16 cur_num;
    u8  pause;
    u8  rec_no_file;
    #if SPIFALSH_MUSIC_BREAK_MEMORY
    msc_breakpiont_t brkpt;//用于保存和恢复播放位置的结构体
    u16 save_time;//用于退出flash模式时保存的时间
    u16 save_num;//用于退出时保存当前播放歌曲的序号
    #endif // SPIFALSH_MUSIC_BREAK_MEMORY
} exspifalsh_msc_t;
extern exspifalsh_msc_t exspi_msc;


void func_exspiflash_music_message(u16 msg);
void register_spi_read_function(int (* read_func)(void *buf, u32 addr, u32 len));
void func_exspifalsh_music_message(u16 msg);
void exspifalsh_music_switch_file(u8 direction);
void exspifalsh_rec_switch_file(u8 direction);
extern void mp3_res_play_kick(u32 addr, u32 len);
void non_volatile_memory(void);


#endif


