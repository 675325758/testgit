#include "clib_jni.h"
#include "clib_jni_linkage.h"
#include "clib_jni_sdk.h"

/************************************************************************************

		Device Probe interface 
		局域网扫描设备相关接口
 ************************************************************************************/
 
 JNIEXPORT jint JNICALL
NAME(ClStartProbeDevices)(JNIEnv* env, jobject this)
{
	return cl_set_probe_callback(nactivCallback, NULL);
}

JNIEXPORT jint JNICALL
NAME(ClStopProbeDevices)(JNIEnv* env, jobject this)
{
	return cl_set_probe_callback(NULL, NULL);
}
static jobjectArray copy_lan_dev(JNIEnv* env, cl_lan_dev_list *ldi)
{
	int i;
	jfieldID fid;
	jclass class_dev_info;
	jstring str;
	jobjectArray dev_info_array;
	jobject dev_info;
	jclass class_lk_lan = NULL;
	jobject obj_lk_lan = NULL;

	if (ldi && ldi->dev_count > 0) {
		class_dev_info = (*env)->FindClass(env, CLASS_LAN_DEV_INFO);
		dev_info_array = (*env)->NewObjectArray(env, ldi->dev_count, class_dev_info, NULL);

		for (i = 0; i < ldi->dev_count; i++) {
			dev_info = (*env)->AllocObject(env, class_dev_info);

			fid = (*env)->GetFieldID(env, class_dev_info, "dev_sn", "J");
			(*env)->SetLongField(env, dev_info, fid, ldi->info[i].dev_sn);

			fid = (*env)->GetFieldID(env, class_dev_info, "dev_type", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].dev_type);

			fid = (*env)->GetFieldID(env, class_dev_info, "last_alive_time", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].last_alive_time);

			fid = (*env)->GetFieldID(env, class_dev_info, "sm_success_time", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].sm_success_time);

			fid = (*env)->GetFieldID(env, class_dev_info, "dev_run_mode", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].dev_run_mode);

			fid = (*env)->GetFieldID(env, class_dev_info, "ext_type", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].exp_type);
			//拷贝开发者ID
			if(ldi->info[i].developer_id != NULL){
				jniCopyString(env,class_dev_info,"developer_id",dev_info,ldi->info[i].developer_id);
			}
			
			JNI_COPY_SIMPLE_CLASS(env, class_dev_info, dev_info, CLASS_LINKAGE_LAN_INFO, la_info,
				                     TRIPLES(boolean, &ldi->info[i].la_info, is_valid),
				                     TRIPLES(boolean, &ldi->info[i].la_info, is_la_new),
				                     TRIPLES(int, &ldi->info[i].la_info, home_id),
				                     JNI_VAR_ARG_END);

			(*env)->SetObjectArrayElement(env, dev_info_array, i, dev_info);	
			
			(*env)->DeleteLocalRef(env, dev_info); // 释放
		}

		//(*env)->DeleteLocalRef(env, dev_info_array);
		(*env)->DeleteLocalRef(env, class_dev_info);

		return dev_info_array;
	}

	return NULL;
}

static jobjectArray copy_lan_dev_v2(JNIEnv* env, cl_lan_dev_list *ldi)
{
	int i;
	jfieldID fid;
	jclass class_dev_info;
	jstring str;
	jobjectArray dev_info_array;
	jobject dev_info;

	if (ldi && ldi->dev_count > 0) {
		class_dev_info = (*env)->FindClass(env, CLASS_SDK_LAN_DEV_INFO);
		dev_info_array = (*env)->NewObjectArray(env, ldi->dev_count, class_dev_info, NULL);

		for (i = 0; i < ldi->dev_count; i++) {
			dev_info = (*env)->AllocObject(env, class_dev_info);

			fid = (*env)->GetFieldID(env, class_dev_info, "handle", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].handle);

			fid = (*env)->GetFieldID(env, class_dev_info, "dev_sn", "J");
			(*env)->SetLongField(env, dev_info, fid, ldi->info[i].dev_sn);

			fid = (*env)->GetFieldID(env, class_dev_info, "dev_type", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].dev_type);

			fid = (*env)->GetFieldID(env, class_dev_info, "last_alive_time", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].last_alive_time);

			fid = (*env)->GetFieldID(env, class_dev_info, "sm_success_time", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].sm_success_time);

			fid = (*env)->GetFieldID(env, class_dev_info, "dev_run_mode", "I");
			(*env)->SetIntField(env, dev_info, fid, ldi->info[i].dev_run_mode);

			fid = (*env)->GetFieldID(env, class_dev_info, "ext_type", "B");
			(*env)->SetByteField(env, dev_info, fid, ldi->info[i].exp_type);

			jniCopyString(env,class_dev_info,"developer_id",dev_info,ldi->info[i].developer_id);

			(*env)->SetObjectArrayElement(env, dev_info_array, i, dev_info);	
			
			(*env)->DeleteLocalRef(env, dev_info); // 释放
		}

		//(*env)->DeleteLocalRef(env, dev_info_array);
		(*env)->DeleteLocalRef(env, class_dev_info);

		return dev_info_array;
	}

	return NULL;
}


