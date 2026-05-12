#ifndef __BSP_SPI_AUDIO_H__
#define __BSP_SPI_AUDIO_H__

#define				SPI3WIREEN			0
#define				SPI2WIREEN			1
#define 			SPI1DMAINT			1


#define             SPIEN               0       //SPI Enable bit 0:disable 1:enable
#define             SPISM               1       //Slave mode 0:master 1:slave
#define             SPI_BUSMODE_LOWBIT  2       //data bus width select low bit
#define             SPI_BUSMODE_HIGNBIT 3       //data bus width select high bit
#define             SPI_RXSEL           4       //TX:0 RX:1
#define             SPI_CLKIDS          5       //SPI clock state when idle 0:clock stay at 0 1:clk stay at 1
#define             SPI_SMPS            6       //SPI output edge select bit 0: falling edge 1:rising edge
#define             SPIIE               7       //SPI interrupt enable 0:disable 1:enable
#define             SPILF_EN            8       //SPI LFSR enable bit 0:disable 1:enable
#define             SPIMBEN             9       //SPI multiple bit bus enable bit
#define             SPIIOSS             10      //SPI sample data is at the same clock edge with output data 0:difference 1:same
#define             SPIPND              16      //SPI pending

#define             SPI1G1              0x01    //SPI通道设置
#define             SPI1G2              0x02
#define             SPI1G3              0x03
#define             SPI1G4              0x04
#define             SPI1G5              0x05


#define             RESET               0       //
#define             DAC_OBUF_SIZE       576     // SIN TEST:16 DAC TEST:576
#define             DAC_OBUF_SPI_SIZE   128

#define             SPI_CHK_REV         0
#define             SPI_IRQ_REV         1
#define             SPI_DMA_REV         2

#define             DAC_BUF_REV_MAX     2      //SIN TEST:75 DAC TEST:
#define 			AUBUFCON        	SFR_RW (SFR1_BASE + 0x02*4)

extern u8 spi_rxbuf_rear;          //循环队列尾指针
extern u8 spi_rxbuf_front;         //循环队列头指针

extern u32 dac_bufs[DAC_BUF_REV_MAX][DAC_OBUF_SPI_SIZE];

u8 is_spi_rebuf_empty(void);
u8 is_spi_rebuf_full(void);
void dac_bufs_init(void);             //循环队列初始化

void spi1_tx_init(u32 baud);            //发送端初始化SPI1
void spi1_read(u8 mode, void *buf, u32 len);    //RX 接收启动接口

void spi1_dma_sendbytes(void *data, u32 len);

void spi1_play(void);

void dac_src0_dma_out_proc(void *buf, uint len);
void dac_src0_onebyte_out_proc(u32 aubufdata);
void dac_src0_drc_stereo_proc(s16 *left, s16 *right);

#endif //__BSP_SPI_AUDIO_H__
