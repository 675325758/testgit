/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 联动逻辑功能头文件
**  File:    linkage_priv.h
**  Author:  liubenlong
**  Date:    11/30/2015
**
**  Purpose:
**    联动逻辑功能头文件.
**************************************************************************/


#ifndef LINKAGE_PRIV_H
#define LINKAGE_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "linkage_client.h"

/* Macro constant definitions. */


/* Type definitions. */
enum {
	LA_ACTION_HOME_CREATE = 1,
	LA_ACTION_HOME_DEL = 2,
	LA_ACTION_HOME_ADDDEV = 3,
	LA_ACTION_HOME_REMOVEDEV = 4,	

	LA_ACTION_TEMPLATE_QUERY,
	LA_ACTION_RULELIST_QUERY,
	LA_ACTION_RULELIST_ADD,
	LA_ACTION_RULELIST_DEL,
	LA_ACTION_EVENT,

	LA_ACTION_HOME_SHARE_CREATE,
	LA_ACTION_HOME_SHARE_REGISTER,
	LA_ACTION_HOME_SHARE_QUERY,
	LA_ACTION_HOME_SHARE_EDIT,
	LA_ACTION_HOME_SHARE_DEL,
	LA_ACTION_SDK_LOGIN_SET,
	LA_ACTION_SDK_CONF_CLEAN,
	LA_ACTION_LINK_RULE_EXEC,
	LA_ACTION_DEV_MOVE,
	LA_ACTION_RULE_MODIFY,
	LA_ACTION_HOME_NAME_MODIFY,

	LA_ACTION_PAD_SC_REQ,
	LA_ACTION_PAD_SC_REGISTER,

	LA_ACTION_ALL_INFO_GET,

	//标签
	LA_ACTION_LABEL_ADD,
	LA_ACTION_LABEL_DEL,
	LA_ACTION_LABEL_BIND,
	//快捷按键
	LA_ACTION_SHORTCUT_ADD,
	LA_ACTION_SHORTCUT_DEL,
	LA_ACTION_SHORTCUT_MOD,
	// 字典
	LA_ACTION_DICT_QUERY,
	LA_ACTION_DICT_SET,
	LA_ACTION_DICT_DEL,
	//s3网关用
	LA_ACTION_S3_LMT_QUERY,
	LA_ACTION_S3_RULE_QUERY,
	LA_ACTION_S3_RULEINFO_GET,
	//pad用来执行本地规则
	LA_ACTION_PAD_EXCUTE_LOCAL_RULE,
	//日志相关
	LA_ACTION_LOG_RULE_QUERY,
	LA_ACTION_LOG_RULE_GET,
	LA_ACTION_LOG_MEM_QUERY,
	LA_ACTION_LOG_MEM_GET,
	//widget相关
	LA_ACTION_WIDGET_KEY_QUERY,
	LA_ACTION_WIDGET_INFO_GET,
};

enum {
	LA_ACTION_PHONE_CREATE = 1,
	LA_ACTION_PHONE_LOGIN,
	LA_ACTION_PHONE_LOGOUT,
	LA_ACTION_PHONE_DEL,
	LA_ACTION_PHONE_SWICH,
	LA_ACTION_PHONE_PASSWD_MODIFY,
	LA_NOTIFY_MSG_GET,
	LA_ACTION_PHONE_RELOGIN,
	LA_ACTION_MISC_QUERY,
	LA_ACTION_RSA_SET,
	LA_ACTION_MISC_CUSTOM_CAP_QUERY,
};

typedef struct {
	u_int32_t home_id;
	u_int8_t num;
	u_int8_t flag;
	u_int64_t sn[0];
}la_home_proc_notify_t;

typedef struct {
	cl_handle_t *handle;
	char name[0];
}la_home_create_notify_t;

typedef struct {
	char name[16];
	char passwd[64];
}la_phone_notify_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

bool sa_do_ucla_reply_pkt(ucla_session_t *s,ucph_t* hdr);
bool sa_do_ucla_request_pkt(ucla_session_t *s,ucph_t* hdr);
void ucla_home_add(ucla_session_t *s, la_home_t *phome);
bool _la_proc_notify(u_int8_t *data, RS *ret);
void ucla_event_push(int event);
void ucla_home_event_push(u_int32_t home_id, int event);
void ucla_template_free(la_home_t *phome);
cl_user_t *cl_la_get_user(cl_handle_t handle);
cl_user_t *la_ctrl_get_invalid_home();
void cll_home_free(cl_la_home_info_t *info);
void cl_phone_fill(cl_la_info_t *info);
void cl_cap_fill(cl_la_info_t *info);
void cl_cap_free(cl_la_info_t *info);
void cl_home_fill(cl_la_home_info_t *phome_info, la_home_t *phome, u_int8_t online);
void ucla_delay_share_regster(ucla_session_t *s);
void do_ucla_home_query(ucla_session_t *s, u_int32_t home_id);
void la_phone_login(char *name, char *passwd, bool is_ctrl);
cl_user_t *la_ctrl_get_invalid_home();
void ucla_sa_do_udp_reply_pkt(ucla_session_t *s,ucph_t* hdr);
void do_ucla_udp_req_proc(ucla_session_t *s,ucph_t* hdr);
void ucla_query_trans_info(ucla_session_t *s);
void do_ucla_query_label_info(ucla_session_t *s, u_int32_t home_id, u_int16_t label_id);
void do_ucla_query_shortcut_info(ucla_session_t *s, u_int32_t home_id, u_int8_t index);
void trans_init();
void _ucla_com_write_info(ucla_session_t *s, char *name, u_int8_t *data, int len);
RS _ucla_com_read_info(ucla_session_t *s, char *name, u_int8_t *data, int len);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LINKAGE_PRIV_H */

