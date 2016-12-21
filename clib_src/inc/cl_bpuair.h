/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: ���ݰ��տ���������
**  File:    cl_bpuair.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/05/2016
**
**  Purpose:
**    ���ݰ��տ���������.
**************************************************************************/


#ifndef CL_BPUAIR_H
#define CL_BPUAIR_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
// cl_bpuair_ctrl��action 
enum {
	ACT_BPUAIR_ONOFF,	// ���ػ�
	ACT_BPUAIR_MODE,    // ����ģʽ 1���� 2 ����
	ACT_BPUAIR_ECO,		
	ACT_BPUAIR_COLD_TEMP,
	ACT_BPUAIR_HOT_TEMP,
	ACT_BPUAIR_TIMER_ENABLE,	// ��ʱ���ܿ���
	ACT_BPUAIR_ONCE_TIMER,	// һ���Զ�ʱ��
	ACT_BPUAIR_PERIOD_TIMER,	// ���ڶ�ʱ��
};


/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	bool valid;	// �Ƿ���Ч
	u_int8_t week;	// ���ڼ���bit 0 - 6��Ӧ����7 1 2 3 4 5 6
	u_int8_t onoff;	// ��ʱ�����ǹ�
	u_int8_t hour;	// ��ʱСʱ
	u_int8_t mins;	// ��ʱ����
} cl_bpuair_timer_t;

typedef struct {
	u_int8_t id;
	bool valid;	// �Ƿ���Ч
	u_int8_t week;	// ���ڼ���bit 0 - 6��Ӧ����1-7
	u_int8_t onoff;	// ��ʱ�����ǹ�
	u_int8_t hour;	// ��ʱСʱ
	u_int8_t mins;	// ��ʱ����
} cl_bpuair_timer_set_t;


typedef struct {
	u_int8_t type;	// �豸����
	/*
		
		0��������
		1�����У�
		2������ͣ����
	*/
	u_int8_t stat1;	
	/*
		0:��
		1��������
		2����˪��
		3�������ͳ�˪��
	*/
	u_int8_t stat2;
	u_int8_t work_mode;	// ����ģʽ 1���� 2 ����
	u_int8_t eco_mode;	// ����ģʽ0 �ر� 1����
	int16_t cold_temp;	// �����趨�¶ȣ���λ0.1�ȣ�10.0 ~ 30.0
	int16_t hot_temp;		// �����趨�¶ȣ���λ0.1�ȣ�10.0 ~ 85.0
	int16_t backwater_temp;	// ��ˮ�¶ȵ�λ0.1��
	int16_t water_temp;	// ϵͳ��ˮ�¶ȵ�λ0.1��
	int16_t env_temp;	// �����¶ȵ�λ0.1��
	int16_t coiler1_temp;	// �̹��¶ȵ�λ0.1��
	int16_t coiler2_temp;	// �̹��¶ȵ�λ0.1��
	int16_t current1;	// ����1 ��λA
	int16_t current2;	// ����2 ��λA

	int16_t eco_cold_temp;
	int16_t eco_hot_temp;

	// ѹ��1�Ĵ���
	bool low_vol_1;	// ѹ����ѹ
	bool high_vol_1;	// ѹ����ѹ
	bool high_current_1;	// ѹ����������
	bool low_current_1;	// ѹ��������С
	bool coiler_high_temp_1;	// �̹��¶�̽ͷ����
	bool exhaust_temp_sensor_1; // �����¶�̽ͷ����
	bool exhaust_temp_high_1;	// �����¶ȹ���

	bool phase_protection_1;	// ȱ�ౣ��
	bool anti_phase_protection_1;	// ���ౣ��

	// ѹ��2�Ĵ���
	bool low_vol_2;	// ѹ����ѹ
	bool high_vol_2;	// ѹ����ѹ
	bool high_current_2;	// ѹ����������
	bool low_current_2;	// ѹ��������С
	bool coiler_high_temp_2;	// �̹��¶�̽ͷ����
	bool exhaust_temp_sensor_err_2; // �����¶�̽ͷ����
	bool exhaust_temp_high_2;	// �����¶ȹ���

	bool fault_phase_protection_2;	// ���ౣ��
	bool eeprom_data_2;	// EEPROM���ݴ���
	bool const_temp_sensor_2; // ����̽ͷ����
	bool sys_temp_return_2;	// ϵͳ���¹���
	bool sys_temp_out_2;	// ϵͳ���¹���
	bool phase_protection_2;	// ȱ�ౣ��
	bool anti_phase_protection_2;	// ���ౣ��

	bool lack_water;	// ˮ������
	bool high_temp_out_water;	// ��ˮ�¶ȹ���
	bool low_temp_out_water;	// ��ˮ�¶ȹ���

	// ��ʱ��
	bool timer_enable;	// ��ʱ���Ƿ�ʹ��

	// һ���Զ�ʱ��
	bool once_timer_enable;	// һ���Զ�ʱ��ʹ��
	bool once_timer_onoff;	// һ���Զ�ʱ������ 0����ʱ�ػ�1����ʱ����
	u_int8_t once_timer_hour;	// һ���Զ�ʱ��Сʱ
	u_int8_t once_timer_min;	// һ���Զ�ʱ������

	u_int16_t uptime;	// ��������

	char unit_code[64];// �������
	char soft_ver1[64]; // ���ư�����汾��
	char soft_ver2[64]; // �ֲ�������汾��

	
	/*
		��������
		[0]
		���ϣ�ĳһλΪ1ʱ���ڸù��ϣ�0ʱ�޹���
		Bit0��1#ѹ����ѹ	   ��24�� Bit1��1#ѹ����ѹ		 ��25��
		Bit2��1#ѹ����������   ��26�� Bit3��1#ѹ��������С	��27��
		Bit4��1#�̹��¶�̽ͷ���ϣ�28��Bit5��1#�����¶�̽ͷ���ϣ�29��
		Bit6��1#�����¶ȹ���   ��30�� Bit7������			 ��31��
		[1]
		���ϣ�ĳһλΪ1ʱ���ڸù��ϣ�0ʱ�޹���
		Bit0������				��32��	Bit1������			��33��
		Bit2������				��34��	Bit3������			��35��
		Bit4������				��36��	Bit5������			 ��37��
		Bit6��ȱ�ౣ��			��38�� Bit7�����ౣ��		 ��39�� 			 
		[2]
		Ԥ��
		[3]
		���ϣ�ĳһλΪ1ʱ���ڸù��ϣ�0ʱ�޹���
		Bit0��2#ѹ����ѹ		 ��48�� Bit1��2#ѹ����ѹ	 ��49��
		Bit2��2#ѹ����������	��50�� Bit3��1#ѹ��������С ��51��
		Bit4��2#�̹��¶�̽ͷ���ϣ�52��Bit5��2#�����¶�̽ͷ���ϣ�53��
		Bit6��2#�����¶ȹ���	��54��		 Bit7������ 	 ��55��
		[4]
		Ԥ��
		[5]
		���ϣ�ĳһλΪ1ʱ���ڸù��ϣ�0ʱ�޹���
		Bit0������		   ��0��		  Bit1�����ౣ��	��1��
		Bit2��EEPROM ���ݴ�2��		  Bit3������̽ͷ���ϣ�3��
		Bit4��ϵͳ���¹��� ��4��		  Bit5��ϵͳ���¹��ϣ�5��
		Bit6������		   ��6��		  Bit7��ȱ�ౣ��	��7��
		[6]
		���ϣ�ĳһλΪ1ʱ���ڸù��ϣ�0ʱ�޹���
		Bit0��ˮ������	   ��8��		   Bit1������		 ��9��
		Bit2����ˮ�¶ȹ��ߣ�10��		   Bit3����ˮ�¶ȹ��ͣ�11��
		Bit4������		   ��12��			Bit5������		   ��13��
		Bit6������		   ��14��			Bit7������			��15��
		
	*/
	u_int8_t fault_array[7];
	
	// �¸���ʱ����ʱ��
	u_int32_t next_timer_stamp;	// �¸���ʱ����ʼ��ʱ�䣬����ʱ��
	bool next_timer_onoff;		// �¸���ʱ���ǿ����ǹ�
} cl_bpuair_stat_t;

