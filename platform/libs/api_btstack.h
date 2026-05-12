#ifndef _API_BTSTACK_H
#define _API_BTSTACK_H

//工作模式
#define MODE_NORMAL                     0
#define MODE_CBT_TEST                   1
#define MODE_FCC_TEST                   2
#define MODE_BQB_RF                     3
#define MODE_BQB_PROFILE                4

//蓝牙功能
#define PROF_A2DP                       0x0007  //蓝牙音乐功能
#define PROF_HID                        0x0018  //蓝牙键盘功能
#define PROF_HFP                        0x0060  //蓝牙通话功能
#define PROF_SPP                        0x0080  //蓝牙串口功能
#define PROF_PBAP                       0x0100  //蓝牙电话本功能
#define PROF_HSP                        0x0200
#define PROF_MAP                        0x0400  //蓝牙短息功能

//蓝牙特性
#define HFP_BAT_REPORT                  0x01    //是否支持手机电量显示
#define HFP_3WAY_CALL                   0x02    //是否支持三通电话
#define HFP_INBAND_RING_RONE            0x04    //是否支持手机来电铃声
#define HFP_CALL_PRIVATE                0x08    //是否打开强制私密接听
#define HFP_SIRI_CMD                    0x10    //是否打开siri控制命令
#define HFP_EC_AND_NR                   0x20    //是否打开手机端回音和降噪
#define HFP_RING_NUMBER_EN              0x40    //是否支持来电报号
#define A2DP_VOL_CTRL                   0x01    //是否支持手机音量控制同步
#define A2DP_IOS_PLAY_STATUS            0x02    //是否支持IOS手机播放状态同步，注意打开后微信小视频会无声，蓝牙后台建议打开
#define A2DP_ANDROID_PLAY_STATUS        0x04    //是否支持ANDROID手机播放状态同步
#define A2DP_RESTORE_PLAYING            0x08    //是否支持掉线回连后恢复播放
#define A2DP_AVDTP_DELAY_REPORT         0x10    //是否支持AVDTP delay report功能

//蓝牙编解码
#define CODEC_SBC                       0x01
#define CODEC_AAC                       0x02
#define CODEC_MSBC                      0x04
#define CODEC_PLC                       0x08

//TWS feature
#define TWS_FEAT_TSCO                   0x0001  //双耳通话
#define TWS_FEAT_TSCO_RING              0x0002  //同步铃声
#define TWS_FEAT_CODEC_MSBC             0x0004  //MSBC高清通话
#define TWS_FEAT_CODEC_AAC              0x0008  //AAC高清音频
#define TWS_FEAT_MS_SWITCH              0x0010  //主从切换

#define BT_TWS_SYNC_TO                  50      //BT同步等待，对齐到一个50ms，250ms超时

//蓝牙状态
enum {
    BT_STA_OFF,                                 //蓝牙模块已关闭
    BT_STA_INITING,                             //初始化中
    BT_STA_IDLE,                                //蓝牙模块打开，未连接

    BT_STA_SCANNING,                            //扫描中
    BT_STA_DISCONNECTING,                       //断开中
    BT_STA_CONNECTING,                          //连接中

    BT_STA_CONNECTED,                           //已连接
    BT_STA_PLAYING,                             //播放
    BT_STA_INCOMING,                            //来电响铃
    BT_STA_OUTGOING,                            //正在呼出
    BT_STA_INCALL,                              //通话中
    BT_STA_OTA,                                 //OTA升级中
};

//通话状态
enum {
    BT_CALL_IDLE,                               //
    BT_CALL_INCOMING,                           //来电响铃
    BT_CALL_OUTGOING,                           //正在呼出
    BT_CALL_ACTIVE,                             //通话中
    BT_CALL_3WAY_CALL,                          //三通电话或两部手机通话
};

