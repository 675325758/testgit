#include "clib_jni.h"

/************************************************************************************

		equipment interface 
		������ؽӿ�
 ************************************************************************************/

// ��ӵ���
JNIEXPORT jint JNICALL
NAME(ClEqAdd)(JNIEnv* env, jobject this, jint slave_handle, jobject obj_eai)
{
	jstring obj_str;
	const char *str;
	cl_equipment_add_info_t eai;
	jclass clazz;
	jfieldID fid;
	cl_handle_t eq_handle;

	memset(&eai, 0, sizeof(eai));
	clazz = (*env)->FindClass(env, CLASS_EQUIPMENT_ADD_INFO);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	obj_str = (*env)->GetObjectField(env, obj_eai, fid);
	str = ((*env)->GetStringUTFChars(env, obj_str, NULL));
	strncpy(eai.name, str, sizeof(eai.name) - 1);
	(*env)->ReleaseStringUTFChars(env, obj_str, str);

	fid = (*env)->GetFieldID(env, clazz, "eq_001e_handle", "I");
	eai.eq_001e_handle = (*env)->GetIntField(env, obj_eai, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "dev_type", "I");
	eai.dev_type = (*env)->GetIntField(env, obj_eai, fid);

	fid = (*env)->GetFieldID(env, clazz, "area_handle", "I");
	eai.area_handle = (*env)->GetIntField(env, obj_eai, fid);

	fid = (*env)->GetFieldID(env, clazz, "group_num", "I");
	eai.group_num = (*env)->GetIntField(env, obj_eai, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "is_more_ctrl", "Z");
	eai.is_more_ctrl = (*env)->GetBooleanField(env, obj_eai, fid);

	(*env)->DeleteLocalRef(env, clazz);
	return cl_eq_add(slave_handle, &eq_handle, &eai);
}

JNIEXPORT jint JNICALL
NAME(ClUDISpecialEqAdd)(JNIEnv* env, jobject this, jint slave_handle, jobject obj_eai)
{
	jstring obj_str;
	const char *str;
	cl_equipment_add_info_t eai;
	jclass clazz;
	jfieldID fid;
	cl_handle_t eq_handle;
	RS ret = RS_ERROR;

	memset(&eai, 0, sizeof(eai));
	clazz = (*env)->FindClass(env, CLASS_EQUIPMENT_ADD_INFO);

	fid = (*env)->GetFieldID(env, clazz, "name", "Ljava/lang/String;");
	obj_str = (*env)->GetObjectField(env, obj_eai, fid);
	str = ((*env)->GetStringUTFChars(env, obj_str, NULL));
	strncpy(eai.name, str, sizeof(eai.name) - 1);
	(*env)->ReleaseStringUTFChars(env, obj_str, str);

	fid = (*env)->GetFieldID(env, clazz, "eq_001e_handle", "I");
	eai.eq_001e_handle = (*env)->GetIntField(env, obj_eai, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "dev_type", "I");
	eai.dev_type = (*env)->GetIntField(env, obj_eai, fid);

	fid = (*env)->GetFieldID(env, clazz, "area_handle", "I");
	eai.area_handle = (*env)->GetIntField(env, obj_eai, fid);

	fid = (*env)->GetFieldID(env, clazz, "group_num", "I");
	eai.group_num = (*env)->GetIntField(env, obj_eai, fid);
	
	fid = (*env)->GetFieldID(env, clazz, "is_more_ctrl", "Z");
	eai.is_more_ctrl = (*env)->GetBooleanField(env, obj_eai, fid);

	(*env)->DeleteLocalRef(env, clazz);
	ret = cl_eq_add(slave_handle, &eq_handle, &eai);
	if(ret != RS_OK){
		return ret;
	}else {
		return eq_handle;
	}
	
}

// ɾ�����
JNIEXPORT jint JNICALL
NAME(ClEqDel)(JNIEnv* env, jobject this, jint eq_handle)
{
	return cl_eq_del(eq_handle);
}

