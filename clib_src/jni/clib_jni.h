 #ifndef	__CLIB_JNI_H__
#define	__CLIB_JNI_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <jni.h>
#include "client_lib.h"
#include "cl_user.h"
#include "cl_video.h"
#include "cl_plug.h"
#include "cl_equipment.h"
#include "cl_notify_push.h"
#include "cl_area.h"
#include "cl_lan_dev_probe.h"
#include "cl_scene.h"
#include "cl_smart_config.h"
#include "cl_smart_appliance.h"
#include "cl_env_mon.h"
#include "cl_ia.h"
#include "cl_health.h"
#include "cl_smart_appliance.h"
#include "cl_lamp.h"  // 高迅 调光灯
#include "cl_lc_furnace.h"
#include "cl_cloud_ac.h"
#include "cl_ch_blanket.h"
#include "cl_common_udp_device.h"
#include "cl_lede_lamp.h"
#include "cl_tl_temp.h"
#include "cl_hxpbj.h"
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include "cl_server.h"

// 锟斤拷锟斤拷log头锟侥硷拷
#include <android/log.h>  

// log锟斤拷签
#define  TAG    "CLibJNI"
// 锟斤拷锟斤拷debug锟斤拷息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 锟斤拷锟斤拷info锟斤拷息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG, __VA_ARGS__)
// 锟斤拷锟斤拷error锟斤拷息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG, __VA_ARGS__)

#define LOGE_FILE_LINE LOGE("%s %d",__FILE__,__LINE__)

#define DEBUG_FUNC_CALL

#ifdef DEBUG_FUNC_CALL
#define DEBUG_ENTER_FUNC() LOGE("ENTER FILE IN %s Func %s LINE %d \n",__FILE__,__FUNCTION__,__LINE__)
#define DEBUG_OUT_FUNC() LOGE("OUT FUNC IN FILE %s Func %s LINE %d \n",__FILE__,__FUNCTION__,__LINE__)
#else
#define DEBUG_ENTER_FUNC() 
#define DEBUG_OUT_FUNC()
#endif

#define JNI_ASSERT_DEBUG

#ifdef JNI_ASSERT_DEBUG
#define	jni_assert(b) _jni_assert(!(!(b)), __FILE__, __LINE__)
#else
#define	jni_assert(b) 
#endif

extern void _jni_assert(bool b, const char *file, int line);

/*
	锟斤拷同锟斤拷锟斤拷锟斤拷锟睫革拷锟斤拷锟斤拷锟紺LASS锟斤拷锟藉即锟斤拷
*/
#define CLASS   com_galaxywind_clib_CLib

#define	PACKET_PATH	"com/galaxywind/clib"
#define	CLASS_CLIB	 PACKET_PATH "/CLib"
#define CLASS_ARRAY_LIST "java/util/ArrayList"
#define CLASS_APP_STAT_INFO PACKET_PATH "/AppStatInfo"

#define CLASS_HEALTH_MENBER PACKET_PATH "/HealthMemberInfo"
#define CLASS_MEASURE_DATA PACKET_PATH "/HealthMeasureData"
#define CLASS_MEASURE_QUREY PACKET_PATH "/HealthMeasureQuery"

#define CLASS_GPSLOCATION_DATA PACKET_PATH "/GpsLoactionData"

//#define	CLASS_USER	 PACKET_PATH "/User"
#define CLASS_DEV_INFO PACKET_PATH "/DevInfo"
#define CLASS_USER_INFO PACKET_PATH "/UserInfo"
#define	CLASS_SLAVE	 PACKET_PATH "/Slave"
#define	CLASS_OBJ	 PACKET_PATH "/Obj"
#define	CLASS_USB_VIDEO	 PACKET_PATH "/UsbVideo"
#define	CLASS_PLUG	 PACKET_PATH "/Plug"
#define	CLASS_PLUG_INFO PACKET_PATH "/PlugInfo"
#define	CLASS_PLUG_TIMER PACKET_PATH "/PlugTimer"
#define CLASS_OPER_RESULT PACKET_PATH "/OperResult"

//小米推送
#define CLASS_XM_PUSH_CONFIG          PACKET_PATH"/ApnsConfig"


#define CLASS_RF_RMT_CTRL_INFO PACKET_PATH "/RFRmtCtrlInfo"
#define CLASS_RF_RMT_CTRL_KEY_INFO PACKET_PATH "/RFRmtCtrlKeyInfo"
#define CLASS_RF_LAMP_RMT_CTRL_INFO PACKET_PATH "/RFLampRmtCtrlInfo"

#define CLASS_RF_DEV_STATU PACKET_PATH "/RFDevStatu"
#define CLASS_RF_DEV_WORK_T PACKET_PATH "/RFDevWorkT"
#define CLASS_RF_LIGHT_STATU PACKET_PATH "/RFLightState"
#define CLASS_RF_DEV_GROUP_STATU PACKET_PATH "/RFDevGroupInfo"
#define CLASS_RF_DEV_GW_STATU PACKET_PATH "/RFDevGwInfo"
#define CLASS_RF_GROUP_CTRL_PARAM PACKET_PATH "/RFGroupCtrlParam"
#define CLASS_RF_DOORLOCK_INFO PACKET_PATH "/RFDoorLockInfo"
#define CLASS_RF_DOORLOCK_STATE PACKET_PATH "/RFDoorLockState"
#define CLASS_RF_DOOR_HISTORY PACKET_PATH "/RFDoorHistoryInfo"
#define CLASS_RF_SWITCH_STAT PACKET_PATH "/RFSwitchStat"
#define CLASS_RF_SWITCH_NAME PACKET_PATH "/RFSwitchName"
#define CLASS_RF_DOORMAGNET_INFO PACKET_PATH "/RFDoorMagnetInfo"
#define CLASS_RF_DOORMAGNET_STATE PACKET_PATH "/RFDoorMagnetState"
#define CLASS_RF_DOOR_ALARM_INFO PACKET_PATH "/RFDoorAlarmInfo"
#define CLASS_RF_AUTO_GUARD_INFO PACKET_PATH "/RFAutoGuardInfo"
#define CLASS_RF_DOOR_WIFI_LOCK PACKET_PATH"/RfDoorWifiLock"
#define CLASS_RF_DOOR_CONTROLLER  PACKET_PATH"/RfDoorLockController"
#define CLASS_RF_HP_GW_INFO PACKET_PATH"/RFHpGwInfo"
#define CLASS_RF_HP_GW_PHONE_USER PACKET_PATH"/HpPhoneUser"
#define CLASS_RF_GW_ONEKEY_CTRL_INFO PACKET_PATH"/RFGwOneKeyCtrlInfo"
#define CLASS_RF_GW_ONEKEY_SET_DEFENCE PACKET_PATH"/RFGwOneKeySetDefence"
#define CLASS_RF_GW_ONEKEY_SET_ONOFF PACKET_PATH"/RFGwOneKeySetOnOff"
#define CLASS_RF_GW_ONEKEY_SET_ALARM_MODE PACKET_PATH"/RFGwOneKeySetAlarmMode"
#define CLASS_RF_GW_ONEKEY_SMART_ALARM PACKET_PATH"/RFGwOneKeySmartAlarm"

