#include "clib_jni.h"
#include "clib_jni_sdk.h"
#include "cl_lanusers.h"
#include "cl_app_pkt.h"
#undef	BIT
#define	BIT(n)	(1 << (n))


enum {
	TDEV_HEATING_VALVE,//暖气阀 
	TDEV_CENTRAL_AIRCON,//中央空调
	TDEV_CENTRAL_RF_WUKONG,//空调贴
	TDEV_ZH_MOTOR,//智皇电机
	TDEV_LK_THERMOSTAT,//linkon温控器
	TDEV_ML_DHXML,//魔乐单火线
	TDEV_ZH_DHX,//智皇单火线
	TDEV_MAX
}dev_type_for_timer;

enum {
	TIMER_TYPE_HEATING_VALVE,//暖气阀定时器
	TIMER_TYPE_CENTRAL_AIRCON,//中央空调，用老的
	TIMER_TYPE_RF_WUKONG,//空调贴
	TIMER_TYPE_ZH_MOTOR,//智皇电机
	TIMER_LK_THERMOSTAT,//linkon温控器
	TIMER_ML_DHXML,//魔乐单火线
	TIMER_ZH_DHX,//智皇单火线
	TIMER_TYPE_MAX
}timer_type;

/************************************************************************************
		user interface
 ************************************************************************************/
extern void copyUdpDev(JNIEnv* env, jclass class_dev_info, jobject obj_dev_info, cl_com_udp_device_data *comm_info);
extern int typeMap(int subtype, int exttype);
static void copyCommTimerInfo(JNIEnv* env, jclass class_slave, jobject obj_slave, 
	cl_comm_timer_head_t *comm_timer, cl_dev_timer_summary_t *timer_summary, int sub_type, int ext_type);
static void CopyAirplugHistoryItem(JNIEnv* env,jclass class_parent, jobject obj_parent, u_int64_t sn, cl_dev_history_item_t *history_item);



 
jobject UpdateGetNewestVersion(JNIEnv* env) {
	jclass class_version;
	jobject obj_version;
	jfieldID fid;
	jstring str;
	float f_version;
	
	cl_phone_version_t * version_t = cl_get_phone_version();
	if(!version_t) {
		return NULL;
	}
	
	class_version = (*env)->FindClass(env, CLASS_NEWEST_VERSION_INFO);
	obj_version = (*env)->AllocObject(env, class_version);
	
	f_version = atof(version_t->iphone_newest_version);
	fid = (*env)->GetFieldID(env, class_version, "iphone_newest_version", "F");
	(*env)->SetFloatField(env, obj_version, fid, f_version);
	
	f_version = atof(version_t->android_newest_version);
	fid = (*env)->GetFieldID(env, class_version, "android_newest_version", "F");
	(*env)->SetFloatField(env, obj_version, fid, f_version);

	jniCopyString(env, class_version, "desc_iphone_en", obj_version, version_t->desc_iphone_en);
	
	jniCopyString(env, class_version, "desc_iphone_ch", obj_version, version_t->desc_iphone_ch);
	
	jniCopyString(env, class_version, "desc_android_en", obj_version, version_t->desc_android_en);
	
	jniCopyString(env, class_version, "desc_android_ch", obj_version, version_t->desc_android_ch);
	
	cl_free_phone_version(version_t);
	
	return obj_version;
}

JNIEXPORT jint JNICALL
NAME(ClUserLoginV2)(JNIEnv* env, jobject this, jobject obj_user)
{
	RS ret = RS_ERROR;
	cl_handle_t user_handle;
	const char *str_username;
	const char *str_passwd;
	jstring obj_username;
	jstring obj_passwd;
	jclass clazz = NULL;
	jfieldID fid;

	clazz = (*env)->FindClass(env, CLASS_SDK_USER);
	fid = (*env)->GetFieldID(env, clazz, "username", "Ljava/lang/String;");
	obj_username = (*env)->GetObjectField(env, obj_user, fid);
	str_username = (*env)->GetStringUTFChars(env, obj_username, NULL);

	fid = (*env)->GetFieldID(env, clazz, "passwd", "Ljava/lang/String;");
	obj_passwd = (*env)->GetObjectField(env, obj_user, fid);
	str_passwd = (*env)->GetStringUTFChars(env, obj_passwd, NULL);

	ret = cl_user_login(&user_handle, str_username, str_passwd, nactivCallback, (void *)0);

	fid = (*env)->GetFieldID(env, clazz, "user_handle", "I");
	(*env)->SetIntField(env, obj_user, fid, user_handle);
	
	(*env)->ReleaseStringUTFChars(env, obj_username, str_username);
	(*env)->ReleaseStringUTFChars(env, obj_passwd, str_passwd); 
done:
	if(clazz != NULL)
		(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

NAME(ClSmartConfigLogin)(JNIEnv* env, jobject this, jobject obj_user, 
	jstring obj_username, jstring obj_passwd, jint callback_handle)
{
	RS ret = RS_ERROR;
	cl_handle_t user_handle;
	const char *str_username;
	const char *str_passwd;
	jclass clazz = NULL;
	jfieldID fid;

	str_username = (*env)->GetStringUTFChars(env, obj_username, NULL);
	str_passwd = (*env)->GetStringUTFChars(env, obj_passwd, NULL);

	ret = cl_user_smartconf_login(&user_handle, str_username, str_passwd, nactivCallback, (void *)(jlong)callback_handle);
	(*env)->ReleaseStringUTFChars(env, obj_username, str_username);
	(*env)->ReleaseStringUTFChars(env, obj_passwd, str_passwd); 
	// save user handle in User.UserHandle
	if(obj_user != NULL){
		clazz = (*env)->FindClass(env, CLASS_USER_INFO);
		if (clazz == NULL) {
			LOGE("FindClass(%s) failed", CLASS_USER_INFO);
			return ret;
		}
		fid = (*env)->GetFieldID(env, clazz, "UserHandle", "I");
		if (fid == NULL) {
			LOGE("GetMethodID(Callback) failed");
			goto done;
		}

		(*env)->SetIntField(env, obj_user, fid, user_handle);
	}
	
//	fid = (*env)->GetFieldID(env, clazz, "username", "Ljava/lang/String;");
//	(*env)->SetObjectField(env, obj_user, fid, obj_username);
//	fid = (*env)->GetFieldID(env, clazz, "password", "Ljava/lang/String;");
//	(*env)->SetObjectField(env, obj_user, fid, obj_passwd);

done:
	if(clazz != NULL)
		(*env)->DeleteLocalRef(env, clazz);
	return ret;
}


JNIEXPORT jint JNICALL
NAME(ClUserLogin)(JNIEnv* env, jobject this, jobject obj_user, 
	jstring obj_username, jstring obj_passwd, jint callback_handle)
{
	RS ret = RS_ERROR;
	cl_handle_t user_handle;
	const char *str_username;
	const char *str_passwd;
	jclass clazz = NULL;
	jfieldID fid;

	str_username = (*env)->GetStringUTFChars(env, obj_username, NULL);
	str_passwd = (*env)->GetStringUTFChars(env, obj_passwd, NULL);

	ret = cl_user_login(&user_handle, str_username, str_passwd, nactivCallback, (void *)(jlong)callback_handle);
	(*env)->ReleaseStringUTFChars(env, obj_username, str_username);
	(*env)->ReleaseStringUTFChars(env, obj_passwd, str_passwd); 
	// save user handle in User.UserHandle
	if(obj_user != NULL){
		clazz = (*env)->FindClass(env, CLASS_USER_INFO);
		if (clazz == NULL) {
			LOGE("FindClass(%s) failed", CLASS_USER_INFO);
			return ret;
		}
		fid = (*env)->GetFieldID(env, clazz, "UserHandle", "I");
		if (fid == NULL) {
			LOGE("GetMethodID(Callback) failed");
			goto done;
		}

		(*env)->SetIntField(env, obj_user, fid, user_handle);
	}
	
//	fid = (*env)->GetFieldID(env, clazz, "username", "Ljava/lang/String;");
//	(*env)->SetObjectField(env, obj_user, fid, obj_username);
//	fid = (*env)->GetFieldID(env, clazz, "password", "Ljava/lang/String;");
//	(*env)->SetObjectField(env, obj_user, fid, obj_passwd);

done:
	if(clazz != NULL)
		(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUserQrLogout)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_user_qr_logout(user_handle);
}


JNIEXPORT jint JNICALL
NAME(ClUserLogout)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_user_logout(user_handle);
}

JNIEXPORT jint JNICALL
NAME(ClUserModifyNickname)(JNIEnv* env, jobject this, jint handle, jstring obj_name)
{
	const char *str_name;
	RS ret = RS_ERROR;
	
	str_name = (*env)->GetStringUTFChars(env, obj_name, NULL);
	ret = cl_user_modify_nickname(handle, str_name);
	
	(*env)->ReleaseStringUTFChars(env, obj_name, str_name);

	return ret;
}


JNIEXPORT jint JNICALL
NAME(ClUserSamePassword)(JNIEnv* env, jobject this, jint handle, jstring obj_passwd)
{
	const char *str_passwd;
	RS ret = RS_ERROR;
	
	str_passwd = (*env)->GetStringUTFChars(env, obj_passwd, NULL);
	ret = cl_user_same_passwd(handle, str_passwd);
	
	(*env)->ReleaseStringUTFChars(env, obj_passwd, str_passwd);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUserModifyPassword)(JNIEnv* env, jobject this, jint handle, jstring obj_passwd)
{
	const char *str_passwd;
	RS ret = RS_ERROR;
	
	str_passwd = (*env)->GetStringUTFChars(env, obj_passwd, NULL);
	ret = cl_user_modify_passwd(handle, str_passwd);
	
	(*env)->ReleaseStringUTFChars(env, obj_passwd, str_passwd);

	return ret;
}
/*************************RF网关*******************************/

JNIEXPORT jint JNICALL
NAME(ClRFGWUpQuery)(JNIEnv* env, jobject this, jint gw_handle)
{
	return cl_dev_up_query(gw_handle);
}

JNIEXPORT jint JNICALL
NAME(ClRFGWModeSet)(JNIEnv* env, jobject this, jint gw_handle, jbyte commpat, jbyte channel)
{
	return cl_dev_commpat(gw_handle, commpat, channel);
}

JNIEXPORT jint JNICALL
NAME(ClRFSlaveUpdateCli)(JNIEnv* env, jobject this, jint gw_handle, jint ext_type, jstring filename)
{
	char *str_filename;
	RS ret = RS_ERROR;
	str_filename = (char *)(*env)->GetStringUTFChars(env, filename, NULL);
	ret =  cl_rfdev_upgrade(gw_handle, ext_type, str_filename);

	(*env)->ReleaseStringUTFChars(env, filename, str_filename);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFChangeSlaveName)(JNIEnv* env, jobject this, jint handle, jstring jname)
{
	const u_int8_t *str_name;
	RS ret = RS_ERROR;

	if (jname == NULL) {
		return ret;
	}
	
	str_name = (*env)->GetStringUTFChars(env, jname, NULL);
	ret = cl_dev_name_set(handle, (u_int8_t *)str_name);
	
	(*env)->ReleaseStringUTFChars(env, jname, str_name);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFGroupQuery)(JNIEnv* env, jobject this, jint gw_handle, jbyte group_id)
{
	return cl_dev_group_query(gw_handle, group_id);
}

JNIEXPORT jint JNICALL
NAME(ClRFGroupSet)(JNIEnv* env, jobject this, jint gw_handle, jobject groupInfo)
{
	jfieldID fid;
	jclass obj_class;
	jobject obj;
	jstring str;
	const char *pstr = NULL;

	cl_dev_group_t group_p= {0};

	int len,i,ret,cnt = 0;
	jlong * sn;
	jlong *sn_p;
	
	if(groupInfo == NULL || gw_handle == 0)
		return -5;

	obj_class = (*env)->FindClass(env, CLASS_RF_DEV_GROUP_STATU);

	fid = (*env)->GetFieldID(env, obj_class, "group_id", "B");
	group_p.group_id = (*env)->GetByteField(env, groupInfo, fid);

	fid = (*env)->GetFieldID(env, obj_class, "group_type", "B");
	group_p.group_type = (*env)->GetByteField(env, groupInfo, fid);

	fid = (*env)->GetFieldID(env, obj_class, "dev_cnt", "B");
	group_p.dev_cnt = (*env)->GetByteField(env, groupInfo, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "query_dev", "Z");
	group_p.query_dev = (*env)->GetBooleanField(env, groupInfo, fid);
	
	fid = (*env)->GetFieldID(env, obj_class, "reserved", "B");
	group_p.reserved = (*env)->GetByteField(env, groupInfo, fid);

	fid = (*env)->GetFieldID(env, obj_class, "name", "Ljava/lang/String;");
	str = (*env)->GetObjectField(env, groupInfo, fid);
	
	if (NULL != str) {
		pstr = (*env)->GetStringUTFChars(env, str, NULL);
		strncpy(group_p.name, pstr, sizeof(group_p.name));
	}
	
	fid = (*env)->GetFieldID(env, obj_class, "dev_sn", "[J");
	obj = (*env)->GetObjectField(env, groupInfo, fid);
	if (NULL != obj) {
		sn = (*env)->GetLongArrayElements(env, obj, NULL);
		cnt = (*env)->GetArrayLength(env,obj);
		sn_p = sn;
		for (i = 0; i < cnt; i++,sn_p++) {
			(group_p.dev_sn)[i] = *sn_p;
		}

		(*env)->ReleaseLongArrayElements(env, obj, sn, 0);
	}

	ret = cl_dev_group_set(gw_handle, &group_p);

	if (NULL != str) {
		(*env)->ReleaseStringUTFChars(env, str, pstr);
	}

	SAFE_DEL_LOCAL_REF(obj_class);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFGroupDel)(JNIEnv* env, jobject this, jint gw_handle, jbyte group_id)
{
	return cl_dev_group_delete(gw_handle, group_id);
}

JNIEXPORT jint JNICALL
NAME(ClRFGWJion)(JNIEnv* env, jobject this, jint gw_handle, jint time_out)
{
	return cl_rfgw_join(gw_handle, time_out);
}

JNIEXPORT jint JNICALL
NAME(ClRFGWJionAction)(JNIEnv* env, jobject this, jint gw_handle, jlong sn)
{
	return cl_rfgw_join_action(gw_handle, sn, 1);
}



JNIEXPORT jint JNICALL
NAME(ClRFGWDevDelAll)(JNIEnv* env, jobject this, jint gw_handle)
{
	return cl_rfgw_dev_delete_all(gw_handle);
}

JNIEXPORT jint JNICALL
NAME(ClRFGWDevDel)(JNIEnv* env, jobject this, jint gw_handle, jintArray handles)
{
	int len, ret;
	cl_handle_t * dev_handles;
	
	len = (*env)->GetArrayLength(env, handles);
	dev_handles = (*env)->GetIntArrayElements(env, handles, NULL);
	ret = cl_rfgw_dev_delete(gw_handle, dev_handles, len);
	
	(*env)->ReleaseIntArrayElements(env, handles, dev_handles, JNI_ABORT);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFGWQueryList)(JNIEnv* env, jobject this, jint gw_handle)
{
	return cl_rfgw_query_devlist(gw_handle);
}

JNIEXPORT jint JNICALL
NAME(ClRFGWDevRGBBatch)(JNIEnv* env, jobject this, jint gw_handle, jintArray handles, jint rgb)
{
	int len, ret;
	cl_handle_t * dev_handles;
	
	len = (*env)->GetArrayLength(env, handles);
	dev_handles = (*env)->GetIntArrayElements(env, handles, NULL);
	ret = cl_rfdev_rgb_batch(gw_handle, dev_handles, len, rgb);
	
	(*env)->ReleaseIntArrayElements(env, handles, dev_handles, JNI_ABORT);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFGWDevWorkQuery)(JNIEnv* env, jobject this, jint gw_handle, jintArray handles)
{
	int len, ret;
	cl_handle_t * dev_handles;
	if (handles == NULL) {
		return RS_INVALID_PARAM;
	}
	
	len = (*env)->GetArrayLength(env, handles);
	dev_handles = (cl_handle_t *) ((*env)->GetIntArrayElements(env, handles, NULL));
	ret = cl_rfdev_work_query(gw_handle, dev_handles, len);
	
	(*env)->ReleaseIntArrayElements(env, handles, dev_handles, JNI_ABORT);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFLightRGB)(JNIEnv* env, jobject this, jint handle, jboolean r, jboolean g, jboolean b)
{
	return cl_rfdev_rgb(handle, r, g, b);
}

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

JNIEXPORT jint JNICALL
NAME(ClRFLightState)(JNIEnv* env, jobject this, jint dev_handle, jobject ledeState)
{
	int ret;
	cl_rf_lamp_stat_t *lamp_info = get_rf_light_state(env, ledeState);
	ret = cl_rf_lamp_ctrl_stat(dev_handle, lamp_info);
	
	SAFE_FREE(lamp_info);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFLampRmtCtrl)(JNIEnv* env, jobject this, jint gw_handle, jint remote_id, jbyte key_id, jobject ledeState)
{
	int ret;
	cl_rf_lamp_stat_t *lamp_info = get_rf_light_state(env, ledeState);
	ret = cl_rf_lamp_remote_ctrl_stat(gw_handle, remote_id, key_id, lamp_info);
	
	SAFE_FREE(lamp_info);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFGroupCtrl)(JNIEnv* env, jobject this, jobject param)
{
	jclass clazz;
	jobject object;
	jfieldID fid;
	int gw_handle, group_id, flag, len;
	void * data;
	int ret = RS_INVALID_PARAM;
	
	if (param == NULL) {
		return RS_INVALID_PARAM;
	}

	clazz = (*env)->FindClass(env, CLASS_RF_GROUP_CTRL_PARAM);
	fid = (*env)->GetFieldID(env, clazz, "gw_handle", "I");
	gw_handle = (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "group_id", "I");
	group_id = (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "flag", "I");
	flag = (*env)->GetIntField(env, param, fid);

	fid = (*env)->GetFieldID(env, clazz, "data", "L"CLASS_JAVA_OBJECT";");
	object = (*env)->GetObjectField(env, param, fid);

	if (flag == GDT_LAMP_COLOR_CTRL) {
		data = get_rf_light_state(env, object);
		ret = cl_rf_dev_group_ctrl(gw_handle, group_id, flag, data, sizeof(data));
	}
	SAFE_DEL_LOCAL_REF(clazz);
	SAFE_DEL_LOCAL_REF(object);
	SAFE_FREE(data);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRFDoorLockCtrl)(JNIEnv* env, jobject this, jint handle, jbyte groupid, jbyte type, jboolean action)
{
	return cl_rf_door_lock_ctrl(handle, groupid, type, action);
}

JNIEXPORT jint JNICALL
NAME(ClRFDevQueryHistory)(JNIEnv* env, jobject this, jint handle, jint last_time)
{
	return cl_rf_dev_query_history(handle, last_time);
}

JNIEXPORT jint JNICALL
NAME(ClRFDoorSetUnlockTimeout)(JNIEnv* env, jobject this, jint handle, jboolean enable, jshort timeout)
{
	return cl_rf_door_lock_set_unlock_timeout(handle, enable, timeout);
}

JNIEXPORT jint JNICALL
NAME(ClRFDoorSetWifiLock)(JNIEnv* env, jobject this, jint handle, jbyte type, jboolean enable, jbyte starthour, jbyte endhour)
{
	return cl_rf_door_lock_set_wifilock(handle, type, enable, starthour, endhour);
}

JNIEXPORT jint JNICALL
NAME(ClRFDoorSetController)(JNIEnv* env, jobject this, jint handle, jobject obj_controller)
{
	jclass class_controller = NULL;
	jfieldID fid = NULL;
	cl_door_lock_controller_set_t c_controller;

	class_controller = (*env)->GetObjectClass(env, obj_controller);
	jni_assign_simple_struct(env, obj_controller, class_controller,
		                        ASSIGN_TRIPLES(byte, &c_controller, id),
		                        ASSIGN_TRIPLES(byte, &c_controller, state),
		                        ASSIGN_QUAD(String,&c_controller, name, 16),
		                        JNI_VAR_ARG_END);
	
	return cl_rf_door_lock_set_controller_info(handle, &c_controller);
}


JNIEXPORT jint JNICALL
NAME(ClRFDevComCtrl)(JNIEnv* env, jobject this, jint handle, jbyte groupid, jbyte groupType, jbyte type, jbyte action)
{
	return cl_rf_dev_com_ctrl(handle, groupid, groupType, type, action);
}

JNIEXPORT jint JNICALL
NAME(ClRfMlRename)(JNIEnv* env, jobject this, jint handle,jbyte witch, jstring key_name)
{
	char *name = NULL;
	name = (char *)((*env)->GetStringUTFChars(env, key_name, NULL));

	return cl_dhx_ml_set_key(handle, witch, name);
}


JNIEXPORT jint JNICALL
NAME(ClYTRFDoorLockCtrl)(JNIEnv* env, jobject this, jint handle, jbyte groupid, jboolean onoff, jint passwd)
{
	return cl_yt_rf_door_lock_ctrl_lock(handle, groupid, onoff, passwd);
}

JNIEXPORT jint JNICALL
NAME(ClYTRFDoorLockModfiyPwd)(JNIEnv* env, jobject this, jint handle, jint oldpasswd, jint newpasswd)
{
	return cl_yt_rf_door_lock_modify_lock_passwd(handle, oldpasswd, newpasswd);
}

JNIEXPORT jint JNICALL
NAME(ClYTRFDoorLockCreatePwd)(JNIEnv* env, jobject this, jint handle, jint newpasswd)
{
	return cl_yt_rf_door_lock_create_lock_passwd(handle, newpasswd);
}

JNIEXPORT jint JNICALL
NAME(ClRFCtrlAutoGuard)(JNIEnv* env, jobject this, jint handle, jobject state)
{
	jclass class_guard = NULL;
	cl_rf_auto_guard_info_t guard_info;

	class_guard = (*env)->GetObjectClass(env, state);
	
	jni_assign_simple_struct(env, state, class_guard,
                          		ASSIGN_TRIPLES(boolean,&guard_info, enable),
		                        ASSIGN_TRIPLES(byte,&guard_info, start_hour),
		                        ASSIGN_TRIPLES(byte,&guard_info, end_hour),
		                        ASSIGN_TRIPLES(byte,&guard_info, type),
		                        JNI_VAR_ARG_END);

	//LOGD("ClRFCtrlAutoGuard enable = %b\n",&guard_info->enable);
	//LOGD("ClRFCtrlAutoGuard start_hour = %d\n",guard_info.start_hour);
	//LOGD("ClRFCtrlAutoGuard end_hour = %d\n",guard_info.end_hour);
	//LOGD("ClRFCtrlAutoGuard type = %d\n",guard_info.type);
	return cl_rf_ctrl_auto_guard(handle, &guard_info);
}
//暖气阀
JNIEXPORT jint JNICALL
NAME(ClHeatingValveCtrl)(JNIEnv* env, jobject this, jint handle, jint action, jbyte value)
{
	return cl_rf_heating_valve_simple_ctrl(handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClHeatingValveDateCtrl)(JNIEnv* env, jobject this, jint handle, jobject date)
{
	jclass class_date = NULL;
	cl_heating_valve_param_date_t c_date;

	class_date = (*env)->GetObjectClass(env, date);
	jni_assign_simple_struct(env, date, class_date,
		                        ASSIGN_TRIPLES(short, &c_date, year),
		                        ASSIGN_TRIPLES(byte, &c_date, month),
		                        ASSIGN_TRIPLES(byte, &c_date, day),
		                        ASSIGN_TRIPLES(byte, &c_date, hour),
		                        ASSIGN_TRIPLES(byte, &c_date, minute),
		                        JNI_VAR_ARG_END);
	return cl_rf_heating_valve_date_ctrl(handle, &c_date);
}

JNIEXPORT jint JNICALL
NAME(ClHeatingValveTempCtrl)(JNIEnv* env, jobject this, jint handle, jobject temp)
{
	jclass class_temp = NULL;
	cl_heating_valve_param_temp_t c_temp;

	class_temp = (*env)->GetObjectClass(env, temp);
	jni_assign_simple_struct(env, temp, class_temp,
		                        ASSIGN_TRIPLES(short, &c_temp, manual_temp),
		                        ASSIGN_TRIPLES(short, &c_temp, heat_temp),
		                        ASSIGN_TRIPLES(short, &c_temp, economy_temp),
		                        JNI_VAR_ARG_END);
	return cl_rf_heating_valve_temp_ctrl(handle, &c_temp);
}
//凯特
JNIEXPORT jint JNICALL
NAME(ClKaiteRfCtrl)(JNIEnv* env, jobject this, jint handle, jbyte action, jbyte value)
{
	return cl_rf_ktcz_simple_ctrl(handle, action, value);
}


/*************************************************************/

static void CopyDevAirCleanerInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;

	jclass obj_class,timer_class,smart_class;

	jobject object_array;
	jobject timer_object;
	jobject smart_object;
	jobject elec_object;
	int i;
	
	cl_ia_aircleaner_info_t* airp;
	cl_air_timer_t* t_timer;

	class = (*env)->FindClass(env, CLASS_IA_AIR_CLEANER);
	if( !class ){
		return;
	}

	airp = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !airp ){
		return;
	}

	jniCopyBooleanValue(env,class,"onoff",object,airp->onoff);
	jniCopyBooleanValue(env,class,"ultra",object,airp->ultra);
	jniCopyBooleanValue(env,class,"anion",object,airp->anion);
	jniCopyIntValue(env,class,"speed",object,airp->speed);
	jniCopyIntValue(env,class,"timer",object,airp->timer);
	jniCopyIntValue(env,class,"pm25",object,airp->pm25);
	jniCopyIntValue(env,class,"temp",object,airp->temp);
	jniCopyIntValue(env,class,"rh",object,airp->rh);
	jniCopyIntValue(env,class,"power",object,airp->power);

	if(info->ia_info.ia_type == IA_UDP_AIR_CLEAR) {

		jniCopyIntValue(env,class,"airclear_type",object,IA_UDP_AIR_CLEAR);
		jniCopyIntValue(env,class,"airclear_subtype",object,IA_EXT_TYPE_813_HK);

		jniCopyIntValue(env,class,"work_mode",object,airp->work_mode);
		jniCopyIntValue(env,class,"timer_type",object,airp->timer_type);
		jniCopyIntValue(env,class,"set_hour",object,airp->set_hour);
			
		
		if(info->ia_info.ia_sub_type == IA_EXT_TYPE_813_NB) {

			jniCopyIntValue(env,class,"airclear_type",object,IA_UDP_AIR_CLEAR);
			jniCopyIntValue(env,class,"airclear_subtype",object,ETYPE_IJ_813_NB);

			jniCopyBooleanValue(env,class,"terilize",object,airp->terilize);
			jniCopyIntValue(env,class,"terilize_minute",object,airp->terilize_minute);
			jniCopyIntValue(env,class,"rosebox_life",object,airp->rosebox_life);

				
			jniCopyBooleanValue(env,class,"on_effect",object,airp->periodic_timer.on_effect);
			jniCopyBooleanValue(env,class,"off_effect",object,airp->periodic_timer.off_effect);
			jniCopyIntValue(env,class,"timer_count",object,airp->periodic_timer.timer_count);
			jniCopyIntValue(env,class,"next_on_day",object,airp->periodic_timer.next_on_day);
			jniCopyIntValue(env,class,"next_on_hour",object,airp->periodic_timer.next_on_hour);
			jniCopyIntValue(env,class,"next_on_min",object,airp->periodic_timer.next_on_min);		
			jniCopyIntValue(env,class,"next_off_day",object,airp->periodic_timer.next_off_day);			
			jniCopyIntValue(env,class,"next_off_hour",object,airp->periodic_timer.next_off_hour);
			jniCopyIntValue(env,class,"next_off_min",object,airp->periodic_timer.next_off_min);
			

			if (airp->periodic_timer.timer_count> 0 && airp->periodic_timer.timers != NULL) {
				
				timer_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TIMER);
				object_array = (*env)->NewObjectArray(env, airp->periodic_timer.timer_count, timer_class, NULL);
				
				t_timer = airp->periodic_timer.timers;
				

				for (i = 0; i < airp->periodic_timer.timer_count; i++,t_timer++) {
					
						timer_object = (*env)->AllocObject(env, timer_class);
						if(!timer_object)
							break;
				
						jniCopyIntValue(env,timer_class,"id",timer_object,t_timer->id);
						jniCopyBooleanValue(env,timer_class,"enable",timer_object,t_timer->enable);
				
						jniCopyIntValue(env,timer_class,"week",timer_object,t_timer->week);
						jniCopyIntValue(env,timer_class,"hour",timer_object,t_timer->hour);
						jniCopyIntValue(env,timer_class,"minute",timer_object,t_timer->minute);
						jniCopyBooleanValue(env,timer_class,"onoff",timer_object,t_timer->onoff);
				
						(*env)->SetObjectArrayElement(env, object_array, i, timer_object);
						(*env)->DeleteLocalRef(env, timer_object);
					}
			
						fid = (*env)->GetFieldID(env, class, "timers", "[L" CLASS_AIR_PLUG_TIMER ";");
						(*env)->SetObjectField(env, object, fid, object_array);
		
						SAFE_DEL_LOCAL_REF(object_array);
						SAFE_DEL_LOCAL_REF(timer_class);

					}
			
			}
		}
	
		fid = (*env)->GetFieldID(env,class_dev_info  , "aircleaner", "L" CLASS_IA_AIR_CLEANER ";");
		(*env)->SetObjectField(env, obj_dev_info, fid, object);
		(*env)->DeleteLocalRef(env, object);
		(*env)->DeleteLocalRef(env, class);

	}


static void CopyDevAirConditionInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_aircondition_info_t* airconditionp;
	class = (*env)->FindClass(env, CLASS_IA_AIR_CONDITION);
	if( !class ){
		return;
	}

	airconditionp = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !airconditionp ){
		return;
	}
	jniCopyBooleanValue(env,class,"onoff",object,airconditionp->onoff);
	jniCopyIntValue(env,class,"mode",object,airconditionp->mode);
	jniCopyIntValue(env,class,"temp",object,airconditionp->temp);
	jniCopyIntValue(env,class,"power",object,airconditionp->power);
	jniCopyIntValue(env,class,"timer",object,airconditionp->timer);
	jniCopyIntValue(env,class,"cur_temp",object,airconditionp->cur_temp);

	fid = (*env)->GetFieldID(env, class_dev_info, "aircondition", "L" CLASS_IA_AIR_CONDITION ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyDevAirHeaterInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_airheater_info_t* airheaterp;

	class = (*env)->FindClass(env, CLASS_IA_AIR_HEATER);
	if( !class ){
		return;
	}

	airheaterp = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !airheaterp ){
		return;
	}

	jniCopyBooleanValue(env,class,"onoff",object,airheaterp->onoff);
	jniCopyIntValue(env,class,"gear",object,airheaterp->gear);
	jniCopyIntValue(env,class,"time",object,airheaterp->time);
	jniCopyIntValue(env,class,"mode",object,airheaterp->mode);
	jniCopyIntValue(env,class,"power",object,airheaterp->power);
	jniCopyIntValue(env,class,"temp",object,airheaterp->temp);

	fid = (*env)->GetFieldID(env, class_dev_info, "airHeater", "L" CLASS_IA_AIR_HEATER ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyGxLampInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	
	cl_ia_gx_led_info_t* cl_gx_lamp_info_p;

	if( !info ){
		return;
	}

	cl_gx_lamp_info_p = info->ia_info.u.ptr;
	if(!cl_gx_lamp_info_p){
		return;
	}

	class = (*env)->FindClass(env, CLASS_DEV_GX_LAMP);
	if(!class){
		return;
	}
	
	object = (*env)->AllocObject(env, class);
	if( !object ){
		(*env)->DeleteLocalRef(env, class);
		return;
	}

	jniCopyIntValue(env,class,"handle", object, info->handle);
	jniCopyIntValue(env,class,"led_status", object, cl_gx_lamp_info_p->led_status);
	jniCopyIntValue(env,class,"brightness", object, cl_gx_lamp_info_p->brightness);
	jniCopyIntValue(env,class,"warmness", object, cl_gx_lamp_info_p->warmness);
	jniCopyIntValue(env,class,"red", object, cl_gx_lamp_info_p->red);
	jniCopyIntValue(env,class,"green", object, cl_gx_lamp_info_p->green);
	jniCopyIntValue(env,class,"blue", object, cl_gx_lamp_info_p->blue);
	jniCopyIntValue(env,class,"night_status", object,cl_gx_lamp_info_p->night_status);

	fid = (*env)->GetFieldID(env, class_dev_info, "gxLampInfo", "L" CLASS_DEV_GX_LAMP ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyDevLCAirHeaterInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	
	cl_ah_work_stat_t* lc_air_heater_p;

	if( !info || !(info->ah_info)){
		return;
	}

	class = (*env)->FindClass(env, CLASS_DEV_AIR_HEATER_LC);
	if(!class){
		return;
	}

	lc_air_heater_p = &(info->ah_info->ah_work_stat);

	
	object = (*env)->AllocObject(env, class);
	if( !object ){
		(*env)->DeleteLocalRef(env, class);
		return;
	}

	jniCopyIntValue(env,class,"handle",object,info->handle);
	jniCopyBooleanValue(env,class,"is_on",object,lc_air_heater_p->is_on);
	jniCopyIntValue(env,class,"heat_mode",object,lc_air_heater_p->heat_mode);
	jniCopyBooleanValue(env,class,"is_shake",object,lc_air_heater_p->is_shake);
	jniCopyBooleanValue(env,class,"is_eco",object,lc_air_heater_p->is_eco);
	jniCopyBooleanValue(env,class,"is_heating",object,lc_air_heater_p->is_heating);
	jniCopyBooleanValue(env,class,"is_topple_protect",object,lc_air_heater_p->is_topple_protect);
	jniCopyBooleanValue(env,class,"is_furnace_high_protect",object,lc_air_heater_p->is_furnace_high_protect);
	jniCopyBooleanValue(env,class,"is_temp_high_protect",object,lc_air_heater_p->is_temp_high_protect);
	jniCopyBooleanValue(env,class,"is_furnace_error",object,lc_air_heater_p->is_furnace_error);
	jniCopyIntValue(env,class,"set_temp",object,lc_air_heater_p->set_temp);
	jniCopyIntValue(env,class,"room_temp",object,lc_air_heater_p->room_temp);
	jniCopyIntValue(env,class,"thermode_1_temp",object,lc_air_heater_p->thermode_1_temp);
	jniCopyIntValue(env,class,"thermode_2_temp",object,lc_air_heater_p->thermode_2_temp);
	jniCopyIntValue(env,class,"remain_minute",object,lc_air_heater_p->remain_minute);
	jniCopyIntValue(env,class,"set_hour",object,lc_air_heater_p->set_hour);
	jniCopyIntValue(env,class,"timer_type",object,lc_air_heater_p->timer_type);
	jniCopyIntValue(env,class,"epower",object,lc_air_heater_p->epower);

	fid = (*env)->GetFieldID(env, class_dev_info, "lcFurnaceInfo", "L" CLASS_DEV_AIR_HEATER_LC ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyDevAirHeaterYcytInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_airheater_ycyt_info_t* airheater_ycyt_p;

	class = (*env)->FindClass(env, CLASS_IA_AIR_HEATER_YCYT);
	if( !class ){
		return;
	}

	airheater_ycyt_p = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !airheater_ycyt_p ){
		return;
	}

	jniCopyIntValue(env,class,"set_temp",object,airheater_ycyt_p->set_temp);
	jniCopyIntValue(env,class,"cur_temp",object,airheater_ycyt_p->cur_temp);
	jniCopyIntValue(env,class,"mode",object,airheater_ycyt_p->mode);
	jniCopyIntValue(env,class,"gear",object,airheater_ycyt_p->gear);
	jniCopyIntValue(env,class,"time_off",object,airheater_ycyt_p->time);
	jniCopyBooleanValue(env,class,"onoff",object,airheater_ycyt_p->onoff);
	jniCopyIntValue(env,class,"time_on",object,airheater_ycyt_p->time_on);

	fid = (*env)->GetFieldID(env, class_dev_info, "airHeaterYcyt", "L" CLASS_IA_AIR_HEATER_YCYT ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
	
}


static void CopyDevEletricFanInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_electricfan_info_t* elefanp;

	class = (*env)->FindClass(env, CLASS_IA_ELETRINC_FAN);
	if( !class ){
		return;
	}

	elefanp = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !elefanp ){
		return;
	}
	jniCopyBooleanValue(env,class,"work",object,elefanp->work);
	jniCopyIntValue(env,class,"gear",object,elefanp->gear);
	jniCopyIntValue(env,class,"timer",object,elefanp->timer);
	jniCopyBooleanValue(env,class,"shake",object,elefanp->shake);
	jniCopyIntValue(env,class,"power",object,elefanp->power);

	fid = (*env)->GetFieldID(env, class_dev_info, "eletricFan", "L" CLASS_IA_ELETRINC_FAN ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyDevBathHeaterAupuInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_bath_heater_info_t* bath_aupu_p;

	class = (*env)->FindClass(env, CLASS_IA_BATH_HEATER_AUPU);
	if( !class ){
		return;
	}

	bath_aupu_p = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !bath_aupu_p ){
		return;
	}
	
	jniCopyBooleanValue(env,class,"onoff",object,bath_aupu_p->power_on_off);
	jniCopyBooleanValue(env,class,"anion",object,bath_aupu_p->anion_on_off);
	jniCopyBooleanValue(env,class,"light",object,bath_aupu_p->light_on_off);
	jniCopyBooleanValue(env,class,"ventilat",object,bath_aupu_p->breath_on_off);
	jniCopyBooleanValue(env,class,"dry",object,bath_aupu_p->dry_on_off);
	jniCopyIntValue(env,class,"wind",object,bath_aupu_p->tronic_on_off);
	jniCopyIntValue(env,class,"timer",object,bath_aupu_p->next_time);
	fid = (*env)->GetFieldID(env, class_dev_info, "bathHeaterAupu", "L" CLASS_IA_BATH_HEATER_AUPU ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyDevWaterHeaterInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_waterheater_info_t* waterp;

	class = (*env)->FindClass(env, CLASS_IA_WATER_HEATER);
	if( !class ){
		return;
	}

	waterp = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !waterp ){
		return;
	}

	jniCopyBooleanValue(env,class,"work",object,waterp->work);
	jniCopyIntValue(env,class,"temp_current",object,waterp->temp_current);
	jniCopyIntValue(env,class,"temp_set",object,waterp->temp_set);
	jniCopyIntValue(env,class,"timer",object,waterp->timer);
	jniCopyIntValue(env,class,"capactity",object,waterp->capactity);
	jniCopyIntValue(env,class,"power",object,waterp->power);

	fid = (*env)->GetFieldID(env, class_dev_info, "waterHeater", "L" CLASS_IA_WATER_HEATER ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static void CopyDevWaterHeaterA9Information(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_ia_waterheater_a9_info_t* waterp_a9;

	class = (*env)->FindClass(env, CLASS_IA_WATER_HEATER_A9);
	if( !class ){
		return;
	}

	waterp_a9 = info->ia_info.u.ptr;
	object = (*env)->AllocObject(env, class);

	if( !object || !waterp_a9 ){
		return;
	}

	jniCopyIntValue(env,class,"temp_set",object,waterp_a9->temp_set);
	jniCopyIntValue(env,class,"temp_current",object,waterp_a9->temp_current);
	jniCopyIntValue(env,class,"mode",object,waterp_a9->mode);
	jniCopyBooleanValue(env,class,"wind_on",object,waterp_a9->work & BIT(0));
	jniCopyBooleanValue(env,class,"water_on",object,waterp_a9->work & BIT(1));
	jniCopyBooleanValue(env,class,"fire_on",object,waterp_a9->work & BIT(2));
	jniCopyBooleanValue(env,class,"is_first",object,waterp_a9->work & BIT(3));
	jniCopyBooleanValue(env,class,"little_on",object,waterp_a9->work & BIT(4));
	jniCopyBooleanValue(env,class,"large_on",object,waterp_a9->work & BIT(5));
	jniCopyBooleanValue(env,class,"is_t1",object,waterp_a9->work & BIT(6));
	jniCopyBooleanValue(env,class,"is_t2",object,waterp_a9->work & BIT(7));
	
	jniCopyIntValue(env,class,"fire_level",object,waterp_a9->fire_level);
	jniCopyIntValue(env,class,"count",object,waterp_a9->count);
	jniCopyIntValue(env,class,"gas",object,waterp_a9->gas);

	fid = (*env)->GetFieldID(env, class_dev_info, "waterHeaterA9", "L" CLASS_IA_WATER_HEATER_A9 ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}


static void copyIntelligentRepeaterInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jobject object = NULL;
	jclass class = NULL;
	jfieldID fid;
	cl_if_info_t *if_info = NULL;


	if (NULL == info || info->sub_type != IJ_805) {
		return;
	}

	class = (*env)->FindClass(env, CLASS_IF_INFO);
	if( !class ){
		return;
	}

	if_info = &info->if_info;
	object = (*env)->AllocObject(env, class);

	if( !object){
		return;
	}

	jniCopyIntValue(env,class,"temp",object,if_info->temp);
	jniCopyIntValue(env,class,"rh",object,if_info->rh);
	jniCopyIntValue(env,class,"pm25",object,if_info->pm25);
	jniCopyIntValue(env,class,"voc",object,if_info->voc);


	fid = (*env)->GetFieldID(env, class_dev_info, "ifInfo", "L" CLASS_IF_INFO ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, class);
}

static int CopyAirTimeInfo(JNIEnv* env,jclass class_udp_info, jobject obj_udp_info, cl_com_udp_device_data *udp_info)
{
	jclass class_timer = NULL;
	jobject object_timer = NULL;
	
	jclass class_dev_timer = NULL;
	jobject object_dev_timer_array = NULL;
	jobject object_dev_timer = NULL;
	
	jclass class_period_timer = NULL;
	jobject object_period_timer_array = NULL;
	jobject object_period_timer = NULL;
	jfieldID fid;
	
	cl_air_timer_info_t *timer = &udp_info->timer_info;
	cl_air_timer_t* dev_timers; // 定时器列表
	cl_period_timer_t* period_timers; //周期性时间
	int ret = RS_OK;
	int i;

	if (timer == NULL) {
		LOGE("common udp info: timer is NULL\n");
		goto quit;
	}

	class_timer = 	(*env)->FindClass(env, CLASS_TIMER_INFO);
	if (class_timer == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	object_timer = (*env)->AllocObject(env, class_timer);
	if (object_timer == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyBooleanValue(env, class_timer, "on_effect", object_timer, timer->on_effect);
	jniCopyBooleanValue(env, class_timer, "off_effect", object_timer, timer->off_effect);
	jniCopyIntValue(env, class_timer, "timer_count", object_timer, timer->timer_count);
	jniCopyIntValue(env, class_timer, "next_on_day", object_timer, timer->next_on_day);
	jniCopyIntValue(env, class_timer, "next_on_hour", object_timer, timer->next_on_hour);
	jniCopyIntValue(env, class_timer, "next_on_min", object_timer, timer->next_on_min);
	jniCopyIntValue(env, class_timer, "next_off_day", object_timer, timer->next_off_day);
	jniCopyIntValue(env, class_timer, "next_off_hour", object_timer, timer->next_off_hour);
	jniCopyIntValue(env, class_timer, "next_off_min", object_timer, timer->next_off_min);
	jniCopyIntValue(env, class_timer, "on_minute", object_timer, timer->on_minute);
	jniCopyIntValue(env, class_timer, "off_minute", object_timer, timer->off_minute);

	if(timer->timer_count > 0) {
		if (udp_info->is_support_period_timer && timer->period_timers!= NULL) {
			class_period_timer = (*env)->FindClass(env, CLASS_UDP_PERIOD_TIMER);
			object_period_timer_array = (*env)->NewObjectArray(env, timer->timer_count, class_period_timer, NULL);
			period_timers = timer->period_timers;
			for (i = 0; i < timer->timer_count; i++, period_timers++) {
				object_period_timer = (*env)->AllocObject(env, class_period_timer);

				jniCopyIntValue(env,class_period_timer,"id",object_period_timer,period_timers->id);
				jniCopyBooleanValue(env,class_period_timer,"enable",object_period_timer,period_timers->enable);
				jniCopyIntValue(env,class_period_timer,"week",object_period_timer,period_timers->week);
				jniCopyIntValue(env,class_period_timer,"hour",object_period_timer,period_timers->hour);
				jniCopyIntValue(env,class_period_timer,"minute",object_period_timer,period_timers->minute);
				jniCopyBooleanValue(env,class_period_timer,"onoff",object_period_timer,period_timers->onoff);
				jniCopyIntValue(env,class_period_timer,"duration",object_period_timer,period_timers->duration);
				jniCopyIntValue(env,class_period_timer,"ext_data_type",object_period_timer,period_timers->ext_data_type);
				if(period_timers->ext_data_type == PT_EXT_DT_QPCP){
					jniCopyIntValue(env,class_period_timer,"scene_id",object_period_timer,period_timers->pt_ext_data_u.qp_time_info.id);
				} else if(period_timers->ext_data_type == PT_EXT_DT_QP_POT){
					jniCopyIntValue(env,class_period_timer,"cook_id",object_period_timer,period_timers->pt_ext_data_u.qp_pot_timer_info.cook_id);
					jniCopyIntValue(env,class_period_timer,"cook_time",object_period_timer,period_timers->pt_ext_data_u.qp_pot_timer_info.cook_time);
					jniCopyIntValue(env,class_period_timer,"hot_degress",object_period_timer,period_timers->pt_ext_data_u.qp_pot_timer_info.hot_degress);
					jniCopyIntValue(env,class_period_timer,"microswitch",object_period_timer,period_timers->pt_ext_data_u.qp_pot_timer_info.microswitch);
					jniCopyIntValue(env,class_period_timer,"warm_temp",object_period_timer,period_timers->pt_ext_data_u.qp_pot_timer_info.warm_temp);
					jniCopyIntValue(env,class_period_timer,"cooking_mode",object_period_timer,period_timers->pt_ext_data_u.qp_pot_timer_info.cooking_mode);
				} else if(period_timers->ext_data_type == PT_EXT_DT_QP_PBJ){
					jniCopyIntValue(env,class_period_timer,"scene_id",object_period_timer,period_timers->pt_ext_data_u.qp_pbj_timer_info.scene_id);
				} else if (period_timers->ext_data_type == PT_EXT_DT_101_OEM){
					jniCopyByteValue(env, class_period_timer, "min_temp", object_period_timer,period_timers->pt_ext_data_u.oem_101_timer_info.min_temp);
					jniCopyByteValue(env, class_period_timer, "max_temp", object_period_timer,period_timers->pt_ext_data_u.oem_101_timer_info.max_temp);
				} else if (period_timers->ext_data_type == PT_EXT_DT_HX_YSH){
					jniCopyShortValue(env, class_period_timer, "hx_scene_id", object_period_timer,period_timers->pt_ext_data_u.hx_ysh_timer_info.scene_id);
					jniCopyByteValue(env, class_period_timer, "temp", object_period_timer,period_timers->pt_ext_data_u.hx_ysh_timer_info.temp);
					jniCopyByteValue(env, class_period_timer, "work_time", object_period_timer,period_timers->pt_ext_data_u.hx_ysh_timer_info.work_time);
					jniCopyByteValue(env, class_period_timer, "power", object_period_timer,period_timers->pt_ext_data_u.hx_ysh_timer_info.power);
					jniCopyByteValue(env, class_period_timer, "keep_temp", object_period_timer,period_timers->pt_ext_data_u.hx_ysh_timer_info.keep_temp);
					jniCopyByteValue(env, class_period_timer, "keep_time", object_period_timer,period_timers->pt_ext_data_u.hx_ysh_timer_info.keep_time);
				}
				(*env)->SetObjectArrayElement(env, object_period_timer_array, i, object_period_timer);
				SAFE_DEL_LOCAL_REF(object_period_timer);
			}
			fid = (*env)->GetFieldID(env, class_timer, "period_timers", "[L" CLASS_UDP_PERIOD_TIMER ";");
			(*env)->SetObjectField(env, object_timer, fid, object_period_timer_array);			
			SAFE_DEL_LOCAL_REF(object_period_timer_array);
			SAFE_DEL_LOCAL_REF(class_period_timer);
			
		} else if (!udp_info->is_support_period_timer && timer->timers != NULL) {
			class_dev_timer = (*env)->FindClass(env, CLASS_UDP_DEV_TIMER);
			object_dev_timer_array = (*env)->NewObjectArray(env, timer->timer_count, class_dev_timer, NULL);
			dev_timers = timer->timers;
			for (i = 0; i < timer->timer_count; i++,dev_timers++) {
				object_dev_timer = (*env)->AllocObject(env, class_dev_timer);

				jniCopyIntValue(env,class_dev_timer,"id",object_dev_timer,dev_timers->id);
				jniCopyBooleanValue(env,class_dev_timer,"enable",object_dev_timer,dev_timers->enable);
				jniCopyIntValue(env,class_dev_timer,"week",object_dev_timer,dev_timers->week);
				jniCopyIntValue(env,class_dev_timer,"hour",object_dev_timer,dev_timers->hour);
				jniCopyIntValue(env,class_dev_timer,"minute",object_dev_timer,dev_timers->minute);
				jniCopyBooleanValue(env,class_dev_timer,"onoff",object_dev_timer,dev_timers->onoff);

				(*env)->SetObjectArrayElement(env, object_dev_timer_array, i, object_dev_timer);
				SAFE_DEL_LOCAL_REF(object_dev_timer);
			}
			fid = (*env)->GetFieldID(env, class_timer, "timers", "[L" CLASS_UDP_DEV_TIMER ";");
			(*env)->SetObjectField(env, object_timer, fid, object_dev_timer_array);
			
			SAFE_DEL_LOCAL_REF(object_dev_timer_array);
			SAFE_DEL_LOCAL_REF(class_dev_timer);
		}
		fid = (*env)->GetFieldID(env, class_udp_info, "timer_info", "L" CLASS_TIMER_INFO ";");
		(*env)->SetObjectField(env, obj_udp_info, fid, object_timer);
	}
	
quit:
	SAFE_DEL_LOCAL_REF(class_timer);
	SAFE_DEL_LOCAL_REF(object_timer);
	SAFE_DEL_LOCAL_REF(class_dev_timer);
	SAFE_DEL_LOCAL_REF(object_dev_timer_array);
	SAFE_DEL_LOCAL_REF(object_dev_timer);
	SAFE_DEL_LOCAL_REF(class_period_timer);
	SAFE_DEL_LOCAL_REF(object_period_timer_array);
	SAFE_DEL_LOCAL_REF(object_period_timer);
	return ret;
}

static int CopyPeakTime(JNIEnv* env,jclass class_elec_stat, char *attrname,
	jobject obj_elec_stat, cl_peak_time_t *peak_time) 
{
	jclass class_peak = NULL;
	jobject object_peak = NULL;
	jfieldID fid;
	int ret = RS_OK;

	if (peak_time == NULL) {
		LOGE("common udp info: peak time is NULL\n");
		ret = RS_ERROR;
		goto quit;
	}

	class_peak = (*env)->FindClass(env, CLASS_PEAK_TIME);
	if (!class_peak) {
		ret = RS_ERROR;
		goto quit;
	}

	object_peak = (*env)->AllocObject(env, class_peak);
	if (!object_peak) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_peak, "begin_minute", object_peak, peak_time->begin_minute);
	jniCopyIntValue(env, class_peak, "last_minute", object_peak, peak_time->last_minute);
	fid = (*env)->GetFieldID(env, class_elec_stat, attrname, "L" CLASS_PEAK_TIME ";");
	(*env)->SetObjectField(env, obj_elec_stat, fid, object_peak);
	
quit:
	SAFE_DEL_LOCAL_REF(class_peak);
	SAFE_DEL_LOCAL_REF(object_peak);
	return ret;
}


static int CopyElecState(JNIEnv* env,jclass class_udp_info, char *attrname, jobject obj_udp_info, cl_elec_stat_info_t *stat)
{
	jclass class_stat = NULL;
	jobject object_stat = NULL;
	jfieldID fid;
	int ret = RS_OK;

	if (stat == NULL) {
		LOGE("common udp info: elec stat is NULL\n");
		ret = RS_ERROR;
		goto quit;
	}

	class_stat = (*env)->FindClass(env, CLASS_ELEC_STAT);
	if (!class_stat) {
		ret = RS_ERROR;
		goto quit;
	}

	object_stat = (*env)->AllocObject(env, class_stat);
	if (!object_stat) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntArray(env, class_stat, "month_peak", object_stat, stat->month_peak, MONTH_PER_YEAR);
	jniCopyIntArray(env, class_stat, "month_valley", object_stat, stat->month_valley, MONTH_PER_YEAR);
	jniCopyIntArray(env, class_stat, "month_normal", object_stat, stat->month_normal, MONTH_PER_YEAR);
	if (CopyPeakTime(env, class_stat, "peak_time", object_stat, &stat->peak_time) != RS_OK) {
		ret = RS_ERROR;
		goto quit;
	}

	if (CopyPeakTime(env, class_stat, "valley_time", object_stat, &stat->valley_time) != RS_OK) {
		ret = RS_ERROR;
		goto quit;
	}

	if (CopyPeakTime(env, class_stat, "flat_time", object_stat, &stat->flat_time) != RS_OK) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_stat, "peak_price", object_stat, stat->peak_price);
	jniCopyIntValue(env, class_stat,"valley_price", object_stat, stat->valley_price);
	jniCopyIntValue(env, class_stat, "flat_price", object_stat, stat->flat_price);

	fid = (*env)->GetFieldID(env, class_udp_info, attrname, "L" CLASS_ELEC_STAT ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_stat);
quit:
	SAFE_DEL_LOCAL_REF(class_stat);
	SAFE_DEL_LOCAL_REF(object_stat);
	return ret;
}

static int CopyElecItem(JNIEnv* env,jclass class_udp_info, char *attrname,
	jobject obj_udp_info, cl_air_elec_item_info *elec_item)
{
	jclass class_item = NULL;
	jobject object_item = NULL;
	jfieldID fid;
	int ret = RS_OK;

	if (elec_item == NULL) {
		LOGE("common udp info: elec item is NULL\n");
		ret = RS_ERROR;
		goto quit;
	}

	class_item = (*env)->FindClass(env, CLASS_ELEC_ITEM);
	if (!class_item) {
		ret = RS_ERROR;
		goto quit;
	}

	object_item = (*env)->AllocObject(env, class_item);
	if (!object_item) {
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_item, "begin_time", object_item, elec_item->begin_time);
	jniCopyIntValue(env, class_item, "elec", object_item, elec_item->elec);

	fid = (*env)->GetFieldID(env, class_udp_info, attrname, "L" CLASS_ELEC_ITEM ";");
	(*env)->SetObjectField(env, obj_udp_info, fid, object_item);
quit:
	SAFE_DEL_LOCAL_REF(class_item);
	SAFE_DEL_LOCAL_REF(object_item);
	return ret;
}

static int CopyElecDaysStatInfo(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_elec_days_stat_info *elec_days)
{
	jclass class_elec_days = NULL;
	jobject obj_elec_days = NULL;
	jobjectArray obj_array = NULL;
	jfieldID fid;
	int i = 0;
	int ret = RS_OK;

	if (obj_parent == NULL || elec_days == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	class_elec_days = (*env)->FindClass(env, CLASS_E_PLUG_DAYS_INFO);
	if( class_elec_days == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	obj_elec_days = (*env)->AllocObject(env, class_elec_days);

	if( obj_elec_days == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyBooleanValue(env, class_elec_days, "is_info_valid", obj_elec_days, elec_days->is_info_valid);
	jniCopyIntValue(env, class_elec_days, "days_count", obj_elec_days, elec_days->days_count);
	jniCopyIntValue(env, class_elec_days, "nearest_data_time", obj_elec_days, elec_days->nearest_data_time);
	jniCopyShortArray( env, class_elec_days, "elec_data", obj_elec_days, elec_days->elec_data, elec_days->days_count);

	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_E_PLUG_DAYS_INFO";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_elec_days);
quit:
	SAFE_DEL_LOCAL_REF(class_elec_days);
	SAFE_DEL_LOCAL_REF(obj_elec_days);

	return ret;
}

static int CopyErrItem(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_dev_err_item_t *err_item)
{
	jclass class_err_item = NULL;
	jobject obj_err_item = NULL;
	jfieldID fid;
	int ret = RS_OK;

	if (obj_parent == NULL || err_item == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	class_err_item = (*env)->FindClass(env, CLASS_COMM_DEV_ERR_ITEM);
	if( class_err_item == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	obj_err_item = (*env)->AllocObject(env, class_err_item);

	if( obj_err_item == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_err_item, "err_id", obj_err_item, err_item->err_id);
	jniCopyIntValue(env, class_err_item, "err_time", obj_err_item, err_item->err_time);
	jniCopyIntValue(env, class_err_item, "err_obj_type", obj_err_item, err_item->err_obj_type);
	jniCopyIntValue(env, class_err_item, "err_type", obj_err_item, err_item->err_type);
	jniCopyIntValue(env, class_err_item, "err_data", obj_err_item, err_item->err_data);


	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_DEV_ERR_ITEM";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_err_item);
quit:
	SAFE_DEL_LOCAL_REF(class_err_item);
	SAFE_DEL_LOCAL_REF(obj_err_item);

	return ret;	
}

static int CopyErrInfo(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_dev_err_info_t *err_info)
{
	jclass class_err = NULL;
	jobject obj_err = NULL;
	jobjectArray obj_array = NULL;
	jobject obj_err_item = NULL;
	jclass class_err_item = NULL;
	jfieldID fid;
	int i = 0;
	int ret = RS_OK;

	if (obj_parent == NULL || err_info == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	class_err = (*env)->FindClass(env, CLASS_COMM_DEV_ERR_INFO);
	if( class_err == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	obj_err = (*env)->AllocObject(env, class_err);

	if( obj_err == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	//异常信息
	jniCopyIntValue(env, class_err, "err_count", obj_err, err_info->err_count);
	if (err_info->err_count > 0 && err_info->err_items != NULL) {
		class_err_item = (*env)->FindClass(env, CLASS_COMM_DEV_ERR_ITEM);
		obj_array = (*env)->NewObjectArray(env, err_info->err_count, class_err_item, NULL);
		for (i = 0; i < err_info->err_count; ++i) {
			obj_err_item = (*env)->AllocObject(env, class_err_item);
			jniCopyIntValue(env, class_err_item, "err_id", obj_err_item, err_info->err_items[i].err_id);
			jniCopyIntValue(env, class_err_item, "err_time", obj_err_item, err_info->err_items[i].err_time);
			jniCopyIntValue(env, class_err_item, "err_obj_type", obj_err_item, err_info->err_items[i].err_obj_type);
			jniCopyIntValue(env, class_err_item, "err_type", obj_err_item, err_info->err_items[i].err_type);
			jniCopyIntValue(env, class_err_item, "err_data", obj_err_item, err_info->err_items[i].err_data);

			(*env)->SetObjectArrayElement(env, obj_array, i, obj_err_item);
			SAFE_DEL_LOCAL_REF(obj_err_item);
		}
		fid = (*env)->GetFieldID(env, class_err, "err_items", "[L" CLASS_COMM_DEV_ERR_ITEM ";");
		(*env)->SetObjectField(env, obj_err, fid, obj_array);
		SAFE_DEL_LOCAL_REF(obj_array);
		SAFE_DEL_LOCAL_REF(class_err_item);
	}
	
	//推送的异常信息
	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_DEV_ERR_INFO";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_err);

	jniCopyIntValue(env, class_err, "push_err_count", obj_err, err_info->pushed_err_count);
	if (err_info->pushed_err_count > 0 && err_info->pushed_err != NULL) {
		class_err_item = (*env)->FindClass(env, CLASS_COMM_DEV_ERR_ITEM);
		obj_array = (*env)->NewObjectArray(env, err_info->pushed_err_count, class_err_item, NULL);
		for (i = 0; i < err_info->pushed_err_count; ++i) {
			obj_err_item = (*env)->AllocObject(env, class_err_item);
			jniCopyIntValue(env, class_err_item, "err_id", obj_err_item, err_info->pushed_err[i].err_id);
			jniCopyIntValue(env, class_err_item, "err_time", obj_err_item, err_info->pushed_err[i].err_time);
			jniCopyIntValue(env, class_err_item, "err_obj_type", obj_err_item, err_info->pushed_err[i].err_obj_type);
			jniCopyIntValue(env, class_err_item, "err_type", obj_err_item, err_info->pushed_err[i].err_type);
			jniCopyIntValue(env, class_err_item, "err_data", obj_err_item, err_info->pushed_err[i].err_data);

			(*env)->SetObjectArrayElement(env, obj_array, i, obj_err_item);
			SAFE_DEL_LOCAL_REF(obj_err_item);
		}
		fid = (*env)->GetFieldID(env, class_err, "push_err_items", "[L" CLASS_COMM_DEV_ERR_ITEM ";");
		(*env)->SetObjectField(env, obj_err, fid, obj_array);
		SAFE_DEL_LOCAL_REF(obj_array);
		SAFE_DEL_LOCAL_REF(class_err_item);
	}
	

	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_DEV_ERR_INFO";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_err);
quit:
	SAFE_DEL_LOCAL_REF(class_err);
	SAFE_DEL_LOCAL_REF(obj_err);
	SAFE_DEL_LOCAL_REF(obj_array);
	SAFE_DEL_LOCAL_REF(class_err_item);
	SAFE_DEL_LOCAL_REF(obj_err_item);
	return ret;	
}

//return HistoryItem 的index
void CopyHistoryItem(JNIEnv* env,jclass class_parent, jobject obj_parent, int dev_map_type, u_int64_t sn, cl_dev_history_item_t history_item){
	switch(dev_map_type) {
	case DEV_WUKONG:		
		CopyAirplugHistoryItem(env, class_parent, obj_parent, sn, &history_item);
		break;
	default:
		break;
	}
}


int CopyHistoryInfo(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_dev_info_t *info)
{
	jclass class_history = NULL;
	jobject obj_history = NULL;
	jobjectArray obj_array = NULL;
	jobject obj_history_item = NULL;
	jclass class_history_item = NULL;
	jfieldID fid;
	int i = 0;
	int ret = RS_OK;

	cl_com_udp_device_data *comm_info = NULL;

	if (obj_parent == NULL || info == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	comm_info = info->com_udp_info;
	if (comm_info == NULL) {
		LOGE("common udp data error: comm_info is null");
		goto quit;
	}

	class_history = (*env)->FindClass(env, CLASS_COMM_DEV_HISTORY_INFO);
	if( class_history == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	obj_history = (*env)->AllocObject(env, class_history);

	if( obj_history == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyIntValue(env, class_history, "index_current", obj_history, comm_info->dev_history.index_current);
	jniCopyIntValue(env, class_history, "max_count", obj_history, comm_info->dev_history.max_count);
	jniCopyIntValue(env, class_history, "index", obj_history, comm_info->dev_history.index);
	
	int dev_map_type = typeMap(comm_info->sub_type, comm_info->ext_type);
	LOGE("CopyHistoryInfo    n = %d\n", comm_info->dev_history.n);
	if (comm_info->dev_history.n > 0 && comm_info->dev_history.item!= NULL) {
		class_history_item = (*env)->FindClass(env, CLASS_COMM_DEV_HISTORY_ITEM);
		obj_array = (*env)->NewObjectArray(env, comm_info->dev_history.n, class_history_item, NULL);
		for (i = 0; i < comm_info->dev_history.n; ++i) {
			obj_history_item = (*env)->AllocObject(env, class_history_item);
			
			CopyHistoryItem(env, class_history_item, obj_history_item, dev_map_type, info->sn, comm_info->dev_history.item[i]);
			(*env)->SetObjectArrayElement(env, obj_array, i, obj_history_item);
			SAFE_DEL_LOCAL_REF(obj_history_item);
		}
		fid = (*env)->GetFieldID(env, class_history, "items", "[L" CLASS_COMM_DEV_HISTORY_ITEM ";");
		(*env)->SetObjectField(env, obj_history, fid, obj_array);
		SAFE_DEL_LOCAL_REF(obj_array);
		SAFE_DEL_LOCAL_REF(class_history_item);
	}

	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_DEV_HISTORY_INFO";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_history);
	
quit:
	SAFE_DEL_LOCAL_REF(class_history);
	SAFE_DEL_LOCAL_REF(obj_history);
	SAFE_DEL_LOCAL_REF(obj_array);
	SAFE_DEL_LOCAL_REF(class_history_item);
	SAFE_DEL_LOCAL_REF(obj_history_item);
	return ret;	
}

int CopyDHCPServerInfo(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_dev_info_t *info)
{
	jclass class_dhcp = NULL;
	jobject obj_dhcp = NULL;
	jfieldID fid;
	int i = 0;
	int ret = RS_OK;

	cl_com_udp_device_data *comm_info = NULL;

	if (obj_parent == NULL || info == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	comm_info = info->com_udp_info;
	if (comm_info == NULL) {
		LOGE("common udp data error: comm_info is null");
		goto quit;
	}

	class_dhcp = (*env)->FindClass(env, CLASS_COMM_DHCP_CONFIG);
	if( class_dhcp == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	obj_dhcp = (*env)->AllocObject(env, class_dhcp);

	if( obj_dhcp == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyString(env,class_dhcp,"getway_ip",obj_dhcp,comm_info->dhcp_config.getway_ip);
	jniCopyString(env,class_dhcp,"start_ip",obj_dhcp,comm_info->dhcp_config.start_ip);
	jniCopyString(env, class_dhcp, "end_ip", obj_dhcp, comm_info->dhcp_config.end_ip);
	jniCopyString(env, class_dhcp, "mask", obj_dhcp, comm_info->dhcp_config.mask);
	jniCopyIntValue(env, class_dhcp, "time", obj_dhcp, comm_info->dhcp_config.time);

	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_DHCP_CONFIG";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_dhcp);
	
quit:
	SAFE_DEL_LOCAL_REF(class_dhcp);
	SAFE_DEL_LOCAL_REF(obj_dhcp);
	return ret;	
}


int CopyWanConfigInfo(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_dev_info_t *info)
{
	jclass class_wanconfig = NULL;
	jclass class_wan_phy_config = NULL;
	jclass class_config_item = NULL;
	jobject obj_config_item = NULL;
	jobject obj_wanconfig = NULL;
	jobject obj_phy_config = NULL;
	jobjectArray array_phy_config = NULL;
	jobjectArray array_config_item = NULL;
	jfieldID fid;
	cl_wan_config_t *wan_info = NULL;
	cl_wan_config_item_t * item = NULL;
	cl_wan_phy_item_t *phy_config = NULL;
	int i = 0, k = 0;
	int ret = RS_OK;

	cl_com_udp_device_data *comm_info = NULL;

	if (obj_parent == NULL || info == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	comm_info = info->com_udp_info;
	if (comm_info == NULL) {
		LOGE("common udp data error: comm_info is null");
		goto quit;
	}

	wan_info = &comm_info->wan_config;
	class_wanconfig = (*env)->FindClass(env, CLASS_COMM_WAN_CONFIG);
	if (wan_info->wan_num == 0) {
		fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_WAN_CONFIG";");
		(*env)->SetObjectField(env, obj_parent, fid, NULL);
		goto quit;
	}

	obj_wanconfig = (*env)->AllocObject(env, class_wanconfig);
	class_wan_phy_config= (*env)->FindClass(env, CLASS_COMM_WAN_PHY_CONFIG);
	class_config_item = (*env)->FindClass(env, CLASS_COMM_WAN_CONFIG_ITEM);
	array_phy_config= (*env)->NewObjectArray(env,wan_info->wan_num, class_wan_phy_config, NULL);
	for (i = 0; i < wan_info->wan_num; ++i) {
		phy_config = &wan_info->config[i];
		obj_phy_config = (*env)->AllocObject(env, class_wan_phy_config);
		jni_copy_simple_class(env, class_wan_phy_config, obj_phy_config,
			                  TRIPLES(byte, phy_config, index),
			                  TRIPLES(byte, phy_config, wan_phy_type),
			                  TRIPLES(byte, phy_config, select_network_type),
			                  JNI_VAR_ARG_END);
		array_config_item = (*env)->NewObjectArray(env, 3, class_config_item, NULL);
		for (k = 0; k < 3; ++k) {
			item = &phy_config->config_item[k];
			obj_config_item = (*env)->AllocObject(env, class_config_item);
			jniCopyByteValue(env, class_config_item,"network_type", obj_config_item,item ->network_type);
			if(item->network_type == 0){
               continue;
			}
			switch(item ->network_type) {
				case 1: 
					jni_copy_simple_class(env, class_config_item, obj_config_item,
				        	             TRIPLES(String, &item->config.config_static, ip),
				            	         TRIPLES(String, &item->config.config_static, mask),
				                	     TRIPLES(String, &item->config.config_static, getway_ip),
				                    	 TRIPLES(String, &item->config.config_static, main_dns),
				   		                 TRIPLES(String, &item->config.config_static, sub_dns),
				        	             JNI_VAR_ARG_END);
					break;
				case 2:
					jni_copy_simple_class(env, class_config_item, obj_config_item,
						                TRIPLES(String, &item->config.config_dhcp, ip),
						                TRIPLES(String, &item->config.config_dhcp, mask),
						                TRIPLES(String, &item->config.config_dhcp, getway_ip),
				         	            TRIPLES(String, &item->config.config_dhcp, main_dns),
				      	                TRIPLES(String, &item->config.config_dhcp, sub_dns),
				     	                JNI_VAR_ARG_END);
					break;
				case 3:
					jni_copy_simple_class(env, class_config_item, obj_config_item,
				       	   	    	    TRIPLES(String, &item->config.config_pppoe, name),
				                	   	TRIPLES(String, &item->config.config_pppoe, pwd),
				                	   	TRIPLES(String, &item->config.config_pppoe, ip),
				                	   	TRIPLES(String, &item->config.config_pppoe, peer_ip),
				                    	TRIPLES(String, &item->config.config_pppoe, main_dns),
				                     	TRIPLES(String, &item->config.config_pppoe, sub_dns),
				                    		 JNI_VAR_ARG_END);
					break;
				default:
					break;
			}
			(*env)->SetObjectArrayElement(env, array_config_item, k, obj_config_item);
			SAFE_DEL_LOCAL_REF(obj_config_item);
		}

		fid = (*env)->GetFieldID(env, class_wan_phy_config, "config", "[L"CLASS_COMM_WAN_CONFIG_ITEM";");
		(*env)->SetObjectField(env, obj_phy_config, fid, array_config_item);
		SAFE_DEL_LOCAL_REF(array_config_item);

		(*env)->SetObjectArrayElement(env, array_phy_config, i, obj_phy_config);
		SAFE_DEL_LOCAL_REF(obj_phy_config);
	}

	fid = (*env)->GetFieldID(env, class_wanconfig, "config", "[L"CLASS_COMM_WAN_PHY_CONFIG";");
	(*env)->SetObjectField(env, obj_wanconfig, fid, array_phy_config);
	SAFE_DEL_LOCAL_REF(array_phy_config);
	
	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_WAN_CONFIG";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_wanconfig);
	
quit:
	SAFE_DEL_LOCAL_REF(class_wanconfig);
	SAFE_DEL_LOCAL_REF(obj_wanconfig);
	SAFE_DEL_LOCAL_REF(class_wan_phy_config);
	SAFE_DEL_LOCAL_REF(class_config_item);
	return ret;	
}

int CopyApConfigInfo(JNIEnv* env,jclass class_parent, char *attrname, jobject obj_parent, cl_dev_info_t *info)
{
	jclass class_ap = NULL;
	jobject obj_ap = NULL;
	jfieldID fid;
	int i = 0;
	int ret = RS_OK;

	cl_com_udp_device_data *comm_info = NULL;

	if (obj_parent == NULL || info == NULL) {
		ret = RS_ERROR;
		goto quit;
	}

	comm_info = info->com_udp_info;
	if (comm_info == NULL) {
		LOGE("common udp data error: comm_info is null");
		goto quit;
	}

	class_ap = (*env)->FindClass(env, CLASS_COMM_AP_CONFIG);
	if( class_ap == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	obj_ap = (*env)->AllocObject(env, class_ap);

	if( obj_ap == NULL){
		ret = RS_ERROR;
		goto quit;
	}

	jniCopyString(env,class_ap,"ssid",obj_ap,comm_info->ap_config.ssid);
	jniCopyString(env, class_ap, "pwd", obj_ap, comm_info->ap_config.pwd);
	jniCopyByteValue(env,class_ap,"is_enc",obj_ap,comm_info->ap_config.is_enc);
	jniCopyByteValue(env, class_ap, "pow", obj_ap, comm_info->ap_config.pow);
	jniCopyByteValue(env, class_ap, "enable", obj_ap, comm_info->ap_config.enable);
	jniCopyByteValue(env, class_ap, "channle_mode", obj_ap, comm_info->ap_config.channle_mode);
	jniCopyByteValue(env, class_ap, "channel", obj_ap, comm_info->ap_config.channel);

	fid = (*env)->GetFieldID(env, class_parent, attrname, "L" CLASS_COMM_AP_CONFIG";");
	(*env)->SetObjectField(env, obj_parent, fid, obj_ap);
	
quit:
	SAFE_DEL_LOCAL_REF(class_ap);
	SAFE_DEL_LOCAL_REF(obj_ap);
	return ret;	
}

void CopyAirplugHistoryItem(JNIEnv* env,jclass class_parent, jobject obj_parent, u_int64_t sn, cl_dev_history_item_t *history_item){
	jclass class_history;
	jobject jobj_history;
	jfieldID fid;

	class_history = (*env)->FindClass(env, CLASS_COMM_AIRPLUG_HISTORY_ITEM);
	jobj_history = (*env)->AllocObject(env, class_history);
	jniCopyLongValue(env, class_history, "sn", jobj_history, sn);
	jniCopyIntValue(env, class_history, "index", jobj_history, history_item->u.wukong.id);
	jniCopyIntValue(env, class_history, "timestamp", jobj_history, history_item->u.wukong.time);
	jniCopyByteValue(env, class_history, "type", jobj_history, history_item->u.wukong.type);
	jniCopyByteValue(env, class_history, "condition", jobj_history, history_item->u.wukong.condition);
	jniCopyByteValue(env, class_history, "action", jobj_history, history_item->u.wukong.action);
	jniCopyBooleanValue(env, class_history, "onoff", jobj_history, history_item->u.wukong.onoff);
	jniCopyByteValue(env, class_history, "mode", jobj_history, history_item->u.wukong.mode);
	jniCopyByteValue(env, class_history, "temp", jobj_history, history_item->u.wukong.temp);
	jniCopyByteValue(env, class_history, "wind", jobj_history, history_item->u.wukong.wind);
	jniCopyByteValue(env, class_history, "winddir", jobj_history, history_item->u.wukong.winddir);
	jniCopyByteValue(env, class_history, "key", jobj_history, history_item->u.wukong.key);
	jniCopyBooleanValue(env, class_history, "valid", jobj_history, history_item->u.wukong.valid);

	fid = (*env)->GetFieldID(env, class_parent, "u", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, obj_parent, fid, jobj_history);

	(*env)->DeleteLocalRef(env, class_history);
	(*env)->DeleteLocalRef(env, jobj_history);
}

static void copyCommUdpLumInfo(JNIEnv* env, jclass class_udp, jobject obj_udp, cl_lanusers_manage_info_t *lum_info)
{
	jclass class_manage = NULL;
	jobject obj_manage = NULL;
	jfieldID fid = NULL;

	class_manage = (*env)->FindClass(env, CLASS_LAN_USER_MANAGE);
	obj_manage = (*env)->AllocObject(env, class_manage);

	jniCopyBooleanValue(env, class_manage, "enable", obj_manage, lum_info->enable);
	JNI_COPY_SIMPLE_CLASS(env, class_manage, obj_manage, CLASS_LAN_USER_RECORD_ITEM, last_record,
		                     TRIPLES(byte, &lum_info->last_record, event_type),
		                     TRIPLES(int, &lum_info->last_record, user_id),
		                     TRIPLES(int, &lum_info->last_record, timestamp),
		                     JNI_VAR_ARG_END);

	if (lum_info->record_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_manage, obj_manage, CLASS_LAN_USER_RECORD_ITEM, record,
		                        lum_info->record_num, sizeof(lanusers_manage_user_record_item_t),
		                        ARRAY_TRIPLES(byte, lum_info->record, event_type),
		                        ARRAY_TRIPLES(int, lum_info->record, user_id),
		                        ARRAY_TRIPLES(int, lum_info->record, user_id),
		                        JNI_VAR_ARG_END);
	}
	fid = (*env)->GetFieldID(env, class_udp, "lum_info", "L" CLASS_LAN_USER_MANAGE";");
	(*env)->SetObjectField(env, obj_udp, fid, obj_manage);
}


static void copyCommUdpHourLinesInfo(JNIEnv* env, jclass class_udp, jobject obj_udp, cl_com_udp_device_data *udp_info)
{
	jclass clazz = NULL;
	jobject obj = NULL;
	jfieldID fid = NULL;

	clazz = (*env)->FindClass(env, CLASS_AIR_PLUG_HOUR_LINES);
	obj = (*env)->AllocObject(env, clazz);
	
	jni_copy_simple_class(env,clazz,obj,
		                    TRIPLES(boolean,&(udp_info->room_temp_line), is_valid), 
		                    TRIPLES(byte,&(udp_info->room_temp_line), num),
		                    QUADRUPLE(byte[],&((udp_info->room_temp_line)),pad, 2),
		                    QUADRUPLE(byte[],&((udp_info->room_temp_line)),data, 24*6),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp, "room_temp_line", "L"CLASS_AIR_PLUG_HOUR_LINES";");
	(*env)->SetObjectField(env, obj_udp, fid, obj);
	SAFE_DEL_LOCAL_REF(obj);

	obj = (*env)->AllocObject(env, clazz);
	jni_copy_simple_class(env,clazz,obj,
		                    TRIPLES(boolean,&(udp_info->humi_line), is_valid), 
		                    TRIPLES(byte,&(udp_info->humi_line), num),
		                    QUADRUPLE(byte[],&((udp_info->humi_line)),pad, 2),
		                    QUADRUPLE(byte[],&((udp_info->humi_line)),data, 24*6),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_udp, "humi_line", "L"CLASS_AIR_PLUG_HOUR_LINES";");
	(*env)->SetObjectField(env, obj_udp, fid, obj);
	SAFE_DEL_LOCAL_REF(obj);

	SAFE_DEL_LOCAL_REF(clazz);
}


static void CopyCommUdpInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jclass class_udp_info = NULL;
	jobject object_udp_info = NULL;
	jfieldID fid;
	cl_com_udp_device_data *comm_data = NULL;

	if( !obj_dev_info || !info) {
		LOGE("common udp data error: input data null\n");
		goto quit;
	}
	comm_data = info->com_udp_info;
	if (comm_data == NULL) {
		LOGE("common udp data error: comm_data is null");
		goto quit;
	}
	
//	LOGE("find info->sub_type = %d,info->ext_type = %d",info->sub_type,info->ext_type);
//	LOGE("find comm_data->sub_type = %d,comm_data->ext_type = %d",comm_data->sub_type,comm_data->ext_type);
	
	if (info->sub_type != comm_data->sub_type || info->ext_type != comm_data->ext_type) {
		LOGE("common udp data error: type mismatch\n");
		goto quit;
	}
	
	class_udp_info = (*env)->FindClass(env, CLASS_COMM_UDP_DEV);
	if( !class_udp_info){
		goto quit;
	}
	object_udp_info = (*env)->AllocObject(env, class_udp_info);
	if (!object_udp_info) {
		goto quit;
	}

	jniCopyIntValue(env, class_udp_info, "sub_type", object_udp_info, comm_data->sub_type);
	jniCopyIntValue(env, class_udp_info, "ext_type", object_udp_info, comm_data->ext_type);
	jniCopyIntValue(env, class_udp_info, "is_lan_connect", object_udp_info, comm_data->is_lan_connect);
	jniCopyIntValue(env, class_udp_info, "is_all_data_update", object_udp_info, comm_data->is_all_data_update);
	jniCopyBooleanValue(env, class_udp_info, "is_system_info_valid", object_udp_info, comm_data->is_system_info_valid);
	jniCopyBooleanValue(env, class_udp_info, "is_stat_info_valid", object_udp_info, comm_data->is_stat_info_valid);
	jniCopyBooleanValue(env, class_udp_info, "is_support_period_timer", object_udp_info, comm_data->is_support_period_timer);
	jniCopyBooleanValue(env, class_udp_info, "is_suppport_elec_stat", object_udp_info, comm_data->is_suppport_elec_stat);
	jniCopyBooleanValue(env, class_udp_info, "is_support_dev_err_info", object_udp_info, comm_data->is_support_dev_err_info);
	jniCopyBooleanValue(env, class_udp_info, "is_support_dev_restory_factory", object_udp_info, comm_data->is_support_dev_restory_factory);
	jniCopyBooleanValue(env, class_udp_info, "is_support_dev_set_wifi_param", object_udp_info, comm_data->is_support_dev_set_wifi_param);
	jniCopyBooleanValue(env, class_udp_info, "is_support_room_temp_ajust", object_udp_info, comm_data->is_support_room_temp_ajust);
	jniCopyBooleanValue(env, class_udp_info, "is_support_elec_ajust", object_udp_info, comm_data->is_support_elec_ajust);
	jniCopyBooleanValue(env, class_udp_info, "is_support_rc", object_udp_info, comm_data->is_support_rc);
	jniCopyShortValue(env,class_udp_info,"env_room_temp_low",object_udp_info,comm_data->env_room_temp_low);
	jniCopyShortValue(env,class_udp_info,"env_room_temp_high",object_udp_info,comm_data->env_room_temp_high);
	jniCopyShortValue(env,class_udp_info,"env_temp_ajust_value",object_udp_info,comm_data->env_temp_ajust_value);
	jniCopyShortValue(env,class_udp_info,"elec_ajust_value",object_udp_info,comm_data->elec_ajust_value);
	jniCopyIntValue(env, class_udp_info, "current_power", object_udp_info, comm_data->current_power);
	jniCopyIntValue(env, class_udp_info, "cur_milli_power", object_udp_info, comm_data->cur_milli_power);
	jniCopyBooleanValue(env, class_udp_info, "is_support_utc_temp_curve", object_udp_info, comm_data->is_support_utc_temp_curve);
	jniCopyBooleanValue(env, class_udp_info, "is_support_public_smart_on", object_udp_info, comm_data->is_support_public_smart_on);
	jniCopyBooleanValue(env, class_udp_info, "smart_on", object_udp_info, comm_data->smart_on);
	jniCopyBooleanValue(env, class_udp_info, "is_support_public_child_lock", object_udp_info, comm_data->is_support_public_child_lock);
	jniCopyByteValue(env, class_udp_info, "child_lock_value", object_udp_info, comm_data->child_lock_value);
	jniCopyBooleanValue(env, class_udp_info, "is_support_public_temp_alarm", object_udp_info, comm_data->is_support_public_temp_alarm);
	jniCopyBooleanValue(env, class_udp_info, "temp_alarm_onoff", object_udp_info, comm_data->temp_alarm_onoff);
	jniCopyByteValue(env, class_udp_info, "temp_alarm_min", object_udp_info, comm_data->temp_alarm_min);
	jniCopyByteValue(env, class_udp_info, "temp_alarm_max", object_udp_info, comm_data->temp_alarm_max);
	jniCopyBooleanValue(env, class_udp_info, "is_support_public_shortcuts_onoff", object_udp_info, comm_data->is_support_public_shortcuts_onoff);
	jniCopyBooleanValue(env, class_udp_info, "has_recv_flag_pkt", object_udp_info, comm_data->has_recv_flag_pkt);
	jniCopyBooleanValue(env, class_udp_info, "is_support_la", object_udp_info, comm_data->is_support_la);
	jniCopyByteValue(env, class_udp_info, "room_temp", object_udp_info, comm_data->room_temp);
	jniCopyByteValue(env, class_udp_info, "temp_humidity", object_udp_info, comm_data->temp_humidity);
	jniCopyBooleanValue(env, class_udp_info, "support_lanusers_manage", object_udp_info, comm_data->support_lanusers_manage);
	jniCopyBooleanValue(env, class_udp_info, "is_support_dev_wifi_state", object_udp_info, comm_data->is_support_dev_wifi_state);
	jniCopyByteValue(env, class_udp_info, "dev_wifi_state", object_udp_info, comm_data->dev_wifi_state);
	jniCopyBooleanValue(env, class_udp_info, "is_support_dev_history", object_udp_info, comm_data->is_support_dev_history);

	jniCopyBooleanValue(env, class_udp_info, "is_support_boot_temp", object_udp_info, comm_data->is_support_boot_temp);
	jniCopyBooleanValue(env, class_udp_info, "boot_temp_enable", object_udp_info, comm_data->boot_temp_enable);
	jniCopyByteValue(env, class_udp_info, "boot_temp", object_udp_info, comm_data->boot_temp);

	jniCopyBooleanValue(env, class_udp_info, "is_support_wan_config", object_udp_info, comm_data->is_support_wan_config);
	jniCopyBooleanValue(env, class_udp_info, "is_support_dhcp_server", object_udp_info, comm_data->is_support_dhcp_server);
	jniCopyBooleanValue(env, class_udp_info, "is_support_ap_config", object_udp_info, comm_data->is_support_ap_config);
	jniCopyBooleanValue(env, class_udp_info, "is_support_repeat", object_udp_info, comm_data->is_support_repeat);
	jniCopyBooleanValue(env, class_udp_info, "repeat_onoff", object_udp_info, comm_data->repeat_onoff);
	
	jni_copy_simple_class(env, class_udp_info, object_udp_info,
		                     TRIPLES(boolean, comm_data, has_utc_temp_curve_data),
		                     TRIPLES(int, comm_data, temp_curve_len),
		                     TRIPLES(boolean, comm_data, is_support_public_utc_temp_ac_ctrl),
		                     TRIPLES(boolean, comm_data, has_utc_temp_ctrl_data),
		                     TRIPLES(boolean, comm_data, is_support_repeat),
		                     TRIPLES(boolean, comm_data, repeat_onoff),
		                     JNI_VAR_ARG_END);
	if (comm_data->support_lanusers_manage) {
		copyCommUdpLumInfo(env, class_udp_info, object_udp_info, &comm_data->lum_info);
	}
	
	/*if (comm_data->has_utc_temp_curve_data && comm_data->temp_curve_len > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_udp_info, object_udp_info, CLASS_AIR_PLUG_TEMP_CURVE, temp_curve,
			                    comm_data->temp_curve_len, sizeof(cl_temp_curve_t),
			                    ARRAY_TRIPLES(byte, comm_data->temp_curve, id),
			                    ARRAY_TRIPLES(boolean, comm_data->temp_curve, enable),
			                    ARRAY_TRIPLES(byte, comm_data->temp_curve, week),
			                    ARRAY_TRIPLES(byte, comm_data->temp_curve, begin_hour),
			                    ARRAY_TRIPLES(byte, comm_data->temp_curve, end_hour),
			                    ARRAY_TRIPLES(byte, comm_data->temp_curve, time_period),
			                    ARRAY_TRIPLES(byte, comm_data->temp_curve, count),
			                    JNI_VAR_ARG_END);
	}*/
	if (comm_data->is_support_utc_temp_curve && NULL != comm_data->temp_curve && 
		comm_data->temp_curve_len >= sizeof(cl_temp_curve_t)) {
		
		jclass smart_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CURVE);
		jobject object_array;
		jobject temp_object;
		
		int total = comm_data->temp_curve_len;
		int offset = 0;
		int cout = 0;
		int len = 0;
		int i = 0;
		jfieldID fid;
		cl_temp_curve_t * temp_curve_p;
		tmp_curve_t*curve_p;
		
		while(total > sizeof(cl_temp_curve_t)){
			temp_curve_p = (cl_temp_curve_t*)(((char*)comm_data->temp_curve) + offset);
			curve_p = (tmp_curve_t*) (temp_curve_p + 1);
			len = sizeof(cl_temp_curve_t) + temp_curve_p->count * sizeof(tmp_curve_t);
			// calc count;
			cout++;
			offset += len;
			total-=len;
		}

		total = comm_data->temp_curve_len;
		offset = 0;
		object_array = (*env)->NewObjectArray(env, cout, smart_class, NULL);

		i = 0;
		while(total > sizeof(cl_temp_curve_t)){
			temp_curve_p = (cl_temp_curve_t*)(((char*)comm_data->temp_curve) + offset);
			curve_p = (tmp_curve_t*) (temp_curve_p + 1);
			len = sizeof(cl_temp_curve_t) + temp_curve_p->count * sizeof(tmp_curve_t);
			temp_object = (*env)->AllocObject(env, smart_class);
			jni_copy_simple_class(env, smart_class, temp_object,
									TRIPLES(byte, temp_curve_p, id),
									TRIPLES(boolean, temp_curve_p, enable),
									TRIPLES(byte, temp_curve_p, week),
									TRIPLES(byte, temp_curve_p, begin_hour),
									TRIPLES(byte, temp_curve_p, end_hour),
									TRIPLES(byte, temp_curve_p, time_period),
									TRIPLES(byte, temp_curve_p, count),
									JNI_VAR_ARG_END
									);
			
			JNI_COPY_ARRAY_CLASS(env, smart_class, temp_object, CLASS_AIR_PLUG_TEMP_CURVE_LINE, 
					                    curves , temp_curve_p->count, sizeof(tmp_curve_t),
				 						ARRAY_TRIPLES(byte, curve_p, flag),
					                    ARRAY_TRIPLES(byte, curve_p, tmp),
					                    JNI_VAR_ARG_END);

			(*env)->SetObjectArrayElement(env, object_array, i, temp_object);
			SAFE_DEL_LOCAL_REF(temp_object);
			
			offset += len;
			total-=len;
			i++;
		}
		

		fid = (*env)->GetFieldID(env, class_udp_info, "temp_curve", "[L" CLASS_AIR_PLUG_TEMP_CURVE ";");
		(*env)->SetObjectField(env, object_udp_info, fid, object_array);

		SAFE_DEL_LOCAL_REF(smart_class);
		SAFE_DEL_LOCAL_REF(object_array);
	}
	
	if (comm_data->has_utc_temp_ctrl_data) {
		JNI_COPY_SIMPLE_CLASS(env, class_udp_info, object_udp_info, CLASS_AIR_PLUG_TEMP_CTRL,tac,
				                  TRIPLES(boolean, &comm_data->tac, enable),
				                  TRIPLES(byte, &comm_data->tac, mode),
				                  TRIPLES(byte, &comm_data->tac, temp_min),
				                  TRIPLES(byte, &comm_data->tac, temp_max),
				                  TRIPLES(byte, &comm_data->tac, week),
				                  TRIPLES(byte, &comm_data->tac, begin_hour),
				                  TRIPLES(byte, &comm_data->tac, end_hour),
				                  JNI_VAR_ARG_END);
	} 

	JNI_COPY_SIMPLE_CLASS(env, class_udp_info, object_udp_info, CLASS_COMM_SHORTCUT_POWER, shortcuts_onoff,
		                     TRIPLES(boolean, &comm_data->shortcuts_onoff, enable),
		                     TRIPLES(boolean, &comm_data->shortcuts_onoff, onoff),
		                     TRIPLES(int, &comm_data->shortcuts_onoff, time),
		                     TRIPLES(int, &comm_data->shortcuts_onoff, remain_time),
		                     JNI_VAR_ARG_END);

	if (CopyAirTimeInfo(env, class_udp_info, object_udp_info, comm_data) != RS_OK) {
		LOGE("common udp data error: copy timer info failed\n");
		goto quit;
	}

	copyCommTimerInfo(env, class_udp_info, object_udp_info, &comm_data->comm_timer_head, &comm_data->timer_summary,
		                comm_data->sub_type, comm_data->ext_type);
	copyCommUdpHourLinesInfo(env, class_udp_info, object_udp_info, comm_data);

	if (comm_data->is_suppport_elec_stat && CopyElecState(env, class_udp_info, "elec_stat_info", object_udp_info, &comm_data->elec_stat_info) != RS_OK) {
		LOGE("common udp data error: copy elec stat info failed\n");
		goto quit;
	}

	if (comm_data->is_suppport_elec_stat && CopyElecItem(env, class_udp_info, "total_elec", object_udp_info, &comm_data->total_elec) != RS_OK) {
		LOGE("common udp data error: copy elec item totle failed\n");
		goto quit;
	}

	if (comm_data->is_suppport_elec_stat && CopyElecItem(env, class_udp_info, "period_elec", object_udp_info, &comm_data->period_elec) != RS_OK) {
		LOGE("common udp data error: copy elec item period failed\n");
		goto quit;
	}

	if (comm_data->is_suppport_elec_stat && CopyElecItem(env, class_udp_info, "last_on_elec", object_udp_info, &comm_data->last_on_elec) != RS_OK) {
		LOGE("common udp data error: copy elec item last on failed\n");
		goto quit;
	}

	if (comm_data->is_suppport_elec_stat && CopyElecDaysStatInfo(env, class_udp_info, "elec_days_info", object_udp_info, &comm_data->elec_days_info) != RS_OK) {
		LOGE("common udp data error: copy elec days info failed\n");
		goto quit;
	}

	if (comm_data->is_support_dev_err_info && CopyErrInfo(env, class_udp_info, "dev_err_info", object_udp_info, &comm_data->dev_err_info) != RS_OK) {
		LOGE("common udp data error: copy dev err info failed\n");
		goto quit;
	}

	if (comm_data->is_support_dev_history && CopyHistoryInfo(env, class_udp_info, "dev_history", object_udp_info, info) != RS_OK) {
		LOGE("common udp data error: copy dev history failed\n");
		goto quit;
	}
	
	if (comm_data->is_support_dhcp_server && CopyDHCPServerInfo(env, class_udp_info, "dhcp_Config", object_udp_info, info) != RS_OK) {
		LOGE("common udp data error: copy dev dhcp config failed\n");
		goto quit;
	}

	if (comm_data->is_support_wan_config && CopyWanConfigInfo(env, class_udp_info, "wan_config", object_udp_info, info) != RS_OK) {
		LOGE("common udp data error: copy dev Wan config failed\n");
		goto quit;
	}

	if (comm_data->is_support_ap_config && CopyApConfigInfo(env, class_udp_info, "ap_config", object_udp_info, info) != RS_OK) {
		LOGE("common udp data error: copy dev Ap config failed\n");
		goto quit;
	}
	
	copyUdpDev(env, class_udp_info, object_udp_info, comm_data);

	fid = (*env)->GetFieldID(env, class_dev_info, "com_udp_info", "L" CLASS_COMM_UDP_DEV ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object_udp_info);
quit:
	SAFE_DEL_LOCAL_REF(class_udp_info);
	SAFE_DEL_LOCAL_REF(object_udp_info);
}

static void CopyElecInfo(JNIEnv* env,jclass class_eplug, char *attrname, jobject obj_eplug, cl_common_elec_info *elec_info)
{
	jclass class_elec_info = NULL;
	jobject obj_elec_info = NULL;
	jfieldID fid;

	if (obj_eplug == NULL || elec_info == NULL) {
		goto quit;
	}

	class_elec_info = (*env)->FindClass(env, CLASS_E_PLUG_ELEC_INFO);
	if( class_elec_info == NULL){
		goto quit;
	}

	obj_elec_info = (*env)->AllocObject(env, class_elec_info);

	if( obj_elec_info == NULL){
		goto quit;
	}

	jniCopyBooleanValue(env, class_elec_info, "is_support_elec_info", obj_elec_info, elec_info->is_support_elec_info);
	if (!elec_info->is_support_elec_info) {
		goto quit;
	}
	jniCopyIntValue(env, class_elec_info, "current_power", obj_elec_info, elec_info->current_power);
	jniCopyIntValue(env, class_elec_info, "current_mil_power", obj_elec_info, elec_info->current_mil_power);

	if (CopyElecState(env, class_elec_info, "elec_stat_info", obj_elec_info, &elec_info->elec_stat_info) != RS_OK) {
		goto quit;
	}

	if (CopyElecItem(env, class_elec_info, "total_elec", obj_elec_info, &elec_info->total_elec) != RS_OK) {
		goto quit;
	}

	if (CopyElecItem(env, class_elec_info, "period_elec", obj_elec_info, &elec_info->period_elec) != RS_OK) {
		goto quit;
	}

	if (CopyElecItem(env, class_elec_info, "last_on_elec", obj_elec_info, &elec_info->last_on_elec) != RS_OK) {
		goto quit;
	}

	if (CopyElecDaysStatInfo(env, class_elec_info, "elec_days_info", obj_elec_info, &elec_info->elec_days_info) != RS_OK) {
		goto quit;
	}

	fid = (*env)->GetFieldID(env, class_eplug, attrname, "L" CLASS_E_PLUG_ELEC_INFO";");
	(*env)->SetObjectField(env, obj_eplug, fid, obj_elec_info);
quit:
	SAFE_DEL_LOCAL_REF(class_elec_info);
	SAFE_DEL_LOCAL_REF(obj_elec_info);
}

static void CopyEPlugInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	
	jobject object = NULL;
	jclass obj_class,timer_class, period_timer_class;
	jfieldID fid;
	cl_ia_eb_info_t* ebplug;
	cl_air_timer_info_t* eb_timer;
	cl_air_timer_t* eb_timer_items;
	cl_period_timer_t *eb_period_timer_items;
	jobject object_array;
	jobject timer_object;
	jobject timer_item_object ;
	int i;

	//check param
	if( !obj_dev_info || !info || info->eb_info== NULL)
		return;

	obj_class = (*env)->FindClass(env, CLASS_E_PLUG);
	if( !obj_class ){
		return;
	}

	ebplug = info->eb_info;
	object = (*env)->AllocObject(env, obj_class);

	if( !object || !ebplug ){
		return;
	}

	jniCopyBooleanValue(env,obj_class,"on_off_valid",object,ebplug->on_off_valid);
	jniCopyBooleanValue(env,obj_class,"on_off",object,ebplug->on_off);
	jniCopyBooleanValue(env,obj_class,"on_effect",object,ebplug->timer.on_effect);
	jniCopyBooleanValue(env,obj_class,"off_effect",object,ebplug->timer.off_effect);
	jniCopyIntValue(env,obj_class,"timer_count",object,ebplug->timer.timer_count);
	jniCopyIntValue(env,obj_class,"next_on_day",object,ebplug->timer.next_on_day);
	jniCopyIntValue(env,obj_class,"next_on_hour",object,ebplug->timer.next_on_hour);
	jniCopyIntValue(env,obj_class,"next_on_min",object,ebplug->timer.next_on_min);
	jniCopyIntValue(env,obj_class,"next_off_day",object,ebplug->timer.next_off_day);
	jniCopyIntValue(env,obj_class,"next_off_hour",object,ebplug->timer.next_off_hour);
	jniCopyIntValue(env,obj_class,"next_off_min",object,ebplug->timer.next_off_min);
	jniCopyBooleanValue(env, obj_class, "is_support_period_timer", object, ebplug->is_support_period_timer);

	jniCopyBooleanValue(env, obj_class, "is_support_ctrl_led", object, ebplug->is_support_ctrl_led);
	jniCopyByteValue(env, obj_class, "led_onoff", object, ebplug->led_onoff);
	if (info->com_udp_info != NULL) {
		jniCopyByteValue(env, obj_class, "led_color_mode", object, info->com_udp_info->hardware_led_ver);
	}

	if (ebplug->timer.timer_count> 0){
		if (!ebplug->is_support_period_timer && ebplug->timer.timers!= NULL) {
			timer_class = (*env)->FindClass(env, CLASS_E_PLUG_TIMER);
			object_array = (*env)->NewObjectArray(env, ebplug->timer.timer_count, timer_class, NULL);

			eb_timer_items = ebplug->timer.timers;
			for (i = 0; i < ebplug->timer.timer_count; i++,eb_timer_items++) {
				timer_object = (*env)->AllocObject(env, timer_class);
				if(!timer_object)
					break;

				jniCopyIntValue(env,timer_class,"id",timer_object,eb_timer_items->id);
				jniCopyBooleanValue(env,timer_class,"enable",timer_object,eb_timer_items->enable);
				jniCopyIntValue(env,timer_class,"week",timer_object,eb_timer_items->week);
				jniCopyIntValue(env,timer_class,"hour",timer_object,eb_timer_items->hour);
				jniCopyIntValue(env,timer_class,"minute",timer_object,eb_timer_items->minute);
				jniCopyBooleanValue(env,timer_class,"onoff",timer_object,eb_timer_items->onoff);


				(*env)->SetObjectArrayElement(env, object_array, i, timer_object);
				(*env)->DeleteLocalRef(env, timer_object);
			}
			
			fid = (*env)->GetFieldID(env, obj_class, "timers", "[L" CLASS_E_PLUG_TIMER ";");
			(*env)->SetObjectField(env, object, fid, object_array);
			
			SAFE_DEL_LOCAL_REF(object_array);
			SAFE_DEL_LOCAL_REF(timer_class);
		} 
		else if (ebplug->is_support_period_timer && ebplug->timer.period_timers!= NULL) {
			period_timer_class = (*env)->FindClass(env, CLASS_E_PLUG_PERIOD_TIMER);
			object_array = (*env)->NewObjectArray(env, ebplug->timer.timer_count, period_timer_class, NULL);
			eb_period_timer_items = ebplug->timer.period_timers;
			for (i = 0; i < ebplug->timer.timer_count; i++,eb_period_timer_items++) {
				timer_object = (*env)->AllocObject(env, period_timer_class);
				if(!timer_object)
					break;

				jniCopyIntValue(env,period_timer_class, "id", timer_object, eb_period_timer_items->id);
				jniCopyIntValue(env,period_timer_class, "hour", timer_object, eb_period_timer_items->hour);
				jniCopyIntValue(env,period_timer_class, "minute", timer_object, eb_period_timer_items->minute);
				jniCopyIntValue(env,period_timer_class, "week", timer_object, eb_period_timer_items->week);
				jniCopyBooleanValue(env,period_timer_class, "enable", timer_object, eb_period_timer_items->enable);
				jniCopyBooleanValue(env,period_timer_class, "onoff", timer_object, eb_period_timer_items->onoff);
				jniCopyIntValue(env,period_timer_class, "duration", timer_object, eb_period_timer_items->duration);
				
				(*env)->SetObjectArrayElement(env, object_array, i, timer_object);
				(*env)->DeleteLocalRef(env, timer_object);
			}

			fid = (*env)->GetFieldID(env, obj_class, "period_timers", "[L" CLASS_E_PLUG_PERIOD_TIMER ";");
			(*env)->SetObjectField(env, object, fid, object_array);
			
			SAFE_DEL_LOCAL_REF(object_array);
			SAFE_DEL_LOCAL_REF(period_timer_class);
		}
		
	}

	CopyElecInfo(env, obj_class, "elec_info", object, &ebplug->elec_info);

	fid = (*env)->GetFieldID(env, class_dev_info, "eplug", "L" CLASS_E_PLUG";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	
	(*env)->DeleteLocalRef(env, object);
}

static void CopyAirplugKeyLearnInfo(JNIEnv * env, jclass obj_class, jobject object, cl_air_info_t * airplug_p) {
	jclass clazz;
	jobject object_array, key_object;
	jfieldID fid;
	cl_air_key * key_p;
	int i;

	if (airplug_p->is_support_key_learn) {
		jniCopyBooleanValue(env, obj_class, "is_support_key_learn",  object, airplug_p->is_support_key_learn);
	}
	if (airplug_p->is_support_learn_snapshort) {
		jniCopyBooleanValue(env, obj_class, "is_support_learn_snapshort",  object, airplug_p->is_support_learn_snapshort);
	}
	if (airplug_p->is_support_switch_pan) {
		jniCopyBooleanValue(env, obj_class, "is_support_switch_pan",  object, airplug_p->is_support_switch_pan);
		jniCopyByteValue(env, obj_class, "current_pan_type",  object,  airplug_p->current_pan_type);
	}

	if (airplug_p->is_support_learn_snapshort) {
		jniCopyBooleanValue(env, obj_class, "is_support_learn_snapshort",  object, airplug_p->is_support_learn_snapshort);
	}
	
	if (airplug_p->key_info.stat_valid) {
		jniCopyBooleanValue(env, obj_class, "is_key_info_stat_valid",  object, airplug_p->key_info.stat_valid);

		air_key_stat_t* stat_p = &airplug_p->key_info.stat;
		clazz = (*env)->FindClass(env, CLASS_AIR_PLUG_KEY_STAT);
		key_object = (*env)->AllocObject(env, clazz);
	
		jni_copy_simple_class(env,clazz,key_object,
								TRIPLES(byte, stat_p, onoff),
								TRIPLES(byte, stat_p, mode),
								TRIPLES(byte, stat_p, temp),
								TRIPLES(byte, stat_p, wind),
								TRIPLES(byte, stat_p, wind_direct),
								TRIPLES(byte, stat_p, key),
								JNI_VAR_ARG_END);

		fid = (*env)->GetFieldID(env, obj_class, "key_stat", "L" CLASS_AIR_PLUG_KEY_STAT ";");
		(*env)->SetObjectField(env, object, fid, key_object);
	
		SAFE_DEL_LOCAL_REF(clazz);
		SAFE_DEL_LOCAL_REF(key_object);
	}
	
	if (airplug_p->key_info.key_num <= 0) {
		return;	
	}
	
	clazz = (*env)->FindClass(env, CLASS_AIR_PLUG_KEY_INFO);
	if (clazz != NULL) {
		object_array = (*env)->NewObjectArray(env, airplug_p->key_info.key_num, clazz, NULL);
		key_p = airplug_p->key_info.keys;
		
		for (i = 0; i < airplug_p->key_info.key_num; i++, key_p++) {
			key_object = (*env)->AllocObject(env, clazz);
			if (!key_object) break;
			
			jniCopyByteValue(env,clazz,"key_id",key_object, key_p->key_id);
			jniCopyBooleanValue(env,clazz,"is_support_learn",key_object, key_p->is_support_learn);
			jniCopyBooleanValue(env,clazz,"is_support_change_name",key_object, key_p->is_support_change_name);
			jniCopyBooleanValue(env,clazz,"is_support_delete",key_object, key_p->is_support_delete);
			jniCopyBooleanValue(env,clazz,"is_learn_code",key_object, key_p->is_learn_code);

			jniCopyBooleanValue(env,clazz,"is_snapshot_key",key_object, key_p->is_snapshot_key);
			jniCopyBooleanValue(env,clazz,"is_need_decode",key_object, key_p->is_need_decode);

			jniCopyString(env, clazz, "name",  key_object, key_p->name);
			
			(*env)->SetObjectArrayElement(env, object_array, i, key_object);
			(*env)->DeleteLocalRef(env, key_object);
		}

		fid = (*env)->GetFieldID(env, obj_class, "keys", "[L" CLASS_AIR_PLUG_KEY_INFO ";");
		(*env)->SetObjectField(env, object, fid, object_array);

		SAFE_DEL_LOCAL_REF(object_array);
		SAFE_DEL_LOCAL_REF(clazz);
	}

	
}

static void copyAirplugRcInfo(JNIEnv* env, jclass class_enhance, jobject obj_enhance, char *attr_name, cl_rc_info *rc_info)
{
	jclass class_rc = NULL;
	jobject obj_rc = NULL;
	jfieldID fid = NULL;

	class_rc = (*env)->FindClass(env, CLASS_AIR_PLUG_RC_INFO);
	obj_rc = (*env)->AllocObject(env, class_rc);

	jni_copy_simple_class(env,class_rc,obj_rc,
		                    TRIPLES(byte,rc_info,d_id),
		                    TRIPLES(boolean,rc_info,is_matched),
		                    TRIPLES(byte,rc_info,fixed_key_num),
		                    TRIPLES(byte,rc_info,user_def_key_num),
		                    TRIPLES(short,rc_info,matched_ir_id),
		                    JNI_VAR_ARG_END);
	if (rc_info->fixed_key_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_rc, obj_rc, CLASS_AIR_PLUG_RC_FIXED_KEY_INFO,
			                    fk,rc_info->fixed_key_num, sizeof(cl_rc_fixed_key_info),
			                    ARRAY_TRIPLES(byte, rc_info->fk,key_id),
			                    ARRAY_TRIPLES(boolean, rc_info->fk,has_code),
			                    JNI_VAR_ARG_END);
	}
	if (rc_info->user_def_key_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_rc, obj_rc, CLASS_AIR_PLUG_RC_USER_KEY_INFO,
			                    uk,rc_info->user_def_key_num, sizeof(cl_rc_user_key_info),
			                    ARRAY_TRIPLES(byte, rc_info->uk, key_id),
			                    ARRAY_TRIPLES(boolean, rc_info->uk, has_code),
			                    ARRAY_TRIPLES(String, rc_info->uk, name),
			                    JNI_VAR_ARG_END);
	}
	
	fid = (*env)->GetFieldID(env, class_enhance, attr_name, "L"CLASS_AIR_PLUG_RC_INFO";");
	(*env)->SetObjectField(env, obj_enhance, fid, obj_rc);

	SAFE_DEL_LOCAL_REF(class_rc);
	SAFE_DEL_LOCAL_REF(obj_rc);
}

static void copyAirplugEnhanceInfo(JNIEnv* env, jclass class_wk, jobject obj_wk, cl_air_info_t *wk_info)
{
	jclass class_enhance = NULL;
	jobject obj_enhance = NULL;
	jfieldID fid = NULL;

	class_enhance = (*env)->FindClass(env, CLASS_AIR_PLUG_PAIR_RC_INFO);
	obj_enhance = (*env)->AllocObject(env, class_enhance);

	jniCopyString(env, class_enhance,"name", obj_enhance ,wk_info->priv_rc.name);
	copyAirplugRcInfo(env, class_enhance, obj_enhance, "tv_info", &wk_info->priv_rc.tv_info);
	copyAirplugRcInfo(env, class_enhance, obj_enhance, "stb_info", &wk_info->priv_rc.stb_info);

	fid = (*env)->GetFieldID(env, class_wk, "priv_rc", "L"CLASS_AIR_PLUG_PAIR_RC_INFO";");
	(*env)->SetObjectField(env, obj_wk, fid, obj_enhance);

	SAFE_DEL_LOCAL_REF(class_enhance);
	SAFE_DEL_LOCAL_REF(obj_enhance);
}

static void copyAirplugHourLinesInfo(JNIEnv* env, jclass class_wk, jobject obj_wk, cl_air_info_t *wk_info)
{
	jclass clazz = NULL;
	jobject obj = NULL;
	jfieldID fid = NULL;

	clazz = (*env)->FindClass(env, CLASS_AIR_PLUG_HOUR_LINES);
	obj = (*env)->AllocObject(env, clazz);
	
	jni_copy_simple_class(env,clazz,obj,
		                    TRIPLES(boolean,&(wk_info->room_temp_line), is_valid), 
		                    TRIPLES(byte,&(wk_info->room_temp_line), num),
		                    QUADRUPLE(byte[],&((wk_info->room_temp_line)),pad, 2),
		                    QUADRUPLE(byte[],&((wk_info->room_temp_line)),data, 24*6),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_wk, "room_temp_line", "L"CLASS_AIR_PLUG_HOUR_LINES";");
	(*env)->SetObjectField(env, obj_wk, fid, obj);
	SAFE_DEL_LOCAL_REF(obj);

	obj = (*env)->AllocObject(env, clazz);
	jni_copy_simple_class(env,clazz,obj,
		                    TRIPLES(boolean,&(wk_info->humi_line), is_valid), 
		                    TRIPLES(byte,&(wk_info->humi_line), num),
		                    QUADRUPLE(byte[],&((wk_info->humi_line)),pad, 2),
		                    QUADRUPLE(byte[],&((wk_info->humi_line)),data, 24*6),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_wk, "humi_line", "L"CLASS_AIR_PLUG_HOUR_LINES";");
	(*env)->SetObjectField(env, obj_wk, fid, obj);
	SAFE_DEL_LOCAL_REF(obj);

	SAFE_DEL_LOCAL_REF(clazz);
}

static void copyShareInfo(JNIEnv * env, jclass class_airplug, jobject obj_airplug, cl_air_info_t * airplug_p)
{
	jclass class_share;
	jobject obj_share;
	jfieldID fid;

	if (airplug_p == NULL) {
		return;
	}
	if (airplug_p->requested_share_code != NULL) {
		jniCopyString(env,class_airplug, "requested_share_code", obj_airplug, airplug_p->requested_share_code);
	}
	class_share = (*env)->FindClass(env, CLASS_SHARE);
	obj_share = (*env)->AllocObject(env, class_share);

	jni_copy_simple_class(env, class_share, obj_share,
		                     TRIPLES(boolean, &airplug_p->share_info, is_share_data_valid),
		                     TRIPLES(boolean, &airplug_p->share_info, is_super_user),
		                     TRIPLES(byte, &airplug_p->share_info, record_num),
		                     TRIPLES(byte, &airplug_p->share_info, v1_remain_days),
		                     TRIPLES(int, &airplug_p->share_info, cur_phone_index),
		                     JNI_VAR_ARG_END);
	if (airplug_p->share_info.record_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_share, obj_share, CLASS_SHARE_RECORD,
			                   records, airplug_p->share_info.record_num, sizeof(cl_share_record_t),
			                   ARRAY_TRIPLES(int, airplug_p->share_info.records, phone_index),
			                   ARRAY_TRIPLES(int, airplug_p->share_info.records, phone_share_time),
			                   ARRAY_TRIPLES(int, airplug_p->share_info.records, phone_operate_num),
			                   ARRAY_TRIPLES(int, airplug_p->share_info.records, phone_last_operate_time),
			                   ARRAY_TRIPLES(String, airplug_p->share_info.records, phone_desc),
			                   JNI_VAR_ARG_END);
	}

	fid = (*env)->GetFieldID(env, class_airplug, "share_info", "L"CLASS_SHARE";");
	(*env)->SetObjectField(env, obj_airplug, fid, obj_share);
	SAFE_DEL_LOCAL_REF(class_share);
	SAFE_DEL_LOCAL_REF(obj_share);
}

static void copyAirPlugMsgConfig(JNIEnv * env, jclass class_airplug, jobject obj_airplug, cl_air_info_t * airplug_p)
{
	jclass class_msg;
	jobject obj_msg;
	jfieldID fid;

	if (airplug_p == NULL) {
		return;
	}
	
	class_msg = (*env)->FindClass(env, CLASS_AIR_PLUG_MSG_CONFIG);
	obj_msg = (*env)->AllocObject(env, class_msg);

	jniCopyBooleanValue(env, class_airplug, "is_support_msg_config", obj_airplug, airplug_p->is_support_msg_config);
	jni_copy_simple_class(env, class_msg, obj_msg,
		                     TRIPLES(boolean, &airplug_p->msg_config, onoff),
		                     TRIPLES(boolean, &airplug_p->msg_config, timer),
		                     TRIPLES(boolean, &airplug_p->msg_config, sync),
		                     TRIPLES(boolean, &airplug_p->msg_config, temp_ctrl),
		                     TRIPLES(boolean, &airplug_p->msg_config, curve),
		                     TRIPLES(boolean, &airplug_p->msg_config, sleep),
		                     TRIPLES(boolean, &airplug_p->msg_config, smart_on),
		                     TRIPLES(boolean, &airplug_p->msg_config, poweron_restore),
		                     TRIPLES(boolean, &airplug_p->msg_config, linkage_ctrl),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_airplug, "msg_config", "L"CLASS_AIR_PLUG_MSG_CONFIG";");
	(*env)->SetObjectField(env, obj_airplug, fid, obj_msg);
	SAFE_DEL_LOCAL_REF(class_msg);
	SAFE_DEL_LOCAL_REF(obj_msg);
}


static void CopyAirPlugInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{

	jobject object = NULL;
	jclass obj_class,timer_class,smart_class,elec_class, code_class;
	jfieldID fid;
	cl_air_info_t* airplugp;
	cl_air_timer_t* t_timer;
	cl_period_timer_t *period_timer;
	cl_ac_code_item_t *code_item_p;
	cl_temp_curve_t *temp_curve_p;
	tmp_curve_t *curve_p;
	jobject object_array;
	jobject timer_object;
	jobject smart_object;
	jobject elec_object;
	jobject code_object;
	jobject temp_object;
	int i,room_temp, current_code_id, len, cout,offset,total;

	//check param
	if( !obj_dev_info || !info || info->air_info == NULL)
		return;

	obj_class = (*env)->FindClass(env, CLASS_AIR_PLUG);
	if( !obj_class ){
		return;
	}

	airplugp = info->air_info;
	object = (*env)->AllocObject(env, obj_class);

	if( !object || !airplugp ){
		return;
	}

	jniCopyIntValue(env,obj_class,"handle",object,airplugp->handle);
	jniCopyBooleanValue(env,obj_class,"air_led_on_off",object,airplugp->air_led_on_off);
	jniCopyIntValue(env,obj_class,"air_led_smart_on_off",object,airplugp->air_led_on_off);

	jniCopyBooleanValue(env,obj_class,"smart_on_enable",object,airplugp->smart_on_enable);
	jniCopyBooleanValue(env,obj_class,"smart_off_enable",object,airplugp->smart_off_enable);
	jniCopyBooleanValue(env,obj_class,"smart_sleep_enable",object,airplugp->smart_sleep_enable);

	if((airplugp->room_temp & 0x80) != 0){
		room_temp = 0xFFFFFF00 | airplugp->room_temp; 
	}else{
		room_temp  = airplugp->room_temp;
	}

	jniCopyIntValue(env,obj_class,"room_temp",object,room_temp);
	jniCopyIntValue(env,obj_class,"cur_power",object,airplugp->cur_power);
	jniCopyIntValue(env,obj_class,"cur_milli_power",object,airplugp->cur_milli_power);
	
	jniCopyBooleanValue(env,obj_class,"onoff",object,airplugp->air_work_stat.onoff);
	jniCopyIntValue(env,obj_class,"mode",object,airplugp->air_work_stat.mode);
	jniCopyIntValue(env,obj_class,"temp",object,airplugp->air_work_stat.temp);
	jniCopyIntValue(env,obj_class,"wind",object,airplugp->air_work_stat.wind);
	
	jniCopyIntValue(env,obj_class,"wind_direct",object,airplugp->air_work_stat.wind_direct);

	jniCopyBooleanValue(env,obj_class,"on_effect",object,airplugp->air_timer_info.on_effect);
	jniCopyBooleanValue(env,obj_class,"off_effect",object,airplugp->air_timer_info.off_effect);
	jniCopyIntValue(env,obj_class,"timer_count",object,airplugp->air_timer_info.timer_count);
	jniCopyIntValue(env,obj_class,"next_on_day",object,airplugp->air_timer_info.next_on_day);
	jniCopyIntValue(env,obj_class,"next_on_hour",object,airplugp->air_timer_info.next_on_hour);
	jniCopyIntValue(env,obj_class,"next_on_min",object,airplugp->air_timer_info.next_on_min);
	jniCopyIntValue(env,obj_class,"next_off_day",object,airplugp->air_timer_info.next_off_day);
	jniCopyIntValue(env,obj_class,"next_off_hour",object,airplugp->air_timer_info.next_off_hour);
	
	jniCopyIntValue(env,obj_class,"next_off_min",object,airplugp->air_timer_info.next_off_min);

	jniCopyBooleanValue(env,obj_class,"is_support_led_color",object,airplugp->is_support_led_color);
	if(airplugp->is_support_led_color){
		cl_air_led_color_info air_led_color_info = airplugp->led_color;
		jniCopyIntValue(env,obj_class,"led_color_on",object,air_led_color_info.air_on_color);
		jniCopyIntValue(env,obj_class,"led_color_off",object,air_led_color_info.air_off_color);
	}
	jniCopyBooleanValue(env, obj_class, "is_support_period_timer", object, airplugp->is_support_peroid_timer);
	jniCopyBooleanValue(env, obj_class, "is_support_peroid_ext_timer", object, airplugp->is_support_peroid_ext_timer);
	if (airplugp->air_timer_info.timer_count> 0) {
		if (airplugp->is_support_peroid_timer) {
			if (airplugp->air_timer_info.period_timers!= NULL) {
				timer_class = (*env)->FindClass(env, CLASS_AIR_PLUG_PERIOD_TIMER);
				object_array = (*env)->NewObjectArray(env, airplugp->air_timer_info.timer_count, timer_class, NULL);
				period_timer = airplugp->air_timer_info.period_timers;

				for (i = 0; i < airplugp->air_timer_info.timer_count; i++,period_timer++) {
					timer_object = (*env)->AllocObject(env, timer_class);
					if(!timer_object)
						break;

					jniCopyIntValue(env,timer_class,"id",timer_object,period_timer->id);
					jniCopyIntValue(env,timer_class,"hour",timer_object,period_timer->hour);
					jniCopyIntValue(env,timer_class,"minute",timer_object,period_timer->minute);
					jniCopyIntValue(env,timer_class,"week",timer_object,period_timer->week);
					jniCopyBooleanValue(env,timer_class,"enable",timer_object,period_timer->enable);
					jniCopyBooleanValue(env,timer_class,"onoff",timer_object,period_timer->onoff);
					jniCopyIntValue(env,timer_class,"duration",timer_object,period_timer->duration);
					jniCopyIntValue(env,timer_class,"ext_data_type",timer_object,period_timer->ext_data_type);
					
					if(airplugp->is_support_peroid_ext_timer){
						jniCopyBooleanValue(env,timer_class,"onOff",timer_object,period_timer->pt_ext_data_u.air_timer_info.onOff);
						jniCopyByteValue(env, timer_class, "mode", timer_object, period_timer->pt_ext_data_u.air_timer_info.mode);
						jniCopyByteValue(env, timer_class, "temp", timer_object, period_timer->pt_ext_data_u.air_timer_info.temp);
						jniCopyByteValue(env, timer_class, "fan_speed", timer_object, period_timer->pt_ext_data_u.air_timer_info.fan_speed);
						jniCopyByteValue(env, timer_class, "fan_dir", timer_object, period_timer->pt_ext_data_u.air_timer_info.fan_dir);
						jniCopyByteValue(env, timer_class, "key_id", timer_object, period_timer->pt_ext_data_u.air_timer_info.key_id);
					}
					(*env)->SetObjectArrayElement(env, object_array, i, timer_object);
					(*env)->DeleteLocalRef(env, timer_object);
				}

				fid = (*env)->GetFieldID(env, obj_class, "period_timers", "[L" CLASS_AIR_PLUG_PERIOD_TIMER ";");
				(*env)->SetObjectField(env, object, fid, object_array);
				
				SAFE_DEL_LOCAL_REF(object_array);
				SAFE_DEL_LOCAL_REF(timer_class);
			}
		} else if (!airplugp->is_support_peroid_timer && airplugp->air_timer_info.timers != NULL) {
			timer_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TIMER);
			object_array = (*env)->NewObjectArray(env, airplugp->air_timer_info.timer_count, timer_class, NULL);

			t_timer = airplugp->air_timer_info.timers;
			for (i = 0; i < airplugp->air_timer_info.timer_count; i++,t_timer++) {
				timer_object = (*env)->AllocObject(env, timer_class);
				if(!timer_object)
					break;

				jniCopyIntValue(env,timer_class,"id",timer_object,t_timer->id);
				jniCopyBooleanValue(env,timer_class,"enable",timer_object,t_timer->enable);

				jniCopyIntValue(env,timer_class,"week",timer_object,t_timer->week);
				jniCopyIntValue(env,timer_class,"hour",timer_object,t_timer->hour);
				jniCopyIntValue(env,timer_class,"minute",timer_object,t_timer->minute);
				jniCopyBooleanValue(env,timer_class,"onoff",timer_object,t_timer->onoff);

				(*env)->SetObjectArrayElement(env, object_array, i, timer_object);
				(*env)->DeleteLocalRef(env, timer_object);
			}
			
			fid = (*env)->GetFieldID(env, obj_class, "timers", "[L" CLASS_AIR_PLUG_TIMER ";");
			(*env)->SetObjectField(env, object, fid, object_array);
			
			SAFE_DEL_LOCAL_REF(object_array);
			SAFE_DEL_LOCAL_REF(timer_class);
		}
		/*
		else if (airplugp->is_support_peroid_timer && airplugp->air_timer_info.period_timers!= NULL) {
			timer_class = (*env)->FindClass(env, CLASS_AIR_PLUG_PERIOD_TIMER);
			object_array = (*env)->NewObjectArray(env, airplugp->air_timer_info.timer_count, timer_class, NULL);
			period_timer = airplugp->air_timer_info.period_timers;

			for (i = 0; i < airplugp->air_timer_info.timer_count; i++,period_timer++) {
				timer_object = (*env)->AllocObject(env, timer_class);
				if(!timer_object)
					break;

				jniCopyIntValue(env,timer_class,"id",timer_object,period_timer->id);
				jniCopyIntValue(env,timer_class,"hour",timer_object,period_timer->hour);
				jniCopyIntValue(env,timer_class,"minute",timer_object,period_timer->minute);
				jniCopyIntValue(env,timer_class,"week",timer_object,period_timer->week);
				jniCopyBooleanValue(env,timer_class,"enable",timer_object,period_timer->enable);
				jniCopyBooleanValue(env,timer_class,"onoff",timer_object,period_timer->onoff);
				jniCopyIntValue(env,timer_class,"duration",timer_object,period_timer->duration);

				(*env)->SetObjectArrayElement(env, object_array, i, timer_object);
				(*env)->DeleteLocalRef(env, timer_object);
			}

			fid = (*env)->GetFieldID(env, obj_class, "period_timers", "[L" CLASS_AIR_PLUG_PERIOD_TIMER ";");
			(*env)->SetObjectField(env, object, fid, object_array);
			
			SAFE_DEL_LOCAL_REF(object_array);
			SAFE_DEL_LOCAL_REF(timer_class);
		}
		*/
	}
	CopyElecState(env, obj_class, "elec_stat_info", object, &airplugp->air_elec_stat_info);
	CopyElecDaysStatInfo(env, obj_class, "elec_days_info", object, &airplugp->elec_days_info);
	elec_class = (*env)->FindClass(env, CLASS_ELEC_ITEM);
	if(elec_class != NULL){
		elec_object =  (*env)->AllocObject(env, elec_class);
		if(elec_object != NULL){
			jniCopyIntValue(env,elec_class,"begin_time",elec_object,airplugp->total_elec.begin_time);
			jniCopyIntValue(env,elec_class,"elec",elec_object,airplugp->total_elec.elec);

			fid = (*env)->GetFieldID(env, obj_class, "total_elec", "L" CLASS_ELEC_ITEM ";");
			(*env)->SetObjectField(env, object, fid, elec_object);
			
			SAFE_DEL_LOCAL_REF(elec_object);
		}
		SAFE_DEL_LOCAL_REF(elec_class);
	}


	elec_class = (*env)->FindClass(env, CLASS_ELEC_ITEM);
	if(elec_class != NULL){
		elec_object =  (*env)->AllocObject(env, elec_class);
		if(elec_object != NULL){


			jniCopyIntValue(env,elec_class,"begin_time",elec_object,airplugp->period_elec.begin_time);
			jniCopyIntValue(env,elec_class,"elec",elec_object,airplugp->period_elec.elec);

			fid = (*env)->GetFieldID(env, obj_class, "period_elec", "L" CLASS_ELEC_ITEM ";");
			(*env)->SetObjectField(env, object, fid, elec_object);
			
			SAFE_DEL_LOCAL_REF(elec_object);
		}
		SAFE_DEL_LOCAL_REF(elec_class);
	}

	elec_class = (*env)->FindClass(env, CLASS_ELEC_ITEM);
	if(elec_class != NULL){
		elec_object =  (*env)->AllocObject(env, elec_class);
		if(elec_object != NULL){

			jniCopyIntValue(env,elec_class,"begin_time",elec_object,airplugp->last_on_elec.begin_time);
			jniCopyIntValue(env,elec_class,"elec",elec_object,airplugp->last_on_elec.elec);

			fid = (*env)->GetFieldID(env, obj_class, "last_on_elec", "L" CLASS_ELEC_ITEM ";");
			(*env)->SetObjectField(env, object, fid, elec_object);
			
			
			SAFE_DEL_LOCAL_REF(elec_object);
		}
		SAFE_DEL_LOCAL_REF(elec_class);
	}
	
	
	smart_class =  (*env)->FindClass(env, CLASS_AIR_PLUG_SMART_ON);
	if(smart_class != NULL){
		smart_object =  (*env)->AllocObject(env, smart_class);
		if(smart_object != NULL){

			jniCopyBooleanValue(env,smart_class,"on",smart_object,airplugp->smart_on_info.on);
			jniCopyBooleanValue(env,smart_class,"push_on",smart_object,airplugp->smart_on_info.push_on);
			jniCopyBooleanValue(env,smart_class,"sum_on",smart_object,airplugp->smart_on_info.sum_on);
			jniCopyBooleanValue(env,smart_class,"win_on",smart_object,airplugp->smart_on_info.win_on);
			jniCopyBooleanValue(env,smart_class,"home_on",smart_object,airplugp->smart_on_info.home_on);

			jniCopyIntValue(env,smart_class,"sum_tmp",smart_object,airplugp->smart_on_info.sum_tmp);
			jniCopyIntValue(env,smart_class,"win_tmp",smart_object,airplugp->smart_on_info.win_tmp);
			
			fid = (*env)->GetFieldID(env, obj_class, "smart_on_info", "L" CLASS_AIR_PLUG_SMART_ON ";");
			(*env)->SetObjectField(env, object, fid, smart_object);
			
			SAFE_DEL_LOCAL_REF(smart_object);
		}
		SAFE_DEL_LOCAL_REF(smart_class);
	}

	smart_class =  (*env)->FindClass(env, CLASS_AIR_PLUG_SMART_OFF);
	if(smart_class != NULL){
		smart_object =  (*env)->AllocObject(env, smart_class);
		if(smart_object != NULL){
			
			jniCopyBooleanValue(env,smart_class,"on",smart_object,airplugp->smart_off_info.on);
			jniCopyBooleanValue(env,smart_class,"push_on",smart_object,airplugp->smart_off_info.push_on);
			jniCopyIntValue(env,smart_class,"off_time",smart_object,airplugp->smart_off_info.off_time);

			fid = (*env)->GetFieldID(env, obj_class, "smart_off_info", "L" CLASS_AIR_PLUG_SMART_OFF ";");
			(*env)->SetObjectField(env, object, fid, smart_object);
			
			SAFE_DEL_LOCAL_REF(smart_object);
		}
		SAFE_DEL_LOCAL_REF(smart_class);
	}

	smart_class =  (*env)->FindClass(env, CLASS_AIR_PLUG_SMART_SLEEP);
	if(smart_class != NULL){
		smart_object =  (*env)->AllocObject(env, smart_class);
		if(smart_object != NULL){

			jniCopyBooleanValue(env,smart_class,"on",smart_object,airplugp->smart_sleep_info.on);

			fid = (*env)->GetFieldID(env, obj_class, "smart_sleep_info", "L" CLASS_AIR_PLUG_SMART_SLEEP ";");
			(*env)->SetObjectField(env, object, fid, smart_object);
			
			SAFE_DEL_LOCAL_REF(smart_object);
		}
		SAFE_DEL_LOCAL_REF(smart_class);
	}

	current_code_id = airplugp->last_match_info.cur_match_id;
	if(current_code_id != 0 && airplugp->last_match_info.code_num > 0){
		jniCopyIntValue(env, obj_class, "current_code_id", object, current_code_id);
		smart_class =  (*env)->FindClass(env, CLASS_AIR_PLUG_MATCH_CODE_INFO);
		if(smart_class != NULL){
			object_array = (*env)->NewObjectArray(env, airplugp->last_match_info.code_num, smart_class, NULL);
			code_item_p = airplugp->last_match_info.items;
			for(i = 0; i < airplugp->last_match_info.code_num; i++, code_item_p++){
				code_object = (*env)->AllocObject(env, smart_class);
				if(!code_object)
					break;

				jniCopyIntValue(env,smart_class,"c_id",code_object, code_item_p->c_id);
				jniCopyBooleanValue(env,smart_class,"is_on",code_object,code_item_p->is_on);
				jniCopyIntValue(env,smart_class,"mode",code_object,code_item_p->mode);
				jniCopyIntValue(env,smart_class,"temp",code_object,code_item_p->temp);
				jniCopyIntValue(env,smart_class,"fan",code_object,code_item_p->fan);
				jniCopyIntValue(env,smart_class,"fan_dir",code_object,code_item_p->fan_dir);
				jniCopyIntValue(env,smart_class,"key",code_object,code_item_p->key);

				(*env)->SetObjectArrayElement(env, object_array, i, code_object);
				(*env)->DeleteLocalRef(env, code_object);
			}

			fid = (*env)->GetFieldID(env, obj_class, "code_infos", "[L" CLASS_AIR_PLUG_MATCH_CODE_INFO ";");
			(*env)->SetObjectField(env, object, fid, object_array);
			SAFE_DEL_LOCAL_REF(object_array);
		}
		SAFE_DEL_LOCAL_REF(smart_class);
	}

	jniCopyBooleanValue(env, obj_class, "is_support_param_ajust", object, airplugp->is_support_param_ajust);
	JNI_COPY_SIMPLE_CLASS(env,obj_class,object,CLASS_WUKONG_PARAM_ADJUST, ajust_info,
		                     TRIPLES(boolean ,&airplugp->ajust_info, is_same_onoff_code),
		                     TRIPLES(boolean ,&airplugp->ajust_info, is_same_fan),
		                     TRIPLES(boolean ,&airplugp->ajust_info, is_fan_speed_opposite),
		                     JNI_VAR_ARG_END);
	jniCopyBooleanValue(env, obj_class, "is_support_room_temp_ajust", object, airplugp->is_support_room_temp_ajust);
	jniCopyBooleanValue(env, obj_class, "is_support_elec_ajust", object, airplugp->is_support_elec_ajust);
	jniCopyShortValue(env, obj_class,"env_temp_ajust_value",object, airplugp->env_temp_ajust_value);
	jniCopyShortValue(env, obj_class,"elec_ajust_value",object, airplugp->elec_ajust_value);

	jniCopyBooleanValue(env, obj_class, "is_support_child_lock", object, airplugp->is_support_child_lock);
	jniCopyByteValue(env, obj_class,"child_lock_value",object, airplugp->child_lock_value);

	jniCopyByteValue(env, obj_class,"temp_humidity",object, airplugp->temp_humidity);
	jniCopyShortValue(env, obj_class,"env_room_temp_low",object, airplugp->env_room_temp_low);
	jniCopyShortValue(env, obj_class,"env_room_temp_high",object, airplugp->env_room_temp_high);


	jniCopyBooleanValue(env, obj_class, "is_support_temp_curve", object, airplugp->is_support_temp_curve);
	if (airplugp->is_support_temp_curve && NULL != airplugp->temp_curve && 
		airplugp->temp_curve_len >= sizeof(cl_temp_curve_t)) {
		
		smart_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CURVE);
		total = airplugp->temp_curve_len;
		offset = 0;
		cout = 0;

		while(total > sizeof(cl_temp_curve_t)){
			temp_curve_p = (cl_temp_curve_t*)(((char*)airplugp->temp_curve) + offset);
			curve_p = (tmp_curve_t*) (temp_curve_p + 1);
			len = sizeof(cl_temp_curve_t) + temp_curve_p->count * sizeof(tmp_curve_t);
			// calc count;
			cout++;
			
			offset += len;
			total-=len;
		}

		total = airplugp->temp_curve_len;
		offset = 0;
		object_array = (*env)->NewObjectArray(env, cout, smart_class, NULL);

		i = 0;
		while(total > sizeof(cl_temp_curve_t)){
			temp_curve_p = (cl_temp_curve_t*)(((char*)airplugp->temp_curve) + offset);
			curve_p = (tmp_curve_t*) (temp_curve_p + 1);
			len = sizeof(cl_temp_curve_t) + temp_curve_p->count * sizeof(tmp_curve_t);

			temp_object = (*env)->AllocObject(env, smart_class);
			jni_copy_simple_class(env, smart_class, temp_object,
									TRIPLES(byte, temp_curve_p, id),
									TRIPLES(boolean, temp_curve_p, enable),
									TRIPLES(byte, temp_curve_p, week),
									TRIPLES(byte, temp_curve_p, begin_hour),
									TRIPLES(byte, temp_curve_p, end_hour),
									TRIPLES(byte, temp_curve_p, time_period),
									TRIPLES(byte, temp_curve_p, count),
									JNI_VAR_ARG_END
									);
			
			JNI_COPY_ARRAY_CLASS(env, smart_class, temp_object, CLASS_AIR_PLUG_TEMP_CURVE_LINE, 
					                    curves , temp_curve_p->count, sizeof(tmp_curve_t),
				 						ARRAY_TRIPLES(byte, curve_p, flag),
					                    ARRAY_TRIPLES(byte, curve_p, tmp),
					                    JNI_VAR_ARG_END);

			(*env)->SetObjectArrayElement(env, object_array, i, temp_object);
			SAFE_DEL_LOCAL_REF(temp_object);
			
			offset += len;
			total-=len;
			i++;
		}
		

		fid = (*env)->GetFieldID(env, obj_class, "tempCurve", "[L" CLASS_AIR_PLUG_TEMP_CURVE ";");
		(*env)->SetObjectField(env, object, fid, object_array);

		SAFE_DEL_LOCAL_REF(smart_class);
		SAFE_DEL_LOCAL_REF(object_array);
	}

	jniCopyBooleanValue(env, obj_class, "is_support_temp_ac_ctrl", object, airplugp->is_support_temp_ac_ctrl);
	if (airplugp->is_support_temp_ac_ctrl) {
		smart_class = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CTRL);
		temp_object = (*env)->AllocObject(env, smart_class);

		jni_copy_simple_class(env, smart_class, temp_object,
								TRIPLES(boolean, &(airplugp->tac),enable),
								TRIPLES(byte, &(airplugp->tac),mode),
								TRIPLES(byte, &(airplugp->tac),temp_min),
								TRIPLES(byte, &(airplugp->tac),temp_max),
								TRIPLES(byte, &(airplugp->tac),week),
								TRIPLES(byte, &(airplugp->tac),begin_hour),
								TRIPLES(byte, &(airplugp->tac),end_hour),
								JNI_VAR_ARG_END);
								
		fid = (*env)->GetFieldID(env, obj_class, "tempCtrl", "L" CLASS_AIR_PLUG_TEMP_CTRL ";");
		(*env)->SetObjectField(env, object, fid, temp_object);

		SAFE_DEL_LOCAL_REF(smart_class);
		SAFE_DEL_LOCAL_REF(temp_object);						
	}

	copyAirplugEnhanceInfo(env, obj_class, object, airplugp);
	CopyAirplugKeyLearnInfo(env, obj_class, object, airplugp);
	copyAirplugHourLinesInfo(env, obj_class, object, airplugp);
	copyShareInfo(env, obj_class, object, airplugp);
	jniCopyBooleanValue(env, obj_class, "is_support_elec_stat", object, info->com_udp_info->is_suppport_elec_stat);
	
	if (info->com_udp_info != NULL) {
		jniCopyBooleanValue(env, obj_class, "is_support_public_shortcuts_onoff", object, info->com_udp_info->is_support_public_shortcuts_onoff);
		
		JNI_COPY_SIMPLE_CLASS(env, obj_class, object, CLASS_COMM_SHORTCUT_POWER, shortcuts_onoff,
		                     TRIPLES(boolean, &info->com_udp_info->shortcuts_onoff, enable),
		                     TRIPLES(boolean, &info->com_udp_info->shortcuts_onoff, onoff),
		                     TRIPLES(int, &info->com_udp_info->shortcuts_onoff, time),
		                     TRIPLES(int, &info->com_udp_info->shortcuts_onoff, remain_time),
		                     JNI_VAR_ARG_END);
	}

	copyAirPlugMsgConfig(env, obj_class, object, airplugp);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "airPlug", "L" CLASS_AIR_PLUG ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, object);
	
	(*env)->DeleteLocalRef(env, object);
	(*env)->DeleteLocalRef(env, obj_class);
	
}

static void CopyBlanketSectorInfo(JNIEnv* env,jclass blanket_class, jobject blanket_object, cl_ia_ch_area_info_t *setcotr_info)
{	
	jniCopyBooleanValue(env, blanket_class, "work_stat", blanket_object, setcotr_info->work_stat);
	jniCopyIntValue(env, blanket_class, "set_temperature", blanket_object, setcotr_info->set_temperature);
	jniCopyIntValue(env, blanket_class, "current_temperature", blanket_object, setcotr_info->current_temperature);
	jniCopyIntValue(env, blanket_class, "off_timer", blanket_object, setcotr_info->off_timer);
	jniCopyBooleanValue(env, blanket_class, "curve_enable", blanket_object, setcotr_info->curve_enable);
	jniCopyIntValue(env, blanket_class, "curve_time_interval", blanket_object, setcotr_info->curve_time_interval);
	jniCopyIntValue(env, blanket_class, "curve_next_work_time", blanket_object, setcotr_info->curve_next_work_time);
	jniCopyIntValue(env, blanket_class, "curve_data_len", blanket_object, setcotr_info->curve_data_len);
	jniCopyIntValue(env, blanket_class, "work_mode", blanket_object, setcotr_info->work_mode);
	jniCopyByteArray(env, blanket_class, "curve_data", blanket_object, setcotr_info->curve_data, setcotr_info->curve_data_len);
}

static void CopyChBlanketInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jfieldID fid;
	jobject blanket_object = NULL;
	jclass blanket_class = NULL;
	jobject left_sector_object = NULL;
	jobject right_sector_object = NULL;
	jclass temp_sector_class = NULL;

	cl_ia_ch_blanket_info_t* cl_ch_blanket = NULL;

	if( !info ){
		return;
	}

	cl_ch_blanket = info->ia_info.u.ptr;
	if(!cl_ch_blanket){
		return;
	}

	blanket_class = (*env)->FindClass(env, CLASS_CH_BLANKET);
	if(!blanket_class){
		goto quit;
	}
	
	blanket_object = (*env)->AllocObject(env, blanket_class);
	if( !blanket_object ){
		goto quit;
	}
	temp_sector_class = (*env)->FindClass(env, CLASS_BLANKET_SECTOR);
	if (NULL == temp_sector_class) {
		goto quit;
	}
	left_sector_object = (*env)->AllocObject(env, temp_sector_class);
	if (NULL == left_sector_object) {
		goto quit;
	}

	CopyBlanketSectorInfo(env, temp_sector_class, left_sector_object, &cl_ch_blanket->left_area_info);
	fid = (*env)->GetFieldID(env, blanket_class, "left_area_info", "L"CLASS_BLANKET_SECTOR";");
	(*env)->SetObjectField(env, blanket_object, fid, left_sector_object);

	right_sector_object = (*env)->AllocObject(env, temp_sector_class);
	if (NULL == right_sector_object) {
		goto quit;
	}
	CopyBlanketSectorInfo(env, temp_sector_class, right_sector_object, &cl_ch_blanket->right_area_info);
	fid = (*env)->GetFieldID(env, blanket_class, "right_area_info", "L"CLASS_BLANKET_SECTOR";");
	(*env)->SetObjectField(env, blanket_object, fid, right_sector_object);

	jniCopyIntValue(env, blanket_class, "dev_handle", blanket_object, info->handle);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "blanket", "L"CLASS_CH_BLANKET";");
	(*env)->SetObjectField(env, obj_dev_info, fid, blanket_object);
quit:
	SAFE_DEL_LOCAL_REF(blanket_object);
	SAFE_DEL_LOCAL_REF(blanket_class);
	SAFE_DEL_LOCAL_REF(left_sector_object);
	SAFE_DEL_LOCAL_REF(right_sector_object);
	SAFE_DEL_LOCAL_REF(temp_sector_class);
}

static void CopyDevIaInformation(JNIEnv* env,jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	//check param
	if( !obj_dev_info || !info || info->ia_info.ia_type == 0 || info->ia_info.u.ptr == NULL)
		return;

	switch ( info->ia_info.ia_type ){
		case IA_AIRCLEANER:
		{
			CopyDevAirCleanerInformation(env,class_dev_info,obj_dev_info,info);
		}
			break;
		case IA_AIRCONDITION:
		{
			CopyDevAirConditionInformation(env,class_dev_info,obj_dev_info,info);
		}
			break;	
		case IA_AIRHEATER:
		{
			if(info->ia_info.ia_sub_type == IA_AIRHEATER_SUBTYPE_PROTO){
				CopyDevAirHeaterInformation(env,class_dev_info,obj_dev_info,info);
			}else if(info->ia_info.ia_sub_type == IA_AIRHEATER_SUBTYPE_YCYT){
				CopyDevAirHeaterYcytInformation(env,class_dev_info,obj_dev_info,info);
			}
		}
			break;
		case IA_ELECTRICFAN:
		{
			CopyDevEletricFanInformation(env,class_dev_info,obj_dev_info,info);
		}
			break;
		case IA_WATERHEATER:
		{
			if(info->ia_info.ia_sub_type == IA_WATERHEATER_SUBTYPE_PROTO){
				CopyDevWaterHeaterInformation(env,class_dev_info,obj_dev_info,info);
			}else if(info->ia_info.ia_sub_type == IA_WATERHEATER_SUBTYPE_A9){
				CopyDevWaterHeaterA9Information(env,class_dev_info,obj_dev_info,info);
			}
		}
		case IA_BATHHEATER_AUPU:
		{
			CopyDevBathHeaterAupuInformation(env,class_dev_info,obj_dev_info,info);
		}
			break;
			
		case IA_UDP_AIR_HEATER:
		{
			 if(info->ia_info.ia_sub_type == IA_EXT_TYPE_LCYT_OIL){
				CopyDevAirHeaterYcytInformation(env,class_dev_info,obj_dev_info,info);
			}
		}
			break;
			
		case IA_UDP_AIR_CLEAR:
		{
			 if(info->ia_info.ia_sub_type == IA_EXT_TYPE_813_HK
			 	|| info->ia_info.ia_sub_type == IA_EXT_TYPE_813_NB){
				CopyDevAirCleanerInformation(env,class_dev_info,obj_dev_info,info);
			}
		}
			break;
			
		case IJ_820:
		{
			if(info->ia_info.ia_sub_type == ETYPE_IJ_820_GX){
				CopyGxLampInformation(env,class_dev_info,obj_dev_info,info);
			}
		}
			break;
		case IJ_821:
		{
			if (ETYPE_IJ_821_CH == info->ia_info.ia_sub_type) {
				CopyChBlanketInformation(env, class_dev_info, obj_dev_info, info);
			}
		}
			break;
		default:
			break;
	}

}

//是否拷贝不支持的设备,开关
static jboolean IS_COPY_NO_SUPPORT_DEV = true;
static jint dev_types_num=0;//dev_types的数量
static dev_type_t * dev_types;

void init_devtype_htl ()
{
	IS_COPY_NO_SUPPORT_DEV = false;

	dev_types_num = 3;
	dev_types = (dev_type_t *)malloc(dev_types_num * sizeof(dev_type_t));

	dev_types[0].sub_type = IJ_RFGW;
	dev_types[0].ext_num = 2;
	dev_types[0].ext_type = (int *)malloc(dev_types[0].ext_num * sizeof(int));
	dev_types[0].ext_type[0] = ETYPE_IJ_RFGW_6621;
	dev_types[0].ext_type[1] = ETYPE_IJ_RFGW_S2;

	dev_types[1].sub_type = IJ_RFGW;
	dev_types[1].ext_num = 1;
	dev_types[1].ext_type = (int *)malloc(dev_types[1].ext_num * sizeof(int));
	dev_types[1].ext_type[0] = ETYPE_IJ_RFGW_S3;

	dev_types[2].sub_type = IJ_RFGW;
	dev_types[2].ext_num = 1;
	dev_types[2].ext_type = (int *)malloc(dev_types[2].ext_num * sizeof(int));
	dev_types[2].ext_type[0] = RF_EXT_TYPE_HTLLOCK;
}

void init_devtype_htl2 ()
{
	IS_COPY_NO_SUPPORT_DEV = false;
	
	dev_types_num = 10;
	dev_types = (dev_type_t *)malloc(dev_types_num * sizeof(dev_type_t));
	
	int i =0,j=0;
	//悟空
	dev_types[i].sub_type = 16;
	dev_types[i].ext_num = 0;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));

	i++;j = 0;
	dev_types[i].sub_type = 152;
	dev_types[i].ext_num = 3;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	dev_types[i].ext_type[j++] = 1;
	dev_types[i].ext_type[j++] = 2;
	dev_types[i].ext_type[j++] = 3;

	//悟能
	i++;j = 0;
	dev_types[i].sub_type = IJ_101;
	dev_types[i].ext_num = 7;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	dev_types[i].ext_type[j++] = 0;
	dev_types[i].ext_type[j++] = 1;
	dev_types[i].ext_type[j++] = 2;
	dev_types[i].ext_type[j++] = 3;
	dev_types[i].ext_type[j++] = 7;
	dev_types[i].ext_type[j++] = 8;
	dev_types[i].ext_type[j++] = 9;


	i++;j = 0;
	dev_types[i].sub_type = IJ_102;
	dev_types[i].ext_num = 7;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	dev_types[i].ext_type[j++] = 0;
	dev_types[i].ext_type[j++] = 1;
	dev_types[i].ext_type[j++] = 2;
	dev_types[i].ext_type[j++] = 3;
	dev_types[i].ext_type[j++] = 7;
	dev_types[i].ext_type[j++] = 8;
	dev_types[i].ext_type[j++] = 9;

	//LEDE
	i++;j = 0;
	dev_types[i].sub_type = 26;
	dev_types[i].ext_num = 0;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));

	//摄像头
	i++;j = 0;
	dev_types[i].sub_type = 55;
	dev_types[i].ext_num = 4;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	dev_types[i].ext_type[j++] = 2;
	dev_types[i].ext_type[j++] = 1;
	dev_types[i].ext_type[j++] = 3;
	dev_types[i].ext_type[j++] = 255;

	i++;j = 0;
	dev_types[i].sub_type = 3;
	dev_types[i].ext_num = 0;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));


	//rf设备
	i++;j = 0;
	dev_types[i].sub_type = IJ_RFGW;
	dev_types[i].ext_num = 25;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	//网关
	dev_types[i].ext_type[j++] = ETYPE_IJ_RFGW_6621;
	dev_types[i].ext_type[j++] = ETYPE_IJ_RFGW_S2;
	dev_types[i].ext_type[j++] = ETYPE_IJ_RFGW_S3;
	//灯
	dev_types[i].ext_type[j++] = 34;
	dev_types[i].ext_type[j++] = 42;
	dev_types[i].ext_type[j++] = 77;
	//门磁
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_DOOR_MAGNET;
	dev_types[i].ext_type[j++] = 39;
	dev_types[i].ext_type[j++] = 43;
	//单火线
	dev_types[i].ext_type[j++] = 37;
	dev_types[i].ext_type[j++] = 75;
	dev_types[i].ext_type[j++] = 76;
	//红外
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_HM_BODY_DETECT;
	//温湿度
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_HM_ENV_DETECT;
	//气感
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_GAS;
	//水感
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_QSJC;
	//海曼情景遥控器
	dev_types[i].ext_type[j++] = 64;
	// 电威遥控器
	dev_types[i].ext_type[j++] = 52;
	dev_types[i].ext_type[j++] = 72;
	//音速灯21键遥控器
	dev_types[i].ext_type[j++] = 98;
	//烟雾
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_HMYW;
	//CO
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_HMCO;
	//SOS
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_HMQJ;
	//智皇电动窗帘
	dev_types[i].ext_type[j++] = RS_EXT_TYPE_ZHDJ;
	//汇泰龙锁
	dev_types[i].ext_type[j++] = RF_EXT_TYPE_HTLLOCK;

	//温控器
	i++;j = 0;
	dev_types[i].sub_type = 96;
	dev_types[i].ext_num = 2;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	dev_types[i].ext_type[j++] = 8;
	dev_types[i].ext_type[j++] = 10;

	//wifi版智皇窗帘
	i++;j = 0;
	dev_types[i].sub_type = 101;
	dev_types[i].ext_num = 1;
	dev_types[i].ext_type = (int *)malloc(dev_types[i].ext_num * sizeof(int));
	dev_types[i].ext_type[j++] = 1;


	i=0;j=0;
	for(i = 0; i < dev_types_num; i++){
		LOGE("find   i = %d, sub_type = %d",i,dev_types[i].sub_type);			
		for(j = 0;j < dev_types[i].ext_num; j++){	
			LOGE("find   j = %d, ext_type = %d",j,dev_types[i].ext_type[j]);
		}

	}
}

JNIEXPORT void JNICALL
NAME(ClSetSupportDevType)(JNIEnv* env, jobject this, jboolean isCopyNoSupportDev,jobject subTypes, jobject extTypes)
{
	if(IS_INIT_DEV_HTL || IS_INIT_DEV_HTL_2){
		return;
	}
	LOGE("find   isCopyNoSupportDev = %d",isCopyNoSupportDev);
	IS_COPY_NO_SUPPORT_DEV = isCopyNoSupportDev;
	if(subTypes == NULL || extTypes == NULL){
		return;
	}
	int i,j;  
	//class ArrayList  
	jclass cls_arraylist = (*env)->FindClass(env,"java/util/ArrayList");  
	//method in class ArrayList  
	jmethodID arraylist_get = (*env)->GetMethodID(env,cls_arraylist,"get","(I)Ljava/lang/Object;");  
	jmethodID arraylist_size = (*env)->GetMethodID(env,cls_arraylist,"size","()I");  
	jint sub_len = (*env)->CallIntMethod(env,subTypes,arraylist_size);  
	//class Integer
	jclass cls_integer = (*env)->FindClass(env,"java/lang/Integer");
	//method in Integer
	jmethodID int_value = (*env)->GetMethodID(env,cls_integer,"intValue","()I");
	jint sub_type = 0;
	jint ext_type = 0;

	LOGE("find   sub_len = %d",sub_len);
	dev_types_num = sub_len;
	dev_types = (dev_type_t *)malloc(sub_len * sizeof(dev_type_t));
			
	for(i=0;i<sub_len;i++){  
		jobject obj_sub = (*env)->CallObjectMethod(env,subTypes,arraylist_get,i);
		sub_type = (*env)->CallIntMethod(env,obj_sub,int_value); 
		dev_types[i].sub_type = sub_type;
		
		jobject obj_ext = (*env)->CallObjectMethod(env,extTypes,arraylist_get,i);
		if(obj_ext == NULL){
			dev_types[i].ext_num = 0;
			dev_types[i].ext_type = NULL;
		}else{
			jint ext_len = (*env)->CallIntMethod(env,obj_ext,arraylist_size);

			dev_types[i].ext_num = ext_len;
			dev_types[i].ext_type = (int *)malloc(ext_len * sizeof(int));
			for(j=0;j<ext_len;j++){  
				jobject obj_e_type = (*env)->CallObjectMethod(env,obj_ext,arraylist_get,j); 
				ext_type = (*env)->CallIntMethod(env,obj_e_type,int_value);
				dev_types[i].ext_type[j] = ext_type; 
			}
		}
	} 
	(*env)->DeleteLocalRef(env, cls_arraylist);
	(*env)->DeleteLocalRef(env, cls_integer);
}

bool filterSlave(cl_slave_t *slave)
{
	if (slave == NULL) {
		return true;
	}
	
	if (slave->dev_type == IJ_RFGW && slave->ext_type == RS_EXT_TYPE_YSD) {
		if (slave->obj.status != BMS_UNBIND) {
			return true;
		}
	}
	return false;
}

jboolean IsSupportDev(JNIEnv* env, jint sub_type,jint ext_type){
	//LOGE("find   dev_types_num = %d",dev_types_num);
	if(IS_COPY_NO_SUPPORT_DEV == true){
		LOGE("find   IS_COPY_NO_SUPPORT_DEV = %d",IS_COPY_NO_SUPPORT_DEV);
		return true;
	}

	//未知类型的，先放到java上层，由上层来过滤
	if (sub_type == 0xFF) {
		return true;
	}
	
	int i,j;
	if(dev_types == NULL || dev_types_num <=0){
		LOGE("find   dev_types = NULL");
		return false;
	}
	for(i = 0; i < dev_types_num; i++){
		//LOGE("find   i = %d, sub_type = %d",i,dev_types[i].sub_type);
		if(dev_types[i].sub_type == sub_type){//支持的设备类型
			if(dev_types[i].ext_num <= 0 || dev_types[i].ext_type == NULL || ext_type == 0xFF){//未定义扩展类型
				return true;
			}else{//逐个比较扩展类型
				for(j = 0;j < dev_types[i].ext_num && dev_types[i].ext_type != NULL; j++){
					//LOGE("find   j = %d, ext_type = %d",j,dev_types[i].ext_type[j] );
					if(dev_types[i].ext_type[j] == ext_type){
						return true;
					}
				}
			}
		}
	}
	return false;
}

static void CopyDevInfoBase(JNIEnv* env, jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	jfieldID fid;
	jstring str;
	int i, j , k;
	jclass class = NULL;
	jobject object_array = NULL;
	jobject object = NULL;

	fid = (*env)->GetFieldID(env, class_dev_info, "handle", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->handle);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "sn", "J");
	(*env)->SetLongField(env, obj_dev_info, fid, info->sn);
	if (info->nickname != NULL) {
		jniCopyString(env, class_dev_info, "nickname", obj_dev_info, info->nickname);
	}
	
	if (info->name != NULL) {
		jniCopyString(env, class_dev_info, "name", obj_dev_info, info->name);
	}

	if (info->passwd != NULL) {
		jniCopyString(env, class_dev_info, "password", obj_dev_info, info->passwd);
	}
	//拷贝开发者ID 给上层
	if(info->developer_id != NULL){
		jniCopyString(env, class_dev_info, "developer_id", obj_dev_info, info->developer_id);
	}
	fid = (*env)->GetFieldID(env, class_dev_info, "is_login", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->is_login);

	fid = (*env)->GetFieldID(env, class_dev_info, "is_online", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->is_online);

	fid = (*env)->GetFieldID(env, class_dev_info, "display_stat", "B");
	(*env)->SetByteField(env, obj_dev_info, fid, info->display_stat);

	fid = (*env)->GetFieldID(env, class_dev_info, "can_bind_phone", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->can_bind_phone);

	fid = (*env)->GetFieldID(env, class_dev_info, "login_type", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->login_type);

	fid = (*env)->GetFieldID(env, class_dev_info, "net_type", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->net_type);

	fid = (*env)->GetFieldID(env, class_dev_info, "last_err", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->last_err);

	fid = (*env)->GetFieldID(env, class_dev_info, "sub_type", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->sub_type);

	fid = (*env)->GetFieldID(env, class_dev_info, "ext_type", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->ext_type);

	fid = (*env)->GetFieldID(env, class_dev_info, "has_scene", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_scene);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_scene_timer", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_scene_timer);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_scene_linkage_alarm", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_scene_linkage_alarm);

	fid = (*env)->GetFieldID(env, class_dev_info, "has_area", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_area);

	fid = (*env)->GetFieldID(env, class_dev_info, "has_ir", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_ir);

	fid = (*env)->GetFieldID(env, class_dev_info, "has_rf", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_rf);

	fid = (*env)->GetFieldID(env, class_dev_info, "has_alarm", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_alarm);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_alarm_swich", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_alarm_swich);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_db_rf", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_db_rf);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_eq_gencode", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_eq_gencode);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_eq_add_by_json", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_eq_add_by_json);

	fid = (*env)->GetFieldID(env, class_dev_info, "dev_ver_is_valid", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->dev_ver_is_valid);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "dev_ver_is_too_low", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->dev_ver_is_too_low);

	fid = (*env)->GetFieldID(env, class_dev_info, "has_green_net", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_green_net);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "has_belter", "Z");
	(*env)->SetBooleanField(env, obj_dev_info, fid, info->has_belter);

	fid = (*env)->GetFieldID(env, class_dev_info, "num_objs", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->num_objs);

	fid = (*env)->GetFieldID(env, class_dev_info, "num_slave", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->num_slave);
	
	fid = (*env)->GetFieldID(env, class_dev_info, "idx_slave", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->idx_slave);

	fid = (*env)->GetFieldID(env, class_dev_info, "num_usb_video", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->num_usb_video);

	fid = (*env)->GetFieldID(env, class_dev_info, "idx_usb_video", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->idx_usb_video);
     
    fid = (*env)->GetFieldID(env, class_dev_info, "num_equipment", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->num_equipment); 
	
    fid = (*env)->GetFieldID(env, class_dev_info, "idx_equipment", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->idx_equipment);

    fid = (*env)->GetFieldID(env, class_dev_info, "num_alarm_conf_phone", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->num_alarm_conf_phone);
	if (info->vendor_id != NULL) {
		jniCopyString(env,class_dev_info,"vendor_id", obj_dev_info, info->vendor_id);	
	}
	if (info->vendor_url != NULL) {
		jniCopyString(env,class_dev_info,"vendor_url", obj_dev_info, info->vendor_url);
	}

    if (info->num_alarm_conf_phone > 0 && info->phone_list != NULL) {
		char* phone;
		
		class = (*env)->FindClass(env, "java/lang/String");
		object_array = (*env)->NewObjectArray(env, info->num_alarm_conf_phone, class, NULL);
        
		for (i = 0; i < info->num_alarm_conf_phone; i++) {
			phone = info->phone_list[i];
			
			doString(&phone);
			if(phone && phone[0] != '\0'){
				str = (*env)->NewStringUTF(env, phone);
				(*env)->SetObjectArrayElement(env, object_array, i, str);
				(*env)->DeleteLocalRef(env, str);
			}
		}
		
		// 锟斤拷锟介构锟斤拷锟斤拷希锟斤拷锟街?
		if( object_array != NULL){
			fid = (*env)->GetFieldID(env, class_dev_info, "phone_list", "[Ljava/lang/String;");
			(*env)->SetObjectField(env, obj_dev_info, fid, object_array);

			(*env)->DeleteLocalRef(env, object_array);
		}
		
		SAFE_DEL_LOCAL_REF(class);
		
		object_array = NULL;
	}
	if (info->num_area > 0 && info->areas != NULL ) {
		class = NULL;
		class = (*env)->FindClass(env, CLASS_AREA);
		object_array = (*env)->NewObjectArray(env, info->num_area, class, NULL);
		
        
        
        for (i = 0; i < info->num_area; i++) {
        		object = (*env)->AllocObject(env, class);

			fid = (*env)->GetFieldID(env, class, "area_handle", "I");
			(*env)->SetIntField(env, object, fid, info->areas[i]->area_handle);


			fid = (*env)->GetFieldID(env, class, "img_resv", "I");
			(*env)->SetIntField(env, object, fid, info->areas[i]->img_resv);
			
			fid = (*env)->GetFieldID(env, class, "item_count", "I");
			(*env)->SetIntField(env, object, fid, info->areas[i]->item_count);
			
			fid = (*env)->GetFieldID(env, class, "create_time", "I");
			(*env)->SetIntField(env, object, fid, info->areas[i]->create_time);
			
			jniCopyString(env, class, "area_name", object, info->areas[i]->area_name);

			if(info->areas[i]->item_count > 0){
				fid = (*env)->GetFieldID(env, class, "items", "[I");
				jintArray items = (*env)->NewIntArray(env, info->areas[i]->item_count);  
				(*env)->SetIntArrayRegion(env, items, 0, info->areas[i]->item_count, info->areas[i]->items);
				(*env)->SetObjectField(env, object, fid, items);
			}

			(*env)->SetObjectArrayElement(env, object_array, i, object);
			(*env)->DeleteLocalRef(env, object);
		}
		
		// 锟斤拷锟介构锟斤拷锟斤拷希锟斤拷锟街?
		fid = (*env)->GetFieldID(env, class_dev_info, "areas", "[L" CLASS_AREA ";");
		(*env)->SetObjectField(env, obj_dev_info, fid, object_array);

		(*env)->DeleteLocalRef(env, object_array);
		(*env)->DeleteLocalRef(env, class);
		class = NULL;
		object_array = NULL;
	}
	jclass class_scene_even;
	jobject even_array = NULL;
	jobject object_even;
	
	class_scene_even = (*env)->FindClass(env, CLASS_SCENE_EVEN);
	
	jclass class_scene_timer;
	jobject timer_array;
	jobject timer;
	
	class_scene_timer = (*env)->FindClass(env, CLASS_SCENE_TIMER);
	
	if (info-> num_scene > 0 && info->scenes != NULL) {
		class = (*env)->FindClass(env, CLASS_SCENE);
		object_array = (*env)->NewObjectArray(env, info->num_scene, class, NULL);
        
        
        for (i = 0; i < info->num_scene; i++) {
        		object = (*env)->AllocObject(env, class);

			fid = (*env)->GetFieldID(env, class, "scene_handle", "I");
			(*env)->SetIntField(env, object, fid, info->scenes[i]->scene_handle);
			
			fid = (*env)->GetFieldID(env, class, "img_resv", "I");
			(*env)->SetIntField(env, object, fid, info->scenes[i]->img_resv);
			
			fid = (*env)->GetFieldID(env, class, "event_count", "I");
			(*env)->SetIntField(env, object, fid, info->scenes[i]->event_count);
			
			fid = (*env)->GetFieldID(env, class, "last_executed", "Z");
			(*env)->SetBooleanField(env, object, fid, info->scenes[i]->last_executed);
			
			fid = (*env)->GetFieldID(env, class, "create_time", "I");
			(*env)->SetIntField(env, object, fid, info->scenes[i]->create_time);
			
			fid = (*env)->GetFieldID(env, class, "next_time", "I");
			(*env)->SetIntField(env, object, fid, info->scenes[i]->next_time);

			jniCopyString(env, class, "scene_name", object, info->scenes[i]->scene_name);
			
			//锟斤拷锟斤拷锟斤拷时锟斤拷
			int num_t = info->scenes[i]->timer_num;

			if( num_t >0 ){
				timer_array = (*env)->NewObjectArray(env, num_t , class_scene_timer, NULL);
			}else{
				timer_array = NULL;
			}
			
			for(k = 0; k < num_t; k++)
			{

				timer = (*env)->AllocObject(env, class_scene_timer);
				
				fid = (*env)->GetFieldID(env, class_scene_timer, "id", "I");
				(*env)->SetIntField(env, timer, fid, info->scenes[i]->timer[k]->id);

				fid = (*env)->GetFieldID(env, class_scene_timer, "hour", "I");
				(*env)->SetIntField(env, timer, fid, info->scenes[i]->timer[k]->hour);

				fid = (*env)->GetFieldID(env, class_scene_timer, "minute", "I");
				(*env)->SetIntField(env, timer, fid, info->scenes[i]->timer[k]->minute);

				fid = (*env)->GetFieldID(env, class_scene_timer, "week", "I");
				(*env)->SetIntField(env, timer, fid, info->scenes[i]->timer[k]->week);
				
				fid = (*env)->GetFieldID(env, class_scene_timer, "enable", "Z");
				(*env)->SetBooleanField(env, timer, fid, info->scenes[i]->timer[k]->enable);

				fid = (*env)->GetFieldID(env, class_scene_timer, "sort", "I");
				(*env)->SetIntField(env, timer, fid,info->scenes[i]->timer[k]->sort);
				
				jniCopyString(env, class_scene_timer, "name", timer, info->scenes[i]->timer[k]->name);
				
				(*env)->SetObjectArrayElement(env, timer_array, k, timer);	
				
				(*env)->DeleteLocalRef(env, timer);
			}


			if (timer_array != NULL){
				fid = (*env)->GetFieldID(env, class, "timer", "[L" CLASS_SCENE_TIMER ";");
				(*env)->SetObjectField(env, object, fid, timer_array);
				(*env)->DeleteLocalRef(env, timer_array);
			}

			if(info->scenes[i]->event_count > 0  ){
				even_array = (*env)->NewObjectArray(env, info->scenes[i]->event_count, class_scene_even, NULL);
			}
			
			for(j = 0; j < info->scenes[i]->event_count; j++)
			{
				object_even = (*env)->AllocObject(env, class_scene_even);
				
				fid = (*env)->GetFieldID(env, class_scene_even, "enent_type", "I");
				(*env)->SetIntField(env, object_even, fid, info->scenes[i]->events[j]->enent_type);
				
				fid = (*env)->GetFieldID(env, class_scene_even, "ev_type", "I");
				(*env)->SetIntField(env, object_even, fid, info->scenes[i]->events[j]->ev_type);
				
				fid = (*env)->GetFieldID(env, class_scene_even, "obj_handle", "I");
				(*env)->SetIntField(env, object_even, fid, info->scenes[i]->events[j]->obj_handle);
				
				jniCopyString(env, class_scene_even, "event_name", object_even, info->scenes[i]->events[j]->event_name);
				
				fid = (*env)->GetFieldID(env, class_scene_even, "action_num", "I");
				(*env)->SetIntField(env, object_even, fid, info->scenes[i]->events[j]->action_num);
				
				fid = (*env)->GetFieldID(env, class_scene_even, "ac_values", "[I");
				jintArray ac_values = (*env)->NewIntArray(env, info->scenes[i]->events[j]->action_num);  
				(*env)->SetIntArrayRegion(env, ac_values, 0, info->scenes[i]->events[j]->action_num, info->scenes[i]->events[j]->ac_values);
				(*env)->SetObjectField(env, object_even, fid, ac_values);
				(*env)->DeleteLocalRef(env, ac_values);
				
				(*env)->SetObjectArrayElement(env, even_array, j, object_even);

				(*env)->DeleteLocalRef(env, object_even);
			}

			if(even_array != NULL){
				fid = (*env)->GetFieldID(env, class, "events", "[L" CLASS_SCENE_EVEN ";");
				(*env)->SetObjectField(env, object, fid, even_array);
				(*env)->DeleteLocalRef(env, even_array);
				even_array = NULL;
			}
			
			(*env)->SetObjectArrayElement(env, object_array, i, object);
			(*env)->DeleteLocalRef(env, object);
			object = NULL;
		}
		
		// 锟斤拷锟介构锟斤拷锟斤拷希锟斤拷锟街?
		fid = (*env)->GetFieldID(env, class_dev_info, "scenes", "[L" CLASS_SCENE ";");
		(*env)->SetObjectField(env, obj_dev_info, fid, object_array);

		(*env)->DeleteLocalRef(env, object_array);
		(*env)->DeleteLocalRef(env, class);
		class = NULL;
		object_array = NULL;
	}
	
	CopyDevIaInformation(env,class_dev_info,obj_dev_info,info);
	CopyAirPlugInformation(env,class_dev_info,obj_dev_info,info);
	CopyDevLCAirHeaterInformation(env,class_dev_info,obj_dev_info,info);
	copyIntelligentRepeaterInformation(env,class_dev_info,obj_dev_info,info);
	CopyEPlugInformation(env,class_dev_info,obj_dev_info,info);
	CopyCommUdpInformation(env,class_dev_info,obj_dev_info,info);
	(*env)->DeleteLocalRef(env, class_scene_even);
	(*env)->DeleteLocalRef(env, class_scene_timer);

	SAFE_DEL_LOCAL_REF( class );
	
}

static void setSlaveNum(JNIEnv* env, jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info, jint slave_num)
{
	jfieldID fid;

	fid = (*env)->GetFieldID(env, class_dev_info, "num_objs", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, slave_num);

	fid = (*env)->GetFieldID(env, class_dev_info, "num_slave", "I");
	(*env)->SetIntField(env, obj_dev_info, fid, info->num_slave -(info->num_objs - slave_num));
	
}

static void CopyRFLightInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass class_rf_statu, jobject rfdev_obj) {
	jclass class_lamp;
	jclass class_rmt_ctrl;

	jobject jobj_lamp;
	jobject jobj_rmt_ctrl;
	jobject obj;
	jfieldID fid;
	int i;

	cl_rf_lamp_remote_info_t *rmt_ctrl_info;
	
	class_lamp = (*env)->FindClass(env, CLASS_RF_LIGHT_STATU);

	jobj_lamp = (*env)->AllocObject(env, class_lamp);

	rmt_ctrl_info = rfdev_p->dev_priv_data.lamp_info.r_info;

	jniCopyIntValue(env, class_lamp, "r", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.R);
	jniCopyIntValue(env, class_lamp, "g", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.G);
	jniCopyIntValue(env, class_lamp, "b", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.B);
	jniCopyIntValue(env, class_lamp, "l", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.L);
	jniCopyIntValue(env, class_lamp, "cold", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.cold);
	jniCopyBooleanValue(env, class_lamp, "power", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.power);
	
	jniCopyIntValue(env, class_lamp, "modeId", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.mod_id);
	jniCopyIntValue(env, class_lamp, "action", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.action);
	
//-----------------------------------------
	jniCopyBooleanValue(env, class_lamp, "is_support_color_temp", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.is_support_color_temp);
	jniCopyBooleanValue(env, class_lamp, "is_support_rgb", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.is_support_rgb);
	jniCopyIntValue(env, class_lamp, "remote_count", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.remote_count);
	jniCopyIntValue(env, class_lamp, "lamp_type", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.lamp_type);

	jniCopyByteValue(env, class_lamp, "flag", jobj_lamp, rfdev_p->dev_priv_data.lamp_info.stat.flag);

	if (rfdev_p->dev_priv_data.lamp_info.remote_count > 0) {
		class_rmt_ctrl = (*env)->FindClass(env, CLASS_RF_LAMP_RMT_CTRL_INFO);
		jobj_rmt_ctrl = (*env)->NewObjectArray(env, rfdev_p->dev_priv_data.lamp_info.remote_count, class_rmt_ctrl, NULL);
		for (i = 0; i < rfdev_p->dev_priv_data.lamp_info.remote_count; i++, rmt_ctrl_info++) {
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

//-----------------------------------------

	fid = (*env)->GetFieldID(env, class_rf_statu, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, rfdev_obj, fid, jobj_lamp);

	(*env)->DeleteLocalRef(env, class_lamp);
	(*env)->DeleteLocalRef(env, jobj_lamp);
}

static void CopyRFHistoryInfo(JNIEnv* env, jclass clazz, jobject obj, cl_rf_door_history_t*history, int num) {

	jclass histroy_clazz;
	jobjectArray history_arr;
	jobject histroy_obj;
	jfieldID fid;
	int i;

	histroy_clazz = (*env)->FindClass(env, CLASS_RF_DOOR_HISTORY);
	history_arr = (*env)->NewObjectArray(env, num, histroy_clazz, NULL); 

	for (i = 0; i < num; i++,history++) {
		histroy_obj = (*env)->AllocObject(env, histroy_clazz);
		jniCopyBooleanValue(env, histroy_clazz, "value", histroy_obj, history->value);
		jniCopyByteValue(env, histroy_clazz, "info_type", histroy_obj, history->info_type);
		jniCopyBooleanValue(env, histroy_clazz, "is_valid", histroy_obj, history->is_valid);
		jniCopyIntValue(env, histroy_clazz, "time_stamp", histroy_obj, history->time_stamp);
		jniCopyByteValue(env, histroy_clazz, "ex_value", histroy_obj, history->ex_value);
		jniCopyByteValue(env, histroy_clazz, "ex_type", histroy_obj, history->ex_type);

		(*env)->SetObjectArrayElement(env, history_arr, i, histroy_obj);
		(*env)->DeleteLocalRef(env, histroy_obj);
	}

	fid = (*env)->GetFieldID(env, clazz, "his", "[L" CLASS_RF_DOOR_HISTORY";");
	(*env)->SetObjectField(env, obj, fid, history_arr);

	(*env)->DeleteLocalRef(env, histroy_clazz);
	(*env)->DeleteLocalRef(env, history_arr);
}

static void CopyRFSwitchName(JNIEnv* env, jclass clazz, jobject obj, cl_dhx_key_name_item_t *names, int num) {

	jclass rfname_clazz;
	jobjectArray rfname_arr;
	jobject rfname_obj;
	jfieldID fid;
	int i;

	rfname_clazz = (*env)->FindClass(env, CLASS_RF_SWITCH_NAME);
	rfname_arr = (*env)->NewObjectArray(env, num, rfname_clazz, NULL); 

	for (i = 0; i < num; i++,names++) {
		rfname_obj = (*env)->AllocObject(env, rfname_clazz);
		jniCopyByteValue(env, rfname_clazz, "valid", rfname_obj, names->valid);
		jniCopyString(env, rfname_clazz, "name", rfname_obj, names->name);

		(*env)->SetObjectArrayElement(env, rfname_arr, i, rfname_obj);
		(*env)->DeleteLocalRef(env, rfname_obj);
	}

	fid = (*env)->GetFieldID(env, clazz, "keys", "[L" CLASS_RF_SWITCH_NAME";");
	(*env)->SetObjectField(env, obj, fid, rfname_arr);

	(*env)->DeleteLocalRef(env, rfname_clazz);
	(*env)->DeleteLocalRef(env, rfname_arr);
}


static void CopyRFDoorLockInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass class_rf_statu, jobject rfdev_obj) {
	jclass clazz;
	jobject jobj;
	jfieldID fid;

	clazz = (*env)->FindClass(env, CLASS_RF_DOORLOCK_INFO);
	jobj = (*env)->AllocObject(env, clazz);
		
	jclass class_state = (*env)->FindClass(env, CLASS_RF_DOORLOCK_STATE);
	jobject jobj_state = (*env)->AllocObject(env, class_state);

	jni_copy_simple_class(env, class_state, jobj_state,
							TRIPLES(int, &(rfdev_p->dev_priv_data.door_lock_info.stat), battary),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), is_door_open),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), is_look_open),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), is_battary_warn),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), has_break_door),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), is_guard),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), is_break_lock),	
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), has_open_passwd),	
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), has_limit_fault),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), has_moto_fault),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), has_unlock_timeout),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_lock_info.stat), unlock_timeout_enable),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_lock_info.stat), unlock_timeout),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_lock_info.stat), last_controller_id),
							JNI_VAR_ARG_END);
		
	fid = (*env)->GetFieldID(env, clazz, "stat", "L" CLASS_RF_DOORLOCK_STATE";");
	(*env)->SetObjectField(env, jobj, fid, jobj_state);

	class_state = (*env)->FindClass(env, CLASS_RF_DOOR_ALARM_INFO);
	jobj_state = (*env)->AllocObject(env, class_state);
		
	jni_copy_simple_class(env, class_state, jobj_state,
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_lock_info.alarm_info), value),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_lock_info.alarm_info), info_type),
							TRIPLES(int, &(rfdev_p->dev_priv_data.door_lock_info.alarm_info), time_stamp),					
							JNI_VAR_ARG_END);
	

	fid = (*env)->GetFieldID(env, clazz, "alarm_info", "L" CLASS_RF_DOOR_ALARM_INFO";");
	(*env)->SetObjectField(env, jobj, fid, jobj_state);

	JNI_COPY_SIMPLE_CLASS(env, clazz, jobj, CLASS_RF_DOOR_WIFI_LOCK, wifilock,
		                     TRIPLES(boolean,  &(rfdev_p->dev_priv_data.door_lock_info.wifilock), don_enable),
		                     TRIPLES(boolean,  &(rfdev_p->dev_priv_data.door_lock_info.wifilock), doff_enable),
		                     TRIPLES(byte,  &(rfdev_p->dev_priv_data.door_lock_info.wifilock), don_starthour),
		                     TRIPLES(byte,  &(rfdev_p->dev_priv_data.door_lock_info.wifilock), don_endhour),
		                     TRIPLES(byte,  &(rfdev_p->dev_priv_data.door_lock_info.wifilock), doff_starthour),
		                     TRIPLES(byte,  &(rfdev_p->dev_priv_data.door_lock_info.wifilock), doff_endhour),
		                     JNI_VAR_ARG_END);

	/*JNI_COPY_ARRAY_CLASS(env, clazz, jobj, CLASS_RF_DOOR_HISTORY,
			                   his,200, sizeof(cl_rf_door_history_t),
			                   ARRAY_TRIPLES(boolean, rfdev_p->dev_priv_data.door_lock_info.his, value),
			                   ARRAY_TRIPLES(boolean, rfdev_p->dev_priv_data.door_lock_info.his, info_type),
			                   ARRAY_TRIPLES(boolean, rfdev_p->dev_priv_data.door_lock_info.his, is_valid),
			                   ARRAY_TRIPLES(int, rfdev_p->dev_priv_data.door_lock_info.his, time_stamp),
			                   JNI_VAR_ARG_END);*/

	CopyRFHistoryInfo(env,clazz,jobj,rfdev_p->dev_priv_data.door_lock_info.his,200);			                   

	if (rfdev_p->dev_priv_data.door_lock_info.controller_num > 0) {
		JNI_COPY_ARRAY_CLASS(env, clazz, jobj, CLASS_RF_DOOR_CONTROLLER, controller,
			                    rfdev_p->dev_priv_data.door_lock_info.controller_num, sizeof(cl_door_lock_controller_info_t),
			                    ARRAY_TRIPLES(byte , rfdev_p->dev_priv_data.door_lock_info.controller, id),
			                    ARRAY_TRIPLES(byte , rfdev_p->dev_priv_data.door_lock_info.controller, state),
			                    ARRAY_TRIPLES(String , rfdev_p->dev_priv_data.door_lock_info.controller, name),
			                    JNI_VAR_ARG_END);
	}


	fid = (*env)->GetFieldID(env, class_rf_statu, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, rfdev_obj, fid, jobj);

	(*env)->DeleteLocalRef(env, class_state);
	(*env)->DeleteLocalRef(env, jobj_state);

	(*env)->DeleteLocalRef(env, clazz);
	(*env)->DeleteLocalRef(env, jobj);	
}


static void CopyRFDoorMagnetInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass class_rf_statu, jobject rfdev_obj) {
	jclass clazz;
	jobject jobj;
	jfieldID fid;

	clazz = (*env)->FindClass(env, CLASS_RF_DOORMAGNET_INFO);
	jobj = (*env)->AllocObject(env, clazz);
		
	jclass class_state = (*env)->FindClass(env, CLASS_RF_DOORMAGNET_STATE);
	jobject jobj_state = (*env)->AllocObject(env, class_state);
		
	jni_copy_simple_class(env, class_state, jobj_state,
							TRIPLES(int, &(rfdev_p->dev_priv_data.door_magnet_info.stat), battary),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.stat), is_door_open),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.stat), is_battary_warn),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.stat), is_break_door),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.stat), is_guard),
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.stat), is_support_new_history),
							JNI_VAR_ARG_END);		

	class_state = (*env)->FindClass(env, CLASS_RF_DOOR_ALARM_INFO);
	jobject jobj_alarm = (*env)->AllocObject(env, class_state);
/*
	LOGD("xxxddd alarm info: value = %hhu, type = %hhu, time_stamp = %d\n", 
		rfdev_p->dev_priv_data.door_magnet_info.alarm_info.value,
		rfdev_p->dev_priv_data.door_magnet_info.alarm_info.info_type,
		rfdev_p->dev_priv_data.door_magnet_info.alarm_info.time_stamp);
*/		
	jni_copy_simple_class(env, class_state, jobj_alarm,
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.alarm_info), value),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.alarm_info), info_type),
							TRIPLES(int, &(rfdev_p->dev_priv_data.door_magnet_info.alarm_info), time_stamp),					
							JNI_VAR_ARG_END);

	class_state = (*env)->FindClass(env, CLASS_RF_AUTO_GUARD_INFO);
	jobject jobj_guard_on = (*env)->AllocObject(env, class_state);
		
	jni_copy_simple_class(env, class_state, jobj_guard_on,
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.auto_on), enable),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.auto_on), start_hour),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.auto_on), end_hour),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.auto_on), type),
							JNI_VAR_ARG_END);

	jobject jobj_guard_off = (*env)->AllocObject(env, class_state);
		
	jni_copy_simple_class(env, class_state, jobj_guard_off,
							TRIPLES(boolean, &(rfdev_p->dev_priv_data.door_magnet_info.auto_off), enable),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.auto_off), start_hour),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.auto_off), end_hour),
							TRIPLES(byte, &(rfdev_p->dev_priv_data.door_magnet_info.auto_off), type),
							JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, clazz, "stat", "L" CLASS_RF_DOORMAGNET_STATE";");
	(*env)->SetObjectField(env, jobj, fid, jobj_state);	
	
	fid = (*env)->GetFieldID(env, clazz, "alarm_info", "L" CLASS_RF_DOOR_ALARM_INFO";");
	(*env)->SetObjectField(env, jobj, fid, jobj_alarm);

	fid = (*env)->GetFieldID(env, clazz, "auto_on", "L" CLASS_RF_AUTO_GUARD_INFO";");
	(*env)->SetObjectField(env, jobj, fid, jobj_guard_on);
	
	fid = (*env)->GetFieldID(env, clazz, "auto_off", "L" CLASS_RF_AUTO_GUARD_INFO";");
	(*env)->SetObjectField(env, jobj, fid, jobj_guard_off);

	CopyRFHistoryInfo(env,clazz,jobj,rfdev_p->dev_priv_data.door_magnet_info.his,200);

	/*JNI_COPY_ARRAY_CLASS(env, clazz, jobj, CLASS_RF_DOOR_HISTORY,
			                    his,200, sizeof(cl_rf_door_history_t),
			                    ARRAY_TRIPLES(boolean, rfdev_p->dev_priv_data.door_magnet_info.his, value),
			                    ARRAY_TRIPLES(boolean, rfdev_p->dev_priv_data.door_lock_info.his, info_type ),
			                    ARRAY_TRIPLES(boolean, rfdev_p->dev_priv_data.door_magnet_info.his, is_valid),
			                    ARRAY_TRIPLES(int, rfdev_p->dev_priv_data.door_magnet_info.his, time_stamp),
			                    JNI_VAR_ARG_END);
	*/
	
	fid = (*env)->GetFieldID(env, class_rf_statu, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, rfdev_obj, fid, jobj);

	(*env)->DeleteLocalRef(env, class_state);
	(*env)->DeleteLocalRef(env, jobj_state);
	(*env)->DeleteLocalRef(env, jobj_alarm);
	(*env)->DeleteLocalRef(env, jobj_guard_on);
	(*env)->DeleteLocalRef(env, jobj_guard_off);

	(*env)->DeleteLocalRef(env, clazz);
	(*env)->DeleteLocalRef(env, jobj);
}

/*
  拷贝单火线的数据
*/
static void CopyRFDHXSwitchInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass class_rf_statu, jobject rfdev_obj) {
	jclass clazz;
	jobject jobj;
	jfieldID fid;

	clazz = (*env)->FindClass(env, CLASS_RF_SWITCH_STAT);
	jobj = (*env)->AllocObject(env, clazz);

	jniCopyByteValue(env, clazz, "group_num", jobj, rfdev_p->dev_priv_data.dhx_info.stat.group_num);
	jniCopyIntValue(env, clazz, "on_off_stat", jobj, rfdev_p->dev_priv_data.dhx_info.stat.on_off_stat);
	jniCopyBooleanValue(env,clazz,"support_time", jobj,rfdev_p->dev_priv_data.dhx_info.support_time);
	jniCopyBooleanValue(env,clazz,"support_name_set",jobj,rfdev_p->dev_priv_data.dhx_info.support_name_set);
	
	CopyRFSwitchName(env,clazz,jobj,rfdev_p->dev_priv_data.dhx_info.keys,DHX_MAX_NUM);	
	fid = (*env)->GetFieldID(env, class_rf_statu, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, rfdev_obj, fid, jobj);

	(*env)->DeleteLocalRef(env, clazz);
	(*env)->DeleteLocalRef(env, jobj);
}

static void copyHmBodyInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_body;
	jobject obj_body;

	class_body = (*env)->FindClass(env, CLASS_HM_BODY);
	obj_body = (*env)->AllocObject(env, class_body);

	jni_copy_simple_class(env, class_body, obj_body,
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info, battary),
		                     TRIPLES(boolean, &rfdev_p->dev_priv_data.hb_info, is_low_battary_warn),
		                     TRIPLES(boolean, &rfdev_p->dev_priv_data.hb_info, is_guard),
		                     TRIPLES(boolean, &rfdev_p->dev_priv_data.hb_info, is_break),
		                     TRIPLES(int, &rfdev_p->dev_priv_data.hb_info, detectd_num),
		                     TRIPLES(int, &rfdev_p->dev_priv_data.hb_info, last_guard_time),
		                     TRIPLES(boolean, &rfdev_p->dev_priv_data.hb_info, is_support_new_history),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info, hw_ver),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info, soft_ver_mar),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info, soft_ver_min),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info, soft_ver_rev),
		                     JNI_VAR_ARG_END);
	CopyRFHistoryInfo(env, class_body, obj_body, rfdev_p->dev_priv_data.hb_info.his, 200);
	JNI_COPY_SIMPLE_CLASS(env, class_body, obj_body, CLASS_RF_DOOR_ALARM_INFO, alarm_info,
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info.alarm_info, value),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.hb_info.alarm_info, info_type),
		                     TRIPLES(int, &rfdev_p->dev_priv_data.hb_info.alarm_info, time_stamp),
		                     JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env, class_body, obj_body, CLASS_COMM_DETECTOR_ALARM_TIME, alarm_time,
		                     QUADRUPLE(int[], &rfdev_p->dev_priv_data.hb_info.alarm_time, time, 4),
		                     JNI_VAR_ARG_END);
	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_body);

	(*env)->DeleteLocalRef(env, class_body);
	(*env)->DeleteLocalRef(env, obj_body);
}

static void copyHmTempHumInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_th;
	jobject obj_th;

	class_th = (*env)->FindClass(env, CLASS_HM_TEMP_HUM);
	obj_th = (*env)->AllocObject(env, class_th);

	jni_copy_simple_class(env, class_th, obj_th,
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.ht_info, cur_temp),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.ht_info, cur_hum),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.ht_info, battary),
		                     TRIPLES(boolean, &rfdev_p->dev_priv_data.ht_info, is_low_battary_warn),
							 TRIPLES(boolean, &rfdev_p->dev_priv_data.ht_info, is_history_data_valid),
		                     TRIPLES(byte, &rfdev_p->dev_priv_data.ht_info, history_hour),
		                     JNI_VAR_ARG_END);
	if (rfdev_p->dev_priv_data.ht_info.support_temp100) {
		jniCopyBooleanValue(env, class_th, "support_temp100", obj_th, rfdev_p->dev_priv_data.ht_info.support_temp100);
		jniCopyIntValue(env, class_th, "cur_temp100", obj_th, rfdev_p->dev_priv_data.ht_info.cur_temp100);
	}

	JNI_COPY_ARRAY_CLASS(env, class_th, obj_th, CLASS_HM_HISTORY, 
		                    his_data, MAX_HM_HISTORY_NUM, sizeof(cl_hm_history_info_t),
		                    ARRAY_TRIPLES(byte, rfdev_p->dev_priv_data.ht_info.his_data, temp),
		                    ARRAY_TRIPLES(byte, rfdev_p->dev_priv_data.ht_info.his_data, hum),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_th);

	(*env)->DeleteLocalRef(env, class_th);
	(*env)->DeleteLocalRef(env, obj_th);
}

static void copyHeatingValveInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_hv;
	jobject obj_hv;
	cl_heating_valve_info_t *hv_info = &rfdev_p->dev_priv_data.hv_info;

	class_hv = (*env)->FindClass(env, CLASS_HEATING_VALVE_INFO);
	obj_hv = (*env)->AllocObject(env, class_hv);

	JNI_COPY_SIMPLE_CLASS(env, class_hv, obj_hv, CLASS_HEATING_VALVE_STAT, stat,
		                     TRIPLES(boolean, &hv_info->stat,onoff),
		                     TRIPLES(boolean, &hv_info->stat,mode),
		                     TRIPLES(boolean, &hv_info->stat,window),
		                     TRIPLES(boolean, &hv_info->stat,against),
		                     TRIPLES(boolean, &hv_info->stat,frost),
		                     TRIPLES(boolean, &hv_info->stat,child_proof),
		                     TRIPLES(byte, &hv_info->stat,summer_winter),
		                     TRIPLES(byte, &hv_info->stat,battery),
		                     TRIPLES(short, &hv_info->stat,current_temp),
		                     TRIPLES(short, &hv_info->stat,manual_temp),
		                     TRIPLES(short, &hv_info->stat,heat_temp),
		                     TRIPLES(short, &hv_info->stat,economy_temp),
		                     TRIPLES(short, &hv_info->stat,year),
		                     TRIPLES(byte, &hv_info->stat,month),
		                     TRIPLES(byte, &hv_info->stat,day),
		                     TRIPLES(byte, &hv_info->stat,hour),
		                     TRIPLES(byte, &hv_info->stat,minute),
		                     TRIPLES(byte, &hv_info->stat,error),
		                     TRIPLES(boolean, &hv_info->stat,windowfun),
		                     TRIPLES(boolean, &hv_info->stat,windowopen),
		                     JNI_VAR_ARG_END);
	
	JNI_COPY_ARRAY_CLASS(env, class_hv, obj_hv, CLASS_HEATING_VALVE_DAY, day_period,
		                   7, sizeof(cl_heating_valve_day_period_t),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, hh1),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, hm1),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, eh1),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, em1),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, hh2),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, hm2),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, eh2),
		                   ARRAY_TRIPLES(byte, hv_info->day_period, em2),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_hv);

	SAFE_DEL_LOCAL_REF(class_hv);
	SAFE_DEL_LOCAL_REF(obj_hv);
}

static void copyKateRfPlug(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_kate = NULL;
	jobject obj_kate = NULL;
	cl_ktcz_info_t *kate_info = &rfdev_p->dev_priv_data.kt_info;

	class_kate = (*env)->FindClass(env, CLASS_KATE_RF_EPLUG_INFO);
	obj_kate = (*env)->AllocObject(env, class_kate);


	JNI_COPY_SIMPLE_CLASS(env, class_kate, obj_kate, CLASS_KATE_RF_EPLUG_STAT, stat,
		                     TRIPLES(boolean, &kate_info->stat,onoff),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_kate);

	SAFE_DEL_LOCAL_REF(class_kate);
	SAFE_DEL_LOCAL_REF(obj_kate);
}

static void copyHtlLock(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_lock = NULL;
	jobject obj_lock = NULL;
	cl_htllock_info_t *lock = &rfdev_p->dev_priv_data.hl_info;

	class_lock = (*env)->FindClass(env, CLASS_HTL_LOCK_INFO);
	obj_lock = (*env)->AllocObject(env, class_lock);

	JNI_COPY_SIMPLE_CLASS(env, class_lock, obj_lock, CLASS_HTL_LOCK_STAT, lock_stat,
		                     TRIPLES(boolean, &lock->lock_stat, onoff),
		                     TRIPLES(byte, &lock->lock_stat, vol),
		                     TRIPLES(byte, &lock->lock_stat, lang),
		                     TRIPLES(byte, &lock->lock_stat, battery),
		                     JNI_VAR_ARG_END);
	JNI_COPY_ARRAY_CLASS(env, class_lock, obj_lock, CLASS_HTL_LOCK_USER_MANAGE, user_manage,
		                    HTLLOCK_MAX_USER_NUM, sizeof(htllock_user_manage_stat_t),
		                    ARRAY_TRIPLES(boolean, lock->user_manage, is_close_stat_reminder),
		                    ARRAY_TRIPLES(short, lock->user_manage, index),
		                    ARRAY_TRIPLES(short, lock->user_manage, pindex),
		                    ARRAY_TRIPLES(short, lock->user_manage, create_id),
		                    ARRAY_TRIPLES(byte, lock->user_manage, pic_id),
		                    ARRAY_TRIPLES(String, lock->user_manage, name),
		                    JNI_VAR_ARG_END);
	//处理有效时间小于1分钟的情况，将次数大于0，时间等于0的值为密码有效且有效时间为1分钟内。
	if(lock->last_pin.cnt > 0 && lock->last_pin.time == 0){
		lock->last_pin.time = 1;
		LOGI("zzzz hutlon last_pin valid, cnt = %d,time=%d,pwd = %s \n",lock->last_pin.cnt,lock->last_pin.time,lock->last_pin.pwd);
	}
	//设置最后的临时密码
	JNI_COPY_SIMPLE_CLASS(env, class_lock, obj_lock, CLASS_HTL_LOCK_SET_PIN, last_pin,
								 TRIPLES(short, &lock->last_pin, time),
								 TRIPLES(byte, &lock->last_pin, cnt),
								 TRIPLES(String, &lock->last_pin, pwd),
								 JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_lock, obj_lock, CLASS_HTL_LOCK_NOTICE, info_notice,
		                    HTLLOCK_MAX_NOTICE_TYPE_NUM, sizeof(htllock_info_notice_stat_t),
		                    ARRAY_TRIPLES(byte, lock->info_notice, type),
		                    ARRAY_TRIPLES(boolean, lock->info_notice, support_remind),
		                    ARRAY_TRIPLES(boolean, lock->info_notice, support_trouble_free),
		                    ARRAY_TRIPLES(boolean, lock->info_notice, support_msg_remind),
		                    ARRAY_TRIPLES(boolean, lock->info_notice, support_tel_remind),
		                    JNI_VAR_ARG_END);
	JNI_COPY_ARRAY_CLASS(env, class_lock, obj_lock, CLASS_HTL_LOCK_HISTORY, history,
		                    200, sizeof(httlock_history_t),
		                    ARRAY_TRIPLES(boolean, lock->history, is_valid),
		                    ARRAY_TRIPLES(byte, lock->history, value),
		                    ARRAY_TRIPLES(byte, lock->history, info_type),
		                    ARRAY_TRIPLES(int, lock->history, time_stamp),
		                    ARRAY_TRIPLES(short, lock->history, create_id),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_lock);

	SAFE_DEL_LOCAL_REF(class_lock);
	SAFE_DEL_LOCAL_REF(obj_lock);
}

static void copyRfWk(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_wk = NULL;
	jobject obj_wk = NULL;
	cl_wk_air_info_t *wk = &rfdev_p->dev_priv_data.wk_air_info;

	class_wk = (*env)->FindClass(env, CLASS_RF_WK_INFO);
	obj_wk = (*env)->AllocObject(env, class_wk);

	JNI_COPY_SIMPLE_CLASS(env, class_wk, obj_wk, CLASS_RF_WK_STAT, stat,
		                     TRIPLES(short, &wk->stat, ir_id),
		                     TRIPLES(byte, &wk->stat, battary),
		                     TRIPLES(byte, &wk->stat, charge),
		                     TRIPLES(int, &wk->stat, room_temp),
		                     TRIPLES(byte, &wk->stat, room_humi),
		                     TRIPLES(byte, &wk->stat, onoff),
		                     TRIPLES(byte, &wk->stat, mode),
		                     TRIPLES(byte, &wk->stat, temp),
		                     TRIPLES(byte, &wk->stat, wind),
		                     TRIPLES(byte, &wk->stat, wind_direct),
		                     TRIPLES(byte, &wk->stat, tmp_adjust),
		                     TRIPLES(byte, &wk->stat, led_mode),
		                     TRIPLES(byte, &wk->stat, addr),
		                     TRIPLES(boolean, &wk->stat, ir_sync_ctrl),
		                     TRIPLES(boolean, &wk->stat, support_ir_sync_ctrl),
		                     JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_wk, obj_wk, CLASS_RF_WK_PAIR, match_stat,
		                     TRIPLES(byte, &wk->match_stat, cur_step),
		                     TRIPLES(byte, &wk->match_stat, max_step),
		                     TRIPLES(byte, &wk->match_stat, error),
		                     TRIPLES(short, &wk->match_stat, matched_id),
		                     TRIPLES(short, &wk->match_stat, num),
		                     QUADRUPLE(short[], &wk->match_stat, matched_ids, 32),
		                     JNI_VAR_ARG_END);
		
	

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_wk);

	SAFE_DEL_LOCAL_REF(class_wk);
	SAFE_DEL_LOCAL_REF(obj_wk);
}

static void copyRfZhMotor(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_zh = NULL;
	jobject obj_zh = NULL;
	cl_zhdj_info_t *zh = &rfdev_p->dev_priv_data.zhdj_info;

	class_zh = (*env)->FindClass(env, CLASS_ZH_MOTOR_INFO);
	obj_zh = (*env)->AllocObject(env, class_zh);
	
	jni_copy_simple_class(env, class_zh, obj_zh,
		                    TRIPLES(int, zh, magic),
		                    TRIPLES(byte, zh, index),
		                    TRIPLES(byte, zh, status),
		                    TRIPLES(byte, zh, percent),
		                    TRIPLES(byte, zh, type),
		                    TRIPLES(byte, zh, support_dir),
		                    TRIPLES(byte, zh, dir),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_zh);

	SAFE_DEL_LOCAL_REF(class_zh);
	SAFE_DEL_LOCAL_REF(obj_zh);
}

static void copyDwkjInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_dw = NULL;
	jobject obj_dw = NULL;
	jclass class_dw_timer = NULL;
	jobject obj_dw_timer = NULL;
	cl_dwkj_info_t *dw = &rfdev_p->dev_priv_data.dwkj_info;

	class_dw = (*env)->FindClass(env, CLASS_DW_INFO);
	obj_dw = (*env)->AllocObject(env, class_dw);
	
	JNI_COPY_SIMPLE_CLASS(env, class_dw, obj_dw, CLASS_DW_STAT, stat,
		                    TRIPLES(boolean,&dw->stat, onoff),
		                    TRIPLES(short,&dw->stat, vol),
		                    TRIPLES(short,&dw->stat, current),
		                    TRIPLES(short,&dw->stat, power),
		                    TRIPLES(byte,&dw->stat, percent),
		                    TRIPLES(int,&dw->stat, degree),
		                    TRIPLES(byte,&dw->stat, error),
		                    JNI_VAR_ARG_END);

	class_dw_timer = (*env)->FindClass(env, CLASS_DW_TIMER);
	obj_dw_timer = (*env)->AllocObject(env, class_dw_timer);

	jniCopyBooleanValue(env, class_dw_timer, "onoff", obj_dw_timer, dw->timer.onoff);
	JNI_COPY_ARRAY_CLASS(env, class_dw_timer, obj_dw_timer, CLASS_DW_TIMER_ITEM, item,
		                   dw->timer.count, sizeof(cl_dwkj_timer_item_t),
		                   ARRAY_TRIPLES(short, dw->timer.item, point),
		                   ARRAY_TRIPLES(byte, dw->timer.item, level),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_dw, "timer", "L"CLASS_DW_TIMER";");
	(*env)->SetObjectField(env, obj_dw, fid, obj_dw_timer);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_dw);

	SAFE_DEL_LOCAL_REF(class_dw_timer);
	SAFE_DEL_LOCAL_REF(obj_dw_timer);
	SAFE_DEL_LOCAL_REF(class_dw);
	SAFE_DEL_LOCAL_REF(obj_dw);
}

static void copyScenePanel(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_panel = NULL;
	jobject obj_panel = NULL;
	cl_scene_controller_info_t *panel = &rfdev_p->dev_priv_data.sc_info;

	class_panel = (*env)->FindClass(env, CLASS_SCENE_PANEL);
	obj_panel = (*env)->AllocObject(env, class_panel);
	
	jni_copy_simple_class(env, class_panel, obj_panel,
		                    TRIPLES(boolean, panel, is_low_battery),
		                    TRIPLES(byte, panel, abc_battery),
		                    TRIPLES(byte, panel, hw_ver),
		                    TRIPLES(byte, panel, soft_ver_mar),
		                    TRIPLES(byte, panel, soft_ver_min),
		                    TRIPLES(byte, panel, soft_ver_rev),
		                    TRIPLES(boolean, panel, is_loss),
		                    TRIPLES(int, panel, svn),
		                    JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_panel, obj_panel, CLASS_SCENE_PANEL_KEY, keys,
		                    4, sizeof(scene_controller_key_item_t),
		                    ARRAY_TRIPLES(boolean, panel->keys, valid),
		                    ARRAY_TRIPLES(String, panel->keys, name),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_panel);

	SAFE_DEL_LOCAL_REF(class_panel);
	SAFE_DEL_LOCAL_REF(obj_panel);
}

static void copyCpScenePanel(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_panel = NULL;
	jobject obj_panel = NULL;
	cl_cdqjmb_info_t *panel = &rfdev_p->dev_priv_data.cdqjmb_info;

	class_panel = (*env)->FindClass(env, CLASS_CP_SCENE_PANEL);
	obj_panel = (*env)->AllocObject(env, class_panel);
	
	jni_copy_simple_class(env, class_panel, obj_panel,
		                    TRIPLES(int, panel, flag),
		                    TRIPLES(byte, panel, key_num),
		                    TRIPLES(byte, panel, ice_rule_maxnum),
		                    TRIPLES(byte, panel, ice_rule_curnum),
		                    JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_panel, obj_panel, CLASS_CP_SCENE_PANEL_KEY, key_conf,
		                    CDQJMB_KEY_MAXNUM, sizeof(cl_cdqjmb_key_item_t),
		                    ARRAY_TRIPLES(String, panel->key_conf, name),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_panel);

	SAFE_DEL_LOCAL_REF(class_panel);
	SAFE_DEL_LOCAL_REF(obj_panel);
}


static void copyRfJQInfo(JNIEnv* env,cl_rfdev_status_t *rfdev_p,jclass clazz, jobject slave_obj){
	jfieldID fid;
	jclass class_jq_info = NULL;
	jobject obj_jq_info = NULL;
	cl_jq_info_t *jq_info = &rfdev_p->dev_priv_data.jq_info;

	class_jq_info = (*env)->FindClass(env,CLASS_JQ_INFO);
	obj_jq_info = (*env)->AllocObject(env,class_jq_info);

	JNI_COPY_SIMPLE_CLASS(env,class_jq_info,obj_jq_info,CLASS_JQ_STAT,stat,
		TRIPLES(boolean,&jq_info->stat,valid),
		TRIPLES(short,&jq_info->stat,cur_ch2o),
		TRIPLES(byte,&jq_info->stat,battery),
		TRIPLES(byte,&jq_info->stat,std),
		TRIPLES(short,&jq_info->stat,period),
		TRIPLES(short,&jq_info->stat,thr_ch2o),
		JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_jq_info, obj_jq_info, CLASS_JQ_HIS_ITEM, items,
							   jq_info->n_item, sizeof(cl_jq_history_item_t),
							   ARRAY_TRIPLES(int, jq_info->items, time),
							   ARRAY_TRIPLES(short, jq_info->items, ch2o),
							   JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_jq_info);

	SAFE_DEL_LOCAL_REF(class_jq_info);
	SAFE_DEL_LOCAL_REF(obj_jq_info);
}

static void copyLightSenseInfo(JNIEnv* env,cl_rfdev_status_t *rfdev_p,jclass clazz, jobject slave_obj){
	jfieldID fid;
	jclass class_light_sense = NULL;
	jobject obj_light_sense = NULL;
	cl_light_sense_info_t *ls_info = &rfdev_p->dev_priv_data.ls;

	class_light_sense = (*env)->FindClass(env,CLASS_LIGHT_SENSE);
	obj_light_sense = (*env)->AllocObject(env,class_light_sense);

	JNI_COPY_SIMPLE_CLASS(env,class_light_sense,obj_light_sense,CLASS_LIGHT_SENSE_STATE,stat,
		TRIPLES(byte,&ls_info->stat,battery),
		TRIPLES(byte,&ls_info->stat,light_level),
		TRIPLES(int,&ls_info->stat,light_val),
		JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_light_sense);

	SAFE_DEL_LOCAL_REF(class_light_sense);
	SAFE_DEL_LOCAL_REF(obj_light_sense);
}

static void copyCommDetector(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj)
{
	jfieldID fid;
	jclass class_cd = NULL;
	jobject obj_cd = NULL;
	cl_com_detector_info_t *cd_info = &rfdev_p->dev_priv_data.cd_info;

	class_cd = (*env)->FindClass(env, CLASS_COMM_DETECTOR_INFO);
	obj_cd = (*env)->AllocObject(env, class_cd);

	JNI_COPY_SIMPLE_CLASS(env, class_cd, obj_cd, CLASS_COMM_DETECTOR_ALARM, alarm_info,
		                     TRIPLES(byte, &cd_info->alarm_info, value),
		                     TRIPLES(byte, &cd_info->alarm_info, info_type),
		                     TRIPLES(int, &cd_info->alarm_info, time_stamp),
		                     JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env, class_cd, obj_cd, CLASS_COMM_DETECTOR_STAT, stat,
		                     TRIPLES(byte, &cd_info->stat,hw_ver),
		                     TRIPLES(byte, &cd_info->stat,soft_ver_mar),
		                     TRIPLES(byte, &cd_info->stat,soft_ver_min),
		                     TRIPLES(byte, &cd_info->stat,soft_ver_rev),
		                     TRIPLES(byte, &cd_info->stat,abc_battery),
		                     TRIPLES(boolean, &cd_info->stat,is_alarm),
		                     TRIPLES(boolean, &cd_info->stat,is_low_battery),
		                     TRIPLES(boolean, &cd_info->stat,is_defence),
		                     TRIPLES(boolean, &cd_info->stat,is_pause_alarm),
		                     TRIPLES(boolean, &cd_info->stat,is_alarm_once),
		                     TRIPLES(boolean, &cd_info->stat,is_support_new_history),
		                     TRIPLES(int, &cd_info->stat,svn),
		                     JNI_VAR_ARG_END);
	
	JNI_COPY_ARRAY_CLASS(env, class_cd, obj_cd, CLASS_COMM_DETECTOR_HISTORY, his,
		                    200, sizeof(cl_com_detector_his_t),
		                    ARRAY_TRIPLES(byte, cd_info->his, value),
		                    ARRAY_TRIPLES(byte, cd_info->his, info_type),
		                    ARRAY_TRIPLES(boolean, cd_info->his, is_valid),
		                    ARRAY_TRIPLES(int, cd_info->his, time_stamp),
		                    JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env, class_cd, obj_cd, CLASS_COMM_DETECTOR_ALARM_TIME, alarm_time,
		                     QUADRUPLE(int[], &cd_info->alarm_time, time, 4),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_cd);

	SAFE_DEL_LOCAL_REF(class_cd);
	SAFE_DEL_LOCAL_REF(obj_cd);
}

static void copyRfCommHistory(JNIEnv* env, cl_rf_dev_comm_history_info_t *history, jclass class_stat, jobject obj_stat)
{
	jfieldID fid = NULL;
	jclass class_his = NULL;
	jobject obj_his = NULL;

	class_his = (*env)->FindClass(env, CLASS_RF_COMM_HISTORY);
	obj_his = (*env)->AllocObject(env, class_his);
	
	jni_copy_simple_class(env, class_his, obj_his,
		                     TRIPLES(int, history, index_current),
		                     TRIPLES(int, history, max_count),
		                     TRIPLES(short, history, index),
		                     JNI_VAR_ARG_END);

	JNI_COPY_ARRAY_CLASS(env, class_his, obj_his, CLASS_RF_COMM_HISTORY_ITEM, items,
		                   history->n, sizeof(cl_rf_dev_comm_history_item_t),
		                   ARRAY_TRIPLES(byte, history->items, type),
		                   ARRAY_TRIPLES(byte, history->items, value),
		                   ARRAY_TRIPLES(byte, history->items, ex_type),
		                   ARRAY_TRIPLES(byte, history->items, ex_value),
		                   ARRAY_TRIPLES(int, history->items, timestamp),
		                   ARRAY_TRIPLES(boolean, history->items, valid),
		                   JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_stat, "chi", "L"CLASS_RF_COMM_HISTORY";");
	(*env)->SetObjectField(env, obj_stat, fid, obj_his);

	SAFE_DEL_LOCAL_REF(class_his);
	SAFE_DEL_LOCAL_REF(obj_his);
}

static void CopyRFYlLightState(JNIEnv* env, cl_yllight_info_t *yllight_info, jclass class_light, jobject light_obj) {
	jclass class_lamp;
	jclass class_rmt_ctrl;

	jobject jobj_lamp;
	jobject jobj_rmt_ctrl;
	jobject obj;
	jfieldID fid;
	int i;

	cl_rf_lamp_remote_info_t *rmt_ctrl_info;
	
	class_lamp = (*env)->FindClass(env, CLASS_RF_LIGHT_STATU);

	jobj_lamp = (*env)->AllocObject(env, class_lamp);

	rmt_ctrl_info = yllight_info->lamp_stat.r_info;

	jniCopyIntValue(env, class_lamp, "r", jobj_lamp, yllight_info->lamp_stat.stat.R);
	jniCopyIntValue(env, class_lamp, "g", jobj_lamp, yllight_info->lamp_stat.stat.G);
	jniCopyIntValue(env, class_lamp, "b", jobj_lamp, yllight_info->lamp_stat.stat.B);
	jniCopyIntValue(env, class_lamp, "l", jobj_lamp, yllight_info->lamp_stat.stat.L);
	jniCopyIntValue(env, class_lamp, "cold", jobj_lamp, yllight_info->lamp_stat.stat.cold);
	jniCopyBooleanValue(env, class_lamp, "power", jobj_lamp, yllight_info->lamp_stat.stat.power);
	
	jniCopyIntValue(env, class_lamp, "modeId", jobj_lamp, yllight_info->lamp_stat.stat.mod_id);
	jniCopyIntValue(env, class_lamp, "action", jobj_lamp, yllight_info->lamp_stat.stat.action);
	jniCopyIntValue(env, class_lamp, "ctrl_mode", jobj_lamp, yllight_info->lamp_stat.stat.ctrl_mode);
	jniCopyIntValue(env, class_lamp, "value", jobj_lamp, yllight_info->lamp_stat.stat.value);
	
//-----------------------------------------
	jniCopyBooleanValue(env, class_lamp, "is_support_color_temp", jobj_lamp, yllight_info->lamp_stat.is_support_color_temp);
	jniCopyBooleanValue(env, class_lamp, "is_support_rgb", jobj_lamp, yllight_info->lamp_stat.is_support_rgb);
	jniCopyIntValue(env, class_lamp, "remote_count", jobj_lamp, yllight_info->lamp_stat.remote_count);
	jniCopyIntValue(env, class_lamp, "lamp_type", jobj_lamp, yllight_info->lamp_stat.lamp_type);

	if (yllight_info->lamp_stat.remote_count > 0) {
		class_rmt_ctrl = (*env)->FindClass(env, CLASS_RF_LAMP_RMT_CTRL_INFO);
		jobj_rmt_ctrl = (*env)->NewObjectArray(env, yllight_info->lamp_stat.remote_count, class_rmt_ctrl, NULL);
		for (i = 0; i < yllight_info->lamp_stat.remote_count; i++, rmt_ctrl_info++) {
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

//-----------------------------------------

	fid = (*env)->GetFieldID(env, class_light, "lamp_stat", "L" CLASS_RF_LIGHT_STATU";");
	(*env)->SetObjectField(env, light_obj, fid, jobj_lamp);

	(*env)->DeleteLocalRef(env, class_lamp);
	(*env)->DeleteLocalRef(env, jobj_lamp);
}

static void CopyRFYlLightInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj){
	jfieldID fid;
	jclass class_light_info = NULL;
	jobject obj_light_info = NULL;
	cl_yllight_info_t *light_info = &rfdev_p->dev_priv_data.yllight_info;

	class_light_info = (*env)->FindClass(env, CLASS_YL_LIGHT_INFO);
	obj_light_info = (*env)->AllocObject(env, class_light_info);

	CopyRFYlLightState(env,  light_info, class_light_info, obj_light_info);
	
	jni_copy_simple_class(env, class_light_info, obj_light_info,
		                    TRIPLES(byte, light_info, mode),
		                    TRIPLES(boolean, light_info, is_alarm),
		                    TRIPLES(boolean, light_info, is_dynamic),
		                    TRIPLES(byte, light_info, alarm_mode),
		                    TRIPLES(short, light_info, on_time),
		                    TRIPLES(int, light_info, off_time),
		                    TRIPLES(int, light_info, total_time),
		                    JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, clazz, "dev_priv_data", "Ljava/lang/Object;");
	(*env)->SetObjectField(env, slave_obj, fid, obj_light_info);

	SAFE_DEL_LOCAL_REF(class_light_info);
	SAFE_DEL_LOCAL_REF(obj_light_info);
}

static jobject CopyRFDevInfo(JNIEnv* env, cl_rfdev_status_t *rfdev_p, jclass clazz, jobject slave_obj) {
	jfieldID fid;
	jclass class_rf_statu;
	jobject rfdev_obj;

	class_rf_statu = (*env)->FindClass(env, CLASS_RF_DEV_STATU);
	rfdev_obj = (*env)->AllocObject(env, class_rf_statu);

	jni_copy_simple_class(env, class_rf_statu, rfdev_obj,
							TRIPLES(long, rfdev_p, ctrl_total),
							TRIPLES(int, rfdev_p, ctrl_min),
							TRIPLES(int, rfdev_p, ctrl_max),
							TRIPLES(int, rfdev_p, ctrl_ok),
							TRIPLES(int, rfdev_p, ctrl_fail),
							TRIPLES(int, rfdev_p, ctrl_msec),
							TRIPLES(int, rfdev_p, rfrx),
							TRIPLES(short, rfdev_p, linkretry),
							TRIPLES(short, rfdev_p, work_list_cnt),
							TRIPLES(byte, rfdev_p, work),
							TRIPLES(byte, rfdev_p, is_ctrl),						
							JNI_VAR_ARG_END);

	JNI_COPY_SIMPLE_CLASS(env,class_rf_statu, rfdev_obj, CLASS_RF_COMM_ALARM,cai,
		                     TRIPLES(int, &rfdev_p->cai, record_time),
		                     TRIPLES(short, &rfdev_p->cai, type),
		                     TRIPLES(int, &rfdev_p->cai, value),
		                     JNI_VAR_ARG_END);		

	if (rfdev_p->work_list_cnt > 0) {
		JNI_COPY_ARRAY_CLASS(env, class_rf_statu, rfdev_obj, CLASS_RF_DEV_WORK_T,
			                    work_list,rfdev_p->work_list_cnt, sizeof(cl_rfdev_work_t),
			                    ARRAY_TRIPLES(byte, rfdev_p->work_list,work),
			                    ARRAY_TRIPLES(int, rfdev_p->work_list,time),
			                    JNI_VAR_ARG_END);
	}
	jniCopyIntValue(env, class_rf_statu, "dev_type", rfdev_obj, rfdev_p->d_type);
	jniCopyByteValue(env,class_rf_statu,"dev_support",rfdev_obj,rfdev_p->dev_support);
	copyRfCommHistory(env, &rfdev_p->chi, class_rf_statu, rfdev_obj);
	//LOGE("CopyRFDevInfo   rfdev_p->d_type = %d\n", rfdev_p->d_type);

	switch(rfdev_p->d_type) {
	case D_T_LAMP:
	case D_T_DWHF:
	case D_T_DWYSTGQ:
	case D_T_LAMP_START:
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case D_T_LAMP_END:
		CopyRFLightInfo(env,  rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_DOOR_LOCK:
		CopyRFDoorLockInfo(env,  rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_DOOR_MAGNET:
	case D_T_HM_MAGNET:
	case D_T_DOOR_MAGNETV2:
	case D_T_YLLOCK:
		CopyRFDoorMagnetInfo(env,  rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_DHX_SWITCH:
	case D_T_DHXZH_SWITCH:
	case D_T_DHXCP_SWITCH:
	case D_T_MLDHX:
	case D_T_LHX:
		CopyRFDHXSwitchInfo(env,  rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_YT_DOOR_LOCK:
		CopyRFDoorLockInfo(env,  rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_HM_BODY_DETECT:
	case D_T_WUANS6:
	case D_T_YLTC:
		copyHmBodyInfo(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_HM_TEMP_HM:
	case D_T_YLWSD:
		copyHmTempHumInfo(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_HEATING_VALVE:
		copyHeatingValveInfo(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_KTCZ:
		copyKateRfPlug(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_GAS:
	case D_T_QSJC:
	case D_T_HMCO:
	case D_T_HMYW:
	case D_T_HMQJ:
	case D_T_YLSOS:
		copyCommDetector(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_HTLLOCK:
		copyHtlLock(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_WK_AIR2:
	case D_T_WK_AIR:
		copyRfWk(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_ZHDJ:
		copyRfZhMotor(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_DWKJ:
		copyDwkjInfo(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_SCENE_CONTROOLER:
		copyScenePanel(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_YLLIGHT:
		CopyRFYlLightInfo(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_CDQJMB:
		copyCpScenePanel(env, rfdev_p, class_rf_statu, rfdev_obj);
		break;
	case D_T_JQ:
		copyRfJQInfo(env,rfdev_p,class_rf_statu, rfdev_obj);
		break;
	case D_T_LIGHT_SENSE:
		copyLightSenseInfo(env,rfdev_p,class_rf_statu, rfdev_obj);
		break;
	default:
		break;
	}

	fid = (*env)->GetFieldID(env, clazz, "rfdev", "L"CLASS_RF_DEV_STATU";");
	(*env)->SetObjectField(env, slave_obj, fid, rfdev_obj);

	(*env)->DeleteLocalRef(env, class_rf_statu);
	(*env)->DeleteLocalRef(env, rfdev_obj);
}

static void copyExtTimerAdvance(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer) {
	jclass class_ext = NULL;
	jobject obj_ext = NULL;
	jfieldID fid = NULL;
	cl_comm_timer_zykt_t * zykt = &timer->extended_data_u.zykt_timer;

	class_ext = (*env)->FindClass(env, CLASS_COMM_TIMER_EXT_ZYKT);
	obj_ext = (*env)->AllocObject(env, class_ext);

	jni_copy_simple_class(env, class_ext, obj_ext,
		                     TRIPLES(byte, zykt, mode),
		                     TRIPLES(byte, zykt, tmp),
		                     JNI_VAR_ARG_END);

	fid = (*env)->GetFieldID(env, class_timer, "extInfo", "L"CLASS_JAVA_OBJECT";");
	(*env)->SetObjectField(env, obj_timer, fid, obj_ext);
	SAFE_DEL_LOCAL_REF(class_ext);
	SAFE_DEL_LOCAL_REF(obj_ext);
}

static void copyCommTimerExtInfo(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{
	switch(timer->type) {
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		copyExtTimerAdvance(env, class_timer, obj_timer, timer);
		break;
	default:
		break;
	}
}

static void copyCommTimers(JNIEnv *env, jclass class_timer_info, jobject obj_timer_info, cl_comm_timer_head_t *timer_info)
{
	jfieldID fid = NULL;
	jclass class_timer = NULL;
	jobject obj_timer = NULL;
	jobjectArray array_timer = NULL;
	int i = 0;

	if (timer_info->timer_count > 0) {
		class_timer = (*env)->FindClass(env, CLASS_RF_TIMER);
		array_timer = (*env)->NewObjectArray(env, timer_info->timer_count, class_timer, NULL);
		for (i = 0; i < timer_info->timer_count; ++i) {
			obj_timer = (*env)->AllocObject(env, class_timer);
			jni_copy_simple_class(env, class_timer, obj_timer,
				                     TRIPLES(byte, timer_info->timer + i, id),
				                     TRIPLES(boolean, timer_info->timer + i, enable),
				                     TRIPLES(byte, timer_info->timer + i, type),
				                     TRIPLES(byte, timer_info->timer + i, hour),
				                     TRIPLES(byte, timer_info->timer + i, min),
				                     TRIPLES(byte, timer_info->timer + i, week),
				                     TRIPLES(short, timer_info->timer + i, duration),
				                     JNI_VAR_ARG_END);
			copyCommTimerExtInfo(env, class_timer, obj_timer, timer_info->timer + i);
			(*env)->SetObjectArrayElement(env, array_timer, i, obj_timer);
			SAFE_DEL_LOCAL_REF(obj_timer);
		}
		
		fid = (*env)->GetFieldID(env, class_timer_info, "timer", "[L" CLASS_RF_TIMER ";");
		(*env)->SetObjectField(env, obj_timer_info, fid, array_timer);
		
		SAFE_DEL_LOCAL_REF(array_timer);
		SAFE_DEL_LOCAL_REF(class_timer);
	}
}

static void copyCommTimerExec(JNIEnv *env, jclass class_ti, jobject obj_ti, char *attrname, next_exec_time_t *exec_info)
{	
	jfieldID fid = NULL;
	jclass class_exec = NULL;
	jobject obj_exec = NULL;

	class_exec = (*env)->FindClass(env, CLASS_COMM_TIMER_EXEC);
	obj_exec = (*env)->AllocObject(env, class_exec);
	jni_copy_simple_class(env, class_exec, obj_exec,
		                     TRIPLES(boolean, exec_info, next_valid),
		                     TRIPLES(byte, exec_info, next_day),
		                     TRIPLES(byte, exec_info, next_hour),
		                     TRIPLES(byte, exec_info, next_min),
		                     JNI_VAR_ARG_END);
	
	fid = (*env)->GetFieldID(env, class_ti, attrname, "L" CLASS_COMM_TIMER_EXEC ";");
	(*env)->SetObjectField(env, obj_ti, fid, obj_exec);
	
	SAFE_DEL_LOCAL_REF(class_exec);
	SAFE_DEL_LOCAL_REF(obj_exec);                   
}

static int rfgw_type_map(int ext_type) {
	int rfgw_type = TDEV_MAX;
	switch(ext_type) {
	case RF_EXT_TYPE_HEATING_VALVE:
		rfgw_type = TDEV_HEATING_VALVE;
		break;
	case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_WK_AIR2:
		rfgw_type = TDEV_CENTRAL_RF_WUKONG;
		break;
	case RS_EXT_TYPE_ZHDJ:
		rfgw_type = TDEV_ZH_MOTOR;
		break;
	case RF_EXT_TYPE_DHXML:
	case RF_EXT_TYPE_DHX:
	case RF_EXT_TYPE_DHXZH:
	case RF_EXT_TYPE_DHXCP:
	case RF_EXT_TYPE_LHX:
		rfgw_type = TDEV_ML_DHXML;
		break;
	default:
		break;
	}
	return rfgw_type;
}

static int kxm_type_map(int ext_type) {
	int kxm_type = TDEV_MAX;
	switch(ext_type) {
	case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
		kxm_type = TDEV_CENTRAL_AIRCON;
		break;
	case ETYPE_IJ_LINKON_THERMOSTAT:
		kxm_type = TDEV_LK_THERMOSTAT;
		break;
	default:
		break;
	}
	return kxm_type;
}

static int zhcl_type_map(int ext_type){
	int zhcl_type = TDEV_MAX;
	switch(ext_type){
		case ETYPE_IJ_ZHCL:
			zhcl_type = TDEV_ZH_MOTOR;
			break;
		default:
			break;
	}
	return zhcl_type;
}

static int dev_type_map(int sub_type, int ext_type) {
	int dev_type = TDEV_MAX;
	switch(sub_type) {
	case IJ_RFGW:
		dev_type = rfgw_type_map(ext_type);
		break;
	case IJ_KXM_DEVICE:
		dev_type = kxm_type_map(ext_type);
		break;
	case IJ_ZHCL:
		dev_type = zhcl_type_map(ext_type);
		break;
	case IJ_ZHDHX:
		dev_type = TDEV_ZH_DHX;
	default:
		break;
	}
	return dev_type;
}
//要跟进设备类型和定时器类型，来判断定时器结构里面到底是什么数据
static int timer_type_map(int tdev_type, int timer_type) {
	int type = TIMER_TYPE_MAX;
	switch(tdev_type) {
	case TDEV_HEATING_VALVE:
		type = TIMER_TYPE_HEATING_VALVE;
		break;
	case TDEV_CENTRAL_AIRCON:
		type = TIMER_TYPE_CENTRAL_AIRCON;
		break;
	case TDEV_CENTRAL_RF_WUKONG:
		type = TIMER_TYPE_RF_WUKONG;
		break;
	case TDEV_ZH_MOTOR:
		type = TIMER_TYPE_ZH_MOTOR;
		break;
	case TDEV_LK_THERMOSTAT:
		type = TIMER_LK_THERMOSTAT;
		break;
	case TDEV_ML_DHXML:
		type = TIMER_ML_DHXML;
		break;
	case TDEV_ZH_DHX:
		type = TIMER_ZH_DHX;
		break;
	default:
		break;
	}
	return type;
}

static void copyCommTimerBase(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{	
	jni_copy_simple_class(env, class_timer, obj_timer,
		                    TRIPLES(byte, timer, id),
		                    TRIPLES(boolean, timer, enable),
		                    TRIPLES(byte, timer, type),
		                    TRIPLES(byte, timer, hour),
		                    TRIPLES(byte, timer, min),
		                    TRIPLES(byte, timer, week),
		                    TRIPLES(short, timer, duration),
		                    JNI_VAR_ARG_END);
}

static jobject copyCommTimerHeatingValve(JNIEnv *env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_HEATING_VALVE_TIMER);
	obj_timer = (*env)->AllocObject(env, class_timer);

	jni_copy_simple_class(env, class_timer, obj_timer,
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, tmp_int),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, tmp_dec),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, start_tmp_int),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, start_tmp_dec),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, end_tmp_int),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, end_tmp_dec),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, max_tmp_int),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, max_tmp_dec),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, min_tmp_int),
		                     TRIPLES(byte, &timer->extended_data_u.hv_timer, min_tmp_dec),
		                     JNI_VAR_ARG_END);

	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}
static jobject copyCommTimerRFWukongValve(JNIEnv *env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_RFWUKONG_VALVE_TIMER);
	obj_timer = (*env)->AllocObject(env, class_timer);

	jni_copy_simple_class(env, class_timer, obj_timer,
		                     TRIPLES(byte, &timer->extended_data_u.wkair_timer, mode),
		                     TRIPLES(byte, &timer->extended_data_u.wkair_timer, tmp),
		                     JNI_VAR_ARG_END);

	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}

static jobject copyCommTimerZhMotor(JNIEnv *env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_ZH_MOTOR_TIMER);
	obj_timer = (*env)->AllocObject(env, class_timer);

	jni_copy_simple_class(env, class_timer, obj_timer,
		                     TRIPLES(byte, &timer->extended_data_u.zhdj_timer, location),
		                     JNI_VAR_ARG_END);

	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}

static jobject copyCommTimerLkThermostat(JNIEnv *env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_LK_THERMOSTAT_TIMER);
	obj_timer = (*env)->AllocObject(env, class_timer);

	jni_copy_simple_class(env, class_timer, obj_timer,
		                     TRIPLES(byte, &timer->extended_data_u.linkon_timer, run_mode),
		                     TRIPLES(byte, &timer->extended_data_u.linkon_timer, wind_speed),
		                     TRIPLES(short, &timer->extended_data_u.linkon_timer, tmp),
		                     TRIPLES(byte, &timer->extended_data_u.linkon_timer, scene_mode),
		                     JNI_VAR_ARG_END);

	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}

static jobject copyCommTiemrMlDHL(JNIEnv *env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_ML_DHX);
	obj_timer = (*env)->AllocObject(env, class_timer);

	jni_copy_simple_class(env, class_timer, obj_timer,
		                     TRIPLES(int, &timer->extended_data_u.dhxml_timer, on_off_stat),
		                     JNI_VAR_ARG_END);

	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}

static jobject copyCommTiemrZhDHX(JNIEnv *env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_ZH_DHX);
	obj_timer = (*env)->AllocObject(env, class_timer);

	jni_copy_simple_class(env, class_timer, obj_timer,
		                     TRIPLES(byte, &timer->extended_data_u.zhdhx_timer, onoff),
		                     TRIPLES(byte, &timer->extended_data_u.zhdhx_timer, mask),
		                     JNI_VAR_ARG_END);

	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}

static jobject copyAllocCommTimerBase(JNIEnv * env, cl_comm_timer_t *timer)
{
	jclass class_timer = NULL;
	jobject obj_timer = NULL;

	class_timer = (*env)->FindClass(env, CLASS_RF_TIMER);
	obj_timer = (*env)->AllocObject(env, class_timer);
	copyCommTimerBase(env, class_timer, obj_timer, timer);
	SAFE_DEL_LOCAL_REF(class_timer);
	return obj_timer;
}

static void copyCommTimersExtends(JNIEnv *env, jclass class_timer_info, jobject obj_timer_info, 
	                                           int tdev_type, cl_comm_timer_head_t *timer_info)
{
	jobject obj_timer = NULL;
	jclass class_arraylist = NULL;
	jobject obj_arraylist = NULL;
	jmethodID arraylist_add = NULL;
	jmethodID arraylist_construct = NULL;
	jfieldID fid = NULL;
	int timer_type;
	int i = 0;

	if (timer_info->timer_count <= 0 || timer_info->timer == NULL) {
		return;
	}

	class_arraylist = (*env)->FindClass(env, CLASS_ARRAY_LIST);
	arraylist_construct = (*env)->GetMethodID(env, class_arraylist,"<init>","()V");
	obj_arraylist = (*env)->NewObject(env, class_arraylist, arraylist_construct, "");
	arraylist_add = (*env)->GetMethodID(env, class_arraylist, "add", "(L"CLASS_JAVA_OBJECT";)Z");

	for (i = 0; i < timer_info->timer_count; ++i) {
		if (timer_info->timer[i].id < 0) {
			continue;
		}
		timer_type = timer_type_map(tdev_type, timer_info->timer[i].type);
		
		switch(timer_type) {
		case TIMER_TYPE_HEATING_VALVE:
			obj_timer = copyCommTimerHeatingValve(env, timer_info->timer + i);
			break;
		case TIMER_TYPE_RF_WUKONG:
			obj_timer = copyCommTimerRFWukongValve(env, timer_info->timer + i);
			break;
		case TIMER_TYPE_ZH_MOTOR:
			obj_timer = copyCommTimerZhMotor(env, timer_info->timer + i);
			break;
		case TIMER_LK_THERMOSTAT:
			obj_timer = copyCommTimerLkThermostat(env, timer_info->timer +i);
			break;
		case TIMER_ML_DHXML:
			obj_timer = copyCommTiemrMlDHL(env,timer_info->timer +i);
			break;
		case TIMER_ZH_DHX:
			obj_timer = copyCommTiemrZhDHX(env,timer_info->timer +i);
			break;
		default:
			obj_timer = copyAllocCommTimerBase(env, timer_info->timer + i);
			break;
		}

		if (obj_timer != NULL) {
			(*env)->CallBooleanMethod(env, obj_arraylist, arraylist_add, obj_timer);
			//SAFE_DEL_LOCAL_REF(obj_timer);
		}
	}
	
	fid = (*env)->GetFieldID(env, class_timer_info, "timers", "L"CLASS_ARRAY_LIST";");
	(*env)->SetObjectField(env, obj_timer_info, fid, obj_arraylist);
	SAFE_DEL_LOCAL_REF(class_arraylist);
	SAFE_DEL_LOCAL_REF(obj_arraylist);
}


static void copyCommTimerInfo(JNIEnv* env, jclass class_slave, jobject obj_slave, 
	cl_comm_timer_head_t *comm_timer, cl_dev_timer_summary_t *timer_summary, int sub_type, int ext_type)
{
	jfieldID fid = NULL;
	jclass class_timer_info = NULL;
	jobject obj_timer_info = NULL;
	int tdev_type = TDEV_MAX;

	class_timer_info = (*env)->FindClass(env, CLASS_RF_TIMER_INFO);
	obj_timer_info = (*env)->AllocObject(env, class_timer_info);

	jniCopyByteValue(env, class_timer_info, "max_timer_count", obj_timer_info, timer_summary->max_timer_count);

	copyCommTimerExec(env, class_timer_info, obj_timer_info, "next_on", &comm_timer->next_on);
	copyCommTimerExec(env, class_timer_info, obj_timer_info, "next_off", &comm_timer->next_off);
	copyCommTimerExec(env, class_timer_info, obj_timer_info, "next_temp_start", &comm_timer->next_temp_start);
	copyCommTimerExec(env, class_timer_info, obj_timer_info, "next_temp_finish", &comm_timer->next_temp_finish);


	tdev_type = dev_type_map(sub_type,ext_type);
	if (tdev_type == TDEV_CENTRAL_AIRCON) {//兼容之前的，中央空调
		copyCommTimers(env, class_timer_info, obj_timer_info, comm_timer);
	} else {
		copyCommTimersExtends(env, class_timer_info, obj_timer_info, tdev_type, comm_timer);
	}
	fid = (*env)->GetFieldID(env, class_slave, "comm_timer", "L" CLASS_RF_TIMER_INFO ";");
	(*env)->SetObjectField(env, obj_slave, fid, obj_timer_info);
	SAFE_DEL_LOCAL_REF(class_timer_info);
	SAFE_DEL_LOCAL_REF(obj_timer_info);
}

static jobject CopySlave(JNIEnv* env, cl_slave_t *slave)
{
	jfieldID fid;
	jclass class_obj;
	jobject obj;
	jstring str;
	char buf[64];

	if(IsSupportDev(env,slave->dev_type,slave->ext_type) == false
		|| filterSlave(slave) == true){
		return NULL;
	}

	cl_rfdev_status_t *rfdev_p = &(slave->rfdev);

//	LOGE("xxxddd copy slave %llu, master handle =%d\n", slave->obj.sn, slave->master_handle);

	class_obj = (*env)->FindClass(env, CLASS_SLAVE);
	obj = (*env)->AllocObject(env, class_obj);

	fid = (*env)->GetFieldID(env, class_obj, "type", "I");
	(*env)->SetIntField(env, obj, fid, slave->obj.type);
	
	fid = (*env)->GetFieldID(env, class_obj, "status", "I");
	(*env)->SetIntField(env, obj, fid, slave->obj.status);
	
	fid = (*env)->GetFieldID(env, class_obj, "handle", "I");
	(*env)->SetIntField(env, obj, fid, slave->obj.handle);

	fid = (*env)->GetFieldID(env, class_obj, "masterDevHandle", "I");
	(*env)->SetIntField(env, obj, fid, slave->master_handle);
	
	jniCopyString(env, class_obj, "name", obj, slave->obj.name);

	fid = (*env)->GetFieldID(env, class_obj, "sn", "J");
	(*env)->SetLongField(env, obj, fid, slave->obj.sn);

	fid = (*env)->GetFieldID(env, class_obj, "dev_type", "I");
	(*env)->SetIntField(env, obj, fid, slave->dev_type);

	fid = (*env)->GetFieldID(env, class_obj, "bind_error", "I");
	(*env)->SetIntField(env, obj, fid, slave->bind_error);
	
	fid = (*env)->GetFieldID(env, class_obj, "other_master_sn", "J");
	(*env)->SetLongField(env, obj, fid, slave->other_master_sn);

	sprintf(buf, "%u.%u.%u", slave->soft_version.major, slave->soft_version.minor, slave->soft_version.revise);
	jniCopyString(env, class_obj, "soft_version", obj, buf);

	sprintf(buf, "%u.%u.%u", slave->upgrade_version.major, slave->upgrade_version.minor, slave->upgrade_version.revise);
	jniCopyString(env, class_obj, "upgrade_version", obj, buf);

	//pad is not zero , the rf_stm_ver is valid.
	if(slave->rf_stm_ver.pad){
		sprintf(buf, "%u.%u.%u", slave->rf_stm_ver.major, slave->rf_stm_ver.minor, slave->rf_stm_ver.revise);
		jniCopyString(env, class_obj, "rf_stm_ver", obj, buf);
	}

	fid = (*env)->GetFieldID(env, class_obj, "uptime", "I");
	(*env)->SetIntField(env, obj, fid, slave->uptime);

	fid = (*env)->GetFieldID(env, class_obj, "online", "I");
	(*env)->SetIntField(env, obj, fid, slave->online);

	fid = (*env)->GetFieldID(env, class_obj, "conn_internet", "I");
	(*env)->SetIntField(env, obj, fid, slave->conn_internet);

	fid = (*env)->GetFieldID(env, class_obj, "area_handle", "I");
	(*env)->SetIntField(env, obj, fid, slave->area_handle);

	fid = (*env)->GetFieldID(env, class_obj, "has_current_detect", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_current_detect);

	fid = (*env)->GetFieldID(env, class_obj, "has_electric_stat", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_electric_stat);

	fid = (*env)->GetFieldID(env, class_obj, "has_video_record", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_video_record);

	fid = (*env)->GetFieldID(env, class_obj, "has_video_flip", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_video_flip);

	fid = (*env)->GetFieldID(env, class_obj, "has_ir", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_ir);

	fid = (*env)->GetFieldID(env, class_obj, "has_ptz", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_ptz);

	fid = (*env)->GetFieldID(env, class_obj, "has_recv_flag_pkt", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_recv_flag_pkt);

	fid = (*env)->GetFieldID(env, class_obj, "is_support_la", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->is_support_la);

	fid = (*env)->GetFieldID(env, class_obj, "has_rf", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_rf);

	fid = (*env)->GetFieldID(env, class_obj, "has_alarm", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_alarm);

	fid = (*env)->GetFieldID(env, class_obj, "area_handle", "I");
	(*env)->SetIntField(env, obj, fid, slave->area_handle);
	
	//拷贝开发者ID
	if(slave->developer_id != NULL){
		jniCopyString(env, class_obj, "developer_id", obj, slave->developer_id);
	}
	//是否支持饱和度变量拷贝
	fid = (*env)->GetFieldID(env, class_obj, "has_v4l2_color_setting", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_v4l2_color_setting);

	//是否支持云台转速变量拷贝
	fid = (*env)->GetFieldID(env, class_obj, "has_roll_speed_ctrl", "Z");
	(*env)->SetBooleanField(env, obj, fid, slave->has_roll_speed_ctrl);

	if( slave->dev_type == IJ_805){
		
		//屏幕是否开
		fid = (*env)->GetFieldID(env, class_obj, "is_805_screen_on", "Z");
		(*env)->SetBooleanField(env, obj, fid, slave->is_805_screen_on);
		
		//蜂鸣是否开
		fid = (*env)->GetFieldID(env, class_obj, "is_805_beep_on", "Z");
		(*env)->SetBooleanField(env, obj, fid, slave->is_805_beep_on);
		
	}

	jni_copy_simple_class(env, class_obj, obj,
		                    TRIPLES(boolean, slave, is_support_public_shortcuts_onoff),
		                    JNI_VAR_ARG_END);
	JNI_COPY_SIMPLE_CLASS(env, class_obj, obj, CLASS_COMM_SHORTCUT_POWER, shortcuts_onoff,
		                     TRIPLES(boolean, &slave->shortcuts_onoff, enable),
		                     TRIPLES(boolean, &slave->shortcuts_onoff, onoff),
		                     TRIPLES(int, &slave->shortcuts_onoff, time),
		                     TRIPLES(int, &slave->shortcuts_onoff, remain_time),
		                     JNI_VAR_ARG_END);

	jniCopyIntValue(env, class_obj, "ext_type", obj, slave->ext_type);
	
	//LOGD("CopyRFDevInfo   ext_type = %d\n", slave->ext_type);
	//LOGD("CopyRFDevInfo   dev_type = %d\n", slave->dev_type);
	//LOGD("CopyRFDevInfo   sn = %"PRIu64"\n", slave->obj.sn);
///////////rf 相关信息
	jniCopyBooleanValue(env, class_obj, "status_valid", obj, slave->status_valid);
	jniCopyBooleanValue(env, class_obj, "is_upgrade_err", obj, slave->is_upgrade_err);
	jniCopyBooleanValue(env, class_obj, "is_support_dbc", obj, slave->is_support_dbc);
	copyCommTimerInfo(env, class_obj, obj, &slave->comm_timer, &slave->timer_summary, slave->dev_type, slave->ext_type);
	CopyRFDevInfo(env, rfdev_p, class_obj, obj);
////////////////
	(*env)->DeleteLocalRef(env, class_obj);

	return obj;
}

static jobject CopyUsbVideo(JNIEnv* env, cl_usb_video_t *video)
{
	jfieldID fid;
	jclass class_obj;
	jobject obj;
	jstring str;

	class_obj = (*env)->FindClass(env, CLASS_USB_VIDEO);
	obj = (*env)->AllocObject(env, class_obj);
	
	fid = (*env)->GetFieldID(env, class_obj, "type", "I");
	(*env)->SetIntField(env, obj, fid, video->obj.type);
	
	fid = (*env)->GetFieldID(env, class_obj, "status", "I");
	(*env)->SetIntField(env, obj, fid, video->obj.status);
	
	fid = (*env)->GetFieldID(env, class_obj, "handle", "I");
	(*env)->SetIntField(env, obj, fid, video->obj.handle);

	jniCopyString(env, class_obj, "name", obj, video->obj.name);

	fid = (*env)->GetFieldID(env, class_obj, "sn", "J");
	(*env)->SetLongField(env, obj, fid, video->obj.sn);

	(*env)->DeleteLocalRef(env, class_obj);

	return obj;
}

static jobject CopyObj(JNIEnv* env, cl_obj_t *cl_obj)
{
	jfieldID fid;
	jclass class_obj;
	jobject obj;
	jstring str;

	class_obj = (*env)->FindClass(env, CLASS_OBJ);
	obj = (*env)->AllocObject(env, class_obj);
	
	fid = (*env)->GetFieldID(env, class_obj, "type", "I");
	(*env)->SetIntField(env, obj, fid, cl_obj->type);
	
	fid = (*env)->GetFieldID(env, class_obj, "status", "I");
	(*env)->SetIntField(env, obj, fid, cl_obj->status);
	
	fid = (*env)->GetFieldID(env, class_obj, "handle", "I");
	(*env)->SetIntField(env, obj, fid, cl_obj->handle);

	jniCopyString(env, class_obj, "name", obj, cl_obj->name);

	fid = (*env)->GetFieldID(env, class_obj, "sn", "J");
	(*env)->SetLongField(env, obj, fid, cl_obj->sn);

	(*env)->DeleteLocalRef(env, class_obj);

	return obj;
}

static jobject CopyKeys(JNIEnv* env, cl_equipment_t *eq)
{
	int i;
	jfieldID fid;
	jclass class_key;
	jobject obj_key;
	jobject obj_array;
	jstring str;
	cl_obj_t *cl_obj;

	class_key = (*env)->FindClass(env, CLASS_KEY);
	obj_array = (*env)->NewObjectArray(env, eq->key_count, class_key, NULL);

	for (i = 0; i < eq->key_count; i++) {
		obj_key = (*env)->AllocObject(env, class_key);
		cl_obj = &eq->keys[i]->obj;

		// 锟斤拷锟斤拷息
		fid = (*env)->GetFieldID(env, class_key, "type", "I");
		(*env)->SetIntField(env, obj_key, fid, cl_obj->type);
		
		fid = (*env)->GetFieldID(env, class_key, "status", "I");
		(*env)->SetIntField(env, obj_key, fid, cl_obj->status);
		
		fid = (*env)->GetFieldID(env, class_key, "handle", "I");
		(*env)->SetIntField(env, obj_key, fid, cl_obj->handle);

		jniCopyString(env, class_key, "name", obj_key, cl_obj->name);

		fid = (*env)->GetFieldID(env, class_key, "sn", "J");
		(*env)->SetLongField(env, obj_key, fid, cl_obj->sn);

		// 锟斤拷展锟斤拷息
		fid = (*env)->GetFieldID(env, class_key, "equipment_handle", "I");
		(*env)->SetIntField(env, obj_key, fid, eq->keys[i]->equipment_handle);
		
		fid = (*env)->GetFieldID(env, class_key, "key_id", "I");
		(*env)->SetIntField(env, obj_key, fid, eq->keys[i]->key_id);

		fid = (*env)->GetFieldID(env, class_key, "had_learned", "Z");
		(*env)->SetBooleanField(env, obj_key, fid, eq->keys[i]->had_learned);
		
		(*env)->SetObjectArrayElement(env, obj_array, i, obj_key);
		(*env)->DeleteLocalRef(env, obj_key);
	}

	(*env)->DeleteLocalRef(env, class_key);

	return obj_array;
}


static jobject CopyAircon(JNIEnv* env, cl_equipment_t *eq)
{
	jfieldID fid;
	jclass clazz;
	jobject obj;
	jintArray match_id;

	cln_ac_key_id_t *aircon = (cln_ac_key_id_t *)&eq->last_presskey;
	if(aircon == NULL){
		return NULL;
	}

	clazz = (*env)->FindClass(env, CLASS_AIRCON_INFO);
	if(clazz == NULL ){
		return NULL;
	}
	obj = (*env)->AllocObject(env, clazz);
	if(obj == NULL ){
		return NULL;
	}
	
	fid = (*env)->GetFieldID(env, clazz, "match_id_num", "I");
	(*env)->SetIntField(env, obj, fid, eq->match_id_num);

	fid = (*env)->GetFieldID(env, clazz, "ac_type", "I");
	(*env)->SetIntField(env, obj, fid, eq->ac_type);
	
	jniCopyShortArray(env,clazz,"match_id",obj,&eq->match_id[0],4);

	fid = (*env)->GetFieldID(env, clazz, "onoff", "Z");
	(*env)->SetBooleanField(env, obj, fid, !aircon->onoff);
	
	fid = (*env)->GetFieldID(env, clazz, "mode", "I");
	(*env)->SetIntField(env, obj, fid, aircon->mode);

	fid = (*env)->GetFieldID(env, clazz, "temp", "I");
	(*env)->SetIntField(env, obj, fid, aircon->temp + AC_TEMP_BASE);

	fid = (*env)->GetFieldID(env, clazz, "speed", "I");
	(*env)->SetIntField(env, obj, fid, aircon->speed);

	fid = (*env)->GetFieldID(env, clazz, "dir", "I");
	(*env)->SetIntField(env, obj, fid, aircon->dir);
	
	fid = (*env)->GetFieldID(env, clazz, "key_v", "I");
	(*env)->SetIntField(env, obj, fid, aircon->key_v);

	fid = (*env)->GetFieldID(env, clazz, "oldkey_v", "I");
	(*env)->SetIntField(env, obj, fid, aircon->oldkey_v);

	(*env)->DeleteLocalRef(env, clazz);

	return obj;
}



static jobject CopyAlarmInfo(JNIEnv* env, cl_equipment_t *eq)
{
	jfieldID fid;
	jclass clazz;
	jobject obj;
	jstring str;
	cl_alarm_info_t *ai = eq->alarm_info;

	clazz = (*env)->FindClass(env, CLASS_ALARM_INFO);
	obj = (*env)->AllocObject(env, clazz);

	fid = (*env)->GetFieldID(env, clazz, "push_enable", "Z");
	(*env)->SetBooleanField(env, obj, fid, ai->push_enable);

	fid = (*env)->GetFieldID(env, clazz, "sms_enable", "Z");
	(*env)->SetBooleanField(env, obj, fid, ai->sms_enable);

	
	fid = (*env)->GetFieldID(env, clazz, "isLearned", "Z");
	(*env)->SetBooleanField(env, obj, fid, ai->isLearned);

	fid = (*env)->GetFieldID(env, clazz, "phone_num", "I");
	(*env)->SetIntField(env, obj, fid, ai->phone_num);

	fid = (*env)->GetFieldID(env, clazz, "soundline_num", "I");
	(*env)->SetIntField(env, obj, fid, ai->soundline_num);

	fid = (*env)->GetFieldID(env, clazz, "soundline_on", "I");
	(*env)->SetIntField(env, obj, fid, ai->soundline_on);

	fid = (*env)->GetFieldID(env, clazz, "scene", "I");
	(*env)->SetIntField(env, obj, fid, ai->scene);

	fid = (*env)->GetFieldID(env, clazz, "soundline", "[I");
	jintArray soundline = (*env)->NewIntArray(env, ai->soundline_num);
	(*env)->SetIntArrayRegion(env, soundline, 0, ai->soundline_num, ai->soundline);
	(*env)->SetObjectField(env, obj, fid, soundline);			

	jniCopyString(env, clazz, "alarm_msg", obj, ai->alarm_msg);

	if (ai->phone_num > 0) {
		int i;
		jclass class_string;
		jobject string_array;
		char *phone;
		
		class_string = (*env)->FindClass(env, "java/lang/String");
		string_array = (*env)->NewObjectArray(env, ai->phone_num, class_string, NULL);

		for (i = 0; i < ai->phone_num; i++) {
			phone = ai->phone_list[i];
			doString(&phone);
			str = (*env)->NewStringUTF(env, phone);
			(*env)->SetObjectArrayElement(env, string_array, i, str);	
			(*env)->DeleteLocalRef(env, str);
		}
		
		// 锟斤拷锟介构锟斤拷锟斤拷希锟斤拷锟街?
		fid = (*env)->GetFieldID(env, clazz, "phone_list", "[Ljava/lang/String;");
		(*env)->SetObjectField(env, obj, fid, string_array);

		(*env)->DeleteLocalRef(env, string_array);
		(*env)->DeleteLocalRef(env, class_string);
	}
	(*env)->DeleteLocalRef(env, clazz);

	return obj;
}

static jobject CopyEq(JNIEnv* env, cl_equipment_t *eq)
{
	jfieldID fid;
	jclass class_obj;
	jobject obj;
	jstring str;
	cl_obj_t *cl_obj = &eq->obj;

	// 锟斤拷锟斤拷息
	class_obj = (*env)->FindClass(env, CLASS_EQUIPMENT);
	obj = (*env)->AllocObject(env, class_obj);
	
	fid = (*env)->GetFieldID(env, class_obj, "type", "I");
	(*env)->SetIntField(env, obj, fid, cl_obj->type);
	
	fid = (*env)->GetFieldID(env, class_obj, "status", "I");
	(*env)->SetIntField(env, obj, fid, cl_obj->status);
	
	fid = (*env)->GetFieldID(env, class_obj, "handle", "I");
	(*env)->SetIntField(env, obj, fid, cl_obj->handle);

	jniCopyString(env, class_obj, "name", obj, cl_obj->name);

	fid = (*env)->GetFieldID(env, class_obj, "sn", "J");
	(*env)->SetLongField(env, obj, fid, cl_obj->sn);

	jniCopyIntValue(env,class_obj,"infr_slave_handle",obj,eq->eq_001e_handle);

	// 锟斤拷锟斤拷锟斤拷展锟斤拷息

	fid = (*env)->GetFieldID(env, class_obj, "area_handle", "I");
	(*env)->SetIntField(env, obj, fid, eq->area_handle);
	
	fid = (*env)->GetFieldID(env, class_obj, "dev_type", "I");
	(*env)->SetIntField(env, obj, fid, eq->dev_type);

	fid = (*env)->GetFieldID(env, class_obj, "key_count", "I");
	(*env)->SetIntField(env, obj, fid, eq->key_count);

	fid = (*env)->GetFieldID(env, class_obj, "is_more_ctrl", "Z");
	(*env)->SetBooleanField(env, obj, fid, eq->is_more_ctrl);

	fid = (*env)->GetFieldID(env, class_obj, "is_rf_repeater", "Z");
	(*env)->SetBooleanField(env, obj, fid, eq->is_rf_repeater);

	fid = (*env)->GetFieldID(env, class_obj, "rf_repeater_on", "Z");
	(*env)->SetBooleanField(env, obj, fid, eq->rf_repeater_on);

	jniCopyIntValue(env, class_obj, "db_dimming_lamp_value", obj, eq->db_dimming_lamp_value);

	fid = (*env)->GetFieldID(env, class_obj, "group_num", "I");
	(*env)->SetIntField(env, obj, fid, eq->group_num);

	fid = (*env)->GetFieldID(env, class_obj, "group_state", "I");
	(*env)->SetIntField(env, obj, fid, eq->group_state);

	fid = (*env)->GetFieldID(env, class_obj, "local_id", "I");
	(*env)->SetIntField(env, obj, fid, eq->local_id);
	if (eq->key_count > 0) {
		jobject obj_array;
		
		obj_array = CopyKeys(env, eq);

		// 锟斤拷锟介构锟斤拷锟斤拷希锟斤拷锟街?
		fid = (*env)->GetFieldID(env, class_obj, "key", "[L" CLASS_KEY ";");
		(*env)->SetObjectField(env, obj, fid, obj_array);
		(*env)->DeleteLocalRef(env, obj_array);
	}

	if (eq->alarm_info != NULL) {
		jobject obj_ai;
		obj_ai = CopyAlarmInfo(env, eq);
		fid = (*env)->GetFieldID(env, class_obj, "alarm_info", "L" CLASS_ALARM_INFO ";");
		(*env)->SetObjectField(env, obj, fid, obj_ai);
		(*env)->DeleteLocalRef(env, obj_ai);
	}
	//if( eq->last_presskey != 0 ){
		jobject obj_aircon;
		obj_aircon = CopyAircon(env, eq);
		fid = (*env)->GetFieldID(env, class_obj, "aircon", "L" CLASS_AIRCON_INFO ";");
		(*env)->SetObjectField(env, obj, fid, obj_aircon);
		(*env)->DeleteLocalRef(env, obj_aircon);
	//}
	

	(*env)->DeleteLocalRef(env, class_obj);

	return obj;
}

static jint CopyDevInfoObjArray(JNIEnv* env, jclass class_dev_info, jobject obj_dev_info, cl_dev_info_t *info)
{
	int i;
	jfieldID fid;
	jclass class_obj;
	jstring str;
	jobjectArray obj_array;
	jobject obj;
	cl_slave_t *slave = NULL;
	int j = 0;

	if(info->num_objs ==0 ){
		return 0;
	}

	for (i = 0; i < info->num_objs; i++) {
		switch (info->objs[i]->type) {
		case OT_SLAVE:
			slave = (cl_slave_t *)info->objs[i];
			if(slave != NULL
				&& IsSupportDev(env, slave->dev_type, slave->ext_type) == true
				&& filterSlave(slave) == false){
				j++;
			}
			break;
		case OT_USB_VIDEO:
		default:
			break;
		}
	}

	class_obj = (*env)->FindClass(env, CLASS_OBJ);
	obj_array = (*env)->NewObjectArray(env, j, class_obj, NULL);

	j = 0;
	for (i = 0; i < info->num_objs; i++) {
		switch (info->objs[i]->type) {
		case OT_SLAVE:
			obj = CopySlave(env, (cl_slave_t *)info->objs[i]);
			break;
		case OT_USB_VIDEO:
			obj = CopyUsbVideo(env, (cl_usb_video_t *)info->objs[i]);
			break;
		case OT_EQUIPMENT:
			obj = CopyEq(env, (cl_equipment_t *)info->objs[i]);
			break;
		default:
			obj = CopyObj(env, info->objs[i]);
			break;
		}
		
		if(obj != NULL){
			(*env)->SetObjectArrayElement(env, obj_array, j, obj);
			(*env)->DeleteLocalRef(env, obj);
			j++;
		}
	}

	// 锟斤拷锟介构锟斤拷锟斤拷希锟斤拷锟街?
	fid = (*env)->GetFieldID(env, class_dev_info, "objs", "[L" CLASS_OBJ ";");
	(*env)->SetObjectField(env, obj_dev_info, fid, obj_array);
	if (obj_array != NULL)
		(*env)->DeleteLocalRef(env, obj_array);
	(*env)->DeleteLocalRef(env, class_obj);
	return j;
}

#if 0
JNIEXPORT jobject JNICALL
NAME(ClUserGetDevInfo)(JNIEnv* env, jobject this, jint user_handle)
{
	cl_dev_info_t *info;
	jclass class_dev_info;
	jobject obj_dev_info;

	if ((info = cl_user_get_dev_info(user_handle)) == NULL)
		return NULL;

	class_dev_info = (*env)->FindClass(env, CLASS_DEV_INFO);
	obj_dev_info = (*env)->AllocObject(env, class_dev_info);

	CopyDevInfoBase(env, class_dev_info, obj_dev_info, info);
	CopyDevInfoObjArray(env, class_dev_info, obj_dev_info, info);

	// free tmp buffer
	cl_user_free_dev_info(info);

	(*env)->DeleteLocalRef(env, class_dev_info);
	
	return obj_dev_info;
}
#endif

JNIEXPORT jint JNICALL
NAME(ClSlaveUnbind)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_slave_unbind(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClSlaveReboot)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_slave_reboot(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClSlaveBind)(JNIEnv* env, jobject this, jint slave_handle, jstring obj_passwd)
{
	const char *str_passwd;
	RS ret = RS_ERROR;
	
	str_passwd = (*env)->GetStringUTFChars(env, obj_passwd, NULL);
	ret = cl_slave_bind(slave_handle, str_passwd);
	
	(*env)->ReleaseStringUTFChars(env, obj_passwd, str_passwd);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClDnsCacheClean)(JNIEnv* env, jobject this)
{
	return cl_dns_cache_clean();
}


static jobject fillInterfaceInfo(JNIEnv* env,cl_ni_t* netstat)
{
	jclass class_if_info;
	jobject if_info;
	jfieldID fid;
	jstring str;

	class_if_info = (*env)->FindClass(env, CLASS_SLAVEINTERFACEINFO);
	if_info = (*env)->AllocObject(env, class_if_info);

	jniCopyString(env, class_if_info, "iName", if_info, netstat->name);

	fid = (*env)->GetFieldID(env, class_if_info, "isup", "Z");
	(*env)->SetBooleanField(env, if_info, fid, netstat->is_up);

	fid = (*env)->GetFieldID(env, class_if_info, "isIpValid", "Z");
	(*env)->SetBooleanField(env, if_info, fid, netstat->is_ip_valid);

	fid = (*env)->GetFieldID(env, class_if_info, "ipaddr", "I");
	(*env)->SetIntField(env, if_info, fid, netstat->ip);

	fid = (*env)->GetFieldID(env, class_if_info, "mtu", "I");
	(*env)->SetIntField(env, if_info, fid, netstat->mtu);

	fid = (*env)->GetFieldID(env, class_if_info, "rxBytes", "J");
	(*env)->SetLongField(env, if_info, fid, netstat->rx_byte);

	fid = (*env)->GetFieldID(env, class_if_info, "txBytes", "J");
	(*env)->SetLongField(env, if_info, fid, netstat->tx_byte);

	(*env)->DeleteLocalRef(env, class_if_info);

	return if_info;
}

static jobjectArray fillClientInfo(JNIEnv* env,int lanCount ,cl_lan_client_t** lanclientlist)
{
	jclass class_client_info;
	jobject client_info;
	jfieldID fid;
	jobjectArray obj_array;
	cl_lan_client_t* client;
	jstring str;
	char macbuf[128];
	int i;

	if(lanCount <= 0){
		return  NULL;
	}

	class_client_info = (*env)->FindClass(env, CLASS_SLAVECLIENTINFO);
	obj_array = (*env)->NewObjectArray(env, lanCount, class_client_info, NULL);

	for(i=0; i<lanCount; i++){
		client_info = (*env)->AllocObject(env, class_client_info);
		client = lanclientlist[i];

		jniCopyString(env, class_client_info, "cliName", client_info, client->name);

		sprintf(macbuf,"%02X.%02X.%02X.%02X.%02X.%02X",client->mac[0],client->mac[1],client->mac[2],
		                                               client->mac[3],client->mac[4],client->mac[5]);
		jniCopyString(env, class_client_info, "cliMacString", client_info, macbuf);

		fid = (*env)->GetFieldID(env, class_client_info, "cliIsFromWifi", "Z");
		(*env)->SetBooleanField(env, client_info, fid, client->is_from_wifi);

		fid = (*env)->GetFieldID(env, class_client_info, "cliIp", "I");
		(*env)->SetIntField(env, client_info, fid, client->ip);

		(*env)->SetObjectArrayElement(env, obj_array, i, client_info);

		(*env)->DeleteLocalRef(env, client_info);
	}

	(*env)->DeleteLocalRef(env, class_client_info);

	return obj_array;
}

void CopyEthkInfo(JNIEnv* env, jclass class_stat_info, jobject stat_info, cl_dev_stat_t *stat) {
	jclass eth_class;
	jobject eth_obj;
	jfieldID fid;
	cl_eth_info_t* eth_info_p = &(stat->udp_dev_stat.eth_info);

	eth_class = (*env)->FindClass(env, CLASS_ETH_INFO);
	if (eth_class == NULL) {
		goto END;
	}
	eth_obj = (*env)->AllocObject(env, eth_class);
	if (eth_obj == NULL) {
		goto END;
	}
	jni_copy_simple_class(env, eth_class, eth_obj,
								TRIPLES(boolean, eth_info_p, valid),
								TRIPLES(byte, eth_info_p, num),
								JNI_VAR_ARG_END);

	if (eth_info_p->num > 0 && eth_info_p->valid) {
		JNI_COPY_ARRAY_CLASS(env, eth_class, eth_obj, CLASS_ETH_ITEM_INFO, eth_item,
			                    eth_info_p->num, sizeof(cl_eth_item_info_t),
			                    ARRAY_TRIPLES(byte, eth_info_p->eth_item, index),
			                    ARRAY_TRIPLES(int, eth_info_p->eth_item, ip),
			                    ARRAY_TRIPLES(String, eth_info_p->eth_item, name),
			                    ARRAY_TRIPLES(int, eth_info_p->eth_item, tx_rate),
			                    ARRAY_TRIPLES(int, eth_info_p->eth_item, rx_rate),
			                    JNI_VAR_ARG_END);
	}
	
	LOGD("copy eth info, valid=%u, eth_num=%u", eth_info_p->valid, eth_info_p->num);

	fid = (*env)->GetFieldID(env, class_stat_info, "eth_info", "L" CLASS_ETH_INFO ";");
	(*env)->SetObjectField(env, stat_info, fid, eth_obj);
END:
	SAFE_DEL_LOCAL_REF(eth_class);
	SAFE_DEL_LOCAL_REF(eth_obj);
}	
void CopyDiskInfo(JNIEnv* env, jclass class_stat_info, jobject stat_info, cl_dev_stat_t *stat) {
	jclass disk_class;
	jobject disk_obj;
	jfieldID fid;
	cl_disk_info_t* disk_info_p = &(stat->udp_dev_stat.disk_info);

	disk_class = (*env)->FindClass(env, CLASS_DISK_INFO);
	if (disk_class == NULL) {
		goto END;
	}
	disk_obj = (*env)->AllocObject(env, disk_class);
	if (disk_obj == NULL) {
		goto END;
	}
	jni_copy_simple_class(env, disk_class, disk_obj,
								TRIPLES(boolean, disk_info_p, valid),
								TRIPLES(byte, disk_info_p, mode),
								TRIPLES(int, disk_info_p, total_capacity),
								TRIPLES(int, disk_info_p, used_capacity),
								TRIPLES(byte, disk_info_p, num),
								JNI_VAR_ARG_END);

	if (disk_info_p->num > 0 && disk_info_p->valid) {
		JNI_COPY_ARRAY_CLASS(env, disk_class, disk_obj, CLASS_DISK_ITEM_INFO, disk_item,
			                    disk_info_p->num, sizeof(cl_disk_item_info_t),
			                    ARRAY_TRIPLES(short, disk_info_p->disk_item, temp),
			                    ARRAY_TRIPLES(int, disk_info_p->disk_item, use_time),
			                    ARRAY_TRIPLES(String, disk_info_p->disk_item, model),
			                    ARRAY_TRIPLES(String, disk_info_p->disk_item, serial),
			                    ARRAY_TRIPLES(int, disk_info_p->disk_item, capacity),
			                    JNI_VAR_ARG_END);
	}
	
	LOGD("copy disk info, valid=%u, disk_num=%u", disk_info_p->valid, disk_info_p->num);

	fid = (*env)->GetFieldID(env, class_stat_info, "disk_info", "L" CLASS_DISK_INFO ";");
	(*env)->SetObjectField(env, stat_info, fid, disk_obj);
END:
	SAFE_DEL_LOCAL_REF(disk_class);
	SAFE_DEL_LOCAL_REF(disk_obj);
}	
void CopyAdvancedInfo(JNIEnv* env, jclass class_stat_info, jobject stat_info, cl_dev_stat_t *stat){
	char buf[64];
	cl_udp_dev_stat udp_stat = stat->udp_dev_stat;

	jniCopyBooleanValue(env, class_stat_info, "is_stat_valid", stat_info, udp_stat.is_stat_valid);
	jniCopyIntValue(env, class_stat_info, "devSvnVersion", stat_info, udp_stat.devSvnVersion);

	jniCopyIntValue(env, class_stat_info, "dev_cur", stat_info, udp_stat.dev_cur);
	jniCopyIntValue(env, class_stat_info, "dev_cur_ad", stat_info, udp_stat.dev_cur_ad);
	jniCopyIntValue(env, class_stat_info, "dev_cur_b", stat_info, udp_stat.dev_cur_b);
	jniCopyIntValue(env, class_stat_info, "dev_cur_k", stat_info, udp_stat.dev_cur_k);
	jniCopyIntValue(env, class_stat_info, "dev_cur_phone_num", stat_info, udp_stat.dev_cur_phone_num);
	
	jniCopyIntValue(env, class_stat_info, "dev_light_ad", stat_info, udp_stat.dev_light_ad);
	jniCopyIntValue(env, class_stat_info, "dev_vol", stat_info, udp_stat.dev_vol);
	jniCopyIntValue(env, class_stat_info, "dev_vol_ad", stat_info, udp_stat.dev_vol_ad);
	jniCopyIntValue(env, class_stat_info, "dev_vol_b", stat_info, udp_stat.dev_vol_b);
	jniCopyIntValue(env, class_stat_info, "dev_vol_k", stat_info, udp_stat.dev_vol_k);
	jniCopyIntValue(env, class_stat_info, "avg_ad", stat_info, udp_stat.avg_ad);
	jniCopyIntValue(env, class_stat_info, "max_ad", stat_info, udp_stat.max_ad);
	jniCopyIntValue(env, class_stat_info, "delay_power_on_time", stat_info, udp_stat.delay_power_on_time);
	jniCopyIntValue(env, class_stat_info, "delay_power_off_time", stat_info, udp_stat.delay_power_off_time);
	jniCopyIntValue(env, class_stat_info, "no_load_ad", stat_info, udp_stat.no_load_ad);
	jniCopyIntValue(env, class_stat_info, "smt_soft_ver", stat_info, udp_stat.smt_soft_ver);
	jniCopyIntValue(env, class_stat_info, "smt_hard_ver", stat_info, udp_stat.smt_hard_ver);
	jniCopyIntValue(env, class_stat_info, "ir_lib_id", stat_info, udp_stat.ir_lib_id);
	jniCopyIntValue(env, class_stat_info, "cold_delay_pn_time", stat_info, udp_stat.cold_delay_pn_time);
	jniCopyIntValue(env, class_stat_info, "cold_delay_pf_time", stat_info, udp_stat.cold_delay_pf_time);
	jniCopyIntValue(env, class_stat_info, "hot_delay_pn_time", stat_info, udp_stat.hot_delay_pn_time);
	jniCopyIntValue(env, class_stat_info, "hot_delay_pf_time", stat_info, udp_stat.hot_delay_pf_time);
	if(udp_stat.stm_32_dbg_info)
		jniCopyString(env, class_stat_info, "stm_32_dbg_info", stat_info, udp_stat.stm_32_dbg_info);
	
	jniCopyIntValue(env, class_stat_info, "yesterday", stat_info, udp_stat.light_study.yesterday);
	jniCopyIntValue(env, class_stat_info, "today", stat_info, udp_stat.light_study.today);
	jniCopyIntArray(env, class_stat_info, "sample", stat_info, udp_stat.light_study.sample,24);
	jniCopyString(env, class_stat_info, "dev_domain",stat_info, udp_stat.dev_domain); //待处理

	jniCopyIntValue(env, class_stat_info, "wifi_conn_time", stat_info, udp_stat.wifi_conn_time);
	jniCopyByteValue(env, class_stat_info, "wifi_rssi", stat_info, udp_stat.wifi_rssi);
	jniCopyByteValue(env, class_stat_info, "wifi_phy_mode", stat_info, udp_stat.wifi_phy_mode);
	sprintf(buf, "%u.%u.%u", udp_stat.wifi_version.major, udp_stat.wifi_version.minor, udp_stat.wifi_version.revise);
	jniCopyString(env, class_stat_info, "wifi_version", stat_info, buf);
}

void CopyDebugInfo(JNIEnv* env, jclass class_stat_info, jobject stat_info, cl_dev_stat_t *stat){
	cl_udp_dev_stat udp_stat = stat->udp_dev_stat;
	char version_str[32];

	
	jniCopyIntValue(env, class_stat_info, "dev_to_server_stat", stat_info, udp_stat.dev_to_server_stat);
	jniCopyIntValue(env, class_stat_info, "dispatch_ip", stat_info, udp_stat.dispatch_ip);
	jniCopyIntValue(env, class_stat_info, "dev_server_ip", stat_info, udp_stat.dev_server_ip);
	jniCopyIntValue(env, class_stat_info, "dev_cur_time", stat_info, udp_stat.dev_cur_time);
	jniCopyString(env, class_stat_info, "dev_img_compile_data", stat_info, udp_stat.dev_img_compile_data);
	jniCopyString(env, class_stat_info, "dev_img_compile_time", stat_info, udp_stat.dev_img_compile_time);

	sprintf(version_str, "%hhu.%hhu.%hhu", udp_stat.kernel_image_version.major,
		udp_stat.kernel_image_version.minor, udp_stat.kernel_image_version.revise);
	jniCopyString(env, class_stat_info, "kernel_image_version", stat_info, version_str);
	jniCopyIntValue(env, class_stat_info, "kernel_image_svn", stat_info, udp_stat.kernel_image_svn);

	sprintf(version_str, "%hhu.%hhu.%hhu", udp_stat.user_image_version.major,
		udp_stat.user_image_version.minor, udp_stat.user_image_version.revise);
	jniCopyString(env, class_stat_info, "user_image_version", stat_info, version_str);
	jniCopyIntValue(env, class_stat_info, "user_image_svn", stat_info, udp_stat.user_image_svn);
	jniCopyIntValue(env, class_stat_info, "wired_wan_ip", stat_info, udp_stat.wired_wan_ip);

	jniCopyByteArray(env, class_stat_info, "wifi_mac", stat_info, udp_stat.wifi_mac,6);
}

JNIEXPORT jobject JNICALL
NAME(clLookupSlaveStatInfo)(JNIEnv* env, jobject this, jint slave_handle)
{
	char buffer[128];
	jclass class_stat_info;
	cl_dev_stat_t* stat = NULL;
	jclass class_client_info;

	jobject stat_info = NULL;
	jobject if_info;
	jobject client_array;
	jfieldID fid;
	jstring str;

	stat = cl_get_dev_stat_info(slave_handle);

	class_stat_info = (*env)->FindClass(env, CLASS_SLAVESTATINFO);
	stat_info = (*env)->AllocObject(env, class_stat_info);
	if(!stat){
		(*env)->DeleteLocalRef(env, class_stat_info);
		return stat_info;
	}
/*
	if(!!stat->udp_dev_stat.is_stat_valid) {
		CopyAdvancedInfo(env, class_stat_info, stat_info, stat);
		CopyDebugInfo(env, class_stat_info, stat_info, stat);
	}
*/
	CopyAdvancedInfo(env, class_stat_info, stat_info, stat);
	CopyDebugInfo(env, class_stat_info, stat_info, stat);
	CopyDiskInfo(env, class_stat_info, stat_info, stat);
	CopyEthkInfo(env, class_stat_info, stat_info, stat);
	
	fid = (*env)->GetFieldID(env, class_stat_info, "slaveHandle", "I");
	(*env)->SetIntField(env, stat_info, fid, stat->handle);

	fid = (*env)->GetFieldID(env, class_stat_info, "sn", "J");
	(*env)->SetLongField(env, stat_info, fid, stat->sn);

	fid = (*env)->GetFieldID(env, class_stat_info, "cpu", "S");
	(*env)->SetShortField(env, stat_info, fid, stat->cpu);
	LOGE("clLookupSlaveStatInfo       cpu = %d\n",stat->cpu);

	fid = (*env)->GetFieldID(env, class_stat_info, "mem", "S");
	(*env)->SetShortField(env, stat_info, fid, stat->mem);
	LOGE("clLookupSlaveStatInfo       mem = %d\n",stat->mem);
	LOGE("clLookupSlaveStatInfo       dispatch_ip = %d\n",stat->udp_dev_stat.dispatch_ip);

	sprintf(buffer,"%u.%u.%u",stat->soft_version.major,
			stat->soft_version.minor,
			stat->soft_version.revise);
	jniCopyString(env, class_stat_info, "softVersion", stat_info, buffer);

	sprintf(buffer,"%u.%u.%u",stat->upgrade_version.major,
				stat->upgrade_version.minor,
				stat->upgrade_version.revise);
	jniCopyString(env, class_stat_info, "upgradeVersion", stat_info, buffer);

	sprintf(buffer,"%u.%u.%u",stat->hardware_version.major,
			stat->hardware_version.minor,
			stat->hardware_version.revise);
	jniCopyString(env, class_stat_info, "hardVersion", stat_info, buffer);
	
	fid = (*env)->GetFieldID(env, class_stat_info, "uptime", "I");
	(*env)->SetIntField(env, stat_info, fid, stat->uptime);

	fid = (*env)->GetFieldID(env, class_stat_info, "onlinetime", "I");
	(*env)->SetIntField(env, stat_info, fid, stat->online);

	fid = (*env)->GetFieldID(env, class_stat_info, "connInterntTime", "I");
	(*env)->SetIntField(env, stat_info, fid, stat->conn_internet);

	jniCopyString(env, class_stat_info, "ssidString", stat_info, stat->ap_ssid);

	jniCopyString(env, class_stat_info, "wifiPasswdString", stat_info, stat->ap_passwd);

	if_info = fillInterfaceInfo(env,&stat->wan);
	fid = (*env)->GetFieldID(env, class_stat_info, "wanInfo", "L" CLASS_SLAVEINTERFACEINFO ";");
	(*env)->SetObjectField(env, stat_info, fid, if_info);
	(*env)->DeleteLocalRef(env, if_info);

	if_info = fillInterfaceInfo(env,&stat->lan);
	fid = (*env)->GetFieldID(env, class_stat_info, "lanInfo", "L" CLASS_SLAVEINTERFACEINFO ";");
	(*env)->SetObjectField(env, stat_info, fid, if_info);
	(*env)->DeleteLocalRef(env, if_info);

	if(stat->client_num>0){
		fid = (*env)->GetFieldID(env,class_stat_info, "clients", "[L" CLASS_SLAVECLIENTINFO ";");
		client_array = fillClientInfo(env,stat->client_num,stat->clients);
		(*env)->SetObjectField(env, stat_info, fid, client_array);
		(*env)->DeleteLocalRef(env, client_array);
	}
	(*env)->DeleteLocalRef(env, class_stat_info);

	cl_free_dev_stat_info(stat);

	return stat_info;
}


JNIEXPORT jint JNICALL
NAME(ClSlaveModifyName)(JNIEnv* env, jobject this, jint slave_handle, jstring obj_name)
{
	const char *str_name;
	RS ret = RS_ERROR;
	
	str_name = (*env)->GetStringUTFChars(env, obj_name, NULL);
	ret = cl_slave_modify_name(slave_handle, str_name);
	
	(*env)->ReleaseStringUTFChars(env, obj_name, str_name);

	return ret;
}

static void CopyUserInfoBase(JNIEnv* env, jclass class_user_info, jobject obj_user_info, cl_user_t *info)
{
	jfieldID fid;
	jstring str;
	jobject obj;

	jniCopyString(env, class_user_info, "username", obj_user_info, info->username);
	
	jniCopyString(env, class_user_info, "password", obj_user_info, info->passwd);
	
	fid = (*env)->GetFieldID(env, class_user_info, "UserHandle", "I");
	(*env)->SetIntField(env, obj_user_info, fid, info->user_handle);
	
	fid = (*env)->GetFieldID(env, class_user_info, "is_phone_user", "Z");
	(*env)->SetBooleanField(env, obj_user_info, fid, info->is_phone_user);
	
	fid = (*env)->GetFieldID(env, class_user_info, "is_login", "Z");
	(*env)->SetBooleanField(env, obj_user_info, fid, info->is_login);
	
	fid = (*env)->GetFieldID(env, class_user_info, "last_err", "I");
	(*env)->SetIntField(env, obj_user_info, fid, info->last_err);
	
	fid = (*env)->GetFieldID(env, class_user_info, "num_dev", "I");
	(*env)->SetIntField(env, obj_user_info, fid, info->num_dev);

	jniCopyString(env,class_user_info,"vendor_id",obj_user_info,info->vendor_id);

	jniCopyString(env,class_user_info,"vendor_url",obj_user_info,info->vendor_url);

	obj = UpdateGetNewestVersion(env);
	if(obj != NULL){
		fid = (*env)->GetFieldID(env, class_user_info, "NewestVersionInfo", "L" CLASS_NEWEST_VERSION_INFO ";");
		(*env)->SetObjectField(env, obj_user_info, fid, obj);
	}
}

jint CopyDevInfo(JNIEnv* env, jclass class_user_info, jobject obj_user_info, cl_user_t *info)
{
	int i,num_dev;
	jclass class_dev_info;
	jobjectArray dev_array;
	jobject obj_dev_info;
	jfieldID fid;

	

	if (info->num_dev == 0 || info->dev == NULL) {
		return;
	}

	num_dev = info->num_dev;
	int j = 0;
	for (i = 0; i < num_dev && (info->dev[i] != NULL); i++) {
		if(IsSupportDev(env,info->dev[i]->sub_type,info->dev[i]->ext_type) == true){
			j++;
		}
	}
	class_dev_info = (*env)->FindClass(env, CLASS_DEV_INFO);
	dev_array = (*env)->NewObjectArray(env, j, class_dev_info, NULL);
	//LOGE("find   dev_array_num = %d",j);
	j = 0;
	for (i = 0; i < num_dev && (info->dev[i] != NULL); i++) {
		//LOGD("xxxddd copy dev %llu info\n", info->dev[i]->sn);	
		
		LOGE("find    dev_sn = %"PRIu64",sub_type = %d,ext_type = %d", info->dev[i]->sn,info->dev[i]->sub_type,info->dev[i]->ext_type);
		if(IsSupportDev(env,info->dev[i]->sub_type,info->dev[i]->ext_type) == true){
			obj_dev_info = (*env)->AllocObject(env, class_dev_info);
			jni_assert(obj_dev_info != NULL);
			CopyDevInfoBase(env, class_dev_info, obj_dev_info, info->dev[i]);
			LOGE("find  CopyDevInfoBase ===end");
			int obj_lenght = CopyDevInfoObjArray(env, class_dev_info, obj_dev_info, info->dev[i]);
			LOGE("find  obj_lenght = %d",obj_lenght);
			setSlaveNum(env, class_dev_info, obj_dev_info, info->dev[i] , obj_lenght);
			(*env)->SetObjectArrayElement(env, dev_array, j, obj_dev_info);
			j++;
			SAFE_DEL_LOCAL_REF(obj_dev_info);
		}
		
	}

	//空数据避免设置
	if(dev_array != NULL){
		fid = (*env)->GetFieldID(env, class_user_info, "dev", "[L" CLASS_DEV_INFO ";");
		(*env)->SetObjectField(env, obj_user_info, fid, dev_array);
	}
	SAFE_DEL_LOCAL_REF(dev_array);
	SAFE_DEL_LOCAL_REF(class_dev_info);
	return j;
	
}

JNIEXPORT jobject JNICALL
NAME(ClUserGetInfo)(JNIEnv* env, jobject this, jint user_handle)
{
	cl_user_t *info;
	jclass class_user_info;
	jobject obj_user_info;

	if ((info = cl_user_get_info(user_handle)) == NULL)
		return NULL;
	class_user_info = (*env)->FindClass(env, CLASS_USER_INFO);
	obj_user_info = (*env)->AllocObject(env, class_user_info);

	jni_assert(class_user_info != NULL);
	jni_assert(obj_user_info != NULL);
	CopyUserInfoBase(env, class_user_info, obj_user_info, info);
	LOGE("ClUserGetInfo  ----num_dev = %d",info->num_dev);
	int num_dev = CopyDevInfo(env, class_user_info, obj_user_info, info);
	jfieldID fid = (*env)->GetFieldID(env, class_user_info, "num_dev", "I");
	(*env)->SetIntField(env, obj_user_info, fid, num_dev);
	LOGE("ClUserGetInfo  ##########num_dev = %d",num_dev);
	cl_user_free_info(info);
	SAFE_DEL_LOCAL_REF(class_user_info);
	
	return obj_user_info;
}

JNIEXPORT jint JNICALL
NAME(ClUserBackground)(JNIEnv* env, jobject this, jint user_handle, jboolean background)
{
	return cl_user_background(user_handle, background);
}

JNIEXPORT jobject JNICALL
NAME(ClEnvMonitorGetInfo)(JNIEnv* env, jobject this, jint user_handle)
{
	cl_env_mon_t* emp;
	jclass obj_class;
	jobject em_info;

	if((emp = cl_em_get_info(user_handle))==NULL)
			return NULL;

	obj_class = (*env)->FindClass(env, CLASS_ENVMONITORINFO);
	jni_assert(obj_class != NULL);
	em_info = (*env)->AllocObject(env, obj_class);
	jni_assert(em_info != NULL);

	jniCopyLongValue(env,obj_class,"sn",em_info,emp->sn);
	jniCopyBooleanValue(env,obj_class,"get_air_done",em_info,emp->get_air_done);
	jniCopyBooleanValue(env,obj_class,"get_weather_done",em_info,emp->get_weather_done);
	jniCopyIntValue(env,obj_class,"aqi",em_info,emp->aqi);
	jniCopyIntValue(env,obj_class,"pm25",em_info,emp->pm25);

	jniCopyString(env,obj_class,"air_quality",em_info,emp->air_quality);
	jniCopyString(env,obj_class,"city",em_info,emp->city);

	jniCopyIntValue(env,obj_class,"temperature",em_info,emp->temperature);
	jniCopyIntValue(env,obj_class,"temp_max",em_info,emp->temp_max);
	jniCopyIntValue(env,obj_class,"temp_min",em_info,emp->temp_min);
	jniCopyIntValue(env,obj_class,"humidity",em_info,emp->humidity);

	jniCopyString(env,obj_class,"weather",em_info,emp->weather);
	
	jniCopyString(env,obj_class,"aqi_up_time",em_info,emp->aqi_last_update_time);
	jniCopyString(env,obj_class,"weather_up_time",em_info,emp->wear_last_update_time);

	if(emp ->suggest_temp != NULL) {
		jniCopyString(env,obj_class,"suggest_temp",em_info,emp->suggest_temp);
	}

	if(emp ->suggest_humidity != NULL) {
		jniCopyString(env,obj_class,"suggest_humidity",em_info,emp->suggest_humidity);
	}

	if(emp ->suggest_air != NULL) {
		jniCopyString(env,obj_class,"suggest_air",em_info,emp->suggest_air);
	}
	
	
	(*env)->DeleteLocalRef(env, obj_class);
	cl_em_free_info(emp);

	return em_info;
}

JNIEXPORT jobjectArray JNICALL
NAME(ClEnvGetCityList)(JNIEnv* env, jobject this)
{
	cl_city_list_t* city_list;
	jclass obj_class;
	jobject province;
	jobjectArray provArray;
	jobjectArray cityArray;
	int i,j;
	jfieldID fid;
	jstring str;
	cl_province_t* cl_prov;
	jclass class_string;

	if((city_list = cl_em_get_city_list())==NULL||city_list->province_num<=0)
		return NULL;

	obj_class = (*env)->FindClass(env, CLASS_ENVPROVINCEINFO);
	provArray = (*env)->NewObjectArray(env, city_list->province_num, obj_class, NULL);
	class_string = (*env)->FindClass(env, "java/lang/String");

	for(i=0; i<city_list->province_num ; i++){
		cl_prov = city_list->province[i];
		province = (*env)->AllocObject(env, obj_class);//省锟斤拷

		//锟斤拷锟?
		jniCopyString(env, obj_class, "provinceName", province, cl_prov->name);

		char* city;
		//锟斤拷锟斤拷
		if(cl_prov->city_num > 0){
			cityArray = (*env)->NewObjectArray(env, cl_prov->city_num, class_string, NULL);
			for(j=0; j<cl_prov->city_num; j++){
				city =  cl_prov->city[j]==NULL?"":cl_prov->city[j];
				doString(&city);
				str = (*env)->NewStringUTF(env, city);
				(*env)->SetObjectArrayElement(env, cityArray, j, str);
				(*env)->DeleteLocalRef(env, str);
			}
			fid = (*env)->GetFieldID(env, obj_class, "cityArray", "[Ljava/lang/String;");
			(*env)->SetObjectField(env, province, fid, cityArray);
			(*env)->DeleteLocalRef(env, cityArray);
		}

		(*env)->SetObjectArrayElement(env, provArray, i, province);
		(*env)->DeleteLocalRef(env, province);
	}

	(*env)->DeleteLocalRef(env, obj_class);
	cl_em_free_city_list(city_list);

	return provArray;
}

JNIEXPORT jint JNICALL
NAME(ClEnvSetCity)(JNIEnv* env, jobject this, jint user_handle, jstring city)
{
	const char *str_name;
	RS ret = RS_ERROR;

	str_name = (*env)->GetStringUTFChars(env, city, NULL);
	ret = cl_em_set_city(user_handle,str_name);

	(*env)->ReleaseStringUTFChars(env, city, str_name);

	return ret;
}

JNIEXPORT jobject JNICALL
NAME(ClUpdateGetNewestVersion)(JNIEnv* env, jobject this)
{
	return UpdateGetNewestVersion(env);
}

JNIEXPORT jint JNICALL
NAME(ClCheckUpgradeVersion)(JNIEnv* env,jint slave_handle, jint lang)
{
	return cl_dev_upgrade_check(slave_handle, lang);
}

JNIEXPORT jint JNICALL
NAME(ClDevUpdateCli)(JNIEnv* env, jobject this, jint slave_handle, jstring filename)
{
	char *str_filename;
	RS ret = RS_ERROR;
	str_filename = (char *)(*env)->GetStringUTFChars(env, filename, NULL);
	ret =  cl_dev_update_cli(slave_handle, str_filename);

	(*env)->ReleaseStringUTFChars(env, filename, str_filename);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClDevStmUpdateCli)(JNIEnv* env, jobject this, jint slave_handle, jstring filename)
{
	char *str_filename;
	RS ret = RS_ERROR;
	str_filename = (char *)(*env)->GetStringUTFChars(env, filename, NULL);
	ret =  cl_dev_stm_update_cli(slave_handle, str_filename);

	(*env)->ReleaseStringUTFChars(env, filename, str_filename);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClIntellForwordQuery)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_intelligent_forward_query(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClEMGetSuggest)(JNIEnv* env, jobject this, jint user_handle, jobject envir)
{	
	env_air_t env_t = {0};	
	jfieldID fid;	
	jclass obj_class ;	
	
	if(env == NULL) 	
		return -5; //无效参数

	obj_class = (*env)->FindClass(env, CLASS_ENVAIR);	
	jni_assert(obj_class != NULL);	

	jniCopyIntValue(env,obj_class,"temp",envir,env_t.temp);	
	jniCopyIntValue(env,obj_class,"humidity",envir,env_t.humidity);	
	jniCopyIntValue(env,obj_class,"voc",envir,env_t.voc);
	jniCopyIntValue(env,obj_class,"pm25",envir,env_t.pm25);
	jniCopyIntValue(env,obj_class,"pm10",envir,env_t.pm10);	
	jniCopyIntValue(env,obj_class,"no2",envir,env_t.no2);	
	jniCopyIntValue(env,obj_class,"so2",envir,env_t.so2);	
	jniCopyIntValue(env,obj_class,"o3",envir,env_t.o3);
	jniCopyIntValue(env,obj_class,"co",envir,env_t.co);	

	(*env)->DeleteLocalRef(env, obj_class);	
	
	return cl_em_get_suggest(user_handle, &env_t);	
}
/*分享，二维码添加设备*/
JNIEXPORT jint JNICALL
NAME(ClUserQrLogin)(JNIEnv* env, jobject this, jobject obj_user, jstring sn, jstring qr_code, jint callback_handle)
{
	cl_handle_t user_handle;
	const char *str_sn = NULL;
	const char *str_qr_code = NULL;
	RS ret = RS_ERROR;
	jclass clazz = NULL;
	jfieldID fid;

	if (obj_user == NULL) {
		goto done;
	}
	if (sn != NULL) {
		str_sn = (*env)->GetStringUTFChars(env, sn, NULL);
	}
	if (qr_code != NULL) {
		str_qr_code = (*env)->GetStringUTFChars(env, qr_code, NULL);
	}
	ret = cl_user_QR_login(&user_handle, (char*)str_sn, str_qr_code, nactivCallback, (void *)(jlong)callback_handle);
	(*env)->ReleaseStringUTFChars(env, sn, str_sn);
	(*env)->ReleaseStringUTFChars(env, qr_code, str_qr_code); 

	// save user handle in User.UserHandle
	clazz = (*env)->FindClass(env, CLASS_USER_INFO);
	fid = (*env)->GetFieldID(env, clazz, "UserHandle", "I");
	(*env)->SetIntField(env, obj_user, fid, user_handle);
done:
	SAFE_DEL_LOCAL_REF(clazz);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClPhoneUserAddQrDev)(JNIEnv* env, jobject this, jint user_handle, jstring sn, jstring qr_code)
{
	RS ret = RS_ERROR;
	const char *str_sn = NULL;
	const char *str_qr_code = NULL;
	if (sn != NULL) {
		str_sn = (*env)->GetStringUTFChars(env, sn, NULL);
	}
	if (qr_code != NULL) {
		str_qr_code = (*env)->GetStringUTFChars(env, qr_code, NULL);
	}
	
	ret = cl_user_add_QR_dev(user_handle, str_sn, str_qr_code);
	
	(*env)->ReleaseStringUTFChars(env, sn, str_sn);
	(*env)->ReleaseStringUTFChars(env, qr_code, str_qr_code);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClReqShareCode)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_com_udp_request_share_code(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClModSharePhone)(JNIEnv* env, jobject this, jint dev_handle, jint index, jstring desc)
{
	const char *c_desc = NULL;
	int ret  = RS_OK;
	if (desc != NULL) {
		c_desc = (*env)->GetStringUTFChars(env, desc, NULL);
	}
	ret = cl_com_udp_modify_shared_phone(dev_handle,index, (char*)c_desc);
	(*env)->ReleaseStringUTFChars(env, desc, c_desc);
	return ret;
}


JNIEXPORT jint JNICALL
NAME(ClDelSharePhone)(JNIEnv* env, jobject this, jint dev_handle, jint index)
{
	return cl_com_udp_del_shared_phone(dev_handle, index);
}

JNIEXPORT jint JNICALL
NAME(ClRefreshShareList)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_com_udp_refresh_shard_list(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClRfSetAlarmTime)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jint time)
{
	return cl_rf_dev_set_alarm_time(dev_handle, type, time);
}

JNIEXPORT jint JNICALL
NAME(ClRfSetAlarmClc)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_rf_dev_set_alarm_clc(dev_handle);
}


static void assign_timer_ext_advance(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{
	jclass class_ext = NULL;
	jobject obj_ext = NULL;
	jfieldID fid = NULL;

	class_ext = (*env)->FindClass(env, CLASS_COMM_TIMER_EXT_ZYKT);
	
	fid = (*env)->GetFieldID(env, class_timer, "extInfo", "L"CLASS_JAVA_OBJECT";");
	if (fid == NULL) {
		return;
	}
	obj_ext = (*env)->GetObjectField(env, obj_timer, fid);
	jni_assign_simple_struct(env, obj_ext, class_ext,
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.zykt_timer, mode),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.zykt_timer, tmp),
		                        JNI_VAR_ARG_END);
	SAFE_DEL_LOCAL_REF(class_ext);
}

static void assign_rf_timer_ext_info(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer) {
	
	
	switch(timer->type) {
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		assign_timer_ext_advance(env, class_timer, obj_timer, timer);
		break;
	default:
		break;
	}
}

static void assign_heating_valve_timer(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, tmp_int),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, tmp_dec),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, start_tmp_int),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, start_tmp_dec),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, end_tmp_int),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, end_tmp_dec),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, max_tmp_int),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, max_tmp_dec),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, min_tmp_int),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.hv_timer, min_tmp_dec),
		                        JNI_VAR_ARG_END);
}
static void assign_rfwukong_valve_timer(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.wkair_timer, mode),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.wkair_timer, tmp),
		                        JNI_VAR_ARG_END);
}

static void assign_zh_motor_timer(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.zhdj_timer, location),
		                        JNI_VAR_ARG_END);
}

static void assign_lk_thermostat_timer(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{	
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.linkon_timer, run_mode),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.linkon_timer, wind_speed),
		                        ASSIGN_TRIPLES(short, &timer->extended_data_u.linkon_timer, tmp),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.linkon_timer, scene_mode),
		                        JNI_VAR_ARG_END);
}

static void assign_Ml_DHX_timer(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{	
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(int, &timer->extended_data_u.dhxml_timer, on_off_stat),
		                        JNI_VAR_ARG_END);
}

static void assign_ZH_DHX_timer(JNIEnv *env, jclass class_timer, jobject obj_timer, cl_comm_timer_t *timer)
{	
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.zhdhx_timer, onoff),
		                        ASSIGN_TRIPLES(byte, &timer->extended_data_u.zhdhx_timer, mask),
		                        JNI_VAR_ARG_END);
}

static void assign_comm_timer_extend(JNIEnv *env, jclass class_timer, jobject obj_timer, int timer_type, cl_comm_timer_t *timer)
{
	switch(timer_type) {
	case TIMER_TYPE_HEATING_VALVE:
		assign_heating_valve_timer(env, class_timer, obj_timer, timer);
		break;
	case TIMER_TYPE_RF_WUKONG:
		assign_rfwukong_valve_timer(env, class_timer, obj_timer, timer);
		break;
	case TIMER_TYPE_ZH_MOTOR:
		assign_zh_motor_timer(env, class_timer, obj_timer, timer);
		break;
	case TIMER_LK_THERMOSTAT:
		assign_lk_thermostat_timer(env, class_timer, obj_timer, timer);
		break;
	case TIMER_ML_DHXML:
		assign_Ml_DHX_timer(env, class_timer, obj_timer, timer);
		break;
	case TIMER_ZH_DHX:
		assign_ZH_DHX_timer(env, class_timer, obj_timer, timer);
		break;
	default:
		break;
	}
}

JNIEXPORT jint JNICALL
NAME(ClRfTimerSet)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_timer, jint type)
{
	jclass class_timer = NULL;
	cl_comm_timer_t c_timer;
	cl_slave_t *slave = NULL;
	cl_dev_info_t *dev = NULL;
	int (*timer_func)(cl_handle_t, cl_comm_timer_t *) = NULL;
	int sub_type = 0;
	int ext_type = 0;
	int tdev_type = 0;
	int timer_type = 0;

	class_timer = (*env)->GetObjectClass(env, obj_timer);
	jni_assign_simple_struct(env, obj_timer, class_timer,
		                                    ASSIGN_TRIPLES(byte, &c_timer, id),
		                                    ASSIGN_TRIPLES(boolean, &c_timer, enable),
		                                    ASSIGN_TRIPLES(byte, &c_timer, type),
		                                    ASSIGN_TRIPLES(byte, &c_timer, hour),
		                                    ASSIGN_TRIPLES(byte, &c_timer, min),
		                                    ASSIGN_TRIPLES(byte, &c_timer, week),
		                                    ASSIGN_TRIPLES(short, &c_timer, duration),
		                                    JNI_VAR_ARG_END);

	if (type == 0) {
		slave = cl_get_slave_info(dev_handle);
		if (slave != NULL) {
			sub_type = slave->dev_type;
			ext_type = slave->ext_type;
		}
		timer_func = cl_rfdef_comm_timer_modify_add;
	} else {
		dev = cl_user_get_dev_info(dev_handle);
		if (dev != NULL) {
			sub_type = dev->sub_type;
			ext_type = dev->ext_type;
		}
		timer_func = cl_misc_comm_timer_modify_add;
	}

	tdev_type = dev_type_map(sub_type,ext_type);
	timer_type = timer_type_map(tdev_type, c_timer.type);
	if (timer_type == TIMER_TYPE_CENTRAL_AIRCON) {
		assign_rf_timer_ext_info(env, class_timer, obj_timer, &c_timer);
	} else {
		assign_comm_timer_extend(env, class_timer, obj_timer, timer_type, &c_timer);	
	}
	if (slave != NULL) {
		cl_free_slave_info(slave);
	}
	if (dev != NULL) {
		cl_user_free_dev_info(dev);
	}
	return timer_func(dev_handle, &c_timer);
}

JNIEXPORT jint JNICALL
NAME(ClRfTimerDel)(JNIEnv* env, jobject this, jint dev_handle, jbyte timer_id, jint type)
{
	if (type == 0) {
		return cl_rfdev_comm_timer_del(dev_handle, timer_id);
	} else {
		return cl_misc_comm_timer_del(dev_handle, timer_id);
	}
}

JNIEXPORT jint JNICALL
NAME(ClRfTimerRefresh)(JNIEnv* env, jobject this, jint dev_handle, jbyte timer_id)
{
	return cl_rfdev_comm_timer_query(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClHtlAdminLogin)(JNIEnv* env, jobject this, jint dev_handle, jstring pwd)
{
	const char *ppwd = NULL;
	htllock_tt_admin_login_t pwdParam = {0};

	ppwd = (*env)->GetStringUTFChars(env, pwd, NULL);
	if (ppwd != NULL) {
		strncpy(pwdParam.pwd, ppwd, sizeof(pwdParam.pwd));
		(*env)->ReleaseStringUTFChars(env, pwd, ppwd);
	}
	
	return cl_rf_htllock_admin_login(dev_handle, &pwdParam);
}

JNIEXPORT jint JNICALL
NAME(ClHtlUserManageSetName)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_name)
{
	jclass class_name = NULL;
	htllock_tt_user_manage_set_name_t c_name;

	class_name = (*env)->GetObjectClass(env, obj_name);
	jni_assign_simple_struct(env, obj_name, class_name,
		                        ASSIGN_TRIPLES(byte , &c_name, op),
		                        ASSIGN_TRIPLES(short , &c_name, index),
		                        ASSIGN_QUAD(String , &c_name, name, 12),
		                        JNI_VAR_ARG_END);
	return cl_rf_htllock_user_manage_set_name(dev_handle, &c_name);
}

JNIEXPORT jint JNICALL
NAME(ClHtlUserManageSetPic)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_pic)
{
	jclass class_pic = NULL;
	htllock_tt_user_manage_set_pic_t c_pic;

	class_pic = (*env)->GetObjectClass(env, obj_pic);
	jni_assign_simple_struct(env, obj_pic, class_pic,
		                        ASSIGN_TRIPLES(byte, &c_pic, op),
		                        ASSIGN_TRIPLES(byte, &c_pic, pic_id),
		                        ASSIGN_TRIPLES(short, &c_pic, index),
		                        JNI_VAR_ARG_END);
	return cl_rf_htllock_user_manage_set_pic(dev_handle, &c_pic);
}

JNIEXPORT jint JNICALL
NAME(ClHtlUserManageSetBind)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_bind)
{
	jclass class_bind = NULL;
	htllock_tt_user_manage_set_bind_t c_bind;

	class_bind = (*env)->GetObjectClass(env, obj_bind);
	jni_assign_simple_struct(env, obj_bind, class_bind,
		                        ASSIGN_TRIPLES(byte, &c_bind, op),
		                        ASSIGN_TRIPLES(short, &c_bind, index),
		                        ASSIGN_TRIPLES(short, &c_bind, cindex),
		                        JNI_VAR_ARG_END);
	return cl_rf_htllock_user_manage_set_bind(dev_handle, &c_bind);
}

JNIEXPORT jint JNICALL
NAME(ClHtlUserManageSetUnbind)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_bind)
{
	jclass class_bind = NULL;
	htllock_tt_user_manage_set_bind_t c_bind;

	class_bind = (*env)->GetObjectClass(env, obj_bind);
	jni_assign_simple_struct(env, obj_bind, class_bind,
		                        ASSIGN_TRIPLES(byte, &c_bind, op),
		                        ASSIGN_TRIPLES(short, &c_bind, index),
		                        ASSIGN_TRIPLES(short, &c_bind, cindex),
		                        JNI_VAR_ARG_END);
	return cl_rf_htllock_user_manage_set_unbind(dev_handle, &c_bind);
}

JNIEXPORT jint JNICALL
NAME(ClHtlNoticeSet)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_notice)
{
	jclass class_notice = NULL;
	htllock_tt_info_notice_set_t c_notice;

	class_notice = (*env)->GetObjectClass(env, obj_notice);
	jni_assign_simple_struct(env, obj_notice, class_notice,
		                        ASSIGN_TRIPLES(byte, &c_notice, op),
		                        ASSIGN_TRIPLES(byte, &c_notice, sbit_temp),
		                        JNI_VAR_ARG_END);
	return cl_rf_htllock_notice_info_set(dev_handle, &c_notice);
}

JNIEXPORT jint JNICALL
NAME(ClHtlSetPin)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_pin)
{
	jclass class_pin = NULL;
	htllock_tt_set_pin_t c_pin;

	class_pin = (*env)->GetObjectClass(env, obj_pin);
	jni_assign_simple_struct(env, obj_pin, class_pin,
		                        ASSIGN_TRIPLES(short , &c_pin, time),
		                        ASSIGN_TRIPLES(byte , &c_pin, cnt),
		                        ASSIGN_QUAD(String , &c_pin, pwd, 6),
		                        JNI_VAR_ARG_END);
	c_pin.pwd_len = 6;
	return cl_rf_htllock_set_pin(dev_handle,&c_pin);
}

JNIEXPORT jint JNICALL
NAME(ClHtlHistoryReq)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_history)
{
	jclass class_history = NULL;
	htllock_tt_info_notice_get_history_t c_history;

	class_history = (*env)->GetObjectClass(env, obj_history);
	jni_assign_simple_struct(env, obj_history, class_history,
		                        ASSIGN_TRIPLES(byte, &c_history, op),
		                        ASSIGN_TRIPLES(int, &c_history, timestamp),
		                        JNI_VAR_ARG_END);
	return cl_rf_htllock_request_history(dev_handle, &c_history);
}

JNIEXPORT jint JNICALL
NAME(clHtlUserManagerSetRemind)(JNIEnv* env, jobject this, jint dev_handle, jboolean close, jshort user_index)
{
	return cl_rf_htllock_user_manage_set_remind_onoff(dev_handle, close, user_index);
}

JNIEXPORT jint JNICALL
NAME(ClHtlSampleCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_rf_htllock_sample_ctrl(dev_handle, action, value);
}



JNIEXPORT jint JNICALL
NAME(ClRfSetDefenseBatch)(JNIEnv* env, jobject this, jint master_handle, jboolean defense)
{
	return cl_rf_dev_set_defense_batch(master_handle, defense);
}

JNIEXPORT jint JNICALL
NAME(ClCommSetTempCtrl)(JNIEnv* env, jobject this, jint dev_handle, jobject temp_ctrl)
{
	jclass class_temp_ctrl = NULL;
	cl_temp_ac_ctrl_t c_temp_ctrl;

	class_temp_ctrl = (*env)->GetObjectClass(env, temp_ctrl);
	jni_assign_simple_struct(env, temp_ctrl, class_temp_ctrl,
		                        ASSIGN_TRIPLES(boolean, &c_temp_ctrl, enable),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, mode),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, temp_min),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, temp_max),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, week),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, begin_hour),
		                        ASSIGN_TRIPLES(byte, &c_temp_ctrl, end_hour),
		                        JNI_VAR_ARG_END);

	return cl_sa_public_set_temp_ctrl(dev_handle, &c_temp_ctrl);
}


JNIEXPORT jint JNICALL
NAME(ClCommSetTempCurv)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_curve)
{
	jclass class_curve = NULL, class_point = NULL;
	jobject obj_point = NULL;
	jobjectArray array_point = NULL;
	cl_temp_curve_t *c_curve = NULL;
	tmp_curve_t *c_point = NULL;
	jbyte jcount;
	jfieldID fid = NULL;
	int i = 0;
	int ret = 0;

	class_curve = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CURVE);
	class_point = (*env)->FindClass(env, CLASS_AIR_PLUG_TEMP_CURVE_LINE);
	fid = (*env)->GetFieldID(env, class_curve, "count", "B");
	jcount = (*env)->GetByteField(env, obj_curve, fid);
	c_curve = malloc(sizeof(cl_temp_curve_t) + jcount * sizeof(tmp_curve_t));
	
	jni_assign_simple_struct(env, obj_curve, class_curve,
		                        ASSIGN_TRIPLES(byte, c_curve, id),
		                        ASSIGN_TRIPLES(boolean, c_curve, enable),
		                        ASSIGN_TRIPLES(byte, c_curve, week),
		                        ASSIGN_TRIPLES(byte, c_curve, begin_hour),
		                        ASSIGN_TRIPLES(byte, c_curve, end_hour),
		                        ASSIGN_TRIPLES(byte, c_curve, time_period),
		                        ASSIGN_TRIPLES(byte, c_curve, count),
		                        JNI_VAR_ARG_END);
	if (jcount > 0) {
		fid = (*env)->GetFieldID(env, class_curve, "curves", "[L"CLASS_AIR_PLUG_TEMP_CURVE_LINE";");
		array_point = (*env)->GetObjectField(env, obj_curve, fid);
		c_point = (tmp_curve_t*)(c_curve + 1);
		for (i = 0; i < jcount; ++i) {
			obj_point = (*env)->GetObjectArrayElement(env, array_point, i);
			jni_assign_simple_struct(env, obj_point, class_point,
				                        ASSIGN_TRIPLES(byte, c_point, flag),
				                        ASSIGN_TRIPLES(byte, c_point, tmp),
				                        ASSIGN_TRIPLES(byte, c_point, wind),
				                        ASSIGN_TRIPLES(byte, c_point, dir),
				                        JNI_VAR_ARG_END);
			++c_point;
		}
	}

	ret = cl_sa_public_modify_temp_curve(dev_handle, c_curve);

	free(c_curve);
	SAFE_DEL_LOCAL_REF(class_curve);
	SAFE_DEL_LOCAL_REF(class_point);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSaPublicHistoryQuery)(JNIEnv* env, jobject this, jint dev_handle, jint index, jint num)
{
	return cl_sa_public_history_query(dev_handle, index, num);
}

JNIEXPORT jint JNICALL
NAME(ClSaPublicSetBootTemp)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable, jbyte temp)
{
	return cl_sa_public_set_boot_temp(dev_handle, enable, temp);
}

JNIEXPORT jint JNICALL
NAME(RfWukongSetAddr)(JNIEnv* env, jobject this, jint dev_handle, jbyte addr)
{
	return cl_set_air_ir_addr(dev_handle, addr);
}

JNIEXPORT jint JNICALL
NAME(RfWukongSetAirIrSyncOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_set_air_ir_sync_onoff(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClRfAirIrCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jbyte value)
{
	return cl_rf_dev_air_ir_ctrl(dev_handle, type, value);
}

JNIEXPORT jobject JNICALL
NAME(ClRfAirGetPairStat)(JNIEnv* env, jobject this, jint dev_handle)
{
	cl_rf_air_code_match_stat_t stat ;
	jclass class_pair =NULL;
	jobject obj_pair = NULL;
	
	if (cl_get_air_code_match_stat(dev_handle, &stat) == RS_OK) {
		class_pair = (*env)->FindClass(env, CLASS_RF_WK_PAIR);
		obj_pair = (*env)->AllocObject(env, class_pair);

		jni_copy_simple_class(env, class_pair, obj_pair,
				                 TRIPLES(byte, &stat, cur_step),
			                     TRIPLES(byte, &stat, max_step),
			                     TRIPLES(byte, &stat, error),
			                     TRIPLES(short, &stat, matched_id),
			                     TRIPLES(short, &stat, num),
			                     QUADRUPLE(short[], &stat, matched_ids, 32),
			                     JNI_VAR_ARG_END);
		SAFE_DEL_LOCAL_REF(class_pair);
	}
	return obj_pair;
}

JNIEXPORT jint JNICALL
NAME(ClHeatingValvePeriodCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte day, jbyte hh1,
                                jbyte hm1, jbyte eh1, jbyte em1, jbyte hh2, jbyte hm2, jbyte eh2, jbyte em2)
{
	return cl_rf_heating_valve_period_ctrl(dev_handle, day, hh1, hm1, eh1, em1, hh2, hm2, eh2, em2);
}

JNIEXPORT jobject JNICALL
NAME(ClGetSlaveInfo)(JNIEnv* env, jobject this, jint slave_handle)
{
	cl_slave_t* slave;
	jobject obj_slave = NULL;

	slave = cl_get_slave_info(slave_handle);
	if(!slave){
		return NULL;
	}
	obj_slave = CopySlave(env,slave);
	cl_free_slave_info(slave);
	
	return obj_slave;
}

JNIEXPORT jint JNICALL
NAME(ClRfCommSetShortcutOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean enable, jboolean onoff, jint time)
{
	return cl_rfdev_public_set_shortcuts_onoff(dev_handle, enable, onoff, time);
}

JNIEXPORT jint JNICALL
NAME(ClRfCommQueryShortcutOnoff)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_rfdev_public_query_shortcuts_onoff(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClRfCommQueryHistroy)(JNIEnv* env, jobject this, jint dev_handle, jint index)
{
	return cl_rfdef_comm_history_query(dev_handle, index);
}

JNIEXPORT jint JNICALL
NAME(ClUserApnsConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_config)
{
	jclass class_config = NULL;
	jfieldID fid = NULL;
	cl_apns_config_t c_config = {0};


	class_config = (*env)->GetObjectClass(env, obj_config);
	jni_assign_simple_struct(env, obj_config, class_config,
		                        ASSIGN_TRIPLES(byte, &c_config, action),
		                        ASSIGN_TRIPLES(boolean, &c_config, need_push),
		                        //ASSIGN_TRIPLES(byte, &c_config, cert_id),//ios
		                        //ASSIGN_TRIPLES(byte, &c_config, token_len),//ios
		                        ASSIGN_QUAD(String, &c_config, phone_ver, 8),
		                        //ASSIGN_QUAD(byte[], &c_config, token, 64), //ios
		                        ASSIGN_QUAD(String, &c_config, msg_prefix, 64),
		                        ASSIGN_QUAD(String, &c_config, language, 64),
		                        //ASSIGN_QUAD(String, &c_config, push_music, 256),
		                        ASSIGN_QUAD(String, &c_config, mipush_packname, 64),
		                        ASSIGN_QUAD(String,&c_config, regid, 256),
		                        JNI_VAR_ARG_END);
	SAFE_DEL_LOCAL_REF(class_config);
	return cl_user_apns_config(dev_handle, &c_config);
}

JNIEXPORT jobject JNICALL
NAME(ClGetUserApnsConfig)(JNIEnv* env, jobject this, jint dev_handle)
{
	jclass class_config = NULL;
	jobject obj_config = NULL;
	jfieldID fid = NULL;
	cl_apns_config_t c_config;

	if (cl_user_apns_config_get(dev_handle, &c_config) != RS_OK) {
		return NULL;
	}

	class_config = (*env)->FindClass(env, CLASS_XM_PUSH_CONFIG);
	obj_config = (*env)->AllocObject(env, class_config);

	jni_copy_simple_class(env, class_config, obj_config,
		                               TRIPLES(byte,&c_config, action),
		                               TRIPLES(boolean,&c_config, need_push),
		                               //TRIPLES(byte,&c_config, cert_id), //ios
		                               //TRIPLES(byte,&c_config, token_len), //ios
		                               TRIPLES(String,&c_config, phone_ver),
		                               //QUADRUPLE(byte[],&c_config, token, 64), //ios
		                               TRIPLES(String,&c_config, msg_prefix),
		                               TRIPLES(String,&c_config, language),
		                               //TRIPLES(String,&c_config, push_music),
		                               TRIPLES(String,&c_config, mipush_packname),
		                               TRIPLES(String,&c_config, regid),
		                               JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(class_config);
	return obj_config;
}

JNIEXPORT jint JNICALL
NAME(RfWukongTempAdjust)(JNIEnv* env, jobject this, jint dev_handle, jbyte temp)
{
	return cl_set_air_code_tmp_adjust(dev_handle, temp);
}

JNIEXPORT jint JNICALL
NAME(RfWukongLedMode)(JNIEnv* env, jobject this, jint dev_handle, jbyte mode)
{
	return cl_set_air_code_led_mode(dev_handle, mode);
}

JNIEXPORT jint JNICALL
NAME(RfWukongCodeDir)(JNIEnv* env, jobject this, jint dev_handle, jbyteArray dir)
{
	jsize len  = (*env)->GetArrayLength(env, dir);
	u_int8_t *c_dir = NULL;
	int ret = RS_OK;
	
	if(len < 0){
		return RS_INVALID_PARAM;
	}
	c_dir = (*env)->GetByteArrayElements(env, dir, NULL);
	ret = cl_set_air_code_dir(dev_handle, c_dir, len);
	(*env)->ReleaseByteArrayElements(env, dir, c_dir, 0);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(RfWukongCodeDirAutoAdjust)(JNIEnv* env, jobject this, jint dev_handle, jbyte num, jbyte interval, jbyte timeout)
{
	return cl_set_air_code_dir_auto_adjust(dev_handle, num, interval, timeout);
}

JNIEXPORT jint JNICALL
NAME(RfWukongCodeCheck)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff, jbyte timeout)
{
	return cl_set_air_code_check(dev_handle, onoff, timeout);
}

JNIEXPORT jint JNICALL
NAME(RfWukongShockAutoCheck)(JNIEnv* env, jobject this, jint dev_handle, jint step, jboolean onoff)
{
	return cl_set_air_shock_auto_check(dev_handle, step, onoff);
}

JNIEXPORT jint JNICALL
NAME(RfWukongShockQuery)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_set_air_shock_status_query(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(RfWukongStatusQuery)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_set_air_ir_status_query(dev_handle);
}



/*智皇电机*/
JNIEXPORT jint JNICALL
NAME(ClZhMotorSetState)(JNIEnv* env, jobject this, jint dev_handle, jbyte state)
{
	return cl_zhdj_status_set(dev_handle, state);
}

JNIEXPORT jint JNICALL
NAME(ClZhMotorSetLocation)(JNIEnv* env, jobject this, jint dev_handle, jbyte location)
{
	return cl_zhdj_location_set(dev_handle, location);
}

JNIEXPORT jint JNICALL
NAME(ClZhMotorQueryLocation)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_zhdj_location_query(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClZhMotorBind)(JNIEnv* env, jobject this, jint dev_handle, jint magic, jbyte index, jbyte type)
{
	return cl_zhdj_bind(dev_handle, magic, index, type);
}

JNIEXPORT jint JNICALL
NAME(ClZhMotorTypeSet)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jbyte index)
{
	return cl_zhdj_type_set(dev_handle, type, index);
}
JNIEXPORT jint JNICALL
NAME(ClZhMotorDirSet)(JNIEnv* env, jobject this, jint dev_handle, jbyte dir)
{
	return cl_zhdj_dir_set(dev_handle, dir);
}

/**WiFi智皇窗帘*/
JNIEXPORT jint JNICALL
NAME(ClZhclSetStatus)(JNIEnv* env,jobject this,jint dev_handle,jbyte status)
{
	return cl_zhcl_status_set(dev_handle,status);
}

JNIEXPORT jint JNICALL
NAME(ClZhclSetLocation)(JNIEnv* env,jobject this,jint dev_handle,jbyte location)
{
	return cl_zhcl_location_set(dev_handle,location);
}

JNIEXPORT jint JNICALL
NAME(ClZhclBind)(JNIEnv* env,jobject this,jint dev_handle,jint magic, jbyte index, jbyte type)
{
return cl_zhcl_bind(dev_handle,magic,index,type);
}

JNIEXPORT jint JNICALL
NAME(ClZhclSetType)(JNIEnv* env,jobject this,jint dev_handle,jbyte type, jbyte index)
{
	return cl_zhcl_type_set(dev_handle,type,index);
}
JNIEXPORT jint JNICALL
NAME(ClZhclSetDir)(JNIEnv* env,jobject this,jint dev_handle, jbyte dir)
{
	return cl_zhcl_dir_set(dev_handle, dir);
}

/*电王科技*/
JNIEXPORT jint JNICALL
NAME(ClDwCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_dwkj_sample_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClDwTimerCtrl)(JNIEnv* env, jobject this, jint dev_handle, jobject timer)
{
	cl_dwkj_timer_t c_timer;
	cl_dwkj_timer_item_t *c_item = NULL;
	jclass class_timer = NULL;
	int i = 0;
	jclass class_item = NULL;
	jobject obj_item = NULL;
	jobjectArray item_array = NULL;
	jfieldID fid = NULL;
	jsize item_length = 0;

	class_timer = (*env)->GetObjectClass(env, timer);

	class_item = (*env)->FindClass(env, CLASS_DW_TIMER_ITEM);
	fid = (*env)->GetFieldID(env, class_timer, "item", "[L"CLASS_DW_TIMER_ITEM";");
	item_array = (*env)->GetObjectField(env, timer, fid);
	item_length = (*env)->GetArrayLength(env, item_array);

	jni_assign_simple_struct(env, timer, class_timer,
		                                    ASSIGN_TRIPLES(boolean, &c_timer, onoff),
		                                    JNI_VAR_ARG_END);
	c_timer.count = (u_int8_t)item_length;
	
	for (i = 0; i < item_length; ++i) {
		c_item = c_timer.item + i;
		obj_item = (*env)->GetObjectArrayElement(env, item_array, i);
		jni_assign_simple_struct(env, obj_item, class_item,
			                        ASSIGN_TRIPLES(short, c_item, point),
			                        ASSIGN_TRIPLES(byte, c_item, level),
			                        JNI_VAR_ARG_END);
		SAFE_DEL_LOCAL_REF(obj_item);
	}

	SAFE_DEL_LOCAL_REF(class_timer);
	SAFE_DEL_LOCAL_REF(class_item);
	SAFE_DEL_LOCAL_REF(item_array);

	return cl_dwkj_timer_ctrl(dev_handle, &c_timer);
}
/*情景遥控器*/
JNIEXPORT jint JNICALL
NAME(ClHmScenePanelSetKey)(JNIEnv* env, jobject this, jint dev_handle, jbyte id, jstring name)
{
	const char *c_name = NULL;
	int ret = RS_OK;

	c_name = (*env)->GetStringUTFChars(env, name, NULL);
	ret = cl_scene_controller_set_key(dev_handle, id, (char*)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClHmScenePanelCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_scene_controller_sample_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClLanuserCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_lanusers_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClRfJqSetThreshold)(JNIEnv* env, jobject this, jint dev_handle, jbyte type, jshort value)
{
	return cl_rf_jq_set_threshold(dev_handle, type, value);
}

JNIEXPORT jint JNICALL
NAME(ClRfJqSetAlarmPeriod)(JNIEnv* env, jobject this, jint dev_handle, jshort period)
{
	return cl_rf_jq_set_alarm_period(dev_handle, period);
}

JNIEXPORT jint JNICALL
NAME(ClRfJqFlushCh2o)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_rf_jq_flush_ch2o(dev_handle);
}


JNIEXPORT jint JNICALL
NAME(ClLanuserSetUerInfo)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_user)
{
	ucp_lanusers_manage_user_info_set_t c_user;
	jclass class_user = NULL;

	class_user = (*env)->GetObjectClass(env, obj_user);
	jni_assign_simple_struct(env, obj_user, class_user,
		                        ASSIGN_TRIPLES(int, &c_user, user_id),
		                        ASSIGN_QUAD(String, &c_user, model, sizeof(c_user.model)),
		                        JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(class_user);
	return cl_lanusers_set_user_info(dev_handle, &c_user);
}

/*夜狼声光报警*/
JNIEXPORT jint JNICALL
NAME(ClRfYlLightLampCtrl)(JNIEnv* env, jobject this, jint dev_handle, jobject lightstat)
{
	int ret;
	cl_rf_lamp_stat_t *lamp_info = get_rf_light_state(env, lightstat);
	ret = cl_rf_yllight_lamp_ctrl(dev_handle, lamp_info);
	
	SAFE_FREE(lamp_info);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClRfYlLightAlarmConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject ylAlarmConfig)
{
	cl_yllight_alarm_config_t yl_alarm_config;
	jclass class_yl_alarm = NULL;

	class_yl_alarm = (*env)->GetObjectClass(env, ylAlarmConfig);
	jni_assign_simple_struct(env, ylAlarmConfig, class_yl_alarm,
		                        ASSIGN_TRIPLES(byte, &yl_alarm_config, alarm_mode),
		                        ASSIGN_TRIPLES(short, &yl_alarm_config, on_time),
		                        ASSIGN_TRIPLES(int, &yl_alarm_config, off_time),
		                        ASSIGN_TRIPLES(int, &yl_alarm_config, total_time),
		                        JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(class_yl_alarm);
	
	return cl_rf_yllight_alarm_config(dev_handle, &yl_alarm_config);
}

JNIEXPORT jint JNICALL
NAME(ClRfYlLightComCtrl)(JNIEnv* env, jobject this, jint dev_handle, jbyte action, jint value)
{
	return cl_rf_yllight_com_ctrl(dev_handle, action, value);
}

JNIEXPORT jint JNICALL
NAME(ClUserSetClick)(JNIEnv* env, jobject this, jint dev_handle, jobject dev, jboolean isSlave)
{
	jclass class_dev = NULL;
	jfieldID fid = NULL;
	int ret = RS_OK;
	u_int8_t stat = 0;
	ret = cl_user_set_click(dev_handle, &stat);

	class_dev = (*env)->GetObjectClass(env, dev);
	if(isSlave){//rf设备
		fid = (*env)->GetFieldID(env, class_dev, "obj_status", "I");
		int obj_status = (*env)->GetIntField(env, dev, fid);
		if(ret != RS_OK && obj_status == BMS_BIND_ONLINE){
			stat = BMS_LOGINING;
			(*env)->SetIntField(env, dev, fid, stat);
		}
	}else{
		fid = (*env)->GetFieldID(env, class_dev, "display_stat", "B");
		(*env)->SetByteField(env, dev, fid, stat);
	}

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClCpScenePanelSetKey)(JNIEnv* env, jobject this, jint dev_handle, jbyte id, jstring name)
{
	const char *c_name = NULL;
	int ret = RS_OK;

	c_name = (*env)->GetStringUTFChars(env, name, NULL);
	ret = cl_set_cdqjmb_name(dev_handle, id, (char*)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSendDevelopPkt)(JNIEnv* env, jobject this, jint type, jlong ident, jbyteArray pkt_data)
{
	char* pkt_str = NULL;
	int len;
	
	len = (*env)->GetArrayLength(env, pkt_data); 
	//LOGI("ClSendDevelopPkt send: type = %d,ident = %"PRIu64", pkt_len = %d",type,ident,len);
	pkt_str = (char*)((*env)->GetByteArrayElements(env, pkt_data, NULL));
	
	return cl_send_pkt(type,ident,pkt_str,len);
}
JNIEXPORT jint JNICALL
NAME(ClInitDevelopCallback)(JNIEnv* env, jobject this)
{
	if(developInitMid(env) != RS_OK){
		return RS_ERROR;
	}
	return cl_set_proc_pkt_callback(developNativeCallback);
}

JNIEXPORT jint JNICALL
NAME(ClSetDeveloperId)(JNIEnv* env, jobject this,jstring developerId)
{
	char* develop_id = NULL;
	develop_id = (*env)->GetStringUTFChars(env, developerId, NULL);
	
	return cl_set_developer_id(develop_id);
}

//设备不支持，废弃该接口
/*JNIEXPORT jbyteArray JNICALL
NAME(ClGetDevelopPeerInfo)(JNIEnv* env, jobject this,jlong ident,jint peer_info_type)
{
	jbyte* buf;
	int len;
	jbyteArray jbyteArr;
	if(cl_get_peer_info(ident,peer_info_type,&buf,&len)!=RS_OK){
		LOGE("cl_get_peer_info failed.");
	}
	jbyteArr = (*env)->NewByteArray(env,len);
	(*env)->SetByteArrayRegion(env,jbyteArr,0,len,buf);
	cl_free_peer_info(buf);
	return jbyteArr;
}*/

JNIEXPORT jint JNICALL
NAME(ClZhdhxOnoff)(JNIEnv* env, jobject this,jint dev_handle, jbyte num, jboolean onoff)
{
	return cl_zhdhx_onoff(dev_handle, num, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClZhdhxKeyNameSet)(JNIEnv* env, jobject this, jint dev_handle, jbyte num, jstring name)
{
	const char *key_name = NULL;
	
	key_name = (*env)->GetStringUTFChars(env, name, NULL);
	return cl_zhdhx_key_name_set(dev_handle, num, key_name);
}

JNIEXPORT jint JNICALL
NAME(ClUpgradeSpecialDev)(JNIEnv* env, jobject this,jint gw_handle,jstring file_path,jlongArray arr_sn,jboolean force)
{
	int len,ret;
	u_int64_t * sn = NULL;
	char *filename = NULL;

	len = (*env)->GetArrayLength(env, arr_sn); 
	sn = (u_int64_t *)(*env)->GetLongArrayElements(env,arr_sn,NULL);
	
	filename = (char *)(*env)->GetStringUTFChars(env, file_path, NULL);

	//LOGI("zzzz sn length : %d,file path : %s",len,filename);
	
	jbyte isForce = force ? 0x2 : 0x1;
	ret = cl_dev_stm_update_spe_cli(gw_handle,filename,len,sn,isForce);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSetWanConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_config)
{
    jclass class_config = NULL;
	jclass class_item = NULL;
	jclass class_static = NULL;
	jclass class_dhcp = NULL;
	jclass class_pppoe = NULL;
	jobject obj_item = NULL;
	jobject obj_static = NULL;
	jobject obj_dhcp = NULL;
	jobject obj_pppoe = NULL;
	
	cl_wan_request_config_t c_wan_config;

	class_config= (*env)->GetObjectClass(env, obj_config);
	jni_assign_simple_struct(env, obj_config, class_config,
		                            ASSIGN_TRIPLES(byte, &c_wan_config, index),
		                            ASSIGN_TRIPLES(byte, &c_wan_config, wan_type),
		                            JNI_VAR_ARG_END);
	
	switch(c_wan_config.wan_type) {
		case 1:
			jni_assign_simple_struct(env, obj_config, class_config,
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_static, ip, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_static, mask, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_static, getway_ip, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_static, main_dns, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_static, sub_dns, 16),
				                     JNI_VAR_ARG_END);
			break;
		case 2:
			jni_assign_simple_struct(env, obj_config, class_config,
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_dhcp, ip, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_dhcp, mask, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_dhcp, getway_ip, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_dhcp, main_dns, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_dhcp, sub_dns, 16),
				                     JNI_VAR_ARG_END);
			break;
		case 3:
			jni_assign_simple_struct(env, obj_config, class_config,
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_pppoe, ip, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_pppoe, peer_ip, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_pppoe, name, 32),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_pppoe, pwd, 32),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_pppoe, main_dns, 16),
				                     ASSIGN_QUAD(String, &c_wan_config.config.config_pppoe, sub_dns, 16),
				                     JNI_VAR_ARG_END);
			break;
		default:
			break;
	}

	SAFE_DEL_LOCAL_REF(class_config);
	return cl_sa_public_set_wan_config(dev_handle, &c_wan_config);
}

JNIEXPORT jint JNICALL
NAME(ClSetDhcpServerConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_config)
{
    jclass class_config = NULL;
	
	cl_dhcp_server_config_t c_dhcp_config;

	class_config= (*env)->GetObjectClass(env, obj_config);
	jni_assign_simple_struct(env, obj_config, class_config,
		                            ASSIGN_QUAD(String, &c_dhcp_config, getway_ip,16),
		                            ASSIGN_QUAD(String, &c_dhcp_config, start_ip,16),
		                            ASSIGN_QUAD(String, &c_dhcp_config, end_ip,16),
		                            ASSIGN_QUAD(String, &c_dhcp_config, mask,16),
		                            ASSIGN_TRIPLES(int, &c_dhcp_config, time),
		                            JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(class_config);
	return cl_sa_public_set_dhcp_server_config(dev_handle, &c_dhcp_config);
}

JNIEXPORT jint JNICALL
NAME(ClSetApConfig)(JNIEnv* env, jobject this, jint dev_handle, jobject obj_config)
{
    jclass class_config = NULL;
	
	cl_ap_config_t c_ap_config;

	class_config= (*env)->GetObjectClass(env, obj_config);
	jni_assign_simple_struct(env, obj_config, class_config,
		                            ASSIGN_QUAD(String, &c_ap_config, ssid,36),
		                            ASSIGN_QUAD(String, &c_ap_config, pwd,68),
		                            ASSIGN_TRIPLES(byte, &c_ap_config, is_enc),
		                            ASSIGN_TRIPLES(byte, &c_ap_config, enable),
		                            ASSIGN_TRIPLES(byte, &c_ap_config, pow),
		                            ASSIGN_TRIPLES(byte, &c_ap_config, channle_mode),
		                            ASSIGN_TRIPLES(byte, &c_ap_config, channel),
		                            JNI_VAR_ARG_END);

	SAFE_DEL_LOCAL_REF(class_config);
	return cl_sa_public_set_ap_config(dev_handle, &c_ap_config);
}

JNIEXPORT jint JNICALL
NAME(ClSetSSIDAndPSD)(JNIEnv* env, jobject this, jint dev_handle, jstring ssids, jstring password)
{

    char *ssid = NULL;
	char *psd = NULL;
	ssid = (char *)(*env)->GetStringUTFChars(env, ssids, NULL);
	psd = (char *)(*env)->GetStringUTFChars(env,password,NULL);
	
	return cl_com_udp_reset_dev_ssid_and_passwd(dev_handle, ssid, psd);
}

JNIEXPORT jint JNICALL
NAME(ClSetRelayOnoff)(JNIEnv* env, jobject this, jint handle, jboolean onoff)
{
	return cl_sa_public_set_repeat_onoff(handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClGetMaxNameLen)(JNIEnv* env, jobject this, jint handle, jint type)
{
	return cl_user_name_len_limit_get(handle, type);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlScene)(JNIEnv* env, jobject this, jint handle, jbyte scene)
{
	return cl_chiffo_set_scene(handle, scene);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoCtrlLoop)(JNIEnv* env, jobject this, jint handle, jbyte loop_type)
{
	return cl_chiffo_set_loop(handle, loop_type);
}

JNIEXPORT jint JNICALL
NAME(ClChiffoSetClock)(JNIEnv* env, jobject this, jint handle)
{
	return cl_chiffo_set_clock(handle);
}


