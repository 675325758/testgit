#ifndef	__COMMON_UDP_DEVICE__
#define	__COMMON_UDP_DEVICE__
#include "cl_smart_appliance.h"
#include "cl_lanusers.h"

enum {
    COMMON_UE_BEGIN = 2000,
    //�豸�쳣��Ϣpush 
    COMMON_UE_DEV_ERR_PUSH_MSG = COMMON_UE_BEGIN + 1,
    COMMON_UE_DEV_COMM_TIMER_ADD_FAILED = COMMON_UE_BEGIN + 2,
    COMMON_UE_DEV_COMM_TIMER_DEL_FAILED = COMMON_UE_BEGIN + 3,
    //��ʱ��ʱ���ͻ�¼�
    COMMON_UE_DEV_COMM_TIMER_TIME_CONFLICT = COMMON_UE_BEGIN + 4,

    COMMON_UE_END = COMMON_UE_BEGIN + 99
};

typedef struct{
	u_int32_t err_id;
	u_int32_t err_time; 
	u_int16_t err_obj_type;
	u_int16_t err_type;
	u_int32_t err_data;
}cl_dev_err_item_t;

typedef struct{
	u_int8_t err_count;
	u_int8_t pushed_err_count;
	u_int8_t pad[2];
	cl_dev_err_item_t * err_items;
	cl_dev_err_item_t* pushed_err;
}cl_dev_err_info_t;


// ͨ�õ�WIFI�豸��ʷ��¼
typedef struct {
	u_int32_t id;
	u_int32_t time;
	u_int8_t type;
	u_int8_t condition;
	u_int8_t action;
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t temp;
	u_int8_t wind;
	u_int8_t winddir;
	u_int8_t key;
	u_int8_t valid;
} cl_wukong_history_item_t;

typedef struct {
	u_int32_t index;
	u_int32_t num;
} cl_dev_history_item_hd_t;

typedef struct {
	union {
		cl_wukong_history_item_t wukong;
	} u;
} cl_dev_history_item_t;

typedef struct {
	
	// ����2���������յ�SAE_DEV_COMM_HISTORY_SUMMARY = 1241 ���ȡ
	u_int32_t index_current;
	u_int32_t max_count;

	// ����2���������յ�SAE_DEV_COMM_HISTORY_ITEM = 1242���ȡ
	u_int32_t n;
	u_int32_t index;	// ����item �ĵ�һ��index��������Ƿ���IOSͬѧʹ��
	cl_dev_history_item_t *item;
} cl_dev_history_t;

