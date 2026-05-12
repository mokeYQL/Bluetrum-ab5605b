#include "include.h"

#if  EXLIB_BT_MONO_XDRC_EN

#define TRACE_EN               1   //1 DEBUG TRACE

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

int eq_huart_putcs(uint8_t *packet, uint16_t len);
uint calc_crc(void *buf, uint len, uint seed);
void xdrc_set_prevol(s32 dig_vol);
u8 xdrc_prevol_effect_update(u8 *buf, u32 len, u8 params);
u8 xdrc_preeq_effect_update(u8 *buf, u32 len, u8 params);
u8 xdrc_posteq_effect_update(u8 *buf, u32 len, u8 params);
u8 xdrc_dyeq1_effect_update(u8 *buf, u32 len, u8 params);
u8 xdrc_dyeq2_effect_update(u8 *buf, u32 len, u8 params);
u8 xdrc_drcall_effect_update(u8 *buf, u32 len, u8 params);
u8 xdrc_drclphp_effect_update(u8 *buf, u32 len, u8 params);

#define STA_UPDATE_OK       0
#define STA_UPDATE_FAIL     1
#define STA_UPDATE_CRC_ERR  0xFF

#define EQ_ADJ_RXBUF_LEN   272
#define XDRC_RX_BUF   cb_xdrc_adj.rx_buf
#define XDRC_ACK_BUF  cb_xdrc_adj.tx_buf

enum {
    DRCLPHP_DRC_LP = 0,
    DRCLPHP_DRC_HP,
    DRCLPHP_EQ_LP,
    DRCLPHP_EQ_HP,
};
typedef struct _EQ_ADJ_T{
    u8 rx_buf[EQ_ADJ_RXBUF_LEN];   //12段EQ至少需要 20+20*12
    u8 tx_buf[16];
}XDRC_ADJ_T;
XDRC_ADJ_T cb_xdrc_adj;

const xdrc_effect_info_cfg_t xdrc_effect_info[CFG_MAX] = {
	[CFG_PRE_EQ]   = {"CFG_PRE_EQ##", BUF_OFS_CFG_PRE_EQ,   BUF_LEN_OFS_CFG_PRE_EQ},
	[CFG_POST_EQ]  = {"CFG_POST_EQ#", BUF_OFS_CFG_POST_EQ,  BUF_LEN_OFS_CFG_POST_EQ},
	[CFG_DRC_LPHP] = {"CFG_DRC_LPHP", BUF_OFS_CFG_DRC_LPHP, BUF_LEN_OFS_CFG_DRC_LPHP},
	[CFG_DYEQ_1]   = {"CFG_DYEQ_1##", BUF_OFS_CFG_DYEQ_1,   BUF_LEN_OFS_CFG_DYEQ_1},
	[CFG_DYEQ_2]   = {"CFG_DYEQ_2##", BUF_OFS_CFG_DYEQ_2,   BUF_LEN_OFS_CFG_DYEQ_2},
	[CFG_DRC_ALL]  = {"CFG_DRC_ALL#", BUF_OFS_CFG_DRC_ALL,  BUF_LEN_OFS_CFG_DRC_ALL},
	[CFG_VOL]      = {"CFG_VOL#####", BUF_OFS_CFG_VOL,      BUF_LEN_OFS_CFG_VOL},
};

const xdrc_effect_update_callback_t xdrc_effect_update_callback_tbl[CFG_MAX] = {
	[CFG_PRE_EQ]   = {xdrc_preeq_effect_update},
	[CFG_POST_EQ]  = {xdrc_posteq_effect_update},  //xdrc_posteq_effect_update
	[CFG_DRC_LPHP] = {xdrc_drclphp_effect_update}, //xdrc_drclphp_effect_update
	[CFG_DYEQ_1]   = {xdrc_dyeq1_effect_update},
	[CFG_DYEQ_2]   = {xdrc_dyeq2_effect_update},
	[CFG_DRC_ALL]  = {xdrc_drcall_effect_update},
	[CFG_VOL]      = {xdrc_prevol_effect_update},
};

