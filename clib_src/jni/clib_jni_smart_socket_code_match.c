#include "clib_jni.h"
#include "clib_jni_sdk.h"
#include <assert.h>

JNIEXPORT jint JNICALL
NAME(ClRefresh24HourLine) (JNIEnv *env, jobject this, jint dev_handle, jbyte type)
{
	return	cl_com_udp_refresh_24hour_line(dev_handle, type);
}

JNIEXPORT jint JNICALL
NAME(ClRcStartNextMatch) (JNIEnv *env, jobject this, jint dev_handle, jbyte rc_id, jint time_out, jbyte key_id)
{
	return	cl_rc_start_next_key_match(dev_handle, rc_id, time_out, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClRcChangeName) (JNIEnv *env, jobject this, jint dev_handle, jstring name)
{
	char *str;
	int ret;
	
	str = (char *)((*env)->GetStringUTFChars(env, name, NULL));
	ret = cl_rc_change_name(dev_handle, str);
	(*env)->ReleaseStringUTFChars(env, name, str);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRcStartMatch) (JNIEnv *env, jobject this, jint dev_handle, jbyte rc_id, jint time_out)
{
	return	cl_rc_start_match(dev_handle, rc_id, time_out);
}

JNIEXPORT jint JNICALL
NAME(ClRcStopMatch) (JNIEnv *env, jobject this, jint dev_handle, jbyte rc_id)
{
	return cl_rc_stop_match(dev_handle, rc_id);
}

JNIEXPORT jobject JNICALL
NAME(ClRcGetMatchStat) (JNIEnv *env, jobject this, jint dev_handle)
{
	jclass clazz = NULL;
	jobject obj = NULL;
	cl_rc_match_stat_t rc_match_stat_t_p = {0};
	int ret;
	
	ret = cl_rc_get_match_stat(dev_handle, &rc_match_stat_t_p);
	if (ret == RS_OK) {
		clazz = (*env)->FindClass(env, CLASS_AIR_PLUG_RC_MATCH_STAT);
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
NAME(ClRcGetLearnStat) (JNIEnv *env, jobject this, jint dev_handle)
{
	jclass clazz = NULL;
	jobject obj = NULL;
	cl_rc_key_learn_stat_t rc_key_learn_stat_t_p = {0};	
	int ret;
	
	ret = cl_rc_get_learn_stat(dev_handle, &rc_key_learn_stat_t_p);
	if (ret == RS_OK) {
		clazz = (*env)->FindClass(env, CLASS_AIR_PLUG_RC_KEY_LEARN_STAT);
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

JNIEXPORT jint JNICALL
NAME(ClRcCtrlKey) (JNIEnv *env, jobject this, jint dev_handle, jbyte rc_id, jbyte key_id)
{
	return cl_rc_ctrl_key(dev_handle, rc_id, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClRcQuickCtrlOnoff) (JNIEnv *env, jobject this, jint dev_handle, jbyte tv_id, jbyte rc_id)
{
	return cl_rc_quick_ctrl_onoff(dev_handle, tv_id, rc_id);
}

JNIEXPORT jboolean JNICALL
NAME(ClAcQuerySameCode) (JNIEnv* env, jobject this, jint dev_handle)
{
	u_int8_t status_p;
	cl_ac_query_samecode(dev_handle, &status_p);
	return !!status_p;
}

JNIEXPORT jint JNICALL
NAME(ClAcModifyMatchOnoffStatus) (JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_ac_modify_onoff_status(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClAcModifyTempCurve) (JNIEnv* env, jobject this, jint dev_handle, jobject j_temp_curve)
{
	jfieldID fid;
	jclass obj_class, inner_class;
	jobject j_curves, j_curve;
	int ret, i;
	u_int8_t count;

	cl_temp_curve_t *ptmp_curve_p = NULL;
	tmp_curve_t *tmp_curve_t_p = NULL;

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CURVE);
	inner_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CURVE_LINE);
	fid = (*env)->GetFieldID(env, obj_class, "curves", "[L"CLASS_AIR_PLUG_TEMP_CURVE_LINE";");
	j_curves = (*env)->GetObjectField(env, j_temp_curve, fid);

	fid = (*env)->GetFieldID(env, obj_class, "count", "B");
	count = (*env)->GetByteField(env, j_temp_curve, fid);
	
	ptmp_curve_p= (cl_temp_curve_t *)malloc(sizeof(cl_temp_curve_t) + sizeof(tmp_curve_t) * count);
	tmp_curve_t_p = (tmp_curve_t *) (ptmp_curve_p + 1);

	fid = (*env)->GetFieldID(env, obj_class, "id", "B");
	ptmp_curve_p->id = (*env)->GetByteField(env, j_temp_curve, fid);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	ptmp_curve_p->enable = (*env)->GetBooleanField(env, j_temp_curve, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "B");
	ptmp_curve_p->week = (*env)->GetByteField(env, j_temp_curve, fid);

	fid = (*env)->GetFieldID(env, obj_class, "begin_hour", "B");
	ptmp_curve_p->begin_hour= (*env)->GetByteField(env, j_temp_curve, fid);

	fid = (*env)->GetFieldID(env, obj_class, "end_hour", "B");
	ptmp_curve_p->end_hour= (*env)->GetByteField(env, j_temp_curve, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "time_period", "B");
	ptmp_curve_p->time_period= (*env)->GetByteField(env, j_temp_curve, fid);
	
	ptmp_curve_p->count= count;
	for (i = 0; i < count; i++, tmp_curve_t_p++) {
		j_curve = (*env)->GetObjectArrayElement(env, j_curves, i);

		fid = (*env)->GetFieldID(env, inner_class, "flag", "B");
		tmp_curve_t_p->flag = (*env)->GetByteField(env, j_curve, fid);
		fid = (*env)->GetFieldID(env, inner_class, "tmp", "B");
		tmp_curve_t_p->tmp = (*env)->GetByteField(env, j_curve, fid);

		SAFE_DEL_LOCAL_REF(j_curve);
	}

	ret = cl_sa_modify_temp_curve(dev_handle, ptmp_curve_p);

	free(ptmp_curve_p);
	SAFE_DEL_LOCAL_REF(obj_class);
	SAFE_DEL_LOCAL_REF(inner_class);
	SAFE_DEL_LOCAL_REF(j_curves);
	SAFE_DEL_LOCAL_REF(j_temp_curve);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClAcSetTempCurve) (JNIEnv* env, jobject this, jint dev_handle, jobject curve_ctrl)
{
	jclass class_curve = NULL;
	jfieldID fid = NULL;
	jclass class_node = NULL;
	jobjectArray obj_nodes = NULL;
	jobject obj_node = NULL;
	cl_temp_curve_t *c_curve_ctrl = NULL;
	tmp_curve_t *c_node = NULL;
	int count = 0;
	int i;

	class_curve = (*env)->GetObjectClass(env, curve_ctrl);
	fid = (*env)->GetFieldID(env, class_curve, "nodes", "[L"CLASS_SDK_AC_CURVE_NODE";");
	obj_nodes = (*env)->GetObjectField(env, curve_ctrl, fid);
	if (obj_nodes == NULL) {
		return RS_INVALID_PARAM;
	}
	count = (*env)->GetArrayLength(env, obj_nodes);
	c_curve_ctrl = malloc(sizeof(cl_temp_curve_t) + count * sizeof(tmp_curve_t));
	c_node = (tmp_curve_t *)(c_curve_ctrl + 1);
	
	jni_assign_simple_struct(env, curve_ctrl, class_curve,
		                        ASSIGN_TRIPLES(byte, c_curve_ctrl, id),
		                        ASSIGN_TRIPLES(boolean, c_curve_ctrl, enable),
		                        ASSIGN_TRIPLES(byte, c_curve_ctrl, week),
		                        ASSIGN_TRIPLES(byte, c_curve_ctrl, begin_hour),
		                        ASSIGN_TRIPLES(byte, c_curve_ctrl, end_hour),
		                        ASSIGN_TRIPLES(byte, c_curve_ctrl, time_period),
		                        JNI_VAR_ARG_END);

	class_node = (*env)->FindClass(env, CLASS_SDK_AC_CURVE_NODE);
	for (i = 0; i < count; i++, c_node++) {
		obj_node = (*env)->GetObjectArrayElement(env, obj_nodes, i);
		jni_assign_simple_struct(env, obj_node, class_node,
			                        ASSIGN_TRIPLES(byte, c_node, flag),
			                        ASSIGN_TRIPLES(byte, c_node, tmp),
			                        ASSIGN_TRIPLES(byte, c_node, wind),
			                        ASSIGN_TRIPLES(byte, c_node, dir),
			                        JNI_VAR_ARG_END);
		
		SAFE_DEL_LOCAL_REF(obj_node);
	}
	SAFE_DEL_LOCAL_REF(obj_nodes);
	SAFE_DEL_LOCAL_REF(class_node);
	return cl_sa_modify_temp_curve(dev_handle, c_curve_ctrl);
}


JNIEXPORT jint JNICALL
NAME(ClAcSetTempCtrlV2) (JNIEnv* env, jobject this, jint dev_handle, jobject temp_ctrl)
{
	jclass class_temp = NULL;
	jfieldID fid = NULL;
	cl_temp_ac_ctrl_t c_temp_ctrl;

	class_temp = (*env)->GetObjectClass(env, temp_ctrl);
	jni_assign_simple_struct(env, temp_ctrl, class_temp,
		                        ASSIGN_TRIPLES(boolean, &c_temp_ctrl, enable),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, mode),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, temp_min),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, temp_max),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, week),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, begin_hour),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, end_hour),
		                        JNI_VAR_ARG_END);
	return cl_ac_set_temp_ctrl(dev_handle, &c_temp_ctrl);
}

