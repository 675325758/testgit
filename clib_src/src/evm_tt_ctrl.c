#include "evm_tt_ctrl.h"
#include "cl_evm_tt.h"
/*
	纯透传设备
*/

/**
	处理透传的串口命令
*/
static void _evm_tt_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	cl_evm_tt_info_t *pv;
	int i = 0;

	log_debug("_evm_tt_do_update_cmd_state, cmd len %u\n", cmd_len);
	
	
	pv = (cl_evm_tt_info_t*)ac->com_udp_dev_info.device_info;


	if (cmd_len > MAX_EVM_TT_BUF_SIZE) {
		log_err(false, "cmd len %u too big\n", cmd_len);
		return;
	}
	
	if (pv->num > MAX_EVM_TT_BUF_NUM) {
		log_err(false, "num %u , now is full\n");
		return;
	}
	
	pv->caches[pv->num].widx = cmd_len;
	for (i = 0 ; i < cmd_len; i++) {
log_debug("******************evm_tt_do_update_cmd_staterecv cmd %02x\n", pcmd[i]);
	}
	
	memcpy(pv->caches[pv->num++].buf, pcmd, cmd_len);
}



static bool _evm_tt_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	uc_tlv_t* tlv;
	int remain = obj->param_len;

	if(remain < sizeof(*tlv)){
		return false;
	}

	log_debug("_yuyuan_update_tlv_data obj param len %u\n", obj->param_len);

	tlv = (uc_tlv_t*)(obj+1);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
		case TLV_TYPE_SCM_COMMAND:
			_evm_tt_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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
	
	return true;
}

bool tt_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_evm_tt_info_t* priv;
	cl_evm_tt_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_evm_tt_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;

	// 本地清空
	memset(priv, 0x00, sizeof(*priv));

	return true;
}

static bool evm_tt_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_evm_tt_info_t *pv;
	ucc_session_t *s;
//	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("evm tt first init priv data\n");

	return true;
}

bool tt_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("yuyuan_do_update_scm_data\n");

	// 初次创建私有信息结构体
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(evm_tt_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _evm_tt_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}

////////////////////////////////////////////////////////////////////


bool tt_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_evm_tt_info_t *pv;
	ucc_session_t *session;
	u_int8_t *var_param;
	int var_param_len = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_evm_tt_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	var_param = &info->u.u8_data[0];
	var_param_len = info->data_len;
	
	switch (info->action) {
		case ACT_EVM_TT_UART:
			if (var_param_len > MAX_EVM_TT_BUF_SIZE) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}
			
			log_debug("evm tt send cmdlen %d\n", var_param_len);

			res = scm_send_single_set_pkt(session, var_param, var_param_len);
			
			
			break;

		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	return res;
}


////////////////////////////////////////////////////////////////////

//纯透传虚拟机实现的通用透传接口


/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@cmd: 自定义串口命令
		@cmd_len: 命令长度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_evm_tt_send_uart_cmd(cl_handle_t dev_handle, u_int8_t *cmd, u_int8_t cmd_len)
{
	u_int8_t buf[1024] = {0};
	
	CL_CHECK_INIT;

	cmd_len = min((u_int8_t)sizeof(buf), cmd_len);
	memcpy(buf, cmd, cmd_len);
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_EVM_TT_UART, buf, cmd_len);
}

CLIB_API int cl_get_tt_cmd_info(cl_handle_t dev_handle, void *info)
{
	cl_evm_tt_info_t *pinfo = NULL;
	cl_dev_info_t *infot = NULL;

	if (0 == dev_handle) {
		return -1;
	}

	infot = cl_user_get_dev_info(dev_handle);
	if (!infot) {
		return -2;
	}

	if (infot->com_udp_info == NULL) {
		return -4;
	}

	pinfo = (cl_evm_tt_info_t *)infot->com_udp_info->device_info;


	if (pinfo) {
		memcpy(info, pinfo, sizeof(*pinfo));
		cl_user_free_dev_info(infot);
	} else {
		return -3;
	}
	
	return 0;
}



