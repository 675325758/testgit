#ifndef CL_COM_RF_DEV_H
#define CL_COM_RF_DEV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//ö��ֵ����д��ֵ�ɣ���Ȼ��ʱ����Ҫ��ѯֵ��ʱ����Ҫ������
typedef enum {
    D_T_UNKNOWN = 0,
    D_T_LAMP = 1, // RGB ��ɫ��
    D_T_DOOR_LOCK = 2, //����
    D_T_DOOR_MAGNET = 3,//�Ŵ�
    D_T_DHX_SWITCH = 4, //�����߿���
    D_T_YT_DOOR_LOCK = 5, //��̩����
    D_T_HM_MAGNET = 6,//�����Ŵ�
    D_T_HM_BODY_DETECT = 7,//��������̽��
    D_T_HM_TEMP_HM = 8,//������ʪ��̽��
    D_T_KTCZ = 9,	// ���ز���
    D_T_HEATING_VALVE = 10,	// ů����
    D_T_GAS = 11,		// ������
    D_T_QSJC = 12,		// ˮ���
    D_T_HTLLOCK = 13,	// ��̩����
    D_T_HMCO = 14,	// һ����̼���
    D_T_HMYW = 15,	// ������
    D_T_DWYK = 16,	//��΢ң����
    D_T_WK_AIR = 17, //��տյ���
    D_T_DOOR_MAGNETV2 = 18,	// V2�汾�Ŵ�
    D_T_SCENE_CONTROOLER = 19,	// �龰ң����
	D_T_DWHF = 20,	//�����Ϸ�оƬ��
	D_T_HMQJ = 21,	// ���������  
	D_T_ZHDJ = 22, //�ǻʵ��
	D_T_DWKJ = 23,	//  �����Ƽ�
	D_T_YLTC = 24,// ҹ�Ǻ���
	D_T_YLWSD = 25,//ҹ����ʪ��
	D_T_YLSOS = 26,//ҹ��sos
	D_T_YLLOCK = 27,//ҹ���Ŵ�
	D_T_YLLIGHT = 28,	// ҹ�����ⱨ����
	D_T_DHXZH_SWITCH = 29,	// �ǻʵ�����
	D_T_DHXCP_SWITCH = 30,	// ���ӵ�����
	D_T_DWYKHF = 31,//��΢�Ϸ�ң����
	D_T_DWYSTGQ = 32,	// �������ٵ�����
	D_T_WK_AIR2 = 33, //�յ���2
	D_T_YSD = 34, //���ٵ�
	D_T_CDQJMB = 35, //�ȵ��龰���
	D_T_JQ = 36,	// ��ȩ������
	D_T_MLDHX = 37,//ħ�ֵ�����
	D_T_LIGHT_SENSE = 38,// ���մ�����
	// �м䲻Ҫ�ټ���
	D_T_LAMP_START = 39,	// ��������չ������ʼ
	D_T_LAMP_END = 54,		// ��������չ���ͽ���
	// �м䲻Ҫ�ټ���
	D_T_LHX = 55,	// �����
	D_T_WUANS6 = 56,	// ��S6�������
	
	D_T_MAX,
}cl_slave_data_type;
    
    enum{
        LAMP_CTRL_MODE_SWITCH = 0x1,
        LAMP_CTRL_MODE_LAYER
    };

#pragma pack(push,1)


///////////////////////////////////////////////////
//��
    
typedef struct {
    u_int8_t R;
    /*�� 0~255*/
    u_int8_t	B;
    /*��0-255*/
    u_int8_t 	G;
    /*��0~255*/
    u_int8_t	L;
    /*����0~100*/
    u_int8_t cold;
    /*ɫ��*/
    u_int8_t	power;
    /*���أ�0Ϊ�أ�1Ϊ��*/
    u_int8_t	mod_id; //�龰ģʽ���
    /*ģʽid*/
    u_int8_t	action; //0:����RGB 1:����ɫ��
    //��Ϊ
    u_int8_t ctrl_mode; //0�����Ƶ� 1�����ƿ��أ�2�����Ʋ�
    u_int8_t value;
	u_int8_t flag;
    u_int8_t pad;
}cl_rf_lamp_stat_t;

#define MAX_DW_LAMP_REMOTE_CNT 4
#define MAX_DW_LAMP_KEY_ID 8
//����ң�������
// key_id 1-4
typedef struct {
    u_int32_t remote_id;
    u_int8_t key_id;
    u_int8_t pad[3];
}cl_rf_lamp_remote_info_t;


enum {
	LED_UI_WC_UPPER = 8, //WC, WC�ֲ��ϲ�
	LED_UI_WC_LOWER,     //WC�ֲ��²�
	LED_UI_WC_DOUBLE,    //WC�ֲ�����
	LED_UI_RGB,          //RGBɫ�壬RGB�龰ģʽҳ��
	LED_UI_SMART_SWITCH, //���ܿ���ҳ��
	LED_UI_POWER_SWITCH, //��Դ���ذ�ť
	LED_UI_WHITE_ONLY,	// ר�ſ��ư׹�
};

enum{
    RL_LT_WC_OR_RGB = 0x1,
    RL_LT_WC_ONLY = 2,
    RL_LT_WC_AND_RGB = 3,
    RL_LT_LAYER = 4,
    RL_LT_SWITCH = 5,
    RL_LT_C_ONLY = 6,
    RL_LT_C_OR_RGB = 7,
    RL_LT_RGB_ONLY = 8,
    RL_LT_RGB_BELT = 9,	// RGB�ƴ�
    RL_LT_MAX
};
    
typedef struct {
    cl_rf_lamp_stat_t stat;
    cl_rf_lamp_remote_info_t r_info[MAX_DW_LAMP_REMOTE_CNT];
    u_int8_t remote_count; //���ڼ���ң����
    u_int8_t is_support_color_temp; //�Ƿ�֧��ɫ�µ���
    u_int8_t is_support_rgb; //�Ƿ�֧��rgb����
    u_int8_t lamp_type; // 0 ֧��RGB��ɫ�£����߻��⣬1:��֧��ɫ�� 2��֧��RGB��ɫ�£����߲���
}cl_rf_lamp_t;

//////////////////////////

////////////
// ҹ�����ⱨ����
enum {
	ACT_RF_YLLIGHT_MOD= 1,		// ģʽ0 ���� 1 ����
	ACT_RF_YLLIGHT_SOUND,	// ��������0 ֹͣ1��ʼ

	// ����SDK�Լ���
	ACT_RF_YLLIGHT_LAMP_CTRL = 100,	
	ACT_RF_YLLIGHT_ALARM_CONFIG,	// ����Ĭ�ϱ���ʱ��
};

enum {
	TLV_YLLIGHT_MODE_SET = 210,
	TLV_YLLIGHT_SOUND = 211,
	TLV_YLLIGHT_ALARM_CONFIG = 212,
	TLV_YLLIGHT_LAMP_SET = 213,
	TLV_YLLIGHT_QUERY = 214,
};

typedef struct {
    u_int8_t R;
    u_int8_t G;
    u_int8_t B;
    u_int8_t W;
    u_int8_t C;
    u_int8_t power;
    u_int8_t mod_id;
    u_int8_t o_wc_l;
    u_int8_t o_r;
    u_int8_t o_g;
    u_int8_t o_b;
    u_int8_t o_l;
    u_int8_t o_c;
    u_int8_t hwconf;
} ucp_yllight_stat_t;

typedef struct {
	cl_rf_lamp_t lamp_stat;	// �������Ϣ
	u_int8_t mode;	// 0 ����ģʽ 1 ����ģʽ
	u_int8_t is_alarm;	// ����ģʽ�±�ʾ���ڱ���
	u_int8_t is_dynamic;	// ����ģʽ�±�ʾ�����Ƿ�Ϊ��̬ģʽ
	//u_int8_t duration;	// ��������ʱ�� 1-180��

		/*
	������ʽ��
	    0 -- �������������ñ�����
	    1 -- ����һ�Σ���ʱ��total_time��Ч
	    2 -- ��Ъ������on_time��off_time����Ϊ��0����ʾһ�������ڵĿ�����ֹͣʱ��
	        total_timeΪ0����ʾ�����������������Ա�������0���ʾ�ڱ���total_time�Ժ�ֹͣ 
	*/
	u_int8_t alarm_mode;
	u_int8_t on_time;	// һ�������ڵĿ���ʱ�䣬��λ�룬��Χ0-255
	u_int16_t off_time;	// һ�������ڵ�ֹͣʱ�䣬��λ�룬��Χ0-65535
	u_int16_t total_time;	// ����ʱ���ܳ��ȣ���λ���ӣ���Χ0-65535
	// ��������SDK��
	bool get_cache;
} cl_yllight_info_t;


typedef struct {
	u_int8_t flagbits;
	u_int8_t hislog_count;
	/*
	������ʽ��
	    0 -- �������������ñ�����
	    1 -- ����һ�Σ���ʱ��total_time��Ч
	    2 -- ��Ъ������on_time��off_time����Ϊ��0����ʾһ�������ڵĿ�����ֹͣʱ��
	        total_timeΪ0����ʾ�����������������Ա�������0���ʾ�ڱ���total_time�Ժ�ֹͣ 
	*/
	u_int8_t alarm_mode;
	u_int8_t on_time;	// һ�������ڵĿ���ʱ�䣬��λ�룬��Χ0-255
	u_int16_t off_time;	// һ�������ڵ�ֹͣʱ�䣬��λ�룬��Χ0-65535
	u_int16_t total_time;	// ����ʱ���ܳ��ȣ���λ���ӣ���Χ0-65535
	u_int32_t hislog_index_current;
	//ucp_yllight_stat_t stat;
} yllight_cache_t;

// ����ʱ��,�±�0��ʼ�������ֵ���ֱ��ʾ
// ��ʼ������ʱ�䣬���ʱ��
typedef struct {
	u_int32_t time[4];
} cl_alarm_time_t;    

typedef struct {
	u_int8_t alarm_mode;
	u_int8_t on_time;
	u_int16_t off_time;
	u_int16_t total_time;
} cl_yllight_alarm_config_t;

//////////////////////////
//����

//������������
enum{
    RDL_CTRL_OPEN = 0x1, //����������
    RDL_CTRL_GRARD //���Ʋ�������
};

// ����˽�е�TLV TYPE 
enum {
	DOOR_LOCK_PRIV_TYPE_QUERY_WIFI_LOCK = 212,	//  ��ѯWIFI��������Ϣ
	DOOR_LOCK_PRIV_TYPE_SET_WIFI_LOCK = 211, //���ý���WIFI�Զ�����,�Ͽ�WIFI�Զ�����
	DOOR_LOCK_PRIV_TYPE_SET_UNLOCK_TIMEOUT = 213, //����δ������ʱʱ��
	DOOR_LOCK_PRIV_TYPE_SET_DISTINGUISH_MODE = 214,	// ����ң����ʶ��ģʽ�����úͻظ�
	DOOR_LOCK_PRIV_TYPE_PUSH_CONTROLLER_ID = 215,	// �ϱ�ң����ID
	DOOR_LOCK_PRIV_TYPE_SET_CONTROLLER_INFO = 216,//ң������������
	DOOR_LOCK_PRIV_TYPE_QUERY_CONTROLLER_INFO = 217,//ң������Ϣ��ѯ

	DOOR_LOCK_PRIV_TYPE_ASSOCIATE = 220,	// ��̩��������
};

//����ң����״̬����
enum{
    DOOR_LOCK_REMOTECONTROLLER_NORAML = 0x1, //����
    DOOR_LOCK_REMOTECONTROLLER_REPORT_LOSS = 0x2, //��ʧ
    DOOR_LOCK_REMOTECONTROLLER_DELETE = 0x3, //ɾ��
};

typedef struct {
	u_int32_t flagbits;
	u_int32_t hislog_index_current;
	u_int8_t max_hislog_count;
	u_int8_t battery;
	u_int8_t ass_state;
	u_int8_t v_alarm;
} yt_door_lock_tt_cache_t;


typedef struct {
	u_int32_t flagbits;
	u_int32_t hislog_index_current;
	u_int8_t max_hislog_count;
	u_int8_t battery;
	u_int8_t open_timeout_en;
	u_int8_t open_timeout_v;
	u_int8_t v_wifilock;
	u_int8_t v_alarm;
	u_int8_t controller;	// ��4λ��ʾ���ٸ�ң��������4λ��ʾ���һ�ΰ��µ�ң����ID
	u_int8_t v_cid;
	u_int8_t controller_idx[5];
	u_int8_t lan_num;
} door_lock_tt_cache_t;