JNIEXPORT jint JNICALL
NAME(ClAcSetTempCtrl) (JNIEnv* env, jobject this, jint dev_handle, jobject j_temp_ctrl)
{
	jfieldID fid;
	jclass obj_class ;
	cl_temp_ac_ctrl_t ptmp_ctrl = {0};

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CTRL);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	ptmp_ctrl.enable = (*env)->GetBooleanField(env, j_temp_ctrl, fid);

	fid = (*env)->GetFieldID(env, obj_class, "mode", "B");
	ptmp_ctrl.mode = (*env)->GetByteField(env, j_temp_ctrl, fid);

	fid = (*env)->GetFieldID(env, obj_class, "temp_min", "B");
	ptmp_ctrl.temp_min= (*env)->GetByteField(env, j_temp_ctrl, fid);

	fid = (*env)->GetFieldID(env, obj_class, "temp_max", "B");
	ptmp_ctrl.temp_max= (*env)->GetByteField(env, j_temp_ctrl, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "B");
	ptmp_ctrl.week= (*env)->GetByteField(env, j_temp_ctrl, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "begin_hour", "B");
	ptmp_ctrl.begin_hour= (*env)->GetByteField(env, j_temp_ctrl, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "end_hour", "B");
	ptmp_ctrl.end_hour= (*env)->GetByteField(env, j_temp_ctrl, fid);
	
	return cl_ac_set_temp_ctrl(dev_handle, &ptmp_ctrl);
}


