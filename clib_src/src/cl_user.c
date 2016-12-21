#include "cl_priv.h"
#include "cl_user.h"
#include "cl_notify.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "ia_priv.h"
#include "smart_appliance_priv.h"
#include "udp_device_common_priv.h"
#include "cl_ia.h"
#include "cl_smart_appliance.h"
#include "rfgw_priv.h"
#include "rfgw_rftt.h"
#include "linkage_priv.h"

#if 0
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(1)
#include "cl_log_debug.h"
#endif

extern void uc_u64_2_char(u_int8_t *pshare, u_int64_t *pkey);
extern void json_dec(char *src, char *dst);

user_t *cl_user_lookup(const char *username)
{
	user_t *user, *ret = NULL;
	bool is_sn = cl_is_sn(username);
	u_int64_t sn;

	if (is_sn)
		sn = atoll(username);

	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (is_sn) {
			if (user->sn == sn) {
				ret = user;
				goto done;
			}
		} else {
			if (user->nickname != NULL && strcmp(user->nickname, username) == 0) {
				ret = user;
				goto done;
			}
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);

	return ret;
}


////////////////////////////////////////////
// user 外部接口
////////////////////////////////////////////



// 状态变迁时会回调，包括登录成功、成功->失败、离线->在线、在线->离线。回调事件为UE_xxx, 参数为 user_t *指针。
// username是utf8格式的
CLIB_API RS cl_user_login(cl_handle_t *user_handle, const char *username, 
				const char *passwd, cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	user_t *user;
	RS ret;

	CL_CHECK_INIT;

	*user_handle = INVALID_HANDLE;	
	
	// 检查是否重复添加
	cl_lock(&cl_priv->mutex);
	if ((user = cl_user_lookup(username)) != NULL)
		*user_handle = user->handle;
	cl_unlock(&cl_priv->mutex);

	if (*user_handle != INVALID_HANDLE) {
		log_err(false, "user login %s failed: exist\n", username);
		return RS_EXIST;
	}

	pkt = cl_notify_pkt_new(4096, CLNE_USER_LOGIN, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->len_name = (u_int16_t)strlen(username) + 1;
	u->len_passwd = (u_int16_t)strlen(passwd) + 1;
	u->len_license = 0;
	u->callback = callback;
	u->callback_handle = callback_handle;
	strcpy(u->data, username);
	strcpy(u->data + u->len_name, passwd);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	if (ret == RS_OK) {
		cl_lock(&cl_priv->mutex);
		if ((user = cl_user_lookup(username)) != NULL)
			*user_handle = user->handle;
		cl_unlock(&cl_priv->mutex);
	}

	return ret;
}

extern void pu_init_passwd(user_t *user);
static void user_passwd_sync(user_t *user, char *passwd)
{
	u_int8_t passwd_md5[16];
	
	if (!user || !passwd) {
		return;
	}
	hash_passwd(passwd_md5, passwd);
	if (memcmp(passwd_md5, user->passwd_md5, 16) == 0) {
		if (user->is_phone_user) {
			user_set_status(user, CS_AUTH);
		} else {
			user_set_status(user, CS_DISP);
		}
		return;
	}
	SAFE_FREE(user->passwd);
	user->passwd = cl_strdup(passwd);
	memcpy(user->passwd_md5, passwd_md5, 16);
	fmt_hex(user->passwd_md5_str, user->passwd_md5, 16);
	if (user->is_phone_user) {
		pu_init_passwd(user);
		user_set_status(user, CS_AUTH);
	} else {
		user_set_status(user, CS_DISP);
	}
}

CLIB_API RS cl_user_smartconf_login(cl_handle_t *user_handle, const char *username, 
				const char *passwd, cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	user_t *user;
	RS ret;

	CL_CHECK_INIT;

	*user_handle = INVALID_HANDLE;	
	
	// 检查是否重复添加
	cl_lock(&cl_priv->mutex);
	if ((user = cl_user_lookup(username)) != NULL) {
		*user_handle = user->handle;
		//既然是一键配置添加，那就不管设备是否被添加过
		user->home_id = 0;
		user->is_smartconf = true;
	}
	cl_unlock(&cl_priv->mutex);

	if (*user_handle != INVALID_HANDLE) {
		//如果一键配置时，设备是已登陆的，那就修改下密码，因为可能以前的是错误密码
		user_passwd_sync(user, (char *)passwd);
		//尽快reset定时器处理
		if (la_is_valid()) {
			la_comm_timer_reset();
		}
		log_err(false, "user login %s failed: exist\n", username);
		return RS_EXIST;
	}

	pkt = cl_notify_pkt_new(4096, CLNE_USER_LOGIN, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->len_name = (u_int16_t)strlen(username) + 1;
	u->len_passwd = (u_int16_t)strlen(passwd) + 1;
	u->len_license = 0;
	u->callback = callback;
	u->callback_handle = callback_handle;
	u->b_proc = true;
	strcpy(u->data, username);
	strcpy(u->data + u->len_name, passwd);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	if (ret == RS_OK) {
		cl_lock(&cl_priv->mutex);
		if ((user = cl_user_lookup(username)) != NULL) {
			*user_handle = user->handle;
			user->is_smartconf = true;
		}
		cl_unlock(&cl_priv->mutex);
	}

	return ret;
}


CLIB_API RS cl_user_QR_login(cl_handle_t *user_handle, char* username, const char *qr_code,
                             cl_callback_t callback, void *callback_handle)
{
    cl_notify_pkt_t *pkt;
    cln_user_t *u;
    user_t *user;
    RS ret;
	char qr_buff[100];
	u_int32_t ver;
	u_int64_t sn;
	u_int64_t sc;
	char str_sn[100];
	char str_sc[100];
    
    CL_CHECK_INIT;
    
    *user_handle = INVALID_HANDLE;

	//这里做解析
	if (!qr_code || (strlen(qr_code) >= sizeof(qr_buff) - 1)) {
		return RS_ERROR;
	}
	memset(qr_buff, 0, sizeof(qr_buff));
	json_dec((char *)qr_code, qr_buff);
	if (sscanf(qr_buff, "\"v\":\"%u\",\"sn\":\"%"PRIx64"\",\"sc\":\"%"PRIx64"\"", &ver, &sn, &sc) != 3) {
		return RS_ERROR;
	}
	if (ver < 1) {
		return RS_ERROR;
	}
	memset(str_sn, 0, sizeof(str_sn));
	memset(str_sc, 0, sizeof(str_sc));
	//uc_u64_2_char(str_sn, &sn);
	sprintf(str_sn, "%"PRIu64"", sn);
	sprintf(str_sc, "%"PRIu64"", sc);
	
    // 检查是否重复添加
    cl_lock(&cl_priv->mutex);
    if ((user = cl_user_lookup(str_sn)) != NULL)
        *user_handle = user->handle;
    cl_unlock(&cl_priv->mutex);

    if (*user_handle != INVALID_HANDLE) {
        log_err(false, "user login %s failed: exist\n", str_sn);
        return RS_EXIST;
    } 

    pkt = cl_notify_pkt_new(4096, CLNE_QR_CODE_LOGIN, CLNPF_ACK);
    
    u = (cln_user_t *)&pkt->data[0];
    u->len_name = (u_int16_t)strlen(str_sn) + 1;
    u->len_passwd = (u_int16_t)strlen(str_sc) + 1;
    u->len_license = 0;
    u->callback = callback;
    u->callback_handle = callback_handle;
    strcpy(u->data, str_sn);
    strcpy(u->data + u->len_name, str_sc);
    
    pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    if (ret == RS_OK) {
        cl_lock(&cl_priv->mutex);
        if ((user = cl_user_lookup(str_sn)) != NULL)
            *user_handle = user->handle;
        cl_unlock(&cl_priv->mutex);
    }
    
    return ret;
}

// 状态变迁时会回调，包括登录成功、成功->失败、离线->在线、在线->离线。回调事件为UE_xxx, 参数为 user_t *指针。
// username是utf8格式的
CLIB_API RS cl_user_modify_login(cl_handle_t user_handle,
				const char *passwd, cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_MODIFY_LOGIN, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->len_passwd = (u_int16_t)strlen(passwd) + 1;
	u->len_license = 0;
	u->callback = callback;
	u->callback_handle = callback_handle;
	strcpy(u->data + u->len_name, passwd);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_set_direct_login(cl_handle_t user_handle, u_int32_t ip)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_SET_DIRECT_IP, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->ip = ip;
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_relogin(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_RELOGIN, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_logout(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_LOGOUT, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->b_proc = false;
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_qr_logout(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_LOGOUT, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->b_proc = true;	
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}


CLIB_API bool cl_user_same_passwd(cl_handle_t user_handle, const char *passwd)
{
	user_t *user;
	bool same = false;
	
	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_same_passwd: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}
	//圈子设备密码比较
	if (la_is_valid()) {
		u_int8_t pwd_md5[16];
		
		hash_passwd(pwd_md5, (char *)passwd);
		if (memcmp(pwd_md5, user->passwd_md5, 16) == 0) {
			same = true;
		}
		goto done;
	}
	if (strcmp(passwd, user->passwd) == 0) {
		same = true;
		goto done;
	}
	if (user->parent != NULL && user->parent->is_phone_user) {
		u_int8_t pwd_md5[16];
		char pwd_str[64];
		
		hash_passwd(pwd_md5, (char *)passwd);
		fmt_hex(pwd_str, pwd_md5, 16);
		if (strcasecmp(pwd_str, user->passwd) == 0)
			same = true;
	}

done:
	cl_unlock(&cl_priv->mutex);
	return same;
}

// 若修改成功，记得重新登录
CLIB_API RS cl_user_modify_passwd(cl_handle_t user_handle, const char *passwd)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_MODIFY_PASSWD, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->len_name = 0;
	u->len_passwd = (u_int16_t)strlen(passwd) + 1;
	u->len_license = 0;
	strcpy(u->data, passwd);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_modify_nickname(cl_handle_t user_handle, const char *name)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_MODIFY_NICKNAME, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->len_name = (u_int16_t)strlen(name) + 1;
	u->len_passwd = 0;
	u->len_license = 0;
	strcpy(u->data, name);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_slave_modify_name(cl_handle_t slave_handle, const char *new_name)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_SLAVE_MODIFY_NAME, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	u->len_name = (u_int16_t)strlen(new_name) + 1;
	u->len_passwd = 0;
	u->len_license = 0;
	strcpy(u->data, new_name);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

static u_int8_t air_net_wind_2_cl_wind(u_int8_t wind,bool reverse)
{
    if (reverse) {
        if(wind == AC_WIND_3)
            return CL_AC_WIND_LOW;
        if(wind == AC_WIND_1)
            return CL_AC_WIND_HIGH;
        
    }else{
        if(wind == AC_WIND_3)
            return CL_AC_WIND_HIGH;
        
        if(wind == AC_WIND_1)
            return CL_AC_WIND_LOW;
    }
    return wind;
}


static void copy_rfdev(cl_slave_t *si, slave_t *slave)
{
	rfdev_work_t *pos, *n;
	int i;
	
    udp_rf_dev_bulid_slave(slave,si);

    if(slave->ext_type >= RF_EXT_TYPE_LED_LAMP){
        memcpy(&si->rfdev, &slave->dev_info.rf_stat, sizeof(cl_rfdev_status_t));
        si->rfdev.work_list = NULL;
    }
    
    if (slave->ext_type == RF_EXT_TYPE_WK_AIR ||
		slave->ext_type == RF_EXT_TYPE_WK_AIR2) {
        cl_wk_air_info_t* air;
        air = &si->rfdev.dev_priv_data.wk_air_info;
        air->stat.wind = air_net_wind_2_cl_wind(air->stat.wind,false);
    }
    
	stlc_list_count(i, &slave->dev_info.rfdev->work_list);
	
	si->rfdev.ctrl_total = slave->dev_info.rfdev->ctrl_total;
	si->rfdev.ctrl_min = slave->dev_info.rfdev->ctrl_min;
	si->rfdev.ctrl_max = slave->dev_info.rfdev->ctrl_max;;
	si->rfdev.ctrl_ok = slave->dev_info.rfdev->ctrl_ok;
	si->rfdev.ctrl_fail = slave->dev_info.rfdev->ctrl_fail;
	si->rfdev.ctrl_msec = slave->dev_info.rfdev->ctrl_msec;
	si->rfdev.rfrx = slave->dev_info.rfdev->rfrx;
	si->rfdev.linkretry = slave->dev_info.rfdev->linkretry;
	si->rfdev.work = slave->dev_info.rfdev->work;
	si->rfdev.is_ctrl = slave->dev_info.rfdev->is_ctrl;
	si->rfdev.work_list = NULL;
	stlc_list_count(si->rfdev.work_list_cnt, &slave->dev_info.rfdev->work_list);
	if(si->rfdev.work_list_cnt > 0){
		si->rfdev.work_list = cl_calloc(sizeof(cl_rfdev_work_t), si->rfdev.work_list_cnt);
		i = 0;
		stlc_list_for_each_entry_safe(rfdev_work_t, pos, n, &slave->dev_info.rfdev->work_list, link){
			si->rfdev.work_list[i].work = pos->work;
			si->rfdev.work_list[i].time = pos->time;
			i++;
			stlc_list_del(&pos->link);
			cl_free(pos);
		}
	}
}

static void copy_slave_info(cl_slave_t *si, slave_t *slave)
{
	dev_info_t *di;
	u_int32_t now;

	now = get_sec();

	si->dev_type = slave->sub_type;
	si->ext_type = slave->ext_type;
	si->bind_error = slave->bind_error;
	si->other_master_sn = slave->other_master_sn;
	si->is_udp = slave->is_udp;
	si->run_time = slave->run_time;
	
	di = &slave->dev_info;
	if (di->query_uptime != 0)
		si->uptime = di->uptime + (now - di->query_uptime);
	if (di->query_online != 0)
		si->online = di->online + (now - di->query_online);
	if (di->query_conn_internet != 0)
		si->conn_internet = di->conn_internet + (now - di->query_conn_internet);
	memcpy(&si->soft_version, &di->version.soft_version, sizeof(si->soft_version));
	memcpy(&si->upgrade_version, &di->version.upgrade_version, sizeof(si->upgrade_version));
	memcpy(&si->rf_stm_ver, &di->rf_stm_ver, sizeof(si->rf_stm_ver));
	if(slave->dev_info.rfdev){
		copy_rfdev(si, slave);
	}

	//log_debug("copy_slave_info sn=%"PRIu64" !!!!!!!!!!!!!!!!!!!!!!1\n", slave->sn);
	//快捷开关copy
	si->is_support_public_shortcuts_onoff = slave->is_support_public_shortcuts_onoff;
	si->is_support_dbc = slave->is_support_dbc;
	memcpy((void *)&si->shortcuts_onoff, (void *)&slave->shortcuts_onoff, sizeof(si->shortcuts_onoff));

	si->has_recv_flag_pkt = slave->has_recv_flag_pkt;
	si->is_support_la = slave->is_support_la;
	
	//这里判断一下，根据类型来判断是否支持快捷定时器
	switch(si->ext_type) {
	case RF_EXT_TYPE_WK_AIR2:
	case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_HEATING_VALVE:
		si->is_support_public_shortcuts_onoff = slave->is_support_public_shortcuts_onoff = 1;
		break;
	default:
		break;
	}
}

static void count_dev_info(user_t *user, cl_dev_info_t *ui)
{
	int i;
	slave_t *slave;
	mod_t *mod;
	
	i = 0;
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		i++;

		if (slave->status != BMS_BIND_ONLINE)
			continue;

		if (slave->equipment != NULL) {
			int count;
			stlc_list_count(count, &slave->equipment->eq_list);
			ui->num_equipment += count;
		}

		stlc_list_for_each_entry(mod_t, mod, &slave->mod_list, link) {
			if (MID_USB_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_USB_VIDEO_MAX) {
				if (mod->flags & MTF_USB_VIDEO_IN) {
					ui->num_usb_video++;
				} 
			} else if (MID_PLUG_MIN <= mod->mod_id && mod->mod_id <= MID_PLUG_MAX) {
			} else if (MID_GNET == mod->mod_id) {
				ui->has_green_net = true;
			} else if ((MID_MJPG_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_MJPG_VIDEO_MAX)
						|| (MID_H264_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_H264_VIDEO_MAX) )
			{
			} else if (MID_BELTER == mod->mod_id) {
				ui->has_belter = true;
			} else if (MID_ALARM == mod->mod_id) {
				ui->has_alarm = true;
			} else if (MID_SCENE_SUPPORT == mod->mod_id) {
				ui->has_scene = true;
				if (mod->flags & MFK_SCENE_TIMER)
					ui->has_scene_timer = true;
				if (mod->flags & MFK_SCENE_LINKAGE_ALARM)
					ui->has_scene_linkage_alarm = true;
			} else if (AREA_FUNC_SUPPORT == mod->mod_id) {
				ui->has_area = true;
			}
		}
	}

	ui->num_objs = ui->num_usb_video + ui->num_slave + ui->num_equipment;
	ui->objs = cl_calloc(sizeof(void *), ui->num_objs);

	ui->idx_slave = 0;
	ui->idx_usb_video = ui->idx_slave + ui->num_slave;
	ui->idx_equipment = ui->idx_usb_video + ui->num_usb_video;
}

static void init_obj_base(cl_obj_t *obj, slave_t *slave, cl_handle_t handle, object_type_t type, char *name)
{
	obj->type = type;
	obj->status = slave->status;
	obj->sn = slave->sn;
	obj->handle = handle;
	obj->name = cl_strdup(name == NULL ? "" : name);
}

void sort_insert(slave_t *slave, struct stlc_list_head *head)
{
	slave_t *pos;

	stlc_list_for_each_entry(slave_t, pos, head, link){
		if(slave->sn <= pos->sn){
			stlc_list_add_prev(&slave->link, &pos->link);
			return;
		}
	}
	stlc_list_add_tail(&slave->link, head);
}

void sort_slave(user_t *user)
{
	struct stlc_list_head head;
	slave_t *slave, *n;
	

	STLC_INIT_LIST_HEAD(&head);
	if(stlc_list_empty(&user->slave))
		return;

	stlc_list_for_each_entry_safe(slave_t , slave, n, &user->slave, link){
		stlc_list_del(&slave->link);
		sort_insert(slave, &head);
	}
	stlc_list_splice(&head, &user->slave);
}


/**
	查找一个从设备不是未绑定状态
*/
static bool slave_is_not_new(u_int64_t sn)
{	
	user_t *user = NULL;
	slave_t *slave = NULL;
	bool not_new = false;
	
	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->sn != sn) {
				continue;
			}

			if (slave->status > BMS_UNBIND) {
				not_new = true;
				goto done;
			}
		}
	}

done:
	cl_unlock(&cl_priv->mutex);

	return not_new;
}

