#include "include.h"
#include "func.h"

#if (GUI_SELECT != GUI_NO)
AT(.text.func.aux.display)
void func_tom_cat_display(void)
{
    gui_box_process();

    switch (box_cb.disp_sta) {
    case GUI_BOX_NULL:
        break;
    default:
        gui_box_display();
        break;
    }
}


#endif