typedef struct {
	u_int32_t timestamp;
	u_int32_t fault_number;
} cl_bpuair_fault_item_t;

typedef struct {
	u_int32_t valid;
	u_int32_t num;
	cl_bpuair_fault_item_t fault[10];
} cl_bpuair_fault_t;

typedef struct {
	u_int32_t num;
	cl_bpuair_fault_item_t fault[10];
} cl_bpuair_current_fault_t;
	
typedef struct {
	cl_bpuair_stat_t stat;
	cl_bpuair_timer_t timers[6];	// ���ڶ�ʱ��

	cl_bpuair_fault_t fault_history;

	cl_bpuair_current_fault_t fault_current;	// ��ǰ����

	// ����SDK˽������
	u_int32_t last_ctrl;
} cl_bpuair_info_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	����: �Կյ���������������
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������ACT_BPUAIR_XXX����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_bpuair_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

/*
	����:һ���Զ�ʱ��
		
	�������:
		@dev_handle: �豸�ľ��
		@enable: �Ƿ�ʹ��
		@onoff: �����ǹ� 0 �� 1 ��
		@hour: ��ʱСʱ
		@min: ��ʱ����
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_bpuair_once_timer(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int8_t hour, u_int8_t min);

/*
	����: ���ڶ�ʱ��
		
	�������:
		@dev_handle: �豸�ľ��
		@id: 1 - 7
		@enable: �Ƿ�ʹ��
		@week:  ���ڼ���bit 0 - 6��Ӧ����1-7
		@onoff: 0 �� 1 ��
		@hour: ��ʱСʱ
		@min: ��ʱ����
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_bpuair_period_timer(cl_handle_t dev_handle, u_int8_t id, u_int8_t enable, u_int8_t week, u_int8_t onoff, u_int8_t hour, u_int8_t min);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_BPUAIR_H */