typedef struct {
    u_int8_t battary; //���ʣ�������,��ֻ̩���Ƿ�Ϊ�͵������� 101������δ֪��102�� �������� 103����ʾ�͵���
    u_int8_t is_door_open; //�Ƿ���״̬��
    u_int8_t is_look_open; //���Ƿ���״̬
    u_int8_t is_battary_warn; //��ص͵�������
    u_int8_t has_break_door; //�Ƿ������ƻ��ţ���˨δ�õ���
    u_int8_t is_guard; //�Ƿ��ڲ���״̬
	u_int8_t unlock_timeout_enable; // �Ƿ�֧���ų�ʱ���ͱ���
	u_int8_t unlock_timeout;	// �Ŷ��û������

	u_int8_t has_limit_fault;	// ��λ������
	u_int8_t has_moto_fault;	// �������
	u_int8_t has_unlock_timeout;	// ��ʱ��û����(��˨δ�õ�)

    //////////////////////////////////////
    //��̩��������
    u_int8_t is_break_lock; //�Ƿ��ƻ�����
    u_int8_t has_open_passwd; //�Ƿ��������˿�������
    u_int8_t ass_state;	// ����״̬ 0 ��ȡ485ģ��ID 1 ������ 2 �����ɹ�3 δ֪״̬

	// ң����
	u_int8_t last_controller_id;	// ���һ�ΰ��µ�ң����ID��SAE_RF_DEV_DOOR_LOCK_CONTROLLER = 1289 �յ�����ź��Ժ��ȡ����Ч

	// �Ƿ�ʹ���µĵ��ͼ��
	// ��ͼ������£�battary ֻ��[5 20 40 60 80 100]�⼸��ֵ
	u_int8_t is_new_battary_show;

	// ������SDK˽�нṹ
	bool has_get_stat;
	u_int8_t wifilock_index;
	u_int8_t alarm_index;
	u_int8_t cid_index;
	u_int8_t controller_index[5];
}cl_rf_door_lock_stat_t;
    
typedef struct {
    u_int8_t value; //�����Ŷ���
    u_int8_t info_type;
    u_int8_t is_valid; //�Ƿ���Ч
    u_int8_t pad;
    u_int32_t time_stamp; //ʱ���

	u_int8_t ex_type; //��չ����
	u_int8_t ex_value;//��չֵ
}cl_rf_door_history_t;

//������Ϣ
typedef struct {
    u_int8_t value; //�����Ŷ���
    u_int8_t info_type; //�������� 0�������� 1������ 2������3: ��ʱδ���� 4:�͵�������
    u_int8_t pad[2];
    u_int32_t time_stamp; //ʱ���
}cl_door_alarm_info_t;

typedef struct {
	u_int8_t don_enable;	// WIFI�����Զ�����ʹ��
	u_int8_t doff_enable;   // WIFI�Ͽ��Զ�����ʹ��
	u_int8_t don_starthour; // WIFI�����Զ�������ʼ��ʱ���
	u_int8_t don_endhour;  // WIFI�����Զ�����������ʱ���
	u_int8_t doff_starthour; // WIFI�Ͽ�������ʼ��ʱ���
	u_int8_t doff_endhour;   // WIFI�Ͽ�����������ʱ���
} cl_door_lock_wifilock_t;

typedef struct {
	u_int8_t id;	// ��Ӧң����id 1-5	
	u_int8_t state;	// ����ң����״̬��1 - ������2 - ��ʧ�� 3 - ɾ��
	u_int8_t pad[2];// ������
	u_int8_t name[16];	// �������Ҫ�������֣���0
} cl_door_lock_controller_set_t;

typedef struct {
	u_int8_t id;
	u_int8_t state;	// ����ң����״̬��1 - ������2 - ��ʧ�� 3 - ɾ��
	u_int8_t pad[2];
	u_int8_t name[16];
} cl_door_lock_controller_info_t;

	
typedef struct {
    cl_rf_door_lock_stat_t stat;
    cl_rf_door_history_t his[200];
    cl_door_alarm_info_t alarm_info;
	cl_door_lock_wifilock_t wifilock;	// wifi ����Ͽ�������
	u_int32_t controller_num;		// ң�������������5��
	cl_door_lock_controller_info_t controller[5];	// ң���� 
}cl_door_lock_info_t;



////////////////////////////////////////////////////
//�Ŵ�
    
//�Զ������ͳ���
enum{
    MODE_AUTO_GUARD_ON = 0x1,
    MODE_AUTO_GUARD_OFF
};
    
//�Ŵſ������� ʹ�� cl_rf_dev_com_ctrl ����
enum{
    RDM_CTRL_OPEN = 0x1, //����������
    RDM_CTRL_GRARD //���Ʋ�������
};

    
typedef struct {
    u_int8_t enable;
    u_int8_t start_hour;
    u_int8_t end_hour;
    u_int8_t type; //ʹ�� MODE_AUTO_GUARD_XX
}cl_rf_auto_guard_info_t;
    
typedef struct {
	bool stat_valid;	// �Ƿ���ȡ������
    u_int8_t battary; //���ʣ�������
    u_int8_t is_door_open; //�Ƿ���״̬��
    u_int8_t is_battary_warn; //��ص͵�������
    u_int8_t is_break_door; //�Ƿ������ƻ��Ŵ�
    u_int8_t is_guard; //�Ƿ��ڲ���״̬
    u_int8_t is_support_new_history;	// ֧���µ���ʷ��¼��ѯ
}cl_rf_door_magnet_stat_t;
    
typedef struct {
    cl_rf_door_magnet_stat_t stat;
    cl_rf_door_history_t his[200];
    cl_door_alarm_info_t alarm_info; //������Ϣ
    cl_rf_auto_guard_info_t auto_on; //�Զ�������Ϣ
    cl_rf_auto_guard_info_t auto_off; //�Զ�������Ϣ

	u_int8_t index_alarm;//sdkר�ã������ж��Ƿ���Ҫ��ѯ��
	u_int8_t index_autodefense;//sdkר��
	u_int32_t max_timestamp;//sdkר�ã���Сʱ���

	u_int8_t send_num;//sdkר�ã����ͼ���
	u_int32_t send_timeout;//sdkר�ã����ͼ�����ʱ
}cl_door_magnet_info_t;

///////////////////////////////////////////////////
//�ȵ��龰���
#define CDQJMB_KEY_MAXNUM	(9)
#define CDQJMB_NAME_MAXLEN	(26)
//cache����
typedef struct {
	u_int32_t flag_len;//����ֽ���len
	u_int32_t hislog_cur_index;//��ǰ��־index
	u_int8_t hislog_count;//��־����
	u_int16_t pad;
	u_int8_t key_num;//��������
	u_int8_t ice_rule_maxnum;//����������������������������1��ʼ��
	u_int8_t ice_rule_curnum;//�豸��ǰ�洢����������������������1��ʼ��
	u_int8_t key_changed[CDQJMB_KEY_MAXNUM];//ÿ�������ı仯��
}cdqjmb_cache_t;

typedef struct {
	char name[CDQJMB_NAME_MAXLEN + 1];
}cl_cdqjmb_key_item_t;

typedef struct {
	u_int32_t flag;//��ʱû�ã�����
	u_int8_t key_num;//��������
	u_int8_t ice_rule_maxnum;//����������������������������1��ʼ��
	u_int8_t ice_rule_curnum;//�豸��ǰ�洢����������������������1��ʼ��
	cl_cdqjmb_key_item_t key_conf[CDQJMB_KEY_MAXNUM];//����������

	//sdkר�ã�app���Բ���
	bool is_valid;
	u_int8_t key_changed[CDQJMB_KEY_MAXNUM];//ÿ�������ı仯��
}cl_cdqjmb_info_t;


typedef struct {
	u_int8_t index;
	u_int8_t len;
	char name[0];
}cdqjmb_set_name_t;

///////////////////////////////////////////////////
// ���ո�Ӧ��
typedef struct {
	u_int32_t flags;
	u_int8_t battery;
	u_int8_t light_level;
	u_int16_t light_val;	
} light_sense_cache_t;

typedef struct {
	u_int8_t battery;	
	u_int8_t light_level;	// ���յȼ�
	u_int16_t light_val;		// ����ֵ
} cl_light_sense_stat_t;

typedef struct {
	cl_light_sense_stat_t stat;
} cl_light_sense_info_t;

///////////////////////////////////////////////////
//�����߿���
// ʹ�� cl_rf_dev_com_ctrl ���ƿ��أ���������Ϊ DHX_CTRL_ON_OFF
#define DHX_CTRL_ON_OFF 0x0

//���·��
#define DHX_MAX_NUM		(4)

//������Ƴ���
#define DHX_MAX_NAME_LEN	(32)

typedef struct {
    u_int8_t group_num; //�ж���·
    u_int8_t pad[3];
    u_int32_t on_off_stat; //��·����״̬����0bit��ʾ��һ·,0��ʾ�أ�1��ʾ��
}cl_dhx_switch_stat_t;

typedef struct {
	u_int8_t valid;
	u_int8_t name[DHX_MAX_NAME_LEN];
} cl_dhx_key_name_item_t;

typedef struct {
    cl_dhx_switch_stat_t stat;

	u_int8_t support_time;//0��֧�֣�1֧��
	u_int8_t support_name_set;//0��֧�֣�1֧��

	cl_dhx_key_name_item_t keys[DHX_MAX_NUM];

	//sdkר��
	u_int8_t init;
	u_int8_t time_cnum;
	u_int8_t index_key[DHX_MAX_NUM];
}cl_dhx_switch_info_t;

typedef struct {
	cl_dhx_key_name_item_t keys[DHX_MAX_NUM];
	u_int8_t index_key[DHX_MAX_NUM];
}cl_dhx_save_conf_t;

typedef struct {
	u_int32_t flagbits;
	u_int32_t pad;
	u_int8_t pad1[2];
	u_int8_t group_num;
	u_int8_t onoff;
} dhx_cache_old_t;

typedef struct {
	u_int32_t flagbits;
	u_int32_t pad;
	u_int8_t pad1[2];
	u_int8_t group_num;
	u_int8_t onoff;
	u_int8_t time_cnum;//��ʱ���仯��������1��ʼ��0����
	u_int8_t index_key[DHX_MAX_NUM];
} dhx_cache_t;
////////////////////////////////////////////////////
//��ʪ��̽��
enum {
	RF_TT_CMD_QUERY_CURVE  =  200,
	RF_TT_CMD_PUSH_CURVE  =  201,
};
    
#define MAX_HM_HISTORY_NUM 12
    
typedef struct {
    int8_t temp; //�¶� 7F��ʾ�õ�δ�ɼ�������
    u_int8_t hum;//ʪ�� 0-100 ,7F��ʾ��ʱ���δ�ɼ�������
}cl_hm_history_info_t;
    
typedef struct {
    int8_t cur_temp; //��ǰ�¶�
    bool support_temp100;	// ֧���¶�С����ʾ
    int16_t cur_temp100;	// ���֧��С����ʾ����ô�¶���ʾ�������ֵ,��λ0.01�ȣ�֧�ָ���
    u_int8_t cur_hum;//��ǰʪ�� 0-100
    u_int8_t battary; //���� 0-100
    u_int8_t is_low_battary_warn; //�Ƿ��ڵ͵�������״̬
    u_int8_t history_hour; //��ʾ���һ������ɼ���utc��ʱ�䣬���ƣ��磬10
    u_int8_t is_history_data_valid; //��ʷ�����Ƿ���Ч
    cl_hm_history_info_t his_data[MAX_HM_HISTORY_NUM];

	// Ӳ���汾
	u_int8_t hw_ver;
	// ����汾x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// svn��
	u_int32_t svn;

	// ��������SDKʹ��
	u_int8_t index_curve;
	u_int8_t index_alarm;
	bool stat_valid;
	int history_query_count;
}cl_hm_temp_hum_info_t;

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t temp;
	u_int8_t humi;
	u_int8_t index_curve;
} hm_temp_hum_cache_old_t;    

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t temp;
	u_int8_t humi;
	u_int8_t index_curve;
	// ������������
	u_int8_t index_alarm;
	int16_t temp100;
} hm_temp_hum_cache_t;    

