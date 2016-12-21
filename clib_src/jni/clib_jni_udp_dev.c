#include "clib_jni.h"
#include "cl_tb_heater_pump.h"
#include "client_lib.h"
#include "cl_yl_thermostat.h"
#include "cl_jnb_device.h"
#include "cl_jcx_power_box.h"
#include "cl_amt_device.h"
#include "cl_chiffo.h"
#include "cl_qpcp.h"
#include "clib_jni_sdk.h"
#include "cl_car.h"
#include "cl_eplug_oem.h"
#include "cl_bimar_dev.h"
#include "cl_xy.h"
#include "cl_bimar_dev.h"
#include "cl_tbb.h"
#include "cl_yt.h"
#include "cl_ads.h"
#include "cl_js_wave.h"
#include "cl_zssx.h"
#include "cl_yj_heater.h"
#include "cl_yuyuan.h"
#include "cl_zkrsq.h"
#include "cl_indiacar.h"
#include "cl_zkcleanner.h"
#include "cl_hythermostat.h"
#include "cl_bpuair.h"
#include "cl_jrxheater.h"
#include "cl_linkon.h"
#include "cl_cjthermostat.h"
#include "cl_drkzq.h"
#include "cl_zhcl.h"
#include "cl_leis.h"
#include "cl_yinsu.h"
#include "cl_zhdhx.h"

#include <time.h>

static cl_rf_lamp_stat_t* get_rf_light_state(JNIEnv* env, jobject ledeState) {
	jfieldID fid;
	jclass obj_class ;

	cl_rf_lamp_stat_t * lamp_info = (cl_rf_lamp_stat_t *)malloc(sizeof(cl_rf_lamp_stat_t));

	if(ledeState == NULL)
		return NULL;

	obj_class = (*env)->FindClass(env, CLASS_RF_LIGHT_STATU);
	
	fid = (*env)->GetFieldID(env, obj_class, "r", "I");
	lamp_info->R= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "g", "I");
	lamp_info->G= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "b", "I");
	lamp_info->B= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "l", "I");
	lamp_info->L= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "cold", "I");
	lamp_info->cold= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "power", "Z");
	lamp_info->power= (*env)->GetBooleanField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "modeId", "I");
	lamp_info->mod_id= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "action", "I");
	lamp_info->action= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "ctrl_mode", "I");
	lamp_info->ctrl_mode= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "value", "I");
	lamp_info->value= (*env)->GetIntField(env, ledeState, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "flag", "B");
	lamp_info->flag= (*env)->GetByteField(env, ledeState, fid);

	SAFE_DEL_LOCAL_REF(obj_class);
	
	return lamp_info;
}

static cl_rfdev_onekey_ctrl_t* get_rfgw_one_key_ctrl(JNIEnv* env, jobject onekey_ctrl_info){
	jfieldID fid;
	jclass info_class ;
	jclass ctrl_class;
	jobject ctrl_obj;
	cl_rfdev_onekey_ctrl_t *one_key_ctrl  = (cl_rfdev_onekey_ctrl_t *)malloc(sizeof(cl_rfdev_onekey_ctrl_t));
	if(onekey_ctrl_info == NULL){
		return NULL;
	}
	info_class = (*env)->FindClass(env,CLASS_RF_GW_ONEKEY_CTRL_INFO);
	fid = (*env)->GetFieldID(env,info_class,"type","B");

	//read the config type.
	one_key_ctrl->type = (*env)->GetByteField(env,onekey_ctrl_info,fid);
	switch(one_key_ctrl->type){
		case OKT_SMART_ALARM:
			one_key_ctrl->len = sizeof(cl_rfdev_onekey_smart_alarm_t);
			//read the smart alarm info object.
			ctrl_class = (*env)->FindClass(env,CLASS_RF_GW_ONEKEY_SMART_ALARM);
			fid = (*env)->GetFieldID(env,info_class,"smart_alarm","L"CLASS_RF_GW_ONEKEY_SMART_ALARM";");
			ctrl_obj = (*env)->GetObjectField(env,onekey_ctrl_info,fid);
			
			//set enable to struct
			fid = (*env)->GetFieldID(env,ctrl_class,"enable","Z");
			one_key_ctrl->ctrl.smart_alarm.enable = (*env)->GetBooleanField(env,ctrl_obj,fid);
			//set gw_sn to struct
			fid = (*env)->GetFieldID(env,ctrl_class,"gw_sn","J");
			one_key_ctrl->ctrl.smart_alarm.gw_sn = (*env)->GetLongField(env,ctrl_obj,fid);
			
			break;
		case OKT_SET_DEFENSE:
			one_key_ctrl->len = sizeof(cl_rfdev_onekey_set_defense_t);
			//read the smart alarm info object.
			ctrl_class = (*env)->FindClass(env,CLASS_RF_GW_ONEKEY_SET_DEFENCE);
			fid = (*env)->GetFieldID(env,info_class,"set_defense","L"CLASS_RF_GW_ONEKEY_SET_DEFENCE";");
			ctrl_obj = (*env)->GetObjectField(env,onekey_ctrl_info,fid);
			
			fid = (*env)->GetFieldID(env,ctrl_class,"defense","Z");
			one_key_ctrl->ctrl.set_defense.defense = (*env)->GetBooleanField(env,ctrl_obj,fid);
			break;
		case OKT_ALARM_MODE:
			one_key_ctrl->len = sizeof(cl_rfdev_onekey_set_alarm_mode_t);
			//read the smart alarm info object.
			ctrl_class = (*env)->FindClass(env,CLASS_RF_GW_ONEKEY_SET_ALARM_MODE);
			fid = (*env)->GetFieldID(env,info_class,"alarm_mode","L"CLASS_RF_GW_ONEKEY_SET_ALARM_MODE";");
			ctrl_obj = (*env)->GetObjectField(env,onekey_ctrl_info,fid);

			//init params
			fid = (*env)->GetFieldID(env,ctrl_class,"enable","B");
			one_key_ctrl->ctrl.alarm_mode.enable = (*env)->GetByteField(env,ctrl_obj,fid);
			fid = (*env)->GetFieldID(env,ctrl_class,"data_valid","B");
			one_key_ctrl->ctrl.alarm_mode.data_valid = (*env)->GetByteField(env,ctrl_obj,fid);
			fid = (*env)->GetFieldID(env,ctrl_class,"alarm_mode","B");
			one_key_ctrl->ctrl.alarm_mode.alarm_mode = (*env)->GetByteField(env,ctrl_obj,fid);
			fid = (*env)->GetFieldID(env,ctrl_class,"on_time","B");
			one_key_ctrl->ctrl.alarm_mode.on_time = (*env)->GetByteField(env,ctrl_obj,fid);
			
			fid = (*env)->GetFieldID(env,ctrl_class,"off_time","S");
			one_key_ctrl->ctrl.alarm_mode.off_time = (*env)->GetShortField(env,ctrl_obj,fid);
			fid = (*env)->GetFieldID(env,ctrl_class,"total_time","S");
			one_key_ctrl->ctrl.alarm_mode.total_time = (*env)->GetShortField(env,ctrl_obj,fid);			
			break;
		case OKT_ONOFF:
			one_key_ctrl->len = sizeof(cl_rfdev_onekey_set_onoff_t);
			//read the smart alarm info object.
			ctrl_class = (*env)->FindClass(env,CLASS_RF_GW_ONEKEY_SET_ONOFF);
			fid = (*env)->GetFieldID(env,info_class,"onoff","L"CLASS_RF_GW_ONEKEY_SET_ONOFF";");
			ctrl_obj = (*env)->GetObjectField(env,onekey_ctrl_info,fid);
			
			fid = (*env)->GetFieldID(env,ctrl_class,"onoff","Z");
			one_key_ctrl->ctrl.onoff.onoff= (*env)->GetBooleanField(env,ctrl_obj,fid);
			break;
	}

	//fres class 
	SAFE_DEL_LOCAL_REF(ctrl_class);
	SAFE_DEL_LOCAL_REF(ctrl_obj);
	
	SAFE_DEL_LOCAL_REF(info_class);
	
	return one_key_ctrl;
}


JNIEXPORT jint JNICALL
NAME(ClYinsuLampCtrl)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeState)
{
	int ret;
	cl_rf_lamp_stat_t *lamp_info = get_rf_light_state(env, ledeState);
	ret = cl_yinsu_lamp_ctrl(dev_handle, lamp_info);
	
	SAFE_FREE(lamp_info);
	
	return ret;
}


JNIEXPORT jint JNICALL
NAME(ClLeisLampCtrl)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeState)
{
	int ret;
	cl_rf_lamp_stat_t *lamp_info = get_rf_light_state(env, ledeState);
	ret = cl_leis_lamp_ctrl(dev_handle, lamp_info);
	
	SAFE_FREE(lamp_info);
	
	return ret;
}

/**/
JNIEXPORT jint JNICALL
NAME(ClDrkzqCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_drkzq_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClDrkzqSetName)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jstring name)
{
	
	if (name == NULL) {
		LOGE("name is null");
		return RS_INVALID_PARAM;
	}
	
	int ret;
	const char * c_name= (*env)->GetStringUTFChars(env, name, NULL);
	
	ret = cl_drkzq_set_name(dev_handle, type, (char*)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClDrkzqSetTime)(JNIEnv* env, jobject this, jint dev_handle, jbyte id, jboolean valid, jbyte start_hour, jbyte start_min, jbyte end_hour, jbyte end_min)
{
	return cl_drkzq_set_timer(dev_handle, id, valid, start_hour, start_min, end_hour, end_min);
}

/*夜郎*/
JNIEXPORT jint JNICALL
NAME(ClRfYlSetTime)(JNIEnv* env, jobject this, jint dev_handle, jshort time)
{
	return cl_rfdev_set_yl_time(dev_handle, time);
}

JNIEXPORT jint JNICALL
NAME(ClRfYlSetVoice)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jbyte value)
{
	return cl_rfdev_set_yl_voice(dev_handle, type, value);
}

JNIEXPORT jint JNICALL
NAME(ClRfYlSetSirenOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_rfdev_set_yl_siren_onoff(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClRfDefenseBatch)(JNIEnv* env, jobject this, jint dev_handle, jlongArray sn)
{
	jlong *c_sn = NULL;
	u_int8_t num = 0;
	int ret = 0;

	num = (u_int8_t)(*env)->GetArrayLength(env, sn);
	c_sn = (*env)->GetLongArrayElements(env, sn, NULL);
	
	ret = cl_rf_dev_config_defense_batch(dev_handle, num, (u_int64_t*)c_sn);
	(*env)->ReleaseLongArrayElements(env, sn, c_sn, JNI_ABORT);
}
//花瓶网关相关的接口
JNIEXPORT jint JNICALL
NAME(ClHpGwConfigSms)(JNIEnv* env, jobject this, jint gw_handle, jboolean onoff,jbyte lang)
{
	return cl_hpgw_config_sms(gw_handle,onoff,lang);
}
JNIEXPORT jint JNICALL
NAME(ClHpGwConfigAppInfoOnOff)(JNIEnv* env, jobject this, jint gw_handle, jboolean onoff)
{
	return cl_hpgw_config_appinfo_onoff(gw_handle,onoff);
}
JNIEXPORT jint JNICALL
NAME(ClHpGwConfigPhoneUser)(JNIEnv* env, jobject this, jint gw_handle, jstring name,jlong number)
{
	const char* user_name = NULL;
	user_name = (*env)->GetStringUTFChars(env, name, NULL);
	return cl_hpgw_config_phone_user(gw_handle,user_name,number);
}
JNIEXPORT jint JNICALL
NAME(ClHpGwDeletePhoneUser)(JNIEnv* env, jobject this, jint gw_handle, jlong number)
{
	return cl_hpgw_del_phone_user(gw_handle,number);
}

JNIEXPORT jint JNICALL
NAME(clHpGwLampCtrl)(JNIEnv* env, jobject this, jint gw_handle, jobject state)
{
	int ret;
	cl_rf_lamp_stat_t *lamp_info = get_rf_light_state(env, state);

	ret =  cl_hpgw_lamp_ctrl(gw_handle,lamp_info);
	SAFE_FREE(lamp_info);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFGwOneKeyCtrl)(JNIEnv* env, jobject this, jint gw_handle, jobject info)
{
	int ret;
	cl_rfdev_onekey_ctrl_t *one_key_ctrl = get_rfgw_one_key_ctrl(env,info);
	ret =  cl_rfgw_onekey_ctrl(gw_handle,one_key_ctrl);
	
	SAFE_FREE(one_key_ctrl);
	return ret;
}


/*创佳温控器*/
JNIEXPORT jint JNICALL
NAME(ClCjCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jbyte value)
{
	return cl_cjthermostat_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClCjSetPeriodTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jbyteArray temp)
{
	jsize len  = (*env)->GetArrayLength(env, temp);
	u_int8_t *c_temp = NULL;
	int ret = RS_OK;
	
	if(len < 0){
		return RS_INVALID_PARAM;
	}
	c_temp = (*env)->GetByteArrayElements(env, temp, NULL);
	ret = cl_cjthermostat_period_temp(dev_handle, type, c_temp);
	(*env)->ReleaseByteArrayElements(env, temp, c_temp, 0);
	
	return ret;
}


/*LINKON温控器*/
JNIEXPORT jint JNICALL
NAME(ClLinkonCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_linkon_sample_ctrl(dev_handle, action, value);
}


/*进睿心热水器*/
JNIEXPORT jint JNICALL
NAME(ClJrxHeaterCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_jrxheater_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClJrxHeaterPeriodTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jboolean valid, jboolean enable, 
	                          jbyte id, jbyte startHour, jbyte startMin, jbyte endHour, jbyte endMin, jbyte temp)
{
	return cl_jrxheater_period_timer(dev_handle, action, valid, enable, id, startHour, startMin, endHour, endMin, temp);
}
/*邦普温控器*/
JNIEXPORT jint JNICALL
NAME(ClBpThermostatCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_bpuair_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClBpThermostatOnceTimer)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable, jboolean onoff,
                               jbyte hour, jbyte minute)
{
	return cl_bpuair_once_timer(dev_handle, enable, onoff, hour, minute);
}

JNIEXPORT jint JNICALL
NAME(ClBpThermostatPeriodTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte id, jboolean enable, 
                                jbyte week, jboolean onoff, jbyte hour, jbyte minute)
{
	return cl_bpuair_period_timer(dev_handle, id, enable, week, onoff, hour, minute);
}



/*宁波华佑温控器*/
JNIEXPORT jint JNICALL
NAME(ClHyThermostatCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_hythermostat_ctrl(dev_handle, action, value);
}


/*中科睿赛空气净化器*/
JNIEXPORT jint JNICALL
NAME(ClZkCleannerCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_zkcleanner_ctrl(dev_handle, action, value);
}


/*印度车*/
JNIEXPORT jint JNICALL
NAME(ClIndiaCarUpgradeReq)(JNIEnv* env, jobject this, jint dev_handle, jbyte major, jbyte minor, jbyte svn, jstring url)
{
	char *cUrl = NULL;
	int ret = RS_OK;

	if (NULL == url) {
		return RS_INVALID_PARAM;
	}
	
	cUrl = (char *)((*env)->GetStringUTFChars(env, url, NULL));
	ret = cl_indiacar_do_dev_upgrade_request(dev_handle, major, minor, svn, cUrl);
	(*env)->ReleaseStringUTFChars(env, url, cUrl);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClIndiaCarHistoryReq)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jbyte id, jint year, jint month, jint day)
{
	return cl_indiacar_do_history_request(dev_handle, type, id, year, month, day);
}

JNIEXPORT jbyteArray JNICALL
NAME(ClIndiaCarGetAudio)(JNIEnv* env, jobject this, jint dev_handle)
{
	u_int32_t size = 0;
	void *audio = NULL;
	int ret = RS_OK;
	jbyteArray audio_bytes = NULL;

	if (cl_indiaocar_video_get_audio(dev_handle, &audio, &size) != RS_OK) {
		return NULL;
	}
	audio_bytes = (*env)->NewByteArray(env, size);
	(*env)->SetByteArrayRegion(env, audio_bytes, 0, size, (jbyte *)audio);
	return audio_bytes;
}


JNIEXPORT jobject JNICALL
NAME(ClIndiaCarGetJourneyIdStat)(JNIEnv* env, jobject this, jint dev_handle, jint year, jint month, jint day)
{
	jclass class_ids = NULL;
	jobject obj_ids = NULL;
	cl_indiacar_journey_id_list_t *journey_ids = cl_indiacar_journey_id_stat_get(dev_handle, year, month, day);

	if (journey_ids == NULL) {
		return NULL;
	}
	class_ids = (*env)->FindClass(env, CLASS_INDIACAR_JOURNEY_IDS);
	obj_ids = (*env)->AllocObject(env, class_ids);

	jni_copy_simple_class(env, class_ids, obj_ids,
		                     QUADRUPLE(byte[], journey_ids, ids, 256),
		                     JNI_VAR_ARG_END);
	cl_indiacar_journey_id_list_free(journey_ids);
	SAFE_DEL_LOCAL_REF(class_ids);
	//SAFE_DEL_LOCAL_REF(obj_ids);
	return obj_ids;
}

JNIEXPORT jobject JNICALL
NAME(ClIndiaCarGetJourneyInfo)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jbyte id, jint year, jint month, jint day)
{
	jclass class_info = NULL;
	jobject obj_info = NULL;
	india_car_history_flash_hd_t *journey_history = cl_indiacar_journey_infomation_get(dev_handle, type, id, year, month, day);

	if (journey_history == NULL) {
		return NULL;
	}
	class_info = (*env)->FindClass(env, CLASS_INDIACAR_HISTORY);
	obj_info = (*env)->AllocObject(env, class_info);

	JNI_COPY_SIMPLE_CLASS(env, class_info, obj_info, CLASS_INDIACAR_HISTORY_HEADER, hd,
		                     TRIPLES(int, journey_history, magic),
		                     TRIPLES(byte, journey_history, type),
		                     TRIPLES(byte, journey_history, id),
		                     TRIPLES(byte, journey_history, ver),
		                     TRIPLES(int, journey_history, date),
		                     TRIPLES(int, journey_history, data_len),
		                     QUADRUPLE(byte[], journey_history, data, journey_history->data_len),
		                     JNI_VAR_ARG_END);
	cl_indiacar_journey_infomation_free(journey_history);
	SAFE_DEL_LOCAL_REF(class_info);
	//SAFE_DEL_LOCAL_REF(obj_info);
	return obj_info;
}

JNIEXPORT int JNICALL
NAME(ClIndiaCarSetWarnning)(JNIEnv* env, jobject this, jint dev_handle, jobject warn)
{
	jclass class_warn = NULL;
	india_car_warn_t c_warn;
	jclass class_longi_lati = NULL;
	jobject obj_longi_lati = NULL;
	jfieldID fid = NULL;

	class_warn = (*env)->FindClass(env, CLASS_INDIACAR_WARN);
	class_longi_lati = (*env)->FindClass(env, CLASS_INDIACAR_LONGILATI);

	jni_assign_simple_struct(env, warn, class_warn,
		                        ASSIGN_TRIPLES(int, &c_warn, onoff),
		                        ASSIGN_TRIPLES(byte, &c_warn, max_speed),
		                        ASSIGN_TRIPLES(byte, &c_warn, reset_time),
		                        ASSIGN_TRIPLES(short, &c_warn, max_distance),
		                        ASSIGN_TRIPLES(short, &c_warn, max_time),
		                        ASSIGN_TRIPLES(short, &c_warn, max_freerun_time),
		                        ASSIGN_TRIPLES(byte, &c_warn, max_vol),
		                        ASSIGN_TRIPLES(byte, &c_warn, report_period),
		                        ASSIGN_TRIPLES(byte, &c_warn, geo_fencing_radius),
		                        ASSIGN_TRIPLES(byte, &c_warn, auto_upgrade_onoff),
		                        ASSIGN_TRIPLES(byte, &c_warn, break_level),
		                        JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_warn, "longitude", "L"CLASS_INDIACAR_LONGILATI";" );
	obj_longi_lati = (*env)->GetObjectField(env, warn, fid);
	jni_assign_simple_struct(env, obj_longi_lati, class_longi_lati,
		                        ASSIGN_TRIPLES(int, &c_warn.longitude, high),
		                        ASSIGN_TRIPLES(int, &c_warn.longitude, low),
		                        JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_warn, "latitude", "L"CLASS_INDIACAR_LONGILATI";" );
	obj_longi_lati = (*env)->GetObjectField(env, warn, fid);
	jni_assign_simple_struct(env, obj_longi_lati, class_longi_lati,
		                        ASSIGN_TRIPLES(int, &c_warn.latitude, high),
		                        ASSIGN_TRIPLES(int, &c_warn.latitude, low),
		                        JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(class_longi_lati);
	return cl_indiacar_warn_set(dev_handle, &c_warn);
}

JNIEXPORT jint JNICALL
NAME(ClIndiaCarSetWifi)(JNIEnv* env, jobject this, jint dev_handle, jobject wifi)
{
	jclass class_wifi = NULL;
	india_car_wifi_config_t c_wifi;
	const char *pssid = NULL;
	const char *ppwd = NULL;
	jfieldID fid = NULL;
	jstring str = NULL;
	jstring pwd_str = NULL;

	class_wifi = (*env)->GetObjectClass(env, wifi);
	
	fid = (*env)->GetFieldID(env, class_wifi, "stat", "B");
	c_wifi.stat = (*env)->GetByteField(env, wifi, fid);
	fid = (*env)->GetFieldID(env, class_wifi, "ssid", "Ljava/lang/String;");
	str = (*env)->GetObjectField(env, wifi, fid);
	pssid = (*env)->GetStringUTFChars(env, str, NULL);

	fid = (*env)->GetFieldID(env, class_wifi, "pwd", "Ljava/lang/String;");
	pwd_str = (*env)->GetObjectField(env, wifi, fid);
	ppwd = (*env)->GetStringUTFChars(env, pwd_str, NULL);

	c_wifi.ssid_len = strlen(pssid);
	c_wifi.pwd_len = strlen(ppwd);
	strncpy(c_wifi.data, pssid, sizeof(c_wifi.data));
	strncpy(c_wifi.data + c_wifi.ssid_len + 1, ppwd, sizeof(c_wifi.data) - c_wifi.ssid_len - 1);

	(*env)->ReleaseStringUTFChars(env, str, pssid);
	(*env)->ReleaseStringUTFChars(env, pwd_str, ppwd);
	
	return cl_indiacar_wifi_config(dev_handle, &c_wifi);
}

JNIEXPORT jint JNICALL
NAME(ClIndiaCarRealtimeTripReq)(JNIEnv* env, jobject this, jint dev_handle, jbyte type)
{
	return cl_indiacar_reatime_trip_request(dev_handle, type);
}

JNIEXPORT jobject JNICALL
NAME(ClIndiaCarGetJourneyCount)(JNIEnv* env, jobject this, jint dev_handle)
{
	jclass class_jn = NULL;
	jobject obj_jn = NULL;
	cl_indiacar_jorney_num_t c_jn;

	if (cl_indiacar_get_jorney_count(dev_handle, &c_jn) == RS_OK) {
		class_jn = (*env)->FindClass(env, CLASS_INDIACAR_JOURNEY_NUM);
		obj_jn = (*env)->AllocObject(env, class_jn);
		jni_copy_simple_class(env, class_jn, obj_jn,
			                     TRIPLES(int, &c_jn, date),
			                     TRIPLES(byte, &c_jn, count),
			                     JNI_VAR_ARG_END);
		SAFE_DEL_LOCAL_REF(class_jn);
	}
	
	return obj_jn;
}

JNIEXPORT jint JNICALL
NAME(ClIndiaCarSetDebug)(JNIEnv* env, jobject this, jint dev_handle, jobject debug)
{
	jclass class_debug = NULL;
	india_car_debug_config_t c_debug;

	class_debug = (*env)->GetObjectClass(env, debug);
	jni_assign_simple_struct(env, debug, class_debug,
		                                    ASSIGN_TRIPLES(byte, &c_debug, cmd),
		                                    ASSIGN_TRIPLES(byte, &c_debug, onoff),
		                                    ASSIGN_TRIPLES(short, &c_debug, cmd_len),
		                                    ASSIGN_TRIPLES(short, &c_debug, gps_time_inv),
		                                    ASSIGN_TRIPLES(short, &c_debug, remote_port),
		                                    ASSIGN_TRIPLES(int, &c_debug, remote_ip),
		                                    ASSIGN_TRIPLES(short, &c_debug, gps_len_inv),
		                                    ASSIGN_TRIPLES(short, &c_debug, file_debug_enable),
		                                    ASSIGN_TRIPLES(short, &c_debug, file_debug_level),
		                                    ASSIGN_TRIPLES(short, &c_debug, file_debug_url_len),
		                                    ASSIGN_TRIPLES(short, &c_debug, bps),
		                                    ASSIGN_TRIPLES(byte, &c_debug, video_rotate),
		                                    ASSIGN_TRIPLES(short, &c_debug, moto_threshold),
		                                    ASSIGN_TRIPLES(short, &c_debug, detail_save_inv),
		                                    ASSIGN_TRIPLES(byte, &c_debug, power),
		                                    ASSIGN_TRIPLES(byte, &c_debug, realtime_inv),
		                                    ASSIGN_QUAD(String, &c_debug, url, 256),
		                                    JNI_VAR_ARG_END);

	return cl_indiacar_debug_config(dev_handle, &c_debug);
}

JNIEXPORT jint JNICALL
NAME(ClIndiaCarStartVideo)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_indiacar_video_start(dev_handle, onoff);
}

JNIEXPORT jobject JNICALL
NAME(ClIndiaCarGetPicture)(JNIEnv* env, jobject this, jint dev_handle)
{
	u_int32_t size = 0;
	void *pic = NULL;
	int ret = RS_OK;
	jbyteArray pic_bytes = NULL;

	if (cl_indiaocar_video_get_picture(dev_handle, &pic, &size) != RS_OK) {
		return NULL;
	}
	pic_bytes = (*env)->NewByteArray(env, size);
	(*env)->SetByteArrayRegion(env, pic_bytes, 0, size, (jbyte *)pic);
	return pic_bytes;
}

JNIEXPORT jint JNICALL
NAME(clIndiaCarLocalVideoReq)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint year,
                               jint month, jint day)
{
	return cl_indiaocar_request_local_watch(dev_handle, action, year, month, day);
}

JNIEXPORT jint JNICALL
NAME(ClIndiaCarVideoRecord)(JNIEnv* env, jobject this, jint dev_handle, jstring path, jboolean onoff)
{
	char *c_path = NULL;
	int ret = RS_OK;

	if (NULL == path) {
		return RS_INVALID_PARAM;
	}
	
	c_path = (char *)((*env)->GetStringUTFChars(env, path, NULL));
	ret = cl_indiacar_video_record(dev_handle,c_path, onoff);
	(*env)->ReleaseStringUTFChars(env, path, c_path);
	return ret;
}

/*智科热水器*/
JNIEXPORT jint JNICALL
NAME(ClZkCtrl)(JNIEnv* env, jobject this, jint dev_handle, jint action, jbyte value)
{
	return cl_zkrsq_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClZkConf)(JNIEnv* env, jobject this, jint dev_handle, jobject config)
{
	jclass class_zk_param = NULL;
	cl_zk_confset_t c_config;

	class_zk_param = (*env)->GetObjectClass(env, config);
	jni_assign_simple_struct(env, config, class_zk_param,
		                        ASSIGN_TRIPLES(byte, &c_config, back_tmp),
		                        ASSIGN_TRIPLES(byte, &c_config, compensation_tmp),
		                        ASSIGN_TRIPLES(byte, &c_config, defrost_time),
		                        ASSIGN_TRIPLES(byte, &c_config, defrost_in_tmp),
		                        ASSIGN_TRIPLES(byte, &c_config, defrost_continue_time),
		                        ASSIGN_TRIPLES(byte, &c_config, defrost_out_tmp),
		                        ASSIGN_TRIPLES(byte, &c_config, back_water_tmp),
		                        ASSIGN_TRIPLES(byte, &c_config, back_water_mode),
		                        JNI_VAR_ARG_END);
	return cl_zkrsq_config(dev_handle, &c_config);
}

JNIEXPORT jint JNICALL
NAME(ClZkTimer)(JNIEnv* env, jobject this, jint dev_handle, jobject timer)
{
	jclass class_zk_timer;
	cl_zkrsq_tiemr_t c_timer;

	class_zk_timer = (*env)->GetObjectClass(env, timer);
	jni_assign_simple_struct(env, timer, class_zk_timer,
		                        ASSIGN_TRIPLES(boolean, &c_timer, timer1_valid),
		                        ASSIGN_TRIPLES(boolean, &c_timer, timer1_onoff),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer1_hour),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer1_min),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer1_hour_end),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer1_min_end),
		                        ASSIGN_TRIPLES(boolean, &c_timer, timer2_valid),
		                        ASSIGN_TRIPLES(boolean, &c_timer, timer2_onoff),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer2_hour),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer2_min),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer2_hour_end),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer2_min_end),
		                        ASSIGN_TRIPLES(boolean, &c_timer, timer3_valid),
		                        ASSIGN_TRIPLES(boolean, &c_timer, timer3_onoff),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer3_hour),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer3_min),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer3_hour_end),
		                        ASSIGN_TRIPLES(byte, &c_timer, timer3_min_end),
		                        JNI_VAR_ARG_END);
	return cl_zkrsq_timer(dev_handle, &c_timer);
}

/*御源净水器*/
JNIEXPORT jint JNICALL
NAME(ClYyWaterBox)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_water_box(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyInletTimeout)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_inlet_timeout(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyImpulseCount)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_impulse_count(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyImpulsePeriod)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_impulse_period(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyMcDelay)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_mcdelay(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyNmValveDelay)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_nm_valve_delay(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyFuncValveTimeout)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_func_valve_timeout(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYySpeed)(JNIEnv* env, jobject this, jint dev_handle, jbyte value1, jbyte value2, jbyte value3)
{
	return cl_yuyuan_speed(dev_handle, value1, value2, value3);
}

JNIEXPORT jint JNICALL
NAME(ClYyLoopOnoff)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_yuyuan_loop_onoff(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClYyLoop)(JNIEnv* env, jobject this, jint dev_handle, jbyte value1, jbyte value2, jbyte value3, jbyte value4)
{
	return cl_yuyuan_loop(dev_handle, value1, value2, value3, value4);
}

JNIEXPORT jint JNICALL
NAME(ClYyMcClean)(JNIEnv* env, jobject this, jint dev_handle, jbyte value1, jbyte value2, jbyte value3, jbyte value4, jbyte value5)
{
	return cl_yuyuan_mcclean(dev_handle, value1, value2, value3, value4, value5);
}

JNIEXPORT jint JNICALL
NAME(ClYyNmClean)(JNIEnv* env, jobject this, jint dev_handle, jbyte value1, jbyte value2, jbyte value3, jbyte value4, jbyte value5)
{
	return cl_yuyuan_nmclean(dev_handle, value1, value2, value3, value4, value5);
}

JNIEXPORT jint JNICALL
NAME(ClYyMincroClean)(JNIEnv* env, jobject this, jint dev_handle, jbyte action)
{
	return cl_yuyuan_mincro_clean(dev_handle, action);
}

JNIEXPORT jint JNICALL
NAME(ClYyNmCleanStart)(JNIEnv* env, jobject this, jint dev_handle, jbyte action)
{
	return cl_yuyuan_nm_clean(dev_handle, action);
}

JNIEXPORT jint JNICALL
NAME(ClYyCheckSelf)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_yuyuan_check_self(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClYyRebootCleaner)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_yuyuan_reboot_cleanner(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClYyConfig)(JNIEnv* env, jobject this, jint dev_handle, jbyteArray cmd)
{
	u_int8_t *c_cmd = NULL;
	int ret = RS_ERROR;
	jsize len  = (*env)->GetArrayLength(env, cmd);
	
	if(len<0){
		return RS_INVALID_PARAM;
	}
	c_cmd = (*env)->GetByteArrayElements(env, cmd, NULL);
	ret = cl_yuyuan_config(dev_handle, c_cmd, len);
	(*env)->ReleaseByteArrayElements(env, cmd, c_cmd, 0);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClYySetPwd)(JNIEnv* env, jobject this, jint dev_handle, jstring pwd)
{
	u_int8_t *cStrPwd = NULL;
	int ret = RS_OK;

	if (NULL == pwd) {
		return RS_INVALID_PARAM;
	}
	
	cStrPwd = (u_int8_t *)((*env)->GetStringUTFChars(env, pwd, NULL));
	ret = cl_yuyuan_set_pwd(dev_handle, cStrPwd, strlen(cStrPwd) + 1);
	
	(*env)->ReleaseStringUTFChars(env, pwd, cStrPwd);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClYySetRemind)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff, jint remindTime)
{
	return cl_yuyuan_set_remind(dev_handle, onoff, remindTime);
}



/*益佳电暖炉*/
JNIEXPORT jint JNICALL
NAME(ClYjCtrl)(JNIEnv* env, jobject this, jint dev_handle, jobject ctrl)
{
	jclass class_yj_set;
	cl_yj_heater_set_t c_yj_set;

	class_yj_set = (*env)->GetObjectClass(env, ctrl);
	
	jni_assign_simple_struct(env, ctrl, class_yj_set,
		                        ASSIGN_TRIPLES(boolean, &c_yj_set, onoff),
		                        ASSIGN_TRIPLES(byte, &c_yj_set, temp_set),
		                        ASSIGN_TRIPLES(byte, &c_yj_set, temp_type),
		                        ASSIGN_TRIPLES(boolean, &c_yj_set, anion),
		                        ASSIGN_TRIPLES(byte, &c_yj_set, power),
		                        ASSIGN_TRIPLES(boolean, &c_yj_set, child_lock),
		                        ASSIGN_TRIPLES(byte, &c_yj_set, timer),
		                        JNI_VAR_ARG_END);
	//LOGD("xxxddd ClYjCtrl: %hhu %hhu %hhu %hhu %hhu %hhu %hhu\n", c_yj_set.onoff,
	//	c_yj_set.temp_set, c_yj_set.temp_type, c_yj_set.anion,
	//	c_yj_set.power, c_yj_set.child_lock, c_yj_set.timer);
	return cl_yj_heater_set_ctrl(dev_handle, &c_yj_set);
}

/*中山商贤电热炉*/
JNIEXPORT jint JNICALL
NAME(ClZssxCtrl)(JNIEnv* env, jobject this, jint dev_handle, jint action, jshort value)
{
	return cl_zssx_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClZssxTimer)(JNIEnv* env, jobject this, jint dev_handle, jboolean on, jshort minute)
{
	return cl_zssx_timer(dev_handle, on, minute);
}

JNIEXPORT jint JNICALL
NAME(ClZssxWifiConf)(JNIEnv* env, jobject this, jint dev_handle, jobject wifi)
{
	jclass class_zssx_wifi;
	cl_zssx_ssid_t c_wifi;

	class_zssx_wifi = (*env)->GetObjectClass(env, wifi);
	jni_assign_simple_struct(env, wifi, class_zssx_wifi,
		                        ASSIGN_QUAD(String, &c_wifi, ssid, sizeof(c_wifi.ssid)),
		                        ASSIGN_QUAD(String, &c_wifi, pswd, sizeof(c_wifi.pswd)),
		                        JNI_VAR_ARG_END);
	return cl_zssx_wifi_conf(dev_handle, &c_wifi);
}

/*思博特温控器*/
JNIEXPORT jint JNICALL
NAME(ClSbtCtrlStat)(JNIEnv* env, jobject this, jint dev_handle, jobject workState)
{
	jclass class_stat = NULL;
	cl_sbt_work_ctrl_t c_stat;

	class_stat = (*env)->GetObjectClass(env, workState);

	jni_assign_simple_struct(env, workState, class_stat,
		                        ASSIGN_TRIPLES(boolean, &c_stat, onoff),
		                        ASSIGN_TRIPLES(byte, &c_stat, temp),
		                        ASSIGN_TRIPLES(byte, &c_stat, mode),
		                        ASSIGN_TRIPLES(byte, &c_stat, fan_speed),
		                        JNI_VAR_ARG_END);
	return cl_sbt_ther_ctrl_stat(dev_handle, &c_stat);
}

JNIEXPORT jint JNICALL
NAME(ClSbtSetParam)(JNIEnv* env, jobject this, jint dev_handle, jobject param)
{
	jclass class_param = NULL;
	cl_sbt_func_setting_t c_param;

	class_param = (*env)->GetObjectClass(env, param);
	jni_assign_simple_struct(env, param, class_param,
		                        ASSIGN_TRIPLES(boolean, &c_param, auto_mode),
		                        ASSIGN_TRIPLES(byte, &c_param, temp_adjust),
		                        ASSIGN_TRIPLES(byte, &c_param, low_temp),
		                        ASSIGN_TRIPLES(byte, &c_param, valve_mode),
		                        ASSIGN_TRIPLES(byte, &c_param, return_temp),
		                        ASSIGN_TRIPLES(boolean, &c_param, is_low_temp_guard),
		                        ASSIGN_TRIPLES(byte, &c_param, max_temp),
		                        ASSIGN_TRIPLES(byte, &c_param, min_temp),
		                        JNI_VAR_ARG_END);
	return cl_sbt_ther_set_param(dev_handle, &c_param);
}

