#include "clib_jni.h"
#include "clib_jni_sdk.h"


int nactivUserInit(JNIEnv *env, user_param_t *up)
{
	memset(up, 0, sizeof(user_param_t));
	up->clazz = (*env)->FindClass(env, CLASS_USER_INFO);
	up->fid_user_handle = (*env)->GetFieldID(env, up->clazz, "UserHandle", "I");
	//up->fid_username = (*env)->GetFieldID(env, up->clazz, "Username", "Ljava/lang/String;");
	//up->fid_callback_handle = (*env)->GetFieldID(env, up->clazz, "CallbackHandle", "I");

	up->fid_callback = (*env)->GetMethodID(env, up->clazz, "Callback", "(III)V");

	//(*env)->DeleteLocalRef(env, up->clazz);
	return 0;
}

static JavaVM *g_jvm = NULL;  
static jclass class_clib = NULL;
static jmethodID fid_callback = NULL;
//¶þ´Î¿ª·¢µÄ»Øµ÷º¯Êý
static jmethodID develop_fid_callback = NULL;
static cl_callback_t video_callback = NULL;

static jmethodID fid_rsa_encrypt = NULL;
static jmethodID fid_rsa_decrypt = NULL;


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;
	
	LOGI("JNI_OnLoad startup~~! JavaVM=%p", vm);
	g_jvm = vm;
	
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) == JNI_OK) {
		result = JNI_VERSION_1_4;
	} else if (env == NULL) {
		LOGE("JNI_OnLoad GetEvn failed");
		return -1;
	}
	
	return result;	
}

void _jni_assert(bool b, const char *file, int line)
{
	if ( ! b ) {
		LOGE("%s line %d assert failed!!!\n", file, line);
		*(char *)0 = 0;
	}
}


JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
	LOGE("call JNI_OnUnload ~~!!");

	cl_stop();
	
	// free objcet ref...
}

JNIEXPORT jint JNICALL
NAME(ClStop)(JNIEnv* env, jobject this)
{
	LOGE("ClStop");
	return cl_stop();
}

bool is_valid_ipv4_address(const char *str)
{
	struct in_addr addr;
	int ret;
	ret = inet_pton(AF_INET, str, &addr);
	if (ret > 0) {
		return true;
	} else if(ret < 0) {
		LOGE("EAFNOSUPPORT, %s \n", str);
	} else {
		LOGE("\"%s\" is not a valid IPv4 address\n", str);
	}
	return false;
}

void javaRsa(jmethodID rsaMethod, int priv_len, u_int8_t *priv, int in_len, u_int8_t *in, u_int8_t *out) {
	jbyteArray key;
	jbyteArray srDatas;
	jbyteArray dsDatas;
	jbyte *ds;
	JNIEnv* env;

	if (g_jvm == NULL) {
		LOGE("event callback failed: JavaVM = NULL");
		return;
	}

	if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
		LOGE("AttachCurrentThread failed");
		return;
	}

	if (priv_len == 0 || priv == NULL || in_len == 0 || in == NULL || out == NULL) {
		LOGE("error param, null pointer!");
		return;
	}

	key = (*env)->NewByteArray(env, priv_len);
	srDatas = (*env)->NewByteArray(env, in_len);
	
	(*env)->SetByteArrayRegion(env, key, 0, priv_len, priv);
	(*env)->SetByteArrayRegion(env, srDatas, 0, in_len, in);
	
	dsDatas = (*env)->CallStaticObjectMethod(env, class_clib, rsaMethod, key, srDatas);
	if (dsDatas == NULL) {
		goto quit;
	}
	
	ds = (*env)->GetByteArrayElements(env, dsDatas, false);
	memcpy(out, ds, 128);
	(*env)->ReleaseByteArrayElements(env, dsDatas, ds, 0);

	quit:
	SAFE_DEL_LOCAL_REF(key);
	SAFE_DEL_LOCAL_REF(srDatas);
}

void javaRsaEncrypty(int priv_len, u_int8_t *priv, int in_len, u_int8_t *in, u_int8_t *out) {
	javaRsa(fid_rsa_encrypt, priv_len, priv, in_len, in, out);
}

void javaRsaDecrypty(int priv_len, u_int8_t *priv, int in_len, u_int8_t *in, u_int8_t *out) {
	javaRsa(fid_rsa_decrypt, priv_len, priv, in_len, in, out);
}

