#include "include.h"
#include "func.h"
#include "func_usbdev.h"

void uda_set_by_pass(u8 bypass);

f_ude_t f_ude;

u8 usbmic_volume_en = UDE_MIC_VOLUME_EN;
u8 usbmic_compli_xp_sys = USB_MIC_COMPATIBILITY_XP_SYS;

#if FUNC_USBDEV_EN

/*
AT(.usbdev.com.table)
//usb device descriptor
const u8 desc_usb_device[18] = {
    18,                 // Num bytes of the descriptor
    1,                  // Device Descriptor type
    0x10,               // Revision of USB Spec. LSB (in BCD)
    0x01,               // Revision of USB Spec. MSB (in BCD)
    0,                  // Mass Storage Class
    0,                  // General Mass Storage Sub Class
    0,                  // Do not use Class specific protocol
    USB_CTRL_SIZE,      // Max packet size of Endpoint 0

    //jl:e3b4
    0x87,               // Vendor ID LSB
    0x80,               // Vendor ID MSB
    0x24,               // Product ID LSB
    0x10,               // Product ID MSB
    0x00,               // Device Revision LSB (in BCD)
    0x01,               // Device Revision MSB (in BCD)

    STR_MANUFACTURER,   // Index of Manufacture string descriptor
    STR_PRODUCT,        // Index of Product string descriptor
    STR_SERIAL_NUM,     // Index of Serial No. string descriptor
    1                   // Num Configurations, Must be 1
};

//修改usb_dev在电脑上的名称
AT(.usbdev.com.table)
const u8 str_product[28] = {
    28,                 // Num bytes of this descriptor   //str_product总长度28,名字长度有变时，需要同时改变这个值
    3,                  // String descriptor  //此值保持不变
    'T',    0,          //以下是可以随便修改的名字  //unicode,2byte一个字符  //这里名字示例为26byte
    'E',    0,
    'S',    0,
    'T',    0,
    '.',    0,
    '0',    0,
    ' ',    0,
    'D',    0,
    'e',    0,
    'v',    0,
    'i',    0,
    'c',    0,
    'e',    0
};
*/

/*
#define HID_CONFIG_SIZE 32
#define HID_REPORT_SIZE 33
AT(.usbdev.com.table)
const u8 desc_config_hid[HID_CONFIG_SIZE] = {
    //Interface Descriptor:
    9,                  // Num bytes of this descriptor
    4,                  // Interface descriptor type
    4,                  // Interface Number
    0,                  // Alternate interface number
    2,                  // Num endpoints of this interface
    0x03,               // Interface Sub Class: Human Interface Device
    0,                  // Interface Sub Class:
    0,                  // Class specific protocol:
    0,                  // Index of Interface string descriptor

    //HID Descriptor:
    9,                  // Num bytes of this descriptor
    0x21,               // HID Type
    0x01, 0x02,         // HID Class Specification compliance
    0x00,               // Country Code: none
    0x01,               // Number of descriptors to follow
    0x22,               // Report descriptor type
    HID_REPORT_SIZE, 0, // Len of Report descriptor ,sizeof(desc_hid_report)

    //EndpointDescriptor:
    7,                  // Num bytes of this descriptor
    5,                  // Endpoint descriptor type
    3,                  // Endpoint number: HID OUT
    0x03,               // Interrupt Endpoint
    64, 0x00,           // Maximum packet size
    0x01,               // Poll every 1 msec seconds

    //EndpointDescriptor:
    7,                  // Num bytes of this descriptor
    5,                  // Endpoint descriptor type
    3 | 0x80,           // Endpoint number: HID IN
    0x03,               // Interrupt Endpoint
    64, 0x00,           // Maximum packet size
    0x01,               // Poll every 1 msec seconds
};

AT(.usbdev.com.table)
const u8 desc_hid_report[HID_REPORT_SIZE] = {
    0x05, 0x0c,         // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,         // USAGE (Consumer Control)
    0xa1, 0x01,         // COLLECTION (Application)

    0x15, 0x00,         // LOGICAL_MINIMUM (0)
    0x25, 0x01,         // LOGICAL_MAXIMUM (1)

    0x09, 0xb3,         // USAGE (fast forward)
    0x09, 0xb5,         // USAGE (Scan Next Track)
    0x09, 0xb6,         // USAGE (Scan Previous Track)
    0x09, 0xb7,         // USAGE (stop)
    0x09, 0xcd,         // USAGE (Play/Pause)
    0x09, 0xe2,         // USAGE (Mute)
    0x09, 0xe9,         // USAGE (Volume Up)
    0x09, 0xea,         // USAGE (Volume Down)

    0x75, 0x01,         // REPORT_SIZE (1)
    0x95, 0x08,         // REPORT_COUNT (8)
    0x81, 0x42,         // INPUT (Data,Var,Abs)
    0xc0,               // END_COLLECTION
};

AT(.usbdev.com)
u8 get_desc_config_hid_size(void){
    return HID_CONFIG_SIZE;
}

AT(.usbdev.com)
u8 get_desc_hid_report_size(void){
    return HID_REPORT_SIZE;
}
*/

