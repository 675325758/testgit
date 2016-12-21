/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: linkage_client.c
**  File:    linkage_client.c
**  Author:  liubenlong
**  Date:    11/30/2015
**
**  Purpose:
**    联动通讯框架c文件.
**************************************************************************/


/* Include files. */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "client_lib.h"
#include "cl_priv.h"
#include "md5.h"
#include "cl_thread.h"
#include "aes.h"

#include "cl_dns.h"
#include "uc_client.h"
#include "udp_ctrl.h"
#include "lan_dev_probe_priv.h"
#include "cl_server.h"
#include "smart_appliance_priv.h"
#include "linkage_client.h"
#include "linkage_priv.h"
#include "cl_linkage.h"
#include "app_pkt_priv.h"

/* Macro constant definitions. */
#define UCLA_PORT		1200
#define UCLA_PORT1		31200
#define UCLA_PORT2		51200
#define UCLA_MAX_PORTS	3

#undef MAX_SEND_RETRY_NUM
#define MAX_SEND_RETRY_NUM  (50)
#define MAX_PARAM_LEN_PER_PKT (1200)
//紧急命令发送失败最大次数
#define MAX_IMPORTANT_CMD_NUM 	(10)

#undef MAX_AUTH_LOST
#define MAX_AUTH_LOST (3)

#define UCLA_LA_SERVER_FILE 		"la_server"
#define UCLA_LA_SERVER_FILE_BACK 	"la_server_back"

#define UCLA_LA_MEMBER_FILE 		"la_member"
#define UCLA_LA_MEMBER_FILE_BACK 	"la_member_back"

#define UCLA_LA_HOME_FILE 			"la_home"
#define UCLA_LA_HOME_FILE_BACK	 	"la_home_back"

#define UCLA_LA_LABEL_FILE			"la_label"
#define UCLA_LA_LABEL_FILE_BACK		"la_label_back"

#define UCLA_LA_USER_FILE			"la_user"
#define UCLA_LA_USER_FILE_BACK		"la_user_back"

#define UCLA_LA_PHONE_FILE			"la_phone"
#define UCLA_LA_PHONE_FILE_BACK		"la_phone_back"

#define UCLA_LA_DEV_WAIT_LIST_FILE		"dev_wait_list"
#define UCLA_LA_DEV_WAIT_LIST_FILE_BACK	"dev_wait_list_back"

//模板配置
#define UCLA_LA_TEMPLATE_FILE		"la_template"
#define UCLA_LA_TEMPLATE_FILE_BACK	"la_template_back"

//规则配置
#define UCLA_LA_RULE_FILE			"la_rule"
#define UCLA_LA_RULE_FILE_BACK		"la_rule_back"

//分享成员配置
#define UCLA_LA_SHARE_FILE			"la_share"
#define UCLA_LA_SHARE_FILE_BACK		"la_share_back"

//快捷按键缓存
#define UCLA_LA_SC_FILE				"la_sc"
#define UCLA_LA_SC_FILE_BACK		"la_sc_back"

//能力文件缓存
#define UCLA_LA_CAP_FILE			"la_cap"
#define UCLA_LA_CAP_FILE_BACK		"la_cap_back"

//自定义能力文件缓存
#define UCLA_LA_CAP_CUSTOM_FILE			"la_cap_custom"
#define UCLA_LA_CAP_CUSTOM_FILE_BACK		"la_cap_custom_back"

#define UCLA_DUMP_INFO				"home_info.txt"

#define UCLA_DONAME_INFO			"la_doname.txt"

//设备版本限制文件
#define UCLA_DEV_VER_LIMIT_FILE			"dev_ver_limit.txt"

//密码同步文件
#define UCLA_DEV_PASS_SYNC_FILE			"dev_pass_sync.txt"
#define UCLA_DEV_PASS_SYNC_FILE_BACK	"dev_pass_sync_back.txt"

//设备类型文件
#define UCLA_DEV_TYPE_FILE		"dev_type.txt"

//备份密码保存文件
#define UCLA_BACK_PASSWD_FILE	"dev_back.txt"

//缓存dns解析的文件
#define UCLA_DNS_IP_RESOLVE_FILE 	"dns_back.txt"

#define UCLA_MAX_DOANME_MAP			(255)

#define UCLA_CONF_SAVE_MAGIC			(0xa1b2c3d4)
#define UCLA_CONF_SVAE_VER				(1)

#define UCLA_CONF_SAVE_MAX_LEN			(1024*1024)

#define UCLA_FILE_PATH_LEN				(258)


#define DMAP_TEST		//服务器还没改，只能这样测试了


//根据ip服务器探测最快分配服务器域名，探测端口
/* 接收udp的端口1 探测本国ICE */
#define APP_REGSERVER_PORT1    42190
/* 接收udp的端口2 探测本国ICE */
#define APP_REGSERVER_PORT2    52190



//表示需要全端口发送的报文
#define ACTION_ALL_PORT_SEND		(100)

/********************打印调试开关********************打印调试开关***************
*************打印调试开关********************打印调试开关*******************/
#if 1
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif

/* Type definitions. */
typedef struct {
	u_int64_t sn;
	u_int8_t passwd[16];
}ucla_back_passwd_t;



//配置保存头部
typedef struct {
	u_int32_t magic;
	u_int32_t ver;
	u_int32_t count;
	u_int32_t crc;
	u_int8_t data[0];
}ucla_conf_save_t;

typedef struct {
	u_int32_t home_id;
	u_int16_t share_num;
	la_share_desc_t share[0];
}la_share_conf_t;

typedef struct {
	u_int32_t home_id;
	la_sc_key_t la_sc_key[LA_SC_A_NUM]; 
}la_sc_conf_t;
/* Local function declarations. */


/* Macro API definitions. */
#define UCLA_DEF_DONAME 		"cn.ice.galaxywind.com"


#define UCLA_DEBUG

/* Global variable declarations. */
la_ctrl_t la_ctrl = {0};
la_ctrl_t *plc = &la_ctrl;

int ucla_mult_port[UCLA_MAX_PORTS] = {UCLA_PORT2, UCLA_PORT1, UCLA_PORT};


char *la_private_key = "sdkfjskdjfdksfjdjafdjkfjskdjfkdsjfksjd";

char *la_conf_key = "s24df5s4f4sd56f4sd3fa13sdf1sa3df1s3d21fsd1fs23df1";

void ucla_set_status(ucla_session_t *s, int status);
static int ucla_keeplive_timer(cl_thread_t *t);
void ucla_user_free(la_user_t *puser);
void do_ucla_share_query(ucla_session_t *s, u_int32_t home_id);
extern bool mem_is_all_zero(u_int8_t *data, int len);
extern void sa_query_debuginfo(ucc_session_t *s);
void ucla_home_free(la_home_t *phome);
extern bool cl_get_ip_by_doname(char *doname, u_int32_t *ip, int *num);
void la_phone_save();static void ucla_phone_timer_reset();
void ucla_all_session_clean();
void ucla_all_session_query();
void la_comm_timer_proc_reset();
static bool ucla_home_valid(la_home_t *phome);
static void la_dev_wait_list_save();
static void la_dev_wait_add(la_member_t *pmem);
extern void la_user_update_homeid(u_int64_t sn, u_int32_t home_id);
void do_all_home_free();
void ucla_all_goto_sleep();
static int la_comm_timer_proc(cl_thread_t *t);
extern bool lan_user_is_home_added(user_t *puser);
void dev_type_sync(user_t *user);
void ucla_phone_recovery();
static void do_comm_info_save();
static void la_file_clean(char *file);
static void ucla_conf_init(char *user_dir);
static u_int64_t la_get_any_sn();
static u_int16_t la_get_err_doname(u_int8_t *buf);
static la_doname_info_t *la_doname_find(bool valid, char *doname);
static void la_doname_add(bool valid, char *doname);
static void la_doname_add2(bool valid, la_doname_info_t *pd);
static void la_doname_clean(bool valid);
static void la_dev_wait_list_init();
static void ucla_other_session_goto_err(ucla_session_t *s);
static void ucla_err_doname_proc(ucla_session_t *s, char *func);
void la_back_passwd_proc();
void la_back_passwd_init();
void la_dev_type_save();
extern bool lan_user_is_la_support(user_t *puser);
static bool ucla_is_reset_pkt(ucla_session_t *s, ucph_t *hdr);
static void la_dev_pass_sync_list_save();
static void la_dev_passwd_sync_list_init();
RS ucla_udp_socket(ucla_session_t *s, bool ipv6);
static int ucla_read(cl_thread_t *t);
static int ucla_read6(cl_thread_t *t);
static void ucla_oem_session_init();
extern u_int32_t cl_get_dns_timeout(char *doname);
extern void cl_set_dns_cache(char *doname, ipc_sock_t *ipc, int count);
static void la_rename_file(ucla_session_t *s, char *file, char *file_back);
int la_get_file_size(ucla_session_t *s, char *file);
bool app_is_la();
static void ucla_enable_disp_ext(u_int8_t index, bool set);
static void ucla_clean_disp_ext();
static void do_session_switch_check(ucla_session_t *s);

/***********************************手机账号处理状态机**************************************************************/
static void ucla_phone_off_all_timer()
{
	CL_THREAD_OFF(plc->t_phone_timer);
	CL_THREAD_OFF(plc->t_phone_die);	
}

//idle
static void ucla_phone_idle_indo()
{
	ucla_phone_off_all_timer();
	la_move_home_clean();
}

static void ucla_phone_idle_outdo()
{

}

static void ucla_phone_idle_timer_proc()
{

}

//create
static int ucla_phone_create_die(cl_thread_t *t)
{
//	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);

	UCLA_ENTER();	
	plc->t_phone_die = NULL;

	// TODO:做些提示。。。。。
	event_push_err(plc->callback, LA_PHONE_CREATE_FAILED, 0, plc->callback_handle, LA_NET_TIMEOUT);
	
	ucla_phone_set_status(UCLA_PHONE_IDLE);
	return 0;
}

void do_phone_force_create(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_create_req_t *acr = NULL;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_create_req_t),
				true, false, 0,	s->client_sid, s->device_sid, s->my_request_id);
    acr  = get_ucp_payload(pkt, uc_app_user_create_req_t);
	acr ->action = UCAU_CREATE;

	memcpy(acr->username, plc->user_name, sizeof(acr->username));
	memset(acr->uuid, 0, sizeof(acr->uuid));
	memcpy(acr->passwd_md5, plc->passwd, sizeof(acr->passwd_md5));

	//log_debug("%s goto phonecreate and username=%s !!!!!!!!!!!!!!\n", s->doname, acr->username);
	ucla_request_add(s, pkt);
	s->is_phone_creat_status = PHONE_CREATE_DOING;
}

static void do_phone_create()
{
	ucla_session_t *s, *sn;

	UCLA_ENTER();
	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		s->is_phone_creat_status = PHONE_CREATE_NONE;
		if (!ucla_session_is_ok(s)) {
			continue;
		}
		do_phone_force_create(s);
	}
}

void ucla_create_all_session_clean()
{
	ucla_session_t *s, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		//所有还原成原始状态，像弱用户创建变化强用户过程
		ucla_user_del(s, s->user_id);
		s->user_id = 0;
		s->has_user = false;
		s->idle_time = 0;
		memcpy(s->uuid, cl_priv->uuid, sizeof(s->uuid));
		ucla_set_status(s, UCLAS_IDLE);
	}

	plc->has_phone = false;
	plc->phone_passwd_err = false;
	plc->need_phone_create_delay = true;
}

static void ucla_phone_create_indo()
{
	ucla_phone_off_all_timer();

	plc->phone_timer = 1;
	plc->phone_die = 20;
	
	//定时器初始化
	ucla_phone_timer_reset();
	CL_THREAD_OFF(plc->t_phone_die);
	CL_THREAD_TIMER_ON(MASTER, plc->t_phone_die, ucla_phone_create_die, NULL, TIME_N_SECOND(plc->phone_die));

	plc->phone_logining = true;
	plc->need_phone_create_delay = false;
	//这里处理一下，如果没有可用的session，则当成手机用户登陆的情况下创建强用户
	if (ucla_get_any_enstablis_session()) {
		do_phone_create();
	} else {
		ucla_create_all_session_clean();
	}
}

static void ucla_phone_create_outdo()
{

}

static void do_phone_create_ok_replace()
{
	ucla_session_t *s;
	la_user_t *puser;
	pkt_t *pkt;
	uc_app_user_phone_replace_t *pr;

	UCLA_ENTER();
	// TODO:选择一个手机强用户替换，可以修改选择策略
	s = ucla_get_any_enstablis_session();
	if (!s) {
		goto err;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto err;
	}
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_phone_replace_t),
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		goto err;
	}
	pr = get_ucp_payload(pkt, uc_app_user_phone_replace_t);
	pr ->action = UCAU_USER_REPLACE;
	pr->user_id = htonl(s->phone_user_id_tmp);
	log_debug("do_phone_create_ok_replace s->phone_user_id_tmp=%u\n", s->phone_user_id_tmp);
	memcpy((void *)pr->user_name, (void *)plc->user_name, APP_USER_UUID_NAME_LEN);
	ucla_request_add(s, pkt);
	
	return;
err:
	event_push_err(plc->callback, LA_PHONE_CREATE_FAILED, 0, plc->callback_handle, 
			LA_PHONE_DEFAULT_HOME_SERVER_OFFLINE);
}

static void do_phone_create_ok_proc()
{
	//现在不需要app做同步了
	log_debug("send do_ucla_sever_set_proc and do_phone_create_ok_proc\n");
	ucla_event_push(LA_PHONE_CREATE_SUCCESS);
}

static void ucla_phone_create_timer_proc()
{
	bool create_ok = false;
	ucla_session_t *s, *sn;

	//如果一直不在线就直接返回等待超时
	if (!ucla_get_any_enstablis_session()) {
		return;
	}

	if (plc->need_phone_create_delay) {
		plc->need_phone_create_delay = false;
		do_phone_create();
		return;
	}

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		if (s->is_phone_creat_status == PHONE_CREATE_NONE) {
			continue;
		}
		if (s->is_phone_creat_status == PHONE_CREATE_FAILED) {
			event_push_err(plc->callback, LA_PHONE_CREATE_FAILED, 0, plc->callback_handle, LA_PHONE_ACOUNT_EXIST);
			ucla_phone_set_status(UCLA_PHONE_IDLE);
			return;
		}
		if (s->is_phone_creat_status == PHONE_CREATE_OK) {
			create_ok = true;
			break;
		}
	}
	
	if (create_ok) {
		plc->phone_logining = false;
		ucla_phone_set_status(UCLA_PHONE_IDLE);
		if (plc->has_phone) {
			//在手机用户登陆情况下做的强用户注册
			do_phone_create_ok_proc();
		} else {
			do_phone_create_ok_replace();
		}
	}
}

//login
static int ucla_phone_login_die(cl_thread_t *t)
{
	plc->t_phone_die = NULL;
	
	//log_debug("%s passwd err\n", __FUNCTION__);
	// TODO:做些提示。。。。。
	event_push_err(plc->callback, LA_PHONE_LOGIN_FAILED, 0, plc->callback_handle, LA_NET_TIMEOUT);
	ucla_phone_set_status(UCLA_PHONE_IDLE);
	ucla_phone_recovery();
	return 0;
}

static void ucla_phone_ok_report()
{
	// TODO:做些提示。。。。。
	ucla_event_push(LA_PHONE_LOGIN_SUCCESS);
	plc->phone_logining = false;
	ucla_phone_set_status(UCLA_PHONE_IDLE);	
}

static void ucla_phone_login_indo()
{
	la_phone_t *phone;

	ucla_phone_off_all_timer();
	phone = ucla_find_phone_by_name((char *)plc->user_name);
	if (!phone) {
		phone = ucla_phone_new();
		if (!phone) {
			return;
		}
		ucla_phone_add(phone);
	}

	plc->phone_logining = true;
	plc->phone_timer = 1;
	plc->phone_die = 20;

	plc->phone_acount_not_exist = false;
	//定时器初始化
	ucla_phone_timer_reset();
	CL_THREAD_OFF(plc->t_phone_die);
	CL_THREAD_TIMER_ON(MASTER, plc->t_phone_die,ucla_phone_login_die, NULL, TIME_N_SECOND(plc->phone_die));
	//保存上次弱用户的圈子
	la_move_home_clean();
	ucla_weak_user_save();
	//先清楚所有手机的当前使用标志
	ucla_phone_flag_clean();
	memcpy((void *)phone->conf.passwd, (void *)plc->passwd, sizeof(phone->conf.passwd));
	memcpy((void *)phone->conf.user_name, (void *)plc->user_name, sizeof(phone->conf.user_name));
	ucla_set_cur_phone(phone);
	//ucla_phone_save();
	//清空各个session
	ucla_all_session_clean();
	//替换session
	ucla_all_session_query();
	ucla_event_push(LA_USER_CHANGED);
	log_debug("ucla_phone_login_indo goto login phonename=%s\n", plc->user_name);
}

static void ucla_phone_login_outdo()
{
}

static void ucla_phone_login_timer_proc()
{
	if (plc->phone_passwd_err) {
		event_push_err(plc->callback, LA_PHONE_LOGIN_FAILED, 0, plc->callback_handle, LA_PHONE_LOGIN_PASSWD_ERR);		
		ucla_phone_set_status(UCLA_PHONE_IDLE);	
		ucla_phone_recovery();
	}
	if (plc->phone_acount_not_exist) {
		event_push_err(plc->callback, LA_PHONE_LOGIN_FAILED, 0, plc->callback_handle, LA_PHONE_NOT_EXIST);		
		ucla_phone_set_status(UCLA_PHONE_IDLE);	
		ucla_phone_recovery();
	}
}

void ucla_maybe_recovery()
{
	UCLA_ENTER();
	do_comm_info_save();
}

//logout
static void ucla_phone_logout_indo()
{
	la_phone_t *phone;
	
	ucla_phone_off_all_timer();

	phone = ucla_find_phone_by_name((char *)plc->user_name);
	if (!phone) {
		return;
	}

	stlc_list_del(&phone->link);

	ucla_all_session_clean();
}

static void ucla_phone_logout_outdo()
{

}

static void ucla_phone_logout_timer_proc()
{

}

//switch
static void ucla_phone_switch_indo()
{
	la_phone_t *phone;

	UCLA_ENTER();
	ucla_phone_off_all_timer();

	phone = ucla_find_phone_by_name((char *)plc->user_name);
	if (!phone) {
		return;
	}

	//先清楚所有手机的当前使用标志
	ucla_phone_flag_clean();
	ucla_set_cur_phone(phone);
	memcpy((void *)plc->passwd, (void *)phone->conf.passwd, sizeof(phone->conf.passwd));
	
	//清空各个session
	ucla_all_session_clean();
	//替换session
	ucla_all_session_query();
}

static void ucla_phone_switch_outdo()
{

}

static void ucla_phone_switch_timer_proc()
{

}

//modify
static int ucla_phone_passwd_modify_die(cl_thread_t *t)
{
//	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);

	UCLA_ENTER();	
	plc->t_phone_die = NULL;

	// TODO:做些提示。。。。。
	event_push_err(plc->callback, LA_PHONE_PASSWD_MODIFY_FAILED, 0, plc->callback_handle, 0);

	ucla_phone_set_status(UCLA_PHONE_IDLE);
	return 0;
}

void phome_passwd_modify_ok()
{
	event_push_err(plc->callback, LA_PHONE_PASSWD_MODIFY_SUCCESS, 0, plc->callback_handle, 0);
	ucla_phone_set_status(UCLA_PHONE_IDLE);	
}

static void do_phone_passwd_modify()
{
	ucla_session_t *s;
	pkt_t *pkt;
	uc_app_user_phone_modify_t *pm;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_debug("not found phone session or sessin is not ok\n");		
		goto err;
	}

	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_phone_modify_t),
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		goto err;
	}

	pm = get_ucp_payload(pkt, uc_app_user_phone_modify_t);
	pm ->action = UCAU_MODIFY;
	pm->user_id = htonl(s->user_id);
	memcpy((void *)pm->user_name, (void *)plc->user_name, sizeof(pm->user_name));
	memcpy((void *)pm->passwd, (void *)plc->passwd, sizeof(pm->passwd));
	ucla_request_add(s, pkt);

	return;
	
err:
	event_push_err(plc->callback, LA_PHONE_PASSWD_MODIFY_FAILED, 0, plc->callback_handle, 0);
	ucla_phone_set_status(UCLA_PHONE_IDLE);
}

static void ucla_phone_passwd_modify_indo()
{
	ucla_phone_off_all_timer();

	plc->phone_die = 10;

	CL_THREAD_OFF(plc->t_phone_die);
	CL_THREAD_TIMER_ON(MASTER, plc->t_phone_die,ucla_phone_passwd_modify_die, NULL, TIME_N_SECOND(plc->phone_die));
	do_phone_passwd_modify();
}

static void ucla_phone_passwd_modify_outdo()
{

}

static void ucla_phone_passwd_modify_timer_proc()
{

}

ucc_proc_t ucla_phone_proc[UCLA_PHONE_MAX] = {
	{"IDLE", ucla_phone_idle_indo, ucla_phone_idle_outdo, ucla_phone_idle_timer_proc},
	{"create", ucla_phone_create_indo, ucla_phone_create_outdo, ucla_phone_create_timer_proc},
	{"login", ucla_phone_login_indo, ucla_phone_login_outdo, ucla_phone_login_timer_proc},
	{"logout", ucla_phone_logout_indo, ucla_phone_logout_outdo, ucla_phone_logout_timer_proc},
	{"switch", ucla_phone_switch_indo, ucla_phone_switch_outdo, ucla_phone_switch_timer_proc},
	{"del", ucla_phone_logout_indo, ucla_phone_logout_outdo, ucla_phone_logout_timer_proc},
	{"passwd_modify", ucla_phone_passwd_modify_indo, ucla_phone_passwd_modify_outdo, ucla_phone_passwd_modify_timer_proc},
};

static int ucla_phone_timer(cl_thread_t *t)
{
	plc->t_phone_timer = NULL;
	CL_THREAD_TIMER_ON(MASTER, plc->t_phone_timer,ucla_phone_timer, NULL, TIME_N_SECOND(plc->phone_timer));

	ucla_phone_proc[plc->phone_status].proc_pkt();

	return 0;
}

static void ucla_phone_timer_reset()
{
	UCLA_ENTER();
	CL_THREAD_OFF(plc->t_phone_timer);
	CL_THREAD_TIMER_ON(MASTER, plc->t_phone_timer,ucla_phone_timer, NULL, TIME_N_SECOND(plc->phone_timer));
}

void ucla_phone_set_status(int status)
{
	int prev_satus = plc->phone_status;
    
	if (status >= UCLA_PHONE_MAX) {
		//log_debug("ucla_phone_set_status unknow new status = %d\n", status);
		return;
	}

	plc->phone_status = status;
    
	log_debug("%s modify status from %s to %s\n",
		__FUNCTION__, ucla_phone_proc[prev_satus].name, ucla_phone_proc[status].name);

	ucla_phone_proc[prev_satus].on_out();
	ucla_phone_proc[status].on_into();
}
/***********************************通用函数**************************************************************/
bool ucla_session_is_ok(ucla_session_t *s)
{
	if (!s) {
		return false;
	}
	
	return (s->status == UCLAS_ESTABLISH);
}

int ucla_get_all_home_num()
{
	int num = 0;
	la_user_t *puser, *n;
	la_home_t *phome, *homen;

	cl_lock(&cl_priv->mutex);	
	stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
		stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
			num++;
		}
	}	
	cl_unlock(&cl_priv->mutex);	

	return num;
}

la_phone_t *ucla_find_phone_by_name(char *name)
{
	la_phone_t *pp, *ppn;
	
	if (!name) {
		return NULL;
	}

	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		if (strcmp(name, pp->conf.user_name) == 0) {
			return pp;
		}
	}

	return NULL;
}

la_user_t *ucla_find_user_by_id(u_int32_t user_id)
{
	la_user_t *puser, *n, *user_ret = NULL;

	cl_lock(&cl_priv->mutex);	
	stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
		if (puser->conf.user_id == user_id) {
			user_ret = puser;
			goto done;
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);	
	
	return user_ret;
}

la_home_t *ucla_find_home_by_id(u_int32_t home_id)
{
	la_user_t *puser, *n;
	la_home_t *phome, *homen, *home_ret = NULL;

	cl_lock(&cl_priv->mutex);	
	stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
		stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
			if (phome->conf.home_id == home_id) {
				home_ret = phome;
				goto done;
			}
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);	

	return home_ret;
}

la_home_t *ucla_find_home_by_handle(cl_handle_t handle)
{
	la_user_t *puser, *n;
	la_home_t *phome, *homen, *home_ret = NULL;

	cl_lock(&cl_priv->mutex);	
	stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
		stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
			if (phome->handle == handle) {
				home_ret = phome;
				goto done;
			}
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);		

	return home_ret;
}

la_rule_t *ucla_find_rule_by_id(la_home_t *phome, u_int32_t rule_id)
{
	la_rule_t *prule, *rulen, *rule_ret = NULL;

	stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
		if (prule->rule_id == rule_id) {
			rule_ret = prule;
			goto done;
		}
	}

done:	

	return rule_ret;
}

ucla_session_t *la_get_session_by_handle(cl_handle_t handle)
{
	la_user_t *puser, *n;
	la_home_t *phome, *homen;
	u_int32_t user_id = 0;
	ucla_session_t *s_ret = NULL, *ps, *sn;

	if (handle == 0) {
		return NULL;
	}

	if (stlc_list_empty(&cl_priv->la_user)) {
		return NULL;
	}

	cl_lock(&cl_priv->mutex);		
	//handle有可能是userhandle,也可能是homehandle，注意处理下
	stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
		if (puser->handle == handle) {
			user_id = puser->conf.user_id;
			s_ret = puser->session_ptr;
			goto done;
		}
		stlc_list_for_each_entry_safe(la_home_t, phome, homen,&puser->home_link, link) {
			if (phome->handle == handle) {
				user_id = puser->conf.user_id;
				s_ret = phome->session_ptr;
				goto done;
			}
		}
	}	

done:
	cl_unlock(&cl_priv->mutex);	

	if (s_ret) {
		return s_ret;
	}
	s_ret = stlc_list_entry(plc->server_client.next, ucla_session_t, link);
	stlc_list_for_each_entry_safe(ucla_session_t, ps, sn, &plc->server_client, link) {
		if (ps->user_id == user_id) {
			s_ret = ps;
			break;
		}
	}

	return s_ret;
}

ucla_session_t *la_get_session_by_handle2(cl_handle_t handle)
{
	la_user_t *puser, *n;
	la_home_t *phome, *homen;
	u_int32_t user_id = 0;
	ucla_session_t *s_ret = NULL, *ps, *sn;

	if (stlc_list_empty(&cl_priv->la_user)) {
		return NULL;
	}

	cl_lock(&cl_priv->mutex);		
	//handle有可能是userhandle,也可能是homehandle，注意处理下
	stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
		if (puser->handle == handle) {
			user_id = puser->conf.user_id;
			s_ret = puser->session_ptr;
			goto done;
		}
		stlc_list_for_each_entry_safe(la_home_t, phome, homen,&puser->home_link, link) {
			if (phome->handle == handle) {
				user_id = puser->conf.user_id;
				s_ret = phome->session_ptr;
				goto done;
			}
		}
	}	

done:
	cl_unlock(&cl_priv->mutex);

	if (s_ret) {
		return s_ret;
	}
	stlc_list_for_each_entry_safe(ucla_session_t, ps, sn, &plc->server_client, link) {
		if (ps->user_id == user_id) {
			return ps;
		}
		
	}

	return NULL;
}

la_member_t *ucla_find_member_by_sn(ucla_session_t *s, u_int64_t sn)
{
	la_user_t *puser;
	la_home_t *phome, *phomen;
	la_member_t *pmem, *pmemn;
	
	if (!s) {
		return NULL;
	}

	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return NULL;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, phomen, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_member_t, pmem, pmemn, &phome->member_link, link) {
			if (pmem->conf.sn == sn) {
				return pmem;
			}
		}
	}

	return NULL;
}

int ucla_get_user_num(u_int64_t sn)
{
	int num = 0;
	la_user_t *puser;
	la_home_t *phome, *phomen;
	la_member_t *pmem, *pmemn;
	ucla_session_t *s, *n;
	
	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		puser = ucla_find_user_by_id(s->user_id);
		if (!puser) {
			continue;
		}
		stlc_list_for_each_entry_safe(la_home_t, phome, phomen, &puser->home_link, link) {
			stlc_list_for_each_entry_safe(la_member_t, pmem, pmemn, &phome->member_link, link) {
				log_debug("la_test ucla_get_user_num num locsn=%"PRIu64" =%llu\n", pmem->conf.sn, sn);
				if (pmem->conf.sn == sn) {
					num++;
				}
			}
		}
	}

	return num;
}

la_member_t *ucla_find_member_by_sn_from_home(la_home_t *phome, u_int64_t sn)
{
	la_member_t *pmem, *pmemn;
	
	if (!phome) {
		return NULL;
	}
	stlc_list_for_each_entry_safe(la_member_t, pmem, pmemn, &phome->member_link, link) {
		if (pmem->conf.sn == sn) {
			return pmem;
		}
	}

	return NULL;
}

u_int8_t ucla_home_is_online(u_int32_t handle)
{
	ucla_session_t *s;

	s = la_get_session_by_handle(handle);
	if (!s) {
		return 0;
	}

	if (ucla_session_is_ok(s)) {
		return 1;
	}

	return 0;
}

la_user_t *ucla_user_new()
{
	la_user_t *puser = NULL;

	puser = cl_calloc(sizeof(*puser), 1);

	if (!puser) {
		return NULL;
	}

	STLC_INIT_LIST_HEAD(&puser->link);
	STLC_INIT_LIST_HEAD(&puser->home_link);
	puser->handle = handle_create(HDLT_LINKAGE);

	return puser;
}

la_phone_t *ucla_phone_new()
{
	la_phone_t *phone = NULL;

	phone = cl_calloc(sizeof(*phone), 1);

	if (!phone) {
		return NULL;
	}

	STLC_INIT_LIST_HEAD(&phone->link);

	return phone;
}

la_home_t *ucla_home_new()
{
	la_home_t *phome = NULL;

	phome = cl_calloc(sizeof(*phome), 1);

	if (!phome) {
		return NULL;
	}

	phome->handle = handle_create(HDLT_LINKAGE);
	STLC_INIT_LIST_HEAD(&phome->link);
	STLC_INIT_LIST_HEAD(&phome->member_link);
	STLC_INIT_LIST_HEAD(&phome->rule_link);
	STLC_INIT_LIST_HEAD(&phome->label_link);
	STLC_INIT_LIST_HEAD(&phome->dict_link);
	
	return phome;	
}

la_member_t *ucla_member_new()
{
	la_member_t *pmem = NULL;

	pmem = cl_calloc(sizeof(*pmem), 1);

	if (pmem) {
		STLC_INIT_LIST_HEAD(&pmem->link);
	}

	return pmem;	
}

la_label_t *ucla_label_new()
{
	la_label_t *plabel = NULL;

	plabel = cl_calloc(sizeof(*plabel), 1);
	if (plabel) {
		STLC_INIT_LIST_HEAD(&plabel->link);
	} 

	return plabel;
}

la_rule_t *ucla_rule_new()
{
	la_rule_t *prule = NULL;

	prule = cl_calloc(sizeof(*prule), 1);
	if (prule) {
		STLC_INIT_LIST_HEAD(&prule->link);
	}

	return prule;
}

/********************************日志相关*****************************************/
la_log_home_rule_change_t *ucla_log_rule_new()
{
	la_log_home_rule_change_t *plog = NULL;

	plog = cl_calloc(sizeof(*plog), 1);
	if (plog) {
		STLC_INIT_LIST_HEAD(&plog->link);
		STLC_INIT_LIST_HEAD(&plog->log_list);
	}

	return plog;
}

la_log_home_rule_node_t *ucla_log_rule_node_new()
{
	la_log_home_rule_node_t *plog = NULL;

	plog = cl_calloc(sizeof(*plog), 1);
	if (plog) {
		STLC_INIT_LIST_HEAD(&plog->link);
	}

	return plog;
}

la_log_home_rule_change_t *ucla_log_rule_find_by_homeid(u_int32_t home_id)
{
	la_log_home_rule_change_t *p, *pn;

	stlc_list_for_each_entry_safe(la_log_home_rule_change_t, 
		p, pn, &plc->g_rule_log_list, link) {
		if (p->home_id == home_id) {
			return p;
		}
	}

	return NULL;
}

la_log_home_member_change_t *ucla_log_mem_new()
{
	la_log_home_member_change_t *plog = NULL;

	plog = cl_calloc(sizeof(*plog), 1);
	if (plog) {
		STLC_INIT_LIST_HEAD(&plog->link);
		STLC_INIT_LIST_HEAD(&plog->log_list);
	}

	return plog;
}

la_log_home_member_node_t *ucla_log_mem_node_new()
{
	la_log_home_member_node_t *plog = NULL;

	plog = cl_calloc(sizeof(*plog), 1);
	if (plog) {
		STLC_INIT_LIST_HEAD(&plog->link);
	}

	return plog;
}

la_log_home_member_change_t *ucla_log_mem_find_by_homeid(u_int32_t home_id)
{
	la_log_home_member_change_t *p, *pn;

	stlc_list_for_each_entry_safe(la_log_home_member_change_t, 
		p, pn, &plc->g_member_log_list, link) {
		if (p->home_id == home_id) {
			return p;
		}
	}

	return NULL;
}

/*********************************************************************/

static int la_conf_read_from_file(u_int8_t *pbuf, int max_len, char *file)
{
	FILE *fp = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	struct stat st_l;
	int ret = 0;

	if ((!pbuf)||
		(!file) ||
		(!cl_priv->priv_dir)) {
		UCLA_ENTER();
        return 0;
    }

	sprintf(fn, "%s/%s", cl_priv->priv_dir, file);

	//判断下大小
	if (stat(fn, &st_l) != 0) {
		//UCLA_ENTER();
		//log_debug("la_conf_read_from_file stat failed\n");
		return 0;
	}

	if (st_l.st_size > max_len) {
		//UCLA_ENTER();
		//log_debug("%s file is too big\n", fn);
		return 0;
	}

	
	fp = fopen(fn, "rb");
	if (!fp) {
		return 0;
	}

	ret = (int)fread((void *)pbuf, 1, (size_t)st_l.st_size, fp);
	fclose(fp);		

	return ret;
}

