/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: ����ͨѶ���ͷ�ļ�
**  File:    linkage_client.h
**  Author:  liubenlong
**  Date:    11/30/2015
**
**  Purpose:
**    ����ͨѶ���ͷ�ļ�.
**************************************************************************/


#ifndef LINKAGE_CLIENT_H
#define LINKAGE_CLIENT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "udp_ctrl.h"
#include "cl_linkage.h"


/* Macro constant definitions. */
#define LA_SUPPORT_FRAGMENT	// �Ƿ�֧�ַ�Ƭ����
#define LA_FRAGMENT_SIZE 1200

#define	LA_MAX_UDP_PKT	(64*1024)

#define UCLA_ADD_MAX_TIMES		(1)

#define UCLA_FRAGMENT_MAX_ITEMS	(8)
#define UCLA_FRAGMENT_MAX_DYNAMIC_ITEMS (255)
//flag
#define 		PHONE_USER				(1)
#define 		DONAME_CONFLICT_USER	(2)


//һ����������ip������
#define DNS_IP_MAX_NUM	(20)

#define VERTION(maj, min, rev)		((maj&0xff) << 24| (min&0xff) << 16| (rev&0xff) <<8)

/* Type definitions. */
//�ֻ��˺��¼�����״̬��
enum {
	UCLA_PHONE_IDLE = 0,//����
	UCLA_PHONE_CREATE,//�����ֻ��˺�״̬
	UCLA_PHONE_LOGIN,//��½�ֻ��˺�״̬
	UCLA_PHONE_LOGOUT,//�ǳ��ֻ��˺�״̬
	UCLA_PHONE_SWICH,//�л��ֻ��˺�״̬
	UCLA_PHONE_DEL,//ɾ���ֻ��˺�״̬
	UCLA_PHONE_PASSWD_MODIFY,//�ֻ��˺������޸�
	UCLA_PHONE_MAX,
};


enum {
	UCLAS_IDLE = 0, //����
	UCLAS_DIS_PROBE,//��������̽��׶�
	UCLAS_DISPATCH, //����������׶�
	UCLAS_SERVER_AUTH,//������������֤
	UCLAS_USER_REGISTER,//�û�ע��
	UCLAS_AUTH_REQ,
	UCLAS_AUTH_ANSWER,
	UCLAS_ESTABLISH,
	UCLAS_ERROR,
	UCLAS_SLEEP,
	UCLAS_MAX,
};

enum {
	PHONE_CREATE_NONE = 0,
	PHONE_CREATE_DOING,//������
	PHONE_CREATE_OK,//�����ɹ�
	PHONE_CREATE_FAILED,//����ʧ��
};

//�������û�����flag
//�Ƿ�������ͻ
#define SERVER_USER_FLAG_CONFLICT		(BIT(0))
//�Ƿ����ֻ��û�
#define SERVER_USER_FLAG_PHONE			(BIT(1))
//û�������û�
#define SERVER_USER_FLAG_NOUSER			(BIT(2))


typedef struct {
	u_int8_t data[1500];	// ucph_t *
} la_fragment_item_t;

enum {
	LA_FGST_IDLE,
	LA_FGST_RECEIVE,
	LA_FGST_ERROR,
};


typedef struct {
	u_int8_t err;
	u_int8_t cid;
	u_int8_t idx;
	//la_fragment_item_t items[UCLA_FRAGMENT_MAX_ITEMS];
	u_int8_t item_num;
	la_fragment_item_t *pitems;
	u_int8_t *item_buf;
} la_fragment_stat_t;

//APP ���������

typedef struct {
	u_int8_t flag;
	u_int32_t rule_len;//rule_len
	u_int32_t rule_id;
	u_int32_t last_exec_time;
	u_int32_t home_id;
	u_int8_t enable;// 1��ʾʹ����
	u_int8_t state;// 1��ʾ����ִ��
	char rule[0];
}la_rule_conf_t;

