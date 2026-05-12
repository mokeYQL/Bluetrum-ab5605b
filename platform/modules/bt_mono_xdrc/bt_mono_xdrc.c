#include "include.h"

#if EXLIB_BT_MONO_XDRC_EN
void xdrc_online_adj_init(void);
extern u32 dac_obuf[576 + 512];
void obuf_put_samples_w(void *buf, uint len);

AT(.com_text.xdrc)
void pcm_dual_left2mon(u8 *buf, u32 samples, u32 is24bit)
{
   if(is24bit){
        s32 *ptr = (s32*)buf;
        for (int i = 0; i < samples; i++) {
            ptr[i] = ptr[2*i];
        }
   } else {
        s16 *ptr = (s16*)buf;
        for (int i = 0; i < samples; i++) {
            ptr[i] = ptr[2*i];
        }
   }
}

AT(.com_text.xdrc)
void pcm_dual_right2mon(u8 *buf, u32 samples, u32 is24bit)
{
   if(is24bit){
        s32 *ptr = (s32*)buf;
        for (int i = 0; i < samples; i++) {
            ptr[i] = ptr[2*i + 1];
        }
   } else {
        s16 *ptr = (s16*)buf;
        for (int i = 0; i < samples; i++) {
            ptr[i] = ptr[2*i + 1];
        }
   }
}

/*
主左声道,副右声道
(16:51:26.361) phone_c = 1, tws_c = 1, master = 0
=>AU0LMIXCOEF = 0x7FFF0000, AU0RMIXCOEF = 0x7FFF0000, DACDIGCON0 = 0x180  //only_right

(16:51:27.559) phone_c = 1, tws_c = 1, master = 1              //only_left
=>AU0LMIXCOEF = 0x7FFF, AU0RMIXCOEF = 0x7FFF, DACDIGCON0 = 0x180

副机断电时  //default 配置是差分单声道
(17:08:35.535) phone_c = 0, tws_c = 0, master = 1
=>AU0LMIXCOEF = 0x3FFF3FFF, AU0RMIXCOEF = 0x3FFF3FFF, DACDIGCON0 = 0x180
*/
//AT(.com_text.strdbug)
//const char str_ch_sta[] = "ch_sta = %d\n";

AT(.com_text.xdrc)
void bt_xdrc_dual2mon(u8 *buf, u32 samples, u32 is_24bit)
{
#if BT_TWS_EN
    u8 ch_sta = 0;
    if( (0x3FFF3FFF == AU0LMIXCOEF) && (0x3FFF3FFF == AU0RMIXCOEF)){
        pcm_dual_2_mon(buf,samples,is_24bit);
        //ch_sta = 0;
    }else if((0x7FFF == AU0LMIXCOEF) && ((0x7FFF == AU0RMIXCOEF) || (0x00008000 == AU0RMIXCOEF)) ) {  //only left
        //ch_sta = 1;
        pcm_dual_left2mon(buf,samples,is_24bit);
    }else if((0x7FFF0000 == AU0LMIXCOEF) && ((0x7FFF0000 == AU0RMIXCOEF) || (0x80000000 == AU0RMIXCOEF)) ){ //only_right
        //ch_sta = 2;
        pcm_dual_right2mon(buf,samples,is_24bit);
    } else
#endif
    {
        //ch_sta = 0;
        pcm_dual_2_mon(buf,samples,is_24bit);
    }
//    static u32 ticks = 0;
//    if (tick_check_expire(ticks,1000)) {
//        ticks = tick_get();
//        printf(str_ch_sta,ch_sta);
//    }
}


#define XDRC_PRINT_PCM   0

uint8_t cfg_dac_pcm_ext_en = 1;  //数据流引出来

AT(.com_text.dac_pcm_debug_str)
const char str_dac_pcm[] = "##samples = %d\n";
AT(.com_text.dac_pcm_ext)
void dac_pcm_ext_buf_process(void *buf,  u32 samples, int dual_ch)
{
    u32 is_24bit = 0;
#if XDRC_PRINT_PCM
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        printf(str_dac_pcm,samples);
        print_r(buf,16);
    }
#endif
    if(dual_ch) {
        bt_xdrc_dual2mon(buf,samples, is_24bit);
        xdrc_mono_process(buf,samples,is_24bit);
        pcm_mon_2_dual(buf,samples, is_24bit);
    } else {
        xdrc_mono_process((u8*)buf, samples,is_24bit);
    }
#if XDRC_PRINT_PCM
    if (tick_check_expire(ticks,1000)) {
        ticks = tick_get();
        print_r(buf,16);
    }
#endif
}

AT(.text.bsp.dac)
void xdrc_sysvol_set(s32 dig_vol)
{
    if (is_xdrc_sysvol_en()){   //蓝牙音频时调法链路的前置音量
        dac_set_dvol(0x7CA0);
        xdrc_softvol_set(dig_vol);
    } else {   //提示音或通话还是调以前的数字音量(DACVOLCON)
        dac_set_dvol(dig_vol);
    }
}


AT(.text.xdrc)
void bt_mono_xdrc_init(void)
{
    xdrc_online_adj_init();
    xdrc_audio_en_cfg(BIT_XDRC_ALL_EFFECT, true);
    xdrc_audio_en_cfg(BIT_XDRC_DYEQ1,false);      //disable DYEQ2
    xdrc_audio_en_cfg(BIT_XDRC_DYEQ2,false);      //disable DYEQ2

    xdrc_res_effect_init(RES_BUF_EFFECT_MON_XDRC_BIN, RES_LEN_EFFECT_MON_XDRC_BIN);
    //other init
    xdrc_huart_dump_init();
    sys_cb.xdrc_sysvol_en = 1;
    xdrc_sysvol_en_set(sys_cb.xdrc_sysvol_en);
    xdrc_softvol_init();

    if(EQ_DBG_IN_UART && xcfg_cb.eq_dgb_uart_en) {
        printf("######uart eg adj, SYS VOL SET TO MAX\n");
        sys_cb.vol = VOL_MAX;   //使能了串口在线调音时,把音量设置到最大
    }
    bsp_change_volume(sys_cb.vol);
}

#endif  //EXLIB_BT_MONO_XDRC_EN
