#include "include.h"
#include "api.h"


#define BT_PROFILE          (PROF_A2DP*BT_A2DP_EN) | (PROF_HFP*BT_HFP_EN) | (PROF_SPP*BT_SPP_EN) | (PROF_HID*BT_HID_EN) | (PROF_HSP*BT_HSP_EN)
#define BT_CODEC            (CODEC_SBC) | (CODEC_AAC*BT_A2DP_AAC_AUDIO_EN) | (CODEC_MSBC * BT_HFP_MSBC_EN) | (CODEC_PLC * BT_PLC_EN)
#define HFP_FEATURE         (HFP_BAT_REPORT*BT_HFP_BAT_REPORT_EN) | (HFP_INBAND_RING_RONE*BT_HFP_PLAY_IOS_RING_EN) | (HFP_CALL_PRIVATE*BT_HFP_CALL_PRIVATE_FORCE_EN) | (HFP_RING_NUMBER_EN*BT_HFP_RING_NUMBER_EN)
#define A2DP_FEATURE        (A2DP_VOL_CTRL*BT_A2DP_VOL_CTRL_EN) | (A2DP_RESTORE_PLAYING) | (A2DP_AVDTP_DELAY_REPORT*BT_AVDTP_DELAY_REPORT_EN)


///baseband
uint8_t cfg_bt_rf_def_txpwr         = 0;        //降低预置参数RF发射功率，单位1dbm
uint8_t cfg_bt_page_inq_txpwr       = 0;        //降低回连和搜索RF发射功率，单位2dbm
uint8_t cfg_bt_sniff_clk_sel        = 2;		//2:31.25KHz 晶振, 3:31.25KHz RC

///stack
uint8_t cfg_bt_work_mode            = WORK_MODE;
uint8_t cfg_bt_max_acl_link         = BT_2ACL_EN+1;

bool cfg_bt_dual_mode               = BT_DUAL_MODE_EN;
bool cfg_bt_tws_mode                = BT_TWS_EN;
bool cfg_bt_scan_ctrl_mode          = BT_DISCOVER_CTRL_EN;
bool cfg_bt_simple_pair_mode        = BT_SIMPLE_PAIR_EN;

uint16_t cfg_bt_support_profile     = BT_PROFILE;
uint16_t cfg_bt_support_codec       = BT_CODEC;

uint8_t cfg_bt_a2dp_feature         = A2DP_FEATURE;
uint8_t cfg_bt_hfp_feature          = HFP_FEATURE;
uint8_t cfg_bt_hid_type             = BT_HID_TYPE;

uint16_t cfg_bt_def_connect_times   = BT_POWER_UP_RECONNECT_TIMES;      //默认上电回连重试次数(一次大约1.3秒),该值设置为0xFFFF时表示一直回连
uint16_t cfg_bt_sup_to_connect_times = BT_TIME_OUT_RECONNECT_TIMES;     //超时断线回连重试次数(一次大约1.3秒),该值设置为0xFFFF时表示一直回连
//uint16_t cfg_bt_sup_to_connect_interval = 2000;                       //超时断线回连时每完成一次,预留给其他手机可连时间间隔,单位MS (一直回连时,每重试一次完成后,开放该时间间隔让其它手机容易连上,库中默认是50ms，若其它手机比较难连,可打开此句改变时间间隔，一般这里的2秒足够了)

//以下两个参数是蓝牙后台,蓝牙有声音则切回蓝牙模式, 可以配置该两个值过滤比如微信或QQ一些短提示音。
//uint8_t cfg_silence_chk_cnt = 20;    //判断静音阈值，值越小判断静音越快，值越大判断静音越慢，结果通过bt_is_silence，获得
//uint8_t cfg_music_chk_cnt  = 10;     //判断音乐阈值，值越小判断是音乐越快，值越大判断是音乐越慢，结果通过bt_is_silence，获得

//uint8_t cfg_bt_ssp_io_cap_auth      = 1;
//bool cfg_bt_support_snatch          = true;   //是否支持被后连接的手机抢连功能,不能打开一拖二功能
bool cfg_bt_carplay                   = BT_CARPLAY_EN;
//bool bt_ssp_numeric_comparison        = true;  //bt_ssp_numeric_comparison_res(1),调用此函数回复应答，配对应答，1：同意，0：拒绝，默认直接应答1


///app
const char bt_local_name[] = BT_NAME_DEFAULT;
const char bt_local_addr[6] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};


//自定义蓝牙类别图标，根据需要选择
u32 bt_get_class_of_device(void)
{
#if (BT_HID_TYPE == HID_TYPE_GAMEPAD)
    return 0x0025c0;    //游戏手柄，GamePad
#elif (BT_HID_TYPE == HID_TYPE_KEYBOARD) //BT_HID_SIMPLE_KEYBOARD
    return 0x002540;    //键盘图标，Android带显示电量，IOS不带电量显示。全部IOS均可连接HID拍照。
#else
    return 0x240418;    //耳机图标，Android和IOS均带电量显示。IOS9之前的机型无法连接HID拍照。（默认使用）
#endif
}

