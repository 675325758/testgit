#ifndef	__UAS_CLEINT_H__
#define	__UAS_CLEINT_H__

#include "udp_ctrl.h"

//#define	UCC_TEST	1

// UASC == UDP APPLICATION SERVER CLIENT
#define	UASC_MAX_FRAG_PET_PKT	256

enum {
	UASCS_IDLE = 0, //����
	UASCS_DISPATCH = 1, //����������׶�
	UASCS_AUTH_REQ = 2,
	UASCS_AUTH_ANSWER = 3,
	UASCS_ESTABLISH = 4,
	UASCS_ERROR = 5,
	UASCS_MAX = 6
};

typedef struct {
    u_int8_t errorcode;
    u_int8_t reserved[3];
} uasc_error_t;

enum{
    UASCP_ERROR_DATASMALL=0x01,
    UASCP_ERROR_MAX,
};

//APP ���������
typedef struct {
	u_int16_t apptype;
	u_int8_t pad[2];
	u_int32_t  time_xor;	//ʱ����DISP_APPSRV_XOR����ֵ
	uc_tlv_t tlv[0];
}uasc_req_disp_hdr_t;

typedef struct {
	u_int32_t errorno;
	u_int32_t ip;
	u_int16_t port;
	u_int8_t pad[2];
	uc_tlv_t tlv[0];
}uasc_disp_res_hdr_t;

typedef struct {
	u_int8_t uuid[MAX_UUID_BIN_LEN];

	char name[32];
	/*
		UCCS_IDLE, UCS_AUTH, UCCS_ESTABLISH, UCS_AUTH_ERROR
	*/
	int status;
	// �ỰID��������֤ʱ��Ϊ0��������/�豸Ӧ��answerʱ��д
	u_int32_t client_sid;
	u_int32_t device_sid;

	u_int8_t my_request_id;
	u_int8_t peer_request_id;
	// ѡ��ļ��ܷ�ʽ
	u_int16_t select_enc;
	u_int8_t md5_passwd[16];

	u_int8_t rand1[4];
	u_int8_t rand2[4];

	u_int8_t last_keep_id;

	// ָ��time_param�е�һ������ǰʹ�õ�
	uc_time_param_item_t *time_param_cur;
	// �Զ˴�������ʱ��������޸ĳɱ����ֽ�����
	uc_time_param_all_t time_param;
	// �Զ˴�������ʱ�������ԭʼ�������ֽ����
	uc_time_param_all_t time_param_net;

	// �մ�����0��������������
	int idle_time;
	// �Զ˵�IP��ַ���˿ںš�������
	u_int32_t ip;
	int port;
	//���������
	SOCKET disp_sock;
	cl_thread_t *t_disp_recv;
	// �Լ��󶨵Ķ˿�
	int my_port;
	SOCKET sock;
	// ���͵����Դ���
	int send_retry;
	// ������֤�����3��û���յ���Ӧ�ͻ�һ�����ض˿�
	int auth_lost;

	// �ӽ�����Կ
	u_int8_t key[16];

	pkt_t *rcv_buf;
	struct stlc_list_head send_list;
    //�ȴ����͵ı���
    struct stlc_list_head send_wait_list;
    
    struct stlc_list_head defrag_list;
    u_int32_t rand_num;

	cl_thread_t *t_recv;
	cl_thread_t *t_timer;
	// ���Է��͵Ķ�ʱ��
	cl_thread_t *t_send;
	cl_thread_t *t_keeplive;
	cl_thread_t *t_die;

	u_int8_t try_count;
} uasc_session_t;

typedef struct {
    struct stlc_list_head link;
    u_int32_t frag_ident; //��Ƭ��־
    u_int16_t total;//�ܹ�����Ƭ
    u_int16_t frag_recved; //�յ�����Ƭ
    u_int32_t total_len;
    pkt_t *pkts[UASC_MAX_FRAG_PET_PKT];
    pkt_t* whole_pkt; //�����Ƭ�ı���
    u_int8_t is_defrag_successed; // �����ˣ������ʧ��(�����ڴ棬ͷ����������Щ)
}uasc_defrag_cache_t;

typedef struct {
    u_int8_t errorcode;
    u_int8_t reserved[3];
}uascp_report_result_t;


extern uasc_session_t* uasc_new();
extern void uasc_free(uasc_session_t *s);
extern void uasc_set_status(uasc_session_t *s, int status);
extern RS uasc_request_add(uasc_session_t *s, pkt_t *pkt);
extern RS uasc_response_send(uasc_session_t *s, pkt_t *pkt);
extern pkt_t *uasc_pkt_new(int cmd, int param_len, 
			bool is_request, bool is_enc, u_int8_t flags,
			u_int32_t client_sid, u_int32_t device_sid, u_int8_t request_id);

extern RS uasc_try_connect_to_server(uasc_session_t *s);

extern RS uasc_remove_wait_pkt_by_ident(uasc_session_t *s, u_int64_t ident);
extern RS uasc_wait_request_add(uasc_session_t *s, pkt_t *pkt,u_int64_t sn);

extern pkt_t* uasc_mk_ctrl_pkt(u_int64_t sn, u_int8_t action, u_int8_t obj_count, void* content, int content_len);

extern RS uasc_request_add_by_data(uasc_session_t *s,int cmd, void* param, int param_len,
                    bool is_request, bool is_enc, u_int8_t flags,
                                   u_int32_t client_sid);
extern RS uasc_ctrl_obj_value(u_int64_t sn, u_int8_t action, u_int8_t obj_count, void* content, int content_len);

#endif

