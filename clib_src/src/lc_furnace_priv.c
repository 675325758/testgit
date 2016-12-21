#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "lc_furnace_priv.h"
#include "cl_ia.h"
#include "cl_ch_blanket.h"
#include "udp_scm_direct_ctrl_priv.h"

//联创暖风机单品

void ah_get_attr(smart_air_ctrl_t *ac, u_int16_t arrt)
{
	ucp_obj_t uo;
	uo.objct = UCOT_IA;
	uo.sub_objct = UCOT_IA_FHF;
	uo.attr = arrt;
	uo.param_len = 0;
	sa_query_objects(ac->sac->user->uc_session, &uo, 1);
}

void ah_proc_set_temp_ctrl(smart_air_ctrl_t* ac, u_int8_t is_add, RS* ret)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lc_furnace_set_temp_t* set = (lc_furnace_set_temp_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_SET_TEMP, sizeof(*set));
	set->is_add = is_add;
	memset(set->pad, 0, sizeof(set->pad));
	*ret = sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*set));
	ah_get_attr(ac, UCAT_FHF_GET_TEMP);
}

void ah_proc_set_timer_ctrl(smart_air_ctrl_t* ac, u_int8_t is_add, RS* ret)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lc_furnace_set_timer_t* set = (lc_furnace_set_timer_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_SET_TIMER, sizeof(*set));
	set->is_add = is_add;
	memset(set->pad, 0, sizeof(set->pad));	
	*ret = sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*set));
	ah_get_attr(ac, UCAT_FHF_GET_TIMER);
}

void ah_proc_work_ctrl(smart_air_ctrl_t* ac, int pkt_type, RS* ret)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lc_furnace_work_t* set = (lc_furnace_work_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_WORK, sizeof(*set));
	memset(set,0xff,sizeof(*set));
	switch(pkt_type){
		case CLNE_AH_POWER:
			set->is_on = ac->ah_info.ah_work_stat.is_on;
			break;
		case CLNE_AH_SHAKE:
			set->is_shake = ac->ah_info.ah_work_stat.is_shake;
			break;
		case CLNE_AH_ECO:
			set->is_eco = ac->ah_info.ah_work_stat.is_eco;
			break;
		case CLNE_AH_MODE:
			set->is_mode_high = ac->ah_info.ah_work_stat.heat_mode;
			break;
		default:
		{
			return;
			break;
		}
	}

	memset(set->pad, 0, sizeof(set->pad));
	*ret = sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*set));
}


void ah_proc_refresh_power(smart_air_ctrl_t* ac, int pkt_type, RS* ret)
{
	ucp_obj_t uo[2]={{UCOT_IA,UCOT_IA_FHF,UCAT_FHF_GET_POWER,0},{UCOT_IA,UCOT_IA_FHF,UCAT_FHF_GET_TIMER,0}};
	
	sa_query_objects(ac->sac->user->uc_session, &uo[0], 2);
}

bool _ah_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	user_t *user;
	smart_air_ctrl_t* ac;
	cln_sa_ah_info_t* sai;
  
	sai = (cln_sa_ah_info_t*)pkt->data;
	
	
	user = lookup_by_handle(HDLT_USER, sai->dev_handle);
	if (!user || user->sub_type != IJ_812 || !user->smart_appliance_ctrl ) {
		log_err(false, "air_proc_notify error handle %08x\n",sai->dev_handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if(user->is_dev_support_scm){
		return scm_proc_notify(user,pkt,ret);
	}
	
	*ret = RS_OK;
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	switch(pkt->type){
		case CLNE_AH_TEMP:
			ah_proc_set_temp_ctrl(ac, sai->action_param, ret);
			break;
			
		case CLNE_AH_TIMER:
			ah_proc_set_timer_ctrl(ac, sai->action_param, ret);
			break;

		case CLNE_AH_POWER:
			ac->ah_info.ah_work_stat.is_on = sai->action_param;		
			ah_proc_work_ctrl(ac,pkt->type, ret);
			break;

			
		case CLNE_AH_SHAKE:
			ac->ah_info.ah_work_stat.is_shake = sai->action_param;		
			ah_proc_work_ctrl(ac,pkt->type, ret);
			break;
			
		case CLNE_AH_ECO:
			ac->ah_info.ah_work_stat.is_eco = sai->action_param;		
			ah_proc_work_ctrl(ac, pkt->type,ret);
			break;
			
		case CLNE_AH_MODE:
			ac->ah_info.ah_work_stat.heat_mode = sai->action_param;		
			ah_proc_work_ctrl(ac, pkt->type,ret);
			break;
		case CLNE_AH_REFRESH_POWER:
			ah_proc_refresh_power(ac,pkt->type,ret);
			break;
			
		default:
            *ret = RS_ERROR;
			break; 
	}
	return true;
}

bool ah_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    if (pkt->type < CLNE_AH_START || pkt->type > CLNE_AH_END) {
		return false;
	}
    
    return _ah_proc_notify(pkt, ret);
    
}
#if 0
RS ah_init(smart_appliance_ctrl_t* sac)
{
    smart_ah_ctrl_t* ah_ctrl;
    
    if (sac->sub_ctrl) {
        return RS_OK;
    }
    
    if (!(ah_ctrl = cl_calloc(sizeof(smart_ah_ctrl_t), 1))) {
        return RS_ERROR;
    }
    
    ah_ctrl->sac = sac;
    sac->sub_ctrl = ah_ctrl;
    
    return RS_OK;
}

void ah_free(smart_ah_ctrl_t* ac)
{
    if (ac) {        
        cl_free(ac);
    }
}
#endif
void ah_build_objs(user_t* user,cl_dev_info_t* ui)
{
    smart_appliance_ctrl_t* sma;
    smart_air_ctrl_t* ac;
    cl_ah_info_t* ai;
	bool fix_fire = false;

	 log_debug("ah_build_objs  \n");
    
    if ( (sma = user->smart_appliance_ctrl) == NULL) {
        return;
    }
    
    if (!(ac = sma->sub_ctrl) || user->sub_type != IJ_812) {
        return;
    }
    
    ai = cl_calloc(sizeof(cl_ah_info_t), 1);
    if (!ai) {
        return;
    }
    
    
    memcpy(ai, &ac->ah_info, sizeof(cl_ah_info_t));
    ai->handle = user->handle;

    //大小火模式修正
    if(ac->ah_info.ah_work_stat.is_eco){
		//透传版本
		if(ac->sac->user->is_dev_support_scm){
			if(ac->sac->user->scm_dev_desc != NULL && (strstr((char*)ac->sac->user->scm_dev_desc,"V1.1") != NULL)){
				fix_fire = true;
			}
		}else{
			fix_fire = true;
		}
		if(fix_fire){
			if(ac->ah_info.ah_work_stat.room_temp < 18){
				ai->ah_work_stat.heat_mode = AH_HEAT_MODE_HIGH;
			}else if(ac->ah_info.ah_work_stat.room_temp >= 22){
				//由于历史原因，true表示不工作
				ai->ah_work_stat.is_heating = true;
			}else{
				ai->ah_work_stat.heat_mode = AH_HEAT_MODE_LOW;
			}
		}
    }
    
    ui->ah_info = ai;
}

