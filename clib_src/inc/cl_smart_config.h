#ifndef	__CL_SMART_CONFIG_H__
#define	__CL_SMART_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

/*
	���ܣ�
		��ʼһ�����á�
	�������:
		@ssid: �ֻ����ӵ����ص�ssid
		@passwd: ��������
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_smart_config_start(const char *ssid, const char *passwd);

/*
	���ܣ�
		��ʼһ�����ã�������÷�ʽ�����鲥����㲥ɶ�ġ�
	�������:
		@ssid: �ֻ����ӵ����ص�ssid
		@passwd: ��������
		@m_time:�鲥ʱ�䣬0-255 s ע������
		@m_i_time:�鲥��������÷��͹㲥,0-255s
		@b_time:�㲥ʱ��,0-255 s ע������
		@b_i_time:�㲥��������÷����鲥,0-255 s 
		ע:m_time��b_time����ͬʱΪ0
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_smart_config_start_ext(const char *ssid, const char *passwd, 
	u_int8_t m_time, u_int8_t m_i_time, u_int8_t b_time, u_int8_t b_i_time);
    
/*
 ���ܣ�
    ��ʼ6621һ�����á�
 �������:
    @ssid: �ֻ����ӵ����ص�ssid
    @passwd: ��������
 �������:
    ��
 ���أ�
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_smart_6621_config_start(const char *ssid, const char *passwd);

/*
 ���ܣ�
    ��ʼ�鲥һ������
 �������:
    @ssid: �ֻ����ӵ����ص�ssid
    @passwd: ��������
 �������:
    ��
 ���أ�
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_smart_mbroadcast_config_start(const char *ssid, const char *passwd);


/*
 ���ܣ�
    ��ʼ�鲥һ�����ã��ȵ�ģʽ�鲥����
 �������:
    @ssid: �ֻ����ӵ����ص�ssid
    @passwd: ��������
 �������:
    ��
 ���أ�
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_smart_mbroadcast_config_start_hotspot(const char *ssid, const char *passwd);

/*
	���ܣ�
		����һ�����á�
	�������:
		��
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_smart_config_stop( );

/*
 ���ܣ�
    �߼�һ�����ýӿ�
 �������:
    @ssid: �ֻ����ӵ����ص�ssid
    @passwd: ��������
    @mode:������1-12, 1-6��ʾ�鲥��7-12��㲥����Ӧÿ����������5ms�����鲥���㲥�ķ���ʱ������Χ��5-30ms
 �������:
    ��
 ���أ�
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_advance_smart_config_start(const char *ssid, const char *passwd, u_int8_t mode);


#ifdef __cplusplus
}
#endif 


#endif


