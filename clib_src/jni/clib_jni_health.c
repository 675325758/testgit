
#include "clib_jni.h"

/************************************************************************************

		健康相关接口
		
 ************************************************************************************/

/*
	功能：
		查询健康家庭成员列表
	输入参数:
		@handle: 设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	回调事件: HE_FM_LIST_OK , 查询成功, 调用cl_family_list_get获取列表
			  	   HE_FM_LIST_FAIL , 查询失败
	
*/

JNIEXPORT jint JNICALL
NAME(ClFamilyListQuery)(JNIEnv* env, jobject this, jint dev_handle )
{
	return cl_family_list_query(dev_handle);
}


JNIEXPORT jobjectArray JNICALL
NAME(ClFamilyListGet)(JNIEnv* env, jobject this, jint dev_handle )
{
	jobjectArray member_array = NULL;
	family_list_t *family_list;
	jclass member_class = NULL;
	jobject member = NULL;
	int i;
	
	family_list = cl_family_list_get(dev_handle);
	if( !family_list )
		goto end;
	
	if( family_list->count >0 ){
		member_class = (*env)->FindClass(env, CLASS_HEALTH_MENBER);
		if( !member_class )
			goto end;
		
		member_array = (*env)->NewObjectArray(env, family_list->count, member_class, NULL);
		if(!member_array)
			goto end;
		
		for (i = 0; i < family_list->count; i++) {
			member = (*env)->AllocObject(env, member_class);
			if(!member)
				goto end;

			jniCopyIntValue(env,member_class,"bd_year",member,family_list->list[i].bd_year);
			jniCopyIntValue(env,member_class,"weight",member,family_list->list[i].weight);
			jniCopyIntValue(env,member_class,"bd_month",member,family_list->list[i].bd_month);
			jniCopyIntValue(env,member_class,"height",member,family_list->list[i].height);
			jniCopyIntValue(env,member_class,"sex",member,family_list->list[i].sex);
			jniCopyIntValue(env,member_class,"step",member,family_list->list[i].step);
			jniCopyIntValue(env,member_class,"id",member,family_list->list[i].id);
			jniCopyIntValue(env,member_class,"action",member,family_list->list[i].action);
			jniCopyIntValue(env,member_class,"is_current",member,family_list->list[i].is_current);
			jniCopyIntValue(env,member_class,"career",member,family_list->list[i].career);
			jniCopyString(env, member_class, "name", member, family_list->list[i].name);
			
			(*env)->SetObjectArrayElement(env, member_array, i, member);
		}
		
	}
	
end:
	if(family_list!=NULL)
		cl_family_list_free(family_list);
	
	if(member_class!=NULL)
		(*env)->DeleteLocalRef(env, member_class);
	
	return member_array;
}



/*
	功能：
		配置健康家庭成员, 一次只能配置一个
	输入参数:
		@handle: 设备句柄
		@fm: 家庭成员信息
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	回调事件: HE_FM_CONFIG_OK HE_FM_CONFIG_FAIL
*/

JNIEXPORT jint JNICALL
NAME( ClMemberConfig )(JNIEnv* env, jobject this, jint dev_handle, jclass member )
{
	jint ret = -1;
	family_t fm;
	jclass class_menber;
	jfieldID fid;
	jobject obj_name;
	jstring str;
	char* p;
	

	memset(&fm, 0, sizeof(family_t));
	class_menber = (*env)->FindClass(env, CLASS_HEALTH_MENBER);
	if( !class_menber )
		return ret;
	
	
	fid = (*env)->GetFieldID(env, class_menber, "bd_year", "I");
	fm.bd_year = (*env)->GetIntField(env, member, fid);
	
	fid = (*env)->GetFieldID(env, class_menber, "weight", "I");
	fm.weight= (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "bd_month", "I");
	fm.bd_month = (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "height", "I");
	fm.height = (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "sex", "I");
	fm.sex = (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "career", "I");
	fm.career = (*env)->GetIntField(env, member, fid);
	
	fid = (*env)->GetFieldID(env, class_menber, "step", "I");
	fm.step = (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "id", "I");
	fm.id = (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "action", "I");
	fm.action = (*env)->GetIntField(env, member, fid);

	fid = (*env)->GetFieldID(env, class_menber, "is_current", "I");
	fm.is_current = (*env)->GetIntField(env, member, fid);


	fid = (*env)->GetFieldID(env, class_menber, "name", "Ljava/lang/String;");
	obj_name = (*env)->GetObjectField(env, member, fid);
	p = (char *)((*env)->GetStringUTFChars(env, obj_name, NULL));
	if(p){
		strcpy(fm.name,p);
	}
	
	ret = cl_family_config(dev_handle, &fm);
	

	if(p)
		(*env)->ReleaseStringUTFChars(env, obj_name,p);
	(*env)->DeleteLocalRef(env, class_menber);

	return ret;
}

JNIEXPORT jint JNICALL
NAME( ClMeasureQuery )(JNIEnv* env, jobject this, jint dev_handle, jclass query )
{
	jint ret = -1;
	measure_query_t mq;
	jclass class_query;
	jfieldID fid;

	memset(&mq, 0, sizeof(measure_query_t));
	class_query = (*env)->FindClass(env, CLASS_MEASURE_QUREY);
	
	if( !class_query )
		return ret;
	
	
	fid = (*env)->GetFieldID(env, class_query, "begin_time", "I");
	mq.begin_time = (*env)->GetIntField(env, query, fid);
	
	fid = (*env)->GetFieldID(env, class_query, "end_time", "I");
	mq.end_time = (*env)->GetIntField(env, query, fid);

	fid = (*env)->GetFieldID(env, class_query, "begin_time", "I");
	mq.begin_time = (*env)->GetIntField(env, query, fid);

	fid = (*env)->GetFieldID(env, class_query, "fm_id", "I");
	mq.fm_id = (*env)->GetIntField(env, query, fid);

	fid = (*env)->GetFieldID(env, class_query, "count", "I");
	mq.count = (*env)->GetIntField(env, query, fid);
	
	ret = cl_measure_query(dev_handle, &mq);
	
	(*env)->DeleteLocalRef(env, class_query);

	return ret;	

}