JNIEXPORT jint JNICALL
NAME(ClInitV2)(JNIEnv* env, jobject this, jobject initParam)
{
	cl_lib_info_t info;
	jint result = RS_OK;
	char *str_ip, *str_dir;
	jclass class_clib_tmp = NULL;
	jsize len = 0;
	jclass class_param = NULL;
	jbyteArray uuid = NULL;
	jbyteArray app_ver = NULL;
	jfieldID fid = NULL;
	jstring server_ip = NULL;

	memset(&info, 0, sizeof(info));

	class_clib_tmp = (*env)->FindClass(env, CLASS_CLIB);
	if (class_clib_tmp	== NULL) {
		LOGE("FindClass(%s) failed", CLASS_CLIB);
		result = RS_NOT_FOUND;
		goto done;
	}
	class_clib = (jclass)(*env)->NewGlobalRef(env, class_clib_tmp);
	if (class_clib == NULL) {
		LOGE("alloc clib global faild\n");
		result = RS_NOT_FOUND;
		goto done;
	}
	fid_callback = (*env)->GetStaticMethodID(env, class_clib, "Callback", "(III)V");
	if (fid_callback == NULL) {
		LOGE("GetMethodID(Callback) failed");
		result = RS_NOT_FOUND;
		goto done;
	}	
	
	class_param = (*env)->GetObjectClass(env, initParam);
	/*uuid*/
	fid = (*env)->GetFieldID(env, class_param, "uuid", "[B");
	uuid = (*env)->GetObjectField(env, initParam, fid); 
	len  = (*env)->GetArrayLength(env,uuid);
	if(len<0){
		len = 0;
	}
	if(len>MAX_UUID_LEN){
		len = MAX_UUID_LEN;
	}
	(*env)->GetByteArrayRegion(env,uuid,0,len,(jbyte*)&info.uuid);

	fid = (*env)->GetFieldID(env, class_param, "app_ver", "[B");
	app_ver = (*env)->GetObjectField(env, initParam, fid);
	len  = (*env)->GetArrayLength(env, app_ver);
	(*env)->GetByteArrayRegion(env, app_ver, 0, len, (jbyte*)&info.app_ver);
	/*Ö¸¶¨µÄ·þÎñÆ÷ip*/
	fid = (*env)->GetFieldID(env, class_param, "server_ip", "Ljava/lang/String;");
	server_ip = (*env)->GetObjectField(env, initParam, fid);
	if(server_ip != NULL) {
		str_ip = (char *)((*env)->GetStringUTFChars(env, server_ip, NULL));
		if(!is_valid_ipv4_address(str_ip)) {
			//æ— æ•ˆçš„IPåœ°å€
			info.reserved = NULL;
		} else {
			info.reserved = strdup(str_ip);
		}
		(*env)->ReleaseStringUTFChars(env, server_ip, str_ip);
	}
	/*ÆäËûÏî*/
	jni_assign_simple_struct(env, initParam, class_param,
		                        ASSIGN_TRIPLES(int, &info, timezone),
		                        ASSIGN_TRIPLES(int, &info, vvid),
		                        ASSIGN_QUAD(String, &info, dir, sizeof(info.dir)),
		                        ASSIGN_QUAD(String, &info, priv_dir, sizeof(info.priv_dir)),
		                        ASSIGN_TRIPLES(int, &info, app_type),
		                        ASSIGN_TRIPLES(int, &info, app_id),
		                        ASSIGN_QUAD(String, &info, phone_desc, sizeof(info.phone_desc)),
		                        JNI_VAR_ARG_END);

	if(IS_INIT_DEV_HTL){
		init_devtype_htl();
	}
	if(IS_INIT_DEV_HTL_2){
		init_devtype_htl2();
	}

done:
	cl_init(CID_ANDROID, &info);

	fid_rsa_encrypt = (*env)->GetStaticMethodID(env, class_clib, "CallRsaEncrypt", "([B[B)[B");
	fid_rsa_decrypt = (*env)->GetStaticMethodID(env, class_clib, "CallRsaDecrypte", "([B[B)[B");
	cl_la_rsa_callback_set(javaRsaEncrypty, javaRsaDecrypty);
	
	if(info.reserved != NULL) {
		free(info.reserved);
	}
	(*env)->DeleteLocalRef(env, class_clib_tmp);

	return result;	
}

JNIEXPORT jint JNICALL
NAME(ClInit)(JNIEnv* env, jobject this, jint time_zone, jint vvid, jbyteArray uuid, jstring server_ip, jstring dir,jint app_type)
{
	cl_lib_info_t info;
	jint result = -1;
	char *str_ip, *str_dir;
	jclass class_clib_tmp = NULL;

	memset(&info, 0, sizeof(info));

	class_clib_tmp = (*env)->FindClass(env, CLASS_CLIB);
	if (class_clib_tmp	== NULL) {
		LOGE("FindClass(%s) failed", CLASS_CLIB);
		goto done;
	}
	class_clib = (jclass)(*env)->NewGlobalRef(env, class_clib_tmp);
	if (class_clib == NULL) {
		LOGE("alloc clib global faild\n");
		goto done;
	}
	fid_callback = (*env)->GetStaticMethodID(env, class_clib, "Callback", "(III)V");
	if (fid_callback == NULL) {
		LOGE("GetMethodID(Callback) failed");
		goto done;
	}

	jsize len  = (*env)->GetArrayLength(env,uuid);
	if(len<0){
		len = 0;
	}
	
	if(len>MAX_UUID_LEN){
		len = MAX_UUID_LEN;
	}
	
	(*env)->GetByteArrayRegion(env,uuid,0,len,(jbyte*)&info.uuid);
	info.vvid = vvid;
	info.timezone = time_zone;
	if(server_ip != NULL) {
		str_ip = (char *)((*env)->GetStringUTFChars(env, server_ip, NULL));
		if(!is_valid_ipv4_address(str_ip)) {
			//æ— æ•ˆçš„IPåœ°å€
			info.reserved = NULL;
		} else {
			info.reserved = strdup(str_ip);
		}
		(*env)->ReleaseStringUTFChars(env, server_ip, str_ip);
	}

	if(dir != NULL) {
		str_dir = (char *)((*env)->GetStringUTFChars(env, dir, NULL));
		strncpy(info.dir,str_dir , sizeof(info.dir));
		(*env)->ReleaseStringUTFChars(env, dir, str_dir);
	}
	info.app_type = app_type;

done:
	cl_init(CID_ANDROID, &info);
	if(info.reserved != NULL) {
		free(info.reserved);
	}
	(*env)->DeleteLocalRef(env, class_clib_tmp);

	return result;	
}

