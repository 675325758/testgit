/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 联动逻辑功能c文件
**  File:    linkage_priv.c
**  Author:  liubenlong
**  Date:    11/30/2015
**
**  Purpose:
**    联动逻辑功能c文件.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_priv.h"
#include "md5.h"
#include "cl_notify.h"
#include "cl_thread.h"
#include "cl_user.h"
#include "net_detect.h"
#include "cmd_misc.h"
#include "user_priv.h"
#include "video_priv.h"
#include "h264_decode.h"
#include "wait_server.h"
#include "cl_community.h"
#include "community_priv.h"
#include "equipment_priv.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "http.h"
#include "phone_user.h"
#include "lan_dev_probe_priv.h"
#include "bindphone_priv.h"
#include "cmd_misc.h"
#include "env_mon_priv.h"
#include "cl_env_mon.h"
#include "ia_priv.h"
#include "health_priv.h"
#include "notify_push_priv.h"
#include "uc_client.h"
#include "smart_appliance_priv.h"
#include "lbs.h"
#include "cl_linkage.h"
#include "linkage_client.h"
#include "linkage_priv.h"
#include <stdlib.h>


/* Macro constant definitions. */


/* Type definitions. */
enum {
	HOME_EVENT_DEV_CHANGED = 0,
	HOME_EVENT_HOME_DELED = 1,
	HOME_EVENT_PASSWD_CHANGED = 2,
	HOME_EVENT_MEMBER_CHANGED = 3,
	HOME_EVENT_HOME_NAME_CHANGED = 4,
	HOME_EVENT_HOME_ADDED = 5,
	HOME_EVENT_LABEL_CHANGED = 6,
	HOME_EVENT_LABEL_DEL = 7,
	//HOME_EVENT_SC_CHANGE = 8,
	//HOME_EVENT_SC_DEL = 9,
	HOME_EVENT_DICT_CHANGE = 8,
	HOME_EVENT_DICT_DEL = 9,
	HOME_EVENT_LOG_CHANGE = 10,
};

enum {
	SHARE_EVENT_MEMBER_CHANGED = 0,
};

enum {
	LINKAGE_EVENT_RULE_QUERY_ALL = 0,//?查询所有规则。
	LINKAGE_EVENT_RULE_MODIFY = 1,//添加或修改了home_id下面的ruleid规则,在规则触发时也会发送该event。
	LINKAGE_EVENT_RULE_DELED = 2,//规则被删除
	LINKAGE_EVENT_ENABLE_CHANGED = 3,//?规则enable被修改。Value=enable。
	LINKAGE_EVENT_STATE_CHANGED = 4,//?规则state被修改。Value=state。执行状态
	LINKAGE_EVENT_LAST_EXE_TIMECHANGED = 5,//规则执行状态改变。头部后跟4个字节的时间。
};


typedef struct {
	u_int8_t trans_ip_num;
	u_int32_t trans_ip[MAX_TRANS_IP_NUM];
}la_trans_s_t;

/* Local function declarations. */

//保存的中转设备信息
#define 	TRANS_DEV_IP_INFO_FILE		"trans_info.txt"
/* Macro API definitions. */
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

/* Global variable declarations. */
bool _la_ctrl_proc_notify(u_int8_t action, u_int8_t *data, u_int32_t data_len ,RS *ret);
void do_ucla_share_query(ucla_session_t *s, u_int32_t home_id);
static void ucla_home_all_mem_logout(u_int32_t home_id);
static void ucla_home_mem_logout(u_int32_t home_id, u_int64_t sn);
extern user_t *user_modify(cl_handle_t handle, char *passwd, cl_callback_t callback, void *callback_handle, bool authing, bool is_la);
static void _ucla_home_event_need_query(ucla_session_t *s, u_int32_t home_id);
extern bool mem_is_all_zero(u_int8_t *data, int len);
static void _ucla_link_conf_event_need_query(ucla_session_t *s, u_int32_t home_id);
bool _la_phone_ctrl_proc_notify(u_int8_t action, u_int8_t *data, u_int32_t data_len, RS *ret);
la_member_t *ucla_find_member_by_sn_from_home(la_home_t *phome, u_int64_t sn);
bool _la_comm_proc_notify(u_int8_t action, u_int8_t *data, u_int32_t data_len, RS *ret);
extern void ucla_each_share_query_sync(ucla_session_t *s);
extern void la_phone_save();
extern void _ucla_user_conf_save(ucla_session_t *s);
static void ucla_user_del_only(u_int64_t sn);
extern int char2hex(char *in, u_int8_t *out, int *outlen);
void ucla_event_push_err(int event, u_int32_t handle, int err);

static char lacmap[] = "7ijg0tBCzhsxR6HbpqaIdFLEyfTm1knWVcJv";

//二维码添加设备相关函数

static void la_json_enc(char *src, char *dst)
{
    char c;
    
    // skip   "v":"1",
    while (*src && *src != ',')
        *dst++ = *src++;
    *dst++ = *src;
    if (*src == '\0')
        return;
    src++;
    
    while ((c = *src)) {
        if ('0' <= c && c <= '9') {
            *dst = lacmap[c - '0'];
        } else if ('a' <= c && c <= 'z') {
            *dst = lacmap[c - 'a' + 10];
        } else {
            *dst = c;
        }
        
        src++;
        dst++;
    }
}

void la_json_dec(char *src, char *dst)
{
    int i;
    char c;
    
    // skip   "id":"1123",
    while (*src && *src != ',')
        *dst++ = *src++;
    *dst++ = *src;
    if (*src == '\0')
        return;
    src++;
    
    while ((c = *src)) {
        for (i = 0; lacmap[i] && lacmap[i] != c; i++) {
        }
        if (lacmap[i]) {
            if (i < 10) {
                *dst = '0' + i;
            } else {
                *dst = 'a' + i - 10;
            }
        } else {
            *dst = c;
        }
        
        src++;
        dst++;
    }
}

void la_u64_2_char(u_int8_t *pshare, u_int64_t *pkey)
{
	int i;

	for(i = 0; i < 8; i++) {
		pshare[i] = (u_int8_t)((*pkey>>(i*8))&0xff);
	}
}

void la_share_2_u64(u_int8_t *pshare, u_int64_t *pkey)
{
	int i;
	u_int64_t value = 0;
	u_int64_t tmp = 0;

	for(i = 0; i < 8; i++) {
		tmp = (u_int64_t)pshare[i];
		value += tmp <<(i*8);
	}

	*pkey = value;
}

bool _la_proc_notify(u_int8_t *data, RS *ret)
{
	cln_la_info_t *pinfo = (cln_la_info_t *)data;

	if (!la_is_valid()) {
		log_debug("_la_proc_notify la_is_unvalid\n");
		return true;
	}

	switch(pinfo->type) {
	case CLNE_LA_CTRL:
		_la_ctrl_proc_notify(pinfo->action, pinfo->data, pinfo->data_len, ret);
		break;
	case CLNE_LA_PHONE_CTRL:
		_la_phone_ctrl_proc_notify(pinfo->action, pinfo->data, pinfo->data_len, ret);
		break;
	case CLNE_LA_COMM_CTRL:
		_la_comm_proc_notify(pinfo->action, pinfo->data, pinfo->data_len, ret);
		break;
	default:
		break;
	}
	
	return true;
}

static bool _la_ctrl_home_create(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	char buf[64];
	pkt_t *pkt;
	la_home_t *phome;
	ucla_session_t *s;
	uc_home_conf_create_t *phomecf;
	la_home_create_notify_t *home_create = (la_home_create_notify_t *)data;
	cl_handle_t *phandle = home_create->handle;

	data += sizeof(*home_create);
	data_len -= sizeof(*home_create);
	data[data_len] = 0;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	//判断下默认圈子存在不，否则失败
	if (!s) {
		ucla_event_push(LA_HOME_CREATE_FAILED);
		*ret = RS_OFFLINE;
		return false;
	}

	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_home_conf_create_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	phome = ucla_home_new();
	if (!phome) {
		pkt_free(pkt);
		return false;
	}

	*phandle = phome->handle;
	
	phome->conf.home_id = 0xffffffff;
	ucla_home_add(s, phome);

	phomecf = get_uascp_payload(pkt, uc_home_conf_create_t);
	phomecf->action = UCAU_CREATE;
	memset(buf, 0, sizeof(buf));
	strncpy(buf, home_create->name, 63);
	memcpy((void *)&phomecf->home_name, (void *)buf, sizeof(phomecf->home_name));
	memcpy((void *)s->home_name, (void *)buf, sizeof(phomecf->home_name));
	memcpy((void *)phome->conf.home_name, (void *)buf, sizeof(phome->conf.home_name));

	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_home_del(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	pkt_t *pkt;
	uc_home_conf_delete_t *phomedel;
	u_int32_t *pid = (u_int32_t *)data;
	la_home_t *phome;
	ucla_session_t *s;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push(LA_HOME_DEL_FAILED);
		*ret = RS_OFFLINE;
		return false;
	}
	phome = ucla_find_home_by_id(*pid);
	if (!phome || ucla_is_def_home(phome)) {
		log_debug("%u is %s and can not del !!!!!!!!!!!!!!!!\n", 
			*pid, phome?"not found":"def home");
		*ret = RS_HOME_DEL_FAILED;
		return false;
	}
	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_home_conf_delete_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	
	phomedel = get_uascp_payload(pkt, uc_home_conf_delete_t);
	phomedel->aciton = UCAU_DELETE;
	phomedel->home_id = htonl(*pid);
	s->home_del_id = *pid;
	
	ucla_request_add(s, pkt);

	return true;
}

static bool la_template_query()
{
	pkt_t *pkt;
	int data_len = sizeof(uc_link_conf_template_t);
	uc_link_conf_template_t request;
	uc_link_conf_template_t *rt;
	ucla_session_t *s = plc->template_session;
	
	if (!s ||
		!ucla_session_is_ok(s)) {
		log_debug("_la_ctrl_template_query failed\n");
		return false;
	}
	request.aciton = UCAU_TEMPLATE;
	request.lang = plc->lang;
	request.home_id = 0;
	request.app_id = ntohl(cl_priv->app_id);

	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	rt = get_uascp_payload(pkt, uc_link_conf_template_t);
	memcpy(rt, &request, data_len);

	log_debug("_la_ctrl_template_query goto %s for tempquery plc->lang=%u\n", s->doname, plc->lang);
	ucla_request_add(s, pkt);

	return true;
}


static bool _la_ctrl_template_query(u_int8_t *data, u_int32_t data_len)
{
	return la_template_query();
}