typedef struct {
	struct stlc_list_head link;

	u_int8_t flag;
	u_int32_t rule_len;//rule_len
	u_int32_t rule_id;
	u_int32_t last_exec_time;
	u_int8_t enable;// 1��ʾʹ����
	u_int8_t state;// 1��ʾ����ִ��
	char *rule;///0�������ַ���
}la_rule_t;

//member flag define
#define LA_MEMBER_FLAG_SMARTCONF	(BIT(0))
#define LA_MEMBER_FLAG_NEED_ADD		(BIT(1))

typedef struct {
	u_int64_t sn;
	u_int32_t home_id;
	u_int32_t flag;
	u_int32_t is_la;//֧������
	u_int8_t dev_passwd[APP_USER_UUID_NAME_LEN];//�豸����
}la_member_conf_t;

typedef struct {
	struct stlc_list_head link;

	void *session_ptr;//�Ựָ��
	u_int8_t add_num;//��Ӵ���
	bool del;
	la_member_conf_t conf;
}la_member_t;

#define IS_DEF_HOME_TYPE 	(1)
typedef struct {
	u_int32_t home_id; //�����������ȫ��Ψһid
	u_int32_t flag;
	char home_name[64];	//��ͥ����
	u_int8_t home_passwd[APP_USER_UUID_NAME_LEN];//��ͥ����
	u_int32_t type; //����1��Ĭ�ϲ���ɾ���ȡ�����	
}la_home_conf_t;

typedef struct {
	u_int16_t id;
	u_int16_t sn_num;
	u_int32_t home_id;
	u_int8_t name[LA_LABEL_NAME_MAX_LEN];	
}la_label_conf_t;

typedef struct {
	struct stlc_list_head link;

	u_int8_t flag;
	u_int64_t *p_sn;
	la_label_conf_t conf;
}la_label_t;

// ����һ����ͥ���ֵ�
typedef struct {
	struct stlc_list_head link;

	u_int8_t *key;
	u_int16_t key_len;
	u_int8_t *value;
	u_int16_t value_len;
} la_dict_t;

typedef struct {
	struct stlc_list_head link;
	struct stlc_list_head member_link; //��ͥ��Ա����
	struct stlc_list_head rule_link; //��ͥ������������
	struct stlc_list_head label_link; //��ǩ����
	struct stlc_list_head dict_link;	// �ֵ�����

	void *session_ptr;//�Ựָ��
	cl_handle_t handle;
	u_int16_t url_num;//ģ�����
	u_int8_t **url_array;//ģ��url����

	u_int8_t *share;//������

	u_int16_t share_desc_num;//�ü�ͥ������ʹ�õĸ�������һ���Ǽ�ͥ������
	la_share_desc_t *share_desc_array;

	u_int8_t del_flag;//ɾ�����

	//��ݰ���
	la_sc_key_t la_sc_key[LA_SC_A_NUM]; 

	//�����������̫����Ƭ��ϵ�
	u_int32_t total_length;
	u_int32_t last_total_len_offset;//�ϴ�λ��
	u_int8_t *ptotal_rule;

	bool rules_is_cache;//���Ϊ�棬��ʾ�����ǻ����
	
	u_int32_t last_rule_time;
	u_int32_t last_template_time;
	la_home_conf_t conf;
}la_home_t;

typedef struct {
	u_int32_t user_id; //�����������ȫ��Ψһid
	u_int32_t flag;
	u_int8_t value[4];
	char user_name[APP_USER_UUID_NAME_LEN];	//16�ֽ��û���,�������û�ΪAPP���ɵ�UUID
	char uuid[APP_USER_UUID_NAME_LEN];	//Ӳ��Ψһ��ʾ
	char passwd[APP_USER_UUID_NAME_LEN]; //����md5	
}la_user_conf_t;

typedef struct {
	struct stlc_list_head link; //�û�����
	struct stlc_list_head home_link; //������ͥ����

	void *session_ptr;//�Ựָ��
	cl_handle_t handle;
	la_user_conf_t conf;
}la_user_t;

