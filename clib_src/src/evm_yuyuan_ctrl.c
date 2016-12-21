#include "evm_yuyuan_ctrl.h"

#define UCAT_EVM_YUYUAN_HISTROY_WATER 1
#define UCAT_EVM_YUYUAN_PWD 2
#define UCAT_EVM_YUYUAN_REMIND 3

typedef struct {
	u_int16_t obj;
	u_int16_t sub_obj;
	u_int16_t attr;
} yuyuan_query_obj_t;

static yuyuan_query_obj_t yy_query_objs[] = {
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_YUYUAN_HISTROY_WATER},
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_YUYUAN_PWD},
	{UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_YUYUAN_REMIND},
};

/*
	��Դ��ˮ��
*/

/**
	����͸���Ĵ�������
*/
static void _yuyuan_do_update_cmd_state(smart_air_ctrl_t* ac, u_int8_t* pcmd, u_int16_t cmd_len)
{
	u_int8_t at_cmd[512] = {0};
	char config[512];
	cl_yuyuan_info_t *pv;
	cl_yuyuan_state_t *state;
	char *token = NULL;
	u_int32_t param[8];
	int ret, i;

	log_debug("_yuyuan_do_update_cmd_state, cmd len %u\n", cmd_len);
	
	
	pv = (cl_yuyuan_info_t*)ac->com_udp_dev_info.device_info;
	state = &pv->state;

	memcpy(at_cmd, pcmd, min(512, cmd_len));
	
	log_debug("xxx yuyuan update state [%s]\n", at_cmd);

	// ��������
	/*
		AT+REPLY&WORK_STAT=0, WATER_LEVEL=1, WATER_BOX=1, WATER_USED_IMPULSE=10, 
		INLET_TIMEOUT=20, IMPULSE_COUNT=3, IMPULSE_PERIOD=500,MCDELAY=115, NM_VALVE_DELAY=100,
		FUNC_VALVE_TIMEOUT=100, SPEED1=11,SPEED2=12,SPEED3=13,LOOP_ONOFF=1,LOOPD=20,LOOPH=1,LOOPM=30,LOOPT=5, 
		MCCLEAN_D = 1��MCCLEAN_H=12, MCCLEAN_M=30, MCCLEAN_RT=14, MCCLEAN_DT=14, 
		NMCLEAN_D=20, NMCLEAN_H=12, NMCLEAN_M=30, NMCLEAN_RT=14, NMCLEAN_DT=14,
		ERROR=1
	*/

	if ((token = strstr(at_cmd, "WORK_STAT")) != NULL && (ret = sscanf(token, "WORK_STAT=%u", &param[0])) == 1) {
		state->WORK_STAT = param[0];
	}

	if ((token = strstr(at_cmd, "WATER_LEVEL")) != NULL && (ret = sscanf(token, "WATER_LEVEL=%u", &param[0])) == 1) {
		state->WATER_LEVEL = param[0];
	}

	if ((token = strstr(at_cmd, "WATER_BOX")) != NULL && (ret = sscanf(token, "WATER_BOX=%u", &param[0])) == 1) {
		state->WATER_BOX = param[0];
	}

	if ((token = strstr(at_cmd, "WATER_USED_IMPULSE")) != NULL && (ret = sscanf(token, "WATER_USED_IMPULSE=%u", &param[0])) == 1) {
		state->WATER_USED_IMPULSE = param[0];
	}

	if ((token = strstr(at_cmd, "INLET_TIMEOUT")) != NULL && (ret = sscanf(token, "INLET_TIMEOUT=%u", &param[0])) == 1) {
		state->INLET_TIMEOUT = param[0];
	}

	if ((token = strstr(at_cmd, "IMPULSE_COUNT")) != NULL && (ret = sscanf(token, "IMPULSE_COUNT=%u", &param[0])) == 1) {
		state->IMPULSE_COUNT = param[0];
	}

	if ((token = strstr(at_cmd, "IMPULSE_PERIOD")) != NULL && (ret = sscanf(token, "IMPULSE_PERIOD=%u", &param[0])) == 1) {
		state->IMPULSE_PERIOD = param[0];
	}

	if ((token = strstr(at_cmd, "MCDELAY")) != NULL && (ret = sscanf(token, "MCDELAY=%u", &param[0])) == 1) {
		state->MCDELAY = param[0];
	}

	if ((token = strstr(at_cmd, "NM_VALVE_DELAY")) != NULL && (ret = sscanf(token, "NM_VALVE_DELAY=%u", &param[0])) == 1) {
		state->NM_VALVE_DELAY = param[0];
	}

	if ((token = strstr(at_cmd, "FUNC_VALVE_TIMEOUT")) != NULL && (ret = sscanf(token, "FUNC_VALVE_TIMEOUT=%u", &param[0])) == 1) {
		state->FUNC_VALVE_TIMEOUT = param[0];
	}

	// SPEED1=11,SPEED2=12,SPEED3=13
	if ((token = strstr(at_cmd, "SPEED1")) != NULL && (ret = sscanf(token, "SPEED1=%u,SPEED2=%u,SPEED3=%u", &param[0], &param[1], &param[2])) == 3) {
		state->SPEED1 = param[0];
		state->SPEED2 = param[1];
		state->SPEED3 = param[2];
	}

	if ((token = strstr(at_cmd, "LOOP_ONOFF")) != NULL && (ret = sscanf(token, "LOOP_ONOFF=%u", &param[0])) == 1) {
		state->LOOP_ONOFF = param[0];
	}

	if ((token = strstr(at_cmd, "LOOPD")) != NULL && (ret = sscanf(token, "LOOPD=%u,LOOPH=%u,LOOPM=%u,LOOPT=%u", &param[0], &param[1], &param[2], &param[3])) == 4) {
		state->LOOPD = param[0];
		state->LOOPH = param[1];
		state->LOOPM = param[2];
		state->LOOPT = param[3];
	}

	// MCCLEAN_D=1,MCCLEAN_H=12, MCCLEAN_M=30, MCCLEAN_RT=14, MCCLEAN_DT=14,
	if ((token = strstr(at_cmd, "MCCLEAN_D")) != NULL && (ret = sscanf(token, "MCCLEAN_D=%u,MCCLEAN_H=%u,MCCLEAN_M=%u,MCCLEAN_RT=%u,MCCLEAN_DT=%u", &param[0], &param[1], &param[2], &param[3], &param[4])) == 5) {
		state->MCCLEAN_D = param[0];
		state->MCCLEAN_H = param[1];
		state->MCCLEAN_M = param[2];
		state->MCCLEAN_RT = param[3];
		state->MCCLEAN_DT = param[4];
	}

	// NMCLEAN_D=20, NMCLEAN_H=12, NMCLEAN_M=30, NMCLEAN_RT=14, NMCLEAN_DT=14,
	if ((token = strstr(at_cmd, "NMCLEAN_D")) != NULL && (ret = sscanf(token, "NMCLEAN_D=%u,NMCLEAN_H=%u,NMCLEAN_M=%u,NMCLEAN_RT=%u,NMCLEAN_DT=%u", &param[0], &param[1], &param[2], &param[3], &param[4])) == 5) {
		state->NMCLEAN_D = param[0];
		state->NMCLEAN_H = param[1];
		state->NMCLEAN_M = param[2];
		state->NMCLEAN_RT = param[3];
		state->NMCLEAN_DT = param[4];
	}

	if ((token = strstr(at_cmd, "ERROR1")) != NULL && 
		(ret = sscanf(token, "ERROR1=%u,ERROR2=%u,ERROR3=%u,ERROR4=%u,ERROR5=%u,ERROR6=%u,ERROR7=%u,ERROR8=%u", 
		&state->ERROR_INFO[0], &state->ERROR_INFO[1], &state->ERROR_INFO[2], &state->ERROR_INFO[3], &state->ERROR_INFO[4], &state->ERROR_INFO[5], &state->ERROR_INFO[6], &state->ERROR_INFO[7])) >= 1) 
	{
		for (i = 0; i < 8 - ret; i++) {
			state->ERROR_INFO[7 - i] = 0;
		}
		
	} else {
		for (i = 0; i < 8; i++) {
			state->ERROR_INFO[i] = 0;
		}
	}

	if ((token = strstr(at_cmd, "CONFIG")) != NULL && (ret = sscanf(token, "CONFIG=%s", config)) == 1) {
		memcpy(state->CONFIG, config, sizeof(state->CONFIG));
	} else {
		memset(state->CONFIG, 0x00, sizeof(state->CONFIG));
	}
}



