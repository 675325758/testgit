/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 联动通讯框架头文件
**  File:    linkage_client.h
**  Author:  liubenlong
**  Date:    11/30/2015
**
**  Purpose:
**    联动通讯框架头文件.
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
#define LA_SUPPORT_FRAGMENT	// 是否支持分片发送
#define LA_FRAGMENT_SIZE 1200

#define	LA_MAX_UDP_PKT	(64*1024)

#define UCLA_ADD_MAX_TIMES		(1)

#define UCLA_FRAGMENT_MAX_ITEMS	(8)
#define UCLA_FRAGMENT_MAX_DYNAMIC_ITEMS (255)
//flag
#define 		PHONE_USER				(1)
#define 		DONAME_CONFLICT_USER	(2)


//一个域名最大的ip解析数
#define DNS_IP_MAX_NUM	(20)

#define VERTION(maj, min, rev)		((maj&0xff) << 24| (min&0xff) << 16| (rev&0xff) <<8)

/* Type definitions. */
//手机账号事件处理状态机
enum {
	UCLA_PHONE_IDLE = 0,//空闲
	UCLA_PHONE_CREATE,//创建手机账号状态
	UCLA_PHONE_LOGIN,//登陆手机账号状态
	UCLA_PHONE_LOGOUT,//登出手机账号状态
	UCLA_PHONE_SWICH,//切换手机账号状态
	UCLA_PHONE_DEL,//删除手机账号状态
	UCLA_PHONE_PASSWD_MODIFY,//手机账号密码修改
	UCLA_PHONE_MAX,
};


enum {
	UCLAS_IDLE = 0, //空闲
	UCLAS_DIS_PROBE,//最快服务器探测阶段
	UCLAS_DISPATCH, //分配服务器阶段
	UCLAS_SERVER_AUTH,//服务器加密认证
	UCLAS_USER_REGISTER,//用户注册
	UCLAS_AUTH_REQ,
	UCLAS_AUTH_ANSWER,
	UCLAS_ESTABLISH,
	UCLAS_ERROR,
	UCLAS_SLEEP,
	UCLAS_MAX,
};

enum {
	PHONE_CREATE_NONE = 0,
	PHONE_CREATE_DOING,//创建中
	PHONE_CREATE_OK,//创建成功
	PHONE_CREATE_FAILED,//创建失败
};

//服务器用户保存flag
//是否域名冲突
#define SERVER_USER_FLAG_CONFLICT		(BIT(0))
//是否是手机用户
#define SERVER_USER_FLAG_PHONE			(BIT(1))
//没有生成用户
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

//APP 分配服务器

typedef struct {
	u_int8_t flag;
	u_int32_t rule_len;//rule_len
	u_int32_t rule_id;
	u_int32_t last_exec_time;
	u_int32_t home_id;
	u_int8_t enable;// 1表示使能中
	u_int8_t state;// 1表示正在执行
	char rule[0];
}la_rule_conf_t;

typedef struct {
	struct stlc_list_head link;

	u_int8_t flag;
	u_int32_t rule_len;//rule_len
	u_int32_t rule_id;
	u_int32_t last_exec_time;
	u_int8_t enable;// 1表示使能中
	u_int8_t state;// 1表示正在执行
	char *rule;///0结束的字符串
}la_rule_t;

//member flag define
#define LA_MEMBER_FLAG_SMARTCONF	(BIT(0))
#define LA_MEMBER_FLAG_NEED_ADD		(BIT(1))

typedef struct {
	u_int64_t sn;
	u_int32_t home_id;
	u_int32_t flag;
	u_int32_t is_la;//支持联动
	u_int8_t dev_passwd[APP_USER_UUID_NAME_LEN];//设备密码
}la_member_conf_t;

typedef struct {
	struct stlc_list_head link;

	void *session_ptr;//会话指针
	u_int8_t add_num;//添加次数
	bool del;
	la_member_conf_t conf;
}la_member_t;