// �޸ĵ������
JNIEXPORT jint JNICALL
NAME(ClEqModifyName)(JNIEnv* env, jobject this, jint eq_handle, jstring obj_name)
{
	const char *str_name;
	RS ret = RS_ERROR;
	
	str_name = (*env)->GetStringUTFChars(env, obj_name, NULL);
	ret = cl_eq_modify_name(eq_handle, str_name);
	
	(*env)->ReleaseStringUTFChars(env, obj_name, str_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClEqSetAlarmMsg)(JNIEnv* env, jobject this, jint eq_handle, jstring obj_msg)
{
	const char *str_msg;
	RS ret = RS_ERROR;
	
	str_msg = (*env)->GetStringUTFChars(env, obj_msg, NULL);
	ret = cl_eq_set_alarm_msg(eq_handle, str_msg);
	
	(*env)->ReleaseStringUTFChars(env, obj_msg, str_msg);
	
	return ret;

}

// �����������
// ��Ӱ���
JNIEXPORT jint JNICALL
NAME(ClKeyAdd)(JNIEnv* env, jobject this, jint eq_handle, jint key_id, jstring key_name)
{
	const char *str;
	RS ret = RS_ERROR;
	
	str = (*env)->GetStringUTFChars(env, key_name, NULL);
	ret = cl_key_add(eq_handle, key_id, (char *)str);
	
	(*env)->ReleaseStringUTFChars(env, key_name, str);

	return ret;
}

// ɾ���
JNIEXPORT jint JNICALL
NAME(ClKeyDel)(JNIEnv* env, jobject this, jint eq_handle, jint key_id)
{
	return cl_key_del(eq_handle, key_id);
}

// �޸İ�������
JNIEXPORT jint JNICALL
NAME(ClKeyModifyName)(JNIEnv* env, jobject this, jint eq_handle, jint key_id, jstring new_name)
{
	const char *str;
	RS ret = RS_ERROR;
	
	str = (*env)->GetStringUTFChars(env, new_name, NULL);
	ret = cl_key_modify_name(eq_handle, key_id, str);
	
	(*env)->ReleaseStringUTFChars(env, new_name, str);

	return ret;
}

// ���Ͱ����������
JNIEXPORT jint JNICALL
NAME(ClKeySendCtrlSignal)(JNIEnv* env, jobject this, jint eq_handle, jint key_id)
{
	return cl_key_send_ctrl_singal(eq_handle, key_id);
}

// ��������ѧϰ
JNIEXPORT jint JNICALL
NAME(ClKeyLearnSetCallback)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_set_key_learn_callback(user_handle, nactivCallback, NULL);
}

JNIEXPORT jint JNICALL
NAME(ClKeyLearnStart)(JNIEnv* env, jobject this, jint user_handle, jint eq_handle, jint key_id, jint learn_mode)
{
	return cl_key_learn_start(user_handle, eq_handle, key_id, learn_mode);
}

JNIEXPORT jint JNICALL
NAME(ClKeyLearnTryCtrl)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_kl_try_ctrl(user_handle);
}

JNIEXPORT jint JNICALL
NAME(ClKeyLearnGenCode)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_kl_gen_code(user_handle);
}

JNIEXPORT jint JNICALL
NAME(ClKeyLearnStop)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_key_learn_stop(user_handle);
}

JNIEXPORT jint JNICALL
NAME(ClKeyLearnAjustCode)(JNIEnv* env, jobject this, jint user_handle, jint ajust_value)
{
	return cl_kl_ajust_code(user_handle, ajust_value);
}

JNIEXPORT jint JNICALL
NAME(ClKeyLearnSetPlusWidth)(JNIEnv* env, jobject this, jint user_handle, jboolean is_narrow)
{
	return cl_kl_set_plus_width(user_handle, is_narrow);
}

/*JNIEXPORT jint JNICALL
NAME(ClKeyLearnQueryPlusWidth)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_kl_query_plus_width(user_handle);
}*/

JNIEXPORT jint JNICALL
NAME(ClKeyLearnSaveCode)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_kl_save_learn_code(user_handle);
}

JNIEXPORT jobject JNICALL
NAME(ClKeyLearnGetStat)(JNIEnv* env, jobject this, jint user_handle)
{
	cl_key_learn_t* stat;
	jclass class_kls;
	jobject obj_kls = NULL;
	jfieldID fid;
	
	stat = cl_key_learn_get_stat(user_handle);
	if(stat != NULL)
	{
		class_kls = (*env)->FindClass(env, CLASS_KEY_LEARN_STAT);
		obj_kls = (*env)->AllocObject(env, class_kls);
		
		fid = (*env)->GetFieldID(env, class_kls, "eq_handle", "I");
		(*env)->SetIntField(env, obj_kls, fid, stat->eq_handle);

		fid = (*env)->GetFieldID(env, class_kls, "key_id", "I");
		(*env)->SetIntField(env, obj_kls, fid, stat->key_id);

		fid = (*env)->GetFieldID(env, class_kls, "remain_time", "I");
		(*env)->SetIntField(env, obj_kls, fid, stat->remain_time);


		fid = (*env)->GetFieldID(env, class_kls, "last_error", "I");
		(*env)->SetIntField(env, obj_kls, fid, stat->last_error);

		fid = (*env)->GetFieldID(env, class_kls, "max_ajust_value", "I");
		(*env)->SetIntField(env, obj_kls, fid, stat->max_ajust_value);

		fid = (*env)->GetFieldID(env, class_kls, "min_ajust_value", "I");
		(*env)->SetIntField(env, obj_kls, fid, stat->min_ajust_value);

		fid = (*env)->GetFieldID(env, class_kls, "is_support_ajust_code", "Z");
		(*env)->SetBooleanField(env, obj_kls, fid, stat->is_support_ajust_code);

		fid = (*env)->GetFieldID(env, class_kls, "is_narrow_plus_width", "Z");
		(*env)->SetBooleanField(env, obj_kls, fid, stat->is_narrow_plus_width);
		
		cl_kl_stat_free(stat);
		(*env)->DeleteLocalRef(env, class_kls);
	}

	return obj_kls;
}

