#ifndef	__CL_CHIFFO_H__
#define	__CL_CHIFFO_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define CHIFFO_TIMER_DAYS_CNT 7
#define CHIFFO_TIMER_SECTION_PER_DAY 8

typedef enum {
	SCENE_AUTO = 1,	// �Զ�ģʽ
	SCENE_BATH,// ԡ��ģʽ
	SCENE_DISHES,// ϴ��ģʽ
	SCENE_VEGETABLES,// ϴ��ģʽ
	SCENE_CLOTHES,	// ϴ��ģʽ
} CHIFFO_SCENE_T;

typedef struct{
	u_int8_t is_enable;
	u_int8_t temp;
	u_int8_t pad[2];
}cl_chiffo_timer_item_t;

typedef struct{
	cl_chiffo_timer_item_t items[CHIFFO_TIMER_SECTION_PER_DAY];
}cl_chiffo_one_day_timer_t;

typedef struct{
	u_int8_t is_on; //����
    u_int8_t fault_code;//���ϱ���
	u_int8_t is_fan_working; //����Ƿ���
	u_int8_t is_fire_working;//ȼ���Ƿ���
	u_int8_t is_floor_heat_working;//���ȹ���
	u_int8_t is_radiator_working;//ɢ�����Ƿ���
	u_int8_t is_pump_working;//ˮ���Ƿ���
	u_int8_t is_water_working;//ˮ���Ƿ���
	u_int8_t next_heat_time; //�¸�����ʱ��
	u_int8_t is_water_mode_on; //��ˮģʽ�Ƿ���
	u_int8_t is_heater_mode_on; //��ůģʽ�Ƿ���
	//��ˮģʽ����
	u_int8_t water_setting_temp; //��ˮ�����¶�
	u_int8_t water_current_temp; //��ˮ��ǰ�¶�
	u_int32_t cur_water_pressure; //��ǰˮѹ
	//��ůģʽ����
	u_int8_t heater_setting_temp; //��ů�����¶�
	u_int8_t heater_current_temp; //��ů��ǰ�¶�
	cl_chiffo_one_day_timer_t timer_info[CHIFFO_TIMER_DAYS_CNT];

	// �龰 �Զ�ԡ��ϴ��ϴ�ˡ���
	CHIFFO_SCENE_T scene;
	// ѭ��
	u_int8_t loop_type;	// 0 ѭ���ر�1 ��ѭ�� 2 ����ѭ��

	// ���������ź�ǿ�ȣ���Ϊ1-4��
	u_int8_t rssi;

	// �豸ʱ��
	u_int8_t hour;
	u_int8_t min;

	// ԡ��ģʽ��Ӧ��ˮ������λL
	u_int16_t water_capacity;
	u_int8_t need_show_water_capacity;	// �Ƿ���ʾˮ������
}cl_chiffo_floor_heater_info_t;


/*
	����:
		���ƿ��ػ�
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_ctrl_on_off(cl_handle_t dev_handle,u_int8_t is_on);

/*
	����:
		������ˮģʽ�¶�
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_setting_water_mode_temp(cl_handle_t dev_handle,u_int8_t temp);

/*
	����:
 �������ӻ������ˮģʽ�¶�
	�������:
 @dev_handle: �豸�ľ��
	�������:
 ��
	����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_chiffo_add_dec_water_mode_temp(cl_handle_t dev_handle,bool is_add);

/*
	����:
 �������ӻ��������ģʽ�¶�
	�������:
 @dev_handle: �豸�ľ��
	�������:
 ��
	����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_chiffo_add_dec_heater_mode_temp(cl_handle_t dev_handle,bool is_add);

/*
	����:
		������ˮģʽ�¶�
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_setting_heater_mode_temp(cl_handle_t dev_handle,u_int8_t temp);


/*
	����:
		����ģʽ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_setting_mode(cl_handle_t dev_handle,u_int8_t water_on_off,u_int8_t heater_on_off);


/*
	����:
		���ƶ�ʱ��
	�������:
		@dev_handle: �豸�ľ��
		@day_index: 1-7 ����1-7
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_setting_timer(cl_handle_t dev_handle,cl_chiffo_one_day_timer_t* day_info,u_int8_t day_index);

/*
	����:
		ˢ��ĳ��Ķ�ʱ��
		
	�������:
		@dev_handle: �豸�ľ��
		day_index 1-7 ������1-7
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_refresh_timer_by_day(cl_handle_t dev_handle,u_int8_t day_index);

/*
	����:
		���ó���
		
	�������:
		@dev_handle: �豸�ľ��
		@scene: �ο�CHIFFO_SCENE_T ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_set_scene(cl_handle_t dev_handle, u_int8_t scene);


/*
	����:
		����ѭ����ʽ
		
	�������:
		@dev_handle: �豸�ľ��
		@loop_type: ��ʽ0 ѭ���ر�1 ��ѭ�� 2 ����ѭ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/

CLIB_API RS cl_chiffo_set_loop(cl_handle_t dev_handle,u_int8_t loop_type);

/*
	����:
		����ǰ��ϵͳ��ʱ��
		
	�������:
		@dev_handle: �豸�ľ��
		
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_set_clock(cl_handle_t dev_handle);

/*
	����:
		�Ӽ�ǰ��ˮ��
		
	�������:
		@dev_handle: �豸�ľ��
		@is_add: 1Ϊ�� 0 Ϊ��
		
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_chiffo_add_dec_water_capacity(cl_handle_t dev_handle,bool is_add);

/*
	����:
		����ǰ���ˮ��
		
	�������:
		@dev_handle: �豸�ľ��
		@capacity: ˮ������λ10L
		
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/

CLIB_API RS cl_chiffo_setting_water_capacity_temp(cl_handle_t dev_handle,u_int16_t capacity);


#ifdef __cplusplus
}
#endif


#endif

