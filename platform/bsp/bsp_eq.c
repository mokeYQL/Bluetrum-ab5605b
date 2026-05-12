#include "include.h"
#include "bsp_eq.h"

#define EQ_CRC_SEED                             0xffff
#define EQ_BAND_NUM                             12
#define CAL_FIX(x)		                        ((int)(x * (1 << 27)))

extern uint8_t cfg_bt_sco_dump;

uint calc_crc(void *buf, uint len, uint seed);
int eq_huart_putcs(uint8_t *packet, uint16_t len);
void huart_eq_init(u8 *rx_buf, uint rx_len);
void huart_rx_kick(void);

extern const eq_param music_eq_tbl[MUSIC_EQ_TBL_LEN];

AT(.text.music)
void music_set_eq_by_num(u8 num)
{
    if (num > (MUSIC_EQ_TBL_LEN - 1)) {
        return;
    }
    //printf("\n%s:%d\n",__func__,num);
#if EXLIB_BT_MONO_XDRC_EN
    //调音参数已加载,在前面的函数xdrc_res_effect_init(RES_BUF_EFFECT_MON_XDRC_BIN, RES_LEN_EFFECT_MON_XDRC_BIN)中。
#else
    music_set_eq_by_res(music_eq_tbl[num].addr, music_eq_tbl[num].len);
#endif
}

#if  0  //FOR TEST
#define MIC_EQ_TBL_LEN                          2
const eq_param mic_eq_tbl[MIC_EQ_TBL_LEN] = {
    {&RES_BUF_EQ_MIC_NORMAL_EQ,     &RES_LEN_EQ_MIC_NORMAL_EQ}, //normal
    {&RES_BUF_EQ_BT_MIC_EQ,         &RES_LEN_EQ_BT_MIC_EQ},     //bt_mic
};
static u8 mic_eq_num = 0;
AT(.text.music)
void sys_mic_set_eq_by_num(u8 mic_eq_num){
    mic_set_eq_by_res(mic_eq_tbl[mic_eq_num-1].addr, mic_eq_tbl[mic_eq_num-1].len);
}

AT(.text.music)
void sys_mic_set_eq(void)
{
    mic_eq_num++;
    if(mic_eq_num > MIC_EQ_TBL_LEN){
        mic_eq_num = 1;
    }
    sys_mic_set_eq_by_num(mic_eq_num);
}
#endif


#if EQ_MODE_EN

AT(.text.music)
void sys_set_eq(void)
{
    sys_cb.eq_mode++;
    if (sys_cb.eq_mode > 5) {
        sys_cb.eq_mode = 0;
    }
    music_set_eq_by_num(sys_cb.eq_mode);
    gui_box_show_eq();
}
#endif // EQ_MODE_EN

u8 eq_rx_buf[EQ_BUFFER_LEN];
#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
eq_dbg_cb_t eq_dbg_cb;
static u8 eq_tx_buf[12];
//格式:[E/D, Q/R, 0, *, (u8)size, (size >> 8), band_cnt, param size, param cnt, rsvd]
const char tbl_eq_version[10] = {'E', 'Q', '0', '*', 6, 0, 12, 4, 5, 0};
const char tbl_drc_version[10] = {'D', 'R', '0', '*', 6, 0, 2, 0, 0, 0};

static void tx_ack(uint8_t *packet, uint16_t len)
{
    delay_5ms(1);   //延时一段时间再ack
    if (eq_dbg_cb.rx_type) {
#if EQ_DBG_IN_UART
        eq_huart_putcs(packet, len);
#endif // EQ_DBG_IN_UART
    } else {
#if EQ_DBG_IN_SPP && BT_SPP_EN
        if (xcfg_cb.eq_dgb_spp_en && (bt_get_status() >= BT_STA_CONNECTED)) {
            bt_spp_tx(packet, len);
        }
#endif // EQ_DBG_IN_SPP
    }
    eq_dbg_cb.rx_type = 0;
}

