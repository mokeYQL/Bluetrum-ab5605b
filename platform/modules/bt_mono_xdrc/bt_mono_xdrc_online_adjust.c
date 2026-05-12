#include "include.h"
#if  0 //EXLIB_BT_MONO_XDRC_EN 旧版Equalizer在线调音,已弃用
void huart_tx(const void *buf, uint len);
int eq_huart_putcs(uint8_t *packet, uint16_t len);
uint calc_crc(void *buf, uint len, uint seed);
void xdrc_adj_coef_update(void);
void xdrc_adj_parse_cmd_car(void);

enum {
    ADJ_EQ = 0,
    ADJ_DRC = 1,
};

enum {
    TOOL_EQUALIZER = 0,
    TOOL_CAR = 1,
};

#define EQ_ADJ_CRC_SEED                             0xffff
#define MKLETAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24)
#define GET_BUF_LE_U16(buf, ofs)         *(uint16_t *)((uint8_t *)buf + (ofs))
#define GET_BUF_LE_U32(buf, ofs)         *(uint32_t *)((uint8_t *)buf + (ofs))
#define EQ_ADJ_RXBUF_LEN   272

typedef struct _EQ_ADJ_T{
    u8 rx_done;
    u8 adj_type;   //ADJ_EQ or ADJ_DRC
    u8 res_type;   //EQ/LP/HP/DRC...
    u8 tool_type;  //0 equalizer  //1 car tool
    u8 rx_buf[EQ_ADJ_RXBUF_LEN];   //12段EQ至少需要 20+20*12
    u8 tx_buf[16];
}XDRC_ADJ_T;
extern XDRC_ADJ_T cb_xdrc_adj ;

XDRC_ADJ_T cb_xdrc_adj;

enum{
    XDRC_NULL = 0,
    XDRC_PRE_EQ,
    XDRC_LP_EQ,
    XDRC_LP_EXP_EQ,
    XDRC_HP_EQ,
    XDRC_HP_EXP_EQ,
    XDRC_LP_DRC,
    XDRC_HP_DRC,
    XDRC_ALL_DRC,
    XDRC_POST_EQ,
    XDRC_MAX_RES_NUM,
};

static const char * tbl_res_name[XDRC_MAX_RES_NUM + 1] = {
    "NULL",
    "PRE.EQ",
    "LP.EQ",
    "LP_EXP.EQ",
    "HP.EQ",
    "HP_EXP.EQ",
    "LP.DRC",
    "HP.DRC",
    "ALL.DRC",
    "POST.EQ",
};

AT(.text.eq_online_adj)
void xdrc_online_adj_init(void)
{
    memset(&cb_xdrc_adj,0x00, sizeof(cb_xdrc_adj));
}

AT(.com_text.eq_online_str)
const char str_dbg_eqrecv[] = "xdrc_huart_rx_done_callback %d\n";
AT(.com_text.eq_online_adj)
bool xdrc_huart_rx_done_callback(u8 *rx_buf)
{
    if ((rx_buf[0]=='C')&&(rx_buf[1]=='A')&&(rx_buf[2]=='R')&&(rx_buf[3]=='_')) {
        cb_xdrc_adj.tool_type = TOOL_CAR;
        msg_enqueue(EVT_ONLINE_SET_EQ);
        cb_xdrc_adj.rx_done = 1;
        printf(str_dbg_eqrecv,cb_xdrc_adj.tool_type);
        print_r(rx_buf,128);
    } else if(((rx_buf[0]=='E')&&(rx_buf[1]=='Q')) || ((rx_buf[0] =='D')&&(rx_buf[1]=='R'))){
        msg_enqueue(EVT_ONLINE_SET_EQ);
        cb_xdrc_adj.rx_done = 1;
        cb_xdrc_adj.tool_type = TOOL_EQUALIZER;
        printf(str_dbg_eqrecv,cb_xdrc_adj.tool_type);
        print_r(rx_buf,32);
        return true;
    }
    return false;
}

AT(.com_text.xdrc_adj)
u8* get_xdrc_adj_rxbuf_addr(void)
{
    return cb_xdrc_adj.rx_buf;
}

AT(.com_text.xdrc_adj)
u32 get_xdrc_adj_rxbuf_len(void)
{
    return EQ_ADJ_RXBUF_LEN;
}

