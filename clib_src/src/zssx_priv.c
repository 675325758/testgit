/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: zssx_priv.c
**  File:    zssx_priv.c
**  Author:  liubenlong
**  Date:    09/30/2015
**
**  Purpose:
**    中山商贤.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "udp_device_common_priv.h"
#include "cl_zssx.h"
#include "zssx_priv.h"


/* Macro constant definitions. */


/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

CLIB_API RS cl_zssx_ctrl(cl_handle_t dev_handle, ZSSX_ACTION_T action, u_int8_t value)
{
	u_int16_t data;

	CL_CHECK_INIT;
	data = BUILD_U16((u_int8_t)action, value);

	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZSSX,
	                         ACT_ZSSX_CTRL_ON, data);	
}

CLIB_API RS cl_zssx_timer(cl_handle_t dev_handle, u_int8_t on, u_int16_t min)
{
	u_int32_t data;

	CL_CHECK_INIT;
	data = ((on<<16)&0xff0000)|(min&0xffff);

	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZSSX,
	                         ACT_ZSSX_TIMER, data);	
}

CLIB_API RS cl_zssx_wifi_conf(cl_handle_t dev_handle, cl_zssx_ssid_t *wifi)
{
	char buff[256];

	CL_CHECK_INIT;
	memcpy(buff, wifi, sizeof(*wifi));

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZSSX,
	                         ACT_ZSSX_WIFI_CONF, buff, sizeof(*wifi));		
}

static void __zssx_data_build(ucp_zssx_work_t *pwork, cl_zssx_info_t *plinfo)
{
	pwork->onoff.power_onoff = plinfo->power_onoff;
	pwork->onoff.lock_onoff = plinfo->lock_onoff;
	pwork->onoff.anion_onoff = plinfo->anion_onoff;
	pwork->onoff.tmp_type = plinfo->tmp_type;

	pwork->onoff.led_onoff = plinfo->led_onoff;
	pwork->onoff.tmp_onoff = plinfo->tmp_onoff;

	pwork->set_status1.fire_level = plinfo->fire_level;
	pwork->set_status1.fake_firewood = plinfo->fake_firewood;
	
	pwork->set_status2.speed_gears = plinfo->speed_gears;

	pwork->timer_hour.timer_on = plinfo->timer_on;
	pwork->timer_hour.timer_off = plinfo->timer_off;
	pwork->timer_hour.timer_hour = plinfo->timer_hour;
	pwork->timer_min = plinfo->timer_min;
	pwork->tmp = plinfo->tmp;
}

static bool __zssx_wificonf_proc(user_t *user, cl_zssx_ssid_t *pwifi, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	ucp_zssx_priv_t* priv = (ucp_zssx_priv_t*)ac->com_udp_dev_info.device_info;
	cl_zssx_ssid_t *pwifi_local;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_zssx_wifi_t *pwifi_net = (ucp_zssx_wifi_t *)(&uo[1]);

	if (!ac || !priv || !pwifi) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));

	pwifi_local = &priv->info.wifi_conf;
	if (memcmp(pwifi_local, pwifi, sizeof(*pwifi)) == 0) {
		return true;
	}
	memcpy(pwifi_local, pwifi, sizeof(*pwifi));

	pwifi->ssid[32] = 0;
	pwifi->pswd[64] = 0;

	len = sizeof(*pwifi_net);
	strcpy(pwifi_net->ssid, pwifi->ssid);
	strcpy(pwifi_net->pswd, pwifi->pswd);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ELEC_HEATER, UCAT_IA_ELEC_HEATER_WIFI_SET,len);
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, true, 0x1, buf, sizeof(*uo)+len);

	return true;
}

static bool __zssx_timer_proc(user_t *user, u_int8_t on, u_int16_t min, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	ucp_zssx_priv_t* priv = (ucp_zssx_priv_t*)ac->com_udp_dev_info.device_info;
	cl_zssx_info_t *plinfo = &priv->info;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_zssx_work_t *pwork = (ucp_zssx_work_t *)(&uo[1]);

	if (!ac || !priv) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));

	if (on) {
		//现阶段没有开机定时器
		plinfo->timer_on = 0;
		plinfo->timer_off = 0;
	} else {
		plinfo->timer_on = 0;
		plinfo->timer_off = 1;		
	}
	
	plinfo->timer_hour = min/60;
	plinfo->timer_min = min%60;

	len = sizeof(*pwork);
	__zssx_data_build(pwork, plinfo);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ELEC_HEATER, UCAT_IA_ELEC_HEATER_PARAM_PRO,len);
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, true, 0x1, buf, sizeof(*uo)+len);

	return true;
}

