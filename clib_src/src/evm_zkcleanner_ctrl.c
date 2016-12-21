#include "evm_zkcleanner_ctrl.h"
#include "cl_zkcleanner.h"

#define UCAT_EVM_ZKCLEANNER_DAY_DATA 1
#define UCAT_EVM_ZKCLEANNER_MONTH_DATA 2

typedef struct {
	u_int16_t obj;
	u_int16_t sub_obj;
	u_int16_t attr;
} query_obj_t;

static query_obj_t query_objs[] = {
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_ZKCLEANNER_DAY_DATA},
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_ZKCLEANNER_MONTH_DATA},
};


static bool _evm_zkcleanner_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	cl_zkcleanner_info_t *pv = (cl_zkcleanner_info_t*)ac->com_udp_dev_info.device_info;
	zkcleanner_uart_hdr_t *hd = (zkcleanner_uart_hdr_t*)pcmd;
	zkcleanner_uart_query_reply_t *stat = (zkcleanner_uart_query_reply_t *)&hd[1];
	zkcleanner_uart_query_reply_v2_t *stat_v2 = (zkcleanner_uart_query_reply_v2_t *)&hd[1];

	log_debug("_evm_zkcleanner_do_update_cmd_state, cmd %u cmd len %u\n", hd->cmd, cmd_len);

	if (cmd_len < sizeof(*hd)) {
		return false;
	}

	if (hd->cmd != ZKCLEANNER_UART_CMD_REPLY) {
		log_err(false, "invalid cmd\n");
		return false;
	}

	if (cmd_len < sizeof(*hd) + sizeof(*stat)) {
		return false;
	}

	pv->stat.onoff = stat->onoff;
	pv->stat.mode = stat->mode;
	pv->stat.wind = stat->wind;
	pv->stat.antibiosis = stat->antibiosis;
	pv->stat.fresh = stat->fresh;
	pv->stat.maintain = stat->maintain;

	pv->stat.ontime = stat->ontime_h << 8 | stat->ontime_l;
	pv->stat.offtime = stat->offtime_h << 8 | stat->offtime_l;
	pv->stat.type = stat->type;
	pv->stat.temp = stat->temp;
	pv->stat.pm25 = stat->pm25_h << 8 | stat->pm25_l;
	pv->stat.co2 = stat->co2_h << 8 | stat->co2_l;
	pv->stat.hcho = stat->hcho_h << 8 | stat->hcho_l;
	pv->stat.voc = stat->voc_h << 8 | stat->voc_l;
	pv->stat.aqi = stat->aqi;
	pv->stat.uptime = stat->uptime_h << 8 | stat->uptime_l;

	if (cmd_len >= sizeof(*hd) + sizeof(*stat_v2)) {
		pv->stat.ver = stat_v2->flag & 0xf;
		pv->stat.on_timer_changed = (stat_v2->flag >> 4) & 0x1;
		pv->stat.off_timer_changed = (stat_v2->flag >> 5) & 0x1;
	}	

	pv->stat.valid = true;

	return true;
}



static bool _evm_zkcleanner_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;
	bool modify = true;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_evm_zkcleanner_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
			case TLV_TYPE_SCM_COMMAND:
			if (_evm_zkcleanner_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len) == true) {
				modify = true;
			}
			break;
		default:
			break;
		}

		tlv = tlv_next(tlv);
		if (remain >= sizeof(uc_tlv_t)) {
			tlv->type = ntohs(tlv->type);
			tlv->len = ntohs(tlv->len);
		}
	}
	
	return modify;
}

bool zkcleanner_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_zkcleanner_info_t* priv;
	cl_zkcleanner_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_zkcleanner_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;
	
	return true;
}

static bool evm_zkcleanner_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_zkcleanner_info_t *pv;
	ucc_session_t *s;
	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("zkcleanner first init priv data\n");

	// 把非透传的信息查询一次，后面靠推送
	for (i = 0; i < ARRAY_SIZE(query_objs); i++) {
		sa_query_obj(s, query_objs[i].obj, query_objs[i].sub_obj, query_objs[i].attr);
	}
	
	return true;
}