typedef struct {
	u_int32_t flag;
	char user_name[APP_USER_UUID_NAME_LEN];	//16�ֽ��û���,�������û�ΪAPP���ɵ�UUID
	char passwd[APP_USER_UUID_NAME_LEN]; //����md5		
}la_phone_conf_t;

typedef struct {
	struct stlc_list_head link; //�û�����

	la_phone_conf_t conf;
}la_phone_t;

//�汾�����ļ�
typedef struct {
	struct stlc_list_head link;

	char name[64];
	char vendor[64];
	u_int16_t type;
	u_int16_t sub_type;
	u_int16_t ext_type;
	u_int32_t ver;
}la_dev_ver_limit_t;

//�豸���͵����ݱ���
typedef struct {
	u_int64_t sn;
	u_int8_t ds_type;
	u_int8_t sub_type;
	u_int8_t ext_type;
	bool dev_ver_is_valid;
	bool dev_ver_is_too_low;
}la_dev_misc_conf_t;
typedef struct {
	struct stlc_list_head link;

	la_dev_misc_conf_t conf;
}la_dev_misc_info_t;

//��������
typedef struct {
	struct stlc_list_head link;

	char valid_doname[64];
}la_doname_info_t;

//���û���½ǿ�û�ʱȦ��Ǩ��
typedef struct {
	u_int8_t num;
	u_int32_t userid;
	u_int32_t *homeid;
}la_move_home_t;

