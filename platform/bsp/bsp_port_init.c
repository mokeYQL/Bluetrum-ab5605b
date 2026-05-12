#include "include.h"

#define IO_INT_EXAMPLE  1

#if IO_INT_EXAMPLE

extern isr_t register_isr(int vector, isr_t isr);
#define IRQ_PORT_INT_VECTOR          26   //PORT_INI的中断号为18


#define PORT_INT_R_F                 1  //下降沿和上升沿同时
#define PORT_INT_FALL                0  //下降沿
#define PORT_INT_RISE                0  //上升沿
AT(.com_text*)
const char strf[] = "[F_0x%X]\n";
AT(.com_text*)
const char strr[] = "[R_0x%X]\n";

AT(.com_text.port_int)
void port_int_isr(void)
{

#if PORT_INT_FALL   //下降沿产生中断示例
    if (WKUPEDG & BIT(22)) {  //fall
        printf(strf,WKUPEDG);
        WKUPCPND = BIT(22);
    }
#endif // PORT_INT_FALL

#if   PORT_INT_RISE    //上升级沿产生中断示例
    if (WKUPEDG & BIT(23)) { //rise
        printf(strr,WKUPEDG);
        WKUPCPND = BIT(23);
    }
#endif // PORT_INT_RISE

#if PORT_INT_R_F
//    //上升级沿和下降沿均产生中断示例
    if (WKUPEDG & BIT(22)) {  //fall -->rise
        printf(strf,WKUPEDG);
        WKUPCPND = BIT(22);
        PORTINTEDG &= ~BIT(8); //rise

    }
    if (WKUPEDG & BIT(23)) { //rise -->fall
        printf(strr,WKUPEDG);
        WKUPCPND = BIT(23);
        PORTINTEDG |= BIT(8);  //fall
   }
#endif // PORT_INT_R_F
}

void port_int_init(void)
{
    printf("port_int_init\n");
    GPIOBDE  |= BIT(0);
    GPIOBDIR |= BIT(0);
    GPIOBPU  |= BIT(0);

    PORTINTEN |= BIT(8);
    WKUPCON |= BIT(16);       //INT EN
    WKUPCPND = 0xff << 16;

//    上升级沿产生中断示例
#if PORT_INT_RISE | PORT_INT_R_F
    PORTINTEDG &= ~BIT(8); //rise
    WKUPEDG &= ~BIT(7);     //rise
    WKUPCON |= BIT(7);     //int_rise
#endif // PORT_INT_RISE

//    下降级沿产生中断示例
#if PORT_INT_FALL | PORT_INT_R_F
    PORTINTEDG |= BIT(8); //fall
    WKUPEDG |= BIT(6);
    WKUPCON |= BIT(6);     //int_fall
#endif // PORT_INT_FALL

//注册中断函数，设置优先级
    register_isr(IRQ_PORT_INT_VECTOR,port_int_isr);
    PICPR &= ~BIT(IRQ_PORT_INT_VECTOR);
    PICEN |= BIT(IRQ_PORT_INT_VECTOR);
    WDT_DIS();

    //测试用例PB1 500ms不断翻转,用于测试PB0边沿中断.(硬件上PB1和PB0用跳线连在一起)
#if 1
    GPIOBFEN  &= ~BIT(1);
    GPIOBDE |= BIT(1);
    GPIOBDIR &= ~BIT(1);
    while (1) {
        delay_ms(500);
        if (GPIOB & BIT(1)) {
            GPIOBCLR = BIT(1);
            printf("->0");
        } else {
            GPIOBSET = BIT(1);
            printf("->1");
        }
    }
#endif // 1
}
#endif // IO_INT_EXAMPLE
