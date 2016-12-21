#ifndef	__CL_SMART_APPLIANCE_H__
#define	__CL_SMART_APPLIANCE_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_rc.h"
    
//event
enum {
	SAE_BEGIN = 1200,
    SAE_INFO_MODIFY = 1201, //״̬�����仯
    SAE_AIR_CTRL_OK = 1202, // ���Ƴɹ���
    SAE_MODIFY_TIMER_OK = 1203, //�������޸Ķ�ʱ��
    SAE_MODIFY_PRICE_OK = 1204, //�޸ĵ��
    SAE_MODIFY_PERIOD_OK  = 1205, // �޸ķ塢�ȼ�������
    SAE_CTRL_LED_OK  = 1206, // ����LED
    SAE_DEL_TIMER_OK  = 1207, //ɾ����ʱ��
    SAE_SMART_CTRL_OK   = 1208,//���ܿ���
    SAE_CODE_MATCH_DEV_READY_OK  = 1209,//�豸׼����ƥ�� 1.��ƥ����׼���ý���ң�ذ���� 2.ȫƥ�����Ѿ���ʼƥ��
    SAE_CODE_MATCH_DEV_RECV_CODE  = 1210,//��ƥ��ģʽ�£��Ѿ����յ����룬��ʼ��ƥ�� 10
    SAE_CODE_MATCH_STAT_MODIFY  = 1211, //ƥ����������״̬�б仯
    SAE_CODE_MATCH_STOP_OK  = 1212, //ֹͣ����ɹ�
    SAE_CODE_MATCH_OK  = 1213,//ƥ�����ɹ�
    SAE_SET_NICK_NAME_OK  = 1214, //�޸��ǳƳɹ�
    SAE_DEV_POWER_NOTIFY  = 1215, //�豸���ʱ仯
    SAE_CTRL_LED_COLOR_OK  = 1216, // ����LED
    SAE_TT_IR = 1217, //͸�����ͻ��˵ĺ������
    SAE_TT_SOUND = 1217, //͸�����ͻ��˵�����

	SAE_HTLLOCK_ADMIN_LOGIN_OK = 1218,		// ��̩����½�ɹ�
	SAE_HTLLOCK_ADMIN_LOGIN_FAILED = 1219, // ��̩����½ʧ��
	SAE_HTLLOCK_SET_NAME_OK = 1220,	// ��̩�������û����ֳɹ�
	SAE_HTLLOCK_SET_NAME_FAILED = 1221,// ��̩�������û�����ʧ��
	SAE_HTLLOCK_SET_PIC_OK = 1222,// ��̩�������û�ͷ��ɹ�
	SAE_HTLLOCK_SET_PIC_FAILED = 1223,// ��̩������ʧ��
	SAE_HTLLOCK_SET_BIND_OK = 1224,// ��̩�������û���ָ�ơ����롢ɨ�迨�ɹ�
	SAE_HTLLOCK_SET_BIND_FAILED = 1225, // ��̩�������û���ָ�ơ����롢ɨ�迨ʧ��
	SAE_HTLLOCK_SET_UNBIND_OK = 1226,// ��̩������ȡ�������ɹ�
	SAE_HTLLOCK_SET_UNBIND_FAILED = 1227,// ��̩������ȡ������ʧ��

	SAE_HTLLOCK_SET_INFO_NOTICE_OK = 1228, // ��̩��������Ϣ���ѳɹ�
	SAE_HTLLOCK_SET_INFO_NOTICE_FAILED = 1229, // ��̩��������Ϣ����ʧ��

	SAE_HTLLOCK_SET_REMINDER_ONOFF_OK = 1230,	// ��̩���������ѿ��سɹ�
	SAE_HTLLOCK_SET_REMINDER_ONOFF_FAILED = 1231, // ��̩���������ѿ���ʧ��


	SAE_ZKCLEANNER_DAY_DATA = 1233,	// �пƾ�������ȡ���˵����������
	SAE_ZKCLEANNER_MONTH_DATA = 1234,	// �пƾ�������ȡ�������һ���²�������

	SAE_DWKJ_SET_TIMER_OK = 1235,	// �����Ƽ����ö�ʱ���ɹ�
	SAE_DWKJ_SET_TIMER_FAILD = 1236, // �����Ƽ����ö�ʱ��ʧ��

	SAE_HTLLOCK_SET_PIN_OK = 1237,	// ��̩������PIN��ɹ�
	SAE_HTLLOCK_SET_PIN_FAILED = 1238,	// ��̩������PIN��ʧ��

	SAE_HTLLOCK_UNLOCK_OK = 1239,	// ��̩�������ɹ�
	SAE_HTLLOCK_PIN_REPLY = 1240,	// ��ѯ������ʱPIN

	SAE_DEV_COMM_HISTORY_SUMMARY = 1241,	// ͨ����ʷ��¼��ժҪ��Ϣ����
	SAE_DEV_COMM_HISTORY_ITEM = 1242,	// ͨ����ʷ��¼�����ݸ���

    SAE_MODIFY_TIMER_FAILED = 1250, //16
    SAE_AIR_CTRL_FAILED = 1251,
    SAE_MODIFY_PRICE_FAILED = 1252,
    SAE_MODIFY_PERIOD_FAILED = 1253,
    SAE_CTRL_LED_FAILED = 1254,
    SAE_DEL_TIMER_FAILED = 1255,
    SAE_SMART_CTRL_FAILED = 1256,
    SAE_CODE_MATCH_START_FAILED = 1257,//23
    SAE_CODE_MATCH_STOP_FAILED = 1258,
    SAE_CODE_MATCH_FAILED = 1259,//ƥ�����ʧ��
    SAE_SET_NICK_NAME_FAILED = 1260,
    SAE_AIR_CTRL_NOT_MATCH = 1261, //���ư���ʱδƥ�����
    SAE_CTRL_LED_COLOR_FAILED  = 1262, // ����LED
    SAE_SMART_HOME_ON = 1263, //���ܻؼҿ����յ�
    SAE_SMART_HOME_CANCEL = 1264, //ȡ�����ܻؼ�(���ܻؼҿ����յ���һ��ʱ�俪δ����)

	SAE_LEARN_KEY_DEV_READY_OK = 1265, //�豸����ѧϰ״̬
	SAE_LEARN_KEY_DEV_BUSY = 1266, //�豸æ������ѧϰ��
	SAE_LEARN_KEY_WAIT_TIME_OUT = 1267, //�豸�ȴ��û���ң�س�ʱ
	SAE_LEARN_KEY_SUCCESSED = 1268, // �豸�ɹ�ѧϰ������

	SAE_SCENE_ID_MAX = 1269,//�龰�����������
	SAE_TIMER_TIME_CONFLICT = 1270,//��ʱ��ʱ���ͻ
	SAE_TIMER_ID_MAX = 1271,//��ʱ�������������
    
    SAE_LEARN_KEY_NEED_NEXT_KEY = 1272,//����ƥ��ʱ��Ҫƥ����һ������
    
    SAE_RF_DEV_ALARM_INFO = 1275,//RF�豸��������
    SAE_YT_LOCK_CTRL_OK = 1276,//�����ųɹ�
    SAE_YT_LOCK_SET_PASSWD_OK = 1277,//��������ɹ�
    SAE_YT_LOCK_CHANGE_PASSWD_OK = 1278, //�޸�����ɹ�
    SAE_YT_LOCK_CTRL_PASSWD_ERR = 1279,//������ʱ�������
    SAE_YT_LOCK_PASSWD_ERR = 1280, //�޸�����ʱ��ԭʼ�������
    SAE_YT_LOCK_CTRL_NO_PASSWD = 1281, //����ʧ�ܣ�δ���ÿ�������

	SAE_SHARE_COEE_GET = 1282,//��ȫ�����ȡ������ɹ����¼�
	SAE_YT_SN_ERR = 1283,//������������ʵ�ʻ��Ͳ�ƥ��
	SAE_RF_DEV_COMM_ALARM_INFO = 1284, // ͨ�õ�RF���豸������Ϣ����cl_rfdev_status_t ��cai��ȡ

	SAE_RF_DEV_COMM_HISTORY_SUMMARY = 1285,	// ͨ����ʷ��¼��ժҪ��Ϣ����
	SAE_RF_DEV_COMM_HISTORY_ITEM = 1286,	// ͨ����ʷ��¼�����ݸ���
    
