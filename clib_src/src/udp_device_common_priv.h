#ifndef	__UDP_DEVICE_COMMON_PRIV_H__
#define	__UDP_DEVICE_COMMON_PRIV_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"

#define DAYS_PER_WEEK 0x7

// udp ͨ�ù���action
enum{
	ACT_UDP_COM_TIMER_SET = 0x0,
	ACT_UDP_COM_TIMER_DEL ,
	ACT_UDP_COM_PEROID_TIMER_SET ,
	ACT_UDP_COM_PEROID_TIMER_DEL ,
	ACT_UDP_COM_TIMER_REFRESH,
	ACT_UDP_COM_REFRESH_ALL_STAT,
	ACT_UDP_COM_SET_PEAK_TIME, 
	ACT_UDP_COM_SET_VALLEY_TIME, 
	ACT_UDP_COM_SET_PEAK_PRICE, 
	ACT_UDP_COM_SET_VALLEY_PRICE, 
	ACT_UDP_COM_SET_FLAT_PRICE,
	ACT_UDP_COM_REFRESH_ELEC_STAT,
	ACT_UDP_COM_CLEAR_ELEC_STAT,
	ACT_UDP_COM_CLEAR_DEV_ERR_INFO,
	ACT_UDP_COM_REFRESH_DEV_ERR_INFO,
	ACT_UDP_COM_SET_PEMIT_STM_UPGRADE,
	ACT_UDP_COM_RESTORY_FACTORY,
	ACT_UDP_COM_SETTING_SSID_PASSWD,
    ACT_UDP_COM_EXT_PEROID_TIMER_SET,
    ACT_UDP_COM_AJUST_ROOM_TEMP,
    ACT_UDP_COM_AJUST_ELEC_VALUE,
    ACT_UDP_COM_REFRESH_24HOUR_LINE,
    ACT_UDP_COM_REQUEST_SHARE_CODE,
    ACT_UDP_COM_DEL_SHARED_PHONE,
    ACT_UDP_COM_REFRESH_SHARED_LIST,
    ACT_UDP_COM_MODFIY_SHARED_PHONE
};
// ���ܱ��¿�������action
enum{
	ACT_JNB_CTRL_ONOFF = 0x0,
	ACT_JNB_CTRL_MODE ,
	ACT_JNB_CTRL_TEMP,
	ACT_JNB_CTRL_SCHED,
	ACT_JNB_CTRL_TEMP_PARAM,
	ACT_JNB_CTRL_HOLIDAY,
	ACT_JNB_CTRL_HOLD_TIME
};

// �ذ����action
enum{
	ACT_TB_CTRL_STAT = 0x0,
	ACT_TB_CTRL_SETTING_WORK_PARAM,
	ACT_TB_CTRL_REFRESH_TEMP_INFO,
	ACT_TB_CTRL_REFRESH_OTHER_INFO,
	ACT_TB_CTRL_REFRESH_FAULT_INFO,
	ACT_TB_CTRL_BIND_BAR_CODE
};

// �����¿�������action
enum{
	ACT_YL_CTRL_ONOFF = 0x0,
	ACT_YL_CTRL_MODE,
	ACT_YL_CTRL_GEAR,
	ACT_YL_CTRL_TEMP,
	ACT_YL_CTRL_SCENE,
	ACT_YL_SETTING_TEMP_PARAM
};

//LEDE��ɫ�ƿ���action
enum{
	ACT_LEDE_CTRL_STAT = 0x0,
	ACT_LEDE_CTRL_TIEMR ,
	ACT_LEDE_DELETE_TIEMR,
	ACT_LEDE_ON_STAT,
};

