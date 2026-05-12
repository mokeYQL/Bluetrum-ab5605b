#ifndef __CONFIG_EXTRA_H__
#define __CONFIG_EXTRA_H__

/*****************************************************************************
 * Module    : 系统功能配置
 *****************************************************************************/
#if !AUX_2_SDADC_EN
#undef AUX_REC_EN
#define AUX_REC_EN                  0
#endif

#ifndef SYS_MODE_BREAKPOINT_EN
#define SYS_MODE_BREAKPOINT_EN      0
#endif // SYS_MODE_BREAKPOINT_EN

#ifndef ENERGY_LED_EN
#define ENERGY_LED_EN               0
#endif

#ifndef PLUGIN_SYS_INIT_FINISH_CALLBACK
#define PLUGIN_SYS_INIT_FINISH_CALLBACK  0
#endif

#ifndef PLUGIN_FUNC_IDLE_ENTER_CHECK
#define PLUGIN_FUNC_IDLE_ENTER_CHECK     0
#endif

#if SYS_EQ_FOR_IDX_EN
#undef SYS_BASS_TREBLE_EN
#define SYS_BASS_TREBLE_EN          0
#endif

#ifndef SPIFLASH_SPEED_UP_EN
#define SPIFLASH_SPEED_UP_EN         1
#endif

#if !I2S_EN
#undef FUNC_I2S_EN
#define FUNC_I2S_EN                     0
#else
    #if FUNC_I2S_EN                     //打开了slave接收
    #undef I2S_MODE_SEL
    #undef I2S_DMA_EN
    #define I2S_MODE_SEL                1
    #define I2S_DMA_EN                  1
    #endif
#endif

#if !HUART_AUDIO_EN
#undef FUNC_HUART_EN
#define FUNC_HUART_EN                     0
#endif

#ifndef WARNING_WAVRES_PLAY
#define WARNING_WAVRES_PLAY              0
#endif

#ifndef MIC_EQ_EN
#define MIC_EQ_EN                        0
#endif

#ifndef KARAOK_DENOISE_EN
#define KARAOK_DENOISE_EN              0
#endif

#ifndef  IIS_EXT_EN
#define  IIS_EXT_EN                      0
#endif

#if !USER_TKEY
#undef  TKEY_CH
#undef  USER_TKEY_DEBUG_EN
#undef  USER_TKEY_HW_EN
#undef  USER_TKEY_MUL_EN
#define TKEY_CH                         0xff
#define USER_TKEY_DEBUG_EN              0
#define USER_TKEY_HW_EN                 0
#define USER_TKEY_MUL_EN                0
#endif

#if USER_TKEY_MUL_EN
#if (TKEY_CH0_EN ? 1 : 0) + (TKEY_CH1_EN ? 1 : 0) + (TKEY_CH2_EN ? 1 : 0) + (TKEY_CH3_EN ? 1 : 0) >= 1
#undef USER_TKEY_HW_EN
#define USER_TKEY_HW_EN                 0
#else
#undef USER_TKEY_MUL_EN
#define USER_TKEY_MUL_EN                0
#endif
#endif


#if MICAUX_ANALOG_OUT_ALWAYS
#undef   AUX_SNR_EN
#undef   DAC_DNR_EN
#undef   DAC_OFF_FOR_BT_CONN_EN
#define  AUX_SNR_EN                    0
#define  DAC_DNR_EN                    0
#define  DAC_OFF_FOR_BT_CONN_EN        0
#endif


#ifndef EXSPI_REC_PITCH_SHIFT_PLAY
#define EXSPI_REC_PITCH_SHIFT_PLAY     0
#endif

#if EXSPI_REC_PITCH_SHIFT_PLAY
#undef  SYS_CLK_SEL
#define SYS_CLK_SEL                     SYS_160M //选择系统时钟
#endif

#if LED_MATRIX_HUART_TX
#undef HUART_DUMP_BUF_EN
#undef BT_ID3_TAG_EN
#define HUART_DUMP_BUF_EN              1
#define BT_ID3_TAG_EN                  1
#endif
/*****************************************************************************
 * Module    : HDMI功能配置
 *****************************************************************************/
#ifndef FUNC_HDMI_EN
#define FUNC_HDMI_EN                   0
#endif

#if  FUNC_HDMI_EN
#undef BT_TWS_EN
#undef GUI_SELECT
#if SYS_CLK_SEL < SYS_120M
#undef  SYS_CLK_SEL
#define SYS_CLK_SEL        SYS_120M
#endif