JNIEXPORT jint JNICALL
NAME(ClAcSetPanelType) (JNIEnv* env, jobject this, jint dev_handle, jbyte pan_type)
{
	return cl_ac_set_pan_type(dev_handle, pan_type);
}

JNIEXPORT jint JNICALL
NAME(ClAcRefreshKeyInfo) (JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_ac_refresh_key_info(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClAcSetKeyInfo) (JNIEnv* env, jobject this, jint dev_handle, jbyte key_id, jstring key_name)
{
	char *name = NULL;
	int ret;
	
	name = (char *)((*env)->GetStringUTFChars(env, key_name, NULL));
		
	if(NULL != name) {
		ret = cl_ac_set_key_info(dev_handle, key_id, name);
		(*env)->ReleaseStringUTFChars(env, key_name, name);
	} else {
		ret = RS_INVALID_PARAM;
	}
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClAcSetKeyInfoV2) (JNIEnv* env, jobject this, jint dev_handle, jbyte key_id, jstring key_name, jbyte flag, jobject stat)
{
	jfieldID fid;
	jclass obj_class ;

	char *name = NULL;
	int ret = RS_INVALID_PARAM;
	air_key_stat_t key_stat = {0};
	
	name = (char *)((*env)->GetStringUTFChars(env, key_name, NULL));

	if(NULL != name && NULL != stat) {
		obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_KEY_STAT);

		fid = (*env)->GetFieldID(env, obj_class, "onoff", "B");
		key_stat.onoff= (*env)->GetByteField(env, stat, fid);
		
		fid = (*env)->GetFieldID(env, obj_class, "mode", "B");
		key_stat.mode= (*env)->GetByteField(env, stat, fid);
		
		fid = (*env)->GetFieldID(env, obj_class, "temp", "B");
		

		key_stat.temp= (*env)->GetByteField(env, stat, fid);
		LOGE("key_temp=%d", key_stat.temp);

		
		fid = (*env)->GetFieldID(env, obj_class, "wind", "B");
		key_stat.wind= (*env)->GetByteField(env, stat, fid);
		
		fid = (*env)->GetFieldID(env, obj_class, "wind_direct", "B");
		key_stat.wind_direct= (*env)->GetByteField(env, stat, fid);
		
		fid = (*env)->GetFieldID(env, obj_class, "key", "B");
		key_stat.key= (*env)->GetByteField(env, stat, fid);
		
		(*env)->DeleteLocalRef(env, obj_class);
	}

	ret = cl_ac_set_key_info_v2(dev_handle, key_id, name, flag, key_stat);

	if (NULL != name) {
		(*env)->ReleaseStringUTFChars(env, key_name, name);
	}
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClAcDeleteKey) (JNIEnv* env, jobject this, jint dev_handle, jbyte key_id)
{
	return cl_ac_delete_key(dev_handle, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClAcStartLearnKey) (JNIEnv* env, jobject this, jint dev_handle, jbyte key_id)
{
	return cl_ac_start_learn_key(dev_handle, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClAcStopLearnKey) (JNIEnv* env, jobject this, jint dev_handle, jbyte key_id)
{
	return cl_ac_stop_learn_key(dev_handle, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClAcCtrKey) (JNIEnv* env, jobject this, jint dev_handle, jbyte key_id)
{
	return cl_ac_ctrl_key(dev_handle, key_id);
}



JNIEXPORT jint JNICALL
NAME(ClSaAirRestIrCodeId)(JNIEnv* env, jobject this, jint dev_handle, jint code_id)
{
	return cl_sa_air_reset_ir_code_id(dev_handle, code_id);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirCtrlPower)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	
	return cl_sa_air_ctrl_power(dev_handle, !!onoff);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirCtrlMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode)
{
	
	return cl_sa_air_ctrl_mode(dev_handle, mode);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirCtrlTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte temp)
{
	
	return cl_sa_air_ctrl_temp(dev_handle, temp);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirCtrlWind)(JNIEnv* env, jobject this, jint dev_handle,  jbyte wind)
{
	
	return cl_sa_air_ctrl_wind(dev_handle, wind);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirCtrlWindDirect)(JNIEnv* env, jobject this, jint dev_handle, jbyte wind_direct)
{
	
	return cl_sa_air_ctrl_direct(dev_handle, wind_direct);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirCtrlLedPower)(JNIEnv* env, jobject this, jint dev_handle, jint onoff)
{
	
	return cl_sa_ctrl_led_power(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetPeakTime)(JNIEnv* env, jobject this, jint dev_handle, jshort begin_time, jshort last_min)
{
	
	return cl_sa_set_air_peak_time( dev_handle,  begin_time,  last_min);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetValleyTime)(JNIEnv* env, jobject this, jint dev_handle, jshort begin_time, jshort last_min)
{
	
	return cl_sa_set_air_valley_time( dev_handle,  begin_time,  last_min);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetPeakPrice)(JNIEnv* env, jobject this, jint dev_handle, jint price)
{
	
	return cl_sa_set_air_peak_price( dev_handle,  price);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetValleyPrice)(JNIEnv* env, jobject this, jint dev_handle, jint price)
{
	
	return cl_sa_set_air_valley_price( dev_handle,  price);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetNormalPrice)(JNIEnv* env, jobject this, jint dev_handle, jint price)
{
	
	 return 0;
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartPowerON)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	
	return cl_sa_set_smart_power_on( dev_handle,  !!onoff);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirDeleteTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte timer_id)
{
	
	return cl_sa_del_air_timer( dev_handle,  timer_id);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirDelPeriodTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte timer_id)
{
	
	return cl_sa_del_period_timer( dev_handle,  timer_id);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirModifyPeriodTimerV2)(JNIEnv* env, jobject this, jint dev_handle,jobject timer_info)
{
	cl_period_timer_t ti = {0};
	jfieldID fid;
	jclass obj_class ;

	if(timer_info == NULL)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_SDK_PERIOD_TIMER);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "id", "B");
	ti.id = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "hour", "B");
	ti.hour = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "minute", "B");
	ti.minute = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "B");
	ti.week = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	ti.enable = (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	ti.onoff= (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "duration", "S");
	ti.duration = (*env)->GetShortField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "ext_data_type", "S");
	ti.ext_data_type = (*env)->GetShortField(env, timer_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	(*env)->DeleteLocalRef(env, timer_info);

	return cl_sa_add_air_period_timer( dev_handle, &ti);
}


