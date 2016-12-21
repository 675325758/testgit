/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_linkage.c
**  File:    cl_linkage.c
**  Author:  liubenlong
**  Date:    12/03/2015
**
**  Purpose:
**    联动外部调用实现文件.
**************************************************************************/


/* Include files. */
#include "cl_priv.h"
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_linkage.h"
#include "linkage_client.h"
#include "linkage_priv.h"

/* Macro constant definitions. */


/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
CLIB_API cl_la_info_t *cl_la_info_get()
{
	int len = 0;
	u_int8_t buff[1024];
	cl_la_info_t *pinfo = NULL;

	if (cl_priv == NULL) {
		return NULL;
	}

	len = sizeof(cl_la_info_t **);
	*(cl_la_info_t ***)buff = &pinfo;	
	if(cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_ALL_INFO_GET, buff, len) != RS_OK) {
		return NULL;
	}	

	return pinfo;
}

CLIB_API RS cl_la_info_free(cl_la_info_t * info)
{
	int i;

	if (!info) {
		return RS_OK;
	}

	for(i = 0; i < info->home_num; i++) {
		cl_user_free_info(info->home[i]);
	}

	for(i = 0; i < info->phone_num; i++) {
		SAFE_FREE(info->phone_name[i]);
	}
	SAFE_FREE(info->phone_name);

	//设备能力文件
	cl_cap_free(info);
	
	if (info) {
		SAFE_FREE(info);
	}

	return RS_OK;
}

CLIB_API RS cl_la_home_create(cl_handle_t *homehandle, char *name)
{
	int len = 0;
	u_int8_t buff[1024];
	la_home_create_notify_t *home_create = (la_home_create_notify_t *)buff;

	CL_CHECK_INIT;
	
	if (!homehandle ||
		!name) {
		return RS_ERROR;
	}

	home_create->handle = homehandle;
	strcpy(home_create->name, name);
	
	len = (int)strlen(name) + 1 + (int)sizeof(*home_create);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_HOME_CREATE, buff, len);
}

CLIB_API RS cl_la_home_del(cl_handle_t homehandle, u_int32_t home_id)
{
	int len = 0;
	u_int8_t buff[1024];
	u_int32_t *pid = (u_int32_t *)buff;

	CL_CHECK_INIT;
	
	*pid = home_id;
	len = (int)sizeof(u_int32_t);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_DEL, buff, len);
}

CLIB_API RS cl_la_home_adddev(cl_handle_t homehandle, u_int32_t home_id, u_int8_t num, u_int64_t *sn)
{
	int len = 0;
	u_int8_t buff[1024];
	la_home_proc_notify_t *pnotify = (la_home_proc_notify_t *)buff;	

	CL_CHECK_INIT;
	
	if (!sn) {
		return RS_ERROR;
	}

	pnotify->home_id = home_id;
	pnotify->num = num;
	len = num*sizeof(*sn);
	memcpy((void *)pnotify->sn, (void *)sn, len);
	len += sizeof(*pnotify);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_ADDDEV, buff, len);
}

CLIB_API RS cl_la_home_removedev(cl_handle_t homehandle, u_int32_t home_id, u_int8_t num, u_int64_t *sn, u_int8_t flag)
{
	int len = 0;
	u_int8_t buff[1024];
	la_home_proc_notify_t *pnotify = (la_home_proc_notify_t *)buff;	

	CL_CHECK_INIT;
	
	if (!sn) {
		return RS_ERROR;
	}

	pnotify->home_id = home_id;
	pnotify->num = num;
	pnotify->flag = flag;
	len = num*sizeof(*sn);
	memcpy((void *)pnotify->sn, (void *)sn, len);
	len += sizeof(*pnotify);
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_REMOVEDEV, buff, len);
}

/*
	联动配置相关
**/

// 查询一个家庭的适用模板
CLIB_API RS cl_la_config_template_query(cl_handle_t homehandle, u_int32_t home_id)
{
	uc_link_conf_template_t request;
	int len = sizeof(request);

	CL_CHECK_INIT;
		
	request.home_id = home_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_TEMPLATE_QUERY, (u_int8_t*)&request, len);
}

