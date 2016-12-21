#ifndef	__CL_PRIV_H__
#define	__CL_PRIV_H__


//#define	DEBUG_PIC_DATA	1

#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif 

// ��ǰ����һЩ���ݽṹ������ͷ�ļ���Ҫ�õ�
struct user_s;
typedef struct _user_s user_t;

struct _slave_s;
typedef struct _slave_s slave_t;

struct _pkt_t;
typedef struct _pkt_t pkt_t;

struct _video_s;
typedef struct _video_s video_t;

struct _plug_s;
typedef struct _plug_s plug_t;

struct _equipment_ctrl_s;
typedef struct _equipment_ctrl_s equipment_ctrl_t;
	
struct _slave_cmt_s;
typedef struct _slave_cmt_s slave_cmt_t;
struct _user_cmt_s;
typedef struct _user_cmt_s user_cmt_t;

struct _ia_ctrl;
typedef struct _ia_ctrl ia_t;
    
struct _notify_push_s;
typedef struct _notify_push_s notify_push_t;

struct _ir_lib_s;
typedef struct _ir_lib_s ir_lib_t;

struct remote_key_learn_s;
typedef struct remote_key_learn_s remote_key_learn_t;

struct _alarm_phone_assist_s;
typedef struct _alarm_phone_assist_s alarm_phone_assist_t;

struct _area_ctrl_s;
typedef struct _area_ctrl_s area_ctrl_t;

struct _scent_ctrl_s;
typedef struct _scent_ctrl_s scene_ctrl_t;

struct _lan_dev_probe_ctrl_s;
typedef struct _lan_dev_probe_ctrl_s lan_dev_probe_ctrl_t;

struct _rfdev_status_s;
typedef struct _rfdev_status_s rfdev_status_t;

struct _cl_rfdev_status_s;
typedef struct _cl_rfdev_status_s cl_rfdev_status_t;
    
#include <stdio.h>
#include <stdlib.h>
#include "cl_sys.h"
#include "stlc_list.h"
#include "cl_log.h"
#include "cl_thread.h"
#include "ds_proto.h"
#include "cl_mem.h"
#include "lookup.h"
#include "cl_user_priv.h"
#include "net_detect.h"
#include "cl_video.h"
#ifndef	NO_COMMUNITY
#include "ds_key.h"
#endif
#include "cl_ia.h"
#include "cl_intelligent_forward.h"
#include "cl_rfgw.h"

#pragma warning(disable: 4996)
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

#define	VERSION_LIB	0
#define	VERSION_MAJOR	8
#define	VERSION_MINOR	3

//ȫ��ͨ�汾���ƺ�
#define MUT_SERVER_ADAPT

// ֧��UDP����
#define UC_AGENT

// ��Ҫ���ú��֣�����׿��ʾ����
#define	CL_DESC	"Galaxywind Corporation iHome SDK 2014/7/15"


#define	MAX_TCP_PKT	(64*1024)
#define	MAX_UDP_PKT	8192

#define	CL_MAX_PATH	256

#define	IS_INVALID_SOCK(sock)	((sock) == INVALID_SOCKET || (sock) == 0)

#define	IP_LOOPBACK	"127.0.0.1"
  
#ifndef MUT_SERVER_ADAPT
    
#ifdef AWS_SERVER //����ѷ������
#define	DFL_DIS_SERVER	"www.iiwifi.com"
#define	DFL_CGI_SERVER "www.iiwifi.com"
#else
#define	DFL_DIS_SERVER	"www.jiazhang008.com"
#define	DFL_CGI_SERVER "www.jiazhang007.com"
#endif
    
#else
    
#define	DFL_DIS_SERVER	"www.jiazhang008.com"
#define	DFL_CGI_SERVER "www.jiazhang007.com"
    
#endif

#define	DEFAULT_DOMAIN DFL_CGI_SERVER":880"
//#define	DFL_DIS_SERVER	"115.28.222.148"
#define	MAX_DISP_SERVER_IP	64
#define	DISP_SERVER_PORT	1180
#define	DISP_SERVER_PORT1	31578
#define	DISP_SERVER_PORT2	51180
//��ʱ�� 1��
#define TIME_PER_SECOND (1000)
//��ʱ�� 1����
#define TIME_PER_MSECOND (1)

#define TIME_N_SECOND(n) ((n)*(TIME_PER_SECOND))
    
#define TIME_N_MSECOND(n) (n)

#define TIME_N_MINUTE(n) (60*TIME_N_SECOND((n)))

#define	TIME_PU_MSG TIME_N_MINUTE(5)
#define	TIME_LOGIN_TIMEOUT	TIME_N_SECOND(30)

#define	TIME_IDLE	1000
// ��ʱʱ�䡢��ʱ��֮��ģ���λ����
#define	TIME_TRY_DISP	500 /* ������²�ѯһ�η�������� */
#define	TIME_CONNECT	8000 /* �����豸��������ʱʱ�� */
#define	TIME_KEEPLIVE	(30*1000) /* ��÷�һ�α���� */
#define TIME_CLIENT_DIE (TIME_KEEPLIVE*3+1000)
// ��ѯ���豸��ģ���б�
#define	TIME_QUERY_MASTER	(15*1000)
// ��ѯ���豸��һЩ��Ϣ
#define	TIME_QUERY_SLAVE	(1*60*1000)

/* 512kbps��ADSL������5��1400���ȱ��ģ����Ҫ110���� */
#define	TIME_NET_DETECT_TIMEOUT	300
#define	TIME_NET_DETECT_FAIL_RETRY	(60*1000)

// ��Ƶʧ�ܺ󣬶�ó�����һ��
#define	TIME_VIDEO_NEXT_TRY	3000
// �ȴ�SYN_A�ĳ�ʱʱ��
#define	TIME_VIDEO_WAIT_VTAP_A	5000
// �ȴ�SYN_A�ĳ�ʱʱ��
#define	TIME_VIDEO_WAIT_SYN_A	5000
// ��Ƶ�쳣��ʱʱ�䣬���û�յ�������Ϊ�����⣬��λms
#define	TIME_VIDEO_DIE	15000
// ϵͳ�ڲ��������ʱ�೤����
#define	TIME_DELAY_SYS_ERROR 3000

#define	TIME_QUERY_VIDEO_TIMER	(20*1000)
#define	TIME_QUERY_IA_TIMER	    (10*1000)

#define	TIME_ROLL	100
#define	ROLL_SEND_COUNT	4

// ��ѯ�����豸�ļ��
#define	TIME_EQUIPMENT_QUERY	15000
#define TIME_REMOTE_KEY_QUERY   20000
#define TIME_ALARM_PHONE_QUERY  25000

// �����⣬��λ�룬�����治ͬ
#define	TIME_QUERY_IR_LIB_QUICK	(10)
#define	TIME_QUERY_IR_LIB_SLOW	(5*60)

