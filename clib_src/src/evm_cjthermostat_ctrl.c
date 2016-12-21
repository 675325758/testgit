#include "evm_cjthermostat_ctrl.h"
#include "cl_cjthermostat.h"

#define UCAT_EVM_CJTHERMOSTAT_PERIOD_TEMP 1

typedef struct {
	u_int16_t obj;
	u_int16_t sub_obj;
	u_int16_t attr;
} query_obj_t;

static query_obj_t query_objs[] = {
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_CJTHERMOSTAT_PERIOD_TEMP},
};


static bool _evm_cjthermostat_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	cl_cjthermostat_info_t *pv = (cl_cjthermostat_info_t*)ac->com_udp_dev_info.device_info;
	cjthermostat_uart_stat_t *stat = (cjthermostat_uart_stat_t *)pcmd;

	if (cmd_len < sizeof(*stat)) {
		return false;
	}

	pv->stat.outtime_hour = ntohs(stat->outtime_hour);
	pv->stat.ver = stat->ver;
	pv->stat.is_heat = stat->is_heat;
	//pv->stat.week = ((stat->week << 1) & ~(0x1)) | ((stat->week >> 7) & 0x1);
	pv->stat.week = stat->week;
	pv->stat.time = stat->time;
	pv->stat.stat = stat->stat;
	pv->stat.set_temp = stat->set_temp;
	pv->stat.inside_temp = stat->inside_temp;
	pv->stat.inside_temp1 = stat->inside_temp1;
	pv->stat.outside_temp = stat->outside_temp;
	pv->stat.outside_temp1 = stat->outside_temp1;
	pv->stat.mode = stat->mode;
	pv->stat.power = stat->power;
	pv->stat.key_lock = stat->key_lock;
	pv->stat.fault = stat->fault;
	pv->stat.temp_adjust = stat->temp_adjust;
	pv->stat.set_temp_upper_limit = stat->set_temp_upper_limit;
	pv->stat.set_temp_lower_limit = stat->set_temp_lower_limit;
	pv->stat.temp_allowance = stat->temp_allowance;
	pv->stat.defrost_temp = stat->defrost_temp;
	pv->stat.overtemp = stat->overtemp;
	pv->stat.overtemp_allowance = stat->overtemp_allowance;
	pv->stat.flag = stat->flag;
	pv->stat.timer_week = stat->timer_week;
	pv->stat.manual_temp = stat->manual_temp;
	
	return true;
}



static bool _evm_cjthermostat_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;
	bool modify = true;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_evm_cjthermostat_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
			case TLV_TYPE_SCM_COMMAND:
			if (_evm_cjthermostat_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len) == true) {
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

bool cjthermostat_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_cjthermostat_info_t* priv;
	cl_cjthermostat_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_cjthermostat_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;
	
	return true;
}

static bool evm_cjthermostat_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_cjthermostat_info_t *pv;
	ucc_session_t *s;
	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("cjthermostat first init priv data\n");

	// 把非透传的信息查询一次，后面靠推送
	for (i = 0; i < ARRAY_SIZE(query_objs); i++) {
		sa_query_obj(s, query_objs[i].obj, query_objs[i].sub_obj, query_objs[i].attr);
	}
	
	return true;
}

