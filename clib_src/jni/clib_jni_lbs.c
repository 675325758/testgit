#include "clib_jni.h"
#include "cl_lbs.h"
#include <math.h>
/************************************************************************************

		位置服务相关接口
		
 ************************************************************************************/


/*
	参数顺序: 纬度、精度
*/
JNIEXPORT jint JNICALL
NAME(ClLbsMark)(JNIEnv* env, jobject this, jdouble latitude, jdouble longitude)
{
	lbs_pos_t pos;
	
	if(latitude>0){
		pos.is_north = 1;
		pos.latitude = latitude;
	}else{
		pos.is_north = 0;
		pos.latitude = fabs(latitude);
	}
	
	if(longitude>0){
		pos.is_east = 1;
		pos.longitude = longitude;
	}else{
		pos.is_east = 0;
		pos.longitude = fabs(longitude);
	}
	
	return lbs_mark(&pos);
}

/*
	基站定位参数设置
*/
JNIEXPORT jint JNICALL
NAME(ClLbsMarkBase)(JNIEnv* env, jobject this,jint count,jintArray arr)
{
	
	lbs_base_t base;
	int i;
	jint* arrs;
    arrs = (*env)->GetIntArrayElements(env,arr,NULL);
	base.count = count;
	
	for(i=0;i<count;i++){
		base.base[i] = arrs[i];
	}
	(*env)->ReleaseIntArrayElements(env, arr, arrs, 0); //需要确认
	return lbs_mark_base(&base);
}

	
JNIEXPORT jobject JNICALL
NAME(ClLbsGetInfoMark)(JNIEnv* env, jobject this)
{
	lbs_gdb_list_t *info_t;
	jclass gps_class = NULL;
	int count, i;	
	jobject list_info = NULL;
	jobject gps_array = NULL;
	char tmpsn[32]={0};
	
	gps_class = (*env)->FindClass(env, CLASS_GPSLOCATION_DATA);	
	if( !gps_class )
		return gps_array;
	
	info_t = (lbs_gdb_list_t *)malloc(sizeof(lbs_gdb_list_t));
	if(!info_t){
		(*env)->DeleteLocalRef(env, gps_class);
		return gps_array;
	}
	memset(info_t,0,sizeof(lbs_gdb_list_t));
	
	lbs_get_info(info_t);	
	if(info_t==NULL)return gps_array;
	
	count = info_t->count;
	gps_array = (*env)->NewObjectArray(env, count , gps_class, NULL);
	if(count>0){
		for(i = 0; i < count; i++) {
			
			list_info = (*env)->AllocObject(env, gps_class);
			jniCopyIntValue(env,gps_class,"count",list_info ,count);
			sprintf(tmpsn,"%"PRIu64,info_t->list[i].sn);
			jniCopyString(env,gps_class ,"sn", list_info , tmpsn);
			jniCopyIntValue(env,gps_class ,"speed", list_info , info_t->list[i].speed*10000);			
			jniCopyIntValue(env,gps_class ,"distance", list_info , info_t->list[i].distance*10000);
			jniCopyIntValue(env,gps_class ,"is_in_lan", list_info ,info_t->list[i].is_in_lan);
			jniCopyIntValue(env,gps_class ,"has_mark_home", list_info ,info_t->list[i].has_mark_home);
			jniCopyIntValue(env,gps_class ,"status", list_info ,info_t->list[i].status);	

			(*env)->SetObjectArrayElement(env, gps_array, i, list_info);
			(*env)->DeleteLocalRef(env, list_info);
		}
	}
	(*env)->DeleteLocalRef(env, gps_class);
	
	free(info_t);
	info_t = NULL;
	return gps_array;
}


JNIEXPORT void JNICALL
NAME(ClLbsSetPrecision)(JNIEnv* env, jobject this, jint precision)
{
	lbs_set_precision(precision);
}

JNIEXPORT void JNICALL
NAME(ClLbsMarkHomeTest)(JNIEnv* env, jobject this)
{
	lbs_mark_home_test();
}

/* 纬度距离 */
JNIEXPORT jdouble JNICALL
NAME(ClLbsGetLatitudeDiff)(JNIEnv* env, jobject this)
{
	return lbs_get_latitude_diff();
}

/* 经度距离 */
JNIEXPORT jdouble JNICALL
NAME(ClLbsGetLongitudeDiff)(JNIEnv* env, jobject this)
{
	return lbs_get_longitude_diff();
}

/**网络类型设置: @net_type: NET_TYPE_xxx，网络类型，包括: 无连接、Wi-Fi、2G/3G、以太*/
JNIEXPORT jint JNICALL
NAME(ClSetNettype)(JNIEnv* env, jobject this, jint nettype, jstring desc)
{
	const char * c_desc = NULL;
	int ret = RS_OK;

	if (desc != NULL) {
		c_desc = (*env)->GetStringUTFChars(env, desc, NULL);
	}
	
	ret = cl_set_net_type(nettype, (char*)c_desc);
	if (desc != NULL) {
		(*env)->ReleaseStringUTFChars(env, desc, c_desc);
	}
	
	return ret;
}