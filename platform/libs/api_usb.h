#ifndef _API_USB_H
#define _API_USB_H

enum {
    USBCHK_ONLY_HOST,
    USBCHK_ONLY_DEVICE,
    USBCHK_OTG,
};

enum {
    USB_DISCONNECT,
    USB_UDISK_CONNECTED,
    USB_PC_CONNECTED,
    USB_DP_GND_CONNECTED,           //假USB设备
    USB_DPDM_SC_CONNECTED,          //Short-Circuit
};

void usb_init(void);
void usb_disable(void);
u8 usbchk_connect(u8 mode);
void udisk_remove(void);
void udisk_insert(void);
void udisk_invalid(void);

void udisk_suspend(void);
void udisk_resume(void);

void usb_device_enter(u8 enum_type);
void usb_device_exit(void);
void usb_device_process(void);
void ude_tmr_isr(void);
void pc_remove(void);
bool ude_hid_key_send(u16 hid_val, u16 release_time);        //发送一个HID COMSUMER按键
bool ude_hid_send_data(u8 *buf,u8 len);                      //发送HID报告
#endif // _API_USB_H

