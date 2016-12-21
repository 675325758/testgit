#include "clib_jni.h"
#include "clib_jni_sdk.h"
#include "cl_tb_heater_pump.h"
#include "cl_tbb.h"
#include "client_lib.h"
#include "cl_com_rf_dev.h"
#include "cl_hythermostat.h"
#include "cl_yl_thermostat.h"
#include "cl_rfgw.h"
#include "cl_xy.h"


typedef struct {
	int info_bit;
	char mem_name[JNI_MEM_NAME_MAX];
	void (*copy_func)(JNIEnv* , jclass, jobject, char *, int, cl_dev_info_t *);
}copy_dev_info_t;

typedef struct {
	int info_bit;
	char mem_name[JNI_MEM_NAME_MAX];
	void (*copy_func)(JNIEnv* , jclass, jobject, char *, cl_slave_t *);
}copy_slave_info_t;

enum {
	DEV_WHERE_AIRPLUG = 0,
	DEV_WHERE_EPLUG = 1,
	DEV_WHERE_COMMON_UDP = 2,
	DEV_WHERE_SLAVE = 3,
};

/*************************************************************************************************/
enum{
	UDP_DEV_HTC_HP = 0, //华天成热彼
	UDP_DEV_TMC_JNB = 1,//杰能宝温控器
	UDP_DEV_TMC_YL = 2,//亿林温控器
	UDP_DEV_PDC_JCX = 3,//金长信配电箱
	UDP_DEV_LEDE_TGD = 4,//lede调光灯
	UDP_DEV_AMT_FAN = 5, //艾美特风扇
	UDP_DEV_CHIFFO_FLOOR_HEATER = 6, //前锋
	UDP_DEV_TELIN_HEATING = 7, //特林
	UDP_DEV_QP_CP = 8,//千帕茶盘
	UDP_DEV_POBIJI = 9,//破壁机
	UDP_DEV_TB_HOUSE = 10,//华天成家用机
	UDP_DEV_TB_POOL = 11,//华天成泳池机
	UDP_DEV_TB_COMMERCIAL = 12, //华天成商用机
	UDP_DEV_RF_GW = 13,//rf网关
	UDP_DEV_HY_THERMOSTAT = 14,//宁波华佑
	UDP_DEV_KXM_WIRE = 15,//科希曼线控器
	UDP_DEV_KXM_THER = 16,//科希曼温控器
	UDP_DEV_XINYUAN = 17,//鑫源温控器
	UDP_DEV_MAX
};

extern int copy_tb_commercial_stat(JNIEnv* env, jclass class_tb_comm, jobject obj_tb_comm, cl_tbb_status_t *stat) ;
extern int copy_tb_commmercial_config(JNIEnv* env, jclass class_tb_comm, jobject obj_tb_comm, cl_tbb_config_set_t *config) ;

static void copy_dev_common_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_timer_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_elec_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info) ;
static void copy_dev_led_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_smart_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_airplug_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_eplug_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_udp_common_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_udp_device_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_eh_airplug_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);
static void copy_dev_slave_types(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info);




static copy_dev_info_t dev_info_copy_objs[] = { {DEV_INFO_BIT_COMMON,  DEV_COMM_INFO_MEM_NAME, copy_dev_common_info},
	                                              {DEV_INFO_BIT_TIMER, DEV_TIMER_INFO_MEM_NAME, copy_dev_timer_info},
                                                  {DEV_INFO_BIT_ELEC, DEV_ELEC_INFO_MEM_NAME, copy_dev_elec_info}, 
                                                  {DEV_INFO_BIT_LED, DEV_LED_INFO_MEM_NAME, copy_dev_led_info}, 
                                                  {DEV_INFO_BIT_SMART, DEV_SMART_INFO_MEM_NAME, copy_dev_smart_info},
                                                  {DEV_INFO_BIT_AIRPLUG, DEV_AIRPLUG_INFO_MEM_NAME, copy_dev_airplug_info},
                                                  {DEV_INFO_BIT_EPLUG, DEV_EPLUG_INFO_MEM_NAME, copy_dev_eplug_info},
                                                  {DEV_INFO_BIT_UDP_COMMON, DEV_COMM_UDP_INFO_MEM_NAME, copy_dev_udp_common_info},
                                                  {DEV_INFO_BIT_UDP_DEV, DEV_UDP_DEV_INFO_MEM_NAME, copy_dev_udp_device_info},
                                                  {DEV_INFO_BIT_EH_WK, DEV_EH_WK_INFO_MEM_NAME, copy_dev_eh_airplug_info},
                                                  {DEV_INFO_BIT_SLAVES_TYPE, DEV_SLAVES_TYPE_MEM_NAME, copy_dev_slave_types},
                                                 };

static void copy_rf_slave_common_info(JNIEnv* env,jclass class_info, jobject obj_info, char *mem, cl_slave_t *slave_info);
static void copy_rf_slave_history(JNIEnv* env,jclass class_info, jobject obj_info, char *mem, cl_slave_t *slave_info);
static void copy_rf_slave_alarm(JNIEnv* env,jclass class_info, jobject obj_info, char *mem, cl_slave_t *slave_info);
static void copy_rf_slave_dev_info(JNIEnv* env,jclass class_info, jobject obj_info, char *mem, cl_slave_t *slave_info);


static copy_slave_info_t slave_info_copy_objs[] = { {SLAVE_INFO_BIT_RF_SLAVE_COMM, "\0", NULL},
	                                                   {SLAVE_INFO_BIT_RF_SLAVE_COMM, RF_SLAVE_COMM_MEM_NAME, copy_rf_slave_common_info},
	                                                   {SLAVE_INFO_BIT_RF_SLAVE_HISTORY, RF_SLAVE_HISTORY_MEM_NAME, copy_rf_slave_history},
	                                                   {SLAVE_INFO_BIT_RF_SLAVE_ALARM, RF_SLAVE_ALARM_MEM_NAME, copy_rf_slave_alarm},
                                                       {SLAVE_INFO_BIT_RF_DEV, RF_SLAVE_DEV_MEM_NAME, copy_rf_slave_dev_info},
                                                     };

static int udp_amt_type_map(int exttype)
{
	int map_type = UDP_DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_AMT_FAN:
		map_type = UDP_DEV_AMT_FAN;
		break;
	default:
		break;
	}
	return map_type;
}

static int udp_chiffo_type_map(int exttype)
{
	int map_type = UDP_DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_CHIFFO_FlOOR_HEATER:
		map_type = UDP_DEV_CHIFFO_FLOOR_HEATER;
		break;
	default:
		break;
	}
	return map_type;
}

static int udp_telin_type_map(int exttype)
{
	int map_type = UDP_DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_TL_HEATER:
		map_type = UDP_DEV_TELIN_HEATING;
		break;
	default:
		break;
	}
	return map_type;
}

static int udp_tb_map(int exttype)
{
	int map_type = UDP_DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_824_HTC:
		map_type = UDP_DEV_TB_HOUSE;
		break;
	case TYPE_IJ_824_YCJ:
		map_type = UDP_DEV_TB_POOL;
		break;
	case ETYPE_IJ_824_HTC_BUSINESS:
		map_type = UDP_DEV_TB_HOUSE;
		break;
	default:
		break;
	}
	return map_type;
}

static int udp_rfgw_map(int exttype)
{
	int map_type = UDP_DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_RFGW_6621:
		map_type = UDP_DEV_RF_GW;
		break;
	default:
		break;
	}
	return map_type;
}

static int evm_map(int ext_type)
{
	int map_type = UDP_DEV_MAX;
	switch(ext_type) {
	case EYPE_EVM_HYTHERMOSTAT_AC:
	case EYPE_EVM_HYTHERMOSTAT_HT:
		map_type = UDP_DEV_HY_THERMOSTAT;
		break;
	default:
		break;
	}
	return map_type;
}

static int kxm_map(int ext_type)
{
	int map_type = UDP_DEV_MAX;
	switch(ext_type) {
    case EYTYP_IJ_KXM_AC:
	case ETYPE_IJ_KXM_HOST:
		map_type = UDP_DEV_KXM_WIRE;
		break;
	case ETYPE_IJ_KXM_THERMOSTAT:
	case ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB:
	case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
		map_type = UDP_DEV_KXM_THER;
		break;
	case ETYPE_IJ_XY_THERMOSTAT:
		map_type = UDP_DEV_XINYUAN;
	default:
		break;
	}
	return map_type;
}



static int udpTypeMap(int subtype, int exttype) 
{
	int common_dev_type = UDP_DEV_MAX;
	
	switch(subtype) {
	case IJ_822:
		common_dev_type = UDP_DEV_TMC_JNB;
		break;
	case IJ_823:
		common_dev_type = UDP_DEV_TMC_YL;
		break;
	case IJ_830:
		common_dev_type = UDP_DEV_LEDE_TGD;
		break;
	case IJ_840:
		common_dev_type = UDP_DEV_PDC_JCX;
		break;
	case IJ_AMT:
		common_dev_type = udp_amt_type_map(exttype);
		break;
	case IJ_CHIFFO:
		common_dev_type = udp_chiffo_type_map(exttype);
		break;
	case IJ_TL_TEMP:
		common_dev_type = udp_telin_type_map(exttype);
		break;
	case IJ_QPCP:
		common_dev_type = UDP_DEV_QP_CP;
		break;
	case IJ_HXPBJ:
		common_dev_type = UDP_DEV_POBIJI;
		break;
	case IJ_824:
		common_dev_type = udp_tb_map(exttype);
		break;
	case IJ_RFGW:
		common_dev_type = udp_rfgw_map(exttype);
		break;
	case IJ_EVM:
		common_dev_type = evm_map(exttype);
		break;
	case IJ_KXM_DEVICE:
		common_dev_type = kxm_map(exttype);
		break;
	case IJ_TEST_DEV:
		common_dev_type = kxm_map(exttype);
	default:
		break;
	}
	return common_dev_type;
}

static void copy_dev_common_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info) 
{
	jclass class_my = NULL; 
	jobject obj_my = NULL; 
	jfieldID fid;
	class_my = (*env)->FindClass(env, CLASS_SDK_DEV_COMMON);  
	obj_my = (*env)->AllocObject(env, class_my); 
	jni_copy_simple_class(env, class_my, obj_my,
		                                  TRIPLES(int, info, handle),
		                                  TRIPLES(long, info, sn),
		                                  TRIPLES(String, info, name),
		                                  TRIPLES(String, info, passwd),
		                                  TRIPLES(String, info, nickname),
		                                  TRIPLES(boolean, info, is_login),
		                                  TRIPLES(boolean, info, is_online),
		                                  TRIPLES(int, info, last_err),
		                                  TRIPLES(byte, info, sub_type),
		                                  TRIPLES(byte, info, ext_type),
		                                  TRIPLES(String, info, vendor_id),
		                                  TRIPLES(String, info, vendor_url),
		                                  TRIPLES(boolean, info, can_bind_phone),
		                                  TRIPLES(byte, info, login_type),
		                                  TRIPLES(byte, info, net_type),
		                                  TRIPLES(String, info, developer_id),
		                                  JNI_VAR_ARG_END);
	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_DEV_COMMON";"); 
	(*env)->SetObjectField(env, obj_dev, fid, obj_my); 
	SAFE_DEL_LOCAL_REF(class_my); 
	SAFE_DEL_LOCAL_REF(obj_my); 
}

static void copy_base_period_timer(JNIEnv* env,jclass class_timer, jobject obj_timer, cl_period_timer_t *timerData) {			                     
	jni_copy_simple_class(env,class_timer,obj_timer,
		                     TRIPLES(byte, timerData, id),
		                     TRIPLES(boolean, timerData, enable),
		                     TRIPLES(byte, timerData, week),
		                     TRIPLES(byte, timerData, hour),
		                     TRIPLES(byte, timerData, minute),
		                     TRIPLES(boolean, timerData, onoff),
		                     TRIPLES(short, timerData, duration),
		                     TRIPLES(short, timerData, ext_data_type),
		                     JNI_VAR_ARG_END);
}

static void copy_period_timer(JNIEnv* env,jclass class_timer, jobject obj_timer, int ext_type, cl_period_timer_t *timerData) {		
	cl_808_timer_ext_t *extAirData = &timerData->pt_ext_data_u.air_timer_info;

	copy_base_period_timer(env, class_timer, obj_timer, timerData);
	switch(ext_type) {
	case PT_EXT_DT_QPCP:
		jniCopyShortValue(env,class_timer,"scene_id",obj_timer, timerData->pt_ext_data_u.qp_time_info.id);
		break;
	case PT_EXT_DT_808:
		jni_copy_simple_class(env,class_timer,obj_timer,
			                     TRIPLES(boolean, extAirData, onOff),
			                     TRIPLES(byte, extAirData, mode),
			                     TRIPLES(byte, extAirData, temp),
			                     TRIPLES(byte, extAirData, fan_speed),
			                     TRIPLES(byte, extAirData, fan_dir),
			                     TRIPLES(byte, extAirData, key_id),
			                     JNI_VAR_ARG_END);
		break;
	default:
		break;
	}
}