//蓝牙通知
enum {
    BT_NOTICE_INIT_FINISH,                      //蓝牙初始化完成
    BT_NOTICE_CONNECT_START,                    //开始回连手机, param[0]=nouse,  param[6:1]=bd_addr
    BT_NOTICE_CONNECT_FAIL,                     //回连手机失败, param[0]=reason, param[6:1]=bd_addr
    BT_NOTICE_DISCONNECT,                       //蓝牙断开,     param[0]=feat,   param[6:1]=bd_addr
    BT_NOTICE_LOSTCONNECT,                      //蓝牙连接丢失, param[0]=nouse,  param[6:1]=bd_addr
    BT_NOTICE_CONNECTED,                        //蓝牙连接成功, param[0]=feat,   param[6:1]=bd_addr
    BT_NOTICE_SCO_SETUP,
    BT_NOTICE_SCO_FAIL,
    BT_NOTICE_SCO_KILL,
    BT_NOTICE_INCOMING,                         //来电
    BT_NOTICE_RING,                             //来电响铃
    BT_NOTICE_OUTGOING,                         //去电
    BT_NOTICE_CALL_NUMBER,                      //来电/去电号码
    BT_NOTICE_CALL,                             //建立通话
    BT_NOTICE_NETWORK_CALL,                     //网络通话
    BT_NOTICE_PHONE_CALL,                       //手机通话
    BT_NOTICE_SET_SPK_GAIN,                     //设置通话音量
    BT_NOTICE_MUSIC_PLAY,                       //蓝牙音乐开始播放
    BT_NOTICE_MUSIC_STOP,                       //蓝牙音乐停止播放
    BT_NOTICE_MUSIC_CHANGE_VOL,                 //改变蓝牙音乐音量
    BT_NOTICE_MUSIC_SET_VOL,                    //设置蓝牙音乐音量
    BT_NOTICE_HID_CONN_EVT,                     //HID服务连接事件
    BT_NOTICE_RECON_FINISH,                     //回连完成, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_ABORT_STATUS,                     //中止状态, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_SSP_NUMERIC_VALUE,
    BT_NOTICE_PULLPHONEBOOK_FINISH,             //获取电话薄完成
    BT_NOTICE_RETRY_CONNECT,                    //上电或超时回连尝试一次连接  //param[1:2], cur_retry_cnt

    BT_NOTICE_TWS_SEARCH_TIMEOUT = 0x40,        //搜索TWS超时
    BT_NOTICE_TWS_CONNECT_START,                //开始回连TWS, param[0]=nouse,  param[6:1]=bd_addr
    BT_NOTICE_TWS_CONNECT_FAIL,                 //TWS回连失败, param[0]=reason, param[6:1]=bd_addr
    BT_NOTICE_TWS_DISCONNECT,                   //TWS牙断开,   param[0]=feat,   param[6:1]=bd_addr
    BT_NOTICE_TWS_LOSTCONNECT,                  //TWS连接丢失, param[0]=nouse,  param[6:1]=bd_addr
    BT_NOTICE_TWS_CONNECTED,                    //TWS连接成功, param[0]=feat,   param[6:1]=bd_addr
    BT_NOTICE_TWS_SET_VOL,                      //设置TWS音乐音量
    BT_NOTICE_TWS_HID_SHUTTER,                  //远端TWS拍照键
    BT_NOTICE_TWS_USER_KEY,                     //TWS自定义按键
    BT_NOTICE_TWS_SET_OPERATION,
    BT_NOTICE_TWS_RES_PLAY,
    BT_NOTICE_TWS_USER_PARAM,                   //TWS 传数据最大10byte
    BT_NOTICE_TWS_RES5,
    BT_NOTICE_TWS_RES6,
    BT_NOTICE_TWS_RES7,

    BT_NOTICE_SET_XOSC_CAP,                      //推出测试盒校准成功消息
};

//公共控制消息
enum bt_msg_comm_ctl_t {
    BT_COMM_CTL_OFF                      = 0,        //关闭蓝牙
    BT_COMM_CTL_ON,                                  //打开蓝牙
    BT_COMM_CTL_AUDIO_BYPASS,                        //忽略蓝牙SBC/SCO AUDIO
    BT_COMM_CTL_AUDIO_ENABLE,                        //使能蓝牙SBC/SCO AUDIO
    BT_COMM_CTL_CONNECT_ADDRESS,
    BT_COMM_CTL_RESET_BT_NAME,
    BT_COMM_CTL_SEARCH_OS_CAPACITY,
    BT_COMM_CTL_FCC_TEST,
    BT_COMM_CTL_GET_RSSI,
    BT_COMM_CTL_MAX,
};

//通用的公共蓝牙消息
enum bt_msg_comm_t {
    BT_COMM_CTL,                                   //无传参的公共控制消息
    BT_COMM_BT_SET_SCAN,                           //设置可被发现/可被连接
    BT_COMM_BT_CONNECT,                            //连接蓝牙
    BT_COMM_BT_DISCONNECT,                         //断开蓝牙
    BT_COMM_ABORT_RECONNECT,                       //中止回连
    BT_COMM_NUMERIC_COMPARISON_RES,
    BT_COMM_SSP_SET_MODE,
};

//TWS无传参控制消息
enum bt_msg_tws_ctl_t {
    TWS_CTL_TWS_DISCONNECT,                      //断开TWS连接
    TWS_CTL_TWS_CONNECT,                         //建立TWS连接
    TWS_CTL_TWS_NOTICE_RING,
    TWS_CTL_TWS_SWITCH,                          //主从切换
    TWS_CTL_NOR_CONNECT,
    TWS_CTL_NOR_DISCONNECT,
    TWS_CTL_STOP_SEARCH_SLAVE,                   //停止搜索TWS配对，
    TWS_CTL_USER_PARAM,
    TWS_CTL_MAX,
};