//  查询一个规则
CLIB_API RS cl_la_config_rule_query(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id)
{
	uc_link_conf_rulelist_t request;
	int len = sizeof(request);

	CL_CHECK_INIT;
		
	request.home_id = home_id;
	request.rule_id = rule_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_RULELIST_QUERY, (u_int8_t*)&request, len);
}

// 添加一个规则
CLIB_API RS cl_la_config_rule_add(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id, char *rule, u_int8_t enable)
{
	u_int8_t *buf = NULL;
	uc_linkage_config_rule_add_t *request;
	int len;
	RS ret;

	CL_CHECK_INIT;
	
	if (!rule) {
		return RS_ERROR;
	}

	buf = cl_calloc((strlen(rule)+100), 1);
	if (!buf) {
		return RS_ERROR;
	}

	request = (uc_linkage_config_rule_add_t *)buf;
	len = (int)(sizeof(*request) + strlen(rule) + 1);
	
	request->home_id = home_id;
	request->rule_id = rule_id;
	request->enable = enable;
	memcpy(request->rule, rule, strlen(rule));
	
	ret = cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_RULELIST_ADD, buf, len);

	cl_free(buf);
	return ret;
}


// 删除一个规则
CLIB_API RS cl_la_config_rule_del(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_config_rule_del_t *request = (uc_linkage_config_rule_del_t *)buf;
	
	int len = sizeof(*request);

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));
	
	request->home_id = home_id;
	request->rule_id = rule_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_RULELIST_DEL, buf, len);
}

// 触发事件
CLIB_API RS cl_la_config_event(cl_handle_t homehandle, u_int8_t event, u_int32_t home_id, u_int32_t rule_id)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_config_event_t *request = (uc_linkage_config_event_t *)buf;
	
	int len = sizeof(*request);

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));

	request->event = event;
	request->home_id = home_id;
	request->rule_id = rule_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_EVENT, buf, len);
}

/*
	家庭成员分享、成员管理
**/
// App向服务器申请家庭的分享码，服务器确定其有效期，默认10分钟。
CLIB_API RS cl_la_home_share_create(cl_handle_t homehandle, u_int32_t home_id)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_home_share_create_t *request = (uc_linkage_home_share_create_t *)buf;
	
	int len = sizeof(*request);

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));
	
	request->home_id = home_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_SHARE_CREATE, buf, len);
}

// 注册
CLIB_API RS cl_la_home_share_register(cl_handle_t homehandle, u_int32_t home_id, u_int8_t *share, u_int8_t *desc)
{
	int len;
	u_int8_t buf[1024] = {0};

	CL_CHECK_INIT;
	
	if (!share || !desc) {
		return RS_ERROR;
	}
	
	*(u_int32_t *)buf = home_id;
	len = sizeof(u_int32_t);
	strcpy((char *)&buf[len], (char *)share);
	len += (int)strlen((char *)share) + 1;
	strcpy((char *)&buf[len], (char *)desc);
	len += (int)strlen((char *)desc) + 1;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_SHARE_REGISTER, buf, len);
}

CLIB_API RS cl_la_app_share_create()
{
	CL_CHECK_INIT;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_PAD_SC_REQ, NULL, 0);
}

CLIB_API RS cl_la_app_share_register(u_int32_t home_id, char *share, char *desc)
{
	int len, tmp_len;
	u_int8_t buf[1024*8] = {0};
	
	CL_CHECK_INIT;

	if (!share || !desc) {
		return RS_INVALID_PARAM;
	}

	memset(buf, 0, sizeof(buf));
	len = 0;
	//homeid
	*(u_int32_t *)&buf[len] = home_id;
	len += sizeof(home_id);
	//share
	tmp_len = (int)strlen((char *)share);
	memcpy((void *)&buf[len], share, tmp_len);
	len += tmp_len + 1;
	//desc
	tmp_len = (int)strlen((char *)desc);
	memcpy((void *)&buf[len], desc, tmp_len);
	len += tmp_len + 1;

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_PAD_SC_REGISTER, buf, len);
}

