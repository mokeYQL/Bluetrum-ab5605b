#include "include.h"
#include "func.h"
#include "func_bt.h"

#if FUNC_BT_EN

void sbc_decode_exit(void);
void mp3_res_play_kick(u32 addr, u32 len);

#if BT_HFP_RING_NUMBER_EN && !MUTIL_LANG_TBL_EN

const res_addr_t ring_num_tbl[10] = {
    {&RES_BUF_NUM_0_MP3, &RES_LEN_NUM_0_MP3},
    {&RES_BUF_NUM_1_MP3, &RES_LEN_NUM_1_MP3},
    {&RES_BUF_NUM_2_MP3, &RES_LEN_NUM_2_MP3},
    {&RES_BUF_NUM_3_MP3, &RES_LEN_NUM_3_MP3},
    {&RES_BUF_NUM_4_MP3, &RES_LEN_NUM_4_MP3},
    {&RES_BUF_NUM_5_MP3, &RES_LEN_NUM_5_MP3},
    {&RES_BUF_NUM_6_MP3, &RES_LEN_NUM_6_MP3},
    {&RES_BUF_NUM_7_MP3, &RES_LEN_NUM_7_MP3},
    {&RES_BUF_NUM_8_MP3, &RES_LEN_NUM_8_MP3},
    {&RES_BUF_NUM_9_MP3, &RES_LEN_NUM_9_MP3},
};

const res_addr_t *res_get_ring_num(u8 index)
{
    return &ring_num_tbl[index];
}
#endif // BT_HFP_RING_NUMBER_EN

#if WARNING_BT_INCALL
typedef struct {
    u8 cur;
    s8 gain_offset;
    u8  ring_cnt;
    u8 ios_ring_sta;
    u32 ios_ring_tick;
    u32 ring_tick;
} t_play_cb;

struct {
    u8 buf[25];
    u8 len;
    t_play_cb play;
    u8 ring_sync;
} f_bt_ring;

void bt_ring_stop(void)
{
    f_bt.ring_stop = 1;
    bt_tws_user_key(0x101);
}

void hfp_ring_number_clear(void)
{
    f_bt_ring.len = 0;
    memset(&f_bt_ring.buf,0x00,8);
}

u8 hfp_notice_ring_number(uint8_t index, char *buf, u32 len)
{
    bt_update_redial_number(index, (char *)buf, len);
    if(f_bt_ring.len == 0) {
        u32 i;
        for(i = 0; i < len; i++){
            if (buf[i] >= '0' && buf[i] <= '9') {
                f_bt_ring.buf[i] = buf[i] - 0x30;
            } else {
                f_bt_ring.buf[i] = 0xff;
            }
        }
        f_bt_ring.len = len;
        return 1;
    }
    return 0;
}

void bsp_bt_get_remote_phone_number(void)
{
    hfp_ring_number_clear();
    bt_call_get_remote_phone_number();
}

AT(.text.func.btring)
static void sfunc_bt_ring_res_play(u32 addr, u32 len)
{
//    printf("%s: addr: %x, len: %x\n", __func__, addr, len);
    if (len == 0) {
        return;
    }
    if (sys_cb.mute) {
        bsp_loudspeaker_unmute();
        delay_5ms(6);
    }
    mp3_res_play_kick(addr, len);
}
#endif

#if BT_HSF_CALL_NUMBER_EN
AT(.text.func.btring)
bool hfp_is_list_curr_calls(void)
{
    return true;
}

AT(.text.func.btring)
void hfp_hf_emit_curr_calls(u8 idx, u8 type, const char * number, u32 len){
    printf("number: %s\n", number);
    bt_update_redial_number(idx, (char *)number, len);
}
#endif

#if WARNING_BT_INCALL
AT(.text.func.btring)
static void sfunc_bt_ring_sync(void)
{
#if BT_TWS_EN
    bt_tws_sync_run(f_bt_ring.ring_sync);           //第一次响铃同步用6，后续用4
#endif
    f_bt_ring.ring_sync = 4;
}
#endif // WARNING_BT_INCALL