static bool _yuyuan_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
			_yuyuan_do_update_cmd_state(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool yuyuan_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_yuyuan_info_t* priv;
	cl_yuyuan_info_t* info;
	
	if (!di || !ac || !ac->com_udp_dev_info.device_info) {
		return false;
	}
	
	priv = (cl_yuyuan_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;

	return true;
}

static bool yuyuan_do_init(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{	
	cl_yuyuan_info_t *pv;
	ucc_session_t *s;
	int i;

	s = air_ctrl->sac->user->uc_session;

	if ((pv = cl_calloc(1, sizeof(*pv))) == NULL) {
		log_err(true, "calloc pv failed\n");
		return false;
	}
	
	air_ctrl->com_udp_dev_info.device_info = pv;

	log_info("yuyuan first init priv data\n");

	// �ѷ�͸������Ϣ��ѯһ�Σ����濿����
	for (i = 0; i < ARRAY_SIZE(yy_query_objs); i++) {
		sa_query_obj(s, yy_query_objs[i].obj, yy_query_objs[i].sub_obj, yy_query_objs[i].attr);
	}
	
	return true;
}

bool yuyuan_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("yuyuan_do_update_scm_data\n");

	// ���δ���˽����Ϣ�ṹ��
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(yuyuan_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _yuyuan_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
			break;
		default:
			break;
	}
	
	return ret;
}

static void history_order(cl_yuyuan_water_history_t *history)
{
	int i, count;
	
	history->last_write_time = ntohl(history->last_write_time);
	history->count = ntohl(history->count);

	count = min(365, history->count);
	for (i = 0; i < count; i++) {
		history->data[i] = ntohs(history->data[i]);
	}
}

static void yuyuan_do_update_histroy_water(smart_air_ctrl_t* ac, u_int8_t action,ucp_obj_t* obj)
{
	cl_yuyuan_info_t *pv;
	cl_yuyuan_water_history_t *history;
	
	pv = (cl_yuyuan_info_t*)ac->com_udp_dev_info.device_info;
	history = (cl_yuyuan_water_history_t *)&obj[1];

	if (is_obj_less_than_len(obj, sizeof(cl_yuyuan_water_history_t))) {
		log_err(false, "history len %u < sizeof(cl_yuyuan_water_history_t) %d\n", obj->param_len, sizeof(cl_yuyuan_water_history_t));
		return;
	}

	history_order(history);

	log_debug("yuyuan water history: last_write_time %u count %u\n", history->last_write_time, history->count);

	memcpy(&pv->histroy, history, sizeof(cl_yuyuan_water_history_t));
}

static void pwd_order(cl_yuyuan_pwd_t *pwd)
{
	pwd->pwd_len = ntohl(pwd->pwd_len);
}

static void yuyuan_do_update_pwd(smart_air_ctrl_t* ac, u_int8_t action,ucp_obj_t* obj)
{
	cl_yuyuan_info_t *pv;
	cl_yuyuan_pwd_t *pwd;
	
	pv = (cl_yuyuan_info_t*)ac->com_udp_dev_info.device_info;
	pwd = (cl_yuyuan_pwd_t *)&obj[1];

	if (is_obj_less_than_len(obj, sizeof(cl_yuyuan_pwd_t))) {
		log_err(false, "pwd len %u < sizeof(cl_yuyuan_pwd_t) %d\n", obj->param_len, sizeof(cl_yuyuan_pwd_t));
		return;
	}

	pwd_order(pwd);

	log_debug("yuyuan pwd: len %u\n", pwd->pwd_len);

	memcpy(&pv->pwd, pwd, sizeof(cl_yuyuan_pwd_t));
}

static void remind_order(cl_yuyuan_remind_t *remind)
{
	remind->onoff = ntohl(remind->onoff);
	remind->remind_time = ntohl(remind->remind_time);
}

static void yuyuan_do_update_remind(smart_air_ctrl_t* ac, u_int8_t action,ucp_obj_t* obj)
{
	cl_yuyuan_info_t *pv;
	cl_yuyuan_remind_t *remind;
	
	pv = (cl_yuyuan_info_t*)ac->com_udp_dev_info.device_info;
	remind = (cl_yuyuan_remind_t *)&obj[1];

	if (is_obj_less_than_len(obj, sizeof(cl_yuyuan_remind_t))) {
		log_err(false, "remind len %u < sizeof(cl_yuyuan_remind_t) %d\n", obj->param_len, sizeof(cl_yuyuan_remind_t));
		return;
	}

	remind_order(remind);

	log_debug("yuyuan remind: valid 0x%x onoff %u remind_time %u\n", remind->valid, remind->onoff, remind->remind_time);

	memcpy(&pv->remind, remind, sizeof(cl_yuyuan_remind_t));
}



bool yuyuan_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	log_info("yuyuan_do_update_device_data attri %u\n", obj->attr);

	// ���δ���˽����Ϣ�ṹ��
	if (air_ctrl->com_udp_dev_info.device_info == NULL) {
		if (!(yuyuan_do_init(air_ctrl, action, obj))) {
			return false;
		}
	}

	switch (obj->attr) {
		case UCAT_EVM_YUYUAN_HISTROY_WATER:
			yuyuan_do_update_histroy_water(air_ctrl, action, obj);
			break;
		case UCAT_EVM_YUYUAN_PWD:
			yuyuan_do_update_pwd(air_ctrl, action, obj);
			break;
		case UCAT_EVM_YUYUAN_REMIND:
			yuyuan_do_update_remind(air_ctrl, action, obj);
			break;
		default:
			return false;
	}
	
	return true;
}


