/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: rfgw_scm_ctrl.c
**  File:    rfgw_scm_ctrl.c
**  Author:  liubenlong
**  Date:    02/27/2016
**
**  Purpose:
**    Rf网关透传c文件.
**************************************************************************/


/* Include files. */
#include "rfgw_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "rfgw_priv.h"
#include "smart_appliance_priv.h"
/* Macro constant definitions. */


/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */
#define ENTER()		log_debug("enter %s %d\n", __FUNCTION__, __LINE__)

/* Global variable declarations. */
static u_int8_t yl_check_sum(u_int8_t *data, int len)
{
	u_int8_t checksum = 0;

	while(len > 0) {
		checksum += data[len-1];
		len--;
	}

	return checksum;
}

static u_int8_t rfgw_yl_do_off_slight(user_t *user, u_int8_t onoff)
{
	char buf[100] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *value = (u_int8_t *)&uo[1];

	*value = onoff;
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_YLSGBJ, sizeof(buf));
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(buf));

	return true;
}

static bool yl_rfgw_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	u_int8_t value, *pvalue;
	u_int8_t buffer[1024] = {0};
	u_int8_t len = 0;
	ucp_rfgw_scm__head_t *phd = (ucp_rfgw_scm__head_t *)buffer;
	ucp_rfgw_scm_alarm_time_t *prsat = (ucp_rfgw_scm_alarm_time_t *)buffer;
	ucp_rfgw_scm_horn_voice_t *prshv = (ucp_rfgw_scm_horn_voice_t *)buffer;
	ucp_rfgw_scm_horn_action_t *prsha = (ucp_rfgw_scm_horn_action_t *)buffer;
	rfgw_priv_t *p;
	smart_air_ctrl_t* air_ctrl;
	smart_appliance_ctrl_t* sac;
	cl_rfdev_scm_yl_t *prsy;

	ENTER();
	if(!user){
		*ret = RS_OFFLINE;
		return false;
	}

	if(!user || !user->smart_appliance_ctrl){
		return false;
	}
	sac = user->smart_appliance_ctrl;
	air_ctrl = sac->sub_ctrl;
	if (!air_ctrl) {
		return false;
	}
	
	p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	prsy = &p->rfdef_scm_dev.rfdev_scm_dev_data.yl_info;
	memset(buffer, 0, sizeof(buffer));
	info = (cln_common_info_t *)&pkt->data[0];

	switch(info->action){
	case ACT_RF_YL_ALARM_TIME:
		value = cci_u8_data(info);
		prsat->head.boot_code = YL_BOOT_CODE;
		prsat->head.func_code = YL_FUNC_CODE_SET;
		prsat->head.len = 0x2;
		prsat->head.check_sum = 0;
		prsat->action = YL_ACTION_SET_ALARM_TIME;
		prsat->time = value;
		len = sizeof(*prsat);
		prsy->time = prsat->time;
		break;
	case ACT_RF_YL_VOICE:
		pvalue = cci_pointer_data(info);
		prshv->head.boot_code = YL_BOOT_CODE;
		prshv->head.func_code = YL_FUNC_CODE_SET;
		prshv->head.len = 0x3;
		prshv->head.check_sum = 0;
		prshv->action = YL_ACTION_SET_SOUND;		
		prshv->func = pvalue[0];
		prshv->voice = pvalue[1];
		len = sizeof(*prshv);
		if (prshv->func == 1) {
			prsy->door_voice = prshv->voice;
		} else if (prshv->func == 2) {
			prsy->alarm_voice = prshv->voice;
		}
		break;
	case ACT_RF_YL_SIREN_OFF:
		prsha->head.boot_code = YL_BOOT_CODE;
		prsha->head.func_code = YL_FUNC_CODE_SET;
		prsha->head.len = 0x3;
		prsha->head.check_sum = 0;
		prsha->action = YL_ACTION_SET_HORN;
		prsha->func = 0x2;
		prsha->ctrl = !!cci_u8_data(info);
		len = sizeof(*prsha);
		prsy->sos_on = prsha->ctrl;

		// 让网关下声光报警器都关闭下
		if (prsha->ctrl == 0) {
			rfgw_yl_do_off_slight(user, 0);
		}
		break;

	default:
		log_debug("err action\n", info->action);
		*ret = RS_INVALID_PARAM;
		break;
	}

	if (len > 0) {
		phd->check_sum = yl_check_sum(buffer, len);
		log_debug("%s send action=%u\n", __FUNCTION__, info->action);
		scm_send_single_set_pkt(user->uc_session,buffer,len);
	}

    return true;
}

bool rfgw_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool retl= false;

	ENTER();
	switch(user->ext_type) {
	case ETYPE_IJ_RFGW_YL:
		retl = yl_rfgw_scm_proc_notify(user, pkt, ret);
		break;
	default:
		log_debug("rfgw_scm_proc_notify get err type=%u\n", user->ext_type);
		break;
	}

	return retl;
}