static jobject CopyLimit(JNIEnv* env, cl_limit_t *limit)
{
	jclass class_limit = NULL;
	jobject object_limit = NULL;
	jclass class_video_param = NULL;
	jobject object_video_param = NULL;
	jobjectArray obj_video_param_array = NULL;
	int video_param_count;
	jfieldID fid;
	
	class_limit = (*env)->FindClass(env, CLASS_LIMIT);
	object_limit =  (*env)->AllocObject(env, class_limit);
	if(!object_limit){
		(*env)->DeleteLocalRef(env, class_limit);
		return NULL;
	}
	
	jniCopyIntValue(env, class_limit, "max_user_name_len", object_limit, limit->max_user_name_len);
	
	jniCopyIntValue(env, class_limit, "max_user_passwd_len", object_limit, limit->max_user_passwd_len);
	
	jniCopyIntValue(env, class_limit, "max_mod_name_len", object_limit, limit->max_mod_name_len);
	
	jniCopyIntValue(env, class_limit, "max_wifi_ssid_len", object_limit, limit->max_wifi_ssid_len);
	
	jniCopyIntValue(env, class_limit, "max_wifi_passwd_len", object_limit, limit->max_wifi_passwd_len);
	
	jniCopyIntValue(env, class_limit, "max_area", object_limit, limit->max_area);
	
	jniCopyIntValue(env, class_limit, "max_area_name_len", object_limit, limit->max_area_name_len);

	jniCopyIntValue(env, class_limit, "max_scene", object_limit, limit->max_scene);

	jniCopyIntValue(env, class_limit, "max_scene_name_len", object_limit, limit->max_scene_name_len);

	jniCopyIntValue(env, class_limit, "max_equipment", object_limit, limit->max_equipment);
	
	jniCopyIntValue(env, class_limit, "max_eq_name_len", object_limit, limit->max_eq_name_len);
	
	jniCopyIntValue(env, class_limit, "max_key_of_eq", object_limit, limit->max_key_of_eq);
	
	jniCopyIntValue(env, class_limit, "max_key_name_len", object_limit, limit->max_key_name_len);
	
	jniCopyIntValue(env, class_limit, "max_alarm_msg_len", object_limit, limit->max_alarm_msg_len);
	
	jniCopyIntValue(env, class_limit, "max_belter_user", object_limit, limit->max_belter_user);
	
	jniCopyIntValue(env, class_limit, "max_belter_user_name_len", object_limit, limit->max_belter_user_name_len);
	
	jniCopyIntValue(env, class_limit, "max_008_timer", object_limit, limit->max_008_timer);
	
	jniCopyIntValue(env, class_limit, "max_008_timer_name_len", object_limit, limit->max_008_timer_name_len);
	
	jniCopyIntValue(env, class_limit, "max_record_timer", object_limit, limit->max_record_timer);
	
	jniCopyIntValue(env, class_limit, "max_record_timer_name_len", object_limit, limit->max_record_timer_name_len);
	
	jniCopyIntValue(env, class_limit, "max_scene_timer", object_limit, limit->max_scene_timer);
	
	jniCopyIntValue(env, class_limit, "max_scene_timer_name_len", object_limit, limit->max_scene_timer_name_len);
	
	jniCopyIntValue(env, class_limit, "max_phone_model_len", object_limit, limit->max_phone_model_len);
	
	jniCopyIntValue(env, class_limit, "max_bind_name_len", object_limit, limit->max_bind_name_len);
	
	jniCopyIntValue(env, class_limit, "max_bind_message_len", object_limit, limit->max_bind_message_len);


	
	jniCopyIntValue(env, class_limit, "min_belter_user_height", object_limit, limit->min_belter_user_height);
	jniCopyIntValue(env, class_limit, "max_belter_user_height", object_limit, limit->max_belter_user_height);
	jniCopyIntValue(env, class_limit, "min_belter_user_weight", object_limit, limit->min_belter_user_weight);
	jniCopyIntValue(env, class_limit, "max_belter_user_weight", object_limit, limit->max_belter_user_weight);
	jniCopyIntValue(env, class_limit, "min_belter_user_age", object_limit, limit->min_belter_user_age);
	jniCopyIntValue(env, class_limit, "max_belter_user_age", object_limit, limit->max_belter_user_age);
	
	if(limit->video_param) {
		class_video_param = (*env)->FindClass(env, CLASS_VIDEO_PARAM);
		video_param_count = sizeof(limit->video_param) / sizeof(cl_video_param_t);
		if(video_param_count > 0) {
			jintArray fps_array;
			int fps_count, i, j;
			obj_video_param_array = (*env)->NewObjectArray(env, video_param_count, class_video_param, NULL);
			for(i = 0; i < video_param_count; i++){
				object_video_param = (*env)->AllocObject(env, class_video_param);
				
				jniCopyIntValue(env, class_video_param, "width", object_video_param, limit->video_param[i].width);
				
				jniCopyIntValue(env, class_video_param, "height", object_video_param, limit->video_param[i].height);
				
				fps_count = sizeof(limit->video_param[i].fps) / sizeof(int);
				for(j = 0; j < fps_count; j++) {
					if(limit->video_param[i].fps[j] == 0) {
						break;
					}
				}
				fps_count = j;
				if(fps_count <= 0) {
					(*env)->DeleteLocalRef(env, object_video_param);
					continue;
				}
				fid = (*env)->GetFieldID(env, class_video_param, "fps", "[I");
				fps_array = (*env)->NewIntArray(env, fps_count);  
				(*env)->SetIntArrayRegion(env, fps_array, 0, fps_count, limit->video_param[i].fps);
				(*env)->SetObjectField(env, object_video_param, fid, fps_array);
				(*env)->DeleteLocalRef(env, fps_array);
				(*env)->SetObjectArrayElement(env, obj_video_param_array, i, object_video_param);
				
				(*env)->DeleteLocalRef(env, object_video_param);
			}
			fid = (*env)->GetFieldID(env, class_limit, "video_param", "[L" CLASS_VIDEO_PARAM ";");
			(*env)->SetObjectField(env, object_limit, fid, obj_video_param_array);
			
			(*env)->DeleteLocalRef(env, obj_video_param_array);
		}
		(*env)->DeleteLocalRef(env, class_video_param);
	}
	
	(*env)->DeleteLocalRef(env, class_limit);
	
	return object_limit;
}

