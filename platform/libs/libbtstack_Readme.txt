1. 升级库时，请整体替换libs文件夹。
2. 关于蓝牙库选择说明（默认链接的是libbtstack.a，使用其它名字的蓝牙库请改成此名字，再重新编译）：
3. 默认蓝牙库带TWS,不带BLE，用的是libbtstack_tws_no_ble.a,打开BLE功能时需要用带BLE的蓝牙库,如libbtstack_ble_no_tws或libbtstack_tws_ble
//-------------------------------------------------
libbtstack.a
默认链接的蓝牙库为(libbtstack_tws_no_ble.a).
该库包含TWS,但不包含BLE,库中以下宏定义为
#define BT_TWS_EN          1
#define COMPILER_BLE       0   //蓝牙库&基带库
//-------------------------------------------------
libbtstack_tws_no_ble.a
该库包含TWS,但不包含BLE,库中以下宏定义为
#define BT_TWS_EN          1
#define COMPILER_BLE       0   //蓝牙库&基带库
libbtstack_tsco_no_ble.a       //基带库中需要开TSCO
//-------------------------------------------------
libbtstack_ble_no_tws.a
该库包含BLE,但不包含TWS,库中以下宏定义为
#define BT_TWS_EN          0  //蓝牙库
#define COMPILER_BLE       1  //蓝牙库&基带库
//-------------------------------------------------
libbtstack_tws_ble.a
该库包含TWS,也包含BLE,库中以下宏定义为
#define BT_TWS_EN          1  //蓝牙库
#define COMPILER_BLE       1  //蓝牙库&基带库
//-------------------------------------------------
libbtstack_no_tws_ble.a
该库不包含TWS,也不包含BLE,库中以下宏定义为
#define BT_TWS_EN          0  //蓝牙库
#define COMPILER_BLE       0  //蓝牙库&基带库
//-------------------------------------------------
libbtstac_tws_fast_pair.a    //快连库 包含TWS不包括BLE，主要是加快TWS之间的连接速度
//蓝牙库&基带库
#define BT_TWS_FAST_CON_EN              1
#define BLE_TINY_ADV                    1
#define BLE_TINY_SCAN                   1
#define BLE_TINY_SCAN_RSP               1
//-------------------------------------------------
libbtstack_spp_240.a        //SPP 发送改为最大240BYTE
修改   SPP_TX_BUF_LEN           为240
//-------------------------------------------------
发布蓝牙库时注意,基带库关不了TWS,但TSCO要控制一下.

