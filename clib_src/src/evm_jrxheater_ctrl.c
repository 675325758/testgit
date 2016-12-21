/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: 进睿芯光芒热水器
**  File:    evm_jrxheater_ctrl.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/06/2016
**
**  Purpose:
**    进睿芯光芒热水器.
**************************************************************************/


/* Include files. */
#include "evm_jrxheater_ctrl.h"
#include "cl_jrxheater.h"

/* Macro constant definitions. */


/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

static void jrxheater_update_once_timer(cl_jrxheater_info_t *pv, u_int8_t valid, u_int8_t enable, u_int8_t id, u_int8_t hour, u_int8_t min, u_int8_t temp)
{
	int i;
	
	if (id == 0) {
		return;
	}

	// 先查找
	for (i = 0; i < 8; i++) {
		if (pv->sample_timer[i].valid == 1 && pv->sample_timer[i].id == id) {
			goto found;
		}
	}

	// 没有再添加
	for (i = 0; i < 8; i++) {
		if (pv->sample_timer[i].valid == 0) {
			goto found;
		}
	}

	return;

found:
	pv->sample_timer[i].valid = true;
	//pv->sample_timer[i].enable = enable;
	pv->sample_timer[i].id = id;
	pv->sample_timer[i].hour = hour;
	pv->sample_timer[i].min = min;
	pv->sample_timer[i].set_temp = temp;
}

static u_int8_t jrxheater_gen_id(cl_jrxheater_info_t *pv)
{
	u_int8_t i;
	
	for (i = 0; i < ARRAY_SIZE(pv->period_timer); i++) {
		if (pv->period_timer[i].valid == 0) {
			return i + 1;
		}
	}

	return 0;
}

static void jrxheater_update_period_timer(cl_jrxheater_info_t *pv, u_int8_t valid, u_int8_t enable, u_int8_t id, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min, u_int8_t temp)
{
	int i;
	
	if (id == 0) {
		return;
	}

	// 先查找
	for (i = 0; i < 8; i++) {
		if (pv->period_timer[i].valid == 1 && pv->period_timer[i].id == id) {
			goto found;
		}
	}

	// 没有添加
	for (i = 0; i < 8; i++) {
		if (pv->period_timer[i].valid == 0) {
			goto found;
		}
	}

	return;

found:
	pv->period_timer[i].valid = true;
	pv->period_timer[i].enable = enable;
	pv->period_timer[i].id = id;
	pv->period_timer[i].start_hour = start_hour;
	pv->period_timer[i].start_min = start_min;
	pv->period_timer[i].end_hour = end_hour;
	pv->period_timer[i].end_min = end_min;
	pv->period_timer[i].set_temp = temp;
}


static bool _evm_jrxheater_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	int i, n, len;
	u_int8_t *ptr;
	cl_jrxheater_info_t *pv = (cl_jrxheater_info_t*)ac->com_udp_dev_info.device_info;
	jrxheater_uart_hd_t *hd = (jrxheater_uart_hd_t *)pcmd;
	jrxheater_uart_stat_t *stat = (jrxheater_uart_stat_t*)&hd[1];
	jrxheater_uart_once_timer_t *ot;
	jrxheater_uart_period_timer_t *pt;

	log_debug("_evm_jrxheater_do_update_cmd_state, cmd %u cmd len %u\n", hd->cmd, cmd_len);

	if (hd->cmd != JRX_UART_CMD_PUSH_STAT) {
		log_err(false, "hd cmd %u != stat\n", hd->cmd);
		return false;
	}

	if (cmd_len < sizeof(*hd) + sizeof(*stat)) {
		return false;
	}

	pv->stat.onoff = stat->onoff;
	pv->stat.mode = stat->mode;
	pv->stat.set_temp = stat->set_temp;
	pv->stat.child_lock = stat->child_lock;
	pv->stat.work_state = stat->work_state;
	pv->stat.temp = stat->temp;
	pv->stat.capacity = stat->capacity;
	pv->stat.power = stat->power;
	pv->stat.hour = stat->hour;
	pv->stat.min = stat->min;

	len = sizeof(*hd) + sizeof(*stat);
	
	// 处理一次性定时器

	ptr = (u_int8_t*)&stat[1];
	n = ptr[0];
	log_debug("has %d once timer\n", n);	

	memset(pv->sample_timer, 0x00, sizeof(pv->sample_timer));

	ot = (jrxheater_uart_once_timer_t*)(ptr + 1);
	for (i = 0; i < n; i++, ot++) {
		jrxheater_update_once_timer(pv, ot->status & BIT(0), !!(ot->status & BIT(1)), ot->id, ot->hour, ot->min, ot->temp);
	}

	ptr += 1 + sizeof(*ot) * n;
	
	// 处理周期定时器
	n = ptr[0];
	log_debug("has %d period timer\n", n);
	
	memset(pv->period_timer, 0x00, sizeof(pv->period_timer));

	pt = (jrxheater_uart_period_timer_t*)(ptr + 1);
	for (i = 0; i < n; i++, pt++) {
		jrxheater_update_period_timer(pv, pt->status & BIT(0), !!(pt->status & BIT(1)), pt->id, pt->start_hour, pt->start_min, pt->end_hour, pt->end_min, pt->temp);
	}

	return true;
}