void ah_quick_query_ia_info(smart_air_ctrl_t* ac)
{
    ucc_session_t *s;
    ucp_obj_t stat_objs[] = {{UCOT_IA,UCOT_IA_FHF,0xFFFF,0}};

    if(!ac)
       return;
    
    s = ac->sac->user->uc_session;
    log_debug("query ah object\n");

    sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

bool ah_update_work(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	lc_furnace_work_t* value;
	cl_ah_work_stat_t *st;
	
	if (!is_except_attr(obj, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_WORK)) {
		return false;
	}
                
	if (obj->param_len < sizeof(*value))
		return false;
	
	value = (lc_furnace_work_t*)(obj+1);
	st = &air_ctrl->ah_info.ah_work_stat;
	st->is_on = value->is_on;
	st->is_eco = value->is_eco;
	st->heat_mode = value->is_mode_high;
	st->is_shake = value->is_shake;
	st->is_heating = value->is_heating;
	st->is_topple_protect = IS_TOPPLE_PROTECT(value->fault_stat);
	st->is_temp_high_protect = IS_TEMP_HIGH_PROTECT(value->fault_stat);
	st->is_furnace_high_protect = IS_FURNACE_HIGH_PROTECT(value->fault_stat);
	st->is_furnace_error = IS_FURNACE_ERROR(value->fault_stat);	
	air_ctrl->work_stat_update_ok = 1;

	//联创处理老设备问题

	//log_debug("furnace: is_on:%hhu, is_eco:%hhu, head_mode:%hhu, is_shake:%hhu, is_heating:%hhu,is_topple_protect:%hhu,is_temp_high_protect:%hhu\n");
	
	return true;
}

bool ah_update_temp(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	lc_furnace_get_temp_t* value;
	cl_ah_work_stat_t *st;
	
	if (!is_except_attr(obj, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_GET_TEMP)) {
		return false;
	}
                
	if (obj->param_len < sizeof(*value))
		return false;
	
	value = (lc_furnace_get_temp_t*)(obj+1);
	st = &air_ctrl->ah_info.ah_work_stat;
	st = &air_ctrl->ah_info.ah_work_stat;
	st->room_temp = value->room_temp;	
	st->thermode_1_temp = value->temp1;
	st->thermode_2_temp = value->temp2;
	st->set_temp = value->set_temp;
	return true;	
}

bool ah_update_timer(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	lc_furnace_get_timer_t* value;
	cl_ah_work_stat_t *st;
	
	if (!is_except_attr(obj, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_GET_TIMER)) {
		return false;
	}
                
	if (obj->param_len < sizeof(*value))
		return false;
	
	value = (lc_furnace_get_timer_t*)(obj+1);
	st = &air_ctrl->ah_info.ah_work_stat;
	st->remain_minute = ntohs(value->remain_minute);
	st->set_hour = value->set_hour;
	st->timer_type = value->timer_type;
	return true;
}

bool ah_update_power(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	lc_furnace_get_power_t* value;
	cl_ah_work_stat_t *st;
	
	if (!is_except_attr(obj, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_GET_POWER)) {
		return false;
	}
	
	if (obj->param_len < sizeof(*value))
		return false;
	
	value = (lc_furnace_get_power_t*)(obj+1);
	st = &air_ctrl->ah_info.ah_work_stat;
	st->epower = ntohl(value->power);
	return true;
}

bool ah_update_sn(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	lc_furnace_get_sn_t* value;
	cl_ah_work_stat_t *st;
	
	if (!is_except_attr(obj, UCOT_IA, UCOT_IA_FHF, UCAT_FHF_GET_SN)) {
		return false;
	}
                
	if (obj->param_len < sizeof(*value))
		return false;
	
	value = (lc_furnace_get_sn_t*)(obj+1);
	st = &air_ctrl->ah_info.ah_work_stat;
	
	return true;
}

bool ah_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	switch (obj->attr) {
		case UCAT_FHF_WORK:
			ret =  ah_update_work(air_ctrl, action, obj);
		break;

		case UCAT_FHF_GET_TIMER:
			ret =  ah_update_timer(air_ctrl, action, obj);
			break;
		
		case UCAT_FHF_GET_TEMP:
			ret =  ah_update_temp(air_ctrl, action, obj);
			break;
			
		case UCAT_FHF_GET_POWER:
			ret =  ah_update_power(air_ctrl, action, obj);
			break;
			
		case UCAT_FHF_GET_SN:
			ret = ah_update_sn(air_ctrl, action, obj);
			break;

		default:
			break;
	}
	
	return ret;
}

int ah_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	
	switch(obj->attr){
		case UCAT_FHF_WORK:
			if(error == ERR_NONE)
				event = AHE_CTRL_OK;
			else
				event = AHE_CTRL_FAIL;			
			break;
			
		case UCAT_FHF_SET_TEMP:
			if(error == ERR_NONE)
				event = AHE_CTRL_TEMP_OK;
			else
				event = AHE_CTRL_TEMP_FAIL;
			break;
			
		case UCAT_FHF_SET_TIMER:
			if(error == ERR_NONE)
				event = AHE_CTRL_TIMER_OK;
			else
				event = AHE_CTRL_TIMER_FAIL;
			break;
			
		default:
			break;
	}
	
	return event;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


#define LCYT_TIME_DEV_OFF 0x1
#define LCYT_TIME_DEV_ON 0x2

static void lcyt_get_attr(smart_air_ctrl_t *ac, u_int16_t arrt)
{
	ucp_obj_t uo;
	uo.objct = UCOT_IA;
	uo.sub_objct = UCSOT_IA_HEATER_LCYT;
	uo.attr = arrt;
	uo.param_len = 0;
	sa_query_objects(ac->sac->user->uc_session, &uo, 1);
}

