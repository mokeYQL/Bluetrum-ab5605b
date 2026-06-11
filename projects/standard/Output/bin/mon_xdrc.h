#ifndef _MON_XDRC_H 
#define _MON_XDRC_H

#define BUF_OFS_CFG_PRE_EQ      		(0x00000016)
#define BUF_LEN_OFS_CFG_PRE_EQ  		(0x00000012)

#define BUF_OFS_CFG_POST_EQ     		(0x0000002A)
#define BUF_LEN_OFS_CFG_POST_EQ 		(0x00000026)

#define BUF_OFS_CFG_DRC_LPHP    		(0x0000003E)
#define BUF_LEN_OFS_CFG_DRC_LPHP		(0x0000003A)

#define BUF_OFS_CFG_DYEQ_1      		(0x00000052)
#define BUF_LEN_OFS_CFG_DYEQ_1  		(0x0000004E)

#define BUF_OFS_CFG_DYEQ_2      		(0x00000066)
#define BUF_LEN_OFS_CFG_DYEQ_2  		(0x00000062)

#define BUF_OFS_CFG_DRC_ALL     		(0x0000007A)
#define BUF_LEN_OFS_CFG_DRC_ALL 		(0x00000076)

#define BUF_OFS_CFG_VOL         		(0x0000008E)
#define BUF_LEN_OFS_CFG_VOL     		(0x0000008A)

#define ALL_MODE_NAME_SUM		0x00000D0C


//EFFECT_MODE_IDX
enum {
	CFG_PRE_EQ = 1,
	CFG_POST_EQ,
	CFG_DRC_LPHP,
	CFG_DYEQ_1,
	CFG_DYEQ_2,
	CFG_DRC_ALL,
	CFG_VOL,
	CFG_MAX,
};

typedef struct {
	char effect_cfg_name[12];
	u32  effect_res_offset;
	u32  effect_len_offset;
} effect_info_cfg_t;

extern const effect_info_cfg_t effect_info[CFG_MAX];

typedef struct {
	void (*effect_update_callback)(u8 *buf, u32 len, u8 params);//0:初始化 1:更新
} effect_update_callback_t;

extern const effect_update_callback_t effect_update_callback_tbl[CFG_MAX];

#endif