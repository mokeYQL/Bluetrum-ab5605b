#include "include.h"
#include "func.h"

#if FUNC_I2S_EN
//I2S接收模式
f_i2s_t f_i2s;

static void func_i2s_process(void)
{
    func_process();
}

static void func_i2s_enter(void)
{
    memset(&f_i2s, 0, sizeof(f_i2s_t));

    func_i2s_enter_display();

    bsp_i2s_init();

    i2s_start();
}

static void func_i2s_exit(void)
{
    func_i2s_exit_display();
    bsp_i2s_exit();
    func_cb.last = FUNC_I2S;
}

AT(.text.func.i2s)
void func_i2s(void)
{
    printf("%s\n", __func__);

    func_i2s_enter();

    while (func_cb.sta == FUNC_I2S) {
        func_i2s_process();
        func_i2s_message(msg_dequeue());
        func_i2s_display();
    }

    func_i2s_exit();
}

#endif