enum bt_msg_tws_t {
    TWS_CTL,                                    //TWS无传参控制消息
    TWS_SET_SCAN,
    TWS_SEARCH_SLAVE,
    TWS_USER_KEY,                               //用户自定义消息（左右互发）
    TWS_SYNC_INFO,                              //同步状态消息（左右同步）
};

//A2DP无传参控制消息
enum bt_msg_a2dp_ctl_t {
    A2DP_CTL_VOL_CHANGE               = 0,        //音量调整
    A2DP_CTL_PLAY_PAUSE,                          //切换播放、暂停
    A2DP_CTL_LOW_LATENCY_ENABLE,                  //打开低延迟
    A2DP_CTL_LOW_LATENCY_DISABLE,                 //关闭低延迟
    A2DP_CTL_SWITCH_PALY,
    A2DP_CTL_GET_PLAY_STATUS_INFO,
    A2DP_CTL_A2DP_CONNECT,
    A2DP_CTL_A2DP_DISCONNECT,
    A2DP_CTL_MAX,

    A2DP_CTL_VOLUME_UP            = 0xff41,     //音量加
    A2DP_CTL_VOLUME_DOWN          = 0xff42,     //音量减
    A2DP_CTL_MUTE                 = 0xff43,     //MUTE
    A2DP_CTL_PLAY                 = 0xff44,     //播放
    A2DP_CTL_STOP                 = 0xff45,     //停止
    A2DP_CTL_PAUSE                = 0xff46,     //暂停
    A2DP_CTL_RECORD               = 0xff47,
    A2DP_CTL_REWIND               = 0xff48,     //快退
    A2DP_CTL_FAST_FORWARD         = 0xff49,     //快进
    A2DP_CTL_EJECT                = 0xff4a,
    A2DP_CTL_FORWARD              = 0xff4b,     //下一曲
    A2DP_CTL_BACKWARD             = 0xff4c,     //上一曲
    A2DP_CTL_REWIND_END           = 0xffc8,     //结束快退
    A2DP_CTL_FAST_FORWARD_END     = 0xffc9,     //结束快进

    A2DP_CTL_NO                   = 0xffff,
};

enum bt_msg_a2dp_t {
    A2DP_CTL,                                 //A2DP无传参控制消息
};

//HFP无传参控制消息
enum bt_msg_hfp_ctl_t {
    HFP_CTL_CALL_REDIAL,                         //回拨电话（最后一次通话）
    HFP_CTL_CALL_REDIAL_NUMBER,                  //回拨电话（从hfp_get_outgoing_number获取号码）
    HFP_CTL_CALL_ANSWER_INCOM,                   //接听来电（三通时挂起当前通话）
    HFP_CTL_CALL_ANSWER_INCOM_REJ_OTHER,         //接听来电（三通时挂断当前通话）
    HFP_CTL_CALL_TERMINATE,                      //挂断通话或来电
    HFP_CTL_CALL_SWAP,                           //切换三通电话
    HFP_CTL_CALL_PRIVATE_SWITCH,                 //切换私密通话
    HFP_CTL_HFP_REPORT_BAT,                      //报告电池电量
    HFP_CTL_HFP_SPK_GAIN,                        //设置通话扬声器音量
    HFP_CTL_HFP_MIC_GAIN,                        //设置通话麦克风音量
    HFP_CTL_HFP_AT_CMD,                          //发送AT命令（从hfp_get_at_cmd获取命令）
    HFP_CTL_HFP_SIRI_SW,                         //唤出/关闭siri
    HFP_CTL_HFP_REMOTE_PHONE_NUM,
    HFP_CTL_HFP_CONNECT,
    HFP_CTL_HFP_DISCONNECT,
    HFP_CTL_MAX,
};

enum bt_msg_hfp_t {
    HFP_CTL,                                  //HFP无传参控制消息
};

//HID无传参控制消息
enum bt_msg_hid_ctl_t {
    HID_CTL_HID_SEND,
    HID_CTL_HID_CONNECT,
    HID_CTL_HID_DISCONNECT,
    HID_CTL_MAX,
};

enum bt_msg_hid_t {
    HID_CTL,                                  //HID无传参控制消息
    HID_KEYBOARD,
    HID_CONSUMER,
    HID_TOUCH_SCREEN,
    HID_DOUYIN,                        //抖音触摸
    HID_SIMPLE_KEYBOARD,
};

//SPP无传参控制消息
enum bt_msg_spp_ctl_t {
    SPP_CTL_SPP_SEND,
    SPP_CTL_SPP_CONNECT,
    SPP_CTL_SPP_DISCONNECT,
    SPP_CTL_MAX,
};

enum bt_msg_spp_t {
    SPP_CTL,                                 //SPP无传参控制消息
    SPP_RX_FC_NEW_CREDIT,
};

