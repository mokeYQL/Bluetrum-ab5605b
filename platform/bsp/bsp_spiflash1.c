#include "include.h"
#include "bsp_spiflash1.h"

#if EX_SPIFLASH_SUPPORT


#define  SPI_2WIRE_MODE     1      //1_2WIRE: CLK DIO CS (PA3,PA4,PA6)  //0_3WIRE: CLK,DO,DI,CS (PA3/PA4/PA5/PA6)  //这里CS可以用任意IO,命名是未记入WIRE数中

#if 0
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


void spi1_cs_en(void);
void spi1_cs_dis(void);
void spi1_cs_init(void);

#define SPI1_CS_INIT()           spi1_cs_init()
#define SPI1_CS_EN()             spi1_cs_en()
#define SPI1_CS_DIS()            spi1_cs_dis()

#define SF_PROGRAM              0x02                //SPIFlash编程
#define SF_READ                 0x03                //SPIFlash读取
#define SF_READSSR              0x05                //SPIFlash读状态寄存器
#define SF_WRITE_EN             0x06                //SPIFlash写使能
#define SF_ERASE                0x20                //SPIFlash擦除 //sector erase(4K擦除)


#define SPIFLASH_ID              0xEF401400  //通过读ID判断FLASH是否在线, 需要改成SPIFLASH对应的ID
#define SPIFALSH_BAUD            (500000)    //SPI波特率500K

AT(.text.spiflash1_drv)
void spi1_cs_init(void)
{
    GPIOAFEN &= ~BIT(6);
    GPIOADE |= BIT(6);
    GPIOADIR &= ~BIT(6);
    GPIOASET = BIT(6);
}

AT(.text.spiflash1_drv)
void spi1_cs_en(void)
{
    GPIOACLR = BIT(6);
}

AT(.text.spiflash1_drv)
void spi1_cs_dis(void)
{
    GPIOASET = BIT(6);
}

///SPIFlash Delay
AT(.text.spiflash1_drv)
static void spi1_delay(void)
{
    uint cnt = 20;
    while (cnt--) {
        asm("nop");
    }
}

///SPI接口获取1Byte数据
AT(.text.spiflash1_drv)
static u8 spi1_getbyte(void)
{
#if SPI_2WIRE_MODE
    SPI1CON |= BIT(4);                              //RX
#endif
    SPI1BUF = 0xff;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
    return SPI1BUF;
}

///SPI接口发送1Byte数据
AT(.text.spiflash1_drv)
static void spi1_sendbyte(u8 data)
{
#if SPI_2WIRE_MODE
    SPI1CON &= ~BIT(4);                             //TX
#endif
    SPI1BUF = data;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
}

AT(.text.spiflash1_drv)
u32 spiflash1_id_read(void)
{
    u32 id = 0,i;
    SPI1_CS_EN();
    spi1_delay();
    spi1_sendbyte(0x9F);
    for (i = 0; i < 4; i++) {
        id <<= 8;
        id |= spi1_getbyte();
    }
    SPI1_CS_DIS();
    return id;
}

AT(.text.spiflash1_drv)
void spi1_init(u32 baud)
{
#if SPI_2WIRE_MODE//2WIRE MODE: CLK DIO CS (PA3,PA4,PA6)  //CS可以用任意IO,命名是未记入WIRE数中
    GPIOAFEN |= BIT(3)|BIT(4);
    GPIOADE  |=  BIT(3)|BIT(4);
    GPIOADIR &= ~(BIT(3) | BIT(4));  //CLK,DO output
    GPIOADIR |= BIT(4);  //DI input
    GPIOAPU  |= BIT(4);   //DI pull up 10K
#else //3WIRE_MODE: CLK,DO,DI,CS (PA3/PA4/PA5/PA6)  //CS可以用任意IO,命名是未记入WIRE数中
    GPIOAFEN |= BIT(3)|BIT(4) | BIT(5);
    GPIOADE  |=  BIT(3)|BIT(4) | BIT(5);;
    GPIOADIR &= ~(BIT(3) | BIT(4));  //CLK,DO output
    GPIOADIR |= BIT(5);     //DI input
    GPIOAPU  |= BIT(5);      //10k Pull up
#endif

    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = (0x01<<12);  //G1


//    //SPI1_G2:  DIO_PA7,  CLK_PA6, //CS_PA5
//    GPIOAFEN |= BIT(6)|BIT(7);
//    GPIOADE |= BIT(6)|BIT(7);
//    GPIOADIR &= ~ BIT(6) ;  //CLK output
//    GPIOADIR |= BIT(7);  //DI input
//    GPIOAPU |= BIT(7);   //DI pull up 10K
//    FUNCMCON1 = (0x0F<<12);
//    FUNCMCON1 = (0x02<<12); //G2

    SPI1_CS_INIT();
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SPI1BAUD = 26000000/baud - 1;   //sys 26M.
#if SPI_2WIRE_MODE
    SPI1CON =  0x01 | (1<<2);  //2 WIRE MODE
#else
    SPI1CON =  0x01;   //3 WIRE MODE
#endif
}

