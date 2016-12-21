/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: evm_bpuair_ctrl
**  File:    evm_bpuair_ctrl.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/05/2016
**
**  Purpose:
**    Evm_bpuair_ctrl.c.
**************************************************************************/


/* Include files. */
#include "evm_bpuair_ctrl.h"
#include "cl_bpuair.h"

/* Macro constant definitions. */
#define UCAT_EVM_BPUAIR_VERSION 1
#define UCAT_EVM_BPUAIR_FAULT_HISTORY 2
#define UCAT_EVM_BPUAIR_FAULT_CURRENT 3


/* Type definitions. */

typedef struct {
	u_int16_t obj;
	u_int16_t sub_obj;
	u_int16_t attr;
} query_obj_t;

static query_obj_t query_objs[] = {
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_BPUAIR_VERSION},
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_BPUAIR_FAULT_HISTORY},
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_BPUAIR_FAULT_CURRENT},
};


/* Local function declarations. */


/* Macro API definitions. */
#if 0
#ifdef log_debug
#undef log_debug
#define	log_debug(...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef log_err
#undef log_err
#define	log_err(print_err, ...) _log_err(__FILE__, __LINE__, print_err, __VA_ARGS__)
#endif

#ifdef log_info
#undef log_info
#define	log_info(...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
#endif
#endif


/* Global variable declarations. */
#if 1
static bool bpuair_find_nearest_timer(cl_bpuair_timer_t *pt, u_int8_t ptn, once_timer_t *ot, nearest_timer_t *nt)
{
	struct tm *date;
	time_t ntime;
	u_int8_t wday, twday;
	int i, j, tmp, tdmins, mins_tmp, mins_near = -1;
	bool today_has_timer = false;

#if 1
	
	ntime = time(NULL);
	date = localtime(&ntime);
	wday = date->tm_wday;
	
	tdmins = date->tm_hour*ONE_HOUR_MINS + date->tm_min;
	
	//一次定时器有效
	if(ot->once_timer_enable){
		
		mins_tmp = ot->once_timer_hour*ONE_HOUR_MINS + ot->once_timer_min - tdmins;
		if(mins_tmp >= 0){
			mins_near = mins_tmp;
		}
		else{
			mins_near = ONE_DAY_MINS + mins_tmp;
		}
		
		nt->onoff = ot->once_timer_onoff;
		
	}
	
	//遍历星期定时器
	for(i=0; i<ptn; i++){
		if (!pt[i].valid) {
			continue;
		}
		
		today_has_timer = false;
		tmp = pt[i].week&0x7f;
		//定时器无效
		if((!tmp) ){
			pt[i].valid = 0;
			continue;
		}
			
		twday = wday;
		//当前星期定时器，找出该星期定时中 的定时器 离当前最近的一天
		for(j=0; j<=6; j++){
			
			//找到了最近一天的定时器。
			if(tmp&(1<<twday)){
				
				//当天存在定时器且时间已过
				if((twday == wday) && (pt[i].hour < date->tm_hour || (pt[i].hour == date->tm_hour && pt[i].mins < date->tm_min))){
					today_has_timer = true;
					//continue;
				} else {
					break;
				}
			}


			if (twday == 6) {
				twday = 0;
			} else {
				twday++;
			}
		}
		
		//计算出该星期定时中  最近的定时器 离当前的天数。
		tmp = twday - wday;
		if((tmp < 0) || (tmp == 0 && today_has_timer)){
			tmp += 7;
		}
		
		//计算出该星期定时中  最近的定时器 离当前的分钟数。
		mins_tmp = tmp*ONE_DAY_MINS + pt[i].hour*ONE_HOUR_MINS + pt[i].mins - tdmins;
		
		//更新 所有定时中 最近的定时器记录
		if((mins_near == -1) || (mins_tmp < mins_near )){
			mins_near = mins_tmp;
			nt->onoff = pt[i].onoff;
		}
		
	}
	
	//时间戳填入
	if(mins_near>=0){
		nt->time  = (u_int32_t)(ntime + mins_near*ONE_MIN_SECS);		
		return true;
	}
#endif
	return false;
}