JNIEXPORT jobject JNICALL
NAME(ClGetClibLimit)(JNIEnv* env, jobject this)
{
	cl_lib_info_t info;
	cl_limit_t * limit_info = NULL;
	int ret = RS_ERROR;
	jclass class_limit = NULL;
	jobject obj_limit = NULL;
	jfieldID fid;
	
	ret = cl_get_info(&info);
	if(ret == RS_ERROR) {
		return NULL;
	}
	limit_info = &info.limit;
	
	class_limit = (*env)->FindClass(env, CLASS_SDK_CLIB_LIMIT);
	obj_limit = (*env)->AllocObject(env, class_limit);
	
	jni_copy_simple_class(env,class_limit,obj_limit,
	                         TRIPLES(int, limit_info, max_user_name_len),
	                         TRIPLES(int, limit_info, max_user_passwd_len),
	                         TRIPLES(int, limit_info, max_mod_name_len),
	                         TRIPLES(int, limit_info, max_wifi_ssid_len),
	                         TRIPLES(int, limit_info, max_wifi_passwd_len),
	                         TRIPLES(int, limit_info, max_key_of_eq),
	                         TRIPLES(int, limit_info, max_key_name_len),
		                     JNI_VAR_ARG_END);
	
	SAFE_DEL_LOCAL_REF(class_limit);
	
	return obj_limit;
}


JNIEXPORT jobject JNICALL
NAME(ClGetInfo)(JNIEnv* env, jobject this)
{
	cl_lib_info_t info;
	int ret = RS_ERROR;
	jclass class_client_info = NULL;
	jobject object_client_info = NULL;
	jobject object_limit = NULL;
	jfieldID fid;
	jstring str;
	u_int8_t version[8];
	jbyteArray jbarray;
	
	ret = cl_get_info(&info);
	if(ret == RS_ERROR) {
		return NULL;
	}
	
	class_client_info = (*env)->FindClass(env, CLASS_CLIB_VERSION_INFO);
	
	object_client_info = (*env)->AllocObject(env, class_client_info);
	if(!object_client_info){
		(*env)->DeleteLocalRef(env, class_client_info);
		return NULL;
	}
	
	object_limit =  CopyLimit(env, &info.limit);
	if(!object_limit) {
		(*env)->DeleteLocalRef(env, class_client_info);
		return NULL;
	}
	fid = (*env)->GetFieldID(env, class_client_info, "limit", "L" CLASS_LIMIT ";");
	(*env)->SetObjectField(env, object_client_info, fid, object_limit);

	fid = (*env)->GetFieldID(env, class_client_info, "timezone", "I");
	(*env)->SetIntField(env, object_client_info, fid, info.timezone);

	fid = (*env)->GetFieldID(env, class_client_info, "vvid", "I");
	(*env)->SetIntField(env, object_client_info, fid, info.vvid);
	
	jbarray = (*env)->NewByteArray(env, strlen(info.uuid));
	(*env)->SetByteArrayRegion(env, jbarray, 0, strlen(info.uuid), (jbyte *)info.uuid);
	fid = (*env)->GetFieldID(env, class_client_info, "uuid", "[B");
	(*env)->SetObjectField(env, object_client_info, fid, jbarray);

	memset(version, 0, sizeof(version));
	sprintf(version, "%u.%u.%u.%u", info.version[0], info.version[1], info.version[2], info.version[3]);
	jniCopyIntValue(env, class_client_info, "svn", object_client_info, info.svn);
	jniCopyString(env, class_client_info, "version", object_client_info, version);
	
	jniCopyString(env, class_client_info, "desc", object_client_info, info.desc);
	
	(*env)->DeleteLocalRef(env, class_client_info);
	
	return object_client_info;
}

void nactivCallback(u_int32_t event, void *user_handle, void *callback_handle)
{
	JNIEnv* env;
	
	LOGD("UserEvent: event=%u, user_handle=%p, callback_handle=%p", event, user_handle, callback_handle);
	if (g_jvm == NULL) {
		LOGE("event callback failed: JavaVM = NULL");
		return;
	}

	if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
		LOGE("AttachCurrentThread failed");
		return;
	}
	
	if (class_clib != NULL && fid_callback != NULL) {
        cl_event_more_info_t *callback_info = (cl_event_more_info_t *)callback_handle;
	 if(callback_info != NULL){
	 	(*env)->CallStaticVoidMethod(env, class_clib, fid_callback, event, (jint)(jlong)user_handle, callback_info->err_no);
	 }else{
        	(*env)->CallStaticVoidMethod(env, class_clib, fid_callback, event, (jint)(jlong)user_handle, 0);
	 }
	}

	(*g_jvm)->DetachCurrentThread(g_jvm);
}

int developInitMid(JNIEnv* env)
{
	if(class_clib == NULL){
		LOGE("clib class is null\n");
		return RS_NOT_INIT;
	}
	develop_fid_callback = (*env)->GetStaticMethodID(env,class_clib,"developCallback","(IJ[B)V");
	if(develop_fid_callback == NULL){
		LOGE("GetMethodID(developCallback) failed");
		return RS_NOT_FOUND;
	}
	return RS_OK;
}


void developNativeCallback(int type,u_int64_t ident,char* data,int len)
{
	JNIEnv* env;
	
	//LOGD("developNativeCallback: type=%d, ident=%llu, len=%u", type, ident, len);
	if (g_jvm == NULL) {
		LOGE("developNativeCallback failed: JavaVM = NULL");
		return;
	}

	if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
		LOGE("developNativeCallback AttachCurrentThread failed");
		return;
	}
	
	if (class_clib != NULL && develop_fid_callback!= NULL) {
       	 	jbyteArray pkt_data = (*env)->NewByteArray(env,len);
	 	if(len > 0){
			(*env)->SetByteArrayRegion(env,pkt_data,0,len,(jbyte*)data);
		}
	 	(*env)->CallStaticVoidMethod(env, class_clib, develop_fid_callback, type,ident,pkt_data);
	}

	(*g_jvm)->DetachCurrentThread(g_jvm);
}

void setVideoCallback(cl_callback_t callback)
{
    video_callback = callback;
}