static int la_conf_dec(u_int8_t *pbuf, int data_len)
{
//	int len_ret = 0;
	int pad_len;
	MD5_CTX ctx;
    u_int8_t result[16] = {0};
	int crc = 0;
	ucla_conf_save_t *pconf = (ucla_conf_save_t *)pbuf;

	if (data_len <= AES128_EKY_LEN) {
		//UCLA_ENTER();
		//log_debug("err len=%u\n", data_len);
		return 0;
	}

	if ((data_len %AES128_EKY_LEN) != 0) {
		//UCLA_ENTER();
		//log_debug("err len=%u\n", data_len);
		return 0;
	}

	dec_block((u_int8_t *)pbuf, data_len, plc->conf_aes_key);
	pad_len = pbuf[data_len - 1];
	if (pad_len > AES128_EKY_LEN || pad_len <= 0) {
		//UCLA_ENTER();
		//log_debug("Drop bad packet: encrypt pad=%d\n", pad_len);
		return 0;
	}

	data_len -= pad_len;

	crc = pconf->crc;

	pconf->crc = 0;
	
	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)pconf, data_len);
	MD5Final(result, &ctx);
	memcpy(&pconf->crc, &result[12], 4);

	if (crc != pconf->crc) {
		log_debug("crc check failed crcn=%08x crcl=%08x\n", crc, pconf->crc);
		return 0;
	}

	if (pconf->magic != UCLA_CONF_SAVE_MAGIC) {
		log_debug("err magic=%08x lmagic=%08x\n", pconf->magic, UCLA_CONF_SAVE_MAGIC);
		return 0;
	}

	return data_len;
}

int la_get_file_size(ucla_session_t *s, char *file)
{
	struct stat st_l;
	char fn[UCLA_FILE_PATH_LEN];

	sprintf(fn, "%s/%s/%s", 
		cl_priv->priv_dir, s->doname, file);
	if (stat(fn, &st_l) != 0) {
		return 0;
	}

	return (int)(st_l.st_size);
}

static int la_get_member_conf_size(ucla_session_t *s, char *file)
{
	return la_get_file_size(s, file);
}

static bool _la_member_init(ucla_session_t *s, char *file)
{
	la_member_conf_t *pmembercf;
	la_member_t *pmember;
	la_home_t *phome;

	int i;
	int read_len = 0;
	int index = 0;
	int len = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	len = la_get_member_conf_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len, fn);
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}

	//家庭可以没有设备
	ret = true;
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*pmembercf)) <= read_len); i++) {
		pmembercf = (la_member_conf_t *)&pconf->data[index];
		index += sizeof(*pmembercf);
		
		pmember = ucla_member_new();
		if (!pmember) {
			UCLA_ENTER();
			goto done;
		}

		memcpy((void *)&pmember->conf, (void *)pmembercf, sizeof(*pmembercf));
		phome = ucla_find_home_by_id(pmember->conf.home_id);
		if (phome) {
			phome->session_ptr = s;
			//这里多处理一下，目的是过滤掉相同的sn，以前3.1前的版本有个bug，一个圈子重复添加相同sn设备可能出现多个
			if (ucla_find_member_by_sn_from_home(phome, pmember->conf.sn)) {
				cl_free(pmember);
				continue;
			}
			stlc_list_add_tail(&pmember->link, &phome->member_link);
			la_user_login(phome->conf.home_id, pmember->conf.sn, pmember->conf.dev_passwd);
		} else {
			cl_free(pmember);	
		}
	}

done:
	SAFE_FREE(read_buf);

	return ret;
}

static void la_member_init(ucla_session_t *s, char *user_dir)
{
	if (_la_member_init(s, UCLA_LA_MEMBER_FILE)) {
		log_debug("%s get member conf from %s\n", s->doname,  UCLA_LA_MEMBER_FILE);
		return;
	}

	if (_la_member_init(s, UCLA_LA_MEMBER_FILE_BACK)) {
		log_debug("%s get member conf from %s\n", s->doname, UCLA_LA_MEMBER_FILE_BACK);
		return;
	}

	//log_debug("there is no member conf\n");
}

static int la_get_label_conf_size(ucla_session_t *s, char *file)
{
	return la_get_file_size(s, file);
}

static bool _la_label_init(ucla_session_t *s, char *file)
{
	la_label_conf_t *plc;
	la_label_t *pl;
	la_home_t *phome;

	int i;
	int read_len = 0;
	int len1 = 0;
	int index = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	bool ret = false;

	len1 = la_get_label_conf_size(s, file);
	if (0 == len1) {
		return false;
	}
	read_buf = cl_calloc(len1, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len1, fn);
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}

	//家庭可以没有标签
	ret = true;
	read_len -= sizeof(*pconf);

	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*plc)) <= read_len); i++) {
		plc = (la_label_conf_t *)&pconf->data[index];
		index += sizeof(*plc);
		
		pl = ucla_label_new();
		if (!pl) {
			UCLA_ENTER();
			goto done;
		}

		memcpy((void *)&pl->conf, (void *)plc, sizeof(*plc));
		if (plc->sn_num) {
			len1 = (int)(plc->sn_num*sizeof(u_int64_t));
			
			if ((index + len1) > read_len) {
				log_info("err len=%u readlen=%u\n", 
					(index + len1), read_len);
				goto done;
			}
			pl->p_sn = cl_calloc(len1, 1);
			memcpy((void *)pl->p_sn, (void *)&pconf->data[index], len1);
			index += len1;
		}
		phome = ucla_find_home_by_id(plc->home_id);
		if (phome) {
			//这里多处理一下，目的是过滤掉相同的标签
			if (ucla_find_home_label_by_id(phome, plc->id)) {
				ucla_label_free(pl);
				continue;
			}
			stlc_list_add_tail(&pl->link, &phome->label_link);
			log_debug("%s id=%u name=%s num=%u\n", __FUNCTION__, pl->conf.id, pl->conf.name, pl->conf.sn_num);
		} else {
			ucla_label_free(pl);	
		}
	}

done:
	SAFE_FREE(read_buf);

	return ret;
}


static void la_label_init(ucla_session_t *s)
{
	if (_la_label_init(s, UCLA_LA_LABEL_FILE)) {
		log_debug("%s get label conf from %s\n", s->doname, UCLA_LA_LABEL_FILE);
		return;
	}

	if (_la_label_init(s, UCLA_LA_LABEL_FILE_BACK)) {
		log_debug("%s get label conf from %s\n", s->doname, UCLA_LA_LABEL_FILE_BACK);
		return;
	}

	//log_debug("there is no label conf\n");
}

static int la_get_rule_conf_size(ucla_session_t *s, char *file)
{
	return la_get_file_size(s, file);
}

static bool _la_rule_init(ucla_session_t *s, char *file)
{
	la_rule_conf_t *prulec;
	la_rule_t *prule;
	la_home_t *phome;

	int i;
	int read_len = 0;
	int index = 0;
	int len = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	len = la_get_rule_conf_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len, fn);
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}

	UCLA_ENTER();
	ret = true;
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*prulec)) <= read_len); i++) {
		prulec = (la_rule_conf_t *)&pconf->data[index];
		index += sizeof(*prulec);
		
		prule = ucla_rule_new();
		if (!prule) {
			goto done;
		}

		prule->flag = prulec->flag;
		prule->rule_len = prulec->rule_len;
		prule->rule_id = prulec->rule_id;
		prule->last_exec_time = prulec->last_exec_time;
		prule->enable = prulec->enable;
		prule->state = prulec->state;
		if ((index + (int)prule->rule_len) > read_len) {
			log_debug("%s err index + prule->rule_len=%u readlen=%u\n", 
				__FUNCTION__, index + len, read_len);
			break;
		}
		if (prulec->rule_len) {
			prule->rule = cl_calloc(prulec->rule_len + 10, 1);
			if (prule->rule) {
				memcpy(prule->rule, &pconf->data[index], prulec->rule_len);
				index += prulec->rule_len;
			} else {
				ucla_rule_free(prule);
				goto done;
			}
		}
		phome = ucla_find_home_by_id(prulec->home_id);
		if (phome) {
			phome->session_ptr = s;
			phome->rules_is_cache = true;
			if (ucla_find_rule_by_id(phome, prule->rule_id)) {
				ucla_rule_free(prule);
				continue;
			}
			stlc_list_add_tail(&prule->link, &phome->rule_link);
			//log_debug("cache init homeid=%u ruldid=%u \n", prulec->home_id,  prule->rule_id);
		} else {
			ucla_rule_free(prule);	
		}
	}

done:
	SAFE_FREE(read_buf);
	UCLA_EXIT();
	
	return ret;
}

static void la_rule_init(ucla_session_t *s)
{
	if (_la_rule_init(s, UCLA_LA_RULE_FILE)) {
		log_debug("%s get  conf from %s\n", s->doname,  UCLA_LA_RULE_FILE);
		return;
	}

	if (_la_rule_init(s, UCLA_LA_RULE_FILE_BACK)) {
		log_debug("%s get  conf from %s\n", s->doname, UCLA_LA_RULE_FILE_BACK);
		return;
	}
}

static bool _la_share_init(ucla_session_t *s, char *file)
{
	la_share_conf_t *psc;
	la_share_desc_t *pdesc;
	la_home_t *phome;

	int i;
	int read_len = 0;
	int index = 0;
	int len = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	len = la_get_file_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len, fn);
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}

	UCLA_ENTER();
	ret = true;
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*psc)) <= read_len); i++) {
		psc = (la_share_conf_t *)&pconf->data[index];
		index += sizeof(*psc);
		
		if (psc->share_num == 0) {
			continue;
		}
		len = psc->share_num*sizeof(*pdesc);
		phome = ucla_find_home_by_id(psc->home_id);
		if (!phome) {
			index += len;
			continue;
		}
		SAFE_FREE(phome->share_desc_array);
		phome->share_desc_num = 0;
		phome->share_desc_array = cl_calloc(len, 1);
		if (!phome->share_desc_array) {
			continue;
		}
		phome->share_desc_num = psc->share_num;
		memcpy((void *)phome->share_desc_array, (void *)psc->share, len);
		index += len;
	}

done:
	SAFE_FREE(read_buf);
	UCLA_EXIT();
	
	return ret;
}

static void la_share_init(ucla_session_t *s)
{
	if (_la_share_init(s, UCLA_LA_SHARE_FILE)) {
		log_debug("%s get  conf from %s\n", s->doname,  UCLA_LA_SHARE_FILE);
		return;
	}

	if (_la_share_init(s, UCLA_LA_SHARE_FILE_BACK)) {
		log_debug("%s get  conf from %s\n", s->doname, UCLA_LA_SHARE_FILE_BACK);
		return;
	}
}

static bool _la_sc_init(ucla_session_t *s, char *file)
{
	la_sc_conf_t *psc;
	la_home_t *phome;

	int i;
	int read_len = 0;
	int index = 0;
	int len = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	len = la_get_file_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len, fn);
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}

	UCLA_ENTER();
	ret = true;
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*psc)) <= read_len); i++) {
		psc = (la_sc_conf_t *)&pconf->data[index];
		index += sizeof(*psc);
		
		phome = ucla_find_home_by_id(psc->home_id);
		if (!phome) {
			continue;
		}
		memcpy((void *)phome->la_sc_key, (void *)psc->la_sc_key, sizeof(phome->la_sc_key));
	}

done:
	SAFE_FREE(read_buf);
	UCLA_EXIT();
	
	return ret;
}

static void la_sc_init(ucla_session_t *s)
{
	if (_la_sc_init(s, UCLA_LA_SC_FILE)) {
		log_debug("%s get  conf from %s\n", s->doname,  UCLA_LA_SC_FILE);
		return;
	}

	if (_la_sc_init(s, UCLA_LA_SC_FILE_BACK)) {
		log_debug("%s get  conf from %s\n", s->doname, UCLA_LA_SC_FILE_BACK);
		return;
	}
}

static void la_cap_init(ucla_session_t *s)
{
	int i;
	u_int32_t time = 0;
	u_int32_t num = 0;
	char buff[1024];
	char str[1024];
	char fn[UCLA_FILE_PATH_LEN];
	FILE *fp = NULL;

	if (!s || 
		!cl_priv->priv_dir) {
		return;
	}

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_FILE);
	fp = fopen(fn, "r");
	if (!fp) {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_FILE_BACK);
		fp = fopen(fn, "r");
		if (!fp) {
			return;
		}
	}

	if (fgets(buff, sizeof(buff) - 1, fp)) {
		if (sscanf(buff, "time=%u num=%u", &time, &num) != 2) {
			goto end;
		}
		if (num == 0) {
			goto end;
		}
		plc->cap_num = 0;
		plc->cap_lastmodifytime = time;
		SAFE_FREE(plc->cap_array);
		plc->cap_array = cl_calloc((num * sizeof(u_int8_t *)), 1);
		if (!plc->cap_array) {
			goto end;
		}
		plc->cap_num = (u_int32_t)num;
	}
	i = 0;
	while(fgets(buff, sizeof(buff) - 1, fp)) {
		if (i >= plc->cap_num) {
			goto end;
		}
		if (sscanf(buff, "url=%s", str) != 1) {
			plc->cap_array[i++] = (u_int8_t *)cl_strdup("");
		} else {
			plc->cap_array[i++] = (u_int8_t *)cl_strdup(str);
		}
	}

end:	
	if (fp) {
		fclose(fp);
	}
}

static void la_cap_custom_init(ucla_session_t *s)
{
	int i;
	u_int32_t time = 0;
	u_int32_t num = 0;
	char buff[1024];
	char str[1024];
	char fn[UCLA_FILE_PATH_LEN];
	FILE *fp = NULL;

	if (!s || 
		!cl_priv->priv_dir) {
		return;
	}
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_CUSTOM_FILE);
	fp = fopen(fn, "r");
	if (!fp) {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_CUSTOM_FILE_BACK);
		fp = fopen(fn, "r");
		if (!fp) {
			return;
		}
	}
	if (fgets(buff, sizeof(buff) - 1, fp)) {
		if (sscanf(buff, "time=%u num=%u", &time, &num) != 2) {
			goto end;
		}
		if (num == 0) {
			goto end;
		}
		plc->cap_custom_num = 0;
		plc->cap_custom_lastmodifytime = time;
		SAFE_FREE(plc->cap_custom_array);
		plc->cap_custom_array = cl_calloc((num * sizeof(u_int8_t *)), 1);
		if (!plc->cap_custom_array) {
			goto end;
		}
		plc->cap_custom_num = (u_int32_t)num;
	}
	i = 0;
	while(fgets(buff, sizeof(buff) - 1, fp)) {
		if (i >= plc->cap_custom_num) {
			goto end;
		}
		if (sscanf(buff, "url=%s", str) != 1) {
			plc->cap_custom_array[i++] = (u_int8_t *)cl_strdup("");
		} else {
			plc->cap_custom_array[i++] = (u_int8_t *)cl_strdup(str);
		}
	}

end:	
	if (fp) {
		fclose(fp);
	}
}

static int la_get_template_conf_size(ucla_session_t *s, char *file)
{
	return la_get_file_size(s, file);
}

static bool _la_template_init(ucla_session_t *s, char *file)
{
	int i;
	int read_len = 0;
	int index = 0;
	int len = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	len = la_get_template_conf_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;
	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len, fn);
	read_len = la_conf_dec(read_buf, read_len);
	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}
	//UCLA_ENTER();
	ret = true;
	read_len -= sizeof(*pconf);
	plc->url_num = (u_int16_t )pconf->count;
	plc->url_array = cl_calloc(plc->url_num * sizeof(u_int8_t *), 1);
	//memdumpone("_la_template_init", pconf->data, read_len);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(len)) <= read_len); i++) {
		len = *(int *)&pconf->data[index];
		index += (int)sizeof(len);
		if ((index + len) > read_len) {
			log_debug("%s err index + len=%u readlen=%u\n", 
				__FUNCTION__, index + len, read_len);
			break;
		}
		plc->url_array[i] = cl_calloc(len + 10, 1);
		if (!plc->url_array[i]) {
			break;
		}
		memcpy(plc->url_array[i], &pconf->data[index], len);
		index += len;
		//log_debug("cache init len=%u rul[%d]=%s\n", 
			//len, i, plc->url_array[i]);
	}
	plc->url_num = i;

done:
	SAFE_FREE(read_buf);
	//UCLA_EXIT();
	
	return ret;
}

static void la_template_init(ucla_session_t *s)
{
	if (_la_template_init(s, UCLA_LA_TEMPLATE_FILE)) {
		log_debug("%s get  conf from %s\n", s->doname,  UCLA_LA_TEMPLATE_FILE);
		return;
	}

	if (_la_template_init(s, UCLA_LA_TEMPLATE_FILE_BACK)) {
		log_debug("%s get  conf from %s\n", s->doname, UCLA_LA_TEMPLATE_FILE_BACK);
		return;
	}	
}

static int la_get_home_conf_size(ucla_session_t *s, char *file)
{
	return la_get_file_size(s, file);
}

static bool _la_home_init(ucla_session_t *s, char *file)
{
	la_home_conf_t *phomecf;
	la_home_t *phome;
	la_user_t *puser;

	int read_len = 0;
	int index = 0;
	int len = 0;
	int i;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	if (!s || !file || !cl_priv->priv_dir) {
		return false;
	}
	len = la_get_home_conf_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;
	//找到所属user
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len, fn);
	read_len = la_conf_dec(read_buf, read_len);	

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		UCLA_ENTER();
		goto done;
	}
	
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*phomecf)) <= read_len); i++) {
		phomecf = (la_home_conf_t *)&pconf->data[index];
		index += sizeof(*phomecf);

		//判断有没有相同的圈子id
		if (ucla_find_home_by_id(phomecf->home_id)) {
			continue;
		}
		phome = ucla_home_new();
		if (!phome) {
			goto done;
		}
		memcpy((void *)&phome->conf, (void *)phomecf, sizeof(*phomecf));
		if (ucla_is_def_home(phome)) {
			ucla_set_def_home(s, phome);
			plc->cur_home_id = phome->conf.home_id;
			log_debug("%s there is def home exsit !!!!!!!!!!!!!!\n", 
				s->doname);
			//现在服务器已经同步了，不用处理多服务器问题了，为了兼容，把默认圈子用户提取出来
			plc->need_switch_session = s;
		}
		plc->has_any_home = true;
		phome->session_ptr = s;		
		stlc_list_add_tail(&phome->link, &puser->home_link);
		ret = true;
	}
	
done:
	SAFE_FREE(read_buf);

	return ret;
}

static void la_home_init(ucla_session_t *s, char *user_dir)
{
	if (_la_home_init(s, UCLA_LA_HOME_FILE)) {
		log_debug("%s get home conf from %s\n", s->doname, UCLA_LA_HOME_FILE);
		return;
	}

	if (_la_home_init(s, UCLA_LA_HOME_FILE_BACK)) {
		log_debug("%s get home conf from %s\n", s->doname, UCLA_LA_HOME_FILE_BACK);
		return;
	}

	//log_debug("there is no home conf\n");
}

static int la_get_user_conf_size(ucla_session_t *s, char *file)
{
	return la_get_file_size(s, file);
}

bool ucla_is_phone_user(la_user_t *puser)
{
	if (puser && 
		mem_is_all_zero((u_int8_t *)puser->conf.uuid, 
		APP_USER_UUID_NAME_LEN)) {
		return true;
	}

	return false;
}

bool ucla_is_phone_session(ucla_session_t *s)
{
	if (!s) {
		return false;
	}
	
	return mem_is_all_zero((u_int8_t *)s->uuid, MAX_UUID_BIN_LEN);
}

static bool _la_user_init(ucla_session_t *s, char *file)
{
	la_user_conf_t *pusercf;
	la_user_t *puser;

	int read_len = 0;
	int len = 0;
	int index = 0;
	int i;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	bool ret = false;

	if (!s || !file || !cl_priv->priv_dir) {
		return false;
	}

	len = la_get_user_conf_size(s, file);
	if (0 == len) {
		return false;
	}
	read_buf = cl_calloc(len, 1);
	if (!read_buf) {
		return false;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	sprintf(fn, "%s/%s", s->doname, file);
	read_len = la_conf_read_from_file(read_buf, len,fn);
	read_len = la_conf_dec(read_buf, read_len);	
	
	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*pusercf)) <= read_len); i++) {
		pusercf = (la_user_conf_t *)&pconf->data[index];
		puser = ucla_user_new();
		if (!puser) {
			goto done;
		}
		memcpy((void *)&puser->conf, (void *)pusercf, sizeof(*pusercf));
		log_debug("%s la_user_init name=%s\n", s->doname, puser->conf.user_name);
		if (ucla_is_phone_user(puser)) {
			plc->has_phone = true;
			memcpy(plc->p_user_name, puser->conf.user_name, sizeof(plc->p_user_name));
			memcpy(plc->p_passwd, puser->conf.passwd, sizeof(plc->p_passwd));
			memcpy(plc->p_passwd_cmp, puser->conf.passwd, sizeof(plc->p_passwd_cmp));
		}
		puser->session_ptr = s;
		stlc_list_add_tail(&puser->link, &cl_priv->la_user);
		s->has_user = true;
		s->user_id = puser->conf.user_id;
		memcpy((void *)s->username, (void *)puser->conf.user_name, sizeof(s->username));
		memcpy((void *)s->key, (void *)puser->conf.passwd, sizeof(s->key));
		memcpy(s->uuid, puser->conf.uuid, sizeof(s->uuid));
		//memdumpone("la_user_init uuid", s->uuid, 16);
		//g_debug("***%s flag=%u username=%s is_phone=%u plc->p_user_name=%s user_name%s userid=%u\n", 
			//>doname, puser->conf.flag, s->username, s->is_phone, plc->p_user_name, puser->conf.user_name, 
			//ser->conf.user_id);
		//一个服务器只需要一个用户
		ret = true;
		break;
	}

done:
	SAFE_FREE(read_buf);

	return ret;
}

static void la_user_init(ucla_session_t *s, char *user_dir)
{
	if (_la_user_init(s, UCLA_LA_USER_FILE)) {
		log_debug("%s get user conf from %s\n", s->doname, UCLA_LA_USER_FILE);
		return;
	}

	if (_la_user_init(s, UCLA_LA_USER_FILE_BACK)) {
		log_debug("%s get user conf from %s\n", s->doname, UCLA_LA_USER_FILE_BACK);
		return;
	}

	//log_debug("there is no user conf\n");
}

void la_user_login(u_int32_t home_id, u_int64_t sn, u_int8_t *passwd_md5)
{
	user_t *user;
	char buff[1024];
	la_dev_misc_info_t *pdmi;

	sprintf(buff, "%"PRIu64"", sn);
	user = user_create(false, buff, (char *)passwd_md5,NULL, NULL, 0, plc->callback, plc->callback_handle, false, true);
	if (user) {
		user->home_id = home_id;
		pdmi = la_dev_type_find_by_sn(sn);
		if (pdmi) {
			user->ds_type = pdmi->conf.ds_type;
			user->sub_type = pdmi->conf.sub_type;
			user->ext_type = pdmi->conf.ext_type;
			user->dev_ver_is_valid = pdmi->conf.dev_ver_is_valid;
			user->dev_ver_is_too_low = pdmi->conf.dev_ver_is_too_low;
		}
	} else {
		log_debug("la_user_login sn login failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n", buff);
	}
}

void ucla_server_doname_save()
{
	char fn[256];
	char fn_back[256];
	FILE *fp;
	ucla_session_t *s , *n;

	if (cl_priv->priv_dir == NULL) {
        return;
    }

    sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_LA_SERVER_FILE);
    sprintf(fn_back, "%s/%s", cl_priv->priv_dir, UCLA_LA_SERVER_FILE_BACK);

	fp = fopen(fn_back, "w");
	if (!fp) {
		return;
	}

	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		sprintf(fn, "%s\n", s->doname);
		fputs(fn, fp);
	}
	
	if (fp) {
		fflush(fp);
		fclose(fp);
	}	

	rename(fn_back, fn);
}
#if 0
static char *dns_a[] = {
    {"cn.ice.galaxywind.com"}, //中国
    {"ie.ice.galaxywind.com"}, //爱尔兰
    {"de.ice.galaxywind.com"}, //德国
    {"us.ice.galaxywind.com"}, //美国
    {"jp.ice.galaxywind.com"}, //日本
    {"sg.ice.galaxywind.com"}, //新加坡
    {"br.ice.galaxywind.com"}, //巴西
    {"au.ice.galaxywind.com"}, //澳大利亚
};
#else
static char *dns_a[8] = {
    "cn.ice.galaxywind.com", //中国
    "ie.ice.galaxywind.com", //爱尔兰
    "de.ice.galaxywind.com", //德国
    "us.ice.galaxywind.com", //美国
    "jp.ice.galaxywind.com", //日本
    "sg.ice.galaxywind.com", //新加坡
    "br.ice.galaxywind.com", //巴西
    "au.ice.galaxywind.com", //澳大利亚
};
#endif

#define ALL_SESSION_NUM 	((int)((sizeof(dns_a)/sizeof(dns_a[0]))))

static bool ucla_session_init()
{
	int i;
	int n = ALL_SESSION_NUM;

	for(i = 0; i < n; i++) {
		ucla_new(dns_a[i]);
	}

	//这里后面服务器数据同步了，sdk只与一个服务器建立连接，要处理下兼容问题，如用户与域名之间的关系。
	for(i = 0; i < n; i++) {
		ucla_conf_init(dns_a[i]);
	}

	//这里还要初始化下其他oem的域名
	ucla_oem_session_init();

	return true;
}

static bool is_our_stand_doname(char *dn)
{
	int i;
	int n = ALL_SESSION_NUM;
	
	if (!dn) {
		return false;
	}

	for(i = 0; i < n; i++) {
		if (strcmp(dn, dns_a[i]) == 0) {
			return true;
		}
	}

	return false;
}

#if 0
static bool ucla_session_init()
{
	bool ret = false;
	char fn[256];
	FILE *fp;
	char buf[1024];
	char doname[512];

	if (cl_priv->priv_dir == NULL) {
        return false;
    }

    sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_LA_SERVER_FILE);

	fp = fopen(fn, "r");
	if (!fp) {
		return false;
	}

	while(fgets(buf, sizeof(buf), fp)) {
		if (strchr(buf, '.')) {
			sscanf(buf, "%s\n", doname);
			ucla_new(doname);
			ret = true;
		}
	}
	
	if (fp) {
		fclose(fp);
	}	

	return ret;
}
#endif

void ucla_conf_clean(ucla_session_t *s)
{
	
}

void ucla_user_add(ucla_session_t *s, la_user_t *puser)
{
	puser->session_ptr = s;
	
	cl_lock(&cl_priv->mutex);	
	stlc_list_add_tail(&puser->link, &cl_priv->la_user);
	cl_unlock(&cl_priv->mutex);	
	
	ucla_user_conf_save(s);
}

//当前手机账号标志
#define UCLA_IS_CUR_PHONE		(1)
void ucla_set_cur_phone(la_phone_t *phone)
{
	if (phone) {
		phone->conf.flag = UCLA_IS_CUR_PHONE;
	}
}

bool ucla_is_cur_phone(la_phone_t *phone)
{
	if (phone &&
		(phone->conf.flag == UCLA_IS_CUR_PHONE)) {
		return true;
	}

	return false;
}

static void ucla_phone_clean()
{
	la_phone_t *pp, *ppn;

	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		if (!ucla_is_cur_phone(pp)) {
			stlc_list_del(&pp->link);
			cl_free(pp);
		}
	}
}

la_phone_t *ucla_find_cur_phone()
{
	la_phone_t *pp, *ppn;

	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		if (ucla_is_cur_phone(pp)) {
			return pp;
		}
	}

	return NULL;
}

void ucla_phone_flag_clean()
{
	la_phone_t *p, *pn;

	memset(plc->back_user_name, 0, sizeof(plc->back_user_name));
	stlc_list_for_each_entry_safe(la_phone_t, p, pn, &plc->la_phone, link) {
		if (ucla_is_cur_phone(p)) {
			strcpy((void *)plc->back_user_name, p->conf.user_name);
		}
		p->conf.flag = 0;
	}

	ucla_phone_save();
}

void ucla_phone_recovery_old_sess()
{
	ucla_session_t *s, *sn;

	UCLA_ENTER();
	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		memcpy((void *)s->username, (void *)s->back_username, sizeof(s->username));
		memcpy((void *)s->key, (void *)s->back_key, sizeof(s->key));
		memcpy((void *)s->uuid, s->back_uuid, sizeof(s->uuid));
		s->idle_time = 0;
		ucla_set_status(s, UCLAS_IDLE);
		s->user_id = 0;
	}
	plc->phone_passwd_err = false;
}

void ucla_phone_recovery()
{
	la_phone_t *phone;

	UCLA_ENTER();
	//恢复以前用的当前手机号 
	phone = ucla_find_cur_phone();
	if (phone) {
		phone->conf.flag = 0;
	}
	phone = ucla_find_phone_by_name((char *)plc->back_user_name);
	if (phone) {
		ucla_set_cur_phone(phone);
	}

	//这里处理一下，非当前手机账号直接删除
	ucla_phone_clean();

	ucla_server_user_query_clean();
	//恢复以前的个session用户
	ucla_phone_recovery_old_sess();
}

void ucla_phone_add(la_phone_t *phone)
{
	if (!phone) {
		return;
	}

	stlc_list_add_tail(&phone->link, &plc->la_phone);
	ucla_phone_save();
}

void ucla_home_add(ucla_session_t *s, la_home_t *phome)
{
	la_user_t *puser;

	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}
	phome->session_ptr = s;
	stlc_list_add_tail(&phome->link, &puser->home_link);
	ucla_user_conf_save(s);
}

void ucla_member_add(la_member_t *pmem, la_home_t *phome)
{
	if (!phome || !pmem) {
		return;
	}

	pmem->session_ptr = phome->session_ptr;
	stlc_list_add_tail(&pmem->link, &phome->member_link);
}

bool ucla_home_free_memlist(la_home_t *phome)
{
	la_member_t *pmem, *memn;
	bool ret = false;
	
	//释放成员链表
	stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &phome->member_link, link) {
		stlc_list_del(&pmem->link);
		cl_free(pmem);
		ret = true;
	}
	
	return ret;
}

void ucla_home_del(ucla_session_t *s, u_int32_t home_id)
{
	la_home_t *phome;

	if (!s) {
		return;
	}
	phome = ucla_find_home_by_id(home_id);
	if (!phome) {
		return;
	}

	ucla_home_del2(s, phome);
}

void ucla_home_del2(ucla_session_t *s, la_home_t *phome)
{
	if (!phome || !s) {
		return;
	}
	
	stlc_list_del(&phome->link);
	ucla_home_free(phome);
	ucla_user_conf_save(s);
}

void ucla_phone_del(la_phone_t *phone)
{
	stlc_list_del(&phone->link);
	cl_free(phone);
	ucla_phone_save();
}

void ucla_user_del(ucla_session_t *s, u_int32_t user_id)
{
	la_user_t *puser;

	puser = ucla_find_user_by_id(user_id);
	if (!puser) {
		return;
	}

	stlc_list_del(&puser->link);
	ucla_user_free(puser);
	
	ucla_user_conf_save(s);
}

bool ucla_mem_del(la_home_t *phome, u_int64_t sn)
{
	la_member_t *pmem, *mn;

	stlc_list_for_each_entry_safe(la_member_t, pmem, mn, &phome->member_link, link) {
		if (pmem->conf.sn == sn) {
			stlc_list_del(&pmem->link);
			cl_free(pmem);
			return true;
		}
	}

	return false;
}

static void la_write_conf_2_file(u_int8_t *pdata, int data_len, char *file)
{
	FILE *fp = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	if ((!file) ||
		(!pdata) ||
		(!cl_priv->priv_dir)) {
        return;
    }

	sprintf(fn, "%s/%s", cl_priv->priv_dir,file);
	fp = fopen(fn, "wb");
	if (!fp) {
		return;
	}

	if (pdata && (data_len > 0)) {
		fwrite((void *)pdata, data_len, 1, fp);
	}
	fflush(fp);
	fclose(fp);
}

static void la_write_conf_rename_file(char *file, char *file_back)
{
	char fn[UCLA_FILE_PATH_LEN];
	char fn_back[UCLA_FILE_PATH_LEN];

	if ((!file) ||
		(!file_back) ||
		(!cl_priv->priv_dir)) {
        return;
    }

	sprintf(fn, "%s/%s", cl_priv->priv_dir, file);
	sprintf(fn_back, "%s/%s", cl_priv->priv_dir, file_back);
	remove(fn_back);
	rename(fn, fn_back);
}

static int la_conf_fill_enc(ucla_conf_save_t *pconf, int data_len, int count)
{
//	int fill_len = 0;
	MD5_CTX ctx;
    u_int8_t result[16] = {0};
	u_int8_t *pdata = (u_int8_t *)pconf;
	int pad_len = 0;
	int aes_len = 0;

	if ((!pconf) ||
		(data_len == 0)) {
		return 0;
	}

	//添加magic头部
	pconf->magic = UCLA_CONF_SAVE_MAGIC;
	pconf->ver = UCLA_CONF_SVAE_VER;
	pconf->count = (u_int32_t)count;
	pconf->crc = 0;

	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)pconf, (sizeof(*pconf) + data_len));
	MD5Final(result, &ctx);
	memcpy(&pconf->crc, &result[12], 4);

	//aes加密
	aes_len = data_len + sizeof(*pconf);
	pad_len = AES128_EKY_LEN - (aes_len & (AES128_EKY_LEN - 1));
	memset(pdata + aes_len, pad_len, pad_len);
	aes_len += pad_len;
	enc_block((u_int8_t *)pdata, aes_len, plc->conf_aes_key);

	return aes_len;
}

static int _la_mem_get_lan(la_user_t *puser)
{
	u_int32_t len = 0;
	la_home_t *ph, *phn;
	la_member_t *pm, *pmn;

	if (!puser) {
		return 0;
	}

	stlc_list_for_each_entry_safe(la_home_t ,ph, phn, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_member_t, pm, pmn, &ph->member_link, link) {
			len += sizeof(pm->conf);
		}
	}

	return (int)(len);
}

static void _la_mem_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	la_home_t *phome, *homen;
	la_member_t *pmem, *memn;

	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	int len = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	//UCLA_ENTER();
	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	len = _la_mem_get_lan(puser);
	if (0 == len) {
		goto done;
	}
	save_buf = cl_calloc(len + 100, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	
	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &phome->member_link, link) {
			//UCLA_ENTER();
			memcpy((void *)&pconf->data[save_len], (void *)&pmem->conf, sizeof(pmem->conf));
			save_len += sizeof(pmem->conf);
			save_count++;
		}
	}
	save_len = la_conf_fill_enc(pconf, save_len, save_count);

done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		//la_file_clean(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);
	//UCLA_EXIT();
}

static void la_mem_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_MEMBER_FILE, UCLA_LA_MEMBER_FILE_BACK);
	_la_mem_save(s, UCLA_LA_MEMBER_FILE, UCLA_LA_MEMBER_FILE_BACK);
}

static int la_get_label_conf_len(la_user_t *puser)
{
	u_int32_t len = 0;
	la_home_t *ph, *phn;
	la_label_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_home_t, ph, phn, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_label_t, pl, pln, &ph->label_link, link) {
			len += sizeof(pl->conf) + pl->conf.sn_num * sizeof(u_int64_t);
		}
	}

	return (int)len;
}

static void _la_label_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	la_home_t *phome, *homen;
	la_label_t *pl, *pln;

	u_int8_t *save_buf = NULL;
	int len1 = 0;
	int save_len = 0;
	int save_count = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	//UCLA_ENTER();
	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	len1 = la_get_label_conf_len(puser);
	if (0 == len1) {
		goto done;
	}
	save_buf = cl_calloc(len1 + 100, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	
	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_label_t, pl, pln, &phome->label_link, link) {
			memcpy((void *)&pconf->data[save_len], (void *)&pl->conf, sizeof(pl->conf));
			save_len += sizeof(pl->conf);
			if (pl->conf.sn_num) {
				len1 = (int)(pl->conf.sn_num*sizeof(u_int64_t));
				memcpy((void *)&pconf->data[save_len], (void *)pl->p_sn, len1);
				save_len += len1;
			}
			save_count++;
		}
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);