static bool _evm_jrxheater_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;
	bool modify = true;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_evm_jrxheater_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
			case TLV_TYPE_SCM_COMMAND:
			if (_evm_jrxheater_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len) == true) {
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

bool jrxheater_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_jrxheater_info_t* priv;
	cl_jrxheater_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_jrxheater_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;
	
	return true;
}

static bool evm_jrxheater_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_jrxheater_info_t *pv;
	ucc_session_t *s;
//	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("jrxheater first init priv data\n");

	return true;
}

bool jrxheater_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("jrxheater_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_jrxheater_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_jrxheater_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////

void jrxheater_send_uart_cmd(void *uc_session, u_int8_t cmd, u_int8_t *param, u_int8_t param_len)
{
	char buf[512] = {0};
	u_int8_t i;
	u_int8_t checksum = 0;
	
	jrxheater_uart_hd_t *hd = (jrxheater_uart_hd_t *)buf;

	hd->syn1 = 0xff;
	hd->syn2 = 0xee;
	hd->cmd = cmd;
	hd->dev_type = 0;
	hd->param_len = param_len;

	memcpy(&hd[1], param, param_len);

	for (i = 0; i < sizeof(*hd) + param_len; i++) {
		checksum += buf[i];
	}

	buf[i] = checksum;

	scm_send_single_set_pkt(uc_session, buf, sizeof(*hd) + param_len + 1);
}

static void jrxheater_ctrl_stat(void *uc_session, cl_jrxheater_info_t *pv)
{
	jrxheater_uart_set_stat_t stat;

	stat.onoff = pv->stat.onoff;
	stat.mode = pv->stat.mode;
	stat.set_temp = pv->stat.set_temp;
	stat.childlock = pv->stat.child_lock;

	jrxheater_send_uart_cmd(uc_session, JRX_UART_CMD_STAT, (u_int8_t*)&stat, sizeof(stat));
}
 
static void jrxheater_ctrl_period_timer(cl_jrxheater_info_t *pv, void *uc_session, jrxheater_uart_period_timer_t *pt)
{
	u_int8_t id = pt->id;
	
	// 先更新本地，如果ID为0，表示是添加
	// 这里测试，先找个空闲的ID
	if (id == 0) {
		id = jrxheater_gen_id(pv);
	}
	jrxheater_update_period_timer(pv, pt->status & BIT(0), !!(pt->status & BIT(1)), id, pt->start_hour, pt->start_min, pt->end_hour, pt->end_min, pt->temp);
	
	jrxheater_send_uart_cmd(uc_session, JRX_UART_CMD_PERIOD_TIMER, (u_int8_t*)pt, sizeof(*pt));
}

bool jrxheater_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_jrxheater_info_t *pv;
	ucc_session_t *session;
	u_int32_t u32value = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_jrxheater_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	u32value = cci_u32_data(info);

	switch (info->action) {
		case ACT_JRXHEATER_ONOFF:
			pv->stat.onoff = (u_int8_t)u32value;
			jrxheater_ctrl_stat(ac->sac->user->uc_session, pv);
			break;
		case ACT_JRXHEATER_MODE:
			pv->stat.mode = (u_int8_t)u32value;
			jrxheater_ctrl_stat(ac->sac->user->uc_session, pv);
			break;
		case ACT_JRXHEATER_TEMP:
			pv->stat.set_temp = (u_int8_t)u32value;
			jrxheater_ctrl_stat(ac->sac->user->uc_session, pv);
			break;
		case ACT_JRXHEATER_CHILDLOCK:
			pv->stat.child_lock = (u_int8_t)u32value;
			jrxheater_ctrl_stat(ac->sac->user->uc_session, pv);
			break;
		case ACT_JRXHEATER_PERIOD_TIMER:
			jrxheater_ctrl_period_timer(pv, ac->sac->user->uc_session, (jrxheater_uart_period_timer_t *)cci_pointer_data(info));
			break;
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}

	return res;
}

bool jrxheater_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	return ret;
}

////////////////////////////////////////////////////////////////////

/*
	功能: 基本设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_JRXHEATER_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jrxheater_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, value);
}

/*
	功能: 周期定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 操作1 添加2 关闭 3 修改 4 删除
		@valid: 是否存在
		@enable: 是否使能
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jrxheater_period_timer(cl_handle_t dev_handle, u_int8_t action, u_int8_t valid, u_int8_t enable, u_int8_t id, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min, u_int8_t temp)
{
	jrxheater_uart_period_timer_t request;

	memset((u_int8_t*)&request, 0x00, sizeof(request));
	
	CL_CHECK_INIT;

	if (valid) {
		request.status |= 1;
	}
	
	if (enable) {
		request.status |= 1 << 1;
	}

	request.id = id;
	request.start_hour = start_hour;
	request.start_min = start_min;
	request.end_hour = end_hour;
	request.end_min = end_min;
	request.temp = temp;
	request.onoff = 1;	// 都是开机

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_JRXHEATER_PERIOD_TIMER, (u_int8_t*)&request, sizeof(request));
}

