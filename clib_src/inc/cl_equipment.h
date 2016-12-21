#ifndef	__CL_EQUIPMENT_H__
#define	__CL_EQUIPMENT_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

enum{
    CL_RT_BASE,
    CL_RT_SMOKE_DETECTOR = 1, /* ����̽���� */
    CL_RT_GAS_LEAK_DETECTOR = 2, /* �����Ӧ̽���� */
    CL_RT_INFRARED_DETECTOR = 3, /* ����̽���� */
    CL_RT_DOOR_SENSOR = 4, /* �ŴŸ�Ӧ�� */
    CL_RT_WINDOW_SENSOR = 5, /* �����Ÿ�Ӧ�� */
    CL_RT_INTELLIGENT_LOCK = 6, /* ������ */
    CL_RT_EMERGENCY_BUTTON = 7, /* ������ť */
    CL_RT_DB_SMOKE_DETECTOR = 21, /* ˫������̽���� */
    CL_RT_DB_GAS_LEAK_DETECTOR = 22, /* ˫�������Ӧ̽���� */
    CL_RT_DB_INFRARED_DETECTOR = 23, /* ˫�����̽���� */
    CL_RT_DB_DOOR_SENSOR = 24, /* ˫���ŴŸ�Ӧ�� */
    CL_RT_DB_WINDOW_SENSOR = 25, /* ˫�򴰻��Ÿ�Ӧ�� */
    CL_RT_DB_INTELLIGENT_LOCK = 26, /* ˫�������� */
    CL_RT_DB_EMERGENCY_BUTTON = 27, /* ˫�������ť */
    CL_RT_DB_MAX = 49,
    
    CL_RT_W_TV = 50,	/*WIFIת�������*/
    CL_RT_W_TVBOX = 51,	/*WIFIת���������*/
    CL_RT_W_AIRCONDITION = 52,	/*WIFIת����յ�*/
    CL_RT_W_OTHER = 53,	/*WIFIת�������ͺ������*/
    CL_RT_W_MAX = 99,
    
    CL_RT_CURTAIN =100,/*����*/
    CL_RT_PLUG = 101,/*����*/
    CL_RT_LAMP = 102,/*��*/
    CL_RT_RF_CUSTOM = 103,/*�Զ������߿���*/
    CL_RT_RF_CUSTOM_TV = 104, /*�Զ������ת�����Ƶ���*/
    CL_RT_RF_CUSTOM_TVBOX = 105, /*�Զ������ת�����ƻ�����*/
    CL_RT_RF_CUSTOM_AIRCONDITION = 106,/*�Զ������ת�����ƿյ�*/
    CL_RT_RF_CUSTOM_INFR_CUSTOM = 107,    /*�Զ����ת�Զ���*/
    CL_RT_SOUNDLIGHT = 120, /* ���ⱨ�������� */
    CL_RT_SCENE_CONTROLLER = 121, /* �龰ң���� */
    CL_RT_RF_MAX = 149,
    
    CL_RT_DB_RF_LAMP = 150,	/* RF ˫��ư� */
    CL_RT_DB_RF_PLUG = 151,	/* RF ˫����� */
    CL_RT_DB_RF_CURTAIN = 152, /* RF ˫���� */
    CL_RT_DB_RF_DIMMING_LAMP = 154,/*RF˫������*/
    CL_RT_DB_RF_SOUNDLIGHT = 155,/*˫�����ⱨ����*/
    CL_RT_DB_RF_MAX = 199,
    
    CL_RT_CLOUD_AIRCONDITION = 200, /* �����ƿյ� */
    CL_RT_CLOUD_TV = 201, /* �����Ƶ��� */
    CL_RT_CLOUD_STB = 202, /* �����ƻ����� */
    CL_RT_CLOUD_MAX = 249, /* ���������ֵ */
    
    CL_RT_MAX,
};

/*���Ӽ�ֵ*/
enum{
    RF_TV_KEYID_UP = 0x20, /*Ƶ��+*/
    RF_TV_KEYID_DOWN = 0x21,/*Ƶ��-*/
    RF_TV_KEYID_VOL_ADD=0x22,/*������*/
    RF_TV_KEYID_VOL_DEC = 0x23,/*������*/
    RF_TV_KEYID_MUTE = 0x24,/*����*/
    RF_TV_KEYID_SWITCH = 0x25,/*���ӣ���Ƶ*/
    RF_TV_KEYID_POWER = 0x26/*��Դ*/
};
/*�յ���ֵ*/
enum{
    RF_AIR_KEYID_HEAT = 0x20,/*����*/
    RF_AIR_KEYID_COOL = 0x21,/*����*/
    RF_AIR_KEYID_OFF = 0x22,/*�ػ�*/
};

