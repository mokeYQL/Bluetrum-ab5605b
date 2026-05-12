/**********************************************************************
*
*   strong_symbol.c
*   定义库里面部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

#if !FUNC_USBDEV_EN
void usb_dev_isr(void){}
void ude_ep_reset(void){}
void ude_control_flow(void){}
void ude_isoc_tx_process(void){}
void ude_isoc_rx_process(void){}
void lock_code_usbdev(void){}
#endif //FUNC_USBDEV_EN

#if (REC_TYPE_SEL != REC_MP3)
int mpa_encode_frame(void) {return 0;}
#endif //(REC_TYPE_SEL != REC_MP3)

#if (REC_TYPE_SEL != REC_SBC)
bool sbc_encode_init(u8 spr, u8 nch){return false;}
u16 sbc_encode_frame(u8 *buf, u16 len) {return 0;}
void sbc_encode_process(void){}
void sbc_encode_exit(void){}
#endif

#if (REC_TYPE_SEL != REC_ADPCM && !BT_HFP_REC_EN)
void adpcm_encode_process(void){}
#endif //(REC_TYPE_SEL != REC_ADPCM)

#if !MUSIC_WAV_SUPPORT
int wav_dec_init(void){return 0;}
bool wav_dec_frame(void){return false;}
void lock_code_wavdec(void){}
#endif // MUSIC_WAV_SUPPORT

#if !MUSIC_WMA_SUPPORT
int wma_dec_init(void){return 0;}
bool wma_dec_frame(void){return false;}
void lock_code_wmadec(void){}
#endif // MUSIC_WMA_SUPPORT

#if !MUSIC_APE_SUPPORT
int ape_dec_init(void){return 0;}
bool ape_dec_frame(void){return false;}
void lock_code_apedec(void){}
#endif // MUSIC_APE_SUPPORT

#if !MUSIC_FLAC_SUPPORT
int flac_dec_init(void){return 0;}
bool flac_dec_frame(void){return false;}
void lock_code_flacdec(void){}
#endif // MUSIC_FLAC_SUPPORT

#if !MUSIC_SBC_SUPPORT
int sbcio_dec_init(void){return 0;}
bool sbcio_dec_frame(void){return false;}
#endif // MUSIC_SBC_SUPPORT

#if !FMRX_REC_EN
void fmrx_rec_start(void){}
void fmrx_rec_stop(void){}
#endif // FMRX_REC_EN

#if !BT_REC_EN
void bt_music_rec_start(void) {}
void bt_music_rec_stop(void) {}
#endif

#if !USB_SUPPORT_EN
void usb_isr(void){}
void usb_init(void){}
#endif

#if ((!SD_SUPPORT_EN) && (!FUNC_USBDEV_EN))
void sd_disk_init(void){}
void sdctl_isr(void){}
void sd_disk_switch(u8 index){}
bool sd0_stop(bool type){return false;}

bool sd0_init(void){return false;}
bool sd0_read(void *buf, u32 lba){return false;}
bool sd0_write(void* buf, u32 lba){return false;}

#endif

#if !FUNC_MUSIC_EN
u32 fs_get_file_size(void){return 0;}
void fs_save_file_info(unsigned char *buf){}
void fs_load_file_info(unsigned char *buf){}
#endif // FUNC_MUSIC_EN

#if !BT_TWS_EN
void btstack_tws_init(void){}
void a2dp_play_init(void){}
AT(.bcom_text.sbc.play)
void a2dp_play_reset(void){}
AT(.bcom_text.sbc.play)
void a2dp_cache_reset(void){}

AT(.bcom_text.sbc_cache)
bool a2dp_cache_fill(uint8_t *packet, uint16_t size) {
    return true;
}

AT(.bcom_text.sbc_cache)
uint8_t avdtp_fill_tws_buffer(u8 *ptr, uint len) {
    return 0;
}

AT(.bcom_text.sbc_cache)
uint8_t a2dp_cache_before_rx(uint8_t *data_ptr, uint16_t data_len) {
    return 0;
}

AT(.sbcdec.sbc.cache)
uint8_t *cache_write_buf(uint8_t *packet, uint32_t frame_size)
{
    return NULL;
}

AT(.sbcdec.sbc.cache)
uint8_t *cache_read_buf(uint8_t *cache_ptr, uint32_t frame_size)
{
    return NULL;
}

AT(.sbcdec.sbc.cache)
void cache_free_for_ptr(uint8_t *cache_ptr, uint16_t size)
{
}

AT(.sbcdec.code)
void tws_deg_clock(void)
{
}

AT(.sbcdec.code)
void sbc_cache_free(void) {
}

AT(.sbcdec.code)
size_t sbc_cache_read(uint8_t **buf) {
    return 0;
}

AT(.sbcdec.code)
void sbc_tws_kick_play(void)
{
}

AT(.sbcdec.code)
bool sbc_tws_dec_frame(void)
{
    return false;
}

AT(.com_text.bb.tws)
void tws_tick_isr(void){
}

AT(.bcom_text.bb.btisr)
void lc_tws_hssi_instant(uint8_t Lid) {
}

AT(.bcom_text.bb.btisr)
uint8_t lc_get_ticks_status(void) {
    return 0x80;
}

uint8_t lc_tws_set_spr(uint8_t Lid, uint SprIdx, uint32_t Hssi) {
    return 0;
}

AT(.bcom_text.sbc.play)
void tws_ticks_trigger(uint32_t ticks) {
}

AT(.bcom_text.sbc.play)
uint32_t tws_get_play_ticks(uint16_t seq_num, uint32_t duration) {
    return 0;
}

AT(.com_text.sbc.play)
void tws_trigger_isr(void) {
}

AT(.bcom_text.sbc.play)
bool tws_cache_is_empty(void) {
    return false;
}

AT(.bcom_text.sbc.play)
void sbc_cache_env_reset(void)
{
}

AT(.bcom_text.sbc.send)
void tws_send_pkt(void)
{
}

AT(.bcom_text.cache)
bool cache_check_for_alloc(uint32_t frames, uint32_t data_len)
{
    return true;
}

void tws_remote_set_ledcnt(uint8_t *param)
{
}
void tws_remote_req_ledcnt(void)
{
}
void bt_tws_get_local_guid(uint8_t *buf, uint8_t buf_len)
{
}
#endif

#if !BT_TWS_EN || !BT_TSCO_EN
void lc_tsco_init(void) {
}
AT(.bcom_text.bb.tws.sco)
void ext_lc_tsco_flush_txbuf(uint8_t lid) {
}
AT(.bcom_text.bb.tws.sco)
void ext_lc_fill_tsco_dat(void) {
}
AT(.bcom_text.bb.tws.sco)
void ext_lc_tsco_set_ticks(int32_t offset) {
}
AT(.bcom_text.bb.tws.sco)
bool ext_lc_tsco_get_active_acl(uint32_t clock, uint8_t *next_lid) {
    return false;
}
AT(.bcom_text.bb.tws.sco)
bool ext_lc_tsco_rx(void *conn, uint8_t rx_type, uint8_t *ptr) {
    return false;
}
AT(.bcom_text.bb.tws.sco)
bool ext_lc_tsco_tx_ack(void *conn, uint8_t tx_type) {
    return false;
}
AT(.bt_voice.bb.tws.sco)
uint8_t ext_lc_tsco_get_status(void) {
    return 0;
}
void lc_tsco_send_setup(uint8_t sco_lid, void *p_air) {
}
void lc_tsco_send_kill(uint8_t sco_lid) {
}
#else
bool bt_tsco_is_en(void) {
    return BT_TSCO_EN;
}
#endif

#if !BT_FCC_TEST_EN
void huart_init(void)
{
}
AT(.bcom_text.stack.uart_isr)
bool bt_uart_isr(void) {
    return false;
}
#endif

#if !BT_HFP_REC_EN
AT(.com_text.bt_rec)
void bt_sco_rec_mix_do(u8 *buf, u32 samples) {}
void bt_sco_fill_remote_buf(u16 *buf, u16 samples) {}
#endif

AT(.com_text.bb.btisr)
void avdtp_acl2_sbc_compare_buf(uint8_t *data_ptr){};

#if !SCO_MAV_EN
void sco_mav_process(u8 *ptr, u32 samples, int ch_mode) {}
#endif

#if ((!SYS_MAGIC_VOICE_EN) && (!SCO_MAV_EN))
void magic_voice_process(void) {}
void mav_kick_start(void) {}
#endif

#if (!AUDIO_STRETCH_EN)
void stretch_dual_voice_process(void *buf, u16 frame_len){}
void speed_kick_start(void){}
void speed_voice_process(void){}
#endif

#if !EXSPI_REC_PITCH_SHIFT_PLAY
AT(.com_text.mp3_pitch)
void mp3_pitch_shift_process(void *audmabuf, u32 frame_len){};
#endif

#if !FUNC_SPDIF_EN && !FUNC_HDMI_EN
void spdif_pcm_process(void){}
bool spdif_smprate_detect(void) {    return false;}
AT(.com_text.isr.spdif)
void spdif_isr(u32 spfrx_pnd){}
#endif

#if !FUNC_SPDIF_TX_EN
AT(.com_text.isr.spdif)
void spdif_tx_isr(u32 spftx_pnd) {}
#endif

#if ((!MUSIC_UDISK_EN)&&(!MUSIC_SDCARD_EN)&&(!MUSIC_SDCARD1_EN))
FRESULT fs_open(const char *path, u8 mode){return 0;}
FRESULT fs_read (void* buff, UINT btr, UINT* br){return 0;}
FRESULT fs_lseek (DWORD ofs, u8 whence){return 0;}
#endif

#if !KARAOK_REC_EN
void karaok_rec_process(u8 *ptr) {}
AT(.com_text.karaok.rec)
bool karaok_rec_fill_buf(u8 *buf, u16 len) {return false;}
#endif

#if !I2S_DMA_EN
void i2s_isr(void) {}
void i2s_process(void) {}
#endif

#if !LE_EN
AT(.bcom_text.bb.leisr)
void ble_isr(void) {}
AT(.bcom_text.bb.leisr)
void ble_evt_instant(void *evt, uint32_t *next_basetimecnt, bool role) {}
AT(.bcom_text.bb.leisr)
void ble_evt_schedule(void) {}
AT(.bcom_text.bb.leisr)
uint32_t ble_evt_time_get(void) { return 0; }
AT(.bcom_text.bb.leisr)
uint8_t mgr_set_adv_data(void const *param) { return 0; }
AT(.bcom_text.bb.leisr)
void ble_lm_adv_data_update(void) {}
AT(.bcom_text.bb.leisr)
void ble_lc_check_update_evt_sent(uint16_t conhdl, void *evt_new) {}
AT(.bcom_text.bb.leisr)
void ble_evt_free_do(void *evt) {}

void ll_con_update_ind(uint16_t conhdl, void *evt_new, void *ll_env_ptr,uint16_t msg_id_con, uint16_t msg_id_sup) {}
void ll_con_update_cmd_complete_send(uint8_t status, uint16_t conhdl, void *evt, uint16_t msg_id) {}
void ll_map_update_ind(uint16_t conhdl, void *param) {}
void ll_common_nb_of_pkt_comp_evt_send(uint16_t conhdl, uint8_t nb_of_pkt, uint16_t msg_id) {}

void ble_ll_init(void) {}
void ble_ll_reset(void) {}
bool ble_event_cmd_complete(uint8_t *packet, int size) { return false; }
void ble_event_meta(uint8_t *packet, int size) {}
int att_server_notify_do(void) { return 1;}
void hci_run_le_connection(void) {}
void btstack_ble_init(void) {}
void btstack_ble_send(void) {}
void btstack_ble_update_conn_param(void) {}
void btstack_ble_set_adv_interval(void) {}
void ble_set_adv_interval(u16 interval) {}
void btstack_adv0_ctrl(uint opcode){}
uint8_t ble_adv0_get_adv_en(void) {return 0;}
#endif

#if !BT_PBAP_EN
void pbap_client_init(void) {}
void btstack_pbap(uint16_t param) {}
void btstack_pbap_pullphonebook_continue_time(uint16_t time){}
#endif
#if !BT_MAP_EN
void map_client_init(void) {}
void btstack_map(uint param) {}
void bt_get_time(char *ptr, uint32_t len, uint8_t format){}
#endif
#if !(BT_PBAP_EN || BT_MAP_EN)
void goep_client_init(void) {}
#endif

#if !BT_TOUCH_SCREEN_EN
void btstack_hid_touch_screen(uint16_t keycode){}
const uint8_t sdp_hid_car_tscreen_service_record[1] = {0};
#endif

#if 1//!DAC DRC EN && !DAC SOFT EQ EN //已弃用
int soft_eq_drc_init(void *param) { return -1; }
void soft_eq_drc_stereo_proc(s16 *ldata, s16 *rdata) {}
void soft_eq_drc_mono_proc(s16 *data) {}
void drc_set_param(const u32 *tbl) {}
#endif

#if !SYS_KARAOK_EN
void set_src0_vol(u16 vol) {}
#endif

#if !MIC_EQ_EN
AT(.com_text.mic_eq)
void sdadc_pcm_peri_eq(u8 *ptr, u32 samples){}
#endif

#if SYS_KARAOK_EN
AT(.text.fmrx)
void fmrx_auto_sysclk_switch(u16 freq){};
#endif

#if !KARAOK_DENOISE_EN
AT(.com_text.mic_dnr)
void micdnr_process(void *buf, u32 samples) {}
#endif

#if !WARNING_WAVRES_PLAY
bool wav_res_stop(void){return false;}
void wav_res_play_kick(u32 addr, u32 len){}
void wav_res_dec_process(void){}
bool wav_res_is_play(void){return false;}
AT(.rodata.wavres.buf)
const int wavres_cb = 0;
#endif

#if !AUX_SNR_EN
u8 aux_dnr_get_sta(void) {return 0;}
void aux_dnr_set_sta(u8 sta) {}
#endif

#if !BT_TWS_FAST_PAIR_EN
void ble_tiny_clock_isr(uint32_t clock) {}
void ble_tiny_isr(void) {}
AT(.com_text.tiny)
bool ble_tiny_adv_is_coming(void){return false;}
#endif

#if (!(MUSIC_WMA_SUPPORT | MUSIC_APE_SUPPORT | MUSIC_FLAC_SUPPORT | MUSIC_WAV_SUPPORT))
void msc_stream_start(u8 *ptr) {}
void msc_stream_end(void) {}
int msc_stream_read(void *buf, unsigned int size) {return 0;}
bool msc_stream_seek(unsigned int ofs, int whence) {return false;}
void msc_stream_fill(void) {}
void os_stream_fill(void) {}
void os_stream_read(void) {}
void os_stream_seek(void) {}
void os_stream_end(void) {}
#endif

#if !BT_AEC_EN
void AecBlockProcess(short *farend, short *nearend, short *output){}

#if !BT_ALC_EN
void ans_process(short *data){}
void nr_process(int *f){}
#endif

#else

//AT(.text.aec.core)
//bool voice_amplification(void)     //发送到远端的数据增大
//{
//    return true;
//}

//AT(.bt_voice.aec)
//bool config_dump_en(void)       //需要dump数据时打开此函数
//{
//    return true;
//}
#endif

#if !BT_ALC_EN
AT(.com_text.bb.cvsd)
void bt_alc_process(u8 *ptr, u32 samples, int ch_mode){}
#endif

#if !BT_SCO_DUMP_EN
void bt_sco_dump_init(void) {}
AT(.com_text.sco_dump)
void bt_sco_huart_tx_done(void){}
AT(.bt_voice.sco_dump)
bool sco_huart_putcs(u8 type, u8 frame_num, void *buf, uint len){return false;}
AT(.bt_voice.sco_dump)
void bt_sco_2ch_dump(s16 *in, s16 *out, u32 len){}
AT(.bt_voice.sco_dump)
void bt_sco_3ch_dump(s16 *near, s16 *far, s16 *out, u32 len){}
#endif

#if !BT_HFP_MSBC_EN
AT(.sbcdec.code)
bool btmsbc_fill_callback(u8 *eptr_in, u8 *eptr_out, u8 *dptr_in, u8 *dptr_out)
{
    return false;
}
void msbc_kick_start(void){}
void bt_msbc_process(void){}
void bt_msbc_pcm_output(u16 *output){}
void msbc_init(void){}
void msbc_exit(void){}
bool msbc_encode_init(void){return false;}
void msbc_encode_exit(void){}
u16 bt_msbc_encode_frame(u8 *buf, u16 len){return 0;}

#endif

#if BT_SRCRAM_PHASECOM_SYNC
AT(.com_text.pll_src)
u8 bt_srcram_phasecom_sync(void) {
    return 1;
}
#endif

#if !EXLIB_DRC_EN && !EXLIB_SOFT_EQ_EN && !KARAOK_REC_EN && !KARAOK_RM_VOICE && !EXLIB_BT_MONO_XDRC_EN && !EXLIB_SOFT_EQ_DRC_EN
AT(.sbcdec.code)
void bt_sbc_pcm_output(void){}
AT(.com_text.dac_pcm_ext)
void dac_pcm_ext_buf_process(void *buf,  u32 samples, int dual_ch){};
#endif

#if !KARAOK_RM_VOICE
AT(.com_text.karaok.rec)
bool karaok_voice_rm_is_enable(void){return false;}
AT(.com_text.karaok.rec)
void dac_rm_voice_proc(s16 left, s16 right){}
#endif

#if (UART0_PRINTF_SEL == PRINTF_NONE)
AT(.com_text.err)
void sys_error_hook(u8 err_no)
{
    WDT_RST();
    while(1);
}

AT(.com_text.exception) WEAK
void exception_isr(void)
{
    sys_error_hook(2);
}
#endif

#if BT_ID3_TAG_EN
bool bt_id3_en(void)
{
    return BT_ID3_TAG_EN;
}
#endif

#if !KARAOK_MIC_DRC
AT(.com_text.karaok_drc)
s16 drc_calc(s32 sample, u8 drc_cb_i){return 0;}
AT(.com_text.karaok_drc)
int drc_calc_dmic(s32 sample_l, s32 sample_r, u8 drc_cb_i){return 0;}
AT(.com_text.karaok_drc)
bool drc_init(const void *bin, int bin_size, u8 drc_cb_i){return 0;}
AT(.com_text.karaok_drc)
bool sdadc_drc_init(u8 *drc_addr, int drc_len){return 0;}
AT(.com_text.karaok_drc)
void drc_mic_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params){}
AT(.com_text.karaok_gain)
int karaok_mic_gain_tbl[32] ={};
#endif

#if FUNC_SPDIF_TX_EN
bool spdif_tx_en_flag = true;
#endif // FUNC_SPDIF_TX_EN


#if !USB_CUSTOM_DETECT
AT(.com_text.usb.dectect.sw)
void usbchk_switch_otg_custom(void){}
#endif

#if !SYS_KARAOK_EN
AT(.com_text.rmproc) WEAK
s32 iir_lp_filter(s32 input,void *coef){ return input;}
#endif

#if !FUNC_BT_EN
u8 bt_hfp_call_karaok_en(void)
{
    return 0;
}
#endif

#if !SYS_PITCH_SHIFT && !EXSPI_REC_PITCH_SHIFT_PLAY
int pitch_shift2_process(s16 *data) {return 0;}
#endif

#if UART_S_UPDATE
WEAK u8 get_uart_upd (u8 *ch){
    return 0;
}
AT(.com_text.uart_isr)
WEAK void put_upd_uart_msg(void)
{
}
WEAK void uart_upd_isr_disable(void)
{

}
#endif // UART_S_UPDATE

#if BT_SUPPORT_EIGHT_DEVICE_EN
u8 get_max_nr_link_key_num(void)
{
    return 8;
}
#endif

#if !USB_DETECT_ONLY_CHECK_HOST
AT(.text.usb.host_det)
void usb_vout_init(void){}
AT(.com_text.usb.host_det)
u8 usbchk_host_connect(void) {return 0;}
AT(.com_text.usb.host_det)
bool usb_is_still_online(void) {return false;}
#endif


