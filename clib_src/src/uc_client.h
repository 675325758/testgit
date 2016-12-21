#ifndef	__UC_CLEINT_H__
#define	__UC_CLEINT_H__

#include "udp_ctrl.h"

//#define	UCC_TEST	1

/*****************************************
	״̬������
 *****************************************/

typedef struct {
    u_int16_t obj;
    u_int16_t sub_obj;
    u_int16_t attr;
}uc_sys_support_obj_t;

#define MAX_SEND_RETRY_NUM  (50)
#define MAX_AUTH_LOST (3)
#define SUPER_PHONE_INDEX (0)

typedef struct _ucc_session_s{
	u_int64_t sn;
	u_int8_t uuid[MAX_UUID_BIN_LEN];

	char *name;
	// ��ָָ��
	user_t *user;
	
	/*
		UCCS_IDLE, UCS_AUTH, UCCS_ESTABLISH, UCS_AUTH_ERROR
	*/
	int status;
	// �ỰID��������֤ʱ��Ϊ0��������/�豸Ӧ��answerʱ��д
	u_int32_t client_sid;
	u_int32_t device_sid;

	u_int32_t my_request_id;
	u_int32_t peer_request_id;
	// ѡ��ļ��ܷ�ʽ
	u_int16_t select_enc;
    //Э��汾
    u_int8_t version;
    u_int8_t has_share_key; //�Ƿ��Ѿ߱�����key�����Դ��豸��Ҳ���Դӱ��ض�ȡ
    u_int8_t v1_remain_days;
    u_int8_t share_key[V2_SHARE_KEY_LEN];
    u_int32_t phone_index;//�������ͥ�û�ʱ���ֻ����к�
    u_int8_t is_qr_login;
	u_int64_t qr_code64;
	u_int32_t v2_client_sid;
	u_int32_t v2_device_sid;
	u_int8_t v2_login_num;

	//��¼�ϴ�rid
	u_int8_t last_rid;
	//xyʹ��,��Ϊ�¿������ݱ仯̫�죬�����ĳЩ״ֵ̬����app��ʾ�쳣
	u_int8_t drop_update_num;
	u_int8_t droped_num;

	u_int32_t up_event_recv_last_time;	

	u_int8_t rand1[4];
	u_int8_t rand2[4];

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

	//�Է��Ƿ���apģʽ
	u_int8_t wifi_mode;

	// �մ�����0��������������
	int idle_time;
	// �Զ˵�IP��ַ���˿ںš�������
	u_int32_t ip;
	int port;
	// �Լ��󶨵Ķ˿�
	int my_port;
	SOCKET sock;
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
	// �ӽ�����Կ
	u_int8_t key[16];

	pkt_t *rcv_buf;
    ucph_t* rcv_hdr;
	struct stlc_list_head send_list;

	//debug��
	bool debug_on;
	bool need_debug_on;

	cl_thread_t *t_recv;
	cl_thread_t *t_timer;
	// ���Է��͵Ķ�ʱ��
	cl_thread_t *t_send;
	cl_thread_t *t_keeplive;
	cl_thread_t *t_die;

	//�����豸ӳ���Ƭ������0��ʾû������
	u_int16_t up_total;
	pkt_t *up_first;
	u_int16_t up_current;
} ucc_session_t;


// ��������,����udp_ctrl.h����ƽ̨�����ã��ͷ�������
// ʹ��ʱ��objͷ�������Ѿ�ת�������ֽ���
static inline bool is_except_attr(ucp_obj_t* obj, u_int16_t want_obj,u_int16_t want_sub,u_int16_t want_attr)
{
    //��ƥ��obj
    if (!want_obj) {
        return true;
    }
    
    if (!want_sub) {
        return (obj->objct == want_obj);
    }
    
    if (!want_attr) {
        return (obj->objct == want_obj) && (obj->sub_objct == want_sub);
    }
    
    return (obj->objct == want_obj) && (obj->sub_objct == want_sub) && (obj->attr == want_attr);
}

static inline bool is_valid_obj_data(ucp_obj_t* obj,int want_len)
{
    if (obj->param_len != want_len) {
        return false;
    }
    return true;
}

//���obj�Ƿ�Ϸ����ͱ���ʣ�೤�ȱȽ�
static inline bool is_valid_obj(ucp_obj_t* obj,u_int32_t remain_len)
{
    if (obj->param_len + sizeof(*obj) <= remain_len) {
        return true;
    }
    
    return false;
}

static inline bool is_obj_less_than_len(ucp_obj_t* obj,u_int32_t min_len)
{
    if (obj->param_len < min_len) {
        return true;
    }
    
    return false;
}

//�ַ���
static inline const char* obj_string_value(ucp_obj_t* obj)
{
    char* p;
    
    if (obj->param_len > 0) {
        p = (char*)(obj+1);
        p[obj->param_len -1] = '\0';
        return (const char*)p;
    }
    
    return NULL;
}

static inline char* obj_string_dup(ucp_obj_t* obj)
{
    char* p = (char*)obj_string_value(obj);
    
    return (p != NULL) ? cl_strdup(p):p;
}

// �ɵ�����ת�ֽ���
static inline u_int8_t obj_u8_value(ucp_obj_t* obj)
{
    return *((u_int8_t*)(obj+1));
}

// �ɵ�����ת�ֽ���
static inline u_int16_t obj_u16_value(ucp_obj_t* obj)
{
    return *((u_int16_t*)(obj+1));
}

// �ɵ�����ת�ֽ���
static inline u_int32_t obj_u32_value(ucp_obj_t* obj)
{
    return *((u_int32_t*)(obj+1));
}

//����ͷ���ֽ���
static inline void ucp_obj_order(ucp_obj_t* obj)
{
    obj->sub_objct = ntohs(obj->sub_objct);
    obj->param_len = ntohs(obj->param_len);
    obj->objct = ntohs(obj->objct);
    obj->attr = ntohs(obj->attr);
}

static inline void fill_net_ucp_obj(ucp_obj_t* uo,u_int16_t obj,u_int16_t sub_obj,u_int16_t attr,u_int16_t param_len)
{
    uo->objct = htons(obj);
    uo->sub_objct = htons(sub_obj);
    uo->attr = htons(attr);
    uo->param_len = htons(param_len);
}

#define OBJ_VALUE(obj,type) (type)(obj+1)

extern void ucc_port_init();
RS ucc_udp_socket(ucc_session_t *s);
extern RS ucc_new(user_t *user);
extern void ucc_free(user_t *user);
extern void ucc_set_status(ucc_session_t *s, int status);
extern RS ucc_request_add(ucc_session_t *s, pkt_t *pkt);
extern RS ucc_response_send(ucc_session_t *s, pkt_t *pkt);

extern void ucc_free_port(int port);
extern int ucc_get_port(u_int64_t sn, const char *name);

#endif

