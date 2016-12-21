#ifndef		__UDP_CTRL_H__
#define	__UDP_CTRL_H__

#define	DBG_UDP_CTRL_PKT	1
#define	UCS_TEST	1

//#include <stdbool.h>
//#include <arpa/inet.h>
#include "ds_proto.h"
#include "stlc_list.h"
#include "cl_priv.h"

#define	EMPTY_FUNC \
	printf("\n\n\nEEEEEEEE please write function %s \n\n\n", __FUNCTION__)

/*****************************************
	UDP����Э�鱨��ͷ(CMD_UDP_XXX)
 *****************************************/

// ���غ�
#define	MAX_UC_PKT_SIZE	1500
//��֤ʱ�ĳ�ʱʱ��
#define DIE_TIME_FOR_AUTH   15

#define BOFP(buf, ofs) (&((char *)(buf))[ofs])
#define V2_SHARE_KEY_LEN 8

#pragma pack(push,1)

/*
	udp ctrl packet header
*/
typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	encrypt:1,
            request:1,
            hlen:3,
            ver:3;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t ver:3,
        hlen:3,
        request:1,
        encrypt:1;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t flags;
	u_int8_t request_id;
	u_int8_t resv;
	// session id
	u_int32_t client_sid;
	u_int32_t device_sid;

	// ���濪ʼ���ܼ���
	u_int16_t command;
	u_int16_t param_len;
} ucph_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    u_int8_t 	encrypt:1,
request:1,
hlen:3,
ver:3;
#elif __BYTE_ORDER == __BIG_ENDIAN
    u_int8_t ver:3,
hlen:3,
request:1,
encrypt:1;
#else
# error "Please fix <bits/endian.h>"
#endif
    u_int8_t flags;
    u_int8_t request_id;
    u_int8_t resv;
    // session id
    u_int32_t client_sid;
    u_int32_t device_sid;
    
    // ���濪ʼ���ܼ���
    u_int16_t command;
    u_int16_t param_len;
    u_int32_t u32_req_id;
    u_int32_t pkt_hash;
} ucph_v2_t;

#define	ucph_hdr_len(ucph)	(((ucph_t *)ucph)->hlen<<2)
#define ucph_hdr_size (sizeof(ucph_t))
#define ucph_v2_hdr_size (sizeof(ucph_v2_t))

#define	get_ucp_payload(pkt, type) (type *)BOFP((pkt)->data, ucph_hdr_len((pkt)->data))
#define	get_net_ucp_payload(hdr, type) (type *)BOFP(hdr, ucph_hdr_len(hdr))

// ͷ���������ĵĳ���
#define	ucph_hdr_plain_size	12


typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	encrypt:1,
            request:1,
            hlen:3,
            ver:3;
	u_int8_t flags:6,
        is_frag_resp:1,
		is_frag:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t ver:3,
        hlen:3,
        request:1,
        encrypt:1;
	u_int8_t is_frag:1,
        is_frag_resp:1,
			flags:6;
#else
# error "Please fix <bits/endian.h>"
#endif
	
	u_int8_t request_id;
	u_int8_t resv;
	// session id
	u_int32_t client_sid;

	// ���濪ʼ���ܼ���
	u_int16_t command;
	u_int16_t frag_ident; //��Ƭ��־
	u_int16_t frag_total;//���ƶ���Ƭ
	u_int16_t frag_offset; //Ƭƫ��
    u_int32_t param_len;
	u_int32_t reserved;
} uascph_t;

#define	uascph_hdr_len(uascph)	(((uascph_t *)uascph)->hlen<<2)
#define uascph_hdr_size (sizeof(uascph_t))

#define	get_uascp_payload(pkt, type) (type *)BOFP((pkt)->data, uascph_hdr_len((pkt)->data))
#define	get_net_uascp_payload(hdr, type) (type *)BOFP(hdr, uascph_hdr_len(hdr))

#define	uascph_hdr_plain_size	uascph_hdr_size

// ��������������flag����
#define UDP_LA_FLAG_IS_FRAG BIT(0)
#define UDP_LA_FLAG_IS_LAST BIT(1)
#define UDP_LA_FLAG_IS_NEED	BIT(2)

// udp control action
#define	UCA_NONE		0
#define	UCA_REQUEST	1
#define	UCA_REPLY		2
#define	UCA_QUESTION	3
#define	UCA_ANSWER		4
#define	UCA_RESULT		5
#define	UCA_REDIRECT	6
#define	UCA_RESET		7
#define	UCA_GET		8
#define	UCA_SET		9
#define	UCA_DELETE		10
#define UCA_PUSH        11


//typedef struct _pkt_t {
//	struct stlc_list_head link;
//	// �ܳ���
//	int total;
//	// ���ܺ󲻺�֪����������һ�����ڵ���
//	int cmd;
//	u_int8_t action;
//	// net_header_t *
//	u_int8_t data[0];
//} pkt_t;

/*
	�ỰID�ķ���
*/
#define	UC_SID_INVALID	0
// 0 - 0x10000000 �������豸�����������������ֱ�����ֻ�
#define	UC_SID_DEV_RESV_BEGIN	0x00000001
#define	UC_SID_DEV_RESV_END	0x0FFFFFFF

// ������������������豸
#define	UC_SID_SERV_DEV_BEGIN_1	0x80000000
#define	UC_SID_SERV_DEV_END_1	0x8FFFFFFF

#define	UC_SID_SERV_DEV_BEGIN_2	0x90000000
#define	UC_SID_SERV_DEV_END_2	0x9FFFFFFF


// ��������������������������ӵ��ֻ�
#define	UC_SID_SERV_CLIENT_BEGIN	0xA0000000
#define	UC_SID_SERV_CLIENT_END	0xAFFFFFFF


// δ��֤�ɹ���������״̬������ʱ��
#define	DFL_TIME_UCS_HALF_DIE	5
// ɾ�����ӵ���ʱ
#define	DFL_TIME_UCS_DEL_DELAY	3

// �ͻ�����IDLE״̬�ȴ�������󣬽�����һ�γ���
#define	DFL_UC_IDLE_TIME	3

#define ALL_SUB_ATTR 0xFFFF


#define	UCDS_CHECK_PLEN(s, hdr, min_len) \
	do { \
		if (hdr->param_len < min_len) { \
			printf("%s recv bad pkt: cmd=%u, param_len=%u, min_len=%lu\n", s->name, hdr->command, hdr->param_len, min_len); \
			return RS_ERROR; \
		} \
	} while (0)

#define	UCDS_CHECK_PLEN_VOID(s, hdr, min_len) \
	do { \
		if (hdr->param_len < min_len) { \
			printf("%s recv bad pkt: cmd=%u, param_len=%u, min_len=%lu\n", s->name, hdr->command, hdr->param_len, min_len); \
			return; \
		} \
	} while (0)
	
/*****************************************
	״̬
 *****************************************/

// udp ctrl server status
enum {
	UCSS_QUESTION	= 0,
	UCSS_ESTABLISH	 = 1,
	UCSS_DIE = 2,
	UCSS_MAX = 3
};

typedef void (* ucs_func_t)();

typedef struct {
	char *name;
	// ���뱾״̬���õĺ���
	ucs_func_t on_into;
	// ������
	ucs_func_t proc_pkt;
} ucs_proc_t;


// udp ctrl client status
enum {
	UCCS_IDLE = 0,
	UCCS_AUTH_REQ = 1,
	UCCS_AUTH_ANSWER = 2,
	UCCS_ESTABLISH = 3,
	UCCS_ERROR = 4,
	UCCS_MAX = 5
};

typedef void (* ucc_func_t)();

typedef struct {
	char *name;
	// ���뱾״̬���õĺ���
	ucc_func_t on_into;
	// �뿪��״̬���õĺ���
	ucc_func_t on_out;
	// ������
	ucc_func_t proc_pkt;
} ucc_proc_t;


/*
	�������ݵķ��������ؼ��ܺ�ĳ���
*/
typedef int (*ucs_enc_func_t)(void *session, u_int8_t *data, int len);
/*
	�������ݵķ��������ؽ��ܺ�ĳ���
*/
typedef int (*ucs_dec_func_t)(void *session, u_int8_t *data, int len);


/*****************************************
	��֤���ĸ�ʽ (CMD_UDP_AUTH)
 *****************************************/


#define MAX_UUID_BIN_LEN 16

// type define for TLV
#define	UCT_VENDOR			1
#define	UCT_LOCAL_TIME		2
#define	UCT_TIME_PARAM		3
//͸���豸����,��������
#define UCT_SCM_DEV_TYPE		6
//͸���豸��������
#define UCT_SCM_FACTORY_DATA   11
#define UCT_808_PAN_TYPE		12
#define	UCT_DEV_NAME			13
//һ�����óɹ�ʱ��,��������ɨ�豨��ʹ��
#define UCT_SM_CONFIG_TIME      14
//RF�豸��Ƭ���汾
#define UCT_RF_STM_VER			15
#define UCT_UCDS_TIMEZONE_PUT		16//�ظ�ʱ�����ͻ���

#define UCT_RF_LAMP_REMOTE_ID 17 //����ң��id
#define UCT_PHONE_INDEX 18 //�������ͥ�û�ʱ���ֻ���index
#define UCT_PHONE_PSK 19 //�����PSK
#define  UCT_IA_IS_AP_MODE     20

#define UCT_DEVICE_EVM_IS_UPGRADING 	21	// ������Ƿ���������

#define UCT_DEVICE_NEW_DEV_EXT_TYPE 22	// ֱ�������豸֤�����ͺ���չ���ͽӿ�

#define UCT_DEV_NEW_LA_SUPPORT 23	// �豸֧��������־�����͵�homeid����

#define UCT_DEV_OEM_DN	24	// �豸���ڵ�OEM����

#define UCT_LED_COLOR_VER	25	//
//������̽�����
#define UCT_SERVER_PORBE_SN			26 //������̽���ϴ�sn
#define UCT_SERVER_PORBE_APP_ID		27 //������̽���ϴ�appid
#define UCT_SERVER_PORBE_VVID		28 //������̽���ϴ����ͻ��˵�OEM id. 0-galaxywind, 10-qh vvid
#define UCT_SERVER_PORBE_VENDOR		29 //������̽���ϴ����ͻ���vendor
#define UCT_SERVER_PROBE_DNS		30 //����������̽��õ������dns
#define UCT_SERVER_PROBE_ERR_DONAME 31 //������̽�ⲻ���õ������ϴ�


#define UCT_DEVICE_DETAIL	32

#define UCT_SLAVE_SUPPORT 33	// ���豸����
#define UCT_DEVELOPER_ID 34	// ������ID

#define UCT_RFDEV_BIND_TIME	35	// RF�豸��ʱ��

#define UCT_RFDEV_NEW_VER	36 //����ʾ���豸���°汾�ƣ�����Ҫ��ʱ��ѯ״̬


#define	tlv_next(tlv) (uc_tlv_t *)BOFP((uc_tlv_t *)tlv + 1, tlv->len)
#define	tlv_n_next(tlv) (uc_tlv_t *)BOFP((uc_tlv_t *)tlv + 1, ntohs(tlv->len))
#define	tlv_val(tlv) ((u_int8_t *)((uc_tlv_t *)tlv + 1))

#define tlv_u32_value(tlv) (*((u_int32_t *)((uc_tlv_t *)tlv + 1)))
#define tlv_u16_value(tlv) (*((u_int16_t *)((uc_tlv_t *)tlv + 1)))
#define tlv_u8_value(tlv) (*((u_int8_t *)((uc_tlv_t *)tlv + 1)))

#define tlv_ntoh_u32_value(tlv) (ntohl(*((u_int32_t *)((uc_tlv_t *)tlv + 1))))
#define tlv_ntoh_u16_value(tlv) (ntohs(*((u_int16_t *)((uc_tlv_t *)tlv + 1))))
#define tlv_u8_value(tlv) (*((u_int8_t *)((uc_tlv_t *)tlv + 1)))

typedef struct {
	// UCT_XX
	u_int16_t type;
	// ���ݳ��ȣ�������ͷ��
	u_int16_t len;
} uc_tlv_t;
/******************debug info********************************/
#define DBG_TLV_HD_LEN		sizeof(uc_tlv_t)
//SVN��
#define DBG_TYPE_SVN		1
//����
#define DBG_TYPE_CUR		2
//������ӦADֵ
#define DBG_TYPE_CUR_AD		3
//����kֵ
#define DBG_TYPE_CUR_K		4
//����bֵ
#define DBG_TYPE_CUR_B		5
//��ѹ
#define DBG_TYPE_VOL		6
//��ѹ��ӦADֵ
#define DBG_TYPE_VOL_AD		7
//��ѹkֵ
#define DBG_TYPE_VOl_K		8
//��ѹbֵ
#define DBG_TYPE_VOL_B		9
//����ADֵ 
#define DBG_TYPE_LIGHT_AD	10
//���ӷ���������
#define DBG_TYPE_SERVER_DONAME	11
//���ӷ�����ʱ��
#define DBG_TYPE_SERVER_CONNTIME 12
//��ǰ�������ֻ���������
#define DBG_TYPE_CLIENTS		13
//�豸uptime
#define DBG_TYPE_UPTIME			14
//���ѧϰ
#define DBG_TYPE_LIGHT_STUDY	15
// CPU ������
#define DBG_TYPE_CPU_USAGE 16
//�ڴ�������
#define DBG_TYPE_MEM_USAGE 17
//����ƽ������
#define DBG_TYPE_AVG_AD				18
//����������
#define DBG_TYPE_MAX_AD 		19
//��������ͻ��ʱ��
#define DBG_TYPE_DELAY_POWER_ON_TIME	20
//�ػ�����ͻ��ʱ��
#define DBG_TYPE_DELAY_POWER_OFF_TIME	21
//���ص�����ADֵ
#define DBG_TYPE_NO_LOAD_AD		22
//��Ƭ������汾
#define DBG_TYPE_SMT_SOFT_VERSION 23
//��Ƭ��Ӳ���汾
#define DBG_TYPE_SMT_HARD_VERSION 24
//��������ID ��
#define DBG_TYPE_IR_LIB_ID			25
//���俪������ͻ��ʱ��
#define DBG_TYPE_COLD_DELAY_PN_TIME	26
//����ػ�����ͻ��ʱ��
#define DBG_TYPE_COLD_DELAY_PF_TIME	27
//���ȿ�������ͻ��ʱ��
#define DBG_TYPE_HOT_DELAY_PN_TIME	28
//���ȹػ�����ͻ��ʱ��
#define DBG_TYPE_HOT_DELAY_PF_TIME	29
//��Ƭ��������Ϣ
#define DBG_TYPE_STM32_INFO	30
//�豸��reset������Ƭ���ȴ�У��
#define DBG_TYPE_ADJUST_PRESSED 31
//�豸������ip
#define DBG_TYPE_DEV_SERVER_IP 32
//��ҵ��ղ���״̬��־
#define DBG_TYPE_PT_STATUS_FLAG 33
//����WiFi(·����)��ʱ��u32 ��
#define DBG_TYPE_TIME_CONN_WIFI		34
//��ǰ����WiFi���ź�ǿ��int8
#define DBG_TYPE_WIFI_RSSI   		35
//��ǰ����WiFi��ģʽ(phy_mode) u8(0 1 2)
#define DBG_TYPE_WIFI_PHY_MODE		36
// ���ߵİ汾��
#define DBG_TYPE_WIFI_VERSION		37

//����WAN��IP
#define DBG_WIRED_WAN_IP					40
//����WAN��IP
#define DBG_WIRELESS_WAN_IP					41
//Kernel����汾��
#define DBG_KERNEL_IMAGE_VERSION			42
//Kernel����SVN��
#define DBG_KERNEL_IMAGE_SVN				43
//User����汾��
#define DBG_USER_IMAGE_VERSION				44
//User����SVN��
#define DBG_USER_IMAGE_SVN					45

// MAC��ַ6�ֽ�
#define DBG_TYPE_MAC_ADDR					46

// IWULINK �汾��
#define DBG_TYPE_IWULINK_VERSION			47

/**************************************************/

typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
	u_int8_t second;
	u_int8_t timezone;
} uc_time_t;

/*
	�ش�ʱ�䡢����ʱ�䡢����ʱ��
*/
typedef struct {
	// �������ش�ʱ�����������κ�����ش�ʱ�䶼�������һ���ˡ���λ100ms
	u_int8_t retry_100ms[3];
	// ���ͱ��������ʱ��������λ��
	u_int8_t keeplive;
	// ������ʱ��������λ��
	u_int16_t die;
} uc_time_param_item_t;

