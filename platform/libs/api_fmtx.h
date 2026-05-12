#ifndef _API_FMTX_H
#define _API_FMTX_H

enum {
    FMTX_VOL_X1 = 0,        //幅度不放大
    FMTX_VOL_X2,            //幅度放大一倍
    FMTX_VOL_X4,            //幅度放大3倍
};

bool fmtx_init(void *cfg);
void fmtx_set_freq(u16 freq);
void fmtx_set_vol(u16 vol);
void fmtx_exit(void);
void fmtx_dnr_process(void);
void fmtx_vol_amp(u8 num_amp);  //设置FMTX AUDIO放大倍数

#endif // _API_FMTX_H