JNIEXPORT jobjectArray JNICALL
NAME(ClGetProbeListV2)(JNIEnv* env, jobject this)
{
	jobjectArray dev_info_array;
	int ret = -1;
	cl_lan_dev_list* dev_list = NULL;
	
	ret = cl_get_probe_dev_list(&dev_list);
	if(ret < 0|| dev_list == NULL)
		return NULL;
	dev_info_array = copy_lan_dev_v2(env, dev_list);
	cl_free_probe_dev_list(dev_list);
	
	return dev_info_array;
}


JNIEXPORT jobjectArray JNICALL
NAME(ClGetProbeList)(JNIEnv* env, jobject this)
{
	jobjectArray dev_info_array;
	int ret = -1;
	cl_lan_dev_list* dev_list = NULL;
	
	ret = cl_get_probe_dev_list(&dev_list);
	if(ret < 0|| dev_list == NULL)
		return NULL;
	dev_info_array = copy_lan_dev(env, dev_list);
	cl_free_probe_dev_list(dev_list);
	
	return dev_info_array;
}


JNIEXPORT jint JNICALL
NAME(ClAdvanceSmartConfig)(JNIEnv* env, jobject this,jstring ssid,jstring passwd, jbyte mode)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;

	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);

	ret = cl_advance_smart_config_start(ssid_str,password, mode);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}



JNIEXPORT jint JNICALL
NAME(ClStartSmartConfig)(JNIEnv* env, jobject this,jstring ssid,jstring passwd)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;

	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);

	ret = cl_smart_config_start(ssid_str,password);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClStartSmartConfigByAirKiss)(JNIEnv* env, jobject this,jstring ssid,jstring passwd,jbyte mtime,jbyte mitime,jbyte btime,jbyte bitime)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;

	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);
	
	ret = cl_smart_config_start_ext(ssid_str, password, mtime, mitime, btime, bitime);	

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}


JNIEXPORT jint JNICALL
NAME(ClStartSmartMboardcastConfig)(JNIEnv* env, jobject this, jstring ssid, jstring passwd)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;
	
	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);

	ret = cl_smart_mbroadcast_config_start(ssid_str,password);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSmartMbroadcastConfigStartHotspot)(JNIEnv* env, jobject this, jstring ssid, jstring passwd)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;
	
	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);

	ret = cl_smart_mbroadcast_config_start_hotspot(ssid_str, password);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClStartSmartMboardcastConfigRaw)(JNIEnv* env, jobject this, jbyteArray ssid, jstring passwd)
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
	ret = cl_smart_mbroadcast_config_start(strs,password);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseByteArrayElements(env, ssid, (jbyte*)ssid_str, JNI_ABORT);
	free(strs);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClStartSmartPhoneWlanConfig)(JNIEnv* env, jobject this, jstring ssid, jstring passwd,jint auth_mode)
{
	const char* password = NULL;
	const char* ssid_str;
	RS ret;
	
	password = (*env)->GetStringUTFChars(env, passwd, NULL);
	ssid_str = (*env)->GetStringUTFChars(env, ssid, NULL);

	ret = cl_phone_wlan_config_start(ssid_str,password,auth_mode);

	(*env)->ReleaseStringUTFChars(env, passwd, password);
	(*env)->ReleaseStringUTFChars(env, ssid, ssid_str);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClStopSmartConfig)(JNIEnv* env, jobject this)
{
	return cl_smart_config_stop();
}

JNIEXPORT jlong JNICALL
NAME(ClGetApSmartConfigDestSn)(JNIEnv* env, jobject this)
{
	return cl_get_ap_dest_sn();
}


