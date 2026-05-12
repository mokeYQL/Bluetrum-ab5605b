#ifndef __APP_FOTA_H
#define __APP_FOTA_H

void bsp_fot_init(void);
void fot_recv_proc(u8 *buf, u16 len);
void bsp_fot_process(void);
void fot_update_pause(void);
void fot_update_continue(void);
u8 is_fot_start(void);
bool bsp_fot_is_connect(void);
u8 fot_app_connect_auth(uint8_t *packet, uint16_t size);
void fot_spp_connect_callback(void);
void fot_spp_disconnect_callback(void);
void fot_ble_disconnect_callback(void);
void fot_ble_connect_callback(void);

#endif // __APP_FOTA_H