//������͵�������
#define TIME_AREA_QUERY   15000
#define TIME_SINGLE_AREA_QUERY 20000
//���龰ģʽ
#define TIME_SCENE_QUERY   15000
#define TIME_SINGLE_SCENE_QUERY 20000

// smart config��÷���һ��UDP����
#define	TIME_SMART_CONFIG_PKT	15
//6621һ�������ظ�����һ���ֽڼ��ʱ��
#define TIME_SC_6621_SEND_BYTE_INTER  3
//6621����һ���ֽں����Ϣʱ��
#define TIME_SC_6621_SEND_BYTE_IDLE     30

#undef max
#undef min
#define	max(a, b)	(((a) > (b)) ? (a) : (b))
#define	min(a, b)	(((a) < (b)) ? (a) : (b))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#undef	IS_SAME_STR
#define	IS_SAME_STR(a, b) ((a) == (b) || ((a) != NULL && (b) != NULL && strcmp((a), (b)) == 0))

#undef IP_SHOW
#define IP_SHOW(IP) ((IP)>>24)&0xFF, ((IP)>>16)&0xFF, ((IP)>>8)&0xFF, (IP)&0xFF
#undef MAC_SHOW
#define	MAC_SHOW(MAC) (MAC)[0], (MAC)[1], (MAC)[2], (MAC)[3], (MAC)[4], (MAC)[5]
#undef TM_SHOW
#define	TM_SHOW(tm) tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec

#define BUILD_U32(u1, u2, u3, u4) \
( ((((u_int32_t)(u1))<<24) & 0xFF000000) \
 | ((((u_int32_t)(u2))<<16) & 0x00FF0000)\
 | ((((u_int32_t)(u3))<<8) & 0x0000FF00)\
 | ((u_int32_t)(u4) & 0x000000FF) )

#define BUILD_U32_FROM_U16(u1,u2) \
( ((((u_int32_t)(u1))<<16) & 0xFFFF0000) \
 | (((u_int32_t)(u2)) & 0x0000FFFF))\

#define BUILD_U16(u1, u2) (((((u_int16_t)(u1))<<8)&0xFF00) | (((u_int16_t)(u2))&0x00FF))

#define BUILD_U64_FROM_U32(u1,u2)\
    ( ((((u_int64_t)(u1))<<32) & 0xFFFFFFFF00000000) \
     | (((u_int64_t)(u2)) & 0x00000000FFFFFFFF))\
    
#undef	BIT
#define	BIT(n)	(1<<(n))

/* ֧�ֶ��û�ͬʱ��¼��Ŀǰ����windows�ͻ���֧�� */
#define	CLIF_MULTI_USER	BIT(0)

#ifndef	SAFE_FREE
#define	SAFE_FREE(x) \
	do { \
		if ((x) != NULL) { \
			cl_free(x); \
			x = NULL; \
		} \
	} while (0)
#endif

#define	STR_REPLACE(dst, src) \
	do { \
		SAFE_FREE(dst); \
		dst = cl_strdup(src); \
	} while (0)

#define	MEM_REPLACE(dst, src, size) \
	do { \
		SAFE_FREE(dst); \
		dst = cl_malloc(size); \
		memcpy(dst, src, size); \
	} while (0)

#undef MAX
#undef MIN
#define MAX(x, y) ( ((x)>(y))?(x):(y) )
#define MIN(x, y) ( ((x)<(y))?(x):(y) )
        
#define PHONE_BACKGROUND_RETURN_CHECK() \
{ \
    if ( cl_priv->run_in_background ){\
        return 0;\
    }\
}


// client status
enum {
	CS_IDLE = 0,
	CS_DISP,
	CS_DEV_CONNECTING,
	CS_AUTH,
	CS_LOGIN_ERR,
	CS_ESTABLISH,
};

typedef struct{
	u_int8_t auth[32];
	u_int32_t global_ip;
} auth_a_t;

#define BOFP(buf, ofs) (&((char *)(buf))[ofs])
#define NORMAL_ERRNO(e) ((e) == EINTR || (e) == EAGAIN || (e) == EWOULDBLOCK)

#define	get_pkt_payload(pkt, type) (type *)BOFP((pkt)->data, net_hdr_len((pkt)->data))
#define	get_net_pkt_payload(hdr, type) (type *)BOFP((hdr), net_hdr_len(hdr))


typedef u_int32_t (* cl_thread_proc_t)(void *param);
typedef void (* cl_timer_proc_t)(u_int32_t tid, void *param);

struct cl_notify_pkt_s;

typedef struct {
	// �߳���Ϣ
	char name[32];
	cl_thread_hadle_t	handle;
	u_int32_t tid;

	// �����߳�֪ͨ���߳��õ�, udp
	SOCKET sock_notify;
	struct sockaddr_in addr_notify;

	int udp_buf_size;
	struct cl_notify_pkt_s *udp_buf;
	// read notify
	cl_thread_t *t_read;

	// function to process notify
	RS (*proc_notify)(struct cl_notify_pkt_s *);
	// ֹͣ��
	bool stopping;
} cl_thread_info_t;

typedef struct _pkt_t {
	struct stlc_list_head link;
	u_int8_t action;
    u_int64_t pkt_ident; //���ڴ洢���ݰ�����
    u_int32_t home_id;//�ڳ�ʱ��ø�app�ϴ�����
	// �ܳ���
	int total;
	// ���ܺ󲻺�֪����������һ�����ڵ���
	int cmd;
	// ������һ��ȫ��Ψһ���
	u_int32_t seq;
	u_int16_t up_total; //�����豸�����ܸ���
	u_int16_t up_current; //�����豸�������
	// net_header_t *
	u_int8_t data[0];
} pkt_t;



#define	NUM_EVENT_MORE_INFO	200

typedef struct {
	struct stlc_list_head link;
	// ͳ��: �����˶��ٴ�
	u_int32_t count;
	// ͳ��: ���һ����ʲôʱ���á���λ ms
	u_int32_t last_used;
	cl_event_more_info_t info;
} event_more_info_t;

typedef struct {
	struct stlc_list_head link;

	// ����Ϣ����ʱ�䣬��λ: ms
	u_int32_t create_time;
	// ��λ: ms������Ժ�ϲ���ʱ
	u_int32_t merge_timeout;
	cl_callback_t func;
	int event;
	cl_handle_t obj_handle;
	void *callback_handle;
	event_more_info_t *more_info;
} event_info_t;


typedef struct {
	struct stlc_list_head link;

	// ���ͷź���
	void (*free_obj)(void *ptr);
	// ȫ��Ψһ��ʶ
	cl_handle_t handle;
	cl_callback_t callback;
	void *callback_handle;
} tmp_obj_t;

