#include "evm_hythermostat_ctrl.h"
#include "cl_hythermostat.h"



// 华祐温控器设置数据地址
enum {
    SET_ADDR_ONOFF = 5,
    SET_ADDR_TEMP = 6,
    SET_ADDR_MODE = 7,
    SET_ADDR_WIND = 8,
    SET_ADDR_RHFUN = 9,
    SET_ADDR_RHVAL = 10,

    SET_ADDR_MAX = 101,
};

typedef struct {
	u_int16_t obj;
	u_int16_t sub_obj;
	u_int16_t attr;
} query_obj_t;

/*
static query_obj_t query_objs[] = {
	{UCOT_IA, UCSOT_IA_TT, UCAT_IA_TT_ALLSTATE},
};
*/

static bool evm_hythermostat_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_hythermostat_info_t *pv;
	ucc_session_t *s;
	//int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("hythermostat first init priv data\n");


	// 把非透传的信息查询一次，后面靠推送
	//for (i = 0; i < ARRAY_SIZE(query_objs); i++) {
		//sa_query_obj(s, query_objs[i].obj, query_objs[i].sub_obj, query_objs[i].attr);
	//}


	return true;
}

static bool _evm_hythermostat_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	//u_int8_t at_cmd[512] = {0};
	cl_hythermostat_info_t *pv = (cl_hythermostat_info_t*)ac->com_udp_dev_info.device_info;
    cl_hythermostat_stat_t *stat = (cl_hythermostat_stat_t*)pcmd;

	log_debug("_evm_hythermostat_do_update_cmd_state, cmd len %u\n", cmd_len);

	if (cmd_len < sizeof(*stat)) {
		return false;
	}

	pv->stat.mcuver = ntohs(stat->mcuver);
	pv->stat.type = ntohs(stat->type);
	pv->stat.temp = ntohs(stat->temp);
	pv->stat.valve = ntohs(stat->valve);
	pv->stat.onoff = ntohs(stat->onoff);
	pv->stat.settemp = ntohs(stat->settemp);
    pv->stat.mode = ntohs(stat->mode);
	pv->stat.wind = ntohs(stat->wind);
	pv->stat.RHfun = ntohs(stat->RHfun);
	pv->stat.RHval = ntohs(stat->RHval);
	pv->stat.RHstate = ntohs(stat->RHstate);

	return true;
}

static bool _evm_hythermostat_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;
	bool modify = true;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_evm_hythermostat_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
			case TLV_TYPE_SCM_COMMAND:
			if (_evm_hythermostat_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len) == true) {
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

bool hythermostat_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("hythermostat_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_hythermostat_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_hythermostat_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}


bool hythermostat_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_hythermostat_info_t* priv;
	cl_hythermostat_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_hythermostat_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;
	
	return true;
}

static void hythermostat_ctrl_stat(void *uc_session, cl_hythermostat_stat_t *stat, u_int16_t dataAddr, u_int16_t data)
{
	u_int8_t buf[16] = {0};


    buf[0] = (dataAddr>>8)& 0xFF;
    buf[1] = dataAddr & 0xFF;
    buf[2] = (data>>8)& 0xFF;
    buf[3] = data & 0xFF;
	

	scm_send_single_set_pkt(uc_session, buf, 4);
}

bool hythermostat_scm_proc_notify(user_t* user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_hythermostat_info_t *pv;
	ucc_session_t *session;
	u_int32_t u32value = 0;
    
	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_hythermostat_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	u32value = cci_u32_data(info);
    
	switch (info->action) {
		case ACT_HYTHERMOSTAT_ONOFF:
			pv->stat.onoff = (u_int8_t)u32value;
			hythermostat_ctrl_stat(session, &pv->stat, SET_ADDR_ONOFF, pv->stat.onoff);
			break;
		case ACT_HYTHERMOSTAT_TEMP:
			pv->stat.settemp= (u_int16_t)u32value;
			hythermostat_ctrl_stat(session, &pv->stat, SET_ADDR_TEMP, pv->stat.settemp);
			break;
		case ACT_HYTHERMOSTAT_MODE:
			pv->stat.mode= (u_int8_t)u32value;
			hythermostat_ctrl_stat(session, &pv->stat, SET_ADDR_MODE, pv->stat.mode);
			break;
		case ACT_HYTHERMOSTAT_WIND:
			pv->stat.wind = (u_int8_t)u32value;
			hythermostat_ctrl_stat(session, &pv->stat, SET_ADDR_WIND, pv->stat.wind);
			break;
		case ACT_HYTHERMOSTAT_RHFUN:
			pv->stat.RHfun = (u_int8_t)u32value;
			hythermostat_ctrl_stat(session, &pv->stat, SET_ADDR_RHFUN, pv->stat.RHfun);
			break;
		case ACT_HYTHERMOSTAT_RHVAL:
			pv->stat.RHval = (u_int8_t)u32value;
			hythermostat_ctrl_stat(session, &pv->stat, SET_ADDR_RHVAL, pv->stat.RHval);
			break;
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	
	return res;
}



////////////////////////////////////////////////////////////////////

/*
	功能: 对温控器设置报文
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_HYTHERMOSTAT_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_hythermostat_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, action, value);
}