enum{
	ACT_AMT_CTRL_ONOFF = 0x0,
	ACT_AMT_CTRL_MODE,
	ACT_AMT_CTRL_GEAR,
	ACT_AMT_CTRL_SHAKE,
	ACT_AMT_CTRL_SCREEN_LIGHT,
	ACT_AMT_CTRL_U_DEF_MODE,
	ACT_AMT_CTRL_S_TIMER,
	ACT_AMT_CTRL_ANION,
	ACT_AMT_CTRL_PLASMA,
	ACT_AMT_CTRL_SMART_PARAM
};

enum{
	ACT_CHIFFO_CTRL_ONOFF = 0x0,
	ACT_CHIFFO_CTRL_WATER_TEMP,
    ACT_CHIFFO_ADD_DEC_WATER_TEMP,
	ACT_CHIFFO_CTRL_HEATER_TEMP,
    ACT_CHIFFO_ADD_DEC_HEATER_TEMP,
	ACT_CHIFFO_SET_MODE,
	ACT_CHIFFO_SET_TIMER,
	ACT_CHIFFO_REFRESH_TIMER,
	ACT_CHIFFO_SET_SCENE,
	ACT_CHIFFO_SET_LOOP,
	ACT_CHIFFO_SET_CLOCK,
	ACT_CHIFFO_SET_WATER_CAP_UP_OR_DOWN,
	ACT_CHIFFO_SET_WATER_CAP
};

//��Ѹaction����
enum {
	ACT_HX_MODE_CMD = 0X0,
	ACT_HX_DIY_NAME = 0X1,
	ACT_HX_FINISH_CLEAR = 0X2,
};

enum{
	ACT_TL_CTRL_ONOFF = 0,
	ACT_TL_CTRL_MODE,
	ACT_TL_CTRL_FAN_SPEED,
	ACT_TL_CTRL_TEMP,
	ACT_TL_CTRL_ECO,
	ACT_TL_CTRL_LEARN,
	ACT_TL_CTRL_TIMER,
    ACT_TL_CTRL_TIME_AUTO_SYNC,
    ACT_TL_CTRL_TIME_SYNC,
    ACT_TL_CTRL_REFRESH_TIME
};

//ǧ������
enum {
	ACT_QPCP_CTRL_ONOFF = 0X0,
	ACT_QPCP_CTRL_ADD_WATER,
	ACT_QPCP_CTRL_HANDLE_CTRL,
	ACT_QPCP_CTRL_SCENE_DEL,
	ACT_QPCP_CTRL_SCENE_MODIFY,
	ACT_QPCP_CTRL_SCENE_EXECUTE,
    ACT_QPCP_CTRL_RESET_FAULT
};

enum{
    ACT_STB_CHANGE_NAME = 0x0,
    ACT_STB_START_MATCH,
    ACT_STB_STOP_MATCH,
    ACT_STB_CTRL_KEY,
    ACT_STB_QUICK_ONOFF,
    ACT_STB_START_LEARN,
    ACT_STB_STOP_LEARN,
    ACT_STB_DELETE_KEY,
    ACT_STB_MODIFY_KEY,
    ACT_STB_START_NKEY_MATCH
};

//ǧ����
enum{
    ACT_QP_POT_CTRL = 0x0,
    ACT_QP_POT_EXEC_SCENE,
    ACT_QP_POT_DEL_SCENE,
    ACT_QP_POT_MODIFY_SCENE
};

//�������
enum {
	ACT_CAR_CONFIG_ON = 0X0,
	ACT_CAR_CTRL_ON = 0X1,
	ACT_CAR_CONFIG_SEARCH = 0X2,
	ACT_CAR_CTRL_SEARCH = 0X3,
	ACT_CAR_CONFIG_VALTAGE = 0X4,
	ACT_CAR_CTRL_VALTAGE = 0X5,
	ACT_CAR_CTRL_POWERSAVE = 0X6,
};

//ɳ�ز���
enum{
    ACT_EO_SET_ONOFF,
    ACT_EO_SET_TEMP_RANGE,
    ACT_EO_SET_THRESHOLD,
    ACT_EO_SET_OFFLINE_ENABLE,
    ACE_EO_SET_PERSON_ENABLE
};