typedef struct {
	// �豸���������
	uc_time_param_item_t dev;
	// �ֻ����豸������ֱ��
	uc_time_param_item_t lan;
	// �ֻ����豸���������ӣ�APP��ǰ������
	uc_time_param_item_t wan_front;
	// �ֻ����豸���������ӣ�APP�ں������
	uc_time_param_item_t wan_background;
} uc_time_param_all_t;


typedef struct {
	// UAA_XXX
	u_int8_t action;
	u_int8_t auth_flag;
    u_int8_t version;
	u_int8_t reserved[1];
	// �����
	u_int8_t rand1[4];
	u_int64_t sn;
	// �ͻ���Ψһ��ʶ
	u_int8_t my_uuid[MAX_UUID_BIN_LEN];
	
	// ��ǰ����ʱ�ӣ����ڿͻ��˸��豸ͬ��ʱ�䡣���ɿ���Դ
	uc_time_t time;
} uc_auth_request_t;

typedef struct {
	u_int64_t sc;
	u_int8_t phone_uuid[MAX_UUID_BIN_LEN];
    u_int8_t phone_desc[16];
}uc_share_register_t;

typedef struct {
    u_int64_t share_code;
    u_int8_t action;
    u_int8_t pad;
    u_int16_t result;
}uc_share_reg_res_t;

typedef struct {
	// UAA_XXX
	u_int8_t action;
	u_int8_t reserved[3];

	u_int32_t ip;
	u_int16_t port;
	u_int16_t pad;
} uc_auth_redirect_t;

typedef struct {
	// UAA_XXX
	u_int8_t action;
    u_int8_t version;
	u_int8_t reserved[2];
	// �����
	u_int8_t rand2[4];
	u_int8_t your_uuid[MAX_UUID_BIN_LEN];
} uc_auth_question_t;

typedef struct {
    // UAA_XXX
    u_int8_t action;
    u_int8_t version;
    u_int8_t remain_days;
    u_int8_t reserved;
    // �����
    u_int8_t rand2[4];
    u_int8_t your_uuid[MAX_UUID_BIN_LEN];
    u_int8_t share_key[V2_SHARE_KEY_LEN];
} uc_auth_question_v2_t;

#define	UC_ENC_NONE	0x0000
#define	UC_ENC_AES128	0x0001
#define	UC_ENC_XXTEA	0x0002

typedef struct {
	// UAA_XXX
	u_int8_t action;
	u_int8_t flags;
	// UC_ENC_xxx
	u_int16_t support_enc;

	u_int8_t answer[16];
	// ��ǰ����ʱ�ӣ����ڿͻ��˸��豸ͬ��ʱ�䡣�ɿ���Դ
	uc_time_t time;
} uc_auth_answer_t;

typedef struct {
	// UAA_XXX
	u_int8_t action;
	u_int8_t version;
	u_int16_t select_enc;

	u_int16_t err_num;
	u_int16_t reserved2;

	// Ӧ��Ϊ�豸��ʱ��Ч��Ϊ������ʱ����
	u_int8_t dev_type;
	u_int8_t ext_type;
	u_int8_t login_type; /* LT_NORMAL, LT_BIND, LT_UNBIND */
	u_int8_t net_type; /* NT_SERVER, NT_DEVICE, NT_DEVICE_OFFLINE */

	// UCT_VENDOR, UCT_LOCAL_TIME, UCT_TIME_PARAM
	uc_tlv_t tlv[0];
} uc_auth_result_t;

/******************************************
������һЩ���ݽṹ
****************************************/
#define APP_USER_UUID_NAME_LEN	16
#define APP_USER_RSA_LEN	128
#define APP_USER_DONAME_LEN		64

#define UCAU_NONE	0
#define UCAU_HELLO	1
#define UCAU_CREATE	2
#define UCAU_AUTH	3
#define UCAU_QUESTION	4
#define UCAU_ANSWER	5
#define UCAU_RESULT	6
#define UCAU_DELETE 7
#define UCAU_ADDDEV 8
#define UCAU_REMOVEDEV 9
#define UCAU_QUERY 10
#define UCAU_EVENT 11
#define UCAU_TEMPLATE 12
#define UCAU_RULELIST 13
#define UCAU_RULEADD 14
#define UCAU_RULEREMOVE 15
#define UCAU_REGISTER 16
#define UCAU_EDIT 17
#define UCAU_RESET 18
#define UCAU_REQUEST 19
#define UCAU_REPLY 20
#define UCAU_EXEC 21
#define UCAU_MOVE 22
#define UCAU_MODIFY 23
#define UCAU_HOME_NAME_MODIFY 24
#define UCAU_RULELIST_PUSH 25
#define UCAU_SERVER_SET 26
#define UCAU_SERVER_QUERY 27
#define UCAU_USER_REPLACE 28
#define UCAU_SERVER_NOTIFY 29
#define UCAU_TABLE_QUERY 30
#define UCAU_TABLE_PUSH 31
#define UCAU_MISC_QUERY 32
#define UCAU_CAPFILE	33
#define UCAU_MISC_SET 	34
#define UCAU_USER_REQ_SC 35
#define UCAU_USER_ADD_HOME 36
#define UCAU_LABEL_BIND 37
#define UCAU_HOME_SHORTCUT 38
#define UCAU_HOME_SHORTCUTDEL 39
#define UCAU_HOME_SHORTQUERY 40
#define UCAU_HOME_SHORTCUTMODIFY 41
#define UCAU_HOME_LAST_RULE_TIME 42
#define UCAU_HOME_RULE_EXCUTE 43
#define UCAU_CUSTOM_CAPFILE	44

//����CMD_QUERY_HISTORY = 308,/*��־��ѯ����*/��action
#define UCAU_HISTORY_RULE_QUERY 1
#define UCAU_HISTORY_MEMBER_QUERY 2


//tlv
//����resulttlv
#define UCLA_RESULT_TLV_USERID				(1)
#define UCLA_RESULT_TLV_LASTTIME 			(2)
#define UCLA_RESULT_LLV_TIME_PARAM			(3)
#define UCLA_RESULT_TLV_SUPPORT_TABEL 		(4)
//�Ƿ�֧����ת����������
#define UCLA_RESULT_TLV_SUPPORT_TRANS		(5)
//�Ƿ�֧�ְ�������
#define UCLA_RESULT_TLV_SUPPORT_SHORTCUT 	(6)
//�������Ƿ�֧��ѹ��
#define UCLA_SERVER_TLV_SUPPORT_COMPRESS	(7)
// WIDGET����Կ
#define UCLA_SERVER_TLV_WIDGET_KEY			(8)

//��������keeplive tlv
//����
#define UCLA_KEEPLIVE_TLV_KICK		(1)
//�˺Ų�����
#define UCLA_KEEPLIVE_TLV_ACOUNT_NOT_EXIST	(2)

//����answer tlv
#define UCLA_ANSWER_TLV_VER (1)
//��ʾapp�Ƿ�֧��ѹ��
#define UCLA_APP_TLV_SUPPORT_COMPRESS (2)
// ������ID
#define UCLA_DEVELOPER_ID (3)

//miscset tlv
#define UCLA_MISCSET_TLV_2_UNLINK	(1)
#define UCLA_MISCSET_TLV_2_LINK		(2)
#define UCLA_MISCSET_TLV_HOMEMOVE	(3)

//��֤�׶νṹ
typedef struct {
	u_int16_t apptype;
	u_int8_t pad[2];
	u_int32_t  time_xor;	//ʱ����DISP_APPSRV_XOR����ֵ
	uc_tlv_t tlv[0];
}ucla_req_disp_hdr_t;

typedef struct {
	u_int32_t errorno;
	u_int32_t ip;
	u_int16_t port;
	u_int8_t global_id;
	u_int8_t pad;
	uc_tlv_t tlv[0];
}ucla_disp_res_hdr_t;

typedef struct {
	u_int8_t action;
	u_int8_t flag;
	u_int8_t reserved[2];
	u_int8_t randon[4];
	u_int8_t rsa[APP_USER_RSA_LEN];
}uc_app_user_req_t;

typedef struct {
	u_int8_t action;
	u_int8_t flag;
	u_int8_t reserved[2];
	u_int8_t rsa[APP_USER_RSA_LEN];
}uc_app_user_reg_ret_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int8_t username[APP_USER_UUID_NAME_LEN];
	u_int8_t uuid[APP_USER_UUID_NAME_LEN];
	u_int8_t passwd_md5[APP_USER_UUID_NAME_LEN];
}uc_app_user_create_req_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t userid;
}uc_app_user_create_ret_t;

typedef struct {
	u_int8_t action;
	u_int8_t version;
	u_int8_t flag;
	u_int8_t reserved;
	u_int8_t random1[4];
	u_int8_t username[APP_USER_UUID_NAME_LEN];
	u_int8_t uuid[APP_USER_UUID_NAME_LEN];
}uc_app_user_auth_t;

typedef struct {
	u_int8_t action;
	u_int8_t resvered[3];
	u_int8_t random2[4];
}uc_app_user_answer_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t support_enc;
	u_int8_t md5[APP_USER_UUID_NAME_LEN];//(rand1 + rand2 + ����md5)
}uc_app_user_question_t;

typedef struct {
	u_int8_t action;
	u_int8_t ver;
	u_int16_t select_enc;
	u_int16_t err;
	u_int16_t reserved;
}uc_app_user_result_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];
	u_int8_t uuid[APP_USER_UUID_NAME_LEN];
	u_int8_t passwd[APP_USER_UUID_NAME_LEN];
}uc_app_user_phone_create_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t user_id;
}uc_app_user_phone_create_r_t;

typedef struct {
	u_int8_t doname[APP_USER_DONAME_LEN];
	u_int32_t user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];
	u_int8_t uuid[APP_USER_UUID_NAME_LEN];
	u_int8_t passwd[APP_USER_UUID_NAME_LEN];
	u_int32_t flag;
	u_int32_t localid;
}uc_phone_conf_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int16_t reserved;
	u_int32_t user_id;
	uc_phone_conf_t conf[0];
}uc_app_user_phone_conf_set_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t user_id;
}uc_app_user_phone_conf_set_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int32_t user_id;
}uc_app_user_phone_conf_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int16_t result;
	u_int32_t user_id;
	uc_phone_conf_t conf[0];
}uc_app_user_phone_conf_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int32_t user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];
}uc_app_user_phone_replace_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t user_id;
	u_int32_t client_id;
}uc_app_user_phone_replace_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int32_t user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];
	u_int8_t uuid[APP_USER_UUID_NAME_LEN];
	u_int8_t passwd[APP_USER_UUID_NAME_LEN];
}uc_app_user_phone_modify_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t user_id;
}uc_app_user_phone_modify_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t user_id;
}uc_app_user_home_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t pad[2];
	u_int32_t user_id;
	u_int32_t home_id[0];
}uc_app_user_home_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
}uc_app_user_misc_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t reserve[2];
	u_int32_t value[0];
}uc_app_user_misc_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
}uc_app_user_misc_set_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
}uc_app_user_misc_set_r_t;

typedef struct {
	u_int32_t userid_src;
	u_int32_t userid_dst;
	u_int8_t num;
	u_int8_t pad[3];
	u_int32_t homeid[0];
}uc_app_user_move_home_t;

typedef struct {
	u_int64_t sn;
	u_int8_t passwd[16];
}uc_app_user_la_change_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
}uc_app_user_req_sc_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int8_t share[8];
}uc_app_user_req_sc_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int32_t user_id;
	u_int32_t home_id;
	u_int8_t share[8];
	char desc[0];
}uc_app_user_add_home_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
}uc_app_user_add_home_r_t;

//Ȧ�Ӳ����׶νṹ
typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int8_t home_name[64];
}uc_home_conf_create_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t home_id;
}uc_home_conf_create_r_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t reserved[3];
	u_int32_t home_id;
}uc_home_conf_delete_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t reserved;
	u_int16_t result;
}uc_home_conf_delete_r_t;


typedef struct {
	u_int64_t sn;
	u_int8_t flag;
	u_int8_t pad[3];
	u_int8_t dev_passwd[APP_USER_UUID_NAME_LEN];
}uc_la_dev_info_t;

typedef struct {
	u_int64_t sn;
	u_int8_t dev_passwd[APP_USER_UUID_NAME_LEN];
}uc_la_dev_query_info_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int32_t home_id;
	uc_la_dev_info_t dev[0];
}uc_home_conf_adddev_t;

typedef struct {
	u_int8_t action;
	u_int8_t okcount;
	u_int8_t reserved[2];
	u_int32_t home_id;
	u_int64_t sn[0];
}uc_home_conf_adddev_r_t;

typedef struct {
	u_int64_t sn;
	u_int8_t dev_passwd[APP_USER_UUID_NAME_LEN];
}uc_la_dev_modify_info_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int32_t home_id;
	uc_la_dev_modify_info_t dev[0];
}uc_home_conf_modify_passwd_dev_t;

typedef struct {
	u_int8_t action;
	u_int8_t okcount;
	u_int8_t reserved[2];
	u_int32_t home_id;
	u_int64_t sn[0];
}uc_home_conf_modify_passwd_dev_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t flag;	// bit 0: ��0��ʾ��Ҫ�ָ����� ;��1��ʾ����Ҫ�ָ�����
	u_int8_t reserved;
	u_int32_t home_id;
	u_int64_t sn[0];
}uc_home_conf_removedev_t;

typedef struct {
	u_int8_t action;
	u_int8_t okcount;
	u_int8_t reserved[2];
	u_int32_t home_id;
	u_int64_t sn[0];
}uc_home_conf_removedev_r_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t reserved[3];
	u_int32_t home_id;
}uc_home_conf_query_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int32_t last_rule_time;//�ϴι������ʱ��
	u_int32_t last_template_time;//�ϴ�ģ���޸�ʱ��
	u_int32_t home_id;
	u_int8_t passwd[APP_USER_UUID_NAME_LEN];
	u_int8_t home_name[64];
	uc_la_dev_query_info_t dev[0];
}uc_home_conf_query_r_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t event_type;
	u_int8_t reserved[2];
	u_int32_t home_id;
	//u_int8_t passwd[APP_USER_UUID_NAME_LEN];//APP_USER_UUID_NAME_LEN
	//u_int8_t name[64];
}uc_home_conf_event_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t reserved;
	u_int16_t result;
}uc_home_conf_event_r_t;

typedef struct {
	u_int64_t sn;
	u_int8_t passwd[APP_USER_UUID_NAME_LEN];
}uc_home_conf_movedev_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int32_t src_home_id;
	u_int32_t dst_home_id;
	uc_home_conf_movedev_t dev_sn[0];
}uc_home_conf_move_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int32_t src_home_id;
	u_int32_t dst_home_id;
	u_int64_t dev_sn[0];
}uc_home_conf_move_r_t;


typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	u_int32_t home_id;
	u_int8_t name[64];
}uc_home_conf_namemodify_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved;
	u_int16_t result;
	u_int32_t home_id;
}uc_home_conf_namemodify_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
	u_int32_t home_id;
}uc_home_last_rule_time_q_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t last_rule_modify;
}uc_home_last_rule_time_q_r_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t lang;
	u_int8_t reserved[2];
	u_int32_t home_id;
	u_int32_t app_id;
}uc_link_conf_template_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int32_t home_id;
	u_int8_t url[0];//\0�������ַ���
}uc_link_conf_template_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t lang;
	u_int8_t reserved[2];
}uc_link_conf_cap_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t reserved[2];
	u_int8_t url[0];
}uc_link_conf_cap_r_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t reserved[3];
	u_int32_t home_id;
	u_int32_t rule_id;
}uc_link_conf_rulelist_t;

typedef struct {
	u_int8_t state;
	u_int8_t reserved[3];
	u_int8_t rule[0];
}uc_rulelist_t;

typedef struct {
	u_int32_t rule_id;
	u_int32_t last_exec_time;
	u_int8_t state;
	u_int8_t enable;
	u_int8_t reserved[2];
	//u_int8_t rule[0];//\0�����ַ���
}uc_rule_net_t;

