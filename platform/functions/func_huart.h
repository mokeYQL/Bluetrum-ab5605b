#ifndef _FUNC_HUART_H
#define _FUNC_HUART_H

void func_huart_message(u16 msg);

#if (GUI_SELECT != GUI_NO)
void func_huart_display(void);
void func_huart_enter_display(void);
void func_huart_exit_display(void);
#else
#define func_huart_display()
#define func_huart_enter_display()
#define func_huart_exit_display()
#endif

#endif