#define	MASTER	(&cl_priv->master)
#define	CL_LOCK	cl_lock(&cl_priv->mutex)
#define	CL_UNLOCK	cl_unlock(&cl_priv->mutex)

//SDK������ͳ��
    
#define SUPPORT_TRAFFIC_STAT
//����32λ���㹻,���64λ�������ǻع�������
typedef struct {
    u_int64_t tx_bytes;
    u_int64_t rx_bytes;
    u_int32_t tx_pkts;
    u_int32_t rx_pkts;
}traffic_stat_item_t;
    
typedef struct {
    traffic_stat_item_t tcp_stat;//���͸��豸�������������ͳ��
    traffic_stat_item_t udp_stat; //��Ҫ��udp��Ƶͳ��
    traffic_stat_item_t http_stat; //http����ͳ��
    traffic_stat_item_t tcp_cmd_stat[CMD_MAX]; //���������ͳ��
}clib_traffic_stat_t;

//����ͳ�ƺ���ʹ��
enum {
    TRAFFIC_UDP = CMD_MAX,
    TRAFFIC_HTTP,
    TRAFFIC_MAX
};
    
#define MAX_DNS_NAME	32
#define MAX_ADDR_CNT	10

#define MAX_DNS_RESLOVE_4G_TRY_CNT 10

// 2��3��4G����£�ֻ����10�Σ�wifi ģʽ�£�Ϊ֧���û�������ߣ���������,����ģʽ�£�������
typedef struct {
    char dns[MAX_DNS_NAME];
    u_int8_t failed_cnt; //��¼ʧ�ܴ���
    u_int8_t pad[3];
}clib_dns_server_info_t;

typedef union {
	struct sockaddr_in sockaddr;//16
	struct sockaddr_in6 sockaddr6;//28
	u_int8_t sockaddrc[40];
}ipc_addr_t;

typedef struct {
	int ai_family;
	int ai_flags;
	int ai_socktype;
	int ai_protocol;
	size_t ai_addrlen;
	//������struct sockaddr_in ,sockaddr_in6
	ipc_addr_t addr;
}ipc_sock_t;

//disp ���Ƴ�ʱ��ʱ��
#define DISP_STAT_TIME_OUT		(30)

#define STAT_PORT_COUNT 	(3)

typedef struct {
	u_int32_t send_pkt;
	u_int32_t recv_pkt;
}disp_pkt_stat_port_t;

typedef struct {
	u_int32_t ip;
	disp_pkt_stat_port_t stat_port[STAT_PORT_COUNT];
}disp_pkt_stat_ip_t;

typedef struct {
	u_int32_t valid_count;
	disp_pkt_stat_ip_t stat_ip[MAX_DISP_SERVER_IP];
}disp_pkt_stat_t;

   
/*
	client libary private data
*/
typedef struct {
	// �ͻ������ͣ�CID_XXX������PC��ƻ������׿��
	u_int32_t cleint_type;
	u_int32_t app_type;
	u_int32_t flags;
	u_int8_t uuid[MAX_UUID_LEN];
	u_int8_t uuid_bin[MAX_UUID_BIN_LEN];
    u_int8_t phone_desc[64];
	// ������ʱ��
	int timezone;
	// ���ͻ��˵�OEM id
	int vvid;
	//app id
	u_int32_t app_id;
	//oem
	char oem_vendor[16];
	// UDP���Ƶı�����ʼ�˿ڣ�����vvid���㣬��������ͬһ�ֻ��ϲ�ͬAPP�Ķ˿��г�ͻ
	int ucc_port_base;
	// ����Ŀ¼����������һЩ��Ϣ������
	char *dir;
	//priv dir
	char *priv_dir;
	//������Ͽ���
	bool nd_debug;
	u_int32_t nd_debug_size;
	//app ver
	u_int8_t app_ver[3];

	// NET_TYPE_xxx
	int net_type;
    //�Ƿ��Ǻ�̨����
    bool run_in_background;
	u_int32_t app_baddr;//�ϲ��·��Ĺ㲥��ַ
	
	cl_thread_master_t master;
	
	// �¼��ص�
	cl_thread_t *t_event;
	cl_thread_t *t_event_timer;
	// event_info_t
	struct stlc_list_head events;
	// event_more_info_t
	struct stlc_list_head event_more_info;
	
	// �����߳�
	cl_thread_info_t thread_main;
	cl_thread_info_t thread_dns_resolv;

	// ����request_id
	cl_mutex_t mutex_request;
	// 0�������������ѱ�֪ͨ�߳���
	u_int32_t request_id;

	// 0���������������ڷ���������ʱ���ұ�����Ϣ��
	u_int32_t server_handle;
	struct stlc_list_head wait_list;
	cl_thread_t *t_check_wait;

	// ����������������������
	cl_mutex_t mutex;

	//�յ�����������ȡ�б�Ϊ�˲��ظ���ȡid
	struct stlc_list_head ir_query_list;
	//����Ȧ���û���
	struct stlc_list_head la_user;
	// ������ӵ��û�, user_t
	struct stlc_list_head user;
	// һЩ��ʱ�Ķ���, tmp_obj_t
	struct stlc_list_head tmp_obj;

	//test dns, must be ip, if set, do not resolve jiazhang007 and jiazhang008
	char *testdns;
	char *disp_server;
    u_int8_t dns_count;
    clib_dns_server_info_t *clib_dns_servers;
	// dns��������ַ
	u_int32_t ip_disp_server[MAX_DISP_SERVER_IP];
	// dns��ַ����
	u_int32_t  num_ip_disp_server;

	//Ϊ��ipv4��ipv6���ݴ�����ӵĵ�ַ��
    u_int8_t dns_count_v6;
    clib_dns_server_info_t *clib_dns_servers_v6;

	//test server ip
	u_int8_t test_ip_num;
	u_int32_t test_ip[MAX_TEST_IP_NUM];	

	// cache dns��������ַ������ip
	u_int32_t ip_disp_server_cache[MAX_DISP_SERVER_IP];
	u_int8_t  num_ip_disp_server_cache;

	//�Ƿ��յ�disp�ظ����൱���ж������������
	bool disp_recv;
	bool disp_need_save;

	//��ת�豸ip
	u_int8_t trans_ip_num;
	u_int32_t trans_ip[MAX_TRANS_IP_NUM];

	//Ϊ�˽�ʡ������������disp�����˿�
	//disp stat
	u_int32_t total_send;
	u_int32_t total_recv;
	u_int32_t total_num;
	u_int32_t last_time;
	disp_pkt_stat_t disp_stat_back;
	disp_pkt_stat_t disp_stat;
	cl_thread_t *t_disp_time_out;

	// ÿ��handleĿǰʹ�õ�����id
	u_int32_t handles[HDLT_MAX];

	ir_lib_t *ir_lib;
    //�豸̽��
    lan_dev_probe_ctrl_t * ldpc;
    //SDK����ͳ��
    bool traffic_enable;
    clib_traffic_stat_t traffic_stat;

	// ���������صĿͻ������°汾��
	char *iphone_newest_version;
	char *android_newest_version;
	// ƻ���汾����Ӣ������
	char *desc_iphone_en;
	char *desc_iphone_ch;
	// ��׿�汾����Ӣ������
	char *desc_android_en;
	char *desc_android_ch;

	// ����Wi-Fi������Ҫ����Ϣ
	void *smart_config;
	// PM2.5������֧�ֵĳ����б�
	void *city_list;
	bool query_city_list_now;
	// �����ʾ����ģʽ
	int is_pt_mode;
	//����utcת����,����
	int time_diff;
	// ����ʱ��
	time_t start_time;
	// ������ͨ
	bool net_offline;
	/*********/
	//APP ���������
	void* uasc_session;
} cl_priv_t;