static RS lcyt_proc_refresh_timer(smart_air_ctrl_t* ac)
{
	lcyt_get_attr( ac, UCAT_HEATER_LCYT_REMAIN_TIME);
	lcyt_get_attr( ac, UCAT_HEATER_LCYT_SET_TIME);
	
	return RS_OK;
}

static RS lcyt_proc_set_timer_ctrl(smart_air_ctrl_t* ac, u_int16_t time,u_int8_t time_type)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lcyt_set_timer_t* st = (lcyt_set_timer_t*)(uo+1);
	user_t* user = ac->sac->user;

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HEATER_LCYT, UCAT_HEATER_LCYT_SET_TIME, sizeof(ucp_obj_t)+sizeof(*st));
	st->time =  htons(time);
	if(time_type == LCYT_TIME_DEV_ON){
		st->time_type = LCYT_TIME_DEV_ON;
	}else{
		st->time_type = LCYT_TIME_DEV_OFF;
	}
	sa_set_obj_value_only(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	lcyt_proc_refresh_timer(ac);
	
	return RS_OK;
}

static RS lcyt_proc_set_work_stat_ctrl(smart_air_ctrl_t* ac, u_int8_t onoff)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lcyt_work_stat_t* st = (lcyt_work_stat_t*)(uo+1);
	user_t* user = ac->sac->user;

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HEATER_LCYT, UCAT_HEATER_LCYT_WORK, sizeof(ucp_obj_t)+sizeof(*st));
	st->stat = !!onoff;
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS lcyt_proc_set_work_mode_ctrl(smart_air_ctrl_t* ac, u_int8_t mode)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lcyt_work_mode_t* st = (lcyt_work_mode_t*)(uo+1);
	user_t* user = ac->sac->user;

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HEATER_LCYT, UCAT_HEATER_LCYT_MODE, sizeof(ucp_obj_t)+sizeof(*st));
	st->mode = mode;
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS lcyt_proc_set_temp_ctrl(smart_air_ctrl_t* ac, u_int8_t temp)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lcyt_set_temp_t* st = (lcyt_set_temp_t*)(uo+1);
	user_t* user = ac->sac->user;

	st->set_temp = temp;
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HEATER_LCYT, UCAT_HEATER_LCYT_SET_TEMP, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS lcyt_proc_set_gear_ctrl(smart_air_ctrl_t* ac, u_int8_t gear)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lcyt_set_gear_t* st = (lcyt_set_gear_t*)(uo+1);
	user_t* user = ac->sac->user;

	st->gear = gear;
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HEATER_LCYT, UCAT_HEATER_LCYT_GEAR, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static int _lcyt_proc_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_common_info_t* ci;
	smart_air_ctrl_t* ac;
	RS ret = RS_OK;
	
	ci = (cln_common_info_t *)&pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->handle);
	if (!user || !user->is_udp_ctrl || !user->smart_appliance_ctrl) {
		return RS_INVALID_PARAM;
	}

	if(! user->online){
		return RS_OFFLINE;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	
	switch(ci->action)
	{
		case IA_AIRHEATER_YCYT_STATUS_TEMP_SET: //设置温度
		{
			ret = lcyt_proc_set_temp_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
		case IA_AIRHEATER_YCYT_STATUS_MODE: //设置温度
		{
			ret = lcyt_proc_set_work_mode_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
		case IA_AIRHEATER_YCYT_STATUS_GEAR: //设置温度
		{
			ret = lcyt_proc_set_gear_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
		
		case IA_AIRHEATER_YCYT_STATUS_ORDER_TIME:
		{
			ret = lcyt_proc_set_timer_ctrl(ac,ci->u.u16_data[0],LCYT_TIME_DEV_ON);
		}
			break;
		case IA_AIRHEATER_YCYT_STATUS_WORK:
		{
			ret = lcyt_proc_set_work_stat_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
		case IA_AIRHEATER_YCYT_STATUS_TIME:
		{
			ret = lcyt_proc_set_timer_ctrl(ac,ci->u.u16_data[0],LCYT_TIME_DEV_OFF);
		}
			break;
		case IA_AIRHEATER_YCYT_REFRESH_TIMER:
		{
			ret = lcyt_proc_refresh_timer(ac);
		}
			break;
		default:
			ret = RS_INVALID_PARAM;
			break;
	}

	return ret;
}

bool lcyt_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	switch (pkt->type) {
	case CLNE_IA_CTRL:
		*ret = _lcyt_proc_notify(pkt);
		break;

	default:
		return false;
	}

	return true;
}

void lcyt_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA,UCSOT_IA_HEATER_LCYT,0xFFFF,0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

void lcyt_build_objs(user_t* user,cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	
	if(!user || !ui || !user->is_udp_ctrl)
		return;
	
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || !udp_dev_type_equl(user,IJ_816,ETYPE_IJ_816_LCYT)) {
		return;
	}

	ui->ia_info.u.ptr = cl_calloc(sizeof(cl_ia_airheater_ycyt_info_t),1);
	if(!ui->ia_info.u.ptr){
		return;
	}

	memcpy(ui->ia_info.u.ptr, &ac->ia_lcyt_info, sizeof(cl_ia_airheater_ycyt_info_t));
	ui->ia_info.ia_type = user->sub_type;
	ui->ia_info.ia_sub_type = user->ext_type;
	
}

static bool lcyt_update_work_stat(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_work_stat_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_work_stat_t*)(obj+1);
	
	info->onoff = !!st->stat;
	
	return true;
}

static bool lcyt_update_work_mode(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_work_mode_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_work_mode_t*)(obj+1);
	
	info->mode = st->mode;
	
	return true;
}

static bool lcyt_update_gear(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_set_gear_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_set_gear_t*)(obj+1);
	
	info->gear = st->gear;
	
	return true;
}

static bool lcyt_update_cur_temp(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_cur_temp_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_cur_temp_t*)(obj+1);
	
	info->cur_temp = st->cur_temp;
	
	return true;
}

static bool lcyt_update_set_temp(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_set_temp_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_set_temp_t*)(obj+1);
	
	info->set_temp = st->set_temp;
	
	return true;
}

static bool lcyt_update_remain_time(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_remain_timer_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_remain_timer_t*)(obj+1);