#endif
static void bpuair_update_period_timer(cl_bpuair_info_t *pv, u_int8_t id, u_int8_t week, u_int8_t onoff, u_int8_t hour, u_int8_t min)
{
	cl_bpuair_timer_t *t;
	
	if (id > ARRAY_SIZE(pv->timers) || id == 0) {
		return;
	}

	t = &pv->timers[id - 1];

	if (week == 0) {
		t->valid = false;
	} else {
		t->valid = true;
	}

	t->week = week;
	t->onoff = onoff;
	t->hour = hour;
	t->mins = min;

	log_debug("bpuair update timer #%d: week 0x%x onoff %u hour %u mins %u\n", 
		id, t->week, t->onoff, t->hour, t->mins);
}

void bpuair_calc_next_timer(cl_bpuair_info_t *pv)
{	
	once_timer_t ot;
	nearest_timer_t nt;

	// 计算最近一次定时器
	ot.once_timer_enable = pv->stat.once_timer_enable;
	ot.once_timer_onoff = pv->stat.once_timer_onoff;
	ot.once_timer_hour = pv->stat.once_timer_hour;
	ot.once_timer_min = pv->stat.once_timer_min;
	
	if (bpuair_find_nearest_timer(pv->timers, ARRAY_SIZE(pv->timers), &ot, &nt) == true) {
		log_debug("get next timer %u onoff %u\n", nt.time, nt.onoff);
		pv->stat.next_timer_stamp = nt.time;
		pv->stat.next_timer_onoff = nt.onoff;
	} else {
		log_debug("not found near timer, set it zero\n");
		pv->stat.next_timer_stamp = 0;
	}
}

static bool _evm_bpuair_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	int i;
//	u_int8_t at_cmd[512] = {0};
	cl_bpuair_info_t *pv = (cl_bpuair_info_t*)ac->com_udp_dev_info.device_info;
	bpuair_tt_stat_t *stat = (bpuair_tt_stat_t*)pcmd;
//	u_int32_t now = (u_int32_t)time(NULL);

	log_debug("_evm_bpuair_do_update_cmd_state len %u\n", cmd_len);

	if (cmd_len < sizeof(*stat)) {
		return false;
	}

#if 0
	// 控制以后同一秒内收到的数据，忽略掉
	if (pv->last_ctrl && now <= pv->last_ctrl + 1) {
		return false;
	}
#endif

	pv->stat.type = stat->type;
	pv->stat.stat1 = stat->stat1;
	pv->stat.stat2 = stat->stat2;
	pv->stat.work_mode = stat->work_mode;
	pv->stat.eco_mode = stat->eco_mode;
	pv->stat.cold_temp = ntohs(stat->cold_temp);
	pv->stat.hot_temp = ntohs(stat->hot_temp);
	pv->stat.backwater_temp = ntohs(stat->backwater_temp);
	pv->stat.water_temp = ntohs(stat->water_temp);
	pv->stat.env_temp = ntohs(stat->env_temp);
	pv->stat.coiler1_temp = ntohs(stat->coiler1_temp);
	pv->stat.coiler2_temp = ntohs(stat->coiler2_temp);
	pv->stat.current1 = ntohs(stat->current1);
	pv->stat.current2 = ntohs(stat->current2);

	
	pv->stat.coiler2_temp = ntohs(stat->coiler2_temp);

	pv->stat.eco_cold_temp = ntohs(stat->eco_cold_temp);
	pv->stat.eco_hot_temp = ntohs(stat->eco_hot_temp);

	pv->stat.uptime = ntohs(stat->uptime);

	log_debug("type %u stat1 %u state2 %u work_mode %u cold_temp %u hot_temp %u\n", 
		pv->stat.type, pv->stat.stat1, pv->stat.stat2, pv->stat.work_mode, pv->stat.cold_temp, pv->stat.hot_temp);

	// 一次性定时器
	pv->stat.once_timer_enable = !!(stat->timer_set & BIT(6));
	pv->stat.timer_enable = !!(stat->timer_set & BIT(0));
	pv->stat.once_timer_onoff = !!(stat->timer_set & BIT(7));
	pv->stat.once_timer_hour = stat->once_timer_hour;
	pv->stat.once_timer_min = stat->once_timer_min;

	for (i = 0; i < ARRAY_SIZE(stat->period_timer); i++) {
		bpuair_update_period_timer(pv, i + 1, stat->period_timer[i].set & 0x7f, !!(stat->period_timer[i].set & 0x80), stat->period_timer[i].hour, stat->period_timer[i].min);
	}

	memcpy(pv->stat.fault_array, stat->err, sizeof(pv->stat.fault_array));

	// 计算最近一次定时器
	bpuair_calc_next_timer(pv);

	log_debug("push event modify\n");

	return true;
}