typedef struct cl_com_udp_device_data_s{
	/*����һ�����ͣ�����App ��ȷ��������ȷ��*/
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t is_lan_connect; //�Ƿ����������0: δ֪, 1 �������ӣ�2 ��������
	u_int8_t is_all_data_update; //�Ƿ����ݸ������, 0, ������, 1 �������

	u_int8_t is_system_info_valid; //ϵͳ�汾״̬�Ƿ��ȡ������
	u_int8_t is_stat_info_valid;// ״̬�Ƿ��ȡ���
	u_int8_t is_support_period_timer; //�Ƿ�֧�������Զ�ʱ��
	u_int8_t is_suppport_elec_stat; // �Ƿ�֧�ֵ���ͳ��
    u_int8_t is_support_ext_period_timer; //�Ƿ�֧����������չ��ʱ�� 
	
	u_int8_t is_support_dev_err_info; //�Ƿ�֧���豸�쳣��Ϣ
	u_int8_t is_support_stm_upgrade; //֧�ֵ�Ƭ����������
	u_int8_t is_support_dev_restory_factory; //�Ƿ�֧�ָֻ���������
	u_int8_t is_support_dev_set_wifi_param; //֧������wifi��������

	u_int8_t is_support_disk;//�Ƿ�֧��Ӳ����Ϣ��ȡ
	u_int8_t is_support_eth;//�Ƿ�֧��������Ϣ��ȡ
    
    u_int8_t is_support_room_temp_ajust;//����У��
    u_int8_t is_support_elec_ajust;//����У��
    u_int8_t is_support_rc; //֧�ֵ��ӡ������п���
    u_int8_t pad;
    
    int16_t env_room_temp_low; //���У������
    int16_t env_room_temp_high; //���У������
    int16_t  env_temp_ajust_value; //�����¶�У��ֵ �Ŵ�10��������������ʾ -5-5�ȣ���ֵ -50-50
    int16_t  elec_ajust_value; //����У��ϵ�� �Ŵ�100�� ��:������ʾ0.5-1.5����ֵΪ 50-150

	u_int8_t is_support_dev_wifi_state;	// ֧���豸����ʾWIFI����״̬
	/*
	      0: WIFIδ����
	      1�����ӳɹ�   //��ʾ�豸�ɹ����ӵ�wifi �һ�ȡ��ip��ַ����������·ͨ
	      2�������С�����
	      3�����ӳɹ������ڻ�ȡip��ַ
	      4��������·��ͨ
	      5������apʧ��
	*/
	u_int8_t dev_wifi_state;		// �豸��WIFI״̬
	//timezone
	u_int8_t timezone_valid;
	u_int32_t timezone;

	u_int8_t is_support_wan_config;	// �Ƿ�֧������������
	cl_wan_config_t wan_config;		// ����������

	u_int8_t is_support_dhcp_server;	// ֧��DHCP����������
	cl_dhcp_server_config_t dhcp_config;	// dhcp����������

	u_int8_t is_support_ap_config;	// �Ƿ�֧��AP����
	cl_ap_config_t ap_config;		// AP����

	u_int8_t is_support_repeat;		// �Ƿ�֧���м���
	u_int8_t repeat_onoff;			// �м������� 0 �� 1 ��
	///////////////////////////////////////////////////////////////////////
	//  ͨ�ö�ʱ����Ϣ
	cl_air_timer_info_t timer_info; // ��ʱ����Ϣ
	cl_dev_timer_summary_t timer_summary;//ͨ�ö�ʱ����ժҪ��Ϣ
	cl_comm_timer_head_t comm_timer_head;//ͨ�ö�ʱ������������豸
	//////////////////////////////////////////////////////////////////////////
	//ͨ�õ���ͳ��
	u_int32_t current_power; // ��ǰ����
	u_int32_t cur_milli_power; //��ǰ���ʣ�����
	cl_elec_stat_info_t elec_stat_info; //����ͳ����Ϣ
	cl_air_elec_item_info total_elec; //�ܵ���
	cl_air_elec_item_info period_elec; //�׶ε���
	cl_air_elec_item_info last_on_elec; //�ϴο�������
	cl_elec_days_stat_info elec_days_info; //����ͳ��֮365��ͳ������
	/////////////////////////////////////////////////////////////////////////////////////////////
	//�豸�쳣��Ϣ
	cl_dev_err_info_t dev_err_info;

	// �豸��ʷ��¼
	u_int8_t is_support_dev_history;
	cl_dev_history_t dev_history;

	// ͨ�õĿ����¶�
	u_int8_t is_support_boot_temp;
	u_int8_t boot_temp_enable;	// �Ƿ����Զ�����
	u_int8_t boot_temp;			// �Զ������¶�

	// �û���������Ϣ
	bool support_lanusers_manage;
	cl_lanusers_manage_info_t lum_info;
	
	// //////////////////////////////////////////////////////////////////////////////////////////
	// ͨ��������Ϣ
	u_int8_t is_support_utc_temp_curve;		// ֧��ͨ�������µ��¶�����
	u_int8_t has_utc_temp_curve_data;		// ���¶���������
	int temp_curve_len;	//ͨ���¶��������ݳ��ȣ���Ϊ����֧�ֶ�����ߣ�Ŀǰ����ֻ��һ��,len���count���Խ������������
	cl_temp_curve_t *temp_curve;//ͨ���¶���������

    cl_24hour_line room_temp_line; // 24Сʱ�¶�����
    cl_24hour_line humi_line; //24Сʱʪ������
	u_int8_t room_temp;/*����*/
	u_int8_t temp_humidity; //ʪ�ȣ���ǿ����ղ���Ч
	
	u_int8_t is_support_public_utc_temp_ac_ctrl;	// ֧��ͨ�������µ��¶ȿ��ƣ��ṹ��air_ctrl->air_info.ta
	u_int8_t has_utc_temp_ctrl_data;	// ���¶ȿ�������
	cl_temp_ac_ctrl_t tac; //�����¶ȿ��ƿյ�
	
	u_int8_t is_support_public_smart_on;			// ֧��ͨ�õ����ܿ�������
	u_int8_t smart_on;								// ͨ�õ����ܿ��ػ�
	
	u_int8_t is_support_public_child_lock;			// ֧��ͨ�������µ�ͯ��
	u_int8_t child_lock_value;						// ͨ�õ�ͯ��

	u_int8_t is_support_public_temp_alarm;			// ֧��ͨ�������µ��¶ȷ�ֵ����
	u_int8_t temp_alarm_onoff;						// ͨ���¶ȷ�ֵ��������
	u_int8_t temp_alarm_min;						// ͨ���¶ȷ�ֵ������С�¶�	
	u_int8_t temp_alarm_max;						// ͨ���¶ȷ�ֵ��������¶�
	
	u_int8_t is_support_public_shortcuts_onoff;		// ֧��ͨ�õĿ�ݿ��ػ�
	cl_shortcuts_onoff_t shortcuts_onoff;			// ͨ�õĿ�ݿ��ػ� 

	bool has_recv_flag_pkt;//�Ƿ��־��Ч
	u_int8_t is_support_la;//�Ƿ�֧����������

	u_int8_t flag_stat_update_ok;	// ֧�ֱ�־�Ѿ�ˢ�¹�

	u_int8_t hardware_led_ver;	// ���ܱ�LEDӲ���汾

	u_int8_t is_support_spe_up;//�Ƿ�֧��ָ������
	//////////////////////////////////////////////////////////////////////////////////////////
	//  �豸˽����Ϣ
	void * device_info; //˽���豸��Ϣ�������豸��ͬ
}cl_com_udp_device_data;