JNIEXPORT jint JNICALL
NAME(ClSaAirModifyPeriodTimer)(JNIEnv* env, jobject this, jint dev_handle,jobject timer_info)
{
	cl_period_timer_t ti = {0};
	jfieldID fid;
	jclass obj_class ;

	if(timer_info == NULL)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_PERIOD_TIMER);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "id", "I");
	ti.id = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "hour", "I");
	ti.hour = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "minute", "I");
	ti.minute = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "I");
	ti.week = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	ti.enable = (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	ti.onoff= (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "duration", "I");
	ti.duration = (*env)->GetIntField(env, timer_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	(*env)->DeleteLocalRef(env, timer_info);

	return cl_sa_add_air_period_timer( dev_handle, &ti);
}


JNIEXPORT jint JNICALL
NAME(ClSaAirModifyTimerV2)(JNIEnv* env, jobject this, jint dev_handle,jobject timer_info)
{
	cl_air_timer_t ti = {0};
	jfieldID fid;
	jclass obj_class ;

	
	
	if(timer_info == NULL)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_SDK_BASE_TIMER);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "id", "B");
	assert(fid!=NULL);
	ti.id = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	assert(fid!=NULL);

	ti.enable = (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "B");
	assert(fid!=NULL);
	ti.week = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "hour", "B");
	assert(fid!=NULL);
	ti.hour = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "minute", "B");
	assert(fid!=NULL);
	ti.minute = (*env)->GetByteField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	assert(fid!=NULL);
	ti.onoff= (*env)->GetBooleanField(env, timer_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	(*env)->DeleteLocalRef(env, timer_info);
	
	
	return cl_sa_add_air_timer( dev_handle, &ti);
}


