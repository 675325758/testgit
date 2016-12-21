#include "clib_jni.h"

/*  空气净化器 */
JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerOnOff)(JNIEnv* env, jobject this, jint dev_handle, jboolean onOff)
{
	return cl_ia_aircleaner_set_onoff(dev_handle,!!onOff);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerSpeed)(JNIEnv* env, jobject this, jint dev_handle, jint speed)
{
	return cl_ia_aircleaner_set_speed(dev_handle,speed);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerTimer)(JNIEnv* env, jobject this, jint dev_handle, jint tmin)
{
	if( tmin < 0 )
		tmin = 0;
	if( tmin > 24*60 )
		tmin = 24*60;

	return cl_ia_aircleaner_set_timer(dev_handle,tmin);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerUltra)(JNIEnv* env, jobject this, jint dev_handle, jboolean onOff)
{
	return cl_ia_aircleaner_set_ultra(dev_handle,!!onOff);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerAnion)(JNIEnv* env, jobject this, jint dev_handle, jboolean onOff)
{
	return cl_ia_aircleaner_set_anion(dev_handle,!!onOff);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerMode)(JNIEnv* env, jobject this, jint dev_handle, jint mode)
{
	return cl_ia_aircleaner_set_mode(dev_handle,mode);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerQueryAll)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_ia_aircleaner_query_all(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerSetTerilize)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_on, jint minute)
{
	return cl_ia_aircleaner_set_terilize(dev_handle, is_on, minute);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirCleanerAddPeriodicTimer)(JNIEnv* env, jobject this, jint dev_handle,jobject timer_info)
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
	
	
	return cl_ia_aircleaner_add_periodic_timer( dev_handle, &ti);
}

JNIEXPORT jint JNICALL
NAME(ClDelAirCleanerPeriodicTimer)(JNIEnv* env, jobject this, jint dev_handle, jint timer_id)
{
	return cl_ia_aircleaner_del_periodic_timer(dev_handle, timer_id);
}

/*  快热炉 */
JNIEXPORT jint JNICALL
NAME(ClSetAirHeaterOnOff)(JNIEnv* env, jobject this, jint dev_handle, jboolean onOff)
{
	return cl_ia_airheater_set_onoff(dev_handle,onOff);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirHeaterGear)(JNIEnv* env, jobject this, jint dev_handle, jint gear)
{
	return cl_ia_airheater_set_gear(dev_handle,gear);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirHeaterTime)(JNIEnv* env, jobject this, jint dev_handle, jint time)
{
	return cl_ia_airheater_set_time(dev_handle,time);
}

JNIEXPORT jint JNICALL
NAME(ClSetAirHeaterMode)(JNIEnv* env, jobject this, jint dev_handle, jint heater_mode)
{
	return cl_ia_airheater_set_mode(dev_handle,heater_mode);
}

/* 热水器 */
JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterWork)(JNIEnv* env, jobject this, jint dev_handle, jboolean work)
{
	return cl_ia_waterheater_set_work(dev_handle,work);
}

JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterTemp)(JNIEnv* env, jobject this, jint dev_handle, jint temp)
{
	return cl_ia_waterheater_set_temp(dev_handle,temp);
}

JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterTimer)(JNIEnv* env, jobject this, jint dev_handle, jint timer)
{
	return cl_ia_waterheater_set_timer(dev_handle,timer);
}

JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterCapactity)(JNIEnv* env, jobject this, jint dev_handle, jint capactity)
{
	return cl_ia_waterheater_set_capactity(dev_handle,capactity);
}

/*	前锋a9热水器 */

JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterA9ClearCnt)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_ia_waterheater_a9_clear_cnt(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterA9Mode)(JNIEnv* env, jobject this, jint dev_handle, jint mode)
{
	return cl_ia_waterheater_a9_set_mode(dev_handle, mode);
}

JNIEXPORT jint JNICALL
NAME(ClSetWaterheaterA9Temp)(JNIEnv* env, jobject this, jint dev_handle, jint temp)
{
	return cl_ia_waterheater_a9_set_temp(dev_handle, temp);
}


/* 智能空调 */
JNIEXPORT jint JNICALL
NAME(ClSetIaAircondition)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff, jint mode, jint temp)
{
	return cl_ia_aircondition_set(dev_handle, onoff, mode, temp);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaAirconditionTimer)(JNIEnv* env, jobject this, jint dev_handle, jint timer)
{
	if( timer < 0 )
		timer = 0;
	if( timer > 24*60 )
		timer = 24*60;
		
	return cl_ia_aircondition_set_timer(dev_handle, timer);
}

/* 风扇 */