static bool _la_ctrl_rulelist_query(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_link_conf_rulelist_t *request = (uc_link_conf_rulelist_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	request->aciton = UCAU_RULELIST;
	request->home_id = ntohl(request->home_id);
	request->rule_id = ntohl(request->rule_id);

	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	rt = get_uascp_payload(pkt, uc_link_conf_rulelist_t);
	memcpy(rt, request, data_len);

	plc->is_rule_all_query = true;
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_rulelist_add(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_config_rule_add_t *request = (uc_linkage_config_rule_add_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	request->action = UCAU_RULEADD;
	request->home_id = ntohl(request->home_id);
	request->rule_id = ntohl(request->rule_id);

	if (data_len < sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	rt = get_uascp_payload(pkt, uc_linkage_config_rule_add_t);
	memcpy(rt, request, data_len);
	SAFE_FREE(s->rule_tmp);
	s->rule_tmp = (u_int8_t *)cl_strdup(request->rule);
	//log_debug("add rule=%s\n", s->rule_tmp);

	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_rulelist_del(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_config_rule_del_t *request = (uc_linkage_config_rule_del_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	request->action = UCAU_RULEREMOVE;
	request->home_id = ntohl(request->home_id);
	request->rule_id = ntohl(request->rule_id);
	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	rt = get_uascp_payload(pkt, uc_linkage_config_rule_del_t);
	memcpy(rt, request, data_len);

	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_rulelist_exec(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_config_rule_exec_t *request = (uc_linkage_config_rule_exec_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	request->action = UCAU_EXEC;
	request->home_id = ntohl(request->home_id);
	request->rule_id = ntohl(request->rule_id);
	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	rt = get_uascp_payload(pkt, uc_linkage_config_rule_exec_t);
	memcpy(rt, request, data_len);
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_event(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_config_event_t *request = (uc_linkage_config_event_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	request->action = UCAU_EVENT;
	request->home_id = ntohl(request->home_id);
	request->rule_id = ntohl(request->rule_id);

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	rt = get_uascp_payload(pkt, uc_linkage_config_event_t);
	memcpy(rt, request, data_len);
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_home_share_create(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_home_share_create_t *request = (uc_linkage_home_share_create_t *)data, *rt;
	pkt_t *pkt;
	la_home_t *phome;
	ucla_session_t *s;

	phome = ucla_find_home_by_id(request->home_id);
	request->action = UCAU_CREATE;
	request->home_id = ntohl(request->home_id);

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push(LA_SHARE_REQ_FAILED);
		*ret = RS_OFFLINE;
		return false;
	}
	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_HOME_SHARE, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	rt = get_uascp_payload(pkt, uc_linkage_home_share_create_t);
	memcpy(rt, request, data_len);
	ucla_request_add(s, pkt);
	if (phome) {
		SAFE_FREE(phome->share);
	}

	return true;
}


static bool _la_ctrl_app_share_create(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt;
	ucla_session_t *s;
	uc_app_user_req_sc_t *request = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push_err(LA_USER_SHARE_REQ_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return false;
	}

	pkt = ucla_pkt_new(CMD_APP_USER, sizeof(*request), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	request = get_uascp_payload(pkt, uc_app_user_req_sc_t);
	request->action = UCAU_USER_REQ_SC;

	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_app_share_register(u_int8_t *data, u_int32_t data_len)
{
	char *ptmp = (char *)data;
	uc_app_user_add_home_t *rt;
	pkt_t *pkt;
	char buf[1024];
	u_int32_t home_id;
	u_int32_t user_id;
	u_int8_t share_code[8];
	u_int64_t sc;
	//char doname[100];
	char *desc;
	char *psc;
	ucla_session_t *s;

	rt = NULL;
	pkt = NULL;

	home_id = *(u_int32_t *)ptmp;
	ptmp += sizeof(u_int32_t);

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push_err(LA_USER_SHARE_ADD_HOME_FILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return false;
	}
	//homeid检查，如果没有就不必添加了
	if (!ucla_find_home_by_id(home_id)) {
		log_info("error homeid=%u\n", home_id);
		ucla_event_push_err(LA_USER_SHARE_ADD_HOME_FILED, 0, LA_APP_USER_REQ_SHARE_HOME_NOT_EXIST);
		return false;
	}
	//\"v\":\"1\",\"sc\":\"%"PRIu64"\",\"uid\":\"%u\",\"doname\":\"%s\"
	//share
	strcpy(buf, ptmp);
	la_json_dec(buf, buf);
	log_debug("%s buf=%s\n", __FUNCTION__, buf);
	//user_id
	if (NULL == (psc = strstr(buf, "\"uid\""))) {
		log_debug("error uid=%s\n", buf);
		ucla_event_push_err(LA_USER_SHARE_ADD_HOME_FILED, 0, LA_APP_USER_REQ_SHARE_NO_USERID);
		return false;
	}
	log_debug("uid=%s\n", psc);
	sscanf(psc, "\"uid\":\"%u\"", &user_id);
	if (s->user_id == user_id) {
		ucla_event_push_err(LA_USER_SHARE_ADD_HOME_FILED, 0, LA_APP_USER_REQ_SHARE_USER_ID_ALREAD);
		return false;
	}
	//sc
	if (NULL == (psc = strstr(buf, "\"sc\""))) {
		log_debug("error sc=%s\n", buf);
		ucla_event_push_err(LA_USER_SHARE_ADD_HOME_FILED, 0, LA_APP_USER_REQ_SHARE_PARSE_ERROR);
		return false;
	}
	log_debug("psc=%s\n", psc);
	sscanf(psc, "\"sc\":\"%"PRIu64"\"", &sc);
	la_u64_2_char(share_code, &sc);
	//desc
	ptmp += strlen(buf) + 1;
	desc = ptmp;
	//log_debug("%s doname=%s sc=%"PRIu64" home_id=%u user_id = %u desc=%s\n", __FUNCTION__, doname, sc, home_id, user_id, desc);	

	pkt = ucla_pkt_new(CMD_APP_USER, (int)(sizeof(*rt) + strlen(desc) + 1), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	rt = get_uascp_payload(pkt, uc_app_user_add_home_t);
	rt->action = UCAU_USER_ADD_HOME;
	rt->user_id = htonl(user_id);
	rt->home_id = htonl(home_id);
	memcpy(rt->share, share_code, sizeof(rt->share));
	strcpy(rt->desc, desc);

	ucla_request_add(s, pkt);

	return true;
}

static void _la_label_add(u_int8_t *data, u_int32_t data_len)
{
	int i,n = 0, sn_len = 0;
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
//	la_label_t *plabel = NULL;
	uc_label_add_t *padd = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_LABEL_ADD_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_LABEL, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	padd = get_uascp_payload(pkt, uc_label_add_t);
	memcpy((void *)padd, (void *)data, data_len);

	if (data_len > sizeof(*padd)) {
		sn_len = (int)(data_len - sizeof(*padd));
		n = (sn_len/sizeof(u_int64_t));
	}
	//本地缓存一份
	if (s->plabel_tmp) {
		SAFE_FREE(s->plabel_tmp->p_sn);
		cl_free(s->plabel_tmp);
	}
	s->plabel_tmp = ucla_label_new();
	if (!s->plabel_tmp) {
		log_err(true, "malloc failed\n");
		pkt_free(pkt);
		return;
	}

	s->plabel_tmp->conf.id = padd->label_id;
	s->plabel_tmp->conf.sn_num = n;
	s->plabel_tmp->conf.home_id = padd->home_id;
	memcpy((void *)s->plabel_tmp->conf.name, padd->name, sizeof(padd->name));
	
	if (n) {
		s->plabel_tmp->p_sn = cl_calloc(sn_len, 1);
		if (!s->plabel_tmp->p_sn) {
			log_err(true, "malloc failed sn_len=%u\n", sn_len);
			pkt_free(pkt);
			return;
		}
		memcpy((void *)s->plabel_tmp->p_sn, (void *)padd->a_sn, sn_len);
	}

	padd->action = UCAU_CREATE;
	padd->label_id = htons(padd->label_id);
	padd->home_id = htonl(padd->home_id);
	for(i = 0; i < n; i++) {
		padd->a_sn[i] = ntoh_ll(padd->a_sn[i]);
	}

	ucla_request_add(s, pkt);
}

static void _la_label_del(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
//	la_label_t *plabel = NULL;
	uc_label_delete_t *pld = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_LABEL_DEL_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_LABEL, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	pld = get_uascp_payload(pkt, uc_label_delete_t);
	memcpy((void *)pld, (void *)data, data_len);
	pld->action = UCAU_DELETE;
	pld->label_id = ntohs(pld->label_id);
	pld->home_id = ntohl(pld->home_id);

	ucla_request_add(s, pkt);
}

static void _la_label_bind(u_int8_t *data, u_int32_t data_len)
{
	int i;
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_label_bind_t *plb = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_LABEL_BIND_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_LABEL, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	plb = get_uascp_payload(pkt, uc_label_bind_t);
	memcpy((void *)plb, (void *)data, data_len);

	//先本地缓存一份
	SAFE_FREE(s->plbind_tmp);
	s->plbind_tmp = cl_calloc(data_len, 1);
	if (!s->plbind_tmp) {
		log_err(true, "malloc failed\n");
		return;
	}
	memcpy(s->plbind_tmp, data, data_len);

	plb->action = UCAU_LABEL_BIND;
	plb->home_id = ntohl(plb->home_id);
	plb->sn = ntoh_ll(plb->sn);
	for(i = 0; i < plb->label_num; i++) {
		plb->a_label_id[i] = ntohs(plb->a_label_id[i]);
	}
	plb->label_num = ntohs(plb->label_num);

	ucla_request_add(s, pkt);
}

static void _la_home_shortcut_add(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_home_shortcut_t *ps = (uc_home_shortcut_t *)data;

	UCLA_ENTER();
	if (!plc->support_shortcut) {
		log_info("not support_shortcut \n");
		ucla_event_push_err(LA_SHORTUT_ADD_FAILED, 0, LA_NOT_SUPPORT);
		return;
	}
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_SHORTUT_ADD_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	if ((ps->index == 0) ||
		(ps->index > LA_SC_A_NUM)) {
		log_info("error index=%u\n", ps->index);
		ucla_event_push(LA_SHORTUT_ADD_FAILED);
		return;
	}
	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	ps = get_uascp_payload(pkt, uc_home_shortcut_t);
	memcpy((void *)ps, (void *)data, data_len);

	ps->action = UCAU_HOME_SHORTCUT;
	ps->home_id = ntohl(ps->home_id);
	ps->rule_id = ntohl(ps->rule_id);
	ps->rule_len = ntohl(ps->rule_len);

	SAFE_FREE(s->ps_tmp);
	s->ps_tmp= cl_calloc(data_len, 1);
	if (s->ps_tmp) {
		memcpy((void *)s->ps_tmp, (void *)data, data_len);
	}
	
	ucla_request_add(s, pkt);
}

static void _la_home_shortcut_del(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_home_shortcut_del_t *psd = (uc_home_shortcut_del_t *)data;

	UCLA_ENTER();
	if (!plc->support_shortcut) {
		log_info("not support_shortcut \n");
		ucla_event_push_err(LA_SHORTCUT_DEL_FAILED, 0, LA_NOT_SUPPORT);
		return;
	}
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_SHORTCUT_DEL_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	if ((psd->index == 0) ||
		(psd->index > LA_SC_A_NUM)) {
		log_info("error index=%u\n", psd->index);
		ucla_event_push(LA_SHORTCUT_DEL_FAILED);
		return;
	}
	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	psd = get_uascp_payload(pkt, uc_home_shortcut_del_t);
	memcpy((void *)psd, (void *)data, data_len);

	psd->action = UCAU_HOME_SHORTCUTDEL;
	psd->home_id = ntohl(psd->home_id);

	ucla_request_add(s, pkt);
}

static void _la_home_shortcut_mod(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_home_shortcut_modify_t *psd = (uc_home_shortcut_modify_t *)data;

	UCLA_ENTER();
	if (!plc->support_shortcut) {
		log_info("not support_shortcut \n");
		ucla_event_push_err(LA_SHORTCUT_MOD_FAILED, 0, LA_NOT_SUPPORT);
		return;
	}
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_SHORTCUT_MOD_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	if ((psd->index == 0) ||
		(psd->index > LA_SC_A_NUM)) {
		log_info("error index=%u\n", psd->index);
		ucla_event_push(LA_SHORTCUT_MOD_FAILED);
		return;
	}
	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	psd = get_uascp_payload(pkt, uc_home_shortcut_modify_t);
	memcpy((void *)psd, (void *)data, data_len);

	psd->action = UCAU_HOME_SHORTCUTMODIFY;
	psd->home_id = ntohl(psd->home_id);
	memcpy((void *)&s->sc_mod_tmp, (void *)psd, sizeof(s->sc_mod_tmp));

	ucla_request_add(s, pkt);
}

static void _la_home_rule_local_excute(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_linkage_config_rule_excute2_t *request = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_LINK_CONF_EXEC_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	request = get_uascp_payload(pkt, uc_linkage_config_rule_excute2_t);
	memcpy((void *)request, (void *)data, data_len);
	request->action = UCAU_HOME_RULE_EXCUTE;
	request->home_id = ntohl(request->home_id);
	request->rule_len = ntohl(request->rule_len);

	ucla_request_add(s, pkt);
}

static void _la_dict_query(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_dict_query_t *request = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_DICTIONARY, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	request = get_uascp_payload(pkt, uc_dict_query_t);
	memcpy((void *)request, (void *)data, data_len);
	request->action = UCAU_QUERY;
	
	request->home_id = ntohl(request->home_id);
	request->key_len = ntohs(request->key_len);

	ucla_request_add(s, pkt);
}


static void _la_dict_set(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_dict_set_t *request = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_DICT_SET_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_DICTIONARY, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	request = get_uascp_payload(pkt, uc_dict_set_t);
	memcpy((void *)request, (void *)data, data_len);
	request->action = UCAU_EDIT;

	log_debug(" dict set: home_id %u key_len %u value_len %u\n", request->home_id, request->key_len, request->value_len);
	
	request->home_id = ntohl(request->home_id);
	request->key_len = ntohs(request->key_len);
	request->value_len = ntohs(request->value_len);

	ucla_request_add(s, pkt);
}

static void _la_dict_del(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt = NULL;
	ucla_session_t *s = NULL;
	uc_dict_del_t *request = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_DICT_DEL_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_DICTIONARY, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	request = get_uascp_payload(pkt, uc_dict_del_t);
	memcpy((void *)request, (void *)data, data_len);
	request->action = UCAU_DELETE;
	
	request->home_id = ntohl(request->home_id);
	request->key_len = ntohs(request->key_len);

	ucla_request_add(s, pkt);
}

static void la_rule_query(ucla_session_t *s, u_int32_t home_id, u_int32_t rule_id)
{
	pkt_t *pkt;
	uc_link_conf_rulelist_t *request = NULL;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_link_conf_rulelist_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	request = get_uascp_payload(pkt, uc_link_conf_rulelist_t);

	request->aciton = UCAU_RULELIST;
	request->home_id = ntohl(home_id);
	request->rule_id = ntohl(rule_id);

	ucla_request_add(s, pkt);
	UCLA_EXIT();
}

static void _la_rule_query(u_int8_t *data, u_int32_t data_len)
{
	ucla_session_t *s;
	uc_link_conf_rulelist_t *request = (uc_link_conf_rulelist_t *)data;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_S3_QUERY_RULE_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}

	la_rule_query(s, request->home_id, request->rule_id);
}

static void _la_rule_rule_info_get(u_int8_t *data, u_int32_t data_len)
{
	int i, len;
	u_int32_t home_id = 0;
	la_rule_t *pr, *prn;
	la_g_rule_t *pgr = NULL;
	cl_home_rule_info_t *pl = NULL;
	cl_home_rule_info_t **pri = NULL;

	home_id = *(u_int32_t *)data;
	pri = *(cl_home_rule_info_t ***)&data[sizeof(home_id)];

	pgr = ucla_g_rule_find_by_homeid(home_id);
	if (!pgr) {
		log_debug("error homeid=%u\n", home_id);
		return;
	}
	pl = cl_calloc(sizeof(*pl), 1);
	if (!pl) {
		return;
	}
	stlc_list_count(pl->rule_num, &pgr->rule_list);
	pl->last_modify_time = pgr->last_modify_time;
	if (pl->rule_num) {
		len = pl->rule_num * sizeof(cl_rule_desc_t);
		pl->rule_array = cl_calloc(len, 1);
		if (!pl->rule_array) {
			cl_free(pl);
			return;
		}
		i = 0;
		stlc_list_for_each_entry_safe(la_rule_t, pr, prn, &pgr->rule_list, link) {
			pl->rule_array[i].enable = pr->enable;
			pl->rule_array[i].state = pr->state;
			pl->rule_array[i].rule_id = pr->rule_id;
			pl->rule_array[i].rule_len = pr->rule_len;
			pl->rule_array[i].last_exec_time = pr->last_exec_time;
			if (pr->rule_len) {
				pl->rule_array[i].rule = (u_int8_t *)cl_strdup(pr->rule);
			} else {
				pl->rule_array[i].rule = (u_int8_t *)cl_strdup("");
			}
			i++;
		}
	}

	*pri = pl;
}

static void _la_rule_last_modify_query(u_int8_t *data, u_int32_t data_len)
{
	pkt_t *pkt;
	ucla_session_t *s;
	uc_home_last_rule_time_q_t *request = NULL;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		ucla_event_push_err(LA_S3_QUERY_RULE_LAST_MODIFY_TIME_FAILED, 0, LA_PHONE_ALL_SESSION_OFFLINE);
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	request = get_uascp_payload(pkt, uc_home_last_rule_time_q_t);
	memcpy(request, data, data_len);
	request->action = UCAU_HOME_LAST_RULE_TIME;
	request->home_id = ntohl(request->home_id);

	ucla_request_add(s, pkt);
}

static void ucla_another_server_apply(char *doname, u_int32_t home_id, u_int8_t share_code[8], char *desc)
{
	ucla_session_t *s;
	uc_linkage_home_share_register_t *rt;
	pkt_t *pkt;

	//这里有修改，只要找个可用的session就行了，服务器有同步
	//s = ucla_get_ses_by_doname(doname);
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push(LA_HOME_SHARE_REGISTOR_FAILED_OFFLINE);
		return;
	}

	pkt = ucla_pkt_new(CMD_HOME_SHARE, sizeof(uc_linkage_home_share_register_t) + 64, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	rt = get_uascp_payload(pkt, uc_linkage_home_share_register_t);
	rt->action = UCAU_REGISTER;
	rt->home_id = ntohl(home_id);
	memcpy((void *)rt->share_code, (void *)share_code, sizeof(rt->share_code));
	strncpy((char *)rt->desc, desc, 64);
	
	ucla_request_add(s, pkt);	
}

static bool _la_ctrl_home_share_register(ucla_session_t *s, u_int8_t *data, u_int32_t data_len, RS *ret)
{
	char *ptmp = (char *)data;
	uc_linkage_home_share_register_t *rt;
	pkt_t *pkt;
	char buf[1024];
	u_int32_t home_id;
	u_int8_t share_code[8];
	u_int64_t sc;
	char doname[100];
	char *desc;
	char *pdoname;
	char *psc;
	char *pvalue;
	la_home_t *phome;

	rt = NULL;
	pkt = NULL;

	home_id = *(u_int32_t *)ptmp;
	ptmp += sizeof(u_int32_t);

	UCLA_ENTER();

	//\"v\":\"1\",\"sc\":\"%"PRIu64"\",\"hid\":\"%u\",\"doname\":\"%s\"
	//share
	strcpy(buf, ptmp);
	la_json_dec(buf, buf);
	log_debug("%s buf=%s\n", __FUNCTION__, buf);
	//doname
	if (NULL == (pdoname = strstr(buf, "\"doname\""))) {
		log_debug("error doname=%s\n", buf);
		ucla_event_push(LA_HOME_SHARE_REGISTOR_FAILED);
		*ret = RS_ERROR;
		return false;
	}
	log_debug("pdoname=%s\n", pdoname);
	memset(doname, 0, sizeof(doname));
	sscanf(pdoname, "\"doname\":\"%s\"", doname);
	pvalue = strchr(doname, '"');
	if (pvalue) {
		*pvalue = 0;
	}
	//homeid
	//sc
	if (NULL == (psc = strstr(buf, "\"hid\""))) {
		log_debug("error hid=%s\n", buf);
		ucla_event_push(LA_HOME_SHARE_REGISTOR_FAILED);
		*ret = RS_ERROR;
		return false;
	}
	log_debug("hid=%s\n", psc);
	sscanf(psc, "\"hid\":\"%u\"", &home_id);
	//sc
	if (NULL == (psc = strstr(buf, "\"sc\""))) {
		log_debug("error sc=%s\n", buf);
		ucla_event_push(LA_HOME_SHARE_REGISTOR_FAILED);
		*ret = RS_ERROR;
		return false;
	}
	log_debug("psc=%s\n", psc);
	sscanf(psc, "\"sc\":\"%"PRIu64"\"", &sc);
	la_u64_2_char(share_code, &sc);
	//desc
	ptmp += strlen(buf) + 1;
	desc = ptmp;
	log_debug("%s doname=%s sc=%"PRIu64" home_id=%u desc=%s\n", __FUNCTION__, doname, sc, home_id, desc);	

	//先判断一下当前服务器session下有没有该家庭，如果有，就不必再注册了
	phome = ucla_find_home_by_id(home_id);
	if (phome) {
		log_debug("%s had register already\n", __FUNCTION__);
		ucla_home_event_push(home_id, LA_HOME_SHARE_REGISTER_ALREADY);
		return true;
	}

	ucla_another_server_apply(doname, home_id, share_code, desc);

	return true;
}

static bool _la_ctrl_home_share_register_ext(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	char buf[1024];
	char *pv = NULL;
	int v = 0;
	ucla_session_t *s;
	char *ptmp = (char *)(data+sizeof(u_int32_t));//过滤掉homeid
	//这里区分一下是把家庭注册到自己用户还是注册到别人用户，用版本来区分，v没加密，v=100以上就是注册到别人用户

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	strcpy(buf, ptmp);
	log_info("befor dec buf=%s\n", buf);
	la_json_dec(buf, buf);
	if (NULL == (pv = strstr(buf, "\"v\""))) {
		ucla_event_push(LA_HOME_SHARE_REGISTOR_FAILED);
		return false;
	}
	sscanf(pv, "\"v\":\"%d\"", &v);
	log_info("buf=%s v=%d\n", buf, v);
	if (v >= 100) {
		return _la_ctrl_app_share_register(data, data_len);
	}

	return _la_ctrl_home_share_register(s, data, data_len, ret);
}

static bool _la_ctrl_home_share_query(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_home_share_query_t *request = (uc_linkage_home_share_query_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	request->action = UCAU_QUERY;
	request->home_id = ntohl(request->home_id);

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_HOME_SHARE, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	rt = get_uascp_payload(pkt, uc_linkage_home_share_query_t);
	memcpy(rt, request, data_len);
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_home_share_edit(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_home_share_edit_t *request = (uc_linkage_home_share_edit_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	request->action = UCAU_EDIT;
	request->home_id = ntohl(request->home_id);
	request->user_id = ntohl(request->user_id);

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push(LA_HOME_SHARE_DESC_CHANGE_FAILED);
		*ret = RS_OFFLINE;
		return false;
	}
	if (data_len < sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_HOME_SHARE, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	rt = get_uascp_payload(pkt, uc_linkage_home_share_edit_t);
	memcpy(rt, request, data_len);
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_home_share_del(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	uc_linkage_home_share_del_t *request = (uc_linkage_home_share_del_t *)data, *rt;
	pkt_t *pkt;
	ucla_session_t *s;

	request->action = UCAU_DELETE;
	request->home_id = ntohl(request->home_id);
	request->user_id = ntohl(request->user_id);

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	if (data_len != sizeof(*request)) {
		return false;
	}

	pkt = ucla_pkt_new(CMD_HOME_SHARE, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	rt = get_uascp_payload(pkt, uc_linkage_home_share_del_t);
	memcpy(rt, request, data_len);
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_home_adddev(u_int8_t *data, u_int32_t data_len)
{
	//废弃接口，担心被上层调用，屏蔽掉
#if 0	
	int i;
	pkt_t *pkt;
	user_t *user;
	uc_home_conf_adddev_t *pa;
	uc_la_dev_info_t *pdev;
	la_home_proc_notify_t *pn = (la_home_proc_notify_t *)data;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, (sizeof(uc_home_conf_adddev_t) + pn->num*sizeof(uc_la_dev_info_t)), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	
	pa = get_uascp_payload(pkt, uc_home_conf_adddev_t);
	pa->action = UCAU_ADDDEV;
	pa->count = pn->num;
	pa->home_id = htonl(pn->home_id);
	for(i = 0; i < pn->num; i++) {
		pdev = &pa->dev[i];
		pdev->sn = ntoh_ll(pn->sn[i]);
		user = find_user_by_sn(pn->sn[i]);
		pdev->flag = 0;
		// TODO:如果不设备不支持联动，则用设备密码添加上去，还不清楚设备端用什么表示不支持联动，先都默认支持
		if (user && !user_is_support_la(user)) {
			memcpy((void *)pdev->dev_passwd, (void *)user->passwd_md5, 16);
		}
	}
	
	ucla_request_add(s, pkt);

	return true;
#else
	return true;
#endif	
}

static bool _la_ctrl_home_removedev(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	int i = 0;
	pkt_t *pkt;
	la_home_t *phome;
	la_member_t *pmem;
	ucla_session_t *s;
	uc_home_conf_removedev_t *pr;
	la_home_proc_notify_t *pn = (la_home_proc_notify_t *)data;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push(LA_HOME_REMOVEDEV_FAILED);
		*ret = RS_OFFLINE;
		return false;
	}
	pmem = la_dev_wait_find_by_sn(pn->sn[0]);
	//这里还有种情况，一直没连接上外网，没有圈子
	if (pmem) {
		la_dev_wait_del(pmem);
		ucla_user_del_only(pn->sn[0]);
		ucla_event_push(LA_USER_CHANGED);
		return true;
	}
	//这里判断一下，如果设备属于没添加到服务器圈子成功的设备可以直接删除,flag不为0的是添加失败
	phome = ucla_find_home_by_id(pn->home_id);
	if (!phome) {
		ucla_event_push(LA_HOME_REMOVEDEV_FAILED);
		*ret = RS_ERROR;
		return false;
	}
	pmem = ucla_find_member_by_sn_from_home(phome, pn->sn[0]);
	if (pmem && pmem->conf.flag) {
		ucla_mem_del(phome, pn->sn[0]);
		ucla_home_mem_logout(phome->conf.home_id, pn->sn[0]);
		ucla_home_event_push(phome->conf.home_id, LA_HOME_ADDDEV_CHANGED);
		if (phome->session_ptr) {
			ucla_user_conf_save(phome->session_ptr);
		}
		return true;
	}

	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_home_event_push(phome->conf.home_id, LA_HOME_REMOVEDEV_FAILED);
		*ret = RS_ERROR;
		return false;
	}
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, (sizeof(uc_home_conf_removedev_t) + pn->num*sizeof(u_int64_t)), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	pkt->home_id = pn->home_id;
	pr = get_uascp_payload(pkt, uc_home_conf_removedev_t);
	pr->action = UCAU_REMOVEDEV;
	pr->count = 1;
	pr->flag = pn->flag;
	pr->home_id = htonl(pn->home_id);
	log_debug("%s del sn=%"PRIu64" flag 0x%x\n", __FUNCTION__, pn->sn[i], pn->flag);
	pr->sn[i] = ntoh_ll(pn->sn[i]);
	ucla_request_add(s, pkt);

	return true;
}

static bool _la_ctrl_home_movedev(u_int8_t *data, u_int32_t data_len)
{
#if 0
	int i;
	pkt_t *pkt;
	user_t *user;
	ucla_session_t *src_s;
	ucla_session_t *dst_s;
	uc_home_conf_move_t *pmn;
	uc_home_conf_move_t *pml = (uc_home_conf_move_t *)data;

	UCLA_ENTER();

	//这里判断下两个移动设备圈子是否是同一个服务器，不是就失败
	src_s = ucla_get_session_by_homeid(pml->src_home_id);
	dst_s = ucla_get_session_by_homeid(pml->dst_home_id);

	if (!src_s ||
		!dst_s ||
		(src_s != dst_s)) {
		log_debug("error move src_s=%p dst_s=%p\n", src_s, dst_s);
		return false;
	}

	s = src_s;
	//判断在不在线
	if (!ucla_session_is_ok(s)) {
		log_debug("s=%p is not ok status=%u\n", s, s->status);
		return false;
	}
	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	
	pmn = get_uascp_payload(pkt, uc_home_conf_move_t);
	pmn->action = UCAU_MOVE;
	pmn->count = pml->count;
	pmn->src_home_id = htonl(pml->src_home_id);
	pmn->dst_home_id = htonl(pml->dst_home_id);
	for(i = 0; i < pml->count; i++) {
		pmn->dev_sn[i].sn = ntoh_ll(pml->dev_sn[i].sn);
		user = find_user_by_sn(pml->dev_sn[i].sn);
		if (user && !user_is_support_la(user)) {
			memcpy((void *)pmn->dev_sn[i].passwd, (void *)user->passwd_md5, 16);
		}
	}
	
	ucla_request_add(s, pkt);

	return true;
#else
	return true;
#endif
}

static bool _la_ctrl_home_namemodify(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	pkt_t *pkt;
	ucla_session_t *s;
	uc_home_conf_namemodify_t *pmn;
	uc_home_conf_namemodify_t *pml = (uc_home_conf_namemodify_t *)data;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		ucla_event_push(LA_HOME_CONF_NAME_MODIFY_FAILED);
		*ret = RS_OFFLINE;
		return false;
	}
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	
	pmn = get_uascp_payload(pkt, uc_home_conf_namemodify_t);
	pml->action = UCAU_HOME_NAME_MODIFY;
	pml->home_id = htonl(pml->home_id);

	memcpy((void *)s->home_name, (void *)pml->name, sizeof(s->home_name));
	memcpy((void *)pmn, (void *)pml, data_len);
	log_debug("_la_ctrl_home_namemodify pml->name=%s\n", pml->name);
	
	ucla_request_add(s, pkt);

	return true;
}


static bool _la_ctrl_rule_modify(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	pkt_t *pkt;
	ucla_session_t *s;
	uc_linkage_config_rule_modify_t *pn = (uc_linkage_config_rule_modify_t *)data;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_OFFLINE;
		return false;
	}
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	pn = get_uascp_payload(pkt, uc_linkage_config_rule_modify_t);
	pn->action = UCAU_MODIFY;
	pn->home_id = htonl(pn->home_id);
	pn->rule_id = htonl(pn->rule_id);
	ucla_request_add(s, pkt);

	return true;
}


static bool _la_ctrl_sdk_login_set(u_int8_t *data, u_int32_t data_len)
{
//	u_int64_t *sn = (u_int64_t *)data;
	u_int16_t num = data_len/sizeof(u_int64_t);

	if (num == 0) {
		return false;
	}

	return true;
}

static bool _la_ctrl_sdk_conf_clean_set(u_int8_t *data, u_int32_t data_len)
{
	return true;
}

static bool _la_ctrl_sdk_all_info_get(u_int8_t *data, u_int32_t data_len)
{
	cl_la_info_t *pinfo = NULL;
	la_user_t *puser, *n;
	la_home_t *phome, *homen;
	int num;

	if (data_len != sizeof(cl_la_info_t **)) {
		log_debug("_la_ctrl_sdk_all_info_get err datalen=%u\n", data_len);
		return true;
	}
	num = ucla_get_all_home_num();
	pinfo = cl_calloc(sizeof(*pinfo), 1);
	pinfo->support_label = plc->support_tabel;
	//没有圈子时整个假的，把所有设备都放里面
	if (num == 0) {
		pinfo->home = cl_calloc(sizeof(cl_user_t *), 1);
		pinfo->home[0] = la_ctrl_get_invalid_home();
		if (pinfo->home[0]) {
			pinfo->home_num = 1;
		}
		//设备能力文件
		cl_cap_fill(pinfo);
	} else {
		pinfo->home = cl_calloc((num*sizeof(cl_user_t *)), 1);
		pinfo->home_num = 0;
		cl_lock(&cl_priv->mutex);		
		stlc_list_for_each_entry_safe(la_user_t, puser, n, &cl_priv->la_user, link) {
			stlc_list_for_each_entry_safe(la_home_t, phome, homen, &puser->home_link, link) {
				pinfo->home[pinfo->home_num++] = cl_user_get_info(phome->handle);
			}
			pinfo->user_id[pinfo->user_num++] = puser->conf.user_id;
		}	
		cl_unlock(&cl_priv->mutex);	
		//手机账号
		cl_phone_fill(pinfo);
		//设备能力文件
		cl_cap_fill(pinfo);
	}

	**(cl_la_info_t ***)data = pinfo;

	return true;
}

static bool _la_ctrl_log_rule_query(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	pkt_t *pkt;
	ucla_session_t *s;
	ucp_log_rule_query_t *request = (ucp_log_rule_query_t *)data;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		*ret = RS_OFFLINE;
		return false;
	}
	pkt = ucla_pkt_new(CMD_QUERY_HISTORY, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	request = get_uascp_payload(pkt, ucp_log_rule_query_t);
	memcpy((void *)request, data, data_len);
	request->action = UCAU_HISTORY_RULE_QUERY;
	request->home_id = ntohl(request->home_id);
	request->index = ntohl(request->index);
	request->query_count = ntohl(request->query_count);
	log_debug("request->index=%u homeid=%u count=%u !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 
		ntohl(request->index), ntohl(request->home_id), ntohl(request->query_count));
	ucla_request_add(s, pkt);
	
	return true;
}

static bool _la_ctrl_log_rule_get(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	int i, len, n;
	u_int32_t home_id = 0;
	home_log_rule_info_t *pinfo = NULL;
	la_log_home_rule_node_t *pn, *pnn;
	la_log_home_rule_change_t *prl = NULL;
	cl_home_log_rule_change_t **pr = NULL;
	cl_home_log_rule_change_t *pl = NULL;

	home_id = *(u_int32_t *)data;
	pr = *(cl_home_log_rule_change_t ***)&data[sizeof(home_id)];

	prl = ucla_log_rule_find_by_homeid(home_id);
	if (!prl) {
		log_debug("there is no homeid=%u\n", home_id);
		return true;
	}
	pl = cl_calloc(sizeof(*pl), 1);
	if (!pl) {
		log_err(true, "calloc failed\n");
		return false;
	}
	stlc_list_count(n, &prl->log_list);
	pl->max_index = prl->max_index;
	
	pl->home_id = home_id;
	if (n == 0) {
		log_debug("n == 0\n");
		goto done;
	}
	pl->plog = cl_calloc((n * sizeof(home_log_rule_info_t)), 1);
	if (!pl->plog) {
		log_err(true, "calloc failed\n");
		goto done;
	}
	i = 0;
	stlc_list_for_each_entry_safe(la_log_home_rule_node_t, 
		pn, pnn, &prl->log_list, link) {
		pinfo = &pl->plog[i++];
		if (pn->log_info.monitor_count) {
			len = pn->log_info.monitor_count * sizeof(u_int64_t);
			pinfo->pmonitor_sn = cl_calloc(len, 1);
			if (!pinfo->pmonitor_sn) {
				log_err(true, "calloc failed \n");
				goto done;
			}
			memcpy((void *)pinfo->pmonitor_sn, (void *)pn->log_info.pmonitor_sn, len);
			pinfo->monitor_count = pn->log_info.monitor_count;
		}
		if (pn->log_info.action_count) {
			len = pn->log_info.action_count * sizeof(u_int64_t);
			pinfo->paction_sn = cl_calloc(len, 1);
			if (!pinfo->paction_sn) {
				log_err(true, "calloc failed\n");
				goto done;
			}
			memcpy((void *)pinfo->paction_sn, (void *)pn->log_info.paction_sn, len);
			pinfo->action_count = pn->log_info.action_count;
		}
		pinfo->ruld_id = pn->log_info.ruld_id;
		pinfo->time_stamp = pn->log_info.time_stamp;
		pinfo->index = pn->log_info.index;
		pinfo->is_test = pn->log_info.is_test;
		pinfo->is_timer = pn->log_info.is_timer;
	}
	pl->log_count = i;

done:
	
	*pr = pl;
	
	return true;
}

static bool _la_ctrl_log_mem_query(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	pkt_t *pkt;
	ucla_session_t *s;
	ucp_log_member_query_t *request = (ucp_log_member_query_t *)data;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		*ret = RS_OFFLINE;
		return false;
	}
	pkt = ucla_pkt_new(CMD_QUERY_HISTORY, data_len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	request = get_uascp_payload(pkt, ucp_log_member_query_t);
	memcpy((void *)request, data, data_len);
	request->action = UCAU_HISTORY_MEMBER_QUERY;
	request->home_id = ntohl(request->home_id);
	ucla_request_add(s, pkt);
	
	return true;
}

static bool _la_ctrl_log_mem_get(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	int i, n;
	u_int32_t home_id = 0;
	home_log_member_info_t *pinfo = NULL;
	la_log_home_member_node_t *pn, *pnn;
	la_log_home_member_change_t *prl = NULL;
	cl_home_log_member_change_t **pr = NULL;
	cl_home_log_member_change_t *pl = NULL;

	home_id = *(u_int32_t *)data;
	pr = *(cl_home_log_member_change_t ***)&data[sizeof(home_id)];

	prl = ucla_log_mem_find_by_homeid(home_id);
	if (!prl) {
		log_err(false, "not found homeid=%u\n", home_id);
		return false;
	}
	pl = cl_calloc(sizeof(*pl), 1);
	if (!pl) {
		log_err(true, "calloc failed\n");
		return false;
	}
	pl->home_id = prl->home_id;
	stlc_list_count(n, &prl->log_list);
	if (n == 0) {
		log_debug("n == 0\n");
		goto done;
	}
	pl->plog = cl_calloc((n * sizeof(home_log_member_info_t)), 1);
	if (!pl->plog) {
		log_err(true, "calloc failed\n");
		goto done;
	}
	i = 0;
	stlc_list_for_each_entry_safe(la_log_home_member_node_t, 
		pn, pnn, &prl->log_list, link) {
		pinfo = &pl->plog[i++];
		pinfo->sn = pn->log_info.sn;
		pinfo->action = pn->log_info.action;
		pinfo->resaon = pn->log_info.resaon;
		pinfo->user_id = pn->log_info.user_id;
		pinfo->time_stamp = pn->log_info.time_stamp;
		memcpy(pinfo->username, pn->log_info.username, sizeof(pinfo->username));
	}
	pl->log_count = i;

done:
	*pr = pl;
	
	return true;
}

static bool _la_ctrl_widget_key_query(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	pkt_t *pkt;
	ucla_session_t *s;
	ucp_widget_key_request_t *request;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_info("there is no enstablish session\n");
		*ret = RS_OFFLINE;
		return false;
	}
	pkt = ucla_pkt_new(CMD_WIDGET_KEY, sizeof(*request), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}
	request = get_uascp_payload(pkt, ucp_widget_key_request_t);
	request->action = UCAU_QUERY;
	ucla_request_add(s, pkt);
	
	return true;
}

static bool _la_ctrl_widget_info_get(u_int8_t *data, u_int32_t data_len, RS *ret)
{
	cl_widget_info_t **pr;
	cl_widget_info_t *pl;
	ucla_session_t *s;

	pr = *(cl_widget_info_t ***)data;

	s = ucla_get_any_enstablis_session();
	if (!s) {
		*ret = RS_ERROR;
		log_err(false, "there is no enstablish session\n");
		return false;
	}
	pl = cl_calloc(sizeof(*pl), 1);
	if (!pl) {
		*ret = RS_ERROR;
		log_err(true, "calloc failed\n");
		return false;
	}
	strcpy((char *)pl->doname, (char *)s->doname);
	memcpy(pl->key, s->widget_key, sizeof(pl->key));

	*pr = pl;
	
	return true;
}


bool _la_ctrl_proc_notify(u_int8_t action, u_int8_t *data, u_int32_t data_len, RS *ret)
{	
	switch(action) {
	case LA_ACTION_HOME_CREATE:
		 _la_ctrl_home_create(data, data_len, ret);
		break;
	case LA_ACTION_HOME_DEL:
		_la_ctrl_home_del(data, data_len, ret);
		break;
	case LA_ACTION_HOME_ADDDEV:
		_la_ctrl_home_adddev(data, data_len);
		break;
	case LA_ACTION_HOME_REMOVEDEV:
		_la_ctrl_home_removedev(data, data_len, ret);
		break;
	case LA_ACTION_DEV_MOVE:
		_la_ctrl_home_movedev(data, data_len);
		break;
	case LA_ACTION_HOME_NAME_MODIFY:
		_la_ctrl_home_namemodify(data, data_len, ret);		
		break;

	// 联动配置
	case LA_ACTION_TEMPLATE_QUERY:
		_la_ctrl_template_query(data, data_len);
		break;
	case LA_ACTION_RULELIST_QUERY:
		_la_ctrl_rulelist_query(data, data_len, ret);
		break;
	case LA_ACTION_RULELIST_ADD:
		_la_ctrl_rulelist_add(data, data_len, ret);
		break;
	case LA_ACTION_RULELIST_DEL:
		_la_ctrl_rulelist_del(data, data_len, ret);
		break;
	case LA_ACTION_LINK_RULE_EXEC:
		_la_ctrl_rulelist_exec(data, data_len, ret);
		break;
	case LA_ACTION_RULE_MODIFY:
		_la_ctrl_rule_modify(data, data_len, ret);
		break;
	case LA_ACTION_EVENT:
		_la_ctrl_event(data, data_len, ret);
		break;

	// 分享相关
	case LA_ACTION_HOME_SHARE_CREATE:
		_la_ctrl_home_share_create(data, data_len, ret);
		break;
	case LA_ACTION_HOME_SHARE_REGISTER:
		_la_ctrl_home_share_register_ext(data, data_len, ret);
		break;
	case LA_ACTION_HOME_SHARE_QUERY:
		_la_ctrl_home_share_query(data, data_len, ret);
		break;
	case LA_ACTION_HOME_SHARE_EDIT:
		_la_ctrl_home_share_edit(data, data_len, ret);
		break;
	case LA_ACTION_HOME_SHARE_DEL:
		_la_ctrl_home_share_del(data, data_len, ret);
		break;
	case LA_ACTION_PAD_SC_REQ:
		_la_ctrl_app_share_create(data, data_len);
		break;
	case LA_ACTION_PAD_SC_REGISTER:
		_la_ctrl_app_share_register(data, data_len);
		break;

	//标签相关
	case LA_ACTION_LABEL_ADD:
		_la_label_add(data, data_len);
		break;
	case LA_ACTION_LABEL_DEL:
		_la_label_del(data, data_len);
		break;
	case LA_ACTION_LABEL_BIND:
		_la_label_bind(data, data_len);
		break;

	//s3网关相关
	case LA_ACTION_S3_LMT_QUERY:
		_la_rule_last_modify_query(data, data_len);
		break;
	case LA_ACTION_S3_RULE_QUERY:
		_la_rule_query(data, data_len);
		break;
	case LA_ACTION_S3_RULEINFO_GET:
		_la_rule_rule_info_get(data, data_len);
		break;

	// 字典相关
	case LA_ACTION_DICT_QUERY:
		_la_dict_query(data, data_len);
		break;
	case LA_ACTION_DICT_SET:
		_la_dict_set(data, data_len);
		break;
	case LA_ACTION_DICT_DEL:
		_la_dict_del(data, data_len);
		break;

	//快捷按键
	case LA_ACTION_SHORTCUT_ADD:
		_la_home_shortcut_add(data, data_len);
		break;
	case LA_ACTION_SHORTCUT_DEL:
		_la_home_shortcut_del(data, data_len);
		break;
	case LA_ACTION_SHORTCUT_MOD:
		_la_home_shortcut_mod(data, data_len);
		break;

	//pad使用
	case LA_ACTION_PAD_EXCUTE_LOCAL_RULE:
		_la_home_rule_local_excute(data, data_len);
		break;
		
	//一般控制
	case LA_ACTION_SDK_LOGIN_SET:
		_la_ctrl_sdk_login_set(data, data_len);
		break;
	case LA_ACTION_SDK_CONF_CLEAN:
		//清除配置
		_la_ctrl_sdk_conf_clean_set(data, data_len);
		break;
	case LA_ACTION_ALL_INFO_GET:
		_la_ctrl_sdk_all_info_get(data, data_len);
		break;

	//日志相关
	case LA_ACTION_LOG_RULE_QUERY:
		_la_ctrl_log_rule_query(data, data_len, ret);
		break;
	case LA_ACTION_LOG_RULE_GET:
		_la_ctrl_log_rule_get(data, data_len, ret);
		break;
	case LA_ACTION_LOG_MEM_QUERY:
		_la_ctrl_log_mem_query(data, data_len, ret);
		break;
	case LA_ACTION_LOG_MEM_GET:
		_la_ctrl_log_mem_get(data, data_len, ret);
		break;

	//widget相关
	case LA_ACTION_WIDGET_KEY_QUERY:
		_la_ctrl_widget_key_query(data, data_len, ret);
		break;
	case LA_ACTION_WIDGET_INFO_GET:
		_la_ctrl_widget_info_get(data, data_len, ret);
		break;
	default:
		break;
	}

	return true;
}

//如果有手机账号，就只能登陆，不能创建
static void la_phone_ctrl_create(char *name, char *passwd, RS *ret)
{
	MD5_CTX ctx;
	//ucla_session_t *s;
	la_phone_t *phone;

	*ret = RS_ERROR;
	UCLA_ENTER();
	if (!name || !passwd) {
		return;
	}
	phone = ucla_find_phone_by_name(name);
	if (phone) {
		ucla_event_push_err(LA_PHONE_CREATE_FAILED, 0, LA_PHONE_ACOUNT_EXIST);
		return;
	}

	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)passwd, (u_int32_t)strlen(passwd));
	MD5Final(plc->passwd, &ctx);
	strcpy((char *)plc->user_name, name);
	
	// TODO:初始化手机账户创建进程
	ucla_phone_set_status(UCLA_PHONE_CREATE);
	
	*ret = RS_OK;	
}

static int la_phone_passwd_is_md5( char *passwd)
{
	char key_str[] = "ismd5str";
	int key_len = (int)strlen(key_str), len, outlen = sizeof(plc->passwd);

	len = (int)strlen(passwd);

	if (len != 32 + key_len) {
		return 0;
	}

	if (memcmp((u_int8_t*)&passwd[32], key_str, key_len) != 0) {
		return false;
	}

	passwd[32] = 0;
	len = 16;

	if (char2hex(passwd, plc->passwd, &outlen) < 0) {
		return -1;
	}

	return 1;
}


void la_phone_login(char *name, char *passwd, bool is_ctrl)
{
	MD5_CTX ctx;
	
	if (!name || !passwd) {
		return;
	}

	if (la_phone_passwd_is_md5(passwd)) {
		;
	} else {	
		MD5Init(&ctx);
		MD5Update(&ctx, (u_int8_t *)passwd, (u_int32_t)strlen(passwd));
		MD5Final(plc->passwd, &ctx);
	}
	strcpy((char *)plc->user_name, name);
	//这里要判断一下，只有连接上服务器后才能直接登陆，否则要延后
	// TODO:初始化手机账户登陆进程
	if (is_ctrl) {
		ucla_phone_set_status(UCLA_PHONE_LOGIN);
	} else {
		plc->need_phone_login_delay = true;
	}
}

static void la_phone_ctrl_login(char *name, char *passwd, RS *ret)
{
	*ret = RS_OK;
	
	la_phone_login(name, passwd, true);	
}

static void la_phone_ctrl_relogin(RS *ret)
{
	ucla_all_sleep_session_reset();
}

static void la_phone_ctrl_passwd_modify(char *name, char *passwd, RS *ret)
{
	MD5_CTX ctx;

	UCLA_ENTER();
	*ret = RS_ERROR;
	
	if (!name || !passwd) {
		return;
	}

	if (!ucla_find_phone_by_name(name)) {
		ucla_event_push(LA_PHONE_PASSWD_MODIFY_FAILED);
		return;
	}
	
	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)passwd, (u_int32_t)strlen(passwd));
	MD5Final(plc->passwd, &ctx);
	strcpy((char *)plc->user_name, name);
	// TODO:初始化手机账户登陆进程
	ucla_phone_set_status(UCLA_PHONE_PASSWD_MODIFY);
	*ret = RS_OK;		
}


static void la_phone_ctrl_logout(char *name, RS *ret)
{
	la_phone_t *phone;

	*ret = RS_ERROR;
	
	if (!name) {
		return;
	}

	if ((phone = ucla_find_phone_by_name(name)) == NULL) {
		return;
	}

	ucla_phone_del(phone);
	strcpy((char *)plc->user_name, name);
	// TODO:初始化手机账户登出进程
	ucla_phone_set_status(UCLA_PHONE_LOGOUT);

	ucla_event_push(LA_PHONE_LOGOUT_SUCCESS);
}

static void la_phone_ctrl_del(char *name, RS *ret)
{
	la_phone_t *phone;

	*ret = RS_ERROR;
	
	if (!name) {
		return;
	}

	if ((phone = ucla_find_phone_by_name(name)) == NULL) {
		ucla_event_push_err(LA_PHONE_DEL_FAILED, 0, LA_PHONE_NOT_EXIST);
		return;
	}

	ucla_phone_del(phone);
	// TODO:初始化手机账户删除进程
	strcpy((char *)plc->user_name, name);
	ucla_phone_set_status(UCLA_PHONE_DEL);

	ucla_event_push(LA_PHONE_DEL_SUCCESS);	
}

static void la_phone_ctrl_swich(char *name, RS *ret)
{
	la_phone_t *phone;

	*ret = RS_ERROR;
	
	if (!name) {
		return;
	}

	if ((phone = ucla_find_phone_by_name(name)) == NULL) {
		return;
	}

	// TODO:初始化手机账户切换进程
	strcpy((char *)plc->user_name, name);
	ucla_phone_set_status(UCLA_PHONE_SWICH);
	ucla_event_push(LA_PHONE_SWICH_SUCCESS);
}

bool _la_phone_ctrl_proc_notify(u_int8_t action, u_int8_t *data, u_int32_t data_len, RS *ret)
{
	la_phone_notify_t *ppn = (la_phone_notify_t *)data;

	switch(action) {
	case LA_ACTION_PHONE_CREATE:
		la_phone_ctrl_create(ppn->name, ppn->passwd, ret);
		break;
	case LA_ACTION_PHONE_LOGIN:
		la_phone_ctrl_login(ppn->name, ppn->passwd, ret);
		break;
	case LA_ACTION_PHONE_RELOGIN:
		la_phone_ctrl_relogin(ret);
		break;
	case LA_ACTION_PHONE_LOGOUT:
		la_phone_ctrl_logout(ppn->name, ret);
		break;
	case LA_ACTION_PHONE_DEL:
		la_phone_ctrl_del(ppn->name, ret);
		break;
	case LA_ACTION_PHONE_SWICH:
		la_phone_ctrl_swich(ppn->name, ret);
		break;
	case LA_ACTION_PHONE_PASSWD_MODIFY:
		la_phone_ctrl_passwd_modify(ppn->name, ppn->passwd, ret);
		break;
	default:
		break;
	}

	return true;
}

static void do_comm_msg_get(u_int8_t *data, u_int32_t data_len)
{
	cl_la_notify_msg_t *pmsg = NULL;

	if (!data ||
		(data_len != sizeof(cl_la_notify_msg_t **))) {
		return;
	}

	pmsg = cl_calloc(sizeof(*pmsg), 1);
	if (!pmsg) {
		return;
	}

	pmsg->type = plc->msg.type;

	if (plc->msg.cn_msg) {
		pmsg->cn_msg = cl_strdup(plc->msg.cn_msg);
	} else {
		pmsg->cn_msg = cl_strdup("");
	}

	if (plc->msg.en_msg) {
		pmsg->en_msg = cl_strdup(plc->msg.en_msg);
	} else {
		pmsg->en_msg = cl_strdup("");
	}

	**(cl_la_notify_msg_t ***)data = pmsg;
}

static void do_comm_cap_query()
{
	pkt_t *pkt;
	ucla_session_t *s;
	uc_link_conf_cap_t *pr;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		return;
	}
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, (sizeof(uc_link_conf_cap_t)), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	pr = get_uascp_payload(pkt, uc_link_conf_cap_t);
	pr->action = UCAU_CAPFILE;
	pr->lang = plc->lang;

	log_debug("do_comm_cap_query goto %s query UCAU_CAPFILE\n", s->doname);
	ucla_request_add(s, pkt);	
}

static void do_comm_cap_custom_query()
{
	pkt_t *pkt;
	ucla_session_t *s;
	uc_link_conf_cap_t *pr;

	UCLA_ENTER();
	s = ucla_get_any_enstablis_session();
	if (!s) {
		return;
	}
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, (sizeof(uc_link_conf_cap_t)), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	pr = get_uascp_payload(pkt, uc_link_conf_cap_t);
	pr->action = UCAU_CUSTOM_CAPFILE;
	pr->lang = plc->lang;

	log_debug("do_comm_cap_custom_query goto %s query UCAU_CUSTOM_CAPFILE\n", s->doname);
	ucla_request_add(s, pkt);	
}

#if 1
const char *test_rsa_pub = 
"-----BEGIN PUBLIC KEY-----\n\
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDL6t8PeT0C3J8dkm5DXgn4NsuX\n\
EBPplfNGngVkzU1IaoRCXPy/IKdto573pFcDwJZlZ6IRnCMlr+cjrRQs6ZbQGLtz\n\
7RIOamFRbJyq6nSFHXodiAmyQ8k846qpXmqJA4H4/qtlGs58nU4tb4IcyI98xE8n\n\
8Z828WT6WagafmHe9QIDAQAB\n\
-----END PUBLIC KEY-----";

const char *test_rsa_priv =
"-----BEGIN PRIVATE KEY-----\n\
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAMvq3w95PQLcnx2S\n\
bkNeCfg2y5cQE+mV80aeBWTNTUhqhEJc/L8gp22jnvekVwPAlmVnohGcIyWv5yOt\n\
FCzpltAYu3PtEg5qYVFsnKrqdIUdeh2ICbJDyTzjqqleaokDgfj+q2UaznydTi1v\n\
ghzIj3zETyfxnzbxZPpZqBp+Yd71AgMBAAECgYEAuPckue5o7w2brsFCDw1f0awy\n\
Y6YLyddgQe/YSKUIsuUbbu7Vu7As7oB6E3sYCqlIQlcMZRjRsL+r02qNYGfJiP4+\n\
3iW1oKA5kIiwz4U5bYVGcxrOV3Jq6PfjDXw8Mx4l1KK3H3kTQfi7PjNoLLIPXxEd\n\
HUFAfOhSVIoXTKU6wUkCQQDnXfMmbt/er0+YRH0fYQK9byq/Oxrd8XmlPFPRP3b7\n\
MiefyrE6+uoojF4wXNUD8pJREeikEts4wVAVONh37oA7AkEA4aDEjEt+LBVK/GoO\n\
6oAdPW2FWkWeuV5o/rIciZHF4EZvYTizC/A4+iXV1SvgmijuImXuzjm50DCuAxkg\n\
S4LajwJAQb+xGXroo5+uPtc3t3MaFDtDKpThTbERoKNELnKanUfjesVfGCO150/8\n\
dEHtRvHoIqG0Ouwg4EAPQfili297gQJBAKUErDvfyvv65skN2LLx2HDg9TM80AXL\n\
YwdExVJoZ0zMtxFoIquMml955JbwbGjTnrdRzgLOfXQi5mIlslUH80kCQHoVfIzr\n\
HJNdhJ/PIuTmT6Q5XCFMRdW69hvPDkdtXO2m/bagTwFMKhzxwVrLKJxyrTQQ03JS\n\
YXhFPlIW9KZzphw=\n\
-----END PRIVATE KEY-----";

void test_rsa(void)
{
	char test[] = "123abc";
	u_int8_t en[256] = {0};
	u_int8_t dec[256] = {0};

#ifndef ANDROID
	printf("rsa test encode priv key len %d\n", (int)strlen(test_rsa_priv));
#endif
	plc->rsa_enc((int)strlen(test_rsa_pub), (u_int8_t *)test_rsa_pub, (int)strlen(test), (u_int8_t *)test, en);

#ifndef ANDROID
	printf("rsa test dec pub key len %d\n", (int)strlen(test_rsa_pub));
#endif

	plc->rsa_dec((int)strlen(test_rsa_priv), (u_int8_t *)test_rsa_priv, 128, en, dec);

#ifndef ANDROID
	printf("after dec [%s]\n", dec);
#endif
}

#endif

static void do_comm_rsa_set(u_int8_t *data, u_int32_t data_len)
{
	cln_la_rsa_t *rsa = NULL;

	UCLA_ENTER();
	rsa = (cln_la_rsa_t *)data;
	if (sizeof(*rsa) != data_len) {
		log_debug("err len=%u\n", data_len);
		return;
	}

	plc->rsa_dec = rsa->dec;
	plc->rsa_enc = rsa->enc;

test_rsa();

	if (!plc->rsa_dec ||
		!plc->rsa_enc ||
		!plc->rsa_priv) {
		plc->use_rsa = false;
		return;
	}

#ifdef LA_USE_RSA
	plc->use_rsa = true;
#endif	

	//test_rsa();
}

bool _la_comm_proc_notify(u_int8_t action, u_int8_t *data, u_int32_t data_len, RS *ret)
{
	switch(action) {
	case LA_NOTIFY_MSG_GET:
		do_comm_msg_get(data, data_len);
		break;
	case LA_ACTION_MISC_QUERY:
		do_comm_cap_query();
		break;
	case LA_ACTION_MISC_CUSTOM_CAP_QUERY:
		do_comm_cap_custom_query();
		break;
	case LA_ACTION_RSA_SET:
		do_comm_rsa_set(data, data_len);
		break;
	default:
		break;
	}

	return true;
}

static void do_ucla_home_create_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_home_conf_create_r_t *phccr = NULL;

	phccr = get_net_ucp_payload(hdr, uc_home_conf_create_r_t);

	phccr->result = htons(phccr->result);
	phccr->home_id = htonl(phccr->home_id);


	UCLA_ENTER();
	phome = ucla_find_home_by_id(0xffffffff);
	if (!phome) {
		log_debug("not found home id 0xffffffff\n");
		return;
	}

	if ((phccr->aciton != UCAU_CREATE) ||
		(phccr->result != 0)) {
		log_debug("do_ucla_home_conf_proc not ok phccr->result=%u\n", phccr->result);
		// TODO:如果失败给app一个事件，让app删除
		ucla_event_push_err(LA_HOME_CREATE_FAILED, phome->handle, 0);
		return;
	}
	
	phome->conf.home_id = phccr->home_id;

	ucla_user_conf_save(s);
	do_ucla_home_query(s, phome->conf.home_id);
	//圈子创建成功后去查询分享成员信息
	do_ucla_share_query(s, phome->conf.home_id);
	log_debug("do_ucla_home_conf_proc home create successed id=%u name=%s\n", phccr->home_id, phome->conf.home_name);
	ucla_home_event_push(phccr->home_id, LA_USER_CHANGED);
	ucla_home_event_push(phccr->home_id, LA_HOME_CREATE_SUCCESSED);
}

void ucla_event_push(int event)
{
	ucla_event_push_err(event, 0, 0);
}

void ucla_event_push_err(int event, u_int32_t handle, int err)
{
	if (plc->callback) {
		event_push_err(plc->callback, event, handle, plc->callback_handle, err);
	} else {
		log_debug("plc->callback is null !!!!!!!!!!!!!!!!!!!\n");
	}
}

void ucla_home_event_push(u_int32_t home_id, int event)
{
	la_home_t *phome;

	phome = ucla_find_home_by_id(home_id);
	if (phome) {
		log_debug("ucla_home_event_push event=%u!!!!!!!!!!!!!!!!!!!\n", event);
		ucla_event_push_err(event, phome->handle, home_id);
		return;
	}

	ucla_event_push(event);
}

static void ucla_home_all_mem_logout(u_int32_t home_id)
{
	user_t *user, *usern;

	UCLA_ENTER();
	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (user->home_id != home_id) {
			continue;
		}

		log_debug("%s del sn=%"PRIu64"\n", __FUNCTION__, user->sn);
		if(!user->is_phone_user){
			//delete_lan_dev_by_sn(user->sn);
		}

		user_destroy(user);
	}
	cl_unlock(&cl_priv->mutex);
}

static void ucla_user_del_only(u_int64_t sn)
{
	user_t *user, *usern;
	
	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (sn != user->sn) {
			continue;
		}

		user_destroy(user);
		break;
	}
	cl_unlock(&cl_priv->mutex);	
}

static void ucla_home_mem_logout(u_int32_t home_id, u_int64_t sn)
{
	user_t *user, *usern;

	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if ((user->home_id != home_id) ||
			(user->sn != sn)) {
			continue;
		}
		
		if(!user->is_phone_user){
			//delete_lan_dev_by_sn(user->sn);
		}

		user_destroy(user);
		break;
	}
	cl_unlock(&cl_priv->mutex);
}

static void ucla_user_logout(u_int64_t sn)
{
	user_t *user, *usern;

	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (user->sn != sn) {
			continue;
		}
		
		if(!user->is_phone_user){
			//delete_lan_dev_by_sn(user->sn);
		}

		user_destroy(user);
		break;
	}
	cl_unlock(&cl_priv->mutex);
}

void la_user_update_homeid(u_int64_t sn, u_int32_t home_id)
{
	user_t *user, *usern;
	
	cl_lock(&cl_priv->mutex);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (user->sn != sn) {
			continue;
		}
		
		user->home_id = home_id;
		break;
	}
	cl_unlock(&cl_priv->mutex);	
}

static void do_ucla_home_del_proc(ucla_session_t *s,ucph_t* hdr)
{
//	la_home_t *phome = NULL;
	uc_home_conf_delete_r_t *pdel = NULL;
	
	pdel = get_net_ucp_payload(hdr, uc_home_conf_delete_r_t);
	pdel->result = htons(pdel->result);

	UCLA_ENTER();	
	if (pdel->aciton != UCAU_DELETE) {
		return;
	}

	if (pdel->result) {
 		// TODO:删除失败
		log_debug("do_ucla_home_del_proc del failed\n");
 		ucla_event_push(LA_HOME_DEL_FAILED);
		return;
	}

	log_debug("do_ucla_home_del_proc del successed\n");
	ucla_home_all_mem_logout(s->home_del_id);
	ucla_home_del(s, s->home_del_id);
	ucla_event_push(LA_USER_CHANGED);
}

static void ucla_set_mem_del(la_home_t *phome)
{
	la_member_t *pmem, *memn;

	if (!phome) {
		return;
	}
	stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &phome->member_link, link) {
		if (pmem->conf.flag == 0) {
			pmem->del = true;
		}
	}
}

static bool ucla_do_del_mem(la_home_t *phome)
{
	la_member_t *pmem, *memn;
	bool ret = false;
	int num = 0;

	//释放成员链表
	stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &phome->member_link, link) {
		if (!pmem->del) {
			continue;
		}

		ret = true;
		//这里要判断下，如果该sn在不通圈子里，就不要logout了
		num = ucla_get_user_num(pmem->conf.sn);
		log_debug("la_test ucla_do_del_mem num sn=%"PRIu64" =%d\n", pmem->conf.sn, num);
		if (num <= 1) {
			ucla_user_logout(pmem->conf.sn);
		}
		stlc_list_del(&pmem->link);
		cl_free(pmem);
	}

	return ret;
}