//	info->time = info->time_on = 0;
	
	if(st->time_type == LCYT_TIME_DEV_ON){
		info->time_on = ntohs(st->remain_time);
	}else{
		info->time = ntohs(st->remain_time);
	}
	
	return true;
}

static bool lcyt_update_set_time(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_airheater_ycyt_info_t *info;
	lcyt_set_timer_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.lcyt_info;
	st = (lcyt_set_timer_t*)(obj+1);

//	info->time_set_on = info->time_set_off = 0;
	
	if(st->time_type == LCYT_TIME_DEV_ON){
		info->time_set_on = ntohs(st->time);
	}else{
		info->time_set_off = ntohs(st->time);
	}
	
	return true;
}


bool lcyt_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	if(!air_ctrl || !obj)
		return ret;
	
	switch (obj->attr) {
		case UCAT_HEATER_LCYT_WORK:
			ret = lcyt_update_work_stat(air_ctrl,action,obj);
			break;
		case UCAT_HEATER_LCYT_MODE:
			ret = lcyt_update_work_mode(air_ctrl,action,obj);
			break;
		case UCAT_HEATER_LCYT_GEAR:
			ret = lcyt_update_gear(air_ctrl,action,obj);
			break;
		case UCAT_HEATER_LCYT_CUR_TEMP:
			ret = lcyt_update_cur_temp(air_ctrl,action,obj);
			break;
		case UCAT_HEATER_LCYT_SET_TEMP:
			ret = lcyt_update_set_temp(air_ctrl,action,obj);
			break;
		case UCAT_HEATER_LCYT_REMAIN_TIME:
			ret = lcyt_update_remain_time(air_ctrl,action,obj);
			break;
		case UCAT_HEATER_LCYT_SET_TIME:
			ret = lcyt_update_set_time(air_ctrl,action,obj);
			break;
		
		default:
			break;
	}
	
	return ret;
}

int lcyt_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	
	switch(obj->attr){
		case UCAT_HEATER_LCYT_WORK:
			if(error == ERR_NONE)
				event = IE_AIRHEATER_YCYT_SET_ONOFF_OK;
			else
				event = IE_AIRHEATER_YCYT_SET_ONOFF_FAULT;			
			break;
			
		case UCAT_HEATER_LCYT_MODE:
			if(error == ERR_NONE)
				event = IE_AIRHEATER_YCYT_SET_MODE_OK;
			else
				event = IE_AIRHEATER_YCYT_SET_MODE_FAULT;
			break;
			
		case UCAT_HEATER_LCYT_GEAR:
			if(error == ERR_NONE)
				event = IE_AIRHEATER_YCYT_SET_GEAR_OK;
			else
				event = IE_AIRHEATER_YCYT_SET_GEAR_FAULT;
			break;
		case UCAT_HEATER_LCYT_SET_TEMP:
			if(error == ERR_NONE)
				event = IE_AIRHEATER_YCYT_SET_TEMP_OK;
			else
				event = IE_AIRHEATER_YCYT_SET_TEMP_FAULT;
			break;
		case UCAT_HEATER_LCYT_SET_TIME:
			if(error == ERR_NONE)
				event = IE_AIRHEATER_YCYT_SET_TIMER_OK;
			else
				event = IE_AIRHEATER_YCYT_SET_TIMER_FAULT;
			break;
			
		default:
			break;
	}
	
	return event;
}

////////////////////////////////////////////////////////
//海科空气净化器
static RS hkac_proc_set_work_ctrl(smart_air_ctrl_t* ac, u_int8_t work)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	hk_air_clean_work_t* st = (hk_air_clean_work_t*)(uo+1);
	user_t* user = ac->sac->user;

	st->is_on = !!work;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_HKAC, UCAT_HKAC_WORK, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS hkac_proc_set_mode_ctrl(smart_air_ctrl_t* ac, u_int8_t mode)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	hk_air_clean_mode_t* st = (hk_air_clean_mode_t*)(uo+1);
	user_t* user = ac->sac->user;
	//海科空气净化器只能轮转设置风速
	memset(st, 0, sizeof(*st));
	st->mode = mode;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_HKAC, UCAT_HKAC_MODE, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS hkac_proc_set_wind_ctrl(smart_air_ctrl_t* ac, u_int8_t wind)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	hk_air_clean_wind_t* st = (hk_air_clean_wind_t*)(uo+1);
	user_t* user = ac->sac->user;
	//海科空气净化器只能轮转设置风速
	memset(st, 0, sizeof(*st));
	st->wind = wind;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_HKAC, UCAT_HKAC_WIND, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}
static RS hkac_proc_set_anion_uvl_ctrl(smart_air_ctrl_t* ac, u_int8_t anion, u_int8_t uvl)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	hk_air_clean_anion_uvl_t* st = (hk_air_clean_anion_uvl_t*)(uo+1);
	user_t* user = ac->sac->user;
	//海科空气净化器负离子UVL只能轮转设置
	//这里还是把参数填上，如果电器支持了，就不用改了
	memset(st, 0, sizeof(*st));
	st->anion = anion;
	st->uvl = uvl;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_HKAC, UCAT_HKAC_ANION_UVL, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS hkac_proc_set_timer_ctrl(smart_air_ctrl_t* ac, u_int16_t timer)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	hk_air_clean_timer_t* st = (hk_air_clean_timer_t*)(uo+1);
	user_t* user = ac->sac->user;
	//海科空气净化器只能轮转设置定时器
	//set时全填0，设备按0(关闭)，1h，2h，4h，8h循环跳转
	memset(st, 0, sizeof(*st));
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_HKAC, UCAT_HKAC_TIMER, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}


