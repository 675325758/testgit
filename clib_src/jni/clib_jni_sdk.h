#ifndef CLIB_JNI_SDK_H
#define CLIB_JNI_SDK_H

/*
 *工具定义
*/
#define BIT(n)	(1 << (n))
#define DEV_


#define SDK_CLASS       com_galaxywind_wukit_clibinterface_JniMethod
#define SDK_METHOD3(CLASS3, FUNC3)     Java_##CLASS3##_##FUNC3  
#define SDK_METHOD2(CLASS2, FUNC2)     SDK_METHOD3(CLASS2, FUNC2)  
#define SDK_METHOD(FUNC)    SDK_METHOD2(SDK_CLASS, FUNC)  
#define SDK_CLASS_NAME_FORMAT(CLASS)    "L"(CLASS)";"
#define SDK_ARRAY_CLASS_NAME_FORMAT(CLASS)    "[L"(CLASS)";"


#define JNI_MEM_NAME_MAX             64

/*
 *JNI类路径定义
*/
#define SDK_PATH      "com/galaxywind/wukit/clibinterface"
#define SDK_DEV_PATH "com/galaxywind/wukit/support_devs"

#define CLASS_BITSET "java/util/BitSet"

//设备类型中包含的主要类型
#define CLASS_SDK_TIMER                                SDK_PATH"/ClibTimerInfo"
#define CLASS_SDK_DEV_COMMON                    SDK_PATH"/ClibCommonDevInfo"
#define CLASS_SDK_DEV_STAT                          SDK_PATH"/ClibDevStat"
#define CLASS_SDK_ELEC                                   SDK_PATH"/ClibElecInfo"
#define CLASS_SDK_USER                                 SDK_PATH"/ClibUser"
#define CLASS_SDK_SMART                                SDK_PATH"/ClibSmartInfo"
#define CLASS_SDK_EPLUG                                 SDK_PATH"/ClibEplugInfo"
#define CLASS_SDK_AIR_PLUG                            SDK_PATH"/ClibAirPlugInfo"
#define CLASS_SDK_LED                                     SDK_PATH"/ClibLedInfo"
#define CLASS_SDK_LAN_DEV_INFO                 SDK_PATH"/ClibLanDevInfo"
#define CLASS_SDK_COMM_UDP                            SDK_PATH"/ClibUdpComm"
#define CLASS_SDK_LEDE                          SDK_PATH"/ClibLedeInfo"
#define CLASS_SDK_TB_HOUSE                     SDK_PATH"/ClibTbHouse"
#define CLASS_SDK_TB_PC                        SDK_PATH"/ClibTbPC"
#define CLASS_SDK_EH_WK                        SDK_PATH"/ClibEhAirplugInfo"