typedef struct {
	struct stlc_list_head link;
	u_int64_t sn;
	u_int8_t uuid[MAX_UUID_BIN_LEN];
	u_int8_t back_uuid[MAX_UUID_BIN_LEN];

	//����豸����ʱsn
	u_int64_t adddev_sn;
	//��ʱ�޸������sn
	u_int64_t modify_passwd_sn;
	char doname[64];
	u_int8_t widget_key[WIDGET_KEY_MAX_LEN];
	u_int32_t disp_ip;
	// ��ָָ��
	user_t *user;
	u_int8_t home_name[64];

	//�Ƿ����ӹ����չ������������Ӧ
	bool has_recv_disp;

	//session index
	u_int8_t index;

	u_int32_t reset_num;

	bool is_def_home;
	bool has_user;
	u_int8_t dmap;//����ӳ��

	//�Ƿ��Ѿ���ѯ���������ֻ��˺���
	bool has_phone_queryd;

	//�Ƿ���Ҫȥ��������ѯ�����û�����Ҫ���ֻ��˺ŵ�½ʱ��Ҫÿ��session��ȥ����һ��
	bool need_login_query_user;

	//�Ƿ���Ҫ��ѯ��ӦȦ�ӵ�������Ϣ����ʱ��ѯ��
	bool need_query_home_all_info;


	//�Ƿ���Ҫ���豸�汾��������
	bool need_dev_ver_req;
	//���������Ƭ��ϵ�
	u_int32_t limit_total_length;
	u_int32_t limit_last_total_len_offset;//�ϴ�λ��
	u_int8_t *pver_limit;
	
	//�Ƿ���Ҫ����
	bool need_save_conf;
	//Ϊ�˰�ȫ�ԣ��û��������뵥�����棬����Ȧ����Ϣ����Ա��Ϣ�ȷ�һ��
	bool need_save_username;
	//��Ҫ�������
	bool need_clean_conf;

	//�Ƿ���Ҫ��������dns
	bool need_save_dns;
	
	//��ʾĬ��Ȧ�������session����
	bool has_def_home;
	bool back_has_def_home;
	u_int32_t user_id;

	//��������ͬ�������
	bool need_map_sync;
	u_int64_t map_cal;

	//���ӷ���������ʱ��
	u_int32_t time_start;
	u_int32_t time_diff;

	//�Ƿ���
	bool is_kicked;
	
	u_int32_t home_del_id;
	//�Ƿ���Ҫ��ѯһ���Լ����еļ�ͥ��
	bool need_query_all_home;

	//�Ƿ񴴽��ֻ��˺���
	bool is_phone_creat_status;//�ֻ�����״̬

	//��ʱ����ǿ�û�userid
	u_int32_t phone_user_id_tmp;

	//��Ҫ����ǿ�û�
	bool need_create_force_user;

	//�Ƿ���Ҫ����ǿ�û������ɹ�
	bool need_report_phone_create;

	//����ע�ᣬ������������
	bool need_register;
	u_int32_t a_share_home_id;//�ӱ�ķ��������ķ���ע��ļ�ͥid
	u_int8_t a_sc[8];//������
	u_int8_t a_desc[64];//desc

	u_int8_t *rule_tmp;//��ӹ���ʱ��ʱ�����ã���ӳɹ�����Ҫ

	//��ǩ���
	la_label_t *plabel_tmp;//��ӱ�ǩʱ���ػ�����
	uc_label_bind_t *plbind_tmp;//�󶨱�ǩʱ���ػ�����

	//��ݰ������
	uc_home_shortcut_t *ps_tmp;
	uc_home_shortcut_modify_t sc_mod_tmp;
	
	/*
		UCCS_IDLE, UCS_AUTH, UCCS_ESTABLISH, UCS_AUTH_ERROR
	*/
	int status;
	int last_status;//�ϴ�״̬
	// �ỰID��������֤ʱ��Ϊ0��������/�豸Ӧ��answerʱ��д
	u_int32_t client_sid;
	u_int32_t device_sid;

	u_int32_t my_request_id;
	u_int32_t peer_request_id;
	// ѡ��ļ��ܷ�ʽ
	u_int16_t select_enc;
    //Э��汾
    u_int8_t version;
	
	u_int8_t rand1[4];
	u_int8_t rand2[4];
	u_int32_t r1;//��ʱ�����
	u_int8_t username[APP_USER_UUID_NAME_LEN];
	u_int8_t back_username[APP_USER_UUID_NAME_LEN];
    //��֤ʱ������ʱ��
    u_int16_t  die_for_auth;
	// ָ��time_param�е�һ������ǰʹ�õ�
	uc_time_param_item_t *time_param_cur;
	// �Զ˴�������ʱ��������޸ĳɱ����ֽ�����
	uc_time_param_all_t time_param;
	// �Զ˴�������ʱ�������ԭʼ�������ֽ����
	uc_time_param_all_t time_param_net;

	//�õ����豸ʱ��
	u_int32_t timezone;

	u_int32_t req_time;//��֤��ʼʱ��
	u_int32_t enstable_time;//���ӳɹ�ʱ�䣬��Ҫ�����ж��ٶ��õ�

	// �մ�����0��������������
	int idle_time;
	//adddr
	ipc_addr_t peer_addr;
	ipc_addr_t ipc_addr;
	//lasterr ipv6 addr
	struct sockaddr_in6 last_ipv6_addr;
	// �Զ˵�IP��ַ���˿ںš�������
	u_int32_t ip;
	int port;
	u_int8_t port_index;
	bool server_die;
	// �Լ��󶨵Ķ˿�
	int my_port;
	bool is_ipv6_sock;
	SOCKET sock;
	//���������
	SOCKET disp_sock;
	SOCKET disp_sock6;
	cl_thread_t *t_disp_recv;
	cl_thread_t *t_disp_recv6;
	bool is_ipv6;
	// ���͵����Դ���
	int send_retry;
	// ������֤�����3��û���յ���Ӧ�ͻ�һ�����ض˿�
	int auth_lost;
    
    /*���ͺͽ��յ��ı��ģ������ϣ����ձ��ĺͷ��ͱ��Ĳ�࣬
     �����������ʱ�򣬷��ͻ�ԶԶ���ڽ��գ���ʱ���е���·���������˶˿ڣ�
     ��һ���˿�����
    */
    u_int32_t send_pkts;
    u_int32_t recv_pkts;
	// �û�����
	u_int8_t key[APP_USER_UUID_NAME_LEN];
	u_int8_t back_key[APP_USER_UUID_NAME_LEN];
	//���ݼӽ�����Կ
	u_int8_t aes_key[APP_USER_UUID_NAME_LEN];
	//��֤��Կ
	u_int8_t *auth_key;

	pkt_t *rcv_buf;
    ucph_t* rcv_hdr;
	struct stlc_list_head send_list;
    
	//�������ӷ�������ʱ����
	u_int8_t timeout_count;

	cl_thread_t *t_recv;
	cl_thread_t *t_timer;
	// ���Է��͵Ķ�ʱ��
	cl_thread_t *t_send;
	cl_thread_t *t_keeplive;
	cl_thread_t *t_die;
	//err timer����error״̬�Ÿ���ʱ������ֹ�����������⣬���������һֱ���Ӳ��Ϸ�����
	cl_thread_t *t_server_check;

	//�����¼��յ���α��ĵ�����
	u_int8_t home_event_last_id;
	u_int8_t share_event_last_id;
	u_int8_t linkage_event_last_id;
	u_int8_t linkage_rulelistpush_last_id;
	u_int8_t linkage_dev_ver_limit_last_id;
	u_int8_t linkage_msg_push_last_id;
	u_int8_t last_rid;
	// ��Ƭ����
	la_fragment_stat_t fs;
	//dns cache
	int ipc_count;
	ipc_sock_t ipc_sock[DNS_IP_MAX_NUM];
} ucla_session_t;

