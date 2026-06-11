/*****************************************************************************
 * Module    : Config
 * File      : config.h
 * Function  : SDK配置文件
 *****************************************************************************/

#ifndef USER_CONFIG_H
#define USER_CONFIG_H
#include "config_define.h"

/*****************************************************************************
 * Module    : Function选择相关配置
 *****************************************************************************/
#define FUNC_MUSIC_EN                   0 // 是否打开MUSIC功能
#define FUNC_CLOCK_EN                   0 // 是否打开时钟功能
#define FUNC_FMRX_EN                    0 // 是否打开FM功能
#define FUNC_BT_EN                      1 // 是否打开蓝牙功能
#define FUNC_BTHID_EN                   0 // 是否打开独立自拍器模式
#define FUNC_BT_DUT_EN                  0 // 是否打开蓝牙的独立DUT测试模式
#define FUNC_BT_FCC_EN                  0 // 是否打开蓝牙的独立FCC测试模式
#define FUNC_AUX_EN                     1 // 是否打开AUX功能
#define FUNC_USBDEV_EN                  0 // 是否打开USB DEVICE功能
#define FUNC_SPEAKER_EN                 0 // 是否打开Speaker模式
#define FUNC_SPDIF_EN                   0 // 是否打开SPDIF功能
#define FUNC_HDMI_EN                    0 // 是否打开HDMI_ARC功能
#define FUNC_I2S_EN                     0 // 是否打开I2S接收功能（需使能I2S为Slave）
#define FUNC_HUART_EN                   0 // 是否打开HUART接收功能
#define FUNC_TOM_CAT_EN                 0 // 是否打开TOM_CAT模式(需要开启录音功能，使用flash 1M的型号进行内录)
#define FUNC_IDLE_EN                    0 // 是否打开IDLE功能

/*****************************************************************************
 * Module    : 系统功能选择配置
 *****************************************************************************/
#define SYS_CLK_SEL                     SYS_120M // 选择系统时钟
#define POWKEY_10S_RESET                xcfg_cb.powkey_10s_reset
#define SOFT_POWER_ON_OFF               1          // 是否使用软开关机功能
#define PWRKEY_2_HW_PWRON               0          // 用PWRKEY模拟硬开关
#define USB_SD_UPDATE_EN                0          // 是否支持UDISK/SD的离线升级
#define EXT_32K_EN                      0          // 是否使用外部32.768K晶振,否则内部32K OSC。
#define PWR_RC_32K_EN                   0          // 软关机时RTC是否省晶振，使用RC 32K
#define SYS_ADJ_DIGVOL_EN               0          // 系统是否调数字音量
#define GUI_SELECT                      GUI_NO     // GUI Display Select
#define UART0_PRINTF_SEL                PRINTF_PA7 // PRINTF_PB3, PRINTF_PA7 选择UART打印信息输出IO，或关闭打印信息输出
#define UART0_CLK_SEL_X26M              1          // 打印用的UART0时钟源选则X26M,系统时钟大于13M时才支持串口1.5M打印输出
#define BT_POWER_OPTIMIZE               0          // 蓝牙电源优化,芯片供电纹波比较大时,蓝牙播放音乐可能卡顿,打开此选项优化蓝牙播放音乐的卡顿
#define LED_MATRIX_HUART_TX             0          // 用作LED点阵屏主机，huart默认使用PB3，1500000波特率

/*****************************************************************************
 * Module    : FLASH配置
 *****************************************************************************/
#define FLASH_SIZE                      FSIZE_512K // LQFP48芯片内置1MB，其它封装芯片内置512KB(实际导出prd文件要小于492K)
#define FLASH_CODE_SIZE                 492K       // 程序使用空间大小
#define FLASH_ERASE_4K                  1          // 是否支持4K擦除
#define FLASH_DUAL_READ                 1          // 是否支持2线模式
#define FLASH_QUAD_READ                 0          // 是否支持4线模式
#define SPIFLASH_SPEED_UP_EN            1          // SPI FLASH提速。

/*****************************************************************************
 * Module    : DAC配置控制
 ******************************************************************************/
#define DAC_CH_SEL                      xcfg_cb.dac_sel      // DAC_MONO ~ DAC_VCMBUF_DUAL
#define DAC_FAST_SETUP_EN               0                    // DAC快速上电，有噪声需要外部功放MUTE
#define DAC_MAX_GAIN                    xcfg_cb.dac_max_gain // 配置DAC最大模拟增益，默认设置为dac_vol_table[VOL_MAX]
#define DAC_OUT_SPR                     DAC_OUT_48K          // dac out sample rate
#define DAC_LDOH_SEL                    xcfg_cb.dac_ldoh_sel
#define DAC_VCM_CAPLESS_EN              xcfg_cb.dac_vcm_less_en  // DAC VCM省电容方案,使用内部VCM
#define DACVDD_BYPASS_EN                xcfg_cb.dacvdd_bypass_en // DACVDD Bypass
#define DAC_PULL_DOWN_DELAY             80                       // 控制DAC隔直电容的放电时间, 无电容时可设为0，减少开机时间。
#define DAC_DNR_EN                      1                        // 是否使能动态降噪

#define EXLIB_SOFT_EQ_DRC_EN            0 // 双声道(软件PRE_EQ(建议不超过8段EQ) =>音量调节=> DRC(1段)=>硬件EQ(POST_EQ))音频链路的实现 //系统时钟需要设置到160M  //该功能比较耗CPU,不支持同时开如KARAOK,高码率/无损音乐播放等本身就比较耗CPU的功能。
#define EXLIB_BT_MONO_XDRC_EN           0 // 蓝牙模式下实现(10段软件EQ => 高低频DRC(2段DRC)=>总DRC ==>硬件EQ) //双声道合并成单声道后,再过该算法链路

#define DYNAMIC_BASS_BOOST_EN           0 // 低音增强(音量越低,低音效果越强,暂只支持16级音量),系统音量调的是EQ前置增益
#define DYNAMIC_BASS_BOOST_ADD_PEAK     0 // 在DYNAMIC_BASS_BOOST_EN功能基础上,再增加一段类似的PEAK调节,音量越小,PEAK值越大。需要先打开DYNAMIC_BASS_BOOST_EN才有效
#define DAC_OFF_FOR_BT_CONN_EN          0 // 连接BT时暂时关闭DAC，防止因为RF不稳定导致的杂音从DAC输出
/*****************************************************************************
 * Module    : 音乐功能配置
 *****************************************************************************/
