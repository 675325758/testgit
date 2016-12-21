#ifndef	__AMT_SCM_CTRL_H__
#define	__AMT_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "cl_amt_device.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

// �����ز�Ʒ����
enum {
	//�����ŷ���
	AMT_PRODUCT_TYPE_VENTILATIN_FAN = 0x9,
	//������ʪ��
	AMT_PRODUCT_TYPE_HUMIDIFIER = 0xA,
	//������ʪ��
	AMT_PRODUCT_TYPE_DEHUMIDIFIER = 0xB,
	// ���������
	AMT_PRODUCT_TYPE_FAN = 0xE
};

//������������
#define AMT_DATA_TYPE_MASK	(0x0F)

//-------------------------------------------------------
//�豸״̬ʮ�ֱ仯����
#define AMT_DATA_CHANGE_MASK (0xF0)
#define AMT_CTRL_CMD_MASK (0xA0)

enum{
	//��Ʒ����
	AMT_DATA_TYPE_PRODUCT_CATEGORY = 0x1,
	//��Ʒ�ͺ�
	AMT_DATA_TYPE_PRODUCT_MODEL = 0x2,
	//���ػ�״̬
	AMT_DATA_TYPE_STATUS = 0x3,
	//ģʽ�͵�λ
	AMT_DATA_TYPE_GEAR_INFO = 0x4,
	//��ʱ��
	AMT_DATA_TYPE_TIMER = 0x5,
	//ҡͷ
	AMT_DATA_TYPE_SHAKE = 0x6,
	//����������Ϣ
	AMT_DATA_TYPE_OTHER_INFO = 0x7,
	//������Ϣ
	AMT_DATA_TYPE_ENV = 0x8 ,
	//�豸������������
	AMT_DATA_TYPE_DEV_RUN_INFO = 0x9,
	//�ĵ���Ϣ
	AMT_DATA_TYPE_ELEC_STAT = 0xA
};

enum{
	// �ƶ˿��ƿ��ؼ���ʾ״̬
	AMT_CTRL_CODE_ONOFF = 0xA2,
	// �ƶ˿��Ʋ�Ʒģʽ�͵�λ
	AMT_CTRL_CODE_GEAR = 0xA3, 
	// �ƶ˿��ƶ�ʱ��
	AMT_CTRL_CODE_TIMER = 0xA4,
	// �ƶ˿���ҡͷ
	AMT_CTRL_CODE_SHAKE = 0xA5,
	// �ƶ˿���������Ϣ
	AMT_CTRL_CODE_OTHER = 0xA6,
	//�ƶ˿��ƻ�����Ϣ
	AMT_CTRL_CODE_ENV = 0xA7,
	//�ƶ˿����龰
	AMT_CTRL_CODE_SCENE = 0xAB
};

//���ݱ仯�������
#define AMT_CHANGE_DATA 		(0x5)
//��ʱ���͸�wifiģ�������
#define AMT_TIMER_DATA 			(0xD)

#define AMT_SMT_ERROR			(0xE1)

//����Щ����
enum{
	AMT_KEY_NO = 0x0, //û�п��ؼ�
	AMT_KEY_ONLY_OFF = 0x1, //ֻ�йذ���
	AMT_KEY_ONLY_ON = 0x2, //ֻ�п�����
	AMT_KEY_ON_OFF = 0x3,//�����Ŀ��͹�
	AMT_KEY_ONOFF = 0x4, //���ذ�����һ  
	AMT_KEY_GEARON_OFF = 0x5, // "��/��λ"��"��"��������
	AMT_KEY_GEARONOFF = 0x6 // ֻ��"��/��λ/��"һ������
};

#define AMT_FUNC_NO (0x0)
#define AMT_FUNC_YES (0x3)

#define AMT_FAN_USER_DEF_MASK 0x8

#define AMT_FIX_START_CODE 0xAAAA
#define AMT_FIX_END_CODE 0x55

#define AMT_DATA_SRC_MASK 0xF0
#define DEV_NO_CHANGE_TO_PHONE 0xD0
#define DEV_HAS_CHANGE_TO_PHONE 0x50
#define DEV_PHONE_CTRL_RESPONSE 0xA0


//��ʱ����
enum{
	AMT_TIME_TYPE_DAY, //��
	AMT_TIME_TYPE_HOUR,//Сʱ
	AMT_TIME_TYPE_MIN,//��
	AMT_TIME_TYPE_SECOND //��
};

enum{
	AMT_ENV_TEMP = 0x0,
	AMT_P1_TEMP,
	AMT_P2_TEMP,
	AMT_P3_TEMP
};

enum{
	AMT_TIMER_DT_REMAIN = 0x0,
	AMT_TIMER_DT_OFF_SETTING,
	AMT_TIMER_DT_ON_SETTING
};

enum{
	AMT_TIMER_MODE_NO,
	AMT_TIMER_MODE_OFF,
	AMT_TIMER_MODE_ON,
	AMT_TIMER_MODE_ON_OFF
};

