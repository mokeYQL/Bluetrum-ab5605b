#include "config.h"
setid(11111111-1111-1111-1111-111111111111);
setflash(1, FLASH_SIZE, FLASH_ERASE_4K, FLASH_DUAL_READ, FLASH_QUAD_READ);
setspace(0x5000);
setuserbin(0x60000, 0x1B000, voc.bin, 0);
make(dcf_buf, header.bin, app.bin, res.bin, xcfg.bin,voc.bin);
save(dcf_buf, app.dcf);
