#include "include.h"
#include "func_hdmi.h"
#include "func_spdif.h"
#include "bsp_hdmi.h"

#if FUNC_HDMI_EN
#define HDMI_CEC_PRINT_RXTX     1     //底层接收和发送的数据打印,调试时查看CEC双向通信是否正常
#if HDMI_CEC_PRINT_RXTX
AT(.com_text.hdmi_str)
const char str_time[] = "[%d_%d_%d_%d] ";
AT(.com_text.hdmi_str)
const char str_rx_byte[] = " <%02X>";
AT(.com_text.hdmi_str)
const char str_rx_cache[] = "\n\n<<-- rx[%02X_%d]: %02X_%02X ";
AT(.com_text.hdmi_str)
const char str_tx_byte[] = " [#%02X]";
AT(.com_text.hdmi_str)
const char str_tx_cache[] = "-->> tx[%02X_%d_%d]: %02X_%02X ";

AT(.com_text.hdmi_isr)
void print_rx_cache(CEC_FRAME_T *p_cache)   //isr rx frame callback
{
    printf(str_rx_cache,  p_cache->opcode,p_cache->arg_len,p_cache->src_dst, p_cache->opcode);
    print_r(p_cache->arg_buf,p_cache->arg_len);
};

AT(.com_text.hdmi_isr)
void print_rx_byte(u8 byte)  //isr rx byte callback
{
    printf(str_rx_byte,byte);
};

AT(.com_text.hdmi_isr)
void print_rx_timeinfo(u32 low_period, u32 total_period, u32 rx_bcnt, u32 time_cnt)  //isr judge bit type callback
{
    //printf(str_time,low_period,total_period,rx_bcnt,time_cnt);
};

AT(.com_text.hdmi_isr)
void print_tx_byte(u8 byte)  //isr rx byte callback
{
    printf(str_tx_byte,byte);
};

AT(.com_text.hdmi_isr)
void tx_cache_print(CEC_FRAME_T *p_cache, u32 kick_wait_time)
{
    printf(str_tx_cache, p_cache->opcode,p_cache->arg_len,kick_wait_time,p_cache->src_dst, p_cache->opcode);
    print_r(p_cache->arg_buf,p_cache->arg_len);
};
#endif

spd_cfg_t hdmi_spdif_cfg;
void cec_frame_process(void);

AT(.text.func.hdmi)
void func_hdmi_process(void)
{
    cec_frame_process();
    func_process();
    //spdif_smp_detect();
#if HDMI_DETECT_EN
    if (!dev_is_online(DEV_HDMI)) {
       func_cb.sta = FUNC_NULL;
    }
#endif
}

AT(.text.func.hdmi)
void func_hdmi_exit(void)
{

    dac_fade_out();
    spdif_exit();
    #if 0//部分三星电视在退出HDMI时需要发送下列命令才可以恢复电视机扬声器的正常播放
    CEC_FRAME_T f_send;
    f_send.opcode  = CECMSG_ARC_TERMINATE;
    f_send.src_dst = AUD2BC;
    f_send.arg_len = 0;
    cec_tx_frame(&f_send);

    f_send.opcode     = CECMSG_SET_SYSTEM_AUDIO_MODE;
    f_send.src_dst    = AUD2BC;
    f_send.arg_len    = 1;
    f_send.arg_buf[0] = 0;
    cec_tx_frame(&f_send);

    f_send.opcode     = CECMSG_GIVE_DEVICE_POWER_STATUS;
    f_send.src_dst    = AUD2TV;
    f_send.arg_len    = 0;
    cec_tx_frame(&f_send);
    #endif
    delay_5ms(10);

    bsp_hdmi_cec_exit();
    func_cb.last = FUNC_HDMI;
}

void func_hdmi_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }
    spdif_exit();
    mp3_res_play(addr, len);
    spdif_init(&hdmi_spdif_cfg);
    spdif_start();
}
void bsp_hdmi_cec_init(void);
AT(.text.func.hdmi)
void func_hdmi_enter(void)
{
    bsp_hdmi_cec_init();
    hdmi_get_physical_address();
    func_spdif_enter_display();
    func_cb.mp3_res_play = func_hdmi_mp3_res_play;
    memset(&hdmi_spdif_cfg, 0, sizeof(spd_cfg_t));
    hdmi_spdif_cfg.channel = bsp_get_hdmi_spdif_ch();
#if WARNING_FUNC_HDMI
    //spdif_exit();
    mp3_res_play(RES_BUF_EN_HDMI_MP3, RES_LEN_EN_HDMI_MP3);
    spdif_init(&hdmi_spdif_cfg);
#else
    spdif_init(&hdmi_spdif_cfg);
#endif
    spdif_start();
    dac_fade_in();
}

AT(.text.func.hdmi)
void func_hdmi(void)
{
    printf("%s\n", __func__);
    func_hdmi_enter();

    while (func_cb.sta == FUNC_HDMI) {
        func_hdmi_process();
        func_hdmi_message(msg_dequeue());
        func_hdmi_display();
    }
    func_hdmi_exit();
}

#endif