CLIB_API char *cl_la_app_get_share()
{
	return plc->req_share?plc->req_share:"";
}

// 查询分享
CLIB_API RS cl_la_home_share_query(cl_handle_t homehandle, u_int32_t home_id)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_home_share_query_t *request = (uc_linkage_home_share_query_t *)buf;
	
	int len = sizeof(*request);

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));

	request->home_id = home_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_SHARE_QUERY, buf, len);
}

// 分享编辑
CLIB_API RS cl_la_home_share_edit(cl_handle_t homehandle, u_int32_t home_id, u_int32_t user_id, u_int8_t role_id, u_int8_t *desc)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_home_share_edit_t *request = (uc_linkage_home_share_edit_t *)buf;
	
	int len = (int)(sizeof(*request) + strlen((char *)desc) + 1);

	if (!desc) {
		return RS_ERROR;
	}

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));
	
	request->home_id = home_id;
	request->user_id = user_id;
	request->role_id = role_id;
	memcpy(request->desc, desc, strlen((char *)desc));
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_SHARE_EDIT, buf, len);
}

// 删除分享
CLIB_API RS cl_la_home_share_del(cl_handle_t homehandle, u_int32_t home_id, u_int32_t user_id)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_home_share_del_t *request = (uc_linkage_home_share_del_t *)buf;

	int len = sizeof(*request);

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));

	request->home_id = home_id;
	request->user_id = user_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_SHARE_DEL, buf, len);
}

CLIB_API RS cl_la_sdk_login_set(cl_handle_t homehandle, u_int64_t *sn, u_int16_t num)
{
	int len;
	u_int8_t buf[1024] = {0};

	CL_CHECK_INIT;
	
	if (!sn) {
		return RS_ERROR;
	}

	len = num*sizeof(u_int64_t);
	memcpy((void *)buf, (void *)sn, len);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_SDK_LOGIN_SET, buf, len);	
}

/*
	功能描述:
	app清楚配置
	参数:
	@homehandle,用户handle
	@sn,若干个sn的数组
	@num,num个sn
*/
// 删除分享
CLIB_API void cl_la_conf_clean()
{
	ucla_session_t *s , *n;
	char buff[1024];
//	FILE *fp = NULL;
	
	if (cl_priv->priv_dir == NULL) {
        return;
    }

	log_debug("enter %s\n", __FUNCTION__);
	//session释放
	stlc_list_for_each_entry_safe(ucla_session_t, s, n, &plc->server_client, link) {
	    sprintf(buff, "%s/%s/la_user", cl_priv->priv_dir, s->doname);
		remove(buff);
	    sprintf(buff, "%s/%s/la_member", cl_priv->priv_dir, s->doname);
		remove(buff);
	    sprintf(buff, "%s/%s/la_home", cl_priv->priv_dir, s->doname);
		remove(buff);
	}
}

CLIB_API RS cl_la_config_rule_exec(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id)
{
	u_int8_t buf[1024] = {0};
	uc_linkage_config_rule_exec_t *request = (uc_linkage_config_rule_exec_t *)buf;
	
	int len = sizeof(*request);

	CL_CHECK_INIT;
	
	memset(buf, 0, sizeof(buf));
	
	request->home_id = home_id;
	request->rule_id = rule_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_LINK_RULE_EXEC, buf, len);	
}

CLIB_API RS cl_la_cur_homeid_set(u_int32_t home_id)
{
	if (!plc->init) {
		return RS_ERROR;
	}

	plc->cur_home_id = home_id;

	return RS_OK;
}

CLIB_API u_int32_t cl_la_cur_homeid_get()
{
	return plc->cur_home_id;
}