typedef struct _user_cmt_s{
	struct stlc_list_head hello_head;
	struct stlc_list_head alarm_head;
	struct stlc_list_head mesure_head;
	struct stlc_list_head notify_head;
	struct stlc_list_head notify_ack_head;	
	struct stlc_list_head dev_add_head;
	struct stlc_list_head dev_del_head;
	struct stlc_list_head dev_query_head;
	/*�����������յ���С��notify_id*/
	u_int32_t my_notify_id;
	u_int32_t service_ip[16];
	 /*��Ϊ�豸�û���¼ʱ�����豸����С���б�*/
	u_int64_t cmt_list[4];
	u_int64_t cmt_report_id[4];
	 /*С���б����*/
	u_int8_t cmt_cnt;	
	u_int8_t ip_count;
}user_cmt_t;


typedef struct _ia_ctrl{
	user_t *user;
	u_int8_t type;
	u_int8_t ia_sub_type;
	cl_thread_t *t_query;
	u_int16_t ns;
	ia_status_t *is_head;
} ia_t;


typedef struct{
	struct stlc_list_head link;
	cl_bind_phone_t bind;
}bind_phone_node_t;

typedef struct{
	struct stlc_list_head link;
	cl_bind_phone_result_t result;	
}bind_phone_result_node_t;

typedef struct _user_bind_phone_s{
	u_int8_t login_type; /*LT_NORMAL LT_BIND LT_UNBIND*/
	u_int8_t net_type; /*NT_SERVER NT_DEVICE NT_DEVICE_OFFLINE*/
	u_int8_t is_binding; 
	u_int8_t pad;
	int request_count;
	cl_bind_phone_t my_bind;
	struct stlc_list_head request_head;
	struct stlc_list_head result_head;
	cl_bind_phone_list_t bpl;
}user_bind_phone_t;

#define	MPF_DEVICE	BIT(1)
#define	MPF_HTTP_DEV	BIT(2)
#define	MPF_HTTP_DICT	BIT(3)

typedef struct _slave_cmt_s{
	struct sockaddr_in addr;
	#if 1
	/*�豸�����յ���С��notify_id*/
	#endif
	u_int32_t my_notify_id;
}slave_cmt_t;

// ����ͬ��ȫ���ĵ�ģ��
typedef struct {
	u_int32_t ip;
	bool valid;
} apn_server_stat_t;

typedef struct {
	cl_thread_t *t_syn;
	bool need_syn;
	u_int32_t try_count;	// ����ͬ���Ĵ���
	u_int32_t server_num;	// ��Ҫͬ���ķ������б����
	apn_server_stat_t stat[MAX_DISP_SERVER_IP];
} cl_apn_syn_t;

typedef struct {
	u_int8_t num;
	u_int32_t ip[MAX_DISP_SERVER_IP];
	u_int8_t last_err[MAX_DISP_SERVER_IP];


	u_int8_t n_reply_ip;
	u_int32_t reply_ip[MAX_DISP_SERVER_IP];
}cl_disp_map_t;

