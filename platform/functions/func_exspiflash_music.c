#include "include.h"
#include "func.h"
#include "func_music.h"
#include "func_exspiflash_music.h"
#include "api_spiflash_ex.h"

uint os_spiflash_read(void *buf, u32 addr, uint len);
void os_spiflash_program(void *buf, u32 addr, uint len);
void os_spiflash_erase(u32 addr);

#if (EX_SPIFLASH_SUPPORT)
exspifalsh_msc_t exspi_msc AT(.buf.func.music);

u32 spi_mp3_offset_ending = 512*0;      //开头偏移多少byte
u32 spi_mp3_offset_beginning = 0;       //结尾偏移多少byte
bool spi_mp3_loopback_en(void)
{
    return SPLFLASH_MP3_LOOPBACK_EN;
}
void spi_mp3_loopback_start(void)
{
    //static u32 spi_loop_cnt = 0;
    //printf("loop times: %d\n", ++spi_loop_cnt);
}

bool exspiflash_init(void)
{
    printf("exspiflash_init\n");
#if !INTERNAL_FLASH_REC
    spiflash1_init();
    if (!is_exspiflash_online()) {
        return false;
    }
#endif
#if  SPIFALSH_MUSIC_BIN_WRITE_TEST    //默认的FLASH_MUSIC.BIN写入SPIFLASH, 可以用于测试外接SPIFALSH播放MP3.
    write_music_bin_to_spiflash();
#endif

#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)

#if INTERNAL_FLASH_REC
    #if FUNC_TOM_CAT_EN
    tom_cat_switch_cycfs_init(1);
    #else
    cycrec_fs_init(SPIFLASH_REC_BEGIN_ADDR, SPIFLASH_REC_END_ADDR, (void *)os_spiflash_read, (void *)os_spiflash_program, (void *)os_spiflash_erase);
    #endif // FUNC_TOM_CAT_EN
#else
    cycrec_fs_init(SPIFLASH_REC_BEGIN_ADDR, SPIFLASH_REC_END_ADDR, spiflash1_read, spiflash1_write, spiflash1_erase);
#endif // INTERNAL_FLASH_REC

#endif // (EX_SPIFLASH_SUPPORT & EXSPI_REC)
    return true;
}

AT(.text.func.music)
void func_exspifalsh_music_process(void)
{
    func_process();
    if (get_music_dec_sta() == MUSIC_STOP)  {
#if SPIFALSH_MUSIC_PLAY_REC
    if (!exspi_msc.rec_no_file) {
        exspifalsh_rec_switch_file(1);
    }
#else
    #if SPIFLASH_MUSIC_BREAK_LOOP
        exspifalsh_music_switch_file(1);
    #else
        exspifalsh_music_num_kick(exspi_msc.cur_num);
    #endif
#endif
    }
}

AT(.text.func.music)
void func_exspifalsh_music_display(void)
{

}

AT(.text.func.music)
void exspifalsh_music_switch_file(u8 direction)   //1 next , 0 prev
{
    music_control(MUSIC_MSG_STOP);
    printf("direction = %d, music_num = %d, total = %d\n",direction, exspi_msc.cur_num,exspi_msc.total_num);
    if (direction) {
        if (++exspi_msc.cur_num > exspi_msc.total_num) {
            exspi_msc.cur_num = 1;
        }
    } else {
        if (exspi_msc.cur_num >= 1) {
            exspi_msc.cur_num--;
        }
        if (exspi_msc.cur_num == 0) {
            exspi_msc.cur_num = exspi_msc.total_num;
        }
    }
    printf("music_num = %d, total = %d\n",exspi_msc.cur_num,exspi_msc.total_num);
    exspifalsh_music_num_kick(exspi_msc.cur_num);
}


#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)
AT(.text.func.music)
void exspifalsh_rec_switch_file(u8 direction)   //1 next , 0 prev
{
    u32 addr = 0, len = 0;
    u8 ret = 1;
    printf("rec next file, direction = %d\n",direction);
    music_control(MUSIC_MSG_STOP);
    if (direction) {
        ret = spifalsh_rec_open_next_file(&addr, &len);
    } else {
        ret = spifalsh_rec_open_prev_file(&addr, &len);
    }
    exspi_msc.rec_no_file = ret;
    printf("open file ret = %d, addr = 0x%X, len = %d\n",ret, addr, len);
    if (!ret) {
         mp3_res_play_kick(addr, len);
    }
}

//播放外接spifalsh中的最后一个录音
bool spifalsh_rec_play_last_file(void)
{
    u32 addr, len;
    if (spifalsh_rec_open_last_file(&addr,&len)) {  //打开最后一个录音文件.
        return false;
    }
    if (func_cb.mp3_res_play != NULL) {
        printf("spifalsh_rec_play_last_file : addr = %d, len = %d\n",addr,len);
#if INTERNAL_FLASH_REC
    #if FUNC_TOM_CAT_EN
        register_spi_read_function((void *)os_spiflash_read_loop);
    #else
        register_spi_read_function((void *)os_spiflash_read);
    #endif
#else
        register_spi_read_function(spiflash1_read);
#endif
#if EXSPI_REC_PITCH_SHIFT_PLAY
        lock_code_pitch();
        pitch_shift2_init(44100, -16000);  //SPEAKER采样率为16K
        mp3_pitch_shift_play_set(true);
#endif
        func_mp3_res_play(addr, len);
#if EXSPI_REC_PITCH_SHIFT_PLAY
        mp3_pitch_shift_play_set(false);
        unlock_code_pitch();

#endif
        if (func_cb.sta != FUNC_EXSPIFLASH_MUSIC) {
            register_spi_read_function(NULL);
        }
    }
    return true;
}
#endif