typedef struct {
	u_int8_t aciton;
	u_int8_t count;
	u_int8_t total_seq;
	u_int8_t reserved;
	u_int32_t home_id;
	u_int32_t total_length;
	uc_rule_net_t rulelist[0];
}uc_link_conf_rulelist_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t total_seq;
	u_int8_t seq;
	u_int32_t home_id;
	u_int32_t total_length;
	u_int8_t data[0];
}uc_link_conf_req_push_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t total_seq;
	u_int8_t seq;
	u_int32_t cur_length;
	u_int32_t total_length;
	u_int8_t data[0];
}uc_dev_ver_limit_req_push_t;

typedef struct {
	u_int8_t action;
	u_int8_t enable;
	u_int8_t pad[2];
	u_int32_t home_id;
	u_int32_t rule_id;
	char rule[0];
} uc_linkage_config_rule_add_t;

typedef struct {
	u_int8_t action;
	u_int8_t enable;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_add_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_del_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_del_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_exec_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_exec_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
	u_int32_t rule_len;
	u_int32_t home_id;
	char rule[0];
}uc_linkage_config_rule_excute2_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
}uc_linkage_config_rule_excute2_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t enable;
	u_int8_t pad[2];
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_modify_t;

typedef struct {
	u_int8_t action;
	u_int8_t enable;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_rule_modify_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t event;
	u_int8_t value;
	u_int8_t pad;
	u_int32_t home_id;
	u_int32_t rule_id;
} uc_linkage_config_event_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
} uc_linkage_home_share_create_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
	u_int8_t share[8];
} uc_linkage_home_share_create_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
	u_int8_t share_code[8];
	u_int8_t desc[0];
} uc_linkage_home_share_register_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad;
	u_int16_t result;
	u_int32_t home_id;
} uc_linkage_home_share_register_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
} uc_linkage_home_share_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t event_type;
	u_int8_t pad[2];
	u_int32_t home_id;
} uc_linkage_home_share_event_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
} uc_linkage_home_share_event_r_t;


typedef struct {
	u_int32_t user_id;
	u_int8_t role_id;
	u_int8_t pad[3];
	u_int32_t join_time;
	u_int32_t lastuse_time;
	u_int8_t desc[0];
} share_query_item_t;

typedef struct {
	u_int8_t action;
	u_int8_t count;
	u_int8_t pad[2];
	u_int32_t home_id;
	//share_query_item_t items[0];
} uc_linkage_home_share_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
	u_int32_t user_id;
	u_int8_t role_id;
	u_int8_t pad1[3];
	u_int8_t desc[0];
} uc_linkage_home_share_edit_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t user_id;
} uc_linkage_home_share_edit_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int32_t home_id;
	u_int32_t user_id;
} uc_linkage_home_share_del_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t user_id;
} uc_linkage_home_share_del_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t label_id;
	u_int32_t home_id;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];
	u_int64_t a_sn[0];
}uc_label_add_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int16_t label_id;
	u_int16_t pad;
	u_int32_t home_id;
}uc_label_add_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t label_id;
	u_int32_t home_id;
}uc_label_delete_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int16_t label_id;
	u_int16_t pad;
	u_int32_t home_id;
}uc_label_delete_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t label_id;
	u_int32_t home_id;
}uc_label_query_t;

typedef struct {
	u_int16_t label_id;
	u_int16_t sn_count;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];
	u_int64_t a_sn[0];
}uc_label_info_t;

typedef struct {
	u_int8_t action;
	u_int8_t reseve;
	u_int16_t result;
	u_int16_t label_id;
	u_int16_t pad;
	u_int32_t home_id;
	//uc_label_info_t a_label_info[0];
}uc_label_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t label_num;
	u_int32_t home_id;
	u_int64_t sn;
	u_int16_t a_label_id[0];
}uc_label_bind_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
}uc_label_bind_r_t;

//��ݰ���
typedef struct {
	u_int8_t action;
	u_int8_t enable;
	u_int8_t index;
	u_int8_t reserve;
	u_int32_t home_id;
	u_int32_t rule_id;
	u_int32_t rule_len;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];
	char rule[0];
}uc_home_shortcut_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t rule_id;
}uc_home_shortcut_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t index;
	u_int8_t reserve[2];
	u_int32_t home_id;
}uc_home_shortcut_del_t;

typedef struct {
	u_int8_t action;
	u_int8_t index;
	u_int16_t result;
	u_int32_t home_id;
}uc_home_shortcut_del_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t query_index;
	u_int8_t reserve[2];
	u_int32_t home_id;
}uc_home_shortcut_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t index;
	u_int8_t enable;
	u_int8_t reserve;
	u_int32_t home_id;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];
}uc_home_shortcut_modify_t;

typedef struct {
	u_int8_t action;
	u_int8_t index;
	u_int16_t result;
	u_int32_t home_id;
}uc_home_shortcut_modify_r_t;

typedef struct {
	u_int8_t index;
	u_int8_t pad[3];
	u_int32_t rule_id;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];
}sc_info_t;

typedef struct {
	u_int8_t action;
	u_int8_t query_index;
	u_int16_t result;
	u_int32_t home_id;
	sc_info_t sc_info[0];
}uc_home_shortcut_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t index;
	u_int8_t reserve[2];
	u_int32_t home_id;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];
}uc_home_sc_modify_t;

typedef struct {
	u_int8_t action;
	u_int8_t index;
	u_int16_t result;
	u_int32_t home_id;	
}uc_home_sc_modify_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
	u_int32_t home_id;
}uc_rule_lmt_q_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t rule_last_modify_time;
}uc_rule_lmt_q_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t key_len;
	u_int32_t home_id;
	u_int8_t key[0];
} uc_dict_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int16_t key_len;
	u_int16_t value_len;
	u_int8_t key_value[0];
} uc_dict_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t flag;
	u_int16_t reserve;
	u_int32_t home_id;
	u_int16_t key_len;
	u_int16_t value_len;
	u_int8_t key_value[0];
} uc_dict_set_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int8_t key[0];
} uc_dict_set_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
	u_int32_t home_id;
	u_int16_t key_len;
	u_int8_t pad[2];
	u_int8_t key[0];
} uc_dict_del_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
} uc_dict_del_r_t;


typedef struct {
	u_int16_t key_len;
	u_int8_t key[0];
} uc_dict_event_t;

//������notify�ṹ
typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int16_t type;
	u_int16_t cn_msg_len;
	u_int16_t en_msg_len;
	u_int8_t value[0];
}uc_notify_info_t;

/*****************************************
	����ĸ�ʽ (CMD_UDP_KEEPLIVE)
 *****************************************/


typedef struct {
	u_int8_t action;
	u_int8_t flags;
	u_int16_t reserved;
	uc_tlv_t tlv[0];
} uc_keeplive_request_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
	uc_tlv_t tlv[0];
} uc_keeplive_reply_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserved[3];
} uc_keeplive_reset_t;


/*****************************************
	�豸֤���ʽ
 *****************************************/
 #define UCKEY_MAC_LEN 6
 #define UCKEY_SN_LEN 6
 #define UCKEY_VENDOR_LEN 16
 #define UCKEY_DFPWD_LEN 8
 #define UCKEY_PRESHAREKEY_LEN 8
 
 typedef struct {
	u_int8_t ver;
	u_int8_t len;
	u_int8_t dev_type;
	u_int8_t ext_type;
	u_int8_t mac[UCKEY_MAC_LEN];
	u_int8_t sn[UCKEY_SN_LEN];
	u_int8_t vendor[UCKEY_VENDOR_LEN];
	u_int8_t df_pwd[UCKEY_DFPWD_LEN];
	u_int8_t preshare_key[UCKEY_PRESHAREKEY_LEN];
} uc_key_t;

/* uc device structure */
typedef struct{
	u_int64_t sn;
	u_int8_t dev_type;
	u_int8_t ext_type;
	u_int8_t vendor[UCKEY_VENDOR_LEN];
	u_int8_t passwd_md5[16];
	u_int8_t preshare_key[UCKEY_PRESHAREKEY_LEN];
	u_int8_t uuid[MAX_UUID_BIN_LEN];
	u_int32_t ucs_globalid;/*udp server global id*/
}uc_dev_info_t;


/*****************************************
	���Ʊ��ĸ�ʽ (CMD_UDP_CTRL)
 *****************************************/


/*****************************************
	UDP Control Object Type 
	UDP Control Sub Object Type
	UDP Control Attribute Type
 ****************************************/
 
/*
	system. (ϵͳ)
*/
#define	UCOT_SYSTEM	1

// system.support.
#define	UCSOT_SYS_SUPPORT	1
// system.support.support
#define	UCAT_SUPPO_RT_SUPPORT	1

// system.support.
#define	UCSOT_SYS_HOSTNAME	2
// system.hostname.hostname
#define	UCAT_HOSTNAME_HOSTNAME	1

// system.software.
#define	UCSOT_SYS_SOFTWARE	3
// system.software.version
#define	UCAT_SYS_VERSION	100 /* ���ܱ�����Ӷ����õģ��Ӵ�һ��Ķ��� */
// system.software.upgrade
#define	UCAT_SYS_UPGRADE	101 /*����ӳ��*/
//system.softwate.stm_upgrade
#define UCAT_SYS_STM_UPGRADE 102 /*stm��Ƭ������*/

//system.softwate.evm_upgrade
#define UCAT_SYS_EVM_UPGRADE 103 /*evm��������*/
//system.softwate.evm_erase
#define UCAT_SYS_EVM_ERASE 104 /* ��������������*/
//system.softwate.evm_info
#define UCAT_SYS_EVM_INFO 105 /* ������������ѯ*/

//system.software.stm2_upgrade
#define UCAT_SYS_STM2_UPGRADE 106 /*rf�豸�µĵ�Ƭ������*/
//system.software.stm_up_preinfo
#define UCAT_SYS_STM_UPGRADE_PREINFO 107 /*ָ����������*/

// system.software.reboot
#define UCAT_SYS_REBOOT 1
// system.software.uptime
#define UCAT_SYS_UPTIME 2
// system.software.ip
#define UCAT_SYS_IP 3
//system.software.svn		     
#define UCAT_SYS_SVN		4
//system.software.ssid
#define UCAT_SYS_SSID		5
//system.software.passwd			
#define UCAT_SYS_PASSWD		6
//system.software.devstatus
#define UCAT_SYS_DEVSTATUS	7
//system.software.compile_date
#define UCAT_SYS_COM_DATE	8
//system.software.compile_time
#define UCAT_SYS_COM_TIME	9
//system.software.systime
#define UCAT_SYS_SYSTIME	10
//system.software.setsystime
#define UCAT_SYS_SET_SYSTIME	11
//system.software.debuginfo
#define UCAT_SYS_DEBUGINFO	12
//�ָ�����
#define UCAT_SYS_RESTORY_FACTORY 13
// WIFI����״̬
#define UCAT_SYS_WIFI_STATE 14
//s3�����е����⣬���ݴ�����
#define UCAT_SYS_WIFI_STATE_S3 15
//wifi�豸ʱ����ȡ��������
#define UCAT_SYS_TIMEZONE	16

// system.upgrade.
#define	UCSOT_SYS_UPGRADE	4

// system.hardware.
#define	UCSOT_SYS_HARDWARE	5
// system.hardware.LED
#define	UCATT_HARDWARE_LED	1
//����LED ����ɫ
#define UCATT_HARDWARE_LED_COLOR 2
// ��Ƭ���汾
#define UCAT_HARDWARE_STM_VERSION 3
// ���õ�Ƭ����������
#define UCAT_HARDWARE_STM_UPGRADE 4
// �豸Ӳ����Ϣ��ȡ
#define UCAT_HARDWARE_DISK_INFO_GET 5
//������Ϣ��ȡ
#define UCAT_HARDWARE_ETH_INFO_GET 6

// system.vendor.
#define	UCSOT_SYS_VENDOR	6
// system.vendor.oem_id
#define	UCAT_VENDOR_OEM_ID	1

// system.server.
#define	UCSOT_SYS_SERVER	7
// system.server.connect_time
#define	UCAT_SERVER_CONNECT_TIME	1
//system.server.domainname
#define UCAT_SERVER_DONAME	2
//system.server.ip
#define UCAT_SERVER_IP		3
//system.server.monitor
#define UCAT_SERVER_MONITOR 4
//system.server.homeid
#define UCAT_SERVER_HOMEID 5
//system.server.lanserverip
#define UCAT_SERVER_LANSERVERIP 6

//system.user
#define UCSOT_SYS_USER 8 /*�û���һЩ����*/
//system.user.password
#define UCAT_SYS_PASSWORD 1
//system.user.location
#define UCAT_SYS_LOCATION 2

//�豸������ر���
// system.license
#define	UCSOT_SYS_LICENSE	9
// system.license.activve
#define	UCAT_SYS_LICENSE_ACTIVE 1


//����
#define UCSOT_SYS_DEV_SHARE 10
//�����ֻ���ȡһ��������
#define UCAT_SYS_CODE 0x1
//�ֻ������б�
#define UCAT_PHONE_LIST 0x2


// �������û����� 
//system.lanusers_manage
#define	UCSOT_LANUSERS_MANAGE	11
#define UCAT_LANUSERS_MANAGE_ENABLE	1	// ʹ�ܿ���
#define UCAT_LANUSERS_MANAGE_USER_INFO	2	// �û���Ϣ
#define UCAT_LANUSERS_MANAGE_STATUS	3	// �û��ؼ������Ϣ
#define UCAT_LANUSERS_MANAGE_RECORD_NUM	4	// ��ʷ��¼����
#define UCAT_LANUSERS_MANAGE_USER_RECORD 5	// ��һؼ���ʷ��¼



/*
	IA(���ܵ���)
*/
// IA.
#define	UCOT_IA	2

// IA.stat.
#define	UCSOT_IA_STAT	1
// IA.stat.total_peak
#define	UCAT_STAT_TOTAL_PEAK	1
// IA.stat.total_valley
#define	UCAT_STAT_TOTAL_VALLEY	2
// IA.stat.period_time
#define	UCAT_STAT_PERIOD_TIME	3
// IA.stat.period_peak
#define	UCAT_STAT_PERIOD_PEAK	4
// IA.stat.period_valley
#define	UCAT_STAT_PERIOD_VALLEY	5
// IA.stat.month_peak
#define	UCAT_STAT_MONTH_PEAK	6
// IA.stat.month_valley
#define	UCAT_STAT_MONTH_VALLEY	7
// IA.stat.cur_power
#define	UCAT_STAT_CUR_POWER	8
// IA.stat.peak_time
#define	UCAT_STAT_PEAK_TIME	9
// IA.stat.peak_price
#define	UCAT_STAT_PEAK_PRICE	10
// IA.stat.valley_price
#define	UCAT_STAT_VALLEY_PRICE	11
//IA.stat.month_flat
#define UCAT_STAT_MONTH_FLAT  12
//IA.stat.flat_time
#define UCAT_STAT_FLAT_TIME   13
//IA.stat.flat_price
#define UCAT_STAT_FLAT_PRICE  14
//IA.stat.flat_time
#define UCAT_STAT_VALLEY_TIME   15
//IA.stat.total_ele
#define UCAT_STAT_TOTAL_ELE		16
//IA.stat.phase_ele
#define UCAT_STAT_PHASE_ELE		17
//IA.stat.on_ele
#define UCAT_STAT_ON_ELE		18
//һ���е�ͳ�����ݣ���Ҫ������ѯ,ֻ��ͨ�õ�2.17.x��֧��
#define UCAT_STAT_DAYS_STAT  21

#define UCAT_STAT_MILI_POWER  22


// IA.AIRCONDITION.
#define	UCSOT_IA_AC	2
// IA.AIRCONDITION.WORK
#define	UCAT_AC_WORK	1
// IA.AIRCONDITION.CUR_TEMP
#define	UCAT_AC_CUR_TEMP	2
// IA.AIRCONDITION.TIMER
#define	UCAT_AC_TIMER	3
//IA.AIRCONDITION.smart_on
#define UCAT_STAT_SMART_ON    4
//IA.AIRCONDITION.smart_off
#define UCAT_STAT_SMART_OFF   5
//IA.AIRCONDITION.smart_sleep
#define UCAT_STAT_SMART_SLEEP 6
//�Ƿ���ƥ����룬���µĻ����ϵĿյ�
#define UCAT_AC_ATTRI  10
//��ǰƥ��״̬
#define UCAT_AC_MATCH_STAT 11