#define UTC_TIME_TO_LOCAL(time) (time+cl_priv->zone*3600)

#ifdef __cplusplus
extern "C" {
#endif 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      ͨ�ö�ʱ������
/*
	����:
		ɾ����ʱ���ع���
	�������:
		@dev_handle: �豸�ľ��
		@id: Ҫɾ���Ĺ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_com_udp_timer_set(cl_handle_t dev_handle, cl_air_timer_t *timer);

/*
	����:
		ɾ����ʱ���ع���
	�������:
		@dev_handle: �豸�ľ��
		@id: Ҫɾ���Ĺ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_com_udp_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	����:
		��ӻ��޸Ķ�ʱ���ع���
	�������:
		@dev_handle: �豸���
		@timer: ��ʱ����Ŀ�Ĳ�����idΪ0��ʾ��ӣ�������Ϊ�޸�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_com_udp_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer);

    
/*
 ����:
    ��ӻ��޸Ķ�ʱ���ع���
 �������:
    @dev_handle: �豸���
    @timer: ��ʱ����Ŀ�Ĳ�����idΪ0��ʾ��ӣ�������Ϊ�޸�
    @other_param,��չ����������ĳ�豸��Ҫ�������������ʽ��APP��SDKЭ��
    @param_len����չ��������
 /////////////////
    ǧ������ other_param == cl_qpcp_scp_t; param_len == sizeof(cl_qpcp_scp_t)
    ���  other_param == cl_808_timer_ext_t,param_len = sizeof(cl_808_timer_ext_t)
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_ext_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer,void* other_param,u_int16_t param_len);
/*
	����:
		ɾ����ʱ���ع���
	�������:
		@dev_handle: �豸�ľ��
		@id: Ҫɾ���Ĺ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_com_udp_period_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	����:
		ˢ�¶�ʱ��
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API void cl_com_udp_refresh_timer(int dev_handle);

/*
	����:
		ˢ���豸��������
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API void cl_com_udp_refresh_dev_all_info(int dev_handle);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//     ͨ�õ���ͳ�ƴ���

/*
 ����:
    ���÷�����ʱ��
 �������:
    @begin_time: ��ʼʱ�� ��λ������  0�� 0��0��  1439��23ʱ59��
    @last_time:  ����ʱ�� ��λ������  max�� 1440
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_peak_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
/*
 ����:
    ���ùȵ����ʱ��
 �������:
    @begin_time: ��ʼʱ�� ��λ������  0�� 0��0��  1439��23ʱ59��
    @last_time:  ����ʱ�� ��λ������  max�� 1440
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_valley_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
    
/*
 ����:
    ���÷��۸�
 �������:
    @price: ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_peak_price(cl_handle_t dev_handle, u_int32_t price);

/*
 ����:
    ���ùȵ�۸�
 �������:
    @price: ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_valley_price(cl_handle_t dev_handle, u_int32_t price);

/*
 ����:
    ����ƽ��۸�
 �������:
    @price: ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_flat_price(cl_handle_t dev_handle, u_int32_t price);

/*
 ����:
    ˢ�µ���ͳ������
 �������:
    @dev_handle: �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_refresh_elec_info(cl_handle_t dev_handle);

/*
 ����:
    ������ڵ���ͳ����Ϣ
 �������:
    @dev_handle �豸���
    @type: 0x0: �������ͳ����Ϣ
               0x1: �����������ĵ���ͳ����Ϣ��
               0x2: ����ۼ�ͳ����Ϣ
               0x3: ������е�����Ϣ
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_clear_elec_stat_info(cl_handle_t dev_handle,int type);
    
/*
 ����:
    ���õ���У��ϵ��
 �������:
    @dev_handle �豸���
    @value: 50-150
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_elec_ajust_value(cl_handle_t dev_handle,int16_t value);

/*
 ����:
    ����豸�쳣��Ϣ
 �������:
    @dev_handle �豸���
    @err_id ,�쳣ID �ţ�0:��ʾ���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_clear_dev_err_info(cl_handle_t dev_handle , u_int32_t err_id);

/*
 ����:
    ˢ���豸�쳣��Ϣ
 �������:
    @dev_handle �豸���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_refresh_dev_err_info(cl_handle_t dev_handle);

/*
 ����:
    ��������������Ƭ��
 �������:
    @dev_handle �豸���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_permit_stm_upgrade(cl_handle_t dev_handle);

/*
 ����: //����ʹ��
    ���豸�ָ�����
 �������:
    @dev_handle �豸���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_dev_restore_factory(cl_handle_t dev_handle);


/*
 ����: 
    �����豸��ssid������
 �������:
    @dev_handle �豸���
 @as
    �������:
 ��
    ����:
 RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_reset_dev_ssid_and_passwd(cl_handle_t dev_handle,const char* ssid,const char* password);
    
/*
 ����:
    ��������У���Ĳ���
 �������:
    @dev_handle �豸���
    @value -50 -- 150
    @value
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_com_udp_set_env_temp_ajust_value(cl_handle_t dev_handle,int16_t value);
    
/*
 ����:
    ˢ����ʪ������
 �������:
    @type 0x0 �¶�
          0x1 ʪ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
    
CLIB_API RS cl_com_udp_refresh_24hour_line(cl_handle_t dev_handle,u_int8_t type);
    
/*
 ����:
 	���豸�˻�ȡһ�������룬����߱�����Ȩ�޵Ŀͻ��˲ſ��Ի�ȡ
 �������:
 �������:
 	��
 ����:
 	RS_OK: �ɹ�
 	����: ʧ��
 */
CLIB_API RS cl_com_udp_request_share_code(cl_handle_t dev_handle);
    
/*
 ����:
 	ɾ��һ���ѷ�����ֻ�
 �������:
 	@share_index ����index
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_del_shared_phone(cl_handle_t dev_handle,u_int32_t share_index);

/*
 ����:
 	ˢ���ѷ����ֻ��б�
 �������:
 �������:
 	��
 ����:
 	RS_OK: �ɹ�
 	����: ʧ��
 */
CLIB_API RS cl_com_udp_refresh_shard_list(cl_handle_t dev_handle);

/*
 ����:
 	�޸�һ���ֻ�������
 �������:
 	@share_index ����index
 	@desc �ֻ������ַ���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_com_udp_modify_shared_phone(cl_handle_t dev_handle,u_int32_t share_index, char *desc);

#ifdef __cplusplus
}
#endif 


#endif