#define GUI_SELECT          GUI_NO      //借用7脚屏 PA0~PA6 测试HDMI_ARC功能.
#define BT_TWS_EN           0

#undef I2C_EN
#undef I2C_MUX_SD_EN
#undef I2C_SCL_IN
#undef I2C_SCL_OUT
#undef I2C_SCL_H
#undef I2C_SCL_L
#undef I2C_SDA_IN
#undef I2C_SDA_OUT
#undef I2C_SDA_H
#undef I2C_SDA_L
#undef I2C_SDA_IS_H
#undef I2C_SDA_SCL_OUT
#undef I2C_SDA_SCL_H
#undef I2C_IO_INIT

#define I2C_EN                          1           //是否使能I2C功能
#define I2C_MUX_SD_EN                   0           //是否I2C复用SD卡的IO
//SCL_PB1  SDA_PB2
#define IIC_CLK     BIT(1)
#define IIC_DAT     BIT(2)
#define I2C_IO_INIT()                   {GPIOBFEN &= ~(IIC_CLK|IIC_DAT); GPIOBDE |= (IIC_CLK|IIC_DAT); GPIOBDIR |= (IIC_CLK|IIC_DAT); GPIOBPU |= IIC_CLK|IIC_DAT;}
#define I2C_SCL_IN()                    {GPIOBDIR |= IIC_CLK;}
#define I2C_SCL_OUT()                   {GPIOBDIR &= ~IIC_CLK;}
#define I2C_SCL_H()                     {GPIOBSET = IIC_CLK;}
#define I2C_SCL_L()                     {GPIOBCLR = IIC_CLK;}
#define I2C_SDA_IN()                    {GPIOBDIR |= IIC_DAT;}
#define I2C_SDA_OUT()                   {GPIOBDIR &= ~IIC_DAT;}
#define I2C_SDA_H()                     {GPIOBSET = IIC_DAT;}
#define I2C_SDA_L()                     {GPIOBCLR = IIC_DAT;}
#define I2C_SDA_IS_H()                  (GPIOB & IIC_DAT)

#define I2C_SDA_SCL_OUT()               {I2C_SDA_OUT(); I2C_SCL_OUT();}
#define I2C_SDA_SCL_H()                 {I2C_SDA_H(); I2C_SCL_H();}

#endif // FUNC_HDMI_EN


/*****************************************************************************
 * Module    : 音乐功能配置
 *****************************************************************************/
#if !FUNC_MUSIC_EN
#undef  MUSIC_UDISK_EN
#undef  MUSIC_SDCARD_EN
#undef  MUSIC_SDCARD1_EN

#undef  MUSIC_WAV_SUPPORT
#undef  MUSIC_WMA_SUPPORT
#undef  MUSIC_APE_SUPPORT
#undef  MUSIC_FLAC_SUPPORT
#undef  MUSIC_SBC_SUPPORT
#undef  MUSIC_ID3_TAG_EN

#define MUSIC_UDISK_EN              0
#define MUSIC_SDCARD_EN             0
#define MUSIC_SDCARD1_EN             0
#define MUSIC_WAV_SUPPORT           0
#define MUSIC_WMA_SUPPORT           0
#define MUSIC_APE_SUPPORT           0
#define MUSIC_FLAC_SUPPORT          0
#define MUSIC_SBC_SUPPORT           0
#define MUSIC_ID3_TAG_EN            0
#endif // FUNC_MUSIC_EN

#if !MUSIC_BREAKPOINT_EN
#undef MUSIC_AB_BREAKPOINT_EN
#define MUSIC_AB_BREAKPOINT_EN      0
#endif

#if !CHARGE_EN
#undef  CHARGE_TRICK_EN
#undef  CHARGE_DC_IN
#undef  CHARGE_DC_NOT_PWRON
#define CHARGE_TRICK_EN             0
#define CHARGE_DC_NOT_PWRON         0
#define CHARGE_DC_IN()              0
#endif

#if !EARPHONE_DETECT_EN
#undef SDCMD_MUX_DETECT_EARPHONE
#undef EARPHONE_DETECT_INIT
#undef EARPHONE_IS_ONLINE
#undef IS_DET_EAR_BUSY
#define EARPHONE_DETECT_INIT()
#define SDCMD_MUX_DETECT_EARPHONE   0
#define EARPHONE_IS_ONLINE()        0
#define IS_DET_EAR_BUSY()           0
#endif

#if !LINEIN_DETECT_EN
#undef SDCMD_MUX_DETECT_LINEIN
#undef LINEIN_DETECT_INIT
#undef LINEIN_IS_ONLINE
#undef IS_DET_LINEIN_BUSY

