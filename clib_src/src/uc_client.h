#ifndef	__UC_CLEINT_H__
#define	__UC_CLEINT_H__

#include "udp_ctrl.h"

//#define	UCC_TEST	1

/*****************************************
	状态机定义
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
	// 回指指针
	user_t *user;
	
	/*
		UCCS_IDLE, UCS_AUTH, UCCS_ESTABLISH, UCS_AUTH_ERROR
	*/
	int status;
	// 会话ID，请求认证时候为0，服务器/设备应答answer时填写
	u_int32_t client_sid;
	u_int32_t device_sid;

	u_int32_t my_request_id;
	u_int32_t peer_request_id;
	// 选择的加密方式
	u_int16_t select_enc;
    //协议版本
    u_int8_t version;
    u_int8_t has_share_key; //是否已具备共享key，可以从设备，也可以从本地读取
    u_int8_t v1_remain_days;
    u_int8_t share_key[V2_SHARE_KEY_LEN];
    u_int32_t phone_index;//分享给家庭用户时的手机序列号
    u_int8_t is_qr_login;
	u_int64_t qr_code64;
	u_int32_t v2_client_sid;
	u_int32_t v2_device_sid;
	u_int8_t v2_login_num;

	//记录上次rid
	u_int8_t last_rid;
	//xy使用,因为温控器数据变化太快，会更新某些状态值导致app显示异常
	u_int8_t drop_update_num;
	u_int8_t droped_num;

	u_int32_t up_event_recv_last_time;	

	u_int8_t rand1[4];
	u_int8_t rand2[4];

    //认证时的死亡时间
    u_int16_t  die_for_auth;
	// 指向time_param中的一个，当前使用的
	uc_time_param_item_t *time_param_cur;
	// 对端传过来的时间参数，修改成本地字节序了
	uc_time_param_all_t time_param;
	// 对端传过来的时间参数，原始的网络字节序的
	uc_time_param_all_t time_param_net;

	//得到的设备时区
	u_int32_t timezone;

	//对方是否是ap模式
	u_int8_t wifi_mode;

	// 刚创建是0，后面变成正常的
	int idle_time;
	// 对端的IP地址、端口号。主机序
	u_int32_t ip;
	int port;
	// 自己绑定的端口
	int my_port;
	SOCKET sock;
	// 发送的重试次数
	int send_retry;
	// 发送认证请求后，3次没有收到响应就换一个本地端口
	int auth_lost;
    
    /*发送和接收到的报文，理论上，接收报文和发送报文差不多，
     网络条件差的时候，发送会远远大于接收，此时，有的是路由器限制了端口，
     换一个端口试试
    */
    u_int32_t send_pkts;
    u_int32_t recv_pkts;
	// 加解密秘钥
	u_int8_t key[16];

	pkt_t *rcv_buf;
    ucph_t* rcv_hdr;
	struct stlc_list_head send_list;

	//debug用
	bool debug_on;
	bool need_debug_on;

	cl_thread_t *t_recv;
	cl_thread_t *t_timer;
	// 重试发送的定时器
	cl_thread_t *t_send;
	cl_thread_t *t_keeplive;
	cl_thread_t *t_die;

	//升级设备映像分片个数，0表示没有升级
	u_int16_t up_total;
	pkt_t *up_first;
	u_int16_t up_current;
} ucc_session_t;


// 对象处理函数,由于udp_ctrl.h各个平台都在用，就放这里了
// 使用时，obj头部必须已经转到本地字节序
static inline bool is_except_attr(ucp_obj_t* obj, u_int16_t want_obj,u_int16_t want_sub,u_int16_t want_attr)
{
    //不匹配obj
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

//检查obj是否合法，和报文剩余长度比较
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

//字符串
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

// 由调用者转字节序
static inline u_int8_t obj_u8_value(ucp_obj_t* obj)
{
    return *((u_int8_t*)(obj+1));
}

// 由调用者转字节序
static inline u_int16_t obj_u16_value(ucp_obj_t* obj)
{
    return *((u_int16_t*)(obj+1));
}

// 由调用者转字节序
static inline u_int32_t obj_u32_value(ucp_obj_t* obj)
{
    return *((u_int32_t*)(obj+1));
}

//对象头部字节序
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