#define CLASS_RF_COMM_ALARM        PACKET_PATH"/RfCommAlarmInfo"
#define CLASS_RF_COMM_HISTORY      PACKET_PATH"/RfCommHistoryInfo"
#define CLASS_RF_COMM_HISTORY_ITEM      PACKET_PATH"/RfCommHistoryItem"

#define	CLASS_KEY	 PACKET_PATH "/Key"
#define	CLASS_ALARM_INFO	 PACKET_PATH "/AlarmInfo"
#define	CLASS_EQUIPMENT	 PACKET_PATH "/Equipment"
#define	CLASS_EQUIPMENT_ADD_INFO	 PACKET_PATH "/EquipmentAddInfo"

#define	CLASS_KEY_LEARN_STAT	 PACKET_PATH "/KeyLearn"

#define	CLASS_ALARM_MSG_LIST	 PACKET_PATH "/AlarmMsgList"
#define	CLASS_ALARM_MSG	 PACKET_PATH "/AlarmMsg"
#define	CLASS_NOTIFY_MSG	 PACKET_PATH "/NotifyMsg"

#define	CLASS_UPGRADE_DEV_INFO	PACKET_PATH "/DevStateInfo"
#define CLASS_CNREQUEST_ITEM PACKET_PATH "/CmtNotifyRequestItem"
#define CLASS_CODE_MATCH_STATE PACKET_PATH "/CodeMatchState"

#define	CLASS_LAN_DEV_INFO	 PACKET_PATH "/LanDevInfo"
#define	CLASS_AREA	 PACKET_PATH "/Area"
#define	CLASS_SCENE	 PACKET_PATH "/Scene"
#define	CLASS_SCENE_EVEN	 PACKET_PATH "/SceneEven"

#define	CLASS_SCENE_TIMER	 PACKET_PATH "/Timer"
#define CLASS_AIRCON_INFO   PACKET_PATH "/AirCon"

#define CLASS_IA_AIR_CLEANER	PACKET_PATH "/IaAirCleaner"
#define CLASS_IA_AIR_CONDITION PACKET_PATH "/IaAircondition"
#define CLASS_IA_WATER_HEATER	PACKET_PATH "/IaWaterHeater"
#define CLASS_IA_AIR_HEATER PACKET_PATH "/IaAirHeater"
#define CLASS_IA_AIR_HEATER_YCYT PACKET_PATH "/IaAirHeaterYcyt"
#define CLASS_IA_ELETRINC_FAN PACKET_PATH "/IaElectricfan"
#define CLASS_IA_WATER_HEATER_A9	PACKET_PATH "/IaWaterHeaterA9"
#define CLASS_IA_BATH_HEATER_AUPU	PACKET_PATH "/IaBathHeaterAupu"

#define CLASS_DEV_AIR_HEATER_LC	PACKET_PATH "/LcFurnaceInfo"
#define CLASS_DEV_GX_LAMP PACKET_PATH "/GxLampInfo"
#define CLASS_IF_INFO PACKET_PATH "/IfInfo"


#define CLASS_AIR_PLUG	 PACKET_PATH "/AirPlug"
#define CLASS_AIR_PLUG_TIMER	PACKET_PATH "/AirplugTimer"
#define CLASS_AIR_PLUG_MATCH_CODE_INFO	PACKET_PATH "/AirPlugMatchCodeInfo"
#define CLASS_AIR_PLUG_KEY_INFO	PACKET_PATH "/AirPlugNoScreenKeyInfo"
#define CLASS_AIR_PLUG_KEY_STAT	PACKET_PATH "/AirPlugKeyStat"
#define CLASS_AIR_PLUG_TEMP_CURVE PACKET_PATH "/AcTempCurve"
#define CLASS_AIR_PLUG_TEMP_CURVE_LINE PACKET_PATH "/TempCurve"
#define CLASS_AIR_PLUG_TEMP_CTRL PACKET_PATH "/AcTempCtrl"
#define CLASS_AIR_PLUG_PERIOD_TIMER PACKET_PATH "/AirplugPeriodTimer"
#define CLASS_AIR_PLUG_SMART_ON	 PACKET_PATH "/AirPlugSmartOn"
#define CLASS_AIR_PLUG_ELEC	 PACKET_PATH "/AirPlugElec"
#define CLASS_AIR_PLUG_SMART_OFF PACKET_PATH "/AirPlugSmartOff"
#define CLASS_AIR_PLUG_SMART_SLEEP PACKET_PATH "/AirPlugSmartSleep"
#define CLASS_AIR_PLUG_HOUR_LINES PACKET_PATH "/Line24Hour"
#define CLASS_AIR_PLUG_MSG_CONFIG	PACKET_PATH "/AirPlugMsgConfig"

#define CLASS_AIR_PLUG_PAIR_RC_INFO PACKET_PATH "/PairRcInfo"
#define CLASS_AIR_PLUG_RC_INFO PACKET_PATH "/RcInfo"
#define CLASS_AIR_PLUG_RC_FIXED_KEY_INFO PACKET_PATH "/RcFixedKeyInfo"
#define CLASS_AIR_PLUG_RC_USER_KEY_INFO PACKET_PATH "/RcUserKeyInfo"
#define CLASS_AIR_PLUG_RC_MATCH_STAT PACKET_PATH "/RcMatchStat"
#define CLASS_AIR_PLUG_RC_KEY_LEARN_STAT PACKET_PATH "/RcKeyLearnStat"
#define CLASS_AIR_PLUG_PRIV_RC_MANAGE_INFO PACKET_PATH "/PrivRcManageInfo"
#define CLASS_SHARE                            PACKET_PATH"/ShareInfo"
#define CLASS_SHARE_RECORD                    PACKET_PATH"/ShareRecord"

