#ifndef	__CL_AMT_DEVICE_H__
#define	__CL_AMT_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_USER_DEFINE_MODE_POINT 7

enum{
	AMT_FAN_STAND = 0x0, //��׼��
	AMT_FAN_NATURAL = 0x1,//��Ȼ��
	AMT_FAN_SLEEP = 0x2,//˯�߷�
	AMT_FAN_SMART = 0x3,//���ܷ�
	AMT_FAN_UD_NATURAL = 0x9//�Զ�����Ȼ��
};

#define AMT_MAX_U_GEAR 32
#define AMT_MIN_GEAR 1

typedef struct{
	u_int8_t onoff; //����
	u_int8_t cur_mode; //��ǰģʽ
	u_int8_t cur_gear; //��ǰ��λ
	u_int8_t screen_light; //����
	u_int8_t is_shake; //�Ƿ���ҡͷ�׶�
	int8_t cur_temp;//��ǰ�¶�
	u_int8_t is_anion_on;//�������Ƿ���
	u_int8_t is_plasma_on;
	u_int8_t cur_user_define_mode[MAX_USER_DEFINE_MODE_POINT]; //�û��Զ���ģʽ�ĵ�λ
	u_int8_t is_timer_on_valid; //ԤԼ�����Ƿ���Ч
	u_int8_t is_timer_off_valid; //��ʱ�ػ��Ƿ���Ч
	u_int8_t cur_power; //��ǰ����
	u_int32_t dev_on_remain_time;//ԤԼ����ʣ��ʱ��
	u_int32_t dev_off_remain_time; // ��ʱ�ػ�ʣ��ʱ��
}cl_amt_fan_device_t;

#ifdef __cplusplus
}
#endif 

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
CLIB_API RS cl_amt_ctrl_on_off(cl_handle_t dev_handle,u_int8_t is_on);

/*
	����:
		�л�ģʽ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode);

/*
	����:
		�л���λ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_gear(cl_handle_t dev_handle,u_int8_t mode,u_int8_t gear);

/*
	����:
		����ҡͷ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_shake(cl_handle_t dev_handle,u_int8_t is_shake);

/*
	����:
		���Ƹ�����
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_anion(cl_handle_t dev_handle,u_int8_t is_on);

/*
	����:
		���Ƶ�����
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_plasma(cl_handle_t dev_handle,u_int8_t is_on);

/*
	����:
		��������
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_screen_light(cl_handle_t dev_handle,u_int8_t is_on);

/*
	����:
		�����û��Զ���ģʽ����
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_config_fan_user_mode(cl_handle_t dev_handle,u_int8_t time_interval,u_int8_t gear_num,u_int8_t* gears);

/*
	����:
		��ʱ�ػ���ԤԼ����
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_ctrl_dev_onoff_timer(cl_handle_t dev_handle,u_int8_t onoff,u_int16_t time);

/*
	����:
		��������ģʽ����
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_amt_config_smart_param(cl_handle_t dev_handle,u_int8_t temp,u_int8_t gear);


#endif