#define LINEIN_DETECT_INIT()
#define SDCMD_MUX_DETECT_LINEIN     0
#define LINEIN_IS_ONLINE()          0
#define IS_DET_LINEIN_BUSY()        0
#endif

#ifndef PWROFF_PRESS_TIME
#define PWROFF_PRESS_TIME           3               //1:大约1秒    3: 1.5s,  6: 2s,  9: 2.5s, 12: 3s
#endif // PWROFF_PRESS_TIME

#ifndef PWRON_PRESS_TIME
#define PWRON_PRESS_TIME            1500           //1.5s
#endif // PWRON_PRESS_TIME

#ifndef DOUBLE_KEY_TIME
#define DOUBLE_KEY_TIME             1
#endif // DOUBLE_KEY_TIME

#ifndef SYS_LIMIT_VOLUME
#define SYS_LIMIT_VOLUME            SYS_INIT_VOLUME
#endif // SYS_LIMIT_VOLUME

#ifndef LINEIN_2_PWRDOWN_TONE_EN
#define LINEIN_2_PWRDOWN_TONE_EN    0   //插入Linein直接关机，是否播放关机提示音。
#endif // LINEIN_2_PWRDOWN_TONE_EN

#ifndef LPWR_WARING_TIMES
#define LPWR_WARING_TIMES           0xff
#endif // LPWR_WARING_TIMES

#if !BUZZER_EN
#undef BUZZER_INIT
#define BUZZER_INIT()
#undef BUZZER_ON
#define BUZZER_ON()
#undef BUZZER_OFF
#define BUZZER_OFF()
#endif

#ifndef LED_LOWBAT_EN
#define LED_LOWBAT_EN              0
#endif // RLED_LOWBAT_EN

#ifndef RLED_LOWBAT_FOLLOW_EN
#define RLED_LOWBAT_FOLLOW_EN       0
#endif // RLED_LOWBAT_FOLLOW_EN

#ifndef SLEEP_DAC_OFF_EN
#define SLEEP_DAC_OFF_EN            1
#endif // SLEEP_DAC_OFF_EN

#ifndef PWRON_FRIST_BAT_EN
#define PWRON_FRIST_BAT_EN          1
#endif // PWRON_FRIST_BAT_EN

#if !LED_DISP_EN
#undef LED_LOWBAT_EN
#undef RLED_LOWBAT_FOLLOW_EN
#undef BLED_FFT_EN
#undef BLED_LOW2ON_EN
#undef BLED_CHARGE_FULL_EN
#undef BT_RECONN_LED_EN
#define LED_LOWBAT_EN              0
#define RLED_LOWBAT_FOLLOW_EN       0
#define BLED_FFT_EN                 0
#define BLED_LOW2ON_EN              0
#define BLED_CHARGE_FULL_EN         0
#define BT_RECONN_LED_EN            0
#endif

#if !LED_DISP_EN
#undef LED_INIT
#define LED_INIT()
#undef LED_SET_ON
#define LED_SET_ON()
#undef LED_SET_OFF
#define LED_SET_OFF()
#endif

#if !LED_PWR_EN
#undef LED_PWR_INIT
#define LED_PWR_INIT()
#undef LED_PWR_SET_ON
#define LED_PWR_SET_ON()
#undef LED_PWR_SET_OFF
#define LED_PWR_SET_OFF()
#endif

#if ((GUI_SELECT & DISPLAY_LCD) == DISPLAY_LCD)
#define GUI_LCD_EN                      1
#else
#define GUI_LCD_EN                      0
#undef  MUSIC_NAVIGATION_EN
#define MUSIC_NAVIGATION_EN             0
#endif

#if !LOUDSPEAKER_MUTE_EN
#undef LOUDSPEAKER_MUTE_INIT
#define LOUDSPEAKER_MUTE_INIT()
#undef LOUDSPEAKER_MUTE_DIS
#define LOUDSPEAKER_MUTE_DIS()
#undef LOUDSPEAKER_MUTE
#define LOUDSPEAKER_MUTE()
#undef LOUDSPEAKER_UNMUTE
#define LOUDSPEAKER_UNMUTE()
#undef LOUDSPEAKER_UNMUTE_DELAY
#define LOUDSPEAKER_UNMUTE_DELAY        0
#endif

#ifndef IR_INPUT_NUM_MAX
#define IR_INPUT_NUM_MAX                999         //最大输入数字9999
#endif // IR_INPUT_NUM_MAX

#ifndef FMRX_THRESHOLD_VAL
#define FMRX_THRESHOLD_VAL              128
#endif // FMRX_THRESHOLD_VAL