#define CLASS_E_PLUG	 PACKET_PATH "/EPlug"
#define CLASS_E_PLUG_TIMER	 PACKET_PATH "/EPlugTimer"
#define CLASS_E_PLUG_PERIOD_TIMER PACKET_PATH"/EplugPeriodTimer"
#define CLASS_E_PLUG_ELEC_INFO PACKET_PATH"/CommElecInfo"
#define CLASS_E_PLUG_DAYS_INFO  PACKET_PATH"/ElecDaysStatInfo" 

#define	CLASS_VIDEO_STAT	 PACKET_PATH "/VideoStat"
#define	CLASS_VIDEO_SATURATION	 PACKET_PATH "/VideoSaturation"
#define	CLASS_SOUND_DATA	 PACKET_PATH "/SoundData"

#define	CLASS_VIDEO_INFO	 PACKET_PATH "/VideoInfo"
#define	CLASS_VRT_ITEM	 PACKET_PATH "/VrtItem"
#define	CLASS_REAL_TIME_INFO	 PACKET_PATH "/AirPlugRealTime"
#define	CLASS_BIND_PHONE_RESULT	 PACKET_PATH "/BindPhoneResult"
#define	CLASS_BIND_PHONE	 PACKET_PATH "/BindPhone"
#define	CLASS_BIND_PHONE_ITEM	 PACKET_PATH "/BindPhoneItem"
//PM 2.5锟斤拷锟?
#define CLASS_ENVMONITORINFO	PACKET_PATH "/EnvMonitorInfo"
#define CLASS_ENVPROVINCEINFO	PACKET_PATH "/EnvProvinceInfo"
//锟借备状态锟斤拷息
#define CLASS_SLAVESTATINFO		PACKET_PATH "/SlaveStatInfo"
#define CLASS_SLAVEINTERFACEINFO		PACKET_PATH "/NinterfaceInfo"
#define CLASS_SLAVECLIENTINFO		PACKET_PATH "/LanClient"
//锟斤拷锟斤拷锟斤拷锟斤拷锟截的客伙拷锟斤拷锟斤拷锟铰版本锟斤拷息
#define CLASS_NEWEST_VERSION_INFO		PACKET_PATH "/SoftNewestVersionInfo"
#define CLASS_CLIB_VERSION_INFO		PACKET_PATH "/CLibInfo"
#define CLASS_LIMIT		PACKET_PATH "/Limit"
#define CLASS_VIDEO_PARAM		PACKET_PATH "/VideoParam"
#define CLASS_TRAFFIC_STAT		PACKET_PATH "/TrafficStat"
#define CLASS_VTAP_INFO		PACKET_PATH "/VtapInfo"
#define CLASS_VTAP_PICTURE		PACKET_PATH "/VtapPicture"
#define CLASS_CH_BLANKET           PACKET_PATH "/ChBlanket"
#define CLASS_BLANKET_SECTOR   PACKET_PATH "/BlanketSector"
#define CLASS_ENVAIR   PACKET_PATH "/EnvAirParams"

//杰能宝温控器
#define CLASS_SET_SCENE  PACKET_PATH "/TmcSetScene"
#define CLASS_TMC_JNB  PACKET_PATH "/TmcJNBInfo"
//亿林温控器
#define CLASS_TMC_YL  PACKET_PATH "/TmcYLInfo"
//金长信配电箱
#define CLASS_PDC_JCX PACKET_PATH "/PdcJCXInfo"


//UDP通用设备信息
#define CLASS_COMM_UDP_DEV    PACKET_PATH"/CommUdpInfo"
#define CLASS_TIMER_INFO                    PACKET_PATH"/TimerInfo"
#define CLASS_ELEC_STAT             PACKET_PATH"/ElecStatInfo"
#define CLASS_PEAK_TIME            PACKET_PATH"/PeakTime"
#define CLASS_ELEC_ITEM           PACKET_PATH"/ElectItemInfo"
#define CLASS_UDP_DEV_TIMER          PACKET_PATH"/UdpTimer"
#define CLASS_UDP_PERIOD_TIMER PACKET_PATH"/UdpPeriodTimer"
#define CLASS_JAVA_OBJECT   "java/lang/Object"
#define CLASS_COMM_DEV_ERR_INFO PACKET_PATH"/DevErrInfo"
#define CLASS_COMM_DEV_ERR_ITEM PACKET_PATH"/DevErrItem"
#define CLASS_COMM_SHORTCUT_POWER  PACKET_PATH"/ShortcutPower"
#define CLASS_COMM_DEV_HISTORY_INFO  PACKET_PATH"/CommDevHistoryInfo"
#define CLASS_COMM_DEV_HISTORY_ITEM  PACKET_PATH"/CommDevHistoryItem"
#define CLASS_COMM_AIRPLUG_HISTORY_ITEM  PACKET_PATH"/AirPlugHistoryItem"
#define CLASS_COMM_DHCP_CONFIG       PACKET_PATH"/CommDhcpConfig"
#define CLASS_COMM_WAN_CONFIG       PACKET_PATH"/CommWanConfig"
#define CLASS_COMM_WAN_PHY_CONFIG       PACKET_PATH"/CommWanPhyConfig"
#define CLASS_COMM_WAN_CONFIG_ITEM  PACKET_PATH"/CommWanConfigItem"
#define CLASS_COMM_WAN_CONFIG_ITEM_STATIC  PACKET_PATH"/CommStaticParam"
#define CLASS_COMM_WAN_CONFIG_ITEM_PPPOE  PACKET_PATH"/CommPPPOEParam"
#define CLASS_COMM_WAN_CONFIG_ITEM_DHCP  PACKET_PATH"/CommDHCPParam"
#define CLASS_COMM_AP_CONFIG       PACKET_PATH"/CommApConfig"

