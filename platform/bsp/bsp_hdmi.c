#include "include.h"
#include "bsp_hdmi.h"
#include "bsp_i2c.h"

#if FUNC_HDMI_EN
u8 edid_buf[256] = {0};

#if 1
#define  H_DEG   printf
#else
#define  H_DEG(...)
#endif
/*
*HDMI ARC IO使用: 一共用到 5 个IO口(IIC(2_PINS),  1个SPDIF信号输入, 1个插入检测,  1个CEC单线双向通信,)
*IIC的2个PIN选择见: config_extra.h中的I2C_SCL_IN/I2C_SCL_OUT等
*其它3个PIN(spdif/detect/cec)均在本文件如下宏中配置
*cec IO模拟时序进行双向通信,还需使能它的边沿中断功能(包括上升沿和下降沿)及两个timer, 其中:
*cec 发送需要1个100us精确中断timer.
*cec 接收需要1个计时timer(cnt计时单位:1us)
*/
//Default: SCL_PB1 SDA_PB2, SPDIF_PE5, CEC_DETECT_PE6, CEC_PE7
//----------------------------------------------------------------------------------
//SPDIF信号输入IO配置
#define HDMI_SPDIF_CH            SPF_PE5_CH1
//HDMI 插入检测IO配置
#define BIT_HDMI_DETECT          BIT(6)
#define HDMI_DETECT_PIN_INIT()  {GPIOEFEN &= ~BIT_HDMI_DETECT; GPIOEDE |= BIT_HDMI_DETECT; GPIOEDIR |= BIT_HDMI_DETECT; GPIOEPU |= BIT_HDMI_DETECT;}
#define HDMI_DETECT_ONLINE()    (!(GPIOE & BIT_HDMI_DETECT))  //HDMI insert, detect io is LOW

//CEC IO配置
#define BIT_HDMI_CEC          BIT(7)
#define HDMI_CEC_PIN_INIT()   {GPIOEFEN &= ~BIT_HDMI_CEC; GPIOEDE |= BIT_HDMI_CEC; GPIOEDIR |= BIT_HDMI_CEC; GPIOEPU |= BIT_HDMI_CEC;}
#define HDMI_CEC_PIN_STA()    (GPIOE & BIT_HDMI_CEC)
#define HDMI_CEC_PIN_HIGH()   {GPIOEDIR &= ~BIT_HDMI_CEC; GPIOESET = BIT_HDMI_CEC;}
#define HDMI_CEC_PIN_LOW()    {GPIOEDIR &= ~BIT_HDMI_CEC; GPIOECLR = BIT_HDMI_CEC;}
#define HDMI_CEC_PIN_INPUT()  {GPIOEDIR |= BIT_HDMI_CEC; GPIOEPU |= BIT_HDMI_CEC;}

//CEC IO 通信IO中断(上升沿及下沿降沿)配置
#define CEC_INT_PORT          (IO_PE7 -1)
#define HDMI_CEC_PIN_IRQ_INIT()      {GPIOEFEN &= ~BIT_HDMI_CEC; GPIOEDE |= BIT_HDMI_CEC; \
                                       GPIOEDIR |= BIT_HDMI_CEC; GPIOEPU |= BIT_HDMI_CEC;  WKUPCPND = BIT(22) | BIT(23);}
#define HDMI_CEC_PIN_IRQ_DIS()       {PORTINTEN &= ~BIT(CEC_INT_PORT);}
#define HDMI_CEC_PIN_IRQ_EN()        {PORTINTEN |= BIT(CEC_INT_PORT);}
#define HDMI_CEC_PIN_IRQ_EN_EDGE()   {PORTINTEN |= BIT(CEC_INT_PORT); WKUPEDG |= BIT(6); \
                                       WKUPCON |= BIT(6) | BIT(7) | BIT(16);  PORTINTEDG |= BIT(CEC_INT_PORT);}

//----------------------------------------------------------------------------------
//AT(.com_rodata.*)
//const char str_t[] = "->";
//CEC TX TIMER   需要保证100us调用一次
AT(.com_text.hdmi_isr)
FIQ void cec_tx_timer_isr(void)
{
    if (TMR4CON & BIT(16)) {
        TMR4CPND = BIT(16);              //Clear Pending
        cec_tx_timer_isr_callback();
        //printk(str_t);
    }
}

AT(.text.bsp.hdmi)
void cec_tx_timer_init(void)  //100us int
{
    H_DEG("%s\n",__func__);
    TMR4CON =  BIT(7);               //Timer overflow interrupt enable
    TMR4CNT = 0;
    TMR4PR  = 100- 1;                 //select xosc26_div 1M clk
    TMR4CON |= BIT(2) | BIT(0);      //Timer works in Counter Mode
    sys_irq_init(IRQ_TMR4_VECTOR, 1, cec_tx_timer_isr);
}