//LE无传参控制消息
enum bt_msg_le_ctl_t {
    LE_CTL_BLE_SEND,
    LE_CTL_BLE_UPDATE_CONN_PARAM,
    LE_CTL_BLE_SET_ADV_INTV,
    LE_CTL_BLE_SET_ADV_DATA,
    LE_CTL_BLE_DISCONNECT,
    LE_CTL_BLE_ADV_ENABLE,
    LE_CTL_BLE_ADV_DISABLE,
    LE_CTL_BLE_SCAN_START,
    LE_CTL_BLE_SCAN_STOP,
    LE_CTL_MAX,
};

enum bt_msg_le_t {
    LE_CTL,                                 //LE无传参控制消息
    LE_SET_ADV0,
};

//蓝牙消息
enum bt_msg_t {
    BT_MSG_COMM,                                //蓝牙公共消息
    BT_MSG_TWS,                                 //TWS消息
    BT_MSG_LE,                                  //LE消息
    BT_MSG_A2DP,                                //A2DP消息
    BT_MSG_HFP,                                 //HFP消息
    BT_MSG_HSP,                                 //HSP消息
    BT_MSG_HID,                                 //HID消息，KEYPAD按键/CONSUMER按键/触摸屏
    BT_MSG_PBAP,                                //PBAP消息
    BT_MSG_LECLT,                               //BLE CLIENT消息
    BT_MSG_MAP,                                 //MAP消息 获取时间
    BT_MSG_SPP,                                 //SPP消息
    BT_MSG_MAX,
    BT_MSG_RES                  = 0xf0,         //0xf0~0xff保留给传参较多的api
};

enum sync_info_enum {
    SYNC_INFO_SETTING,                  //VOL/LANG/EQ/BAT/...
    SYNC_INFO_LEDCNT,                   //同步 led cnt
    SYNC_INFO_EQPARAM,
};

extern uint8_t cfg_bt_work_mode;
extern uint8_t cfg_bt_max_acl_link;
extern bool cfg_bt_dual_mode;
extern bool cfg_bt_tws_mode;
extern bool cfg_bt_scan_ctrl_mode;
extern bool cfg_bt_simple_pair_mode;
extern bool cfg_bt_spp_rx_flow_ctrl_en;

extern uint16_t cfg_bt_support_profile;
extern uint16_t cfg_bt_support_codec;
extern uint8_t cfg_bt_hid_type;
extern uint16_t cfg_bt_def_connect_times;
extern uint16_t cfg_bt_sup_to_connect_times;

extern uint8_t cfg_bt_rf_def_txpwr;
extern uint8_t cfg_bt_page_inq_txpwr;

extern uint8_t cfg_bt_a2dp_feature;
extern uint8_t cfg_bt_hfp_feature;
extern uint8_t cfg_bt_tws_pair_mode;
extern uint16_t cfg_bt_tws_feat;

#define bt_a2dp_is_vol_ctrl()                   (cfg_bt_a2dp_feature & A2DP_VOL_CTRL)
#define bt_a2dp_is_ios_play_status()            (cfg_bt_a2dp_feature & A2DP_IOS_PLAY_STATUS)

//control
void bt_fcc_init(void);
void bt_init(void);                             //初始化蓝牙变量
int bt_setup(void);                             //打开蓝牙模块
void bt_off(void);                              //关闭蓝牙模块
void bt_wakeup(void);                           //唤醒蓝牙模块
void bt_send_msg_do(uint msg);                  //蓝牙控制消息，参数详见bt_msg_t
void bt_audio_bypass(void);                     //蓝牙SBC/SCO通路关闭
void bt_audio_enable(void);                     //蓝牙SBC/SCO通路使能
void bt_set_sco_far_delay(void *buf, uint size, uint delay);

void bt_set_scan_do(u8 scan);                   //打开/关闭可被发现和可被连接
void bt_connect(uint times);                    //蓝牙设备回连, times: 重试次数（每次6s）
void bt_connect_address(void);                  //蓝牙设备回连地址，地址在bt_get_connect_addr中设置
void bt_disconnect(uint reason);                //蓝牙设备断开, reason: 0=单独断开（入仓），1=断开并同步关机（按键/自动关机）
void bt_hid_connect(void);                      //蓝牙HID服务回连
void bt_hid_disconnect(void);                   //蓝牙HID服务断开
void bt_a2dp_connect(void);                     //蓝牙A2DP服务连接
void bt_a2dp_disconnect(void);                  //蓝牙A2DP服务断开
void bt_hfp_connect(void);                      //蓝牙HFP服务连接
void bt_hfp_disconnect(void);                   //蓝牙HFP服务断开
void bt_spp_connect(void);
void bt_spp_disconnect(void);
bool bt_play_data_check(u32 check_time_ms);
uint32_t bt_sleep_proc(void);
void bt_enter_sleep(void);
void bt_exit_sleep(void);
void updata_bt_name(char *bt_name);
void bt_ssp_numeric_comparison_res(u8 result);
void bt_ssp_set_mode(u8 enable);                //1:SSP,0:PIN code

