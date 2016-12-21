#ifndef	__CL_RFGW_H__
#define	__CL_RFGW_H__

#include "cl_com_rf_dev.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define PSK_LEN 8
#define MAX_GW_COUNT 16
#define RF_MAX_NAME 16
#define MAX_SLAVE_PER_GW 128
    
//��չ����
enum{
    RF_EXT_TYPE_GW = 0x01, //������չ����
    RF_EXT_TYPE_LIGHT = 0x21,//��ɫ��0x21
    RF_EXT_TYPE_LED_LAMP = 0x22,//LED��
    RF_EXT_TYPE_DOOR_LOCK = 0x23, //��������/����
    RF_EXT_TYPE_DOOR_MAGNET = 0x24, //�����Ŵ�
    RF_EXT_TYPE_DHX = 0x25, //�����߿���
    RF_EXT_TYPE_YT_DOOR_LOCK = 0x26, //��̩����
    RF_EXT_TYPE_HM_MAGENT = 0x27,//�����Ŵ�
    RF_EXT_TYPE_HM_BODY_DETECT = 0x28,//��������̽��
    RF_EXT_TYPE_HM_ENV_DETECT = 0x29,//������ʪ��̽��

	RF_EXT_TYPE_DWHF = 0x2A,//�����Ϸ�оƬ��
	
    RF_EXT_TYPE_DOOR_MAGNETV2 = 0x2b,	// V2Ӳ���汾���Ŵ�
    RF_EXT_TYPE_KTCZ = 0x30,	// ���ز���
    RF_EXT_TYPE_HTLLOCK = 0x31,	// ��̩������
    RF_EXT_TYPE_GAS = 0x32,		// ������
    RF_EXT_TYPE_QSJC = 0x33,	// ˮ���
    RF_EXT_TYPE_DWYK = 0X34,	//��΢ң����
    RF_EXT_TYPE_HEATING_VALVE = 0x35,	// ů����
    RF_EXT_TYPE_HMCO = 0x36,	// һ����̼���    
    RF_EXT_TYPE_JQ = 0X37,	// ��ȩ������
    RF_EXT_TYPE_HMYW = 0x38,	// ������
    RF_EXT_TYPE_HMQJ = 0x39,	// ����������Ⱥ�����
    RF_EXT_TYPE_SCENE_CONTROLLER = 0x40,	// �龰ң����
    RF_EXT_TYPE_WK_AIR = 0x41, //�յ���
    RS_EXT_TYPE_ZHDJ = 0x42,//�ǻʵ��
    RF_EXT_TYPE_YLLOCK = 0x43,//ҹ���Ŵ�
    RF_EXT_TYPE_DWKJ = 0x44,	// �����Ƽ�
    RF_EXT_TYPE_YLTC = 0x45,//ҹ�Ǻ���
    RF_EXT_TYPE_YLWSD = 0x46, //ҹ����ʪ��
    RF_EXT_TYPE_YLSOS = 0x47,//ҹ��sos
    RF_EXT_TYPE_DWYKHF = 0x48,//��΢�Ϸ�ң����
    RF_EXT_TYPE_YLLIGHT = 0x4a,	// ҹ�����ⱨ����
    RF_EXT_TYPE_DHXZH = 0x4b, //�ǻʵ�����
    RF_EXT_TYPE_DHXCP = 0x4c, // ���ӵ�����
    RF_EXT_TYPE_DWYSTGQ = 0x4d,	// �������ٵ�����

	RF_EXT_TYPE_LAMP_START = 0x50,	// ������ϵ�п�ʼ����
	RF_EXT_TYPE_LAMP_END = 0x5f,	// ������ϵ�н�������

	
    RF_EXT_TYPE_WK_AIR2 = 0x61, //��������յ�
    RS_EXT_TYPE_YSD = 0x62, //���ٵ�
    RS_EXT_TYPE_CDQJMB = 0x63, //�ȵ��龰���
    RF_EXT_TYPE_LIGHT_SENSE = 0X64,	// ���ո�Ӧ
    RF_EXT_TYPE_DHXML = 0x66, //ħ�ֵ�����
    RF_EXT_TYPE_DEMO = 0x67,	// ��ʾ�ô��豸
    RF_EXT_TYPE_S9 = 0x68,		// S9����(��ƿ)
    RF_EXT_TYPE_LHX = 0x6a,		// RF�����
    RF_EXT_TYPE_LHX_CONTROLLER = 0x6b,	// RF�����ң����
    RF_EXT_TYPE_WUANS6 = 0x6c,	// ��S6���ܺ��������Ӧ
};
//����������
enum{
    GDT_NONE_SPCAIL = 0,
    GDT_LAMP_COLOR_CTRL
};