/**
	同一个从设备在多个网关下面显示，遵循以下原则
	在线> 绑定中 > 其他状态(未绑定、 绑定离线)
*/
static bool slave_need_hide(slave_t *cslave)
{	
	user_t *user = NULL;
	slave_t *slave = NULL;
	cl_bms_t count[BMS_MAX] = {0}, slave_count = 0;
	cl_bms_t first_bms = BMS_MAX;

	cl_lock(&cl_priv->mutex);

	// 先统计有多少状态
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->sn != cslave->sn) {
				continue;
			}

			slave_count++;

			if (slave->status >= BMS_MAX) {
				continue;
			}

			count[slave->status]++;
		}
	}


	cl_unlock(&cl_priv->mutex);

	// 唯一的设备需要显示
	if (slave_count < 2) {
		return false;
	}

	if (count[BMS_BIND_ONLINE] > 0) {
		first_bms = BMS_BIND_ONLINE;
	} else if (count[BMS_BINDING] > 0) {
		first_bms = BMS_BINDING;
	} else {
		return false;
	}	

	if (cslave->status != first_bms) {
		return true;
	}

	return false;
}


CLIB_API cl_slave_t *cl_get_slave_info(cl_handle_t handle)
{
	slave_t *slave;
	cl_slave_t *csp = NULL;

	cl_lock(&cl_priv->mutex);

	slave = lookup_by_handle(HDLT_SLAVE, handle);
    if (!slave) {
        goto done;
    }

	// 只能是RF网关下面的从设备
	if (slave->sub_type != IJ_RFGW || slave->ext_type <= 1) {
		goto done;
	}

	csp = cl_calloc(sizeof(cl_slave_t), 1);
	if (!csp) {
		goto done;
	}

	// 拷贝主设备的handle
	csp->master_handle = slave->user->handle;
	
	init_obj_base(&csp->obj, slave, slave->handle, OT_SLAVE, slave->name);
	copy_slave_info(csp, slave);

	csp->status_valid = slave->status_valid;

	memcpy(csp->developer_id, slave->developer_id, 32);

	csp->area_handle = area_get_handle_by_id(slave->user, slave->area_id);
	
	// 设置从设备局部变量
	csp->has_rf = slave->has_rf;
	csp->has_ir = slave->has_ir;
	csp->is_805_beep_on = slave->is_805_beep_on;
	csp->is_805_screen_on = slave->is_805_screen_on;
	csp->has_alarm = slave->has_alarm;
	csp->has_alarm_swich = slave->has_alarm_swich;
	csp->has_eq_add_by_json = slave->has_eq_add_by_json;
    csp->has_v4l2_color_setting = slave->has_v4l2_color_setting;
    csp->has_roll_speed_ctrl = slave->has_roll_speed_ctrl;
	csp->is_upgrade_err = slave->is_upgrade_err;
    csp->has_ptz =  slave->has_ptz;

done:

	cl_unlock(&cl_priv->mutex);
	
	return csp;
}

