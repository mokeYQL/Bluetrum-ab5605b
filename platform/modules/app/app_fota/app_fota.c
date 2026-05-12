#include "include.h"

#if FOT_EN

#define FOT_DEBUG_EN                    0
#if FOT_DEBUG_EN
#define FOT_DEBUG(...)                  printf(__VA_ARGS__)
#define FOT_DEBUG_R(...)                print_r(__VA_ARGS__)
#else
#define FOT_DEBUG(...)
#define FOT_DEBUG_R(...)
#endif

/**********************************************/
#define FOT_STA_INIT            BIT(0)
#define FOT_STA_START           BIT(1)
#define FOT_STA_PAUSE           BIT(2)

/**********************************************/
#define FOT_FLAG_UPDATE_OK      BIT(0)
#define FOT_FLAG_SYS_RESET      BIT(1)
#define FOT_FLAG_ROLE_SWITCH    BIT(2)
#define FOT_FLAG_APP_CONNECT    BIT(3)
#define FOT_FLAG_CLK_SET        BIT(4)
#define FOT_FLAG_UPDATE_EXIT    BIT(5)

/**********************************************/
#define FOT_NOTIFY_STA          0x90
#define FOT_GET_INFO            0x91
#define FOT_GET_INFO_TLV        0x92
#define FOT_OUT_DATA_START      0xA0
#define FOT_OUT_DATA_CONTINUE   0x20

/**********************************************/
#define FOT_CMD_POS             0
#define FOT_SEQ_POS             1
#define FOT_ADDR_POS            2
#define FOT_DATA_LEN_POS        6
#define DATA_START_POS          10
#define DATA_CONTINUE_POS       2

/**********************************************/
#define FOT_BLOCK_LEN           512     //must be 512


extern bool ble_fot_send_packet(u8 *buf, u8 len);

AT(.com_text.const)
static const u8 fot_auth_data[] = {0xCC, 0xAA, 0x55, 0xEE, 0x12, 0x19, 0xE4};

typedef enum{
    INFO_DEV_VER  = 1,
    INFO_UPDATE_REQ,
    INFO_DEV_FEATURE,
    INFO_DEV_CONNECT_STA,
    INFO_EDR_ADDR,
    INFO_DEV_CHANNEL,
}DEV_INFO_E;


typedef struct __attribute__((packed)){
    u16 dev_ver;
    u16 remote_ver;
    u32 total_len;
    u32 remain_len;
    u32 hash;
    u16 data_pos;
    u8 recv_ok;
    u32 addr;
    u32 tick;
    u8 sys_clk;
    u8 seq;
    u8 remote_seq;
    u8 sta;
    u8 flag;
}fot_s;

static fot_s fot_var AT(.fot_data.com.var);

static u8 fot_data[FOT_BLOCK_LEN] AT(.fot_data.com.data);

static u16 connect_sta;

AT(.text.fot.cache)
void app_fota_write(void *buf, u32 addr, u32 len)
{
    fot_write(buf, addr, len);
}

AT(.text.fot.cache)
u8 app_fota_get_err(void)
{
    return fot_get_err();
}

AT(.text.fot.cache)
void app_fota_verify(void)
{

}

bool app_fota_breakpoint_info_read(void)
{
    return fot_breakpoint_info_read();
}

void app_fota_init(void)
{
    fot_init();
}

u32 app_fota_get_curaddr(void)
{
    return fot_get_curaddr();
}

void app_fota_update_done(void)
{

}

bool app_fota_is_write_done(void)
{
    return is_fot_update_success();
}

AT(.text.fot.cache)
static void fot_sent_proc(u8 *buf,u8 len)
{
    if((fot_var.flag & FOT_FLAG_APP_CONNECT) == 0){
        return;
    }

    FOT_DEBUG("fot tx:");
    FOT_DEBUG_R(buf,len);

#if LE_AB_FOT_EN
    if(ble_is_connect()){
        ble_fot_send_packet(buf,len);
    }else
#endif
    {
        bt_spp_tx(buf, len);
    }
}

