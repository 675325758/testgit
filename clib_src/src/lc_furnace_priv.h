#ifndef	__LC_FURNACE_PRIV_H__
#define	__LC_FURNACE_PRIV_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "cl_lc_furnace.h"

//�㵹����
#define IS_TOPPLE_PROTECT(flags) (!!((flags)&BIT(0)))
// �����¶ȹ��߱���
#define IS_TEMP_HIGH_PROTECT(flags) (!!((flags)&BIT(1)))
//�������¶ȹ��߱���
#define IS_FURNACE_HIGH_PROTECT(flags) (!!((flags)&BIT(2)))
//ů����������
#define IS_FURNACE_ERROR(flags) (!!((flags)&BIT(3)))


#pragma pack(push,1)

///////////////////////////////////////////////////////////////////////
/////////////////////// ���� ů���/////////////////////////

//IA.FAST_HOT_FURNCE.WORK
//UCAT_FHF_WORK        1
typedef struct{
	u_int8_t is_on;
	u_int8_t is_eco;
	u_int8_t is_mode_high;
	u_int8_t is_shake;
	u_int8_t is_heating;
	u_int8_t fault_stat;
	u_int8_t pad[2];
}lc_furnace_work_t;

//IA.FAST_HOT_FURNCE.SET_TIMER
//UCAT_FHF_SET_TIMER			2
typedef struct{
	u_int8_t is_add;
	u_int8_t pad[3];
}lc_furnace_set_timer_t;

//IA.FAST_HOT_FURNCE.GET_TIME
//UCAT_FHF_GET_TIMER			3
typedef struct{
	u_int16_t remain_minute;
	u_int8_t timer_type;
	u_int8_t set_hour;
}lc_furnace_get_timer_t;

//IA.FAST_HOT_FURNCE.POWER
//UCAT_FHF_GET_POWER			4
typedef struct{
	u_int32_t power;
}lc_furnace_get_power_t;

//IA.FAST_HOT_FURNCE.SET_TEMP	
//UCAT_FHF_SET_TEMP			5
typedef struct{
	u_int8_t is_add;
	u_int8_t pad[3];
}lc_furnace_set_temp_t;

//IA.FAST_HOT_FURNCE.GET_TEMP
//UCAT_FHF_GET_TEMP			6
typedef struct{
	u_int8_t room_temp;
	u_int8_t temp1;
	u_int8_t temp2;
	u_int8_t set_temp;
}lc_furnace_get_temp_t;

//IA.FAST_HOT_FURNCE.GET_SN
//UCAT_FHF_GET_SN				7
typedef struct{
	u_int8_t sn[8];
}lc_furnace_get_sn_t;

///////////////////////////////////////////////////////////////////////
//////////////////// ������͡ //////////////////////////////
typedef struct{
	u_int8_t stat;
	u_int8_t pad[3];
}lcyt_work_stat_t;

typedef struct{
	u_int8_t mode;
	u_int8_t pad[3];
}lcyt_work_mode_t;

typedef struct{
	u_int8_t cur_temp;
	u_int8_t pad[3];
}lcyt_cur_temp_t;

typedef struct{
	u_int8_t set_temp;
	u_int8_t pad[3];
}lcyt_set_temp_t;

typedef struct{
	u_int8_t gear;
	u_int8_t pad[3];
}lcyt_set_gear_t;

typedef struct{
	u_int8_t time_type; /*1 ��ʱ�ػ� 2 ԤԼ����*/
	u_int8_t pad;
	u_int16_t time;
}lcyt_set_timer_t;

typedef struct{
	u_int16_t remain_time;
	u_int8_t time_type;
	u_int8_t pad;
}lcyt_remain_timer_t;

///////////////////////////////////////////////////////////////////////
//////////////////// ���ƿ��������� //////////////////////////////

//IA.HK_AIR_CLEANER.WORK    get/set
typedef struct{
	u_int8_t is_on;
	u_int8_t pad[3];
}hk_air_clean_work_t;

//IA.HK_AIR_CLEANER.MODE    get/set
#define HK_AIR_CLEAN_MODE_AUTO		1	//�Զ�
#define HK_AIR_CLEAN_MODE_MANUAL		2	//�ֶ�
#define HK_AIR_CLEAN_MODE_SLEEP		3	//˯��
typedef struct{
	u_int8_t mode; //HK_AIR_CLEAN_MODE_XXX
	u_int8_t pad[3];
}hk_air_clean_mode_t;