#define IS_DEF_HOME_TYPE 	(1)
typedef struct {
	u_int32_t home_id; //服务器分配的全局唯一id
	u_int32_t flag;
	char home_name[64];	//家庭名称
	u_int8_t home_passwd[APP_USER_UUID_NAME_LEN];//家庭密码
	u_int32_t type; //比如1，默认不能删除等。。。	
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

// 联动一个家庭的字典
typedef struct {
	struct stlc_list_head link;

	u_int8_t *key;
	u_int16_t key_len;
	u_int8_t *value;
	u_int16_t value_len;
} la_dict_t;

typedef struct {
	struct stlc_list_head link;
	struct stlc_list_head member_link; //家庭成员链表
	struct stlc_list_head rule_link; //家庭联动配置链表
	struct stlc_list_head label_link; //标签链表
	struct stlc_list_head dict_link;	// 字典链表

	void *session_ptr;//会话指针
	cl_handle_t handle;
	u_int16_t url_num;//模板个数
	u_int8_t **url_array;//模板url数组

	u_int8_t *share;//分享码

	u_int16_t share_desc_num;//该家庭被分享使用的个数，第一个是家庭创建者
	la_share_desc_t *share_desc_array;

	u_int8_t del_flag;//删除标记

	//快捷按键
	la_sc_key_t la_sc_key[LA_SC_A_NUM]; 

	//用来处理规则太长分片组合的
	u_int32_t total_length;
	u_int32_t last_total_len_offset;//上次位移
	u_int8_t *ptotal_rule;

	bool rules_is_cache;//如果为真，表示规则不是缓存的
	
	u_int32_t last_rule_time;
	u_int32_t last_template_time;
	la_home_conf_t conf;
}la_home_t;

typedef struct {
	u_int32_t user_id; //服务器分配的全局唯一id
	u_int32_t flag;
	u_int8_t value[4];
	char user_name[APP_USER_UUID_NAME_LEN];	//16字节用户名,初期弱用户为APP生成的UUID
	char uuid[APP_USER_UUID_NAME_LEN];	//硬件唯一标示
	char passwd[APP_USER_UUID_NAME_LEN]; //密码md5	
}la_user_conf_t;

typedef struct {
	struct stlc_list_head link; //用户链表
	struct stlc_list_head home_link; //所属家庭链表

	void *session_ptr;//会话指针
	cl_handle_t handle;
	la_user_conf_t conf;
}la_user_t;

typedef struct {
	u_int32_t flag;
	char user_name[APP_USER_UUID_NAME_LEN];	//16字节用户名,初期弱用户为APP生成的UUID
	char passwd[APP_USER_UUID_NAME_LEN]; //密码md5		
}la_phone_conf_t;

typedef struct {
	struct stlc_list_head link; //用户链表

	la_phone_conf_t conf;
}la_phone_t;

//版本限制文件
typedef struct {
	struct stlc_list_head link;

	char name[64];
	char vendor[64];
	u_int16_t type;
	u_int16_t sub_type;
	u_int16_t ext_type;
	u_int32_t ver;
}la_dev_ver_limit_t;

//设备类型等数据保存
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

//可用域名
typedef struct {
	struct stlc_list_head link;

	char valid_doname[64];
}la_doname_info_t;

//弱用户登陆强用户时圈子迁移
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

	//添加设备的临时sn
	u_int64_t adddev_sn;
	//临时修改密码的sn
	u_int64_t modify_passwd_sn;
	char doname[64];
	u_int8_t widget_key[WIDGET_KEY_MAX_LEN];
	u_int32_t disp_ip;
	// 回指指针
	user_t *user;
	u_int8_t home_name[64];

	//是否连接过接收过分配服务器响应
	bool has_recv_disp;

	//session index
	u_int8_t index;

	u_int32_t reset_num;

	bool is_def_home;
	bool has_user;
	u_int8_t dmap;//域名映射

	//是否已经查询过服务器手机账号了
	bool has_phone_queryd;

	//是否需要去服务器查询所有用户，主要是手机账号登陆时需要每个session都去试下一下
	bool need_login_query_user;

	//是否需要查询相应圈子的所有信息，延时查询，
	bool need_query_home_all_info;


	//是否需要做设备版本限制请求
	bool need_dev_ver_req;
	//用来处理分片组合的
	u_int32_t limit_total_length;
	u_int32_t limit_last_total_len_offset;//上次位移
	u_int8_t *pver_limit;
	
	//是否需要保存
	bool need_save_conf;
	//为了安全性，用户名和密码单独保存，不和圈子信息，成员信息等放一块
	bool need_save_username;
	//需要清除配置
	bool need_clean_conf;

	//是否需要缓存域名dns
	bool need_save_dns;
	
	//表示默认圈子在这个session下面
	bool has_def_home;
	bool back_has_def_home;
	u_int32_t user_id;

	//用来处理同步问题的
	bool need_map_sync;
	u_int64_t map_cal;

	//连接服务器花费时间
	u_int32_t time_start;
	u_int32_t time_diff;

	//是否被踢
	bool is_kicked;
	
	u_int32_t home_del_id;
	//是否需要查询一下自己所有的家庭数
	bool need_query_all_home;

	//是否创建手机账号中
	bool is_phone_creat_status;//手机创建状态

	//临时创建强用户userid
	u_int32_t phone_user_id_tmp;

	//需要创建强用户
	bool need_create_force_user;

	//是否需要报告强用户创建成功
	bool need_report_phone_create;

	//分享注册，跨服务器的情况
	bool need_register;
	u_int32_t a_share_home_id;//从别的服务器来的分享注册的家庭id
	u_int8_t a_sc[8];//分享码
	u_int8_t a_desc[64];//desc

	u_int8_t *rule_tmp;//添加规则时临时保存用，添加成功后需要

	//标签相关
	la_label_t *plabel_tmp;//添加标签时本地缓存用
	uc_label_bind_t *plbind_tmp;//绑定标签时本地缓存用

	//快捷按键相关
	uc_home_shortcut_t *ps_tmp;
	uc_home_shortcut_modify_t sc_mod_tmp;
	
	/*
		UCCS_IDLE, UCS_AUTH, UCCS_ESTABLISH, UCS_AUTH_ERROR
	*/
	int status;
	int last_status;//上次状态
	// 会话ID，请求认证时候为0，服务器/设备应答answer时填写
	u_int32_t client_sid;
	u_int32_t device_sid;

	u_int32_t my_request_id;
	u_int32_t peer_request_id;
	// 选择的加密方式
	u_int16_t select_enc;
    //协议版本
    u_int8_t version;
	
	u_int8_t rand1[4];
	u_int8_t rand2[4];
	u_int32_t r1;//临时随机数
	u_int8_t username[APP_USER_UUID_NAME_LEN];
	u_int8_t back_username[APP_USER_UUID_NAME_LEN];
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

	u_int32_t req_time;//认证开始时间
	u_int32_t enstable_time;//连接成功时间，主要用来判断速度用的

	// 刚创建是0，后面变成正常的
	int idle_time;
	//adddr
	ipc_addr_t peer_addr;
	ipc_addr_t ipc_addr;
	//lasterr ipv6 addr
	struct sockaddr_in6 last_ipv6_addr;
	// 对端的IP地址、端口号。主机序
	u_int32_t ip;
	int port;
	u_int8_t port_index;
	bool server_die;
	// 自己绑定的端口
	int my_port;
	bool is_ipv6_sock;
	SOCKET sock;
	//分配服务器
	SOCKET disp_sock;
	SOCKET disp_sock6;
	cl_thread_t *t_disp_recv;
	cl_thread_t *t_disp_recv6;
	bool is_ipv6;
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
	// 用户密码
	u_int8_t key[APP_USER_UUID_NAME_LEN];
	u_int8_t back_key[APP_USER_UUID_NAME_LEN];
	//数据加解密秘钥
	u_int8_t aes_key[APP_USER_UUID_NAME_LEN];
	//认证工钥
	u_int8_t *auth_key;

	pkt_t *rcv_buf;
    ucph_t* rcv_hdr;
	struct stlc_list_head send_list;
    
	//联动连接服务器超时次数
	u_int8_t timeout_count;

	cl_thread_t *t_recv;
	cl_thread_t *t_timer;
	// 重试发送的定时器
	cl_thread_t *t_send;
	cl_thread_t *t_keeplive;
	cl_thread_t *t_die;
	//err timer，在error状态放个定时器，防止服务器出问题，密码错误导致一直连接不上服务器
	cl_thread_t *t_server_check;

	//处理事件收到多次报文的问题
	u_int8_t home_event_last_id;
	u_int8_t share_event_last_id;
	u_int8_t linkage_event_last_id;
	u_int8_t linkage_rulelistpush_last_id;
	u_int8_t linkage_dev_ver_limit_last_id;
	u_int8_t linkage_msg_push_last_id;
	u_int8_t last_rid;
	// 分片处理
	la_fragment_stat_t fs;
	//dns cache
	int ipc_count;
	ipc_sock_t ipc_sock[DNS_IP_MAX_NUM];
} ucla_session_t;

