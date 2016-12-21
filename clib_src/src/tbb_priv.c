/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: tbb_priv.c
**  File:    tbb_priv.c
**  Author:  liubenlong
**  Date:    08/10/2015
**
**  Purpose:
**    拓邦商用.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_tbb.h"
#include "tbb_priv.h"
#include "udp_device_common_priv.h"
#include "uc_client.h"
/* Macro constant definitions. */


/* Type definitions. */
//模式
enum {
	TBB_MODE_AUTO = 0,//自动
	TBB_MODE_COOL, //制冷
	TBB_MODE_HEAT,//制热
	TBB_MODE_HOT,//供暖
};
/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
static void eev_order(cl_tbb_eev_t *pnet);
static void misc_order(cl_tbb_misc_t *pnet);
static void protect_order(cl_tbb_protect_t *pnet);
static void bind_ordor(cl_tbb_bindinfo_t *pnet);
static void alltmp_order(cl_tbb_tmp_t *pnet);
static void defrost_order(cl_tbb_defrost_t *pnet);
extern RS sa_set_obj_value_only(ucc_session_t *s,u_int8_t obj_count,void* content,int content_len);

static int _tbb_ctrl_onoff(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t onoff, cl_tbb_info_t *ptbb)
{
	int len = 0;
	ucp_tbb_on_t *pttmp;

	if (onoff == ptbb->on) {
		log_debug("ther on is same=%u\n", onoff);
		return len;
	}

	ptbb->on = onoff;
	
	pttmp = (ucp_tbb_on_t *)pdata;
	pttmp->on = ptbb->on;
	len = sizeof(ucp_tbb_on_t);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_ONOFF,len);

	return len;
}

static bool _tbb_mode_sync_tmp(cl_tbb_info_t *ptbb)
{
	bool ret = false;
	
	switch(ptbb->mode) {
	case TBB_MODE_AUTO:
		if (ptbb->tmp != ptbb->config.auto_tmp.auto_tmp) {
			ptbb->tmp = ptbb->config.auto_tmp.auto_tmp;
			ret = true;
		}
		break;
	case TBB_MODE_COOL:
		if (ptbb->tmp != ptbb->config.auto_tmp.cool_tmp) {
			ptbb->tmp = ptbb->config.auto_tmp.cool_tmp;
			ret = true;
		}
		break;
	case TBB_MODE_HEAT:
		if (ptbb->tmp != ptbb->config.auto_tmp.heat_tmp) {
			ptbb->tmp = ptbb->config.auto_tmp.heat_tmp;
			ret = true;
		}
		break;
	case TBB_MODE_HOT:
		break;
	default:
		break;
	}

	return ret;	
}

static int _tbb_ctrl_mode(ucc_session_t *s, u_int8_t mode, cl_tbb_info_t *ptbb)
{
	int len = 0;
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_tbb_tmp_t *palltmp = (cl_tbb_tmp_t *)(&uo[1]);
	ucp_tbb_mode_t *pmode = (ucp_tbb_mode_t *)(&uo[1]);
	
	if (mode == ptbb->mode) {
		log_debug("ther on is same=%u\n", mode);
		return 0;
	}

	ptbb->mode = mode;
	
	pmode->mode = ptbb->mode;
	len = sizeof(ucp_tbb_mode_t);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_MODE,len);
	sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);

	memcpy((void *)palltmp, (void *)&ptbb->config.auto_tmp, sizeof(*palltmp));
	len = sizeof(*palltmp);
	alltmp_order(palltmp);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_ALLTMP,len);
	sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
	s->user->last_ctrl = true;

	return 0;
}

static int _tbb_ctrl_tmp(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int16_t tmp, cl_tbb_info_t *ptbb)
{
	int len = 0;
	ucp_tbb_tmp_t *pttmp;
	
	if (tmp == ptbb->tmp) {
		log_debug("ther on is same=%u\n", tmp);
		return len;
	}

	ptbb->tmp = tmp;
	
	pttmp = (ucp_tbb_tmp_t *)pdata;
	pttmp->tmp = htons(ptbb->tmp);
	len = sizeof(ucp_tbb_tmp_t);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_TEMP,len);

	return len;
}

static int _tbb_ctrl_bind(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t *ptmp, cl_tbb_info_t *ptbb)
{
	int len = 0;
	ucp_tb_bind_info *bi;

	len = sizeof(ucp_tb_bind_info);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_BINDSN,len);
	bi = (ucp_tb_bind_info *)pdata;
	memcpy(bi->tb_sn,ptmp,17);

	return len;
}