typedef struct _user_s {
	struct stlc_list_head link;
	// ���ڵ�
	struct _user_s *parent;
	
	// ���к�
	u_int64_t sn;

	// ȫ��Ψһ��ʶ
	cl_handle_t handle;

#ifndef	NO_COMMUNITY
	//��Ȩ��Ϣ�����豸��ʽ��¼��������
	ds_key_t *dskey;
	//�Ự��Կ
	u_int8_t skey[SKEY_SIZE];
#endif	
	u_int8_t ds_type;
	// ���Ե�¼�ɹ�ʱcmd_ok�е�һЩ������Ϣ
	u_int8_t sub_type;
	u_int8_t ext_type;
	char developer_id[32];	// ������ID
	u_int8_t df_flags;
    u_int8_t real_sub_type;
    u_int8_t real_ext_type;
	bool type_fixed;	// �̻����ͣ����ٱ���̬�޸���

	u_int8_t login_type; /* LT_NORMAL, LT_BIND, LT_UNBIND */
	u_int8_t net_type; /* NT_SERVER, NT_DEVICE, NT_DEVICE_OFFLINE */
	u_int8_t pu_from_cache; /* �ֻ��ʺŵ��豸��Ϣ�Ǵӱ��ػ����ȡ�� */
	u_int8_t pad[1];

	char *vendor_id;
	char *vendor_url;

	//upgrade flash info
	u_int32_t flash_size;
	flash_block_t block[FLASH_UPGRADE_BLOCK_NUM];

	//�жϸ��豸�汾�Ƿ����
	bool dev_ver_is_valid;//�Ƿ���Ч��Ϊ��ʱdev_ver_is_too_low����Ч
	bool dev_ver_is_too_low;

	//�ж�����֧�������Ƿ�仯��
	bool is_dev_la_changed;

	//***pc��ʾ��
	u_int32_t dev_ip;
	u_int32_t upgrade_version;
	//***
	
	// �ǳ�
	char *nickname;
	// �û�������û�����������SN��Ҳ�������ǳƣ�Ҳ�������ֻ���
	char *name;
	char *passwd;
    char *qr_code;
	//back passwd ����Ӹ��������룬��ֹ����ʱ�����쳣����������豸��ʱ�Բ�ͬ�������
	u_int8_t auth_err_num;
	u_int8_t back_passwd_md5[16];
	u_int8_t passwd_md5[16];
	// passwd_md5 ��ӡ��ʮ�������ַ���
	char passwd_md5_str[64];
	// �޸�����ʱ��
	char *new_passwd;
	u_int8_t new_passwd_md5[16];

	// �ֻ��ʺ�Ԥ������Կ
	u_int8_t pu_preshare_passwd[16];
	// �ֻ��ʺ�ʹ�õ������ַ�����32���ַ�
	char pu_passwd[64];
	// �ֻ��ʺŵ�¼�ɹ��󣬷��������صġ��û������Ի��õ�ƾ֤
	char pu_key[64];
	// ���λỰ��ʱʱ��
	u_int32_t pu_die;
	json_t *json;
	jvalue_t *dict;

	//�Ƿ�������rf�µ�Ƭ������
	bool is_rfstm_up;

	//�Ƿ��豸����reset����push�����¼�ʱ��Ҫ
	bool is_reset_active;

	// �Ƿ����ֻ��ʺ�
	bool is_phone_user;
	// �ֻ��ʺŹ�����豸�б�ÿ���豸�ڵ��ֶ�Ӧһ��user_t
	struct stlc_list_head dev;
	// �豸�ո���ӵ��ֻ��û��ʺ��¡���û��֤�ɹ�ǰ�����ϸñ�ʶ
	bool is_pud_authing;

	// ��ǰ״̬, CS_XXX
	u_int32_t status;
    
    int err_backup;
	// ���һ������
	int last_err;
	//��ת�豸��½
	int back_last_err;
	bool need_trans_login;
	//�Ƿ��Ǵ�enstablish���ߵģ������������Ҫע���豸������ʾ
	bool last_enstablish;

	// �Ƿ���ͨ��UDP����Э��������
	bool is_udp_ctrl;
	// ��̨ģʽ
	bool background;
	// �Ƿ�����
	bool online;
	// �Ƿ��Ǿ�������¼��
	bool m_bLanLoginOk;
	cl_callback_t callback;
	void *callback_handle;

	u_int8_t is_support_spe_up;//�Ƿ�֧��ָ������
	char *stm_spe_up_file;//ָ����������·��

	u_int8_t is_support_background_set;//�Ƿ�֧�ֺ�̨����
	u_int8_t last_background_status;

	u_int8_t is_support_telnet;//�Ƿ�֧��telnet����

	//�����Ƿ�֧�ִ����������û�
	bool is_support_batch_user;
	//�Ƿ���Ҫǿ�в�ѯһ�Σ�����app�������߹������б仯
	u_int8_t establish_num;

	// �޸�������Ҫ֪ͨ�ദ��ȫ��֪ͨ�����ɹ���MPF_XXX
	int modify_passwd_flags;

	// �׽���
	SOCKET sock_udp; /* dispatcher/video/service to aijia device*/
	cl_thread_t *t_read_udp;
	pkt_t *udp_buf;

	// ֱ��IP��ַ�����û�����
	u_int32_t direct_ip; 
	
	// ͨ�������������ѯ�����ģ�����ʱ��ת������������
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int16_t sock_udp_port; /* dispatcher/video/service to aijia device*/

	u_int32_t local_ip;
	// ��֤ʱ�򷵻�ȫ��IP������ʱ��ת��Ϊ��������
	u_int32_t global_ip;

	// �Ƿ��δ�ɹ���¼��
	bool never_establish;
	// ֪ͨ��¼ʧ�ܵĴ���
	u_int32_t notify_login_err_count;
	u_int32_t udp_recv_pkts;
	u_int32_t tcp_recv_pkts;
	cl_thread_t *t_timer_login_timeout;
	
	SOCKET sock_dev_server; /* device server*/
	cl_thread_t *t_read;
	cl_thread_t *t_write;
	cl_thread_t *t_timer;
    cl_thread_t *t_disp_die_timer;
	cl_thread_t *t_uc_agent;	// �������
	// ����
	cl_thread_t *t_timer_keeplive;
	// ��ѯ���豸ģ���б�
	cl_thread_t *t_timer_query_master;
	// ��ѯ���豸ͳ����Ϣ��̽���б��
	cl_thread_t *t_timer_query_slave;
	// ��ѯ�龰�뱨��������
	cl_thread_t *t_timer_query_link_scene;
	// ��ѯ��������������������PM2.5
	cl_thread_t *t_timer_query_env;
	// user�µ�ͨ�ö�ʱ��
	cl_thread_t *t_timer_comm;
	int query_env_count;
	//��ѯ�ֻ�����Ϣ����
	cl_thread_t *t_timer_pu_msg;
	u_int64_t pu_msg_expect_id;

	// ���ƿ��ٲ�ѯ�Ĵ���
	int quick_query_master;
	int quick_query_time;
	
	pkt_t *tcp_buf;
	u_int32_t tcp_recv_len; /* �Ѿ����˶����ֽ� */
	struct stlc_list_head tcp_send_list; /* tcp ���Ͷ��� */
	u_int32_t tcp_send_len; /* ��һ�����ķ��˶����ֽ� */

	// ���豸�б�, slave_t
	struct stlc_list_head slave;
	// С���¼����, user_cmt
	user_cmt_t *user_cmt;
	//��Ҫ��ѯ�Ĵ��豸����
	u_int8_t query_num;

	//�Ƿ�֧������һ����������
	bool is_support_dbc;
	// �ǻۼҾ�
	ia_t *ia; 

	//֧��˫��RF����
	bool has_db_rf;
    //֧��������
    bool has_area;
    //֧���龰ģʽ
    bool has_scene;
	// ֧���龰��ʱ��
	bool has_scene_timer;
	// ֧�ֱ����������龰ģʽ
	bool has_scene_linkage_alarm;
    //֧�ְ汾1�ĵ�������,��֧��001e��003y,usbRF��
    bool support_v1_remote_pkt;
    /*��Ϣ֪ͨ*/
    notify_push_t * np;
    /*����ѧϰ*/
    remote_key_learn_t* kl;
    /*�豸��������*/
    alarm_phone_assist_t* ap;
    //����
    area_ctrl_t* ac;
    //�龰ģʽ
    scene_ctrl_t* sc;
	// ���ֻ�
	user_bind_phone_t *bp;
	// Apple Push Notification Service Config
	cl_apns_config_t *apns_cfg;
	cl_apn_syn_t apns_syn;
	//disp������������صĴ���ֵ����Ҫ��Ϊ�˴��������������������
	cl_disp_map_t disp_map;

	// ��һ�ε��龰������ѯ��Ϣ
	int prev_scene_linkage_len;
	// net_scene_linkage_t
	void *prev_scene_linkage;

	// cl_env_mon_t
	void *env_mon;
	void *health;
	// lbs_info_t
	void *lbs;

	// ucc_session_t
	void *uc_session;
    void* smart_appliance_ctrl;
    cl_if_info_t if_info;

	//��Ƭ��͸������
	bool is_dev_support_scm;
	u_int8_t pan_type_for_808;
      u_int8_t support_set_pan;
	u_int8_t* scm_dev_desc;
	int scm_dev_desc_len;
	u_int32_t scm_user_data;

	//rf����������
	bool rf_need_up;
	u_int32_t rf_up_type;//rf�����豸����
	char *rf_up_filepath;//rf����·��

	//homeid
	u_int32_t home_id;
	//������Ҫͬ������
	bool maybe_need_pd_sync;
	//����豸ʧ�ܴ�������Ҫ�Ǳ������ʧ�ܺ�һֱ��ӵ�����
	u_int8_t add_failed;

	//�����Ƿ���modify��
	bool last_ctrl;

	//�ж��豸�ǲ���ɨ����ӵ�
	bool is_smartconf;

	// ua_mgr_t UDP������ؽṹ
	void *umgr;

	// ����״̬ƽ����ʾ���
	u_int8_t has_load_display_stat;
	u_int8_t last_display_stat;	// DISPLAY_STAT_XXX
	cl_thread_t *t_display_stat_timer;
	time_t last_click_time;	// �ϴε������豸��ʱ�䣬����豸���ߣ������־����
	u_int32_t send_disp_num;
	u_int32_t dis_send_disp_num;

	//����disp�˿�
	u_int32_t disp_die_num;
	bool last_run_in_background;
} user_t;