////////////////////////////////////////////////////////////////////

void yuyuan_do_set_pwd(void *uc_session, u_int8_t *param, int param_len)
{
	u_int8_t buf[64];	
    ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_yuyuan_pwd_t *pwd;

	if (param_len < sizeof(*pwd)) {
		log_err(false, "param_len %u too short\n", param_len);
		return;
	}

	pwd = (cl_yuyuan_pwd_t *)param;

	log_debug("yuyuan_do_set_pwd len %u\n", pwd->pwd_len);

	pwd_order(pwd);

	memcpy(&uo[1], pwd, sizeof(*pwd));
	
	fill_net_ucp_obj(uo, UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_YUYUAN_PWD, sizeof(*pwd));
    
    sa_set_obj_value_only(uc_session, 1, buf, sizeof(*uo) + sizeof(*pwd));
}

void yuyuan_do_set_remind(void *uc_session, u_int8_t *param, int param_len)
{
	u_int8_t buf[64];	
    ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_yuyuan_remind_t *remind;

	if (param_len < sizeof(*remind)) {
		log_err(false, "param_len %u too short\n", param_len);
		return;
	}

	remind = (cl_yuyuan_remind_t *)param;

	log_debug("yuyuan_do_set_remind onoff %u time %u\n", remind->onoff, remind->remind_time);

	remind_order(remind);

	memcpy(&uo[1], remind, sizeof(*remind));
	
	fill_net_ucp_obj(uo, UCOT_EVM, UCSOT_EVM_STAT, UCAT_EVM_YUYUAN_REMIND, sizeof(*remind));
    
    sa_set_obj_value_only(uc_session, 1, buf, sizeof(*uo) + sizeof(*remind));
}


