#ifndef CL_DRKZQ_H
#define CL_DRKZQ_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

// cl_drkzq_ctrl��action 
enum {
	act_drkzq_humi_threshold1 = 1,	// ����1ʪ�ȷ�ֵ����0-100
	act_drkzq_humi_threshold2,	// ����1ʪ�ȷ�ֵ����0-100
	act_drkzq_humi_threshold3,	// ����1ʪ�ȷ�ֵ����0-100
	act_drkzq_humi_threshold4,	// ����1ʪ�ȷ�ֵ����0-100
	act_drkzq_safe_temp_threshold,	// ������ȫ�¶ȷ�ֵ0-60���϶�
	act_drkzq_cycling_water_temp_threshold,	// ��ˮ�ذ�ȫ�¶ȷ�ֵ 0-60���϶�
	act_drkzq_fertilzer_hour,	// ʩ��ʱ�� Сʱ
	act_drkzq_sunshine_threshold,	// �����շ�ֵ
	act_drkzq_safe_ph_threshold,	// ��ȫPH��ֵ0-100
	act_drkzq_watering_onoff1,// ����1�������
	act_drkzq_watering_onoff2,// ����2�������
	act_drkzq_watering_onoff3,// ����3�������
	act_drkzq_watering_onoff4,// ����4�������
	act_drkzq_sunshine_onoff,	// ���տ���
	act_drkzq_feretilize_onoff,	// ʩ�ʿ���
	act_drkzq_light_onoff,	// ��������
	act_drkzq_water_pump_onoff,	// ˮ�ÿ���
	act_drkzq_resv_onoff1,	// ��������1
	act_drkzq_resv_onoff2,	// ��������1
	act_drkzq_resv_onoff3,	// ��������1
	act_drkzq_resv_onoff4,	// ��������1
	act_drkzq_work_mode,	// ����ģʽ1ȫ�Զ�ģʽ��2��ʱģʽ��3�ֶ�ģʽ��4��ʱ�Զ�ģʽ
	act_drkzq_sys_time, // ϵͳʱ�� ��2�ֽڱ�ʾ����8λΪСʱ����8Ϊ����

	// �����SDK�Լ�ʹ��
	act_drkzq_timer,
	act_drkzq_name,
};

enum {
	// 4�����������
	DRKZQ_NAME_ZERO1,
	DRKZQ_NAME_ZERO2,
	DRKZQ_NAME_ZERO3,
	DRKZQ_NAME_ZERO4,

	DRKZQ_NAME_LIGHT,	// ������������
	DRKZQ_NAME_WATER_PUMP,	// ˮ��ˮ��ˮ�ÿ���

	// 4��������������
	DRKZQ_NAME_RESV1,
	DRKZQ_NAME_RESV2,
	DRKZQ_NAME_RESV3,
	DRKZQ_NAME_RESV4,

	DRKZQ_NAME_MAX,
};

typedef struct {
	// 4���������ʪ��ֵ
	u_int8_t humi1;
	u_int8_t temp1;
	u_int8_t humi2;
	u_int8_t temp2;
	u_int8_t humi3;
	u_int8_t temp3;
	u_int8_t humi4;
	u_int8_t temp4;

	u_int8_t ph;
	u_int8_t cycling_water_temp;	// ��ˮ��ˮ��
	u_int8_t analog;	// ģ��ɼ�ֵ

	// �ĸ����򽽹����0 �� 1 ��
	u_int8_t watering_onoff1;
	u_int8_t watering_onoff2;
	u_int8_t watering_onoff3;
	u_int8_t watering_onoff4;

	u_int8_t sunshine_onoff;	// ���տ���
	u_int8_t feretilize_onoff;	// ʩ�ʿ���
	u_int8_t light_onoff;	// ��������
	u_int8_t water_pump_onoff;	// ˮ�ÿ���

	// 4�����ÿ���
	u_int8_t resv_onoff1;
	u_int8_t resv_onoff2;
	u_int8_t resv_onoff3;
	u_int8_t resv_onoff4;

	u_int8_t work_mode;	// ����ģʽ1ȫ�Զ�ģʽ��2��ʱģʽ��3�ֶ�ģʽ��4��ʱ�Զ�ģʽ

	// �ĸ����򽽹�ʪ�ȷ�ֵ
	u_int8_t humi_threshold1;	
	u_int8_t humi_threshold2;
	u_int8_t humi_threshold3;
	u_int8_t humi_threshold4;

	u_int8_t safe_temp_threshold;	// ������ȫ�¶ȷ�ֵ
	u_int8_t cycling_water_temp_threshold;	// ��ˮ�ذ�ȫ�¶ȷ�ֵ
	u_int8_t fertilzer_second;	// ʩ��ʱ��4~120,��λΪ�룬��ʩ�ʿ���ʱ��
	u_int8_t sunshine_threshold;	// �����շ�ֵ
	u_int8_t safe_ph_threshold;	// ��ȫPH��ֵ
	u_int8_t sys_min;	// ϵͳʱ�� ����
	u_int8_t sys_hour;	// ϵͳʱ��Сʱ

	// ��һ�鶨ʱ��
	u_int8_t timer1_start_min;
	u_int8_t timer1_start_hour;
	u_int8_t timer1_end_min;
	u_int8_t timer1_end_hour;
	u_int8_t timer1_valid;	// �Ƿ���Ч

	// �ڶ��鶨ʱ��
	u_int8_t timer2_start_min;
	u_int8_t timer2_start_hour;
	u_int8_t timer2_end_min;
	u_int8_t timer2_end_hour;
	u_int8_t timer2_valid;	// �Ƿ���Ч
} cl_drkzq_stat_t;

// ��ǰ�쳣
typedef struct {
	u_int8_t temp_sensor;	// �¶�̽ͷ�쳣
	u_int8_t humi_sensor;	// ʪ��̽ͷ�쳣
	u_int8_t ph_sensor;	// ph̽ͷ�쳣
	u_int8_t sunshine_sensor; // ����̽ͷ�쳣
	u_int8_t high_temp;	// �¶ȹ����쳣
	u_int8_t lack_water;	// ȱˮ�쳣
	u_int8_t ph;	// ph�쳣
	u_int8_t humi;	// ʪ���쳣
} cl_drkzq_fault_t;

typedef struct {
	char name[64];
} cl_drkzq_name_item_t;

typedef struct {
	cl_drkzq_stat_t stat;
	cl_drkzq_fault_t fault;
	// Ŀǰһ����10���ط���Ҫ�������֣����Ϊ����Ĭ��
	cl_drkzq_name_item_t name[DRKZQ_NAME_MAX];
} cl_drkzq_info_t;

/*
	����: �Ծ��������ñ���
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������act_drkzq_xxx����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_drkzq_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

/*
	����: �Ծ���������ĳ��ģ�������
		
	�������:
		@dev_handle: �豸�ľ��
		@type: ����DRKZQ_NAME_XXX
		@name: ���֣�������ҪС��64�ֽ�
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_drkzq_set_name(cl_handle_t dev_handle, u_int8_t type, char *name);


/*
	����: �Ծ���������ĳ����ʱ��
		
	�������:
		@dev_handle: �豸�ľ��
		@id: Ŀǰֻ֧�ֶ�ʱ��1��2
		@valid:�������ǹرն�ʱ��
		@start_hour start_min: ��ʼ��Сʱ����
		@end_hour end_min: ������Сʱ����
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_drkzq_set_timer(cl_handle_t dev_handle, u_int8_t id, u_int8_t valid, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min);

#ifdef __cplusplus
}
#endif 


#endif



