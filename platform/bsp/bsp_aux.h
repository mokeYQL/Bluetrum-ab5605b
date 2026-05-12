#ifndef _BSP_AUX_H
#define _BSP_AUX_H

//AUXL3(PF0), AUXR3(PF1)模拟增益列表
#define AUX3_P6DB            0x03       //AUX Analog +6DB
#define AUX3_P5DB            0x02       //AUX Analog +5DB
#define AUX3_P4DB            0x01       //AUX Analog +4DB
#define AUX3_P3DB            0x07       //AUX Analog +3DB
#define AUX3_P2DB            0x06       //AUX Analog +2DB
#define AUX3_P1DB            0x05       //AUX Analog +1DB
#define AUX3_N0DB            0x0B       //AUX Analog 0DB
#define AUX3_N1DB            0x0A       //AUX Analog -1DB
#define AUX3_N2DB            0x09       //AUX Analog -2DB
#define AUX3_N3DB            0x0F       //AUX Analog -3DB
#define AUX3_N4DB            0x0E       //AUX Analog -4DB
#define AUX3_N5DB            0x0D       //AUX Analog -5DB
#define AUX3_N6DB            0x13       //AUX Analog -6DB
#define AUX3_N7DB            0x12       //AUX Analog -7DB
#define AUX3_N8DB            0x11       //AUX Analog -8DB
#define AUX3_N9DB            0x17       //AUX Analog -9DB
#define AUX3_N10DB           0x16       //AUX Analog -10DB
#define AUX3_N11DB           0x15       //AUX Analog -11DB
#define AUX3_N12DB           0x14       //AUX Analog -12DB
#define AUX3_N15DB           0x1B       //AUX Analog -15DB
#define AUX3_N18DB           0x18       //AUX Analog -18DB
#define AUX3_N21DB           0x1F       //AUX Analog -21DB
#define AUX3_N24DB           0x1C       //AUX Analog -24DB
#define AUX3_N27DB           0x23       //AUX Analog -27DB
#define AUX3_N30DB           0x20       //AUX Analog -30DB
#define AUX3_N33DB           0x27       //AUX Analog -33DB
#define AUX3_N36DB           0x24       //AUX Analog -36DB
#define AUX3_N39DB           0x2B       //AUX Analog -39DB
#define AUX3_N42DB           0x28       //AUX Analog -42DB
#define AUX3_MUTE            0x3f

//其它AUX通路模拟增益列表(GC=3,可以兼容AUX3)
#define AUXG_P6DB            0x03       //AUX Analog +6DB
#define AUXG_P3DB            0x07       //AUX Analog +3DB
#define AUXG_N0DB            0x0B       //AUX Analog 0DB
#define AUXG_N3DB            0x0F       //AUX Analog -3DB
#define AUXG_N6DB            0x13       //AUX Analog -6DB
#define AUXG_N9DB            0x17       //AUX Analog -9DB
#define AUXG_N15DB           0x1B       //AUX Analog -15DB
#define AUXG_N21DB           0x1F       //AUX Analog -21DB
#define AUXG_N27DB           0x23       //AUX Analog -27DB
#define AUXG_N33DB           0x27       //AUX Analog -33DB
#define AUXG_N39DB           0x2B       //AUX Analog -39DB
#define AUXG_MUTE            0x3f


void bsp_aux_anl2pa_start(u8 channel, u8 aux_anl_gain);
void bsp_aux_anl2pa_stop(u8 channel);
void bsp_aux_start(u8 channel, u8 aux_anl_gain, u8 aux2adc);
void bsp_aux_stop(u8 channel, u8 aux2adc);
void bsp_aux_piano_start(void);
void bsp_aux_piano_exit(void);
void aux_var_init(void);
void micaux_analog_out_init(void);
void aux_analog_channel_select(u8 channel);
#endif