AT(.text.eq_online_adj)
void xdrc_tx_buf_ack(u8* buf, u16 len)
{
    delay_5ms(1);   //延时一段时间再ack
    printf("xdrc_tx_buf_ack, rx_done = %d\n",cb_xdrc_adj.rx_done);
    if (cb_xdrc_adj.rx_done) {
        eq_huart_putcs(buf, len);  //eq_huart_putcs(packet, len); //huart_tx
        print_r(buf, len);
    }
    cb_xdrc_adj.rx_done = 0;
}

AT(.text.eq_online_adj)
void xdrc_tx_para_ack(u8 para, u8 ack)
{
    u8 *tx_buf = cb_xdrc_adj.tx_buf;
    if (ADJ_DRC == cb_xdrc_adj.adj_type) {
        tx_buf[0] = 'D';
    } else {
        tx_buf[0] = 'A';
    }
    tx_buf[1] = para;
    tx_buf[2] = ack;
    tx_buf[3] = -(tx_buf[0] + tx_buf[1] + tx_buf[2]);
    xdrc_tx_buf_ack(tx_buf, 4);
}

AT(.text.eq_online_adj)
void xdrc_ack_ver(u8 type)
{
    const char tbl_eq_ver[10] = {'E', 'Q', '0', '*', 6, 0, 12, 4, 5, 0};
    const char tbl_drc_ver[10] = {'D', 'R', '0', '*', 6, 0, 2, 0, 0, 0};
    u8 *tx_buf = cb_xdrc_adj.tx_buf;
     printf("xdrc_ack_ver:\n");
    if (ADJ_EQ == type) {
        memcpy(tx_buf, tbl_eq_ver, 10);
    } else {
        memcpy(tx_buf, tbl_drc_ver, 10);
    }
    tx_buf[9] |= BIT(0);   //是否下发文件名
    u16 crc = calc_crc(tx_buf, 10, EQ_ADJ_CRC_SEED);
    tx_buf[10] = crc;
    tx_buf[11] = crc >> 8;
    xdrc_tx_buf_ack(tx_buf, 12);
}

AT(.text.eq_online_adj)
void xdrc_res_name_parse(u8 adj_type)
{
    cb_xdrc_adj.adj_type = adj_type;
    char *rx_name = (char*)&cb_xdrc_adj.rx_buf[6];
    cb_xdrc_adj.res_type = XDRC_NULL;
    for(int i = 0; i < XDRC_MAX_RES_NUM; i++) {
        //printf("i = %d,tbl:%s, rx_name:%s\n",i,tbl_res_name[i], rx_name);
        if(strcmp(tbl_res_name[i], rx_name) == 0) {
            cb_xdrc_adj.res_type = i;
            break;
        }
    }
    if (cb_xdrc_adj.res_type && cb_xdrc_adj.res_type < XDRC_MAX_RES_NUM) {  //get name ok
        xdrc_tx_para_ack(0,0);
        printf("get name ok: type = %d,  %s\n",cb_xdrc_adj.res_type, rx_name);
    } else {  //get name fail
        xdrc_tx_para_ack(0,1);
        printf("get name fail: type = %d,  %s\n",cb_xdrc_adj.res_type, rx_name);
    }
}

AT(.text.eq_online_adj)
bool xdrc_res_coef_crc_check(void)
{
    u8 *rx_buf = cb_xdrc_adj.rx_buf;
    u8 band_num = rx_buf[6];
    u32 size = GET_BUF_LE_U16(rx_buf, 4);
    u32 crc = calc_crc(rx_buf, size+4, EQ_ADJ_CRC_SEED);   //
    if (crc == GET_BUF_LE_U16(rx_buf, 4+size)) {
        printf("coef recv crc ok\n");
        xdrc_tx_para_ack(band_num, 0);
        return true;
    } else {
        printf("coef recv crc fail\n");
        xdrc_tx_para_ack(band_num, 1);
        return false;
    }
}


