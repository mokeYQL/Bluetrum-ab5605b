#ifndef __AUDIO_SOFTEQ_DRC_H
#define __AUDIO_SOFTEQ_DRC_H

extern s32 sed_preeql_cb[10*7 + 6];
extern s32 sed_preeqr_cb[10*7 + 6];
extern u8  sed_drcl_cb[14 * 4];
extern u8  sed_drcr_cb[14 * 4] ;

void audio_softeq_drc_param_init(void);
void softeq_drc_online_adj_init(void);
void softeq_drc_vol_set(s32 vol);

u8* get_sed_adj_rxbuf_addr(void);
u32 get_sed_adj_rxbuf_len(void);
bool sed_huart_rx_done_callback(u8 *rx_buf);
void sed_adj_parse_cmd_equalizer(void);

#endif
