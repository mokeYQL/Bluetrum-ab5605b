#include "include.h"

#if 0 //IIC_PUTCHAR_DEBUG_EN

//iic delay config
#define IIC_DBG_DELAY_CNT       (1)
//iic io select  //如下配置的是PA6为IIC_CLK, PA5为IIC_DATA
#define IICDBG_CLK_GPIO         A
#define IICDBG_CLK_BIT          5
#define IICDBG_DAT_GPIO         A
#define IICDBG_DAT_BIT          4
//io macro
#define _GPIO_CAT1(y)           GPIO##y1
#define GPIO_CAT1(y)            _GPIO_CAT1(y)
#define _GPIO_CAT2(x,y)         GPIO##x##y
#define GPIO_CAT2(x,y)          _GPIO_CAT2(x,y)
#if 0   //read then write to set io sta
#define IIC_DBG_CLK_H()         GPIO_CAT1(IICDBG_CLK_GPIO) |= BIT(IICDBG_CLK_BIT);
#define IIC_DBG_CLK_L()         GPIO_CAT1(IICDBG_CLK_GPIO) &= ~BIT(IICDBG_CLK_BIT);
#define IIC_DBG_DAT_H()         GPIO_CAT1(IICDBG_DAT_GPIO) |= BIT(IICDBG_DAT_BIT);
#define IIC_DBG_DAT_L()         GPIO_CAT1(IICDBG_DAT_GPIO) &= ~BIT(IICDBG_DAT_BIT);
#else   //direct set io sta
#define IIC_DBG_CLK_H()         GPIO_CAT2(IICDBG_CLK_GPIO,SET) = BIT(IICDBG_CLK_BIT);
#define IIC_DBG_CLK_L()         GPIO_CAT2(IICDBG_CLK_GPIO,CLR) = BIT(IICDBG_CLK_BIT);
#define IIC_DBG_DAT_H()         GPIO_CAT2(IICDBG_DAT_GPIO,SET) = BIT(IICDBG_DAT_BIT);
#define IIC_DBG_DAT_L()         GPIO_CAT2(IICDBG_DAT_GPIO,CLR) = BIT(IICDBG_DAT_BIT);
#endif
#define IIC_DBG_CLK_DAT_INIT()  GPIO_CAT2(IICDBG_CLK_GPIO,FEN) &= ~BIT(IICDBG_CLK_BIT);  GPIO_CAT2(IICDBG_CLK_GPIO,DE) |= BIT(IICDBG_CLK_BIT); GPIO_CAT2(IICDBG_CLK_GPIO,DIR) &= ~BIT(IICDBG_CLK_BIT); \
                                GPIO_CAT2(IICDBG_DAT_GPIO,FEN) &= ~BIT(IICDBG_DAT_BIT);  GPIO_CAT2(IICDBG_DAT_GPIO,DE) |= BIT(IICDBG_DAT_BIT); GPIO_CAT2(IICDBG_DAT_GPIO,DIR) &= ~BIT(IICDBG_DAT_BIT);
AT(.com_text.iic_debug)
static void iic_dbg_delay(u32 delay)
{
    while(delay--) {
        asm("nop");
    }
}


AT(.com_text.iic_debug)
void iic_putchar(char dat)
{
    u8 i;
    //printf("iic_putchar = %c\n",dat);
    //start
    IIC_DBG_CLK_DAT_INIT();
    iic_dbg_delay(IIC_DBG_DELAY_CNT);
    IIC_DBG_DAT_L();
    iic_dbg_delay(IIC_DBG_DELAY_CNT);
    IIC_DBG_CLK_L();
    //tx 8 8bit
    for (i=0; i<8; i++) {
        iic_dbg_delay(IIC_DBG_DELAY_CNT);      //数据最好在SCL低电平中点处变化, 没有这个延时有的IIC从机会接收出错
        if (dat & BIT(7)) {
            IIC_DBG_DAT_H();
        } else {
            IIC_DBG_DAT_L();
        }
        iic_dbg_delay(IIC_DBG_DELAY_CNT);
        IIC_DBG_CLK_H();
        iic_dbg_delay(IIC_DBG_DELAY_CNT*2);
        IIC_DBG_CLK_L();
        dat <<= 1;
    }
    //tx stop
    iic_dbg_delay(IIC_DBG_DELAY_CNT);
    IIC_DBG_DAT_L();
    iic_dbg_delay(IIC_DBG_DELAY_CNT);
    IIC_DBG_CLK_H();
    iic_dbg_delay(IIC_DBG_DELAY_CNT);
    IIC_DBG_DAT_H();
    iic_dbg_delay(IIC_DBG_DELAY_CNT);
}

void iic_putchar_test(void)
{
    printf("iic_putchar_test run\n");
    WDT_DIS();
    while(1) {
        iic_putchar(0x55);
        iic_putchar(0x56);
        iic_putchar(0x57);
        delay_5ms(50);
        printf("->");

    }
}
//把上面的 iic_putchar 注册到printf函数中, my_printf_init(iic_putchar), 就可以使用printf打印了.

#endif // IIC_PUTCHAR_DEBUG_EN
