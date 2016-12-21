/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: 用户上下线管理
**  File:    lanusers_priv.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    04/18/2016
**
**  Purpose:
**    Xxx.
**************************************************************************/


/* Include files. */
#include "lanusers_priv.h"

#if 0
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

/* Macro constant definitions. */
#define LUSER_RECORD_ONCE_COUNT	5

/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */


static void lanusers_dump_items(const char *msg, lanusers_manage_user_record_item_t *record, int record_num)
{
	int i;

	log_debug("\n%s\n", msg);

	for (i = 0; i < record_num; i++) {
		log_debug("type %u user id %u, time %u\n", record[i].event_type, record[i].user_id, record[i].timestamp);
	}
	log_debug("============\n");
}

static void lanusers_save_config(u_int64_t sn, cl_lanusers_manage_info_t *info)
{
	lanusers_record_hd_t hd;
	char path[256];
	FILE *fp = NULL;

	hd.magic = 0x11234;
	hd.min_idx = info->min_index;
	hd.record_num = info->record_num;

	sprintf(path, "%s/%"PRIu64"/lanusers.conf", cl_priv->dir, sn);

	if (info->record == NULL) {
		return;
	}

	if ((fp = fopen(path, "w+b")) == NULL) {
		goto done;
	}

	fwrite((u_int8_t*)&hd, 1, sizeof(hd), fp);
	fwrite((u_int8_t*)info->record, 1, sizeof(lanusers_manage_user_record_item_t) * info->record_num, fp);

	log_debug("lanusers_save_config success\n");
done:
	if (fp) {
		fclose(fp);
	}
}

static void lanusers_load_config(u_int64_t sn, cl_lanusers_manage_info_t *info)
{
	lanusers_record_hd_t hd;
	char path[256];
	FILE *fp = NULL;
	int n, want;

	sprintf(path, "%s/%"PRIu64"/lanusers.conf", cl_priv->dir, sn);

	if ((fp = fopen(path, "rb")) == NULL) {
		log_debug("no lanusers config\n");
		goto done;
	}

	if ((n = (int)fread((u_int8_t*)&hd, 1, sizeof(hd), fp)) <= 0) {
		log_debug("fread [%s] failed\n", path);
		goto done;
	}

	if (hd.magic != 0x11234) {
		log_err(false, "invalid magic 0x%x\n", hd.magic);
		goto done;
	}

	info->record = cl_calloc(1, sizeof(lanusers_manage_user_record_item_t) * hd.record_num);
	if (!info->record) {
		goto done;
	}

	want = sizeof(lanusers_manage_user_record_item_t) * hd.record_num;
	n = (int)fread((u_int8_t*)info->record, 1, want, fp);
	if (n < want) {
		log_err(false, "lanusers config get invalid read ret %d want %u\n", n, want);
		goto done;
	}

	info->min_index = hd.min_idx;
	info->record_num = hd.record_num;

	log_debug("load record: min_idx %u num %u\n", info->min_index, info->record_num);
	lanusers_dump_items("loaded:", info->record, info->record_num);
done:
	if (fp) {
		fclose(fp);
	}
}

bool lanusers_update_enable(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_lanusers_manage_info_t *info = &air_ctrl->com_udp_dev_info.lum_info;
	ucp_lanusers_manage_enable_t *reply = (ucp_lanusers_manage_enable_t *)&obj[1];

	if (obj->param_len < sizeof(*reply)) {
		log_err(false, "invalid param len %u\n", obj->param_len);
		return false;
	}

	info->enable = reply->enable;

	log_debug("update enable %u\n", info->enable);

	return true;
}

bool lanusers_update_status(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_lanusers_manage_info_t *info = &air_ctrl->com_udp_dev_info.lum_info;
	ucp_lanusers_manage_user_status_t *reply = (ucp_lanusers_manage_user_status_t *)&obj[1];

	if (obj->param_len < sizeof(*reply)) {
		log_err(false, "invalid param len %u\n", obj->param_len);
		return false;
	}

	reply->user_id = ntohl(reply->user_id);
	reply->timestamp = ntohl(reply->timestamp);
	
	memcpy((u_int8_t*)&info->last_record, (u_int8_t*)reply, sizeof(*reply));

	log_debug("update lan users status user id %u time %u event type %u\n", 
		reply->user_id, reply->timestamp, reply->event_type);

	return true;
}