static void do_ucla_home_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i;
	u_int64_t sn;
	user_t *puser;
	la_home_t *phome = NULL;
	la_member_t *pmem = NULL;
	uc_home_conf_query_r_t *pq = NULL;

	pq = get_net_ucp_payload(hdr, uc_home_conf_query_r_t);
	//长度check一下
	if (hdr->param_len < (pq->count*sizeof(uc_la_dev_query_info_t) + 
		sizeof(uc_home_conf_query_r_t))) {
		log_debug("err len=%u cont=%u callen=%u\n", hdr->param_len, pq->count, 
			(pq->count*sizeof(uc_la_dev_query_info_t) + sizeof(uc_home_conf_query_r_t)));
		return;
	}
	pq->home_id = htonl(pq->home_id);
	pq->last_rule_time = htonl(pq->last_rule_time);
	pq->last_template_time = htonl(pq->last_template_time);
	//这里看情况取得最大的上次模板变化时间
	if (plc->last_template_time < pq->last_template_time) {
		plc->last_template_time = pq->last_template_time;
		plc->template_session = s;
		la_template_query();
	}
	phome = ucla_find_home_by_id(pq->home_id);
	if (!phome) {
		log_debug("pq->home_id=%u not found\n", pq->home_id);
		return;
	}
	phome->last_rule_time = pq->last_rule_time;
	phome->last_template_time = pq->last_template_time;
	log_debug("%s count=%u pq->home_id=%u pq->home_name=%s paramlen=%u "
		"pq->last_rule_time=%u pq->last_template_time=%u\n", 
		__FUNCTION__, pq->count, pq->home_id, pq->home_name, hdr->param_len, 
		pq->last_rule_time, pq->last_template_time);
	if (memcmp((void *)phome->conf.home_name, (void *)pq->home_name, sizeof(phome->conf.home_name))) {
		memcpy((void *)phome->conf.home_name, (void *)pq->home_name, sizeof(phome->conf.home_name));
	}	
	if (memcmp((void *)phome->conf.home_passwd, (void *)pq->passwd, APP_USER_UUID_NAME_LEN)) {
		memcpy((void *)phome->conf.home_passwd, (void *)pq->passwd, APP_USER_UUID_NAME_LEN);
	}
	//本地数据结构打标志，好做删除同步 
	ucla_set_mem_del(phome);
	for(i = 0; i < pq->count; i++) {
		sn = ntoh_ll(pq->dev[i].sn);
		pmem = ucla_find_member_by_sn_from_home(phome, sn);
		if (!pmem) {
			pmem = ucla_member_new();
			if (!pmem) {
				return;
			}
			ucla_member_add(pmem, phome);
		}
		pmem->del = false;
		pmem->conf.home_id = pq->home_id;
		pmem->conf.sn = sn;
		pmem->conf.flag = 0;
		log_debug("%s sn=%"PRIu64" homeid=%u i=%d\n", 
			__FUNCTION__, pmem->conf.sn, pmem->conf.home_id, i);
		//如果设备密码全为0，表示是联动设备，要用家庭密码
		if (mem_is_all_zero(pq->dev[i].dev_passwd, sizeof(pq->dev[i].dev_passwd))) {
			log_debug("%"PRIu64" user home passwd\n", pmem->conf.sn);
			pmem->conf.is_la = 1;
			memcpy((void *)pmem->conf.dev_passwd, (void *)pq->passwd, sizeof(pmem->conf.dev_passwd));
			//memdumpone("pmem->conf.dev_passwd", pmem->conf.dev_passwd, 16);
		} else {
			log_debug("%"PRIu64" user ourself passwd\n", pmem->conf.sn);
			pmem->conf.is_la = 0;
			memcpy((void *)pmem->conf.dev_passwd, (void *)pq->dev[i].dev_passwd, sizeof(pmem->conf.dev_passwd));
			//memdumpone("pmem->conf.dev_passwd", pmem->conf.dev_passwd, 16);
		}

		puser = user_lookup_by_sn(pmem->conf.sn);
		if (!puser) {
			la_user_login(phome->conf.home_id, pmem->conf.sn, pmem->conf.dev_passwd);
		}else if (memcmp(puser->passwd_md5, pmem->conf.dev_passwd, APP_USER_UUID_NAME_LEN)) {
			memcpy(puser->passwd_md5, pmem->conf.dev_passwd, APP_USER_UUID_NAME_LEN);
			puser->home_id = pmem->conf.home_id;
			//这里判断一下，如果设备处于err状态，则触发登陆一下,用disp可能会跑服务器去认证
			if (puser->status != CS_ESTABLISH) {
				user_set_status(puser, CS_IDLE);	
			}
		} else {
			puser->home_id = pmem->conf.home_id;
		}
	}
	//将有标记的设备删除，表示是删除成员列表里的设备
	ucla_do_del_mem(phome);
	ucla_user_conf_save(s);

	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
}

static void do_ucla_home_adddev_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	la_member_t *pmem = NULL;
	uc_home_conf_adddev_r_t *pq = NULL;
	int i;
	bool changed = false;

	UCLA_ENTER();	
	pq = get_net_ucp_payload(hdr, uc_home_conf_adddev_r_t);
	pq->home_id = htonl(pq->home_id);
	phome = ucla_find_home_by_id(pq->home_id);
	if (!phome) {
		log_debug("ucla_find_home_by_id failed home_id=%u\n", pq->home_id);
		return;
	}
	if (pq->okcount == 0) {
		log_debug("home_id=%u add err pq->okcount=%u s->adddev_sn=%"PRIu64"\n", pq->home_id, pq->okcount, s->adddev_sn);
		nd_la_debug(s, "home_id=%u add err pq->okcount=%u s->adddev_sn=%"PRIu64"\n", pq->home_id, pq->okcount, s->adddev_sn);
		//添加失败后就将设备删除掉吧,本地处理
		if (s->adddev_sn) {
			ucla_mem_del(phome, s->adddev_sn);
			ucla_home_mem_logout(phome->conf.home_id, s->adddev_sn);
			ucla_user_conf_save(s);
		}
		ucla_home_event_push(pq->home_id, LA_HOME_ADDDEV_FAILED);
		ucla_home_event_push(pq->home_id, LA_HOME_CHANGED);
		goto end;
	}
	for(i = 0; i < pq->okcount; i++) {
		pmem = ucla_find_member_by_sn_from_home(phome, ntoh_ll(pq->sn[i]));
		if (!pmem) {
			continue;
		}
		pmem->conf.flag = 0;
		pmem->add_num = 0;
		changed = true;
	}
	if (changed) {
		// TODO:通知一下app，设备添加成功
		ucla_user_conf_save(s);
		ucla_home_event_push(pq->home_id, LA_HOME_ADDDEV_CHANGED);
		ucla_home_event_push(pq->home_id, LA_HOME_CHANGED);
		log_debug("do_ucla_home_adddev_proc dev sn=%"PRIu64" add successed pq->okcount=%u\n", ntoh_ll(pq->sn[0]), pq->okcount);
		nd_la_debug(s, "do_ucla_home_adddev_proc dev sn=%"PRIu64" add successed pq->okcount=%u\n", ntoh_ll(pq->sn[0]), pq->okcount);
	}

end:	
	//添加下一个
	s->adddev_sn = 0;
	la_comm_timer_proc_reset();
}

static void do_ucla_home_move_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
//	la_member_t *pmem = NULL;
	uc_home_conf_move_r_t *pm = NULL;

	UCLA_ENTER();	
	pm = get_net_ucp_payload(hdr, uc_home_conf_move_r_t);

	pm->src_home_id = htonl(pm->src_home_id);
	pm->dst_home_id = htonl(pm->dst_home_id);

	//如果添加出错，就给个事件
	if (pm->count == 0) {
		log_debug("homeid=%u move to homeid=%u failed\n", pm->src_home_id, pm->dst_home_id);
		ucla_home_event_push(pm->src_home_id, LA_HOME_CONF_DEV_MOVE_FAILED);
		return;
	}
	
	phome = ucla_find_home_by_id(pm->src_home_id);
	if (!phome) {
		log_debug("ucla_find_home_by_id failed home_id=%u\n", pm->src_home_id);
		return;
	}

	phome = ucla_find_home_by_id(pm->dst_home_id);
	if (!phome) {
		log_debug("ucla_find_home_by_id failed home_id=%u\n", pm->dst_home_id);
		return;
	}

	// TODO:移动成功就去查询两个圈子，先查src，再查dst
	do_ucla_home_query(s, pm->src_home_id);
	do_ucla_home_query(s, pm->dst_home_id);
}

static void do_ucla_home_namemodify_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_home_conf_namemodify_r_t *pm = NULL;

	UCLA_ENTER();	
	pm = get_net_ucp_payload(hdr, uc_home_conf_namemodify_r_t);

	pm->result = htons(pm->result);
	pm->home_id = htonl(pm->home_id);

	//如果修改错误
	if (pm->result != 0) {
		log_debug("homeid=%u name modify failed\n", pm->home_id);
		ucla_home_event_push(pm->home_id, LA_HOME_CONF_NAME_MODIFY_FAILED);
		return;
	}
	
	phome = ucla_find_home_by_id(pm->home_id);
	if (!phome) {
		log_debug("ucla_find_home_by_id failed home_id=%u\n", pm->home_id);
		return;
	}

	memcpy((void *)phome->conf.home_name, s->home_name, sizeof(phome->conf.home_name));
	ucla_user_conf_save(s);
	ucla_home_event_push(pm->home_id, LA_HOME_CHANGED);
}