////////////////////////////////////////////////////
//����̽��
//����������������
#define RDM_HM_CTRL_GUARD 0x1
// ���ñ���Ƶ�ʣ���λΪ5��
#define RDM_HM_CTRL_SET_FREQ 0x2  
typedef struct {
    u_int8_t battary; //���� 0-100 Ϊ������ʾ����(�򰲲�֧��) 103 ��ʾ��������104 ��ʾ����� 105��ʾ����״̬
    u_int8_t is_low_battary_warn; //�Ƿ��ڵ͵�������״̬
    u_int8_t is_guard; //�Ƿ��ڲ���״̬
    u_int8_t is_break;	// �Ƿ����쳣��
    u_int32_t detectd_num; //���һ�β�������̽�⵽�ж��ٴα���
    u_int32_t last_guard_time; //UTC�����һ�β���ʱ��
    cl_door_alarm_info_t alarm_info;//������Ϣ
    cl_alarm_time_t alarm_time;	// ������ʼ�������ͼ��ʱ��
    cl_rf_door_history_t his[200]; //������ʷ��¼��ֻ��ע�Ƿ���Ч�������Ч����̽�⵽���˾���

	u_int8_t is_support_new_history;	// ֧���µ���ʷ��¼��ѯ

	// Ӳ���汾
	u_int8_t hw_ver;
	// ����汾x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// svn��
	u_int32_t svn;
	// ��������SDK�Լ�ʹ��
	bool stat_valid;
	u_int8_t index_alarm;
	u_int8_t index_time;
}cl_hm_body_info_t;

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_time;	// ������ʼ�����ͼ��
	u_int16_t alarm_num;
} hm_body_detect_cache_t;

////////////////////////////////////////////////////
// ���ز���
typedef enum {
	ACT_KTCZ_ONOFF = 0x43,
} ACT_KTCZ_T;

typedef struct {
	u_int8_t onoff;
} cl_ktcz_stat_t;
// �ϲ��ȡ����ȫ��״̬��Ϣ
typedef struct {
	cl_ktcz_stat_t stat;
} cl_ktcz_info_t;

////////////////////////////////////////////////////
// ů����
// APP���ײ��������
typedef enum {
	//ACT_HEATING_VALVE_ONOFF = 0x81,
	ACT_HEATING_VALVE_MODE = 0X82,	// value 1 �Զ� 0 �ֶ�
	//ACT_HEATING_VALVE_ANTI_LIME = 0x83,
	//ACT_HEATING_VALVE_FROST_PROTECTION = 0x84,
	ACT_HEATING_VALVE_CHILD_LOCK = 0x85,
	ACT_HEATING_VALVE_WINDOW = 0x86,	// ���ڹ��� 0 ȡ�� 1ʹ��
	//ACT_HEATING_VALVE_SUMMER_WINTER = 0X87,
	ACT_HEATING_VALVE_DATE = 0x88,
	ACT_HEATING_VALVE_TEMP = 0x89,
	ACT_HEATING_VALVE_PERIOD = 0x8a,
	
} ACT_HEATING_VALVE_T;

enum {
	UP_TLV_GET_STATUS = 212,
	UP_TLV_GET_TIME = 213,
	UP_TLV_GET_CIRCLE = 214,
};

typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
} cl_heating_valve_param_date_t;

typedef struct {
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;
} cl_heating_valve_param_temp_t;

// ���ü��Ⱥ;������ڵ�4��ʱ���
typedef struct {
	u_int8_t day;	// ���ڼ���ȡֵ��Χ��1-7��
	u_int8_t resv;	
	u_int8_t hh1;	// ��һ�������� Сʱ ��ȡֵ��Χ��0 - 24��
	u_int8_t hm1;	// ��һ�������� ���� ��ȡֵ��Χ��0 - 50������10��
	u_int8_t eh1;	// ��һ�������� Сʱ��ȡֵ��Χ��0 - 24��
	u_int8_t em1;	// ��һ�������� ���ӣ�ȡֵ��Χ��0 - 50������10��
	u_int8_t hh2;	// �ڶ��������� Сʱ��ȡֵ��Χ��0 - 24��
	u_int8_t hm2;	// �ڶ��������� ���ӣ�ȡֵ��Χ��0 - 50������10��
	u_int8_t eh2;	// �ڶ��������� Сʱ��ȡֵ��Χ��0 - 24��
	u_int8_t em2;	// �ڶ��������� ���ӣ�ȡֵ��Χ��0 - 50������10��
} cl_heating_valve_param_period_t;

typedef struct {
	u_int8_t hh1;	// ��һ�������� Сʱ ��ȡֵ��Χ��0 - 24��
	u_int8_t hm1;	// ��һ�������� ���� ��ȡֵ��Χ��0 - 50������10��
	u_int8_t eh1;	// ��һ�������� Сʱ��ȡֵ��Χ��0 - 24��
	u_int8_t em1;	// ��һ�������� ���ӣ�ȡֵ��Χ��0 - 50������10��
	u_int8_t hh2;	// �ڶ��������� Сʱ��ȡֵ��Χ��0 - 24��
	u_int8_t hm2;	// �ڶ��������� ���ӣ�ȡֵ��Χ��0 - 50������10��
	u_int8_t eh2;	// �ڶ��������� Сʱ��ȡֵ��Χ��0 - 24��
	u_int8_t em2;	// �ڶ��������� ���ӣ�ȡֵ��Χ��0 - 50������10��
} cl_heating_valve_day_period_t;

typedef struct {
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t syn3;
	u_int8_t ver;
	u_int8_t cmd;
	u_int8_t plen;
	u_int8_t seq;
	u_int8_t checksum;
	//u_int8_t param[0];
} heating_valve_uart_hd_t;

#if 0
typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t window;
	u_int8_t against;
	u_int8_t frost;
	u_int8_t child_proof;
	u_int8_t summer_winter;
	u_int8_t battery;
	u_int16_t current_temp;
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;
} heating_valve_uart_stat_param_t;
#else
typedef struct {
	u_int8_t error;
	u_int8_t mode;
	u_int8_t windowfun;
	u_int8_t windowopen;
	u_int8_t child_proof;
	u_int8_t battery;
	u_int16_t current_temp;
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;
} heating_valve_uart_stat_param_t;

#endif
typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
} heating_valve_uart_time_t;

typedef struct {
	// ������Ϣ
	u_int8_t onoff;	// ����
	u_int8_t mode;
	u_int8_t window;
	u_int8_t against;	// ����
	u_int8_t frost;		// ����
	u_int8_t child_proof;
	u_int8_t summer_winter;	// ����
	u_int8_t battery;
	u_int16_t current_temp;
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;

	// ��λ��ʱ��
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;

	// 
	u_int8_t error;	// �����	
	u_int8_t windowfun;	// �Ƿ���������⹦��
	u_int8_t windowopen;	// ���Ƿ��
} cl_heating_valve_stat_t;

typedef struct {
	int seq;
	int recv_count;
	int total_count;
	cl_heating_valve_day_period_t day_period[7];
} heating_valve_day_period_cache_t;

typedef struct {
	u_int8_t slice;
	u_int8_t slice_idx;
} heating_valve_period_slice_hd_t;

typedef struct {
	u_int8_t mcu_stat_index;//״̬��ϢժҪ
	u_int8_t mcu_time_index;//ʱ����ϢժҪ
	u_int8_t mcu_circle_index;//�������ں;������� ժҪ
	u_int8_t quick_timer_index;//�����ʱ��ժҪ
} heaing_valve_cache_t;

// �ϲ��ȡ����ȫ��״̬��Ϣ
typedef struct {
	// ����״̬
	cl_heating_valve_stat_t stat;

	// һ������ÿ��ļ��Ⱥ;���ʱ��
	cl_heating_valve_day_period_t day_period[7];

	// �ڲ�����APP�ϲ�����
	heating_valve_day_period_cache_t dpc;
	heaing_valve_cache_t cache;
} cl_heating_valve_info_t;

////////////////////////////////////////////////////
// ͨ�õ�̽�������������� ˮ�е�
//�������� ʹ�� cl_rf_dev_com_ctrl ����
enum {
    ACT_COM_DETECTOR_CTRL_DEFENSE = 0x1, //���ų���
    ACT_COM_DETECTOR_CTRL_REPORT_FREQ = 0x2,	// �ϱ�Ƶ�ʣ���Ҫ����5
    ACT_COM_DETECTOR_CTRL_ALARM_TYPE = 0x3,		// ���ñ�����ʽ0 :������һֱ����ֱ��APP ȷ�� 1 ֻ����һ��
    ACT_COM_DETECTOR_CTRL_ALARM_DEMO = 0x4,		// ������ʾ
};

enum {
	CD_ALARM_INFO_TYPE_DEV,			// �豸����
	CD_ALARM_INFO_TYPE_BATTERY,		// ��ع��ͱ���
	CD_ALARM_INFO_TYPE_ALARM_PAUSE, // ������ͣ
} ;

//������Ϣ
typedef struct {

    u_int8_t value; 
	/*
		 ��ʷ��Ϣ����
		 	0: �豸����/�����ָ�������value 1��ʾ���� 0��ʾ�����ָ�
		 	1: �������ͱ���/�����ָ�,����value 1��ʾ�������ͱ�����0��ʾ�����ָ� 
		 	2: ������ͣ/�������¿�ʼ,����value 1��ʾ������ͣ, 0��ʾ�������¿�ʼ
	*/
    u_int8_t info_type;
    u_int8_t pad[2];
    u_int32_t time_stamp; //ʱ���
} cl_com_detector_alarm_info_t;

typedef struct {
	// Ӳ���汾
	u_int8_t hw_ver;
	// ����汾x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// ���ʣ�������103��ʾ�������ͣ�102��ʾ��������
	u_int8_t abc_battery;
	u_int8_t is_alarm;	// �Ƿ����ڱ��� 1 ���ڱ��� 0 �����ָ�
	u_int8_t is_low_battery;	// ����Ƿ�͵��� 1��ʾ��ص����ͱ�����0��ʾδ����
	u_int8_t is_defence;	// �Ƿ��ڲ��� 1 �� 0 ��
	u_int8_t is_pause_alarm;	// �Ƿ�����ͣ���� 1 ��ͣ 0 δ��ͣ
	u_int8_t is_alarm_once;	// 0 һֱ���ͱ���ֱ��APPȷ�� 1 ֻ����һ��

	u_int8_t is_support_new_history;	// ֧������ʷ��¼��ѯ
	u_int32_t svn;	// svn��

	// ��������SDK�Լ�ʹ��
	u_int8_t index_alarm;
	u_int8_t index_time;
	u_int8_t stat_valid;
} cl_com_detector_stat_t;

typedef struct {
	u_int8_t value; // �������;���ֵ����
	/*
		 ��ʷ��Ϣ����
		 	0: �豸����/�����ָ�������value 1��ʾ���� 0��ʾ�����ָ�
		 	1: �������ͱ���/�����ָ�,����value 1��ʾ�������ͱ�����0��ʾ�����ָ� 
		 	2: ������ͣ/�������¿�ʼ,����value 1��ʾ������ͣ, 0��ʾ�������¿�ʼ
	*/
    u_int8_t info_type; 
    u_int8_t is_valid; //�Ƿ���Ч
    u_int8_t pad;
    u_int32_t time_stamp; //ʱ���
} cl_com_detector_his_t;


// �ϲ��ȡ����ȫ��״̬��Ϣ
typedef struct {
	cl_com_detector_alarm_info_t alarm_info;
	cl_com_detector_stat_t stat;
	cl_alarm_time_t alarm_time;	// ����ʱ�䣬������ʼ������ʱ�䣬���ʱ��
	cl_com_detector_his_t his[200];
} cl_com_detector_info_t;

//#if 1
typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_time;	// ������ʼ�����ͼ��
} com_detector_cache_t;

// ��ȩ������
enum {
	ACT_JQ_SET_ALARM_THR,
	ACT_JQ_SET_ALARM_PERIOD,
	ACT_JQ_FLUSH_CH2O,
};

enum {
	UP_TLV_JQ_SET_ALARM_THR = 201,
	UP_TLV_JQ_SET_ALARM_PERIOD = 202,	
	UP_TLV_JQ_FLUSH_CH2O = 203,
	UP_TLV_JQ_GET_CH2O_HISTORY = 204,
};


typedef struct {
	u_int8_t valid;	// �����Ƿ���Ч
	
	u_int16_t cur_ch2o;	// ��ǰŨ�ȣ���λppb (1ppb = 1ppm / 1000 = 0.0007466mg/m? = 0.7466��g/m?)
	u_int8_t battery;	// ��ص��� 0��20%��40%��60%��80%��100% 	   101��ʾ����δ֪
	u_int8_t std;	// ������ֵ��׼��0 = ���꣬1 = ŷ�꣬2 = �Զ���
	u_int16_t thr_ch2o; // �Զ��巧ֵ����λ��g/m3
	u_int16_t period;	// �����������λ�����ӣ�Ĭ��ֵΪ1����
	// ��������APP���ù���
	u_int8_t v_alarm;	
} cl_jq_stat_t;

typedef struct {
	u_int32_t time;	// �ɼ���ʱ���
	u_int16_t ch2o;	// Ũ�ȣ�PPB
} cl_jq_history_item_t;

#define MAX_JQ_HISTORY_ITEM_NUM 31

