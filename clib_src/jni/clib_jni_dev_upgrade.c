#include "clib_jni.h"
#include "clib_jni_sdk.h"

JNIEXPORT jint JNICALL
NAME (ClDevUpgradeCheck)(JNIEnv* env, jobject this, jint dev_handle, jint lang) {
	return cl_dev_upgrade_check(dev_handle, lang);
}

JNIEXPORT jobject JNICALL
NAME (ClGetDevStateInfoV2)(JNIEnv* env, jobject this, jint slave_handle) 
{
	jclass class_version = NULL;
	jobject obj_version = NULL;
	cl_dev_stat_t * version_info = NULL;
	char buffer[256] = {0};
	cl_version_t *ver = NULL;

	class_version = (*env)->FindClass(env, CLASS_SDK_MODULE_VERSION);	
	obj_version = (*env)->AllocObject(env, class_version);
	version_info = cl_get_dev_stat_info(slave_handle);
	if (version_info == NULL) {
		SAFE_DEL_LOCAL_REF(obj_version);
		goto done;
	}
	
	ver = &version_info->soft_version;
	sprintf(buffer, "%d.%d.%d", ver->major, ver->minor, ver->revise);
	jniCopyString(env, class_version, "soft_version", obj_version, buffer);

	ver = &version_info->upgrade_version;
	sprintf(buffer, "%d.%d.%d", ver->major, ver->minor, ver->revise);
	jniCopyString(env, class_version, "upgrade_version", obj_version, buffer);

	ver = &version_info->new_version;
	sprintf(buffer, "%d.%d.%d", ver->major, ver->minor, ver->revise);
	jniCopyString(env, class_version, "new_version", obj_version, buffer);

	jniCopyBooleanValue(env, class_version, "can_update", obj_version, version_info->can_update);
	jniCopyString(env, class_version, "release_desc", obj_version, version_info->release_desc);

	jniCopyString(env, class_version, "release_url", obj_version, version_info->release_url);

	jniCopyString(env, class_version, "release_date", obj_version, version_info->release_date);
	jniCopyBooleanValue(env, class_version, "stm_can_update", obj_version, version_info->stm_can_update);
	
	ver = &version_info->stm_cur_version;
	sprintf(buffer, "%d.%d.%d", ver->major, ver->minor, ver->revise);
	jniCopyString(env, class_version, "stm_cur_version", obj_version, buffer);

	ver = &version_info->stm_newest_version;
	sprintf(buffer, "%d.%d.%d", ver->major, ver->minor, ver->revise);
	jniCopyString(env, class_version, "stm_newest_version", obj_version, buffer);
	jniCopyString(env, class_version, "stm_release_url", obj_version, version_info->stm_release_url);

done:
	SAFE_DEL_LOCAL_REF(class_version);
	if (version_info != NULL) {
		cl_free_dev_stat_info(version_info);
	}
	return obj_version;
}


JNIEXPORT jobject JNICALL
NAME (ClGetDevStateInfo)(JNIEnv* env, jobject this, jint slave_handle) {
	jclass class_dev_info;
	jobject obj_dev_info = NULL;
	char buffer[256] = {0};
	cl_version_t version;

	cl_dev_stat_t * dev_info = NULL;

	class_dev_info = (*env)->FindClass(env, CLASS_UPGRADE_DEV_INFO);	
	if(NULL == class_dev_info){
		return class_dev_info;
	}
	
	obj_dev_info = (*env)->AllocObject(env, class_dev_info);
	if (!obj_dev_info) {
		(*env)->DeleteLocalRef(env, class_dev_info);
		return obj_dev_info;
	}

	dev_info = cl_get_dev_stat_info(slave_handle);
	if(dev_info == NULL){
		(*env)->DeleteLocalRef(env, class_dev_info);
		(*env)->DeleteLocalRef(env, obj_dev_info);
		return NULL;
	}

	version = dev_info->soft_version;
	sprintf(buffer, "%d.%d.%d", version.major, version.minor, version.revise);
	
	jniCopyString(env, class_dev_info, "soft_version", obj_dev_info, buffer);
			
	version = dev_info->upgrade_version;
	sprintf(buffer, "%d.%d.%d", version.major, version.minor, version.revise);
	
	jniCopyString(env, class_dev_info, "upgrade_version", obj_dev_info,buffer);

	version = dev_info->new_version;
	sprintf(buffer, "%d.%d.%d", version.major, version.minor, version.revise);
	
	jniCopyString(env, class_dev_info, "new_version", obj_dev_info,
			buffer);

	jniCopyBooleanValue(env, class_dev_info, "can_update", obj_dev_info,
			dev_info->can_update);

	jniCopyBooleanValue(env, class_dev_info, "can_auto_update", obj_dev_info,
			dev_info->can_auto_update);

	jniCopyString(env, class_dev_info, "release_desc", obj_dev_info,
			dev_info->release_desc);

	jniCopyString(env, class_dev_info, "release_url", obj_dev_info,
			dev_info->release_url);
	
	jniCopyString(env, class_dev_info, "ap_ssid", obj_dev_info, dev_info->ap_ssid);

	jniCopyString(env, class_dev_info, "ap_passwd", obj_dev_info, dev_info->ap_passwd);

	jniCopyString(env, class_dev_info, "release_date", obj_dev_info,
			dev_info->release_date);

	sprintf(buffer,"%u.%u.%u",dev_info->stm_cur_version.major,
				dev_info->stm_cur_version.minor,
				dev_info->stm_cur_version.revise);
	jniCopyString(env, class_dev_info, "stm_cur_version", obj_dev_info, buffer);

	sprintf(buffer,"%u.%u.%u",dev_info->stm_newest_version.major,
				dev_info->stm_newest_version.minor,
				dev_info->stm_newest_version.revise);
	jniCopyString(env, class_dev_info, "stm_newest_version", obj_dev_info, buffer);
	
	jniCopyBooleanValue(env, class_dev_info, "stm_can_update", obj_dev_info, dev_info->stm_can_update);

	jniCopyString(env, class_dev_info, "ap_ssid", obj_dev_info, dev_info->ap_ssid);

	jniCopyString(env, class_dev_info, "ap_passwd", obj_dev_info, dev_info->ap_passwd);

	jniCopyString(env, class_dev_info, "stm_release_url", obj_dev_info,
			dev_info->stm_release_url);

	cl_free_dev_stat_info(dev_info); //数据拷贝完成�?释放空间
	(*env)->DeleteLocalRef(env, class_dev_info);

	return obj_dev_info;
}

JNIEXPORT jint JNICALL
NAME (ClSetDevUpdate)(JNIEnv* env, jobject this, jint slave_handle) {
	return cl_dev_update_set(slave_handle);
}

JNIEXPORT jint JNICALL
NAME (ClDevUpdateNow)(JNIEnv* env, jobject this, jint slave_handle) {
	return cl_dev_update_now(slave_handle);
}

