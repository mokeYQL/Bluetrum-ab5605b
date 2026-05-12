#ifndef _FUNC_I2S_H
#define _FUNC_I2S_H

typedef struct {
    u8  disp_update :   1;
} f_i2s_t;
extern f_i2s_t f_i2s;

void func_i2s_message(u16 msg);
void i2s_start(void);
void i2s_stop(void);
u8 i2s_get_play_sta(void);
void i2s_play_pause(void);

#if (GUI_SELECT != GUI_NO)
void func_i2s_display(void);
void func_i2s_enter_display(void);
void func_i2s_exit_display(void);
#else
#define func_i2s_display()
#define func_i2s_enter_display()
#define func_i2s_exit_display()
#endif

#endif // _FUNC_I2S_H