static bool __zssx_on_proc(user_t *user, u_int8_t action, u_int8_t value, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	ucp_zssx_priv_t* priv = (ucp_zssx_priv_t*)ac->com_udp_dev_info.device_info;
	cl_zssx_info_t *plinfo = &priv->info;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_zssx_work_t *pwork = (ucp_zssx_work_t *)(&uo[1]);

	if (!ac || !priv) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));
	switch(action) {
	case ZSSX_ACTION_POWER_ONOFF:
		len = sizeof(*pwork);
		plinfo->power_onoff = value;
		break;
	case ZSSX_ACTION_LOCK_ONOFF:
		len = sizeof(*pwork);
		plinfo->lock_onoff = value;
		break;
	case ZSSX_ACTION_ANION_ONOFF:
		len = sizeof(*pwork);
		plinfo->anion_onoff = value;
		break;
	case ZSSX_ACTION_LED_ONOFF:
		len = sizeof(*pwork);
		plinfo->led_onoff = value;
		break;
	case ZSSX_ACTION_FIRE_LEVEL:
		len = sizeof(*pwork);
		plinfo->fire_level = value;
		plinfo->fake_firewood = value;
		if(value != 0) {
			plinfo->led_onoff = 0;
		} else {
			plinfo->led_onoff = 1;
		}
		break;
	case ZSSX_ACTION_FAKE_LEVEL:
		len = sizeof(*pwork);
		plinfo->fake_firewood = value;
		break;
	case ZSSX_ACTION_GEARS:
		len = sizeof(*pwork);
		plinfo->speed_gears = value;
		break;
	case ZSSX_ACTION_TMP_TYPE:
		len = sizeof(*pwork);
		plinfo->tmp_type = value;
		break;
	case ZSSX_ACTION_TMP_SET:
		len = sizeof(*pwork);
		plinfo->tmp = value;
		plinfo->tmp_onoff = 1;
		
		if ((value&0x7f) < 50) {
			plinfo->tmp_type = 0;
		} else {
			plinfo->tmp_type = 1;
		}
		break;
	case ZSSX_ACTION_TMP_ONOFF:
		len = sizeof(*pwork);
		plinfo->tmp_onoff = value;
		break;
	default:
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if (len > 0) {
		__zssx_data_build(pwork, plinfo);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ELEC_HEATER, UCAT_IA_ELEC_HEATER_PARAM_PRO,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, true, 0x1, buf, sizeof(*uo)+len);
	}

	return true;
}


bool _zssx_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	u_int8_t action, value;
	u_int16_t data16;
	u_int32_t data32;
	u_int8_t on;
	u_int16_t min;
	void *tmpvar;
	
	info = (cln_common_info_t *)&pkt->data[0];

	switch(info->action){
	case ACT_ZSSX_CTRL_ON:
		data16 = cci_u16_data(info);
        action = (data16 >> 8)&0xff;
        value = data16 &0xff;
		__zssx_on_proc(user, action, value, ret);
		break;
	case ACT_ZSSX_TIMER:
		data32 = cci_u32_data(info);
		on = (data32>>16)&0xff;
		min = data32&0xffff;
		__zssx_timer_proc(user, on, min, ret);
		break;
	case ACT_ZSSX_WIFI_CONF:
		tmpvar = cci_pointer_data(info);
		__zssx_wificonf_proc(user, tmpvar, ret);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;
	}
	
	return true;
}

static bool __zssx_update_wifi(ucp_zssx_priv_t* pinfo, ucp_obj_t* obj)
{
	ucp_zssx_wifi_t *pwifi_net;
	cl_zssx_ssid_t *wifi_local;

	wifi_local = &pinfo->info.wifi_conf;
	pwifi_net = (ucp_zssx_wifi_t *)(obj+1);

	pwifi_net->ssid[32] = 0;
	pwifi_net->pswd[64] = 0;

	if (strcmp(wifi_local->ssid, pwifi_net->ssid) == 0 &&
		strcmp(wifi_local->pswd, pwifi_net->pswd) == 0) {
		return false;
	}

	strcpy(wifi_local->ssid, pwifi_net->ssid);
	strcpy(wifi_local->pswd, pwifi_net->pswd);

	return true;
}

