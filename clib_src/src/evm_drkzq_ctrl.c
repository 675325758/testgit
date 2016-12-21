#include "evm_drkzq_ctrl.h"
#include "cl_drkzq.h"

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

#define UCAT_EVM_DRKZQ_EXCEPTION 1
#define UCAT_EVM_DRKZQ_NAME 2

typedef struct {
	u_int16_t obj;
	u_int16_t sub_obj;
	u_int16_t attr;
} query_obj_t;

static query_obj_t query_objs[] = {
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_DRKZQ_EXCEPTION},
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_DRKZQ_NAME},
};


static bool _evm_drkzq_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
//	u_int8_t at_cmd[512] = {0};
	cl_drkzq_info_t *pv = (cl_drkzq_info_t*)ac->com_udp_dev_info.device_info;
	drkzq_uart_query_reply_t *stat = (drkzq_uart_query_reply_t *)pcmd;

//	log_debug("_evm_drkzq_do_update_cmd_state, cmd %u cmd len %u\n", hd->cmd, cmd_len);

	if (cmd_len < sizeof(*stat)) {
		return false;
	}

	memcpy((u_int8_t*)&pv->stat, pcmd, sizeof(pv->stat));

	return true;
}



static bool _evm_drkzq_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;
	bool modify = true;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_evm_drkzq_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
			case TLV_TYPE_SCM_COMMAND:
			if (_evm_drkzq_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len) == true) {
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

bool drkzq_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_drkzq_info_t* priv;
	cl_drkzq_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_drkzq_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;
	
	return true;
}

static bool evm_drkzq_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_drkzq_info_t *pv;
	ucc_session_t *s;
	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("drkzq first init priv data\n");

	// 把非透传的信息查询一次，后面靠推送
	for (i = 0; i < ARRAY_SIZE(query_objs); i++) {
		sa_query_obj(s, query_objs[i].obj, query_objs[i].sub_obj, query_objs[i].attr);
	}
	
	return true;
}