AT(.text.xdrc_adj)
void xdrc_online_adj_init(void)
{
    memset(&cb_xdrc_adj,0x00, sizeof(cb_xdrc_adj));
}

AT(.com_text.xdrc_dbg_str)
const char str_dbg_eqrecv[] = "xdrc_huart_rx_done_callback\n";
AT(.com_text.eq_online_adj)
bool xdrc_huart_rx_done_callback(u8 *rx_buf)
{
    //print_r(rx_buf, 64);
    if ((rx_buf[0]=='C')&&(rx_buf[1]=='F')&&(rx_buf[2]=='G')&&(rx_buf[3]=='_')) {
        printf(str_dbg_eqrecv);
        //print_r(rx_buf,128);
        msg_enqueue(EVT_ONLINE_SET_EQ);
        return true;
    }
    return false;
}

AT(.text.xdrc_adj)
u8 effect_adjust_check_sum(u8 *buf, u16 size)
{
    u32 i, sum = 0;
    for (i = 0; i < size; i++) {
        sum += buf[i];
    }
    return (u8)(-sum);
}

AT(.text.xdrc_adj)
void xdrc_tx_ack(u8* buf, u16 len, u8 ack_sta)
{
    buf[12] = ack_sta;
    buf[13] =  effect_adjust_check_sum(XDRC_ACK_BUF, 13);
    TRACE("==>ack buf: ");
    print_r(buf,len);
    eq_huart_putcs(buf,len);
}

AT(.com_text.xdrc_adj)
u8* get_xdrc_adj_rxbuf_addr(void)
{
    return XDRC_RX_BUF;
}

AT(.com_text.xdrc_adj)
u32 get_xdrc_adj_rxbuf_len(void)
{
    return EQ_ADJ_RXBUF_LEN;
}

AT(.text.xdrc_adj)
bool xdrc_rx_buf_crc_check(u8 *rx_buf)
{
    u16 size = little_endian_read_16(rx_buf,12);
    u32 crc_calc = calc_crc(rx_buf, size + 12, 0xFFFF);
    u32 crc_recv = little_endian_read_16(rx_buf,size + 12);
    TRACE("crc_calc = 0x%X, crc_recv = 0x%X\n",crc_calc,crc_recv);
    if (crc_calc == crc_recv) {
        return true;
    } else {
        return false;
    }
}

AT(.text.xdrc_adj)
u16 mono_xdrc_link_check_sum(void)
{
    u16 chk_sum = 0;
     for(int i = 1; i < CFG_MAX; i++){
        for(int j = 4; j < 12 ; j++) {
            if (xdrc_effect_info[i].effect_cfg_name[j] != '#') {
                chk_sum +=  xdrc_effect_info[i].effect_cfg_name[j];
            } else {
                break;
            }
        }
     }
     return chk_sum;
}

AT(.text.xdrc_adj)
bool xdrc_all_effect_send_cmd_check(u8* ack_sta)
{
    u8 all_effect_name[12] = "CFG_########";
    if (memcmp(XDRC_RX_BUF,all_effect_name,12)) {  //name compare fail
        return false;
    }
    if (mono_xdrc_link_check_sum() == little_endian_read_16(XDRC_RX_BUF, 14)) {
        *ack_sta = STA_UPDATE_OK;
    } else {
        *ack_sta = STA_UPDATE_FAIL;
    }
    TRACE("CMD all link name check&compare, ack = %d\n",ack_sta);
    return true;
}

