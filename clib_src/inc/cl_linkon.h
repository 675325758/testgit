/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: linkon�¿���
**  File:    cl_linkon.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    02/17/2016
**
**  Purpose:
**    Linkon�¿���.
**************************************************************************/


#ifndef CL_LINKON_H
#define CL_LINKON_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
// LINKON�¿���
enum {
	ACT_LINKON_POWER,	// 0 �ػ� 1 ����
	ACT_LINKON_LOCK,	//  0 �ر�ͯ�� 1 ����ͯ��
	ACT_LINKON_WORK_MODE,	// 0 ����ģʽ 1 ����ģʽ2���ģʽ
	ACT_LINKON_RUNNING_MODE,	// 0 ���� 1����2����
	ACT_LINKON_WIND_SPEED,	// 0�ͷ� 2 �з� 3 �߷�
	ACT_LINKON_TEMP,	// �����¶ȣ���λ0.1��
};


/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	u_int8_t power;	// 0:�ػ� 1������
	u_int8_t lock;	// 0-ͯ���� 1-ͯ����
	int16_t house_temp;	// ���£�����Ϊ����,��λ0.1��
	u_int16_t const_temp;	// �����¶�,��λ0.1��
	u_int16_t go_out_temp;	// ����¶�,��λ0.1��
	u_int16_t save_temp;	// �����¶�,��λ0.1��
	u_int8_t humidity;	// ʪ��
	u_int8_t work_mode;	// ����ģʽ��0-���£�1-���ܣ�2-���
	u_int8_t running_mode;	// ����ģʽ��0-���ȣ�1-���䣬2-����
	u_int8_t wind_speed;	// ���٣�0-�ͷ磬1-�з磬2-�߷�
} cl_linkon_stat_t;

typedef struct {
	cl_linkon_stat_t stat;
} cl_linkon_info_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	����:LINKON�¿����������ƽӿ�
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������,ACT_LINKON_XXX
		@value: ����ֵ
	�������:
		��
	����:

		
*/
CLIB_API RS cl_linkon_sample_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_LINKON_H */