static void CopyAlarmMsgArray(JNIEnv* env, jclass class_msg_list, jobject obj_msg_list, alarm_msg_list_t* msg_list)
{
	int i;
	jfieldID fid;
	jclass class_obj;
	jobjectArray obj_array;
	jobject obj;
	jstring str;
	u_int64_t dev_sn;
    
	class_obj = (*env)->FindClass(env, CLASS_ALARM_MSG);
	dev_sn = msg_list->dev_sn;
	if (msg_list->count > 0)
	{
		obj_array = (*env)->NewObjectArray(env, msg_list->count, class_obj, NULL);
	}
	else
	{
		obj_array = NULL;
	}
	for (i = 0; i < msg_list->count; i++) {
		obj = (*env)->AllocObject(env, class_obj);

		fid = (*env)->GetFieldID(env, class_obj, "dev_sn", "J");
		(*env)->SetLongField(env, obj, fid, dev_sn);
		
		fid = (*env)->GetFieldID(env, class_obj, "alarm_time", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->msg[i].alarm_time);
		
		fid = (*env)->GetFieldID(env, class_obj, "alarm_duration", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->msg[i].alarm_duration);
		
		fid = (*env)->GetFieldID(env, class_obj, "alarm_type", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->msg[i].alarm_type);
		
		fid = (*env)->GetFieldID(env, class_obj, "alarm_id", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->msg[i].alarm_id);
		
		fid = (*env)->GetFieldID(env, class_obj, "alarm_uuid", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->first_report_id + i);
		
		fid = (*env)->GetFieldID(env, class_obj, "len_name", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->msg[i].len_name);
		
		jniCopyString(env, class_obj, "alarm_name", obj, msg_list->msg[i].arlam_name);
		
		fid = (*env)->GetFieldID(env, class_obj, "len_msg", "I");
		(*env)->SetIntField(env, obj, fid, msg_list->msg[i].len_msg);
		
		jniCopyString(env, class_obj, "alarm_msg", obj, msg_list->msg[i].alarm_msg);
		
		(*env)->SetObjectArrayElement(env, obj_array, i, obj);
	}
    
	// ���鹹����ϣ���ֵ
	fid = (*env)->GetFieldID(env, class_msg_list, "msg", "[L" CLASS_ALARM_MSG ";");
	(*env)->SetObjectField(env, obj_msg_list, fid, obj_array);
	//(*env)->DeleteLocalRef(env, obj_array);
	(*env)->DeleteLocalRef(env, class_obj);
}


JNIEXPORT jobject JNICALL
NAME(ClGetAlarmMsg)(JNIEnv* env, jobject this, jint user_handle)
{
    alarm_msg_list_t* msg_list;
    jclass class_aml;
	jobject obj_aml = NULL;
	jfieldID fid;
    jfieldID fid_msg;
    
    msg_list = cl_get_alarm(user_handle);
    if(msg_list != NULL)
	{
        class_aml = (*env)->FindClass(env, CLASS_ALARM_MSG_LIST);
        obj_aml = (*env)->AllocObject(env, class_aml);

        fid = (*env)->GetFieldID(env, class_aml, "dev_sn", "J");
        (*env)->SetLongField(env, obj_aml, fid, msg_list->dev_sn);

        fid = (*env)->GetFieldID(env, class_aml, "first_report_id", "J");
        (*env)->SetLongField(env, obj_aml, fid, msg_list->first_report_id);

        fid = (*env)->GetFieldID(env, class_aml, "count", "I");
        (*env)->SetIntField(env, obj_aml, fid, msg_list->count);

        CopyAlarmMsgArray(env, class_aml, obj_aml, msg_list);        

        cl_free_alarm(msg_list);

        (*env)->DeleteLocalRef(env, class_aml);
    }
	
	return obj_aml;
}