AT(.text.xdrc_adj)
void mono_xdrc_onlie_adjust_process(void)
{
    u8 ack_sta = STA_UPDATE_OK;  //0 is ok  //other: fail
    TRACE("\n###### mono_xdrc_onlie_adjust_process =>\n");
    memcpy(XDRC_ACK_BUF, XDRC_RX_BUF, 12);

    if(xdrc_all_effect_send_cmd_check(&ack_sta)) {
        xdrc_tx_ack(XDRC_ACK_BUF,14, ack_sta);
        return;
    }
    // print_r(effect_adjust_tx_ack_buf, 14);
    for(int i = 1; i < CFG_MAX; i++){
        if (0 == memcmp(XDRC_RX_BUF,xdrc_effect_info[i].effect_cfg_name, 12)) {
            if (!xdrc_rx_buf_crc_check(XDRC_RX_BUF) ) {
                TRACE("%s Recv CRC ERR!!!\n",xdrc_effect_info[i].effect_cfg_name);
                ack_sta = STA_UPDATE_CRC_ERR;  //ack 0xFF表示CRC出错
                continue;
            }
            TRACE("######%s Online\n",xdrc_effect_info[i].effect_cfg_name);
            if(xdrc_effect_update_callback_tbl[i].effect_update_callback){
                ack_sta = xdrc_effect_update_callback_tbl[i].effect_update_callback(XDRC_RX_BUF,little_endian_read_16(XDRC_RX_BUF, 12)+14,1);
                TRACE("######%s update finish, ack_sta = %d\n\n",xdrc_effect_info[i].effect_cfg_name, ack_sta);
            }else{
                TRACE("%s NO INIT CALLBACK\n",xdrc_effect_info[i].effect_cfg_name);
            }
        }
    }
    xdrc_tx_ack(XDRC_ACK_BUF,14, ack_sta);
    memset(XDRC_RX_BUF, 0, EQ_ADJ_RXBUF_LEN);
}

AT(.text.xdrc_adj)
u8 xdrc_prevol_effect_update(u8 *buf, u32 len, u8 params)
{
    u8 en = buf[14+1];
    xdrc_audio_en_cfg(BIT_XDRC_PREVOL,en);
    u32 vol = little_endian_read_32(buf,16);
    TRACE("prevol update: en = %d, vol = 0x%X(%d)\n", en, vol, vol);
    xdrc_prevol_set(vol);
    return STA_UPDATE_OK;
}


AT(.text.xdrc_adj)
u8 xdrc_preeq_effect_update(u8 *buf, u32 len, u8 params)
{
    bool ret;
    u8 en = buf[14+1];
    u8 band_cnt = buf[14 + 3];
    xdrc_audio_en_cfg(BIT_XDRC_PREEQ,en);
    ret = exlib_softeq_coef_update(xdrc_preeq_cb,sizeof(xdrc_preeq_cb),(u32 *)&buf[14+10],band_cnt,true);
    if (ret) {
        return STA_UPDATE_OK;
    } else {
        return STA_UPDATE_FAIL;
    }
}

AT(.text.xdrc_adj)
u8 xdrc_posteq_effect_update(u8 *buf, u32 len, u8 params)
{
    u8 en = buf[14+1];
    u8 band_cnt = buf[14 + 3];
    printf("posteq update: band = %d, gain = 0x%X\n", band_cnt, little_endian_read_32(buf,24));
    xdrc_audio_en_cfg(BIT_XDRC_POSTEQ,en);
    music_set_eq(band_cnt, (u32 *)&buf[14+10]);
    return STA_UPDATE_OK;
}

AT(.text.xdrc_adj)
u8 xdrc_dyeq1_effect_update(u8 *buf, u32 len, u8 params)
{
    u8 en = buf[14];
    xdrc_audio_en_cfg(BIT_XDRC_DYEQ1,en);
    dynamic_eq_update(dynamic_eq_cb1, (s32*)&buf[14+2]);
    return STA_UPDATE_OK;  //0 OK
}