done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		//la_file_clean(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}
	SAFE_FREE(save_buf);
	
	//UCLA_EXIT();
}


static void la_label_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_LABEL_FILE, UCLA_LA_LABEL_FILE_BACK);
	_la_label_save(s, UCLA_LA_LABEL_FILE, UCLA_LA_LABEL_FILE_BACK);
}

static int _la_rule_get_lan(la_user_t *puser)
{
	u_int32_t len = 0;
	la_home_t *phome, *homen;
	la_rule_t *prule, *prulen;

	if (!puser) {
		return 0;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_rule_t, prule, prulen, &phome->rule_link, link) {
			len += prule->rule_len + sizeof(la_rule_conf_t);
		}
	}

	return (int)len;
}

static void _la_rule_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	la_home_t *phome, *homen;
	la_rule_t *prule, *prulen;
	la_rule_conf_t rulec;

	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	int len = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	len = _la_rule_get_lan(puser);
	if (0 == len) {
		goto done;
	}
	save_buf = cl_calloc(len + 100, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	
	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		stlc_list_for_each_entry_safe(la_rule_t, prule, prulen, &phome->rule_link, link) {
			rulec.flag = prule->flag;
			rulec.rule_len = prule->rule_len;
			rulec.rule_id = prule->rule_id;
			rulec.last_exec_time = prule->last_exec_time;
			rulec.enable = prule->enable;
			rulec.state = prule->state;
			rulec.home_id = phome->conf.home_id;
			memcpy((void *)&pconf->data[save_len], (void *)&rulec, sizeof(rulec));
			save_len += sizeof(rulec);
			//rule
			if (prule->rule_len) {
				strcpy((char *)&pconf->data[save_len], (char *)prule->rule);
				save_len += prule->rule_len;
			}
			
			save_count++;
		}
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);

done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);
}

static void la_rule_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_RULE_FILE, UCLA_LA_RULE_FILE_BACK);
	_la_rule_save(s, UCLA_LA_RULE_FILE, UCLA_LA_RULE_FILE_BACK);
}

static int _la_share_get_lan(la_user_t *puser)
{
	int len = 0;
	la_home_t *phome, *phomen;

	if (!puser) {
		return 0;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, 
		phomen, &puser->home_link, link) {
		if (phome->share_desc_num == 0) {
			continue;
		}
		len += (int)(sizeof(la_share_conf_t) + phome->share_desc_num*sizeof(la_share_desc_t));
	}

	return len;
}

static void _la_share_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	la_home_t *phome, *homen;
	la_share_conf_t sc;

	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	int len = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	len = _la_share_get_lan(puser);
	if (0 == len) {
		goto done;
	}
	save_buf = cl_calloc(len + 100, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	
	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (phome->share_desc_num == 0) {
			continue;
		}
		sc.home_id = phome->conf.home_id;
		sc.share_num = phome->share_desc_num;
		memcpy((void *)&pconf->data[save_len], (void *)&sc, sizeof(sc));
		save_len += sizeof(sc);
		//rule
		if (phome->share_desc_num) {
			len = phome->share_desc_num*sizeof(la_share_desc_t);
			memcpy((void *)&pconf->data[save_len], (void *)phome->share_desc_array, len);
			save_len += len;
		}
		
		save_count++;
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);

done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);
}

static void la_share_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_SHARE_FILE, UCLA_LA_SHARE_FILE_BACK);
	_la_share_save(s, UCLA_LA_SHARE_FILE, UCLA_LA_SHARE_FILE_BACK);
}

static bool ucla_home_sc_valid(la_home_t *phome)
{
	int i;

	if (!phome) {
		return false;
	}

	for(i = 0; i < LA_SC_A_NUM; i++) {
		if (phome->la_sc_key[i].valid) {
			return true;
		}
	}

	return false;
}

static int _la_sc_get_lan(la_user_t *puser)
{
	int len = 0;
	la_home_t *phome, *phomen;

	if (!puser) {
		return 0;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, 
		phomen, &puser->home_link, link) {
		if (!ucla_home_sc_valid(phome)) {
			continue;
		}
		len += (int)(sizeof(la_sc_conf_t));
	}

	return len;
}

static void _la_sc_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	la_home_t *phome, *homen;
	la_sc_conf_t sc;

	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	int len = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	len = _la_sc_get_lan(puser);
	if (0 == len) {
		goto done;
	}
	save_buf = cl_calloc(len + 100, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	
	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_sc_valid(phome)) {
			continue;
		}
		sc.home_id = phome->conf.home_id;
		memcpy((void *)&sc.la_sc_key, (void *)&phome->la_sc_key, sizeof(sc.la_sc_key));
		memcpy((void *)&pconf->data[save_len], (void *)&sc, sizeof(sc));
		save_len += sizeof(sc);
		
		save_count++;
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);

done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);
}

static void la_sc_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_SC_FILE, UCLA_LA_SC_FILE_BACK);
	_la_sc_save(s, UCLA_LA_SC_FILE, UCLA_LA_SC_FILE_BACK);
}

static int _la_template_get_lan(la_user_t *puser)
{
	int i;
	u_int32_t len = 0;

	for(i = 0; i < plc->url_num; i++) {
		len += (u_int32_t)strlen((char *)plc->url_array[i]) + 1 + sizeof(int);
	}

	//log_debug("enter %s num=%u len=%u\n", __FUNCTION__, plc->url_num, len);
	return (int)len;
}

static void _la_template_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;

	u_int8_t *save_buf = NULL;
	int i = 0;
	int save_len = 0;
	int save_count = 0;
	int len = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	//UCLA_ENTER();
	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	len = _la_template_get_lan(puser);
	if (0 == len) {
		goto done;
	}
	save_buf = cl_calloc(len + 100, 1);

	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	
	for(i = 0; i < plc->url_num; i++) {
		len = (int)strlen((char *)plc->url_array[i]) + 1;
		memcpy((void *)&pconf->data[save_len], (void *)&len, sizeof(len));
		save_len += sizeof(len);
		strcpy((char *)&pconf->data[save_len], (char *)plc->url_array[i]);
		save_len += (u_int32_t)len;
		
		save_count++;
		//log_debug("enter %s save_count=%u len=%u \nplc->url_array[%d]=%s\n", 
			//__FUNCTION__, save_count, len, i, plc->url_array[i]);
	}
	//memdumpone("_la_template_save", pconf->data, save_len);
	save_len = la_conf_fill_enc(pconf, save_len, save_count);


done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);
	//UCLA_EXIT();
}

static void la_template_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_TEMPLATE_FILE, UCLA_LA_TEMPLATE_FILE_BACK);
	_la_template_save(s, UCLA_LA_TEMPLATE_FILE, UCLA_LA_TEMPLATE_FILE_BACK);
}

static void _la_cap_save(ucla_session_t *s, char *file, char *file_back)
{
	int i;
	char buff[1024];
	char fn[UCLA_FILE_PATH_LEN];
	FILE *fp = NULL;

	if (!s || 
		!file ||
		!file_back ||
		!cl_priv->priv_dir) {
		return;
	}

	if (plc->cap_num == 0) {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
		return;
	}

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
	fp = fopen(fn, "w");
	if (!fp) {
		log_err(true, "fn=%s fopen failed\n", fn);
		return;
	}
	//num
	sprintf(buff, "time=%u num=%u\n", plc->cap_lastmodifytime, plc->cap_num);
	fputs(buff, fp);
	for(i = 0; i < plc->cap_num; i++) {
		sprintf(buff, "url=%s\n", plc->cap_array[i]);
		fputs(buff, fp);
	}

	if (fp) {
		fclose(fp);
	}	
}

static void la_cap_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_CAP_FILE, UCLA_LA_CAP_FILE_BACK);
	_la_cap_save(s, UCLA_LA_CAP_FILE, UCLA_LA_CAP_FILE_BACK);
}

static void _la_cap_custom_save(ucla_session_t *s, char *file, char *file_back)
{
	int i;
	char buff[1024];
	char fn[UCLA_FILE_PATH_LEN];
	FILE *fp = NULL;

	if (!s || 
		!file ||
		!file_back ||
		!cl_priv->priv_dir) {
		return;
	}

	if (plc->cap_custom_num == 0) {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
		return;
	}

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
	fp = fopen(fn, "w");
	if (!fp) {
		log_err(true, "fn=%s fopen failed\n", fn);
		return;
	}
	//num
	sprintf(buff, "time=%u num=%u\n", plc->cap_custom_lastmodifytime, plc->cap_custom_num);
	fputs(buff, fp);
	for(i = 0; i < plc->cap_custom_num; i++) {
		sprintf(buff, "url=%s\n", plc->cap_custom_array[i]);
		fputs(buff, fp);
	}

	if (fp) {
		fclose(fp);
	}	
}


static void la_cap_custom_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_CAP_CUSTOM_FILE, UCLA_LA_CAP_CUSTOM_FILE_BACK);
	_la_cap_custom_save(s, UCLA_LA_CAP_CUSTOM_FILE, UCLA_LA_CAP_CUSTOM_FILE_BACK);
}

static void _la_home_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	la_home_t *phome, *homen;

	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	//UCLA_ENTER();
	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	save_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;
	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_valid(phome)) {
			continue;
		}
		memcpy((void *)&pconf->data[save_len], (void *)&phome->conf, sizeof(phome->conf));
		save_len += sizeof(phome->conf);
		save_count++;
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);


done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		//la_file_clean(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);
	//UCLA_EXIT();
}

static void la_home_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_HOME_FILE, UCLA_LA_HOME_FILE_BACK);
	_la_home_save(s, UCLA_LA_HOME_FILE, UCLA_LA_HOME_FILE_BACK);
}

static void _la_user_save(ucla_session_t *s, char *file, char *file_back)
{
	la_user_t *puser;
	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	ucla_conf_save_t *pconf = NULL;
	char fn[UCLA_FILE_PATH_LEN];

	if (!s || !file || !cl_priv->priv_dir) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		goto done;
	}
	save_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!save_buf) {
		log_err(false, "calloc failed\n");
		return;
	}
	if (memcmp(puser->conf.uuid, s->uuid, MAX_UUID_BIN_LEN)) {
		memcpy(puser->conf.uuid, s->uuid, MAX_UUID_BIN_LEN);
	}
	pconf = (ucla_conf_save_t *)save_buf;

	log_debug("%s la_user_save name=%s userid=%u\n", 
		s->doname, puser->conf.user_name, puser->conf.user_id );
	memcpy((void *)&pconf->data[save_len], (void *)&puser->conf, sizeof(puser->conf));
	save_len += sizeof(puser->conf);
	save_count++;

	save_len = la_conf_fill_enc(pconf, save_len, save_count);


done:
	sprintf(fn, "%s/%s", s->doname, file);
	if (save_len) {
		la_write_conf_2_file(save_buf, save_len, fn);
	} else {
		//la_file_clean(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);
		remove(fn);
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);
		remove(fn);
	}

	SAFE_FREE(save_buf);	
}

static void la_rename_file(ucla_session_t *s, char *file, char *file_back)
{
	char fn[UCLA_FILE_PATH_LEN];
	char fn_back[UCLA_FILE_PATH_LEN];

	if (!s ||
		!file ||
		!file_back ||
		!cl_priv->priv_dir) {
		return;
	}

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, file);	
	sprintf(fn_back, "%s/%s/%s", cl_priv->priv_dir, s->doname, file_back);

	remove(fn_back);
	rename(fn, fn_back);
}

static void la_conf_clean(ucla_session_t *s)
{
	char fn[UCLA_FILE_PATH_LEN];

	if (!s ||
		!cl_priv->priv_dir) {
		return;
	}

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_USER_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_USER_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_HOME_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_HOME_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_MEMBER_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_MEMBER_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_TEMPLATE_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_TEMPLATE_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_RULE_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_RULE_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_SHARE_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_SHARE_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_DNS_IP_RESOLVE_FILE);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_SC_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_SC_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_CUSTOM_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_CAP_CUSTOM_FILE_BACK);
	remove(fn);

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_LABEL_FILE);
	remove(fn);
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_LA_LABEL_FILE_BACK);
	remove(fn);
}

static void la_user_save(ucla_session_t *s)
{
	la_rename_file(s, UCLA_LA_USER_FILE, UCLA_LA_USER_FILE_BACK);
	_la_user_save(s, UCLA_LA_USER_FILE, UCLA_LA_USER_FILE_BACK);
}

void ucla_user_conf_save(ucla_session_t *s)
{
	//log_debug("enter ucla_user_conf_save\n");
	s->need_save_conf = true;
	//reset save time
	la_comm_timer_proc_reset();
}

void ucla_phone_save()
{
	//log_debug("enter ucla_user_conf_save\n");
	plc->need_save_phone = true;
	//reset save time
	la_comm_timer_proc_reset();
}

void ucla_all_home_query_set(ucla_session_t *s)
{
	s->need_query_all_home = true;
	la_comm_timer_proc_reset();
}

void _ucla_user_conf_save(ucla_session_t *s)
{
	char fn[UCLA_FILE_PATH_LEN];

	//UCLA_ENTER();
    sprintf(fn, "%s/%s", cl_priv->priv_dir, s->doname);
    MKDIR(fn, 0777);

	if (s->need_clean_conf) {
		s->need_clean_conf = false;
		s->need_save_username = false;
		la_conf_clean(s);
		RMDIR(fn);
		return;
	}

	if (s->need_save_username) {
		la_user_save(s);
		s->need_save_username = false;
	}	
	la_home_save(s);
	la_mem_save(s);
	la_label_save(s);
	la_rule_save(s);
	la_share_save(s);
	la_sc_save(s);
	la_template_save(s);
	la_cap_save(s);
	la_cap_custom_save(s);
	//UCLA_EXIT();
}

static void la_dns_init(ucla_session_t *s)
{
	int i;
	FILE *fp = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	
	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_DNS_IP_RESOLVE_FILE);
	fp = fopen(fn, "rb");
	if (!fp) {
		return;
	}

	if (fread((void *)s->ipc_sock, (off_t)sizeof(s->ipc_sock), 1, fp) != 1) {
		log_info("fn=%s read error\n", fn);
		goto done;
	}

	for(i = 0; i < DNS_IP_MAX_NUM; i++) {
		if (s->ipc_sock[i].ai_family == 0) {
			break;
		}
	}
	s->ipc_count = i;
	log_debug("enter %s doname=%s ipc_count=%u\n", __FUNCTION__, s->doname, i);
	
done:
	if (fp) {
		fclose(fp);
	}
}

static void la_dns_save(ucla_session_t *s)
{
	int count = 0;
	FILE *fp = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	ipc_sock_t ipc_sock[DNS_IP_MAX_NUM];
	u_int32_t dns_timeout = 0;//等于0表示是缓存的，不用保存

	UCLA_ENTER();
	dns_timeout = cl_get_dns_timeout(s->doname);
	if (dns_timeout == 0) {
		log_debug("dns_timeout==0 !!!!!!!!!!!!!!!!!!!!!!!!!\n");
		return;
	}

	count = 0;
	memset(&ipc_sock, 0, sizeof(ipc_sock));
	if(!cl_get_addr_by_doname(s->doname, ipc_sock, &count)) {
		log_debug("cl_get_addr_by_doname ==0 !!!!!!!!!!!!!!!!!!!!!!!!!\n");
		return;
	}
	s->need_save_dns = false;

	sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_DNS_IP_RESOLVE_FILE);
	fp = fopen(fn, "wb");
	if (!fp) {
		return;
	}

	fwrite((void *)ipc_sock, sizeof(ipc_sock), 1, fp);

	if (fp) {
		fclose(fp);
	}
}

void do_la_dns_cache_clean()
{
	ucla_session_t *s, *sn;
	char fn[UCLA_FILE_PATH_LEN];

	if (!app_is_la() || 
		!la_is_valid() ||
		!cl_priv->priv_dir) {
		return;
	}

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		sprintf(fn, "%s/%s/%s", cl_priv->priv_dir, s->doname, UCLA_DNS_IP_RESOLVE_FILE);
		remove(fn);
	}
}

#ifdef UCLA_DEBUG
#define DEBUG_MAX_LEN	(1024*1024)
void ucla_session_info_dump()
{
	int i;
	char *buff = NULL;
	char fn[UCLA_FILE_PATH_LEN];
	FILE *fp = NULL;
	la_user_t *puser;
	la_home_t *phome, *homen;
	la_member_t *pmem, *memn;
	la_rule_t *prule, *rulen;
	la_label_t *pl, *pln;
	ucla_session_t *s, *sn;
	la_phone_t *pp, *ppn;
	la_sc_key_t *pkey;
	time_t now;

	now = time(NULL);
	
	if (cl_priv->priv_dir == NULL) {
        return;
    }

	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DUMP_INFO);
	fp = fopen(fn, "w");
	if (!fp) {
		goto end;
	}
	buff = cl_calloc(DEBUG_MAX_LEN, 1);
	if (!buff) {
		goto end;
	}
	
	//phone info
	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		sprintf(buff, "phone username=%s\n", pp->conf.user_name);
		fputs(buff, fp);
	}

	//dump template capfile
	sprintf(buff, "last_template_time=%u cap_lastmodifytime=%u lang=%u\n", 
	plc->last_template_time, plc->cap_lastmodifytime, plc->lang);
	fputs(buff, fp);
	for(i = 0; i < plc->url_num; i++) {
		sprintf(buff, "template%u=%s\n", i, plc->url_array[i]);
		fputs(buff, fp);
	}
	for(i = 0; i < plc->cap_num; i++) {
		sprintf(buff, "capfile%u=%s\n", i, plc->cap_array[i]);
		fputs(buff, fp);
	}
	for(i = 0; i < plc->cap_custom_num; i++) {
		sprintf(buff, "capfile_custom%u=%s\n", i, plc->cap_custom_array[i]);
		fputs(buff, fp);
	}

	//session info
	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		sprintf(buff, "\n****%s username=%s disp=%u.%u.%u.%u and timenow=%s dmap=%u is_phone=%u "
			"username=%s status=%u userid=%u time_start=%u time_diff=%u has_def=%u\n", 
			s->doname, s->username, IP_SHOW(s->disp_ip), ctime(&now), s->dmap, ucla_is_phone_session(s), s->username,
			s->status, s->user_id, s->time_start, (s->status == UCLAS_ESTABLISH)?s->time_diff:0, s->has_def_home);
		fputs(buff, fp);
	
		puser = ucla_find_user_by_id(s->user_id);
		if (!puser) {
			sprintf(buff, "%s has no user\n", s->doname);
			fputs(buff, fp);
			continue;
		}
		
		if (!ucla_session_is_ok(s)) {
			sprintf(buff, "%s status=%u is not connected server\n", s->doname, s->status);
			fputs(buff, fp);
			//continue;
		}

		stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
			//homeinfo
			sprintf(buff, "homeinfo id=%u name=%s handle=%08x is_def=%u\n", 
			phome->conf.home_id, phome->conf.home_name, phome->handle, phome->conf.type);
			fputs(buff, fp);
			//member
			stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &phome->member_link, link) {
				sprintf(buff, "mem=>sn=%"PRIu64" is_la=%u\n", pmem->conf.sn, pmem->conf.is_la);
				fputs(buff, fp);
			}
			//rule
			stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
				sprintf(buff, "rule=>id=%08x enable=%u state=%u last_exetime=%u rule_len=%u rule=%s\n", 
					prule->rule_id, prule->enable, prule->state, prule->last_exec_time, prule->rule_len, 
					prule->rule_len > 0?prule->rule:"NULL");
				fputs(buff, fp);
				if (prule->last_exec_time) {
					now = prule->last_exec_time;
					sprintf(buff, "prule->last_exec_time=%s\n", ctime(&now));
					fputs(buff, fp);
				}
			}
			//share
			sprintf(buff, "sharecode=%s\n", phome->share?(char *)phome->share:"NULL");
			fputs(buff, fp);
			//url
			for(i = 0; i < phome->url_num; i++) {
				sprintf(buff, "url=%s\n", phome->url_array[i]);
				fputs(buff, fp);
			}
			//share
			for(i = 0; i < phome->share_desc_num; i++) {
				sprintf(buff, "share_mem=>userid=%08x jointime=%u lastusetime=%u role_id=%08x desc=%s\n", 
					phome->share_desc_array[i].user_id, phome->share_desc_array[i].join_time, 
					phome->share_desc_array[i].lastuse_time, phome->share_desc_array[i].role_id, 
					phome->share_desc_array[i].desc);
				fputs(buff, fp);
			}
			//last
			sprintf(buff, "last_rule_time=%u last_template_time=%u\n\n\n", 
				phome->last_rule_time, phome->last_template_time);
			fputs(buff, fp);
			//label
			stlc_list_for_each_entry_safe(la_label_t, pl, pln, &phome->label_link, link) {
				sprintf(buff, "label name=%s id=%u sn_num=%u\n", pl->conf.name, pl->conf.id, pl->conf.sn_num);
				fputs(buff, fp);
				for(i = 0; i < pl->conf.sn_num; i++) {
					sprintf(buff, "label sn=%"PRIu64"\n", pl->p_sn[i]);
					fputs(buff, fp);
				}
			}
			//key
			for(i = 0; i < LA_SC_A_NUM; i++) {
				pkey = &phome->la_sc_key[i];
				if (!pkey->valid) {
					continue;
				}
				sprintf(buff, "sc key index=%u ruleid=%u name=%s\n", i + 1, pkey->rule_id, pkey->name);
				fputs(buff, fp);
			}
		}
	}

end:	
	if (fp) {
		fclose(fp);
	}	
	SAFE_FREE(buff);
}
#else
void ucla_session_info_dump()
{

}
#endif

pkt_t *ucla_pkt_new(int cmd, int param_len,
			bool is_request, bool is_enc, u_int8_t flags,
			u_int32_t client_sid, u_int32_t device_sid, u_int32_t request_id)
{
	pkt_t *pkt;
	ucph_t *hdr;
	int total_len ;
    

    total_len = sizeof(pkt_t) + ucph_hdr_size + param_len;
	// 多申请一块内存，用于加密时候用
	if ((pkt = (pkt_t *)cl_calloc(total_len + AES128_EKY_LEN, 1)) == NULL) {
		log_debug( "%s alloc %d failed\n", __FUNCTION__, total_len);
		return NULL;
	}
	pkt->total = ucph_hdr_size + param_len;
	pkt->cmd = cmd;

	hdr = (ucph_t *)&pkt->data[0];
	
	hdr->ver = PROTO_VER1;
	hdr->hlen = ucph_hdr_size/4;
	hdr->request = is_request;
	hdr->encrypt = is_enc;
	hdr->flags = flags;
	hdr->client_sid = htonl(client_sid);
	hdr->device_sid = htonl(device_sid);
	hdr->request_id = request_id & 0xFF;
	
	hdr->command = htons(cmd);
	hdr->param_len = htons(param_len);

	return pkt;
}

static bool ucla_home_timeout_need_report(pkt_t *pkt)
{
	bool ret = false;

	switch(pkt->action) {
	case UCAU_REMOVEDEV:
		//ucla_event_push(LA_HOME_REMOVEDEV_FAILED);
		ucla_home_event_push(pkt->home_id, LA_HOME_REMOVEDEV_FAILED);
		log_debug("LA_HOME_REMOVEDEV_FAILED !!!!!!!!!!!!!!!!!!!!!\n");
		ret = true;
		break;
	default:
		break;
	}

	return ret;
}

static bool ucla_home_label_timeout_need_report(pkt_t *pkt)
{
	bool ret = false;

	switch(pkt->action) {
	case UCAU_CREATE:
		event_push_err(plc->callback, LA_LABEL_ADD_FAILED, 0, 
			plc->callback_handle, LA_PHONE_ALL_SESSION_OFFLINE);
		ret = true;
		break;
	case UCAU_DELETE:
		event_push_err(plc->callback, LA_LABEL_DEL_FAILED, 0, 
			plc->callback_handle, LA_PHONE_ALL_SESSION_OFFLINE);
		ret = true;
		break;
	case UCAU_LABEL_BIND:
		event_push_err(plc->callback, LA_LABEL_BIND_FAILED, 0, 
			plc->callback_handle, LA_PHONE_ALL_SESSION_OFFLINE);
		ret = true;
		break;
	default:
		break;
	}

	return ret;
}

static bool ucla_timeout_need_report(pkt_t *pkt)
{
	bool ret = false;
	ucph_t *hdr;

	if (!pkt) {
		return ret;
	}
	hdr = (ucph_t *)pkt->data;

	switch(pkt->cmd) {
	case CMD_HOME_CONFIG:
		ret = ucla_home_timeout_need_report(pkt);
		break;
	case CMD_HOME_SHARE:
		break;
	case CMD_LINKAGE_CONFIG:
		break;
	case CMD_APP_USER:
		break;
	case CMD_HOME_LABEL:
		ret = ucla_home_label_timeout_need_report(pkt);
		break;
	default:
		break;
	}

	return ret;
}

static int ucla_send_timer(cl_thread_t *t)
{
	int n;
	ucla_session_t *s;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	pkt_t *pkt;

	s = (ucla_session_t *)CL_THREAD_ARG(t);
	s->t_send = NULL;
	if (stlc_list_empty(&s->send_list)) {
		log_debug( "Big bug: %s but send list is empty\n", __FUNCTION__);
		return 0;
	}

	if (s->send_retry >= 3)
		n = 2;
	else
		n = s->send_retry;
	n = s->time_param_cur->retry_100ms[n] * 100;

	if(s->send_retry > MAX_SEND_RETRY_NUM){
		ucla_set_status(s, UCLAS_IDLE);
		return 0;
	}
	
	CL_THREAD_TIMER_ON(MASTER, s->t_send, ucla_send_timer, (void *)s, n);
	s->send_retry++;
    s->send_pkts++;

	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);

	////"这里处理一下，如果是重要命令，可能需要尽快判断是否成功，好尽快给上层app做提示，比如删除设备是否成功啥的"
	if ((s->send_retry > MAX_IMPORTANT_CMD_NUM) && 
		ucla_timeout_need_report(pkt)) {
		ucla_set_status(s, UCLAS_IDLE);
		return 0;
	}

	if (s->send_retry > MAX_IMPORTANT_CMD_NUM) {
		nd_la_debug(s, "ucla_send_timer s->send_retry=%u cmd=%u action=%u", s->send_retry, pkt->cmd, pkt->action);
	}

	if (s->is_ipv6) {
		memcpy((void *)&addr6, (void *)&s->ipc_addr.sockaddr6, sizeof(addr6));
		if (pkt->action == ACTION_ALL_PORT_SEND) {
			addr6.sin6_port = htons(ucla_mult_port[s->port_index%UCLA_MAX_PORTS]);
			s->port = htons(addr6.sin6_port);
			n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr6, sizeof(addr6));
		} else {
			addr6.sin6_port = htons(s->port);
			n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr6, sizeof(addr6));
		}
	} else {
		memcpy((void *)&addr, (void *)&s->ipc_addr.sockaddr, sizeof(addr));
		if (pkt->action == ACTION_ALL_PORT_SEND) {
			addr.sin_port = htons(ucla_mult_port[s->port_index%UCLA_MAX_PORTS]);
			s->port = htons(addr.sin_port);
			n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
		} else {
			addr.sin_port = htons(s->port);
			n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
		}
	}
	if (n <= 0) {
		log_debug("%s %s send pkt failed: ip=%u.%u.%u.%u, port=%d, want=%d, send=%d errno[%d]\n",
			s->is_ipv6?"ipv6":"ipv4", s->doname, IP_SHOW(s->ip), s->port, pkt->total, n,errno);
	} else {
#ifdef	DBG_UDP_CTRL_PKT	
		ucph_t *hdr;
        ucph_v2_t* v2_hdr;
		char upg[128] = "";
        
		if(pkt->up_total){
			sprintf(upg, " upgrade %hu / %hu ", pkt->up_current, pkt->up_total);
		}

		hdr = (ucph_t *)pkt->data;
        if (hdr->ver <= PROTO_VER1) {
            log_debug("%s sock=%u send ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, action=%u, req=%d, enc=%d, csid=%08x, dsid=%08x, rid=%u %s\n",
                      s->doname, s->sock, IP_SHOW(s->ip), s->port, n,
                      pkt->cmd, *((u_int8_t *)(hdr + 1)), hdr->request, hdr->encrypt,
                      ntohl(hdr->client_sid), ntohl(hdr->device_sid), hdr->request_id, upg);
        }else if(hdr->ver == PROTO_VER2){
            v2_hdr = (ucph_v2_t*)pkt->data;
            log_debug("%s sock=%u send v2 pkt ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, action=%u, req=%d, enc=%d, csid=%08x, dsid=%08x, rid=%u hash=%08x %s\n",
                      s->doname, s->sock, IP_SHOW(s->ip), s->port, n,
                      pkt->cmd, *((u_int8_t *)(hdr + 1)), hdr->request, hdr->encrypt,
                      ntohl(hdr->client_sid), ntohl(hdr->device_sid), ntohl(v2_hdr->u32_req_id),v2_hdr->pkt_hash, upg);
        }
		
//        mem_dump("pkt content", pkt->data, pkt->total);
#endif
	}

	return 0;
}


RS ucla_enc_pkt(ucla_session_t *s, pkt_t *pkt)
{
	ucph_t *hdr = (ucph_t *)pkt->data;
	
	if (s->select_enc == UC_ENC_AES128) {
		int len;
		u_int8_t pad;

		len = pkt->total - ucph_hdr_plain_size;
		pad = AES128_EKY_LEN - (len & (AES128_EKY_LEN - 1));
		len += pad;
		
		memset(pkt->data + pkt->total, pad, pad);

		enc_block((u_int8_t *)BOFP(pkt->data, ucph_hdr_plain_size), len, s->aes_key);
		pkt->total = len + ucph_hdr_plain_size;

		hdr->encrypt = 1;
	} else {
		hdr->encrypt = 0;
	}

	return RS_OK;
}

void ucla_request_del(ucla_session_t *s)
{
	pkt_t *pkt;

	if (stlc_list_empty(&s->send_list))
		return;
	
	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	stlc_list_del(&pkt->link);
	pkt_free(pkt);

	CL_THREAD_OFF(s->t_send);

#if 0
{
	int num = 0;
	stlc_list_count(num, &s->send_list);
	log_debug("%s num=%d\n", __FUNCTION__, num);
}
#endif	
	if (stlc_list_empty(&s->send_list)) {
		if (ucla_session_is_ok(s)) {
			CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, ucla_keeplive_timer, (void *)s, TIME_N_SECOND(s->time_param_cur->keeplive));
		}
		return;
	}

	s->send_retry = 0;
	CL_THREAD_TIMER_ON(MASTER, s->t_send, ucla_send_timer, (void *)s, 0);
}

//分片个数
#define LA_FRAG_MAX 	(64)
static RS ucla_request_add_fragment(ucla_session_t *s, pkt_t *pkt)
{
	ucph_t *hdr = (ucph_t *)pkt->data;
	u_int16_t param_len = ntohs(hdr->param_len), command = ntohs(hdr->command);
	u_int32_t client_sid = ntohl(hdr->client_sid), device_sid = ntohl(hdr->device_sid);
	int want, remain, i = 0;
	u_int8_t *ptr, *p1, flags;
	pkt_t *np[LA_FRAG_MAX];

	log_debug("request id 0x%x need fragment send, total param_len %u\n", hdr->request_id, param_len);

	if (!ucla_session_is_ok(s)) {
		goto err_out;
	}

	if (stlc_list_empty(&s->send_list)) {
		s->send_retry = 0;
		CL_THREAD_TIMER_ON(MASTER, s->t_send, ucla_send_timer, (void *)s, 0);
	}
	//片数用宏，64k
	memset(np, 0, sizeof(np));
	for (ptr = (u_int8_t*)&hdr[1], remain = param_len; remain > 0 && i < LA_FRAG_MAX; i++) {
		want = MIN(LA_FRAGMENT_SIZE, remain);
		remain -= want;

		flags = hdr->flags;
		flags |= UDP_LA_FLAG_IS_FRAG;
		if (remain == 0) {
			flags |= UDP_LA_FLAG_IS_LAST;
		}

		log_debug("fragment[%d] param_len %u rid %u\n", i, want, s->my_request_id);
		np[i] = ucla_pkt_new(command, want, hdr->request, hdr->encrypt, flags, client_sid, device_sid, s->my_request_id++);
		if (!np[i]) {
			goto err_out;
		}

		p1 = get_uascp_payload(np[i], u_int8_t);
		memcpy(p1, ptr, want);

		if (ucla_enc_pkt(s, np[i]) != RS_OK) {
			goto err_out;
		}

		
		stlc_list_add_tail(&np[i]->link, &s->send_list);

		ptr += want;
	}
	



	CL_THREAD_OFF(s->t_keeplive);

	return RS_OK;

err_out:
	pkt_free(pkt);

	for (; i > 0; i--) {
		if (!np[i]) {
			continue;
		}
		if (np[i]->link.next) {
			stlc_list_del(&np[i]->link);
		}
		pkt_free(np[i]);
		s->my_request_id--;
	}

	return RS_ERROR;
}


RS ucla_request_add(ucla_session_t *s, pkt_t *pkt)
{
	if (!pkt) {
		return RS_ERROR;
	}

	if (pkt->action == 0) {
		pkt->action = *(get_net_ucp_payload(pkt->data, u_int8_t));
	}
#ifdef LA_SUPPORT_FRAGMENT
	if (pkt->total - ucph_hdr_size > LA_FRAGMENT_SIZE) {
		return ucla_request_add_fragment(s, pkt);
	}
#endif	
	if (ucla_session_is_ok(s)) {
		if (ucla_enc_pkt(s, pkt) != RS_OK) {
			pkt_free(pkt);
			return RS_ERROR;
		}
		s->my_request_id++;
	}

	if (stlc_list_empty(&s->send_list)) {
		s->send_retry = 0;
		CL_THREAD_TIMER_ON(MASTER, s->t_send, ucla_send_timer, (void *)s, 0);
	}

#if 0	
{
	//int num = 0;
	//stlc_list_count(num, &s->send_list);
	//log_debug("%s num=%d\n", __FUNCTION__, num);
}	
#endif

	stlc_list_add_tail(&pkt->link, &s->send_list);

	CL_THREAD_OFF(s->t_keeplive);

	return RS_OK;
}

static void ucla_request_reply(ucla_session_t *s, ucph_t* hdr)
{
	pkt_t *pkt = NULL;
	u_int8_t *pe;

	log_debug("ucla_request_reply hdr->request_id=%02x last_id=%02x\n", hdr->request_id, s->home_event_last_id);
	pe = get_net_ucp_payload(hdr, u_int8_t);

	pkt = ucla_pkt_new(hdr->command, 0, 
				false, true, hdr->flags, s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);
}


static void ucla_off_all_timer(ucla_session_t*s)
{
	CL_THREAD_OFF(s->t_disp_recv);
	CL_THREAD_OFF(s->t_disp_recv6);
	CL_THREAD_OFF(s->t_send);
	CL_THREAD_OFF(s->t_keeplive);
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_OFF(s->t_timer);
	CL_THREAD_OFF(s->t_server_check);
}