#ifndef FMRX_SEEK_DISP_CH_EN
#define FMRX_SEEK_DISP_CH_EN            0
#endif // FMRX_SEEK_DISP_CH_EN

#ifndef SD_SOFT_DETECT_EN
#define SD_SOFT_DETECT_EN               0
#undef SD_IS_SOFT_DETECT
#define SD_IS_SOFT_DETECT()             0
#endif // SD_SOFT_DETECT_EN

#if USER_ADKEY_MUX_LED
#undef  USER_ADKEY
#undef  USER_ADKEY_MUX_SDCLK
#undef  ADKEY_PU10K_EN

#define USER_ADKEY                      1
#define USER_ADKEY_MUX_SDCLK            0
#define ADKEY_PU10K_EN                  0
#endif // USER_ADKEY_MUX_LED

#if ((SD0_MAPPING == SD0MAP_G6) || (SD0_MAPPING == SD0MAP_G5) || (SD0_MAPPING == SD0MAP_G4)) && MUSIC_UDISK_EN && MUSIC_SDCARD_EN
#define SD_USB_MUX_IO_EN				1
#elif((SD1_MAPPING == SD0MAP_G6) || (SD1_MAPPING == SD0MAP_G5) || (SD1_MAPPING == SD0MAP_G4)) && MUSIC_UDISK_EN && MUSIC_SDCARD1_EN
#define SD_USB_MUX_IO_EN				1
#else
#define SD_USB_MUX_IO_EN				0
#endif


#if FMTX_EN
#undef  DAC_OUT_SPR
#undef  FUNC_FMRX_EN
#define DAC_OUT_SPR                     DAC_OUT_48K
#define FUNC_FMRX_EN                    0
#endif // FMTX_EN

///FMRX不支持蓝牙后台
#if BT_BACKSTAGE_EN
#undef  FUNC_FMRX_EN
#define FUNC_FMRX_EN                    0
#endif

#if FUNC_SPDIF_TX_EN
#undef  DAC_OUT_SPR
#define DAC_OUT_SPR                     DAC_OUT_48K
#endif

#if !FMTX_EN
#undef  FMTX_DNR_EN
#define FMTX_DNR_EN                     0
#endif

 /*****************************************************************************
 * Module    : TOM_CAT功能配置
 *****************************************************************************/
#if FUNC_TOM_CAT_EN
#undef FLASH_SIZE
#undef FUNC_REC_EN
#undef TOM_CAT_REC_EN
#undef EX_SPIFLASH_SUPPORT
#undef EXSPI_REC_PITCH_SHIFT_PLAY
#undef INTERNAL_FLASH_REC
#undef REC_DELETE_FILE
#define FLASH_SIZE                      FSIZE_1M
#define FUNC_REC_EN                     1
#define TOM_CAT_REC_EN                  1
#define EX_SPIFLASH_SUPPORT             EXSPI_REC
#define EXSPI_REC_PITCH_SHIFT_PLAY      1
#define INTERNAL_FLASH_REC              1
#define REC_DELETE_FILE                 1
#endif

/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#if !FUNC_REC_EN
#undef  FMRX_REC_EN
#undef  AUX_REC_EN
#undef  MIC_REC_EN
#undef  REC_TYPE_SEL
#undef  REC_AUTO_PLAY
#undef  REC_FAST_PLAY
#undef  BT_REC_EN
#undef  BT_HFP_REC_EN
#undef  KARAOK_REC_EN
#undef  REC_STOP_MUTE_1S

#define FMRX_REC_EN                 0
#define AUX_REC_EN                  0
#define MIC_REC_EN                  0
#define REC_AUTO_PLAY               0
#define REC_FAST_PLAY               0
#define BT_REC_EN                   0
#define BT_HFP_REC_EN               0
#define KARAOK_REC_EN               0
#define REC_TYPE_SEL                REC_NO
#define REC_STOP_MUTE_1S            0
#endif //FUNC_REC_EN


#if FMRX_INSIDE_EN
#undef  FMRX_QN8035_EN
#define FMRX_QN8035_EN              0
#else
#undef  I2C_EN
#define I2C_EN                      1
#endif

#if !I2C_EN
#undef  I2C_MUX_SD_EN
#define I2C_MUX_SD_EN               0
#endif

#if ((GUI_SELECT & DISPLAY_LCD) == DISPLAY_LCD)
#define GUI_LCD_EN                      1
#else
#define GUI_LCD_EN                      0
#endif


#ifndef EX_SPIFLASH_SUPPORT
#define EX_SPIFLASH_SUPPORT             0
#endif
/*****************************************************************************
 * Module    : karaok相关配置
 *****************************************************************************/