//华天成
#define CLASS_HTCHP                                PACKET_PATH"/HtcHp"
#define CLASS_HTCHP_USER_CONFIG         PACKET_PATH"/HtchpUserConfig"
#define CLASS_HTCHP_WORK_CONFIG        PACKET_PATH"/HtchpWorkConfig"
#define CLASS_HTCHP_TEMP_INFO              PACKET_PATH"/HtchpTempInfo"
#define CLASS_HTCHP_FAULT_STAT             PACKET_PATH"/HtchpFaultStat"
#define CLASS_HTCHP_OTHER_INFO             PACKET_PATH"/HtchpOtherInfo"
// lede调光灯
#define CLASS_LEDE_LAMP_INFO            PACKET_PATH"/LedeLampInfo"
#define CLASS_LEDE_LAMP_TIMER            PACKET_PATH"/LedeLampTimerInfo"
#define CLASS_LEDE_ON_STAT           PACKET_PATH"/LedeLedOnStat"

//艾美特
#define CLASS_AMT_INFO          PACKET_PATH"/AmtFanInfo"
//前锋热水器
#define CLASS_CHIFFO          PACKET_PATH"/ChiffoWall"
#define CLASS_CHIFFO_DAY_TIMER          PACKET_PATH"/ChiffoDayTimer"
#define CLASS_CHIFFO_SECTION_TIMER          PACKET_PATH"/ChiffoSectionTimer"
#define CLASS_TELIN_HEATING          PACKET_PATH"/TeLinHeating"
#define CLASS_TELIN_HEATING_TIME_ITEM          PACKET_PATH"/TelinTimerItem"

//千帕茶盘
#define CLASS_QP_INFO	PACKET_PATH"/QianpaTeaInfo"
#define CLASS_QP_CONFIG PACKET_PATH"/QianpaTeaConfig"
#define CLASS_QP_TEA_SCENE_PARAM	PACKET_PATH"/QianPaTeaSceneparam"
#define CLASS_QP_SCENES	PACKET_PATH"/QianpaTeaScenes"
#define CLASS_QP_SCENE_ITEM PACKET_PATH"/QianpaTeaSenceItem"

//千帕锅
#define CLASS_QP_PAN_INFO	PACKET_PATH"/QianpaPanInfo"
#define CLASS_QP_PAN_SCENE_ITEM PACKET_PATH"/QianpaPanSenceItem"
#define CLASS_QP_PAN_SCENE_PARAM	PACKET_PATH"/QianPaPanSceneparam"
#define CLASS_QP_PAN_PERIOD_TIMER PACKET_PATH "/QianpaPanPeriodTimer"

//千帕破壁机
#define CLASS_QP_POBIJI_INFO	PACKET_PATH"/QianpaPbjInfo"
#define CLASS_QP_POBIJI_SCENE	PACKET_PATH"/QianpaPbjScene"
#define CLASS_QP_POBIJI_ACTION	PACKET_PATH"/QianpaPbjAction"
#define CLASS_QP_POBIJI_MIX_INFO	PACKET_PATH"/QianpaPbjMixInfo"
#define CLASS_QP_POBIJI_HOT_INFO	PACKET_PATH"/QianpaPbjHotInfo"

//破壁机 
#define CLASS_HX_POBIJI PACKET_PATH"/Pobiji"

#define NAME3(CLASS3, FUNC3) Java_##CLASS3##_##FUNC3  
#define NAME2(CLASS2, FUNC2) NAME3(CLASS2, FUNC2)  
#define NAME(FUNC) NAME2(CLASS, FUNC)  

//编码匹配调整信息
#define CLASS_WUKONG_PARAM_ADJUST PACKET_PATH"/WkParamAdjust"

//CZWK车载悟空
#define CLASS_CAR_INFO PACKET_PATH"/CarInfo"
#define CLASS_CAR_ALARM PACKET_PATH"/CarAlarm"
#define CLASS_CAR_SEARCH PACKET_PATH"/CarSearch"

//OEM 插座，目前包括:沙特插座,晴乐插座
#define CLASS_WUNENG_OEM PACKET_PATH"/EplugOEMInfo"

//BIMAR暖风机
#define CLASS_BIMAR_HEATER PACKET_PATH"/BimarHeater"

//鑫源温控器
#define CLASS_XYWKQ PACKET_PATH"/XYWkq"
#define CLASS_XYWKQTP PACKET_PATH"/XYWkqTp"
#define CLASS_XYWKQ_SMART_MODE PACKET_PATH"/XYWkqSmartMode"
#define CLASS_XYWKQ_ADJUST PACKET_PATH"/XYWkqAdjust"

//华天成热水泵，商用
#define CLASS_TB_COMM_INFO                   PACKET_PATH"/TbCommercial"
#define CLASS_TB_COMM_VER                    PACKET_PATH"/TbCommercialHdVer"
#define CLASS_TB_COMM_SCROLL                 PACKET_PATH"/TbCommercialScroll"
#define CLASS_TB_COMM_CONFIG                 PACKET_PATH"/TbCommercialConfig"
#define CLASS_TB_COMM_DEFROST                PACKET_PATH"/TbCommercialDefrost"
#define CLASS_TB_COMM_PROTECT                PACKET_PATH"/TbCommercialProtect"
#define CLASS_TB_COMM_EEV                    PACKET_PATH"/TbCommercialEev"
#define CLASS_TB_COMM_MISC                    PACKET_PATH"/TbCommercialMisc"
#define CLASS_TB_COMM_STAT                    PACKET_PATH"/TbCommercialStat"
#define CLASS_TB_COMM_UPGRADE                 PACKET_PATH"/TbCommercialUpgrade"
#define CLASS_TB_COMM_BIND                    PACKET_PATH"/TbCommercialBind"
#define CLASS_TB_COMM_CONF_TEMP              PACKET_PATH"/TbCommercialConfTemp"

//海迅养生壶
#define CLASS_HX_POT_INFO                      PACKET_PATH"/HxPot"
#define CLASS_HX_POT_STAT                      PACKET_PATH"/HxPotStat"
#define CLASS_HX_POT_SCENE                     PACKET_PATH"/HxPotScene"