static void do_ucla_home_removedev_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i;
	la_home_t *phome = NULL;
	uc_home_conf_removedev_r_t *pqr = NULL;
	bool changed = false;

	UCLA_ENTER();	
	pqr = get_net_ucp_payload(hdr, uc_home_conf_removedev_r_t);

	pqr->home_id = htonl(pqr->home_id);
	phome = ucla_find_home_by_id(pqr->home_id);
	if (!phome) {
		log_debug("ucla_find_home_by_id failed home_id=%u\n", pqr->home_id);
		return;
	}
	log_debug("%s pqr->okcount=%u\n", __FUNCTION__, pqr->okcount);
	//如果删除出错，就给个事件
	if (pqr->okcount == 0) {
		ucla_home_event_push(pqr->home_id, LA_HOME_REMOVEDEV_FAILED);
		return;
	}
	for(i = 0; i < pqr->okcount; i++) {
		pqr->sn[i] = ntoh_ll(pqr->sn[i]);
		ucla_mem_del(phome, pqr->sn[i]);
		ucla_home_mem_logout(phome->conf.home_id, pqr->sn[i]);
		log_debug("%s del mem sn=%"PRIu64"\n", __FUNCTION__, pqr->sn[i]);
		changed = true;
	}
	if (changed) {
		ucla_user_conf_save(s);
		// TODO:通知一下app，设备添加成功
		ucla_home_event_push(pqr->home_id, LA_HOME_ADDDEV_CHANGED);
	}
}

static void _ucla_home_event_need_query(ucla_session_t *s, u_int32_t home_id)
{
	pkt_t *pkt;
	uc_home_conf_query_t *pq;

	UCLA_ENTER();	
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_home_conf_query_t), 
					true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	pq = get_uascp_payload(pkt, uc_home_conf_query_t);
	pq->aciton = UCAU_QUERY;
	pq->home_id = htonl(home_id);
	ucla_request_add(s, pkt);
}

static void ucla_home_update_all_mem_passwd(la_home_t *phome)
{
	la_member_t *pmem, *memn;
	user_t *puser = NULL;

	stlc_list_for_each_entry_safe(la_member_t, pmem, memn, &phome->member_link, link) {
		if (pmem->conf.is_la) {
			memcpy((void *)pmem->conf.dev_passwd, (void *)phome->conf.home_passwd, APP_USER_UUID_NAME_LEN);
			//只修改密码，等待设备session reset
			puser = user_lookup_by_sn(pmem->conf.sn);
			if (puser) {
				memcpy(puser->passwd_md5, phome->conf.home_passwd, APP_USER_UUID_NAME_LEN);
				//这里判断一下，如果设备处于err状态，则触发登陆一下
				if (puser->status == CS_LOGIN_ERR) {
					user_set_status(puser, CS_DISP);	
				}
			}
		}
	}
}

static void _ucla_home_event_update_passwd(ucla_session_t *s, uc_home_conf_event_t *pe)
{
	la_home_t *phome = NULL;

	UCLA_ENTER();	
	phome = ucla_find_home_by_id(pe->home_id);
	if (!phome) {
		log_debug("_ucla_home_event_update_passwd failed home_id=%u\n", pe->home_id);
		return;
	}

	if (memcmp((void *)phome->conf.home_passwd, (void *)&pe[1], APP_USER_UUID_NAME_LEN) == 0) {
		return;
	}

	memcpy((void *)phome->conf.home_passwd, (void *)&pe[1], APP_USER_UUID_NAME_LEN);
	ucla_home_update_all_mem_passwd(phome);
	//ucla_home_modify_all_login_passwd(pe);
	ucla_user_conf_save(s);
}

static void do_ucla_home_modify_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	la_member_t *pmem = NULL;
	la_member_t *ppassmem = NULL;
	uc_home_conf_modify_passwd_dev_r_t *pq = NULL;
	int i;
	bool changed = false;
	user_t *user;

	UCLA_ENTER();
	la_comm_timer_proc_reset();
	pq = get_net_ucp_payload(hdr, uc_home_conf_modify_passwd_dev_r_t);

	pq->home_id = htonl(pq->home_id);
	phome = ucla_find_home_by_id(pq->home_id);
	if (!phome) {
		log_debug("do_ucla_home_modify_proc failed home_id=%u\n", pq->home_id);
		return;
	}

	//如果修改出错，就给个事件
	if (pq->okcount == 0) {
		log_debug("sn=%"PRIu64" modify passwd err home_id=%u add err pq->okcount=%u\n", 
			s->modify_passwd_sn, pq->home_id, pq->okcount);
		user = user_lookup_by_sn(s->modify_passwd_sn);
		if (user) {
			event_push_err(user->callback, UE_MODIFY_PASSWD_FAIL, user->handle, user->callback_handle, pq->home_id);
		}
		ppassmem = la_dev_pass_sync_find_by_sn(s->modify_passwd_sn);
		if (ppassmem) {
			stlc_list_del(&ppassmem->link);
			cl_free(ppassmem);
			plc->need_dev_pass_sync_save = true;
		}
		s->modify_passwd_sn = 0;
		return;
	}

	for(i = 0; i < pq->okcount; i++) {
		pq->sn[i] = ntoh_ll(pq->sn[i]);
		ppassmem = la_dev_pass_sync_find_by_sn(pq->sn[i]);
		if (ppassmem) {
			stlc_list_del(&ppassmem->link);
			cl_free(ppassmem);
			plc->need_dev_pass_sync_save = true;
		}
		pmem = ucla_find_member_by_sn(s, pq->sn[i]);
		if (!pmem) {
			continue;
		}
		user = user_lookup_by_sn(pq->sn[i]);
		if (!user) {
			continue;
		}
		memcpy((void *)pmem->conf.dev_passwd, user->new_passwd_md5, 16);
		memcpy((void *)user->passwd_md5, user->new_passwd_md5, 16);
		event_push_err(user->callback, UE_MODIFY_PASSWD_OK, user->handle, user->callback_handle, pq->home_id);
		changed = true;
	}

	if (changed) {
		ucla_user_conf_save(s);
		ucla_home_event_push(pq->home_id, LA_HOME_CHANGED);
		log_debug("do_ucla_home_modify_proc dev add successed pq->okcount=%u\n", pq->okcount);
	}
	s->modify_passwd_sn = 0;
}

static void do_ucla_home_sc_add_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	la_rule_t *prule = NULL;
	la_sc_key_t *psk = NULL;
	uc_home_shortcut_r_t *pscr = NULL;

	UCLA_ENTER();
	pscr = get_net_ucp_payload(hdr, uc_home_shortcut_r_t);
	pscr->result = ntohs(pscr->result);
	pscr->home_id = ntohl(pscr->home_id);
	pscr->rule_id = ntohl(pscr->rule_id);
	if (pscr->rule_id == 0) {
		log_debug("%s err ruleid=0\n", __FUNCTION__);
		goto err;
	}
	if (pscr->result) {
		log_debug("%s err result=%u\n", __FUNCTION__, pscr->result);
		goto err;
	}
	if (!s->ps_tmp) {
		log_debug("%s s->ps_tmp is null\n", __FUNCTION__);
		goto err;
	}
	phome = ucla_find_home_by_id(pscr->home_id);
	if (!phome) {
		log_debug("%s err homeid=%u\n", __FUNCTION__, pscr->home_id);
		goto err;
	}
	prule = ucla_find_rule_by_id(phome, pscr->rule_id);
	if (!prule) {
		prule = ucla_rule_new();
		stlc_list_add_tail(&prule->link, &phome->rule_link);
	}
	if (!prule) {
		log_debug("%s err rule is null\n", __FUNCTION__);
		goto err;
	}
	SAFE_FREE(prule->rule);

	//rule
	prule->enable = s->ps_tmp->enable;
	prule->rule_id = pscr->rule_id;
	prule->rule_len = s->ps_tmp->rule_len;
	if (prule->rule_len) {
		prule->rule = cl_calloc(prule->rule_len, 1);
		if (!prule->rule) {
			prule->rule_len = 0;
		} else {
			memcpy((void *)prule->rule, (void *)s->ps_tmp->rule, prule->rule_len);
		}
	}
	//sc
	psk = &phome->la_sc_key[(s->ps_tmp->index - 1)%LA_SC_A_NUM];
	psk->rule_id = pscr->rule_id;
	psk->valid = true;
	memcpy((void *)psk->name, (void *)s->ps_tmp->name, sizeof(psk->name));
	
	SAFE_FREE(s->ps_tmp);

	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_SHORTCUT_ADD_SUCCESSD);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
	return;
	
err:
	ucla_event_push(LA_SHORTUT_ADD_FAILED);
}

static void do_ucla_home_sc_mod_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	la_sc_key_t *pk = NULL;
	uc_home_shortcut_modify_r_t *psdr = NULL;

	UCLA_ENTER();
	psdr = get_net_ucp_payload(hdr, uc_home_shortcut_modify_r_t);
	psdr->result = ntohs(psdr->result);
	psdr->home_id = ntohl(psdr->home_id);

	if (psdr->result) {
		log_debug("%s err result=%u\n", __FUNCTION__, psdr->result);
		goto err;
	}
	phome = ucla_find_home_by_id(psdr->home_id);
	if (!phome) {
		log_debug("%s err homeid=%u\n", __FUNCTION__, psdr->home_id);
		goto err;
	}
	if ((psdr->index == 0) ||
		(psdr->index > LA_SC_A_NUM)) {
		log_debug("%s err index=%u\n", __FUNCTION__, psdr->index);
		goto err;
	}
	if (psdr->index != s->sc_mod_tmp.index) {
		log_debug("%s err index=%u s->sc_mod_tmp.index=%u\n", 
			__FUNCTION__, psdr->index, s->sc_mod_tmp.index);
	}

	pk = &phome->la_sc_key[psdr->index - 1];
	pk->valid = true;
	strncpy(pk->name, (char *)s->sc_mod_tmp.name, sizeof(pk->name) - 1);

	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_SHORTCUT_MOD_SUCCESSD);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
	return;

err:
	ucla_event_push(LA_SHORTCUT_MOD_FAILED);
}


static void do_ucla_home_sc_del_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_home_shortcut_del_r_t *psdr = NULL;

	UCLA_ENTER();
	psdr = get_net_ucp_payload(hdr, uc_home_shortcut_del_r_t);
	psdr->result = ntohs(psdr->result);
	psdr->home_id = ntohl(psdr->home_id);

	if (psdr->result) {
		log_debug("%s err result=%u\n", __FUNCTION__, psdr->result);
		goto err;
	}
	phome = ucla_find_home_by_id(psdr->home_id);
	if (!phome) {
		log_debug("%s err homeid=%u\n", __FUNCTION__, psdr->home_id);
		goto err;
	}
	if ((psdr->index == 0) ||
		(psdr->index > LA_SC_A_NUM)) {
		log_debug("%s err index=%u\n", __FUNCTION__, psdr->index);
		goto err;
	}

	phome->la_sc_key[psdr->index - 1].valid = false;

	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_SHORTCUT_DEL_SUCCESSD);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
	return;

err:
	ucla_event_push(LA_SHORTCUT_DEL_FAILED);
}

static void do_ucla_home_sc_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i, n;
	la_home_t *phome = NULL;
	sc_info_t *pinfo = NULL;
	la_sc_key_t *pkey = NULL;
	uc_home_shortcut_query_r_t *pqr = NULL;

	UCLA_ENTER();
	pqr = get_net_ucp_payload(hdr, uc_home_shortcut_query_r_t);
	pqr->result = ntohs(pqr->result);
	pqr->home_id = ntohl(pqr->home_id);
	
	if (pqr->result) {
		log_debug("%s %d error result=%u\n" ,
			__FUNCTION__, __LINE__, pqr->result);
		return;
	}
	if (hdr->param_len < sizeof(*pqr)) {
		log_debug("%s %d error palen=%u sizeof(*pqr)=%u\n" ,
			__FUNCTION__, __LINE__, hdr->param_len, sizeof(*pqr));
		return;
	}
	phome = ucla_find_home_by_id(pqr->home_id);
	if (!phome) {
		log_debug("%s %d error homeid=%u\n" ,
			__FUNCTION__, __LINE__, pqr->home_id);
		return;		
	}
	//表示全查，清空上次的
	if (pqr->query_index == 0) {
		memset((void *)phome->la_sc_key, 0, sizeof(phome->la_sc_key));
	}

	n = (int)((hdr->param_len - sizeof(*pqr))/sizeof(sc_info_t));

	log_debug("%s qu_index=%u n=%u\n", __FUNCTION__, pqr->query_index, n);
	for(i = 0; i < n; i++) {
		pinfo = &pqr->sc_info[i];
		//log_debug("%s pinfo->index=%u\n", __FUNCTION__, pinfo->index);
		if ((pinfo->index == 0) || 
			(pinfo->index > LA_SC_A_NUM)) {
			continue;
		}
		pkey = &phome->la_sc_key[pinfo->index - 1];
		pkey->rule_id = ntohl(pinfo->rule_id);
		memcpy((void *)pkey->name, (void *)pinfo->name, sizeof(pkey->name));
		pkey->valid = true;
		//log_debug("shortcut index=%u name=%s ruid=%u\n", pinfo->index, pkey->name, pkey->rule_id);
	}

	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
}

static void do_ucla_home_rule_proc(ucla_session_t *s,ucph_t* hdr)
{
	int len = 0;
	uc_rule_net_t nr;
	int remain_len = 0;
	char *ptmp = NULL;
	la_g_rule_t *pgr = NULL;
	la_rule_t *plrule = NULL;
	uc_rule_net_t *pnrule = NULL;
	uc_link_conf_rulelist_r_t *ptr = NULL;
	
	UCLA_ENTER();
	ptr = get_net_ucp_payload(hdr, uc_link_conf_rulelist_r_t);
	ptr->home_id = htonl(ptr->home_id);
	ptr->total_length = htonl(ptr->total_length);

	pgr = ucla_g_rule_find_by_homeid(ptr->home_id);
	if (!pgr) {
		log_debug("enter %s err homeid=%u\n", __FUNCTION__, ptr->home_id);
		ucla_event_push(LA_S3_QUERY_RULE_FAILED);
		return;
	}
	//直接释放圈子规则
	g_rule_list_free(pgr);

	pnrule = (uc_rule_net_t *)ptr->rulelist;
	remain_len = (int)ptr->total_length;
	ptmp = (char *)pnrule;
	ptmp[remain_len] = 0;
	log_debug("paramlen=%u total=%u hd=%u\n", hdr->param_len, remain_len, sizeof(*ptr));
	while(remain_len > (sizeof(*pnrule) + 1)) {
		ptmp = (char *)&pnrule[1];
		len = (int)strlen(ptmp) + 1;
		//判断下长度
		if (remain_len < (len + (int)sizeof(uc_rule_net_t))) {
			log_debug("%s error len remain_len=%u datalen=%u !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 
				__FUNCTION__, remain_len, (len + (int)sizeof(uc_rule_net_t)));
			break;
		}
		memcpy((void *)&nr, (void *)pnrule, sizeof(nr));
		nr.rule_id = htonl(nr.rule_id);
		nr.last_exec_time = htonl(nr.last_exec_time);
		plrule = ucla_rule_new();
		if (!plrule) {
			log_debug("%s %d calloc failed\n", __FUNCTION__, __LINE__);
			return;
		}
		STLC_INIT_LIST_HEAD(&plrule->link);
		stlc_list_add_tail(&plrule->link, &pgr->rule_list);
		plrule->flag = 0;
		
		remain_len -= len + (int)sizeof(uc_rule_net_t);
		
		plrule->rule_id = nr.rule_id;
		plrule->last_exec_time = nr.last_exec_time;
		plrule->state = nr.state;
		plrule->enable = nr.enable;
		plrule->rule_len = len;
		if (len) {
			plrule->rule = cl_strdup(ptmp);
		} else {
			plrule->rule = cl_strdup("");
		}
		
		log_debug("%s ruleid=%u last_exec_time=%u sate=%u len=%u str=%s remain_len=%u\n", 
			__FUNCTION__, plrule->rule_id, plrule->last_exec_time, plrule->state, plrule->rule_len, 
			plrule->rule, remain_len);
		pnrule = (uc_rule_net_t *)((u_int8_t *)pnrule + len + (int)sizeof(uc_rule_net_t));
	}

	ucla_event_push(LA_S3_QUERY_RULE_SUCCESSD);
}

static void do_ucla_home_lmrt_proc(ucla_session_t *s,ucph_t* hdr)
{
	bool need_query = false;
//	la_home_t *phome = NULL;
	la_g_rule_t *pgr = NULL;
	uc_home_last_rule_time_q_r_t *pr = NULL;

	UCLA_ENTER();
	pr = get_net_ucp_payload(hdr, uc_home_last_rule_time_q_r_t);
	pr->result = ntohs(pr->result);
	pr->home_id = ntohl(pr->home_id);
	pr->last_rule_modify = ntohl(pr->last_rule_modify);

	if (pr->result) {
		log_debug("%s result=%u\n", __FUNCTION__, pr->result);
		ucla_event_push(LA_S3_QUERY_RULE_LAST_MODIFY_TIME_FAILED);
		return;
	}

	pgr = ucla_g_rule_find_by_homeid(pr->home_id);
	if (!pgr) {
		pgr = ucla_g_rule_new();
		if (!pgr) {
			log_err(true, "malloc failed\n");
			return;
		}
		stlc_list_add_tail(&pgr->link, &plc->g_rule_list);
	}

	pgr->home_id = pr->home_id;
	if (pgr->last_modify_time != pr->last_rule_modify) {
		pgr->last_modify_time = pr->last_rule_modify;
		need_query = true;
	}

	if (need_query) {
		la_rule_query(s, pr->home_id, 0);
	}

	log_debug("%s homeid=%u pgr->last_modify_time=%u \n", 
		__FUNCTION__, pgr->home_id, pgr->last_modify_time);
	ucla_event_push(LA_S3_QUERY_RULE_LAST_MODIFY_TIMER_SUCCESSD);
}

static void do_ucla_home_conf_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	//log_debug("do_ucla_home_conf_proc action=%u\n", *paction);
	switch(*paction) {
	case UCAU_CREATE:
		do_ucla_home_create_proc(s, hdr);
		break;
	case UCAU_DELETE:
		do_ucla_home_del_proc(s, hdr);
		break;
	case UCAU_ADDDEV:
		do_ucla_home_adddev_proc(s, hdr);
		break;
	case UCAU_REMOVEDEV:
		do_ucla_home_removedev_proc(s, hdr);
		break;
	case UCAU_QUERY:
		do_ucla_home_query_proc(s, hdr);
		break;
	case UCAU_MOVE:
		do_ucla_home_move_proc(s, hdr);
		break;
	case UCAU_HOME_NAME_MODIFY:
		do_ucla_home_namemodify_proc(s, hdr);		
		break;
	case UCAU_EVENT:
		//do_ucla_home_event_proc(s, hdr);
		break;
	case UCAU_MODIFY:
		do_ucla_home_modify_proc(s, hdr);
		break;
	case UCAU_HOME_SHORTCUT:
		do_ucla_home_sc_add_proc(s, hdr);
		break;
	case UCAU_HOME_SHORTCUTDEL:
		do_ucla_home_sc_del_proc(s, hdr);
		break;
	case UCAU_HOME_SHORTQUERY:
		do_ucla_home_sc_query_proc(s, hdr);
		break;
	case UCAU_HOME_SHORTCUTMODIFY:
		do_ucla_home_sc_mod_proc(s, hdr);
		break;
	case UCAU_RULELIST:
		do_ucla_home_rule_proc(s, hdr);
		break;
	case UCAU_HOME_LAST_RULE_TIME:
		do_ucla_home_lmrt_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void do_ucla_template_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i, len;
	uc_link_conf_template_r_t *pt = NULL;
	int remain_len = (int)hdr->param_len;
	char *purl = NULL;
	
	pt = get_net_ucp_payload(hdr, uc_link_conf_template_r_t);
	pt->home_id = htonl(pt->home_id);

	UCLA_ENTER();	
	//先提前释放
	la_misc_template_free();
	if (pt->count == 0) {
		log_debug("count=%u pt->home_id=%u \n", pt->count, pt->home_id);
		return;
	}

	remain_len -= (int)sizeof(uc_link_conf_template_r_t);
	plc->url_array = cl_calloc((pt->count*sizeof(u_int8_t *)), 1);
	if (!plc->url_array) {
		log_debug("calloc failed\n");
		return;
	}
	purl = (char *)pt->url;
	if (remain_len) {
		purl[remain_len-1] = 0;
	}
	for(i = 0; (remain_len > 0) && (i < pt->count); i++) {
		plc->url_array[i] = (u_int8_t *)cl_strdup(purl);
		len = (int)strlen(purl)+1;
		if (len > remain_len) {
			log_debug("%s err len\n", __FUNCTION__);
			break;
		}
		remain_len -= len;
		purl += len;
	}

	plc->url_num = i;

	ucla_user_conf_save(s);
	ucla_event_push(LA_USER_CHANGED);
}

static void do_ucla_capfile_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i, len;
//	la_home_t *phome = NULL;
	uc_link_conf_cap_r_t *pt = NULL;
	int remain_len = (int)hdr->param_len;
	char *purl = NULL;
	
	pt = get_net_ucp_payload(hdr, uc_link_conf_cap_r_t);
	UCLA_ENTER();
	log_debug("enter do_ucla_capfile_proc pt->count=%u parlen=%u\n", pt->count, hdr->param_len);
	if (pt->count == 0) {
		log_debug("count=%u \n", pt->count);
		return;
	}
	remain_len -= (int)sizeof(uc_link_conf_cap_r_t);
	la_misc_capfile_free();
	plc->cap_array = cl_calloc((pt->count*sizeof(u_int8_t *)), 1);
	if (!plc->cap_array) {
		log_debug("calloc failed\n");
		return;
	}

	purl = (char *)pt->url;
	if (remain_len) {
		purl[remain_len-1] = 0;
	}
	for(i = 0; (remain_len > 0) && (i < pt->count); i++) {
		plc->cap_array[i] = (u_int8_t *)cl_strdup(purl);
		len = (int)strlen(purl)+1;
		if (len > remain_len) {
			log_debug("%s err len\n", __FUNCTION__);
			break;
		}
		remain_len -= len;
		purl += len;
	}

	plc->cap_num = i;
	ucla_event_push(LA_CAP_FILE_CHANGED);
	ucla_user_conf_save(s);
}

static void do_ucla_custom_capfile_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i, len;
//	la_home_t *phome = NULL;
	uc_link_conf_cap_r_t *pt = NULL;
	int remain_len = (int)hdr->param_len;
	char *purl = NULL;
	
	pt = get_net_ucp_payload(hdr, uc_link_conf_cap_r_t);
	UCLA_ENTER();
	log_debug("enter do_ucla_capfile_proc pt->count=%u parlen=%u\n", pt->count, hdr->param_len);
	if (pt->count == 0) {
		log_debug("count=%u \n", pt->count);
		return;
	}
	remain_len -= (int)sizeof(uc_link_conf_cap_r_t);
	la_misc_custom_capfile_free();
	plc->cap_custom_array = cl_calloc((pt->count*sizeof(u_int8_t *)), 1);
	if (!plc->cap_custom_array) {
		log_debug("calloc failed\n");
		return;
	}
	purl = (char *)pt->url;
	if (remain_len) {
		purl[remain_len-1] = 0;
	}
	for(i = 0; (remain_len > 0) && (i < pt->count); i++) {
		plc->cap_custom_array[i] = (u_int8_t *)cl_strdup(purl);
		len = (int)strlen(purl)+1;
		if (len > remain_len) {
			log_debug("%s err len\n", __FUNCTION__);
			break;
		}
		remain_len -= len;
		purl += len;
	}

	plc->cap_custom_num = i;
	ucla_event_push(LA_CAP_FILE_CHANGED);
	ucla_user_conf_save(s);
}

static void do_ucla_home_rule_excute_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_linkage_config_rule_excute2_r_t *pr = NULL;

	UCLA_ENTER();
	pr = get_net_ucp_payload(hdr, uc_linkage_config_rule_excute2_r_t);
	pr->home_id = ntohl(pr->home_id);
	pr->result = ntohs(pr->result);

	if (pr->result) {
		log_debug("%s result=%u\n", __FUNCTION__, pr->result);
		ucla_event_push(LA_LINK_CONF_EXEC_FAILED);
		return;
	}

	ucla_event_push(LA_LINK_CONF_EXEC_SUCCESSED);
}

static void do_rule_set_delflag(la_home_t *phome, u_int8_t flag)
{
	la_rule_t *prule, *rulen;

	stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
		prule->flag = flag;
	}	
}

static void do_rule_del_by_flag(la_home_t *phome, u_int8_t flag)
{
	la_rule_t *prule, *rulen;

	stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
		if (prule->flag != flag) {
			continue;
		}

		stlc_list_del(&prule->link);
		ucla_rule_free(prule);
	}	
}

static void do_ucla_rule_parse(la_home_t *phome, u_int8_t *pdata, int data_len)
{
	int len = 0;
	uc_rule_net_t nr;
	uc_rule_net_t *pnrule = NULL;
	int remain_len = 0;
	char *ptmp = NULL;
	la_rule_t *plrule = NULL;
//	FILE *fp = NULL;
	
	pnrule = (uc_rule_net_t *)pdata;
	remain_len = data_len;
	ptmp = (char *)pnrule;
	ptmp[remain_len] = 0;

	phome->rules_is_cache = false;
	//给规则打上删除标记
	if (plc->is_rule_all_query) {
		do_rule_set_delflag(phome, 1);
	}
	while(remain_len > (sizeof(*pnrule) + 1)) {
		ptmp = (char *)&pnrule[1];
		len = (int)strlen(ptmp) + 1;
		//判断下长度
		if (remain_len < (len + (int)sizeof(uc_rule_net_t))) {
			log_debug("do_ucla_rule_parse error len remain_len=%u datalen=%u !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 
				remain_len, (len + (int)sizeof(uc_rule_net_t)));
			break;
		}
		memcpy((void *)&nr, (void *)pnrule, sizeof(nr));
		nr.rule_id = htonl(nr.rule_id);
		nr.last_exec_time = htonl(nr.last_exec_time);
		//先查找已存在的
		plrule = ucla_find_rule_by_id(phome, nr.rule_id);
		//没找到就创建一个
		if (!plrule) {
			plrule = cl_calloc(sizeof(la_rule_t), 1);
			if (!plrule) {
				log_debug("%s %d calloc failed\n", __FUNCTION__, __LINE__);
				return;
			}
			STLC_INIT_LIST_HEAD(&plrule->link);
			stlc_list_add_tail(&plrule->link, &phome->rule_link);
		}
		plrule->flag = 0;
		
		remain_len -= len + (int)sizeof(uc_rule_net_t);
		
		plrule->rule_id = nr.rule_id;
		plrule->last_exec_time = nr.last_exec_time;
		plrule->state = nr.state;
		plrule->enable = nr.enable;
		plrule->rule_len = len;
		SAFE_FREE(plrule->rule);
		if (len) {
			plrule->rule = cl_strdup(ptmp);
		}
		
		log_debug("do_ucla_rule_parse ruleid=%u last_exec_time=%u sate=%u len=%u str=%s remain_len=%u\n", 
			plrule->rule_id, plrule->last_exec_time, plrule->state, plrule->rule_len, plrule->rule, remain_len);
		pnrule = (uc_rule_net_t *)((u_int8_t *)pnrule + len + (int)sizeof(uc_rule_net_t));
		//log_debug("do_ucla_rule_parse strlen=%u\n", strlen(plrule->rule));
		#if 0
		sprintf(name, "%s/ruid-%d", cl_priv->dir, plrule->rule_id);
		fp = fopen(name, "wb");
		if (fp) {
			fwrite(plrule->rule, plrule->rule_len, 1, fp);
			fclose(fp);
		}
		#endif
	}
	if (plc->is_rule_all_query) {
		do_rule_del_by_flag(phome, 1);
	}
	plc->is_rule_all_query = false;
}