CLIB_API RS cl_la_home_movedev(cl_handle_t homehandle, u_int32_t src_home_id, u_int32_t dst_home_id, u_int8_t num, u_int64_t *sn)
{
	int i;
	int len;
	u_int8_t buf[1024];
	uc_home_conf_move_t *pmove = (uc_home_conf_move_t *)buf;

	CL_CHECK_INIT;
	
	if (!sn) {
		return RS_ERROR;
	}

	memset(buf, 0, sizeof(buf));
	
	pmove->src_home_id = src_home_id;
	pmove->dst_home_id = dst_home_id;
	pmove->count = num;

	len = sizeof(uc_home_conf_movedev_t)*num;
	for(i = 0; i < num; i++) {
		pmove->dev_sn[i].sn = sn[i];
	}
	len += sizeof(uc_home_conf_move_t);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_DEV_MOVE, buf, len);	
}

CLIB_API RS cl_la_config_rule_modify(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id, u_int8_t enable)
{
	int len;
	u_int8_t buf[1024];
	uc_linkage_config_rule_modify_t *pm = (uc_linkage_config_rule_modify_t *)buf;

	CL_CHECK_INIT;
	
	pm->enable = enable;
	pm->home_id = home_id;
	pm->rule_id = rule_id;

	len = sizeof(uc_linkage_config_rule_modify_t);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_RULE_MODIFY, buf, len);	
}

CLIB_API RS cl_la_home_name_modify(cl_handle_t homehandle, u_int32_t home_id, char *name)
{
	int len;
	u_int8_t buf[1024];
	uc_home_conf_namemodify_t *pm = (uc_home_conf_namemodify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name) {
		return RS_ERROR;
	}

	pm->home_id = home_id;
	strncpy((char *)pm->name, name, sizeof(pm->name)-1);
	len = sizeof(*pm);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, homehandle, CLNE_LA_CTRL, LA_ACTION_HOME_NAME_MODIFY, buf, len);	
}

CLIB_API RS cl_la_phone_create(char *name, char *passwd)
{
	int len;
	u_int8_t buf[1024];
	la_phone_notify_t *ppn = (la_phone_notify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name || !passwd) {
		return RS_ERROR;
	}

	if (strlen(name) > APP_USER_UUID_NAME_LEN - 1) {
		return RS_ERROR;
	}
	
	strcpy(ppn->name, name);
	strcpy(ppn->passwd, passwd);
	len = sizeof(*ppn);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_CREATE, buf, len);	
}

CLIB_API RS cl_la_phone_login(const char *name, const char *passwd)
{
	int len;
	u_int8_t buf[1024];
	la_phone_notify_t *ppn = (la_phone_notify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name || !passwd) {
		return RS_ERROR;
	}

	if (strlen(name) > APP_USER_UUID_NAME_LEN - 1) {
		return RS_ERROR;
	}
	
	strcpy(ppn->name, name);
	strcpy(ppn->passwd, passwd);
	len = sizeof(*ppn);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_LOGIN, buf, len);	
}

CLIB_API RS cl_la_phone_relogin()
{
	u_int8_t buf[1024];
	
	CL_CHECK_INIT;

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_RELOGIN, buf, 0);
}

CLIB_API RS cl_la_phone_logout(char *name)
{
	int len;
	u_int8_t buf[1024];
	la_phone_notify_t *ppn = (la_phone_notify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name) {
		return RS_ERROR;
	}

	if (strlen(name) > APP_USER_UUID_NAME_LEN - 1) {
		return RS_ERROR;
	}
	
	strcpy(ppn->name, name);
	len = sizeof(*ppn);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_LOGOUT, buf, len);	
}

CLIB_API RS cl_la_phone_del(char *name)
{
	int len;
	u_int8_t buf[1024];
	la_phone_notify_t *ppn = (la_phone_notify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name) {
		return RS_ERROR;
	}

	if (strlen(name) > APP_USER_UUID_NAME_LEN - 1) {
		return RS_ERROR;
	}
	
	strcpy(ppn->name, name);
	len = sizeof(*ppn);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_DEL, buf, len);	
}

CLIB_API RS cl_la_phone_swich(char *name)
{
	int len;
	u_int8_t buf[1024];
	la_phone_notify_t *ppn = (la_phone_notify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name) {
		return RS_ERROR;
	}

	if (strlen(name) > APP_USER_UUID_NAME_LEN - 1) {
		return RS_ERROR;
	}
	
	strcpy(ppn->name, name);
	len = sizeof(*ppn);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_SWICH, buf, len);		
}

