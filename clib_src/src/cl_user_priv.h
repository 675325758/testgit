#ifndef	__CL_USER_PRIV_H__
#define	__CL_USER_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_user.h"

#pragma pack(push,1)

/* �豸�汾��Ϣ */
typedef struct ms_version_s{
	u_int8_t major;	/* ���汾 */
	u_int8_t minor;	/* �ΰ汾 */
	u_int8_t revise;	/* �޶��汾 */
	u_int8_t pad;		/* ����ֽ� */
} ms_version_t;

/* �豸�Ĺ̼��汾�������汾��Ϣ */
typedef struct ms_dev_version_s{
	ms_version_t soft_version;		/* �̼��汾��Ϣ */
	ms_version_t upgrade_version;	/* �������汾��Ϣ */
} ms_dev_version_t;

typedef struct {
	u_int16_t ssid_len;
	u_int16_t psswd_len;
	char ssid[32];
	char psswd[64];
} misc_ssid_pwd_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t pad[3];
	misc_ssid_pwd_item_t item[0];
} misc_ssid_pwd_t;

#define	MNIF_CONNECT	BIT(0)
#define	MNIF_WAN	BIT(1)
#define	MNIF_IP_INVALID	BIT(2)

typedef struct {
	char name[16];
	// MNIF_XXX
	u_int8_t flags;
	u_int8_t pad;
	u_int16_t mtu;
	u_int32_t ip;
	u_int64_t rx_bytes;
	u_int64_t tx_bytes;
} misc_ni_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t item_len;
	u_int8_t pad[2];
	misc_ni_item_t item[1];
} misc_ni_t;

typedef struct {
	u_int32_t ip;
	u_int8_t mac[6];
	u_int8_t is_from_wifi;
	u_int8_t name_len;
	char name[64];
} misc_client_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t item_len;
	u_int8_t pad[2];
	misc_client_item_t items[0];
} misc_client_t;

// ���ڴ���RF�豸����
typedef struct {
    struct stlc_list_head link;
    u_int8_t frame_index;
    u_int8_t buf[1024];
}rf_dev_defrag_info;

typedef struct {
	// �豸�����೤ʱ����
	u_int32_t uptime;
	// ʲôʱ���ѯ�����ģ���λ����
	u_int32_t query_uptime;
	
	// �豸����������Ӷ೤ʱ����
	u_int32_t online;
	// ʲôʱ���ѯ�����ģ���λ����
	u_int32_t query_online;
	
	// �豸�ϻ������೤ʱ����
	u_int32_t conn_internet;
	// ʲôʱ���ѯ�����ģ���λ����
	u_int32_t query_conn_internet;
	
	// ����汾��
	ms_dev_version_t version;
	//rf�豸�µ�Ƭ���汾��
	ms_version_t rf_stm_ver;
	// �°汾��Ϣ ��can_updateΪ��0ʱ��Ч
	ms_version_t new_version;
	// ��0��ʾ�豸���°汾��������
	char can_update;
	// ��0��ʾ�����Զ�������0��ʾ��Ҫ�ֹ������̼�
	char can_auto_update;
	char pad[2];
	// �豸�°汾������Ϣ
	char *release_desc;
	// �豸�°汾url
	char *release_url;
	// �豸�°汾��������
	char *release_date;

	// CPUʹ���ʣ�С������λ
	u_int16_t cpu;
	// �ڴ�ʹ���ʣ�С������λ
	u_int16_t mem;
	char ap_ssid[32 + 4];
	char ap_passwd[64 + 4];

	// ������
	misc_ni_item_t wan;
	misc_ni_item_t lan;

	// ������
	misc_client_t *clients;
    
    cl_rfdev_status_t rf_stat;
	rfdev_status_t *rfdev;
    
} dev_info_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 


#endif