JNIEXPORT jint JNICALL
NAME(ClSetIaEleFanWork)(JNIEnv* env, jobject this, jint dev_handle, jboolean work)
{
	return cl_ia_electricfan_set_work(dev_handle, work);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaEleFanGear)(JNIEnv* env, jobject this, jint dev_handle, jint gear)
{
	return cl_ia_electricfan_set_gear(dev_handle, gear);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaEleFanTimer)(JNIEnv* env, jobject this, jint dev_handle, jint timer)
{
	return cl_ia_electricfan_set_timer(dev_handle, timer);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaEleFanShark)(JNIEnv* env, jobject this, jint dev_handle, jboolean shake)
{
	return cl_ia_electricfan_set_shake(dev_handle, shake);
}

/*  联创油汀 */
JNIEXPORT jint JNICALL
NAME(ClSetIaAirHeaterYcytTemp)(JNIEnv* env, jobject this, jint dev_handle, jint temp)
{
	return cl_ia_airheater_ycyt_set_temp(dev_handle, temp);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaAirHeaterYcytMode)(JNIEnv* env, jobject this, jint dev_handle, jint ycyt_mode)
{
	return cl_ia_airheater_ycyt_set_mode(dev_handle, ycyt_mode);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaAirHeaterYcytGear)(JNIEnv* env, jobject this, jint dev_handle, jint gear)
{
	return cl_ia_airheater_ycyt_set_gear(dev_handle, gear);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaAirHeaterYcytTimer)(JNIEnv* env, jobject this, jint dev_handle, jint timer)
{
	return cl_ia_airheater_ycyt_set_timer(dev_handle, timer);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaAirHeaterYcytOnoff)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_ia_airheater_ycyt_set_onoff(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaAirHeaterYcytTimerOn)(JNIEnv* env, jobject this, jint dev_handle, jint time)
{
	return cl_ia_airheater_ycyt_set_order_timer(dev_handle, time);
}

JNIEXPORT jint JNICALL
NAME(ClRefreshAirHeaterYcytTimer)(JNIEnv* env, jobject this,  jint dev_handle)
{
	return cl_ia_airheater_ycyt_refresh_timer( dev_handle);
}


/**奥普浴霸***/
JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterWork)(JNIEnv* env, jobject this, jint dev_handle, jboolean onoff)
{
	return cl_ia_bath_heater_set_work(dev_handle, onoff);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterAnion)(JNIEnv* env, jobject this, jint dev_handle, jboolean anion)
{
	return cl_ia_bath_heater_set_anion(dev_handle, anion);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterLight)(JNIEnv* env, jobject this, jint dev_handle, jboolean light)
{
	return cl_ia_bath_heater_set_light(dev_handle, light);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterBreath)(JNIEnv* env, jobject this, jint dev_handle, jboolean breath)
{
	return cl_ia_bath_heater_set_breath(dev_handle, breath);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterDry)(JNIEnv* env, jobject this, jint dev_handle, jboolean dry)
{
	return cl_ia_bath_heater_set_dry(dev_handle, dry);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterTronic)(JNIEnv* env, jobject this, jint dev_handle, jint tronic)
{
	return cl_ia_bath_heater_set_tronic(dev_handle, tronic);
}

JNIEXPORT jint JNICALL
NAME(ClSetIaBathHeaterTimer)(JNIEnv* env, jobject this, jint dev_handle, jint timer)
{
	return cl_ia_bath_heater_set_timer(dev_handle, timer);
}

/** 云空调***/


JNIEXPORT jint JNICALL
NAME(ClCloudMatch)(JNIEnv* env, jobject this, jint ac_handle, jboolean do_match, jint match_type)
{
	return cl_cloud_match(ac_handle, do_match, match_type);
}

JNIEXPORT jint JNICALL
NAME(ClAcSendCtl)(JNIEnv* env, jobject this, jint ac_handle, jboolean onoff, jint temp, jint mode, jint speed, jint dir, jint presskey,jint oldkey)
{
	return cl_ac_send_ctl( ac_handle, !onoff,  temp,  mode,  speed,  dir,  presskey,  oldkey);
}

JNIEXPORT jint JNICALL
NAME(ClAcSetMatchId)(JNIEnv* env, jobject this, jint ac_handle, jint select_match_id, jint match_type)
{
	return cl_ac_set_match_id( ac_handle,select_match_id, match_type);
}


JNIEXPORT jint JNICALL
NAME(ClSetScreenOnOff)(JNIEnv* env, jobject this, jint slave_handle, jboolean on_off)
{
	return cl_805_set_screen_on_off(slave_handle, on_off);
}

JNIEXPORT jint JNICALL
NAME(ClSetBeepOnOff)(JNIEnv* env, jobject this, jint slave_handle, jboolean on_off)
{
	return cl_805_set_beep_on_off(slave_handle, on_off);
}

JNIEXPORT jint JNICALL
NAME(ClQueryScreenOnOff)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_805_query_screen_on_off(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClQueryBeepOnOff)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_805_query_beep_on_off(slave_handle);
}