CLIB_API RS cl_la_phone_passwd_modify(char *name, char *passwd)
{
	int len;
	u_int8_t buf[1024];
	la_phone_notify_t *ppn = (la_phone_notify_t *)buf;

	CL_CHECK_INIT;
	
	if (!name || !passwd) {
		return RS_ERROR;
	}

	if (strlen(name) > APP_USER_UUID_NAME_LEN - 1) {
		return RS_ERROR;
	}
	
	strcpy(ppn->name, name);
	strcpy(ppn->passwd, passwd);
	len = sizeof(*ppn);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_PHONE_CTRL, LA_ACTION_PHONE_PASSWD_MODIFY, 
		buf, len);	
}

CLIB_API bool cl_la_phone_is_same_passwd(char *name, char *passwd)
{
	u_int8_t pwd_md5[16];
	la_phone_t *phone;
	
	if (!name || !passwd) {
		return false;
	}

	phone = ucla_find_phone_by_name(name);
	if (!phone) {
		return false;
	}

	hash_passwd(pwd_md5, (char *)passwd);

	return (memcmp((void *)pwd_md5, (void *)phone->conf.passwd, 16) == 0);
}

CLIB_API cl_la_notify_msg_t *cl_la_get_notify_msg()
{
	int len;
	u_int8_t buf[256];
	cl_la_notify_msg_t *pmsg = NULL;

	if (cl_priv == NULL) {
		return NULL;
	}

	len = sizeof(cl_la_notify_msg_t **);
	*(cl_la_notify_msg_t ***)buf = &pmsg;
	cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_COMM_CTRL, LA_NOTIFY_MSG_GET, 
		(u_int8_t *)buf, len);	

	return pmsg;
}

CLIB_API void cl_la_free_notify_msg(cl_la_notify_msg_t *msg)
{
	if (msg) {
		SAFE_FREE(msg->cn_msg);
		SAFE_FREE(msg->en_msg);
	}

	SAFE_FREE(msg);
}

CLIB_API RS cl_la_cur_lang_set(u_int8_t lang)
{
	if (lang != LANG_CH &&
		lang != LANG_EN) {
		return RS_ERROR;
	}
	
	plc->lang = lang;

	return RS_OK;
}

CLIB_API RS cl_la_cur_lang_get(u_int8_t *lang)
{
	if (!lang) {
		return RS_ERROR;
	}

	*lang = plc->lang;

	return RS_OK;
}

CLIB_API RS cl_la_cap_file_query()
{
	int len = 1;
	u_int8_t buf[10];

	CL_CHECK_INIT;
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_COMM_CTRL, LA_ACTION_MISC_QUERY, 
		buf, len);
}

CLIB_API RS cl_la_cap_custom_file_query()
{
	int len = 1;
	u_int8_t buf[10];

	CL_CHECK_INIT;
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_COMM_CTRL, LA_ACTION_MISC_CUSTOM_CAP_QUERY, 
		buf, len);
}

CLIB_API RS cl_la_rsa_callback_set(cl_rsa_callback rsa_enc, cl_rsa_callback rsa_dec)
{
	u_int8_t buf[1024];
	int len = sizeof(cln_la_rsa_t);
	cln_la_rsa_t *rsa = (cln_la_rsa_t *)buf;

	CL_CHECK_INIT;
	rsa->enc = rsa_enc;
	rsa->dec = rsa_dec;

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_COMM_CTRL, LA_ACTION_RSA_SET, 
		buf, len);	
}

CLIB_API RS cl_la_label_add_modify(u_int32_t home_id, u_int16_t label_id, char *name, u_int16_t sn_num, u_int64_t *sn)
{
	int len = 0;
	u_int8_t buf[MAX_UDP_PKT];
	uc_label_add_t *padd = (uc_label_add_t *)buf;

	CL_CHECK_INIT;
	
	if (!name) {
		return RS_INVALID_PARAM;
	}

	padd->home_id = home_id;
	padd->label_id = label_id;
	strncpy((char *)padd->name, name, sizeof(padd->name));
	len = (int)(sn_num * sizeof(u_int64_t));
	memcpy((void *)padd->a_sn, (void *)sn, len);
	len += (int)sizeof(*padd);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LABEL_ADD, 
		buf, len);	
}