void doString(char** pBuffer) {
	char* start = (char *)*pBuffer;
	char* end = (char *)*pBuffer + strlen(start);
	int i = 0;
	while(start < end) {
		if(*start < 0x80) {
			//å€¼å°äºŽ0Ã—80çš„ä¸ºASCIIå­—ç¬¦
			start++;
		} else if(*start < 0xC0) {
			//å€¼ä»‹äºŽ0Ã—80ä¸Ž0xC0ä¹‹é—´çš„ä¸ºæ— æ•ˆUTF-8å­—ç¬¦
			*start = '?';
			start++;
			continue;
		} else if(*start < 0xE0) {
			//æ­¤èŒƒå›´å†…ä¸º2å­—èŠ‚UTF-8å­—ç¬¦
			if(start >= end - 1) {
				for(i = 0; i < end - start; i++) {
					*(start + i)= '\0';
				}
				break;
			}
			if((start[1] & (0xC0)) != 0x80) {
				*start = '?';
				*(start + 1) = '?';
				continue;
			}
			start += 2;
		} else if(*start < 0xF0) {
			//æ­¤èŒƒå›´å†…ä¸º3å­—èŠ‚UTF-8å­—ç¬¦
			if(start >= end - 2) {
				for(i = 0; i < end - start; i++) {
					*(start + i)= '\0';
				}
				break;
			}
			if((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80) {
				*start = '?';
				*(start + 1) = '?';
				*(start + 2) = '?';
				continue;
			}
			start += 3;
		} else {
			*start = '?';
			start++;
			continue;
		}
	}
}

void jniCopyString(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, char* src)
{
	jfieldID fid;
	jstring str;

	if(!src) {
		src = "";
	}else{
		doString(&src);
	}
	
	fid = (*env)->GetFieldID(env, obj_class, attrname, "Ljava/lang/String;");
	str = (*env)->NewStringUTF(env, !src ? "" : src);

	(*env)->SetObjectField(env, obj, fid, str);
	(*env)->DeleteLocalRef(env, str);
	
}

void jniCopyStringArray(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, char** src, int num)
{
	jclass class_string = NULL;
	jobject array_obj = NULL;
	jobject obj_string = NULL;
	int i = 0;
	jstring str;
	jfieldID fid;

	class_string = (*env)->FindClass(env, "java/lang/String");
	array_obj = (*env)->NewObjectArray(env, num, class_string, NULL);
	
	for (i = 0; i < num; ++i) {
		doString(&src[i]);
		str = (*env)->NewStringUTF(env, src[i]);
		(*env)->SetObjectArrayElement(env, array_obj, i, str);
		SAFE_DEL_LOCAL_REF(str);
	}
	fid = (*env)->GetFieldID(env, obj_class, attrname, "[Ljava/lang/String;");
	(*env)->SetObjectField(env, obj, fid, array_obj);

	SAFE_DEL_LOCAL_REF(class_string);
	SAFE_DEL_LOCAL_REF(array_obj);
	SAFE_DEL_LOCAL_REF(obj_string);
}

void jniCopyByteValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, u_int8_t value)
{
	jfieldID fid;
	
	if(!value )
		return;

	fid = (*env)->GetFieldID(env, obj_class, attrname, "B");
	(*env)->SetByteField(env, obj, fid, value);
}


void jniCopyShortValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, short value)
{
	jfieldID fid;

	if(!value )
		return;

	fid = (*env)->GetFieldID(env, obj_class, attrname, "S");
	(*env)->SetShortField(env, obj, fid, value);
}

void jniCopyIntValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, int value)
{
	jfieldID fid;
	
	if(!value )
		return;

	fid = (*env)->GetFieldID(env, obj_class, attrname, "I");
	(*env)->SetIntField(env, obj, fid, value);
}

void jniCopyLongValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, u_int64_t value)
{
	jfieldID fid;
	
	if(!value )
		return;

	fid = (*env)->GetFieldID(env, obj_class, attrname, "J");
	(*env)->SetLongField(env, obj, fid, value);
}

//æ‹·è´Boolåˆ°å¯¹è±¡
void jniCopyBooleanValue(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, bool value)
{
	jfieldID fid;

	if(!value )
		return;

	fid = (*env)->GetFieldID(env, obj_class, attrname, "Z");
	(*env)->SetBooleanField(env, obj, fid, !!value);
}

void jniCopyIntArray(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, int* array,int array_num)
{
	jfieldID fid;
	jintArray t_array = NULL;

	if(!array || !array_num){
		return;
	}

	fid = (*env)->GetFieldID(env, obj_class, attrname,"[I");
	if(fid != NULL && array_num > 0 && array != NULL){
		t_array = (*env)->NewIntArray(env,array_num);
		if(t_array != NULL){
			(*env)->SetIntArrayRegion(env, t_array, 0,array_num, array);
			(*env)->SetObjectField(env, obj, fid, t_array);
		}
		SAFE_DEL_LOCAL_REF(t_array);
	}
	
}

void jniCopyLongArray(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, u_int64_t* array,int array_num)
{
	jfieldID fid;
	jlongArray t_array = NULL;

	if(!array || !array_num){
		return;
	}

	fid = (*env)->GetFieldID(env, obj_class, attrname,"[J");
	if(fid != NULL && array_num > 0 && array != NULL){
		t_array = (*env)->NewLongArray(env,array_num);
		if(t_array != NULL){
			(*env)->SetLongArrayRegion(env, t_array, 0,array_num, (jlong*)array);
			(*env)->SetObjectField(env, obj, fid, t_array);
		}
		SAFE_DEL_LOCAL_REF(t_array);
	}
	
}



void jniCopyByteArray(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, u_int8_t* array,int array_num)
{
	jfieldID fid;
	jbyteArray jniByteArray = NULL;
	
	if (NULL == array || array_num <= 0) {
		return;
	}
	fid = (*env)->GetFieldID(env, obj_class, attrname,"[B");
	if ( NULL != fid) {
		jniByteArray = (*env)->NewByteArray(env, array_num);
		if (NULL != jniByteArray) {
			(*env)->SetByteArrayRegion(env, jniByteArray, 0, array_num, array);
			(*env)->SetObjectField(env, obj, fid, jniByteArray);
		}
		SAFE_DEL_LOCAL_REF(jniByteArray);
	}
}