CLIB_API void cl_free_slave_info(cl_slave_t *slave)
{
	_cl_rfdev_free(slave);
	SAFE_FREE(slave->obj.name);
	cl_free(slave);
}


CLIB_API cl_dev_info_t *cl_user_get_dev_info(cl_handle_t dev_handle)
{
	int idx_usb_video = 0, idx_eq = 0, idx_slave = 0,idx_aphone;
    char* phone;
	
	cl_dev_info_t *di = NULL;
	
	user_t *user;
	slave_t *slave;
	mod_t *mod;

	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
//		log_err(false, "cl_user_get_dev_info: lookup user handle=0x%08x failed\n", dev_handle);
		goto done;
	}
	
	di = cl_calloc(sizeof(cl_dev_info_t), 1);

	di->handle = user->handle;
	di->name = cl_strdup(user->name);
	di->sn = user->sn;
	di->nickname = cl_strdup(user->nickname == NULL ? "" : user->nickname);
	di->passwd = cl_strdup(user->passwd);

	di->home_id = user->home_id;
	di->dev_ip = user->dev_ip;
	di->upgrade_version = user->upgrade_version;
	
	di->is_udp_ctrl = user->is_udp_ctrl;
	
	di->is_login = (user->status == CS_ESTABLISH);
	di->is_online = user->online;
	di->last_err = user->last_err;
	di->display_stat = user->last_display_stat;
	di->sub_type = user->sub_type;
	di->ext_type = user->ext_type;
	memcpy(di->developer_id, user->developer_id, 32);
	di->vendor_id = cl_strdup(user->vendor_id == NULL ? "" : user->vendor_id);
	di->vendor_url = cl_strdup(user->vendor_url == NULL ? "" : user->vendor_url);

	di->dev_ver_is_valid = user->dev_ver_is_valid;
	di->dev_ver_is_too_low = user->dev_ver_is_too_low;
	di->is_support_telnet = user->is_support_telnet;
	
	if(user->bp->login_type && user->bp->net_type)
		di->can_bind_phone = 1;
	else
		di->can_bind_phone = 0;
	di->login_type = user->bp->login_type;
	di->net_type = user->bp->net_type;
	di->has_db_rf = user->has_db_rf;
	    
    if (user->ap && user->ap->nacp && user->ap->nacp->count>0 ) {
        di->phone_list = cl_calloc(sizeof(char*)*user->ap->nacp->count, 1);
        if (di->phone_list) {
            phone = (char*)user->ap->nacp->phones;
            for (idx_aphone = 0; idx_aphone<user->ap->nacp->count;idx_aphone++,phone+=ALARM_PHONE_LENGTH) {
                phone[ALARM_PHONE_LENGTH-1]='\0';
                di->phone_list[idx_aphone] = cl_strdup(phone);
            }
            di->num_alarm_conf_phone = user->ap->nacp->count;
        }
    }
    
	area_build_objs(user, di);
    scene_build_objs(user,di);
    ia_build_objs(user, di);
    sa_build_objs(user, di);
	comm_timer_build(user, di);
	memcpy(&di->if_info, &user->if_info, sizeof(cl_if_info_t));
	//stlc_list_count(di->num_slave, &user->slave);

	// 计算从设备个数，需要过滤掉一些情况
	di->num_slave = 0;
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		// 如果当前设备下的从设备是未绑定状态，但是在其他设备下有个相同SN的从设备不是未绑定状态，
		// 需要过滤下
		if (slave_need_hide(slave) == true) {
			continue;
		}
		di->num_slave += 1;
	}
	
	if (di->num_slave == 0)
		goto done;	

	count_dev_info(user, di);