/**
	����һ������õ�AT���\r����
*/
void yuyuan_send_cmd(void *uc_session, const char *fmt, ...)
{
	int len = 0;
	char buf[512] = {0};
	va_list vl;

    va_start(vl, fmt);
	len += vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);

	len += sprintf(&buf[len], "\r");

	log_debug("yuyuan send cmd[%s] len %d\n", buf, len);

	scm_send_single_set_pkt(uc_session, buf, len);
}


bool yuyuan_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_yuyuan_info_t *pv;
	ucc_session_t *session;
	u_int8_t *var_param;
	int var_param_len = 0;
	u_int16_t u16value;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = (cl_yuyuan_info_t *)(ac->com_udp_dev_info.device_info);
	session = ac->sac->user->uc_session;

	var_param = &info->u.u8_data[0];
	var_param_len = info->data_len;
	
	switch (info->action) {
		case ACT_YUYUAN_WATER_BOX:
			pv->state.WATER_BOX = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+WATER_BOX=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_INLET_TIMEOUT:
			pv->state.INLET_TIMEOUT = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+INLET_TIMEOUT=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_IMPULSE_COUNT:
			pv->state.IMPULSE_COUNT = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+IMPULSE_COUNT=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_IMPULSE_PERIOD:
			pv->state.IMPULSE_PERIOD = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+IMPULSE_PERIOD=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_MCDELAY:
			pv->state.MCDELAY = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+MCDELAY=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_NM_VALVE_DELAY:
			pv->state.NM_VALVE_DELAY = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+NM_VALVE_DELAY=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_FUNC_VALE_TIMEOUT:
			pv->state.FUNC_VALVE_TIMEOUT = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+FUNC_VALVE_TIMEOUT=%u", cci_u16_data(info));
			break;
		case ACT_YUYUAN_LOOP_ONOFF:		
			pv->state.LOOP_ONOFF = cci_u16_data(info);
			yuyuan_send_cmd(session, "AT+LOOP_ONOFF=%u", cci_u16_data(info));
			break;


		case ACT_YUYUAN_SPEED:
			if (var_param_len < 3) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}

			pv->state.SPEED1 = var_param[0];
			pv->state.SPEED2 = var_param[1];
			pv->state.SPEED3 = var_param[2];
			yuyuan_send_cmd(session, "AT+SPEED&SPEED1=%u,SPEED2=%u,SPEED3=%u", 
				var_param[0], var_param[1], var_param[2]);
			break;

		case ACT_YUYUAN_LOOP:
			if (var_param_len < 4) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}

			pv->state.LOOPD = var_param[0];
			pv->state.LOOPH = var_param[1];
			pv->state.LOOPM = var_param[2];
			pv->state.LOOPT = var_param[3];
			yuyuan_send_cmd(session, "AT+LOOP&LOOPD=%u,LOOPH=%u,LOOPM=%u,LOOPT=%u", 
				var_param[0], var_param[1], var_param[2], var_param[3]);
			break;

		case ACT_YUYUAN_MCCLEAN:
			if (var_param_len < 5) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}

			pv->state.MCCLEAN_D = var_param[0];
			pv->state.MCCLEAN_H = var_param[1];
			pv->state.MCCLEAN_M = var_param[2];
			pv->state.MCCLEAN_RT = var_param[3];
			pv->state.MCCLEAN_DT = var_param[4];
			yuyuan_send_cmd(session, "AT+MCCLEAN&MCCLEAN_D=%u,MCCLEAN_H=%u,MCCLEAN_M=%u,MCCLEAN_RT=%u,MCCLEAN_DT=%u", 
				var_param[0], var_param[1], var_param[2], var_param[3], var_param[4]);
			break;

		case ACT_YUYUAN_NMCLEAN:
			if (var_param_len < 5) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}

			pv->state.NMCLEAN_D = var_param[0];
			pv->state.NMCLEAN_H = var_param[1];
			pv->state.NMCLEAN_M = var_param[2];
			pv->state.NMCLEAN_RT = var_param[3];
			pv->state.NMCLEAN_DT = var_param[4];
			yuyuan_send_cmd(session, "AT+NMCLEAN&NMCLEAN_D=%u,NMCLEAN_H=%u,NMCLEAN_M=%u,NMCLEAN_RT=%u,NMCLEAN_DT=%u", 
				var_param[0], var_param[1], var_param[2], var_param[3], var_param[4]);
			break;

		case ACT_YUYUAN_BEGIN_MICRO_CLEAN:		
			u16value = cci_u16_data(info);
			if (u16value == 0) {
				yuyuan_send_cmd(session, "AT+BEGIN_MICRO_CLEAN");
			} else {
				yuyuan_send_cmd(session, "AT+STOP_MICRO_CLEAN");
			}
			break;
		case ACT_YUYUAN_NM_CLEAN:	
			u16value = cci_u16_data(info);
			if (u16value == 0) {
				yuyuan_send_cmd(session, "AT+BEGIN_NM_CLEAN");
			} else {
				yuyuan_send_cmd(session, "AT+STOP_NM_CLEAN");
			}
			break;
		case ACT_YUYUAN_CHECK_SELF:			
			yuyuan_send_cmd(session, "AT+CHECK_SELF");
			break;
		case ACT_YUYUAN_REBOOT_CLEANER:			
			yuyuan_send_cmd(session, "AT+REBOOT_CLEANER");
			break;

		case ACT_YUYUAN_CONFIG:
			if (var_param_len > 50) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}
			
			yuyuan_send_cmd(session, "AT+CONFIG=%s", var_param);
			break;

		case ACT_YUYUAN_PWD:
			yuyuan_do_set_pwd(session, var_param, var_param_len);
			break;
		case ACT_YUYUAN_REMIND:
			yuyuan_do_set_remind(session, var_param, var_param_len);
			break;
			
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	return res;
}