static void copy_dev_timer_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info) 
{
	jclass class_timer_info = NULL;
	jobject object_timer_info = NULL;
	bool is_period_timer = false;
	bool is_period_ext_timer = false;
	jclass classListTimer = NULL;
	jobject objListTimer = NULL;
	jclass classTimer = NULL;
	jobject objTimer = NULL;
	jmethodID addId = NULL;
	jmethodID construct = NULL; 
	int i = 0;
	cl_air_timer_info_t *timer_info = NULL;
	jfieldID fid;
	int timerExtType = PT_EXT_DT_UNKNOWN;
	

	switch(where) {
	case DEV_WHERE_AIRPLUG:
		if (info->air_info != NULL) {
			is_period_timer = info->air_info->is_support_peroid_timer;
			is_period_ext_timer = info->air_info->is_support_peroid_ext_timer;
			timer_info = &info->air_info->air_timer_info;
		}
		break;
	case DEV_WHERE_EPLUG:
		if (info->eb_info != NULL) {
			is_period_timer = info->eb_info->is_support_period_timer;
			is_period_ext_timer = false;
			timer_info = &info->eb_info->timer;
		}
		break;
	case DEV_WHERE_COMMON_UDP:
		if (info->com_udp_info != NULL) {
			is_period_timer = info->com_udp_info->is_support_period_timer;
			is_period_ext_timer = info->com_udp_info->is_support_ext_period_timer;
			timer_info = &info->com_udp_info->timer_info;
		}
		break;
	default:
		break;
	}

	if (timer_info == NULL) {
		return;
	}
	class_timer_info = (*env)->FindClass(env, CLASS_SDK_TIMER);
	object_timer_info = (*env)->AllocObject(env, class_timer_info);

	if (timer_info->timer_count > 0) {
		classListTimer = (*env)->FindClass(env, "java/util/ArrayList");
		construct = (*env)->GetMethodID(env, classListTimer,"<init>","()V"); 
		objListTimer = (*env)->NewObject(env, classListTimer , construct);
		addId = (*env)->GetMethodID(env, classListTimer, "add", "(Ljava/lang/Object;)Z");
		
		if (is_period_timer && timer_info->period_timers != NULL) {
			if (is_period_ext_timer) {
				timerExtType = timer_info->period_timers[0].ext_data_type;
			} else {
				timerExtType = PT_EXT_DT_UNKNOWN;
			}
			switch(timerExtType) {
			case PT_EXT_DT_QPCP:
				classTimer = (*env)->FindClass(env, CLASS_SDK_QPCP_EXT_TIMER);
				break;
			case PT_EXT_DT_808:
				classTimer = (*env)->FindClass(env, CLASS_SDK_WK_EXT_TIMER);
				break;
			default:
				classTimer = (*env)->FindClass(env, CLASS_SDK_PERIOD_TIMER);
				break;
			}

			for (i = 0; i < timer_info->timer_count; ++i) {
				cl_period_timer_t *timerData = timer_info->period_timers + i;
				
				objTimer = (*env)->AllocObject(env, classTimer);
				copy_period_timer(env, classTimer, objTimer, timerExtType, timerData);
				(*env)->CallBooleanMethod(env, objListTimer, addId, objTimer);
				SAFE_DEL_LOCAL_REF(objTimer);
			}
			fid = (*env)->GetFieldID(env, class_timer_info, "timers", "Ljava/util/ArrayList;");
			(*env)->SetObjectField(env, object_timer_info, fid, objListTimer);
	
			SAFE_DEL_LOCAL_REF(classTimer);
		}else if(!is_period_timer && timer_info->timers != NULL)  {
			classTimer = (*env)->FindClass(env, CLASS_SDK_BASE_TIMER);
			for (i = 0; i < timer_info->timer_count; ++i) {
				cl_air_timer_t *timerData = timer_info->timers + i;

				objTimer = (*env)->AllocObject(env, classTimer);    
				jni_copy_simple_class(env,classTimer,objTimer,
					                    TRIPLES(byte, timerData, id),
					                    TRIPLES(boolean, timerData, enable),
					                    TRIPLES(byte, timerData, week),
					                    TRIPLES(byte, timerData, hour),
					                    TRIPLES(byte, timerData, minute),
					                    TRIPLES(boolean, timerData, onoff),
					                    JNI_VAR_ARG_END);
				(*env)->CallBooleanMethod(env, objListTimer, addId, objTimer);
				SAFE_DEL_LOCAL_REF(objTimer);
			}
			fid = (*env)->GetFieldID(env, class_timer_info, "timers", "Ljava/util/ArrayList;");
			(*env)->SetObjectField(env, object_timer_info, fid, objListTimer);
			SAFE_DEL_LOCAL_REF(classTimer);
		}

		SAFE_DEL_LOCAL_REF(classListTimer);
		SAFE_DEL_LOCAL_REF(objListTimer);
	}

	jni_copy_simple_class(env, class_timer_info, object_timer_info,
	                               WRAP_QUOTE(boolean),is_period_timer,WRAP_QUOTE(is_support_period_timer),
	                               WRAP_QUOTE(boolean),is_period_ext_timer,WRAP_QUOTE(is_support_ext_period_timer),
		                           TRIPLES(boolean, timer_info, on_effect),
		                           TRIPLES(boolean, timer_info, off_effect),
		                           TRIPLES(byte, timer_info, timer_count),
		                           TRIPLES(byte, timer_info, next_on_day),
		                           TRIPLES(byte, timer_info, next_on_hour),
		                           TRIPLES(byte, timer_info, next_on_min),
		                           TRIPLES(byte, timer_info, next_off_day),
		                           TRIPLES(byte, timer_info, next_off_hour),
		                           TRIPLES(byte, timer_info, next_off_min),
		                           TRIPLES(short, timer_info, on_minute),
		                           TRIPLES(short, timer_info, off_minute),
		                           JNI_VAR_ARG_END);
	fid = (*env)->GetFieldID(env, class_dev, mem, "L" CLASS_SDK_TIMER ";");
	(*env)->SetObjectField(env, obj_dev, fid, object_timer_info);

	SAFE_DEL_LOCAL_REF(class_timer_info);
	SAFE_DEL_LOCAL_REF(object_timer_info);
}

static void copy_elec_item(JNIEnv* env, jclass class_elec, jobject obj_elec, char *attrname, cl_air_elec_item_info* item_info)
{
	jclass class_my = NULL; 
	jobject obj_my = NULL; 
	jfieldID fid;
	class_my = (*env)->FindClass(env, CLASS_SDK_ELEC_ITEM);  
	obj_my = (*env)->AllocObject(env, class_my); 
	
	jni_copy_simple_class(env, class_my, obj_my,
		                                 TRIPLES(int, item_info, begin_time),
		                                 TRIPLES(int, item_info, elec),
		                                 JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_elec, attrname, "L"CLASS_SDK_ELEC_ITEM";"); 
	(*env)->SetObjectField(env, obj_elec, fid, obj_my); 
	SAFE_DEL_LOCAL_REF(class_my); 
	SAFE_DEL_LOCAL_REF(obj_my); 
}

static void copy_elec_days_info(JNIEnv* env, jclass class_elec, jobject obj_elec, cl_elec_days_stat_info* days_info)
{
	JNI_COPY_SIMPLE_CLASS(env, class_elec, obj_elec, CLASS_SDK_ELEC_DAYS, elec_days_info, 
		                                  TRIPLES(boolean, days_info, is_info_valid),
		                                  TRIPLES(short, days_info, days_count),
		                                  TRIPLES(int, days_info, nearest_data_time),
		                                  QUADRUPLE(short[], days_info, elec_data, days_info->days_count),
		                                  JNI_VAR_ARG_END);
}

static void copy_elec_stat(JNIEnv* env, jclass class_elec, jobject obj_elec, cl_elec_stat_info_t* stat_info)
{
	jclass class_state = NULL;
	jobject obj_state = NULL;
	jfieldID fid;

	class_state = (*env)->FindClass(env, CLASS_SDK_ELEC_STAT);
	obj_state = (*env)->AllocObject(env, class_state);

	jni_copy_simple_class(env, class_state, obj_state, 
		                           QUADRUPLE(int[], stat_info, month_peak, MONTH_PER_YEAR),
		                           QUADRUPLE(int[], stat_info, month_valley, MONTH_PER_YEAR),
		                           QUADRUPLE(int[], stat_info, month_normal, MONTH_PER_YEAR),
		                           TRIPLES(int, stat_info, peak_price),
		                           TRIPLES(int, stat_info, valley_price),
		                           TRIPLES(int, stat_info, flat_price),
		                           JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_state, obj_state, CLASS_SDK_ELEC_PERIOD, peak_time, 
		                                 TRIPLES(short, &stat_info->peak_time, begin_minute),
		                                 TRIPLES(short, &stat_info->peak_time, last_minute),
		                                 JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_state, obj_state, CLASS_SDK_ELEC_PERIOD, valley_time, 
		                                 TRIPLES(short, &stat_info->valley_time, begin_minute),
		                                 TRIPLES(short, &stat_info->valley_time, last_minute),
		                                 JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_state, obj_state, CLASS_SDK_ELEC_PERIOD, flat_time, 
		                                 TRIPLES(short, &stat_info->flat_time, begin_minute),
		                                 TRIPLES(short, &stat_info->flat_time, last_minute),
		                                 JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_elec, "elec_stat_info", "L"CLASS_SDK_ELEC_STAT";");
	(*env)->SetObjectField(env, obj_elec, fid, obj_state);

	SAFE_DEL_LOCAL_REF(class_state);
	SAFE_DEL_LOCAL_REF(obj_state);
}

static void copy_elec_info_air(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, cl_air_info_t *air_info) 
{
	jclass class_elec = NULL;
	jobject obj_elec = NULL;
	jfieldID fid;

	class_elec = (*env)->FindClass(env, CLASS_SDK_ELEC);
	obj_elec = (*env)->AllocObject(env, class_elec);

	jni_copy_simple_class(env, class_elec, obj_elec, 
		                           TRIPLES(int, air_info, cur_power),
		                           TRIPLES(int, air_info, cur_milli_power),
		                           TRIPLES(boolean, air_info, is_support_elec_ajust),
		                           TRIPLES(short, air_info, elec_ajust_value),
		                           JNI_VAR_ARG_END);

	copy_elec_item(env, class_elec, obj_elec, "total_elec", &air_info->total_elec);
	copy_elec_item(env, class_elec, obj_elec, "period_elec", &air_info->period_elec);
	copy_elec_item(env, class_elec, obj_elec, "last_on_elec", &air_info->last_on_elec);
	copy_elec_stat(env, class_elec, obj_elec, &air_info->air_elec_stat_info);
	copy_elec_days_info(env, class_elec, obj_elec, &air_info->elec_days_info);

	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_ELEC";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_elec);

	SAFE_DEL_LOCAL_REF(class_elec);
	SAFE_DEL_LOCAL_REF(obj_elec);
}

static void copy_elec_info_eplug(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, cl_common_elec_info *eplug_elec_info) 
{
	jclass class_elec = NULL;
	jobject obj_elec = NULL;
	jfieldID fid;

	if (!eplug_elec_info->is_support_elec_info) {
		return;
	}

	class_elec = (*env)->FindClass(env, CLASS_SDK_ELEC);
	obj_elec = (*env)->AllocObject(env, class_elec);

	jniCopyIntValue(env, class_elec, "cur_power", obj_elec, eplug_elec_info->current_power);
	jniCopyIntValue(env, class_elec, "cur_milli_power", obj_elec, eplug_elec_info->current_mil_power);
	
	copy_elec_item(env, class_elec, obj_elec, "total_elec", &eplug_elec_info->total_elec);
	copy_elec_item(env, class_elec, obj_elec, "period_elec", &eplug_elec_info->period_elec);
	copy_elec_item(env, class_elec, obj_elec, "last_on_elec", &eplug_elec_info->last_on_elec);
	copy_elec_stat(env, class_elec, obj_elec, &eplug_elec_info->elec_stat_info);
	copy_elec_days_info(env, class_elec, obj_elec, &eplug_elec_info->elec_days_info);

	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_ELEC";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_elec);
	
	SAFE_DEL_LOCAL_REF(class_elec);
	SAFE_DEL_LOCAL_REF(obj_elec);
}

static void copy_elec_info_comm(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, cl_com_udp_device_data *comm_elec_info)
{
	jclass class_elec = NULL;
	jobject obj_elec = NULL;
	jfieldID fid;

	if (!comm_elec_info->is_suppport_elec_stat) {
		return;
	}

	class_elec = (*env)->FindClass(env, CLASS_SDK_ELEC);
	obj_elec = (*env)->AllocObject(env, class_elec);

	jniCopyIntValue(env, class_elec, "cur_power", obj_elec, comm_elec_info->current_power);
	jniCopyIntValue(env, class_elec, "cur_milli_power", obj_elec, comm_elec_info->cur_milli_power);

	copy_elec_item(env, class_elec, obj_elec, "total_elec", &comm_elec_info->total_elec);
	copy_elec_item(env, class_elec, obj_elec, "period_elec", &comm_elec_info->period_elec);
	copy_elec_item(env, class_elec, obj_elec, "last_on_elec", &comm_elec_info->last_on_elec);
	copy_elec_stat(env, class_elec, obj_elec, &comm_elec_info->elec_stat_info);
	copy_elec_days_info(env, class_elec, obj_elec, &comm_elec_info->elec_days_info);

	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_ELEC";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_elec);

	SAFE_DEL_LOCAL_REF(class_elec);
	SAFE_DEL_LOCAL_REF(obj_elec);
}

static void copy_dev_elec_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info) 
{
	switch(where) {
	case DEV_WHERE_AIRPLUG:
		if (info->air_info != NULL) {
			copy_elec_info_air(env, class_dev, obj_dev, mem, info->air_info);
		}
		break;
	case DEV_WHERE_EPLUG:
		if (info->eb_info != NULL) {
			copy_elec_info_eplug(env, class_dev, obj_dev, mem, &info->eb_info->elec_info);
		}
		break;
	case DEV_WHERE_COMMON_UDP:
		if (info->com_udp_info != NULL) {
			copy_elec_info_comm(env, class_dev, obj_dev, mem, info->com_udp_info);
		}
		break;
	default:
		break;
	}

}

static void copy_led_info_air(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, cl_air_info_t *air_info)
{
	jclass class_led = NULL;
	jobject obj_led = NULL;
	jfieldID fid;
	
	class_led = (*env)->FindClass(env, CLASS_SDK_LED);
	obj_led = (*env)->AllocObject(env, class_led);

	jniCopyIntValue(env, class_led, "led_mod", obj_led, air_info->air_led_on_off);
	jniCopyBooleanValue(env, class_led, "is_support_led_color", obj_led, air_info->is_support_led_color);

	if (air_info->is_support_led_color) {
		JNI_COPY_SIMPLE_CLASS(env, class_led, obj_led, CLASS_SDK_LED_COLOR, led_color, 
			                                  TRIPLES(byte, &air_info->led_color, air_on_color),
			                                  TRIPLES(byte, &air_info->led_color, air_off_color),
			                                  JNI_VAR_ARG_END);
	}


	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_LED";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_led);

	SAFE_DEL_LOCAL_REF(class_led);
	SAFE_DEL_LOCAL_REF(obj_led);
}

static void copy_dev_led_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	switch(where) {
	case DEV_WHERE_AIRPLUG:
		if (info->air_info != NULL) {
			copy_led_info_air(env, class_dev, obj_dev, mem, info->air_info);
		}
		break;
	default:
		break;
	}
}

static void copy_smart_info_air(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, cl_air_info_t *air_info)
{
	jclass class_smart = NULL;
	jobject obj_smart = NULL;
	jfieldID fid;
	
	class_smart = (*env)->FindClass(env, CLASS_SDK_SMART);
	obj_smart = (*env)->AllocObject(env, class_smart);

	jni_copy_simple_class(env, class_smart, obj_smart, 
		                           TRIPLES(boolean, air_info, is_smart_on_data_valid),
		                           TRIPLES(boolean, air_info, smart_on_enable),
		                           TRIPLES(boolean, air_info, smart_off_enable),
		                           TRIPLES(boolean, air_info, smart_sleep_enable),
			                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_smart, obj_smart, CLASS_SDK_SMART_ON_PARAM, smart_on_info, 
		                                  TRIPLES(boolean, &air_info->smart_on_info, on),
		                                  TRIPLES(boolean, &air_info->smart_on_info, push_on),
		                                  TRIPLES(boolean, &air_info->smart_on_info, sum_on),
		                                  TRIPLES(byte, &air_info->smart_on_info, sum_tmp),
		                                  TRIPLES(boolean, &air_info->smart_on_info, win_on),
		                                  TRIPLES(byte, &air_info->smart_on_info, win_tmp),
		                                  TRIPLES(boolean, &air_info->smart_on_info, home_on),
		                                  JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_smart, obj_smart, CLASS_SDK_SMART_OFF_PARAM, smart_off_info,
		                                  TRIPLES(boolean, &air_info->smart_off_info, on),
		                                  TRIPLES(boolean, &air_info->smart_off_info, push_on),
		                                  TRIPLES(byte, &air_info->smart_off_info, off_time),
		                                  JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_smart, obj_smart, CLASS_SDK_SMART_SLEEP_PARAM, smart_sleep_info,
		                                  TRIPLES(boolean, &air_info->smart_sleep_info, on),
		                                  JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_SMART";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_smart);

	SAFE_DEL_LOCAL_REF(class_smart);
	SAFE_DEL_LOCAL_REF(obj_smart);
}

static void copy_dev_smart_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	switch(where) {
	case DEV_WHERE_AIRPLUG:
		if (info->air_info != NULL) {
			copy_smart_info_air(env, class_dev, obj_dev, mem, info->air_info);
		}
		break;
	default:
		break;
	}
}