AT(.usbdev.com)
void AB_set_udisk_time(u8 *get_time_buf, u8 len)
{
    /*
    使用自家小工具更新时间：
    get_time_buf 时间数据
    格式：
    buf[0] 年
    buf[1] 月
    buf[2] 日
    buf[3] 时
    buf[4] 分
    buf[5] 秒
    */
    u8 *buf = get_time_buf;

    //十六进制原始数据
    print_r(buf, 6);

    //转成十进制整形数据
    for(u8 i = 0; i < 6; i++) {
        my_printf("%d ", (buf[i]>>4)*10 + (buf[i]&0x0F));
    }
}

///hid get buf
AT(.text.func.usbdev)
void ude_hid_get_data(u8 *data,u8 len) {
//     my_print_r(data,len);
}

#if UDE_MIC_KARAOK_EN
uint8_t usbdev_karaok_en = 0;
#endif

AT(.text.func.usbdev)
void ude_mic_start(void)
{
#if UDE_MIC_KARAOK_EN
    usbdev_karaok_en = 1;
#else
    audio_path_init(AUDIO_PATH_USBMIC);
    audio_path_start(AUDIO_PATH_USBMIC);
    dac_spr_set(SPR_48000);
#endif
}

AT(.text.func.usbdev)
void ude_mic_stop(void)
{
#if UDE_MIC_KARAOK_EN
    usbdev_karaok_en = 0;
#else
    audio_path_exit(AUDIO_PATH_USBMIC);
#endif
}

AT(.usbdev.com)
u8 ude_get_sys_volume(void)
{
    return sys_cb.vol;
}

AT(.usbdev.com)
u8 ude_get_sys_vol_level(void)
{
    return 256 / VOL_MAX;
}

AT(.usbdev.com)
void ude_set_sys_volume(u8 vol)
{
    if (sys_cb.vol != vol) {
        sys_cb.vol = vol;
        msg_enqueue(EVT_UDE_SET_VOL);
    }
}

#if UDE_STORAGE_EN
AT(.text.func.usbdev)
void ude_sdcard_switch(u8 dev)
{
//    printf("%s, %d, %d\n", __func__, f_ude.cur_dev, dev);
    if (f_ude.cur_dev != dev) {
        ude_sd_remove();
        f_ude.cur_dev = dev;
        fsdisk_callback_init(f_ude.cur_dev);
        f_ude.dev_change = 1;
    } else {
        sd0_init();
    }
}

AT(.text.func.usbdev)
void ude_sdcard_change_process(void)
{
    static u8 cnt_s = 0;
    if (f_ude.dev_change) {
        cnt_s++;
        if (cnt_s >= 5) {
            f_ude.dev_change = 0;
            sd0_init();
        }
    } else {
        cnt_s = 0;
    }
}
#endif // UDE_STORAGE_EN