bool drkzq_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("drkzq_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_drkzq_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_drkzq_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////
static u_int8_t drkzq_uart_calc_checksum(u_int8_t *buf, int len)
{
	int i;
	u_int8_t checksum = 0;
	
	for (i = 0; i < len; i++) {
		checksum += buf[i];	
	}

	return checksum;
}


void drkzq_send_cmd(void *uc_session, u_int8_t cmd1, u_int8_t cmd2, u_int8_t *data, u_int8_t len)
{
	u_int8_t buf[256] = {0}, *ptr;
	drkzq_uart_hdr_t *hd = (drkzq_uart_hdr_t *)buf;
	int plen = sizeof(*hd) + len + 1;

	hd->syn = 0x55;
	hd->cmd1 = cmd1;
	hd->len = len + 1;	// 加上二级指令长度

	ptr = (u_int8_t*)&hd[1];
	ptr[0] = cmd2;
	memcpy((u_int8_t*)&ptr[1], data, len);

	
	//buf[plen++] = drkzq_uart_calc_checksum(buf, plen);
	buf[plen] = drkzq_uart_calc_checksum(buf, plen);
	plen++;
	buf[plen++] = 0xaa;

	scm_send_single_set_pkt(uc_session, buf, plen);
}

bool drkzq_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_drkzq_info_t *pv;
	ucc_session_t *session;
	u_int32_t u32value = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_drkzq_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	u32value = cci_u32_data(info);

	switch (info->action) {
		case act_drkzq_humi_threshold1:
			pv->stat.humi_threshold1 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 1, &pv->stat.humi_threshold1, 1);
			break;
		case act_drkzq_humi_threshold2:
			pv->stat.humi_threshold2 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 2, &pv->stat.humi_threshold2, 1);
			break;
		case act_drkzq_humi_threshold3:
			pv->stat.humi_threshold3 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 3, &pv->stat.humi_threshold3, 1);
			break;
		case act_drkzq_humi_threshold4:
			pv->stat.humi_threshold4 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 4, &pv->stat.humi_threshold4, 1);
			break;
		case act_drkzq_safe_temp_threshold:
			pv->stat.safe_temp_threshold = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 5, &pv->stat.safe_temp_threshold, 1);
			break;
		case act_drkzq_cycling_water_temp_threshold:
			pv->stat.cycling_water_temp_threshold = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 6, &pv->stat.cycling_water_temp_threshold, 1);
			break;
		case act_drkzq_fertilzer_hour:
			pv->stat.fertilzer_second = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 7, &pv->stat.fertilzer_second, 1);
			break;
		case act_drkzq_sunshine_threshold:
			pv->stat.sunshine_threshold = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 8, &pv->stat.sunshine_threshold, 1);
			break;
		case act_drkzq_safe_ph_threshold:
			pv->stat.safe_ph_threshold = (u_int8_t)u32value;
			drkzq_send_cmd(session, 10, 9, &pv->stat.safe_ph_threshold, 1);
			break;

		case act_drkzq_watering_onoff1:
			pv->stat.watering_onoff1 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 1, &pv->stat.watering_onoff1, 1);
			break;
		case act_drkzq_watering_onoff2:
			pv->stat.watering_onoff2 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 2, &pv->stat.watering_onoff2, 1);
			break;
		case act_drkzq_watering_onoff3:
			pv->stat.watering_onoff3 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 3, &pv->stat.watering_onoff3, 1);
			break;
		case act_drkzq_watering_onoff4:
			pv->stat.watering_onoff4 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 4, &pv->stat.watering_onoff4, 1);
			break;
		case act_drkzq_sunshine_onoff:
			pv->stat.sunshine_onoff = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 5, &pv->stat.sunshine_onoff, 1);
			break;
		case act_drkzq_feretilize_onoff:
			pv->stat.feretilize_onoff = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 6, &pv->stat.feretilize_onoff, 1);
			break;
		case act_drkzq_light_onoff:
			pv->stat.light_onoff = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 7, &pv->stat.light_onoff, 1);
			break;
		case act_drkzq_water_pump_onoff:
			pv->stat.water_pump_onoff = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 8, &pv->stat.water_pump_onoff, 1);
			break;
		case act_drkzq_resv_onoff1:
			pv->stat.resv_onoff1 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 9, &pv->stat.resv_onoff1, 1);
			break;
		case act_drkzq_resv_onoff2:
			pv->stat.resv_onoff2 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 10, &pv->stat.resv_onoff2, 1);
			break;
		case act_drkzq_resv_onoff3:
			pv->stat.resv_onoff3 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 11, &pv->stat.resv_onoff3, 1);
			break;
		case act_drkzq_resv_onoff4:
			pv->stat.resv_onoff4 = (u_int8_t)u32value;
			drkzq_send_cmd(session, 13, 12, &pv->stat.resv_onoff4, 1);
			break;
		case act_drkzq_work_mode:
			pv->stat.work_mode = (u_int8_t)u32value;
			drkzq_send_cmd(session, 11, 1, &pv->stat.work_mode, 1);
			break;

		case act_drkzq_sys_time:
			{
				u_int16_t t = (u_int16_t)u32value;

				pv->stat.sys_hour = (u_int8_t)u32value;
				pv->stat.sys_min = (u_int8_t)(u32value >> 8) & 0xff;
				
				t = ntohs(t);
				drkzq_send_cmd(session, 11, 2, (u_int8_t*)&t, sizeof(t));
			}						
			break;
		case act_drkzq_timer:
			{
				drkzq_set_timer_t *request = (drkzq_set_timer_t *)cci_pointer_data(info);
				drkzq_timer_set_param_t set;
				u_int8_t cmd2;

				if (request->id == 1) {
					cmd2 = 1;

					pv->stat.timer1_start_hour = request->start_hour;
					pv->stat.timer1_start_min = request->start_min;
					pv->stat.timer1_end_hour = request->end_hour;
					pv->stat.timer1_end_min = request->end_min;
					pv->stat.timer1_valid = request->valid;
				} else {
					cmd2 = 2;

					pv->stat.timer2_start_hour = request->start_hour;
					pv->stat.timer2_start_min = request->start_min;
					pv->stat.timer2_end_hour = request->end_hour;
					pv->stat.timer2_end_min = request->end_min;
					pv->stat.timer2_valid = request->valid;
				}

				set.valid = request->valid;
				set.start_hour = request->start_hour;
				set.start_min = request->start_min;
				set.end_hour = request->end_hour;
				set.end_min = request->end_min;

				drkzq_send_cmd(session, 12, cmd2, (u_int8_t*)&set, sizeof(set));
			}
			break;

		case act_drkzq_name:
			{
				drkzq_set_name_t *request = (drkzq_set_name_t *)cci_pointer_data(info);
				int len = sizeof(pv->name);
				
				memcpy((u_int8_t*)&pv->name[request->type], request->name, sizeof(request->name));	

				fill_net_ucp_obj(uo, UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_DRKZQ_NAME, len);
				memcpy((u_int8_t*)&uo[1], (u_int8_t*)pv->name, len);				
				sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + len);
			}
			break;

		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}

	return res;
}