//IA. AIRCONDITION.TMP_CTRL�¶ȱ仯���ơ�
#define UCAT_IA_TMP_CTRL		12
//IA. AIRCONDITION.ON_USER �û����ÿ��ػ�״̬(���Ƿ����⣬ֻ���豸�յ�״̬)
 #define UCAT_IA_ON_USER_SET		13
//ʪ������
#define UCAT_IA_CUR_HUMIDITY 14
//IA.AIRCONDITION.TMP_SAMPLE_CURVE
#define UCAT_AC_TMP_SAMPLE_CURVE    15   //�����¶ȱ仯���߻�ȡ
//IA.AIRCONDITION.RH_SAMPLE_CURVE
#define UCAT_AC_RH_SAMPLE_CURVE    16     //����ʪ�ȱ仯���߻�ȡ

#define UCAT_AC_CHILD_LOCK    17     //ͯ�����

//С��������
#define UCAT_AC_POWERCHECK	18

//
#define UCAT_AC_UTC_TMP_CTRL    20

#define UCAT_AC_MSG_CONFIG 21	// ��������

#define UCAT_AC_MSG_CONFIG_V2 22	// ��������

//IA.CODE
#define UCSOT_IA_CODE    3
//IA.CODE.PROCESS
#define UCAT_CODE_PROCESS		1
#define UCAT_CODE_MATCH		2
#define UCAT_CODE_LIST		3
#define UCAT_CODE_INFO		4
#define UCAT_CODE_DATA		5
#define UCAT_CODE_DATA2		6
#define UCAT_CONTROL_BOARD	7
#define UCAT_CONTROL_KEY	8
#define UCAT_CONTROL_LEARN	9
#define UCAT_CONTROL_KEY_CODE 10
// STB AND TV CTRL
#define UCAT_CODE_RC_INFO   16
#define UCAT_CODE_RC_KEY    17
#define UCAT_CODE_RC_LEARN  18
#define UCAT_CODE_RC_CTRL   19
#define UCAT_CODE_RC_MATCH  20
// Ӧ�����豸��ƥ��״̬
#define UCAT_CODE_RC_MATCH_STATE  21
// �������֧��ѧϰ�Ϳ��չ���
#define UCAT_CODE_LEARN_SNAPSHOT  22

//������͡
#define UCSOT_IA_HEATER_LCYT  4

#define UCAT_HEATER_LCYT_WORK 1
#define UCAT_HEATER_LCYT_MODE 2
#define UCAT_HEATER_LCYT_GEAR 3
#define UCAT_HEATER_LCYT_CUR_TEMP 4
#define UCAT_HEATER_LCYT_SET_TEMP 5
#define UCAT_HEATER_LCYT_SET_TIME 6
#define UCAT_HEATER_LCYT_REMAIN_TIME 7
/*
*��������¯���
*/
//IA.FAST_HOT_FURNACE
#define UCOT_IA_FHF					5
//IA.FAST_HOT_FURNCE.WORK
#define UCAT_FHF_WORK				1
//IA.FAST_HOT_FURNCE.SET_TIMER
#define UCAT_FHF_SET_TIMER			2
//IA.FAST_HOT_FURNCE.GET_TIME
#define UCAT_FHF_GET_TIMER			3
//IA.FAST_HOT_FURNCE.POWER
#define UCAT_FHF_GET_POWER			4
//IA.FAST_HOT_FURNCE.SET_TEMP	
#define UCAT_FHF_SET_TEMP			5
//IA.FAST_HOT_FURNCE.GET_TEMP
#define UCAT_FHF_GET_TEMP			6
//IA.FAST_HOT_FURNCE.GET_SN
#define UCAT_FHF_GET_SN				7

/*
���ƿ���������
*/
//IA.AIR_CLEANER
#define UCOT_IA_HKAC					6
//IA.HK_AIR_CLEANER.WORK
#define UCAT_HKAC_WORK				1
//IA.HK_AIR_CLEANER.MODE
#define UCAT_HKAC_MODE				2
//IA.HK_AIR_CLEANER.WIND
#define UCAT_HKAC_WIND				3
//IA.HK_AIR_CLEANER.TEMP
#define UCAT_HKAC_TEMP				4
//IA.HK_AIR_CLEANER.PM25
#define UCAT_HKAC_PM25				5
//IA.HK_AIR_CLEANER.HUMIDITY
#define UCAT_HKAC_HUMIDITY				6
//IA.HK_AIR_CLEANER.ANION_UVL
#define UCAT_HKAC_ANION_UVL			7
//IA.HK_AIR_CLEANER.TIMER
#define UCAT_HKAC_TIMER				8

// ��Ѷ�����
#define UCOT_IA_GX_LED		7
#define UCAT_GX_LED_STATUS	 1

/*
 * E��
 */
//IA.EB
#define UCSOT_IA_EB		8
//IA.EB.WORK
#define UCAT_EB_WORK		1
//IA.EB.TIMER
#define UCAT_EB_TIMER		2

/*
�ϰؿ���������
*/
//IA.NB_AIR_CLEANER
#define UCOT_IA_NBAC				9
//IA.NB_AIR_CLEANER.WORK
#define UCAT_NBAC_WORK			1
//IA.NB_AIR_CLEANER.MODE
#define UCAT_NBAC_MODE			2
//IA.NB_AIR_CLEANER.WIND
#define UCAT_NBAC_WIND			3
//IA.NB_AIR_CLEANER.TEMP
#define UCAT_NBAC_TEMP				4
//IA.NB_AIR_CLEANER.PM25
#define UCAT_NBAC_PM25				5
//IA.NB_AIR_CLEANER.HUMIDITY
#define UCAT_NBAC_HUMIDITY		6
//IA.NB_AIR_CLEANER.ANION
#define UCAT_NBAC_ANION			7
//IA.NB_AIR_CLEANER.TERILIZE
#define UCAT_NBAC_TERILIZE			8
//IA.NB_AIR_CLEANER.TIMER
#define UCAT_NBAC_TIMER			9
//IA.NB_AIR_CLEANER.PERIODIC_TIMER
#define UCAT_NBAC_PERIODIC_TIMER	10
//IA.NB_AIR_CLEANER.ROSEBOX_LIFE
#define UCAT_NBAC_PERIODIC_ROSEBOX_LIFE	11
/*
* �ʺ����̺
*/
#define UCOT_IA_CH_BLANKET 10
//״̬
#define UCAT_CH_STATUS 1
#define UCAT_CH_QUERY_LEFT 2
#define UCAT_CH_QUERY_RIGHT 3

//IA.TT ���ܵ���֮͸������: Transparent transmission
#define UCSOT_IA_TT  11
//IA.TT. ALLSTATE ��ѯ��push���е�״̬�������Է����ֻ�ͳһ��������WIFIģ�鶨ʱ��ѯά����ֵ
#define UCAT_IA_TT_ALLSTATE 1
//IA.TT.command ctrl �������ѯ����
#define UCAT_IA_TT_CMD 2
//IA.TT.command result ���һ��APP��������Ľ��
#define UCAT_IA_TT_CMDRET 3
//IA.TT.uart_cmd �豸͸���Ŀͻ��˴�������
#define UCAT_IA_TT_UARTCMD 4
//IA.TT.ir_notify �豸�ϱ��ĺ������
#define UCAT_IA_TT_IR_NOTIFY 5
//IA.TT.reset �豸����ͻ�������reset����
#define UCAT_IA_TT_RESET 6
//IA.TT.ir_notify �豸�ϱ��������ɼ�����
#define UCAT_IA_TT_IR_SOUND_NOTIFY 8
//��ָ��û�͸������
#define UCAT_IA_TT_ON_CMD 9

#define UCAT_IA_TT_OFF_CMD 10

//͸������TLV ����
#define TLV_TYPE_SCM_COMMAND 0x1
//͸������TLV ����2,ǰ��̶�����
#define TLV_TYPE_SCM_OTHER  0x2

#define UCSOT_IA_PUBLIC 12
//ͨ�ö�ʱ��
#define UCAT_IA_PUBLIC_PERIOD_TIMER 1
//ͨ���쳣�ϱ�����
#define UCAT_IA_PUBLIC_EXCEPTION 2
//�����豸ssid������
#define UCAT_IA_PUBLIC_WIFI_SETTING 3
//��չ��ͨ�ö�ʱ��
#define UCAT_IA_PUBLIC_EXT_PERIOD_TIMER 4
//IA.PUBLIC.CURVE �¶�����
#define UCAT_IA_TMP_CURVE		5
//UTC �¶�����
#define UCAT_IA_TMP_UTC_CURVE   6
//ͨ�ö�ʱ��
//ժҪ��ѯ
#define UCAT_IA_COM_TIMER_PROC	7
// ��ݿ��ػ�
#define UCAT_IA_PUBLIC_SHORTCUTS_ONOFF	8 
// ���ܺ���
#define UCAT_IA_PUBLIC_UTC_TMP_CTRL 9
// APP������ر����ܿ�������
#define UCAT_IA_PUBLIC_SMART_ON 10
// ͨ��ͯ��
#define UCAT_IA_PUBLIC_CHILD_LOCK 11
//�Ƿ�֧�ֿ�ݶ�ʱ��,��Ҫ�Ǵ�����������⡣
#define UCAT_IA_PUBLIC_SHORTCUT_TIMER 12
// �¶ȷ�ֵ��������
#define UCAT_IA_PUBLIC_TEMP_ALARM 13
// ͨ����ʷ��¼ժҪ
#define UCAT_IA_PUBLIC_HISTORY_INFO 14
// ͨ����ʷ��¼����
#define UCAT_IA_PUBLIC_HISTORY_ITEM 15
// ͨ�õĿ����¶�
#define UCAT_IA_PUBLIC_BOOT_TEMP	16
//ͨ���쳣�ϱ�����V2
#define UCAT_IA_PUBLIC_EXCEPTION_V2 17
// ͨ�õ��������ýӿ�
#define UCAT_IA_PUBLIC_WAN_CONFIG 18
// ͨ�õ�DHCP����������
#define UCAT_IA_PUBLIC_DHCP_SERVER 19
// ͨ�õ�AP�ȵ�����
#define UCAT_IA_PUBLIC_AP_CONFIG 20
// ͨ�õ��м�������
#define UCAT_IA_PUBLIC_REPEATER 21
// ͨ�õĺ�̨֪ͨ��֪ͨ�豸���APP���ں�̨ģʽ
#define UCAT_IA_PUBLIC_BACKGROUND 22
// ͨ�õĿ����豸��̨����telnetd��������
#define UCAT_IA_PUBLIC_TELNET	23

//�������ˮ��
#define UCSOT_IA_TBHEATER 13

#define UCAT_TBHEATER_USERCONFIG     1
/* IA.TBHEATER.WORKCONFIG */
#define UCAT_TBHEATER_WORKCONFIG     2
/* IA.TBHEATER.HEATER_TEMP */
#define UCAT_TBHEATER_HEATER_TEMP    3
/* IA.TBHEATER.STATE */
#define UCAT_TBHEATER_STATE          4
/* IA.TBHEATER.SYSINFO */
#define UCAT_TBHEATER_SYSINFO        5
/* IA.TBHEATER.UPGRADE_SLAVE */
#define UCAT_TBHEATER_UPGRADE_SLAVE 6
/* IA.TBHEATER.BINDSN */
#define UCAT_TBHEATER_BINDSN         7

//�����ذ�
//״̬���ݻ�ȡ����
#define UCAT_IA_TB_STATUS				21
//��������
//�¶�����
#define UCAT_IA_TB_TEMP					22
//ģʽ����
#define UCAT_IA_TB_MODE					23
//��������
#define UCAT_IA_TB_ONOFF				24
//��˪����
#define UCAT_IA_TB_DEFROST				25
//���޵��籣������������
#define UCAT_IA_TB_MISC					26
//һЩ��������,�������
#define UCAT_IA_TB_PROTECT				27
//���̵���������¶�����
#define UCAT_IA_TB_BERT					28
//����״̬�ϴ�
#define UCAT_IA_TB_UPGRADE_INFO				29
//�������ͷ�����
#define UCAT_IA_TB_EEV					30
//Ӳ���汾��
#define UCAT_IA_TB_VER					31
//�����¶�����
#define UCAT_IA_TB_ALLTMP				32



// �������������˽������
#define UCSOT_IA_JCX 16

#define UCAT_IA_JCX_VOL 1
#define UCAT_IA_JCX_CURRENT 2
#define UCAT_IA_JCX_ACTIVE_POWER 3
#define UCAT_IA_JCX_REACTIVE_POWER 4
#define UCAT_IA_JCX_POWER_FACTOR 5
#define UCAT_IA_JCX_FREQ 6
#define UCAT_IA_JCX_ACTIVE_DEGREE 7 
#define UCAT_IA_JCX_REACTIVE_DEGREE 8
#define UCAT_IA_JCX_CHANNEL_NAME 9
#define UCAT_IA_JCX_CHANNEL_INFO 10
#define UCAT_IA_JCX_SN 11
#define UCAT_IA_JCX_SOFT_VER 12
#define UCAT_IA_JCX_HARWARE_VER 13

//ͨ�õ���ͳ��
#define	UCSOT_IA_COMMON_STAT	17
// IA.stat.total_peak

//�����¿�������ռ��
//IA.YL �����Ӷ���
#define UCSOT_IA_THERMOSTAT_YL			18
//�����¿�������
#define UCAT_IA_THERMOSTAT_YL_WORK		1
//�����¿���ģʽ
#define UCAT_IA_THERMOSTAT_YL_MODE		2
//�����¿����¶�����
#define UCAT_IA_THERMOSTAT_YL_SET_TMP	3
//�����¿�������
#define UCAT_IA_THERMOSTAT_YL_ROOM_TMP	4

/*
*LEDE led
*/
//IA.LEDE_LED
#define	UCSOT_IA_LEDE_LED				19
//IA.LEDE_LED.STATE
#define UCAT_LEDE_LED_STATE				1
//IA.LEDE_LED.TIMER
#define UCAT_LEDE_LED_TIMER				2

#define UCAT_LEDE_LED_UPOWERON_STATUE	3

//���ܱ�
#define	UCSOT_IA_JNB_THERMOSTAT			20
//IA.LEDE_LED.STATE
#define UCAT_JNB_STAT				1

//2.4G RF ����
//IA.RFGW
#define UCSOT_IA_RFGW				21
//IA.RFGW.JOIN
#define UCAT_IA_RFGW_JOIN			1
//IA.RFGW.JOIN_FIND
#define UCAT_IA_RFGW_JOIN_FIND	2
//IA.RFGW.JOIN_ACTION
#define UCAT_IA_RFGW_JOIN_ACTION	3
//IA.RFGW.GROUP
#define UCAT_IA_RFGW_GROUP		4
//IA.RFGW.LIST
#define UCAT_IA_RFGW_LIST			5
//IA.RFGW.TT
#define UCAT_IA_RFGW_TT			6
//IA.RFGW.DEV_SYNC
#define UCAT_IA_RFGW_DEV_SYNC	7
#define UCAT_IA_RFGW_DEV_GROUP		8
#define UCAT_IA_RFGW_DEV_GROUP_MEMBER		9
#define UCAT_IA_RFGW_DEV_NAME		10
#define UCAT_IA_RFGW_DEV_GROUP_TT		11
#define UCAT_IA_RFGW_PARAM				12
#define UCAT_IA_RFGW_TT_CACHE			13
#define UCAT_IA_RFGW_LIST_DIGEST		14
#define UCAT_IA_RFGW_LIST_DIGEST_ACK	15
#define UCAT_IA_RFGW_LIST_WITH_ID		16
#define UCAT_IA_RFGW_HW_PUSH    		17
#define UCAT_IA_RFGW_DBC				18
#define UCAT_IA_RFGW_YLSGBJ				19
//#define UCAT_IA_RFGW_REPEATER			20
#define UCAT_IA_RFGW_IMG				21
#define UCAT_IA_RFGW_ONEKEY				22

//���ش���������
#define UCAT_IA_RFGW_BIGDATA_CMD		120