    SAE_CODE_MATCH_START_DOWNLOAD_CODE = 1287, //ƥ����룺��ʼ�ӷ��������ر��룻
    
    SAE_CODE_MATCH_START_NOTIFY_DEV = 1288, //ƥ�����: ֪ͨ�豸ѧϰ����ɹ�

	SAE_RF_DEV_DOOR_LOCK_CONTROLLER = 1289,	// �а���ң���������豸��Ϣ�����last_controller_id�����ȡ
    
    SAE_CODE_MATCH_START_RECV_IR_CODE = 1290, //ƥ����룺 �豸�ѽ��յ�ң�������룬�ֻ�׼������

	SAE_RF_DEV_COMM_HISTORY_ITEM_V2 = 1291, // ���߼�����ʷ��¼�����¼�

	SAE_COMMON_CTRL_OK= 1297, //ͨ�õĿ��Ƴɹ���Ϣ
	SAE_COMMON_CTRL_FAILED = 1298, //ͨ�õĿ���ʧ����Ϣ
    
	SAE_END = SAE_BEGIN + 99
};
    
//ƥ�������̳���
enum{
    ERR_CM_WAIT_IR_SINGLE = 0x1, //�ȴ��û���ң�ذ峬ʱ
    ERR_CM_PROCESS_MATCHING = 0x2, // Ŀǰ�豸���ڴ���ƥ��
    ERR_CM_WAIT_SERVER_CODE =0x3, //�ȴ��ӷ�������ȡ���볬ʱ
    ERR_CM_NO_RESULT = 0x4,             //ƥ�����б��룬�޷�ƥ��
    ERR_CM_SEVER_NO_CONNECT = 0x5, //�豸δ���ӷ�����
    ERR_CM_TO_DEV_TIME_OUT = 0x10

};
// LED ��ɫ��
typedef enum {
	AIR_LED_DEFAULT_COLOR = 0,		// Ϩ��
	AIR_LED_RED_COLOR,			//��ɫ
	AIR_LED_GREEN_COLOR,		//��ɫ
	AIR_LED_BLUE_COLOR,			//��ɫ 
	AIR_LED_YELLOW_COLOR,		//��ɫ
	AIR_LED_PURPLE_COLOR,		//��ɫ
	AIR_LED_LIGHT_BLUE_COLOR,     //ǳ��ɫ
	AIR_LED_WHITE_COLOR			//��ɫ
}COLOR_T;


//һ��12����
#define MONTH_PER_YEAR  (12)

// ����ƥ��
#define CL_AIR_CODE_MATCH_STOP       0x0
#define CL_AIR_CODE_MATCH_CLOUD 0x1
#define CL_AIR_CODE_MATCH_ALL      0x2

// ģʽ
#define	CL_AC_MODE_AUTO	0
#define	CL_AC_MODE_COLD	1
#define	CL_AC_MODE_AREFACTION 2
#define	CL_AC_MODE_WIND	3
#define	CL_AC_MODE_HOT	4

// ����
#define	CL_AC_WIND_AUTO	0
#define	CL_AC_WIND_HIGH	1
#define	CL_AC_WIND_MIDD	2
#define	CL_AC_WIND_LOW	3
    
// ����
#define	CL_AC_DIR_AUTO	0
#define	CL_AC_DIR_1	1
#define	CL_AC_DIR_2	2
#define	CL_AC_DIR_3	3

//��ʽ���յ���ֵ
#define AC_CKEY_POWER 0
#define AC_CKEY_MODE_AUTO 1
#define AC_CKEY_MODE_COLD 2
#define AC_CKEY_MODE_AREFACTION 3
#define AC_CKEY_MODE_WIND 4
#define AC_CKEY_MODE_HOT 5
#define AC_CKEY_TEMP_UP 6
#define AC_CKEY_TEMP_DOWN 7
#define AC_CKEY_WIND_AUTO 8
#define AC_CKEY_WIND_LOW 9
#define AC_CKEY_WIND_MID 10
#define AC_CKEY_WIND_HIGH 11
#define AC_CKEY_DIR_MANUAL 12
#define AC_CKEY_DIR_AUTO 13



//�豸��֧�ֶ�ʱ�����20��
#define ID_TIMER_MAX	20

#pragma pack(push, 1)
typedef struct {
	u_int16_t ad;
	u_int16_t ad2;
}ad_data_t;
#pragma pack(pop)


//�����ն��豸(UDPͨѶģʽ��)
typedef struct{
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
}cl_air_work_stat_t;

typedef struct {
    u_int8_t id; //��ʱ��ID
    u_int8_t enable; //�Ƿ�����
    u_int8_t week; // ���ڼ��ظ�
    u_int8_t hour; //Сʱ
    u_int8_t minute; //����
    // APP��������true/false(trueΪ��), CLIB�ڲ���������AC_POWER_ON / AC_POWER_OFF
    u_int8_t onoff; //����
	u_int8_t repeat;
	u_int8_t reserved;
} cl_air_timer_t;
    
enum{
    PT_EXT_DT_UNKNOWN = 0x0,
    PT_EXT_DT_QPCP, /*ǧ�����̶�ʱ��*/
    PT_EXT_DT_808,
    PT_EXT_DT_101_OEM ,/*ɳ�ء����ֲ���*/
    PT_EXT_DT_QP_POT, //ǧ������ʱ��
    PT_EXT_DT_QP_PBJ, //ǧ���Ʊڻ���ʱ��
    PT_EXT_DT_HX_YSH, //��Ѹ��������ʱ��
    PT_EXT_DT_HTC_POOL, //�����Ӿ�ػ�
};
    
typedef struct {
    u_int8_t min_temp;
    u_int8_t max_temp;
    u_int8_t pad[2];
}cl_101_oem_timer_t;
    
/*ǧ�����̶�ʱ��˽������*/
typedef struct {
    u_int16_t id;
}cl_qp_timer_ext_t;
    
typedef struct {
    u_int8_t onOff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t fan_speed;
    u_int8_t fan_dir;
    u_int8_t key_id;
    u_int16_t pad;
}cl_808_timer_ext_t;

typedef struct {
    u_int16_t cook_id;
    u_int16_t cook_time;
    u_int8_t hot_degress;//����ǿ�ȣ��ȼ���
    u_int8_t microswitch; //΢�����ش���
    u_int8_t warm_temp; //�����¶�
    u_int8_t cooking_mode; //������� �� QPP_MODE_XXX
}cl_qp_pot_timer_t;

typedef struct {
    u_int16_t scene_id;
    u_int8_t pad;
}cl_qp_pbj_timer_t;
    
typedef struct {
    u_int16_t scene_id;
    u_int8_t temp;
    u_int8_t work_time;
    u_int8_t power;
    u_int8_t keep_temp;
    u_int8_t keep_time;
    u_int8_t pad;
}cl_hx_ysh_timer_t;
    
typedef struct {
	u_int8_t id;				/* ����ID */
	u_int8_t hour;			/* Сʱ 0-23 */
	u_int8_t minute;			/* ���� 0-59 */
	u_int8_t week;				/* bit 0-6λ��Ӧ�����쵽������ */
	u_int8_t enable;			/* �Ƿ���Ч(�ֻ�����) �����Ѿ���Ч(�豸����) */
    u_int8_t onoff;				/* �Ƿ��Ƕ�ʱ���� */
	u_int16_t duration;			/* ���������� */
    u_int16_t ext_data_type;    /*��չ�������� */
    union {
        cl_qp_timer_ext_t qp_time_info; //ǧ��������չ����
        cl_808_timer_ext_t air_timer_info; //808�յ���չ
        cl_101_oem_timer_t oem_101_timer_info; //ɳ�ء�������չ
        cl_qp_pot_timer_t qp_pot_timer_info; //ǧ������ʱ��
        cl_qp_pbj_timer_t qp_pbj_timer_info; //ǧ���Ʊڻ���ʱ��
        cl_hx_ysh_timer_t hx_ysh_timer_info; //��Ѹ��������ʱ��
    }pt_ext_data_u;
}cl_period_timer_t;

typedef struct {
    u_int8_t on_effect; //�´ο����Ƿ���Ч
    u_int8_t off_effect; //�´ιر��Ƿ���Ч
    u_int8_t timer_count; //���ٸ���ʱ��
    u_int8_t next_on_day;
    u_int8_t next_on_hour;
    u_int8_t next_on_min;
    u_int8_t next_off_day;
    u_int8_t next_off_hour;
    u_int8_t next_off_min;
    u_int8_t pad[3];
    u_int16_t on_minute; //�����´ο����ķ�����
    u_int16_t off_minute; //�����´ιرյķ�����
    cl_air_timer_t* timers; // ��ʱ���б�
    cl_period_timer_t* period_timers; //������ʱ��
}cl_air_timer_info_t;

