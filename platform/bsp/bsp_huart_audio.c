#include "include.h"
#include "bsp_huart_audio.h"

#if HUART_AUDIO_EN

#define PLL_CLK                             26000000
#define HUART_DMA_BUF_LEN                   256
#define SRC_HIGH_THR                        240
#define SRC_LOW_THR                         140

//ALIGNED(4)
//static u8 sinetbl[128] = {   //32 samples  //sine_16k_500hz_dual正弦波测试数据
//	0x00, 0x00, 0xFF, 0xFF, 0xF9, 0x18, 0xF9, 0x18, 0xFB, 0x30, 0xFB, 0x30,
//	0x1F, 0x47, 0x1E, 0x47, 0x7F, 0x5A, 0x81, 0x5A, 0x72, 0x6A, 0x6F, 0x6A,
//	0x3D, 0x76, 0x42, 0x76, 0x8F, 0x7D, 0x88, 0x7D,
//
//    0xFC, 0x7F, 0xFF, 0x7F, 0x8D, 0x7D, 0x88, 0x7D, 0x40, 0x76, 0x44, 0x76,
//    0x6F, 0x6A, 0x6D, 0x6A, 0x82, 0x5A, 0x81, 0x5A, 0x1D, 0x47, 0x1E, 0x47,
//    0xFB, 0x30, 0xFB, 0x30, 0xFA, 0x18, 0xFA, 0x18,
//
//    0xFE, 0xFF, 0xFF, 0xFF, 0x0A, 0xE7, 0x07, 0xE7, 0x03, 0xCF, 0x04, 0xCF,
//    0xE2, 0xB8, 0xE4, 0xB8, 0x7F, 0xA5, 0x7B, 0xA5, 0x92, 0x95, 0x95, 0x95,
//    0xBD, 0x89, 0xBD, 0x89, 0x79, 0x82, 0x76, 0x82,
//
//	0x01, 0x80, 0x00, 0x80, 0x79, 0x82, 0x78, 0x82, 0xBC, 0x89, 0xBA, 0x89,
//	0x95, 0x95, 0x99, 0x95, 0x7A, 0xA5, 0x76, 0xA5, 0xE7, 0xB8, 0xE9, 0xB8,
//	0x01, 0xCF, 0x01, 0xCF, 0x09, 0xE7, 0x0A, 0xE7
//};

typedef struct {
    psfr_t sfr;
    u32 bit;
} huart_map_t;

AT(.com_huart.rodata)
static const huart_map_t huart_map[] = {
    [0] = {(psfr_t)(&GPIOASET),     BIT(7)},    //PA7 - HSTRX-G1
    [1] = {(psfr_t)(&GPIOBSET),     BIT(2)},    //PB2 - HSTRX-G2
    [2] = {(psfr_t)(&GPIOBSET),     BIT(3)},    //PB3 - HSTRX-G3
    [3] = {(psfr_t)(&GPIOESET),     BIT(7)},    //PE7 - HSTRX-G4
    [4] = { NULL,                   NULL  },    //NULL
    [5] = {(psfr_t)(&GPIOASET),     BIT(6)},    //PA6 - HSTRX-G6
    [6] = {(psfr_t)(&GPIOBSET),     BIT(1)},    //PB1 - HSTRX-G7
    [7] = {(psfr_t)(&GPIOBSET),     BIT(4)},    //PB4 - HSTRX-G8
    [8] = {(psfr_t)(&GPIOESET),     BIT(6)},    //PE6 - HSTRX-G9
    [9] = { NULL,                   NULL  },    //NULL
    [10] ={ NULL,                   BIT(0)},    //vusb - HSTRX-G11
};

static huart_t huart0;
static volatile bool huar0_tx_busy;
static bool huart_paly_sta;

void huart_irq_init(void);
void huart_rx_process(void *buf, u32 samples, bool is_32bit);

AT(.com_huart.text)
WEAK void huart_rx_done_cb(void){};
AT(.com_huart.text)
WEAK void huart_tx_done_cb(void){};

AT(.com_rodata.huart_buffer);
u8 huart_buffer_rx[HUART_DMA_BUF_LEN];

AT(.com_huart.text) WEAK
void huart_mux_set_rx(void)
{
    if(huart0.tx_port == huart0.rx_port) {
        const huart_map_t *hmap = &huart_map[huart0.tx_port];

        HSUT0CON |= BIT(0);         //RX_EN enable

        hmap->sfr[GPIOxDIR] |= hmap->bit;
    }
}

AT(.com_huart.text) WEAK
void huart_mux_set_tx(void)
{
    if(huart0.tx_port == huart0.rx_port) {
        const huart_map_t *hmap = &huart_map[huart0.tx_port];

        HSUT0CON &= ~BIT(0);        //RX_EN DIS

        hmap->sfr[GPIOxDIR] &= ~hmap->bit;

    }
}