typedef struct {
	struct stlc_list_head link;

	int mod_id;
	// ģ�����֣�UTF-8
	char *name;
	int flags;
} mod_t;
    
    
#define RF_B_PKT_FRAME_SIZE	22
#define MAX_RF_B_PKT_RECV_CACHE_NUM 3
#define MAX_RF_B_PKT_SEND_CACHE_NUM 3
// RF �豸����
typedef struct {
    u_int8_t seq; //�����
    u_int8_t frame_num; //�����ж���֡
    u_int8_t recv_frame;
    u_int8_t byte_per_frame; //ÿ��֡�ж����ֽ�
    u_int8_t data_type; //��������
    u_int8_t is_proc_pkt; //�Ƿ��Ѿ�����������ݰ����������ݣ���ֹ���һ�������ظ���
    u_int16_t pkt_len; //���ܳ�
    u_int8_t* mask; // �Ƿ��հ���mask;
    u_int8_t* pkt; // ���ݰ�
}rf_b_pkt_defrag_t;
    
typedef struct {
    u_int8_t seq; //�����
    u_int8_t frame_num; //�����ж���֡
    u_int8_t byte_per_frame; //ÿ��֡�ж���֡
    u_int8_t data_type;
    u_int32_t req_send_time;//���͵�һ�������ĵ�ʱ��
    u_int16_t pkt_len;
    u_int8_t req_pkt_len;
    u_int8_t has_send_pkt; //�Ѿ������˶��ٸ�����
    u_int8_t send_req_cache[64]; //�����Ļ���
    u_int8_t* pkt; // ���ݰ�
}rf_b_pkt_send_cache_t;
    
typedef struct {
    u_int8_t match_type; //��ǰƥ������
    u_int8_t cur_step; // ��ǰ���е��ڼ�����������ƥ��ʱ��Ч
    u_int8_t max_step; //�ܹ����ٲ���������ƥ��ʱ��Ч
    u_int8_t error; // ƥ����ִ���
    u_int8_t cur_stat; //��ǰ״̬
    u_int16_t matched_id; // �������0��Ч
    u_int16_t num; // ƥ��յ�����ʹ��
    u_int16_t matched_ids[32]; //ƥ��յ�����ʹ��
    u_int16_t code_len;
    u_int8_t* matched_code; // ����ź� 2k
}priv_rf_ir_match_t;

typedef struct {
    u_int16_t ir_id; //��ǰ���صĺ���id
    u_int8_t cur_down_type; // 0x1:info 0x2:detail
    u_int16_t cur_pos; //��ǰ�����˶���
    u_int16_t data_total_size; //�����ж��
    u_int8_t* data; //��ǰ����
}ir_down_load_task;
    
    
typedef struct {
    u_int16_t ir_id;
    u_int8_t mode_len;
    u_int8_t data_mode_len;
    u_int8_t wk_stat_len;
    u_int8_t pad;
}ucp_air_ir_info_t;
    
typedef struct {
    u_int8_t onoff;
    // AC_MODE_XXX
    u_int8_t mode;
    // �յ��¶�
    u_int8_t temp;
    // ���٣�AC_WIND_xxx
    u_int8_t wind;
    u_int8_t wind_direct;
    u_int8_t key_id;
}ucp_air_ir_server_match_info_t;
    
typedef struct {
    // AC_POWER_xxx
    u_int8_t onoff;
    // AC_MODE_XXX
    u_int8_t mode;
    // �յ��¶�
    u_int8_t temp;
    // ���٣�AC_WIND_xxx
    u_int8_t wind;
    u_int8_t wind_direct;
    u_int8_t key_id;

    u_int8_t back_onoff;
    u_int8_t back_mode;
    u_int8_t back_temp;
    u_int8_t back_wind;
    u_int8_t back_wind_direct;
    u_int8_t back_key_id;
    
    u_int8_t is_ir_info_valid; //�����Ƿ���Ч
    u_int8_t d_ir_bit_num; //����λ��
    
    u_int8_t is_downloading; //������������
    u_int8_t pad;
    
    u_int16_t download_id;
    u_int32_t download_total_len;
    u_int16_t cur_dev_ir_id; //�豸�õ�����id,�ͱ������ݿ���п��ܲ�ͬ������ͬ�������ֻ����أ�
    u_int16_t d_ir_id; //���ݿ�id
    u_int16_t d_item_num; //���ݿ�����
    u_int8_t item_len;	// ÿ����Ŀ��Ϣ����

	u_int8_t ext_tlv[128];	// һЩ��������
	u_int16_t ext_tlv_len;

	u_int8_t common_data[512];	// ��������
	u_int8_t mask_data[512];	// ������������
	u_int8_t bytes_map[512];	// ��Ҫ����

	u_int8_t level_count2;
	u_int8_t delta;
    
    int ir_info_len;
    int ir_detail_len;
    u_int8_t* ir_info;
    u_int8_t* ir_detail;	
}priv_air_ir_stat_cache;

typedef struct {
    priv_rf_ir_match_t rf_ir_match;
    priv_air_ir_stat_cache ir_cache; //���Ϳյ�����ָ��ʱ����
    ir_down_load_task task_info;
}priv_rf_ir_mana_t;

/*
*���豸���Ĵ������
*/
enum{
	RF_BIG_PKT_IR_COLLECTION = 0,
	RF_BIG_PKT_IR_DB,
	RF_BIG_PKT_IR_CTRL,
	RF_BIG_PKT_IR_TIMER,	
	RF_BIG_PKT_COMMON,
	RF_BIG_PKT_IR_BYTE_CODE,
	RF_BIG_PKT_IR_MAX,
};

