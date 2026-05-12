
#include "include.h"
#include "func.h"
#include "func_update.h"


#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if UART_M_UPDATE||UART_S_UPDATE

void uart0_isr_init(void);
void uart1_isr_init(void);
void uart2_isr_init(void);
void uart1_putchar(u8 ch);
void uart2_putchar(u8 ch);
bool uart_getchar(u8* ch);
bool uart1_getchar(u8* ch);
bool uart2_getchar(u8* ch);

//与系统时钟一一对应
u8 sys_clk4_uart_update[]= {2,26,48,52,80,120,160};
extern u8 upd_buf[512] AT(.upd_buf);
void uart_isr_rx_reset(void);
void ex_uart_rx_buf(u8 flag);

void uart_upd_port_init(u8 uart_port)
{

    u8 rx_port_sel=uart_port&0x03;
    u8 tx_port_sel=(uart_port&0x0C>>2);
    if (rx_port_sel==0){
        GPIOADE     |= UART_RX_BIT;
        GPIOAPU     |= UART_RX_BIT;
        GPIOADIR    |= UART_RX_BIT;
        GPIOAFEN    |= UART_RX_BIT;
        GPIOADRV    |= UART_RX_BIT;
    }else if(rx_port_sel==1) {
        GPIOBDE     |= UART_RX_BIT;
        GPIOBPU     |= UART_RX_BIT;
        GPIOBDIR    |= UART_RX_BIT;
        GPIOBFEN    |= UART_RX_BIT;
        GPIOBDRV    |= UART_RX_BIT;
    }else if(rx_port_sel==2) {
        GPIOEDE     |= UART_RX_BIT;
        GPIOEPU     |= UART_RX_BIT;
        GPIOEDIR    |= UART_RX_BIT;
        GPIOEFEN    |= UART_RX_BIT;
        GPIOEDRV    |= UART_RX_BIT;
    }else if(rx_port_sel==3) {
        GPIOFDE     |= UART_RX_BIT;
        GPIOFPU     |= UART_RX_BIT;
        GPIOFDIR    |= UART_RX_BIT;
        GPIOFFEN    |= UART_RX_BIT;
        GPIOFDRV    |= UART_RX_BIT;

    }
    if (tx_port_sel==0){
        GPIOADE  |= UART_TX_BIT;
        GPIOADIR &=~ UART_TX_BIT;
        GPIOAFEN |= UART_TX_BIT;
        GPIOADRV |= UART_TX_BIT;
    }else if(tx_port_sel==1) {
        GPIOBDE  |= UART_TX_BIT;
        GPIOBDIR &=~ UART_TX_BIT;
        GPIOBFEN |= UART_TX_BIT;
        GPIOBDRV |= UART_TX_BIT;
    }else if(tx_port_sel==2) {
        GPIOEDE  |= UART_TX_BIT;
        GPIOEDIR &=~ UART_TX_BIT;
        GPIOEFEN |= UART_TX_BIT;
        GPIOEDRV |= UART_TX_BIT;
    }else if(tx_port_sel==3) {
        GPIOFDE  |= UART_TX_BIT;
        GPIOFDIR &=~ UART_TX_BIT;
        GPIOFFEN |= UART_TX_BIT;
        GPIOFDRV |= UART_TX_BIT;
    }
}
void uart_upd_init(u32 uart_port_map,u32 uart_baud,u8 sys_clk)
{
    TRACE("uart_upd_init\n");
    u8 uart_port=(uart_port_map&0x0f);
    uart_port_map = uart_port_map&(~0x0f);
    if(UART0CON & BIT(0)) {
        while (!(UART0CON & BIT(8)));
    }
    CLKCON1 |= BIT(14);
    u32 uartxcon = BIT(5)|BIT(7)|BIT(4)|BIT(0);  //bit5 uart_inc , sel 26M div2 TO UART CLK
    u16 uart_baud_val = (26000000/2 + uart_baud/2)/uart_baud - 1;  //时钟源x26m_div2 //uart_baud/2 四舍五入
    u32 uartxbaud=(u32) uart_baud_val<<16|uart_baud_val;

#if UPDATE_UART_SEL==UPDATE_UART2
        if(uart_port_map&(0xf<<4)){
              FUNCMCON1 |=(0x0f<<4);
         }
         if(uart_port_map&(0xf<<8)){
              FUNCMCON1 |=(0x0f<<8);
         }
          CLKGAT1 |= BIT(11);
          FUNCMCON1 |= (u32)uart_port_map;
          UART2BAUD =uartxbaud;
          UART2CON =uartxcon;//RX_EN UART_EN
#else
        if(uart_port_map&(0xf<<8)){
            FUNCMCON0|=(0x0f<<8);
        }
        if(uart_port_map&(0xf<<12)){
              FUNCMCON0 |=(0x0f<<12);
        }
        if(uart_port_map&(0xf<<24)){
              FUNCMCON0|=(0x0f<<24);
        }
        if(uart_port_map&(0xf<<28)){
              FUNCMCON0 |=(0x0f<<28);
        }
         FUNCMCON0 |= (u32)uart_port_map;
#if UPDATE_UART_SEL==UPDATE_UART1
        UART1CON = 0;
        CLKGAT0 |= BIT(21);
        UART1BAUD = uartxbaud;
        UART1CON = uartxcon;
#else
        CLKGAT0 |= BIT(10);//UART0_CLK_EN
        UART0BAUD =uartxbaud;
        UART0CON =uartxcon;
#endif
#endif // UPDATE_UART_SEL
        uart_upd_port_init(uart_port);

}