//s3专用
typedef struct {
	struct stlc_list_head link;

	u_int32_t home_id;
	u_int32_t last_modify_time;
	struct stlc_list_head rule_list;
}la_g_rule_t;

//日志相关
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
	cl_thread_t *t_timer;//延时定时器
	cl_thread_t *t_comm;//做些乱七八糟的定时器
	cl_thread_t *t_def_home_timer;//默认圈子创建定时器
	cl_thread_t *t_modify_passwd;//tcp等不支持联动的设备修改密码定时器
	cl_thread_t *t_comm_proc;//用来处理一下延时操作的处理
	cl_thread_t *t_dev_add_proc;//用来处理一下延时操作的处理

	//域名占用映射
	char **doname_map;

	//手机用户链表
	struct stlc_list_head la_phone;
	char cur_phone_name[16];

	//当没有圈子时，添加设备存放链表
	struct stlc_list_head dev_wait_list;
	bool need_dev_wait_save;

	bool need_dev_pass_sync_save;

	//是否需要保存正确密码
	bool need_back_passwd_save;

	//配置保存aes
	u_int8_t conf_aes_key[16];

	//session index 计数
	u_int8_t index;
	//计数映射
	u_int8_t index_map[20];
	//是否有手机账号
	bool has_phone;
	//是否需要清除配置
	bool need_clean_all_conf;

	//是否是主动查询的
	bool is_rule_all_query;

	//是否需要判断设备版本限制问题
	bool need_cal_dev_ver;

	//是否需要保存手机账号
	bool need_save_phone;

	//是否是后台模式
	bool run_in_background;

	//手机账号处理状态
	u_int8_t phone_status;
	cl_thread_t *t_phone_timer;//手机账号处理定时器
	cl_thread_t *t_phone_die;
	u_int32_t phone_timer;
	u_int32_t phone_die;
	u_int32_t phone_user_id;
	u_int8_t user_name[APP_USER_UUID_NAME_LEN];	//16字节用户名,初期弱用户为APP生成的UUID
	u_int8_t passwd[APP_USER_UUID_NAME_LEN]; //密码md5
	u_int8_t uuid[MAX_UUID_BIN_LEN];
	u_int8_t back_user_name[APP_USER_UUID_NAME_LEN];

	//需要延时创建强用户，因为有可能当前session不在线
	bool need_phone_create_delay;

	bool phone_logining;

	//是否需要保存所有域名，因为加入了oem域名，为了离线显示必须保存下来
	bool need_doname_save;

	//为了手机账号延时创建，如创建时网络不通，另一个手机登陆后可以创建
	u_int8_t p_user_name[APP_USER_UUID_NAME_LEN];	//16字节用户名,初期弱用户为APP生成的UUID
	u_int8_t p_passwd[APP_USER_UUID_NAME_LEN]; //密码md5
	u_int8_t p_passwd_cmp[APP_USER_UUID_NAME_LEN]; //密码md5
	
	bool need_phone_upload;//需要上传服务器配置

	bool phone_passwd_err;//手机账号登陆密码错误
	bool phone_acount_not_exist;//手机账号不存在
	bool acount_not_exist;
		
	u_int32_t cur_home_id;//当前homeid
	bool has_def_home;//是否有默认圈子
	bool has_any_home;//是否有任何圈子
	bool need_enstablish;//如果没有圈子，者需要保持一个session连接服务器

	//现在改成服务器同步数据了，为了兼容处理添加
	void *need_switch_session;

	//是否需要手机登陆
	bool need_phone_login_delay;

	//检查设备支持联动属性是否变化了，如升级，降级，数据没及时读取等
	bool need_la_sync_check;

	//中英文语言类型
	u_int8_t lang;

	//设备能力文件上次修改时间
	u_int32_t cap_lastmodifytime;
	u_int16_t cap_num;//能力文件个数
	u_int8_t **cap_array;//能力文件url数组

	//自定义设备能力文件上次修改时间
	u_int32_t cap_custom_lastmodifytime;
	u_int16_t cap_custom_num;//能力文件个数
	u_int8_t **cap_custom_array;//能力文件url数组

	//模板信息
	u_int32_t last_template_time;//上次模板修改时间
	u_int16_t url_num;//模板个数
	u_int8_t **url_array;//模板url数组
	void *template_session;//取得最新模板的session
	
	//服务器消息
	cl_la_notify_msg_t msg;

	//服务器是否支持标签操作
	bool support_tabel;
	//服务器是否支持快捷按键
	bool support_shortcut;
	//服务器是否支持中转
	bool support_trans;

	///0结束的pad请求分享码字符串
	char *req_share;

	u_int32_t disp_bit;

	//app启动时间
	u_int32_t init_time;

	//rsa
	bool use_rsa;
	int rsa_priv_len;
	u_int8_t *rsa_priv;
	cl_rsa_callback rsa_enc;
	cl_rsa_callback rsa_dec;

	//设备各类型最小版本限制文件数据
	u_int8_t last_ver[4];//限制文件上次调整时间
	struct stlc_list_head dev_ver_limit_list;//限制文件链表

	struct stlc_list_head dev_pass_sync_list;//密码同步链表

	//设备类型链表
	struct stlc_list_head dev_type_list;

	//s3专用，规则链表
	struct stlc_list_head g_rule_list;
	//日志相关
	struct stlc_list_head g_rule_log_list;
	struct stlc_list_head g_member_log_list;

	//可用和不可用服务器探测联动服务器域名
	struct stlc_list_head valid_doname;
	struct stlc_list_head invalid_doname;

	//迁移圈子
	la_move_home_t move_home;

	//一个空圈子，用来表示当前没有圈子时给app用的，回来个handle可以查询到
	la_home_t *p_null_home;
	
//	RSA *rsa_private;
	cl_callback_t callback;
	void *callback_handle;
	struct stlc_list_head server_client;//服务器会话
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

