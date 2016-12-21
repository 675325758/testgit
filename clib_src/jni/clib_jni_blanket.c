#include "clib_jni.h"


/************************************************************************************

		²ÊºçµçÈÈÌºÏà¹Ø½Ó¿Ú
		
 ************************************************************************************/

/*
	¹¦ÄÜ:
		²éÑ¯ÎÂÇøÐÅÏ¢
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketQueryInfo)(JNIEnv* env, jobject this, jint dev_handle, jbyte area)
{
	return cl_blanket_query_info(dev_handle, area);
}

/*
	¹¦ÄÜ:
		¿ª¹ØÎÂÇø
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
		@on_off   0x0: ¹Ø±Õ 1:¿ªÆô
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetOnOff)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte on_off)
{
	return cl_blanket_set_on_off(dev_handle, area, on_off);
}

/*
	¹¦ÄÜ:
		ÉèÖÃÎÂÇøÎÂ¶È
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
		@temp 18-48¶È
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte temp)
{
	return cl_blanket_set_temp(dev_handle, area, temp);
}

/*
	¹¦ÄÜ:
		ÉèÖÃÎÂÇøÇúÏßÊÇ·ñÉúÐ§
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
		@is_enable  0x0:¹Ø±Õ 0x1:¿ªÆô
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/

JNIEXPORT jint JNICALL
 NAME(CLBlanketSetCurveEnable)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte enable)
{
	return cl_blanket_set_curve_enable(dev_handle, area, enable);
}
/*
	¹¦ÄÜ:
		ÉèÖÃÎÂÇøÊÖ¶¯¶¨Ê±
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
		@time  0x0:¹Ø±Õ 0x1--0x9 ¶¨Ê±Ê±¼ä
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/
JNIEXPORT jint JNICALL
 NAME(CLBlanketSetTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte time)
{
	return cl_blanket_set_timer(dev_handle, area, time);
}
/*
	¹¦ÄÜ:
		ÉèÖÃÎÂÇøÇúÏßå£(Ã¿Ð¡Ê±Ò»¸öµã)
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
		@curve: ´Ó0µãÖÁ23µã,¹Ø±ÕÖÃ0,¿ªÆôÊ±ÉèÖÃÎÂ¶È
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetHourCurve)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyteArray time)
{
	u_int8_t *curveData = (*env)->GetByteArrayElements(env, time, NULL);
	int ret = 0;
	
	ret = cl_blanket_set_hour_curve(dev_handle, area, curveData);
	(*env)->ReleaseByteArrayElements(env, time, curveData, 0);
	return ret;
}

/*
	¹¦ÄÜ:
		ÉèÖÃÎÂÇøÇúÏß(Ã¿°ëÐ¡Ê±Ò»¸öµã)
	ÊäÈë²ÎÊý:
		@dev_handle: µçÈÈÌºµÄ¾ä±ú
		@area  ÎÂÇøÑ¡Ôñ
		@curve: ´Ó0µãÖÁ23:30ã,¹Ø±ÕÖÃ0,¿ªÆôÊ±ÉèÖÃÎÂ¶È
	Êä³ö²ÎÊý:
		ÎÞ
	·µ»Ø:
		RS_OK: ³É¹¦
		ÆäËû: Ê§°Ü
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetHalfHourCurve)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyteArray time)
{
	u_int8_t *curveData = (*env)->GetByteArrayElements(env, time, NULL);
	int ret = 0;
	
	ret = cl_blanket_set_half_hour_curve(dev_handle, area, curveData);
	(*env)->ReleaseByteArrayElements(env, time, curveData, 0);
	return ret;
}

JNIEXPORT jint JNICALL
 NAME(ClBlanketSetMode)(JNIEnv* env, jobject this, jint dev_handle, jboolean area, jbyte mode)
{
	return cl_blanket_set_work_mode(dev_handle, area, mode);
}


