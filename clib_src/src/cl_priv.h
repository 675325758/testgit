#ifndef	__CL_PRIV_H__
#define	__CL_PRIV_H__


//#define	DEBUG_PIC_DATA	1

#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif 

// 提前声明一些数据结构，其他头文件中要用到
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

//全球通版本控制宏
#define MUT_SERVER_ADAPT

// 支持UDP代理
#define UC_AGENT

// 不要试用汉字，否则安卓显示乱码
#define	CL_DESC	"Galaxywind Corporation iHome SDK 2014/7/15"


#define	MAX_TCP_PKT	(64*1024)
#define	MAX_UDP_PKT	8192

#define	CL_MAX_PATH	256

#define	IS_INVALID_SOCK(sock)	((sock) == INVALID_SOCKET || (sock) == 0)

#define	IP_LOOPBACK	"127.0.0.1"
  
#ifndef MUT_SERVER_ADAPT
    
#ifdef AWS_SERVER //亚马逊服务器
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
//定时器 1秒
#define TIME_PER_SECOND (1000)
//定时器 1毫秒
#define TIME_PER_MSECOND (1)

#define TIME_N_SECOND(n) ((n)*(TIME_PER_SECOND))
    
#define TIME_N_MSECOND(n) (n)

#define TIME_N_MINUTE(n) (60*TIME_N_SECOND((n)))

#define	TIME_PU_MSG TIME_N_MINUTE(5)
#define	TIME_LOGIN_TIMEOUT	TIME_N_SECOND(30)

#define	TIME_IDLE	1000
// 超时时间、定时器之类的，单位毫秒
#define	TIME_TRY_DISP	500 /* 多久重新查询一次分配服务器 */
#define	TIME_CONNECT	8000 /* 连接设备服务器超时时间 */
#define	TIME_KEEPLIVE	(30*1000) /* 多久发一次保活报文 */
#define TIME_CLIENT_DIE (TIME_KEEPLIVE*3+1000)
// 查询主设备的模块列表
#define	TIME_QUERY_MASTER	(15*1000)
// 查询从设备的一些信息
#define	TIME_QUERY_SLAVE	(1*60*1000)

/* 512kbps的ADSL，发送5个1400长度报文，大概要110毫秒 */
#define	TIME_NET_DETECT_TIMEOUT	300
#define	TIME_NET_DETECT_FAIL_RETRY	(60*1000)

// 视频失败后，多久尝试下一次
#define	TIME_VIDEO_NEXT_TRY	3000
// 等待SYN_A的超时时间
#define	TIME_VIDEO_WAIT_VTAP_A	5000
// 等待SYN_A的超时时间
#define	TIME_VIDEO_WAIT_SYN_A	5000
// 视频异常超时时间，多久没收到数据认为出问题，单位ms
#define	TIME_VIDEO_DIE	15000
// 系统内部错误后，延时多长重试
#define	TIME_DELAY_SYS_ERROR 3000

#define	TIME_QUERY_VIDEO_TIMER	(20*1000)
#define	TIME_QUERY_IA_TIMER	    (10*1000)

#define	TIME_ROLL	100
#define	ROLL_SEND_COUNT	4

// 查询电器设备的间隔
#define	TIME_EQUIPMENT_QUERY	15000
#define TIME_REMOTE_KEY_QUERY   20000
#define TIME_ALARM_PHONE_QUERY  25000

// 查红外库，单位秒，跟上面不同
#define	TIME_QUERY_IR_LIB_QUICK	(10)
#define	TIME_QUERY_IR_LIB_SLOW	(5*60)

//查区域和单个区域
#define TIME_AREA_QUERY   15000
#define TIME_SINGLE_AREA_QUERY 20000
//查情景模式
#define TIME_SCENE_QUERY   15000
#define TIME_SINGLE_SCENE_QUERY 20000

// smart config多久发送一个UDP报文
#define	TIME_SMART_CONFIG_PKT	15
//6621一键配置重复发送一个字节间隔时间
#define TIME_SC_6621_SEND_BYTE_INTER  3
//6621发送一个字节后的休息时间
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