#define MUSIC_UDISK_EN                  1 // 是否支持播放UDISK
#define MUSIC_SDCARD_EN                 1 // 是否支持播放SDCARD
#define MUSIC_SDCARD1_EN                0 // 是否支持播放双卡

#define MUSIC_WAV_SUPPORT               1 // 是否支持WAV格式解码
#define MUSIC_WMA_SUPPORT               0 // 是否支持WMA格式解码
#define MUSIC_APE_SUPPORT               0 // 是否支持APE格式解码
#define MUSIC_FLAC_SUPPORT              0 // 是否支持FLAC格式解码
#define MUSIC_SBC_SUPPORT               0 // 是否支持SBC格式解码(SD/UDISK的SBC歌曲, 此宏不影响蓝牙音乐)

#define MUSIC_FOLDER_SELECT_EN          1 // 文件夹选择功能
#define MUSIC_AUTO_SWITCH_DEVICE        1 // 双设备循环播放
#define MUSIC_BREAKPOINT_EN             1 // 音乐断点记忆播放
#define MUSIC_AB_BREAKPOINT_EN          1 // AB断点记忆播放，需在MUSIC_BREAKPOINT_EN使能的情况下才能使用
#define MUSIC_QSKIP_EN                  1 // 快进快退功能
#define MUSIC_PLAYMODE_NUM              4 // 音乐播放模式总数
#define MUSIC_MODE_RETURN               1 // 退出音乐模式之后是否返回原来的模式
#define MUSIC_PLAYDEV_BOX_EN            1 // 是否显示“USB”, "SD"界面
#define MUSIC_ID3_TAG_EN                0 // 是否获取MP3 ID3信息
#define MUSIC_REC_FILE_FILTER           0 // 是否区分录音文件与非录音文件分别播放
#define MUSIC_LRC_EN                    0 // 是否支持歌词显示
#define MUSIC_NAVIGATION_EN             0 // 音乐文件导航功能(LCD点阵屏功能)
#define MUSIC_ENCRYPT_EN                0 // 是否支持加密MP3文件播放(使用MusicEncrypt.exe工具进行MP3加密)
#define MUSIC_MP3_LOOPBACK_EN           0 // 是否开启MP3音乐单曲无缝循环播放
#define USB_CUSTOM_DETECT               0 // 是否支持假USB检测（DP短路检测，DP DM短接检测）
#define MUSIC_SD_LOOPBACK_EN            0 // SD卡音乐循环功能

#define MUSIC_ENCRYPT_KEY               12345 // MusicEncrypt.exe工具上填的加密KEY
#define WAV_SRC1_MIX_PLAY_EN            0     // WAV(PCM,16K,单声)推SRC1 混合正常音乐SRC0播放 //不支持同时打开KARAOK(KARAOK MIC也会用到SRC1)

#define IPHONE_POWER_VAL                50  // 苹果充电电流设置
#define IPHONE_POWER_INDEX              190 // 苹果充电电流设置

/*****************************************************************************
 * Module    : 蓝牙功能配置
 *****************************************************************************/
#define BT_BACKSTAGE_EN                 1        // 蓝牙后台管理（全模式使用蓝牙(fmrx不能蓝牙后台)）
#define BT_BACKSTAGE_PLAY_DETECT_EN     0        // 非蓝牙模式下检测到手机蓝牙播放音乐，则切换到蓝牙模式
#define BT_NAME_DEFAULT                 "BT-BOX" // 默认蓝牙名称（不超过31个字符）
#define BT_NAME_WITH_ADDR_EN            0        // 蓝牙名称是否附加地址信息（调试用，例如：btbox-***）
#define BT_POWER_UP_RECONNECT_TIMES     3        // 上电回连次数
#define BT_TIME_OUT_RECONNECT_TIMES     20       // 掉线回连次数
#define BT_SIMPLE_PAIR_EN               1        // 是否打开蓝牙简易配对功能（关闭时需要手机端输入PIN码）
#define BT_DISCOVER_CTRL_EN             0        // 是否使用按键打开可被发现（按一下按键才能被连接配对）
#define BT_PWRKEY_5S_DISCOVER_EN        0        // 是否使用长按5S开机进入可被发现(耳机长按开机功能)
#define BT_DISCOVER_TIMEOUT             100      // 按键打开可被发现后，多久后仍无连接自动关闭，0不自动关闭，单位100ms
#define BT_ANTI_LOST_EN                 0        // 是否打开蓝牙防丢报警
#define BT_BQB_RF_EN                    0        // 蓝牙RF BR/EDR BQB测试
#define BT_CBT_TEST_EN                  0        // 蓝牙CBT测试使能
#define BT_FCC_TEST_EN                  0        // 蓝牙FCC测试使能   //默认PB3 波特率1500000通信
#define BT_LOCAL_ADDR                   0        // 蓝牙是否使用本地地址，0使用配置工具地址

#define BT_2ACL_EN                      0 // 是否支持连接两部手机
#define BT_A2DP_EN                      1 // 是否打开蓝牙音乐服务
#define BT_HFP_EN                       0 // 是否打开蓝牙通话服务
#define BT_HSP_EN                       0 // 是否打开蓝牙HSP通话服务
#define BT_PBAP_EN                      0 // 是否打开蓝牙电话簿服务
#define BT_MAP_EN                       0 // 是否打开蓝牙短信服务(用于获取设备时间，支持IOS/Android)