static bool _tbb_ctrl_conf_tmp_sync(cl_tbb_info_t *ptbb, cl_tbb_tmp_t *pauto)
{
	bool ret = false;
	
	switch(ptbb->mode) {
	case TBB_MODE_AUTO:
		if (ptbb->tmp != pauto->auto_tmp) {
			ptbb->tmp = pauto->auto_tmp;
			ret = true;
		}
		break;
	case TBB_MODE_COOL:
		if (ptbb->tmp != pauto->cool_tmp) {
			ptbb->tmp = pauto->cool_tmp;
			ret = true;
		}
		break;
	case TBB_MODE_HEAT:
		if (ptbb->tmp != pauto->heat_tmp) {
			ptbb->tmp = pauto->heat_tmp;
			ret = true;
		}
		break;
	case TBB_MODE_HOT:
		break;
	default:
		break;
	}

	return ret;
}

static int _tbb_ctrl_config(ucc_session_t *s, u_int8_t *ptmp, cl_tbb_info_t *ptbb)
{
	int len = 0;
	char buf[1024] = {0};
	cl_tbb_config_set_t *plocal = &ptbb->config;
	cl_tbb_config_set_t *pnet = (cl_tbb_config_set_t *)ptmp;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_tbb_defrost_t *pdefrost = (cl_tbb_defrost_t *)(&uo[1]);
	cl_tbb_misc_t *pmisc = (cl_tbb_misc_t *)(&uo[1]);
	cl_tbb_protect_t *pprotect = (cl_tbb_protect_t *)(&uo[1]);
	cl_tbb_eev_t *peev = (cl_tbb_eev_t *)(&uo[1]);
	ucp_tbb_bert_t *pbert = (ucp_tbb_bert_t *)(&uo[1]);
	cl_tbb_tmp_t *palltmp = (cl_tbb_tmp_t *)(&uo[1]);
	ucp_tbb_tmp_t *pttmp = (ucp_tbb_tmp_t *)(&uo[1]);
	bool is_reset = false;

	if (memcmp((void *)&plocal->misc, (void *)&pnet->misc, sizeof(plocal->misc))) {
		// TODO:这里要判断下，如果是回复出厂或者切换温度单位，就不要发下面的命令了
		//bit 10温度单位转换，bit 15恢复出厂
		if (((BIT(10)&plocal->misc.work) != (BIT(10)&pnet->misc.work)) ||
			((BIT(15)&plocal->misc.work) != (BIT(15)&pnet->misc.work))) {
			// TODO: 这里判断下就只发misc命令
			is_reset = true;
			memcpy((void *)plocal, (void *)pnet, sizeof(*plocal));
		}
		
		memcpy((void *)&plocal->misc, (void *)&pnet->misc, sizeof(plocal->misc));
		memcpy((void *)pmisc, (void *)&pnet->misc, sizeof(*pmisc));
		misc_order(pmisc);
		len = sizeof(cl_tbb_misc_t);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_MISC,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
		if (is_reset) {
			return 0;
		}
	}

	if (memcmp((void *)&plocal->defrost, (void *)&pnet->defrost, sizeof(plocal->defrost))) {
		memcpy((void *)&plocal->defrost, (void *)&pnet->defrost, sizeof(plocal->defrost));
		memcpy((void *)pdefrost, (void *)&pnet->defrost, sizeof(*pdefrost));
		len = sizeof(cl_tbb_defrost_t);
		defrost_order(pdefrost);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_DEFROST,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
	}

	if (memcmp((void *)&plocal->protect, (void *)&pnet->protect, sizeof(plocal->protect))) {
		memcpy((void *)&plocal->protect, (void *)&pnet->protect, sizeof(plocal->protect));
		memcpy((void *)pprotect, (void *)&pnet->protect, sizeof(*pprotect));
		protect_order(pprotect);
		len = sizeof(cl_tbb_protect_t);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_PROTECT,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
	}	

	if (memcmp((void *)&plocal->eev, (void *)&pnet->eev, sizeof(plocal->eev))) {
		memcpy((void *)&plocal->eev, (void *)&pnet->eev, sizeof(plocal->eev));
		memcpy((void *)peev, (void *)&pnet->eev, sizeof(*peev));
		eev_order(peev);
		len = sizeof(cl_tbb_eev_t);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_EEV,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
	}

	if (plocal->bottom_ele_heat_tmp != pnet->bottom_ele_heat_tmp) {
		plocal->bottom_ele_heat_tmp = pnet->bottom_ele_heat_tmp;
		pbert->tmp = htons(pnet->bottom_ele_heat_tmp);
		len = sizeof(*pbert);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_BERT,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
	}

	if (memcmp((void *)&plocal->auto_tmp, (void *)&pnet->auto_tmp, sizeof(plocal->auto_tmp))) {
		memcpy((void *)&plocal->auto_tmp, (void *)&pnet->auto_tmp, sizeof(plocal->auto_tmp));
		if (_tbb_ctrl_conf_tmp_sync(ptbb, &pnet->auto_tmp)) {
			pttmp->tmp = htons(ptbb->tmp);
			len = sizeof(ucp_tbb_tmp_t);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_TEMP,len);
			sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
			s->user->last_ctrl = true;
		}
		//
		memcpy((void *)palltmp, (void *)&pnet->auto_tmp, sizeof(*palltmp));
		len = sizeof(*palltmp);
		alltmp_order(palltmp);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_IA_TB_ALLTMP,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
	}
	
	return 0;
}

