#include "include.h"
//软件PRE_EQ(建议不超过8段EQ) =>音量调节=> DRC(1段)=>硬件EQ(POST_EQ)
#if EXLIB_SOFT_EQ_DRC_EN
extern u32 dac_obuf[576 + 512];
extern void obuf_put_samples_w(void *buf, uint len);

AT(.sbcdec.code)
void bt_sbc_pcm_output(void)
{
    u32 *outbuf = &dac_obuf[576];
    if (!(AUCON0 & BIT(26))) {
        AUCON0 |= BIT(26);                      //SBC DMA to RAM
        AUDMAADR = DMA_ADR(outbuf);
    } else {
        //xdrc_dual_processs((u8*)outbuf,128, 0);
        obuf_put_samples_w(outbuf, 128);
    }
}


//max eq band 10
s32 sed_preeql_cb[10*7 + 6] AT(.buf.softeqdrc_ram);
s32 sed_preeqr_cb[10*7 + 6] AT(.buf.softeqdrc_ram);
u8  sed_drcl_cb[14 * 4] AT(.buf.softeqdrc_ram);
u8  sed_drcr_cb[14 * 4] AT(.buf.softeqdrc_ram);

T_SOFT_VOL sed_soft_vol_l AT(.buf.softeqdrc_ram);
T_SOFT_VOL sed_soft_vol_r AT(.buf.softeqdrc_ram);


AT(.text.softvol)
void softeq_drc_vol_set(s32 vol)
{
    if (cfg_dac_pcm_ext_en) {
        dac_set_dvol(0x7CA0);
        soft_vol_set(&sed_soft_vol_l, vol);
        soft_vol_set(&sed_soft_vol_r, vol);
    } else {
        dac_set_dvol(vol);
    }
}


AT(.text.eqdrc)
void sys_soft_vol_init(void)
{
    memset(&sed_soft_vol_l,0x00,sizeof(sed_soft_vol_l));
    memset(&sed_soft_vol_r,0x00,sizeof(sed_soft_vol_r));
    sed_soft_vol_l.step = 10;
    sed_soft_vol_r.step = 10;
    sed_soft_vol_l.fade_flag = 1;
    sed_soft_vol_r.fade_flag = 1;
}




AT(.text.xdrc)
void audio_softeq_drc_param_init(void)
{
    //PA6 FOR TEST
    GPIOAFEN &= ~BIT(6);   //PB1
    GPIOADE |= BIT(6);
    GPIOADIR &= ~BIT(6);

    softeq_drc_online_adj_init();
#if EXLIB_SOFT_EQ_EN
    //PRE EQ
    exlib_softeq_cb_init(sed_preeql_cb, sizeof(sed_preeql_cb),RES_BUF_SOFTEQ_DRC_PRE_EQ,RES_LEN_SOFTEQ_DRC_PRE_EQ);
    exlib_softeq_cb_init(sed_preeqr_cb, sizeof(sed_preeqr_cb),RES_BUF_SOFTEQ_DRC_PRE_EQ,RES_LEN_SOFTEQ_DRC_PRE_EQ);
#endif

#if EXLIB_DRC_PREVOL_EN
    sys_soft_vol_init();
#endif

#if EXLIB_DRC_EN
    exlib_drcv1_cb_init(sed_drcl_cb,sizeof(sed_drcl_cb),RES_BUF_SOFTEQ_DRC_DRC_DRC,RES_LEN_SOFTEQ_DRC_DRC_DRC);
    exlib_drcv1_cb_init(sed_drcr_cb,sizeof(sed_drcr_cb),RES_BUF_SOFTEQ_DRC_DRC_DRC,RES_LEN_SOFTEQ_DRC_DRC_DRC);
#endif
    //POST EQ
    music_set_eq_by_res(&RES_BUF_SOFTEQ_DRC_POST_EQ, &RES_LEN_SOFTEQ_DRC_POST_EQ);
}





//------------------------------------------------------------------------
//音频数据流处理
uint8_t cfg_dac_pcm_ext_en = 1;
//AT(.com_text.pcm_ext.print)
//const char strp1[] = "stereo:%d\n";
//AT(.com_text.pcm_ext.print)
//const char strp2[] = "mono:%d\n";

AT(.com_text.pcm_ext)
void dac_pcm_ext_stereo(s16 *ldata, s16 *rdata)
{
    s32 lpcm32 = *ldata;
    s32 rpcm32 = *rdata;

#if 1
    //软件EQ
    lpcm32 = exlib_softeq_proc(sed_preeql_cb, lpcm32, 0);
    rpcm32 = exlib_softeq_proc(sed_preeqr_cb, rpcm32, 0);
    //音量
    lpcm32 = soft_vol_proc(&sed_soft_vol_l,lpcm32);
    rpcm32 = soft_vol_proc(&sed_soft_vol_r,rpcm32);
    //DRC
    lpcm32 = exlib_drc_process_s16(sed_drcl_cb,lpcm32);
    rpcm32 = exlib_drc_process_s16(sed_drcr_cb,rpcm32);    //pcm32
#endif
    *ldata = (s16)lpcm32;
    *rdata = (s16)rpcm32;

    //debug info
//    static u32 ticks = 0;
//    static u32 samples_cnt = 0;
//    samples_cnt++;
//    if(tick_check_expire(ticks,1000)) {
//        printf(strp1,samples_cnt);
//        samples_cnt = 0;
//        ticks = tick_get();
//    }
}

AT(.com_text.pcm_ext)
void dac_pcm_ext_mono(s16 *data)
{
    s32 lpcm32 = *data;
    //软件EQ
    lpcm32 = exlib_softeq_proc(sed_preeql_cb, lpcm32, 0);
    //软件音量
    lpcm32 = soft_vol_proc(&sed_soft_vol_l,lpcm32);
    //DRC
    lpcm32 = exlib_drc_process_s16(sed_drcl_cb,lpcm32);
    *data = (s16)lpcm32;

    //debug info
//    static u32 ticks = 0;
//    static u32 samples_cnt = 0;
//    samples_cnt++;
//    if(tick_check_expire(ticks,1000)) {
//        printf(strp2,samples_cnt);
//        samples_cnt = 0;
//        ticks = tick_get();
//    }
}

AT(.com_text.dac_pcm_ext)
void dac_pcm_ext_buf_process(void *buf,  u32 samples, int dual_ch)
{
    //GPIOASET = BIT(6);
    s16 *pcm16 = (s16*)buf;
    if (dual_ch) { //双声道
        for (int i = 0; i < samples; i++) {
            dac_pcm_ext_stereo(pcm16, pcm16 + 1);
            pcm16 += 2;
        }
    } else { //单声道
        for (int i = 0; i < samples; i++) {
            dac_pcm_ext_mono(pcm16);
            pcm16++;
        }

    }
    //GPIOACLR = BIT(6);
}
#endif  //EXLIB_BT_MONO_XDRC_EN