/*�����м�ֵ*/
enum{
    RF_TVBOX_KEYID_UP = 0x20, /*Ƶ��+*/
    RF_TVBOX_KEYID_DOWN = 0x21,/*Ƶ��-*/
    RF_TVBOX_KEYID_VOL_ADD=0x22,/*������*/
    RF_TVBOX_KEYID_VOL_DEC = 0x23,/*������*/
    RF_TVBOX_KEYID_MUTE = 0x24,/*����*/
    RF_TVBOX_KEYID_SWITCH = 0x25,/*���ӣ���Ƶ*/
    RF_TVBOX_KEYID_POWER = 0x26,/*��Դ*/
    RF_TVBOX_KEYID_EXIT = 0x27,/*�˳�*/
    RF_TVBOX_KEYID_MENU = 0x28,/*�˵�*/
    RF_TVBOX_KEYID_OK = 0x29,/*�˳�*/
    RF_TVBOX_KEYID_ARROW_UP = 0x2A,/*��*/
    RF_TVBOX_KEYID_ARROW_DOWN = 0x2B,/*��*/
    RF_TVBOX_KEYID_ARROW_LEFT = 0x2C,/*��*/
    RF_TVBOX_KEYID_ARROW_RIGHT = 0x2D /*��*/
};
/*������ֵ*/
enum{
    RF_CURTAIN_KEYID_ON = 0x1,
    RF_CURTAIN_KEYID_OFF = 0x2,
    RF_CURTAIN_KEYID_STOP = 0x3
};
/*�ƺ��Ų��ֵ*/
#define SWITCH_ON_KEY_ID (101) /*�ܿ�*/
#define SWITCH_OFF_KEY_ID (102) /*�ܹ�*/
    /*����ӵ�ж����Ŀ��͹�ָ��base*/

/*�������ؼ�ֵ51-98  ��: 
id=51   ��һ·��
id=52   ��һ·��
id= 53  �ڶ�·������������
*/

#define SWITCH_KEYID_M_BASE (51) 
#define SWITCH_KEYID_M_MAX  (98)

#define SWITCH_KEYID_S_MIN  (1)
#define SWITCH_KEYID_S_MAX  (50) /*��ת���� 1-50*/
    
#define MAX_DIMMING_VALUE   (100)
#define MIN_DIMMING_VALUE   (0)

typedef struct {
    cl_obj_t obj;
    cl_handle_t equipment_handle; /*�����������*/
    u_int32_t key_id;
    bool had_learned; /*�ð����Ƿ�ѧϰ*/
}cl_key_t;
    
typedef struct {
	bool push_enable; /* �Ƿ�ʹ����Ϣ���� */
	bool sms_enable; /* �Ƿ�������Ϣ���� */
	bool isLearned; /*�Ƿ��Ѿ�ѧϰ�������ź�*/
	u_int8_t phone_num; /*�����绰����*/
	u_int8_t soundline_num; /* ���������ⱨ�������� */
	u_int8_t soundline_on; /* ʹ�����ⱨ���� */
	char* alarm_msg; /*������Ϣ*/
	char** phone_list; /*�����ĵ绰*/
	cl_handle_t *soundline; /* ���������ⱨ���� */
	cl_handle_t scene; /* �������龰��0Ϊû�� */
}cl_alarm_info_t;
    
/* electrical equipment�������������豸�� */
typedef struct {
    cl_obj_t obj;
    cl_handle_t area_handle; /*������*/
    cl_handle_t eq_001e_handle;/*�������豸���*/
    u_int8_t dev_type;		/* CL_RT_XXX */
    u_int8_t key_count;     //��������
    u_int8_t is_more_ctrl;   //�Ƿ�תʽ���� 0����ת 1����������
    u_int8_t is_rf_repeater; //1��ʾ֧��RF����
    u_int8_t rf_repeater_on; //is_rf_repeater��0ʱ��Ч��1��ʾ��������״̬��
    u_int8_t db_dimming_lamp_value;       //˫��������ֵ /*25-100*/
    u_int16_t group_num;     //�ƻ��Ų�·����˫����ȫ��ʱ��
    u_int16_t group_state;	//˫��RF����״̬��ÿ��bit��Ӧһ·״̬����˫������ǰλ��
    u_int16_t local_id; //����local_id
	u_int8_t match_id_num;	// �ƿյ�ƥ�䵽���ٸ����룬���4
    u_int8_t ac_type;	//�ƿյ�����
	u_int16_t match_id[4];	//���4��ƥ�䵽��ID��
	u_int16_t last_presskey;//���ƿյ���һ�ΰ��µļ�
    u_int16_t pad;
    cl_key_t** keys;            /*�����б�*/
    cl_alarm_info_t* alarm_info;    /*��������Ϣ*/
} cl_equipment_t;
    