AT(.text.xdrc_adj)
u8 xdrc_dyeq2_effect_update(u8 *buf, u32 len, u8 params)
{
    u8 en = buf[14];
    xdrc_audio_en_cfg(BIT_XDRC_DYEQ2,en);
    dynamic_eq_update(dynamic_eq_cb2, (s32*)&buf[14+2]);
    return STA_UPDATE_OK;
}

AT(.text.xdrc_adj)
u8 xdrc_drcall_effect_update(u8 *buf, u32 len, u8 params)
{
    u8 en = buf[14+1];
    xdrc_audio_en_cfg(BIT_XDRC_ALLDRC,en);
    exlib_drcv1_coef_update(xdrc_drcall_cb, (u32*)&buf[14+6]);
    return STA_UPDATE_OK;
}

AT(.text.xdrc_adj)
bool drclphp_coef_update_do(u8 *buf)  //form rx_buf[14]
{
    bool ret = false;
    u32 err_cnt = 0;
    u8 type = buf[0];
    if (type >= 4) {
        TRACE("!!!!drclphp coef update do, type(%d) invalid\n\n");
        return false;
    }
    //TRACE("type[%d]:%s\n",type,drc_lphp_type_name[type]);
    switch(type) {
        case DRCLPHP_DRC_LP:  //drc block 8bye info
            TRACE("DRCLPHP_DRC_LP:\n");
            ret = exlib_drcv1_coef_update(xdrc_drclp_cb, (u32*)&buf[9]);
            break;

        case DRCLPHP_DRC_HP:
            TRACE("DRCLPHP_DRC_HP:\n");
            ret = exlib_drcv1_coef_update(xdrc_drchp_cb, (u32*)&buf[9]);
            break;

        case DRCLPHP_EQ_LP:
            err_cnt = 0;
            //FILTER_LP 4阶
            TRACE("DRCLPHP_EQ_LP:\n");
            if (!exlib_softeq_coef_update(xdrc_lp_cb,sizeof(xdrc_lp_cb),(u32 *)&buf[13],1,true)){
                err_cnt++;
            }
            if (!exlib_softeq_coef_update(xdrc_lp_cb2,sizeof(xdrc_lp_cb2),(u32 *)&buf[13],1,true)){
                 err_cnt++;
            }
            //FILTER LP DRC (EXPAND)
            TRACE("EQ_LP_EXPAND:\n");
            if (!exlib_softeq_coef_update(xdrc_lp_exp_cb,sizeof(xdrc_lp_exp_cb),(u32 *)&buf[13+ 4 +20],1,false)){
               err_cnt++;
            }
            if (err_cnt) {
                TRACE("!!!DRCLPHP_EQ_LP fail %d\n",err_cnt);
                ret = false;
            } else {
                ret = true;
            }
            break;

        case DRCLPHP_EQ_HP:
            err_cnt = 0;
            //FILTER_HP 4阶
            TRACE("DRCLPHP_EQ_HP:\n");
            if (!exlib_softeq_coef_update(xdrc_hp_cb,sizeof(xdrc_hp_cb),(u32 *)&buf[13],1,true)){
                err_cnt++;
            }
            if (!exlib_softeq_coef_update(xdrc_hp_cb2,sizeof(xdrc_hp_cb2),(u32 *)&buf[13],1,true)) {
                err_cnt++;
            }
            //FILTER HP DRC (EXPAND)
            TRACE("EQ_HP_EXPAND:\n");
            if (!exlib_softeq_coef_update(xdrc_hp_exp_cb,sizeof(xdrc_hp_exp_cb),(u32 *)&buf[13 + 4 +20],1,false)) {
                err_cnt++;
            }
            if (err_cnt) {
                TRACE("!!!DRCLPHP_EQ_HP fail %d\n",err_cnt);
                ret = false;
            } else {
                ret = true;
            }
            break;

        default:
            TRACE("xdrc_effect_update, unknow type(%d)\n", type);
            ret = false;
            break;
    }
    return ret;
}

