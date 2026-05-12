#ifndef _BSP_HUART_AUDIO_H
#define _BSP_HUART_AUDIO_H

#if HUART_AUDIO_EN

//enum {
//    HUART_TR_PA7    = 0,
//    HUART_TR_PB2,
//    HUART_TR_PB3,
//    HUART_TR_PE7,
//    HUART_TR_PA1,
//    HUART_TR_PA6,
//    HUART_TR_PB1,
//    HUART_TR_PB4,
//    HUART_TR_PE6,
//    HUART_TR_PA0,
//};

typedef void (*huart_callback_t)(u8 *ptr, u32 samples, bool is_32bit);

typedef struct {
    union {
        struct {
            uint8_t  tx_port : 4;
            uint8_t  rx_port : 4;
            uint8_t  rxisr_en : 1;
            uint8_t  txisr_en : 1;
            uint8_t  rxbuf_loop : 1;
            uint8_t  tx_1st : 1;
        };
        uint16_t all_setting;
    };
    uint16_t rxbuf_size;
    uint8_t *rxbuf;
    huart_callback_t callback;
} huart_t;

void bsp_huart_init(huart_t *huart, uint32_t baud_rate);
void bsp_huart_start(void);
void bsp_huart_stop(void);
void bsp_huart_exit(void);
bool huart_get_play_sta(void);
void bsp_huart_set_baudrate(uint baudrate);
void bsp_huart_tx(const void *buf, uint len);
uint bsp_huart_get_rxcnt(void);
char bsp_huart_getchar(void);

#endif

#endif
