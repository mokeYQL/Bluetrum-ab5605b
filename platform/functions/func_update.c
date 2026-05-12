#include "include.h"
#include "func.h"
#include "func_update.h"

#if USB_SD_UPDATE_EN || UART_S_UPDATE ||UART_M_UPDATE

typedef struct _func_upd_t {
    uint sta;
} func_upd_t;

func_upd_t f_upd;

const char upd_filename[13] = UPD_FILENAME;

AT(.text.func.update)
void func_update(void)
{
    int res;
    res = updatefile_init(upd_filename);
    if (res == -1) {
        return;
    }
    if (res == 0) {
#if WARNING_UPDATE_DONE
        mp3_res_play(RES_BUF_UPDATE_DONE_MP3, RES_LEN_UPDATE_MP3);
#endif
        return;
    }
#if WARNING_UPDATE_DONE
    mp3_res_play(RES_BUF_UPDATE_MP3, RES_LEN_UPDATE_MP3);
#endif

#if SYS_KARAOK_EN
    bsp_karaok_exit(AUDIO_PATH_KARAOK);
#endif
#if UART_S_UPDATE
    u8 get_uart_upd_step(void);
    if(get_uart_upd_step()){
        bt_disconnect(0);
        bt_off();
        TMR1CON = 0;
        TMR4CON = 0;
        IRRXCON = 0;
    }
#endif
    updateproc();                               //升级
    while (1);
}
#endif  //USB_SD_UPDATE_EN

#if UART_S_UPDATE
u8 get_uart_upd_step(void);
void uart_upd_param_init(void);
void set_uart_upd_step(u8 step);
void uart_s_update(void)
{
    if(get_uart_upd_step()){
         delay_ms(20);//发送完start响应之后，延时一段时间等待主机进入升级模式。防止主机接收不到cmd
         uart_upd_param_init();
         func_update();                                  //尝试升级
         set_uart_upd_step(0);
    }
}
#if (!UART0_CLK_SEL_X26M) && (UPDATE_UART_SEL == UPDATE_UART0)
void update_uart0baud_in_sysclk_set(u32 uart_baud) {};
#endif
#endif