bool tbb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_tbb_info_t* ptbb;
	char buf[1024] = {0};
	u_int8_t *ptmp = NULL;
	u_int8_t tmp = 0;
	u_int16_t tmp16 = 0;
	ucc_session_t *s = NULL;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pdata = (u_int8_t *)(&uo[1]);	
	u_int32_t max_data_len = sizeof(buf) - sizeof(ucp_obj_t);

	int len = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ptmp = (u_int8_t *)info->u.u8_data;
	
	if(!user->smart_appliance_ctrl) {
		log_err(false, "%s error handle %08x\n", __FUNCTION__, info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "%s error handle %08x\n", __FUNCTION__, info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	s = ac->sac->user->uc_session;
	ptbb = ac->com_udp_dev_info.device_info;

	tmp = cci_u8_data(info);
	ptmp = cci_pointer_data(info);
	tmp16 = cci_u16_data(info);
	switch(info->action){
	case ACT_TBB_ON:
		len = _tbb_ctrl_onoff(uo, pdata, max_data_len, tmp, ptbb);
		break;
	case ACT_TBB_MODE:
		len = _tbb_ctrl_mode(s, tmp, ptbb);
		break;
	case ACT_TBB_TMP:
		len = _tbb_ctrl_tmp(uo, pdata, max_data_len, tmp16, ptbb);
		break;
	case ACT_TBB_CONFIG:
		len = _tbb_ctrl_config(s, ptmp, ptbb);
		break;
	case ACT_TBB_BIND:
		len = _tbb_ctrl_bind(uo, pdata, max_data_len, ptmp, ptbb);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;		
	}

	//控制后立马更新数据
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    event_cancel_merge(user->handle);

	if(len > 0){
		sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
		s->user->last_ctrl = true;
	}
		
	return res;
}

static void status_order(cl_tbb_status_t *pnet)
{
	int i;
	
	pnet->water_box_tmp = htons(pnet->water_box_tmp);
	pnet->in_water_tmp = htons(pnet->in_water_tmp);
	pnet->out_water_tmp = htons(pnet->out_water_tmp);
	pnet->env_tmp = htons(pnet->env_tmp);
	pnet->back_water_tmp = htons(pnet->back_water_tmp);
	pnet->support_mode = htons(pnet->support_mode);

	for(i = 0; i < 4; i++) {
		pnet->scroll_tmp[i].scroll_tmp = htons(pnet->scroll_tmp[i].scroll_tmp);
		pnet->scroll_tmp[i].out_air_tmp = htons(pnet->scroll_tmp[i].out_air_tmp);
		pnet->scroll_tmp[i].back_air_tmp = htons(pnet->scroll_tmp[i].back_air_tmp);
		pnet->ele_valve[i] = htons(pnet->ele_valve[i]);
	}

	pnet->pump_info = htons(pnet->pump_info);
	pnet->set_on = htons(pnet->set_on);
	pnet->set_on1 = htons(pnet->set_on1);
	pnet->set_on2 = htons(pnet->set_on2);
	pnet->slave_status = htons(pnet->slave_status);
	pnet->fault1 = htons(pnet->fault1);
	pnet->fault2 = htons(pnet->fault2);
	pnet->fault3 = htons(pnet->fault3);
	pnet->sys_run_days = htons(pnet->sys_run_days);
}

static bool _tbb_update_status(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_status_t *plocal = &info->status;
	cl_tbb_status_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_status_t *)(obj+1);
	status_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static bool _tbb_update_temp(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	ucp_tbb_tmp_t *ptmp = NULL;

	if (obj->param_len < sizeof(*ptmp)) {
		log_debug("obj->param_len=%u sizeof(*pstatus=%u\n", obj->param_len, sizeof(*ptmp));
		return ret;
	}

	ptmp = (ucp_tbb_tmp_t *)(obj+1);
	ptmp->tmp = htons(ptmp->tmp);
	if (ptmp->tmp != info->tmp) {
		ret = true;
		info->tmp = ptmp->tmp;
	}

	return ret;
}

static bool _tbb_update_mode(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	ucp_tbb_mode_t *pmode = NULL;

	if (obj->param_len < sizeof(*pmode)) {
		log_debug("obj->param_len=%u sizeof(*pstatus=%u\n", obj->param_len, sizeof(*pmode));
		return ret;
	}	

	pmode = (ucp_tbb_mode_t *)(obj+1);
	if (info->mode != pmode->mode) {
		ret = true;
		info->mode = pmode->mode;
	}

	return ret;
}

static bool _tbb_update_onoff(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	ucp_tbb_on_t *ptmp = NULL;

	if (obj->param_len < sizeof(*ptmp)) {
		log_debug("obj->param_len=%u sizeof(*ptmp)=%u\n", obj->param_len ,sizeof(*ptmp));
		return ret;
	}	

	ptmp = (ucp_tbb_on_t *)(obj+1);
	if (info->on != ptmp->on) {
		ret = true;
		info->on = ptmp->on;
	}

	return ret;
}

static void defrost_order(cl_tbb_defrost_t *pnet)
{
	pnet->in_defrost_time = htons(pnet->in_defrost_time);
	pnet->in_defrost_tmp = htons(pnet->in_defrost_tmp);
	pnet->out_defrost_tmp = htons(pnet->out_defrost_tmp);
	pnet->out_defrost_time = htons(pnet->out_defrost_time);
	pnet->env_tmp = htons(pnet->env_tmp);
	pnet->dif = htons(pnet->dif);
	pnet->defrost_continue_time = htons(pnet->defrost_continue_time);
}

static bool _tbb_update_defrost(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_config_set_t *config = &info->config;
	cl_tbb_defrost_t *plocal = &config->defrost;
	cl_tbb_defrost_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_defrost_t *)(obj+1);
	defrost_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static void misc_order(cl_tbb_misc_t *pnet)
{
	pnet->work = htons(pnet->work);
	pnet->heat_pump_diff = htons(pnet->heat_pump_diff);
	pnet->cool_diff = htons(pnet->cool_diff);
	pnet->hot_diff = htons(pnet->hot_diff);
}

static bool _tbb_update_misc(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_config_set_t *config = &info->config;
	cl_tbb_misc_t *plocal = &config->misc;
	cl_tbb_misc_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_misc_t *)(obj+1);
	misc_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static void protect_order(cl_tbb_protect_t *pnet)
{
	pnet->dst_tmp_pro = htons(pnet->dst_tmp_pro);
	pnet->cool_out_water = htons(pnet->cool_out_water);
	pnet->heat_out_water = htons(pnet->heat_out_water);
	pnet->in_out_tmp = htons(pnet->in_out_tmp);
	pnet->pump_begin_time = htons(pnet->pump_begin_time);
	pnet->pump_delay_time = htons(pnet->pump_delay_time);
	pnet->wind_ordor_tmp = htons(pnet->wind_ordor_tmp);
	pnet->env_tmp = htons(pnet->env_tmp);
	pnet->in_water_tmp = htons(pnet->in_water_tmp);
}

static bool _tbb_update_protect(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_config_set_t *config = &info->config;
	cl_tbb_protect_t *plocal = &config->protect;
	cl_tbb_protect_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_protect_t *)(obj+1);
	protect_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static bool _tbb_update_bert(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_config_set_t *config = &info->config;
	ucp_tbb_bert_t *pbert = NULL;
	
	if (obj->param_len < sizeof(*pbert)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pbert));
		return ret;
	}	

	pbert = (ucp_tbb_bert_t *)(obj+1);
	pbert->tmp = htons(pbert->tmp);
	if (config->bottom_ele_heat_tmp != pbert->tmp) {
		ret = true;
		config->bottom_ele_heat_tmp = pbert->tmp;
	}

	return ret;
}

