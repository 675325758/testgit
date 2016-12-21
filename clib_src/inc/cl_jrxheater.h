/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: ���о��â��ˮ��
**  File:    cl_jrxheater.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/06/2016
**
**  Purpose:
**    ���о��â��ˮ��.
**************************************************************************/


#ifndef CL_JRXHEATER_H
#define CL_JRXHEATER_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
enum {
	ACT_JRXHEATER_ONOFF,
	ACT_JRXHEATER_MODE,
	ACT_JRXHEATER_TEMP,
	ACT_JRXHEATER_CHILDLOCK,
	ACT_JRXHEATER_PERIOD_TIMER,
};

/* Type definitions. */
typedef struct {
	u_int8_t valid;
	u_int8_t enable;
	u_int8_t id;	// ID ��1��ʼ
	u_int8_t hour;
	u_int8_t min;
	u_int8_t set_temp;	// ��ʱ������������¶ȣ���Χ20-80���϶�
} cl_jrxheater_sample_timer_t;

typedef struct {
	u_int8_t valid;
	u_int8_t enable;
	u_int8_t id;	// ID ��1��ʼ
	u_int8_t start_hour;
	u_int8_t start_min;
	u_int8_t end_hour;
	u_int8_t end_min;
	u_int8_t set_temp;	// ��ʱ������������¶ȣ���Χ20-80���϶�
} cl_jrxheater_period_timer_t;


typedef struct {
	u_int8_t onoff;	// �豸���أ�0-�ػ� 1-����
	u_int8_t mode;	// 0-����ģʽ 1-����ģʽ 2-ȫ���ģʽ 3-���Ի�ģʽ
	u_int8_t set_temp; // �û�������ˮ���������¶ȣ���Χ20-80���϶�
	u_int8_t child_lock; // ͯ�����أ�0-�ر� 1-����
	u_int8_t work_state; // ״̬��1-���� 2-����
	u_int8_t temp;	// ˮ���¶ȣ���ǰˮ����¶ȣ���Χ20-80���϶�
	u_int8_t capacity;	// ������ˮ���С����λL
	u_int8_t power; // ��ˮ�����ʣ���λW

	// APPҳ����Ҫ��ʾ��ˮ���ﵽˮ���¶ȵ�ʱ��
	u_int8_t hour;
	u_int8_t min;
} cl_jrxheater_stat_t;

typedef struct {
	cl_jrxheater_stat_t stat;
	cl_jrxheater_sample_timer_t sample_timer[8];
	cl_jrxheater_period_timer_t period_timer[8];
} cl_jrxheater_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */


/*
	����: ��������
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������ACT_JRXHEATER_XXX����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jrxheater_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

/*
	����: ���ڶ�ʱ��
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ����1 ���2 �ر� 3 �޸� 4 ɾ��
		@valid: �Ƿ����
		@enable: �Ƿ�ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jrxheater_period_timer(cl_handle_t dev_handle, u_int8_t action, u_int8_t valid, u_int8_t enable, u_int8_t id, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min, u_int8_t temp);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_JRXHEATER_H */