//主要类型中的子类型
#define CLASS_SDK_AIR_PLUG_WORK              SDK_PATH"/ClibAirplugWorkStat"
#define CLASS_SDK_AC_CODE                        SDK_PATH"/ClibAirconCode"
#define CLASS_SDK_AC_CODE_MATCH            SDK_PATH"/ClibAirconCodeMatchInfo"
#define CLASS_SDK_AC_KEY                           SDK_PATH"/CLibAirconKey"
#define CLASS_SDK_AC_NC_KEY                     SDK_PATH"/ClibAirconNcKeyInfo"
#define CLASS_SDK_AC_MATCH_STATE             SDK_PATH"/ClibAirplugMatchState"
#define CLASS_SDK_24_HOUR_DATA                     SDK_PATH"/Clib24HourData"
#define CLASS_SDK_AC_TEMP_CTRL                     SDK_PATH"/ClibAirplugTempCtrl"
#define CLASS_SDK_AC_CURVE_CTRL                    SDK_PATH"/ClibAirplugCurveCtrl"
#define CLASS_SDK_AC_CURVE_NODE                    SDK_PATH"/ClibAirplugCurveNode"
#define CLASS_SDK_AC_PARAM_ADJUST                  SDK_PATH"/ClibAirplugAdjustParam"
#define CLASS_SDK_AC_TEMP_ADJUST                   SDK_PATH"/ClibAirplugAdjustTemp"
//智能模块
#define CLASS_SDK_SMART_OFF_PARAM         SDK_PATH"/ClibAirconSmartOffParam"
#define CLASS_SDK_SMART_ON_PARAM          SDK_PATH"/ClibAirconSmartOnParam"
#define CLASS_SDK_SMART_SLEEP_PARAM      SDK_PATH"/ClibAirconSmartSleepParam"
//电量模块
#define CLASS_SDK_ELEC_DAYS                         SDK_PATH"/ClibElecDaysInfo"
#define CLASS_SDK_ELEC_ITEM                          SDK_PATH"/ClibElecItem"
#define CLASS_SDK_ELEC_PERIOD                      SDK_PATH"/ClibElecPeriod"
#define CLASS_SDK_ELEC_STAT                          SDK_PATH"/ClibElecStat"
//C库信息
#define CLASS_SDK_CLIB_INFO                           SDK_PATH"/CLibInfo"
//局域网扫描数据
#define CLASS_SDK_LAN_CLIENT_STAT               SDK_PATH"/ClibLanClientStat"
#define CLASS_SDK_LAN_DEV                             SDK_PATH"/ClibLanDevInfo"
//LED控制模块
#define CLASS_SDK_LED_COLOR                         SDK_PATH"/ClibLedColor"
#define CLASS_SDK_LEDE_STAT                        SDK_PATH"/ClibLedeStat"
//定时器模块
#define CLASS_SDK_BASE_TIMER                      SDK_PATH"/ClibBaseTimer"
#define CLASS_SDK_PERIOD_TIMER                   SDK_PATH"/ClibPeriodTimer"
#define CLASS_SDK_WK_EXT_TIMER                   SDK_PATH"/ClibAirExtTimer"
#define CLASS_SDK_QPCP_EXT_TIMER                   SDK_PATH"/ClibQpcpTimer"
#define CLASS_SDK_LEDE_TIMER                      SDK_PATH"/ClibLedeTimer"

#define CLASS_SDK_LIGHT_SAMPLE                    SDK_PATH"/ClibLightSample"
#define CLASS_SDK_CLIB_LIMIT                         SDK_PATH"/ClibLimit"
#define CLASS_SDK_NEW_VERSION                    SDK_PATH"/ClibNewVersion"
#define CLASS_SDK_NI_STAT                             SDK_PATH"/ClibNiStat"
#define CLASS_SDK_ADVANCE_STAT                   SDK_PATH"/ClibUdpDevStat"
#define CLASS_SDK_MODULE_VERSION                 SDK_PATH"/ClibModuleVersion"

//华天成热水泵
#define CLASS_SDK_TB_HOUSE_UCONFIG               SDK_PATH"/ClibTbHouseUconfig"
#define CLASS_SDK_TB_HOUSE_WCONFIG               SDK_PATH"/ClibTbHouseWconfig"
#define CLASS_SDK_TB_HOUSE_TEMP                  SDK_PATH"/ClibTbHouseTemp"
#define CLASS_SDK_TB_HOUSE_FAULT                 SDK_PATH"/ClibTbHouseFault"
#define CLASS_SDK_TB_HOUSE_OTHER                 SDK_PATH"/ClibTbHouseOther"
//商用机和泳池机
#define CLASS_SDK_TB_PC_UPGRADE                  SDK_PATH"/ClibTbPCUpgrade"
#define CLASS_SDK_TB_PC_STAT                      SDK_PATH"/ClibTbPcStat"
#define CLASS_SDK_TB_PC_VER                       SDK_PATH"/ClibTbPCHdVer"
#define CLASS_SDK_TB_PC_BIND                      SDK_PATH"/ClibTbPCHdVer"
#define CLASS_SDK_TB_PC_CONFIG                    SDK_PATH"/ClibTbPCConfig"

