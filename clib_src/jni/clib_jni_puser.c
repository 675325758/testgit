#include "clib_jni.h"


/************************************************************************************
		�ֻ��û��˺Ź���ӿ�
 ************************************************************************************/

JNIEXPORT jint JNICALL
NAME(ClUserRegister)(JNIEnv* env, jobject this, 
	jstring obj_number, jint callback_handle)
{
	RS ret = RS_ERROR;
	const char *str_number;
	jclass clazz;
	jfieldID fid;

	str_number = (*env)->GetStringUTFChars(env, obj_number, NULL);
	
	ret = cl_user_register(str_number, nactivCallback, (void *)(jlong)callback_handle);
	
	(*env)->ReleaseStringUTFChars(env, obj_number, str_number);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUserResetPasswd)(JNIEnv* env, jobject this, 
	jstring obj_number, jint callback_handle)
{
	RS ret = RS_ERROR;
	const char *str_number;
	jclass clazz;
	jfieldID fid;

	str_number = (*env)->GetStringUTFChars(env, obj_number, NULL);
	
	ret = cl_user_reset_passwd(str_number, nactivCallback, (void *)(jlong)callback_handle);
	
	(*env)->ReleaseStringUTFChars(env, obj_number, str_number);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUserSendVcode)(JNIEnv* env, jobject this, 
	jstring obj_number, jstring obj_pwd, jstring obj_vcode, jint callback_handle)
{
	RS ret = RS_ERROR;
	const char *str_number;
	const char *str_pwd;
	const char *str_vcode;
	jclass clazz;
	jfieldID fid;

	str_number = (*env)->GetStringUTFChars(env, obj_number, NULL);
	str_pwd = (*env)->GetStringUTFChars(env, obj_pwd, NULL);
	str_vcode = (*env)->GetStringUTFChars(env, obj_vcode, NULL);
	
	ret = cl_user_send_vcode(str_number, str_pwd, str_vcode, nactivCallback, (void *)(jlong)callback_handle);
	
	(*env)->ReleaseStringUTFChars(env, obj_number, str_number);
	(*env)->ReleaseStringUTFChars(env, obj_pwd, str_pwd);
	(*env)->ReleaseStringUTFChars(env, obj_vcode, str_vcode);

	return ret;
}

JNIEXPORT jobject JNICALL
NAME(ClUserAddDev)(JNIEnv* env, jobject this, 
	jint user_handle, jstring obj_name, jstring obj_pwd)
{
	RS ret = RS_ERROR;
	const char *str_name;
	const char *str_pwd;
	cl_handle_t dev_handle = 0;
	jclass obj_class;
	jobject retObj;
	jfieldID fid;

	str_name = (*env)->GetStringUTFChars(env, obj_name, NULL);
	str_pwd = (*env)->GetStringUTFChars(env, obj_pwd, NULL);
	
	obj_class = (*env)->FindClass(env, CLASS_OPER_RESULT);
	retObj = (*env)->AllocObject(env, obj_class);

	ret = cl_user_add_dev_v2(user_handle,&dev_handle, str_name, str_pwd);
	
	(*env)->ReleaseStringUTFChars(env, obj_name, str_name);
	(*env)->ReleaseStringUTFChars(env, obj_pwd, str_pwd);
	
	jniCopyIntValue(env,obj_class,"ret",retObj,ret);
	jniCopyIntValue(env,obj_class,"out_data_type",retObj,0x1);
	jniCopyIntValue(env,obj_class,"int_out",retObj,dev_handle);

	return retObj;
}


JNIEXPORT jint JNICALL
NAME(ClUserDelDev)(JNIEnv* env, jobject this, 
	jint dev_handle)
{
	return cl_user_del_dev(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClUserModifyLogin)(JNIEnv* env,jobject this, 
	jint user_handle,jstring obj_pwd,jint callback_handle)
{
	RS ret = RS_ERROR;
	const char *str_pwd;
	jclass clazz;
	jfieldID fid;
	
	str_pwd = (*env)->GetStringUTFChars(env, obj_pwd, NULL);
	
	ret = cl_user_modify_login(user_handle, str_pwd, nactivCallback, (void *)(jlong)callback_handle);
	
	(*env)->ReleaseStringUTFChars(env, obj_pwd, str_pwd);
	return ret;
}

JNIEXPORT jobject JNICALL
NAME(ClGetNotifyMsg)(JNIEnv* env, jobject this, jint user_handle)
{
	jclass class_notify;
	jobject obj_notify = NULL;
	notify_msg_t* notify_msg = NULL;

	notify_msg = cl_get_notify(user_handle);
	if(!notify_msg)
		return obj_notify;

	class_notify = (*env)->FindClass(env, CLASS_NOTIFY_MSG);
	obj_notify = (*env)->AllocObject(env, class_notify);

	if(!obj_notify){
		cl_free_notify(notify_msg);
		return obj_notify;
	}

	jniCopyLongValue(env,class_notify,"dev_sn",obj_notify,notify_msg->dev_sn);
	jniCopyIntValue(env,class_notify,"msg_time",obj_notify,notify_msg->msg_time);
	jniCopyLongValue(env,class_notify,"report_id",obj_notify,notify_msg->report_id);
	jniCopyIntValue(env,class_notify,"msg_type",obj_notify,notify_msg->msg_type);
	jniCopyIntValue(env,class_notify,"msg_format",obj_notify,notify_msg->msg_format);
	jniCopyIntValue(env,class_notify,"msg_len",obj_notify,notify_msg->msg_len);
	jniCopyString(env,class_notify,"msg",obj_notify,notify_msg->msg);
	jniCopyString(env,class_notify,"content",obj_notify,notify_msg->content);

	cl_free_notify(notify_msg);

	return obj_notify;
}