//月兔空调
#define CLASS_YT_AIRPLUG_INFO                 PACKET_PATH"/MoonRabbit"
#define CLASS_YT_TIMER                         PACKET_PATH"/MoonRabbiTimer"
#define CLASS_YT_AC_TYPE                       PACKET_PATH"/MoonRabbitAcType"
#define CLASS_YT_ELEC                          PACKET_PATH"/MoonRabbitElec"
#define CLASS_YT_ELEC_Q                        PACKET_PATH"/MoonRabbitElecQuery"
//集利舞台灯
#define CLASS_JL_INFO                          PACKET_PATH"/JLStageLamp"
#define CLASS_JL_3200_LAMP                     PACKET_PATH"/JL3200Lamp"
//奥德绅
#define CLASS_ADS_INFO                          PACKET_PATH"/Aodesheng"
#define CLASS_ADS_CONFIG                        PACKET_PATH"/AdsConfig"
//晶石微波炉
#define CLASS_JS_WAVE_OVEN                      PACKET_PATH"/JsWaveOven"
#define CLASS_JS_WAVE_SETTING                   PACKET_PATH"/JsVaveSetting"
//科希曼热水器
#define CLASS_KXM_INFO                           PACKET_PATH"/KxmHeaterPump"
#define CLASS_KXM_STAT                           PACKET_PATH"/KxmStat"
#define CLASS_KXM_SUB_STAT                       PACKET_PATH"/KxmSubStat"
#define CLASS_KXM_HOST                           PACKET_PATH"/KxmHost"
#define CLASS_KXM_TIMER                          PACKET_PATH"/KxmTimer"
#define CLASS_KXM_THERMOST                       PACKET_PATH"/KxmThermost"
//思博特温控器
#define CLASS_SBT_INFO                            PACKET_PATH"/SbtThermostat"
#define CLASS_SBT_SMART                           PACKET_PATH"/SbtSmartInfo"
#define CLASS_SBT_SMART_DAY                       PACKET_PATH"/SbtSmartDay"
#define CLASS_SBT_SMART_ITEM                      PACKET_PATH"/SbtSmartItem"
//中山商贤电热炉
#define CLASS_ZSSX_OVEN                            PACKET_PATH"/ZssxWallOven"
#define CLASS_ZSSX_WIFI_CONF                      PACKET_PATH"/ZssxWifiConf"
//海曼
#define CLASS_HM_BODY                              PACKET_PATH"/HmBodyInfo"
#define CLASS_HM_TEMP_HUM                          PACKET_PATH"/HmTempHumInfo"
#define CLASS_HM_HISTORY                           PACKET_PATH"/HmHistory"
//益佳电暖炉
#define CLASS_YJ_HEATER                            PACKET_PATH"/YijiaHeater"
#define CLASS_YJ_HEATER_SET                        PACKET_PATH"/YijiaHeaterSet"
//御源净水器
#define CLASS_YY_STAT                               PACKET_PATH"/YuYuanStat"
#define CLASS_YY_WATER_HISTORY                     PACKET_PATH"/YuYuanWaterHistory"
#define CLASS_YY_PWD                                PACKET_PATH"/YuYuanPwd"
#define CLASS_YY_REMIND                             PACKET_PATH"/YuYuanRemind"
#define CLASS_YY_INFO                               PACKET_PATH"/YuYuanInfo"
//暖气阀
#define CLASS_HEATING_VALVE_INFO                   PACKET_PATH"/HeatingValue"
#define CLASS_HEATING_VALVE_STAT                   PACKET_PATH"/HeatingValveStat"
#define CLASS_HEATING_VALVE_DATE                   PACKET_PATH"/HeatingValveDate"
#define CLASS_HEATING_VALVE_TEMP                   PACKET_PATH"/HeatingValveTemp"
#define CLASS_HEATING_VALVE_DAY                    PACKET_PATH"/HeatingValveDayPeriod"
//智科热水器
#define CLASS_ZK_WATER_HEATER_INFO                 PACKET_PATH"/ZkWaterHeater"
#define CLASS_ZK_WATER_HEATER_PARAM                PACKET_PATH"/ZkWaterHeaterParam"
#define CLASS_ZK_WATER_HEATER_TIMER                PACKET_PATH"/ZkWaterHeaterTimer"
//印度车
#define CLASS_INDIACAR_INFO                         PACKET_PATH"/IndiaCar"
#define CLASS_INDIACAR_STAT                         PACKET_PATH"/IndiaCarStat"
#define CLASS_INDIACAR_DEV_STAT                     PACKET_PATH"/IndiaCarDevStat"
#define CLASS_INDIACAR_STORE_STAT                   PACKET_PATH"/IndiaCarStoreStat"
#define CLASS_INDIACAR_WARN                          PACKET_PATH"/IndiaCarWarn"
#define CLASS_INDIACAR_WIFI_CONF                    PACKET_PATH"/IndiaCarWifiConf"
#define CLASS_INDIACAR_UPGRADE                      PACKET_PATH"/IndiaCarUpgrade"
#define CLASS_INDIACAR_JOURNEY_IDS                  PACKET_PATH"/IndiaCarJourneyIds"
#define CLASS_INDIACAR_HISTORY                      PACKET_PATH"/IndiaCarHistory"
#define CLASS_INDIACAR_HISTORY_HEADER               PACKET_PATH"/IndiaCarHistoryHeader"
#define CLASS_INDIACAR_LONGILATI                     PACKET_PATH"/LongiLatitude"
#define CLASS_INDIACAR_JOURNEY_NUM                   PACKET_PATH"/IndiaCarJourneyNum"
#define CLASS_INDIACAR_REALTIME_TRIP                 PACKET_PATH"/IndiaCarRealtimeTrip"
#define CLASS_INDIACAR_REALTIME_TRIP_ITEM            PACKET_PATH"/IndiaCarRealtimeTripItem"
#define CLASS_INDIACAR_DEBUG_INFO                     PACKET_PATH"/IndiaCarDebugInfo"
#define CLASS_INDIACAR_LOCAL_VIDEO                    PACKET_PATH"/IndiaCarLocalVideo"



//凯特rf插座
#define CLASS_KATE_RF_EPLUG_INFO                     PACKET_PATH"/KateRfEPlug"
#define CLASS_KATE_RF_EPLUG_STAT                     PACKET_PATH"/KateRfEplugStat"
//烟感、气感探测器
#define CLASS_COMM_DETECTOR_INFO                     PACKET_PATH"/CommDetector"
#define CLASS_COMM_DETECTOR_STAT                     PACKET_PATH"/CommDetectorStat"
#define CLASS_COMM_DETECTOR_HISTORY                  PACKET_PATH"/CommDetectorHistory"
#define CLASS_COMM_DETECTOR_ALARM                    PACKET_PATH"/CommDetectorAlarm"
#define CLASS_COMM_DETECTOR_ALARM_TIME               PACKET_PATH"/CommDetectorAlarmTime"