AT(.com_text.hdmi_isr)
void cec_tx_timer_clr(void)
{
    TMR4CNT = 0;
}

AT(.text.bsp.hdmi)
void cec_tx_timer_exit(void)
{
    TMR4CON = 0;
}
//----------------------------------------------------------------------------------
//CEC RX TIMER用于接收数据判断
AT(.com_text.hdmi_isr)
u32 cec_rx_timer_cnt(void)
{
    return TMR5CNT;
}

AT(.text.bsp.hdmi)
void cec_rx_timer_init(void)
{
    printf("%s\n",__func__);
    TMR5CNT = 0;
    TMR5PR  = 0xFFFFFFFF;            //1ms, select xosc26_div 1M clk
    TMR5CON |= BIT(2) | BIT(0);      //Timer works in Counter Mode
}
AT(.text.bsp.hdmi)
void cec_rx_timer_exit(void)
{
    TMR5CON = 0x00;
}
//----------------------------------------------------------------------------------

//CEC 边沿中断,用于接收数据解析
AT(.com_text.hdmi_isr)
FIQ void cec_rx_port_isr(void)
{
    if (WKUPEDG & BIT(23)) {  //rise
        //GPIOFSET = BIT(3);   //GPIOF3 for rx int Test
        cec_rx_port_isr_call_back(1);
        WKUPCPND = BIT(23);
        PORTINTEDG |= BIT(CEC_INT_PORT);  //en fall
    }

    if (WKUPEDG & BIT(22)) {  //fall
        //GPIOFCLR = BIT(3);   //GPIOF3 for rx int Test
        cec_rx_port_isr_call_back(2);
        WKUPCPND = BIT(22);
        PORTINTEDG &= ~BIT(CEC_INT_PORT); //en rise
    }
}

AT(.text.bsp.hdmi)
void cec_rx_port_isr_init(void)
{
    //GPIOFFEN &= ~BIT(3);  //TEST CODE
    //GPIOFDE |= BIT(3);
    //GPIOFDIR &= ~BIT(3);

    HDMI_CEC_PIN_IRQ_INIT();
    HDMI_CEC_PIN_IRQ_EN_EDGE();
    sys_irq_init(IRQ_PORT_VECTOR, 1, cec_rx_port_isr);
    HDMI_CEC_PIN_IRQ_DIS();
}