void ucla_reset_send(ucla_session_t *s)
{
	pkt_t *pkt, *next;

	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_list, link) {
		stlc_list_del(&pkt->link);
		pkt_free(pkt);
	}
	s->send_retry = 0;
	CL_THREAD_OFF(s->t_send);
}

int ucla_send_pkt_raw(SOCKET sock, bool ipv6, ipc_addr_t *ipc_addr, u_int16_t port, pkt_t *pkt)
{
	int n;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;

	if (ipv6) {
		memcpy(&addr6, &ipc_addr->sockaddrc, sizeof(addr6));
		addr6.sin6_port = htons(port);
		n = (int)sendto(sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr6, sizeof(addr6));
	} else {
		memcpy(&addr, &ipc_addr->sockaddrc, sizeof(addr));
		addr.sin_port = htons(port);
		n = (int)sendto(sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
	}

	return n;
}

/***********************idle************************************/
static int ucla_idle_timer(cl_thread_t *t)
{
	char res_doname[64];
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_timer = NULL;

	// TODO:判断是否去分配服务器获取设备服务器ip
	//这里要判断下，如果不是标准的8个域名，就不要探测了，直接dispatch
	if (is_our_stand_doname(s->doname)) {
		ucla_set_status(s, UCLAS_DIS_PROBE);
	} else {
		log_debug("%s is not stand doname !!!!!!!!!!!\n", s->doname);
		strcpy(res_doname, s->doname);
		disp_resolv_doname(1, (const char (*const )[64])res_doname);
		ucla_set_status(s, UCLAS_DISPATCH);
	}

	return 0;
}
static void ucla_idle_into(ucla_session_t *s)
{
	ucla_reset_send(s);
	
	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;
	s->time_diff = 0;
	s->port_index = 0;
	
	ucla_off_all_timer(s);
	
	CL_THREAD_TIMER_ON(MASTER, s->t_timer, ucla_idle_timer, (void *)s, TIME_N_SECOND(s->idle_time));
	s->idle_time = (1 + s->timeout_count) * DFL_UC_IDLE_TIME;
}

static void ucla_idle_out(ucla_session_t *s)
{
	// do none
}

static void ucla_idle_proc(ucla_session_t *s)
{
	// do none
}

/****************disp**************************************************/
static RS ucla_renew_disp_socket(ucla_session_t *s)
{
	if(!s){
		return RS_ERROR;
	}

	if ((s->disp_sock != INVALID_SOCKET) && 
		(s->disp_sock6 != INVALID_SOCKET)) {
		return RS_OK;
	}

	if(s->disp_sock != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock);
	}
	if(s->disp_sock6 != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock6);
	}
	s->disp_sock = create_udp_server(0, 0);
	if (s->disp_sock == INVALID_SOCKET) {
		if ((s->disp_sock = create_udp_server(0, 0)) == INVALID_SOCKET) {
			return RS_ERROR;
		}
	}
	s->disp_sock6 = create_udp_server6();
	if (s->disp_sock6 == INVALID_SOCKET) {
		if ((s->disp_sock6 = create_udp_server6()) == INVALID_SOCKET) {
			CLOSE_SOCK(s->disp_sock);
			return RS_ERROR;
		}
	}
	
	return RS_OK;

}

static RS ucla_proc_disp_udp(ucla_session_t *s, pkt_t *pkt, bool ipv6, ipc_addr_t *peer_addr)
{
	net_header_t *hdr;
	ucla_disp_res_hdr_t* rh;
	int len;
	u_int8_t *pdata = NULL;

	hdr = (net_header_t *)pkt->data;
	rh = (ucla_disp_res_hdr_t*)(hdr+1);
	len = hdr->param_len;
	
	if(len < sizeof(*rh)){
		log_debug("ucla_proc_disp_udp len=%u sizeof(*rh)=%u\n", len, sizeof(*rh));
		return RS_INVALID_PARAM;
	}
	
    rh->errorno = ntohl(rh->errorno);
	if(rh->errorno != ERR_NONE){
		//TODO: 判断致命错误，告知APP
        log_debug("dispatch return error %u\n",rh->errorno);
		//ucla_err_doname_proc(s, __FUNCTION__);
        ucla_set_status(s,UCLAS_ERROR);
		return RS_INVALID_PARAM;
	}

	if(rh->ip == 0 || rh->port == 0){
		log_debug("%s ip=%u.%u.%u.%u port=%u\n", s->doname, IP_SHOW(htonl(rh->ip)), htons(rh->port));
		//ucla_err_doname_proc(s, __FUNCTION__);
		ucla_set_status(s,UCLAS_ERROR);
		return RS_INVALID_PARAM;
	}

	s->is_ipv6 = ipv6;
	
	s->ip = ntohl(rh->ip);
	s->port = ntohs(rh->port);
	memcpy((void *)&s->ipc_addr, (void *)peer_addr, sizeof(ipc_addr_t));
	memcpy((void *)&s->peer_addr, (void *)peer_addr, sizeof(ipc_addr_t));
	
	s->server_die = false;
	// TODO:添加多端口
	log_debug("%s %s s->ip=%u.%u.%u.%u s->port=%u ntohs(rh->port)=%u s->port_index=%u rh->global_id=%u\n", 
	s->doname, ipv6?"ipv6":"ipv4",IP_SHOW(s->ip), s->port, ntohs(rh->port), s->port_index, rh->global_id);
	
	nd_la_debug(NULL, "%s s->ip=%u.%u.%u.%u s->port=%u ntohs(rh->port)=%u s->port_index=%u rh->global_id=%u\n", 
		s->doname, IP_SHOW(s->ip), s->port, ntohs(rh->port), s->port_index, rh->global_id);

	if (ipv6) {
		memcpy((void *)&s->last_ipv6_addr, (void *)&peer_addr->sockaddr6, sizeof(s->last_ipv6_addr));
		pdata = (u_int8_t *)&s->last_ipv6_addr.sin6_addr;
		memcpy((void *)&pdata[12], (void *)&rh->ip, sizeof(rh->ip));
		memcpy((void *)&s->ipc_addr.sockaddr6, (void *)&s->last_ipv6_addr, sizeof(s->last_ipv6_addr));
	} else {
		s->ipc_addr.sockaddr.sin_addr.s_addr = rh->ip;
	}
	
	// 有目的IP 和端口了，连接
	if (s->has_user) {
		ucla_set_status(s, UCLAS_AUTH_REQ);
	} else {
		ucla_set_status(s, UCLAS_SERVER_AUTH);
	}
	
	return RS_OK;
}

static int _ucla_disp_read(bool ipv6, ucla_session_t *s)
{
	net_header_t *hdr;
	pkt_t *pkt;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	ipc_addr_t peer_addr;
	socklen_t len_addr;

	memset((void *)&peer_addr, 0, sizeof(peer_addr));
	pkt = s->rcv_buf;
	if (ipv6) {
		len_addr = sizeof(addr6);
		pkt->total = (int)recvfrom(s->disp_sock6, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr6, &len_addr);
		memcpy((void *)&peer_addr.sockaddr6, (void *)&addr6, sizeof(addr6));
	} else {
		len_addr = sizeof(addr);
		pkt->total = (int)recvfrom(s->disp_sock, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, &len_addr);
		memcpy((void *)&peer_addr.sockaddr, (void *)&addr, sizeof(addr));
	}
	if (pkt->total < (int)net_header_size) {
		return 0;
	}
	hdr_order(pkt);
	hdr = (net_header_t *)pkt->data;
	if (hdr->ver >= 2) {
		//分配服务器没有v2报文
		return 0;
	}	
	if( pkt->total < (int)(hdr->param_len + net_header_real_size(hdr))){
		log_debug( "Ignore too short udp packet: %d  < %d\n", 
			pkt->total, (hdr->param_len + net_header_real_size(hdr)));
		return 0;
	}
	log_debug("ucla_disp_read %s hdr->command=%u\n", s->doname, hdr->command); 
	nd_la_debug(NULL, "ucla_disp_read %s hdr->command=%u from=%u.%u.%u.%u\n", 
		s->doname, hdr->command, IP_SHOW(htonl(addr.sin_addr.s_addr))); 
	switch(hdr->command){
		case CMD_APP_SERVER_DISP:
			ucla_proc_disp_udp(s,pkt,ipv6, &peer_addr);
			break;
		default:
			break;
	}

	return 0;
}

static int ucla_disp_read(cl_thread_t *thread)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(thread);

	UCLA_ENTER();
	s->t_disp_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_disp_recv, ucla_disp_read, (void *)s, s->disp_sock);
	
	_ucla_disp_read(false, s);
	
	return 0;
}

static int ucla_disp_read6(cl_thread_t *thread)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(thread);

	UCLA_ENTER();
	s->t_disp_recv6 = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_disp_recv6, ucla_disp_read6, (void *)s, s->disp_sock6);

	_ucla_disp_read(true, s);
	
	return 0;
}

static void _ucla_send_disp(ucla_session_t *s, int count, ipc_sock_t *ipc_sock)
{
	pkt_t *pkt;
	ucla_req_disp_hdr_t *dh;
	int i, len, r, j;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	ipc_sock_t *pipc_sock = NULL;
	bool ipv6 = false;
	u_int16_t dst_port[3] = {DISP_SERVER_PORT, DISP_SERVER_PORT1, DISP_SERVER_PORT2};

	log_debug("enter %s doname=%s\n", __FUNCTION__, s->doname);	
	//暂时直接产生数据包，后续有TLV数据时增加函数
	pkt = pkt_new(CMD_APP_SERVER_DISP, sizeof(ucla_req_disp_hdr_t),TP_USER);
	if (pkt == NULL) {
		return;
	}
	((net_header_t *)pkt->data)->handle = 0;
	dh = get_pkt_payload(pkt, ucla_req_disp_hdr_t);
	//dh->apptype = cl_priv->app_type & 0xF;
	dh->apptype = 4;
	dh->time_xor = (u_int32_t)time(NULL);
	//时间取反
	dh->time_xor^=0xFFFFFFFF;
    dh->time_xor = htonl(dh->time_xor);
    dh->apptype = htons(dh->apptype);
	
	if (count) {
		for (i = 0; i < (int)count; i++) {
			pipc_sock = &ipc_sock[i];
			for(j = 0; j < 3; j++) {
				if (s->is_ipv6 && (ipc_sock[i].ai_family == AF_INET6)) {
					memcpy((void *)&addr6, (void *)pipc_sock->addr.sockaddrc, sizeof(addr6));
					addr6.sin6_port = htons(dst_port[j]);
					len = sizeof(addr6);
					r = (int)sendto(s->disp_sock6 , pkt->data, pkt->total, 0, (struct sockaddr *)&addr6, len);
					ipv6 = true;
					//log_debug("ipv6 %s %d send r=%u\n", __FUNCTION__, __LINE__, r);
				} else if (!s->is_ipv6 && (ipc_sock[i].ai_family == AF_INET)){
					memcpy((void *)&addr, (void *)pipc_sock->addr.sockaddrc, sizeof(addr));
					addr.sin_port = htons(dst_port[j]);
					len = sizeof(addr);
					r = (int)sendto(s->disp_sock , pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len);
					ipv6 = false;
					//log_debug("ipv4 %s %d send r=%u ip=%u.%u.%u.%u\n", __FUNCTION__, __LINE__, r, IP_SHOW(htonl(addr.sin_addr.s_addr)));
				}
				if (r < 0) {
					log_debug("%s %d send r=%u %s\n", __FUNCTION__, __LINE__, r, ipv6?"ipv6":"ipv4");
					nd_la_debug(NULL, "%s %d send r=%u %s\n", __FUNCTION__, __LINE__, r, ipv6?"ipv6":"ipv4");
				}
			}
		}
	}

	pkt_free(pkt);
}

static void ucla_send_disp(ucla_session_t *s)
{
	int count;
	ipc_sock_t ipc_sock[DNS_IP_MAX_NUM];
	
	if (s->ipc_count) {
		log_debug("enter %s go to use dns cache\n", __FUNCTION__);
		_ucla_send_disp(s, s->ipc_count, s->ipc_sock);
	}

	count = 0;
	memset(&ipc_sock, 0, sizeof(ipc_sock));
	if(!cl_get_addr_by_doname(s->doname, ipc_sock, &count)){
		log_debug("%s not found ip\n", s->doname);
		nd_la_debug(NULL, "%s not found ip\n", s->doname);
		return;
	}

	_ucla_send_disp(s, count, ipc_sock);
}

static int ucla_disp_timer(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);

	s->t_timer = NULL;
	
	CL_THREAD_TIMER_ON(&cl_priv->master, s->t_timer, ucla_disp_timer, (void *)s, TIME_TRY_DISP);
	ucla_send_disp(s);

	return 0;
}

static int disp_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	log_debug("%s disp_die die\n", s->doname);
	
	if (s->last_status == UCLAS_DIS_PROBE) {
		ucla_err_doname_proc(s, (char *)__FUNCTION__);
	}
	
	s->timeout_count = 5;
	ucla_set_status(s, UCLAS_IDLE);
	
	return 0;
}

static void ucla_disp_into(ucla_session_t *s)
{
	ucla_reset_send(s);

	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;
	ucla_off_all_timer(s);
	if(ucla_renew_disp_socket(s) == RS_OK){
		CL_THREAD_TIMER_ON(MASTER, s->t_die, disp_die, (void *)s, TIME_N_SECOND(5));
		CL_THREAD_READ_ON(MASTER, s->t_disp_recv6, ucla_disp_read6, (void *)s, s->disp_sock6);
		CL_THREAD_READ_ON(MASTER, s->t_disp_recv, ucla_disp_read, (void *)s, s->disp_sock);
		CL_THREAD_TIMER_ON(MASTER, s->t_timer, ucla_disp_timer,
			(void *)s, range_rand(0, 100));	
	}else{
		ucla_set_status(s,UCLAS_IDLE);
	}
}

static void ucla_disp_out(ucla_session_t *s)
{
	CL_THREAD_OFF(s->t_recv);
	ucla_udp_socket(s, s->is_ipv6);

	if (s->sock != INVALID_SOCKET) {
		if (s->is_ipv6) {
			CL_THREAD_READ_ON(MASTER, s->t_recv, ucla_read6, (void *)s, s->sock);
		} else {
			CL_THREAD_READ_ON(MASTER, s->t_recv, ucla_read, (void *)s, s->sock);
		}
	} else {
		log_err(true, "ucla_udp_socket failed\n");
	}
	log_debug("ucla_disp_into goto %s s->sock=%u!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 
		s->is_ipv6?"ipv6":"ipv4", s->sock);
}

static void ucla_disp_proc(ucla_session_t *s)
{
	// do none
}

/***************************************server probe**************************************************************/
static void _ucla_send_disp_probe(ucla_session_t *s, int count, ipc_sock_t *ipc_sock)
{
	pkt_t *pkt;
	u_int8_t *value;
	int i, len, r, j;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	ipc_sock_t *pipc_sock = NULL;
	u_int64_t any_sn = 0;
	u_int8_t doname_buf[800];
	u_int16_t doname_len = 0;
	bool ipv6 = false;
	u_int8_t tlv_buf[1024];
	u_int32_t *pip = (u_int32_t *)tlv_buf;
	uc_tlv_t* tlv = (uc_tlv_t *)&pip[1];
	u_int16_t tlv_len = sizeof(u_int32_t);
	u_int16_t dst_port[3] = {APP_REGSERVER_PORT1, APP_REGSERVER_PORT2, DEF_DISPATCHER_UDP_PORT};

	log_debug("enter %s doname=%s\n", __FUNCTION__, s->doname);
	//为了服务器兼容，处理下，服务器默认写死第一个u32是测试ip，第二个u64是sn......
	memset(tlv_buf, 0, sizeof(tlv_buf));
	*pip = 0;
	//可能的sn
	any_sn = la_get_any_sn();
	if (any_sn != 0) {
		//算了，这里上传sn给服务器判断暂时没啥必要，感觉手机还是连接最快的服务器靠外网ip判断比较好，不必非要登陆到设备连接的服务器上去，
		//因为各服务器是有同步的
		//*psn = ntoh_ll(any_sn);
		tlv->type = htons(UCT_SERVER_PORBE_SN);
		tlv->len = htons(sizeof(any_sn));
		*(u_int64_t *)tlv_val(tlv) = ntoh_ll(any_sn);
		tlv_len += sizeof(*tlv) + sizeof(any_sn);
		tlv = tlv_n_next(tlv);
	}
	//appid
	tlv->type = htons(UCT_SERVER_PORBE_APP_ID);
	tlv->len = htons(sizeof(u_int32_t));
	*(u_int32_t *)tlv_val(tlv)= htonl(cl_priv->app_id);
	tlv_len += sizeof(*tlv) + sizeof(cl_priv->app_id);
	tlv = tlv_n_next(tlv);
	//vvid
	tlv->type = htons(UCT_SERVER_PORBE_VVID);
	tlv->len = htons(sizeof(u_int32_t));
	*(u_int32_t *)tlv_val(tlv)= htonl(cl_priv->vvid);
	tlv_len += sizeof(*tlv) + sizeof(cl_priv->vvid);
	tlv = tlv_n_next(tlv);
	//vendor
	tlv->type = htons(UCT_SERVER_PORBE_VENDOR);
	tlv->len = htons(sizeof(cl_priv->oem_vendor));
	memcpy((void *)tlv_val(tlv), (void *)cl_priv->oem_vendor, sizeof(cl_priv->oem_vendor));
	tlv_len += sizeof(*tlv) + sizeof(cl_priv->oem_vendor);
	tlv = tlv_n_next(tlv);
	//可能错误的域名
	doname_len = la_get_err_doname(doname_buf);
	if (doname_len != 0) {
		tlv->type = htons(UCT_SERVER_PROBE_ERR_DONAME);
		tlv->len = htons(doname_len);
		memcpy((void *)tlv_val(tlv), (void *)doname_buf, doname_len);
		tlv = tlv_n_next(tlv);
	}
	//暂时直接产生数据包，后续有TLV数据时增加函数
	pkt = pkt_new(CMD_UDP_DONAME_PROB, tlv_len, TP_USER);
	if (pkt == NULL) {
		return;
	}
	value = get_pkt_payload(pkt, u_int8_t);
	//添加tlv
	if (tlv_len) {
		memcpy((void *)value, tlv_buf, tlv_len);
	}
	if (count) {
		for (i = 0; i < (int)count; i++) {
			pipc_sock = &ipc_sock[i];
			for(j = 0; j < 3; j++) {
				if (ipc_sock[i].ai_family == AF_INET) {
					memcpy((void *)&addr, (void *)pipc_sock->addr.sockaddrc, sizeof(addr));
					addr.sin_port = htons(dst_port[j]);
					len = sizeof(addr);
					r = (int)sendto(s->disp_sock , pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len);
					ipv6 = false;
					//log_debug("%s %d send r=%u  ip=%u.%u.%u.%u s->disp_sock=%u\n", __FUNCTION__, __LINE__, r, IP_SHOW(htonl(addr.sin_addr.s_addr)), s->disp_sock);
				} else {
					memcpy((void *)&addr6, (void *)pipc_sock->addr.sockaddrc, sizeof(addr6));
					addr6.sin6_port = htons(dst_port[j]);
					len = sizeof(addr6);
					r = (int)sendto(s->disp_sock6 , pkt->data, pkt->total, 0, (struct sockaddr *)&addr6, len);
					ipv6 = true;
					//log_debug("%s %d send r=%u s->disp_sock6=%u\n", __FUNCTION__, __LINE__, r, s->disp_sock6);
				}
				if (r < 0) {
					log_debug("%s %d send r=%u %s\n", __FUNCTION__, __LINE__, r, ipv6?"ipv6":"ipv4");
					nd_la_debug(NULL, "%s %d send r=%u %s\n", __FUNCTION__, __LINE__, r, ipv6?"ipv6":"ipv4");
				}
			}
		}
	}

	pkt_free(pkt);
}

static void ucla_send_disp_probe(ucla_session_t *s)
{
	int count;
	ipc_sock_t ipc_sock[DNS_IP_MAX_NUM];
	
	if (s->ipc_count) {
		_ucla_send_disp_probe(s, s->ipc_count, s->ipc_sock);
	}

	count = 0;
	memset(&ipc_sock, 0, sizeof(ipc_sock));
	if(!cl_get_addr_by_doname(s->doname, ipc_sock, &count)){
		log_debug("%s not found ip\n", s->doname);
		nd_la_debug(NULL, "%s not found ip\n", s->doname);
		return;
	}

	_ucla_send_disp_probe(s, count, ipc_sock);
}

static int ucla_disp_probe_timer(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);

	s->t_timer = NULL;
	
	CL_THREAD_TIMER_ON(&cl_priv->master, s->t_timer, ucla_disp_probe_timer, (void *)s, TIME_TRY_DISP);
	ucla_send_disp_probe(s);
	
	return 0;
}

static void ucla_session_sync(ucla_session_t *s)
{
	la_user_t *puser;
	la_home_t *phome, *phomen;
	la_member_t *pmem, *pmemn;

	if (!s) {
		return;
	}

	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}

	puser->session_ptr = s;
	stlc_list_for_each_entry_safe(la_home_t, phome, phomen, &puser->home_link, link) {
		phome->session_ptr = s;
		stlc_list_for_each_entry_safe(la_member_t, pmem, pmemn, &phome->member_link, link) {
			pmem->session_ptr = s;
		}
	}
}

static void ucla_session_copy(ucla_session_t *s, ucla_session_t *ss)
{
	if (!s || !ss) {
		return;
	}

	s->has_user = ss->has_user;
	s->has_def_home = ss->has_def_home;
	s->user_id = ss->user_id;
	//给圈子，用户同步session
	ucla_session_sync(s);
	ss->has_user = false;
	ss->user_id = 0;
	ss->has_def_home = false;
	memcpy((void *)s->username, (void *)ss->username, sizeof(s->username));
	memcpy((void *)s->key, (void *)ss->key, sizeof(s->key));
	memcpy((void *)s->uuid, (void *)ss->uuid, sizeof(s->uuid));
	//memdumpone("ucla_session_copy uuid", s->uuid, 16);
	_ucla_user_conf_save(s);
	_ucla_user_conf_save(ss);
}

static void ucla_other_session_goto_err(ucla_session_t *s)
{
	ucla_session_t *sc, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, sc, sn, &plc->server_client, link) {
		if (s && (sc == s)) {
			continue;
		}
		if (sc->status == UCLAS_ERROR) {
			continue;
		}
		ucla_set_status(sc, UCLAS_ERROR);
	}
}

static void ucla_other_session_goto_idle(ucla_session_t *s)
{
	ucla_session_t *sc, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, sc, sn, &plc->server_client, link) {
		if (s && (sc == s)) {
			continue;
		}
		ucla_set_status(sc, UCLAS_IDLE);
	}
}

static void ucla_err_doname_proc(ucla_session_t *s, char *func)
{
	la_doname_info_t *pd;
	
	if (!s || !func) {
		return;
	}
	log_debug("enter ucla_err_doname_proc called by %s\n", func);
	pd = la_doname_find(true, s->doname);
	if (pd) {
		if (!la_doname_find(false, pd->valid_doname)) {
			stlc_list_del(&pd->link);
			la_doname_add2(false, pd);
		}
	}
	ucla_other_session_goto_idle(s);
}

static void do_session_goto_disp(char *doname)
{
	ucla_session_t *s;
	ipc_sock_t ipc_sock[DNS_IP_MAX_NUM];
	int count = 0;

	UCLA_ENTER();
	if (!doname) {
		return;
	}
	s = ucla_get_ses_by_doname_only(doname);
	if (!s) {
		return;
	}
	if(!cl_get_addr_by_doname(s->doname, ipc_sock, &count)){
		return;
	}
	if (count == 0) {
		return;
	}
	if (s->status < UCLAS_DISPATCH) {
		//兼容处理，获取以前没升级版本的用户名及密码。。。。
		do_session_switch_check(s);
		ucla_set_status(s,UCLAS_DISPATCH);
		//将其他session全部err去
		ucla_other_session_goto_err(s);
	}
	UCLA_EXIT();
}

static void do_session_switch_check(ucla_session_t *s)
{
	ucla_session_t *ss = plc->need_switch_session;

	if (ss && 
		(ss != s)) {
		plc->need_switch_session = s;
		//这里session切换，服务器切换，需要重新保存下账号，这里需要保存下用户名称
		s->need_save_username = true;
		ss->need_save_username = true;
		ss->need_clean_conf = true;
		ucla_session_copy(s, ss);
	}
}

static RS ucla_proc_disp_probe_udp(ucla_session_t *s, pkt_t *pkt, bool ipv6)
{
	net_header_t *hdr;
	u_int32_t ip;
	char *doname; 
	u_int8_t *pdata;
	char res_doname[64];
	ucla_session_t *rs = NULL;
//	ucla_session_t *ss = plc->need_switch_session;

	hdr = (net_header_t *)pkt->data;
	if (hdr->param_len <= sizeof(u_int32_t)) {
		log_err(false, "ucla_proc_disp_probe_udp Bad CMD_UDP_DONAME_PROB lenght, except %d bytes\n", sizeof(net_device_location_t));
		return RS_ERROR;
	}
	pdata = get_net_ucp_payload(hdr, u_int8_t);
	pdata[hdr->param_len] = 0;
	ip = htonl(*(u_int32_t *)pdata);
	doname = (char *)&pdata[sizeof(u_int32_t)];
	log_debug("ucla_proc_disp_probe_udp getip=%u.%u.%u.%u doname=%s\n", 
		IP_SHOW(ip), doname);
	
	nd_la_debug(NULL, "ucla_proc_disp_probe_udp getip=%u.%u.%u.%u doname=%s\n", 
		IP_SHOW(ip), doname);
	//处理tlv
	//do_ucla_bind_a_tlv(s, hdr->param_len-sizeof(net_device_location_t), (uc_tlv_t*)(loc+1));
	//处理返回doname
	la_doname_clean(true);
	//doname = "sg.ice.galaxywind.com";
	//la_doname_add(true, "sg.ice.galaxywind.com");
	la_doname_add(true, doname);

	//可能是oem域名，不存在，创建
	if (!ucla_get_ses_by_doname(doname)) {
		log_debug("need create new session doname=%s\n", doname);
		nd_la_debug(NULL, "need create new session doname=%s\n", doname);
		rs = ucla_new(doname);
		if (!rs) {
			log_err(true, "ucla_new failed\n");
			return RS_ERROR;
		}
		memset((void *)res_doname, 0, sizeof(res_doname));
		strcpy(res_doname, doname);
		disp_resolv_doname(1, (const char (*const )[64])res_doname);
		ucla_conf_init(doname);
		ucla_set_status(s, UCLAS_ERROR);
		s = rs;
		//需要保存
		plc->need_doname_save = true;
	}

	//如果不是自己
	if (!la_doname_find(true, s->doname)) {
		//这里优化一下，如果对于doname有解析，就直接帮相应session进入dispatch好了
		do_session_goto_disp(doname);
		log_debug("the valid doname is not %s\n", s->doname);
		ucla_set_status(s, UCLAS_ERROR);
		return RS_OK;
	}
	//兼容处理，获取以前没升级版本的用户名及密码。。。。
	do_session_switch_check(s);
	
	s->is_ipv6 = ipv6;
	ucla_set_status(s, UCLAS_DISPATCH);
	//将其他session全部err去
	ucla_other_session_goto_err(s);
	
	return RS_OK;
}

static int _ucla_disp_probe_read(bool ipv6, ucla_session_t *s)
{
	net_header_t *hdr;
	pkt_t *pkt;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	socklen_t len_addr;

	pkt = s->rcv_buf;
	if (ipv6) {
		len_addr = sizeof(addr6);
		pkt->total = (int)recvfrom(s->disp_sock6, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr6, &len_addr);
	} else {
		len_addr = sizeof(addr);
		pkt->total = (int)recvfrom(s->disp_sock, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, &len_addr);
	}
	if (pkt->total < (int)net_header_size) {
		log_debug("ucla_disp_probe_read %s total=%u\n", s->doname, pkt->total);
		return 0;
	}
	hdr_order(pkt);
	hdr = (net_header_t *)pkt->data;
	if (hdr->ver >= 2) {
		log_debug("ucla_disp_probe_read %s ver>2\n", s->doname);
		nd_la_debug(NULL, "ucla_disp_probe_read %s ver>2\n", s->doname);
		return 0;
	}	
	if( pkt->total < (int)(hdr->param_len + net_header_real_size(hdr))){
		log_debug( "Ignore too short udp packet: %d  < %d\n", 
			pkt->total, (hdr->param_len + net_header_real_size(hdr)));
		nd_la_debug(NULL, "Ignore too short udp packet: %d  < %d\n", 
			pkt->total, (hdr->param_len + net_header_real_size(hdr)));
		return 0;
	}
	log_debug("ucla_disp_probe_read %s hdr->command=%u\n", 
		s->doname, hdr->command);
	nd_la_debug(NULL, "ucla_disp_probe_read %s hdr->command=%u from=%u.%u.%u.%u\n", 
		s->doname, hdr->command, IP_SHOW(htonl(addr.sin_addr.s_addr)));
	
	switch(hdr->command){
		case CMD_UDP_DONAME_PROB:
			ucla_proc_disp_probe_udp(s,pkt, ipv6);
			break;
		default:
			break;
	}

	return 0;
}

static int ucla_disp_probe_read(cl_thread_t *thread)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(thread);
	
	s->t_disp_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_disp_recv, ucla_disp_probe_read, (void *)s, s->disp_sock);
	_ucla_disp_probe_read(false, s);

	return 0;
}

static int ucla_disp_probe_read6(cl_thread_t *thread)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(thread);
	
	s->t_disp_recv6 = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_disp_recv6, ucla_disp_probe_read6, (void *)s, s->disp_sock6);
	_ucla_disp_probe_read(true, s);

	return 0;
}

static int disp_probe_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	log_debug("%s disp_probe_die die\n", s->doname);

	s->timeout_count = 5;
	ucla_set_status(s, UCLAS_IDLE);
	
	return 0;
}

static void ucla_disp_probe_into(ucla_session_t *s)
{
	ucla_reset_send(s);

	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;

	ucla_off_all_timer(s);
	if(ucla_renew_disp_socket(s) == RS_OK){
		CL_THREAD_TIMER_ON(MASTER, s->t_die, disp_probe_die, (void *)s, TIME_N_SECOND(5));
		CL_THREAD_READ_ON(MASTER, s->t_disp_recv, ucla_disp_probe_read, (void *)s, s->disp_sock);
		//ipv6
		CL_THREAD_READ_ON(MASTER, s->t_disp_recv6, ucla_disp_probe_read6, (void *)s, s->disp_sock6);
		CL_THREAD_TIMER_ON(MASTER, s->t_timer, ucla_disp_probe_timer,
			(void *)s, range_rand(0, 100));	
	}else{
		ucla_set_status(s,UCLAS_IDLE);
	}
}

/*******************************server auth***********************************************************/
static int sa_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	s->server_die = true;
	log_debug("%s sa_die die\n", s->doname);

	s->timeout_count = 5;
	//端口没测试完，重来
	s->port_index++;
	if (s->port_index < UCLA_MAX_PORTS) {
		ucla_set_status(s, UCLAS_SERVER_AUTH);
		return 0;
	}

	if (s->last_status == UCLAS_DISPATCH) {
		ucla_err_doname_proc(s, (char *)__FUNCTION__);
	}
	ucla_set_status(s, UCLAS_IDLE);
	return 0;
}
#if 0
static void ucla_sa_into_norsa(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_req_t *ar = NULL;
	
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_req_t),
				true, false, 0,	0, 0, 0);
    ar = get_ucp_payload(pkt, uc_app_user_req_t);
	ar->action = UCAU_HELLO;
	memcpy(&ar->randon, &s->r1, sizeof(ar->randon));
	memcpy((void *)&ar->rsa, (void *)&ar->randon, sizeof(ar->randon));
	
	ucla_request_add(s, pkt);	
}

static bool ucla_rsa_enc_check(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_req_t *ar = NULL;
	cl_la_rsa_t *rsa = plc->rsa_ctrl;
	
	UCLA_ENTER();
	if (!rsa->finish) {
		return false;
	}

	if (rsa->to_len != APP_USER_RSA_LEN) {
		log_debug("rsa enc len is err len=%u\n", rsa->to_len);
		return false;
	}
	
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_req_t),
				true, false, 0,	0, 0, 0);
    ar = get_ucp_payload(pkt, uc_app_user_req_t);
	ar->action = UCAU_HELLO;
	ar->flag |= BIT(0 );
	memcpy(&ar->randon, &s->r1, sizeof(ar->randon));
	memcpy((void *)&ar->rsa, (void *)rsa->to, sizeof(ar->rsa));
	//memdump("ucla_rsa_enc_check", rsa->to, 128);
	
	ucla_request_add(s, pkt);

	return true;
}

static int ucla_rsa_enc_timer(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);

	s->t_timer = NULL;

	if (!ucla_rsa_enc_check(s)) {
		CL_THREAD_TIMER_ON(&cl_priv->master, s->t_timer, ucla_rsa_enc_timer, (void *)s, TIME_N_SECOND(1));
	}

	return 0;
}

static void ucla_sa_into_rsa(ucla_session_t *s)
{
	cl_la_rsa_t *rsa = plc->rsa_ctrl;

	rsa->finish = false;
	rsa->action = ACT_RSA_ENC;
	rsa->from_len = sizeof(s->rand1);
	rsa->to_len = 0;
	memcpy((void *)rsa->from, (void *)s->rand1, rsa->from_len);

	//开启超时定时器
	CL_THREAD_OFF(s->t_timer);
	CL_THREAD_TIMER_ON(MASTER, s->t_timer, ucla_rsa_enc_timer, (void *)s, TIME_N_SECOND(1));	
	event_push_err(plc->callback, LA_RSA_REQ, 0, plc->callback_handle, (int)rsa);
}
#endif

static void ucla_sa_into(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_req_t *ar = NULL;
	u_int8_t buf[1024];
	
	ucla_reset_send(s);
	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;
	ucla_off_all_timer(s);

	if (s->r1 == 0) {
		srand(get_sec());
	    s->r1 = rand();
	}

	CL_THREAD_OFF(s->t_die);
	//5秒超时后下个端口
	CL_THREAD_TIMER_ON(MASTER, s->t_die, sa_die, (void *)s, TIME_N_SECOND(10));
	
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_req_t),
				true, false, 0,	0, 0, 0);
    ar = get_ucp_payload(pkt, uc_app_user_req_t);
	ar->action = UCAU_HELLO;
	memcpy((void *)ar->randon, (void *)&s->r1, sizeof(ar->randon));
	if (plc->use_rsa && plc->rsa_enc) {
		ar->flag |= BIT(0);
		memset(buf, 0, sizeof(buf));
		plc->rsa_enc(plc->rsa_priv_len, plc->rsa_priv, sizeof(ar->randon), ar->randon, buf);
		memcpy((void *)ar->rsa, (void *)buf, sizeof(ar->rsa));
	} else {
		memcpy((void *)ar->rsa, (void *)ar->randon, sizeof(ar->randon));
	}
	pkt->action = ACTION_ALL_PORT_SEND;
	ucla_request_add(s, pkt);
}

static void ucla_sa_out(ucla_session_t *s)
{
	// do none
}

