/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: �����¿���
**  File:    cl_cjthermostat.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    02/18/2016
**
**  Purpose:
**    �����¿���.
**************************************************************************/


#ifndef CL_CJTHERMOSTAT_H
#define CL_CJTHERMOSTAT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */

enum {
	ACT_CJTHERMOSTAT_LOCK,	// 0  ������� 1 �����
	ACT_CJTHERMOSTAT_POWER,	//  0 �ػ� 1 ���������ʾ2 �������Ϩ��
	ACT_CJTHERMOSTAT_TIMER_WEEK,	// �Զ����ڣ�bit0 ������ bit1-6 ����1��6
	ACT_CJTHERMOSTAT_TEMP_ADJUST,	// ��У-9 �� 9
	ACT_CJTHERMOSTAT_TEMP_UPPER_LIMIT,	// ��������
	ACT_CJTHERMOSTAT_TEMP_LOWER_LIMIT,	// ��������
	ACT_CJTHERMOSTAT_TEMP_ALLOWANCE,	// �¿��ݲ�
	ACT_CJTHERMOSTAT_DEFROST_TEMP,	// ��˪�¶�
	ACT_CJTHERMOSTAT_OVERTEMP,	// �����¶�
	ACT_CJTHERMOSTAT_OVERTEMP_ALLOWANCE,	// �����ݲ�
	ACT_CJTHERMOSTAT_FLAG,				// bit 0 �ͺ�:0: S��1:D��  bit 1 Ԥ�ñ�־: 0 סլģʽ1�칫ģʽ
	ACT_CJTHERMOSTAT_MODE,	// ����ģʽ1 ���� 2 �ֶ�3 ��Ϣ4 �Զ� 5 ��˪ 6 Ԥ��
	ACT_CJTHERMOSTAT_MANUAL_TEMP,	// �ֶ�ģʽ�¶��趨
	ACT_CJTHERMOSTAT_RESET,	// ��λ
	// ���沿��ʹ��cl_zkcleanner_ctrl �ӿ�
	
	ACT_CJTHERMOSTAT_PERIOD_TEMP_WORKDAY,
	ACT_CJTHERMOSTAT_PERIOD_TEMP_OFFDAY,
};

enum {
    CJ_WORKMODE_TYPE_WORK = 1,
    CJ_WORKMODE_TYPE_MANUAL,
    CJ_WORKMODE_TYPE_REST,
    CJ_WORKMODE_TYPE_AUTO,
    CJ_WORKMODE_TYPE_DEFORST,
    CJ_WORKMODE_TYPE_PRESET,
};

/* Type definitions. */
typedef struct {
	u_int16_t outtime_hour;	// ���ʱ����Сʱ
	u_int8_t outtime_min;	// ���ʱ��������
	u_int8_t ver;	// �汾��
	u_int8_t is_heat;	// �Ƿ��������� 0 δ���� 1 ����
	u_int8_t week;	// �¿�������
	u_int8_t time;	// ��ǰʱ��0-240,��λΪ6����
	u_int8_t stat;	// 0���޲���1 һ����� 2 ��̨�趨 3 ��������  ��0�����޲�������ָ�¿����ϵ���ӵ������ָ���û���ٶ԰������й������� Ҳ�������Ϊ���޲����� ���UI������û���õ���
	u_int8_t set_temp;	// �趨�¶ȣ�����
	u_int8_t inside_temp;		// ��̽ͷ�¶ȣ��������֡�����
	u_int8_t inside_temp1;	// ��̧ͷ�¶ȣ�С������
	u_int8_t outside_temp;	// ��̧ͷ�¶ȣ�������������
	u_int8_t outside_temp1;	// ��̧ͷ�¶ȣ�С������
	u_int8_t mode;	// ģʽ1 ���� 2 �ֶ�3 ��Ϣ4 �Զ� 5 ��˪ 6 Ԥ��
	u_int8_t power;	// 0 �ػ�1 ������Ļ��ʾ2������Ļ�ر�
	u_int8_t key_lock;	// 0 ������ 1 ������
	u_int8_t fault;	// ���� 0 ���� 1 ����̽ͷ��2����̽ͷ��3����̽ͷ��4��������
	int8_t temp_adjust;	// ��У:-9��9
	u_int8_t set_temp_upper_limit;	// �¶���������
	u_int8_t set_temp_lower_limit;	// �¶���������
	u_int8_t temp_allowance;	// �¿��ݲ�
	u_int8_t defrost_temp;	// ��˪�¶�
	u_int8_t overtemp;	// �����¶�
	u_int8_t overtemp_allowance;	// �����ݲ�
	u_int8_t flag;	// bit 0 �ͺ�:0: S��1:D��  bit 1 Ԥ�ñ�־: 0 סլģʽ1�칫ģʽ
	u_int8_t timer_week;	// �Զ���������bit0-bit7��ʾ����7��1 ~6
	u_int8_t manual_temp;	// �ֶ�ģʽ�¶��趨
} cl_cjthermostat_stat_t;

typedef struct {
	cl_cjthermostat_stat_t stat;
	u_int8_t work_period_temp[48];		// ������48��ʱ���¶����ã�ÿ��Сʱһ����
	u_int8_t offday_period_temp[48];	// ��Ϣ��48��ʱ���¶����ã�ÿ��Сʱһ����
} cl_cjthermostat_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	����: �Դ����¿����Ļ���״̬����
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������ACT_CJTHERMOSTAT_XXX����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_cjthermostat_ctrl(cl_handle_t dev_handle, u_int8_t action, int8_t value);

/*
	����: �Դ����¿���������ʱ���������
		
	�������:
		@dev_handle: �豸�ľ��
		@type: 0 ���ù������¶� 1 ������Ϣ���¶�
		@temp: 48���¶ȵ㣬ÿ�����ʾ��Сʱ
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_cjthermostat_period_temp(cl_handle_t dev_handle, u_int8_t type, u_int8_t temp[48]);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_CJTHERMOSTAT_H */