/* 支持多用户同时登录，目前仅仅windows客户端支持 */
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
	// 线程信息
	char name[32];
	cl_thread_hadle_t	handle;
	u_int32_t tid;

	// 其它线程通知本线程用的, udp
	SOCKET sock_notify;
	struct sockaddr_in addr_notify;

	int udp_buf_size;
	struct cl_notify_pkt_s *udp_buf;
	// read notify
	cl_thread_t *t_read;

	// function to process notify
	RS (*proc_notify)(struct cl_notify_pkt_s *);
	// 停止中
	bool stopping;
} cl_thread_info_t;

typedef struct _pkt_t {
	struct stlc_list_head link;
	u_int8_t action;
    u_int64_t pkt_ident; //用于存储数据包归属
    u_int32_t home_id;//在超时后好给app上传返回
	// 总长度
	int total;
	// 加密后不好知道命令，在这放一个便于调试
	int cmd;
	// 给报文一个全局唯一序号
	u_int32_t seq;
	u_int16_t up_total; //升级设备报文总个数
	u_int16_t up_current; //升级设备报文序号
	// net_header_t *
	u_int8_t data[0];
} pkt_t;



#define	NUM_EVENT_MORE_INFO	200

typedef struct {
	struct stlc_list_head link;
	// 统计: 被用了多少次
	u_int32_t count;
	// 统计: 最后一次是什么时候用。单位 ms
	u_int32_t last_used;
	cl_event_more_info_t info;
} event_more_info_t;

typedef struct {
	struct stlc_list_head link;

	// 本消息创建时间，单位: ms
	u_int32_t create_time;
	// 单位: ms。多久以后合并超时
	u_int32_t merge_timeout;
	cl_callback_t func;
	int event;
	cl_handle_t obj_handle;
	void *callback_handle;
	event_more_info_t *more_info;
} event_info_t;


typedef struct {
	struct stlc_list_head link;

	// 自释放函数
	void (*free_obj)(void *ptr);
	// 全局唯一标识
	cl_handle_t handle;
	cl_callback_t callback;
	void *callback_handle;
} tmp_obj_t;

#define	MASTER	(&cl_priv->master)
#define	CL_LOCK	cl_lock(&cl_priv->mutex)
#define	CL_UNLOCK	cl_unlock(&cl_priv->mutex)

//SDK库流量统计
    
#define SUPPORT_TRAFFIC_STAT
//本来32位就足够,搞成64位，不考虑回滚问题了
typedef struct {
    u_int64_t tx_bytes;
    u_int64_t rx_bytes;
    u_int32_t tx_pkts;
    u_int32_t rx_pkts;
}traffic_stat_item_t;
    
typedef struct {
    traffic_stat_item_t tcp_stat;//发送给设备或服务器的数据统计
    traffic_stat_item_t udp_stat; //主要是udp视频统计
    traffic_stat_item_t http_stat; //http数据统计
    traffic_stat_item_t tcp_cmd_stat[CMD_MAX]; //命令发送数据统计
}clib_traffic_stat_t;

//流量统计函数使用
enum {
    TRAFFIC_UDP = CMD_MAX,
    TRAFFIC_HTTP,
    TRAFFIC_MAX
};
    
#define MAX_DNS_NAME	32
#define MAX_ADDR_CNT	10

#define MAX_DNS_RESLOVE_4G_TRY_CNT 10

// 2、3、4G情况下，只请求10次，wifi 模式下，为支持用户插拔网线，不断请求,断网模式下，不请求
typedef struct {
    char dns[MAX_DNS_NAME];
    u_int8_t failed_cnt; //记录失败次数
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
	//可能是struct sockaddr_in ,sockaddr_in6
	ipc_addr_t addr;
}ipc_sock_t;

