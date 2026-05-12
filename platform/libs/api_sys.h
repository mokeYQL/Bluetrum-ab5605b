#ifndef _API_SYS_H_
#define _API_SYS_H_

#define IRQ_TMR1_VECTOR                 4
#define IRQ_TMR2_VECTOR                 5
#define IRQ_IRRX_VECTOR                 6
#define IRQ_UART_VECTOR                 14
#define IRQ_HUART_VECTOR                15
#define IRQ_TMR3_VECTOR                 16
#define IRQ_TMR4_VECTOR                 17
#define IRQ_TMR5_VECTOR                 18
#define IRQ_SPI_VECTOR                  20
#define IRQ_PORT_VECTOR                 26


typedef struct {
    u16 vbg;            //VBG Voltage,  标准600*2 mV
    u8  vbat;           //trim到4.2V的寄存器配置值      CHAGCON[4:0]
    u8  vddcore;        //trim到1.06V的寄存器配置值     PWRCON0[3:0]
    u8  vddio;          //trim到3.3V的寄存器配置值      PWRCON0[6:4]
    u8  vbko;           //trim到1.6V的寄存器配置值      RTCCONT[4:0]
    u8  vddaud;         //trim到2.9V的寄存器配置值      AUANGCON0[6:4]
} sys_trim_t;
extern sys_trim_t sys_trim;

typedef void (*isr_t)(void);

extern u16 tmr5ms_cnt;

void sys_set_tmr_enable(bool tmr5ms_en, bool tmr1ms_en);
u32 sys_get_rand_key(void);

void xosc_init(void);

/*
*   pr = 1, 高优先级中断, 中断入口函数isr放公共区且要加FIQ, 例如：
*   AT(.com_text.isr) FIQ
*   void timer3_isr(void) {}
*
*   pr = 0, 低优先级中断,不用加FIQ, 函数放入公共区, 例如：
*   AT(.com_text.isr)
*   void timer3_isr(void) {}
*
*   中断函数需要尽量精简，否则影响系统效率。推荐用低优先级中断，需要很高响应速度时才用高优先级中断
*/
bool sys_irq_init(int vector, int pr, isr_t isr);           //初始化一个中断, pr为优先级

u16 get_random(u16 num);                                    //获取[0, num-1]之间的随机数
//输出时钟(type <=13)
//1:xosc26M //2:X26M->32K //3:osc32k //4:plldiv2/10 //5:xosc52m/10 //6:fmadc_d4//7:rc2m//8:rtc_rc2m
//9:system_clk/10 //12:dac_clkpp //13:fmtxpll_div4
void sys_clk_output(u32 type, u32 mapping);     //sys_clk_output(9, CLKOMAP_PA5);  //test for: sysclk/10
void sys_clk_output_div(u8 div);                            //在sys_clk_output 输出时钟基础上还可以进行（1~31）的分频


u16 convert_uni2gbk(u16 code, u32 addr, u32 len);           //输入utf16, 返回gbk编码, 找不到编码，使用'□'
u8 utf8_char_size(u8 code);                                 //返回UTF8占用几字节
u16 utf8_convert_to_unicode(u8 *in, u8 char_size);          //UTF-8转成UTF-16 LE编码
uint8_t * bt_get_package_rf_param(u8 package);
void vddio_voltage_configure(void);
u16 aux_dnr_get_max_pow(void);
void print_cache_lock_map(void);
u8  is_sd_insert_reset(void);      //是否是sd插入异常复位
void sd_insert_reset_clear(void);  //清除sd插入异常复位标志,调用它后,is_sd_insert_reset返回值会被清为0
//-------------------------------------------------
//dump 相关函数
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
//为了减少等待上次dma完成的时间,可以先通过dump_put2ram把数据封包放到ram中,再调用dump_dma_kick一次性导出数据
void dump_put2ram(void *dump_dma_buf, void *tx_buf, int tx_buf_len, u8 file_idx);
void dump_dma_kick(void *dma_buf,u32 dma_len);  //kick一次dma发送
void dump_dma_wait(void);  //等待上一次发送完成
#endif // _API_SYS_H_