//Rf通用定时器
#define CLASS_RF_TIMER_INFO                           PACKET_PATH"/CommTimerInfo"
#define CLASS_RF_TIMER                                 PACKET_PATH"/CommTimer"
#define CLASS_COMM_TIMER_EXEC                         PACKET_PATH"/CommTimerExec"
#define CLASS_COMM_TIMER_EXT_ZYKT                     PACKET_PATH"/CommTimerExtZykt"
#define CLASS_HEATING_VALVE_TIMER                     PACKET_PATH"/CommTimerExtHv"
#define CLASS_RFWUKONG_VALVE_TIMER                    PACKET_PATH"/CommTimerExtRfwk"
#define CLASS_ZH_MOTOR_TIMER                           PACKET_PATH"/CommTimerExtZhMotor"
#define CLASS_LK_THERMOSTAT_TIMER                      PACKET_PATH"/LkThermostatTimer"
#define CLASS_ML_DHX                                   PACKET_PATH"/MlDHXTimer"
#define CLASS_ZH_DHX                                   PACKET_PATH"/CommTimerExtZhDhx"

//汇泰龙门锁
#define CLASS_HTL_LOCK_INFO                            PACKET_PATH"/HtlLockInfo"
#define CLASS_HTL_LOCK_STAT                            PACKET_PATH"/HtlLockStat"
#define CLASS_HTL_LOCK_USER_MANAGE                     PACKET_PATH"/HtlUserManageStat"
#define CLASS_HTL_LOCK_NOTICE                          PACKET_PATH"/HtlNoticeStat"
#define CLASS_HTL_LOCK_HISTORY                         PACKET_PATH"/HtlHistory"
#define CLASS_HTL_LOCK_SET_PIN                         PACKET_PATH"/HtlSetPinParam"

//中科新风空气净化器
#define CLASS_ZK_CLEANNER_INFO                         PACKET_PATH"/ZKCleanner"
#define CLASS_ZK_CLEANNER_STAT                         PACKET_PATH"/ZkCleannerStat"
#define CLASS_ZK_CLEANNER_DAY                          PACKET_PATH"/ZkCleannerStatistics"
#define CLASS_ZK_CLEANNER_SAMPLE                       PACKET_PATH"/ZkCleannerSample"
//悟空i8空调贴
#define CLASS_RF_WK_INFO                                PACKET_PATH"/RfWukongInfo"
#define CLASS_RF_WK_STAT                                PACKET_PATH"/RfWukongStat"
#define CLASS_RF_WK_PAIR                                PACKET_PATH"/RfWukongPair"
//华佑温控器
#define CLASS_HY_THERMOSTAT_INFO                       PACKET_PATH"/HyThermostatInfo"
#define CLASS_HY_THERMOSTAT_STAT                       PACKET_PATH"/HyThermostatStat"
//邦普温控器
#define CLASS_BP_THERMOSTAT_INFO                       PACKET_PATH"/BpThermostatInfo"
#define CLASS_BP_THERMOSTAT_STAT                       PACKET_PATH"/BpThermostatStat"
#define CLASS_BP_THERMOSTAT_TIMER                       PACKET_PATH"/BpThermostatTimer"
#define CLASS_BP_THERMOSTAT_FAULT                       PACKET_PATH"/BpThermostatFault"
//进睿心光芒热水器
#define CLASS_JRX_HEATER_INFO                           PACKET_PATH"/JrxHeaterInfo"
#define CLASS_JRX_HEATER_STAT                           PACKET_PATH"/JrxHeaterStat"
#define CLASS_JRX_HEATER_PERIOD_TIMER                  PACKET_PATH"/JrxHeaterPeriodTimer"
//linkon温控器
#define CLASS_LK_THERMOSTAT_INFO                        PACKET_PATH"/LinkonThermostatInfo"
#define CLASS_LK_THERMOSTAT_STAT                        PACKET_PATH"/LinkonThermostatStat"
//智皇电机
#define CLASS_ZH_MOTOR_INFO                              PACKET_PATH"/ZhMotorInfo"
//创佳温控器
#define CLASS_CJ_THERMOSTAT_INFO                        PACKET_PATH"/CjThermostatInfo"
#define CLASS_CJ_THERMOSTAT_STAT                        PACKET_PATH"/CjThermostatStat"
//夜郎网关
#define CLASS_YL_RFGW_INFO                               PACKET_PATH"/YlRfgwInfo"
// 夜狼声光报警器
#define CLASS_YL_LIGHT_INFO                               PACKET_PATH"/RfYlLightInfo"
#define CLASS_YL_LIGHT_ALARM_CONFIG                           PACKET_PATH"/RfYlLightAlarmConfig"

//一品德仁植物绿化智能控制器
#define CLASS_DRKZQ_INFO							PACKET_PATH"/DrkzqInfo"
#define CLASS_DRKZQ_STAT							PACKET_PATH"/DrkzqStat"
#define CLASS_DRKZQ_FAULT							PACKET_PATH"/DrkzqFault"
#define CLASS_DRKZQ_NAME_TIEM							PACKET_PATH"/DrkzqNameItem"


//电王科技
#define CLASS_DW_INFO                                     PACKET_PATH"/DwRfInfo"
#define CLASS_DW_STAT                                     PACKET_PATH"/DwStat"
#define CLASS_DW_TIMER                                    PACKET_PATH"/DwTimer"
#define CLASS_DW_TIMER_ITEM                              PACKET_PATH"/DwTimerItem"
//情景遥控器
#define CLASS_SCENE_PANEL                                PACKET_PATH"/ScenePanel"
#define CLASS_SCENE_PANEL_KEY                            PACKET_PATH"/ScenePanelKey"
//情景遥控器
#define CLASS_CP_SCENE_PANEL                                PACKET_PATH"/CpScenePanel"
#define CLASS_CP_SCENE_PANEL_KEY                            PACKET_PATH"/CpScenePanelKey"


//局域网用户管理
#define CLASS_LAN_USER_MANAGE                            PACKET_PATH"/LanUserManage"
#define CLASS_LAN_USER_RECORD_ITEM                      PACKET_PATH"/LanUserRecordItem"

