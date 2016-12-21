#ifndef	__CL_YL_THERMOSTAT_H__
#define	__CL_YL_THERMOSTAT_H__

#ifdef __cplusplus
extern "C" {
#endif 

//�¶�����
#define YL_SET_TMP_MAX 	(30)
#define YL_SET_TMP_MIN 	(10)
#define YL_SET_TMP_DEF 	(23)

/* Type definitions. */
//����״̬
enum {
	YL_TS_OFF = 0X0,
	YL_TS_ON = 0X1,
};

//ģʽ
enum {
	YL_TS_MODE_FAN = 0x0,
	YL_TS_MODE_COOL = 0x1,
	YL_TS_MODE_HEAT = 0x2,
	YL_TS_MODE_MAX,
};

//��λ
enum{
	YL_TS_GEAR_AUTO = 0x0,
	YL_TS_GEAR_LOW = 0x1,
	YL_TS_GEAR_MIDDLE = 0x2,
	YL_TS_GEAR_HIGH = 0x3
};

//����ģʽ
enum {
	//�ֶ�ģʽ
	YL_TS_FUNC_MODE_HAND = 0X0,
	//���ģʽ
	YL_TS_FUNC_MODE_PROG = 0x1,
	//����ģʽ
	YL_TS_FUNC_MODE_COMF = 0X2,
	//����ģʽ
	YL_TS_FUNC_MODE_SAVE = 0X3,
	//���ģʽ
	YL_TS_FUNC_MODE_LEAVE = 0X4,
	//����ģʽ
	YL_TS_FUNC_MODE_SMART = 0X5,
	YL_TS_FUNC_MODE_MAX,
};

typedef struct ts_scene_s{
	u_int8_t set_tmp;
	u_int8_t gear;
}ts_scene_t;

typedef struct {
	u_int8_t onoff; //����
	u_int8_t work_mode; //����ģʽ
	u_int8_t gear;  //��λ
	u_int8_t temp; //�¶�
	u_int8_t room_temp; //����10-30�ȣ�Ĭ��23��
	u_int8_t cur_scene; //��ǰ����ģʽ
	ts_scene_t scene[YL_TS_FUNC_MODE_MAX*YL_TS_MODE_MAX]; 
} cl_yl_thermostat_info;


/*
	����:
		���ƿ���
	�������:
		@dev_handle: �豸�ľ��
		@is_on: 0-�ػ���1������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yl_thermostat_ctrl_onoff(cl_handle_t dev_handle, bool is_on);


/*
	����:
		���ƹ���ģʽ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yl_thermostat_ctrl_work_mode(cl_handle_t dev_handle, u_int8_t work_mode);

/*
	����:
		���Ƶ�λ����ͨ������Ч����work_mode����ֹ��չ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yl_thermostat_ctrl_gear(cl_handle_t dev_handle,u_int8_t work_mode, u_int8_t gear);

/*
	����:
		�����¶�
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yl_thermostat_ctrl_temp(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t temp);

/*
	����:
		�����л��龰
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yl_thermostat_ctrl_scene(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t scene);

/*
	����:
		���ø�ģʽ�µ��¶Ȳ���
	�������:
		@dev_handle: �豸�ľ��
		@temp_param: һά��ֵ��С: YL_TS_MODE_MAX*YL_TS_FUNC_MODE_MAX
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yl_thermostat_set_temp_param(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t scene_count,ts_scene_t* temp_param);
    
/////////////////////////////////////////////////////////////////
// ��ϣ���¿���

#define MAX_KXM_SUB_PUMP_NUM 8
#define MAX_KXM_TIMER_CNT 3
#define MAX_KXM_SUB_SYSTEM 4

#define KOCHEM_THERMOSTAT_COOLMODE_TEMPERATURE_MIN 5
#define KOCHEM_THERMOSTAT_COOLMODE_TEMPERATURE_MAX 35
#define KOCHEM_THERMOSTAT_HEATMODE_TEMPERATURE_MIN 5
#define KOCHEM_THERMOSTAT_HEATEMODE_TEMPERATURE_MAX 35
#define KOCHEM_THERMOSTAT_ENERGY_COOL_TEMPERATURE 28
#define KOCHEM_THERMOSTAT_ENERGY_HOT_TEMPERATURE 16
    
#define KOCHEM_DEVICE_COOLMODE_TEMPERATURE_MIN 10
#define KOCHEM_DEVICE_COOLMODE_TEMPERATURE_MAX 30
#define KOCHEM_DEVICE_HEATMODE_TEMPERATURE_MIN 20
#define KOCHEM_DEVICE_HEATMODE_TEMPERATURE_MAX 55
    
typedef struct {
    u_int8_t scoll_temp; //�̹��¶�
    u_int8_t inhale_temp; //�����¶�
    u_int8_t exhaust_temp; //�����¶�
    u_int8_t exv_value; //EXV����
    u_int8_t compress_stat; //ѹ����״̬
    u_int8_t spray_stat; //���ܷ�״̬
    u_int8_t is_low_press_fault; //��ѹ����
    u_int8_t is_high_press_fault; //��ѹ����
    u_int8_t is_over_curr_fault; //��������
    u_int8_t is_exhault_fault; //��������
    u_int8_t pad[2];
}cl_kxm_sub_system_stat_t;
    
//����״̬
typedef struct {
    u_int8_t is_online; //�Ƿ����
    u_int8_t machine_type; //�����ͺ� 0����ˮ 1�� �յ�
    u_int8_t back_water_temp; //��ˮ�¶�
    u_int8_t water_temp; //��ˮ�¶�
    u_int8_t env_temp; // �����¶�
    u_int8_t water_box_temp; // ˮ���¶�
    u_int8_t in_water_temp; // ��ˮ�¶�
    u_int8_t out_water_temp; // ��ˮ�¶�
    u_int8_t run_stat;// ����״̬ 0����;1����;2����;3��˪;4����
    u_int8_t water_pos;//ˮλ״̬ �ٷֱ�
    u_int8_t is_fan_high; // �����״̬ 0���ر�  1������
    u_int8_t is_fan_low; // �����״̬ 0���ر�  1������
    u_int8_t cir_pump_run; // ѭ����״̬ 0���ر�  1������
    u_int8_t back_pump_run; // ��ˮ��״̬ 0���ر�  1������
    u_int8_t in_water_pump_run; // ��ˮ��״̬ 0���ر�  1������
    u_int8_t water_pump_run; // ��ˮ��״̬ 0���ر�  1������
    u_int8_t is_elec_hot_run; // �����״̬ 0���ر�  1������
    u_int8_t sw_dir_tap_run; // ����״̬ 0���ر�  1������
//    -------------------------------------------------
    u_int8_t sensor_back_water_fault; //��ˮ����������
    u_int8_t sensor_water_fault; //��ˮ����������
    u_int8_t sensor_env_fault; //�����¶ȴ���������
    u_int8_t sensor_water_box_fault; //ˮ�䴫��������
    u_int8_t sensor_in_water_fault; //��ˮ����������
    u_int8_t sensor_out_water_fault; //��ˮ����������
    u_int8_t is_out_water_temp_low_fault; //��ˮ�¶ȵ͹���
    u_int8_t is_out_water_temp_high_fault; //��ˮ�¶ȸ߹���
    u_int8_t is_in_out_temp_big_fault; //����ˮ�²�����
    u_int8_t is_anti_phase_fault; //�������
    u_int8_t is_no_phase_L2_fault; //ȱ��L2����
    u_int8_t is_no_phase_L3_fault; // ȱ��L3����
    u_int8_t is_ctrl_comu_fault; //������ͨ�Ź���
    u_int8_t pad;
    cl_kxm_sub_system_stat_t sub_system_info[MAX_KXM_SUB_SYSTEM];
}cl_kxm_pump_stat_info_t;

//��ʱ��
typedef struct {
    u_int8_t on_hour;
    u_int8_t on_min;
    u_int8_t off_hour;
    u_int8_t off_min;
}cl_kxm_timer_info_t;

//ģʽ
enum{
    KXM_MODE_HOT_WATER,
    KXM_MODE_HOT,
    KXM_MODE_COLD
};
    
//�߿���(�յ���/��ˮ��)������Ϣ��״̬
typedef struct {
    u_int8_t on_off;
    u_int8_t mode; // 0:��ˮ 1������ 2�� ����
    u_int8_t hot_water_setting_temp; //��ˮ�����¶� 20-60
    u_int8_t hot_setting_temp; //�����趨�¶� 20-60
    u_int8_t cold_setting_temp; // �����趨�¶� 10-30
    u_int8_t t_hour; //�豸�ϵ�ʱ�ӣ�Сʱ
    u_int8_t t_min; //�豸�ϵ�ʱ��: ����
    u_int8_t pad;
    cl_kxm_timer_info_t timer[MAX_KXM_TIMER_CNT];
}cl_kxm_host_info_t;
    
typedef struct {
    u_int8_t is_data_valid;
    u_int8_t has_receive_data;
    u_int8_t pad[2];
    cl_kxm_pump_stat_info_t pump[MAX_KXM_SUB_PUMP_NUM];
    cl_kxm_host_info_t hinfo;
    
    u_int8_t sdk_priv_data[128]; //SDKʹ�ã�APP��Ҫ�������
}cl_kxm_info_t;

/*
 ���ܣ�
	 �����߿�������
 �������:
	 @handle: �豸���
 �������:
	 ��
 ���أ�
	 RS_OK:
	 ����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_kxm_ctrl_host_onoff(cl_handle_t dev_handle,u_int8_t on_off);
    
/*
 ���ܣ�
	 �����߿�����ʱ��
 �������:
	 @handle: �豸���
 	@timer_index: 1-3
 �������:
	 ��
 ���أ�
	 RS_OK:
	 ����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_kxm_ctrl_timer(cl_handle_t dev_handle,u_int8_t timer_index,cl_kxm_timer_info_t* timer);


/*
 ���ܣ�
	 �����߿���ģʽ���¶�
 �������:
	 @handle: �豸���
 �������:
	 ��
 ���أ�
	 RS_OK:
	 ����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_kxm_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode,u_int8_t temp);
    
/*
 ���ܣ�
	 �����߿���ģʽ���¶�
 �������:
	 @handle: �豸���
 �������:
	 ��
 ���أ�
	 RS_OK:
	 ����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_kxm_set_dev_time(cl_handle_t dev_handle,u_int8_t min,u_int8_t sec);
    

/*
 ���ܣ�
 	�����߿�����ʱ��
 �������:
	 @handle: �豸���
 �������:
	 ��
 ���أ�
	 RS_OK:
 	����: ʧ��
	 �¼�֪ͨ:
 
 */
CLIB_API RS cl_kxm_ctrl_all_timer(cl_handle_t dev_handle,cl_kxm_timer_info_t timer[MAX_KXM_TIMER_CNT]);

/////////////////////////////////////////////////////////////////////
//��ϣ���¿���
    
enum{
    KXM_WM_UNKNOWN = 0x00,
    KXM_WM_COLD, //����ģʽ
    KXM_WM_FAN_HOT, //��������
    KXM_WM_WATER_HOT, // ˮů����
    KXM_WM_FAN_WATER_HOT //���̺�ˮů����
};
    
enum{
    KXM_FS_UNKNOWN,
    KXM_FS_LOW, //�ͷ�
    KXM_FS_MIDDLE, //�з�
    KXM_FS_HIGH, //�߷�
    KXM_FS_AUTO //�Զ���
};
    
typedef struct {
    u_int8_t onoff; //����״̬
    u_int8_t mode; //����ģʽ
    u_int8_t setting_temp; //�¶�
    int8_t room_temp; //����
    u_int8_t fan_speed; //����
    u_int8_t energy_cons;//����
}cl_kxm_thermost_info_t;

enum{
    KXM_CA_NONE = 0,
    KXM_CA_ONOFF, //���ƿ���
    KXM_CA_MODE, //����ģʽ
    KXM_CA_TEMP, //�����¶�
    KXM_CA_FS, //���Ʒ���
    KXM_CA_EC //���ƽ���
};
    
/*
 ���ܣ�
 	���ƿ�ϣ���¿���
 �������:
 	@handle: �豸���
 �������:
 	��
 ���أ�
 	RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_kxm_ther_ctrl(cl_handle_t dev_handle,u_int8_t action,u_int8_t value);

/////////////////////////////////////////////////////////////////////////////
//˼�����¿���

//״̬����
typedef struct {
    u_int8_t onoff; //���ػ�
    u_int8_t temp; //�¶� 10-32
    u_int8_t mode; //����ģʽ 00����ģʽ��01����ģʽ��2ͨ��ģʽ�� 3 ����ģʽ
    u_int8_t fan_speed; //���� 0���Զ���1���͵���2---�е��� 3���ߵ���
}cl_sbt_work_ctrl_t;

//�߼���������
typedef struct {
    u_int8_t auto_mode; //�Զ�ģʽ״̬ 0�رգ�1ִ���Զ�ģʽ
    int8_t temp_adjust; //�¶Ȳ��� -5��5
    u_int8_t low_temp; //���±����¶� 3-9
    u_int8_t valve_mode; //(�������)����ģʽ���� 1�����ط����ͣ0�����ط��ͣ
    u_int8_t return_temp; //�ز��¶� 1~31
    u_int8_t is_low_temp_guard; //���±���ʹ��
    u_int8_t max_temp; //��������¶� 10-32
    u_int8_t min_temp; //��������¶� 10-32
}cl_sbt_func_setting_t;

//ʱ��У��
typedef struct {
    u_int8_t scm_hour; //��Ƭ��ʱ��,ʱ
    u_int8_t scm_min; //��Ƭ��ʱ��,��
    u_int8_t scm_sec; //��Ƭ��ʱ��,��
    u_int8_t scm_weekday; //��Ƭ��ʱ��,����
}cl_sbt_time_adjust_t;

#pragma pack(push,1)
    
// ����ģʽ�������
typedef struct {
    u_int8_t hour;
    u_int8_t temp;
}cl_sbt_smart_item_t;
    
typedef struct {
    cl_sbt_smart_item_t si[4];
}cl_sbt_smart_day_t;
    
typedef struct {
    cl_sbt_smart_day_t work_day;
    cl_sbt_smart_day_t sat_day;
    cl_sbt_smart_day_t sun_day;
}cl_smart_smart_ctrl_t;

#pragma pack(pop)
    
typedef struct {
    u_int8_t onoff; //���ػ�
    u_int8_t temp; //�¶� 10-32
    u_int8_t mode; //����ģʽ 00����ģʽ��01����ģʽ��2ͨ��ģʽ�� 3 ����ģʽ
    u_int8_t fan_speed; //���� 0���Զ���1���͵���2---�е��� 3���ߵ���
    //----------------------//
    u_int8_t auto_mode; //�Զ�ģʽ״̬ 0�رգ�1ִ���Զ�ģʽ
    int8_t temp_adjust; //�¶Ȳ��� -5��5
    u_int8_t low_temp; //���±����¶� 3-9
    u_int8_t valve_mode; //(�������)����ģʽ���� 1�����ط����ͣ0�����ط��ͣ
    u_int8_t return_temp; //�ز��¶� 1~31
    u_int8_t is_low_temp_guard; //���±���ʹ��
    u_int8_t max_temp; //��������¶� 10-32
    u_int8_t min_temp; //��������¶� 10-32
    /*�����������APP������ʱ������,jni���Բ��ϴ�*/
    u_int8_t lock_screen; //�Ƿ�������
    u_int8_t sesor_type; //�¶ȴ���������
    u_int8_t broad_cast_type; //�㲥����
    u_int8_t no_paid_mode; //Ƿ��ģʽ
    u_int8_t max_sesor_temp; //�ⲿ�����������¶�
    u_int8_t pad;
    //�����Ĳ�����//
    int16_t room_temp; //����*10��ʹ�õ�ʱ����Ҫ����10
    u_int8_t scm_hour; //��Ƭ��ʱ��,ʱ
    u_int8_t scm_min; //��Ƭ��ʱ��,��
    u_int8_t scm_sec; //��Ƭ��ʱ��,��
    u_int8_t scm_weekday; //��Ƭ��ʱ��,����
    cl_smart_smart_ctrl_t smart_info; //����ģʽ��ǰ��Ϣ
    u_int8_t is_data_valid;//�����Ƿ���Ч
    u_int8_t pad2[3];
}cl_sbt_ther_info_t;
    