AT(.text.fot.update)
static void fot_reply_info_tlv(u8 *buf,u8 len)
{
    u8 read_offset = 0;
    u8 write_offset = 0;
    u8 rsp[32];
    u8 val_len = 0;

    if((buf == NULL) || (len == 0)){
        return;
    }

    rsp[write_offset++] = FOT_GET_INFO_TLV;
    rsp[write_offset++] = fot_var.seq++;

    while(read_offset < len){
        switch(buf[read_offset]){
            case INFO_DEV_VER:
                FOT_DEBUG("INFO_DEV_VER\n");
                val_len = buf[read_offset + 1];
                rsp[write_offset++] = INFO_DEV_VER;
                rsp[write_offset++] = 2;
                rsp[write_offset++] = fot_var.dev_ver & 0xff;
                rsp[write_offset++] = (fot_var.dev_ver >> 8) & 0xff;
            break;

            case INFO_DEV_FEATURE:
                FOT_DEBUG("INFO_DEV_FEATURE\n");
                {
                    u16 dev_ability = 0;
                    val_len = buf[read_offset + 1];
                    rsp[write_offset++] = INFO_DEV_FEATURE;
                    rsp[write_offset++] = 2;
                    rsp[write_offset++] = dev_ability & 0xff;
                    rsp[write_offset++] = (dev_ability >> 8) & 0xff;
                }
            break;

            case INFO_DEV_CONNECT_STA:
                FOT_DEBUG("INFO_DEV_CONNECT_STA\n");
                val_len = buf[read_offset + 1];
                rsp[write_offset++] = INFO_DEV_CONNECT_STA;
                rsp[write_offset++] = 2;
                rsp[write_offset++] = connect_sta & 0xff;
                rsp[write_offset++] = (connect_sta >> 8) & 0xff;
                break;

            default:
                val_len = buf[read_offset + 1];
                break;
        }
        read_offset += (2 + val_len);
    }

    if(write_offset > sizeof(rsp)){
        printf("fot:rsp buf overflow!!!\n");
        while(1);
    }

    fot_sent_proc(rsp,write_offset);
}


AT(.text.fot.update)
static void fot_reply_dev_version(void)
{
    u8 data[5];

    data[0] = FOT_GET_INFO;
    data[1] = fot_var.seq++;
    data[2] = INFO_DEV_VER;
    memcpy(&data[3], &fot_var.dev_ver, 2);
    fot_sent_proc(data, 5);
}

AT(.text.fot.cache)
static void fot_dev_notify_sta(u8 sta)
{
    u8 buf[3];

    buf[0] = FOT_NOTIFY_STA;
    buf[1] = fot_var.seq++;
    buf[2] = sta;

    fot_sent_proc(buf, 3);
}

AT(.text.fot.update)
u8 is_fot_start(void)
{
    return (fot_var.sta & FOT_STA_START) ? 1:0;
}

AT(.text.fot.update)
void bsp_fot_init(void)
{
    u8  dev_version_str[] = SW_VERSION;
    u16 version_temp = 0;

    memset(&fot_var, 0, sizeof(fot_var));

    fot_var.dev_ver = 0x00;
    version_temp = dev_version_str[1]-'0';
    version_temp <<= 12;
    fot_var.dev_ver |= version_temp;

    version_temp = dev_version_str[3]-'0';
    version_temp <<= 8;
    fot_var.dev_ver |= version_temp;

    version_temp = dev_version_str[5]-'0';
    version_temp <<= 4;
    fot_var.dev_ver |= version_temp;

    fot_var.sta = FOT_STA_INIT;

    fot_var.flag |= FOT_FLAG_APP_CONNECT;
}

void bsp_fot_exit(void)
{
    if(fot_var.flag & FOT_FLAG_APP_CONNECT){
        fot_var.sta = 0;
        fot_var.seq = 0;
        fot_var.remote_seq = 0;
        fot_var.flag &= ~FOT_FLAG_APP_CONNECT;

        if(fot_var.sys_clk){
            set_sys_clk(fot_var.sys_clk);
        }
    }
}

void fot_update_pause(void)
{
    if(fot_var.sta & FOT_STA_START){
        fot_var.sta = FOT_STA_PAUSE;
        fot_dev_notify_sta(FOT_UPDATE_PAUSE);
    }
}

void fot_update_continue(void)
{
    if(fot_var.sta & FOT_STA_PAUSE){
        fot_var.sta = FOT_STA_INIT;
        fot_var.total_len = 0;
        fot_var.remain_len = 0;
        fot_var.data_pos = 0;
        fot_var.recv_ok = 0;
        fot_dev_notify_sta(FOT_UPDATE_CONTINUE);
    }
}