#if SYS_KARAOK_EN
#undef FUNC_SPEAKER_EN
#undef DAC_DNR_EN
#undef BT_PLC_EN
#undef AUX_REC_EN
#undef BT_HFP_REC_EN
#undef MUSIC_WMA_SUPPORT
#undef AUX_2_SDADC_EN
#undef AUX_SNR_EN

#if SYS_CLK_SEL < SYS_120M
#undef  SYS_CLK_SEL
#define SYS_CLK_SEL                 SYS_120M
#endif

#define BT_PLC_EN                   0   //暂时先关掉PLC
#define FUNC_SPEAKER_EN             0
#define DAC_DNR_EN                  0   //暂时先关掉动态降噪
#define BT_HFP_REC_EN               0   //Karaok不支持通话录音
#define AUX_REC_EN                  0   //Karaok不支持AUX录音(直通)
#define MUSIC_WMA_SUPPORT           0   //Karaok不支持WMA
#define AUX_2_SDADC_EN              0   //Karaok AUX设为直通（一个ADC）
#define AUX_SNR_EN                  0   //Karaok AUX直通，ADC用于MIC，不能通过SDADC的数据动态降噪

#else   //不开KARAOK时
#undef  KARAOK_RM_VOICE
#define KARAOK_RM_VOICE             0    //消人声目前只在KARAOK模式下用
#endif

#if SYS_HOWLING_EN
#undef SYS_MAGIC_VOICE_EN
#define SYS_MAGIC_VOICE_EN          1
#endif
#ifndef MUSIC_SDCARD1_EN
#define MUSIC_SDCARD1_EN            0
#endif
/*****************************************************************************
 * Module    : 蓝牙相关配置
 *****************************************************************************/
#ifndef BT_A2DP_RECON_EN
#define BT_A2DP_RECON_EN            0
#endif

#if BT_AEC_EN || BT_HFP_REC_EN || SCO_MAV_EN
#undef  BT_HFP_MSBC_EN
#define BT_HFP_MSBC_EN              0
#endif // BT_AEC_EN

#if BT_TWS_FAST_PAIR_EN
#undef BT_TWS_PAIR_MODE
#define BT_TWS_PAIR_MODE            0
#undef LE_EN
#undef LE_PAIR_EN
#define LE_EN                       0
#define LE_PAIR_EN                  0
#endif

#if !LE_EN
#undef LE_AB_FOT_EN
#define LE_AB_FOT_EN                0
#endif

#if LE_EN
    #define BT_DUAL_MODE_EN         1
#else
    #define BT_DUAL_MODE_EN         0
#endif

#if BT_AB_FOT_EN || LE_AB_FOT_EN
#undef FOT_EN
#define FOT_EN                      1   //是否打开FOTA升级功能
#endif

#if BT_AB_FOT_EN
#if !BT_SPP_EN
#error "FOTA: please open BT_AB_FOT_EN & BT_SPP_EN at the same time\n"
#endif
#endif // BT_AB_FOT_EN

#ifndef BT_PAIR_SLEEP_EN
#define BT_PAIR_SLEEP_EN            0
#endif // BT_PAIR_SLEEP_EN

#if BT_BACKSTAGE_EN
#undef FMRX_THRESHOLD_FZ

#define FMRX_THRESHOLD_FZ           0
#endif

#ifndef BT_HFP_RINGS_BEFORE_NUMBER
#define BT_HFP_RINGS_BEFORE_NUMBER   0
#endif


#if BT_FCC_TEST_EN    //FCC 默认PB3 (USB_DP) 波特率1500000通信, 关闭用到PB3的程序
#undef FUNC_USBDEV_EN
#undef MUSIC_UDISK_EN
#define FUNC_USBDEV_EN             0
#define MUSIC_UDISK_EN             0
#if (UART0_PRINTF_SEL == PRINTF_PB3)
#undef UART0_PRINTF_SEL
#define UART0_PRINTF_SEL  PRINTF_NONE
#endif
#endif

#if !BT_TWS_EN
#undef BT_TSCO_EN
#define BT_TSCO_EN                  0

#undef  WARNING_BT_TWS_CONNECT
#undef  WARNING_BT_TWS_DISCON
#define WARNING_BT_TWS_CONNECT      0
#define WARNING_BT_TWS_DISCON       0
#endif

#if BT_FCC_TEST_EN
    #define WORK_MODE           MODE_FCC_TEST
#elif BT_BQB_RF_EN
    #define WORK_MODE           MODE_BQB_RF