////////////////////////////////////////////////////////////////////

//��Դ��ˮ��SDK���ýӿ�ʵ��

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ˮ���С0-��ˮ�䣬1-Сˮ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_water_box(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_WATER_BOX, value);
}
/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ��ˮ��ʱ����λS����Χ0-100
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_inlet_timeout(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_INLET_TIMEOUT, value);
}
/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: �������������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_impulse_count(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_IMPULSE_COUNT, value);
}

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ��������������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_impulse_period(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_IMPULSE_PERIOD, value);
}

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ΢����λ��ʱ����λS��Ĭ��90����Χ0-300
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_mcdelay(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_MCDELAY, value);
}

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ���׷���λ��ʱ����λS��Ĭ��90����Χ0-300
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_nm_valve_delay(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_NM_VALVE_DELAY, value);
}

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: 
	�������:���ܷ���λ��ʱʱ�䣬Ĭ��120����Χ0-300
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_func_valve_timeout(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_FUNC_VALE_TIMEOUT, value);
}

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@speed1-3: 3��������ת�٣���Χ0-100
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_speed(cl_handle_t dev_handle, u_int8_t speed1, u_int8_t speed2, u_int8_t speed3)
{
	u_int8_t buf[3];
	
	CL_CHECK_INIT;

	buf[0] = speed1;
	buf[1] = speed2;
	buf[2] = speed3;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_SPEED, buf, sizeof(buf));
}