static void lanusers_request_record(ucc_session_t *session, u_int8_t count, u_int32_t start_index)
{
	u_int8_t buf[256] ={0};
	ucp_obj_t *uo = (ucp_obj_t *)buf;
	ucp_lanusers_manage_user_records_request_t *request = (ucp_lanusers_manage_user_records_request_t *)&uo[1];

	log_debug("  => reqeust record: count %u start_index %u\n", count, start_index);

	request->count = count;
	request->start_index = ntohl(start_index);
	
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_LANUSERS_MANAGE, UCAT_LANUSERS_MANAGE_USER_RECORD, sizeof(*request));
	sa_ctrl_obj_value(session, UCA_GET, false, 0x1, uo, sizeof(*uo) + sizeof(*request));
}

static void lanusers_record_cache_init(user_record_cache_t *rc)
{
	if (rc->items) {
		cl_free(rc->items);
	}

	memset(rc, 0x00, sizeof(*rc));
}

static void lanusers_start_get_record(ucc_session_t *session, cl_lanusers_manage_info_t *info, u_int32_t start_index, u_int32_t end_index)
{
	user_record_cache_t *rc = &info->rc;
	u_int8_t want = 0, count;


	lanusers_record_cache_init(rc);

	count = end_index - start_index + 1;

	rc->start_index = rc->request_start_index = start_index;
	rc->end_index = end_index;

	rc->total = count;

	log_debug(" start get record: start_index %u end_index %u total %u\n", rc->start_index, rc->end_index, rc->total);

	rc->items = cl_calloc(1, sizeof(lanusers_manage_user_record_item_t) * rc->total);
	if (rc->items == NULL) {
		log_err(false, "calloc rc item failed\n");
		rc->total = 0;
		return;
	}

	want = min(count, LUSER_RECORD_ONCE_COUNT);

	lanusers_request_record(session, want, rc->start_index);
}

static void lanusers_request_enable(ucc_session_t *session, u_int8_t enable)
{
	u_int8_t buf[256] ={0};
	ucp_obj_t *uo = (ucp_obj_t *)buf;
	ucp_lanusers_manage_enable_t *request = (ucp_lanusers_manage_enable_t *)&uo[1];

	log_debug("  lanusers_request_enable %u\n", enable);

	request->enable = enable;
	
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_LANUSERS_MANAGE, UCAT_LANUSERS_MANAGE_ENABLE, sizeof(*request));
	sa_ctrl_obj_value(session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(*request));
}

static void lanusers_set_model(ucc_session_t *session, u_int32_t user_id, u_int8_t *model)
{
	u_int8_t buf[256] ={0};
	ucp_obj_t *uo = (ucp_obj_t *)buf;
	ucp_lanusers_manage_user_info_set_t *request = (ucp_lanusers_manage_user_info_set_t *)&uo[1];

	log_debug("  lanusers_set_model user_id %u model[%s]\n", user_id, model);

	if (strlen((char *)model) >= 127) {
		return;
	}

	request->user_id = ntohl(user_id);
	memcpy(request->model, model, strlen((char *)model));
	
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_LANUSERS_MANAGE, UCAT_LANUSERS_MANAGE_USER_INFO, sizeof(*request));
	sa_ctrl_obj_value(session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(*request));
}


