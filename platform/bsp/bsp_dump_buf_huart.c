#include "include.h"

#if HUART_DUMP_BUF_EN //HUART_DUMP_BUF_EN
//----dump  api-------------------------------------------------------------
//head_buf:   dump模块需要的包头管理buf,长度为 file_total*14
//file_toal:  dump模块同时支持导出的文件个数. 目前PC端软件UartDump暂时只支持最多同时接收6路并保存.
//putbuf:     底层dump函数,这里一般为高速串口DMA发送函数
//wait:       dump等待函数,下一次dump_putbuf时,需要等待上一次putbuf结束
void dump_buf_init(u8 *head_buf, int file_toal,void (*putbuf)(void*buf,u32 len), void (*wait)(void));

//dump_dma_buf:长度需要大于或等于 sizeof(tx_buf) + 14, 其中14是包头的长度. tx_buf中的数据会先打包放到dump_dma_buf中待发送
//tx_buf:      导出的数据存放的buf
//tx_buf_len:  导出数据的长度
//file_idx: 文件序号,0表示UartDump收到数据后保存到文件0, 1表示保存到文件1...
void dump_putbuf(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
//为了减少dma等待时间,可以先通过dump_put2ram把数据封包放到ram中,再调用dump_dma_kick一次性导出数据
void dump_put2ram(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
void dump_dma_kick(void *dma_buf,u32 dma_len);  //kick一次dma发送
void dump_dma_wait(void);  //等待上一次发送完成
//-----user config---------------------------------------------------------
//DUMP_BUF 测试程序,使用高速串口DMA方式导出数据,需要导出数据时,建议系统时钟设置为120M及以上

#define HUART_BAUD_SEL_6M   1     //1 huart 6M波特率  //0 则表示1.5M波特率

#if HUART_BAUD_SEL_6M
#define HUART_BAUD          6000000
#define HUART_BAUD_VAL      (((52000000 + (HUART_BAUD / 2)) / HUART_BAUD) - 1)
#else
#define HUART_BAUD          1500000
#define HUART_BAUD_VAL      (((26000000 + (HUART_BAUD / 2)) / HUART_BAUD) - 1)
#endif

void huart_module_init(void)   //huart DMA输出测试  //测试发现huart暂时只支持26M时钟
{
    printf("huart_module_init\n");
    FUNCMCON0 = 0x0f << 20 | 0x0f << 16;
#if 0  //PA7
    FUNCMCON0 = 0x0f << 8 | 0x0f << 12;  //dis uart0 map

    FUNCMCON0 = 0x01 << 20 | 0x01 << 16;
    GPIOADE |= BIT(7);
    GPIOADIR &= ~BIT(7);
    GPIOAFEN |= BIT(7);
#else   //PB3     //使用PB3(USB_DP)导出时，注意关闭USB相关功能宏 FUNC_USBDEV_EN 和 MUSIC_UDISK_EN
    FUNCMCON0 = 0x03 << 20 | 0x03 << 16;
    GPIOBDE |= BIT(3);
    GPIOBDIR &= ~BIT(3);
    GPIOBFEN |= BIT(3);
#endif


#if HUART_BAUD_SEL_6M //52M时钟源
    CLKGAT0 |= BIT(11);
    CLKCON0 |= BIT(30);  //26 DOUBLE TO 52
    CLKCON0 |= BIT(17);   //1 52m, 0 26M
    CLKCON0 &= ~BIT(18);
#else //26M时钟源OK
    CLKGAT0 |= BIT(11);                             //HUART CLK enable
    CLKCON0 &= ~BIT(17);   //1 52m, 0 26M
    CLKCON0 &= ~BIT(18);
#endif
    HSUT0BAUD = (HUART_BAUD_VAL << 16) | HUART_BAUD_VAL;
    HSUT0CON = 0;
    HSUT0CON |= BIT(9);        //BIT9 = 1, 2 bit stop bit  //0 : 1BIT   //打开2bit stop 不容易传输出错
    HSUT0CON |= BIT(1);       //TX EN
    HSUT0CON |= BIT(7);       //TX DMA MODE
    HSUT0CON |= BIT(17) | BIT(16);   //cfg sync
    HSUT0CPND = 0xffff;

    static u8 buf_first_kick[2] = {0x55,0xAA};  //kick for first Pending of DMA finish.
    HSUT0TXADR = (u32)buf_first_kick;
    HSUT0TXCNT = 2;
    printf("hsut0 init,HSUT0CON = 0x%X,HUART_BAUD_VAL = %d\n",HSUT0CON,HUART_BAUD_VAL);
    //printf("CLKCON0 = 0x%X, CLKCON2 = 0x%X\n",CLKCON0,CLKCON2);
}

AT(.com_text.dumpbuf_test)
void huart_wait_tx_finish(void)
{
    while(!(HSUT0CON&BIT(13))){
        WDT_CLR();
    }; //1 DMA finish  //0 dma not finish
}

AT(.com_text.dumpbuf_test)
void huart_putbuf(void *buf, u32 len)
{
    if (0 == len) {
        return;
    }
    huart_wait_tx_finish();     //等待上一次发送完成
    HSUT0TXADR = DMA_ADR(buf);
    HSUT0TXCNT = len;           //kick start auto clear tx pending
}

#if LED_MATRIX_HUART_TX
u8 led_tx_buf[6];
u8 freq_disp_en = 0;            //频谱显示开启时不发送ID3
void huart_tx_mode(u8 index)    //切模式发送UI显示
{
    led_tx_buf[0] = 'M';
    led_tx_buf[1] = 'O';
    led_tx_buf[2] = 'D';
    led_tx_buf[3] = index;
    huart_putbuf(led_tx_buf, 4);
}

void huart_tx_cmd(u8 cmd)       //命令发送
{
    led_tx_buf[0] = 'C';
    led_tx_buf[1] = 'M';
    led_tx_buf[2] = 'D';
    led_tx_buf[3] = cmd;
    huart_putbuf(led_tx_buf, 4);
}

extern u8 id3_buf[512];
void freq_disp_switch(void)
{
    if (!freq_disp_en) {
        freq_disp_en = 1;
        huart_tx_cmd(1);
    } else {
        freq_disp_en = 0;
        huart_tx_cmd(2);
    }
}
#endif

//测试 huart_putbuf 函数能否发送成功, dump_buf_init初始化前需要确认发送函数是否OK.
void dumphuart_io_test(void)
{
    huart_module_init();
    WDT_DIS();
    u8 dma_test_buf[] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    while(1) {
        huart_putbuf(dma_test_buf,8);
        delay_5ms(10);
        printf("->");
    }
}

//dump使用示例
//file_total 最大为6, 支持同时保存生成6个文件
u8 dump_head_buf[14*6];   //管理文件头需要的buf
void dump_buf_huart_init(void)
{
    printf("------> %s\n",__func__);
    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_head_buf,6,huart_putbuf,huart_wait_tx_finish);  //初始化dump模块
}
#endif  // HUART_DUMP_BUF_EN

//-------------------------------------------------
#if 0  //TEST_HUART_DUMP 测试代码
//循环发送正弦波数组进行测试
static u8 sinetbl_test[128] = {   //32 samples  //sine_16k_500hz_dual
	0x00, 0x00, 0x00, 0x00, 0x84, 0x0C, 0x84, 0x0C, 0x8C, 0x18, 0x8C, 0x18, 0xA4, 0x23, 0xA4, 0x23,
	0x5C, 0x2D, 0x5C, 0x2D, 0x56, 0x35, 0x56, 0x35, 0x44, 0x3B, 0x44, 0x3B, 0xEB, 0x3E, 0xEB, 0x3E,
	0x26, 0x40, 0x26, 0x40, 0xEB, 0x3E, 0xEB, 0x3E, 0x43, 0x3B, 0x43, 0x3B, 0x57, 0x35, 0x57, 0x35,
	0x5D, 0x2D, 0x5D, 0x2D, 0xA4, 0x23, 0xA4, 0x23, 0x8D, 0x18, 0x8D, 0x18, 0x83, 0x0C, 0x83, 0x0C,
	0x00, 0x00, 0x00, 0x00, 0x7C, 0xF3, 0x7C, 0xF3, 0x74, 0xE7, 0x74, 0xE7, 0x5D, 0xDC, 0x5D, 0xDC,
	0xA4, 0xD2, 0xA4, 0xD2, 0xA9, 0xCA, 0xA9, 0xCA, 0xBC, 0xC4, 0xBC, 0xC4, 0x16, 0xC1, 0x16, 0xC1,
	0xD9, 0xBF, 0xD9, 0xBF, 0x15, 0xC1, 0x15, 0xC1, 0xBC, 0xC4, 0xBC, 0xC4, 0xA8, 0xCA, 0xA8, 0xCA,
	0xA4, 0xD2, 0xA4, 0xD2, 0x5C, 0xDC, 0x5C, 0xDC, 0x74, 0xE7, 0x74, 0xE7, 0x7B, 0xF3, 0x7B, 0xF3
};

u8 dump_dma_buf[14+128];  //DMA需要的BUF
void dump_buf_huart_test(void)
{
    //dumphuart_io_test();  //测试高速串口DMA发送是否正常
	dump_buf_huart_init();
    printf("dump run...\n");
    int cnt =1024*3; //65536;//65536;  //65536*128 = 8M (8388608 byte)
    WDT_DIS();

#if 1
    while(cnt--) {  //同时导出6个文件测试
        dump_putbuf(dump_dma_buf,sinetbl_test,128,0);  //导出的数据会保存到文件0
        dump_putbuf(dump_dma_buf,sinetbl_test,128,1);  //导出的数据会保存到文件1
        dump_putbuf(dump_dma_buf,sinetbl_test,128,2);  //导出的数据会保存到文件2
        dump_putbuf(dump_dma_buf,sinetbl_test,128,3);  //导出的数据会保存到文件3
        dump_putbuf(dump_dma_buf,sinetbl_test,128,4);  //导出的数据会保存到文件4
        dump_putbuf(dump_dma_buf,sinetbl_test,128,5);  //导出的数据会保存到文件5
//        delay_5ms(100);
        if(cnt % 1024 == 0) {
            printf(".");
        }
    }
#else
 while(cnt--) {  //同时导出6个文件测试(先dma到ram的方式)
        dump_dma_wait();
        dump_put2ram(dump_dma_buf,sinetbl_test,128,0); //文件0数据先准备到ram中待发送
        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送

        dump_dma_wait();
        dump_put2ram(dump_dma_buf,sinetbl_test,128,1); //文件1数据先准备到ram中待发送
        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送

        dump_dma_wait();
        dump_put2ram(dump_dma_buf,sinetbl_test,128,2); //文件2数据先准备到ram中待发送
        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送

        dump_dma_wait();
        dump_put2ram(dump_dma_buf,sinetbl_test,128,3); //文件3数据先准备到ram中待发送
        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送

        dump_dma_wait();
        dump_put2ram(dump_dma_buf,sinetbl_test,128,4); //文件4数据先准备到ram中待发送
        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送

        dump_dma_wait();
        dump_put2ram(dump_dma_buf,sinetbl_test,128,5); //文件5数据先准备到ram中待发送
        dump_dma_kick(dump_dma_buf,14+128);            //KICK数据发送
//        delay_5ms(100);
        if(cnt % 1024 == 0) {
            printf(".");
        }
    }
#endif
    printf("dump_buf_test finish\n"); while(1);
}
#endif  //TEST_HUART_DUMP