static void do_ucla_rulelist_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_link_conf_rulelist_r_t *ptr = NULL;
//	uc_rule_net_t *pnrule = NULL;
	int remain_len = (int)hdr->param_len;
//	char *ptmp = NULL;
//	la_rule_t *plrule = NULL;

	UCLA_ENTER();
	ptr = get_net_ucp_payload(hdr, uc_link_conf_rulelist_r_t);
	ptr->home_id = htonl(ptr->home_id);
	ptr->total_length = htonl(ptr->total_length);

	log_debug("**** total_seq=%u total_length=%u count=%u param_len=%u", 
		ptr->total_seq, ptr->total_length, ptr->count, hdr->param_len);
	phome = ucla_find_home_by_id(ptr->home_id);
	if (!phome) {
		log_debug("%s not find home=%u\n", __FUNCTION__, ptr->home_id);
		return;
	}
	if (ptr->count == 0 ||
		ptr->total_seq == 0) {
		phome->rules_is_cache = false;
		log_debug("%s count=0 or totalseq=0\n", __FUNCTION__);
		return;
	}
	//分片处理,这里要特别注意的是，如果发现buff长度没变，就不要清空了，因为可能是app多次查询的多次返回，但服务器分片已经放到发送队列里了。
	if (ptr->total_seq > 1) {
		if ((phome->last_total_len_offset != 0) && 
			(phome->total_length == ptr->total_length)) {
			log_debug("calloc %u mem already\n", ptr->total_length);
			return;
		}
		SAFE_FREE(phome->ptotal_rule);
		phome->ptotal_rule = cl_calloc(ptr->total_length + 10, 1);
		if (!phome->ptotal_rule) {
			log_err(true, "calloc failed totallen=%u\n", ptr->total_length);
			return;
		}
		phome->total_length = ptr->total_length;
		phome->last_total_len_offset = 0;
		log_debug("pre calloc %u mem\n", ptr->total_length);
		return;
	}

	if (remain_len < (int)(ptr->total_length + sizeof(*ptr))) {
		log_debug("err len remlen=%d ptr->total_length=%u hd=%u\n", 
			remain_len, ptr->total_length, sizeof(*ptr));
		return;
	}


	log_debug("%s count=%u homeid=%u len=%u\n", 
		__FUNCTION__, ptr->count, ptr->home_id, remain_len);

	do_ucla_rule_parse(phome, (u_int8_t *)ptr->rulelist, (int)ptr->total_length);
	ucla_user_conf_save(s);
	ucla_home_event_push(ptr->home_id, LA_HOME_CHANGED);
}

static void do_ucla_rulelist_add_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_config_rule_add_r_t *prr = NULL;
	la_rule_t *plrule = NULL;

	UCLA_ENTER();
	prr = get_net_ucp_payload(hdr, uc_linkage_config_rule_add_r_t);
	prr->home_id = htonl(prr->home_id);
	prr->result = htons(prr->result);
	prr->rule_id = htonl(prr->rule_id);
	
	if (prr->result != 0) {
		log_debug("%s result=%u\n", __FUNCTION__, prr->result);
		return;
	}

	if (!s->rule_tmp) {
		log_debug("s->ruletmp is null\n");
		return;
	}

	phome = ucla_find_home_by_id(prr->home_id);
	if (!phome) {
		log_debug("%s not found id=%u\n", __FUNCTION__, prr->home_id);
		return;
	}

	plrule = ucla_find_rule_by_id(phome, prr->rule_id);
	//没找到就生成一个
	if (!plrule) {
		plrule = cl_calloc(sizeof(la_rule_t), 1);
		if (!plrule) {
			log_debug("%s %d calloc failed\n", __FUNCTION__, __LINE__);
			return;
		}
		STLC_INIT_LIST_HEAD(&plrule->link);
		stlc_list_add_tail(&plrule->link, &phome->rule_link);
	}
	
	plrule->rule_id = prr->rule_id;
	plrule->enable = prr->enable;
	plrule->rule_len = (u_int32_t)strlen((char *)s->rule_tmp) + 1;
	SAFE_FREE(plrule->rule);
	plrule->rule = cl_strdup((char *)s->rule_tmp);
	SAFE_FREE(s->rule_tmp);
	ucla_home_event_push(prr->home_id, LA_HOME_CHANGED);
}

static void do_ucla_rulelist_remove_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_config_rule_del_r_t *prdr = NULL;
	la_rule_t *plrule = NULL;

	prdr = get_net_ucp_payload(hdr, uc_linkage_config_rule_del_r_t);
	prdr->home_id = htonl(prdr->home_id);
	prdr->result = htons(prdr->result);
	prdr->rule_id = htonl(prdr->rule_id);

	UCLA_ENTER();	
	if (prdr->result != 0) {
		log_debug("%s count=%u\n", __FUNCTION__, prdr->result);
		return;
	}

	phome = ucla_find_home_by_id(prdr->home_id);
	if (!phome) {
		log_debug("%s not found id=%u\n", __FUNCTION__, prdr->home_id);
		return;
	}

	plrule = ucla_find_rule_by_id(phome, prdr->rule_id);
	if (plrule) {
		stlc_list_del(&plrule->link);
		ucla_rule_free(plrule);
		ucla_home_event_push(prdr->home_id, LA_HOME_CHANGED);
	}
}

static void do_ucla_rule_modify_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_config_rule_modify_r_t *prdr = NULL;
	la_rule_t *plrule = NULL;

	prdr = get_net_ucp_payload(hdr, uc_linkage_config_rule_modify_r_t);
	prdr->home_id = htonl(prdr->home_id);
	prdr->result = htons(prdr->result);
	prdr->rule_id = htonl(prdr->rule_id);

	UCLA_ENTER();	
	if (prdr->result != 0) {
		log_debug("%s count=%u\n", __FUNCTION__, prdr->result);
		return;
	}

	phome = ucla_find_home_by_id(prdr->home_id);
	if (!phome) {
		log_debug("%s not found id=%u\n", __FUNCTION__, prdr->home_id);
		return;
	}

	plrule = ucla_find_rule_by_id(phome, prdr->rule_id);
	if (plrule) {
		plrule->enable = prdr->enable;
		ucla_home_event_push(prdr->home_id, LA_HOME_CHANGED);
	}
}

static void do_ucla_rule_exec_proc(ucla_session_t *s,ucph_t* hdr)
{
//	la_home_t *phome = NULL;
	uc_linkage_config_rule_exec_r_t *prdr = NULL;
//	la_rule_t *plrule = NULL;

	prdr = get_net_ucp_payload(hdr, uc_linkage_config_rule_exec_r_t);
	prdr->home_id = htonl(prdr->home_id);
	prdr->result = htons(prdr->result);
	prdr->rule_id = htonl(prdr->rule_id);

	UCLA_ENTER();	
	if (prdr->result != 0) {
		log_debug("%s result=%u\n", __FUNCTION__, prdr->result);
		ucla_home_event_push(prdr->home_id, LA_LINK_CONF_EXEC_FAILED);
		return;
	}

	ucla_home_event_push(prdr->home_id, LA_LINK_CONF_EXEC_SUCCESSED);	
}


static void do_ucla_rulelist_event_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_config_event_t *pe = NULL;
	la_rule_t *plrule = NULL;

	UCLA_ENTER();
	pe = get_net_ucp_payload(hdr, uc_linkage_config_event_t);
	pe->home_id = htonl(pe->home_id);
	pe->rule_id = htonl(pe->rule_id);

	log_debug("enter %s homeid=%u ruleid=%u\n", __FUNCTION__, pe->home_id, pe->rule_id);

	phome = ucla_find_home_by_id(pe->home_id);
	if (!phome) {
		log_debug("%s not found id=%u\n", __FUNCTION__, pe->home_id);
		return;
	}

	plrule = ucla_find_rule_by_id(phome, pe->rule_id);
	if (plrule) {
		// TODO:事件这里不知道做些什么，后面再说
		
	}	
}

static void do_ucla_link_conf_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	switch(*paction) {
	case UCAU_TEMPLATE:
		do_ucla_template_query_proc(s, hdr);
		break;
	case UCAU_RULELIST:
		do_ucla_rulelist_query_proc(s, hdr);
		break;
	case UCAU_EXEC:
		do_ucla_rule_exec_proc(s, hdr);
		break;
	case UCAU_RULEADD:
		do_ucla_rulelist_add_proc(s, hdr);
		break;
	case UCAU_RULEREMOVE:
		do_ucla_rulelist_remove_proc(s, hdr);
		break;
	case UCAU_EVENT:
		do_ucla_rulelist_event_proc(s, hdr);
		break;
	case UCAU_MODIFY:
		do_ucla_rule_modify_proc(s, hdr);
		break;
	case UCAU_CAPFILE:
		do_ucla_capfile_proc(s, hdr);
		break;
	case UCAU_CUSTOM_CAPFILE:
		do_ucla_custom_capfile_proc(s, hdr);
		break;
	case UCAU_HOME_RULE_EXCUTE:
		do_ucla_home_rule_excute_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void do_ucla_share_create_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_home_share_create_r_t *pscr = NULL;
	char buf[1024];
	u_int64_t sc = 0;

	UCLA_ENTER();
	pscr = get_net_ucp_payload(hdr, uc_linkage_home_share_create_r_t);

	pscr->home_id = htonl(pscr->home_id);

	phome = ucla_find_home_by_id(pscr->home_id);
	if (!phome) {
		log_debug("%s not found id=%u\n", __FUNCTION__, pscr->home_id);
		return;
	}

	SAFE_FREE(phome->share);

	la_share_2_u64(pscr->share, &sc);
	sprintf(buf, "\"v\":\"1\",\"sc\":\"%"PRIu64"\",\"hid\":\"%u\",\"doname\":\"%s\"", sc, pscr->home_id, s->doname);
	log_debug("buf=%s\n", buf);

	la_json_enc(buf, buf);
	
	phome->share = (u_int8_t *)cl_strdup(buf);

	ucla_home_event_push(pscr->home_id, LA_SHARE_REQ_SUCCESSED);
}

void do_ucla_home_query(ucla_session_t *s, u_int32_t home_id)
{
	pkt_t *pkt;
	uc_home_conf_query_t *pq;

	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_home_conf_query_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	pq = get_uascp_payload(pkt, uc_home_conf_query_t);
	pq->aciton = UCAU_QUERY;
	pq->home_id = htonl(home_id);
	ucla_request_add(s, pkt);
	log_debug("do_ucla_home_query home_id=%u\n", home_id);
}

static void do_ucla_share_register_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_home_share_register_r_t *psrr = NULL;
//	u_int8_t *pdata = NULL;

	UCLA_ENTER();
	psrr = get_net_ucp_payload(hdr, uc_linkage_home_share_register_r_t);

	psrr->home_id = htonl(psrr->home_id);
	psrr->result = htons(psrr->result);
	if (psrr->result != 0) {
		log_debug("%s register failed psrr->result=%u\n", __FUNCTION__, psrr->result);
		ucla_event_push(LA_HOME_SHARE_REGISTOR_FAILED);
		return;
	}

	//注册成功后，生成一个圈子
	phome = ucla_find_home_by_id(psrr->home_id);
	if (phome) {
		log_debug("%s had register already\n", __FUNCTION__);
		ucla_home_event_push(psrr->home_id, LA_HOME_SHARE_REGISTER_SUCCESSED);
		return;
	}

	log_debug("%s psrr->home_id=%u need create home now\n", __FUNCTION__, psrr->home_id);
	phome = ucla_home_new();
	phome->conf.home_id = psrr->home_id;
	ucla_home_add(s, phome);
	plc->has_any_home = true;
	ucla_home_event_push(psrr->home_id, LA_USER_CHANGED);
	ucla_home_event_push(psrr->home_id, LA_HOME_SHARE_REGISTER_SUCCESSED);
	
	//注册成功后查询一下
	do_ucla_home_query(s, psrr->home_id);
	do_ucla_share_query(s, psrr->home_id);
	if (plc->support_tabel) {
		do_ucla_query_label_info(s, psrr->home_id, 0);
	}
	if (plc->support_shortcut) {
		do_ucla_query_shortcut_info(s, psrr->home_id, 0);
	}
	
}

void do_ucla_def_home_sync(ucla_session_t *s)
{
	int i;
	la_user_t *puser;
	la_home_t *phome, *phomen;
	la_share_desc_t *pdesc;

	if (ucla_get_def_home()) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		log_debug("%s not found user id=%u\n", 
			__FUNCTION__, s->user_id);
		return;
	}
	//设置第一个找到的自己创建的圈子为默认圈子
	stlc_list_for_each_entry_safe(la_home_t, phome, 
		phomen, &puser->home_link, link) {
		for(i = 0; i < phome->share_desc_num; i++) {
			pdesc = &phome->share_desc_array[i];
			if (s->user_id == pdesc->user_id) {
				ucla_set_def_home(s, phome);
				ucla_user_conf_save(s);
				return;
			}
		}
	}
}

static void do_ucla_share_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i, len;
	la_home_t *phome = NULL;
	uc_linkage_home_share_query_r_t *psqr = NULL;
	share_query_item_t it;
	share_query_item_t *pitem = NULL;
	u_int8_t *pdata = NULL;
	int remain_len = (int)hdr->param_len;
	bool is_in_share = false;

	UCLA_ENTER();	
	psqr = get_net_ucp_payload(hdr, uc_linkage_home_share_query_r_t);
	psqr->home_id = htonl(psqr->home_id);
	//专门处理强用户创建后跟服务器同步userid用的,否则返回成功事件太早分享userid关系变了会搞挂app
	if (s->need_map_sync) {
		if (s->map_cal > psqr->home_id) {
			s->map_cal -= psqr->home_id;
		} else {
			s->map_cal = 0;
		}
		if (s->map_cal == 0) {
			s->need_map_sync = false;
			//这里判断下是否都收到了,在后面发事件
			log_debug("all_server_create_ok now !!!!!!!!!!!!!!!!!!!!!!!\n");
			ucla_event_push(LA_PHONE_CREATE_SUCCESS);
		}
	}
	if (psqr->count == 0) {
		log_debug("%s register failed\n", __FUNCTION__);
		return;
	}
	pdata = (u_int8_t *)psqr;
	pdata[remain_len-1] = 0;
	phome = ucla_find_home_by_id(psqr->home_id);
	if (!phome) {
		log_debug("%s not found id=%u\n", 
			__FUNCTION__, psqr->home_id);
		return;
	}
	SAFE_FREE(phome->share_desc_array);
	phome->share_desc_num = 0;
	log_debug("%s count=%u remain_len=%u\n", 
		__FUNCTION__, psqr->count, remain_len);
	phome->share_desc_array = cl_calloc((psqr->count*sizeof(la_share_desc_t)), 1);
	if (!phome->share_desc_array) {
		log_debug("%s calloc failed\n", __FUNCTION__);
		return;
	}
	pitem = (share_query_item_t *)&psqr[1];
	for(i = 0; (i < psqr->count) && 
		(remain_len > (int)sizeof(share_query_item_t)); i++) {
		memcpy((void *)&it, (void *)pitem, sizeof(it));
		phome->share_desc_array[i].user_id = htonl(it.user_id);
		phome->share_desc_array[i].join_time = htonl(it.join_time);
		phome->share_desc_array[i].lastuse_time = htonl(it.lastuse_time);
		phome->share_desc_array[i].role_id = it.role_id;
		strncpy((char *)phome->share_desc_array[i].desc, (char *)pitem->desc, 
			sizeof(phome->share_desc_array[i].desc)-1);
		len = (int)sizeof(*pitem) + (int)strlen((char *)pitem->desc) + 1;
		log_debug("userid=%u jontime=%u lasusertime=%08x desc=%s len=%u\n", 
			phome->share_desc_array[i].user_id, 
			phome->share_desc_array[i].join_time,
			phome->share_desc_array[i].lastuse_time, 
			phome->share_desc_array[i].desc, len);
		if (len > remain_len) {
			log_debug("%s err len\n", __FUNCTION__);
			break;
		}
		remain_len -= len;
		pitem = (share_query_item_t *)((u_int8_t *)pitem + len);
		//判断下自己是否在分享列表中
		if (phome->share_desc_array[i].user_id == s->user_id) {
			is_in_share = true;
		}
	}
	phome->share_desc_num = i;
	//这里判断一下，该用户是否在分享里，如果没有，可能是在app关闭情况下被别人踢了，要自己删除下圈子
	if (!is_in_share) {
		log_debug("%s usrid=%u is not in homeid=%u share\n", 
			__FUNCTION__, s->user_id, psqr->home_id);
		ucla_home_del2(s, phome);
		ucla_event_push(LA_USER_CHANGED);
		return;
	}

	//这里做一下默认圈子同步处理，其实默认圈子的作用只是希望用户下至少有一个圈子，所以默认圈子是可以改变的。。。
	do_ucla_def_home_sync(s);
	ucla_user_conf_save(s);
	//排序下,现在不排序，用服务器排的序
#if 0	
	if (psqr->count) {
		qsort(phome->share_desc_array, psqr->count, sizeof(la_share_desc_t), QShareCmp);
	}
#endif	
	ucla_home_event_push(psqr->home_id, LA_HOME_CHANGED);
}

static void do_ucla_share_edit_proc(ucla_session_t *s,ucph_t* hdr)
{
//	la_home_t *phome = NULL;
	uc_linkage_home_share_edit_r_t *pser = NULL;
//	u_int8_t *pdata = NULL;

	UCLA_ENTER();
	pser = get_net_ucp_payload(hdr, uc_linkage_home_share_edit_r_t);

	pser->home_id = htonl(pser->home_id);
	pser->user_id = htonl(pser->user_id);
	pser->result = htons(pser->result);
	if (pser->result != 0) {
		log_debug("%s register failed\n", __FUNCTION__);
		ucla_home_event_push(pser->home_id, LA_HOME_SHARE_DESC_CHANGE_FAILED);
		return;
	}

	// TODO:这里修改成功后不知道成功后需不需要去查询一下。。。
	//修改成功后查询一下
	//do_ucla_share_query(s, pser->home_id);
}

static void do_ucla_share_delete_proc(ucla_session_t *s,ucph_t* hdr)
{
//	la_home_t *phome = NULL;
	uc_linkage_home_share_del_r_t *psdr = NULL;
//	u_int8_t *pdata = NULL;

	psdr = get_net_ucp_payload(hdr, uc_linkage_home_share_del_r_t);

	UCLA_ENTER();
	psdr->home_id = htonl(psdr->home_id);
	psdr->user_id = htonl(psdr->user_id);
	psdr->result = htons(psdr->result);
	if (psdr->result != 0) {
		log_debug("%s register failed\n", __FUNCTION__);
		ucla_home_event_push(psdr->home_id, LA_HOME_SHARE_DESC_DELETE_FAILED);
		return;
	}

	// TODO:这里删除成功后不知道成功后需不需要去查询一下。。。
	//删除成功后查询一下
	//do_ucla_share_query(s, psdr->home_id);
}

void do_ucla_share_query(ucla_session_t *s, u_int32_t home_id)
{
	uc_linkage_home_share_query_t *sq = NULL;
	pkt_t *pkt;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_HOME_SHARE, sizeof(uc_linkage_home_share_query_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}

	sq = get_uascp_payload(pkt, uc_linkage_home_share_query_t);
	sq->action = UCAU_QUERY;
	sq->home_id = ntohl(home_id);

	ucla_request_add(s, pkt);
}

static void do_ucla_home_share_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	switch(*paction) {
	case UCAU_CREATE:
		do_ucla_share_create_proc(s, hdr);
		break;
	case UCAU_REGISTER:
		do_ucla_share_register_proc(s, hdr);
		break;
	case UCAU_QUERY:
		do_ucla_share_query_proc(s, hdr);
		break;
	case UCAU_EDIT:
		do_ucla_share_edit_proc(s, hdr);
		break;
	case UCAU_DELETE:
		do_ucla_share_delete_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void ucla_home_set_flag(u_int32_t user_id, u_int8_t flag)
{
	la_user_t *puser;
	la_home_t *h, *hn;
	
	puser = ucla_find_user_by_id(user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, h, hn, &puser->home_link, link) {
		h->del_flag = flag;
	}
}

static void ucla_home_del_by_flag(ucla_session_t *s, u_int32_t user_id, u_int8_t flag)
{
	la_user_t *puser;
	la_home_t *h, *hn;
//	bool save = false;
	
	puser = ucla_find_user_by_id(user_id);
	if (!puser) {
		return;
	}

	if (stlc_list_empty(&puser->home_link)) {
		return;
	}

	stlc_list_for_each_entry_safe(la_home_t, h, hn, &puser->home_link, link) {
		if (h->del_flag != flag) {
			continue;
		}

		ucla_home_del2(s, h);
	}	
}

static void do_ucla_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i;
	la_user_t *puser;
	la_home_t *phome;
	uc_app_user_home_query_r_t *pqr;

	UCLA_ENTER();
	pqr = get_net_ucp_payload(hdr, uc_app_user_home_query_r_t);
	pqr->user_id = htonl(pqr->user_id);
	puser = ucla_find_user_by_id(pqr->user_id);
	if (!puser) {
		log_debug("not found user by id=%u\n", pqr->user_id);
		return;
	}
	log_debug("%s pqr->count=%u\n", __FUNCTION__, pqr->count);
	if (pqr->count == 0) {
		//判断一下，如果发现没有圈子就创建一个
		ucla_create_def_home(s);
		return;
	}
	//给所有该用户下的圈子打上删除标记
	ucla_home_set_flag(s->user_id, 1);
	for(i = 0; i < pqr->count; i++) {
		pqr->home_id[i] = htonl(pqr->home_id[i]);
		if (pqr->home_id[i] == 0) {
			log_debug("%s get err homeid=0 \n", __FUNCTION__);
			continue;
		}
		log_debug("do_ucla_query_proc get homeid=%u\n", pqr->home_id[i]);
		nd_la_debug(s, "do_ucla_query_proc get homeid=%u\n", pqr->home_id[i]);
		//如果本地有，就不需要处理了
		phome = ucla_find_home_by_id(pqr->home_id[i]);
		if (phome) {
			//清除删除圈子上的标记
			phome->del_flag = 0;
			continue;
		}
		phome = ucla_home_new();
		if (!phome) {
			return;
		}
		plc->has_any_home = true;
		phome->conf.home_id = pqr->home_id[i];
		ucla_home_add(s, phome);
	}
	//有删除标记的圈子是多余的，要删除
	ucla_home_del_by_flag(s, s->user_id, 1);
	//有新添的圈子再去查询
	ucla_each_home_query(s);
	ucla_each_share_query(s);
	if (plc->support_tabel) {
		ucla_each_label_query(s);
	}
	if (plc->support_shortcut) {
		ucla_each_home_shortcut_query(s);
	}
	ucla_event_push(LA_USER_CHANGED);
}

static void do_ucla_create_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_app_user_create_ret_t *rd;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_create_ret_t);

	rd->result = htons(rd->result);
	rd->userid = htonl(rd->userid);

	//s->user_id = rd->userid;
	// TODO:判断创建是否成功
	if (rd->result) {
		//失败
		log_debug("enter %s rd->result=%u\n", 
		__FUNCTION__, rd->result);
		s->is_phone_creat_status = PHONE_CREATE_FAILED;
		return;
	}

	s->is_phone_creat_status = PHONE_CREATE_OK;
	s->phone_user_id_tmp = rd->userid;
	log_debug("%s user create successed get userid=%u\n", 
		s->doname, s->phone_user_id_tmp);
}

static void do_ucla_replace_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_app_user_phone_replace_r_t *rd;
	la_user_t *puser;
	la_phone_t *phone;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_phone_replace_r_t);

	rd->result = htons(rd->result);
	rd->user_id = htonl(rd->user_id);
	rd->client_id = htonl(rd->client_id);

	if (rd->result) {
		log_debug("enter %s rd->result=%u\n", 
		__FUNCTION__, rd->result);
		ucla_event_push_err(LA_PHONE_CREATE_FAILED, 0, LA_PHONE_REPLACE_FAILED);
		return;
	}

	//将创建的强用户替换到本用户
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		log_debug("not found user %u from user list\n", rd->user_id);
		ucla_event_push_err(LA_PHONE_CREATE_FAILED, 0, LA_PHONE_REPLACE_FAILED);
		return;
	}

	puser->conf.user_id = rd->user_id;
	s->user_id = rd->user_id;
	memcpy((void *)puser->conf.user_name, (void *)plc->user_name, 
		sizeof(puser->conf.user_name));
	memcpy((void *)puser->conf.passwd, (void *)plc->passwd, 
		sizeof(puser->conf.passwd));
	memset((void *)puser->conf.uuid, 0, sizeof(puser->conf.uuid));

	memcpy((void *)s->username, (void *)plc->user_name, sizeof(s->username));
	memcpy((void *)s->key, (void *)plc->passwd, sizeof(s->key));
	memset((void *)s->uuid, 0, sizeof(puser->conf.uuid));

	s->need_save_username = true;
	//替换sessionid
	_ucla_user_conf_save(s);
	plc->has_phone = true;
	//第一个手机用户创建成功
	phone = ucla_find_phone_by_name((char *)plc->user_name);
	if (!phone) {
		phone = ucla_phone_new();
		if (!phone) {
			return;
		}
		ucla_phone_add(phone);
		la_phone_save();
	}
	ucla_set_cur_phone(phone);
	memcpy((void *)phone->conf.passwd, (void *)plc->passwd, sizeof(phone->conf.passwd));
	memcpy((void *)phone->conf.user_name, (void *)plc->user_name, sizeof(phone->conf.user_name));
	//创建强用户后userid有变化，需要重新去服务器查询下分享信息,原因是app会挂。。。
	ucla_each_share_query_sync(s);
	if (s->map_cal == 0) {
		//这里判断下是否都收到了,在后面发事件
		log_debug("all_server_create_ok now !!!!!!!!!!!!!!!!!!!!!!!\n");
		ucla_event_push(LA_PHONE_CREATE_SUCCESS);
	}
}

static void do_ucla_sever_set_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_app_user_phone_conf_set_r_t *rd;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_phone_conf_set_r_t);

	rd->result = htons(rd->result);
	rd->user_id = htonl(rd->user_id);

	if (rd->result) {
		log_debug("%s failed\n", __FUNCTION__);
		return;
	}

	if (s->need_report_phone_create) {
		s->need_report_phone_create = false;
		log_debug("%s send do_ucla_sever_set_proc and do_phone_create_ok_proc\n", s->doname);
		ucla_event_push(LA_PHONE_CREATE_SUCCESS);
	}

	log_debug("do_ucla_sever_set_proc successed\n");
}

static void do_ucla_app_modify_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_phone_t *phone;
	uc_app_user_phone_modify_r_t *rd;
	la_user_t *puser;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_phone_modify_r_t);

	rd->result = htons(rd->result);
	rd->user_id = htonl(rd->user_id);

	if (rd->result ||
		(rd->user_id != s->user_id)) {
		log_debug("%s failed result=%u rd->user_id=%u s->userid=%u\n", 
			__FUNCTION__, rd->result, rd->user_id, s->user_id);
		return;
	}

	phone = ucla_find_phone_by_name((char *)plc->user_name);
	if (!phone) {
		log_debug("not found phone %s\n", plc->user_name);
		return;
	}

	puser = ucla_find_user_by_id(rd->user_id);
	if (!puser) {
		log_debug("not found user %u\n", rd->user_id);
		return;
	}

	log_debug("do_ucla_app_modify_proc %s modify\n", s->doname);
	memcpy((void *)s->key, (void *)plc->passwd, sizeof(s->key));
	memcpy((void *)phone->conf.passwd, (void *)plc->passwd, sizeof(phone->conf.passwd));
	memcpy((void *)puser->conf.passwd, (void *)plc->passwd, sizeof(puser->conf.passwd));
	//同步保存文件
	la_phone_save();
	//修改密码，需要保存配置。
	s->need_save_username = true;
	_ucla_user_conf_save(s);
	phome_passwd_modify_ok();
	//密码修改后要上传下服务器
	//ucla_phone_need_upload();
}

