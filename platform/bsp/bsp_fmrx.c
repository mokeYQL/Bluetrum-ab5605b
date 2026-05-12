#include "include.h"

#if FUNC_FMRX_EN
AT(.com_text.fmrx.isr)
void fmrx_tmr1ms_isr(void)
{
    if (func_cb.sta == FUNC_FMRX) {
        fmrx_dac_sync();
    }
}

AT(.text.bsp.fmrx)
bool fmrx_is_playing(void)
{
    if (fmrx_cb.sta == FMRX_PLAY) {
        return true;
    }
    return false;
}

AT(.text.bsp.fmrx)
u8 get_fmrx_seek_staus(void)
{
    return fmrx_cb.seek_start;
}

AT(.text.bsp.fmrx)
void set_fmrx_seek_staus(u8 val)
{
    fmrx_cb.seek_start = val;
}

#if FMRX_TEST_CHANNEL  //FM 固定某些电台测试,可用于对比其它样机,定位到特定的一些台对比声音清晰度
static const u16 fm_test_channel_tbl[] = {8750,8780, 8880,8980, 9050,9070, 9150,9180,9200,9280,9390,9510,9640,9670,9710,9800,9910,9950,10000,10070,10160,10300,10380,10400,10430,10490,10540,10710};
static u8 fm_test_ch = 0;
u16 fmrx_test_first_channel_get(void)
{

    fm_test_ch = 0;
    printf("fmrx test first ch = %d, total ch = %d\n",fm_test_channel_tbl[0],sizeof(fm_test_channel_tbl)/2);
    return fm_test_channel_tbl[0];
}

void fmrx_test_channel_switch(u8 dir)
{
    int total_ch = sizeof(fm_test_channel_tbl)/2;
    if (dir) {
        fm_test_ch++;
        if (fm_test_ch >= total_ch) {
            fm_test_ch = 0;
        }
    } else {
        if (fm_test_ch > 0) {
            fm_test_ch--;
        } else {
            fm_test_ch = total_ch-1;
        }
    }
    fmrx_cb.freq = fm_test_channel_tbl[fm_test_ch];
    printf("fm_test_ch = %d, freq = %d\n",fm_test_ch,fmrx_cb.freq);
    bsp_fmrx_set_freq(fmrx_cb.freq);
}
#endif

#if FMRX_OPTIMIZE_TRY       //FM 收台效果尝试优化,可以修改CLK控制等,需要实际样机去测试效果
static u32 gat0_bak,gat1_bak,clkcon2_bak,clkcon3_bak;
//CLKGAT0://9_sd0 //10_uart0 //11_hsut0 //13_saradc //14_usb  //16_audec //18_mbist //20_bt // 21_uart1 //22|23 sdadc // 24_tmr1  //25_tmr2 //26_rtcc  //30_aec //31 cvsd   //6|7|15|28|29 rsv
static const char gat0dis_tbl[] = {9,6,7,11,18,21,24,25,28,29,30,31};  //15,20关掉会死机  //14 有usb_detect时不能关
//CLKGAT1://2_src //3_irrx //4_iis //5_sbc //7_plc //8_tmr3 //9_tmr4 //10_tmr5 //11_uart2 //12_spi1 //23_aecram //24_rom2 //25_rom3 //26 freqdet //27_pbf //28_dbg //29_26m32k //30_m2m //31_ef  //0|1|6|13|14 rsv
static const char gat1dis_tbl[] = {26,0,1,3,4,5,6,7,8,9,10,11,12,13,14,26,27,28,30,31}; //3是红外时钟，如果要使用红外，需要去掉3

AT(.text.func.fmrx)
void fmrx_optimize_try_set(void)
{
    int i,gat;
    printf("FMRX OPTIMIZE TRY\n");
    //Backup CLKGAT/CLKCON
    gat0_bak = CLKGAT0;
    gat1_bak = CLKGAT1;
    clkcon2_bak = CLKCON2;
    clkcon3_bak = CLKCON3;

    //Close Some CLKGAT
    printf("ORG CLKGAT0 = 0x%X,CLKGAT1 = 0x%X\n", CLKGAT0,CLKGAT1);
    for(i = 0,gat = 0; i < sizeof(gat0dis_tbl); i++) {
        gat |= (u32)(BIT(gat0dis_tbl[i]));
    }
    CLKGAT0 &= ~gat;
    printf("gat0num = %d, ~gat = 0x%X, CLKGAT0 = 0x%X\n",sizeof(gat0dis_tbl),~gat, CLKGAT0);
    for(i = 0,gat = 0; i < sizeof(gat1dis_tbl); i++) {
        gat |= BIT((u32)gat1dis_tbl[i]);
    }
    CLKGAT1 &= ~gat;
    printf("gat1num = %d,~gat = 0x%X, CLKGAT1 = 0x%X\n",sizeof(gat1dis_tbl),~gat, CLKGAT1);


    //CLK DIV
    //printf("CLKCON2/3 DIV org = 0x%X,0x%X\n",CLKCON2,CLKCON3);
    u32 clkdiv_test =    0x01;     //(1~15)   //default all is 0
    CLKCON2 =  (CLKCON2 & (~(0xF<<13))) | (clkdiv_test << 13);
    CLKCON3 =  (CLKCON3 & (~(0xF<<12))) | (clkdiv_test << 12);
    CLKCON3 =  (CLKCON3 & (~(0xF<<0))) | (clkdiv_test << 0);
    CLKCON3 =  (CLKCON3 & (~(0xF<<4))) | (clkdiv_test << 4);
    CLKCON3 =  (CLKCON3 & (~(0xF<<19))) | (clkdiv_test <<19);
    CLKCON3 =  (CLKCON3 & (~(0xF<<27))) | (clkdiv_test <<27);
    CLKCON3 =  (CLKCON3 & (~(0xF<<23))) | (clkdiv_test <<23);
    //printf("CLKCON2/3 DIV set = 0x%X,0x%X\n",CLKCON2,CLKCON3);
}