void eq_tx_ack(u8 bank_num, u8 ack)
{
    if (eq_dbg_cb.type) {
        eq_tx_buf[0] = 'D';
    } else {
        eq_tx_buf[0] = 'A';
    }
    eq_tx_buf[1] = bank_num;
    eq_tx_buf[2] = ack;
    eq_tx_buf[3] = -(eq_tx_buf[0] + eq_tx_buf[1] + eq_tx_buf[2]);
    tx_ack(eq_tx_buf, 4);
}

void eq_tx_version(void)
{
    if (eq_dbg_cb.type) {
        memcpy(eq_tx_buf, tbl_drc_version, 10);
    } else {
        memcpy(eq_tx_buf, tbl_eq_version, 10);
    }
    u16 crc = calc_crc(eq_tx_buf, 10, EQ_CRC_SEED);
    eq_tx_buf[10] = crc;
    eq_tx_buf[11] = crc >> 8;
    tx_ack(eq_tx_buf, 12);
}

#if BT_TWS_EN
uint16_t tws_get_spp_eq_info(uint8_t lid, uint8_t *buf)
{
    if(buf != NULL) {
        memcpy(buf, eq_rx_buf, EQ_BUFFER_LEN);
    }
    //printf("get_spp_eq: %d\n", EQ_BUFFER_LEN);
    return EQ_BUFFER_LEN;
}

void tws_set_spp_eq_info(uint8_t *buf, uint16_t len)
{
    if(buf != NULL) {
        memcpy(eq_rx_buf, buf, len);
        msg_enqueue(EVT_ONLINE_SET_EQ);
    }
    //printf("set_spp_eq: %d\n", len);
}
#else
void tws_set_spp_eq_info(uint8_t *buf, uint16_t len)
{
}
#endif

void eq_parse_cmd(void)
{
    if (eq_rx_buf[0] == 'E' || eq_rx_buf[1] == 'Q') {
        eq_dbg_cb.type = 0;
    } else if (eq_rx_buf[0] == 'D' && eq_rx_buf[1] == 'R') {
        eq_dbg_cb.type = 1;
    } else {
        return;
    }
    if (eq_rx_buf[2] == '?' && eq_rx_buf[3] == '#') {
        eq_tx_version();
        return;
    }

//    print_r(eq_rx_buf, EQ_BUFFER_LEN);
    u8 band_num = eq_rx_buf[6];
    u32 size = little_endian_read_16(eq_rx_buf, 4);
    u32 crc = calc_crc(eq_rx_buf, size+4, EQ_CRC_SEED);
    if (crc == little_endian_read_16(eq_rx_buf, 4+size)) {
        eq_tx_ack(band_num, 0);
    } else {
        eq_tx_ack(band_num, 1);
        return;
    }

    if (crc == eq_dbg_cb.pre_crc) {
        return;
    }
    eq_dbg_cb.pre_crc = crc;
#if BT_TWS_EN
    if(!bt_tws_is_slave()) {
        bt_tws_sync_eq_param();
    }
#endif
    if (eq_dbg_cb.type == 0) {
        u8 band_cnt = eq_rx_buf[7];
        printf("band_cnt = %d\n",band_cnt);
#if (UART0_PRINTF_SEL != PRINTF_NONE)
        printf("%08x\n", little_endian_read_32(eq_rx_buf, 14));
        u8 k;
        u32 *ptr = (u32 *)&eq_rx_buf[18];
        for (k = 0; k < band_cnt*5; k++) {
            printf("%08x", *ptr++);
            if (k % 5 == 4) {
                printf("\n");
            } else {
                printf(" ");
            }
        }
#endif

#if MIC_EQ_EN
        if(func_cb.sta == FUNC_SPEAKER || SYS_KARAOK_EN){
            mic_set_eq(band_cnt,(u32 *)&eq_rx_buf[14]);
        }else{
            music_set_eq(band_cnt, (u32 *)&eq_rx_buf[14]);
        }
#else
        music_set_eq(band_cnt, (u32 *)&eq_rx_buf[14]);
#endif
    } else {
        //drc_set_param((u32 *)&eq_rx_buf[10]);
#if (UART0_PRINTF_SEL != PRINTF_NONE)
        u32 *ptr = (u32 *)&eq_rx_buf[10];
        for (int i = 0; i < 10; i++) {
            printf("%08x\n", ptr[i]);
        }
        printf("\n");
#endif
    }

    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
}

