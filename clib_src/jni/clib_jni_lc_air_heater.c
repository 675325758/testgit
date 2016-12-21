#include "clib_jni.h"

/*  暖风机 */
JNIEXPORT jint JNICALL
NAME(ClAirHeaterSwitch)(JNIEnv* env, jobject this, jint dev_handle, jboolean onOff)
{
	return cl_sa_ah_ctrl_power(dev_handle, !!onOff);
}

JNIEXPORT jint JNICALL
NAME(ClAirHeaterChangeMode)(JNIEnv* env, jobject this, jint dev_handle, jint mode)
{
	return cl_sa_ah_ctrl_mode(dev_handle, mode);
}

JNIEXPORT jint JNICALL
NAME(ClAirHeaterChangeTemperature)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_add)
{
	return cl_sa_ah_ctrl_temp(dev_handle, !!is_add);
}


JNIEXPORT jint JNICALL
NAME(ClAirHeaterCtrlTimer)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_add)
{
	return cl_sa_ah_ctrl_timer(dev_handle, !!is_add);
}

JNIEXPORT jint JNICALL
NAME(ClAirHeaterOpenECO)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_eco)
{
	return cl_sa_ah_ctrl_eco(dev_handle, !!is_eco);
}

JNIEXPORT jint JNICALL
NAME(ClAirHeaterCtrlShake)(JNIEnv* env, jobject this, jint dev_handle, jboolean is_shake)
{
	return cl_sa_ah_ctrl_shake(dev_handle, !!is_shake);
}

JNIEXPORT void JNICALL
NAME(ClAirHeaterRefreshPowerAndTimer)(JNIEnv* env, jobject this, jint dev_handle)
{
	cl_sa_ah_refresh_power_and_timer(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClAirHeaterCtrlTest)(JNIEnv* env, jobject this, jint dev_handle, jbyte timeout)
{
	return cl_sa_ah_ctrl_test(dev_handle, timeout);
}