CLIB_API RS cl_la_label_del(u_int32_t home_id, u_int16_t label_id)
{
	int len = 0;
	u_int8_t buf[1024];
	uc_label_delete_t *pdel = (uc_label_delete_t *)buf;

	CL_CHECK_INIT;

	pdel->home_id = home_id;
	pdel->label_id = label_id;
	len = (int)sizeof(*pdel);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LABEL_DEL, 
		buf, len);
}

CLIB_API RS cl_la_bind_devs_to_label(u_int32_t home_id, u_int64_t sn, u_int16_t label_id_num, u_int16_t *label_id)
{
	int len = 0;
	u_int8_t buf[MAX_UDP_PKT];
	uc_label_bind_t *pbind = (uc_label_bind_t *)buf;

	CL_CHECK_INIT;

	pbind->sn = sn;
	pbind->home_id = home_id;
	pbind->label_num = label_id_num;
	len += (int)(sizeof(*label_id)*label_id_num);
	memcpy((void *)pbind->a_label_id, (void *)label_id, len);
	len += sizeof(*pbind);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LABEL_BIND, 
		buf, len);
}

CLIB_API RS cl_la_dict_query(u_int32_t home_id, u_int8_t *key, u_int16_t key_len)
{
	int len = 0;
	u_int8_t buf[1024];
	uc_dict_query_t *request = (uc_dict_query_t *)buf;

	CL_CHECK_INIT;

	if (key_len >= 1024) {
		return RS_INVALID_PARAM;
	}

	request->home_id = home_id;
	request->key_len = key_len;

	memcpy(request->key, key, key_len);
	
	len = (int)sizeof(*request) + key_len;

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_DICT_QUERY, 
		buf, len);
}


CLIB_API RS cl_la_dict_set(u_int32_t home_id, u_int8_t *key, u_int8_t *value, u_int16_t key_len, u_int16_t value_len)
{
	int len = 0;
	u_int8_t buf[2048] = {0};
	uc_dict_set_t *request = (uc_dict_set_t *)buf;

	CL_CHECK_INIT;

	if (key_len == 0 || value_len == 0) {
		return RS_INVALID_PARAM;
	}

	if (key_len + value_len >= 1024) {
		return RS_INVALID_PARAM;
	}

	request->home_id = home_id;
	request->key_len = key_len;
	request->value_len = value_len;

	memcpy(request->key_value, key, key_len);
	memcpy(request->key_value + key_len, value, value_len);
	
	len = (int)sizeof(*request) + key_len + value_len;

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_DICT_SET, 
		buf, len);
}

CLIB_API RS cl_la_dict_del(u_int32_t home_id, u_int8_t *key, u_int16_t key_len)
{
	int len = 0;
	u_int8_t buf[1024] = {0};
	uc_dict_del_t *request = (uc_dict_del_t *)buf;

	CL_CHECK_INIT;

	if (key_len >= 1024) {
		return RS_INVALID_PARAM;
	}

	request->home_id = home_id;
	request->key_len = key_len;

	memcpy(request->key, key, key_len);
	
	len = (int)sizeof(*request) + key_len;

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_DICT_DEL, 
		buf, len);
}


CLIB_API RS cl_la_shortcut_rule_bind(u_int32_t home_id, u_int8_t index, u_int8_t enabel, char *name, u_int32_t rule_id, char *rule)
{
	int len = 0;
	u_int8_t *buf;
	RS ret = 0;
	uc_home_shortcut_t *ps = NULL;

	if (!name || !rule) {
		return RS_INVALID_PARAM;
	}

	CL_CHECK_INIT;

	len = (int)(strlen(rule) + strlen(name) + sizeof(*ps));
#if 1	
	if (len > 63*1024) {
		return RS_DATA_TOO_BIG;
	}
#else
	if (len > 300*1024) {
		return RS_DATA_TOO_BIG;
	}
#endif

	buf = cl_calloc(len + 1024, 1);
	if (!buf) {
		return RS_ERROR;
	}	

	ps = (uc_home_shortcut_t *)buf;
	ps->home_id = home_id;
	ps->enable = enabel;
	ps->index = index;
	strncpy((char *)ps->name, name, sizeof(ps->name) - 1);
	ps->rule_id = rule_id;
	ps->rule_len = (u_int32_t)strlen(rule) + 1;
	strcpy(ps->rule, rule);

	len = (int)(ps->rule_len + sizeof(*ps));

	ret = cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_SHORTCUT_ADD, 
		buf, len);
	cl_free(buf);

	return ret;
}

