depend(0x00090000);

config(LEVEL, 0x0F);
config(SUB, "系统配置", "系统的相关配置");
config(LEVEL, 0x03);
config(LIST, "提示音语言选择", "选择系统的语言", LANG_ID, 4, "英文", "中文", "中英文(出厂默认英文)", "中英文(出厂默认中文)", 0);
config(CHECK, "SD/U盘播放功能", "是否需要SD/UDISK播放功能", FUNC_MUSIC_EN, 1);
config(CHECK, "AUX输入功能", "是否需要AUX功能", FUNC_AUX_EN, 1);
config(CHECK, "FM收音功能", "是否FM收音功能", FUNC_FMRX_EN, 1);
config(CHECK, "POWKEY 10s复位系统", "是否长按POWKEY 10s复位系统，用于防止系统死机", POWKEY_10S_RESET, 0);
config(CHECK, "功放MUTE功能", "是否使能功放MUTE功能", SPK_MUTE_EN, 1);
config(LISTVAL, "功放MUTE控制IO选择", "功放MUTE控制GPIO选择", SPK_MUTE_IO_SEL, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), SPK_MUTE_EN);
config(CHECK, "高MUTE", "是否为高MUTE，否则为低MUTE", HIGH_MUTE, 1, SPK_MUTE_EN);
config(BYTE, "功放MUTE延时(单位5ms)", "功放MUTE的延时控制，防止解MUTE时间不够导致声音不全。", LOUDSPEAKER_UNMUTE_DELAY, 0, 255, 6, SPK_MUTE_EN);
config(LIST, "功放AB/D控制模式", "功放AB/D控制模式选择", AMPABD_TYPE, 2, "独立IO电平控制", "mute脉冲控制", 0);
config(LISTVAL, "功放AB/D控制IO选择", "功放AB/D控制GPIO选择", AMPABD_IO_SEL, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27));
config(LISTVAL, "耳机检测IO选择", "选择耳机检测IO选择", EARPHONE_DET_IOSEL, BIT, 6, 0, 23, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), ("复用SDCLK检测", 28), ("复用SDCMD检测", 29), ("复用PWRKEY检测", 30));
config(LISTVAL, "自动休眠时间", "设置自动休眠时间", SYS_SLEEP_TIME, SHORT, 45, 21, ("不休眠", 0), ("10秒钟后", 10), ("20秒钟后", 20), ("30秒钟后", 30), ("45秒钟后", 45), ("1分钟后", 60), ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600));
config(LISTVAL, "自动关机时间", "设置自动关机时间", SYS_OFF_TIME, 	SHORT, 0, 18, ("不关机", 0), ("30秒钟后", 30), ("1分钟后", 60),  ("2分钟后", 120), ("3分钟后", 180), ("4分钟后", 240), ("5分钟后", 300), ("6分钟后", 360), ("7分钟后", 420), ("8分钟后", 480), ("9分钟后", 540), ("10分钟后", 600), ("15分钟后", 900), ("20分钟后", 1200), ("25分钟后", 1500), ("30分钟后", 1800), ("45分钟后", 2700), ("1小时后", 3600));
config(CHECK, "低电提示", "当电压比较低的时候，会有提示音警告", LOWPOWER_WARNING_EN, 1);
config(LIST, "低电语音提示电压", "选择低电语音提示电压", LPWR_WARNING_VBAT, 10, "2.8V", "2.9V", "3.0V", "3.1V", "3.2V", "3.3V", "3.4V", "3.5V", "3.6V", "3.7V", 4, LOWPOWER_WARNING_EN);
config(LIST, "低电关机电压", "选择低电关机电压", LPWR_OFF_VBAT, 11, "不关机", "2.8V", "2.9V", "3.0V", "3.1V", "3.2V", "3.3V", "3.4V", "3.5V", "3.6V", "3.7V", 2, LOWPOWER_WARNING_EN);
config(BYTE, "低电语音播报周期(秒)", "设置低电语音播报周期(秒)", LPWR_WARNING_PERIOD, 1, 240, 30, LOWPOWER_WARNING_EN);
config(LIST, "音量级数", "选择系统音量级数", VOL_MAX, 4, "16级音量", "30级音量", "32级音量", "50级音量", 0);
config(BYTE, "开机默认音量", "开机默认音量级数", SYS_INIT_VOL, 0, 50, 5);
config(BYTE, "提示音播放音量", "提示音播放音量级数", WARNING_VOLUME, 0, 50, 6);
config(LEVEL, 0x0F);
config(LIST, "OSC基础电容", "同时配置26M晶振OSCI与OSCO负载电容", OSC_BOTH_CAP, 2, "0PF", "6PF", 0);
config(BYTE, "OSCI电容(0.25PF)", "独立配置26M晶振OSCI负载电容，单位0.25PF。OSCI电容大小：n * 0.25PF + 基础电容", OSCI_CAP, 0, 63, 23);
config(BYTE, "OSCO电容(0.25PF)", "独立配置26M晶振OSCO负载电容，单位0.25PF。OSCO电容大小：n * 0.25PF + 基础电容", OSCO_CAP, 0, 63, 23);
config(LEVEL, 0x03);
config(CHECK, "EQ调试（蓝牙串口）", "是否使用SPP调试EQ功能", EQ_DGB_SPP_EN, 0);
config(CHECK, "EQ调试（UART）", "是否使用UART调试EQ功能", EQ_DGB_UART_EN, 0);
config(LISTVAL, "EQ调试串口选择", "选择调试串口的IO", EQ_UART_SEL, BIT, 2, 0, 3, ("PA7", 0), ("PB2", 1), ("PB3", 2), EQ_DGB_UART_EN);