#elif BT_BQB_PROFILE_EN
    #define WORK_MODE           MODE_BQB_PROFILE
#elif BT_CBT_TEST_EN
    #define WORK_MODE           MODE_CBT_TEST
#else
    #define WORK_MODE           MODE_NORMAL
#endif

#if BT_HID_SIMPLE_KEYBOARD
#undef  BT_HID_TYPE
#define BT_HID_TYPE             HID_TYPE_KEYBOARD
#endif


#if BT_A2DP_VOL_HID_CTRL_EN
#undef BT_HID_EN
#undef BT_HID_TYPE
#undef BT_A2DP_VOL_CTRL_EN
#define BT_HID_EN                       1   //是否打开蓝牙HID服务
#define BT_HID_TYPE                     HID_TYPE_PHOTO1   //选择HID服务类型
#define BT_A2DP_VOL_CTRL_EN             1   //是否支持A2DP音量与手机同步
#endif

#ifndef DAC_OFF_FOR_BT_CONN_EN
#define DAC_OFF_FOR_BT_CONN_EN      0
#endif

#ifndef BT_TOUCH_SCREEN_EN
#define BT_TOUCH_SCREEN_EN              0
#endif

#ifndef BT_SRCRAM_PHASECOM_SYNC
#define BT_SRCRAM_PHASECOM_SYNC         0
#endif

/*****************************************************************************
 * Module    : uart0 printf 功能自动配置(自动关闭SD卡，USB)
 *****************************************************************************/
#if (UART0_PRINTF_SEL == PRINTF_PB2)
    #if (SD0_MAPPING == SD0MAP_G2)
    #undef  MUSIC_SDCARD_EN
    #define MUSIC_SDCARD_EN         0
    #endif

    #if (SD1_MAPPING == SD0MAP_G2)
    #undef  MUSIC_SDCARD1_EN
    #define MUSIC_SDCARD1_EN        0
    #endif
#elif (UART0_PRINTF_SEL == PRINTF_PA7)
    #if (SD0_MAPPING == SD0MAP_G1)
    #undef  MUSIC_SDCARD_EN
    #define MUSIC_SDCARD_EN         0
    #endif

    #if (SD1_MAPPING == SD0MAP_G1)
    #undef  MUSIC_SDCARD1_EN
    #define MUSIC_SDCARD1_EN        0
    #endif
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    #if ((SD0_MAPPING == SD0MAP_G6) || (SD0_MAPPING == SD0MAP_G5) || (SD0_MAPPING == SD0MAP_G6))
    #undef  MUSIC_SDCARD_EN
    #define MUSIC_SDCARD_EN         0
    #endif

    #if ((SD1_MAPPING == SD0MAP_G6) || (SD1_MAPPING == SD0MAP_G5) || (SD1_MAPPING == SD0MAP_G6))
    #undef  MUSIC_SDCARD1_EN
    #define MUSIC_SDCARD1_EN        0
    #endif

    #undef  FUNC_USBDEV_EN
    #undef  MUSIC_UDISK_EN
    #define FUNC_USBDEV_EN          0
    #define MUSIC_UDISK_EN          0
#elif (UART0_PRINTF_SEL == PRINTF_NONE)
    //关闭所以打印信息
    #undef printf
    #undef vprintf
    #undef print_r
    #undef print_r16
    #undef print_r32
    #undef printk
    #undef vprintk
    #undef print_kr
    #undef print_kr16
    #undef print_kr32

    #define printf(...)
    #define vprintf(...)
    #define print_r(...)
    #define print_r16(...)
    #define print_r32(...)
    #define printk(...)
    #define vprintk(...)
    #define print_kr(...)
    #define print_kr16(...)
    #define print_kr32(...)
#endif

#if !MUSIC_SDCARD_EN
#undef SDCLK_MUX_DETECT_SD
#undef SD_DETECT_INIT
#undef SD_IS_ONLINE
#undef IS_DET_SD_BUSY

#define SD_DETECT_INIT()
#define SDCLK_MUX_DETECT_SD         0
#define SD_IS_ONLINE()              0
#define IS_DET_SD_BUSY()            0
#endif

#if !MUSIC_SDCARD1_EN
#undef SD1_DETECT_INIT
#undef SD1_IS_ONLINE
#undef IS_DET_SD1_BUSY

#define SD1_DETECT_INIT()
#define SD1_IS_ONLINE()              0
#define IS_DET_SD1_BUSY()            0
#endif

/*****************************************************************************
 * Module    : usb device 功能配置
 *****************************************************************************/
