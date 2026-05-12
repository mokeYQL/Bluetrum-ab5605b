#include "include.h"
#include "api.h"


void bt_new_name_init(void);
FRESULT fs_open(const char *path, u8 mode);
FRESULT fs_read (void* buff, UINT btr, UINT* br);
#define param_write(a, b, c)    cm_write(PAGE0(a), b, c)

/********************************************************/
extern void bt_tws_set_operation(uint8_t *cmd);


#if 0
//是否支持android & ios播放暂停快速切换，需要时定义该函数
//注意：打开后ios播放微信小视频会无声，播放器听音乐不影响
//      部分android4.0及之前系统打开多个播放器可能有兼容问题
bool bt_force_super_fast_status(void)
{
    return true;
}
#endif

#if 0
//是否支持ios播放暂停快速切换，需要时定义该函数，蓝牙后台建议打开
//注意：打开后ios播放微信小视频会无声，播放器听音乐不影响
bool bt_support_ios_fast_status(void)
{
    return true;
}
#endif
void bsp_bt_init(void)
{
    //更新配置工具的设置
    cfg_bt_rf_def_txpwr = 0;  //xcfg_cb.bt_txpwr;
    cfg_bt_page_inq_txpwr = xcfg_cb.bt_rf_txpwr_recon;
#if BT_HID_SIMPLE_KEYBOARD
    cfg_bt_support_profile = (PROF_HID*BT_HID_EN) | (PROF_HFP*BT_HFP_EN*xcfg_cb.bt_sco_en);
#else
    cfg_bt_support_profile = (PROF_A2DP*BT_A2DP_EN*xcfg_cb.bt_a2dp_en) | (PROF_HFP*BT_HFP_EN*xcfg_cb.bt_sco_en)\
        | (PROF_SPP*BT_SPP_EN*(xcfg_cb.bt_spp_en||xcfg_cb.eq_dgb_spp_en)) | (PROF_HID*BT_HID_EN*xcfg_cb.bt_hid_en) | (PROF_HSP*BT_HSP_EN*xcfg_cb.bt_sco_en)\
        | (PROF_PBAP*BT_PBAP_EN) | (PROF_MAP*BT_MAP_EN);
#endif

#if BT_A2DP_VOL_CTRL_EN
    if(!xcfg_cb.bt_a2dp_vol_ctrl_en) {
        cfg_bt_a2dp_feature &= ~A2DP_VOL_CTRL;
    } else {
        cfg_bt_a2dp_feature |= A2DP_VOL_CTRL;
    }
#endif
    cfg_bt_dual_mode = BT_DUAL_MODE_EN * xcfg_cb.ble_en;
    cfg_bt_max_acl_link = BT_2ACL_EN * xcfg_cb.bt_2acl_en + 1;
#if BT_TWS_EN
    cfg_bt_tws_mode = BT_TWS_EN * xcfg_cb.bt_tws_en;
    cfg_bt_tws_pair_mode &= ~0x0f;
    cfg_bt_tws_pair_mode |= xcfg_cb.bt_tws_pair_mode & 0x0f;

    if(xcfg_cb.bt_tws_lr_mode > 8) {    //开机时PWRKEY可能按住，先不检测
        tws_lr_xcfg_sel();
    }
#if BT_TWS_AUTO_SWITCH
    if(xcfg_cb.bt_tswi_en) {
        cfg_bt_tws_feat |= TWS_FEAT_MS_SWITCH;
    } else {
        cfg_bt_tws_feat &= ~TWS_FEAT_MS_SWITCH;
    }
#endif
    if(!cfg_bt_tws_mode){
        cfg_bt_tws_feat = 0;
    }
#endif
#if BT_HFP_EN
    if(!xcfg_cb.bt_hfp_ring_number_en) {
        cfg_bt_hfp_feature &= ~HFP_RING_NUMBER_EN;
    }
#endif

#if BT_FCC_TEST_EN
    bt_fcc_init();
#endif

#if FUNC_BTHID_EN
    if (is_bthid_mode()) {
        cfg_bt_support_profile = PROF_HID;
        cfg_bt_max_acl_link = 1;
        cfg_bt_dual_mode = 0;
        cfg_bt_tws_mode = 0;
    }
#endif // FUNC_BTHID_EN

    memset(&f_bt, 0, sizeof(func_bt_t));
    f_bt.disp_status = 0xfe;
    f_bt.need_pairing = 1;  //开机若回连不成功，需要播报pairing
    if (!is_bthid_mode()) {
        f_bt.hid_menu_flag = 1;
    }

    bt_setup();
}