//status
uint bt_get_disp_status(void);                  //获取蓝牙的当前显示状态
uint bt_get_status(void);                       //获取蓝牙的当前状态
uint bt_get_call_indicate(void);                //获取通话的当前状态
uint bt_get_siri_status(void);                  //获取SIRI当前状态, 0=SIRI已退出, 1=SIRI已唤出
bool bt_is_calling(void);                       //判断是否正在通话
bool bt_is_playing(void);                       //判断是否正在播放
bool bt_is_ring(void);                          //判断是否正在响铃
bool bt_is_testmode(void);                      //判断当前蓝牙是否处于测试模式
bool bt_is_sleep(void);                         //判断蓝牙是否进入休眠状态
bool bt_is_connected(void);                     //判断蓝牙是否已连接（TWS副耳被连接，或主耳与手机已连接）
bool bt_is_ios_device(void);                    //判断当前播放的是否ios设备
bool sco_is_connected(void);                    //判断当前SCO链路是否连接
bool bt_sco_is_msbc(void);                      //判断当前通话是否是宽带通话
bool bt_a2dp_is_connected(void);                //判断蓝牙A2DP是否已连接
bool bt_hfp_is_connected(void);                 //判断蓝牙HFP是否已连接
int bt_hid_is_connected(void);
bool bt_hid_is_ready_to_discon(void);
bool bt_spp_is_connected(void);
bool bt_is_silence(void);
void bt_other_dev_is_silence(void);
bool bt_sync_tick(void);                        //根据蓝牙时钟同步到50ms，误差为5ms

//info
bool bt_get_link_btname(uint8_t index, char *name, uint8_t max_size); //index: 0=link0, 1=link1, 0xff=auto(default link0)

//tws api
bool bt_tws_is_connected(void);                 //tws设备是否已连接
bool bt_tws_is_slave(void);                     //tws主从判断，注意要先判断tws是否已连接
bool bt_tws_is_scaning(void);                   //tws是否已打开可被搜索与连接
void bt_tws_set_scan_do(u8 scan);                  //打开/关闭tws可被搜索与连接
void bt_tws_disconnect(void);                   //断开tws设备
void bt_tws_connect(void);                      //回连tws设备，连接成功返回BT_NOTICE_TWS_CONNECTED，连接失败返回BT_NOTICE_TWS_CONNECT_FAIL
void bt_tws_search_slave(uint timeout);         //搜索tws设备，搜索失败返回BT_NOTICE_TWS_SEARCH_TIMEOUT，搜索成功并建立连接返回BT_NOTICE_TWS_CONNECTED
uint bt_tws_get_link_info(uint8_t *bd_addr);    //获取tws配对信息，返回值（0=未连接过TWS，0x80=作为master连过副耳，0x81=作为slave连接过主耳），被连地址放在bd_addr
void bt_tws_delete_link_info(void);             //删除tws配对信息
bool bt_tws_user_key(uint keycode);             //发送tws自定义按键, 16bit有效
bool bt_tws_user_param(u8 *param, u8 len);      //TWS之间传递参数，最大10BYTE
void bt_tws_sync_ring_tone(void);               //同步tws响铃
void bt_tws_clr_ring_tone(void);                //
void bt_tws_switch(void);                       //主从切换，例如通话时切主副MIC
bool bt_tws_need_switch(bool calling);          //查看是否需要切换
void bt_tws_sync_run(uint ntick);               //同步到ntick * 50ms
bool bt_nor_is_connected(void);                 //是否已连接手机
void bt_nor_connect(void);                      //回连手机, 支持slave操作
void bt_nor_disconnect(void);                   //断开手机, 支持slave操作
bool bt_nor_get_link_info(uint8_t *bd_addr);    //获取手机配对信息，bd_addr=NULL时仅查询是否存在回连信息
bool bt_nor_get_link_info_addr(uint8_t *bd_addr, uint8_t order);    //获取第n个手机配对信息，bd_addr=NULL时仅查询是否存在回连信息
void bt_nor_delete_link_info(void);             //删除手机配对信息
u8 bt_tws_get_force_role(void);
bool bt_tws_get_first_role(void);
u32 bt_tws_get_ticks(void);
int8_t bt_get_rssi_value(void);                       //获取rssi的值，返回值为127则表示失败
#define bt_tws_is_auto_switch()         ((cfg_bt_tws_feat & TWS_FEAT_MS_SWITCH) != 0)
#define bt_tws_is_tsco()                ((cfg_bt_tws_feat & TWS_FEAT_TSCO) != 0)