//ÿ������
enum {
	UP_STATUS_NONE = 0,
	UP_STATUS_NEED_UPGRADE,
	UP_STATUS_UPGRADING,
};

typedef struct{
	u_int64_t sn;
	u_int8_t subtype;
	u_int8_t extype;
	u_int8_t pad[2];
}cl_rfgw_dev_find_t;

//RGB��ɫ��״̬λ
#define	BIT(n)	(1 << (n))
#define RGB_WORK_R BIT(0)
#define RGB_WORK_G BIT(1)
#define RGB_WORK_B BIT(2)
//�Ŵ�״̬����λ��1��ʾ���ţ�0��ʾ����
#define MC_WORK_OPEN BIT(3)

//��������
enum{
    RF_GPT_KNOWN,
    RF_GPT_LAMP, //����
    RF_GPT_DOOR_LOCK, //������
    RF_GPT_DOOR_MAGNET,//�Ŵ���
    RF_GPT_DHX_SWITCH, //�����߿�����
    RF_GFT_HM_MAGNET,//�����Ŵ���
    RF_GFT_DWKJ_LAMP,	// �����Ƽ�����
};

typedef struct{
	u_int8_t work;
	u_int32_t time;
}cl_rfdev_work_t;
    
/* �豸�汾��Ϣ */
typedef struct {
    u_int8_t major;	/* ���汾 */
    u_int8_t minor;	/* �ΰ汾 */
    u_int8_t revise;	/* �޶��汾 */
    u_int8_t pad;		/* ����ֽ� */
} ud_version_t;

typedef struct{
	u_int8_t group_id;
	u_int8_t group_type;
	u_int8_t dev_cnt;
	u_int8_t query_dev;//��0��ʾ��ѯ�������Ա
	u_int8_t reserved;
	u_int8_t pad[3];
	u_int8_t name[RF_MAX_NAME];
	u_int64_t dev_sn[128];
}cl_dev_group_t;
    
typedef struct {
    u_int8_t slave_count; //������
    u_int8_t real_count; //ʵ������
    u_int8_t lamp_type; //�����ͣ��������ư��������ʾҳ��
    u_int8_t pad;
    u_int32_t* slave_handle;
}cl_lamp_remote_key_info;

//�����ƣ�ң������Ϣ
typedef struct {
    u_int32_t handle; //���ؾ��
    u_int32_t r_id; //ң���� id��
    u_int8_t lamp_type; //�����ͣ�ң�������ư��������ʾҳ��
    u_int8_t pad[3];
    cl_lamp_remote_key_info key[4];
}cl_lamp_remote_info_t;

//�����������ݽṹ
//ҹ�����ݶ���
enum {
	RFDEV_SCM_TYPE_YL = 1,//ҹ��������������
};
typedef struct {
	u_int8_t is_guard; //�Ƿ��ڲ���״̬
	u_int8_t battery;//0 ��100
	u_int8_t power;//0��أ�1��ӵ�Դ
	u_int8_t sos_on;//SOS����ָ�� ��0���ޣ�1��SOS������
	u_int8_t key_info;//������Ϣ��0 �� 255�������߼�����
	u_int8_t door_voice;//����������С��0 �� 100
	u_int8_t alarm_voice;//����������С��0 �� 100
	u_int8_t time;//����ʱ�䣺0 - 255��
}cl_rfdev_scm_yl_t;

typedef struct {
	u_int8_t valid;
	union {
		cl_rfdev_scm_yl_t yl_info;
	}rfdev_scm_dev_data;
}cl_rfdev_scm_t;