//PIN配对码（最多16个字符），默认"0000"
//#if !BT_SIMPLE_PAIR_EN
//const char *bt_get_pin_code(void)
//{
//    return "0000";
//}
//#endif

//回连间隔（N+3000）mS，间隔越大，下一次回连越慢，更容易被其他手机搜索连接，N应大于等于2000
//u32 bt_get_conn_fail_delay(void)
//{
//    return 2000;
//}

//是否需要一直回连手机
//bool bt_is_always_reconn(void)
//{
//    return true;  //true一直回连
//}

bool bt_is_force_master(void)  //开TSCO功能时,该函数需要返回true
{
    bool master =  BT_TWS_EN && BT_TSCO_EN && xcfg_cb.bt_tws_en;
    if (master){
        return true;
    } else {
        return false;
    }
}
//const bool cfg_bt_sniff_mode_dis   = true;  //true: 关闭蓝牙sniff功能

//uint8_t bt_get_connect_addr(uint8_t *bd_addr)
//{
//    memcpy(bd_addr, connect_addr, 6);
//   return 1;
//}

//uint16_t bt_pbap_set_phonebook_count() {    //设置获取的通讯录数量,0为获取全部
//    return 0;
//}
//
//uint16_t bt_pbap_set_phonebook_offset() {   //设置获取的通讯录偏移量,0为不偏移
//    return 0;
//}

#if USB_BT_NAME_UPDATA
char bt_name_upd_buf[34];
#define param_read(a, b, c)     cm_read(PAGE0(a), b, c)

void get_flash_bt_name(void)
{
    u8 i = 0;
    u8 name_len = 0;
    char *name_buff = bt_name_upd_buf;
    memset(bt_name_upd_buf, 0, 34);
    param_read(name_buff, BT_NAME_UPDATA, 36);
    if ((bt_name_upd_buf[0] == 0x23) && (bt_name_upd_buf[1] == 0x23)) {
        for(i = 2; i < 34; i++) {
            if((bt_name_upd_buf[i] == 0x23) && (bt_name_upd_buf[i-1] == 0x23)) {
                name_len = i;
                break;
            }
        }
        if(name_len > 2) {
            memset(xcfg_cb.bt_name, 0, 32);
            printf("bt_name_write:");
            my_print_r(&name_buff[2],(name_len - 3));
            memcpy(xcfg_cb.bt_name, &name_buff[2], (name_len - 3));
        }
    }
}
#endif // USB_BT_NAME_UPDATA
const char *bt_get_local_name(void)
{
#if USB_BT_NAME_UPDATA
    get_flash_bt_name();
#endif // USB_BT_NAME_UPDATA

#if 1
#if FUNC_BTHID_EN
    if (is_bthid_mode()) {
        return xcfg_cb.bthid_name;
    }
#endif // FUNC_BTHID_EN
    return xcfg_cb.bt_name;
#else
    return bt_local_name;
#endif // 1

}

#if BT_TWS_EN
void bt_save_master_addr(u8 *addr)
{
    if (bt_tws_is_auto_switch()) {
        cm_write8(PARAM_MASTER_ADDR_VALID, 1);
        cm_write(addr, PARAM_MASTER_ADDR, 6);
        cm_sync();
    }
}

bool bt_get_master_addr(u8 *addr)
{
    if (bt_tws_is_auto_switch() && cm_read8(PARAM_MASTER_ADDR_VALID)) {
        cm_read(addr, PARAM_MASTER_ADDR, 6);
        return true;
    }
    return false;
}
#endif

void bt_get_local_bd_addr(u8 *addr)
{
#if BT_TWS_EN
    if (bt_get_master_addr(addr)) {
        return;
    }
#endif
#if BT_LOCAL_ADDR
    param_random_key_read(&addr[2]);
    addr[0] = 0x41;
    addr[1] = 0x42;
#else
    memcpy(addr, xcfg_cb.bt_addr, 6);
#endif
#if FUNC_BTHID_EN
    if (is_bthid_mode()) {
        addr[5] ^= BIT(0);
    }
#endif // FUNC_BTHID_EN
}

u32 bt_get_rand_seed(void)
{
    return sys_cb.rand_seed;
}

void bt_get_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_read: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
#if FUNC_BTHID_EN
    if (is_bthid_mode()) {
        cm_read(buf, BTHID_CM_PAGE(addr), size);
    } else
#endif // FUNC_BTHID_EN