void bsp_bt_close(void)
{
}

#if BT_PWRKEY_5S_DISCOVER_EN
bool bsp_bt_w4_connect(void)
{
    if (xcfg_cb.bt_pwrkey_nsec_discover) {
        while (sys_cb.pwrkey_5s_check) {            //等待检测结束
            WDT_CLR();
            delay_5ms(2);
        }

        //已检测到长按5S，需要直接进入配对状态。播放PAIRING提示音。
        if (sys_cb.pwrkey_5s_flag) {
            return false;
        }
    }
    return true;
}

bool bsp_bt_pwrkey5s_check(void)
{
    bool res = !bsp_bt_w4_connect();
    delay_5ms(2);
    return res;
}

void bsp_bt_pwrkey5s_clr(void)
{
    if (!xcfg_cb.bt_pwrkey_nsec_discover) {
        return;
    }
    sys_cb.pwrkey_5s_flag = 0;
#if !BT_DISCOVER_CTRL_EN    //使用按键打开可被发现时scan_ctrl=1
    cfg_bt_scan_ctrl_mode = 0;   //scan_ctrl=0，则连接断开后会自动打开可被发现
#endif
}
#endif // BT_PWRKEY_5S_DISCOVER_EN

void bsp_bt_vol_change(void)
{
#if BT_A2DP_VOL_CTRL_EN || BT_TWS_EN
    if((xcfg_cb.bt_a2dp_vol_ctrl_en && (bt_get_status() >= BT_STA_CONNECTED)) || bt_tws_is_connected()) {
        bt_music_vol_change();     //通知手机音量已调整
    }
#endif
}

uint bsp_bt_get_hfp_vol(uint hfp_vol)
{
    uint vol;
    vol = (hfp_vol + 1) * sys_cb.hfp2sys_mul;
    if (vol > VOL_MAX) {
        vol = VOL_MAX;
    }
    return vol;
}