typedef struct {
    char name[64]; //���63�ֽڣ�������Զ��ض�
    cl_handle_t eq_001e_handle; /*�����������ʱ��������*/
    cl_handle_t area_handle;
    u_int8_t dev_type; /*�����򱨾��豸���� CL_RT_XX */
    u_int8_t group_num; //�ƺͲ�����·�������֧��8·,
    u_int8_t is_more_ctrl; //�Ƿ�תʽ���� 0����ת 1����������
    u_int8_t pad; 
}cl_equipment_add_info_t;
 
typedef struct {
    cl_handle_t eq_handle; /*�����򱨾��豸handle*/
    u_int32_t key_id; /*�����İ���handle,���������*/
    u_int32_t remain_time; /*ʣ�൹��ʱ�䣬��EE_KL_HOST_WAIT_TIME��EE_KL_TRY_WAIT_TIME��Ч*/
    int last_error; /*���ִ���ʱ����־�ϴδ�����ʲôԭ��*/
    int max_ajust_value; /*��΢����Ƶ�ź����ֵ,��EE_KL_RF_SUPPORT_AJUST��Ч*/
    int min_ajust_value; /*��΢����Ƶ�ź����ֵ,��EE_KL_RF_SUPPORT_AJUST��Ч*/
    bool is_support_ajust_code;/*�Ƿ�֧��΢��,��EE_KL_RF_SUPPORT_AJUST��Ч*/
    bool is_support_ajust_plus_width;/*�Ƿ��������,��EE_KL_RF_SUPPORT_PLUS_WIDTH��Ч*/
    bool is_narrow_plus_width; /*��ǰ�������ֵ,�Ƿ���խ����������ǿ�����*/
}cl_key_learn_t;

/*���߱���ѧϰģʽ*/
typedef enum {
    KLM_LEARN_CODE_BY_HOST = 0x0,  /*�ǻۼ�ͥ����ѧϰ�����ı���*/
    KLM_LEARN_CODE_BY_EQUIPMENT = 0x1 /*����ѧϰ�����ı��룬��ͨ��˵�Ķ���*/
}KL_LEARN_MODE_T;

enum{
    ERR_KL_SAVE_FAIL = 660, /*����ѧϰ��ť��Ϣʧ��*/
    ERR_KL_NOTIFY_DEV_TO_LEARN_TIME_OUT, /*֪ͨ�豸�˽���ѧϰ״̬��ʱ*/
    ERR_KL_WAIT_USER_INPUT_TIME_OUT,/*�û���ң������ʱ*/
    ERR_KL_RECV_ALARM_TIME_OUT,/*���Խ���ѧϰ���ı����źų�ʱ���п���δѧϰ�ɹ������û�δ���������ź�*/
    ERR_KL_WAIT_TRY_RES_TIME_OUT,/*�ȴ��豸���س��Խ����ʱ*/
    ERR_KL_GEN_CODE_TIME_OUT,/*�ȴ��豸���ز�����������ʱ*/
    ERR_KL_STOP_TIME_OUT, //�ȴ��豸��Ӧֹͣѧϰ���ĳ�ʱ
    ERR_KL_PLUS_WIDTH_QUERY_TIME_OUT, //�ȴ��豸��Ӧ�����ѯ���ĳ�ʱ
    ERR_KL_PLUS_WIDTH_SET_TIME_OUT, //�ȴ��豸��Ӧ�������ñ��ĳ�ʱ
    ERR_KL_RF_AJUST_TIMEOUT, //�ȴ��豸��Ӧ��Ƶ΢�����ĳ�ʱ
    ERR_KL_DEVICE_BUSY_OR_OFFLINE, //�豸æ������ѧϰ���ߴ�������ź�
    ERR_KL_DUPLICATE_REMOTE_CODE //ѧϰ���ı��������ظ���
};
    
/*
 ע��: �����Ͱ���������ɾ���Ĳ�����ͳһ���û��ص�������֪ͨ.��Ϣ��������
 1->.�յ���Ӧ�����Ƿ�ɹ�����Ϣ
 2->.�յ��豸��Ϣ�䶯����Ϣ.UE_INFO_MODIFY
 */
    