typedef struct {
    // ��ʼʱ�䣬��λ�����ӣ�0:0����
    u_int16_t begin_minute;
    // �����೤����λ����
    u_int16_t last_minute;
}cl_peak_time_t;
    
typedef struct{
    u_int32_t month_peak[MONTH_PER_YEAR]; //���12�·�ֵ
    u_int32_t month_valley[MONTH_PER_YEAR]; //���12�¹�ֵ
    u_int32_t month_normal[MONTH_PER_YEAR]; //���12��ƽֵ
    cl_peak_time_t peak_time; //ÿ��ķ�ֵ����ʱ���
    cl_peak_time_t valley_time; //ÿ��Ĺȵ����ʱ���
    cl_peak_time_t flat_time; //ÿ��ƽ��ʱ���
    u_int32_t peak_price; // ��ֵ�۸�:��/��
    u_int32_t valley_price; // ��ֵ�۸�; ��/��
    u_int32_t flat_price; //ƽ��۸�: ��/��
}cl_elec_stat_info_t;

typedef struct {
    //���ܿ���
    u_int8_t on;
    //���Ϳ���
    u_int8_t push_on;
    //�����¶ȸ����������俪��
    u_int8_t sum_on ;
    //�����¶�����,Ĭ�ϣ���
    u_int8_t sum_tmp;
    //�����¶ȸ���xx�������ȿ���
    u_int8_t win_on;
    //����xx�ȣ�Ĭ�ϣ���
    u_int8_t win_tmp;
    //���ҿ����ǵ���ǰ�����ӿ��յ��Ŀ���
    u_int8_t home_on;
    u_int8_t pad[1];
}cl_smart_air_on_param_t;
    
typedef struct {
    //���ܹع��ܿ���
    u_int8_t on;
    //���Ϳ���
    u_int8_t push_on;
    // 1Сʱ��û�ˣ��Զ��ػ�(�ɸ�ʱ��0.5-3Сʱ����0.5СʱΪ��λ������ΧΪ��������Ĭ��Ϊ��)
    u_int8_t off_time;
    u_int8_t pad[1];
}cl_smart_air_off_param_t;

typedef struct {
    u_int8_t on;
    u_int8_t pad[3];
}cl_smart_air_sleep_param_t;

typedef struct{
	u_int32_t begin_time; //UTC ʱ��
	u_int32_t elec; //ͳ�Ƶĵ���
}cl_air_elec_item_info;

typedef struct{
	u_int8_t air_on_color;
	u_int8_t air_off_color;
	u_int16_t pad;
}cl_air_led_color_info;

typedef struct {
	u_int8_t is_info_valid;
	u_int8_t pad;
	u_int16_t days_count; //�ж���������
	u_int32_t nearest_data_time; //���һ����¼UTC ʱ��
	u_int16_t* elec_data; //��������
}cl_elec_days_stat_info;

typedef struct{
	u_int16_t c_id; //�������ID
	u_int8_t is_on; //���ػ� 
	u_int8_t mode;//ģʽ
	u_int8_t temp; //�¶�
	u_int8_t fan; //����
	u_int8_t fan_dir; //����
	u_int8_t key; //�ؼ��֣��豸��
}cl_ac_code_item_t;

typedef struct{
	u_int16_t cur_match_id; // ��ǰʹ�õ�id
	u_int16_t code_num; //���л���������
	cl_ac_code_item_t* items;
}cl_ac_code_match_info_t;

#define AC_PAN_CLOUD		0x0 //��ƥ�����
#define AC_PAN_WINDOW 	0x1 //������壬�̶�10������ AC_KEY_ID_POWER---AC_KEY_ID_TEMP_MODE_AUTO
#define AC_PAN_ONLY_ONOFF 0x2 //������壬�̶�2������ ����

enum{
	AC_KEY_ID_UN_KNOWN = 0x0, //δ֪��������ID
	AC_KEY_ID_POWER = 0x1,//��Դ��
	AC_KEY_ID_TEMP_ADD  = 0x2, // �¶ȼ�
	AC_KEY_ID_TEMP_DEC  = 0x3, //�¶ȼ�
	AC_KEY_ID_FAN_ADD  = 0x4, //����+
	AC_KEY_ID_FAN_DEC  = 0x5,//����-
	AC_KEY_ID_MODE_COLD  = 0x6, //����
	AC_KEY_ID_MODE_HOT  = 0x7, //����
	AC_KEY_ID_MODE_FAN  = 0x8, //�ͷ�
	AC_KEY_ID_MODE_ENERGY_SAVE  = 0x9 //����
};

//ɳ�ش�LED��ť type = AC_PAN_ONLY_ONOFF
enum{
    AC_KEY_ID_SAUDI_KNOWN = 0x0,
    AC_KEY_ID_SAUDI_OFF = 0x1, //�ػ�
    AC_KEY_ID_SAUDI_ON = 0x2 //����
};

typedef struct {
	// AC_POWER_xxx
	u_int8_t onoff;
	// AC_MODE_XXX
	u_int8_t mode;
	// 16 - 30
	u_int8_t temp;
	// ���٣�AC_WIND_xxx
	u_int8_t wind;
	// ����AC_DIR_xxx
	u_int8_t wind_direct;
	// ��ֵ��AC_KEY_xxx
	u_int8_t key;
} air_key_stat_t;


#define MAX_KEY_NAME_LEN 16
typedef struct{
	u_int8_t key_id;
	u_int8_t is_support_learn; //�Ƿ�֧��ѧϰ
	u_int8_t is_support_change_name; //�Ƿ�֧���޸�����
	u_int8_t is_support_delete; //�Ƿ�֧��ɾ���ð���
	u_int8_t is_learn_code; //�Ƿ��Ѿ�ѧϰ������
	u_int8_t is_snapshot_key;	// ��������Ƿ��ǿ���
	u_int8_t is_need_decode;	// ѧϰ�İ�����Ϣ�Ƿ���Ҫ�ڿ��Ƶ�ʱ�򷴽���������
	u_int8_t name[MAX_KEY_NAME_LEN];
}cl_air_key;

typedef struct{
	u_int8_t key_num; //��������
	cl_air_key* keys;	//������Ϣ

	// ���������յ� SAE_LEARN_KEY_SUCCESSED �¼���ȥ��ȡ
	u_int8_t stat_valid;	// �����״̬�Ƿ���Ч��״ֻ̬��ѧϰ��ʱ���豸ѧ���󷵻زŻḳֵ
	air_key_stat_t stat;	// ״̬����Ӧ���ػ�ģʽ�¶ȷ�������
}cl_no_screen_key_info;

//*************************************�¶��������ݽṹ********************************
//�¶�����flag��־����
enum{
    ACT_TC_INVALID, //��Ч
    ACT_TC_OFF, //����
    ACT_TC_COLD, //����
    ACT_TC_HOT, //����
    ACT_TC_AUTO, //�Զ�
    ACT_TC_AREFACTION,//��ʪ
    ACT_TC_WIND //�ͷ�
};
    
typedef  struct {
	u_int8_t  flag; //ģʽ
	u_int8_t  tmp;//�¶�
	u_int8_t   wind; //����
    u_int8_t dir; //����
}tmp_curve_t;

typedef struct {
	u_int8_t id; //����id������֧�ֶ�����
	u_int8_t enable;//�Ƿ�ʹ�ܸ�����
	u_int8_t week;//����0-6λ��ʾ�����յ�������
	u_int8_t begin_hour;//24Сʱ�еĿ�ʼʱ�䣬��0��ʼ
	u_int8_t end_hour;//24Сʱ�еĽ���ʱ��
	u_int8_t time_period;//ʱ���������ָ�1Сʱ����30����1Сʱ��Ϊ2���ڵ㣬��Ҫ60������
	u_int8_t count;//��time_period,being_hour, end_hour�����ʱ��������㷨:count=(end_hour - begin_hour + 1 + 24)%24*60/time_period
	u_int8_t pad;
	//tmp_curve_t tmp_curve[0];
}cl_temp_curve_t;