u16 bsp_fot_mtu_get(void)
{
    u16 packet_len = 0;
#if LE_AB_FOT_EN
    if(ble_is_connect()){
        packet_len = ble_get_gatt_mtu();
    }else
#endif
    {
        packet_len = 140;    //spp mtu - 6, spp mtu is define 150 in libs
    }

    return packet_len;
}


AT(.text.fot.update)
static void fot_reply_update_request(void)
{
    u16 flash_remote_ver;
    u32 hash;
    u32 addr = 0;
    u32 block_len = FOT_BLOCK_LEN;
    u16 packet_len = 0;
    u8 data[14];
    u8 need_update = 1;

    if(bt_get_status() >= BT_STA_INCOMING){
        need_update = 0;
        fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
        goto fot_req_reply;
    }

#if LE_AB_FOT_EN
    if(ble_is_connect()){
        ble_update_conn_param(12, 0, 400);
    }
#endif

    packet_len = bsp_fot_mtu_get();

    printf("fot_packet_len:%d\n",packet_len);

    app_fota_init();

    fot_var.flag |= FOT_FLAG_CLK_SET;
    fot_var.sta |= FOT_STA_START;

    FOT_DEBUG("hash_val:0x%x\n",fot_var.hash);

    param_fot_remote_ver_read((u8*)&flash_remote_ver);
    param_fot_hash_read((u8*)&hash);

    FOT_DEBUG("flash hash val:0x%x\n",hash);

    if((fot_var.hash != 0xFFFFFFFF) && (flash_remote_ver == fot_var.remote_ver) && (hash == fot_var.hash)){
        if(app_fota_breakpoint_info_read() == true){
            addr = app_fota_get_curaddr();
        }
    }else{
        param_fot_addr_write((u8*)&addr);
        param_fot_remote_ver_write((u8*)&fot_var.remote_ver);
        param_fot_hash_write((u8*)&fot_var.hash);
    }

fot_req_reply:
    data[0] = FOT_GET_INFO;
    data[1] = fot_var.seq++;
    data[2] = INFO_UPDATE_REQ;
    memcpy(&data[3], &addr, 4);
    memcpy(&data[7], &block_len, 4);
    memcpy(&data[11], &packet_len, 2);
    data[13] = need_update;
    fot_sent_proc(data,14);
}

#if LE_AB_FOT_EN
void fot_ble_disconnect_callback(void)
{
    printf("--->fot_ble_disconnect_callback\n");

    fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
}

void fot_ble_connect_callback(void)
{
    printf("--->fot_ble_connect_callback\n");
}
#endif  //LE_AB_FOT_EN

void fot_spp_connect_callback(void)
{
    printf("--->fot_spp_connect_callback\n");
}

void fot_spp_disconnect_callback(void)
{
    printf("--->fot_spp_disconnect_callback\n");

    fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
}