static void ucla_userid_sync(ucla_session_t *s, u_int32_t user_id)
{
	la_user_t *puser;
	
	UCLA_ENTER();
	log_debug("%s user_id=%u suser_id=%u\n", s->doname, user_id, s->user_id);
	nd_la_debug(NULL, "%s user_id=%u suser_id=%u\n", s->doname, user_id, s->user_id);
	puser = ucla_find_user_by_id(s->user_id);
	if (puser) {
		if (s->user_id != user_id) {
			puser->conf.user_id = s->user_id = user_id;
			ucla_user_conf_save(s);
		}
	} else {
		puser = ucla_user_new();
		if (!puser) {
			return;
		}
		puser->conf.user_id = s->user_id = user_id;
		memcpy((void *)&puser->conf.user_name, (void *)s->username, sizeof(puser->conf.user_name));
		memcpy((void *)&puser->conf.passwd, (void *)s->key, sizeof(puser->conf.passwd));
		memcpy((void *)&puser->conf.uuid, (void *)s->uuid, sizeof(puser->conf.uuid));
		
		ucla_user_add(s, puser);
	}
}

#if 0
static bool ucla_rsa_dec_check(ucla_session_t *s)
{
	cl_la_rsa_t *rsa = plc->rsa_ctrl;
	
	UCLA_ENTER();
	if (!rsa->finish) {
		return false;
	}

	log_debug("rsa declen=%u\n", rsa->to_len);
	//memdump("rsa to", rsa->to, 4);
	//memdump("rand1", s->rand1, 4);
	if (memcmp((void *)rsa->to, (void *)s->rand1, 4) == 0) {
		ucla_set_status(s,UCLAS_USER_REGISTER);
	} else {
		ucla_set_status(s,UCLAS_IDLE);
	}

	return true;
}


static int ucla_rsa_dec_timer(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);

	s->t_timer = NULL;

	if (!ucla_rsa_dec_check(s)) {
		CL_THREAD_TIMER_ON(&cl_priv->master, s->t_timer, ucla_rsa_dec_timer, (void *)s, TIME_N_SECOND(1));
	}

	return 0;
}
#endif

static void do_sa_hello(ucla_session_t *s, uc_app_user_reg_ret_t *rd)
{
	u_int8_t buf[1024];
	
	if (!plc->use_rsa) {
		ucla_set_status(s,UCLAS_USER_REGISTER);
		return;
	}

	if (plc->rsa_dec) {
		plc->rsa_dec(plc->rsa_priv_len, plc->rsa_priv, sizeof(rd->rsa), rd->rsa, buf);
		if (memcmp(buf, (void *)&s->r1, sizeof(s->r1)) == 0) {
			ucla_set_status(s,UCLAS_USER_REGISTER);
			return;
		}
	}

	log_debug("do_sa_hello rsa dec failed\n");
}

static void ucla_sa_proc(ucla_session_t *s)
{
	ucph_t *hdr;
	uc_app_user_reg_ret_t *rd;
		
	hdr = (ucph_t *)s->rcv_buf->data;

	rd = get_ucp_payload(s->rcv_buf, uc_app_user_reg_ret_t);
	switch (rd->action) {
	case UCAU_HELLO:
		do_sa_hello(s, rd);
		break;
	default:
		break;
	}
}
/****************************rsa wait dec*************************************************************************/

/*****************************************************************************************************/
/*******************************user reg***********************************************************/
static int ur_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;

	s->timeout_count = 5;
	log_debug("%s sa_die die\n", s->doname);
	ucla_set_status(s, UCLAS_IDLE);

	return 0;
}

static void ucla_get_rand_uuid_str(u_int8_t *uuid)
{
	MD5_CTX ctx;
	char msec[30];
	u_int64_t time1 = get_sec();
	u_int64_t time2 = get_msec();
	u_int32_t pid = (u_int32_t)MY_THREAD_ID;

	memset((void *)msec, 0, sizeof(msec));
	sprintf(msec, "%"PRIu64"", time1*1000 + time2%1000);
	//log_debug("ucla_get_rand_uuid_str =%s time1=%"PRIu64" sec=%llu \n", msec, time1, time2);
	MD5Update(&ctx, cl_priv->uuid, sizeof(cl_priv->uuid));
	MD5Update(&ctx, (u_int8_t *)msec, sizeof(msec));
	MD5Update(&ctx, (u_int8_t *)&pid, sizeof(pid));
	MD5Final(uuid, &ctx);
	uuid[15] = 0;
}

static void ucla_ur_into(ucla_session_t *s)
{
	pkt_t *pkt;
	MD5_CTX ctx;
	char buf[16];
	uc_app_user_create_req_t *acr = NULL;
	
	ucla_reset_send(s);
	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;
	ucla_off_all_timer(s);

	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, ur_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_create_req_t),
				true, false, 0,	0, 0, 0);
    acr  = get_ucp_payload(pkt, uc_app_user_create_req_t);
	acr ->action = UCAU_CREATE;

	memset(buf, 0, sizeof(buf));
	if (s->r1 == 0) {
		srand(get_sec());
	    s->r1 = rand();
	}
	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)&s->r1, sizeof(s->r1));
	MD5Final(s->key, &ctx);
	ucla_get_rand_uuid_str((u_int8_t *)buf);
	memcpy(s->username, buf, sizeof(s->username));
	memcpy(acr->username, s->username, sizeof(s->username));
	memcpy(acr->uuid, s->uuid, sizeof(s->uuid));
	memcpy(acr->passwd_md5, s->key, sizeof(s->key));
	// TODO:这里私钥加密没看懂，等问哈大象到。
	//memdumpone("ucla_ur_into name", acr->username, sizeof(acr->username));
	//memdumpone("ucla_ur_into passwd", acr->passwd_md5, sizeof(acr->passwd_md5));

	ucla_request_add(s, pkt);	
}

static void ucla_ur_out(ucla_session_t *s)
{
	// do none
}

static void do_sa_create(ucla_session_t *s, uc_app_user_create_ret_t *rd)
{
	la_user_t *puser = NULL;
	
	rd->result = htons(rd->result);
	rd->userid = htonl(rd->userid);

	s->user_id = rd->userid;
	log_debug("%s get new userid=%08x rd->result=%u\n", __FUNCTION__, s->user_id, rd->result);
	/*判断创建是否成功,这里可能有两种失败情况，
	*一种服务器出问题，一种是手机账号登陆时被其他手机登陆时先创建了
	*/
	if (!plc->has_phone && rd->result) {
		//失败
		log_debug("enter do_sa_create %s rd->result=%u\n", s->doname, rd->result);
		nd_la_debug(NULL, "enter do_sa_create %s rd->result=%u\n", s->doname, rd->result);
		nd_la_memdump("s->username", s->username, sizeof(s->username));
		nd_la_memdump("s->key", s->key, sizeof(s->key));
		nd_la_memdump("s->uuid", s->uuid, sizeof(s->uuid));
		ucla_set_status(s, UCLAS_ERROR);
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		puser = ucla_user_new();
		if (!puser) {
			ucla_set_status(s, UCLAS_ERROR);
			return;
		}
	}
	puser->conf.user_id = s->user_id;
	memcpy((void *)&puser->conf.user_name, (void *)s->username, sizeof(puser->conf.user_name));
	memcpy((void *)&puser->conf.passwd, (void *)s->key, sizeof(puser->conf.passwd));
	memcpy((void *)&puser->conf.uuid, (void *)s->uuid, sizeof(puser->conf.uuid));

	//这里刚创建账号，这里需要保存下用户名称
	s->need_save_username = true;
	ucla_user_add(s, puser);
	s->has_user = true;
	ucla_set_status(s,UCLAS_AUTH_REQ);
}

static void ucla_ur_proc(ucla_session_t *s)
{
	ucph_t *hdr;
	uc_app_user_create_ret_t *rd;
		
	hdr = (ucph_t *)s->rcv_buf->data;

	rd = get_ucp_payload(s->rcv_buf, uc_app_user_create_ret_t);
	log_debug("enter ucla_ur_proc cmd=%u rd->action=%u\n", hdr->command, rd->action);	
	switch (rd->action) {
	case UCAU_CREATE:
		do_sa_create(s, rd);
		break;
	default:
		break;
	}	
}

/**********************************auth req**************************************************/
static int la_req_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;

	s->server_die = true;
	log_debug("%s la_req_die die\n", s->doname);
	nd_la_debug(NULL, "%s la_req_die die\n", s->doname);

	s->timeout_count = 5;
	//端口没测试完，重来
	s->port_index++;
	if (s->port_index < UCLA_MAX_PORTS) {
		ucla_set_status(s, UCLAS_AUTH_REQ);
		return 0;
	}
	
	if (s->last_status == UCLAS_DISPATCH) {
		ucla_err_doname_proc(s, (char *)__FUNCTION__);
	}
	ucla_set_status(s, UCLAS_IDLE);

	return 0;
}

static void ucla_auth_req_into(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_auth_t *aua = NULL;
	
	ucla_reset_send(s);
	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;
	ucla_off_all_timer(s);

	if (s->time_diff == 0) {
		s->time_diff = (u_int32_t)get_msec();
	}

	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, la_req_die, (void *)s, TIME_N_SECOND(10));	

	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_auth_t),
				true, false, 0,	0, 0, 0);
    aua  = get_ucp_payload(pkt, uc_app_user_auth_t);
	aua ->action = UCAU_AUTH;
	if (s->r1 == 0) {
		srand(get_sec());
	    s->r1 = rand();
	}
	aua->version = 2;
	//标志用户是否是强用户，用1 和2是为了兼容以前的版本
	if (ucla_is_phone_session(s)) {
		aua->flag = 1;
	} else {
		aua->flag = 2;
	}
	memcpy((void *)s->rand1, (void *)&s->r1, sizeof(aua->random1));
	memcpy((void *)aua->random1, (void *)&s->r1, sizeof(aua->random1));
	memcpy((void *)aua->username, (void *)s->username, sizeof(aua->username));
	memcpy((void *)aua->uuid, (void *)s->uuid, sizeof(aua->uuid));
	nd_la_debug(s, "ucla_auth_req_into username=%s !!!!!!!!!!!!!!!!!!!!!!!!!!\n", s->username);
	//memdumpone("ucla_auth_req_into aua->username", aua->username, sizeof(aua->username));
	//log_debug("username=%s !!!!!!!!!!!!!!!!!!!!!!!!!!\n", s->username);
	//memdumpone("ucla_auth_req_into uuid", s->uuid, 16);
	pkt->action = ACTION_ALL_PORT_SEND;
	ucla_request_add(s, pkt);
}

static void ucla_auth_req_out(ucla_session_t *s)
{
	// do none
}

static void do_sa_anser(ucla_session_t *s, uc_app_user_answer_t *rd)
{
	memcpy((void *)s->rand2, rd->random2, sizeof(s->rand2));
	ucla_set_status(s, UCLAS_AUTH_ANSWER);
}

static void ucla_auth_req_proc(ucla_session_t *s)
{
	ucph_t *hdr;
	uc_app_user_answer_t *rd;
		
	hdr = (ucph_t *)s->rcv_buf->data;

	rd = get_ucp_payload(s->rcv_buf, uc_app_user_answer_t);
	log_debug("%s enter ucla_auth_req_proc rd->action=%u  cmd=%u hdr->client_sid=%08x hdr->device_sid=%08x\n", 
		s->doname, rd->action, hdr->command, hdr->client_sid, hdr->device_sid);

	if (hdr->command == CMD_UDP_KEEPLIVE) {
		plc->acount_not_exist = false;
		ucla_is_reset_pkt(s, hdr);
		//当账号不存在，直接error不要一直连接了
		if (plc->acount_not_exist) {
			nd_la_memdump("s->username is not exist", s->username, sizeof(s->username));
			ucla_set_status(s, UCLAS_ERROR);
		}
		return;
	}
	
	switch (rd->action) {
	case UCAU_QUESTION:
		s->client_sid = hdr->client_sid;
		s->device_sid = hdr->device_sid;
		do_sa_anser(s, rd);
		break;
	default:
		break;
	}	
}

/**********************************auth answer**************************************************/
static int answer_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;

	log_debug("%s sa_die die\n", s->doname);

	s->timeout_count = 5;
	ucla_set_status(s, UCLAS_AUTH_REQ);

	return 0;
}
static void ucla_auth_answer_into(ucla_session_t *s)
{
	pkt_t *pkt;
	MD5_CTX ctx;
	int len = 0;
	uc_app_user_question_t *acr = NULL;
	uc_tlv_t *ptlv = NULL;
	u_int8_t *pver = NULL;
	u_int8_t *developer_id = app_get_developer_id();
	
	ucla_reset_send(s);
	s->my_request_id = 0;
	s->peer_request_id = 0;
	ucla_off_all_timer(s);

	len = sizeof(*ptlv) + 4;

	if (strlen(developer_id)) {
		len += (int)sizeof(*ptlv) + (int)strlen(developer_id);
	}
	
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, answer_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(uc_app_user_question_t) + len,
				true, false, 0,	s->client_sid, s->device_sid, 0);
    acr  = get_ucp_payload(pkt, uc_app_user_question_t);
	acr->action = UCAU_ANSWER;
	acr->support_enc = htons(0x1);//先直接这样，表示aes加密
	
	//tlv ver
	ptlv = (uc_tlv_t *)&acr[1];
	pver = (u_int8_t *)&ptlv[1];
	ptlv->type = htons(UCLA_ANSWER_TLV_VER);
	ptlv->len = htons(4);
	memcpy((void *)pver, (void *)cl_priv->app_ver, 3);

	if (strlen(developer_id)) {
		ptlv = tlv_n_next(ptlv);
		ptlv->type = htons(UCLA_DEVELOPER_ID);
		ptlv->len = htons((int)strlen(developer_id));
		memcpy(tlv_val(ptlv), developer_id, strlen(developer_id));
	}

	MD5Init(&ctx);
	MD5Update(&ctx, s->rand1, sizeof(s->rand1));
	MD5Update(&ctx, s->rand2, sizeof(s->rand2));
	MD5Update(&ctx, s->key, sizeof(s->key));
	MD5Final(acr->md5, &ctx);
	//memdumpone("ucla_auth_answer_into s->key", s->key, sizeof(s->key));
	
	ucla_request_add(s, pkt);	
}

static void ucla_auth_answer_out(ucla_session_t *s)
{
	// do none
}

static RS ucla_time_param_proc(ucla_session_t *s, uc_tlv_t *tlv)
{
	if (tlv->len < sizeof(uc_time_param_all_t)) {
		log_err(false, "%s ignore time param len=%u\n", s->doname, tlv->len);
		return RS_ERROR;
	}

	if (memcmp(tlv_val(tlv), &s->time_param_net, sizeof(uc_time_param_all_t)) == 0) {
		return RS_OK;
	}

	memcpy(&s->time_param_net, tlv_val(tlv), sizeof(uc_time_param_all_t));
	memcpy(&s->time_param, tlv_val(tlv), sizeof(uc_time_param_all_t));
	s->time_param.dev.die = ntohs(s->time_param.dev.die);
	s->time_param.lan.die = ntohs(s->time_param.lan.die);
	s->time_param.wan_front.die = ntohs(s->time_param.wan_front.die);
	s->time_param.wan_background.die = ntohs(s->time_param.wan_background.die);

	log_info("%s do_tlv_time_param update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
		s->doname, s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
		s->time_param_cur->keeplive, s->time_param_cur->die);

	return RS_OK;
}


static void ucla_do_tlv_parse_result(ucla_session_t *s, uc_tlv_t *ptlv, int remain_len)
{
	u_int32_t user_id = 0;
	u_int8_t last_ver[4];
	u_int32_t ver1;
	u_int32_t ver2;
	
	if (!s || !ptlv) {
		return;
	}

	if (remain_len >= (int)sizeof(uc_tlv_t)) {
		ptlv->type = ntohs(ptlv->type);
		ptlv->len = ntohs(ptlv->len);
	}
	
	while((remain_len > 0) && 
		(remain_len >= (int)sizeof(uc_tlv_t)) && 
		(remain_len >= (int)(sizeof(uc_tlv_t) + ptlv->len))) {
		remain_len -= (int)(sizeof(uc_tlv_t) + ptlv->len);

		log_debug("ucla_do_tlv_parse tlv remainlen=%u type=%u len=%u \n", 
			remain_len, ptlv->type, ptlv->len);		
		switch(ptlv->type) {
		case UCLA_RESULT_TLV_USERID:
			if (ptlv->len >= sizeof(u_int32_t)) {
				user_id = htonl(*(u_int32_t *)tlv_val(ptlv));
				ucla_userid_sync(s, user_id);
			}
			break;
		case UCLA_RESULT_TLV_LASTTIME:
			if (ptlv->len >= sizeof(last_ver)) {
				memcpy((void *)last_ver, (void *)tlv_val(ptlv), sizeof(last_ver));
				log_debug("last_ver ver=%u.%u.%u and server ver=%u.%u.%u\n", 
					plc->last_ver[0], plc->last_ver[1], plc->last_ver[2], 
					last_ver[0], last_ver[1], last_ver[2]);
				nd_la_debug(NULL, "last_ver ver=%u.%u.%u and server ver=%u.%u.%u\n", 
					plc->last_ver[0], plc->last_ver[1], plc->last_ver[2], 
					last_ver[0], last_ver[1], last_ver[2]);
				ver1 = VERTION(plc->last_ver[0], plc->last_ver[1], plc->last_ver[2]);
				ver2 = VERTION(last_ver[0], last_ver[1], last_ver[2]);
				if (ver2 > ver1) {
					//里面更新，避免另一个服务器也来推送
					memcpy((void *)plc->last_ver, (void *)last_ver, sizeof(plc->last_ver));
					log_debug("UCLA_RESULT_TLV_LASTTIME need req limit file\n");
					s->need_dev_ver_req = true;
					//在线后再请求
				}
			}			
			break;
		case UCLA_RESULT_LLV_TIME_PARAM:
			ucla_time_param_proc(s, ptlv);
			break;
		case UCLA_RESULT_TLV_SUPPORT_TABEL:
			plc->support_tabel = true;
			break;
		case UCLA_RESULT_TLV_SUPPORT_TRANS:
			plc->support_trans = true;
			break;
		case UCLA_RESULT_TLV_SUPPORT_SHORTCUT:
			plc->support_shortcut = true;
			break;
		case UCLA_SERVER_TLV_WIDGET_KEY:
			if (ptlv->len >= sizeof(s->widget_key)) {
				memcpy((void *)s->widget_key, (void *)tlv_val(ptlv), sizeof(s->widget_key));
				ucla_event_push(LA_WIDGET_KEY_DONAME_CHANGED);
			}
			break;
		}

		ptlv = tlv_next(ptlv);
		if (remain_len >= sizeof(uc_tlv_t)) {
			ptlv->type = ntohs(ptlv->type);
			ptlv->len = ntohs(ptlv->len);
		}
	}
}

static void ucla_do_tlv_parse_keeplive(ucla_session_t *s, uc_tlv_t *ptlv, int remain_len)
{
//	u_int32_t user_id = 0;
	
	if (!s || !ptlv) {
		return;
	}

	if (remain_len >= (int)sizeof(uc_tlv_t)) {
		ptlv->type = ntohs(ptlv->type);
		ptlv->len = ntohs(ptlv->len);
	}
	
	log_debug("ucla_do_tlv_parse tlv remainlen=%u type=%u len=%u \n", 
		remain_len, ptlv->type, ptlv->len);
	while((remain_len > 0) && 
		(remain_len >= (int)sizeof(uc_tlv_t)) && 
		(remain_len >= (int)(sizeof(uc_tlv_t) + ptlv->len))) {
		remain_len -= (int)(sizeof(uc_tlv_t) + ptlv->len);
		
		switch(ptlv->type) {
		case UCLA_KEEPLIVE_TLV_KICK:
			s->is_kicked = true;
			break;
		case UCLA_KEEPLIVE_TLV_ACOUNT_NOT_EXIST:
			plc->phone_acount_not_exist = true;
			plc->acount_not_exist = true;
			break;
		default:
			break;
		}

		ptlv = tlv_next(ptlv);
		if (remain_len >= sizeof(uc_tlv_t)) {
			ptlv->type = ntohs(ptlv->type);
			ptlv->len = ntohs(ptlv->len);
		}
	}
}


static void do_sa_result(ucla_session_t *s, uc_app_user_result_t *rd, ucph_t *hdr)
{
	MD5_CTX ctx;
	int remain_len = 0;
	uc_tlv_t *ptlv;

	rd->err = htons(rd->err);
	rd->select_enc = htons(rd->select_enc);	
	log_debug("%s do_sa_result rd->err=%u ver=%u\n", 
		s->doname, rd->err, rd->ver);

	nd_la_debug(NULL, "%s do_sa_result rd->err=%u ver=%u\n", 
		s->doname, rd->err, rd->ver);
	s->need_dev_ver_req = false;
	//tlv解析
	if (rd->err == 0) {
		remain_len = (int)hdr->param_len - (int)sizeof(*rd);
		ptlv = (uc_tlv_t *)&rd[1];
		ucla_do_tlv_parse_result(s, ptlv, remain_len);
	}
	
	if (rd->err == 0) {
		s->select_enc = rd->select_enc;
		MD5Init(&ctx);
		MD5Update(&ctx, s->rand1, sizeof(s->rand1));
		MD5Update(&ctx, s->rand2, sizeof(s->rand2));
		MD5Update(&ctx, s->key, sizeof(s->key));
		MD5Update(&ctx, s->key, sizeof(s->key));
		MD5Final(s->aes_key, &ctx);
		ucla_set_status(s, UCLAS_ESTABLISH);
	} else {
		nd_la_memdump("s->username do_sa_result err", s->username, sizeof(s->username));
		nd_la_memdump("s->key do_sa_result err", s->key, sizeof(s->key));
		nd_la_memdump("s->rand1 do_sa_result err", s->rand1, sizeof(s->rand1));
		nd_la_memdump("s->rand2 do_sa_result err", s->rand2, sizeof(s->rand2));
		
		if (rd->err == ERR_PASSWD_INVALID) {
			plc->phone_passwd_err = true;
			if (ucla_is_phone_session(s)) {
				event_push_err(plc->callback, LA_PHONE_PASSWD_ERR, 0, plc->callback_handle, (int)rd->err);
			}
		}
		if (rd->err == ERR_SOFT_VER_LOW) {
			event_push_err(plc->callback, LA_SERVER_LINK_FAILED, 0, plc->callback_handle, (int)rd->err);
		}
		log_debug("do_sa_result rd->err=%u\n", rd->err);
		ucla_set_status(s, UCLAS_ERROR);
		ucla_clean_disp_ext();
	}
}

static void ucla_auth_answer_proc(ucla_session_t *s)
{
	ucph_t *hdr;
	uc_app_user_result_t *rd;
		
	hdr = (ucph_t *)s->rcv_buf->data;

	rd = get_ucp_payload(s->rcv_buf, uc_app_user_result_t);
	log_debug("enter ucla_auth_answer_proc rd->action=%u\n", rd->action);
	switch (rd->action) {
	case UCAU_RESULT:
		do_sa_result(s, rd, hdr);
		break;
	default:
		break;
	}
}

/**********************************estab**************************************************/
static int ucla_estab_die(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;

	log_debug("%s sa_die die\n", s->doname);
	nd_la_debug(NULL ,"%s sa_die die\n", s->doname);

	ucla_set_status(s, UCLAS_AUTH_REQ);

	return 0;
}

#define 	TRANS_INFO_QUERY_TIME	(10*60)
static void do_trans_query_check()
{
	ucla_session_t *s;
	static u_int32_t last_time = 0;
	u_int32_t now = (u_int32_t)time(NULL);

	s = ucla_get_any_enstablis_session();
	if (!s) {
		return;
	}

	if (last_time == 0) {
		last_time = now;
		return;
	}
	if ((last_time + TRANS_INFO_QUERY_TIME) > now) {
		return;
	}
	last_time = now;

	ucla_query_trans_info(s);
}

static int ucla_keeplive_timer(cl_thread_t *t)
{
	ucla_session_t *s;
	pkt_t *pkt;
	uc_keeplive_request_t *kr;

	s = (ucla_session_t *)CL_THREAD_ARG(t);
	s->t_keeplive = NULL;

	// 发送队列目前还有报文等待发送，没必要再发保活报文了
	if ( ! stlc_list_empty(&s->send_list) )
		return 0;

	pkt = ucla_pkt_new(CMD_UDP_KEEPLIVE, sizeof(uc_keeplive_request_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	kr = get_uascp_payload(pkt, uc_keeplive_request_t);
	kr->action = UCAU_REQUEST;
	
	ucla_request_add(s, pkt);

	return 0;
}

void ucla_create_def_home(ucla_session_t *s)
{
	char buf[64];
	pkt_t *pkt;
	la_home_t *phome;
	uc_home_conf_create_t *phomecf;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_home_conf_create_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		log_debug("errrr !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		return;
	}
	phome = ucla_home_new();
	if (!phome) {
		log_debug("errrr !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		return;
	}
	log_debug("ucla_create_def_home s->my_request_id=%u !!!!!!!!!!!!!!!!!!!!\n", s->my_request_id);
	phome->conf.home_id = 0xffffffff;
	ucla_home_add(s, phome);
	phomecf = get_uascp_payload(pkt, uc_home_conf_create_t);
	phomecf->action = UCAU_CREATE;
	//name 全0
	memset(buf, 0, sizeof(buf));
	s->is_def_home = true;
	plc->has_any_home = true;
	memcpy((void *)phomecf->home_name, (void *)buf, sizeof(phomecf->home_name));
	memcpy((void *)&phome->conf.home_name, (void *)buf, sizeof(phomecf->home_name));
	memcpy((void *)s->home_name, (void *)buf, sizeof(phomecf->home_name));

	ucla_request_add(s, pkt);
}

void ucla_each_home_query(ucla_session_t *s)
{
//	uc_home_conf_query_t *pq = NULL;
	la_home_t *phome = NULL, *homen = NULL;
	la_user_t *puser = NULL;

	UCLA_ENTER();
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_valid(phome)) {
			continue;
		}
		phome->last_total_len_offset = 0;
		do_ucla_home_query(s, phome->conf.home_id);
		//_ucla_link_conf_rule_need_query(s, phome->conf.home_id, 0);
		//plc->is_rule_all_query = true;
	}
}

void ucla_each_share_query(ucla_session_t *s)
{
	la_home_t *phome = NULL, *homen = NULL;
	la_user_t *puser = NULL;

	UCLA_ENTER();
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_valid(phome)) {
			continue;
		}
		do_ucla_share_query(s, phome->conf.home_id);
	}
}

void ucla_each_label_query(ucla_session_t *s)
{
	la_home_t *phome = NULL, *homen = NULL;
	la_user_t *puser = NULL;

	UCLA_ENTER();
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_valid(phome)) {
			continue;
		}

		//查询一下标签信息
		do_ucla_query_label_info(s, phome->conf.home_id, 0);
	}
}

void ucla_each_home_shortcut_query(ucla_session_t *s)
{
	la_home_t *phome = NULL, *homen = NULL;
	la_user_t *puser = NULL;

	UCLA_ENTER();
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_valid(phome)) {
			continue;
		}

		//查询一下标签信息
		do_ucla_query_shortcut_info(s, phome->conf.home_id, 0);
	}
}

void ucla_each_share_query_sync(ucla_session_t *s)
{
	la_home_t *phome = NULL, *homen = NULL;
	la_user_t *puser = NULL;

	UCLA_ENTER();
	s->map_cal = 0;
	s->need_map_sync = false;
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		if (!ucla_home_valid(phome)) {
			continue;
		}
		do_ucla_share_query(s, phome->conf.home_id);
		s->map_cal += phome->conf.home_id;
		s->need_map_sync = true;
	}
}

static bool ucla_home_valid(la_home_t *phome)
{
	return (phome->conf.home_id != 0xffffffff);
}

static void ucla_all_home_query(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_home_query_t *pq;

	log_debug("%s enter %s\n", s->doname, __FUNCTION__);
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(*pq), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	pq = get_uascp_payload(pkt, uc_app_user_home_query_t);
	pq->action = UCAU_QUERY;
	pq->user_id = htonl(s->user_id);
	
	ucla_request_add(s, pkt);
}

static void ucla_all_misc_query(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_misc_query_t *pq;

	log_debug("%s enter %s\n", s->doname, __FUNCTION__);
	
	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(*pq), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	pq = get_uascp_payload(pkt, uc_app_user_misc_query_t);
	pq->action = UCAU_MISC_QUERY;
	
	ucla_request_add(s, pkt);
}

static void ucla_phone_login_user_save(ucla_session_t *s)
{
	la_user_t *puser;
	
	puser = ucla_find_user_by_id(s->user_id);
	if (puser) {
		memcpy((void *)puser->conf.user_name, (void *)s->username, APP_USER_UUID_NAME_LEN);
		memset((void *)puser->conf.uuid, 0, APP_USER_UUID_NAME_LEN);
		memcpy((void *)puser->conf.passwd, (void *)s->key, APP_USER_UUID_NAME_LEN);
		memset((void *)s->uuid, 0, APP_USER_UUID_NAME_LEN);
		
		memcpy(plc->p_user_name, s->username, sizeof(plc->p_user_name));
		memcpy(plc->p_passwd, s->key, sizeof(plc->p_passwd));
		memcpy(plc->p_passwd_cmp, s->key, sizeof(plc->p_passwd_cmp));
	}
	
	ucla_server_user_query_clean();
	//里面保存
	la_phone_save();
	//这里需要保存下用户名称
	s->need_save_username = true;
	_ucla_user_conf_save(s);
}

static void ucla_phone_home_move(ucla_session_t *s)
{
	pkt_t *pkt;
	uc_app_user_misc_set_t *pms;
	uc_app_user_move_home_t *pm;
	la_move_home_t *mh = &plc->move_home;
	uc_tlv_t *ptlv;
	int len = 0;
	int len1 = 0;
	int i;

	UCLA_ENTER();
	if (mh->num == 0) {
		return;
	}
	len1 = sizeof(*pm) + sizeof(u_int32_t)*mh->num;
	len = sizeof(*pms) + sizeof(*ptlv) + len1;
	pkt = ucla_pkt_new(CMD_APP_USER, len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	pms = get_uascp_payload(pkt, uc_app_user_misc_set_t);
	pms->action = UCAU_MISC_SET;
	ptlv = (uc_tlv_t *)&pms[1];
	pm = (uc_app_user_move_home_t*)&ptlv[1];
	//TLV
	ptlv->type = htons(UCLA_MISCSET_TLV_HOMEMOVE);
	ptlv->len = htons(len1);
	//head
	pm->userid_src = htonl(mh->userid);
	pm->userid_dst = htonl(s->user_id);
	pm->num = mh->num;
	for(i = 0; i < pm->num; i++) {
		pm->homeid[i] = htonl(mh->homeid[i]);
	}
	ucla_request_add(s, pkt);
	UCLA_EXIT();
}

static bool ucla_need_save_dns(ucla_session_t *s)
{
	int i;
	
	if (s->ipc_count == 0) {
		return true;
	}
	
	UCLA_ENTER();
	for(i = 0; i < s->ipc_count; i++) {
		//log_debug("%s ipcache=%u.%u.%u.%u ipnew=%u.%u.%u.%u\n", __FUNCTION__,
		//	IP_SHOW(htonl(s->ipc_sock[i].addr.sockaddr.sin_addr.s_addr)), 
		//	IP_SHOW(htonl(s->peer_addr.sockaddr.sin_addr.s_addr)));
		if (s->is_ipv6) {
			if (memcmp((void *)&s->ipc_sock[i].addr.sockaddr6.sin6_addr, 
				(void *)&s->peer_addr.sockaddr6.sin6_addr, 
				sizeof(s->peer_addr.sockaddr6.sin6_addr)) == 0) {
				log_debug("ipv6 %s has same ip[%d]\n", __FUNCTION__, i);
				return false;
			}
		} else {
			if (memcmp((void *)&s->ipc_sock[i].addr.sockaddr.sin_addr, 
				(void *)&s->peer_addr.sockaddr.sin_addr, 
				sizeof(s->peer_addr.sockaddr.sin_addr)) == 0) {
				log_debug("ipv4 %s has same ip[%d]=%u.%u.%u.%u in dns cache\n", __FUNCTION__, i, 
					IP_SHOW(htonl(s->ipc_sock[i].addr.sockaddr.sin_addr.s_addr)));
				return false;
			}
		}
	}

	return true;
}

static void ucla_estab_into(ucla_session_t *s)
{
	s->my_request_id = 0;
	s->peer_request_id = 0;
	s->home_event_last_id = 0xff;
	s->share_event_last_id = 0xff;
	s->linkage_event_last_id = 0xff;
	s->linkage_rulelistpush_last_id = 0xff;
	s->linkage_dev_ver_limit_last_id = 0xff;
	s->linkage_msg_push_last_id = 0xff;
	s->adddev_sn = 0;
	s->reset_num++;

	//将其他session全部err去
	ucla_other_session_goto_err(s);

	//计算连接服务器花费时间
	s->time_start = s->time_diff;
	s->time_diff = (u_int32_t)get_msec(NULL) - s->time_diff;
	
	ucla_reset_send(s);
	ucla_off_all_timer(s);
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, ucla_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));

	// 先快速发一个keeplive，获取一些参数
	CL_THREAD_OFF(s->t_keeplive);
	CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, ucla_keeplive_timer, (void *)s, 0);

	if (s->need_login_query_user) {
		s->need_login_query_user = false;
		plc->has_phone = true;
		//登陆成功后可能要迁移下以前弱用户的圈子
		ucla_phone_home_move(s);
		//手机账号第一次登陆这里报一下手机登陆成功
		ucla_phone_ok_report();
		//处理一下账号配置保存
		ucla_phone_login_user_save(s);
	} else if (ucla_is_phone_session(s)) {
		//手机账号重启后登陆时也给个登陆成功事件
		ucla_phone_ok_report();
	} 

	plc->phone_logining = false;
	s->timeout_count = 0;
	
	ucla_all_misc_query(s);
	//去查询是否字节的圈子变化了
	ucla_all_home_query(s);
	
	la_comm_timer_proc_reset();
	//连接成功后可以保存下dns缓存
	if (ucla_need_save_dns(s)) {
		s->need_save_dns = true;
	}
	//查询一下中转设备信息
	ucla_query_trans_info(s);
//test_rule_add(s);
}

static void ucla_estab_out(ucla_session_t *s)
{
	// do none
	s->time_diff = 0;
	ucla_event_push(LA_USER_CHANGED);
}

static RS ucla_check_request_id(ucla_session_t *s, ucph_t *request)
{
	if (request->request_id == (s->peer_request_id &0xFF)) {
		return RS_OK;
	} else if (request->request_id == (u_int8_t)(s->peer_request_id + 1)) {
		// good, update to next
		log_debug("%s update rquest id to %u\n", s->doname, request->request_id);
		s->peer_request_id++;
		return RS_OK;
	}

	log_debug( "%s ignore request pkt: reply id=%u, but local is %u.\n",
		s->doname, request->request_id, s->peer_request_id);

	return RS_ERROR;
}