//增强型悟空
#define CLASS_SDK_RC_PAIR                          SDK_PATH"/ClibPairRcInfo"
#define CLASS_SDK_RC_INFO                          SDK_PATH"/ClibRcInfo"
#define CLASS_SDK_RC_FIX_KEY                       SDK_PATH"/ClibRcFixKey"
#define CLASS_SDK_RC_CUS_KEY                       SDK_PATH"/ClibRcCustomKey"
#define CLASS_SDK_RC_PAIR_STAT                     SDK_PATH"/ClibRcPairStat"
#define CLASS_SDK_RC_KEY_LEARN_STAT               SDK_PATH"/ClibRcKeyLearningStat"

//rf网关
#define CLASS_SDK_RFGW_INFO                         SDK_PATH"/ClibRfGwInfo"
#define CLASS_SDK_RFGW_GROUP                        SDK_PATH"/ClibRfgwDevGroup"
#define CLASS_SDK_RFGW_LAMP_REMOTE                 SDK_PATH"/ClibRfgwLampRemote"
#define CLASS_SDK_RFGW_REMOTE_KEY                  SDK_PATH"/ClibRfgwLampRemoteKey"
#define CLASS_SDK_DEV_TYPE                          SDK_DEV_PATH"/RfSlaveDigest"

//宁波华佑
#define CLASS_SDK_HY_THERMOSTAT                     SDK_PATH"/ClibHyThermostatInfo"
#define CLASS_SDK_HY_THERMOSTAT_STAT               SDK_PATH"/ClibHyThermostatStat"


//设备类型
#define CLASS_SDK_DEV_AIRPLUG                      SDK_DEV_PATH"/wukong/AirplugInfo"
#define CLASS_SDK_DEV_EPLUG                        SDK_DEV_PATH"/wuneng/EplugInfo"
#define CLASS_SDK_DEV_ElECEPLUG                    SDK_DEV_PATH"/wuneng/ElecEplugInfo"
#define CLASS_SDK_DEV_LEDE                         SDK_DEV_PATH"/lede/LedeInfo"
#define CLASS_SDK_DEV_EH_WK                        SDK_DEV_PATH"/eh_wk/EhAirplugInfo"
//从设备
//单火线
#define CLASS_SDK_RF_SLAVE_COMM                       SDK_PATH"/ClibRfDevCommInfo"
#define CLASS_SDK_RF_SLAVE_SLF                        SDK_PATH"/ClibRfSlaveSlf"
#define CLASS_SDK_RF_SLAVE_SLF_STAT                  SDK_PATH"/ClibRfSlaveSlfStat"

//科希曼线控器
#define CLASS_SDK_KXM_WIRE                            SDK_PATH"/ClibKxmWireDrive"
#define CLASS_SDK_KXM_WIRE_STAT                       SDK_PATH"/ClibKxmWireStat"
#define CLASS_SDK_KXM_WIRE_SUB_STAT                  SDK_PATH"/ClibKxmWireSubStat"
#define CLASS_SDK_KXM_WIRE_HOST                       SDK_PATH"/ClibKxmWireHost"
#define CLASS_SDK_KXM_WIRE_TIMER                      SDK_PATH"/ClibKxmWireTimer"
//科希曼温控器
#define CLASS_SDK_KXM_THERM                            SDK_PATH"/ClibKxmTherInfo"

//RF设备通信历史记录
#define CLASS_SDK_RF_HISTORY                           SDK_PATH"/ClibRfSlaveHistory"
#define CLASS_SDK_RF_HISTORY_ITEM                      SDK_PATH"/ClibRfSlaveHistoryItem"

//RF设备通用报警记录
#define CLASS_SDK_RF_ALARM                              SDK_PATH"/ClibRfSlaveAlarm"

//汇泰龙云锁
#define CLASS_SDK_HTL_INFO                              SDK_PATH"/ClibRfSlaveHtl"
#define CLASS_SDK_HTL_STAT                              SDK_PATH"/ClibHtlLockStat"
#define CLASS_SDK_HTL_USER                              SDK_PATH"/ClibHtlUserManageStat"
#define CLASS_SDK_HTL_NOTICE                            SDK_PATH"/ClibHtlNoticeStat"
#define CLASS_SDK_HTL_PIN                            SDK_PATH"/ClibHtlSetPinParam"