#if BT_SUPPORT_EIGHT_DEVICE_EN
    if((addr + size) >= 256){
        u16 addr_temp = 0;
        addr = addr + size - 256;
        if(addr < size){
            addr_temp = 0;
        }
        for(;addr > size;addr -= size){
            addr_temp += size;
        }
        cm_read(buf, BT_CM_PAGE_2(addr_temp), size);
    }else
#endif
    {
        cm_read(buf, BT_CM_PAGE(addr), size);
    }
    //print_r(buf, size);
}

void bt_put_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_write: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    //print_r(buf, size);
#if FUNC_BTHID_EN
    if (is_bthid_mode()) {
        cm_write(buf, BTHID_CM_PAGE(addr), size);
    } else
#endif // FUNC_BTHID_EN

#if BT_SUPPORT_EIGHT_DEVICE_EN
    if((addr + size) >= 256){
        u16 addr_temp = 0;
        addr = addr + size - 256;
        if(addr < size){
            addr_temp = 0;
        }
        for(;addr > size;addr -= size){
            addr_temp += size;
        }
        cm_write(buf, BT_CM_PAGE_2(addr_temp), size);
    }else
#endif
    {
        cm_write(buf, BT_CM_PAGE(addr), size);
    }
}

void bt_get_ext_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_read: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    cm_read(buf, EXT_CM_PAGE(addr), size);
    //print_r(buf, size);
}

void bt_put_ext_link_info(void *buf, u16 addr, u16 size)
{
    //printf("bt_write: %04x,%04x, %08lx\n", addr, size, BT_CM_PAGE(addr));
    //print_r(buf, size);
    cm_write(buf, EXT_CM_PAGE(addr), size);
}

void bt_sync_link_info(void)
{
    cm_sync();
}

#if BT_FCC_TEST_EN
ALIGNED(4)
u8 huart_buffer[512];

void huart_init(void)
{
    huart_init_do(HUART_TR_PB3, HUART_TR_PB3, 1500000, huart_buffer, 512);
}
#endif


#if   BT_RF_EXT_CTL_EN || BT_RF_POWER_BALANCE_EN
void btrf_power_balance_init(void);
void btrf_power_balance_level_set(u8 level);
void btrf_power_balance_level_delay_set(u32 delay_us, u8 level);   //unit us

bool cfg_bb_rf_ctl = true;
void bb_rf_ext_ctl_init(void)   //蓝牙库回调函数
{
    //初始化控制IO
#if BT_RF_POWER_BALANCE_EN
    btrf_power_balance_init();
#endif
//    GPIOAFEN &= ~(BIT(6)|BIT(5));
//    GPIOADE |= (BIT(6)|BIT(5));
//    GPIOADIR &= ~(BIT(6)|BIT(5));
//    GPIOBFEN &= ~(BIT(1)|BIT(2));
//    GPIOBDE |= (BIT(1)|BIT(2));
//    GPIOBDIR &= ~(BIT(1)|BIT(2));
}

//char strtest[] = " %x";
AT(.com_text.isr.txrx)
void bb_rf_ext_ctl_cb(u32 rf_sta) //蓝牙库回调函数
{
    //根据TX/RX状态设置IO电平
//	if(rf_sta & BIT(8)) {			//tx on
//	} else if(rf_sta & BIT(9)){	    //tx down
//	} else if(rf_sta & BIT(10)){	//rx on
//	} else if(rf_sta & BIT(11)){	//rx down
//	} else {						//idle
//	}
#if BT_RF_POWER_BALANCE_EN
	if(rf_sta & BIT(8)) {			//tx on
        btrf_power_balance_level_delay_set(53,0);  //TX开始后,TX延时一段时间才真正耗电,这里延时一段时间后才降低功耗
	} else if(rf_sta & BIT(9)){	    //tx down
	    btrf_power_balance_level_delay_set(6,5);  //TX结束,增加耗电
	} else if(rf_sta & BIT(10)){	//rx on
	    btrf_power_balance_level_delay_set(65,1);  //RX开始后,RX延时一段时间才真正耗电,这里延时一段时间后才降低功耗
	} else if(rf_sta & BIT(11)){	//rx down
        btrf_power_balance_level_set(5);           //RX结束,增加耗电
	} else {						//idle
        btrf_power_balance_level_set(5);           //空闲状态,增加耗电
	}

#endif
}
#endif //BT_RF_EXT_CTL_EN || BT_RF_POWER_BALANCE_EN


