#ifndef _SPIFLASH_H
#define _SPIFLASH_H

void spiflash1_init(void);
int spiflash1_read(void *buf, u32 addr, u32 len);
void spiflash1_write(void *buf, u32 addr, u32 len);
u32 spiflash1_id_read(void);
void spiflash1_erase(u32 addr);
void spiflash1_erase_block(u32 addr);
bool is_exspiflash_online(void);

#if UDE_STOAGE_FLASH_EN
void spi1_cs_en(void);
void spi1_cs_dis(void);
void spi1_cs_init(void);
u16 spiflash1_manu_id_read(void);
#endif // UDE_STOAGE_FLASH_EN

bool exspiflash_init(void);
void write_music_bin_to_spiflash(void);
void register_spi_read_function(int (* read_func)(void *buf, u32 addr, u32 len));
#endif // _SPIFLASH_H
