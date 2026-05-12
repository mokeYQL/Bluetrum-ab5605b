#include "include.h"
#include "api.h"


#if !BT_A2DP_RECON_EN
void a2dp_recon_hook(void)
{
}
#endif

uint a2dp_get_vol(void)
{
    uint vol;

    vol = ((u32)sys_cb.vol * 1280L /VOL_MAX) /10;
    if(vol > 0x7f) {
        vol = 0x7f;
    }

    return vol;
}
#if SYS_KARAOK_EN
void a2dp_set_vol(u8 a2dp_vol)
{
    u16 vol;
    vol = 0x7FFF / VOL_MAX * a2dp_vol;
    if(vol > 0x7FFF || a2dp_vol == VOL_MAX) {
        vol = 0x7FFF;
    }
    SRC0VOLCON = vol | (0x02 << 16);
}
#endif