enum {
    EE_BEGIN = 600,
    /*�����򰲷��豸��ӳɹ�*/
    EE_EQ_ADD_OK = EE_BEGIN+1,
    /*�����򰲷��豸���ʧ��*/
    EE_EQ_ADD_FAIL = EE_BEGIN+2,
    /*�����򰲷��豸ɾ���ɹ�*/
    EE_EQ_DEL_OK = EE_BEGIN+3,
    /*�����򰲷��豸ɾ��ʧ��*/
    EE_EQ_DEL_FAIL = EE_BEGIN+4,
    /*�����򰲷����޸ĳɹ�*/
    EE_EQ_MODIFY_OK = EE_BEGIN+5,
    /*�����򰲷����޸�ʧ��*/
    EE_EQ_MODIFY_FAIL = EE_BEGIN+6,
    /*˫��RFɨ��ɹ�*/
    EE_EQ_DB_RF_SCAN_OK = EE_BEGIN+7,
    /*˫��RFɨ��ʧ��*/
    EE_EQ_DB_RF_SCAN_FAIL = EE_BEGIN+8,
    
    /*������ӳɹ�*/
    EE_EKEY_ADD_OK = EE_BEGIN+20,
    /*�������ʧ��*/
    EE_EKEY_ADD_FAIL = EE_BEGIN+21,
    /*����ɾ���ɹ�*/
    EE_EKEY_DEL_OK = EE_BEGIN+22,
    /*����ɾ��ʧ��*/
    EE_EKEY_DEL_FAIL = EE_BEGIN+23,
    /*�����޸ĳɹ�*/
    EE_EKEY_MODIFY_OK = EE_BEGIN+24,
    /*�����޸�ʧ��*/
    EE_EKEY_MODIFY_FAIL = EE_BEGIN+25,
    /*�����źŷ��ͳɹ�*/
    EE_EKEY_SEND_SINGAL_OK = EE_BEGIN+26,
    /*�����źŷ���ʧ��*/
    EE_EKEY_SEND_SINGAL_FAIL = EE_BEGIN+27,
    
    /*��ʼѧϰ,�ȴ��û�����ѧϰ�İ�ť,30s����ʱ*/
    EE_KL_HOST_WAIT_TIME = EE_BEGIN+41,
    /*�ź�ѧϰ�ɹ�*/
    EE_KL_LEARN_OK = EE_BEGIN+42,
    /*���������źųɹ�*/
    EE_KL_GEN_CODE_OK = EE_BEGIN+43,
    /*ȷ���źŵ���ʱ,���������豸�д���Ϣ*/
    EE_KL_TRY_WAIT_TIME =     EE_BEGIN+44,
    /*�������豸�ã���ʾ����ʱʶ���˱����ź�*/
    EE_KL_RECV_ALARM_OK = EE_BEGIN+45,
    /*����ѧϰ����������źųɹ�*/
    EE_KL_TRY_SEND_OK = EE_BEGIN+46,
    /*����ѧϰ(����΢������������)�����źųɹ�*/
    EE_KL_SAVE_CODE_OK = EE_BEGIN+47,
    /*�õ��������ź�֧��΢��*/
    EE_KL_RF_SUPPORT_AJUST = EE_BEGIN+48,
    /*΢���ɹ�*/
    EE_KL_RF_AJUST_OK = EE_BEGIN+49,
    /*֧������*/
    EE_KL_RF_SUPPORT_PLUS_WIDTH = EE_BEGIN+50,
    /*��������ɹ�*/
    EE_KL_PLUS_WIDTH_AJUST_OK = EE_BEGIN+51,
    /*ֹͣѧϰ�ɹ�*/
    EE_KL_STOP_OK = EE_BEGIN+52,
    /*���ִ���*/
    EE_KL_ERROR = EE_BEGIN+53,

	// �龰����
    EE_LINKAGE_SCENE_DEL_OK = EE_BEGIN + 60,
    EE_LINKAGE_SCENE_DEL_FAIL = EE_BEGIN + 61,
    EE_LINKAGE_SCENE_MODIFY_OK = EE_BEGIN + 62,
    EE_LINKAGE_SCENE_MODIFY_FAIL = EE_BEGIN + 63,
	
    EE_END = EE_BEGIN + 99
};
/***************************************************************************************************/
//ÿ���豸���֧������10���ֻ�����
/*
     ����:
         �������������ֻ�����
     �������:
         @user_handle: ���豸���
         @phone: ��ӵ��ֻ�����
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��
     ע������:

        �յ� UE_MODIFY_ALARM_PHONE_OK �豸����ӳɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
        �յ� UE_MODIFY_ALARM_PHONE_FAIL �豸�����ʧ��
 */

