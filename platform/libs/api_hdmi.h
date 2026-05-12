#ifndef __API_HDMI
#define __API_HDMI

// CEC帧结构体
typedef struct _CEC_FRAME
{
    u8 src_dst;
    u8 opcode;
    s8 arg_len;
    u8 resv;
    u8 arg_buf[16];
} CEC_FRAME_T;

typedef struct
{
    void(*high)(void);
    void(*low)(void);
    void(*input)(void);
    bool(*sta)(void);
    void(*irq_init)(void);
    void(*irq_dis)(void);
    void(*irq_en) (void);
    void(*irq_edge)(void);
    void(*tx_timer_clr)(void);
    u32 (*rx_timer_cnt)(void);
}CEC_IO_FUNC;

enum
{
	CEC_LOGADDR_TV         = 0x00,
	CEC_LOGADDR_RECDEV1    = 0x01,
	CEC_LOGADDR_RECDEV2    = 0x02,
	CEC_LOGADDR_TUNER1     = 0x03,
	CEC_LOGADDR_PLAYBACK1  = 0x04,
	CEC_LOGADDR_AUDSYS     = 0x05,
	CEC_LOGADDR_TUNER2     = 0x06,
	CEC_LOGADDR_TUNER3     = 0x07,
	CEC_LOGADDR_PLAYBACK2  = 0x08,
	CEC_LOGADDR_RECDEV3    = 0x09,
	CEC_LOGADDR_TUNER4     = 0x0A,
	CEC_LOGADDR_PLAYBACK3  = 0x0B,
	CEC_LOGADDR_RESV3      = 0x0C,
	CEC_LOGADDR_RESV4      = 0x0D,
	CEC_LOGADDR_SPECUSE    = 0x0E,
	CEC_LOGADDR_UNREG_BC   = 0x0F
};

#define AUD2TV          ((CEC_LOGADDR_AUDSYS<<4)|CEC_LOGADDR_TV)
#define AUD2BC          ((CEC_LOGADDR_AUDSYS<<4)|CEC_LOGADDR_UNREG_BC)

void cec_lib_init(void);
void cec_lib_exit(void);
void cec_io_func_cfg(CEC_IO_FUNC *p_func); //io_cfg
u8 edid_analyse(u8 *buf, u16*phys_addr); //返回0 表示成功得到phys_addr, 其它值表示失败
void cec_tx_frame(CEC_FRAME_T *frame);       //kick发送后即退出
bool cec_tx_frame_wait(CEC_FRAME_T *frame);  //kick发送后,等待发送完成后才退出,并返回是否收到ack (true表示收到ack)
void cec_rx_port_isr_call_back(u8 isr_sta);
void cec_tx_timer_isr_callback(void);
void cec_rx_start(void);
u8 hdmi_cec_default_process(CEC_FRAME_T *p_frame);
void cec_broadcast_physical_addr(u16 addr_phy);
bool get_cec_frame(CEC_FRAME_T *recv_frame);
void cec_dev_logic_addr_set(u8 logic_addr);
void cec_tx_no_ack_retry_times(u8 times);   //发送未收到ack时, 重发的次数














#endif