/*
 ���ܣ�
	 ˼�����¿���״̬����
 �������:
	 @handle: �豸���
 �������:
 	��
 ���أ�
	 RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_sbt_ther_ctrl_stat(cl_handle_t dev_handle,cl_sbt_work_ctrl_t* wc);
    

/*
 ���ܣ�
 	˼�����¿�����������
 �������:
 	@handle: �豸���
 �������:
 ��
 ���أ�
 RS_OK:
 ����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_sbt_ther_set_param(cl_handle_t dev_handle,cl_sbt_func_setting_t* fs);
    
/*
 ���ܣ�
 	˼�����¿���У��ʱ��
 �������:
 	@handle: �豸���
 �������:
	 ��
 ���أ�
 	RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_sbt_ther_set_ajust_scm_time(cl_handle_t dev_handle,cl_sbt_time_adjust_t* st);

/*
 ���ܣ�
 	˼�����¿�����������ģʽ������
 �������:
 	@handle: �豸���
 �������:
 	��
 ���أ�
 	RS_OK:
    ����: ʧ��
 	�¼�֪ͨ:
 
 */
CLIB_API RS cl_sbt_ther_set_smart_config(cl_handle_t dev_handle,cl_smart_smart_ctrl_t* ss);
    

/*
 ���ܣ�
 	˼�����¿������ú�ͣ������ģʽ
 �������:
 	@handle: �豸���
 �������:
 	��
 ���أ�
 	RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_sbt_ther_set_smart_config_mode(cl_handle_t dev_handle,u_int8_t onoff);


    
#ifdef __cplusplus
}
#endif 


#endif

