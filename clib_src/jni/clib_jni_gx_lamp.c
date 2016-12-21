#include "clib_jni.h"

// µ÷¹âµÆ
JNIEXPORT jint JNICALL
NAME(ClLampCtrlPower)(JNIEnv* env, jobject this, jint dev_handle, jboolean onOff)
{
	return cl_lamp_ctrl_power(dev_handle, !!onOff);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlFullLight)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_full_light(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlWhite)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_white(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlWarm)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_warm(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlMix)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_mix(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlBrightness)(JNIEnv* env, jobject this, jint dev_handle, jint brightness)
{	
	return cl_lamp_ctrl_brightness(dev_handle, brightness);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlWarmness)(JNIEnv* env, jobject this, jint dev_handle, jint warmness)
{
	return cl_lamp_ctrl_warmness(dev_handle, warmness);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlRGB)(JNIEnv* env, jobject this, jint dev_handle, jint r, jint g, jint b)
{
	return cl_lamp_ctrl_rgb(dev_handle, r, g, b);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlAuto)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_auto(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlSaving)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_energy_saving(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlNight)(JNIEnv* env, jobject this, jint dev_handle, jint is_on)
{
	return cl_lamp_ctrl_night(dev_handle, is_on);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlMode1)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_model1(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampCtrlMode2)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_ctrl_model2(dev_handle);
}

JNIEXPORT jint JNICALL
NAME(ClLampQueryAll)(JNIEnv* env, jobject this, jint dev_handle)
{
	return cl_lamp_query_all(dev_handle);
}