//disp 控制超时定时器
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
	// 客户的类型，CID_XXX，包含PC、苹果、安卓等
	u_int32_t cleint_type;
	u_int32_t app_type;
	u_int32_t flags;
	u_int8_t uuid[MAX_UUID_LEN];
	u_int8_t uuid_bin[MAX_UUID_BIN_LEN];
    u_int8_t phone_desc[64];
	// 本机的时区
	int timezone;
	// 本客户端的OEM id
	int vvid;
	//app id
	u_int32_t app_id;
	//oem
	char oem_vendor[16];
	// UDP控制的本地起始端口，根据vvid计算，这样避免同一手机上不同APP的端口有冲突
	int ucc_port_base;
	// 工作目录，用来保存一些信息到本地
	char *dir;
	//priv dir
	char *priv_dir;
	//网络诊断开关
	bool nd_debug;
	u_int32_t nd_debug_size;
	//app ver
	u_int8_t app_ver[3];

	// NET_TYPE_xxx
	int net_type;
    //是否是后台运行
    bool run_in_background;
	u_int32_t app_baddr;//上层下发的广播地址
	
	cl_thread_master_t master;
	
	// 事件回调
	cl_thread_t *t_event;
	cl_thread_t *t_event_timer;
	// event_info_t
	struct stlc_list_head events;
	// event_more_info_t
	struct stlc_list_head event_more_info;
	
	// 几个线程
	cl_thread_info_t thread_main;
	cl_thread_info_t thread_dns_resolv;

	// 保护request_id
	cl_mutex_t mutex_request;
	// 0保留，用来唤醒被通知线程用
	u_int32_t request_id;

	// 0保留，用来来回于服务器请求时查找本地信息的
	u_int32_t server_handle;
	struct stlc_list_head wait_list;
	cl_thread_t *t_check_wait;

	// 互斥锁，保护数据完整性
	cl_mutex_t mutex;

	//空调贴红外编码获取列表，为了不重复获取id
	struct stlc_list_head ir_query_list;
	//联动圈子用户链
	struct stlc_list_head la_user;
	// 所有添加的用户, user_t
	struct stlc_list_head user;
	// 一些临时的东西, tmp_obj_t
	struct stlc_list_head tmp_obj;

	//test dns, must be ip, if set, do not resolve jiazhang007 and jiazhang008
	char *testdns;
	char *disp_server;
    u_int8_t dns_count;
    clib_dns_server_info_t *clib_dns_servers;
	// dns服务器地址
	u_int32_t ip_disp_server[MAX_DISP_SERVER_IP];
	// dns地址个数
	u_int32_t  num_ip_disp_server;

	//为了ipv4，ipv6兼容处理添加的地址列
    u_int8_t dns_count_v6;
    clib_dns_server_info_t *clib_dns_servers_v6;

	//test server ip
	u_int8_t test_ip_num;
	u_int32_t test_ip[MAX_TEST_IP_NUM];	

	// cache dns服务器地址，缓存ip
	u_int32_t ip_disp_server_cache[MAX_DISP_SERVER_IP];
	u_int8_t  num_ip_disp_server_cache;

	//是否收到disp回复，相当于判断网络基本正常
	bool disp_recv;
	bool disp_need_save;

	//中转设备ip
	u_int8_t trans_ip_num;
	u_int32_t trans_ip[MAX_TRANS_IP_NUM];

	//为了节省流量，控制下disp发包端口
	//disp stat
	u_int32_t total_send;
	u_int32_t total_recv;
	u_int32_t total_num;
	u_int32_t last_time;
	disp_pkt_stat_t disp_stat_back;
	disp_pkt_stat_t disp_stat;
	cl_thread_t *t_disp_time_out;

	// 每类handle目前使用的最大的id
	u_int32_t handles[HDLT_MAX];

	ir_lib_t *ir_lib;
    //设备探测
    lan_dev_probe_ctrl_t * ldpc;
    //SDK流量统计
    bool traffic_enable;
    clib_traffic_stat_t traffic_stat;

	// 服务器返回的客户端最新版本号
	char *iphone_newest_version;
	char *android_newest_version;
	// 苹果版本的中英文描述
	char *desc_iphone_en;
	char *desc_iphone_ch;
	// 安卓版本的中英文描述
	char *desc_android_en;
	char *desc_android_ch;

	// 智能Wi-Fi配置需要的信息
	void *smart_config;
	// PM2.5、天气支持的城市列表
	void *city_list;
	bool query_city_list_now;
	// 非零表示产测模式
	int is_pt_mode;
	//用来utc转换的,分钟
	int time_diff;
	// 启动时间
	time_t start_time;
	// 外网不通
	bool net_offline;
	/*********/
	//APP 服务器相关
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
	/*服务器期望收到本小区notify_id*/
	u_int32_t my_notify_id;
	u_int32_t service_ip[16];
	 /*作为设备用户登录时，该设备所属小区列表*/
	u_int64_t cmt_list[4];
	u_int64_t cmt_report_id[4];
	 /*小区列表个数*/
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
	/*设备期望收到本小区notify_id*/
	#endif
	u_int32_t my_notify_id;
}slave_cmt_t;

// 处理同步全球订阅的模块
typedef struct {
	u_int32_t ip;
	bool valid;
} apn_server_stat_t;