//��Դ�¿���
enum {
	ACT_XY_CTRL_ONOFF = 0X0,
	ACT_XY_CTRL_TEMP,
	ACT_XY_CTRL_MODE,
	ACT_XY_CTRL_TIME,
	ACT_XY_CTRL_ADJUST,
	ACT_XY_CTRL_LOCK_ONOFF,
	ACT_XY_CONFIG_SMART_MODE,
	ACT_XY_CTRL_EXTERN_TEMP,
	ACT_XY_CTRL_SMARTHOME_ONOFF,
	ACT_XY_CTRL_SMART_MODE,
};

//bimarů���
enum{
    ACT_BIMAR_NONE = 0x0,
    ACT_BIMAR_COMMON_CTRL
};

//ǧ���Ʊڻ�
enum{
    ACT_QP_PBJ_CTRL_ONOFF = 0x0,
    ACT_QP_PBJ_CTRL_EXEC_SCENE,
    ACT_QP_PBJ_CTRL_MODIFY_SCENE,
    ACT_QP_PBJ_CTRL_FAULT_STAT
};

//���û����
enum {
	ACT_TBB_ON = 0X0,
	ACT_TBB_MODE,
	ACT_TBB_TMP,
	ACT_TBB_CONFIG,
	ACT_TBB_BIND,
};

//ǧ���Ʊڻ�
enum{
    ACT_HX_YSH_CTRL_ONOFF = 0x0,
    ACT_HX_YSH_CTRL_EXEC_SCENE,
    ACT_HX_YSH_CTRL_MODIFY_SCENE,
    ACT_HX_YSH_CTRL_DEL_OR_STOP_SCENE
};

//����
enum {
	ACT_YT_CTRL = 0X0,
	ACT_YT_TIMER,
	ACT_YT_SN,
	ACT_YT_ELE_INFO,
	ACT_YT_QUERY_ELE,
	ACT_YT_QUERY_POWER,
};
//�麣������
enum{
	ACT_JL_UNKNOWN = 0x0,
	ACT_JL_3200_CTRL,
    ACT_JL_3200_AL_CTRL
};


//�ĵ���
enum {
	ACT_ADS_CTRL = 0X0,
	ACT_ADS_CONF,
};

//��ʯ΢��¯
enum{
    ACT_JS_WAVE_CTRL = 0x0,
    ACT_JS_AUTO_MENU,
    ACT_JS_CHILD_LOCK,
    ACT_JS_FAST_CTRL
};

//��ϣ��
enum{
    ACT_KXM_CTRL_ONOFF,
    ACT_KXM_CTRL_MODE,
    ACT_KXM_CTRL_TIMER,
    ACT_KXM_CTRL_DEV_TIME,
    ACT_KXM_CTRL_ALL_TIMER,
    ///////////////////////////////
    ACT_KXM_THER_COMMON_CTRL
};

//˼����
enum{
    ACT_SBT_CTRL_STAT,
    ACT_SBT_SETTINT_PARAM,
    ACT_SBT_AJUST_TIME,
    ACT_SBT_SMART_MODE_PARAM,
    ACT_SBT_SMART_MODE_ENABLE
};

//��ɽ����
enum {
	ACT_ZSSX_CTRL_ON,
	ACT_ZSSX_TIMER,
	ACT_ZSSX_WIFI_CONF,
};

// ��ɽ��ѵ�ů¯
enum {
    ACT_YJ_HEATER_CTRL
};