#if EQ_DBG_IN_UART

AT(.com_eq.text)
void huart_rx_done(void)
{

#if EXLIB_SOFT_EQ_DRC_EN
    if (sed_huart_rx_done_callback(get_sed_adj_rxbuf_addr())) {
        return;
    }
#elif EXLIB_BT_MONO_XDRC_EN
    if (xdrc_huart_rx_done_callback(get_xdrc_adj_rxbuf_addr())) {
        return;
    }
#else
    msg_enqueue(EVT_ONLINE_SET_EQ);
    eq_dbg_cb.rx_type = 1;
#endif

}

AT(.com_eq.text)
void huart_rx_set_dir(void)
{
    if (xcfg_cb.eq_uart_sel == 0) {
        GPIOADIR |= BIT(7);
    } else if (xcfg_cb.eq_uart_sel == 1) {
        GPIOBDIR |= BIT(2);
    } else if (xcfg_cb.eq_uart_sel == 2) {
        GPIOBDIR |= BIT(3);
    }
}

AT(.com_eq.text)
void huart_tx_set_dir(void)
{
    if (xcfg_cb.eq_uart_sel == 0) {
        GPIOADIR &= ~BIT(7);
    } else if (xcfg_cb.eq_uart_sel == 1) {
        GPIOBDIR &= ~BIT(2);
    } else if (xcfg_cb.eq_uart_sel == 2) {
        GPIOBDIR &= ~BIT(3);
    }
}

void eq_dbg_huart_init(void)
{
    FUNCMCON0 = 0x0f << 20 | 0x0f << 16;
    if (xcfg_cb.eq_uart_sel == 0) {
        if (UART0_PRINTF_SEL == PRINTF_PA7) {
            FUNCMCON0 = 0x0f << 8;
        }
        FUNCMCON0 = 0x01 << 20 | 0x01 << 16;
        GPIOADE |= BIT(7);
        GPIOAFEN |= BIT(7);
        GPIOAPU |= BIT(7);
    } else if (xcfg_cb.eq_uart_sel == 1) {
        if (UART0_PRINTF_SEL == PRINTF_PB2) {
            FUNCMCON0 = 0x0f << 8;
        }
        FUNCMCON0 = 0x02 << 20 | 0x02 << 16;
        GPIOBDE |= BIT(2);
        GPIOBFEN |= BIT(2);
        GPIOBPU |= BIT(2);
    } else if (xcfg_cb.eq_uart_sel == 2) {
        if (UART0_PRINTF_SEL == PRINTF_PB3) {
            FUNCMCON0 = 0x0f << 8;
        }
	  	printf("audio adj huart sel PB3\n");
        FUNCMCON0 = 0x03 << 20 | 0x03 << 16;
        GPIOBDE |= BIT(3);
        GPIOBFEN |= BIT(3);
        GPIOBPU |= BIT(3);
    }

#if EXLIB_SOFT_EQ_DRC_EN
    huart_eq_init(get_sed_adj_rxbuf_addr(), get_sed_adj_rxbuf_len());
#elif EXLIB_BT_MONO_XDRC_EN
    huart_eq_init(get_xdrc_adj_rxbuf_addr(), get_xdrc_adj_rxbuf_len());
#else
    huart_eq_init(eq_rx_buf, EQ_BUFFER_LEN);
#endif
    huart_rx_set_dir();
    huart_rx_kick();
}
#endif

void eq_dbg_init(void)
{
    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
    memset(&eq_dbg_cb, 0, sizeof(eq_dbg_cb_t));
#if EQ_DBG_IN_UART
    if (xcfg_cb.eq_dgb_uart_en) {
        eq_dbg_huart_init();
    } else {
        cfg_bt_sco_dump = 0;
    }
#endif // EQ_DBG_IN_UART

}
#endif

#if !EQ_DBG_IN_UART
void huart_rx_done(void) {}
void huart_rx_set_dir(void) {}
void huart_tx_set_dir(void) {}
#endif