typedef struct {
	cl_jq_stat_t stat;
	int n_item;	// �ж�������ʷƽ����ȩŨ�ȼ�¼
	cl_jq_history_item_t items[MAX_JQ_HISTORY_ITEM_NUM];	// ƽ����ȩŨ�ȼ�¼
	
	// ��������APP���ù���
	u_int8_t v_ch2o_history;//��ʷ��¼�仯����
	u_int8_t query_map[MAX_JQ_HISTORY_ITEM_NUM];	// ƽ����ȩŨ�ȼ�¼
} cl_jq_info_t;

typedef struct {
	u_int8_t len;
	u_int8_t battery;
	u_int16_t cur_ch2o;
	u_int8_t std;
	u_int8_t v_alarm;
	
	u_int16_t thr_ch2o;
	u_int16_t alarm_period;

	u_int16_t hislog_count;
	u_int32_t hislog_index_current;

	u_int8_t v_ch2o_history;
	u_int8_t n_ch2o_history;
} jq_cache_t;



//#endif
////////////////////////
// ��̩��������
//#define HTLLOCK_MAX_USER_NUM 32
//�û����������
#define HTLLOCK_MAX_USER_NUM 210
#define HTLLOCK_MAX_NOTICE_TYPE_NUM 16
#define HLLLOCK_MAX_TRY_QUERY 10

// ����͸����TLV����ö��
enum {
	HTLLOCK_RAW_DATA = 4,		// ͸������
	HTLLOCK_USER_MANAGE = 50,	// �û�����
	HTLLOCK_INFO_NOTICE = 51,	// ��Ϣ����	
};


enum {
	ACT_HTLLOCK_ADMIN_LOGIN = 51,
    ACT_HTLLOCK_USER_MANAGE_SET_NAME,
    ACT_HTLLOCK_USER_MANAGE_SET_PIC,
    ACT_HTLLOCK_USER_MANAGE_SET_BIND,
    ACT_HTLLOCK_USER_MANAGE_SET_UNBIND,
    ACT_HTLLOCK_USER_MANAGE_SET_REMIND_ONOFF,
    ACT_HTLLOCK_SET_INTICE_INFO,
    ACT_HTLLOCK_GET_HISTORY,    
    ACT_HTLLOCK_SET_PIN,	// ��ʱPIN������
    ACT_HTLLOCK_SET_VOL = 100,	// ��������1 ���� 2 ����3���� 4 ����
    ACT_HTLLOCK_SET_LANG,	// ��������1 ���� 2Ӣ��
    ACT_HTLLOCK_LOCAL_OPEN,	// ����������
    ACT_HTLLOCK_QUERY_PIN,	// ��ѯ��ʱPIN
};

// cache����
typedef struct {
	u_int32_t devstat_bits;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t index_notice;
	u_int8_t index_alarm;
	u_int8_t resv1;
	u_int16_t index_user;
	u_int16_t user_count;
} htllock_tt_cache_t;

//�����Ŵ�flagλ����
#define 	DORR_MANAGE_V2_FLAG_ONOFF		(BIT(0))
#define 	DORR_MANAGE_V2_FLAG_ALARM		(BIT(1))
#define 	DORR_MANAGE_V2_FLAG_BREAK_DOOR	(BIT(2))
#define 	DORR_MANAGE_V2_FLAG_SECURITY	(BIT(3))

//�����Ŵ�cache����
typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_autodefense;
}door_manage_v2_cache_t;

typedef struct {
	u_int8_t start;
	u_int8_t pid;
	u_int8_t len;
	u_int8_t mcmd;
	u_int8_t scmd;
} htllock_uart_hdr_t;

typedef struct {
	u_int8_t checksum;
	u_int8_t end;
} htllock_uart_tail_t;

// ����Ա��½Э��
typedef struct {
	u_int8_t start;
	u_int8_t pid;
	u_int8_t len;
	u_int8_t mcmd;
	u_int8_t scmd;

	u_int16_t id;
	u_int8_t pwd[6];
	u_int8_t checksum;
	u_int8_t end;
} htllock_uart_cmd_login_t;

typedef struct {
	u_int8_t start;
	u_int8_t pid;
	u_int8_t len;
	u_int8_t mcmd;
	u_int8_t scmd;

	u_int8_t err;
	u_int8_t checksum;
	u_int8_t end;
} htllock_uart_cmd_login_reply_t;

typedef struct {
	u_int8_t doorstate;
	u_int8_t lockstate;
	u_int8_t deadbolt;	// ����״̬0 ���� 1 ����
	u_int8_t latchbolt;	// б��״̬0 ���� 1 ����
	u_int8_t fingerermain;
	u_int8_t cardremain;
	u_int8_t coderemain;
	u_int8_t battery;
	u_int8_t vol;
	u_int8_t lang;
} htllock_uart_cmd_stat_reply_t;

typedef struct {
	u_int8_t op; // �û��������Ϣ�����reply
	u_int8_t err; // 0-�����ɹ� 1-����ʧ�� 
	u_int16_t index;// �û����
} htllock_tt_set_reply_t;

typedef struct {
	u_int16_t id;
	u_int8_t pwd[6];
} htllock_tt_admin_login_t;


/*  �û�����*/

// ��һ���ֽ�OP�Ķ���
typedef enum {
	HTLLOCK_USER_MANAGE_OPT_TYPE_QUERY = 1,	// ��ѯ�û�����
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_NAME = 2,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_PIC = 3,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_BIND = 4,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_UNBIND = 5,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_REMIND_ONOFF = 8,
} HTLLOCK_USER_MANAGE_OPT_TYPE_T;

//query type
enum {
	HTLLOCK_USER_QUERY_BATCH = 1,
	HTLLOCK_USER_QUERY_ONE = 2,
	HTLLOCK_USER_QUERY_SUMMARY = 3,
};

typedef struct {
	u_int8_t op; // ��1
	u_int8_t resv;
} htllock_tt_user_manage_get_t;


#if 0
typedef struct {
	u_int8_t op; // �û�����Ķ�����1-��ѯ�û�����
	u_int8_t index_user;	// ��ǰϵͳά�����û��仯��Ŀ
	u_int16_t slice; // ��ѯ�û�ʱ�����ű�֤����0��ʼ���ϸ������ӣ����bitλΪ1��ʾ������ɡ�

	u_int16_t index; // �û����
	u_int16_t pindex; // ���û��ĸ����û��ı�š������0����ʾ���û����Ǹ����û��� �ֻ�app����ݸ�ֵ��Ӧ��ϵչʾ�����û��Ľ���
	/*
		
	�������û�ԭʼ��id��λ0-11��ʾ�û���ţ�
		λ12-15��ʾ�û����ͣ�1��ʾָ�����ͣ�2��ʾ�������ͣ�3��ʾ��Ӧ������
	*/
	u_int16_t create_id;
	u_int8_t pic_id; // �û�ͷ���Ӧ�ı�š� app���õ�ÿ��ͼƬ����Ӧ��ͼƬ��Ź̶���Ϊ0��ʾ"��ͷ��"
	/*
		
	��������
		���ҵ�������
		bit0�����û����п�������״̬�����Ƿ�رա� 1-�ر�״̬���� 0-���ر�
	*/
	u_int8_t flagbit;
	u_int8_t name[12]; // �û����ƣ�������12�ֽڣ�Ϊ0����ʾ"δ����"
} htllock_tt_user_manage_push_t;
#else
typedef struct {
	u_int8_t op; // �û�����Ķ�����1-��ѯ�û�����
	u_int8_t resv;
	
	
	u_int16_t index_user;	// ��ǰϵͳά�����û����ñ仯����
	u_int16_t get_useridex;	// �û�������ȡ���û���ţ�1��ʼ�� ��������û����index��;��һ����

	u_int16_t index; // �û����
	u_int16_t pindex; // ���û��ĸ����û��ı�š������0����ʾ���û����Ǹ����û��� �ֻ�app����ݸ�ֵ��Ӧ��ϵչʾ�����û��Ľ���
	/*
		
	�������û�ԭʼ��id��λ0-11��ʾ�û���ţ�
		λ12-15��ʾ�û����ͣ�1��ʾָ�����ͣ�2��ʾ�������ͣ�3��ʾ��Ӧ������
	*/
	u_int16_t create_id;
	u_int8_t pic_id; // �û�ͷ���Ӧ�ı�š� app���õ�ÿ��ͼƬ����Ӧ��ͼƬ��Ź̶���Ϊ0��ʾ"��ͷ��"
	/*
		
	��������
		���ҵ�������
		bit0�����û����п�������״̬�����Ƿ�رա� 1-�ر�״̬���� 0-���ر�
	*/
	u_int8_t flagbit;
	u_int8_t name[12]; // �û����ƣ�������12�ֽڣ�Ϊ0����ʾ"δ����"
} htllock_tt_user_manage_push_t;

#endif

typedef struct {
	u_int8_t is_close_stat_reminder;	// ���û����п�������״̬�����Ƿ�رա� 1-�ر�״̬���� 0-���ر�
	
	u_int16_t index; // �û����
	u_int16_t pindex; // ���û��ĸ����û��ı�š������0����ʾ���û����Ǹ����û��� �ֻ�app����ݸ�ֵ��Ӧ��ϵչʾ�����û��Ľ���
	/*
		
	�������û�ԭʼ��id��λ0-11��ʾ�û���ţ�
		λ12-15��ʾ�û����ͣ�1��ʾָ�����ͣ�2��ʾ�������ͣ�3��ʾ��Ӧ������
		create_id Ψһ��
	*/
	u_int16_t create_id;
	u_int8_t pic_id; // �û�ͷ���Ӧ�ı�š� app���õ�ÿ��ͼƬ����Ӧ��ͼƬ��Ź̶���Ϊ0��ʾ"��ͷ��"
	u_int8_t pad[3];
	u_int8_t name[12]; // �û����ƣ�������12�ֽڣ�Ϊ0����ʾ"δ����"
} htllock_user_manage_stat_t;

typedef struct {
	u_int8_t op; // �û�����Ķ�����2-�޸��û���
	u_int8_t resv;
	u_int16_t index;// �û����
	u_int8_t name[12];
} htllock_tt_user_manage_set_name_t;

typedef struct {
	u_int64_t sn;
	u_int16_t index;// �û����
	u_int16_t resv;
	u_int8_t name[12];
} htllock_tt_user_manage_set_name_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t index_type;
	u_int8_t num;
	u_int16_t get_index;
}httlock_bid_user_query_v2_t;

typedef struct {
	u_int64_t sn;
	u_int16_t id_index;
	u_int8_t pic_id;
	u_int8_t pad;
}httlock_bid_user_picid_set_v2_t;

typedef struct {
	u_int64_t sn;
	u_int16_t id_index;
	u_int16_t cindex;
}httlock_bid_user_bind_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t isclose;
	u_int8_t resv;
	u_int16_t id_index;
}httlock_bid_notice_v2_t;

typedef struct {
	u_int16_t id_index;
	u_int16_t pid_index;
	u_int16_t create_id;
	u_int8_t pic_id; 
	u_int8_t flagbit;
	u_int8_t name[12];
}httlock_bid_cache_user_item_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t index_type;
	u_int8_t resv;
	u_int16_t index_userconf;
	u_int16_t all_usernum;
	u_int16_t tmp_index;
	httlock_bid_cache_user_item_v2_t item[0];
}httlock_bid_cache_hd_v2_t;


typedef struct {
	u_int64_t sn;
	u_int8_t op;//op���û��仯�Ķ�����1-�����û�  2-ɾ���û�
	u_int8_t resv;
	u_int16_t create_id;
	u_int16_t id_index;
	u_int16_t pad;
}httlock_bid_user_adddel_v2_t;

typedef struct {
	u_int8_t op; // �û�����Ķ�����3-�޸��û�ͷ��
	u_int8_t pic_id;
	u_int16_t index;// �û����
} htllock_tt_user_manage_set_pic_t;

typedef struct {
	u_int8_t op; // �û�����Ķ�����4-�����û�
	u_int8_t resv;
	u_int16_t index;// �û����
	u_int16_t cindex;// �������ĺ����û��ı�� 
} htllock_tt_user_manage_set_bind_t;

typedef struct {
	u_int8_t op; // �û�����Ķ�����5-��������û�
	u_int8_t resv;
	u_int16_t index;// �û����
	u_int16_t cindex;// ����������ĺ����û��ı��  
} htllock_tt_user_manage_set_unbind_t;

typedef struct {
	u_int8_t op;
	u_int8_t isclose;
	u_int16_t index;
} htllock_tt_user_manage_set_remind_onoff_t;

typedef struct {
	u_int8_t op;
	u_int8_t resv;
	u_int16_t get_userindex;
} htllock_tt_user_manage_get_info_t;