config(SUB, "电源配置", "电源的相关配置");
config(CHECK, "VDDBT省电容", "是否VDDBT省电容，只能LDO模式才能capless", VDDBT_CAPLESS_EN, 1);
config(LISTVAL, "VDDIO电压", "选择VDDIO电压, VDDAUD与VDDIO短接可能需要调VDDIO电压。", VDDIO_SEL, BIT, 4, 7, 12, ("None", 0), ("2.7V", 3), ("2.8V", 4), ("2.9V", 5), ("3.0V", 6), ("3.1V", 7), ("3.2V", 8), ("3.3V", 9), ("3.4V", 10), ("3.5V", 11), ("3.6V", 12), ("3.8V", 13));
config(LISTVAL, "VDDBT电压", "选择VDDBT电压, None表示使用trim值(1.6V)", VDDBT_SEL, BIT, 5, 26, 22, ("None", 0), ("2.2V", 8), ("2.3V", 10), ("2.4V", 12), ("2.5V", 14), ("2.55V", 15), ("2.6V", 16), ("2.65V", 17), ("2.7V", 18), ("2.75V", 19), ("2.8V", 20), ("2.85V", 21), ("2.9", 22), ("2.95V", 23), ("3.0V", 24), ("3.05V", 25), ("3.1V", 26), ("3.15V", 27), ("3.2V", 28), ("3.25V", 29), ("3.3V", 30), ("3.35V", 31));


config(SUB, "DAC配置", "DAC的相关配置");
config(LISTVAL, "DAC声道选择", "选择DAC的输出方式", DAC_SEL, BIT, 4, 1, 6, ("单声道", 0), ("双声道", 1), ("VCMBUF单声道", 2), ("VCMBUF双声道", 3), ("MIX_VCMBUF单声道", 4), ("软件差分单声道", 6));
config(LISTVAL, "VDDDAC电压", "选择VDDDAC电压", DAC_LDOH_SEL, BIT, 4, 3, 4, ("2.5V", 0), ("2.7V", 1), ("2.9V", 2), ("3.0V", 3));
config(CHECK, "省VCM方案", "是否使用内部VCM，SOP16, SSOP24, SSOP28使用内部VCM", DAC_VCM_LESS_EN, 1);
config(CHECK, "省VDDDAC方案", "是否VDDDAC Bypass，内部VDDDAC与VDDIO短接。", DACVDD_BYPASS_EN, 0);
config(LIST, "DAC最大音量", "配置系统最大模拟增益，自动调整模拟音量表。", DAC_MAX_GAIN, 15, "-9DB", "-8DB", "-7DB", "-6DB", "-5DB", "-4DB", "-3DB", "-2DB", "-1DB", "0DB", "+1DB", "+2DB", "+3DB", "+4DB", "+5DB", 9);
config(LIST, "通话最大音量", "配置通话时最大模拟增益，自动调整模拟音量表。", BT_CALL_MAX_GAIN, 15, "-9DB", "-8DB", "-7DB", "-6DB", "-5DB", "-4DB", "-3DB", "-2DB", "-1DB", "0DB", "+1DB", "+2DB", "+3DB", "+4DB", "+5DB", 8);

config(SUB, "充电配置", "充电功能的相关配置");
config(CHECK, "充电使能", "是否打开充电功能", CHARGE_EN, 1);
config(CHECK, "涓流充电使能", "是否使能涓流充电", CHARGE_TRICK_EN, 1, CHARGE_EN);
config(CHECK, "插入DC复位系统", "是否插入DC充电复位系统，用于防止系统死机", CHARGE_DC_RESET, 1, CHARGE_EN);
config(CHECK, "插入DC禁止软开机", "PWRKEY软开机，DC Online时，禁止软开机", CHARGE_DC_NOT_PWRON, 1, CHARGE_EN);
config(LISTVAL, "充电截止电流", "选择充电截止电流", CHARGE_STOP_CURR, BIT, 3, 2, 8, ("2.5mA", 0), ("5mA", 1), ("10mA", 2), ("15mA", 3), ("20mA", 4), ("25mA", 5), ("30mA", 6), ("35mA", 7), CHARGE_EN);
config(LISTVAL, "恒流充电电流", "恒流充电（电池电压大于2.9v）电流", CHARGE_CONSTANT_CURR, BIT, 4, 3, 16, ("10mA", 0), ("35mA", 1), ("55mA", 2), ("80mA", 3), ("100mA", 4), ("125mA", 5), ("150mA", 6), ("170mA", 7), ("195mA", 8), ("220mA", 9), ("240mA", 10), ("265mA", 11), ("290mA", 12), ("310mA", 13), ("330mA", 14), ("350mA", 15), CHARGE_EN);
config(LISTVAL, "涓流充电电流", "涓流充电（电池电压小于2.9v）电流", CHARGE_TRICKLE_CURR, BIT, 2, 2, 4, ("10mA", 0), ("20mA", 1), ("30mA", 2), ("40mA", 3), CHARGE_TRICK_EN);
config(LISTVAL, "充满电蓝灯亮", "设置充满电蓝灯亮时间", CHARGE_FULL_BLED, BIT, 3, 3, 8,  ("不亮蓝灯", 0), ("亮10秒", 1), ("亮20秒", 2), ("亮30秒", 3), ("1分钟", 4), ("2分钟", 5), ("3分钟", 6), ("常亮", 7), CHARGE_EN);