static void ucla_do_keeplive(ucla_session_t *s, ucph_t *hdr)
{
	uc_keeplive_reply_t *kr;
	uc_tlv_t *tlv;
	int remain;

	if (hdr->param_len < sizeof(uc_keeplive_reply_t)) {
		log_debug( "bad keeplive packet: param_len=%u\n", hdr->param_len);
		return;
	}
	
	kr = get_net_ucp_payload(hdr, uc_keeplive_reply_t);
	if (kr->action == UCAU_RESET) {
		log_debug("%s reset by peer\n", s->doname);
		ucla_set_status(s, UCLAS_AUTH_REQ);
		return;
	}

	if (kr->action != UCAU_REPLY) {
		log_debug("%s ignore keeplive action=%u\n", kr->action);
		return;
	}

	tlv = &kr->tlv[0];

	remain = hdr->param_len - sizeof(uc_keeplive_reply_t);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		switch (tlv->type) {
		case UCT_TIME_PARAM:
			break;
		}

		remain -= (sizeof(uc_tlv_t) + tlv->len);
		tlv = tlv_next(tlv);
		if (remain >= sizeof(uc_tlv_t)) {
			tlv->type = ntohs(tlv->type);
			tlv->len = ntohs(tlv->len);
		}
	}
}

static void ucla_do_linkage_user(ucla_session_t *s, ucph_t *hdr)
{
	u_int8_t *ptr;
	pkt_t *pkt;

	if (hdr->param_len < sizeof(ucp_app_data_hdr_t)) {
		log_debug( "bad ucla_do_linkage_user packet: param_len=%u\n", hdr->param_len);
		return;
	}

	ptr = get_net_ucp_payload(hdr, u_int8_t);

	app_proc_pkt_from_server(ptr, hdr->param_len);

	//先回答
	pkt = ucla_pkt_new(CMD_APP_LINKAGE_USER, 0, 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);
}

static void ucla_big_buf_free(ucla_session_t *s)
{
	la_fragment_stat_t *fs = &s->fs;

	fs->item_num = 0;
	SAFE_FREE(fs->pitems);
	SAFE_FREE(fs->item_buf);
}

static bool ucla_new_big_buf(ucla_session_t *s, u_int8_t item_num)
{
	int len = 0;
	la_fragment_stat_t *fs = &s->fs;
//	u_int8_t *ptmp = NULL;

	item_num = max(item_num, 2);

	fs->item_num = item_num;
	len = fs->item_num * sizeof(la_fragment_item_t);

	fs->pitems = cl_realloc(fs->pitems, len);
	if (!fs->pitems) {
		goto err;
	}
	fs->item_buf = cl_realloc(fs->item_buf, len);
	if (!fs->item_buf) {
		goto err;
	}

	log_debug("enter %s num=%u len=%u\n", __FUNCTION__, item_num, len);
	
	return true;
	
err:
	fs->item_num = 0;;
	SAFE_FREE(fs->pitems);
	SAFE_FREE(fs->item_buf);
	
	return false;
}

static RS ucla_proc_fragment_ext(ucla_session_t *s, ucph_t *hdr)
{
	la_fragment_stat_t *fs = &s->fs;
	u_int8_t is_last_fragment = 0, *ptr;
	la_fragment_item_t *ci = NULL;
	int i, total_len = sizeof(*hdr) + hdr->param_len;
	ucph_t *hd, *newhd = NULL;


	if ((hdr->flags & UDP_LA_FLAG_IS_FRAG) == 0) {
		return RS_ERROR;
	}

	is_last_fragment = !!(hdr->flags & UDP_LA_FLAG_IS_LAST);

	log_debug("%s fragment pkt: ext rid %u is_last %u  current write idx=%u err_stat=%u num=%u\n", 
		s->doname, hdr->request_id, is_last_fragment, fs->idx, fs->err, fs->item_num);

	// 先回复
	ucla_request_reply(s, hdr);

	// 当发生错误，只有等收到一个结束报文后，才恢复
	if (fs->err && !is_last_fragment) {
		log_err(false, "fs is err, but not last fragment\n");
		return RS_OK;
	}
	fs->err = false;

	if (fs->idx >= fs->item_num) {
		ucla_new_big_buf(s, fs->item_num*2);
	}
	if (fs->item_num == 0) {
		log_err(false, "err item_num = 0\n");
		return RS_OK;
	}
	ci = &fs->pitems[fs->idx];
	// 检查request_id
	if (fs->idx == 0) {
		fs->cid = hdr->request_id;
		log_debug("first receive fragment, update cid to %u\n", fs->cid);

		// 分片报文至少得是2片
		if (is_last_fragment) {
			log_debug("is last fragment, but not receive fragment yet, maybe is resend\n");
			goto err_out;
		}
	} else {
		if (hdr->request_id != fs->cid + 1) {
			log_debug("ignore same pkt need %u, but now %u\n", fs->cid + 1, hdr->request_id);
			return RS_OK;
		}
		fs->cid++;
		log_debug("expect next request id %u\n", fs->cid);
	}

	// 组报文	
	if (total_len > sizeof(ci->data)) {
		log_err(false, "total len %u too big\n", total_len);
		goto err_out;
	}
	
	memcpy(ci->data, hdr, total_len);

	if (++fs->idx >= UCLA_FRAGMENT_MAX_DYNAMIC_ITEMS) {
		log_debug("%s have no space receive next fragment fs->idx=%u\n", s->doname, fs->idx);
	}

	if (!is_last_fragment) {
		log_debug("fragment need more\n");
		return RS_OK;
	}

	// 开始合并
	newhd = (ucph_t *)fs->item_buf;
	memcpy(newhd, hdr, sizeof(*newhd));
	ptr = (u_int8_t*)&newhd[1];
	for (i = total_len = 0; i < fs->idx; i++) {
		hd = (ucph_t*)&(fs->pitems[i].data);
		total_len += hd->param_len;

		memcpy(ptr, &hd[1], hd->param_len);
		ptr += hd->param_len;

		if (newhd->command == 0) {
			newhd->command = hd->command;
			newhd->param_len = hd->param_len;
		} else {
			if (newhd->command != hd->command) {
				log_debug("BUG ON, local cmd %u but now cmd %u\n", newhd->command, hd->command);
				goto err_out;
			}
		}
	}

	newhd->param_len = total_len;
	log_debug("command %u param_len %u\n", newhd->command, newhd->param_len);

	// 假装是回复报文，其实是push过来的
	sa_do_ucla_reply_pkt(s, newhd);
	
	fs->idx = 0;

	return RS_OK;

err_out:
	if (!is_last_fragment) {
		fs->err = true;
	}

	return RS_OK;
}

#if 0
static RS ucla_proc_fragment(ucla_session_t *s, ucph_t *hdr)
{
	la_fragment_stat_t *fs = &s->fs;
	u_int8_t is_last_fragment = 0, *ptr;
	la_fragment_item_t *ci = &fs->items[fs->idx];
	int i, total_len = sizeof(*hdr) + hdr->param_len, offset = 0;
	ucph_t *hd, *newhd = (ucph_t *)s->rcv_hdr;
	
	if ((hdr->flags & UDP_LA_FLAG_IS_FRAG) == 0) {
		return RS_ERROR;
	}

	is_last_fragment = !!(hdr->flags & UDP_LA_FLAG_IS_LAST);

	log_debug("%s fragment pkt: rid %u is_last %u  current write idx %u err_stat %u\n", s->doname, hdr->request_id, is_last_fragment, fs->idx, fs->err);

	// 先回复
	ucla_request_reply(s, hdr);

	// 当发生错误，只有等收到一个结束报文后，才恢复
	if (fs->err && !is_last_fragment) {
		log_err(false, "fs is err, but not last fragment\n");
		return RS_OK;
	}
	fs->err = false;

	// 检查request_id
	if (fs->idx == 0) {
		fs->cid = hdr->request_id;
		log_debug("first receive fragment, update cid to %u\n", fs->cid);

		// 分片报文至少得是2片
		if (is_last_fragment) {
			log_debug("is last fragment, but not receive fragment yet, maybe is resend\n");
			goto err_out;
		}
	} else {
		if (hdr->request_id != fs->cid + 1) {
			log_debug("ignore same pkt need %u, but now %u\n", fs->cid + 1, hdr->request_id);
			return RS_OK;
		}
		fs->cid++;
		log_debug("expect next request id %u\n", fs->cid);
	}

	// 组报文	
	if (total_len > sizeof(ci->data)) {
		log_err(false, "total len %u too big\n", total_len);
		goto err_out;
	}
	
	memcpy(ci->data, hdr, total_len);

	if (++fs->idx >= UCLA_FRAGMENT_MAX_ITEMS) {
		log_debug("%s have no space receive next fragment\n", s->doname);
	}


	if (!is_last_fragment) {
		log_debug("fragment need more\n");
		return RS_OK;
	}

	// 开始合并
	log_debug("start combin\n");
	
	ptr = (u_int8_t*)&newhd[1];
	for (i = total_len = 0; i < fs->idx; i++) {
		hd = (ucph_t*)&(fs->items[i].data);
		total_len += hd->param_len;

		memcpy(ptr, &hd[1], hd->param_len);
		ptr += hd->param_len;

		if (newhd->command == 0) {
			newhd->command = hd->command;
			newhd->param_len = hd->param_len;
		} else {
			if (newhd->command != hd->command) {
				log_debug("BUG ON, local cmd %u but now cmd %u\n", newhd->command, hd->command);
				goto err_out;
			}
		}
	}

	newhd->param_len = total_len;
	log_debug("command %u param_len %u\n", newhd->command, newhd->param_len);

	// 假装是回复报文，其实是push过来的
	sa_do_ucla_reply_pkt(s, hdr);
	
	fs->idx = 0;

	return RS_OK;

err_out:
	if (!is_last_fragment) {
		fs->err = true;
	}

	return RS_OK;
}
#endif

static void ucla_estab_do_request(ucla_session_t *s, ucph_t *hdr)
{
	if (ucla_check_request_id(s, hdr) != RS_OK)
		return;
#if 1
	// 处理分片报文
	if (ucla_proc_fragment_ext(s, hdr) == RS_OK) {
		return;
	}
#else
	// 处理分片报文
	if (ucla_proc_fragment(s, hdr) == RS_OK) {
		return;
	}
#endif

    //预处理
	switch (hdr->command) {
	case CMD_UDP_KEEPLIVE:
		ucla_do_keeplive(s, hdr);
        return;
		break;
	case CMD_APP_LINKAGE_USER:
		ucla_do_linkage_user(s, hdr);
		return;
    default:
		break;
	}
    sa_do_ucla_request_pkt(s, hdr);
}

static RS ucla_check_reply_id(ucla_session_t *s, ucph_t *reply)
{
	pkt_t *pkt;
	ucph_t *request;
	
	if (stlc_list_empty(&s->send_list)) {
		log_debug( "%s ignore reply pkt: no request in send list.\n", s->doname);
		return RS_ERROR;
	}

	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	request = (ucph_t *)&pkt->data[0];
	if (request->request_id != reply->request_id) {
		log_debug( "%s ignore reply pkt: my request id=%u, but %u in pkt.\n",
			s->doname, request->request_id, reply->request_id);
		return RS_ERROR;
	}

	return RS_OK;
}

static bool ucla_is_reset_pkt(ucla_session_t *s, ucph_t *hdr)
{
	uc_keeplive_reply_t *kr;
    
	if (hdr->command != CMD_UDP_KEEPLIVE)
		return false;
	
	kr = get_net_ucp_payload(hdr, uc_keeplive_reply_t);
	// TODO:处理一些参数。。。,如被踢。
	s->is_kicked = false;
	if (kr->action == UCAU_RESET) {
		ucla_do_tlv_parse_keeplive(s, kr->tlv, (int)(hdr->param_len - sizeof(*kr)));
	}
    
	return (kr->action == UCAU_RESET);
}

static void ucla_estab_proc(ucla_session_t *s)
{
	ucph_t *hdr;

	hdr = (ucph_t *)s->rcv_buf->data;

	// check if it's keeplive.reset
	if (ucla_is_reset_pkt(s, hdr)) {
		log_debug("%s reset by peer\n", s->doname);
		if (s->is_kicked) {
			//log_debug("%s is kicked by server\n", s->username);
			event_push_err(plc->callback, LA_KICKED_BY_OTHER_USER, 0, plc->callback_handle, 0);
			//all goto sleep,被踢了，都跑去睡眠
			ucla_all_goto_sleep();
		} else {
			ucla_set_status(s, UCLAS_AUTH_REQ);
		}
		return;
	}


	// process request packet
	if (hdr->request) {
		ucla_estab_do_request(s, hdr);
		return;
	}

	/*
		bellow process reply packet
	*/
	
	if (ucla_check_reply_id(s, hdr) != RS_OK) {
		return;
	}

	//  返回标志有这个，说明是APP去请求一个数据，服务器发现返回报文
	//  太大，需要分片传回来。先返回一个。后面通过主动PUSH
	if (hdr->flags & UDP_LA_FLAG_IS_NEED) {
		log_debug("%s rid 0x%x need fragment, wait push fragment...\n", s->doname, hdr->request_id);
		goto done;
	}

	// 返回的报文有分片标志，说明这个是APP正在分片上传信息
	// 服务会在最后一个报文里面把返回参数填上
	if (hdr->flags & UDP_LA_FLAG_IS_FRAG) {
		log_debug("%s rid 0x%x is fragment reply\n", s->doname, hdr->request_id);
		if ((hdr->flags & UDP_LA_FLAG_IS_LAST) == 0) {
			goto done;
		}
		log_debug("is last\n");
	}

	switch (hdr->command) {
	case CMD_UDP_KEEPLIVE:
		ucla_do_keeplive(s, hdr);
		break;
	case CMD_APP_LINKAGE_USER:
		app_proc_pkt_from_server(get_net_ucp_payload(hdr, u_int8_t), hdr->param_len);
		break;
	case CMD_HOME_CONFIG:
    case CMD_HOME_SHARE:
	case CMD_LINKAGE_CONFIG:
	case CMD_APP_USER:
	case CMD_HOME_LABEL:
	case CMD_HOME_DICTIONARY:
	case CMD_QUERY_HISTORY:
	case CMD_WIDGET_KEY:
        sa_do_ucla_reply_pkt(s, hdr);
		break;
	case CMD_UDP_CTRL:
		ucla_sa_do_udp_reply_pkt(s, hdr);
		break;
	default:
		log_debug("%s at %s ignore reply pkt cmd=%u\n", s->doname, ucla_proc[s->status].name, hdr->command);
        return;
		break;
	}

done:
	// 删除掉已经应答的报文
	ucla_request_del(s);

	if (ucla_session_is_ok(s)) {
		CL_THREAD_OFF(s->t_die);
		CL_THREAD_TIMER_ON(MASTER, s->t_die, ucla_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
	}
}

/**********************************error**************************************************/
#define ERR_SERVER_DELAY_CHECK_TIMEOUT 60

static int ucla_err_server_check_delay(cl_thread_t *t)
{
	ucla_session_t *s = (ucla_session_t *)CL_THREAD_ARG(t);
	
	s->t_server_check = NULL;
	CL_THREAD_TIMER_ON(MASTER, s->t_server_check, ucla_err_server_check_delay, (void *)s, TIME_N_SECOND(ERR_SERVER_DELAY_CHECK_TIMEOUT));

	//log_debug("%s %s\n", s->doname, __FUNCTION__);
	//nd_la_debug(NULL ,"%s %s \n", s->doname, __FUNCTION__);

#if 1
	//超时后判断下，有没有连接服务器成功的session，如果不成功，就重新来
	if (!ucla_get_any_enstablis_session()) {
		log_debug("linkage never linked server need restart\n");
		ucla_set_status(s, UCLAS_IDLE);	
	}
#else
	if (!linkage_has_linked_server) {
		log_debug("linkage never linked server need restart\n");
		ucla_set_status(s, UCLAS_IDLE);	
	}
#endif

	return 0;
}

static void ucla_error_into(ucla_session_t *s)
{
	ucla_reset_send(s);
	ucla_off_all_timer(s);

	s->timeout_count = 10;	
	//close sock
	if(s->disp_sock != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock);
	}
	if(s->disp_sock6 != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock6);
	}

	CL_THREAD_OFF(s->t_recv);
	if(s->sock != INVALID_SOCKET){
		CLOSE_SOCK(s->sock);
	}

	//开启服务器检查定时器，给次机会判断服务器是否挂了
	CL_THREAD_TIMER_ON(MASTER, s->t_server_check, ucla_err_server_check_delay, (void *)s, TIME_N_SECOND(ERR_SERVER_DELAY_CHECK_TIMEOUT));
}

static void ucla_error_out(ucla_session_t *s)
{
	// do none
}

static void ucla_error_proc(ucla_session_t *s)
{
	ucph_t *hdr;

	hdr = (ucph_t *)s->rcv_buf->data;

	// do nothing
	log_debug("%s at %s ignore reply pkt cmd=%u\n", s->doname, ucla_proc[s->status].name, hdr->command);
}

static void ucla_sleep_into(ucla_session_t *s)
{
	ucla_reset_send(s);
	ucla_off_all_timer(s);

	s->timeout_count = 0;	
	//close sock
	if(s->disp_sock != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock);
	}
	if(s->disp_sock6 != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock6);
	}
	CL_THREAD_OFF(s->t_recv);
	if(s->sock != INVALID_SOCKET){
		CLOSE_SOCK(s->sock);
	}
}

ucc_proc_t ucla_proc[UCLAS_MAX] = {
	{"IDLE", ucla_idle_into, ucla_idle_out, ucla_idle_proc},
	{"disp probe fast", ucla_disp_probe_into, ucla_idle_out, ucla_disp_proc},
	{"disp", ucla_disp_into, ucla_disp_out, ucla_disp_proc},
	{"server auth", ucla_sa_into, ucla_sa_out, ucla_sa_proc},
	{"user_reg", ucla_ur_into, ucla_ur_out, ucla_ur_proc},
	{"AUTH_REQ", ucla_auth_req_into, ucla_auth_req_out, ucla_auth_req_proc},
	{"AUTH_ANSWER", ucla_auth_answer_into, ucla_auth_answer_out, ucla_auth_answer_proc},
	{"ESTAB", ucla_estab_into, ucla_estab_out, ucla_estab_proc},
	{"ERROR", ucla_error_into, ucla_error_out, ucla_error_proc},
	{"SLEEP", ucla_sleep_into, ucla_error_out, ucla_error_proc},
};

static void ucla_status_proc(ucla_session_t *s, int status)
{
	switch(status) {
	case UCLAS_DIS_PROBE:
	case UCLAS_DISPATCH:
	case UCLAS_SERVER_AUTH:
	case UCLAS_USER_REGISTER:
	case UCLAS_AUTH_REQ:
	case UCLAS_AUTH_ANSWER:
		ucla_enable_disp_ext(s->index, true);
		break;
	case UCLAS_IDLE:
		//超时情况应该直接清空，每个session的超时都是一样的
		ucla_clean_disp_ext();
		break;
	default:
		ucla_enable_disp_ext(s->index, false);
		break;
	}
}

void ucla_set_status(ucla_session_t *s, int status)
{
	s->last_status = s->status;
    
	if (status >= UCLAS_MAX) {
		log_debug("ucla_set_status unknow new status = %d\n", status);
		return;
	}

	s->status = status;

	ucla_status_proc(s, status);
	
	log_debug("%s modify status from %s to %s\n",
		s->doname, ucla_proc[s->last_status].name, ucla_proc[status].name);

	nd_la_debug(NULL, "%s modify status from %s to %s\n",
		s->doname, ucla_proc[s->last_status].name, ucla_proc[status].name);
	
	ucla_proc[s->last_status].on_out(s);
	ucla_proc[status].on_into(s);
}

ucla_session_t *ucla_get_any_enstablis_session()
{
	ucla_session_t *s, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		if (!ucla_session_is_ok(s)) {
			continue;
		}
		return s;
	}

	return NULL;
}

//表示获取一个可用的session，即已经或者到最快服务器了
ucla_session_t *ucla_get_any_valid_session()
{
	ucla_session_t *s, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		if ((s->status >= UCLAS_DISPATCH) &&
			(s->status != UCLAS_ERROR)) {
			return s;
		}
	}

	return NULL;
}

void ucla_session_reset(ucla_session_t *s)
{
	if (!s) {
		return;
	}

	log_debug("%s need reset session\n", s->doname);
	//fast
	s->idle_time = 0;
	ucla_set_status(s, UCLAS_IDLE);
}

void ucla_server_user_query_clean()
{
	ucla_session_t *s, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		s->need_login_query_user = false;
	}	
}

RS ucla_udp_socket(ucla_session_t *s, bool ipv6)
{
//    int port = s->my_port;
    
	if((s->sock != INVALID_SOCKET) && 
		s->is_ipv6_sock == ipv6){
		return RS_OK;
	}

    CLOSE_SOCK(s->sock);
	s->is_ipv6_sock = ipv6;
    //使用随机端口,否则，因序列号相同，端口相同，会占用路由器以前的通道
    if (!ipv6) {
	    s->my_port = 0;
		s->sock = create_udp_server(0, 0);
		if (s->sock == INVALID_SOCKET) {
			if ((s->sock = create_udp_server(0, 0)) == INVALID_SOCKET) {
				return RS_ERROR;
			}
			s->my_port = 0;
		}
    } else {
	    s->my_port = 0;
		s->sock = create_udp_server6();
		if (s->sock == INVALID_SOCKET) {
			if ((s->sock = create_udp_server6()) == INVALID_SOCKET) {
				log_debug("ucla_udp_socket failed\n");
				return RS_ERROR;
			}
			s->my_port = 0;
		}
	}
	
	return RS_OK;
}

static int ucla_read(cl_thread_t *t)
{
	ucla_session_t *s;
	struct sockaddr_in addr;
	int n, pad_len;
	int addr_len;
	pkt_t *pkt;
	ucph_t *hdr;

	s = (ucla_session_t *)CL_THREAD_ARG(t);
	s->t_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_recv, ucla_read, (void *)s, s->sock);

	addr_len = sizeof(struct sockaddr_in);
	pkt = s->rcv_buf;

	n = pkt->total = (int)recvfrom(s->sock, (char *)pkt->data, LA_MAX_UDP_PKT, 0, 
			(struct sockaddr *)&addr, (socklen_t *)&addr_len);
	if (n <= (int)ucph_hdr_size) {
		log_err(true, "%s read udp failed\n", s->doname);
		return 0;
	}

	memset((void *)&s->last_ipv6_addr, 0, sizeof(s->last_ipv6_addr));
    s->recv_pkts++;
	hdr = (ucph_t *)pkt->data;
    s->rcv_hdr = hdr;

	//log_mem_dump("ucla_readdhr1", hdr, sizeof(*hdr));
	// 1. 解密、检查长度合法性
	if (hdr->encrypt) {
		if (((n - ucph_hdr_plain_size)%AES128_EKY_LEN) != 0) {
			log_debug("%s Drop bad packet: n=%d, but encrypt=1\n", s->doname, n);
			return 0;
		}
		dec_block((u_int8_t *)BOFP(hdr, ucph_hdr_plain_size), (n - ucph_hdr_plain_size), s->aes_key);
		pad_len = ((u_int8_t *)hdr)[n - 1];
		if (pad_len > AES128_EKY_LEN || pad_len <= 0) {
			log_debug("%s Drop bad packet: encrypt pad=%d\n", s->doname, pad_len);
			return 0;
		}
		pkt->total -= pad_len;
	}

	uc_hdr_order(hdr);
	
	if (ucph_hdr_len(hdr) < ucph_hdr_size 
		|| pkt->total != hdr->param_len + ucph_hdr_len(hdr))
	{
		log_debug("%s Drop bad packet: total=%d, ucph_hdr_len=%d, param_len=%d\n", 
			s->doname, pkt->total, ucph_hdr_len(hdr), hdr->param_len);
		return 0;
	}

	// 2. 检查会话ID
	if (s->client_sid != 0 && hdr->client_sid != s->client_sid) {
		//log_mem_dump("ucla_readdhr", hdr, sizeof(*hdr)+hdr->param_len);
		log_debug("%s s->status=%u Drop bad packet: packet session id=%08x, but now is %08x  hdr->encrypt=%u\n",
			s->doname, s->status, hdr->client_sid, s->client_sid, hdr->encrypt);
		return 0;
	}

	// update ip address and port
	s->ip = ntohl(addr.sin_addr.s_addr);
	s->port = ntohs(addr.sin_port);

	log_debug("port=%u  resetnum=%u status=%u %s recv from %u.%u.%u.%u:%u: version=%u cmd=%u, action=%u, param_len=%u, csid=%08x, dsid=%08x, request id=%u(is %s)\n",
		s->port, s->reset_num, s->status, s->doname, IP_SHOW(s->ip), s->port,hdr->ver, hdr->command, *((u_int8_t *)(hdr + 1)), hdr->param_len,
		hdr->client_sid, hdr->device_sid, hdr->request_id, hdr->request ? "request" : "reply");
	// 处理报文
	ucla_proc[s->status].proc_pkt(s);
	
	return 0;
}

static int ucla_read6(cl_thread_t *t)
{
	ucla_session_t *s;
	struct sockaddr_in6 addr;
	int n, pad_len;
	int addr_len;
	pkt_t *pkt;
	ucph_t *hdr;

	UCLA_ENTER();
	s = (ucla_session_t *)CL_THREAD_ARG(t);
	s->t_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_recv, ucla_read6, (void *)s, s->sock);

	addr_len = sizeof(struct sockaddr_in6);
	pkt = s->rcv_buf;

	n = pkt->total = (int)recvfrom(s->sock, (char *)pkt->data, LA_MAX_UDP_PKT, 0, 
			(struct sockaddr *)&addr, (socklen_t *)&addr_len);
	if (n <= (int)ucph_hdr_size) {
		log_err(true, "%s read udp failed\n", s->doname);
		return 0;
	}

	memset((void *)&s->last_ipv6_addr, 0, sizeof(s->last_ipv6_addr));
    s->recv_pkts++;
	hdr = (ucph_t *)pkt->data;
    s->rcv_hdr = hdr;

	//log_mem_dump("ucla_readdhr1", hdr, sizeof(*hdr));
	// 1. 解密、检查长度合法性
	if (hdr->encrypt) {
		if (((n - ucph_hdr_plain_size)%AES128_EKY_LEN) != 0) {
			log_debug("%s Drop bad packet: n=%d, but encrypt=1\n", s->doname, n);
			return 0;
		}
		dec_block((u_int8_t *)BOFP(hdr, ucph_hdr_plain_size), (n - ucph_hdr_plain_size), s->aes_key);
		pad_len = ((u_int8_t *)hdr)[n - 1];
		if (pad_len > AES128_EKY_LEN || pad_len <= 0) {
			log_debug("%s Drop bad packet: encrypt pad=%d\n", s->doname, pad_len);
			return 0;
		}
		pkt->total -= pad_len;
	}

	uc_hdr_order(hdr);
	
	if (ucph_hdr_len(hdr) < ucph_hdr_size 
		|| pkt->total != hdr->param_len + ucph_hdr_len(hdr))
	{
		log_debug("%s Drop bad packet: total=%d, ucph_hdr_len=%d, param_len=%d\n", 
			s->doname, pkt->total, ucph_hdr_len(hdr), hdr->param_len);
		return 0;
	}

	// 2. 检查会话ID
	if (s->client_sid != 0 && hdr->client_sid != s->client_sid) {
		//log_mem_dump("ucla_readdhr", hdr, sizeof(*hdr)+hdr->param_len);
		log_debug("%s s->status=%u Drop bad packet: packet session id=%08x, but now is %08x  hdr->encrypt=%u\n",
			s->doname, s->status, hdr->client_sid, s->client_sid, hdr->encrypt);
		return 0;
	}

	// update ip address and port
	s->ip = 0;
	s->port = ntohs(addr.sin6_port);

	log_debug("ipv6 read port=%u username=%s resetnum=%u status=%u %s recv from %u.%u.%u.%u:%u: version=%u cmd=%u, action=%u, param_len=%u, csid=%08x, dsid=%08x, request id=%u(is %s)\n",
		s->port, s->username, s->reset_num, s->status, s->doname, IP_SHOW(s->ip), s->port,hdr->ver, hdr->command, *((u_int8_t *)(hdr + 1)), hdr->param_len,
		hdr->client_sid, hdr->device_sid, hdr->request_id, hdr->request ? "request" : "reply");
	// 处理报文
	ucla_proc[s->status].proc_pkt(s);
	
	return 0;
}

ucla_session_t*  ucla_new(char *user_dir)
{
	ucla_session_t *s;
	
	s = (ucla_session_t *)cl_calloc(sizeof(ucla_session_t), 1);
	if (!s) {
		return NULL;
	}

	//UCLA_ENTER();
	strcpy(s->doname, user_dir);
	STLC_INIT_LIST_HEAD(&s->link);
	
	s->status = UCLAS_IDLE;
	s->last_status = UCLAS_IDLE;
	s->idle_time = 0;
	s->dmap = 0xff;

	// init default time parament
	uc_set_default_time_param(&s->time_param, &s->time_param_net);

	//这里还是改成30s吧，服务器现在超时用的30s，主要是服务器把reset报文去掉了，如果app超时时间长的话会很久才能连接上
	s->time_param_cur = &s->time_param.dev;
	//s->time_param_cur = &s->time_param.wan_front;

	memcpy(s->uuid, cl_priv->uuid_bin, sizeof(s->uuid));
	
	s->port = UCLA_PORT;
	s->sock = INVALID_SOCKET;
	s->disp_sock = INVALID_SOCKET;
	s->disp_sock6 = INVALID_SOCKET;
	
	if (ucla_udp_socket(s, s->is_ipv6) != RS_OK) {
		cl_free(s);
		return NULL;
	}
	ucla_renew_disp_socket(s);

	STLC_INIT_LIST_HEAD(&s->send_list);
	s->rcv_buf = pkt_new(0, LA_MAX_UDP_PKT, 0);

	//触发状态机
	ucla_set_status(s, UCLAS_IDLE);

	stlc_list_add_tail(&s->link, &plc->server_client);

	s->index = plc->index++;

	return s;
}

static void ucla_conf_init(char *user_dir)
{
	ucla_session_t *s;

	s = ucla_get_ses_by_doname_only(user_dir);
	if (!s) {
		return;
	}

	//用户配置读取
	la_user_init(s, user_dir);
	//圈子配置读取
	la_home_init(s, user_dir);
	//设备配置读取
	la_member_init(s, user_dir);
	//标签配置读取
	la_label_init(s);
	//规则配置读取
	la_rule_init(s);
	//模板配置读取
	la_template_init(s);
	//分享成员配置读取
	la_share_init(s);
	//快捷按键配置读取
	la_sc_init(s);
	//能力文件缓存读取
	la_cap_init(s);
	//自定义文件缓存读取
	la_cap_custom_init(s);
	
	//dns缓存读取
	la_dns_init(s);
}

static bool user_la_add_valid(user_t *user)
{
	//设备已添加圈子
	if ((user->home_id != 0) &&
		(user->home_id != 0xffffffff)) {
		return false;
	}
	if (user->is_phone_user ||
		(user->parent != NULL && 
		user->parent->is_phone_user)) {
		return false;
	}
	//tcp设备直接添加
	if (!user->is_udp_ctrl) {
		return true;
	}
	if (user->sn == 0) {
		return false;
	}

	return true;
}

bool user_is_support_la(user_t *user)
{
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	
	if (user->is_phone_user ||
		(user->parent != NULL && 
		user->parent->is_phone_user)) {
		return false;
	}

	if (!user->is_udp_ctrl) {
		return false;
	}

	if (lan_user_is_la_support(user)) {
		return true;
	}

	if (!user->smart_appliance_ctrl) {
		UCLA_ENTER();
		return false;
	}
	
	sac = user->smart_appliance_ctrl;
	if (!sac->sub_ctrl) {
		UCLA_ENTER();
		return false;
	}

	air_ctrl = sac->sub_ctrl;

	if (!air_ctrl->com_udp_dev_info.is_support_la) {
		return false;
	}

	return true;
}

void la_do_dev_estab_proc(user_t *puser)
{
	if (!la_is_valid()) {
		return;
	}
	if (plc->has_any_home ||
		(!puser) ||
		(!plc->callback) ||
		(!plc->p_null_home)) {
		return;
	}
	event_push_err(plc->callback, LA_HOME_ADDDEV_CHANGED, plc->p_null_home->handle, 
		plc->callback_handle, plc->p_null_home->conf.home_id);
}

void la_do_dev_error_proc(user_t *puser)
{
	if (!la_is_valid()) {
		return;
	}
	if (plc->has_any_home ||
		(!puser) ||
		(!plc->callback) ||
		(!plc->p_null_home)) {
		return;
	}
	event_push_err(plc->callback, LA_HOME_ADDDEV_CHANGED, plc->p_null_home->handle, 
		plc->callback_handle, plc->p_null_home->conf.home_id);
}

ucla_session_t *ucla_get_session_by_homeid(u_int32_t home_id)
{
	la_home_t *phome;
	ucla_session_t *s;

	phome = ucla_find_home_by_id(home_id);
	if (!phome) {
		return NULL;
	}

	if (phome->session_ptr) {
		return phome->session_ptr;
	}

	s = la_get_session_by_handle2(phome->handle);

	return s;
}

static u_int64_t la_get_any_sn()
{
	user_t *user, *usern;
	user_t *puser_online = NULL;
	user_t *puser_first = NULL;

	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (!puser_first) {
			puser_first = user;
		}
		if (user->online) {
			puser_online = user;
			break;
		}
	}
	cl_unlock(&cl_priv->mutex);

	if (puser_online) {
		return puser_online->sn;
	}
	if (puser_first) {
		return puser_first->sn;
	}

	return 0;
}

static u_int16_t la_get_err_doname(u_int8_t *buf)
{
	u_int16_t len = 0;
	la_doname_info_t *pd, *pdn;

	stlc_list_for_each_entry_safe(la_doname_info_t, pd, pdn, &plc->invalid_doname, link) {
		pd->valid_doname[sizeof(pd->valid_doname)-1] = 0;
		len += sprintf((char *)buf+len, "%s\t", pd->valid_doname);
	}
	
	return len;
}

static la_doname_info_t *la_doname_find(bool valid, char *doname)
{
	la_doname_info_t *pd, *pdn;
	struct stlc_list_head *valid_doname = NULL;

	if (!doname) {
		return NULL;
	}

	if (valid) {
		valid_doname = &plc->valid_doname;
	} else {
		valid_doname = &plc->invalid_doname;
	}

	stlc_list_for_each_entry_safe(la_doname_info_t, pd, pdn, valid_doname, link) {
		if (strcmp(pd->valid_doname, doname) == 0) {
			return pd;
		}
	}

	return NULL;
}

static void la_doname_add(bool valid, char *doname)
{
	la_doname_info_t *pd = NULL;
	struct stlc_list_head *valid_doname = NULL;
	
	if (!doname) {
		return;
	}
	if (valid) {
		valid_doname = &plc->valid_doname;
	} else {
		valid_doname = &plc->invalid_doname;
	}
	pd = la_doname_find(valid, doname);
	if (pd) {
		return;
	}
	pd = cl_calloc(sizeof(*pd), 1);
	if (!pd) {
		return;
	}
	strncpy(pd->valid_doname, doname, (sizeof(pd->valid_doname) - 1));
	STLC_INIT_LIST_HEAD(&pd->link);
	stlc_list_add_tail(&pd->link, valid_doname);
}