AT(.com_huart.text)
void bsp_huart_rx_kick(void)
{
    HSUT0CPND = BIT(0);                             //Clear RX to idle
    HSUT0CPND = BIT(12);                            //Clear RX Pending
    HSUT0CPND = BIT(15);
    HSUT0RXADR = DMA_ADR(huart0.rxbuf);
    HSUT0RXCNT = huart0.rxbuf_size;

    if (huart0.callback){
        huart0.callback((void*)huart_buffer_rx,HUART_SAMPLES,NULL);
    }
}

AT(.com_huart.text)
void bsp_huart_isr(void)
{

    if ((HSUT0CON & BIT(2)) && (HSUT0CON & (BIT(12) | BIT(11)))) {         //RX Pending or TimeOUT
        huart_rx_done_cb();
        bsp_huart_rx_kick();

    }
    if ((HSUT0CON & BIT(3)) && (HSUT0CON & BIT(13))) {  //TX Done
        HSUT0CPND = BIT(13);
        huar0_tx_busy = false;
//        huart_mux_set_rx();
//        bsp_huart_rx_kick();
//        huart_tx_done_cb();
    }
}

AT(.com_huart.text)
void huart_wait_txdone(void)
{
    if(huart0.txisr_en == 0) {
        if(huart0.tx_1st) {
            huart0.tx_1st = 0;
        } else {
            while (!(HSUT0CON & BIT(13)));
        }
    }
//    else {
//        while(huar0_tx_busy);
//    }
}

///HUART初始化
AT(.text.huart.init)
void bsp_huart_init(huart_t *huart, uint32_t baud_rate)
{
//    if(huart->tx_port > HUART_TR_VUSB || huart->rx_port > HUART_TR_VUSB) {
//        return;
//    }
    memcpy(&huart0, huart, sizeof(huart_t));

    CLKGAT0 |= BIT(11);                             //HUART CLK enable
    CLKCON0 &= ~(BIT(17)|BIT(18));                  //HUART 26M CLK

//    if(!HUART0_IS_VUSB()) {
        const huart_map_t *hmap;

        hmap = &huart_map[huart0.tx_port];                 //TX
        hmap->sfr[GPIOxDE] |= hmap->bit;
        hmap->sfr[GPIOxSET] |= hmap->bit;
        hmap->sfr[GPIOxDIR] &= ~hmap->bit;
        hmap->sfr[GPIOxFEN] |= hmap->bit;

        hmap = &huart_map[huart0.rx_port];                 //RX
        hmap->sfr[GPIOxPU] |= hmap->bit;
        hmap->sfr[GPIOxDE] |= hmap->bit;
        hmap->sfr[GPIOxSET] |= hmap->bit;
        hmap->sfr[GPIOxDIR] |= hmap->bit;
        hmap->sfr[GPIOxFEN] |= hmap->bit;
//      }

    u32 baud_val = (((PLL_CLK + (baud_rate / 2)) / baud_rate) - 1);
    FUNCMCON0 = ((huart0.rx_port+1) << 20) | ((huart0.tx_port+1) << 16);
    HSUT0BAUD = (baud_val << 16) | baud_val;

    HSUT0CON = BIT(9);                  //2 bit stop bit
    HSUT0CON |= BIT(7) | BIT(4);        //TX/RX DMA mode
    HSUT0CON |= BIT(1) | BIT(0);        //TX EN, RX EN
    HSUT0TMRCNT = 32;                   //32个时钟没收完，则结束
    HSUT0CON |= BIT(10);                //Enable TMRCNT
    HSUT0CON |= BIT(17) | BIT(16);      //SYNC CONFIG
    HSUT0CPND = 0xFFFF;                 //clr RX_PEND
    huart0.tx_1st = 1;

    if(huart0.rxbuf != NULL) {
        HSUT0CON |= BIT(4);             //DMA Mode
        if(huart0.rxbuf_loop) {
            HSUT0CON |= BIT(6);         //Loop Mode
        }
        HSUT0RXADR = DMA_ADR(huart0.rxbuf);
        HSUT0RXCNT = huart0.rxbuf_size;
    }

    if(huart0.rxisr_en || huart0.txisr_en) {
        if(huart0.rxisr_en) {
            HSUT0CON |= BIT(2);             //RX IE enable
        }
        if(huart0.txisr_en) {
            HSUT0CPND = BIT(13);            //clr TX_PEND
            HSUT0CON |= BIT(3);             //TX IE enable
        }
        bsp_huart_irq_init();
    }
    huart0.callback = huart_rx_process;

    bsp_huart_rx_kick();
}

AT(.com_text.dac_adj) WEAK
void dac_adjust_process(void)
{
//    u16 au_size = AUBUFSIZE << 2;                  //1/4 AUBUFSIZE
    if (!(DACDIGCON0 & BIT(6))) {
       DACDIGCON0 |= BIT(6);
    }
    if (AUBUFFIFOCNT <= SRC_LOW_THR) {
        PHASECOMP = ((PHASECOMP & ~(0xFFFF <<0)) |(0xFFFF <<0));                //低16位有符号数调节SRC0 FIFO SPEED
    } else if (AUBUFFIFOCNT >= SRC_HIGH_THR) {
        PHASECOMP = ((PHASECOMP & ~(0xFFFF <<0)) |(0x0001 <<0));
    } else {
        PHASECOMP = ((PHASECOMP & ~(0xFFFF <<0)) |(0x0000 <<0));
    }
}

