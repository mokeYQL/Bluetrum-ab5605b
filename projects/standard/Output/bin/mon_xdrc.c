#include "include.h"
#include "mon_xdrc.h"

const effect_info_cfg_t effect_info[CFG_MAX] = {
	[CFG_PRE_EQ]   = {"CFG_PRE_EQ##", BUF_OFS_CFG_PRE_EQ,   BUF_LEN_OFS_CFG_PRE_EQ},
	[CFG_POST_EQ]  = {"CFG_POST_EQ#", BUF_OFS_CFG_POST_EQ,  BUF_LEN_OFS_CFG_POST_EQ},
	[CFG_DRC_LPHP] = {"CFG_DRC_LPHP", BUF_OFS_CFG_DRC_LPHP, BUF_LEN_OFS_CFG_DRC_LPHP},
	[CFG_DYEQ_1]   = {"CFG_DYEQ_1##", BUF_OFS_CFG_DYEQ_1,   BUF_LEN_OFS_CFG_DYEQ_1},
	[CFG_DYEQ_2]   = {"CFG_DYEQ_2##", BUF_OFS_CFG_DYEQ_2,   BUF_LEN_OFS_CFG_DYEQ_2},
	[CFG_DRC_ALL]  = {"CFG_DRC_ALL#", BUF_OFS_CFG_DRC_ALL,  BUF_LEN_OFS_CFG_DRC_ALL},
	[CFG_VOL]      = {"CFG_VOL#####", BUF_OFS_CFG_VOL,      BUF_LEN_OFS_CFG_VOL},
};


/*模块初始化更新回调注册表 cv到应用层使用
const effect_update_callback_t effect_update_callback_tbl[CFG_MAX] = {
	[CFG_PRE_EQ]   = {NULL},
	[CFG_POST_EQ]  = {NULL},
	[CFG_DRC_LPHP] = {NULL},
	[CFG_DYEQ_1]   = {NULL},
	[CFG_DYEQ_2]   = {NULL},
	[CFG_DRC_ALL]  = {NULL},
	[CFG_VOL]      = {NULL},
};
*/