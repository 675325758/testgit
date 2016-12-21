/*
	本文件用于实现EB一代相关功能
*/
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_eb.h"
#include "eb_priv.h"

extern void airplug_timer_local_2_utc(ucp_ac_timer_item_t *timer, int zone);

static bool eb_update_work(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	net_eb_work_t *neb_work;
	cl_ia_eb_info_t *info;
	
	          
	if (!is_valid_obj_data( obj, sizeof(*neb_work)))
		return false;
	
	info = &ac->u.eb_info;
	neb_work = (net_eb_work_t*)(obj+1);

	log_info("%s, EB update work from %d to %d\n", __FUNCTION__, info->on_off, neb_work->on_off);
	
	info->on_off = neb_work->on_off;

	info->on_off_valid = 1;
	
	return true;
}

static bool eb_update_timer(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	ucp_ac_timer_t *hdr;
	cl_ia_eb_info_t *info;
	
	info = &ac->u.eb_info;
	hdr = (ucp_ac_timer_t *)(obj + 1);


	if (obj->param_len >= sizeof(ucp_ac_timer_t)
			&& (obj->param_len-sizeof(ucp_ac_timer_t))%sizeof(ucp_ac_timer_item_t) == 0)
	{
		info->timer.timer_count = 0;
		SAFE_FREE(info->timer.timers);
		SAFE_FREE(info->timer.period_timers);
		air_net_timer2_cl_timer(obj, &info->timer);
		log_debug("EB timer OK\n");
	}
	
	return true;
}

bool eb_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	
	if (air_ctrl == NULL || obj == NULL)
		return ret;

	switch (obj->attr) {
	case UCAT_EB_WORK:
		ret = eb_update_work(air_ctrl, action, obj);
		break;

	case UCAT_EB_TIMER:
		if(!air_ctrl->air_info.is_support_peroid_timer){
			ret = eb_update_timer(air_ctrl, action, obj);
		}
		break;

	default:
		break;
	}
	
	return ret;
}

static void eb_set_work(user_t *user, u_int8_t on_off)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_eb_work_t* w = (net_eb_work_t*)(uo+1);

	log_debug("try turn %s EB now...\n", on_off ? "ON" : "OFF");
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EB, UCAT_EB_WORK, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	w->on_off = on_off ? AC_POWER_ON : AC_POWER_OFF;
	
	sa_set_obj_value(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+sizeof(*w));
}

static void eb_set_timer(user_t *user, periodic_timer_t *t)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_ac_timer_item_t* w = (ucp_ac_timer_item_t *)(uo + 1);

	log_debug("try set EB timer now: id=%u, enable=%d, week=0x%02x, hour=%u, minute=%u, turn %s, repeat=%d\n",
		t->id, t->enable, t->week, t->hour, t->minute, t->onoff ? "ON" : "OFF", t->repeat);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EB, UCAT_EB_TIMER, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	w->id = t->id;
	w->enable = t->enable;
	w->week = t->week;
	w->hour = t->hour;
	w->minute = t->minute;
	w->onoff = t->onoff ? AC_POWER_ON : AC_POWER_OFF;
	w->repeat = t->repeat;
	airplug_timer_local_2_utc(w, cl_priv->timezone);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo)+sizeof(*w));
}

static void eb_del_timer(user_t *user, u_int8_t id)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *mem;
	int n = 4;

	log_debug("try delete EB timer id=%u\n", id);
	
	mem = (u_int8_t*)(uo + 1);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EB, UCAT_EB_TIMER, (u_int16_t)(sizeof(ucp_obj_t) + n));
	memset(mem, 0, n);
	mem[0] = id;
	
	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+n);
}