#if !FUNC_USBDEV_EN
    #undef  UDE_STORAGE_EN
    #undef  UDE_SPEAKER_EN
    #undef  UDE_HID_EN
    #undef  UDE_MIC_EN
    #undef  UDE_ENUM_TYPE

    #define UDE_STORAGE_EN              0
    #define UDE_SPEAKER_EN              0
    #define UDE_HID_EN                  0
    #define UDE_MIC_EN                  0
    #define UDE_ENUM_TYPE               0
#else
    #define UDE_ENUM_TYPE               (UDE_STORAGE_EN*0x01 + UDE_SPEAKER_EN*0x02 + UDE_HID_EN*0x04 + UDE_MIC_EN*0x08)
#endif

/*****************************************************************************
 * Module    : 提示音配置
 *****************************************************************************/
#if (!WARNING_TONE_EN)
#undef WARNING_POWER_ON
#undef WARNING_POWER_OFF
#undef WARNING_FUNC_MUSIC
#undef WARNING_FUNC_BT
#undef WARNING_FUNC_CLOCK
#undef WARNING_FUNC_FMRX
#undef WARNING_FUNC_AUX
#undef WARNING_FUNC_USBDEV
#undef WARNING_FUNC_SPEAKER
#undef WARNING_LOW_BATTERY
#undef WARNING_BT_WAIT_CONNECT
#undef WARNING_BT_CONNECT
#undef WARNING_BT_DISCONNECT
#undef WARNING_BT_DISCONNECT
#undef WARNING_BT_TWS_CONNECT
#undef WARNING_BT_INCALL
#undef WARNING_BT_CALLOUT
#undef WARNING_USB_SD
#undef WARNING_BT_HID_MENU
#undef WARNING_BTHID_CONN
#undef WARNING_MAX_VOLUME
#undef WARNING_BT_PAIR
#undef WARNING_UPDATE_DONE
#undef BT_HFP_PLAY_IOS_RING_EN

#define WARNING_POWER_ON                0
#define WARNING_POWER_OFF               0
#define WARNING_FUNC_MUSIC              0
#define WARNING_FUNC_BT                 0
#define WARNING_FUNC_CLOCK              0
#define WARNING_FUNC_FMRX               0
#define WARNING_FUNC_AUX                0
#define WARNING_FUNC_USBDEV             0
#define WARNING_FUNC_SPEAKER            0
#define WARNING_LOW_BATTERY             0
#define WARNING_BT_WAIT_CONNECT         0
#define WARNING_BT_CONNECT              0
#define WARNING_BT_DISCONNECT           0
#define WARNING_BT_TWS_CONNECT          0
#define WARNING_BT_TWS_DISCON           0
#define WARNING_BT_INCALL               0
#define WARNING_BT_CALLOUT              0
#define WARNING_USB_SD                  0
#define WARNING_BT_HID_MENU             0
#define WARNING_BTHID_CONN              0
#define WARNING_MAX_VOLUME              0
#define WARNING_BT_PAIR                 0
#define WARNING_UPDATE_DONE             0
#define BT_HFP_PLAY_IOS_RING_EN         0
#endif

/*****************************************************************************
 * Module    : SD/USB 升级和复用配置
 *****************************************************************************/
#if ((!MUSIC_UDISK_EN) && (!MUSIC_SDCARD_EN) && (!MUSIC_SDCARD1_EN))
#undef  USB_SD_UPDATE_EN
#define USB_SD_UPDATE_EN               0
#endif

#if SD_USB_MUX_IO_EN == 1
	#undef FUNC_USBDEV_EN
	#define FUNC_USBDEV_EN 			   0
#endif

#ifndef UPD_FILENAME
#define UPD_FILENAME                   "fw5000.upd"
#endif

#if UART_M_UPDATE||UART_S_UPDATE
#undef GUI_SELECT
#define GUI_SELECT                      GUI_NO                  //GUI Display Select
#endif

#if UART_S_UPDATE && (((UART_UPD_PORT_SEL&UART_RX0_G3_PB4)==UART_RX0_G3_PB4)||((UART_UPD_PORT_SEL&UART_RX0_G3_PB3)==UART_RX0_G3_PB3))

#undef FUNC_USBDEV_EN
#undef MUSIC_UDISK_EN

#define FUNC_USBDEV_EN               0
#define MUSIC_UDISK_EN               0

#endif