// ��ƿ����
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
    u_int32_t r_id;
} hpgw_lamp_ctrl_param_t;


// ��ƿ���ص�һЩ������Ϣ
typedef struct {
	u_int8_t name[24];		// �û���
	u_int64_t phome_number;	// �ֻ���
} cl_hpgw_phone_user_t;

typedef struct {
	u_int8_t support_appinfo;	// �Ƿ�֧�����Ͷ����ϵ��APP��Ϣ
	u_int8_t support_sms;		// �Ƿ�֧�ֶ���
	u_int8_t sms_lang;			// �������� 0 ���� 1 Ӣ��
	u_int8_t phone_user_num;	// �ֻ��û����������16��
	cl_hpgw_phone_user_t users[16];

	cl_rf_lamp_t lamp_stat;	// ��״̬
} cl_hpgw_info_t;

typedef struct{
	// ��ƿ����
	cl_hpgw_info_t hpinfo;
	u_int8_t support_hpinfo;	// ֧�ֻ�ƿ�����������
	
	u_int8_t commpat;//0Ĭ�ϼ��ݼ���1~255�����ݼ�������Խ��Խ�������ϰ汾����ʹ�������ԡ�
	u_int8_t channel;//0Ĭ���ŵ���1~255Ϊ�ŵ��ţ�2400+�ŵ���Ϊ����ʹ��Ƶ�ʡ�
	u_int8_t is_upgrade;//0û�������У�1��������
	
	u_int8_t dev_group_cnt;
	cl_dev_group_t *dev_group;
    u_int8_t lamp_remote_cnt;
    cl_lamp_remote_info_t* lr_info;
	u_int8_t upgrade_status[D_T_MAX];	//rt����������
	char *upgrade_url[D_T_MAX];//����url

	u_int8_t img_cache_num;
	cl_rfdev_img_cache_info_t *pimg_cache;

	//����͸����������
	cl_rfdev_scm_t rfdef_scm_dev;
}cl_gw_info_t;

typedef struct {
	u_int32_t id;			// ���ù���
	u_int32_t record_time;	// ��¼ʱ��
	u_int16_t object;	// ���ù���
	u_int16_t type; 	// �������ͣ������CAT_TYPE_XXX
	u_int32_t value;	// ��ͬ������˼��һ��
} cl_rf_dev_comm_alarm_info_t;

typedef struct {
	u_int8_t type;
	u_int8_t value;
	u_int8_t ex_type;
	u_int8_t ex_value;
	u_int32_t timestamp;
	u_int8_t valid;	// �Ƿ���Ч
} cl_rf_dev_comm_history_item_t;

typedef struct {
	// �����������ݵ�SAE_RF_DEV_COMM_HISTORY_SUMMARY �¼�����ʱ����
	u_int32_t index_current;	// ��ʷ��¼��ǰ���������磺index_current == 400����ô��ǰ��ʷ��¼�������Ϊ400
	/*
		
		��ʷ��¼���������磺max_count == 100����ô�豸��ǰ���е���ʷ��¼Ϊ100��
		���磺index_current = 1000�� max_count = 100����ô��Ч����ʷ��¼����Ϊ��
			  901 -- 1000
	*/
	u_int32_t max_count;			

	// �����������ݵ�SAE_RF_DEV_COMM_HISTORY_ITEM�¼�����ʱ����
	u_int16_t index;	// ��ʾ������ʷ��¼������ֵ��16λ����16λ��������
	u_int16_t n;		// ����һ����������ʷ��¼�����3��
	cl_rf_dev_comm_history_item_t items[3];
} cl_rf_dev_comm_history_info_t;