AT(.text.eq_online_adj)
void xdrc_adj_parse_cmd_equalizer(void)
{
    printf("%s:\n",__func__);
    print_r(cb_xdrc_adj.rx_buf,16);
    u8 *rx_buf = cb_xdrc_adj.rx_buf;
    u32 tag = GET_BUF_LE_U32(rx_buf,0);
    switch (tag) {
    case MKLETAG('E','Q','?','#'):     //ASK EQ VER
        xdrc_ack_ver(ADJ_EQ);
        return;

    case MKLETAG('D','R','?','#'):     //ASK DRC VER
        xdrc_ack_ver(ADJ_DRC);
        break;

    case MKLETAG('E','Q','0',':'):     //EQ RES NAME
         xdrc_res_name_parse(ADJ_EQ);
         break;

    case MKLETAG('D','R','0',':'):     //DRC RES NAME
         xdrc_res_name_parse(ADJ_DRC);
         break;

    case MKLETAG('E','Q','0','#'):     //EQ COEF
    case MKLETAG('D','R','0','#'):     //DRC COEF
        if (xdrc_res_coef_crc_check()) {
            xdrc_adj_coef_update();
        }
        cb_xdrc_adj.res_type = XDRC_NULL;
        memset(rx_buf, 0, EQ_ADJ_RXBUF_LEN);
        break;

    default:
        printf("!!!Unknow CMD\n");
        print_r(&tag,4);
        print_r(rx_buf,4);
        break;
    }
}

AT(.text.eq_online_adj)
void xdrc_adj_coef_update(void)
{
    u8 *rx_buf = cb_xdrc_adj.rx_buf;
    u8 band_cnt;
    my_printf("adj_coef_update==>");
    switch(cb_xdrc_adj.res_type) {
    case XDRC_PRE_EQ:   //软件eq 示例
        printf("XDRC_PRE_EQ:\n");
        band_cnt = rx_buf[7];
        exlib_softeq_coef_update(xdrc_preeq_cb,sizeof(xdrc_preeq_cb),(u32 *)&rx_buf[14],band_cnt,true);
        break;

    case XDRC_LP_EQ:
        printf("XDRC_LP_EQ:\n");
        exlib_softeq_coef_update(xdrc_lp_cb,sizeof(xdrc_lp_cb),(u32 *)&rx_buf[14],1,true);
        exlib_softeq_coef_update(xdrc_lp_cb2,sizeof(xdrc_lp_cb2),(u32 *)&rx_buf[14],1,true);
        break;

    case XDRC_LP_EXP_EQ:
        printf("XDRC_LP_EXP_EQ:\n");
        exlib_softeq_coef_update(xdrc_lp_exp_cb,sizeof(xdrc_lp_exp_cb),(u32 *)&rx_buf[14],1,true);
        break;

    case XDRC_HP_EQ:
        printf("XDRC_HP_EQ:\n");
        exlib_softeq_coef_update(xdrc_hp_cb,sizeof(xdrc_hp_cb),(u32 *)&rx_buf[14],1,true);
        exlib_softeq_coef_update(xdrc_hp_cb2,sizeof(xdrc_hp_cb2),(u32 *)&rx_buf[14],1,true);
        break;

    case XDRC_HP_EXP_EQ:
        printf("XDRC_HP_EXP_EQ:\n");
        exlib_softeq_coef_update(xdrc_hp_exp_cb,sizeof(xdrc_hp_exp_cb),(u32 *)&rx_buf[14],1,true);
        break;

    case XDRC_LP_DRC:
        printf("XDRC_LP_DRC:\n");
        exlib_drcv1_coef_update(xdrc_drclp_cb, (u32*)&rx_buf[10]);
        break;
    case XDRC_HP_DRC:
        printf("XDRC_HP_DRC:\n");
        exlib_drcv1_coef_update(xdrc_drchp_cb, (u32*)&rx_buf[10]);
        break;

    case XDRC_ALL_DRC:
        printf("XDRC_ALL_DRC:\n");
        exlib_drcv1_coef_update(xdrc_drcall_cb, (u32*)&rx_buf[10]);
        break;

    case XDRC_POST_EQ:
        printf("XDRC_POST_EQ:\n");
        band_cnt = rx_buf[7];
        music_set_eq(band_cnt, (u32 *)&rx_buf[14]);
        break;

    default:
        printf("!!!Unknow Res Type\n");
        break;
    }
}
#endif  //EXLIB_BT_MONO_XDRC_EN