enum {
    TWS_RES_NUM_0,
    TWS_RES_NUM_1,
    TWS_RES_NUM_2,
    TWS_RES_NUM_3,
    TWS_RES_NUM_4,
    TWS_RES_NUM_5,
    TWS_RES_NUM_6,
    TWS_RES_NUM_7,
    TWS_RES_NUM_8,
    TWS_RES_NUM_9,
    TWS_RES_RING,
    TWS_RES_LANGUAGE_EN,
    TWS_RES_LANGUAGE_ZH,
    TWS_RES_LOW_BATTERY,
    TWS_RES_CONNECTED,
    TWS_RES_TWS_CONNECTED,
    TWS_RES_DISCONNECT,
    TWS_RES_PAIRING,
    TWS_RES_CALL_HANGUP,
    TWS_RES_CALL_REJECT,
    TWS_RES_REDIALING,
    TWS_RES_MAX_VOL,
    TWS_PIANO_MAX_VOL,      //max volume tone
    TWS_RES_MAX,
};

void tws_res_init(void);                        //api
void tws_res_play(uint8_t index);               //api

void tws_mp3_res_play_kick(u32 addr, u32 len);  //tws_res开始播放
u32 tws_res_sync(u32 limit);                    //tws_res同步
void tws_res_add(u8 index, u32 ticks);          //tws_res加入列表
void tws_res_proc(void);                        //tws_res流程处理
void tws_res_done(void);

#define bt_send_msg(ogf, ocf)                   bt_send_msg_do((ogf<<24) | (ocf))
#define bt_comm_ctrl_msg(msg)                   bt_send_msg(BT_MSG_COMM, (BT_COMM_CTL<<16) | msg)
#define bt_comm_param_msg(msg, param)           bt_send_msg(BT_MSG_COMM, (msg<<16) | (u16)param)
#define bt_tws_ctrl_msg(msg)                    bt_send_msg(BT_MSG_TWS, (BT_COMM_CTL<<16) | msg)
#define bt_tws_param_msg(msg, param)            bt_send_msg(BT_MSG_TWS, (msg<<16) | (u16)param)
#define bt_a2dp_ctrl_msg(msg)                   bt_send_msg(BT_MSG_A2DP, (A2DP_CTL<<16) | msg)
#define bt_a2dp_param_msg(msg, param)           bt_send_msg(BT_MSG_A2DP, (msg<<16) | (u16)param)
#define bt_hfp_ctrl_msg(msg)                    bt_send_msg(BT_MSG_HFP, (HFP_CTL<<16) | msg)
#define bt_hfp_param_msg(msg, param)            bt_send_msg(BT_MSG_HFP, (msg<<16) | (u16)param)
#define bt_hid_ctrl_msg(msg)                    bt_send_msg(BT_MSG_HID, (HID_CTL<<16) | msg)
#define bt_hid_param_msg(msg, param)            bt_send_msg(BT_MSG_HID, (msg<<16) | (u16)param)
#define bt_spp_ctrl_msg(msg)                    bt_send_msg(BT_MSG_SPP, (SPP_CTL<<16) | msg)
#define bt_spp_param_msg(msg, param)            bt_send_msg(BT_MSG_SPP, (msg<<16) | (u16)param)
#define bt_le_ctrl_msg(msg)                     bt_send_msg(BT_MSG_LE, (LE_CTL<<16) | msg)
#define bt_le_param_msg(msg, param)             bt_send_msg(BT_MSG_LE, (msg<<16) | (u16)param)



//蓝牙连接
#define bt_scan_enable()                        bt_set_scan_do(0x3)                       //打开扫描
#define bt_scan_disable()                       bt_set_scan_do(0x0)                       //关闭扫描
#define bt_search_os_capacity()                 bt_comm_ctrl_msg(BT_COMM_CTL_SEARCH_OS_CAPACITY)  //主动发起查询当前连接的设备是否IOS，发起查询后，调用判断bt_is_ios_device(),连接2个手机此函数无效
#define bt_abort_reconnect()                    bt_comm_param_msg(BT_COMM_ABORT_RECONNECT, 0xffff)        //终止回接
#define bt_abort_reconnect_silence(feat)        bt_comm_param_msg(BT_COMM_ABORT_RECONNECT, (u16)feat)     //终止回接，没有消息回调。feat:0=手机, 1=TWS
#define bt_fcc_test_start()                     bt_comm_ctrl_msg(BT_COMM_CTL_FCC_TEST)

