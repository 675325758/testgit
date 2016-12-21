#include "clib_jni.h"

/************************************************************************************

		area interface 
		锟斤拷锟斤拷锟斤拷亟涌锟� ************************************************************************************/
 

JNIEXPORT jint JNICALL
NAME(ClAreaDel)(JNIEnv* env, jobject this, jint area_handle)
{
    return cl_area_del(area_handle);
}

JNIEXPORT jint JNICALL
NAME(ClAreaChangeName)(JNIEnv* env, jobject this, jint area_handle, jstring area_name)
{
	const char *name;
	RS ret = RS_ERROR;
	
	name = (*env)->GetStringUTFChars(env, area_name, NULL);
	ret = cl_area_change_name(area_handle, name);
	
	(*env)->ReleaseStringUTFChars(env, area_name, name);
	
    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClAreaChangeImageResv)(JNIEnv* env, jobject this, jint area_handle, jbyte img_resv)
{
    return cl_area_change_imge_resv(area_handle, img_resv);
}

JNIEXPORT jint JNICALL
NAME(ClAreaModifyAppliances)(JNIEnv* env, jobject this, jint area_handle, jintArray handles, jint handles_count)
{
	cl_handle_t *handles_array = NULL;
	int ret;

	if (handles != NULL)
		handles_array = (*env)->GetIntArrayElements(env, handles,false);
	//handles_count = (*env)->GetArrayLength(env, handles);

    ret = cl_area_modify_appliances(area_handle, handles_count, handles_array);
    LOGE("%s, ret=%d, t_handle=0x%08x", __FUNCTION__, ret, area_handle);

    if (handles_array != NULL)
    	(*env)->ReleaseIntArrayElements(env, handles,handles_array,0);

    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClAppliancesChangeArea)(JNIEnv* env, jobject this, jint appliances_handle, jint area_handle)
{
    return cl_appliance_change_area(appliances_handle, area_handle);
}


//CLIB_API RS cl_area_modify(cl_handle_t area_handle, const char* name, u_int8_t img_resv, u_int8_t item_count,cl_handle_t* handles);
JNIEXPORT jint JNICALL
NAME(ClAreaModify)(JNIEnv* env, jobject this, jint area_handle,  jstring area_name, jbyte img_resv,  jint handles_count, jintArray handles)
{
	const char *name;
	cl_handle_t *handles_array = NULL;
	int ret;

	name = (*env)->GetStringUTFChars(env, area_name, NULL);
	if (handles != NULL)
		handles_array = (*env)->GetIntArrayElements(env, handles,false);
	//handles_count = (*env)->GetArrayLength(env, handles);

    ret = cl_area_modify(area_handle, name, img_resv, handles_count, handles_array);
    LOGE("%s, ret=%d, t_handle=0x%08x", __FUNCTION__, ret, area_handle);

    if (handles_array)
	(*env)->ReleaseIntArrayElements(env, handles,handles_array,0);
    (*env)->ReleaseStringUTFChars(env, area_name, name);

    return ret;
}

//CLIB_API RS cl_area_add_3(cl_handle_t user_handle, cl_handle_t *area_handle, const char* name, u_int8_t img_resv, u_int8_t item_count,cl_handle_t* handles)

JNIEXPORT jint JNICALL
NAME(ClAreaAdd)(JNIEnv* env, jobject this, jint user_handle,
		jint area_handle,  jstring area_name, jbyte img_resv,
		jint handles_count, jintArray handles)
{
	const char *name;
	cl_handle_t *handles_array = NULL;
	int ret;
	cl_handle_t t_handle = 0;

	name = (*env)->GetStringUTFChars(env, area_name, NULL);
	if (handles != NULL) {
		handles_array = (*env)->GetIntArrayElements(env, handles,false);
	}
	//handles_count = (*env)->GetArrayLength(env, handles);

    ret = cl_area_add_3(user_handle, &t_handle, name, img_resv, handles_count, handles_array);
    LOGE("%s, ret=%d, t_handle=0x%08x", __FUNCTION__, ret, t_handle);

    if (handles_array != NULL) {
    	(*env)->ReleaseIntArrayElements(env, handles,handles_array,0);
    }
    (*env)->ReleaseStringUTFChars(env, area_name, name);

    return ret;
}

JNIEXPORT jint JNICALL
NAME(ClUDISpecialAreaAdd)(JNIEnv* env, jobject this, jint user_handle,
		jint area_handle,  jstring area_name, jbyte img_resv,
		jint handles_count, jintArray handles)
{
	const char *name;
	cl_handle_t *handles_array = NULL;
	int ret;
	cl_handle_t t_handle = 0;

	name = (*env)->GetStringUTFChars(env, area_name, NULL);
	if (handles != NULL) {
		handles_array = (*env)->GetIntArrayElements(env, handles,false);
	}
	//handles_count = (*env)->GetArrayLength(env, handles);

    ret = cl_area_add_3(user_handle, &t_handle, name, img_resv, handles_count, handles_array);
    LOGE("%s, ret=%d, t_handle=0x%08x", __FUNCTION__, ret, t_handle);

	if(ret!=RS_OK){
		t_handle = -1;
	}
	
    if (handles_array != NULL) {
    	(*env)->ReleaseIntArrayElements(env, handles,handles_array,0);
    }
    (*env)->ReleaseStringUTFChars(env, area_name, name);

    return t_handle;
}
