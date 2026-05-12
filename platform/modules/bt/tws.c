#include "include.h"
#include "api.h"

#if BT_TWS_EN
extern const char *bt_get_local_name(void);


#define BT_TWS_FEATS        ((TWS_FEAT_TSCO|TWS_FEAT_TSCO_RING)*BT_TSCO_EN)

//bit[7:6]: pair mode
//0: 使用蓝牙名称配对
//1: 使用蓝牙ID配对
//bit[3:0]: pair operation
//0: 使用内置自动配对，上电先回连TWS，再回连手机
//1: 使用api操作，不自动配对，上电不回连
//2: 使用按键调用内置配对，上电先回连TWS，再回连手机
#define TWS_PAIR_MODE       ((BT_TWS_PAIR_MODE<<6) | 0)



uint8_t  cfg_bt_tws_pair_mode            = TWS_PAIR_MODE;
uint16_t cfg_bt_tws_feat                 = BT_TWS_FEATS;
uint16_t cfg_bt_tws_sup_to_connect_times = BT_TWS_TIME_OUT_RECONNECT_TIMES;     //TWS 掉线回连次数,1大约5.12S，0XFF一直回连
//uint8_t cfg_bt_tws_force_slave           = 1;                  //TWS 某一边强制做slave，特定方案使用

////上电回连TWS次数，次数越多，时间越长，成功率越大
////注意：不小于5次
//uint32_t bt_get_tws_connect_times(void)
//{
//    return 5;//1.28s * n
//}
//
////上电回连手机次数，次数越多，时间越长，成功率越大
////注意：不小于5次
//uint32_t bt_get_nor_connect_times(void)
//{
//    return 5;//1.28s * n
//}

////tws配对后，是否只允许固定的主副箱连接
////需要重新配对时，调用bt_tws_delete_link_info()删除配对信息
//bool bt_tws_is_chk_link(void)
//{
//    return true;
//}

//设置search slave次数，每次9s（搜索5秒 + 空闲4秒）
//uint8_t bt_tws_get_search_slave_times(void)
//{
//    return 3;
//}

////自定义对箱连接示例, 在func_bt_message增加以下自定义配对消息
//    case KU_PAIR:
//        if(f_bt.tws_had_pair) {                     //判断是否已连接过tws
//            if(!bt_tws_is_connected()) {
//                bt_tws_connect();
//            } else {
//                bt_tws_disconnect();
//            }
//        } else {
//            bt_set_tws_scan(!bt_tws_is_scaning());  //是否打开对箱连接模式
//        }
//        break;
//    case KL_PAIR:
//        if(!f_bt.tws_had_pair && bt_tws_is_scaning() && !bt_tws_is_connected()) {
//            bt_tws_search_slave();                  //搜索并连接副箱
//        }
//        break;

//pair_mode选择ID配对时，用该函数获取ID
uint32_t bt_tws_get_pair_id(void)
{
    return BT_TWS_PAIR_ID;
}

bool bt_tws_name_suffix_replace(char *buf)
{
    if (sys_cb.name_suffix_en) {
        if(buf[0] == 'L') {
            buf[0] = 'R';
            return true;
        } else if(buf[0] == 'R') {
            buf[0] = 'L';
            return true;
        }
    }
    return false;
}

uint8_t bt_tws_get_func_sta(void)
{
    if(func_cb.sta == FUNC_PWROFF) {
        return FUNC_PWROFF;
    }
    return 0xff;
}

bool bt_tws_set_func_sta(uint8_t func_sta)
{
    if(func_sta != 0xff) {
        sys_cb.pwrdwn_tone_en = 1;
//        xcfg_cb.bt_tswi_en = 0;
        func_cb.sta = FUNC_PWROFF;
    }
    return true;
}

#if BT_TWS_EN
//tws搜索配对时，获取channel避免左左（或右右）配对
bool bt_tws_get_channel(uint8_t *channel)
{
    if(xcfg_cb.bt_tws_lr_mode == 9) {
        *channel = sys_cb.tws_left_channel;     //GPIOx有接地为左声道
        return true;
    } else if(10 == xcfg_cb.bt_tws_lr_mode) {   //配置选择为左声道
        *channel = 1;
        return true;
    } else if(11 == xcfg_cb.bt_tws_lr_mode) {   //配置选择为右声道
        *channel = 0;
        return true;
    }

    return false;
}
#endif

//tws一些同步信息，例如EQ, 语言等，param最大是12byte
bool bt_tws_get_info(uint8_t *param)
{
#if LANG_SELECT == LANG_EN_ZH
    param[0] = sys_cb.lang_id & 0x0f;  //bit0~3
#endif
#if EQ_MODE_EN
    param[0] |= (u8)sys_cb.eq_mode<<4;
#endif
    param[1] = sys_cb.hfp_vol;      //同步初始通话音量
    return true;
}

void bt_tws_set_info(uint8_t *param)
{
    uint8_t tmp = 0;
#if LANG_SELECT == LANG_EN_ZH
    tmp = (param[0] & 0x0f);
    if(xcfg_cb.lang_id >= 2 && tmp < 2 && tmp != sys_cb.lang_id) {  //bit0~3
        sys_cb.lang_id = tmp;
        msg_enqueue(EVT_BT_SET_LANG_ID);
    }
#endif
#if EQ_MODE_EN
    tmp = (param[0] & 0xf0)>>4;
    if(tmp < 6 && tmp != sys_cb.eq_mode) {  //bit4~7
        sys_cb.eq_mode = tmp;
        msg_enqueue(EVT_BT_SET_EQ);
    }
#endif
    sys_cb.hfp_vol = param[1];      //同步初始通话音量
    tmp = tmp;      //避免编译警告
}

#endif