AT(.com_text.huart_text)
void huart_rx_process(void *buf, u32 samples, bool is_32bit)
{
    dac_adjust_process();
    huart_audio_out_audio_input(buf,samples,NULL,NULL);
}

AT(.com_text.haurt_audio_out)
void huart_audio_out_audio_input(u8 *ptr, u32 samples, int ch_mode, void *params)
{
    s32 sample;
    s16 *lptr = (s16 *)ptr;

    if ((AUBUFCON & BIT(8)) == 0){
        for(int i = 0; i < samples / 2; i++) {
            sample = lptr[i];
            sample &= 0xffff;
            sample = (sample << 16) | sample;
            AUBUFDATA = sample;
        }
    }
}

///设置HUART波特率
AT(.text.huart.huart_set_baudrate)
void huart_set_baudrate(uint baud_rate)
{
    u32 baud_val = (((PLL_CLK + (baud_rate / 2)) / baud_rate) - 1);
    while (!(HSUT0CON & BIT(13)));
    HSUT0BAUD = (baud_val << 16) | baud_val;
    HSUT0CON |= BIT(17) | BIT(16);      //SYNC CONFIG
}

///HUART 发送1Byte数据
AT(.com_huart.text)
void bsp_huart_putchar(const char ch)
{
    HSUT0CON &= ~BIT(7);
    if(huart0.tx_1st) {
        huart0.tx_1st = 0;
    } else {
        while (!(HSUT0CON & BIT(13)));
    }
    huar0_tx_busy = true;
    HSUT0DATA = ch;
}

///HUART 发送数据
AT(.com_huart.text)
void bsp_huart_tx(const void *buf, uint len)
{
    huart_wait_txdone();
    huart_mux_set_tx();
//    HSUT0CON &= ~BIT(2);                            //RX IE Dis
//    HSUT0CON |= BIT(3);                             //TX IE En

    if ((u32)buf >= 0x10000000) {
        const char *txbuf = buf;
        while (len--) {
            bsp_huart_putchar(*txbuf++);
        }
    } else {
        huar0_tx_busy = true;
        HSUT0CON |= BIT(7);                 //DMA Mode
        HSUT0TXADR = DMA_ADR(buf);
        HSUT0TXCNT = len;
    }

    if(!huart0.txisr_en && huart0.tx_port == huart0.rx_port) {
        huart_wait_txdone();
        huart_mux_set_rx();
    }
}

///HUART 获取RXFIFO中的数据个数
AT(.com_text.huart.huart_getchar)
uint huart_get_rxcnt(void)
{
    return HSUT0FIFOCNT;
}

///HUART RXFIFO清空
AT(.com_text.huart.huart_getchar)
void huart_rxfifo_clear(void)
{
    HSUT0CPND = BIT(11);
}

///HUART 获取1Byte数据
AT(.com_text.huart.huart_getchar)
char huart_getchar(void)
{
    HSUT0FIFO = BIT(8);
    while ((HSUT0FIFO & BIT(9)) == 0);
    return (HSUT0FIFO & 0xff);
}

AT(.text.huart.huart_start)
void bsp_huart_start(void)
{
    dac_fade_wait();                    //等待淡出完成
    dac_fade_in();

    HSUT0CON |= BIT(2);                 //RX IE enable

    dac_spr_set(DAC_IN_SAMPLE);

    huart_paly_sta = true;
}

AT(.text.huart.huart_stop)
void bsp_huart_stop(void)
{
    dac_fade_out();

    HSUT0CON &= ~BIT(2);               //RX IE disenable

    huart_paly_sta = false;
}

AT(.text.huart.huart_exit)
void bsp_huart_exit(void)
{
    HSUT0CON = 0;
    HSUT0CON |= BIT(17) | BIT(16);                  //sync by UPTXCFG, UPRXCFG

    dac_fade_out();
}

bool huart_get_play_sta(void)
{
    return huart_paly_sta;
}

AT(.text.huart.huart_audio_init)
void huart_audio_init(void)
{
    huart_t huart0;

    memset(&huart0, 0x00, sizeof(huart0));
    huart0.rx_port = HUART_RX_PORT;
    huart0.tx_port = HUART_TX_PORT;
    huart0.rxbuf_loop = 1;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 0;
    huart0.rxbuf      = huart_buffer_rx;
    huart0.rxbuf_size = HUART_DMA_BUF_LEN;

    bsp_huart_init(&huart0, HUART_AUDIO_BAUD);        //波特率设置默认是1.5MHz
}

#endif
