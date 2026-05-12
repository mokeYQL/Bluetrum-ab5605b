#include "include.h"
#include "func_hdmi.h"
#if FUNC_HDMI_EN
AT(.text.func.hdmi.msg)
void func_hdmi_message(u16 msg)
{
    switch (msg) {
        case KU_PLAY:
        case KU_PLAY_POWER:
        case KU_PLAY_MODE:
            //cec_tx_test();
            break;

        case EVT_HDMI_REMOVE:
            func_cb.sta = func_cb.last;
            break;

        default:
            func_message(msg);
            break;
    }
}



#endif // FUNC_HDMI_EN