void bt_emit_notice(uint evt, u32 param)
{
    switch(evt) {
    case BT_NOTICE_INIT_FINISH:
        print_btrf_sfr();
#if WORK_MODE != MODE_BQB_RF
#if BT_PWRKEY_5S_DISCOVER_EN
        if(bsp_bt_w4_connect()) {
            bt_connect(BT_POWER_UP_RECONNECT_TIMES);
        } else {
            cfg_bt_scan_ctrl_mode = 1;
            bt_set_scan(true, true);        //长按5S开机，直接进bt scanning
        }
#else
        bt_connect(BT_POWER_UP_RECONNECT_TIMES);
#endif
#endif
        break;

    case BT_NOTICE_DISCONNECT:
         printf("BT_NOTICE_DISCONNECT\n");
#if LED_MATRIX_HUART_TX
        huart_tx_mode(3);
#endif
        f_bt.warning_status |= BT_WARN_DISCON;
        bt_redial_reset(((u8 *)param)[0] & 0x01);
        delay_5ms(5);
        break;
    case BT_NOTICE_CONNECTED:
         printf("BT_NOTICE_CONNECTED\n");
#if LED_MATRIX_HUART_TX
        huart_tx_mode(4);
#endif
        f_bt.warning_status |= BT_WARN_CON;
        bt_redial_reset(((u8 *)param)[0] & 0x01);

#if BT_PWRKEY_5S_DISCOVER_EN
        bsp_bt_pwrkey5s_clr();
#endif // BT_PWRKEY_5S_DISCOVER_EN
        delay_5ms(5);
#if BT_HID_SIMPLE_KEYBOARD
        bt_search_os_capacity();   //修正打开HFP时,无法区别ios/android的问题，后面回拨电话时需要区分ios/android去处理
#endif
        break;

    case BT_NOTICE_LOSTCONNECT:
        printf("BT_NOTICE_LOSTCONNECT\n");
        break;

//    case BT_NOTICE_INCOMING:
//    case BT_NOTICE_RING:
//    case BT_NOTICE_OUTGOING:
//    case BT_NOTICE_CALL:
//        break;


    case BT_NOTICE_NETWORK_CALL: //网络通话
        printf("NETWORK_CALL\n");
        break;
    case BT_NOTICE_PHONE_CALL:   //手机通话
        printf("PHONE_CALL\n");
        break;

    case BT_NOTICE_SET_XOSC_CAP:
        break;

    case BT_NOTICE_SET_SPK_GAIN:
        if(param != sys_cb.hfp_vol) {
            sys_cb.hfp_vol = param;
            msg_enqueue(EVT_HFP_SET_VOL);
        }
        break;

    case BT_NOTICE_MUSIC_PLAY:
        msg_enqueue(EVT_A2DP_MUSIC_PLAY);
        break;

    case BT_NOTICE_MUSIC_STOP:
        if (bt_get_disp_status() > BT_STA_PLAYING) {
            break;
        }
        msg_enqueue(EVT_A2DP_MUSIC_STOP);
        break;

    case BT_NOTICE_MUSIC_CHANGE_VOL:
        if(param == 0) {
            msg_enqueue(KU_VOL_DOWN);
        } else {
            msg_enqueue(KU_VOL_UP);
        }
        break;
    case BT_NOTICE_HID_CONN_EVT:
        printf("BT_NOTICE_HID_CONN_EVT : %d \n",param);
#if BT_HID_MENU_EN
        if (xcfg_cb.bt_hid_menu_en) {
            if (f_bt.hid_menu_flag == 2) {
                //按键连接/断开HID Profile完成
                if (param) {
                    f_bt.warning_status |= BT_WARN_HID_CON;
                } else {
                    f_bt.warning_status |= BT_WARN_HID_DISCON;
                }
                f_bt.hid_menu_flag = 1;
            }
    #if BT_HID_DISCON_DEFAULT_EN
            else if (f_bt.hid_menu_flag == 1) {
                if ((param) & (xcfg_cb.bt_hid_discon_default_en)) {
                    f_bt.hid_discon_flag = 1;
                }
            }
    #endif // BT_HID_DISCON_DEFAULT_EN
        }
#endif // BT_HID_MENU_EN
        break;

#if BT_TWS_EN
//    case BT_NOTICE_TWS_SEARCH_TIMEOUT:
//        break;
//    case BT_NOTICE_TWS_CONNECT_START:
//        break;
    case BT_NOTICE_TWS_DISCONNECT:
        f_bt.tws_status = 0;
        f_bt.warning_status |= BT_WARN_TWS_DISCON;      //TWS断线不播报提示音，仅更改声道配置
        break;
    case BT_NOTICE_TWS_CONNECTED:
#if LE_EN
        if(bt_tws_is_slave()) {
            ble_disable_adv();
        }
#endif
        f_bt.tws_status = ((u8 *)param)[0];
        if(f_bt.tws_status & 0x08) {
            f_bt.warning_status |= BT_WARN_TWS_CON;     //无连接提示音，仅更改声道配置
        } else if(f_bt.tws_status & 0x80) {
            f_bt.warning_status |= BT_WARN_TWS_SCON;    //TWS连接提示音
        } else {
            f_bt.warning_status |= BT_WARN_TWS_MCON;    //TWS连接提示音
        }
        break;
    case BT_NOTICE_TWS_LOSTCONNECT:
        break;
    case BT_NOTICE_TWS_RES_PLAY:
        sys_cb.tws_res_brk = 1;    //打断当前提示音
        tws_res_add(*(u8 *)param, *(u32 *)(param+1));
        break;

    case BT_NOTICE_TWS_SET_VOL:
        param = (param+1) * VOL_MAX / 128;
        sys_cb.vol = param;
        if (sys_cb.vol > VOL_MAX) {
            sys_cb.vol = VOL_MAX;
        }
        msg_enqueue(EVT_TWS_SET_VOL);
        break;

    //TWS自定义按键,通过bt_tws_user_key函数发送
    case BT_NOTICE_TWS_USER_KEY:
        if (param == 0x101) {
            f_bt.ring_stop = 1;
        }
        break;
    case BT_NOTICE_TWS_USER_PARAM:
//        printf("BT_NOTICE_TWS_USER_PARAM\n");
//        print_r((u8*)param,10);
        break;
#endif

    case BT_NOTICE_MUSIC_SET_VOL:
        param = (param+1) * VOL_MAX / 128;
        if(param != sys_cb.vol) {
            sys_cb.vol = param;
            if (sys_cb.vol > VOL_MAX) {
                sys_cb.vol = VOL_MAX;
            }
            msg_enqueue(EVT_A2DP_SET_VOL);
        }
        break;
    case BT_NOTICE_SSP_NUMERIC_VALUE:
        printf("param:%d\n",param);
        bt_ssp_numeric_comparison_res(1);  //配对应答，1：同意，0：拒绝
        break;
    case BT_NOTICE_PULLPHONEBOOK_FINISH:
        break;

    case BT_NOTICE_RETRY_CONNECT:
        printf("[%d|%d]BT_NOTICE_RETRY_CONNECT: %d\n", BT_POWER_UP_RECONNECT_TIMES,BT_TIME_OUT_RECONNECT_TIMES,*(u16*)(param+1));
        break;
    }
}