struct eq_coef_t {
    u32 param0;                         //频点参数，由工具计算
    u32 param1;
    u32 coef[5];                        //频段参数，由bass_treble_coef_cal计算
};

struct eq_cfg_t {
    struct eq_coef_t *msc_coef[12];
    struct eq_coef_t *mic_coef[8];
};

#if SYS_BASS_TREBLE_EN

struct eq_coef_t msc_bass = {
    .param0 = CAL_FIX(0.0284864965),    //BASS:300Hz
    .param1 = CAL_FIX(0.9990866674),    //BASS:300Hz
};

struct eq_coef_t msc_treb = {
    .param0 = CAL_FIX(0.2763541250),    //TREB:3000Hz
    .param1 = CAL_FIX(0.9100351062),    //TREB:3000Hz
};

struct eq_coef_t mic_bass = {
    .param0 = CAL_FIX(0.0284864965),    //BASS:300Hz
    .param1 = CAL_FIX(0.9990866674),    //BASS:300Hz
};

struct eq_coef_t mic_treb = {
    .param0 = CAL_FIX(0.2763541250),    //TREB:3000Hz
    .param1 = CAL_FIX(0.9100351062),    //TREB:3000Hz
};

void bsp_bass_treble_init(void)
{
    bass_treble_coef_cal(&mic_bass, 0, 0);
    bass_treble_coef_cal(&mic_treb, 0, 1);

    bass_treble_coef_cal(&msc_bass, 0, 0);
    bass_treble_coef_cal(&msc_treb, 0, 1);
}

//设置MIC高低音（不会影响其他段EQ，可以边调EQ边调高低音）
void mic_bass_treble_set(int mode, int gain)
{
    if (mode == 0) {            //bass
        bass_treble_coef_cal(&mic_bass, gain, 0);
    } else if (mode == 1) {     //treble
        bass_treble_coef_cal(&mic_treb, gain, 1);
    }
#ifdef RES_BUF_EQ_MIC_NORMAL_EQ
    mic_set_eq_by_res(&RES_BUF_EQ_MIC_NORMAL_EQ, &RES_LEN_EQ_MIC_NORMAL_EQ);    //更新参数
#endif
}

//设置MUSIC高低音（不会影响其他段EQ，可以边调EQ边调高低音）
void music_bass_treble_set(int mode, int gain)
{
    if (mode == 0) {            //bass
        bass_treble_coef_cal(&msc_bass, gain, 0);
    } else if (mode == 1) {     //treble
        bass_treble_coef_cal(&msc_treb, gain, 1);
    }

    music_set_eq_by_num(sys_cb.eq_mode);        //更新参数
}
#endif

#if SYS_EQ_LOUD_EN
struct eq_coef_t msc_loud = {
    .param0 = CAL_FIX(0.0094980694),    //LOUD:100Hz
    .param1 = CAL_FIX(0.9998985049),    //LOUD:100Hz
};

void bsp_loud_init(void)
{
    eq_coef_cal(&msc_loud,0);
}

void music_loud_set(int gain)
{
    if(gain > 12) {
        gain = 12;
    }   else if(gain <-12) {
        gain = -12;
    }
    eq_coef_cal(&msc_loud,gain);
    music_set_eq_by_num(sys_cb.eq_mode);
}
#endif


#if DYNAMIC_BASS_BOOST_EN
extern const u32 eq_param_normal_tbl[5];
extern const u8 music_bass_boost_eq_table[863];
extern const u8 music_peak_boost_eq_table[863];
struct eq_coef_t msc_bass_boost;
struct eq_coef_t msc_peak_boost;
const u32 eq_vol_tbl_16[16 + 1] = {
    DIG_N60DB,  DIG_N43DB,  DIG_N32DB,  DIG_N26DB,  DIG_N24DB,  DIG_N22DB,  DIG_N20DB,  DIG_N18DB, DIG_N16DB,
    DIG_N14DB,  DIG_N12DB,  DIG_N10DB,  DIG_N8DB,   DIG_N6DB,   DIG_N4DB,   DIG_N2DB,   DIG_N0DB,
};