#ifdef WIN32
#ifdef _DEBUG
	// windows下调试方便观察，从设备排序
	sort_slave(user);
#endif
#endif
	// 构造对象数组，注意有些地方名字用模块名字，有些用设备名字
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		cl_slave_t *csp;

		if (slave_need_hide(slave) == true) {
			continue;
		}

		csp = cl_calloc(sizeof(cl_slave_t), 1);
		di->objs[di->idx_slave + idx_slave] = &csp->obj;
		idx_slave++;
				
		init_obj_base(&csp->obj, slave, slave->handle, OT_SLAVE, slave->name);
		copy_slave_info(csp, slave);

		csp->area_handle = area_get_handle_by_id(slave->user, slave->area_id);

		csp->status_valid = slave->status_valid;

		log_debug(" <<<< slave %"PRIu64" copy stat_valid %u\n", slave->sn, slave->status_valid);

		memcpy(csp->developer_id, slave->developer_id, 32);
		//if (slave->status != BMS_BIND_ONLINE)
		//	continue;

		// 设置从设备局部变量
		csp->has_rf = slave->has_rf;
		csp->has_ir = slave->has_ir;
		csp->is_805_beep_on = slave->is_805_beep_on;
		csp->is_805_screen_on = slave->is_805_screen_on;
		csp->has_alarm = slave->has_alarm;
		csp->has_alarm_swich = slave->has_alarm_swich;
		csp->has_eq_add_by_json = slave->has_eq_add_by_json;
        csp->has_v4l2_color_setting = slave->has_v4l2_color_setting;
        csp->has_roll_speed_ctrl = slave->has_roll_speed_ctrl;
		csp->is_upgrade_err = slave->is_upgrade_err;
        csp->has_ptz =  slave->has_ptz;

		//联动支持
		csp->is_support_la = slave->is_support_la;
		csp->has_recv_flag_pkt = slave->has_recv_flag_pkt;

		// 设置全局标志
		di->has_rf |= csp->has_rf;
		di->has_ir |= csp->has_ir;
		di->has_alarm |= csp->has_alarm;
		di->has_alarm_swich |= csp->has_alarm_swich;
		di->has_eq_add_by_json |= csp->has_eq_add_by_json;
		di->has_eq_gencode  |=  slave->has_eq_gencode;		

		// 如果不在线，下面的USB摄像头和EQ设备没分配内存的
		if (slave->status != BMS_BIND_ONLINE) {
			continue;
		}

		stlc_list_for_each_entry(mod_t, mod, &slave->mod_list, link) {
			if (MID_USB_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_USB_VIDEO_MAX) { /* USB摄像头 */
				if (mod->flags & MTF_USB_VIDEO_IN) {
					cl_usb_video_t *cuv;

					cuv = cl_calloc(sizeof(cl_usb_video_t), 1);
					di->objs[di->idx_usb_video + idx_usb_video] = &cuv->obj;
					idx_usb_video++;

					init_obj_base(&cuv->obj, slave, slave->video->handle, OT_USB_VIDEO, mod->name);
				} 
			} else if ((MID_MJPG_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_MJPG_VIDEO_MAX)
						|| (MID_H264_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_H264_VIDEO_MAX) )
			{ /* 支持云台的摄像头 */
				csp->has_video_record = !(!(mod->flags & MTF_VIDEO_RECORD));
				csp->has_video_flip = !(!(mod->flags & MTF_VIDEO_FLIP));
			} else if (MID_PLUG_MIN <= mod->mod_id && mod->mod_id <= MID_PLUG_MAX) { /* 遥控插座 */
				csp->has_current_detect = !(!(mod->flags & MTF_VC_DETECT));
				csp->has_electric_stat = !(!(mod->flags & MTF_PLUG_ELECTRIC_STAT));
			}
		}

		eq_build_objs(di, slave, &idx_eq);
	}
	