JNIEXPORT jint JNICALL
NAME(ClSbtAdjustScmTime)(JNIEnv* env, jobject this, jint dev_handle, jobject adjust)
{
	jclass class_adjust = NULL;
	cl_sbt_time_adjust_t c_adjust;

	class_adjust = (*env)->GetObjectClass(env, adjust);
	jni_assign_simple_struct(env, adjust, class_adjust,
		                        ASSIGN_TRIPLES(byte, &c_adjust, scm_hour),
		                        ASSIGN_TRIPLES(byte, &c_adjust, scm_min),
		                        ASSIGN_TRIPLES(byte, &c_adjust, scm_sec),
		                        ASSIGN_TRIPLES(byte, &c_adjust, scm_weekday),
		                        JNI_VAR_ARG_END);
	return cl_sbt_ther_set_ajust_scm_time(dev_handle, &c_adjust);
}

static void assign_sbt_smart_day(JNIEnv* env, jclass class_day, jobject obj_day, cl_sbt_smart_day_t *day_info)
{
	jclass class_item = NULL;
	jobject obj_item = NULL;
	jobject itemArray = NULL;
	int i = 0;
	jfieldID fid;

	class_item = (*env)->FindClass(env, CLASS_SBT_SMART_ITEM);
	fid = (*env)->GetFieldID(env, class_day, "si", "[L"CLASS_SBT_SMART_ITEM";");
	itemArray = (*env)->GetObjectField(env, obj_day, fid);

	 for (i = 0; i < 4; ++i) {
		obj_item = (*env)->GetObjectArrayElement(env, itemArray, i);
		
		fid = (*env)->GetFieldID(env, class_item, "hour", "B");
		day_info->si[i].hour= (*env)->GetByteField(env, obj_item, fid);

		fid = (*env)->GetFieldID(env, class_item, "temp", "B");
		day_info->si[i].temp= (*env)->GetByteField(env, obj_item, fid);
		
		SAFE_DEL_LOCAL_REF(obj_item);
	}
	SAFE_DEL_LOCAL_REF(itemArray);
	SAFE_DEL_LOCAL_REF(class_item);
}


JNIEXPORT jint JNICALL
NAME(ClSbtSetSmartSetting)(JNIEnv* env, jobject this, jint dev_handle, jobject smart)
{
	jclass class_smart = NULL;
	cl_smart_smart_ctrl_t c_smart;
	jclass class_day = NULL;
	jobject obj_day = NULL;
	jfieldID fid = NULL;


	class_smart = (*env)->GetObjectClass(env, smart);
	fid = (*env)->GetFieldID(env, class_smart, "work_day", "L"CLASS_SBT_SMART_DAY";");
	obj_day = (*env)->GetObjectField(env, smart, fid);
	class_day = (*env)->GetObjectClass(env, obj_day);
	assign_sbt_smart_day(env, class_day, obj_day, &c_smart.work_day);
	
	fid = (*env)->GetFieldID(env, class_smart, "sat_day", "L"CLASS_SBT_SMART_DAY";");
	obj_day = (*env)->GetObjectField(env, smart, fid);
	assign_sbt_smart_day(env, class_day, obj_day, &c_smart.sat_day);

	fid = (*env)->GetFieldID(env, class_smart, "sun_day", "L"CLASS_SBT_SMART_DAY";");
	obj_day = (*env)->GetObjectField(env, smart, fid);
	assign_sbt_smart_day(env, class_day, obj_day, &c_smart.sun_day);
	
	return cl_sbt_ther_set_smart_config(dev_handle, &c_smart);
}

JNIEXPORT jint JNICALL
NAME(ClSbtSetSmartMode)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_sbt_ther_set_smart_config_mode(dev_handle, onoff);
}

/*晶石微波炉*/
JNIEXPORT jint JNICALL
NAME(ClJsWaveSetting)(JNIEnv* env, jobject this, jint dev_handle, jobject setting)
{
	jclass class_js = NULL;
	cl_js_wave_work_setting_t c_setting;

	class_js = (*env)->GetObjectClass(env, setting);

	jni_assign_simple_struct(env, setting, class_js,
		                        ASSIGN_TRIPLES(byte, &c_setting, work_mode),
		                        ASSIGN_TRIPLES(byte, &c_setting, work_min),
		                        ASSIGN_TRIPLES(byte, &c_setting, work_sec),
		                        ASSIGN_TRIPLES(byte, &c_setting, wave_fire),
		                        ASSIGN_TRIPLES(byte, &c_setting, barbecue_fire),
		                        ASSIGN_TRIPLES(byte, &c_setting, hot_fan_temp),
		                        ASSIGN_TRIPLES(short, &c_setting, food_weight),
		                        ASSIGN_TRIPLES(byte, &c_setting, work_sub_mode),
		                        ASSIGN_TRIPLES(byte, &c_setting, action),
		                        JNI_VAR_ARG_END);
	return cl_js_wave_ctrl(dev_handle, &c_setting);
}

JNIEXPORT jint JNICALL
NAME(ClJsWaveFastCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action)
{
	return cl_js_wave_fast_ctrl(dev_handle, action);
}

JNIEXPORT jint JNICALL
NAME(ClJsWaveChildLock)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_js_wave_ctrl_child_lock(dev_handle, onoff);
}

/*奥德绅*/
JNIEXPORT jint JNICALL
NAME(ClAdsCtrl)(JNIEnv* env, jobject this, jint dev_handle, jint action, jint value)
{
	return cl_ads_ctrl(dev_handle, action, (u_int8_t)value);
}

JNIEXPORT jint JNICALL
NAME(ClAdsConfiger)(JNIEnv* env, jobject this, jint dev_handle, jobject config)
{
	jclass class_ads_conf = NULL;
	cl_ads_conf_t c_conf;

	class_ads_conf = (*env)->GetObjectClass(env, config);
	
	jni_assign_simple_struct(env, config, class_ads_conf,
                          		ASSIGN_TRIPLES(short,&c_conf, sys_clock_time),
		                        ASSIGN_TRIPLES(short,&c_conf, first_start_time),
		                        ASSIGN_TRIPLES(short,&c_conf, first_end_time),
		                        ASSIGN_TRIPLES(short,&c_conf, second_start_time),
		                        ASSIGN_TRIPLES(short,&c_conf, second_end_time),
		                        ASSIGN_TRIPLES(short,&c_conf, defrost_time),
		                        ASSIGN_TRIPLES(short,&c_conf, defrost_tmp),
		                        JNI_VAR_ARG_END);

	return cl_ads_conf(dev_handle, &c_conf);
}



/*集利舞台灯*/
JNIEXPORT jint JNICALL
NAME(ClJlCtrl3200Lamp)(JNIEnv* env, jobject this, jint dev_handle, jboolean on_off, jshort color, jshort bright)
{
	return cl_jl_ctrl_3200_lamp(dev_handle, on_off, color, bright);
}

JNIEXPORT jint JNICALL
NAME(ClJlCtrl3200LampTotal)(JNIEnv* env, jobject this, jint dev_handle, jboolean on_off, jshort color, jshort bright, jshort total_bright)
{
	return cl_jl_ctrl_3200_total_bright_lamp(dev_handle, on_off, (u_int8_t)color, (u_int8_t)bright, (u_int8_t)total_bright);
}



/*月兔空调*/
JNIEXPORT jint JNICALL
NAME(ClYtCtrl)(JNIEnv* env, jobject this, jint dev_handle, jint action, jbyte value)
{
	return cl_yt_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClYtTimerConfiger)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_timer)
{
	jclass class_timer = NULL;
	cl_yt_timer_t timer;

	class_timer = (*env)->GetObjectClass(env, obj_timer);
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &timer, on_enable),
		                        ASSIGN_TRIPLES(short, &timer, on_remain_min),
		                        ASSIGN_TRIPLES(byte, &timer, off_enable),
		                        ASSIGN_TRIPLES(short, &timer, off_remain_min),
		                        JNI_VAR_ARG_END);
	
	return cl_yt_timer_config(dev_handle, &timer);
}

JNIEXPORT jint JNICALL
NAME(ClYtScanSn)(JNIEnv* env, jobject this, jint dev_handle, jstring sn)
{
	char *strSn = NULL;
	int ret = RS_OK;

	if (NULL == sn) {
		return RS_INVALID_PARAM;
	}
	
	strSn = (char *)((*env)->GetStringUTFChars(env, sn, NULL));
	ret = cl_yt_scan_sn(dev_handle, strSn);
	
	(*env)->ReleaseStringUTFChars(env, sn, strSn);
	
	return ret;
}
/*
JNIEXPORT jint JNICALL
NAME(ClYtGetElec)(JNIEnv* env, jobject this, jint dev_handle, jobject elec)
{
	cl_yt_ele_t c_elec;
	jclass class_yt_elec = NULL;
	
	int ret = cl_yt_get_ele(dev_handle, &c_elec);
	if (ret != RS_OK) {
		return ret;
	}

	class_yt_elec = (*env)->GetObjectClass(env, elec);
	jni_copy_simple_class(env, class_yt_elec, elec,
		                    TRIPLES(int, &c_elec, ele_total),
		                    TRIPLES(byte, &c_elec, cur_month),
		                    QUADRUPLE(int[], &c_elec, month, 12),
		                    TRIPLES(int, &c_elec, on_ele),
		                    JNI_VAR_ARG_END);
	
	return ret;
}
*/
JNIEXPORT jint JNICALL
NAME(ClYtQueryElec)(JNIEnv* env, jobject this, jint dev_handle, jobject query)
{
	jclass class_query = NULL;
	cl_query_ele_time_t c_query;

	class_query = (*env)->GetObjectClass(env, query);
	jni_assign_simple_struct(env, query, class_query,
		                        ASSIGN_TRIPLES(short, &c_query, begin_year),
		                        ASSIGN_TRIPLES(byte, &c_query, begin_month),
		                        ASSIGN_TRIPLES(byte, &c_query, begin_day),
		                        ASSIGN_TRIPLES(short, &c_query, end_year),
		                        ASSIGN_TRIPLES(byte, &c_query, end_month),
		                        ASSIGN_TRIPLES(byte, &c_query, end_day),
		                        JNI_VAR_ARG_END);
	
	return cl_yt_query_ele(dev_handle, &c_query);
}

/*科希曼*/
JNIEXPORT jint JNICALL
NAME(ClKxmCtrlPower)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_kxm_ctrl_host_onoff(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClKxmCtrlTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte idx, jobject timer)
{
	jclass class_timer = NULL;
	cl_kxm_timer_info_t kxm_timer;

	class_timer = (*env)->GetObjectClass(env, timer);
	
	jni_assign_simple_struct(env, timer, class_timer,
                          		ASSIGN_TRIPLES(byte,&kxm_timer, on_hour),
		                        ASSIGN_TRIPLES(byte,&kxm_timer, on_min),
		                        ASSIGN_TRIPLES(byte,&kxm_timer, off_hour),
		                        ASSIGN_TRIPLES(byte,&kxm_timer, off_min),
		                        JNI_VAR_ARG_END);
	
	return cl_kxm_ctrl_timer(dev_handle, idx, &kxm_timer);
}

JNIEXPORT jint JNICALL
NAME(ClKxmCtrlMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode,jbyte temp)
{
	return cl_kxm_ctrl_mode(dev_handle, mode, temp);
}


JNIEXPORT jint JNICALL
NAME(ClKxmSetDevTime)(JNIEnv* env, jobject this, jint dev_handle, jbyte min, jbyte sec)
{
	return cl_kxm_set_dev_time(dev_handle, min, sec);
}

JNIEXPORT jint JNICALL
NAME(ClKxmCtrlAllTimer)(JNIEnv* env, jobject this, jint dev_handle, jobjectArray timer)
{
	cl_kxm_timer_info_t c_timer[MAX_KXM_TIMER_CNT];
	int i = 0;
	jobject obj_timer = NULL;
	jclass class_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_KXM_TIMER);

	for (i = 0; i < MAX_KXM_TIMER_CNT; ++i) {
		obj_timer = (*env)->GetObjectArrayElement(env, timer, i);;
		jni_assign_simple_struct(env, obj_timer, class_timer,
                          		ASSIGN_TRIPLES(byte,&c_timer[i], on_hour),
		                        ASSIGN_TRIPLES(byte,&c_timer[i], on_min),
		                        ASSIGN_TRIPLES(byte,&c_timer[i], off_hour),
		                        ASSIGN_TRIPLES(byte,&c_timer[i], off_min),
		                        JNI_VAR_ARG_END);
	}

	SAFE_DEL_LOCAL_REF(class_timer);
	return cl_kxm_ctrl_all_timer(dev_handle, c_timer);
}

JNIEXPORT jint JNICALL
NAME(ClKxmTherCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jbyte value)
{
	return cl_kxm_ther_ctrl(dev_handle, action, value);
}


/*海迅养生壶*/
JNIEXPORT jint JNICALL
NAME(clHxPotCtrlPower)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_hx_ysh_ctrl_onoff(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(clHxPotCtrlScene)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jshort sceneId)
{
	return cl_hx_ysh_ctrl_scene(dev_handle, action, sceneId);
}

JNIEXPORT jint JNICALL
NAME(clHxPotExecScene)(JNIEnv* env, jobject this, jint dev_handle, jobject scene)
{
	jclass class_scene = NULL;
	cl_hx_ysh_scene_t c_scene;

	class_scene = (*env)->FindClass(env, CLASS_HX_POT_SCENE);
	jni_assign_simple_struct(env,scene, class_scene,
		                        ASSIGN_TRIPLES(byte, &c_scene, temp),
		                        ASSIGN_TRIPLES(byte, &c_scene, time),
		                        ASSIGN_TRIPLES(byte, &c_scene, power),
		                        ASSIGN_TRIPLES(byte, &c_scene, keep_temp),
		                        ASSIGN_TRIPLES(byte, &c_scene, keep_time),
		                        ASSIGN_TRIPLES(short, &c_scene, scene_id),
		                        ASSIGN_TRIPLES(int, &c_scene, create_time),
		                        ASSIGN_QUAD(String, &c_scene, name, sizeof(c_scene.name)),
		                        JNI_VAR_ARG_END);

	return cl_hx_ysh_exec_scene(dev_handle, &c_scene);
}


JNIEXPORT jint JNICALL
NAME(clHxPotModScene)(JNIEnv* env, jobject this, jint dev_handle, jobject scene)
{
	jclass class_scene = NULL;
	cl_hx_ysh_scene_t c_scene;

	class_scene = (*env)->GetObjectClass(env, scene);
	jni_assign_simple_struct(env,scene, class_scene,
		                        ASSIGN_TRIPLES(byte, &c_scene, temp),
		                        ASSIGN_TRIPLES(byte, &c_scene, time),
		                        ASSIGN_TRIPLES(byte, &c_scene, power),
		                        ASSIGN_TRIPLES(byte, &c_scene, keep_temp),
		                        ASSIGN_TRIPLES(byte, &c_scene, keep_time),
		                        ASSIGN_TRIPLES(short, &c_scene, scene_id),
		                        ASSIGN_TRIPLES(int, &c_scene, create_time),
		                        ASSIGN_QUAD(String, &c_scene, name, sizeof(c_scene.name)),
		                        JNI_VAR_ARG_END);
	return cl_hx_ysh_modify_scene(dev_handle, &c_scene);
}




/*华天成泳池机*/

JNIEXPORT jint JNICALL
NAME(ClTbCommCtrlTemp)(JNIEnv* env, jobject this, jint dev_handle, jshort temp)
{
	return cl_tbb_ctrl_tmp(dev_handle,temp);
}


JNIEXPORT jint JNICALL
NAME(ClTbCommCtrlPower)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_tbb_ctrl_onoff(dev_handle,onoff);
}

JNIEXPORT jint JNICALL
NAME(ClTbCommCtrlMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode)
{
	return cl_tbb_ctrl_mode(dev_handle,mode);
}

void print_tb_config(cl_tbb_config_set_t *config)
{
	LOGD("xxxddd defrost: ------------\n");
	LOGD("xxxddd %hu %hu %hu %hu %hu %hu %hu \n", config->defrost.in_defrost_time,
		 config->defrost.in_defrost_tmp,  config->defrost.out_defrost_tmp,
		  config->defrost.out_defrost_time,  config->defrost.env_tmp,
		   config->defrost.dif,  config->defrost.defrost_continue_time);
	LOGD("xxxddd misc: ------------\n");
	LOGD("xxxddd %hhu %hhu %hu %hu %hu %hu\n", config->misc.sys_num, config->misc.sys_select,
		config->misc.work, config->misc.heat_pump_diff, config->misc.cool_diff, config->misc.hot_diff);
	LOGD("xxxddd protect: ------------\n");
	LOGD("xxxddd %hu %hu %hu %hu %hu %hu %hu %hu %hu \n", config->protect.dst_tmp_pro,
		config->protect.cool_out_water, config->protect.heat_out_water,
		config->protect.in_out_tmp, config->protect.pump_begin_time,
		config->protect.pump_delay_time, config->protect.wind_ordor_tmp,
		config->protect.env_tmp, config->protect.in_water_tmp);
	LOGD("xxxddd eev: ------------\n");
	LOGD("xxxddd %hu %hu %hu %hu \n", config->eev.ele_cycle, config->eev.hand_ctrl_step,
		config->eev.cool_ele_valve, config->eev.limit_day);
	LOGD("xxxddd set temp: ------------\n");
	LOGD("xxxddd %hu %hu %hu\n", config->auto_tmp.cool_tmp, config->auto_tmp.heat_tmp, config->auto_tmp.auto_tmp);
	LOGD("xxxddd bottom_ele_heat_tmp: ---------------\n");
	LOGD("xxxddd %hu\n", config->bottom_ele_heat_tmp);
}	

JNIEXPORT jint JNICALL
NAME(ClTbCommConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject config)
{
	jclass class_config = NULL;
	jfieldID fid  = NULL;
	jclass class_defrost = NULL;
	jobject obj_defrost = NULL;
	jclass class_misc = NULL;
	jobject obj_misc = NULL;
	jclass class_protect = NULL;
	jobject obj_protect = NULL;
	jclass class_eev = NULL;
	jobject obj_eev = NULL;
	jclass class_conf_temp = NULL;
	jobject obj_conf_temp = NULL;
	cl_tbb_config_set_t c_config;

	class_config = (*env)->FindClass(env, CLASS_TB_COMM_CONFIG);
	
	fid = (*env)->GetFieldID(env, class_config, "defrost", "L"CLASS_TB_COMM_DEFROST";");
	obj_defrost = (*env)->GetObjectField(env, config, fid);
	class_defrost = (*env)->GetObjectClass(env, obj_defrost);
	jni_assign_simple_struct(env, obj_defrost, class_defrost,
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, in_defrost_time),
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, in_defrost_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, out_defrost_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, out_defrost_time),
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, env_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, dif),
		                                    ASSIGN_TRIPLES(short ,&c_config.defrost, defrost_continue_time),
		                                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_config, "misc", "L"CLASS_TB_COMM_MISC";");
	obj_misc = (*env)->GetObjectField(env, config, fid);
	class_misc = (*env)->GetObjectClass(env, obj_misc);
	jni_assign_simple_struct(env, obj_misc, class_misc,
		                                    ASSIGN_TRIPLES(byte ,&c_config.misc, sys_num),
		                                    ASSIGN_TRIPLES(byte ,&c_config.misc, sys_select),
		                                    ASSIGN_TRIPLES(short ,&c_config.misc, work),
		                                    ASSIGN_TRIPLES(short ,&c_config.misc, heat_pump_diff),
		                                    ASSIGN_TRIPLES(short ,&c_config.misc, cool_diff),
		                                    ASSIGN_TRIPLES(short ,&c_config.misc, hot_diff),
		                                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_config, "protect", "L"CLASS_TB_COMM_PROTECT";");
	obj_protect = (*env)->GetObjectField(env, config, fid);
	class_protect = (*env)->GetObjectClass(env, obj_protect);
	jni_assign_simple_struct(env, obj_protect, class_protect,
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, dst_tmp_pro),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, cool_out_water),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, heat_out_water),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, in_out_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, pump_begin_time),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, pump_delay_time),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, wind_ordor_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, env_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.protect, in_water_tmp),
		                                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_config, "eev", "L"CLASS_TB_COMM_EEV";");
	obj_eev = (*env)->GetObjectField(env, config, fid);
	class_eev = (*env)->GetObjectClass(env, obj_eev);
	jni_assign_simple_struct(env , obj_eev, class_eev,
		                                    ASSIGN_TRIPLES(short ,&c_config.eev, ele_cycle),
		                                    ASSIGN_TRIPLES(short ,&c_config.eev, hand_ctrl_step),
		                                    ASSIGN_TRIPLES(short ,&c_config.eev, cool_ele_valve),
		                                    ASSIGN_TRIPLES(short ,&c_config.eev, limit_day),
		                                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_config, "auto_tmp", "L"CLASS_TB_COMM_CONF_TEMP";");
	obj_conf_temp = (*env)->GetObjectField(env, config, fid);
	class_conf_temp = (*env)->GetObjectClass(env, obj_conf_temp);
	jni_assign_simple_struct(env , obj_conf_temp, class_conf_temp,
		                                    ASSIGN_TRIPLES(short ,&c_config.auto_tmp, cool_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.auto_tmp, heat_tmp),
		                                    ASSIGN_TRIPLES(short ,&c_config.auto_tmp, auto_tmp),
		                                    JNI_VAR_ARG_END);
	jni_assign_simple_struct(env , config, class_config,
		                                    ASSIGN_TRIPLES(short ,&c_config, bottom_ele_heat_tmp),
		                                    JNI_VAR_ARG_END);
	//不注释掉要崩溃
	//SAFE_DEL_LOCAL_REF(class_config);
	//SAFE_DEL_LOCAL_REF(class_defrost);
	//SAFE_DEL_LOCAL_REF(class_misc);
	//SAFE_DEL_LOCAL_REF(class_protect);
	//SAFE_DEL_LOCAL_REF(class_eev);
	SAFE_DEL_LOCAL_REF(obj_defrost);
	SAFE_DEL_LOCAL_REF(obj_misc);
	SAFE_DEL_LOCAL_REF(obj_protect);
	SAFE_DEL_LOCAL_REF(obj_eev);

	print_tb_config(&c_config);

	return cl_tbb_config(dev_handle, &c_config);
}

/*bimar暖风机*/

JNIEXPORT jint JNICALL
NAME(ClBimarCtrl)(JNIEnv* env, jobject this, jint dev_handle, jint action, jbyte value)
{
	return cl_bimar_ctrl(dev_handle, action, value);
}

/*破壁机*/
JNIEXPORT jint JNICALL
NAME(ClHxCtrlWorkMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode)
{
	return cl_hx_ctrl_work_mode(dev_handle, mode);
}

JNIEXPORT jint JNICALL
NAME(ClHxCtrlWorkFinish)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_hx_finish_clear(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClHxCtrlDiyName)(JNIEnv* env, jobject this, jint dev_handle, jbyte id, jstring diy_name)
{
	char * name_p;
	int ret = RS_INVALID_PARAM;

	if (NULL == diy_name) {
		return ret;
	}
	
	name_p = (char *)((*env)->GetStringUTFChars(env, diy_name, NULL));
	ret = cl_hx_ctrl_diy_name(dev_handle, id, name_p);
	
	(*env)->ReleaseStringUTFChars(env, diy_name, name_p);
	
	return ret;
}

/*特林*/
JNIEXPORT jint JNICALL
NAME(ClTelinCtrlRefreshDevTime)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_tl_refresh_dev_time(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClTelinCtrlSync)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_tl_ctrl_clock_sync(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClTelinCtrlAutoSync)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_auto_sync)
{
	return cl_tl_ctrl_clock_auto_sync(dev_handle, is_auto_sync);
}

JNIEXPORT jint JNICALL
NAME(ClTelinCtrlOff)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_tl_ctrl_off(dev_handle, onoff);
}
JNIEXPORT jint JNICALL
NAME(ClTelinCtrlMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode)
{
	return cl_tl_ctrl_mode(dev_handle, mode);
}
JNIEXPORT jint JNICALL
NAME(ClTelinCtrlFanSpeed)(JNIEnv* env, jobject this, jint dev_handle, jbyte speed)
{
	return cl_tl_ctrl_fan_speed(dev_handle, speed);
}

JNIEXPORT jint JNICALL
NAME(ClTelinCtrlTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte temp)
{
	return cl_tl_ctrl_temp(dev_handle, temp);
}
JNIEXPORT jint JNICALL
NAME(ClTelinCtrlEco)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable)
{
	return cl_tl_ctrl_eco(dev_handle, enable);
}

JNIEXPORT jint JNICALL
NAME(ClTelinCtrlLearn)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable)
{
	return cl_tl_ctrl_learn(dev_handle, enable);
}

JNIEXPORT jint JNICALL
NAME(ClTelinSettingTimer)(JNIEnv* env, jobject this, jint dev_handle, jobject timer)
{
	cl_tl_timer_info_t timer_st = {0};
	jclass timer_class;
	jobject timer_obj;
	jfieldID fid;
	int i;

	timer_class = (*env)->FindClass(env, CLASS_TELIN_HEATING_TIME_ITEM);

	for (i = 0; i < TL_TIME_CNT_PER_DAY; i++) {
		timer_obj = (jobject)(*env)->GetObjectArrayElement(env, timer, i);

		fid = (*env)->GetFieldID(env, timer_class, "temp", "S");
		timer_st.time[i].temp= (*env)->GetShortField(env, timer_obj, fid);

		fid = (*env)->GetFieldID(env, timer_class, "hour", "S");
		timer_st.time[i].hour= (*env)->GetShortField(env, timer_obj, fid);

		fid = (*env)->GetFieldID(env, timer_class, "min", "S");
		timer_st.time[i].min= (*env)->GetShortField(env, timer_obj, fid);

		SAFE_DEL_LOCAL_REF(timer_obj);
	}
	
	SAFE_DEL_LOCAL_REF(timer_class);
	
	return cl_tl_setting_timer(dev_handle, &timer_st);
}


/*通用UDP控制接口*/

JNIEXPORT jint JNICALL
NAME(ClSetDevRestoreFactory)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_com_udp_set_dev_restore_factory(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClResetDevWifi)(JNIEnv* env, jobject this, jint dev_handle, jstring jssid, jstring jpwd)
{
	char * ssid;
	char * pwd;
	int ret = RS_ERROR;
	
	ssid = (char *)((*env)->GetStringUTFChars(env, jssid, NULL));
	pwd = (char *)((*env)->GetStringUTFChars(env, jpwd, NULL));

	ret = cl_com_udp_reset_dev_ssid_and_passwd(dev_handle, ssid, pwd);
	
	(*env)->ReleaseStringUTFChars(env, jssid, ssid);
	(*env)->ReleaseStringUTFChars(env, jpwd, pwd);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClResetDevWifiRaw)(JNIEnv* env, jobject this, jint dev_handle, jbyteArray ssid, jstring passwd)
{
	const char* password = NULL;
	char* ssid_str = NULL;
	char* strs = NULL;
	int len, i;
	RS ret;

	len = (*env)->GetArrayLength(env, ssid); 

	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (char*)((*env)->GetByteArrayElements(env, ssid, NULL));
	strs = (char*) malloc(len +1);

	for(i = 0; i < len; i++) {
		strs[i] = ssid_str[i];
	}
	
	strs[len] = '\0';
	ret = cl_com_udp_reset_dev_ssid_and_passwd(dev_handle,strs,password);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseByteArrayElements(env, ssid, (jbyte*)ssid_str, JNI_ABORT);
	free(strs);

	return ret;
	
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpTimerSet)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_timer)
{
	cl_air_timer_t timer;
	jclass clazz;
	jfieldID fid;

	memset(&timer, 0, sizeof(cl_air_timer_t));
	
	clazz = (*env)->FindClass(env, CLASS_UDP_DEV_TIMER);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	timer.id= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	timer.enable = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	timer.week= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	timer.hour = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	timer.minute = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "onoff", "I");
	timer.onoff = (*env)->GetIntField(env, obj_timer, fid);

	SAFE_DEL_LOCAL_REF(clazz);

	return cl_com_udp_timer_set(dev_handle, &timer);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpTimerDel)(JNIEnv* env, jobject this, jint dev_handle, jint id)
{
	return cl_com_udp_timer_del(dev_handle, id);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpPeriodTimerSet)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_timer)
{
	cl_period_timer_t period_timer;
	jclass clazz;
	jfieldID fid;

	memset(&period_timer, 0, sizeof(cl_period_timer_t));
	
	clazz = (*env)->FindClass(env, CLASS_UDP_PERIOD_TIMER);

	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	period_timer.id= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	period_timer.hour = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	period_timer.minute = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	period_timer.week= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	period_timer.enable = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "onoff", "Z");
	period_timer.onoff = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "duration", "I");
	period_timer.duration = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "ext_data_type", "I");
	period_timer.ext_data_type = (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);

	switch(period_timer.ext_data_type) {
	case PT_EXT_DT_QPCP:
		fid = (*env)->GetFieldID(env, clazz, "scene_id", "I");
		period_timer.pt_ext_data_u.qp_time_info.id= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);
		break;
	case PT_EXT_DT_QP_POT:
		fid = (*env)->GetFieldID(env, clazz, "cook_id", "I");
		period_timer.pt_ext_data_u.qp_pot_timer_info.cook_id= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);

		fid = (*env)->GetFieldID(env, clazz, "cook_time", "I");
		period_timer.pt_ext_data_u.qp_pot_timer_info.cook_time= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);

		fid = (*env)->GetFieldID(env, clazz, "hot_degress", "I");
		period_timer.pt_ext_data_u.qp_pot_timer_info.hot_degress= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);

		fid = (*env)->GetFieldID(env, clazz, "microswitch", "I");
		period_timer.pt_ext_data_u.qp_pot_timer_info.microswitch= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);

		fid = (*env)->GetFieldID(env, clazz, "warm_temp", "I");
		period_timer.pt_ext_data_u.qp_pot_timer_info.warm_temp= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);

		fid = (*env)->GetFieldID(env, clazz, "cooking_mode", "I");
		period_timer.pt_ext_data_u.qp_pot_timer_info.cooking_mode= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);
		break;
	case PT_EXT_DT_QP_PBJ:
		fid = (*env)->GetFieldID(env, clazz, "scene_id", "I");
		period_timer.pt_ext_data_u.qp_pbj_timer_info.scene_id= (u_int16_t)(*env)->GetIntField(env, obj_timer, fid);
		break;
	case PT_EXT_DT_101_OEM:
		fid = (*env)->GetFieldID(env, clazz, "min_temp", "B");
		period_timer.pt_ext_data_u.oem_101_timer_info.min_temp = (*env)->GetByteField(env, obj_timer, fid);
		fid = (*env)->GetFieldID(env, clazz, "max_temp", "B");
		period_timer.pt_ext_data_u.oem_101_timer_info.max_temp = (*env)->GetByteField(env, obj_timer, fid);
		break;
	}
	SAFE_DEL_LOCAL_REF(clazz);
	return cl_com_udp_period_timer_set(dev_handle, &period_timer);
}



JNIEXPORT jint JNICALL
NAME(ClCommUdpPeriodTimerDel)(JNIEnv* env, jobject this, jint dev_handle, jint id)
{
	return cl_com_udp_period_timer_del(dev_handle, id);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpRefreshTimer)(JNIEnv* env, jobject this, jint dev_handle)
{
	cl_com_udp_refresh_timer(dev_handle);
	return RS_OK;
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpRefreshDevAllInfo)(JNIEnv* env, jobject this, jint dev_handle)
{
	cl_com_udp_refresh_dev_all_info(dev_handle);
	return RS_OK;
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetPeakTime)(JNIEnv* env, jobject this, jint dev_handle, jint begin, jint last_min)
{
	return cl_com_udp_set_peak_time(dev_handle, begin, last_min);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetValleyTimer)(JNIEnv* env, jobject this, jint dev_handle, jint begin, jint last_min)
{
	return cl_com_udp_set_valley_time(dev_handle, begin, last_min);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetPeakPrice)(JNIEnv* env, jobject this, jint dev_handle, jint price)
{
	return cl_com_udp_set_peak_price(dev_handle, price);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetValleyPrice)(JNIEnv* env, jobject this, jint dev_handle, jint price)
{
	return cl_com_udp_set_valley_price(dev_handle, price);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetFlatPrice)(JNIEnv* env, jobject this, jint dev_handle, jint price)
{
	return cl_com_udp_set_flat_price(dev_handle, price);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpRefreshElecInfo)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_com_udp_refresh_elec_info(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpClearElecStatInfo)(JNIEnv* env, jobject this, jint dev_handle, jint type)
{
	return cl_com_udp_clear_elec_stat_info(dev_handle, type);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpClearDevErrInfo)(JNIEnv* env, jobject this, jint dev_handle, jint err_id)
{
	return cl_com_udp_clear_dev_err_info(dev_handle, err_id);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpRefreshDevErrInfo)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_com_udp_refresh_dev_err_info(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpPermitStmUpgrade)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_com_udp_set_permit_stm_upgrade(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetSmart)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable)
{
	return cl_sa_public_set_smart_on(dev_handle, enable);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetChildLock)(JNIEnv* env, jobject this, jint dev_handle, jbyte type)
{
	return cl_sa_public_set_child_lock(dev_handle, type);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpShortcutPower)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable,
                                      jboolean onoff, jint time)
{
	return cl_sa_public_set_shortcuts_onoff(dev_handle, enable, onoff, time);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetTemp)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff, 
						jbyte min, jbyte max)
{
	return cl_sa_public_set_temp_alarm(dev_handle,  onoff, min, max);
}


JNIEXPORT jint JNICALL
NAME(ClCommUdpSetEnvTempAdjustValue)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_com_udp_set_env_temp_ajust_value(dev_handle, value);
}

JNIEXPORT jint JNICALL
NAME(ClCommUdpSetElecAjustValue)(JNIEnv* env, jobject this, jint dev_handle, jshort value)
{
	return cl_com_udp_set_elec_ajust_value(dev_handle, value);
}


/*  亿林温控器 */  
JNIEXPORT jint JNICALL
NAME(ClCtrlTmcYLOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_on)
{
	return cl_yl_thermostat_ctrl_onoff(dev_handle,!!is_on);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcYLMode)(JNIEnv* env, jobject this, jint dev_handle, jint work_mode)
{
	return cl_yl_thermostat_ctrl_work_mode(dev_handle,work_mode);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcYLGear)(JNIEnv* env, jobject this, jint dev_handle, jint work_mode, jint gear)
{
	return cl_yl_thermostat_ctrl_gear(dev_handle,work_mode,gear);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcYLTemp)(JNIEnv* env, jobject this, jint dev_handle, jint work_mode, jint temp)
{
	return cl_yl_thermostat_ctrl_temp(dev_handle,work_mode,temp);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcYLScene)(JNIEnv* env, jobject this, jint dev_handle, jint work_mode, jint scene)
{
	return cl_yl_thermostat_ctrl_scene(dev_handle,work_mode,scene);
}


JNIEXPORT jint JNICALL
NAME(ClCtrlTmcYLTempparams)(JNIEnv* env, jobject this, jint dev_handle, jint work_mode, jint scene_count, jobjectArray scene_params)
{

	ts_scene_t scenes_array[YL_TS_MODE_MAX * YL_TS_FUNC_MODE_MAX];
	
	jclass clazz;
	jfieldID fid;
	jobject obj_scene;
	int i;
	int val;
	int ret = RS_OK;


	clazz = (*env)->FindClass(env, CLASS_SET_SCENE);

	for(i = 0; i < scene_count; i++)
	{
	    obj_scene = (jobject)(*env)->GetObjectArrayElement(env, scene_params, i);
		fid = (*env)->GetFieldID(env, clazz, "set_tmp", "I");
		val =  (*env)->GetIntField(env, obj_scene, fid);
		scenes_array[i].set_tmp = val & 0xff;

		fid = (*env)->GetFieldID(env, clazz, "gear", "I");
		scenes_array[i].gear = (*env)->GetIntField(env, obj_scene, fid);

		//LOGD("scenes_array pos [%d] = [%u]\n",i,scenes_array[i].set_tmp);
	}

	ret = cl_yl_thermostat_set_temp_param(dev_handle, work_mode, scene_count, scenes_array);
	 //LOGD("Call cl_yl_thermostat_set_temp_param ret = %d\n",ret);

	(*env)->DeleteLocalRef(env, clazz);
	return ret;

}