//雷士灯数据类
#define CLASS_LEIS_LAMP_STATE PACKET_PATH "/LeisLampState"
//音速灯数据类
#define CLASS_YINSU_LAMP_STATE PACKET_PATH "/YinsuLampState"
//甲醛数据类
#define CLASS_JQ_INFO 			PACKET_PATH"/RFCh2oInfo"
#define CLASS_JQ_STAT 			PACKET_PATH"/RFCh2oStat"
#define CLASS_JQ_HIS_ITEM		PACKET_PATH"/RFCh2oHistoryItem"
//光照感应数据类
#define CLASS_LIGHT_SENSE			             PACKET_PATH"/LightSense"
#define CLASS_LIGHT_SENSE_STATE 			PACKET_PATH"/LightSenseState"
//智皇零火线数据类
#define CLASS_ZHDHX_INFO		             PACKET_PATH"/ZhdhxInfo"
#define CLASS_ZHDHX_NAME			PACKET_PATH"/ZhdhxKeyName"

//分区信息
#define CLASS_FLASH_BLOCK PACKET_PATH "/FlashBlock"

//家庭服务器磁盘信息
#define CLASS_DISK_INFO PACKET_PATH "/DiskInfo"
#define CLASS_DISK_ITEM_INFO PACKET_PATH "/DiskItemInfo"
//家庭服务器网口信息
#define CLASS_ETH_INFO PACKET_PATH "/EthInfo"
#define CLASS_ETH_ITEM_INFO PACKET_PATH "/EthItemInfo"


#ifndef	 SAFE_FREE
#define	SAFE_FREE(x) \
	do { \
		if ((x) != NULL) { \
			free(x); \
			x = NULL; \
		} \
	} while (0)
#endif

#define SAFE_DEL_LOCAL_REF(obj) \
do{\
	if(obj !=NULL){\
		(*env)->DeleteLocalRef(env, obj);\
		obj=NULL;\
	}\
}while(0)

typedef struct {
	// 锟斤拷
	jclass clazz;

	// 锟斤拷员锟斤拷锟斤拷
	jfieldID fid_user_handle;
	jfieldID fid_username;
	jfieldID fid_passwd;
	jfieldID fid_callback_handle;

	// 锟截碉拷锟斤拷锟斤拷
	jmethodID fid_callback;
} user_param_t;

typedef struct {
	jint sub_type;//设备类型
	jint * ext_type;//扩展类型
	jint ext_num;//扩展类型数量
} dev_type_t;//支持的设备结构

//是否初始化支持的设备,仅汇泰龙开源分支使用true(只支持3款设备)
#define IS_INIT_DEV_HTL false
//是否初始化支持的设备,仅汇泰龙开源分支使用true
#define IS_INIT_DEV_HTL_2 false


/*************************************************************************************************/
enum{
	DEV_WUKONG = 0, //悟空
	DEV_WUNENG_USB= 1, //泊鳸SB的悟能
	DEV_WUNENG = 2, //不带USB的悟能
	DEV_JNB_INTER = 3, //杰能宝国际版
	DEV_JNB_DRIP = 4, //杰能宝水滴版
	DEV_FOXCONN_PLUG = 5, //富士康插座
	DEV_HTC_HP = 6, //华天成热彼
	DEV_TMC_JNB = 7,//杰能宝温控器
	DEV_TMC_YL = 8,//亿林温控器
	DEV_PDC_JCX = 9,//金长信配电箱
	DEV_LEDE_TGD = 10,//lede调光灯
	DEV_AMT_FAN = 11, //艾美特风扇
	DEV_CHIFFO_FLOOR_HEATER = 12, //前锋
	DEV_TELIN_HEATING = 13, //特林
	DEV_QP_CP = 14,//千帕茶盘
	DEV_POBIJI = 15,//破壁机
	DEV_CZWK = 16,//车载悟空
	DEV_WUNENG_OEM = 17,//沙特插座, 晴乐插座
	DEV_BIMAR_HEATER = 18,//bimar暖风机
	DEV_XYWKQ = 19,//鑫源温控器
	DEV_QP_PAN = 20,//千帕锅
	DEV_RF_GW_INFO = 21,//网关分组
	DEV_AUCMA_HEATER = 22,//澳柯玛暖风机
	DEV_QP_POBIJI = 23,//千帕破壁机
	DEV_TB_POOL = 24,//华天成泳池机
	DEV_HX_POT = 25,//海迅养生壶
	DEV_YT_AIRPLUG = 26, //月兔空调
	DEV_JL_STAGE_LAMP = 27, //集利舞台灯
	DEV_TB_COMMERCIAL = 28, //华天成商用机
	DEV_ADS = 29, //奥德绅热水器
	DEV_JS_WAVE_OVEN = 30,//晶石微波炉
	DEV_KXM_HEATER_PUMP = 31,//科希曼线控器
	DEV_KXM_HEATER_PUMP_2 = 32,//科希曼温控器
	DEV_SBT_THERMOSTAT = 33,//思博特温控器
	DEV_ZSSX_OVEN = 34,//中山商贤壁挂炉
	DEV_YJ_HEATER = 35,//益佳电暖炉
	DEV_YY_WATER_CLEANER = 36,//御源净水器
	DEV_ZK_WATER_HEATER = 37,//智科热水器
	DEV_INDIA_CAR = 38,//印度车
	DEV_ZK_CLEANNER = 39,//中科新风净化器
	DEV_HY_THERMOSTAT = 40,//宁波华佑温控器
	DEV_BP_THERMOSTAT = 41,//邦普温控器
	DEV_GM_HEATER = 42,//光芒热水器
	DEV_LINKON_THERMOSTAT = 43,//linkon温控器
	DEV_CJ_THERMOSTAT = 44,//创佳热水器
	DEV_YL_RFGW = 45,//夜郎网关
	DEV_DRKZQ = 46, //一品德仁植物绿化智能控制器
	DEV_ZHCL = 47, //WiFi智皇窗帘
	DEV_LEIS = 48, //雷士照明
	DEV_YINSU = 49, //音速照明
	DEV_ZH_DHX = 50,//智皇零火线
	DEV_MAX
};

