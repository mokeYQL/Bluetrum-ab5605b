#include "include.h"
#include "api.h"

extern const uint8_t   sdp_spp_service_record[];
extern const uint8_t   sdp_hfp_service_record[];
extern const uint8_t   sdp_hsp_service_record[];
extern const uint8_t   sdp_avdtp_service_record[];      //a2dp1.3
extern const uint8_t   sdp_avdtp12_service_record[];    //a2dp1.2
extern const uint8_t   sdp_avctp_controller_service_record[];
extern const uint8_t   sdp_avctp_target_category1_service_record[];
extern const uint8_t   sdp_avctp_target_category2_service_record[];
extern const uint8_t   sdp_hid_keyboard_service_record[];
extern const uint8_t   sdp_hid_consumer_service_record[];
extern const uint8_t   sdp_hid_tscreen_service_record[];
extern const uint8_t   sdp_hid_tscreen_ext_service_record[];
extern const uint8_t   sdp_devic_id_service_record[];
extern const uint8_t   sdp_hid_dy_pnp_service_record[];
extern const uint8_t   sdp_pnp_service_record[];
extern const uint8_t   sdp_hid_simple_keyboard_service_record[];
extern const uint8_t   sdp_hid_douyin_service_record[];
extern const uint8_t   sdp_hid_car_tscreen_service_record[];

extern const uint8_t   sdp_spp_custom_service_record[];
extern const uint8_t   sdp_spp_carplay_service_record[];

extern bool bt_hid_simple_keyboard_is_en(void);

typedef struct {
    void    *item;
    uint32_t service_record_handle;
    uint8_t *service_record;
} service_record_item_t;


#if BT_SPP_EN
service_record_item_t spp_sdp_record_item = {
    .service_record_handle  = 0x00010001,
    .service_record         = (uint8_t *)sdp_spp_service_record,
};
#endif


#if BT_HFP_EN
service_record_item_t hfp_sdp_record_item = {
    .service_record_handle  = 0x00010002,
    .service_record         = (uint8_t *)sdp_hfp_service_record,
};
#endif

#if BT_HSP_EN
service_record_item_t hsp_sdp_record_item = {
    .service_record_handle  = 0x00010008,
    .service_record         = (uint8_t *)sdp_hsp_service_record,
};
#endif

#if BT_A2DP_EN
service_record_item_t avdtp_sdp_record_item = {
    .service_record_handle  = 0x00010003,
    .service_record         = (uint8_t *)sdp_avdtp_service_record,
};

service_record_item_t avctp_ct_sdp_record_item = {
    .service_record_handle  = 0x00010004,
    .service_record         = (uint8_t *)sdp_avctp_controller_service_record,
};

service_record_item_t avctp_tg_sdp_record_item = {
    .service_record_handle  = 0x00010005,
#if BT_A2DP_VOL_CTRL_EN
    .service_record         = (uint8_t *)sdp_avctp_target_category2_service_record,
#else
    .service_record         = (uint8_t *)sdp_avctp_target_category1_service_record,
#endif
};
#endif // BT_A2DP_EN


#if BT_HID_EN
service_record_item_t hid_sdp_record_item = {
    .service_record_handle  = 0x00010006,
#if (BT_HID_TYPE == HID_TYPE_PHOTO1)
    .service_record         = (uint8_t *)sdp_hid_consumer_service_record,
#elif (BT_HID_TYPE == HID_TYPE_PHOTO2)
    .service_record         = (uint8_t *)sdp_hid_keyboard_service_record,
#elif (BT_HID_TYPE == HID_TYPE_GAMEPAD)
    .service_record         = (uint8_t *)sdp_hid_tscreen_service_record,
#elif (BT_HID_TYPE == HID_TYPE_KEYBOARD)
    .service_record         = (uint8_t *)sdp_hid_simple_keyboard_service_record,
#elif (BT_HID_TYPE == HID_TYPE_DOUYIN)
    .service_record         = (uint8_t *)sdp_hid_douyin_service_record,
#elif (BT_HID_TYPE == HID_TYPE_TSCREEN)
    .service_record         = (uint8_t *)sdp_hid_car_tscreen_service_record,
#else
    .service_record         = ...
#endif
};
#endif // BT_HID_EN

service_record_item_t device_id_sdp_record_item = {
    .service_record_handle  = 0x00010007,
#if BT_HID_EN && (BT_HID_TYPE == HID_TYPE_DOUYIN)
    .service_record         = (uint8_t *)sdp_hid_dy_pnp_service_record,
#else
    .service_record         = (uint8_t *)sdp_devic_id_service_record,
#endif // BT_HID_EN
};

#if BT_CARPLAY_EN
service_record_item_t spp_sdp_custom_record_item = {
    .service_record_handle  = 0x00010008,
    .service_record         = (uint8_t *)sdp_spp_custom_service_record,
};

service_record_item_t spp_sdp_carplay_record_item = {
    .service_record_handle  = 0x00010009,
    .service_record         = (uint8_t *)sdp_spp_carplay_service_record,
};
#endif

void bt_init_lib(void)
{
    uint32_t profile = cfg_bt_support_profile;
#if BT_SPP_EN
    if (profile & PROF_SPP) {
        spp_init();
        sdp_add_service(&spp_sdp_record_item);
#if BT_CARPLAY_EN
        sdp_add_service(&spp_sdp_custom_record_item);
        sdp_add_service(&spp_sdp_carplay_record_item);
#endif
    }
#endif

#if BT_HFP_EN
    if (profile & PROF_HFP) {
        hfp_hf_init();
        sdp_add_service(&hfp_sdp_record_item);
    }
#endif

	hsp_hs_init_var();
#if BT_HSP_EN
    if (profile & PROF_HSP) {
        hsp_hs_init();
        sdp_add_service(&hsp_sdp_record_item);
    }
#endif

#if BT_A2DP_EN
    if (profile & PROF_A2DP) {
        a2dp_init();
#if BT_A2DP_VOL_CTRL_EN
        //xcfg配置时，需要动态选择
        if(bt_a2dp_is_vol_ctrl()) {
            avctp_tg_sdp_record_item.service_record         = (uint8_t *)sdp_avctp_target_category2_service_record;
        } else {
            avctp_tg_sdp_record_item.service_record         = (uint8_t *)sdp_avctp_target_category1_service_record;
        }
#endif
        sdp_add_service(&avdtp_sdp_record_item);
        sdp_add_service(&avctp_ct_sdp_record_item);
        sdp_add_service(&avctp_tg_sdp_record_item);
        sdp_add_service(&device_id_sdp_record_item);
    }
#endif

#if BT_HID_EN
    if (profile & PROF_HID) {
        hid_device_init();
        sdp_add_service(&hid_sdp_record_item);
    }
#endif

#if BT_PBAP_EN || BT_MAP_EN
    if (profile & (PROF_PBAP|PROF_MAP)) {
        goep_client_init();
    }
#endif

#if BT_PBAP_EN
    // init PBAP
    if (profile & PROF_PBAP) {
        // init PBAP Client
        pbap_client_init();
    }
#endif
#if BT_MAP_EN
    if (profile & PROF_MAP) {
        // init MAP Client
        map_client_init();
    }
#endif
}
