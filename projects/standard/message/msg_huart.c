#include "include.h"
#include "func.h"
#include "func_huart.h"

#if FUNC_HUART_EN
AT(.text.func.huart.msg)
void func_huart_message(u16 msg)
{
    switch (msg) {
        case KU_PLAY:
        case KU_PLAY_POWER:
        case KU_PLAY_MODE:

            break;

        default:
            func_message(msg);
            break;
    }
}
#endif