//获取结构体成员
#define GET_MEMBER(s, m) ((s)->m)
//获取结构体成员指针
#define GET_MEMBER_PTR(s, m) ((void*)&GET_MEMBER(s, m))
//用双引号包裹成字符串
#define WRAP_QUOTE(s) (""#s"")
/*
* 当成员是单个变量时
* 可变参数格式应该是若干个三元组:(类型命，值，成员名)
* 这里用宏 TRIPLES 生成三元组
* 参数:
* type : 类型,如int
* obj  : 结构体指针
* mem  : 结构体成员名
* !注意:java类的成员名必须和clib中的成员名相同
*/
#define TRIPLES(type, obj, mem) WRAP_QUOTE(type),GET_MEMBER(obj, mem),WRAP_QUOTE(mem)
/*
* 当成员是数组时
* 可变参数格式应该是若干个四元组:(类型命，值，成员名)
* 这里用宏 QUADRUPLE 生成四元组
* 参数:
* type : 类型,如int
* obj  : 结构体指针
* mem  : 结构体成员名
* count: 数组成员个数
* !注意:java类的成员名必须和clib中的成员名相同
*/
#define QUADRUPLE(type, obj, mem, count) WRAP_QUOTE(type),GET_MEMBER(obj, mem),count,WRAP_QUOTE(mem)
/*
 *当拷贝类的成员是类的数组时
 *可变参数格式应该是若干个三元组:(类型名，类成员的地址，成员名)
 * 参数:
 * type : 类型,如int
 * obj  : 结构体指针
 * mem  : 结构体成员名
 *!注意:java类的成员名不仅必须和clib中的成员名相同,类型也必须相同
*/
#define ARRAY_TRIPLES(type, obj, mem) WRAP_QUOTE(type),GET_MEMBER_PTR(obj, mem),WRAP_QUOTE(mem)

/*
 *从java拷贝到c的四元组
 * type : 类型,如int
 * obj  : 结构体指针
 * mem  : 结构体成员名
 * count: 数组成员个数
*/
#define ASSIGN_QUAD(type, obj, mem, count) WRAP_QUOTE(type),GET_MEMBER_PTR(obj, mem),count,WRAP_QUOTE(mem)

#define ASSIGN_TRIPLES(type, obj, mem) ARRAY_TRIPLES(type, obj, mem)


#define JNI_TYPE_LONG "long"
#define JNI_TYPE_INT "int"
#define JNI_TYPE_SHORT "short"
#define JNI_TYPE_BYTE "byte"
#define JNI_TYPE_BOOLEAN "boolean"
#define JNI_TYPE_STRING "String"
#define JNI_TYPE_SHORT_BYTE "short_byte" //java是short，sdk用的是u_int8_t，用于值超过127
#define JNI_TYPE_INT_ARRAY "int[]"
#define JNI_TYPE_SHORT_ARRAY "short[]"
#define JNI_TYPE_BYTE_ARRAY "byte[]"
#define JNI_TYPE_STRING_ARRAY "String[]"
#define JNI_TYPE_LONG_ARRAY "long[]"

#define JNI_VAR_ARG_END "end"

extern int jni_copy_simple_class(JNIEnv *env, jclass wrap_class, jobject wrap_object, ...);
extern int jni_assign_simple_struct(JNIEnv* env, jobject orgObj, jclass orgClass, ...);



//拷贝某个类的所有成员的宏，此类为一个简单类，即:只有内置类型的成员变量
#define JNI_COPY_SIMPLE_CLASS(env, wrap_class, wrap_obj, class_name, member_name, ...)  do{\
	jclass class_my = NULL; \
	jobject obj_my = NULL; \
	jfieldID fid;\
	class_my = (*env)->FindClass(env, class_name);  \
	obj_my = (*env)->AllocObject(env, class_my); \
	jni_copy_simple_class(env, class_my, obj_my, __VA_ARGS__) ;\
	fid = (*env)->GetFieldID(env, wrap_class, WRAP_QUOTE(member_name), "L"class_name";"); \
	(*env)->SetObjectField(env, wrap_obj, fid, obj_my); \
	SAFE_DEL_LOCAL_REF(class_my); \
	SAFE_DEL_LOCAL_REF(obj_my); \
}while(0)

/*
 *拷贝某个类的类数组成员
*/
#define JNI_COPY_ARRAY_CLASS(env, wrap_class, wrap_obj, mem_class_name, mem_name, count, data_obj_size,  ...) do {\
	jclass class_my = NULL; \
	jobject obj_my = NULL; \
	jfieldID fid;\
	jobject obj_array = NULL; \
	int i = 0; \
	class_my = (*env)->FindClass(env, mem_class_name);  \
	obj_array = (*env)->NewObjectArray(env, count, class_my, NULL); \
	for ( i = 0; i < count; ++i) { \
		obj_my = (*env)->AllocObject(env, class_my); \
		jni_copy_array_class_item(env, class_my, obj_my, data_obj_size, i, __VA_ARGS__); \
		(*env)->SetObjectArrayElement(env, obj_array, i, obj_my); \
		SAFE_DEL_LOCAL_REF(obj_my); \
	} \
	fid = (*env)->GetFieldID(env, wrap_class, WRAP_QUOTE(mem_name), "[L"mem_class_name";"); \
	(*env)->SetObjectField(env, wrap_obj, fid, obj_array); \
	SAFE_DEL_LOCAL_REF(class_my); \
	SAFE_DEL_LOCAL_REF(obj_my); \
	SAFE_DEL_LOCAL_REF(obj_array); \
}while(0)

//二次开发的初始化函数
int developInitMid(JNIEnv* env);
//二次开发的回调函数
void developNativeCallback(int type,u_int64_t ident,char* data,int len);
void nactivCallback(u_int32_t event, void *user_handle, void *callback_handle);
//锟斤拷锟斤拷锟街凤拷锟斤拷锟斤拷
void jniCopyString(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, char* src);
//锟斤拷锟斤拷short锟斤拷锟斤拷锟斤拷
void jniCopyShortValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, short value);
//锟斤拷锟斤拷Inter锟斤拷锟斤拷锟斤拷
void jniCopyIntValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, int value);
//锟斤拷锟斤拷Long锟斤拷锟斤拷锟斤拷
void jniCopyLongValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, u_int64_t value);
//锟斤拷锟斤拷Bool锟斤拷锟斤拷锟斤拷
void jniCopyBooleanValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, bool value);

#ifdef __cplusplus
}
#endif 

#endif