void jniCopyShortArray(JNIEnv* env, jclass obj_class,char* attrname, jobject obj, short* array,int array_num)
{
	jfieldID fid;
	jshortArray jnishortArray = NULL;
	
	if (NULL == array || array_num <= 0) {
		return;
	}
	fid = (*env)->GetFieldID(env, obj_class, attrname,"[S");
	if ( NULL != fid) {
		jnishortArray = (*env)->NewShortArray(env, array_num);
		if (NULL != jnishortArray) {
			(*env)->SetShortArrayRegion(env, jnishortArray, 0, array_num, array);
			(*env)->SetObjectField(env, obj, fid, jnishortArray);
		}
		SAFE_DEL_LOCAL_REF(jnishortArray);
	}
}
/************************************************************************************
		tools 
 ************************************************************************************/
 /*
  *²»½öÒªÇójavaÀà³ÉÔ±Ãû×ÖºÍC ½á¹¹Ìå³ÉÔ±Ãû×ÖÏàÍ¬£¬»¹ÒªÇóËüÃÇÀàÐÍÏàÍ¬!
 */
 int jni_copy_array_class_item(JNIEnv *env, jclass wrap_class, jobject wrap_object, int value_obj_size, int index, ...) 
{
	va_list arg_ptr; 
	char *type = NULL;
	char *attrname = NULL;
	void *addr = NULL;
	int ptr_offset = value_obj_size * index;

	va_start(arg_ptr, index); 
	type = va_arg(arg_ptr, char*);
	//LOGD("xxxddd jni_copy_array_class_item type, offset = %d\n", ptr_offset);
	while(strcmp(type, JNI_VAR_ARG_END) != 0) {
		//LOGD("xxxddd jni_copy_array_class_item type = %s\n", type);
		addr = va_arg(arg_ptr, void*) + ptr_offset;
		if (strcmp(type, JNI_TYPE_SHORT_BYTE) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int8_t *pv = (u_int8_t *)addr;
			jniCopyShortValue(env,wrap_class, attrname, wrap_object, *pv);
		} else if (strcmp(type, JNI_TYPE_LONG) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int64_t *pv = (u_int64_t*)addr;
			jniCopyLongValue(env, wrap_class, attrname, wrap_object, *pv);
		} else if (strcmp(type, JNI_TYPE_INT) == 0) {
			attrname = va_arg(arg_ptr, char*);
			int *pv = (int*)addr;
			jniCopyIntValue(env, wrap_class, attrname, wrap_object, *pv);
		} else if (strcmp(type, JNI_TYPE_SHORT) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int16_t *pv = (u_int16_t*)addr;
			jniCopyShortValue(env,wrap_class, attrname, wrap_object, *pv);
		} else if (strcmp(type, JNI_TYPE_BYTE) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int8_t *pv = (u_int8_t *)addr;
			//LOGD("xxxddd jni_copy_array_class_item addr = %p, value = %d\n", addr, *pv);
			jniCopyByteValue(env,wrap_class, attrname, wrap_object, *pv);
		} else if (strcmp(type, JNI_TYPE_BOOLEAN) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int8_t *pv = (u_int8_t *)addr;
			//LOGD("xxxddd jni_copy_array_class_item addr = %p, value = %d\n",addr, *pv);
			jniCopyBooleanValue(env,wrap_class, attrname, wrap_object, *pv);
		} else if (strcmp(type, JNI_TYPE_STRING) == 0) {
			attrname = va_arg(arg_ptr, char*);
			char *pv = (char *)addr;
			jniCopyString(env,wrap_class, attrname, wrap_object, pv);
		} else if (strcmp(type, JNI_TYPE_LONG_ARRAY) == 0) {
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			u_int64_t **pv = (u_int64_t **)addr;
			jniCopyLongArray(env, wrap_class, attrname, wrap_object, *pv, count);
		} else if (strcmp(type, JNI_TYPE_INT_ARRAY) == 0) {
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			int **pv = (int **)addr;
			jniCopyIntArray(env, wrap_class, attrname, wrap_object, *pv, count);
		} else if (strcmp(type, JNI_TYPE_SHORT_ARRAY) == 0) {
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			short **pv = (short **)addr;
			jniCopyShortArray(env, wrap_class, attrname, wrap_object, *pv, count);
		} else if (strcmp(type, JNI_TYPE_BYTE_ARRAY) == 0) {
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			u_int8_t **pv = (u_int8_t **)addr;
			jniCopyByteArray(env, wrap_class, attrname, wrap_object, *pv, count);
		} else if (strcmp(type, JNI_TYPE_STRING_ARRAY) == 0) {
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			char ***pv = (char ***)addr;
			jniCopyStringArray(env, wrap_class, attrname, wrap_object, *pv, count);
		}
		else {
			LOGE("jni_copy_simple_class param error, type = %s\n", type);
			return RS_ERROR;
		}
		type = va_arg(arg_ptr, char*);
	}

	return RS_OK;
}

