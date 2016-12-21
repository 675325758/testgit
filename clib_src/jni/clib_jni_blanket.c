#include "clib_jni.h"


/************************************************************************************

		�ʺ����̺��ؽӿ�
		
 ************************************************************************************/

/*
	����:
		��ѯ������Ϣ
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketQueryInfo)(JNIEnv* env, jobject this, jint dev_handle, jbyte area)
{
	return cl_blanket_query_info(dev_handle, area);
}

/*
	����:
		��������
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@on_off   0x0: �ر� 1:����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetOnOff)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte on_off)
{
	return cl_blanket_set_on_off(dev_handle, area, on_off);
}

/*
	����:
		���������¶�
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@temp 18-48��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
JNIEXPORT jint JNICALL
 NAME(ClBlanketSetTemp)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte temp)
{
	return cl_blanket_set_temp(dev_handle, area, temp);
}

/*
	����:
		�������������Ƿ���Ч
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@is_enable  0x0:�ر� 0x1:����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/

JNIEXPORT jint JNICALL
 NAME(CLBlanketSetCurveEnable)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte enable)
{
	return cl_blanket_set_curve_enable(dev_handle, area, enable);
}
/*
	����:
		���������ֶ���ʱ
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@time  0x0:�ر� 0x1--0x9 ��ʱʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
JNIEXPORT jint JNICALL
 NAME(CLBlanketSetTimer)(JNIEnv* env, jobject this, jint dev_handle, jbyte area, jbyte time)
{
	return cl_blanket_set_timer(dev_handle, area, time);
}
/*
	����:
		�������������(ÿСʱһ����)
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@curve: ��0����23��,�ر���0,����ʱ�����¶�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
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
	����:
		������������(ÿ��Сʱһ����)
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@curve: ��0����23:30�,�ر���0,����ʱ�����¶�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
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