CLIB_API RS cl_la_shortcut_modify(u_int32_t home_id, u_int8_t index, char *name)
{
	int len = 0;
	u_int8_t buf[MAX_UDP_PKT];
	uc_home_shortcut_modify_t *ps = (uc_home_shortcut_modify_t *)buf;

	if (!name) {
		return RS_INVALID_PARAM;
	}

	CL_CHECK_INIT;
	
	memset((void *)ps, 0, sizeof(*ps));
	ps->home_id = home_id;
	ps->enable = 1;
	ps->index = index;
	strncpy((char *)ps->name, name, sizeof(ps->name) - 1);

	len = (int)sizeof(*ps);

	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_SHORTCUT_MOD, 
		buf, len);
}

CLIB_API RS cl_la_shortcut_del(u_int32_t home_id, u_int8_t index)
{
	int len = 0;
	u_int8_t buf[1024];
	uc_home_shortcut_del_t *psd = (uc_home_shortcut_del_t *)buf;

	CL_CHECK_INIT;
	
	memset((void *)psd, 0, sizeof(*psd));
	psd->home_id = home_id;
	psd->index = index;

	len = (int)sizeof(*psd);
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_SHORTCUT_DEL, 
		buf, len);
}

//s3专用
CLIB_API RS cl_la_home_rule_query(u_int32_t home_id)
{
	uc_link_conf_rulelist_t request;
	int len = sizeof(request);

	CL_CHECK_INIT;
		
	request.home_id = home_id;
	request.rule_id = 0;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_S3_RULE_QUERY, 
		(u_int8_t*)&request, len);
}

CLIB_API cl_home_rule_info_t *cl_la_home_rule_get(u_int32_t home_id)
{
	int len = 0;
	u_int8_t buf[100];
	cl_home_rule_info_t *pinfo = NULL;
	
	if (!cl_priv) {
		return NULL;
	}

	*(u_int32_t *)buf = home_id;
	len = sizeof(u_int32_t);
	*(cl_home_rule_info_t ***)&buf[len] = &pinfo;
	len += sizeof(cl_home_rule_info_t **);

	cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_S3_RULEINFO_GET, 
		buf, len);

	return pinfo;
}

CLIB_API RS cl_la_home_rule_free(cl_home_rule_info_t *pinfo)
{
	int i;

	if (pinfo) {
		for(i = 0; i < pinfo->rule_num; i++) {
			SAFE_FREE(pinfo->rule_array[i].rule);
		}
		SAFE_FREE(pinfo->rule_array);
		SAFE_FREE(pinfo);
	}
	
	return RS_OK;
}

CLIB_API RS cl_la_home_rule_query_last_modify_time(u_int32_t home_id)
{
	uc_home_last_rule_time_q_t request;
	int len = sizeof(request);

	CL_CHECK_INIT;

	request.home_id = home_id;
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_S3_LMT_QUERY, 
		(u_int8_t*)&request, len);
}