//��Ѹ�Ʊڻ�
#define UCSOT_IA_BREAK_MACHINE                 22
//���ú�Ѹ�Ʊڻ��Ĺ���ģʽ
#define UCAT_IA_BREAK_MACHINE_SET_CMD          1
//��ȡ�豸�ѹ���ʱ��
#define UCAT_IA_BREAK_MACHINE_ON_TIME          2
//��ȡ�¶�
#define UCAT_IA_BREAK_MACHINE_TEMP             3
//��ȡת��
#define UCAT_IA_BREAK_MACHINE_SPEED            4
//�������
#define UCAT_IA_BREAK_MACHINE_FINISH           5
//�����쳣��ֹ
#define UCAT_IA_BREAK_MACHINE_STOP             6
//��ȡ����ģʽ
#define  UCAT_IA_BREAK_MACHINE_DIYNAME          7
//��ͣ
#define UCAT_IA_BREAK_MACHINE_PAUSE				8
//����״̬
#define UCAT_IA_BREAK_MACHINE_IDLE				9
//�����¶�
#define UCAT_IA_BREAK_MACHINE_KEEP_WARM			10


//�����¿���
#define UCSOT_IA_TL_TEMP	23

#define UCAT_IA_TELIN_BASE  (1)
/* IA.TELIN.ADVANCE */
#define UCAT_IA_TELIN_ADV   (2)
/* IA.TELIN.STAT */
#define UCAT_IA_TELIN_STAT  (3)
/* IA.TELIN.TIMER */
#define UCAT_IA_TELIN_TIMER (4)
/**/
#define UCAT_IA_TELIN_TIME_SYNC (5)

//ǧ�����ܲ���
//IA.TEA_TRAY_QP ǧ�����ܲ����Ӷ���
#define UCSOT_IA_TEA_TRAY_QP			24
//ǧ�����ܲ��̿���
#define UCAT_IA_TEA_TRAY_QP_WORK		1
//ǧ�����ܲ����ֶ���ˮ
#define UCAT_IA_TEA_TRAY_QP_WATER		2
//ǧ�����ܲ����ֶ�����
#define UCAT_IA_TEA_TRAY_QP_CTRL	3
//ǧ�����ܲ��̷���
#define UCAT_IA_TEA_TRAY_QP_PLAN	4
//ǧ�����ܲ��̷���ID��ѯ
#define UCAT_IA_TEA_TRAY_QP_PLAN_ID	5
//ǧ�����ܲ��̷���ִ��
#define UCAT_IA_TEA_TRAY_QP_PLAN_EXECUTE	6
//�ָ�����״̬
#define UCAT_IA_TEA_TRAY_QP_RESET_FAULT     7


//ǧ����
#define UCSOT_IA_QP_POT                 			25

//����ǧ�������������
#define UCAT_IA_QP_POT_SET_PROC         1
//�龰����
#define UCAT_IA_QP_POT_SCENE_CTRL       2
//��״̬
#define UCAT_IA_QP_POT_CURR_STAT        3
//�龰�б�
#define UCAT_IA_QP_POT_SCENE_ID_LIST    4
//�龰�仯PUSH
#define UCAT_IA_QP_POT_SCENE_MODIDY_PUSH    5

/*���֡�ɳ�ز���*/
#define UCSOT_IA_EPLUG_OEM      27
//����״̬
#define UCAT_IA_EP_OEM_ONOFF    0x1
//����
#define UCAT_IA_EP_OEM_ROOM_TEMP   0x2
//����
#define UCAT_IA_EP_OEM_TEMP_RANGE    0x3
//����¶ȷ�Χ
#define UCAT_IA_EP_OEM_MAX_TEMP    0x4
//���߱���
#define UCAT_IA_EP_OEM_OFFLINE_PROTECT    0x5
//����̽��
#define UCAT_IA_EP_OEM_PERSION_DETECT   0x6


//��Դ�¿���
//IA_THERMOSTAT.XY
#define UCSOT_IA_THERMOSTAT_XY			28
//͸����������
#define UCAT_IA_THERMOSTAT_CTRL			1
//push����
#define UCAT_IA_THERMOSTAT_PUSH			2
//����ģʽʱ�������
#define UCAT_IA_THERMOSTAT_TIME			3
//�ⲿ�¶�����
#define UCAT_IA_THERMOSTAT_EX_TMP		4
//���ܻؼҿ���
#define UCAT_IA_THERMOSTAT_SMART_HOME	5



/*�������*/
//IA.CARWUKONG ��������Ӷ���
#define UCSOT_IA_CAR_WUKONG				29
//����
#define UCAT_IA_CAR_WUKONG_ON			1
//Ѱ������
#define UCAT_IA_CAR_WUKONG_SERCH		2
//�����¶Ȳ�ѯ
#define UCAT_IA_CAR_WUKONG_TEMP			3
//����ƿ��ȫ��ѹ��⿪��
#define UCAT_IA_CAR_WUKONG_VALCHECK		4
//�豸���ܿ���
#define UCAT_IA_CAR_WUKONG_POWERSAVE	5
//����push����
#define UCAT_IA_CAR_WUKONG_ALARM		6
//�����ٷֱ�
#define UCAT_IA_CAR_WUKONG_ELE_PERCENTAGE 7
//debug
#define UCAT_IA_CAR_WUKONG_DEBUG_INFO	8

//ǧ���Ʊڻ�
#define UCSOT_IA_QP_PBJ     30
//״̬
#define UCAT_IA_QP_PBJ_WORK_STAT 0x1
//�龰ִ��
#define UCAT_IA_QP_PBJ_EXEC_SCENE   0x2
//�Զ����龰
#define UCAT_IA_QP_PBJ_EDIT_SCENE   0x3
//�龰ID�б�
#define UCAT_IA_QP_PBJ_SCENE_LIST   0x4
//ȥ��fault_stat
#define UCAT_IA_QP_PBJ_CLEAR_FAULT  0x5


//��Ѹ������
#define UCSOT_IA_HX_YS_POT     31
//״̬
#define UCAT_IA_HX_YS_POT_WORK_STAT 0x1
//�龰ִ��
#define UCAT_IA_HX_YS_POT_EXEC_SCENE   0x2
//�Զ����龰
#define UCAT_IA_HX_YS_POT_EDIT_SCENE   0x3
//�龰ID�б�
#define UCAT_IA_HX_YS_POT_SCENE_LIST   0x4

//���ÿյ�
#define UCSOT_IA_YUETU     32

//IA.YUETU.WORK
#define UCAT_IA_YUETU_WORK 1
//IA.YUETU.TIMER
#define UCAT_IA_YUETU_TIMER 2
//IA.YUETU.STAT
#define UCAT_IA_YUETU_STAT 3
//IA.YUETU.ERROR_PROTECT
#define UCAT_IA_YUETU_ERROR_PROTECT 4
//IA.YUETU.ELE
#define UCAT_IA_YUETU_ELE 5
//IA.YUETU.SN
#define UCAT_IA_YUETU_SN	6


//�ĵ�����ˮ��
#define UCSOT_IA_ADS	33

//��������
#define UCAT_IA_ADS_CTRL		1
//��������
#define UCAT_IA_ADS_CONF		2

//��ʯ΢��¯
#define UCSOT_IA_JS_MICWAVE 34
//״̬��Ϣ
#define UCAT_IA_JS_STATE 0x1
//���ò���
#define UCAT_IA_JS_SETTING 0x2
//���ò���
#define UCAT_IA_JS_FAST_EXEC 0x3
//ͯ��
#define UCAT_IA_JS_CHILD_LOCK 0x4

//��ϣ���¿���
#define UCSOT_IA_KXM_THER	35
//����
#define UCAT_IA_KXM_T_ONOFF 0x1
//����ģʽ
#define UCAT_IA_KXM_T_MODE 0x2
//�����¶�
#define UCAT_IA_KXM_T_TEMP 0x3
//����
#define UCAT_IA_KXM_T_ENV_TEMP 0x4
//����
#define UCAT_IA_KXM_T_FAN_SPEED 0x5
//����
#define UCAT_IA_KXM_T_ENERGY 0x6

//��ɽ���¯
#define   UCSOT_IA_ELEC_HEATER  36
//WIFI��������
#define UCAT_IA_ELEC_HEATER_WIFI_SET      0x1
//�����������
#define UCAT_IA_ELEC_HEATER_PARAM_PRO   0x2

// ӡ�ȳ�
#define UCSOT_IA_INDIA_CAR	38
	#define UCAT_IA_INDIA_CAR_STAT	1	// ��ȡ/���ͳ���ʵʱ״̬��֧��get/push��india_car_stat_t
	#define UCAT_IA_INDIA_CAR_DEV_STAT 2  // ��ȡ/�����豸ʵʱ״̬��֧��get/push/set india_car_dev_stat_t
	#define UCAT_IA_INDIA_CAR_STORE_STAT 3	// ��ȡ�豸�洢״̬��֧��get
	#define UCAT_IA_INDIA_CAR_WARN		4 // ��ȡ�����õ�ǰ����������֧��get/set/push
	#define UCAT_IA_INDIA_CAR_WIFI_CONFIG 5 // ��ȡ/����WiFi�ȵ�״̬��֧��get/set/push��
	#define UCAT_IA_INDIA_CAR_HISTORY 6 // ��ȡ�豸��ʷ��¼��֧��get/push��
	#define UCAT_IA_INDIA_CAR_DEV_UPGRADE 7 // �ֻ������豸���̽���������֧��set��
	#define UCAT_IA_INDIA_CAR_REALTIME_TRIP	8	// ʵʱ�ó����ݣ�֧��set
	#define UCAT_IA_INDIA_CAR_DEBUG_CONFIG 9	// ������Ϣ����
	#define UCAT_IA_INDIA_CAR_LOCAL_WATCH 10	// ����������Ƶ
	#define UCAT_IA_INDIA_CAR_AGENT_INFO 11	// ��Ƶ�����������Ϣ



//�ǿ���ˮ��
#define UCSOT_IA_ZKRSQ	39
#define UCAT_IA_ZKRSQ_ONOFF	1
#define UCAT_IA_ZKRSQ_BACKWATERONOFF	2
#define UCAT_IA_ZKRSQ_HOTWATER_TEMPERATURE	3
#define UCAT_IA_ZKRSQ_DEV_TEMPERATURE	4
#define UCAT_IA_ZKRSQ_COMPENSATE_TEMPERATURE	5
#define UCAT_IA_ZKRSQ_DEFROST_PERIOD	6
#define UCAT_IA_ZKRSQ_DEFROST_IN_TEMPERATURE	7
#define UCAT_IA_ZKRSQ_DEFROST_PERSISTENCE	8
#define UCAT_IA_ZKRSQ_DEFROST_OUT_TEMPERATURE	9
#define UCAT_IA_ZKRSQ_BACKWATER_TEMPERATURE	10
#define UCAT_IA_ZKRSQ_BACKWATER_MODE	11
#define UCAT_IA_ZKRSQ_TIMER1_STATE	12
#define UCAT_IA_ZKRSQ_TIMER2_STATE	13
#define UCAT_IA_ZKRSQ_TIMER3_STATE	14
#define UCAT_IA_ZKRSQ_FAULT_STATE	15
#define UCAT_IA_ZKRSQ_RECOVER_DEFAULT	16
#define UCAT_IA_ZKRSQ_HOTWATER_TEMPERATURE_CURRENT 17
#define UCAT_IA_ZKRSQ_COILPIPE_TEMPERATURE_CURRENT	18
#define UCAT_IA_ZKRSQ_SURROUNDINGS_TEMPERATURE_CURRENT	19
#define UCAT_IA_ZKRSQ_BACKWATER_TEMPERATURE_CURRENT	20
#define UCAT_IA_ZKRSQ_WORK_MODE	21

// LINKON�¿���
#define  UCSOT_IA_LINKONWKQ  40
	//���ÿ���
	#define  UCAT_IA_LINKONWKQ_POWER  0x01
	//����ͯ��
	#define  UCAT_IA_LINKONWKQ_LOCK  0x02
	//���ù���ģʽ
	#define  UCAT_IA_LINKONWKQ_WORK_MODE  0x03
	//��������ģʽ
	#define  UCAT_IA_LINKONWKQ_RUNNING_MODE  0x04
	//���÷���
	#define  UCAT_IA_LINKONWKQ_WIND  0x05
	//�����¶�
	#define  UCAT_IA_LINKONWKQ_TEMP  0x06
		//����ģʽʱ���
	#define  UCAT_IA_LINKONWKQ_PROG  0x06
	//��ѯ״̬
	#define  UCAT_IA_LINKONWKQ_QUERY  0x08


//�ǻʴ���
#define UCSOT_IA_ZHCL	41
	//����״̬����
	#define UCAT_IA_ZHCL_STATUS_SET		0x01
	//����λ������
	#define UCAT_IA_ZHCL_LOCATION		0x02
	//������
	#define UCAT_IA_ZHCL_BIND			0x03
	//������������
	#define UCAT_IA_ZHCL_TYPE_SET		0x04
	//������������
	#define UCAT_IA_ZHCL_DIR_SET		0x05

//�ǻʵ�����
#define UCSOT_IA_ZHDHX	42
	//�ǻʵ����߿��ؿ���
	#define UCAT_IA_ZHDHX_ONOFF	0x01
	//�ǻʰ�������
	#define UCAT_IA_ZHDHX_NAME	0x02

// �߼����ض����������
#define UCSOT_IA_HPGW 43
	// ���ر������ò�ѯ
	#define UCAT_IA_HPGW_ALARM_CONFIG 1
	// ��Ϣ���ͱ�������
	#define UCAT_IA_HPGW_APPINFO	2
	// �������ѱ�������
	#define UCAT_IA_HPGW_SMS 3
	// ��Ӷ������ѹ������ֻ��û�
	#define UCAT_IA_HPGW_ADDUSER	4
	// ɾ���������ѹ������ֻ��û�
	#define UCAT_IA_HPGW_DELSUER	5
	// ��
	#define UCAT_IA_HPGW_LAMP 6

//��ͥ������������û�ã���ռ���ӣ��Ժ����Ҫ�õ�
#define UCSOT_IA_HOMESERVER	44
	//xxxxxxxx
	#define UCAT_IA_HOMESERVER_TEST 1

/*
	STORAGE(�洢)
*/
// storage.
#define	UCOT_STORAGE	3
// storage.packet.
#define	UCSOT_STORAGE_PACKET	1
// storage.packet.receive
#define	UCAT_PACKET_STRING	1
#define	UCAT_PACKET_BINARY	2
#define	UCAT_PACKET_PACKET	3

/*
*�������
*/
//upgrade
#define UCOT_UPGRADE	4
//upgrade.flash
#define UCSOT_UPGRADE_FLASH 	1
//upgrade.flash.flashinfo
#define UCAT_FLASH_FLASHINFO	1
//upgrade.flash.erase
#define UCAT_FLASH_ERASE		2
//upgrade.flash.upgrade
#define UCAT_FLASH_UPGRADE		3
//upgrade.flash.copy
#define UCAT_FLASH_COPY			4
//upgrade.flash.erasestm
#define UCAT_FLASH_ERASE_STM	5


/*
*�������
*/
//dev_product
#define UCOT_DEVPRO 			5
//devproduct.ir ���������Ӷ���������
#define UCSOT_DEVPRO_IR			1
//devpro.ir.vol_kb ��ѹ getʱad/ad2,setʱk/ad/ad2ֵ
#define UCAT_IR_VOLKADKB		2
//devpro.ir.cur_kb ���� getʱad,setʱb/k
#define UCAT_IR_CURKB			3
//devpro.ir.adjust
#define UCAT_IR_ADJUST			4
//devpro.ir.cur_kb ���� getʱad,setʱb/k
#define UCAT_IR_ADJ			5

/*
	��������豸δ����ͨ�ű���Ԥ����
*/
// IA.
#define	UCOT_EVM	7

// ���������豸��ͳһʹ��IA.EVM ,attri �Զ���
#define UCSOT_EVM_STAT  1

/*
*�������
*/
//plug_product
#define UCOT_PLUG_DEVPRO 			7
//plug_product.av
#define UCSOT_PLUG_DEVPRO_AV        1	
//plug_product.av.adjust
#define UCAT_AV_ADJUST	1

//DBG
#define UCOT_DEBUG                    9

//DBG.LOG ������־sub_obj
#define UCSOT_DBG_LOG                 1
//DBG.LOG.INFO ������־ժҪ��Ϣattr
#define UCAT_DBG_LOG_INFO             1
//DBG.LOG.READ ������־����attr
#define UCAT_DBG_LOG_TEXT             2
//DBG.LOG.ERASE ����������־attr
#define UCAT_DBG_LOG_ERASE            3