/*  ��Ϣ����*/
// ��һ���ֽ�OP�Ķ���
typedef enum {
	HTLLOCK_INFO_NOTICE_OPT_TYPE_QUERY = 1,	// ��ѯ������Ϣ����
	HTLLOCK_INFO_NOTICE_OPT_TYPE_SET = 2,	// ����������Ϣ
	HTLLOCK_INFO_NOTICE_OPT_TYPE_GET_HISTORY = 3,	// ��ȡ��ʷ��Ϣ
} HTLLOCK_INFO_NOTICE_OPT_TYPE_T;


typedef struct {
	u_int8_t op; // ��1
	u_int8_t resv;
} htllock_tt_info_notice_get_t;

/*
	
op����Ϣ���ѵĶ�����1-��ѯ��Ϣ��������
	sbit_xx ��Ϣ��������bitͳһ��ʽ��
	  ���ҵ��������ţ�
	  �������ݣ�bit0-bit3����
		0-"app֪ͨ"��1-��������2-�͵�����3-������4-��δ����5-���󱨾���6-�ٳֱ�����7-				��е������
	  ���ѷ�ʽ��bit4-bit7����
		ȫ0-��Ҫ���ѡ�bit4-Ϊ1��ʾ֧��֪ͨ���ѡ�bit5-Ϊ1��ʾ֧���ڼ�����š�bit6-Ϊ1		  		��ʾ֧�ֶ������ѡ�bit7-Ϊ1��ʾ֧�ֵ绰����
*/
typedef struct {
	u_int8_t op;	// 
	u_int8_t index_notice;	// ��ǰ�豸ά����֪ͨ���ñ仯��
	u_int8_t sbit[8];
} htllock_tt_info_notice_push_t;

typedef struct {
	u_int8_t op;
	u_int8_t sbit_temp;
} htllock_tt_info_notice_set_t;

typedef struct {
	u_int8_t op;	// ��Ϣ���ѵĶ�����2-������Ϣ����
	u_int8_t err;
	u_int8_t sbit_tmp;
	u_int8_t resv;
} htllock_tt_info_notice_set_reply_t;


typedef struct {
	u_int8_t op;
	u_int8_t pad[3];
	u_int32_t timestamp;
} htllock_tt_info_notice_get_history_t;

enum {
	HTLLOCK_HISTORY_TYPE_LOCKONOFF = 1,
	HTLLOCK_HISTORY_TYPE_LOWAC,
	HTLLOCK_HISTORY_TYPE_FORCELOCK,
	HTLLOCK_HISTORY_TYPE_DOORISOPEN,
	HTLLOCK_HISTORY_TYPE_ERRORINPUT,
	HTLLOCK_HISTORY_TYPE_HIJACK,
	HTLLOCK_HISTORY_TYPE_MANDLE
};

typedef struct {
	u_int8_t op;
	/*
	
	0  bit: ��ʾ���ֶκͺ����ʱ����Ƿ���Ч��1-��ʾ��Ч��0-��ʾ��Ч
	1  bit: ����2-7bit��ʾ�Ĳ�ͬ���ͣ�0��1��ʾ�ĺ��岻ͬ
	2-7 bit: ��ʷ��Ϣ����
	   1����/���������� ����bit1��ֵ����Ϊ��1-��ʾ���� 0-��ʾ����
	   2���͵���������bit1��ֵ����Ϊ��1-��ʾ�͵��� 0-��ʾ����
	   3������������ ����bit1��ֵ����Ϊ��1-��ʾ���� 0-��ʾ����
	   4����δ���� ����bit1��ֵ����Ϊ��1-��ʾ��δ�� 0-��ʾ����
	   5�����󱨾��� ����bit1��ֵ����Ϊ��1-��ʾ���󱨾� 0-��ʾ����
	   6���ٳֱ����� ����bit1��ֵ����Ϊ��1-��ʾ�ٳֱ���0-��ʾ����
	   7����е�������� ����bit1��ֵ����Ϊ��1-��ʾ��е������0-��ʾ����
	*/
	u_int8_t info;
	u_int16_t id;
	u_int32_t timestamp;
} htllock_tt_info_notice_get_history_reply_t;

// ������ʱPIN
typedef struct {
	u_int16_t time;	// ��Чʱ�䣬��λ����(0xffff��ʾ������)
	u_int8_t cnt;	// ������ô���(0xff��ʾ������)
	u_int8_t pwd_len;	// ���볤�ȣ�ֻ����6�ֽ�
	u_int8_t pwd[6];	// ����
} htllock_tt_set_pin_t;

enum {
	HTLLOCK_INFO_NOTICE_TYPE_APP = 1,		// APP֪ͨ
	HTLLOCK_INFO_NOTICE_TYPE_LOCKONOFF = 2,	// ������
	HTLLOCK_INFO_NOTICE_TYPE_LOWAC = 3,		// �͵���
	HTLLOCK_INFO_NOTICE_TYPE_FORCELOCK = 4,	// ����
	HTLLOCK_INFO_NOTICE_TYPE_DOORISOPEN = 5,	// ��δ��
	HTLLOCK_INFO_NOTICE_TYPE_ERRORINPUT = 6,	// ���󱨾�
	HTLLOCK_INFO_NOTICE_TYPE_HIJACK = 7,	// �ٳֱ���
	HTLLOCK_INFO_NOTICE_TYPE_MANDLE = 8,	// ��е������
};

typedef struct {
	u_int8_t type;	// HTLLOCK_INFO_NOTICE_TYPE_XXX ��������
	u_int8_t support_remind;	// ֧��֪ͨ����
	u_int8_t support_trouble_free;	// ֧���ڼ������
	u_int8_t support_msg_remind;	// ֧�ֶ�������
	u_int8_t support_tel_remind;	//  ֧�ֵ绰����
} htllock_info_notice_stat_t;



typedef struct {
	u_int8_t onoff;	// ������״̬ 1 �� 0 ��
	u_int8_t vol;	// ����1 ��2 �� 3��4 ����
	u_int8_t lang;	// ���� 1 ����2 Ӣ��
	u_int8_t battery;	// ����0-100
} htllock_lock_stat_t;

typedef struct {	
	
    u_int8_t is_valid; //�Ƿ���Ч
    u_int8_t value; 
	/*
	  �������ͣ���ö��ֵ HTLLOCK_HISTORY_TYPE_XXX
	  1����/���������� ����value��ֵ����Ϊ��1-��ʾ���� 0-��ʾ����
	  2���͵���������value��ֵ����Ϊ��1-��ʾ�͵��� 0-��ʾ����
	  3������������ ����value��ֵ����Ϊ��1-��ʾ���� 0-��ʾ����
	  3������������ ����value��ֵ����Ϊ��1-��ʾ���� 0-��ʾ����
	  4����δ���� ����value��ֵ����Ϊ��1-��ʾ��δ�� 0-��ʾ����
	  5�����󱨾��� ����value��ֵ����Ϊ��1-��ʾ���󱨾� 0-��ʾ����
	  6���ٳֱ����� ����value��ֵ����Ϊ��1-��ʾ�ٳֱ���0-��ʾ����
	  7����е�������� ����value��ֵ����Ϊ��1-��ʾ��е������0-��ʾ����
	*/
    u_int8_t info_type;
    u_int8_t pad;
    u_int32_t time_stamp; //ʱ���
    /*
		�û�ԭʼ��id��
		λ0-11��ʾ�û���ţ�
		λ12-15��ʾ�û����ͣ�1��ʾָ�����ͣ�2��ʾ�������ͣ�3��ʾ��Ӧ������
	*/
   	u_int16_t create_id; 
} httlock_history_t;

// �ϲ��ȡ����ȫ��״̬��Ϣ
typedef struct {
	htllock_lock_stat_t lock_stat;		// ��״̬
	htllock_tt_set_pin_t last_pin;		// �ϴ����õ���ʱPIN���յ� SAE_HTLLOCK_PIN_REPLY ��ȥ��ȡ
	// �û���Ϣ
	htllock_user_manage_stat_t user_manage[HTLLOCK_MAX_USER_NUM]; // �û�������Ϣ	
	htllock_info_notice_stat_t info_notice[HTLLOCK_MAX_NOTICE_TYPE_NUM]; // ��Ϣ������Ϣ
	httlock_history_t history[200];
	
	// APP����������Щ
	u_int8_t get_cache;
	htllock_tt_cache_t cache;	// cache ��Ϣ

	u_int16_t index_user;	// ��cache��ȡ������user �仯��
	u_int16_t user_count;	// ��cache��ȡ������user����
	u_int16_t user_write_index;
	htllock_tt_user_manage_push_t user_slice[HTLLOCK_MAX_USER_NUM];
} cl_htllock_info_t;

#define USER_INFO_FLASH_MAGIC	0x6831
// �������ļ�����Ľṹ
typedef struct {
	u_int32_t magic;	// 0x6831
	u_int16_t index_user;
	htllock_user_manage_stat_t user_manage[HTLLOCK_MAX_USER_NUM]; // �û�������Ϣ
} htllock_user_info_flash_t;

//////////////////////////////////////////////////
//��տյ���
    
//ʹ��ͨ�ÿ���
enum{
    CT_WK_ONOFF = 0x0, //���ƿ���
    CT_WK_MODE, //����ģʽ
    CT_WK_TEMP, //����temp
    CT_WK_WIND, //���Ʒ���
    CT_WK_WIND_DIR, //���Ʒ���
    CT_WK_START_MATCH, //��������ƥ��
    CT_WK_STOP_MATCH //ֹͣ����ƥ��
};
    
enum {
    MS_WK_ERR_NONE = 0x0,
    MS_WK_ERR_DEV_NETWORK, //���豸���粻ͨ
    MS_WK_ERR_DEV_BUSY, //�豸æ������ƥ��������
    MS_WK_ERR_WAIT_DEV_RESP, //�ȴ��豸��Ӧ��ʱ
    MS_WK_ERR_DEV_WAIT_IR_TIME_OUT, //�ȴ���ң������ʱ
    MS_WK_ERR_DEV_TRANS_IR, //IR�źŴ���ʧ��
    MS_WK_ERR_SERVER_NETWORK, //�����������粻ͨ
    MS_WK_ERR_SERVER_RESP, //�ȴ���������Ӧ��ʱ
    MS_WK_ERR_SERVER_IR, //������˵��Ч�źŻ�ƥ��ʧ��
    MS_WK_ERR_SERVER_DOWNLOAD_CODE, //���غ������ʧ��
    MS_WK_ERR_IR_TO_DEVICE_TIMEOUT //���⴫�䵽�豸ʧ��
};

//�յ���tlv ,��210��ʼ
enum {
	UP_TLV_WKAIR_SHORTCUTS_QUERY = 210,
	UP_TLV_WKAIR_SHORTCUTS_SET = 211,
	UP_TLV_WKAIR_TEMP_ADJUST  =  212,
	UP_TLV_WKAIR_LED_MODE = 213,
	UP_TLV_WKAIR_DIR = 214,
	UP_TLV_WKAIR_CHECK = 215,
	UP_TLV_WKAIR_ADJUST = 216,
	UP_TLV_WKAIR_IR_QUERY = 217,
	UP_TLV_WKAIR_SHOCK_QUERY = 218,
	UP_TLV_WKAIR_STATUS_SYNC = 219,
	UP_TLV_WKAIR_ADDR_SET = 220,
	UP_TLV_WKAIR_GET_IR_CODE = 223,
	UP_TLV_WKAIR_SYNC_CTRL = 224,
};

//�յ���action
enum {
	ACT_WKAIR_DIR = 50,
	ACT_WKAIR_CHECK,
	ACT_WKAIR_DIR_ADJUST,
	ACT_WKAIR_SHOCK_ADJUST,
	ACT_WKAIR_SHOCK_QUERY,
	ACT_WKAIR_IR_QUERY,
	ACT_WKAIR_ADDR_SET,
	ACT_WKAIR_IR_SET,
	ACT_WKAIR_IR_SYNC,
};

typedef struct {
    u_int8_t cur_step; // ��ǰ���е��ڼ�����������ƥ��ʱ��Ч
    u_int8_t max_step; //�ܹ����ٲ���������ƥ��ʱ��Ч
    u_int8_t error; // ƥ����ִ���
    u_int8_t pad; //
    u_int16_t matched_id; // �������0��Ч
    u_int16_t num; // ƥ��յ�����ʹ��
    u_int16_t matched_ids[32]; //ƥ��յ�����ʹ��
}cl_rf_air_code_match_stat_t;
    
