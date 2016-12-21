#include "clib_jni.h"


/************************************************************************************

		彩虹电热毯相关接口
		
 ************************************************************************************/

/*
	功能:
		查询温区信息
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketQueryInfo)(JNIEnv* env, jobject this, jint dev_handle, jbyte area)
{
	return cl_blanket_query_info(dev_handle, area);
}

/*
	功能:
		开关温区
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@on_off   0x0: 关闭 1:开启
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetOnOff)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte on_off)
{
	return cl_blanket_set_on_off(dev_handle, area, on_off);
}

/*
	功能:
		设置温区温度
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@temp 18-48度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte temp)
{
	return cl_blanket_set_temp(dev_handle, area, temp);
}

/*
	功能:
		设置温区曲线是否生效
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@is_enable  0x0:关闭 0x1:开启
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

JNIEXPORT jint JNICALL
 NAME(CLBlanketSetCurveEnable)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte enable)
{
	return cl_blanket_set_curve_enable(dev_handle, area, enable);
}
/*
	功能:
		设置温区手动定时
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@time  0x0:关闭 0x1--0x9 定时时间
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
JNIEXPORT jint JNICALL
 NAME(CLBlanketSetTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte time)
{
	return cl_blanket_set_timer(dev_handle, area, time);
}
/*
	功能:
		设置温区曲线澹(每小时一个点)
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@curve: 从0点至23点,关闭置0,开启时设置温度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
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
	功能:
		设置温区曲线(每半小时一个点)
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@curve: 从0点至23:30�,关闭置0,开启时设置温度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
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


