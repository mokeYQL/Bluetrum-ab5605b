#include "include.h"
#include "func.h"
#include "func_aux.h"

#define AUX_ADC_DUMP_TEST        0

#if FUNC_AUX_EN
func_aux_t f_aux;

#if AUX_REC_EN || REC_WHITOUT_KARAOK
AT(.text.bsp.aux)
void aux_rec_start(void)
{
#if REC_WHITOUT_KARAOK
    if(func_cb.sta == FUNC_AUX){
        func_aux_stop();
#if SYS_KARAOK_EN
        bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif // SYS_KARAOK_EN
        f_aux.aux2adc |= AUX2PA_ADC_MASK;
        func_aux_start();
    }
#endif
    if (f_aux.aux2adc & AUX2PA_ADC_MASK) {
        aux_shorting_enable();       //L+R叠加录音
    }
    f_aux.rec_en = 1;
}

AT(.text.bsp.aux)
void aux_rec_stop(void)
{
    f_aux.rec_en = 0;
    if (f_aux.aux2adc & AUX2PA_ADC_MASK) {
        aux_shorting_disable();     //恢复模拟直通立体声输出
    }
#if REC_WHITOUT_KARAOK
    if(func_cb.sta == FUNC_AUX){
        func_aux_stop();
        f_aux.aux2adc &= ~AUX2PA_ADC_MASK;
        func_aux_start();
#if SYS_KARAOK_EN
        bsp_karaok_init(AUDIO_PATH_KARAOK, FUNC_AUX);
#endif // SYS_KARAOK_EN
    }
#endif
}

AT(.text.bsp.aux)
void aux_rec_init(void)
{
    rec_src.spr = SPR_44100;
    rec_src.nchannel = 1;
    rec_src.source_start = aux_rec_start;
    rec_src.source_stop  = aux_rec_stop;
    f_aux.rec_en = 0;
}
#endif // AUX_REC_EN


//AT(.com_text)
//void pcm_manual_amplify(void *pcm_buf,u32 len, u16 gain)  //2^13  0DB
//{
//    s32 tem;
//    s16 *buf = (s16*)pcm_buf;
//    for(u32 i=0; i<len/2; i++){
//        tem = (buf[i]* gain)>>13;   //2^13 8192
//         if(tem < -32768){
//            tem = -32768;
//       }else if(tem > 32767){
//            tem = 32767;
//       }
//       buf[i] = tem;
//    }
//}


#if AUX_ADC_DUMP_TEST
static u8 aux_dump_head_buf[14*1];   //管理包头需要的buf,uartDump工具目前支持最大同时保存生成6个文件,这里AUX只用到了1路
static u8 aux_dump_dma_buf[14+256];  //DMA需要的BUF

void aux_adc_dump_init(void)
{
    printf("------> %s\n",__func__);
    set_sys_clk(SYS_120M);  //HUART跑1.5M时,时钟需要设置高一些。
    huart_module_init();    //初始化高速串口模块
    dump_buf_init(aux_dump_head_buf,6,huart_putbuf,huart_wait_tx_finish);  //初始化dump模块
}
#endif


#define AUX_SDADC_PRINT       0

#if AUX_SDADC_PRINT
AT(.com_text.func.aux_cst)
const char straux_info[] = "[%d]";
#endif

AT(.text.aux_sdadc)
void aux_sdadc_process(u8 *ptr, u32 samples, int ch_mode)
{
#if AUX_SDADC_PRINT
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        ticks = tick_get();
        printf(straux_info,samples);
		print_r(ptr,16);
    }
#endif

#if AUX_ADC_DUMP_TEST
    dump_putbuf(aux_dump_dma_buf,ptr,samples*2,0);
#endif

#if AUX_REC_EN || REC_WHITOUT_KARAOK
    if (f_aux.rec_en) {
        puts_rec_obuf(ptr, (u16)(samples << (1 + ch_mode)));
    }
#endif // AUX_REC_EN

#if AUX_SNR_EN
    aux_dnr_process(ptr, samples);
#endif // AUX_SNR_EN

    //pcm_manual_amplify(ptr,(samples*2)<<ch_mode,GAIN_DIG_P7DB);  //ch_mode 0 单声道, 1双声道
    if (f_aux.aux2adc & AUX2ADC_MASK) {
        sdadc_pcm_2_dac(ptr, samples, ch_mode);
    }
}


AT(.text.bsp.aux)
void func_aux_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }

#if AUX_REC_EN
    sfunc_record_pause();
