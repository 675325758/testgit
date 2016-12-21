#include "clib_jni.h"


JNIEXPORT jint JNICALL
NAME(ClStartConnectAppServer)(JNIEnv* env, jobject this)
{
	return cl_start_connect_to_app_server();
}

JNIEXPORT jboolean JNICALL
NAME(ClGetAppServerInfo)(JNIEnv* env, jobject this)
{
	cl_app_server_connect_info_t c_info = {0};
	int ret;
	
	ret = cl_get_app_server_info(&c_info);
	if (ret != RS_OK) return false;
	LOGD("app server info cur_stat=%hhu server_ip=%u server_port=%hu", c_info.cur_stat, c_info.server_ip, c_info.server_port);

	return c_info.is_establish;
}

JNIEXPORT jint JNICALL
NAME(ClPushAppStatInfo)(JNIEnv* env, jobject this, jobject app_info)
{
	jclass clazz;
	jfieldID fid;
	jstring jstr;
	const char * str;
	cl_app_stat_info_t info ={0};
	int ret;

	
	clazz =  (*env)->FindClass(env, CLASS_APP_STAT_INFO);
		
	fid = (*env)->GetFieldID(env, clazz, "hard_ware_type", "I");
	info.hard_ware_type = (*env)->GetIntField(env, app_info, fid);

	fid = (*env)->GetFieldID(env, clazz, "manufacturer_info", "Ljava/lang/String;");
	jstr = (*env)->GetObjectField(env, app_info, fid);
	str = ((*env)->GetStringUTFChars(env, jstr, NULL));
	strncpy(info.manufacturer_info, str, sizeof(info.manufacturer_info) - 1);
	(*env)->ReleaseStringUTFChars(env, jstr, str);

	fid = (*env)->GetFieldID(env, clazz, "os_info", "Ljava/lang/String;");
	jstr = (*env)->GetObjectField(env, app_info, fid);
	str = ((*env)->GetStringUTFChars(env, jstr, NULL));
	strncpy(info.os_info, str, sizeof(info.os_info) - 1);
	(*env)->ReleaseStringUTFChars(env, jstr, str);

	fid = (*env)->GetFieldID(env, clazz, "app_version_info", "Ljava/lang/String;");
	jstr = (*env)->GetObjectField(env, app_info, fid);
	str = ((*env)->GetStringUTFChars(env, jstr, NULL));
	strncpy(info.app_version_info, str, sizeof(info.app_version_info) - 1);
	(*env)->ReleaseStringUTFChars(env, jstr, str);

	ret = cl_push_app_stat_info(&info);

	return ret;
}