void uart_upd_isr_init(void)
{
    uart_isr_rx_reset();
    ex_uart_rx_buf(0);
#if UPDATE_UART_SEL==UPDATE_UART0
    uart0_isr_init();
#elif UPDATE_UART_SEL==UPDATE_UART1
    uart1_isr_init();
#else
    uart2_isr_init();
#endif // UPDATE_UART_SEL
}


void upd_putchar(u8 ch)
{
#if UPDATE_UART_SEL==UPDATE_UART0
    uart_putchar(ch);
#elif UPDATE_UART_SEL==UPDATE_UART1
    uart1_putchar(ch);
#else
    uart2_putchar(ch);
#endif
}


bool upd_getchar(u8* ch)
{
#if UPDATE_UART_SEL==UPDATE_UART0
    return uart_getchar(ch);
#elif UPDATE_UART_SEL==UPDATE_UART1
    return uart1_getchar(ch);
#else
    return uart2_getchar(ch);
#endif
}
#endif //UART_M_UPDATE||UART_S_UPDATE

#if UART_M_UPDATE
extern const char upd_filename[];
const u8 str_send_upd_start_sign[]={'S','T','A','R','T','_','U','P','D','^','_','^'};
const u8 str_recive_success[]     ={'R','E','C','E','I','V','E','S','T','A','R','T'};
enum
{
UPD_M_SEND_START_SIGN,
UPD_M_RECIVE_CMD,
UPD_M_DONE,
};
enum
{
CMD_CHECK_UART_UPD=0x01,
CMD_SEND_DATA=0x02,
CMD_READ_STATUS=0x03,


};
#define UPD_CMD_LEN   12
typedef struct
{
    u16 sign;
    u8 cmd;
    u8 status;
    u32 addr;
    u32 len;//当为读取数据时，为读取数据的长度
    u16 crc;
}uart_upd_m_rxcmd_t;

//总共12byte
typedef struct
{
    u16 sign;
    u8 cmd;
    u8 status;
    u32 addr;
    u32 data_crc;//当为发送数据时，是发送数据对应的crc
    u16 crc;
}uart_upd_m_txcmd_t;