typedef struct {
	cl_thread_t *t_syn;
	bool need_syn;
	u_int32_t try_count;	// 尝试同步的次数
	u_int32_t server_num;	// 需要同步的服务器列表个数
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
	// 父节点
	struct _user_s *parent;
	
	// 序列号
	u_int64_t sn;

	// 全局唯一标识
	cl_handle_t handle;

#ifndef	NO_COMMUNITY
	//授权信息，以设备方式登录到服务器
	ds_key_t *dskey;
	//会话密钥
	u_int8_t skey[SKEY_SIZE];
#endif	
	u_int8_t ds_type;
	// 来自登录成功时cmd_ok中的一些附带信息
	u_int8_t sub_type;
	u_int8_t ext_type;
	char developer_id[32];	// 开发者ID
	u_int8_t df_flags;
    u_int8_t real_sub_type;
    u_int8_t real_ext_type;
	bool type_fixed;	// 固化类型，不再被动态修改了

	u_int8_t login_type; /* LT_NORMAL, LT_BIND, LT_UNBIND */
	u_int8_t net_type; /* NT_SERVER, NT_DEVICE, NT_DEVICE_OFFLINE */
	u_int8_t pu_from_cache; /* 手机帐号的设备信息是从本地缓存读取的 */
	u_int8_t pad[1];

	char *vendor_id;
	char *vendor_url;

	//upgrade flash info
	u_int32_t flash_size;
	flash_block_t block[FLASH_UPGRADE_BLOCK_NUM];

	//判断改设备版本是否过低
	bool dev_ver_is_valid;//是否有效，为真时dev_ver_is_too_low才有效
	bool dev_ver_is_too_low;

	//判断联动支持属性是否变化了
	bool is_dev_la_changed;

	//***pc显示用
	u_int32_t dev_ip;
	u_int32_t upgrade_version;
	//***
	
	// 昵称
	char *nickname;
	// 用户输入的用户名，可能是SN，也可能是昵称，也可能是手机号
	char *name;
	char *passwd;
    char *qr_code;
	//back passwd ，添加个备份密码，防止联动时各种异常导致密码跟设备暂时性不同步的情况
	u_int8_t auth_err_num;
	u_int8_t back_passwd_md5[16];
	u_int8_t passwd_md5[16];
	// passwd_md5 打印成十六进制字符串
	char passwd_md5_str[64];
	// 修改密码时用
	char *new_passwd;
	u_int8_t new_passwd_md5[16];

	// 手机帐号预共享密钥
	u_int8_t pu_preshare_passwd[16];
	// 手机帐号使用的密码字符串，32个字符
	char pu_passwd[64];
	// 手机帐号登录成功后，服务器返回的、用户后续对话用的凭证
	char pu_key[64];
	// 本次会话超时时间
	u_int32_t pu_die;
	json_t *json;
	jvalue_t *dict;

	//是否是升级rf下单片机镜像
	bool is_rfstm_up;

	//是否被设备主动reset，在push在线事件时需要
	bool is_reset_active;

	// 是否是手机帐号
	bool is_phone_user;
	// 手机帐号管理的设备列表，每个设备节点又对应一个user_t
	struct stlc_list_head dev;
	// 设备刚刚添加到手机用户帐号下、还没认证成功前，置上该标识
	bool is_pud_authing;

	// 当前状态, CS_XXX
	u_int32_t status;
    
    int err_backup;
	// 最后一个错误
	int last_err;
	//中转设备登陆
	int back_last_err;
	bool need_trans_login;
	//是否是从enstablish掉线的，这种情况还是要注意设备离线提示
	bool last_enstablish;

	// 是否是通过UDP控制协议来控制
	bool is_udp_ctrl;
	// 后台模式
	bool background;
	// 是否在线
	bool online;
	// 是否是局域网登录的
	bool m_bLanLoginOk;
	cl_callback_t callback;
	void *callback_handle;

	u_int8_t is_support_spe_up;//是否支持指定升级
	char *stm_spe_up_file;//指定升级镜像路径

	u_int8_t is_support_background_set;//是否支持后台设置
	u_int8_t last_background_status;

	u_int8_t is_support_telnet;//是否支持telnet设置

	//网关是否支持大数据批量用户
	bool is_support_batch_user;
	//是否需要强行查询一次，可能app离线上线过程中有变化
	u_int8_t establish_num;

	// 修改密码需要通知多处。全部通知完才算成功。MPF_XXX
	int modify_passwd_flags;

	// 套接字
	SOCKET sock_udp; /* dispatcher/video/service to aijia device*/
	cl_thread_t *t_read_udp;
	pkt_t *udp_buf;

	// 直连IP地址，由用户设置
	u_int32_t direct_ip; 
	
	// 通过分配服务器查询回来的，保存时候转化成主机序了
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int16_t sock_udp_port; /* dispatcher/video/service to aijia device*/

	u_int32_t local_ip;
	// 认证时候返回全局IP，保存时候转化为主机序了
	u_int32_t global_ip;

	// 是否从未成功登录过
	bool never_establish;
	// 通知登录失败的次数
	u_int32_t notify_login_err_count;
	u_int32_t udp_recv_pkts;
	u_int32_t tcp_recv_pkts;
	cl_thread_t *t_timer_login_timeout;
	
	SOCKET sock_dev_server; /* device server*/
	cl_thread_t *t_read;
	cl_thread_t *t_write;
	cl_thread_t *t_timer;
    cl_thread_t *t_disp_die_timer;
	cl_thread_t *t_uc_agent;	// 请求代理
	// 保活
	cl_thread_t *t_timer_keeplive;
	// 查询主设备模块列表
	cl_thread_t *t_timer_query_master;
	// 查询从设备统计信息、探测列表等
	cl_thread_t *t_timer_query_slave;
	// 查询情景与报警的联动
	cl_thread_t *t_timer_query_link_scene;
	// 查询环境参数，包括天气、PM2.5
	cl_thread_t *t_timer_query_env;
	// user下的通用定时器
	cl_thread_t *t_timer_comm;
	int query_env_count;
	//查询手机用消息推送
	cl_thread_t *t_timer_pu_msg;
	u_int64_t pu_msg_expect_id;

	// 控制快速查询的次数
	int quick_query_master;
	int quick_query_time;
	
	pkt_t *tcp_buf;
	u_int32_t tcp_recv_len; /* 已经读了多少字节 */
	struct stlc_list_head tcp_send_list; /* tcp 发送队列 */
	u_int32_t tcp_send_len; /* 第一个报文发了多少字节 */

	// 从设备列表, slave_t
	struct stlc_list_head slave;
	// 小区事件相关, user_cmt
	user_cmt_t *user_cmt;
	//需要查询的从设备个数
	u_int8_t query_num;

	//是否支持批量一键布防撤防
	bool is_support_dbc;
	// 智慧家居
	ia_t *ia; 

	//支持双向RF功能
	bool has_db_rf;
    //支持区域功能
    bool has_area;
    //支持情景模式
    bool has_scene;
	// 支持情景定时器
	bool has_scene_timer;
	// 支持报警器触发情景模式
	bool has_scene_linkage_alarm;
    //支持版本1的电器报文,即支持001e，003y,usbRF等
    bool support_v1_remote_pkt;
    /*消息通知*/
    notify_push_t * np;
    /*按键学习*/
    remote_key_learn_t* kl;
    /*设备报警号码*/
    alarm_phone_assist_t* ap;
    //区域
    area_ctrl_t* ac;
    //情景模式
    scene_ctrl_t* sc;
	// 绑定手机
	user_bind_phone_t *bp;
	// Apple Push Notification Service Config
	cl_apns_config_t *apns_cfg;
	cl_apn_syn_t apns_syn;
	//disp分配服务器返回的错误值，主要是为了处理多国服务器报错的问题
	cl_disp_map_t disp_map;

	// 上一次的情景联动查询信息
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

	//单片机透传控制
	bool is_dev_support_scm;
	u_int8_t pan_type_for_808;
      u_int8_t support_set_pan;
	u_int8_t* scm_dev_desc;
	int scm_dev_desc_len;
	u_int32_t scm_user_data;

	//rf升级用数据
	bool rf_need_up;
	u_int32_t rf_up_type;//rf升级设备类型
	char *rf_up_filepath;//rf升级路径

	//homeid
	u_int32_t home_id;
	//可能需要同步密码
	bool maybe_need_pd_sync;
	//添加设备失败次数，主要是避免添加失败后一直添加的问题
	u_int8_t add_failed;

	//控制是否发送modify的
	bool last_ctrl;

	//判断设备是不是扫描添加的
	bool is_smartconf;

	// ua_mgr_t UDP代理相关结构
	void *umgr;

	// 网络状态平滑显示检测
	u_int8_t has_load_display_stat;
	u_int8_t last_display_stat;	// DISPLAY_STAT_XXX
	cl_thread_t *t_display_stat_timer;
	time_t last_click_time;	// 上次点击这个设备的时间，如果设备在线，这个标志清零
	u_int32_t send_disp_num;
	u_int32_t dis_send_disp_num;

	//可用disp端口
	u_int32_t disp_die_num;
	bool last_run_in_background;
} user_t;