JNIEXPORT jint JNICALL
NAME(ClSaAirModifyTimer)(JNIEnv* env, jobject this, jint dev_handle,jobject timer_info)
{
	cl_air_timer_t ti = {0};
	jfieldID fid;
	jclass obj_class ;

	
	
	if(timer_info == NULL)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TIMER);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "id", "I");
	assert(fid!=NULL);
	ti.id = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "enable", "Z");
	assert(fid!=NULL);

	ti.enable = (*env)->GetBooleanField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "week", "I");
	assert(fid!=NULL);
	ti.week = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "hour", "I");
	assert(fid!=NULL);
	ti.hour = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "minute", "I");
	assert(fid!=NULL);
	ti.minute = (*env)->GetIntField(env, timer_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	assert(fid!=NULL);
	ti.onoff= (*env)->GetBooleanField(env, timer_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	(*env)->DeleteLocalRef(env, timer_info);
	
	
	return cl_sa_add_air_timer( dev_handle, &ti);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartPowerOFF)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	
	return cl_sa_set_smart_power_off( dev_handle,  !!onoff);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartSleep)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	
	return cl_sa_set_smart_sleep( dev_handle,  !!onoff);
}
	
JNIEXPORT jint JNICALL
NAME(ClStartCodeMatch)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_all_match, jint time_out)
{
	
	return cl_sa_start_code_match(dev_handle, is_all_match, time_out);
}


JNIEXPORT jint JNICALL
NAME(ClStopCodeMatch)(JNIEnv* env, jobject this, jint dev_handle)
{
	
	return cl_sa_stop_code_match(dev_handle);
}

JNIEXPORT jobject JNICALL
NAME(ClGetCodeMatchStateV2)(JNIEnv* env, jobject this, jint dev_handle)
{
	jclass class_match_state;
	jobject obj_state = NULL;
	cl_air_code_match_stat_t  *cl_code_match_p = NULL;
	
	
	
	class_match_state = (*env)->FindClass(env, CLASS_SDK_AC_MATCH_STATE);		
	
	
	cl_code_match_p = (cl_air_code_match_stat_t *)malloc(sizeof(cl_air_code_match_stat_t));
	if(cl_code_match_p == NULL){
		goto done;
	}
	obj_state = (*env)->AllocObject(env, class_match_state);
	cl_sa_get_code_match_stat(dev_handle, cl_code_match_p);	
	
	jniCopyIntValue(env, class_match_state, "action", obj_state, cl_code_match_p->action);
	jniCopyBooleanValue(env, class_match_state, "is_cloud_matching", obj_state, !!(cl_code_match_p->is_cloud_matching));
	jniCopyIntValue(env, class_match_state, "cur_step", obj_state, cl_code_match_p->cur_step);
	jniCopyIntValue(env, class_match_state, "max_step", obj_state, cl_code_match_p->max_step);
	jniCopyIntValue(env, class_match_state, "error", obj_state, cl_code_match_p->error);
	
done:
	SAFE_DEL_LOCAL_REF(class_match_state);
	if (cl_code_match_p) {
		free(cl_code_match_p);
		cl_code_match_p = NULL;
	}
	
	return obj_state;
}

