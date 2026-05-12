#ifndef _FUNC_BT_H
#define _FUNC_BT_H

#define BT_WARN_CON             BIT(0)
#define BT_WARN_HID_CON         BIT(1)
#define BT_WARN_TWS_MCON        BIT(2)
#define BT_WARN_TWS_SCON        BIT(3)
#define BT_WARN_DISCON          BIT(4)
#define BT_WARN_HID_DISCON      BIT(5)
#define BT_WARN_PAIRING         BIT(6)
#define BT_WARN_TWS_DISCON      BIT(7)
#define BT_WARN_TWS_CON         BIT(8)

enum {
    AEC_PARAM_MIC_ANL_GAIN,
    AEC_PARAM_MIC_DIG_GAIN,
    AEC_PARAM_MIC_POST_GAIN,
    AEC_PARAM_NOISE,
    AEC_PARAM_LEVEL,
    AEC_PARAM_OFFSET,
};

typedef struct{
    u16 warning_status;
    u8 disp_status;
    u8 hid_menu_flag;
    u8 hid_discon_flag;
    u8 siri_kl_flag;
    u8 user_kl_flag;
    u8 tws_status;
    u8 autoplay;            //蓝牙连接后自动播放控制
    u8 ring_stop;
    u8 bt_is_inited;
#if BT_SCO_DBG_EN
    u8 mic_anl_gain;
    u8 mic_dig_gain;
    u8 aec_level;
    u8 aec_offset;
    u16 noise;
#endif
    u8 rec_pause    : 1;
    u8 pp_2_unmute  : 1;
    u8 need_pairing : 1;
    u8 tws_had_pair : 1;
#if DAC_OFF_FOR_BT_CONN_EN
    u8 dac_sta      : 1;
#endif // DAC_OFF_FOR_BT_CONN_EN
    // lcd display相关变量
    u8 draw_update  : 1,
       sta_update   : 1;

} func_bt_t;

extern func_bt_t f_bt;
extern int sfunc_bt_call_flag;

void sfunc_bt_ring(void);
void sfunc_bt_ring_message(u16 msg);

void sfunc_bt_call(void);
void sfunc_bt_call_message(u16 msg);

void sfunc_bt_ota(void);

void func_bt_status(void);
void func_bt_message(u16 msg);
void func_bt_mp3_res_play(u32 addr, u32 len);
void func_bt_switch_voice_lang(void);
u16 func_bt_chkclr_warning(u16 bits);
void bt_music_rec_init(void);
void func_bt_sco_rec_init(void);

void sco_audio_set_param(u8 type, u16 value);
u8 func_bt_tws_get_channel(void);

void bt_call_volume_change(u16 msg);
bool user_def_key_msg(u8 func_sel);
void func_bt_exit(void);
void func_bt_init(void);
void func_bt_chk_off(void);
void func_bt_sub_process(void);
void func_bt_process(void);
void func_bt_enter(void);

#if (GUI_SELECT != GUI_NO)
void func_bt_display(void);
void func_bt_enter_display(void);
void func_bt_exit_display(void);
#else
#define func_bt_display()
#define func_bt_enter_display()
#define func_bt_exit_display()
#endif
bool simple_keyboard_message(u16 msg);
bool hid_message(u16 msg);
#endif // _FUNC_BT_H