//***********************************�������¿��ƿյ�*******************************************************
typedef struct {
	u_int8_t enable;
	u_int8_t mode;
	u_int8_t temp_min;
	u_int8_t temp_max;
	u_int8_t week;
	u_int8_t begin_hour;
	u_int8_t end_hour;
	u_int8_t pad;
}cl_temp_ac_ctrl_t;
    
typedef struct {
    u_int8_t is_same_onoff_code; //�Ƿ񿪹ػ�ͬ��
    u_int8_t is_same_fan; // �Ƿ�ɨ��ͬ��
    u_int8_t is_fan_speed_opposite; //�Ƿ�����෴
    u_int8_t pad;
}cl_808_param_ajust_info;
    
typedef struct {
    u_int8_t is_valid;
    u_int8_t num;
    u_int8_t pad[2];
    u_int8_t data[24*6]; //144���㣬10����һ��
}cl_24hour_line;

enum{
    CHILD_LOCK_NONE = 0, //����
    CHILD_LOCK_ALL, //��ס���а���
    CHILD_LOCK_NO_ONOFF //��ס������������а���
};
    
typedef struct {
    u_int32_t phone_index; //��������ֻ����
    u_int32_t phone_share_time;//�����ȥ��ʱ��
    u_int32_t phone_operate_num;//�������ֻ������˶��ٴ��豸
    u_int32_t phone_last_operate_time; //�������ֻ����һ�β����豸��ʱ��
    u_int8_t phone_desc[16];//�������ֻ���������Ϣ
}cl_share_record_t;
    
typedef struct {
    u_int8_t is_share_data_valid; //���������Ƿ���Ч
    bool is_super_user;//�Ƿ��ǳ����ֻ�
    u_int8_t record_num; //�����¼����
    u_int8_t v1_remain_days; //������ʣ��ʱ��
    u_int32_t cur_phone_index; //��ǰ��½�ֻ�index
    cl_share_record_t* records; //�����¼
}cl_share_info_t;

// ��ݿ��ػ�
typedef struct {
	u_int8_t enable; // ���ܿ��� 0-�ر� 1-����
	u_int8_t onoff; // ���ػ��� 0-�ػ� 1-����
	
	u_int32_t time;	// localʱ���ֵ��������λ
	u_int32_t remain_time;//ʣ��ʱ�䣬������λ����app����time,remain_time���������ִ��ʱ��
} cl_shortcuts_onoff_t;

typedef struct {
	u_int8_t onoff;	// �ܿ���
	u_int8_t timer;	// ��ʱ������
	u_int8_t sync;	// ң����ͬ��
	u_int8_t temp_ctrl;	//  ���ܺ���
	u_int8_t curve;	//  �¶�����
	u_int8_t sleep;	// ����˯��
	u_int8_t smart_on;	// ���ܿ���
	u_int8_t poweron_restore;	// ������Զ��ָ�����
	u_int8_t linkage_ctrl;	// ��������
	u_int8_t resv[3];
} cl_ac_msg_config_t;


typedef struct{	 
	u_int32_t ck;	/*current k*/
	u_int32_t cad;	/*current ad*/
	u_int32_t cad2;	/*current ad2*/
	u_int32_t vk;	/*voltage k*/
	u_int32_t vb;	/*voltage b*/
}cl_air_pt_adkb_t;


//�յ�������Ϣ
typedef struct _cl_air_info_s{
    cl_handle_t handle; //�豸���
	
    u_int8_t air_led_on_off; /*LED*/
    u_int8_t room_temp;/*����*/
    u_int8_t smart_on_enable; /*���ܿ���*/
    u_int8_t smart_off_enable; /*���ܹػ�*/
    
    u_int8_t smart_sleep_enable; /*����˯��*/
	u_int8_t is_match_code_valid;//�Ƿ��Ѿ�ƥ���˱��룬�Ƿ���Ч
    u_int8_t is_match_code; //�Ƿ��Ѿ�ƥ���˱���
    u_int8_t is_old_air; // �Ƿ����Ͽյ�
    u_int8_t is_support_led_color;
   //�����������
    u_int8_t current_pan_type; //�������
    u_int8_t is_support_switch_pan; //�Ƿ�֧���л����
    u_int8_t is_support_key_learn;	// �Ƿ��������֧�ְ���ѧϰ���ܣ���������
    u_int8_t is_work_stat_data_valid; //����״̬�����Ƿ���Ч
    u_int8_t is_smart_on_data_valid; //���ܿ��������Ƿ���Ч


	u_int8_t is_support_learn_snapshort;	// ����֧��ѧϰ�Ϳ��չ��ܣ����������
    
    u_int8_t is_support_peroid_timer; //�Ƿ�֧��������ʱ��
    u_int8_t is_support_temp_curve;		//�Ƿ�֧���¶�����
    u_int8_t is_support_temp_ac_ctrl;//�Ƿ�֧�ָ����¶ȿ��ƿյ�
    u_int8_t is_support_peroid_ext_timer; //�Ƿ�֧����������չ��ʱ��
    
    u_int8_t is_support_utc_temp_curve;		//�Ƿ�֧���¶�����
    u_int8_t is_support_utc_temp_ac_ctrl;//�Ƿ�֧�ָ����¶ȿ��ƿյ�
    u_int8_t has_utc_temp_curve_data;
    u_int8_t has_utc_temp_ctrl_data;
    
    u_int8_t is_support_param_ajust;//����΢��
    u_int8_t is_support_room_temp_ajust;//����У��
    u_int8_t is_support_elec_ajust;//����У��
    u_int8_t temp_humidity; //ʪ�ȣ���ǿ����ղ���Ч
    
    int16_t env_room_temp_low; //���У������
    int16_t env_room_temp_high; //���У������
    int16_t  env_temp_ajust_value; //�����¶�У��ֵ �Ŵ�10��������������ʾ -5-5�ȣ���ֵ -50-50
    int16_t  elec_ajust_value; //����У��ϵ�� �Ŵ�100�� ��:������ʾ0.5-1.5����ֵΪ 50-150
    
    u_int8_t is_support_child_lock;//�Ƿ�֧��ͯ��
    u_int8_t child_lock_value; //CHILD_LOCK_XX
    
    cl_24hour_line room_temp_line; // 24Сʱ�¶�����
    cl_24hour_line humi_line; //24Сʱʪ������
    
    cl_808_param_ajust_info ajust_info;
    //��ǿ�����֧�ֵ��Ӻͻ�������Ϣ
    cl_pair_rc_info priv_rc;
   //////////////////////////
    u_int32_t cur_power; //��ǰ����
    ad_data_t ad;//adjust
    u_int32_t cur_milli_power; //��ǰ���ʣ�����
    cl_air_elec_item_info total_elec; //�ܵ���
    cl_air_elec_item_info period_elec; //�׶ε���
    cl_air_elec_item_info last_on_elec; //�ϴο�������
    cl_air_work_stat_t air_work_stat; //�յ���ǰ����״̬
    cl_air_timer_info_t air_timer_info; //�յ���ʱ����Ϣ
    cl_elec_stat_info_t air_elec_stat_info; //�յ�����ͳ����Ϣ
    cl_elec_days_stat_info elec_days_info; //����ͳ��֮365��ͳ������
    cl_smart_air_on_param_t smart_on_info; //���ܿ�������
    cl_smart_air_off_param_t smart_off_info; //���ܹرղ���
    cl_smart_air_sleep_param_t smart_sleep_info; //����˯�߲���
    cl_ac_code_match_info_t last_match_info; //����ƥ����Ϣ
    cl_air_led_color_info led_color; //���ܵ���ɫ
    cl_no_screen_key_info key_info;
    cl_share_info_t share_info;
	u_int8_t* requested_share_code;//�����ֻ�����ķ�����
	//ͨ���¶���������
	int temp_curve_len;	//ͨ���¶��������ݳ��ȣ���Ϊ����֧�ֶ�����ߣ�Ŀǰ����ֻ��һ��,len���count���Խ������������
	cl_temp_curve_t *temp_curve;//ͨ���¶���������
	//�����¶ȿ��ƿյ�
	cl_temp_ac_ctrl_t tac;
	u_int32_t id;
	//С��������
	u_int8_t scc_onoff_valid;
	u_int8_t scc_onoff;
	//������ѹУ��ֵ
	cl_air_pt_adkb_t adkb;
	u_int32_t smt_hard_ver; //��Ƭ��Ӳ���汾

	// ��������
	u_int8_t is_support_msg_config;	// ֧����������
	cl_ac_msg_config_t msg_config;
}cl_air_info_t;

