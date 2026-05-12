#include "include.h"

#define UART_EXAMPLE_EN   0    ///普通uart示例程序

#define UART_EXAMPLE1_EN   1    ///普通uart示例程序

#if  UART_EXAMPLE_EN
//u32 uart_buf = 0;
//u8 uart_rx_flag = 0;

///发送数据
AT(.com_text.uart)
void uart1_putc(char ch)
{
    while(!(UART1CON & BIT(8)));
    UART1DATA = ch;
}

///打印字符串
AT(.com_text.uart)
void uart1_puts(const char *str)
{
    while(*str){
        uart1_putc(*str++);
    }
}

///串口中断处理
AT(.com_text.uart)
void uart1_isr(void)
{
    if(UART1CON & BIT(9)){      //接收完成
        UART1CPND |= BIT(9);
        //需要把数据保存起来
//        uart_buf = UART1DATA;
//        uart_rx_flag = 1;
    }
}

void uart1_rx_printf(void)
{
    if(uart_rx_flag) {
        printf("\nuart_buf = 0x%X\n", uart_buf);
        uart_rx_flag = 0;
    }
}

///串口初始化
AT(.text.at_mode)
void uart1_init(u32 baud)
{
    u32 baud_cfg;
    //PA3_RX,PA4_TX(G2)
    GPIOAFEN |= BIT(3)|BIT(4);
    GPIOADIR |= BIT(3);
    GPIOADIR &= ~BIT(4);
    GPIOAPU |= BIT(3)|BIT(4);
    GPIOADE |= BIT(3)|BIT(4);
    //mapping uart1(tx/rx) -> G2
    FUNCMCON0 |= (0xf << 28)|(0xf << 24);
    FUNCMCON0 |= (0x2 << 28)|(0x2 << 24);

    CLKGAT0 |= BIT(14);                                 //enable uart1 clk

    UART1CON = 0;
    UART1CON |= BIT(5);                                 //clock src select uart_inc
    CLKCON1 |= BIT(14);                                 //uart_inc select x26m_clkdiv2
    baud_cfg = ((26000000/2+ baud/2)/baud) - 1;         //baud_cfg=(串口时钟/波特率)-1; 四舍五入
    UART1BAUD = (baud_cfg << 16)|baud_cfg;

    UART1CON |= (BIT(0)|BIT(2)|BIT(7));                 //使能uart,接收中断

    register_isr(IRQ_UART_VECTOR, uart1_isr);
    PICPR &= ~BIT(IRQ_UART_VECTOR);
    PICEN |= BIT(IRQ_UART_VECTOR);
}

//vusb作串口收发用
AT(.text.at_mode)
void vusb_uart1_init(u32 baud)
{
    u32 baud_cfg;

    //VUSB_TX+RX(G3)
    PWRCON0 |= BIT(30);
    RTCCON &= ~BIT(6);
    //单线收发map
    FUNCMCON0 |= (0xf << 28) | (0xf << 24);
    FUNCMCON0 |= (0x7 << 28) | (0x3 << 24);             //单线时RXMAP配0x7

    CLKGAT0 |= BIT(21);                                 //enable uart1 clk

    UART1CON = 0;
    UART1CON |= BIT(5);                                 //clock src select uart_inc
    CLKCON1 |= BIT(14);                                 //uart_inc select x26m_clkdiv2
    baud_cfg = ((26000000/2+ baud/2)/baud) - 1;         //baud_cfg=(串口时钟/波特率)-1; 四舍五入
    UART1BAUD = (baud_cfg << 16)|baud_cfg;
    UART1CON |= (BIT(0) | BIT(2) | BIT(6) | BIT(7));            //使能uart,接收中断,BIT(6)单线

    register_isr(IRQ_UART_VECTOR, uart1_isr);
    PICPR &= ~BIT(IRQ_UART_VECTOR);
    PICEN |= BIT(IRQ_UART_VECTOR);
}


#endif

#if  UART_EXAMPLE1_EN

void uart1_g1_rx_printf(void);
u32 uart_buf = 0;   //rx 中断接收保存
u8 uart_rx_flag = 0;//rx 中断接收标志位

volatile u16 tx_idx = 0;    //中断发送下标
volatile u16 tx_len = 0;    //中断发送字符串长度

AT(.com_text.buf)
char buf_tx[20];            //中断发送缓存buf

//中断发送数据
void uart1_tx_kick(u8* buf, u16 len)
{
    if (len > 20) {
        printf("exceeds the len = 12\n");
        return;
    }
    memcpy(buf_tx,buf,len);
    tx_idx = 0;
    tx_len = len;
    //printf("tx_idx = %d,len = %d, buf = %s\n",tx_idx,len,buf);
    UART1CON |= BIT(3);//tx interrup en
}



///串口中断处理
AT(.com_text.uart)
void uart1_isr(void)
{
    if(UART1CON & BIT(9)){      //接收完成
        UART1CPND |= BIT(9);
        //需要把数据保存起来
        uart_buf = UART1DATA;
        uart1_g1_rx_printf();
        uart_rx_flag = 1;
    }

    if(UART1CON & BIT(8)){      //发送完成
        UART1CPND |= BIT(8);
        if (tx_idx < tx_len) {
            UART1DATA = buf_tx[tx_idx];
            tx_idx++;
        }
        if (tx_idx >= tx_len) {
            UART1CON &= ~BIT(3);
        }

    }
}

void uart1_g1_rx_printf(void)
{
    if(uart_rx_flag) {
        printf("%c", uart_buf);
        uart_rx_flag = 0;
    }
}

///串口初始化
AT(.text.at_mode)
void uart1_g1_init(u32 baud)
{
    u32 baud_cfg;
    //PA6_RX,PA7_TX(G1)
    GPIOAFEN |= BIT(6)|BIT(7);
    GPIOADIR |= BIT(6);
    GPIOADIR &= ~BIT(7);
    GPIOAPU |= BIT(6)|BIT(7);
    GPIOADE |= BIT(6)|BIT(7);
    //mapping uart1(tx/rx) -> G1
    FUNCMCON0 |= (0xf << 28)|(0xf << 24);
    FUNCMCON0 |= (0x1 << 28)|(0x1 << 24);

    UART1CON = 0;
    UART1CON |= BIT(5);                                 //clock src select uart_inc
    CLKCON1 |= BIT(14);                                 //uart_inc select x26m_clkdiv2
    baud_cfg = ((26000000/2+ baud/2)/baud) - 1;         //baud_cfg=(串口时钟/波特率)-1; 四舍五入
    UART1BAUD = (baud_cfg << 16)|baud_cfg;

    UART1CON |= (BIT(0)|BIT(2)|BIT(7));                 //使能uart,bit(2)接收中断,bit(3)发送中断

    register_isr(IRQ_UART_VECTOR, uart1_isr);
    PICPR &= ~BIT(IRQ_UART_VECTOR);
    PICEN |= BIT(IRQ_UART_VECTOR);
}

char buf_tx_test[] = "1234567890a";

void uart1_g1_test(void)
{
    uart1_g1_init(115200);  //9600
    WDT_DIS();
    while(1) {
        delay_ms(1000);
        uart1_tx_kick((u8*)buf_tx_test,sizeof(buf_tx_test));//中断发送 每秒发送一次
        uart1_g1_rx_printf();  //中断接收
    }
}
#endif