typedef struct _cl_rfdev_status_s{
	u_int64_t ctrl_total;
	u_int32_t ctrl_min;
	u_int32_t ctrl_max;
	u_int32_t ctrl_ok;
	u_int32_t ctrl_fail;
	u_int32_t ctrl_msec;
	cl_rfdev_work_t *work_list;
	u_int32_t rfrx; // from RF device
	u_int16_t linkretry; // from RF device
	u_int16_t work_list_cnt;
	u_int8_t work;  // from RF device
	u_int8_t is_ctrl;
    cl_slave_data_type d_type; //��������
    //////////////////////////////////////
    ud_version_t soft_ver; //�ն�����汾
    ud_version_t hardware_ver; //�ն�Ӳ���汾
    ////////////////////////////////////
    // ͨ�õı�����Ϣ������һ����SDKͨ��cache֪���б�����Ϣ��Ȼ������ȥ��ѯ
    cl_rf_dev_comm_alarm_info_t cai;

	// ͨ�õ���ʷ��¼
	cl_rf_dev_comm_history_info_t chi;

	/*
		
	��Ӧ����bitλֵΪ1��ʾ֧�֣�0��ʾ��֧��
	bit0:����/������������
	bit1:����/����״̬�ϱ�����
	bit2:������������
	bit3:ִ�б�������
	bit4:��/�ؿ�������
	bit5:��/��״̬�ϱ�����
	bit6:���ܱ�������
	*/
	u_int8_t dev_support;
	
    union{
        cl_door_lock_info_t door_lock_info;
        cl_rf_lamp_t lamp_info;
        cl_door_magnet_info_t door_magnet_info;
        cl_dhx_switch_info_t dhx_info;
        cl_hm_body_info_t hb_info; //��������̽��
        cl_hm_temp_hum_info_t ht_info; //������ʪ��
        cl_heating_valve_info_t hv_info;	// ���ů����
        cl_ktcz_info_t kt_info;				// ���ز���
        cl_com_detector_info_t cd_info;		// ͨ�õ�̽���豸
        cl_htllock_info_t hl_info;	// ��̩����
        cl_wk_air_info_t wk_air_info; //��տյ���
        cl_zhdj_info_t zhdj_info;//�ǻʵ��
        cl_dwkj_info_t dwkj_info;	// �����Ƽ�
        cl_scene_controller_info_t sc_info;	// �龰ң����
        cl_yllight_info_t yllight_info;	// ҹ�����ⱨ����
        cl_cdqjmb_info_t cdqjmb_info;//�ȵ��龰ģ��
        cl_jq_info_t jq_info;	// ��ȩ������
        cl_light_sense_info_t ls;	// ���
    }dev_priv_data;
}cl_rfdev_status_t;


typedef struct {
	ud_version_t ver;
	ud_version_t upver;
}cl_rf_dev_debug_info_t;


/*
	����:
		ָʾ���ؽ������ģʽ
	�������:
		@gw_handle: ���صľ��
		@timeout: �������ģʽ��ĳ�ʱʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rfgw_join(cl_handle_t gw_handle, u_int16_t timeout);
//UE_RFGW_DEV_FIND
CLIB_API cl_rfgw_dev_find_t *cl_rfgw_get_join_dev(cl_handle_t gw_handle, u_int8_t *cnt);
CLIB_API void cl_rfgw_free_join_dev(cl_rfgw_dev_find_t *dev);
CLIB_API RS cl_rfgw_join_action(cl_handle_t gw_handle, u_int64_t dev_sn, u_int16_t accept);
CLIB_API RS cl_rfgw_group(cl_handle_t *gw_handle, u_int8_t gw_count, u_int8_t *psk);

/*
	����:
		ɾ���Ѿ���Ե��豸
	�������:
		@gw_handle: ���ؾ��
		@dev_handle: �豸�������
		@cnt: �豸������������32��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rfgw_dev_delete(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt);

/*
	����:
		ɾ���Ѿ���Ե������豸
	�������:
		@gw_handle: ���ؾ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rfgw_dev_delete_all(cl_handle_t gw_handle);


/*
	����:
		�����ز�ѯ�豸�б�
	�������:
		@gw_handle: ���صľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/

CLIB_API RS cl_rfgw_query_devlist(cl_handle_t gw_handle);
/*
	����:
		����͸�����ݵ��豸
	�������:
		@dev_handle: �豸�ľ��
		@data: ͸������
		@len: ͸�����ݳ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rfgw_set_tt(cl_handle_t dev_handle, u_int8_t *data, u_int16_t len);

/*
	����:
		�����豸͸������
	�������:
		@dev_handle: �豸�ľ��		
	�������:
		@len: ͸�����ݳ���
		��
	����:
		���ݵ�ַ: �ɹ�
		NULL: ʧ��
	˵��:
		UE_RFGW_DEV_TT �¼�ʱ���ñ�����

*/
CLIB_API u_int8_t *cl_rfgw_get_tt(cl_handle_t dev_handle, u_int16_t *len);
CLIB_API void cl_rfgw_free_tt(u_int8_t *data);

