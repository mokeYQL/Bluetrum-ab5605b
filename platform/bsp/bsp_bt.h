#ifndef _BSP_BT_H
#define _BSP_BT_H

//标准HID键
#define HID_KEY_ENTER       0x28

//自定义HID键 HIT_TYPE_PHOTO1
#define HID_KEY_VOL_DOWN    0x00EA
#define HID_KEY_VOL_UP      0x00E9
#define HID_KEY_IOS_HOME    0x0040      //ios home
#define HID_KEY_IOS_POWER   0x0030      //ios 锁屏
#define HID_KEY_IOS_LAYOUT  0x01AE      //ios Keyboard Layout


//抖音神器REPORT ID
#define RPT_ID_DOUYIN_KBD       	0x01
#define RPT_ID_DOUYIN_FINGER        0x02
#define RPT_ID_DOUYIN_POINTER   	0x03
#define RPT_ID_DOUYIN_MOUSE  	    0x04
#define RPT_ID_DOUYIN_CONSUMER  	0x05

//base HID_TYPE_DOUYIN 的SIMPLE KEYBOARD
#define HID_KBD_RELEASE  0x000
#define HID_VOL_DOWN     0x001
#define HID_VOL_UP       0x002
#define HID_MENU         0x004
#define HID_POWER        0x008
#define HID_NEXT_TRACK   0x010
#define HID_PREV_TRACK   0x020
#define HID_AC_HOME      0x040
#define HID_PLAY_PAUSE   0x080
#define HID_FLASH_TEL    0x100

struct vs_test_ctrl_cmd
{
    uint8_t mode;
    uint8_t test_scenario;
    uint8_t hopping_mode;
    uint8_t tx_freq;
    uint8_t rx_freq;
    uint8_t power_control;
    uint8_t poll_period;
    uint8_t packet_type;
    uint16_t payload_length;
} __attribute__ ((packed)) ;

void bsp_bt_init(void);
void bsp_bt_close(void);
void bsp_bt_vol_change(void);
void bsp_bt_hid_photo(void);
bool bsp_bt_pwrkey5s_check(void);
void bsp_bt_pwrkey5s_clr(void);
void bsp_bt_hid_tog_conn(void);
void bsp_bt_hid_screen_left(void);
void bsp_bt_hid_screen_right(void);
void bsp_bt_hid_keyboard(u16 keycode);
void bt_name_suffix_set(void);
uint bsp_bt_get_hfp_vol(uint hfp_vol);
void bt_tws_switch_for_charge(void);

void bt_redial_init(void);
void bt_redial_reset(uint8_t index);
void bt_update_redial_number(uint8_t index, char *buf, u32 len);
const char *bt_get_last_call_number(uint8_t index);
bool bsp_bt_hid_vol_change(u16 keycode);
#endif //_BSP_BT_H