typedef struct
{
    u8 step;
    uart_upd_m_rxcmd_t rxcmd;
    uart_upd_m_txcmd_t txcmd;
    u32 addr;

}upd_uart_m_t;
upd_uart_m_t upd_uart_m;
void uart_m_upd_send(void * buf,u32 len)
{
    u8 *ptr=(u8 *)buf;
#if TRACE_EN
    my_print_r(ptr,len);
#endif // TRACE_EN
    for (u32 j=0;j<len;j++) {
        upd_putchar(ptr[j]);
    }
}
bool get_slave_rsp(void)
{
    u8 ch;
    static u8 cnt=0;
    //不要在里面加打印，不然从机发送太快会导致接收缺数据
    if(upd_getchar(&ch)){
        if(ch==str_recive_success[cnt]){
            cnt++;
            if(sizeof(str_recive_success)==cnt){
                upd_uart_m.step=UPD_M_RECIVE_CMD;
                return true;
            }
        }else {
            cnt=0;
        }
    }
//     my_printf("%x_%x  ",cnt,ch);
    return false;
}
void func_uart_update_enter(void)
{

    u8 buf[UPD_CMD_LEN];
    u32 get_rsp_tick=tick_get();
    fs_lseek(0, SEEK_SET);
    memset(&upd_uart_m,0,sizeof(upd_uart_m));
    memcpy(buf,str_send_upd_start_sign,sizeof(str_send_upd_start_sign));
    while(1){
//        printf("send_upd_start_sign\n");
      //100ms发送一次start命令
        if(tick_check_expire(get_rsp_tick,100)) {
            uart_m_upd_send(buf,sizeof(str_send_upd_start_sign));
            get_rsp_tick=tick_get();
        }
        if(get_slave_rsp()){
            break;
        }
        WDT_CLR();
    }



}
AT(.comm_rodate)
const char str_recive_cmd[]="recive_cmd:";
void  recive_cmd(u8 *buf,u16 len)
{
    TRACE(str_recive_cmd);
    GLOBAL_INT_DISABLE();
    u8 ch;
    u8 cnt=0;
    while(cnt!=len) {
        if(upd_getchar(&ch)){
            if(ch==0xAA&&cnt==0){
                buf[cnt++]=ch;
            }else if(ch==0x55&&cnt==1){
                buf[cnt++]=ch;
            }else if(cnt>1){
                 buf[cnt++]=ch;
            }
        }
        WDT_CLR();
    }
     GLOBAL_INT_RESTORE();
}
u32 uart_cal_calc_crc(void *buf,u32 len)
{
    u8 *ptr=(u8*)buf;
    u32 crc_sum=0;
    for(u32 i=0;i<len;i++){
        crc_sum += ptr[i];
    }
    return crc_sum;
}