///SPIFlash写使能
AT(.text.spiflash1_drv)
void spiflash1_init(void)
{
    spi1_init(SPIFALSH_BAUD);
    //while(1) {WDT_CLR();printf("Flash ID = 0x%X\n", spiflash1_id_read());delay_ms(300);}

}

///SPIFlash写使能
AT(.text.spiflash1_drv)
static void spiflash1_write_enable(void)
{
    SPI1_CS_EN();
    spi1_sendbyte(SF_WRITE_EN);
    SPI1_CS_DIS();
    spi1_delay();
}

///SPIFlash读状态寄存器
AT(.text.spiflash1_drv)
static uint spiflash1_readssr(void)
{
    uint ssr;
    SPI1_CS_EN();
    spi1_sendbyte(SF_READSSR);
    ssr = spi1_getbyte();
    SPI1_CS_DIS();
    spi1_delay();
    return ssr;
}

///SPIFlash等待BUSY
AT(.text.spiflash1_drv)
static void spiflash1_waitbusy(void)
{
    do {
        spi1_delay();
    } while (spiflash1_readssr() & 0x01);
}

///发送SPIFlash的地址
AT(.text.spiflash1_drv)
static void spiflash1_sendaddr(u32 addr)
{
    spi1_sendbyte(addr >> 16);
    spi1_sendbyte(addr >> 8);
    spi1_sendbyte(addr);
}

///SPIFlash读取
AT(.text.spiflash1_drv)
int spiflash1_read(void *buf, u32 addr, u32 len)
{
    TRACE("[r:0x%X,%d]",addr, len);
    u32 i;
    u8 *read_buf = (u8*)buf;
#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)
    u32 remain = 0;
    if (addr >= SPIFLASH_REC_END_ADDR) {
        addr = SPIFLASH_REC_BEGIN_ADDR + (addr - SPIFLASH_REC_END_ADDR);
    }

    if (addr <= SPIFLASH_REC_END_ADDR - len) {
        remain = 0;
    } else if (addr < SPIFLASH_REC_END_ADDR) {  //last sector
        remain = len - (SPIFLASH_REC_END_ADDR - addr);
        len = (SPIFLASH_REC_END_ADDR - addr);
    }

    SPI1_CS_EN();
    spi1_sendbyte(SF_READ);
    spiflash1_sendaddr(addr);
    for (i = 0; i < len; i++) {
       read_buf[i] = spi1_getbyte();
    }
    SPI1_CS_DIS();

    if (remain) {  //read remain
        addr = SPIFLASH_REC_BEGIN_ADDR;
        len = remain;
        SPI1_CS_EN();
        spi1_sendbyte(SF_READ);
        spiflash1_sendaddr(addr);
        for (i = 0; i < len; i++) {
           read_buf[i] = spi1_getbyte();
        }
        SPI1_CS_DIS();
    }

#else
    SPI1_CS_EN();
    spi1_sendbyte(SF_READ);
    spiflash1_sendaddr(addr);
#if 0  //DMA READ MODE
#if SPI_2WIRE_MODE
    SPI1CON |= BIT(4);                             //RX
#endif
    SPI1DMAADR = DMA_ADR(read_buf);
    SPI1DMACNT = len;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
#else  //BYTE READ MODE
    for (i = 0; i < len; i++) {
       read_buf[i] = spi1_getbyte();
    }
    SPI1_CS_DIS();
#endif

#endif
    return len;
}