static void do_ucla_misc_query_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_app_user_misc_query_r_t *rd;
	int remain_len = (int)hdr->param_len;
	bool changed = false;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_misc_query_r_t);
	if (rd->count == 0) {
		log_debug("%s count = 0\n", __FUNCTION__);
		return;
	}
	if (remain_len < (int)(sizeof(uc_app_user_misc_query_r_t) + rd->count*sizeof(u_int32_t))) {
		UCLA_ENTER();
		log_debug("err len\n");
		return;
	}
	if (rd->count > 0) {
		rd->value[0] = htonl(rd->value[0]);
		if (plc->cap_lastmodifytime < rd->value[0]) {
			plc->cap_lastmodifytime = rd->value[0];
			do_comm_cap_query();
			changed = true;
			log_debug("do_ucla_misc_query_proc get cap_lastmodifytime from %s plc->cap_lastmodifytime=%u\n", s->doname, plc->cap_lastmodifytime);
		}
	}
	if (rd->count > 1) {
		rd->value[1] = htonl(rd->value[1]);
		if (plc->last_template_time < rd->value[1]) {
			plc->last_template_time = rd->value[1];
			plc->template_session = s;
			la_template_query();
			log_debug("do_ucla_misc_query_proc get last_template_time from %s\n", s->doname);
			changed = true;
		}
	}
	if (rd->count > 2) {
		rd->value[2] = htonl(rd->value[2]);
		if (plc->cap_custom_lastmodifytime < rd->value[2]) {
			plc->cap_custom_lastmodifytime = rd->value[2];
			do_comm_cap_custom_query();
			changed = true;
			log_debug("do_ucla_misc_query_proc get cap_custom_lastmodifytime from %s plc->cap_custom_lastmodifytime=%u\n", s->doname, plc->cap_custom_lastmodifytime);
		}
	}
	
	if (changed) {
		ucla_event_push(LA_USER_CHANGED);
	}
}

static void do_ucla_misc_set_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_app_user_misc_set_r_t *rd;
	uc_tlv_t *ptlv;
	//int remain_len = (int)hdr->param_len;
	//bool changed = false;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_misc_set_r_t);
	rd->result = htons(rd->result);
	
	ptlv = (uc_tlv_t *)&rd[1];
	ptlv->type = htons(ptlv->type);
	if (rd->result) {
		log_debug("type=%u failed\n");
	} else {
		log_debug("type=%u successed\n");
	}

	ucla_event_push(LA_USER_CHANGED);
}

static void do_ucla_user_req_create_proc(ucla_session_t *s,ucph_t* hdr)
{
	char buf[1024];
	u_int64_t sc = 0;
	uc_app_user_req_sc_r_t *rd;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_req_sc_r_t);
	rd->result = htons(rd->result);

	if (rd->result) {
		log_info("rd->result=%u\n", rd->result);
		ucla_event_push(LA_USER_SHARE_REQ_FAILED);
		return;
	}

	SAFE_FREE(plc->req_share);
	la_share_2_u64(rd->share, &sc);
	sprintf(buf, "\"v\":\"100\",\"sc\":\"%"PRIu64"\",\"uid\":\"%u\",\"doname\":\"%s\"", sc, s->user_id, s->doname);
	log_info("befor enc buf=%s\n", buf);
	la_json_enc(buf, buf);
	log_info("after enc buf=%s\n", buf);
	
	plc->req_share = cl_strdup(buf);

	ucla_event_push(LA_USER_SHARE_REQ_SUCCESSED);
}

static void do_ucla_user_req_add_proc(ucla_session_t *s,ucph_t* hdr)
{
	uc_app_user_add_home_r_t *rd;

	UCLA_ENTER();
	rd = get_net_ucp_payload(hdr, uc_app_user_add_home_r_t);
	rd->result = htons(rd->result);

	log_info("rd->result=%u\n", rd->result);
	if (rd->result) {
		if (ERR_USER_EXIST == rd->result) {
			ucla_event_push_err(LA_USER_SHARE_ADD_HOME_FILED, 0, LA_APP_USER_REQ_SHRAE_HOME_EXIST);
		} else {
			ucla_event_push(LA_USER_SHARE_ADD_HOME_FILED);
		}
	} else {
		ucla_event_push(LA_USER_SHARE_ADD_HOME_SUCCESSED);
	}
}

static void do_ucla_app_user_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);
	
	switch(*paction) {
	case UCAU_SERVER_QUERY:
		//do_ucla_server_query_proc(s, hdr);
		break;
	case UCAU_QUERY:
		do_ucla_query_proc(s, hdr);
		break;
	case UCAU_CREATE:
		do_ucla_create_proc(s, hdr);
		break;
	case UCAU_SERVER_SET:
		do_ucla_sever_set_proc(s, hdr);
		break;
	case UCAU_USER_REPLACE:
		do_ucla_replace_proc(s, hdr);
		break;
	case UCAU_MODIFY:
		do_ucla_app_modify_proc(s, hdr);
		break;
	case UCAU_MISC_QUERY:
		do_ucla_misc_query_proc(s, hdr);
		break;
	case UCAU_MISC_SET:
		do_ucla_misc_set_proc(s, hdr);
		break;
	case UCAU_USER_REQ_SC:
		do_ucla_user_req_create_proc(s, hdr);
		break;
	case UCAU_USER_ADD_HOME:
		do_ucla_user_req_add_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void do_ucla_home_label_query_proc(ucla_session_t *s, ucph_t* hdr )
{
	int i;
	int len = 0;
	int len1 = 0;
	u_int8_t *pt = NULL;
//	u_int64_t *psn = NULL;
	bool need_save = false;
	la_home_t *phome = NULL;
	la_label_t *plabel = NULL;
	uc_label_query_r_t *pr = NULL;
	uc_label_info_t *pinfo = NULL;
	int remain_len = (int )hdr->param_len;
	
	UCLA_ENTER();
	pr = get_net_ucp_payload(hdr, uc_label_query_r_t);
	pr->result = htons(pr->result);
	pr->home_id = htonl(pr->home_id);
	pr->label_id = htons(pr->label_id);

	if (remain_len < (int)sizeof(*pr)) {
		log_info("err len remain_len=%d sizeof(pr)=%u\n", 
			remain_len, sizeof(*pr));
		return;
	}
	remain_len -= (int)sizeof(*pr);
	if (pr->result) {
		log_info("proc error result=%u \n", pr->result);
		return;
	}
	phome = ucla_find_home_by_id(pr->home_id);
	if (!phome) {
		log_info("not found homeid=%u\n", pr->home_id);
		return;
	}
	//为0表示是全查询，那么为了简单处理就删除下上次的链表
	if (pr->label_id == 0) {
		need_save = ucla_home_label_free(phome);
	}
	//parse
	pt = (u_int8_t *)&pr[1];
	while(remain_len >= (int)sizeof(*pinfo)) {
		pinfo = (uc_label_info_t *)pt;
		pinfo->label_id = ntohs(pinfo->label_id);
		pinfo->sn_count = ntohs(pinfo->sn_count);
		
		len1 = (int)(pinfo->sn_count * sizeof(u_int64_t));
		len = (int)sizeof(*pinfo) + len1;
		log_debug("pinfo->label_id=%u pinfo->sn_count=%u len1=%d len=%d remain_len=%d(int)sizeof(*pinfo)=%d\n", 
			pinfo->label_id, pinfo->sn_count, len1, len, remain_len, (int)sizeof(*pinfo));
		if (remain_len < len) {
			log_info("err len reml=%d len=%d\n", remain_len, len);
			break;
		}
		remain_len -= len;
		//cal next
		pt += len;
		
		plabel = ucla_find_home_label_by_id(phome, pinfo->label_id);
		if (!plabel) {
			plabel = ucla_label_new();
			if (!plabel) {
				log_err(true, "ucla_label_new failed\n");
				return;
			}
			stlc_list_add_tail(&plabel->link, &phome->label_link);
		}
		plabel->conf.id = pinfo->label_id;
		plabel->conf.sn_num = pinfo->sn_count;
		plabel->conf.home_id = phome->conf.home_id;
		memcpy((void *)plabel->conf.name, (void *)pinfo->name, 
			sizeof(plabel->conf.name));
		SAFE_FREE(plabel->p_sn);
		if (len1) {
			plabel->p_sn = cl_calloc(len1, 1);
			if (!plabel->p_sn) {
				log_err(true, "malloc failed len1=%d\n", len1);
				return;
			}
			for(i = 0; i < pinfo->sn_count; i++) {
				plabel->p_sn[i] = ntoh_ll(pinfo->a_sn[i]);
			}
		}
		need_save = true;
	}

	ucla_user_conf_save(s);
	ucla_home_event_push(pr->home_id, LA_HOME_CHANGED);
}

static void do_ucla_home_label_add_proc(ucla_session_t *s, ucph_t* hdr )
{
	la_home_t *phome = NULL;
	la_label_t *plabel = NULL;
	la_label_t *plabel2 = NULL;
	uc_label_add_r_t *par = NULL;

	UCLA_ENTER();
	par = get_net_ucp_payload(hdr, uc_label_add_r_t);
	par->result = ntohs(par->result);
	par->label_id = ntohs(par->label_id);
	par->home_id = ntohl(par->home_id);

	log_debug("par->label_id=%u par->home_id=%u par->result=%u\n", 
		par->label_id, par->home_id, par->result);
	if (par->result) {
		log_info("result = %u failed\n", par->result);
		goto err;
	}
	if (!s->plabel_tmp) {
		log_info("s->plabel_tmp is null\n");
		goto err;
	}
	if (s->plabel_tmp->conf.home_id != par->home_id) {
		log_info("err s->homeid=%u par->homeid=%u\n", 
			s->plabel_tmp->conf.home_id, par->home_id);
		goto err;
	}
	phome = ucla_find_home_by_id(par->home_id);
	if (!phome) {
		log_info("err homeid=%u\n", par->home_id);
		goto err;
	}
	plabel = ucla_find_home_label_by_id(phome, par->label_id);
	if (!plabel) {
		plabel2 = (la_label_t *)stlc_list_entry(&s->plabel_tmp->link, la_label_t, link);
		plabel2->conf.id = par->label_id;
		stlc_list_add_tail(&s->plabel_tmp->link, &phome->label_link);
		s->plabel_tmp = NULL;
	} else {
		SAFE_FREE(plabel->p_sn);
		plabel->p_sn = s->plabel_tmp->p_sn;
		s->plabel_tmp->p_sn = NULL;
		memcpy((void *)&plabel->conf, (void *)&s->plabel_tmp->conf,
			sizeof(plabel->conf));
		ucla_label_free(s->plabel_tmp);
		s->plabel_tmp = NULL;
	}
	
	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_LABEL_ADD_SUCCESS);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
	
	return;
	
err:
	ucla_event_push(LA_LABEL_ADD_FAILED);
}

static void do_ucla_home_label_del_proc(ucla_session_t *s, ucph_t* hdr )
{
	la_home_t *phome = NULL;
	la_label_t *plabel = NULL;
	uc_label_delete_r_t *pdr = NULL;

	UCLA_ENTER();
	pdr = get_net_ucp_payload(hdr, uc_label_delete_r_t);
	pdr->result = ntohs(pdr->result);
	pdr->label_id = ntohs(pdr->label_id);
	pdr->home_id = ntohl(pdr->home_id);

	if (pdr->result) {
		log_info("label del failed result=%u\n", pdr->result);
		goto err;
	}
	phome = ucla_find_home_by_id(pdr->home_id);
	if (!phome) {
		log_info("err homeid=%u\n", pdr->home_id);
		goto err;
	}
	plabel = ucla_find_home_label_by_id(phome, pdr->label_id);
	if (!plabel) {
		log_info("err labelid=%u\n", pdr->label_id);
		goto err;
	}
	stlc_list_del(&plabel->link);
	ucla_label_free(plabel);

	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_LABEL_DEL_SUCCESS);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);

	return;

err:
	ucla_event_push(LA_LABEL_DEL_FAILED);
}

bool ucla_label_find_sn(u_int64_t *psn, u_int16_t num, u_int64_t sn)
{
	int i;

	for(i = 0; i < num; i++) {
		if (psn[i] == sn) {
			return true;
		}
	}

	return false;
}

static void ucla_label_add_sn(la_label_t *plabel, u_int64_t sn)
{
	int len;

	if (ucla_label_find_sn(plabel->p_sn, plabel->conf.sn_num, sn)) {
		return;
	}
	len = (int)((plabel->conf.sn_num + 1)*sizeof(u_int64_t));
	plabel->p_sn = cl_realloc(plabel->p_sn, len);
	if (!plabel->p_sn) {
		plabel->conf.sn_num = 0;
		return;
	}
	plabel->p_sn[plabel->conf.sn_num++] = sn;
}

static void ucla_label_del_sn(la_label_t *plabel, u_int64_t sn)
{
	int len, i ,j;
	u_int64_t *psn_t = NULL;

	if (plabel->conf.sn_num == 0) {
		return;
	}
	if (!ucla_label_find_sn(plabel->p_sn, plabel->conf.sn_num, sn)) {
		return;
	}
	len = (int)(sizeof(u_int64_t)*plabel->conf.sn_num);

	psn_t = cl_calloc(len, 1);
	if (!psn_t) {
		log_err(true, "malloc failed\n");
		return;
	}
	memcpy((void *)psn_t, (void *)plabel->p_sn, len);

	for(i = 0, j = 0; i < plabel->conf.sn_num; i++) {
		if (psn_t[i] != sn) {
			plabel->p_sn[j++] = psn_t[i];
		}
	}

	cl_free(psn_t);
	plabel->conf.sn_num = j;
}

static void ucla_label_bind_sn_del_sync(la_home_t *phome, u_int8_t flag, u_int64_t sn)
{
	la_label_t *pl, *pln;

	stlc_list_for_each_entry_safe(la_label_t, pl, pln, &phome->label_link, link) {
		if (pl->flag != flag) {
			continue;
		}

		ucla_label_del_sn(pl, sn);
	}
}

static void do_ucla_home_label_bind_proc(ucla_session_t *s, ucph_t* hdr )
{
	int i;
	la_home_t *phome = NULL;
	la_label_t *plabel = NULL;
	uc_label_bind_t *pt = NULL;
	uc_label_bind_r_t *pbr = NULL;

	UCLA_ENTER();
	pbr = get_net_ucp_payload(hdr, uc_label_bind_r_t);
	pbr->result = ntohs(pbr->result);
	pbr->home_id = ntohl(pbr->home_id);

	if (pbr->result) {
		log_info("err result=%u\n", pbr->result);
		goto err;
	}
	phome = ucla_find_home_by_id(pbr->home_id);
	if (!phome) {
		log_info("err homeid=%u\n", pbr->home_id);
		goto err;
	}
	if (!s->plbind_tmp) {
		log_info("err s->plbind_tmp is null\n");
		goto err;
	}
	if (s->plbind_tmp->home_id != pbr->home_id) {
		log_info("err homeid tmp->home_id=%u pbr->home_id=%u\n", 
			s->plbind_tmp->home_id, pbr->home_id);
		goto err;
	}

	pt = s->plbind_tmp;
	//给所有标签打上标志，好同步删除
	ucla_label_flag_set(phome, 1);
	for(i = 0; i < pt->label_num; i++) {
		log_debug("pt->label_num=%u i=%d\n", pt->label_num, i);
		plabel = ucla_find_home_label_by_id(phome, pt->a_label_id[i]);
		if (!plabel) {
			continue;
		}
		plabel->flag = 0;
		ucla_label_add_sn(plabel, pt->sn);
	}
	ucla_label_bind_sn_del_sync(phome, 1, pt->sn);
	ucla_label_flag_set(phome, 0);

	SAFE_FREE(s->plbind_tmp);
	ucla_user_conf_save(s);
	ucla_home_event_push(phome->conf.home_id, LA_LABEL_BIND_SUCCESS);
	ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);

	return;

err:
	ucla_event_push(LA_LABEL_BIND_FAILED);
}

static void do_ucla_home_label_proc(ucla_session_t *s, ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	log_debug("enter %s action=%u hdr->param_len=%u\n", 
		__FUNCTION__, *paction, hdr->param_len);
	
	switch(*paction) {
	case UCAU_QUERY:
		do_ucla_home_label_query_proc(s, hdr);
		break;
	case UCAU_CREATE:
		do_ucla_home_label_add_proc(s, hdr);
		break;
	case UCAU_DELETE:
		do_ucla_home_label_del_proc(s, hdr);
		break;
	case UCAU_LABEL_BIND:
		do_ucla_home_label_bind_proc(s, hdr);
		break;
	default:
		break;
	}
	
}

static void do_ucla_home_dict_query_proc(ucla_session_t *s, ucph_t* hdr )
{
	u_int8_t *pkey = NULL, *pvalue = NULL;
	la_home_t *phome = NULL;
	uc_dict_query_r_t *pr = NULL;
	int remain_len = (int )hdr->param_len;
	
	UCLA_ENTER();
	pr = get_net_ucp_payload(hdr, uc_dict_query_r_t);
	pr->result = htons(pr->result);
	pr->home_id = htonl(pr->home_id);
	pr->key_len = htons(pr->key_len);
	pr->value_len = htons(pr->value_len);

	log_debug("home dict query reply: result %u home_id %u key_len %u value_len %u\n", 
		pr->result, pr->home_id, pr->key_len, pr->value_len);


	if ((int)pr->key_len + (int)pr->value_len + (int)sizeof(*pr) > remain_len) {
		log_err(false, "invalid dict param key_len %u value_len %u remain %d\n", pr->key_len, pr->value_len, remain_len);
		return;
	}

	if (pr->key_len == 0 || pr->value_len == 0) {
		log_err(false, "invalid key and value len\n");
		ucla_home_event_push(pr->home_id, LA_DICT_QUERY_FAILED);
		return;
	}

	if (pr->result) {
		log_info("proc error result=%u \n", pr->result);
		ucla_home_event_push(pr->home_id, LA_DICT_QUERY_FAILED);
		return;
	}

	pkey = pr->key_value;
	pvalue = pr->key_value + pr->key_len;

	
	phome = ucla_find_home_by_id(pr->home_id);
	if (!phome) {
		log_info("not found homeid=%u\n", pr->home_id);
		return;
	}

	// parse
	ucla_dict_add_or_modify(phome, pkey, pr->key_len, pvalue, pr->value_len);

	log_debug("query home dict done\n");

	ucla_home_event_push(pr->home_id, LA_DICT_MODIFY);
}

static void do_ucla_home_dict_set_proc(ucla_session_t *s, ucph_t* hdr )
{
	uc_dict_set_r_t *pr = NULL;
	
	UCLA_ENTER();
	
	pr = get_net_ucp_payload(hdr, uc_dict_set_r_t);
	pr->result = htons(pr->result);
	pr->home_id = htonl(pr->home_id);

	log_debug("dict set reply: result %u home_id %u\n", pr->result, pr->home_id);

	if (pr->result) {
		log_info("proc error result=%u \n", pr->result);
		ucla_home_event_push(pr->home_id, LA_DICT_SET_FAILED);
		return;
	}
	
	ucla_home_event_push(pr->home_id, LA_DICT_SET_SUCCESS);
}

static void do_ucla_home_dict_del_proc(ucla_session_t *s, ucph_t* hdr)
{
	uc_dict_del_r_t *pr = NULL;
	
	UCLA_ENTER();
	
	pr = get_net_ucp_payload(hdr, uc_dict_del_r_t);
	pr->home_id = htonl(pr->home_id);

	log_debug("dict del reply: home_id %u key_len %d\n", pr->result, pr->home_id, hdr->param_len - sizeof(*pr));

	ucla_home_event_push(pr->home_id, LA_DICT_DEL_SUCCESS);
}

static void do_ucla_home_dict_proc(ucla_session_t *s, ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	log_debug("enter %s action=%u hdr->param_len=%u\n", 
		__FUNCTION__, *paction, hdr->param_len);
	
	switch(*paction) {
	case UCAU_QUERY:
		do_ucla_home_dict_query_proc(s, hdr);
		break;
	case UCAU_EDIT:
		do_ucla_home_dict_set_proc(s, hdr);
		break;
	case UCAU_DELETE:
		do_ucla_home_dict_del_proc(s, hdr);
		break;
	default:
		break;
	}
	
}

//#define LOG_RULE_NODE_LEN_MIN	(4 + 4 + 2 + 2 + 4 + 4 + 8 + 8)
#define LOG_RULE_NODE_LEN_MIN	(sizeof(u_int32_t)*2 + sizeof(u_int16_t)*2 + sizeof(u_int32_t)*2 + sizeof(u_int64_t)*2)

#define LOG_FLAG_IS_TEST	(BIT(0))
#define LOG_FLAG_IS_TIMER	(BIT(1))

static void do_log_rule_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i, len, sn_len;
	u_int8_t *pdata = NULL;
	u_int16_t monitor_count = 0;
	u_int16_t action_count = 0;
	u_int32_t index;
	u_int32_t flag;
	ucp_log_rule_query_r_t *pr = NULL;
	la_log_home_rule_node_t *pnode = NULL;
	la_log_home_rule_change_t *prl = NULL;
	int remain_len = (int)hdr->param_len;

	if (hdr->param_len < (u_int16_t)sizeof(*pr)) {
		log_err(false, "err len=%u\n", hdr->param_len);
		return;
	}

	pr = get_net_ucp_payload(hdr, ucp_log_rule_query_r_t);
	pr->result = ntohs(pr->result);
	pr->home_id = ntohl(pr->home_id);
	pr->max_index = ntohl(pr->max_index);
	pr->record_count = ntohs(pr->record_count);
	pdata = (u_int8_t *)&pr[1];
	remain_len  -= (int)sizeof(*pr);
	
	log_debug("enter %s home_id=%u remmain_len=%d max_index=%u record_count=%u\n", 
		__FUNCTION__, pr->home_id, remain_len, pr->max_index, pr->record_count);
	if (pr->result) {
		log_err(false, "failed result=%u\n", pr->result);
		return;
	}

	prl = ucla_log_rule_find_by_homeid(pr->home_id);
	if (!prl) {
		prl = ucla_log_rule_new();
		if (!prl) {
			log_err(true, "malloc failed\n");
			return;
		}
		stlc_list_add_tail(&prl->link, &plc->g_rule_log_list);
	}
	prl->max_index = pr->max_index;
	prl->home_id = pr->home_id;
	//直接清空一下
	la_g_rule_node_list_free(prl);

	while(remain_len >= LOG_RULE_NODE_LEN_MIN) {
		len = 0;
		memcpy((void *)&index, pdata, sizeof(index));
		index = ntohl(index);
		len += sizeof(index);
		pdata += sizeof(index);
		
		memcpy((void *)&flag, pdata, sizeof(flag));
		flag = ntohl(flag);
		len += sizeof(flag);
		pdata += sizeof(flag);
		
		memcpy((void *)&monitor_count, pdata, sizeof(monitor_count));
		monitor_count = ntohs(monitor_count);
		len += sizeof(monitor_count);
		pdata += sizeof(monitor_count);

		memcpy((void *)&action_count, pdata, sizeof(action_count));
		action_count = ntohs(action_count);
		len += sizeof(action_count);
		pdata += sizeof(action_count);
		
		sn_len = (int)((monitor_count + action_count) * sizeof(u_int64_t));
		log_debug("remain_len=%d len=%d sn_len=%d monitorc_count=%u action_count=%u index=%u flag=%02x\n", 
			remain_len, len, sn_len, monitor_count, action_count, index, flag);
		len += sn_len;
		//加上usr_id长度和时间戳长度
		len += sizeof(u_int32_t) * 2;
		if (len > remain_len) {
			log_err(false, "err len=%d remainlen=%d\n", len, remain_len);
			break;
		}
		pnode = ucla_log_rule_node_new();
		if (!pnode) {
			log_err(true, "calloc failed\n");
			break;
		}
		stlc_list_add_tail(&pnode->link, &prl->log_list);
		//index
		pnode->log_info.index = index;
		//monitor_sn
		sn_len = (int)(monitor_count * sizeof(u_int64_t));
		pnode->log_info.pmonitor_sn = cl_calloc(sn_len, 1);
		if (!pnode->log_info.pmonitor_sn) {
			log_err(true, "calloc failed\n");
			break;
		}
		pnode->log_info.monitor_count = monitor_count;
		memcpy((void *)pnode->log_info.pmonitor_sn, pdata, sn_len);
		for(i = 0; i < monitor_count; i++) {
			pnode->log_info.pmonitor_sn[i] = ntoh_ll(pnode->log_info.pmonitor_sn[i]);
			log_debug("monitor sn[%d]=%"PRIu64"\n", i, pnode->log_info.pmonitor_sn[i]);
		}
		pdata += sn_len;
		//action_sn
		sn_len = (int)(action_count * sizeof(u_int64_t));
		pnode->log_info.paction_sn = cl_calloc(sn_len, 1);
		if (!pnode->log_info.paction_sn) {
			log_err(true, "calloc failed\n");
			break;
		}
		pnode->log_info.action_count = action_count;
		memcpy((void *)pnode->log_info.paction_sn, pdata, sn_len);
		for(i = 0; i < action_count; i++) {
			pnode->log_info.paction_sn[i] = ntoh_ll(pnode->log_info.paction_sn[i]);
			log_debug("action sn[%d]=%"PRIu64"\n", i, pnode->log_info.paction_sn[i]);
		}
		pdata += sn_len;
		//ruleid
		memcpy((void *)&pnode->log_info.ruld_id, pdata, sizeof(u_int32_t));
		pnode->log_info.ruld_id = ntohl(pnode->log_info.ruld_id);
		log_debug("get rule_id=%u\n", pnode->log_info.ruld_id);
		pdata += (int)sizeof(u_int32_t);
		//timestamp
		memcpy((void *)&pnode->log_info.time_stamp, pdata, sizeof(u_int32_t));
		pnode->log_info.time_stamp = ntohl(pnode->log_info.time_stamp);
		pdata += (int)sizeof(u_int32_t);
		//flag
		if (flag&LOG_FLAG_IS_TEST) {
			pnode->log_info.is_test = true;
		}
		if (flag&LOG_FLAG_IS_TIMER) {
			pnode->log_info.is_timer = true;
		}
		log_debug("get time_stamp=%u is_test=%u is_timer=%u\n", 
			pnode->log_info.time_stamp, pnode->log_info.is_test, pnode->log_info.is_timer);
		
		remain_len -= len;
	}

	ucla_event_push_err(LA_LOG_RULE_EXCUTE, 0, pr->home_id);
}

static void do_log_mem_proc(ucla_session_t *s,ucph_t* hdr)
{
	int i;
	ucp_log_member_query_r_t *pr = NULL;
	ucp_log_member_item_t *pitem = NULL;
	la_log_home_member_change_t *pml = NULL;
	la_log_home_member_node_t *pnode = NULL;

	if (hdr->param_len < (u_int16_t)sizeof(*pr)) {
		log_err(false, "err len=%u\n", hdr->param_len);
		return;
	}

	pr = get_net_ucp_payload(hdr, ucp_log_member_query_r_t);
	pr->result = ntohs(pr->result);
	pr->home_id = ntohl(pr->home_id);
	pr->record_count = ntohs(pr->record_count);

	log_debug("enter %s record_count=%u\n", __FUNCTION__, pr->record_count);
	if (pr->result) {
		log_err(false, "err result=%u\n", pr->result);
		return;
	}
	//len check
	if (hdr->param_len < (sizeof(*pr) + pr->record_count * sizeof(*pitem))) {
		log_err(false, "err len %u %u\n", hdr->param_len, 
			(sizeof(*pr) + pr->record_count * sizeof(*pitem)));
		return;
	}
	pml = ucla_log_mem_find_by_homeid(pr->home_id);
	if (!pml) {
		pml = ucla_log_mem_new();
		if (!pml) {
			log_err(true, "calloc failed\n");
			return;
		}
		stlc_list_add_tail(&pml->link, &plc->g_member_log_list);
	}
	pml->home_id = pr->home_id;
	//清空下节点
	la_g_member_node_list_free(pml);

	for(i = 0; i < pr->record_count; i++) {
		pitem = &pr->item[i];
		pnode = ucla_log_mem_node_new();
		if (!pnode) {
			log_err(true, "calloc failed\n");
			break;
		}
		stlc_list_add_tail(&pnode->link, &pml->log_list);
		pnode->log_info.sn = ntoh_ll(pitem->sn);
		pnode->log_info.action = pitem->action;
		pnode->log_info.resaon = pitem->reason;
		pnode->log_info.user_id = ntohl(pitem->user_id);
		pnode->log_info.time_stamp = ntohl(pitem->time_stamp);
		memcpy(pnode->log_info.username, pitem->user_name, sizeof(pitem->user_name));
		log_debug("index=%d sn=%"PRIu64" action=%u resaon=%u userid=%u timestamp=%u name=%s\n", 
			i, pnode->log_info.sn, pnode->log_info.action, pnode->log_info.resaon, 
			pnode->log_info.user_id, pnode->log_info.time_stamp, pnode->log_info.username);
	}

	ucla_event_push_err(LA_LOG_HOME_DEV_CHANGE, 0, pr->home_id);
}