#if ((SD0_MAPPING == SD0MAP_G1) || (SD0_MAPPING == SD0MAP_G4) || (SD0_MAPPING == SD0MAP_G5) || (SD0_MAPPING == SD0MAP_G6))
#define SDCLK_AD_CH                    ADCCH_PA6
#elif (SD0_MAPPING == SD0MAP_G2)
#define SDCLK_AD_CH                    ADCCH_PB1
#else
#define SDCLK_AD_CH                    ADCCH_PE6
#endif
#define USB_SUPPORT_EN                 (MUSIC_UDISK_EN | FUNC_USBDEV_EN)
#define SD_SUPPORT_EN                  (MUSIC_SDCARD_EN | MUSIC_SDCARD1_EN)

#if MUSIC_SDCARD_EN
#define SDCMD_MUX_DETECT_EN            1
#define SDCLK_MUX_DETECT_EN            1
#else
#undef  USER_ADKEY_MUX_SDCLK
#undef  I2C_MUX_SD_EN

#define I2C_MUX_SD_EN                  0
#define SDCMD_MUX_DETECT_EN            0
#define SDCLK_MUX_DETECT_EN            0
#define USER_ADKEY_MUX_SDCLK           0
#endif // MUSIC_SDCARD_EN

#if !SD_SUPPORT_EN
#undef  UDE_STORAGE_EN
#define UDE_STORAGE_EN                 0
#endif

/*****************************************************************************
* Module    : 其他功能配置
*****************************************************************************/
#if EXLIB_SOFT_EQ_DRC_EN
#define EXLIB_SOFT_EQ_EN                1     //软件EQ使能
#define EXLIB_DRC_PREVOL_EN             1     //DAC前置音量使能
#define EXLIB_DRC_EN                    1     //软件DRC使能
#else
#define EXLIB_SOFT_EQ_EN                0     //软件EQ使能
#define EXLIB_DRC_PREVOL_EN             0     //DAC前置音量使能
#define EXLIB_DRC_EN                    0     //软件DRC使能
#endif

#if EXLIB_DRC_EN || EXLIB_SOFT_EQ_EN || EXLIB_BT_MONO_XDRC_EN ||EXLIB_SOFT_EQ_DRC_EN
#undef  UART0_PRINTF_SEL
#undef  FUNC_USBDEV_EN
#undef  MUSIC_UDISK_EN
#undef  SYS_ADJ_DIGVOL_EN
#undef  EQ_DBG_IN_UART

#define UART0_PRINTF_SEL                PRINTF_PA7   //PA7打印调试  //PRINTF_PA7  //PRINTF_NONE
#define FUNC_USBDEV_EN                  0            //该功能暂时屏蔽,USB_PB3临时用于在线调软件EQ
#define MUSIC_UDISK_EN                  0            //该功能暂时屏蔽,USB_PB3临时用于在线调软件EQ
#define SYS_ADJ_DIGVOL_EN               1            //XDRC 一般调所以算法前的PRE_VOL
#define EQ_DBG_IN_UART                  1            //串口在线调音效使能,需要在工具配置界面中打开"EQ调试"功能,且选EQ调试串口PB3

#if SYS_CLK_SEL < SYS_160M
#undef  SYS_CLK_SEL
#define SYS_CLK_SEL                     SYS_160M
#endif
#endif


#if IIS_MASTER_RX_2_UDE_MIC
#undef  IIS_EXT_EN
#define IIS_EXT_EN                     1
#endif
/*****************************************************************************
* Module    : 有冲突或功能上不能同时打开的宏
*****************************************************************************/
#if (SYS_KARAOK_EN && BT_BACKSTAGE_EN)
#error "SYS_KARAOK_EN and BT_BACKSTAGE_EN (include BT_APP_EN) can't open at sametime"
#endif

#if BT_AEC_DUMP_EN && BT_ALC_DUMP_EN
#error "BT_AEC_DUMP_EN and BT_ALC_DUMP_EN can't open at sametime"   //alc和aec导出数据,只能打开其中一个
#endif

#if EXLIB_SOFT_EQ_DRC_EN && EXLIB_BT_MONO_XDRC_EN
#error "EXLIB_SOFT_EQ_DRC_EN and BEXLIB_BT_MONO_XDRC_EN can't open at sametime"
#endif
/*****************************************************************************
* Module    : 计算FunctionKey
*****************************************************************************/
#if FMTX_EN
    #define FUNCKEY_FMTX                0xc3a2a472
#else
    #define FUNCKEY_FMTX                0
#endif

#if UDE_MIC_KARAOK_EN
#undef DAC_OUT_SPR
#define DAC_OUT_SPR                 DAC_OUT_48K
#undef UDE_STORAGE_EN
#define UDE_STORAGE_EN                 0
#endif

#endif // __CONFIG_EXTRA_H__