///SPIFlash编程
AT(.text.spiflash1_drv)
void spiflash1_write(void *buf, u32 addr, u32 len)
{
    TRACE("[w:0x%X,%d]",addr, len);

    u8 *write_buf = (u8*)buf;
    spiflash1_write_enable();
    SPI1_CS_EN();
    spi1_sendbyte(SF_PROGRAM);
    spiflash1_sendaddr(addr);

#if 0   //DMA WRITE MODE
#if SPI_2WIRE_MODE
    SPI1CON &= ~BIT(4);                             //TX
#endif
#if SPIFALSH_MUSIC_BIN_WRITE_TEST
    u32 w_rambuf[256/4];
    memcpy(w_rambuf,write_buf,256);  //SPI1DMAADR 地址需要在RAM中,如果没在RAM中(在FLASH中),需要拷贝放到RAM中中转一次
    SPI1DMAADR = DMA_ADR(w_rambuf);
#else
    SPI1DMAADR = DMA_ADR(write_buf);
#endif
    SPI1DMACNT = len;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
#else    //BYTE WRITE MODE
    u32 i;
    for (i = 0; i < len; i++) {
        spi1_sendbyte(write_buf[i]);
    }
#endif
    SPI1_CS_DIS();
    spiflash1_waitbusy();
}

///SPIFlash擦除
AT(.text.spiflash1_drv)
void spiflash1_erase(u32 addr)
{
    TRACE("spi erase: %x\n", addr);
    spiflash1_write_enable();

    SPI1_CS_EN();
    spi1_sendbyte(SF_ERASE);
    spiflash1_sendaddr(addr);
    SPI1_CS_DIS();

    spiflash1_waitbusy();
}

////#define SF_ERASE_BLOCK          0xD8              //SPIFlash擦除  //block erase   //64K
//AT(.text.spiflash1_drv)
//void spiflash1_erase_block(u32 addr)
//{
//    TRACE("spi erase: %x\n", addr);
//    spiflash1_write_enable();
//
//    SPI1_CS_EN();
//    spi1_sendbyte(SF_ERASE_BLOCK);
//    spiflash1_sendaddr(addr);
//    SPI1_CS_DIS();
//
//    spiflash1_waitbusy();
//}

AT(.text.spiflash1_drv)
bool is_exspiflash_online(void)
{
#if INTERNAL_FLASH_REC
    return true;
#endif
    u32 flash_id;
    u32 i = 5;
    while(i--) {
        WDT_CLR();
        flash_id = spiflash1_id_read();
        printf("flash id = 0x%X\n",flash_id);
        if (SPIFLASH_ID == flash_id) {
            printf("flash_id ok\n");
            return true;
        } else {
            printf("--->!!!Read Flash ID Err\n",flash_id);
            delay_ms(1);
        }
    }
    return false;

}

#if SPIFALSH_MUSIC_BIN_WRITE_TEST
extern u8 const exspiflash_music_bin[36864];
void write_music_bin_to_spiflash(void)
{
    spiflash1_init();
    u8 rx_buf[256];
    int block_total,block_ok, block_fail, i;
    block_total = sizeof(exspiflash_music_bin)/256;
    for (i = 0; i < sizeof(exspiflash_music_bin); ) {
        spiflash1_erase(i);
        printf("erase addr = 0x%X\n",i);
        i += 4096;
    }
    printf("write exspiflash_music_bin,len = %d, block_total = %d\n", sizeof(exspiflash_music_bin) ,block_total);
    //write
    for (i = 0; i < block_total; i++) {
        spiflash1_write((void*)&exspiflash_music_bin[i*256], i*256, 256);
        printf("[0x%X]",i*256);
    }
    //read_back and compare
    printf("\nread_back and compare\n");
    block_ok = 0;
    block_fail = 0;
    for (i = 0; i < block_total; i++) {
        spiflash1_read(rx_buf,i*256,256);
        if (memcmp(rx_buf,&exspiflash_music_bin[i*256],256)) {
            printf("[fail:0x%X]",i*256);
            block_fail++;
        } else {
            printf("[ok:0x%X]",i*256);
            block_ok++;
        }
    }
    printf("block_ok = %d  block_fail = %d\n", block_ok, block_fail);
    WDT_DIS(); while(1);
}
#endif  //SPIFALSH_MUSIC_BIN_WRITE_TEST