// ӡ�ȳ���׷����
enum {
	ACT_INDIACAR_REQUEST_HISTORY,		// ������ʷ��¼
	ACT_INDIACAR_REQUEST_UPGRADE,		// ��������
	ACT_INDIACAR_REQUEST_WARN_SETTING,	// �澯����
	ACT_INDIACAR_REQUEST_WIFI_CONFIG,	// ����WIFI����
	ACT_INDIACAR_REQUEST_REALTIME_TRIP,	// ����ʵʱ����
	ACT_INDIACAR_REQUEST_DEBUG_CONFIG,	//  ������Ϣ
	ACT_INDIACAR_REQUEST_VIDEO,	// ��ʼ����ֹͣ��Ƶ
	ACT_INDIACAR_REQUEST_LOCAL_VIDEO_WATCH,	// �������鿴��Ƶ
	ACT_INDIACAR_REQUEST_RECORD,	// �������ֹͣ¼��
	ACT_INDIACAR_REQUEST_DECODE_MP4,	// ���󲥷�MP4
};

//�ǿ���ˮ��
enum {
	ACT_ZKRSQ_CTRL,
	ACT_ZKRSQ_CONFIG,
	ACT_ZKRSQ_TIMER,
};

//�ǻʴ���
enum{
	ACT_ZHCL_STATUS,
	ACT_ZHCL_LOCATION,
	ACT_ZHCL_BIND,
	ACT_ZHCL_TYPE,
	ACT_ZHCL_DIR,
};

//�ǻʵ�����
enum {
	ACT_ZHDHX_ONOFF,
	ACT_ZHDHX_KEY_NAME,
};
///////////////////////////////////////////////////////////////////////////
//���豸˽��ͨѶЭ��
#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
//�ذ���

typedef struct{
	/* �ӻ��¿�ID */    
	u_int8_t cid;    
	/* ���ػ� */    
	u_int8_t onoff;   
	/* ����ģʽ */   
	u_int8_t work_mode;   
	/* �¶�ֵ */    
	u_int8_t temp;    
	/* Ԥ�� */    
	u_int8_t reserved[4];
}ucp_tb_user_config_t;


typedef struct  {
	 /* �ӻ��¿�ID */    
	u_int8_t cid;    
	/* ��ý�ؿ��� */    
	u_int8_t return_cold_switch;    
	/* �豸��װ״̬ */    
	u_int16_t facility_state;  
	/* ϵͳ����ѡ�� */   
	u_int16_t sysfunc;    
	/* �ز��¶� */    
	u_int8_t return_diff_temp;    
	/* ���ȳ�˪���� */    
	u_int8_t heat_defrost_circle;    
	/* ���Ƚ����˪�¶� */   
	u_int8_t start_heat_defrost_temp;    
	/* �����˳���˪�¶� */    
	u_int8_t stop_heat_defrost_temp;    
	/* �����˳���˪ʱ�� */    
	u_int8_t stop_heat_defrost_time;    
	/* ����������趨ֵ */    
	u_int8_t eheat_value;    
	/* ������ʱ�ر�ʱ�� */    
	u_int8_t backlight_delay_time;    
	/*����ģʽ*/
	u_int8_t fan_mode;
	u_int8_t reserved[2];
}ucp_tb_work_config_t;
	

typedef struct  {
	/*�ӻ��¿�ID*/
	u_int8_t cid;
	u_int8_t mode;   /* 0���ֶ���1���Զ� */
	u_int8_t year;   /* 0 ~ 99 (2000 ~ 2099)*/
	u_int8_t month;  /* 1 ~ 12 */
	u_int8_t mday;   /* 1 ~ 31 */
	u_int8_t hour;   /* 0 ~ 23 */
	u_int8_t minute; /* 0 ~ 59 */
	u_int8_t second; /* 0 ~ 59 */
}ucp_tb_rtc_t;

