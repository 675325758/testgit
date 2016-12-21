#ifndef CL_ZKCLEANNER_H
#define CL_ZKCLEANNER_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

// cl_zkcleanner_ctrl��action 
enum {
	ACT_ZKCLEANNER_ONOFF = 1,// ���ػ�1 �� 0 �ر�
	ACT_ZKCLEANNER_MODE,// ģʽ 1���Զ���2���·磻3��ѭ��?
	ACT_ZKCLEANNER_WIND,// ������λ1�����٣�2�����٣�3�����٣�
	ACT_ZKCLEANNER_ANTIBIOSIS,// ����0���رգ�1���򿪣�
	ACT_ZKCLEANNER_FRESH, // ����0���رգ�1���򿪣�
	ACT_ZKCLEANNER_MAINTAIN,// ������־0���ޣ�1�����������־��
	ACT_ZKCLEANNER_ONTIME,// ����ʱ��(���޸ģ�value ��16λ��ʾ����ʱ�䣬��16λ�����Ϊ0����ʾ�ػ�ʱ��)
	ACT_ZKCLEANNER_OFFTIME,// �ػ�ʱ��
	/*
		 ��ѯһ��������ݣ���ѯ�ɹ����յ��¼�SAE_ZKCLEANNER_DAY_DATA
		 Ȼ���ȡcl_zkcleanner_info_t����day_data����
	*/
	ACT_ZKCLEANNER_QUERY_ONE_DAY_DATA, 
	/*
		 ��ѯ���һ�²������ݣ���ѯ�ɹ����յ��¼�SAE_ZKCLEANNER_MONTH_DATA
		 Ȼ���ȡcl_zkcleanner_info_t����month_data����
	*/
	//ACT_ZKCLEANNER_QUERY_ONE_MONTH_DATA, 
};

typedef struct {
	u_int8_t valid;	// ���������Ƿ���Ч
	u_int8_t onoff;	// ���ػ�1 �� 0 �ر�
	u_int8_t mode;	// ģʽ 1���Զ���2���·磻3��ѭ����
	u_int8_t wind;	// ������λ1�����٣�2�����٣�3�����٣�
	u_int8_t antibiosis;	// ����0���رգ�1���򿪣�
	u_int8_t fresh;	// ����0���رգ�1���򿪣�
	u_int8_t maintain; // ������־0���ޣ�1�����������־��
	u_int16_t ontime;	// ����ʱ��
	u_int16_t offtime;	// �ػ�ʱ��

	u_int8_t type;	// �ͺ�
	u_int8_t temp;	// ��ǰ�¶�
	u_int16_t pm25; // ��ǰPM2.5  ��λ1ug/m3
	u_int16_t co2;	// ��ǰCO2Ũ�ȣ���λ1ppm
	u_int16_t hcho;	// ��ǰ��ȩŨ�ȵ�λ1ugm3
	u_int16_t voc;	// ��ǰVOCŨ�ȣ���λ1ppm
	u_int8_t aqi;	// ��������1���ţ�2������3���
	u_int16_t uptime;	// �豸����ʱ�䵥λСʱ0-1440

	// �¼ӵı�־
	u_int8_t on_timer_changed;	// ���� ��ʱ���Ƿ�ı�
	u_int8_t off_timer_changed;	// �ػ���ʱ���Ƿ�ı�
	u_int8_t ver;	// �汾�ţ�������1
} cl_zkcleanner_stat_t;


// ��������� PM2.5��CO2
typedef struct {
	u_int16_t pm25;
	u_int16_t co2;
} cl_zkcleanner_date_t;

typedef struct {
	u_int32_t time;	// �õ������ݵ�ʱ��� UTC
	cl_zkcleanner_date_t items[24];
} cl_zkcleanner_day_data_t;

typedef struct {
	u_int32_t time; // �õ������ݵ�ʱ��� UTC
	cl_zkcleanner_date_t items[31];
} cl_zkcleanner_month_data_t;


typedef struct {
	cl_zkcleanner_stat_t stat;

	/*
		һ��24Сʱ�����ݣ�����м���ͣ�磬����û�вɼ������ݶ���д0��
	*/
	cl_zkcleanner_day_data_t day_data;

	/*
		��¼һ���µ����ݣ��ӵ�����ǰ��¼�����û�м�¼���㡣
		�磺��һ����11��15�յģ��ڶ�������11��14�����ݣ�
	*/
	cl_zkcleanner_month_data_t month_data;
} cl_zkcleanner_info_t;

/*
	����: �Ծ��������ñ���
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������ACT_ZKCLEANNER_XXX����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_zkcleanner_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

#ifdef __cplusplus
}
#endif 


#endif