config(LEVEL, 0x0F);
config(SUB, "蓝牙配置", "蓝牙的相关配置");
config(TEXT, "蓝牙名称", "手机上可以看到的蓝牙名称", BT_NAME, 32, "BT-BOX");
config(MAC, "蓝牙地址", "蓝牙的MAC地址", BT_ADDR, 6, 41:42:00:00:00:00, 41:42:00:00:00:FF, 41:42:00:00:00:01);
config(LEVEL, 0x00);
config(LIST, "预置RF发射功率", "配置预置RF发射功率", BT_TXPWR, 9, "+4DBM", "+3DBM", "+2DBM", "+1DBM", "0DBM", "-1DBM", "-2DBM", "-3DBM", "-4DBM", 0);
config(LEVEL, 0x03);
config(LISTVAL, "预置RF参数选择", "影响发射、接收质量", BT_RF_PARAM, BIT, 3, 1, 6, ("DEFAULT_2DB", 0), ("DEFAULT_4DB", 1),("DEFAULT_6DB", 2),("SOP16_2DB", 3), ("SOP16_4DB", 4),("SOP16_6DB", 5));
config(CHECK, "是否自定义RF参数", "选用自定义RF参数后，预置RF参数无效", BT_USER_PARAM_EN, 0);
config(LEVEL, 0x00);
config(BYTE, "PA_GAIN",  	"PA_GAIN, 参考值7",      	RF_PA_GAIN,  	0, 7,  7,  BT_USER_PARAM_EN);
config(LEVEL, 0x03);
config(BYTE, "MIX_GAIN", 	"MIX_GAIN, 参考值1~4",   	RF_MIX_GAIN, 	1, 7,  2,  BT_USER_PARAM_EN);
config(BYTE, "DIG_GAIN", 	"DIG_GAIN, 参考值30~59", 	RF_DIG_GAIN, 	0, 61, 59, BT_USER_PARAM_EN);
config(LEVEL, 0x00);
config(BYTE, "PA_AUX_GAIN", "PA_AUX_GAIN, 参考值5~7", 	PA_AUX_GAIN,  	0, 7,   7,  BT_USER_PARAM_EN);
config(LEVEL, 0x03);
config(BYTE, "GL_PA_CAP",   "PA_CAP, 参考值0~15",       RF_PA_CAP,     	0, 15,  0, BT_USER_PARAM_EN);
config(BYTE, "GL_MIX_CAP",  "MIX_CAP, 参考值7~9",       RF_MIX_CAP,    	7, 9,  8, BT_USER_PARAM_EN);
config(LEVEL, 0x00);
config(BYTE, "PA_VCASN2", 	"DIG_GAIN, 参考值4", 		RF_PA_VCASN2, 	0, 7, 	4,  BT_USER_PARAM_EN);
config(BYTE, "PA_VCASN", 	"PA_AUX_GAIN, 参考值4", 	PA_PA_VCASN,  	0, 7,  	4,  BT_USER_PARAM_EN);
config(BYTE, "PA_NBIAS",	"PA_CAP, 参考值4或6",        RF_PA_NBIAS,    0, 31,  4, BT_USER_PARAM_EN);
config(BYTE, "PA_AUX_NBIAS", "MIX_CAP, 参考值2~3",      RF_PA_AUX_NBIAS,0, 31,  2,  BT_USER_PARAM_EN);
config(BYTE, "GL_TX_DBM",     "Cable实测dbm值",           RF_TXDBM,      0, 12, 6, BT_USER_PARAM_EN);
config(LEVEL, 0x03);
config(LISTVAL, "降低回连TXPWR", "是否降低回连TXPWR", BT_RF_TXPWR_RECON, BIT, 3, 0, 4, ("不降低", 0), ("降低3dbm", 1), ("降低6dbm", 2), ("降低9dbm", 3));
config(CHECK, "连接两部手机功能", "是否支持连接两部手机功能", BT_2ACL_EN, 1);
config(CHECK, "音乐播放功能", "是否支持蓝牙音乐播放功能", BT_A2DP_EN, 1);
config(CHECK, "音乐音量同步", "是否支持A2DP音量与手机同步", BT_A2DP_VOL_CTRL_EN, 1, BT_A2DP_EN);
config(CHECK, "通话功能", "是否支持蓝牙通话的功能", BT_SCO_EN, 1);
config(CHECK, "私密接听功能", "是否使用手动私密接听（切换到手机端接听）", BT_HFP_PRIVATE_EN, 1, BT_SCO_EN);
config(CHECK, "来电报号功能", "是否支持来电报号功能", BT_HFP_RING_NUMBER_EN, 1, BT_SCO_EN);
config(CHECK, "来电播放手机铃声", "打开此功能后，可以播放手机铃声，例如ios", BT_HFP_PLAY_IOS_RING_EN, 0, BT_SCO_EN);
config(CHECK, "串口功能", "是否支持蓝牙串口的功能", BT_SPP_EN, 1);
config(CHECK, "拍照功能", "是否支持蓝牙HID拍照的功能", BT_HID_EN, 1);
config(CHECK, "按键HID连接/断开功能", "是否支持蓝牙HID服务，按键手动连接/断开", BT_HID_MENU_EN, 1, BT_HID_EN);
config(CHECK, "HID默认不连接", "HID服务默认不连接，需要按键手动连接", BT_HID_DISCON_DEFAULT_EN, 0, BT_HID_EN);
config(CHECK, "HID独立自拍器模式", "是否支持独立自拍器模式功能", FUNC_BTHID_EN, 1);
config(TEXT, "HID蓝牙名称", "手机上可以看到的独立自拍器模式蓝牙名称", BTHID_NAME, 32, "BT-Photo", FUNC_BTHID_EN);
config(CHECK, "TWS功能", "是否支持TWS功能，打开后<连接两部手机功能>无效", BT_TWS_EN, 0);
config(LISTVAL, "TWS配对方式选择", "选择TWS配对方式", BT_TWS_PAIR_MODE, BIT, 3, 0, 4, ("自动配对", 0), ("自定义配对<调用api>", 1), ("双击PLAY键配对", 2), ("长按MODE键配对", 3), BT_TWS_EN);
config(LISTVAL, "TWS声道分配选择", "选择TWS声道分配方式", BT_TWS_LR_MODE, BIT, 4, 1, 7, ("不分配，主副均双声道输出", 0), ("自动分配，主右声道副左声道", 1), ("自动分配，主左声道副右声道", 2), ("PWRKEY,有820K接地为左", 8), ("GPIOx有接地为左声道", 9), ("配置选择为左声道", 10), ("配置选择为右声道", 11), BT_TWS_EN);
config(LISTVAL, "TWS声道GPIOx", "GPIOx有接地为左声道", TWS_SEL_LEFT_GPIO_SEL, BIT, 5, 0, 21, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5/WKO", 26), ("PE0", 14), ("PE4", 18), ("PE5", 19), ("PE6", 20), ("PE7", 21), ("PF0", 22), ("PF1", 23), ("PF2", 24), ("PF3", 25), BT_TWS_EN);
config(CHECK, "TWS蓝牙名加L或R后缀", "是否使能TWS蓝牙名称加L或R后缀, 需要GPIOx固定左右耳", BT_TWS_NAME_SUFFIX_EN, 0, BT_TWS_EN);
config(CHECK, "BLE控制功能", "是否支持BLE音乐控制的功能", BLE_EN, 0);
config(TEXT, "BLE名称", "手机上可以看到的BLE蓝牙名称", LE_NAME, 32, "LE-Remoter", BLE_EN);

config(SUB, "MIC参数", "蓝牙和Karaok的MIC参数");
config(LIST, "MIC通路选择", "选择MIC通路", BT_CH_MIC, 3, "None", "MIC_PF2", "MIC_PF3", 1);
config(LISTVAL, "MIC偏置电路配置", "MIC电路BIAS配置，省电容，省电阻配置", MIC_BIAS_METHOD, BIT, 2, 1, 3, ("None", 0), ("MIC省电阻电容", 1), ("MIC只省电阻", 2));
config(BIT, "MIC内部偏置电阻大小", "MIC只省电阻, PE7给MIC供电方案才有效", MIC_BIAS_RESISTOR, 4, 0, 15, 3);
config(BYTE, "MIC模拟增益", "MIC模拟增益配置(3DB ~ +42DB), Step 3DB", BT_ANL_GAIN, 0, 13, 12);
config(BYTE, "MIC数字增益(0~32DB)", "MIC SDADC数字增益, Step 0.5DB", BT_DIG_GAIN, 0, 63, 0);
config(BIT, "MIC后置数字增益", "调节算法后置MIC数字增益", MIC_POST_GAIN, 4, 0, 15, 0, BT_SCO_EN);
config(LISTVAL, "MIC插入检测配置", "选择MIC检测GPIO，或Disable MIC检测", MIC_DET_IOSEL, BIT, 6, 0, 23, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), ("复用SDCLK检测", 28), ("复用SDCMD检测", 29), ("复用PWRKEY检测", 30));