done:
	cl_unlock(&cl_priv->mutex);
	
	return di;
}

CLIB_API void cl_user_free_dev_info(cl_dev_info_t *di)
{
	int i, k;
    
    if (di->phone_list) {
        for (i=0; i< (int)di->num_alarm_conf_phone; i++) {
            SAFE_FREE(di->phone_list[i]);
        }
        SAFE_FREE(di->phone_list);
    }

	for (i = 0; i < (int)di->num_objs; i++) {
		if (di->objs[i]->type == OT_EQUIPMENT) {
			eq_free_objs((cl_equipment_t *)di->objs[i]);
		}
		if (di->objs[i]->type == OT_SLAVE) {
			_cl_rfdev_free((cl_slave_t*)di->objs[i]);
		}
		SAFE_FREE(di->objs[i]->name);
		cl_free(di->objs[i]);
	}
	SAFE_FREE(di->objs);
	SAFE_FREE(di->vendor_id);
 	SAFE_FREE(di->vendor_url);
   
    if (di->areas) {
        for (i=0; i<(int)di->num_area; i++) {
            SAFE_FREE(di->areas[i]);
        }
        cl_free(di->areas);
        di->areas = NULL;
    }
    
    if (di->scenes) {
        for (i=0; i<(int)di->num_scene; i++) {
			cl_scene_t *scene;
			scene = di->scenes[i];
			for (k = 0; k < scene->event_count; k++) {
				SAFE_FREE(scene->events[k]);
			}
			scene_free_timer(scene);
            SAFE_FREE(scene);
        }
        cl_free(di->scenes);
        di->scenes = NULL;
    }
    
    if (di->air_info) {
        SAFE_FREE(di->air_info->air_timer_info.timers);
	  SAFE_FREE(di->air_info->air_timer_info.period_timers);
	 SAFE_FREE(di->air_info->elec_days_info.elec_data); 
	 SAFE_FREE(di->air_info->last_match_info.items); 
	 SAFE_FREE(di->air_info->key_info.keys);
	 SAFE_FREE(di->air_info->temp_curve);
        SAFE_FREE(di->air_info->priv_rc.stb_info.uk);
        SAFE_FREE(di->air_info->priv_rc.stb_info.fk);
        SAFE_FREE(di->air_info->priv_rc.tv_info.uk);
        SAFE_FREE(di->air_info->priv_rc.tv_info.fk);
        SAFE_FREE(di->air_info->share_info.records);
		SAFE_FREE(di->air_info->requested_share_code);
        SAFE_FREE(di->air_info);
    }
    SAFE_FREE(di->ah_info);

	if (di->eb_info != NULL) {
		SAFE_FREE(di->eb_info->timer.timers);
		SAFE_FREE(di->eb_info->timer.period_timers);
		SAFE_FREE(di->eb_info->elec_info.elec_days_info.elec_data);
		SAFE_FREE(di->eb_info);
	}

	if(di->com_udp_info != NULL){
		udp_free_objs_hook(di->com_udp_info);
		SAFE_FREE(di->com_udp_info);
		di->com_udp_info = NULL;
	}
    

	SAFE_FREE(di->ia_info.u.aircleaner_info);
	SAFE_FREE(di->name);
	cl_free(di->nickname);
	cl_free(di->passwd);

	

	cl_free(di);
}

CLIB_API RS cl_user_set_click(cl_handle_t dev_handle, u_int8_t *stat)
{
	user_t *user;
	slave_t *slave = NULL;
	time_t now = time(NULL);
	RS ret = RS_OK;
	bool found = false, modify = false;;
	
	cl_lock(&cl_priv->mutex);

	// 网关
	if ((user = lookup_by_handle(HDLT_USER, dev_handle)) != NULL) {
		found = true;
		user->last_click_time = now;

		if (!user->online) {
			ret = RS_ERROR;
			if (user->last_display_stat != DISPLAY_STAT_CONNECTING) {
				user->last_display_stat = DISPLAY_STAT_CONNECTING;
				modify = true;
			}
			
		}

		if (user->last_err == ULGE_BAD_PASSWORD) {
			user->last_display_stat = DISPLAY_STAT_REAL_STAT;
			modify = true;	
		}

		*stat = user->last_display_stat;

		// 如果网关不在线或者不是假在线，下面的从设备之前在线的，全部变成登录中
		if (!(user->online || user->last_display_stat == DISPLAY_STAT_VIRTUAL_ONLINE)) {
			stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
				if (slave->status == BMS_BIND_ONLINE) {					
					slave->status = BMS_LOGINING;
					modify = true;
				}
			}
		}

		if (modify) {		
			event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
		}
				
		goto done;
	}

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->handle == dev_handle) {
				found = true;
				goto found;
			}
		}
	}

	if (!found) {
		ret = RS_INVALID_PARAM;
		goto done;
	}

found:
	// 没有获取到从设备在线信息，原来在线的改成登录中
	if (slave->status_valid == false) {
		if (slave->status == BMS_BIND_ONLINE) {
			*stat = slave->status = BMS_LOGINING;				
			event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
		}
		ret = RS_ERROR;		
	}

done:
	cl_unlock(&cl_priv->mutex);

	user_request_display_info_save();		

	return ret;
}