bool upd_send_data(uart_upd_m_rxcmd_t *rxcmd,uart_upd_m_txcmd_t *txcmd)
{
    delay_ms(5);//每次接收到cmd等待5ms的时间，等待从机准备完毕
    uint rlen=0;
    for(u32 len=0;len<rxcmd->len;len+=512)
    {
        if(upd_uart_m.addr!=rxcmd->addr){
            if (FR_OK != fs_lseek(rxcmd->addr >> 9, SEEK_SET)) {
                return false;
            }
        }
        upd_uart_m.addr = rxcmd->addr + len;

        if(FR_OK == fs_read(upd_buf, 512, &rlen)) {
//            my_print_r(upd_buf,rlen);
            txcmd->sign =0x55AA;
            txcmd->cmd=rxcmd->cmd;
            txcmd->addr=rxcmd->addr+len;
            txcmd->data_crc=uart_cal_calc_crc(upd_buf,rlen);
            txcmd->crc=(u16)uart_cal_calc_crc(txcmd,sizeof(uart_upd_m_txcmd_t)-2);
            uart_m_upd_send(txcmd,sizeof(uart_upd_m_txcmd_t));
            uart_m_upd_send(upd_buf,rlen);

        }

    }
    return true;

}
void rsp_check_uart_mode(uart_upd_m_txcmd_t *txcmd)
{
    delay_ms(5);//每次接收到cmd等待5ms的时间，等待从机准备完毕
    memset(txcmd,0,sizeof(uart_upd_m_txcmd_t));
    txcmd->sign =0x55AA;
    txcmd->cmd=CMD_CHECK_UART_UPD;
    txcmd->crc=(u16)uart_cal_calc_crc(txcmd,sizeof(uart_upd_m_txcmd_t)-2);
    uart_m_upd_send(txcmd,sizeof(uart_upd_m_txcmd_t));
}
void upd_read_status(uart_upd_m_rxcmd_t *rxcmd)
{
    TRACE(" status %x\n",rxcmd->status);
    if(rxcmd->status==0xff) {
        TRACE("UPD_DONE\n");
         upd_uart_m.step=UPD_M_DONE;
    }
}
void func_uart_update_event(void)
{
    uart_upd_m_rxcmd_t *rxcmd=&upd_uart_m.rxcmd;
    uart_upd_m_txcmd_t *txcmd =&upd_uart_m.txcmd;
    u16 len=sizeof(uart_upd_m_rxcmd_t);
    while(upd_uart_m.step!=UPD_M_DONE) {
        WDT_CLR();
        recive_cmd((u8*)rxcmd,len);
#if TRACE_EN
        my_print_r(rxcmd,len);
#endif // TRACE_EN
        switch(rxcmd->cmd)
        {
            case CMD_CHECK_UART_UPD:
                rsp_check_uart_mode(txcmd);
                break;
            case CMD_SEND_DATA:
                upd_send_data(rxcmd,txcmd);
                break;
            case CMD_READ_STATUS:
                upd_read_status(rxcmd);
                break;
            default:
                break;
        }

    }
}
void func_uart_update_exit(void)
{

}
void func_uart_update(void)
{
    if(fs_open(upd_filename, FA_READ)==FR_OK){
        func_uart_update_enter();
        func_uart_update_event();
        func_uart_update_exit();
    }

}
#endif
#if UART_S_UPDATE

typedef struct
{
    u32 uart_upd_baud;
    u32 uart_port_sel;
    u8 rx_bit;
    u8 tx_bit;
    u8 uart_sel;
}uart_upd_init_t;

uart_upd_init_t uart_upd_p_init = {
    .uart_upd_baud    = UART_UPD_BAUD,
    .uart_port_sel    = UART_UPD_PORT_SEL,
    .rx_bit           = UART_RX_BIT,
    .tx_bit           = UART_TX_BIT,
    .uart_sel         = UPDATE_UART_SEL,
};


u8 uart0_get(u8 *ch);
u8 uart1_get(u8 *ch);
u8 uart2_get(u8 *ch);
//void uart_putchar(u8 ch);
void uart1_putchar(u8 ch);
void uart2_putchar(u8 ch);

void get_uart_upd_info(u8 *buf,u8 len)
{
    memcpy(buf,&uart_upd_p_init,len);
}

AT(.com_text.uart_isr)
void put_upd_uart_msg(void)
{
     msg_enqueue(EVT_UART_UPDATE);
}
//中断获取
u8 upd_uart_isr_get (u8 *ch)
{
    #if UPDATE_UART_SEL==UPDATE_UART0
        return uart0_get(ch);
    #elif UPDATE_UART_SEL==UPDATE_UART1
        return  uart1_get(ch);
    #else
        return  uart2_get(ch);
    #endif
}

void upd_uart_putchar(u8 ch)
{
    #if UPDATE_UART_SEL==UPDATE_UART0
        uart_putchar(ch);
    #elif UPDATE_UART_SEL==UPDATE_UART1
        uart1_putchar(ch);
    #else
        uart2_putchar(ch);
    #endif // UART_UPD_PORT_SEL
}

void uart_upd_isr_disable(void)
{
#if UPDATE_UART_SEL==UPDATE_UART0
    UART0CON &=~(BIT(2)|BIT(3));
#elif UPDATE_UART_SEL==UPDATE_UART1
    UART1CON &=~(BIT(2)|BIT(3));
#else
    UART2CON &=~(BIT(2)|BIT(3));
#endif // UPDATE_UART_SEL
}
#endif // UART_S_UPDATE