typedef struct {
    u_int16_t ir_id; // ��ǰ�����id
    u_int8_t battary; //������1-100��120Ϊδ֪
    int8_t room_temp; //���£�120Ϊδ֪
    u_int8_t room_humi; //ʪ�ȣ�120Ϊδ֪
    u_int8_t charge;//����ǣ�1������� 0��û�г��
    // AC_POWER_xxx
    u_int8_t onoff;
    // AC_MODE_XXX
    u_int8_t mode;
    // �յ��¶�
    u_int8_t temp;
    // ���٣�AC_WIND_xxx
    u_int8_t wind;
    // ����AC_DIR_xxx
    u_int8_t wind_direct;
	int8_t tmp_adjust;//�����¶�
	u_int8_t led_mode;//led��ģʽ
	//�յ���ַ
	u_int8_t addr;//1-16
	// ����ͬ������
	u_int8_t support_ir_sync_ctrl;	// ֧��ͬ�����ؿ���
	u_int8_t ir_sync_ctrl;	// 0 �ر� 1����
}cl_wk_air_work_stat_t;
    
//��� �յ���
typedef struct {
    cl_wk_air_work_stat_t stat; // ״̬��Ϣ
    cl_rf_air_code_match_stat_t match_stat; //ƥ����Ϣ
	
	u_int8_t ir_valid;//��ʾ�Ƿ���Ч���յ�������Ҫ�յ�tt����ܱ�ʾ��Ч
	u_int8_t shortcuts_count;//1-255,0��ʾ��֧�ֿ��ٶ�ʱ��
	u_int8_t status_count;//1-255
	u_int8_t comm_timer_count;//1-255,0��ʾ��֧��

	// ������SDK�Լ�ʹ��
	u_int8_t sync_num; // 1-255
	u_int8_t code[256];
	u_int8_t code_widx;
}cl_wk_air_info_t;

//�յ���cache����
typedef struct {
	u_int8_t shortcuts_count;//1-255,0��ʾ��֧�ֿ��ٶ�ʱ��
	u_int8_t status_count;//1-255
	u_int8_t comm_timer_count;//1-255,0��ʾ��֧��
	u_int8_t ir_num;//1-255������ѧϰ�������

    u_int8_t battary; //����
    u_int8_t room_temp;
    u_int8_t room_humi;
    u_int8_t onoff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t wind;
    u_int8_t wind_direct;

	u_int8_t addr;

	u_int8_t channel;
	u_int8_t power;
	u_int8_t sync_num;
	
	u_int8_t ir_sync_ctrl;
}wk_air_cache_t;

typedef struct {
	u_int16_t ir_id;
	u_int8_t sync_num;
	u_int8_t offset;
	u_int8_t pad[2];
} wk_air_ir_code_get_t;

typedef struct {
	u_int8_t ret;
	u_int8_t sync_num;
	u_int8_t offset;
	u_int8_t data[0];
} wk_air_ir_code_get_reply_t;


//�ȵ��龰���
enum {
	ACT_CDQJMB_NAME_SET = 1,
};

enum {
	UP_TLV_CDQJMB_NAME_SET = 200,
	UP_TLV_CDQJMB_NAME_QUERY = 201,
	UP_TLV_CDQJMB_NAME_PUSH = 202,
};

//�ǻʵ������
enum{
	ACT_ZHDJ_STATUS_SET = 1,
	ACT_ZHDJ_LOCATION_SET,
	ACT_ZHDJ_LOCATION_QUERY,
	ACT_ZHDJ_BIND,
	ACT_ZHDJ_TYPE,
	ACT_ZHDJ_DIR,
};

//�ǻʵ��tlv
enum {
	UP_TLV_ZHDJ_STATUS_SET = 210,
	UP_TLV_ZHDJ_LOCATION_SET = 211,
	UP_TLV_ZHDJ_LOCATION_GET  =  212,
	UP_TLV_ZHDJ_BIND = 213,
	UP_TLV_ZHDJ_TYPE = 214,
	UP_TLV_ZHDJ_DIR = 215,
};
//�ǻʵ������״̬
enum{
	ZHCJ_STATUS_OPEN = 0,
	ZHCJ_STATUS_CLOSE = 1,
	ZHCJ_STATUS_STOP = 3,
};

typedef struct {
	u_int32_t magic;
	u_int8_t index;
	u_int8_t type;
	u_int8_t pad[2];
}zhdj_bind_t;

//�ǻʵ��cache����
typedef struct {
	u_int32_t magic;//��������
	u_int8_t index;//0������1��ɴ
	u_int8_t status;//Open = 0 :����  STOP =1��ֹͣ  CLOSE = 3���ر�
	u_int8_t percent;//����λ�ðٷְ�
	u_int8_t type;
	u_int8_t max_timer_count;
	u_int8_t max_data_len;
	u_int8_t support_dir;//�Ƿ�֧��ת��,0��֧�֣�1��ʾ֧��
	u_int8_t dir;//0��ʾû����1��ʾ�ѻ���
	u_int32_t map;
	u_int8_t change_num[0];
}zhdj_cache_t;

typedef struct {
	u_int32_t magic;//��������
	u_int8_t index;//0������1��ɴ
	u_int8_t status;//Open = 0 :����  STOP =1��ֹͣ  CLOSE = 3���ر�
	u_int8_t percent;//����λ�ðٷְ�	
	u_int8_t type;//�������ͣ�0��������1������
	u_int8_t support_dir;//�Ƿ�֧��ת��,0��֧�֣�1��ʾ֧��
	u_int8_t dir;//0��ʾû����1��ʾ�ѻ���
}cl_zhdj_info_t;

// �����Ƽ�

enum {
	RF_DEV_DWKJ_TT_TYPE_SET_TIMER_POINT = 211,
	RF_DEV_DWKJ_TT_TYPE_QUERY_TIMER_POINT = 212,
} ;

enum {
	ACT_DWKJ_ONOFF, // 0 �ػ� 1����
	ACT_DWKJ_PERCENT,	// ����
	// ����ACTION �ڽӿ�cl_dwkj_sample_ctrl ʹ��

	ACT_DWKJ_TIMER,	// ��ʱ������
};

typedef struct {
	u_int8_t syn;
	u_int8_t len;
	u_int8_t cmd;
} dwkj_uart_hdr_t;

typedef struct {
	u_int8_t onoff;	// 0 �ػ� 1����
	u_int16_t vol;	// ��ѹ����λV
	u_int16_t current;	// ��������λmA
	u_int16_t power;	// ���ʣ���λW
	u_int8_t percent;	// ����ٷֱ�ֵ������50����50%
	u_int32_t degree;	//�õ�������λ��ʱ

	/*		
		��������
		B5������״̬��1=���£�0=OK��
		B4����ѹ״̬��1=��ѹ��0=OK��
		B3��Ƿѹ״̬��1=Ƿѹ��0=OK��
		B2�����״̬��1=���������0=OK��
		B1����·״̬��1=��·��0=OK��
		B0����·״̬��1=��·��0=OK��
		ע����·�Ͷ�·����ʾΪ�ƹ��ϡ�
	*/
	u_int8_t error;	

	// ��������SDK�Լ���
	u_int8_t v_timer;
} cl_dwkj_stat_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t count;
	u_int16_t point[12];
} ucp_dwkj_timer_t;

typedef struct {
	u_int16_t point;	// ��ʾһ����ĳһʱ��㣬����Ϊ5���ӣ�����287��ʱ���Ϊ287*5=1435����ʾÿ���23:55������Чȡֵ��ΧΪ0-287��0:00 - 23:55����������Чȡֵ��Χ��ֵ��Ϊ��Ч
	u_int8_t level;		// ��ʾ�����Ƶĵ�λֵ����Ч��ΧΪ1��2��1��ʾ������2��ʾ�ػ�������50-110������50����Ӧ50%������������Ч��Χ��ֵ��Ϊ��Ч
} cl_dwkj_timer_item_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t count; // �ж��ٸ���ʱ��������count Ϊ5����ʾ�±�0 - 4 ������Ч
	cl_dwkj_timer_item_t item[12];
} cl_dwkj_timer_t;

typedef struct {
	cl_dwkj_stat_t stat;	// ����״̬

	// ��ʱ��
	cl_dwkj_timer_t timer;	// һ��12����ʱ��

	// ��������SDK�Լ���
	cl_dwkj_timer_t timer_bk;	// ����timer���յ��豸����������ʧ�ܺ󣬻�ԭ֮ǰ��
} cl_dwkj_info_t;

typedef struct {
	dwkj_uart_hdr_t hd;
	u_int8_t pad1[3];	// ��ʱ����
	u_int8_t onoff;	// 0 �ػ� 1����
	u_int16_t vol;	// ��ѹ����λV
	u_int16_t current;	// ��������λmA
	u_int16_t power;	// ���ʣ���λW
	u_int8_t percent;	// ����ٷֱ�ֵ������50����50%
	u_int32_t degree;	//�õ�������λ��ʱ
	u_int8_t pad2;	// ��ʱ����
	u_int8_t error;	
	u_int8_t pad3[5];	// ��ʱ����
	u_int8_t tail;
	
	u_int8_t v_timer;	
	u_int8_t pad;
} dwkj_cache_t;

// �龰ң����

enum {
	RF_TT_SC_TYPE_SET_KEY = 200,
	RF_TT_SC_TYPE_QUERY_KEY = 201,
	RF_TT_SC_TYPE_PUSH_KEY = 202,
	RF_TT_SC_TYPE_SET_LOSS = 203,
};

enum {
	ACT_SC_SET_NAME,
	ACT_SC_SET_LOSS,	// ��ʧvalue��1��ʾ��ʧ ��0��ʾ�����ָ�
};

enum {
	DWKJ_CONF_ID_NAME = 1,	// ����ID����
	COMM_TIMER_CONFI_ID,
	CDQJMB_CONF_ID,
	DHX_CONF_ID,
};

//ħ�ֵ�����
enum {
	RF_TT_DHX_TYPE_SET_KEY = 200,
	RF_TT_DHX_TYPE_QUERY_KEY = 201,
	RF_TT_DHX_TYPE_PUSH_KEY = 202,
};

enum {
	ACT_DHX_SET_NAME,
};

typedef struct {
	u_int8_t valid;
	u_int8_t id;
	u_int8_t idx;
	u_int8_t name[32];
} dwkj_flash_key_t;


typedef struct {
	u_int8_t id;
	u_int8_t len;
	u_int8_t name[0];
} ucp_scene_controller_key_set_t;


typedef struct {
	u_int8_t id;
	u_int8_t len;
	u_int8_t name[0];
} ucp_scene_controller_key_push_t;

typedef struct {
	u_int8_t valid;	// �Ƿ���Ч
	char name[32];
} scene_controller_key_item_t;

typedef struct {
	u_int8_t id;
	u_int8_t name[24];
} cl_scene_controller_key_set_t;

typedef struct {
	u_int8_t hw_ver;
	u_int8_t soft_ver;
	u_int16_t resv;
	u_int32_t svn;
} ucp_scene_controller_version_t;


typedef struct {
	u_int8_t is_low_battery;
	u_int8_t abc_battery;
	// ������Ϣ���ֱ��ӦID 1 2 3 4
	scene_controller_key_item_t keys[4];

	// Ӳ���汾
	u_int8_t hw_ver;
	// ����汾x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// svn��
	u_int32_t svn;

	// �Ƿ��ڹ�ʧ״̬
	u_int8_t is_loss;

	// ��������SDK�Լ���
	u_int8_t stat_valid;
	u_int8_t index_alarm;
	u_int8_t index_key[4];
} cl_scene_controller_info_t;

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_key[4];
} scene_controller_cache_t;

//���ػ��澵��
typedef struct {
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t img_type;//1-macbee���豸app��2-macbee���豸rf  3��λ��
	u_int8_t up_action;// 1-ָ����������,2-ǿ����������
	u_int8_t index;//�±꣬1��ʼ
	u_int8_t reserve;
}cl_rfdev_img_cache_info_t;

typedef struct {
	u_int8_t img_num;
	u_int8_t reserve[3];
	cl_rfdev_img_cache_info_t cache[0];
}cl_rfdev_img_cache_query_t;


// ���ܱ�������
typedef struct {
	u_int8_t enable;
	u_int8_t pad[3];
	/*
		
	����sn�������ܱ�������������£������������豸����Ҫ�Ѹ�SN��װ���㲥���������У�
	ִ�б����豸�յ��㲥���ĺ����豸�ڲ������gw_sn���бȽϣ�һ����ִ�б����� 
	*/
	u_int64_t gw_sn;
} cl_rfdev_onekey_smart_alarm_t;

// һ�����ų���
typedef struct {
	u_int8_t defense;	// 1 ���ò��� 0 ����
	u_int8_t pad;		// ��ʷ���⣬������0xff��SDK����
} cl_rfdev_onekey_set_defense_t;