static void copy_air_match_info(JNIEnv* env,jclass class_airplug, jobject obj_airplug, char *mem, cl_ac_code_match_info_t *match_info)
{
	jclass class_match = NULL;
	jobject obj_match = NULL;
	jfieldID fid;


	class_match = (*env)->FindClass(env, CLASS_SDK_AC_CODE_MATCH);
	obj_match = (*env)->AllocObject(env, class_match);

	jniCopyShortValue(env, class_match, "cur_match_id",obj_match, match_info->cur_match_id);
	jniCopyShortValue(env, class_match, "code_num",obj_match, match_info->code_num);

	if (match_info->code_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_match, obj_match, CLASS_SDK_AC_CODE, 
			                    items , match_info->code_num, sizeof(cl_ac_code_item_t),
			                    ARRAY_TRIPLES(short, match_info->items, c_id),
			                    ARRAY_TRIPLES(byte, match_info->items, is_on),
			                    ARRAY_TRIPLES(byte, match_info->items, mode),
			                    ARRAY_TRIPLES(byte, match_info->items, temp),
			                    ARRAY_TRIPLES(byte, match_info->items, fan),
			                    ARRAY_TRIPLES(byte, match_info->items, fan_dir),
			                    ARRAY_TRIPLES(byte, match_info->items, key),
			                    JNI_VAR_ARG_END);
	}
	


	fid = (*env)->GetFieldID(env, class_airplug, mem, "L"CLASS_SDK_AC_CODE_MATCH";");
	(*env)->SetObjectField(env, obj_airplug, fid, obj_match);
	
	SAFE_DEL_LOCAL_REF(class_match);
	SAFE_DEL_LOCAL_REF(obj_match);
}


static void copy_air_no_screen_info(JNIEnv* env,jclass class_airplug, jobject obj_airplug, char *mem, cl_no_screen_key_info *key_info)
{
	jclass class_nc = NULL;
	jobject obj_nc = NULL;
	jfieldID fid;


	class_nc = (*env)->FindClass(env, CLASS_SDK_AC_NC_KEY);
	obj_nc = (*env)->AllocObject(env, class_nc);

	jniCopyByteValue(env, class_nc,"key_num", obj_nc, key_info->key_num);
	if (key_info->key_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_nc, obj_nc, CLASS_SDK_AC_KEY, 
			                    keys , key_info->key_num, sizeof(cl_air_key),
			                    ARRAY_TRIPLES(byte, key_info->keys, key_id),
			                    ARRAY_TRIPLES(boolean, key_info->keys, is_support_learn),
			                    ARRAY_TRIPLES(boolean, key_info->keys, is_support_change_name),
			                    ARRAY_TRIPLES(boolean, key_info->keys, is_support_delete),
			                    ARRAY_TRIPLES(boolean, key_info->keys, is_learn_code),
			                    ARRAY_TRIPLES(String, key_info->keys, name),
			                    JNI_VAR_ARG_END);
	}

	fid = (*env)->GetFieldID(env, class_airplug, mem, "L"CLASS_SDK_AC_NC_KEY";");
	(*env)->SetObjectField(env, obj_airplug, fid, obj_nc);
	
	SAFE_DEL_LOCAL_REF(class_nc);
	SAFE_DEL_LOCAL_REF(obj_nc);
}

static void copy_dev_udp_common_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	jclass class_udp_comm = NULL;
	jobject obj_udp_comm = NULL;
	jfieldID fid;
	cl_com_udp_device_data *udp_info = info->com_udp_info;
	
	if (udp_info == NULL) {
		return;
	}

	class_udp_comm = (*env)->FindClass(env, CLASS_SDK_COMM_UDP);
	obj_udp_comm = (*env)->AllocObject(env, class_udp_comm);

	jni_copy_simple_class(env,class_udp_comm,obj_udp_comm,
		                     TRIPLES(byte,udp_info,is_lan_connect),
		                     TRIPLES(byte,udp_info,is_all_data_update),
		                     TRIPLES(boolean,udp_info,is_system_info_valid),
		                     TRIPLES(boolean,udp_info,is_stat_info_valid),
		                     TRIPLES(boolean,udp_info,is_support_stm_upgrade),
		                     TRIPLES(boolean,udp_info,is_support_dev_restory_factory),
		                     TRIPLES(boolean,udp_info,is_support_dev_set_wifi_param),
		                     JNI_VAR_ARG_END);
	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_COMM_UDP";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_udp_comm);
	SAFE_DEL_LOCAL_REF(class_udp_comm);
	SAFE_DEL_LOCAL_REF(obj_udp_comm);
}

static void copy_udp_tb_pc_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_tb_pc = NULL;
	jobject obj_tb_pc = NULL;
	jfieldID fid = NULL;
	cl_tbb_info_t *tb_pc_info = (cl_tbb_info_t *)udp_info->device_info;

	if (tb_pc_info == NULL) {
		return;
	}

	class_tb_pc = (*env)->FindClass(env, CLASS_SDK_TB_PC);
	obj_tb_pc = (*env)->AllocObject(env, class_tb_pc);

	jni_copy_simple_class(env,class_tb_pc,obj_tb_pc,
		                    TRIPLES(boolean ,tb_pc_info, on),
		                    TRIPLES(byte ,tb_pc_info, mode),
		                    TRIPLES(short ,tb_pc_info, tmp),
		                    JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env, class_tb_pc, obj_tb_pc,
		                     CLASS_SDK_TB_PC_UPGRADE, upgrade_info,
		                     TRIPLES(int ,&tb_pc_info->upgrade_info, upgradeing),
		                     TRIPLES(int ,&tb_pc_info->upgrade_info, upgrade_role),
		                     TRIPLES(int ,&tb_pc_info->upgrade_info, upgrade_state),
		                     TRIPLES(int ,&tb_pc_info->upgrade_info, up_state),
		                     JNI_VAR_ARG_END);

	copy_tb_commercial_stat(env, class_tb_pc, obj_tb_pc, &tb_pc_info->status);	
	JNI_COPY_SIMPLE_CLASS(env, class_tb_pc, obj_tb_pc,
		                     CLASS_SDK_TB_PC_VER, hd_ver, 
		                     TRIPLES(short, &tb_pc_info->hd_ver, sys_type),
		                     TRIPLES(short, &tb_pc_info->hd_ver, ele_band_mcu),
		                     TRIPLES(short, &tb_pc_info->hd_ver, ele_band_ver),
		                     TRIPLES(short, &tb_pc_info->hd_ver, line_band_mcu),
		                     TRIPLES(short, &tb_pc_info->hd_ver, line_band_ver),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_tb_pc, obj_tb_pc,
		                     CLASS_SDK_TB_PC_BIND, bindinfo,
		                     TRIPLES(short ,&tb_pc_info->bindinfo, dev_state),
		                     TRIPLES(short ,&tb_pc_info->bindinfo, bind_state),
		                     TRIPLES(String ,&tb_pc_info->bindinfo, tb_sn),
		                     JNI_VAR_ARG_END);

	copy_tb_commmercial_config(env, class_tb_pc, obj_tb_pc, &tb_pc_info->config);

	fid = (*env)->GetFieldID(env, class_dev, "tbPC", "L" CLASS_SDK_TB_PC ";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_tb_pc);
	
	SAFE_DEL_LOCAL_REF(class_tb_pc);
	SAFE_DEL_LOCAL_REF(obj_tb_pc);
}


static void copy_udp_tb_house_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_tbhp = NULL;
	jobject object_tbhp = NULL;
	jfieldID fid;
	cl_tb_info_t *tbhp_dev = (cl_tb_info_t*)udp_info->device_info;

	class_tbhp = (*env)->FindClass(env, CLASS_SDK_TB_HOUSE);
	object_tbhp = (*env)->AllocObject(env, class_tbhp);

	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp, 
		                                 CLASS_SDK_TB_HOUSE_UCONFIG, u_config,
		                                 TRIPLES(int, &tbhp_dev->u_config, cid),
		                                 TRIPLES(boolean, &tbhp_dev->u_config, onoff),
		                                 TRIPLES(int, &tbhp_dev->u_config, work_mode),
		                                 TRIPLES(int, &tbhp_dev->u_config, temp),
		                                 JNI_VAR_ARG_END);
		
	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp,
		                                  CLASS_SDK_TB_HOUSE_WCONFIG, w_config,
		                                  TRIPLES(int, &tbhp_dev->w_config, cid),
		                                  TRIPLES(boolean, &tbhp_dev->w_config, return_cold_switch),
		                                  TRIPLES(int, &tbhp_dev->w_config, facility_state),
		                                  TRIPLES(int, &tbhp_dev->w_config, sysfunc),
		                                  TRIPLES(int, &tbhp_dev->w_config, return_diff_temp),
		                                  TRIPLES(int, &tbhp_dev->w_config, heat_defrost_circle),
		                                  TRIPLES(byte, &tbhp_dev->w_config, start_heat_defrost_temp),
		                                  TRIPLES(int, &tbhp_dev->w_config, stop_heat_defrost_temp),
		                                  TRIPLES(int, &tbhp_dev->w_config, stop_heat_defrost_time),
		                                  TRIPLES(byte, &tbhp_dev->w_config, eheat_value),
		                                  TRIPLES(int, &tbhp_dev->w_config, backlight_delay_time),
		                                  TRIPLES(int, &tbhp_dev->w_config, fan_mode),
		                                  JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp,
		                                  CLASS_SDK_TB_HOUSE_TEMP, temp_info,
		                                  TRIPLES(int, &tbhp_dev->temp_info, cid),
		                                  TRIPLES(int, &tbhp_dev->temp_info, env_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, tankbottom_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, tanktop_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, coil_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, air_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, returnair_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, outwater_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, inwater_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, returnwater_temp),
		                                  TRIPLES(int, &tbhp_dev->temp_info, heat_capacity),
		                                  TRIPLES(int, &tbhp_dev->temp_info, heat_time),
		                                  TRIPLES(int, &tbhp_dev->temp_info, consumption_power),
		                                  TRIPLES(int, &tbhp_dev->temp_info, saving_power),
		                                  JNI_VAR_ARG_END);
	
	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp,
		                                  CLASS_SDK_TB_HOUSE_FAULT, fault_stat,
		                                  TRIPLES(int, &tbhp_dev->fault_stat, cid),
		                                  TRIPLES(boolean, &tbhp_dev->fault_stat, valve_expansion),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, slave_onoff),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, dev_fault),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, dev_guard),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, load_state),
		                                  JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp,
		                                  CLASS_SDK_TB_HOUSE_OTHER, other_info,
		                                  TRIPLES(int, &tbhp_dev->other_info, cid),
		                                  TRIPLES(int, &tbhp_dev->other_info, dev_info),
		                                  TRIPLES(int, &tbhp_dev->other_info, dev_mode),
		                                  TRIPLES(int, &tbhp_dev->other_info, fw_version),
		                                  TRIPLES(int, &tbhp_dev->other_info, mb_version),
		                                  TRIPLES(int, &tbhp_dev->other_info, svn_version),
		                                  TRIPLES(byte, &tbhp_dev->other_info, stm_up_stat),
		                                  JNI_VAR_ARG_END);

	jni_copy_simple_class(env, class_tbhp, object_tbhp,  
		                       TRIPLES(int, tbhp_dev, bind_state),
		                       TRIPLES(String, tbhp_dev, tb_sn),
		                       JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_dev, "tbHouse", "L" CLASS_SDK_TB_HOUSE ";");
	(*env)->SetObjectField(env, obj_dev, fid, object_tbhp);
	
	SAFE_DEL_LOCAL_REF(class_tbhp);
	SAFE_DEL_LOCAL_REF(object_tbhp);
}

static jobject copy_udp_rfgw_remote_key(JNIEnv *env, cl_lamp_remote_key_info *key)
{
	jclass class_key = NULL;
	jclass object_key = NULL;
	jobjectArray array_remote = NULL;
	jfieldID fid = NULL;
	int i = 0;

	class_key = (*env)->FindClass(env, CLASS_SDK_RFGW_REMOTE_KEY);
	object_key = (*env)->AllocObject(env, class_key);

	jni_copy_simple_class(env, class_key, object_key,
		                    TRIPLES(short, key, slave_count),
		                    TRIPLES(short, key, real_count),
		                    TRIPLES(byte, key, lamp_type),
		                    QUADRUPLE(int[], key, slave_handle, key->real_count),
		                    JNI_VAR_ARG_END);
	SAFE_DEL_LOCAL_REF(class_key);
	return object_key;
}

static void copy_udp_rfgw_lamp_remote(JNIEnv *env, jclass class_rfgw, jobject obj_rfgw, cl_gw_info_t *rfgw)
{
	jclass class_remote = NULL, class_key = NULL;
	jobject object_remote = NULL, obj_key = NULL;
	jobjectArray array_remote = NULL, array_key = NULL;
	jfieldID fid = NULL;
	int i = 0, j = 0;

	class_remote = (*env)->FindClass(env, CLASS_SDK_RFGW_LAMP_REMOTE);
	class_key = (*env)->FindClass(env, CLASS_SDK_RFGW_REMOTE_KEY);
	if (rfgw->lamp_remote_cnt > 0 && rfgw->lr_info != NULL) 
	{
		array_remote = (*env)->NewObjectArray(env, rfgw->dev_group_cnt, class_remote, NULL);
		for (i = 0; i < rfgw->lamp_remote_cnt; ++i) {
			object_remote= (*env)->AllocObject(env, class_remote);
			
			jni_copy_simple_class(env, class_remote, object_remote,
				                    TRIPLES(int, &rfgw->lr_info[i], handle),
				                    TRIPLES(int, &rfgw->lr_info[i], r_id),
				                    TRIPLES(byte, &rfgw->lr_info[i], lamp_type),
 				                    JNI_VAR_ARG_END);
			array_key = (*env)->NewObjectArray(env,4, class_key, NULL);
			for (j = 0; j < 4; ++j) {
				obj_key = copy_udp_rfgw_remote_key(env, &rfgw->lr_info[i].key[j]);
				(*env)->SetObjectArrayElement(env, array_key, j, obj_key);
				SAFE_DEL_LOCAL_REF(obj_key);
			}
			fid = (*env)->GetFieldID(env, class_remote, "key", "[L"CLASS_SDK_RFGW_REMOTE_KEY";");
			(*env)->SetObjectField(env, object_remote, fid, array_key);
			SAFE_DEL_LOCAL_REF(array_key);
		}
		fid = (*env)->GetFieldID(env, class_rfgw, "lr_info", "[L"CLASS_SDK_RFGW_LAMP_REMOTE";");
		(*env)->SetObjectField(env, obj_rfgw, fid, array_remote);
		SAFE_DEL_LOCAL_REF(array_remote);
	}
	SAFE_DEL_LOCAL_REF(class_remote);
	SAFE_DEL_LOCAL_REF(class_key);
}