static void upgrade_order(cl_stm_upgrade_info_t *pnet)
{
	pnet->upgradeing = htonl(pnet->upgradeing);
	pnet->upgrade_role = htonl(pnet->upgrade_role);
	pnet->upgrade_state = htonl(pnet->upgrade_state);
	pnet->up_state = htonl(pnet->up_state);
}

static bool _tbb_update_upgrade_info(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_stm_upgrade_info_t *plocal = &info->upgrade_info;
	cl_stm_upgrade_info_t *pnet = NULL;
	
	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}

	pnet = (cl_stm_upgrade_info_t *)(obj+1);
	upgrade_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static void eev_order(cl_tbb_eev_t *pnet)
{
	pnet->ele_cycle = htons(pnet->ele_cycle);
	pnet->hand_ctrl_step = htons(pnet->hand_ctrl_step);
	pnet->cool_ele_valve = htons(pnet->cool_ele_valve);
	pnet->limit_day = htons(pnet->limit_day);
}

static bool _tbb_update_eev(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_config_set_t *config = &info->config;
	cl_tbb_eev_t *plocal = &config->eev;
	cl_tbb_eev_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_eev_t *)(obj+1);
	eev_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static void ver_ordor(cl_tbb_hd_ver_t *pver)
{
	pver->sys_type = htons(pver->sys_type);
	pver->ele_band_mcu = htons(pver->ele_band_mcu);
	pver->ele_band_ver = htons(pver->ele_band_ver);
	pver->line_band_mcu = htons(pver->line_band_mcu);
	pver->line_band_ver = htons(pver->line_band_ver);
}