JNIEXPORT jint JNICALL
NAME(ClUserAddAlarmPhone)(JNIEnv* env, jobject this, jint user_handle, jstring phone)
{
    const char *str;
    RS ret = RS_ERROR;
	
	str = (*env)->GetStringUTFChars(env, phone, NULL);
	ret = cl_user_add_alarm_phone(user_handle, str);
	
	(*env)->ReleaseStringUTFChars(env, phone, str);
    
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUserDelAlarmPhone)(JNIEnv* env, jobject this, jint user_handle, jstring phone)
{
    const char *str;
    RS ret = RS_ERROR;
	
	str = (*env)->GetStringUTFChars(env, phone, NULL);
	ret = cl_user_del_alarm_phone(user_handle, str);
	
	(*env)->ReleaseStringUTFChars(env, phone, str);
    
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClEqSetAlarmAssist001e)(JNIEnv* env, jobject this, jint eq_handle, jint slave_handle)
{
    return cl_eq_set_alarm_assist_001e(eq_handle, slave_handle);
}

JNIEXPORT jint JNICALL
NAME(ClEqSetAlarmPhones)(JNIEnv* env, jobject this, jint eq_handle, jobjectArray phonelist, jint phone_count)
{
	int ret;
    const char *str;
    jstring phone;
    int i;
    char **phones;

	if (phone_count == 0) {
		ret = cl_eq_set_alarm_phones(eq_handle, NULL, phone_count);
	} else {
		phones = (char **)calloc(sizeof(char *), phone_count);
		
	    for(i = 0; i < phone_count; i++)
	    {
	        jstring phone = (jstring)(*env)->GetObjectArrayElement(env, phonelist, i);
	        str = (*env)->GetStringUTFChars(env, phone, NULL);
			phones[i] = (char *)str;
//			(*env)->ReleaseStringUTFChars(env, phone, str);
	    }
		ret = cl_eq_set_alarm_phones(eq_handle, phones, phone_count);

		free(phones);
	}
	
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUserSetNotifyExpect)(JNIEnv* env, jobject this, jint user_handle, jlong dev_sn, jlong expect_report_id)
{
    notify_expect_t expect;
    expect.dev_sn = dev_sn;
    expect.expect_report_id = expect_report_id;
    return cl_set_notify_expect(user_handle, &expect, nactivCallback, NULL);
}

JNIEXPORT jint JNICALL
NAME(ClEqEnableAlarm)(JNIEnv* env, jobject this, jint eq_handle, jboolean enable)
{
    return cl_eq_enable_alarm(eq_handle, enable);
}


JNIEXPORT jint JNICALL
NAME(ClEqLinkageSceneSet)(JNIEnv* env, jobject this, jint eq_handle, jint scene_handle )
{
    return cl_eq_linkage_scene_set(eq_handle, scene_handle);
}


JNIEXPORT jint JNICALL
NAME(ClEqLinkageSceneDel)(JNIEnv* env, jobject this, jint eq_handle, jint scene_handle )
{
    return cl_eq_linkage_scene_del(eq_handle, scene_handle);
}


JNIEXPORT jint JNICALL
NAME(ClEqEnableAlarmPush)(JNIEnv* env, jobject this, jint eq_handle, jboolean enable )
{
    return cl_eq_enable_alarm_push(eq_handle, enable);
}


JNIEXPORT jint JNICALL
NAME(ClEqEnableAlarmSms)(JNIEnv* env, jobject this, jint eq_handle, jboolean enable )
{
    return cl_eq_enable_alarm_sms(eq_handle, enable);
}

JNIEXPORT jint JNICALL
NAME(ClEqBindSoundLight)(JNIEnv* env, jobject this, jint eq_handle, jboolean ison,jintArray soundArray)
{
	cl_handle_t *handles_array;
	int ret,handles_count;

	handles_array = (*env)->GetIntArrayElements(env, soundArray,NULL);
	handles_count = (*env)->GetArrayLength(env, soundArray);

	ret = cl_eq_bind_soundlight(eq_handle,ison,handles_array,handles_count&0xff);

	(*env)->ReleaseIntArrayElements(env, soundArray,handles_array,0);

    return ret;
}

//ˢ�µ���״̬
JNIEXPORT jint JNICALL
NAME(ClRefreshEquipmentStat)(JNIEnv* env, jobject this, jint eq_handle)
{
    return cl_eq_refresh(eq_handle);
}

//ɨ��˫�����
JNIEXPORT jint JNICALL
NAME(ClUserScanDbRfEquipment)(JNIEnv* env, jobject this, jint user_handle)
{
    return cl_user_scan_db_rf(user_handle);
}

//�����м�״̬
JNIEXPORT jint JNICALL
NAME(ClDbEquipmentSetRepeat)(JNIEnv* env, jobject this, jint eq_handle,jboolean ison)
{
    return cl_eq_rf_repeater_set(eq_handle,!!ison);
}

JNIEXPORT jint JNICALL
NAME(ClSetDbDimmingValue)(JNIEnv* env, jobject this, jint eq_handle, jint value)
{
    return cl_set_db_dimming_value(eq_handle, value);
}