typedef struct {
	/* �ӻ��¿�ID */    
	u_int8_t cid;   
	/* �����¶� */    
	u_int8_t env_temp;    
	/* ˮ���²��¶� */    
	u_int8_t tankbottom_temp;    
	/* ˮ���ϲ��¶� */   
	u_int8_t tanktop_temp;   
	/* �̹��¶� */  
	u_int8_t coil_temp;   
	/* �����¶� */  
	u_int8_t air_temp;    
	/* �����¶� */    
	u_int8_t returnair_temp;  
	/* ��ˮ�¶� */   
	u_int8_t outwater_temp;  
	/* ��ˮ�¶� */   
	u_int8_t inwater_temp;   
	/* ��ˮ�¶� */    
	u_int8_t returnwater_temp;   
	/* ������ */    
	u_int16_t heat_capacity;   
	/* ����ʱ�� */   
	u_int16_t heat_time;   
	/* �ĵ��� */   
	u_int16_t consumption_power
	;    /* ��ʡ���� */   
	u_int16_t saving_power;  
	u_int16_t reserved;
}ucp_tb_temp_info_t;

typedef struct {
	/* �ӻ��¿�ID */  
	u_int8_t cid;   
	/* ���ӷ����� */  
	u_int8_t valve_expansion; 
	/* �ӻ��豸�ϵ�״̬ */   
	u_int16_t slave_onoff;   
	/* �豸/����1 */  
	u_int16_t dev_fault;   
	/* �豸/����2 */   
	u_int16_t dev_guard; 
	u_int16_t load_state;
	u_int16_t reserved;
}ucp_tb_fault_stat;

typedef struct {
	/*�ӻ��¿�ID*/
	u_int8_t cid;
	/* ������Ϣ */
	u_int8_t dev_info;
	/* ģʽѡ�� */
	u_int8_t dev_mode;
	/* �̼��汾�� */
	u_int8_t fw_version;
	/* ���ذ�̼��汾�� */
	u_int8_t mb_version;
	u_int8_t svn_version;
	u_int8_t stm_up_state;
	u_int8_t pad;
}ucp_tb_other_info;

typedef struct{
	/* �豸״̬ */  
	u_int16_t dev_state;   
	/* ��״̬ */  
	u_int16_t  bind_state;  
	/* �ذ�SN��17�ַ� */ 
	u_int8_t tb_sn[24]; 
	u_int8_t reserved[8];
}ucp_tb_bind_info;

///////////////////////////////////////////////////////////////////////////
// ���������
typedef struct {
	u_int16_t voltage; //��ѹ ��λ0.001V
	u_int16_t elec;  //���� ��λ0.001A
	int32_t active_power;	// �й����� ��λ0.1W
	int32_t reactive_power; // �޹����� ��λ 0.1 ����
	int16_t power_factor;	// �������أ���λ0.001
	u_int16_t frequency; //Ƶ��
	u_int32_t active_degree;	// ��Ч��� ��λǧ��ʱ
	u_int32_t reactive_degree;	// ��Ч��� ��λǧ����ʱ
	u_int32_t sn;				// ���к�
	u_int16_t soft_ver;			// ����汾
	u_int16_t hardware_ver;		// Ӳ���汾
} ucp_jcx_power_box_info;

/////////////////////////////////////////////////////////////////////////////////////
// LEDE ��
typedef struct{	
	u_int8_t R;			
	/*�� 0~255*/	
	u_int8_t 	G; 			
	/*��0~255*/	
	u_int8_t	B;		
	/*��0-255*/
	u_int8_t	L;	
	/*����0~100*/	
	u_int8_t cold;
	/*ɫ�� 0~100*/
	u_int8_t	power;		
	/*���أ�0Ϊ�أ�1Ϊ��*/
	u_int8_t	mod_id;		
	/*ģʽid*/	
	u_int8_t	action;
	//��Ϊ
}ucp_lede_led_state_t;