static jobject copy_kxm_wire_stat_item(JNIEnv* env, cl_kxm_info_t *kxm_info, int index)
{
	jclass class_kxm_stat = NULL;
	jobject obj_kxm_stat = NULL;
	jfieldID fid = NULL;
	cl_kxm_pump_stat_info_t *pump = &kxm_info->pump[index];

	class_kxm_stat = (*env)->FindClass(env, CLASS_SDK_KXM_WIRE_STAT);
	obj_kxm_stat = (*env)->AllocObject(env, class_kxm_stat);

	jni_copy_simple_class(env, class_kxm_stat, obj_kxm_stat,
		                     TRIPLES(boolean, pump, is_online),
		                     TRIPLES(byte, pump, machine_type),
		                     TRIPLES(byte, pump, back_water_temp),
		                     TRIPLES(byte, pump, water_temp),
		                     TRIPLES(byte, pump, env_temp),
		                     TRIPLES(byte, pump, water_box_temp),
		                     TRIPLES(byte, pump, in_water_temp),
		                     TRIPLES(byte, pump, out_water_temp),
		                     TRIPLES(byte, pump, run_stat),
		                     TRIPLES(byte, pump, water_pos),
		                     TRIPLES(boolean, pump, is_fan_high),
		                     TRIPLES(boolean, pump, is_fan_low),
		                     TRIPLES(boolean, pump, cir_pump_run),
		                     TRIPLES(boolean, pump, back_pump_run),
		                     TRIPLES(boolean, pump, in_water_pump_run),
		                     TRIPLES(boolean, pump, water_pump_run),
		                     TRIPLES(boolean, pump, is_elec_hot_run),
		                     TRIPLES(boolean, pump, sw_dir_tap_run),
		                     TRIPLES(boolean, pump, sensor_back_water_fault),
		                     TRIPLES(boolean, pump, sensor_water_fault),
		                     TRIPLES(boolean, pump, sensor_env_fault),
		                     TRIPLES(boolean, pump, sensor_water_box_fault),
		                     TRIPLES(boolean, pump, sensor_in_water_fault),
		                     TRIPLES(boolean, pump, sensor_out_water_fault),
		                     TRIPLES(boolean, pump, is_out_water_temp_low_fault),
		                     TRIPLES(boolean, pump, is_out_water_temp_high_fault),
		                     TRIPLES(boolean, pump, is_in_out_temp_big_fault),
		                     TRIPLES(boolean, pump, is_anti_phase_fault),
		                     TRIPLES(boolean, pump, is_no_phase_L2_fault),
		                     TRIPLES(boolean, pump, is_no_phase_L3_fault),
		                     TRIPLES(boolean, pump, is_ctrl_comu_fault),
		                     JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_kxm_stat, obj_kxm_stat, CLASS_SDK_KXM_WIRE_SUB_STAT,
		                    sub_system_info, MAX_KXM_SUB_SYSTEM , sizeof(cl_kxm_sub_system_stat_t),
		                    ARRAY_TRIPLES(byte, pump->sub_system_info, scoll_temp),
		                    ARRAY_TRIPLES(byte, pump->sub_system_info, inhale_temp),
		                    ARRAY_TRIPLES(byte, pump->sub_system_info, exhaust_temp),
		                    ARRAY_TRIPLES(byte, pump->sub_system_info, exv_value),
		                    ARRAY_TRIPLES(byte, pump->sub_system_info, compress_stat),
		                    ARRAY_TRIPLES(byte, pump->sub_system_info, spray_stat),
		                    ARRAY_TRIPLES(boolean, pump->sub_system_info, is_low_press_fault),
		                    ARRAY_TRIPLES(boolean, pump->sub_system_info, is_high_press_fault),
		                    ARRAY_TRIPLES(boolean, pump->sub_system_info, is_over_curr_fault),
		                    ARRAY_TRIPLES(boolean, pump->sub_system_info, is_exhault_fault),
		                    JNI_VAR_ARG_END);
	SAFE_DEL_LOCAL_REF(class_kxm_stat);

	return obj_kxm_stat;
}


static void copy_kxm_wire_stat(JNIEnv* env, jclass class_kxm, jobject obj_kxm, cl_kxm_info_t *kxm_info)
{
	jclass class_kxm_stat = NULL;
	jobject obj_array = NULL;
	jobject obj_kxm_stat = NULL;
	jfieldID fid = NULL;
	int i = 0;

	class_kxm_stat = (*env)->FindClass(env, CLASS_SDK_KXM_WIRE_STAT);
	obj_array = (*env)->NewObjectArray(env, MAX_KXM_SUB_PUMP_NUM, class_kxm_stat, NULL);

	for (i = 0; i < MAX_KXM_SUB_PUMP_NUM; ++i) {
		obj_kxm_stat = copy_kxm_wire_stat_item(env, kxm_info, i);
		(*env)->SetObjectArrayElement(env, obj_array, i, obj_kxm_stat);
		SAFE_DEL_LOCAL_REF(obj_kxm_stat);
	}

	fid = (*env)->GetFieldID(env, class_kxm,  "pump", "[L"CLASS_SDK_KXM_WIRE_STAT";");
	(*env)->SetObjectField(env, obj_kxm, fid, obj_array);
	
	SAFE_DEL_LOCAL_REF(class_kxm_stat);
	SAFE_DEL_LOCAL_REF(obj_array);
}

static void copy_kxm_wire_host(JNIEnv* env, jclass class_kxm, jobject obj_kxm, cl_kxm_info_t *kxm_info)
{
	jclass class_kxm_host = NULL;
	jobject obj_kxm_host = NULL;
	jfieldID fid = NULL;

	class_kxm_host = (*env)->FindClass(env, CLASS_SDK_KXM_WIRE_HOST);
	obj_kxm_host = (*env)->AllocObject(env, class_kxm_host);
	
	jni_copy_simple_class(env, class_kxm_host,obj_kxm_host,
		                     TRIPLES(boolean, &kxm_info->hinfo, on_off),
		                     TRIPLES(byte, &kxm_info->hinfo, mode),
		                     TRIPLES(byte, &kxm_info->hinfo, hot_water_setting_temp),
		                     TRIPLES(byte, &kxm_info->hinfo, hot_setting_temp),
		                     TRIPLES(byte, &kxm_info->hinfo, cold_setting_temp),
		                     TRIPLES(byte, &kxm_info->hinfo, t_hour),
		                     TRIPLES(byte, &kxm_info->hinfo, t_min),
		                     JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_kxm_host, obj_kxm_host, CLASS_SDK_KXM_WIRE_TIMER,
		                   timer, MAX_KXM_TIMER_CNT, sizeof(cl_kxm_timer_info_t),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, on_hour),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, on_min),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, off_hour),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, off_min),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_kxm, "hinfo", "L" CLASS_SDK_KXM_WIRE_HOST ";");
	(*env)->SetObjectField(env, obj_kxm, fid, obj_kxm_host);
	
	SAFE_DEL_LOCAL_REF(class_kxm_host);
	SAFE_DEL_LOCAL_REF(obj_kxm_host);
}

static void copy_udp_kxm_wire_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_wire = NULL;
	jobject obj_wire = NULL;
	jfieldID fid = NULL;
	cl_kxm_info_t *wire_info = (cl_kxm_info_t *)udp_info->device_info;

	if (wire_info == NULL) {
		return;
	}
	class_wire = (*env)->FindClass(env, CLASS_SDK_KXM_WIRE);
	obj_wire = (*env)->AllocObject(env, class_wire);
	jniCopyBooleanValue(env, class_wire, "is_data_valid", obj_wire, wire_info->is_data_valid);
	jniCopyBooleanValue(env, class_wire, "has_receive_data", obj_wire, wire_info->has_receive_data);

	copy_kxm_wire_stat(env, class_wire, obj_wire, wire_info);
	copy_kxm_wire_host(env, class_wire, obj_wire, wire_info);

	fid = (*env)->GetFieldID(env, class_dev, WRAP_QUOTE(wireInfo), "L"CLASS_SDK_KXM_WIRE";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_wire);
	SAFE_DEL_LOCAL_REF(class_wire);
	SAFE_DEL_LOCAL_REF(obj_wire);
}

static void copy_udp_kxm_therm_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_therm = NULL;
	jobject obj_therm = NULL;
	jfieldID fid = NULL;
	cl_kxm_thermost_info_t *therm_info = (cl_kxm_thermost_info_t *)udp_info->device_info;

	if (therm_info == NULL) {
		return;
	}
	class_therm = (*env)->FindClass(env, CLASS_SDK_KXM_THERM);
	obj_therm = (*env)->AllocObject(env, class_therm);

	jni_copy_simple_class(env, class_therm, obj_therm,
		                     TRIPLES(boolean, therm_info, onoff),
		                     TRIPLES(byte, therm_info, mode),
		                     TRIPLES(byte, therm_info, setting_temp),
		                     TRIPLES(byte, therm_info, room_temp),
		                     TRIPLES(byte, therm_info, fan_speed),
		                     TRIPLES(byte, therm_info, energy_cons),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_dev, WRAP_QUOTE(therInfo), "L"CLASS_SDK_KXM_THERM";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_therm);
	
	SAFE_DEL_LOCAL_REF(class_therm);
	SAFE_DEL_LOCAL_REF(obj_therm);
}


static void copy_udp_hy_thermostat_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_hy = NULL;
	jobject obj_hy = NULL;
	jfieldID fid = NULL;
	cl_hythermostat_info_t *hy_info = (cl_hythermostat_info_t *)udp_info->device_info;

	if (hy_info == NULL) {
		return;
	}
	class_hy = (*env)->FindClass(env, CLASS_SDK_HY_THERMOSTAT);
	obj_hy = (*env)->AllocObject(env, class_hy);		

	JNI_COPY_SIMPLE_CLASS(env, class_hy, obj_hy, CLASS_SDK_HY_THERMOSTAT_STAT, stat,
		                     TRIPLES(short, &hy_info->stat, mcuver),
		                     TRIPLES(short, &hy_info->stat, type),
		                     TRIPLES(short, &hy_info->stat, temp),
		                     TRIPLES(short, &hy_info->stat, valve),
		                     TRIPLES(short, &hy_info->stat, onoff),
		                     TRIPLES(short, &hy_info->stat, settemp),
		                     TRIPLES(short, &hy_info->stat, mode),
		                     TRIPLES(short, &hy_info->stat, wind),
		                     TRIPLES(short, &hy_info->stat, RHfun),
		                     TRIPLES(short, &hy_info->stat, RHval),
		                     TRIPLES(short, &hy_info->stat, RHstate),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_dev, WRAP_QUOTE(hythInfo), "L"CLASS_SDK_HY_THERMOSTAT";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_hy);
	SAFE_DEL_LOCAL_REF(class_hy);
	SAFE_DEL_LOCAL_REF(obj_hy);
}