//s3ר��
typedef struct {
	struct stlc_list_head link;

	u_int32_t home_id;
	u_int32_t last_modify_time;
	struct stlc_list_head rule_list;
}la_g_rule_t;

//��־���
typedef struct {
	struct stlc_list_head link;

	home_log_rule_info_t log_info;
}la_log_home_rule_node_t;

typedef struct {
	struct stlc_list_head link;

	u_int32_t home_id;
	u_int32_t max_index;

	struct stlc_list_head log_list;
}la_log_home_rule_change_t;

typedef struct {
	struct stlc_list_head link;

	home_log_member_info_t log_info;
}la_log_home_member_node_t;

typedef struct {
	struct stlc_list_head link;

	u_int32_t home_id;

	struct stlc_list_head log_list;
}la_log_home_member_change_t;

typedef struct {
	bool init;
	cl_thread_t *t_timer;//��ʱ��ʱ��
	cl_thread_t *t_comm;//��Щ���߰���Ķ�ʱ��
	cl_thread_t *t_def_home_timer;//Ĭ��Ȧ�Ӵ�����ʱ��
	cl_thread_t *t_modify_passwd;//tcp�Ȳ�֧���������豸�޸����붨ʱ��
	cl_thread_t *t_comm_proc;//��������һ����ʱ�����Ĵ���
	cl_thread_t *t_dev_add_proc;//��������һ����ʱ�����Ĵ���

	//����ռ��ӳ��
	char **doname_map;

	//�ֻ��û�����
	struct stlc_list_head la_phone;
	char cur_phone_name[16];

	//��û��Ȧ��ʱ������豸�������
	struct stlc_list_head dev_wait_list;
	bool need_dev_wait_save;

	bool need_dev_pass_sync_save;

	//�Ƿ���Ҫ������ȷ����
	bool need_back_passwd_save;

	//���ñ���aes
	u_int8_t conf_aes_key[16];

	//session index ����
	u_int8_t index;
	//����ӳ��
	u_int8_t index_map[20];
	//�Ƿ����ֻ��˺�
	bool has_phone;
	//�Ƿ���Ҫ�������
	bool need_clean_all_conf;

	//�Ƿ���������ѯ��
	bool is_rule_all_query;

	//�Ƿ���Ҫ�ж��豸�汾��������
	bool need_cal_dev_ver;

	//�Ƿ���Ҫ�����ֻ��˺�
	bool need_save_phone;

	//�Ƿ��Ǻ�̨ģʽ
	bool run_in_background;

	//�ֻ��˺Ŵ���״̬
	u_int8_t phone_status;
	cl_thread_t *t_phone_timer;//�ֻ��˺Ŵ���ʱ��
	cl_thread_t *t_phone_die;
	u_int32_t phone_timer;
	u_int32_t phone_die;
	u_int32_t phone_user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];	//16�ֽ��û���,�������û�ΪAPP���ɵ�UUID
	u_int8_t passwd[APP_USER_UUID_NAME_LEN]; //����md5
	u_int8_t uuid[MAX_UUID_BIN_LEN];
	u_int8_t back_user_name[APP_USER_UUID_NAME_LEN];

	//��Ҫ��ʱ����ǿ�û�����Ϊ�п��ܵ�ǰsession������
	bool need_phone_create_delay;

	bool phone_logining;

	//�Ƿ���Ҫ����������������Ϊ������oem������Ϊ��������ʾ���뱣������
	bool need_doname_save;

	//Ϊ���ֻ��˺���ʱ�������紴��ʱ���粻ͨ����һ���ֻ���½����Դ���
	u_int8_t p_user_name[APP_USER_UUID_NAME_LEN];	//16�ֽ��û���,�������û�ΪAPP���ɵ�UUID
	u_int8_t p_passwd[APP_USER_UUID_NAME_LEN]; //����md5
	u_int8_t p_passwd_cmp[APP_USER_UUID_NAME_LEN]; //����md5
	
	bool need_phone_upload;//��Ҫ�ϴ�����������

	bool phone_passwd_err;//�ֻ��˺ŵ�½�������
	bool phone_acount_not_exist;//�ֻ��˺Ų�����
	bool acount_not_exist;
		
	u_int32_t cur_home_id;//��ǰhomeid
	bool has_def_home;//�Ƿ���Ĭ��Ȧ��
	bool has_any_home;//�Ƿ����κ�Ȧ��
	bool need_enstablish;//���û��Ȧ�ӣ�����Ҫ����һ��session���ӷ�����

	//���ڸĳɷ�����ͬ�������ˣ�Ϊ�˼��ݴ������
	void *need_switch_session;

	//�Ƿ���Ҫ�ֻ���½
	bool need_phone_login_delay;

	//����豸֧�����������Ƿ�仯�ˣ�������������������û��ʱ��ȡ��
	bool need_la_sync_check;

	//��Ӣ����������
	u_int8_t lang;

	//�豸�����ļ��ϴ��޸�ʱ��
	u_int32_t cap_lastmodifytime;
	u_int16_t cap_num;//�����ļ�����
	u_int8_t **cap_array;//�����ļ�url����

	//�Զ����豸�����ļ��ϴ��޸�ʱ��
	u_int32_t cap_custom_lastmodifytime;
	u_int16_t cap_custom_num;//�����ļ�����
	u_int8_t **cap_custom_array;//�����ļ�url����

	//ģ����Ϣ
	u_int32_t last_template_time;//�ϴ�ģ���޸�ʱ��
	u_int16_t url_num;//ģ�����
	u_int8_t **url_array;//ģ��url����
	void *template_session;//ȡ������ģ���session
	
	//��������Ϣ
	cl_la_notify_msg_t msg;

	//�������Ƿ�֧�ֱ�ǩ����
	bool support_tabel;
	//�������Ƿ�֧�ֿ�ݰ���
	bool support_shortcut;
	//�������Ƿ�֧����ת
	bool support_trans;

	///0������pad����������ַ���
	char *req_share;

	u_int32_t disp_bit;

	//app����ʱ��
	u_int32_t init_time;

	//rsa
	bool use_rsa;
	int rsa_priv_len;
	u_int8_t *rsa_priv;
	cl_rsa_callback rsa_enc;
	cl_rsa_callback rsa_dec;

	//�豸��������С�汾�����ļ�����
	u_int8_t last_ver[4];//�����ļ��ϴε���ʱ��
	struct stlc_list_head dev_ver_limit_list;//�����ļ�����

	struct stlc_list_head dev_pass_sync_list;//����ͬ������

	//�豸��������
	struct stlc_list_head dev_type_list;

	//s3ר�ã���������
	struct stlc_list_head g_rule_list;
	//��־���
	struct stlc_list_head g_rule_log_list;
	struct stlc_list_head g_member_log_list;

	//���úͲ����÷�����̽����������������
	struct stlc_list_head valid_doname;
	struct stlc_list_head invalid_doname;

	//Ǩ��Ȧ��
	la_move_home_t move_home;

	//һ����Ȧ�ӣ�������ʾ��ǰû��Ȧ��ʱ��app�õģ�������handle���Բ�ѯ��
	la_home_t *p_null_home;
	