static void do_ucla_log_reply_pkt(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);
	log_debug("enter %s action=%u hdr->param_len=%u\n", 
		__FUNCTION__, *paction, hdr->param_len);
	
	switch(*paction) {
	case UCAU_HISTORY_RULE_QUERY:
		do_log_rule_proc(s, hdr);
		break;
	case UCAU_HISTORY_MEMBER_QUERY:
		do_log_mem_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void do_widget_proc(ucla_session_t *s,ucph_t* hdr)
{
	ucp_widget_key_reply_t *pr;

	if (hdr->param_len < (u_int16_t)sizeof(*pr)) {
		log_err(false, "err len=%u\n", hdr->param_len);
		return;
	}

	pr = get_net_ucp_payload(hdr, ucp_widget_key_reply_t);
	if (pr->result) {
		log_err(false, "pr->result=%u\n", pr->result);
	}
	
	memcpy(s->widget_key, pr->key, sizeof(s->widget_key));

	ucla_event_push(LA_WIDGET_KEY_DONAME_CHANGED);
}

static void do_ucla_widget_reply_pkt(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);
	log_debug("enter %s action=%u hdr->param_len=%u\n", 
		__FUNCTION__, *paction, hdr->param_len);
	
	switch(*paction) {
	case UCAU_QUERY:
		do_widget_proc(s, hdr);
		break;
	default:
		break;
	}
}

bool sa_do_ucla_reply_pkt(ucla_session_t *s,ucph_t* hdr)
{
    if (!s || !hdr) {
        return true;
    }
    
    switch (hdr->command) {
        case CMD_HOME_CONFIG:
			do_ucla_home_conf_proc(s, hdr);
            break;
        case CMD_HOME_SHARE:
			do_ucla_home_share_proc(s, hdr);
            break;
        case CMD_LINKAGE_CONFIG:
			do_ucla_link_conf_proc(s, hdr);
            break;
		case CMD_APP_USER:
			do_ucla_app_user_proc(s, hdr);
			break;
		case CMD_HOME_LABEL:
			do_ucla_home_label_proc(s, hdr);
			break;
		case CMD_HOME_DICTIONARY:
			do_ucla_home_dict_proc(s, hdr);
			break;
		case CMD_QUERY_HISTORY:
			do_ucla_log_reply_pkt(s, hdr);
			break;
		case CMD_WIDGET_KEY:
			do_ucla_widget_reply_pkt(s, hdr);
			break;
        default:
            break;
	}
	
    return true;
}

RS ucla_sa_query_objects(ucla_session_t *s,ucp_obj_t* objs,int count)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    ucp_obj_t* uo;
    int i;
   
    if (!s || !objs ||count <= 0) {
        return RS_ERROR;
    }

	UCLA_ENTER();
    pkt = ucla_pkt_new(CMD_UDP_CTRL, sizeof(ucp_ctrl_t) + sizeof(ucp_obj_t)*count,
                     true, true, 0, s->client_sid, s->device_sid, s->my_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_GET;
    uc->count = count;
    uo = (ucp_obj_t*)(uc+1);
    for (i = 0; i < count; i++,uo++) {
        fill_net_ucp_obj(uo, objs[i].objct, objs[i].sub_objct, objs[i].attr, 0);
    }
    
    ucla_request_add(s, pkt);
    
    return RS_OK;
}


void ucla_query_trans_info(ucla_session_t *s)
{
	ucp_obj_t obj = {UCOT_SYSTEM, UCSOT_SYS_SERVER, UCAT_SERVER_LANSERVERIP, 0};

	if (plc->support_trans) {
		ucla_sa_query_objects(s, &obj, 0x1);
	}
}

void do_ucla_del_label(ucla_session_t *s, la_home_t *phome, u_int16_t label_id)
{
	la_label_t *plabel = NULL;
	
	if (!s || !phome) {
		return;
	}

	plabel = ucla_find_home_label_by_id(phome, label_id);
	if (plabel) {
		stlc_list_del(&plabel->link);
		ucla_label_free(plabel);
		ucla_user_conf_save(s);
		ucla_event_push_err(LA_HOME_CHANGED, phome->handle, 0);
	}
}

void do_ucla_query_label_info(ucla_session_t *s, u_int32_t home_id, u_int16_t label_id)
{
	pkt_t *pkt;
	uc_label_query_t *plq = NULL;
	
	if (!s ||
		!ucla_session_is_ok(s)) {
		log_info("failed\n");
		return;
	}

	pkt = ucla_pkt_new(CMD_HOME_LABEL, sizeof(*plq), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		log_err(false, "ucla_pkt_new failed\n");
		return;
	}

	plq = get_uascp_payload(pkt, uc_label_query_t);
	plq->action = UCAU_QUERY;
	plq->label_id = ntohs(label_id);
	plq->home_id = htonl(home_id);

	log_info("%s goto query home_id=%u s->my_request_id=%u\n", __FUNCTION__, home_id, s->my_request_id);
	ucla_request_add(s, pkt);
}

void do_ucla_query_shortcut_info(ucla_session_t *s, u_int32_t home_id, u_int8_t index)
{
	pkt_t *pkt;
	uc_home_shortcut_query_t *pq = NULL;
	
	if (!s ||
		!ucla_session_is_ok(s)) {
		log_info("failed\n");
		return;
	}
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(*pq), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		log_err(false, "ucla_pkt_new failed\n");
		return;
	}

	pq = get_uascp_payload(pkt, uc_home_shortcut_query_t);
	pq->action = UCAU_HOME_SHORTQUERY;
	pq->query_index = index;
	pq->home_id = ntohl(home_id);

	log_info("%s goto query home_id=%u s->my_request_id=%u\n", __FUNCTION__, home_id, s->my_request_id);
	ucla_request_add(s, pkt);
}

void do_ucla_shortcut_del(ucla_session_t *s, u_int32_t home_id, u_int8_t index)
{
	la_home_t *phome = NULL;

	log_debug("enter %s homeid=%u index=%u\n", 
		__FUNCTION__, home_id, index);
	
	if ((index == 0) ||
		(index > LA_SC_A_NUM)) {
		return;
	}

	phome = ucla_find_home_by_id(home_id);
	if (!phome) {
		return;
	}

	phome->la_sc_key[index - 1].valid = false;
}

void do_ucla_dict_query(ucla_session_t *s, u_int32_t home_id, u_int8_t *key, u_int16_t key_len)
{
	u_int8_t buf[1024] = {0};
	uc_dict_query_t *request = (uc_dict_query_t *)buf;

	log_debug("do_ucla_query_dict key_len %u \n", key_len);

	request->key_len = key_len;
	request->home_id = home_id;
	memcpy(request->key, key, key_len);
	
	_la_dict_query((u_int8_t*)request, sizeof(*request) + key_len);
}

void do_ucla_dict_del(ucla_session_t *s, u_int32_t home_id, u_int8_t *key, u_int16_t key_len)
{
	la_home_t *phome = NULL;

	log_debug("do_ucla_dict_del key_len %u\n", key_len);
	
	phome = ucla_find_home_by_id(home_id);
	if (!phome) {
		log_info("not found homeid=%u\n", home_id);
		return;
	}

	ucla_dict_del(phome, key, key_len);
}

static RS ucla_response_send(ucla_session_t *s, pkt_t *pkt)
{
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);
    
	return RS_OK;
}

static void ucla_sa_send_push_response(ucla_session_t *s,ucp_ctrl_t* suc)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    ucp_obj_t* obj;
    
    pkt = ucla_pkt_new(CMD_UDP_CTRL, sizeof(ucp_ctrl_t) + sizeof(ucp_obj_t)
                     , false, true, 0, s->client_sid, s->device_sid, s->peer_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
    obj = (ucp_obj_t*)(uc+1);
	uc->action = UCA_PUSH;
    if (suc->count > 0 ) {
        uc->count = 1;
        memcpy(obj, suc+1, sizeof(*obj));
        obj->param_len = 0x0;
    }else{
        uc->count = 0;
    }
    
    log_info("send PUSH PACKET to peer obj count [%u]  to us[%u] rid 0x%x\n", 
		uc->count,suc->count, s->peer_request_id);
	
    ucla_response_send(s,pkt);
}

static bool ucla_sa_rid_check(ucla_session_t *s, u_int8_t rid)
{
	if (s->last_rid == rid) {
		return false;
	}

	s->last_rid = rid;

	return true;
}

static bool ucla_hand_ctrl_result(ucla_session_t *s,ucp_obj_t* obj,u_int16_t error)
{
	log_info("%s %u.%u.%u ctrl result=%u\n", 
		__FUNCTION__, obj->objct, obj->sub_objct, obj->attr, error);

	return true;
}

//固定长度读取
RS _ucla_com_read_info(ucla_session_t *s, char *name, u_int8_t *buf, int len)
{
	char path[1000] = {0};
	int nread, i, crc = 0, crc1 = 0;
	FILE *fp = NULL;

	if (s) {
		sprintf(path, "%s/%s/%s", cl_priv->dir, s->doname, name);
	} else {
		sprintf(path, "%s/%s", cl_priv->dir, name);
	}
	if ((fp = fopen(path, "rb")) == NULL) {
		log_err(false, "user info file [%s] not created..\n", path);
		goto err;
	}

	nread = (int)fread((u_int8_t*)buf, 1, len, fp);

	if (len != nread) {
		log_err(false, "need %u, but get %u\n", len, nread);
		goto err;
	}

	// 计算数据校验和
	for (i = 0; i < len; i++) {
		crc ^= buf[i];
	}

	// 再尝试读取4字节校验和
	nread = (int)fread((u_int8_t*)&crc1, 1, sizeof(crc1), fp);

	if (nread != sizeof(crc1)) {
		log_err(false, "have no crc\n");
		goto err;
	}

	fclose(fp);

	if (crc1 != crc) {
		log_err(false, "file crc 0x%x != 0x%x\n", crc1, crc);
		return RS_ERROR;
	}

	//log_debug("read %s len %d crc 0x%x\n", path, len, crc);

	return RS_OK;
err:
	if (fp) {
		fclose(fp);
	}

	return RS_ERROR;
}

//固定长度保存
void _ucla_com_write_info(ucla_session_t *s, char *name, u_int8_t *data, int len)
{
	char path[1000] = {0};
	FILE *fp;
	int i, crc = 0;

	if (s) {
		sprintf(path, "%s/%s/%s", cl_priv->dir, s->doname, name);
	} else {
		sprintf(path, "%s/%s", cl_priv->dir, name);
	}
	if ((fp = fopen(path, "wb")) == NULL) {
		log_err(true, "fopen path [%s] failed\n", path);
		return;
	}

	len = (int)fwrite(data, 1, len, fp);

	for (i = 0; i < len; i++) {
		crc ^= data[i];
	}

	fwrite((u_int8_t*)&crc, 1, sizeof(crc), fp);

	fclose(fp);

	
	log_debug("write %s done len %d, crc 0x%x\n", path, len, crc);
}

void trans_save(ucla_session_t *s)
{
	la_trans_s_t save;

	save.trans_ip_num = cl_priv->trans_ip_num;
	memcpy((void *)save.trans_ip, (void *)cl_priv->trans_ip, sizeof(save.trans_ip));
	_ucla_com_write_info(s, TRANS_DEV_IP_INFO_FILE, (u_int8_t *)&save, sizeof(save));
}

void trans_init()
{
	la_trans_s_t save;

	if (_ucla_com_read_info(NULL, TRANS_DEV_IP_INFO_FILE, (u_int8_t *)&save, sizeof(save)) != RS_OK) {
		return;
	}

	cl_priv->trans_ip_num = save.trans_ip_num;
	memcpy((void *)cl_priv->trans_ip, (void *)save.trans_ip, sizeof(cl_priv->trans_ip));
}

static bool ucla_up_sys_server_lanip_info(ucla_session_t *s, u_int8_t action, ucp_obj_t* obj)
{
	int i, n;
	u_int32_t ip = 0;
	u_int32_t *pvalue32 = (u_int32_t *)&obj[1];

	n = (int)(obj->param_len/sizeof(ip));
	log_info("get ip num=%d\n", n);
	if (n == 0) {
		return false;
	}

	if (n > MAX_TRANS_IP_NUM) {
		n = MAX_TRANS_IP_NUM;
	}

	cl_priv->trans_ip_num = n;
	for(i = 0; i < n; i++) {
		ip = htonl(pvalue32[i]);
		cl_priv->trans_ip[i] = ip;
		log_info("get trans ip=%u.%u.%u.%u\n", IP_SHOW(ip));
	}

	trans_save(NULL);
		
	return true;
}

static bool ucla_up_sys_server_info(ucla_session_t *s, u_int8_t action, ucp_obj_t* obj)
{
    switch (obj->attr) {
        case UCAT_SERVER_LANSERVERIP:
			return ucla_up_sys_server_lanip_info(s, action, obj);
        default:
            break;
    }
	
    return false;	
}

static bool ucla_up_sys_info(ucla_session_t *s, u_int8_t action, ucp_obj_t* obj)
{
    switch (obj->sub_objct) {
        case UCSOT_SYS_SERVER:
			return ucla_up_sys_server_info(s, action, obj);
        default:
            break;
    }
	
    return false;	
}

static bool ucla_update_data_with_mib(ucla_session_t *s, u_int8_t action, ucp_obj_t* obj)
{   
    switch (obj->objct) {
        case UCOT_SYSTEM:
			return ucla_up_sys_info(s, action, obj);
        default:
            break;
    }
	
    return false;
}

void ucla_sa_parse_ctrl_pkt(ucla_session_t *s,ucph_t* hdr)
{
    ucp_ctrl_t* uc;
    ucp_obj_t* obj;
    int total = hdr->param_len;
    int index;
    bool modify = false;
    u_int16_t error;

    uc = get_net_ucp_payload(hdr, ucp_ctrl_t);
    log_debug("%s actin[%u] count[%u] request[%u] \n", __FUNCTION__, uc->action, uc->count,hdr->request);
    //先回响应
    if (uc->action == UCA_PUSH && hdr->request) {
       	ucla_sa_send_push_response(s,uc);
    }
    if (!uc->count) {
        return;
    }
	if (!ucla_sa_rid_check(s, hdr->request_id)) {
		return;
	}
		
    total -= sizeof(*uc);
    obj = (ucp_obj_t*)(uc+1);
    
    for (index = 0; index < uc->count && total >= sizeof(ucp_obj_t); index++) {
		ucp_obj_order(obj);

		if (!is_valid_obj(obj,total)) {
		    break;
		}
		log_debug("********* %s sa handle object[%u] sub_obj[%u] attr[%u] param_len[%u], action[%hhu]\n",
			__FUNCTION__, obj->objct,obj->sub_objct,obj->attr,obj->param_len, uc->action);

		//统一预处理
		total-=(sizeof(ucp_obj_t)+obj->param_len);
		
		if (uc->action == UCA_GET ||
		    uc->action == UCA_PUSH) {
		    if(ucla_update_data_with_mib(s,uc->action,obj)){
		        modify = true;
		    }
		}else if(uc->action == UCA_SET){
			
		    if (obj->param_len != sizeof(u_int16_t)) {
		        break;
		    }
		    error = ntohs(*((u_int16_t*)(obj+1)));
		    ucla_hand_ctrl_result(s, obj,error);
		}

		obj = (ucp_obj_t*)((char*)(obj+1)+obj->param_len);
	}
}

void ucla_sa_do_udp_reply_pkt(ucla_session_t *s,ucph_t* hdr)
{
	ucla_sa_parse_ctrl_pkt(s,hdr);
}

static void ucla_home_change_added(ucla_session_t *s, u_int32_t home_id)
{
	la_user_t *puser;
	la_home_t *phome;

	if (!s) {
		return;
	}
	puser = ucla_find_user_by_id(s->user_id);
	if (!puser) {
		return;
	}
	phome = ucla_find_home_by_id(home_id);
	if (!phome) {
		phome = ucla_home_new();
		if (!phome) {
			return;
		}
		ucla_home_add(s, phome);
	}
	plc->has_any_home = true;
	phome->conf.home_id = home_id;
	do_ucla_home_query(s, home_id);
	do_ucla_share_query(s, home_id);
	if (plc->support_tabel) {
		do_ucla_query_label_info(s, home_id, 0);
	}
	if (plc->support_shortcut) {
		do_ucla_query_shortcut_info(s, home_id, 0);
	}
	ucla_user_conf_save(s);
}

static void do_ucla_home_event_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_home_conf_event_t *pe = NULL;
	uc_home_conf_event_r_t *per = NULL;
	pkt_t *pkt = NULL;
	u_int16_t label_id;
	u_int32_t index;
	la_log_home_rule_change_t *prl;

	log_debug("do_ucla_home_event_req_proc hdr->request_id=%02x last_id=%02x\n", hdr->request_id, s->home_event_last_id);
	pe = get_net_ucp_payload(hdr, uc_home_conf_event_t);
	//先回答
	pkt = ucla_pkt_new(CMD_HOME_CONFIG, sizeof(uc_home_conf_event_r_t), 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	per = get_uascp_payload(pkt, uc_home_conf_event_r_t);
	per->aciton = UCAU_EVENT;
	per->result = 0;
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);
	if (s->home_event_last_id == hdr->request_id) {
		log_debug("%s recv same peer_request_id=%u\n", __FUNCTION__, hdr->request_id);
		return;
	}
	s->home_event_last_id = hdr->request_id;
	pe->home_id = htonl(pe->home_id);
	phome = ucla_find_home_by_id(pe->home_id);
	if (!phome && 
		(pe->event_type != HOME_EVENT_HOME_ADDED) &&
		(pe->event_type != HOME_EVENT_MEMBER_CHANGED)) {
		log_info("err pe->event_type=%u pe->home_id=%u\n", pe->event_type, pe->home_id);
		return;
	}

	log_debug("enter %s eventtype=%u\n", __FUNCTION__, pe->event_type);
	switch(pe->event_type) {
	case HOME_EVENT_DEV_CHANGED:
		//设备发生变化，APP按需更新设备信息
		_ucla_home_event_need_query(s, pe->home_id);
		break;
	case HOME_EVENT_HOME_DELED:
		ucla_home_all_mem_logout(pe->home_id);
		ucla_home_del(s, pe->home_id);
		ucla_event_push_err(LA_HOME_SHARE_DELETED_BY_CREATOR, 0, pe->home_id);
		ucla_event_push(LA_USER_CHANGED);
		//这里有可能是被其他app删除默认圈子了，要同步一下，不通app间可能出现默认圈子不是一个的情况
		do_ucla_def_home_sync(s);
		break;
	case HOME_EVENT_PASSWD_CHANGED:
		// TODO:更新圈子密码，并迫使所有圈子设备重新登陆
		_ucla_home_event_update_passwd(s, pe);
		break;
	case HOME_EVENT_MEMBER_CHANGED:
		//成员发生变化，APP按需更新成员信息
		if (phome) {
			_ucla_link_conf_event_need_query(s, pe->home_id);
		} else {
			//服务器为了推送方便，统一发事件3，要判断下本地有没有家庭，没有就添加
			ucla_home_change_added(s, pe->home_id);
			//ucla_event_push(LA_USER_SHARE_ADD_ONE_HOME_SUCCESSED);
			ucla_home_event_push(pe->home_id, LA_USER_SHARE_ADD_ONE_HOME_SUCCESSED);
			//ucla_event_push(LA_USER_CHANGED);
		}
		break;
	case HOME_EVENT_HOME_NAME_CHANGED:
		//圈子名称发生改变
		memcpy((void *)phome->conf.home_name, (void *)&pe[1], 64);
		log_debug("home name modify=%s\n", phome->conf.home_name);
		ucla_user_conf_save(s);
		ucla_home_event_push(phome->conf.home_id, LA_HOME_CHANGED);
		break;
	case HOME_EVENT_HOME_ADDED:
		//有圈子添加
		if (pe->home_id == 0) {
			log_debug("HOME_EVENT_HOME_ADDED err homeid=0\n");
			return;
		}
		ucla_home_change_added(s, pe->home_id);
		ucla_user_conf_save(s);
		ucla_event_push(LA_USER_CHANGED);
		break;
	case HOME_EVENT_LABEL_CHANGED:
		label_id = htons(*(u_int16_t *)&pe[1]);
		log_debug("%s query labelid=%u\n", __FUNCTION__, label_id);
		do_ucla_query_label_info(s, pe->home_id, label_id);
		break;
	case HOME_EVENT_LABEL_DEL:
		label_id = htons(*(u_int16_t *)&pe[1]);
		log_debug("%s del labelid=%u\n", __FUNCTION__, label_id);
		do_ucla_del_label(s, phome, label_id);
		break;
//	case HOME_EVENT_SC_CHANGE:
		//do_ucla_query_shortcut_info(s, phome->conf.home_id, *(u_int8_t *)&pe[1]);
	//	break;
//	case HOME_EVENT_SC_DEL:
		//do_ucla_shortcut_del(s, phome->conf.home_id, *(u_int8_t *)&pe[1]);
	//	break;
	case HOME_EVENT_DICT_CHANGE:
		{
			uc_dict_event_t *ev = (uc_dict_event_t *)&pe[1];
			ev->key_len = ntohs(ev->key_len);
			log_debug("home event dict change: key_len %u\n", ev->key_len);
			do_ucla_dict_query(s, phome->conf.home_id, ev->key, ev->key_len);
		}
		break;

	case HOME_EVENT_DICT_DEL:
		{
			uc_dict_event_t *ev = (uc_dict_event_t *)&pe[1];
			ev->key_len = ntohs(ev->key_len);
			log_debug("home event dict del: key_len %u\n", ev->key_len);
			do_ucla_dict_del(s, phome->conf.home_id, ev->key, ev->key_len);
		}
		break;
	case HOME_EVENT_LOG_CHANGE:
		{
			index = *(u_int32_t *)&pe[1];
			prl = ucla_log_rule_find_by_homeid(phome->conf.home_id);
			if (prl) {
				prl->max_index = index;
				ucla_event_push_err(LA_LOG_RULE_EXCUTE_MAX_INDEX, phome->handle, index);
			}
		}
		break;
	default:
		break;
	}

}

static void do_ucla_home_conf_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	switch(*paction) {
	case UCAU_CREATE:
		break;
	case UCAU_DELETE:
		break;
	case UCAU_ADDDEV:
		break;
	case UCAU_REMOVEDEV:
		break;
	case UCAU_QUERY:
		break;
	case UCAU_EVENT:
		do_ucla_home_event_req_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void _ucla_home_share_event_need_query(ucla_session_t *s, u_int32_t home_id)
{
	do_ucla_share_query(s, home_id);
}

static void do_ucla_home_share_event_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_home_share_event_t *pe = NULL;
	uc_linkage_home_share_event_r_t *per;
	pkt_t *pkt = NULL;
	
	pe = get_net_ucp_payload(hdr, uc_linkage_home_share_event_t);

	//先回答
	pkt = ucla_pkt_new(CMD_HOME_SHARE, sizeof(uc_linkage_home_share_event_r_t), 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	per = get_uascp_payload(pkt, uc_linkage_home_share_event_r_t);
	per->action = UCAU_EVENT;
	per->result = 0;
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);

	if (s->share_event_last_id == hdr->request_id) {
		log_debug("%s recv same peer_request_id=%u\n", __FUNCTION__, hdr->request_id);
		return;
	}
	s->share_event_last_id = hdr->request_id;

	pe->home_id = htonl(pe->home_id);
	phome = ucla_find_home_by_id(pe->home_id);
	if (!phome) {
		log_debug("%s not found homeid=%u\n", __FUNCTION__, pe->home_id);
		return;
	}

	log_debug("enter %s eventtype=%u\n", __FUNCTION__, pe->event_type);
	switch(pe->event_type) {
	case SHARE_EVENT_MEMBER_CHANGED:
		//分享成员发生变化，APP按需更新信息
		_ucla_home_share_event_need_query(s, pe->home_id);
		break;
	default:
		break;
	}

}

static void do_ucla_home_share_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	switch(*paction) {
	case UCAU_CREATE:
		break;
	case UCAU_DELETE:
		break;
	case UCAU_ADDDEV:
		break;
	case UCAU_REMOVEDEV:
		break;
	case UCAU_QUERY:
		break;
	case UCAU_EVENT:
		do_ucla_home_share_event_req_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void _ucla_link_conf_event_need_query(ucla_session_t *s, u_int32_t home_id)
{
	uc_linkage_home_share_query_t *rt ;
	pkt_t *pkt;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_HOME_SHARE, sizeof(uc_linkage_home_share_query_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return;
	}
	rt = get_uascp_payload(pkt, uc_linkage_home_share_query_t);
	rt->action = UCAU_QUERY;
	rt->home_id = ntohl(home_id);

	ucla_request_add(s, pkt);
}

bool _ucla_link_conf_rule_need_query(ucla_session_t *s, u_int32_t home_id, u_int32_t rule_id)
{
	uc_link_conf_rulelist_t *request;
	pkt_t *pkt;

	UCLA_ENTER();
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, sizeof(uc_link_conf_rulelist_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return false;
	}

	request = get_uascp_payload(pkt, uc_link_conf_rulelist_t);
	request->aciton = UCAU_RULELIST;
	request->home_id = ntohl(home_id);
	request->rule_id = ntohl(rule_id);

	plc->is_rule_all_query = false;
	ucla_request_add(s, pkt);

	return true;
}

static void do_ucla_link_conf_event_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_linkage_config_event_t *pe = NULL;
	uc_linkage_config_event_t *per = NULL;
	pkt_t *pkt = NULL;
	la_rule_t *plrule;
	u_int8_t *pdata = NULL;
	
	pe = get_net_ucp_payload(hdr, uc_linkage_config_event_t);

	//先回答
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, sizeof(uc_linkage_config_event_t), 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	per = get_uascp_payload(pkt, uc_linkage_config_event_t);
	per->action = UCAU_EVENT;
	//copy
	memcpy((void *)per, (void *)pe, sizeof(*per));
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);

	if (s->linkage_event_last_id == hdr->request_id) {
		log_debug("%s recv same peer_request_id=%u\n", __FUNCTION__, hdr->request_id);
		return;
	}
	s->linkage_event_last_id = hdr->request_id;	

	pe->home_id = htonl(pe->home_id);
	pe->rule_id = htonl(pe->rule_id);
	pdata = (u_int8_t *)&pe[1];
	phome = ucla_find_home_by_id(pe->home_id);
	if (!phome) {
		log_debug("%s not found homeid=%u\n",__FUNCTION__, pe->home_id);
		return;
	}

	log_debug("enter %s eventtype=%u pe->pad=%u\n", __FUNCTION__, pe->event, pe->pad);
	switch(pe->event) {
	case LINKAGE_EVENT_RULE_QUERY_ALL:
	case LINKAGE_EVENT_RULE_MODIFY:
		_ucla_link_conf_rule_need_query(s, pe->home_id, pe->rule_id);
		if (pe->pad) {
			do_ucla_query_shortcut_info(s, phome->conf.home_id, pe->pad);
		}
		break;
	case LINKAGE_EVENT_RULE_DELED:
		log_debug("LINKAGE_EVENT_RULE_DELED pe->rule_id=%u pad=%u\n", pe->rule_id, pe->pad);
		plrule = ucla_find_rule_by_id(phome, pe->rule_id);
		if (plrule) {
			stlc_list_del(&plrule->link);
			ucla_rule_free(plrule);
			ucla_home_event_push(pe->home_id, LA_HOME_CHANGED);
		}
		if (pe->pad) {
			do_ucla_shortcut_del(s, phome->conf.home_id, pe->pad);
		}
		ucla_user_conf_save(s);
		break;
	case LINKAGE_EVENT_ENABLE_CHANGED:
		plrule = ucla_find_rule_by_id(phome, pe->rule_id);
		if (plrule) {
			if (plrule->enable != pe->value) {
				plrule->enable = pe->value;
				ucla_home_event_push(pe->home_id, LA_HOME_CHANGED);
			}
		}
		ucla_user_conf_save(s);
		break;
	case LINKAGE_EVENT_STATE_CHANGED:
		plrule = ucla_find_rule_by_id(phome, pe->rule_id);
		if (plrule) {
			if (plrule->state != pe->value) {
				plrule->state = pe->value;
				ucla_home_event_push(pe->home_id, LA_HOME_CHANGED);
			}
		}
		_ucla_link_conf_rule_need_query(s, pe->home_id, pe->rule_id);
		break;
	case LINKAGE_EVENT_LAST_EXE_TIMECHANGED:
		plrule = ucla_find_rule_by_id(phome, pe->rule_id);
		if (plrule) {
			plrule->state = 1;
			plrule->last_exec_time = htonl(*(u_int32_t *)pdata);
			ucla_home_event_push(pe->home_id, LA_HOME_CHANGED);
			log_debug("plrule->last_exec_time=%u\n", plrule->last_exec_time);
		}
		ucla_user_conf_save(s);
		break;
	default:
		// 规则发生变化
		_ucla_link_conf_rule_need_query(s, pe->home_id, pe->rule_id);
		break;
	}
}