CLIB_API RS cl_user_add_alarm_phone(cl_handle_t user_handle, const char* phone);

/*
     ����:
         ɾ�����������ֻ�����
     �������:
         @user_handle: ���豸���
         @phone: ɾ�����ֻ�����
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��
     ע������:
     
         �յ� UE_MODIFY_ALARM_PHONE_OK �豸����ӳɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
         �յ� UE_MODIFY_ALARM_PHONE_FAIL �豸�����ʧ��
 */
CLIB_API RS cl_user_del_alarm_phone(cl_handle_t user_handle, const char* phone);

/*
     ����:
         �������������ⱨ����
     �������:
         @eq_handle: ���������豸���
         @on: �Ƿ񿪹�
         @soundline: Ҫ���������ⱨ�����ľ������
         @soundline_count: Ҫ���������ⱨ�����ĸ���
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��
     ע������:
     
         �յ� UE_MODIFY_ALARM_PHONE_OK �豸����ӳɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
         �յ� UE_MODIFY_ALARM_PHONE_FAIL �豸�����ʧ��
 */
CLIB_API RS cl_eq_bind_soundlight(cl_handle_t eq_handle, bool on, 
				cl_handle_t *soundline, u_int8_t soundline_count);

/***************************************************************************************************/
// ��������
    
//1.ֻ�������豸����ӵ���  2.�����豸���ܷ������ź� 3.������ҪԤ������ˢ�µ����Ͱ����Ľӿ�
/*
     ����:
         ��ӵ����򱨾���
     �������:
         @slave_handle: ���豸���
         @eq_handle: �洢��������ĵ�ַָ��
         @info: ��ӵ���ʱ��Ҫ����Ϣ
     �������:
         @eq_handle: �µĵ����豸���
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     ע������:
         Ŀǰ����ת����ֻ֧��001e
         �յ� EE_EQ_ADD_OK �豸����ӳɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
         �յ� EE_EQ_ADD_FAIL �豸�����ʧ��
 
 */
CLIB_API RS cl_eq_add(cl_handle_t slave_handle,cl_handle_t* eq_handle,cl_equipment_add_info_t* info);

/*
     ����:
         �޸ĵ�����������
     �������:
         @eq_handle: �������
         @area_handle: ������
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     	����: ʧ��

	�¼�: EE_EQ_MODIFY_OK��EE_EQ_MODIFY_FAIL
 */
CLIB_API RS cl_eq_modify_area(cl_handle_t eq_handle, cl_handle_t area_handle);


/*
     ����:
         ��ӵ����򱨾���(ʹ�ö�ά��ɨ������ӣ�����Ҫѧϰ)
     �������:
         @slave_handle: ���豸���
         @eq_handle: �洢��������ĵ�ַָ��
         @json: ��ӵ���ʱ��Ҫ����Ϣ, json��ʽ
     �������:
         @eq_handle: �µĵ����豸���
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     ע������:
         �յ� EE_EQ_ADD_OK �豸����ӳɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
         �յ� EE_EQ_ADD_FAIL �豸�����ʧ��
 
 */
CLIB_API RS cl_eq_add_by_json(cl_handle_t slave_handle,cl_handle_t* eq_handle, const char *json);

/*
     ����:
         ɾ�������򱨾���
     �������:
         @eq_handle: �����豸���
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     ע������:
     �յ� EE_EQ_DEL_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EQ_DEL_FAIL �豸�˴���ʧ��
     
 */
CLIB_API RS cl_eq_del(cl_handle_t eq_handle);


/*
     ����:
         �޸ĵ����򱨾�������
     �������:
         @eq_handle: �����豸���
         @new_name: �µĵ��������63�ֽ�
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     �յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
     
 */

CLIB_API RS cl_eq_modify_name(cl_handle_t eq_handle, const char* new_name);

/*
 ����:
    ����˫����������
 �������:
    @eq_handle: �����豸���
    @key_id: ����ư���ID
    @value: ����ֵ(25-100֮��)
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 
ע������:
 �յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
 �յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
 
 */
CLIB_API RS cl_set_db_dimming_value(cl_handle_t eq_handle,u_int8_t value);

/*
     ����:
         ���ñ��������� (������Ϣ)
     �������:
         @eq_handle: �����豸���
         @msg: �µı�����Ϣ���63�ֽ�
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     �յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
     
 */
CLIB_API RS cl_eq_set_alarm_msg(cl_handle_t eq_handle, const char* msg);

/*
     ����:
         ���ú����豸�󶨵�001E ����ת����
     �������:
         @eq_handle: �����豸���
         @slave_handle: 001E �豸���
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     �յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
     
 */    