static void la_doname_add2(bool valid, la_doname_info_t *pd)
{
	struct stlc_list_head *valid_doname = NULL;
	
	if (!pd) {
		return;
	}
	if (valid) {
		valid_doname = &plc->valid_doname;
	} else {
		valid_doname = &plc->invalid_doname;
	}
	
	STLC_INIT_LIST_HEAD(&pd->link);
	stlc_list_add_tail(&pd->link, valid_doname);
}

static void la_doname_clean(bool valid)
{
	la_doname_info_t *pd, *pdn;
	struct stlc_list_head *valid_doname = NULL;
	
	if (valid) {
		valid_doname = &plc->valid_doname;
	} else {
		valid_doname = &plc->invalid_doname;
	}

	stlc_list_for_each_entry_safe(la_doname_info_t, pd, pdn, valid_doname, link) {
		stlc_list_del(&pd->link);
		SAFE_FREE(pd);
	}
}

static void do_add_free_dev_to_dev_wait()
{
	user_t *user, *usern;
	la_member_t *pmem;

	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (!user_la_add_valid(user)) {
			continue;
		}
		//check
		pmem = la_dev_wait_find_by_sn(user->sn);
		if (pmem) {
			continue;
		}
		//这里要单独处理下，如果是家庭服务器，就直接smartconf增加，国为家庭服务器没有wifi。
		if (IJ_HOMESERVER == user->sub_type) {
			user->is_smartconf = true;
		}
		//如果不是一键配置的且已经被添加过圈子，者不要添加
		if (!user->is_smartconf && lan_user_is_home_added(user)) {
			continue;
		}
		//这里判断下密码是否非法
		if (mem_is_all_zero(user->passwd_md5, 16)) {
			log_debug("sn=%"PRIu64" passwd is all zero\n", user->sn);
			continue;
		}
		//malloc
		pmem = ucla_member_new();
		if (!pmem) {
			break;
		}
		if (user->is_smartconf) {
			pmem->conf.flag |= LA_MEMBER_FLAG_SMARTCONF;
		}
		pmem->conf.flag |= LA_MEMBER_FLAG_NEED_ADD;
		pmem->conf.sn = user->sn;
		memcpy((void *)pmem->conf.dev_passwd, (void *)user->passwd_md5, 16);
		
		log_debug("sn=%"PRIu64" la add to home and is_la=%u\n", 
			user->sn, pmem->conf.is_la);		
		la_dev_wait_add(pmem);
	}
	cl_unlock(&cl_priv->mutex);
}

static void do_backgroud_proc()
{
	ucla_session_t *s, *n;
	
	//session释放
	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		//处于后台
		if (plc->run_in_background ) {
			if (s->status != UCLAS_SLEEP) {
				ucla_set_status(s, UCLAS_SLEEP);
			}
			continue;
		}
		//处于非后台
		if (!plc->run_in_background) {
			if (s->status == UCLAS_SLEEP) {
				s->idle_time = 0;
				ucla_set_status(s, UCLAS_IDLE);
			}
			continue;
		}
	}	
}

static void do_backgroud()
{
	if (plc->run_in_background == cl_priv->run_in_background) {
		return;
	}
	
	//log_debug("do_backgroud plc->run_in_background=%u cl_priv->run_in_background=%u\n", plc->run_in_background, cl_priv->run_in_background);
	
	plc->run_in_background = cl_priv->run_in_background;
	do_backgroud_proc();
}

static void do_disp_ext_proc()
{
	static u_int32_t last_time = 0;
	u_int32_t now = get_sec();

	if (last_time == 0) {
		last_time = now;
		return;
	}

	if ((last_time + 6) > now) {
		return;
	}
	ucla_clean_disp_ext();
	last_time = now;
}

static int la_comm_timer(cl_thread_t *t)
{
//	int num = 0;
	
	plc->t_comm = NULL;
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm, la_comm_timer, NULL, TIME_N_SECOND(2));

	//这里先将没在圈子里的设备添加到等待设备链表中
	do_add_free_dev_to_dev_wait();

	//为了节省手机流量啥的，这里做下判断，后台模式时session都去睡眠，变为非后台时再去唤醒
	do_backgroud();

	//这里关闭下拦截dispatch发送，避免意外
	do_disp_ext_proc();

	//大概10分钟查询一次中转设备信息
	do_trans_query_check();
	
	return 0;	
}

static void la_doname_save()
{
	char buf[1024];
	FILE *fp = NULL;
	ucla_session_t *s, *n;
	char fn[UCLA_FILE_PATH_LEN];

	if (!cl_priv->priv_dir) {
        return;
    }
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DONAME_INFO);
	fp = fopen(fn, "w");
	if (!fp) {
		return;
	}

	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		sprintf(buf, "%s\n", s->doname);
		fputs(buf, fp);
	}

	fclose(fp);
}

static void ucla_oem_session_init()
{
	char buf[1024];
	char res_doname[64];
	char doname[64];
	FILE *fp = NULL;
	ucla_session_t *s;
	char fn[UCLA_FILE_PATH_LEN];

	if (!cl_priv->priv_dir) {
        return;
    }
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DONAME_INFO);
	fp = fopen(fn, "r");
	if (!fp) {
		return;
	}

	while(fgets(buf, sizeof(buf)-1, fp)) {
		memset((void *)doname, 0, sizeof(doname));
		if (sscanf(buf, "%s\n", doname) != 1) {
			continue;
		}
		s = ucla_get_ses_by_doname(doname);
		if (s) {
			continue;
		}

		memset((void *)res_doname, 0, sizeof(res_doname));
		strcpy(res_doname, doname);
		disp_resolv_doname(1, (const char (* const)[64])res_doname);
		log_debug("%s add oem server=%s\n", __FUNCTION__, doname);
		s = ucla_new(doname);
		ucla_conf_init(doname);
		//oem域名直接连接服务器去不用regserver了
		//ucla_set_status(s, UCLAS_DISPATCH);
	}

	fclose(fp);
}

static void do_comm_info_save()
{
	ucla_session_t *s, *n;
	
	if (!cl_priv->priv_dir) {
		log_debug("cl_priv->priv_dir is null !!!!!!!!!!!!!!!!!!!!1\n");
		return;
	}

	if (plc->need_back_passwd_save) {
		plc->need_back_passwd_save = false;
		la_back_passwd_proc();
	}

	if (plc->need_dev_wait_save) {
		plc->need_dev_wait_save = false;
		la_dev_wait_list_save();
	}

	if (plc->need_dev_pass_sync_save) {
		plc->need_dev_pass_sync_save = false;
		la_dev_pass_sync_list_save();
	}

	if (plc->need_doname_save) {
		plc->need_doname_save = false;
		la_doname_save();
	}

	//手机界面强用户登陆中时不要保存配置
	if (plc->phone_logining) {
		log_debug("do_comm_info_save phone_logining !!!!!!!!!!!!!!!!!!!!!!!!!!\n ");
		return;
	}
	
	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		if (s->need_save_conf) {
			s->need_save_conf = false;
			//UCLA_ENTER();
			_ucla_user_conf_save(s);
		}
		
		if (s->need_save_dns) {
			la_dns_save(s);
		}
	}

	if (plc->need_save_phone) {
		plc->need_save_phone = false;
		la_phone_save();
	}
}

static void do_info_dump()
{
#ifdef WIN32	
	ucla_session_info_dump();
#endif
}

static void do_all_home_query()
{
	ucla_session_t *s, *sn;
	
	if (!plc->has_def_home) {
		return;
	}
	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		if (!ucla_session_is_ok(s)) {
			continue;
		}
		if (s->need_query_all_home) {
			s->need_query_all_home = false;
			ucla_all_misc_query(s);
			ucla_all_home_query(s);
			s->need_query_home_all_info = false;
		}
		if (s->need_query_home_all_info) {
			s->need_query_home_all_info = false;
			// TODO:主动去查询各圈子设备等
			ucla_each_home_query(s);
			// TODO:主动去查询各圈子成员等
			ucla_each_share_query(s);
		}
	}		
}

static void do_dev_wait_proc()
{
	ucla_session_t *s;
	la_member_t *m, *mn, *pt;
	la_home_t *phome;
	bool need_push = false;
	user_t *puser;
	//dev_probe_info_t *pprobe = NULL;

	if (!plc->has_def_home) {
		return;
	}
	if (stlc_list_empty(&plc->dev_wait_list)) {
		return;
	}
	//得到圈子
	if (plc->cur_home_id) {
		phome = ucla_find_home_by_id(plc->cur_home_id);
	} else {
		phome = ucla_get_def_home();
	}
	if (!phome) {
		return;
	}
	plc->cur_home_id = phome->conf.home_id;
	//得到session
	if (!phome->session_ptr) {
		s = la_get_session_by_handle2(phome->handle);
	} else {
		s = phome->session_ptr;
	}
	//这里就不判断会话是否在线了，不在线也要添加
	if (!s) {
		return;
	}
	//将这些设备添加到对应圈子下
	stlc_list_for_each_entry_safe(la_member_t , m, mn, &plc->dev_wait_list, link) {
		puser = user_lookup_by_sn(m->conf.sn);
		if (!puser) {
			//估计设备删除了
			la_dev_wait_del(m);
			continue;
		}
		//这里要尽量判断下设备到底有没有被添加过圈子，有可能被添加到其他服务器去了，尽量判断吧。。。。
		//局域网扫描可以判断设备是是否添加个圈子。。
		if (!puser->is_smartconf && lan_user_is_home_added(puser)) {
			la_dev_wait_del(m);
			continue;
		}
		//直接当成非联动添加到服务器去，让服务器来判断是否真的支持联动
		m->conf.is_la = 0;
		stlc_list_del(&m->link);
		//这里处理一种情况，就是在本圈子已有设备的情况下还原设备重新一键配置,删除老的重新添加新的，因为服务器上保存的密码可能不是123456
		pt = ucla_find_member_by_sn_from_home(phome, m->conf.sn);
		if (pt) {
			stlc_list_del(&pt->link);
			cl_free(pt);
		}
		ucla_member_add(m, phome);
		la_user_update_homeid(m->conf.sn, phome->conf.home_id);
		m->conf.home_id = phome->conf.home_id;
		if (s) {
			s->need_save_conf = true;
		}
		plc->need_dev_wait_save = true;
		need_push = true;
	}

	if (need_push) {
#if 1		
		//先直接报成功，后面添加失败后再删除吧
		ucla_home_event_push(phome->conf.home_id, LA_HOME_ADDDEV_CHANGED);
#else
		//如果外网不通的情况下，直接报成功快速
		if (!ucla_get_any_enstablis_session()) {
			ucla_home_event_push(phome->conf.home_id, LA_HOME_ADDDEV_CHANGED);
		}
#endif
		ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
 	} 
}

static void do_dev_add_proc()
{
	pkt_t *pkt;
	ucla_session_t *s;
	la_home_t *phome, *phomen;
	la_user_t *puser, *pusern;
	la_member_t *pmem, *pmemn;
	uc_home_conf_adddev_t *pa;

	//如果没有可用的会话就不用管了
	if (!ucla_get_any_enstablis_session()) {
		return;
	}
	//判断所有待添加设备的圈子是否可以添加
	cl_lock(&cl_priv->mutex);	
	stlc_list_for_each_entry_safe(la_user_t, puser, pusern, &cl_priv->la_user, link) {
		stlc_list_for_each_entry_safe(la_home_t, phome, phomen, &puser->home_link, link) {
			stlc_list_for_each_entry_safe(la_member_t, pmem, pmemn, &phome->member_link, link) {
				if (!(pmem->conf.flag&LA_MEMBER_FLAG_NEED_ADD)) {
					continue;
				}
				// TODO:添加失败过,这里没想好怎么处理，是直接删除吗?先留着吧
				if (pmem->add_num) {
					continue;
				}

				s = ucla_get_any_enstablis_session();
				if (!s) {
					goto done;
				}
				if (s->adddev_sn) {
					//已经有设备在添加了，先不忙，一个一个添加，错误时好删除
					goto done;
				}
				pkt = ucla_pkt_new(CMD_HOME_CONFIG, (sizeof(uc_home_conf_adddev_t) + sizeof(uc_la_dev_info_t)), 
							true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
				if (!pkt) {
					log_debug("ucla_pkt_new faled\n");
					goto done;
				}
				pa = get_uascp_payload(pkt, uc_home_conf_adddev_t);
				pa->action = UCAU_ADDDEV;
				pa->count = 1;
				pa->home_id = htonl(phome->conf.home_id);
				//数据填充,一个一个添加，主要是为了错误提示
				pa->dev[0].sn = ntoh_ll(pmem->conf.sn);
				s->adddev_sn = pmem->conf.sn;
				
				if (pmem->conf.flag&LA_MEMBER_FLAG_SMARTCONF) {
					pa->dev[0].flag |= BIT(0);
				}
				//添加个局域网添加标志，表示当前设备在局域网扫描中
				if (get_dev_lan_ipaddr(pmem->conf.sn) != 0) {
					pa->dev[0].flag |= BIT(1);
				}
				memcpy((void *)pa->dev[0].dev_passwd, (void *)pmem->conf.dev_passwd, 16);
				nd_la_memdump("add passpa->dev[0].dev_passwd", pa->dev[0].dev_passwd, 16);
				log_debug("%s add sn=%"PRIu64" to home=%08x is_la=%u\n", 
					__FUNCTION__, pmem->conf.sn, phome->conf.home_id, pmem->conf.is_la);
				nd_la_debug(s, "%s add sn=%"PRIu64" to home=%08x is_la=%u\n", 
					__FUNCTION__, pmem->conf.sn, phome->conf.home_id, pmem->conf.is_la);
				
				ucla_request_add(s, pkt);
				pmem->add_num++;
				break;
			}
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);
}

static void ucla_dev_ver_limit_req(ucla_session_t *s)
{
	u_int8_t *req;
	pkt_t *pkt;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_APP_USER, 4, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	req = get_uascp_payload(pkt, u_int8_t);

	*req = UCAU_TABLE_QUERY;

	ucla_request_add(s, pkt);
}

static void do_dev_ver_req()
{
	ucla_session_t *s, *n;
	
	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		if (!s->need_dev_ver_req) {
			continue;
		}

		if (!ucla_session_is_ok(s)) {
			break;;
		}

		s->need_dev_ver_req = false;
		ucla_dev_ver_limit_req(s);
		break;
	}
}

static u_int32_t dev_ver_get_local_ver(user_t *puser)
{
	u_int32_t ver = 0;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;

    if ((sac = puser->smart_appliance_ctrl) == NULL || 
		!sac->sub_ctrl) {
        return ver;
    }

	air_ctrl = (smart_air_ctrl_t*)sac->sub_ctrl;

	ver = VERTION((air_ctrl->stat.soft_version.major), (air_ctrl->stat.soft_version.minor), (air_ctrl->stat.soft_version.revise));

	return ver;
}

static u_int32_t dev_ver_get_server_ver(user_t *puser)
{
	la_dev_ver_limit_t *d, *n;

	stlc_list_for_each_entry_safe(la_dev_ver_limit_t, d, n, &plc->dev_ver_limit_list, link) {
		if (d->type == TP_DS007 &&
			d->sub_type == puser->sub_type &&
			d->ext_type == puser->ext_type) {
			return d->ver;
		}
	}

	return 0;
}

void tcp_dev_online(user_t *user)
{
	//tcp上线，直接保存一下类型缓存
	if (app_is_la()) {
		la_dev_type_save();
	}
}

static void do_dev_ver_check()
{
	user_t *puser, *n;
	u_int32_t verl = 0;
	u_int32_t vers = 0;
	bool too_low = false;
	
	if (!plc->need_cal_dev_ver) {
		return;
	}
	//如果没有限制文件，可以不管了
	if (mem_is_all_zero(plc->last_ver, sizeof(plc->last_ver))) {
		la_dev_type_save();
		return;
	}
	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, puser, n, &cl_priv->user, link) {
		too_low = false;
		
		if (puser->status != CS_ESTABLISH) {
			continue;
		}
		verl = dev_ver_get_local_ver(puser);
		if (0 == verl) {
			continue;
		}
		vers = dev_ver_get_server_ver(puser);
		if (0 == vers) {
			//如果服务器没限制的设备就直接认为是有效的。
			puser->dev_ver_is_valid = true;
			puser->dev_ver_is_too_low = false;
			plc->need_cal_dev_ver = false;
			dev_type_sync(puser);
			continue;
		}
		puser->dev_ver_is_valid = true;
		if (verl < vers) {
			too_low = true;
		} 
		if (puser->dev_ver_is_too_low != too_low) {
			puser->dev_ver_is_too_low = too_low;
			event_push(puser->callback, UE_INFO_MODIFY, puser->handle, puser->callback_handle);
		}
		plc->need_cal_dev_ver = false;
		dev_type_sync(puser);
	}
	cl_unlock(&cl_priv->mutex);
}

static bool do_user_sync(user_t *puser, la_member_t *pmem)
{
	ucla_session_t *s;
	pkt_t *pkt;
	uc_home_conf_modify_passwd_dev_t *pmpd = NULL;
	uc_la_dev_modify_info_t *pmi = NULL;

	UCLA_ENTER();
	//无会话不修改
	s = ucla_get_any_enstablis_session();
	if (!s) {
		return false;
	}
	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, (sizeof(uc_home_conf_modify_passwd_dev_t) + 
		sizeof(uc_la_dev_modify_info_t)), true, true, 0, s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		log_debug("ucla_pkt_new faled\n");
		return false;
	}	
	
	pmpd = get_uascp_payload(pkt, uc_home_conf_modify_passwd_dev_t);
	pmpd->action = UCAU_MODIFY;
	pmpd->count = 1;
	pmpd->home_id = htonl(puser->home_id);
	pmi = (uc_la_dev_modify_info_t *)&pmpd[1];
	pmi->sn = ntoh_ll(puser->sn);
	memcpy((void *)pmi->dev_passwd, (void *)pmem->conf.dev_passwd, 16);
	//memdumpone("do_user_sync", pmi->dev_passwd, 16);
	s->modify_passwd_sn = puser->sn;
	log_debug("goto %s for passwd modify s->my_request_id=%u\n", s->doname, s->my_request_id);

	ucla_request_add(s, pkt);

	return true;
}

static void do_dev_passwd_sync_check()
{
	user_t *puser;
	la_member_t *pmem, *pmemn;
	ucla_session_t *s;

	if (stlc_list_empty(&plc->dev_pass_sync_list)) {
		return;
	}
	//没连接上服务器
	s = ucla_get_any_enstablis_session();
	if (!s) {
		return;
	}
	//上次修改的还没完成
	if (s->modify_passwd_sn != 0) {
		return;
	}
	UCLA_ENTER();
	//找到user
	stlc_list_for_each_entry_safe(la_member_t, pmem, pmemn, &plc->dev_pass_sync_list, link) {
		puser = user_lookup_by_sn(pmem->conf.sn);
		if (!puser) {
			stlc_list_del(&pmem->link);
			cl_free(pmem);
			plc->need_dev_pass_sync_save = true;
			continue;
		}
		
		if (do_user_sync(puser, pmem)) {
			break;
		}
	}
}

static void do_phone_login_session_replace(ucla_session_t *s)
{
	la_user_t *puser;

	UCLA_ENTER();
	if (!s) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		puser = ucla_user_new();
		if (!puser) {
			return;
		}
		//随便写，主要是为了保存下去
		puser->conf.user_id = s->user_id = 1000;
		ucla_user_add(s, puser);
	}
	s->has_user = true;
	memcpy((void *)s->username, (void *)plc->user_name, sizeof(s->username));
	memcpy((void *)s->key, (void *)plc->passwd, sizeof(s->key));
	memset((void *)s->uuid, 0, sizeof(s->uuid));
	memcpy((void *)&puser->conf.user_name, (void *)s->username, sizeof(puser->conf.user_name));
	memcpy((void *)&puser->conf.passwd, (void *)s->key, sizeof(puser->conf.passwd));
	memcpy((void *)&puser->conf.uuid, (void *)s->uuid, sizeof(puser->conf.uuid));

	ucla_session_reset(s);
	_ucla_user_conf_save(s);
}

static void do_phone_login_check()
{
	ucla_session_t *s;
	la_phone_t *phone;
	
	if (!plc->need_phone_login_delay) {
		return;
	}
	//这里表示app版本从2.9非联动 升级到3.0支持联动以上版本时出现的手机账号登陆处理情况
	//这里的处理办法是直接替换会话session用户
	//先直接保存手机账号
	phone = ucla_find_phone_by_name((char *)plc->user_name);
	if (!phone) {
		phone = ucla_phone_new();
		if (!phone) {
			return;
		}
		ucla_phone_add(phone);
		ucla_set_cur_phone(phone);
		memcpy((void *)phone->conf.passwd, (void *)plc->passwd, sizeof(phone->conf.passwd));
		memcpy((void *)phone->conf.user_name, (void *)plc->user_name, sizeof(phone->conf.user_name));
	}
	//再替换session用户
	s = ucla_get_any_valid_session();
	if (!s) {
		return;
	}
	plc->need_phone_login_delay = false;
	
	do_phone_login_session_replace(s);
}

void do_dev_la_sync()
{
	if (!la_is_valid()) {
		return;
	}
	plc->need_la_sync_check = true;
	la_comm_timer_proc_reset();
}

static int la_comm_timer_proc(cl_thread_t *t)
{
	plc->t_comm_proc = NULL;
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm_proc, la_comm_timer_proc, NULL, TIME_N_SECOND(5));

	//UCLA_ENTER();
	//处理设备等待链表
	do_dev_wait_proc();

	//向服务器发送添加报文处理,这个要在查询圈子信息前
	do_dev_add_proc();

	//强用户创建
	//do_comm_force_user_create();

	//检查设备支持联动属性是否变化了，如升级，降级，数据没及时读取等
	//do_dev_la_sync_check();

	//检查设备密码是否需要同步
	do_dev_passwd_sync_check();

	//保存配置
	do_comm_info_save();

	//打印信息dump
	do_info_dump();

	//查询所有家庭数
	do_all_home_query();

	//设备版本限制请求
	do_dev_ver_req();

	//设备版本限制检查
	do_dev_ver_check();

	//检查是否需要登陆手机账号
	do_phone_login_check();

	return 0;
}

void dev_ver_reset_cal(user_t *user)
{
	if (!la_is_valid()) {
		if (user) {
			//如果服务器没限制的设备就直接认为是有效的。
			user->dev_ver_is_valid = true;
			user->dev_ver_is_too_low = false;
			dev_type_sync(user);
		}
		return;
	}
	plc->need_cal_dev_ver = true;
	la_comm_timer_proc_reset();
}

void do_passwd_sync_to_server(user_t *user)
{
	la_home_t *phome = NULL;
	la_member_t *pmem = NULL;
	la_member_t *ppassmem = NULL;
	ucla_session_t *s = NULL;

	UCLA_ENTER();
	if (!user ||
		!la_is_valid()) {
		return;
	}
	if ((user->home_id == 0) ||
		(user->home_id == 0xffffffff)) {
		return;
	}
	pmem = la_dev_wait_find_by_sn(user->sn);
	if (pmem) {
		memcpy(pmem->conf.dev_passwd, user->passwd_md5, 16);
		plc->need_dev_wait_save = true;
		la_comm_timer_proc_reset();
		return;
	}
	if (user_is_support_la(user)) {
		return;
	}	
	phome = ucla_find_home_by_id(user->home_id);
	if (!phome) {
		return;
	}
	s = phome->session_ptr;
	if (!s) {
		s = ucla_get_session_by_homeid(user->home_id);
	}
	if (!s) {
		return;
	}
	pmem = ucla_find_member_by_sn_from_home(phome, user->sn);
	if (!pmem) {
		return;
	}
	memcpy(pmem->conf.dev_passwd, user->passwd_md5, 16);
	ucla_user_conf_save(s);
	//flag不为0，表示设备还没往服务器添加成功，不用向服务器同步了.
	if (pmem->conf.flag) {
		return;
	}
	//添加到密码同步链表
	ppassmem = la_dev_pass_sync_find_by_sn(user->sn);
	if (!ppassmem) {
		ppassmem = ucla_member_new();
		if (!ppassmem) {
			return;
		}
		stlc_list_add(&ppassmem->link, &plc->dev_pass_sync_list);
	}
	log_debug("%"PRIu64" need passwd sync for server !!!!!!!!!!!!!\n");
	memcpy((void *)&ppassmem->conf, (void *)&pmem->conf, sizeof(ppassmem->conf));
	
	plc->need_dev_pass_sync_save = true;
	
	la_comm_timer_proc_reset();
}

void la_comm_timer_proc_reset()
{
	CL_THREAD_OFF(plc->t_comm_proc);
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm_proc, la_comm_timer_proc, NULL, 0);
}

void la_comm_timer_reset()
{
	CL_THREAD_OFF(plc->t_comm);
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm, la_comm_timer, NULL, TIME_N_SECOND(0));	
}

void la_comm_timer_delay()
{
	UCLA_ENTER();
	CL_THREAD_OFF(plc->t_comm);
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm, la_comm_timer, NULL, TIME_N_SECOND(15));	
}

static int la_delay_timer(cl_thread_t *t)
{
	plc->t_timer = NULL;
	
	UCLA_ENTER();
	
	if (plc->callback == 0) {
		log_debug("plc->callback is null !!!!!!!!!!!!\n");
		CL_THREAD_TIMER_ON(MASTER, plc->t_timer, la_delay_timer, NULL, 50);
		return 0;
	}

	//设备待添加链表配置初始化
	la_dev_wait_list_init();
	//各session初始化
	ucla_session_init();
	la_dev_passwd_sync_list_init();
	//初始化备份密码
	la_back_passwd_init();

	plc->run_in_background = cl_priv->run_in_background;

	//如果有默认圈子，表示是有圈子可以发送事件
	ucla_event_push(LA_USER_CHANGED);
	
	return 0;	
}

void ucla_rule_free(la_rule_t *prule)
{
	if (prule) {
		SAFE_FREE(prule->rule);
		cl_free(prule);
	}
}

void ucla_home_rule_free(la_home_t *phome)
{
	la_rule_t *prule, *rulen;
	
	stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
		stlc_list_del(&prule->link);
		ucla_rule_free(prule);
	}	
}

void ucla_label_free(la_label_t *pl)
{
	if (pl) {
		SAFE_FREE(pl->p_sn);
		cl_free(pl);
	}	
}

bool ucla_home_label_free(la_home_t *phome)
{
	bool ret = false;
	la_label_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_label_t, pl, pln, &phome->label_link, link) {
		stlc_list_del(&pl->link);
		ucla_label_free(pl);
		ret = true;
	}

	return ret;
}

la_label_t *ucla_find_home_label_by_id(la_home_t *phome, u_int16_t id)
{
	la_label_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_label_t, pl, pln, &phome->label_link, link) {
		if (pl->conf.id == id) {
			return pl;
		}
	}

	return NULL;
}

void ucla_label_flag_set(la_home_t *phome, u_int8_t flag)
{
	la_label_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_label_t, pl, pln, &phome->label_link, link) {
		pl->flag = flag;
	}	
}

la_dict_t *ucla_dict_add_or_modify(la_home_t *phome, u_int8_t *key, u_int16_t key_len, u_int8_t *value, u_int16_t value_len)
{
	la_dict_t *dict = NULL;

	dict = ucla_find_home_dict_by_key(phome, key, key_len);
	if (!dict) {
		log_debug("add new home dict key_len %u value_len %u\n", key_len, value_len);
		dict = cl_calloc(1, sizeof(*dict));
		if (!dict) {
			return NULL;
		}
		
		stlc_list_add_tail(&dict->link, &phome->dict_link);
		
	} else {
		log_debug("modify home dict from key_len %u to key_len %u\n", dict->key_len, key_len);
		
		SAFE_FREE(dict->key);
		SAFE_FREE(dict->value);
	}
	

	dict->key = cl_calloc(1, key_len);
	if (!dict->key) {
		goto err;
	}
	memcpy(dict->key, key, key_len);
	dict->key_len = key_len;

	dict->value = cl_calloc(1, value_len);
	if (!dict->value) {
		goto err;
	}
	memcpy(dict->value, value, value_len);
	dict->value_len = value_len;


	return dict;
err:
	if (dict) {
		stlc_list_del(&dict->link);
		SAFE_FREE(dict->key);
		SAFE_FREE(dict->value);
		cl_free(dict);
	}
	
	return NULL;
}

void ucla_dict_free(la_dict_t *pl)
{
	if (pl) {
		stlc_list_del(&pl->link);

		SAFE_FREE(pl->key);
		SAFE_FREE(pl->value);
		SAFE_FREE(pl);
	}	
}

/**
	删除家庭全部dict
*/
void ucla_home_dict_free(la_home_t *phome)
{
	la_dict_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_dict_t, pl, pln, &phome->dict_link, link) {
		ucla_dict_free(pl);
	}
}

la_dict_t *ucla_find_home_dict_by_key(la_home_t *phome, u_int8_t *key, u_int16_t key_len)
{
	la_dict_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_dict_t, pl, pln, &phome->dict_link, link) {
		if (pl->key && pl->key_len == key_len && memcmp(pl->key, key, key_len) == 0) {
			return pl;
		}
	}

	return NULL;
}

void ucla_dict_del(la_home_t *phome, u_int8_t *key, u_int16_t key_len)
{
	la_dict_t *dict = ucla_find_home_dict_by_key(phome, key, key_len);
	if (dict) {
		ucla_dict_free(dict);		
		ucla_home_event_push(phome->conf.home_id, LA_DICT_MODIFY);
	}
}


void ucla_template_free(la_home_t *phome)
{
	int i;

	for(i = 0; phome->url_array && (i < phome->url_num); i++) {
		SAFE_FREE(phome->url_array[i]);
	}

	SAFE_FREE(phome->url_array);
	phome->url_num = 0;
}

void ucla_home_free(la_home_t *phome)
{
	if (!phome) {
		return;
	}
	
	//释放url
	ucla_template_free(phome);
	
	//释放成员
	ucla_home_free_memlist(phome);

	//释放规则
	ucla_home_rule_free(phome);

	//释放share
	SAFE_FREE(phome->share);

	//释放share注册表
	SAFE_FREE(phome->share_desc_array);
	phome->share_desc_num = 0;

	//释放规则缓存buffer
	SAFE_FREE(phome->ptotal_rule);

	//标签释放
	ucla_home_label_free(phome);

	// 字典释放
	ucla_home_dict_free(phome);

	cl_free(phome);
}

void ucla_user_free(la_user_t *puser)
{
	la_home_t *phome, *homen;

	if (!puser) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
		stlc_list_del(&phome->link);
		ucla_home_free(phome);
	}

	cl_free(puser);
}

void ucla_all_session_clean()
{
	ucla_session_t *s, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		ucla_user_del(s, s->user_id);
		s->idle_time = 0;
		ucla_set_status(s, UCLAS_IDLE);
	}

	plc->phone_passwd_err = false;
}

void ucla_all_session_query()
{
	ucla_session_t *s, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, s, sn, &plc->server_client, link) {
		s->need_login_query_user = true;
		//back old
		memcpy((void *)s->back_username, (void *)s->username, sizeof(s->username));
		memcpy((void *)s->back_key, (void *)s->key, sizeof(s->key));
		memcpy((void *)s->back_uuid, (void *)s->uuid, sizeof(s->uuid));

		memcpy((void *)s->username, (void *)plc->user_name, sizeof(s->username));
		memcpy((void *)s->key, (void *)plc->passwd, sizeof(s->key));
		memset((void *)s->uuid, 0, sizeof(s->uuid));
	}
}

void ucla_all_sleep_session_reset()
{
	ucla_session_t *sc, *sn;

	stlc_list_for_each_entry_safe(ucla_session_t, sc, sn, &plc->server_client, link) {
		if (sc->status == UCLAS_SLEEP) {
			ucla_session_reset(sc);
		}
	}	
}

ucla_session_t *ucla_get_ses_by_doname(char *doname)
{
	ucla_session_t *s , *n, *sret = NULL;

	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		if (strcmp(doname, s->doname) == 0) {
			sret = s;
			break;
		}
	}

	return sret;
}

ucla_session_t *ucla_get_ses_by_doname_only(char *doname)
{
	ucla_session_t *s , *n;
	bool found = false;

	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		if (strcmp(doname, s->doname) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		return NULL;
	}

	return s;
}

ucla_session_t *ucla_get_dehome_session()
{
	ucla_session_t *s , *n;
	
	if (!plc->has_def_home) {
		return NULL;
	}	

	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		if (s->has_def_home) {
			return s;
		}
	}

	return NULL;	
}

void ucla_all_goto_sleep()
{
	ucla_session_t *s , *n;

	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		if (s->status == UCLAS_SLEEP) {
			continue;
		}
		
		ucla_set_status(s, UCLAS_SLEEP);
	}	
}

u_int32_t ucla_get_dehome_id()
{
	la_user_t *puser, *usern;
	la_home_t *phome, *homen;
	u_int32_t home_id = 0;
	
	if (!plc->has_def_home) {
		return 0;
	}	

	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry_safe(la_user_t, puser, usern, &cl_priv->la_user, link) {
		stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
			if (ucla_is_def_home(phome)) {
				home_id = phome->conf.home_id;
				goto end;
			}
		}
	}

end:	
	cl_unlock(&cl_priv->mutex);

	return home_id;	
}

void ucla_set_def_home(ucla_session_t *s, la_home_t *phome)
{
	if (!phome || !s) {
		return;
	}

	phome->conf.type = IS_DEF_HOME_TYPE;
	plc->has_def_home = true;
	s->has_def_home = true;
}

bool ucla_is_def_home(la_home_t *phome)
{
	if (!phome) {
		return false;
	}

	return (phome->conf.type == IS_DEF_HOME_TYPE);
}

la_home_t *ucla_get_def_home()
{
	la_user_t *puser, *usern;
	la_home_t *phome, *homen;
	la_home_t *phome_ret = NULL;
//	u_int32_t home_id = 0;
	
	if (!plc->has_def_home) {
		return NULL;
	}	

	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry_safe(la_user_t, puser, usern, &cl_priv->la_user, link) {
		stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
			if (ucla_is_def_home(phome)) {
				phome_ret = phome;
				goto end;
			}
		}
	}

end:	
	cl_unlock(&cl_priv->mutex);

	return phome_ret;	
}

bool ucla_is_any_session_online()
{
	ucla_session_t *ps, *psn;

	stlc_list_for_each_entry_safe(ucla_session_t, ps, psn, &plc->server_client, link) {
		if (ucla_session_is_ok(ps)) {
			return true;
		}
	}

	return false;
}

static void ucla_session_free(ucla_session_t *s)
{
	if (!s) {
		return;
	}

	SAFE_FREE(s->pver_limit);	
	SAFE_FREE(s->rule_tmp);
	if (s->plabel_tmp) {
		SAFE_FREE(s->plabel_tmp->p_sn);
		SAFE_FREE(s->plabel_tmp);
	}
	SAFE_FREE(s->plbind_tmp);
	SAFE_FREE(s->ps_tmp);

	ucla_big_buf_free(s);
	
	cl_free(s);	
}

bool app_is_la()
{
	return (cl_priv->app_type == APP_TYPE_LINKAGE_SUPPORT);
}