typedef struct {
    u_int8_t action; // ��ǰ״̬ AIR_CODE_XX
    u_int8_t is_cloud_matching; // �Ƿ�������ƥ���У�ȫƥ����ֶ���Ч
    u_int8_t cur_step; // ��ǰ���е��ڼ���
    u_int8_t max_step; //�ܹ����ٲ�
    u_int8_t error; // ƥ����ִ���
    u_int8_t flag; //ƥ��ͬ�������,ע:ֻ��ƥ���������Ч��
}cl_air_code_match_stat_t;
    
typedef struct {
    u_int32_t cur_power; /*��ǰ����*/
}cl_air_real_time_data_t;

enum{
	ELEC_CLEAR_PERIOD,
	ELEC_CLEAR_LAST_ON,
	ELEC_CLEAR_TOTAL,
    ELEC_CLEAR_ALL_ELEC
};

//���ܲ����ĵ�����ѹƵ��
typedef struct{	 
	u_int32_t freq_current;
	u_int32_t k_current;
	u_int32_t freq_voltage;
	u_int32_t k_voltage;
}cl_plug_pt_adkb_t;



/***********ͨ�ö�ʱ��************/
//ͨ�ö�ʱ�����Ͷ��壬����Ǹ�appЭ�̵�����
enum {
	//������(�ɶ�ʱ��ҳ���Լ)
	UT_TYPE_ON = 1, //��ʱ��
	UT_TYPE_PERIOD_ONOFF = 3, //ʱ��� ��ʼʱ������ʱ��ν���ʱ�ر�
    
    
    /*�¶�ʱ��ҳ��ֻ�м�Լ��,�߼���,�׶�*/
    UT_TYPE_OFF = 2, //��ʱ��
	UT_TYPE_ADVANCE_TIMER = 4,//ʱ��� ��ʼʱ������ʱ��ν���ʱ�ر�, �߼���ʱ��,����ʱ���ڱ����¶�ģʽ���仯(�������յ�)
	UT_TYPE_ON_ADVANCE_TIMER = 5,//��ʱ��(�߼�)

    
	//������
	UT_TYPE_COMM_TYPE_CONST_TMP = 6,//ʱ��� ���¶�ʱ��
	
	UT_TYPE_COMM_MAX = 100,
};

//��ʱ������
enum {
	UT_TIMER_TYPE_ONOFF = 1,//���ض�ʱ��
	UT_TIMER_TYPE_TMP = 2,//�¶��ඨʱ���ȣ��Ժ���
};
/******************�ô���100��type��ʾ�������Լ��ģ�С��100����Ϊͨ��********************************************/
//ů������ʱ������
enum {
	UT_TIMER_HT_ONOFF = 1,//ů�������㿪��ʱ��
	UT_TIMER_HT_DURATION = 3,//ů����ʱ��ζ�ʱ��
	UT_TIMER_HT_CONST_TMP = 6,//ů�������¶�ʱ��
};

//�յ�����ʱ������
enum {
	UT_TIMER_WKAIR_ON = 1,//����
	UT_TIMER_WKAIR_OFF = 2,//����
	UT_TIMER_WKAIR_DURATION = 3,//ʱ���
	UT_TIMER_WKAIR_ADVANCE_1 = 4,//ʱ���
	UT_TIMER_WKAIR_ADVANCE_2 = 5,//ʱ���
};

typedef struct {
	u_int8_t mode;
	int8_t tmp;
}cl_comm_timer_zykt_t;

//�յ�����ʱ������
typedef struct {
	u_int8_t mode;
	int8_t tmp;
}cl_comm_timer_wkair_t;

//�ǻʵ����ʱ������
typedef struct {
	u_int8_t location;
}cl_comm_timer_zhdj_t;

//linkon�¿�����ʱ������
typedef struct {
	u_int8_t run_mode;//����ģʽ,0-���ȣ�1-���䣬2-����
	u_int8_t wind_speed;//���٣�0-�ͷ磬1-�з磬2-�߷�
	u_int16_t tmp;//�����¶ȣ�350��ʾ35��,��Χ50-350
	u_int8_t scene_mode;//�龰ģʽ��0-���£�1-���ܣ�2-���
}cl_comm_timer_linkon_t;

//ħ�ֵ����߶�ʱ������
typedef struct {
    u_int32_t on_off_stat; //ע:��16λΪmask��bitΪ1��ʾ��Ч����16λ��ֵ��bitΪ1��ʾ����//��·����״̬����0bit��ʾ��һ·,0��ʾ�أ�1��ʾ��
}cl_comm_timer_dhxml_t;

//�ǻʵ����߶�ʱ������
typedef struct {
	u_int8_t onoff;//bit��ʾ��1Ϊ����0Ϊ��
	u_int8_t mask;//bit��ʾ����ӦλΪ1��ʾ��Ч
}cl_comm_timer_zhdhx_t;

typedef struct {
	//���㶨ʱ�������¶�
	u_int8_t tmp_int;//��������
	u_int8_t tmp_dec;//С������

	//ʱ��ζ�ʱ�����ÿ�ʼ�¶ȣ������¶�
	u_int8_t start_tmp_int;//��������
	u_int8_t start_tmp_dec;//С������
	u_int8_t end_tmp_int;//��������
	u_int8_t end_tmp_dec;//С������

	//���¶�ʱ�������С�����¶�
	u_int8_t max_tmp_int;//��������
	u_int8_t max_tmp_dec;//С������
	u_int8_t min_tmp_int;//��������
	u_int8_t min_tmp_dec;//С������	
}cl_ct_heating_valve_t;

typedef struct {
	u_int8_t id;//��ʱ��id
	u_int8_t enable;//��ʱ���Ƿ�ʹ��
	u_int8_t type;//��ʱ�������ͣ��������Ҫ��app��sdk�������ж϶�ʱ����ʵ���߼�ҵ��
	u_int8_t hour;
	u_int8_t min;
	u_int8_t week;//Week: ���������ѭ����ʱ����bit7Ϊ1��bit0-6���������졢����һ������������ʾÿ�����Ǽ�����Ч,�����һ���Զ�ʱ����bit7Ϊ0��bit0-6���������졢����һ������������ʾ��һ����Ч
	u_int16_t duration;//ʱ��ζ�ʱ������ʱ��
	
	u_int8_t extened_len;//������չ���ݳ��ȣ�app������Բ���
	u_int8_t valid;//�����sdk�ڲ�ʹ��

	u_int32_t min_time;//�´�ִ�ж�ʱʱ�䣬sdkר��
	u_int8_t week_cal;//sdkר�ã����������´�ִ�ж�ʱ����
	union {
		u_int8_t data[10];
		cl_comm_timer_zykt_t zykt_timer;
		cl_comm_timer_wkair_t wkair_timer;
		cl_ct_heating_valve_t hv_timer;
		cl_comm_timer_zhdj_t zhdj_timer;//�ǻʵ�����ǻʴ����õ���չ����
		cl_comm_timer_linkon_t linkon_timer;
		cl_comm_timer_dhxml_t dhxml_timer;//ħ�ֵ�����
		cl_comm_timer_zhdhx_t zhdhx_timer;//�ǻʵ�����
    }extended_data_u;
}cl_comm_timer_t;

typedef struct {
	u_int8_t ip[16];	// �ַ���
	u_int8_t mask[16];	// �ַ���
	u_int8_t getway_ip[16];	// �ַ���
	u_int8_t main_dns[16];	// �ַ���
	u_int8_t sub_dns[16];	// �ַ�������ѡ������û���������ȫ��0
} cl_wan_static_t;

typedef struct {
	u_int8_t main_dns[16];	// �ַ���
	u_int8_t sub_dns[16];	// �ַ�������ѡ������û���������ȫ��0
	u_int8_t ip[16];	// �ַ�������ʾ��
	u_int8_t mask[16];	// �ַ�������ʾ��
	u_int8_t getway_ip[16];	// ����IP����ʾ��
} cl_wan_dhcp_t;

typedef struct {
	u_int8_t name[32];		// �˻�
	u_int8_t pwd[32];		// ����
	u_int8_t main_dns[16];	// �ַ���
	u_int8_t sub_dns[16];	// �ַ�������ѡ������û���������ȫ��0
	u_int8_t ip[16];	// �ַ�������ʾ��
	u_int8_t peer_ip[16];	// �ַ������Զ�IP����ʾ��
} cl_wan_pppoe_t;