static void copy_udp_xinyuan_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_xywkq = NULL;
	jobject object_xywkq = NULL;

	jclass class_xywkq_smartmode = NULL;
	jobject object_xywkq_smartmode = NULL;

	jclass class_xywkq_adjust = NULL;
	jobject object_xywkq_adjust = NULL;

	jclass class_xywkq_timepoint = NULL;
	jobject object_xywkq_timepoint = NULL;
	jobjectArray array_timepoint = NULL;

	jfieldID fid;
	
	cl_xy_smartmode_com_info_t *xywkq_smartmode;
	cl_xy_adjust_t *xywkq_adjust;
	cl_xy_st_mode_t *xywkq_st_mode;
	
	cl_xy_info_t *xywkq = (cl_xy_info_t*)udp_info->device_info;
	int i = 0;
	int timePointCount = XY_SMARTHOME_SUB_MODE_NUM * XY_SMART_WEEK_MAX * XY_SMART_DAY_TIMEPOINT_MAX;

	if(!xywkq){
		return;
	}else{
		xywkq_smartmode = &xywkq->smart_mode;
		xywkq_st_mode = &xywkq_smartmode->sub_mode;
		xywkq_adjust = &xywkq->adjust;
	}

	class_xywkq = (*env)->FindClass(env, CLASS_SDK_XINYUAN_INFO);
	if(!class_xywkq){
		goto quit;
	}

	object_xywkq = (*env)->AllocObject(env, class_xywkq);
	if(!object_xywkq){
		goto quit;
	}

	//copy cl_xy_info_t成员到object_xywkq
	jniCopyByteValue(env, class_xywkq, "onoff", object_xywkq, xywkq->onoff);
	jniCopyByteValue(env, class_xywkq, "mode", object_xywkq, xywkq->mode);
	jniCopyShortValue(env, class_xywkq, "root_temp", object_xywkq, xywkq->root_temp);
	jniCopyShortValue(env, class_xywkq, "di_temp", object_xywkq, xywkq->di_temp);
	jniCopyByteValue(env, class_xywkq, "cur_dst_temp", object_xywkq, xywkq->cur_dst_temp);
	jniCopyByteValue(env, class_xywkq, "heat", object_xywkq, xywkq->heat);
	jniCopyByteValue(env, class_xywkq, "lock", object_xywkq, xywkq->lock);
	jniCopyByteValue(env, class_xywkq, "err", object_xywkq, xywkq->err);
	jniCopyByteValue(env, class_xywkq, "cons_temp", object_xywkq, xywkq->cons_temp);
	jniCopyByteValue(env, class_xywkq, "holiday_temp", object_xywkq, xywkq->holiday_temp);
	jniCopyShortValue(env, class_xywkq, "remain_time", object_xywkq, xywkq->remain_time);
	jniCopyByteValue(env, class_xywkq, "last_cmd", object_xywkq, xywkq->last_cmd);
	jniCopyByteValue(env, class_xywkq, "extern_temp", object_xywkq, xywkq->extern_temp);
	jniCopyByteValue(env, class_xywkq, "probe_err", object_xywkq, xywkq->probe_err);
	jniCopyByteValue(env, class_xywkq, "smarthome_onoff", object_xywkq, xywkq->smarthome_onoff);

	//copy cl_xy_smartmode_info_t成员到object_xywkq_smartmode
	class_xywkq_smartmode = (*env)->FindClass(env, CLASS_SDK_XINYUAN_MODE);
	if(!class_xywkq_smartmode){
		goto quit;
	}
	object_xywkq_smartmode = (*env)->AllocObject(env, class_xywkq_smartmode);
	if(!object_xywkq_smartmode){
		goto quit;
	}

	class_xywkq_timepoint = (*env)->FindClass(env, CLASS_SDK_XINYUAN_TP);
	
	jniCopyByteValue(env, class_xywkq_smartmode, "mode", object_xywkq_smartmode, xywkq_st_mode->mode);

	array_timepoint = (*env)->NewObjectArray(env, timePointCount, class_xywkq_timepoint, NULL);
	for(i = 0; i< timePointCount; i++){
		object_xywkq_timepoint = (*env)->AllocObject(env, class_xywkq_timepoint);
		jni_copy_simple_class(env, class_xywkq_timepoint, object_xywkq_timepoint,
			                     TRIPLES(byte, &xywkq_smartmode->timepoint[i], valid),
			                     TRIPLES(byte, &xywkq_smartmode->timepoint[i], start_index),
			                     TRIPLES(byte, &xywkq_smartmode->timepoint[i], end_index),
			                     TRIPLES(byte, &xywkq_smartmode->timepoint[i], temp),
			                     JNI_VAR_ARG_END);
		(*env)->SetObjectArrayElement(env, array_timepoint, i, object_xywkq_timepoint);
		SAFE_DEL_LOCAL_REF(object_xywkq_timepoint);
	}
	fid = (*env)->GetFieldID(env, class_xywkq_smartmode, "timepoint", "[L" CLASS_SDK_XINYUAN_TP ";");
	(*env)->SetObjectField(env, object_xywkq_smartmode, fid, array_timepoint);
	SAFE_DEL_LOCAL_REF(array_timepoint);
	
	fid = (*env)->GetFieldID(env, class_xywkq, "smart_mode", "L" CLASS_SDK_XINYUAN_MODE ";");
	(*env)->SetObjectField(env, object_xywkq, fid, object_xywkq_smartmode);

	//copy cl_xy_adjust_t成员到object_xywkq_adjust
	class_xywkq_adjust = (*env)->FindClass(env, CLASS_SDK_XINYUAN_ADJUST);
	if(!class_xywkq_adjust){
		goto quit;
	}
	object_xywkq_adjust = (*env)->AllocObject(env, class_xywkq_adjust);
	if(!object_xywkq_adjust){
		goto quit;
	}
	
	jniCopyByteValue(env, class_xywkq_adjust, "temp_adj", object_xywkq_adjust, xywkq_adjust->temp_adj);
	jniCopyByteValue(env, class_xywkq_adjust, "temp_top", object_xywkq_adjust, xywkq_adjust->temp_top);
	jniCopyByteValue(env, class_xywkq_adjust, "temp_tolr", object_xywkq_adjust, xywkq_adjust->temp_tolr);
	jniCopyByteValue(env, class_xywkq_adjust, "temp_otemp", object_xywkq_adjust, xywkq_adjust->temp_otemp);
	jniCopyByteValue(env, class_xywkq_adjust, "temp_prottrig", object_xywkq_adjust, xywkq_adjust->temp_prottrig);
	jniCopyByteValue(env, class_xywkq_adjust, "temp_protlen", object_xywkq_adjust, xywkq_adjust->temp_protlen);
	
	fid = (*env)->GetFieldID(env, class_xywkq, "adjust", "L" CLASS_SDK_XINYUAN_ADJUST ";");
	(*env)->SetObjectField(env, object_xywkq, fid, object_xywkq_adjust);
	
	fid = (*env)->GetFieldID(env, class_dev, "device_info", "L" CLASS_SDK_XINYUAN_INFO ";");
	(*env)->SetObjectField(env, obj_dev, fid, object_xywkq);
	quit:
		SAFE_DEL_LOCAL_REF(class_xywkq);
		SAFE_DEL_LOCAL_REF(object_xywkq);
		SAFE_DEL_LOCAL_REF(class_xywkq_smartmode);
		SAFE_DEL_LOCAL_REF(object_xywkq_smartmode);
		SAFE_DEL_LOCAL_REF(class_xywkq_adjust);
		SAFE_DEL_LOCAL_REF(object_xywkq_adjust);
		SAFE_DEL_LOCAL_REF(class_xywkq_timepoint);
		SAFE_DEL_LOCAL_REF(object_xywkq_timepoint);
		SAFE_DEL_LOCAL_REF(array_timepoint);
}


static void copy_udp_rfgw_info(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_rfgw = NULL;
	jobject obj_rfgw = NULL;
	jfieldID fid = NULL;
	cl_gw_info_t *rfgw = (cl_gw_info_t*)udp_info->device_info;

	
	class_rfgw = (*env)->FindClass(env, CLASS_SDK_RFGW_INFO);
	obj_rfgw = (*env)->AllocObject(env, class_rfgw);

	jni_copy_simple_class(env, class_rfgw, obj_rfgw,
		                     TRIPLES(byte, rfgw, commpat),
		                     TRIPLES(byte, rfgw, channel),
		                     TRIPLES(boolean, rfgw, is_upgrade),
		                     QUADRUPLE(byte[], rfgw, upgrade_status, D_T_MAX),
		                     QUADRUPLE(String[], rfgw, upgrade_url, D_T_MAX),
		                     JNI_VAR_ARG_END);
	
/*
	JNI_COPY_ARRAY_CLASS(env, class_rfgw, obj_rfgw, CLASS_SDK_RFGW_GROUP, dev_group,
		                    rfgw->dev_group_cnt, sizeof(cl_dev_group_t),
		                    ARRAY_TRIPLES(byte, rfgw->dev_group, group_id),
		                    ARRAY_TRIPLES(byte, rfgw->dev_group, group_type),
		                    ARRAY_TRIPLES(short, rfgw->dev_group, dev_cnt),
		                    ARRAY_TRIPLES(boolean, rfgw->dev_group, query_dev),
		                    ARRAY_TRIPLES(String, rfgw->dev_group, name),
		                    QUADRUPLE(long[], rfgw->dev_group, dev_sn, 128),
		                    JNI_VAR_ARG_END);

	copy_udp_rfgw_lamp_remote(env, class_rfgw, obj_rfgw, rfgw);
*/
	fid = (*env)->GetFieldID(env, class_dev, WRAP_QUOTE(rfgwInfo), "L"CLASS_SDK_RFGW_INFO";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_rfgw);
	SAFE_DEL_LOCAL_REF(class_rfgw);
	SAFE_DEL_LOCAL_REF(obj_rfgw);
}


static void copy_udp_dev_lede(JNIEnv* env, jclass class_dev, jobject obj_dev, cl_com_udp_device_data *udp_info)
{
	jclass class_lede = NULL;
	jobject obj_lede = NULL;
	jfieldID fid;
	cl_lede_lamp_info *lede_info = (cl_lede_lamp_info*)udp_info->device_info;
	cl_lede_led_state_t *stat_info = &lede_info->cur_lamp_stat;

	class_lede = (*env)->FindClass(env, CLASS_SDK_LEDE);
	obj_lede = (*env)->AllocObject(env, class_lede);

	JNI_COPY_SIMPLE_CLASS(env, class_lede, obj_lede, CLASS_SDK_LEDE_STAT, stat, 
		                     WRAP_QUOTE(byte),GET_MEMBER(stat_info, R),WRAP_QUOTE(r),
		                     WRAP_QUOTE(byte),GET_MEMBER(stat_info, G),WRAP_QUOTE(g),
		                     WRAP_QUOTE(byte),GET_MEMBER(stat_info, B),WRAP_QUOTE(b),
		                     WRAP_QUOTE(byte),GET_MEMBER(stat_info, L),WRAP_QUOTE(l),
		                     TRIPLES(byte, stat_info, cold),
		                     TRIPLES(byte, stat_info, mod_id),
		                     TRIPLES(byte, stat_info, action),
		                     JNI_VAR_ARG_END);
	if (lede_info->timer_count <= 0) {
		goto done;
	}
	JNI_COPY_ARRAY_CLASS(env, class_lede, obj_lede, CLASS_SDK_LEDE_TIMER, timers,
		                   lede_info->timer_count, sizeof(cl_lede_led_timer_t),
		                   ARRAY_TRIPLES(byte, lede_info->timer_info, id),
		                   WRAP_QUOTE(boolean),GET_MEMBER_PTR(lede_info->timer_info, flags),WRAP_QUOTE(enable),
		                   WRAP_QUOTE(byte),GET_MEMBER_PTR(lede_info->timer_info, week_loop),WRAP_QUOTE(week),
		                   ARRAY_TRIPLES(byte, lede_info->timer_info, hour),
		                   WRAP_QUOTE(byte),GET_MEMBER_PTR(lede_info->timer_info, min),WRAP_QUOTE(minute),
		                   WRAP_QUOTE(boolean),GET_MEMBER_PTR(&lede_info->timer_info->config, power),WRAP_QUOTE(onoff),
		                   WRAP_QUOTE(byte),GET_MEMBER_PTR(&lede_info->timer_info->config, R),WRAP_QUOTE(r),
		                   WRAP_QUOTE(byte),GET_MEMBER_PTR(&lede_info->timer_info->config, G),WRAP_QUOTE(g),
		                   WRAP_QUOTE(byte),GET_MEMBER_PTR(&lede_info->timer_info->config, B),WRAP_QUOTE(g),
		                   WRAP_QUOTE(byte),GET_MEMBER_PTR(&lede_info->timer_info->config, L),WRAP_QUOTE(l),
		                   ARRAY_TRIPLES(byte, &lede_info->timer_info->config, cold),
		                   ARRAY_TRIPLES(byte, &lede_info->timer_info->config, mod_id),
		                   ARRAY_TRIPLES(byte, &lede_info->timer_info->config, action),
		                   JNI_VAR_ARG_END);
	
done:
	fid = (*env)->GetFieldID(env, class_dev, WRAP_QUOTE(ledeInfo), "L"CLASS_SDK_LEDE";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_lede);
	SAFE_DEL_LOCAL_REF(class_lede);
	SAFE_DEL_LOCAL_REF(obj_lede);
}

/*
 *拷贝cl_com_udp_device_data中的device_info成员
*/
static void copy_dev_udp_device_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	int udp_dev_type = 0;
	cl_com_udp_device_data *udp_info = info->com_udp_info;
	
	if (udp_info == NULL || udp_info->device_info == NULL) {
		return;
	}
	if (where != DEV_WHERE_COMMON_UDP) {
		return;
	}
	udp_dev_type = udpTypeMap(udp_info->sub_type, udp_info->ext_type);
	switch(udp_dev_type) {
	case UDP_DEV_HTC_HP:
		break;
	case UDP_DEV_TMC_JNB:
		break;
	case UDP_DEV_TMC_YL:
		break;
	case UDP_DEV_PDC_JCX:
		break;
	case UDP_DEV_LEDE_TGD:
		copy_udp_dev_lede(env, class_dev, obj_dev, udp_info);
		break;
	case UDP_DEV_AMT_FAN:
		break;
	case UDP_DEV_CHIFFO_FLOOR_HEATER:
		break;
	case UDP_DEV_TELIN_HEATING:
		break;
	case UDP_DEV_QP_CP:
		break;
	case UDP_DEV_POBIJI:
		break;
	case UDP_DEV_TB_HOUSE:
		copy_udp_tb_house_info(env, class_dev, obj_dev, udp_info);
		break;
	case UDP_DEV_TB_POOL:
	case UDP_DEV_TB_COMMERCIAL:
		break;
	case UDP_DEV_RF_GW:
		copy_udp_rfgw_info(env, class_dev, obj_dev, udp_info);
		break;
	case UDP_DEV_HY_THERMOSTAT:
		copy_udp_hy_thermostat_info(env, class_dev, obj_dev, udp_info);
		break;
	case UDP_DEV_KXM_WIRE:
		copy_udp_kxm_wire_info(env, class_dev, obj_dev, udp_info);
		break;
	case UDP_DEV_KXM_THER:
		copy_udp_kxm_therm_info(env, class_dev, obj_dev, udp_info);
		break;
	case UDP_DEV_XINYUAN:
		copy_udp_xinyuan_info(env, class_dev, obj_dev, udp_info);
		break;
	default:
		break;
	}
}

static void copy_rc_info(JNIEnv* env, jclass wrap_class, jclass wrap_obj, char *mem, cl_rc_info *rc_info)
{
	jclass class_rc = NULL;
	jobject obj_rc = NULL;
	jfieldID fid;

	if (rc_info == NULL) {
		return;
	}

	class_rc = (*env)->FindClass(env, CLASS_SDK_RC_INFO);
	obj_rc = (*env)->AllocObject(env, class_rc);
	
	jni_copy_simple_class(env, class_rc, obj_rc,
		                     TRIPLES(short, rc_info, d_id),
		                     TRIPLES(boolean, rc_info, is_matched),
		                     TRIPLES(byte, rc_info, fixed_key_num),
		                     TRIPLES(byte, rc_info, user_def_key_num),
		                     TRIPLES(short, rc_info, matched_ir_id),
		                     TRIPLES(String, rc_info, name),
		                     JNI_VAR_ARG_END);
	if (rc_info->fixed_key_num > 0 && rc_info->fk != NULL) {
		JNI_COPY_ARRAY_CLASS(env, class_rc, obj_rc, CLASS_SDK_RC_FIX_KEY, fk,
				                 rc_info->fixed_key_num, sizeof(cl_rc_fixed_key_info),
				                 ARRAY_TRIPLES(short_byte, rc_info->fk, key_id),
				                 ARRAY_TRIPLES(boolean, rc_info->fk, has_code),
				                 JNI_VAR_ARG_END);
	}
	if (rc_info->user_def_key_num > 0 && rc_info->uk != NULL) {
		JNI_COPY_ARRAY_CLASS(env, class_rc, obj_rc, CLASS_SDK_RC_CUS_KEY, uk,
			                    rc_info->user_def_key_num, sizeof(cl_rc_user_key_info),
			                    ARRAY_TRIPLES(short_byte, rc_info->uk, key_id),
			                    ARRAY_TRIPLES(boolean, rc_info->uk, has_code),
			                    ARRAY_TRIPLES(String, rc_info->uk, name),
			                    JNI_VAR_ARG_END);
	}

	fid = (*env)->GetFieldID(env, wrap_class, mem, "L"CLASS_SDK_RC_INFO";");
	(*env)->SetObjectField(env, wrap_obj, fid, obj_rc);
		
	SAFE_DEL_LOCAL_REF(class_rc);
	SAFE_DEL_LOCAL_REF(obj_rc);
}

static void copy_pair_rc_info(JNIEnv* env, jclass wrap_class, jclass wrap_obj, char *mem, cl_pair_rc_info *pair_info)
{
	jclass class_rc = NULL;
	jobject obj_rc = NULL;
	jfieldID fid;
	
	if (pair_info == NULL) {
		return;
	}
	class_rc = (*env)->FindClass(env, CLASS_SDK_RC_PAIR);
	obj_rc = (*env)->AllocObject(env, class_rc);
	
	jniCopyString(env, class_rc, "name", obj_rc, pair_info->name);
	copy_rc_info(env, class_rc, obj_rc, "tv_info", &pair_info->tv_info);
	copy_rc_info(env, class_rc, obj_rc, "stb_info", &pair_info->stb_info);

	fid = (*env)->GetFieldID(env, wrap_class, mem, "L"CLASS_SDK_RC_PAIR";");
	(*env)->SetObjectField(env, wrap_obj, fid, obj_rc);
		
	SAFE_DEL_LOCAL_REF(class_rc);
	SAFE_DEL_LOCAL_REF(obj_rc);
}

static int get_slave_num(cl_dev_info_t *info)
{
	int num = 0;
	int i = 0;
	
	if (info == NULL || info->num_objs == 0 || info->objs== NULL) {
		goto done;
	}
	for (i = 0; i < info->num_objs; ++i) {
		if (info->objs[i]->type == OT_SLAVE) {
			++num;
		}
	}
done:
	return num;
}

static void copy_dev_slave_types(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	int slave_num = get_slave_num(info);
	int i = 0, index = 0;
	cl_slave_t *slave = NULL;
	jclass class_type = NULL;
	jobject obj_type = NULL;
	jobjectArray typeArray = NULL;
	jfieldID fid;

	if (slave_num == 0) {
		return;
	}

	class_type = (*env)->FindClass(env, CLASS_SDK_DEV_TYPE);
	typeArray = (*env)->NewObjectArray(env, slave_num, class_type, NULL);
	for (i = 0; i < info->num_objs; ++i) {
		if (info->objs[i]->type == OT_SLAVE) {
			slave = (cl_slave_t *)info->objs[i];
			obj_type = (*env)->AllocObject(env, class_type);
			jni_copy_simple_class(env, class_type, obj_type,
				                     TRIPLES(int, &slave->obj, handle),
				                     TRIPLES(byte, slave, dev_type),
				                     TRIPLES(byte, slave, ext_type),
				                     JNI_VAR_ARG_END);
			(*env)->SetObjectArrayElement(env, typeArray, index, obj_type);
			++index;
			SAFE_DEL_LOCAL_REF(obj_type);
		}
	}

	fid = (*env)->GetFieldID(env, class_dev, mem, "[L"CLASS_SDK_DEV_TYPE";");
	(*env)->SetObjectField(env, obj_dev, fid, typeArray);
	SAFE_DEL_LOCAL_REF(typeArray);
	SAFE_DEL_LOCAL_REF(class_type);
}

static void copy_dev_eh_airplug_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	jclass class_ehwk = NULL;
	jobject obj_ehwk = NULL;
	jfieldID fid;
	cl_air_info_t *air_info = info->air_info;

	if (air_info == NULL) {
		return;
	}

	if (where != DEV_WHERE_AIRPLUG) {
		return;
	}

	class_ehwk = (*env)->FindClass(env, CLASS_SDK_EH_WK);
	obj_ehwk = (*env)->AllocObject(env, class_ehwk);

	jni_copy_simple_class(env,class_ehwk,obj_ehwk,
		                     TRIPLES(byte, air_info, temp_humidity),
		                     JNI_VAR_ARG_END);
	/*
	JNI_COPY_SIMPLE_CLASS(env, class_ehwk, obj_ehwk, CLASS_SDK_24_HOUR_DATA, room_temp_line,
		                     TRIPLES(boolean, &air_info->room_temp_line, is_valid),
		                     TRIPLES(byte, &air_info->room_temp_line, num),
		                     QUADRUPLE(byte[], &air_info->room_temp_line, data, 24 * 6),
		                     JNI_VAR_ARG_END);
	*/
	JNI_COPY_SIMPLE_CLASS(env, class_ehwk, obj_ehwk, CLASS_SDK_24_HOUR_DATA, humi_line,
		                     TRIPLES(boolean, &air_info->room_temp_line, is_valid),
		                     TRIPLES(byte, &air_info->room_temp_line, num),
		                     QUADRUPLE(byte[], &air_info->room_temp_line, data, 24 * 6),
		                     JNI_VAR_ARG_END);

	copy_pair_rc_info(env, class_ehwk, obj_ehwk, "priv_rc", &air_info->priv_rc);

	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_EH_WK";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_ehwk);
		
	SAFE_DEL_LOCAL_REF(class_ehwk);
	SAFE_DEL_LOCAL_REF(obj_ehwk);
}