static void la_dmap_init()
{
	plc->doname_map = cl_calloc((UCLA_MAX_DOANME_MAP + 1)*sizeof(char *), 1);
}

static void la_rsa_init()
{
	// TODO:读取私钥plc->rsa_priv
}

static void la_rsa_exit()
{
	SAFE_FREE(plc->rsa_priv);
}

static void la_conf_aes_init()
{
	MD5_CTX ctx;
	
	//初始化配置保存加密aes
	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)la_conf_key, (int)strlen(la_conf_key));
	MD5Final(plc->conf_aes_key, &ctx);
}

bool la_dmap_replace_doname(u_int8_t id, char *doname)
{	
	SAFE_FREE(plc->doname_map[id]);

	plc->doname_map[id] = cl_strdup(doname);

	return true;
}

static void la_file_clean(char *file)
{
	char fn[UCLA_FILE_PATH_LEN];

	if ((!file) ||
		(!cl_priv->priv_dir)) {
        return;
    }

	sprintf(fn, "%s/%s", cl_priv->priv_dir,file);
	remove(fn);	
}

void la_phone_save()
{
	la_phone_t *pp, *ppn;

	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	ucla_conf_save_t *pconf = NULL;

	save_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!save_buf) {
		log_debug("calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;

	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		//这里只保存当前手机号
		if (!ucla_is_cur_phone(pp)) {
			continue;
		}
		log_debug("la_phone_save %s flag=%u\n", pp->conf.user_name, pp->conf.flag);
		memcpy((void *)&pconf->data[save_len], (void *)&pp->conf, sizeof(pp->conf));
		save_len += sizeof(pp->conf);
		save_count++;
	}

	save_len = la_conf_fill_enc(pconf, save_len, save_count);

	if (save_len) {
		la_write_conf_rename_file(UCLA_LA_PHONE_FILE, UCLA_LA_PHONE_FILE_BACK);
		la_write_conf_2_file(save_buf, save_len, UCLA_LA_PHONE_FILE);
	} else {
		la_file_clean(UCLA_LA_PHONE_FILE);
		la_file_clean(UCLA_LA_PHONE_FILE_BACK);
	}
	
	SAFE_FREE(save_buf);	
}

static void la_phone_init()
{
	la_phone_conf_t *pcf;
	la_phone_t *ppt = NULL;

	int read_len = 0;
	int index = 0;
	int i;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;

	read_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!read_buf) {
		return;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	read_len = la_conf_read_from_file(read_buf, UCLA_CONF_SAVE_MAX_LEN, UCLA_LA_PHONE_FILE);
	if (read_len == 0) {
		read_len = la_conf_read_from_file(read_buf, UCLA_CONF_SAVE_MAX_LEN, UCLA_LA_PHONE_FILE_BACK);
	}
	read_len = la_conf_dec(read_buf, read_len);		

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		UCLA_ENTER();
		goto done;
	}
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*pcf)) <= read_len); i++) {
		pcf = (la_phone_conf_t *)&pconf->data[index];
		index += sizeof(*pcf);
		
		ppt = cl_calloc(sizeof(*ppt), 1);
		if (!ppt) {
			goto done;
		}
		STLC_INIT_LIST_HEAD(&ppt->link);
		memcpy((void *)&ppt->conf, (void *)pcf, sizeof(*pcf));
		stlc_list_add_tail(&ppt->link, &plc->la_phone);
	}
	
done:
	SAFE_FREE(read_buf);
}

static void la_phone_exit()
{
	la_phone_t *pp, *ppn;

	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		stlc_list_del(&pp->link);
		cl_free(pp);
	}
}

la_member_t *la_dev_wait_find_by_sn(u_int64_t sn)
{
	la_member_t *m, *mn;

	stlc_list_for_each_entry_safe(la_member_t, m, mn, &plc->dev_wait_list, link) {
		if (sn == m->conf.sn) {
			return m;
		}
	}	

	return NULL;
}

static void la_dev_wait_add(la_member_t *pmem)
{
	if (pmem) {
		stlc_list_add_tail(&pmem->link, &plc->dev_wait_list);
		plc->need_dev_wait_save = true;
		la_comm_timer_proc_reset();
	}
}

void la_dev_wait_del(la_member_t *pmem)
{
	if (pmem) {
		stlc_list_del(&pmem->link);
		SAFE_FREE(pmem);
		plc->need_dev_wait_save = true;
	}	
}

static void la_dev_wait_list_init()
{
	la_member_conf_t *pmembercf;
	la_member_t *pmember;

	int i;
	int read_len = 0;
	int index = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;

	read_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!read_buf) {
		return;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	read_len = la_conf_read_from_file(read_buf, UCLA_CONF_SAVE_MAX_LEN, UCLA_LA_DEV_WAIT_LIST_FILE);
	if (read_len == 0) {
		read_len = la_conf_read_from_file(read_buf, UCLA_CONF_SAVE_MAX_LEN, UCLA_LA_DEV_WAIT_LIST_FILE_BACK);
	}
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*pmembercf)) <= read_len); i++) {
		pmembercf = (la_member_conf_t *)&pconf->data[index];
		index += sizeof(*pmembercf);
		
		pmember = ucla_member_new();
		if (!pmember) {
			goto done;
		}

		memcpy((void *)&pmember->conf, (void *)pmembercf, sizeof(*pmembercf));
		stlc_list_add_tail(&pmember->link, &plc->dev_wait_list);
		la_user_login(0, pmember->conf.sn, pmember->conf.dev_passwd);
		log_debug("la_dev_wait_list_init sn=%"PRIu64"\n", pmember->conf.sn);
	}

done:
	SAFE_FREE(read_buf);		
}

static void la_dev_wait_list_free()
{
	la_member_t *m, *mn;

	stlc_list_for_each_entry_safe(la_member_t, m, mn, &plc->dev_wait_list, link) {
		stlc_list_del(&m->link);
		cl_free(m);
	}
}

static void la_dev_wait_list_save()
{
	la_member_t *pmem, *memn;
	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	ucla_conf_save_t *pconf = NULL;

	if (stlc_list_empty(&plc->dev_wait_list)) {
		la_file_clean(UCLA_LA_DEV_WAIT_LIST_FILE);
		la_file_clean(UCLA_LA_DEV_WAIT_LIST_FILE_BACK);
		return;
	}
	
	save_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!save_buf) {
		log_debug("calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;

	stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &plc->dev_wait_list, link) {
		memcpy((void *)&pconf->data[save_len], (void *)&pmem->conf, sizeof(pmem->conf));
		save_len += sizeof(pmem->conf);
		save_count++;
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);

	if (save_len) {
		la_write_conf_rename_file(UCLA_LA_DEV_WAIT_LIST_FILE, UCLA_LA_DEV_WAIT_LIST_FILE_BACK);
		la_write_conf_2_file(save_buf, save_len, UCLA_LA_DEV_WAIT_LIST_FILE);
	} else {
		la_file_clean(UCLA_LA_DEV_WAIT_LIST_FILE);
		la_file_clean(UCLA_LA_DEV_WAIT_LIST_FILE_BACK);
	}

	SAFE_FREE(save_buf);
}

void g_rule_list_free(la_g_rule_t *pr)
{
	la_rule_t *pru, *prun;

	if (!pr) {
		return;
	}
	
	stlc_list_for_each_entry_safe(la_rule_t, pru, prun, &pr->rule_list, link) {
		stlc_list_del(&pru->link);
		ucla_rule_free(pru);
	}
}

void g_rule_free(la_g_rule_t *pr)
{
	if (pr) {
		g_rule_list_free(pr);
		SAFE_FREE(pr);
	}
}

static void la_g_rule_free()
{
	la_g_rule_t *pr, *prn;

	stlc_list_for_each_entry_safe(la_g_rule_t, pr, prn, &plc->g_rule_list, link) {
		stlc_list_del(&pr->link);
		g_rule_free(pr);
	}
}

static void log_rule_node_free(la_log_home_rule_node_t *pn)
{
	if (!pn) {
		return;
	}
	
	SAFE_FREE(pn->log_info.pmonitor_sn);
	SAFE_FREE(pn->log_info.paction_sn);
	cl_free(pn);
}

void la_g_rule_node_list_free(la_log_home_rule_change_t *pr)
{
	la_log_home_rule_node_t *pn, *pnn;
	
	if (!pr) {
		return;
	}

	stlc_list_for_each_entry_safe(la_log_home_rule_node_t, 
		pn, pnn, &pr->log_list, link) {
		stlc_list_del(&pn->link);
		log_rule_node_free(pn);
	}
}

static void la_g_rule_log_free()
{
	la_log_home_rule_change_t *pr, *prn;

	stlc_list_for_each_entry_safe(la_log_home_rule_change_t, 
		pr, prn, &plc->g_rule_log_list, link) {
		stlc_list_del(&pr->link);
		la_g_rule_node_list_free(pr);
		cl_free(pr);
	}
}

void la_g_member_node_list_free(la_log_home_member_change_t *pm)
{
	la_log_home_member_node_t *pn, *pnn;
	
	if (!pm) {
		return;
	}

	stlc_list_for_each_entry_safe(la_log_home_member_node_t, 
		pn, pnn, &pm->log_list, link) {
		stlc_list_del(&pn->link);
		cl_free(pn);
	}
}

static void la_g_member_log_free()
{
	la_log_home_member_change_t *pm, *pmn;

	stlc_list_for_each_entry_safe(la_log_home_member_change_t, 
		pm, pmn, &plc->g_member_log_list, link) {
		stlc_list_del(&pm->link);
		la_g_member_node_list_free(pm);
		cl_free(pm);
	}
}

static void la_g_log_free()
{
	la_g_rule_log_free();
	la_g_member_log_free();
}

la_g_rule_t *ucla_g_rule_find_by_homeid(u_int32_t home_id)
{
	la_g_rule_t *pr, *prn;

	stlc_list_for_each_entry_safe(la_g_rule_t, pr, prn, &plc->g_rule_list, link) {
		if (pr->home_id == home_id) {
			return pr;
		}
	}

	return NULL;
}

la_g_rule_t *ucla_g_rule_new()
{
	la_g_rule_t *pr = NULL;

	pr = cl_calloc(sizeof(*pr), 1);
	if (pr) {
		STLC_INIT_LIST_HEAD(&pr->link);
		STLC_INIT_LIST_HEAD(&pr->rule_list);
	}

	return pr;
}

void la_back_passwd_proc()
{
	int count = 0;
	user_t *puser, *n;
	FILE *fp = NULL;
	u_int8_t *buff = NULL;
	u_int32_t save_len = 0;
	char fn[258];
	ucla_back_passwd_t *pbp = NULL;

	if (!cl_priv->priv_dir) {
		return;
	}
	stlc_list_count(count, &cl_priv->user);
	if (count == 0) {
		goto done;
	}
	buff = cl_calloc((count * sizeof(*pbp)), 1);
	if (!buff) {
		goto done;
	}
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_BACK_PASSWD_FILE);
	fp = fopen(fn, "wb");
	if (!fp) {
		goto done;
	}
	pbp = (ucla_back_passwd_t*)buff;
	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, puser, n, &cl_priv->user, link) {
		if (mem_is_all_zero(puser->back_passwd_md5, sizeof(puser->back_passwd_md5))) {
			continue;
		}
		pbp->sn = puser->sn;
		memcpy(pbp->passwd, puser->back_passwd_md5, sizeof(pbp->passwd));
		save_len += sizeof(*pbp);
		pbp++;
	}
	cl_unlock(&cl_priv->mutex);

	if (save_len) {
		fwrite(buff, save_len, 1, fp);
		fflush(fp);
	}

done:
	if (fp) {
		fclose(fp);
	}
	SAFE_FREE(buff);
}

static bool la_is_home_passwd(user_t *puser)
{
	la_home_t *phome = NULL;

	if (puser->home_id == 0) {
		return false;
	}
	phome = ucla_find_home_by_id(puser->home_id);
	if (!phome) {
		return false;
	}
	if (memcmp(phome->conf.home_passwd, puser->passwd_md5, 16) == 0) {
		return true;
	}

	return false;
}

void do_la_back_passwd(user_t *puser)
{
	if (!la_is_valid()) {
		return;
	}
	if (!puser) {
		return;
	}
	//puser->auth_err_num > 0表示用备份密码登录的，不需要再保存了。
	if (puser->auth_err_num > 0) {
		return;
	}
	//如果是圈子密码，就不需要保存了
	if (la_is_home_passwd(puser)) {
		return;
	}
	if (memcmp(puser->back_passwd_md5, puser->passwd_md5, sizeof(puser->back_passwd_md5)) == 0) {
		return;
	}
	memcpy(puser->back_passwd_md5, puser->passwd_md5, sizeof(puser->back_passwd_md5));
	plc->need_back_passwd_save = true;
	la_comm_timer_proc_reset();
}

int la_get_file_size2(char *fn)
{
	struct stat st_l;
	
	if (stat(fn, &st_l) != 0) {
		return 0;
	}

	return (int)(st_l.st_size);
}

void la_back_passwd_init()
{
	int len = 0;
	user_t *puser;
	FILE *fp = NULL;
	char fn[256+8];
	u_int8_t *buff = NULL;
	int index = 0;
	u_int32_t read_len = 0;
	ucla_back_passwd_t *pbp = NULL;

	if (!cl_priv->priv_dir) {
		return;
	}
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_BACK_PASSWD_FILE);
	len = la_get_file_size2(fn);
	if (0 == len) {
		return;
	}
	buff = cl_calloc(len, 1);
	if (!buff) {
		return;
	}
	
	fp = fopen(fn, "rb");
	if (!fp) {
		goto done;
	}
	pbp = (ucla_back_passwd_t*)buff;
	read_len = (u_int32_t)fread(buff, 1, (size_t)len, fp);
	for(index = 0; (index + sizeof(*pbp)) <= read_len; index += sizeof(*pbp), pbp++) {
		puser = user_lookup_by_sn(pbp->sn);
		if (puser) {
			memcpy(puser->back_passwd_md5, pbp->passwd, sizeof(puser->back_passwd_md5));
		}
	}

done:
	if (fp) {
		fclose(fp);
	}
	SAFE_FREE(buff);
}

static void la_dev_ver_limit_init()
{
	FILE *fp;
	u_int8_t buf[1024];
	u_int32_t a,b,c,v1,v2,v3;
	char name[64];
	char vendor[64];
	char vertion[10];
	char fn[UCLA_FILE_PATH_LEN];
	la_dev_ver_limit_t *pdvl = NULL;
	int num = 0;

	UCLA_ENTER();
	if (!cl_priv->priv_dir) {
        return;
    }

	//获取限制文件
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DEV_VER_LIMIT_FILE);
	fp = fopen(fn, "r");
	if (!fp) {
		return;
	}	

	//先获取上次版本
	if (!fgets((char *)buf, sizeof(buf), fp)) {
		goto end;
	}
	if (sscanf((char *)buf, "ver:%u.%u.%u", &a, &b, &c) != 3) {
		log_debug("get last ver failed buf=%s\n", buf);
		goto end;
	}
	plc->last_ver[0] = a;
	plc->last_ver[1] = b;
	plc->last_ver[2] = c;

	log_debug("get dev limit ver=%u.%u.%u\n", a, b, c);
	// TODO:根据版本处理
	while(fgets((char *)buf, sizeof(buf), fp)) {
		if (sscanf((char *)buf, "%[^,],%[^,],%u,%u,%u,%[^,]", name, vendor, &a, &b, &c, vertion) != 6) {
			log_debug("scanf failed buf=%s\n", buf);
			goto end;
		}

		if (sscanf(vertion, "%u.%u.%u", &v1, &v2, &v3) != 3) {
			log_debug("scanf failed version=%s\n", vertion);
			goto end;
		}

		//log_debug("get name=%s vendor=%s type=%u subtype=%u exttype=%u version=%u.%u.%u\n", 
			//name, vendor, a, b, c, v1, v2, v3);

		pdvl = cl_calloc(sizeof(la_dev_ver_limit_t), 1);
		if (!pdvl) {
			goto end;
		}
		STLC_INIT_LIST_HEAD(&pdvl->link);
		stlc_list_add(&pdvl->link, &plc->dev_ver_limit_list);
		strcpy(pdvl->name, name);
		strcpy(pdvl->vendor, vendor);
		pdvl->type = a;
		pdvl->sub_type = b;
		pdvl->ext_type =c;
		pdvl->ver = VERTION(v1, v2, v3);
		num++;
	}

	log_debug("get %u items\n", num);

end:
	if (fp) {
		fclose(fp);
	}
}

void la_dev_ver_limit_free()
{
	la_dev_ver_limit_t *d, *n;

	stlc_list_for_each_entry_safe(la_dev_ver_limit_t, d, n, &plc->dev_ver_limit_list, link) {
		stlc_list_del(&d->link);
		SAFE_FREE(d);
	}
}

static void la_dev_ver_limit_save(ucla_session_t *s)
{
	FILE *fp;
	char fn[UCLA_FILE_PATH_LEN];

	if (!s->pver_limit ||
		s->limit_total_length == 0) {
		return;
	}
	
	if (!cl_priv->priv_dir) {
        return;
    }

	//保存数据
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DEV_VER_LIMIT_FILE);
	fp = fopen(fn, "wb");
	if (!fp) {
		return;
	}

	fwrite(s->pver_limit, s->limit_total_length, 1, fp);
	fflush(fp);
	fclose(fp);
}

void do_dev_ver_file_sync(ucla_session_t *s)
{
	la_dev_ver_limit_save(s);
	la_dev_ver_limit_free();
	la_dev_ver_limit_init();
	//获取到版本限制文件后处理一下设备。。。
	dev_ver_reset_cal(NULL);
}

static void la_phone_upgrade_save()
{
	ucla_session_t *s;
	
	if (!plc->need_phone_login_delay) {
		return;
	}

	s = ucla_get_ses_by_doname(UCLA_DEF_DONAME);
	if (s) {
		do_phone_login_session_replace(s);
	}
}

void la_dev_type_save()
{
	FILE *fp;
	char buf[1024];
	char fn[UCLA_FILE_PATH_LEN];

	user_t *puser, *n;

	if (!cl_priv->priv_dir) {
        return;
    }
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DEV_TYPE_FILE);
	fp = fopen(fn, "w");
	if (!fp) {
		return;
	}

	cl_lock(&cl_priv->mutex);
	sprintf(buf, "#sn,type,subtype,exttype,devveristoolow,devvervalid\n");
	fputs(buf, fp);
	stlc_list_for_each_entry_safe(user_t, puser, n, &cl_priv->user, link) {
		sprintf(buf, "%"PRIu64",%u,%u,%u,%u,%u\n", 
			puser->sn, puser->ds_type, puser->sub_type, 
			puser->ext_type, puser->dev_ver_is_too_low, 
			puser->dev_ver_is_valid);
		fputs(buf, fp);
	}
	
	cl_unlock(&cl_priv->mutex);
	fflush(fp);
	fclose(fp);	
}

la_dev_misc_info_t *la_dev_type_find_by_sn(u_int64_t sn)
{
	la_dev_misc_info_t *pdmi, *n;

	stlc_list_for_each_entry_safe(la_dev_misc_info_t, pdmi, n, &plc->dev_type_list, link) {
		if (pdmi->conf.sn == sn) {
			return pdmi;
		}
	}

	return NULL;
}

void dev_type_sync(user_t *user)
{
	la_dev_misc_info_t *pdmi;

	if (!la_is_valid()) {
		return;
	}
	pdmi = la_dev_type_find_by_sn(user->sn);
	if (pdmi) {
		if ((pdmi->conf.dev_ver_is_valid != user->dev_ver_is_valid) ||
			(pdmi->conf.dev_ver_is_too_low != user->dev_ver_is_too_low)) {
			pdmi->conf.dev_ver_is_valid = user->dev_ver_is_valid;
			pdmi->conf.dev_ver_is_too_low = user->dev_ver_is_too_low;
			la_dev_type_save();
		}
		return;
	}
	pdmi = cl_calloc(sizeof(*pdmi), 1);
	if (!pdmi) {
		return;
	}
	STLC_INIT_LIST_HEAD(&pdmi->link);
	stlc_list_add_tail(&pdmi->link, &plc->dev_type_list);
	pdmi->conf.sn = user->sn;
	pdmi->conf.ds_type = user->ds_type;
	pdmi->conf.sub_type = user->sub_type;
	pdmi->conf.ext_type = user->ext_type;
	pdmi->conf.dev_ver_is_too_low = user->dev_ver_is_too_low;
	pdmi->conf.dev_ver_is_valid = user->dev_ver_is_valid;
	la_dev_type_save();
}

void la_doname_sync(user_t* puser)
{
	// TODO:因为服务器做了数据同步，还是先不同步了吧，容易把国外服务器设备拉倒国内来
	return;
}

void la_dev_type_init()
{
	FILE *fp;
	char buf[1024];
	char fn[UCLA_FILE_PATH_LEN];

	u_int64_t sn;
	u_int32_t ds_type;
	u_int32_t sub_type;
	u_int32_t ext_type;
	u_int32_t toolow=0;
	u_int32_t valid = 0;

	la_dev_misc_info_t *pdmi;

	if (!cl_priv->priv_dir) {
        return;
    }
	sprintf(fn, "%s/%s", cl_priv->priv_dir, UCLA_DEV_TYPE_FILE);
	fp = fopen(fn, "r");
	if (!fp) {
		return;
	}

	while(fgets(buf, sizeof(buf), fp)) {
		if (buf[0] == '#') {
			continue;
		}
		if (sscanf(buf, "%"PRIu64",%u,%u,%u,%u,%u", &sn, &ds_type, &sub_type, &ext_type, &toolow, &valid) != 6) {
			log_debug("err sscanf buf=%s\n", buf);
			goto end;
		}

		pdmi = cl_calloc(sizeof(*pdmi), 1);
		if (!pdmi) {
			goto end;
		}

		STLC_INIT_LIST_HEAD(&pdmi->link);
		stlc_list_add_tail(&pdmi->link, &plc->dev_type_list);
		pdmi->conf.sn = sn;
		pdmi->conf.ds_type = ds_type;
		pdmi->conf.sub_type = sub_type;
		pdmi->conf.ext_type = ext_type;
		pdmi->conf.dev_ver_is_too_low = toolow;
		pdmi->conf.dev_ver_is_valid = valid;
		//log_debug("sn=%"PRIu64" t=%u s=%u e=%u tl=%u iv=%u\n", 
			//sn, ds_type, sub_type, ext_type, toolow, valid);
	}

end:
	if (fp) {
		fclose(fp);
	}
}

void la_dev_type_free()
{
	la_dev_misc_info_t *pdmi, *n;

	stlc_list_for_each_entry_safe(la_dev_misc_info_t, pdmi, n, &plc->dev_type_list, link) {
		stlc_list_del(&pdmi->link);
		SAFE_FREE(pdmi);
	}
}

void la_misc_capfile_free()
{
	int i;

	for(i = 0; i < plc->cap_num; i++) {
		SAFE_FREE(plc->cap_array[i]);
	}
	SAFE_FREE(plc->cap_array);
	plc->cap_num = 0;		
}

void la_misc_custom_capfile_free()
{
	int i;

	for(i = 0; i < plc->cap_custom_num; i++) {
		SAFE_FREE(plc->cap_custom_array[i]);
	}
	SAFE_FREE(plc->cap_custom_array);
	plc->cap_custom_num = 0;	
}

void la_misc_template_free()
{
	int i;
	
	for(i = 0; i < plc->url_num; i++) {
		SAFE_FREE(plc->url_array[i]);
	}
	SAFE_FREE(plc->url_array);
	plc->url_num = 0;	
}

void la_misc_free()
{
	la_misc_capfile_free();
	la_misc_custom_capfile_free();
	la_misc_template_free();
}

static void la_doname_init()
{
	STLC_INIT_LIST_HEAD(&plc->valid_doname);
	STLC_INIT_LIST_HEAD(&plc->invalid_doname);
}

static void la_doname_free()
{
	la_doname_clean(true);
	la_doname_clean(false);
}

void la_move_home_clean()
{
	if (plc->move_home.num) {
		plc->move_home.num = 0;
		SAFE_FREE(plc->move_home.homeid);
	}
}

static void la_null_home_init()
{
	plc->p_null_home = ucla_home_new();
}

static void la_null_home_clean()
{
	SAFE_FREE(plc->p_null_home);
}

void ucla_weak_user_save()
{
	ucla_session_t *s;
	la_user_t *puser;
	la_home_t *h, *n;
	int home_num = 0;
	int mem_num = 0;
	int num = 0;

	UCLA_ENTER();
	//如果已经是手机用户了，就没必要保存了
	if (plc->has_phone) {
		return;
	}
	if (!(s = ucla_get_dehome_session())) {
		return;
	}
	if (!(puser = ucla_find_user_by_id(s->user_id))) {
		return;
	}
	stlc_list_for_each_entry_safe(la_home_t, h, n, &puser->home_link, link) {
		home_num++;
		stlc_list_count(num, &h->member_link);
		mem_num += num;
	}
	//没设备的圈子没必要添加
	if ((home_num == 0) ||
		(mem_num == 0)) {
		return;
	}
	if (!(plc->move_home.homeid = cl_calloc(sizeof(u_int32_t)*home_num, 1))) {
		return;
	}
	num = 0;
	stlc_list_for_each_entry_safe(la_home_t, h, n, &puser->home_link, link) {
		plc->move_home.homeid[num++] = h->conf.home_id;
	}
	plc->move_home.num = home_num;
	plc->move_home.userid = s->user_id;
	UCLA_EXIT();
}

la_member_t *la_dev_pass_sync_find_by_sn(u_int64_t sn)
{
	la_member_t *m, *mn;

	stlc_list_for_each_entry_safe(la_member_t, m, mn, &plc->dev_pass_sync_list, link) {
		if (sn == m->conf.sn) {
			return m;
		}
	}	

	return NULL;
}

static void la_dev_passwd_sync_list_init()
{
	la_member_conf_t *pmembercf;
	la_member_t *pmember;

	int i;
	int read_len = 0;
	int index = 0;
	u_int8_t *read_buf= NULL;
	ucla_conf_save_t *pconf = NULL;

	read_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!read_buf) {
		return;
	}
	pconf = (ucla_conf_save_t *)read_buf;

	read_len = la_conf_read_from_file(read_buf, UCLA_CONF_SAVE_MAX_LEN, UCLA_DEV_PASS_SYNC_FILE);
	if (read_len == 0) {
		read_len = la_conf_read_from_file(read_buf, UCLA_CONF_SAVE_MAX_LEN, UCLA_DEV_PASS_SYNC_FILE_BACK);
	}
	read_len = la_conf_dec(read_buf, read_len);

	if ((read_len == 0) ||
		(read_len <= sizeof(*pconf))) {
		goto done;
	}
	read_len -= sizeof(*pconf);
	for(i = 0; (i < (int)pconf->count) && 
		((index + (int)sizeof(*pmembercf)) <= read_len); i++) {
		pmembercf = (la_member_conf_t *)&pconf->data[index];
		index += sizeof(*pmembercf);
		
		pmember = ucla_member_new();
		if (!pmember) {
			goto done;
		}
		memcpy((void *)&pmember->conf, (void *)pmembercf, sizeof(*pmembercf));
		stlc_list_add_tail(&pmember->link, &plc->dev_pass_sync_list);
		log_debug("la_dev_passwd_sync_list_init sn=%"PRIu64"\n", pmember->conf.sn);
	}

done:
	SAFE_FREE(read_buf);
}

static void la_dev_passwd_sync_list_exit()
{
	la_member_t *m, *mn;

	stlc_list_for_each_entry_safe(la_member_t, m, mn, &plc->dev_pass_sync_list, link) {
		stlc_list_del(&m->link);
		cl_free(m);
	}
}

static void la_dev_pass_sync_list_save()
{
	la_member_t *pmem, *memn;
	u_int8_t *save_buf = NULL;
	int save_len = 0;
	int save_count = 0;
	ucla_conf_save_t *pconf = NULL;

	if (stlc_list_empty(&plc->dev_pass_sync_list)) {
		la_file_clean(UCLA_DEV_PASS_SYNC_FILE);
		la_file_clean(UCLA_DEV_PASS_SYNC_FILE_BACK);
		return;
	}
	
	save_buf = cl_calloc(UCLA_CONF_SAVE_MAX_LEN, 1);
	if (!save_buf) {
		log_debug("calloc failed\n");
		return;
	}
	pconf = (ucla_conf_save_t *)save_buf;

	stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &plc->dev_pass_sync_list, link) {
		memcpy((void *)&pconf->data[save_len], (void *)&pmem->conf, sizeof(pmem->conf));
		save_len += sizeof(pmem->conf);
		save_count++;
	}
	
	save_len = la_conf_fill_enc(pconf, save_len, save_count);
	if (save_len) {
		la_write_conf_rename_file(UCLA_DEV_PASS_SYNC_FILE, UCLA_DEV_PASS_SYNC_FILE_BACK);
		la_write_conf_2_file(save_buf, save_len, UCLA_DEV_PASS_SYNC_FILE);
	} else {
		la_file_clean(UCLA_DEV_PASS_SYNC_FILE);
		la_file_clean(UCLA_DEV_PASS_SYNC_FILE_BACK);
	}

	SAFE_FREE(save_buf);	
}

static void la_timer_off()
{
	CL_THREAD_OFF(plc->t_timer);
	CL_THREAD_OFF(plc->t_comm);
	CL_THREAD_OFF(plc->t_def_home_timer);
	CL_THREAD_OFF(plc->t_modify_passwd);
	CL_THREAD_OFF(plc->t_comm_proc);
}

void linkage_init()
{
	memset((void*)&la_ctrl, 0, sizeof(la_ctrl));

	UCLA_ENTER();
	if (!app_is_la()) {
		log_debug("this app is not support linkage\n");
		return;
	}	
	nd_la_debug(NULL, "enter linkage_init\n");
	if (cl_priv->priv_dir) {
		MKDIR(cl_priv->priv_dir, 0777);	
	}
	plc->init = true;
	plc->lang = LANG_CH;
	plc->init_time = get_sec();

	//doname list init
	la_doname_init();
	//保存aes加密初始化
	la_conf_aes_init();
	//域名占位映射
	la_dmap_init();
	//手机账号配置初始化
	STLC_INIT_LIST_HEAD(&plc->la_phone);
	la_phone_init();
	//设备版本限制文件初始化
	STLC_INIT_LIST_HEAD(&plc->dev_ver_limit_list);
	la_dev_ver_limit_init();
	//设备类型初始化
	STLC_INIT_LIST_HEAD(&plc->dev_type_list);
	la_dev_type_init();
	//全局规则链表初始化，给s3专门提供
	STLC_INIT_LIST_HEAD(&plc->g_rule_list);
	STLC_INIT_LIST_HEAD(&plc->g_rule_log_list);
	STLC_INIT_LIST_HEAD(&plc->g_member_log_list);
	//私钥获取
	//plc->rsa_private = a2i_RSAPrivateKey((u_int8_t *)la_private_key, (int)strlen(la_private_key));
	la_rsa_init();
	//空圈子初始化
	la_null_home_init();
	//中转设备ip初始化
	trans_init();
	plc->disp_bit |= BIT(1);
	STLC_INIT_LIST_HEAD(&plc->server_client);
	STLC_INIT_LIST_HEAD(&cl_priv->la_user);
	STLC_INIT_LIST_HEAD(&plc->dev_wait_list);
	//初始化密码同步链表
	STLC_INIT_LIST_HEAD(&plc->dev_pass_sync_list);
	//因为这时handle还没取回，所以要用延时定时器去开启各状态机
	CL_THREAD_TIMER_ON(MASTER, plc->t_timer, la_delay_timer, NULL, TIME_N_SECOND(0));
	//通用定时器，主要是来处理一些延后操作，如果设备登陆后，在定时器里判断登陆成功及是否可以添加设备到圈子里
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm, la_comm_timer, NULL, TIME_N_SECOND(2));
	//用来处理一下延时操作，如session还没在线时置上标志，等session在线后再处理
	CL_THREAD_TIMER_ON(MASTER, plc->t_comm_proc, la_comm_timer_proc, NULL, TIME_N_SECOND(10));
	//初始化手机账号状态机
	plc->phone_status = UCLA_PHONE_IDLE;
	ucla_phone_set_status(UCLA_PHONE_IDLE);
}

void linkage_exit()
{
	la_user_t *puser, *usern;
	ucla_session_t *s , *n;

	if (!app_is_la()) {
		return;
	}

	if (!plc->init) {
		return;
	}
	//timer释放
	la_timer_off();
	//free msg
	SAFE_FREE(plc->msg.cn_msg);
	SAFE_FREE(plc->msg.en_msg);
	SAFE_FREE(plc->req_share);
	//设备版本限制文件释放
	la_dev_ver_limit_free();
	//设备类型配置释放
	la_dev_type_free();
	//释放手机用户
	la_phone_exit();
	//释放一些模板，设备能力文件url
	la_misc_free();
	//rsa free
	la_rsa_exit();
	//是否域名链表
	la_doname_free();
	//迁移圈子列表释放
	la_move_home_clean();
	//空圈子释放
	la_null_home_clean();
	//释放密码同步链表
	la_dev_passwd_sync_list_exit();
	//用户释放
	stlc_list_for_each_entry_safe(la_user_t, puser, usern, &cl_priv->la_user, link) {
		stlc_list_del(&puser->link);
		ucla_user_free(puser);
	}
	//session释放
	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
		stlc_list_del(&s->link);
		ucla_session_free(s);
	}
	//待添加设备链表释放
	la_dev_wait_list_free();
	//全局规则链表初始化，给s3专门提供,需要释放一下
	la_g_rule_free();
	//日志相关
	la_g_log_free();
	//也许需要还原一下
	ucla_maybe_recovery();
	//退出直接保存下设备类型缓存
	la_dev_type_save();
	//这里要处理一种特殊情况，非联动app升级到联动app登陆手机账号时，可能网络不通，一直没找到最快服务器，不知道保存手机账号到哪个配置下
	la_phone_upgrade_save();
#if 0
	if (plc->rsa_private) {
		RSA_free(plc->rsa_private);
	}
#endif	
	nd_la_debug(NULL, "linkage_exit\n");
}

void ucla_set_callback(cl_callback_t callback, void *callback_handle)
{
	log_debug("ucla_set_callback callback=%08x callback_handle=%08x \n", callback, callback_handle);
	plc->callback = callback;
	plc->callback_handle = callback_handle;
}

bool la_is_valid()
{
	return plc->init;
}

static void ucla_clean_disp_ext()
{
	plc->disp_bit = 0;
}

//启动多久还没连接上服务器的话就不要限制设备登录了
#define LINKSERVER_MAX_TIME		(5*60)
static void ucla_enable_disp_ext(u_int8_t index, bool set)
{
	u_int32_t now = get_sec();

	if ((plc->init_time + LINKSERVER_MAX_TIME) < now) {
		plc->disp_bit = 0;
		return;
	}
	
	if (set) {
		plc->disp_bit |= BIT((index%32));
	} else {
		plc->disp_bit &= ~(BIT((index%32)));	
	}
}

bool can_send_disp()
{
	if (!app_is_la() || 
		!la_is_valid()) {
		return true;
	}

	//log_debug("plc->disp_bit=%08x\n", plc->disp_bit);

	return (plc->disp_bit == 0);
}

