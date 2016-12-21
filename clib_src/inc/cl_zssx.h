/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zssx.h
**  File:    cl_zssx.h
**  Author:  liubenlong
**  Date:    09/30/2015
**
**  Purpose:
**    ��ɽ���͵翾¯.
**************************************************************************/


#ifndef CL_ZSSX_H
#define CL_ZSSX_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */
typedef enum {
	ZSSX_ACTION_POWER_ONOFF,//����
	ZSSX_ACTION_LOCK_ONOFF,//������
	ZSSX_ACTION_ANION_ONOFF,//�����ӿ���
	ZSSX_ACTION_LED_ONOFF,//�ƹ⿪��
	ZSSX_ACTION_FIRE_LEVEL,//���漶��
	ZSSX_ACTION_FAKE_LEVEL,//�Ӳ񼶱�
	ZSSX_ACTION_GEARS,//��λ�趨
	ZSSX_ACTION_TMP_TYPE,//�¶���������
	ZSSX_ACTION_TMP_SET,//�¶�����
	ZSSX_ACTION_TMP_ONOFF,//�¶ȿ���
}ZSSX_ACTION_T;

typedef struct {
	char ssid[33];
	char pswd[65];
}cl_zssx_ssid_t;

typedef struct {
	//����
	u_int8_t power_onoff;//0-�أ�1-��
	u_int8_t lock_onoff;//ͯ��[App������û��]��0-�أ�1-��
	u_int8_t anion_onoff;//�����ӿ���,0-�أ�1-��
	u_int8_t tmp_type;//�¶����ͣ�0-���϶ȣ�1-���϶�
	u_int8_t led_onoff;//�ƹ�[App������û�п���]��0-�أ�1-����
	u_int8_t tmp_onoff;//�¶ȿ��أ�0-�أ�1-��
	//�����趨״̬1
	u_int8_t fire_level;//���漶�� 0:��;01:L1;02:L2-------09:L9����9��
	u_int8_t fake_firewood;//�ٲ񼶱� 0:��;01:L1;02:L2-------09:L9����9��
	//�����趨״̬2
	u_int8_t speed_gears;//�޼�����:00:D1;02:D2----08:D9 ��9��
	u_int8_t timer_on;//��ʱ����0-�ޣ�1-����
	u_int8_t timer_off;//��ʱ�أ�0-�ޣ�1-����
	u_int8_t timer_hour;//��ʱСʱ��0-23
	u_int8_t timer_min;//��ʱ���ӣ�0-59
	//�趨�¶�
	u_int8_t tmp;//�趨�¶ȣ�û�и���
	//����
	u_int8_t ntc_fault;//NTC����
	u_int8_t thermostat_fault;//�¿�������
	//������ʾ״̬
	u_int8_t work_status;//Bit0�����ȹ�����0-�أ�1-��
	//apģʽ
	u_int8_t wifi_mode;//0=sta,1=ap

	//wifi config
	cl_zssx_ssid_t wifi_conf;
}cl_zssx_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	����:
		�������������
	�������:
		@dev_handle: �豸�ľ��
		@action:������Ϊ
		@value:ֵ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_zssx_ctrl(cl_handle_t dev_handle, ZSSX_ACTION_T action, u_int8_t value);

/*
	����:
		��ʱ��
	�������:
		@dev_handle: �豸�ľ��
		@on:��ʱ�����أ�0-��ʱ�أ�1-��ʱ��
		@min:��ʱ��������ʱ�䣬�÷��ӱ�ʾ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_zssx_timer(cl_handle_t dev_handle, u_int8_t on, u_int16_t min);

/*
	����:
		wifi����
	�������:
		@dev_handle: �豸�ľ��
		@wifi:wifi ssid, pswd����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_zssx_wifi_conf(cl_handle_t dev_handle, cl_zssx_ssid_t *wifi);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZSSX_H */