/*  杰能宝温控器 */  
JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_on)
{
	return cl_jnb_th_ctrl_onoff(dev_handle,!!is_on);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBMode)(JNIEnv* env, jobject this, jint dev_handle, jint mode)
{
	return cl_jnb_th_ctrl_mode(dev_handle,mode);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBTemp)(JNIEnv* env, jobject this, jint dev_handle, jint temp)
{
	return cl_jnb_th_ctrl_temp(dev_handle,temp);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBScheduler)(JNIEnv* env, jobject this, jint dev_handle, jintArray schedluer)
{
	u_int32_t *schedluer_array = NULL;
	int ret;

	if (schedluer != NULL)
		schedluer_array = (*env)->GetIntArrayElements(env, schedluer, false);

    ret = cl_jnb_th_set_schedluer(dev_handle, schedluer_array);

    if (schedluer_array != NULL)
    	(*env)->ReleaseIntArrayElements(env, schedluer,schedluer_array,0);

    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBParam)(JNIEnv* env, jobject this, jint dev_handle, jint mode, jint temp)
{
	return cl_jnb_th_set_temp_param(dev_handle,mode,temp);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBHolidaydays)(JNIEnv* env, jobject this, jint dev_handle, jint temp, jint hours)
{
	return cl_jnb_th_set_holiday_days(dev_handle,temp,hours);
}

JNIEXPORT jint JNICALL
NAME(ClCtrlTmcJNBHoldTime)(JNIEnv* env, jobject this, jint dev_handle,jint mode,jint hours)
{
	return cl_jnb_th_set_hold_time(dev_handle,mode,hours);
}
/***********LEDE************/

JNIEXPORT jint JNICALL
NAME(ClLEDECtrStateV2)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeState)
{
	jfieldID fid;
	jclass obj_class ;

	cl_lede_led_state_t ledeInfo = {0};

	if(ledeState == NULL || dev_handle == 0)
		return -5;

	obj_class = (*env)->FindClass(env, CLASS_SDK_LEDE_STAT);
	
	jni_assign_simple_struct(env, ledeState, obj_class, 
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeInfo, R),WRAP_QUOTE(r),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeInfo, G),WRAP_QUOTE(g),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeInfo, B),WRAP_QUOTE(b),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeInfo, L),WRAP_QUOTE(l),
		                        ASSIGN_TRIPLES(byte ,&ledeInfo,cold),
		                        ASSIGN_TRIPLES(byte ,&ledeInfo,mod_id),
		                        ASSIGN_TRIPLES(byte ,&ledeInfo,action),
		                        JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(obj_class);
	
	return cl_lede_ctrl_stat(dev_handle, &ledeInfo);
}

JNIEXPORT jint JNICALL
NAME(ledeSetTimer)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeTimer)
{
	jfieldID fid;
	jclass obj_class ;

	cl_lede_led_timer_t ledeTimerInfo = {0};

	if(ledeTimer == NULL || dev_handle == 0)
		return -5;

	obj_class = (*env)->FindClass(env, CLASS_SDK_LEDE_TIMER);

	jni_assign_simple_struct(env, ledeTimer, obj_class,
		                        ASSIGN_TRIPLES(byte, &ledeTimerInfo, id),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo, flags),WRAP_QUOTE(enable),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo, week_loop),WRAP_QUOTE(week),
		                        ASSIGN_TRIPLES(byte, &ledeTimerInfo, hour),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo, min),WRAP_QUOTE(minute),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo.config, R),WRAP_QUOTE(r),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo.config, G),WRAP_QUOTE(g),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo.config, B),WRAP_QUOTE(b),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo.config, L),WRAP_QUOTE(l),
		                        ASSIGN_TRIPLES(byte, &ledeTimerInfo.config, cold),
		                        WRAP_QUOTE(byte),GET_MEMBER_PTR(&ledeTimerInfo.config, power),WRAP_QUOTE(onoff),
		                        ASSIGN_TRIPLES(byte, &ledeTimerInfo.config, mod_id),
		                        ASSIGN_TRIPLES(byte, &ledeTimerInfo.config, action),
		                        JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(obj_class);
	return cl_lede_ctrl_timer(dev_handle, &ledeTimerInfo);
}

JNIEXPORT jint JNICALL
NAME(ClLEDECtrState)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeState)
{
	jfieldID fid;
	jclass obj_class ;

	cl_lede_led_state_t lamp_info = {0};

	if(ledeState == NULL || dev_handle == 0)
		return -5;

	obj_class = (*env)->FindClass(env, CLASS_LEDE_LAMP_INFO);
	assert(obj_class!=NULL);
	
	fid = (*env)->GetFieldID(env, obj_class, "r", "I");
	assert(fid!=NULL);
	lamp_info.R= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "g", "I");
	assert(fid!=NULL);
	lamp_info.G= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "b", "I");
	assert(fid!=NULL);
	lamp_info.B= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "l", "I");
	assert(fid!=NULL);
	lamp_info.L= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "cold", "I");
	assert(fid!=NULL);
	lamp_info.cold= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "modeId", "I");
	assert(fid!=NULL);
	lamp_info.mod_id= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "action", "I");
	assert(fid!=NULL);
	lamp_info.action= (*env)->GetIntField(env, ledeState, fid);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	assert(fid!=NULL);
	lamp_info.power= (*env)->GetBooleanField(env, ledeState, fid);

	SAFE_DEL_LOCAL_REF(obj_class);
	
	return cl_lede_ctrl_stat(dev_handle, &lamp_info);
}

JNIEXPORT jint JNICALL
NAME(ClLEDECtrTimer)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeTimer)
{
	jfieldID fid;
	jclass obj_class ;

	cl_lede_led_timer_t timer_info = {0};
	
	if(ledeTimer == NULL || dev_handle == 0)
		return -5;

	obj_class = (*env)->FindClass(env, CLASS_LEDE_LAMP_TIMER);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "r", "I");
	assert(fid!=NULL);
	timer_info.config.R= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "g", "I");
	assert(fid!=NULL);
	timer_info.config.G= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "b", "I");
	assert(fid!=NULL);
	timer_info.config.B= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "l", "I");
	assert(fid!=NULL);
	timer_info.config.L= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "cold", "I");
	assert(fid!=NULL);
	timer_info.config.cold= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "modeId", "I");
	assert(fid!=NULL);
	timer_info.config.mod_id= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "action", "I");
	assert(fid!=NULL);
	timer_info.config.action= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	assert(fid!=NULL);
	timer_info.config.power = (*env)->GetBooleanField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "hour", "I");
	assert(fid!=NULL);
	timer_info.hour = (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "minute", "I");
	assert(fid!=NULL);
	timer_info.min = (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "id", "I");
	assert(fid!=NULL);
	timer_info.id= (*env)->GetIntField(env, ledeTimer, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "I");
	assert(fid!=NULL);
	timer_info.week_loop= (*env)->GetIntField(env, ledeTimer, fid);


	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	assert(fid!=NULL);
	timer_info.flags = (*env)->GetBooleanField(env, ledeTimer, fid);

	SAFE_DEL_LOCAL_REF(obj_class);

	return cl_lede_ctrl_timer(dev_handle, &timer_info);
}

JNIEXPORT jint JNICALL
NAME(ClLEDECtrDeleteTimer)(JNIEnv* env, jobject this, jint dev_handle, jint id)
{ 
	return cl_lede_delete_timer(dev_handle, id);
}

JNIEXPORT jint JNICALL
NAME(ClLEDEOnStateConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject uconfig)
{ 
	jfieldID fid;	
	jclass obj_class ;	
	LOGE("ClLEDEOnStateConfig======");
	int ret;
	cl_lede_led_on_stat_t lamp_info = {0};
	obj_class = (*env)->FindClass(env, CLASS_LEDE_ON_STAT);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	lamp_info.enable= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "type", "B");
	lamp_info.type= (*env)->GetIntField(env, uconfig, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "r", "I");	
	lamp_info.stat.R= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "g", "I");	
	lamp_info.stat.G= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "b", "I");	
	lamp_info.stat.B= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "l", "I");	
	lamp_info.stat.L= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "cold", "I");	
	lamp_info.stat.cold= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "modeId", "I");	
	lamp_info.stat.mod_id= (*env)->GetIntField(env, uconfig, fid);
	fid = (*env)->GetFieldID(env, obj_class, "action", "I");	
	lamp_info.stat.action= (*env)->GetIntField(env, uconfig, fid);
	SAFE_DEL_LOCAL_REF(obj_class);		
	ret = cl_lede_on_state_config(dev_handle, &lamp_info);	
	return ret;
}


/*************************************************************************************************
*                               华天成
*************************************************************************************************/

JNIEXPORT jint JNICALL
NAME(ClTbCtrlStat)(JNIEnv* env, jobject this,  jint dev_handle, jobject obj_user_config)
{
	jint ret = 0;
	cl_tb_user_config_t user_config;
	jclass clazz;
	jfieldID fid;
	
	memset(&user_config, 0, sizeof(cl_tb_user_config_t));
	
	clazz = (*env)->FindClass(env, CLASS_HTCHP_USER_CONFIG);
	
	fid = (*env)->GetFieldID(env, clazz, "cid", "I");
	user_config.cid= (*env)->GetIntField(env, obj_user_config, fid);
	
	
	fid = (*env)->GetFieldID(env, clazz, "onoff", "Z");
	user_config.onoff = (*env)->GetBooleanField(env, obj_user_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "work_mode", "I");
	user_config.work_mode = (*env)->GetIntField(env, obj_user_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp", "I");
	user_config.temp = (*env)->GetIntField(env, obj_user_config, fid);

	ret = cl_tb_ctrl_stat(dev_handle, &user_config);
	
	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClTbSetWorkParam)(JNIEnv* env, jobject this,  jint dev_handle, jobject obj_work_config)
{
	jint ret = 0;
	cl_tb_work_config_t work_config;
	jclass clazz;
	jfieldID fid;

	memset(&work_config, 0, sizeof(cl_tb_work_config_t));
	
	clazz = (*env)->FindClass(env, CLASS_HTCHP_WORK_CONFIG);
	
	fid = (*env)->GetFieldID(env, clazz, "cid", "I");
	work_config.cid= (*env)->GetIntField(env, obj_work_config, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "return_cold_switch", "Z");
	work_config.return_cold_switch = (*env)->GetBooleanField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "facility_state", "I");
	work_config.facility_state = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "sysfunc", "I");
	work_config.sysfunc = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "return_diff_temp", "I");
	work_config.return_diff_temp = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "heat_defrost_circle", "I");
	work_config.heat_defrost_circle = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "start_heat_defrost_temp", "B");
	work_config.start_heat_defrost_temp = (*env)->GetByteField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "stop_heat_defrost_temp", "I");
	work_config.stop_heat_defrost_temp = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "stop_heat_defrost_time", "I");
	work_config.stop_heat_defrost_time = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "eheat_value", "B");
	work_config.eheat_value = (*env)->GetByteField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "backlight_delay_time", "I");
	work_config.backlight_delay_time = (*env)->GetIntField(env, obj_work_config, fid);

	fid = (*env)->GetFieldID(env, clazz, "fan_mode", "I");
	work_config.fan_mode = (*env)->GetIntField(env, obj_work_config, fid);

	ret = cl_tb_setting_work_param(dev_handle, &work_config);
	
	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClTbRefreshTemp)(JNIEnv* env, jobject this,  jint dev_handle)
{
	return cl_tb_refresh_temp_info(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClTbRefreshOtherInfo)(JNIEnv* env, jobject this,  jint dev_handle)
{
	return cl_tb_refresh_other_info(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClTbBindDevBarCode)(JNIEnv* env, jobject this,  jint dev_handle, jstring bar_code)
{
	u_int8_t *str_bar_code = NULL;
	int ret = RS_ERROR;
	
	if (bar_code != NULL) {
		str_bar_code = (u_int8_t *)((*env)->GetStringUTFChars(env, bar_code, NULL));
		ret = cl_tb_bind_dev_bar_code(dev_handle, str_bar_code);
		(*env)->ReleaseStringUTFChars(env, bar_code, str_bar_code);
	}
	
	return ret;
}

//艾美特
JNIEXPORT jint JNICALL
NAME(ClAmtCtrlOnoff)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_on)
{
	return cl_amt_ctrl_on_off(dev_handle, is_on);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlMode)(JNIEnv* env, jobject this,  jint dev_handle, jbyte mode)
{
	return cl_amt_ctrl_mode(dev_handle, mode);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlGear)(JNIEnv* env, jobject this,  jint dev_handle, jbyte mode, jbyte gear)
{
	return cl_amt_ctrl_gear(dev_handle, mode, gear);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlShark)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_shake)
{
	return cl_amt_ctrl_shake(dev_handle, is_shake);
}


JNIEXPORT jint JNICALL
NAME(ClAmtCtrlScreenLight)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_on)
{
	return cl_amt_ctrl_screen_light(dev_handle, is_on);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlAnion)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_on)
{
	return cl_amt_ctrl_anion(dev_handle, is_on);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlPlasma)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_on)
{
	return cl_amt_ctrl_plasma(dev_handle, is_on);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlCtrlSmartParams)(JNIEnv* env, jobject this,  jint dev_handle, jbyte temp , jbyte gear)
{
	return cl_amt_config_smart_param(dev_handle, temp, gear);
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlConfigUserMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte time_interval, jbyte gear_num, jbyteArray gears)
{
	u_int8_t *gears_array = NULL;
	int ret;

	if (gears != NULL)
		gears_array = (*env)->GetByteArrayElements(env,gears,false);


	ret = cl_amt_config_fan_user_mode(dev_handle,time_interval,gear_num,gears_array);

    if (gears_array != NULL)
    	(*env)-> ReleaseByteArrayElements(env, gears,gears_array,0);

    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClAmtCtrlOnoffTimer)(JNIEnv* env, jobject this,  jint dev_handle, jboolean onoff, jint time)
{
	return cl_amt_ctrl_dev_onoff_timer(dev_handle,onoff,time);
}


/************前锋***********/

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlOnoff)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_on)
{
	return cl_chiffo_ctrl_on_off(dev_handle, is_on);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlWaterModeTemp)(JNIEnv* env, jobject this,  jint dev_handle, jint temp)
{
	return cl_chiffo_setting_water_mode_temp(dev_handle, temp);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlHeaterModeTemp)(JNIEnv* env, jobject this,  jint dev_handle, jint temp)
{
	return cl_chiffo_setting_heater_mode_temp(dev_handle, temp);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlMode)(JNIEnv* env, jobject this,  jint dev_handle, jboolean water_on_off, jboolean heater_on_off)
{
	return cl_chiffo_setting_mode(dev_handle, water_on_off, heater_on_off);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlDecWaterModeTemper)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_add)
{
	return cl_chiffo_add_dec_water_mode_temp(dev_handle, is_add);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlDecHeaterModeTemper)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_add)
{
	return cl_chiffo_add_dec_heater_mode_temp(dev_handle, is_add);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoRefreshTimerByDay)(JNIEnv* env, jobject this,  jint dev_handle, jint day_index)
{
	return cl_chiffo_refresh_timer_by_day(dev_handle, day_index);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlTimer)(JNIEnv* env, jobject this, jint dev_handle, jobjectArray oneday_timer, jint day_index)
{
	cl_chiffo_one_day_timer_t ot = {0};
	jclass clazz;
	jfieldID fid;
	jobject obj_day_section;
	int i;
	int ret = RS_OK;

	clazz = (*env)->FindClass(env, CLASS_CHIFFO_SECTION_TIMER);

	for(i = 0; i < CHIFFO_TIMER_SECTION_PER_DAY; i++)
	{
	    obj_day_section = (jobject)(*env)->GetObjectArrayElement(env, oneday_timer, i);

		fid = (*env)->GetFieldID(env, clazz, "is_enable", "Z");
		ot.items[i].is_enable = (*env)->GetBooleanField(env, obj_day_section, fid);

		fid = (*env)->GetFieldID(env, clazz, "temp", "I");
		ot.items[i].temp = (*env)->GetIntField(env, obj_day_section, fid);

	}
	
	ret = cl_chiffo_setting_timer(dev_handle, &ot, day_index);

	SAFE_DEL_LOCAL_REF(clazz);
	return ret;
}

/************千帕茶盘o***********/
JNIEXPORT jint JNICALL
NAME(ClQpcpCtrlonoff)(JNIEnv* env, jobject this, jint dev_handle,jboolean onoff)
{
	return cl_qpcp_ctrl_on(dev_handle,onoff);
}

JNIEXPORT jint JNICALL
NAME(ClQpcpAddWater)(JNIEnv* env, jobject this, jint dev_handle,jint action,jint time)
{
	return cl_qpcp_add_water(dev_handle,action,time);
}

static void get_qp_pot_ext_timer_info(JNIEnv* env,jobject timer_info, cl_qp_pot_timer_t *cl_qp_pot_timer_t_p) {
	jclass obj_class ;
	jfieldID fid;

	obj_class = (*env)->FindClass(env, CLASS_QP_PAN_PERIOD_TIMER);

	fid = (*env)->GetFieldID(env, obj_class, "cook_id", "I");
	cl_qp_pot_timer_t_p->cook_id = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "cook_time", "I");
	cl_qp_pot_timer_t_p->cook_time = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "hot_degress", "I");
	cl_qp_pot_timer_t_p->hot_degress = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "microswitch", "I");
	cl_qp_pot_timer_t_p->microswitch = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "warm_temp", "I");
	cl_qp_pot_timer_t_p->warm_temp = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "cooking_mode", "I");
	cl_qp_pot_timer_t_p->cooking_mode= (*env)->GetIntField(env, timer_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
}


static void get_808_ext_timer_info(JNIEnv* env,jobject timer_info, cl_808_timer_ext_t *cl_808_timer_ext_t_p) {
	jclass obj_class ;
	jfieldID fid;

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_PERIOD_TIMER);

	fid = (*env)->GetFieldID(env, obj_class, "onOff", "Z");
	cl_808_timer_ext_t_p->onOff = (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "mode", "B");
	cl_808_timer_ext_t_p->mode = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "temp", "B");
	cl_808_timer_ext_t_p->temp = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "fan_speed", "B");
	cl_808_timer_ext_t_p->fan_speed = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "fan_dir", "B");
	cl_808_timer_ext_t_p->fan_dir = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "key_id", "B");
	cl_808_timer_ext_t_p->key_id = (*env)->GetByteField(env, timer_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
}

static void get_101_oem_ext_timer_info(JNIEnv* env,jobject timer_info, cl_101_oem_timer_t *cl_101_oem_timer_ext_t_p) {
	jclass obj_class ;
	jfieldID fid;

	obj_class = (*env)->FindClass(env, CLASS_UDP_PERIOD_TIMER);

	fid = (*env)->GetFieldID(env, obj_class, "min_temp", "B");
	cl_101_oem_timer_ext_t_p->min_temp = (*env)->GetByteField(env, timer_info, fid);
	fid = (*env)->GetFieldID(env, obj_class, "max_temp", "B");
	cl_101_oem_timer_ext_t_p->max_temp = (*env)->GetByteField(env, timer_info, fid);
	(*env)->DeleteLocalRef(env, obj_class);
}

static void _get_scene_param_by_object(JNIEnv* env,jobject param, cl_qpcp_scp_t* scp)
{
	jclass clazz;
	jfieldID fid;

	if(param == NULL || !scp){
		return;
	}
	
	clazz = (*env)->FindClass(env, CLASS_QP_TEA_SCENE_PARAM);

	fid = (*env)->GetFieldID(env, clazz, "action", "I");
	scp->action = (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "temp", "I");
	scp->temp= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "thermal_temp", "I");
	scp->thermal_temp= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "thermal_time", "I");
	scp->thermal_time= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "time", "I");
	scp->time= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "power", "I");
	scp->power= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "water_time", "I");
	scp->water_time= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "index", "I");
	scp->index= (*env)->GetIntField(env, param, fid);
	
	(*env)->DeleteLocalRef(env, clazz);

}

JNIEXPORT jint JNICALL
NAME(ClQpcpHandleCtrl)(JNIEnv* env, jobject this, jint dev_handle,jobject handle_info_obj)
{
	jint ret = 0;
	cl_qpcp_scp_t scp = {0};

	_get_scene_param_by_object(env,handle_info_obj,&scp);

	ret = cl_qpcp_handle_ctrl(dev_handle,&scp);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClQpcpDelScene)(JNIEnv* env, jobject this, jint dev_handle,jint scene_id)
{
	return cl_qpcp_scene_del(dev_handle,scene_id);
}

JNIEXPORT jint JNICALL
NAME(ClQpcpSceneModify)(JNIEnv* env, jobject this, jint dev_handle,jobject scene)
{
	cl_qpcp_sp_t qs = {0};
	jclass clazz;
	jfieldID fid;
	jobject obj_str,obj_t;
	const char *str;

	
	clazz = (*env)->FindClass(env, CLASS_QP_SCENE_ITEM);
	
	fid = (*env)->GetFieldID(env, clazz, "scene_id", "I");
	qs.scene_id = (*env)->GetIntField(env, scene, fid);

	fid = (*env)->GetFieldID(env, clazz, "create_time", "I");
	qs.create_time = (*env)->GetIntField(env, scene, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, scene, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(qs.name, str, sizeof(qs.name) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }

	fid = (*env)->GetFieldID(env, clazz, "com_param", "L"CLASS_QP_TEA_SCENE_PARAM";");
	obj_t = (*env)->GetObjectField(env, scene, fid);
	
	_get_scene_param_by_object(env,obj_t,&qs.com_param);

	
	(*env)->DeleteLocalRef(env, clazz);

	return cl_qpcp_scene_modify(dev_handle,&qs);
}

JNIEXPORT jint JNICALL
NAME(ClQpcpSceneExecute)(JNIEnv* env, jobject this, jint dev_handle,jint scene_id,jobject scene_param)
{
	cl_qpcp_scp_t scp = {0};
	
	_get_scene_param_by_object(env,scene_param,&scp);	

	return cl_qpcp_scene_execute(dev_handle,scene_id,&scp);
}

//千帕锅

static void _get_pot_scene_param_by_object(JNIEnv* env,jobject param, cl_qp_pot_scene_param_t* scp)
{
	jclass clazz;
	jfieldID fid;

	if(param == NULL || !scp){
		return;
	}

	clazz = (*env)->FindClass(env, CLASS_QP_PAN_SCENE_PARAM);

	fid = (*env)->GetFieldID(env, clazz, "s_id", "I");
	scp->s_id = (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "cook_time", "I");
	scp->cook_time= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "hot_degress", "I");
	scp->hot_degress= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "microswitch", "I");
	scp->microswitch= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "warm_temp", "I");
	scp->warm_temp= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "cooking_mode", "I");
	scp->cooking_mode= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "set_action", "I");
	scp->set_action= (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "rice_degress", "I");
	scp->rice_degress= (*env)->GetIntField(env, param, fid);

	(*env)->DeleteLocalRef(env, clazz);

}


JNIEXPORT jint JNICALL
NAME(ClQpcpPotCtrl)(JNIEnv* env, jobject this, jint dev_handle,jobject handle_info_obj)
{
	jint ret = 0;
	cl_qp_pot_scene_param_t scp = {0};

	_get_pot_scene_param_by_object(env,handle_info_obj,&scp);

	ret = cl_qp_pot_ctrl(dev_handle,&scp);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClQpcpPotExecScene)(JNIEnv* env, jobject this, jint dev_handle,jobject handle_info_obj)
{
	jint ret = 0;
	cl_qp_pot_scene_param_t scp = {0};

	_get_pot_scene_param_by_object(env,handle_info_obj,&scp);

	ret = cl_qp_pot_exec_scene(dev_handle,&scp);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClQpcpPotDelScene)(JNIEnv* env, jobject this, jint dev_handle,jint scene_id)
{
	return cl_qp_pot_del_scene(dev_handle,scene_id);
}


JNIEXPORT jint JNICALL
NAME(ClQpcpPotModifyScene)(JNIEnv* env, jobject this, jint dev_handle,jobject scene)
{
	cl_qp_pot_scene_t qs = {0};
	jclass clazz;
	jfieldID fid;
	jobject obj_str,obj_t;
	const char *str;

	clazz = (*env)->FindClass(env, CLASS_QP_PAN_SCENE_ITEM);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, scene, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(qs.name, str, sizeof(qs.name) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }

	 fid = (*env)->GetFieldID(env, clazz, "create_time", "I");
	qs.create_time = (*env)->GetIntField(env, scene, fid);

	fid = (*env)->GetFieldID(env, clazz, "s_info", "L"CLASS_QP_PAN_SCENE_PARAM";");
	obj_t = (*env)->GetObjectField(env, scene, fid);
	
	_get_pot_scene_param_by_object(env,obj_t,&qs.s_info);
	

	(*env)->DeleteLocalRef(env, clazz);

	return cl_qp_pot_modify_scene(dev_handle,&qs);
}

static void assign_hx_pot_timer(JNIEnv* env, jobject obj_timer, cl_hx_ysh_timer_t *c_timer) {
	jclass class_timer = NULL;
	jfieldID fid = NULL;

	class_timer = (*env)->FindClass(env, CLASS_UDP_PERIOD_TIMER);

	fid = (*env)->GetFieldID(env, class_timer, "hx_scene_id", "S");
	c_timer->scene_id = (*env)->GetShortField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, class_timer, "temp", "B");
	c_timer->temp = (*env)->GetByteField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, class_timer, "work_time", "B");
	c_timer->work_time = (*env)->GetByteField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, class_timer, "power", "B");
	c_timer->power = (*env)->GetByteField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, class_timer, "keep_temp", "B");
	c_timer->keep_temp = (*env)->GetByteField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, class_timer, "keep_time", "B");
	c_timer->keep_time = (*env)->GetByteField(env, obj_timer, fid);

	(*env)->DeleteLocalRef(env, class_timer);
}


JNIEXPORT jint JNICALL
NAME(ClCommUdpExtPeriodTimerSet)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_timer,jobject ext_obj)
{
	cl_period_timer_t period_timer;
	jclass clazz;
	jclass class_ext = NULL;
	jfieldID fid;
	cl_qpcp_scp_t scp = {0};
	cl_qp_pot_timer_t cl_qp_pot_timer_t_p = {0};
	cl_qp_pbj_timer_t qp_pbj_timer_t_p = {0}; //千帕锅定时器
	cl_808_timer_ext_t cl_808_timer_ext_t_p = {0};
	cl_101_oem_timer_t cl_101_oem_timer_ext_t_p = {0};
	cl_hx_ysh_timer_t cl_hx_pot_timer = {0};
	u_int16_t len = 0;
	void* param = NULL;

	memset(&period_timer, 0, sizeof(cl_period_timer_t));
	
	clazz = (*env)->FindClass(env, CLASS_UDP_PERIOD_TIMER);

	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	period_timer.id= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	period_timer.hour = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	period_timer.minute = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	period_timer.week= (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	period_timer.enable = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "onoff", "Z");
	period_timer.onoff = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "duration", "I");
	period_timer.duration = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "ext_data_type", "I");
	period_timer.ext_data_type= (*env)->GetIntField(env, obj_timer, fid);

	LOGD("xxxddd timer ext type = %hu\n", period_timer.ext_data_type);

	if(period_timer.ext_data_type == PT_EXT_DT_QPCP){
		fid = (*env)->GetFieldID(env, clazz, "scene_id", "I");
		period_timer.pt_ext_data_u.qp_time_info.id = (*env)->GetIntField(env, obj_timer, fid);
		_get_scene_param_by_object(env,ext_obj,&scp);
		param= (void*)&scp;
		len = sizeof(scp);
	} else if (period_timer.ext_data_type == PT_EXT_DT_QP_POT){
		get_qp_pot_ext_timer_info(env, obj_timer, &cl_qp_pot_timer_t_p);
		memcpy(&period_timer.pt_ext_data_u.qp_pot_timer_info, &cl_qp_pot_timer_t_p, sizeof(cl_qp_pot_timer_t));
		len = sizeof(cl_qp_pot_timer_t);
		param= (void*)&cl_qp_pot_timer_t_p;
	} else if(period_timer.ext_data_type == PT_EXT_DT_QP_PBJ) {
		fid = (*env)->GetFieldID(env, clazz, "scene_id", "I");
		period_timer.pt_ext_data_u.qp_pbj_timer_info.scene_id = (*env)->GetIntField(env, obj_timer, fid);
		param= (void*)&qp_pbj_timer_t_p;
		len = sizeof(qp_pbj_timer_t_p);
	}else if (period_timer.ext_data_type == PT_EXT_DT_808) {
		get_808_ext_timer_info(env, obj_timer, &cl_808_timer_ext_t_p);
		memcpy(&period_timer.pt_ext_data_u.air_timer_info, &cl_808_timer_ext_t_p, sizeof(cl_808_timer_ext_t));
		len = sizeof(cl_808_timer_ext_t);
		param= (void*)&cl_808_timer_ext_t_p;
	} else if (period_timer.ext_data_type == PT_EXT_DT_101_OEM) {
		get_101_oem_ext_timer_info(env, obj_timer, &cl_101_oem_timer_ext_t_p);
		memcpy(&period_timer.pt_ext_data_u.oem_101_timer_info, &cl_101_oem_timer_ext_t_p, sizeof(cl_101_oem_timer_t));
		len = sizeof(cl_101_oem_timer_t);
		param= (void*)&cl_101_oem_timer_ext_t_p;
	} else if (period_timer.ext_data_type == PT_EXT_DT_HX_YSH) {	
		assign_hx_pot_timer(env, obj_timer, &cl_hx_pot_timer);
		memcpy(&period_timer.pt_ext_data_u.hx_ysh_timer_info, &cl_hx_pot_timer, sizeof (cl_hx_ysh_timer_t));
		len = sizeof(cl_hx_ysh_timer_t);
		param= (void*)&cl_hx_pot_timer;
	} /*else if (period_timer.ext_data_type != PT_EXT_DT_HTC_POOL) {
		return RS_INVALID_PARAM;
	}*/

	SAFE_DEL_LOCAL_REF(clazz);
	return cl_com_udp_ext_period_timer_set(dev_handle, &period_timer,param,len);
}


JNIEXPORT jint JNICALL
NAME(ClQpcpResetFault)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_qpcp_reset_fault(dev_handle);
}

/******************千帕破壁机****************/
JNIEXPORT jint JNICALL
NAME(ClQPPbjCtrlOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean on)
{
	return cl_qp_pbj_ctrl_onoff(dev_handle, on);
}

JNIEXPORT jint JNICALL
NAME(ClQPPbjCtrlScene)(JNIEnv* env, jobject this, jint dev_handle, jint action, jint scene_id)
{
	return cl_qp_pbj_ctrl_scene(dev_handle, action, scene_id);
}

JNIEXPORT jint JNICALL
NAME(ClQPPbjModifyScene)(JNIEnv* env, jobject this, jint dev_handle, jobject scene)
{
	cl_qp_pbj_scene_t qs_scene = {0};
	cl_qp_pbj_action_t *qs_ation = NULL;
	cl_qp_pbj_mix_info_t *qs_mix = NULL;
    cl_qp_pbj_hot_info_t *qs_hot = NULL;
	jclass clazz,class_action, class_mix, class_hot;
	jfieldID fid;
	jobject obj_str,obj_action, obj_mix, obj_hot;
	jobject actionArray;
	const char *str;
	int i;
	int ret = RS_OK;

	clazz = (*env)->FindClass(env, CLASS_QP_POBIJI_SCENE);
	class_action = (*env)->FindClass(env, CLASS_QP_POBIJI_ACTION);
	class_mix = (*env)->FindClass(env, CLASS_QP_POBIJI_MIX_INFO);
	class_hot= (*env)->FindClass(env, CLASS_QP_POBIJI_HOT_INFO);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	obj_str = (*env)->GetObjectField(env, scene, fid);
	if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
		strncpy(qs_scene.name, str, sizeof(qs_scene.name) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	}

	fid = (*env)->GetFieldID(env, clazz, "scene_id", "I");
	qs_scene.scene_id = (*env)->GetIntField(env, scene, fid);

	fid = (*env)->GetFieldID(env, clazz, "step_count", "I");
	qs_scene.step_count = (*env)->GetIntField(env, scene, fid);

	fid = (*env)->GetFieldID(env, clazz, "warm_time", "I");
	qs_scene.warm_time = (*env)->GetIntField(env, scene, fid);

	fid = (*env)->GetFieldID(env, clazz, "warm_temp", "I");
	qs_scene.warm_temp = (*env)->GetIntField(env, scene, fid);

	if(qs_scene.action != NULL) {
		fid = (*env)->GetFieldID(env, clazz, "action", "[L"CLASS_QP_POBIJI_ACTION";");
		actionArray = (*env)->GetObjectField(env, scene, fid);

		for(i = 0; i < qs_scene.step_count; i++){
			qs_ation = &qs_scene.action[i];
			obj_action = (jobject)(*env)->GetObjectArrayElement(env, actionArray, i);

			fid = (*env)->GetFieldID(env, class_action, "data_type", "I");
			qs_ation->data_type= (*env)->GetIntField(env, obj_action, fid);

			LOGE_FILE_LINE;
			if(qs_ation->data_type == QP_MODE_MIX) {

				qs_mix = &qs_ation->m_info;
				fid = (*env)->GetFieldID(env, class_action, "m_info", "L"CLASS_QP_POBIJI_MIX_INFO";");
				obj_mix = (*env)->GetObjectField(env, obj_action, fid);

				fid = (*env)->GetFieldID(env, class_mix, "gear", "I");
				qs_mix->gear= (*env)->GetIntField(env, obj_mix, fid);

				fid = (*env)->GetFieldID(env, class_mix, "time", "I");
				qs_mix->time= (*env)->GetIntField(env, obj_mix, fid);

				fid = (*env)->GetFieldID(env, class_mix, "freq", "I");
				qs_mix->freq= (*env)->GetIntField(env, obj_mix, fid);

			} else if(qs_ation->data_type == QP_MODE_HOT) {

				qs_hot= &qs_ation->h_info;
				fid = (*env)->GetFieldID(env, class_action, "h_info", "L"CLASS_QP_POBIJI_HOT_INFO";");
				obj_hot= (*env)->GetObjectField(env, obj_action, fid);

				fid = (*env)->GetFieldID(env, class_hot, "temp", "I");
				qs_hot->temp= (*env)->GetIntField(env, obj_hot, fid);

				fid = (*env)->GetFieldID(env, class_hot, "time", "I");
				qs_hot->time= (*env)->GetIntField(env, obj_hot, fid);
			}
		}
	}

	(*env)->DeleteLocalRef(env, clazz);
	(*env)->DeleteLocalRef(env, class_action);
	(*env)->DeleteLocalRef(env, class_mix);
	(*env)->DeleteLocalRef(env, class_hot);

	return cl_qp_pbj_modify_scene(dev_handle, &qs_scene);
}

JNIEXPORT jint JNICALL
NAME(ClQpPbjResetFault)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_qp_pbj_reset_fault(dev_handle);
}

/**************CZWK START车载悟空*************/
JNIEXPORT jint JNICALL
NAME(ClCarConfigKeepTime)(JNIEnv* env, jobject this, jint dev_handle, jbyte keep_time)
{
	return cl_car_config_keeptime(dev_handle, keep_time);
}

JNIEXPORT jint JNICALL
NAME(ClCarCtrlOn)(JNIEnv* env, jobject this, jint dev_handle, jboolean on)
{
	return cl_car_ctrl_on(dev_handle, on);
}

JNIEXPORT jint JNICALL
NAME(ClCarConfigSeach)(JNIEnv* env, jobject this, jint dev_handle, jobject search)
{
	return cl_car_config_search(dev_handle, search);
}

JNIEXPORT jint JNICALL
NAME(ClCarCtrlSeach)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_car_ctrl_search(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClCarConfigValtage)(JNIEnv *env, jobject this, jint dev_handle, jshort valtage)
{
	return cl_car_config_valtage(dev_handle, valtage);
}

JNIEXPORT jint JNICALL
NAME(ClCarCtrlValtage)(JNIEnv *env, jobject this, jint dev_handle, jboolean on)
{
	return cl_car_ctrl_valtage(dev_handle, on);
}

JNIEXPORT jint JNICALL
NAME(ClCarCtrlPowerSave)(JNIEnv *env, jobject this, jint dev_handle, jboolean on)
{
	return cl_car_ctrl_powersave(dev_handle, on);
}