/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ѭ�����أ�1-����0-��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_loop_onoff(cl_handle_t dev_handle,u_int16_t value)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_LOOP_ONOFF, value);
}

/*
	����:ѭ��ʱ��, �ڴﵽѭ������LPD��LPHʱLPM��ִ��LPT����ѭ��
		
	�������:
		@dev_handle: �豸�ľ��
		@day: ѭ����
		@hour: ѭ��Сʱ
		@min:ѭ������
		@hold_min: ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_loop(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t hold_min)
{
	u_int8_t buf[4];
	
	CL_CHECK_INIT;

	buf[0] = day;
	buf[1] = hour;
	buf[2] = min;
	buf[3] = hold_min;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_LOOP, buf, sizeof(buf));
}

/*
	����:΢�����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
		
	�������:
		@dev_handle: �豸�ľ��
		@day: ѭ����
		@hour: ѭ��Сʱ
		@rt_min:����ʱ��
		@dt_min: ֱϴʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_mcclean(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t rt_min, u_int8_t dt_min)
{
	u_int8_t buf[5];
	
	CL_CHECK_INIT;

	buf[0] = day;
	buf[1] = hour;
	buf[2] = min;
	buf[3] = rt_min;
	buf[4] = dt_min;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_MCCLEAN, buf, sizeof(buf));
}

/*
	����:�����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
		
	�������:
		@dev_handle: �豸�ľ��
		@day: ѭ����
		@hour: ѭ��Сʱ
		@min: ѭ������
		@rt_min:����ʱ��
		@dt_min: ֱϴʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_nmclean(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t rt_min, u_int8_t dt_min)
{
	u_int8_t buf[5];
	
	CL_CHECK_INIT;

	buf[0] = day;
	buf[1] = hour;
	buf[2] = min;
	buf[3] = rt_min;
	buf[4] = dt_min;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_NMCLEAN, buf, sizeof(buf));
}


/*
	����:������ʼ΢������ϴ
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_mincro_clean(cl_handle_t dev_handle, u_int8_t action)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_BEGIN_MICRO_CLEAN, action);
}

/*
	����:������ʼ������ϴ
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_nm_clean(cl_handle_t dev_handle, u_int8_t action)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_NM_CLEAN, action);
}

/*
	����:������ʼ�Լ졣�Լ������ģ����ȡ��ERROR�����ϱ���APP��
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_check_self(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_CHECK_SELF, 0);
}

/*
	����:����������ˮ��
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_reboot_cleanner(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_REBOOT_CLEANER, 0);
}


/*
	����:����ģʽ���APP����һ������ģʽ�����������ڵ������滻��VALUE�����͸���λ�����������䡣VALUE�ĳ�������Ϊ50�ֽ��ڡ�
		
	�������:
		@dev_handle: �豸�ľ��
		@cmd: �Զ���AT����
		@cmd_len: �����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_config(cl_handle_t dev_handle, u_int8_t *cmd, u_int8_t cmd_len)
{
	u_int8_t buf[50] = {0};
	
	CL_CHECK_INIT;

	cmd_len = min(sizeof(buf), cmd_len);
	memcpy(buf, cmd, cmd_len);

	buf[sizeof(buf) - 1] = 0;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_CONFIG, buf, sizeof(buf));
}


/*
	����:����һ�����룬���볤��С��50�ֽ�
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ����
		@value_len: ���볤��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_set_pwd(cl_handle_t dev_handle, u_int8_t *value, u_int8_t value_len)
{
	cl_yuyuan_pwd_t info;
	
	CL_CHECK_INIT;

	info.pwd_len = min(sizeof(info.pwd), value_len);
	memcpy(info.pwd, value, value_len);
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_PWD, (u_int8_t*)&info, sizeof(info));
}

/*
	����:����������Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@onoff: 0 �ر����� 1 ��������
		@value_len: ���볤��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_set_remind(cl_handle_t dev_handle, u_int32_t onoff, u_int32_t remind_time)
{
	cl_yuyuan_remind_t info;
	
	CL_CHECK_INIT;

	info.onoff = onoff;
	info.remind_time = remind_time;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EVM_DEVICE, ACT_YUYUAN_REMIND, (u_int8_t*)&info, sizeof(info));
}


