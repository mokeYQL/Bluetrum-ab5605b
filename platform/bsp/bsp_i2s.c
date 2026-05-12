#include "include.h"
#include "bsp_i2s_ta5711.h"

#if I2S_EN
//   IIS_G1 = 0,  //MCLK_PA3, BCLK_PA5, LRC_PA4, DO_PA6, //DI_PA7
//   IIS_G2 = 1,  //MCLK_PE0, BCLK_PE6, LRC_PE7, DO_PE5, //DI_PB0

#define I2S_DMA_BUF_LEN         512
#define I2S_DMA_BUF_INDEX       5
typedef struct {
    volatile u8 wptr;
    volatile u8 rptr;
    u16 dma_cnt;
    u32 dma_in0;
    u32 dma_in1;
    u32 dma_out0;
    u32 dma_out1;
    u8 (*buf)[I2S_DMA_BUF_LEN];
}i2s_dma_t;

typedef struct {
    i2s_dma_t *i2s_dma;
    u16 mapping      : 1;
    u16 mode         : 1;
    u16 bit_mode     : 1;
    u16 data_mode    : 1;
    u16 dma_en       : 1;
    u16 mclk_en      : 1;
    u16 mclk_sel     : 2;
    u16 pcm_mode_en  : 1;
    u16 spr          : 1;       //slave输入的采样率
    u16 mode_ext     : 2;
}i2s_cfg_t;

void i2s_init(void *cfg);
void i2s_exit(void);

#if I2S_DMA_EN
i2s_dma_t i2s_dma;
#endif

i2s_cfg_t i2s_cfg = {
#if I2S_DMA_EN
    .i2s_dma     = &i2s_dma,
#endif
    .mapping     = I2S_MAPPING_SEL,
    .mode        = I2S_MODE_SEL,
    .bit_mode    = I2S_BIT_MODE,
    .data_mode   = I2S_DATA_MODE,
    .dma_en      = I2S_DMA_EN,
    .mclk_en     = I2S_MCLK_EN,
    .mclk_sel    = I2S_MCLK_SEL,
    .pcm_mode_en = I2S_PCM_MODE,
};

void bsp_i2s_init(void)
{
#if I2S_DEVICE == I2S_DEV_TAS5711
    bsp_tas5711_init();
#endif
    i2s_init(&i2s_cfg);
}

void bsp_i2s_exit(void)
{
    i2s_exit();
}
#endif