void copy_car_info(JNIEnv* env, jclass class_udp_info, jobject object_udp_info, void *udp_dev) 
{
	jclass class_car_info = NULL;
	jclass class_car_alarm = NULL;
	jobject object_car_info = NULL;
	jobject object_car_alarm = NULL;
	jobjectArray object_array_alarm = NULL;
	cl_car_alarm_t *car_alarm;
	
	jfieldID fid;
	int i, alarm_num;
	cl_car_info_t *car_info = (cl_car_info_t*)udp_dev;
	if(!car_info){
		return;
	}else{
		alarm_num = car_info->alarm_num;
		car_alarm = car_info->car_alarm;
	}
	
	class_car_info = (*env)->FindClass(env, CLASS_CAR_INFO);
	if (!class_car_info) {
		goto quit;
	}
	class_car_alarm = (*env)->FindClass(env, CLASS_CAR_ALARM);
	if (!class_car_alarm) {
		goto quit;
	}
	object_car_info = (*env)->AllocObject(env, class_car_info);
	if (!object_car_info) {
		goto quit;
	}
	object_array_alarm = (*env)->NewObjectArray(env, car_info->alarm_num, class_car_alarm, NULL);
	if(!object_array_alarm){
		goto quit;
	}

	jniCopyBooleanValue(env, class_car_info, "on", object_car_info, car_info->on);
	jniCopyByteValue(env, class_car_info, "on_keep_time", object_car_info, car_info->on_keep_time);
	jniCopyByteValue(env, class_car_info, "ele_percentage", object_car_info, car_info->ele_percentage);
	jniCopyByteValue(env, class_car_info, "horn_num", object_car_info, car_info->horn_num);
	jniCopyByteValue(env, class_car_info, "horn_time", object_car_info, car_info->horn_time);
	jniCopyByteValue(env, class_car_info, "horn_interval", object_car_info, car_info->horn_interval);
	jniCopyByteValue(env, class_car_info, "light_num", object_car_info, car_info->light_num);
	jniCopyByteValue(env, class_car_info, "light_time", object_car_info, car_info->light_time);
	jniCopyByteValue(env, class_car_info, "light_interval", object_car_info, car_info->light_interval);
	jniCopyByteValue(env, class_car_info, "temp", object_car_info, car_info->temp);
	jniCopyShortValue(env, class_car_info, "valtage", object_car_info, car_info->valtage);
	jniCopyByteValue(env, class_car_info, "last_on_time", object_car_info, car_info->last_on_time);
	jniCopyShortValue(env, class_car_info, "alarm_num", object_car_info, car_info->alarm_num);
	jniCopyString(env, class_car_info, "debug_info", object_car_info, car_info->debug_info);

	if(car_alarm != NULL)
	{
		for(i=0; i<alarm_num; i++,car_alarm++)
		{
			object_car_alarm = (*env)->AllocObject(env, class_car_alarm);
			if (!object_car_alarm) 
			{
				break;
			}

			jniCopyIntValue(env, class_car_alarm, "time", object_car_alarm, car_alarm->time);
			//LOGI("test time %s",ctime(&(car_alarm->time)));
			jniCopyByteValue(env, class_car_alarm, "id", object_car_alarm, car_alarm->id);

			(*env)->SetObjectArrayElement(env, object_array_alarm, i, object_car_alarm);
			(*env)->DeleteLocalRef(env, object_car_alarm);
		}

		fid = (*env)->GetFieldID(env, class_car_info, "car_alarm", "[L" CLASS_CAR_ALARM";");
		(*env)->SetObjectField(env, object_car_info, fid, object_array_alarm);
	}
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, object_udp_info, fid, object_car_info);

	LOGI("COPY CZWK_OK");
quit:
	SAFE_DEL_LOCAL_REF(class_car_info);
	SAFE_DEL_LOCAL_REF(class_car_alarm);
	SAFE_DEL_LOCAL_REF(object_car_info);
	SAFE_DEL_LOCAL_REF(object_array_alarm);
}

/****************CZWK END车载悟空****************/

/****************沙特插座****************/
JNIEXPORT jint JNICALL
NAME(ClEoSetOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean on)
{
	return cl_eo_set_onoff(dev_handle, on);
}

JNIEXPORT jint JNICALL
NAME(ClEoSetTempRange)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable,jbyte maxTemp,jbyte minTemp)
{
	return cl_eo_set_temp_range(dev_handle, enable,maxTemp,minTemp);
}

JNIEXPORT jint JNICALL
NAME(ClEoSetThreshold)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable,jbyte maxTemp)
{
	return cl_eo_set_threshold(dev_handle, enable,maxTemp);
}

JNIEXPORT jint JNICALL
NAME(ClEoSetOffLineCloseEnable)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable)
{
	return cl_eo_set_off_line_close_enable(dev_handle, enable);
}

JNIEXPORT jint JNICALL
NAME(ClEoSetPersonDetectEnable)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable)
{
	return cl_eo_set_person_detect_enable(dev_handle, enable);
}

/**************** OEM 插座****************/

void copy_wuneng_oem_info(JNIEnv* env, jclass class_udp_info, jobject object_udp_info, void *udp_dev)
{
	jclass class_wuneng_oem = NULL;
	jobject obj_wuneng_oem = NULL;
	jfieldID fid;
	cl_eplug_oem_stat *eplug_oem = (cl_eplug_oem_stat*)udp_dev;

	if(!eplug_oem){
		return;
	}
	
	class_wuneng_oem = (*env)->FindClass(env, CLASS_WUNENG_OEM);
	obj_wuneng_oem = (*env)->AllocObject(env, class_wuneng_oem);

	jni_copy_simple_class(env, class_wuneng_oem, obj_wuneng_oem,
		                           TRIPLES(boolean, eplug_oem, onoff),
		                           TRIPLES(byte, eplug_oem, room_temp),
		                           TRIPLES(boolean, eplug_oem, range_enable),
		                           TRIPLES(byte, eplug_oem, range_max_temp),
		                           TRIPLES(byte, eplug_oem, range_min_temp),
		                           TRIPLES(boolean, eplug_oem, temp_threshold_enable),
		                           TRIPLES(byte, eplug_oem, temp_threshold_value),
		                           TRIPLES(boolean, eplug_oem, off_line_close_enable),
		                           TRIPLES(boolean, eplug_oem, is_support_person_detect),
		                           TRIPLES(boolean, eplug_oem, person_detect_enable),
		                           JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, object_udp_info, fid, obj_wuneng_oem);

	SAFE_DEL_LOCAL_REF(class_wuneng_oem);
	SAFE_DEL_LOCAL_REF(obj_wuneng_oem);
}

/****************XYWKQ START鑫源温控器****************/
/*开关机*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlOnoff)(JNIEnv* env, jobject this, jint dev_handle, jbyte onoff){
	return cl_xy_ctrl_onoff(dev_handle, onoff);
}

/*恒温模式下的温度设置*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte temp){
	return cl_xy_ctrl_temp(dev_handle, temp);
}

/*休假模式时间设置设置*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlTime)(JNIEnv* env, jobject this, jint dev_handle, jshort time){
	return cl_xy_ctrl_time(dev_handle, time);
}

/*按键锁定控制*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlLockOnoff)(JNIEnv* env, jobject this, jint dev_handle, jbyte onoff){
	return cl_xy_ctrl_lock_onoff(dev_handle, onoff);
}

/*模式设置*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode){
	return cl_xy_ctrl_mode(dev_handle, mode);
}

/*智能模式参数配置*/
JNIEXPORT jint JNICALL
NAME(ClXyConfigSmartMode)(JNIEnv* env, jobject this, jint dev_handle, jobject pst_info){
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
	
	class_xywkq_xystmode = (*env)->FindClass(env, CLASS_XYWKQ_SMART_MODE);
	class_xy_tp = (*env)->FindClass(env, CLASS_XYWKQTP);
	
	fid = (*env)->GetFieldID(env, class_xywkq_xystmode, "mode", "B");
	xy_pst_info.sub_mode.mode = (*env)->GetByteField(env, pst_info, fid);

	fid = (*env)->GetFieldID(env, class_xywkq_xystmode, "timepoint", "[L" CLASS_XYWKQTP ";");
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
NAME(ClXyCtrlAdjust)(JNIEnv* env, jobject this, jint dev_handle, jobject padjust){
	jint ret = 0;
	cl_xy_adjust_t xy_adjust;
	jclass clazz;
	jfieldID fid;
	
	memset(&xy_adjust, 0, sizeof(cl_xy_adjust_t));
	
	clazz = (*env)->FindClass(env, CLASS_XYWKQ_ADJUST);
	
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

/*外部温度设置*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlExternTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte temp){
	return cl_xy_ctrl_extern_temp(dev_handle, temp);
}

/*智能距离感应开关设置*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlSmarthomeOnoff)(JNIEnv* env, jobject this, jint dev_handle, jbyte onoff){
	return cl_xy_ctrl_smarthome_onoff(dev_handle, onoff);
}

void copy_hp_gw_info(JNIEnv* env, jclass class_gw, jobject obj_gw, cl_gw_info_t *gw_info_p)
{
	jclass class_hp_info;
	jobject obj_hp_info;
	jclass class_phone_user;
	jobject obj_phone_user;
	jclass class_led_stat;
	jobject obj_led_stat;
	jobject obj_phone_user_array;
	jfieldID fid;
	int i;
	
	class_hp_info = (*env)->FindClass(env, CLASS_RF_HP_GW_INFO);
	obj_hp_info = (*env)->AllocObject(env, class_hp_info);
	class_phone_user = (*env)->FindClass(env, CLASS_RF_HP_GW_PHONE_USER);
	obj_phone_user = (*env)->AllocObject(env, class_phone_user);
	
	class_led_stat = (*env)->FindClass(env, CLASS_RF_LIGHT_STATU);
	obj_led_stat = (*env)->AllocObject(env, class_led_stat);

	jniCopyBooleanValue(env,class_hp_info,"support_appinfo",obj_hp_info,gw_info_p->hpinfo.support_appinfo);
	jniCopyBooleanValue(env,class_hp_info,"support_sms",obj_hp_info,gw_info_p->hpinfo.support_sms);
	jniCopyByteValue(env,class_hp_info,"sms_lang",obj_hp_info,gw_info_p->hpinfo.sms_lang);

	LOGE("zzzz copy_hp_gw_info, phone size = %d.",gw_info_p->hpinfo.phone_user_num);

	//拷贝短信通知人
	if(gw_info_p->hpinfo.phone_user_num > 0){
		obj_phone_user_array = (*env)->NewObjectArray(env, gw_info_p->hpinfo.phone_user_num, class_phone_user, NULL);
		for(i = 0;i < gw_info_p->hpinfo.phone_user_num;i++) {
			obj_phone_user  =  (*env)->AllocObject(env, class_phone_user);
			jni_copy_simple_class(env, class_phone_user, obj_phone_user, 
				TRIPLES(long, &gw_info_p->hpinfo.users[i], phome_number),
				TRIPLES(String, &gw_info_p->hpinfo.users[i], name),
				JNI_VAR_ARG_END);
			(*env)->SetObjectArrayElement(env, obj_phone_user_array, i, obj_phone_user);
			SAFE_DEL_LOCAL_REF(obj_phone_user);
		}
		fid = (*env)->GetFieldID(env, class_hp_info, "users", "[L" CLASS_RF_HP_GW_PHONE_USER ";");
		(*env)->SetObjectField(env, obj_hp_info, fid, obj_phone_user_array);

		SAFE_DEL_LOCAL_REF(obj_phone_user_array);
	}

	//拷贝灯的相关信息
	cl_rf_lamp_t *rf_lamp_t = &gw_info_p->hpinfo.lamp_stat;
	jniCopyIntValue(env,class_led_stat,"r",obj_led_stat,rf_lamp_t->stat.R);
	jniCopyIntValue(env,class_led_stat,"g",obj_led_stat,rf_lamp_t->stat.G);
	jniCopyIntValue(env,class_led_stat,"b",obj_led_stat,rf_lamp_t->stat.B);
	jniCopyIntValue(env,class_led_stat,"l",obj_led_stat,rf_lamp_t->stat.L);
	jniCopyIntValue(env,class_led_stat,"cold",obj_led_stat,rf_lamp_t->stat.cold);
	jniCopyIntValue(env,class_led_stat,"modeId",obj_led_stat,rf_lamp_t->stat.mod_id);
	jniCopyIntValue(env,class_led_stat,"action",obj_led_stat,rf_lamp_t->stat.action);
	jniCopyIntValue(env,class_led_stat,"lamp_type",obj_led_stat,rf_lamp_t->lamp_type);
	jniCopyIntValue(env,class_led_stat,"ctrl_mode",obj_led_stat,rf_lamp_t->stat.ctrl_mode);
	jniCopyIntValue(env,class_led_stat,"value",obj_led_stat,rf_lamp_t->stat.value);
	jniCopyBooleanValue(env,class_led_stat,"is_support_color_temp",obj_led_stat,rf_lamp_t->is_support_color_temp);
	jniCopyBooleanValue(env,class_led_stat,"is_support_rgb",obj_led_stat,rf_lamp_t->is_support_rgb);
	jniCopyBooleanValue(env,class_led_stat,"power",obj_led_stat,rf_lamp_t->stat.power);
	jniCopyByteValue(env,class_led_stat,"flag",obj_led_stat,rf_lamp_t->stat.flag);
	
	fid = (*env)->GetFieldID(env, class_hp_info, "lamp_stat", "L"CLASS_RF_LIGHT_STATU";");
	(*env)->SetObjectField(env, obj_hp_info, fid, obj_led_stat);

	fid = (*env)->GetFieldID(env,class_gw,"hpinfo","L"CLASS_RF_HP_GW_INFO";");
	(*env)->SetObjectField(env, obj_gw, fid, obj_hp_info);
	
	SAFE_DEL_LOCAL_REF(class_hp_info);
	SAFE_DEL_LOCAL_REF(class_led_stat);
	
}

void copy_rf_group_info(JNIEnv* env, jclass class_gw, jobject obj_gw, cl_gw_info_t *gw_info_p){
	jclass class_group;
	
	jobject obj_group;
	jobject obj_arr_group;
	jobject obj_long_arr;
	jfieldID fid;
	
	int i,j;
	
	cl_dev_group_t *group_info_p = gw_info_p->dev_group;

	class_group = (*env)->FindClass(env, CLASS_RF_DEV_GROUP_STATU);
	obj_arr_group = (*env)->NewObjectArray(env, gw_info_p->dev_group_cnt, class_group, NULL);

	for (i = 0; i < gw_info_p->dev_group_cnt; i++,group_info_p++) {
		obj_group = (*env)->AllocObject(env, class_group);

		jni_copy_simple_class(env, class_group, obj_group,
								TRIPLES(byte, group_info_p, group_id),		
								TRIPLES(byte, group_info_p, group_type),
								TRIPLES(byte, group_info_p, dev_cnt),
								TRIPLES(boolean, group_info_p, query_dev),
								TRIPLES(byte, group_info_p, reserved),
								JNI_VAR_ARG_END);
		
		if (group_info_p->dev_cnt > 0) {
			obj_long_arr = (*env)->NewLongArray(env, group_info_p->dev_cnt);
			(*env)->SetLongArrayRegion(env, obj_long_arr,0, group_info_p->dev_cnt, (jlong*)group_info_p->dev_sn);

			fid = (*env)->GetFieldID(env, class_group, "dev_sn", "[J");
			(*env)->SetObjectField(env, obj_group, fid, obj_long_arr);
			SAFE_DEL_LOCAL_REF(obj_long_arr);
		}
		
		jniCopyString(env, class_group, "name", obj_group, group_info_p->name);
		(*env)->SetObjectArrayElement(env, obj_arr_group, i, obj_group);
		
		SAFE_DEL_LOCAL_REF(obj_group);
	}

	fid = (*env)->GetFieldID(env, class_gw, "groupInfos", "[L" CLASS_RF_DEV_GROUP_STATU ";");
	(*env)->SetObjectField(env, obj_gw, fid, obj_arr_group);
	
	SAFE_DEL_LOCAL_REF(class_group);
	SAFE_DEL_LOCAL_REF(obj_arr_group);
	
}

void copy_rf_rmt_ctrl_info(JNIEnv* env, jclass class_gw, jobject obj_gw, cl_gw_info_t *gw_info_p){
	jclass class_rmt_ctrl_info;
	jclass class_rmt_ctrl_key_info;

	jobject obj_arr_rmt_ctrl_info;
	jobject obj_arr_rmt_ctrl_keys;
	
	jobject obj_rmt_ctrl_info;
	jobject obj_rmt_ctrl_key;
	
	jfieldID fid;

	cl_lamp_remote_info_t * lr_info = gw_info_p->lr_info;
	cl_lamp_remote_key_info * key;
	
	int i, j;

	class_rmt_ctrl_info = (*env)->FindClass(env, CLASS_RF_RMT_CTRL_INFO);
	class_rmt_ctrl_key_info = (*env)->FindClass(env, CLASS_RF_RMT_CTRL_KEY_INFO);
	
	obj_arr_rmt_ctrl_info = (*env)->NewObjectArray(env, gw_info_p->lamp_remote_cnt, class_rmt_ctrl_info, NULL);

	for (i = 0; i < gw_info_p->lamp_remote_cnt; i++, lr_info++) {
		obj_rmt_ctrl_info = (*env)->AllocObject(env, class_rmt_ctrl_info);
		jniCopyIntValue(env, class_rmt_ctrl_info, "handle", obj_rmt_ctrl_info, lr_info->handle);
		jniCopyIntValue(env, class_rmt_ctrl_info, "r_id", obj_rmt_ctrl_info, lr_info->r_id);
		jniCopyIntValue(env, class_rmt_ctrl_info, "lamp_type", obj_rmt_ctrl_info, lr_info->lamp_type);

		obj_arr_rmt_ctrl_keys = (*env)->NewObjectArray(env, 4, class_rmt_ctrl_key_info, NULL);
		key = lr_info->key;
		for (j = 0; j < 4; j++,key++) {
			obj_rmt_ctrl_key = (*env)->AllocObject(env, class_rmt_ctrl_key_info);
			jniCopyIntValue(env, class_rmt_ctrl_key_info, "slave_count", obj_rmt_ctrl_key, key->slave_count);
			jniCopyIntValue(env, class_rmt_ctrl_key_info, "real_count", obj_rmt_ctrl_key, key->real_count);
			jniCopyIntValue(env, class_rmt_ctrl_key_info, "lamp_type", obj_rmt_ctrl_key, key->lamp_type);
			if (key->real_count > 0) {
				jniCopyIntArray(env, class_rmt_ctrl_key_info, "slave_handle", obj_rmt_ctrl_key, key->slave_handle, key->real_count);
			}

			(*env)->SetObjectArrayElement(env, obj_arr_rmt_ctrl_keys, j, obj_rmt_ctrl_key);

			SAFE_DEL_LOCAL_REF(obj_rmt_ctrl_key);
		}

		fid = (*env)->GetFieldID(env, class_rmt_ctrl_info, "key", "[L" CLASS_RF_RMT_CTRL_KEY_INFO ";");
		(*env)->SetObjectField(env, obj_rmt_ctrl_info, fid, obj_arr_rmt_ctrl_keys);
		
		(*env)->SetObjectArrayElement(env, obj_arr_rmt_ctrl_info, i, obj_rmt_ctrl_info);

		SAFE_DEL_LOCAL_REF(obj_arr_rmt_ctrl_keys);
		SAFE_DEL_LOCAL_REF(obj_rmt_ctrl_info);
	}

	fid = (*env)->GetFieldID(env, class_gw, "rmtCtrlInfos", "[L" CLASS_RF_RMT_CTRL_INFO ";");
	(*env)->SetObjectField(env, obj_gw, fid, obj_arr_rmt_ctrl_info);

	SAFE_DEL_LOCAL_REF(obj_arr_rmt_ctrl_info);
	SAFE_DEL_LOCAL_REF(class_rmt_ctrl_info);
	SAFE_DEL_LOCAL_REF(class_rmt_ctrl_key_info);
}

void copy_rf_gw_info(JNIEnv* env, jclass class_udp_info, jobject object_udp_info, void *udp_dev){
	jclass class_gw;
	jobject obj_gw;
	jfieldID fid;
	
	cl_gw_info_t *gw_info_p;

	gw_info_p = (cl_gw_info_t *)udp_dev;

	class_gw = (*env)->FindClass(env, CLASS_RF_DEV_GW_STATU);
	obj_gw = (*env)->AllocObject(env, class_gw);

	if (gw_info_p->dev_group_cnt > 0) {
		copy_rf_group_info(env, class_gw, obj_gw,gw_info_p);
	}

	if (gw_info_p->lamp_remote_cnt > 0) {
		copy_rf_rmt_ctrl_info(env, class_gw, obj_gw,gw_info_p);
	}
	
	jniCopyByteArray(env, class_gw, "upgrade_status", obj_gw, gw_info_p->upgrade_status, D_T_MAX);
	jniCopyStringArray(env, class_gw, "upgrade_url", obj_gw, gw_info_p->upgrade_url, D_T_MAX);

	jniCopyByteValue(env, class_gw, "commpat", obj_gw, gw_info_p->commpat);
	jniCopyByteValue(env, class_gw, "channel", obj_gw, gw_info_p->channel);
	jniCopyBooleanValue(env, class_gw, "is_upgrade", obj_gw, gw_info_p->is_upgrade);

	//拷贝花瓶网关的参数
	if(gw_info_p->support_hpinfo){
		jniCopyBooleanValue(env,class_gw,"support_hpinfo",obj_gw,gw_info_p->support_hpinfo);
		copy_hp_gw_info(env, class_gw, obj_gw,gw_info_p);
	}
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, object_udp_info, fid, obj_gw);

	SAFE_DEL_LOCAL_REF(class_gw);
	SAFE_DEL_LOCAL_REF(obj_gw);
}

/*智能子模式设置*/
JNIEXPORT jint JNICALL
NAME(ClXyCtrlSmarthomeMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode){
	return cl_xy_ctrl_smarthome_mode(dev_handle, mode);
}

void copy_xywkq_info(JNIEnv* env, jclass class_udp_info, jobject object_udp_info, void *udp_dev){
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
	
	cl_xy_info_t *xywkq = (cl_xy_info_t*)udp_dev;
	int i = 0;
	int timePointCount = XY_SMARTHOME_SUB_MODE_NUM * XY_SMART_WEEK_MAX * XY_SMART_DAY_TIMEPOINT_MAX;

	if(!xywkq){
		return;
	}else{
		xywkq_smartmode = &xywkq->smart_mode;
		xywkq_st_mode = &xywkq_smartmode->sub_mode;
		xywkq_adjust = &xywkq->adjust;
	}

	class_xywkq = (*env)->FindClass(env, CLASS_XYWKQ);
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
	class_xywkq_smartmode = (*env)->FindClass(env, CLASS_XYWKQ_SMART_MODE);
	if(!class_xywkq_smartmode){
		goto quit;
	}
	object_xywkq_smartmode = (*env)->AllocObject(env, class_xywkq_smartmode);
	if(!object_xywkq_smartmode){
		goto quit;
	}

	class_xywkq_timepoint = (*env)->FindClass(env, CLASS_XYWKQTP);
	
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
	fid = (*env)->GetFieldID(env, class_xywkq_smartmode, "timepoint", "[L" CLASS_XYWKQTP ";");
	(*env)->SetObjectField(env, object_xywkq_smartmode, fid, array_timepoint);
	SAFE_DEL_LOCAL_REF(array_timepoint);
	
	fid = (*env)->GetFieldID(env, class_xywkq, "smart_mode", "L" CLASS_XYWKQ_SMART_MODE ";");
	(*env)->SetObjectField(env, object_xywkq, fid, object_xywkq_smartmode);

	//copy cl_xy_adjust_t成员到object_xywkq_adjust
	class_xywkq_adjust = (*env)->FindClass(env, CLASS_XYWKQ_ADJUST);
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
	
	fid = (*env)->GetFieldID(env, class_xywkq, "adjust", "L" CLASS_XYWKQ_ADJUST ";");
	(*env)->SetObjectField(env, object_xywkq, fid, object_xywkq_adjust);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, object_udp_info, fid, object_xywkq);
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

/****************XYWKQ END鑫源温控器****************/

/********copy数据**********/
int copy_tb_hp_user_config(JNIEnv* env, jclass class_tb_hp, jobject obj_tb_hp, cl_tb_info_t *tb_hp)
{
	cl_tb_user_config_t *uconfig = &tb_hp->u_config;

	JNI_COPY_SIMPLE_CLASS(env, class_tb_hp, obj_tb_hp, 
		                                 CLASS_HTCHP_USER_CONFIG, u_config,
		                                 TRIPLES(int, uconfig, cid),
		                                 TRIPLES(boolean, uconfig, onoff),
		                                 TRIPLES(int, uconfig, work_mode),
		                                 TRIPLES(int, uconfig, temp),
		                                 JNI_VAR_ARG_END);
	return RS_OK;
		
}


int copy_tb_hp_work_config(JNIEnv* env, jclass class_tb_hp, jobject obj_tb_hp, cl_tb_info_t *tb_hp)
{
	jclass class_workconfig = NULL;
	jobject object_workconfig = NULL;
	jfieldID fid;
	cl_tb_work_config_t *workconfig = &tb_hp->w_config;
	int ret = RS_OK;

	class_workconfig = (*env)->FindClass(env, CLASS_HTCHP_WORK_CONFIG);
	if (!class_workconfig) {
		ret = RS_ERROR;
		goto quit;
	}

	object_workconfig = (*env)->AllocObject(env, class_workconfig);
	if (!object_workconfig) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_workconfig, "cid", object_workconfig, workconfig->cid);
	jniCopyBooleanValue(env, class_workconfig, "return_cold_switch", object_workconfig, workconfig->return_cold_switch);
	jniCopyIntValue(env, class_workconfig, "facility_state", object_workconfig, workconfig->facility_state);
	jniCopyIntValue(env, class_workconfig, "sysfunc", object_workconfig, workconfig->sysfunc);
	jniCopyIntValue(env, class_workconfig, "return_diff_temp", object_workconfig, workconfig->return_diff_temp);
	jniCopyIntValue(env, class_workconfig, "heat_defrost_circle", object_workconfig, workconfig->heat_defrost_circle);
	jniCopyByteValue(env, class_workconfig, "start_heat_defrost_temp", object_workconfig, workconfig->start_heat_defrost_temp);
	jniCopyIntValue(env, class_workconfig, "stop_heat_defrost_temp", object_workconfig, workconfig->stop_heat_defrost_temp);
	jniCopyIntValue(env, class_workconfig, "stop_heat_defrost_time", object_workconfig, workconfig->stop_heat_defrost_time);
	jniCopyIntValue(env, class_workconfig, "eheat_value", object_workconfig, workconfig->eheat_value);
	jniCopyIntValue(env, class_workconfig, "backlight_delay_time", object_workconfig, workconfig->backlight_delay_time);
	jniCopyIntValue(env, class_workconfig, "fan_mode", object_workconfig, workconfig->fan_mode);

	fid = (*env)->GetFieldID(env, class_tb_hp, "w_config", "L" CLASS_HTCHP_WORK_CONFIG ";");
	(*env)->SetObjectField(env, obj_tb_hp, fid, object_workconfig);
quit:
	SAFE_DEL_LOCAL_REF(class_workconfig);
	SAFE_DEL_LOCAL_REF(object_workconfig);
	return ret;
}

int copy_tb_hp_temp_info(JNIEnv* env, jclass class_tb_hp, jobject obj_tb_hp, cl_tb_info_t *tb_hp)
{
	jclass class_temp_info = NULL;
	jobject object_temp_info = NULL;
	jfieldID fid;
	cl_tb_temp_info_t *temp_info = &tb_hp->temp_info;
	int ret = RS_OK;

	class_temp_info = (*env)->FindClass(env, CLASS_HTCHP_TEMP_INFO);
	if (!class_temp_info) {
		ret = RS_ERROR;
		goto quit;
	}

	object_temp_info = (*env)->AllocObject(env, class_temp_info);
	if (!object_temp_info) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_temp_info, "cid", object_temp_info, temp_info->cid);
	jniCopyIntValue(env, class_temp_info, "env_temp", object_temp_info, temp_info->env_temp);
	jniCopyIntValue(env, class_temp_info, "tankbottom_temp", object_temp_info, temp_info->tankbottom_temp);
	jniCopyIntValue(env, class_temp_info, "tanktop_temp", object_temp_info, temp_info->tanktop_temp);
	jniCopyIntValue(env, class_temp_info, "coil_temp", object_temp_info, temp_info->coil_temp);
	jniCopyIntValue(env, class_temp_info, "air_temp", object_temp_info, temp_info->air_temp);
	jniCopyIntValue(env, class_temp_info, "returnair_temp", object_temp_info, temp_info->returnair_temp);
	jniCopyIntValue(env, class_temp_info, "outwater_temp", object_temp_info, temp_info->outwater_temp);
	jniCopyIntValue(env, class_temp_info, "inwater_temp", object_temp_info, temp_info->inwater_temp);
	jniCopyIntValue(env, class_temp_info, "returnwater_temp", object_temp_info, temp_info->returnwater_temp);
	jniCopyIntValue(env, class_temp_info, "heat_capacity", object_temp_info, temp_info->heat_capacity);
	jniCopyIntValue(env, class_temp_info, "heat_time", object_temp_info, temp_info->heat_time);
	jniCopyIntValue(env, class_temp_info, "consumption_power", object_temp_info, temp_info->consumption_power);
	jniCopyIntValue(env, class_temp_info, "saving_power", object_temp_info, temp_info->saving_power);

	fid = (*env)->GetFieldID(env, class_tb_hp, "temp_info", "L" CLASS_HTCHP_TEMP_INFO ";");
	(*env)->SetObjectField(env, obj_tb_hp, fid, object_temp_info);
quit:
	SAFE_DEL_LOCAL_REF(class_temp_info);
	SAFE_DEL_LOCAL_REF(object_temp_info);
	return ret;
}

int copy_tb_hp_fault_stat(JNIEnv* env, jclass class_tb_hp, jobject obj_tb_hp, cl_tb_info_t *tb_hp)
{
	jclass class_fault_stat = NULL;
	jobject object_fault_stat = NULL;
	jfieldID fid;
	cl_tb_fault_stat *fault_stat = &tb_hp->fault_stat;
	int ret = RS_OK;

	class_fault_stat = (*env)->FindClass(env, CLASS_HTCHP_FAULT_STAT);
	if (!class_fault_stat) {
		ret = RS_ERROR;
		goto quit;
	}

	object_fault_stat = (*env)->AllocObject(env, class_fault_stat);
	if (!object_fault_stat) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_fault_stat, "cid", object_fault_stat, fault_stat->cid);
	jniCopyBooleanValue(env, class_fault_stat, "valve_expansion", object_fault_stat, fault_stat->valve_expansion);
	jniCopyIntValue(env, class_fault_stat, "slave_onoff", object_fault_stat, fault_stat->slave_onoff);
	jniCopyIntValue(env, class_fault_stat, "dev_fault", object_fault_stat, fault_stat->dev_fault);
	jniCopyIntValue(env, class_fault_stat, "dev_guard", object_fault_stat, fault_stat->dev_guard);
	jniCopyIntValue(env, class_fault_stat, "load_state", object_fault_stat, fault_stat->load_state);

	fid = (*env)->GetFieldID(env, class_tb_hp, "fault_stat", "L" CLASS_HTCHP_FAULT_STAT ";");
	(*env)->SetObjectField(env, obj_tb_hp, fid, object_fault_stat);
quit:
	SAFE_DEL_LOCAL_REF(class_fault_stat);
	SAFE_DEL_LOCAL_REF(object_fault_stat);
	return ret;
}

int copy_tb_hp_other_info(JNIEnv* env, jclass class_tb_hp, jobject obj_tb_hp, cl_tb_info_t *tb_hp)
{
	jclass class_other_info = NULL;
	jobject object_other_info = NULL;
	jfieldID fid;
	cl_tb_other_info *other_info = &tb_hp->other_info;
	int ret = RS_OK;

	class_other_info = (*env)->FindClass(env, CLASS_HTCHP_OTHER_INFO);
	if (!class_other_info) {
		ret = RS_ERROR;
		goto quit;
	}

	object_other_info = (*env)->AllocObject(env, class_other_info);
	if (!object_other_info) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_other_info, "cid", object_other_info, other_info->cid);
	jniCopyIntValue(env, class_other_info, "dev_info", object_other_info, other_info->dev_info);
	jniCopyIntValue(env, class_other_info, "dev_mode", object_other_info, other_info->dev_mode);
	jniCopyIntValue(env, class_other_info, "fw_version", object_other_info, other_info->fw_version);
	jniCopyIntValue(env, class_other_info, "mb_version", object_other_info, other_info->mb_version);

	fid = (*env)->GetFieldID(env, class_tb_hp, "other_info", "L" CLASS_HTCHP_OTHER_INFO ";");
	(*env)->SetObjectField(env, obj_tb_hp, fid, object_other_info);
quit:
	SAFE_DEL_LOCAL_REF(class_other_info);
	SAFE_DEL_LOCAL_REF(object_other_info);
	return ret;
}

void copy_udp_tmc_jnb(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_tmc_jnb = NULL;
	jobject object_tmc_jnb = NULL;
	jfieldID fid;
	cl_jnb_thermotat_info *tmc_jnb_dev = (cl_jnb_thermotat_info*)udp_dev;

	class_tmc_jnb = (*env)->FindClass(env, CLASS_TMC_JNB);
	if (!class_tmc_jnb) {
		goto quit;
	}

	object_tmc_jnb = (*env)->AllocObject(env, class_tmc_jnb);
	if (!object_tmc_jnb) {
		goto quit;
	}
	
	jniCopyIntValue(env, class_tmc_jnb, "work_status", object_tmc_jnb, tmc_jnb_dev->work_status);
	jniCopyIntValue(env, class_tmc_jnb, "env_temperature", object_tmc_jnb, tmc_jnb_dev->env_temperature);
	jniCopyIntValue(env, class_tmc_jnb, "temp_temperature", object_tmc_jnb, tmc_jnb_dev->temp_temperature);
	jniCopyIntValue(env, class_tmc_jnb, "vacation_remain_days", object_tmc_jnb, tmc_jnb_dev->vacation_remain_days);
	jniCopyIntValue(env, class_tmc_jnb, "vacation_temperature", object_tmc_jnb, tmc_jnb_dev->vacation_temperature);
	jniCopyIntValue(env, class_tmc_jnb, "vacation_days", object_tmc_jnb, tmc_jnb_dev->vacation_days);
	jniCopyIntValue(env, class_tmc_jnb, "comfort_temperaute", object_tmc_jnb, tmc_jnb_dev->comfort_temperaute);
	jniCopyIntValue(env, class_tmc_jnb, "comfort_hold_time", object_tmc_jnb, tmc_jnb_dev->comfort_hold_time);
	jniCopyIntValue(env, class_tmc_jnb, "economic_temperature", object_tmc_jnb, tmc_jnb_dev->economic_temperature);
	jniCopyIntValue(env, class_tmc_jnb, "economic_hold_time", object_tmc_jnb, tmc_jnb_dev->economic_hold_time);
	jniCopyIntArray(env, class_tmc_jnb, "scheduler", object_tmc_jnb, tmc_jnb_dev->scheduler,sizeof( tmc_jnb_dev->scheduler)/sizeof(u_int32_t));

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_tmc_jnb);
quit:
	SAFE_DEL_LOCAL_REF(class_tmc_jnb);
	SAFE_DEL_LOCAL_REF(object_tmc_jnb);
}

void copy_udp_tmc_yl(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_tmc_yl = NULL;
	jobject object_tmc_yl = NULL;
	jfieldID fid;
	jclass class_set_scene = NULL;
	jobject object_set_scene = NULL;
	jobjectArray object_scene_array = NULL;
	int i;
	int count = YL_TS_FUNC_MODE_MAX*YL_TS_MODE_MAX;

	cl_yl_thermostat_info *tmc_yl_dev = (cl_yl_thermostat_info*)udp_dev;
	ts_scene_t *tmc_scenes = NULL;

	class_tmc_yl = (*env)->FindClass(env, CLASS_TMC_YL);
	if (!class_tmc_yl) {
		goto quit;
	}

	object_tmc_yl = (*env)->AllocObject(env, class_tmc_yl);
	if (!object_tmc_yl) {
		goto quit;
	}

	jniCopyBooleanValue(env, class_tmc_yl, "onoff", object_tmc_yl, tmc_yl_dev->onoff);
	jniCopyIntValue(env, class_tmc_yl, "work_mode", object_tmc_yl, tmc_yl_dev->work_mode);
	jniCopyIntValue(env, class_tmc_yl, "gear", object_tmc_yl, tmc_yl_dev->gear);
	jniCopyIntValue(env, class_tmc_yl, "cur_scene", object_tmc_yl, tmc_yl_dev->cur_scene);
	jniCopyIntValue(env, class_tmc_yl, "temp", object_tmc_yl, tmc_yl_dev->temp);
	jniCopyIntValue(env, class_tmc_yl, "room_temp", object_tmc_yl, tmc_yl_dev->room_temp);

	class_set_scene = (*env)->FindClass(env, CLASS_SET_SCENE);
	object_scene_array = (*env)->NewObjectArray(env, count, class_set_scene, NULL);
	tmc_scenes = tmc_yl_dev->scene;
	if (!class_set_scene) {
		goto quit;
	}

	if(tmc_scenes != NULL) {
		for (i = 0; i < count; i++,tmc_scenes++) {
				object_set_scene = (*env)->AllocObject(env, class_set_scene);
				if(!object_set_scene){
					break;
				}
				jniCopyIntValue(env,class_set_scene,"set_tmp",object_set_scene,tmc_scenes->set_tmp);
				jniCopyIntValue(env,class_set_scene,"gear",object_set_scene,tmc_scenes->gear);
				
				(*env)->SetObjectArrayElement(env, object_scene_array, i, object_set_scene);
				(*env)->DeleteLocalRef(env, object_set_scene);
			}

			fid = (*env)->GetFieldID(env, class_tmc_yl, "scene", "[L" CLASS_SET_SCENE ";");
			(*env)->SetObjectField(env, object_tmc_yl, fid, object_scene_array);
						
			SAFE_DEL_LOCAL_REF(object_scene_array);
	}
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_tmc_yl);
quit:
	SAFE_DEL_LOCAL_REF(class_tmc_yl);
	SAFE_DEL_LOCAL_REF(object_tmc_yl);
}