JNIEXPORT jobject JNICALL
NAME(ClGetCodeMatchState)(JNIEnv* env, jobject this, jint dev_handle)
{	
	jclass class_match_state;
	jobject obj_state = NULL;
	cl_air_code_match_stat_t  *cl_code_match_p = NULL;
	
	
	
	class_match_state = (*env)->FindClass(env, CLASS_CODE_MATCH_STATE);		
	if(NULL == class_match_state){
		
		return class_match_state;
	}
	
	cl_code_match_p = (cl_air_code_match_stat_t *)malloc(sizeof(cl_air_code_match_stat_t));
	if(!cl_code_match_p){
		(*env)->DeleteLocalRef(env, class_match_state);
		
		return cl_code_match_p;
	}
	
	cl_sa_get_code_match_stat(dev_handle, cl_code_match_p);	

	obj_state = (*env)->AllocObject(env, class_match_state);
	if(!obj_state){
		free(cl_code_match_p);
		(*env)->DeleteLocalRef(env, class_match_state);
		
		return obj_state;
	}
	
	jniCopyIntValue(env, class_match_state, "action", obj_state, cl_code_match_p->action);
	jniCopyBooleanValue(env, class_match_state, "is_cloud_matching", obj_state, !!(cl_code_match_p->is_cloud_matching));
	jniCopyIntValue(env, class_match_state, "cur_step", obj_state, cl_code_match_p->cur_step);
	jniCopyIntValue(env, class_match_state, "max_step", obj_state, cl_code_match_p->max_step);
	jniCopyIntValue(env, class_match_state, "error", obj_state, cl_code_match_p->error);
	jniCopyIntValue(env, class_match_state, "flag", obj_state, cl_code_match_p->flag);

done:
	free(cl_code_match_p);
	(*env)->DeleteLocalRef(env, class_match_state);
	
	return obj_state;
}

/*
 功能:
    获取
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    @rdata 实时数据
 返回:
收到 SAE_DEV_POWER_NOTIFY 调用
 其他: 失败
 */
//CLIB_API RS cl_sa_air_get_real_time_data(cl_handle_t dev_handle,cl_air_real_time_data_t* rdata);
JNIEXPORT jobject JNICALL
NAME(ClAirGetRealTimeData)(JNIEnv* env, jobject this, jint dev_handle )
{
	cl_air_real_time_data_t rdata = {0};
	jclass class_real_time_info;
	jobject obj_rt;

	if (cl_sa_air_get_real_time_data(dev_handle, &rdata) != RS_OK) {
		return NULL;
	}
	
	class_real_time_info = (*env)->FindClass(env, CLASS_REAL_TIME_INFO);
	if(!class_real_time_info)
		return NULL;
	
	obj_rt = (*env)->AllocObject(env, class_real_time_info);
	if(!obj_rt)
		return NULL;
	
	jniCopyIntValue(env, class_real_time_info, "cur_power", obj_rt, rdata.cur_power);
	(*env)->DeleteLocalRef(env, class_real_time_info);
	
	return obj_rt;
}


/*
 功能:
    获取空调当前功率
 输入参数:
    @time_interval
 输出参数:
    无
 返回:
    RS_OK: 成功
 */
//CLIB_API RS cl_sa_air_start_get_cur_power(cl_handle_t dev_handle,u_int8_t time_interval);
JNIEXPORT jint JNICALL
NAME(ClAirStartGetCurPower)(JNIEnv* env, jobject this, jint dev_handle, jint seconds)
{
	return cl_sa_air_start_get_cur_power( dev_handle, seconds );
}

    
/*
 功能:
    停止获取空调当前功率
 输入参数:
    @time_interval
 输出参数:
    无
 返回:
    RS_OK: 成功
 */
//CLIB_API RS cl_sa_air_stop_get_cur_power(cl_handle_t dev_handle);
JNIEXPORT jint JNICALL
NAME( ClAirStopGetCurPower )(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_sa_air_stop_get_cur_power( dev_handle );
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartPowerOffParam)(JNIEnv* env, jobject this, jint dev_handle, jobject smart_off_info)
{
	cl_smart_air_off_param_t air_off = {0};
	jfieldID fid;
	jclass obj_class ;

	if(smart_off_info == NULL || dev_handle == 0)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_SDK_SMART_OFF_PARAM);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "on", "Z");
	assert(fid!=NULL);
	air_off.on = (*env)->GetBooleanField(env, smart_off_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "push_on", "Z");
	assert(fid!=NULL);
	air_off.push_on= (*env)->GetBooleanField(env, smart_off_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "off_time", "B");
	assert(fid!=NULL);
	air_off.off_time= (*env)->GetByteField(env, smart_off_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	
	return cl_sa_set_smart_power_off_detail(dev_handle,&air_off);
}



JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartPowerOFFDetail)(JNIEnv* env, jobject this, jint dev_handle, jobject smart_off_info)
{
	cl_smart_air_off_param_t air_off = {0};
	jfieldID fid;
	jclass obj_class ;

	if(smart_off_info == NULL || dev_handle == 0)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_SMART_OFF);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "on", "Z");
	assert(fid!=NULL);
	air_off.on = (*env)->GetBooleanField(env, smart_off_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "push_on", "Z");
	assert(fid!=NULL);
	air_off.push_on= (*env)->GetBooleanField(env, smart_off_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "off_time", "I");
	assert(fid!=NULL);
	air_off.off_time= (*env)->GetIntField(env, smart_off_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	
	return cl_sa_set_smart_power_off_detail(dev_handle,&air_off);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartSleepParam)(JNIEnv* env, jobject this, jint dev_handle,jobject smart_sleep_info)
{
	cl_smart_air_sleep_param_t air_sleep = {0};
	jfieldID fid;
	jclass obj_class ;

	if(smart_sleep_info == NULL || dev_handle == 0)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_SDK_SMART_SLEEP_PARAM);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "on", "Z");
	assert(fid!=NULL);
	air_sleep.on = (*env)->GetBooleanField(env, smart_sleep_info, fid);


	(*env)->DeleteLocalRef(env, obj_class);
	
	
	return cl_sa_set_smart_sleep_detail(dev_handle,&air_sleep);
}


JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartSleepDetail)(JNIEnv* env, jobject this, jint dev_handle,jobject smart_sleep_info)
{
	cl_smart_air_sleep_param_t air_sleep = {0};
	jfieldID fid;
	jclass obj_class ;

	if(smart_sleep_info == NULL || dev_handle == 0)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_SMART_SLEEP);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "on", "Z");
	assert(fid!=NULL);
	air_sleep.on = (*env)->GetBooleanField(env, smart_sleep_info, fid);


	(*env)->DeleteLocalRef(env, obj_class);
	
	
	return cl_sa_set_smart_sleep_detail(dev_handle,&air_sleep);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartOnParam)(JNIEnv* env, jobject this, jint dev_handle, jobject smart_on_info)
{
	cl_smart_air_on_param_t air_on = {0};
	jfieldID fid;
	jclass obj_class ;

	if(smart_on_info == NULL || dev_handle == 0)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_SDK_SMART_ON_PARAM);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "on", "Z");
	assert(fid!=NULL);
	air_on.on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "push_on", "Z");
	assert(fid!=NULL);
	air_on.push_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "sum_on", "Z");
	assert(fid!=NULL);
	air_on.sum_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "win_on", "Z");
	assert(fid!=NULL);
	air_on.win_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "home_on", "Z");
	assert(fid!=NULL);
	air_on.home_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "win_tmp", "B");
	assert(fid!=NULL);
	air_on.win_tmp = (*env)->GetByteField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "sum_tmp", "B");
	assert(fid!=NULL);
	air_on.sum_tmp = (*env)->GetByteField(env, smart_on_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	
	
	return cl_sa_set_smart_power_on_detail(dev_handle,&air_on);
}	


JNIEXPORT jint JNICALL
NAME(ClSaAirSetSmartOnDetail)(JNIEnv* env, jobject this, jint dev_handle, jobject smart_on_info)
{
	cl_smart_air_on_param_t air_on = {0};
	jfieldID fid;
	jclass obj_class ;

	if(smart_on_info == NULL || dev_handle == 0)
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_SMART_ON);
	assert(obj_class!=NULL);

	fid = (*env)->GetFieldID(env, obj_class, "on", "Z");
	assert(fid!=NULL);
	air_on.on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "push_on", "Z");
	assert(fid!=NULL);
	air_on.push_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "sum_on", "Z");
	assert(fid!=NULL);
	air_on.sum_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "win_on", "Z");
	assert(fid!=NULL);
	air_on.win_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "home_on", "Z");
	assert(fid!=NULL);
	air_on.home_on = (*env)->GetBooleanField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "win_tmp", "I");
	assert(fid!=NULL);
	air_on.win_tmp = (*env)->GetIntField(env, smart_on_info, fid);

	fid = (*env)->GetFieldID(env, obj_class, "sum_tmp", "I");
	assert(fid!=NULL);
	air_on.sum_tmp = (*env)->GetIntField(env, smart_on_info, fid);

	(*env)->DeleteLocalRef(env, obj_class);
	
	
	return cl_sa_set_smart_power_on_detail(dev_handle,&air_on);
}	