static int _hkac_proc_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_common_info_t* ci;
	smart_air_ctrl_t* ac;
	RS ret = RS_OK;
	
	ci = (cln_common_info_t *)&pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->handle);
	if (!user || !user->is_udp_ctrl || !user->smart_appliance_ctrl) {
		return RS_INVALID_PARAM;
	}

	if(! user->online){
		return RS_OFFLINE;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	
	switch(ci->action)
	{
		case IA_AIRCLEANER_STATUS_WORK:
		{
			ret = hkac_proc_set_work_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
			
		case IA_AIRCLEANER_STATUS_MODE:
		{
			ret = hkac_proc_set_mode_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
		
		case IA_AIRCLEANER_STATUS_SPEED:
		{
			ret = hkac_proc_set_wind_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
			
		case IA_AIRCLEANER_STATUS_TIMER: 
		{
			ret = hkac_proc_set_timer_ctrl(ac,ci->u.u16_data[0]);
		}
			break;
		
		case IA_AIRCLEANER_STATUS_ULTRAVIOLET:
		{
			ret = hkac_proc_set_anion_uvl_ctrl(ac, 0xff, ci->u.u16_data[0]&0xff);
		}
			break;

		case IA_AIRCLEANER_STATUS_ANION:
		{
			ret = hkac_proc_set_anion_uvl_ctrl(ac, ci->u.u16_data[0]&0xff, 0xff);
		}
			break;

		case IA_AIRCLEANER_STATUS_QUERY:
		{
			ret = RS_OK;
			hkac_quick_query_info(ac);
		}
			break;
			
		default:
			ret = RS_INVALID_PARAM;
			break;
	}

	return ret;
}

bool hkac_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	switch (pkt->type) {
	case CLNE_IA_CTRL:
		*ret = _hkac_proc_notify(pkt);
		break;

	default:
		return false;
	}

	return true;
}

void hkac_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA, UCOT_IA_HKAC, 0xFFFF, 0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

void hkac_build_objs(user_t* user,cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	
	if(!user || !ui || !user->is_udp_ctrl)
		return;
	
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || !udp_dev_type_equl(user,IJ_813,ETYPE_IJ_813_HK)) {
		return;
	}

	ui->ia_info.u.ptr = cl_calloc(sizeof(cl_ia_aircleaner_info_t),1);
	if(!ui->ia_info.u.ptr){
		return;
	}

	memcpy(ui->ia_info.u.ptr, &ac->u.hkac_info, sizeof(cl_ia_aircleaner_info_t));
	ui->ia_info.ia_type = user->sub_type;
	ui->ia_info.ia_sub_type = user->ext_type;
	
}

bool hkac_update_work_stat(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_work_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_work_t*)(obj+1);
	
	info->onoff = !!st->is_on;
	
	return true;
}

bool hkac_update_work_mode(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_mode_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_mode_t*)(obj+1);
	
	info->work_mode = st->mode;
	
	return true;
}

bool hkac_update_wind(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_wind_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_wind_t*)(obj+1);
	
	info->speed = st->wind;
	
	return true;
}

bool hkac_update_temp(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_temp_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_temp_t*)(obj+1);
	
	info->temp = ntohs(st->temp);
	
	return true;
}

bool hkac_update_pm25(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_pm25_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_pm25_t*)(obj+1);
	
	info->pm25 = ntohs(st->pm25);
	
	return true;
}

bool hkac_update_humidity(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_humitity_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_humitity_t*)(obj+1);
	
	info->rh = ntohs(st->humidity);
	
	return true;
}

bool hkac_update_anion_uvl(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_anion_uvl_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_anion_uvl_t*)(obj+1);
	
	info->anion = !!st->anion;
	info->ultra = !!st->uvl;
	
	return true;
}

bool hkac_update_timer(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	hk_air_clean_timer_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (hk_air_clean_timer_t*)(obj+1);
	
	info->timer = ntohs(st->remain_minute);
	info->timer_type = st->timer_type;
	info->set_hour = st->set_hour;
	
	return true;
}

bool hkac_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	if(!air_ctrl || !obj)
		return ret;
	
	switch (obj->attr) {
		case UCAT_HKAC_WORK:
			ret = hkac_update_work_stat(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_MODE:
			ret = hkac_update_work_mode(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_WIND:
			ret = hkac_update_wind(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_TEMP:
			ret = hkac_update_temp(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_PM25:
			ret = hkac_update_pm25(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_HUMIDITY:
			ret = hkac_update_humidity(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_ANION_UVL:
			ret = hkac_update_anion_uvl(air_ctrl,action,obj);
			break;
		case UCAT_HKAC_TIMER:
			ret = hkac_update_timer(air_ctrl,action,obj);
			break;
		
		default:
			break;
	}
	
	return ret;
}

int hkac_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	
	switch(obj->attr){
		case UCAT_HKAC_WORK:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_ONOFF_OK;
			else
				event = IE_AIRCLEANER_SET_ONOFF_FAULT;
			break;
			
		case UCAT_HKAC_MODE:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_MODE_OK;
			else
				event = IE_AIRCLEANER_SET_MODE_FAULT;
			break;
			
		case UCAT_HKAC_WIND:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_SPEED_OK;
			else
				event = IE_AIRCLEANER_SET_SPEED_FAULT;
			break;
			
		case UCAT_HKAC_ANION_UVL:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_ANION_OK;
			else
				event = IE_AIRCLEANER_SET_ANION_FAULT;
			break;
			
		case UCAT_HKAC_TIMER:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_TIMER_OK;
			else
				event = IE_AIRCLEANER_SET_TIMER_FAULT;
			break;
			
		default:
			break;
	}
	
	return event;
}

static RS nbac_proc_set_work_ctrl(smart_air_ctrl_t* ac, u_int8_t work)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	nb_air_clean_work_t* st = (nb_air_clean_work_t*)(uo+1);
	user_t* user = ac->sac->user;

	st->is_on = !!work;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_WORK, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS nbac_proc_set_mode_ctrl(smart_air_ctrl_t* ac, u_int8_t mode)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	nb_air_clean_mode_t* st = (nb_air_clean_mode_t*)(uo+1);
	user_t* user = ac->sac->user;
	
	memset(st, 0, sizeof(*st));
	st->mode = mode;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_MODE, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS nbac_proc_set_wind_ctrl(smart_air_ctrl_t* ac, u_int8_t wind)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	nb_air_clean_wind_t* st = (nb_air_clean_wind_t*)(uo+1);
	user_t* user = ac->sac->user;

	memset(st, 0, sizeof(*st));
	st->wind = wind;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_WIND, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS nbac_proc_set_timer_ctrl(smart_air_ctrl_t* ac, u_int16_t timer)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	nb_air_clean_timer_t* st = (nb_air_clean_timer_t*)(uo+1);
	user_t* user = ac->sac->user;

	memset(st, 0, sizeof(*st));
	st->minute = ntohs(timer);
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_TIMER, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS nbac_proc_set_anion_ctrl(smart_air_ctrl_t* ac, u_int8_t anion)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	nb_air_clean_anion_t* st = (nb_air_clean_anion_t*)(uo+1);
	user_t* user = ac->sac->user;

	memset(st, 0, sizeof(*st));
	st->anion = anion;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_ANION, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS nbac_proc_set_terilize_ctrl(smart_air_ctrl_t* ac, u_int8_t is_on, u_int8_t minute)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	nb_air_clean_terilize_t* st = (nb_air_clean_terilize_t*)(uo+1);
	user_t* user = ac->sac->user;

	memset(st, 0, sizeof(*st));
	st->is_on = !!is_on;
	st->minute = minute;
	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_TERILIZE, sizeof(ucp_obj_t)+sizeof(*st));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*st));
	
	return RS_OK;
}