config(SUB, "通话参数", "蓝牙的通话参数", BT_SCO_EN);
config(NUM, "ANS噪声阈值", "ANS噪声阈值(低于此阈值便认为是噪声)", BT_NOISE_THRESHOID, 0, 65535, 600, BT_SCO_EN);
config(BIT, "ANS降噪强度", "ANS降噪深度, 噪声衰减多少DB, 值越大降噪越猛。", BT_SCO_ANS_RADIO, 5, 0, 31, 30, BT_SCO_EN);
config(CHECK, "AEC功能", "是否支持AEC功能，AEC/ALC只能二选一", BT_AEC_EN, 1, BT_SCO_EN);
config(BIT, "AEC回声消除级别", "回声消除级别(级别越高，回声衰减越明显，但通话效果越差)", BT_ECHO_LEVEL, 4, 0, 15, 2, BT_AEC_EN);
config(BYTE, "AEC远端补偿值", "远端补偿值(0~255)", BT_FAR_OFFSET, 0, 255, 114, BT_AEC_EN);
config(CHECK, "ALC功能", "是否支持ALC功能，AEC/ALC只能二选一", BT_ALC_EN, 0, BT_SCO_EN);
config(LEVEL, 0x03);
config(LIST, "ALC淡入延时", "ALC近端延时多少时间才淡入", BT_ALC_IN_DELAY, 6, "16ms", "64ms", "112ms", "160ms", "224ms", "288ms", 3, BT_ALC_EN);
config(LIST, "ALC淡入速度", "ALC近端淡入速度", BT_ALC_IN_STEP, 6, "4ms", "8ms", "16ms", "32ms", "64ms", "128ms", 4, BT_ALC_EN);
config(LIST, "ALC淡出延时", "ALC近端延时多少时间才淡出", BT_ALC_OUT_DELAY, 6, "16ms", "64ms", "112ms", "160ms", "224ms", "288ms", 0, BT_ALC_EN);
config(LIST, "ALC淡出速度", "ALC近端淡出速度", BT_ALC_OUT_STEP, 6, "4ms", "8ms", "16ms", "32ms", "64ms", "128ms", 0, BT_ALC_EN);
config(LEVEL, 0x03);