//DBG.MEM �ڴ����sub_obj
#define UCSOT_DBG_MEM                 2
//DBG.MEM.USAGE �ڴ�ʹ����attr        
#define UCAT_DBG_MEM_USAGE            1
//DBG.MEM.TESTPKT �ڴ���԰�attr
#define UCAT_DBG_MEM_TESTPKT          2


typedef struct{
	u_int32_t total; //tlv �ܳ�
	u_int32_t used_len; // tlv_�����ܳ�
	u_int8_t* cur;
	u_int8_t * tlv_data;
}tlv_manger_t;



typedef struct {
	u_int16_t objct;
	u_int16_t sub_objct;
	u_int16_t attr;
	u_int16_t param_len;
	// u_int8_t param[0];
} ucp_obj_t;

typedef struct {
	// UCA_XXX
	u_int8_t action;
	u_int8_t count;
	u_int16_t reserved;
	// ucp_obj_t objs[0];
} ucp_ctrl_t;

typedef struct {
	// ���汾��
	u_int8_t major;
	// �ΰ汾��
	u_int8_t minor;
	// �޶��汾��
	u_int8_t revise;
	u_int8_t reserved;
} ucp_version_t;

typedef struct {
	u_int8_t user_enable;
	u_int8_t on_off;
} ucp_led_t;

typedef struct led_color_s {
	u_int8_t color_of_ac_on;// �յ�����ʱ�����ɫ
	u_int8_t color_of_ac_off;// �յ��ػ�ʱ�����ɫ
	u_int16_t pad;	
}ucp_led_color_t;

typedef struct {
	// AC_POWER_xxx
	u_int8_t onoff;
	// AC_MODE_XXX
	u_int8_t mode;
	// ʵ���¶� �� temp - AC_TEMP_BASE
	u_int8_t temp;
	// ���٣�AC_WIND_xxx
	u_int8_t wind;
	// ����AC_DIR_xxx
	u_int8_t wind_direct;
	// ��ֵ��AC_KEY_xxx
	u_int8_t key;
	//��ʽ�յ�AC_KEY_xxx
//	u_int8_t old_key_value;
} ucp_ac_work_t;

typedef struct {
	u_int8_t id;
	u_int8_t enable;
	u_int8_t week;
	u_int8_t hour;
	u_int8_t minute;
	// AC_POWER_ON / AC_POWER_OFF
	u_int8_t onoff;
	u_int8_t repeat;
	u_int8_t reserved;
} ucp_ac_timer_item_t;

typedef struct {
	u_int8_t on_effect;
    //�´ο���ʱ��
	u_int8_t next_on_day;
	u_int8_t next_on_hour;
    u_int8_t next_on_min;
	//�´ιػ�ʱ��
	u_int8_t off_effect;
	u_int8_t next_off_day;
	u_int8_t next_off_hour;
    u_int8_t next_off_min;
    
	// ucp_ac_timer_item_t item[0];
} ucp_ac_timer_t;

//������ʱ��ͷ��
// 2015.1.20 by tq
typedef struct  ac_timer_head_s{
	u_int8_t on_valid;			/* �Ƿ����´ζ�ʱ���� */
	u_int8_t on_day;			/* �����һ�ζ�ʱ�������� */
	u_int8_t on_hour;			/* �����һ�ζ�ʱ����Сʱ�� */
	u_int8_t on_min;			/* �����һ�ζ�ʱ���������� */
    u_int16_t on_duration;		/* ���һ�ζ�ʱ�������������� */
	u_int8_t off_valid;			/* �Ƿ����´ζ�ʱ�ػ� */
	u_int8_t off_day;			/* �����һ�ζ�ʱ�ػ����� */
	u_int8_t off_hour;			/* �����һ�ζ�ʱ�ػ�Сʱ��*/
	u_int8_t off_min;			/* �����һ�ζ�ʱ�ػ������� */
    u_int16_t off_duration;		/* ���һ�ζ�ʱ�ػ����������� */
}period_timer_head_t;

//������ʱ��
typedef struct {
	u_int8_t id;				/* ����ID */
	u_int8_t hour;			/* Сʱ 0-23 */
	u_int8_t minute;			/* ���� 0-59 */
	u_int8_t week;				/* bit 0-6λ��Ӧ�����쵽������ */
	u_int8_t enable;			/* �Ƿ���Ч(�ֻ�����) �����Ѿ���Ч(�豸����) */
	u_int8_t onoff;				/* �Ƿ��Ƕ�ʱ���� */
	u_int16_t duration;			/* ���������� */
    u_int32_t ext_data_len;     /*��չ���ݳ���*/
} net_period_timer_t;

//���紫��ĵ���ͳ��
typedef struct ia_stat_net_s{
	u_int32_t begin_time;/*utcʱ��*/
	u_int32_t ele;/*ͳ�Ƶ���*/
}ia_stat_net_t;


typedef struct {
	// ��ʼʱ�䣬��λ�����ӣ�0:0����
	u_int16_t begin_minute;
	// �����೤����λ����
	u_int16_t last_minute;
} ucp_stat_peak_time_t;

typedef struct {
    u_int8_t action;
    u_int8_t time_out;
    u_int16_t new_ir_code_id; /*�����л�����ʱ��Ч������ʱ��Ϊpad*/
}ucp_ac_code_match_ctrl_t;

typedef struct {
	u_int8_t is_matched_code;
	u_int8_t is_old_air;
	u_int8_t pad[2];
}ucp_ac_air_attri_t;

typedef struct {
    u_int8_t action;
    u_int8_t step_type;
    u_int8_t step_num;
    u_int8_t cur_step;
    u_int8_t err;
	u_int8_t flagbits;
	u_int8_t pad[2];
}ucp_ac_code_match_stat_t;

typedef struct {
    u_int8_t action;
    u_int8_t step_type;
    u_int8_t step_num;
    u_int8_t cur_step;
    u_int8_t err;
    u_int8_t flagbits;
    u_int8_t pad[2];
    u_int8_t next_key;
    u_int8_t pad1[3];
}ucp_ac_code_next_key_match_stat_t;

typedef struct {
    u_int16_t obj;
    u_int16_t sub_obj;
}ucp_notify_item_t;

typedef struct{
	u_int16_t c_id;
	u_int8_t is_on;
	u_int8_t mode;
	u_int8_t temp;
	u_int8_t fan;
	u_int8_t fan_dir;
	u_int8_t key;
}ucp_ac_code_item_t;

typedef struct{
	u_int16_t cur_match_id;
	u_int16_t code_num;
	ucp_ac_code_item_t items[0];
}ucp_ac_code_match_info_t;

typedef struct {
    u_int32_t phone_index; //��������ֻ����
    u_int32_t phone_share_time;//�����ȥ��ʱ��
    u_int32_t phone_operate_num;//�������ֻ������˶��ٴ��豸
    u_int32_t phone_last_operate_time; //�������ֻ����һ�β����豸��ʱ��
    u_int8_t phone_desc[16];//�������ֻ���������Ϣ
}ucp_share_record_t;

typedef struct {
    u_int8_t action;
    u_int8_t obj_num;
    u_int8_t pad[2];
}ucp_notify_head_t;

/*device product test cur set attribute param UCAT_IR_VOLKADKB*/
typedef struct{
	u_int32_t k;
	u_int16_t ad;
	u_int16_t ad2;
}ucp_pt_set_cur_t;

typedef struct {
	u_int32_t cur_k;
	u_int16_t cur_ad;
	u_int16_t cur_ad2;
	u_int32_t val_k;
	u_int32_t val_b;
}ir_adjust_t;

/*device product test cur get attribute param UCAT_IR_VOLKADKB*/
typedef struct{
	u_int16_t ad;
	u_int16_t ad2;
}ucp_pt_get_cur_t;

typedef struct{
	u_int32_t k;
	u_int32_t b;
}ucp_pt_set_vol_t;

typedef struct{
	u_int16_t ad;
}ucp_pt_get_vol_t;

typedef struct {
	u_int32_t nearest_data_time; //���һ����¼UTC ʱ��
	u_int16_t days_count; //�ж���������
	u_int16_t pad;
	u_int16_t elec_data[0]; //��������
}ucp_elec_days_stat;

typedef struct{
	u_int32_t err_id;
	u_int32_t err_time; 
	u_int16_t err_obj_type;
	u_int16_t err_type;
	u_int32_t err_data;
}ucp_dev_err_item_t;

typedef struct{
	u_int8_t dev_sub_type;
	u_int8_t dev_ext_type;
	u_int8_t err_count;
	u_int8_t pad;
	ucp_dev_err_item_t items[0];
}ucp_dev_err_info_t;

typedef struct {
	u_int32_t current_id;
	u_int32_t num;
} ucp_dev_comm_history_info_t;

typedef struct {
	u_int32_t index;
	u_int32_t num;
} ucp_dev_comm_history_request_t;


typedef struct {
	u_int16_t num;
	u_int16_t item_size;
} ucp_dev_comm_history_hdr_t;

typedef struct {
	u_int32_t id;
	u_int32_t time;
	u_int8_t type;
	u_int8_t condition;
	u_int8_t action;
	u_int8_t pad;
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t temp;
	u_int8_t wind;
	u_int8_t winddir;
	u_int8_t key;
	u_int8_t pad1;
	u_int8_t crc;
} ucp_wukong_history_item_t;

typedef struct{
	u_int8_t up_type; // 1:wifiģ��2: MCU
	u_int8_t pad[3];
}ucp_stm_upgrade_ctrl_t;

typedef struct {
	u_int8_t current_type;
	u_int8_t num;
	u_int8_t supported_types[0];
} ucp_control_board;

#define UCP_KEY_FLAG_SUPPORT_LEARN	BIT(0)
#define UCP_KEY_FLAG_SUPPORT_RENAME	BIT(1)
#define UCP_KEY_FLAG_SUPPORT_DEL	BIT(2)
#define UCP_KEY_FLAG_LEARNED		BIT(3)

#define UCP_KEY_FLAG_SNAPSHOT	    BIT(4) // ����
#define UCP_KEY_FLAG_KNOWN      	BIT(5) // �ܱ�������״̬

typedef struct {
	u_int8_t id;
	u_int8_t flag;
	u_int16_t pad;
	u_int8_t name[16];
} ucp_key_item_t;

typedef struct {
	u_int8_t id;
	u_int8_t flag;
	u_int16_t pad;
	u_int8_t name[16];
	u_int8_t stat[6];
} ucp_snapshort_key_item_t;

typedef struct {
	u_int16_t err;
	u_int8_t stat[6];
} ucp_key_learn_result_t;

typedef struct {
	u_int8_t count;
	u_int8_t pad[3];
	ucp_key_item_t items[0];
} ucp_key_t;

typedef struct {
	u_int8_t action;
	u_int8_t id;
} ucp_key_learn_t;

typedef struct{
	u_int8_t key_id;
	u_int8_t pad[3];
}ucp_key_ctrl_t;

typedef struct{
	u_int8_t pan_type;
	u_int8_t pad[3];
}ucp_pan_ctrl_t;

typedef struct {
	u_int16_t error;
	u_int8_t id;
	u_int8_t pad;
} ucp_key_learn_res_t;

typedef struct {
	u_int16_t error;
	u_int8_t id;
	u_int8_t pad;
	u_int8_t stat[6];
} ucp_key_learn_res_v2_t;

typedef struct{
	u_int32_t active_status; //����״̬ACTIVE_STATUS_XXX
	u_int64_t active_sn;//������ʱ���伤���sn��
}net_active_st_t;


typedef struct {
    u_int8_t action;
    u_int8_t timeout;
    u_int16_t ir_id;
    u_int8_t type;
    u_int8_t rc_id;
    u_int8_t pad[2];
    u_int8_t next_except_key;
    u_int8_t pad1[3];
}ucp_rc_match_t;

typedef struct {
    u_int8_t num;
    u_int8_t item_size;
    u_int8_t pad[2];
}ucp_rc_info_head_t;

typedef struct {
    u_int8_t type;
    u_int8_t rc_id;
    u_int16_t ir_id;
    u_int8_t stat_flag;
    u_int8_t pad[3];
    u_int8_t name[16];
}ucp_rc_info_t;

typedef struct {
    u_int8_t type;
    u_int8_t key_id;
    u_int16_t ir_id;
    u_int8_t rc_id;
    u_int8_t flags;
    u_int8_t pad[2];
    u_int8_t name[16];
}ucp_rc_key_oper_t;

typedef struct {
    u_int8_t type;
    u_int8_t flags;
    u_int16_t ir_id;
    u_int8_t rc_id;
    u_int8_t pad;
    u_int8_t fix_key_size;
    u_int8_t fix_key_num;
    u_int8_t def_key_size;
    u_int8_t def_key_num;
    u_int8_t pad1[2];
}ucp_rc_key_info_head_t;

typedef struct {
    u_int8_t key_id;
    u_int8_t flags;
    u_int8_t pad[2];
}ucp_rc_fixed_key_info_t;

typedef struct {
    u_int8_t key_id;
    u_int8_t flags;
    u_int8_t pad[2];
    u_int8_t name[16];
}ucp_rc_user_def_key_info_t;

typedef struct {
    u_int8_t type;
    u_int8_t key_id;
    u_int8_t rc_id;
    u_int8_t action;
    u_int8_t pad[4];
}ucp_rc_key_learn_t;

typedef struct {
    u_int8_t type;
    u_int8_t key_id;
    u_int16_t ir_id;
    u_int8_t rc_id;
    u_int8_t flags;
    u_int8_t pad[6];
}ucp_rc_key_ctrl_t;

typedef struct {
    u_int8_t num;
    u_int8_t pad[3];
    u_int8_t line[24*6];
}ucp_24hour_line;

typedef struct {
    u_int8_t action;
    u_int8_t pad[3];
}ucp_child_lock_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t min;
	u_int8_t max;
	u_int8_t resv;
} ucp_temp_alarm_t;

typedef struct {
	u_int8_t enable;
	u_int8_t onoff;
	u_int16_t remain_time;//��ݶ�ʱ��ʣ��ʱ�䣬����
} ucp_shortcuts_onoff_t; 
////////////////////////////////////////////////////////////////////////////////////////////////////////
//�������
typedef struct {
	u_int8_t on;
	u_int8_t keep_time;
	u_int8_t last_on_time;
	u_int8_t pad;
}ucp_car_on_t;

typedef struct {
	u_int8_t horn_num;
	u_int8_t horn_time;
	u_int8_t horn_interval;
	u_int8_t light_num;
	u_int8_t light_time;
	u_int8_t light_interval;
	u_int8_t pad[2];
}ucp_car_search_t;

typedef struct {
	u_int8_t temp;
	u_int8_t pad[3];
}ucp_car_temp_t;

typedef struct {
	u_int16_t valtage;
	u_int8_t on;
	u_int8_t pad;
}ucp_car_valcheck_t;

typedef struct {
	u_int8_t on;
	u_int8_t pad[3];
}ucp_car_powersave_t;

typedef struct {
	u_int8_t ele_per;
	u_int8_t pad[3];
}ucp_car_percentage_t;

typedef struct {
    u_int8_t enable;
    u_int8_t low;
    u_int8_t max;
    u_int8_t pad;
}ucp_ep_temp_range_t;

#define SC_SET 0x0
#define SC_GET 0x1

typedef struct {
    u_int8_t sub_cmd;
    u_int8_t type;
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
}ucp_rf_led_lamp_t;

////////////////////////
//����

enum{
    UP_TLV_SMTDLOCK_CMD_GET_SUMMARY = 0x1,
    UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY = 2,
    RF_TT_CMD_PUSH_ALARM = 3,	// ��ȡ������Ϣ
	//rf�豸��һЩͨ��tlv type
	RF_TT_CMD_RAWDATA = 4,	// ����͸��
	RF_TT_CMD_GET_TIMER_SUMMARY  =  5,//��ȡrf�豸��ʱ��ժҪ
	RF_TT_CMD_ADD_TIMER  =  6,//�޸���Ӷ�ʱ��
	RF_TT_CMD_DEL_TIMER_RET  =  7,//ɾ����ʱ��
	RF_TT_CMD_GET_NEXT_TIMERS  =  8,//��ѯ�´�ִ�ж�ʱ��
	RF_TT_CMD_GET_TIMER  =  9,//��ѯ����Ķ�ʱ����Ϣ
	RF_TT_CMD_TIMER_UPDATE_PUSH = 10,//�豸����push�����Ķ�ʱ��Ϣ
	RF_TT_CMD_TIMER_DEL_PUSH = 11,//ɾ����ʱ�����豸push�����Ķ�ʱ����Ϣ