static RS nbac_proc_set_peridoc_timer_ctrl(smart_air_ctrl_t* ac, periodic_timer_t *ti)
{
    char buf[128] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_ac_timer_item_t* timer = (ucp_ac_timer_item_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_NBAC, UCAT_NBAC_PERIODIC_TIMER, sizeof(*timer));
    timer->id = ti->id;
    timer->enable = ti->enable;
    timer->hour = ti->hour;
    timer->minute = ti->minute;
    timer->onoff = ti->onoff?AC_POWER_ON:AC_POWER_OFF;
    timer->week = ti->week;
    airplug_timer_local_2_utc(timer,cl_priv->timezone);
    
    if (!ti->is_del) {
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,true,0x1, buf, sizeof(*uo)+sizeof(*timer));
    }else {
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,true,0x1, buf, sizeof(*uo)+sizeof(*timer));
    }
    
    return RS_OK;
}


static int _nbac_proc_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_common_info_t* ci;
	smart_air_ctrl_t* ac;
	RS ret = RS_OK;
	
	ci = (cln_common_info_t *)&pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->handle);
	if (!user || !user->is_udp_ctrl || !user->smart_appliance_ctrl) {
		return RS_INVALID_PARAM;
	}

	if(! user->online){
		return RS_OFFLINE;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	
	switch(ci->action)
	{
		case IA_AIRCLEANER_STATUS_WORK:
		{
			ret = nbac_proc_set_work_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
			
		case IA_AIRCLEANER_STATUS_MODE:
		{
			ret = nbac_proc_set_mode_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
		
		case IA_AIRCLEANER_STATUS_SPEED:
		{
			ret = nbac_proc_set_wind_ctrl(ac,ci->u.u16_data[0]&0xff);
		}
			break;
			
		case IA_AIRCLEANER_STATUS_TIMER: 
		{
			ret = nbac_proc_set_timer_ctrl(ac,ci->u.u16_data[0]);
		}
			break;

		case IA_AIRCLEANER_STATUS_ANION:
		{
			ret = nbac_proc_set_anion_ctrl(ac, ci->u.u16_data[0]&0xff);
		}
			break;
		
		case IA_AIRCLEANER_STATUS_TERILIZE:
		{
			//cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_TERILIZE, BUILD_U16(is_on, minute));
			ret = nbac_proc_set_terilize_ctrl(ac, (ci->u.u16_data[0]>>8)&0xff, 
				(ci->u.u16_data[0])&0xff);
		}
			break;
			
		case IA_AIRCLEANER_STATUS_PERIODIC_TIMER:
		{
			ret = nbac_proc_set_peridoc_timer_ctrl(ac, &ci->u.timer_info);
		}
			break;

		case IA_AIRCLEANER_STATUS_QUERY:
		{
			ret = RS_OK;
			nbac_quick_query_info(ac);
		}
			break;
			
		default:
			ret = RS_INVALID_PARAM;
			break;
	}

	return ret;
}

bool nbac_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	switch (pkt->type) {
	case CLNE_IA_CTRL:
		*ret = _nbac_proc_notify(pkt);
		break;

	default:
		return false;
	}

	return true;
}

void nbac_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA, UCOT_IA_NBAC, 0xFFFF, 0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

void nbac_build_objs(user_t* user,cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	cl_ia_aircleaner_info_t *upinfo;
	
	if(!user || !ui || !user->is_udp_ctrl)
		return;
	
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || !udp_dev_type_equl(user,IJ_813,ETYPE_IJ_813_NB)) {
		return;
	}

	ui->ia_info.u.ptr = cl_calloc(sizeof(cl_ia_aircleaner_info_t),1);
	if(!ui->ia_info.u.ptr){
		return;
	}
	upinfo = ui->ia_info.u.aircleaner_info;

	memcpy(upinfo, &ac->u.hkac_info, sizeof(cl_ia_aircleaner_info_t));
	air_timer_dup(&upinfo->periodic_timer ,&ac->air_info.air_timer_info);

	ui->ia_info.ia_type = user->sub_type;
	ui->ia_info.ia_sub_type = user->ext_type;
	
}

bool nbac_update_work_stat(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_work_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_work_t*)(obj+1);
	
	info->onoff = !!st->is_on;
	
	return true;
}

bool nbac_update_work_mode(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_mode_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_mode_t*)(obj+1);
	
	info->work_mode = st->mode;
	
	return true;
}

bool nbac_update_wind(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_wind_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_wind_t*)(obj+1);
	
	info->speed = st->wind;
	
	return true;
}

bool nbac_update_temp(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_temp_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_temp_t*)(obj+1);
	
	info->temp = ntohs(st->temp);
	
	return true;
}

bool nbac_update_pm25(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_pm25_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_pm25_t*)(obj+1);
	
	info->pm25= ntohs(st->pm25);
	
	return true;
}

bool nbac_update_humidity(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_humitity_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_humitity_t*)(obj+1);
	
	info->rh = ntohs(st->humidity);
	
	return true;
}

bool nbac_update_anion(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_anion_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_anion_t*)(obj+1);
	
	info->anion = st->anion;
	
	return true;
}

bool nbac_update_timer(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_timer_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_timer_t*)(obj+1);
	
	info->timer = ntohs(st->minute);
	info->timer_type = st->timer_type;
	
	return true;
}

bool nbac_update_terilize(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_terilize_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_terilize_t*)(obj+1);
	
	info->terilize = st->is_on;
	info->terilize_minute = st->minute;
	
	return true;
}

bool nbac_update_periodic_timer(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	if (obj->param_len >= sizeof(ucp_ac_timer_t) &&
	(obj->param_len-sizeof(ucp_ac_timer_t))%sizeof(ucp_ac_timer_item_t) == 0) {

		ac->air_info.air_timer_info.timer_count = 0;
		SAFE_FREE(ac->air_info.air_timer_info.timers);
		air_net_timer2_cl_timer(obj,&ac->air_info.air_timer_info);
		log_debug("Air timer OK\n");
	}
	return true;
}