static void copy_air_curve_ctrl(JNIEnv* env,jclass class_airplug, jobject obj_airplug, cl_air_info_t *air_info)
{
	jclass class_curve = NULL;
	jobject obj_curve = NULL;
	jfieldID fid = NULL;
	tmp_curve_t *pnodes = NULL;

	if (air_info == NULL || !air_info->is_support_temp_curve || air_info->temp_curve == NULL) {
		return;
	}

	if (air_info->temp_curve_len < sizeof(cl_temp_curve_t) + air_info->temp_curve->count * sizeof(tmp_curve_t)) {
		return;
	}
	class_curve = (*env)->FindClass(env, CLASS_SDK_AC_CURVE_CTRL);
	obj_curve = (*env)->AllocObject(env, class_curve);

	jni_copy_simple_class(env, class_curve, obj_curve,
		                     TRIPLES(byte, air_info->temp_curve, id),
		                     TRIPLES(boolean, air_info->temp_curve, enable),
		                     TRIPLES(byte, air_info->temp_curve, week),
		                     TRIPLES(byte, air_info->temp_curve, begin_hour),
		                     TRIPLES(byte, air_info->temp_curve, end_hour),
		                     TRIPLES(byte, air_info->temp_curve, time_period),
		                     JNI_VAR_ARG_END);
	pnodes = (tmp_curve_t *)(air_info->temp_curve + 1);
	JNI_COPY_ARRAY_CLASS(env, class_curve, obj_curve, CLASS_SDK_AC_CURVE_NODE, nodes,
		                   air_info->temp_curve->count, sizeof (tmp_curve_t),
		                   ARRAY_TRIPLES(byte, pnodes, flag),
		                   ARRAY_TRIPLES(byte, pnodes, tmp),
		                   ARRAY_TRIPLES(byte, pnodes, wind),
		                   ARRAY_TRIPLES(byte, pnodes, dir),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_airplug, "tempCurve", "L"CLASS_SDK_AC_CURVE_CTRL";");
	(*env)->SetObjectField(env, obj_airplug, fid, obj_curve);
		
	SAFE_DEL_LOCAL_REF(class_curve);
	SAFE_DEL_LOCAL_REF(obj_curve);
}


static void copy_dev_airplug_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	jclass class_airplug = NULL;
	jobject obj_airplug = NULL;
	jfieldID fid;
	cl_air_info_t *air_info = info->air_info;

	if (air_info == NULL) {
		return;
	}

	if (where != DEV_WHERE_AIRPLUG) {
		return;
	}
	
	class_airplug = (*env)->FindClass(env, CLASS_SDK_AIR_PLUG);
	obj_airplug = (*env)->AllocObject(env, class_airplug);

	jni_copy_simple_class(env,class_airplug,obj_airplug,
		                     TRIPLES(byte, air_info, room_temp),
		                     TRIPLES(boolean, air_info, is_match_code),
		                     TRIPLES(boolean, air_info, is_old_air),
		                     TRIPLES(byte, air_info, current_pan_type),
		                     TRIPLES(boolean, air_info, is_support_switch_pan),
		                     TRIPLES(boolean, air_info, is_support_temp_ac_ctrl),
		                     TRIPLES(boolean, air_info, is_support_temp_curve),
		                     TRIPLES(boolean, air_info, is_support_child_lock),
		                     TRIPLES(byte, air_info, child_lock_value),
		                     TRIPLES(boolean, air_info, is_support_param_ajust),
		                     TRIPLES(boolean, air_info, is_support_room_temp_ajust),
		                     JNI_VAR_ARG_END);
	
	JNI_COPY_SIMPLE_CLASS(env, class_airplug, obj_airplug, CLASS_SDK_AIR_PLUG_WORK, air_work_stat,
		                     TRIPLES(boolean, air_info, is_work_stat_data_valid),
		                     TRIPLES(boolean, &air_info->air_work_stat, onoff),
		                     TRIPLES(byte, &air_info->air_work_stat, mode),
		                     TRIPLES(byte, &air_info->air_work_stat, temp),
		                     TRIPLES(byte, &air_info->air_work_stat, wind),
		                     TRIPLES(byte, &air_info->air_work_stat, wind_direct),
		                     JNI_VAR_ARG_END);

	copy_air_match_info(env, class_airplug, obj_airplug, "last_match_info", &air_info->last_match_info);
	//copy_air_no_screen_info(env, class_airplug, obj_airplug, "key_info", &air_info->key_info);

	/*
	 * 2015年，新增的标准悟空数据，新增的包括智能恒温、24小时曲线控制，24小时温度历史数据，
	 * 童锁，参数校正，电量校正，室温校正
	 */
	 
	JNI_COPY_SIMPLE_CLASS(env, class_airplug, obj_airplug, CLASS_SDK_24_HOUR_DATA, room_temp_line,
		                     TRIPLES(boolean, &air_info->room_temp_line, is_valid),
		                     TRIPLES(byte, &air_info->room_temp_line, num),
		                     QUADRUPLE(byte[], &air_info->room_temp_line, data, 24 * 6),
		                     JNI_VAR_ARG_END);
	if (air_info->is_support_temp_ac_ctrl) {
		JNI_COPY_SIMPLE_CLASS(env, class_airplug, obj_airplug, CLASS_SDK_AC_TEMP_CTRL, tac,
			                     TRIPLES(boolean, &air_info->tac, enable),
			                     TRIPLES(byte, &air_info->tac, mode),
			                     TRIPLES(byte, &air_info->tac, temp_min),
			                     TRIPLES(byte, &air_info->tac, temp_max),
			                     TRIPLES(byte, &air_info->tac, week),
			                     TRIPLES(byte, &air_info->tac, begin_hour),
			                     TRIPLES(byte, &air_info->tac, end_hour),
			                     JNI_VAR_ARG_END);
	}
	copy_air_curve_ctrl(env, class_airplug, obj_airplug, air_info);
	if (air_info->is_support_param_ajust) {
		JNI_COPY_SIMPLE_CLASS(env, class_airplug, obj_airplug, CLASS_SDK_AC_PARAM_ADJUST, ajust_info,
			                     TRIPLES(boolean, &air_info->ajust_info, is_same_onoff_code),
			                     TRIPLES(boolean, &air_info->ajust_info, is_same_fan),
			                     TRIPLES(boolean, &air_info->ajust_info, is_fan_speed_opposite),
			                     JNI_VAR_ARG_END);
	}

	if (air_info->is_support_room_temp_ajust) {	
		JNI_COPY_SIMPLE_CLASS(env, class_airplug, obj_airplug, CLASS_SDK_AC_TEMP_ADJUST, room_temp_adjust,
			                     TRIPLES(short, air_info, env_room_temp_low),
			                     TRIPLES(short, air_info, env_room_temp_high),
			                     TRIPLES(short, air_info, env_temp_ajust_value),
			                     JNI_VAR_ARG_END);
	}
	
	fid = (*env)->GetFieldID(env, class_dev, mem, "L"CLASS_SDK_AIR_PLUG";");
	(*env)->SetObjectField(env, obj_dev, fid, obj_airplug);
		
	SAFE_DEL_LOCAL_REF(class_airplug);
	SAFE_DEL_LOCAL_REF(obj_airplug);
}

static void copy_dev_eplug_info(JNIEnv* env,jclass class_dev, jobject obj_dev, char *mem, int where, cl_dev_info_t *info)
{
	jclass class_eb = NULL;
	jobject obj_eb = NULL;
	jfieldID fid;
	cl_ia_eb_info_t *eb_info = info->eb_info;

	if (eb_info == NULL) {
		return;
	}

	JNI_COPY_SIMPLE_CLASS(env, class_dev, obj_dev, CLASS_SDK_EPLUG, eplugInfo,
		                     TRIPLES(boolean, eb_info, on_off),
		                     JNI_VAR_ARG_END);
}


JNIEXPORT jobject JNICALL
NAME(ClSdkGetUserInfo)(JNIEnv* env, jobject this, jint user_handle)
{
	cl_user_t *info;
	jclass class_user_info;
	jobject obj_user_info;

	info = clsdk_get_user_info(user_handle);
	if (info == NULL)
		return NULL;
	
	class_user_info = (*env)->FindClass(env, CLASS_SDK_USER);
	obj_user_info = (*env)->AllocObject(env, class_user_info);

	jni_assert(class_user_info != NULL);
	jni_assert(obj_user_info != NULL);
	jni_copy_simple_class(env, class_user_info, obj_user_info,
		                          TRIPLES(int, info, user_handle),
		                          TRIPLES(boolean, info, is_phone_user),
		                          TRIPLES(String, info, username),
		                          TRIPLES(String, info, passwd),
		                          TRIPLES(boolean, info, is_login),
		                          TRIPLES(int, info, last_err),
		                          TRIPLES(byte, info, sub_type),
		                          TRIPLES(byte, info, ext_type),
		                          TRIPLES(String, info, vendor_id),
		                          TRIPLES(String, info, vendor_url),
		                          JNI_VAR_ARG_END);
	cl_user_free_info(info);
	SAFE_DEL_LOCAL_REF(class_user_info);
	return obj_user_info;
}

int copy_info_items(JNIEnv* env, int handle, jclass class_dev, jobject obj_dev, jobject info_flag, int where)
{
	jclass class_bitset = NULL;
	jmethodID getId = NULL;
	cl_dev_info_t *dev_all_info = NULL;
	int i = 0;

	if ((dev_all_info = cl_user_get_dev_info(handle)) == NULL)
		return RS_NOT_FOUND;

	class_bitset = (*env)->FindClass(env, CLASS_BITSET);
	getId = (*env)->GetMethodID(env, class_bitset, "get", "(I)Z");

	for (i = 0; i < DEV_INFO_BIT_HIGHEST; ++i) {
		if ((*env)->CallBooleanMethod(env, info_flag, getId, i) == JNI_TRUE) {

			LOGD("xxxddd copy_info_items, i = %d\n", i);
			dev_info_copy_objs[i].copy_func(env, class_dev, obj_dev, 
				dev_info_copy_objs[i].mem_name, where, dev_all_info);
		}
	}
	cl_user_free_dev_info(dev_all_info);
	SAFE_DEL_LOCAL_REF(class_bitset);

	return RS_OK;
}

