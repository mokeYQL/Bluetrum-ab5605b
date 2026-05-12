#include "include.h"
#include "func.h"
#include "func_huart.h"
#include "sfr.h"

#if FUNC_HUART_EN

#define DAC_OBUF0_SIZE      576

//u32 dac_src0_obuf[2*DAC_OBUF0_SIZE];

AT(.text.func.huart)
static void func_huart_process(void)
{
    func_process();
}

AT(.text.dac)
void src0_aubuf_init(void)
{
    u32 aubuf_size = DAC_OBUF0_SIZE;
    u32 threshold = DAC_OBUF0_SIZE - 64;

    AUBUFCON |= BIT(0);                                //Reset audio Buffer
//    AUBUFCON &= ~(BIT(6) | BIT(7));                    //Audio start play sample size (64 samples)
    AUBUFSIZE = (aubuf_size - 1);                      //Configure audio buffer size
    AUBUFSIZE |= (threshold) << 16;                    //SRC1 audio Buffer Threshold
//    AUBUFSTARTADDR = DMA_ADR(dac_src0_obuf);           //Configure audio buffer start address
//    DACDIGCON0 |= BIT(16);                             //SRC1 ENABLE
}

AT(.text.func.huart)
static void func_huart_enter(void)
{
    func_huart_display();
    src0_aubuf_init();
    huart_audio_init();

    bsp_huart_start();
}

AT(.text.func.huart)
static void func_huart_exit(void)
{
    func_huart_exit_display();
    bsp_huart_exit();
    func_cb.last = FUNC_HUART;
}

AT(.text.func.huart)
void func_huart(void)
{
    printf("%s\n", __func__);

    func_huart_enter();

    while (func_cb.sta == FUNC_HUART) {
        func_huart_process();
        func_huart_message(msg_dequeue());
        func_huart_display();
    }

    func_huart_exit();
}

#endif