void copy_udp_pdc_jcx(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_pdc_jcx = NULL;
	jobject object_pdc_jcx = NULL;
	jfieldID fid;
	cl_jcx_power_box_info *pdc_jcx_dev = (cl_jcx_power_box_info*)udp_dev;

	class_pdc_jcx = (*env)->FindClass(env, CLASS_PDC_JCX);
	if (!class_pdc_jcx) {
		goto quit;
	}

	object_pdc_jcx = (*env)->AllocObject(env, class_pdc_jcx);
	if (!object_pdc_jcx) {
		goto quit;
	}

	jniCopyIntValue(env, class_pdc_jcx, "voltage", object_pdc_jcx, pdc_jcx_dev->voltage);
	jniCopyIntValue(env, class_pdc_jcx, "elec", object_pdc_jcx, pdc_jcx_dev->elec);
	jniCopyIntValue(env, class_pdc_jcx, "active_power", object_pdc_jcx, pdc_jcx_dev->active_power);
	jniCopyIntValue(env, class_pdc_jcx, "reactive_power", object_pdc_jcx, pdc_jcx_dev->reactive_power);
	jniCopyIntValue(env, class_pdc_jcx, "power_factor", object_pdc_jcx, pdc_jcx_dev->power_factor);
	jniCopyIntValue(env, class_pdc_jcx, "frequency", object_pdc_jcx, pdc_jcx_dev->frequency);
	jniCopyIntValue(env, class_pdc_jcx, "active_degree", object_pdc_jcx, pdc_jcx_dev->active_degree);
	jniCopyIntValue(env, class_pdc_jcx, "reactive_degree", object_pdc_jcx, pdc_jcx_dev->reactive_degree);
	jniCopyIntValue(env, class_pdc_jcx, "jcx_sn", object_pdc_jcx, pdc_jcx_dev->jcx_sn);
	jniCopyIntValue(env, class_pdc_jcx, "jcx_soft_ver", object_pdc_jcx, pdc_jcx_dev->jcx_soft_ver);
	jniCopyIntValue(env, class_pdc_jcx, "jcx_hardware_ver", object_pdc_jcx, pdc_jcx_dev->jcx_hardware_ver);
	jniCopyIntValue(env, class_pdc_jcx, "jcx_hardware_ver", object_pdc_jcx, pdc_jcx_dev->jcx_hardware_ver);
	jniCopyIntValue(env, class_pdc_jcx, "channel_num", object_pdc_jcx, pdc_jcx_dev->channel_num);
	jniCopyIntValue(env, class_pdc_jcx, "channelstat", object_pdc_jcx, pdc_jcx_dev->channelstat);
//	jniCopyString(env, class_pdc_jcx, "channel_names", object_pdc_jcx, pdc_jcx_dev->channel_names);


	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_pdc_jcx);
quit:
	SAFE_DEL_LOCAL_REF(class_pdc_jcx);
	SAFE_DEL_LOCAL_REF(object_pdc_jcx);
}

void copy_chiffo(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_chiffo = NULL;
	jobject object_chiffo= NULL;
	jclass class_chiffo_daytimer = NULL;
	jobject object_chiffo_daytimer= NULL;
	jclass class_chiffo_section_timer = NULL;
	jobject object_chiffo_section_daytimer= NULL;
	jobject section_array = NULL;
	jobject day_array = NULL;
	jfieldID fid;
	int i = 0 , j = 0;
	cl_chiffo_floor_heater_info_t *chiffo_dev = (cl_chiffo_floor_heater_info_t*)udp_dev;

	class_chiffo = (*env)->FindClass(env, CLASS_CHIFFO);
	object_chiffo = (*env)->AllocObject(env, class_chiffo);
	class_chiffo_daytimer = (*env)->FindClass(env, CLASS_CHIFFO_DAY_TIMER);
	class_chiffo_section_timer = (*env)->FindClass(env, CLASS_CHIFFO_SECTION_TIMER);

//	LOGD("find copy_chiffo info: hour = %d,minute=%d",chiffo_dev->hour,chiffo_dev->min);

	jni_copy_simple_class(env, class_chiffo, object_chiffo,  
		                       TRIPLES(boolean, chiffo_dev, is_on),
		                       TRIPLES(int, chiffo_dev, fault_code),
		                       TRIPLES(boolean, chiffo_dev, is_fan_working),
		                       TRIPLES(boolean, chiffo_dev, is_fire_working),
		                       TRIPLES(boolean, chiffo_dev, is_floor_heat_working),
		                       TRIPLES(boolean, chiffo_dev, is_radiator_working),
		                       TRIPLES(boolean, chiffo_dev, is_pump_working),
		                       TRIPLES(boolean, chiffo_dev, is_water_working),
		                       TRIPLES(int, chiffo_dev, next_heat_time),
		                       TRIPLES(boolean, chiffo_dev, is_water_mode_on),
		                       TRIPLES(boolean, chiffo_dev, is_heater_mode_on),
		                       TRIPLES(int, chiffo_dev, water_setting_temp),
		                       TRIPLES(int, chiffo_dev, water_current_temp),
		                       TRIPLES(int, chiffo_dev, cur_water_pressure),
		                       TRIPLES(int, chiffo_dev, heater_setting_temp),
		                       TRIPLES(int, chiffo_dev, heater_current_temp),
		                       TRIPLES(byte, chiffo_dev, scene),
		                       TRIPLES(byte, chiffo_dev, loop_type),
		                       TRIPLES(byte, chiffo_dev, rssi),
		                       TRIPLES(byte, chiffo_dev, hour),
		                       TRIPLES(byte, chiffo_dev, min),
		                       JNI_VAR_ARG_END);

	day_array = (*env)->NewObjectArray(env, CHIFFO_TIMER_DAYS_CNT, class_chiffo_daytimer, NULL);

	for (i = 0; i < CHIFFO_TIMER_DAYS_CNT; ++i) {
		object_chiffo_daytimer = (*env)->AllocObject(env, class_chiffo_daytimer);
		section_array = (*env)->NewObjectArray(env, CHIFFO_TIMER_SECTION_PER_DAY, class_chiffo_section_timer, NULL);
		for (j = 0; j < CHIFFO_TIMER_SECTION_PER_DAY; ++j) {
			object_chiffo_section_daytimer = (*env)->AllocObject(env, class_chiffo_section_timer);

			jni_copy_simple_class(env, class_chiffo_section_timer, object_chiffo_section_daytimer, 
			                           TRIPLES(boolean, &chiffo_dev->timer_info[i].items[j], is_enable),
			                           TRIPLES(int, &chiffo_dev->timer_info[i].items[j], temp),
			                           JNI_VAR_ARG_END);
			(*env)->SetObjectArrayElement(env, section_array, j, object_chiffo_section_daytimer);
			SAFE_DEL_LOCAL_REF(object_chiffo_section_daytimer);
		}
		fid = (*env)->GetFieldID(env, class_chiffo_daytimer, "items", "[L" CLASS_CHIFFO_SECTION_TIMER ";");
		(*env)->SetObjectField(env, object_chiffo_daytimer, fid, section_array);
		SAFE_DEL_LOCAL_REF(section_array);

		(*env)->SetObjectArrayElement(env, day_array, i, object_chiffo_daytimer);
		SAFE_DEL_LOCAL_REF(object_chiffo_daytimer);
	}

	fid = (*env)->GetFieldID(env, class_chiffo, "timer_info", "[L" CLASS_CHIFFO_DAY_TIMER ";");
	(*env)->SetObjectField(env, object_chiffo, fid, day_array);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_chiffo);
	SAFE_DEL_LOCAL_REF(class_chiffo);
	SAFE_DEL_LOCAL_REF(object_chiffo);
	SAFE_DEL_LOCAL_REF(class_chiffo_daytimer);
	

	SAFE_DEL_LOCAL_REF(class_chiffo_section_timer);
	SAFE_DEL_LOCAL_REF(day_array);
}

void copy_hx_pobiji_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_string = NULL;
	jobject array_obj = NULL;
	jobject obj_string = NULL;
	int i = 0;
	jstring str;
	jfieldID fid;

	jclass pobiji_class;
	jobject pobiji_obj;

	cl_hx_info * pobiji_p = (cl_hx_info *)udp_dev;
	
	pobiji_class = (*env)->FindClass(env, CLASS_HX_POBIJI);
	pobiji_obj = (*env)->AllocObject(env, pobiji_class);
	
	jni_copy_simple_class(env, pobiji_class, pobiji_obj,  
		                       	TRIPLES(byte, pobiji_p, cur_mode),
								TRIPLES(byte, pobiji_p, cur_tmp),
								TRIPLES(byte, pobiji_p, cur_speed),
								TRIPLES(byte, pobiji_p, cur_send_finsh),
								TRIPLES(boolean, pobiji_p, cur_pause),
								TRIPLES(int, pobiji_p, work_time),
								TRIPLES(byte, pobiji_p, cur_send_err),
								TRIPLES(boolean, pobiji_p, idle_status),
								TRIPLES(boolean, pobiji_p, keep_warm),
		                      	JNI_VAR_ARG_END);

	class_string = (*env)->FindClass(env, "java/lang/String");
	array_obj = (*env)->NewObjectArray(env, HX_MODE_MAX, class_string, NULL);
	
	for (i = HX_MODE_DIY1; i < HX_MODE_MAX; ++i) {
		if (NULL == pobiji_p->name[i]) {
			LOGD("pobiji diy_name%d is null", i);
			continue;
		}
		char *pstr = pobiji_p->name[i];
		doString(&pstr);
		str = (*env)->NewStringUTF(env, pobiji_p->name[i]);
		(*env)->SetObjectArrayElement(env, array_obj, i, str);
		SAFE_DEL_LOCAL_REF(str);
	}
	
	fid = (*env)->GetFieldID(env, pobiji_class, "name", "[Ljava/lang/String;");
	(*env)->SetObjectField(env, pobiji_obj, fid, array_obj);

	SAFE_DEL_LOCAL_REF(class_string);
	SAFE_DEL_LOCAL_REF(array_obj);
	SAFE_DEL_LOCAL_REF(obj_string);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, pobiji_obj);

	SAFE_DEL_LOCAL_REF(pobiji_class);
	SAFE_DEL_LOCAL_REF(pobiji_obj);
}

void copy_tl_heating_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{	
	jclass telin_class, telin_time_item_class;
	jobject telin_info_obj, telin_time_obj, telin_time_item;
	jfieldID fid;
	int i;
	
	cl_tl_info_t * tl_heating_p = (cl_tl_info_t *)udp_dev;
	cl_tl_ctrl_stat_t *tl_ctrl_stat = &(tl_heating_p->ctrl_stat);
	cl_tl_adv_info_t *tl_adv_info = &(tl_heating_p->adv_info);
	cl_tl_total_time_t *tl_total_info = &(tl_heating_p->total_info);
	cl_tl_timer_item_t* tl_timers = tl_heating_p->time_info.time;
	cl_tl_time_sync_t* cl_tl_time_sync_t_p = &(tl_heating_p->dev_time_sync_info);

	telin_time_item_class = (*env)->FindClass(env, CLASS_TELIN_HEATING_TIME_ITEM);
	telin_class = (*env)->FindClass(env, CLASS_TELIN_HEATING);
	telin_info_obj = (*env)->AllocObject(env, telin_class);

	jni_copy_simple_class(env, telin_class, telin_info_obj,  
		                       TRIPLES(boolean, tl_ctrl_stat, onoff),
		                       TRIPLES(byte, tl_ctrl_stat, mode),
		                       TRIPLES(byte, tl_ctrl_stat, speed),
		                       TRIPLES(byte, tl_ctrl_stat, temp),
		                       TRIPLES(short, tl_ctrl_stat, room_temp),
		                       TRIPLES(short, tl_ctrl_stat, state),
		                       TRIPLES(byte, tl_ctrl_stat, valve_stat),
		                       TRIPLES(byte, tl_ctrl_stat, cool_valve_stat),
		                       TRIPLES(byte, tl_ctrl_stat, hot_valve_stat),
		                       TRIPLES(boolean, tl_ctrl_stat, charge_enable),
		                       JNI_VAR_ARG_END);

	jni_copy_simple_class(env, telin_class, telin_info_obj,  
		                       TRIPLES(int, tl_total_info, high_gear_time_cal),
		                       TRIPLES(int, tl_total_info, low_gear_time),
		                       TRIPLES(int, tl_total_info, mid_gear_time),
		                       TRIPLES(int, tl_total_info, high_gear_time),
		                       JNI_VAR_ARG_END);

	jni_copy_simple_class(env, telin_class, telin_info_obj,  
		                       TRIPLES(short, tl_adv_info, lock_flags),
		                       TRIPLES(short, tl_adv_info, temp_bandwidth),
		                       TRIPLES(short, tl_adv_info, charge_factor_low),
		                       TRIPLES(short, tl_adv_info, charge_factor_mid),
		                       TRIPLES(boolean, tl_adv_info, fan_under_control),
		                       TRIPLES(boolean, tl_adv_info, time_on),
		                       TRIPLES(boolean, tl_adv_info, eco_mode),
		                       TRIPLES(boolean, tl_adv_info, self_learn),
		                       JNI_VAR_ARG_END);

	telin_time_obj = (*env)->NewObjectArray(env, TL_TIME_CNT_PER_DAY, telin_time_item_class, NULL);
	for (i = 0; i < TL_TIME_CNT_PER_DAY; i++) {
		telin_time_item = (*env)->AllocObject(env, telin_time_item_class);
		jni_copy_simple_class(env, telin_time_item_class, telin_time_item, 
			                       TRIPLES(short, tl_timers + i, temp),
			                       TRIPLES(short, tl_timers + i, hour),
			                       TRIPLES(short, tl_timers + i, min),
			                       JNI_VAR_ARG_END);
			
		(*env)->SetObjectArrayElement(env, telin_time_obj, i, telin_time_item);
		SAFE_DEL_LOCAL_REF(telin_time_item);
	}

	fid = (*env)->GetFieldID(env, telin_class, "time", "[L" CLASS_TELIN_HEATING_TIME_ITEM ";");
	(*env)->SetObjectField(env, telin_info_obj, fid, telin_time_obj);

	jni_copy_simple_class(env, telin_class, telin_info_obj,  
		                       TRIPLES(boolean, cl_tl_time_sync_t_p, is_data_valid),
		                       TRIPLES(boolean, cl_tl_time_sync_t_p, is_auto_sync),
		                       TRIPLES(int, cl_tl_time_sync_t_p, dev_time),
		                       TRIPLES(int, cl_tl_time_sync_t_p, local_update_time),
		                       JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, telin_info_obj);

	SAFE_DEL_LOCAL_REF(telin_time_obj);
	SAFE_DEL_LOCAL_REF(telin_class);
	SAFE_DEL_LOCAL_REF(telin_time_item_class);
	SAFE_DEL_LOCAL_REF(telin_info_obj);
}

void copy_qp_cp_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{	
	jfieldID fid;
	int i;
	jclass info_class, config_class,scenes_class,scene_item_class,scene_param_class;
	jobject info_obj, config_obj,scenes_obj,scene_item_obj,scene_param_obj,scene_array;
	cl_qpcp_info_t* qp_info = (cl_qpcp_info_t*)udp_dev;	
	cl_qpcp_config_t* qc;
	cl_qpcp_sp_t* qs;
	cl_qpcp_scp_t* param;

	info_class = (*env)->FindClass(env, CLASS_QP_INFO);
	config_class = (*env)->FindClass(env, CLASS_QP_CONFIG);
	scenes_class = (*env)->FindClass(env, CLASS_QP_SCENES);
	scene_item_class = (*env)->FindClass(env, CLASS_QP_SCENE_ITEM);
	scene_param_class = (*env)->FindClass(env, CLASS_QP_TEA_SCENE_PARAM);
	
	if(!qp_info || !info_class || !config_class || !scenes_class || !scene_item_class || !scene_param_class){
		return ;
	}

	//基本数据
	info_obj = (*env)->AllocObject(env, info_class);
	jni_copy_simple_class(env, info_class, info_obj,  
		                       TRIPLES(boolean, qp_info, cur_onof),
		                       TRIPLES(int, qp_info, cur_mode),
		                       TRIPLES(int, qp_info, cur_water_temp),
		                       TRIPLES(int, qp_info, cur_water_time),
		                       TRIPLES(int, qp_info, cur_water_state),
		                       TRIPLES(int, qp_info, cur_error),
		                       TRIPLES(int, qp_info, cur_remain_water_time),
		                       TRIPLES(int, qp_info, cur_production_status),
							   TRIPLES(int, qp_info, disinfect_plan_id),
							   TRIPLES(int, qp_info, boil_plan_id),
		                       TRIPLES(int, qp_info, disinfect_index),
		                       TRIPLES(int, qp_info, boil_index),
		                       TRIPLES(int, qp_info, mode),
		                       TRIPLES(int, qp_info, water_warning),

	

		                       JNI_VAR_ARG_END);

	//消毒数据
	config_obj = (*env)->AllocObject(env, config_class);
	qc = &qp_info->disinfect;
	jni_copy_simple_class(env, config_class, config_obj,  
		                       TRIPLES(int, qc, temp),
		                       TRIPLES(int, qc, power),
		                       TRIPLES(int, qc, time),
		                       TRIPLES(int, qc, thermal_temp),
					    TRIPLES(int, qc, thermal_time),
		                       JNI_VAR_ARG_END);
	fid = (*env)->GetFieldID(env, info_class, "disinfect", "L" CLASS_QP_CONFIG ";");
	(*env)->SetObjectField(env, info_obj, fid, config_obj);
	SAFE_DEL_LOCAL_REF(config_obj);


	//煮水数据
	config_obj = (*env)->AllocObject(env, config_class);
	qc = &qp_info->boil;
	jni_copy_simple_class(env, config_class, config_obj,  
		                       TRIPLES(int, qc, temp),
		                       TRIPLES(int, qc, power),
		                       TRIPLES(int, qc, time),
		                       TRIPLES(int, qc, thermal_temp),
					    TRIPLES(int, qc, thermal_time),
		                       JNI_VAR_ARG_END);
	fid = (*env)->GetFieldID(env, info_class, "boil", "L" CLASS_QP_CONFIG ";");
	(*env)->SetObjectField(env, info_obj, fid, config_obj);
	SAFE_DEL_LOCAL_REF(config_obj);

	//情景
	
	if(qp_info->pscene != NULL && qp_info->pscene->num > 0){
		scenes_obj = (*env)->AllocObject(env, scenes_class);
		jni_copy_simple_class(env, scenes_class, scenes_obj,  
		                       TRIPLES(int, qp_info->pscene, num),
		                       JNI_VAR_ARG_END);

		scene_array = (*env)->NewObjectArray(env, qp_info->pscene->num , scene_item_class, NULL);
		for (i = 0,qs = &qp_info->pscene->scene[0]; i < qp_info->pscene->num; i++,qs++) {
			scene_item_obj =  (*env)->AllocObject(env, scene_item_class);

			jni_copy_simple_class(env, scene_item_class, scene_item_obj, 
				                       TRIPLES(int, qs, scene_id),
				                       TRIPLES(int, qs, create_time),
				                       TRIPLES(String, qs, name),
				                       JNI_VAR_ARG_END);

			scene_param_obj = (*env)->AllocObject(env, scene_param_class);
			param = &qs->com_param;

			jni_copy_simple_class(env, scene_param_class, scene_param_obj, 
				                       TRIPLES(int, param, action),
				                       TRIPLES(int, param, temp),
							    TRIPLES(int, param, thermal_temp),
				                       TRIPLES(int, param, thermal_time),
							    TRIPLES(int, param, time),
				                       TRIPLES(int, param, power),
				                       TRIPLES(int, param, water_time),
				                       TRIPLES(int, param, index),
			
				                       JNI_VAR_ARG_END);

			fid = (*env)->GetFieldID(env, scene_item_class, "com_param", "L" CLASS_QP_TEA_SCENE_PARAM ";");
			(*env)->SetObjectField(env, scene_item_obj, fid, scene_param_obj);
			SAFE_DEL_LOCAL_REF(scene_param_obj);
				
			(*env)->SetObjectArrayElement(env, scene_array, i, scene_item_obj);
			SAFE_DEL_LOCAL_REF(scene_item_obj);
		}

		fid = (*env)->GetFieldID(env, scenes_class, "scene", "[L" CLASS_QP_SCENE_ITEM ";");
		(*env)->SetObjectField(env, scenes_obj, fid, scene_array);
		SAFE_DEL_LOCAL_REF(scene_array);
		
		fid = (*env)->GetFieldID(env, info_class, "scene", "L" CLASS_QP_SCENES ";");
		(*env)->SetObjectField(env, info_obj, fid, scenes_obj);
		SAFE_DEL_LOCAL_REF(scenes_obj);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, info_obj);

	SAFE_DEL_LOCAL_REF(info_obj);
	SAFE_DEL_LOCAL_REF(info_class);
	SAFE_DEL_LOCAL_REF(config_class);
	SAFE_DEL_LOCAL_REF(scenes_class);
	SAFE_DEL_LOCAL_REF(scene_item_class);
	SAFE_DEL_LOCAL_REF(scene_param_class);
}

void copy_qp_pan_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{	
	jfieldID fid;
	int i;
	jclass info_class,scene_item_class,scene_param_class;
	jobject info_obj,scene_item_obj,scene_param_obj,scene_array;
	cl_qp_pot_info_t* qp_info = (cl_qp_pot_info_t*)udp_dev;	
	cl_qp_pot_scene_t* qs;
	cl_qp_pot_scene_param_t* param;

	info_class = (*env)->FindClass(env, CLASS_QP_PAN_INFO);
	scene_item_class = (*env)->FindClass(env, CLASS_QP_PAN_SCENE_ITEM);
	scene_param_class = (*env)->FindClass(env, CLASS_QP_PAN_SCENE_PARAM);

	if(!qp_info || !info_class  || !scene_item_class || !scene_param_class){
		return ;
	}

	//基本数据
	info_obj = (*env)->AllocObject(env, info_class);
	jni_copy_simple_class(env, info_class, info_obj,  


		                       TRIPLES(int, qp_info, cur_id),
		                       TRIPLES(int, qp_info, cooking_remain_time),
		                       TRIPLES(int, qp_info, stat),
		                       TRIPLES(int, qp_info, cooking_mode),
		                       TRIPLES(boolean, qp_info, is_complete),
		                       TRIPLES(boolean, qp_info, is_pot_cover_open),
							   TRIPLES(int, qp_info, microswitch),
							   TRIPLES(int, qp_info, warm_temp),
							   TRIPLES(int, qp_info, pot_temp),
							   TRIPLES(int, qp_info, food_quantity),
							   TRIPLES(int, qp_info, err_num),
							   TRIPLES(int, qp_info, scene_count),
		                       JNI_VAR_ARG_END);
	//情景
	if(qp_info->scene_count > 0 && qp_info->scenes != NULL){
		scene_array = (*env)->NewObjectArray(env, qp_info->scene_count , scene_item_class, NULL);

		for (i = 0,qs = &qp_info->scenes[0]; i < qp_info->scene_count; i++,qs++) {
			
			scene_item_obj =  (*env)->AllocObject(env, scene_item_class);
			
			jni_copy_simple_class(env, scene_item_class, scene_item_obj, 
				                       TRIPLES(String, qs, name),
				                       TRIPLES(int, qs, create_time),
				                       JNI_VAR_ARG_END);

			scene_param_obj = (*env)->AllocObject(env, scene_param_class);
			param = &qs->s_info;
			jni_copy_simple_class(env, scene_param_class, scene_param_obj, 

				                       TRIPLES(int, param, s_id),
				                       TRIPLES(int, param, cook_time),
							    	   TRIPLES(int, param, hot_degress),
				                       TRIPLES(int, param, microswitch),
							           TRIPLES(int, param, warm_temp),
				                       TRIPLES(int, param, cooking_mode),
				                       TRIPLES(int, param, set_action),
				                       TRIPLES(int, param, rice_degress),
			
				                       JNI_VAR_ARG_END);

			fid = (*env)->GetFieldID(env, scene_item_class, "s_info", "L" CLASS_QP_PAN_SCENE_PARAM ";");
			(*env)->SetObjectField(env, scene_item_obj, fid, scene_param_obj);
			SAFE_DEL_LOCAL_REF(scene_param_obj);

			(*env)->SetObjectArrayElement(env, scene_array, i, scene_item_obj);
			SAFE_DEL_LOCAL_REF(scene_item_obj);

		}

		fid = (*env)->GetFieldID(env, info_class, "scenes", "[L" CLASS_QP_PAN_SCENE_ITEM ";");
		(*env)->SetObjectField(env, info_obj, fid, scene_array);
		SAFE_DEL_LOCAL_REF(scene_array);

	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, info_obj);

	SAFE_DEL_LOCAL_REF(info_obj);
	SAFE_DEL_LOCAL_REF(info_class);
	SAFE_DEL_LOCAL_REF(scene_item_class);
	SAFE_DEL_LOCAL_REF(scene_param_class);
}

void copy_qp_pobiji_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{	
	jfieldID fid;
	int i, j;
	jclass info_class,scene_class,mix_class,hot_class,action_class;
	jobject info_obj,scene_item_obj,scene_array,mix_obj, hot_obj,action_obj,action_array;
	
	cl_qp_pbj_info_t* qp_info = (cl_qp_pbj_info_t*)udp_dev;	
	cl_qp_pbj_scene_t* qs_scene;
	cl_qp_pbj_action_t* qs_action;
	cl_qp_pbj_mix_info_t* qs_mix;
	cl_qp_pbj_hot_info_t* qs_hot;


	info_class = (*env)->FindClass(env, CLASS_QP_POBIJI_INFO);
	scene_class = (*env)->FindClass(env, CLASS_QP_POBIJI_SCENE);
	mix_class = (*env)->FindClass(env, CLASS_QP_POBIJI_MIX_INFO);
	hot_class = (*env)->FindClass(env, CLASS_QP_POBIJI_HOT_INFO);
	action_class = (*env)->FindClass(env, CLASS_QP_POBIJI_ACTION);
	
	
	if(!qp_info || !info_class  || !scene_class || !mix_class || !hot_class || !action_class){
		return ;
	}

	//基本数据
	info_obj = (*env)->AllocObject(env, info_class);

	if(&qp_info->stat == NULL) {
		return ;
	}
	
	jni_copy_simple_class(env, info_class, info_obj,  

		TRIPLES(boolean, &qp_info->stat, is_data_valid),
		TRIPLES(boolean, &qp_info->stat, on_off),
		TRIPLES(int, &qp_info->stat, cur_exec_id),
		TRIPLES(int, &qp_info->stat, work_stat),
		TRIPLES(int, &qp_info->stat, cur_mode),
		TRIPLES(int, &qp_info->stat, mix_power),
		TRIPLES(int, &qp_info->stat, err_no),
		TRIPLES(int, &qp_info->stat, temp),
		TRIPLES(int, qp_info, scene_num),
		JNI_VAR_ARG_END);

	//情景
	if(qp_info->scene_num > 0 && qp_info->scene != NULL) {
		scene_array = (*env)->NewObjectArray(env, qp_info->scene_num, scene_class, NULL);
		for(i = 0, qs_scene = &qp_info->scene[0]; i < qp_info->scene_num; i++,qs_scene++) {

			scene_item_obj  =  (*env)->AllocObject(env, scene_class);
			jni_copy_simple_class(env, scene_class, scene_item_obj, 
				TRIPLES(int, qs_scene, scene_id),
				TRIPLES(String, qs_scene, name),
				TRIPLES(int, qs_scene, step_count),
				TRIPLES(int, qs_scene, warm_time),
				TRIPLES(int, qs_scene, warm_temp),
				JNI_VAR_ARG_END);

			if(&qs_scene->action != NULL) {

				action_array = (*env)->NewObjectArray(env, qs_scene->step_count, action_class, NULL);

				for(j = 0 ,qs_action = &qs_scene->action[0]; j < qs_scene->step_count; j ++, qs_action ++) {
					action_obj	=  (*env)->AllocObject(env, action_class);
					
					jni_copy_simple_class(env, action_class, action_obj, 
					TRIPLES(int, qs_action, data_type),
					JNI_VAR_ARG_END);

					if(qs_action->data_type == QP_MODE_MIX) {

						if(&qs_scene->action != NULL) {
							mix_obj =  (*env)->AllocObject(env, mix_class);
							qs_mix = &qs_action->m_info;
	
							jni_copy_simple_class(env, mix_class, mix_obj,
								TRIPLES(int, qs_mix, gear),
								TRIPLES(int, qs_mix, time),
								TRIPLES(int, qs_mix, freq),
								JNI_VAR_ARG_END);
											
							fid = (*env)->GetFieldID(env, action_class, "m_info", "L"CLASS_QP_POBIJI_MIX_INFO";");
							(*env)->SetObjectField(env, action_obj, fid, mix_obj);
							SAFE_DEL_LOCAL_REF(mix_obj);
							
						}
					} else if(qs_action->data_type == QP_MODE_HOT) {
					
						if(&qs_scene->action != NULL) {
							
							hot_obj = (*env)->AllocObject(env, hot_class);
							qs_hot= &qs_action->h_info;
						
							jni_copy_simple_class(env, hot_class, hot_obj,
								TRIPLES(int, qs_hot, temp),
								TRIPLES(int, qs_hot, time),
								JNI_VAR_ARG_END);
						
			
							fid = (*env)->GetFieldID(env, action_class, "h_info", "L"CLASS_QP_POBIJI_HOT_INFO";");
							(*env)->SetObjectField(env, action_obj, fid, hot_obj);
							SAFE_DEL_LOCAL_REF(hot_obj);
							
						}
						

					}
				
					(*env)->SetObjectArrayElement(env, action_array, j, action_obj);
					SAFE_DEL_LOCAL_REF(action_obj);

				}
			
				fid = (*env)->GetFieldID(env, scene_class, "action", "[L" CLASS_QP_POBIJI_ACTION";");
		        (*env)->SetObjectField(env, scene_item_obj, fid, action_array);
		         SAFE_DEL_LOCAL_REF(action_array);
		
			}

			

			(*env)->SetObjectArrayElement(env, scene_array, i, scene_item_obj);
			SAFE_DEL_LOCAL_REF(scene_item_obj);

		}
	

		fid = (*env)->GetFieldID(env, info_class, "scene", "[L" CLASS_QP_POBIJI_SCENE";");
		(*env)->SetObjectField(env, info_obj, fid, scene_array);
		SAFE_DEL_LOCAL_REF(scene_array);

	}


	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, info_obj);

	SAFE_DEL_LOCAL_REF(info_obj);
	SAFE_DEL_LOCAL_REF(info_class);
	SAFE_DEL_LOCAL_REF(scene_class);
	SAFE_DEL_LOCAL_REF(mix_class);
	SAFE_DEL_LOCAL_REF(hot_class);
	SAFE_DEL_LOCAL_REF(action_class);
}
	

void copy_tb_heater_pump(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_tbhp = NULL;
	jobject object_tbhp = NULL;
	jfieldID fid;
	cl_tb_info_t *tbhp_dev = (cl_tb_info_t*)udp_dev;

	class_tbhp = (*env)->FindClass(env, CLASS_HTCHP);
	object_tbhp = (*env)->AllocObject(env, class_tbhp);

	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp, 
		                                 CLASS_HTCHP_USER_CONFIG, u_config,
		                                 TRIPLES(int, &tbhp_dev->u_config, cid),
		                                 TRIPLES(boolean, &tbhp_dev->u_config, onoff),
		                                 TRIPLES(int, &tbhp_dev->u_config, work_mode),
		                                 TRIPLES(int, &tbhp_dev->u_config, temp),
		                                 JNI_VAR_ARG_END);
		
	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp,
		                                  CLASS_HTCHP_WORK_CONFIG, w_config,
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
		                                  CLASS_HTCHP_TEMP_INFO, temp_info,
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
		                                  CLASS_HTCHP_FAULT_STAT, fault_stat,
		                                  TRIPLES(int, &tbhp_dev->fault_stat, cid),
		                                  TRIPLES(boolean, &tbhp_dev->fault_stat, valve_expansion),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, slave_onoff),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, dev_fault),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, dev_guard),
		                                  TRIPLES(int, &tbhp_dev->fault_stat, load_state),
		                                  JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_tbhp, object_tbhp,
		                                  CLASS_HTCHP_OTHER_INFO, other_info,
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

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_tbhp);
	
	SAFE_DEL_LOCAL_REF(class_tbhp);
	SAFE_DEL_LOCAL_REF(object_tbhp);
}


/*
void copy_tb_heater_pump(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_tbhp = NULL;
	jobject object_tbhp = NULL;
	jfieldID fid;
	cl_tb_info_t *tbhp_dev = (cl_tb_info_t*)udp_dev;


	class_tbhp = (*env)->FindClass(env, CLASS_HTCHP);
	if (!class_tbhp) {
		goto quit;
	}

	object_tbhp = (*env)->AllocObject(env, class_tbhp);
	if (!object_tbhp) {
		goto quit;
	}
	jniCopyIntValue(env, class_tbhp, "bind_state", object_tbhp, tbhp_dev->bind_state);
	jniCopyString(env, class_tbhp, "tb_sn", object_tbhp, tbhp_dev->tb_sn);

	if (copy_tb_hp_user_config(env, class_tbhp, object_tbhp, tbhp_dev) != RS_OK) {
		goto quit;
	}
	
	if (copy_tb_hp_work_config(env, class_tbhp, object_tbhp, tbhp_dev) != RS_OK) {
		goto quit;
	}
	
	if (copy_tb_hp_temp_info(env, class_tbhp, object_tbhp, tbhp_dev) != RS_OK) {
		goto quit;
	}

	if (copy_tb_hp_fault_stat(env, class_tbhp, object_tbhp, tbhp_dev) != RS_OK) {
		goto quit;
	}

	if (copy_tb_hp_other_info(env, class_tbhp, object_tbhp, tbhp_dev) != RS_OK) {
		goto quit;
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_tbhp);
quit:
	SAFE_DEL_LOCAL_REF(class_tbhp);
	SAFE_DEL_LOCAL_REF(object_tbhp);
}
*/

void copyLampState(JNIEnv* env, jclass clazz, jobject object, cl_lede_led_state_t *state)
{
	if (state->L > 100 && state->L <= 127) {
		state->L = 100;
	} else if (state->L > 127) {
		state->L = 0;
	}
	jniCopyIntValue(env, clazz, "r", object, state->R);
	jniCopyIntValue(env, clazz, "g", object, state->G);
	jniCopyIntValue(env, clazz, "b", object, state->B);
	jniCopyIntValue(env, clazz, "l", object, state->L);
	jniCopyIntValue(env, clazz, "cold", object, state->cold);
	jniCopyIntValue(env, clazz, "modeId", object, state->mod_id);
	jniCopyIntValue(env, clazz, "action", object, state->action);

	jniCopyBooleanValue(env, clazz, "onoff", object, state->power);
}

void copyLampTimer(JNIEnv* env, jclass clazz, jobject object, cl_lede_led_timer_t *timer)
{
	jniCopyIntValue(env,clazz,"id",object,timer->id);
	jniCopyBooleanValue(env,clazz,"enable",object,timer->flags);
	jniCopyIntValue(env,clazz,"week",object,timer->week_loop);
	jniCopyIntValue(env,clazz,"hour",object,timer->hour);
	jniCopyIntValue(env,clazz,"minute",object,timer->min);

	copyLampState(env, clazz, object, &timer->config);
}