const u32 vol_bass_boost_tbl[16 + 1] = {
    0,  20, 18, 16, 14, 12, 10, 9,
    8,  7,  6,  5,  4,  3,  2,  1, 0,
};

#if DYNAMIC_BASS_BOOST_ADD_PEAK
const u32 vol_peak_boost_tbl[16 + 1] = {
    0,  20, 18, 16, 14, 12, 10, 9,
    8,  7,  6,  5,  4,  3,  2,  1, 0,
};
#endif

#if EXLIB_SOFT_EQ_EN
u32 soft_eq_boost_tbl[5];
AT(.text.exlib_softeq)
const u32 *exlib_soft_eq_get_tbl(u8 index)   //第7段用于做软件低音增强(index序号从0开始)
{
    printf("eq_idx = %d\n",index);
    if ((7-1) == index) {
        return soft_eq_boost_tbl;
    }
    return NULL;
}
#endif

void music_bass_boost_init(void)
{

#if EXLIB_SOFT_EQ_EN
    music_set_soft_eq_gain(eq_vol_tbl_16[sys_cb.vol]);
    memcpy(soft_eq_boost_tbl,eq_param_normal_tbl,20);
#else
    music_set_eq_gain(eq_vol_tbl_16[sys_cb.vol]);
    memcpy(&msc_bass_boost.coef,eq_param_normal_tbl,20);
#if DYNAMIC_BASS_BOOST_ADD_PEAK
    memcpy(&msc_peak_boost.coef,eq_param_normal_tbl,20);
#endif
#endif
}


void music_bass_boost_vol_set(u8 vol)
{
    u16 bass_idx;
    dac_set_dvol(0x7ca0);
    bass_idx = 23 + vol_bass_boost_tbl[vol] * 20;

#if EXLIB_SOFT_EQ_EN
    printf("boost vol set = %d, sys_cb.eq_mode = %d, total_band = %d\n",vol, sys_cb.eq_mode, exlib_soft_eq_total_band_get());
    music_set_soft_eq_gain(eq_vol_tbl_16[vol]);
    memcpy(soft_eq_boost_tbl, &music_bass_boost_eq_table[bass_idx],20);
#else
	printf("boost vol set = %d , sys_cb.eq_mode = %d\n",vol, sys_cb.eq_mode);
    music_set_eq_gain(eq_vol_tbl_16[vol]);
    memcpy(&msc_bass_boost.coef,&music_bass_boost_eq_table[bass_idx],20);
#if DYNAMIC_BASS_BOOST_ADD_PEAK
    u16 peak_idx;
    peak_idx = 23 + vol_peak_boost_tbl[vol] * 20;
    memcpy(&msc_peak_boost.coef,&music_peak_boost_eq_table[peak_idx],20);
#endif
#endif
    music_set_eq_by_num(sys_cb.eq_mode);
    delay_5ms(6);
}
#endif


static struct eq_cfg_t eq_cfg = {
#if SYS_BASS_TREBLE_EN
    .msc_coef[10] = &msc_bass,
    .msc_coef[11] = &msc_treb,

    .mic_coef[6] = &mic_bass,
    .mic_coef[7] = &mic_treb,
#endif
#if SYS_EQ_LOUD_EN
    .msc_coef[11] = &msc_loud,
#endif

#if DYNAMIC_BASS_BOOST_EN
#if DYNAMIC_BASS_BOOST_ADD_PEAK
    .msc_coef[10] = &msc_peak_boost,
#endif
    .msc_coef[11] = &msc_bass_boost,

#endif
};

#if  0 //EQ_DBG_DUMP //设置EQ时,导出实际EQ设置的数据,怀疑在线调EQ和最终整合的资源EQ不一致。这里导出数据进行对比
enum EQDUMP_TYPE {
    EQDUMP_GAIN0 = 0,
    EQDUMP_GAIN1 = 1,
    EQDUMP_EXTBL = 2,
    EQDUMP_RESTBL = 3,
    EQDUMP_DEFAULT = 4 ,
    EQDUMP_MAX,
};
const char *eqdump_type[] = {
   "Gain0:",
   "Gain1:",
   "ExtTbl:",
   "ResTbl:",
   "Default:"
};