config(SUB, "音乐配置", "音乐播放模式的相关配置", FUNC_MUSIC_EN);
config(CHECK, "WAV解码", "是否支持WAV格式解码", MUSIC_WAV_SUPPORT, 1, FUNC_MUSIC_EN);
config(CHECK, "WMA解码", "是否支持WMA格式解码", MUSIC_WMA_SUPPORT, 1, FUNC_MUSIC_EN);
config(CHECK, "APE解码", "是否支持APE格式解码", MUSIC_APE_SUPPORT, 1, FUNC_MUSIC_EN);
config(CHECK, "FLAC解码", "是否支持FLAC格式解码", MUSIC_FLAC_SUPPORT, 1, FUNC_MUSIC_EN);
config(CHECK, "SDCARD播放功能", "是否支持SD/MMC卡播放功能", MUSIC_SDCARD_EN, 1, FUNC_MUSIC_EN);
config(CHECK, "UDISK播放功能", "是否支持UDISK播放功能", MUSIC_UDISK_EN, 1, FUNC_MUSIC_EN);
config(LISTVAL, "SDCARD检测IO选择", "选择SDCARD检测GPIO", SDDET_IOSEL, BIT, 6, 0, 24, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), ("复用SDCLK检测", 28), ("复用SDCMD检测", 29), ("复用PWRKEY检测", 30),("软件检测", 31), MUSIC_SDCARD_EN);
config(LISTVAL, "SDCARD1检测IO选择", "选择SDCARD1检测GPIO(双SD卡, 默认宏关闭的)", SD1DET_IOSEL, BIT, 6, 0, 23, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), ("复用SDCLK检测", 28), ("复用SDCMD检测", 29), ("复用PWRKEY检测", 30), MUSIC_SDCARD_EN);

config(SUB, "FM配置", "FM收音参数的相关配置");
config(BYTE, "FMRX_THD_R_VAL", "FM搜台参数:RSSI阈值,大于该阈值才会被判断为真台,该值越小台越多(默认值128)", FMRX_R_VAL,  0, 255, 128);
config(BIT, "FMRX_THD_Z_VAL", "FM搜台参数:音质Z阈值,小于该阈值才会判断为真台,该值越大台越多(默认值1100).", FMRX_Z_VAL,  13, 0, 8191, 1100);
config(BIT, "FMRX_THD_FZ_VAL","FM搜台参数:音质FZ阈值,小于该阈值才会判断为真台,该值越大台越多(默认值600).", FMRX_FZ_VAL, 13, 0, 8191, 600);
config(BIT, "FMRX_THD_D_VAL", "FM搜台参数:音质D阈值,真台一般在3000内,此参数一般不用更改.该值越大台越多(默认值3000).", FMRX_D_VAL, 13, 0, 8191, 3000);
config(CHECK, "固定CS_FILTER", "FM指标测试需要固定CS_FILTER,避免自动切换带来其它影响,正常使用一般关该功能,让弱台自动使用小一些的CS_FILER杂音更小", FMRX_CS_FILTER_FIXED, 1);
config(LISTVAL, "CS_FILTER", "CS_FILTER选择", FMRX_CS_FILTER_SEL, BIT, 2, 1, 4, ("125K", 0), ("75K", 1), ("50K", 2), ("25K", 3)),FMRX_CS_FILTER_FIXED);
config(LISTVAL, "FM声道选则", "FM声道输出声道,一般单声道比双声道噪音会小些", FMRX_AUDIO_CH, BIT, 2, 0, 2, ("单声道", 0), ("双声道", 1));

config(SUB, "AUX配置", "AUX模式的相关配置", FUNC_AUX_EN);
config(CHECK, "AUX通路进ADC", "AUX是否先通过ADC采样,否则直通DAC。进ADC可以调AUX EQ,及AUX录音等功能。", AUX_2_SDADC_EN, 1, FUNC_AUX_EN);
config(CHECK, "插入LINEIN关机", "是否使能插入LINEIN, 直接关机功能，大耳包功能", LINEIN_2_PWRDOWN_EN, 0, FUNC_AUX_EN);
config(CHECK, "模式切换进AUX模式", "有LINEIN检测时，LINEIN线没有插入，是否能通过模式转换进入AUX模式。", MODE_2_AUX_EN, 0, FUNC_AUX_EN);
config(LISTVAL, "AUX模拟增益", "AUX模拟增益选择", AUX_ANL_GAIN, BIT, 4, 2, 11, ("+6DB", 0), ("+3DB", 1), ("0DB", 2), ("-3DB", 3), ("-6DB", 4), ("-9DB", 5), ("-15DB", 6), ("-21DB", 7), ("-27DB", 8), ("-33DB", 9), ("-39DB", 10), FUNC_AUX_EN);
config(BIT, "AUX数字增益(0~32DB)", "SDADC数字增益, Step 0.5DB", AUX_DIG_GAIN, 5, 0, 31, 0, AUX_2_SDADC_EN);
config(LISTVAL, "AUXL通路选择", "AUX左声道通路选择", AUXL_SEL, BIT, 4, 0, 9, ("无AUXL输入", 0), ("AUXL_PA6", 1), ("AUXL_PB1", 2), ("AUXL_PE6", 3), ("AUXL_PF0", 4), ("AUXL_VCMBUF", 5), ("AUXL_MIC_PF2", 6), ("AUXL_MIC_PF3", 7), ("AUXL_VOUTRP", 8), FUNC_AUX_EN);
config(LISTVAL, "AUXR通路选择", "AUX右声道通路选择", AUXR_SEL, BIT, 4, 0, 9, ("无AUXR输入", 0), ("AUXR_PA7", 1), ("AUXR_PB2", 2), ("AUXR_PE7", 3), ("AUXR_PF1", 4), ("AUXR_VCMBUF", 5), ("AUXR_MIC_PF2", 6), ("AUXL_MIC_PF3", 7), ("AUXR_VOUTRP", 8), FUNC_AUX_EN);
config(LISTVAL, "LINEIN插入检测配置", "选择LINEIN检测GPIO，或Disable LINEIN检测", LINEIN_DET_IOSEL, BIT, 6, 0, 24, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), ("复用SDCLK检测", 28), ("复用SDCMD检测", 29), ("复用PWRKEY检测", 30), ("VUSB", 32), FUNC_AUX_EN);