AT(.text.bsp.hdmi)
void cec_rx_port_isr_exit(void)
{
    HDMI_CEC_PIN_IRQ_DIS();
}
//hdmi IO或timer 配置结束
//----------------------------------------------------------------------------------
AT(.com_text.hdmi_io)
bool hdmi_cec_pin_sta(void) {
    return HDMI_CEC_PIN_STA();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_high(void) {
    HDMI_CEC_PIN_HIGH();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_low(void) {
    HDMI_CEC_PIN_LOW();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_input(void) {
    HDMI_CEC_PIN_INPUT();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_irq_init(void) {
    HDMI_CEC_PIN_IRQ_INIT();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_irq_dis(void) {
    HDMI_CEC_PIN_IRQ_DIS();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_irq_en(void) {
    HDMI_CEC_PIN_IRQ_EN();
}

AT(.com_text.hdmi_io)
void hdmi_cec_pin_irq_edge(void) {
    HDMI_CEC_PIN_IRQ_EN_EDGE();
}

AT(.text.bsp.hdmi)
void cec_pin_func_init(void)
{
    CEC_IO_FUNC cec_io;
    cec_io.high = hdmi_cec_pin_high;
    cec_io.low = hdmi_cec_pin_low;
    cec_io.input = hdmi_cec_pin_input;
    cec_io.sta = hdmi_cec_pin_sta;
    cec_io.irq_init = hdmi_cec_pin_irq_init;
    cec_io.irq_edge = hdmi_cec_pin_irq_edge;
    cec_io.irq_en = hdmi_cec_pin_irq_en;
    cec_io.irq_dis = hdmi_cec_pin_irq_dis;
    cec_io.tx_timer_clr = cec_tx_timer_clr;
    cec_io.rx_timer_cnt = cec_rx_timer_cnt;
    cec_io_func_cfg(&cec_io);
}

//AT(.com_text.hdmi_io1)
//const char hdmi_in_str[] = "-->HDMI_IN\n";
//AT(.com_text.hdmi_io1)
//const char hdmi_out_str[] = "-->HDMI_OUT\n";
AT(.com_text.hdmi_io)
void hdmi_detect(void)  //called in timer_isr 50ms
{
    if (HDMI_DETECT_ONLINE()) {
        if (dev_online_filter(DEV_HDMI)) {
          msg_enqueue(EVT_HDMI_INSERT);
//          printf(hdmi_in_str);
        }
    } else {
        if (dev_offline_filter(DEV_HDMI)) {
          msg_enqueue(EVT_HDMI_REMOVE);
//          printf(hdmi_out_str);
        }
    }
}

AT(.text.bsp.i2c)
bool hdmi_i2c_write_byte(unsigned char data)
{
	bool ret = false;
	bsp_i2c_start();
	bsp_i2c_tx_byte(0xA0);
	if (bsp_i2c_rx_ack()){
		bsp_i2c_tx_byte(data);
		if(bsp_i2c_rx_ack()){
			ret = true;
		}
	}
	bsp_i2c_stop();
	return ret;
}

AT(.text.bsp.i2c)
bool hdmi_i2c_read_byte(u8* read_data)
{
	bool ret = false;
	if (read_data){
		bsp_i2c_start();
		bsp_i2c_tx_byte(0xA0 |0x01);
		if (bsp_i2c_rx_ack()){
			*read_data = bsp_i2c_rx_byte();
			bsp_i2c_tx_nack();
			ret = true;
		}
		bsp_i2c_stop();
	}
	return ret;
}

AT(.text.bsp.i2c)
bool hdmi_read_edid(u8 *buf, u16 buf_len)
{
    bool ret = false;
    const u8 edid_header[8] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
    u8 addr = 0;
	while(1){
        WDT_CLR();
		if(!hdmi_i2c_write_byte(addr)){
			printf("#!!!hdmi_i2c write err\n");
			break;
		}
		if(!hdmi_i2c_read_byte(buf+addr)){
			printf("#!!!hdmi_i2c read err\n");
			break;
		}
		if (7 == addr ) {
            if (memcmp(buf, edid_header, sizeof(edid_header)) != 0) {
                printf("#!!!hdmi_i2c read edid_header error:\n");
                break;
            } else {
                printf("hdmi_i2c read edid_header OK\n");
            }
		}
		if ((255 == addr) || (addr >= buf_len)) {   //all edid read OK.
		    printf("hdmi read %d byte edid success\n",addr);
            ret = true;
            break;
		}
        addr++;
	}
    return ret;
}


AT(.text.bsp.hdmi)
void hdmi_detect_io_init(void)
{
    printf("%s\n", __func__);
    //GPIOF3 for rx int Test
    GPIOFFEN &= ~BIT(3);
    GPIOFDE |= BIT(3);
    GPIOFDIR &= ~BIT(3);

    HDMI_DETECT_PIN_INIT();
    dev_delay_times(DEV_HDMI, 10);  //HDMI 检测消抖次数
}

u8 wait_and_active_tv(void);
AT(.text.bsp.hdmi)
bool hdmi_get_physical_address(void)
{
    u16 physical_addr = 0;

    u16 retry = 20, ret;
    do {
        hdmi_read_edid(edid_buf,256);
        ret = edid_analyse(edid_buf, &physical_addr);   //physical_addr 要确保得到正确的physical_addr,否则电视可能无声音输出
        printf("edid_analyse ret =%d, addr = 0x%X, retry = %d\n",ret,physical_addr,retry);
        if (0 == ret) {
            break;
        }
        delay_5ms(2);
        retry--;
    } while(ret!=0 && retry);
    cec_broadcast_physical_addr(physical_addr);
    wait_and_active_tv();
    return true;
}

AT(.text.bsp.hdmi)
u8 bsp_get_hdmi_spdif_ch(void)
{
    return HDMI_SPDIF_CH;
}

bool hdmi_init_flag = false;
AT(.text.bsp.hdmi)
void bsp_hdmi_cec_init(void)
{
    if (hdmi_init_flag) return;
    hdmi_init_flag = true;
    I2C_IO_INIT();
    HDMI_CEC_PIN_INIT();
    cec_pin_func_init();
    cec_rx_port_isr_init();
    cec_rx_timer_init();   //timer_3
    cec_tx_timer_init();   //timer_1
    cec_lib_init();
    cec_dev_logic_addr_set(CEC_LOGADDR_AUDSYS);
}

AT(.text.bsp.hdmi)
void bsp_hdmi_cec_exit(void)
{
    hdmi_init_flag = false;
    cec_rx_timer_exit();
    cec_tx_timer_exit();
    cec_rx_port_isr_exit();
    cec_lib_exit();
}


u8 cec_tx_test(void)
{
    delay_us(1000*50);
    CEC_FRAME_T frame = {0};
    frame.src_dst = (0x00<<4) | (0x05);
    frame.opcode = 0xA5;
    frame.arg_len = 3;
    for(int i = 0; i<= frame.arg_len; i++) {
        frame.arg_buf[i] = i;
    }
#if 1
    cec_tx_frame(&frame);
#else
    bool ack = cec_tx_frame_wait(&frame);
    if (ack) {
        printf("recv ack");
    } else {
        printf("not recv ack\n");
    }
#endif
    return true;
}
#endif