bool cjthermostat_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("cjthermostat_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_cjthermostat_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_cjthermostat_update_tlv_data(air_ctrl,action,obj);
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
static u_int8_t cjthermostat_uart_calc_checksum(u_int8_t *buf, int len)
{
	int i;
	u_int8_t checksum = 0;
	
	for (i = 0; i < len; i++) {
		checksum += buf[i];	
	}

	return ~checksum;
}
*/

u_int8_t cjthermostat_calc_checksum(u_int8_t *data, u_int8_t len)
{
	u_int8_t checksum = 0;
	int i;
	
	for (i = 0; i < len; i++) {
		checksum += data[i];
	}

	return checksum;
}

void cjthermostat_send_cmd(void *uc_session, u_int8_t cmd, u_int8_t *data, u_int8_t len)
{
	u_int8_t buf[256] = {0};
	cjthermostat_uart_hdr_t *hd = (cjthermostat_uart_hdr_t *)buf;
	int plen = sizeof(*hd) + len;

	hd->syn = 0x6A;
	hd->addr = 0;
	hd->cmd = cmd;

	memcpy((u_int8_t*)&hd[1], data, len);

	buf[plen] = cjthermostat_calc_checksum(buf, plen);

	scm_send_single_set_pkt(uc_session, buf, plen + 1);
}

void cjthermostat_send_keystat(void *uc_session, cl_cjthermostat_info_t *pv)
{
	u_int8_t buf[8] = {0};

	buf[0] = pv->stat.key_lock;
	buf[1] = pv->stat.power;

	cjthermostat_send_cmd(uc_session, 0xa2, buf, sizeof(buf));
}

void cjthermostat_send_autoperiod(void *uc_session, cl_cjthermostat_info_t *pv)
{
	u_int8_t buf[8] = {0};

	buf[0] = pv->stat.timer_week;

	cjthermostat_send_cmd(uc_session, 0xa4, buf, sizeof(buf));
}

void cjthermostat_send_tempset(void *uc_session, cl_cjthermostat_info_t *pv)
{
	u_int8_t buf[8] = {0};

	buf[0] = pv->stat.temp_adjust;
	buf[1] = pv->stat.set_temp_upper_limit;
	buf[2] = pv->stat.set_temp_lower_limit;
	buf[3] = pv->stat.temp_allowance;
	buf[4] = pv->stat.defrost_temp;
	buf[5] = pv->stat.overtemp;
	buf[6] = pv->stat.overtemp_allowance;
	buf[7] = pv->stat.flag;

	cjthermostat_send_cmd(uc_session, 0xa5, buf, sizeof(buf));
}

void cjthermostat_send_mode(void *uc_session, cl_cjthermostat_info_t *pv)
{
	u_int8_t buf[8] = {0};

	buf[0] = pv->stat.mode;

	cjthermostat_send_cmd(uc_session, 0xa6, buf, sizeof(buf));
}

void cjthermostat_send_manualtemp(void *uc_session, cl_cjthermostat_info_t *pv)
{
	u_int8_t buf[8] = {0};

	buf[0] = pv->stat.manual_temp;

	cjthermostat_send_cmd(uc_session, 0xa7, buf, sizeof(buf));
}

void cjthermostat_send_periodtemp(void *uc_session, u_int8_t type, cl_cjthermostat_info_t *pv)
{
	u_int8_t *buf, cmd;

	if (type == 0) {
		buf = pv->work_period_temp;
		cmd = 0xc0;
	} else {
		buf = pv->offday_period_temp;
		cmd = 0xc1;
	}

	cjthermostat_send_cmd(uc_session, cmd, buf, 48);
}


bool cjthermostat_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_cjthermostat_info_t *pv;
	ucc_session_t *session;
	u_int8_t u8value = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_cjthermostat_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	u8value = cci_u8_data(info);

	switch (info->action) {
		case ACT_CJTHERMOSTAT_LOCK:
			pv->stat.key_lock = u8value;
			cjthermostat_send_keystat(session, pv);
			break;
			
		case ACT_CJTHERMOSTAT_POWER:
			pv->stat.power = u8value;
			
			cjthermostat_send_keystat(session, pv);
			break;
		case ACT_CJTHERMOSTAT_TIMER_WEEK:
			// 因为对端要求的bit0是星期一，这里需要转下			
			pv->stat.timer_week = ((u8value >> 1) & 0x3f) | ((u8value << 6) & 0x40);
			
			cjthermostat_send_autoperiod(session, pv);
			break;
		case ACT_CJTHERMOSTAT_TEMP_ADJUST:
			pv->stat.temp_adjust = (int8_t)u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_TEMP_UPPER_LIMIT:
			pv->stat.set_temp_upper_limit = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_TEMP_LOWER_LIMIT:
			pv->stat.set_temp_lower_limit = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_TEMP_ALLOWANCE:
			pv->stat.temp_allowance = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_DEFROST_TEMP:
			pv->stat.defrost_temp = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_OVERTEMP:
			pv->stat.overtemp = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_OVERTEMP_ALLOWANCE:
			pv->stat.overtemp_allowance = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_FLAG:
			pv->stat.flag = u8value;
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_RESET:
			// 把温设的值还原成默认值
			pv->stat.temp_adjust = -2;
			pv->stat.set_temp_upper_limit = 60;
			pv->stat.set_temp_lower_limit = 5;
			pv->stat.temp_allowance = 1;
			pv->stat.defrost_temp = 4;
			pv->stat.overtemp = 60;
			pv->stat.overtemp_allowance = 5;
			pv->stat.flag = 0;
			
			cjthermostat_send_tempset(session, pv);
			break;
		case ACT_CJTHERMOSTAT_MODE:
			pv->stat.mode = u8value;

			cjthermostat_send_mode(session, pv);
			break;
		case ACT_CJTHERMOSTAT_MANUAL_TEMP:
			pv->stat.manual_temp = u8value;

			cjthermostat_send_manualtemp(session, pv);
			break;

		case ACT_CJTHERMOSTAT_PERIOD_TEMP_WORKDAY:
			memcpy(pv->work_period_temp, cci_pointer_data(info), 48);
			cjthermostat_send_periodtemp(session, 0, pv);
			break;
			
		case ACT_CJTHERMOSTAT_PERIOD_TEMP_OFFDAY:
			memcpy(pv->offday_period_temp, cci_pointer_data(info), 48);
			cjthermostat_send_periodtemp(session, 1, pv);
			break;
			
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	
	return res;
}

static void cjthermostat_do_update_temp(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{	
	cl_cjthermostat_info_t * pv = (cl_cjthermostat_info_t *)(ac->com_udp_dev_info.device_info);
	u_int8_t *ptr = (u_int8_t *)&obj[1];

	if (obj->param_len < 48 * 2) {
		return;
	}

	memcpy(pv->work_period_temp, ptr, 48);
	memcpy(pv->offday_period_temp, &ptr[48], 48);
}


bool cjthermostat_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{

	log_info("cjthermostat_do_update_device_data attri %u\n", obj->attr);

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_cjthermostat_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_EVM_CJTHERMOSTAT_PERIOD_TEMP:
			cjthermostat_do_update_temp(air_ctrl, action, obj);
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
		@action: 控制类型ACT_CJTHERMOSTAT_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_cjthermostat_ctrl(cl_handle_t dev_handle, u_int8_t action, int8_t value)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, (u_int8_t)value);
}

/*
	功能: 对创佳温控器的周期时间进行设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@type: 0 设置工作日温度 1 设置休息日温度
		@temp: 48个温度点，每个点表示半小时
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_cjthermostat_period_temp(cl_handle_t dev_handle, u_int8_t type, u_int8_t temp[48])
{
	u_int8_t action;
	
	CL_CHECK_INIT;

	if (type > 1) {
		return RS_INVALID_PARAM;
	}

	if (type == 0) {
		action = ACT_CJTHERMOSTAT_PERIOD_TEMP_WORKDAY;
	} else {
		action = ACT_CJTHERMOSTAT_PERIOD_TEMP_OFFDAY;
	}
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, temp, 48);
}