	UP_TLV_BIG_DATA_SEND_REQUEST = 12,//���ķ�������
    UP_TLV_BIG_DATA_GET = 13,//���ķ��ͻ�ȡ
    UP_TLV_BIG_DATA_RESPONSE = 14,//���ķ���
    UP_TLV_BIG_DATA_RET = 15, // ���ķ��ؽ��

	RF_TT_CMD_GET_RUN_TIME = 16,// ��ȡ���豸����ʱ��

	RF_TT_CMD_QUERY_HISLOG_V2 = 32,	// �µ���ʷ��¼��ѯ�ӿ�
    UP_TLV_SMTDLOCK_CMD_GET_ONOFFTIME = 33,
    UP_TLV_SMTDLOCK_CMD_PUSH_ONOFFTIME = 34,
    UP_TLV_SMTDLOCK_CMD_SET_ONOFF_DOORLOCK = 35,
    UP_TLV_SMTDLOCK_CMD_SET_AUTO_DOORLOCK = 36,
    UP_TLV_SMTDLOCK_CMD_SET_DEFENSE = 37,
    RF_TT_CMD_QUERY_HISLOG = 38,
    UP_TLV_SMTDLOCK_CMD_SET_SLALARM = 39,
    UP_TLV_SMTDLOCK_CMD_SET_PRESS_RCTRL = 40,
    UP_TLV_SMTDLOCK_CMD_PUSH_RTCTRLID = 41,
    UP_TLV_SMTDLOCK_CMD_SET_RTCTRL_INFO = 41,
    UP_TLV_SMTDLOCK_CMD_GET_AUTO_DEFENSE = 42,
    UP_TLV_SMTDLOCK_CMD_PUSH_RTCTRL_INFO = 43,
    UP_TLV_YT_LOCK_CMD_CTRL_LOCK = 44,
    SMTDLOCK_CMD_SET_AUTO_GUARD = 45,
    SENSORFT_CMD_PUSH_SUMMARY = 46,
    SENSOR_BODY_DETECT_PUSH = 47,
    UP_TLV_RF_DEV_SET_ALARMTIME = 48,
    UP_TLV_RF_DEV_PUSH_ALARMTIME = 49,
    UP_TLV_SMTDLOCK_CMD_SET_WIFI_LOCK = 51, //���ý���WIFI�Զ�����,�Ͽ�WIFI�Զ�����
	UP_TLV_SMTDLOCK_CMD_SET_UNLOCK_TIMEOUT = 52, //����δ������ʱʱ��
    UP_TLV_AIR_IR_LEARN_CTRL = 53, // �ֻ�֪ͨ�豸��ʼ��ֹͣ����ѧϰ
    UP_TLV_AIR_IR_LEARN_RET = 54, //	�豸����ѧϰ���
    UP_TLV_AIR_IR_CTRL = 55,
    UP_TLV_RF_DEV_SET_ALARMCLC = 56,	// ���������������Щ�豸��һֱ����
    UP_TLV_RF_DEV_SET_ALARM_TYPE = 57,	// ���ñ������ͣ���һֱ����ֱ��APPȷ�ϻ���ֻ����һ��
    UP_TLV_RF_DEV_VERSION = 59,	// �豸�汾��Ϣ
    UP_TLV_RF_DEV_ALARM_DEMO = 60,	// �豸������ʾ
};

typedef struct {
    u_int8_t d_type; //��������
    u_int8_t frame_seq; //�������
}ucp_big_pkt_hdr;

//�յ���
enum{
    WK_TLV_CODE_MATCH,
    WK_TLV_STAT_INFO
};

typedef struct {
    u_int8_t type; // 0:��ȡ����״̬ 1����ȡ����ң������Ϣ 0xff�����пɲ�ѯ״̬
    u_int8_t pad;
}ucp_rf_door_lock_q_t;

typedef struct {
    u_int8_t value;
    u_int8_t pad;
}ucp_rf_door_com_ctrl_t;

typedef struct {
    u_int8_t action;
    u_int8_t pad[3];
    u_int32_t old_pass;
    u_int32_t cur_pass;
}ucp_yt_rf_door_lock_ctrl_t;

typedef struct {
    //u_int8_t h_ver;
    //u_int8_t s_ver;
    u_int8_t unlock_timeout_enable;
	u_int8_t unlock_timeout;
    u_int8_t battery;
    u_int8_t pad;
    u_int32_t flag_bits;
}ucp_rf_door_lock_stat_t;

// ���ó�ʱ��δ���ų�ʱʱ��
typedef struct {
	u_int8_t enable;
	u_int8_t timeout;
} ucp_door_unlock_timeout_set_t;

// ����������߶Ͽ�WIFI�Զ��������߹���
typedef struct {
	u_int8_t type;	// 
	u_int8_t enable;
	u_int8_t starthour;
	u_int8_t endhour;
} ucp_door_wifi_lock_set_t;

typedef struct {
    u_int8_t action;
    u_int8_t error;
}ucp_yt_lock_ctrl_resp_t;

typedef struct {
    u_int8_t h_ver;
    u_int8_t s_ver;
    u_int8_t battery;
    u_int8_t pad;
    u_int32_t flag_bits;
    int8_t temp;
    u_int8_t hum;
}ucp_hm_temp_hum_t;

typedef struct {
    u_int8_t h_ver;
    u_int8_t s_ver;
    u_int8_t battery;
    u_int8_t pad;
    u_int32_t flag_bits;
    u_int32_t time;
    u_int32_t num;
}ucp_hm_body_t;

typedef struct {
    u_int32_t flags;
    u_int32_t time;
}ucp_hm_body_push_t;

typedef struct {
    int8_t temp;
    u_int8_t hum;
}ucp_hm_temp_hum_item_t;

typedef struct {
    u_int8_t hour;
    u_int8_t pad;
    ucp_hm_temp_hum_item_t th[12];
}ucp_hm_temp_hum_his_t;

typedef struct {
	u_int8_t index;
	u_int8_t pad;
	ucp_hm_temp_hum_item_t th[12];
} ucp_hm_temp_hum_his_v2_t;

typedef struct {
    u_int8_t on_off1;
    u_int8_t on_off2;
    u_int8_t on_off3;
    u_int8_t on_off4;
    u_int32_t time_1;
    u_int32_t time_2;
    u_int32_t time_3;
    u_int32_t time_4;
}ucp_rf_door_history_t;


typedef struct {
	u_int8_t type;	//��ʷ��¼����:�����ţ����ţ��͵�����������(������˭����)
	u_int8_t value;	//����ֵ
	u_int8_t ex_type; //��չ����
	u_int8_t ex_value;//��չֵ
	u_int32_t timestapm;//ʱ���
} ucp_rf_hislog_info_t;

typedef struct {
	u_int32_t timestapm;//ʱ���
	// type �ĸ�4λΪ�����ͣ���4λΪ��չ����
	u_int8_t type;	//��ʷ��¼����:�����ţ����ţ��͵�����������(������˭����)
	u_int8_t value;	//����ֵ
	u_int8_t ex_value;//��չֵ
	u_int8_t crc;	// xor
} ucp_rf_hislog_info_v2_t;

typedef struct {
    u_int8_t rc_id;
    u_int8_t op; //1-ȡ����ʧ��2-��ʧ��3-���ϣ�4-����
    u_int8_t pad;
    u_int8_t name_len;
}ucp_rf_door_lock_remote_t;

typedef struct {
    u_int8_t on_enable;
    u_int8_t off_enable;
    u_int8_t on_start_hour;
    u_int8_t on_stop_hour;
    u_int8_t off_start_hour;
    u_int8_t off_stop_hour;
}ucp_rf_auto_guard_stat_t;

typedef struct {
    u_int8_t type;
    u_int8_t enable;
    u_int8_t start_hour;
    u_int8_t end_hour;
}ucp_rf_auto_guard_ctrl_t;

typedef struct {
	/*	
	0 bit���������;���ֵ����
	1-7bit��ָ������
	0���豸������ʼ����ʱ�䣬����bit 0�����ĺ���Ϊ��1-������ʼʱ��0-��������ʱ��
	*/
	u_int8_t ctrl;
	u_int8_t pad[3];
	u_int32_t time;
} ucp_rf_alarm_time_t;

typedef struct {
	u_int8_t alarm_clc;	// �Ƿ�������� 1��ʾ���
	u_int8_t resv;
} ucp_rf_alarm_clc_t;
////////////////////////
// RFů����

enum {
	UP_TLV_HV_UART_TT = RF_TT_CMD_RAWDATA,	// ͸���Ĵ����豸
	UP_TLV_HV_PERIOD = 200,	// ������Ϣ����Ƭ�����

	UP_TLV_HV_GET_QUICK_TIMER = 210,
	UP_TLV_HV_SET_QUICK_TIMER = 211,

	UP_TLV_HV_GET_STATUS = 212,
	UP_TLV_HV_GET_TIME = 213,
	UP_TLV_HV_GET_CIRCLE = 214,
	UP_TLV_HV_STAT_QUERY = 215,
};

////////////////////////
// ����RF����
enum {
	UP_TLV_KTCZ_GET_SUMMARY = 1,
	UP_TLV_KTCZ_SET_ONOFF = 52,
	UP_TLV_KTCZ_GET_ONOFF = 53,
	UP_TLV_KTCZ_PUSH_ONOFF = 53,
}; 

////////////////////////
// ͨ��̽����
enum {
	UP_TLV_COM_DETECTOR_GET_SUMMARY = 1,
	UP_TLV_COM_DETECTOR_PUSH_SUMMARY = 2,
	UP_TLV_COM_DETECTOR_GET_ONOFFTIME = 33,
	UP_TLV_COM_DETECTOR_PUSH_ONOFFTIME = 34,
	UP_TLV_COM_DETECTOR_SET_DEFENSE = 37,
	UP_TLV_COM_DETECTOR_SET_ALARMTIME = 48,
	UP_TLV_COM_DETECTOR_PUSH_ALARMTIME = 49,
}; 

typedef struct {
    u_int8_t on_off1;
    u_int8_t on_off2;
    u_int8_t on_off3;
    u_int8_t on_off4;
    u_int32_t time_1;
    u_int32_t time_2;
    u_int32_t time_3;
    u_int32_t time_4;
} ucp_com_detector_history_t;

typedef struct {
    u_int8_t ctl_1;
    u_int8_t ctl_2;
    u_int8_t ctl_3;
    u_int8_t ctl_4;
    u_int32_t time_1;
    u_int32_t time_2;
    u_int32_t time_3;
    u_int32_t time_4;
} ucp_com_detector_alarm_time_t;



typedef struct {
	u_int8_t hw_ver;
	u_int8_t soft_ver;
	u_int8_t abc_battery;
	u_int8_t resv;
	u_int32_t flagbits;
} ucp_com_detector_stat_t;

typedef struct {
    u_int8_t value;
    u_int8_t pad;
} ucp_rf_com_detector_ctrl_t;

typedef struct {
	u_int32_t id;
	u_int32_t record_time;
	u_int16_t object;
	u_int16_t type;
	u_int8_t alarm_index;
	u_int8_t resv;
	u_int16_t value;
} ucp_rf_push_alarm_t;

typedef struct {
	u_int8_t hw_ver;
	u_int8_t soft_ver;
	u_int16_t resv;
	u_int32_t svn;
} ucp_com_detector_version_t;


//���û����
typedef struct {
	u_int16_t tmp;
	u_int16_t pad;
}ucp_tbb_tmp_t;

typedef struct {
	u_int8_t mode;
	u_int8_t pad[3];
}ucp_tbb_mode_t;

typedef struct {
	u_int8_t on;
	u_int8_t pad[3];
}ucp_tbb_on_t;

typedef struct {
	u_int16_t tmp;
	u_int16_t pad;
}ucp_tbb_bert_t;

//���� 
typedef struct {
	u_int8_t compressor_onoff;
	u_int8_t extern_wind_onoff;
	u_int8_t four_valve;
	u_int8_t ele_hot;
	u_int16_t compressor_work_hours;
	u_int8_t down_reboot;
	u_int8_t wind_real_spee;
	u_int8_t inner_tmp;
	u_int8_t heat_defrost;
	u_int8_t sys_type;
	u_int8_t pad1;
	u_int8_t dc_busway_val;
	u_int8_t extern_ac_val;
	u_int8_t extern_ac_cur;
	u_int8_t compressor_cur;
	u_int8_t compressor_freq;
	u_int8_t outside_tmp;
	u_int8_t exhaust_tmp;
	u_int8_t ipm_tmp;
	u_int8_t in_fans_gears;
	u_int16_t assis_work_hours;
	u_int8_t pad2;
}ucp_yt_stat_t;

typedef struct {
	u_int8_t fault_b2;
	u_int8_t fault_b15;
	u_int8_t protect_b16;
	u_int8_t protect_b17;
	u_int8_t protect_b18;
	u_int8_t ele_hot;
	u_int8_t four_valve;
	u_int8_t pad;
}ucp_yt_protect_t;

//��ʯ
typedef struct {
    u_int16_t stat_flag;
    u_int8_t work_mode; //����ģʽ,�� JS_WM_XX()
    u_int8_t t_min; // ʱ�䣺����
    u_int8_t t_sec; // ʱ�䣺����
    u_int8_t wave_fire; // ΢������
    u_int8_t barbecue_fire; // �տ�����
    u_int8_t hot_fan_temp; // �ȷ��¶�
    u_int16_t food_weight; // ʳ������
    u_int8_t cur_temp; // ��ǰ�¶�
    u_int8_t work_sub_mode; //������ģʽ
    u_int8_t cur_min; //��ǰʱ��
    u_int8_t cur_sec; // ��ǰ����
    u_int8_t pad[2];
}ucp_js_stat_t;

typedef struct {
    u_int8_t work_mode; //����ģʽ
    u_int8_t work_min; //���������� 1-60����
    u_int8_t work_sec; //�������� 0-60 �룬��������0ʱ��������������0
    u_int8_t wave_fire; //΢��������С
    u_int8_t barbecue_fire; // �տ�����
    u_int8_t hot_fan_temp; // �ȷ��¶�
    u_int16_t food_weight; // ʳ�������������Ƿ���Ҫ��
    u_int8_t work_sub_mode; //������ģʽ
    u_int8_t action; // ������ ������ȡ������ͣ
    u_int8_t pad[2];
}ucp_js_ctrl_t;

typedef struct {
    u_int8_t action;
    u_int8_t pad[3];
}ucp_js_fast_ctrl_t;

typedef struct {
    u_int8_t on_off;
    u_int8_t pad[3];
}ucp_js_child_lock_ctrl_t;

typedef struct {
    u_int8_t value;
    u_int8_t pad[3];
}ucp_kxm_ther_ctrl_t;

typedef struct {
	u_int32_t index;//��������ֻ����
	u_int8_t desc[16];//�������ֻ���������Ϣ
}ucp_share_desc_info_t;

/*****************ͨ�ö�ʱ��****************************/
//ͨ�ö�ʱ�����Ͷ��壬����Ǹ��豸��Э�̵�����
enum {
	UT_DEV_TYPE_ONOFF = 1, //���ض�ʱ��
	UT_DEV_TYPE_OFF = 2,//��ʱ��
	UT_DEV_TYPE_PERIOD_ONCE = 3,//ʱ��ζ�ʱ������ִֻ�п�ʼ�ͽ������������м�״̬
	UT_DEV_TYPE_ADVANCE_TIMER = 4,//�߼���ʱ��
	UT_DEV_TYPE_PERIOD_CONSTANT_TEMP = 5, //����
	UT_DEV_TYPE_PERIOD_TEMP_CURVE = 6, //�¶�����
	UT_DEV_TYPE_MAX
};

typedef struct {
	u_int8_t mode;
	u_int8_t tmp;
	u_int16_t pad;
}ucp_comm_advance_timer_t;

typedef struct {
	u_int8_t run_mode;//����ģʽ,0-���ȣ�1-���䣬2-����
	u_int8_t wind_speed;//���٣�0-�ͷ磬1-�з磬2-�߷�
	u_int16_t tmp;//�����¶ȣ�350��ʾ35��,��Χ50-350
	u_int8_t scene_mode;//�龰ģʽ��0-���£�1-���ܣ�2-���
	u_int8_t pad[3];
}ucp_linkon_advance_timer_t;