/*
	����:
		�����豸��Դ
	�������:
		@handle: �豸�ľ��	���Ƶ����豸�����ؾ�����������������豸
		@r: true��ʾ��ƿ���false��ʾ��Դ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
	˵��:
	

*/
CLIB_API RS cl_rfdev_rgb(cl_handle_t handle, bool r, bool g, bool b);

/*
	����:
		���������豸��Դ
	�������:
		@gw_handle: ���ؾ��
		@dev_handle: �豸�������
		@cnt: �豸������������32��
		@rgb: ��ɫ�ƿ��ƣ��ο�RGB_WORK_R
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
	˵��:	

*/
CLIB_API RS cl_rfdev_rgb_batch(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt, u_int8_t rgb);

/*
	����:
		������ѯ�豸����״̬
	�������:
		@gw_handle: ���ؾ��
		@dev_handle: �豸�������
		@cnt: �豸������������32��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
	˵��:	

*/
CLIB_API RS cl_rfdev_work_query(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt);

/*
	����:
		��ѯ����
	�������:
		@gw_handle: ���ؾ��
		@group_id: ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_dev_group_query(cl_handle_t gw_handle, u_int8_t group_id);

/*
	����:
		��������·���
	�������:
		@gw_handle: ���ؾ��
		@group_id: ����id
		@group_name: ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_dev_group_set(cl_handle_t gw_handle, cl_dev_group_t *group);

/*
	����:
		ɾ������
	�������:
		@gw_handle: ���ؾ��
		@group_id: ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_dev_group_delete(cl_handle_t gw_handle, u_int8_t group_id);


/*
 ����:
    �������ָ��
 �������:
    @gw_handle: ���ؾ��
    @group_id: ����id
    @flag:Ĭ����GDT_NONE_SPCAIL������ǵ������ɫ��Щ���ƣ���GDP_LAMP_COLOR_CTRL����ΪҪת�����ݣ�
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rf_dev_group_ctrl(cl_handle_t gw_handle, u_int8_t group_id,u_int32_t flag,u_int8_t* data,int len);
    
/*
	����:
		�޸Ĵ��豸����
	�������:
		@dev_handle: ���豸���
		@name: ���豸���ƣ��15�ֽ�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_dev_name_set(cl_handle_t dev_handle, u_int8_t *name);

/*
	����:
		�޸Ĵ��豸����
	�������:
		@dev_handle: ���ؾ��
		@upgrade_type: ���豸��������
		@filepath:�����ļ�·��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_rfdev_upgrade(cl_handle_t gw_handle, u_int32_t upgrade_type, u_int8_t *filepath);

/*
	����:
		����ģʽ����
	�������:
		@gw_handle: ���ؾ��
		@commpat: ���ؼ���ģʽ0,1,2
		@channel:0Ĭ���ŵ���1~255Ϊ�ŵ��ţ�2400+�ŵ���Ϊ����ʹ��Ƶ�ʡ�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_dev_commpat(cl_handle_t gw_handle, u_int8_t commpat, u_int8_t channel);

/*
	����:
		����״̬��ѯ
	�������:
		@gw_handle: ���ؾ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_dev_up_query(cl_handle_t gw_handle);


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
CLIB_API RS cl_rfdef_comm_timer_modify_add(cl_handle_t dev_handle, cl_comm_timer_t *ptimer);

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
CLIB_API RS cl_rfdev_comm_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
����: 
    ͨ�ö�ʱ����ѯ�����½ʱ��ѯ���л�����ʱ��ҳ��ʱ��ѯ
    @id: ��ʱ��id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rfdev_comm_timer_query(cl_handle_t dev_handle);


/*
����: 
    ͨ�ò�ѯ���豸��ʷ��¼
    @index: ��ʷ��¼����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rfdef_comm_history_query(cl_handle_t dev_handle, u_int32_t index);

/*
����: 
    ��ѯrf���豸�汾�ţ���Ҫ��pc��������ã�app���ù�
    @id: ��ʱ��id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rfdev_debug_info_query(cl_handle_t dev_handle, cl_rf_dev_debug_info_t *ver);

/*
 ����:
    rf���豸��ݿ��ػ�
 �������:
    @dev_handle �豸���
    @enable: �Ƿ�ʹ��
    @onoff: 0: �ػ� 1����
    @time: ʣ��ʱ�䣬������λ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rfdev_public_set_shortcuts_onoff(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int32_t time);

/*
 ����:
    rf���豸��ݿ��ػ���ѯ��������Ҫ��ʱ��ѯ
 �������:
    @dev_handle �豸���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rfdev_public_query_shortcuts_onoff(cl_handle_t dev_handle);

/**************************************����͸��������ƽӿ�**********************************************************************************/
//����͸��������ƽӿ�
/*
 ����:
    ҹ�Ǿ���ʱ������
 �������:
    @gw_handle:���ؾ��
    @time:����ʱ��:0-255
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rfdev_set_yl_time(cl_handle_t gw_handle, u_int8_t time);

/*
 ����:
    ҹ������������С����
 �������:
    @gw_handle:���ؾ��
    @type:1-��ʾ������Ϊ��������������2-��ʾ������Ϊ��������������
    @voice:0-100
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rfdev_set_yl_voice(cl_handle_t gw_handle, u_int8_t type, u_int8_t voice);

/*
 ����:
    ҹ�Ǿ��ѹر�
 �������:
    @gw_handle:���ؾ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rfdev_set_yl_siren_off(cl_handle_t gw_handle);

/*
 ����:
    ҹ�Ǿ��ѿ�������
 �������:
    @gw_handle:���ؾ��
    @onoff:1,������0�ر�
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rfdev_set_yl_siren_onoff(cl_handle_t gw_handle, u_int8_t onoff);

/*
*ָ������sn��ѯ�Ƿ���豸��������
*/
CLIB_API RS cl_rfdev_up_check(cl_handle_t gw_handle);

