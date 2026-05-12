#include "include.h"
#include "func.h"
#include "func_i2s.h"

#if FUNC_I2S_EN
AT(.text.func.i2s.msg)
void func_i2s_message(u16 msg)
{
    switch (msg) {
        case KU_PLAY:
        case KU_PLAY_POWER:
        case KU_PLAY_MODE:
            bsp_clr_mute_sta();
            i2s_play_pause();
            break;

        default:
            func_message(msg);
            break;
    }
}
#endif // FUNC_I2S_EN