static void eb_set_period_timer(user_t *user, cl_period_timer_t *t)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_period_timer_t* w = (net_period_timer_t *)(uo + 1);

	log_debug("try set EB timer now: id=%u, enable=%d, week=0x%02x, hour=%u, minute=%u, turn %s, duration=%d\n",
		t->id, t->enable, t->week, t->hour, t->minute, t->onoff ? "ON" : "OFF", t->duration);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_PERIOD_TIMER, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	w->id = t->id;
	w->enable = t->enable;
	w->week = t->week;
	w->hour = t->hour;
	w->minute = t->minute;
	w->onoff = t->onoff ? AC_POWER_ON : AC_POWER_OFF;
	w->duration = htons(t->duration);
	period_timer_local_2_utc(w, cl_priv->timezone);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo)+sizeof(*w));
}

static void eb_del_period_timer(user_t *user, u_int8_t id)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *mem;
	int n = 4;

	log_debug("try delete EB timer id=%u\n", id);
	
	mem = (u_int8_t*)(uo + 1);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_PERIOD_TIMER, (u_int16_t)(sizeof(ucp_obj_t) + n));
	memset(mem, 0, n);
	mem[0] = id;
	
	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+n);
}

static void eb_set_pt_adj(user_t *user, cl_plug_pt_adkb_t *t)
{
	char buf[128] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_plug_pt_adkb_t* w = (cl_plug_pt_adkb_t *)(uo + 1);
	fill_net_ucp_obj(uo, UCOT_PLUG_DEVPRO, UCSOT_PLUG_DEVPRO_AV, UCAT_AV_ADJUST, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	w->freq_current = htonl(t->freq_current);
	w->k_current = htonl(t->k_current);
	w->freq_voltage = htonl(t->freq_voltage);
	w->k_voltage = htonl(t->k_voltage);
	printf("eb_set_pt_adj: freq_current:%u, k_current:%u, freq_voltage:%u, k_voltage:%u\n", 
		t->freq_current, t->k_current, t->freq_voltage, t->k_voltage);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo)+sizeof(*w));
}

static void eb_del_pt_adj(user_t *user, cl_plug_pt_adkb_t *t)
{
	char buf[128] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_plug_pt_adkb_t* w = (cl_plug_pt_adkb_t *)(uo + 1);

	fill_net_ucp_obj(uo, UCOT_PLUG_DEVPRO, UCSOT_PLUG_DEVPRO_AV, UCAT_AV_ADJUST, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));

	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+sizeof(*w));
}

static bool eb_proc_led_ctrl(user_t *user,bool onoff)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_led_t* led = (ucp_led_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED, sizeof(*led));
    led->user_enable = onoff;
    
    sa_set_obj_value(user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*led));
    
    return true;
}

/*
	处理APP下来的用户请求
*/
bool eb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
    cl_plug_pt_adkb_t* pa;

	info = (cln_common_info_t *)&pkt->data[0];
	
	switch(info->action){
	case IA_EB_WORK:
		eb_set_work(user, info->u.u8_data[0]);
		break;
	case IA_EB_TIMER_SET:
		eb_set_timer(user, &info->u.timer_info);
		break;
	case IA_EB_TIMER_DEL:
		eb_del_timer(user, info->u.timer_info.id);
		break;
	case IA_EB_PERIOD_TIMER_SET:
		eb_set_period_timer(user,&info->u.period_timer_info);
		break;
	case IA_EB_PERIOD_TIMER_DEL:
		eb_del_period_timer(user, info->u.timer_info.id);
		break;
	case IA_EB_PT_ADJ_SET:
        pa = cci_pointer_data(info);
		eb_set_pt_adj(user,pa);
		break;	
	case IA_EB_PT_ADJ_DEL:
        pa = cci_pointer_data(info);
		eb_del_pt_adj(user,pa);
        break;
    case IA_EB_LED_CTRL:
        eb_proc_led_ctrl(user,cci_u8_data(info));
        break;
	default:
		res = false;
		break;
	}
	return res;
}