AT(.text.xdrc_adj)
u8 xdrc_drclphp_effect_update(u8 *buf, u32 buf_len, u8 params)  //form rx_buf[14]
{
    u8 type, err_cnt = 0;
    u16 len;
    u8 en = buf[14+1];
    u8 *buf_low_drc = &buf[14 + 2 + 1];    //
    u8 *buf_high_drc = &buf[14 + 51 + 1];
    u8 *buf_low_eq = &buf[14 + 100 + 1];
    u8 *buf_high_eq = &buf[14 + 157 + 1];

    xdrc_audio_en_cfg(BIT_XDRC_LPHP,en);

    type = buf_low_drc[0];
    len = little_endian_read_16(buf_low_drc,1);
    if(type != DRCLPHP_DRC_LP || len != 0x2E) {
        TRACE("!!!drclphp coef err in drc_lp[0x%X, 0x%X]\n",type, len);
        err_cnt++;
    }

    type = buf_high_drc[0];
    len = little_endian_read_16(buf_high_drc,1);
    if(type != DRCLPHP_DRC_HP || len  != 0x2E) {
        TRACE("!!!drclphp coef err in drc_hp[0x%X, 0x%X]\n",type, len);
        err_cnt++;
    }

    type = buf_low_eq[0];
    len = little_endian_read_16(buf_low_eq,1);
    if(type != DRCLPHP_EQ_LP || len != 0x36) {
        TRACE("!!!drclphp coef err in eq_lp[0x%X, 0x%X]\n",type, len);
        err_cnt++;
    }

    type = buf_high_eq[0];
    len = little_endian_read_16(buf_high_eq,1);
    if(type != DRCLPHP_EQ_HP || len != 0x36) {
        TRACE("!!!drclphp coef err in eq_lp[0x%X, 0x%X]\n",type, len);
        err_cnt++;
    }

    if (err_cnt) {
        TRACE("!!!drclphp coef err_cnt = %d\n",err_cnt);
        return 1;  //0 ok,  1 fail
    }

    err_cnt = 0;
    if (!drclphp_coef_update_do(buf_low_drc)) {
        err_cnt++;
    }
    if (!drclphp_coef_update_do(buf_high_drc)) {
        err_cnt++;
    }
    if (!drclphp_coef_update_do(buf_low_eq)) {
        err_cnt++;
    }
    if (!drclphp_coef_update_do(buf_high_eq)) {
        err_cnt++;
    }
    if (err_cnt) {
        TRACE("!!!drclphp_coef_update some fail %d\n",err_cnt);
        return STA_UPDATE_FAIL;
    } else {
        return STA_UPDATE_OK;
    }
}


AT(.text.xdrc_adj)
void xdrc_res_effect_init(u32 res_addr, u32 res_len)
{
    TRACE("%s\n", __func__);
    u32 effect_addr;
    u32 effect_len;

    for(int i = 1; i < CFG_MAX; i++){
        effect_addr = *(u32*)(res_addr + xdrc_effect_info[i].effect_res_offset) + res_addr;
        effect_len  = *(u32*)(res_addr + xdrc_effect_info[i].effect_len_offset);
        if(xdrc_effect_update_callback_tbl[i].effect_update_callback){
            TRACE("==>%s init run, addr = 0x%X, len = %d\n",xdrc_effect_info[i].effect_cfg_name, effect_addr,effect_len);
            if(0 == xdrc_effect_update_callback_tbl[i].effect_update_callback((u8*)effect_addr,effect_len, 0)) {
                TRACE("%s init OK\n\n",xdrc_effect_info[i].effect_cfg_name);
            } else {
                TRACE("%s init Err!!!!\n\n",xdrc_effect_info[i].effect_cfg_name);
            }
        }else{
            TRACE("%s NO INIT CALLBACK\n",xdrc_effect_info[i].effect_cfg_name);
        }
    }
}

#endif  //EXLIB_BT_MONO_XDRC_EN