void copy_lede_lamp(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass clazz_state = NULL;
	jclass clazz_timer = NULL;
	jclass class_on_stat = NULL;
	jobject object = NULL;
	jobject object_lamp_state  = NULL;
	jobject object_on_state  = NULL;
	jobject object_arr  = NULL;
	jfieldID fid;
	int i = 0;

	cl_lede_led_state_t lamp_state;
	cl_lede_led_on_stat_t on_stat;
	cl_lede_led_timer_t *lamp_timer_p;
	cl_lede_lamp_info *lede_lamp_p = (cl_lede_lamp_info*)udp_dev;

	if (!lede_lamp_p) {

		return;
	} else {
		lamp_timer_p = lede_lamp_p->timer_info;
		lamp_state = lede_lamp_p->cur_lamp_stat;
		on_stat = lede_lamp_p->on_stat;
	}

	clazz_state = (*env)->FindClass(env, CLASS_LEDE_LAMP_INFO);
	if (!clazz_state) {

		goto quit;
	}
	
	clazz_timer = (*env)->FindClass(env, CLASS_LEDE_LAMP_TIMER);
	if (!clazz_timer) {
		goto quit;
	}

	object_lamp_state = (*env)->AllocObject(env, clazz_state);
	if (!object_lamp_state) {
		goto quit;
	}

	class_on_stat = (*env)->FindClass(env, CLASS_LEDE_ON_STAT);
	if (!class_on_stat) {
		goto quit;
	}
	object_on_state = (*env)->AllocObject(env, class_on_stat);
	if (!object_on_state) {
		goto quit;
	}
	copyLampState(env, clazz_state, object_lamp_state, &lamp_state);

	jniCopyBooleanValue(env, class_on_stat, "valid", object_on_state, on_stat.valid);
	jniCopyBooleanValue(env, class_on_stat, "enable", object_on_state, on_stat.enable);
	jniCopyByteValue(env, class_on_stat, "type", object_on_state, on_stat.type);
	jniCopyIntValue(env, class_on_stat, "r", object_on_state, on_stat.stat.R);
	jniCopyIntValue(env, class_on_stat, "g", object_on_state, on_stat.stat.G);
	jniCopyIntValue(env, class_on_stat, "b", object_on_state, on_stat.stat.B);
	jniCopyIntValue(env, class_on_stat, "l", object_on_state, on_stat.stat.L);
	jniCopyIntValue(env, class_on_stat, "cold", object_on_state, on_stat.stat.cold);
	jniCopyIntValue(env, class_on_stat, "modeId", object_on_state, on_stat.stat.mod_id);
	jniCopyIntValue(env, class_on_stat, "action", object_on_state, on_stat.stat.action);
	
	fid = (*env)->GetFieldID(env, clazz_state, "on_stat", "L" CLASS_LEDE_ON_STAT ";");
	(*env)->SetObjectField(env, object_lamp_state, fid, object_on_state);
	
	if (lede_lamp_p->timer_count > 0) {
		object_arr = (*env)->NewObjectArray(env, lede_lamp_p->timer_count, clazz_timer, NULL);
		for (i = 0; i < lede_lamp_p->timer_count; i++, lamp_timer_p++) {
			object = (*env)->AllocObject(env, clazz_timer);
			copyLampTimer(env, clazz_timer, object, lamp_timer_p);
			(*env)->SetObjectArrayElement(env, object_arr, i, object);	
			SAFE_DEL_LOCAL_REF(object);
		}

		fid = (*env)->GetFieldID(env, clazz_state, "lampTimers", "[L" CLASS_LEDE_LAMP_TIMER ";");
		(*env)->SetObjectField(env, object_lamp_state, fid, object_arr);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_lamp_state);
quit:
	SAFE_DEL_LOCAL_REF(clazz_state);
	SAFE_DEL_LOCAL_REF(clazz_timer);
	SAFE_DEL_LOCAL_REF(object_lamp_state);
	SAFE_DEL_LOCAL_REF(object_arr);
}

void copy_amt_fan(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_amtfan = NULL;
	jobject object_amtfan = NULL;
	jfieldID fid;
	cl_amt_fan_device_t *amt_dev = (cl_amt_fan_device_t*)udp_dev;

	class_amtfan = (*env)->FindClass(env, CLASS_AMT_INFO);
	object_amtfan = (*env)->AllocObject(env, class_amtfan);

	jni_copy_simple_class(env, class_amtfan, object_amtfan,  
		                       TRIPLES(boolean, amt_dev, onoff),
		                       TRIPLES(byte, amt_dev, cur_mode),
		                       TRIPLES(byte, amt_dev, cur_gear),
		                       TRIPLES(boolean, amt_dev, screen_light),
		                       TRIPLES(boolean, amt_dev, is_shake),
		                       TRIPLES(boolean, amt_dev, is_anion_on),
		                       TRIPLES(boolean, amt_dev, is_plasma_on),
					    TRIPLES(int, amt_dev, cur_power),
		                       TRIPLES(byte, amt_dev, cur_temp),
		                       QUADRUPLE(byte[], amt_dev, cur_user_define_mode, MAX_USER_DEFINE_MODE_POINT),
							   TRIPLES(boolean, amt_dev, is_timer_on_valid),
							   TRIPLES(boolean, amt_dev, is_timer_off_valid),
							   TRIPLES(int, amt_dev, dev_on_remain_time),
							   TRIPLES(int, amt_dev, dev_off_remain_time),
		                       JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_amtfan);
	
	SAFE_DEL_LOCAL_REF(class_amtfan);
	SAFE_DEL_LOCAL_REF(object_amtfan);
}

static void copy_bimar_heater_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev) 
{
	jclass class_bimar = NULL;
	jobject object_bimar = NULL;
	jfieldID fid;
	cl_bimar_heater_info_t *bimar_dev = (cl_bimar_heater_info_t*)udp_dev;

	if (bimar_dev == NULL) {
		return;
	}

	class_bimar = (*env)->FindClass(env, CLASS_BIMAR_HEATER);
	object_bimar = (*env)->AllocObject(env, class_bimar);

	jni_copy_simple_class(env, class_bimar, object_bimar,
		                    TRIPLES(boolean, bimar_dev, is_data_valid),
		                    TRIPLES(boolean, bimar_dev, is_on),
		                    TRIPLES(boolean, bimar_dev, is_shake),
		                    TRIPLES(boolean, bimar_dev, is_anion_enable),
		                    TRIPLES(byte, bimar_dev, fan_speed),
		                    TRIPLES(byte, bimar_dev, temp_unit),
		                    TRIPLES(byte, bimar_dev, power_gear),
		                    TRIPLES(byte, bimar_dev, cur_set_cels_temp),
		                    TRIPLES(byte, bimar_dev, cur_set_fahr_temp),
		                    TRIPLES(byte, bimar_dev, cur_set_timer),
		                    TRIPLES(byte, bimar_dev, cur_remain_min),
		                    TRIPLES(byte, bimar_dev, cels_room_temp),
		                    TRIPLES(byte, bimar_dev, fahr_room_temp),
		                    TRIPLES(byte, bimar_dev, machine_type),
		                    TRIPLES(int, bimar_dev, loc_update_gnu_time),
		                    TRIPLES(byte, bimar_dev, scm_run_mode),
		                    JNI_VAR_ARG_END);
	

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_bimar);
	
	SAFE_DEL_LOCAL_REF(class_bimar);
	SAFE_DEL_LOCAL_REF(object_bimar);
	
}

int copy_tb_commmercial_config(JNIEnv* env, jclass class_tb_comm, jobject obj_tb_comm, cl_tbb_config_set_t *config) 
{
	jclass class_config = NULL;
	jclass obj_config = NULL;
	jfieldID fid = NULL;
	int temp_uint = 0;

	LOGD("xxxddd config cool %hu, heat %hu, auto %hu\n", config->auto_tmp.cool_tmp, config->auto_tmp.heat_tmp, config->auto_tmp.auto_tmp);

	class_config = (*env)->FindClass(env, CLASS_TB_COMM_CONFIG);
	obj_config = (*env)->AllocObject(env, class_config);

	JNI_COPY_SIMPLE_CLASS(env, class_config, obj_config,
		                     CLASS_TB_COMM_DEFROST, defrost, 
		                     TRIPLES(short, &config->defrost, in_defrost_time),
		                     TRIPLES(short, &config->defrost, in_defrost_tmp),
		                     TRIPLES(short, &config->defrost, out_defrost_tmp),
		                     TRIPLES(short, &config->defrost, out_defrost_time),
		                     TRIPLES(short, &config->defrost, env_tmp),
		                     TRIPLES(short, &config->defrost, dif),
		                     TRIPLES(short, &config->defrost, defrost_continue_time),
		                     JNI_VAR_ARG_END);

	temp_uint = ((config->misc.work & 0x0400) >> 10) ;
	JNI_COPY_SIMPLE_CLASS(env, class_config, obj_config,
		                     CLASS_TB_COMM_MISC, misc, 
		                     TRIPLES(byte, &config->misc, sys_num),
		                     TRIPLES(byte, &config->misc, sys_select),
		                     TRIPLES(short, &config->misc, work),
		                     TRIPLES(short, &config->misc, heat_pump_diff),
		                     TRIPLES(short, &config->misc, cool_diff),
		                     TRIPLES(short, &config->misc, hot_diff),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_config, obj_config,
		                     CLASS_TB_COMM_PROTECT, protect, 
		                     TRIPLES(short, &config->protect, dst_tmp_pro),
		                     TRIPLES(short, &config->protect, cool_out_water),
		                     TRIPLES(short, &config->protect, heat_out_water),
		                     TRIPLES(short, &config->protect, in_out_tmp),
		                     TRIPLES(short, &config->protect, pump_begin_time),
		                     TRIPLES(short, &config->protect, pump_delay_time),
		                     TRIPLES(short, &config->protect, wind_ordor_tmp),
		                     TRIPLES(short, &config->protect, env_tmp),
		                     TRIPLES(short, &config->protect, in_water_tmp),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_config, obj_config,
		                     CLASS_TB_COMM_EEV, eev, 
		                     TRIPLES(short, &config->eev, ele_cycle),
		                     TRIPLES(short, &config->eev, hand_ctrl_step),
		                     TRIPLES(short, &config->eev, cool_ele_valve),
		                     TRIPLES(short, &config->eev, limit_day),
		                     JNI_VAR_ARG_END);


	JNI_COPY_SIMPLE_CLASS(env, class_config, obj_config,
		                     CLASS_TB_COMM_CONF_TEMP, auto_tmp, 
		                     TRIPLES(short, &config->auto_tmp, cool_tmp),
		                     TRIPLES(short, &config->auto_tmp, heat_tmp),
		                     TRIPLES(short, &config->auto_tmp, auto_tmp),
		                     JNI_VAR_ARG_END);

	jni_copy_simple_class(env, class_config, obj_config,
		                    TRIPLES(short ,config, bottom_ele_heat_tmp),
		                    WRAP_QUOTE(byte),temp_uint,WRAP_QUOTE(org_type_tmp),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_tb_comm, "config", "L" CLASS_TB_COMM_CONFIG ";");
	(*env)->SetObjectField(env, obj_tb_comm, fid, obj_config);
	
	SAFE_DEL_LOCAL_REF(class_config);
	SAFE_DEL_LOCAL_REF(obj_config);
}

int copy_tb_commercial_stat(JNIEnv* env, jclass class_tb_comm, jobject obj_tb_comm, cl_tbb_status_t *stat) 
{
	jclass class_stat = NULL;
	jclass obj_stat = NULL;
	jfieldID fid = NULL;

	class_stat = (*env)->FindClass(env, CLASS_TB_COMM_STAT);
	obj_stat = (*env)->AllocObject(env, class_stat);
	jni_copy_simple_class(env, class_stat, obj_stat,
		                     TRIPLES(short ,stat, water_box_tmp),
		                     TRIPLES(short ,stat, in_water_tmp),
		                     TRIPLES(short ,stat, out_water_tmp),
		                     TRIPLES(short ,stat, env_tmp),
		                     TRIPLES(short ,stat, back_water_tmp),
		                     TRIPLES(short ,stat, support_mode),
		                     QUADRUPLE(short[], stat, ele_valve, 4),
	                         TRIPLES(short ,stat, pump_info),
		                     TRIPLES(short ,stat, set_on),
		                     TRIPLES(short ,stat, set_on1),
		                     TRIPLES(short ,stat, set_on2),
		                     TRIPLES(short ,stat, slave_status),
		                     TRIPLES(short ,stat, fault1),
		                     TRIPLES(short ,stat, fault2),
		                     TRIPLES(short ,stat, fault3),
		                     TRIPLES(short ,stat, sys_run_days),
		                     JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_stat, obj_stat, CLASS_TB_COMM_SCROLL,
		                   scroll_tmp, 4, sizeof(cl_tbb_scroll_t),
		                   ARRAY_TRIPLES(short, stat->scroll_tmp, scroll_tmp),
		                   ARRAY_TRIPLES(short, stat->scroll_tmp, out_air_tmp),
		                   ARRAY_TRIPLES(short, stat->scroll_tmp, back_air_tmp),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_tb_comm, "status", "L" CLASS_TB_COMM_STAT ";");
	(*env)->SetObjectField(env, obj_tb_comm, fid, obj_stat);
	
	SAFE_DEL_LOCAL_REF(class_stat);
	SAFE_DEL_LOCAL_REF(obj_stat);
}

static void copy_tb_commercial_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_tb_commercial = NULL;
	jobject obj_tb_commercial = NULL;
	jfieldID fid = NULL;
	cl_tbb_info_t *tb_commercial_info = (cl_tbb_info_t *)udp_dev;

	if (tb_commercial_info == NULL) {
		return;
	}

	class_tb_commercial = (*env)->FindClass(env, CLASS_TB_COMM_INFO);
	obj_tb_commercial = (*env)->AllocObject(env, class_tb_commercial);

	jni_copy_simple_class(env,class_tb_commercial,obj_tb_commercial,
		                    TRIPLES(boolean ,tb_commercial_info, on),
		                    TRIPLES(byte ,tb_commercial_info, mode),
		                    TRIPLES(short ,tb_commercial_info, tmp),
		                    JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env, class_tb_commercial, obj_tb_commercial,
		                     CLASS_TB_COMM_UPGRADE, upgrade_info,
		                     TRIPLES(int ,&tb_commercial_info->upgrade_info, upgradeing),
		                     TRIPLES(int ,&tb_commercial_info->upgrade_info, upgrade_role),
		                     TRIPLES(int ,&tb_commercial_info->upgrade_info, upgrade_state),
		                     TRIPLES(int ,&tb_commercial_info->upgrade_info, up_state),
		                     JNI_VAR_ARG_END);

	copy_tb_commercial_stat(env, class_tb_commercial, obj_tb_commercial, &tb_commercial_info->status);	
	JNI_COPY_SIMPLE_CLASS(env, class_tb_commercial, obj_tb_commercial,
		                     CLASS_TB_COMM_VER, hd_ver, 
		                     TRIPLES(short, &tb_commercial_info->hd_ver, sys_type),
		                     TRIPLES(short, &tb_commercial_info->hd_ver, ele_band_mcu),
		                     TRIPLES(short, &tb_commercial_info->hd_ver, ele_band_ver),
		                     TRIPLES(short, &tb_commercial_info->hd_ver, line_band_mcu),
		                     TRIPLES(short, &tb_commercial_info->hd_ver, line_band_ver),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_tb_commercial, obj_tb_commercial,
		                     CLASS_TB_COMM_BIND, bindinfo,
		                     TRIPLES(short ,&tb_commercial_info->bindinfo, dev_state),
		                     TRIPLES(short ,&tb_commercial_info->bindinfo, bind_state),
		                     TRIPLES(String ,&tb_commercial_info->bindinfo, tb_sn),
		                     JNI_VAR_ARG_END);

	copy_tb_commmercial_config(env, class_tb_commercial, obj_tb_commercial, &tb_commercial_info->config);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_tb_commercial);
	
	SAFE_DEL_LOCAL_REF(class_tb_commercial);
	SAFE_DEL_LOCAL_REF(obj_tb_commercial);
}

static void copy_hx_pot(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_pot = NULL;
	jobject obj_pot = NULL;
	jfieldID fid = NULL;
	cl_hx_ysh_info_t *hx_pot = (cl_hx_ysh_info_t *)udp_dev;

	if (hx_pot == NULL) {
		return;
	}

	class_pot = (*env)->FindClass(env, CLASS_HX_POT_INFO);
	obj_pot = (*env)->AllocObject(env, class_pot);

	jniCopyIntValue(env, class_pot, "scene_num", obj_pot, hx_pot->scene_num);

	JNI_COPY_SIMPLE_CLASS(env, class_pot, obj_pot, CLASS_HX_POT_STAT, stat,
		                     TRIPLES(boolean ,&hx_pot->stat, is_data_valid),
		                     TRIPLES(boolean ,&hx_pot->stat, on_off),
		                     TRIPLES(short ,&hx_pot->stat, work_remain_time),
		                     TRIPLES(short ,&hx_pot->stat, cur_exec_id),
		                     TRIPLES(byte ,&hx_pot->stat, work_stat),
		                     TRIPLES(byte ,&hx_pot->stat, cur_power),
		                     TRIPLES(byte ,&hx_pot->stat, temp),
		                     TRIPLES(short ,&hx_pot->stat, mcu_timer),
		                     TRIPLES(byte ,&hx_pot->stat, is_hot),
		                     TRIPLES(byte ,&hx_pot->stat, err_no),
		                     TRIPLES(short ,&hx_pot->stat, wifi_timer_exec_id),
		                     TRIPLES(short ,&hx_pot->stat, wifi_timer),
		                     //TRIPLES(int ,&hx_pot->stat, update_time),
		                     JNI_VAR_ARG_END);

	if (hx_pot->scene_num > 0 && hx_pot->scene != NULL) {
		JNI_COPY_ARRAY_CLASS(env, class_pot, obj_pot, CLASS_HX_POT_SCENE,
			                    scene, hx_pot->scene_num, sizeof(cl_hx_ysh_scene_t),
			                    ARRAY_TRIPLES(byte, hx_pot->scene, temp),
			                    ARRAY_TRIPLES(byte, hx_pot->scene, time),
			                    ARRAY_TRIPLES(byte, hx_pot->scene, power),
			                    ARRAY_TRIPLES(byte, hx_pot->scene, keep_temp),
			                    ARRAY_TRIPLES(byte, hx_pot->scene, keep_time),
			                    ARRAY_TRIPLES(short, hx_pot->scene, scene_id),
			                    ARRAY_TRIPLES(int, hx_pot->scene, create_time),
			                    ARRAY_TRIPLES(String, hx_pot->scene, name),
			                    JNI_VAR_ARG_END);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_pot);
	
	SAFE_DEL_LOCAL_REF(class_pot);
	SAFE_DEL_LOCAL_REF(obj_pot);
}
	
static void copy_yt_airplug(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_yt = NULL;
	jobject obj_yt = NULL;
	jfieldID fid = NULL;
	cl_yt_info_t *yt = (cl_yt_info_t *)udp_dev;

	if (yt == NULL) {
		return ;
	}
	class_yt = (*env)->FindClass(env, CLASS_YT_AIRPLUG_INFO);
	obj_yt = (*env)->AllocObject(env, class_yt);


	jni_copy_simple_class(env, class_yt, obj_yt,
		                     TRIPLES(byte, yt,onoff),
		                     TRIPLES(byte, yt,mode),
		                     TRIPLES(byte, yt,tmp),
		                     TRIPLES(byte, yt,wind_speed),
		                     TRIPLES(byte, yt,wind_dir),
		                     TRIPLES(byte, yt,ele_assist),
		                     TRIPLES(byte, yt,sleep),
		                     TRIPLES(byte, yt,swing),
		                     TRIPLES(byte, yt,room_tmp),
		                     TRIPLES(byte, yt,extern_tmp),
							 TRIPLES(byte, yt,sn_err),

	
		                     TRIPLES(byte, yt,compressor_onoff),
		                     TRIPLES(byte, yt,extern_wind_onoff),
		                     TRIPLES(byte, yt,four_valve),
		                     TRIPLES(byte, yt,ele_hot),
		                     TRIPLES(short, yt,compressor_work_hours),
		                     TRIPLES(short, yt,assit_work_hours),
		                    
		                     TRIPLES(short, yt,wind_real_spee),
		                     TRIPLES(byte, yt,inner_tmp),
		                     TRIPLES(short, yt,dc_busway_val),
		                     TRIPLES(short, yt,extern_ac_val),
		                     TRIPLES(short, yt,extern_ac_cur),
		                     TRIPLES(short, yt,compressor_cur),
		                     TRIPLES(short, yt,compressor_freq),
		                     TRIPLES(byte, yt,outside_tmp),
		                     TRIPLES(byte, yt,exhaust_tmp),
		                     TRIPLES(byte, yt,ipm_tmp),
		                     TRIPLES(byte, yt,heat_defrost),
							 TRIPLES(byte, yt,sys_type),
		                     TRIPLES(byte, yt,fault_b2),
		                     TRIPLES(byte, yt,fault_b12),
		                     TRIPLES(byte, yt,protect_b13),
		                     TRIPLES(byte, yt,protect_b14),
		                     TRIPLES(int, yt,ele_total),
		                     TRIPLES(byte, yt, in_fans_gears),
		                     TRIPLES(byte, yt, protect_b18),
							 TRIPLES(String, yt, name),

		                     JNI_VAR_ARG_END);
	
	JNI_COPY_SIMPLE_CLASS(env, class_yt, obj_yt, CLASS_YT_TIMER, timer,
		                     TRIPLES(byte, &yt->timer,on_enable),
		                     TRIPLES(short, &yt->timer,on_remain_min),
		                     TRIPLES(byte, &yt->timer,off_enable),
		                     TRIPLES(short, &yt->timer,off_remain_min),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_yt, obj_yt, CLASS_YT_AC_TYPE, ac_info,
		                     TRIPLES(short, &yt->ac_info, index),
							 TRIPLES(short, &yt->ac_info, ele_assist_power),
		                     TRIPLES(byte, &yt->ac_info, freq_type),
		                     TRIPLES(byte, &yt->ac_info, cool_type),
		                     TRIPLES(byte, &yt->ac_info, rl_swing),
		                     TRIPLES(short, &yt->ac_info, cool_power),
		                     TRIPLES(short, &yt->ac_info, hot_power),
		                     TRIPLES(long, &yt->ac_info, sn),
		                     JNI_VAR_ARG_END);


	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_yt);
	
	SAFE_DEL_LOCAL_REF(class_yt);
	SAFE_DEL_LOCAL_REF(obj_yt);
}

static void copy_jl_stage_lamp(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_jl = NULL;
	jobject obj_jl = NULL;
	jfieldID fid = NULL;
	cl_jl_lamp_info_t *jl = (cl_jl_lamp_info_t *)udp_dev;

	if (jl == NULL) {
		return ;
	}
	class_jl = (*env)->FindClass(env, CLASS_JL_INFO);
	obj_jl = (*env)->AllocObject(env, class_jl);

	jni_copy_simple_class(env, class_jl, obj_jl,
		                     TRIPLES(byte, jl, lamp_type),
		                     TRIPLES(boolean, jl, on_off),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_jl, obj_jl, CLASS_JL_3200_LAMP, lamp_3200_info,
		                     TRIPLES(short, &jl->lamp_3200_info, color),
		                     TRIPLES(short, &jl->lamp_3200_info, bright),
		                     TRIPLES(short, &jl->lamp_3200_info, total_bright),
		                     JNI_VAR_ARG_END);

	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_jl);
	
	SAFE_DEL_LOCAL_REF(class_jl);
	SAFE_DEL_LOCAL_REF(obj_jl);
}


static void copy_ads_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_ads = NULL;
	jobject obj_ads = NULL;
	jfieldID fid = NULL;
	cl_ads_info_t *ads_info = (cl_ads_info_t *)udp_dev;

	if (ads_info == NULL) {
		return;
	}

	class_ads = (*env)->FindClass(env, CLASS_ADS_INFO);
	obj_ads = (*env)->AllocObject(env, class_ads);
	
	jni_copy_simple_class(env, class_ads, obj_ads,
		                     TRIPLES(boolean, ads_info, on),
		                     TRIPLES(boolean, ads_info, back_water_on),
		                     TRIPLES(short, ads_info, water_box_show_tmp),
		                     TRIPLES(short, ads_info, water_box_ctrl_tmp),
		                     TRIPLES(short, ads_info, out_water_tmp),
		                     TRIPLES(short, ads_info, env_tmp),
		                     TRIPLES(short, ads_info, back_diff_tmp),
		                     TRIPLES(short, ads_info, compressor_cur),
		                     TRIPLES(short, ads_info, scroll_tmp),
		                     TRIPLES(short, ads_info, exhaust_tmp),
		                     TRIPLES(byte, ads_info, defrost),
		                     TRIPLES(byte, ads_info, wind),
		                     TRIPLES(byte, ads_info, fault_code),
		                     TRIPLES(short, ads_info, pump_clock),
		                     TRIPLES(byte, ads_info, fault_code2),
		                     TRIPLES(short, ads_info, water_show_tmp),
		                     JNI_VAR_ARG_END);
	
	JNI_COPY_SIMPLE_CLASS(env, class_ads, obj_ads, CLASS_ADS_CONFIG, conf,
	                         TRIPLES(short, &ads_info->conf, sys_clock_time),
		                     TRIPLES(short, &ads_info->conf, first_start_time),
		                     TRIPLES(short, &ads_info->conf, first_end_time),
		                     TRIPLES(short, &ads_info->conf, second_start_time),
		                     TRIPLES(short, &ads_info->conf, second_end_time),
		                     TRIPLES(short, &ads_info->conf, defrost_time),
		                     TRIPLES(short, &ads_info->conf, defrost_tmp),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_ads);
	
	SAFE_DEL_LOCAL_REF(class_ads);
	SAFE_DEL_LOCAL_REF(obj_ads);
}

static void copy_js_wave_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_js = NULL;
	jobject obj_js = NULL;
	jfieldID fid = NULL;
	cl_js_wave_info_t *js_info = (cl_js_wave_info_t *)udp_dev;

	if (js_info == NULL) {
		return;
	}

	class_js = (*env)->FindClass(env, CLASS_JS_WAVE_OVEN);
	obj_js = (*env)->AllocObject(env, class_js);

	jniCopyBooleanValue(env, class_js, "is_data_valid", obj_js, js_info->stat.is_data_valid);
	jniCopyBooleanValue(env, class_js, "is_waiting", obj_js, js_info->stat.is_waiting);
	jniCopyBooleanValue(env, class_js, "is_working", obj_js, js_info->stat.is_working);
	jniCopyBooleanValue(env, class_js, "is_pausing", obj_js, js_info->stat.is_pausing);
	jniCopyBooleanValue(env, class_js, "child_lock_onoff", obj_js, js_info->stat.child_lock_onoff);
	jniCopyBooleanValue(env, class_js, "is_door_open", obj_js, js_info->stat.is_door_open);
	jniCopyBooleanValue(env, class_js, "is_fault_stat", obj_js, js_info->stat.is_fault_stat);
	jniCopyBooleanValue(env, class_js, "is_chain_stat", obj_js, js_info->stat.is_chain_stat);
	jniCopyByteValue(env, class_js, "work_mode", obj_js, js_info->stat.work_mode);
	jniCopyByteValue(env, class_js, "work_sub_mode", obj_js, js_info->stat.work_sub_mode);
	jniCopyByteValue(env, class_js, "setting_min", obj_js, js_info->stat.setting_min);
	jniCopyByteValue(env, class_js, "setting_sec", obj_js, js_info->stat.setting_sec);
	jniCopyByteValue(env, class_js, "wave_fire", obj_js, js_info->stat.wave_fire);
	jniCopyByteValue(env, class_js, "barbecue_fire", obj_js, js_info->stat.barbecue_fire);
	jniCopyByteValue(env, class_js, "hot_fan_temp", obj_js, js_info->stat.hot_fan_temp);
	jniCopyShortValue(env, class_js, "food_weight", obj_js, js_info->stat.food_weight);
	jniCopyByteValue(env, class_js, "cur_temp", obj_js, js_info->stat.cur_temp);
	jniCopyByteValue(env, class_js, "remain_min", obj_js, js_info->stat.remain_min);
	jniCopyByteValue(env, class_js, "remain_sec", obj_js, js_info->stat.remain_sec);
	jniCopyIntValue(env, class_js, "local_refresh_time", obj_js, js_info->stat.local_refresh_time);
	

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_js);
	
	SAFE_DEL_LOCAL_REF(class_js);
	SAFE_DEL_LOCAL_REF(obj_js);
}

static jobject copy_kxm_stat_item(JNIEnv* env, cl_kxm_info_t *kxm_info, int index)
{
	jclass class_kxm_stat = NULL;
	jobject obj_kxm_stat = NULL;
	jfieldID fid = NULL;
	cl_kxm_pump_stat_info_t *pump = &kxm_info->pump[index];

	class_kxm_stat = (*env)->FindClass(env, CLASS_KXM_STAT);
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

	JNI_COPY_ARRAY_CLASS(env, class_kxm_stat, obj_kxm_stat, CLASS_KXM_SUB_STAT,
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

static void copy_kxm_stat(JNIEnv* env, jclass class_kxm, jobject obj_kxm, cl_kxm_info_t *kxm_info)
{
	jclass class_kxm_stat = NULL;
	jobject obj_array = NULL;
	jobject obj_kxm_stat = NULL;
	jfieldID fid = NULL;
	int i = 0;

	class_kxm_stat = (*env)->FindClass(env, CLASS_KXM_STAT);
	obj_array = (*env)->NewObjectArray(env, MAX_KXM_SUB_PUMP_NUM, class_kxm_stat, NULL);

	for (i = 0; i < MAX_KXM_SUB_PUMP_NUM; ++i) {
		obj_kxm_stat = copy_kxm_stat_item(env, kxm_info, i);
		(*env)->SetObjectArrayElement(env, obj_array, i, obj_kxm_stat);
		SAFE_DEL_LOCAL_REF(obj_kxm_stat);
	}

	fid = (*env)->GetFieldID(env, class_kxm,  "pump", "[L"CLASS_KXM_STAT";");
	(*env)->SetObjectField(env, obj_kxm, fid, obj_array);
	
	SAFE_DEL_LOCAL_REF(class_kxm_stat);
	SAFE_DEL_LOCAL_REF(obj_array);
}


static void copy_kxm_host(JNIEnv* env, jclass class_kxm, jobject obj_kxm, cl_kxm_info_t *kxm_info)
{
	jclass class_kxm_host = NULL;
	jobject obj_kxm_host = NULL;
	jfieldID fid = NULL;

	class_kxm_host = (*env)->FindClass(env, CLASS_KXM_HOST);
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

	JNI_COPY_ARRAY_CLASS(env, class_kxm_host, obj_kxm_host, CLASS_KXM_TIMER,
		                   timer, MAX_KXM_TIMER_CNT, sizeof(cl_kxm_timer_info_t),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, on_hour),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, on_min),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, off_hour),
		                   ARRAY_TRIPLES(byte, kxm_info->hinfo.timer, off_min),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_kxm, "hinfo", "L" CLASS_KXM_HOST ";");
	(*env)->SetObjectField(env, obj_kxm, fid, obj_kxm_host);
	
	SAFE_DEL_LOCAL_REF(class_kxm_host);
	SAFE_DEL_LOCAL_REF(obj_kxm_host);
}

static void copy_sbt_smart_day(JNIEnv* env, jclass class_sbt_smart, jobject obj_sbt_smart, char *mem, cl_sbt_smart_day_t *day_info)
{
	jclass class_day = NULL;
	jobject obj_day = NULL;
	jfieldID fid = NULL;

	class_day = (*env)->FindClass(env, CLASS_SBT_SMART_DAY);
	obj_day = (*env)->AllocObject(env, class_day);
	JNI_COPY_ARRAY_CLASS(env, class_day, obj_day,CLASS_SBT_SMART_ITEM,
		                   si, 4, sizeof(cl_sbt_smart_item_t),
		                   ARRAY_TRIPLES(byte, day_info->si, hour),
		                   ARRAY_TRIPLES(byte, day_info->si, temp),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_sbt_smart, mem, "L" CLASS_SBT_SMART_DAY ";");
	(*env)->SetObjectField(env, obj_sbt_smart, fid, obj_day);
	
	SAFE_DEL_LOCAL_REF(class_day);
	SAFE_DEL_LOCAL_REF(obj_day);
}

static void copy_sbt_smart_info(JNIEnv* env, jclass class_sbt_info, jobject obj_sbt_info, cl_smart_smart_ctrl_t *smart_info)
{
	jclass class_sbt_smart = NULL;
	jobject obj_sbt_smart = NULL;
	jfieldID fid = NULL;

	class_sbt_smart = (*env)->FindClass(env, CLASS_SBT_SMART);
	obj_sbt_smart = (*env)->AllocObject(env, class_sbt_smart);

	copy_sbt_smart_day(env, class_sbt_smart, obj_sbt_smart, "work_day", &smart_info->work_day);
	copy_sbt_smart_day(env, class_sbt_smart, obj_sbt_smart, "sat_day", &smart_info->sat_day);
	copy_sbt_smart_day(env, class_sbt_smart, obj_sbt_smart, "sun_day", &smart_info->sun_day);

	fid = (*env)->GetFieldID(env, class_sbt_info, "smart_info", "L" CLASS_SBT_SMART ";");
	(*env)->SetObjectField(env, obj_sbt_info, fid, obj_sbt_smart);
	
	SAFE_DEL_LOCAL_REF(class_sbt_smart);
	SAFE_DEL_LOCAL_REF(obj_sbt_smart);
}

static void copy_zssx_oven_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_oven = NULL;
	jobject obj_oven = NULL;
	jfieldID fid = NULL;
	cl_zssx_info_t *oven_info = (cl_zssx_info_t*)udp_dev;

	if (oven_info == NULL) {
		return;
	}

	class_oven = (*env)->FindClass(env, CLASS_ZSSX_OVEN);
	obj_oven = (*env)->AllocObject(env, class_oven);

	jni_copy_simple_class(env, class_oven, obj_oven,
		                     TRIPLES(boolean, oven_info, power_onoff),
		                     TRIPLES(boolean, oven_info, lock_onoff),
		                     TRIPLES(boolean, oven_info, anion_onoff),
		                     TRIPLES(byte, oven_info, tmp_type),
		                     TRIPLES(boolean, oven_info, led_onoff),
		                     TRIPLES(boolean, oven_info, tmp_onoff),
		                     TRIPLES(byte, oven_info, fire_level),
		                     TRIPLES(byte, oven_info, fake_firewood),
		                     TRIPLES(byte, oven_info, speed_gears),
		                     TRIPLES(boolean, oven_info, timer_on),
		                     TRIPLES(boolean, oven_info, timer_off),
		                     TRIPLES(byte, oven_info, timer_hour),
		                     TRIPLES(byte, oven_info, timer_min),
		                     TRIPLES(short, oven_info, tmp),
		                     TRIPLES(byte, oven_info, ntc_fault),
		                     TRIPLES(byte, oven_info, thermostat_fault),
		                     TRIPLES(byte, oven_info, work_status),
		                     TRIPLES(byte, oven_info, wifi_mode),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_oven, obj_oven,CLASS_ZSSX_WIFI_CONF, wifi_conf,
		                     TRIPLES(String, &oven_info->wifi_conf, ssid),
		                     TRIPLES(String, &oven_info->wifi_conf, pswd),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_oven);
	
	SAFE_DEL_LOCAL_REF(class_oven);
	SAFE_DEL_LOCAL_REF(obj_oven);
}

static void copy_sbt_thermostat_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_sbt = NULL;
	jobject obj_sbt = NULL;
	jfieldID fid = NULL;
	cl_sbt_ther_info_t *sbt = (cl_sbt_ther_info_t*)udp_dev;

	if (sbt == NULL) {
		return;
	}

	class_sbt = (*env)->FindClass(env, CLASS_SBT_INFO);
	obj_sbt = (*env)->AllocObject(env, class_sbt);
	
	jni_copy_simple_class(env, class_sbt, obj_sbt,
		                     TRIPLES(boolean, sbt, is_data_valid),
		                     TRIPLES(boolean, sbt, onoff),
		                     TRIPLES(byte, sbt, temp),
		                     TRIPLES(byte, sbt, mode),
		                     TRIPLES(byte, sbt, fan_speed),
		                     TRIPLES(byte, sbt, auto_mode),
		                     TRIPLES(byte, sbt, temp_adjust),
		                     TRIPLES(byte, sbt, low_temp),
		                     TRIPLES(byte, sbt, valve_mode),
		                     TRIPLES(byte, sbt, return_temp),
		                     TRIPLES(boolean, sbt, is_low_temp_guard),
		                     TRIPLES(byte, sbt, max_temp),
		                     TRIPLES(byte, sbt, min_temp),
		                     TRIPLES(short, sbt, room_temp),
		                     TRIPLES(byte, sbt, scm_hour),
		                     TRIPLES(byte, sbt, scm_min),
		                     TRIPLES(byte, sbt, scm_sec),
		                     TRIPLES(byte, sbt, scm_weekday),
		                     JNI_VAR_ARG_END);
	copy_sbt_smart_info(env, class_sbt, obj_sbt, &sbt->smart_info);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_sbt);
	
	SAFE_DEL_LOCAL_REF(class_sbt);
	SAFE_DEL_LOCAL_REF(obj_sbt);
}


