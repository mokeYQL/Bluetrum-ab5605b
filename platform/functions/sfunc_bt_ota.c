#include "include.h"
#include "func.h"
#include "func_bt.h"

#if FUNC_BT_EN

AT(.text.func.btring)
void sfunc_bt_ota(void)
{
    printf("%s\n", __func__);
    bt_audio_bypass();
    led_off();
    rled_off();
    ota_enter();
    while (bt_get_status() == BT_STA_OTA) {
        delay_5ms(4);
    }
    ota_exit();
}

#endif //FUNC_BT_EN
