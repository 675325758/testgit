#ifndef	__UAS_CLEINT_H__
#define	__UAS_CLEINT_H__

#include "udp_ctrl.h"

//#define	UCC_TEST	1

// UASC == UDP APPLICATION SERVER CLIENT
#define	UASC_MAX_FRAG_PET_PKT	256

enum {
	UASCS_IDLE = 0, //空闲
	UASCS_DISPATCH = 1, //分配服务器阶段
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

//APP 分配服务器
typedef struct {
	u_int16_t apptype;
	u_int8_t pad[2];
	u_int32_t  time_xor;	//时间与DISP_APPSRV_XOR异或的值
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
	// 会话ID，请求认证时候为0，服务器/设备应答answer时填写
	u_int32_t client_sid;
	u_int32_t device_sid;

	u_int8_t my_request_id;
	u_int8_t peer_request_id;
	// 选择的加密方式
	u_int16_t select_enc;
	u_int8_t md5_passwd[16];

	u_int8_t rand1[4];
	u_int8_t rand2[4];

	u_int8_t last_keep_id;

	// 指向time_param中的一个，当前使用的
	uc_time_param_item_t *time_param_cur;
	// 对端传过来的时间参数，修改成本地字节序了
	uc_time_param_all_t time_param;
	// 对端传过来的时间参数，原始的网络字节序的
	uc_time_param_all_t time_param_net;

	// 刚创建是0，后面变成正常的
	int idle_time;
	// 对端的IP地址、端口号。主机序
	u_int32_t ip;
	int port;
	//分配服务器
	SOCKET disp_sock;
	cl_thread_t *t_disp_recv;
	// 自己绑定的端口
	int my_port;
	SOCKET sock;
	// 发送的重试次数
	int send_retry;
	// 发送认证请求后，3次没有收到响应就换一个本地端口
	int auth_lost;

	// 加解密秘钥
	u_int8_t key[16];

	pkt_t *rcv_buf;
	struct stlc_list_head send_list;
    //等待发送的报文
    struct stlc_list_head send_wait_list;
    
    struct stlc_list_head defrag_list;
    u_int32_t rand_num;

	cl_thread_t *t_recv;
	cl_thread_t *t_timer;
	// 重试发送的定时器
	cl_thread_t *t_send;
	cl_thread_t *t_keeplive;
	cl_thread_t *t_die;

	u_int8_t try_count;
} uasc_session_t;

typedef struct {
    struct stlc_list_head link;
    u_int32_t frag_ident; //分片标志
    u_int16_t total;//总共多少片
    u_int16_t frag_recved; //收到多少片
    u_int32_t total_len;
    pkt_t *pkts[UASC_MAX_FRAG_PET_PKT];
    pkt_t* whole_pkt; //集齐分片的报文
    u_int8_t is_defrag_successed; // 收齐了，但组包失败(比如内存，头参数不对这些)
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