static bool __zssx_update_work(ucp_zssx_priv_t* pinfo, ucp_obj_t* obj)
{
	bool ret = false;
	cl_zssx_info_t *info = &pinfo->info;
	ucp_zssx_work_t *pwork_net = (ucp_zssx_work_t *)(&obj[1]);
	ucp_zssx_work_t *pwork_local = (ucp_zssx_work_t *)(&pinfo->set_work);
	zssx_on_info_t *ponoff = &pwork_net->onoff;
	zssx_set_status1_t *pset_status1 = &pwork_net->set_status1;
	zssx_set_status2_t *pset_status2 = &pwork_net->set_status2;
	zssx_timer_hour_t *ptimer_hour = &pwork_net->timer_hour;

	if (memcmp(pwork_local, pwork_net, sizeof(*pwork_local))== 0) {
		return false;
	}

	memcpy(pwork_local, pwork_net, sizeof(*pwork_local));

	//onoff
	if (ponoff->led_onoff != info->led_onoff) {
		info->led_onoff = ponoff->led_onoff;
		ret = true;
	}
	if (ponoff->tmp_type != info->tmp_type) {
		info->tmp_type = ponoff->tmp_type;
		ret = true;
	}
	if (ponoff->anion_onoff != info->anion_onoff) {
		info->anion_onoff = ponoff->anion_onoff;
		ret = true;
	}
	if (ponoff->lock_onoff != info->lock_onoff) {
		info->lock_onoff = ponoff->lock_onoff;
		ret = true;
	}
	if (ponoff->power_onoff != info->power_onoff) {
		info->power_onoff = ponoff->power_onoff;
		ret = true;
	}
	if (ponoff->tmp_onoff != info->tmp_onoff) {
		info->tmp_onoff = ponoff->tmp_onoff;
		ret = true;
	}

	//set1
	if (pset_status1->fake_firewood != info->fake_firewood) {
		info->fake_firewood = pset_status1->fake_firewood;
		ret = true;
	}
	if (pset_status1->fire_level != info->fire_level) {
		info->fire_level = pset_status1->fire_level;
		ret = true;
	}

	//set2
	if (pset_status2->speed_gears != info->speed_gears) {
		info->speed_gears = pset_status2->speed_gears;
		ret = true;
	}

	//tmp
	if (info->tmp != pwork_net->tmp) {
		info->tmp = pwork_net->tmp;
		ret = true;
	}

	//timer
	if (ptimer_hour->timer_on != info->timer_on) {
		info->timer_on = ptimer_hour->timer_on;
		ret = true;
	}
	if (ptimer_hour->timer_off != info->timer_off) {
		info->timer_off = ptimer_hour->timer_off;
		ret = true;
	}
	if (ptimer_hour->timer_hour != info->timer_hour) {
		info->timer_hour = ptimer_hour->timer_hour;
		ret = true;
	}

	//misc
	if (pwork_net->timer_min != info->timer_min) {
		info->timer_min = pwork_net->timer_min;
		ret = true;
	}
	if (pwork_net->ntc_fault != info->ntc_fault) {
		info->ntc_fault = pwork_net->ntc_fault;
		ret = true;
	}
	if (pwork_net->thermostat_fault != info->thermostat_fault) {
		info->thermostat_fault = pwork_net->thermostat_fault;
		ret = true;
	}
	if (pwork_net->work_status != info->work_status) {
		info->work_status = pwork_net->work_status;
		ret = true;
	}
	
	return true;
	//return ret;
}


bool _zssx_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	ucp_zssx_priv_t* pinfo = ac->com_udp_dev_info.device_info; 
	bool ret = false;

	if(!obj ){
		return false;
	}

	ac->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
		case UCAT_IA_ELEC_HEATER_WIFI_SET:
			if(is_valid_obj_data(obj, sizeof(ucp_zssx_wifi_t))){
				ret = __zssx_update_wifi(pinfo, obj);
			}else{
				return false;
			}
			break;
		case UCAT_IA_ELEC_HEATER_PARAM_PRO:
			if(is_valid_obj_data(obj, sizeof(ucp_zssx_work_t))){
				ret = __zssx_update_work(pinfo, obj);
			}else{
				return false;
			}
			break;
		default:
			ret = false;
			break;
	}

	return ret;	
}

bool zssx_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
    ucp_zssx_priv_t *priv;
    cl_zssx_info_t* ci;
	ucc_session_t *s = NULL;
    
    if(!di || !ac || !ac->com_udp_dev_info.device_info){
        return false;
    }
    
    priv = ac->com_udp_dev_info.device_info;
    ci = cl_calloc(sizeof(*ci), 1);
    if (!ci) {
        return false;
    }
    
    memcpy(ci, &priv->info, sizeof(*ci));
    if (ac->sac && ac->sac->user && ac->sac->user->uc_session) {
		s = ac->sac->user->uc_session;
		ci->wifi_mode = s->wifi_mode;
		priv->info.wifi_mode = s->wifi_mode;
	}
    
    di->device_info = ci;
    
    return true;
}