CLIB_API RS cl_eq_set_alarm_assist_001e(cl_handle_t eq_handle,cl_handle_t slave_handle);

/*
     ����:
         ���ñ������������ֻ������б�
     �������:
         @eq_handle: �����豸���
         @phonelist: ���õ��ֻ������б�,�ַ���ָ������
         @phone_count��Ҫ���ö��ٸ��ֻ�����,���֧��10��
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     phonelist�е��ֻ��б������cl_user_set_alarm_phones ���õ��Ӽ�
     
     �յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
     
 */ 

CLIB_API RS cl_eq_set_alarm_phones(cl_handle_t eq_handle,char** phonelist,u_int8_t phone_count);

/*
	������Ҫע�����: ���Ų��ܵ�������! �����űؿ���Ϣ���ͣ�����Ϣ���ͱعض��š�
*/

/*
	!!!!!!!! ע�� !!!!!!!!
		���ӿڷ�������ʹ��cl_eq_enable_alarm_push �� cl_eq_enable_alarm_sms

     ����:
         ������������رն�������/��Ϣ����
     �������:
         @eq_handle: �����豸���
         @enable: false �ر�true ����
     �������:
         ��
     ����:
		RS_OK: �ɹ�
		����: ʧ��
     
     ע������:
		�յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
		�յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
     
		cl_dev_info_t->has_alarm_swichΪfalseʱ�����ӿڿ��Ƶ��Ƕ��ź���Ϣ����������
		cl_dev_info_t->has_alarm_swichΪtrueʱ�����ӿڿ��Ƶ��Ƕ���
 */    
CLIB_API RS cl_eq_enable_alarm(cl_handle_t eq_handle,bool enable);

/*
     ����:
         ������������ر���Ϣ����
     �������:
         @eq_handle: �����豸���
         @enable: false �ر�true ����
     �������:
         ��
     ����:
		RS_OK: �ɹ�
		����: ʧ��
     
     ע������:
		�յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
		�յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��
*/    
CLIB_API RS cl_eq_enable_alarm_push(cl_handle_t eq_handle,bool enable);

/*
     ����:
         ������������رն�������
     �������:
         @eq_handle: �����豸���
         @enable: false �ر�true ����
     �������:
         ��
     ����:
		RS_OK: �ɹ�
		����: ʧ��
     
     ע������:
		�յ� EE_EQ_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
		�յ� EE_EQ_MODIFY_FAIL �豸�˴���ʧ��

		cl_dev_info_t->has_alarm_swichΪtrueʱ����֧�ֱ����ܡ�
*/    
CLIB_API RS cl_eq_enable_alarm_sms(cl_handle_t eq_handle,bool enable);

/*
     ����:
       �ͷű�������Ϣ����
     �������:
         @ai, ��������Ϣָ��
     �������:
         ��
     ����:
         RS_OK: ��
     ����: ʧ��
     
     ע������:
     ��
     
 */  

CLIB_API void cl_alarm_info_free(cl_alarm_info_t* ai);

/***************************************************************************************************/
// ������������
/*
     ����:
         ��Ӱ���
     �������:
         @eq_handle: �����豸���
         @key_id: �û�����ָ��,�����ƽ̨������ͬ
         @key_name: �������ƣ��63�ֽ�
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     key_id ������Բο���ͷ�ļ��϶�key����
     
     �յ� EE_EKEY_ADD_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EKEY_ADD_FAIL �豸�˴���ʧ��
     
 */   

CLIB_API RS cl_key_add(cl_handle_t eq_handle,u_int32_t key_id, const char* key_name);

/*
     ����:
         ɾ������
     �������:
         @eq_handle: �����豸���
         @key_id: ��Ҫɾ���İ���id
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     �յ� EE_EKEY_DEL_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EKEY_DEL_FAIL �豸�˴���ʧ��
     
 */   
CLIB_API RS cl_key_del(cl_handle_t eq_handle,u_int32_t key_id);

/*
     ����:
         �޸İ�������
     �������:
         @eq_handle: �����豸���
         @key_id: ��Ҫ�޸ĵİ���id
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     
     �յ� EE_EKEY_MODIFY_OK �豸�˴���ɹ� �������յ� UE_INFO_MODIFY ��ɸ�����Ϣ
     �յ� EE_EKEY_MODIFY_FAIL �豸�˴���ʧ��
     
 */ 
CLIB_API RS cl_key_modify_name(cl_handle_t eq_handle,u_int32_t key_id, const char* new_name);