static void lanusers_adjust_record(cl_lanusers_manage_info_t *info, u_int32_t current_index, u_int16_t count)
{	
	lanusers_manage_user_record_item_t *new_item = NULL;
	u_int32_t offset, new_start_index, old_end_index;

	if (info->min_index == 0) {
		log_debug("record is null, no need adjust\n");
		return;
	}

	old_end_index = info->min_index + info->record_num - 1;
	new_start_index = current_index - count + 1;

	// 有效数据得后移
	if (new_start_index > info->min_index) {
		log_debug("need shift min_index local: start_idx %u end_idx %u, new : start_index %u end_idx %u\n", 
		info->min_index, old_end_index, new_start_index, current_index);
			
		if (new_start_index > old_end_index) {
			log_debug("new_start_index > old_end_index all record clear\n");
			info->min_index = info->record_num = 0;
		} else {
			offset = new_start_index - info->min_index;

			info->min_index = new_start_index;
			info->record_num = old_end_index - new_start_index + 1;

			new_item = cl_calloc(1, info->record_num * sizeof(*new_item));
			if (!new_item) {
				log_err(false, "calloc new_item failed\n");
				return;
			}

			memcpy(new_item, (u_int8_t*)&info->record[offset], info->record_num * sizeof(lanusers_manage_user_record_item_t));
			cl_free(info->record);
			info->record = new_item;

			log_debug("min index right shift to %u record num %u\n", info->min_index, info->record_num);
			lanusers_dump_items("after shift:", info->record, info->record_num);
		}
	}

	// 设备数据出现异常，全部重新获取
	if (new_start_index < info->min_index) {
		log_debug("new_start_index < old_end_index all record clear\n");
			info->min_index = info->record_num = 0;
	}

	if (current_index < old_end_index) {
		log_debug("current_index < old_end_index all record clear\n");
			info->min_index = info->record_num = 0;
	}
}

bool lanusers_update_record_idx(smart_air_ctrl_t* air_ctrl, u_int8_t action, ucp_obj_t* obj)
{
	cl_lanusers_manage_info_t *info = &air_ctrl->com_udp_dev_info.lum_info;
	ucp_lanusers_manage_user_records_num_t *reply = (ucp_lanusers_manage_user_records_num_t *)&obj[1];
	u_int32_t local_end_index = 0, next_start_index;

	if (obj->param_len < sizeof(*reply)) {
		log_err(false, "invalid param len %u\n", obj->param_len);
		return false;
	}

	reply->count = ntohs(reply->count);
	reply->current_index = ntohl(reply->current_index);

	if (info->load_config == false) {
		lanusers_load_config(air_ctrl->sac->user->sn, info);
		info->load_config = true;
	}

	if (reply->count == 0) {
		log_debug("all items is empty\n");
		info->min_index = info->record_num = 0;
		return false;
	}
	
	// 判断有效数据得后移
	lanusers_adjust_record(info, reply->current_index, reply->count);

	if (info->min_index != 0) {
		local_end_index = info->min_index + info->record_num - 1;
	}

	log_debug("update lan user record num: count %u current_Index %u\n", reply->count, reply->current_index);
	log_debug("local min_index %u, record_num %u end_index %u\n", info->min_index, info->record_num, local_end_index);

	if (reply->current_index <= local_end_index) {
		log_err(false, "reply->current_index %u <= local_end_index %u, ignore\n", reply->current_index, local_end_index);
		return false;
	}

	// 有数据变化	
	if (info->min_index != 0) {
		next_start_index = local_end_index + 1;
	} else {
		next_start_index = reply->current_index - reply->count + 1;
	}
	
	lanusers_start_get_record(air_ctrl->sac->user->uc_session, info, next_start_index, reply->current_index);

	return false;
}


static void lanusers_items_order(ucp_lanusers_manage_user_records_reply_t *reply)
{
	int i;

	reply->start_index = ntohl(reply->start_index);

	for (i = 0; i < reply->count; i++) {
		reply->items[i].user_id = ntohl(reply->items[i].user_id);
		reply->items[i].timestamp = ntohl(reply->items[i].timestamp);
	}
}

