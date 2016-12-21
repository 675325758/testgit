/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: zkrsq_priv.c
**  File:    zkrsq_priv.c
**  Author:  liubenlong
**  Date:    11/16/2015
**
**  Purpose:
**    E:\低功率wifi\new-win-sdk\inc.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "udp_device_common_priv.h"
#include "cl_zkrsq.h"
#include "zkrsq_priv.h"

/* Macro constant definitions. */


/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
bool _zkrsq_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_zkrsq_info_t* info = air_ctrl->com_udp_dev_info.device_info; 
	ucp_zkrsq_on_t *pon = NULL;
	ucp_zkrsq_mode_t *pmode = NULL;
	ucp_zkrsq_tmp_t *ptmp = NULL;
	ucp_zkrsq_time_t *ptime = NULL;
	ucp_zkrsq_timer_t *ptimer = NULL;
	ucp_zkrsq_fault_t *pfault = NULL;
	ucp_zkrsq_reset_t *preset = NULL;
		
	if( !info || !obj ){
		return false;
	}

	switch(obj->attr){
	case UCAT_IA_ZKRSQ_ONOFF:
		if (obj->param_len < sizeof(ucp_zkrsq_on_t)) {
			return false;
		}
		pon = (ucp_zkrsq_on_t *)(&obj[1]);
		info->onoff = pon->onoff;
		break;
	case UCAT_IA_ZKRSQ_BACKWATERONOFF:
		if (obj->param_len < sizeof(ucp_zkrsq_on_t)) {
			return false;
		}
		pon = (ucp_zkrsq_on_t *)(&obj[1]);
		info->back_water_onoff = pon->onoff;
		break;
	case UCAT_IA_ZKRSQ_HOTWATER_TEMPERATURE:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->hot_water_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_DEV_TEMPERATURE:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->conf.back_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_COMPENSATE_TEMPERATURE:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->conf.compensation_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_DEFROST_PERIOD:
		if (obj->param_len < sizeof(ucp_zkrsq_time_t)) {
			return false;
		}
		ptime = (ucp_zkrsq_time_t *)(&obj[1]);
		info->conf.defrost_time = ptime->time;
		break;
	case UCAT_IA_ZKRSQ_DEFROST_IN_TEMPERATURE:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->conf.defrost_in_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_DEFROST_PERSISTENCE:
		if (obj->param_len < sizeof(ucp_zkrsq_time_t)) {
			return false;
		}
		ptime = (ucp_zkrsq_time_t *)(&obj[1]);
		info->conf.defrost_continue_time = ptime->time;
		break;
	case UCAT_IA_ZKRSQ_DEFROST_OUT_TEMPERATURE:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->conf.defrost_out_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_BACKWATER_TEMPERATURE:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->conf.back_water_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_BACKWATER_MODE:
		if (obj->param_len < sizeof(ucp_zkrsq_mode_t)) {
			return false;
		}
		pmode = (ucp_zkrsq_mode_t *)(&obj[1]);
		info->conf.back_water_mode = pmode->mode;
		break;
	case UCAT_IA_ZKRSQ_TIMER1_STATE:
		if (obj->param_len < sizeof(ucp_zkrsq_timer_t)) {
			return false;
		}
		ptimer = (ucp_zkrsq_timer_t *)(&obj[1]);
		info->timer.timer1_valid = 1;
		info->timer.timer1_onoff = ptimer->onoff;
		info->timer.timer1_hour = ptimer->hour;
		info->timer.timer1_min = ptimer->min;
		info->timer.timer1_hour_end = ptimer->hour_end;
		info->timer.timer1_min_end = ptimer->min_end;
		break;
	case UCAT_IA_ZKRSQ_TIMER2_STATE:
		if (obj->param_len < sizeof(ucp_zkrsq_timer_t)) {
			return false;
		}
		ptimer = (ucp_zkrsq_timer_t *)(&obj[1]);
		info->timer.timer2_valid = 1;
		info->timer.timer2_onoff = ptimer->onoff;
		info->timer.timer2_hour = ptimer->hour;
		info->timer.timer2_min = ptimer->min;
		info->timer.timer2_hour_end = ptimer->hour_end;
		info->timer.timer2_min_end = ptimer->min_end;
		break;
	case UCAT_IA_ZKRSQ_TIMER3_STATE:
		if (obj->param_len < sizeof(ucp_zkrsq_timer_t)) {
			return false;
		}
		ptimer = (ucp_zkrsq_timer_t *)(&obj[1]);
		info->timer.timer3_valid = 1;
		info->timer.timer3_onoff = ptimer->onoff;
		info->timer.timer3_hour = ptimer->hour;
		info->timer.timer3_min = ptimer->min;
		info->timer.timer3_hour_end = ptimer->hour_end;
		info->timer.timer3_min_end = ptimer->min_end;
		break;
	case UCAT_IA_ZKRSQ_FAULT_STATE:
		if (obj->param_len < sizeof(ucp_zkrsq_fault_t)) {
			return false;
		}
		pfault = (ucp_zkrsq_fault_t *)(&obj[1]);
		info->fault = pfault->fault;
		break;
	case UCAT_IA_ZKRSQ_RECOVER_DEFAULT:
		if (obj->param_len < sizeof(ucp_zkrsq_reset_t)) {
			return false;
		}
		preset = (ucp_zkrsq_reset_t *)(&obj[1]);
		info->reset_status = preset->reset;
		break;
	case UCAT_IA_ZKRSQ_HOTWATER_TEMPERATURE_CURRENT:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->hot_water_tmp_cur = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_COILPIPE_TEMPERATURE_CURRENT:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->coilpipe_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_SURROUNDINGS_TEMPERATURE_CURRENT:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->room_tmp = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_BACKWATER_TEMPERATURE_CURRENT:
		if (obj->param_len < sizeof(ucp_zkrsq_tmp_t)) {
			return false;
		}
		ptmp = (ucp_zkrsq_tmp_t *)(&obj[1]);
		info->back_water_tmp_cur = (int8_t)ptmp->tmp;
		break;
	case UCAT_IA_ZKRSQ_WORK_MODE:
		if (obj->param_len < sizeof(ucp_zkrsq_mode_t)) {
			return false;
		}
		pmode = (ucp_zkrsq_mode_t *)(&obj[1]);
		info->mode = pmode->mode;
		break;
	default:
		break;
	}
	
	return true;
}