//------------------------------------------------------------------------------
AT(.text.fot.cache)
void fot_recv_proc(u8 *buf, u16 len)
{
    u32 addr;
    u32 recv_data_len;
    u8 cmd;

    if(fot_var.remote_seq != buf[FOT_SEQ_POS]){
        if(memcmp(fot_auth_data, buf, 7)){      //接入码先过掉
            printf("remote seq err:%d,%d\n", fot_var.remote_seq, buf[FOT_SEQ_POS]);
            fot_dev_notify_sta(FOT_ERR_SEQ);
            fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
        }
        return;
    }

    fot_var.remote_seq++;

    if((fot_var.sta & FOT_STA_INIT) == 0){
        return;
    }

    cmd = buf[FOT_CMD_POS];

    switch(cmd){
    case FOT_GET_INFO_TLV:
        fot_reply_info_tlv(&buf[2], len - 2);
        break;

    case FOT_GET_INFO:
        if(buf[2] == INFO_DEV_VER){
            fot_reply_dev_version();
        }else if(buf[2] == INFO_UPDATE_REQ){
            memcpy(&fot_var.remote_ver, &buf[3], 2);
            memcpy(&fot_var.hash, &buf[5], 4);
            fot_reply_update_request();
        }
        break;

    case FOT_OUT_DATA_START:
        if(fot_var.remain_len){
            FOT_DEBUG("--->len err:%d\n",fot_var.remain_len);
            fot_dev_notify_sta(FOT_ERR_DATA_LEN);
            fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
            return;
        }
        recv_data_len = len - DATA_START_POS;
        memcpy(&fot_var.total_len, &buf[FOT_DATA_LEN_POS], 4);
        fot_var.remain_len = fot_var.total_len - recv_data_len;
        memcpy(&addr, &buf[FOT_ADDR_POS], 4);
        memcpy(&fot_data[fot_var.data_pos], &buf[DATA_START_POS], recv_data_len);
        fot_var.data_pos += recv_data_len;
        if(fot_var.remain_len == 0){
            fot_var.recv_ok = 1;
            fot_var.data_pos = 0;
        }
        break;

    case FOT_OUT_DATA_CONTINUE:
        recv_data_len = len - DATA_CONTINUE_POS;
        if(fot_var.remain_len < recv_data_len){
           recv_data_len =  fot_var.remain_len;
        }
        fot_var.remain_len -= recv_data_len;
        memcpy(&fot_data[fot_var.data_pos], &buf[DATA_CONTINUE_POS], recv_data_len);
        fot_var.data_pos += recv_data_len;
        if(fot_var.remain_len == 0){
            fot_var.recv_ok = 1;
            fot_var.data_pos = 0;
        }
        break;
    }
}

AT(.text.fot.cache)
void bsp_fot_process_do(void)
{
    if(fot_var.recv_ok){
        //FOT_DEBUG("--->fot_recv_ok\n");
        fot_var.recv_ok = 0;
        app_fota_write(fot_data, app_fota_get_curaddr(), fot_var.total_len);

        if(app_fota_is_write_done()){
            app_fota_verify();
            if(app_fota_get_err()){
                fot_dev_notify_sta(app_fota_get_err());
                fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
            }else{
                FOT_DEBUG("--->fot update success\n");
                app_fota_update_done();
                fot_var.flag |= (FOT_FLAG_UPDATE_OK | FOT_FLAG_SYS_RESET);
                fot_var.tick = tick_get();
                fot_dev_notify_sta(FOT_UPDATE_DONE);
            }
        }else{
            fot_dev_notify_sta(app_fota_get_err());
            if(app_fota_get_err()){
                fot_var.flag |= FOT_FLAG_UPDATE_EXIT;
            }
        }
    }
}

AT(.text.fot.cache)
void bsp_fot_flag_do(void)
{
    if(fot_var.flag & FOT_FLAG_SYS_RESET){
        if(tick_check_expire(fot_var.tick, 3000)){
            fot_var.flag &= ~FOT_FLAG_SYS_RESET;
            FOT_DEBUG("-->fota update ok,sys reset\n");
            WDT_RST();
        }
    }

    if(fot_var.flag & FOT_FLAG_CLK_SET){
        FOT_DEBUG("--->FOT_FLAG_CLK_SET\n");
        fot_var.sys_clk = get_cur_sysclk();
        set_sys_clk(SYS_120M);
        fot_var.flag &= ~FOT_FLAG_CLK_SET;
    }

    if(fot_var.flag & FOT_FLAG_UPDATE_EXIT){
        FOT_DEBUG("-->FOT_FLAG_UPDATE_EXIT");
        bsp_fot_exit();
        fot_var.flag &= ~FOT_FLAG_UPDATE_EXIT;
    }
}

AT(.text.fot.cache)
void bsp_fot_process(void)
{
    if(fot_var.sta & FOT_STA_START){
        bsp_fot_process_do();
    }

    if(fot_var.flag){
        bsp_fot_flag_do();
    }
}


AT(.com_text.fot)
u8 fot_app_connect_auth(uint8_t *packet, uint16_t size)
{
    if(!fot_var.sta){
        if(size == 7 && !memcmp(fot_auth_data, packet, 7)){     //sizeof(fot_auth_data)
            bsp_fot_init();
        }
    }

    return fot_var.sta;
}

AT(.com_text.fot)
bool bsp_fot_is_connect(void)
{
    return (fot_var.sta > 0);
}


#else
WEAK void fot_update_pause(void)
{

}

WEAK void fot_update_continue(void)
{

}

#endif  //FOT_EN