bool lanusers_update_record_item(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_lanusers_manage_info_t *info = &air_ctrl->com_udp_dev_info.lum_info;
	user_record_cache_t *rc = &info->rc;
	ucp_lanusers_manage_user_records_reply_t *reply = (ucp_lanusers_manage_user_records_reply_t *)&obj[1];
	int n, idx;
	lanusers_manage_user_record_item_t *new_item = NULL;

	if (obj->param_len < sizeof(*reply)) {
		log_err(false, "invalid param len %u\n", obj->param_len);
		return false;
	}

	if (obj->param_len < sizeof(*reply) + sizeof(lanusers_manage_user_record_item_t) * reply->count) {
		log_err(false, "invalid param len %u\n", obj->param_len);
		return false;
	}

	lanusers_items_order(reply);

	log_debug(" <=== get lan user record item: start_index %u count %u\n", reply->start_index, reply->count);

	if (reply->count == 0) {
		log_err(false, "count == 0\n");
		return false;
	}

	lanusers_dump_items("append items:", reply->items, reply->count);

	if (rc->items == NULL) {
		log_err(false, "get user item, but cache item buffer is null, igore\n");
		return false;
	}

	if (reply->start_index != rc->request_start_index) {
		log_debug("invalid reply start_index %u request_start_index %u\n", reply->start_index, rc->request_start_index);
		return false;
	}

	idx = reply->start_index - rc->start_index;
	n = min(reply->count, rc->end_index - reply->start_index + 1);

	log_debug("  write idx %d start_index %u, n %u\n", idx, reply->start_index, n);

	memcpy((u_int8_t*)&(info->rc.items[idx]), reply->items, n * sizeof(lanusers_manage_user_record_item_t));

	
	if (reply->start_index + reply->count <= rc->end_index) {
		log_debug("	xx start_index %u + count %u <= end_index %u\n", reply->start_index, reply->count, rc->end_index);
		rc->request_start_index = reply->start_index + n;
		lanusers_request_record(air_ctrl->sac->user->uc_session, min(rc->end_index - rc->request_start_index + 1, LUSER_RECORD_ONCE_COUNT), rc->request_start_index);
		return false;
	}


	new_item = cl_calloc(1, sizeof(*new_item) * (info->record_num + rc->total));
	if (!new_item) {
		log_err(false, "calloc record failed\n");
		return false;
	}

	// 拷贝之前的
	if (info->record) {
		memcpy(new_item, info->record, sizeof(*new_item) * info->record_num);
	}
	// 追加的
	memcpy((u_int8_t*)&new_item[info->record_num], (u_int8_t*)rc->items, sizeof(*new_item) * rc->total);
	if (info->record) {
		cl_free(info->record);
	}
	info->record = new_item;
	info->record_num += rc->total;
	if (info->min_index == 0) {
		info->min_index = rc->start_index;
	}
	
	log_debug("good, all item received, now min_idx %u record_num %u\n", info->min_index, info->record_num);
	
	lanusers_record_cache_init(rc);

	lanusers_dump_items("newest record: " ,info->record, info->record_num);
	lanusers_save_config(air_ctrl->sac->user->sn, info);

	// 事件	
	event_push(air_ctrl->sac->user->callback, UE_INFO_MODIFY, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);

	return false;
}


bool lanusers_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	switch (obj->attr) {
		case UCAT_LANUSERS_MANAGE_ENABLE:
			return lanusers_update_enable(air_ctrl, action, obj);
		case UCAT_LANUSERS_MANAGE_USER_INFO:
			break;
		case UCAT_LANUSERS_MANAGE_STATUS:
			return lanusers_update_status(air_ctrl, action, obj);
		case UCAT_LANUSERS_MANAGE_RECORD_NUM:
			return lanusers_update_record_idx(air_ctrl, action, obj);
		case UCAT_LANUSERS_MANAGE_USER_RECORD:
			return lanusers_update_record_item(air_ctrl, action, obj);
			
		default:
			return false;
	}
	return true;
}

bool lanusers_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "zhcl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "zhcl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	switch(info->action){
	case ACT_LUM_SET_ENABLE:
		{
			u_int8_t value = (u_int8_t)cci_u32_data(info);
			lanusers_request_enable(ac->sac->user->uc_session, value);
		}
		break;

	case ACT_LUM_SET_MODEL:
		{
			ucp_lanusers_manage_user_info_set_t *request = cci_pointer_data(info);
			lanusers_set_model(ac->sac->user->uc_session, request->user_id, request->model);
		}
		break;
	
	default:
		break;
	}
	return res;
}


CLIB_API RS cl_lanusers_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;

	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_COMMON_UDP_LUM, action, value);	
}

CLIB_API RS cl_lanusers_set_user_info(cl_handle_t slave_handle, ucp_lanusers_manage_user_info_set_t *request)
{
	CL_CHECK_INIT;

	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_COMMON_UDP_LUM, ACT_LUM_SET_MODEL, (u_int8_t*)request, sizeof(*request)); 
}