bool zkcleanner_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("zkcleanner_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_zkcleanner_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_zkcleanner_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////
static u_int8_t zkcleanner_uart_calc_checksum(u_int8_t *buf, int len)
{
	int i;
	u_int8_t checksum = 0;
	
	for (i = 0; i < len; i++) {
		checksum += buf[i];	
	}

	return ~checksum;
}

static void zkcleanner_ctrl_stat(void *uc_session, cl_zkcleanner_stat_t *stat, u_int16_t ontime, u_int16_t offtime)
{
	u_int8_t buf[512] = {0};
	zkcleanner_uart_hdr_t *hd = (zkcleanner_uart_hdr_t *)buf;
	zkcleanner_uart_ctrl_t *ctrl = (zkcleanner_uart_ctrl_t *)&hd[1];	
	int len = sizeof(*hd) + sizeof(*ctrl);

	hd->syn = 0xa5;
	hd->checksum = 0;
	hd->len = len;
	hd->cmd = ZKCLEANNER_UART_CMD_SET;

	ctrl->onoff = stat->onoff;
	ctrl->mode = stat->mode;
	ctrl->wind = stat->wind;
	ctrl->antibiosis = stat->antibiosis;
	ctrl->fresh = stat->fresh;
	ctrl->maintain = stat->maintain;

	ctrl->ontime_h = (ontime >> 8) & 0xff;
	ctrl->ontime_l = (ontime) & 0xff;

	ctrl->offtime_h = (offtime >> 8) & 0xff;
	ctrl->offtime_l = (offtime) & 0xff;

	hd->checksum = zkcleanner_uart_calc_checksum(buf, len);

	scm_send_single_set_pkt(uc_session, buf, len);
}

#if 0
static void zkcleanner_ctrl_onofftime(void *uc_session, u_int16_t time, bool is_on)
{
	u_int8_t buf[512] = {0};
	zkcleanner_uart_hdr_t *hd = (zkcleanner_uart_hdr_t *)buf;
	zkcleanner_uart_set_time_t *ctrl = (zkcleanner_uart_set_time_t *)&hd[1];	
	int len = sizeof(*hd) + sizeof(*ctrl);

	hd->syn = 0xa5;
	hd->checksum = 0;
	hd->len = len;
	hd->cmd = ZKCLEANNER_UART_CMD_SET;


	if (is_on) {
		ctrl->ontime_h = (time >> 8) & 0xff;
		ctrl->ontime_l = (time) & 0xff;
	} else {
		ctrl->offtime_h = (time >> 8) & 0xff;
		ctrl->offtime_l = (time) & 0xff;
	}

	hd->checksum = zkcleanner_uart_calc_checksum(buf, len);

	scm_send_single_set_pkt(uc_session, buf, len);
}
#endif

bool zkcleanner_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_zkcleanner_info_t *pv;
	ucc_session_t *session;
	u_int32_t u32value = 0, *u32ptr = NULL;
	char buf[256];
	ucp_obj_t* uo = (ucp_obj_t*)buf;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_zkcleanner_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	u32value = cci_u32_data(info);

	switch (info->action) {
		case ACT_ZKCLEANNER_ONOFF:
			pv->stat.onoff = (u_int8_t)u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_MODE:
			pv->stat.mode = (u_int8_t)u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_WIND:
			pv->stat.wind = (u_int8_t)u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_ANTIBIOSIS:
			pv->stat.antibiosis = (u_int8_t)u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_FRESH:
			pv->stat.fresh = (u_int8_t)u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_MAINTAIN:
			pv->stat.maintain = (u_int8_t)u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_ONTIME:
			pv->stat.ontime = (u_int16_t)(u32value & 0xffff);
			pv->stat.offtime = (u_int16_t)((u32value >> 16) & 0xffff);
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_OFFTIME:
			pv->stat.offtime = u32value;
			zkcleanner_ctrl_stat(session, &pv->stat, pv->stat.ontime, pv->stat.offtime);
			break;
		case ACT_ZKCLEANNER_QUERY_ONE_DAY_DATA:
			u32ptr = (u_int32_t*)&uo[1];
			// 填0表示取当天数据
			if (u32value != 0) {
				u32value = u32value - cl_priv->timezone * 3600;
			} else {
				u32value = 0;
			}
			*u32ptr = ntohl(u32value);
			fill_net_ucp_obj(uo, UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_ZKCLEANNER_DAY_DATA, sizeof(*u32ptr));
			*ret = sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 1, buf, sizeof(*uo) + sizeof(*u32ptr));
			break;
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	
	return res;
}

static void item_order(cl_zkcleanner_date_t *item, int n)
{
	int i;
	u_int8_t *ptr;

	for (i = 0; i < n; i++) {
		ptr = (u_int8_t*)&item[i].pm25;
		item[i].pm25 = ptr[1] << 8 | ptr[0];

		ptr = (u_int8_t*)&item[i].co2;
		item[i].co2 = ptr[1] << 8 | ptr[0];
	}
}

static void zkcleanner_do_update_day_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_zkcleanner_info_t *pv;
	cl_zkcleanner_date_t *item;
	u_int32_t *u32ptr;
	int num = 24;
	
	pv = (cl_zkcleanner_info_t*)ac->com_udp_dev_info.device_info;
	u32ptr = (u_int32_t *)&obj[1];

	if (is_obj_less_than_len(obj, sizeof(*item) * num + sizeof(*u32ptr))) {
		log_err(false, "zkcleanner_do_update_day_data len %u invalid\n", obj->param_len);
		return;
	}

	item = (cl_zkcleanner_date_t *)&u32ptr[1];

	item_order(item, num);

	memcpy(pv->day_data.items, item, sizeof(*item) * num);
	pv->day_data.time = ntohl(*u32ptr);

	event_push(ac->sac->user->callback, SAE_ZKCLEANNER_DAY_DATA, ac->sac->user->handle, ac->sac->user->callback_handle);
}

static void zkcleanner_do_update_month_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_zkcleanner_info_t *pv;
	cl_zkcleanner_date_t *item;
	int num = 31;
	
	pv = (cl_zkcleanner_info_t*)ac->com_udp_dev_info.device_info;
	item = (cl_zkcleanner_date_t *)&obj[1];

	if (is_obj_less_than_len(obj, sizeof(*item) * num)) {
		log_err(false, "len %u invalid\n", obj->param_len);
		return;
	}

	item_order(item, num);

	memcpy(pv->month_data.items, item, sizeof(*item) * num);
	pv->month_data.time = (u_int32_t)time(NULL);
	
	event_push(ac->sac->user->callback, SAE_ZKCLEANNER_MONTH_DATA, ac->sac->user->handle, ac->sac->user->callback_handle);
}


bool zkcleanner_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{

	log_info("zkcleanner_do_update_device_data attri %u\n", obj->attr);

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_zkcleanner_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_EVM_ZKCLEANNER_DAY_DATA:
			zkcleanner_do_update_day_data(air_ctrl, action, obj);
			break;
		case UCAT_EVM_ZKCLEANNER_MONTH_DATA:
			zkcleanner_do_update_month_data(air_ctrl, action, obj);
			break;
		default:
			return false;
	}
	
	return true;
}

////////////////////////////////////////////////////////////////////

/*
	功能: 对净化器设置报文
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_ZKCLEANNER_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zkcleanner_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, value);
}