typedef struct {
	u_int32_t on_off_stat;
}ucp_dhxml_advance_timer_t;

typedef struct {
	u_int16_t on_off_stat;
}ucp_dhxml_advance_timer_2_t;

//�յ�����ʱ����չ����
typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t tmp;
	u_int8_t win;
	u_int8_t dir;
	u_int8_t key;
	u_int8_t code[0];
}ucp_comm_wkair_ext_t;

typedef struct {
	u_int8_t id;
	u_int8_t enable;
	u_int8_t type;
	u_int8_t week;
	u_int32_t start_time;
	u_int16_t duration;
	u_int8_t extened_len;
    u_int8_t data[0];
}ucp_comm_timer_t;

typedef struct {
	u_int8_t id;
	u_int8_t enable;
	u_int8_t type;
	u_int8_t week;
	u_int8_t hour;
	u_int8_t min;
	u_int16_t duration;
	u_int8_t extened_len;
    u_int8_t data[0];
}ucp_comm_timer_rf_t;

typedef struct {
	u_int8_t max_timer_count;
	u_int8_t max_type_count;
	u_int8_t max_data_len;
	u_int8_t reserve;
	u_int32_t map;
}net_dev_timer_summary_t;

typedef struct {
	u_int8_t max_timer_count;//���֧�ֶ��ٸ���ʱ��
	u_int8_t max_data_len;//��չ����֧�ֵ���󳤶�
	u_int16_t support_type;//bit0-bit15�ֱ��ʾ֧��type1-type16������
	u_int32_t map;
	u_int8_t time_change[0];//map���Ӧ��ʱ��ͳ�Ƽ��� max_timer_count*u8
}net_dev_timer_summary_ext_t;


////////////////////////
// ͨ��RF ����ͷ��

typedef struct {
    u_int8_t d_type; //��������
    u_int8_t pkt_seq; //���������
}ucp_rf_big_pkt_hdr_t;


typedef struct {
    u_int16_t pkt_len;
    u_int8_t frame_size;
}ucp_rf_big_pkt_req_t; //ͨ�濪ʼ���������

typedef struct {
    u_int8_t index;
    u_int8_t crc;
}ucp_rf_big_pkt_content_t; //�����屨�ĵ�

enum{
    BP_ERR_NONE = 0x0,
    BP_ERR_DEV_BUSY,
    BP_ERR_PKT_TOO_BIG,
    BP_SEND_PKT_OK,
    BP_RECV_PKT_OK
};

typedef struct {
    u_int8_t error;
    u_int8_t pad;
}ucp_rf_big_pkt_err_t; //���ݽ��������Ż᷵��

////////////////////////////////////////////
// RF�豸�յ�����ѧϰ���
enum{
    ACT_START_IR_LEARN = 0x1,
    ACT_STOP_IR_LEARN,
    ACT_DEV_WAIT_IR_SINGAL,
    ACT_DEV_DATABASE_RECV,
    ACT_DEV_RECV_IR
};
enum{
    UCP_IR_LEARN_ERR_DEV_BUSY = 0x1,
    UCP_IR_LEARN_ERR_DEV_TIME_OUT
};

typedef struct {
    u_int8_t action;
	u_int8_t ir_num;
	u_int8_t timeout;//��1��Ϊ��λ��0��ʾĬ�ϳ�ʱ���豸�Ͽ�����30��
    u_int8_t pad;
}ucp_air_ir_learn_ctrl_t;

typedef struct {
    u_int8_t action;
    u_int8_t error;
	u_int8_t ir_num;//�����������Լ���ǰ�β��������ⶪ��
    u_int8_t pad;
}ucp_air_ir_learn_resp_t;

////////////////////////
// �յ���

enum{
    // ����֮�����������
    BD_IR_CT_IR_UPLOAD = 0x1, //  �豸�ϴ�������Ϣ���ֻ�
    BD_IR_CT_IR_DOWNLOAD, //  �ֻ�������ɺ󣬴��ݲ��ֱ������Ϣ���豸
    BD_IR_CT_IR_CTRL,  //  �ֻ����ͺ������������ֻ�
    BD_COMM_TIMER_CTRL,//ͨ�ö�ʱ����
};

typedef struct {
    u_int8_t onoff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t fan;
    u_int8_t fan_dir;
    u_int8_t key_id;
    u_int8_t code[0];
}ucp_wk_air_ir_ctrl; //�������ָ��


typedef struct{
    u_int16_t ir_id;
    u_int8_t battary; //����
    int8_t room_temp;
    u_int8_t room_humi;
    u_int8_t onoff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t fan;
    u_int8_t fan_dir;
    u_int8_t key_id;
    u_int8_t charge;
	u_int8_t tmp_adjust;//�����¶�
	u_int8_t led_mode;//led��ģʽ
}ucp_wk_air_stat_t;

typedef struct {
	u_int8_t action;
	u_int8_t sub_type;
	u_int16_t total_len;
}ucp_wk_air_req_t;

typedef struct {
	u_int8_t action;
	u_int8_t sub_type;
	u_int8_t ret;
	u_int16_t total_len;
	u_int8_t pad;
}ucp_wk_air_req_r_t;

typedef struct {
	u_int8_t sub_type;
	u_int16_t offset;
	u_int8_t rw_len;
	u_int8_t checksum;
	u_int8_t data[0];
}ucp_wk_air_rw_t;

typedef struct {
	u_int8_t sub_type;
	u_int16_t offset;
	u_int8_t ret;
	u_int8_t check_sum;
	u_int8_t data[0];
}ucp_wk_air_rw_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t sub_type;
	u_int8_t checksum;	// ȫ�����ݵ�У���
}ucp_wk_air_ret_t;

typedef struct {
	u_int8_t action;
	u_int8_t sub_type;
	u_int8_t ret;
}ucp_wk_air_ret_r_t;

// linkon�¿���
typedef struct {
	u_int8_t power;
	u_int8_t lock;
	int16_t house_temp;
	u_int16_t const_temp;
	u_int16_t go_out_temp;
	u_int16_t save_temp;
	u_int8_t humidity;
	u_int8_t work_mode;
	u_int8_t running_mode;
	u_int8_t wind_speed;
	u_int16_t pad;
} ucp_linkon_stat_t;

typedef struct {
	u_int8_t value;
	u_int8_t pad[3];
} ucp_linkon_set_t;

typedef struct {
	u_int16_t value;
	u_int8_t pad[2];
} ucp_linkon_temp_set_t;

//�ǻʴ����������ݽṹ
typedef struct {
	u_int32_t magic;
	u_int8_t index;
	u_int8_t type;
	u_int16_t pad;
}ucp_zhcl_bind_t;

typedef struct {
	u_int8_t index;
	u_int8_t type;
	u_int16_t pad;
}ucp_zhcl_type_t;

enum {
	CODE_PROCESS_ACTION_CANCEL = 0,
	CODE_PROCESS_ACTION_MATCH = 1,
	CODE_PROCESS_ACTION_ALL_MATCH = 2,
	CODE_PROCESS_ACTION_COMPLETE = 3,
	CODE_PROCESS_ACTION_ERR = 4,
	CODE_PROCESS_ACTION_SET = 5,
	CODE_PROCESS_ACTION_MATCH_STATE = 6,
};
typedef struct {
	// CODE_PROCESS_ACTION_XXX
	u_int8_t action;
	u_int8_t time_out;
	u_int16_t id;
} ucp_code_process_request_t;


typedef struct {
	u_int16_t developer_id;
	u_int8_t rsv;
	u_int8_t ver;
	u_int8_t data[0];
} ucp_app_data_hdr_t;

typedef struct {
	u_int8_t std;
	u_int8_t resv;
	u_int16_t thr;
} ucp_rf_jq_set_thr_t;

typedef struct {
	u_int32_t timestamp;
	u_int16_t ch20;
}ucp_rf_jq_his_item_t;

typedef struct {
	u_int8_t hislog_index;
	u_int8_t v_history_index;
	ucp_rf_jq_his_item_t item[0];
}ucp_rf_jq_his_query_t;

//�ǻʵ�����
typedef struct {
	u_int8_t onoff;
	u_int8_t mask;
}ucp_zhdhx_onoff_t;

typedef struct {
	u_int8_t key_num;
	u_int8_t key_len;
	u_int8_t name[0];
}ucp_zhdhx_name_set_t;

//��־���
typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
	u_int32_t home_id;
	u_int32_t index;
	u_int32_t query_count;
}ucp_log_rule_query_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int32_t max_index;
	u_int16_t record_count;
	u_int16_t pad;
}ucp_log_rule_query_r_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve[3];
	u_int32_t home_id;
}ucp_log_member_query_t;

typedef struct {
	u_int64_t sn;
	u_int8_t action;
	u_int8_t reason;
	u_int8_t pad[2];
	u_int32_t user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];
	u_int32_t time_stamp;
}ucp_log_member_item_t;

typedef struct {
	u_int8_t action;
	u_int8_t reserve;
	u_int16_t result;
	u_int32_t home_id;
	u_int16_t record_count;
	u_int16_t pad;
	ucp_log_member_item_t item[0];
}ucp_log_member_query_r_t;


typedef struct {
	u_int8_t name[24];		// �û���
	u_int64_t phome_number;	// �ֻ���
} ucp_hpgw_phone_user_t;

typedef struct {
	u_int8_t support_appinfo;	// �Ƿ�֧�ֶ����ϵ��APP��Ϣ����
	u_int8_t support_sms;		// �Ƿ�֧�ֶ���
	u_int8_t sms_lang;			// �������� 0 ���� 1 Ӣ��
	u_int8_t phone_user_num;	// �ֻ��û����������16��
	ucp_hpgw_phone_user_t users[0];
} ucp_hpgw_info_t;

typedef struct {
	u_int8_t appinfo;
	u_int8_t pad[3];
} ucp_hpgw_appinfo_request_t;

typedef struct {
	u_int8_t sms;
	u_int8_t lang;
	u_int8_t pad[2];
} ucp_hpgw_hpgw_sms_request_t;

typedef struct {
	u_int8_t name[24];			
	u_int64_t phone_number;
} ucp_hpgw_phone_user_request_t;

typedef struct {
	u_int8_t name[24];
	u_int64_t phone_number;
} ucp_hpgw_phone_user_del_t;


typedef struct {
	u_int8_t is_front; //app�Ƿ���ǰ̨
	u_int8_t pad[3];
	u_int32_t reserved[7];
} ucp_phone_status_t;

typedef struct {
	u_int32_t seq;
	uc_tlv_t tlv[0];
} ucp_app2server_t;

//Ӳ����Ϣ���ݽṹ
typedef struct {
	u_int16_t temp;
	u_int8_t pad[2];
	u_int32_t use_time;
	u_int32_t capacity;
	u_int8_t model[64];
	u_int8_t serial[64];
	u_int8_t reserved[8];
}ucp_disk_info_item_t;

typedef struct {
	u_int8_t mode;
	u_int8_t num;
	u_int8_t pad[2];
	u_int32_t total_capacity;
	u_int32_t used_capacity;
	u_int8_t reserved[8];
	ucp_disk_info_item_t disk_item[0];
}ucp_disk_info_t;

typedef struct {
	u_int8_t index;
	u_int8_t pad[3];
	u_int32_t tx_rate;
	u_int32_t rx_rate;
	u_int32_t ip;
	u_int8_t name[16];
	u_int8_t reserved[8];
}ucp_eth_info_item_t;

typedef struct {
	u_int8_t num;
	u_int8_t pad[3];
	u_int8_t reserved[8];
	ucp_eth_info_item_t eth_item[0];
}ucp_eth_info_t;

typedef struct {
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t lang;
	u_int8_t vendor[16];
	u_int8_t developer_id[32];
	u_int64_t mastersn;
	u_int64_t sn;
	u_int8_t upstm;
	u_int8_t uprfstm;
	u_int8_t pad[2];
} misc_upgrade_query_t;

typedef struct {
	u_int8_t result;
	u_int8_t pad[3];
	u_int64_t sn;
	ucp_version_t ver;
	u_int32_t url_ver_len;
	ucp_version_t stm_ver;
	u_int32_t url_stm_ver_len;
	ucp_version_t rfstm_ver;
	u_int32_t url_rfstm_ver_len;
	u_int8_t desc[64];
	u_int8_t release_time[64];
	u_int8_t url[0];
} misc_upgrade_query_reply_t;

typedef struct {
	u_int64_t sn;
	u_int8_t token[256];
	u_int8_t action;
	u_int8_t need;
	u_int16_t cid;
	u_int8_t iver[8];
	u_int8_t prefix[64];
	u_int8_t mipush[64];
	u_int8_t language[16];
	u_int8_t push_music[64];
	u_int32_t user_id;
} misc_apns_request_t;

typedef struct {
	u_int8_t result;
	u_int8_t need;
	u_int8_t pad[2];
	u_int64_t sn;
	u_int8_t language[16];
	u_int8_t push_music[64];
} misc_apns_reply_t;

//widget���
typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
}ucp_widget_key_request_t;

typedef struct {
	u_int8_t action;
	u_int8_t pad;
	u_int16_t result;
	u_int8_t key[32];
}ucp_widget_key_reply_t;

/*ͨ��Э�����ݽṹ1�ֽڶ��룬�����ݽṹ����ڱ���֮ǰ*/
#pragma pack(pop)

#define TLV_2_2_13_SAME_ONOFF       0x1
#define TLV_2_2_13_SAME_FAN   0x2
#define TLV_2_2_13_FAN_SPEED_REVERSE 0x3
#define TLV_2_2_13_ROOM_TEMP_AJUST  0x4
#define TLV_2_2_13_ELE_AJUST        0x5
#define TLV_2_2_13_ALL_ELE_CLEAR        0x6

enum{
    ACT_RC_KL_START = 0x0,
    ACT_RC_KL_STOP
};

/*****************************************
	���ú���
 *****************************************/
#ifndef	 WIN32
#define SOCKET int
#endif

//�ϲ�ʹ���豸����

static inline u_int16_t mk_udp_dev_type(u_int8_t sub_type, u_int8_t ext_type)
{
	return (((sub_type)<<8)|(ext_type));
}

static inline bool udp_dev_type_equl(user_t * user,u_int16_t sub_type,u_int16_t ext_type)
{
	if(!user)
		return false;
	return mk_udp_dev_type(user->sub_type,user->ext_type)==mk_udp_dev_type((u_int8_t)sub_type,(u_int8_t)ext_type);
}

u_int32_t get_msec();	


extern pkt_t *uc_pkt_new(void* s,int cmd, int param_len,
			bool is_request, bool is_enc, u_int8_t flags,
			u_int32_t client_sid, u_int32_t device_sid, u_int32_t request_id);

void pkt_free(void *pkt);

extern void uc_hdr_order(ucph_t *hdr);

extern void uc_set_time_param(uc_time_param_item_t *it,
		u_int8_t retry0, u_int8_t retry1, u_int8_t retry2,
		u_int8_t keeplive, u_int16_t die);

extern void order_uc_time(uc_time_t *t);

extern int uc_send_pkt_raw(SOCKET sock, u_int32_t ip, u_int16_t port, pkt_t *pkt);
extern int uc_send_data_raw(SOCKET sock, u_int32_t ip, u_int16_t port, void *data, int len);

extern void gen_rand_block(u_int8_t *dst, int size);
extern void uc_set_default_time_param(uc_time_param_all_t *time_param, uc_time_param_all_t *time_param_net);
extern void gen_uuid_zero(u_int8_t *v, int len);

////////////////////////////////////////////////////////////////////////////
// TLV ͨ�ú���

#define MAX_TLVM_LENGTH (8192)

extern RS tlvm_init(tlv_manger_t* m);
extern RS tlvm_free_data(tlv_manger_t* m);
extern RS tlvm_add_tlv(tlv_manger_t* m,u_int16_t type,u_int16_t len,void* data);
extern RS tlvm_add_u8_tlv(tlv_manger_t* m,u_int16_t type,u_int8_t u8_data);
extern RS tlvm_add_u16_tlv(tlv_manger_t* m,u_int16_t type,u_int16_t u16_data,bool sw_bit_order);
extern RS tlvm_add_u32_tlv(tlv_manger_t* m,u_int16_t type,u_int32_t u32_data,bool sw_bit_order);
extern RS tlvm_add_string_tlv(tlv_manger_t* m,u_int16_t type,char* string);

#endif