// ���ڵ�ĳ����������
typedef struct {	
	u_int8_t network_type;	// 0 û������ 1 ��̬IP 2 DHCP 3 PPPOE
	u_int8_t pad[3];
	union {
		cl_wan_static_t config_static;	// ���wan_type Ϊ1��������ӿ�
		cl_wan_dhcp_t config_dhcp;		// ���wan_type Ϊ2��������ӿ�
		cl_wan_pppoe_t config_pppoe;	// ���wan_type Ϊ3��������ӿ�
		u_int8_t config_pad[128];		// ����
	} config;
} cl_wan_config_item_t;

// ���ڵ���Ϣ
typedef struct {
	u_int8_t index;		// �±꣬�ڼ���WAN��
	u_int8_t wan_phy_type;	// �������� 1 ���� 2 . 2.4G���ߣ��������ÿ��Բ���
	u_int8_t select_network_type;	// ѡ�������0-δ����  1-��̬IP  2-DHCP  3-pppoe
	u_int8_t pad;
	cl_wan_config_item_t config_item[3];	// ���������
} cl_wan_phy_item_t;

typedef struct {
	u_int8_t wan_num;	// �м������ڿ�������(���ߡ�����)
	u_int8_t pad[3];
	cl_wan_phy_item_t config[2];	// ���2������
} cl_wan_config_t;

// ����������
typedef struct {
	u_int8_t index;	// �����±�
	u_int8_t pad;
	u_int8_t wan_type;	// ��Ҫ���õ�����1-��̬IP  2-DHCP  3-pppoe
	u_int8_t pad1;
	union {
		cl_wan_static_t config_static;	// ���wan_type Ϊ1��������ӿ�
		cl_wan_dhcp_t config_dhcp;		// ���wan_type Ϊ2��������ӿ�
		cl_wan_pppoe_t config_pppoe;	// ���wan_type Ϊ3��������ӿ�
		u_int8_t config_pad[128];		// ����
	} config;
} cl_wan_request_config_t;

typedef struct {
	u_int8_t getway_ip[16];	// ����IP
	u_int8_t start_ip[16];	// ��ʼIP
	u_int8_t end_ip[16];	// ����IP
	u_int8_t mask[16];	// ����
	u_int32_t time;	// ���ڣ���λ����
} cl_dhcp_server_config_t;

typedef struct {
	u_int8_t ssid[36];
	u_int8_t pwd[68];
	u_int8_t is_enc;	// �Ƿ����0 ������ 1 ����(WAP2)
	u_int8_t pow;		// �ź�ǿ��	1 ����ģʽ 2 ��ͨģʽ 3 ��ǽģʽ
	u_int8_t channle_mode;	// �ŵ�ѡ��ģʽ 0 �Զ�ѡ�� 1 �ֶ�ѡ��
	u_int8_t channel;	// �ŵ�1-13
	u_int8_t enable;	// ���� 
} cl_ap_config_t;
/*
app��ʱ����ʾ�߼�:

timer_next_count ��ֵ�������֤�����´�ִ�ж�ʱ����ʾ����������app�ϵ��ĸ�������ʾ�͸��ݽ����߼�����,
���磬�����������ƽ�����ʾ���ض�ʱ������Ҫ�������´ζ�ʱ������һ�£��ҳ�type=
UT_TYPE_ON 
UT_TYPE_OFF
UT_TYPE_PERIOD_ONOFF 
�����ඨʱ������ʾ

���������������жϸ������ʾʱ��:
	
type = 	UT_TYPE_ON						UT_TYPE_OFF					UT_TYPE_PERIOD_ONOFF ���������ʾ����
		���next_start_valid=1			���next_start_valid=1			���next_start_valid=1	���next_finish_valid=1
		��ʾ�´ο�						��ʾ�´ι�						��ʾ�´ο�				��ʾ�´ι�
		next_start_day					next_start_day					next_start_day			next_finish_day
		next_start_hour					next_start_hour					next_start_hour			next_finish_hour
		next_start_min					next_start_min					next_start_min			next_finish_min

����������ֻ��������һ��(����ʱ��ʱ���ͻ���)

�������ͣ��Ժ���˵ 	

*/
/*
	�ϰ汾����
    u_int8_t timer_next_count; //�´�Ҫִ�е��ж��ٸ���ʱ��
    u_int8_t next_start_valid;//�´ο�ʼ��ʱ��ʱ���Ƿ���Ч
    u_int8_t next_start_day;
    u_int8_t next_start_hour;
    u_int8_t next_start_min;
	
    u_int8_t next_finish_valid;//�´ν�����ʱ��ʱ���Ƿ���Ч
    u_int8_t next_finish_day;
    u_int8_t next_finish_hour;
    u_int8_t next_finish_min;
    u_int8_t id[32];//��һ�ζ�ʱ����id����,Ϊ1��ʾ��Ч,�豸�˲�֧�ֳ���32����ʱ��

	u_int32_t min_time;//sdkר��
	u_int8_t type;//sdkר��
	u_int8_t id_tmp;//sdkר��


*/
typedef struct {
    u_int8_t next_valid;
    u_int8_t next_day;
    u_int8_t next_hour;
    u_int8_t next_min;
}next_exec_time_t;

typedef struct {
	bool comm_timer_valid;//Ϊ��ʱ�ű�ʾ�����������Ч
	//����Ϊ�˱���ͨ�ö�ʱ���ĸ���̫�鷳�ˣ������޸��£��´ιض�ʱ�����´ο���ʱ���������´������¶�����ʲô��
	//�´�ִ�п�����ʱ����ʱ��
    next_exec_time_t next_on;
	//�´�ִ�йػ���ʱ����ʱ��
	next_exec_time_t next_off;

	//�´��¶����߿�ʼ����ʱ��ʲô��
	next_exec_time_t next_temp_start;
	//�´��¶����߽�������ʱ��ʲô��
    next_exec_time_t next_temp_finish;

	//�Ժ���Ҫʲô���ʲô�ɣ�Ҫ��Ȼû�����ͨ�õ��ˡ�������������������������������

	u_int32_t min_time_start;//sdkר��
	u_int32_t min_time_finish;//sdkר��
	u_int32_t min_time;//sdkר��
	bool is_slave;//sdkר��
	u_int8_t sub_type;//sdkר��
	u_int8_t ext_type;//sdkר��
	
	u_int8_t timer_count;
	u_int8_t real_count;
	cl_comm_timer_t *timer;
}cl_comm_timer_head_t;

/*
��ע:
support_type֧�������Ǳ�ʾ�豸��֧�ֵ����ͣ�Ŀǰ��ʱ����udp_ctrl.h�ļ�����3��
enum {
	UT_DEV_TYPE_ONOFF = 1, //���ض�ʱ��
	UT_DEV_TYPE_PERIOD_CONSTANT_TEMP = 2, //����
	UT_DEV_TYPE_PERIOD_TEMP_CURVE = 3, //�¶�����
	UT_DEV_TYPE_MAX
};

*/
typedef struct {
	u_int8_t max_timer_count; //app��
	u_int8_t max_type_count;//app��,������Ϊ�˿��ؼ��ݵģ��Ժ��ú����support_type
	u_int8_t max_data_len;//app��
	u_int16_t support_type;//app�ã�bit0-bit15�ֱ��ʾ֧��type1-type16������

	//������sdkר�ã�app���ù�
	u_int8_t reserve;
	u_int32_t map;
	u_int64_t sn;
	u_int32_t need_query;
	u_int16_t *stat_count;
}cl_dev_timer_summary_t;

/***********************/

typedef struct {
	u_int32_t light;
	u_int32_t temp;
	u_int32_t power;
	u_int32_t humidity;
}cl_air_debug_info_set_t;