/*
     ����:
         ���Ϳ����ź�
     �������:
         @eq_handle: �����豸���
         @key_id: ��Ҫ���Ƶİ���id
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     
     �յ� EE_EKEY_SEND_SINGAL_OK �豸�˴���ɹ�
     �յ� EE_EKEY_SEND_SINGAL_FAIL �豸�˴���ʧ��
     
 */ 
CLIB_API RS cl_key_send_ctrl_singal(cl_handle_t eq_handle,u_int32_t key_id);


/***************************************************************************************************/
// ��������ѧϰ
/*
*******************************************
�ص���Ϣͨ�ô���:
{
    cl_key_learn_t* kl = cl_key_learn_get_stat(xx_u_handle);
    
    switch(event){
       case EE_KL_HOST_WAIT_TIME:
         ��ʾ����ʱʱ��
         break;
         default:
           break;
    }
}
********************************************
����ת��ѧϰ����
1. ��Ӻð�001e�ĺ���ת������������֧��001e��
2. ����cl_set_key_learn_callback
3. ����cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_EQUIPMENT
4.����cl_kl_gen_code �������룬�ú���ת����ѧϰ��RF �ź�,(���յ�EE_KL_GEN_CODE_OK�¼�)
5. ������ң�����϶�Ӧ��ť���ú���ת����ѧ�������ź�
6. ����cl_kl_try_ctrl ���Կ���(��ѡ)��(���յ�EE_KL_TRY_SEND_OK �¼�)
7.����cl_kl_save_learn_code ����(���յ�EE_KL_SAVE_CODE_OK �¼�)
**************************************************************
������ѧϰ����
1. �ȵ���cl_eq_set_alarm_msg ���úñ�����Ϣ.(�����ڴ�����ɺ󣬺�̨�Զ�����)
2. ����cl_set_key_learn_callback
3. ����cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_HOST key_id =0 (���յ�EE_KL_HOST_WAIT_TIME �¼�)
4. �ȴ��û�����������(�յ�EE_KL_LEARN_OK ��ʾѧϰ�ɹ�)
5. �յ�EE_KL_SAVE_CODE_OK �¼�(�������豸�Զ������ź�)
6. ����cl_kl_try_ctrl ���Կ���(��ѡ)��(�յ�EE_KL_TRY_WAIT_TIME ��ʾ�ȴ��û��ٴ�
�������������յ�EE_KL_RECV_ALARM_OK �¼���ʾ����ƥ�䱨���źųɹ�)
************************************************************************************
RF ����ѧϰ����
1. ����cl_set_key_learn_callback  
2. ����cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_EQUIPMENT 
3.����cl_kl_gen_code ��������(���յ�EE_KL_GEN_CODE_OK�¼�)
4. ����cl_kl_try_ctrl ���Կ���(���յ�EE_KL_TRY_SEND_OK �¼�)
5.����cl_kl_save_learn_code ����(���յ�EE_KL_SAVE_CODE_OK �¼�)
************************************************************************************
RF����ѧϰ��������
1. ����cl_set_key_learn_callback  
2. ����cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_HOST (���յ�EE_KL_HOST_WAIT_TIME �¼�)
3. �ȴ��û�������ң����(�յ�EE_KL_LEARN_OK ��ʾѧϰ�ɹ�)
4. ����cl_kl_try_ctrl ���Կ���(��ѡ)��(�յ�EE_KL_TRY_SEND_OK �¼���ʾ�������Ϳ����źųɹ�)
5.����cl_kl_save_learn_code ����(���յ�EE_KL_SAVE_CODE_OK �¼�)
************************************************************************************
RF �ź�΢��
1.ѧϰ�ɹ����յ�EE_KL_RF_SUPPORT_AJUST����ʾ֧���ź�΢����
2.��cl_key_learn_t ��ȡ��΢����Χ��
3. ����cl_kl_ajust_code ����΢��
************************************************************************************
�������
1. ����ģʽ�У��յ�EE_KL_RF_SUPPORT_PLUS_WIDTH����ʾ֧���������
2.��cl_key_learn_t ��ȡ�õ�ǰ����
3.  ����cl_kl_set_plus_width ��������
*/


/*
     ����:
         ���ð���ѧϰ�ص�������ÿ�û��ڳ��������ڵ���1�ξͿ���
     �������:
         @user_handle: �豸���
         @callback: �ص�������ַ
         @callback_param: �ص���������
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     ��
 */ 

CLIB_API RS cl_set_key_learn_callback(cl_handle_t user_handle,cl_callback_t callback,void* callback_param);