static void drkzq_do_update_exception(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_drkzq_info_t *pv;
	
	pv = (cl_drkzq_info_t*)ac->com_udp_dev_info.device_info;

	if (is_obj_less_than_len(obj, sizeof(pv->fault))) {
		log_err(false, "drkzq_do_update_day_data len %u invalid\n", obj->param_len);
		return;
	}

	memcpy((u_int8_t*)&pv->fault, (u_int8_t*)&obj[1], sizeof(pv->fault));
}

static void drkzq_do_update_name(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_drkzq_info_t *pv;
	int i;
	
	pv = (cl_drkzq_info_t*)ac->com_udp_dev_info.device_info;

	if (is_obj_less_than_len(obj, sizeof(pv->name))) {
		log_err(false, "drkzq_do_update_name len %u invalid\n", obj->param_len);
		return;
	}

	memcpy((u_int8_t*)&pv->name, (u_int8_t*)&obj[1], sizeof(pv->name));

	for (i = 0; i < DRKZQ_NAME_MAX; i++) {
		log_debug("name idx %d name[%s]\n", i, pv->name[i].name);
	}
}


bool drkzq_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
//	bool ret = false;

	log_info("drkzq_do_update_device_data attri %u\n", obj->attr);

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_drkzq_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_EVM_DRKZQ_EXCEPTION:
			drkzq_do_update_exception(air_ctrl, action, obj);
			break;
		case UCAT_EVM_DRKZQ_NAME:
			drkzq_do_update_name(air_ctrl, action, obj);
			break;
		default:
			return false;
	}
	
	return true;
}

////////////////////////////////////////////////////////////////////

/*
	功能: 对德仁控制器的通用控制
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_DRKZQ_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_drkzq_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, value);
}

/*
	功能: 对净化器设置某个模块的名字
		
	输入参数:
		@dev_handle: 设备的句柄
		@type: 名字DRKZQ_NAME_XXX
		@name: 名字，长度需要小于64字节
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_drkzq_set_name(cl_handle_t dev_handle, u_int8_t type, char *name)
{
	drkzq_set_name_t request;
	
	CL_CHECK_INIT;

	memset((u_int8_t*)&request, 0x00, sizeof(request));

	if (type >= DRKZQ_NAME_MAX) {
		return RS_INVALID_PARAM;
	}

	if (strlen(name) >= sizeof(request.name)) {
		return RS_INVALID_PARAM;
	}

	request.type = type;
	memcpy(request.name, name, strlen(name));
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, act_drkzq_name, (u_int8_t*)&request, sizeof(request));
}


/*
	功能: 对净化器设置某个定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@id: 目前只支持定时器1和2
		@valid:开启还是关闭定时器
		@start_hour start_min: 开始的小时分钟
		@end_hour end_min: 结束的小时分钟
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_drkzq_set_timer(cl_handle_t dev_handle, u_int8_t id, u_int8_t valid, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min)
{
	
	drkzq_set_timer_t request;
	
	CL_CHECK_INIT;

	if (id == 0 || id > 2) {
		return RS_INVALID_PARAM;
	}

	request.id = id;
	request.valid = valid;
	request.start_hour = start_hour;
	request.start_min = start_min;
	request.end_hour = end_hour;
	request.end_min = end_min;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, act_drkzq_timer, (u_int8_t*)&request, sizeof(request));
}