static void do_ucla_link_conf_rulelist_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	la_home_t *phome = NULL;
	uc_link_conf_req_push_t *pe = NULL;
	pkt_t *pkt = NULL;
	u_int8_t *pdata = NULL;

	UCLA_ENTER();
	pe = get_net_ucp_payload(hdr, uc_link_conf_req_push_t);
	//先回答
	pkt = ucla_pkt_new(CMD_LINKAGE_CONFIG, 0, 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);

	if (s->linkage_rulelistpush_last_id == hdr->request_id) {
		log_debug("%s recv same peer_request_id=%u\n", __FUNCTION__, hdr->request_id);
		return;
	}
	s->linkage_rulelistpush_last_id = hdr->request_id;	

	pe->home_id = htonl(pe->home_id);
	pe->total_length = htonl(pe->total_length);
	
	pdata = (u_int8_t *)&pe[1];
	phome = ucla_find_home_by_id(pe->home_id);
	if (!phome) {
		log_debug("%s not found homeid=%u\n",__FUNCTION__, pe->home_id);
		return;
	}

	if (!phome->ptotal_rule) {
		log_debug("phome->ptotal_rule is null !!!!!!!!!!!\n");
		return;
	}

	if (hdr->param_len < (sizeof(*pe) + pe->total_length)) {
		log_debug("err len param_len=%u totaolen=%u\n", 
			hdr->param_len, (sizeof(*pe) + pe->total_length));
		return;
	}

	if ((phome->last_total_len_offset + pe->total_length) > phome->total_length) {
		log_debug("err len last_total_len_offset=%u pe->total_length=%u total_length=%u\n", 
			phome->last_total_len_offset, pe->total_length, phome->total_length);
		
		do_ucla_rule_parse(phome, (u_int8_t *)phome->ptotal_rule, phome->last_total_len_offset);
		SAFE_FREE(phome->ptotal_rule);
		phome->last_total_len_offset = 0;
		ucla_home_event_push(pe->home_id, LA_HOME_CHANGED);
		ucla_user_conf_save(s);
		return;
	} else if ((phome->last_total_len_offset + pe->total_length) == phome->total_length) {
		log_debug("print last_total_len_offset=%u pe->total_length=%u total_length=%u\n", 
			phome->last_total_len_offset, pe->total_length, phome->total_length);
		
		memcpy((void *)&phome->ptotal_rule[phome->last_total_len_offset], (void *)pe->data, pe->total_length);
		do_ucla_rule_parse(phome, (u_int8_t *)phome->ptotal_rule, phome->total_length);
		SAFE_FREE(phome->ptotal_rule);
		phome->last_total_len_offset = 0;
		ucla_home_event_push(pe->home_id, LA_HOME_CHANGED);
		ucla_user_conf_save(s);
		return;
	}
	
	memcpy((void *)&phome->ptotal_rule[phome->last_total_len_offset], (void *)pe->data, pe->total_length);
	
	phome->last_total_len_offset += pe->total_length;
	log_debug("do_ucla_link_conf_rulelist_req_proc pe->total_length=%u\n", pe->total_length);
}

static void do_ucla_link_conf_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	switch(*paction) {
	case UCAU_CREATE:
		break;
	case UCAU_DELETE:
		break;
	case UCAU_ADDDEV:
		break;
	case UCAU_REMOVEDEV:
		break;
	case UCAU_QUERY:
		break;
	case UCAU_EVENT:
		do_ucla_link_conf_event_req_proc(s, hdr);
		break;
	case UCAU_RULELIST_PUSH:
		do_ucla_link_conf_rulelist_req_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void do_push_response(ucla_session_t *s,ucph_t* hdr)
{
	pkt_t *pkt = NULL;
	u_int8_t *pdata = NULL;
	u_int8_t *pdata2 = NULL;
	int len = 4;
	
	pdata = get_net_ucp_payload(hdr, u_int8_t);

	//先回答
	pkt = ucla_pkt_new(CMD_PUSH_NOTIFY, len, 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	pdata2 = get_uascp_payload(pkt, u_int8_t);
	//copy
	memcpy((void *)pdata2, (void *)pdata, len);
	ucla_enc_pkt(s, pkt);
	ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);	
}

static void do_ucla_nofity_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int16_t len;
	u_int8_t *pdata = NULL;
	uc_notify_info_t *pn = NULL;

	UCLA_ENTER();
	pn = get_net_uascp_payload(hdr, uc_notify_info_t);

	pn->type = htons(pn->type);
	pn->cn_msg_len = htons(pn->cn_msg_len);
	pn->en_msg_len = htons(pn->en_msg_len);

	//log_mem_dump("do_ucla_nofity_req_proc", (u_int8_t *)&hdr[1], hdr->param_len);
	log_debug("pn->type=%u pn->cn_msg_len=%u pn->en_msg_len=%u\n", 
		pn->type, pn->cn_msg_len, pn->en_msg_len);
	len = sizeof(*pn) + pn->cn_msg_len + pn->en_msg_len;
	if (len > hdr->param_len) {
		log_debug("do_ucla_nofity_req_proc err len=%u paramlen=%u\n", 
			len, hdr->param_len);
		return;
	}

	pdata = (u_int8_t *)&pn[1];
	plc->msg.type = pn->type;
	SAFE_FREE(plc->msg.cn_msg);
	plc->msg.cn_msg = cl_calloc((pn->cn_msg_len+10), 1);
	if (plc->msg.cn_msg) {
		memcpy((void *)plc->msg.cn_msg, pdata, pn->cn_msg_len);
		log_debug("cn_msg=%s \n", plc->msg.cn_msg);
	}
	
	pdata += pn->cn_msg_len;
	SAFE_FREE(plc->msg.en_msg);
	plc->msg.en_msg = cl_calloc((pn->en_msg_len+10), 1);
	if (plc->msg.en_msg) {
		memcpy((void *)plc->msg.en_msg, pdata, pn->en_msg_len);
		log_debug("en_msg=%s \n", plc->msg.en_msg);
	}

	ucla_event_push(LA_SERVER_PUSH_MSG);
}

static void do_ucla_push_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	//response
	do_push_response(s, hdr);

	if (s->linkage_msg_push_last_id == hdr->request_id) {
		log_debug("%s recv same peer_request_id=%u\n", __FUNCTION__, hdr->request_id);
		return;
	}
	s->linkage_msg_push_last_id = hdr->request_id;

	switch(*paction) {
	case UCAU_SERVER_NOTIFY:
		do_ucla_nofity_req_proc(s, hdr);
		break;
	default:
		break;
	}
}

static void do_ucla_app_table_query_proc(ucla_session_t *s,ucph_t* hdr)
{
//	la_home_t *phome = NULL;
	uc_dev_ver_limit_req_push_t *pe = NULL;
	u_int8_t *pdata = NULL;

	UCLA_ENTER();
	pe = get_net_ucp_payload(hdr, uc_dev_ver_limit_req_push_t);

	if (s->linkage_dev_ver_limit_last_id == hdr->request_id) {
		log_debug("%s recv same peer_request_id=%u\n", __FUNCTION__, hdr->request_id);
		return;
	}
	s->linkage_dev_ver_limit_last_id = hdr->request_id;	

	pe->total_length = htonl(pe->total_length);
	pe->cur_length = htonl(pe->cur_length);
	
	pdata = (u_int8_t *)&pe[1];

	if (pe->cur_length > pe->total_length) {
		log_debug("err lencurl=%u tolen=%u\n", pe->cur_length, pe->total_length);
		return;
	}

	//这里做内存预分配
	if (pe->seq == 1) {
		SAFE_FREE(s->pver_limit);
		s->pver_limit = cl_calloc(pe->total_length, 1);
		s->limit_total_length = pe->total_length;
		s->limit_last_total_len_offset = 0;
	}

	if (!s->pver_limit) {
		log_debug("err pver_limit is null\n");
		return;
	}

	if (hdr->param_len < (sizeof(*pe) + pe->cur_length)) {
		log_debug("err len param_len=%u totaolen=%u\n", 
			hdr->param_len, (sizeof(*pe) + pe->cur_length));
		return;
	}

	log_debug("limit_last_total_len_offset=%u cur_length=%u total_length=%u seq=%u totalseq=%u\n", 
		s->limit_last_total_len_offset, pe->cur_length, pe->total_length, pe->seq, pe->total_seq);

	if ((s->limit_last_total_len_offset + pe->cur_length) > s->limit_total_length) {
		log_debug("err len limit_last_total_len_offset=%u pe->cur_length=%u limit_total_length=%u\n", 
			s->limit_last_total_len_offset, pe->cur_length, s->limit_total_length);
		// TODO:保存文件，解析。。。
		do_dev_ver_file_sync(s);
		s->limit_last_total_len_offset = 0;
		return;
	} else if ((s->limit_last_total_len_offset + pe->cur_length) == s->limit_total_length) {
		log_debug("ok limit_last_total_len_offset=%u pe->cur_length=%u limit_total_length=%u\n", 
			s->limit_last_total_len_offset, pe->cur_length, s->limit_total_length);
		memcpy((void *)&s->pver_limit[s->limit_last_total_len_offset], (void *)pe->data, pe->cur_length);
		// TODO:
		do_dev_ver_file_sync(s);
		s->limit_last_total_len_offset = 0;
		return;
	}
	
	memcpy((void *)&s->pver_limit[s->limit_last_total_len_offset], (void *)pe->data, pe->cur_length);
	
	s->limit_last_total_len_offset += pe->cur_length;
}

static void do_app_user_response(ucla_session_t *s,ucph_t* hdr)
{
	pkt_t *pkt = NULL;
	u_int8_t *pdata = NULL;
	u_int8_t *pdata2 = NULL;
	int len = 4;
	int n;
	
	pdata = get_net_ucp_payload(hdr, u_int8_t);
	//先回答
	pkt = ucla_pkt_new(CMD_APP_USER, len, 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	pdata2 = get_uascp_payload(pkt, u_int8_t);
	//copy
	memcpy((void *)pdata2, (void *)pdata, len);
	ucla_enc_pkt(s, pkt);
	n = ucla_send_pkt_raw(s->sock, s->is_ipv6, &s->ipc_addr, s->port, pkt);
	pkt_free(pkt);
}

static void do_ucla_app_user_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	//response
	do_app_user_response(s, hdr);

	switch(*paction) {
	case UCAU_TABLE_PUSH:
		do_ucla_app_table_query_proc(s, hdr);
		break;
	default:
		break;
	}
}

void do_ucla_udp_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	ucla_sa_parse_ctrl_pkt(s,hdr);
}

static void do_ucla_home_label_req_proc(ucla_session_t *s,ucph_t* hdr)
{
	u_int8_t *paction = NULL;

	paction = get_net_ucp_payload(hdr, u_int8_t);

	//response
	do_app_user_response(s, hdr);

	switch(*paction) {
	default:
		break;
	}
}

bool sa_do_ucla_request_pkt(ucla_session_t *s,ucph_t* hdr)
{
    if (!s || !hdr) {
        return true;
    }
	
    switch (hdr->command) {
        case CMD_HOME_CONFIG:
			do_ucla_home_conf_req_proc(s, hdr);
            break;
        case CMD_HOME_SHARE:
			do_ucla_home_share_req_proc(s, hdr);
            break;
        case CMD_LINKAGE_CONFIG:
			do_ucla_link_conf_req_proc(s, hdr);
            break;
		case CMD_PUSH_NOTIFY:
			do_ucla_push_req_proc(s, hdr);
			break;
		case CMD_APP_USER:
			do_ucla_app_user_req_proc(s, hdr);
			break;
		case CMD_HOME_LABEL:
			do_ucla_home_label_req_proc(s, hdr);
			break;
		case CMD_UDP_CTRL:
			do_ucla_udp_req_proc(s, hdr);
			break;
        default:
            //log_debug("%s ignore reqeust pkt cmd=%u\n", s->name, hdr->command);
            break;
	}
	
    return true;
}

cl_user_t *cl_la_get_user(cl_handle_t handle)
{
	int i;
	user_t *user;
	la_member_t *mem, *memn;
	cl_user_t *cu = NULL;
	la_home_t *phome = NULL;
	u_int8_t online = 0;
	cl_dev_info_t *pdevinfo = NULL;

	cl_lock(&cl_priv->mutex);	

	phome = ucla_find_home_by_handle(handle);
	if (!phome) {
		goto done;
	}

	cu = (cl_user_t *)cl_calloc(sizeof(cl_user_t), 1);

	cu->is_la = true;
	cu->user_handle = handle;

	if (phome->session_ptr) {
		online = ucla_session_is_ok(phome->session_ptr);
	} else {
		online = ucla_home_is_online(phome->handle);
	}
	//家庭信息build
	cl_home_fill(&cu->home, phome, online);

	//家庭下的设备信息
	cu->num_dev = 0;
	stlc_list_count(cu->num_dev, &phome->member_link);
	if (cu->num_dev == 0) {
		goto done;
	}
	cu->dev = cl_calloc(sizeof(cl_dev_info_t *), cu->num_dev);
	i = 0;
	stlc_list_for_each_entry_safe(la_member_t, mem, memn, &phome->member_link, link) {
		user = user_lookup_by_sn(mem->conf.sn);
		if (!user) {
			continue;
		}
		pdevinfo = cl_user_get_dev_info(user->handle);
		if (pdevinfo) {
			cu->dev[i++] = pdevinfo;
		}
	}

	cu->num_dev = i;
	
done:
	cl_unlock(&cl_priv->mutex);
	
	return cu;	
}

cl_user_t *la_ctrl_get_invalid_home()
{
	int i;
	cl_user_t *cu = NULL;
	user_t *dev, *devn;
	cl_dev_info_t *pdevinfo = NULL;

	UCLA_ENTER();
	cu = (cl_user_t *)cl_calloc(sizeof(cl_user_t), 1);
	if (!cu) {
		return NULL;
	}
	cu->is_la = true;

	if (plc->p_null_home) {
		cu->home.handle = plc->p_null_home->handle;
		cu->home.home_id = plc->p_null_home->conf.home_id;
		cu->home.online = 1;
	}
	
	cl_lock(&cl_priv->mutex);	
	//家庭下的设备信息
	cu->num_dev = 0;
	stlc_list_for_each_entry_safe(user_t, dev, devn, &cl_priv->user, link) {
		cu->num_dev++;
	}
	if (cu->num_dev == 0) {
		goto done;
	}
	cu->dev = cl_calloc(sizeof(cl_dev_info_t *), cu->num_dev);
	
	i = 0;
	stlc_list_for_each_entry_safe(user_t, dev, devn, &cl_priv->user, link) {
		pdevinfo = cl_user_get_dev_info(dev->handle);
		if (pdevinfo) {
			if (i < (int)cu->num_dev) {
				cu->dev[i++] = pdevinfo;
			} else {
				cl_user_free_dev_info(pdevinfo);
			}
		}
	}
	cu->num_dev = i;
	
done:
	cl_unlock(&cl_priv->mutex);

	return cu;
}

static void cl_share_reg_build(cl_la_home_info_t *phome_info, la_home_t *phome)
{
	int len = 0;
	
	if (phome->share_desc_num == 0) {
		return;
	}

	len = phome->share_desc_num*sizeof(la_share_desc_t);
	phome_info->share_desc_array = cl_calloc(len, 1);
	if (!phome_info->share_desc_array) {
		return;
	}

	phome_info->share_desc_num = phome->share_desc_num;
	memcpy((void *)phome_info->share_desc_array, (void *)phome->share_desc_array, len);
}

static void cl_url_free(cl_la_home_info_t *phome_info)
{
	int i;

	for(i = 0; i < phome_info->url_num; i++) {
		SAFE_FREE(phome_info->url_array[i]);
	}

	SAFE_FREE(phome_info->url_array);
}

static void cl_rule_free(cl_la_home_info_t *phome_info)
{
	int i;

	for(i = 0; i < phome_info->rule_num; i++) {
		SAFE_FREE(phome_info->rule_array[i].rule);
	}
	SAFE_FREE(phome_info->rule_array);
	phome_info->rule_num = 0;
}

static void cl_label_free(cl_la_home_info_t *phome_info)
{
	int i;

	if (!phome_info ||
		!phome_info->label) {
		return;
	}

	for(i = 0; i < phome_info->label_num; i++) {
		SAFE_FREE(phome_info->label[i].sn);
	}

	cl_free(phome_info->label);
}

static void cl_dict_free(cl_la_home_info_t *phome_info)
{
	int i;

	if (!phome_info ||
		!phome_info->dict) {
		return;
	}

	for (i = 0; i < phome_info->dict->ndict; i++) {
		SAFE_FREE(phome_info->dict->dict[i].key);
		SAFE_FREE(phome_info->dict->dict[i].value);
	}
}

void cll_home_free(cl_la_home_info_t *info)
{
	//释放url
	cl_url_free(info);
	//释放rule
	cl_rule_free(info);
	//释放share
	SAFE_FREE(info->share);
	//释放share注册表
	SAFE_FREE(info->share_desc_array);
	info->share_desc_num = 0;
	//释放标签
	cl_label_free(info);
	// 释放字典
	cl_dict_free(info);
	
}

void cl_phone_fill(cl_la_info_t *info)
{
	la_phone_t *pp, *ppn;
	int n = 0;

	info->phone_num = 0;
	if (!plc->has_phone) {
		return;
	}
	
	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		n++;
	}

	info->phone_name = cl_calloc(n*sizeof(char *), 1);
	stlc_list_for_each_entry_safe(la_phone_t, pp, ppn, &plc->la_phone, link) {
		info->phone_name[info->phone_num] = cl_strdup(pp->conf.user_name);
		if (ucla_is_cur_phone(pp)) {
			info->cur_phone = info->phone_num;
		}
		info->phone_num++;
	}
}

void cl_cap_fill(cl_la_info_t *info)
{
	int i;
	u_int8_t num = 2;
	cap_file_type_t *pfile_type;
	
	if (!info) {
		return;
	}

	info->cap_file.pfile_type = cl_calloc(sizeof(cap_file_type_t)*num, 1);
	if (!info->cap_file.pfile_type) {
		return;
	}
	info->cap_file.file_type_num = num;
	//普通能力文件
	pfile_type = &info->cap_file.pfile_type[0];
	pfile_type->type = CAP_FILE_TYPE_COM;
	pfile_type->last_cap_time = plc->cap_lastmodifytime;
	if (plc->cap_num != 0) {
		pfile_type->cap_array = cl_calloc((sizeof(u_int8_t *)*plc->cap_num), 1);
		if (!pfile_type->cap_array) {
			return;
		}
		pfile_type->cap_num = plc->cap_num;
		for(i = 0; i < plc->cap_num; i++) {
			pfile_type->cap_array[i] = (u_int8_t *)cl_strdup((char *)plc->cap_array[i]);
		}
	}
	//自定义能力文件
	pfile_type = &info->cap_file.pfile_type[1];
	pfile_type->type = CAP_FILE_TYPE_CUSTOM;
	pfile_type->last_cap_time = plc->cap_custom_lastmodifytime;
	if (plc->cap_custom_num != 0) {
		pfile_type->cap_array = cl_calloc((sizeof(u_int8_t *)*plc->cap_custom_num), 1);
		if (!pfile_type->cap_array) {
			return;
		}
		pfile_type->cap_num = plc->cap_custom_num;
		for(i = 0; i < plc->cap_custom_num; i++) {
			pfile_type->cap_array[i] = (u_int8_t *)cl_strdup((char *)plc->cap_custom_array[i]);
		}
	}
}

void cl_cap_free(cl_la_info_t *info)
{
	int i,j;
	cap_file_type_t *pfile_type;
	
	if (!info) {
		return;
	}

	for(i = 0; i < info->cap_file.file_type_num; i++) {
		pfile_type = &info->cap_file.pfile_type[i];
		for(j = 0; j < pfile_type->cap_num; j++) {
			SAFE_FREE(pfile_type->cap_array[j]);
		}
		SAFE_FREE(pfile_type->cap_array);
	}
	SAFE_FREE(info->cap_file.pfile_type);
}

static void cl_url_build(cl_la_home_info_t *phome_info, la_home_t *phome)
{
	int i;

	phome_info->url_array = cl_calloc((plc->url_num*sizeof(u_int8_t *)), 1);
	if (!phome_info->url_array) {
		return;
	}

	phome_info->url_num = plc->url_num;
	for(i = 0; i < plc->url_num; i++) {
		phome_info->url_array[i] = (u_int8_t *)cl_strdup((char *)plc->url_array[i]);
	}	
}

static void cl_rule_build(cl_la_home_info_t *phome_info, la_home_t *phome)
{
	int n = 0;
	la_rule_t *prule, *rulen;

	stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
		n++;
	}

	phome_info->rule_array = cl_calloc(n*(sizeof(cl_rule_desc_t)), 1);
	
	stlc_list_for_each_entry_safe(la_rule_t, prule, rulen, &phome->rule_link, link) {
		if (phome_info->rule_num >= n) {
			break;
		}
		phome_info->rule_array[phome_info->rule_num].enable = prule->enable;
		phome_info->rule_array[phome_info->rule_num].state = prule->state;
		phome_info->rule_array[phome_info->rule_num].rule_id = prule->rule_id;
		phome_info->rule_array[phome_info->rule_num].rule_len = prule->rule_len;
		phome_info->rule_array[phome_info->rule_num].last_exec_time = prule->last_exec_time;
		if (prule->rule_len) {
			phome_info->rule_array[phome_info->rule_num].rule = (u_int8_t *)cl_strdup((char *)prule->rule);
		} else {
			phome_info->rule_array[phome_info->rule_num].rule = (u_int8_t *)cl_strdup("");
		}
		phome_info->rule_num++;
	}
}


static void cl_dict_build(cl_la_home_info_t *phome_info, la_home_t *phome)
{
	cl_la_dict_info_t *rinfo = NULL;
	la_dict_t *pdict = NULL;
	int i;

	rinfo = cl_calloc(1, sizeof(*rinfo));
	if (!rinfo) {
		goto err;
	}

	rinfo->home_id = phome_info->home_id;

	stlc_list_for_each_entry(la_dict_t, pdict, &phome->dict_link, link) {
		
		if ((rinfo->dict[rinfo->ndict].key = cl_calloc(1, pdict->key_len)) == NULL) {
			goto err;
		}
		memcpy(rinfo->dict[rinfo->ndict].key, pdict->key, pdict->key_len);
		rinfo->dict[rinfo->ndict].key_len = pdict->key_len;

		if ((rinfo->dict[rinfo->ndict].value = cl_calloc(1, pdict->value_len)) == NULL) {
			goto err;
		}
		memcpy(rinfo->dict[rinfo->ndict].value, pdict->value, pdict->value_len);
		rinfo->dict[rinfo->ndict].value_len = pdict->value_len;

		rinfo->ndict++;

		if (rinfo->ndict > MAX_LA_DICT_NUM) {
			break;
		}
	}

	phome_info->dict = rinfo;

	return;
err:
	if (rinfo) {
		for (i = 0; i < rinfo->ndict; i++) {
			SAFE_FREE(rinfo->dict[i].key);
			SAFE_FREE(rinfo->dict[i].value);
		}
	}

	phome_info->dict = NULL;
}


static void cl_label_build(cl_la_home_info_t *phome_info, la_home_t *phome)
{
	int n, len, len1;
	la_label_t *pl, *pln;
	la_label_desc_t *pld = NULL;

	phome_info->label_num = 0;
	stlc_list_count(n, &phome->label_link);
	if (n == 0) {
		return;
	}
	len = (int)(n*sizeof(la_label_desc_t));
	phome_info->label = cl_calloc(len, 1);
	if (!phome_info->label) {
		return;
	}

	stlc_list_for_each_entry_safe(la_label_t , pl, pln, &phome->label_link, link) {
		//log_debug("%s n=%u name=%s pl->conf.sn_num=%u\n", __FUNCTION__, n, pl->conf.name, pl->conf.sn_num);
		pld = &phome_info->label[phome_info->label_num++];
		pld->id = pl->conf.id;
		memcpy((void *)pld->label_name, (void *)pl->conf.name, sizeof(pld->label_name));
		if (pl->conf.sn_num) {
			len1 = (int)(pl->conf.sn_num*sizeof(u_int64_t));
			pld->sn = cl_calloc(len1, 1);
			if (!pld->sn) {
				log_err(true, "malloc failed\n");
				return;
			}
			memcpy((void *)pld->sn, (void *)pl->p_sn, len1);
		}
		pld->sn_num = pl->conf.sn_num;
		//log_debug("cl_label_build pld->sn_num=%u\n", pld->sn_num);
	}
}

void cl_home_fill(cl_la_home_info_t *phome_info, la_home_t *phome, u_int8_t online)
{
	phome_info->handle = phome->handle;
	phome_info->home_id = phome->conf.home_id;
	memcpy((void *)phome_info->home_name, (void *)phome->conf.home_name, sizeof(phome_info->home_name));
	memcpy((void *)phome_info->home_passwd, (void *)phome->conf.home_passwd, sizeof(phome_info->home_passwd));

	phome_info->last_rule_time = phome->last_rule_time;
	phome_info->rules_is_cache = phome->rules_is_cache;
	phome_info->last_template_time = plc->last_template_time;

	phome_info->online = online;

	//判断是不是默认圈子
	if (ucla_is_def_home(phome)) {
		phome_info->is_def_home = true;
	}
	
	//url处理
	cl_url_build(phome_info, phome);
	//rule处理
	cl_rule_build(phome_info, phome);
	//share处理
	if (phome->share != NULL) {
		phome_info->share = (u_int8_t *)cl_strdup((char *)phome->share);
	} else {
		phome_info->share = (u_int8_t *)cl_strdup("");
	}
	//share 注册表处理
	cl_share_reg_build(phome_info, phome);
	//label处理
	cl_label_build(phome_info, phome);

	// dict
	cl_dict_build(phome_info, phome);
	memcpy((void *)phome_info->la_sc_key, (void *)phome->la_sc_key, sizeof(phome_info->la_sc_key));
}