/*
����: 
	rf�豸���澵���ѯ
	@gw_handle
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rfdev_img_cache_query(cl_handle_t gw_handle);

/*
����: 
	rf�豸���澵��ɾ��
	@gw_handle
	@pindex, ɾ����ʼ�Ļ����±�����
	@num, ɾ������
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rfdev_img_cache_del(cl_handle_t gw_handle, u_int8_t num, u_int8_t *pindex);


/*
����: 
	��ƿ���������Ƿ����Ͷ����ϵ���Ϣ
	@gw_handle
	@onoff: ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hpgw_config_appinfo_onoff(cl_handle_t gw_handle, u_int8_t onoff);

/*
����: 
	��ƿ�������ö���
	@gw_handle
	@sms_onoff: �Ƿ�������
	@lang: ��������
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hpgw_config_sms(cl_handle_t gw_handle, u_int8_t sms_onoff, u_int8_t lang);

/*
����: 
	��ƿ�������ö����û���ע�⣬��������Ѿ�������ֻ��ţ��Ǿ����޸�����
	@gw_handle
	@name: �û��Զ�������
	@phone_number: �ֻ���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hpgw_config_phone_user(cl_handle_t gw_handle, u_int8_t *name, u_int64_t phone_number);

/*
����: 
	��ƿ����ɾ��һ���ֻ��û�
	@gw_handle
	@sms_onoff: �Ƿ�������
	@phone_number: �ֻ���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hpgw_del_phone_user(cl_handle_t gw_handle, u_int64_t phone_number);

/*
����: 
	��ƿ���صƿ���
	@gw_handle
	@stat: ���Ʋ���
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hpgw_lamp_ctrl(cl_handle_t gw_handle, cl_rf_lamp_stat_t *stat);

#ifdef __cplusplus
}
#endif 


#endif

