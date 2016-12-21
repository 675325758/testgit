
#include "clib_jni.h"

/************************************************************************************

		绑定手机相关接口
		
 ************************************************************************************/


 /*
	 功能：
		 绑定手机申请
	 输入参数:
		 @request: 绑定手机相关参数
	 输出参数:
		 无
	 返回：
		 RS_OK: 成功
		 其他: 失败
	 事件通知: UE_BIND_PHONE_REQUEST_OK 提交绑定申请成功
		 UE_BIND_PHONE_REQUEST_OK 提交绑定申请被批准
 */
 
 //CLIB_API RS cl_user_bind_phone(cl_handle_t user_handle, cl_bind_phone_t *request);
 
 JNIEXPORT jint JNICALL
 NAME(ClUserBindPhone)(JNIEnv* env, jobject this, jint user_handle, jobject obj_bind_info )
 {
	 jint ret = 0;
	 cl_bind_phone_t bind_info;
	 jclass clazz;
	 jfieldID fid;
	 jobject obj_str;
	 const char *str;
	 
	 memset(&bind_info, 0, sizeof(cl_bind_phone_t));
	 
	 clazz = (*env)->FindClass(env, CLASS_BIND_PHONE_ITEM);
	 
	 fid = (*env)->GetFieldID(env, clazz, "phone_number", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, obj_bind_info, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(bind_info.phone_number, str, sizeof(bind_info.phone_number) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }

	 fid = (*env)->GetFieldID(env, clazz, "phone_model", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, obj_bind_info, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(bind_info.phone_model, str, sizeof(bind_info.phone_model) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }
 
	 fid = (*env)->GetFieldID(env, clazz, "bind_name", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, obj_bind_info, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(bind_info.bind_name, str, sizeof(bind_info.bind_name) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }

 	 fid = (*env)->GetFieldID(env, clazz, "bind_uuid", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, obj_bind_info, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(bind_info.bind_uuid, str, sizeof(bind_info.bind_uuid) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }
	 
 	 fid = (*env)->GetFieldID(env, clazz, "bind_message", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, obj_bind_info, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(bind_info.bind_message, str, sizeof(bind_info.bind_message) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }

 	 fid = (*env)->GetFieldID(env, clazz, "timestamp", "Ljava/lang/String;");
	 obj_str = (*env)->GetObjectField(env, obj_bind_info, fid);
	 if ((str = (*env)->GetStringUTFChars(env, obj_str, NULL)) != NULL) {
	 	strncpy(bind_info.timestamp, str, sizeof(bind_info.timestamp) - 1);
		(*env)->ReleaseStringUTFChars(env, obj_str, str);
	 }
	 
	 ret = cl_user_bind_phone(user_handle, &bind_info);
	 
	 return ret;
 }


 /*
	 功能：
		 允许/禁止普通用户登录
	 输入参数:
		 allow@ 非0表示允许普通用户登录，0表示禁止普通用户登录
	 输出参数:
		 无
	 返回：
		 RS_OK: 成功
		 其他: 失败
	 事件通知: 
 */
 
 //CLIB_API RS cl_user_bind_phone_allow_normal(cl_handle_t user_handle, int allow);

JNIEXPORT jint JNICALL
NAME( ClUserBindPhoneAllowNormal )(JNIEnv* env, jobject this, jint user_handle, jint allow)
{
	return cl_user_bind_phone_allow_normal(user_handle,allow);
}
 
 /*
	 功能：
		 查询已绑定手机列表
	 输入参数:
		 无
	 输出参数:
		 无
	 返回：
		 RS_OK: 成功
		 其他: 失败
	 事件通知: UE_BIND_PHONE_LIST 收到已绑定列表
	 
 */
 
 //CLIB_API RS cl_user_bind_phone_query(cl_handle_t user_handle);

JNIEXPORT jint JNICALL
NAME(ClUserBindPhoneQuery)(JNIEnv* env, jobject this, jint user_handle)
{
	return cl_user_bind_phone_query(user_handle);
}
 
 //CLIB_API cl_bind_phone_list_t *cl_user_get_bind_phone_list(cl_handle_t user_handle);

 

JNIEXPORT jobject JNICALL
NAME(ClUserGetBindPhoneList)(JNIEnv* env, jobject this, jint user_handle)
{
	int i;
	jfieldID fid;
	jclass class_bind_phone, class_bind_phone_item;
	jobjectArray obj_bind_list_arr;
	jobject bind_phone, item;
	cl_bind_phone_list_t *bpl;
	jstring str;
	
	if ((bpl = cl_user_get_bind_phone_list(user_handle)) == NULL)
		return NULL;

	class_bind_phone = (*env)->FindClass(env, CLASS_BIND_PHONE);
	class_bind_phone_item = (*env)->FindClass(env, CLASS_BIND_PHONE_ITEM);

	bind_phone = (*env)->AllocObject(env, class_bind_phone);

	fid = (*env)->GetFieldID(env, class_bind_phone, "count", "I");
	(*env)->SetIntField(env, bind_phone, fid, bpl->count);
	
	fid = (*env)->GetFieldID(env, class_bind_phone, "allow_normal", "Z");
	(*env)->SetBooleanField(env, bind_phone, fid, bpl->allow_normal);

	if (bpl->count > 0) {
		obj_bind_list_arr = (*env)->NewObjectArray(env, bpl->count, class_bind_phone_item, NULL);
	} else {
		obj_bind_list_arr = NULL;
	}

	
	for (i = 0; i < bpl->count; i++) {
		item = (*env)->AllocObject(env, class_bind_phone_item);

		str = (*env)->NewStringUTF(env, bpl->bind_array[i].phone_number);
		fid = (*env)->GetFieldID(env, class_bind_phone_item, "phone_number", "Ljava/lang/String;");
		(*env)->SetObjectField(env, item, fid, str);

		str = (*env)->NewStringUTF(env, bpl->bind_array[i].phone_model);
		fid = (*env)->GetFieldID(env, class_bind_phone_item, "phone_model", "Ljava/lang/String;");
		(*env)->SetObjectField(env, item, fid, str);

		str = (*env)->NewStringUTF(env, bpl->bind_array[i].bind_name);
		fid = (*env)->GetFieldID(env, class_bind_phone_item, "bind_name", "Ljava/lang/String;");
		(*env)->SetObjectField(env, item, fid, str);

		str = (*env)->NewStringUTF(env, bpl->bind_array[i].bind_uuid);
		fid = (*env)->GetFieldID(env, class_bind_phone_item, "bind_uuid", "Ljava/lang/String;");
		(*env)->SetObjectField(env, item, fid, str);

		str = (*env)->NewStringUTF(env, bpl->bind_array[i].bind_message);
		fid = (*env)->GetFieldID(env, class_bind_phone_item, "bind_message", "Ljava/lang/String;");
		(*env)->SetObjectField(env, item, fid, str);

		str = (*env)->NewStringUTF(env, bpl->bind_array[i].timestamp);
		fid = (*env)->GetFieldID(env, class_bind_phone_item, "timestamp", "Ljava/lang/String;");
		(*env)->SetObjectField(env, item, fid, str);

		(*env)->SetObjectArrayElement(env, obj_bind_list_arr, i, item); 	 
	}

	fid = (*env)->GetFieldID(env, class_bind_phone, "items", "[L" CLASS_BIND_PHONE_ITEM ";");
	(*env)->SetObjectField(env, bind_phone, fid, obj_bind_list_arr);

	cl_user_free_bind_phone_list(bpl);
	
	return bind_phone;
}
 
 /*
	 功能：
		 查询绑定手机申请列表
	 输入参数:
		 无
	 输出参数:
		 无
	 返回：
		 RS_OK: 成功
		 其他: 失败
	 事件通知: UE_BIND_PHONE_REQUEST_LIST 收到绑定申请列表
 
 */
 
 //CLIB_API RS cl_user_bind_phone_request_query(cl_handle_t user_handle);

JNIEXPORT jint JNICALL
NAME( ClUserBindPhoneRequestQuery )(JNIEnv* env, jobject this, jint user_handle )
{
	return cl_user_bind_phone_request_query(user_handle);
}
 

 //CLIB_API cl_bind_phone_request_list_t *cl_user_get_bind_phone_request_list(cl_handle_t user_handle);

JNIEXPORT jobject JNICALL
NAME(ClUserGetBindPhoneRequestList)(JNIEnv* env, jobject this, jint user_handle)
 {
 	 int i;
	 jfieldID fid;
	 jclass class_bind_phone, class_bind_phone_item;
	 jobjectArray obj_bind_list_arr;
	 jobject bind_phone, item;
	 cl_bind_phone_request_list_t *bpl;
	 jstring str;
	 
	 if ((bpl = cl_user_get_bind_phone_request_list(user_handle)) == NULL)
		 return NULL;
 
	 class_bind_phone = (*env)->FindClass(env, CLASS_BIND_PHONE);
	 class_bind_phone_item = (*env)->FindClass(env, CLASS_BIND_PHONE_ITEM);
 
	 bind_phone = (*env)->AllocObject(env, class_bind_phone);
 
	 fid = (*env)->GetFieldID(env, class_bind_phone, "count", "I");
	 (*env)->SetIntField(env, bind_phone, fid, bpl->count);
	 
	
	 if (bpl->count > 0) {
		 obj_bind_list_arr = (*env)->NewObjectArray(env, bpl->count, class_bind_phone_item, NULL);
	 } else {
		 obj_bind_list_arr = NULL;
	 }
 
	 
	 for (i = 0; i < bpl->count; i++) {
		 item = (*env)->AllocObject(env, class_bind_phone_item);
 
		 jniCopyString(env,class_bind_phone_item, "phone_number", item, bpl->request_list[i].phone_number);

		 jniCopyString(env,class_bind_phone_item, "phone_model", item, bpl->request_list[i].phone_model);
 
		 jniCopyString(env,class_bind_phone_item, "bind_name", item, bpl->request_list[i].bind_name);
 
		 jniCopyString(env,class_bind_phone_item, "bind_uuid", item, bpl->request_list[i].bind_uuid);
 
		 jniCopyString(env,class_bind_phone_item, "bind_message", item, bpl->request_list[i].bind_message);

		 jniCopyString(env,class_bind_phone_item, "timestamp", item, bpl->request_list[i].timestamp);
 
		 (*env)->SetObjectArrayElement(env, obj_bind_list_arr, i, item);	  
	 }
 
	 fid = (*env)->GetFieldID(env, class_bind_phone, "items", "[L" CLASS_BIND_PHONE_ITEM ";");
	 (*env)->SetObjectField(env, bind_phone, fid, obj_bind_list_arr);
 
	 cl_user_free_bind_phone_request_list(bpl);
	 
	 (*env)->DeleteLocalRef(env, class_bind_phone_item);
	 (*env)->DeleteLocalRef(env, class_bind_phone);
	 
	 return bind_phone;
 }

 
 /*
	 功能：
		 对绑定手机申请进行批准/拒绝操作
	 输入参数:
		 action@: BIND_PHONE_ACCEPT 批准BIND_PHONE_DEN 拒绝
		 uuid@: 申请者uuid
	 输出参数:
		 无
	 返回：
		 RS_OK: 成功
		 其他: 失败
	 事件通知: UE_BIND_PHONE_OPERATION_RESULT 操作结果
 
 */
 
 //CLIB_API RS cl_user_bind_phone_operation(cl_handle_t user_handle, char action, char *uuid);

 JNIEXPORT jint JNICALL
 NAME( ClUserBindPhoneOperation )(JNIEnv* env, jobject this, jint user_handle, jint action, jstring uuid)
 {
 	 jobject obj_str;
	 RS ret;
	 const char *str_uuid;

	 str_uuid = (*env)->GetStringUTFChars(env, uuid, NULL);
	 
	 ret = cl_user_bind_phone_operation(user_handle, action&0xff, (char *)str_uuid);
	 
	 (*env)->ReleaseStringUTFChars(env, uuid, str_uuid);

	 return ret;
 }

 
 //CLIB_API cl_bind_phone_result_t *cl_user_get_bind_phone_operation_result(cl_handle_t user_handle);
JNIEXPORT jobject JNICALL
NAME(ClUserGetBindPhoneOpResult)(JNIEnv* env, jobject this, jint user_handle)
 {
 	jfieldID fid;
 	jclass  class_bind_result;
	jobject bind_phone_result;
	cl_bind_phone_result_t *bpr;
	char *str;

	if ((bpr = cl_user_get_bind_phone_operation_result(user_handle)) == NULL)
		 return NULL;

	class_bind_result = (*env)->FindClass(env, CLASS_BIND_PHONE_RESULT);
	bind_phone_result = (*env)->AllocObject(env, class_bind_result);


	fid = (*env)->GetFieldID(env, class_bind_result, "err_code", "I");
	(*env)->SetIntField(env, bind_phone_result, fid, bpr->err_code);

	fid = (*env)->GetFieldID(env, class_bind_result, "action", "I");
	(*env)->SetIntField(env, bind_phone_result, fid, bpr->action);
	
	jniCopyString(env, class_bind_result, "request_uuid", bind_phone_result, bpr->request_uuid);
	
	jniCopyString(env, class_bind_result, "phone_number", bind_phone_result, bpr->operator_info.phone_number);

	jniCopyString(env, class_bind_result, "phone_model", bind_phone_result, bpr->operator_info.phone_model);

	jniCopyString(env, class_bind_result, "bind_name", bind_phone_result, bpr->operator_info.bind_name);

	jniCopyString(env, class_bind_result, "bind_uuid", bind_phone_result, bpr->operator_info.bind_uuid);
	
	jniCopyString(env, class_bind_result, "bind_message", bind_phone_result, bpr->operator_info.bind_message);

	jniCopyString(env, class_bind_result, "timestamp", bind_phone_result, bpr->operator_info.timestamp);
	 
	cl_user_free_bind_phone_operation_result(bpr);
	
	(*env)->DeleteLocalRef(env, class_bind_result);
	
	return bind_phone_result;
 }
 

 JNIEXPORT jint JNICALL
  NAME( ClUserRelogin )(JNIEnv* env, jobject this, jint user_handle)
  {
	  RS ret;
	  ret = cl_user_relogin(user_handle);
	  return ret;
  }