static bool _tbb_update_ver(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_hd_ver_t *plocal = &info->hd_ver;
	cl_tbb_hd_ver_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_hd_ver_t *)(obj+1);
	ver_ordor(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static void alltmp_order(cl_tbb_tmp_t *pnet)
{
	pnet->cool_tmp = htons(pnet->cool_tmp);
	pnet->heat_tmp = htons(pnet->heat_tmp);
	pnet->auto_tmp = htons(pnet->auto_tmp);
}

static bool _tbb_update_alltmp(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_tmp_t *plocal = &info->config.auto_tmp;
	cl_tbb_tmp_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	log_debug("_tbb_update_alltmp !!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	pnet = (cl_tbb_tmp_t *)(obj+1);
	alltmp_order(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static void bind_ordor(cl_tbb_bindinfo_t *pnet)
{
	pnet->bind_state = htons(pnet->bind_state);
	pnet->dev_state = htons(pnet->dev_state);
}

static bool _tbb_update_bind(smart_air_ctrl_t* air_ctrl, cl_tbb_info_t *info, ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_bindinfo_t *plocal = &info->bindinfo;
	cl_tbb_bindinfo_t *pnet = NULL;

	if (obj->param_len < sizeof(*pnet)) {
		log_debug("obj->param_len=%u sizeof(*pnet)=%u\n", obj->param_len ,sizeof(*pnet));
		return ret;
	}	

	pnet = (cl_tbb_bindinfo_t *)(obj+1);
	bind_ordor(pnet);
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*pnet))) {
		ret = true;
		memcpy((void *)plocal, (void *)pnet, sizeof(*pnet));
	}

	return ret;
}

static RS attr_single_object(ucc_session_t *s,u_int16_t obj,u_int16_t sub_obj, u_int16_t attr)
{
	ucp_obj_t stat_objs[1] = {0};

	stat_objs[0].objct = obj;
	stat_objs[0].sub_objct = sub_obj;
	stat_objs[0].attr = attr;
	
	return sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

bool _tbb_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	cl_tbb_info_t* info = air_ctrl->com_udp_dev_info.device_info; 
//	cl_tbb_status_t *pstatus = NULL;
	static bool tmp_all_query = false;
		
	if( !info || !obj ){
		return false;
	}

	switch(obj->attr){
	case UCAT_IA_TB_STATUS:
		ret = _tbb_update_status(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_TEMP:
		ret = _tbb_update_temp(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_MODE:
		ret = _tbb_update_mode(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_ONOFF:
		ret = _tbb_update_onoff(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_DEFROST:
		ret = _tbb_update_defrost(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_MISC:
		ret = _tbb_update_misc(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_PROTECT:
		ret = _tbb_update_protect(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_BERT:
		ret = _tbb_update_bert(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_UPGRADE_INFO:
		ret = _tbb_update_upgrade_info(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_EEV:
		ret = _tbb_update_eev(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_VER:
		ret = _tbb_update_ver(air_ctrl, info, obj);
		break;
	case UCAT_IA_TB_ALLTMP:
		ret = _tbb_update_alltmp(air_ctrl, info, obj);
		break;
	case UCAT_TBHEATER_BINDSN:
		ret = _tbb_update_bind(air_ctrl, info, obj);
		break;
	default:
		break;
	}

	if (!tmp_all_query) {
		//只需要查询一次
		attr_single_object(air_ctrl->sac->user->uc_session,UCOT_IA,UCSOT_IA_TBHEATER,UCAT_IA_TB_ALLTMP);
		tmp_all_query = true;
	}
	
	return true;
}