config(SUB, "按键配置", "选择按键及按键功能配置");
config(CHECK, "PWRKEY功能", "是否使用PWRKEY，使用软开关机时必须使能", USER_PWRKEY_EN, 0);
config(CHECK, "ADKEY1功能", "是否使用第一组ADKEY", USER_ADKEY_EN, 1);
config(CHECK, "ADKEY2功能", "是否使用第二组ADKEY", USER_ADKEY2_EN, 0);
config(CHECK, "IOKEY功能", "是否使用IOKEY", USER_IOKEY_EN, 0);
config(CHECK, "复用SDCLK的ADKEY", "是否使用复用SDCLK的ADKEY按键", USER_ADKEY_MUX_SDCLK_EN, 0);
config(CHECK, "按键多击功能", "是否使能按键多击（2/3/4/5击）功能", USER_KEY_MULTI_PRESS_EN, 1);
config(LISTVAL, "双击响应时间选择", "选择双击按键响应间隔时间", DOUBLE_KEY_TIME, BIT, 3, 1, 8, ("200ms", 0), ("300ms", 1), ("400ms", 2), ("500ms", 3), ("600ms", 4), ("700ms", 5), ("800ms", 6), ("900ms", 7), USER_KEY_MULTI_PRESS_EN);
config(LIST, "ADKEY1通路选择", "第一组ADKEY io mapping select", ADKEY_CH, 11, "ADCCH_PA5", "ADCCH_PA6", "ADCCH_PA7", "ADCCH_PB1", "ADCCH_PB2", "ADCCH_PB3", "ADCCH_PB4", "ADCCH_PE5", "ADCCH_PE6", "ADCCH_PE7", "ADCCH_PF2", 9, USER_ADKEY_EN);
config(LIST, "ADKEY2通路选择", "第二组ADKEY io mapping select", ADKEY2_CH, 11, "ADCCH_PA5", "ADCCH_PA6", "ADCCH_PA7", "ADCCH_PB1", "ADCCH_PB2", "ADCCH_PB3", "ADCCH_PB4", "ADCCH_PE5", "ADCCH_PE6", "ADCCH_PE7", "ADCCH_PF2", 8, USER_ADKEY2_EN);
config(LISTVAL, "软开机长按时间选择", "长按PWRKEY多长时间后软开机", PWRON_PRESS_TIME,  BIT, 3, 3, 8, ("0.1秒", 0), ("0.5秒", 1), ("1秒", 2), ("1.5秒", 3), ("2秒", 4), ("2.5秒", 5), ("3秒", 6), ("3.5秒", 7));
config(LISTVAL, "软关机长按时间选择", "长按PWRKEY多长时间后软关机", PWROFF_PRESS_TIME, BIT, 3, 3, 8, ("1.5秒", 0), ("2秒", 1), ("2.5秒", 2), ("3秒", 3), ("3.5秒", 4), ("4秒", 5), ("4.5秒", 6), ("5秒", 7));

config(CHECK, "支持多击的按键定制", "是否配置支持多击的按键，不勾选使用SDK默认的双击按键配置列表", KEY_MULTI_CONFIG_EN, 0, USER_KEY_MULTI_PRESS_EN);
config(LIST, "多击按键1选择", "支持多击第1个按键", DBLKEY_NUM0, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 1, KEY_MULTI_CONFIG_EN);
config(LIST, "多击按键2选择", "支持多击第2个按键", DBLKEY_NUM1, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 2, KEY_MULTI_CONFIG_EN);
config(LIST, "多击按键3选择", "支持多击第3个按键", DBLKEY_NUM2, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 5, KEY_MULTI_CONFIG_EN);
config(LIST, "多击按键4选择", "支持多击第4个按键", DBLKEY_NUM3, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 7, KEY_MULTI_CONFIG_EN);
config(LIST, "多击按键5选择", "支持多击第5个按键", DBLKEY_NUM4, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 14, KEY_MULTI_CONFIG_EN);