#define BT_SPP_EN                       0               // 是否打开蓝牙串口服务
#define BT_HID_EN                       0               // 是否打开蓝牙HID服务
#define BT_HID_TYPE                     HID_TYPE_PHOTO1 // 选择HID服务类型
#define BT_HID_MENU_EN                  0               // 蓝牙HID是否需要手动连接/断开
#define BT_HID_DISCON_DEFAULT_EN        0               // 蓝牙HID服务默认不连接，需要手动进行连接。
#define BT_HID_SIMPLE_KEYBOARD          0               // 蓝牙HID，枚举成键盘，做音量加减，播放/暂停，上下曲切歌和唤醒语音助手 (需要把 BT_HID_TYPE 配置为3)
#define BT_HFP_CALL_PRIVATE_EN          0               // 是否使能私密接听与蓝牙接听切换功能
#define BT_HFP_CALL_PRIVATE_FORCE_EN    0               // 是否强制使用私密接听（手机端接听）
#define BT_HFP_RING_NUMBER_EN           0               // 是否支持来电报号
#define BT_HFP_PLAY_IOS_RING_EN         0               // 是否支持ios来电铃声（android默认用RING提示音）
#define BT_HFP_BAT_REPORT_EN            0               // 是否支持电量显示
#define BT_HFP_MSBC_EN                  0               // 是否打开宽带语音功能
#define BT_HSF_CALL_NUMBER_EN           0               // 是否支持获取回拔,主动拨出电话的号码
#define BT_A2DP_AAC_AUDIO_EN            0               // 是否支持蓝牙AAC音频格式
#define BT_A2DP_VOL_CTRL_EN             1               // 是否支持A2DP音量与手机同步
#define BT_A2DP_VOL_HID_CTRL_EN         0               // 是否打开HID调A2DP音量与手机同步功能，同时需打开HID拍照功能，音量同步功能
#define BT_A2DP_RECON_EN                0               // 是否支持A2DP控制键（播放/暂停、上下曲键）回连
#define BT_AVDTP_DELAY_REPORT_EN        1               // 是否上报耳机当前延迟给手机进行音视频同步
#define BT_SCO_DBG_EN                   0               // 是否打开无线SPP调试通话参数功能
#define BT_CONNECTED_AUTO_PLAY_EN       0               // 是否打开蓝牙连接后自动播放音乐功能
#define BT_ID3_TAG_EN                   0               // 是否获取蓝牙ID3信息,蓝牙ID3信息会从bt_id3_tag_callback函数中输出
#define BT_TOUCH_SCREEN_EN              0               // 车机HID 触摸屏 (传统蓝牙HID, 只支持苹果)
#define BT_CARPLAY_EN                   0               // 是否使能车机carplay功能
#define BT_SUPPORT_EIGHT_DEVICE_EN      0               // 是否支持8台设备信息保存
#define BT_SRCRAM_PHASECOM_SYNC         0               // 蓝牙同步方式使用SRCRAM_PHASECOM

#define BT_TWS_EN                       0 // 是否支持TWS    //注意使用不带TWS的蓝牙库时,需要关闭该TWS的宏,否则会有死机情况
#define BT_TSCO_EN                      0 // 是否支持TWS双路通话  //bt_is_force_master函数需要返回true
#define BT_TWS_FAST_PAIR_EN             0 // 是否支持TWS快速配对（注：快速配对时需要修改为通过蓝牙名字配对）
#define BT_TWS_PAIR_MODE                0 // 0=通过蓝牙名字配对，1=通过ID配对
#define BT_TWS_PAIR_ID                  0x38383730
#define BT_TWS_LTX_EN                   0 // 是否降低搜索功率
#define BT_TWS_TIME_OUT_RECONNECT_TIMES 1 // TWS 掉线回连次数,1大约5.12S，0XFF一直回连
#define BT_TWS_SLAVE_ID3                0 // BT ID3 信息是否要传给副机

#define BT_RF_EXT_CTL_EN                0 // 是否外接BT RF LAN 放大器
#define BT_RF_POWER_BALANCE_EN          0 // BT RF功耗配平,降低蓝牙RF TX/RX耗电不均引起的DAC噪声,但芯片耗电会加大  //默认使用了 //PA4/PA3/PF1/PB0/PB5 这五个IO口 (SSOP24/SOP16 没有用到的GPIO)

#define LE_EN                           0 // 是否打开BLE功能  打开ble需要同步替换带BLE的库 libbtstack_ble_no_tws.a 或 libbtstack_tws_ble.a (改名为libbtstack.a再编译),相关库的描述见libs/libbtstack_Readme.txt
#define LE_PAIR_EN                      0 // 是否使能BLE的加密配对

// 独立FOTA功能配置
// 独立FOTA主要用于第三方APP支持.fot文件格式，方便客户APP支持FOTA功能
// 独立FOTA，可通过wiki上的ab-ota-demo APP进行演示（可申请源码）
#define LE_AB_FOT_EN                    0 // 是否打开BLE协议的独立FOTA服务，一般IOS使用（需要配合BLE APP使用）
#define BT_AB_FOT_EN                    0 // 是否支持SPP协议的独立FOTA服务，一般Android使用（需要配合SPP APP使用）

#define LE_ADV0_EN                      0 // 是否打开无连接广播功能
#define LE_WIN10_POPUP                  0 // 是否打开win10 swift pair快速配对

#define USB_BT_NAME_UPDATA              0 // 打开U盘升级蓝牙名字功能文件名：bt_name.btn

/*****************************************************************************
 * Module    : 通话功能配置
 *****************************************************************************/
// 通话参数
#define BT_SCO_DUMP_EN                  0 // 是否通过HART DUMP通话数据, 需要打开HUART调EQ功能

#define BT_PLC_EN                       1
#define BT_NOISE_THRESHOID              xcfg_cb.bt_noise_threshoid // 环境噪声阈值（低于此阈值便认为是噪声）(0~65535)
#define BT_ANL_GAIN                     xcfg_cb.bt_anl_gain        // MIC模拟增益(0~23)
#define BT_DIG_GAIN                     xcfg_cb.bt_dig_gain        // MIC数字增益(0-31),且于0~3DB范围细调,步进3/32DB
#define BT_CALL_MAX_GAIN                xcfg_cb.bt_call_max_gain   // 配置通话时DAC最大模拟增益

#define BT_AEC_EN                       0
#define BT_ECHO_LEVEL                   xcfg_cb.bt_echo_level // 回声消除级别（级别越高，回声衰减越明显，但通话效果越差）(0~15)
#define BT_FAR_OFFSET                   xcfg_cb.bt_far_offset // 远端补偿值(0~255)
#define AEC_DUMP_FILE                   0

#define BT_ALC_EN                       1                        // 是否使能ALC
#define BT_ALC_FADE_IN_DELAY            xcfg_cb.bt_alc_in_delay  // 近端淡入延时
#define BT_ALC_FADE_IN_STEP             xcfg_cb.bt_alc_in_step   // 近端淡入速度
#define BT_ALC_FADE_OUT_DELAY           xcfg_cb.bt_alc_out_delay // 远端淡入延时
#define BT_ALC_FADE_OUT_STEP            xcfg_cb.bt_alc_out_step  // 远端淡入速度
#define BT_ALC_VOICE_THR                0x50000

#define BT_SCO_FAR_NR_EN                0   // 是否打开远端降噪算法
#define BT_SCO_FAR_NOISE_THRESHOID      600 // 远端环境噪声阈值（低于此阈值便认为是噪声）(0~65535)

#define SCO_MAV_EN                      0 // 蓝牙通话魔音使能，需要关闭BT_HFP_MSBC_EN，配置中选ALC通话