#endif //(EX_SPIFLASH_SUPPORT & EXSPI_REC)

#if UDE_STOAGE_FLASH_EN
/************* SPI FLASH USB ******************/
//最好是小文件，大文件写入会很慢
#define W25Q80                  0xEF13       //1M
#define W25Q16                  0xEF14       //2M
#define W25Q32                  0xEF15       //4M
#define W25Q64                  0xEF16       //8M
#define W25Q128                 0xEF17       //16M
#define P25Q16SH                0x8514       //16M  //(manu_id >> 8) == 0xe
#define P25Q64SH                0x8516       //16M
#define W25Q80_SEC              (1024*1024*1/512)
#define W25Q16_SEC              (1024*1024*2/512)
#define W25Q32_SEC              (1024*1024*4/512)
#define W25Q64_SEC              (1024*1024*8/512)
#define W25Q128_SEC             (1024*1024*16/512)
#define P25Q16SH_SEC            (1024*1024*4/512)
#define P25Q64SH_SEC            (1024*1024*8/512)   //实际有64Mbyte
#define SF_PROGRAM              0x02                //SPIFlash编程
#define SF_READ                 0x03                //SPIFlash读取
#define SF_READSSR              0x05                //SPIFlash读状态寄存器
#define SF_WRITE_EN             0x06                //SPIFlash写使能
#define SF_ERASE                0x20                //SPIFlash擦除 //sector erase(4K擦除)
#if FLASH_GREATER_THAN_16MBYTE
#define Enter4byteAddrs         0xb7
#endif
#define SPI_BAUD_USB            15   //15M  //40
#define SPITEXT_USB_DIR         .usbdev.spiflash1_drv      //.com_text.spiflash1_drv


//AT(.usbdev.com.spiflash_more_cache)
//bool ude_spiflash_more_cache_enable(void)   //libdriver.a中有同名WEAK函数,默认返回true 加大缓存提速 //返回false时为以前不加缓存方式
//{
//	return true;
//}

//dma 模式
AT(.spi_flash_ram)
u8 spi_flash_ram[4096]; //AT(.spi_flash_ram);
u16 manu_id;
AT(.usbdev.spiflash1_drv)
static u8 spi1_getbyte_usb(void)
{
    SPI1CON |= (BIT(4) | BIT(6));                              //RX
    SPI1BUF = 0xff;
    while (!(SPI1CON & BIT(16)));                              //Wait pending
    SPI1CON &= ~BIT(6);
    return SPI1BUF;
}

AT(.usbdev.spiflash1_drv)
static void spi1_delay_usb(void)
{
    uint cnt = 20;
    while (cnt--) {
        asm("nop");
    }
}

AT(.usbdev.spiflash1_drv)
void spi1_cs_dis_usb(void)
{
    GPIOASET = BIT(5);
}

AT(.usbdev.spiflash1_drv)
void spi1_cs_en_usb(void)
{
    GPIOACLR = BIT(5);
}

AT(.usbdev.spiflash1_drv)
static void spi1_sendbyte_usb(u8 data)
{
    SPI1CON &= ~BIT(4);                             //TX
    SPI1BUF = data;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
}

///发送SPIFlash的地址
AT(.usbdev.spiflash1_drv)
static void spiflash1_sendaddr_usb(u32 addr)
{
//#if FLASH_GREATER_THAN_16MBYTE
//    spi1_sendbyte_usb(addr >> 24);
//#endif
    spi1_sendbyte_usb(addr >> 16);
    spi1_sendbyte_usb(addr >> 8);
    spi1_sendbyte_usb(addr);
}

///SPIFlash读状态寄存器
AT(.usbdev.spiflash1_drv)
static uint spiflash1_readssr_usb(void)
{
    uint ssr;
    spi1_cs_en_usb();
    spi1_sendbyte_usb(SF_READSSR);
    ssr = spi1_getbyte_usb();
    spi1_cs_dis_usb();
    spi1_delay_usb();
    return ssr;
}