//蓝牙音乐
#define bt_music_play()                         bt_a2dp_ctrl_msg(A2DP_CTL_PLAY)                //播放
#define bt_music_pause()                        bt_a2dp_ctrl_msg(A2DP_CTL_PAUSE)               //暂停
#define bt_music_play_pause()                   bt_a2dp_ctrl_msg(A2DP_CTL_PLAY_PAUSE)          //切换播放/暂停
#define bt_music_stop()                         bt_a2dp_ctrl_msg(A2DP_CTL_STOP)                //停止
#define bt_music_prev()                         bt_a2dp_ctrl_msg(A2DP_CTL_BACKWARD)            //上一曲
#define bt_music_next()                         bt_a2dp_ctrl_msg(A2DP_CTL_FORWARD)             //下一曲
#define bt_music_rewind()                       bt_a2dp_ctrl_msg(A2DP_CTL_REWIND)              //开始快退
#define bt_music_rewind_end()                   bt_a2dp_ctrl_msg(A2DP_CTL_REWIND_END)          //结束快退
#define bt_music_fast_forward()                 bt_a2dp_ctrl_msg(A2DP_CTL_FAST_FORWARD)        //开始快进
#define bt_music_fast_forward_end()             bt_a2dp_ctrl_msg(A2DP_CTL_FAST_FORWARD_END)    //结束快进
#define bt_music_vol_change()                   bt_a2dp_ctrl_msg(A2DP_CTL_VOL_CHANGE)          //通知手机音量已调整
#define bt_2acl_switch_music_play()             bt_a2dp_ctrl_msg(A2DP_CTL_SWITCH_PALY);
#define bt_music_paly_status_info()             bt_a2dp_ctrl_msg(A2DP_CTL_GET_PLAY_STATUS_INFO);//获取歌曲时长单位ms,当前播放位置,播放状态信息

//蓝牙通话
#define bt_call_redial_last_number()            bt_hfp_ctrl_msg(HFP_CTL_CALL_REDIAL)         //电话回拨（最后一次通话）
#define bt_call_answer_incoming()               bt_hfp_ctrl_msg(HFP_CTL_CALL_ANSWER_INCOM)   //接听电话，三通时挂起当前通话
#define bt_call_answer_incom_rej_other()        bt_hfp_ctrl_msg(HFP_CTL_CALL_ANSWER_INCOM_REJ_OTHER)     //接听电话，三通时挂断当前通话
#define bt_call_terminate()                     bt_hfp_ctrl_msg(HFP_CTL_CALL_TERMINATE)      //挂断电话
#define bt_call_swap()                          bt_hfp_ctrl_msg(HFP_CTL_CALL_SWAP)           //切换三通电话
#define bt_call_private_switch()                bt_hfp_ctrl_msg(HFP_CTL_CALL_PRIVATE_SWITCH) //切换私密通话
#define bt_call_redial_number()                 bt_hfp_ctrl_msg(HFP_CTL_CALL_REDIAL_NUMBER)
#define bt_call_get_remote_phone_number()       bt_hfp_ctrl_msg(HFP_CTL_HFP_REMOTE_PHONE_NUM) //获取远端号码
#define bt_hfp_siri_switch()                    bt_hfp_ctrl_msg(HFP_CTL_HFP_SIRI_SW)         //开关SIRI, android需要在语音助手中打开“蓝牙耳机按键启动”, ios需要打开siri功能
#define bt_hfp_report_bat()                     bt_hfp_ctrl_msg(HFP_CTL_HFP_REPORT_BAT)
#define bt_hfp_set_spk_gain()                   bt_hfp_ctrl_msg(HFP_CTL_HFP_SPK_GAIN)
#define bt_hfp_send_at_cmd()                    bt_hfp_ctrl_msg(HFP_CTL_HFP_AT_CMD)

#define bt_tws_sync_setting()                   bt_tws_param_msg(TWS_SYNC_INFO, SYNC_INFO_SETTING)
#define bt_tws_sync_ledcnt()                    bt_tws_param_msg(TWS_SYNC_INFO, SYNC_INFO_LEDCNT)
#define bt_tws_sync_eq_param()                  bt_tws_param_msg(TWS_SYNC_INFO, SYNC_INFO_EQPARAM)
#define bt_tws_stop_search_slave()              bt_tws_ctrl_msg(TWS_CTL_STOP_SEARCH_SLAVE)    //停止搜索TWS配对

#define ble_adv_en()                            bt_le_ctrl_msg(LE_CTL_BLE_ADV_ENABLE)
#define ble_adv_dis()                           bt_le_ctrl_msg(LE_CTL_BLE_ADV_DISABLE)

#define ble_scan_start()                        bt_le_ctrl_msg(LE_CTL_BLE_SCAN_START)
#define ble_scan_stop()                         bt_le_ctrl_msg(LE_CTL_BLE_SCAN_STOP)

void spp_init(void);
void hfp_hf_init(void);
void a2dp_init(void);
void aap_init(void);
uint8_t sdp_add_service(void *item);
uint bt_get_hfp_feature(void);
int bt_spp_tx(uint8_t *packet, uint16_t len);
void print_btrf_sfr(void);
//hid
struct ts_ctl_t {
    uint8_t act;
    uint8_t res;
    uint16_t x;
    uint16_t y;
};

enum {
    HID_MOUSE_BUTTON_1,
    HID_MOUSE_BUTTON_MAX,

    HID_MOUSE_UP_SLIDE =    0x10,
    HID_MOUSE_DOWN_SLIDE,