JNIEXPORT jobject JNICALL
NAME(ClSdkGetDevAirplug)(JNIEnv* env, jobject this, jint handle, jobject info_flag)
{
	jclass class_dev = NULL;
	jobject obj_dev = NULL;
	jclass class_bitset = NULL;

	class_dev = (*env)->FindClass(env, CLASS_SDK_DEV_AIRPLUG);
	obj_dev = (*env)->AllocObject(env, class_dev);

	if (copy_info_items(env, handle, class_dev, obj_dev, info_flag, DEV_WHERE_AIRPLUG) != RS_OK) {
		SAFE_DEL_LOCAL_REF(class_dev);
		SAFE_DEL_LOCAL_REF(obj_dev);
		return NULL;
	}
	
	SAFE_DEL_LOCAL_REF(class_dev);
	return obj_dev;
}

JNIEXPORT jobject JNICALL
NAME(ClSdkGetDevElecEplug)(JNIEnv* env, jobject this, jint handle, jobject info_flag)
{
	jclass class_dev = NULL;
	jobject obj_dev = NULL;
	jclass class_bitset = NULL;

	class_dev = (*env)->FindClass(env, CLASS_SDK_DEV_ElECEPLUG);
	obj_dev = (*env)->AllocObject(env, class_dev);

	if (copy_info_items(env, handle, class_dev, obj_dev, info_flag, DEV_WHERE_EPLUG) != RS_OK) {
		SAFE_DEL_LOCAL_REF(class_dev);
		SAFE_DEL_LOCAL_REF(obj_dev);
		return NULL;
	}
	
	SAFE_DEL_LOCAL_REF(class_dev);
	return obj_dev;
}


JNIEXPORT jobject JNICALL
NAME(ClSdkGetDevEhAirplug)(JNIEnv* env, jobject this, jint handle, jobject info_flag)
{
	jclass class_dev = NULL;
	jobject obj_dev = NULL;
	jclass class_bitset = NULL;

	class_dev = (*env)->FindClass(env, CLASS_SDK_DEV_EH_WK);
	obj_dev = (*env)->AllocObject(env, class_dev);

	if (copy_info_items(env, handle, class_dev, obj_dev, info_flag, DEV_WHERE_AIRPLUG) != RS_OK) {
		SAFE_DEL_LOCAL_REF(class_dev);
		SAFE_DEL_LOCAL_REF(obj_dev);
		return NULL;
	}
	
	SAFE_DEL_LOCAL_REF(class_dev);
	return obj_dev;
}


JNIEXPORT jobject JNICALL
NAME(ClSdkGetDevEplug)(JNIEnv* env, jobject this, jint handle, jobject info_flag)
{
	jclass class_dev = NULL;
	jobject obj_dev = NULL;
	jclass class_bitset = NULL;

	class_dev = (*env)->FindClass(env, CLASS_SDK_DEV_EPLUG);
	obj_dev = (*env)->AllocObject(env, class_dev);

	if (copy_info_items(env, handle, class_dev, obj_dev, info_flag, DEV_WHERE_EPLUG) != RS_OK) {
		SAFE_DEL_LOCAL_REF(class_dev);
		SAFE_DEL_LOCAL_REF(obj_dev);
		return NULL;
	}
	
	SAFE_DEL_LOCAL_REF(class_dev);
	return obj_dev;
}

JNIEXPORT jobject JNICALL
NAME(ClSdkGetUdpDev)(JNIEnv* env, jobject this, jint handle, jobject info_flag, jstring dev_class_name)
{
	jclass class_dev = NULL;
	jobject obj_dev = NULL;
	jclass class_bitset = NULL;
	const char *str_class_name = NULL;
	char str_class_path[256];

	str_class_name = (*env)->GetStringUTFChars(env, dev_class_name, NULL);
	sprintf(str_class_path, "%s/%s", SDK_DEV_PATH, str_class_name);
	(*env)->ReleaseStringUTFChars(env, dev_class_name, str_class_name);

	class_dev = (*env)->FindClass(env, str_class_path);
	if (class_dev == NULL) {
		return NULL;
	}
	obj_dev = (*env)->AllocObject(env, class_dev);

	if (copy_info_items(env, handle, class_dev, obj_dev, info_flag, DEV_WHERE_COMMON_UDP) != RS_OK) {
		SAFE_DEL_LOCAL_REF(class_dev);
		SAFE_DEL_LOCAL_REF(obj_dev);
		return NULL;
	}
	
	SAFE_DEL_LOCAL_REF(class_dev);
	return obj_dev;
}

static cl_slave_t *get_rf_slave(cl_dev_info_t *dev_all_info, int slave_handle)
{
	cl_slave_t *slave = NULL;
	int i = 0;
	
	if (dev_all_info->num_objs == 0 || dev_all_info->objs == NULL) {
		return NULL;
	}

	for (; i < dev_all_info->num_objs; ++i) {
		if (dev_all_info->objs[i]->type == OT_SLAVE) {
			slave = (cl_slave_t *)dev_all_info->objs[i];
			if (slave->obj.handle == slave_handle) {
				return slave;
			}
		}
	}
	return NULL;
}

static void getVersionString(cl_version_t *version, char *version_str) {
	sprintf(version_str, "%hhu.%hhu.%hhu", version->major, version->minor, version->revise);
}

static void copy_rf_slave_common_info(JNIEnv* env,jclass class_info, jobject obj_info, 
	                                                 char *mem, cl_slave_t *slave)
{
	jclass class_comm = NULL;
	jobject obj_comm = NULL;
	jfieldID fid = NULL;
	char version_str[32];

	class_comm = (*env)->FindClass(env, CLASS_SDK_RF_SLAVE_COMM);
	obj_comm = (*env)->AllocObject(env, class_comm);
	
	jni_copy_simple_class(env, class_comm, obj_comm,
		                     TRIPLES(int, &slave->obj, handle),
		                     TRIPLES(long, &slave->obj, sn),
		                     TRIPLES(String, &slave->obj, name),
		                     TRIPLES(byte, slave, ext_type),
		                     TRIPLES(boolean, slave, is_support_la),
		                     TRIPLES(int, slave, master_handle),
		                     TRIPLES(int, slave, bind_error),
		                     TRIPLES(long, slave, other_master_sn),
		                     TRIPLES(int, slave, uptime),
		                     TRIPLES(int, slave, online),
		                     TRIPLES(int, slave, conn_internet),
		                     JNI_VAR_ARG_END);
	getVersionString(&slave->soft_version, version_str);
	jniCopyString(env, class_comm, "soft_version", obj_comm, version_str);
	getVersionString(&slave->upgrade_version, version_str);
	jniCopyString(env, class_comm, "upgrade_version", obj_comm, version_str);
	jniCopyIntValue(env, class_comm, "rfType", obj_comm, slave->rfdev.d_type);
	jniCopyByteValue(env, class_comm, "sub_type", obj_comm, (u_int8_t)slave->dev_type);
	jniCopyIntValue(env, class_comm, "bindStat", obj_comm, slave->obj.status);
	jniCopyString(env, class_comm, "developer_id", obj_comm, slave->developer_id);
	fid = (*env)->GetFieldID(env, class_info, mem, "L"CLASS_SDK_RF_SLAVE_COMM";");
	(*env)->SetObjectField(env, obj_info, fid, obj_comm);
		
	SAFE_DEL_LOCAL_REF(class_comm);
	SAFE_DEL_LOCAL_REF(obj_comm);
}

static void copy_rf_slave_history(JNIEnv* env,jclass class_info, jobject obj_info, char *mem, cl_slave_t *slave_info)
{
	jclass class_his = NULL;
	jobject obj_his = NULL;
	jfieldID fid = NULL;
	cl_rf_dev_comm_history_info_t *his = NULL;

	if (slave_info == NULL) {
		return;
	}
	his = &slave_info->rfdev.chi;
	class_his = (*env)->FindClass(env, CLASS_SDK_RF_HISTORY);
	obj_his = (*env)->AllocObject(env, class_his);

	jni_copy_simple_class(env, class_his, obj_his,
		                     TRIPLES(int, his, index_current),
		                     TRIPLES(int, his, max_count),
		                     TRIPLES(short, his, index),
		                     JNI_VAR_ARG_END);
	if (his->n > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_his, obj_his, CLASS_SDK_RF_HISTORY_ITEM, items,
			                    his->n, sizeof(cl_rf_dev_comm_history_item_t),
			                    ARRAY_TRIPLES(byte, his->items, type),
			                    ARRAY_TRIPLES(byte, his->items, value),
			                    ARRAY_TRIPLES(byte, his->items, ex_type),
			                    ARRAY_TRIPLES(byte, his->items, ex_value),
			                    ARRAY_TRIPLES(int, his->items, timestamp),
			                    ARRAY_TRIPLES(boolean, his->items, valid),
			                    JNI_VAR_ARG_END);
	}
	

	fid = (*env)->GetFieldID(env, class_info, mem, "L"CLASS_SDK_RF_HISTORY";");
	(*env)->SetObjectField(env, obj_info, fid, obj_his);
		
	SAFE_DEL_LOCAL_REF(class_his);
	SAFE_DEL_LOCAL_REF(obj_his);
}

static void copy_rf_slave_alarm(JNIEnv* env,jclass class_info, jobject obj_info, char *mem, cl_slave_t *slave_info)
{
	jclass class_alarm = NULL;
	jobject obj_alarm = NULL;
	jfieldID fid = NULL;
	cl_rf_dev_comm_alarm_info_t *alarm = NULL;

	if (slave_info == NULL) {
		return;
	}
	alarm = &slave_info->rfdev.cai;
	class_alarm = (*env)->FindClass(env, CLASS_SDK_RF_ALARM);
	obj_alarm = (*env)->AllocObject(env, class_alarm);

	jni_copy_simple_class(env, class_alarm, obj_alarm,
		                     TRIPLES(int, alarm, record_time),
		                     TRIPLES(short, alarm, type),
		                     TRIPLES(int, alarm, value),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_info, mem, "L"CLASS_SDK_RF_ALARM";");
	(*env)->SetObjectField(env, obj_info, fid, obj_alarm);
		
	SAFE_DEL_LOCAL_REF(class_alarm);
	SAFE_DEL_LOCAL_REF(obj_alarm);
}


cl_slave_data_type rf_type_map_2_dt_type(u_int8_t sub_type, u_int8_t ext_type)
{
	cl_slave_data_type map_type = D_T_MAX;
	if (sub_type != IJ_RFGW) {
		return map_type;
	}
	switch(ext_type) {
	case RF_EXT_TYPE_DHX:
	case RF_EXT_TYPE_DHXCP:
		map_type = D_T_DHX_SWITCH;
		break;
	case RF_EXT_TYPE_HTLLOCK:
		map_type = D_T_HTLLOCK;
		break;
	default:
		break;
	}
	return map_type;
}

static void copy_rf_slave_slf(JNIEnv* env,jclass class_rf_dev, jobject obj_rf_dev, char *mem, cl_slave_t *slave_info)
{
	jclass class_slf = NULL;
	jobject obj_slf = NULL;
	jfieldID fid = NULL;

	class_slf = (*env)->FindClass(env, CLASS_SDK_RF_SLAVE_SLF);
	obj_slf = (*env)->AllocObject(env, class_slf);

	JNI_COPY_SIMPLE_CLASS(env, class_slf, obj_slf, CLASS_SDK_RF_SLAVE_SLF_STAT, stat,
		                    TRIPLES(byte, &slave_info->rfdev.dev_priv_data.dhx_info.stat, group_num),
		                    TRIPLES(int, &slave_info->rfdev.dev_priv_data.dhx_info.stat, on_off_stat),
		                    JNI_VAR_ARG_END);

    LOGD("rfdev.dev_priv_data.dhx_info.stat number%d:",&slave_info->rfdev.dev_priv_data.dhx_info.stat.group_num);
	fid = (*env)->GetFieldID(env, class_rf_dev, mem, "L"CLASS_SDK_RF_SLAVE_SLF";");
	(*env)->SetObjectField(env, obj_rf_dev, fid, obj_slf);
	
	SAFE_DEL_LOCAL_REF(class_slf);
	SAFE_DEL_LOCAL_REF(obj_slf);
}

static void copy_rf_slave_htl(JNIEnv* env,jclass class_rf_dev, jobject obj_rf_dev, char *mem, cl_slave_t *slave_info)
{
	jclass class_htl = NULL;
	jobject obj_htl = NULL;
	jfieldID fid = NULL;
	cl_htllock_info_t *lock_info = NULL;
	int i = 0;

	if (slave_info == NULL) {
		return;
	}
	lock_info = &slave_info->rfdev.dev_priv_data.hl_info;
	class_htl = (*env)->FindClass(env, CLASS_SDK_HTL_INFO);
	obj_htl = (*env)->AllocObject(env, class_htl);

	JNI_COPY_SIMPLE_CLASS(env, class_htl, obj_htl, CLASS_SDK_HTL_STAT, lock_stat,
		                    TRIPLES(boolean, &lock_info->lock_stat, onoff),
		                    TRIPLES(byte, &lock_info->lock_stat, vol),
		                    TRIPLES(byte, &lock_info->lock_stat, lang),
		                    TRIPLES(byte, &lock_info->lock_stat, battery),
		                    JNI_VAR_ARG_END);
	//处理有效时间小于1分钟的情况，将次数大于0，时间等于0的值为密码有效且有效时间为1分钟内。
	if(lock_info->last_pin.cnt > 0 && lock_info->last_pin.time == 0){
		lock_info->last_pin.time = 1;
		LOGI("zzzz hutlon last_pin valid, cnt = %d,time=%d,pwd = %s \n",lock_info->last_pin.cnt,lock_info->last_pin.time,lock_info->last_pin.pwd);
	}
	//设置最后的临时密码
	JNI_COPY_SIMPLE_CLASS(env, class_htl, obj_htl, CLASS_SDK_HTL_PIN, last_pin,
		                    TRIPLES(short, &lock_info->last_pin, time),
		                    TRIPLES(byte, &lock_info->last_pin, cnt),
		                    TRIPLES(String, &lock_info->last_pin, pwd),
		                    JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_htl, obj_htl, CLASS_SDK_HTL_USER, user_manage,
		                    HTLLOCK_MAX_USER_NUM, sizeof(htllock_user_manage_stat_t),
		                    ARRAY_TRIPLES(boolean, lock_info->user_manage, is_close_stat_reminder),
		                    ARRAY_TRIPLES(short, lock_info->user_manage, index),
		                    ARRAY_TRIPLES(short, lock_info->user_manage, pindex),
		                    ARRAY_TRIPLES(short, lock_info->user_manage, create_id),
		                    ARRAY_TRIPLES(byte, lock_info->user_manage, pic_id),
		                    ARRAY_TRIPLES(String, lock_info->user_manage, name),
		                    JNI_VAR_ARG_END);


	JNI_COPY_ARRAY_CLASS(env, class_htl, obj_htl, CLASS_SDK_HTL_NOTICE, info_notice,
		                    HTLLOCK_MAX_NOTICE_TYPE_NUM, sizeof(htllock_info_notice_stat_t),
		                    ARRAY_TRIPLES(byte, lock_info->info_notice, type),
		                    ARRAY_TRIPLES(boolean, lock_info->info_notice, support_remind),
		                    ARRAY_TRIPLES(boolean, lock_info->info_notice, support_trouble_free),
		                    ARRAY_TRIPLES(boolean, lock_info->info_notice, support_msg_remind),
		                    ARRAY_TRIPLES(boolean, lock_info->info_notice, support_tel_remind),
		                    JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_rf_dev, mem, "L"CLASS_SDK_HTL_INFO";");
	(*env)->SetObjectField(env, obj_rf_dev, fid, obj_htl);
	
	SAFE_DEL_LOCAL_REF(class_htl);
	SAFE_DEL_LOCAL_REF(obj_htl);
}