int jni_assign_simple_struct(JNIEnv* env, jobject orgObj, jclass orgClass, ...)
{
	va_list arg_ptr; 
	char *type = NULL;
	char *attrname = NULL;
	jfieldID fid = NULL;
	jbyteArray byteArray = NULL;
	void *addr = NULL;
	int len = 0;

	va_start(arg_ptr, orgClass); 
	type = va_arg(arg_ptr, char*);

	while(strcmp(type, JNI_VAR_ARG_END) != 0) {
		addr = va_arg(arg_ptr, void*);
		//LOGD("xxxddd assign type = %s\n", type);
		if (strcmp(type, JNI_TYPE_LONG) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int64_t *pv = (u_int64_t*)addr;
			fid = (*env)->GetFieldID(env, orgClass, attrname, "J");
			(*pv) = (*env)->GetLongField(env, orgObj, fid);
		}else if (strcmp(type, JNI_TYPE_INT) == 0) {
			attrname = va_arg(arg_ptr, char*);
			int *pv = (int*)addr;
			fid = (*env)->GetFieldID(env, orgClass, attrname, "I");
			(*pv) = (*env)->GetIntField(env, orgObj, fid);
		} else if (strcmp(type, JNI_TYPE_SHORT) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int16_t *pv = (u_int16_t*)addr;
			fid = (*env)->GetFieldID(env, orgClass, attrname, "S");
			(*pv) = (*env)->GetShortField(env, orgObj, fid);
			//LOGD("xxxddd pv = %hu\n", *pv);
		} else if (strcmp(type, JNI_TYPE_BYTE) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int8_t *pv = (u_int8_t *)addr;
			fid = (*env)->GetFieldID(env, orgClass, attrname, "B");
			(*pv) = (*env)->GetByteField(env, orgObj, fid);
			//LOGD("xxxddd pv = %hu\n", *pv);
		} else if (strcmp(type, JNI_TYPE_BOOLEAN) == 0) {
			attrname = va_arg(arg_ptr, char*);
			u_int8_t *pv = (u_int8_t *)addr;
			fid = (*env)->GetFieldID(env, orgClass, attrname, "Z");
			(*pv) = (*env)->GetBooleanField(env, orgObj, fid);
		} else if (strcmp(type, JNI_TYPE_BYTE_ARRAY) == 0) {
			int maxLen = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			char *pv = (char *)addr;
			fid = (*env)->GetFieldID(env, orgClass, attrname, "[B");
			byteArray = (*env)->GetObjectField(env, orgObj, fid);
			len  = (*env)->GetArrayLength(env, byteArray);
			if (len < 0) {
				*pv = '\0';
			} else {
				if (len > maxLen) {
					len = maxLen;
				}
				(*env)->GetByteArrayRegion(env, byteArray, 0, len,(jbyte*)pv);
			}
		}
		else if (strcmp(type, JNI_TYPE_STRING) == 0) {
			int maxLen = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			char *pv = (char *)addr;
			char *str = NULL;

			fid = (*env)->GetFieldID(env, orgClass, attrname, "Ljava/lang/String;");
			jobject obj_str = (*env)->GetObjectField(env, orgObj, fid);
			if (obj_str != NULL) {
				str = (char*)(*env)->GetStringUTFChars(env, obj_str, NULL);
				if (str != NULL) {
					strncpy(pv, str, maxLen);
					(*env)->ReleaseStringUTFChars(env, obj_str, str);
				}
			}
			
		} else {
			LOGE("jni_assign_simple_struct, type = %s\n", type);
			return RS_ERROR;
		}
		type = va_arg(arg_ptr, char*);
	}
	return RS_OK;
}


int jni_copy_simple_class(JNIEnv *env, jclass wrap_class, jobject wrap_object, ...) {
	va_list arg_ptr; 
	char *type = NULL;
	char *attrname = NULL;

	va_start(arg_ptr, wrap_object); 
	type = va_arg(arg_ptr, char*);
	
	while(strcmp(type, JNI_VAR_ARG_END) != 0) {
		//LOGD("xxxxddddd copy type = %s\n", type);
		if (strcmp(type, JNI_TYPE_LONG) == 0) {
			u_int64_t value = va_arg(arg_ptr, u_int64_t);
			attrname = va_arg(arg_ptr, char*);
			jniCopyLongValue(env, wrap_class, attrname, wrap_object, value);
		} else if (strcmp(type, JNI_TYPE_INT) == 0) {
			int value = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyIntValue(env, wrap_class, attrname, wrap_object, value);
		} else if (strcmp(type, JNI_TYPE_SHORT) == 0) {
			int value = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyShortValue(env,wrap_class, attrname, wrap_object, value);
		} else if (strcmp(type, JNI_TYPE_BYTE) == 0) {
		 	int value = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyByteValue(env,wrap_class, attrname, wrap_object, value);
		} else if (strcmp(type, JNI_TYPE_BOOLEAN) == 0) {
			int value = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyBooleanValue(env,wrap_class, attrname, wrap_object, value);
		} else if (strcmp(type, JNI_TYPE_STRING) == 0) {
			char *value = va_arg(arg_ptr, char*);
			attrname = va_arg(arg_ptr, char*);
			jniCopyString(env,wrap_class, attrname, wrap_object, value);
		} else if (strcmp(type, JNI_TYPE_INT_ARRAY) == 0) {
			int *value = va_arg(arg_ptr, int*);
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyIntArray(env, wrap_class, attrname, wrap_object, value, count);
		} else if (strcmp(type, JNI_TYPE_SHORT_ARRAY) == 0) {
			short *value = va_arg(arg_ptr, short*);
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyShortArray(env, wrap_class, attrname, wrap_object, value, count);
		} else if (strcmp(type, JNI_TYPE_BYTE_ARRAY) == 0) {
			u_int8_t *value = va_arg(arg_ptr, u_int8_t*);
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyByteArray(env, wrap_class, attrname, wrap_object, value, count);
		}  else if (strcmp(type, JNI_TYPE_LONG_ARRAY) == 0) {
			u_int64_t *value = va_arg(arg_ptr, u_int64_t *);
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyLongArray(env, wrap_class, attrname, wrap_object, value, count);
		} else if (strcmp(type, JNI_TYPE_STRING_ARRAY) == 0) {
			char**value = va_arg(arg_ptr, char**);
			int count = va_arg(arg_ptr, int);
			attrname = va_arg(arg_ptr, char*);
			jniCopyStringArray(env, wrap_class, attrname, wrap_object, value, count);
		} 
		else {
			LOGE("jni_copy_simple_class param error, type = %s\n", type);
			return RS_ERROR;
		}
		type = va_arg(arg_ptr, char*);
	}
	return RS_OK;
}