//״̬
enum{
	RBPI_IDLE = 0,
	RBPI_REQEST,//����
	RBPI_PROC,//���ݴ���
	RBPI_RET,//��ѯ�Ƿ�ɹ�
	RBPI_MAX,
};

//time
#define RBPI_SEND 		(2)
#define RBPI_TIME_OUT 	(30)

//��д����
#define RBPI_RW_LEN		(20)

//��ʱ��ʱ�䶨��
#define BMP_TIMER 	(3)
#define BMP_TIMER_DIE	(16)

//rf�豸������ȡ�����ݳ���
#define BMP_RW_DATA_LEN	(21)

//errno
enum{
	BMP_ERR_NONE = 0,
	BMP_ERR_TIMEOUT,//��ʱ
	BMP_ERR_MALLOC_FAILED,//�ڴ����ʧ��
	BMP_ERR_NO_RET,//û��Ӧ
	BMP_ERR_NO_SEND,//��������
	BMP_ERR_NO_RECV,//���������
	BMP_ERR_SEND_FAILED,//����ʧ��
	BMP_ERR_RECV_FAILED,//����ʧ��
};

//action
enum {
	BMP_ACTION_GET = 0,
	BMP_ACTION_SET,
};

//������������
#define BMP_MAX_CACHE	(10)

//�����
#define USE_BMP_TEST

typedef void (* bmp_func_t)();

typedef struct {
	char *name;
	// ���뱾״̬���õĺ���
	bmp_func_t on_into;
	// �뿪��״̬���õĺ���
	bmp_func_t on_out;
	// ������
	bmp_func_t proc_pkt;
	// ��ʱ��
	bmp_func_t timer;
} bmp_proc_t;

extern bmp_proc_t bmp_proc[RBPI_MAX];

//�����ص�����
typedef void (*rbpi_callback)(slave_t *slave, u_int8_t data_type, u_int8_t errcode);

typedef struct {
	u_int8_t status;//״̬
	bool is_send;//�Ƿ���
	cl_thread_t* t_time;//��ʱ���������ط�
	cl_thread_t* t_timeout;//��ʱ����ɶ��
	u_int8_t *send_pkt;
	u_int8_t *recv_pkt;
	slave_t *slave;
	u_int8_t data_type;
	u_int8_t cur_send_len;
	u_int16_t send_total;
	u_int16_t send_index;
	u_int16_t recv_total;
	u_int16_t recv_index;
	rbpi_callback finish;//��β����
}rf_big_pkt_proc_t;

typedef struct {
	bool busy;
	slave_t *slave;
	bool is_send;
	u_int8_t data_type;
	u_int8_t *pdata;
	u_int16_t len;
	rbpi_callback finish;
}rbmp_cache_t;


//#define ELE_STAT_CHECK

#define ELE_STAT_MAX_NUM	(100)
typedef struct {
	u_int8_t has_onlined;
	u_int32_t offline_time;//����ʱ��
	u_int8_t first_battary;
	u_int8_t index;
	u_int8_t battary_diff[ELE_STAT_MAX_NUM];
}ele_stat_t;

typedef struct _slave_s {
	struct stlc_list_head link;

	// ��ָָ��
	user_t *user;
	
	u_int64_t sn;
	// ���豸���֣�UTF-8
	char *name;
	// �豸�û��绰������绰��'\t' ����
	char *phone;
	char str_sn[32];
	char *vendor_id;
	u_int8_t id;//���豸id��¼
	u_int8_t need_cache_query;//�Ƿ�Ҫ��ѯcache

	cl_handle_t handle;

#ifdef ELE_STAT_CHECK
	ele_stat_t ele_stat;
#endif

	// ��ѯ�˶��ٴ�
	u_int32_t query_count;

	/* bind_errnoΪ2ʱ��Ч����¼���󶨵����豸SN */
	u_int64_t other_master_sn;
	/* �󶨲��������Ĵ����, BIND_ERR_XXX, 0δ����1�������,2�ѱ������豸�� */
	u_int8_t bind_error;
    
	u_int8_t area_id;
	// BMS_XX
	u_int8_t status;
	u_int8_t status_valid;	// �Ƿ����ٻ�ȡ��һ��״̬��Ϣ������
	u_int8_t sub_type;
	u_int8_t ext_type;
	bool has_ext_type;
	bool has_video_flip;
	bool has_video_record;
	bool has_video_detect;

	// ���豸������ID
	u_int8_t developer_id[32];
    // �Ƿ���̨ת�ٿ���
    bool has_roll_speed_ctrl;
    bool has_v4l2_color_setting; //֧�ֱ��Ͷ�����
	// ֧���Զ��廭��
	bool has_video_custom;
	// ֧����̨
	bool has_ptz;

	//��ʾ���°汾���豸������Ҫ��ʱ��ѯ
	bool is_new_ver;

	video_t *video;

	// �Ƿ���ң�ز���
	bool has_plug;
	// ��ң�ز����Ƿ���������
	bool has_current_detect;
	//֧�ֵ���ͳ�ƹ���
	bool has_electric_stat;
	//flag
	u_int8_t flag;
	// �����ڲ�������Ϣ 
	plug_t *plug;

	// Eϵ�����豸����豸
	// ֧�ֵ���, �Ƿ���281ģ��
	bool has_eq;
	// �Ƿ�֧�ֺ��⹦��(���ӻ����յ��ȵ�����֧�ֿ��ƺ�ѧϰ)��281ģ���еı�־
	bool has_ir;
	// �Ƿ�֧�����߹���(��忪�ء��������ȣ�֧�ַ��ͺ�ѧϰ)��281ģ���еı�־
	bool has_rf;
	// �Ƿ�֧�ָ澯(���ⱨ�������ŴŸ�Ӧ�ȣ�֧�ַ��ͺ�ѧϰ)��282ģ��
	bool has_alarm;
	// �Ƿ�֧�ֶ��źͱ������͵�������
	bool has_alarm_swich;
	
	// ֧�ֶ��빦��
	bool has_eq_gencode;
	// ֧������΢��
	bool has_eq_adjust;
	// ֧�ֶ�����������
	bool has_eq_width;
	//֧��001E��ع���
	bool has_eq_001e;
	// ֧�ֶ�ά��ɨ��json��ӵ���
	bool has_eq_add_by_json;
	// �����������豸�ȵĿ�����Ϣ
	equipment_ctrl_t *equipment;
	
	slave_cmt_t *community;

	//��Ļ�Ƿ�
	bool is_805_screen_on;
	//�����Ƿ�
	bool is_805_beep_on;
	//UDP���豸
	bool is_udp;
    //�ն������Ƿ��ȡ������״̬
    bool is_update_state;
	//�Ƿ�����ʧ��
	bool is_upgrade_err;

	//�Ƿ��Ѿ���ѯ��ժҪ��
	bool is_queryed_bd_summary;

	//rfͨ�ö�ʱ��
	cl_comm_timer_head_t comm_timer;
	cl_dev_timer_summary_t timer_summary;
    //RF ѧϰ��ʱ��
    cl_thread_t* t_rf_code_match;
	//RF ѧϰ��ʱ������Ҫ��������һ��ѧϰ�ط��ģ���һ���豸����ѧϰ�ظ����ĺ���Ҫ�����ܶ�ʧ���Ӹ��ط���ʱ��
	cl_thread_t *t_rf_code_match_resend;
	//rf ��ʱ����ѯ��ʱ��
	cl_thread_t* t_rf_com_timer_query;
    //ѧϰ��ػ���
    priv_rf_ir_mana_t match_mana;
	// ��ѯRF�豸״̬
	cl_thread_t *t_stat_query;
    
	// ��ģ���б�, cl_mod_t
	struct stlc_list_head mod_list;
    
    //������ն���
    int recv_pos;
    rf_b_pkt_defrag_t frag_recv_cache[MAX_RF_B_PKT_RECV_CACHE_NUM];
    //����������
    u_int8_t send_seq;
    int send_pos;
    cl_thread_t* t_big_pkt_send;
    rf_b_pkt_send_cache_t frag_send_cache[MAX_RF_B_PKT_SEND_CACHE_NUM];

	bool has_recv_flag_pkt;//�Ƿ��־��Ч
	u_int8_t is_support_la;//�Ƿ�֧����������

	//�Ƿ�֧������һ����������
	bool is_support_dbc;

	u_int8_t is_support_public_shortcuts_onoff;		// ֧��ͨ�õĿ�ݿ��ػ�
	cl_shortcuts_onoff_t shortcuts_onoff;			// ͨ�õĿ�ݿ��ػ� 

	//���¶�ʱ�������С�����¶�
	u_int8_t max_tmp_int;//��������
	u_int8_t max_tmp_dec;//С������
	u_int8_t min_tmp_int;//��������
	u_int8_t min_tmp_dec;//С������	

	//stat
	u_int32_t run_time;//�豸����ʱ��

	//���Ĵ���
	u_int8_t ir_num;
	rbmp_cache_t cache[BMP_MAX_CACHE];
	rf_big_pkt_proc_t rbpi_proc[RF_BIG_PKT_IR_MAX];
	
	dev_info_t dev_info;
} slave_t;