CLIB_API cl_user_t *clsdk_get_user_info(cl_handle_t user_handle) 
{
	user_t *user;
	cl_user_t *cu = NULL;

	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "clsdk_user_get_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	cu = (cl_user_t *)cl_calloc(sizeof(cl_user_t), 1);

	cu->username = cl_strdup(user->name);
	cu->is_phone_user = user->is_phone_user;
	cu->passwd = cl_strdup(user->passwd);
	cu->passwd_md5_str = cl_strdup(user->passwd_md5_str);
	cu->is_login = (user->status == CS_ESTABLISH);
	cu->last_err = user->last_err;
	cu->user_handle = user_handle;

	cu->sub_type = user->sub_type;
	cu->ext_type = user->ext_type;
	cu->vendor_id = cl_strdup(user->vendor_id == NULL ? "" : user->vendor_id);
	cu->vendor_url = cl_strdup(user->vendor_url == NULL ? "" : user->vendor_url);
done:
	cl_unlock(&cl_priv->mutex);
	
	return cu;
}

CLIB_API cl_user_t *cl_user_get_info(cl_handle_t user_handle)
{
	int i;
	user_t *user;
	cl_user_t *cu = NULL;
	user_t *dev;

	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		if (lookup_by_handle(HDLT_LINKAGE, user_handle)) {
			cu = cl_la_get_user(user_handle);
			goto done;
		}
		if (plc->p_null_home && (plc->p_null_home->handle == user_handle)) {
			cu = la_ctrl_get_invalid_home();
			goto done;
		}
		log_err(false, "cl_user_get_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	cu = (cl_user_t *)cl_calloc(sizeof(cl_user_t), 1);

	cu->username = cl_strdup(user->name);
	cu->is_phone_user = user->is_phone_user;
	cu->passwd = cl_strdup(user->passwd);
	cu->passwd_md5_str = cl_strdup(user->passwd_md5_str);
	cu->is_login = (user->status == CS_ESTABLISH);
	cu->last_err = user->last_err;
	cu->user_handle = user_handle;
	cu->is_support_spe_up = user->is_support_spe_up;

	cu->sub_type = user->sub_type;
	cu->ext_type = user->ext_type;
	cu->vendor_id = cl_strdup(user->vendor_id == NULL ? "" : user->vendor_id);
	cu->vendor_url = cl_strdup(user->vendor_url == NULL ? "" : user->vendor_url);
	
	if (user->is_phone_user) {
		stlc_list_count(cu->num_dev, &user->dev);
		if (cu->num_dev > 0) {
			cu->dev = cl_calloc(sizeof(cl_dev_info_t *), cu->num_dev);
		}
		i = 0;
		stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
			cu->dev[i++] = cl_user_get_dev_info(dev->handle);
		}
	} else {
		cu->num_dev = 1;
		cu->dev = cl_calloc(sizeof(cl_dev_info_t *), cu->num_dev);
		cu->dev[0] = cl_user_get_dev_info(user_handle);
	}

done:
	cl_unlock(&cl_priv->mutex);
	return cu;
}

CLIB_API void cl_user_free_info(cl_user_t *info)
{
	unsigned int i;

	//释放联动信息
	if (info->is_la) {
		cll_home_free(&info->home);
	}

	for (i = 0; i < info->num_dev; i++) {
		cl_user_free_dev_info(info->dev[i]);
	}
	SAFE_FREE(info->username);
	SAFE_FREE(info->dev);
	SAFE_FREE(info->passwd);
	SAFE_FREE(info->passwd_md5_str);
	SAFE_FREE(info->vendor_id);
	SAFE_FREE(info->vendor_url);
	cl_free(info);
}

/*
	功能：
		绑定从设备
	输入参数:
		@slave_handle: 从设备句柄
		@passwd: 绑定密码
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
*/
CLIB_API RS cl_slave_bind(cl_handle_t slave_handle, const char *passwd)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_SLAVE_BIND, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	u->len_passwd = (u_int16_t)(strlen(passwd) + 1);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	strcpy(u->data, passwd);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	功能：
		解绑定从设备
	输入参数:
		@slave_handle: 从设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
