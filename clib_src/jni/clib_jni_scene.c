#include "clib_jni.h"

/************************************************************************************

		scene interface 
		情景相关接口
 ************************************************************************************/
 
 JNIEXPORT jint JNICALL
NAME(ClSceneAdd)(JNIEnv* env, jobject this, jint user_handle, jbyte img_resv, jstring scene_name)
{
	int scene_handle;
	const char *name;
	RS ret = RS_ERROR;
	
	name = (*env)->GetStringUTFChars(env, scene_name, NULL);
	ret = cl_scene_add(user_handle, &scene_handle, img_resv, name);
	
	(*env)->ReleaseStringUTFChars(env, scene_name, name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSceneDel)(JNIEnv* env, jobject this, jint scene_handle)
{
    return cl_scene_del(scene_handle);
}

JNIEXPORT jint JNICALL
NAME(ClSceneChangeName)(JNIEnv* env, jobject this, jint scene_handle, jstring scene_name)
{
	const char *name;
	RS ret = RS_ERROR;
	
	name = (*env)->GetStringUTFChars(env, scene_name, NULL);
	ret = cl_scene_change_name(scene_handle, name);
	
	(*env)->ReleaseStringUTFChars(env, scene_name, name);
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSceneChangeImageResv)(JNIEnv* env, jobject this, jint scene_handle, jbyte img_resv)
{
    return cl_scene_change_img_resv(scene_handle, img_resv);
}

JNIEXPORT jint JNICALL
NAME(ClSceneModifyEvents)(JNIEnv* env, jobject this, jint scene_handle, jobjectArray events, jint event_count)
{
	cl_scene_event_t *events_array;
	cl_scene_event_t *even;
	jclass class_scene_event;
	jobject obj_even;
	jintArray ac_value_array;
	jfieldID fid;
	jstring obj_str;
	const char *str;
	int *ac_value;
	int i, j, ret;
	
	class_scene_event = (*env)->FindClass(env, CLASS_LAN_DEV_INFO);
	
	events_array = (cl_scene_event_t *)malloc(event_count * sizeof(*events_array));

	
	for(i = 0; i < event_count; i++)
	{
	    obj_even = (jobject)(*env)->GetObjectArrayElement(env, events, i);
	    even = &events_array[i];
	    
		fid = (*env)->GetFieldID(env, class_scene_event, "enent_type", "I");
		even->enent_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "ev_type", "I");
		even->ev_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "obj_handle", "I");
		even->obj_handle = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "event_name", "Ljava/lang/String;");
		obj_str = (*env)->GetObjectField(env, obj_even, fid);
		str = ((*env)->GetStringUTFChars(env, obj_str, NULL));
		strncpy(even->event_name, str, sizeof(even->event_name) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "ac_values", "[I");
		ac_value_array = (*env)->GetObjectField(env, obj_even, fid);
		even->action_num = (*env)->GetArrayLength(env, ac_value_array);
		
		memcpy(even->ac_values, (*env)->GetIntArrayElements(env, ac_value_array, 0), even->action_num * sizeof(int));
		//&even->ac_values = (*env)->GetIntArrayElements(env, ac_value_array, 0);
	}

	ret = cl_scene_modify_events(scene_handle, event_count, &events_array);
	free(events_array);
	(*env)->DeleteLocalRef(env, class_scene_event);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSceneExec)(JNIEnv* env, jobject this, jint scene_handle)
{
	return cl_scene_exec(scene_handle);
}