#define AMT_A10_CUR_POWER 0xFF

enum{
	AMT_A7_DB_ANION = 0, //������
	AMT_A7_DB_PLASMA //������
};

/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	u_int16_t fix_s_code;
	u_int8_t len;
	u_int8_t product_type;
	u_int8_t data_type;
}amt_pkt_header_t;

//���ػ������ԣ�ָ��53
typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN  
	u_int8_t is_on:1,
		on_off_key:3, 
		is_screen_display:2, // 0x00,�أ�0x11 ��
		has_screen:2; // 0x00,�ޣ�0x11 ��
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t has_screen:2,
		is_screen_display:2, // 0x00,�أ�0x11 ��
		on_off_key:3, 
		is_on:1;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_53h_info_t;

//��Ʒģʽ�͵�λ,0x54
typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t cur_mode:4, //�����������ĸ�ģʽ�� 
		total_mode:4;//�ܹ�֧�ֶ�����ģʽ
		
	u_int8_t max_gear:6, //��Ʒ���ٷ�ʽ
		speed_mode:2; // ��Ʒ���λ
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t total_mode:4,//�ܹ�֧�ֶ�����ģʽ
		cur_mode:4; //�����������ĸ�ģʽ��
	u_int8_t speed_mode:2, //��Ʒ���ٷ�ʽ
		max_gear:6; // ��Ʒ���λ
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t cur_gear; // ��Ʒ��ǰ������λ
}amt_54h_info_t;

//�û��Զ���ģʽ

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t group:4, //���
		index:4; //����index
	u_int8_t total_mode:4,//�ܹ�֧�ֶ�����ģʽ
		config_mode:4; //�����������ĸ�ģʽ��
	u_int8_t gear;
	u_int8_t time_type:2, //ʱ������
		time:6; //��ʱʱ��
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t index:4, //���
		group:4; //����index
	u_int8_t config_mode:4,//�ܹ�֧�ֶ�����ģʽ
		total_mode:4; //�����������ĸ�ģʽ��
	u_int8_t gear;
	u_int8_t time:6, //ʱ������
		time_type:2; //��ʱʱ��
#else
# error "Please fix <bits/endian.h>"
#endif
	
}amt_user_define_mode;

typedef struct{
	u_int8_t crc;
	u_int8_t fix_end_code1;
	u_int8_t fix_end_code2;
}amt_pkt_tail_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t env_dir:4,
		env_type:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t env_type:4,
		env_dir:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t env_val;
}amt_env_item_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t time:6,
		time_type:2;
	
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t time_type:2,
		time:6;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_time_item_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t timer_index:4,
			timer_type:2,
			timer_mode:2;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t timer_mode:2,
			timer_type:2,
		timer_index:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	amt_time_item_t item[4];	
}amt_machine_timer_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	
	u_int8_t shake_type:4,
		pad:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t pad:4,
		shake_type:4;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_shake_type_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t cur_angle:4,
		max_angle:4;
	
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t max_angle:4,
		cur_angle:4;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_shake_angle_t;

typedef struct{

	u_int8_t d_type;
	u_int16_t d_value;
}amt_elec_item_t;

#pragma pack(pop)

typedef struct{
	amt_54h_info_t cmd_54_info;
	u_int8_t cur_user_define_mode[MAX_USER_DEFINE_MODE_POINT];
}amt_mode_info_t;

typedef struct{
	amt_shake_type_t shake_type;
	amt_shake_angle_t shake_angle;
}amt_shake_info_t;

typedef struct{
	u_int8_t onoff; //����
	u_int8_t cur_mode; //��ǰģʽ
	u_int8_t cur_gear; //��ǰ��λ
	u_int8_t screen_light; //����
	u_int8_t is_shake; //�Ƿ���ҡͷ�׶�
	int8_t cur_temp;//��ǰ�¶�
	u_int8_t is_anion_on; //������
	u_int8_t is_plasma_on; //������
	u_int16_t cur_power; //��ǰ����
	
	u_int8_t is_timer_on_valid; //ԤԼ�����Ƿ���Ч
	u_int8_t is_timer_off_valid; //��ʱ�ػ��Ƿ���Ч
	u_int32_t dev_on_remain_time;//ԤԼ����ʣ��ʱ��
	u_int32_t dev_off_remain_time; // ��ʱ�ػ�ʣ��ʱ��

	u_int8_t product_type;
	u_int8_t product_category[32];
	amt_53h_info_t cmd_53_info;
	amt_mode_info_t mode_info;
	amt_shake_info_t shake_info;
}amt_fan_priv_data_t;

extern bool amt_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool amt_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern int amt_scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
extern bool amt_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
extern bool amt_send_set_timer_ctrl_cmd(smart_air_ctrl_t* ac);

extern int amt_get_ext_type_by_tlv(uc_tlv_t* tlv);

#endif