*/
CLIB_API RS cl_slave_unbind(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_SLAVE_UNBIND, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	功能：
		手机帐号添加一个设备
	输入参数:
		@user_handle: 手机帐号句柄
		@dev_name: 要添加的设备的昵称或SN号
		@passwd: 设备的密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_add_dev(cl_handle_t user_handle, const char *dev_name, const char *passwd)
{
	return cl_user_add_dev_v2(user_handle, NULL, dev_name, passwd);
}


CLIB_API RS cl_user_add_QR_dev_v2(cl_handle_t user_handle, cl_handle_t *dev_handle, const char *dev_sn, const char *QR_code)
{
    cl_notify_pkt_t *pkt;
    cln_user_t *u;
    RS ret;
    
    CL_CHECK_INIT;
    
    pkt = cl_notify_pkt_new(4096, CLNE_USER_ADD_DEV, CLNPF_ACK);
    
    u = (cln_user_t *)&pkt->data[0];
    u->user_handle = user_handle;
    u->callback_handle = dev_handle;
    u->len_name = (u_int16_t)(strlen(dev_sn) + 1);
    u->len_passwd = (u_int16_t)(strlen(QR_code) + 1);
    
    pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;
    
    strcpy(u->data, dev_sn);
    strcpy(u->data + u->len_name, QR_code);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
    
}

CLIB_API RS cl_user_add_QR_dev(cl_handle_t user_handle, const char *dev_sn, const char *QR_code)
{
    return cl_user_add_QR_dev_v2(user_handle, NULL, dev_sn, QR_code);
}

/*
	功能：
		手机帐号添加一个设备
	输入参数:
		@user_handle: 手机帐号句柄
		@dev_handle: 保存本次添加的设备的句柄
		@dev_name: 要添加的设备的昵称或SN号
		@passwd: 设备的密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_add_dev_v2(cl_handle_t user_handle, cl_handle_t *dev_handle, const char *dev_name, const char *passwd)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_ADD_DEV, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->callback_handle = dev_handle;
	u->len_name = (u_int16_t)(strlen(dev_name) + 1);
	u->len_passwd = (u_int16_t)(strlen(passwd) + 1);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	strcpy(u->data, dev_name);
	strcpy(u->data + u->len_name, passwd);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;

}

/*
	功能：
		手机帐号删除一个设备
	输入参数:
		@dev_handle: 要删除的设备的句柄
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_del_dev(cl_handle_t dev_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_DEL_DEV, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = dev_handle;
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	功能：
		注册一个手机用户账号
	输入参数:
		@phone_number: 手机号码
		@callback: 回调函数。回调事件为 
			UE_PHONE_USER_REGISTER_OK、UE_PHONE_USER_REGISTER_FAILED
		@callback_handle: 回调时回传回来的
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_register(const char *phone_number, 
				cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_REGISTER, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->len_name = (u_int16_t)strlen(phone_number) + 1;
	u->len_passwd = 0;
	u->len_license = 0;
	u->callback = callback;
	u->callback_handle = callback_handle;
	strcpy(u->data, phone_number);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	功能：
		重置手机账号密码
	输入参数:
		@phone_number: 手机号码
		@callback: 回调函数。回调事件为 
		@callback_handle: 回调时回传回来的
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_PHONE_USER_RESET_OK/UE_PHONE_USER_RESET_FAILED。
		如果是UE_PHONE_USER_RESET_OK，需要用户输入验证码、新密码
*/
CLIB_API RS cl_user_reset_passwd(const char *phone_number, 
				cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_RESET, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->len_name = (u_int16_t)strlen(phone_number) + 1;
	u->len_passwd = 0;
	u->len_license = 0;
	u->callback = callback;
	u->callback_handle = callback_handle;
	strcpy(u->data, phone_number);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	功能：
		手机用户发送验证码
	输入参数:
		@phone_number: 手机号码
		@password: 密码
		@vcode: 认证码
		@callback: 回调函数。回调事件为 
			UE_PHONE_USER_GOOD_VCODE、UE_PHONE_USER_GOOD_FAILED
		@callback_handle: 回调时回传回来的
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_send_vcode(const char *phone_number, 
				const char *password, const char *vcode,
				cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_SEND_VCODE, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->len_name = (u_int16_t)strlen(phone_number) + 1;
	u->len_passwd = (u_int16_t)strlen(password) + 1;
	u->len_license = (u_int16_t)strlen(vcode) + 1;
	u->callback = callback;
	u->callback_handle = callback_handle;
	
	strcpy(u->data, phone_number);
	strcpy(u->data + u->len_name, password);
	strcpy(u->data + u->len_name + u->len_passwd, vcode);

	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd + u->len_license;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_apns_config(cl_handle_t dev_handle, cl_apns_config_t *cfg)
{
	cl_notify_pkt_t *pkt;
	cln_apns_config_t *u;
	RS ret;

	CL_CHECK_INIT;

	if(cfg == NULL)
		return RS_INVALID_PARAM;
	if(cfg->action < ACTION_QUERY || cfg->action > ACTION_DEL)
		return RS_INVALID_PARAM;
	if(cfg->token_len < 32 && cfg->regid[0] == 0)
		return RS_INVALID_PARAM;

	cfg->phone_ver[sizeof(cfg->phone_ver)-1] = 0;
	cfg->msg_prefix[sizeof(cfg->msg_prefix)-1] = 0;

	pkt = cl_notify_pkt_new(4096, CLNE_APNS_CONFIG, CLNPF_ACK);	
	if(pkt == NULL)
		return RS_MEMORY_MALLOC_FAIL;
	
	u = (cln_apns_config_t *)&pkt->data[0];
	u->user_handle = dev_handle;
	memcpy(&u->cfg, cfg, sizeof(*cfg));	
	pkt->param_len = sizeof(*u);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);	
	cl_notify_pkt_free(pkt);
	return ret;
}

CLIB_API RS cl_user_apns_config_by_sn(u_int64_t sn, cl_apns_config_t *cfg)
{
	cl_notify_pkt_t *pkt;
	cln_apns_config_v2_t *u;
	RS ret;

	CL_CHECK_INIT;

	if(cfg == NULL)
		return RS_INVALID_PARAM;
	if(cfg->action < ACTION_QUERY || cfg->action > ACTION_DEL)
		return RS_INVALID_PARAM;
	if(cfg->token_len < 32 )
		return RS_INVALID_PARAM;

	cfg->phone_ver[sizeof(cfg->phone_ver)-1] = 0;
	cfg->msg_prefix[sizeof(cfg->msg_prefix)-1] = 0;

	pkt = cl_notify_pkt_new(4096, CLNE_APNS_CONFIG_BY_SN, CLNPF_ACK);	
	if(pkt == NULL)
		return RS_MEMORY_MALLOC_FAIL;
	
	u = (cln_apns_config_v2_t *)&pkt->data[0];
	u->sn = sn;
	memcpy(&u->cfg, cfg, sizeof(*cfg));	
	pkt->param_len = sizeof(*u);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);	
	cl_notify_pkt_free(pkt);
	return ret;
}


CLIB_API RS cl_user_apns_config_get(cl_handle_t dev_handle, cl_apns_config_t * cfg)
{
	user_t *user;
	RS ret = RS_OK;

	if(cfg == NULL)
		return RS_INVALID_PARAM;
	
	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "cl_user_apns_config_get: lookup user handle=0x%08x failed\n", dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if(user->apns_cfg == NULL){
		ret = RS_EMPTY;
		goto done;
	}
	memcpy(cfg, user->apns_cfg, sizeof(*cfg));

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}

CLIB_API RS cl_user_background(cl_handle_t user_handle, bool background)
{
	user_t *user;
	RS ret = RS_OK;
	
	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
    
    //相同，直接返回
    if ((!user->background && !background) ||
        (user->background && background)) {
        goto done;
    }
    
	user->background = background;
    
    printf("%s: handle=0x%08x background %u\n", __FUNCTION__, user_handle,user->background);

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}

CLIB_API RS cl_slave_open_telnet(cl_handle_t slave_handle, u_int32_t ip, int port)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_SLAVE_OPEN_TELNET, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	u->ip = ip;
	u->port = port;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_slave_reboot(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_SLAVE_REBOOT, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_805_set_screen_on_off(cl_handle_t slave_handle,bool on_off)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_DEV_805_CONFIG, ACT_805_SCREEN_CTRL, !(!on_off));
}

CLIB_API RS cl_805_set_beep_on_off(cl_handle_t slave_handle,bool on_off)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_DEV_805_CONFIG, ACT_805_BEEP_CTRL, !(!on_off));
}

CLIB_API RS cl_805_query_screen_on_off(cl_handle_t slave_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_DEV_805_CONFIG, ACT_805_SCREEN_QUERY, 0);
}

CLIB_API RS cl_805_query_beep_on_off(cl_handle_t slave_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_DEV_805_CONFIG, ACT_805_BEEP_QUERY,0);
}

CLIB_API RS cl_dev_upgrade_check(cl_handle_t handle, int lang)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	if ((lang <= LANG_BEGIN) || (lang >= LANG_MAX))
		return RS_INVALID_PARAM;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_Q, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->ip = lang;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_dev_update_set(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_SET, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_dev_update_now(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_NOW, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = slave_handle;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_dev_update_cli(cl_handle_t handle,char *filename)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_CLI, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len_name = (u_int16_t)strlen(filename) + 1;
	strcpy(u->data, filename);	
	pkt->param_len = sizeof(cln_user_t)+u->len_name;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}
	
CLIB_API RS cl_dev_update_cli_no_head(cl_handle_t handle,char *filename)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_CLI_NO_HEARD, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len_name = (u_int16_t)strlen(filename) + 1;
	strcpy(u->data, filename);	
	pkt->param_len = sizeof(cln_user_t)+u->len_name;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}


