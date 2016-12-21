#include "clib_jni.h"

/************************************************************************************
		plug interface 
 ************************************************************************************/

JNIEXPORT jint JNICALL
NAME(ClPlugQueryStart)(JNIEnv* env, jobject this, jint slave_handle, jint seconds, jint callback_handle)
{
	return cl_plug_query_start(slave_handle, seconds, nactivCallback, (void *)(jlong)callback_handle);
}

JNIEXPORT jint JNICALL
NAME(ClPlugQueryStop)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_plug_query_stop(slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClPlugTurnOn)(JNIEnv* env, jobject this, jint slave_handle, jboolean on)
{
	return cl_plug_turn_on(slave_handle, 0, on);
}

static jobjectArray copy_plug_timer(JNIEnv* env, cl_plug_info_t *pi)
{
	int i;
	jfieldID fid;
	jclass class_timer;
	jobjectArray timer_array;
	jobject timer;

	class_timer = (*env)->FindClass(env, CLASS_PLUG_TIMER);
	if (pi->num_timer > 0) {
		timer_array = (*env)->NewObjectArray(env, pi->num_timer, class_timer, NULL);
	} else {
		timer_array = NULL;
	}
	for (i = 0; i < pi->num_timer; i++) {
		timer = (*env)->AllocObject(env, class_timer);

		fid = (*env)->GetFieldID(env, class_timer, "id", "I");
		(*env)->SetIntField(env, timer, fid, pi->timer[i]->id);

		fid = (*env)->GetFieldID(env, class_timer, "hour", "I");
		(*env)->SetIntField(env, timer, fid, pi->timer[i]->hour);

		fid = (*env)->GetFieldID(env, class_timer, "minute", "I");
		(*env)->SetIntField(env, timer, fid, pi->timer[i]->minute);

		fid = (*env)->GetFieldID(env, class_timer, "week", "I");
		(*env)->SetIntField(env, timer, fid, pi->timer[i]->week);
		
		fid = (*env)->GetFieldID(env, class_timer, "enable", "Z");
		(*env)->SetBooleanField(env, timer, fid, pi->timer[i]->enable);

		fid = (*env)->GetFieldID(env, class_timer, "last", "I");
		(*env)->SetIntField(env, timer, fid, pi->timer[i]->last);

		fid = (*env)->GetFieldID(env, class_timer, "sort", "I");
		(*env)->SetIntField(env, timer, fid, pi->timer[i]->sort);

		jniCopyString(env, class_timer, "name", timer, pi->timer[i]->name);

		(*env)->SetObjectArrayElement(env, timer_array, i, timer);	

		(*env)->DeleteLocalRef(env, timer);	
	}

	//if (timer_array != NULL)
	//	(*env)->DeleteLocalRef(env, timer_array);
	(*env)->DeleteLocalRef(env, class_timer);

	return timer_array;
}

JNIEXPORT jobject JNICALL 
NAME(ClPlugGetInfo)(JNIEnv* env, jobject this, jint slave_handle, jint tz)
{
	cl_plug_info_t *pi;
	jclass clazz;
	jfieldID fid;
	jobject obj;
	jobjectArray obj_timer_arr;

	if ((pi = cl_plug_get_info(slave_handle, 0, tz)) == NULL)
		return NULL;

	clazz = (*env)->FindClass(env, CLASS_PLUG_INFO);
	obj = (*env)->AllocObject(env, clazz);

	fid = (*env)->GetFieldID(env, clazz, "is_on", "Z");
	(*env)->SetBooleanField(env, obj, fid, pi->is_on);

	fid = (*env)->GetFieldID(env, clazz, "current", "I");
	(*env)->SetIntField(env, obj, fid, pi->current);

	fid = (*env)->GetFieldID(env, clazz, "voltage", "I");
	(*env)->SetIntField(env, obj, fid, pi->voltage);

	fid = (*env)->GetFieldID(env, clazz, "temperature", "I");
	(*env)->SetIntField(env, obj, fid, pi->temperature);

	fid = (*env)->GetFieldID(env, clazz, "electric_total", "I");
	(*env)->SetIntField(env, obj, fid, pi->electric_total);

	fid = (*env)->GetFieldID(env, clazz, "electric_section", "I");
	(*env)->SetIntField(env, obj, fid, pi->electric_section);

	fid = (*env)->GetFieldID(env, clazz, "section_begin_time", "I");
	(*env)->SetIntField(env, obj, fid, pi->section_begin_time);


	fid = (*env)->GetFieldID(env, clazz, "next_effect", "Z");
	(*env)->SetBooleanField(env, obj, fid, pi->next_effect);

	fid = (*env)->GetFieldID(env, clazz, "next_on", "Z");
	(*env)->SetBooleanField(env, obj, fid, pi->next_on);

	fid = (*env)->GetFieldID(env, clazz, "next_minute", "I");
	(*env)->SetIntField(env, obj, fid, pi->next_minute);

	obj_timer_arr = copy_plug_timer(env, pi);
	// 数组构造完毕，赋值
	fid = (*env)->GetFieldID(env, clazz, "timer", "[L" CLASS_PLUG_TIMER ";");
	(*env)->SetObjectField(env, obj, fid, obj_timer_arr);

	(*env)->DeleteLocalRef(env, obj_timer_arr);
	(*env)->DeleteLocalRef(env, clazz);

	return obj;
}

JNIEXPORT jint JNICALL
NAME(ClPlugTimerSet)(JNIEnv* env, jobject this, jint slave_handle, jobject obj_timer, jint tz)
{
	jint ret = 0;
	cl_plug_timer_t pt;
	jclass clazz;
	jfieldID fid;
	jobject obj_name;

	memset(&pt, 0, sizeof(cl_plug_timer_t));
	clazz = (*env)->FindClass(env, CLASS_PLUG_TIMER);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	pt.id = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	pt.hour = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	pt.minute = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	pt.week = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	pt.enable = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "last", "I");
	pt.last = (*env)->GetIntField(env, obj_timer, fid);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	obj_name = (*env)->GetObjectField(env, obj_timer, fid);
	pt.name = (char *)((*env)->GetStringUTFChars(env, obj_name, NULL));

	ret = cl_plug_timer_set(slave_handle, 0, &pt, tz);

	(*env)->ReleaseStringUTFChars(env, obj_name, pt.name);

	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClPlugTimerDel)(JNIEnv* env, jobject this, jint slave_handle, jint id)
{
	return cl_plug_timer_del(slave_handle, 0, id);
}

JNIEXPORT jint JNICALL
NAME(ClPlugClearElectricStat)(JNIEnv* env, jobject this, jint slave_handle)
{
	return cl_plug_clear_electric_stat(slave_handle, 0);
}