AT(.text.func.usbdev)
void func_usbdev_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }

    uda_set_by_pass(1);

    DACDIGCON0 &= ~BIT(6);
    adpll_spr_set(DAC_OUT_SPR);

    dac_fade_out();
    dac_fade_wait();

    bsp_change_volume(WARNING_VOLUME);
    bsp_piano_warning_play(WARNING_TONE, TONE_MAX_VOL);
    dac_fade_out();
    dac_fade_wait();
    bsp_change_volume(sys_cb.vol);

    dac_fade_in();

    adpll_spr_set(1);
    dac_spr_set(0);                 //samplerate 48K
    DACDIGCON0 |= BIT(6);           //Src0 Sample Rate Synchronization Enable

    uda_set_by_pass(0);
}

AT(.text.func.usbdev)
void func_usbdev_process(void)
{
    func_process();
    usb_device_process();
    f_ude.rw_sta = sd0_get_rw_sta();
}

static void func_usbdev_enter(void)
{
    f_ude.vol = sys_cb.vol;         //restore system volume
    if (!dev_is_online(DEV_USBPC)) {
        func_cb.sta = FUNC_NULL;
        return;
    }

    func_cb.mp3_res_play = func_usbdev_mp3_res_play;
    f_ude.cur_dev = DEV_SDCARD;
    f_ude.dev_change = 0;
    func_usbdev_enter_display();
    led_idle();
#if WARNING_FUNC_USBDEV
    mp3_res_play(RES_BUF_PC_MODE_MP3, RES_LEN_PC_MODE_MP3);
#endif // WARNING_FUNC_USBDEV

#if UDE_SPEAKER_EN
    adpll_spr_set(1);
    dac_spr_set(0);                 //samplerate 48K
    DACDIGCON0 |= BIT(6);           //Src0 Sample Rate Synchronization Enable
    sys_cb.vol = VOL_MAX;
    bsp_change_volume(sys_cb.vol);
    dac_fade_in();
#endif // UDE_SPEAKER_EN

#if UDE_STORAGE_EN

#if UDE_STOAGE_FLASH_EN
    if(spiflash1_manu_id_read()){
        sys_cb.cur_dev = DEV_SPIFLASH;
        fsdisk_callback_init(f_ude.cur_dev);
    } else
#endif // UDE_STOAGE_FLASH_EN

    if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1)) {
        if (dev_is_online(DEV_SDCARD)) {
            f_ude.cur_dev = DEV_SDCARD;
        } else if (dev_is_online(DEV_SDCARD1)) {
            f_ude.cur_dev = DEV_SDCARD1;
        }
        fsdisk_callback_init(f_ude.cur_dev);
        fs_mount();
    }
#endif // UDE_STORAGE_EN
    usb_device_enter(UDE_ENUM_TYPE);

#if MICAUX_ANALOG_OUT_ALWAYS
    micaux_analog_out_init();
#endif // MICAUX_ANALOG_OUT_ALWAYS
}

static void func_usbdev_exit(void)
{
    usb_device_exit();
    func_usbdev_exit_display();
#if UDE_SPEAKER_EN
    DACDIGCON0 &= ~BIT(6);
    adpll_spr_set(DAC_OUT_SPR);
    sys_cb.vol = f_ude.vol;         //recover system volume
    bsp_change_volume(sys_cb.vol);
    dac_fade_out();
#endif // UDE_SPEAKER_EN
#if UDE_STORAGE_EN
    if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1)) {
        sd0_stop(1);
    #if I2C_MUX_SD_EN
        sd0_go_idle_state();
    #endif
    }
#endif // UDE_STORAGE_EN
    func_cb.last = FUNC_USBDEV;
    func_cb.sta  = FUNC_IDLE;
}

AT(.text.func.usbdev)
void func_usbdev(void)
{
    printf("%s\n", __func__);

    func_usbdev_enter();

    while (func_cb.sta == FUNC_USBDEV) {
        func_usbdev_process();
        func_usbdev_message(msg_dequeue());
        func_usbdev_display();
    }

    func_usbdev_exit();
}
#else
AT(.text.func.usbdev)
void ude_mic_start(void){}
AT(.text.func.usbdev)
void ude_mic_stop(void){}
AT(.usbdev.com)
u8 ude_get_sys_volume(void){return 0;}
AT(.usbdev.com)
u8 ude_get_sys_vol_level(void){return 0;}
AT(.usbdev.com)
void ude_set_sys_volume(u8 vol){}
#endif // FUNC_USBDEV_EN