#define BT_AEC_DUMP_EN                  0 // 是否打开aec dump数据功能      //alc和aec导出数据,只能打开其中一个,dump出远、近端原始数据，近端降噪消音后数据
#define BT_ALC_DUMP_EN                  0 // 是否打开alc dump数据功能      //alc和aec导出数据,只能打开其中一个

/*****************************************************************************
 * Module    : FMRX功能配置
 *****************************************************************************/
#define FMRX_HALF_SEEK_EN               0 // 是否打开半自动搜台

#define FMRX_THRESHOLD_VAL              xcfg_cb.fmrx_r_val    // 内置FMRX搜台阈值(0~255), 值越小台越多，假台也可能增多。 // 128
#define FMRX_THRESHOLD_Z                xcfg_cb.fmrx_z_val    // 该值越大台越多, 想减少很弱的台，可以适当改小            //1100
#define FMRX_THRESHOLD_FZ               xcfg_cb.fmrx_fz_val   // 该值越大台越多, 想减少很弱的台，可以适当改小            //600
#define FMRX_THRESHOLD_D                xcfg_cb.fmrx_d_val    // 3000
#define FMRX_AUDIO_CHANNEL              xcfg_cb.fmrx_audio_ch // FM声道输出声道,一般单声道比双声道噪音会小些  //0 Mono  //1 Dual

#define FMRX_OPTIMIZE_TRY               0 // FM 收台效果尝试优化,可以修改CLK控制等,需要实际样机去测试效果
#define FMRX_TEST_CHANNEL               0 // FM 固定某些电台测试,可用于对比其它样机,定位到特定的一些台对比声音清晰度

/*****************************************************************************
 * Module    : FMTX功能配置(只支持PA6, 只有AB5602F芯片才支持)
 *****************************************************************************/
#define FMTX_EN                         0           // 是否使能FM TX功能
#define FMTX_POWER                      2           // 发射功率选择(0 ~ 3共4档, 发射POWER依次增强)
#define FMTX_DAC_EN                     1           // 是否同时DAC输出
#define FMTX_AMP_EN                     0           // 是否经过放大器输出
#define FMTX_PUD_OUT                    1           // 是否通过上下拉电阻方式输出
#define FMTX_DNR_EN                     0           // 是否打开FMTX动态降噪
#define FMTX_CHANNEL_MODE               FMTX_STEREO // FMTX输出声道模式
#define FMTX_AMP_TIMES                  FMTX_VOL_X2 // 设置FMTX AUDIO放大倍数

/*****************************************************************************
 * Module    : AUX功能配置
 ******************************************************************************/
#define AUX_CHANNEL_CFG                 (CH_AUXL_PB1 | CH_AUXR_PB2) // 选择LINEIN通路
#define MIC_CHANNEL_CFG                 CH_MIC_PF2                  // 选择MIC的通路
#define AUX_2_SDADC_EN                  1                           // AUX是否进SDADC, 否则直通DAC。进SDADC可以调AUX EQ, 及AUX录音等功能
#define AUX_SNR_EN                      0                           // AUX模式动态降噪(AUX模拟直通也能用)
#define LINEIN_DETECT_EN                1                           // 是否打开LINEIN检测
#define SDCMD_MUX_DETECT_LINEIN         0                           // 是否复用SDCMD检测LINEIN插入
#define LINEIN_2_PWRDOWN_EN             0                           // 是否插入Linein后直接软关机（大耳包功能）
#define AUX_CHANNEL_EXCHANGE            0                           // 是否AUX左右声道互换

#define MICAUX_ANALOG_OUT_ALWAYS        0                                   // 所有模式下均打开AUX或MIC通道
#define MICAUX_ANALOG_OUT_CH            (CH_AUXR_MIC_PF2 | CH_AUXL_MIC_PF2) // 选择直通MIC通道 eg: MICL TO AUXL/AUXR

/// 通过配置工具选择检测GPIO
#define LINEIN_DETECT_INIT()            linein_detect_init()
#define LINEIN_IS_ONLINE()              linein_is_online()
#define IS_DET_LINEIN_BUSY()            is_detect_linein_busy()

/*****************************************************************************
 * Module    : HDMI ARC配置
 *****************************************************************************/
#define HDMI_DETECT_EN                  0 // 是否开启HDMI插入检测
// other config see "bsp_hdmi.c"

/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#define FUNC_REC_EN                     0       // 录音功能总开关
#define FMRX_REC_EN                     0       // 是否打开FM录音功能
#define AUX_REC_EN                      0       // 是否打开AUX录音功能
#define MIC_REC_EN                      0       // 是否打开MIC录音功能
#define TOM_CAT_REC_EN                  0       // 是否打开TOM_CAT录音功能
#define BT_REC_EN                       0       // 是否打开蓝牙音乐录音功能
#define BT_HFP_REC_EN                   0       // 是否打开蓝牙通话录音功能（不支持karaok）
#define KARAOK_REC_EN                   0       // 是否打开KARAOK录音功能，需同步打开KARAOK功能
#define REC_ONE_FOLDER_EN               0       // 是否各模式录音放到同一目录下
#define REC_DIR_LFN                     1       // 是否打开长文件名目录
#define REC_AUTO_PLAY                   0       // 录音结束是否回放当前录音内容
#define REC_FAST_PLAY                   1       // 播卡播U下快速播放最新的录音文件(双击REC)
#define REC_STOP_MUTE_1S                0       // 录音停止时, MUTE 1S功放. //提醒客户录音结束.
#define REC_TYPE_SEL                    REC_MP3 // 注意REC_ADPCM格式只支持通话录音
#define REC_DIG_GAIN_EN                 0       // 录音是否需要加大数字增益
#define REC_WHITOUT_KARAOK              0       // 录音退出karaok，ram不够
#define MUSIC_REC_MIC_EN                0       // 是否打开MUSIC模式下录音MIC功能（需要开启 FUNC_SPEAKER_EN 和 MIC_REC_EN）

/*****************************************************************************
 * Module    :外接SPIFLASH配置, 外接SPIFLASH可以播放MP3音乐文件, 及录音
 *****************************************************************************/
#define EX_SPIFLASH_SUPPORT             0 // 可以配置为 EXSPI_NOT_SUPPORT(0) 或 EXSPI_MUSIC 或 EXSPI_REC 或 (EXSPI_MUSIC | EXSPI_REC)
#define FLASH_GREATER_THAN_16MBYTE      0
#define SPLFLASH_MP3_LOOPBACK_EN        0 // 是否开启SPLFLASH_MP3音乐单曲无缝循环播放
#define SPIFALSH_MUSIC_BIN_WRITE_TEST   0 // 默认的FLASH_MUSIC.BIN写入SPIFLASH, 可以在func_exspifalsh_music中测试外接SPIFALSH播放MP3.