AT(.text.func.btring)
static void sfunc_bt_ring_process(void)
{
    func_process();
    func_bt_sub_process();

#if WARNING_BT_INCALL
    if (bt_is_ring() && (get_music_dec_sta() == MUSIC_STOP) && (f_bt.disp_status == BT_STA_INCOMING) && (!f_bt.ring_stop)) {
        if (!f_bt_ring.play.ios_ring_sta) {
            music_control(MUSIC_MSG_STOP);
            if (sys_cb.mute) {
                bsp_loudspeaker_mute();
            }
        }

#if BT_HFP_RING_NUMBER_EN
        if(bt_hfp_ring_number_en() && f_bt_ring.play.ring_cnt >= BT_HFP_RINGS_BEFORE_NUMBER && f_bt_ring.play.cur < f_bt_ring.len) {
            if (f_bt_ring.buf[f_bt_ring.play.cur] != 0xff) {
                const res_addr_t *p = res_get_ring_num(f_bt_ring.buf[f_bt_ring.play.cur]);
                sfunc_bt_ring_sync();
                sfunc_bt_ring_res_play(*p->ptr, *p->len);
            }
            f_bt_ring.play.cur++;
            f_bt_ring.play.ring_tick = tick_get();
        } else
#endif // BT_HFP_RING_NUMBER_EN
        {
#if BT_HFP_PLAY_IOS_RING_EN
            if (xcfg_cb.bt_hfp_play_ios_ring_en) {
                if (sco_is_connected()) {
                    if (f_bt_ring.play.ios_ring_sta != 2) {
                        f_bt_ring.play.ios_ring_sta = 2;
                        bt_audio_enable();
                        bsp_change_volume(sys_cb.vol);                                    //解决开菜单功能后偶尔本地铃声无声
                        dac_fade_in();
                        //printf("ios ring\n");
                    }
                } else if (!tick_check_expire(f_bt_ring.play.ios_ring_tick, 3500)) {      //等待2.5S
                    if (f_bt_ring.play.ios_ring_sta != 2) {
                        f_bt_ring.play.ios_ring_sta = 1;
                    }
                } else {
                      if (f_bt_ring.play.ios_ring_sta == 1) {
                          f_bt_ring.play.ios_ring_sta = 0;                                  //超时还未建立sco的话，播放本地ring
                      }
                }
            }
#endif // BT_HFP_PLAY_IOS_RING_EN
            if ((!f_bt_ring.play.ios_ring_sta) && tick_check_expire(f_bt_ring.play.ring_tick, 2500)) {
                sfunc_bt_ring_sync();
                sfunc_bt_ring_res_play(RES_BUF_RING_MP3, RES_LEN_RING_MP3);
                f_bt_ring.play.ring_cnt++;
                f_bt_ring.play.ring_tick = tick_get();
            }
        }
    }
#endif
}

static void sfunc_bt_ring_enter(void)
{
#if EXLIB_BT_MONO_XDRC_EN
    sys_cb.xdrc_sysvol_en = is_xdrc_sysvol_en();
    xdrc_sysvol_en_set(0);
#endif

#if WARNING_BT_INCALL
    memset(&f_bt_ring.play, 0, sizeof(t_play_cb));
    f_bt_ring.play.gain_offset = sys_cb.anl_gain_offset;
    f_bt_ring.play.ring_tick = tick_get() - 5000;
    f_bt_ring.play.ios_ring_tick = tick_get();
    f_bt.ring_stop = 0;
    dac_set_anl_offset(0);
    bt_audio_bypass();
    music_control(MUSIC_MSG_STOP);
    bsp_change_volume(WARNING_VOLUME);

#if BT_TSCO_EN
    bt_tws_sync_ring_tone();
#else
    //先报号码，再响铃
    for(int i=0; i<30; i++) {
        func_bt_status();
        if(f_bt_ring.len != 0 || !bt_is_ring() || (f_bt.disp_status != BT_STA_INCOMING)) {
            break;
        }
        delay_5ms(10);
    }
#endif
    f_bt_ring.ring_sync = 6;
#endif // WARNING_BT_INCALL
}

static void sfunc_bt_ring_exit(void)
{
#if EXLIB_BT_MONO_XDRC_EN
    xdrc_sysvol_en_set(sys_cb.xdrc_sysvol_en);
#endif

#if BT_TWS_EN
    bt_tws_clr_ring_tone();
#endif
#if BT_HFP_PLAY_IOS_RING_EN
    if (f_bt_ring.play.ios_ring_sta == 2) {
        bt_audio_bypass();
    }
#endif
#if WARNING_BT_INCALL
    f_bt_ring.len = 0;
    music_control(MUSIC_MSG_STOP);
    sys_cb.anl_gain_offset = f_bt_ring.play.gain_offset;
    bsp_change_volume(sys_cb.vol);
    delay_5ms(20);
    bt_audio_enable();
#endif
#if SYS_KARAOK_EN
    if((f_bt.disp_status != BT_STA_INCALL) && (func_cb.sta == FUNC_BT)) {
        bsp_karaok_init(AUDIO_PATH_KARAOK, FUNC_BT);
    }
#endif // SYS_KARAOK_EN
}

AT(.text.func.btring)
void sfunc_bt_ring(void)
{
    printf("%s\n", __func__);

    func_process();
    sfunc_bt_ring_enter();
#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif // SYS_KARAOK_EN
    while ((f_bt.disp_status == BT_STA_INCOMING) && (func_cb.sta == FUNC_BT)) {
        sfunc_bt_ring_process();
        sfunc_bt_ring_message(msg_dequeue());
        func_bt_display();
    }

    sfunc_bt_ring_exit();
}

#endif //FUNC_BT_EN