// һ�����ر���ִ���豸
typedef struct {
	u_int8_t enable;	// 0 �������� 1 ������ʼ
	/*
		
	Bit0��Ϊ0 -- Ĭ�ϱ���ģʽ���������ã�����ʱdata_valid�ֶ��Ժ��������Ч
	Bit0��Ϊ1 -- ��̬����ģʽ�����ݱ��ģ�����ʱdata_valid�ֶ��Ժ��������Ч
	*/
	u_int8_t data_valid;
	/*
		
	������ʽ��
	0 -- �������������ñ�����
	1 -- ����һ�Σ���ʱ��total_time��Ч
	2 -- ��Ъ������on_time��off_time����Ϊ��0����ʾһ�������ڵĿ�����ֹͣʱ��
			total_timeΪ0����ʾ�����������������Ա�������0���ʾ�ڱ���total_time�Ժ�ֹͣ 
	*/
	u_int8_t alarm_mode;

	u_int8_t on_time;	// һ�������ڵĿ���ʱ�䣬��λ�룬��Χ0-255
	u_int16_t off_time;	// һ�������ڵ�ֹͣʱ�䣬��λ�룬��Χ0-65535
	u_int16_t total_time;	// ����ʱ���ܳ��ȣ���λ�룬��Χ0-65535
} cl_rfdev_onekey_set_alarm_mode_t;

// ����RF���豸
typedef struct {
	u_int8_t onoff;
	u_int8_t pad[3];
} cl_rfdev_onekey_set_onoff_t;

typedef enum {	
	OKT_SET_DEFENSE = 37,		
	OKT_ONOFF = 52,
	OKT_SMART_ALARM = 61,
	OKT_ALARM_MODE = 62,
//	OKT_ALARM_MODE = 211,
} ONEKEY_TYPE_T;

// һ������
typedef struct {
	u_int8_t type;	// �������� OKT_XXX
	u_int8_t len;
	union {
		cl_rfdev_onekey_smart_alarm_t smart_alarm;
		cl_rfdev_onekey_set_defense_t set_defense;
		cl_rfdev_onekey_set_alarm_mode_t alarm_mode;
		cl_rfdev_onekey_set_onoff_t onoff;
	} ctrl;
} cl_rfdev_onekey_ctrl_t;

#pragma pack(pop)
    
/*
 ����:
	 �����Ŵ��Զ�����
 �������:
	 @dev_handle: �豸���
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_ctrl_auto_guard(cl_handle_t slave_handle,cl_rf_auto_guard_info_t* stat);

/*
 ����:
    ���Ƶ���״̬
 �������:
    @dev_handle: �����ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rf_lamp_ctrl_stat(cl_handle_t slave_handle,cl_rf_lamp_stat_t* stat);


/*
 ����:
 	���Ƶ�
 �������:
	 @dev_handle: �����ľ��
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_lamp_remote_ctrl_stat(cl_handle_t gw_handle,u_int32_t remote_id,u_int8_t key_id,cl_rf_lamp_stat_t* stat);

/*
 ����:
    ��������
 �������:
    @dev_handle: �����ľ��
    @group_id: ��handleΪ��ѡһ��ϵ,�ֱ��ʾ����豸������group_id��Ϊ0ʱ��dev_handle�����ص�id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rf_door_lock_ctrl(cl_handle_t dev_handle,u_int8_t group_id,u_int8_t type,u_int8_t action);

/////////////////////////////////////////////////
//��̩����
    
/*
 ����:
	 ��������
 �������:
	 @dev_handle: �����ľ��
	 @group_id: ��handleΪ��ѡһ��ϵ,�ֱ��ʾ����豸������group_id��Ϊ0ʱ��dev_handle�����ص�id
 	 @passwd������ʱ������ΪȫF������-1
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_yt_rf_door_lock_ctrl_lock(cl_handle_t dev_handle,u_int8_t group_id,u_int8_t on_off, u_int32_t passwd);

/*
 ����:
	 �޸�����
 �������:
	 @dev_handle: �����ľ��
 	 @old_passwd: ������ new_passwd: ������
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_yt_rf_door_lock_modify_lock_passwd(cl_handle_t dev_handle,u_int32_t old_passwd, u_int32_t new_passwd);

/*
 ����:
	 �½�����
 �������:
	 @dev_handle: �����ľ��
 	 @new_passwd: �½�������
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_yt_rf_door_lock_create_lock_passwd(cl_handle_t dev_handle, u_int32_t new_passwd);

/*
 ����:
	 ��̩��������ң����
 �������:
	 @dev_handle: �����ľ��
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_yt_rf_door_lock_set_associate(cl_handle_t dev_handle);

/*
 ����:
	 �����Ŷ��û�ؾͱ���
 �������:
	 @dev_handle: �����ľ��
 	 @enable: �Ƿ�������
 	 @timeout: ��ʱʱ�䣬��λ����
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rf_door_lock_set_unlock_timeout(cl_handle_t dev_handle, u_int8_t enable, u_int8_t timeout);


enum {
    RFDOORLOCK_SETTINGS_WIFIUNLOCK = 0x1,
    RFDOORLOCK_SETTINGS_WIFILOCK = 0x2,
    RFDOORLOCK_SETTINGS_SUPPORTPUSHNOTIFY = 0x3,
    RFDOORLOCK_SETTINGS_SUPPORTNOTIFDOOROPEN = 0x4
};
/*
 ����:
	 ����wifi������߶Ͽ��Ժ��Զ�������������
 �������:
	 @dev_handle: �����ľ��
 	 @type: 1 �����Զ����� 2 �����Զ�����
 	 @enable:  ʹ��
 	 @starthour: ��ʼ��ʱ��
 	 @endhour: ������ʱ��
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rf_door_lock_set_wifilock(cl_handle_t dev_handle, u_int8_t type, u_int8_t enable, u_int8_t starthour, u_int8_t endhour);

/*
 ����:
	 ��������ң����
 �������:
	 @dev_handle: �����ľ��
 	 @request: ���ò��������忴cl_door_lock_controller_set_t ����
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rf_door_lock_set_controller_info(cl_handle_t dev_handle, cl_door_lock_controller_set_t *request);

///*
// ����:
//    ��������ң��������
// �������:
//    @dev_handle: �����ľ��
// �������:
// ��
// ����:
// RS_OK: �ɹ�
// ����: ʧ��
// */
//CLIB_API RS cl_rf_door_lock_rc_ctrl(cl_handle_t slave_handle,cl_rf_door_lock_remote_t* lr);
    
/////////////////////////////////////////////////////////
    
