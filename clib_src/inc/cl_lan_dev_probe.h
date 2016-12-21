#ifndef	__CL_LAN_DEV_PROBE_H__
#define	__CL_LAN_DEV_PROBE_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "client_lib.h"

//�豸��ǰ����ģʽ
typedef enum enum_dev_run_mode{
    DR_MODE_SLAVE, //���豸
    DR_MODE_MASTER, //���豸ģʽ
    DR_MODE_INDPD  //����ģʽ��������ͷ
}dev_run_mode_t;

//������ɨ��tlv��Ϣ���Ƿ�֧��������homeid�Ƕ���?
typedef struct {
	u_int8_t is_valid;//�Ƿ���Ч��Ϊ1ʱ�������ݲ���Ч
	u_int8_t is_la_new; //�Ƿ�Ϊ���豸
	u_int8_t back_is_la_new;
	u_int32_t home_id;
}cl_la_lan_info_t;
    
typedef struct _cl_lan_dev_info_s{
    cl_handle_t handle; //device unique handle
    u_int64_t dev_sn; //�豸���к�
    u_int32_t dev_type; //IJ_XXX
    u_int32_t last_alive_time; //���һ�δ��ʱ��
    u_int32_t ip_addr;//������ip��ַ
    u_int32_t sm_success_time;//һ�����óɹ������
    dev_run_mode_t dev_run_mode; //��ǰ����ģʽ
    u_int8_t is_upgrading; //����������־
    u_int8_t evm_is_upgrading;
    u_int8_t is_udp_ctrl; // �Ƿ�֧��UDP����
    u_int8_t exp_type;
    u_int8_t real_ext_type;
	u_int8_t developer_id[32];	// ������ID
	cl_la_lan_info_t la_info;//����������ɨ������
}cl_lan_dev_info;
    
typedef struct _cl_lan_dev_list_s{
    u_int32_t dev_count;
    cl_lan_dev_info info[0];
}cl_lan_dev_list;
    
// �������豸ɨ�� event
enum {
    LDPE_BEGIN = 900,
    LDPE_DEVICE_CHANGED,
    LDPE_DEV_AUTH_OK,
    LDPE_DEV_AUTH_FAIL,
    LDPE_WIFI_CFG_OK,
    LDPE_WIFI_CFG_FAIL,
    LDPE_TIMEOUT_FAIL,
    LDPE_PHONE_CONFIG_DEV_OK,
    LDPE_END = LDPE_BEGIN + 99
};

CLIB_API RS cl_set_probe_callback(cl_callback_t callback, void *handle);

CLIB_API RS cl_get_probe_dev_list(cl_lan_dev_list** list);
    
CLIB_API void cl_free_probe_dev_list(cl_lan_dev_list* list);

CLIB_API RS cl_reset_probe_dev_list(void);

/*
	���ܣ�
		һ�������豸��֤
	�������:
		@dev: �豸��Ϣ
		@pwd: �豸����
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ:	LDPE_DEV_AUTH_OK LDPE_DEV_AUTH_FAIL	
*/
CLIB_API RS cl_dev_auth(cl_lan_dev_info *dev, const char *pwd);

/*
	���ܣ�
		һ�������豸����wifi����
	�������:
		@dev: �豸��Ϣ
		@ssid: wifi ssid
		@pwd: wifi����
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ:	LDPE_WIFI_CFG_OK LDPE_WIFI_CFG_FAIL	
*/
CLIB_API RS cl_wifi_config(cl_lan_dev_info *dev, const char *ssid, const char *pwd);

/*
	���ܣ�
		��ȡ�ֻ��ȵ�һ�������豸��Ŀ��sn
	�������:
		��
	�������:
		��
	���أ�
	   �ֻ���Ϊ�ȵ�ʱ��Ŀ��sn
*/
CLIB_API u_int64_t cl_get_ap_dest_sn();
    
#ifdef __cplusplus
}
#endif 


#endif