static void copy_kxm_thermost_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_kxm = NULL;
	jobject obj_kxm = NULL;
	jfieldID fid = NULL;
	cl_kxm_thermost_info_t *kxm_info = (cl_kxm_thermost_info_t *)udp_dev;

	if (kxm_info == NULL) {
		return;
	}

	class_kxm = (*env)->FindClass(env, CLASS_KXM_THERMOST);
	obj_kxm = (*env)->AllocObject(env, class_kxm);

	jni_copy_simple_class(env, class_kxm, obj_kxm,
		                     TRIPLES(boolean, kxm_info,onoff),
		                     TRIPLES(byte, kxm_info,mode),
		                     TRIPLES(byte, kxm_info,setting_temp),
		                     TRIPLES(byte, kxm_info,room_temp),
		                     TRIPLES(byte, kxm_info,fan_speed),
		                     TRIPLES(boolean, kxm_info,energy_cons),
		                     JNI_VAR_ARG_END);


	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_kxm);
	
	SAFE_DEL_LOCAL_REF(class_kxm);
	SAFE_DEL_LOCAL_REF(obj_kxm);
}

static void copy_kxm_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_kxm = NULL;
	jobject obj_kxm = NULL;
	jclass class_kxm_host = NULL;
	jobject obj_kxm_host = NULL;
	jfieldID fid = NULL;
	cl_kxm_info_t *kxm_info = (cl_kxm_info_t *)udp_dev;

	if (kxm_info == NULL) {
		return;
	}

	class_kxm = (*env)->FindClass(env, CLASS_KXM_INFO);
	obj_kxm = (*env)->AllocObject(env, class_kxm);

	copy_kxm_stat(env, class_kxm, obj_kxm, kxm_info);
	copy_kxm_host(env, class_kxm, obj_kxm, kxm_info);
	

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_kxm);
	
	SAFE_DEL_LOCAL_REF(class_kxm);
	SAFE_DEL_LOCAL_REF(obj_kxm);
}


static void copy_yj_heater_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_yj_heater = NULL;
	jobject obj_yj_heater = NULL;
	jfieldID fid = NULL;
	cl_yj_heater_info_t *yj_heater = (cl_yj_heater_info_t *)udp_dev;

	if (yj_heater == NULL) {
		return;
	}

	class_yj_heater = (*env)->FindClass(env, CLASS_YJ_HEATER);
	obj_yj_heater = (*env)->AllocObject(env, class_yj_heater);

	jni_copy_simple_class(env, class_yj_heater, obj_yj_heater,
		                     TRIPLES(boolean, yj_heater, onoff),
		                     TRIPLES(byte, yj_heater, temp_set),
		                     TRIPLES(byte, yj_heater, temp_type),
		                     TRIPLES(boolean, yj_heater, anion),
		                     TRIPLES(byte, yj_heater, power),
		                     TRIPLES(boolean, yj_heater, child_lock),
		                     TRIPLES(short, yj_heater, temp_now),
		                     TRIPLES(byte, yj_heater, timer),
		                     TRIPLES(byte, yj_heater, err_code),
		                     TRIPLES(byte, yj_heater, process),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_yj_heater);
	
	SAFE_DEL_LOCAL_REF(class_yj_heater);
	SAFE_DEL_LOCAL_REF(obj_yj_heater);
}

static void copy_yy_cleaner_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_yy_info;
	jobject obj_yy_info;
	jclass class_history = NULL;
	jclass obj_history = NULL;
	jfieldID fid = NULL;
	cl_yuyuan_info_t *yy_info = (cl_yuyuan_info_t *)udp_dev;
	int his_data[368];
	int i = 0;

	if (yy_info == NULL) {
		return;
	}

	class_yy_info = (*env)->FindClass(env, CLASS_YY_INFO);
	obj_yy_info = (*env)->AllocObject(env, class_yy_info);

	JNI_COPY_SIMPLE_CLASS(env, class_yy_info, obj_yy_info, CLASS_YY_STAT, state,
		                    TRIPLES(int, &yy_info->state, WORK_STAT),
		                    TRIPLES(int, &yy_info->state, WATER_LEVEL),
		                    TRIPLES(int, &yy_info->state, WATER_BOX),
		                    TRIPLES(int, &yy_info->state, WATER_USED_IMPULSE),
		                    TRIPLES(int, &yy_info->state, INLET_TIMEOUT),
		                    TRIPLES(int, &yy_info->state, IMPULSE_COUNT),
		                    TRIPLES(int, &yy_info->state, IMPULSE_PERIOD),
		                    TRIPLES(int, &yy_info->state, MCDELAY),
		                    TRIPLES(int, &yy_info->state, NM_VALVE_DELAY),
		                    TRIPLES(int, &yy_info->state, FUNC_VALVE_TIMEOUT),
		                    TRIPLES(int, &yy_info->state, SPEED1),
		                    TRIPLES(int, &yy_info->state, SPEED2),
		                    TRIPLES(int, &yy_info->state, SPEED3),
		                    TRIPLES(int, &yy_info->state, LOOP_ONOFF),
		                    TRIPLES(int, &yy_info->state, LOOPD),
		                    TRIPLES(int, &yy_info->state, LOOPH),
		                    TRIPLES(int, &yy_info->state, LOOPM),
		                    TRIPLES(int, &yy_info->state, LOOPT),
		                    TRIPLES(int, &yy_info->state, MCCLEAN_D),
		                    TRIPLES(int, &yy_info->state, MCCLEAN_H),
		                    TRIPLES(int, &yy_info->state, MCCLEAN_M),
		                    TRIPLES(int, &yy_info->state, MCCLEAN_RT),
		                    TRIPLES(int, &yy_info->state, MCCLEAN_DT),
		                    TRIPLES(int, &yy_info->state, NMCLEAN_D),
		                    TRIPLES(int, &yy_info->state, NMCLEAN_H),
		                    TRIPLES(int, &yy_info->state, NMCLEAN_M),
		                    TRIPLES(int, &yy_info->state, NMCLEAN_RT),
		                    TRIPLES(int, &yy_info->state, NMCLEAN_DT),
		                    QUADRUPLE(int[], &yy_info->state, ERROR_INFO, 8),
		                    TRIPLES(String, &yy_info->state, CONFIG),
		                    JNI_VAR_ARG_END);

	class_history = (*env)->FindClass(env, CLASS_YY_WATER_HISTORY);
	obj_history = (*env)->AllocObject(env, class_history);
	jni_copy_simple_class(env, class_history, obj_history,
                		     TRIPLES(int, &yy_info->histroy, valid),
		                     TRIPLES(int, &yy_info->histroy, last_write_time),
		                     TRIPLES(int, &yy_info->histroy, count),
		                     JNI_VAR_ARG_END);

	for (i = 0; i < 368; ++i) {
		his_data[i] = yy_info->histroy.data[i];
	}
	jniCopyIntArray(env, class_history, "data", obj_history, his_data, 368);
	fid = (*env)->GetFieldID(env, class_yy_info, "histroy", "L"CLASS_YY_WATER_HISTORY";"); 
	(*env)->SetObjectField(env, obj_yy_info, fid, obj_history); 
	SAFE_DEL_LOCAL_REF(class_history); 
	SAFE_DEL_LOCAL_REF(obj_history); 

	JNI_COPY_SIMPLE_CLASS(env, class_yy_info, obj_yy_info, CLASS_YY_PWD, pwd,
		                     TRIPLES(int, &yy_info->pwd, valid),
		                     TRIPLES(int, &yy_info->pwd, pwd_len),
		                     TRIPLES(String, &yy_info->pwd, pwd),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_yy_info, obj_yy_info, CLASS_YY_REMIND, remind,
		                     TRIPLES(int, &yy_info->remind, valid),
		                     TRIPLES(int, &yy_info->remind, onoff),
		                     TRIPLES(int, &yy_info->remind, remind_time),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_yy_info);
	
	SAFE_DEL_LOCAL_REF(class_yy_info);
	SAFE_DEL_LOCAL_REF(obj_yy_info);
}

static void copy_zk_water_heater_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_zk_info;
	jobject obj_zk_info;
	jfieldID fid = NULL;
	cl_zkrsq_info_t *zk_info = (cl_zkrsq_info_t *)udp_dev;

	if (zk_info == NULL) {
		return;
	}

	class_zk_info = (*env)->FindClass(env, CLASS_ZK_WATER_HEATER_INFO);
	obj_zk_info = (*env)->AllocObject(env, class_zk_info);
	jni_copy_simple_class(env, class_zk_info, obj_zk_info,
		                     TRIPLES(boolean, zk_info, onoff),
		                     TRIPLES(boolean, zk_info, back_water_onoff),
		                     TRIPLES(byte, zk_info, hot_water_tmp),
		                     TRIPLES(byte, zk_info, mode),
		                     TRIPLES(byte, zk_info, fault),
		                     TRIPLES(byte, zk_info, reset_status),
		                     TRIPLES(byte, zk_info, hot_water_tmp_cur),
		                     TRIPLES(byte, zk_info, coilpipe_tmp),
		                     TRIPLES(byte, zk_info, room_tmp),
		                     TRIPLES(byte, zk_info, back_water_tmp_cur),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_zk_info, obj_zk_info, CLASS_ZK_WATER_HEATER_PARAM, conf,
		                     TRIPLES(byte, &zk_info->conf,back_tmp),
		                     TRIPLES(byte, &zk_info->conf,compensation_tmp),
		                     TRIPLES(byte, &zk_info->conf,defrost_time),
		                     TRIPLES(byte, &zk_info->conf,defrost_in_tmp),
		                     TRIPLES(byte, &zk_info->conf,defrost_continue_time),
		                     TRIPLES(byte, &zk_info->conf,defrost_out_tmp),
		                     TRIPLES(byte, &zk_info->conf,back_water_tmp),
		                     TRIPLES(byte, &zk_info->conf,back_water_mode),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_zk_info, obj_zk_info, CLASS_ZK_WATER_HEATER_TIMER, timer,
		                     TRIPLES(boolean, &zk_info->timer, timer1_valid),
		                     TRIPLES(boolean, &zk_info->timer, timer1_onoff),
		                     TRIPLES(byte, &zk_info->timer, timer1_hour),
		                     TRIPLES(byte, &zk_info->timer, timer1_min),
		                     TRIPLES(byte, &zk_info->timer, timer1_hour_end),
		                     TRIPLES(byte, &zk_info->timer, timer1_min_end),
		                     TRIPLES(boolean, &zk_info->timer, timer2_valid),
		                     TRIPLES(boolean, &zk_info->timer, timer2_onoff),
		                     TRIPLES(byte, &zk_info->timer, timer2_hour),
		                     TRIPLES(byte, &zk_info->timer, timer2_min),
		                     TRIPLES(byte, &zk_info->timer, timer2_hour_end),
		                     TRIPLES(byte, &zk_info->timer, timer2_min_end),
		                     TRIPLES(boolean, &zk_info->timer, timer3_valid),
		                     TRIPLES(boolean, &zk_info->timer, timer3_onoff),
		                     TRIPLES(byte, &zk_info->timer, timer3_hour),
		                     TRIPLES(byte, &zk_info->timer, timer3_min),
		                     TRIPLES(byte, &zk_info->timer, timer3_hour_end),
		                     TRIPLES(byte, &zk_info->timer, timer3_min_end),
		                     JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_zk_info);
	
	SAFE_DEL_LOCAL_REF(class_zk_info);
	SAFE_DEL_LOCAL_REF(obj_zk_info);
}

static void copy_india_car_wifi_conf(JNIEnv* env, jclass class_car, jobject obj_car, india_car_wifi_config_t *car_wifi)
{
	jclass class_car_wifi;
	jobject obj_car_wifi;
	jfieldID fid = NULL;
	char *p = car_wifi->data;
	
	class_car_wifi = (*env)->FindClass(env, CLASS_INDIACAR_WIFI_CONF);
	obj_car_wifi = (*env)->AllocObject(env, class_car_wifi);
	
	jniCopyByteValue(env, class_car_wifi, "stat", obj_car_wifi, car_wifi->stat);
	jniCopyString(env, class_car_wifi, "ssid", obj_car_wifi, p);
	p += car_wifi->ssid_len + 1;
	jniCopyString(env, class_car_wifi, "pwd", obj_car_wifi, p);

	fid = (*env)->GetFieldID(env, class_car, "wifi_config", "L" CLASS_INDIACAR_WIFI_CONF ";");
	(*env)->SetObjectField(env, obj_car, fid, obj_car_wifi);
	
	SAFE_DEL_LOCAL_REF(class_car_wifi);
	SAFE_DEL_LOCAL_REF(obj_car_wifi);
}

static void copy_india_car_warn(JNIEnv* env, jclass class_car, jobject obj_car, india_car_warn_t *car_warn)
{
	jclass class_car_warn;
	jobject obj_car_warn;
	jfieldID fid = NULL;
	
	class_car_warn = (*env)->FindClass(env, CLASS_INDIACAR_WARN);
	obj_car_warn = (*env)->AllocObject(env, class_car_warn);

	jni_copy_simple_class(env, class_car_warn, obj_car_warn,
		                     TRIPLES(int, car_warn, onoff),
		                     TRIPLES(byte, car_warn, max_speed),
		                     TRIPLES(byte, car_warn, reset_time),
		                     TRIPLES(short, car_warn, max_distance),
		                     TRIPLES(short, car_warn, max_time),
		                     TRIPLES(short, car_warn, max_freerun_time),
		                     TRIPLES(byte, car_warn, max_vol),
		                     TRIPLES(byte, car_warn, report_period),
		                     TRIPLES(byte, car_warn, geo_fencing_radius),
		                     TRIPLES(byte, car_warn, auto_upgrade_onoff),
		                     TRIPLES(byte, car_warn, break_level),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_car_warn, obj_car_warn, CLASS_INDIACAR_LONGILATI, longitude,
		                     TRIPLES(int, &car_warn->longitude, high),
		                     TRIPLES(int, &car_warn->longitude, low),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_car_warn, obj_car_warn, CLASS_INDIACAR_LONGILATI, latitude,
		                     TRIPLES(int, &car_warn->latitude, high),
		                     TRIPLES(int, &car_warn->latitude, low),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_car, "warn", "L" CLASS_INDIACAR_WARN ";");
	(*env)->SetObjectField(env, obj_car, fid, obj_car_warn);
	
	SAFE_DEL_LOCAL_REF(class_car_warn);
	SAFE_DEL_LOCAL_REF(obj_car_warn);
}

static void copy_india_car_stat(JNIEnv* env, jclass class_car, jobject obj_car, india_car_stat_t *car_stat)
{
	jclass class_car_stat;
	jobject obj_car_stat;
	jfieldID fid = NULL;
	
	class_car_stat = (*env)->FindClass(env, CLASS_INDIACAR_STAT);
	obj_car_stat = (*env)->AllocObject(env, class_car_stat);

	jni_copy_simple_class(env, class_car_stat, obj_car_stat,
		                     TRIPLES(short, car_stat, id),
		                     TRIPLES(byte, car_stat, engine_stat),
		                     TRIPLES(byte, car_stat, vol),
		                     TRIPLES(byte, car_stat, temp_now_degree),
		                     TRIPLES(byte, car_stat, temp_avg_degree),
		                     TRIPLES(byte, car_stat, temp_max_degree),
		                     TRIPLES(byte, car_stat, temp_now_fahrenheit),
		                     TRIPLES(byte, car_stat, temp_avg_fahrenheit),
		                     TRIPLES(byte, car_stat, temp_max_fahrenheit),
		                     TRIPLES(byte, car_stat, speed_now),
		                     TRIPLES(byte, car_stat, speed_avg),
		                     TRIPLES(byte, car_stat, speed_max),
		                     TRIPLES(byte, car_stat, is_runfree),
		                     TRIPLES(short, car_stat, runfree_time),
		                     TRIPLES(short, car_stat, total_time),
		                     TRIPLES(byte, car_stat, start_hour),
		                     TRIPLES(byte, car_stat, start_min),
		                     TRIPLES(short, car_stat, total_distance),
		                     TRIPLES(byte, car_stat, power),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_car_stat, obj_car_stat, CLASS_INDIACAR_LONGILATI, longitude,
		                     TRIPLES(int, &car_stat->longitude, high),
		                     TRIPLES(int, &car_stat->longitude, low),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_car_stat, obj_car_stat, CLASS_INDIACAR_LONGILATI, latitude,
		                     TRIPLES(int, &car_stat->latitude, high),
		                     TRIPLES(int, &car_stat->latitude, low),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_car, "car_stat", "L" CLASS_INDIACAR_STAT ";");
	(*env)->SetObjectField(env, obj_car, fid, obj_car_stat);
	
	SAFE_DEL_LOCAL_REF(class_car_stat);
	SAFE_DEL_LOCAL_REF(obj_car_stat);
}

static void copy_india_car_realtime_trip(JNIEnv* env, jclass class_car, jobject obj_car, cl_indiacar_realtime_trip_t *trip)
{
	jclass class_trip = NULL, class_trip_item = NULL;
	jobject obj_trip = NULL, obj_trip_item = NULL;
	jobjectArray array_trip_item = NULL;
	jfieldID fid = NULL;
	int i = 0;
	
	class_trip = (*env)->FindClass(env, CLASS_INDIACAR_REALTIME_TRIP);
	obj_trip = (*env)->AllocObject(env, class_trip);

	jni_copy_simple_class(env,class_trip, obj_trip,
		                     TRIPLES(int, trip, last_start_idx),
		                     TRIPLES(int, trip, last_end_idx),
		                     JNI_VAR_ARG_END);

	if (trip->max_items_end_idx) {
		class_trip_item = (*env)->FindClass(env, CLASS_INDIACAR_REALTIME_TRIP_ITEM);
		array_trip_item = (*env)->NewObjectArray(env, trip->max_items_end_idx, class_trip_item, NULL);
		for (i = 0; i < trip->max_items_end_idx; ++i) {
			obj_trip_item = (*env)->AllocObject(env, class_trip_item);
			jni_copy_simple_class(env, class_trip_item, obj_trip_item,
				                     TRIPLES(byte, trip->items, temp),
				                     TRIPLES(byte, trip->items, vol),
				                     TRIPLES(byte, trip->items, speed),
				                     TRIPLES(byte, trip->items, stat),
				                     JNI_VAR_ARG_END);
			JNI_COPY_SIMPLE_CLASS(env, class_trip_item, obj_trip_item, CLASS_INDIACAR_LONGILATI, longitude,
				                     TRIPLES(int, &trip->items[i].longitude, high),
				                     TRIPLES(int, &trip->items[i].longitude, low),
				                     JNI_VAR_ARG_END);
			JNI_COPY_SIMPLE_CLASS(env, class_trip_item, obj_trip_item, CLASS_INDIACAR_LONGILATI, latitude,
				                     TRIPLES(int, &trip->items[i].latitude, high),
				                     TRIPLES(int, &trip->items[i].latitude, low),
				                     JNI_VAR_ARG_END);
			(*env)->SetObjectArrayElement(env, array_trip_item, i, obj_trip_item); 
			SAFE_DEL_LOCAL_REF(obj_trip_item); 
		}
		fid = (*env)->GetFieldID(env, class_trip, "items", "[L"CLASS_INDIACAR_REALTIME_TRIP_ITEM";");
		(*env)->SetObjectField(env, obj_trip, fid, array_trip_item);
		SAFE_DEL_LOCAL_REF(array_trip_item);
		SAFE_DEL_LOCAL_REF(class_trip_item);
	}
	
	fid = (*env)->GetFieldID(env, class_car, "rt", "L"CLASS_INDIACAR_REALTIME_TRIP";");
	(*env)->SetObjectField(env, obj_car, fid, obj_trip);
	
	SAFE_DEL_LOCAL_REF(class_trip);
	SAFE_DEL_LOCAL_REF(obj_trip);
}


static void copy_india_car_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_car = NULL;
	jobject obj_car = NULL;
	jfieldID fid = NULL;
	cl_indiacar_info_t *car_info = (cl_indiacar_info_t *)udp_dev;

	if (car_info == NULL) {
		return;
	}

	class_car = (*env)->FindClass(env, CLASS_INDIACAR_INFO);
	obj_car = (*env)->AllocObject(env, class_car);
	copy_india_car_stat(env, class_car, obj_car, &car_info->car_stat);
	JNI_COPY_SIMPLE_CLASS(env, class_car, obj_car, CLASS_INDIACAR_DEV_STAT, dev_stat,
		                     TRIPLES(byte, &car_info->dev_stat, gps),
		                     TRIPLES(byte, &car_info->dev_stat, hotspot),
		                     TRIPLES(byte, &car_info->dev_stat, battery_left),
		                     TRIPLES(byte, &car_info->dev_stat, front_camera),
		                     TRIPLES(byte, &car_info->dev_stat, rear_camera),
		                     TRIPLES(byte, &car_info->dev_stat, microphone),
		                     TRIPLES(byte, &car_info->dev_stat, power_supply_mode),
		                     TRIPLES(byte, &car_info->dev_stat, voltage),
		                     TRIPLES(byte, &car_info->dev_stat, tree_axis_accelerometer),
		                     TRIPLES(byte, &car_info->dev_stat, temp),
		                     JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env,class_car, obj_car, CLASS_INDIACAR_STORE_STAT, store_stat,
		                     TRIPLES(short, &car_info->store_stat, tf_total),
		                     TRIPLES(short, &car_info->store_stat, tf_left),
		                     TRIPLES(short, &car_info->store_stat, flash_total),
		                     JNI_VAR_ARG_END);
	copy_india_car_warn(env, class_car, obj_car, &car_info->warn);
	copy_india_car_wifi_conf(env, class_car, obj_car, &car_info->wifi_config);	
	JNI_COPY_SIMPLE_CLASS(env,class_car, obj_car, CLASS_INDIACAR_UPGRADE, upgrade_stat,
		                     TRIPLES(byte, &car_info->upgrade_stat, major),
		                     TRIPLES(byte, &car_info->upgrade_stat, minor),
		                     TRIPLES(int, &car_info->upgrade_stat, svn),
		                     TRIPLES(int, &car_info->upgrade_stat, data_len),
		                     TRIPLES(byte, &car_info->upgrade_stat, stat),
		                     TRIPLES(byte, &car_info->upgrade_stat, process),
		                     TRIPLES(byte, &car_info->upgrade_stat, err),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env,class_car, obj_car, CLASS_INDIACAR_JOURNEY_NUM, jn,
		                     TRIPLES(int, &car_info->jn, date),
		                     TRIPLES(byte, &car_info->jn, count),
		                     JNI_VAR_ARG_END);
	copy_india_car_realtime_trip(env, class_car, obj_car, &car_info->rt);	
	JNI_COPY_SIMPLE_CLASS(env, class_car, obj_car, CLASS_INDIACAR_DEBUG_INFO, dc,
		                     TRIPLES(byte, &car_info->dc, cmd),
		                     TRIPLES(byte, &car_info->dc, onoff),
		                     TRIPLES(short, &car_info->dc, cmd_len),
		                     TRIPLES(short, &car_info->dc, gps_time_inv),
		                     TRIPLES(short, &car_info->dc, remote_port),
		                     TRIPLES(int, &car_info->dc, remote_ip),
		                     TRIPLES(short, &car_info->dc, gps_len_inv),
		                     TRIPLES(short, &car_info->dc, file_debug_enable),
		                     TRIPLES(short, &car_info->dc, file_debug_level),
		                     TRIPLES(short, &car_info->dc, file_debug_url_len),
		                     TRIPLES(short, &car_info->dc, bps),
		                     TRIPLES(byte, &car_info->dc, video_rotate),
		                     TRIPLES(short, &car_info->dc, moto_threshold),
		                     TRIPLES(short, &car_info->dc, detail_save_inv),
		                     TRIPLES(byte, &car_info->dc, power),
		                     TRIPLES(byte, &car_info->dc, realtime_inv),
		                     TRIPLES(String, &car_info->dc, url),
		                     JNI_VAR_ARG_END);
	
	JNI_COPY_SIMPLE_CLASS(env, class_car, obj_car, CLASS_INDIACAR_LOCAL_VIDEO, wi,
		                     TRIPLES(byte, &car_info->wi, errcode),
		                     TRIPLES(int, &car_info->wi, ip),
		                     TRIPLES(int, &car_info->wi, port),
		                     TRIPLES(int, &car_info->wi, year),
		                     TRIPLES(int, &car_info->wi, month),
		                     TRIPLES(int, &car_info->wi, day),
		                     QUADRUPLE(int[], &car_info->wi, name_list, car_info->wi.num),
		                     JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_car);
	SAFE_DEL_LOCAL_REF(class_car);
	SAFE_DEL_LOCAL_REF(obj_car);
}

static jobject copy_zk_cleaner_item_info(JNIEnv *env, jclass wrap_class, jobject wrap_obj, char *attrname, void *info)
{
	jclass class_day = NULL;
	jobject obj_day = NULL;
	jfieldID fid = NULL;
	u_int32_t t = 0;
	cl_zkcleanner_date_t *items = NULL;

	if (strcmp(attrname, "day_data") == 0) {
		t = ((cl_zkcleanner_day_data_t*)info)->time;
		items = ((cl_zkcleanner_day_data_t*)info)->items;
	} else {
		t = ((cl_zkcleanner_month_data_t*)info)->time;
		items = ((cl_zkcleanner_month_data_t*)info)->items;
	}
		
	class_day = (*env)->FindClass(env, CLASS_ZK_CLEANNER_DAY);
	obj_day = (*env)->AllocObject(env, class_day);
	jniCopyIntValue(env, class_day, "time", obj_day, t);
	JNI_COPY_ARRAY_CLASS(env, class_day, obj_day, CLASS_ZK_CLEANNER_SAMPLE, items,
		                    24, sizeof(cl_zkcleanner_date_t),
		                    ARRAY_TRIPLES(short, items, pm25),
		                    ARRAY_TRIPLES(short, items, co2),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, wrap_class, attrname, "L" CLASS_ZK_CLEANNER_DAY ";");
	(*env)->SetObjectField(env, wrap_obj, fid, obj_day);
	SAFE_DEL_LOCAL_REF(class_day);
	SAFE_DEL_LOCAL_REF(obj_day);
	return NULL;
}

static copy_hy_thermostat_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_hy = NULL;
	jobject obj_hy = NULL;
	jfieldID fid = NULL;
	cl_hythermostat_info_t *hy_info = (cl_hythermostat_info_t *)udp_dev;

	if (hy_info == NULL) {
		return;
	}
	class_hy = (*env)->FindClass(env, CLASS_HY_THERMOSTAT_INFO);
	obj_hy = (*env)->AllocObject(env, class_hy);		

	JNI_COPY_SIMPLE_CLASS(env, class_hy, obj_hy, CLASS_HY_THERMOSTAT_STAT, stat,
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

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_hy);
	SAFE_DEL_LOCAL_REF(class_hy);
	SAFE_DEL_LOCAL_REF(obj_hy);
}

static copy_zk_cleanner_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_zk = NULL;
	jobject obj_zk = NULL;
	jfieldID fid = NULL;
	cl_zkcleanner_info_t *zk_info = (cl_zkcleanner_info_t *)udp_dev;

	if (zk_info == NULL) {
		return;
	}

	class_zk = (*env)->FindClass(env, CLASS_ZK_CLEANNER_INFO);
	obj_zk = (*env)->AllocObject(env, class_zk);

	JNI_COPY_SIMPLE_CLASS(env, class_zk, obj_zk, CLASS_ZK_CLEANNER_STAT, stat,
					 TRIPLES(boolean, &zk_info->stat, valid),
		                     TRIPLES(boolean, &zk_info->stat, onoff),
		                     TRIPLES(byte, &zk_info->stat, mode),
		                     TRIPLES(byte, &zk_info->stat, wind),
		                     TRIPLES(byte, &zk_info->stat, antibiosis),
		                     TRIPLES(byte, &zk_info->stat, fresh),
		                     TRIPLES(byte, &zk_info->stat, maintain),
		                     TRIPLES(short, &zk_info->stat, ontime),
		                     TRIPLES(short, &zk_info->stat, offtime),
		                     TRIPLES(byte, &zk_info->stat, type),
		                     TRIPLES(byte, &zk_info->stat, temp),
		                     TRIPLES(short, &zk_info->stat, pm25),
		                     TRIPLES(short, &zk_info->stat, co2),
		                     TRIPLES(short, &zk_info->stat, hcho),
		                     TRIPLES(short, &zk_info->stat, voc),
		                     TRIPLES(byte, &zk_info->stat, aqi),
		                     TRIPLES(short, &zk_info->stat, uptime),
		                     TRIPLES(boolean, &zk_info->stat, on_timer_changed),
		                     TRIPLES(boolean, &zk_info->stat, off_timer_changed),
		                     TRIPLES(boolean, &zk_info->stat, ver),
		                     JNI_VAR_ARG_END);
	copy_zk_cleaner_item_info(env, class_zk, obj_zk, "day_data", &zk_info->day_data);
	copy_zk_cleaner_item_info(env, class_zk, obj_zk, "month_data", &zk_info->month_data);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_zk);
	SAFE_DEL_LOCAL_REF(class_zk);
	SAFE_DEL_LOCAL_REF(obj_zk);
}

static copy_jrx_heater_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_jrx = NULL;
	jobject obj_jrx = NULL;
	jfieldID fid = NULL;
	cl_jrxheater_info_t *jrx_info = (cl_jrxheater_info_t *)udp_dev;

	if (jrx_info == NULL) {
		return;
	}

	class_jrx = (*env)->FindClass(env, CLASS_JRX_HEATER_INFO);
	obj_jrx = (*env)->AllocObject(env, class_jrx);

	JNI_COPY_SIMPLE_CLASS(env, class_jrx, obj_jrx, CLASS_JRX_HEATER_STAT, stat,
		                     TRIPLES(boolean, &jrx_info->stat, onoff),
		                     TRIPLES(byte, &jrx_info->stat, mode),
		                     TRIPLES(byte, &jrx_info->stat, set_temp),
		                     TRIPLES(boolean, &jrx_info->stat, child_lock),
		                     TRIPLES(byte, &jrx_info->stat, work_state),
		                     TRIPLES(byte, &jrx_info->stat, temp),
		                     TRIPLES(byte, &jrx_info->stat, capacity),
		                     TRIPLES(byte, &jrx_info->stat, power),
		                     TRIPLES(byte, &jrx_info->stat, hour),
		                     TRIPLES(byte, &jrx_info->stat, min),
		                     JNI_VAR_ARG_END);	

	JNI_COPY_ARRAY_CLASS(env, class_jrx, obj_jrx, CLASS_JRX_HEATER_PERIOD_TIMER, period_timer,
		                   8, sizeof(cl_jrxheater_period_timer_t),
		                   ARRAY_TRIPLES(boolean, jrx_info->period_timer, valid),
		                   ARRAY_TRIPLES(boolean, jrx_info->period_timer, enable),
		                   ARRAY_TRIPLES(byte, jrx_info->period_timer, id),
		                   ARRAY_TRIPLES(byte, jrx_info->period_timer, start_hour),
		                   ARRAY_TRIPLES(byte, jrx_info->period_timer, start_min),
		                   ARRAY_TRIPLES(byte, jrx_info->period_timer, end_hour),
		                   ARRAY_TRIPLES(byte, jrx_info->period_timer, end_min),
		                   ARRAY_TRIPLES(byte, jrx_info->period_timer, set_temp),
		                   JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_jrx);
	SAFE_DEL_LOCAL_REF(class_jrx);
	SAFE_DEL_LOCAL_REF(obj_jrx);
}

static copy_linkon_thermostat_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_lk = NULL;
	jobject obj_lk = NULL;
	jfieldID fid = NULL;
	cl_linkon_info_t *lk_info = (cl_linkon_info_t *)udp_dev;

	if (lk_info == NULL) {
		return;
	}

	class_lk = (*env)->FindClass(env, CLASS_LK_THERMOSTAT_INFO);
	obj_lk = (*env)->AllocObject(env, class_lk);

	JNI_COPY_SIMPLE_CLASS(env, class_lk, obj_lk, CLASS_LK_THERMOSTAT_STAT, stat,
		                     TRIPLES(boolean, &lk_info->stat, power),
		                     TRIPLES(boolean, &lk_info->stat, lock),
		                     TRIPLES(short, &lk_info->stat, house_temp),
		                     TRIPLES(short, &lk_info->stat, const_temp),
		                     TRIPLES(short, &lk_info->stat, go_out_temp),
		                     TRIPLES(short, &lk_info->stat, save_temp),
		                     TRIPLES(byte, &lk_info->stat, humidity),
		                     TRIPLES(byte, &lk_info->stat, work_mode),
		                     TRIPLES(byte, &lk_info->stat, running_mode),
		                     TRIPLES(byte, &lk_info->stat, wind_speed),
		                     JNI_VAR_ARG_END);	
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_lk);
	SAFE_DEL_LOCAL_REF(class_lk);
	SAFE_DEL_LOCAL_REF(obj_lk);
}


static copy_bp_thermostat_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_bp = NULL;
	jobject obj_bp = NULL;
	jfieldID fid = NULL;
	cl_bpuair_info_t *bp_info = (cl_bpuair_info_t *)udp_dev;

	if (bp_info == NULL) {
		return;
	}
	class_bp = (*env)->FindClass(env, CLASS_BP_THERMOSTAT_INFO);
	obj_bp = (*env)->AllocObject(env, class_bp);	

	JNI_COPY_SIMPLE_CLASS(env, class_bp, obj_bp, CLASS_BP_THERMOSTAT_STAT, stat,
		                     TRIPLES(byte, &bp_info->stat, type),
		                     TRIPLES(byte, &bp_info->stat, stat1),
		                     TRIPLES(byte, &bp_info->stat, stat2),
		                     TRIPLES(byte, &bp_info->stat, work_mode),
		                     TRIPLES(byte, &bp_info->stat, eco_mode),
		                     TRIPLES(short, &bp_info->stat, cold_temp),
		                     TRIPLES(short, &bp_info->stat, hot_temp),
		                     TRIPLES(short, &bp_info->stat, backwater_temp),
		                     TRIPLES(short, &bp_info->stat, water_temp),
		                     TRIPLES(short, &bp_info->stat, env_temp),
		                     TRIPLES(short, &bp_info->stat, coiler1_temp),
		                     TRIPLES(short, &bp_info->stat, coiler2_temp),
		                     TRIPLES(short, &bp_info->stat, current1),
		                     TRIPLES(short, &bp_info->stat, current2),
		                     TRIPLES(short, &bp_info->stat, eco_cold_temp),
		                     TRIPLES(short, &bp_info->stat, eco_hot_temp),
		                     TRIPLES(boolean, &bp_info->stat, low_vol_1),
		                     TRIPLES(boolean, &bp_info->stat, high_vol_1),
		                     TRIPLES(boolean, &bp_info->stat, high_current_1),
		                     TRIPLES(boolean, &bp_info->stat, low_current_1),
		                     TRIPLES(boolean, &bp_info->stat, coiler_high_temp_1),
		                     TRIPLES(boolean, &bp_info->stat, exhaust_temp_sensor_1),
		                     TRIPLES(boolean, &bp_info->stat, exhaust_temp_high_1),
		                     TRIPLES(boolean, &bp_info->stat, phase_protection_1),
		                     TRIPLES(boolean, &bp_info->stat, anti_phase_protection_1),
	                         TRIPLES(boolean, &bp_info->stat, low_vol_2),
		                     TRIPLES(boolean, &bp_info->stat, high_vol_2),
		                     TRIPLES(boolean, &bp_info->stat, high_current_2),
		                     TRIPLES(boolean, &bp_info->stat, low_current_2),
		                     TRIPLES(boolean, &bp_info->stat, coiler_high_temp_2),
		                     TRIPLES(boolean, &bp_info->stat, exhaust_temp_sensor_err_2),
		                     TRIPLES(boolean, &bp_info->stat, exhaust_temp_high_2),
		                     TRIPLES(boolean, &bp_info->stat, fault_phase_protection_2),
		                     TRIPLES(boolean, &bp_info->stat, eeprom_data_2),
		                     TRIPLES(boolean, &bp_info->stat, const_temp_sensor_2),
		                     TRIPLES(boolean, &bp_info->stat, sys_temp_return_2),
		                     TRIPLES(boolean, &bp_info->stat, sys_temp_out_2),
		                     TRIPLES(boolean, &bp_info->stat, phase_protection_2),
		                     TRIPLES(boolean, &bp_info->stat, anti_phase_protection_2),
		                     TRIPLES(boolean, &bp_info->stat, lack_water),
		                     TRIPLES(boolean, &bp_info->stat, high_temp_out_water),
		                     TRIPLES(boolean, &bp_info->stat, low_temp_out_water),
		                     TRIPLES(boolean, &bp_info->stat, timer_enable),
		                     TRIPLES(boolean, &bp_info->stat, once_timer_enable),
		                     TRIPLES(boolean, &bp_info->stat, once_timer_onoff),
		                     TRIPLES(byte, &bp_info->stat, once_timer_hour),
		                     TRIPLES(byte, &bp_info->stat, once_timer_min),
		                     TRIPLES(short, &bp_info->stat, uptime),
		                     QUADRUPLE(byte[], &bp_info->stat, unit_code, 64),
		                     TRIPLES(String, &bp_info->stat, soft_ver1),
		                     TRIPLES(String, &bp_info->stat, soft_ver2),
		                     QUADRUPLE(byte[], &bp_info->stat, fault_array, 7),
		                     TRIPLES(int, &bp_info->stat, next_timer_stamp),
		                     TRIPLES(boolean, &bp_info->stat, next_timer_onoff),
		                     JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_bp, obj_bp, CLASS_BP_THERMOSTAT_TIMER, timers, 
		                   6, sizeof(cl_bpuair_timer_t),
		                   ARRAY_TRIPLES(boolean, bp_info->timers, valid),
		                   ARRAY_TRIPLES(byte, bp_info->timers, week),
		                   ARRAY_TRIPLES(byte, bp_info->timers, onoff),
		                   ARRAY_TRIPLES(byte, bp_info->timers, hour),
		                   ARRAY_TRIPLES(byte, bp_info->timers, mins),
		                   JNI_VAR_ARG_END);

	if (bp_info->fault_history.valid && bp_info->fault_history.num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_bp, obj_bp, CLASS_BP_THERMOSTAT_FAULT, fault_history,
			                   bp_info->fault_history.num, sizeof(cl_bpuair_fault_item_t),
			                   ARRAY_TRIPLES(int, bp_info->fault_history.fault, timestamp),
		                       ARRAY_TRIPLES(int, bp_info->fault_history.fault, fault_number) ,
			                   JNI_VAR_ARG_END);
	}

	if (bp_info->fault_current.num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_bp, obj_bp, CLASS_BP_THERMOSTAT_FAULT, fault_current,
			                   bp_info->fault_current.num, sizeof(cl_bpuair_fault_item_t),
			                   ARRAY_TRIPLES(int, bp_info->fault_current.fault, timestamp),
		                       ARRAY_TRIPLES(int, bp_info->fault_current.fault, fault_number) ,
			                   JNI_VAR_ARG_END);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_bp);
	SAFE_DEL_LOCAL_REF(class_bp);
	SAFE_DEL_LOCAL_REF(obj_bp);
}