//IA.HK_AIR_CLEANER.WIND   get/set
typedef struct{
	u_int8_t wind;	//0 for set, 1,2,3,4 for get
	u_int8_t pad[3];
}hk_air_clean_wind_t;

//IA.HK_AIR_CLEANER.TEMP get
typedef struct{
	u_int16_t temp;  //���϶�
	u_int8_t pad[2];
}hk_air_clean_temp_t;

//IA.HK_AIR_CLEANER.PM25   get
typedef struct{
	u_int16_t pm25;  //��λ΢��/ÿ����
	u_int8_t pad[2];
}hk_air_clean_pm25_t;

//IA.HK_AIR_CLEANER.HUMIDITY    get
typedef struct{
	u_int16_t humidity;  //ʪ��
	u_int8_t pad[2];
}hk_air_clean_humitity_t;

//IA.HK_AIR_CLEANER.ANION_UVL    get/set
//�л����ƿ����������ĸ����ӡ�UVLɱ������������
//set����ȫ��0��
typedef struct{
	u_int8_t anion;  //������0-�رգ�1����
	u_int8_t uvl;  //UVLɱ����0-�رգ�1����
	u_int8_t pad[2];
}hk_air_clean_anion_uvl_t;

//IA.HK_AIR_CLEANER.TIMER    get/set  set ȫ��0
typedef struct{
	u_int16_t remain_minute;  // get����ʣ�����
	u_int8_t timer_type;  //��ʱ�����ͣ�1 ��ʱ����2 ��ʱ��
	u_int8_t set_hour;  //get�����趨��Сʱ
}hk_air_clean_timer_t;

///////////////////////////////////////////////////////////////////////
//////////////////// �ϰؿ��������� //////////////////////////////
typedef struct{//get or set
	u_int8_t is_on; //0: off, 1: on
	u_int8_t pad[3];
}nb_air_clean_work_t;

typedef struct{//get or set
	u_int8_t mode; //0 =  auto, 1 = manual
	u_int8_t pad[3];
}nb_air_clean_mode_t;

typedef struct{// get or set
	u_int8_t wind;	// low ::  1 < 2 < 3 < 4  :: high
	u_int8_t pad[3];
}nb_air_clean_wind_t;

typedef struct{// get 
	u_int16_t temp;
	u_int8_t pad[2];
}nb_air_clean_temp_t;

typedef struct{// get 
	u_int16_t pm25;
	u_int8_t pad[2];
}nb_air_clean_pm25_t;

typedef struct{// get
	u_int16_t humidity;  //ʪ��
	u_int8_t pad[2];
}nb_air_clean_humitity_t;

typedef struct{//get or set
	u_int8_t anion; //0: off, 1: on
	u_int8_t pad[3];
}nb_air_clean_anion_t;

typedef struct{//get or set
	u_int8_t is_on; //0: off, 1: on
	u_int8_t minute; //����ɱ��������
	u_int8_t pad[2];
}nb_air_clean_terilize_t;

typedef struct{// set or get
	u_int16_t minute;  // ��ʱ��������
	u_int8_t timer_type; // ��ʱ������
	u_int8_t pad[1]; 
}nb_air_clean_timer_t;

typedef struct{// set or get or push
	u_int32_t minute;  // ����ʹ����������λ����
}nb_air_clean_rosebox_life_t;

///////////////////////////////////////////////////////////////////////
//��Ѷ�����
#define GX_LED_QUERY_ALL (0x00)		//��ѯ����״̬����SDK�ڲ�ʹ��
#define GX_LED_OFF (0x01) 			// �ص�
#define GX_LED_ON (0x02) 			// ����
#define GX_LED_FULL_LIGHT (0x10) 	// ȫ��

#define GX_LED_WHITE (0x11) 		//�׹�
#define GX_LED_WARM (0x12) 			// ů��
#define GX_LED_MIX (0x13) 			// ���
#define GX_LED_LIGHT_OR_DARK (0x14) // ��/��
#define GX_LED_WHITE_OR_WARM (0x15) // ��/ů
#define GX_LED_RGB (0x16) 			// RGB