typedef struct {
	struct stlc_list_head link;

	int mod_id;
	// 模块名字，UTF-8
	char *name;
	int flags;
} mod_t;
    
    
#define RF_B_PKT_FRAME_SIZE	22
#define MAX_RF_B_PKT_RECV_CACHE_NUM 3
#define MAX_RF_B_PKT_SEND_CACHE_NUM 3
// RF 设备大报文
typedef struct {
    u_int8_t seq; //包序号
    u_int8_t frame_num; //包内有多少帧
    u_int8_t recv_frame;
    u_int8_t byte_per_frame; //每个帧有多少字节
    u_int8_t data_type; //数据类型
    u_int8_t is_proc_pkt; //是否已经处理过该数据包的完整数据（防止最后一个报文重复）
    u_int16_t pkt_len; //包总长
    u_int8_t* mask; // 是否收包的mask;
    u_int8_t* pkt; // 数据包
}rf_b_pkt_defrag_t;
    
typedef struct {
    u_int8_t seq; //包序号
    u_int8_t frame_num; //包内有多少帧
    u_int8_t byte_per_frame; //每个帧有多少帧
    u_int8_t data_type;
    u_int32_t req_send_time;//发送第一个请求报文的时间
    u_int16_t pkt_len;
    u_int8_t req_pkt_len;
    u_int8_t has_send_pkt; //已经发送了多少个报文
    u_int8_t send_req_cache[64]; //请求报文缓存
    u_int8_t* pkt; // 数据包
}rf_b_pkt_send_cache_t;
    