typedef struct {
	u_int8_t enable;
	u_int8_t type;
	u_int8_t pad[2];
	
	u_int8_t R;			
	/*�� 0~255*/	
	u_int8_t 	G; 			
	/*��0~255*/	
	u_int8_t	B;		
	/*��0-255*/
	u_int8_t	L;	
	/*����0~100*/	
	u_int8_t cold;
	/*ɫ�� 0~100*/
	u_int8_t	power;		
	/*���أ�0Ϊ�أ�1Ϊ��*/
	u_int8_t	mod_id;		
	/*ģʽid*/	
	u_int8_t	action;
	//��Ϊ
} ucp_lede_led_on_stat_t;


typedef struct {	
	u_int8_t	id;				
	/*��ʱ��id,��Чֵ��1��ʼ*/	
	u_int8_t	flags;			
	/*bit0:enable*/	
	u_int8_t	week_loop;		
	/*bit0���������죬bit1��������һ���ȵȡ�Ϊ0����ѭ�������ں��Զ�ɾ��*/	
	u_int8_t	hour;			
	/*Сʱ*/	
	u_int8_t	min;			
	/*��*/	
	u_int8_t	pad1;	
	u_int16_t pad2;	
	ucp_lede_led_state_t	config;	
	/*��ʱ�����ں�ʹ�ø����ø���led״̬*/
} ucp_lede_led_timer_t;

/////////////////////////////////////////////////////////////////////////////////////////
// ���ܱ��¿���

enum{
	U_JNB_OP_DEV_STATUS = 0,
	U_JNB_OP_MODE_CHANGE  ,
	U_JNB_OP_MODE_CONFIG ,
	U_JNB_OP_TEMP_CHANGE
};

typedef struct {
	u_int8_t version;		// �汾��
	u_int8_t operaton_type;	// �������ͣ����ϡ�
	u_int8_t work_status;		// ָ����ģʽ�����ϡ�
	u_int8_t vacation_remain_days; // ���ü���ģʽ�󣬵�ʣ��������

	u_int8_t env_temperature;	// �����¶�
	u_int8_t temp_temperature;	// ��ʱ�����¶�
	u_int8_t vacation_temperature;// �������õ��¶�
	u_int8_t vacation_days;	// �����趨������

	u_int8_t comfort_temperaute;	// ����ģʽ���õ��¶�
	u_int8_t comfort_hold_time;	// ��ʱHOLDʱ�䣬��λСʱ
	u_int8_t economic_temperature;// ����ģʽ���õ��¶�
	u_int8_t economic_hold_time;	// ��ʱHOLDʱ�䣬��λСʱ
	
	u_int32_t scheduler[7];	// scheduler[0]��ʾ�����죬��������Ϊ����1������6��
				// ����1��24λ�ֱ��ʾÿ��ʱ��ε�ģʽѡ��25λ��ʾ�Ƿ�����:0-δ���ã�1-������
} ucp_jnb_thermotat_info;
////////////////////////////////////////////////
//���� �¿���

typedef struct ts_data_work_s{
	u_int8_t onoff;
	u_int8_t pad[3];
}ucp_yl_work_stat;

typedef struct ts_data_mode_s{
	u_int8_t mode;
	u_int8_t scene;
	u_int8_t gear;
	u_int8_t tmp;
}ucp_yl_data_mode_t;

typedef struct ucp_yl_scene_s{
	u_int8_t set_tmp;
	u_int8_t gear;
}ucp_yl_scene_t;

////////////////////////////////////////////////////////////////////////////////
//��Ѹͨ�����ݽṹ
typedef struct ucp_hx_data_s{
	u_int8_t value;
	u_int8_t pad[3];
}ucp_hx_data_t;

typedef struct ucp_hx_diy_name_s{
	u_int8_t id;
	u_int8_t len;
	//u_int8_t name[0];
}ucp_hx_diy_name_t;

//////////////////////////////////////////////////////////////////////////////////
//�����¿����Ͳ�ůͨ��

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
	/* λ��־ */
	u_int16_t state;
	u_int8_t reserved[4];
}ucp_tl_conf_t;

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
    u_int8_t run_timer;
    u_int8_t reserved[3];
}ucp_tl_adv_t;