//鑫源温控器
#define CLASS_SDK_XINYUAN_INFO                          SDK_PATH"/ClibXYWkq"
#define CLASS_SDK_XINYUAN_ADJUST                        SDK_PATH"/ClibXYWkqAdjust"
#define CLASS_SDK_XINYUAN_MODE                          SDK_PATH"/ClibXYWkqSmartMode"
#define CLASS_SDK_XINYUAN_TP                            SDK_PATH"/ClibXYWkqTp"





/*
 *ANDROID DK申明，必须与ANDROID SDK定义一致
*/
enum {
	DEV_INFO_BIT_COMMON = 0,//设备共有的信息
	DEV_INFO_BIT_TIMER = 1,//设备定时器信息
	DEV_INFO_BIT_ELEC = 2,//设备电量统计信息
	DEV_INFO_BIT_LED = 3,//led灯信息
	DEV_INFO_BIT_SMART = 4,//智能控制信息
	DEV_INFO_BIT_AIRPLUG = 5,//空调信息
	DEV_INFO_BIT_EPLUG = 6,//插座信息
	DEV_INFO_BIT_UDP_COMMON = 7,//DUP设备通用信息
	DEV_INFO_BIT_UDP_DEV = 8,//UDP设备中的指定设备信息
	DEV_INFO_BIT_EH_WK = 9, //增强型悟空
	DEV_INFO_BIT_SLAVES_TYPE = 10,//从设备类型
	DEV_INFO_BIT_HIGHEST
};

enum {
	SLAVE_INFO_BIT_SLAVE_COMM = 0,//从设备通用信息
	SLAVE_INFO_BIT_RF_SLAVE_COMM = 1,//RF从设备通用信息
	SLAVE_INFO_BIT_RF_SLAVE_HISTORY = 2,//RF从设备历史记录
	SLAVE_INFO_BIT_RF_SLAVE_ALARM = 3,//RF从设备报警信息
	SLAVE_INFO_BIT_RF_DEV = 4,//RF从设备特有信息
	SLAVE_INFO_BIT_HIGHEST 
};

#define DEV_COMM_INFO_MEM_NAME   "commonInfo"
#define DEV_STAT_INFO_MEM_NAME "statInfo"
#define DEV_AIRPLUG_INFO_MEM_NAME "airPlugInfo"
#define DEV_LED_INFO_MEM_NAME "led_info"
#define DEV_SMART_INFO_MEM_NAME "smart_info"
#define DEV_ELEC_INFO_MEM_NAME "elec_info"
#define DEV_TIMER_INFO_MEM_NAME "timer_info"
#define DEV_EPLUG_INFO_MEM_NAME "eplugInfo"
#define DEV_COMM_UDP_INFO_MEM_NAME "commUdpInfo"
#define DEV_UDP_DEV_INFO_MEM_NAME ""
#define DEV_EH_WK_INFO_MEM_NAME "ehanceInfo"
#define DEV_SLAVES_TYPE_MEM_NAME "slaveDigests"

//RF从设备
#define RF_SLAVE_COMM_MEM_NAME "commonInfo"
#define RF_SLAVE_DEV_MEM_NAME "rfSlaveDevInfo"
#define RF_SLAVE_HISTORY_MEM_NAME "history"
#define RF_SLAVE_ALARM_MEM_NAME "alarm"

#define DEV_TIMER_DATA_MEM_NAME timers
#define DEV_PERIOD_TIMER_DATA_MEM_NAME period_timers


#define SDK_NORMAL_TIMER_CLASS_NAME SDK_PATH"/ClibBaseTimer"
#define SDK_PERIOD_TIMER_CLASS_NAME SDK_PATH"/ClibPeriodTimer"


#endif