//	RSA *rsa_private;
	cl_callback_t callback;
	void *callback_handle;
	struct stlc_list_head server_client;//�������Ự
}la_ctrl_t;


/* External function declarations. */


/* Macro API definitions. */
#define UCLA_ENTER() log_debug("enter %s %d\n", __FUNCTION__, __LINE__)
#define UCLA_EXIT() log_debug("exit %s %d\n", __FUNCTION__, __LINE__)

/* Global variable declarations. */
extern ucc_proc_t ucla_proc[UCLAS_MAX];
extern la_ctrl_t *plc;

void linkage_init();
void linkage_exit();
void ucla_set_callback(cl_callback_t callback, void *callback_handle);
la_phone_t *ucla_phone_new();
la_user_t *ucla_user_new();
la_home_t *ucla_home_new();
la_member_t *ucla_member_new();
la_label_t *ucla_label_new();
la_rule_t *ucla_rule_new();
la_log_home_rule_change_t *ucla_log_rule_new();
la_log_home_rule_node_t *ucla_log_rule_node_new();
la_log_home_rule_change_t *ucla_log_rule_find_by_homeid(u_int32_t home_id);
la_log_home_member_change_t *ucla_log_mem_new();
la_log_home_member_node_t *ucla_log_mem_node_new();
la_log_home_member_change_t *ucla_log_mem_find_by_homeid(u_int32_t home_id);
la_phone_t *ucla_find_phone_by_name(char *name);
la_user_t *ucla_find_user_by_id(u_int32_t user_id);
la_home_t *ucla_find_home_by_id(u_int32_t home_id);
la_dict_t *ucla_find_home_dict_by_key(la_home_t *phome, u_int8_t *key, u_int16_t key_len);
la_dict_t *ucla_dict_add_or_modify(la_home_t *phome, u_int8_t *key, u_int16_t key_len, u_int8_t *value, u_int16_t value_len);
void ucla_dict_del(la_home_t *phome, u_int8_t *key, u_int16_t key_len);
la_dict_t *ucla_dict_add(la_home_t *phome, u_int8_t *key, u_int16_t key_len, u_int8_t *value, u_int16_t value_len);
la_rule_t *ucla_find_rule_by_id(la_home_t *phome, u_int32_t rule_id);
ucla_session_t *la_get_session_by_handle(cl_handle_t handle);
bool la_is_valid();
RS ucla_request_add(ucla_session_t *s, pkt_t *pkt);
pkt_t *ucla_pkt_new(int cmd, int param_len,bool is_request, bool is_enc, u_int8_t flags, u_int32_t client_sid, u_int32_t device_sid, u_int32_t request_id);
void ucla_home_del(ucla_session_t *s, u_int32_t home_id);
void ucla_user_del(ucla_session_t *s, u_int32_t user_id);
void ucla_phone_del(la_phone_t *phone);
bool ucla_home_free_memlist(la_home_t *phome);
void ucla_user_conf_save(ucla_session_t *s);
bool ucla_mem_del(la_home_t *phome, u_int64_t sn);
void ucla_home_rule_free(la_home_t *phome);
void ucla_template_free(la_home_t *phome);
int ucla_get_all_home_num();
la_home_t *ucla_find_home_by_handle(cl_handle_t handle);
void la_user_login(u_int32_t home_id, u_int64_t sn, u_int8_t *passwd_md5);
ucla_session_t*  ucla_new(char *user_dir);
ucla_session_t *ucla_get_ses_by_doname(char *doname);
void ucla_server_doname_save();
ucla_session_t *ucla_get_dehome_session();
u_int32_t ucla_get_dehome_id();
int ucla_send_pkt_raw(SOCKET sock, bool ipv6, ipc_addr_t *ipc_addr, u_int16_t port, pkt_t *pkt);
RS ucla_enc_pkt(ucla_session_t *s, pkt_t *pkt);
void ucla_conf_clean(ucla_session_t *s);
u_int8_t ucla_home_is_online(u_int32_t handle);
bool ucla_session_is_ok(ucla_session_t *s);
ucla_session_t *ucla_get_session_by_homeid(u_int32_t home_id);
void la_comm_timer_reset();
bool user_is_support_la(user_t *user);
void la_comm_timer_delay();
void ucla_session_info_dump();
void ucla_rule_free(la_rule_t *prule);
void ucla_label_free(la_label_t *pl);
bool ucla_home_label_free(la_home_t *phome);
la_label_t *ucla_find_home_label_by_id(la_home_t *phome, u_int16_t id);
void ucla_label_flag_set(la_home_t *phome, u_int8_t flag);
la_member_t *ucla_find_member_by_sn(ucla_session_t *s, u_int64_t sn);
void la_modify_passwd_timer_reset();
bool ucla_is_any_session_online();
void ucla_each_home_query(ucla_session_t *s);
void ucla_each_share_query(ucla_session_t *s);
void ucla_each_label_query(ucla_session_t *s);
void ucla_session_reset(ucla_session_t *s);
void ucla_user_add(ucla_session_t *s, la_user_t *puser);
void ucla_phone_add(la_phone_t *phone);
void ucla_phone_flag_clean();
void ucla_phone_set_status(int status);
ucla_session_t *ucla_get_any_enstablis_session();
void phome_passwd_modify_ok();
void ucla_phone_save();
ucla_session_t *ucla_get_ses_by_doname_only(char *doname);
bool la_dmap_replace_doname(u_int8_t id, char *doname);
void ucla_server_user_query_clean();
void ucla_all_home_query_set(ucla_session_t *s);
la_home_t *ucla_get_def_home();
void ucla_set_def_home(ucla_session_t *s, la_home_t *phome);
bool ucla_is_def_home(la_home_t *phome);
void do_ucla_def_home_sync(ucla_session_t *s);
void ucla_member_add(la_member_t *pmem, la_home_t *phome);
void do_dev_ver_file_sync(ucla_session_t *s);
la_dev_misc_info_t *la_dev_type_find_by_sn(u_int64_t sn);
void ucla_create_def_home(ucla_session_t *s);
void ucla_all_sleep_session_reset();
void ucla_home_del2(ucla_session_t *s, la_home_t *phome);
void la_doname_sync(user_t* puser);
void la_misc_capfile_free();
void la_misc_custom_capfile_free();
void la_misc_template_free();
void la_move_home_clean();
void ucla_weak_user_save();
la_member_t *la_dev_wait_find_by_sn(u_int64_t sn);
void la_dev_wait_del(la_member_t *pmem);
void la_comm_timer_proc_reset();
la_member_t *ucla_find_member_by_sn_from_home(la_home_t *phome, u_int64_t sn);
int ucla_get_user_num(u_int64_t sn);
la_member_t *la_dev_pass_sync_find_by_sn(u_int64_t sn);
void ucla_each_home_shortcut_query(ucla_session_t *s);
void g_rule_free(la_g_rule_t *pr);
void la_g_rule_node_list_free(la_log_home_rule_change_t *pr);
la_g_rule_t *ucla_g_rule_find_by_homeid(u_int32_t home_id);
la_g_rule_t *ucla_g_rule_new();
void g_rule_list_free(la_g_rule_t *pr);
void ucla_set_cur_phone(la_phone_t *phone);
bool ucla_is_cur_phone(la_phone_t *phone);
la_phone_t *ucla_find_cur_phone();
bool ucla_is_phone_user(la_user_t *puser);
bool ucla_is_phone_session(ucla_session_t *s);
void la_g_member_node_list_free(la_log_home_member_change_t *pm);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LINKAGE_CLIENT_H */