config(CHECK, "PWRKEY按键定制", "是否配置PWRKEY按键功能，不勾选使用SDK默认按键配置", PWRKEY_CONFIG_EN, 0, USER_PWRKEY_EN);
config(LIST, "PWRKEY按键1功能", "PWRKEY第1个按键功能定制", PWRKEY_NUM0, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 1, PWRKEY_CONFIG_EN);
config(LIST, "PWRKEY按键2功能", "PWRKEY第2个按键功能定制", PWRKEY_NUM1, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 7, PWRKEY_CONFIG_EN);
config(LIST, "PWRKEY按键3功能", "PWRKEY第3个按键功能定制", PWRKEY_NUM2, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 5, PWRKEY_CONFIG_EN);
config(LIST, "PWRKEY按键4功能", "PWRKEY第4个按键功能定制", PWRKEY_NUM3, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 12, PWRKEY_CONFIG_EN);
config(LIST, "PWRKEY按键5功能", "PWRKEY第5个按键功能定制", PWRKEY_NUM4, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 14, PWRKEY_CONFIG_EN);

config(CHECK, "ADKEY1按键定制", "是否配置第一组ADKEY按键功能，不勾选使用SDK默认按键配置", ADKEY_CONFIG_EN, 0);
config(LIST, "ADKEY1按键1功能", "ADKEY1第1个按键功能定制", ADKEY1_NUM0, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 2, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键2功能", "ADKEY1第2个按键功能定制", ADKEY1_NUM1, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 11, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键3功能", "ADKEY1第3个按键功能定制", ADKEY1_NUM2, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 10, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键4功能", "ADKEY1第4个按键功能定制", ADKEY1_NUM3, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 12, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键5功能", "ADKEY1第5个按键功能定制", ADKEY1_NUM4, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 14, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键6功能", "ADKEY1第6个按键功能定制", ADKEY1_NUM5, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 9, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键7功能", "ADKEY1第7个按键功能定制", ADKEY1_NUM6, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 8, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键8功能", "ADKEY1第8个按键功能定制", ADKEY1_NUM7, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 15, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键9功能", "ADKEY1第9个按键功能定制", ADKEY1_NUM8, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 16, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键10功能", "ADKEY1第10个按键功能定制", ADKEY1_NUM9, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 7, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键11功能", "ADKEY1第11个按键功能定制", ADKEY1_NUM10, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 5, ADKEY_CONFIG_EN);
config(LIST, "ADKEY1按键12功能", "ADKEY1第12个按键功能定制", ADKEY1_NUM11, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 0, ADKEY_CONFIG_EN);

config(CHECK, "ADKEY2按键定制", "是否配置第二组ADKEY，不勾选使用SDK默认按键配置", ADKEY2_CONFIG_EN, 0, USER_ADKEY2_EN);
config(LIST, "ADKEY2按键1功能", "ADKEY2第1个按键功能定制", ADKEY2_NUM0, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 2, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键2功能", "ADKEY2第2个按键功能定制", ADKEY2_NUM1, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 11, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键3功能", "ADKEY2第3个按键功能定制", ADKEY2_NUM2, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 10, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键4功能", "ADKEY2第4个按键功能定制", ADKEY2_NUM3, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 12, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键5功能", "ADKEY2第5个按键功能定制", ADKEY2_NUM4, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 14, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键6功能", "ADKEY2第6个按键功能定制", ADKEY2_NUM5, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 9, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键7功能", "ADKEY2第7个按键功能定制", ADKEY2_NUM6, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 8, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键8功能", "ADKEY2第8个按键功能定制", ADKEY2_NUM7, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 15, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键9功能", "ADKEY2第9个按键功能定制", ADKEY2_NUM8, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 16, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键10功能", "ADKEY2第10个按键功能定制", ADKEY2_NUM9, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 7, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键11功能", "ADKEY2第11个按键功能定制", ADKEY2_NUM10, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 5, ADKEY2_CONFIG_EN);
config(LIST, "ADKEY2按键12功能", "ADKEY2第12个按键功能定制", ADKEY2_NUM11, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 0, ADKEY2_CONFIG_EN);

config(CHECK, "IOKEY按键定制", "是否配置IOKEY，不勾选使用SDK默认按键配置", IOKEY_CONFIG_EN, 0, USER_IOKEY_EN);
config(LISTVAL, "IOKEY按键1的IO", "IOKEY第1个按键的GPIO口选择", IOKEY_IO0, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键2的IO", "IOKEY第2个按键的GPIO口选择", IOKEY_IO1, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键3的IO", "IOKEY第3个按键的GPIO口选择", IOKEY_IO2, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键4的IO", "IOKEY第4个按键的GPIO口选择", IOKEY_IO3, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), IOKEY_CONFIG_EN);
config(LISTVAL, "IOKEY按键5的IO", "IOKEY第5个按键的GPIO口选择", IOKEY_IO4, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), IOKEY_CONFIG_EN);

config(LIST, "IOKEY按键1功能", "IOKEY第1个按键功能定制", IOKEY_NUM0, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 2, IOKEY_CONFIG_EN);
config(LIST, "IOKEY按键2功能", "IOKEY第2个按键功能定制", IOKEY_NUM1, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 6, IOKEY_CONFIG_EN);
config(LIST, "IOKEY按键3功能", "IOKEY第3个按键功能定制", IOKEY_NUM2, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 4, IOKEY_CONFIG_EN);
config(LIST, "IOKEY按键4功能", "IOKEY第4个按键功能定制", IOKEY_NUM3, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 12, IOKEY_CONFIG_EN);
config(LIST, "IOKEY按键5功能", "IOKEY第5个按键功能定制", IOKEY_NUM4, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 14, IOKEY_CONFIG_EN);
config(LIST, "两IO中间的按键功能", "IOKEY1与IOKEY2两IO中间的按键", IOKEY_MIDKEY_NUM, 23, "None", "PP/POWER", "P/P", "POWER", "NEXT/VOL+", "VOL+/NEXT", "PREV/VOL-", "VOL-/PREV", "VOL+", "VOL-", "NEXT", "PREV", "MODE", "PP/MODE", "HSF", "EQ", "REPEAT", "MUTE", "MODE/POWER", "REC", "PLAY/HSF", "A-B", "DUT_MODE", 16, IOKEY_CONFIG_EN);