typedef struct{
	/* �ۼ�����ߵ�ʱ�� */
	u_int32_t high_gear_time_cal;
	/* �ۼƵ͵�ʱ�� */
	u_int32_t low_gear_time;
	/* �ۼ��е�ʱ�� */
	u_int32_t mid_gear_time;
	/* �ۼƸߵ�ʱ�� */
	u_int32_t high_gear_time;
}ucp_tl_total_time_t;

typedef struct{
	u_int16_t temp;
	u_int16_t hour;
	u_int16_t min;
}ucp_tl_timer_item_t;

typedef struct{
	ucp_tl_timer_item_t time[4];
}ucp_tl_timer_info_t;

typedef struct {
    u_int32_t utc_time;
    u_int8_t is_auto;
    u_int8_t just_sync;
    u_int8_t pad[2];
}ucp_tl_time_sync_t;

#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////

/*�Ƿ�֧���豸��������ʱ����д*/
extern bool is_supported_udp_device(u_int8_t sub_type, u_int8_t ext_type);
/*��ʼ��AC ʱ�Ļص�����Ҫ�Ƿ����ڴ�*/
extern bool udp_init_ac_hook(user_t* user,smart_air_ctrl_t* ac);
/*��ʼ��AC ʱ�Ļص�����Ҫ�Ƿ����ڴ�*/
extern void udp_set_support_flag_hook(smart_air_ctrl_t* ac, uc_sys_support_obj_t* so);
// APP ����ָ�SDK��Ĵ���
extern bool udp_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret);
// �ж�APP���͵������Ƿ�֧�����߲���
extern bool udp_proc_support_offline(u_int32_t notify_type);
// ��ѯϵͳ��Ϣ�󣬽�������Ҫ��ѯ���豸״̬��Ϣ
extern bool udp_quick_query_info_hook(smart_air_ctrl_t* ac);
// ���³�ϵͳ��Ϣ���˽����Ϣ
extern bool udp_update_data_hook(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
//���ƺ�ķ������
extern int udp_proc_ctrl_hook(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
// ��APP �����ݷ���
extern bool udp_build_objs_hook(user_t* user,cl_dev_info_t* ui);
//�ͷ��ⲿ˽������ 
extern void udp_free_objs_hook(struct cl_com_udp_device_data_s* udp_dev_info);
//�ͷ��ڲ�˽������ 
extern void udp_free_sdk_priv_data(struct cl_com_udp_device_data_s* udp_dev_info);
//���Կ���������Ƿ��ѯһ���豸����״̬���Ƿ��ģ�鷢modify
int udp_proc_ctrl_modify_hook(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
void do_rfgw_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
// ��������Ƿ�֧����APP����������
bool is_supported_udp_app_server(u_int8_t obj, u_int8_t sub_obj, u_int8_t attr);

//////////////////////////////////////////////////////
//֧��ͨ����չ��ʱ�ģ��Ҷ�ʱ���������ǿ��ز�������Ҫ������ģ��밴���豸Э����д���º���
//�����ϲ���������չ���ֱ��ģ��������ĳ���
extern int udp_fill_ext_peroid_timer_modify_pkt(smart_air_ctrl_t* ac,cl_period_timer_t* t_hdr,void* src_priv,void* dest);
//�������ݰ���������APP������,
//ext_pkt ��ָ����չ���ݣ��������豸��ͬ��
//len,��չ���ݳ���
extern void udp_updata_ext_peroid_timer_by_ext_data(smart_air_ctrl_t* ac,void* ext_pkt,int len ,cl_period_timer_t* dest);
//��һ��ͨ�ö�ʱ��
void ucp_comm_update_timer_modify_pkt(user_t *user, bool is_slave, u_int64_t slave_sn, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer);
int ucp_comm_fill_timer_modify_pkt(user_t *user, bool is_slave, u_int64_t slave_sn, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer);

#endif