#define GX_LED_AUTO (0x20) 			// �Զ�ģʽ
#define GX_LED_SAVE (0x21) 			// ����ģʽ
#define GX_LED_NIGHT (0x22) 		// ҹ��
#define GX_LED_MODEL1 (0x23) 		// ģʽ1
#define GX_LED_MODEL2 (0x24) 		// ģʽ2

typedef struct {	
	u_int8_t action;	// ȡֵ�� GX_LED_AUTO	
	u_int8_t param1;	
	u_int8_t param2;	
	u_int8_t param3;
}lamp_gx_led_ctl_t;

typedef struct {
	u_int8_t led_status; // LED״̬�� ȡֵ����,�� GX_LED_STATUS_ON	
	u_int8_t brightness;		 // ����ֵ	
	u_int8_t warmness;		 // ů��ֵ	
	u_int8_t red;		 // RED	
	u_int8_t green;      // GREEN	
	u_int8_t blue;       // BLUE	
	u_int8_t night_status;// ҹ��״̬ 0-�رգ�1-����	
	u_int8_t reserved;	 // ����
}lamp_gx_led_stat_t;

/*
//�ʺ����̺
*/


#define CH_BLANKET_QUERY_ALL 0x0 //��ѯ״̬
#define CH_BLANKET_ON_OFF 0x1 //��������
#define CH_BLANKET_CURVE_ON_OFF 0x2 //��������
#define CH_BLANKET_CONFIG_CURVE 0x3 // ��������
#define CH_BLANKET_AREA_TEMP 0x4 // �����¶�
#define CH_BLANKET_CONFIG_TIMER 0x5 //���õ���ʱ
#define CH_BLANKET_SET_MODE 0x6 //���ù���ģʽ
#define CH_BLANKET_ACTION_MAX 0x7

typedef struct {
	u_int8_t cmd_type;
	u_int8_t area_num;
	u_int16_t curve_data_len;
	u_int8_t work_stat; //����
	u_int8_t set_temperature;//�û������¶�
	u_int8_t current_temperature; //������ǰ�¶�
	u_int8_t off_timer; //�ֹ���ʱʣ��ʱ��
	u_int8_t curve_enable; //�����Ƿ�����
	u_int8_t curve_week; //������������
	u_int16_t curve_time_interval;//���߼��ʱ������, 60����һ��Сʱ
	u_int16_t curve_next_work_time;//�´ο������߹ر�ʱ��
    u_int8_t work_mode; //����ģʽ
	u_int8_t pad;
	u_int8_t curve_data[0];
}blanket_ch_stat_pkt_t;

#pragma pack(pop)


bool ah_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
//RS ah_init(smart_appliance_ctrl_t* sac);
//void ah_free(smart_ah_ctrl_t* ac);
void ah_build_objs(user_t* user,cl_dev_info_t* ui);
void ah_quick_query_ia_info(smart_air_ctrl_t* ac);
bool ah_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
int ah_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
int ah_proc_get_or_push_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

//////////////////////////////////////////////////////////
//  ��͡

bool lcyt_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
void lcyt_quick_query_info(smart_air_ctrl_t* ac);
void lcyt_build_objs(user_t* user,cl_dev_info_t* ui);
bool lcyt_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
int lcyt_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

//////////////////////////////////////////////////////////
//  (����)����������

bool hkac_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
void hkac_quick_query_info(smart_air_ctrl_t* ac);
void hkac_build_objs(user_t* user,cl_dev_info_t* ui);
bool hkac_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
int hkac_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

//////////////////////////////////////////////////////////
//  (�ϰ�)����������

bool nbac_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
void nbac_quick_query_info(smart_air_ctrl_t* ac);
void nbac_build_objs(user_t* user,cl_dev_info_t* ui);
bool nbac_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
int nbac_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

//////////////////////////////////////////////////////////
// ��Ѷ�����
bool lamp_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
void lamp_quick_query_info(smart_air_ctrl_t* ac);
void lamp_build_objs(user_t* user,cl_dev_info_t* ui);
bool lamp_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
int lamp_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
/////////////////////////////////////////////////////////////////////
//�ʺ����̺
bool ch_blanket_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
void ch_blanket_quick_query_info(smart_air_ctrl_t* ac);
void ch_blanket_build_objs(user_t* user,cl_dev_info_t* ui);
bool ch_blanket_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
int ch_blanket_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

#endif