// APP能看见的状态数据
void eb_build_objs(user_t* user, cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	cl_ia_eb_info_t *info;

	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || user->sub_type != IJ_101) {
		return;
	}

	info = cl_calloc(sizeof(cl_ia_eb_info_t), 1);
	if (!info) {
		return;
	}

    
	memcpy(info, &ac->u.eb_info, sizeof(cl_ia_eb_info_t));
	info->on_off = (info->on_off == AC_POWER_ON  ? true : false) ;
	info->is_support_period_timer = ac->air_info.is_support_peroid_timer;
	if(!info->is_support_period_timer){
		air_timer_dup(&info->timer, &ac->u.eb_info.timer);
	}else{
		air_timer_dup(&info->timer, &ac->air_info.air_timer_info);
	}
	if(ac->com_udp_dev_info.is_suppport_elec_stat){
		info->elec_info.is_support_elec_info = true;
		info->elec_info.current_power = ac->com_udp_dev_info.current_power;
		info->elec_info.current_mil_power = ac->com_udp_dev_info.cur_milli_power;
		memcpy(&info->elec_info.elec_stat_info,&ac->com_udp_dev_info.elec_stat_info,sizeof(info->elec_info.elec_stat_info));
		memcpy(&info->elec_info.total_elec,&ac->com_udp_dev_info.total_elec,sizeof(info->elec_info.total_elec));
		memcpy(&info->elec_info.period_elec,&ac->com_udp_dev_info.period_elec,sizeof(info->elec_info.period_elec));
		memcpy(&info->elec_info.last_on_elec,&ac->com_udp_dev_info.last_on_elec,sizeof(info->elec_info.last_on_elec));
		memcpy(&info->elec_info.elec_days_info,&ac->com_udp_dev_info.elec_days_info,sizeof(info->elec_info.elec_days_info));
		info->elec_info.elec_days_info.days_count  = 0;
		info->elec_info.elec_days_info.elec_data = NULL;
		if(ac->com_udp_dev_info.elec_days_info.days_count > 0){
			info->elec_info.elec_days_info.elec_data = cl_calloc(sizeof(u_int16_t)*ac->com_udp_dev_info.elec_days_info.days_count,1);
			if(info->elec_info.elec_days_info.elec_data != NULL){
				memcpy(info->elec_info.elec_days_info.elec_data,ac->com_udp_dev_info.elec_days_info.elec_data,sizeof(u_int16_t)*ac->com_udp_dev_info.elec_days_info.days_count);
				info->elec_info.elec_days_info.days_count  = ac->com_udp_dev_info.elec_days_info.days_count;
			}
		}
	}
    
    info->led_onoff = ac->air_info.air_led_on_off;
	
	
	ui->eb_info = info;
}

int eb_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;

	log_info("%s, attr=%u, error=%d\n", __FUNCTION__, obj->attr, error);
	
	switch(obj->attr){
	case UCAT_EB_WORK:
		if (error == ERR_NONE)
			event = EBE_SET_WORK_OK;
		else
			event = EBE_SET_WORK_FAULT;
		break;

	case UCAT_EB_TIMER:
		if (error == ERR_NONE)
			event = EBE_SET_TIMER_OK;
		else
			event = EBE_SET_TIMER_FAULT;
		break;
		
	default:
		break;
	}
	
	return event;
}


void eb_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA, UCSOT_IA_EB, 0xFFFF, 0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
	 if(ac->air_info.is_support_peroid_timer){
	  	log_debug("eb_quick_query_info query public info\n");
		stat_objs[0].sub_objct = UCSOT_IA_PUBLIC;
		stat_objs[0].attr= UCAT_IA_PUBLIC_PERIOD_TIMER;
	  	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
	  }
    
    if(ac->com_udp_dev_info.is_suppport_elec_stat){
        stat_objs[0].sub_objct = UCSOT_IA_COMMON_STAT;
        stat_objs[0].attr= ALL_SUB_ATTR;
        sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
        //单独获取每天的电量统计
        stat_objs[0].sub_objct = UCSOT_IA_COMMON_STAT;
        stat_objs[0].attr= UCAT_STAT_DAYS_STAT;
        sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
    }
}


