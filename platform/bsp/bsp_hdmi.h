#ifndef __BSP_HDMI_H
#define __BSP_HDMI_H

#include "api_hdmi.h"

typedef enum{
    CECCTR_PRESSED_POWER    	    = 0x40,
	CECCTR_PRESSED_VOLUME_UP	    = 0x41,
	CECCTR_PRESSED_VOLUME_DOWN	    = 0x42,
	CECCTR_PRESSED_MUTE			    = 0x43,
	CECCTR_PRESSED_PLAY			    = 0x43,
	CECCTR_PRESSED_STOP			    = 0x43,
	CECCTR_PRESSED_PAUSE		    = 0x43,
} CEC_CONTROL_CMD;

typedef enum
{
	CECMSG_FEATURE_ABORT                 = 0x00,
	CECMSG_IMAGE_VIEW_ON                 = 0x04,
	CECMSG_TUNER_STEP_INCREMENT          = 0x05,
	CECMSG_TUNER_STEP_DECREMENT          = 0x06,
	CECMSG_TUNER_DEVICE_STATUS           = 0x07,
	CECMSG_GIVE_TUNER_DEVICE_STATUS      = 0x08,
	CECMSG_RECORD_ON                     = 0x09,
	CECMSG_RECORD_STATUS                 = 0x0A,
	CECMSG_RECORD_OFF                    = 0x0B,
	CECMSG_TEXT_VIEW_ON                  = 0x0D,
	CECMSG_RECORD_TV_SCREEN              = 0x0F,
	CECMSG_GIVE_DECK_STATUS              = 0x1A,
	CECMSG_DECK_STATUS                   = 0x1B,
	CECMSG_SET_MENU_LANGUAGE             = 0x32,
	CECMSG_CLEAR_ANALOGUE_TIMER          = 0x33,
	CECMSG_SET_ANALOGUE_TIMER            = 0x34,
	CECMSG_TIMER_STATUS                  = 0x35,
	CECMSG_STANDBY                       = 0x36,
	CECMSG_PLAY                          = 0x41,
	CECMSG_DECK_CONTROL                  = 0x42,
	CECMSG_TIMER_CLEARED_STATUS          = 0x43,
	CECMSG_USER_CONTROL_PRESSED          = 0x44,
	CECMSG_USER_CONTROL_RELEASED         = 0x45,
	CECMSG_GIVE_OSD_NAME                 = 0x46,
	CECMSG_SET_OSD_NAME                  = 0x47,
	CECMSG_SET_OSD_STRING                = 0x64,
	CECMSG_SET_TIMER_PROGRAM_TITLE       = 0x67,
	CECMSG_SYSTEM_AUDIO_MODE_REQUEST     = 0x70,
	CECMSG_GIVE_AUDIO_STATUS             = 0x71,
	CECMSG_SET_SYSTEM_AUDIO_MODE         = 0x72,
	CECMSG_REPORT_AUDIO_STATUS           = 0x7A,
	CECMSG_GIVE_SYSTEM_AUDIO_MODE_STATUS = 0x7D,
	CECMSG_SYSTEM_AUDIO_MODE_STATUS      = 0x7E,
	CECMSG_ROUTING_CHANGE                = 0x80,
	CECMSG_ROUTING_INFORMATION           = 0x81,
	CECMSG_ACTIVE_SOURCE                 = 0x82,
	CECMSG_GIVE_PHYSICAL_ADDRESS         = 0x83,
	CECMSG_REPORT_PHYSICAL_ADDRESS       = 0x84,
	CECMSG_REQUEST_ACTIVE_SOURCE         = 0x85,
	CECMSG_SET_STREAM_PATH               = 0x86,
	CECMSG_DEVICE_VENDOR_ID              = 0x87,
	CECMSG_VENDOR_COMMAND                = 0x89,
	CECMSG_VENDOR_REMOTE_BUTTON_DOWN     = 0x8A,
	CECMSG_VENDOR_REMOTE_BUTTON_UP       = 0x8B,
	CECMSG_GIVE_DEVICE_VENDOR_ID         = 0x8C,
	CECMSG_MENU_REQUEST                  = 0x8D,
	CECMSG_MENU_STATUS                   = 0x8E,
	CECMSG_GIVE_DEVICE_POWER_STATUS      = 0x8F,
	CECMSG_REPORT_POWER_STATUS           = 0x90,
	CECMSG_GET_MENU_LANGUAGE             = 0x91,
	CECMSG_SELECT_ANALOGUE_SERVICE       = 0x92,
	CECMSG_SELECT_DIGITAL_SERVICE        = 0x93,
	CECMSG_SET_DIGITAL_TIMER             = 0x97,
	CECMSG_CLEAR_DIGITAL_TIMER           = 0x99,
	CECMSG_SET_AUDIO_RATE                = 0x9A,
	CECMSG_INACTIVE_SOURCE               = 0x9D,
	CECMSG_CEC_VERSION                   = 0x9E,
	CECMSG_GET_CEC_VERSION               = 0x9F,
	CECMSG_VENDOR_COMMAND_WITH_ID        = 0xA0,
	CECMSG_CLEAR_EXTERNAL_TIMER          = 0xA1,
	CECMSG_SET_EXTERNAL_TIMER            = 0xA2,
	CECMSG_REPORT_SHORT_AUDIO            = 0xA3,
	CECMSG_REQUEST_SHORT_AUDIO    	     = 0xA4,
	CECMSG_ARC_INITIATE                  = 0xC0,
	CECMSG_ARC_REPORT_INITIATED          = 0xC1,
	CECMSG_ARC_REPORT_TERMINATED         = 0xC2,
	CECMSG_ARC_REQUEST_INITIATION        = 0xC3,
	CECMSG_ARC_REQUEST_TERMINATION       = 0xC4,
	CECMSG_ARC_TERMINATE                 = 0xC5,
	CECMSG_CDC_HEADER                    = 0xF8,
	CECMSG_ABORT                         = 0xFF,
} CEC_MSG_T;

void hdmi_detect_io_init(void);
void hdmi_detect(void);
bool hdmi_get_physical_address(void);
void bsp_hdmi_cec_init(void);
void bsp_hdmi_cec_exit(void);
u8   bsp_get_hdmi_spdif_ch(void);
u8   cec_tx_test(void);
void cec_frame_process(void);

#endif