static bool yl_scm_do_parse_scm_command(smart_air_ctrl_t* air_ctrl,u_int8_t* pcmd, u_int16_t cmd_len)
{
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	ucp_rfwg_scm_allstate_t* recv_info = (ucp_rfwg_scm_allstate_t* )pcmd;
	cl_rfdev_scm_yl_t *prsy = &p->rfdef_scm_dev.rfdev_scm_dev_data.yl_info;
//	u_int8_t checksum = 0;
//	u_int8_t last_checksum = 0;
    
	if (cmd_len < sizeof(*recv_info)) {
		log_debug("%s err cmd_len=%u sizeof(*recv_info)=%u\n", 
			__FUNCTION__, cmd_len, sizeof(*recv_info));
		return false;
	}

#if 0
	if ((recv_info->head.boot_code != YL_BOOT_CODE) ||
		(recv_info->head.func_code != YL_FUNC_CODE_GET) ||
		(recv_info->head.len != 0xc)) {
		log_debug("%s err check bc=%u fc=%u len=%u\n", 
			__FUNCTION__, recv_info->head.boot_code, recv_info->head.func_code, recv_info->head.len);
		return false;
	}

	last_checksum = recv_info->head.check_sum;
	recv_info->head.check_sum = 0;
	recv_info->head.check_sum = yl_check_sum(pcmd, sizeof(*recv_info));
	if (last_checksum != recv_info->head.check_sum) {
		log_debug("%s checksum failed last_checksum=%u newcheck_sum=%u\n", 
			__FUNCTION__, last_checksum, recv_info->head.check_sum);
		return false;
	}
#endif
	prsy->battery = recv_info->battery&0x7f;
	prsy->power = ((recv_info->battery>>7)&0x01);
	prsy->sos_on = recv_info->sos_alarm;
	prsy->door_voice = recv_info->door_voice;
	prsy->alarm_voice = recv_info->alarm_voice;
	prsy->time = recv_info->alarm_time;
	prsy->is_guard = recv_info->is_guard;

	p->rfdef_scm_dev.valid = true;

	log_debug("yl_scm_do_parse_scm_command battery=%u power=%u sos_on=%u door_voice=%u "
		"alarm_voice=%u time=%u is_guard=%u\n", 
		prsy->battery, prsy->power, prsy->sos_on, prsy->door_voice, prsy->alarm_voice,
		prsy->time, prsy->is_guard);
	return true;
}


static bool _rfgw_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
    uc_tlv_t* tlv;
    int remain = obj->param_len;
    
    if(remain < sizeof(*tlv)){
        return false;
    }
    
    tlv = (uc_tlv_t*)(obj+1);
    tlv->type = ntohs(tlv->type);
    tlv->len = ntohs(tlv->len);
    while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
        remain -= (sizeof(uc_tlv_t) + tlv->len);
		log_debug("_rfgw_scm_update_tlv_data tlv->type=%u\n", tlv->type);
        switch (tlv->type) {
            case TLV_TYPE_SCM_COMMAND:
                ret = yl_scm_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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
    
    return ret;
}

static bool rfgw_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	user_t *puser = NULL;
//	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;

	if (!air_ctrl || !air_ctrl->sac || !air_ctrl->sac->user) {
		return false;
	}
	puser = air_ctrl->sac->user;
	
	switch(puser->ext_type) {
	case ETYPE_IJ_RFGW_YL:
		ret = _rfgw_scm_update_tlv_data(air_ctrl,action,obj);
		break;
	default:
		log_debug("rfgw_scm_update_tlv_data get err type=%u\n", puser->ext_type);
		break;
	}

	return ret;
}

bool rfgw_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
	
    switch (obj->attr) {
        case UCAT_IA_TT_ALLSTATE:
            ret = rfgw_scm_update_tlv_data(air_ctrl,action,obj);
            break;
        case UCAT_IA_TT_CMD:
            break;
        case UCAT_IA_TT_CMDRET:
            break;
        default:
            break;
    }
    
    return ret;
}

void rfgw_do_query_hook(smart_air_ctrl_t* air_ctrl)
{

}

int rfgw_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
{
    u_int8_t value = 0;
    tlv_dev_ident_t* ti;

	log_debug("enter rfgw_get_ext_type_by_tlv tlv->len=%u\n", tlv->len);
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
		log_debug("rfgw_get_ext_type_by_tlv value=%u\n", value);
    }
    
    return value;
}

void yl_scm_dev_set_defense_batch(user_t *puser, u_int8_t is_defence)
{
	u_int8_t buffer[1024] = {0};
	u_int8_t len = 0;
	rfgw_priv_t *p;
	smart_air_ctrl_t* air_ctrl;
	smart_appliance_ctrl_t* sac;
	ucp_rfgw_scm__head_t *phd = (ucp_rfgw_scm__head_t *)buffer;
	ucp_rfgw_scm_defense_t *prsd = (ucp_rfgw_scm_defense_t *)buffer;

	if(!puser || !puser->smart_appliance_ctrl){
		return;
	}
	sac = puser->smart_appliance_ctrl;
	air_ctrl = sac->sub_ctrl;
	if (!air_ctrl) {
		return;
	}
	
	p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	memset(buffer, 0, sizeof(buffer));
	
	prsd->head.boot_code = YL_BOOT_CODE;
	prsd->head.func_code = 0x2;
	prsd->head.len = 0x8;
	prsd->head.check_sum = 0;
	prsd->action = 0x2;
	prsd->is_defense = is_defence;
	len = sizeof(*prsd);
	phd->check_sum = yl_check_sum(buffer, len);
	scm_send_single_set_pkt(puser->uc_session,buffer,len);
	
	p->rfdef_scm_dev.rfdev_scm_dev_data.yl_info.is_guard = is_defence;
}

void rf_scm_dev_set_defense_batch(user_t *puser, u_int8_t is_defence)
{
	if (puser->sub_type != IJ_RFGW) {
		return;
	}
	
	switch(puser->ext_type) {
	case ETYPE_IJ_RFGW_YL:
		yl_scm_dev_set_defense_batch(puser, is_defence);
		break;
	default:
		log_debug("rf_scm_dev_set_defense_batch get err type=%u\n", puser->ext_type);
		break;
	}
}

