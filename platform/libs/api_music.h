#ifndef _API_MUSIC_H
#define _API_MUSIC_H

enum {
    MUSIC_STOP = 0,
    MUSIC_PAUSE,
    MUSIC_PLAYING,
};

enum {
    NONE_TYPE = 0,
    WAV_TYPE,
    MP3_TYPE,
    WMA_TYPE,
    APE_TYPE,
    FLAC_TYPE,
};

enum {
    //decode msg
    MUSIC_MSG_STOP = 0,
    MUSIC_MSG_PAUSE,
    MUSIC_MSG_PLAY,

    //encode msg
    ENC_MSG_MP3 = 64,
    ENC_MSG_AEC,
    ENC_MSG_ALC,
    ENC_MSG_PLC,
    ENC_MSG_SBC,
    ENC_MSG_WAV,
    ENC_MSG_ADPCM,
};

enum {
    QSKIP_BACKWARD,
    QSKIP_FORWARD,
};

extern unsigned char avio_buf[556];

void music_control(u8 msg);
u8 get_music_dec_sta(void);
int music_decode_init(void);
int wav_decode_init(void);
int mp3_decode_init(void);
int wma_decode_init(void);
int ape_decode_init(void);
int flac_decode_init(void);
int sbcio_decode_init(void);
void sbc_decode_init(void);
u16 music_get_total_time(void);
u16 music_get_cur_time(void);
void music_set_cur_time(u32 cur_time);
void music_qskip(bool direct, u8 second);      //快进快退控制 direct = 1为快退，0为快进
void music_qskip_end(void);
void music_set_jump(void *brkpt);
void music_get_breakpiont(void *brkpt);
u32 get_music_bitrate(void);
void music_stream_var_init(void);
int music_is_encrypt(u16 key);                 //返回0表示为加密音乐
void music_stream_decrypt(void *buf, unsigned int len);
void res_play_loop_back(u8 enable);
void res_play_loop_back_skip_size(u16 skip_star_size, u16 skip_end_size); //外接FLASH音乐循环播放开关,设置循环时候跳过前后多少字节音频数据，用于部分应用避开静音区数
void rec_mp3_filelen_reduce(u16 ms);            //剪掉REC尾部 ms
void spiflash_rec_mp3_filelen_reduce(u16 ms);   //剪掉REC尾部 ms(spiflash)
void music_mp3_loopback_replay(void);
bool music_mp3_loopback_size(u32 start_byte_offset, u32 end_byte_offset);   //SD音乐循环,首尾地址偏移，返回true表示设置成功
bool music_mp3_loopback_set_play(u32 second);                               //设置循环播放的起点，返回true表示设置成功
bool music_mp3_loopback_reduce_end(u32 ms);                                 //循环播放，尾部ms不播放，返回true表示设置成功
#endif // _API_MUSIC_H
