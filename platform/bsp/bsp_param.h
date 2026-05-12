#ifndef __BSP_PARAM_H
#define __BSP_PARAM_H

#define PARAM_MSC_NUM_SD            0
#define PARAM_MSC_NUM_USB           2
#define PARAM_SYS_VOL               4
#define PARAM_HSF_VOL               5
#define PARAM_LANG_ID               6
#define PARAM_RANDOM_KEY            7           //4Byte随机数
#define PARAM_BT_XOSC_CAP           0x0b        //1Byte xosc value
#define PARAM_SYS_MODE              0x0c        //系统模式记忆
#define PARAM_MSC_NUM_SD1           0x0d        //2BYTE

#define PARAM_MSC_BRKPT_SD          0x10        //10BYTE = frame_count(4byte) +  file_ptr(4byte) + fname_crc(2byte)
#define PARAM_MSC_BRKPT_USB         0x1a        //10BYTE = frame_count(4byte) +  file_ptr(4byte) + fname_crc(2byte)
#define PARAM_MSC_BRKPT_SD1         0x24        //10BYTE = frame_count(4byte) +  file_ptr(4byte) + fname_crc(2byte)

#define PARAM_FMRX_CHCUR            0x30
#define PARAM_FMRX_CHCNT            0x31
#define PARAM_FMRX_CHBUF            0x32        //26Byte
#define PARAM_FMTX_FREQ             0x4C        //FM TX freq 2 Byte
#define PARAM_ECHO_LEVEL            0x4E        //echo level 1 Byte
#define PARAM_ECHO_DELAY            0x4F        //echo delay 1 Byte

#define PARAM_MASTER_ADDR_VALID     0x50        //用于记录主耳的地址是否有效
#define PARAM_MASTER_ADDR           0x51        //6BYTE
#define PARAM_NEW_LOCAL_NAME        0x57        //64byte=len+sum+62BYTE

#define RTCRAM_PWROFF_FLAG          63         //软关机的标识放在RTCRAM的最后一BYTE

#define BT_NAME_UPDATA              0x98        //U盘升级蓝牙名字 //32+4 byte

#define PARAM_SPIFLASH_MEMORY       0xBC        //使用10byte   // SPIFLASH断电记忆功能
#define PARAM_SPIFLASH_SAVE_TIME    0xC6        //使用2byte
#define PARAM_SPIFLASH_SAVE_NUM     0xC8        //使用2byte

#define PARAM_FOT_ADDR              0xCA        //4BYTE
#define PARAM_FOT_REMOTE_VER        0xCE        //2BYTE
#define PARAM_FOT_HEAD_INFO         0xD0        //8BYTE
#define PARAM_FOT_HASH              0xD8        //4BYTE

#define PARAM_SPIFLASH_LOOP0        0xDC        //4BYTE 记录spiflash循环读写起始地址
#define PARAM_SPIFLASH_LOOP1        0xE0        //4BYTE 记录spiflash循环读写起始地址

#if PWR_RC_32K_EN
#define PARAM_RTC_CNT               0xDC        //4BYTE
#define PARAM_RTC_32CLK             0xE0        //4BYTE
#define PARAM_RTC_ALARM             0xE4        //4BYTE
#define PARAM_RTC_ALARMSTA          0xE8        //1BYTE
#define PARAM_RTC_NSEC              0xE9        //4BYTE
#define PARAM_RTC_RCHZ              0xED        //4BYTE
#endif // PWR_RC_32K_EN

void param_init(bool reset);
void param_sync(void);
void param_msc_num_write(void);
void param_msc_num_read(void);
void param_msc_breakpoint_write(void);
void param_msc_breakpoint_read(void);
void param_fmrx_chcur_write(void);
void param_fmrx_chcur_read(void);
void param_fmrx_chcnt_write(void);
void param_fmrx_chcnt_read(void);
void param_fmrx_chbuf_write(void);
void param_fmrx_chbuf_read(void);
void param_sys_vol_write(void);
void param_sys_vol_read(void);
void param_lang_id_write(void);
void param_lang_id_read(void);
void param_random_key_write(void);
void param_random_key_read(u8 *key);
u8 param_sys_mode_read(void);
void param_sys_mode_write(u8 mode);
void param_fmtx_freq_write(void);
void param_fmtx_freq_read(void);
void param_spiflash_breakpoint_write(void);
void param_spiflash_breakpoint_read(void);

void param_fot_addr_write(u8 *param);
void param_fot_addr_read(u8 *param);
void param_fot_remote_ver_write(u8 *param);
void param_fot_remote_ver_read(u8 *param);
void param_fot_head_info_write(u8 *param);
void param_fot_head_info_read(u8 *param);
void param_fot_hash_write(u8 *param);
void param_fot_hash_read(u8 *param);

void param_spiflash_loop0_write(u8 *param);
void param_spiflash_loop0_read(u8 *param);
void param_spiflash_loop1_write(u8 *param);
void param_spiflash_loop1_read(u8 *param);
#endif // __BSP_PARAM_H