AT(.text.func.fmrx)
void fmrx_optimize_try_recover(void)
{
    CLKGAT0 = gat0_bak;
    CLKGAT1 = gat1_bak;
    CLKCON2 = clkcon2_bak;
    CLKCON3 = clkcon3_bak;
}

AT(.text.fmrx)
u32 fmrx_seek_config(void)   //BIT(0),判台时多放些台出来
{
    return BIT(0);      //FM灵敏度不高时,置位BIT(1)、BIT(2)多放些台进来
}
#endif

//AT(.text.fmrx)
//u32 fmrx_zerocrosscon_cfg(void)     //改快速过零点和过零点采样点数
//{
//    return ((400 << 16) | 8192*2);
//}

AT(.text.fmrx)
bool fmrx_auto_adjust_aupll_en(void)
{
#if FMRX_REC_EN
    return false;
#else
    return true;
#endif
}

AT(.text.fmrx)
bool fmrx_auto_adjust_sysclk_en(void)
{
    //fmrx_set_freq 内部时钟会在 fmrx_sysclk_config时钟和 其它时钟之间切换,需要固定时钟(如KARAOK需要120M及以上),则需要返回false.
    if (fmrx_sysclk_config() > SYS_52M) {
        return false;
    } else {
        return true;
    }
}

AT(.text.fmrx)
u8 fmrx_sysclk_config(void)
{
    //除KAROAK或软件EQ等需要大量计算外，其它情况下FM时钟一般设置为26M收台效果好一些
#if SYS_KARAOK_EN
    return SYS_CLK_SEL;
#else
    return SYS_26M;
#endif
}

AT(.text.fmrx)
u8 fmrx_auto_narrow_filter(void)
{
    //返回true(默认): 25K和75K自动切换 //返回false:固定75K
    //过指标测试时,如果遇到发射机设置为50K导致不停台，此时需要返回false(内部会自动设置为75K,修正该问题)
    return true;
}


AT(.text.bsp.fmrx)
void fmrx_threshold_init(void)
{
    fmrx_thd.r_val = FMRX_THRESHOLD_VAL;
    fmrx_thd.z_val = FMRX_THRESHOLD_Z;
    fmrx_thd.fz_val = FMRX_THRESHOLD_FZ;
    fmrx_thd.d_val = FMRX_THRESHOLD_D;
}

AT(.text.bsp.fmrx)
void bsp_fmrx_init(void)
{
    printf("%s\n", __func__);

    fmrx_power_on(FMRX_THRESHOLD_VAL);
#if FMRX_REC_EN
    rec_src.spr = SPR_44100;
    if (DACDIGCON0 & BIT(1)) {              //当前DAC输出采样率是否为48KHz
        rec_src.spr = SPR_48000;
    }
#if KARAOK_REC_EN
    rec_src.nchannel = 0x11;
#else
    rec_src.nchannel = 0x02;
#endif
    rec_src.source_start = fmrx_rec_start;
    rec_src.source_stop  = fmrx_rec_stop;
#endif
}


AT(.text.bsp.fmrx)
void bsp_fmrx_exit(void)
{
    fmrx_power_off();
}


AT(.text.bsp.fmrx)
void bsp_fmrx_set_freq(u16 freq)
{
    fmrx_set_freq(freq);
    fmrx_set_audio_channel_dual(FMRX_AUDIO_CHANNEL);  //FFM输出声道选则,一般单声道比双声道噪音会小些  //0 Mono  //1 Dual
    if (xcfg_cb.fmrx_cs_filter_fixed) {
        //若指标测试时发射机调制深度加大,FM收到后有失真,可以继续加大cs_filter值即可减少失真, 但也要留意,cs_filter值越大,理论上引入的噪声越多。
        fmrx_set_cs_filter(xcfg_cb.fmrx_cs_filter_sel);
    }
}


AT(.text.bsp.fmrx)
u8 bsp_fmrx_check_freq(u16 freq)
{
    return fmrx_check_freq(freq);
}

AT(.text.bsp.fmrx)
void bsp_fmrx_logger_out(void)
{
    printf("Seek out radio stations(%d):\n", fmrx_cb.ch_cnt);
    for (u8 j = 1; j <= fmrx_cb.ch_cnt; j++) {
        printf("%d\n", get_ch_freq(j));
    }
    fmrx_logger_out();
}

#endif // FUNC_FMRX_EN