static bool _zkrsq_ctrl_proc(user_t *user, u_int8_t action, u_int8_t value, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_zkrsq_info_t* pinfo = (cl_zkrsq_info_t *)ac->com_udp_dev_info.device_info;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_zkrsq_on_t *pon = (ucp_zkrsq_on_t *)(&uo[1]);
	ucp_zkrsq_mode_t *pmode = (ucp_zkrsq_mode_t *)(&uo[1]);
	ucp_zkrsq_tmp_t *ptmp = (ucp_zkrsq_tmp_t *)(&uo[1]);

	if (!ac || !pinfo) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));
	switch(action) {
	case ZKRSQ_ACTION_ONOFF:
		len = sizeof(ucp_zkrsq_on_t);
		pon->onoff = value;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_ONOFF,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
		break;
	case ZKRSQ_ACTION_MODE:
		len = sizeof(ucp_zkrsq_mode_t);
		pmode->mode = value;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_WORK_MODE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
		break;
	case ZKRSQ_ACTION_TMP:
		len = sizeof(ucp_zkrsq_tmp_t);
		ptmp->tmp = value;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_HOTWATER_TEMPERATURE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		return false;
	}

	*ret = RS_OK;

	return true;
}

static bool _zkrsq_config_proc(user_t *user, u_int8_t *pvalue, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_zkrsq_info_t* pinfo = (cl_zkrsq_info_t *)ac->com_udp_dev_info.device_info;
	cl_zk_confset_t *pconf_src = (cl_zk_confset_t *)pvalue;
	cl_zk_confset_t *pconf_local = &pinfo->conf;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_zkrsq_tmp_t *ptmp = (ucp_zkrsq_tmp_t *)(&uo[1]);
	ucp_zkrsq_time_t *ptime = (ucp_zkrsq_time_t *)(&uo[1]);
	ucp_zkrsq_mode_t *pmode = (ucp_zkrsq_mode_t *)(&uo[1]);


	if (!pconf_src || !pinfo) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));
	//回差温度设置
	if (pconf_src->back_tmp != pconf_local->back_tmp) {
		len = sizeof(ucp_zkrsq_tmp_t);
		ptmp->tmp = (u_int8_t )pconf_src->back_tmp;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_DEV_TEMPERATURE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	//补充温度
	if (pconf_src->compensation_tmp != pconf_local->compensation_tmp) {
		len = sizeof(ucp_zkrsq_tmp_t);
		ptmp->tmp = (u_int8_t )pconf_src->compensation_tmp;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_COMPENSATE_TEMPERATURE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	//除霜时间
	if (pconf_src->defrost_time != pconf_local->defrost_time) {
		len = sizeof(ucp_zkrsq_time_t);
		ptime->time = pconf_src->defrost_time;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_DEFROST_PERIOD,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}
	
	//除霜进入温度
	if (pconf_src->defrost_in_tmp != pconf_local->defrost_in_tmp) {
		len = sizeof(ucp_zkrsq_tmp_t);
		ptmp->tmp = (u_int8_t )pconf_src->defrost_in_tmp;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_DEFROST_IN_TEMPERATURE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}
	
	//除霜最大时间
	if (pconf_src->defrost_continue_time != pconf_local->defrost_continue_time) {
		len = sizeof(ucp_zkrsq_time_t);
		ptime->time = pconf_src->defrost_continue_time;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_DEFROST_PERSISTENCE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	//除霜退出温度
	if (pconf_src->defrost_out_tmp != pconf_local->defrost_out_tmp) {
		len = sizeof(ucp_zkrsq_tmp_t);
		ptmp->tmp = (u_int8_t )pconf_src->defrost_out_tmp;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_DEFROST_OUT_TEMPERATURE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}
	
	//回水模式
	if (pconf_src->back_water_mode != pconf_local->back_water_mode) {
		len = sizeof(ucp_zkrsq_mode_t);
		pmode->mode = pconf_src->back_water_mode;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_BACKWATER_MODE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}
	
	//回水温度
	if (pconf_src->back_water_tmp != pconf_local->back_water_tmp) {
		len = sizeof(ucp_zkrsq_tmp_t);
		ptmp->tmp = (u_int8_t )pconf_src->back_water_tmp;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_BACKWATER_TEMPERATURE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}
	
	*ret = RS_OK;
	
	return true;
}


static bool _zkrsq_timer_proc(user_t *user, u_int8_t *pvalue, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_zkrsq_info_t* pinfo = (cl_zkrsq_info_t *)ac->com_udp_dev_info.device_info;
	cl_zkrsq_tiemr_t *ptimer_src = (cl_zkrsq_tiemr_t *)pvalue;
	cl_zkrsq_tiemr_t *ptimer_local = &pinfo->timer;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_zkrsq_timer_t *ptimer = (ucp_zkrsq_timer_t *)(&uo[1]);

	if (!ptimer_src || !pinfo) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));
	//timer1
	if ((ptimer_src->timer1_valid != ptimer_local->timer1_valid) ||
		(ptimer_src->timer1_onoff != ptimer_local->timer1_onoff) ||
		(ptimer_src->timer1_hour != ptimer_local->timer1_hour) ||
		(ptimer_src->timer1_min != ptimer_local->timer1_min) ||
		(ptimer_src->timer1_hour_end != ptimer_local->timer1_hour_end) ||
		(ptimer_src->timer1_min_end != ptimer_local->timer1_min_end)) {
		len = sizeof(ucp_zkrsq_timer_t);
		ptimer->onoff = ptimer_src->timer1_onoff;
		ptimer->hour = ptimer_src->timer1_hour;
		ptimer->min = ptimer_src->timer1_min;
		ptimer->hour_end = ptimer_src->timer1_hour_end;
		ptimer->min_end = ptimer_src->timer1_min_end;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_TIMER1_STATE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	//timer2
	if ((ptimer_src->timer2_valid != ptimer_local->timer2_valid) ||
		(ptimer_src->timer2_onoff != ptimer_local->timer2_onoff) ||
		(ptimer_src->timer2_hour != ptimer_local->timer2_hour) ||
		(ptimer_src->timer2_min != ptimer_local->timer2_min) ||
		(ptimer_src->timer2_hour_end != ptimer_local->timer2_hour_end) ||
		(ptimer_src->timer2_min_end != ptimer_local->timer2_min_end)) {
		len = sizeof(ucp_zkrsq_timer_t);
		ptimer->onoff = ptimer_src->timer2_onoff;
		ptimer->hour = ptimer_src->timer2_hour;
		ptimer->min = ptimer_src->timer2_min;
		ptimer->hour_end = ptimer_src->timer2_hour_end;
		ptimer->min_end = ptimer_src->timer2_min_end;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_TIMER2_STATE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	//timer2
	if ((ptimer_src->timer3_valid != ptimer_local->timer3_valid) ||
		(ptimer_src->timer3_onoff != ptimer_local->timer3_onoff) ||
		(ptimer_src->timer3_hour != ptimer_local->timer3_hour) ||
		(ptimer_src->timer3_min != ptimer_local->timer3_min) ||
		(ptimer_src->timer3_hour_end != ptimer_local->timer3_hour_end) ||
		(ptimer_src->timer3_min_end != ptimer_local->timer3_min_end)) {
		len = sizeof(ucp_zkrsq_timer_t);
		ptimer->onoff = ptimer_src->timer3_onoff;
		ptimer->hour = ptimer_src->timer3_hour;
		ptimer->min = ptimer_src->timer3_min;
		ptimer->hour_end = ptimer_src->timer3_hour_end;
		ptimer->min_end = ptimer_src->timer3_min_end;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZKRSQ, UCAT_IA_ZKRSQ_TIMER3_STATE,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	*ret = RS_OK;
	
	return true;
}


bool zkrsq_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	u_int16_t data16;
	u_int8_t action, value;
	void *tmpvar;
	
	info = (cln_common_info_t *)&pkt->data[0];

	switch(info->action){
	case ACT_ZKRSQ_CTRL:
		data16 = cci_u16_data(info);
        action = (data16 >> 8)&0xff;
        value = data16 &0xff;
		_zkrsq_ctrl_proc(user, action, value, ret);
		break;
	case ACT_ZKRSQ_CONFIG:
		tmpvar = cci_pointer_data(info);
		_zkrsq_config_proc(user, tmpvar, ret);
		break;
	case ACT_ZKRSQ_TIMER:
		tmpvar = cci_pointer_data(info);
		_zkrsq_timer_proc(user, tmpvar, ret);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;
	}
	
	return true;
}


CLIB_API RS cl_zkrsq_ctrl(cl_handle_t dev_handle, ZKRSQ_ACTION_T action, u_int8_t value)
{
	u_int16_t data;

	CL_CHECK_INIT;
	data = BUILD_U16((u_int8_t)action, value);

	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZKRSQ,
	                         ACT_ZKRSQ_CTRL, data);
}

CLIB_API RS cl_zkrsq_config(cl_handle_t dev_handle, cl_zk_confset_t *pconfig)
{
	CL_CHECK_INIT;
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZKRSQ,
		ACT_ZKRSQ_CONFIG, (u_int8_t *)pconfig, sizeof(*pconfig));
}

CLIB_API RS cl_zkrsq_timer(cl_handle_t dev_handle, cl_zkrsq_tiemr_t *ptimer)
{
	CL_CHECK_INIT;
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZKRSQ,
		ACT_ZKRSQ_TIMER, (u_int8_t *)ptimer, sizeof(*ptimer));
}