// FLASH_MUSIC.BIN 镜像文件占用区域(BYTE)
#define SPIFLASH_MUSIC_BEGIN_ADDR       0           // FLASH_MUSIC.BIN镜像文件默认从0地址开始存放,此宏暂不支持修改.
#define SPIFLASH_MUSIC_END_ADDR         (1024 * 36) // FLASH_MUSIC.BIN镜像文件结束地址. 测试DEMO的镜像文件是36K大小.

// 录音占用区域(BYTE)  //注意SPIFALSH的录音区域不要覆盖 FLASH_MUSIC.BIN区域
#define SPIFLASH_REC_BEGIN_ADDR         (1024 * 37)   // 录音起始地址
#define SPIFLASH_REC_END_ADDR           (1024 * 1024) // 录音结束地址

// SPIFLASH录音配置
#define EXSPI_REC_PITCH_SHIFT_PLAY      0  // SPEAKER下的SPIFLASH录音变调播放 //类似TOM猫效果
#define INTERNAL_FLASH_REC              0  // 录音到内部flash
#define REC_DELETE_FILE                 0  // flash录音自动覆盖
#define REC_AUTO_END_EN                 0  // flash录音自动停止功能
#define REC_AUTO_END_TIME               10 // flash自动停止录音时间(s)

#define SPIFALSH_MUSIC_PLAY_REC         0 // 1 FUNC_EXSPIFLASH_MUSIC 模式下播放录音示例： 支持录音上下曲，及删除当前播放的录音  //0 播放镜像音乐示例
#define SPIFALSH_MUSIC_BREAK_MEMORY     0 // 断点记忆，切出flash模式时记忆断点，再次切入flash播放时从上次记忆的断点开始播放，加入100ms断电记忆一次功能
#define SPIFLASH_MUSIC_BREAK_LOOP       1 // 0-打开断点记忆时,对当前音乐循环播放 1-打开断点记忆时,对Flash中的所有歌曲循环播放
#define EXFLASH_MUSIC_WARNING           0 // 播flash音乐时同时播放wav提示音

/*****************************************************************************
 * Module    : TOM_CAT功能配置
 *****************************************************************************/
#define TOM_CAT_STOP_TIME               1000 // 录音结束检测时间(ms)，1秒没检测到声音录音结束
#define TOM_CAT_CUT_TIME                500  // 录音文件尾部裁剪(ms)

// 起始地址不要和代码段冲突，地址要4K对齐，最后要留20K给记忆区
#define TOM_CAT_SPK_REC_START_ADDR      (512 * 1024)  // 声控录音起始地址
#define TOM_CAT_SPK_REC_END_ADDR        (768 * 1024)  // 声控录音结束地址
#define TOM_CAT_KEY_REC_START_ADDR      (768 * 1024)  // 按键录音起始地址
#define TOM_CAT_KEY_REC_END_ADDR        (1000 * 1024) // 按键录音结束地址

/*****************************************************************************
 * Module    : K歌功能配置
 *****************************************************************************/
#define SYS_KARAOK_EN                   0  // 是否打开K歌功能（暂不支持与录音、WMA、FLAC同时打开）
#define BT_HFP_CALL_KARAOK_EN           0  // 通话是否支持KARAOK.
#define SYS_ECHO_EN                     0  // 是否使能混响功能
#define SYS_ECHO_DELAY                  16 // 混响间隔级数
#define SYS_BASS_TREBLE_EN              0  // 是否使能高低音调节
#define SYS_MAGIC_VOICE_EN              0  // 是否使能魔音功能,不支持同时开启BT_HFP_CALL_KARAOK_EN宏
#define SYS_HOWLING_EN                  0  // 是否使能防啸叫功能
#define SYS_PITCH_SHIFT                 0  // 是否使能新的魔音算法,要更改链接文件才能支持魔音, 可与SYS_MAGIC_VOICE_EN搭配使用，择优

#define MIC_DETECT_EN                   0  // 是否使能MIC检测
#define MIC_DNR_EN                      0  // 是否使能MIC动态降噪
#define KARAOK_DENOISE_EN               0  // 是否使能高级版本MIC动态降噪,主要优化MIC_DNR_EN效果不明显,可以和MIC_DNR_EN同时使用,目前主要用于KARAOK MIC降噪
#define KARAOK_DENOISE_LEVEL            16 // 降噪级别，值越大降噪效果越明显,声音还原度会相应变差. 取值范围0~31
#define KARAOK_MIC_DRC                  0  // 是否使能mic的drc功能，开启后需要拉sdadc.drc文件曲线（不支持在线调节）。//后置数字增益也受此宏控制

#define KARAOK_RM_VOICE                 0 // 蓝牙音乐或播放mp3音乐消人声
#define AUDIO_STRETCH_EN                0 // 变速不变调 buf与混响共用           加速模式下主频需要120M 且要关闭K歌和魔音宏开关

#define KARAOK_MIX_PLAY_WAV_EN          0 // PCM资源文件混到KARAOK_MIC数据流中一起播放出来(常用于开KARAOK时,该WAV提示音与KARAOK一起输出)

/// 通过配置工具选择检测GPIO
#define MIC_DETECT_INIT()               mic_detect_init()
#define MIC_IS_ONLINE()                 mic_is_online()
#define IS_DET_MIC_BUSY()               is_detect_mic_busy()

/*****************************************************************************
 * Module    : User按键配置 (可以同时选择多组按键)
 *****************************************************************************/
#define USER_ADKEY                      0 // ADKEY的使用， 0为不使用
#define USER_ADKEY2                     0 // ADKEY2的使用，0为不使用
#define USER_PWRKEY                     1 // PWRKEY的使用，0为不使用
#define USER_IOKEY                      0 // IOKEY的使用， 0为不使用
#define USER_ADKEY_MUX_SDCLK            0 // 是否使用复用SDCLK的ADKEY, 共用USER_ADKEY的按键table
#define USER_ADKEY_MUX_LED              0 // 是否使用ADKEY与LED复用, 共用USER_ADKEY的流程(ADKEY与BLED配置同一IO)
#define ADKEY_PU10K_EN                  0 // ADKEY是否使用内部10K上拉, 按键数量及阻值见port_key.c

#define USER_KEY_KNOB_EN                0  // 旋钮的使用，0为不使用
#define USER_KEY_KNOB_LEVEL             16 // 旋钮的级数

