#ifndef	__CL_TL_TEMP_H__
#define	__CL_TL_TEMP_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum{
	MODE_TL_FAN = 0x0,
	MODE_TL_AUTO,
	MODE_TL_COOL,
	MODE_TL_HOT
};

enum{
	FAN_TL_AUTO = 0x0,
	FAN_TL_LOW,
	FAN_TL_MID,
	FAN_TL_HIGH
};

typedef struct{
	/* ���أ��ء��� */
	u_int8_t onoff;
	/* ����ģʽ��ͨ�硢�Զ������䡢���� */
	u_int8_t mode;
	/* �����趨���͡��С��� */
	u_int8_t speed;
	/* �¶��趨��5 - 35 */
	u_int8_t temp;
	/* �����¶� */
	u_int16_t room_temp;
	/*״̬��־��APP ����ɲ���ע*/
	u_int16_t state;
	/*����״̬*/
	u_int8_t valve_stat;
	//���䷧״̬
	u_int8_t cool_valve_stat;
	//���ȷ���״̬
	u_int8_t hot_valve_stat;
	/*�Ʒ�״̬*/
	u_int8_t charge_enable;
}cl_tl_ctrl_stat_t;

typedef struct  {
	/* λ���� */
	u_int16_t lock_flags;
	/* �¶ȴ����趨 */
	u_int16_t temp_bandwidth;
	/* �͵��Ʒ����� */
	u_int16_t charge_factor_low;
	/* �е��Ʒ����� */
	u_int16_t charge_factor_mid;
	/* ����Ƿ��ܿ� */
	u_int8_t fan_under_control;
	/* ��ʱ���� */
	u_int8_t time_on;
	/* ECO���� */
	u_int8_t eco_mode;
	/* ��ѧϰ */
	u_int8_t self_learn;
    /*��ǰ�������ĸ�ʱ��*/
    u_int8_t run_timer;
}cl_tl_adv_info_t;

typedef struct{
	/* �ۼ�����ߵ�ʱ�� */
	u_int32_t high_gear_time_cal;
	/* �ۼƵ͵�ʱ�� */
	u_int32_t low_gear_time;
	/* �ۼ��е�ʱ�� */
	u_int32_t mid_gear_time;
	/* �ۼƸߵ�ʱ�� */
	u_int32_t high_gear_time;
}cl_tl_total_time_t;

typedef struct{
	u_int16_t temp;
	u_int16_t hour;
	u_int16_t min;
}cl_tl_timer_item_t;
    
typedef struct {
    u_int8_t is_data_valid; //�����Ƿ���Ч
    u_int8_t is_auto_sync; // �Ƿ��Զ�ͬ��
    u_int8_t pad[2];
    u_int32_t dev_time; //�豸ʱ�� UTC
    u_int32_t local_update_time; //���ظ���ʱ��,UTC
}cl_tl_time_sync_t;

#define TL_TIME_CNT_PER_DAY 4

typedef struct{
	cl_tl_timer_item_t time[TL_TIME_CNT_PER_DAY];
}cl_tl_timer_info_t;

typedef struct{
	cl_tl_ctrl_stat_t ctrl_stat;
	cl_tl_adv_info_t adv_info;
	cl_tl_total_time_t total_info;
	cl_tl_timer_info_t time_info;
    cl_tl_time_sync_t dev_time_sync_info;
}cl_tl_info_t;
    
enum{
    TLH_M_HAND = 0, //���ֲ�ů���ֹ�����ģʽ
    TLH_M_PROGRAM//���ֲ�ů����̿���ģʽ
};

/*
	����:
		���ÿ���
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_ctrl_off(cl_handle_t dev_handle,bool onoff);

/*
	����:
		����ģʽ
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode);


/*
	����:
		���÷���
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_ctrl_fan_speed(cl_handle_t dev_handle,u_int8_t fan_speed);

/*
	����:
		�����¶�
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_ctrl_temp(cl_handle_t dev_handle,u_int8_t temp);


/*
	����:
		����ECO ģʽ
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_ctrl_eco(cl_handle_t dev_handle,bool is_enable);

/*
	����:
		����learn ����
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_ctrl_learn(cl_handle_t dev_handle,bool is_enable);


/*
	����:
		����ʱ�β���
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tl_setting_timer(cl_handle_t dev_handle,cl_tl_timer_info_t* timer);

/*
 ����:
    �����Զ�ͬ��ʱ��
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_tl_ctrl_clock_auto_sync(cl_handle_t dev_handle,bool is_auto_sync);

/*
 ����:
    �ֶ�ͬ��ʱ��
 �������:
    @dev_handle: �豸���
 �������:
    ��
����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_tl_ctrl_clock_sync(cl_handle_t dev_handle);
    
/*
 ����:
    ˢ���豸��ǰʱ��
 �������:
    @dev_handle: �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_tl_refresh_dev_time(cl_handle_t dev_handle);


#ifdef __cplusplus
}
#endif 


#endif