JNIEXPORT jint JNICALL
NAME(ClSceneModify)(JNIEnv* env, jobject this, 
	jint scene_handle, jstring scene_name, jint img_resv,jint event_count, jobjectArray events)
{
	RS ret = RS_ERROR;

	const char *name;
	
	cl_scene_event_t **events_array;
	cl_scene_event_t *event;
	jclass class_scene_event;
	jobject obj_even;
	jintArray ac_value_array;
	jfieldID fid;
	jstring obj_str;
	const char *str;
	int *ac_value;
	int i, j, action_num;
	
	
	name = (*env)->GetStringUTFChars(env, scene_name, NULL);
	
	class_scene_event = (*env)->FindClass(env, CLASS_SCENE_EVEN);
	events_array = (cl_scene_event_t **)malloc(event_count * sizeof(cl_scene_event_t *));
	
	for(i = 0; i < event_count; i++)
	{
	    obj_even = (jobject)(*env)->GetObjectArrayElement(env, events, i);
		fid = (*env)->GetFieldID(env, class_scene_event, "ac_values", "[I");
		ac_value_array = (*env)->GetObjectField(env, obj_even, fid);
		action_num = (*env)->GetArrayLength(env, ac_value_array);

	    event = calloc(sizeof(*event) + sizeof(int)*action_num, 1);
		events_array[i] = event;
	    
		fid = (*env)->GetFieldID(env, class_scene_event, "enent_type", "I");
		event->enent_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "ev_type", "I");
		event->ev_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "obj_handle", "I");
		event->obj_handle = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "event_name", "Ljava/lang/String;");
		obj_str = (*env)->GetObjectField(env, obj_even, fid);
		str = ((*env)->GetStringUTFChars(env, obj_str, NULL));
		if (str != NULL) {
			strncpy(event->event_name, str, sizeof(event->event_name) - 1);
		}
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
		
		event->action_num = action_num;
		
		memcpy(event->ac_values, (*env)->GetIntArrayElements(env, ac_value_array, 0), event->action_num * sizeof(int));
		//&even->ac_values = (*env)->GetIntArrayElements(env, ac_value_array, 0);
	}
	
	
	ret = cl_scene_modify(scene_handle, name, img_resv, event_count,events_array);
	
	(*env)->ReleaseStringUTFChars(env, scene_name, name);
	for (i = 0; i < event_count; i++) {
		free(events_array[i]);
	}
	free(events_array);
	(*env)->DeleteLocalRef(env, class_scene_event);
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClSceneAdd3)(JNIEnv* env, jobject this, 
	jint user_handle, jint scene_handle, jstring scene_name, jint img_resv,
	jint event_count, jobjectArray events)
{
	RS ret = RS_ERROR;

	const char *name;
	
	cl_scene_event_t **events_array;
	cl_scene_event_t *event;
	jclass class_scene_event;
	jobject obj_even;
	jintArray ac_value_array;
	jfieldID fid;
	jstring obj_str;
	const char *str;
	int *ac_value;
	int i, j, action_num;
	
	
	name = (*env)->GetStringUTFChars(env, scene_name, NULL);
	
	class_scene_event = (*env)->FindClass(env, CLASS_SCENE_EVEN);
	events_array = (cl_scene_event_t **)malloc(event_count * sizeof(cl_scene_event_t *));
	
	for(i = 0; i < event_count; i++)
	{
	    obj_even = (jobject)(*env)->GetObjectArrayElement(env, events, i);
		fid = (*env)->GetFieldID(env, class_scene_event, "ac_values", "[I");
		ac_value_array = (*env)->GetObjectField(env, obj_even, fid);
		action_num = (*env)->GetArrayLength(env, ac_value_array);

	    event = calloc(sizeof(*event) + sizeof(int)*action_num, 1);
		events_array[i] = event;
	    
		fid = (*env)->GetFieldID(env, class_scene_event, "enent_type", "I");
		event->enent_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "ev_type", "I");
		event->ev_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "obj_handle", "I");
		event->obj_handle = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "event_name", "Ljava/lang/String;");
		obj_str = (*env)->GetObjectField(env, obj_even, fid);
		str = ((*env)->GetStringUTFChars(env, obj_str, NULL));
		if (str != NULL) {
			strncpy(event->event_name, str, sizeof(event->event_name) - 1);
		}
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
		
		event->action_num = action_num;
		
		memcpy(event->ac_values, (*env)->GetIntArrayElements(env, ac_value_array, 0), event->action_num * sizeof(int));
		//&even->ac_values = (*env)->GetIntArrayElements(env, ac_value_array, 0);
	}
	
	
	ret = cl_scene_add_3(user_handle, &scene_handle, name, img_resv, event_count, events_array);
	
	(*env)->ReleaseStringUTFChars(env, scene_name, name);
	for (i = 0; i < event_count; i++) {
		free(events_array[i]);
	}
	free(events_array);
	(*env)->DeleteLocalRef(env, class_scene_event);
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUDISpecialSceneAdd3)(JNIEnv* env, jobject this, 
	jint user_handle, jint scene_handle, jstring scene_name, jint img_resv,
	jint event_count, jobjectArray events)
{
	RS ret = RS_ERROR;

	const char *name;
	
	cl_scene_event_t **events_array;
	cl_scene_event_t *event;
	jclass class_scene_event;
	jobject obj_even;
	jintArray ac_value_array;
	jfieldID fid;
	jstring obj_str;
	const char *str;
	int *ac_value;
	int i, j, action_num;
	
	
	name = (*env)->GetStringUTFChars(env, scene_name, NULL);
	
	class_scene_event = (*env)->FindClass(env, CLASS_SCENE_EVEN);
	events_array = (cl_scene_event_t **)malloc(event_count * sizeof(cl_scene_event_t *));
	
	for(i = 0; i < event_count; i++)
	{
	    obj_even = (jobject)(*env)->GetObjectArrayElement(env, events, i);
		fid = (*env)->GetFieldID(env, class_scene_event, "ac_values", "[I");
		ac_value_array = (*env)->GetObjectField(env, obj_even, fid);
		action_num = (*env)->GetArrayLength(env, ac_value_array);

	    event = calloc(sizeof(*event) + sizeof(int)*action_num, 1);
		events_array[i] = event;
	    
		fid = (*env)->GetFieldID(env, class_scene_event, "enent_type", "I");
		event->enent_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "ev_type", "I");
		event->ev_type = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "obj_handle", "I");
		event->obj_handle = (*env)->GetIntField(env, obj_even, fid);
		
		fid = (*env)->GetFieldID(env, class_scene_event, "event_name", "Ljava/lang/String;");
		obj_str = (*env)->GetObjectField(env, obj_even, fid);
		str = ((*env)->GetStringUTFChars(env, obj_str, NULL));
		if (str != NULL) {
			strncpy(event->event_name, str, sizeof(event->event_name) - 1);
		}
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
		
		event->action_num = action_num;
		
		memcpy(event->ac_values, (*env)->GetIntArrayElements(env, ac_value_array, 0), event->action_num * sizeof(int));
		//&even->ac_values = (*env)->GetIntArrayElements(env, ac_value_array, 0);
	}
	
	
	ret = cl_scene_add_3(user_handle, &scene_handle, name, img_resv, event_count, events_array);
	
	(*env)->ReleaseStringUTFChars(env, scene_name, name);
	for (i = 0; i < event_count; i++) {
		free(events_array[i]);
	}
	
	if(ret!=RS_OK){
		scene_handle = -1;
	}
	
	free(events_array);
	(*env)->DeleteLocalRef(env, class_scene_event);
    return scene_handle;
}