void eq_info_dump(u8 type, u32 val)
{
    static u32 cnt = 0;
    static u32 last_type = 0;
    if (EQDUMP_GAIN0 == type) {
        my_printf("\nEQ info dump:\n");
        my_printf("Gain0: 0x%X\n", val);
        cnt = 0;
    } else if(1 == type) {
        my_printf("Gain1: 0x%X\n", val);
        cnt = 0;
    } else {
        if (last_type != type) {
            last_type = type;
            if (type < EQDUMP_MAX) {
                my_printf("type:%s\n",eqdump_type[type]);
            }
        }
        cnt++;
        my_printf("%X ",val);
        if (cnt % 5 == 0) {
            printf("\n");
        }
    }
}
#endif // EQ_DBG_DUMP


void bsp_eq_init(void)
{
    eq_var_init(&eq_cfg);
#if SYS_BASS_TREBLE_EN
    bsp_bass_treble_init();
#endif
#if SYS_EQ_LOUD_EN
    bsp_loud_init();
#endif

#if DYNAMIC_BASS_BOOST_EN
    music_bass_boost_init();
#endif
}

////根据index设置12条EQ示例，包含高低音
////Q:0.750000
//struct eq_coef_t tbl_coef[12] = {
//    //param0                    param1
//    { CAL_FIX(0.0029444916),    CAL_FIX(0.9999902462)},     //Band:0(31Hz)
//    { CAL_FIX(0.0059839058),    CAL_FIX(0.9999597162)},     //Band:1(63Hz)
//    { CAL_FIX(0.0151961090),    CAL_FIX(0.9997401793)},     //Band:2(160Hz)
//    { CAL_FIX(0.0379729998),    CAL_FIX(0.9983764898)},     //Band:3(400Hz)
//    { CAL_FIX(0.0946628813),    CAL_FIX(0.9898674722)},     //Band:4(1000Hz)
//    { CAL_FIX(0.3597005044),    CAL_FIX(0.8419530754)},     //Band:5(4000Hz)
//    { CAL_FIX(0.6057018917),    CAL_FIX(0.4177699622)},     //Band:6(8000Hz)
//    { CAL_FIX(0.5060881129),    CAL_FIX(-0.6509365173)},    //Band:7(16000Hz)
//    { CAL_FIX(0.5060881129),    CAL_FIX(-0.6509365173)},    //Band:8(16000Hz)
//    { CAL_FIX(0.5060881129),    CAL_FIX(-0.6509365173)},    //Band:9(16000Hz)
//    { CAL_FIX(0.0284864965),    CAL_FIX(0.9990866674)},     //BASS:300Hz
//    { CAL_FIX(0.2763541250),    CAL_FIX(0.9100351062)},     //TREB:3000Hz
//};
//
//void eq_index_init(void)
//{
//    int i;
//    for (i = 0; i < 12; i++) {
//        eq_cfg.msc_coef[i] = &tbl_coef[i];
//    }
//    for (i = 0; i < 12; i++) {
//        if (i == 10) {           //bass
//            bass_treble_coef_cal(eq_cfg.msc_coef[i], 0, 0);
//        } else if (i == 11) {    //treble
//            bass_treble_coef_cal(eq_cfg.msc_coef[i], 0, 1);
//        } else {                    //eq
//            eq_coef_cal(eq_cfg.msc_coef[i], 0);
//        }
//    }
//}
//
////根据EQ号来设置EQ[11:0]
//void music_set_eq_for_index(u8 index, int gain)
//{
//    if (index > 12 || gain < -12 || gain > 12) {
//        return;
//    }
//    if (index == 10) {           //bass
//        bass_treble_coef_cal(eq_cfg.msc_coef[index], gain, 0);
//    } else if (index == 11) {   //treble
//        bass_treble_coef_cal(eq_cfg.msc_coef[index], gain, 1);
//    } else {                    //eq
//        eq_coef_cal(eq_cfg.msc_coef[index], gain);
//    }
//
//    music_set_eq_by_num(sys_cb.eq_mode);        //更新参数
//}