// ͨ�ýӿ�
/*
 ����:
    ��ѯ��ʷ��Ϣ
 �������:
    @dev_handle: �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rf_dev_query_history(cl_handle_t slave_handle,u_int32_t last_time);
    
/*
 ����:
    ��������
 �������:
    @dev_handle: RF�豸�ľ��
    @group_id: ��handleΪ��ѡһ��ϵ,�ֱ��ʾ����豸������group_id��Ϊ0ʱ��dev_handle�����ص�id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rf_dev_com_ctrl(cl_handle_t slave_handle,u_int8_t group_id,u_int8_t group_type,u_int8_t ctrl_type,u_int8_t value);

/*
	����: ���ñ���ʱ�䣬����ɶʱ���ٿ�ʼɶ��
		
	�������:
		@slave_handle: �豸�ľ��
		@type: 1 : ���ñ�����ʼʱ��0 ���ñ�������ʱ��
		@time: �����Ӧtype״̬���ʱ�䣬��λ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_dev_set_alarm_time(cl_handle_t slave_handle, u_int8_t type, u_int32_t time);

/*
	����: ������Щһ��������ͻ�һֱ��Ӧ���豸��
	��Ҫ����һ������ȡ������
		
	�������:
		@slave_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_dev_set_alarm_clc(cl_handle_t slave_handle);

/*
 ����: �յ�����������
 
 �������:
 	@slave_handle: �豸�ľ��
 	@type CT_WK_XXX
 �������:
 	��
 ����:
 	RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_dev_air_ir_ctrl(cl_handle_t slave_handle,u_int8_t type,u_int8_t value);

//////////////////////////////////////////////////////////////////////////////////
// ů����
/*
	����: ů�����ļ򵥿��ƽӿ�
		
	�������:
		@slave_handle: �豸�ľ��
		@action: ��������(ACT_HEATING_VALVE_T)
			ACT_HEATING_VALVE_ONOFF ���ƿ��ػ�0 �ػ� 1����
			ACT_HEATING_VALVE_MODE ����ģʽ0 �Զ� 1�ֶ�
			ACT_HEATING_VALVE_ANTI_LIME  ��ʯ�� 0 ȡ�� 1ʹ��
			ACT_HEATING_VALVE_FROST_PROTECTION ˪������ 0 ȡ�� 1ʹ��
			ACT_HEATING_VALVE_CHILD_LOCK ͯ�� 0 ȡ�� 1ʹ��
			ACT_HEATING_VALVE_WINDOW 0 ȡ�� 1ʹ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_heating_valve_simple_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int8_t value);


/*
	����: ů���������ڿ��ƽӿ�
		
	�������:
		@slave_handle: �豸�ľ��
		@date: ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_heating_valve_date_ctrl(cl_handle_t slave_handle, cl_heating_valve_param_date_t *date);


/*
	����: ů�������¶����ýӿ�
		
	�������:
		@slave_handle: �豸�ľ��
		@date: ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_heating_valve_temp_ctrl(cl_handle_t slave_handle, cl_heating_valve_param_temp_t *temp);

/*
	����: ����ĳ��ļ��Ⱥ;���ʱ���
		
	�������:
		@slave_handle: �豸�ľ��
		@day: 	 ���ڼ�(ȡֵ1-7)
		@hh1;	 ��һ�������� Сʱ ��ȡֵ��Χ��0 - 24��
		@hm1;	 ��һ�������� ���� ��ȡֵ��Χ��0 - 50������10��
		@eh1;	 ��һ�������� Сʱ��ȡֵ��Χ��0 - 24��
		@em1;	 ��һ�������� ���ӣ�ȡֵ��Χ��0 - 50������10��
		@hh2;	 �ڶ��������� Сʱ��ȡֵ��Χ��0 - 24��
		@hm2;	 �ڶ��������� ���ӣ�ȡֵ��Χ��0 - 50������10��
		@eh2;	 �ڶ��������� Сʱ��ȡֵ��Χ��0 - 24��
		@em2;	 �ڶ��������� ���ӣ�ȡֵ��Χ��0 - 50������10��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_heating_valve_period_ctrl(cl_handle_t slave_handle, u_int8_t day, 
	u_int8_t hh1, u_int8_t hm1, 
	u_int8_t eh1, u_int8_t em1, 
	u_int8_t hh2, u_int8_t hm2, 
	u_int8_t eh2, u_int8_t em2);

////////////////////////////////////////////////////////////////////////////////////
// ���ز���
/*
	����:  ���ز����ļ򵥿��ƽӿ�
		
	�������:
		@slave_handle: �豸�ľ��
		@action: ��������(ACT_KTCZ__T)
			ACT_KTCZ_ONOFF ����ͨ�ϵ�0 �ϵ� 1ͨ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_ktcz_simple_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int8_t value);

//////////////////////////////////////////////////////////////////////////////////// ͨ��̽�����ӿ�
// 



/*
	����:  ͨ��̽�������ƽӿڣ�����cl_rf_dev_com_ctrl
		
	�������:
		@slave_handle: �豸�ľ��
		@group_id: ��ʱ����
		@group_type: ��ʱ����
		@type: �������ͣ���value����ʹ��
			ACT_COM_DETECTOR_CTRL_DEFENSE 0: ���� 1 ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/

//CLIB_API RS cl_rf_dev_com_ctrl(cl_handle_t slave_handle,u_int8_t group_id,u_int8_t group_type,u_int8_t type,u_int8_t value)


/*
	����:  һ�����ų����ӿ�
		
	�������:
		@master_handle:���صľ��
		@is_defense: 1 ���� 0 ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_dev_set_defense_batch(cl_handle_t master_handle, u_int8_t is_defense);

/*
	����:  һ�����ų����豸����
		
	�������:
		@master_handle:���صľ��
		@num: �����豸����
		@sn: ���õ�sn����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_dev_config_defense_batch(cl_handle_t master_handle, u_int8_t num, u_int64_t *sn);

/////////////////////////////////////////////////////////////////////////////////
// ��̩��
/*
	����:  �����û���½
	��Ҫ���������¼��ж��Ƿ�ɹ�
	SAE_HTLLOCK_ADMIN_LOGIN_OK = 1218,		// ��̩����½�ɹ�
	SAE_HTLLOCK_ADMIN_LOGIN_FAILED = 1219, // ��̩����½ʧ��
		
	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_admin_login(cl_handle_t slave_handle, htllock_tt_admin_login_t *request);

/*
	����:  ������ʱPIN��
	��Ҫ���������¼��ж��Ƿ�ɹ�
	SAE_HTLLOCK_SET_PIN_OK = 1237,	// ��̩������PIN��ɹ�
	SAE_HTLLOCK_SET_PIN_FAILED = 1238,	// ��̩������PIN��ʧ��
		
	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_set_pin(cl_handle_t slave_handle, htllock_tt_set_pin_t *request);

/*
	����:  �û���������
	��Ҫ���������¼����ж��Ƿ�ɹ�
	SAE_HTLLOCK_SET_NAME_OK = 1220,	// ��̩�������û����ֳɹ�
	SAE_HTLLOCK_SET_NAME_FAILED = 1221,// ��̩�������û�����ʧ��
		
	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���,op������
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_user_manage_set_name(cl_handle_t slave_handle, htllock_tt_user_manage_set_name_t *request);

/*
	����:  �û�����ͷ��
	��Ҫ���������¼����ж��Ƿ�ɹ�
	SAE_HTLLOCK_SET_PIC_OK = 1222,// ��̩�������û�ͷ��ɹ�
	SAE_HTLLOCK_SET_PIC_FAILED = 1223,// ��̩������ʧ��
		
	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���,op������
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_user_manage_set_pic(cl_handle_t slave_handle, htllock_tt_user_manage_set_pic_t *request);

/*
	����:  �û����ù���
	��Ҫ���������¼����ж��Ƿ�ɹ�
	SAE_HTLLOCK_SET_BIND_OK = 1224,// ��̩�������û���ָ�ơ����롢ɨ�迨�ɹ�
	SAE_HTLLOCK_SET_BIND_FAILED = 1225, // ��̩�������û���ָ�ơ����롢ɨ�迨ʧ��
		
	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���,op������
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_user_manage_set_bind(cl_handle_t slave_handle, htllock_tt_user_manage_set_bind_t *request);

/*
	����:  �û����ù���ȡ��
	��Ҫ���������¼����ж��Ƿ�ɹ�
	SAE_HTLLOCK_SET_UNBIND_OK = 1226,// ��̩������ȡ�������ɹ�
	SAE_HTLLOCK_SET_UNBIND_FAILED = 1227,// ��̩������ȡ������ʧ��
	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���,op������
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_user_manage_set_unbind(cl_handle_t slave_handle, htllock_tt_user_manage_set_bind_t *request);

/*
	����:  �޸��û�����������֪ͨ
	�������:
		@slave_handle: �豸�ľ��
		@is_close: �Ƿ�ر�����
		@user_index:�û����
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_user_manage_set_remind_onoff(cl_handle_t slave_handle, u_int8_t isclose, u_int16_t user_index);
/*
	����:  �û�������Ϣ
	��Ҫ���������¼����ж��Ƿ�ɹ�
	SAE_HTLLOCK_SET_INFO_NOTICE_OK = 1228, // ��̩��������Ϣ���ѳɹ�
	SAE_HTLLOCK_SET_INFO_NOTICE_FAILED = 1229, // ��̩��������Ϣ����ʧ��

	�������:
		@slave_handle: �豸�ľ��
		@request: ���Ʋ���,op������
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_notice_info_set(cl_handle_t slave_handle, htllock_tt_info_notice_set_t *request);


/*
	����:  һЩ�����Ŀ��ƣ�����

	�������:
		@request: ���Ʋ���,op������
		@action:
			    ACT_HTLLOCK_SET_VOL,	// ��������1 ���� 2 ����3���� 4 ����
			    ACT_HTLLOCK_SET_LANG,	// ��������
			    ACT_HTLLOCK_LOCAL_OPEN // ����������
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);

/*
	����:  ������ʷ���ݣ����ص���Ϣ������ cl_htllock_info_t��history����

	�������:
		@slave_handle: �豸�ľ��
		@request: �������,op �����ֻ��Ҫ��timestamp��ʹ�ñ���ʱ���
 	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rf_htllock_request_history(cl_handle_t slave_handle, htllock_tt_info_notice_get_history_t *request);
    
    
//////////////////////////////////////////////////////////////////////////////////
//// �յ���API

/*
	 ����:  ��ȡ�յ�������ƥ��״̬��Ϣ
 
	 �������:
	 	stat
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_get_air_code_match_stat(cl_handle_t dev_handle,cl_rf_air_code_match_stat_t* stat);
/*
	 ����:  ���ÿյ����¶Ƚ���ֵ
 
	 �������:
	 	tmp:-5~5���϶�
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_code_tmp_adjust(cl_handle_t slave_handle, int8_t tmp);

/*
	 ����:  ���ÿյ���led��ģʽ
 
	 �������:
	 	mode:1 ����ģʽ ,2 �ر�led��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_code_led_mode(cl_handle_t slave_handle, u_int8_t mode);

/*
	 ����:  ���ÿյ������ⷽ��sdk͸����app���壬���4������4��bit��ÿ��bit��λ����ʾ���塣
	 Bit0:1������� 0 �����ͺ���
 	 Bit1:1 ���ʷ��� 0 С���ʷ���	 
 
	 �������:
	 	dir:�������������
	 	len:�����С
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_code_dir(cl_handle_t slave_handle, u_int8_t *dir, u_int8_t len);

CLIB_API RS cl_set_air_ir_id(cl_handle_t slave_handle, u_int16_t id);

/*
	 ����:  ���ÿյ������ⷽ������Ӧ	 
 
	 �������:
	 	send_num:�������ʹ�����һ��4������
	 	send_inter:���ͼ������
	 	send_timeout:�������ͳ�ʱ��һ����ʱҪ*4
	 	len:�����С
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_code_dir_auto_adjust(cl_handle_t slave_handle, u_int8_t send_num, u_int8_t send_inter, u_int8_t send_timeout);

/*
	 ����:  ��鿪������ 
 
	 �������:
	 	on:1������0ֹͣ
	 	timeout:��鳬ʱ����
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_code_check(cl_handle_t slave_handle, u_int8_t on, u_int8_t timeout);

/*
	 ����:  ���Զ���鿪�ع��� 
 
	 �������:
	 	step: 0-֪ͨ�豸��һ�μ�飬1-֪ͨ�豸�ڶ��μ��
	 	onoff:1-����ǰ�յ����ڿ���״̬��0-����ǰ�յ����ڹػ�״̬
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_shock_auto_check(cl_handle_t slave_handle, u_int8_t step, u_int8_t onoff);


/*
	 ����:  ���Զ�����м�״̬��ѯ
 
	 �������:
		@slave_handle,���豸handle
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_shock_status_query(cl_handle_t slave_handle);

/*
	 ����:  �����Զ�����м�״̬��ѯ
 
	 �������:
		@slave_handle,���豸handle
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_ir_status_query(cl_handle_t slave_handle);

/*
	 ����:  �յ������Ƶ�ַ����
 
	 �������:
		@slave_handle,���豸handle
		@addr:1-16
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_ir_addr(cl_handle_t slave_handle, u_int8_t addr);

/*
	 ����:  �յ������ƺ���ͬ������
 
	 �������:
		@slave_handle,���豸handle
		@onoff: ���� 0 �� 1 ��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_air_ir_sync_onoff(cl_handle_t slave_handle, u_int8_t onoff);

//////////////////////////////////////////////////////////////////////////////////
//�ȵ����

/*
	 ����:  �ȵ��龰��尴����������
 
	 �������:
		@slave_handle,���豸handle
		@index:����index��1-6
		@name:��/0�������ַ���������Ϊ�գ��26�ֽ�
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_set_cdqjmb_name(cl_handle_t slave_handle, u_int8_t index, char *name);

//////////////////////////////////////////////////////////////////////////////////
// ҹ�����ⱨ����
/*
	 ����:  ҹ�����ⱨ������״̬����
 
	 �������:
	 	@slave_hande: ���豸handle
	 	@stat: ���忴�ṹ�嶨��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_yllight_lamp_ctrl(cl_handle_t slave_hande, cl_rf_lamp_stat_t* stat);


/*
	 ����:  ҹ�����ⱨ��������Ĭ�ϱ�����ʽ
 
	 �������:
	 	@slave_hande: ���豸handle
	 	@config: ���忴�ṹ�嶨��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_yllight_alarm_config(cl_handle_t slave_hande, cl_yllight_alarm_config_t* config);



/*
	 ����:  ҹ�����ⱨ����ͨ�ÿ���
 
	 �������:
	 	@slave_hande: ���豸handle
	 	@action: ACT_RF_YLLIGHT_XXX
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_yllight_com_ctrl(cl_handle_t slave_hande, u_int8_t action, u_int32_t value);


 
//////////////////////////////////////////////////////////////////////////////////
//// �ǻʵ��API
/*
	 ����:  �ǻʵ��������״̬����
 
	 �������:
	 	status:Open = 0 :����  Close =1��ֹͣ  Stop = 3���ر�
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_status_set(cl_handle_t slave_handle, u_int8_t status);

/*
	 ����:  �ǻʵ��������λ������
 
	 �������:
	 	location: 0~100,���Ǹ��ٷְ�
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_location_set(cl_handle_t slave_handle, u_int8_t location);

/*
	 ����:  �ǻʵ��������λ�ò�ѯ
 
	 �������:
		��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_location_query(cl_handle_t slave_handle);

/*
	 ����:  �ǻʵ����������
 
	 �������:
	 	magic:xxx
	 	index:xxx
	 	type:0/1
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_bind(cl_handle_t slave_handle, u_int32_t magic, u_int8_t index, u_int8_t type);

/*
	 ����:  �ǻʵ����������������
 
	 �������:
	 	type: 0/1
	 	index:xxx
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_type_set(cl_handle_t slave_handle, u_int8_t type, u_int8_t index);

/*
	 ����:  �ǻʵ����������������
 
	 �������:
	 	dir: 0��ʾû�л���1��ʾ�ѻ���
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_dir_set(cl_handle_t slave_handle, u_int8_t dir);


// �����Ƽ���

/*
	 ����:  �����Ƽ��ƵĻ�������
 
	 �������:
	 	type: 0/1
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);


/*
	 ����:  �����Ƽ��ƵĶ�ʱ������
 
	 �������:
	 	type: 0/1
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdj_timer_ctrl(cl_handle_t slave_handle, cl_dwkj_timer_t *request);



// �����Ƽ���

/*
	 ����:  �����Ƽ��ƵĻ�������
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@action: ACT_DWKJ_ONOFF ACT_DWKJ_PERCENT
	 	@value: ���actionʹ��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_dwkj_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);


/*
	 ����:  �����Ƽ��ƵĶ�ʱ������
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@request: ��ʱ������
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_dwkj_timer_ctrl(cl_handle_t slave_handle, cl_dwkj_timer_t *request);


/*
	 ����:  �龰ң�������ð�������
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@id: �龰ң��������ID��1-4
	 	@name: �龰ң�������ƣ�����\0�������24
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_scene_controller_set_key(cl_handle_t slave_handle, u_int8_t id, char *name);

/*
	 ����:  �龰ң������һЩ�������ƣ�Ŀǰ֧�ֹ�ʧ
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@action: ACT_SC_SET_LOSS value ��1��ʾ��ʧ��0��ʾ�ָ�����
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_scene_controller_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);



/*
	 ����:  ��ȩ���������÷�ֵ
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@type:��ֵ���� 0 ���� 1 ŷ��2 �Զ���
	 	@threshold:  �Զ��巧ֵ����λ��g/m3
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_jq_set_threshold(cl_handle_t slave_handle, u_int8_t type, u_int16_t threshold);

/*
	 ����:  ��ȩ���������ñ�����϶
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@peroid: ������϶����λ����
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_jq_set_alarm_period(cl_handle_t slave_handle, u_int16_t period);

/*
	 ����:  ��ȩ�������������һ�ε�ǰֵ
 
	 �������:
	 	@slave_handle: ���豸handle
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_jq_flush_ch2o(cl_handle_t slave_handle);

/*
	 ����:  ħ�ֵ���������·���Զ�������
 
	 �������:
	 	@slave_handle: ���豸handle
	 	@id:·����1-3 
	 	@name: �Զ������ƣ�����\0�������24
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_dhx_ml_set_key(cl_handle_t slave_handle, u_int8_t id, char *name);

/*
	 ����:  ���豸����ʱ���ѯ
 
	 �������:
	 	@slave_handle: ���豸handle
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rf_com_runtime_query(cl_handle_t slave_handle);

/*
	 ����:  �Դ��豸����һ������
	 �������:
	 	@gw_handle: ����handle
	 	@ctrl: ���Ʋ���
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_rfgw_onekey_ctrl(cl_handle_t gw_handle, cl_rfdev_onekey_ctrl_t *ctrl);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_COM_RF_DEV_H */