static bool _evm_bpuair_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;
	bool modify = true;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_evm_bpuair_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
			case TLV_TYPE_SCM_COMMAND:
			if (_evm_bpuair_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len) == true) {
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

bool bpuair_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_bpuair_info_t* priv;
	cl_bpuair_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_bpuair_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;
	
	return true;
}

static bool evm_bpuair_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_bpuair_info_t *pv;
	ucc_session_t *s;
	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("bpuair first init priv data\n");

	// 把非透传的信息查询一次，后面靠推送
	for (i = 0; i < ARRAY_SIZE(query_objs); i++) {
		sa_query_obj(s, query_objs[i].obj, query_objs[i].sub_obj, query_objs[i].attr);
	}
	
	return true;
}

bool bpuair_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("bpuair_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_bpuair_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_bpuair_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////
/*
static u_int8_t bpuair_uart_calc_checksum(u_int8_t *buf, int len)
{
	int i;
	u_int8_t checksum = 0;
	
	for (i = 0; i < len; i++) {
		checksum += buf[i];	
	}

	return ~checksum;
}
*/

static void bpuair_ctrl_stat(void *uc_session, cl_bpuair_info_t *pv, u_int8_t onoff)
{
	u_int8_t buf[512] = {0};
	int i;
	bpuair_tt_ctrl_t *ctrl = (bpuair_tt_ctrl_t *)buf;
	int len = sizeof(*ctrl);

	// 开机
	if (onoff == 1) {
		ctrl->onoff |= 1 << 1;	
	} else if (onoff == 2) {
		ctrl->onoff |= 1;	
	}


	if (pv->stat.work_mode == 2) {
		ctrl->onoff |= 1 << 2;
	}

	if (pv->stat.eco_mode) {
		ctrl->onoff |= 1 << 3;
	}

	ctrl->cold_temp = ntohs(pv->stat.cold_temp);
	ctrl->hot_temp = ntohs(pv->stat.hot_temp);

	if (pv->stat.timer_enable) {
		ctrl->timer_set |= 1;
	}

	if (pv->stat.once_timer_onoff) {
		ctrl->timer_set |= 1 << 7;
	}

	if (pv->stat.once_timer_enable) {
		ctrl->timer_set |= 1 << 6;
	}

	ctrl->once_timer_hour = pv->stat.once_timer_hour;
	ctrl->once_timer_min = pv->stat.once_timer_min;


	for (i = 0; i < ARRAY_SIZE(ctrl->period_timer); i++) {
		if (pv->timers[i].valid == false) {
			continue;
		}

		ctrl->period_timer[i].set = pv->timers[i].week & 0x7f;
		if (pv->timers[i].onoff) {
			ctrl->period_timer[i].set |= 1 << 7;
		}
		ctrl->period_timer[i].hour = pv->timers[i].hour;
		ctrl->period_timer[i].min = pv->timers[i].mins;
	}


	scm_send_single_set_pkt(uc_session, buf, len);
}

bool bpuair_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_bpuair_info_t *pv;
	ucc_session_t *session;
	u_int32_t u32value = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_bpuair_info_t *)(ac->com_udp_dev_info.device_info);
	if (pv == NULL) {
		*ret = RS_NOT_INIT;
		return false;
	}

	session = ac->sac->user->uc_session;

	u32value = cci_u32_data(info);

	switch (info->action) {
		case ACT_BPUAIR_ONOFF:
			pv->stat.stat1 = u32value;
			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, u32value + 1);
			break;
		case ACT_BPUAIR_MODE:
			pv->stat.work_mode = u32value;
			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);
			break;
		case ACT_BPUAIR_ECO:	
			pv->stat.eco_mode = u32value;