#if USB_BT_NAME_UPDATA
u8 write_name_len = 0;
#define BT_NAME_UPD                   "bt_name.btn"
const char upd_bt_name[14] = BT_NAME_UPD;
char bt_name_buf[34];
char buff[512];
int bt_name_updata_init(const char *file)
{
    uint len;
    char *bt_name = bt_name_buf;
    FRESULT res = fs_open(file, FA_READ);
    if (res != FR_OK) {
//        printf("bt_name file open fail\n");
        return -1;
    }
    memset(bt_name, 0, 34);
    FRESULT res_read = fs_read(buff, 512, &len);
    memcpy(bt_name_buf, buff, 34);
    if ((res_read == FR_OK) && (len < 34)) {
        if ((bt_name[0] == 0x23) && (bt_name[1] == 0x23) && (bt_name[len-3] == 0x23) && (bt_name[len-4] == 0x23)) {
            return len;
        } else {
//            printf("##--error--##\n");
            return -1;
        }
    } else {
//        printf("##--FR_NO_OK--##\n");
        return -1;
    }

}

void bt_name_updata(void)
{
    int res;
    res = bt_name_updata_init(upd_bt_name);
    if (res == -1) {
        return;
    }
    if (res) {
        write_name_len = res - 2;
        param_write(bt_name_buf, BT_NAME_UPDATA, write_name_len);
        cm_sync();
        printf("write_name_ok\n");
        mp3_res_play(RES_BUF_UPDATE_MP3, RES_LEN_UPDATE_MP3);
    }
}
#endif // USB_BT_NAME_UPDATA
#if BT_TWS_EN
//TWS蓝牙名称是否增加_L或_R后缀
void bt_name_suffix_set(void)
{
    sys_cb.name_suffix_en = 0;
    if (xcfg_cb.bt_tws_name_suffix_en) {
        if ( (xcfg_cb.bt_tws_lr_mode < 9) ||
             (xcfg_cb.bt_tws_lr_mode == 9 && xcfg_cb.tws_sel_left_gpio_sel == 0) ) {
            return;
        }

        if (strlen(xcfg_cb.bt_name) < 30) {
            if (sys_cb.tws_left_channel) {
                strcat(xcfg_cb.bt_name, "L");
            } else {
                strcat(xcfg_cb.bt_name, "R");
            }
            sys_cb.name_suffix_en = 1;
            printf("bt_name_suffix: %s\n", xcfg_cb.bt_name);
        }
    }
}
#endif // BT_TWS_EN
#if BT_MAP_EN
char * hfp_get_at_cmd(void) {
//    return "AT+VTS=1;"; //例如，通话过程发起号码键"1"
//    return "ATD10086;"; //也可以，发起回拨号码"10086"
    return "AT+CCLK?\r";//获取IOS手机时间（安卓暂不支持），获取回调函数hfp_get_time
}
//
////获取IOS手机时间（安卓暂不支持）,需要先发送"AT+CCLK?\r"AT命令
void bt_get_time(char *ptr, uint32_t len, uint8_t format);
void hfp_notice_network_time(u8 *buf, u16 len)
{
    bt_get_time((char *)buf, len, 1);
}
#endif

//void fcc_param_print(struct vs_test_ctrl_cmd  const *param)
//{
//    my_printf("\nFCC------------------------------------------------>\n");
//    my_printf("mode(0 is fcc) = %d\n",param->mode);
//    my_printf("test_scenario = %d\n",param->test_scenario);
//    my_printf("hopping_mode = %d\n",param->hopping_mode);
//    my_printf("tx_freq = %d\n",param->tx_freq);
//    my_printf("rx_freq = %d\n",param->rx_freq);
//    my_printf("power_control = %d\n",param->power_control);
//    my_printf("poll_period = %d\n",param->poll_period);
//    my_printf("packet_type = %d\n",param->packet_type);
//    my_printf("payload_length = %d\n\n",param->payload_length);
//}