CLIB_API RS cl_dev_stm_update_cli(cl_handle_t handle, char *filename)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_STM, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len_name = (u_int16_t)strlen(filename) + 1;
	strcpy(u->data, filename);	
	pkt->param_len = sizeof(cln_user_t)+u->len_name;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_dev_stm_update_spe_cli(cl_handle_t handle, char *filename, u_int8_t num, u_int64_t *sn, u_int8_t force)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	u_int8_t *pdata;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_STM_SPE_UP, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->b_proc = force;
	u->len_name = (u_int16_t)strlen(filename) + 1;
	strcpy(u->data, filename);	
	pkt->param_len = sizeof(cln_user_t) + u->len_name;

	pdata = u->data;
	pdata[u->len_name] = num;
	pkt->param_len++;
	if (num) {
		memcpy(&pdata[u->len_name + 1], (void *)sn, num*sizeof(u_int64_t));
		pkt->param_len += num*sizeof(u_int64_t);
	}

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_dev_flash_upgrade(cl_handle_t handle, u_int32_t num)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;		


	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_FLASH_UPGRADE, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->num = num;
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);
	

	return ret;
}

CLIB_API RS cl_dev_flash_query(cl_handle_t handle, u_int32_t *flash_size)
{
	user_t *user;

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, handle)) == NULL) {
		log_debug("not find user\n");
		return RS_ERROR;
	}

	*flash_size = user->flash_size;

	return RS_OK;
}


CLIB_API RS cl_dev_flash_upgrade_query(cl_handle_t handle, flash_block_t *block)
{
	user_t *user;

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, handle)) == NULL) {
		log_debug("not find user\n");
		return RS_ERROR;
	}

	memcpy((void *)block, user->block, sizeof(user->block));

	return RS_OK;
}

CLIB_API RS cl_dev_flash_erase(cl_handle_t handle, u_int32_t num)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;		


	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_UPGRADE_FLASH_ERASE, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->num = num;
	pkt->param_len = sizeof(cln_user_t);
	log_debug("cl_dev_flash_erase num=%u\n", num);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);
	

	return ret;
}

CLIB_API RS cl_dev_active(cl_handle_t handle, u_int8_t *pdata, int len)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_DEV_ACTIVE, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len = len;
	pkt->param_len = sizeof(cln_user_t) + len;

	memcpy(u->data, pdata, len);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;
}

CLIB_API RS cl_get_timezone(cl_handle_t dev_handle,u_int32_t *timezone)
{
	RS ret = RS_OK;
	user_t *user;
	ucc_session_t *s = NULL;
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}	

	s = user->uc_session;

	*timezone = s->timezone;
	
done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}

CLIB_API RS cl_get_debug_info(cl_handle_t dev_handle,cl_dev_debug_info_t *debug_info)
{
	RS ret = RS_OK;
	user_t *user;
	ucc_session_t *s = NULL;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}	
	if (!user->smart_appliance_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}
	
	sac = user->smart_appliance_ctrl;
	if (!sac->sub_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}
	
	air_ctrl = sac->sub_ctrl;

	memcpy((void *)&debug_info->ver, (void *)&air_ctrl->stat.soft_version, sizeof(cl_version_t));
	debug_info->svn = air_ctrl->stat.udp_dev_stat.devSvnVersion;
	memcpy((void *)&debug_info->hardver, (void *)&air_ctrl->stat.hardware_version, sizeof(cl_version_t));
	
done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

CLIB_API RS cl_misc_tmp_24hour_line_import(cl_handle_t dev_handle, char *file)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
	int datalen = 0;
    
    CL_CHECK_INIT;

	if (!file) {
		return RS_ERROR;
	}

	datalen = (int)strlen(file);
	len = sizeof(cln_misc_info_t) + datalen +1;
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_TMP_24HOUR_LINE_IMPORT, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = datalen;
	memcpy(cln_mi->data, (void *)file, datalen);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}


CLIB_API RS cl_misc_humi_24hour_line_import(cl_handle_t dev_handle, char *file)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
	int datalen = 0;
    
    CL_CHECK_INIT;

	if (!file) {
		return RS_ERROR;
	}

	datalen = (int)strlen(file);
	len = sizeof(cln_misc_info_t) + datalen +1;
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_HUMI_24HOUR_LINE_IMPORT, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = datalen;
	memcpy(cln_mi->data, (void *)file, datalen);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_dev_set_irid(cl_handle_t handle, int id)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_COMM_IRID_SET, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len = 0;
	u->ip = id;
	pkt->param_len = sizeof(cln_user_t);

	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;
}

CLIB_API RS cl_dev_nd_debug_set(bool debug_on, u_int32_t size)
{
	cl_priv->nd_debug = debug_on;
	cl_priv->nd_debug_size = size;

	return RS_OK;
}

CLIB_API RS cl_dev_nd_wan_switch(char *desc)
{
	if (!desc) {
		return RS_ERROR;
	}

	nd_login_debug(NULL, "cl_dev_nd_wan_switch:desc=%s\n", desc);

	return RS_OK;
}

CLIB_API RS cl_dev_timezone_set(cl_handle_t handle, int value)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_COMM_TIMEZONE_SET, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len = 0;
	u->ip = (u_int32_t)value;
	pkt->param_len = sizeof(cln_user_t);

	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;	
}

CLIB_API RS cl_dev_scc_onoff_set(cl_handle_t handle, int value)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_COMM_SCC_ONOFF_SET, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len = 0;
	u->ip = (u_int32_t)value;
	pkt->param_len = sizeof(cln_user_t);

	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;	
}

CLIB_API RS cl_dev_stm_erase(cl_handle_t handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_COMM_STM_ERASE, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len = 0;
	pkt->param_len = sizeof(cln_user_t);

	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;	
}

CLIB_API RS cl_dev_debug_info_set(cl_handle_t handle, cl_air_debug_info_set_t *pinfo)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_COMM_DEBUG_CONF, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len = DBG_TYPE_LIGHT_AD;
	pkt->param_len = sizeof(cln_user_t) + sizeof(*pinfo);
	memcpy(u->data, (void *)pinfo, sizeof(*pinfo));

	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;	
}

CLIB_API RS cl_dev_day_ele_import(cl_handle_t handle, char *filename)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;	

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_COMM_DAYS_ELE_IMPORT, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = handle;
	u->len_name = (u_int16_t)strlen(filename) + 1;
	pkt->param_len = sizeof(cln_user_t) + u->len_name;
	strcpy((char *)u->data, filename);

	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);


	return ret;	
}

CLIB_API int cl_user_name_len_limit_get(cl_handle_t handle, int type)
{
	int len = -1;
	user_t *user = NULL;
	slave_t *slave = NULL;

	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, handle)) != NULL) {
		if (user->is_udp_ctrl) {
			len = MAX_HOSTNAME/2;
		} else {
			len = MAX_NICKNAME;
		}
		goto done;
	}
	if ((slave = lookup_by_handle(HDLT_SLAVE, handle)) != NULL) {
		len = MAX_NICKNAME;
	}

done:
	cl_unlock(&cl_priv->mutex);

	return len;
}