JNIEXPORT jobjectArray JNICALL
NAME(ClMeasureListGet)(JNIEnv* env, jobject this, jint dev_handle )
{
	jobjectArray measure_array = NULL;
	measure_list_t *measure_list;
	jclass measure_class = NULL;
	jobject measure_obj = NULL;
	int i,j,k;
	int step[MAX_PEDOMETER];
	int calorie[MAX_PEDOMETER];
	int oxygen_step[MAX_PEDOMETER];
	int oxygen_calorie[MAX_PEDOMETER];
	measure_list = cl_measure_list_get(dev_handle);
	if( !measure_list )
		goto end;
	if( measure_list->count >0 ){
		measure_class = (*env)->FindClass(env, CLASS_MEASURE_DATA);
		if( !measure_class )
			goto end;
		
		measure_array = (*env)->NewObjectArray(env, measure_list->count, measure_class, NULL);
		
		if(!measure_array)
			goto end;	
		for (i = 0; i < measure_list->count; i++){
			measure_obj = (*env)->AllocObject(env, measure_class);
			if(!measure_obj)
				goto end;
			int type = measure_list->list[i].hdt;
			jniCopyIntValue(env,measure_class,"mtime",measure_obj,measure_list->list[i].mtime);
			jniCopyIntValue(env,measure_class,"fm_id",measure_obj,measure_list->list[i].fm_id);
			jniCopyIntValue(env,measure_class,"hdt",measure_obj,type);
			jniCopyIntValue(env,measure_class,"measure_cnt",measure_obj,measure_list->list[i].measure_cnt);
	
			if( type == HDT_WEIGTH ){
				jniCopyIntValue(env,measure_class,"weight",measure_obj,measure_list->list[i].measure_data.weight.weight*10);
			}else if( type == HDT_FAT ){
				jniCopyIntValue(env,measure_class,"fat",measure_obj,measure_list->list[i].measure_data.fat.fat*10);
				jniCopyIntValue(env,measure_class,"water",measure_obj,measure_list->list[i].measure_data.fat.water*10);
				jniCopyIntValue(env,measure_class,"muscle",measure_obj,measure_list->list[i].measure_data.fat.muscle*10);
				jniCopyIntValue(env,measure_class,"visual_fat",measure_obj,measure_list->list[i].measure_data.fat.visual_fat*10);
			}else if( type == HDT_BLOOD_PRESSURE ){
				jniCopyIntValue(env,measure_class,"plus",measure_obj,measure_list->list[i].measure_data.blood_pressure.plus*10);
				jniCopyIntValue(env,measure_class,"pressure_high",measure_obj,measure_list->list[i].measure_data.blood_pressure.pressure_high*10);
				jniCopyIntValue(env,measure_class,"pressure_low",measure_obj,measure_list->list[i].measure_data.blood_pressure.pressure_low*10);
			}else if( type == HDT_BLOOD_SUGAR ){
				jniCopyIntValue(env,measure_class,"blood_sugar",measure_obj,measure_list->list[i].measure_data.blood_sugar.sugar*10);
			}else if( type == HDT_BLOOD_OXYGN ){
				jniCopyIntValue(env,measure_class,"blood_oxygen",measure_obj,measure_list->list[i].measure_data.blood_oxygen.oxygen*10);
			}else if( type == HDT_PEDOMETER ){
				
				measure_pedometer_t *mt = &(measure_list->list[i].measure_data.meter);				
				j = k = 0;
				for(; j < MAX_PEDOMETER; j++){
					if(mt->is_valid[j] == 0){
						continue;
					}
					step[k] =  mt->pedometer[j].step *10; 
					calorie[k] = mt->pedometer[j].calorie*10;
					oxygen_step[k] = mt->pedometer[j].oxygen_step*10; 
					oxygen_calorie[k] = mt->pedometer[j].oxygen_calorie*10;
					k++;
				}
				jniCopyIntArray(env,measure_class ,"step", measure_obj, step,k);
				jniCopyIntArray(env,measure_class ,"calorie", measure_obj, calorie,k);
				jniCopyIntArray(env,measure_class ,"oxygen_step", measure_obj, oxygen_step,k);
				jniCopyIntArray(env,measure_class ,"oxygen_calorie", measure_obj,oxygen_calorie,k);
	
			}else if( type == HDT_EAR_TEMP ){
				jniCopyIntValue(env,measure_class,"temperature",measure_obj,measure_list->list[i].measure_data.ear_temp.temperature*10);
			}
			(*env)->SetObjectArrayElement(env, measure_array, i, measure_obj);
		}
		
	}
	
end:
	if(measure_list!=NULL)
		cl_measure_list_free(measure_list);
	
	if(measure_class!=NULL)
		(*env)->DeleteLocalRef(env, measure_class);
	
	return measure_array;
}



JNIEXPORT jint JNICALL
NAME( ClMeasureDelete )(JNIEnv* env, jobject this, jint dev_handle, jint mtime, jint id, jint type, jint measure_cont )
{
	jint ret = -1;
	measure_del_t del = {0};

	memset(&del, 0, sizeof(measure_del_t));
	
	del.mtime = mtime;
	del.fm_id = id;
	del.hdt = type;
	del.measure_cnt = measure_cont;
	
	ret = cl_measure_delete(dev_handle, &del);

	return ret;	

}


