/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zkrsq.h
**  File:    cl_zkrsq.h
**  Author:  liubenlong
**  Date:    11/16/2015
**
**  Purpose:
**    �ǿ���ˮ��.
**************************************************************************/


#ifndef CL_ZKRSQ_H
#define CL_ZKRSQ_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */

typedef enum {
	ZKRSQ_ACTION_ONOFF,//����
	ZKRSQ_ACTION_MODE,// ģʽ
	ZKRSQ_ACTION_TMP,//�¶�
}ZKRSQ_ACTION_T;

/*
	fault:
		BIT0	0	RESERVED
		BIT1	1	ͨѶ����
		BIT2	1	ˮ���¶ȹ���
		BIT3	1	�����¶ȹ���
		BIT4	1	�̹��¶ȹ���
		BIT5	1	��ˮ�¶ȹ���
		BIT6	1	��ѹ����
		BIT7	1	��ѹ����
*/

typedef struct {
	int8_t back_tmp;//�ز��¶� 1�� ��10��
	int8_t compensation_tmp;//�����¶� 0�� ��10��
	u_int8_t defrost_time;//��˪ʱ�䣬��Χ 20MIN �C 90MIN Ĭ��45MIN
	int8_t defrost_in_tmp;//��˪�����¶� -9����5��
	u_int8_t defrost_continue_time;//��˪����ʱ�� 3MIN �C 12MIN Ĭ��8MIN
	int8_t defrost_out_tmp;//��˪�˳��¶�	5�� ��25��
	int8_t back_water_tmp;//��ˮ�¶� 5����60��
	u_int8_t back_water_mode;//��ˮģʽ 0:�Զ�ģʽ/1:��ͨģʽ��
}cl_zk_confset_t;

typedef struct{
	u_int8_t timer1_valid;//�Ƿ���Ч
	u_int8_t timer1_onoff;//1:on, 0:off
	u_int8_t timer1_hour;//��ʱʱ�䣬Сʱ
	u_int8_t timer1_min;//��ʱʱ�䣬����
	u_int8_t timer1_hour_end;//��ʱʱ�䣬����Сʱ
	u_int8_t timer1_min_end;//��ʱʱ�䣬��������
	
	u_int8_t timer2_valid;//�Ƿ���Ч
	u_int8_t timer2_onoff;//1:on, 0:off
	u_int8_t timer2_hour;//��ʱʱ�䣬Сʱ
	u_int8_t timer2_min;//��ʱʱ�䣬���ӱ�ʾ
	u_int8_t timer2_hour_end;//��ʱʱ�䣬����Сʱ
	u_int8_t timer2_min_end;//��ʱʱ�䣬��������
	
	u_int8_t timer3_valid;//�Ƿ���Ч
	u_int8_t timer3_onoff;//1:on, 0:off
	u_int8_t timer3_hour;//��ʱʱ�䣬Сʱ
	u_int8_t timer3_min;//��ʱʱ�䣬���ӱ�ʾ
	u_int8_t timer3_hour_end;//��ʱʱ�䣬����Сʱ
	u_int8_t timer3_min_end;//��ʱʱ�䣬��������
}cl_zkrsq_tiemr_t;

typedef struct {
	u_int8_t onoff;//���أ�1����ʾON  , 0����ʾOFF.
	u_int8_t back_water_onoff;//���أ�1����ʾON  , 0����ʾOFF.
	int8_t hot_water_tmp;//�����¶� 15�桪60�� 
	u_int8_t mode;//����ģʽ 0���յ����䣻1���յ����ȣ�2����ˮ��3���յ�����+��ˮ��4���յ�����+��ˮ��5���Զ�
	//���ò���
	cl_zk_confset_t conf;

	//��ʱ
	cl_zkrsq_tiemr_t timer;

	//����
	u_int8_t fault;//����

	//״̬,�豸��λ
	u_int8_t reset_status;// 1:�豸����λ
	int8_t hot_water_tmp_cur;//��ǰ��ˮ�¶� ��ʾ��Χ(-9����99��)
	int8_t coilpipe_tmp;//�̹��¶� ��ʾ��Χ(-9����99��)
	int8_t room_tmp;//�����¶� ��ʾ��Χ(-9����99��)
	int8_t back_water_tmp_cur;//��ǰ��ˮ�¶� ��ʾ��Χ(-9����99��)
	
}cl_zkrsq_info_t;

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
CLIB_API RS cl_zkrsq_ctrl(cl_handle_t dev_handle, ZKRSQ_ACTION_T action, u_int8_t value);

/*
	����:
		�����������
	�������:
		@dev_handle: �豸�ľ��
		@pconfig:�������ò���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_zkrsq_config(cl_handle_t dev_handle, cl_zk_confset_t *pconfig);

/*
	����:
		��ʱ������
	�������:
		@dev_handle: �豸�ľ��
		@ptimer:��ʱ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_zkrsq_timer(cl_handle_t dev_handle, cl_zkrsq_tiemr_t *ptimer);



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZKRSQ_H */