bool nbac_update_rosebox_life(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	cl_ia_aircleaner_info_t *info;
	nb_air_clean_rosebox_life_t* st;
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.hkac_info;
	st = (nb_air_clean_rosebox_life_t*)(obj+1);
	
	info->rosebox_life = ntohl(st->minute);
	
	return true;
}

bool nbac_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	if(!air_ctrl || !obj)
		return ret;
	
	switch (obj->attr) {
		case UCAT_NBAC_WORK:
			ret = nbac_update_work_stat(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_MODE:
			ret = nbac_update_work_mode(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_WIND:
			ret = nbac_update_wind(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_TEMP:
			ret = nbac_update_temp(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_PM25:
			ret = nbac_update_pm25(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_HUMIDITY:
			ret = nbac_update_humidity(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_ANION:
			ret = nbac_update_anion(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_TERILIZE:
			ret = nbac_update_terilize(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_TIMER:
			ret = nbac_update_timer(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_PERIODIC_TIMER:
			ret = nbac_update_periodic_timer(air_ctrl,action,obj);
			break;
		case UCAT_NBAC_PERIODIC_ROSEBOX_LIFE:
			ret = nbac_update_rosebox_life(air_ctrl,action,obj);
			break;
		default:
			break;
	}
	
	return ret;
}

int nbac_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	
	switch(obj->attr){
		case UCAT_NBAC_WORK:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_ONOFF_OK;
			else
				event = IE_AIRCLEANER_SET_ONOFF_FAULT;
			break;
			
		case UCAT_NBAC_MODE:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_MODE_OK;
			else
				event = IE_AIRCLEANER_SET_MODE_FAULT;
			break;
			
		case UCAT_NBAC_WIND:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_SPEED_OK;
			else
				event = IE_AIRCLEANER_SET_SPEED_FAULT;
			break;
			
		case UCAT_NBAC_ANION:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_ANION_OK;
			else
				event = IE_AIRCLEANER_SET_ANION_FAULT;
			break;
			
		case UCAT_NBAC_TIMER:
		case UCAT_NBAC_PERIODIC_TIMER:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_TIMER_OK;
			else
				event = IE_AIRCLEANER_SET_TIMER_FAULT;
			break;
			
		case UCAT_NBAC_TERILIZE:
			if(error == ERR_NONE)
				event = IE_AIRCLEANER_SET_TERILIZE_OK;
			else
				event = IE_AIRCLEANER_SET_TERILIZE_FAULT;
			
		default:
			break;
	}
	
	return event;
}

static int _lamp_proc_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_common_info_t* ci;
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	lamp_gx_led_ctl_t *ctrl = (lamp_gx_led_ctl_t*)(uo+1);
	

	ci = (cln_common_info_t *)&pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->handle);
	if (!user || !user->is_udp_ctrl || !user->smart_appliance_ctrl) {
		return RS_INVALID_PARAM;
	}

	if(! user->online){
		return RS_OFFLINE;
	}

	if(ci->action == GX_LED_QUERY_ALL){
		smart_air_ctrl_t* ac;
		ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
		if(ac == NULL)
			return RS_ERROR;
		lamp_quick_query_info(ac);
		return RS_OK;
	}

	ctrl->action = ci->action;
	ctrl->param1 = (ci->u.u32_data[0]>>16)&0xff;
	ctrl->param2 = (ci->u.u32_data[0]>>8)&0xff;
	ctrl->param3 = ci->u.u32_data[0]&0xff;

	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_GX_LED, UCAT_GX_LED_STATUS, sizeof(ucp_obj_t)+sizeof(*ctrl));
	sa_set_obj_value(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*ctrl));
	return RS_OK;
	
}

bool lamp_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	switch (pkt->type) {
	case CLNE_IA_CTRL:
		*ret = _lamp_proc_notify(pkt);
		break;

	default:
		return false;
	}

	return true;
}

void lamp_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA, UCOT_IA_GX_LED, 0xFFFF, 0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}
void lamp_build_objs(user_t* user,cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	
	if(!user || !ui || !user->is_udp_ctrl)
		return;
	
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || !udp_dev_type_equl(user,IJ_820,ETYPE_IJ_820_GX)) {
		return;
	}

	ui->ia_info.u.ptr = cl_calloc(sizeof(cl_ia_gx_led_info_t),1);
	if(!ui->ia_info.u.ptr){
		return;
	}

	memcpy(ui->ia_info.u.ptr, &ac->u.gxled_info, sizeof(cl_ia_gx_led_info_t));
	ui->ia_info.ia_type = user->sub_type;
	ui->ia_info.ia_sub_type = user->ext_type;
}

bool lamp_update_work_stat(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	lamp_gx_led_stat_t *st;
	cl_ia_gx_led_info_t *info;
	
	          
	if (!is_valid_obj_data( obj, sizeof(*st)))
		return false;
	
	info = &ac->u.gxled_info;
	st = (lamp_gx_led_stat_t*)(obj+1);

	info->led_status = st->led_status;
	info->brightness = st->brightness;
	info->warmness = st->warmness;
	info->red = st->red;
	info->green = st->green;
	info->blue = st->blue;
	info->night_status = st->night_status;
	info->reserved = st->reserved;
	
	return true;
}


bool lamp_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	

	if(!air_ctrl || !obj)
		return ret;

	switch (obj->attr) {
		case UCAT_GX_LED_STATUS:
			ret = lamp_update_work_stat(air_ctrl, action, obj);
			break;

		default:
			break;
	}
	
	return ret;
}

int lamp_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	
	switch(obj->attr){
		case UCAT_GX_LED_STATUS:
			if(error == ERR_NONE)
				event = LPE_CTRL_OK;
			else
				event = LPE_CTRL_FAIL;
			break;
			
			
		default:
			break;
	}
	
	return event;
}
//彩虹电热毯

static void blanket_query_area_info(u_int8_t area_num,ucc_session_t *s)
{
	ucp_obj_t stat_obj = {UCOT_IA, UCOT_IA_CH_BLANKET, 0xFFFF, 0};

	if(!s){
		return;
	}
	
	if(area_num == AREA_BLANKET_LEFT){
		stat_obj.attr = UCAT_CH_QUERY_LEFT;
	}else if(area_num == AREA_BLANKET_RIGHT){
		stat_obj.attr = UCAT_CH_QUERY_RIGHT;
	}else{
		return;
	}

	sa_query_objects(s, &stat_obj, 0x1);
}