#define USER_MULTI_PRESS_EN             0                                        // 按键N击检测使能
#define USER_KEY_DOUBLE                 {KU_HSF, KU_REC, KU_PLAY_POWER, KU_PLAY} // 支持双击/三击/四击/五击的按键
#define ADKEY_CH                        xcfg_cb.adkey_ch                         // ADCCH_PE7
#define ADKEY2_CH                       xcfg_cb.adkey2_ch                        // ADCCH_PE6

#define IS_PWRKEY_PRESS()               (0 == (RTCCON & BIT(19)))
#define DOUBLE_KEY_TIME                 (xcfg_cb.double_key_time)           // 按键双击响应时间（单位50ms）
#define PWRON_PRESS_TIME                (500 * xcfg_cb.pwron_press_time)    // 长按PWRKEY多长时间开机？
#define PWROFF_PRESS_TIME               (3 + 3 * xcfg_cb.pwroff_press_time) // 长按PWRKEY多长时间关机？

// 上电快速检测ADC,需要把header_adc.bin修改成header.bin并重新编译,目前支持PWRKEY,PB2,PB4,检测结果放在地址0x58400的RAM中供后面的程序使用
#define PWRON_QUICK_ADC_PRINT           0 // 上电快速检测ADC,主要用于PWREKY短按唤醒并得到按键的ADC用于区分按键

/*****************************************************************************
 * Module    : TouchKey按键配置
 *****************************************************************************/
#define USER_TKEY                       0          // TouchKEY总开关，0为不使用
#define USER_TKEY_DEBUG_EN              0          // TouchKey SPP调试
#define USER_TKEY_HW_EN                 0          // TouchKEY使用硬件方法,支持一路
#define TKEY_CH                         TK_PE7_CH3 // 硬件通道选择

#define USER_TKEY_MUL_EN                0 // TouchKEY使用软件方法,支持四路,和USER_TKEY_HW_EN同时开启时优先级更高
#define TKEY_CH0_EN                     0 // PB1作tkey开关,需要开启TouchKEY,开启时注意引脚复用
#define TKEY_CH1_EN                     0 // PB2作tkey开关,需要开启TouchKEY
#define TKEY_CH2_EN                     0 // PE6作tkey开关,需要开启TouchKEY,和IR脚配置成PE6冲突，不能同时开启
#define TKEY_CH3_EN                     0 // PE7作tkey开关,需要开启TouchKEY,和ADKEY冲突，不能和USER_ADKEY同时开启

/*****************************************************************************
 * Module    : SD0配置
 *****************************************************************************/
#define SD_SOFT_DETECT_EN               0         // 是否使用软件检测（SD发命令检测）
#define SDCLK_MUX_DETECT_SD             1         // 是否复用SDCLK检测SD卡
#define SD0_MAPPING                     SD0MAP_G2 // 选择SD0 mapping
#define SD1_MAPPING                     SD0MAP_G3 // 选择SD1 mapping

#define SD_INSERT_EXCEPTION_RESET       0 // SD插入小概率引起复位时,复位后(跳过开机提示音)直接进入音乐模式播放
/// 通过配置工具选择检测GPIO
#define SD_DETECT_INIT()                sdcard_detect_init()
#define SD_IS_ONLINE()                  sdcard_is_online()
#define IS_DET_SD_BUSY()                is_det_sdcard_busy()
#define SD_IS_SOFT_DETECT()             (xcfg_cb.sddet_iosel == 31) // 配置工具中选则31是软件检测

#define SD1_DETECT_INIT()               sdcard1_detect_init()
#define SD1_IS_ONLINE()                 sdcard1_is_online()
#define IS_DET_SD1_BUSY()               is_det_sdcard1_busy()

/*****************************************************************************
 * Module    : I2C配置
 *****************************************************************************/
#define I2C_EN                          0 // 是否使能I2C功能
#define I2C_MUX_SD_EN                   1 // 是否I2C复用SD卡的IO

#if I2C_MUX_SD_EN
    #define I2C_SCL_IN()   SD_CMD_DIR_IN()
    #define I2C_SCL_OUT()  SD_CMD_DIR_OUT()
    #define I2C_SCL_H()    SD_CMD_OUT_H()
    #define I2C_SCL_L()    SD_CMD_OUT_L()

    #define I2C_SDA_IN()   SD_DAT_DIR_IN()
    #define I2C_SDA_OUT()  SD_DAT_DIR_OUT()
    #define I2C_SDA_H()    SD_DAT_OUT_H()
    #define I2C_SDA_L()    SD_DAT_OUT_L()
    #define I2C_SDA_IS_H() SD_DAT_STA()
#else
    #define I2C_SCL_IN()        \
        {                       \
            GPIOEDIR |= BIT(5); \
            GPIOEPU |= BIT(5);  \
        }
    #define I2C_SCL_OUT()        \
        {                        \
            GPIOEDE |= BIT(5);   \
            GPIOEDIR &= ~BIT(5); \
        }
    #define I2C_SCL_H()        \
        {                      \
            GPIOESET = BIT(5); \
        }
    #define I2C_SCL_L()        \
        {                      \
            GPIOECLR = BIT(5); \
        }

    #define I2C_SDA_IN()        \
        {                       \
            GPIOEDIR |= BIT(7); \
            GPIOEPU |= BIT(7);  \
        }
    #define I2C_SDA_OUT()        \
        {                        \
            GPIOEDE |= BIT(7);   \
            GPIOEDIR &= ~BIT(7); \
        }
    #define I2C_SDA_H()        \
        {                      \
            GPIOESET = BIT(7); \
        }
    #define I2C_SDA_L()        \
        {                      \
            GPIOECLR = BIT(7); \
        }
    #define I2C_SDA_IS_H() (GPIOE & BIT(7))
#endif // I2C_MUX_SD_EN

#define I2C_SDA_SCL_OUT() \
    {                     \
        I2C_SDA_OUT();    \
        I2C_SCL_OUT();    \
    }
#define I2C_SDA_SCL_H() \
    {                   \
        I2C_SDA_H();    \
        I2C_SCL_H();    \
    }

/*****************************************************************************
 * Module    : I2S配置
 *****************************************************************************/
#define I2S_EN           0          // 是否使能I2S功能
#define I2S_DEVICE       I2S_DEV_NO // I2S设备选择
#define I2S_MAPPING_SEL  I2S_GPIOA  // I2S IO口选择
#define I2S_MODE_SEL     0          // I2S主从模式选择 0: master; 1:slave
#define I2S_BIT_MODE     0          // I2S数据位宽选择 0:16bit; 1:32bit
#define I2S_DATA_MODE    1          // I2S数据格式选择 0:left-justified mode; 1:normal mode
#define I2S_DMA_EN       0          // I2S数据源选择 0:src; 1:dma
#define I2S_MCLK_EN      1          // I2S是否打开MCLK
#define I2S_MCLK_SEL     2          // I2S MCLK选择 0:64fs 1:128fs 2:256fs
#define I2S_PCM_MODE     0          // I2S是否打开PCM mode