    HID_MOUSE_WHEEL_UP =    0xF0,
    HID_MOUSE_WHEEL_DOWN,
    HID_MOUSE_ACPAN_UP,
    HID_MOUSE_ACPAN_DOWN,

    HID_MOUSE_MAX,
};

extern u8* douyin_hid_code;
void hid_device_init(void);
bool bt_hid_send(void *buf, uint len, bool auto_release);                                           //自定义HID数组
bool bt_hid_send_key(uint type, uint keycode);                                                      //标准HID按键
#define bt_hid_key(keycode)                     bt_hid_send_key(HID_KEYBOARD, keycode)              //标准HID键, 如Enter
#define bt_hid_consumer(keycode)                bt_hid_send_key(HID_CONSUMER, keycode)              //自定义HID键, 如VOL+ VOL-
#define bt_hid_touch_screen(keycode)            bt_hid_send_key(HID_TOUCH_SCREEN, keycode)          //触屏
bool bt_hid_touch_screen_set_key(void *ts);
bool hid_mouse_handler(u8 opcode);
void user_finger_down();
void user_finger_up();
void bsp_bt_hid_finger(bool is_press, s16 x, s16 y);

void goep_client_init(void);
//pbap
void pbap_client_init(void);
void bt_pbap_start(u8 type);                    //type：获取本地号码:BIT(0)、获取来电号码:BIT(1)、获取去电号码:BIT(2)、获取未接号码:BIT(3)
void bt_pbap_abort(void);                       //终止电话本的获取
u32 bt_pbap_get_sta(void);                      //获取电话本状态，0：IDLE； 1：获取中
//map
void map_client_init(void);
void bt_map_start(void);                     	//MAP的获取
void bt_map_abort(void);                        //终止MAP的获取
//hsp
void hsp_hs_init(void);
void hsp_hs_init_var(void);
void bt_hsp_call_switch(void);                  //挂断/接听
void bt_hsp_sco_conn(void);                     //建立HSP SCO连接
void bt_hsp_sco_disconn(void);                  //断开HSP SCO连接

/*****************************************************************************
 * SIMPLE KEYBOARD API
 *****************************************************************************/
#define bt_hid_simple_keyboard(keycode)         bt_hid_send_key(HID_SIMPLE_KEYBOARD, keycode)       //简易键盘
#define SIMPLE_KEYBOARD_VOL_UP      0x01
#define SIMPLE_KEYBOARD_VOL_DOWN    0x02
#define SIMPLE_KEYBOARD_NEXT        0x10
#define SIMPLE_KEYBOARD_PREV        0x20
#define SIMPLE_KEYBOARD_PLAY        0x88
#define SIMPLE_KEYBOARD_PLAY_UP     0x66
#define SIMPLE_KEYBOARD_RELEASE     0x00
void btstack_hid_simple_keyboard(uint);

/*****************************************************************************
 * BLE API
 *****************************************************************************/

#define BLE_ADV0_EN_BIT             0x01
#define BLE_ADV0_MS_VAR_BIT         0x02
#define BLE_ADV0_ADDR_PUBIC_BIT     0x04

u8 ble_get_status(void);
bool ble_is_connect(void);
u16 ble_get_gatt_mtu(void);
void ble_set_adv(u8 chanel, u8 type);
void ble_set_adv_interval(u16 interval);
void ble_enable_adv(void);
void ble_disable_adv(void);
void ble_disconnect(void);
bool ble_tx_notify(u8 index, u8* buf, u8 len);
void ble_update_conn_param(u16 interval, u16 latency, u16 timeout);
bool ble_set_adv_data(const u8 *adv_buf, u32 size);

uint8_t ble_adv0_get_adv_en(void);
void ble_adv0_set_ctrl(uint opcode);                    //0=关闭广播, 1=打开广播, 2=更新广播数据（打开时直接广播，关闭时仅更新buffer）

/********************************************************************
        修改蓝牙库，向下兼容，不需要时可删掉
********************************************************************/
#define BT_CTL_HFP_SPK_GAIN         0x01
#define bt_ctrl_msg(x)             do { if(x == BT_CTL_HFP_SPK_GAIN) {bt_hfp_set_spk_gain();}}while(0)

//兼容旧版本修改bt_set_scan成bt_set_scan_do，不需要兼容时可将bt_set_scan_do 改成bt_set_scan
#define bt_set_scan(x,y)           do{u8 scan = 0; if(x){scan |= BIT(0);} if(y){scan |= BIT(1);} bt_set_scan_do(scan);}while(0)
//兼容旧版本修改bt_tws_set_scan成bt_tws_set_scan_do，不需要兼容时可将bt_tws_set_scan_do 改成bt_tws_set_scan
#define bt_tws_set_scan(x,y)       do{u8 scan = 0; if(x){scan |= BIT(0);} if(y){scan |= BIT(1);} bt_tws_set_scan_do(scan);}while(0)

#endif