CLIB_API RS cl_la_home_rule_excute(u_int32_t home_id, char *rule)
{
	RS ret;
	int len = 0;
	u_int8_t *buf = NULL;
	uc_linkage_config_rule_excute2_t *request = NULL;

	if (!rule || (home_id == 0)) {
		return RS_NOT_SUPPORT;
	}
	
	CL_CHECK_INIT;

	buf = cl_calloc((strlen(rule) + sizeof(*request) + 100), 1);
	if (!buf) {
		return RS_ERROR;
	}
	request = (uc_linkage_config_rule_excute2_t *)buf;
	request->home_id = home_id;
	request->rule_len = (u_int32_t)strlen(rule) + 1;
	strcpy(request->rule, rule);
	len = sizeof(*request) + request->rule_len;
	ret = cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_PAD_EXCUTE_LOCAL_RULE, 
		(u_int8_t *)request, len);
	cl_free(buf);
	
	return ret;
}

CLIB_API RS cl_la_home_rules_logs_query(u_int32_t home_id, u_int32_t index, u_int32_t count)
{
	ucp_log_rule_query_t request;

	CL_CHECK_INIT;
	request.home_id = home_id;
	request.index = index;
	request.query_count = count;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LOG_RULE_QUERY, 
		(u_int8_t *)&request, sizeof(request));
}

CLIB_API cl_home_log_rule_change_t *cl_la_home_rules_logs_get(u_int32_t home_id)
{
	int len = 0;
	u_int8_t buf[100];
	cl_home_log_rule_change_t *pinfo = NULL;
	
	if (!cl_priv) {
		return NULL;
	}

	*(u_int32_t *)buf = home_id;
	len = sizeof(u_int32_t);
	*(cl_home_log_rule_change_t ***)&buf[len] = &pinfo;
	len += sizeof(cl_home_log_rule_change_t **);

	cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LOG_RULE_GET, 
		buf, len);

	return pinfo;
}

CLIB_API RS cl_la_home_rules_logs_free(cl_home_log_rule_change_t *pinfo)
{
	int i;

	if (pinfo) {
		for(i = 0; i < pinfo->log_count; i++) {
			SAFE_FREE(pinfo->plog[i].pmonitor_sn);
			SAFE_FREE(pinfo->plog[i].paction_sn);
		}
		SAFE_FREE(pinfo->plog);
		cl_free(pinfo);
	}
	
	return RS_OK;
}

CLIB_API RS cl_la_home_members_logs_query(u_int32_t home_id)
{
	ucp_log_member_query_t request;

	CL_CHECK_INIT;
	
	request.home_id = home_id;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LOG_MEM_QUERY, 
		(u_int8_t *)&request, sizeof(request));
}

CLIB_API cl_home_log_member_change_t *cl_la_home_members_logs_get(u_int32_t home_id)
{
	int len = 0;
	u_int8_t buf[100];
	cl_home_log_member_change_t *pinfo = NULL;
	
	if (!cl_priv) {
		return NULL;
	}

	*(u_int32_t *)buf = home_id;
	len = sizeof(u_int32_t);
	*(cl_home_log_member_change_t ***)&buf[len] = &pinfo;
	len += sizeof(cl_home_log_member_change_t **);

	cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_LOG_MEM_GET, 
		buf, len);

	return pinfo;
}

CLIB_API RS cl_la_home_members_logs_free(cl_home_log_member_change_t *pinfo)
{
	if (pinfo) {
		SAFE_FREE(pinfo->plog);
		cl_free(pinfo);
	}
	
	return RS_OK;
}

CLIB_API RS cl_la_widget_key_query()
{
	u_int8_t buff[10];

	CL_CHECK_INIT;
	
	return cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_WIDGET_KEY_QUERY, 
		(u_int8_t *)buff, 1);
}

CLIB_API cl_widget_info_t *cl_la_widget_info_get()
{
	int len = 0;
	u_int8_t buf[100];
	cl_widget_info_t *pinfo = NULL;
	
	if (!cl_priv) {
		return NULL;
	}

	*(cl_widget_info_t ***)buf = &pinfo;
	len = sizeof(cl_widget_info_t **);

	cl_la_send_var_data_notify(&cl_priv->thread_main, 0, CLNE_LA_CTRL, LA_ACTION_WIDGET_INFO_GET, 
		buf, len);

	return pinfo;
}

CLIB_API RS cl_la_widget_info_free(cl_widget_info_t *pinfo)
{
	if (pinfo) {
		cl_free(pinfo);
	}
	
	return RS_OK;
}