/*
     ����:
         ����ѧϰ��������ʼ
     �������:
         @user_handle: �豸���
         @eq_handle; �����򱨾������
         @key_id: ����key_id  ,������ʱ��0
         @learn_mode : ѧϰģʽ
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     ÿ��ѧϰ�������
 */ 
CLIB_API RS cl_key_learn_start(cl_handle_t user_handle,cl_handle_t eq_handle,u_int32_t key_id,
                               KL_LEARN_MODE_T learn_mode);

/*
     ����:
         ֪ͨ�豸����ѧϰ��(����΢����)���ź�
         ��֪ͨ�豸׼�����ձ������ź�
     �������:
         @user_handle: �豸���
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     ��
 */ 

CLIB_API RS cl_kl_try_ctrl(cl_handle_t user_handle);

/*
     ����:
         ����ģʽ�����ת��ģʽ�²�������
     �������:
         @user_handle: �豸���
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     ��
 */ 
CLIB_API RS cl_kl_gen_code(cl_handle_t user_handle);

 /*
     ����:
         ֹͣѧϰ
     �������:
         @user_handle: �豸���
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
     ѧϰ��ɺ�������(����ʧ�ܻ��ǳɹ�)
 */ 
CLIB_API RS cl_key_learn_stop(cl_handle_t user_handle);

/*
     ����:
         ΢��RF �ź�
     �������:
         @user_handle: �豸���
         @ajust_value: ��Ҫ΢����ֵ
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 
CLIB_API RS cl_kl_ajust_code(cl_handle_t user_handle,int ajust_value);

/*
     ����:
         ��������
     �������:
         @user_handle: �豸���
         @is_narrow:  false:����Ϊ������
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 
CLIB_API RS cl_kl_set_plus_width(cl_handle_t user_handle,bool is_narrow);

/*
     ����:
         ��ѯ��ǰ����
     �������:
         @user_handle: �豸���
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 

CLIB_API RS cl_kl_save_learn_code(cl_handle_t user_handle);

/*
     ����:
         ��ȡ��ǰѧϰ״̬�Ͳ���
     �������:
         @user_handle: �豸���
     �������:
         ��
     ����:
         �ǿ�: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 
CLIB_API cl_key_learn_t* cl_key_learn_get_stat(cl_handle_t user_handle);

/*
     ����:
        �ͷ�cl_key_learn_get_stat �õ�������
     �������:
         @stat: cl_key_learn_get_stat ����ָ��
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 

CLIB_API void cl_kl_stat_free(cl_key_learn_t* stat);

/*
     ����:
        ���豸��������Χ˫���������һ�ΰ󶨳���
     �������:
         ��
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 
CLIB_API RS cl_user_scan_db_rf(cl_handle_t user_handle);

/*
     ����:
         ���ñ����������龰
     �������:
         @eq_handle: ���������豸���
         @scene_handle: Ҫ�������龰��
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��

     �����յ��¼�:
     	EE_LINKAGE_SCENE_MODIFY_OK �޸��龰�����ɹ�
     	EE_LINKAGE_SCENE_MODIFY_FAIL �޸��龰����ʧ��
 */
CLIB_API RS cl_eq_linkage_scene_set(cl_handle_t eq_handle, cl_handle_t scene_handle);

/*
     ����:
         ɾ�������������龰
     �������:
         @eq_handle: ���������豸���
         @scene_handle: ֮ǰ�������龰��
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��

     �����յ��¼�:
     	EE_LINKAGE_SCENE_DEL_OK ɾ���龰�����ɹ�
     	EE_LINKAGE_SCENE_DEL_FAIL ɾ���龰����ʧ��
 */
CLIB_API RS cl_eq_linkage_scene_del(cl_handle_t eq_handle, cl_handle_t scene_handle);

/*
     ����:
         ˢ�µ���״̬
     �������:
         @eq_handle: �����豸���
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��

     �����յ��¼�:
     	UE_INFO_MODIFY 
 */
CLIB_API RS cl_eq_refresh(cl_handle_t eq_handle);

/*
     ����:
         RF����ģʽ����
     �������:
         @eq_handle: �����豸���
         @repeater_on: 1��ʾ����Ϊ����ģʽ��0��ʾȡ������ģʽ
     �������:
         ��
     ����:
         RS_OK: ���������ĳɹ�
     ����: ʧ��

     �����յ��¼�:
     	UE_INFO_MODIFY 
 */
CLIB_API RS cl_eq_rf_repeater_set(cl_handle_t eq_handle, u_int8_t repeater_on);
/***************************************************************************************************/

#ifdef __cplusplus
}
#endif 

#endif