//cl_scene_timer_add(cl_handle_t scene_handle, cl_scene_timer_t *timer);
//cl_scene_timer_modify(cl_handle_t scene_handle, cl_scene_timer_t *timer);
//cl_scene_timer_del(cl_handle_t scene_handle, u_int8_t tid);

JNIEXPORT jint JNICALL
NAME(ClSceneTimerDel)(JNIEnv* env, jobject this, jint scene_handle, jint id)
{
	return cl_scene_timer_del(scene_handle, id);
}


JNIEXPORT jint JNICALL
NAME(ClSceneTimerAdd)(JNIEnv* env, jobject this, jobject obj_timer, jint scene_handle)
{
	jint ret = 0;
	cl_scene_timer_t scene_timer;
	jclass clazz;
	jfieldID fid;
	jobject timer_name;
	memset(&scene_timer, 0, sizeof(cl_scene_timer_t));
	
	clazz = (*env)->FindClass(env, CLASS_SCENE_TIMER);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	scene_timer.id = (*env)->GetIntField(env, obj_timer, fid);
	
	//fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	//scene_timer.hour = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	scene_timer.hour= (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	scene_timer.minute = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	scene_timer.week = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	scene_timer.enable = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	timer_name = (*env)->GetObjectField(env, obj_timer, fid);
	scene_timer.name = (char *)((*env)->GetStringUTFChars(env, timer_name, NULL));


	ret = cl_scene_timer_add(scene_handle, &scene_timer);
			

	(*env)->ReleaseStringUTFChars(env, timer_name, scene_timer.name);
	(*env)->DeleteLocalRef(env, clazz);
	return ret;
}

//cl_scene_timer_modify(cl_handle_t scene_handle, cl_scene_timer_t *timer);
JNIEXPORT jint JNICALL
NAME(ClSceneTimerModify)(JNIEnv* env, jobject this, jobject obj_timer, jint scene_handle)
{
	jint ret = 0;
	cl_scene_timer_t scene_timer;
	jclass clazz;
	jfieldID fid;
	jobject timer_name;
	memset(&scene_timer, 0, sizeof(cl_scene_timer_t));
	
	clazz = (*env)->FindClass(env, CLASS_SCENE_TIMER);
	
	fid = (*env)->GetFieldID(env, clazz, "id", "I");
	scene_timer.id = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "hour", "I");
	scene_timer.hour= (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "minute", "I");
	scene_timer.minute = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "week", "I");
	scene_timer.week = (*env)->GetIntField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "enable", "Z");
	scene_timer.enable = (*env)->GetBooleanField(env, obj_timer, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	timer_name = (*env)->GetObjectField(env, obj_timer, fid);
	scene_timer.name = (char *)((*env)->GetStringUTFChars(env, timer_name, NULL));


	ret = cl_scene_timer_modify(scene_handle, &scene_timer);
			
	(*env)->ReleaseStringUTFChars(env, timer_name, scene_timer.name);
	
	return ret;
}