/*
 ����:
    �����Ͽյ�
 �������:
    @key
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_old_air_ctrl(cl_handle_t dev_handle, u_int8_t key_id);

/*
 ����:
    ���ƿյ�
 �������:
    @stat, �������������0����ʹ�õ�ǰ״ֵ̬
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_air_ctrl(cl_handle_t dev_handle, cl_air_work_stat_t* stat);

/*
 ����:
    ���ƿյ����ػ�
 �������:
    @onoff, false:OFF true:ON
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_air_ctrl_power(cl_handle_t dev_handle, bool onoff);
    
/*
 ����:
    ���ƿյ�
 �������:
    @mode: CL_AC_MOD_XXX
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_air_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
 ����:
    ���ƿյ��¶�
 �������:
    @temp�� �¶�: 16-30
 �������:
 ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_air_ctrl_temp(cl_handle_t dev_handle, u_int8_t temp);
    
/*
����:
    ���ƿյ�����
�������:
    @wind: ���� CL_AC_WIND_XX
�������:
    ��
����:
 RS_OK: �ɹ�
����: ʧ��
*/
CLIB_API RS cl_sa_air_ctrl_wind(cl_handle_t dev_handle, u_int8_t wind);

/*
 ����:
    ���ƿյ�����
 �������:
    @wind: ���� CL_AC_DIRECT_XX
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_air_ctrl_direct(cl_handle_t dev_handle, u_int8_t direct);

/*
 ����:
    ���ƿյ�LED��
 �������:
    @onoff: 0:�رգ�1��������2������
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_ctrl_led_power(cl_handle_t dev_handle, u_int8_t onoff);
    
/*
 ����:
    ���ÿյ�������ʱ��
 �������:
    @begin_time: ��ʼʱ�� ��λ������  0�� 0��0��  1439��23ʱ59��
    @last_time:  ����ʱ�� ��λ������  max�� 1440
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_air_peak_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
/*
 ����:
    ���ÿյ��ȵ����ʱ��
 �������:
    @begin_time: ��ʼʱ�� ��λ������  0�� 0��0��  1439��23ʱ59��
    @last_time:  ����ʱ�� ��λ������  max�� 1440
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_air_valley_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
    
/*
 ����:
    ���ÿյ����۸�
 �������:
    @price: ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_air_peak_price(cl_handle_t dev_handle, u_int32_t price);

/*
 ����:
    ���ÿյ��ȵ�۸�
 �������:
    @price: ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_air_valley_price(cl_handle_t dev_handle, u_int32_t price);

/*
 ����:
    ���ÿյ�ƽ��۸�
 �������:
    @price: ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_air_flat_price(cl_handle_t dev_handle, u_int32_t price);
/*
 ����:
    ��ӿյ���ʱ��
 �������:
    @time_info: ��ʱ����Ϣ id�����ڣ���Ϊ�޸�
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_add_air_timer(cl_handle_t dev_handle, cl_air_timer_t* time_info);

/*
 ����:
    ��ӿյ����ڶ�ʱ��
 �������:
    @time_info: ��ʱ����Ϣ id�����ڣ���Ϊ�޸�
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_add_air_period_timer(cl_handle_t dev_handle, cl_period_timer_t* time_info);

/*
 ����:
    ɾ���յ���ʱ��
 �������:
    @timer_id : ��ʱ��ID
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_del_air_timer(cl_handle_t dev_handle, u_int8_t timer_id);


/*
 ����:
    ɾ���յ���ʱ��
 �������:
    @timer_id : ��ʱ��ID
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_del_period_timer(cl_handle_t dev_handle, u_int8_t timer_id);
    
/*
 ����:
    ��ʼ����ƥ��
 �������:
    @all_match true��ȫƥ��  false:��ƥ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_start_code_match(cl_handle_t dev_handle, bool all_match,u_int32_t timeout);
    
/*
 ����:
    ֹͣ����ƥ��
 �������:
    @dev_handle �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_stop_code_match(cl_handle_t dev_handle);

/*
 ����:
    ��ȡ��ǰ����ƥ��״̬
 �������:
    @dev_handle �豸���
 �������:
    @stat: ״ֵ̬
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_get_code_match_stat(cl_handle_t dev_handle,cl_air_code_match_stat_t* stat);

/*
 ����:
    �������ܿ���
 �������:
    @dev_handle �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_smart_power_on(cl_handle_t dev_handle,bool enable);

/*
 ����:
    �������ܹػ�
 �������:
    @dev_handle �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */

CLIB_API RS cl_sa_set_smart_power_off(cl_handle_t dev_handle,bool enable);

/*
 ����:
    ��������˯��
 �������:
    @dev_handle �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_smart_sleep(cl_handle_t dev_handle,bool enable);

/*
 ����:
    ��ȡ
 �������:
    @dev_handle �豸���
 �������:
    @rdata ʵʱ����
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_air_get_real_time_data(cl_handle_t dev_handle,cl_air_real_time_data_t* rdata);

/*
 ����:
    ��ȡ�յ���ǰ����
 �������:
    @time_interval
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 */
CLIB_API RS cl_sa_air_start_get_cur_power(cl_handle_t dev_handle,u_int8_t time_interval);
    
/*
 ����:
    ֹͣ��ȡ�յ���ǰ����
 �������:
    @time_interval
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 */
CLIB_API RS cl_sa_air_stop_get_cur_power(cl_handle_t dev_handle);

/*
 ����:
    �������ܿ�����ϸ����
 �������:
    @dev_handle �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_smart_power_on_detail(cl_handle_t dev_handle,cl_smart_air_on_param_t* ao);

/*
 ����:
 �������ܹػ���ϸ����
 �������:
 @dev_handle �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */

CLIB_API RS cl_sa_set_smart_power_off_detail(cl_handle_t dev_handle,cl_smart_air_off_param_t* af);

/*
 ����:
    ��������˯����ϸ����
 �������:
    @dev_handle �豸���
    @as
 �������:
 ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_set_smart_sleep_detail(cl_handle_t dev_handle,cl_smart_air_sleep_param_t* as);

/*
 ����:
    ˢ�¶�ʱ����Ϣ
 �������:
    @dev_handle �豸���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_sa_air_refresh_timer_info(cl_handle_t dev_handle);

/*
 ����:
    ˢ�µ���ͳ����Ϣ
 �������:
    @dev_handle �豸���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_sa_air_refresh_elec_info(cl_handle_t dev_handle);

/*
 ����:
    ˢ�µ���ͳ����Ϣ
 �������:
    @dev_handle �豸���
    @type: 0x0: �������ͳ����Ϣ
               0x1: �����������ĵ���ͳ����Ϣ��
               0x2: ����ۼ�ͳ����Ϣ
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_sa_air_clear_elec_stat_info(cl_handle_t dev_handle,int type);

/*
 ����:
    ����led ����ɫ
    @ on_color  �յ�����ʱ�Ƶ���ɫ
    @off_color  �յ��ر�ʱ�Ƶ���ɫ

 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_sa_air_set_led_color(cl_handle_t dev_handle,int on_color,int off_color);

/*
	͸�����������Ƭ��
	ע��: ��������õ�
*/
#define MAX_TRANS_LEN 1024
CLIB_API RS cl_sa_air_trans_send(cl_handle_t dev_handle, u_int8_t *buf, int len);

/*
	��ȡ�豸͸�����ͻ��˵ĺ������
	SAE_TT_IR�¼�ʱ���ñ�����
	����:������볤��
	
*/
CLIB_API int cl_sa_get_trans_ir(cl_handle_t dev_handle, u_int8_t *buf, int len);

/*
	��ȡ�豸͸�����ͻ��˵�����
	SAE_TT_SOUND�¼�ʱ���ñ�����
	����:�������볤�ȣ�����0�ɹ���С�ڵ���0ʧ��
	
*/
CLIB_API int cl_sa_get_trans_sound(cl_handle_t dev_handle, u_int8_t *buf, int len);

/*
	�ָ�����
	ע��: ��������õģ�����
*/
CLIB_API RS cl_sa_air_restore_factory(cl_handle_t dev_handle);

/*
	���õ�ѹ����У׼ֵ
	ע��: ��������õģ�����
*/
CLIB_API RS cl_sa_pt_set_adkb(cl_handle_t dev_handle,  cl_air_pt_adkb_t *v);

/*
	���õ�ѹ����У׼ֵ
	ע��: ��������õģ�����
*/
CLIB_API RS cl_sa_pt_set_adkb_ext(cl_handle_t dev_handle,  cl_air_pt_adkb_t *v);

/*
	���õ�ѹ����У׼ֵ(V7����У��)
	ע��: ��������õģ�����
*/
CLIB_API RS cl_sa_pt_set_adj(cl_handle_t dev_handle,  cl_plug_pt_adkb_t *v);


