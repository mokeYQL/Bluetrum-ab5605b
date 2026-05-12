#include "include.h"

AT(.rodata.func.table)
const u8 func_sort_table[] = {
#if FUNC_MUSIC_EN
    FUNC_MUSIC,
#endif // FUNC_MUSIC_EN

#if FUNC_FMRX_EN
    FUNC_FMRX,
#endif

#if FUNC_BT_EN
    FUNC_BT,
#endif

#if FUNC_BTHID_EN
    FUNC_BTHID,
#endif // FUNC_BTHID

#if FUNC_AUX_EN
    FUNC_AUX,
#endif // FUNC_AUX_EN

#if FUNC_CLOCK_EN
    FUNC_CLOCK,
#endif

#if FUNC_USBDEV_EN
    FUNC_USBDEV,
#endif // FUNC_USBDEV_EN

#if FUNC_SPDIF_EN
    FUNC_SPDIF,
#endif

#if FUNC_HDMI_EN
    FUNC_HDMI,
#endif

#if FUNC_SPEAKER_EN
    FUNC_SPEAKER,
#endif // FUNC_SPEAKER_EN

#if EX_SPIFLASH_SUPPORT && !FUNC_TOM_CAT_EN
    FUNC_EXSPIFLASH_MUSIC,
#endif

#if FUNC_TOM_CAT_EN
    FUNC_TOM_CAT,
#endif

#if FUNC_I2S_EN
    FUNC_I2S,
#endif

#if FUNC_HUART_EN
    FUNC_HUART,
#endif

#if FUNC_IDLE_EN
    FUNC_IDLE,
#endif
};

AT(.text.func)
u8 get_funcs_total(void)
{
    return sizeof(func_sort_table);
}

#if BT_POWER_OPTIMIZE
u8 getcfg_vddif_vset(void){
    return 1;
}
u8 getcfg_vddrf_vset(void){
    return 0;
}
u8 getcfg_mix_vcmsel(void){
    return 2;
}
AT(.text.dac)
u8 getcfg_aud_vrefps(void){
    return  0;
}
u32 getcfg_vddbt_sel(void){
    return 8;     //VDDBT 2.2V
}

u32 getcfg_vddio_sel(void){
   if(FUNC_BT == func_cb.sta) {
       return 3;   //vddio 2.7V
   } else {
       return xcfg_cb.vddio_sel;
   }
}
#else
u32 getcfg_vddbt_sel(void)
{
    return xcfg_cb.vddbt_sel;
}

u32 getcfg_vddio_sel(void)
{
    return xcfg_cb.vddio_sel;
}
#endif



u32 getcfg_vddbt_capless_en(void)
{
    return xcfg_cb.vddbt_capless_en;
}

AT(.com_text.mictrim)
u32 getcfg_mic_bias_method(void)
{
    return xcfg_cb.mic_bias_method;
}

u32 getcfg_mic_bias_resistor(void)
{
    return xcfg_cb.mic_bias_resistor;
}

u8 is_tkey_mul_en(void)
{
    return USER_TKEY_MUL_EN;
}

#if (USER_TKEY_MUL_EN && !TKEY_CH0_EN)
bool tkey_mul_release_ch0(void)
{
    printf("tkey_mul_release_ch0\n");
    return true;
}
#endif