static copy_cj_thermostat_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_cj = NULL;
	jobject obj_cj = NULL;
	jfieldID fid = NULL;
	cl_cjthermostat_info_t *cj_info = (cl_cjthermostat_info_t *)udp_dev;

	if (cj_info == NULL) {
		return;
	}

	class_cj = (*env)->FindClass(env, CLASS_CJ_THERMOSTAT_INFO);
	obj_cj = (*env)->AllocObject(env, class_cj);
	

	JNI_COPY_SIMPLE_CLASS(env, class_cj, obj_cj, CLASS_CJ_THERMOSTAT_STAT, stat,
		                     TRIPLES(short, &cj_info->stat, outtime_hour),
		                     TRIPLES(byte, &cj_info->stat, outtime_min),
		                     TRIPLES(byte, &cj_info->stat, ver),
		                     TRIPLES(boolean, &cj_info->stat, is_heat),
		                     TRIPLES(byte, &cj_info->stat, week),
		                     TRIPLES(byte, &cj_info->stat, time),
		                     TRIPLES(byte, &cj_info->stat, stat),
		                     TRIPLES(byte, &cj_info->stat, set_temp),
		                     TRIPLES(byte, &cj_info->stat, inside_temp),
		                     TRIPLES(byte, &cj_info->stat, inside_temp1),
		                     TRIPLES(byte, &cj_info->stat, outside_temp),
		                     TRIPLES(byte, &cj_info->stat, outside_temp1),
		                     TRIPLES(byte, &cj_info->stat, mode),
		                     TRIPLES(byte, &cj_info->stat, power),
		                     TRIPLES(byte, &cj_info->stat, key_lock),
		                     TRIPLES(byte, &cj_info->stat, fault),
		                     TRIPLES(byte, &cj_info->stat, temp_adjust),
		                     TRIPLES(byte, &cj_info->stat, set_temp_upper_limit),
		                     TRIPLES(byte, &cj_info->stat, set_temp_lower_limit),
		                     TRIPLES(byte, &cj_info->stat, temp_allowance),
		                     TRIPLES(byte, &cj_info->stat, defrost_temp),
		                     TRIPLES(byte, &cj_info->stat, overtemp),
		                     TRIPLES(byte, &cj_info->stat, overtemp_allowance),
		                     TRIPLES(byte, &cj_info->stat, flag),
		                     TRIPLES(byte, &cj_info->stat, timer_week),
		                     TRIPLES(byte, &cj_info->stat, manual_temp),
		                     JNI_VAR_ARG_END);

	jni_copy_simple_class(env, class_cj, obj_cj,
		                     QUADRUPLE(byte[], cj_info, work_period_temp, 48),
		                     QUADRUPLE(byte[], cj_info, offday_period_temp, 48),
		                     JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_cj);
	SAFE_DEL_LOCAL_REF(class_cj);
	SAFE_DEL_LOCAL_REF(obj_cj);
}


void copy_rf_gw_oem_base(JNIEnv* env, jclass class_rfgw_oem, jobject object_rfgw_oem, void *udp_dev)
{
	jfieldID fid;
	
	cl_gw_info_t *gw_info_p;

	gw_info_p = (cl_gw_info_t *)udp_dev;

	if (gw_info_p->dev_group_cnt > 0) {
		copy_rf_group_info(env, class_rfgw_oem, object_rfgw_oem,gw_info_p);
	}

	if (gw_info_p->lamp_remote_cnt > 0) {
		copy_rf_rmt_ctrl_info(env, class_rfgw_oem, object_rfgw_oem,gw_info_p);
	}
	
	jniCopyByteArray(env, class_rfgw_oem, "upgrade_status", object_rfgw_oem, gw_info_p->upgrade_status, D_T_MAX);
	jniCopyStringArray(env, class_rfgw_oem, "upgrade_url", object_rfgw_oem, gw_info_p->upgrade_url, D_T_MAX);

	jniCopyByteValue(env, class_rfgw_oem, "commpat", object_rfgw_oem, gw_info_p->commpat);
	jniCopyByteValue(env, class_rfgw_oem, "channel", object_rfgw_oem, gw_info_p->channel);
	jniCopyBooleanValue(env, class_rfgw_oem, "is_upgrade", object_rfgw_oem, gw_info_p->is_upgrade);
}


static copy_yl_rfgw_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_yl = NULL;
	jobject obj_yl = NULL;
	jfieldID fid = NULL;
	cl_gw_info_t *gw_info_p = (cl_gw_info_t*)udp_dev;
	cl_rfdev_scm_yl_t *yl_info = NULL;
	cl_rfdev_scm_t *comm_rfgw_oem = NULL;

	if (gw_info_p == NULL) {
		return;
	}

	comm_rfgw_oem = (cl_rfdev_scm_t *)(&gw_info_p->rfdef_scm_dev);
	yl_info = &comm_rfgw_oem->rfdev_scm_dev_data.yl_info;

	class_yl = (*env)->FindClass(env, CLASS_YL_RFGW_INFO);
	obj_yl = (*env)->AllocObject(env, class_yl);

	copy_rf_gw_oem_base(env, class_yl, obj_yl, udp_dev);
	jniCopyBooleanValue(env, class_yl, "valid", obj_yl, comm_rfgw_oem->valid);

	jni_copy_simple_class(env, class_yl, obj_yl,
		                     TRIPLES(boolean, yl_info, is_guard),
		                     TRIPLES(byte, yl_info, battery),
		                     TRIPLES(byte, yl_info, power),
		                     TRIPLES(byte, yl_info, sos_on),
		                     TRIPLES(short, yl_info, key_info),
		                     TRIPLES(byte, yl_info, door_voice),
		                     TRIPLES(byte, yl_info, alarm_voice),
		                     TRIPLES(short, yl_info, time),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_yl);
	SAFE_DEL_LOCAL_REF(class_yl);
	SAFE_DEL_LOCAL_REF(obj_yl);
}

static copy_drkzq_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	cl_drkzq_info_t *c_dr_info;
	cl_drkzq_stat_t *c_dr_stat;
	cl_drkzq_fault_t *c_dr_fault;
	cl_drkzq_name_item_t *c_name_item;
	
	int i = 0;
	
	jclass class_dr_info;
	jobject obj_dr_info;
	jfieldID fid = NULL;
	
	c_dr_info = (cl_drkzq_info_t*)udp_dev;
	if (c_dr_info == NULL) {
		return;
	}
	c_dr_stat = & c_dr_info->stat;
	c_dr_fault = & c_dr_info->fault;
	c_name_item = c_dr_info->name;
	
	class_dr_info = (*env)->FindClass(env, CLASS_DRKZQ_INFO);
	obj_dr_info = (*env)->AllocObject(env, class_dr_info);
	
	JNI_COPY_SIMPLE_CLASS(env, class_dr_info, obj_dr_info, CLASS_DRKZQ_STAT, stat,
								 TRIPLES(byte, c_dr_stat, humi1),
								 TRIPLES(byte, c_dr_stat, temp1),
								 TRIPLES(byte, c_dr_stat, humi2),
								 TRIPLES(byte, c_dr_stat, temp2),
								 TRIPLES(byte, c_dr_stat, humi3),
								 TRIPLES(byte, c_dr_stat, temp3),
								 TRIPLES(byte, c_dr_stat, humi4),
								 TRIPLES(byte, c_dr_stat, temp4),
								 
								 TRIPLES(byte, c_dr_stat, ph), 
								 TRIPLES(byte, c_dr_stat, cycling_water_temp),
								 TRIPLES(byte, c_dr_stat, analog),
								 
								 TRIPLES(byte, c_dr_stat, watering_onoff1),
								 TRIPLES(byte, c_dr_stat, watering_onoff2),
								 TRIPLES(byte, c_dr_stat, watering_onoff3),
								 TRIPLES(byte, c_dr_stat, watering_onoff4),
								 
								 TRIPLES(byte, c_dr_stat, sunshine_onoff),
								 TRIPLES(byte, c_dr_stat, feretilize_onoff),
								 TRIPLES(byte, c_dr_stat, light_onoff),
								 TRIPLES(byte, c_dr_stat, water_pump_onoff),
								 
								 TRIPLES(byte, c_dr_stat, resv_onoff1),
								 TRIPLES(byte, c_dr_stat, resv_onoff2),
								 TRIPLES(byte, c_dr_stat, resv_onoff3),
								 TRIPLES(byte, c_dr_stat, resv_onoff4),
								 
								 TRIPLES(byte, c_dr_stat, work_mode),
								 
								 TRIPLES(byte, c_dr_stat, humi_threshold1),
								 TRIPLES(byte, c_dr_stat, humi_threshold2),
								 TRIPLES(byte, c_dr_stat, humi_threshold3),
								 TRIPLES(byte, c_dr_stat, humi_threshold4),
								 
								 TRIPLES(byte, c_dr_stat, safe_temp_threshold),
								 TRIPLES(byte, c_dr_stat, cycling_water_temp_threshold),
								 TRIPLES(byte, c_dr_stat, fertilzer_second),
								 TRIPLES(byte, c_dr_stat, sunshine_threshold),
								 TRIPLES(byte, c_dr_stat, safe_ph_threshold),
								 TRIPLES(byte, c_dr_stat, sys_min),
								 TRIPLES(byte, c_dr_stat, sys_hour),
								 
								 TRIPLES(byte, c_dr_stat, timer1_start_min),
								 TRIPLES(byte, c_dr_stat, timer1_start_hour),
								 TRIPLES(byte, c_dr_stat, timer1_end_min),
								 TRIPLES(byte, c_dr_stat, timer1_end_hour),
								 TRIPLES(byte, c_dr_stat, timer1_valid),
								 
								 TRIPLES(byte, c_dr_stat, timer2_start_min),
								 TRIPLES(byte, c_dr_stat, timer2_start_hour),
								 TRIPLES(byte, c_dr_stat, timer2_end_min),
								 TRIPLES(byte, c_dr_stat, timer2_end_hour),
								 TRIPLES(byte, c_dr_stat, timer2_valid),
								 JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env, class_dr_info, obj_dr_info, CLASS_DRKZQ_FAULT, fault,
								TRIPLES(byte, c_dr_fault, temp_sensor),
								TRIPLES(byte, c_dr_fault, humi_sensor),
								TRIPLES(byte, c_dr_fault, ph_sensor),
								TRIPLES(byte, c_dr_fault, sunshine_sensor),
								TRIPLES(byte, c_dr_fault, high_temp),
								TRIPLES(byte, c_dr_fault, lack_water),
								TRIPLES(byte, c_dr_fault, ph),
								TRIPLES(byte, c_dr_fault, humi),
								JNI_VAR_ARG_END);
	
	JNI_COPY_ARRAY_CLASS(env, class_dr_info, obj_dr_info, CLASS_DRKZQ_NAME_TIEM, name, 
							   DRKZQ_NAME_MAX, sizeof(cl_drkzq_name_item_t),
							   ARRAY_TRIPLES(String, c_name_item, name),
							   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, obj_dr_info);
	
	SAFE_DEL_LOCAL_REF(class_dr_info);
	SAFE_DEL_LOCAL_REF(obj_dr_info);
}

static copy_zhcl_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass cls_zhcl = NULL;
	jobject obj_zhcl = NULL;
	jfieldID fid = NULL;
	cl_zhcl_info_t *zhcl = (cl_zhcl_info_t*)udp_dev;
	if(zhcl == NULL){
		return;
	}
	
	cls_zhcl = (*env)->FindClass(env,CLASS_ZH_MOTOR_INFO);
	obj_zhcl = (*env)->AllocObject(env,cls_zhcl);

	jni_copy_simple_class(env,cls_zhcl,obj_zhcl,
		TRIPLES(int, zhcl,magic),
		TRIPLES(byte, zhcl,index),
		TRIPLES(byte, zhcl,status),
		TRIPLES(byte, zhcl,percent),
		TRIPLES(byte, zhcl,type),
		TRIPLES(byte, zhcl,support_dir),
		TRIPLES(byte, zhcl,dir),
		JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env,class_udp_info,"device_info","L"CLASS_JAVA_OBJECT";");
	(*env)->SetObjectField(env,obj_udp_info,fid,obj_zhcl);
	
	SAFE_DEL_LOCAL_REF(cls_zhcl);
	SAFE_DEL_LOCAL_REF(obj_zhcl);
	
}
static copy_leis_lamp_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_lamp;
	jclass class_rmt_ctrl;

	jobject jobj_lamp;
	jobject jobj_rmt_ctrl;
	jobject obj;
	jfieldID fid;
	int i;

	cl_leis_info_t* leis_lamp_p = (cl_leis_info_t*)udp_dev;
	cl_rf_lamp_t lamp_info = leis_lamp_p->lamp_stat;
	cl_rf_lamp_stat_t stat = lamp_info.stat;
	cl_rf_lamp_remote_info_t *rmt_ctrl_info = lamp_info.r_info;
	
	class_lamp = (*env)->FindClass(env, CLASS_LEIS_LAMP_STATE);
	jobj_lamp = (*env)->AllocObject(env, class_lamp);

	jniCopyIntValue(env, class_lamp, "r", jobj_lamp, stat.R);
	jniCopyIntValue(env, class_lamp, "g", jobj_lamp, stat.G);
	jniCopyIntValue(env, class_lamp, "b", jobj_lamp, stat.B);
	jniCopyIntValue(env, class_lamp, "l", jobj_lamp, stat.L);
	jniCopyIntValue(env, class_lamp, "cold", jobj_lamp, stat.cold);
	jniCopyBooleanValue(env, class_lamp, "power", jobj_lamp, stat.power);
	
	jniCopyIntValue(env, class_lamp, "modeId", jobj_lamp, stat.mod_id);
	jniCopyIntValue(env, class_lamp, "action", jobj_lamp, stat.action);
	
	jniCopyBooleanValue(env, class_lamp, "is_support_color_temp", jobj_lamp, lamp_info.is_support_color_temp);
	jniCopyBooleanValue(env, class_lamp, "is_support_rgb", jobj_lamp, lamp_info.is_support_rgb);
	jniCopyIntValue(env, class_lamp, "remote_count", jobj_lamp, lamp_info.remote_count);
	jniCopyIntValue(env, class_lamp, "lamp_type", jobj_lamp, lamp_info.lamp_type);

	if (lamp_info.remote_count > 0) {
		class_rmt_ctrl = (*env)->FindClass(env, CLASS_RF_LAMP_RMT_CTRL_INFO);
		jobj_rmt_ctrl = (*env)->NewObjectArray(env, lamp_info.remote_count, class_rmt_ctrl, NULL);
		for (i = 0; i < lamp_info.remote_count; i++, rmt_ctrl_info++) {
			obj = (*env)->AllocObject(env, class_rmt_ctrl);
			jniCopyIntValue(env, class_rmt_ctrl, "remote_id", obj, rmt_ctrl_info->remote_id);
			jniCopyByteValue(env, class_rmt_ctrl, "key_id", obj, rmt_ctrl_info->key_id);
			(*env)->SetObjectArrayElement(env, jobj_rmt_ctrl, i, obj);
			(*env)->DeleteLocalRef(env, obj);
		}

		fid = (*env)->GetFieldID(env, class_lamp, "r_info", "[L"CLASS_RF_LAMP_RMT_CTRL_INFO";");
		(*env)->SetObjectField(env, jobj_lamp, fid, jobj_rmt_ctrl);
	
		(*env)->DeleteLocalRef(env, class_rmt_ctrl);
		(*env)->DeleteLocalRef(env, jobj_rmt_ctrl);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, obj_udp_info, fid, jobj_lamp);

	SAFE_DEL_LOCAL_REF(class_lamp);
	SAFE_DEL_LOCAL_REF(jobj_lamp);
}

static copy_yinsu_lamp_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_lamp;
	jclass class_rmt_ctrl;

	jobject jobj_lamp;
	jobject jobj_rmt_ctrl;
	jobject obj;
	jfieldID fid;
	int i;

	cl_yinsu_info_t* yinsu_lamp_p = (cl_yinsu_info_t*)udp_dev;
	cl_rf_lamp_t lamp_info = yinsu_lamp_p->lamp_stat;
	cl_rf_lamp_stat_t stat = lamp_info.stat;
	cl_rf_lamp_remote_info_t *rmt_ctrl_info = lamp_info.r_info;
	
	class_lamp = (*env)->FindClass(env, CLASS_YINSU_LAMP_STATE);
	jobj_lamp = (*env)->AllocObject(env, class_lamp);

	jniCopyIntValue(env, class_lamp, "r", jobj_lamp, stat.R);
	jniCopyIntValue(env, class_lamp, "g", jobj_lamp, stat.G);
	jniCopyIntValue(env, class_lamp, "b", jobj_lamp, stat.B);
	jniCopyIntValue(env, class_lamp, "l", jobj_lamp, stat.L);
	jniCopyIntValue(env, class_lamp, "cold", jobj_lamp, stat.cold);
	jniCopyBooleanValue(env, class_lamp, "power", jobj_lamp, stat.power);
	
	jniCopyIntValue(env, class_lamp, "modeId", jobj_lamp, stat.mod_id);
	jniCopyIntValue(env, class_lamp, "action", jobj_lamp, stat.action);
	
	jniCopyBooleanValue(env, class_lamp, "is_support_color_temp", jobj_lamp, lamp_info.is_support_color_temp);
	jniCopyBooleanValue(env, class_lamp, "is_support_rgb", jobj_lamp, lamp_info.is_support_rgb);
	jniCopyIntValue(env, class_lamp, "remote_count", jobj_lamp, lamp_info.remote_count);
	jniCopyIntValue(env, class_lamp, "lamp_type", jobj_lamp, lamp_info.lamp_type);

	if (lamp_info.remote_count > 0) {
		class_rmt_ctrl = (*env)->FindClass(env, CLASS_RF_LAMP_RMT_CTRL_INFO);
		jobj_rmt_ctrl = (*env)->NewObjectArray(env, lamp_info.remote_count, class_rmt_ctrl, NULL);
		for (i = 0; i < lamp_info.remote_count; i++, rmt_ctrl_info++) {
			obj = (*env)->AllocObject(env, class_rmt_ctrl);
			jniCopyIntValue(env, class_rmt_ctrl, "remote_id", obj, rmt_ctrl_info->remote_id);
			jniCopyByteValue(env, class_rmt_ctrl, "key_id", obj, rmt_ctrl_info->key_id);
			(*env)->SetObjectArrayElement(env, jobj_rmt_ctrl, i, obj);
			(*env)->DeleteLocalRef(env, obj);
		}

		fid = (*env)->GetFieldID(env, class_lamp, "r_info", "[L"CLASS_RF_LAMP_RMT_CTRL_INFO";");
		(*env)->SetObjectField(env, jobj_lamp, fid, jobj_rmt_ctrl);
	
		(*env)->DeleteLocalRef(env, class_rmt_ctrl);
		(*env)->DeleteLocalRef(env, jobj_rmt_ctrl);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, obj_udp_info, fid, jobj_lamp);

	SAFE_DEL_LOCAL_REF(class_lamp);
	SAFE_DEL_LOCAL_REF(jobj_lamp);
}

static void copy_zh_dhx_info(JNIEnv* env, jclass class_udp_info, jobject obj_udp_info, void *udp_dev)
{
	jclass class_dhx_info = NULL;
	jclass class_dhx_name = NULL;

	jobject jobj_dhx_info = NULL;
	jobjectArray array_dhx_name = NULL;
	jobject obj = NULL;
	jfieldID fid = NULL;
	int i =0;

	cl_zhdhx_info_t* dhx_info = (cl_zhdhx_info_t*)udp_dev;
	cl_zhdhx_key_name_t* key_name = dhx_info->key_name;
	
	class_dhx_info = (*env)->FindClass(env, CLASS_ZHDHX_INFO);
	jobj_dhx_info = (*env)->AllocObject(env, class_dhx_info);

	jni_copy_simple_class(env,class_dhx_info,jobj_dhx_info,
		TRIPLES(int, dhx_info,dhx_type),
		TRIPLES(int, dhx_info,on_off_stat),
		JNI_VAR_ARG_END);
	
	if (ZHDHX_KEY_NAME_MAX > 0) {
		class_dhx_name = (*env)->FindClass(env, CLASS_ZHDHX_NAME);
		array_dhx_name = (*env)->NewObjectArray(env, ZHDHX_KEY_NAME_MAX, class_dhx_name, NULL);
		for (i = 0; i < ZHDHX_KEY_NAME_MAX && key_name != NULL; i++, key_name++) {
			LOGE("copy_zh_dhx_info      i = %d",i);
			obj = (*env)->AllocObject(env, class_dhx_name);
			jniCopyBooleanValue(env, class_dhx_name, "valid", obj, key_name->valid);
			jniCopyString(env, class_dhx_name, "name", obj, key_name->name);
			(*env)->SetObjectArrayElement(env, array_dhx_name, i, obj);
			(*env)->DeleteLocalRef(env, obj);
		}

		fid = (*env)->GetFieldID(env, class_dhx_info, "key_name", "[L"CLASS_ZHDHX_NAME";");
		(*env)->SetObjectField(env, jobj_dhx_info, fid, array_dhx_name);
	
		(*env)->DeleteLocalRef(env, class_dhx_name);
		(*env)->DeleteLocalRef(env, array_dhx_name);
	}

	fid = (*env)->GetFieldID(env, class_udp_info, "device_info", "L" CLASS_JAVA_OBJECT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, jobj_dhx_info);

	SAFE_DEL_LOCAL_REF(class_dhx_info);
	SAFE_DEL_LOCAL_REF(jobj_dhx_info);
}

static int wuneng_type_map(int exttype)
{
	int map_type = DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_EB_USB:
		map_type = DEV_WUNENG_USB;
		break;
	case ETYPE_IJ_EB_NO_USB:
		map_type = DEV_WUNENG;
		break;
	case ETYPE_IJ_EB_JNB_GLOBAL:
		map_type = DEV_JNB_INTER;
		break;
	case ETYPE_IJ_EB_JNB_WATER_DROP:
		map_type = DEV_JNB_DRIP;
		break;
	case ETYPE_IJ_EB_FOXCONN:
		map_type = DEV_FOXCONN_PLUG;
		break;
	default:
		break;
	}
	return map_type;
}

static int AMT_type_map(int exttype)
{
	int map_type = DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_AMT_FAN:
		map_type = DEV_AMT_FAN;
		break;
	default:
		break;
	}
	return map_type;
}

static int Chiffo_type_map(int exttype)
{
	int map_type = DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_CHIFFO_FlOOR_HEATER:
	case ETYPE_IJ_CHIFFO_WATER_HEATER:
		map_type = DEV_CHIFFO_FLOOR_HEATER;
		break;
	default:
		break;
	}
	return map_type;
}

static int Telin_type_map(int exttype)
{
	int map_type = DEV_MAX;
	switch(exttype) {
	case ETYPE_IJ_TL_HEATER:
		map_type = DEV_TELIN_HEATING;
		break;
	default:
		break;
	}
	return map_type;
}

static int Rf_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_RFGW_6621:
	case ETYPE_IJ_RFGW_S2:
	case ETYPE_IJ_RFGW_S3:
	case ETYPE_IJ_RFGW_S4:
	case ETYPE_IJ_RFGW_S9:
		map_type = DEV_RF_GW_INFO;
		break;
	case ETYPE_IJ_RFGW_YL:
		map_type = DEV_YL_RFGW;
		break;
	default:
		break;
	}
	return map_type;
}
static int ij_test_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_TEST_CAR_WK:
		map_type = DEV_CZWK;
		break;
	case ETYPE_IJ_TEST_XY:
		map_type = DEV_XYWKQ;
		break;
	case ETYPE_IJ_TEST_BITMAR:
		map_type = DEV_BIMAR_HEATER;
		break;
	default:
		break;
	}
	return map_type;
}

static int bimar_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_HEATER_BIMAR_C2000:
		map_type = DEV_BIMAR_HEATER;
		break;
	case ETYPE_IJ_HEATER_AKM_0505L:
	case ETYPE_IJ_HEATER_AKM_5162L:
		map_type = DEV_AUCMA_HEATER;
		break;
	default:
		break;
	}
	return map_type;
}

static int yt_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ838_YT:
		map_type = DEV_YT_AIRPLUG;
		break;
	default:
		break;
	}
	return map_type;
}
static int jl_stage_lamp_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_JL_STAGE_LAMP:
		map_type = DEV_JL_STAGE_LAMP;
		break;
	default:
		break;
	}
	return map_type;
}

static int ads_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ839_ADS:
		map_type = DEV_ADS;
		break;
	default:
		break;
	}
	return map_type;
}

static int js_wave_oven_map(int ext_type)
{
	int map_type = DEV_MAX;
	switch(ext_type) {
	case ETYPE_IJ_JS_MICWARE:
	case ETYPE_IJ_JS_MIC_BARBECUE:
	case ETYPE_IJ_JS_ONLY_MIC:
		map_type = DEV_JS_WAVE_OVEN;
		break;
	default:
		break;
	}
	return map_type;
}

static int evm_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case EYPE_EVM_YUYUAN_WATER_CLEANNER:
		map_type = DEV_YY_WATER_CLEANER;
		break;
	case EYPE_EVM_ZKCLEANNER:
		map_type = DEV_ZK_CLEANNER;
		break;
	case EYPE_EVM_HYTHERMOSTAT_AC:
	case EYPE_EVM_HYTHERMOSTAT_HT:
		map_type = DEV_HY_THERMOSTAT;
		break;
	case EYPE_EVM_BPUAIR_1:
	case EYPE_EVM_BPUAIR_2:
	case EYPE_EVM_BPUAIR_3:	
	case EYPE_EVM_BPUAIR_4:
		map_type = DEV_BP_THERMOSTAT;
		break;
	case EYPE_EVM_JRXHEATER:
		map_type = DEV_GM_HEATER;
		break;
	case EYPE_EVM_CJTHERMOSTAT:
		map_type = DEV_CJ_THERMOSTAT;
		break;
	case EYPE_EVM_DRKZQ:
		map_type = DEV_DRKZQ;
		break;
	default:
		break;
	}
	return map_type;
}

static int indiacar_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_INDIACAR_DEFAULT:
		map_type = DEV_INDIA_CAR;
		break;
	default:
		break;
	}
	return map_type;
}

static int leis_map(int ext_type) 
{
	int map_type = DEV_MAX;
	switch(ext_type) {
	case ETYPE_IJ_LEIS:
		map_type = DEV_LEIS;
		break;
	case ETYPE_LEIS_YINSU:
		map_type = DEV_YINSU;
		break;
	default:
		break;
	}

	return map_type;
}

static int kxm_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_KXM_HOST:
	case EYTYP_IJ_KXM_AC:
		map_type = DEV_KXM_HEATER_PUMP;
		break;
	case ETYPE_IJ_KXM_THERMOSTAT:
	case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
	case ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB:
		map_type = DEV_KXM_HEATER_PUMP_2;
		break;
	case ETYPE_IJ_XY_THERMOSTAT:
		map_type = DEV_XYWKQ;
		break;
	case ETYPE_IJ_SBT_THER:
		map_type = DEV_SBT_THERMOSTAT;
		break;
	case EYTPE_IJ_ZSSX_FURN:
		map_type = DEV_ZSSX_OVEN;
		break;
	case ETYPE_IJ_YJ_HEATER:
		map_type = DEV_YJ_HEATER;
		break;
	case ETYPE_IJ_LINKON_THERMOSTAT:
		map_type = DEV_LINKON_THERMOSTAT;
		break;
	default:
		break;
	}
	return map_type;
}

static int htchp_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_824_HTC:
		map_type = DEV_HTC_HP;
		break;
	case TYPE_IJ_824_YCJ:
		map_type = DEV_TB_POOL;
		break;
	case ETYPE_IJ_824_HTC_BUSINESS:
		map_type = DEV_TB_COMMERCIAL;
		break;
	case ETYPE_IJ_824_ZKRSQ:
		map_type = DEV_ZK_WATER_HEATER;
		break;
	default:
		break;
	}
	return map_type;
}

static int QianPa_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_QP_POT:
		map_type = DEV_QP_PAN;
		break;
	case ETYPE_IJ_QP_PBJ:
		map_type = DEV_QP_POBIJI;
		break;
	default:
		break;
	}
	return map_type;
}

static int hx_type_map(int ext_type)
{
	int map_type = DEV_MAX;
	
	switch(ext_type) {
	case ETYPE_IJ_HX_PBJ:
		map_type = DEV_POBIJI;
		break;
	case ETYPE_IJ_HX_POT:
		map_type = DEV_HX_POT;
		break;
	default:
		break;
	}
	return map_type;
}


int typeMap(int subtype, int exttype) 
{
	int common_dev_type = DEV_MAX;
	
	switch(subtype) {
	case IJ_808:
		common_dev_type = DEV_WUKONG;
		break;
	case IJ_101:
	case IJ_102:
		common_dev_type = wuneng_type_map(exttype);
		break;
	case IJ_821:
		break;
	case IJ_822:
		common_dev_type = DEV_TMC_JNB;
		break;
	case IJ_823:
		common_dev_type = DEV_TMC_YL;
		break;
	case IJ_824:
		common_dev_type = htchp_type_map(exttype);
		break;
	case IJ_830:
		common_dev_type = DEV_LEDE_TGD;
		break;
	case IJ_840:
		common_dev_type = DEV_PDC_JCX;
		break;
	case IJ_AMT:
		common_dev_type = AMT_type_map(exttype);
		break;
	case IJ_CHIFFO:
		common_dev_type = Chiffo_type_map(exttype);
		break;
	case IJ_TL_TEMP:
		common_dev_type = Telin_type_map(exttype);
		break;
	case IJ_QPCP:
		common_dev_type = DEV_QP_CP;
		break;
	case IJ_QP_POT:
		common_dev_type = QianPa_type_map(exttype);
		break;
	case IJ_HXPBJ:
		common_dev_type = hx_type_map(exttype);
		break;
	case IJ_TEST_DEV:
		common_dev_type = ij_test_type_map(exttype);
		break;
	case IJ_101_OEM:
		common_dev_type = DEV_WUNENG_OEM;
		break;
	case IJ_RFGW:
		common_dev_type = Rf_type_map(exttype);
		break;
	case IJ_HEATER_DEV:
		common_dev_type = bimar_type_map(exttype);
		break;
	case IJ_838_YT:
		common_dev_type = yt_type_map(exttype);
		break;
	case IJ_JL_STAGE_LAMP:
		common_dev_type = jl_stage_lamp_type_map(exttype);
		break;
	case IJ_839_ADS:
		common_dev_type = ads_map(exttype);
		break;
	case IJ_JS_MICWAVE:
		common_dev_type = js_wave_oven_map(exttype);
		break;
	case IJ_KXM_DEVICE:
		common_dev_type = kxm_map(exttype);
		break;
	case IJ_EVM:
		common_dev_type = evm_map(exttype);
		break;
	case IJ_INDIACAR:
		common_dev_type = indiacar_map(exttype);
		break;
	case IJ_ZHCL:
		common_dev_type = DEV_ZHCL;
		break;
	case IJ_LEIS:
		common_dev_type = leis_map(exttype);
		break;
	case IJ_ZHDHX:
		common_dev_type = DEV_ZH_DHX;
		break;
			
	default:
		break;
	}
	return common_dev_type;
}

void copyUdpDev(JNIEnv* env, jclass class_dev_info, jobject obj_dev_info, cl_com_udp_device_data *comm_info)
{
	void * dev_info = comm_info->device_info;

	if (dev_info == NULL) {
		LOGE("common udp info: device_info is NULL\n");
		return;
	}

	LOGE("common udp info : copy udp dev type, subtype = %d, exttype = %d\n", 
			comm_info->sub_type, comm_info->ext_type);

	int dev_map_type = typeMap(comm_info->sub_type, comm_info->ext_type);

	
	LOGE("dev_map_type = %d\n", dev_map_type);
	switch(dev_map_type) {
	case DEV_WUKONG:
		break;
	case DEV_CZWK:
		copy_car_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_WUNENG_USB:
		break;
	case DEV_WUNENG:
		break;
	case DEV_JNB_INTER:
		break;
	case DEV_JNB_DRIP:
		break;
	case DEV_FOXCONN_PLUG:
		break;
	case DEV_HTC_HP:
		copy_tb_heater_pump(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;

	case DEV_TMC_JNB:
		copy_udp_tmc_jnb(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_TMC_YL:
		copy_udp_tmc_yl(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_PDC_JCX:
		copy_udp_pdc_jcx(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_LEDE_TGD:
		copy_lede_lamp(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_AMT_FAN:
		copy_amt_fan(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_CHIFFO_FLOOR_HEATER:
		copy_chiffo(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_TELIN_HEATING:
		copy_tl_heating_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_QP_CP:
		copy_qp_cp_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_QP_PAN:
		copy_qp_pan_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_QP_POBIJI:
		copy_qp_pobiji_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_POBIJI:
		copy_hx_pobiji_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_WUNENG_OEM:
		copy_wuneng_oem_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_BIMAR_HEATER:
	case DEV_AUCMA_HEATER:
		copy_bimar_heater_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_XYWKQ:
		copy_xywkq_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_RF_GW_INFO:
		copy_rf_gw_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_TB_POOL:
	case DEV_TB_COMMERCIAL:
		copy_tb_commercial_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_HX_POT:
		copy_hx_pot(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_YT_AIRPLUG:
		copy_yt_airplug(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_JL_STAGE_LAMP:
		copy_jl_stage_lamp(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_ADS:
		copy_ads_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_JS_WAVE_OVEN:
		copy_js_wave_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_KXM_HEATER_PUMP:
		copy_kxm_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_KXM_HEATER_PUMP_2:
		copy_kxm_thermost_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_SBT_THERMOSTAT:
		copy_sbt_thermostat_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_ZSSX_OVEN:
		copy_zssx_oven_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_YJ_HEATER:
		copy_yj_heater_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_YY_WATER_CLEANER:
		copy_yy_cleaner_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_ZK_WATER_HEATER:
		copy_zk_water_heater_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_INDIA_CAR:
		copy_india_car_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_ZK_CLEANNER:
		copy_zk_cleanner_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_HY_THERMOSTAT:
		copy_hy_thermostat_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_BP_THERMOSTAT:
		copy_bp_thermostat_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_GM_HEATER:
		copy_jrx_heater_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_LINKON_THERMOSTAT:
		copy_linkon_thermostat_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_CJ_THERMOSTAT:
		copy_cj_thermostat_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_YL_RFGW:
		copy_yl_rfgw_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_DRKZQ:
		copy_drkzq_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_ZHCL:
		copy_zhcl_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_LEIS:
		copy_leis_lamp_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;	
	case DEV_YINSU:
		copy_yinsu_lamp_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
	case DEV_ZH_DHX:
		copy_zh_dhx_info(env, class_dev_info, obj_dev_info, comm_info->device_info);
		break;
		
	default:
		LOGE("common udp info : copy udp dev type mismatch, subtype = %d, exttype = %d\n", 
			comm_info->sub_type, comm_info->ext_type);
		break;
	}
}