AT(.text.func.music)
bool exspifalsh_music_image_check(void)
{
    if (is_exspiflash_online()) {
        exspi_msc.total_num = exspifalsh_music_get_file_total();
        printf("exspi_msc.total_num = %d\n",exspi_msc.total_num);
        if (exspi_msc.total_num > 0) {
            return true;
        }
    }
    return false;
}

AT(.text.func.music)
void func_exspifalsh_mp3_res_play(u32 addr, u32 len)
{
    msc_breakpiont_t brkpt;
    music_get_breakpiont(&brkpt);           //保存当前播放位置
    u32 cur_time = music_get_cur_time();
    music_control(MUSIC_MSG_STOP);
    register_spi_read_function(NULL);       //恢复内部SPI读接口.
    mp3_res_play(addr, len);
    register_spi_read_function(spiflash1_read);
    music_set_cur_time(cur_time);
    exspifalsh_music_num_kick(exspi_msc.cur_num);
    music_set_jump(&brkpt);                 //恢复播放位置
}

AT(.text.func.music)
static void func_exspifalsh_music_enter(void)
{
    msg_queue_clear();
#if SPIFALSH_MUSIC_PLAY_REC
    u32 addr, len;
    u32 rec_total = spifalsh_rec_get_total_file();
    printf("func_exspifalsh_music_enter, rec_total = %d\n",rec_total);
    func_cb.mp3_res_play = func_exspifalsh_mp3_res_play;
    register_spi_read_function(spiflash1_read);
    if ((!rec_total) ||spifalsh_rec_open_last_file(&addr,&len)) {  //打开最后一个录音文件.
        func_cb.sta = FUNC_NULL;
        return;
    }
    bsp_change_volume(sys_cb.vol);
    exspi_msc.pause = false;
    mp3_res_play_kick(addr, len);
#else
    if ((!exspifalsh_music_image_check())){
        func_cb.sta = FUNC_NULL;
        return;
    }
    func_cb.mp3_res_play = func_exspifalsh_mp3_res_play;
    register_spi_read_function(spiflash1_read);
    bsp_change_volume(sys_cb.vol);
    exspi_msc.cur_num = 1;            //默认从1首开始播放
    exspi_msc.pause = false;

#if SPIFALSH_MUSIC_BREAK_MEMORY
    param_spiflash_breakpoint_read();             //进入flash_music模式，恢复到上次退出时的播放位置 ，
    if(0 == exspi_msc.save_num){
        exspi_msc.cur_num = 1;
    }
    else{
        exspi_msc.cur_num = exspi_msc.save_num;
    }
    music_set_cur_time(exspi_msc.save_time);
    exspifalsh_music_num_kick(3);
    music_set_jump(&(exspi_msc.brkpt));
#else
    exspifalsh_music_num_kick(exspi_msc.cur_num);
#endif // SPIFALSH_MUSIC_BREAK_MEMORY

#endif // SPIFALSH_MUSIC_PLAY_REC
}

AT(.text.func.music)
static void func_exspifalsh_music_exit(void)
{
    music_control(MUSIC_MSG_STOP);
    register_spi_read_function(NULL);  //恢复内部SPI读接口.
    func_cb.last = FUNC_MUSIC;
}

AT(.text.func.music)
void non_volatile_memory(void)
{
    #if SPIFALSH_MUSIC_BREAK_MEMORY
    static u32 ticks = 0;
    u32 curFileLen = 0;
    u32 ptrTemp = 0;
    if(tick_check_expire(ticks,100)){      //100ms记忆一次
        ticks = tick_get();
        music_get_breakpiont(&(exspi_msc.brkpt));

        curFileLen = exspiflash_music_get_cur_file_len();
        if(exspi_msc.brkpt.file_ptr >= curFileLen)
        {
            ptrTemp = (exspi_msc.brkpt.file_ptr % curFileLen);
            exspi_msc.brkpt.file_ptr = ptrTemp;
        }

        exspi_msc.save_time = music_get_cur_time();
        exspi_msc.save_num = exspi_msc.cur_num;
        param_spiflash_breakpoint_write();
        param_sync();
    }
    #endif // SPIFALSH_MUSIC_BREAK_MEMORY
}

AT(.text.func.music)
void func_exspifalsh_music(void)
{
    printf("%s\n", __func__);
    func_exspifalsh_music_enter();
    while (func_cb.sta == FUNC_EXSPIFLASH_MUSIC) {
        func_exspifalsh_music_process();
        func_exspifalsh_music_message(msg_dequeue());
        func_exspifalsh_music_display();
    }
    func_exspifalsh_music_exit();
}
#endif