JNIEXPORT jint JNICALL
NAME(ClStartSmartSocketConfig)(JNIEnv* env, jobject this, jstring ssid, jstring passwd)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;
	
	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);

	ret = cl_smart_6621_config_start(ssid_str,password);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRefreshAirPlugTimer)(JNIEnv* env, jobject this,  jint dev_handle)
{
	return cl_sa_air_refresh_timer_info( dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClRefreshElecInfo)(JNIEnv* env, jobject this,  jint dev_handle)
{
	return cl_sa_air_refresh_elec_info( dev_handle);
}
JNIEXPORT jint JNICALL
NAME(ClElecClear)(JNIEnv* env, jobject this,  jint dev_handle, jint type)
{
	return cl_sa_air_clear_elec_stat_info( dev_handle, type);
}

JNIEXPORT jint JNICALL
NAME(ClSaAirSetLedColor)(JNIEnv* env, jobject this,  jint dev_handle, jint on_color, jint off_color)
{
	return cl_sa_air_set_led_color(dev_handle, on_color,off_color );
}

JNIEXPORT jint JNICALL
NAME(ClSaSetFanSpeedOpposite)(JNIEnv* env, jobject this,  jint dev_handle, jboolean is_opposite)
{
	return cl_ac_set_fan_speed_opposite(dev_handle, !!is_opposite);
}

JNIEXPORT jint JNICALL
NAME(ClSaSetFanStat)(JNIEnv* env, jobject this,  jint dev_handle, jbyte fan_stat)
{
	return cl_ac_set_fan_stat(dev_handle,fan_stat);
}

JNIEXPORT jint JNICALL
NAME(ClSaSetChildLockStat)(JNIEnv* env, jobject this,  jint dev_handle, jbyte lock_stat)
{
	return cl_ac_set_child_lock_stat(dev_handle,lock_stat);
}

JNIEXPORT jint JNICALL
NAME(ClSaSetAirMsgConfig)(JNIEnv* env, jobject this,  jint dev_handle, jobject config)
{
	jfieldID fid;	
	jclass obj_class ;	
	LOGE("ClLEDEOnStateConfig======");
	int ret;
	cl_ac_msg_config_t msg_config = {0};
	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG_MSG_CONFIG);

	fid = (*env)->GetFieldID(env, obj_class, "onoff", "Z");
	msg_config.onoff= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "timer", "Z");
	msg_config.timer= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "sync", "Z");
	msg_config.sync= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "temp_ctrl", "Z");
	msg_config.temp_ctrl= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "curve", "Z");
	msg_config.curve= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "sleep", "Z");
	msg_config.sleep= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "smart_on", "Z");
	msg_config.smart_on= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "poweron_restore", "Z");
	msg_config.poweron_restore= (*env)->GetBooleanField(env, config, fid);
	fid = (*env)->GetFieldID(env, obj_class, "linkage_ctrl", "Z");
	msg_config.linkage_ctrl= (*env)->GetBooleanField(env, config, fid);
	SAFE_DEL_LOCAL_REF(obj_class);		
	ret = cl_ac_set_msg_config(dev_handle, &msg_config);	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRcStartLearn)(JNIEnv* env, jobject this,  jint dev_handle, jbyte rc_id, jbyte key_id)
{
	return cl_rc_start_learn(dev_handle, rc_id, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClRcStopLearn)(JNIEnv* env, jobject this,  jint dev_handle, jbyte rc_id, jbyte key_id)
{
	return cl_rc_stop_learn(dev_handle, rc_id, key_id);
}

JNIEXPORT jint JNICALL
NAME(ClRcDelKey)(JNIEnv* env, jobject this,  jint dev_handle, jbyte rc_id, jbyte key_id)
{
	return cl_rc_delete_key(dev_handle, rc_id, key_id);
}


JNIEXPORT jint JNICALL
NAME(ClRcModKey)(JNIEnv* env, jobject this,  jint dev_handle, jbyte rc_id, jbyte key_id, jstring jname)
{
	char *name = NULL;
	int ret = 0;

	name = (char*)(*env)->GetStringUTFChars(env, jname, NULL);
	ret = cl_rc_modify_key(dev_handle, rc_id, key_id, name);
	(*env)->ReleaseStringUTFChars(env, jname, name);
	return ret;
}

//cl_sa_air_clear_elec_stat_info(cl_handle_t dev_handle,int type)