#define IIS_EXT_EN       0 // IIS寄存器配置全部开源，有更灵活需求可以在这里面做

/*****************************************************************************
 * Module    : HUART配置
 *****************************************************************************/
#define HUART_AUDIO_EN   0            // 是否使能Huart功能
#define HUART_AUDIO_BAUD 1500000      // haurt波特率
#define HUART_SAMPLES    120 * 2      // 16bit = 8bit * 2
#define HUART_RX_PORT    HUART_TR_PB1 // huart接收引脚
#define HUART_TX_PORT    HUART_TR_PB2 // huart发射引脚
#define DAC_IN_SAMPLE    SPR_48000    // DAC输入采样率

/*****************************************************************************
 * Module    : SPDIF配置
 *****************************************************************************/
#define FUNC_SPDIF_TX_EN 0 // 是否打开SPDIF TX功能
#define SPDIF_TX_CH      SPF_PF1_CH4
#define SPDIF_IN_CH      SPF_PF1_CH4
#define SPDIF_IN_SHAPE   0 // 是否打开整形直通输出功能,SPDIF in信号通过整形电路后输出到PB5

/*****************************************************************************
 * Module    : IRRX配置
 *****************************************************************************/
#define IRRX_HW_EN       0   // 是否打IRRX硬件模块
#define IRRX_SW_EN       0   // 是否打开timer capture ir
#define IR_NUMKEY_EN     0   // 是否打开数字键输入
#define IR_INPUT_NUM_MAX 999 // 最大输入数字9999

// 可以打开一个或多个
#define IR_ADDR_FF00_EN  1
#define IR_ADDR_BF00_EN  0
#define IR_ADDR_FD02_EN  0
#define IR_ADDR_FE01_EN  0
#define IR_ADDR_7F80_EN  0

#define IR_CAPTURE_PORT()   \
    {                       \
        GPIOEDE |= BIT(6);  \
        GPIOEPU |= BIT(6);  \
        GPIOEDIR |= BIT(6); \
    }
#define IRRX_MAPPING                 IRMAP_PE6
#define TMR3CAP_MAPPING              TMR3MAP_PE6

/*****************************************************************************
 * Module    : usb device 功能选择
 *****************************************************************************/
#define UDE_STORAGE_EN               0
#define UDE_SPEAKER_EN               0
#define UDE_HID_EN                   0
#define UDE_MIC_EN                   0
#define UDE_STOAGE_FLASH_EN          0 // 外挂flash虚拟U盘
#define UDE_MIC_KARAOK_EN            0 // USB_DEV karaok

#define UDE_MIC_VOLUME_EN            0 // 电脑端ude mic音量设置大小
#define USB_MIC_COMPATIBILITY_XP_SYS 0 // USB_MIC兼容xp系统
#define USB_DETECT_ONLY_CHECK_HOST   0 // USB插入检测的新机制      //用于只检测主机,U盘检测功能可能会受影响

/*****************************************************************************
 * Module    : 系统细节配置
 *****************************************************************************/
#define PWM_OUT_EN                   0          // 设置定时器5 (PA5、PA6、PA7)PWM输出
#define RGB_WS2812_EN                1          // ★ WS2812 灯带（SPI1 G4: PE7→DIN）
#define RGB_SERIAL_EN                0          // 串行RGB推灯功能
#define PWM_RGB_EN                   0          // PWM RGB三色灯功能
#define ENERGY_LED_EN                0          // 能量灯软件PWM显示,声音越大,点亮的灯越多.
#define SYS_PARAM_RTCRAM             0          // 是否系统参数保存到RTCRAM
#define PWRON_ENTER_BTMODE_EN        1          // 是否上电默认进蓝牙模式
#define VBAT_DETECT_EN               0          // 电池电量检测功能
#define VBAT2_ADCCH                  ADCCH_VBAT // ADCCH_VBAT为内部1/2电压通路，带升压应用需要外部ADC通路检测1/2电池电压
#define VBAT_FILTER_USE_PEAK         0          // 电池检测滤波选则://0 取平均值.//1 取峰值(适用于播放音乐时,电池波动比较大的音箱方案).
#define VUSB_ADCCH                   0          // 使用VUSB做ADC检测,蓝牙模式最好关闭宏BT_POWER_OPTIMIZE，不然检测不准确
#define EQ_MODE_EN                   0          // 是否调节EQ MODE (POP, Rock, Jazz, Classic, Country)
#define MIC_EQ_EN                    0          // 是否调节MIC_EQ
#define EQ_DBG_IN_UART               0          // 是否使能UART在线调节EQ
#define EQ_DBG_IN_SPP                0          // 是否使能SPP在线调节EQ
#define EQ_DBG_DUMP                  0          // 设置DAC EQ时,导出实际EQ设置的数据,怀疑在线调EQ和最终整合的资源EQ不一致。这里导出数据进行对比
#define SYS_EQ_FOR_IDX_EN            0          // 是否使能10条EQ独立调节(包括高低音)
#define SYS_EQ_LOUD_EN               0          // 是否使能LOUD EQ独立调节（100Hz）

#define SLEEP_DAC_OFF_EN             (is_sleep_dac_off_enable()) // sfunc_sleep是否关闭DAC， 复用MICL检测方案不能关DAC。
#define SYS_INIT_VOLUME              xcfg_cb.sys_init_vol        // 系统默认音量
#define LPWR_WARNING_VBAT            xcfg_cb.lpwr_warning_vbat   // 低电提醒电压
#define LPWR_OFF_VBAT                xcfg_cb.lpwr_off_vbat       // 低电关机电压
#define LOWPWR_REDUCE_VOL_EN         1                           // 低电是否降低音量
#define LPWR_WARING_TIMES            0xff                        // 报低电次数

/*****************************************************************************
 * Module    : LED指示灯配置
 *****************************************************************************/
#define LED_DISP_EN                  1                        // 是否使用LED指示灯(蓝灯)
#define LED_PWR_EN                   1                        // 充电及电源指示灯(红灯)
#define LED_LOWBAT_EN                0                        // 电池低电是否闪红灯
#define BLED_CHARGE_FULL             xcfg_cb.charge_full_bled // 充电满是否亮蓝灯
#define BT_RECONN_LED_EN             0                        // xcfg_cb.bt_reconn_led_en //蓝牙回连状态是否不同的闪灯方式