///SPIFlash等待BUSY
AT(.usbdev.spiflash1_drv)
static void spiflash1_waitbusy_usb(void)
{
    do {
        spi1_delay_usb();
    } while (spiflash1_readssr_usb() & 0x01);
}

AT(.usbdev.spiflash1_drv)
void spiflash1_read_dma(void *buf, u32 addr, u32 len)
{
//    printf("read_addr:%x\n",addr);
    u8 *read_buf = (u8*)buf;
    spi1_cs_en_usb();
    spi1_sendbyte_usb(SF_READ);

    spiflash1_sendaddr_usb(addr);
    SPI1CON |= (BIT(4) | BIT(6));
    SPI1DMAADR = DMA_ADR(read_buf);
    SPI1DMACNT = len;
    while(!(SPI1CON & BIT(16)));
    SPI1CON &= ~BIT(6);
    spi1_cs_dis_usb();
}

///SPIFlash写使能
AT(.usbdev.spiflash1_drv)
static void spiflash1_write_enable_usb(void)
{
    spi1_cs_en_usb();
    spi1_sendbyte_usb(SF_WRITE_EN);
    spi1_cs_dis_usb();
    spi1_delay_usb();
    spiflash1_waitbusy_usb();
}

///SPIFlash擦除
AT(.usbdev.spiflash1_drv)
void spiflash1_erase_usb(u32 addr)
{
    spiflash1_write_enable_usb();

    spi1_cs_en_usb();
    spi1_sendbyte_usb(SF_ERASE);
    spiflash1_sendaddr_usb(addr);
    spi1_cs_dis_usb();

    spiflash1_waitbusy_usb();
}

//要256字节页写 一次最大写256个字节
AT(.usbdev.spiflash1_drv)
void spi_flash_page_write_do(u8 *buf, u32 addr, u32 len)
{
    u8 *write_buf = buf;
    spiflash1_write_enable_usb();
    spi1_cs_en_usb();
    spi1_sendbyte_usb(SF_PROGRAM);
    spiflash1_sendaddr_usb(addr);
    SPI1CON &= ~BIT(4);                             //TX
    SPI1DMAADR = DMA_ADR(write_buf);
    SPI1DMACNT = len;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
    spi1_cs_dis_usb();
    spiflash1_waitbusy_usb();
}

AT(.usbdev.spiflash1_drv)
void spiflash1_page_write(u8 *buf, u32 addr, u32 len)
{
    u8 *page_buf = buf;
    u32 pagerem;
    pagerem = 0xff - (addr&0xff) + 1;
//    pagerem =256 - addr%256;
    pagerem = (pagerem < len) ? pagerem : len;
    while(1){
        spi_flash_page_write_do(page_buf,addr,pagerem);
        if(pagerem == len) break;
        else {
            page_buf += pagerem;
            addr += pagerem;
            len -= pagerem;
            if(len > 256){
                pagerem = 256;
            } else {
                pagerem = len;
            }
        }
    }
}

AT(.usbdev.spiflash1_drv)
void spiflash1_write_dma(void *buf, u32 addr, u32 len)
{
    u32 secpos;
    u32 secoff;
    u32 secremain,i;
    u8 *dma_buf = (u8*)buf;

    secpos = addr >> 12;
    secoff = addr & 0xfff;
    secremain = 4096 - secoff;
    secremain = (secremain > len) ? len : secremain;
    //printf("secpos:%d secoff:%d secremain:%d\n",secpos,secoff,secremain);

    while(1){
        WDT_CLR();
        spiflash1_read_dma(spi_flash_ram,secpos << 12,4096);
        for(i = 0; i < secremain; i++){
            if(spi_flash_ram[secoff + i] != 0xFF) break;
        }
//        print_r(&spi_flash_ram[secoff],512);
        if(i < secremain){
            spiflash1_erase_usb(secpos << 12);
            //memset(spi_flash_ram,0xff,sizeof(spi_flash_ram));
            for(i = 0; i < secremain; i++){
                spi_flash_ram[secoff + i] = dma_buf[i];
            }
            //重写整个4k扇区
            spiflash1_page_write(spi_flash_ram,secpos << 12,4096);
        } else {
            spiflash1_page_write(dma_buf,addr,secremain);
        }
        if(secremain == len){
            break;
        } else {
            secpos++;
            secoff = 0;
            dma_buf += secremain;
            addr += secremain;
            len -= secremain;
            secremain = (len > 4096) ? 4096 : len;
        }
    }

}

