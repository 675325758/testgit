#include "clib_jni.h"

jobject get_msg(JNIEnv* env, jclass class_notify,notify_msg_t* msg_p)
{
	jobject obj_notify = NULL;

	obj_notify = (*env)->AllocObject(env, class_notify);

	if(!obj_notify){
		return obj_notify;
	}

	jniCopyLongValue(env,class_notify,"dev_sn",obj_notify,msg_p->dev_sn);
	jniCopyIntValue(env,class_notify,"msg_time",obj_notify,msg_p->msg_time);
	jniCopyLongValue(env,class_notify,"report_id",obj_notify,msg_p->report_id);
	jniCopyIntValue(env,class_notify,"msg_type",obj_notify,msg_p->msg_type);
	jniCopyIntValue(env,class_notify,"msg_format",obj_notify,msg_p->msg_format);
	jniCopyIntValue(env,class_notify,"msg_len",obj_notify,msg_p->msg_len);
	jniCopyString(env,class_notify,"msg",obj_notify,msg_p->msg);
	jniCopyString(env,class_notify,"content",obj_notify,msg_p->content);

	return obj_notify;
}

JNIEXPORT jobject JNICALL
NAME(ClGetCmtMsgArray)(JNIEnv* env, jobject this, jint dev_handle)
{
	jclass arrEv;
	jobjectArray jArr;
	notify_msg_t** list_p;
	notify_msg_list_t* msg_list_p;
	int len, i;
	
	arrEv = (*env)->FindClass(env, CLASS_NOTIFY_MSG);

	if(NULL == arrEv){
		return arrEv;
	}
	
	msg_list_p = cl_get_notify_list(dev_handle);

	if(NULL == msg_list_p){
		(*env)->DeleteLocalRef(env, arrEv);
		return msg_list_p;
	}
	
	len = msg_list_p->count;
	
	if(len <= 0){
		cl_free_notify_list(msg_list_p);
		(*env)->DeleteLocalRef(env, arrEv);
		return NULL;
	}

	jArr = (*env)->NewObjectArray(env, len, arrEv, NULL);
	list_p = msg_list_p->list;

	for(i = 0; i < len; i++){
		jobject msg_list_item = get_msg(env, arrEv, list_p[i]);
		
		if(NULL == msg_list_item){
			break;
		}

		(*env)->SetObjectArrayElement(env, jArr, i, msg_list_item);
		(*env)->DeleteLocalRef(env, msg_list_item);
	}

	(*env)->DeleteLocalRef(env, arrEv);
	cl_free_notify_list(msg_list_p);

	return jArr;
}
JNIEXPORT jobject JNICALL
NAME(ClGetCmtsnAndReportidArray)(JNIEnv* env, jobject this, jint dev_handle)
{
	jobjectArray jArr;
	jclass arrEv;
	int cnt, ret, i;
	int size = 2;	
	cmt_notify_info_t cmt[MAX_CMT];
	
	arrEv = (*env)->FindClass(env, "[J");
	
	if(NULL == arrEv){
		return arrEv;
	}
	
	ret = cl_get_community(dev_handle, cmt, &cnt);
	
	if(ret != RS_OK){
		(*env)->DeleteLocalRef(env, arrEv);
		return NULL;
	}
	if(cnt <= 0){
		(*env)->DeleteLocalRef(env, arrEv);
		return NULL;
	}	
	
	jArr = (*env)->NewObjectArray(env, cnt, arrEv, NULL);
	
	for(i = 0; i < cnt; i++){
		jlong temp[size];
		jlongArray larr = (*env)->NewLongArray(env, size);
		
		if(NULL == larr){
			(*env)->DeleteLocalRef(env, arrEv);
			return NULL;
		}
		
		temp[0] = cmt[i].cmt_sn;
		temp[1] = cmt[i].max_report_id;
		
		(*env)->SetLongArrayRegion(env, larr, 0, size, temp);
        (*env)->SetObjectArrayElement(env, jArr, i, larr);
        (*env)->DeleteLocalRef(env, larr);
	}
		
	(*env)->DeleteLocalRef(env, arrEv);
	return jArr;
}
JNIEXPORT jint JNICALL
NAME(ClCmtMsgQuery)(JNIEnv* env, jobject this, jint dev_handle, jobjectArray cnRequestItems)
{
	jclass itemClass;
	jfieldID id_member;
	int len;
	int i, ret;
	
	itemClass = (*env)->FindClass(env, CLASS_CNREQUEST_ITEM);
	
	if(NULL == itemClass){
		return RS_ERROR;
	}

	if(NULL == cnRequestItems){
		(*env)->DeleteLocalRef(env, itemClass);
		return RS_ERROR;
	}

	len = (*env)->GetArrayLength(env, cnRequestItems);	

	if(len <= 0 || len > MAX_CMT){
		(*env)->DeleteLocalRef(env, itemClass);
		return RS_ERROR;
	}

	cmt_notify_query_t cmt_query[len];

	for(i = 0; i < len; i++){
		jobject tempItem = (*env)->GetObjectArrayElement(env, cnRequestItems, i);

		id_member = (*env)->GetFieldID(env, itemClass, "cmt_sn", "J");  
		cmt_query[i].cmt_sn = (*env)->GetLongField(env, tempItem, id_member); 

		id_member = (*env)->GetFieldID(env, itemClass, "report_id_begin", "J");  
		cmt_query[i].report_id_begin = (*env)->GetLongField(env, tempItem, id_member); 
		
		id_member = (*env)->GetFieldID(env, itemClass, "report_id_end", "J");  
		cmt_query[i].report_id_end = (*env)->GetLongField(env, tempItem, id_member); 
		
		id_member = (*env)->GetFieldID(env, itemClass, "is_descending", "I");  
		cmt_query[i].is_descending = (*env)->GetIntField(env, tempItem, id_member); 
		
		id_member = (*env)->GetFieldID(env, itemClass, "query_cnt", "I");  
		cmt_query[i].query_cnt = (*env)->GetIntField(env, tempItem, id_member); 	

		SAFE_DEL_LOCAL_REF(tempItem);
	}

	ret = cl_query_cmt_notify(dev_handle, cmt_query, len);
	
	(*env)->DeleteLocalRef(env, itemClass);
	return ret;
}