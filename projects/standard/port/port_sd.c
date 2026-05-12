#include "include.h"

#if SD_USB_MUX_IO_EN
u8 read_single_block_en = 1;       //sd单块读取
u8 write_single_block_en = 1;      //sd单块写入
u8 sdclk_idle_state = 1;           //sd空闲时clk状态为1
#endif

#if MUSIC_SDCARD_EN
static gpio_t sddet_gpio;

//AT(.text.sdio)
//u32 get_sdarg_data(u8 arg_num)      //这里返回print_sdarg打印的两个值arg2，和arg3
//{
//    if (arg_num == 2) {
//        return 0x50505344;
//    } else if (arg_num == 3) {
//        return 0x41;
//    }
//    return 0;
//}
//AT(.text.sdio)
//void print_sdarg(u32 arg2, u32 arg3)
//{
//    my_printf("arg2:0x%X, arg3:0x%X\n", arg2, arg3);
//}
void sdcard_detect_init(void)
{
    gpio_t *g = &sddet_gpio;
    u8 io_num = xcfg_cb.sddet_iosel;
    bsp_gpio_cfg_init(g, io_num);

    if (!io_num) {
        return;
    } else if (io_num == IO_MUX_SDCLK) {
        SD_MUX_DETECT_INIT();
    } else if (io_num <= IO_PF3) {
        g->sfr[GPIOxDE] |= BIT(g->num);
        g->sfr[GPIOxPU] |= BIT(g->num);
        g->sfr[GPIOxDIR] |= BIT(g->num);
    }
}

AT(.com_text.sdio)
bool sdcard_is_online(void)
{
    gpio_t *g = &sddet_gpio;
    u8 io_num = xcfg_cb.sddet_iosel;

    if (!io_num) {
        return false;
    } else if (io_num == IO_MUX_SDCLK) {
        return SD_MUX_IS_ONLINE();
    } else if (io_num == IO_MUX_SDCMD) {
        return SD_CMD_MUX_IS_ONLINE();
    } else if (io_num == IO_MUX_PWRKEY) {
        return pwrkey_detect_flag;
    } else {
        return (!(g->sfr[GPIOx] & BIT(g->num)));
    }
}

AT(.com_text.sdio)
bool is_det_sdcard_busy(void)
{
    u8 io_num = xcfg_cb.sddet_iosel;

    //无SD检测
    if (!io_num) {
        return true;
    }

    if (io_num <= IO_MUX_SDCLK) {           //普通IO或复用SDCLK检测
        return SD_MUX_IS_BUSY();
    }else if(io_num == IO_MUX_SDCMD){       //复用SDCMD检测
        return SD_MUX_CMD_IS_BUSY();
    }
    return false;
}

//设置SD卡波特率,波特率最大为系统时钟的一半(sysclk/(SDBAUD+1)),且一般不建议超过14M,太高SD卡可能有兼容性问题.
//开发板上测试有的卡SDCLK为60M也能跑(系统时钟为120M时)
//u8 get_sd_rate(void)
//{
//    return 12;   //unit: M
//}

AT(.text.sdcard)
void sd_gpio_init(u8 type)
{
    if (type == 0) {
        SD_MUX_IO_INIT();
    #if SDCMD_MUX_DETECT_EN
        SD_CMD_MUX_PU300R();                    //正常通信，SDCMD上拉300R
    #endif
    } else if (type == 1) {
    #if USER_ADKEY_MUX_SDCLK || VBAT_DETECT_MUX_SDCLK
        adc_mux_sdclk_w4_convert();
    #endif
	#if SD_USB_MUX_IO_EN
		get_usb_chk_sta_convert();
	#endif
        SD_CLK_DIR_OUT();
    #if SDCMD_MUX_DETECT_EN
        SD_CMD_MUX_PU300R();                    //正常通信，SDCMD上拉300R
    #endif
        FUNCMCON0 = SD0_MAPPING;
    } else {
    #if ((USER_ADKEY_MUX_SDCLK  && (!ADKEY_PU10K_EN)) || VBAT_DETECT_MUX_SDCLK)
        if (xcfg_cb.user_adkey_mux_sdclk_en || VBAT_DETECT_MUX_SDCLK) {     //有外部上拉, 关掉内部上拉
            SD_CLK_IN_DIS_PU10K();
        } else
    #endif
        {
            if(xcfg_cb.sddet_iosel != IO_MUX_SDCMD){
                SD_CLK_DIR_IN();
            }
        }
    #if SDCMD_MUX_DETECT_EN
        SD_CMD_MUX_PU10K();                         //IDLE状态，SDCMD上拉10K检测外设
    #endif
    }
}

#else
AT(.text.sdcard)
void sd_gpio_init(u8 type)
{
}
AT(.com_text.sdio)
bool is_det_sdcard_busy(void){ return false;};
#endif          // MUSIC_SDCARD_EN