AT(.usbdev.spiflash1_drv)
u16 spiflash1_manu_id_read(void)
{
    u32 i;
    u8 id_reg[] = {0x90,0x00,0x00,0x00};
    manu_id = 0;
    spi1_cs_en_usb();
    spi1_delay_usb();

    SPI1CON &= ~BIT(4);                             //TX
    SPI1DMAADR = DMA_ADR(id_reg);
    SPI1DMACNT = 4;
    while (!(SPI1CON & BIT(16)));                   //Wait pending

    for (i = 0; i < 2; i++) {
        manu_id <<= 8;
        manu_id |= spi1_getbyte_usb();
    }
    spi1_cs_dis_usb();
//    printf("id:%x\n",manu_id);
    if((manu_id >> 8) == 0xef || (manu_id >> 8) == 0x85){

    }else{
        printf("%s:id = 0x%x\n",__func__,manu_id);
        manu_id = 0;
    }
    //printf("id:%x\n",manu_id);
    return manu_id;
}

AT(.usbdev.spiflash1_drv)
u32 get_spi_flash_capacity(void)
{
    u32 num_sectors = 0;

    switch(spiflash1_manu_id_read()){
    case W25Q80:
        num_sectors = W25Q80_SEC;
        break;

    case W25Q16:
        num_sectors = W25Q16_SEC;
        break;

    case W25Q32:
        num_sectors = W25Q32_SEC;
        break;

    case W25Q64:
        num_sectors = W25Q64_SEC;
        break;

    case W25Q128:
        num_sectors = W25Q128_SEC;
        break;
    case P25Q16SH:
        num_sectors = P25Q16SH_SEC;
        break;
    case P25Q64SH:
        num_sectors = P25Q64SH_SEC;
        break;

    default:
        break;
    }

    return num_sectors;
}

AT(.usbdev.spiflash1_drv)
bool spiflash1_virtual_usb_en(void)
{
    return UDE_STOAGE_FLASH_EN;
}

AT(.usbdev.spiflash1_drv)
bool spiflash_disk_read(void *buf, u32 lba)
{
    spiflash1_read_dma(buf,lba*512,512);
    return true;
}

AT(.usbdev.spiflash1_drv)
bool spiflash_disk_write(void *buf, u32 lba)
{
    spiflash1_write_dma(buf,lba*512,512);
    return true;
}


void spi1_cs_init_usb(void)
{
    GPIOAFEN &= ~BIT(5);
    GPIOADE |= BIT(5);
    GPIOADIR &= ~BIT(5);
    GPIOASET = BIT(5);
}


void spiflash1_init_usb(void)
{
    //SPI1_G1: CLK_PA3, DIO_PA4,   //CS_PA5
    GPIOAFEN |= BIT(3)|BIT(4);
    GPIOADE |=  BIT(3)|BIT(4);
    GPIOADIR &= ~(BIT(3) | BIT(4));  //CLK,DO output
    GPIOADIR |= BIT(4);  //DI input
    GPIOAPU |= BIT(4);   //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = (0x01<<12);  //G1

    spi1_cs_init_usb();
    u8 sys_clk_s = get_cur_sysclk();
    u32 sys_clk_spi[] = {2,26,48,52,80,120,160};
    printf("cur_clk:%dM\n",sys_clk_spi[sys_clk_s]);
    SPI1BAUD = sys_clk_spi[sys_clk_s]/SPI_BAUD_USB - 1;   //15M.
    SPI1CON =  0x01 | (1<<2);
#if FLASH_GREATER_THAN_16MBYTE
    delay_5ms(10);
    enter_4byte_addr();
    delay_5ms(10);
#endif
}

bool spiflash_disk_init(void)
{
#if 0  //erase chip test
    spiflash1_write_enable_usb();
    spi1_cs_en_usb();
    spi1_sendbyte_usb(0xc7);
    spi1_cs_dis_usb();
    spiflash1_waitbusy_usb();
    my_printf("ERASE CHIP FININSH\r\n");
#endif
    spiflash1_init_usb();
    return true;
}

#endif // UDE_STOAGE_FLASH_EN
