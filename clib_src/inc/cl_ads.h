/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_ads.h
**  File:    cl_ads.h
**  Author:  liubenlong
**  Date:    08/25/2015
**
**  Purpose:
**    �ĵ�����ˮ��.
**************************************************************************/


#ifndef CL_ADS_H
#define CL_ADS_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
typedef enum {
	ADS_ACTION_ONOFF = 1,//��ˮ������
	ADS_ACTION_BW_ONOFF,//��ˮ���ÿ���
	ADS_ACTION_TMP,//�¶�����
	ADS_ACTION_MAX,
}ADS_ACTION_T;

/* Type definitions. */
#pragma pack(push, 1)

typedef struct{
	u_int8_t water_box_show_tmp;
	u_int8_t water_box_ctrl_tmp;
	u_int8_t scroll_tmp;
	u_int8_t exhaust_tmp;

	u_int8_t out_water_tmp;
	u_int8_t env_tmp;
	u_int8_t back_diff_tmp;
	u_int8_t back_water_tmp;

	u_int8_t compressor_cur;
	u_int8_t compressor_work_time_l;
	u_int8_t compressor_work_time_h;
	u_int8_t sys_uptime_l;
	
	u_int8_t sys_uptime_h;
	u_int8_t defrost_time;
	u_int8_t defrost_tmp;
	u_int8_t water_box_set_tmp;

	u_int8_t first_start_time_h;
	u_int8_t first_start_time_m;
	u_int8_t first_end_time_h;
	u_int8_t first_end_time_m;
	
	u_int8_t second_start_time_h;
	u_int8_t second_start_time_m;
	u_int8_t second_end_time_h;
	u_int8_t second_end_time_m;

	u_int8_t back_water_status;//���ɿ��ȿ���״̬
	u_int8_t defrost_status;
	u_int8_t compressor_status;//ѹ��������
	u_int8_t fault_code;

	u_int8_t work_mode;//ģʽ��������״̬
	u_int8_t sys_type;
	u_int8_t water_level;
	u_int8_t water_box_tmp2;

	u_int8_t water_box_tmp3;
	u_int8_t water_box_tmp4;
	u_int8_t water_pump_arg1;
	u_int8_t water_pump_arg2;

	u_int8_t fault_code2;
	u_int8_t ele_valve_l;
	u_int8_t ele_valve_h;
	u_int8_t pump_clock_h;

	u_int8_t pump_clock_m;
	u_int8_t wind_status;
	u_int8_t pad[6];
}cl_ads_stat_t;

typedef struct {
	u_int16_t sys_clock_time;//ϵͳʱ��(����)
	u_int16_t first_start_time;//��һ�ζ�ʱ�����ȿ�ʼʱ��(����)
	u_int16_t first_end_time;//��һ�ζ�ʱ�����Ƚ���ʱ��(����)
	u_int16_t second_start_time;//�ڶ��ζ�ʱ�����ȿ�ʼʱ��(����)
	u_int16_t second_end_time;//�ڶ��ζ�ʱ�����Ƚ���ʱ��(����)

	u_int16_t defrost_time;//��˪���ʱ�䣬0-240����
	u_int16_t defrost_tmp;//��˪�����¶ȣ�-15~5��
	u_int8_t pad[2];
}cl_ads_conf_t;

#pragma pack(pop)
/*
��˪�¶ȣ�0��~-9��C
�����¶ȣ�-300��C--130��C
�̹��¶ȣ������¶ȣ�ˮ���¶ȣ���ˮ�¶ȣ������¶ȵȣ�-30��C--90��C.
����ʱ�䣺0--99999Сʱ

���� 2015/9/22 14:17:41
ֻ�ǳ�˪��������-30-90�������¶���-30-130
14:20:43
��ʱ�� 2015/9/22 14:20:43
Ŷ���ǳ�˪�����ô�㣬0-15��-30��-30~-15������ǳ�˪��Χ��

���� 2015/9/22 14:21:21
���÷�Χ��0--9.

������:
		fault_code								fault_code2
bit0		E7��0�޹��ϣ�1����						���ǹ��ϣ�����//ѭ���� 0ֹͣ,1Ϊ�������̹ܻ�ʱ��Ϊ0��

bit1		E5��0�޹��ϣ�1����						���ǹ��ϣ�����//��ͨ�� 0ֹͣ,1Ϊ����

bit2		E4��0�޹��ϣ�1����						���ǹ��ϣ�����//��ʱ��0��Ч,1Ϊ��Ч(��ʱ��ʾ)

bit3		E3��0�޹��ϣ�1����						���ǹ��ϣ�����//��ѹ���أ�0Ϊ��Ч��1��Ч

bit4		E2��0�޹��ϣ�1����						E12��0�޹��ϣ�1���� ��ͨ������

bit5		E1��0�޹��ϣ�1����						E10��0�޹��ϣ�1����

bit6		E0��0�޹��ϣ�1����						E9��0�޹��ϣ�1����

bit7		�������й���; 0Ϊδ����1Ϊ����.			E8��0�޹��ϣ�1����

ע��:
E7,E8,E10���ϲ�Ӱ�����С���ֻҪ����e0-e6,e12�Ͳ��ܿ����豸�ˡ�

*/
typedef struct {
	//����������
	u_int8_t on;//��ˮ�����أ�1��ʾ����0��ʾ��
	u_int8_t back_water_on;//ֱ�ӵ��ɿ��ȿ��غ��ˣ������jni��ֱ����
	//u_int8_t tmp;//�����¶ȣ�30~60 ���϶�
	cl_ads_conf_t conf;
	

	//��ʾ����
	u_int16_t water_box_show_tmp;//ˮ����ʾ�¶ȣ�-30-90 ���϶�
	u_int16_t water_box_ctrl_tmp;//�趨�¶�,-30-90 ���϶�
	u_int16_t out_water_tmp;//��ˮ���⣬-30-90 ���϶�
	u_int16_t env_tmp;//�����¶ȣ�-30-90 ���϶�
	u_int16_t back_diff_tmp;//�ز��¶ȣ�-30-90 ���϶�
	u_int16_t compressor_cur;//ѹ������������λ����֪�����Է���û˵��0.1a
	u_int16_t scroll_tmp;//�̹��¶�
	u_int16_t exhaust_tmp;//�����¶�
	u_int8_t defrost;//��˪״̬��0��ʾû�г�˪��1��ʾ��˪
	u_int8_t wind;//���״̬��0��ʾ�أ�1��ʾ��
	u_int8_t fault_code;//�������
	u_int16_t pump_clock;//�ȱ�ϵͳʱ�䣬���ӣ��ɻ����ʱ�֣���ʾ�ȱõ�ǰϵͳʱ��
	u_int8_t fault_code2;//�������
	u_int16_t water_show_tmp;//ˮ������¶���ʾ-30-90 ���϶�

	u_int8_t compressor_status;//ѹ����״̬��app���Բ���


	//һ�±�֤���ݣ�sdk�ã�app���Բ��ܡ���������������������
	cl_ads_stat_t stat;
}cl_ads_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	����:
		�������������
	�������:
		@dev_handle: �豸�ľ��
		@action:������Ϊ
		@value:
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ads_ctrl(cl_handle_t dev_handle, ADS_ACTION_T action, u_int8_t value);

/*
	����:
		�������������
	�������:
		@dev_handle: �豸�ľ��
		@pconf:��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ads_conf(cl_handle_t dev_handle,  cl_ads_conf_t *pconf);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ADS_H */