/*
 ����:�л���ѧϰ�ı���
	@new_code_id; �л����µ�id
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_sa_air_reset_ir_code_id(cl_handle_t dev_handle,  u_int16_t new_code_id);


///////////////////////////////////////////////////////////////
// �����������ӿ�
/*
	����������� pan_type ��AC_PAN_WINDOW�ȶ���
*/
CLIB_API RS cl_ac_set_pan_type(cl_handle_t dev_handle,u_int8_t pan_type);

/*
 ����:ˢ�����а�����Ϣ
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_refresh_key_info(cl_handle_t dev_handle);

/*
	 ����:���ð�����Ϣ
	@key_id,���豸û�и�id������Ϊ��ӣ�������Ϊ�޸�
	@key_name,��������15�ֽ�
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_set_key_info(cl_handle_t dev_handle,u_int8_t key_id,char* key_name);

/*
	 ����:���ð�����Ϣ
	@key_id,���豸û�и�id������Ϊ��ӣ�������Ϊ�޸�
	@key_name,��������15�ֽ�
	@flag: ��־��Ŀǰʹ��bit 4: ����KEYΪ���� bit5: ����KEY֧��״̬����
	@air_stat: ��flag���Ͽ���ģʽʱ��,air_stat����״̬��״̬����ѧϰ�ɹ��󷵻ص�air_info->key_info->stat
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_set_key_info_v2(cl_handle_t dev_handle, u_int8_t key_id, char* key_name, u_int8_t flag, air_key_stat_t air_stat);

/*
	 ����:ɾ������
	@key_id, �Ѵ�����key id
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_delete_key(cl_handle_t dev_handle,u_int8_t key_id);

/*
	 ����: ��ʼ��������ѧϰ
	@key_id, �Ѵ�����key id
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_start_learn_key(cl_handle_t dev_handle,u_int8_t key_id);

/*
	 ����: ֹͣ��������ѧϰ
	@key_id, �Ѵ�����key id
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_stop_learn_key(cl_handle_t dev_handle,u_int8_t key_id);

/*
	 ����: ��������
	@key_id, �Ѵ�����key id
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_ctrl_key(cl_handle_t dev_handle,u_int8_t key_id);

//************************************************************����ͬ�������ӿ�**************************************************************
/*
	 ����: ��ѯ�Ƿ���ͬ��
    �������:	@status, ��ѯͬ��״̬��1��ʾͬ�롣
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_query_samecode(cl_handle_t dev_handle,u_int8_t *status);

/*
	 ����: ���ÿյ�����״ֵ̬
    �������:	@on, ��������״̬��0�رգ�1������
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_modify_onoff_status(cl_handle_t dev_handle,u_int8_t on);

//*********************************************************�¶����ߴ���ӿ�****************************************************************************************
/*
 ����: ����޸��¶���������
 �������:	@ptmp_curve,�¶��������ò���
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_sa_modify_temp_curve(cl_handle_t dev_handle,cl_temp_curve_t *ptmp_curve);

//***********************************�������¿��ƿյ�*******************************************************
/*
 ����: ������ø����¶ȿ��ƿյ��ӿ�
 �������:	@ptmp_ctrl,���ò���
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
*/
CLIB_API RS cl_ac_set_temp_ctrl(cl_handle_t dev_handle,cl_temp_ac_ctrl_t *ptmp_ctrl);
    
/*
 ����: ���ÿյ������Ƿ��ˣ���⵽ͬ����ܵ��ô˺���
 �������:	is_opposite 0: ������1������
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ac_set_fan_speed_opposite(cl_handle_t dev_handle,u_int8_t is_opposite);
    
/*
 ����: ���ÿյ�ɨ�繤��״̬
 �������:	is_opposite 0: ɨ��رգ�1��ɨ�翪��
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ac_set_fan_stat(cl_handle_t dev_handle,u_int8_t fan_stat);

/*
����: 
    ����ͯ��״̬
    @lock_stat: CHILD_LOCK_XX
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_ac_set_child_lock_stat(cl_handle_t dev_handle,u_int8_t lock_stat);

/*
����: 
    ����I8��Ϣ���Ϳ���
    @config: ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_ac_set_msg_config(cl_handle_t dev_handle, cl_ac_msg_config_t *config);

/*
����: 
    ͨ�ö�ʱ������
    @ptimer: ��ʱ������
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_misc_comm_timer_modify_add(cl_handle_t dev_handle, cl_comm_timer_t *ptimer);

/*
����: 
    ͨ�ö�ʱ������
    @id: ��ʱ��id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_misc_comm_timer_del(cl_handle_t dev_handle, u_int8_t id);


/*
 ����:
    ͨ�õ��������ܿ�������ѯ��
    air_ctrl->com_udp_dev_info.smart_on
 �������:
    @dev_handle �豸���
    @enable: �Ƿ�ʹ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_smart_on(cl_handle_t dev_handle, u_int8_t enable);

/*
 ����:
    ͨ�õ�����ͯ������ѯ��
    air_ctrl->com_udp_dev_info.child_lock_value
 �������:
    @dev_handle �豸���
    @type: ͯ������:0-�ر�ͯ�� 1-����������� 2-�����ز���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_child_lock(cl_handle_t dev_handle, u_int8_t type);


/*
 ����:
    ͨ�õ����ÿ����¶�
    air_ctrl->com_udp_dev_info.boot_temp
 �������:
    @dev_handle �豸���
    @enable: �Ƿ���
    @temp: �����¶ȣ����϶�
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_boot_temp(cl_handle_t dev_handle, u_int8_t enable, u_int8_t temp);

/*
 ����:
    ͨ�õ������¶ȷ�ֵ
    air_ctrl->com_udp_dev_info.temp_alarm_xxx
 �������:
    @dev_handle �豸���
    @onoff: ����
    @min: ��С�¶�
    @max: ����¶�
    
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_temp_alarm(cl_handle_t dev_handle, u_int8_t onoff, u_int8_t min, u_int8_t max);

/*
 ����:
    ͨ�õ�����ͯ������ѯ��
    air_ctrl->com_udp_dev_info.shortcuts_onoff
 �������:
    @dev_handle �豸���
    @enable: �Ƿ�ʹ��
    @onoff: 0: �ػ� 1����
    @time: ʣ��ʱ�䣬����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_shortcuts_onoff(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int32_t time);

/*
 ����:
    ͨ�õ����ú���
    air_ctrl->com_udp_dev_info.tac
 �������:
    @dev_handle �豸���
    @ptmp_ctrl: ���Ʋ���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_temp_ctrl(cl_handle_t dev_handle, cl_temp_ac_ctrl_t *ptmp_ctrl);

/*
 ����:
    ͨ�õ������¶�����
    air_ctrl->com_udp_dev_info.temp_curve
 �������:
    @dev_handle �豸���
    @ptmp_ctrl: ���Ʋ���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_modify_temp_curve(cl_handle_t dev_handle, cl_temp_curve_t *ptmp_curve);

/*
 ����:
    ͨ�õĲ�ѯWIFI�豸����ʷ��¼
 �������:
    @dev_handle �豸���
    @index: ��ʼID
    @num: ��ѯ�ĸ���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 ��ע: ���յ�SAE_DEV_COMM_HISTORY_ITEM = 1242����ʾ��ѯ�����������
                com_udp_dev_info.dev_historyȥ��ȡ����
 */
CLIB_API RS cl_sa_public_history_query(cl_handle_t dev_handle, u_int32_t index, u_int32_t num);

/*
 ����:
    ͨ�õ������豸����������
    air_ctrl->com_udp_dev_info.wan_config
 �������:
    @dev_handle �豸���
    @config: ������Ϣ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_wan_config(cl_handle_t dev_handle, cl_wan_request_config_t *config);

/*
 ����:
    ͨ�õ������豸DHCP������
    air_ctrl->com_udp_dev_info.dhcp_config
 �������:
    @dev_handle �豸���
    @config: ������Ϣ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_dhcp_server_config(cl_handle_t dev_handle, cl_dhcp_server_config_t *config);

/*
 ����:
    ͨ�õ������豸AP
    air_ctrl->com_udp_dev_info.ap_config
 �������:
    @dev_handle �豸���
    @config: ������Ϣ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_ap_config(cl_handle_t dev_handle, cl_ap_config_t *config);
/*
 ����:
    ͨ�õ������豸�м�������
    air_ctrl->com_udp_dev_info.repeat_config
 �������:
    @dev_handle �豸���
    @onoff: ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_public_set_repeat_onoff(cl_handle_t dev_handle, u_int8_t onoff);

#ifdef __cplusplus
}
#endif 

#endif