typedef struct {
    u_int8_t match_type; //当前匹配类型
    u_int8_t cur_step; // 当前进行到第几步，服务器匹配时有效
    u_int8_t max_step; //总共多少步，服务器匹配时有效
    u_int8_t error; // 匹配出现错误
    u_int8_t cur_stat; //当前状态
    u_int16_t matched_id; // 启动后非0有效
    u_int16_t num; // 匹配空调编码使用
    u_int16_t matched_ids[32]; //匹配空调编码使用
    u_int16_t code_len;
    u_int8_t* matched_code; // 最大信号 2k
}priv_rf_ir_match_t;

typedef struct {
    u_int16_t ir_id; //当前下载的红外id
    u_int8_t cur_down_type; // 0x1:info 0x2:detail
    u_int16_t cur_pos; //当前下载了多少
    u_int16_t data_total_size; //数据有多大
    u_int8_t* data; //当前数据
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
    // 空调温度
    u_int8_t temp;
    // 风速，AC_WIND_xxx
    u_int8_t wind;
    u_int8_t wind_direct;
    u_int8_t key_id;
}ucp_air_ir_server_match_info_t;
    
typedef struct {
    // AC_POWER_xxx
    u_int8_t onoff;
    // AC_MODE_XXX
    u_int8_t mode;
    // 空调温度
    u_int8_t temp;
    // 风速，AC_WIND_xxx
    u_int8_t wind;
    u_int8_t wind_direct;
    u_int8_t key_id;

    u_int8_t back_onoff;
    u_int8_t back_mode;
    u_int8_t back_temp;
    u_int8_t back_wind;
    u_int8_t back_wind_direct;
    u_int8_t back_key_id;
    
    u_int8_t is_ir_info_valid; //数据是否有效
    u_int8_t d_ir_bit_num; //数据位数
    
    u_int8_t is_downloading; //正在下载数据
    u_int8_t pad;
    
    u_int16_t download_id;
    u_int32_t download_total_len;
    u_int16_t cur_dev_ir_id; //设备用的真正id,和本地数据库的有可能不同，若不同，触发手机下载；
    u_int16_t d_ir_id; //数据库id
    u_int16_t d_item_num; //数据库条数
    u_int8_t item_len;	// 每个条目信息长度

	u_int8_t ext_tlv[128];	// 一些额外数据
	u_int16_t ext_tlv_len;

	u_int8_t common_data[512];	// 公共数据
	u_int8_t mask_data[512];	// 公共数据掩码
	u_int8_t bytes_map[512];	// 重要数据

	u_int8_t level_count2;
	u_int8_t delta;
    
    int ir_info_len;
    int ir_detail_len;
    u_int8_t* ir_info;
    u_int8_t* ir_detail;	
}priv_air_ir_stat_cache;