static void copy_rf_slave_dev_info(JNIEnv* env,jclass class_info, jobject obj_info, 
	                                             char *mem, cl_slave_t *slave_info)
{
	cl_slave_data_type dt_type = D_T_MAX;

	dt_type = rf_type_map_2_dt_type((u_int8_t)slave_info->dev_type, slave_info->ext_type);
	switch(dt_type) {
	case D_T_DHX_SWITCH:
		copy_rf_slave_slf(env, class_info, obj_info, mem, slave_info);
		break;
	case D_T_HTLLOCK:
		copy_rf_slave_htl(env, class_info, obj_info, mem, slave_info);
		break;
	}
}

static void copy_rf_slave_info_items(JNIEnv* env, jclass class_dev, jobject obj_dev, 
	                                                jobject info_flag, cl_slave_t *slave)
{
	jclass class_bitset = NULL;
	jmethodID getId = NULL;
	int i = 0;

	class_bitset = (*env)->FindClass(env, CLASS_BITSET);
	getId = (*env)->GetMethodID(env, class_bitset, "get", "(I)Z");

	for (i = 0; i < SLAVE_INFO_BIT_HIGHEST; ++i) {
		if ((*env)->CallBooleanMethod(env, info_flag, getId, i) == JNI_TRUE) {
			LOGD("xxxddd copy_rf_slave_info_items, i = %d\n", i);
			if (slave_info_copy_objs[i].copy_func != NULL) {
				slave_info_copy_objs[i].copy_func(env, class_dev, obj_dev, 
					slave_info_copy_objs[i].mem_name, slave);
			}
			
		}
	}
	SAFE_DEL_LOCAL_REF(class_bitset);
}


JNIEXPORT jobject JNICALL
NAME(ClSdkGetRfSlaveInfo)(JNIEnv* env, jobject this, jint masterHandle, jint slaveHandle, 
                       jstring slave_class_name, jobject info_flag)
{
	jclass class_slave = NULL;
	jobject obj_slave = NULL;
	const char *c_class_name = NULL;
	const char *c_dev_class_name = NULL;
	cl_dev_info_t *dev_info = NULL;
	cl_slave_t *slave = NULL;


	dev_info = cl_user_get_dev_info(masterHandle);
	if (dev_info == NULL) {
		return NULL;
	}
	slave = get_rf_slave(dev_info, slaveHandle);
	if (slave == NULL) {
		return NULL;
	}

	slave->master_handle = masterHandle;
	c_class_name = (*env)->GetStringUTFChars(env, slave_class_name, NULL);
	class_slave = (*env)->FindClass(env, c_class_name);
	if (class_slave == NULL) {
		return NULL;
	}
	obj_slave = (*env)->AllocObject(env, class_slave);
	
	copy_rf_slave_info_items(env, class_slave, obj_slave, info_flag, slave);

	(*env)->ReleaseStringUTFChars(env, slave_class_name, c_class_name);

	SAFE_DEL_LOCAL_REF(class_slave);

	cl_user_free_dev_info(dev_info);

	return obj_slave;
}


static void assign_period_timer(JNIEnv* env, jobject periodTimer, cl_period_timer_t *c_period_timer)
{
	jclass class_period_timer = NULL;
	jclass class_ext = NULL;

	memset(c_period_timer, 0, sizeof(cl_period_timer_t));
	class_period_timer = (*env)->GetObjectClass(env, periodTimer);
	jni_assign_simple_struct(env, periodTimer, class_period_timer,
		                        ASSIGN_TRIPLES(byte, c_period_timer, id),
		                        ASSIGN_TRIPLES(boolean, c_period_timer, enable),
		                        ASSIGN_TRIPLES(byte, c_period_timer, week),
		                        ASSIGN_TRIPLES(byte, c_period_timer, hour),
		                        ASSIGN_TRIPLES(byte, c_period_timer, minute),
		                        ASSIGN_TRIPLES(boolean, c_period_timer, onoff),
		                        ASSIGN_TRIPLES(short, c_period_timer, duration),
		                        ASSIGN_TRIPLES(short, c_period_timer, ext_data_type),
		                        JNI_VAR_ARG_END);
	switch(c_period_timer->ext_data_type) {
	case PT_EXT_DT_808:
		jni_assign_simple_struct(env, periodTimer, class_period_timer,
			                        ASSIGN_TRIPLES(boolean, &c_period_timer->pt_ext_data_u.air_timer_info, onOff),
			                        ASSIGN_TRIPLES(byte, &c_period_timer->pt_ext_data_u.air_timer_info, mode),
			                        ASSIGN_TRIPLES(byte, &c_period_timer->pt_ext_data_u.air_timer_info, temp),
			                        ASSIGN_TRIPLES(byte, &c_period_timer->pt_ext_data_u.air_timer_info, fan_speed),
			                        ASSIGN_TRIPLES(byte, &c_period_timer->pt_ext_data_u.air_timer_info, fan_dir),
			                        ASSIGN_TRIPLES(byte, &c_period_timer->pt_ext_data_u.air_timer_info, key_id),
			                        JNI_VAR_ARG_END);
		break;
	default:
		break;
	}
	//SAFE_DEL_LOCAL_REF(class_period_timer);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpPeriodTimerSetV2)(JNIEnv* env, jobject this, jint handle, jobject periodTimer)
{
	cl_period_timer_t c_period_timer;
	assign_period_timer(env, periodTimer, &c_period_timer);
	return cl_com_udp_period_timer_set(handle, &c_period_timer);
}

JNIEXPORT jint JNICALL
NAME(ClEbPeriodTimerSetV2)(JNIEnv* env, jobject this, jint handle, jobject periodTimer)
{
	cl_period_timer_t c_period_timer;
	assign_period_timer(env, periodTimer, &c_period_timer);
	return cl_eb_period_timer_set(handle, &c_period_timer);
}

JNIEXPORT jint JNICALL
NAME(ClEbTimerSetV2)(JNIEnv* env, jobject this, jint handle, jobject timer)
{
	cl_air_timer_t c_timer;
	jclass class_timer = NULL;
	int ret = RS_OK;

	class_timer = (*env)->GetObjectClass(env, timer);	
	jni_assign_simple_struct(env, timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &c_timer, id),
		                        ASSIGN_TRIPLES(boolean, &c_timer, enable),
		                        ASSIGN_TRIPLES(byte, &c_timer, week),
		                        ASSIGN_TRIPLES(byte, &c_timer, hour),
		                        ASSIGN_TRIPLES(byte, &c_timer, minute),
		                        ASSIGN_TRIPLES(boolean, &c_timer, onoff),
		                        JNI_VAR_ARG_END);
	ret = cl_eb_timer_set(handle, &c_timer);
	//SAFE_DEL_LOCAL_REF(class_timer);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpExtPeriodTimerSetV2)(JNIEnv* env, jobject this, jint handle, jobject periodTimer)
{
	cl_period_timer_t c_period_timer;
	void *param = NULL;
	u_int16_t param_len = 0;
	
	assign_period_timer(env, periodTimer, &c_period_timer);
	switch(c_period_timer.ext_data_type) {
	case PT_EXT_DT_808:
		param = &c_period_timer.pt_ext_data_u.air_timer_info;
		param_len = sizeof(cl_808_timer_ext_t);
		break;
	default:
		break;
	}
	return cl_com_udp_ext_period_timer_set(handle, &c_period_timer, param, param_len);
}

JNIEXPORT jobject JNICALL
NAME(ClRcGetMatchStatV2) (JNIEnv *env, jobject this, jint dev_handle)
{
	jclass clazz = NULL;
	jobject obj = NULL;
	cl_rc_match_stat_t rc_match_stat_t_p = {0};
	int ret;
	
	ret = cl_rc_get_match_stat(dev_handle, &rc_match_stat_t_p);
	if (ret == RS_OK) {
		clazz = (*env)->FindClass(env, CLASS_SDK_RC_PAIR_STAT);
		obj = (*env)->AllocObject(env, clazz);

		jni_copy_simple_class(env, clazz, obj,
									TRIPLES(byte, &rc_match_stat_t_p, type),
									TRIPLES(byte, &rc_match_stat_t_p, rc_id),
									TRIPLES(byte, &rc_match_stat_t_p, action),
									TRIPLES(boolean, &rc_match_stat_t_p, is_cloud_matching),
									TRIPLES(byte, &rc_match_stat_t_p, cur_step),
									TRIPLES(byte, &rc_match_stat_t_p, max_step),
									TRIPLES(byte, &rc_match_stat_t_p, error),
									TRIPLES(byte, &rc_match_stat_t_p, flag),
									TRIPLES(byte, &rc_match_stat_t_p, recommon_key_id),
									JNI_VAR_ARG_END
									);

		SAFE_DEL_LOCAL_REF(clazz);
		//SAFE_DEL_LOCAL_REF(obj);
	}
	
	return obj;
}

JNIEXPORT jobject JNICALL
NAME(ClRcGetLearnStatV2) (JNIEnv *env, jobject this, jint dev_handle)
{
	jclass clazz = NULL;
	jobject obj = NULL;
	cl_rc_key_learn_stat_t rc_key_learn_stat_t_p = {0};	
	int ret;
	
	ret = cl_rc_get_learn_stat(dev_handle, &rc_key_learn_stat_t_p);
	if (ret == RS_OK) {
		clazz = (*env)->FindClass(env, CLASS_SDK_RC_KEY_LEARN_STAT);
		obj = (*env)->AllocObject(env, clazz);

		jni_copy_simple_class(env, clazz, obj,
									TRIPLES(boolean, &rc_key_learn_stat_t_p, isLearn),
									TRIPLES(byte, &rc_key_learn_stat_t_p, rc_id),
									TRIPLES(byte, &rc_key_learn_stat_t_p, type),
									TRIPLES(byte, &rc_key_learn_stat_t_p, key_id),
									TRIPLES(byte, &rc_key_learn_stat_t_p, code_len),
									QUADRUPLE(byte[], &rc_key_learn_stat_t_p, code, MAX_CODE_LEN),
									JNI_VAR_ARG_END
									);
		SAFE_DEL_LOCAL_REF(clazz);
		//SAFE_DEL_LOCAL_REF(obj);
	}
	
	return obj;
}

/*智能模式参数配置*/
JNIEXPORT jint JNICALL
NAME(ClXyConfigSmartModeV2)(JNIEnv* env, jobject this, jint dev_handle, jobject pst_info){
	jint ret = 0;
	cl_xy_smartmode_com_info_t xy_pst_info;
	jclass class_xy_tp = NULL;
	jclass class_xywkq_xystmode = NULL;
	jobjectArray array_tp = NULL;
	jobject object_tp = NULL;
	jfieldID fid;
	int i;
	int tpCount = XY_SMARTHOME_SUB_MODE_NUM * XY_SMART_WEEK_MAX * XY_SMART_DAY_TIMEPOINT_MAX;
	
	memset(&xy_pst_info, 0, sizeof(cl_xy_smartmode_com_info_t));
	
	class_xywkq_xystmode = (*env)->FindClass(env, CLASS_SDK_XINYUAN_MODE);
	class_xy_tp = (*env)->FindClass(env, CLASS_SDK_XINYUAN_TP);
	
	fid = (*env)->GetFieldID(env, class_xywkq_xystmode, "mode", "B");
	xy_pst_info.sub_mode.mode = (*env)->GetByteField(env, pst_info, fid);

	fid = (*env)->GetFieldID(env, class_xywkq_xystmode, "timepoint", "[L" CLASS_SDK_XINYUAN_TP ";");
	array_tp = (jobjectArray)(*env)->GetObjectField(env, pst_info, fid );
	
	for(i = 0; i< tpCount; i++){
		object_tp = (*env)->GetObjectArrayElement(env, array_tp, i);
		fid = (*env)->GetFieldID(env, class_xy_tp, "valid", "B");
		xy_pst_info.timepoint[i].valid = (*env)->GetByteField(env, object_tp, fid);
		
		fid = (*env)->GetFieldID(env, class_xy_tp, "start_index", "B");
		xy_pst_info.timepoint[i].start_index = (*env)->GetByteField(env, object_tp, fid);
		
		fid = (*env)->GetFieldID(env, class_xy_tp, "end_index", "B");
		xy_pst_info.timepoint[i].end_index = (*env)->GetByteField(env, object_tp, fid);
		
		fid = (*env)->GetFieldID(env, class_xy_tp, "temp", "B");
		xy_pst_info.timepoint[i].temp = (*env)->GetByteField(env, object_tp, fid);
		SAFE_DEL_LOCAL_REF(object_tp);
	}
	SAFE_DEL_LOCAL_REF(class_xywkq_xystmode);
	SAFE_DEL_LOCAL_REF(class_xy_tp);
	SAFE_DEL_LOCAL_REF(array_tp);

	return cl_xy_config_smart_mode(dev_handle, &xy_pst_info);
}


/*后台校正参数*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlAdjustV2)(JNIEnv* env, jobject this, jint dev_handle, jobject padjust){
	jint ret = 0;
	cl_xy_adjust_t xy_adjust;
	jclass clazz;
	jfieldID fid;
	
	memset(&xy_adjust, 0, sizeof(cl_xy_adjust_t));
	
	clazz = (*env)->FindClass(env, CLASS_SDK_XINYUAN_ADJUST);
	
	fid = (*env)->GetFieldID(env, clazz, "temp_adj", "B");
	xy_adjust.temp_adj = (*env)->GetByteField(env, padjust, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp_top", "B");
	xy_adjust.temp_top = (*env)->GetByteField(env, padjust, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp_tolr", "B");
	xy_adjust.temp_tolr = (*env)->GetByteField(env, padjust, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp_otemp", "B");
	xy_adjust.temp_otemp = (*env)->GetByteField(env, padjust, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp_prottrig", "B");
	xy_adjust.temp_prottrig = (*env)->GetByteField(env, padjust, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp_protlen", "B");
	xy_adjust.temp_protlen = (*env)->GetByteField(env, padjust, fid);

	ret = cl_xy_ctrl_adjust(dev_handle, &xy_adjust);
	
	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}



