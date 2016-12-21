#include "clib_jni.h"

/************************************************************************************

		E宝接口
 ************************************************************************************/ 
JNIEXPORT jint JNICALL
NAME(ClEbCtrlLedPower)(JNIEnv* env, jobject this, jint dev_handle, jbyte onoff)
{
	return cl_eb_ctrl_led_power(dev_handle,onoff);
}

JNIEXPORT jint JNICALL
NAME(ClEbCtrlWork)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_on)
{
	return cl_eb_ctrl_work(dev_handle,is_on);
}

JNIEXPORT jint JNICALL
NAME(ClEbTimerDel)(JNIEnv* env, jobject this, jint dev_handle, jint id)
{
	return cl_eb_timer_del(dev_handle, id);
}

JNIEXPORT jint JNICALL
NAME(ClEbTimerSet)(JNIEnv* env, jobject this,  jint dev_handle, jobject obj_timer)
{
	jint ret = 0;
	cl_air_timer_t e_plug_timer;
	jclass clazz;
	jfieldID fid;
	
	LOGD("xxxxxx set eplug timer\n");
	memset(&e_plug_timer, 0, sizeof(cl_air_timer_t));
	
	clazz = (*env)->FindClass(env, CLASS_E_PLUG_TIMER);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	e_plug_timer.id= (*env)->GetIntField(env, obj_timer, fid);
	
	
	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	e_plug_timer.enable = (*env)->GetBooleanField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	e_plug_timer.hour = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	e_plug_timer.minute = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "onoff", "Z");
	e_plug_timer.onoff = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	e_plug_timer.week= (*env)->GetIntField(env, obj_timer, fid);

	LOGD("xxxxxx eplug timer id = %d, hour = %d, min = %d, onoff = %dr\n", e_plug_timer.id, e_plug_timer.hour, e_plug_timer.minute, e_plug_timer.onoff);

	ret = cl_eb_timer_set(dev_handle, &e_plug_timer);
			
	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClEbPeriodTimerDel)(JNIEnv* env, jobject this, jint dev_handle, jint id)
{
	return cl_eb_period_timer_del(dev_handle, id);
}

JNIEXPORT jint JNICALL
NAME(ClEbPeriodTimerSet)(JNIEnv* env, jobject this,  jint dev_handle, jobject obj_timer)
{
	jint ret = 0;
	cl_period_timer_t period_timer;
	jclass clazz;
	jfieldID fid;

	if(obj_timer == NULL)
		return -5; //无效参数

	memset(&period_timer, 0, sizeof(cl_period_timer_t));
	clazz = (*env)->FindClass(env, CLASS_E_PLUG_PERIOD_TIMER);

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

	ret = cl_eb_period_timer_set(dev_handle, &period_timer);

	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