#if BT_RF_POWER_BALANCE_EN
//使用空闲的IO,配置内部300欧上下拉,消耗电流来实现btrf tx,rx及空闲时电流不均衡问题
//PA4 / PA3 / PF1 / PB0/PB5  -- SSOP24/SOP16 没有用到的GPIO
#define GPIOA_TEST_BIT  (BIT(3)|BIT(4))
#define GPIOB_TEST_BIT  (BIT(0)|BIT(5))
#define GPIOF_TEST_BIT   BIT(1)
void timer3_init_for_btrf_power_balance(void);
void timer3_exit_for_btrf_power_balance(void);
volatile u8 btrf_power_level = 0;
volatile u8 btrf_power_balance_en = 1;  //for en test
AT(.com_text.btrf_pwrbalance)
void btrf_power_balance_init(void)
{
    printf("%s\n",__func__);
    timer3_init_for_btrf_power_balance();
    GPIOADE |= GPIOA_TEST_BIT;
    GPIOAFEN &= ~GPIOA_TEST_BIT;
    GPIOADIR |= GPIOA_TEST_BIT;
    GPIOBDE |=  GPIOB_TEST_BIT;
    GPIOBFEN &= ~GPIOB_TEST_BIT;
    GPIOBDIR |= GPIOB_TEST_BIT;
    GPIOFDE |= GPIOF_TEST_BIT;
    GPIOFFEN &= ~GPIOF_TEST_BIT;
    GPIOFDIR |= GPIOF_TEST_BIT;

}

void btrf_power_balance_exit(void)
{
    printf("%s\n",__func__);
    timer3_exit_for_btrf_power_balance();
    GPIOADE |= GPIOA_TEST_BIT;
    GPIOAFEN &= ~GPIOA_TEST_BIT;
    GPIOADIR |= GPIOA_TEST_BIT;
    GPIOBDE |=  GPIOB_TEST_BIT;
    GPIOBFEN &= ~GPIOB_TEST_BIT;
    GPIOBDIR |= GPIOB_TEST_BIT;
    GPIOFDE |= GPIOF_TEST_BIT;
    GPIOFFEN &= ~GPIOF_TEST_BIT;
    GPIOFDIR |= GPIOF_TEST_BIT;
    btrf_power_balance_level_set(0);
}

AT(.com_text.btrf_pwrbalance)
void btrf_power_balance_level_set(u8 level)   //unit 3.3V/600 = 5.5mA
{
    GPIOAPU300 &= ~GPIOA_TEST_BIT;
    GPIOAPD300 &= ~GPIOA_TEST_BIT;
    GPIOBPU300 &= ~GPIOB_TEST_BIT;
    GPIOBPD300 &= ~GPIOB_TEST_BIT;
    GPIOFPU300 &= ~GPIOF_TEST_BIT;
    GPIOFPD300 &= ~GPIOF_TEST_BIT;
    if (0 == level) {
        return;
    }
    if (!btrf_power_balance_en) {
        return;
    }
    if(level >= 1){
        GPIOAPU300 |= BIT(3);
        GPIOAPD300 |= BIT(3);
    }
    if(level >= 2) {
        GPIOAPU300 |= BIT(4);
        GPIOAPD300 |= BIT(4);
    }
    if(level >= 3) {
        GPIOBPU300 |= BIT(0);
        GPIOBPD300 |= BIT(0);
    }
    if(level >= 4) {
        GPIOBPU300 |= BIT(5);
        GPIOBPD300 |= BIT(5);
    }
    if(level >= 5) {
        GPIOFPU300 |= BIT(1);
        GPIOFPD300 |= BIT(1);
    }
}

//char str_t3[] = "3 ";
//char str_t2[] = "2 ";
AT(.com_text.btrf_pwrbalance) FIQ
void timer3_isr_for_btrf_power_balance(void)
{
    if (TMR3CON & BIT(16)) {
        TMR3CPND = BIT(16);              //Clear Pending
        TMR3CON &=  ~BIT(7);
        btrf_power_balance_level_set(btrf_power_level);
    }
}

AT(.com_text.btrf_pwrbalance)
void btrf_power_balance_level_delay_set(u32 delay_us, u8 level)   //unit us
{
    btrf_power_level = level;
    TMR3CNT = 0;
    TMR3PR = delay_us;
    TMR3CPND = BIT(16);
    TMR3CON |=  BIT(7);
}

void timer3_init_for_btrf_power_balance(void)
{
    printf("%s\n",__func__);
    CLKCON1 |= BIT(8);                  //timer3 clk en
    TMR3CON =  BIT(7);                  //Timer overflow interrupt enable
    TMR3CNT = 0;
    TMR3PR  = 1000000 / 2 - 1;          //1000ms, select xosc26_div 1M clk
    TMR3CON |= BIT(2) | BIT(0);         //Timer works in Counter Mode
    sys_irq_init(IRQ_TMR3_VECTOR, 1, timer3_isr_for_btrf_power_balance);
}

void timer3_exit_for_btrf_power_balance(void)
{
    printf("%s\n",__func__);
    TMR3CNT = 0;
    TMR3CON = 0;
}
#endif //BT_RF_POWER_BALANCE_EN