#define LED_INIT()                   bled_func.port_init(&bled_gpio)
#define LED_SET_ON()                 bled_func.set_on(&bled_gpio)
#define LED_SET_OFF()                bled_func.set_off(&bled_gpio)

#define LED_PWR_INIT()               rled_func.port_init(&rled_gpio)
#define LED_PWR_SET_ON()             rled_func.set_on(&rled_gpio)
#define LED_PWR_SET_OFF()            rled_func.set_off(&rled_gpio)

/*****************************************************************************
 * Module    : Loudspeaker mute及耳机检测配置
 *****************************************************************************/
#define EARPHONE_DETECT_EN           0 // 是否打开耳机检测
#define SDCMD_MUX_DETECT_EARPHONE    1 // 是否复用SDCMD检测耳机插入

/// 通过配置工具选择检测GPIO
#define EARPHONE_DETECT_INIT()       earphone_detect_init()
#define EARPHONE_IS_ONLINE()         earphone_is_online()
#define IS_DET_EAR_BUSY()            is_detect_earphone_busy()

#define LOUDSPEAKER_MUTE_EN          1 // 是否使能功放MUTE
#define LOUDSPEAKER_MUTE_INIT()      loudspeaker_mute_init()
#define LOUDSPEAKER_MUTE_DIS()       loudspeaker_disable()
#define LOUDSPEAKER_MUTE()           loudspeaker_mute()
#define LOUDSPEAKER_UNMUTE()         loudspeaker_unmute()
#define LOUDSPEAKER_UNMUTE_DELAY     6 // UNMUTE延时配置，单位为5ms

#define AMPLIFIER_SEL_INIT()         amp_sel_cfg_init(xcfg_cb.ampabd_io_sel)
#define AMPLIFIER_SEL_D()            amp_sel_cfg_d()
#define AMPLIFIER_SEL_AB()           amp_sel_cfg_ab()

/*****************************************************************************
 * Module    : 充电功能选择
 *****************************************************************************/
#define CHARGE_EN                    0                           // 是否打开充电功能
#define CHARGE_OUT_POWER_ON          0                           // 充电时, 拔出充电则开机(DC不断电才支持)
#define CHARGE_TRICK_EN              xcfg_cb.charge_trick_en     // 是否打开涓流充电功能
#define CHARGE_DC_RESET              xcfg_cb.charge_dc_reset     // 是否打开DC插入复位功能
#define CHARGE_DC_NOT_PWRON          xcfg_cb.charge_dc_not_pwron // DC插入，是否软开机。 1: DC IN时不能开机
#define CHARGE_DC_IN()               charge_dc_detect()
#define SUPPORT_EXT_CHARGE_IC        0 // 外接充电

// 充电截止电流：0:2.5mA; 1:5mA; 2:10mA; 3:15mA; 4:20mA; 5:25mA; 6:30mA; 7:35mA
#define CHARGE_STOP_CURR             xcfg_cb.charge_stop_curr // 0~7
// 充电截止电压：0:4.2v；1:4.35v
#define CHARGE_STOP_VOLT             0 // 0~1
// 恒流充电（电池电压大于2.9v）电流：0: 10mA; 1: 35mA; 2:55mA; 3:80mA; 4:100mA; 5:125mA; 6:150mA; 7:170mA; 8:195mA; 9:220mA;
//                                   10:240mA; 11:265mA; 12:290mA; 13: 310mA; 14:330mA; 15:350mA
#define CHARGE_CONSTANT_CURR         xcfg_cb.charge_constant_curr
// 涓流截止电压：0:2.9v; 1:3v
#define CHARGE_TRICK_STOP_VOLT       1
// 涓流充电（电池电压小于2.9v）电流：0: 10mA 1: 20mA 2:30mA; 3:40mA
#define CHARGE_TRICKLE_CURR          xcfg_cb.charge_trickle_curr

/*****************************************************************************
 * Module    : UART_UPD功能
 *****************************************************************************/
#define UART_S_UPDATE                0 // 是否支持从机UART升级
#define UART_M_UPDATE                0 // 是否支持主机UART升级
#define UART_UPD_BAUD                115200

#define UART_RX_BIT                  BIT(3)
#define UART_TX_BIT                  BIT(4)
#define UART_UPD_PORT_SEL            (UART_RX1_G2_PA3 | UART_TX1_G2_PA4)
#define UPDATE_UART_SEL              UPDATE_UART1

/*****************************************************************************
 * Module    : 提示音 功能选择
 *****************************************************************************/
#define WARNING_TONE_EN              1                      // 是否打开提示音功能, 总开关
#define WARING_MAXVOL_MP3            1                      // 最大音量提示音WAV或MP3选择， 播放WAV可以与MUSIC叠加播放。
#define WARNING_WAVRES_PLAY          1                      // 是否支持WAV提示音播放
#define WARNING_VOLUME               xcfg_cb.warning_volume // 播放提示音的音量级数
#define LANG_SELECT                  LANG_EN                // 提示音语言选择

#define WARNING_POWER_ON             1
#define WARNING_POWER_OFF            1
#define WARNING_FUNC_MUSIC           0
#define WARNING_FUNC_BT              0
#define WARNING_FUNC_CLOCK           0
#define WARNING_FUNC_FMRX            0
#define WARNING_FUNC_AUX             0
#define WARNING_FUNC_USBDEV          0
#define WARNING_FUNC_SPEAKER         0
#define WARNING_LOW_BATTERY          0
#define WARNING_BT_WAIT_CONNECT      0
#define WARNING_BT_CONNECT           1
#define WARNING_BT_DISCONNECT        1
#define WARNING_BT_TWS_CONNECT       1
#define WARNING_BT_TWS_DISCON        0
#define WARNING_BT_INCALL            1
#define WARNING_BT_CALLOUT           1
#define WARNING_USB_SD               0
#define WARNING_MAX_VOLUME           1
#define WARNING_MIN_VOLUME           0
#define WARNING_BT_HID_MENU          0 // BT HID MENU手动连接/断开HID Profile提示音
#define WARNING_BTHID_CONN           0 // BTHID模式是否有独立的连接/断开提示音
#define WARNING_BT_PAIR              0 // BT PAIRING提示音
#define WARNING_UPDATE_DONE          1

#define SW_VERSION                   "V0.0.1" // 只能使用数字0-9,ota需要转码

#include "config_extra.h"

#endif // USER_CONFIG_H
