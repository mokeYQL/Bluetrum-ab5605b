#include "include.h"

#if USER_TKEY
AT(.rodata.tkey)
const tkey_ch_t tkey0 = {
    .cdpr = CDPR,
    .in_ear = 0,
};

//#if USER_TKEY_INEAR
//AT(.rodata.tkey)
//const tkey_ch_t tkey_inear = {
//    .cdpr = EAR_CDPR,
//    .in_ear = 1,
//};
//#endif

#if USER_TKEY_MUL_EN
scan_tkcb_t scan_tkcb = {            //触摸扫描方法参数
    .tkpress_thd = 30,               //打印容值后, 查看按下稳定后的跳变大小, 该值设置成比跳变值稍小
    .llong_press_times = 300,
    .long_press_times = 60,
    .short_press_times = 15,
    .frq_long_press = 1,
    .cail_tkcnt = 60,                //校准延时扫描次数,刚开机时采的CNT值不准,延时45次扫描再采,避免开机时扫描到按下
    .cail_dwthd_left = 29,
    .cail_dwthd_right = 5,
    .cail_upthd_left = 5,
    .cail_upthd_right = 29,          //建议设成tkpress_thd-1
    .cali_thd = 20,
};
#endif // USER_TKEY_MUL_EN

AT(.rodata.tkey)
const tkey_cfg_t tkey_cfg = {
    .key[0] = NULL,                 //PB1
    .key[1] = NULL,                 //PB2
    .key[2] = NULL,                 //PE6
    .key[3] = NULL,                 //PE7

    //TKTMR Register
    .fil_low = FIL_LOW,
    .fil_high = FIL_HIGH,
    .fil_except = FIL_EXCEPT,
    .fil_val = FIL_VAL,
    .to_except = TO_EXCEPT,

    //TKPTHD Register
    .tkpthd = TKPTHRESH,
    .tkrthd = TKRTHRESH,
    .tkfathd = TKFATHD,

    //TKETHD Register
    .tksthd = SMALLTHD,
    .tklthd = LARGETHD,

    //TETMR Register
    .tkpwup = TKPWUP,
};

AT(.rodata.tkey)
const tkacon_trim_t tkacon_trim = {
    .rtrim  = 0x2,
    .ctrim  = 0x0,
    .itrim  = 0x14,
};

#endif