#endif // AUX_REC_EN

    if (!f_aux.pause) {
        func_aux_stop();
        mp3_res_play(addr, len);
        func_aux_start();
    } else {
        mp3_res_play(addr, len);
    }

#if AUX_REC_EN
    sfunc_record_continue();
#endif // AUX_REC_EN
}

AT(.text.func.aux)
void func_aux_start(void)
{
    bsp_aux_start(get_aux_channel_cfg(), xcfg_cb.aux_anl_gain, f_aux.aux2adc);
#if AUX_CHANNEL_EXCHANGE
    aux_channel_exchange_start(get_aux_channel_cfg());   //AUX 直通左右声道互换示例
#endif // AUX_CHANNEL_EXCHANGE
}

AT(.text.func.aux)
void func_aux_stop(void)
{
    bsp_aux_stop(get_aux_channel_cfg(), f_aux.aux2adc);
}

AT(.text.func.aux)
void func_aux_pause_play(void)
{
    if (f_aux.pause) {
        led_aux_play();
        func_aux_start();
    } else {
        led_idle();
        func_aux_stop();
    }
    f_aux.pause ^= 1;
}

AT(.text.func.aux)
void func_aux_setvol_callback(u8 dir)
{
    if (f_aux.pause) {
        func_aux_pause_play();
    }

    if (sys_cb.vol == 0) {
        aux_channel_mute();
    } else if ((sys_cb.vol == 1) && (dir)) {
        aux_channel_unmute();
    }
}

AT(.text.func.aux)
void func_aux_process(void)
{
    func_process();
}

static void func_aux_enter(void)
{
    if (!is_linein_enter_enable()) {
        func_cb.sta = FUNC_NULL;
        return;
    }
#if AUX_ADC_DUMP_TEST
    aux_adc_dump_init();
#endif

    memset(&f_aux, 0, sizeof(func_aux_t));
    f_aux.aux2adc = ((u8)(AUX_2_SDADC_EN & xcfg_cb.aux_2_sdadc_en) << 7) | AUDIO_PATH_AUX;
#if ((!SYS_KARAOK_EN) && (AUX_REC_EN || AUX_SNR_EN))
    if (((f_aux.aux2adc & AUX2ADC_MASK) == 0) && (get_aux_channel_cfg() & CHANNEL_R) && (get_aux_channel_cfg() & CHANNEL_L)) {
        //AUX立体声模拟直通, 需要同时开启SDADC用于录音或能量检测
        f_aux.aux2adc |= AUX2PA_ADC_MASK;
    }
#endif // AUX_REC_EN
    func_cb.mp3_res_play = func_aux_mp3_res_play;
    func_cb.set_vol_callback = func_aux_setvol_callback;
    msg_queue_clear();
    lock_code_aux_dnr();

#if AUX_SNR_EN
    //3ms检测一次 //连续超过2次大于能量 580，连续超过100次能量低于300 就认为无声.aux能量值可以通过函数 aux_dnr_get_max_pow() 获取。
    //aux_dnr_get_max_pow() 的返回的值需要在 aux_dnr_process 有运行时，才有效.
    aux_dnr_init(2, 600, 200, 300);
#endif // AUX_SNR_EN

#if AUX_REC_EN || REC_WHITOUT_KARAOK
    aux_rec_init();
#endif // AUX_REC_EN

    led_aux_play();
    func_aux_enter_display();
#if WARNING_FUNC_AUX
    mp3_res_play(RES_BUF_AUX_MODE_MP3, RES_LEN_AUX_MODE_MP3);
#endif // WARNING_FUNC_AUX

#if SYS_KARAOK_EN
    dac_fade_out();
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif

    func_aux_start();

#if SYS_KARAOK_EN
    bsp_karaok_init(AUDIO_PATH_KARAOK, FUNC_AUX);
#endif

    led_aux_play();

#if MICAUX_ANALOG_OUT_ALWAYS
    micaux_analog_out_init();
#endif // MICAUX_ANALOG_OUT_ALWAYS
}

static void func_aux_exit(void)
{
#if AUX_REC_EN || REC_WHITOUT_KARAOK
    sfunc_record_stop();
#endif // AUX_REC_EN

    func_aux_exit_display();
    func_aux_stop();
    unlock_code_aux_dnr();
    func_cb.last = FUNC_AUX;
}

AT(.text.func.aux)
void func_aux(void)
{
    printf("%s\n", __func__);

    func_aux_enter();

    while (func_cb.sta == FUNC_AUX) {
        func_aux_process();
        func_aux_message(msg_dequeue());
        func_aux_display();
    }

    func_aux_exit();
}
#endif  //FUNC_AUX_EN