config(SUB, "LED灯配置", "选择及配置LED状态指示灯");
config(CHECK, "系统指示灯(蓝灯)", "是否使用系统状态指示灯", LED_DISP_EN, 1);
config(CHECK, "电源状态灯(红灯)", "是否使用充电/电源指示灯", LED_PWR_EN, 1);
config(CHECK, "电池低电闪灯", "电池低电时，红灯或蓝灯是否闪烁？", RLED_LOWBAT_EN, 0, LED_DISP_EN);
config(CHECK, "省电阻1个IO推两个灯", "省电阻1个IO推两个灯，不能兼容升级功能", PORT_2LED_RESLESS_EN, 1, LED_DISP_EN);
config(LISTVAL, "蓝灯IO选择", "蓝灯的GPIO口选择", BLED_IO_SEL, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), LED_DISP_EN);
config(LISTVAL, "红灯IO选择", "红灯的GPIO口选择", RLED_IO_SEL, BIT, 5, 0, 20, ("None", 0), ("PA3", 4), ("PA4", 5), ("PA5", 6), ("PA6", 7), ("PA7", 8), ("PB0", 9), ("PB1", 10), ("PB2", 11), ("PB3", 12), ("PB4", 13), ("PB5", 14), ("PE0", 16), ("PE5", 21), ("PE6", 22), ("PE7", 23), ("PF0", 24), ("PF1", 25), ("PF2", 26), ("PF3", 27), LED_PWR_EN);

config(CHECK, "开机状态配置LED", "是否配置开机LED指示灯状态", LED_PWRON_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "开机", "配置开机状态下的闪灯状态", LED_POWERON, 0x02, 0x01, 10, 255, 		LED_PWRON_CONFIG_EN);

config(CHECK, "关机状态配置LED", "是否配置等待关机状态指示灯", LED_PWROFF_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "关机", "配置关机状态下的闪灯状态", LED_POWEROFF, 0x40, 0x15, 4, 255, 		LED_PWROFF_CONFIG_EN);

config(CHECK, "蓝牙初始化状态配置LED", "是否配置初始化状态指示灯(蓝牙开机播提示音、等待5秒按键时)", LED_BTINIT_CONFIG_EN, 0);
config(LED, "初始化", "配置蓝牙初始化状态下的闪灯状态", LED_BTINIT, 0x00, 0xaa, 10, 0, 	LED_BTINIT_CONFIG_EN);

config(CHECK, "蓝牙回连状态配置LED", "是否配置蓝牙回连状态指示灯(蓝牙未连接)", LED_RECONNECT_CONFIG_EN, 0);
config(LED, "回连", "配置回连状态下的闪灯状态", LED_RECONNECT, 0x00, 0xaa, 6, 0, 		LED_RECONNECT_CONFIG_EN);

config(CHECK, "蓝牙配对状态配置LED", "是否配置蓝牙配对状态指示灯(蓝牙未连接)", LED_BTPAIR_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "配对", "配置配对状态下的闪灯状态", LED_PAIRING, 0x60, 0x03, 3, 6, 			LED_BTPAIR_CONFIG_EN);

config(CHECK, "蓝牙已连接状态配置LED", "是否配置蓝牙已连接状态指示灯", LED_BTCONN_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "已连接", "配置已连接状态下的闪灯状态", LED_CONNECTED, 0x00, 0x02, 2, 86, 	LED_BTCONN_CONFIG_EN);

config(CHECK, "蓝牙音乐状态配置LED", "是否配置蓝牙音乐状态指示灯", LED_BTMUSIC_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "播放", "配置播放状态下的闪灯状态", LED_PLAYING, 0x00, 0x02, 2, 86, 		LED_BTMUSIC_CONFIG_EN);

config(CHECK, "蓝牙来电状态配置LED", "是否配置蓝牙来电状态指示灯", LED_BTRING_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "来电", "配置来电状态下的闪灯状态", LED_RING, 0x00, 0x02, 2, 86, 			LED_BTRING_CONFIG_EN);

config(CHECK, "蓝牙通话状态配置LED", "是否配置蓝牙通话状态指示灯", LED_BTCALL_CONFIG_EN, 0, LED_DISP_EN);
config(LED, "通话", "配置通话状态下的闪灯状态", LED_CALLING, 0x00, 0x02, 2, 86, 		LED_BTCALL_CONFIG_EN);

config(CHECK, "低电状态配置LED", "是否配置低电状态指示灯", LED_LOWBAT_CONFIG_EN, 0, RLED_LOWBAT_EN);
config(LED, "低电", "配置低电状态下的闪灯状态", LED_LOWBAT, 0xaa, 0x00, 6, 0, 			LED_LOWBAT_CONFIG_EN);

config(LEVEL, 0x100);
makecfgfile(xcfg.bin);
makecfgdef(xcfg.h);
xcopy(xcfg.h, ../../xcfg.h);