#define	PKT_HANDLE(pkt)	(((net_header_t *)((pkt)->data))->handle)

typedef struct{
	u_int32_t err_code;
	char err_str[128];
} err_no_str_t;


extern cl_priv_t *cl_priv;
#define	CL_CHECK_INIT	\
	do { \
		if (cl_priv == NULL) { \
			log_err(false, "client libary is not init now. please call cl_init first!!!!\n"); \
			return RS_ERROR; \
		} \
	} while (0)

#define	CL_CHECK_INIT_RP	\
	do { \
		if (cl_priv == NULL) { \
			log_err(false, "client libary is not init now. please call cl_init first!!!!\n"); \
			return NULL; \
		} \
	} while (0)
        

extern char * get_err_str(u_int32_t err_no);
extern bool cl_is_sn(const char *sn);
extern bool cl_is_phone(const char *str);
extern void cl_notify_destroy_thread(cl_thread_info_t *info);

extern RS cl_lock(cl_mutex_t *mutex);
extern RS cl_unlock(cl_mutex_t *mutex);

extern bool u8_is_bigger(u_int8_t a, u_int8_t b);
extern bool u16_is_bigger(u_int16_t a, u_int16_t b);
extern bool u32_is_bigger(u_int32_t a, u_int32_t b);
extern u_int32_t cl_get_request_id();
extern RS cl_create_thread(cl_thread_info_t *ti, const char *name, cl_thread_proc_t proc, void *param);
extern void cl_destroy_thread(cl_thread_info_t *ti);
extern int range_rand(int begin, int end);

extern user_t *cl_user_lookup(const char *obj_username);

extern RS event_more_info_init();
extern void event_cancel_merge(cl_handle_t handle);
extern RS event_push(cl_callback_t func, int event, cl_handle_t obj_handle, void *callback_handle);
extern RS event_push_err(cl_callback_t func, int event, cl_handle_t handle, void *callback_handle, int err);

extern pkt_t *pkt_new(int cmd, int param_len, u_int8_t ds_type);
extern pkt_t *pkt_new_v2(int cmd, int param_len, u_int8_t flags, u_int64_t slave_sn, u_int8_t ds_type);
extern void user_add_pkt(user_t *user, pkt_t *pkt);
        
/* ip��port������������õ�ַ���˿ڷ���TCP���� */
extern SOCKET connect_tcp(u_int32_t ip, int port, bool *inprogress);
extern SOCKET connect_udp(u_int32_t ip, int port);
extern SOCKET create_udp_server(u_int32_t ip, int port);
extern SOCKET create_udp_server6();
extern void pkt_free(void *pkt);
extern void hdr_order(pkt_t *pkt);
extern int tcp_read(cl_thread_t *thread);
extern int tcp_write(cl_thread_t *thread);
extern void hash_passwd(u_int8_t *result, char *passwd);
extern char *fmt_hex(char *result, u_int8_t *data, int len);
extern void update_slave_addr(slave_t *slave, struct sockaddr_in *addr);
extern tmp_obj_t *tmp_obj_alloc(int ex_size, void (*free_func)(void *));
extern void tmp_obj_free(tmp_obj_t *to);
extern void tmp_obj_del_all();
extern unsigned char timer_week_right_shift(unsigned char week);
extern unsigned char timer_week_left_shift(unsigned char week);
extern RS parse_jpg_wh(u_int8_t *date, int JpgLen, int *pOutW, int *pOutH);
extern SOCKET create_tcp_server(u_int32_t ip, int port);


#ifdef SUPPORT_TRAFFIC_STAT
extern void clib_traffic_stat(u_int32_t data_type,bool is_rx,u_int32_t bytes);
#else
#define clib_traffic_stat(type,is_rx,bytes)
#endif

#define TCP_CMD_PKT_STAT(cmd,is_rx,bytes) clib_traffic_stat(cmd,is_rx,bytes)
#define UDP_PKT_STAT(is_rx,bytes) clib_traffic_stat(TRAFFIC_UDP,is_rx,bytes)
#define HTTP_PKT_STAT(is_rx,bytes) clib_traffic_stat(TRAFFIC_HTTP,is_rx,bytes)

/*****************************************************************************
	����һЩͷ�ļ�
 *****************************************************************************/

#include "cl_notify.h"
#include "user_priv.h"
#include "video_priv.h"
#include "equipment_priv.h"

#ifdef __cplusplus
}
#endif 

#endif

