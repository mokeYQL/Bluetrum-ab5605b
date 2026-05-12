#include "include.h"

#if BT_TWS_LTX_EN
static const uint8_t tws_inquiry_rf_param[11] = {
     5,     //pa_gain
     3,     //mix_gain
    18,     //dig_gain
     5,     //pa_aux_gain
     3,     //pa_cap
     8,     //mix_cap
     5,     //pa_vcasn2
     4,     //pa_vcasn
    15,     //pa_nbias
     7,     //pa_aux_nbias
     4,     //txdbm
};
#endif

const uint8_t *bt_rf_get_inq_param(void)
{
#if BT_TWS_LTX_EN
    return tws_inquiry_rf_param;
#else
    return NULL;
#endif
}

u8 get_chip_package(void)
{
    u8 package = 0;
    if(xcfg_cb.bt_rf_param <= 2) {
        package = 0;   //0 default
    } else if(xcfg_cb.bt_rf_param <= 5) {
         package =  1;   //1 SOP16
    }
    //printf("get_chip_package = %d\n",package);
    return package;
}

const uint8_t *bt_rf_get_param(void)
{
    uint8_t *rf = NULL;
    if(xcfg_cb.bt_user_param_en) {
        printf("bt user param:\n");
        xcfg_cb.rf_pa_aux_nbias = xcfg_cb.rf_pa_nbias/2;
        rf = (uint8_t *)&xcfg_cb.rf_pa_gain;
    } else {
        printf("bt package param = %d\n",xcfg_cb.bt_rf_param);
        rf = bt_get_package_rf_param(xcfg_cb.bt_rf_param);
    }
    if(rf != NULL) {
        printf("rf_param: ");
        for(int i = 0; i < 11; i++) {
            printf("%d ",rf[i]);
        }
        printf("\n");
    }
    return rf;    //使用内部参数
}

//正常启动Main函数
int main(void)
{
    printf("Hello AB560X: %08x\n", (LVDCON & 0x1ff0000));
    bsp_sys_init();
    func_run();
    return 0;
}

//升级完成
void update_complete(int mode)
{
    bsp_update_init();
    if (mode == 0) {
        WDT_DIS();
        while (1);
    }
    WDT_RST();
}