JNIEXPORT jint JNICALL
NAME(ClSetTrafficStatEnable)(JNIEnv* env, jobject this, jboolean is_enable)
{
	return cl_set_traffic_stat_enable(is_enable);
}

JNIEXPORT jobject JNICALL
NAME(ClGetTrafficStat)(JNIEnv* env, jobject this, jboolean is_clear_data)
{
	jclass obj_class;
	jobject stat_info;
	cl_traffic_stat_t stat = {0};
	
	cl_get_traffic_stat(&stat, is_clear_data);
	
	obj_class = (*env)->FindClass(env, CLASS_TRAFFIC_STAT);
	stat_info = (*env)->AllocObject(env, obj_class);
	
	jniCopyLongValue(env, obj_class, "tx_bytes", stat_info, stat.tx_bytes);
	jniCopyLongValue(env, obj_class, "rx_bytes", stat_info, stat.rx_bytes);
	jniCopyIntValue(env, obj_class, "tx_pkts", stat_info, stat.tx_pkts);
	jniCopyIntValue(env, obj_class, "rx_pkts", stat_info, stat.rx_pkts);
	
	(*env)->DeleteLocalRef(env, obj_class);
	
	return stat_info;
}

JNIEXPORT jint JNICALL
NAME(ClSetPhoneBackground)(JNIEnv* env, jobject this, jboolean is_enable)
{
	return cl_set_phone_background(is_enable);
}

JNIEXPORT jobject JNICALL
NAME(ClGetTrafficStatDetails)(JNIEnv* env, jobject this, jboolean is_clear_data)
{
	int count, i;
	cl_traffic_stat_t* traffic;
	jclass obj_class;
	jobject stat_info;
	jobject traffic_array;
	
	traffic = cl_get_tcp_cmd_traffic_stat(&count, is_clear_data);
	if (!traffic || count <= 0) {
        return NULL;
	}
	obj_class = (*env)->FindClass(env, CLASS_TRAFFIC_STAT);
	traffic_array = (*env)->NewObjectArray(env, count , obj_class, NULL);
	for(i = 0; i < count; i++) {
		if (!traffic[i].tx_pkts && !traffic[i].rx_bytes) {
			continue;
		}
		stat_info = (*env)->AllocObject(env, obj_class);
		jniCopyLongValue(env, obj_class, "tx_bytes", stat_info, traffic[i].tx_bytes);
		jniCopyLongValue(env, obj_class, "rx_bytes", stat_info, traffic[i].rx_bytes);
		jniCopyIntValue(env, obj_class, "tx_pkts", stat_info, traffic[i].tx_pkts);
		jniCopyIntValue(env, obj_class, "rx_pkts", stat_info, traffic[i].rx_pkts);
		
		(*env)->SetObjectArrayElement(env, traffic_array, i, stat_info);
		(*env)->DeleteLocalRef(env, stat_info);
	}
	(*env)->DeleteLocalRef(env, obj_class);
	free(traffic);
	traffic = NULL;
	return traffic_array;
}

JNIEXPORT jint JNICALL
NAME(ClDevndDebugSet)(JNIEnv* env, jobject this, jboolean debug_on, jint size)
{
	return cl_dev_nd_debug_set(debug_on, size);
}

JNIEXPORT jint JNICALL
NAME(ClDevFlashUpgradeQuery)(JNIEnv* env, jobject this, jint handle, jobject blocks)
{	
	flash_block_t *flash_block = (flash_block_t *)malloc(FLASH_UPGRADE_BLOCK_NUM * sizeof(flash_block_t));
	int ret = RS_ERROR;
	jclass class_flash_block = NULL;
	
	if(blocks == NULL){
		return;
	}
	  
	//class ArrayList  
	jclass cls_arraylist = (*env)->FindClass(env,"java/util/ArrayList");  
	//method in class ArrayList  
	jmethodID arraylist_add = (*env)->GetMethodID(env, cls_arraylist, "add", "(L"CLASS_JAVA_OBJECT";)Z");  
	
	ret = cl_dev_flash_upgrade_query(handle, flash_block);
	if(ret == RS_ERROR) {
		return ret;
	}
	
	class_flash_block = (*env)->FindClass(env, CLASS_FLASH_BLOCK);

	int i =0;
	for(i=0;i< FLASH_UPGRADE_BLOCK_NUM;i++){
		jobject obj_flash_block = (*env)->AllocObject(env, class_flash_block);
		LOGE("-----run = %d",flash_block[i].run);
		jniCopyIntValue(env, class_flash_block, "flash_addr", obj_flash_block, flash_block[i].flash_addr);
		jniCopyIntValue(env, class_flash_block, "valid", obj_flash_block, flash_block[i].valid);
		jniCopyIntValue(env, class_flash_block, "soft_ver", obj_flash_block, flash_block[i].soft_ver);
		jniCopyIntValue(env, class_flash_block, "svn", obj_flash_block, flash_block[i].svn);
		jniCopyIntValue(env, class_flash_block, "len", obj_flash_block, flash_block[i].len);
		jniCopyIntValue(env, class_flash_block, "crc", obj_flash_block, flash_block[i].crc);
		jniCopyIntValue(env, class_flash_block, "run", obj_flash_block, flash_block[i].run);
		
		/*jni_copy_simple_class(env,class_flash_block,obj_flash_block,
	                         TRIPLES(int, &(flash_block[i]), flash_addr),
	                         TRIPLES(int, &(flash_block[i]), valid),
	                         TRIPLES(int, &(flash_block[i]), soft_ver),
	                         TRIPLES(int, &flash_block[i], svn),
	                         TRIPLES(int, &flash_block[i], len),
	                         TRIPLES(int, &flash_block[i], crc),
	                         TRIPLES(int, &flash_block[i], run),
		                     JNI_VAR_ARG_END);*/
		(*env)->CallBooleanMethod(env, blocks, arraylist_add, obj_flash_block);
	}
	SAFE_DEL_LOCAL_REF(class_flash_block);
	return ret;
}