#if 0
			if (pv->stat.eco_mode == 1) {
				if (pv->stat.work_mode == 1) {
					// 把上传的ECO模式的制冷制热温度赋给当前制冷制热
					pv->stat.cold_temp = pv->stat.eco_cold_temp;
				} else {
					pv->stat.hot_temp = pv->stat.eco_hot_temp;
				}
			}
#endif			
			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);
			break;
		case ACT_BPUAIR_COLD_TEMP:
			pv->stat.cold_temp = u32value;
			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);
			break;
		case ACT_BPUAIR_HOT_TEMP:
			pv->stat.hot_temp = u32value;
			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);
			break;
		case ACT_BPUAIR_TIMER_ENABLE:
			pv->stat.timer_enable = u32value;
			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);
			break;

		case ACT_BPUAIR_ONCE_TIMER:
			if ((u32value & 0xff) == 0) {
				pv->stat.once_timer_enable = false;
			} else {
				pv->stat.once_timer_enable = true;
			}
			
			pv->stat.once_timer_onoff = (u_int8_t)((u32value >> 8) & 0xff);
			pv->stat.once_timer_hour = (u_int8_t)((u32value >> 16) & 0xff);
			pv->stat.once_timer_min = (u_int8_t)((u32value >> 24) & 0xff);

			bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);

			// 计算最近一次定时器
			bpuair_calc_next_timer(pv);
			
			break;

		case ACT_BPUAIR_PERIOD_TIMER:
			{
				cl_bpuair_timer_set_t *timer = cci_pointer_data(info);

				if (timer->id == 0 || timer->id > ARRAY_SIZE(pv->timers)) {
					break;
				}
				
				pv->timers[timer->id - 1].valid = timer->valid;
				pv->timers[timer->id - 1].onoff = timer->onoff;
				pv->timers[timer->id - 1].week = timer->week;
				pv->timers[timer->id - 1].hour = timer->hour;
				pv->timers[timer->id - 1].mins = timer->mins;

				if (pv->timers[timer->id - 1].valid == false) {
					pv->timers[timer->id - 1].week = 0;
					pv->timers[timer->id - 1].hour = 0;
					pv->timers[timer->id - 1].mins = 0;	
				}

				bpuair_ctrl_stat(ac->sac->user->uc_session, pv, 0);

				// 计算最近一次定时器
				bpuair_calc_next_timer(pv);
			}
			break;
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}

	//user->last_ctrl = true;
	pv->last_ctrl = (u_int32_t)time(NULL);
	
	return res;
}

static int item_order(cl_bpuair_fault_t *item)
{
	u_int32_t i;

	item->num = ntohl(item->num);

	if (item->num > 10) {
		return -1;
	}

	for (i = 0; i < item->num; i++) {
		item->fault[i].timestamp = ntohl(item->fault[i].timestamp);
		item->fault[i].fault_number = ntohl(item->fault[i].fault_number);
	}

	return 0;
}

static void bpuair_do_update_fault_history(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_bpuair_info_t *pv;
	cl_bpuair_fault_t *history;
//	int num = 31;
	
	pv = (cl_bpuair_info_t*)ac->com_udp_dev_info.device_info;

	if (is_obj_less_than_len(obj, sizeof(u_int32_t) * 2)) {
		log_err(false, "len %u invalid\n", obj->param_len);
		return;
	}

	history = (cl_bpuair_fault_t *)&obj[1];

	if (item_order(history) < 0) {
		return;
	}

	memcpy((u_int8_t*)&pv->fault_history, history, sizeof(pv->fault_history));
}