static int _blanket_proc_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_common_info_t* ci;
	char buf[256] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	blanket_ch_stat_pkt_t *ctrl = (blanket_ch_stat_pkt_t*)(uo+1);
	int data_len = 0;
	

	ci = (cln_common_info_t *)&pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->handle);
	if (ci->action >= CH_BLANKET_ACTION_MAX ||!user || !user->is_udp_ctrl || !user->smart_appliance_ctrl) {
		return RS_INVALID_PARAM;
	}

	if(! user->online){
		return RS_OFFLINE;
	}

	ctrl->cmd_type = ci->action;
	ctrl->area_num = ci->u.u8_data[0];
	if(ctrl->area_num > AREA_BLANKET_RIGHT){
		return RS_INVALID_PARAM;
	}
	switch(ci->action){
		case CH_BLANKET_QUERY_ALL:
			blanket_query_area_info(ctrl->area_num,user->uc_session);
			return RS_OK;
			break;
		case CH_BLANKET_ON_OFF:
			ctrl->work_stat = !!ci->u.u8_data[1];
			break;
		case CH_BLANKET_CURVE_ON_OFF:
			ctrl->curve_enable = !!ci->u.u8_data[1];
			break;
		case CH_BLANKET_CONFIG_CURVE:
			if(ci->data_len-1 > MAX_CURVE_DATA_NUM){
				return RS_INVALID_PARAM;
			}
			memcpy(ctrl->curve_data,&ci->u.u8_data[1],ci->data_len-1);
			ctrl->curve_data_len = htons(ci->data_len-1);
			data_len = ci->data_len-1;
			ctrl->curve_week = 0x7F;//每天
			if(data_len == 24){
				ctrl->curve_time_interval = htons(60);
			}else if(data_len == 48){
				ctrl->curve_time_interval = htons(30);
			}
			break;
		case CH_BLANKET_AREA_TEMP:
			ctrl->set_temperature = ci->u.u8_data[1];
			break;
		case CH_BLANKET_CONFIG_TIMER:
			ctrl->off_timer = ci->u.u8_data[1];
			break;
        case CH_BLANKET_SET_MODE:
            ctrl->work_mode  = ci->u.u8_data[1];
            break;
		default:
			log_err(false,"_blanket_proc_notify action error[%u]\n",ctrl->cmd_type);
			return RS_INVALID_PARAM; //正常不会到这里
			break;
	}
    
    if (ctrl->work_mode == BLANKET_MODE_KNOWN || ctrl->work_mode > BLANKET_MODE_SLEEP) {
        ctrl->work_mode = BLANKET_MODE_SLEEP;
    }

	fill_net_ucp_obj(uo, UCOT_IA, UCOT_IA_CH_BLANKET, UCAT_CH_STATUS, (u_int16_t)(sizeof(ucp_obj_t)+sizeof(*ctrl)+data_len));
	sa_set_obj_value_only(user->uc_session,0x1,uo, sizeof(ucp_obj_t)+sizeof(*ctrl)+data_len);
	blanket_query_area_info(ctrl->area_num,user->uc_session);
	
	return RS_OK;
	
}

bool ch_blanket_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	switch (pkt->type) {
	case CLNE_IA_CTRL:
		*ret = _blanket_proc_notify(pkt);
		break;

	default:
		return false;
	}

	return true;
}

void ch_blanket_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA, UCOT_IA_CH_BLANKET, 0xFFFF, 0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

void ch_blanket_build_objs(user_t* user,cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	
	if(!user || !ui || !user->is_udp_ctrl)
		return;
	
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || !udp_dev_type_equl(user,IJ_821,ETYPE_IJ_821_CH)) {
		return;
	}

	ui->ia_info.u.ptr = cl_calloc(sizeof(cl_ia_ch_blanket_info_t),1);
	if(!ui->ia_info.u.ptr){
		return;
	}

	memcpy(ui->ia_info.u.ptr, &ac->u.ch_blanket_info, sizeof(cl_ia_ch_blanket_info_t));
	ui->ia_info.ia_type = user->sub_type;
	ui->ia_info.ia_sub_type = user->ext_type;
}

static bool ch_blanket_update_work_stat(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	blanket_ch_stat_pkt_t *st;
	cl_ia_ch_blanket_info_t*blanket;
	cl_ia_ch_area_info_t* info;
	
	          
	if (obj->param_len < sizeof(*st))
		return false;
	
	blanket = &ac->u.ch_blanket_info;
	st = (blanket_ch_stat_pkt_t*)(obj+1);

	if(st->area_num == AREA_BLANKET_LEFT){
		info = &blanket->left_area_info;
	}else if(st->area_num == AREA_BLANKET_RIGHT){
		info = &blanket->right_area_info;
	}else{
		return false;
	}
	st->curve_data_len = ntohs(st->curve_data_len);
	
	info->work_stat = !!st->work_stat;
	info->set_temperature = st->set_temperature;
	info->current_temperature = st->current_temperature;
	info->off_timer = st->off_timer;
	info->curve_enable = !!st->curve_enable;
	info->curve_week = st->curve_week;
    info->work_mode = st->work_mode;
	info->curve_time_interval = ntohs(st->curve_time_interval);
	info->curve_next_work_time = ntohs(st->curve_next_work_time);
	if(st->curve_data_len <= MAX_CURVE_DATA_NUM){
		info->curve_data_len = st->curve_data_len&0xFF;
		if(info->curve_data_len > 0){
			memcpy(&info->curve_data[0],&st->curve_data[0],info->curve_data_len);
		}else{
			memset(&info->curve_data[0],0,sizeof(info->curve_data));
		}
	}

	return true;
}

bool ch_blanket_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	

	if(!air_ctrl || !obj)
		return ret;

	switch (obj->attr) {
		case UCAT_CH_STATUS:
		case UCAT_CH_QUERY_LEFT:
		case UCAT_CH_QUERY_RIGHT:
			ret = ch_blanket_update_work_stat(air_ctrl, action, obj);
			break;

		default:
			break;
	}
	return ret;
}

int ch_blanket_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	
	switch(obj->attr){
		case UCAT_CH_STATUS:
			if(error == ERR_NONE){
				event = SAE_COMMON_CTRL_OK;
			}else{
				event = SAE_COMMON_CTRL_FAILED;
			}
			break;
			
		default:
			break;
	}
	
	return event;
}