typedef struct {
    priv_rf_ir_match_t rf_ir_match;
    priv_air_ir_stat_cache ir_cache; //发送空调控制指令时传送
    ir_down_load_task task_info;
}priv_rf_ir_mana_t;

/*
*从设备大报文处理相关
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

//状态
enum{
	RBPI_IDLE = 0,
	RBPI_REQEST,//请求
	RBPI_PROC,//数据处理
	RBPI_RET,//查询是否成功
	RBPI_MAX,
};

//time
#define RBPI_SEND 		(2)
#define RBPI_TIME_OUT 	(30)

//读写长度
#define RBPI_RW_LEN		(20)

//定时器时间定义
#define BMP_TIMER 	(3)
#define BMP_TIMER_DIE	(16)

//rf设备期望读取的数据长度
#define BMP_RW_DATA_LEN	(21)

//errno
enum{
	BMP_ERR_NONE = 0,
	BMP_ERR_TIMEOUT,//超时
	BMP_ERR_MALLOC_FAILED,//内存分配失败
	BMP_ERR_NO_RET,//没回应
	BMP_ERR_NO_SEND,//不允许发送
	BMP_ERR_NO_RECV,//不允许接收
	BMP_ERR_SEND_FAILED,//发送失败
	BMP_ERR_RECV_FAILED,//接收失败
};

//action
enum {
	BMP_ACTION_GET = 0,
	BMP_ACTION_SET,
};

//大报文命令缓存个数
#define BMP_MAX_CACHE	(10)

//宏控制
#define USE_BMP_TEST

typedef void (* bmp_func_t)();

typedef struct {
	char *name;
	// 进入本状态调用的函数
	bmp_func_t on_into;
	// 离开本状态调用的函数
	bmp_func_t on_out;
	// 处理报文
	bmp_func_t proc_pkt;
	// 定时器
	bmp_func_t timer;
} bmp_proc_t;

extern bmp_proc_t bmp_proc[RBPI_MAX];

//几个回调函数
typedef void (*rbpi_callback)(slave_t *slave, u_int8_t data_type, u_int8_t errcode);

typedef struct {
	u_int8_t status;//状态
	bool is_send;//是否发送
	cl_thread_t* t_time;//定时器处理，如重发
	cl_thread_t* t_timeout;//超时报错啥的
	u_int8_t *send_pkt;
	u_int8_t *recv_pkt;
	slave_t *slave;
	u_int8_t data_type;
	u_int8_t cur_send_len;
	u_int16_t send_total;
	u_int16_t send_index;
	u_int16_t recv_total;
	u_int16_t recv_index;
	rbpi_callback finish;//收尾函数
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
	u_int32_t offline_time;//离线时间
	u_int8_t first_battary;
	u_int8_t index;
	u_int8_t battary_diff[ELE_STAT_MAX_NUM];
}ele_stat_t;

typedef struct _slave_s {
	struct stlc_list_head link;

	// 回指指针
	user_t *user;
	
	u_int64_t sn;
	// 从设备名字，UTF-8
	char *name;
	// 设备用户电话，多个电话用'\t' 隔开
	char *phone;
	char str_sn[32];
	char *vendor_id;
	u_int8_t id;//从设备id记录
	u_int8_t need_cache_query;//是否要查询cache

	cl_handle_t handle;

#ifdef ELE_STAT_CHECK
	ele_stat_t ele_stat;
#endif

	// 查询了多少次
	u_int32_t query_count;

	/* bind_errno为2时生效，记录被绑定的主设备SN */
	u_int64_t other_master_sn;
	/* 绑定操作产生的错误号, BIND_ERR_XXX, 0未出错，1密码错误,2已被其它设备绑定 */
	u_int8_t bind_error;
    
	u_int8_t area_id;
	// BMS_XX
	u_int8_t status;
	u_int8_t status_valid;	// 是否至少获取过一次状态信息从网关
	u_int8_t sub_type;
	u_int8_t ext_type;
	bool has_ext_type;
	bool has_video_flip;
	bool has_video_record;
	bool has_video_detect;

	// 从设备开发者ID
	u_int8_t developer_id[32];
    // 是否云台转速控制
    bool has_roll_speed_ctrl;
    bool has_v4l2_color_setting; //支持饱和度设置
	// 支持自定义画质
	bool has_video_custom;
	// 支持云台
	bool has_ptz;

	//表示是新版本从设备，不需要定时查询
	bool is_new_ver;

	video_t *video;

	// 是否有遥控插座
	bool has_plug;
	// 该遥控插座是否带电流检测
	bool has_current_detect;
	//支持电量统计功能
	bool has_electric_stat;
	//flag
	u_int8_t flag;
	// 插座内部控制信息 
	plug_t *plug;

	// E系列主设备或从设备
	// 支持电器, 是否有281模块
	bool has_eq;
	// 是否支持红外功能(电视机、空调等电器，支持控制和学习)。281模块中的标志
	bool has_ir;
	// 是否支持无线功能(面板开关、窗帘机等，支持发送和学习)。281模块中的标志
	bool has_rf;
	// 是否支持告警(声光报警器、门磁感应等，支持发送和学习)。282模块
	bool has_alarm;
	// 是否支持短信和报警推送单独配置
	bool has_alarm_swich;
	
	// 支持对码功能
	bool has_eq_gencode;
	// 支持无线微调
	bool has_eq_adjust;
	// 支持对码脉宽设置
	bool has_eq_width;
	//支持001E相关功能
	bool has_eq_001e;
	// 支持二维码扫描json添加电器
	bool has_eq_add_by_json;
	// 电器、安防设备等的控制信息
	equipment_ctrl_t *equipment;
	
	slave_cmt_t *community;

	//屏幕是否开
	bool is_805_screen_on;
	//蜂鸣是否开
	bool is_805_beep_on;
	//UDP从设备
	bool is_udp;
    //终端外设是否获取了最新状态
    bool is_update_state;
	//是否升级失败
	bool is_upgrade_err;

	//是否已经查询过摘要了
	bool is_queryed_bd_summary;

	//rf通用定时器
	cl_comm_timer_head_t comm_timer;
	cl_dev_timer_summary_t timer_summary;
    //RF 学习定时器
    cl_thread_t* t_rf_code_match;
	//RF 学习定时器，主要用来做第一步学习重发的，第一部设备允许学习回复报文很重要，不能丢失，加个重发定时器
	cl_thread_t *t_rf_code_match_resend;
	//rf 定时器查询定时器
	cl_thread_t* t_rf_com_timer_query;
    //学习相关缓存
    priv_rf_ir_mana_t match_mana;
	// 查询RF设备状态
	cl_thread_t *t_stat_query;
    
	// 子模块列表, cl_mod_t
	struct stlc_list_head mod_list;
    
    //大包接收队列
    int recv_pos;
    rf_b_pkt_defrag_t frag_recv_cache[MAX_RF_B_PKT_RECV_CACHE_NUM];
    //大包缓存队列
    u_int8_t send_seq;
    int send_pos;
    cl_thread_t* t_big_pkt_send;
    rf_b_pkt_send_cache_t frag_send_cache[MAX_RF_B_PKT_SEND_CACHE_NUM];

	bool has_recv_flag_pkt;//是否标志有效
	u_int8_t is_support_la;//是否支持联动控制

	//是否支持批量一键布防撤防
	bool is_support_dbc;

	u_int8_t is_support_public_shortcuts_onoff;		// 支持通用的快捷开关机
	cl_shortcuts_onoff_t shortcuts_onoff;			// 通用的快捷开关机 

	//恒温定时器最大，最小设置温度
	u_int8_t max_tmp_int;//整数部分
	u_int8_t max_tmp_dec;//小数部分
	u_int8_t min_tmp_int;//整数部分
	u_int8_t min_tmp_dec;//小数部分	

	//stat
	u_int32_t run_time;//设备运行时间

	//大报文处理
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
        
/* ip和port都是主机序，向该地址、端口发起TCP连接 */
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
	其他一些头文件
 *****************************************************************************/

#include "cl_notify.h"
#include "user_priv.h"
#include "video_priv.h"
#include "equipment_priv.h"

#ifdef __cplusplus
}
#endif 

#endif