static int current_item_order(cl_bpuair_current_fault_t *item)
{
	u_int32_t i;

	item->num = ntohl(item->num);

	if (item->num > 10) {
		return -1;
	}

	for (i = 0; i < item->num; i++) {
		item->fault[i].timestamp = ntohl(item->fault[i].timestamp);
		item->fault[i].fault_number = ntohl(item->fault[i].fault_number);
	}

	return 0;
}

static void bpuair_do_update_fault_current(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_bpuair_info_t *pv;
	cl_bpuair_current_fault_t *fault_current;
//	cl_bpuair_fault_item_t *item;
	
	pv = (cl_bpuair_info_t*)ac->com_udp_dev_info.device_info;

	if (is_obj_less_than_len(obj, sizeof(u_int32_t))) {
		log_err(false, "len %u invalid\n", obj->param_len);
		return;
	}

	fault_current = (cl_bpuair_current_fault_t *)&obj[1];

	if (current_item_order(fault_current) < 0) {
		return;
	}

	memcpy((u_int8_t*)&pv->fault_current, fault_current, sizeof(fault_current->num) + sizeof(fault_current->fault[0]) * fault_current->num);
}


/**
	
P0~P19	机组编码
P20~P51	控制板软件版本号
P52~P83	手操器软件版本号
*/
static void bpuair_do_update_ver(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_bpuair_info_t *pv;
	u_int8_t *str;
	int min_len = 84;
	
	pv = (cl_bpuair_info_t*)ac->com_udp_dev_info.device_info;

	if (is_obj_less_than_len(obj, sizeof(min_len))) {
		log_err(false, "len %u invalid\n", obj->param_len);
		return;
	}

	str = (u_int8_t*)&obj[1];

	memcpy((u_int8_t*)&pv->stat.unit_code, str, 20);
	memcpy((u_int8_t*)&pv->stat.soft_ver1, &str[20], 32);
	memcpy((u_int8_t*)&pv->stat.soft_ver2, &str[52], 32);
}


bool bpuair_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
//	bool ret = false;

	log_info("bpuair_do_update_device_data attri %u\n", obj->attr);

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_bpuair_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_EVM_BPUAIR_VERSION:
			bpuair_do_update_ver(air_ctrl, action, obj);
			break;
		case UCAT_EVM_BPUAIR_FAULT_HISTORY:
			bpuair_do_update_fault_history(air_ctrl, action, obj);
			break;
		case UCAT_EVM_BPUAIR_FAULT_CURRENT:
			bpuair_do_update_fault_current(air_ctrl, action, obj);
			break;
		default:
			return false;
	}
	
	return true;
}

////////////////////////////////////////////////////////////////////

/*
	功能: 对空调控制器进行设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_BPUAIR_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_bpuair_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, value);
}

/*
	功能:一次性定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@enable: 是否使能
		@onoff: 开还是关 0 关 1 开
		@hour: 定时小时
		@min: 定时分钟
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_bpuair_once_timer(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int8_t hour, u_int8_t min)
{
	u_int32_t value;
	
	CL_CHECK_INIT;

	//value = BUILD_U32(enable, onoff, hour, min);
	value = BUILD_U32(min, hour, onoff, enable);
	
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_BPUAIR_ONCE_TIMER, value);
}

/*
	功能: 周期定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@id: 1 - 7
		@enable: 是否使能
		@week:  星期几，bit 0 - 6对应星期1-7
		@onoff: 0 关 1 开
		@hour: 定时小时
		@min: 定时分钟
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_bpuair_period_timer(cl_handle_t dev_handle, u_int8_t id, u_int8_t enable, u_int8_t week, u_int8_t onoff, u_int8_t hour, u_int8_t min)
{
	cl_bpuair_timer_set_t request;
	
	CL_CHECK_INIT;

	if (id == 0 || id > 6) {
		return RS_INVALID_PARAM;
	}

	request.id = id;
	request.valid = enable;
	request.week = week & 0x7f;
	request.onoff = onoff;
	request.hour = hour;
	request.mins = min;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_BPUAIR_PERIOD_TIMER, (u_int8_t*)&request, sizeof(request));
}

