#include "cl_priv.h"
#include "cl_sys.h"
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_eb.h"
#include "eb_priv.h"
#include "udp_device_common_priv.h"
#include "cl_jcx_power_box.h"
#include "cl_lede_lamp.h"
#include "cl_tb_heater_pump.h"
#include "cl_yl_thermostat.h"
#include "cl_jnb_device.h"
#include "cl_amt_device.h"
#include "amt_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "chiffo_scm_ctrl.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"
#include "cl_hxpbj.h"
#include "cl_tl_temp.h"
#include "cl_qpcp.h"
#include "qpcp_priv.h"
#include "cl_ia.h"
#include "cl_car.h"
#include "cl_eplug_oem.h"
#include "rfgw_priv.h"
#include "cl_xy.h"
#include "xy_priv.h"
#include "cl_tbb.h"
#include "tbb_priv.h"
#include "cl_bimar_dev.h"
#include "bimar_scm_ctrl.h"
#include "cl_yt.h"
#include "zh_jl_lamp_ctrl.h"
#include "cl_ads.h"
#include "cl_js_wave.h"
#include "yt_priv.h"
#include "kxm_scm_ctrl.h"
#include "sbt_ther_scm_ctrl.h"
#include "yj_heater_scm_ctrl.h"
#include "zssx_priv.h"
#include "evm_scm_ctrl.h"
#include "cl_indiacar.h"
#include "cl_linkon.h"
#include "cl_zkrsq.h"
#include "zkrsq_priv.h"
#include "yt_priv.h"
#include "uas_client.h"
#include "ica_priv.h"
#include "cl_rfgw.h"
#include "rfgw_scm_ctrl.h"
#include "cl_zhcl.h"
#include "lanusers_priv.h"
#include "cl_leis.h"
#include "leis_scm_ctrl.h"
#include "cl_yinsu.h"
#include "yinsu_scm_ctrl.h"
#include "cl_zhdhx.h"
#if 0
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif


enum{
    RC_STOP_MATCH = 0x0,
    RC_CLOUD_MATCH ,
    RC_ALL_MATCH ,
    RC_MATCH_OK ,
    RC_MATCH_FAILED,
    RC_MATCH_NEXT_KEY = 0x7
};

enum{
    RC_CMS_IDLE,
    RC_CMS_START_ALL_MATCH,
    RC_CMS_START_CLOUD_MATCH,
    RC_CMS_WAIT_USER_IR_SIGNAL,
    RC_CMS_WAIT_CLOUD_MATCH_RESULT,
    RC_CMS_WAIT_ALL_MATCH_RESULT
};

static void rc_code_match_switch_to_status(smart_air_ctrl_t* ac,u_int32_t status);
extern slave_t * _find_slave_at_list(u_int64_t sn, struct stlc_list_head *head);
extern bool air_ir_get_ir_detail_code(slave_t* slave,u_int8_t* code,int* codelen);
extern bool hpgw_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj);

static void ntoh_u16_array(u_int16_t*  array,u_int32_t count)
{
	u_int32_t i = 0;

	if(!array){
		return;
	}

	for(;i < count ; i++){
		array[i] = ntohs(array[i]);
	}
}

static void ntoh_u32_array(u_int32_t*  array,u_int32_t count)
{
	u_int32_t i = 0;

	if(!array){
		return;
	}

	for(;i < count ; i++){
		array[i] = ntohl(array[i]);
	}
}

bool is_supported_udp_device(u_int8_t sub_type, u_int8_t ext_type)
{
	bool ret = false;
	
	switch(sub_type){
		case IJ_SMART_PLUG:
		case IJ_822:
		case IJ_823:
		case IJ_824:
		case IJ_830:
		case IJ_840:
		case IJ_AMT:
		case IJ_CHIFFO:
		case IJ_EVM:
		case IJ_RFGW:
		case IJ_TL_TEMP:
		case IJ_QPCP:
        case IJ_QP_POT:
		case IJ_101:
        case IJ_101_OEM:
		case IJ_838_YT:
		case IJ_839_ADS:
		case IJ_INDIACAR:
		case IJ_ZHCL:
		case IJ_LEIS:
		case IJ_ZHDHX:
		case IJ_HOMESERVER:
			ret = true;
			break;
        case IJ_HXPBJ:
        {
            switch (ext_type) {
                case ETYPE_IJ_HX_PBJ:
                case ETYPE_IJ_HX_POT:
                    ret = true;
                    break;
                default:
                    break;
            }
        }
            break;
        case IJ_KXM_DEVICE:
        {
            switch (ext_type) {
                case ETYPE_IJ_KXM_HOST:
                case ETYPE_IJ_KXM_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB:
				case ETYPE_IJ_XY_THERMOSTAT:
                case ETYPE_IJ_SBT_THER:
				case EYTPE_IJ_ZSSX_FURN:
				case EYTYP_IJ_KXM_AC:
				case ETYPE_IJ_YJ_HEATER:
				case ETYPE_IJ_LINKON_THERMOSTAT:
                    ret = true;
                    break;
                default:
                    break;
            }
        }
            break;
        case IJ_TEST_DEV:
        {
            switch (ext_type) {
                case ETYPE_IJ_TEST_CAR_WK:
                case ETYPE_IJ_TEST_XY:
                case ETYPE_IJ_TEST_BITMAR:
                    ret = true;
                    break;
                default:
                    break;
            }
        }
            break;
        case IJ_HEATER_DEV:
        {
            switch (ext_type) {
                case ETYPE_IJ_HEATER_BIMAR_C2000:
                case ETYPE_IJ_HEATER_AKM_0505L:
                case ETYPE_IJ_HEATER_AKM_5162L:
                    ret = true;
                    break;
                    
                default:
                    break;
            }
        }
            break;
	case IJ_JL_STAGE_LAMP:
		switch (ext_type) {
                case ETYPE_JL_STAGE_LAMP:
                    ret = true;
                    break;
                    
                default:
                    break;
            }
		break;
    case IJ_JS_MICWAVE:
        switch (ext_type) {
            case ETYPE_IJ_JS_MICWARE:
            case ETYPE_IJ_JS_MIC_BARBECUE:
            case ETYPE_IJ_JS_ONLY_MIC:
                ret = true;
                break;
                
            default:
                break;
        }
        break;
		default:
			break;
	}
	
	return ret;
}

bool udp_init_ac_hook(user_t* user,smart_air_ctrl_t* ac)
{
	void* dev_infop = NULL;

	if(!ac || !user){
		return false;
	}
	
	switch(user->sub_type){
		case IJ_SMART_PLUG:			
			dev_infop = cl_calloc(1, 1);
			break;
		case IJ_INDIACAR:
			dev_infop = cl_calloc(sizeof(cl_indiacar_info_t),1);
			break;
		case IJ_822:
			dev_infop = cl_calloc(sizeof(cl_jnb_thermotat_info),1);
			break;
		case IJ_823:
			dev_infop = cl_calloc(sizeof(cl_yl_thermostat_info), 1);
			break;
		case IJ_824:
			switch (user->ext_type) {
			case ETYPE_IJ_824_HTC_BUSINESS:
			case TYPE_IJ_824_YCJ:
				dev_infop = cl_calloc(sizeof(cl_tbb_info_t), 1);
				break;
			case ETYPE_IJ_824_ZKRSQ:
				dev_infop = cl_calloc(sizeof(cl_zkrsq_info_t), 1);
				break;
			default:
				dev_infop = cl_calloc(sizeof(cl_tb_info_t), 1);
				break;
			}
			break;
		case IJ_830:
			dev_infop = cl_calloc(sizeof(cl_lede_lamp_info), 1);
			break;
		case IJ_840:
			dev_infop = cl_calloc(sizeof(cl_jcx_power_box_info), 1);
			break;
		case IJ_AMT:
			//艾美特全是透传
			dev_infop = cl_calloc(sizeof(amt_fan_priv_data_t), 1);
			user->is_dev_support_scm = true;
			break;
		case IJ_CHIFFO:
			dev_infop = cl_calloc(sizeof(chiffo_priv_data_t), 1);
			user->is_dev_support_scm = true;
			break;
		case IJ_EVM:
			// dev_infop 在第一次收到状态后初始化
			user->is_dev_support_scm = true;
			goto done;
		case IJ_RFGW:
			dev_infop = init_rfgw_sdk();
			user->is_dev_support_scm = true;
			break;
		case IJ_HXPBJ:
            switch (user->ext_type) {
                case ETYPE_IJ_HX_POT:
                    dev_infop = qpcp_priv_init();
                    break;
                case ETYPE_IJ_HX_PBJ:
                    dev_infop = cl_calloc(sizeof(cl_hx_info), 1);
                    break;
                    
                default:
                    break;
            }
			break;
		case IJ_TL_TEMP:
			dev_infop  = cl_calloc(sizeof(cl_tl_info_t),1);
			break;
        case IJ_KXM_DEVICE:
        {
            switch (user->ext_type) {
                case ETYPE_IJ_KXM_HOST:
				case EYTYP_IJ_KXM_AC:
                    dev_infop = cl_calloc(sizeof(cl_kxm_info_t), 1);
                    user->is_dev_support_scm = true;
                    break;
                case ETYPE_IJ_KXM_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB:
                    dev_infop = cl_calloc(sizeof(cl_kxm_thermost_info_t), 1);
                    break;
				case ETYPE_IJ_XY_THERMOSTAT:
					dev_infop = cl_calloc(sizeof(cl_xy_info_t), 1);
					break;
                case ETYPE_IJ_SBT_THER:
                    dev_infop = cl_calloc(sizeof(cl_sbt_ther_info_t), 1);
                    user->is_dev_support_scm = true;
                    break;
				case ETYPE_IJ_YJ_HEATER:
					dev_infop = cl_calloc(sizeof(cl_yj_heater_info_t), 1);
                    user->is_dev_support_scm = true;
					break;
				case EYTPE_IJ_ZSSX_FURN:
					dev_infop = cl_calloc(sizeof(ucp_zssx_priv_t), 1);
					break;
				case ETYPE_IJ_LINKON_THERMOSTAT:
					dev_infop = cl_calloc(sizeof(cl_linkon_info_t), 1);
					break;
                default:
                    break;
            }
        }
            break;
		case IJ_QPCP:
        case IJ_QP_POT:
			dev_infop  = qpcp_priv_init();
			break;
		case IJ_TEST_DEV:
			switch(user->ext_type) {
			case ETYPE_IJ_TEST_CAR_WK:
				dev_infop = cl_calloc(sizeof(cl_car_info_t), 1);
				break;
			case ETYPE_IJ_TEST_XY:
				dev_infop = cl_calloc(sizeof(cl_xy_info_t), 1);
				break;
            case ETYPE_IJ_TEST_BITMAR:
                dev_infop = cl_calloc(sizeof(cl_bimar_heater_info_t), 0x1);
                user->is_dev_support_scm  = true;
                break;
			default:
				break;
			}
			break;
        case IJ_HEATER_DEV:
            switch (user->ext_type) {
                case ETYPE_IJ_HEATER_BIMAR_C2000:
                case ETYPE_IJ_HEATER_AKM_0505L:
                case ETYPE_IJ_HEATER_AKM_5162L:
                    dev_infop = cl_calloc(sizeof(cl_bimar_heater_info_t), 0x1);
                    user->is_dev_support_scm  = true;
                    break;
                    
                default:
                    break;
            }
            break;
	case IJ_JL_STAGE_LAMP:
		switch (user->ext_type) {
                case ETYPE_JL_STAGE_LAMP:
                     dev_infop = cl_calloc(sizeof(cl_jl_lamp_info_t), 1);
			user->is_dev_support_scm = true;
                    break;
                    
                default:
                    break;
            }
		break;
		case IJ_101:
			dev_infop = cl_calloc(1, 1);
			break;
        case IJ_101_OEM:
            dev_infop = cl_calloc(sizeof(cl_eplug_oem_stat), 1);
            break;
		case IJ_838_YT:
			dev_infop = cl_calloc(sizeof(cl_yt_info_t), 1);
			break;
		case IJ_839_ADS:
			dev_infop = cl_calloc(sizeof(cl_ads_info_t), 1);
			break;
        case IJ_JS_MICWAVE:
            switch (user->ext_type) {
                case ETYPE_IJ_JS_MICWARE:
                case ETYPE_IJ_JS_MIC_BARBECUE:
                case ETYPE_IJ_JS_ONLY_MIC:
                    dev_infop = cl_calloc(sizeof(cl_js_wave_info_t), 1);
                    break;
                    
                default:
                    break;
            }
            break;
		case IJ_ZHCL:
			dev_infop = cl_calloc(sizeof(cl_zhcl_info_t), 1);
			break;
		case IJ_LEIS:
			switch (user->ext_type) {
				case ETYPE_LEIS_DEFAULT:
					dev_infop = cl_calloc(1, sizeof(cl_leis_info_t));
					user->is_dev_support_scm = true;
					break;
				case ETYPE_LEIS_YINSU:
					dev_infop = cl_calloc(1, sizeof(cl_yinsu_info_t));
					user->is_dev_support_scm = true;
					break;
				default:
					break;
			}
			
			break;
		case IJ_ZHDHX:
			dev_infop = cl_calloc(sizeof(cl_zhdhx_info_t), 1);
			break;
		case IJ_HOMESERVER:
			dev_infop = cl_calloc(sizeof(u_int8_t), 1);
		default:
			break;
	}

	if(dev_infop == NULL){
		return false;
	}

done:
	ac->com_udp_dev_info.sub_type = user->sub_type;
	ac->com_udp_dev_info.ext_type = user->ext_type;
	ac->com_udp_dev_info.device_info = dev_infop;
	
	return true;
}

//辅助函数，查询单个对象
static RS udp_query_single_object(ucc_session_t *s,u_int16_t obj,u_int16_t sub_obj, u_int16_t attr)
{
	ucp_obj_t stat_objs[1] = {0};

	stat_objs[0].objct = obj;
	stat_objs[0].sub_objct = sub_obj;
	stat_objs[0].attr = attr;
	
	return sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 处理定时器

static void udp_set_timer(user_t *user, periodic_timer_t *t)
{
//	char buf[64] = {0};
//	ucp_obj_t* uo = (ucp_obj_t*)buf;
//	ucp_ac_timer_item_t* w = (ucp_ac_timer_item_t *)(uo + 1);

//	log_debug("try set EB timer now: id=%u, enable=%d, week=0x%02x, hour=%u, minute=%u, turn %s, repeat=%d\n",
//		t->id, t->enable, t->week, t->hour, t->minute, t->onoff ? "ON" : "OFF", t->repeat);

//	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EB, UCAT_EB_TIMER, sizeof(ucp_obj_t)+sizeof(*w));
//	memset(w, 0, sizeof(*w));
//	w->id = t->id;
//	w->enable = t->enable;
//	w->week = t->week;
//	w->hour = t->hour;
//	w->minute = t->minute;
//	w->onoff = t->onoff ? AC_POWER_ON : AC_POWER_OFF;
//	w->repeat = t->repeat;
//	airplug_timer_local_2_utc(w, cl_priv->timezone);
//	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo)+sizeof(*w));
}

static void udp_del_timer(user_t *user, u_int8_t id)
{
//	char buf[64] = {0};
//	ucp_obj_t* uo = (ucp_obj_t*)buf;
//	u_int8_t *mem;
//	int n = 4;

//	log_debug("try delete EB timer id=%u\n", id);
//	
//	mem = (u_int8_t*)(uo + 1);

//	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EB, UCAT_EB_TIMER, (u_int16_t)(sizeof(ucp_obj_t) + n));
//	memset(mem, 0, n);
//	mem[0] = id;
//	
//	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+n);
}

static void udp_set_period_timer(user_t *user, cl_period_timer_t *t)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_period_timer_t* w = (net_period_timer_t *)(uo + 1);

	log_debug("try set udp timer now: id=%u, enable=%d, week=0x%02x, hour=%u, minute=%u, turn %s, duration=%d\n",
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

static bool udp_ext_period_timer_pq_check(user_t *user,smart_air_ctrl_t* ac,  cln_common_info_t *info)
{
#if 0
	int i;
	cl_period_timer_t* period_timers;
#endif
	cl_air_timer_info_t *pati = &ac->air_info.air_timer_info;
	cl_period_timer_t* t_src= (cl_period_timer_t*)&info->u.u8_data[0];


	//添加定时器时
	if (t_src->id == 0) {
		if (pati->timer_count >= ID_TIMER_MAX) {
			event_push(user->callback, SAE_TIMER_ID_MAX, user->handle, user->callback_handle);
	        event_cancel_merge(user->handle);
			return false;
		}

		return true;
	} 
#if 0
	//判断一下是否有时间冲突,定时器是时间点定时器
	for(i = 0; i < pati->timer_count; i++) {
		period_timers = &pati->period_timers[i];
		if (period_timers->id == t_src->id) {
			continue;
		}

		if (period_timers->hour == t_src->hour &&
			period_timers->minute == t_src->minute) {
			if (period_timers->week == 0 ||
				t_src->week == 0 ||
				(period_timers->week && t_src->week &&
				period_timers->week & t_src->week)) {
				event_push(user->callback, SAE_TIMER_TIME_CONFLICT, user->handle, user->callback_handle);
		        event_cancel_merge(user->handle);
				return false;
			}
		}
	}
#endif	
	
	return true;	
}

static bool udp_ext_period_timer_check(user_t *user,smart_air_ctrl_t* ac,  cln_common_info_t *info)
{
	bool ret = true;
    cl_period_timer_t* t_src;

	if (!user || !info || !ac) {
		return true;
	}

    t_src = (cl_period_timer_t*)&info->u.u8_data[0];

	switch(t_src->ext_data_type) {
	 case PT_EXT_DT_QPCP: //千帕茶盘
		ret = udp_ext_period_timer_pq_check(user, ac, info);
	 	break;
	 default:
		break;
	}

	return ret;
}

static void udp_set_ext_period_timer(user_t *user, smart_air_ctrl_t* ac,cln_common_info_t *info)
{
    char buf[128] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_period_timer_t* w = (net_period_timer_t *)(uo + 1);
    cl_period_timer_t* t_src;
    cl_qpcp_scp_t* t_scp;
    int len = 0;
    t_src = (cl_period_timer_t*)&info->u.u8_data[0];
    t_scp = (cl_qpcp_scp_t*)(t_src+1);
    
    
    w->id = t_src->id;
    w->enable = t_src->enable;
    w->week = t_src->week;
    w->hour = t_src->hour;
    w->minute = t_src->minute;
    w->onoff = t_src->onoff ? AC_POWER_ON : AC_POWER_OFF;
    w->duration = htons(t_src->duration);
	
    len = udp_fill_ext_peroid_timer_modify_pkt(ac,t_src,t_scp,w+1);
    if (len < 0) {
        log_err(false, "udp_set_ext_period_timer udp_fill_ext_peroid_timer_modify_pkt ret = %d\n",len);
        return;
    }
    period_timer_local_2_utc(w, cl_priv->timezone);
    w->ext_data_len = htonl(len);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_EXT_PERIOD_TIMER, (u_int16_t)(sizeof(*uo)+sizeof(*w)+len));
    
    sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo)+sizeof(*w)+len);
}

static void udp_del_period_timer(user_t *user, u_int16_t attr,u_int8_t id)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *mem;
	int n = 4;

	log_debug("try delete Period timer id=%u\n", id);
	
	mem = (u_int8_t*)(uo + 1);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, attr, (u_int16_t)(sizeof(ucp_obj_t) + n));
	memset(mem, 0, n);
	mem[0] = id;
	
	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+n);
}

static bool _udp_proc_timer_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    
    info = (cln_common_info_t *)&pkt->data[0];
    
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac || (user->sub_type != IJ_808 && !ac->com_udp_dev_info.device_info)) {
        log_err(false, "air_proc_notify error handle %08x\n",info->handle);
        *ret = RS_INVALID_PARAM;
        return false;
    }

	
	
	switch(info->action){
	case ACT_UDP_COM_TIMER_SET:
		udp_set_timer(user, &info->u.timer_info);
		break;
	case ACT_UDP_COM_TIMER_DEL:
		udp_del_timer(user, info->u.timer_info.id);
		break;
	case ACT_UDP_COM_PEROID_TIMER_SET:
        if (ac->com_udp_dev_info.is_support_ext_period_timer) {
            *ret = RS_INVALID_PARAM;
        }else{
            udp_set_period_timer(user,&info->u.period_timer_info);
        }
		
		break;
	case ACT_UDP_COM_PEROID_TIMER_DEL:
        if (ac->com_udp_dev_info.is_support_ext_period_timer) {
            udp_del_period_timer(user, UCAT_IA_PUBLIC_EXT_PERIOD_TIMER,info->u.timer_info.id);
        }else{
            udp_del_period_timer(user, UCAT_IA_PUBLIC_PERIOD_TIMER,info->u.timer_info.id);
        }
		break;
	case ACT_UDP_COM_TIMER_REFRESH:
        if (ac->com_udp_dev_info.is_support_ext_period_timer) {
            udp_query_single_object(user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_EXT_PERIOD_TIMER);
        }else{
            udp_query_single_object(user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_PERIOD_TIMER);
        }
		break;
    case ACT_UDP_COM_EXT_PEROID_TIMER_SET:
		//先检查一下有无各种错误什么的
		if (udp_ext_period_timer_check(user,ac,info)) {
        	udp_set_ext_period_timer(user,ac,info);
		}
        break;
	default:
		res = false;
		break;
	}
	return res;
}

static bool udp_quick_query_dev_priv_info(smart_air_ctrl_t* ac)
{
	bool res = true;
	
	switch(ac->sac->user->sub_type){
		case IJ_INDIACAR:
			udp_query_single_object(ac->sac->user->uc_session, UCOT_IA,UCSOT_IA_INDIA_CAR, ALL_SUB_ATTR);
			break;
		case IJ_EVM:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_TT,UCAT_IA_TT_ALLSTATE);
			break;
		case IJ_822:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_JNB_THERMOSTAT,ALL_SUB_ATTR);
			break;
		case IJ_823:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_THERMOSTAT_YL,ALL_SUB_ATTR);
			break;
		case IJ_824:
			switch(ac->sac->user->ext_type) {
			case ETYPE_IJ_824_ZKRSQ:
				udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_ZKRSQ,ALL_SUB_ATTR);
				break;
			default:
				//log_debug("ALL_SUB_ATTR query !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
				udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_TBHEATER,ALL_SUB_ATTR);
				break;
			}
			break;
		case IJ_830:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_LEDE_LED,ALL_SUB_ATTR);
			break;
		case IJ_840:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_JCX,ALL_SUB_ATTR);
			break;
		case IJ_AMT:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_TT,UCAT_IA_TT_ALLSTATE);
			break;
		case IJ_HXPBJ:
            switch (ac->sac->user->ext_type) {
                case ETYPE_IJ_HX_POT:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_HX_YS_POT,ALL_SUB_ATTR);
                    break;
                case ETYPE_IJ_HX_PBJ:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_BREAK_MACHINE,ALL_SUB_ATTR);
                    break;
                    
                default:
                    break;
            }
			
			break;
		case IJ_TL_TEMP:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_TL_TEMP,ALL_SUB_ATTR);
			break;
		case IJ_QPCP:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_TEA_TRAY_QP,ALL_SUB_ATTR);
			break;
        case IJ_QP_POT:
        {
            switch(ac->sac->user->ext_type) {
                case ETYPE_IJ_QP_POT:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_QP_POT,ALL_SUB_ATTR);
                    break;
                case ETYPE_IJ_QP_PBJ:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_QP_PBJ,ALL_SUB_ATTR);
                    break;
                default:
                    res = false;
            }
        }
            
            break;
		case IJ_TEST_DEV:
			switch(ac->sac->user->ext_type) {
                case ETYPE_IJ_TEST_CAR_WK:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_CAR_WUKONG,ALL_SUB_ATTR);
                    break;
                case ETYPE_IJ_TEST_XY:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_THERMOSTAT_XY,ALL_SUB_ATTR);
                    break;
                default:
                    res = false;
			}
            break;
		case IJ_KXM_DEVICE:
			switch(ac->sac->user->ext_type) {
                case ETYPE_IJ_XY_THERMOSTAT:
                    //udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_THERMOSTAT_XY,ALL_SUB_ATTR);
                    
					udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_THERMOSTAT_XY,UCAT_IA_THERMOSTAT_PUSH);
					udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_THERMOSTAT_XY,UCAT_IA_THERMOSTAT_TIME);
					udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_THERMOSTAT_XY,UCAT_IA_THERMOSTAT_SMART_HOME);
                    break;
                case ETYPE_IJ_KXM_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_KXM_THER,ALL_SUB_ATTR);
                    break;
				case EYTPE_IJ_ZSSX_FURN:
					udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_ELEC_HEATER,ALL_SUB_ATTR);
					break;
				case ETYPE_IJ_LINKON_THERMOSTAT:
					udp_query_single_object(ac->sac->user->uc_session, UCOT_IA, UCSOT_IA_LINKONWKQ ,ALL_SUB_ATTR);
					break;
                default:
                    res = false;
                    break;
			}
			break;
        case IJ_101_OEM:
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_EPLUG_OEM,ALL_SUB_ATTR);
            break;
		case IJ_838_YT:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_YUETU,ALL_SUB_ATTR);
			break;
		case IJ_839_ADS:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_ADS,ALL_SUB_ATTR);
			break;
        case IJ_JS_MICWAVE:
            switch(ac->sac->user->ext_type) {
                case ETYPE_IJ_JS_MICWARE:
                case ETYPE_IJ_JS_MIC_BARBECUE:
                case ETYPE_IJ_JS_ONLY_MIC:
                    udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_JS_MICWAVE,ALL_SUB_ATTR);
                    break;
                default:
                    res = false;
            }
            break;
		case IJ_ZHCL:
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_ZHCL,ALL_SUB_ATTR);
			break;
		case IJ_ZHDHX:
			{
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_ZHDHX,ALL_SUB_ATTR);
			}
			break;
		case IJ_HOMESERVER:
			{
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_HOMESERVER,ALL_SUB_ATTR);
			}
			break;
		default:
			res = false;
			break;
	}
	
	return res;
}

static bool udp_proc_peak_period(smart_air_ctrl_t* ac,cln_common_info_t* info,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_stat_peak_time_t* period = (ucp_stat_peak_time_t*)(uo+1);
	u_int32_t value = cci_u32_data(info);
	u_int16_t begin,end;
	begin = (value >> 16)&0xFFFF;
	end = value &0xFFFF;
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_PEAK_TIME, sizeof(*period));
    period->begin_minute = htons(begin);
    period->last_minute = htons(end);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*period));
    
    return true;
}

static bool udp_proc_valley_period(smart_air_ctrl_t* ac,cln_common_info_t* info,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_stat_peak_time_t* period = (ucp_stat_peak_time_t*)(uo+1);
    u_int32_t value = cci_u32_data(info);
	u_int16_t begin,end;
	begin = (value >> 16)&0xFFFF;
	end = value &0xFFFF;
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_VALLEY_TIME, sizeof(*period));
    period->begin_minute = htons(begin);
    period->last_minute = htons(end);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*period));
    
    return true;
}

static bool udp_proc_peak_price(smart_air_ctrl_t* ac,cln_common_info_t* info,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int32_t* price = (u_int32_t*)(uo+1);
    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_PEAK_PRICE, sizeof(*price));
    *price = htonl(cci_u32_data(info));
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*price));
    
    return true;
}

static bool udp_proc_valley_price(smart_air_ctrl_t* ac,cln_common_info_t* info,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int32_t* price = (u_int32_t*)(uo+1);
    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_VALLEY_PRICE, sizeof(*price));
    *price = htonl(cci_u32_data(info));
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*price));
    
    return true;
}

static bool udp_proc_flat_price(smart_air_ctrl_t* ac,cln_common_info_t* info,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int32_t* price = (u_int32_t*)(uo+1);
    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_FLAT_PRICE, sizeof(*price));
    *price = htonl(cci_u32_data(info));
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*price));
    
    return true;
}

static void udp_clear_all_elec_data(smart_air_ctrl_t* ac)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    uc_tlv_t* tlv = (uc_tlv_t*)(uo+1);
    int len;
    
    len = sizeof(*tlv)+sizeof(u_int32_t);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_ON_USER_SET, len);
    tlv->type = htons(TLV_2_2_13_ALL_ELE_CLEAR);
    tlv->len = htons(sizeof(u_int32_t));
    
    *((u_int8_t*)(tlv+1)) = 0x1;
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
}

static void udp_clear_elec_stat_info(smart_air_ctrl_t* ac,u_int32_t type)
{
	 char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ia_stat_net_t* isn = (ia_stat_net_t*)(uo+1);
   u_int16_t attr = UCAT_STAT_PHASE_ELE;

	if(type == ELEC_CLEAR_TOTAL){
		attr = UCAT_STAT_TOTAL_ELE;
	}else if(type == ELEC_CLEAR_LAST_ON){
	      attr = UCAT_STAT_ON_ELE;
    }else if(type == ELEC_CLEAR_ALL_ELEC){
        udp_clear_all_elec_data(ac);
        return ;
    }
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, attr, sizeof(*isn));
    isn->begin_time = (u_int32_t)time(NULL);
    isn->begin_time = htonl(isn->begin_time);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*isn));
}

static void _udp_del_dev_err_info(smart_air_ctrl_t* ac,u_int32_t err_id)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int32_t* value = (u_int32_t*)(uo+1);

	if (ac->com_udp_dev_info.is_support_dev_err_info == 1) {
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_EXCEPTION, sizeof(u_int32_t));
	} else if (ac->com_udp_dev_info.is_support_dev_err_info == 2) { 		
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_EXCEPTION_V2, sizeof(u_int32_t));
	}

	*value = htonl(err_id);

	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,true,0x1, buf, sizeof(*uo)+sizeof(u_int32_t));
}

static void _udp_set_permit_stm_upgrade(smart_air_ctrl_t* ac)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_stm_upgrade_ctrl_t* stc = (ucp_stm_upgrade_ctrl_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_HARDWARE_STM_UPGRADE, sizeof(u_int32_t));
	//0x2 :允许单片机升级
	stc->up_type = 0x2;
	sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*stc));	
}

static void _udp_dev_restory_factory(smart_air_ctrl_t* ac)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	char* value = (char*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_RESTORY_FACTORY, sizeof(u_int32_t));
	*value =0x1;

	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,true,0x1, buf, sizeof(*uo)+sizeof(u_int32_t));
}

static void _udp_set_dev_ssid_and_password(smart_air_ctrl_t* ac,cl_notify_pkt_t *pkt)
{
	char buf[256] = {0};
	cln_common_info_t *info;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	uc_tlv_t * tlv;
	char* p,*src;
	int len = 0;
	info = (cln_common_info_t *)&pkt->data[0];
	src = (char*)(&info->u.u8_data[0]);
	
	tlv = (uc_tlv_t *)(uo+1);
	len += sizeof(tlv);
	p = (char*)(tlv+1);
	tlv->type = htons(0x1);
	tlv->len = htons(src[0]);
	strcpy(p,&src[2]);
	len+= src[0];
	
	tlv = (uc_tlv_t *)(p+src[0]);
	len += sizeof(tlv);
	p = (char*)(tlv+1);
	tlv->type = htons(0x2);
	tlv->len = htons(src[1]);
	if(src[1] > 0 ){
		strcpy(p,&src[128]);
	}
	len+= src[1];

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_WIFI_SETTING, len);
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,false,0x1, buf, sizeof(*uo)+len);
}

static void _udp_porc_adjust_room_temp(smart_air_ctrl_t* ac,int16_t temp)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    uc_tlv_t* tlv = (uc_tlv_t*)(uo+1);
    int8_t t_temp = (int8_t)temp;
    int len;
    
    len = sizeof(*tlv)+sizeof(u_int32_t);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_ON_USER_SET, len);
    tlv->type = htons(TLV_2_2_13_ROOM_TEMP_AJUST);
    tlv->len = htons(sizeof(u_int32_t));
    
    *((u_int8_t*)(tlv+1)) = (u_int8_t)t_temp;
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
}

static void _udp_porc_adjust_elec_value(smart_air_ctrl_t* ac, int16_t value)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    uc_tlv_t* tlv = (uc_tlv_t*)(uo+1);
    int8_t t_value = (int8_t)value;
    int len;
    
    len = sizeof(*tlv)+sizeof(u_int32_t);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_ON_USER_SET, len);
    tlv->type = htons(TLV_2_2_13_ELE_AJUST);
    tlv->len = htons(sizeof(u_int32_t));
    
    *((u_int8_t*)(tlv+1)) = (u_int8_t)t_value;
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
}

static bool _udp_proc_common_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
    u_int8_t u8_value;
    ucc_session_t* s = user->uc_session;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    
	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	switch(info->action){
	case ACT_UDP_COM_REFRESH_ALL_STAT:
		udp_quick_query_dev_priv_info(ac);
		break;
	case ACT_UDP_COM_SET_PEAK_TIME:
		udp_proc_peak_period(ac,info,ret);
		break;
	case ACT_UDP_COM_SET_VALLEY_TIME:
		udp_proc_valley_period(ac,info,ret);
		break;
	case ACT_UDP_COM_SET_PEAK_PRICE:
		udp_proc_peak_price(ac,info,ret);
		break;
	case ACT_UDP_COM_SET_VALLEY_PRICE:
		udp_proc_valley_price(ac,info,ret);
		break;
	case ACT_UDP_COM_SET_FLAT_PRICE:
		udp_proc_flat_price(ac,info,ret);
		break;
	case ACT_UDP_COM_REFRESH_ELEC_STAT:
		udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_COMMON_STAT,ALL_SUB_ATTR);
		udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_COMMON_STAT,UCAT_STAT_DAYS_STAT);
		break;
	case ACT_UDP_COM_CLEAR_ELEC_STAT:
		udp_clear_elec_stat_info(ac,cci_u8_data(info));
		break;
	case ACT_UDP_COM_CLEAR_DEV_ERR_INFO:
		_udp_del_dev_err_info(ac,cci_u32_data(info));
		break;
	case ACT_UDP_COM_REFRESH_DEV_ERR_INFO:
		if (ac->com_udp_dev_info.is_support_dev_err_info == 1) {
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_EXCEPTION);
		} else if (ac->com_udp_dev_info.is_support_dev_err_info == 2) {			
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_EXCEPTION_V2);
		}
		break;
	case ACT_UDP_COM_SET_PEMIT_STM_UPGRADE:
		if(ac->com_udp_dev_info.is_support_stm_upgrade){
			_udp_set_permit_stm_upgrade(ac);
			ac->stat.stm_can_update = false;
		}else{
			res  = false;
			*ret = RS_INVALID_PARAM;
		}
		break;
	case ACT_UDP_COM_RESTORY_FACTORY:
		_udp_dev_restory_factory(ac);
		break;
	case ACT_UDP_COM_SETTING_SSID_PASSWD:
		_udp_set_dev_ssid_and_password(ac,pkt);
		break;
    case ACT_UDP_COM_AJUST_ROOM_TEMP:
        _udp_porc_adjust_room_temp(ac,(int16_t)cci_u16_data(info));
        break;
    case ACT_UDP_COM_AJUST_ELEC_VALUE:
        _udp_porc_adjust_elec_value(ac,(int16_t)cci_u16_data(info));
        break;
    case ACT_UDP_COM_REFRESH_24HOUR_LINE:
        u8_value = cci_u8_data(info);
        if ( !u8_value ) {
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_AC,UCAT_AC_TMP_SAMPLE_CURVE);
        }else{
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_AC,UCAT_AC_RH_SAMPLE_CURVE);
        }
        break;
    case ACT_UDP_COM_REQUEST_SHARE_CODE:
        if (!s->has_share_key || s->phone_index != SUPER_PHONE_INDEX) {
            *ret = RS_INVALID_PARAM;
            break;
        }
        udp_query_single_object(s,UCOT_SYSTEM,UCSOT_SYS_DEV_SHARE,UCAT_SYS_CODE);
        break;
    case ACT_UDP_COM_DEL_SHARED_PHONE:
        if (!s->has_share_key || s->phone_index != SUPER_PHONE_INDEX) {
            *ret = RS_INVALID_PARAM;
            break;
        }
        fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_DEV_SHARE, UCAT_SYS_CODE,sizeof(u_int32_t));
        *((u_int32_t*)(uo+1)) = htonl(cci_u32_data(info));
        sa_ctrl_obj_value(s, UCA_DELETE, false, 0x1, uo, sizeof(*uo)+sizeof(u_int32_t));
		udp_query_single_object(s, UCOT_SYSTEM, UCSOT_SYS_DEV_SHARE, UCAT_PHONE_LIST);
        break;
    case ACT_UDP_COM_REFRESH_SHARED_LIST:
        if (!s->has_share_key || s->phone_index != SUPER_PHONE_INDEX) {
            *ret = RS_INVALID_PARAM;
            break;
        }
        udp_query_single_object(s, UCOT_SYSTEM, UCSOT_SYS_DEV_SHARE, UCAT_PHONE_LIST);
        break;
	case ACT_UDP_COM_MODFIY_SHARED_PHONE:
		{
		ucp_share_desc_info_t *pnet = NULL;
		u_int8_t *tmp;
		
        if (!s->has_share_key || s->phone_index != SUPER_PHONE_INDEX) {
            *ret = RS_INVALID_PARAM;
            break;
        }
		pnet = (ucp_share_desc_info_t *)(uo+1);
		tmp = cci_pointer_data(info);
		memcpy((void *)pnet, tmp, sizeof(ucp_share_desc_info_t));
		pnet->index = htonl(pnet->index);
		pnet->desc[15] = 0;
		fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_DEV_SHARE, UCAT_PHONE_LIST,sizeof(ucp_share_desc_info_t));
		sa_ctrl_obj_value(s, UCA_SET, true, 0x1, uo, sizeof(*uo)+sizeof(ucp_share_desc_info_t));
		}
		break;
	default:
		res = false;
		*ret = RS_INVALID_PARAM;
		break;
	}
	return res;
}

static bool _udp_common_update_elec_stat_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	switch(obj->attr){
		case UCAT_STAT_MONTH_PEAK:
			 if (obj->param_len > sizeof(u_int32_t) && obj->param_len%sizeof(u_int32_t) == 0) {
                 memcpy(&air_ctrl->com_udp_dev_info.elec_stat_info.month_peak[0],(void*)(obj+1),sizeof(air_ctrl->com_udp_dev_info.elec_stat_info.month_peak));
               ntoh_u32_array(&air_ctrl->com_udp_dev_info.elec_stat_info.month_peak[0],MONTH_PER_YEAR);
            }
			break;
		case UCAT_STAT_MONTH_VALLEY:
			if (obj->param_len > sizeof(u_int32_t) && obj->param_len%sizeof(u_int32_t) == 0) {
				memcpy(&air_ctrl->com_udp_dev_info.elec_stat_info.month_valley[0],(void*)(obj+1),sizeof(air_ctrl->com_udp_dev_info.elec_stat_info.month_valley));
	                   ntoh_u32_array(&air_ctrl->com_udp_dev_info.elec_stat_info.month_valley[0],MONTH_PER_YEAR);
	                }else{
				return false;
			}
			break;
		case UCAT_STAT_MONTH_FLAT:
			if (obj->param_len > sizeof(u_int32_t) && obj->param_len%sizeof(u_int32_t) == 0) {
				memcpy(&air_ctrl->com_udp_dev_info.elec_stat_info.month_normal[0],(void*)(obj+1),sizeof(air_ctrl->com_udp_dev_info.elec_stat_info.month_normal));
	                   ntoh_u32_array(&air_ctrl->com_udp_dev_info.elec_stat_info.month_normal[0],MONTH_PER_YEAR);
	                }else{
				return false;
			}
			break;
		case UCAT_STAT_CUR_POWER:
			 if (is_valid_obj_data(obj, sizeof(u_int32_t))){
				 air_ctrl->com_udp_dev_info.current_power = ntohl(obj_u32_value(obj));
			}else{
				return false;
			}
			break;
		case UCAT_STAT_MILI_POWER:
			if (is_valid_obj_data(obj, sizeof(u_int32_t))){
				 air_ctrl->com_udp_dev_info.cur_milli_power = ntohl(obj_u32_value(obj));
			}else{
				return false;
			}
			break;
		case UCAT_STAT_PEAK_TIME:
		{
			cl_peak_time_t* cpt;
            if (is_valid_obj_data(obj, sizeof(cl_peak_time_t))) {
                cpt = (cl_peak_time_t*)(obj+1);
                air_ctrl->com_udp_dev_info.elec_stat_info.peak_time.begin_minute = ntohs(cpt->begin_minute);
                air_ctrl->com_udp_dev_info.elec_stat_info.peak_time.last_minute = ntohs(cpt->last_minute);
            }else{
				return false;
			}
			break;
		}
		case UCAT_STAT_VALLEY_TIME:
		{
			cl_peak_time_t* cpt;
            if (is_valid_obj_data(obj, sizeof(cl_peak_time_t))) {
                cpt = (cl_peak_time_t*)(obj+1);
                air_ctrl->com_udp_dev_info.elec_stat_info.valley_time.begin_minute = ntohs(cpt->begin_minute);
                air_ctrl->com_udp_dev_info.elec_stat_info.valley_time.last_minute = ntohs(cpt->last_minute);
            }else{
				return false;
			}
			break;
		}
		case UCAT_STAT_FLAT_TIME:
		{
			cl_peak_time_t* cpt;
            if (is_valid_obj_data(obj, sizeof(cl_peak_time_t))) {
                cpt = (cl_peak_time_t*)(obj+1);
                air_ctrl->com_udp_dev_info.elec_stat_info.flat_time.begin_minute = ntohs(cpt->begin_minute);
                air_ctrl->com_udp_dev_info.elec_stat_info.flat_time.last_minute = ntohs(cpt->last_minute);
            }else{
				return false;
			}
			break;
		}
		case UCAT_STAT_PEAK_PRICE:
			if (is_valid_obj_data(obj, sizeof(u_int32_t))){
				 air_ctrl->com_udp_dev_info.elec_stat_info.peak_price = ntohl(obj_u32_value(obj));
			}else{
				return false;
			}
			break;
		case UCAT_STAT_VALLEY_PRICE:
			if (is_valid_obj_data(obj, sizeof(u_int32_t))){
				 air_ctrl->com_udp_dev_info.elec_stat_info.valley_price = ntohl(obj_u32_value(obj));
			}else{
				return false;
			}
			break;
		case UCAT_STAT_FLAT_PRICE:
			if (is_valid_obj_data(obj, sizeof(u_int32_t))){
				 air_ctrl->com_udp_dev_info.elec_stat_info.flat_price = ntohl(obj_u32_value(obj));
			}else{
				return false;
			}
			break;
		case UCAT_STAT_TOTAL_ELE:
			if (is_valid_obj_data(obj, sizeof(ia_stat_net_t))) {
				ia_stat_net_t* elec_stat = (ia_stat_net_t*)(obj+1);
				air_ctrl->com_udp_dev_info.total_elec.begin_time = ntohl(elec_stat->begin_time);
				air_ctrl->com_udp_dev_info.total_elec.elec = ntohl(elec_stat->ele);
			}else{
				return false;
			}
			break;
		case UCAT_STAT_PHASE_ELE:
			if (is_valid_obj_data(obj, sizeof(ia_stat_net_t))) {
				ia_stat_net_t* elec_stat = (ia_stat_net_t*)(obj+1);
				air_ctrl->com_udp_dev_info.period_elec.begin_time = ntohl(elec_stat->begin_time);
				air_ctrl->com_udp_dev_info.period_elec.elec = ntohl(elec_stat->ele);
			}else{
				return false;
			}
			break;
		case UCAT_STAT_ON_ELE:
			if (is_valid_obj_data(obj, sizeof(ia_stat_net_t))) {
				ia_stat_net_t* elec_stat = (ia_stat_net_t*)(obj+1);
				air_ctrl->com_udp_dev_info.last_on_elec.begin_time = ntohl(elec_stat->begin_time);
				air_ctrl->com_udp_dev_info.last_on_elec.elec = ntohl(elec_stat->ele);
			}else{
				return false;
			}
			break;
		case UCAT_STAT_DAYS_STAT:
			if(obj->param_len >= sizeof(ucp_elec_days_stat) ){
				ucp_elec_days_stat* ds =  (ucp_elec_days_stat*)(obj+1);
				air_ctrl->com_udp_dev_info.elec_days_info.is_info_valid = true;
				air_ctrl->com_udp_dev_info.elec_days_info.days_count = 0;
				air_ctrl->com_udp_dev_info.elec_days_info.nearest_data_time = ntohl(ds->nearest_data_time);
				ds->days_count = ntohs(ds->days_count);
				SAFE_FREE(air_ctrl->com_udp_dev_info.elec_days_info.elec_data);
				if(ds->days_count > 0 && (obj->param_len - sizeof(ucp_elec_days_stat))/sizeof(u_int16_t) == ds->days_count){
					air_ctrl->com_udp_dev_info.elec_days_info.elec_data = cl_calloc(obj->param_len - sizeof(ucp_elec_days_stat),1);
					if(air_ctrl->com_udp_dev_info.elec_days_info.elec_data != NULL){
						memcpy(air_ctrl->com_udp_dev_info.elec_days_info.elec_data,&ds->elec_data[0],obj->param_len - sizeof(ucp_elec_days_stat));
						air_ctrl->com_udp_dev_info.elec_days_info.days_count = ds->days_count;
						ntoh_u16_array(air_ctrl->com_udp_dev_info.elec_days_info.elec_data,ds->days_count);
					}
				}
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}


static bool _udp_update_rc_pan(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
//    int len = obj->param_len;
    ucp_rc_info_t * info;
    ucp_rc_info_head_t* head;
    cl_rc_info * dest = NULL;
    u_int8_t i,*src;
    
    head = OBJ_VALUE(obj, ucp_rc_info_head_t*);
    if (head->item_size <sizeof(*info)) {
        return false;
    }
    
    if (head->num == 0) {
        
        memset(ac->rc_pm.pair_rc.name, 0, sizeof(ac->rc_pm.pair_rc.name));
        
        dest = &ac->rc_pm.pair_rc.stb_info;
        
        SAFE_FREE(dest->uk);
        SAFE_FREE(dest->fk);
        memset(dest, 0, sizeof(*dest));
        
        dest = &ac->rc_pm.pair_rc.tv_info;
        
        SAFE_FREE(dest->uk);
        SAFE_FREE(dest->fk);
        
        memset(dest, 0, sizeof(*dest));
        
        return true;
    }
    
    src = (u_int8_t*)(head+1);
    for (i = 0; i < head->num; i++) {
        info = (ucp_rc_info_t *)src;
        
        if (info->type == RC_TYPE_STB) {
            dest = &ac->rc_pm.pair_rc.stb_info;
        }else if(info->type == RC_TYPE_TV){
            dest = &ac->rc_pm.pair_rc.tv_info;
        }else{
            return false;
        }
        
        dest->d_id = info->rc_id;
        dest->is_matched = !!(info->stat_flag & BIT(0));
        dest->matched_ir_id = ntohs(info->ir_id);
        memcpy(dest->name, info->name, sizeof(dest->name)-1);
        if (info->type == RC_TYPE_STB) {
            memcpy(ac->rc_pm.pair_rc.name, dest->name, sizeof(dest->name));
        }
        
        src = src + head->item_size;
    }
    
    if (!ac->rc_pm.is_query_key_info) {
        udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_CODE,UCAT_CODE_RC_KEY);
        ac->rc_pm.is_query_key_info = true;
    }

    return true;
}

static bool _udp_update_rc_key(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    int len = obj->param_len;
    ucp_rc_key_info_head_t * head;
    ucp_rc_fixed_key_info_t* fi;
    ucp_rc_user_def_key_info_t* ui;
    cl_rc_fixed_key_info* rcf;
    cl_rc_user_key_info* rcu;
    cl_rc_info * dest = NULL;
    u_int8_t* content = OBJ_VALUE(obj, u_int8_t*);
    u_int8_t i;
    
    head = OBJ_VALUE(obj, ucp_rc_key_info_head_t *);
    
    if (is_obj_less_than_len(obj, sizeof(*head)) ) {
        return false;
    }
    
    
    while (len >= sizeof(*head)) {
        head = (ucp_rc_key_info_head_t *)content;
        if (head->type == RC_TYPE_TV) {
            dest = &ac->rc_pm.pair_rc.tv_info;
        }else if(head->type == RC_TYPE_STB){
            dest = &ac->rc_pm.pair_rc.stb_info;
        }else{
            return false;
            continue;
        }
        
        SAFE_FREE(dest->uk);
        SAFE_FREE(dest->fk);
        dest->fixed_key_num = 0;
        dest->user_def_key_num = 0;
        
        content = content+sizeof(*head);
        
        if (head->fix_key_num >0) {
            dest->fk = cl_calloc(sizeof(cl_rc_fixed_key_info)*head->fix_key_num, 1);
            if (!dest->fk) {
                return false;
            }
            fi = (ucp_rc_fixed_key_info_t*)content;
            rcf = (dest->fk);
            for (i = 0; i < head->fix_key_num; i++,rcf++) {
                rcf->key_id = fi->key_id;
                rcf->has_code = !! (fi->flags & BIT(0));
                content = content + head->fix_key_size;
                fi = (ucp_rc_fixed_key_info_t*)content;
            }
            dest->fixed_key_num = head->fix_key_num;
        }
        
        if (head->def_key_num >0 ) {
            dest->uk = cl_calloc(sizeof(cl_rc_user_key_info)*head->def_key_num, 1);
            if (!dest->uk) {
                return false;
            }
            ui = (ucp_rc_user_def_key_info_t*)content;
            rcu = dest->uk;
            for (i = 0; i < head->def_key_num; i++,rcu++) {
                rcu->key_id = ui->key_id;
                rcu->has_code = !! (ui->flags & BIT(0));
                memcpy(rcu->name, ui->name, sizeof(rcu->name)-1);
                content = content + head->def_key_size;
                ui = (ucp_rc_user_def_key_info_t*)content;
            }
            dest->user_def_key_num = head->def_key_num;

        }
        
        len = len - (sizeof(*head) + head->fix_key_num* head->fix_key_size +
                     head->def_key_num* head->def_key_size);
        
    }
    
    return true;
}

static bool _udp_update_rc_key_learn(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    ucp_rc_key_learn_t* kl;
    int event = 0,i;
    cl_rc_info* dest = NULL;
    bool is_find = false;
    
    if (is_obj_less_than_len(obj, sizeof(*kl)) ) {
        return false;
    }
    kl = OBJ_VALUE(obj, ucp_rc_key_learn_t*);
    
    if (action == UCA_PUSH) {
        
        if (kl->action == 0) {
            event = SAE_LEARN_KEY_SUCCESSED;
            if (ac->rc_pm.pair_rc.stb_info.d_id == kl->rc_id) {
                dest = &ac->rc_pm.pair_rc.stb_info;
            }else if(ac->rc_pm.pair_rc.tv_info.d_id == kl->rc_id){
                dest = &ac->rc_pm.pair_rc.tv_info;
            }
            
        }else{
            event = SAE_LEARN_KEY_WAIT_TIME_OUT;
        }
        
        event_push(ac->sac->user->callback, event, ac->sac->user->handle, ac->sac->user->callback_handle);
        event_cancel_merge(ac->sac->user->handle);
        
        if (dest != NULL ) {
            if (dest->uk != NULL) {
                for (i = 0; i< dest->user_def_key_num; i++) {
                    if (dest->uk[i].key_id == kl->key_id) {
                        dest->uk[i].has_code = true;
                        is_find = true;
                        break;
                    }
                }
            }
            
            
            if (!is_find && dest->fk != NULL) {
                for (i = 0; i< dest->fixed_key_num ; i++) {
                    if (dest->fk[i].key_id == kl->key_id) {
                        dest->fk[i].has_code = true;
                        is_find = true;
                        break;
                    }
                }
            }
            
            if (is_find) {
                event_push(ac->sac->user->callback, UE_INFO_MODIFY, ac->sac->user->handle, ac->sac->user->callback_handle);
                event_cancel_merge(ac->sac->user->handle);
            }
        }
        
        return false;
    }
    
    return true;
}

static int rc_timer_code_match(cl_thread_t *t)
{
    smart_air_ctrl_t* ac;
    
    ac = (smart_air_ctrl_t*)CL_THREAD_ARG(t);
    if (!ac) {
        return 0;
    }
    
    ac->rc_match_timer = NULL;
    memset(&ac->match_stat, 0x0 ,sizeof(ac->match_stat));
    
    if(ac->cur_match_step != RC_CMS_IDLE){
        rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
        
        ac->match_stat.error = ERR_CM_TO_DEV_TIME_OUT;
        log_debug("rc_timer_code_match ! send event to user!\n");
        event_push(ac->sac->user->callback, SAE_CODE_MATCH_FAILED, ac->sac->user->handle, ac->sac->user->callback_handle);
        event_cancel_merge(ac->sac->user->handle);
    }
    
    return 0;
}

static void rc_rest_code_match_timer(smart_air_ctrl_t* ac,u_int32_t sec_time_out)
{
    CL_THREAD_TIMER_OFF(ac->rc_match_timer);
    
    if(sec_time_out > 0){
        CL_THREAD_TIMER_ON(&cl_priv->master, ac->rc_match_timer, rc_timer_code_match, (void*)ac,
                           sec_time_out*TIME_PER_SECOND);
    }
}

static void rc_code_match_switch_to_status(smart_air_ctrl_t* ac,u_int32_t status)
{
    rc_rest_code_match_timer(ac,0);
    
    switch(status){
        case RC_CMS_START_ALL_MATCH:
            rc_rest_code_match_timer(ac,3);
            break;
        case RC_CMS_START_CLOUD_MATCH:
            rc_rest_code_match_timer(ac,3);
            break;
        case RC_CMS_WAIT_ALL_MATCH_RESULT:
            rc_rest_code_match_timer(ac,60);
            break;
        case RC_CMS_WAIT_CLOUD_MATCH_RESULT:
            rc_rest_code_match_timer(ac,60);
            break;
        case RC_CMS_WAIT_USER_IR_SIGNAL:
            rc_rest_code_match_timer(ac,3*60+10);
            break;
        default:
            status = RC_CMS_IDLE;
            break;
    }
    
    ac->cur_match_step = status ;
    
}

static bool _udp_update_rc_match(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    int event = 0;
	
    if (!ac ) {
        return false;
    }
    
    switch (ac->rc_pm.match_stat.action) {
        case RC_STOP_MATCH:
            if (ac->rc_pm.match_stat.error == ERR_NONE) {
                event = SAE_CODE_MATCH_STOP_OK;
            }else{
                event = SAE_CODE_MATCH_STOP_FAILED;
            }
            break;
        case RC_ALL_MATCH:
            if (ac->rc_pm.match_stat.error == ERR_NONE) {
                if(ac->cur_match_step != RC_CMS_IDLE){
                    rc_code_match_switch_to_status(ac,RC_CMS_WAIT_CLOUD_MATCH_RESULT);
                }
                
                if (action == UCA_PUSH) {
                    event = SAE_CODE_MATCH_STAT_MODIFY;
                }else{
                    event = SAE_CODE_MATCH_DEV_READY_OK;
                }
            }else{
                rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
                if (ac->rc_pm.match_stat.error == ERR_CM_PROCESS_MATCHING) {
                    event = SAE_CODE_MATCH_START_FAILED;
                }else{
                    event = SAE_CODE_MATCH_FAILED;
                }
            }
            break;
        case RC_MATCH_OK:
        {
            event = SAE_CODE_MATCH_OK;
            rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
			ac->air_info.is_match_code = true;
        }
            break;
        case RC_MATCH_FAILED:
        {
            rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
            event = SAE_CODE_MATCH_FAILED;
            log_debug("receive Dev error info ! send event to user!\n");
        }
            break;
        case RC_CLOUD_MATCH:
        {
            if (ac->rc_pm.match_stat.error == ERR_NONE) {
                if (ac->rc_pm.match_stat.is_cloud_matching == 0) {
                    event = SAE_CODE_MATCH_DEV_READY_OK;
                    if(ac->cur_match_step != RC_CMS_IDLE){
                        rc_code_match_switch_to_status(ac,RC_CMS_WAIT_USER_IR_SIGNAL);
                    }
                }else{
                    if(ac->cur_match_step != RC_CMS_IDLE){
                        rc_code_match_switch_to_status(ac,RC_CMS_WAIT_CLOUD_MATCH_RESULT);
                    }
                    
                    if (ac->rc_pm.match_stat.max_step > 0 && ac->rc_pm.match_stat.cur_step == 0) {
                        event = SAE_CODE_MATCH_DEV_RECV_CODE;
                    }else{
                        event = SAE_CODE_MATCH_STAT_MODIFY;
                    }
                }
            }else{
                log_debug("receive Dev error info ! send event to user!\n");
                event = SAE_CODE_MATCH_START_FAILED;
                rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
            }
        }
            break;
        case RC_MATCH_NEXT_KEY:
            if (ac->rc_pm.match_stat.error == ERR_NONE) {
                rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
                event = SAE_LEARN_KEY_NEED_NEXT_KEY;
            }else{
                log_debug("receive Dev error info ! send event to user!\n");
                event = SAE_CODE_MATCH_START_FAILED;
                rc_code_match_switch_to_status(ac,RC_CMS_IDLE);
            }
            break;
            
        default:
            break;
    }
    log_debug("_udp_update_rc_match event %u action %u is_code_match %u cur_step %u max_step %u\n",
              event,ac->rc_pm.match_stat.action,ac->rc_pm.match_stat.is_cloud_matching,ac->rc_pm.match_stat.cur_step,
              ac->rc_pm.match_stat.max_step);
    
    if (event != 0) {
        event_push(ac->sac->user->callback, event, ac->sac->user->handle, ac->sac->user->callback_handle);
        event_cancel_merge(ac->sac->user->handle);
    }
    return false;
}

static bool _udp_update_rc_info(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    bool res = false;
    
    switch (obj->attr) {
        case UCAT_CODE_RC_INFO:
            res = _udp_update_rc_pan(ac,action,obj);
            break;
        case UCAT_CODE_RC_KEY:
            res = _udp_update_rc_key(ac,action,obj);
            break;
        case UCAT_CODE_RC_LEARN:
            res = _udp_update_rc_key_learn(ac,action,obj);
            break;
        case UCAT_CODE_RC_MATCH:
            if (!is_obj_less_than_len(obj, sizeof(ucp_ac_code_match_stat_t))) {
                ucp_ac_code_match_stat_t* ms = (ucp_ac_code_match_stat_t*)(obj+1);
                ac->rc_pm.match_stat.action = ms->action;
                ac->rc_pm.match_stat.cur_step = ms->cur_step;
                ac->rc_pm.match_stat.error = ms->err;
                ac->rc_pm.match_stat.is_cloud_matching = ms->step_type?true:false;
                ac->rc_pm.match_stat.max_step = ms->step_num;
                ac->rc_pm.match_stat.flag = ms->flagbits;
                
                if (!is_obj_less_than_len(obj, sizeof(ucp_ac_code_next_key_match_stat_t))) {
                    ucp_ac_code_next_key_match_stat_t* nk = (ucp_ac_code_next_key_match_stat_t*)(obj+1);
                    ac->rc_pm.match_stat.recommon_key_id = nk->next_key;
                }
                
                log_debug("_udp_update_rc_info match action[%u] cur_step[%u] error[%u] is_match[%u] max_step[%u],next_key[%u]\n",
                          ac->rc_pm.match_stat.action,
                          ac->rc_pm.match_stat.cur_step,
                          ac->rc_pm.match_stat.error,
                          ac->rc_pm.match_stat.is_cloud_matching,
                          ac->rc_pm.match_stat.max_step,
                          ac->rc_pm.match_stat.recommon_key_id);
                
               res = _udp_update_rc_match(ac,action,obj);
            }
            
            break;
        default:
            break;
    }
    return res;
}

static u_int8_t dev_history_item_xor(u_int8_t *buf, int len)
{
	u_int8_t xor = 0;
	int i;
	
	for (i = 0; i < len; i++) {
		xor ^= buf[i];
	}

	return xor;
}

static bool _udp_common_do_2_12_public_attr(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    ucp_dev_err_info_t* ei;
    cl_dev_err_item_t* e_dest;
    ucp_dev_err_item_t* e_src;
    int len;
    user_t* user ;
    cl_dev_err_item_t * dest = NULL;
    
    switch(obj->attr)
    {
        case UCAT_IA_PUBLIC_EXCEPTION:
		case UCAT_IA_PUBLIC_EXCEPTION_V2:
            if(obj->param_len >= sizeof(*ei) && (((obj->param_len - sizeof(*ei)) %sizeof(ucp_dev_err_item_t)) == 0)){
                ei = (ucp_dev_err_info_t*)(obj+1);
                if(ei->err_count > 0 && ((obj->param_len - sizeof(*ei)) /sizeof(ucp_dev_err_item_t) == ei->err_count) ){
                    len = ei->err_count * sizeof(cl_dev_err_item_t);
                    dest = cl_calloc(len,1);
                    if(dest != NULL){
                        e_dest = dest;
                        e_src = &ei->items[0];
                        for(len = 0; len < ei->err_count; len++,e_dest++,e_src++){
                            e_dest->err_data = ntohl(e_src->err_data);
                            e_dest->err_id = ntohl(e_src->err_id);
                            e_dest->err_time = ntohl(e_src->err_time);
                            e_dest->err_type= ntohs(e_src->err_type);
                            e_dest->err_obj_type = ntohs(e_src->err_obj_type);
                            log_debug("Dev error info id[%u] err_type[%u] err_obj_type[%u] err_data[%u]\n",
                                      e_dest->err_id,
                                      e_dest->err_type,
                                      e_dest->err_obj_type,
                                      e_dest->err_data);
                        }
                    }
                } else {
                	return false;
                }
                
                //推送的异常信息
                if(action == UCA_PUSH){
                    air_ctrl->com_udp_dev_info.dev_err_info.pushed_err = dest;
                    air_ctrl->com_udp_dev_info.dev_err_info.pushed_err_count = ei->err_count;
                    
                    user = air_ctrl->sac->user;
                    event_push(user->callback, COMMON_UE_DEV_ERR_PUSH_MSG, user->handle, user->callback_handle);
                    return false;
                }else{
                    //清除老数据
                    air_ctrl->com_udp_dev_info.dev_err_info.err_count = 0;
                    SAFE_FREE(air_ctrl->com_udp_dev_info.dev_err_info.err_items);
                    //新数据
                    air_ctrl->com_udp_dev_info.dev_err_info.err_items = dest;
                    air_ctrl->com_udp_dev_info.dev_err_info.err_count = ei->err_count;
                }
                
            }
            break;

		
		case UCAT_IA_PUBLIC_HISTORY_INFO:
			{
				ucp_dev_comm_history_info_t *ud;

				if (is_obj_less_than_len(obj, sizeof(ucp_dev_comm_history_info_t))) {
	                break;
	            }

				ud = OBJ_VALUE(obj, ucp_dev_comm_history_info_t*);

				air_ctrl->com_udp_dev_info.dev_history.index_current = ntohl(ud->current_id);
				air_ctrl->com_udp_dev_info.dev_history.max_count = ntohl(ud->num);
				
				event_push(air_ctrl->sac->user->callback, SAE_DEV_COMM_HISTORY_SUMMARY, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);
			}
			break;

		case UCAT_IA_PUBLIC_HISTORY_ITEM:
			{
				cl_dev_history_t *dh = &air_ctrl->com_udp_dev_info.dev_history;
				ucp_dev_comm_history_hdr_t *hd;
				ucp_wukong_history_item_t *item;
				int i;
				u_int8_t xor;

				if (is_obj_less_than_len(obj, sizeof(ucp_dev_comm_history_hdr_t))) {
	                break;
	            }

				hd = OBJ_VALUE(obj, ucp_dev_comm_history_hdr_t*);
				hd->num = ntohs(hd->num);
				hd->item_size = ntohs(hd->item_size);

				log_debug("public history item: num %u item_size %u\n", hd->num, hd->item_size);

				if (hd->num * hd->item_size + sizeof(*hd) != obj->param_len) {
					log_err(false, "invalid item_size and num\n");
					break;
				}

				// 目前只处理悟空的
				if (air_ctrl->sac->user->sub_type != IJ_808) {
					log_err(false, "not support sub_type %u\n", air_ctrl->sac->user->sub_type);
					break;
				}

				if (hd->item_size != sizeof(*item)) {
					log_err(false, "item_size %u invalid, should be %d \n", hd->item_size, sizeof(*item));
					break;
				}

				if (hd->num == 0) {
					break;
				}				

				SAFE_FREE(dh->item);
				dh->item = cl_calloc(1, hd->num * hd->item_size);
				if (!dh->item) {
					dh->n = 0;
					break;
				}
				dh->n = hd->num;

				item = (ucp_wukong_history_item_t*)&hd[1];
				dh->index = ntohl(item->id);
				for (i = 0; i < hd->num; i++, item++) {
					if ((xor = dev_history_item_xor((u_int8_t*)item, sizeof(*item) - 1)) != item->crc) {
						dh->item[i].u.wukong.valid = false;
						continue;
					} else {
						dh->item[i].u.wukong.valid = true;
					}
					
					dh->item[i].u.wukong.id = ntohl(item->id);
					dh->item[i].u.wukong.time = ntohl(item->time);
					dh->item[i].u.wukong.type = item->type;
					dh->item[i].u.wukong.condition = item->condition;
					dh->item[i].u.wukong.action = item->action;
					dh->item[i].u.wukong.onoff = item->onoff;
					dh->item[i].u.wukong.mode = item->mode;
					dh->item[i].u.wukong.temp = item->temp;
					dh->item[i].u.wukong.wind = item->wind;
					dh->item[i].u.wukong.winddir = item->winddir;
					dh->item[i].u.wukong.key = item->key;
				}
				
				event_push(air_ctrl->sac->user->callback, SAE_DEV_COMM_HISTORY_ITEM, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);
			}
			break;

		case UCAT_IA_PUBLIC_BOOT_TEMP:
			{				
				u_int8_t *value = OBJ_VALUE(obj, u_int8_t*);

				if (is_obj_less_than_len(obj, sizeof(u_int16_t))) {
	                break;
	            }				

				air_ctrl->com_udp_dev_info.boot_temp_enable = value[0];
				air_ctrl->com_udp_dev_info.boot_temp = value[1];
				
			}
			break;

		case UCAT_IA_PUBLIC_WAN_CONFIG:
			{
				cl_wan_config_t *value = OBJ_VALUE(obj, cl_wan_config_t *);
				int plen, i, j;
				cl_wan_config_item_t *item;

				log_debug("	get wan config, param len %u\n", obj->param_len);

				if (is_obj_less_than_len(obj, 4)) {
	                break;
	            }	

				if (value->wan_num > 3) {
					break;
				}

				plen = 4 + value->wan_num * sizeof(cl_wan_phy_item_t);
				if (is_obj_less_than_len(obj, plen)) {
	                break;
	            }	

				memcpy((u_int8_t*)&air_ctrl->com_udp_dev_info.wan_config, (u_int8_t*)value, plen);	

				for (i = 0; i < value->wan_num; i++) {		
					log_debug("wan idx select type %u\n", value->config[i].select_network_type);
					for (j = 0; j < 3; j++) {		
						item = &value->config[i].config_item[j];
						
						log_debug("wan idx %u wan_phy_type %u\n", i, item->network_type);

						switch (value->config[i].config_item[j].network_type) {
						case 0:
							log_debug("	not config\n");
							break;
						case 1:
							log_debug("static:	ip %s mask %s gw %s dns1 %s dns2 %s\n", 
								item->config.config_static.ip, item->config.config_static.mask, 
								item->config.config_static.getway_ip, item->config.config_static.main_dns, item->config.config_static.sub_dns);
							break;
						case 2:
							log_debug("dhcp:	dns1 %s dns2 %s ip %s mask %s gw %s\n", item->config.config_dhcp.main_dns, item->config.config_dhcp.sub_dns, item->config.config_dhcp.ip, item->config.config_dhcp.mask, item->config.config_dhcp.getway_ip);
							break;
						case 3:
							log_debug("	name %s pwd %s dns1 %s dns2 %s\n", item->config.config_pppoe.name, item->config.config_pppoe.pwd, item->config.config_pppoe.main_dns, item->config.config_pppoe.sub_dns);
							break;
						default:
							log_debug("	ignore invalid wan_type\n");
							break;
						}
					}					
				}
			}
			break;
		case UCAT_IA_PUBLIC_DHCP_SERVER:
			{
				cl_dhcp_server_config_t *value = OBJ_VALUE(obj, cl_dhcp_server_config_t *);

				if (is_obj_less_than_len(obj, sizeof(*value))) {
	                break;
	            }

				value->time = ntohl(value->time);

				log_debug("	public dhcp server: start ip [%s] end ip [%s] mask [%s] time %u\n", 
					value->start_ip, value->end_ip, value->mask, value->time);

				
				memcpy((u_int8_t*)&air_ctrl->com_udp_dev_info.dhcp_config, (u_int8_t*)value, sizeof(*value));				
				
			}
			break;

		case UCAT_IA_PUBLIC_AP_CONFIG:
			{
				cl_ap_config_t *value = OBJ_VALUE(obj, cl_ap_config_t *);

				log_debug("get ap config\n");

				if (is_obj_less_than_len(obj, sizeof(*value))) {
	                break;
	            }

				log_debug("	public ap config: ssid[%s] pwd[%s] enc %u pow %u channel_mode %u channel %u enable %u\n", 
					value->ssid, value->pwd, value->is_enc, value->pow, value->channle_mode, value->channel, value->enable);

				
				memcpy((u_int8_t*)&air_ctrl->com_udp_dev_info.ap_config, (u_int8_t*)value, sizeof(*value));				
				
			}
			break;
		case UCAT_IA_PUBLIC_REPEATER:
			{
				u_int8_t *value = OBJ_VALUE(obj, u_int8_t *);

				if (is_obj_less_than_len(obj, sizeof(*value))) {
	                break;
	            }

				log_debug("	public repeater onoff %u\n", *value);

				air_ctrl->com_udp_dev_info.repeat_onoff = *value;
			}
			break;
        default:
            return false;
            break;
    }
    
    return true;
}


// 拷贝通用数据
static bool udp_build_common_info(user_t* user,smart_air_ctrl_t* ac,cl_dev_info_t* ui)
{
    cl_com_udp_device_data *info;
    int len;
    
    info = cl_calloc(sizeof(cl_com_udp_device_data), 1);
    if (!info) {
        return false;
    }
    
    //拷贝数据结构，并清空指针相关数据
    memcpy(info, &ac->com_udp_dev_info, sizeof(*info));

	// USER下面的ext_type 会被改变
	info->sub_type = user->sub_type;
	info->ext_type = user->ext_type;

    info->device_info = NULL;
    info->is_support_period_timer = ac->air_info.is_support_peroid_timer;
    if (!info->is_support_period_timer) {
        info->is_support_period_timer = ac->com_udp_dev_info.is_support_period_timer;
    }
    info->elec_days_info.days_count = 0;
    info->elec_days_info.elec_data = NULL;
    info->dev_err_info.err_count = 0;
    info->dev_err_info.err_items = NULL;
    info->dev_err_info.pushed_err_count= 0;
    info->dev_err_info.pushed_err= NULL;
    
    //通用电量统计
    if(ac->com_udp_dev_info.elec_days_info.days_count > 0 && ac->com_udp_dev_info.elec_days_info.elec_data != NULL){
		len = sizeof(u_int16_t)*ac->com_udp_dev_info.elec_days_info.days_count;
		info->elec_days_info.elec_data = cl_calloc(len,1);
		if(info->elec_days_info.elec_data != NULL){
			memcpy(info->elec_days_info.elec_data,ac->com_udp_dev_info.elec_days_info.elec_data,len);
			info->elec_days_info.days_count = ac->com_udp_dev_info.elec_days_info.days_count;
		}
	}
	//通用定时器
	if(info->is_support_period_timer){
		air_timer_dup(&info->timer_info, &ac->air_info.air_timer_info);
	}
	//通用设备错误信息拷贝
	if(ac->com_udp_dev_info.dev_err_info.err_count > 0 && ac->com_udp_dev_info.dev_err_info.err_items != NULL){
		len = ac->com_udp_dev_info.dev_err_info.err_count*sizeof(cl_dev_err_item_t);
		info->dev_err_info.err_items = cl_calloc(len, 1);
		if(info->dev_err_info.err_items != NULL){
			memcpy(info->dev_err_info.err_items,ac->com_udp_dev_info.dev_err_info.err_items,len);
			info->dev_err_info.err_count = ac->com_udp_dev_info.dev_err_info.err_count;
		}
	}

	// 推送的错误信息
	if(ac->com_udp_dev_info.dev_err_info.pushed_err_count > 0 && ac->com_udp_dev_info.dev_err_info.pushed_err != NULL){
		len = ac->com_udp_dev_info.dev_err_info.pushed_err_count*sizeof(cl_dev_err_item_t);
		info->dev_err_info.pushed_err = cl_calloc(len, 1);
		if(info->dev_err_info.pushed_err != NULL){
			memcpy(info->dev_err_info.pushed_err,ac->com_udp_dev_info.dev_err_info.pushed_err,len);
			info->dev_err_info.pushed_err_count = ac->com_udp_dev_info.dev_err_info.pushed_err_count;
		}
	}

	info->has_recv_flag_pkt = ac->com_udp_dev_info.has_recv_flag_pkt;
	info->is_support_la = ac->com_udp_dev_info.is_support_la;

	// 联动用户上下线信息
	if (ac->com_udp_dev_info.lum_info.record) {
		info->lum_info.record = cl_calloc(1, info->lum_info.record_num * sizeof(lanusers_manage_user_record_item_t));
		if (info->lum_info.record != NULL) {
			memcpy(info->lum_info.record, ac->com_udp_dev_info.lum_info.record, info->lum_info.record_num * sizeof(lanusers_manage_user_record_item_t));
		}
	}
	
	ui->com_udp_info = info;
	
	return true;
}

static bool udp_bulid_pure_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di,u_int32_t priv_data_size)
{
	void* dest;
	
	if(priv_data_size ==0 || !ac || !di || !ac->com_udp_dev_info.device_info){
		return false;
	}

	dest  = cl_calloc(priv_data_size,0x1);
	if(!dest){
		return false;
	}

	memcpy(dest,ac->com_udp_dev_info.device_info, priv_data_size);
	
	di->device_info = dest;
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//添加各种设备的处理

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//金长信配电箱

static bool jcx_power_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_jcx_power_box_info* info = air_ctrl->com_udp_dev_info.device_info; 
	if( !info || !obj ){
		return false;
	}

	air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
		case UCAT_IA_JCX_VOL:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->voltage = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_CURRENT:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->elec = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_ACTIVE_POWER:
			if(is_valid_obj_data(obj,sizeof(u_int32_t))){
				info->active_power = ntohl(obj_u32_value(obj));
			}
			break;
		case UCAT_IA_JCX_REACTIVE_POWER:
			if(is_valid_obj_data(obj,sizeof(u_int32_t))){
				info->reactive_power = ntohl(obj_u32_value(obj));
			}
			break;
		case UCAT_IA_JCX_POWER_FACTOR:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->power_factor = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_FREQ:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->frequency = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_ACTIVE_DEGREE:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->active_degree = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_REACTIVE_DEGREE:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->reactive_degree = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_CHANNEL_NAME:
			
			break;
		case UCAT_IA_JCX_CHANNEL_INFO:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->channelstat = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_SN:
			if(is_valid_obj_data(obj,sizeof(u_int32_t))){
				info->jcx_sn = ntohl(obj_u32_value(obj));
			}
			break;
		case UCAT_IA_JCX_SOFT_VER:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->jcx_soft_ver = ntohs(obj_u16_value(obj));
			}
			break;
		case UCAT_IA_JCX_HARWARE_VER:
			if(is_valid_obj_data(obj,sizeof(u_int16_t))){
				info->jcx_hardware_ver = ntohs(obj_u16_value(obj));
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}

static bool jcx_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_jcx_power_box_info * bi = NULL;
	cl_jcx_power_box_info* src;
	int i;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	src = ac->com_udp_dev_info.device_info;
	bi = cl_calloc(sizeof(*bi) , 0x1);
	if(!bi){
		return false;
	}
	
	memcpy(bi,src,sizeof(*src));
	
	// 清空和拷贝名字
	memset(bi->channel_names,0,sizeof(bi->channel_names));
	for(i = 0; i < bi->channel_num; i++){
		if(src->channel_names[i] != NULL){
			bi->channel_names[i] = cl_strdup(src->channel_names[i]);
		}
	}
	
	di->device_info = bi;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//车载悟空
static bool _car_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	int len = 0;
    cl_car_info_t *ci;
    cl_car_info_t *ci_up;
    
    if(!di || !ac || !ac->com_udp_dev_info.device_info){
        return false;
    }
    
    ci = ac->com_udp_dev_info.device_info;
    ci_up = cl_calloc(sizeof(*ci), 1);
    if (!ci_up) {
        return false;
    }
    
    memcpy((void *)ci_up, (void *)ci, sizeof(*ci));
	if (ci_up->alarm_num > 0) {
		len = ci_up->alarm_num*sizeof(cl_car_alarm_t);
		ci_up->car_alarm = cl_calloc(len, 1);
		if (ci_up->car_alarm) {
			memcpy((void *)ci_up->car_alarm, (void *)ci->car_alarm, len);
		} else {
			ci_up->alarm_num = 0;
		}
	} else {
		ci_up->car_alarm = NULL;
	}

	if (ci_up->debug_info) {
		ci_up->debug_info = cl_strdup(ci->debug_info);
	}
    
    di->device_info = ci_up;
    
    return true;
}
static void _free_jcx_info(cl_jcx_power_box_info* info)
{
	int i ;

	if(!info){
		return;
	}

	for(i = 0; i < info->channel_num; i++){
		SAFE_FREE(info->channel_names[i]);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//拓邦

static int _tb_fill_packet(cln_common_info_t* info,ucp_obj_t* uo,void* dest)
{
	int dest_pkt_len = -1;
	ucp_tb_user_config_t* uc;
	ucp_tb_bind_info* bi;
	ucp_tb_work_config_t* wc;

	cl_tb_user_config_t* src_uc;
	u_int8_t* src_bi;
	cl_tb_work_config_t* src_wc;

	if(!info || !uo || !dest){
		return dest_pkt_len;
	}

	switch(info->action){
	case ACT_TB_CTRL_STAT:
	{
		dest_pkt_len = sizeof(ucp_tb_user_config_t);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_USERCONFIG,dest_pkt_len);
		uc = dest;
		src_uc = (cl_tb_user_config_t*)(&info->u.u8_data[0]);
		uc->cid = src_uc->cid;
		uc->onoff = src_uc->onoff;
		uc->temp = src_uc->temp;
		uc->work_mode = src_uc->work_mode;
	}	
		break;
	case ACT_TB_CTRL_SETTING_WORK_PARAM:
	{
		dest_pkt_len = sizeof(ucp_tb_work_config_t);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_WORKCONFIG,dest_pkt_len);
		wc = dest;
		src_wc = (cl_tb_work_config_t*)(&info->u.u8_data[0]);
		wc->cid = src_wc->cid;
		wc->backlight_delay_time = src_wc->backlight_delay_time;
		wc->eheat_value = src_wc->eheat_value;
		wc->facility_state = htons(src_wc->facility_state);
		wc->heat_defrost_circle = src_wc->heat_defrost_circle;
		wc->return_cold_switch = src_wc->return_cold_switch;
		wc->return_diff_temp = src_wc->return_diff_temp;
		wc->start_heat_defrost_temp = src_wc->start_heat_defrost_temp;
		wc->stop_heat_defrost_temp = src_wc->stop_heat_defrost_temp;
		wc->stop_heat_defrost_time = src_wc->stop_heat_defrost_time;
		wc->sysfunc = htons(src_wc->sysfunc);
		wc->fan_mode = src_wc->fan_mode;
	}
		break;
	case ACT_TB_CTRL_BIND_BAR_CODE:
	{
		dest_pkt_len = sizeof(ucp_tb_bind_info);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_BINDSN,dest_pkt_len);
		bi = dest;
		src_bi = (u_int8_t*)(&info->u.u8_data[0]);
		memcpy(bi->tb_sn,src_bi,17);
	}
		break;
	default:
		
		break;
	}

	return dest_pkt_len;
}

static bool _tb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	char buf[256] = {0};
	int dest_pkt_len = 0;

	ucp_obj_t* uo = (ucp_obj_t*)buf;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	switch(info->action){
	case ACT_TB_CTRL_STAT:
		dest_pkt_len = _tb_fill_packet(info,uo,(void*)(uo+1));
		break;
	case ACT_TB_CTRL_SETTING_WORK_PARAM:
		dest_pkt_len = _tb_fill_packet(info,uo,(void*)(uo+1));
		break;
	case ACT_TB_CTRL_REFRESH_TEMP_INFO:
		return udp_query_single_object(ac->sac->user->uc_session,  UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_HEATER_TEMP);
		break;
	case ACT_TB_CTRL_REFRESH_OTHER_INFO:
		return udp_query_single_object(ac->sac->user->uc_session,  UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_SYSINFO);
		break;
	case ACT_TB_CTRL_REFRESH_FAULT_INFO:
		return udp_query_single_object(ac->sac->user->uc_session,  UCOT_IA, UCSOT_IA_TBHEATER, UCAT_TBHEATER_STATE);
		break;
	case ACT_TB_CTRL_BIND_BAR_CODE:
		dest_pkt_len = _tb_fill_packet(info,uo,(void*)(uo+1));
		break;
	default:
		break;
	}
	
	if(dest_pkt_len <= 0 ){
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+dest_pkt_len);

	return res;
}

static bool _tb_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_tb_info_t* info = air_ctrl->com_udp_dev_info.device_info; 
	ucp_tb_user_config_t* src_uc;
	ucp_tb_bind_info* src_bi;
	ucp_tb_work_config_t* src_wc;
	ucp_tb_temp_info_t* src_ti;
	ucp_tb_fault_stat* src_fs;
	ucp_tb_other_info* src_other;

	if( !info || !obj ){
		return false;
	}

	switch(obj->attr){
		case UCAT_TBHEATER_USERCONFIG:
			if(is_valid_obj_data(obj, sizeof(*src_uc))){
				src_uc = OBJ_VALUE(obj,ucp_tb_user_config_t*);
				info->u_config.cid = src_uc->cid;
				info->u_config.onoff = src_uc->onoff;
				info->u_config.temp = src_uc->temp;
				info->u_config.work_mode = src_uc->work_mode;
				air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;
			}else{
				return false;
			}
			break;
		case UCAT_TBHEATER_WORKCONFIG:
			if(is_valid_obj_data(obj, sizeof(*src_wc))){
				src_wc = OBJ_VALUE(obj,ucp_tb_work_config_t*);
				info->w_config.backlight_delay_time = src_wc->backlight_delay_time;
				info->w_config.cid = src_wc->cid;
				info->w_config.eheat_value = src_wc->eheat_value;
				info->w_config.facility_state = ntohs(src_wc->facility_state);
				info->w_config.heat_defrost_circle = src_wc->heat_defrost_circle;
				info->w_config.return_cold_switch = src_wc->return_cold_switch;
				info->w_config.return_diff_temp = src_wc->return_diff_temp;
				info->w_config.start_heat_defrost_temp = src_wc->start_heat_defrost_temp;
				info->w_config.stop_heat_defrost_temp = src_wc->stop_heat_defrost_temp;
				info->w_config.stop_heat_defrost_time = src_wc->stop_heat_defrost_time;
				info->w_config.fan_mode = src_wc->fan_mode;
				info->w_config.sysfunc = ntohs(src_wc->sysfunc);
			}else{
				return false;
			}
			break;
		case UCAT_TBHEATER_HEATER_TEMP:
			if(is_valid_obj_data(obj, sizeof(*src_ti))){
				src_ti = OBJ_VALUE(obj,ucp_tb_temp_info_t*);
				info->temp_info.air_temp = src_ti->air_temp;
				info->temp_info.cid = src_ti->cid;
				info->temp_info.coil_temp = src_ti->coil_temp;
				info->temp_info.consumption_power = ntohs(src_ti->consumption_power);
				info->temp_info.env_temp = src_ti->env_temp;
				info->temp_info.heat_capacity = ntohs(src_ti->heat_capacity);
				info->temp_info.heat_time = ntohs(src_ti->heat_time);
				info->temp_info.inwater_temp = src_ti->inwater_temp;
				info->temp_info.outwater_temp = src_ti->outwater_temp;
				info->temp_info.returnair_temp = src_ti->returnair_temp;
				info->temp_info.returnwater_temp = src_ti->returnwater_temp;
				info->temp_info.saving_power = ntohs(src_ti->saving_power);
				info->temp_info.tankbottom_temp = src_ti->tankbottom_temp;
				info->temp_info.tanktop_temp = src_ti->tanktop_temp;
			}else{
				return false;
			}
			break;
		case UCAT_TBHEATER_STATE:
			if(is_valid_obj_data(obj, sizeof(*src_fs))){
				src_fs = OBJ_VALUE(obj,ucp_tb_fault_stat*);
				info->fault_stat.cid = src_fs->cid;
				info->fault_stat.dev_fault = ntohs(src_fs->dev_fault);
				info->fault_stat.dev_guard = ntohs(src_fs->dev_guard);
				info->fault_stat.slave_onoff = ntohs(src_fs->slave_onoff);
				info->fault_stat.valve_expansion = src_fs->valve_expansion;
				info->fault_stat.load_state = ntohs(src_fs->load_state);
			}else{
				return false;
			}
			break;
		case UCAT_TBHEATER_SYSINFO:
			if(is_valid_obj_data(obj, sizeof(*src_other))){
				src_other = OBJ_VALUE(obj,ucp_tb_other_info*);
				info->other_info.cid = src_other->cid;
				info->other_info.dev_info = src_other->dev_info;
				info->other_info.dev_mode = src_other->dev_mode;
				info->other_info.fw_version = src_other->fw_version;
				info->other_info.mb_version = src_other->mb_version;
				info->other_info.svn_version = src_other->svn_version;
				info->other_info.stm_up_stat= src_other->stm_up_state;
			}else{
				return false;
			}
			break;
		case UCAT_TBHEATER_UPGRADE_SLAVE:
			
			break;
		case UCAT_TBHEATER_BINDSN:
			if(is_valid_obj_data(obj, sizeof(*src_bi))){
				src_bi = OBJ_VALUE(obj,ucp_tb_bind_info*);
				info->bind_state = ntohs(src_bi->bind_state);
				memcpy(info->tb_sn,src_bi->tb_sn,sizeof(info->tb_sn));
			}else{
				return false;
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 亿林温控器
static bool _yl_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	ucp_yl_work_stat* ws;
	ucp_yl_data_mode_t* ym;
	char buf[256] = {0};
	int dest_pkt_len = 0;
	cl_yl_thermostat_info* stat;
	u_int32_t data;
	u_int32_t count,i;

	ucp_obj_t* uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "_yl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "_yl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	stat = ac->com_udp_dev_info.device_info;
	
	switch(info->action){
	case ACT_YL_CTRL_ONOFF:
		dest_pkt_len = sizeof(*ws);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_YL, UCAT_IA_THERMOSTAT_YL_WORK,dest_pkt_len);
		ws = (ucp_yl_work_stat*)(uo+1);
		ws->onoff = cci_u8_data(info);
		break;
	case ACT_YL_CTRL_MODE:
		dest_pkt_len = sizeof(*ym);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_YL, UCAT_IA_THERMOSTAT_YL_MODE,dest_pkt_len);
		ym = (ucp_yl_data_mode_t*)(uo+1);
		ym->mode = cci_u8_data(info);
		ym->scene = stat->cur_scene;
		ym->gear = stat->gear;
		ym->tmp = stat->temp;
		break;
	case ACT_YL_CTRL_GEAR:
		dest_pkt_len = sizeof(*ym);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_YL, UCAT_IA_THERMOSTAT_YL_MODE,dest_pkt_len);
		ym = (ucp_yl_data_mode_t*)(uo+1);
		data = cci_u32_data(info);
		ym->mode =  (data >> 24) &0xff;
		ym->scene = stat->cur_scene;
		ym->gear = (data >> 16) &0xff;
		ym->tmp = stat->temp;
		break;
	case ACT_YL_CTRL_TEMP:
		dest_pkt_len = sizeof(*ym);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_YL, UCAT_IA_THERMOSTAT_YL_MODE,dest_pkt_len);
		ym = (ucp_yl_data_mode_t*)(uo+1);
		data = cci_u32_data(info);
		ym->mode =  (data >> 24) &0xff;
		ym->scene = stat->cur_scene;
		ym->tmp = (data >> 16) &0xff;
		ym->gear = stat->gear;
		break;
	case ACT_YL_CTRL_SCENE:
		dest_pkt_len = sizeof(*ym);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_YL, UCAT_IA_THERMOSTAT_YL_MODE,dest_pkt_len);
		ym = (ucp_yl_data_mode_t*)(uo+1);
		data = cci_u32_data(info);
		ym->mode =  (data >> 24) &0xff;
		ym->scene = (data >> 16) &0xff;
		ym->gear = stat->gear;
		ym->tmp = stat->temp;
		break;
	case ACT_YL_SETTING_TEMP_PARAM:
		{
			ts_scene_t * src;
			ucp_yl_scene_t* us;
			u_int8_t* dest;
			
			count = info->u.u8_data[1];
			dest = (u_int8_t*)(uo+1);
			
			us = (ucp_yl_scene_t*)&dest[2];
			src = (ts_scene_t *)&info->u.u8_data[2];
			
			dest_pkt_len = sizeof(ucp_yl_scene_t)*count+2;
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_YL, UCAT_IA_THERMOSTAT_YL_SET_TMP,dest_pkt_len);
			memcpy((void*)(uo+1),&info->u.u8_data[0],2);
			for(i = 0 ; i < count ; i++,src++,us++){
				us->gear = src->gear;
				us->set_tmp = src->set_tmp;
			}
		}
		break;
	default:
		break;
	}

	if(dest_pkt_len <= 0 ){
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+dest_pkt_len);

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 海迅破壁机
static bool _hx_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	ucp_hx_data_t* hd;
	cl_hx_info *cht;
	char buf[256] = {0};
	ucp_obj_t* uo = NULL;
	int dest_pkt_len = 0;
	u_int8_t *pvalue = NULL;
	ucp_hx_diy_name_t *pdiy = NULL;	
	int len = 0;

	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "_hx_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "_hx_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	cht = ac->com_udp_dev_info.device_info;
	
	switch(info->action){
	case ACT_HX_MODE_CMD:
		dest_pkt_len = sizeof(*hd);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_BREAK_MACHINE, UCAT_IA_BREAK_MACHINE_SET_CMD,dest_pkt_len);
		hd = (ucp_hx_data_t*)(uo+1);
		hd->value = cci_u8_data(info);
		cht->cur_mode = hd->value;
		break;
	case ACT_HX_DIY_NAME:
		pvalue = (u_int8_t *)info->u.u8_data;
		len = (int)strlen((char *)&pvalue[1]) + 1;
		dest_pkt_len = 2 + len;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_BREAK_MACHINE, UCAT_IA_BREAK_MACHINE_DIYNAME,dest_pkt_len);
		pdiy = (ucp_hx_diy_name_t *)(uo+1);
		pdiy->id = pvalue[0];
		pdiy->len = len;
		strcpy((char *)&pdiy[1], (char *)&pvalue[1]);
		break;
	case ACT_HX_FINISH_CLEAR:
		cht->cur_send_finsh = 0;
		return res;
	default:
		break;
	}

	if(dest_pkt_len <= 0 ){
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+dest_pkt_len);

	return res;
}

static bool _yl_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_yl_thermostat_info* info = air_ctrl->com_udp_dev_info.device_info; 
	ucp_yl_work_stat* ws;
	ucp_yl_data_mode_t* ym;
	int i;
	ucp_yl_scene_t* dp;

	if( !info || !obj ){
		return false;
	}

	air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
		case UCAT_IA_THERMOSTAT_YL_WORK:
			if(is_valid_obj_data(obj, sizeof(ucp_yl_work_stat))){
				ws = OBJ_VALUE(obj, ucp_yl_work_stat*);
				info->onoff = ws->onoff;
			}else{
				return false;
			}
			break;
		case UCAT_IA_THERMOSTAT_YL_MODE:
			if(is_valid_obj_data(obj, sizeof(ucp_yl_data_mode_t))){
				ym = OBJ_VALUE(obj, ucp_yl_data_mode_t*);
				info->work_mode = ym->mode;
				info->gear = ym->gear;
				info->cur_scene = ym->scene;
				if(ym->tmp < 10 || ym->tmp > 30){
					ym->tmp = 23;
				}
				info->temp = ym->tmp;
			}else{
				return false;
			}
			break;
		case UCAT_IA_THERMOSTAT_YL_SET_TMP:
			if(is_valid_obj_data(obj, sizeof(ucp_yl_scene_t)*YL_TS_FUNC_MODE_MAX*YL_TS_MODE_MAX)){
				dp = OBJ_VALUE(obj, ucp_yl_scene_t*);
				
				for(i = 0; i< YL_TS_FUNC_MODE_MAX*YL_TS_MODE_MAX; i++,dp++){
					info->scene[i].set_tmp = dp->set_tmp;
					info->scene[i].gear = dp->gear;
				}
			}else{
				return false;
			}
			break;
		case UCAT_IA_THERMOSTAT_YL_ROOM_TMP:
			if(is_valid_obj_data(obj, sizeof(u_int32_t))){
				info->room_temp = (u_int8_t)((ntohl(obj_u32_value(obj)) >> 24) & 0xff);
			}else{
				return false;
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}

//海迅数据更新
static bool _hx_update_data_diy(cl_hx_info* info, ucp_obj_t* obj)
{
	int remain_len = (int)obj->param_len;
	int len = 0;
	ucp_hx_diy_name_t *pdiy = NULL;

	if (obj->param_len < sizeof(*pdiy)) {
		return false;
	}

	pdiy = (ucp_hx_diy_name_t *)(obj+1);
	while(remain_len > (int)sizeof(ucp_hx_diy_name_t) && 
		remain_len >= (int)(sizeof(ucp_hx_diy_name_t) + pdiy->len)) {
		if (pdiy->id < HX_MODE_MAX) {
			memcpy((void *)info->name[pdiy->id], (void *)(pdiy+1), pdiy->len);
			info->name[pdiy->id][pdiy->len] = 0;
		}
		len = (int)(sizeof(ucp_hx_diy_name_t) + pdiy->len);
		pdiy = (ucp_hx_diy_name_t *)((u_int8_t *)pdiy + len);	
		remain_len -= len;
	}

	return true;
}

static bool _hx_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_hx_info* info = air_ctrl->com_udp_dev_info.device_info; 
	ucp_hx_data_t* hx_hd;

	if( !info || !obj ){
		return false;
	}

	air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
		case UCAT_IA_BREAK_MACHINE_SET_CMD:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				//check
				if (hx_hd->value >= HX_MODE_MAX) {
					return false;
				} 
				info->cur_mode = hx_hd->value;
			}else{
				return false;
			}			
			break;
		case UCAT_IA_BREAK_MACHINE_ON_TIME:
			if(is_valid_obj_data(obj, sizeof(u_int32_t))){
				info->work_time = ntohl(obj_u32_value(obj));
			}else{
				return false;
			}			
			break;
		case UCAT_IA_BREAK_MACHINE_TEMP:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->cur_tmp = hx_hd->value;
			}else{
				return false;
			}			
			break;
		case UCAT_IA_BREAK_MACHINE_SPEED:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->cur_speed = hx_hd->value;
			}else{
				return false;
			}			
			break;
		case UCAT_IA_BREAK_MACHINE_FINISH:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->cur_send_finsh = hx_hd->value;
			}else{
				return false;
			}			
			break;
		case UCAT_IA_BREAK_MACHINE_STOP:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->cur_send_err = hx_hd->value;
			}else{
				return false;
			}			
			break;
		case UCAT_IA_BREAK_MACHINE_DIYNAME:
			if (_hx_update_data_diy(info, obj) == false) {
				return false;
			}
			break;	
		case UCAT_IA_BREAK_MACHINE_PAUSE:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->cur_pause = hx_hd->value;
			}else{
				return false;
			}	
			break;
		case UCAT_IA_BREAK_MACHINE_IDLE:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->idle_status = hx_hd->value;
			}else{
				return false;
			}	
			break;
		case UCAT_IA_BREAK_MACHINE_KEEP_WARM:
			if(is_valid_obj_data(obj, sizeof(ucp_hx_data_t))){
				hx_hd = OBJ_VALUE(obj, ucp_hx_data_t*);
				info->keep_warm = hx_hd->value;
			}else{
				return false;
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 印度车载
static void latitude_longitud_order(latitude_longitud_data_t *data)
{
	data->high = ntohl(data->high);
	data->low = ntohl(data->low);
}


static void india_car_stat_order(india_car_stat_t *stat)
{
	stat->id = ntohs(stat->id);
	stat->runfree_time = ntohs(stat->runfree_time);
	stat->total_time = ntohs(stat->total_time);
	stat->total_distance = ntohs(stat->total_distance);

	latitude_longitud_order(&stat->longitude);
	latitude_longitud_order(&stat->latitude);

	stat->journey_date = ntohl(stat->journey_date);

}

static void india_car_warn_order(india_car_warn_t *warn)
{
	warn->onoff = ntohl(warn->onoff);
	warn->max_distance = ntohs(warn->max_distance);
	warn->max_time = ntohs(warn->max_time);
	warn->max_freerun_time = ntohs(warn->max_freerun_time);

	latitude_longitud_order(&warn->longitude);
	latitude_longitud_order(&warn->latitude);
}

void india_car_history_reply_hd_order(india_car_history_reply_t *reply)
{
	reply->date = ntohl(reply->date);
}

void india_car_debug_config_order(india_car_debug_config_t *request)
{
	request->cmd_len = ntohs(request->cmd_len);
	request->gps_time_inv = ntohs(request->gps_time_inv);
	request->remote_port = ntohs(request->remote_port);
	request->remote_ip = ntohl(request->remote_ip);
	request->gps_len_inv = ntohs(request->gps_len_inv);
	request->file_debug_enable = ntohs(request->file_debug_enable);
	request->file_debug_level = ntohs(request->file_debug_level);
	request->file_debug_url_len = ntohs(request->file_debug_url_len);
	request->bps = ntohs(request->bps);
	request->moto_threshold = ntohs(request->moto_threshold);
	request->detail_save_inv = ntohs(request->detail_save_inv);
}

static void indiacar_history_download_init(history_download_stat_t *ds)
{
	log_debug("indiacar_history_download_init... ds->data = %p\n", ds->data);
	
	// 之前有缓存没下载完，又来新的
	if (ds->data != NULL) {
		cl_free(ds->data);
		ds->data = NULL;
	}

	memset(ds, 0x00, sizeof(*ds));
}

static bool indiacar_do_history_get_reply(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;
	india_car_history_reply_t *reply;
	history_download_stat_t *ds;
	cl_indiacar_jorney_num_t *jn;
	
	user_t *user = ac->sac->user;
	
	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	ds = &priv_info->ds;
	jn = &priv_info->jn;

	reply = (india_car_history_reply_t *)&obj[1];

	india_car_history_reply_hd_order(reply);

	log_debug("get indiacar history reply type %u id %u date %u\n", reply->type, reply->id, reply->date);

	if (reply->err) {
		return false;
	}

	indiacar_history_download_init(ds);

	//  请求某天有多少旅程的返回值
	if (reply->type == 3) {
		log_debug("have %d journy this day\n", reply->journey_count);

		jn->date = reply->date;
		jn->count = reply->journey_count;
		
		event_push(user->callback, UE_INDIACAR_JORNEY_COUNT, user->handle, user->callback_handle);
		
		return false;
	}

	ds->id = reply->id;
	ds->date = reply->date;
	ds->journey_count = reply->journey_count;

	log_info("indiacar : start download id %u data %u\n", ds->id, ds->date);

	// 都返回FALSE
	return false;
}

void india_car_history_push_hd_order(india_car_history_notify_t *notify)
{
	notify->date = ntohl(notify->date);
	notify->data_len = ntohl(notify->data_len);
	notify->data_idx = ntohl(notify->data_idx);
}

/**
	把下载好的数据写FLASH
	路径: base_path/sn/indiacar_history/date/jorney_id/info.dat
	               base_path/sn/indiacar_history/date/jorney_id/detail.dat
*/
static void indiacar_history_write_flash(u_int64_t sn, history_download_stat_t *ds)
{
	char file_path[256] = {0};
	FILE *fp;
	india_car_history_flash_hd_t hd;
	int len = 0;
	
	if (ds->data == NULL)
		return;

	// 不管三七二十一，先吧文件夹建好
	len += sprintf(&file_path[len], "%s/%012"PRIu64"", cl_priv->dir, sn);
	MKDIR(file_path, 0777);

	len += sprintf(&file_path[len], "/indiacar_history");
	MKDIR(file_path, 0777);

	len += sprintf(&file_path[len], "/%u", ds->date);
	MKDIR(file_path, 0777);

	len += sprintf(&file_path[len], "/%u",  ds->id);
	MKDIR(file_path, 0777);

	if (ds->type == 1) {
		len += sprintf(&file_path[len], "/info.dat");
	} else if (ds->type == 2) {
		len += sprintf(&file_path[len], "/detail.dat");
	} else {
		log_err(false, "invalid jorney history type %u\n", ds->type);
		return;
	}

	if ((fp = fopen(file_path, "w+")) == NULL) {
        log_err(false, "open file %s failed\n", file_path);
        return ;
    }

	hd.magic = INDIACAR_HISTORY_MAGIC;
	hd.type = ds->type;
	hd.id = ds->id;
	hd.ver = ds->ver;
	hd.date = ds->date;
	hd.data_len = ds->total_size;

	// header
	fwrite(&hd, sizeof(hd), 1, fp);
	// data
	fwrite(ds->data, ds->total_size, 1, fp); 

	fclose(fp);

	log_debug("write flash to %s done\n", file_path);
}

static bool indiacar_do_history_get_push(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;
	int len = 0;
	india_car_history_notify_t *push_data;
	history_download_stat_t *ds;
	cl_indiacar_jorney_num_t *jn;
	user_t *user = ac->sac->user;
	u_int32_t payload_size = obj->param_len - sizeof(india_car_history_notify_t);
	
	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	ds = &priv_info->ds;
	jn = &priv_info->jn;
	
	push_data = (india_car_history_notify_t *)&obj[1];

	india_car_history_push_hd_order(push_data);

	log_debug("history push jid %u total_size %u idx %u payload_size %u\n", push_data->id, push_data->data_len, push_data->data_idx, payload_size);

	// 第一次收到数据
	if (ds->total_size == 0) {
		if (ds->id != push_data->id) {
			log_debug("jorney id changed %u => %u\n", ds->id, push_data->id);
			ds->id = push_data->id;
		}

		if (ds->date != push_data->date) {
			log_debug("date changed %u => %u\n", ds->date, push_data->date);
			ds->date = push_data->date;
		}
		
		// 
		if (push_data->data_idx != 0) {
			log_err(false, "first data, but data_idx=0\n");
			return false;
		}

		ds->data = cl_calloc(1, push_data->data_len);
		if (!ds) {
			log_err(true, "calloc history data cache len %u failed\n", push_data->data_len);
			goto failed;
		} else {
			log_debug("alloc %u data len for recv push data\n", push_data->data_len);
		}

		ds->total_size = push_data->data_len;
		ds->last_idx = 0;
		ds->ver = push_data->ver;
		ds->type = push_data->type;
	} else {
		if (ds->ver != push_data->ver) {
			log_err(false, "ver changed %u => %u\n", ds->ver, push_data->ver);
			goto failed;
		}
	
		if (ds->type != push_data->type) {
			log_err(false, "type changed %u => %u\n", ds->type, push_data->type);
			goto failed;
		}

		if (ds->date != push_data->date) {
			log_err(false, "date changed %u => %u\n", ds->date, push_data->date);
			goto failed;
		}
	
		if (ds->id != push_data->id) {
			log_err(false, "jorney id changed %u => %u\n", ds->id, push_data->id);
			goto failed;
		}

		// 可能重传报文
		if (push_data->data_idx < ds->last_idx) {
			return false;
		}

		if (ds->last_idx != push_data->data_idx) {
			log_err(false, "ds->last_idx %u != push_data->data_idx %u\n", ds->last_idx, push_data->data_idx);
			goto failed;
		}

		if (ds->total_size != push_data->data_len) {
			log_err(false, "total_size changed %u => %u\n", ds->total_size, push_data->data_len);
			goto failed;
		}

		if (push_data->data_idx + payload_size > push_data->data_len) {
			log_err(false, "data_idx %u + payload_size %u > data_len %u\n", push_data->data_idx, payload_size, push_data->data_len);
			goto failed;
		}
	}

	if (ds->last_idx + payload_size >= ds->total_size) {
		len = ds->total_size - ds->last_idx;
	} else {
		len = payload_size;
	}
	
	memcpy(&ds->data[ds->last_idx], (u_int8_t*)&push_data[1], len);

	ds->last_idx += payload_size;

	log_debug("now last_idx %u total_size %u\n", ds->last_idx, ds->total_size);

	if (ds->last_idx < ds->total_size) {
		// 没收到完整数据前都返回false
		return false;
	}

	// 写FLASH
	indiacar_history_write_flash(user->sn, ds);

	// 通知上层历史事件发生变化了
	if (ds->type == 1) {
		event_push(user->callback, UE_INDIACAR_HISOTRY_INFO_UPDATE, user->handle, user->callback_handle);
	} else if (ds->type == 2) {
		event_push(user->callback, UE_INDIACAR_HISOTRY_DETAIL_UPDATE, user->handle, user->callback_handle);
	}

	indiacar_history_download_init(ds);
	
	return true;
	
failed:
	indiacar_history_download_init(ds);
	return false;
}

static bool indiacar_do_history(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;
	history_download_stat_t *ds;
	u_int32_t now = (u_int32_t)time(NULL);

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	ds = &priv_info->ds;

	//log_debug("get indiacar history, action %u param_len %u\n", action, obj->param_len);
	if (ds->last_time == 0) {
		ds->last_time = now;
	} else {
		if (now - ds->last_time >= 10) {
			log_err(false, "get history timeout now %u last_time %u\n", now, ds->last_time);
			indiacar_history_download_init(ds);
		} else {
			ds->last_time = now;
		}
	}

	log_debug("	do histroy action %u ds->data %p\n", action, ds->data);

	// 处理请求下载时候，对端回复的命令
	if (action == UCA_GET) {
		return indiacar_do_history_get_reply(ac, action, obj);
	} else if (action == UCA_PUSH) {
		return indiacar_do_history_get_push(ac, action, obj);
	}
	
	return false;
}

static bool indiacar_do_dev_upgrade(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;
	india_car_dev_upgrade_push_t *upgrade_stat;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	upgrade_stat = (india_car_dev_upgrade_push_t *)&obj[1];

	log_debug("get indiacar_do_dev_upgrade, action %u param_len %u\n", action, obj->param_len);

	if (action != UCA_PUSH) {
		return false;
	}

	upgrade_stat->svn = ntohs(upgrade_stat->svn);
	upgrade_stat->data_len = ntohl(upgrade_stat->data_len);

	memcpy((u_int8_t*)(&priv_info->upgrade_stat), (u_int8_t*)&obj[1], sizeof(priv_info->upgrade_stat));
	//	
	event_push(ac->sac->user->callback, UE_INDIACAR_DEV_UPGRADE, ac->sac->user->handle, ac->sac->user->callback_handle);
	
	return true;
}

void india_car_store_stat_order(india_car_store_stat_t *store_stat)
{
	store_stat->tf_total = ntohs(store_stat->tf_total);
	store_stat->tf_left = ntohs(store_stat->tf_left);
	store_stat->flash_total = ntohs(store_stat->flash_total);
}

static RS indiacar_realtime_order_and_check(india_car_dev_realtime_trip_hd_t *hdr, int len)
{
	int n, i;
//	india_car_dev_realtime_trip_item_t *new_item, *item;
	
	if (len < sizeof(*hdr)) {
		log_err(false, "invalid realtime len %d\n");
		return RS_ERROR;
	}

	hdr->date = ntohl(hdr->date);
	hdr->len = ntohs(hdr->len);
	hdr->start_idx = ntohs(hdr->start_idx);
	hdr->end_idx = ntohs(hdr->end_idx);

	if (hdr->end_idx <= hdr->start_idx) {
		log_err(false, "hdr->end_idx %u <= hdr->start_idx %u\n", hdr->end_idx, hdr->start_idx);
		return RS_ERROR;
	}

	if (hdr->end_idx > INDIACAR_REALTIME_ITEM_NUM) {
		log_err(false, "invalid end idx %u\n", hdr->end_idx);
		return RS_ERROR;
	}
	
	n = hdr->end_idx - hdr->start_idx;

	if (n * (int)sizeof(india_car_dev_realtime_trip_item_t) + (int)sizeof(*hdr) < len) {
		log_err(false, "invalid realtime len %d\n");
		return RS_ERROR;
	}

	for (i = 0; i < n; i++) {
		latitude_longitud_order(&hdr->data[i].longitude);
		latitude_longitud_order(&hdr->data[i].latitude);
	}
	
	return RS_OK;
}

static u_int32_t indiacar_calc_item_sum(india_car_dev_realtime_trip_item_t *item, int n)
{
	u_int32_t sum = 0, i, len = sizeof(*item) * n;
	u_int8_t *ptr = (u_int8_t *)item;

	for (i = 0; i < len; i++) {
		sum ^= ptr[i];
	}

	return sum;
}

static RS indiacar_load_realtime_trip(smart_air_ctrl_t* ac)
{
	cl_indiacar_info_t *priv_info;
	cl_indiacar_realtime_trip_t *rt;
	char path[256] = {0};
	int nread, need;
	indiacar_reatime_record_t record;
	u_int32_t checksum = 0;
	FILE *fp = NULL;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	rt = &priv_info->rt;

	log_debug("try load realtime trip car_stat_init %u realtime_trip_config_read %u\n", 
		priv_info->car_stat_init, priv_info->rt.realtime_trip_config_read);

	if (!priv_info->car_stat_init) {
		return RS_ERROR;
	}

	if (priv_info->rt.realtime_trip_config_read == true) {
		return RS_ERROR;
	}

	priv_info->rt.realtime_trip_config_read = true;

	log_debug("indiacar try load reatime trip config\n");

	// 分配空间
	rt->num = INDIACAR_REALTIME_ITEM_NUM;
	rt->items = cl_calloc(1, sizeof(india_car_dev_realtime_trip_item_t) * rt->num);
	if (!rt->items) {
		rt->num = 0;
		log_err(true, "calloc rt item failed\n");
		return false;
	}

	sprintf(path, "%s/%"PRIu64"/realtime.conf", cl_priv->dir, ac->sac->user->sn);

	if ((fp = fopen(path, "rb")) == NULL) {
		log_err(false, "realtime file [%s] not created..\n", path);
		goto err;
	}

	nread = (int)fread((u_int8_t*)&record, 1, sizeof(record), fp);
	if (sizeof(record) != nread) {
		log_err(false, "need %u, but get %u\n", sizeof(record), nread);
		goto err;
	}

	if (record.max_write_idx > INDIACAR_REALTIME_ITEM_NUM) {
		log_err(false, "invalid max_write_idx %u\n", record.max_write_idx);
		goto err;
	}

	if (record.min_request_idx > INDIACAR_REALTIME_ITEM_NUM) {		
		log_err(false, "invalid min_request_idx %u\n", record.min_request_idx);
		goto err;
	}

	// 判断保存的数据是否和当前旅程符合
	if (record.date != priv_info->car_stat.journey_date || record.jid != priv_info->car_stat.id) {
		log_err(false, "record date %u id %u != current date %u id %u\n", record.date, record.jid, 
			priv_info->car_stat.journey_date, priv_info->car_stat.id);
		goto err;
	}

	// 读取item
	need = record.max_write_idx * sizeof(india_car_dev_realtime_trip_item_t);
	
	nread = (int)fread((u_int8_t*)rt->items, 1, need, fp);

	if (nread < need) {
		log_err(false, "read item failed nread %d need %d\n", nread, need);
		goto err;
	}

	// 读取checksum
	nread = (int)fread((u_int8_t*)&checksum, 1, sizeof(checksum), fp);
	if (nread != sizeof(checksum)) {
		log_err(false, "no checksum\n");
		goto err;
	}

	if (indiacar_calc_item_sum(rt->items, record.max_write_idx) != checksum) {
		log_err(false, "checksum failed calc 0x%x, real 0x%x\n", indiacar_calc_item_sum(rt->items, record.max_write_idx), checksum);
		goto err;
	}

	fclose(fp);

	rt->date = record.date;
	rt->jid = record.jid;
	rt->min_request_idx = record.min_request_idx;
	rt->max_write_idx = record.max_write_idx;
	
	log_debug("load indiacar realtime trip: date %u jid %u max_write_idx %u min_request_min %u\n", 
		record.date, record.jid, record.max_write_idx, record.min_request_idx);

	rt->last_start_idx = 0;
	rt->last_end_idx = rt->min_request_idx;

	rt->max_items_end_idx = rt->last_end_idx;

	log_debug(" call app %u to %u received\n", rt->last_start_idx, rt->last_end_idx);
	event_push(ac->sac->user->callback, UE_INDIACAR_REALTIME_TRIP, ac->sac->user->handle, ac->sac->user->callback_handle);

	return RS_OK;
err:
	if (fp) {
		fclose(fp);
	}

	memset((u_int8_t*)rt->items, 0x00, sizeof(india_car_dev_realtime_trip_item_t) * rt->num);
	
	return RS_ERROR;
}


static void indiacar_save_realtime_trip(smart_air_ctrl_t* ac)
{
	cl_indiacar_info_t *priv_info;
	cl_indiacar_realtime_trip_t *rt;
	char path[256] = {0};
	int n;
	indiacar_reatime_record_t record;
	u_int32_t checksum = 0;
	FILE *fp = NULL;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	rt = &priv_info->rt;

	//if (rt->max_write_idx == 0) {
	//	return;
	//}

	sprintf(path, "%s/%"PRIu64"/realtime.conf", cl_priv->dir, ac->sac->user->sn);

	if ((fp = fopen(path, "w+b")) == NULL) {
		log_err(false, "open realtime write file failed\n", path);
		goto err;
	}

	record.date = rt->date;
	record.jid = rt->jid;
	record.max_write_idx = rt->max_write_idx;
	record.min_request_idx = rt->min_request_idx;

	n = (int)fwrite((u_int8_t*)&record, 1, sizeof(record), fp);
	if (n != sizeof(record)) {
		log_err(true, "write realtime header failed\n");
		goto err;
	}

	n = (int)fwrite((u_int8_t*)rt->items, 1, rt->max_write_idx * sizeof(india_car_dev_realtime_trip_item_t), fp);
	if (n <= 0) {
		log_err(true, "write realtime items failed n %d\n", n);
		goto err;
	}

	checksum = indiacar_calc_item_sum(rt->items, rt->max_write_idx);

	// 最后写校验
	fwrite((u_int8_t*)&checksum, 1, sizeof(checksum), fp);

	log_debug("save realtime trip done crc 0x%x\n", checksum);
err:
	if (fp) {
		fclose(fp);
	}
}

static void indiacar_request_realtime_trip_idx(smart_air_ctrl_t* ac, u_int32_t start_idx, u_int32_t end_idx)
{
	cl_indiacar_info_t *priv_info;
	cl_indiacar_realtime_trip_t *rt;	
	india_car_dev_realtime_requst_t request;
	int dest_pkt_len = sizeof(request);
	u_int8_t buf[128] = {0};
	ucp_obj_t *uo = (ucp_obj_t *)buf;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	rt = &priv_info->rt;

	rt->is_geting = true;

	log_debug(" >>> request realtime trip from %u to %u\n", start_idx, end_idx);

	request.type = 1;	// 请求实时数据
	request.start_idx = ntohs((u_int16_t)start_idx);
	request.end_idx = ntohs((u_int16_t)end_idx);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_REALTIME_TRIP, dest_pkt_len);
	memcpy(uo + 1, &request, sizeof(request));
	
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
}

static void indiacar_try_get_history_realtime(smart_air_ctrl_t *ac)
{
	cl_indiacar_info_t *priv_info;
	cl_indiacar_realtime_trip_t *rt;
	u_int32_t i, end;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	rt = &priv_info->rt;
	
	// 继续请求下一段
	for (i = rt->min_request_idx; i < rt->max_write_idx; i++) {
		if (rt->items[i].stat == 0) {
			break;
		}
	}

	log_debug("update min request idx to %u\n", rt->min_request_idx);
	
	rt->min_request_idx = i;

	if (i == rt->max_write_idx) {
		log_debug("now have get all history realtime at idx %u\n", rt->min_request_idx);
		rt->is_geting = false;
		return;
	}

	end = rt->min_request_idx + INDIACAR_REALTIME_ONE_PACKET_ITEM;
	if (end > rt->max_write_idx) {
		end = rt->max_write_idx;
	}

	indiacar_request_realtime_trip_idx(ac, i, end);
}

static bool indiacar_do_car_realtime_trip_get(smart_air_ctrl_t* ac, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;
	india_car_dev_realtime_trip_hd_t *hdr;
	cl_indiacar_realtime_trip_t *rt;

	hdr = (india_car_dev_realtime_trip_hd_t *)&obj[1];

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	rt = &priv_info->rt;

	if (rt->is_geting == false) {
		log_err(false, "not getting realtime\n");
		return false;
	}

	// 有错误
	if (obj->param_len == 4) {		
		event_push(ac->sac->user->callback, UE_INDIACAR_REALTIME_TRIP_ERR, ac->sac->user->handle, ac->sac->user->callback_handle);
		return false;
	}

	if (rt->date != hdr->date) {
		log_err(false, "get realtime get, local date %u != hdr date %u\n", rt->date, hdr->date);
		return false;
	}

	if (hdr->start_idx != rt->min_request_idx) {
		log_err(false, "need idx %u, but get %u\n", rt->min_request_idx, hdr->start_idx);
		return false;
	}

	if (hdr->end_idx < hdr->start_idx) {
		log_err(false, "xxx\n");
		return false;
	}

	memcpy(&rt->items[hdr->start_idx], hdr->data, (hdr->end_idx - hdr->start_idx) * sizeof(india_car_dev_realtime_trip_item_t));

	rt->last_start_idx = hdr->start_idx;
	rt->last_end_idx = hdr->end_idx;

	rt->min_request_idx = rt->last_end_idx;

	log_debug("########## call app %u to %u received\n", rt->last_start_idx, rt->last_end_idx);
	event_push(ac->sac->user->callback, UE_INDIACAR_REALTIME_TRIP, ac->sac->user->handle, ac->sac->user->callback_handle);
	//indiacar_save_realtime_trip(ac);	

	// 继续请求下一段
	indiacar_try_get_history_realtime(ac);
	
	return false;
}

static void indiacar_realtime_trip_free(cl_indiacar_realtime_trip_t *rt)
{
	if (rt->items) {
			cl_free(rt->items);
	}
	memset(rt, 0x00, sizeof(*rt));
}

static bool indiacar_do_car_realtime_trip(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;
	india_car_dev_realtime_trip_hd_t *hdr;
	cl_indiacar_realtime_trip_t *rt;
	u_int32_t num;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	rt = &priv_info->rt;
	
	hdr = (india_car_dev_realtime_trip_hd_t *)&obj[1];

	if (indiacar_realtime_order_and_check(hdr, obj->param_len) != RS_OK) {
		log_err(false, "realtime check failed\n");
		return false;
	}
	
	log_debug("reatime trip dump: action %u date %u jid %u start %u end %u\n", action, hdr->date, hdr->jid, hdr->start_idx, hdr->end_idx);

	log_debug("local: max_idx %u need_realtime = %u date %u jid %u min_request_idx %u is_geting %u\n", 
		rt->max_write_idx, rt->need_realtime, rt->date, rt->jid, rt->min_request_idx, rt->is_geting);

	if (rt->need_realtime == false) {
		log_err(false, "not need realtime, ignore\n");
		return false;
	}

	if (action == UCA_GET) {
		return indiacar_do_car_realtime_trip_get(ac, obj);
	}

	if (action != UCA_PUSH) {
		return false;
	}

		
	//  当前旅程结束
	if (hdr->type == 4) {
		log_debug("realtime trip is over\n");
		indiacar_realtime_trip_free(rt);
		indiacar_save_realtime_trip(ac);
        event_push(ac->sac->user->callback, UE_INDIACAR_REALTIME_TRIP_OVER, ac->sac->user->handle, ac->sac->user->callback_handle);
		return false;
	}


	// 旅程更新了
	if (rt->date != hdr->date || rt->jid != hdr->jid) {
		log_debug("realtime trip update to new one\n");
		memset((u_int8_t*)rt->items, 0x00, sizeof(india_car_dev_realtime_trip_item_t) * rt->num);

		rt->min_request_idx = rt->max_write_idx = rt->is_geting = 0;

		rt->date = hdr->date;
		rt->jid = hdr->jid;
	}

	if (rt->max_write_idx >= hdr->start_idx) {
		log_err(false, "invalid hdr->end_idx %u because max write idx %u\n", hdr->end_idx, rt->max_write_idx);
		return false;
	}

	if (rt->max_write_idx < hdr->start_idx) {
		rt->max_write_idx = hdr->start_idx;
		rt->max_items_end_idx = hdr->end_idx;
		log_debug("update max_idx to %u\n", rt->max_write_idx);
	} 

	if (rt->items == NULL) {		
		rt->num = INDIACAR_REALTIME_ITEM_NUM;
		rt->items = cl_calloc(1, sizeof(india_car_dev_realtime_trip_item_t) * rt->num);
		if (!rt->items) {
			rt->num = 0;
			log_err(true, "calloc rt item failed\n");
			return false;
		}
	}

	if (rt->is_geting == false) {
		indiacar_try_get_history_realtime(ac);
	}	
	
	num = (hdr->end_idx - hdr->start_idx);


	// 类型3为实时数据
	if (hdr->type != 3) {
		log_err(false, "invalid push type");
		return false;
	}

	memcpy(&rt->items[hdr->start_idx], hdr->data, num * sizeof(india_car_dev_realtime_trip_item_t));

	rt->last_start_idx = hdr->start_idx;
	rt->last_end_idx = hdr->end_idx;

	log_debug("########## call app %u to %u received\n", rt->last_start_idx, rt->last_end_idx);
	event_push(ac->sac->user->callback, UE_INDIACAR_REALTIME_TRIP, ac->sac->user->handle, ac->sac->user->callback_handle);
	indiacar_save_realtime_trip(ac);
	
	return true;
}

static bool indiacar_do_local_watch(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	int i;
	cl_indiacar_info_t *priv_info;
	india_car_local_watch_reply_t *reply = (india_car_local_watch_reply_t *)&obj[1];
	india_car_local_watch_push_t *push = (india_car_local_watch_push_t *)&obj[1];
	india_car_local_watch_info_t *wi;
	u_int8_t errcode;
	u_int32_t ip, port;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);
	wi = &priv_info->wi;

	if (action == UCA_GET) {
		if (is_obj_less_than_len(obj, sizeof(*reply))) {
			return false;
		}
		
		reply->ip = ntohl(reply->ip);
		reply->port = ntohs(reply->port);

		wi->errcode = reply->err;
		wi->ip = reply->ip;
		wi->port = reply->port;

		log_debug("india car get local ip %u port %u errcode %u\n", 
			wi->ip, wi->port, wi->errcode);
		
        event_push(ac->sac->user->callback, UE_INDIACAR_GET_LOCAL_WATCH_INFO, ac->sac->user->handle, ac->sac->user->callback_handle);

		return false;
	}

	if (action != UCA_PUSH) {
		return false;
	}

	if (is_obj_less_than_len(obj, sizeof(*push))) {
		return false;
	}

	push->date = ntohl(push->date);
	push->file_num = ntohs(push->file_num);

	log_debug("local watch push: total %u index %u file_num %u date %u ip %u port %u\n", 
		push->total, push->index, push->file_num, push->date, wi->ip, wi->port);

	if (is_obj_less_than_len(obj, push->file_num * 4 + sizeof(*push))) {
		log_err(false, "invalid file num\n");
		return false;
	}

	for (i = 0; i < push->file_num; i++) {
		push->file_name[i] = ntohl(push->file_name[i]);
	}

	if (push->index == 0) {
		errcode = wi->errcode;
		ip = wi->ip;
		port = wi->port;
		
		memset((u_int8_t*)wi, 0x00, sizeof(*wi));

		wi->errcode = errcode;
		wi->ip = ip;
		wi->port = port;
	} 

	if (wi->_write_index + push->file_num > 2048) {
		log_err(false, "write idx %u + file_num %u > 2048\n", wi->_write_index, push->file_num);
		goto err_out;
	}

	memcpy(&wi->_name[wi->_write_index], push->file_name, push->file_num * sizeof(wi->_name[0]));
	wi->_write_index += push->file_num;

	if (push->index + 1 == push->total) {
		memcpy(wi->name_list, wi->_name, sizeof(wi->name_list));
		wi->num = wi->_write_index;
		wi->year = (push->date >> 16) & 0xffff;
		wi->month = (push->date >> 8) & 0xff;
		wi->day = push->date & 0xff;

		log_debug("local watch get all, good\n");
		
        event_push(ac->sac->user->callback, UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST, ac->sac->user->handle, ac->sac->user->callback_handle);
	}

	return false;	

err_out:
	wi->_total = wi->_index = wi->_write_index = 0;

	return false;
}

static void indiacar_free(cl_indiacar_info_t *priv_info)
{
	ica_client_t *c = priv_info->icc;
	// 
	if (c) {
		icac_stop(c);
	}
}

static bool indiacar_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_indiacar_info_t *priv_info;

	priv_info = (cl_indiacar_info_t *)(ac->com_udp_dev_info.device_info);

	if (!priv_info) {
		log_err(false, "not initd cl_indiacar_info_t\n");
		return false;
	}
    
    switch (obj->attr) {
        case UCAT_IA_INDIA_CAR_STAT:
			if (is_obj_less_than_len(obj, sizeof(india_car_stat_t))) {
				return false;
			}

			memcpy(&priv_info->car_stat, &obj[1], sizeof(india_car_stat_t));
			
			india_car_stat_order(&priv_info->car_stat);

			log_debug("indiacar stat: id %u journey_date %u total_time %u\n", 
				priv_info->car_stat.id, priv_info->car_stat.journey_date, priv_info->car_stat.total_time);

			// 如果旅程结束了，之前的实时数据清空下
			if (priv_info->car_stat.id == 0 && priv_info->rt.items) {
				indiacar_realtime_trip_free(&priv_info->rt);
			}
			
			// 
			if (priv_info->car_stat_init == false) {
				priv_info->car_stat_init = true;
				if (priv_info->rt.need_realtime) {
					indiacar_load_realtime_trip(ac);
				}
			}
			
			break;
			
		case UCAT_IA_INDIA_CAR_DEV_STAT:
			if (is_obj_less_than_len(obj, sizeof(india_car_dev_stat_t))) {
				return false;
			}

			memcpy(&priv_info->dev_stat, &obj[1], sizeof(india_car_dev_stat_t));
			
			break;
		case UCAT_IA_INDIA_CAR_STORE_STAT:
			if (is_obj_less_than_len(obj, sizeof(india_car_store_stat_t))) {
				return false;
			}

			memcpy(&priv_info->store_stat, &obj[1], sizeof(india_car_store_stat_t));

			india_car_store_stat_order(&priv_info->store_stat);

			break;
		case UCAT_IA_INDIA_CAR_WARN:
			if (is_obj_less_than_len(obj, sizeof(india_car_warn_t))) {
				return false;
			}

			memcpy(&priv_info->warn, &obj[1], sizeof(india_car_warn_t));
			
			india_car_warn_order(&priv_info->warn);
			break;
		case UCAT_IA_INDIA_CAR_WIFI_CONFIG:
			if (is_obj_less_than_len(obj, 4)) {
				return false;
			}

			if (obj->param_len > sizeof(india_car_wifi_config_t)) {
				return false;
			}

			memcpy(&priv_info->wifi_config, &obj[1], obj->param_len);
			
			break;
		case UCAT_IA_INDIA_CAR_HISTORY:
			return indiacar_do_history(ac, action, obj);
			
		case UCAT_IA_INDIA_CAR_DEV_UPGRADE:
			return indiacar_do_dev_upgrade(ac, action, obj);

		case UCAT_IA_INDIA_CAR_REALTIME_TRIP:
			return indiacar_do_car_realtime_trip(ac, action, obj);

		case UCAT_IA_INDIA_CAR_DEBUG_CONFIG:
			if (is_obj_less_than_len(obj, sizeof(priv_info->dc) - sizeof(priv_info->dc.url))) {
				return false;
			}

			if (obj->param_len > sizeof(priv_info->dc)) {
				return false;
			}
			
			memcpy(&priv_info->dc, &obj[1], obj->param_len);
						
			india_car_debug_config_order(&priv_info->dc);

			break;

		case UCAT_IA_INDIA_CAR_AGENT_INFO:
			{
				india_car_video_agent_reply_t *reply;

				reply = (india_car_video_agent_reply_t *)&obj[1];

				if (is_obj_less_than_len(obj, sizeof(*reply))) {
					return false;
				}


				reply->agent_ip = ntohl(reply->agent_ip);
				reply->agent_port = ntohs(reply->agent_port);
				reply->select_enc = ntohs(reply->select_enc);

				log_debug("get video agent info: %u.%u.%u.%u:%u enc 0x%x\n", IP_SHOW(reply->agent_ip), reply->agent_port, reply->select_enc);

				icac_set_agent_info(priv_info->icc, reply->agent_ip, reply->agent_port, reply->select_enc, reply->key);
				
			}
			break;

		case UCAT_IA_INDIA_CAR_LOCAL_WATCH:
			return indiacar_do_local_watch(ac, action, obj);
			break;
    }

	return true;
}

static bool indiacar_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_indiacar_info_t *priv_info;
	video_t *video = NULL;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	int dest_pkt_len = 0;
	u_int8_t u8value = 0;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "_hx_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "indiacar_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;
	
	switch(info->action){
	// 请求历史数据
	case ACT_INDIACAR_REQUEST_HISTORY:
		{
			india_car_history_request_t *request = cci_pointer_data(info);

			request->date = ntohl(request->date);
			dest_pkt_len = sizeof(*request);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_HISTORY, dest_pkt_len);
			memcpy(uo + 1, request, sizeof(*request));

			if (ac->sac->user->online) {
			//if (0) {
	            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
			} else {
				uasc_ctrl_obj_value(ac->sac->user->sn, UCA_GET, 0x1, buf, sizeof(*uo) + dest_pkt_len);
			}
		}
		
		break;

	case ACT_INDIACAR_REQUEST_UPGRADE:
		{
			india_car_dev_upgrade_set_t *request = cci_pointer_data(info);
			dest_pkt_len = info->data_len;

			request->url_len = ntohs(request->url_len);
			request->svn = ntohl(request->svn);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_DEV_UPGRADE, dest_pkt_len);
			memcpy(uo + 1, request, info->data_len);

            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_INDIACAR_REQUEST_WARN_SETTING:
		{			
			india_car_warn_t *request = cci_pointer_data(info);
			dest_pkt_len = info->data_len;

			india_car_warn_order(request);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_WARN, dest_pkt_len);
			memcpy(uo + 1, request, info->data_len);

            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_INDIACAR_REQUEST_WIFI_CONFIG:
		{			
			india_car_wifi_config_t *request = cci_pointer_data(info);
			dest_pkt_len = info->data_len;
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_WIFI_CONFIG, dest_pkt_len);
			memcpy(uo + 1, request, info->data_len);

            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_INDIACAR_REQUEST_REALTIME_TRIP:
		{			
			india_car_dev_realtime_requst_t request;

			memset((u_int8_t*)&request, 0x00, sizeof(request));
			dest_pkt_len = sizeof(request);
			u8value = cci_u8_data(info);
			if (u8value == 1) {
				request.type = 1;	// 请求实时数据
				priv_info->rt.need_realtime = true;

				indiacar_load_realtime_trip(ac);
			} else {
				request.type = 2;	// 结束实时数据
				
				indiacar_realtime_trip_free(&priv_info->rt);
				log_debug("@@@@@@@@@@ stop realtime request\n");
			}

			log_debug("realtime trip request type %u\n", request.type);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_REALTIME_TRIP, dest_pkt_len);
			memcpy(uo + 1, &request, sizeof(request));

            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
	case ACT_INDIACAR_REQUEST_DEBUG_CONFIG:
		{			
			india_car_debug_config_t *request = cci_pointer_data(info);
			dest_pkt_len = info->data_len;

			india_car_debug_config_order(request);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_DEBUG_CONFIG, dest_pkt_len);
			memcpy(uo + 1, request, info->data_len);

			sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_INDIACAR_REQUEST_VIDEO:
		{
			u_int8_t onoff = cci_u8_data(info);

			if (onoff) {
				log_debug("ACT_INDIACAR_REQUEST_VIDEO video %p\n", video);
				icac_start(user, (ica_client_t**)&(priv_info->icc));

				// 请求设备看视频
				log_debug("reqeust dev video agent\n");
				sa_query_obj(ac->sac->user->uc_session, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_AGENT_INFO);
			} else {
				log_debug("ACT_INDIACAR_REQUEST_VIDEO video off\n");
				icac_stop(priv_info->icc);
			}
		}
		break;

	case ACT_INDIACAR_REQUEST_LOCAL_VIDEO_WATCH:
		{
			u_int8_t *request = cci_pointer_data(info);
			dest_pkt_len = info->data_len;
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_INDIA_CAR, UCAT_IA_INDIA_CAR_LOCAL_WATCH, dest_pkt_len);
			memcpy(uo + 1, request, info->data_len);

			sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_INDIACAR_REQUEST_RECORD:
		{
			u_int8_t path[256] = {0}, onoff = true;
			
			if (icac_is_working(priv_info->icc) == false) {
				res = RS_NOT_INIT;
				break;
			}

			memcpy(path, cci_pointer_data(info), sizeof(path));

			if (path[0] == 0) {
				onoff = false;
			}
			
			icac_set_record(priv_info->icc, path, onoff);
		}
		break;

	case ACT_INDIACAR_REQUEST_DECODE_MP4:
		{
			cl_indiancar_mp4_decode_request_t *request = cci_pointer_data(info);

			ica_do_mp4_decode(user, (ica_mp4_decode_t**)&priv_info->mp4_decode, request->seek, request->path, request->action);
		}
		break;
	default:
		break;
	}
	
	return res;
}

bool indiacar_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_indiacar_info_t* priv;
	cl_indiacar_info_t* info;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	priv = (cl_indiacar_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void linkon_stat_order(cl_linkon_stat_t *stat)
{
	stat->house_temp = ntohs(stat->house_temp);
	stat->const_temp = ntohs(stat->const_temp);
	stat->go_out_temp = ntohs(stat->go_out_temp);
	stat->save_temp = ntohs(stat->save_temp);
}

static bool zhcl_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	u_int8_t *pvalue;
	ucp_zhcl_bind_t *pbind;
	ucp_zhcl_type_t *ptype;
	cl_zhcl_info_t *priv_info;

	priv_info = (cl_zhcl_info_t *)(ac->com_udp_dev_info.device_info);

	if (!priv_info) {
		log_err(false, "not initd zhcl_update_data\n");
		return false;
	}
    
    switch (obj->attr) {
    case UCAT_IA_ZHCL_STATUS_SET:
		if (is_obj_less_than_len(obj, sizeof(u_int8_t))) {
			return false;
		}
		pvalue = (u_int8_t *)&obj[1];
		priv_info->status = *pvalue;
		break;
	case UCAT_IA_ZHCL_LOCATION:
		if (is_obj_less_than_len(obj, sizeof(u_int8_t))) {
			return false;
		}
		pvalue = (u_int8_t *)&obj[1];
		priv_info->percent = *pvalue;
		break;
	case UCAT_IA_ZHCL_BIND:
		if (is_obj_less_than_len(obj, sizeof(ucp_zhcl_bind_t))) {
			return false;
		}
		pbind = (ucp_zhcl_bind_t *)&obj[1];
		pbind->magic = htonl(pbind->magic);
		priv_info->magic = pbind->magic;
		priv_info->index = pbind->index;
		priv_info->type = pbind->type;
		break;
	case UCAT_IA_ZHCL_TYPE_SET:
		if (is_obj_less_than_len(obj, sizeof(ucp_zhcl_type_t))) {
			return false;
		}
		ptype = (ucp_zhcl_type_t *)&obj[1];
		priv_info->index = ptype->index;
		priv_info->type = ptype->type;
		break;

	case UCAT_IA_ZHCL_DIR_SET:
		if (is_obj_less_than_len(obj, sizeof(u_int8_t))) {
			return false;
		}
		pvalue = (u_int8_t *)&obj[1];
		priv_info->dir = *pvalue;
		priv_info->support_dir = 1;
		break;
	default:
		break;
    }

	return true;
}

static bool zhdhx_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_zhdhx_info_t *priv_info;
	ucp_zhdhx_onoff_t *ponoff;
	ucp_zhdhx_name_set_t *pname;
	cl_zhdhx_key_name_t *pkn;

	priv_info = (cl_zhdhx_info_t *)(ac->com_udp_dev_info.device_info);

	if (!priv_info) {
		log_err(false, "not initd %s\n", __FUNCTION__);
		return false;
	}
    
    switch (obj->attr) {
    case UCAT_IA_ZHDHX_ONOFF:
		if (is_obj_less_than_len(obj, sizeof(*ponoff))) {
			log_debug("err len=%u needlen=%u %s %d\n", 
				obj->param_len, sizeof(*ponoff), __FUNCTION__, __LINE__);
			return false;
		}
		ponoff = (ucp_zhdhx_onoff_t *)&obj[1];
		priv_info->on_off_stat = ponoff->onoff&ponoff->mask;
		log_debug("on=%x mask=%x\n", ponoff->onoff, ponoff->mask);
		break;
	case UCAT_IA_ZHDHX_NAME:
		if (is_obj_less_than_len(obj, sizeof(*pname))) {
			log_debug("err len=%u needlen=%u %s %d\n", 
				obj->param_len, sizeof(*pname), __FUNCTION__, __LINE__);
			return false;
		}
		pname = (ucp_zhdhx_name_set_t *)&obj[1];
		if ((pname->key_num == 0) ||
			(pname->key_num > ((BIT(priv_info->dhx_type)-1)&0xff)) ||
			(pname->key_num > ZHDHX_KEY_NAME_MAX)) {
			log_err(false, "err key_num=%u\n", pname->key_num);
			return false;
		}
		if (pname->key_len > ZHDHX_NAME_MAX) {
			log_err(false, "err keylen=%u > 64\n", pname->key_len);
		}
		pkn = &priv_info->key_name[pname->key_num - 1];
		pkn->valid = true;
		memset((void *)pkn->name, 0, sizeof(pkn->name));
		memcpy((void *)pkn->name, (void *)pname->name, pname->key_len);
		pkn->name[ZHDHX_NAME_MAX - 1] = 0;
		log_debug("keynum=%u name=%s\n", pname->key_num, pname->name);
		break;
	default:
		break;
    }

	return true;
}

static bool linkon_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_linkon_info_t *priv_info;

	priv_info = (cl_linkon_info_t *)(ac->com_udp_dev_info.device_info);

	if (!priv_info) {
		log_err(false, "not initd cl_indiacar_info_t\n");
		return false;
	}
    
    switch (obj->attr) {
        case UCAT_IA_LINKONWKQ_QUERY:
			if (is_obj_less_than_len(obj, sizeof(cl_linkon_stat_t))) {
				return false;
			}
			
			memcpy(&priv_info->stat, &obj[1], sizeof(cl_linkon_stat_t));
			
			linkon_stat_order(&priv_info->stat);
			
			break;
    }

	return true;
}

static bool zhcl_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_zhcl_info_t *priv_info;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	int dest_pkt_len = 0;
	u_int8_t u8value = 0;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "zhcl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "zhcl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;

	switch(info->action){
	case ACT_ZHCL_STATUS:
		{
			u_int8_t status[4];

			memset(status, 0x00, sizeof(status));

			status[0] = (u_int8_t)cci_u8_data(info);
			dest_pkt_len = sizeof(status);
			switch(status[0]) {
			case ZHCL_STATUS_OPEN:
			case ZHCL_STATUS_STOP:
			case ZHCL_STATUS_CLOSE:
				break;
			default:
				log_debug("zhcl_proc_notify err value status=%u\n", status[0]);
				*ret= RS_INVALID_PARAM;
				return res;
			}
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHCL, UCAT_IA_ZHCL_STATUS_SET, dest_pkt_len);
			memcpy(uo + 1, (u_int8_t*)status, sizeof(status));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);

			priv_info->status = status[0];
		}
		break;
	case ACT_ZHCL_LOCATION:
		{
			u_int8_t location[4];

			memset(location, 0x00, sizeof(location));

			location[0] = (u_int8_t)cci_u8_data(info)%101;
			dest_pkt_len = sizeof(location);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHCL, UCAT_IA_ZHCL_LOCATION, dest_pkt_len);
			memcpy(uo + 1, (u_int8_t*)location, sizeof(location));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);

			priv_info->percent = location[0];
		}
		break;
	case ACT_ZHCL_BIND:
		{
			ucp_zhcl_bind_t *pbind;

			pbind = cci_pointer_data(info);
			dest_pkt_len = sizeof(*pbind);

			//index check
			if (pbind->index != ZHCL_INDEX_1 &&
				pbind->index != ZHCL_INDEX_2) {
				log_debug("zhcl_proc_notify err index=%u\n", pbind->index);
				*ret = RS_INVALID_PARAM;
				return res;
			}
			//type check
			if (pbind->type != ZHCL_TYPE_1 &&
				pbind->type != ZHCL_TYPE_2) {
				log_debug("zhcl_proc_notify err type=%u\n", pbind->type);
				*ret = RS_INVALID_PARAM;
				return res;
			}
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHCL, UCAT_IA_ZHCL_BIND, dest_pkt_len);
			pbind->magic = htonl(pbind->magic);
			memcpy(uo + 1, (u_int8_t*)pbind, sizeof(*pbind));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
	case ACT_ZHCL_TYPE:
		{
			ucp_zhcl_type_t *ptype;

			ptype = cci_pointer_data(info);
			dest_pkt_len = sizeof(*ptype);

			//index check
			if (ptype->index != ZHCL_INDEX_1 &&
				ptype->index != ZHCL_INDEX_2) {
				log_debug("zhcl_proc_notify err index=%u\n", ptype->index);
				*ret = RS_INVALID_PARAM;
				return res;
			}
			//type check
			if (ptype->type != ZHCL_TYPE_1 &&
				ptype->type != ZHCL_TYPE_2) {
				log_debug("zhcl_proc_notify err type=%u\n", ptype->type);
				*ret = RS_INVALID_PARAM;
				return res;
			}
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHCL, UCAT_IA_ZHCL_TYPE_SET, dest_pkt_len);
			memcpy(uo + 1, (u_int8_t*)ptype, sizeof(*ptype));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
	case ACT_ZHCL_DIR:
		{
			u_int8_t dir[4];
			
			memset(dir, 0x00, sizeof(dir));
			dir[0] = (u_int8_t)cci_u8_data(info);
			dest_pkt_len = sizeof(dir);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHCL, UCAT_IA_ZHCL_DIR_SET, dest_pkt_len);
			memcpy(uo + 1, (u_int8_t*)dir, sizeof(dir));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
			priv_info->dir = dir[0];
		}
		break;
	default:
		break;
	}

	return res;
}

static bool zhdhx_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_zhdhx_info_t *priv_info;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	u_int8_t *pdata = NULL;
	u_int8_t dest_pkt_len = 0;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
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
	
	priv_info = ac->com_udp_dev_info.device_info;

	switch(info->action){
	case ACT_ZHDHX_ONOFF:
		{
			u_int8_t onoff_value[2];
			u_int8_t num = 0;
			u_int8_t value = 0;

			memset(onoff_value, 0, sizeof(onoff_value));
			dest_pkt_len = sizeof(onoff_value);
			
			if (priv_info->dhx_type == 0) {
				*ret = RS_NOT_SUPPORT;
				return true;
			}
			pdata = cci_pointer_data(info);
			num = pdata[0];
			value = pdata[1];
			
			if (num > priv_info->dhx_type) {
				*ret = RS_INVALID_PARAM;
				return true;
			}
			if (num == 0) {
				onoff_value[1] = (BIT(priv_info->dhx_type) - 1)&0xff;
			} else {
				onoff_value[1] = BIT((num - 1));
			}
			onoff_value[0] = (value != 0)?(onoff_value[1]):0;
				
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHDHX, UCAT_IA_ZHDHX_ONOFF, dest_pkt_len);
			memcpy(uo + 1, onoff_value, dest_pkt_len);
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
	case ACT_ZHDHX_KEY_NAME:
		{
			u_int8_t name_buf[100];
			u_int8_t num = 0;
			u_int8_t name_len = 0;
			u_int8_t *pname = NULL;

			if (priv_info->dhx_type == 0) {
				*ret = RS_NOT_SUPPORT;
				return true;
			}
			pdata = cci_pointer_data(info);
			num = pdata[0];
			name_len = pdata[1];
			pname = &pdata[2];
			if (num > priv_info->dhx_type) {
				*ret = RS_INVALID_PARAM;
				return true;
			}
			if ((name_len == 0) ||
				(name_len > ZHDHX_NAME_MAX)) {
				*ret = RS_INVALID_PARAM;
				return true;
			}
			name_buf[0] = num;
			name_buf[1]= name_len;
			memcpy((char *)&name_buf[2], pname, name_len);
			dest_pkt_len = 2 + name_len;
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ZHDHX, UCAT_IA_ZHDHX_NAME, dest_pkt_len);
			memcpy(uo + 1, name_buf, dest_pkt_len);
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
	default:
		break;
	}

	return true;
}

static bool linkon_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_linkon_info_t *priv_info;
	video_t *video = NULL;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	int dest_pkt_len = 0;
	u_int8_t u8value = 0;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "_hx_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "linkon_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;

	switch(info->action){
	case ACT_LINKON_POWER:
		{
			ucp_linkon_set_t request;

			memset((u_int8_t*)&request, 0x00, sizeof(request));

			request.value = (u_int8_t)cci_u32_data(info);
			dest_pkt_len = sizeof(request);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LINKONWKQ, UCAT_IA_LINKONWKQ_POWER, dest_pkt_len);
			memcpy(uo + 1, (u_int8_t*)&request, sizeof(request));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);

			priv_info->stat.power = request.value;
		}
		break;
		
	case ACT_LINKON_LOCK:
		{
			ucp_linkon_set_t request;

			memset((u_int8_t*)&request, 0x00, sizeof(request));

			request.value = (u_int8_t)cci_u32_data(info);
			dest_pkt_len = sizeof(request);
			
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LINKONWKQ, UCAT_IA_LINKONWKQ_LOCK, dest_pkt_len);
			memcpy(uo + 1, (u_int8_t*)&request, sizeof(request));
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);

			priv_info->stat.lock = request.value;
		}
		break;

	case ACT_LINKON_WORK_MODE:
			{
				ucp_linkon_set_t request;
	
				memset((u_int8_t*)&request, 0x00, sizeof(request));
	
				request.value = (u_int8_t)cci_u32_data(info);
				dest_pkt_len = sizeof(request);
				
				fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LINKONWKQ, UCAT_IA_LINKONWKQ_WORK_MODE, dest_pkt_len);
				memcpy(uo + 1, (u_int8_t*)&request, sizeof(request));
				sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
	
				priv_info->stat.work_mode = request.value;
			}
			break;

	case ACT_LINKON_RUNNING_MODE:
			{
				ucp_linkon_set_t request;
	
				memset((u_int8_t*)&request, 0x00, sizeof(request));
	
				request.value = (u_int8_t)cci_u32_data(info);
				dest_pkt_len = sizeof(request);
				
				fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LINKONWKQ, UCAT_IA_LINKONWKQ_RUNNING_MODE, dest_pkt_len);
				memcpy(uo + 1, (u_int8_t*)&request, sizeof(request));
				sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
	
				priv_info->stat.running_mode = request.value;
			}
			break;

	case ACT_LINKON_WIND_SPEED:
			{
				ucp_linkon_set_t request;
	
				memset((u_int8_t*)&request, 0x00, sizeof(request));
	
				request.value = (u_int8_t)cci_u32_data(info);
				dest_pkt_len = sizeof(request);
				
				fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LINKONWKQ, UCAT_IA_LINKONWKQ_WIND, dest_pkt_len);
				memcpy(uo + 1, (u_int8_t*)&request, sizeof(request));
				sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
	
				priv_info->stat.wind_speed = request.value;
			}
			break;

	case ACT_LINKON_TEMP:
			{
				ucp_linkon_temp_set_t request;
	
				memset((u_int8_t*)&request, 0x00, sizeof(request));
	
				request.value = (u_int16_t)cci_u32_data(info);
				request.value = ntohs(request.value);
				dest_pkt_len = sizeof(request);
				
				fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LINKONWKQ, UCAT_IA_LINKONWKQ_TEMP, dest_pkt_len);
				memcpy(uo + 1, (u_int8_t*)&request, sizeof(request));
				sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);

				if (priv_info->stat.work_mode == 0) {
					priv_info->stat.const_temp = (u_int16_t)cci_u32_data(info);
				} else if (priv_info->stat.work_mode == 1) {
					priv_info->stat.go_out_temp = (u_int16_t)cci_u32_data(info);
				} else if (priv_info->stat.work_mode == 2) {
					priv_info->stat.save_temp = (u_int16_t)cci_u32_data(info);
				}
			}
			break;
	default:
		break;
	}
	
	return res;
}

bool linkon_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_indiacar_info_t* priv;
	cl_indiacar_info_t* info;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	priv = (cl_indiacar_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if (!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDE 调色灯

static void _lede_timer_utc_2_local(ucp_lede_led_timer_t *timer)
{
 #ifdef USE_TIME_MINS	
	int time_min = 0;

	if(cl_priv == NULL){
		return;
	}
	time_min = timer->hour*60 + timer->min + cl_priv->time_diff + 24*60;
	if (time_min < 24*60) {
		timer->week_loop = timer_week_right_shift(timer->week_loop);
	} else if (time_min >= 48*60) {
		timer->week_loop = timer_week_left_shift(timer->week_loop);
	}
	timer->hour = (time_min/60)%24;
	timer->min = time_min%60;
#else	
	int hour;
	int zone ;

	if(cl_priv == NULL){
		return;
	}
	zone = cl_priv->timezone;
	hour = timer->hour + 24 + zone;
	if (hour < 24) {
		timer->week_loop = timer_week_right_shift(timer->week_loop);
	} else if (hour >= 48) {
		timer->week_loop = timer_week_left_shift(timer->week_loop);
	}
	timer->hour = hour%24;
#endif	
}

static void _lede_timer_local_2_utc(ucp_lede_led_timer_t *timer)
{	
 #ifdef USE_TIME_MINS	
	int time_min = 0;

	if(cl_priv == NULL){
		return;
	}
	time_min = timer->hour*60 + timer->min - cl_priv->time_diff + 24*60;
	if (time_min < 24*60) {
		timer->week_loop = timer_week_right_shift(timer->week_loop);
	} else if (time_min >= 48*60) {
		timer->week_loop = timer_week_left_shift(timer->week_loop);
	}
	timer->hour = (time_min/60)%24;
	timer->min = time_min%60;
#else
	int hour;
	int zone ;

	if(cl_priv == NULL){
		return;
	}
	zone = cl_priv->timezone;
	hour = timer->hour + 24 - zone;
	if (hour < 24) {
		timer->week_loop= timer_week_right_shift(timer->week_loop);
	} else if (hour >= 48) {
		timer->week_loop = timer_week_left_shift(timer->week_loop);
	}
	timer->hour = hour%24;
#endif	
}

static bool _lede_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	ucp_lede_led_state_t* stat;
	ucp_lede_led_timer_t* timer;
	cl_lede_led_state_t* src_stat;
	cl_lede_led_timer_t* src_timer;
	char buf[256] = {0};

	ucp_obj_t* uo = (ucp_obj_t*)buf;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	switch(info->action){
	case ACT_LEDE_CTRL_STAT:
		src_stat = (cl_lede_led_state_t*)(&info->u.u8_data[0]);
		stat = (ucp_lede_led_state_t*)(uo+1);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LEDE_LED, UCAT_LEDE_LED_STATE,sizeof(ucp_lede_led_state_t));
		stat->R = src_stat->R;
		stat->G =  src_stat->G;
		stat->B = src_stat->B;
		stat->L = src_stat->L;
		stat->action = src_stat->action;
		stat->mod_id = src_stat->mod_id;
		stat->power = src_stat->power;
		stat->cold = src_stat->cold;
		sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(ucp_lede_led_state_t));
		break;
	case ACT_LEDE_ON_STAT:
		{
			cl_lede_led_on_stat_t *src_stat = cci_pointer_data(info);
			ucp_lede_led_on_stat_t *on_stat = (ucp_lede_led_on_stat_t*)(uo+1);;

			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LEDE_LED, UCAT_LEDE_LED_UPOWERON_STATUE, sizeof(ucp_lede_led_on_stat_t));

			on_stat->enable = src_stat->enable;
			on_stat->type = src_stat->type;

			on_stat->R = src_stat->stat.R;
			on_stat->G = src_stat->stat.G;
			on_stat->B = src_stat->stat.B;
			on_stat->L = src_stat->stat.L;
			on_stat->mod_id = src_stat->stat.mod_id;
			on_stat->power = src_stat->stat.power;
			on_stat->cold = src_stat->stat.cold;
			on_stat->action = src_stat->stat.action;
			sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(ucp_lede_led_on_stat_t));
		}
		break;
	case ACT_LEDE_CTRL_TIEMR:
		src_timer = (cl_lede_led_timer_t*)(&info->u.u8_data[0]);
		timer = (ucp_lede_led_timer_t*)(uo+1);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LEDE_LED, UCAT_LEDE_LED_TIMER,sizeof(ucp_lede_led_timer_t));
		timer->id = src_timer->id;
		timer->flags = src_timer->flags;
		timer->hour = src_timer->hour;
		timer->week_loop = src_timer->week_loop;
		timer->min = src_timer->min;
		timer->config.R = src_timer->config.R;
		timer->config.G = src_timer->config.G;
		timer->config.B = src_timer->config.B;
		timer->config.L = src_timer->config.L;
		timer->config.action = src_timer->config.action;
		timer->config.mod_id = src_timer->config.mod_id;
		timer->config.power = src_timer->config.power;
		timer->config.cold = src_timer->config.cold;
		_lede_timer_local_2_utc(timer);
		sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(ucp_lede_led_timer_t));
		break;
	case ACT_LEDE_DELETE_TIEMR:
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_LEDE_LED, UCAT_LEDE_LED_TIMER,sizeof(u_int8_t));
		*((char*)(uo+1)) = cci_u8_data(info);
		sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+sizeof(u_int8_t));
		break;
	default:
		res = false;
		break;
	}
	return res;
}

static bool _lede_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_lede_lamp_info* info = air_ctrl->com_udp_dev_info.device_info; 
	ucp_lede_led_state_t* stat;
	ucp_lede_led_on_stat_t *on_stat;
	ucp_lede_led_timer_t* timer;
	cl_lede_led_timer_t* d_timer;	
	u_int32_t t_count,i;

	if( !info || !obj){
		return false;
	}

	switch(obj->attr){
		case UCAT_LEDE_LED_STATE:
			if(is_valid_obj_data(obj, sizeof(ucp_lede_led_state_t))){
				stat = (ucp_lede_led_state_t*)(obj+1);
				info->cur_lamp_stat.R= stat->R;
				info->cur_lamp_stat.G= stat->G;
				info->cur_lamp_stat.B= stat->B;
				info->cur_lamp_stat.L = stat->L;
				info->cur_lamp_stat.mod_id = stat->mod_id;
				info->cur_lamp_stat.power = stat->power;
				info->cur_lamp_stat.cold = stat->cold;
				info->cur_lamp_stat.action = stat->action;
				air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;
			}else{
				return false;
			}
			break;
		case UCAT_LEDE_LED_UPOWERON_STATUE:
			{
				log_debug("get UCAT_LEDE_LED_UPOWERON_STATUE, len %u\n", obj->param_len);
				
				if (!is_valid_obj_data(obj, sizeof(ucp_lede_led_on_stat_t))) {
					return false;
				}

				on_stat = OBJ_VALUE(obj, ucp_lede_led_on_stat_t *);

				info->on_stat.valid = true;
				info->on_stat.enable = on_stat->enable;
				info->on_stat.type = on_stat->type;

				info->on_stat.stat.R= on_stat->R;
				info->on_stat.stat.G= on_stat->G;
				info->on_stat.stat.B= on_stat->B;
				info->on_stat.stat.L = on_stat->L;
				info->on_stat.stat.mod_id = on_stat->mod_id;
				info->on_stat.stat.power = on_stat->power;
				info->on_stat.stat.cold = on_stat->cold;
				info->on_stat.stat.action = on_stat->action;

				log_debug("	enable %u type %u\n", on_stat->enable, on_stat->type);
			}
			break;
		case UCAT_LEDE_LED_TIMER:
			if((obj->param_len % sizeof(ucp_lede_led_timer_t)) == 0){
				SAFE_FREE(info->timer_info);
				info->timer_count = 0;
				
				t_count = obj->param_len/sizeof(ucp_lede_led_timer_t);
				if(t_count > 0){
					info->timer_info = cl_calloc(sizeof(cl_lede_led_timer_t)*t_count,1);
					if(!info->timer_info){
						return false;
					}
					d_timer = info->timer_info;
					timer = (ucp_lede_led_timer_t*)(obj+1);
					for(i = 0; i < t_count; i++,d_timer++,timer++){
						_lede_timer_utc_2_local(timer);
						d_timer->id = timer->id;
						d_timer->flags = timer->flags;
						d_timer->hour = timer->hour;
						d_timer->week_loop = timer->week_loop;
						d_timer->min = timer->min;
						d_timer->config.R = timer->config.R;
						d_timer->config.G = timer->config.G;
						d_timer->config.B = timer->config.B;
						d_timer->config.action = timer->config.action;
						d_timer->config.L = timer->config.L;
						d_timer->config.mod_id = timer->config.mod_id;
						d_timer->config.power = timer->config.power;
						d_timer->config.cold = timer->config.cold;
					}
					info->timer_count = t_count;
				}
			}else{
				return false;
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}

static bool _lede_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_lede_lamp_info* li = NULL;
	cl_lede_lamp_info* src;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	src = ac->com_udp_dev_info.device_info;
	li = cl_calloc(sizeof(*li) , 0x1);
	if(!li){
		return false;
	}
	
	memcpy(li,src,sizeof(*src));
	li->timer_count = 0;
	li->timer_info = NULL;
	//拷贝定时器;
	if(src->timer_count > 0 ){
		li->timer_info = cl_calloc(src->timer_count* sizeof(cl_lede_led_timer_t),1);
		if(li->timer_info != NULL){
			memcpy(li->timer_info,src->timer_info,src->timer_count* sizeof(cl_lede_led_timer_t));
			li->timer_count = src->timer_count;
		}
	}
	
	di->device_info = li;

	return true;
}

static void _free_lede_info(cl_lede_lamp_info* info)
{
	if(!info){
		return;
	}

	SAFE_FREE(info->timer_info);
	info->timer_count = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//车载悟空
static void _free_car_info(cl_car_info_t* info)
{
	if (info->alarm_num > 0 && info->car_alarm) {
		info->alarm_num = 0;
		SAFE_FREE(info->car_alarm);
	}

	if (info->debug_info) {
		SAFE_FREE(info->debug_info);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 杰能宝
static bool _jnb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	char buf[256]={0};
	ucp_jnb_thermotat_info* dest_ucp;
	cl_jnb_thermotat_info* local_d_info;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int32_t data;


	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac || !ac->com_udp_dev_info.device_info) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	local_d_info = ac->com_udp_dev_info.device_info;
	
	dest_ucp = (ucp_jnb_thermotat_info*)(uo+1);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_JNB_THERMOSTAT, UCAT_JNB_STAT,sizeof(ucp_jnb_thermotat_info));
	
	dest_ucp->work_status = local_d_info->work_status;
	dest_ucp->comfort_hold_time = local_d_info->comfort_hold_time;
	dest_ucp->comfort_temperaute = local_d_info->comfort_temperaute;
	dest_ucp->economic_hold_time = local_d_info->economic_hold_time;
	dest_ucp->economic_temperature = local_d_info->economic_temperature;
	dest_ucp->env_temperature = local_d_info->env_temperature;
	dest_ucp->temp_temperature = local_d_info->temp_temperature;
	dest_ucp->vacation_days = local_d_info->vacation_days;
	dest_ucp->vacation_temperature = local_d_info->vacation_temperature;
	memcpy(&dest_ucp->scheduler[0],&local_d_info->scheduler[0],sizeof(dest_ucp->scheduler));	

	switch(info->action){
	case ACT_JNB_CTRL_ONOFF:
		dest_ucp->operaton_type = U_JNB_OP_MODE_CHANGE;
		if(!!cci_u8_data(info)){
			dest_ucp->work_status = JT_MODE_AUTO;
		}else{
			dest_ucp->work_status = JT_MODE_OFF;
		}
		break;
	case ACT_JNB_CTRL_MODE:
		dest_ucp->operaton_type = U_JNB_OP_MODE_CHANGE;
		dest_ucp->work_status = cci_u8_data(info);
		break;
	case ACT_JNB_CTRL_TEMP:
		dest_ucp->operaton_type = U_JNB_OP_TEMP_CHANGE;
		dest_ucp->work_status = JT_MODE_AUTO;
		dest_ucp->temp_temperature = cci_u8_data(info);
		break;
	case ACT_JNB_CTRL_SCHED:
		dest_ucp->operaton_type = U_JNB_OP_MODE_CONFIG;
		dest_ucp->work_status = JT_MODE_AUTO;
		memcpy(&dest_ucp->scheduler[0],&info->u.u32_data[0],sizeof(dest_ucp->scheduler));	
		break;
	case ACT_JNB_CTRL_TEMP_PARAM:
		data = cci_u32_data(info);
		dest_ucp->operaton_type = U_JNB_OP_MODE_CONFIG;
		if(((data >> 24) & 0xFF) == JT_MODE_HOLD_COMFORTABLE ){
			dest_ucp->work_status = JT_MODE_HOLD_COMFORTABLE;
			dest_ucp->comfort_temperaute = (data >> 16)&0xff;
		}else if(((data >> 24) & 0xFF) == JT_MODE_HOLD_ECONOMY){
			dest_ucp->work_status = JT_MODE_HOLD_ECONOMY;
			dest_ucp->economic_temperature = (data >> 16)&0xff;
		}
		break;
	case ACT_JNB_CTRL_HOLIDAY:
		data = cci_u32_data(info);
		dest_ucp->operaton_type = U_JNB_OP_MODE_CONFIG;
		dest_ucp->work_status = JT_MODE_HOLIDAY;
		dest_ucp->vacation_temperature = (data >> 24) & 0xFF;
		dest_ucp->vacation_days = (data >> 16) & 0xFF;
		break;
	case ACT_JNB_CTRL_HOLD_TIME:
		data = cci_u32_data(info);
		dest_ucp->operaton_type = U_JNB_OP_MODE_CHANGE;
		dest_ucp->work_status = (data >> 24) & 0xFF;

		if(dest_ucp->work_status == JT_MODE_HOLD_COMFORTABLE){
			dest_ucp->comfort_hold_time = (data >> 16) & 0xFF;
		}else if(dest_ucp->work_status == JT_MODE_HOLD_ECONOMY){
			dest_ucp->economic_hold_time = (data >> 16) & 0xFF;
		}else{
			*ret = RS_INVALID_PARAM;
			return false;
		}
		break;
	default:
		*ret = RS_INVALID_PARAM;
		return false;
		break;
	}
	
	ntoh_u32_array(&dest_ucp->scheduler[0], DAYS_PER_WEEK);
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(ucp_jnb_thermotat_info));
	
	return res;
}

static bool _jnb_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_jnb_thermotat_info* info = air_ctrl->com_udp_dev_info.device_info; 
	ucp_jnb_thermotat_info* pkt;
	if( !info || !obj ){
		return false;
	}

	switch(obj->attr){
		case UCAT_JNB_STAT:
			if(is_valid_obj_data(obj, sizeof(ucp_jnb_thermotat_info))){
				pkt = (ucp_jnb_thermotat_info*)(obj+1);
				info->work_status = pkt->work_status;
				info->comfort_hold_time = pkt->comfort_hold_time;
				info->comfort_temperaute = pkt->comfort_temperaute;
				info->economic_hold_time = pkt->economic_hold_time;
				info->economic_temperature = pkt->economic_temperature;
				info->env_temperature = pkt->env_temperature;
				info->temp_temperature = pkt->temp_temperature;
				info->vacation_days = pkt->vacation_days;
				info->vacation_temperature = pkt->vacation_temperature;
				info->vacation_remain_days = pkt->vacation_remain_days;
				memcpy(&info->scheduler[0],&pkt->scheduler[0],sizeof(info->scheduler));
				ntoh_u32_array(&info->scheduler[0], DAYS_PER_WEEK);
				air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;
			}else{
				return false;
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//特林温控器

static void _tl_fill_conf_pkt(cl_tl_info_t* s_ti,ucp_obj_t* uo,ucp_tl_conf_t * tc)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TL_TEMP, UCAT_IA_TELIN_BASE,sizeof(*tc));
	tc->mode = s_ti->ctrl_stat.mode;
	tc->onoff = s_ti->ctrl_stat.onoff;
	tc->room_temp = s_ti->ctrl_stat.room_temp;
	tc->speed = s_ti->ctrl_stat.speed;
	tc->state = htons(s_ti->ctrl_stat.state);
    tc->temp = s_ti->ctrl_stat.temp;
}

static void _tl_fill_adv_pkt(cl_tl_info_t* s_ti,ucp_obj_t* uo,ucp_tl_adv_t * ta)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TL_TEMP, UCAT_IA_TELIN_ADV,sizeof(*ta));
	
	ta->charge_factor_low = htons(s_ti->adv_info.charge_factor_low);
	ta->charge_factor_mid = htons(s_ti->adv_info.charge_factor_mid);
	ta->eco_mode = s_ti->adv_info.eco_mode;
	ta->self_learn = s_ti->adv_info.self_learn;
	ta->fan_under_control = s_ti->adv_info.fan_under_control;
	ta->lock_flags = htons(s_ti->adv_info.lock_flags);
	ta->temp_bandwidth = htons(s_ti->adv_info.temp_bandwidth);
	ta->time_on = (u_int8_t)htons(ta->time_on);
}

static void _tl_fill_timer_pkt(cl_tl_info_t* s_ti,ucp_obj_t* uo,ucp_tl_timer_info_t * uti)
{
	int i;

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TL_TEMP, UCAT_IA_TELIN_TIMER,sizeof(*uti));
	for(i = 0 ; i < TL_TIME_CNT_PER_DAY; i++ ){
		uti->time[i].hour = ntohs(s_ti->time_info.time[i].hour);
		uti->time[i].min = ntohs(s_ti->time_info.time[i].min);
		uti->time[i].temp = ntohs(s_ti->time_info.time[i].temp);
	}
}

static bool _tl_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	bool res = false;
	char buf[256] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_tl_conf_t * tc = (ucp_tl_conf_t*)(uo+1);
	ucp_tl_adv_t* ta = (ucp_tl_adv_t*)(uo+1);
	ucp_tl_timer_info_t*  uti = (ucp_tl_timer_info_t*)(uo+1);
	cl_tl_timer_info_t* timer;
	smart_air_ctrl_t* ac;
	cl_tl_info_t* s_ti;
    ucp_tl_time_sync_t* ts = (ucp_tl_time_sync_t*)(uo+1);
	int len = 0,i;
	
	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac || !ac->com_udp_dev_info.device_info) {
		log_err(false, "_tl_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	s_ti = (cl_tl_info_t*)ac->com_udp_dev_info.device_info;

	switch(info->action){
		case ACT_TL_CTRL_ONOFF:
			len = sizeof(*tc);
			s_ti->ctrl_stat.onoff = !!cci_u8_data(info);
			_tl_fill_conf_pkt(s_ti,uo,tc);
			break;
		case ACT_TL_CTRL_MODE:
			len = sizeof(*tc);
			s_ti->ctrl_stat.mode = cci_u8_data(info);
			_tl_fill_conf_pkt(s_ti,uo,tc);
			break;
		case ACT_TL_CTRL_FAN_SPEED:
			len = sizeof(*tc);
			s_ti->ctrl_stat.speed = cci_u8_data(info);
			_tl_fill_conf_pkt(s_ti,uo,tc);
			break;
		case ACT_TL_CTRL_TEMP:
			s_ti->ctrl_stat.temp = cci_u8_data(info);
			len = sizeof(*tc);
			_tl_fill_conf_pkt(s_ti,uo,tc);
			break;
		case ACT_TL_CTRL_ECO:
			s_ti->adv_info.eco_mode = !!cci_u8_data(info);
			len = sizeof(*ta);
			_tl_fill_adv_pkt(s_ti,uo,ta);
			break;
		case ACT_TL_CTRL_LEARN:
			s_ti->adv_info.self_learn = !!cci_u8_data(info);
			len = sizeof(*ta);
			_tl_fill_adv_pkt(s_ti,uo,ta);
			break;
		case ACT_TL_CTRL_TIMER:
			timer = (cl_tl_timer_info_t*)&info->u.u8_data[0];
			for(i = 0 ; i < TL_TIME_CNT_PER_DAY; i++ ){
				s_ti->time_info.time[i].hour = timer->time[i].hour;
				s_ti->time_info.time[i].min = timer->time[i].min;
				s_ti->time_info.time[i].temp = timer->time[i].temp;
			}
			len = sizeof(*uti);
			_tl_fill_timer_pkt(s_ti,uo,uti);
			break;
        case ACT_TL_CTRL_TIME_AUTO_SYNC:
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TL_TEMP, UCAT_IA_TELIN_TIME_SYNC,sizeof(*ts));
            len = sizeof(*ts);
            ts->utc_time = (u_int32_t)time(NULL);
            ts->utc_time = htonl(ts->utc_time);
            ts->is_auto = !!cci_u8_data(info);
            ts->just_sync = false;
            break;
        case ACT_TL_CTRL_TIME_SYNC:
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TL_TEMP, UCAT_IA_TELIN_TIME_SYNC,sizeof(*ts));
            ts->utc_time = (u_int32_t)time(NULL);
            ts->utc_time = htonl(ts->utc_time);
            ts->is_auto = s_ti->dev_time_sync_info.is_auto_sync;
            ts->just_sync = true;
            len = sizeof(*ts);
            break;
        case ACT_TL_CTRL_REFRESH_TIME:
            udp_query_single_object(ac->sac->user->uc_session, UCOT_IA, UCSOT_IA_TL_TEMP, UCAT_IA_TELIN_TIME_SYNC);
            break;
		default:
			*ret = RS_INVALID_PARAM;
			break;
	}

	if(len > 0){
		sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	}
	
	return res;
}

static bool _tl_update_data(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    cl_tl_info_t* ti;
	ucp_tl_conf_t * tc;
	ucp_tl_adv_t* ta;
	ucp_tl_total_time_t* tt;
	ucp_tl_timer_info_t*  uti;
    ucp_tl_time_sync_t* ts;
	int i;

	if(!ac || !obj || !ac->com_udp_dev_info.device_info){
		return false;
	}
    
    ti = ac->com_udp_dev_info.device_info;
	
	switch(obj->attr){
		case UCAT_IA_TELIN_BASE:
			if(!is_valid_obj_data(obj,sizeof(*tc))){
				return false;
			}

			tc = OBJ_VALUE(obj, ucp_tl_conf_t*);
			ti->ctrl_stat.onoff = tc->onoff;
			ti->ctrl_stat.mode = tc->mode;
			ti->ctrl_stat.temp = tc->temp;
			ti->ctrl_stat.room_temp = ntohs(tc->room_temp);
			ti->ctrl_stat.state = ntohs(tc->state);
			ti->ctrl_stat.valve_stat = !!(ti->ctrl_stat.state & BIT(0));
			ti->ctrl_stat.cool_valve_stat = !!(ti->ctrl_stat.state & BIT(1));
			ti->ctrl_stat.hot_valve_stat = !!(ti->ctrl_stat.state & BIT(2));
			ti->ctrl_stat.charge_enable = !!(ti->ctrl_stat.state & BIT(11));
			if(ti->ctrl_stat.state & BIT(8)){
				ti->ctrl_stat.speed = FAN_TL_LOW;
			}else if(ti->ctrl_stat.state & BIT(9)){
				ti->ctrl_stat.speed = FAN_TL_MID;
			}else if(ti->ctrl_stat.state & BIT(10)){
				ti->ctrl_stat.speed = FAN_TL_HIGH;
			}else{
				ti->ctrl_stat.speed = FAN_TL_AUTO;
			}
	
			break;
		case UCAT_IA_TELIN_ADV:
			if(!is_valid_obj_data(obj,sizeof(*ta))){
				return false;
			}
			
			ta = OBJ_VALUE(obj, ucp_tl_adv_t*);
			ti->adv_info.charge_factor_low = ntohs(ta->charge_factor_low);
			ti->adv_info.charge_factor_mid = ntohs(ta->charge_factor_mid);
			ti->adv_info.eco_mode = ta->eco_mode;
			ti->adv_info.fan_under_control = ta->fan_under_control;
			ti->adv_info.lock_flags = ntohs(ta->lock_flags);
			ti->adv_info.self_learn = ta->self_learn;
			ti->adv_info.temp_bandwidth = ntohs(ta->temp_bandwidth);
			ti->adv_info.time_on = ta->time_on;
            ti->adv_info.run_timer = ta->run_timer;
			
			break;
		case UCAT_IA_TELIN_STAT:
			if(!is_valid_obj_data(obj,sizeof(*tt))){
				return false;
			}

			tt = OBJ_VALUE(obj, ucp_tl_total_time_t*);
			ti->total_info.high_gear_time = ntohl(tt->high_gear_time);
			ti->total_info.high_gear_time_cal = ntohl(tt->high_gear_time_cal);
			ti->total_info.low_gear_time = ntohl(tt->low_gear_time);
			ti->total_info.mid_gear_time = ntohl(tt->mid_gear_time);

			break;
		case UCAT_IA_TELIN_TIMER:
			if(!is_valid_obj_data(obj,sizeof(*uti))){
				return false;
			}
			
			uti = OBJ_VALUE(obj, ucp_tl_timer_info_t*);
			for(i = 0 ; i < TL_TIME_CNT_PER_DAY; i++ ){
				ti->time_info.time[i].hour = ntohs(uti->time[i].hour);
				ti->time_info.time[i].min = ntohs(uti->time[i].min);
				ti->time_info.time[i].temp = ntohs(uti->time[i].temp);
			}

			break;
        case UCAT_IA_TELIN_TIME_SYNC:
            if (is_obj_less_than_len(obj, sizeof(ucp_tl_time_sync_t))) {
                return false;
            }
            
            ts = OBJ_VALUE(obj, ucp_tl_time_sync_t*);
            ti->dev_time_sync_info.dev_time = ntohl(ts->utc_time);
            ti->dev_time_sync_info.is_auto_sync = !!ts->is_auto;
            ti->dev_time_sync_info.is_data_valid = true;
            ti->dev_time_sync_info.local_update_time = (u_int32_t)time(NULL);
            
            break;
		default:
			return false;
			break;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////
//月兔
static void _yt_ctrl_timer_sync(user_t *user, cl_yt_info_t* pyti);

static bool _yt_ctrl(user_t *user, u_int8_t action, u_int8_t value, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_yt_info_t* pyti = (cl_yt_info_t*)ac->com_udp_dev_info.device_info;
	int len = 0;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_yt_work_t *pwork = (cl_yt_work_t *)(&uo[1]);

	if (!ac || !pyti) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memcpy((void *)pwork, (void *)&pyti->work, sizeof(*pwork));

	switch(action) {
	case YT_ACTION_ONOFF:
		//if (value != pyti->onoff) {
			len = sizeof(*pwork);
			pwork->onoff = value;
			pyti->onoff = value;
			//关机时应该关闭睡眠
			if (value == 0) {
				pwork->sleep = value;
				pyti->sleep = value;
			}
			//开关机时，清空定时器
			_yt_ctrl_timer_sync(user, pyti);
		//}
		break;
	case YT_ACTION_MODE:
		//if (pyti->mode != value) {
			len = sizeof(*pwork);
			pwork->mode = value;
			pyti->mode = value;
			pwork->sleep = 0;
			pyti->sleep = 0;
			//这里再要处理一下，切换为制热模式，或者自动模式下四通阀开启时，开启电辅热
			if (value == YT_MODE_HOT ||
				((value == YT_MODE_AUTO) && pyti->four_valve)) {
				pwork->ele_assist = 1;
				pyti->ele_assist = 1;
			}
		//}
		break;
	case YT_ACTION_TMP:
		//if (pyti->tmp != value) {
			if (value > YT_TMP_MAX) {
				value = YT_TMP_MAX;
			}

			if (value < YT_TMP_MIN) {
				value = YT_TMP_MIN;
			}
			
			len = sizeof(*pwork);
			pwork->tmp = value;
			pyti->tmp = value;
		//}
		break;
	case YT_ACTION_WINDSPEED:
		//if (pyti->wind_speed != value) {
			len = sizeof(*pwork);
			pwork->wind_speed = value;
			pyti->wind_speed = value;
			pwork->sleep = 0;
			pyti->sleep = 0;
		//}
		break;
	case YT_ACTION_WINDDIR:
		//if (pyti->wind_dir != value) {
			len = sizeof(*pwork);
			pwork->wind_dir = value;
			pyti->wind_dir = value;
		//}
		break;
	case YT_ACTION_ELEASSIST:
		//if (value != pyti->ele_assist) {
			len = sizeof(*pwork);
			pwork->ele_assist = value;
			pyti->ele_assist = value;
		//}
		break;
	case YT_ACTION_SLEEP:
		//if (value != pyti->sleep) {
			len = sizeof(*pwork);
			pwork->sleep = value;
			pyti->sleep = value;
		//}
		break;
	case YT_ACTION_SWING:
		//if (value != pyti->swing) {
			len = sizeof(*pwork);
			pwork->swing = value;
			pyti->swing = value;
		//}
		break;
	default:
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if (len > 0) {
		//控制后立马更新数据
		//event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	    //event_cancel_merge(user->handle);
		pwork->action = action;
		memcpy((void *)&pyti->work, (void *)pwork, sizeof(*pwork));
		pwork->tmp = pwork->tmp - YT_TMP_BASE;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_WORK,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
		user->last_ctrl = true;
	}

	return true;
}

static void yt_timer_order(cl_yt_timer_t *ptimer_net)
{
	ptimer_net->off_remain_min = htons(ptimer_net->off_remain_min);
	ptimer_net->on_remain_min = htons(ptimer_net->on_remain_min);
}

static bool _yt_timer(user_t *user, u_int8_t *ptmp, RS *ret, bool modify)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_yt_info_t* pyti = (cl_yt_info_t*)ac->com_udp_dev_info.device_info;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_yt_timer_t *ptimer_net = (cl_yt_timer_t *)(&uo[1]);
	cl_yt_work_t *pwork = (cl_yt_work_t *)(&uo[1]);
	cl_yt_timer_t *ptimer_src = (cl_yt_timer_t *)ptmp;
	cl_yt_timer_t *ptimer_local = &pyti->timer;
	int len = 0;
	
	if (!ac || !pyti) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if (ptimer_src->off_enable && 
		ptimer_src->on_enable &&
		(ptimer_src->off_remain_min == ptimer_src->on_remain_min)) {
		event_push(user->callback, UE_CTRL_YT_TIME_ERR, user->handle, user->callback_handle);
    	event_cancel_merge(user->handle);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_TIMER,0);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 0x1, buf, sizeof(*uo));
		return false;
	}

	//判断下如果相同就不处理
	if ((ptimer_local->off_enable == ptimer_src->off_enable) &&
		(ptimer_local->off_remain_min == ptimer_src->off_remain_min) &&
		(ptimer_local->on_enable == ptimer_src->on_enable) &&
		(ptimer_local->on_remain_min == ptimer_src->on_remain_min)) {
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_TIMER,0);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 0x1, buf, sizeof(*uo));
		return false;
	}

	memcpy((void *)ptimer_local, (void *)ptimer_src, sizeof(*ptimer_local));
	memcpy((void *)ptimer_net, (void *)ptimer_src, sizeof(*ptimer_local));
	len = sizeof(*ptimer_net);
	yt_timer_order(ptimer_net);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_TIMER,len);
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);

	
	if ((ptimer_src->off_enable == 0) &&
		(ptimer_src->on_enable == 0)) {
		len = sizeof(*pwork);
		memcpy((void *)pwork, (void *)&pyti->work, sizeof(*pwork));
		pwork->action = YT_ACTION_ONOFF;
		pwork->tmp = pwork->tmp - YT_TMP_BASE;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_WORK,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}
	
	user->last_ctrl = true;
#if 0
	//控制后立马更新数据
	if (modify) {
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    	event_cancel_merge(user->handle);
	}
#endif	


	return true;	
}

static void _yt_ctrl_timer_sync(user_t *user, cl_yt_info_t* pyti)
{
	cl_yt_timer_t timer;
	RS ret;

	if (!pyti->timer.off_enable && !pyti->timer.on_enable) {
		return;
	}
	memcpy((void *)&timer, (void *)&pyti->timer, sizeof(timer));

	timer.on_enable = 0;
	timer.on_remain_min = 0;
	timer.off_enable = 0;
	timer.off_remain_min = 0;

	_yt_timer(user, (u_int8_t *)&timer, &ret, false);
}

bool _yt_query_power(user_t *user, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_ELE,0);
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET, false, 0x1, buf, sizeof(*uo));

	*ret = RS_OK;
	
	return true;	
}



static bool _yt_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	u_int8_t action, value;
	u_int16_t data16;
	u_int8_t *ptmp = NULL;
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_yt_info_t* pyti = (cl_yt_info_t*)ac->com_udp_dev_info.device_info;
	
	info = (cln_common_info_t *)&pkt->data[0];

	//判断一下，如果有故障，就报错
	if (pyti->fault_b2 || pyti->fault_b12 ||
		pyti->protect_b13 || pyti->protect_b14 || pyti->protect_b18) {
		event_push(user->callback, UE_CTRL_YT_CTRL_FAULT, user->handle, user->callback_handle);
		event_cancel_merge(user->handle);
		*ret = RS_INVALID_PARAM;
		return RS_OK;
	}

	switch(info->action){
	case ACT_YT_CTRL:
		data16 = cci_u16_data(info);
        action = (data16 >> 8)&0xff;
        value = data16 &0xff;
		return _yt_ctrl(user, action, value , ret);
		break;
	case ACT_YT_TIMER:
		ptmp = cci_pointer_data(info);
		_yt_timer(user, ptmp, ret, true);
		break;
	case ACT_YT_SN:
		ptmp = cci_pointer_data(info);
		_yt_sn(user, ptmp, ret);
		break;
	case ACT_YT_QUERY_POWER:
		_yt_query_power(user, ret);
		break;
	case ACT_YT_QUERY_ELE:
		_yt_query_phase_ele(user, ptmp, ret);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;
	}
	
	return RS_OK;
}

static bool _yt_update_work(cl_yt_info_t* info, ucp_obj_t* obj)
{
	bool ret = true;
	u_int8_t on;
	u_int8_t tmp;
	cl_yt_work_t *pwork_net = (cl_yt_work_t *)(&obj[1]);
	cl_yt_work_t *pwork_local = &info->work;

	info->work_info_valid = true;
	if (memcmp((void *)pwork_local, (void *)pwork_net, sizeof(*pwork_local)) == 0) {
		return true;
	}

	memcpy((void *)pwork_local, (void *)pwork_net, sizeof(*pwork_local));
		
	on = pwork_local->onoff;
	if (info->onoff != on) {
		info->onoff = on;
		ret = true;
	}

	if (info->mode != pwork_local->mode) {
		info->mode = pwork_local->mode;
		ret = true;
	}

	tmp = pwork_local->tmp + YT_TMP_BASE;
	if (info->tmp != tmp) {
		info->tmp = tmp;
		ret = true;
	}

	if (info->wind_speed != pwork_local->wind_speed) {
		info->wind_speed = pwork_local->wind_speed;
		ret = true;
	}	

	if (info->wind_dir != pwork_local->wind_dir) {
		info->wind_dir = pwork_local->wind_dir;
		ret = true;
	}		

	on = pwork_local->ele_assist;
	if (info->ele_assist != on) {
		info->ele_assist = on;
		if (info->ele_assist == 0) {
			info->ele_hot = 0;
		}
		ret = true;
	}

	on = pwork_local->sleep;
	if (info->sleep != on) {
		info->sleep = on;
		ret = true;
	}

#if 1
	on = pwork_local->swing;
	if (info->swing != on) {
		info->swing = on;
		ret = true;
	}	
#endif
	if (info->room_tmp != pwork_local->room_tmp) {
		info->room_tmp = pwork_local->room_tmp;
		ret = true;
	}
	
	if (info->extern_tmp != pwork_local->extern_tmp) {
		info->extern_tmp = pwork_local->extern_tmp;
		ret = true;
	}	

	return ret;
}

static bool _yt_update_timer(cl_yt_info_t* info, ucp_obj_t* obj)
{
	bool ret = true;
	cl_yt_timer_t *pl = &info->timer;
	cl_yt_timer_t *pn = (cl_yt_timer_t *)(&obj[1]);

	yt_timer_order(pn);
	if (memcmp((void *)pl, (void *)pn, sizeof(*pl))) {
		memcpy((void *)pl, (void *)pn, sizeof(*pl));
		ret = true;
	}
	
	return ret;
}

static bool _yt_update_stat(cl_yt_info_t* info, ucp_obj_t* obj)
{
	u_int8_t on;
	u_int16_t tmp16;
	bool ret = true;
	ucp_yt_stat_t *pstat = (ucp_yt_stat_t *)(&obj[1]);

	info->stat_info_valid = true;
	on = pstat->compressor_onoff;
	if (info->compressor_onoff != on) {
		info->compressor_onoff = on;
		ret = true;
	}

	if (info->extern_wind_onoff != pstat->extern_wind_onoff) {
		info->extern_wind_onoff = pstat->extern_wind_onoff;
		ret = true;
	}	

	if (info->in_fans_gears != pstat->in_fans_gears) {
		info->in_fans_gears = pstat->in_fans_gears;
		ret = true;
	}

	on = pstat->four_valve;
	if (info->four_valve != on) {
		info->four_valve = on;
		ret = true;
	}

	on = pstat->ele_hot;
	if (info->ele_hot != on) {
		info->ele_hot = on;
		ret = true;
	}

	tmp16= htons(pstat->compressor_work_hours);
	if (info->compressor_work_hours != tmp16) {
		info->compressor_work_hours = tmp16;
		ret = true;
	}

	tmp16 = htons(pstat->assis_work_hours);
	if (info->assit_work_hours != tmp16) {
		info->assit_work_hours = tmp16;
		ret = true;
	}
	
	on = pstat->down_reboot;
	if (info->down_reboot != on) {
		info->down_reboot = on;
		ret = true;
	}

	if (info->wind_real_spee != pstat->wind_real_spee) {
		info->wind_real_spee = pstat->wind_real_spee;
		ret = true;
	}

	if (info->inner_tmp != pstat->inner_tmp) {
		info->inner_tmp = pstat->inner_tmp;
		ret = true;
	}

	if (info->dc_busway_val != pstat->dc_busway_val) {
		info->dc_busway_val = pstat->dc_busway_val;
		ret = true;
	}

	if (info->extern_ac_val != pstat->extern_ac_val) {
		info->extern_ac_val = pstat->extern_ac_val;
		ret = true;
	}

	if (info->extern_ac_cur != pstat->extern_ac_cur) {
		info->extern_ac_cur = pstat->extern_ac_cur;
		ret = true;
	}

	if (info->compressor_cur != pstat->compressor_cur) {
		info->compressor_cur = pstat->compressor_cur;
		ret = true;
	}

	if (info->compressor_freq != pstat->compressor_freq) {
		info->compressor_freq = pstat->compressor_freq;
		ret = true;
	}

	if (info->outside_tmp != pstat->outside_tmp) {
		info->outside_tmp = pstat->outside_tmp;
		ret = true;
	}

	if (info->exhaust_tmp != pstat->exhaust_tmp) {
		info->exhaust_tmp = pstat->exhaust_tmp;
		ret = true;
	}

	if (info->ipm_tmp != pstat->ipm_tmp) {
		info->ipm_tmp = pstat->ipm_tmp;
		ret = true;
	}

	on = pstat->heat_defrost;
	if (info->heat_defrost != on) {
		info->heat_defrost = on;
		ret = true;
	}
	
	return ret;	
}

static bool _yt_update_ele(cl_yt_info_t* info, ucp_obj_t* obj)
{
	return false;
}

static bool _yt_update_protect(cl_yt_info_t* info, ucp_obj_t* obj)
{
	bool ret = true;
	ucp_yt_protect_t *pn = (ucp_yt_protect_t *)(&obj[1]);

	if (info->fault_b2 != pn->fault_b2) {
		info->fault_b2 = pn->fault_b2;
		ret = true;
	}

	if (info->fault_b12 != pn->fault_b15) {
		info->fault_b12 = pn->fault_b15;
		ret = true;
	}

	if (info->protect_b13 != pn->protect_b16) {
		info->protect_b13 = pn->protect_b16;
		ret = true;
	}

	if (info->protect_b14 != pn->protect_b17) {
		info->protect_b14 = pn->protect_b17;
		ret = true;
	}
	
	if (info->protect_b18 != pn->protect_b18) {
		info->protect_b18 = pn->protect_b18;
		ret = true;
	}

	if (info->ele_hot != pn->ele_hot) {
		info->ele_hot = pn->ele_hot;
		ret = true;
	}

	if (info->four_valve != pn->four_valve) {
		info->four_valve = pn->four_valve;
		ret = true;
	}
	
	return ret;
}

static bool _yt_update_acinfo(smart_air_ctrl_t* ac, cl_yt_info_t* info, ucp_obj_t* obj)
{
	bool ret = true;
	char *ptmp = NULL;
	cl_yt_ac_type_t *pyat_net = (cl_yt_ac_type_t *)(&obj[1]);
	cl_yt_ac_type_t *pyat_local = &info->ac_info;
	cl_yt_ac_type_t def;
	int len;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_yt_ac_type_t *ptmp_net = (cl_yt_ac_type_t *)(&uo[1]);

	info->ac_info_valid = true;
	ac_type_order(pyat_net);
	
	if (memcmp((void *)pyat_local, (void *)pyat_net, sizeof(*pyat_local))) {
		memcpy((void *)pyat_local, (void *)pyat_net, sizeof(*pyat_local));
		ptmp = ac_type_get_name(pyat_local->index);
		strcpy(info->name, ptmp);
		if (pyat_local->index != 0xffff) {
			info->sys_type = ac_sys_type_get(pyat_local->index);
		}
	}

	//做下兼容处理，对方要求复位后默认KFR-35G/DY-CA(A3)这个型号
	if ((pyat_local->index == 0xffff) &&
		(pyat_local->sn == 0)) {
		memset((void *)&def, 0, sizeof(def));
		if (!yt_get_ac_info("23741111111111111", &def)) {
			return true;
		}
		ptmp = ac_type_get_name(def.index);
		strcpy(info->name, ptmp);
		info->sys_type = ac_sys_type_get(def.index);
		def.index = 0xffff;
		memcpy((void *)pyat_local, (void *)&def, sizeof(def));
		ac_type_order(&def);
		memcpy((void *)ptmp_net, (void *)&def, sizeof(def));
		len = sizeof(def);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_SN,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, true, 0x1, buf, sizeof(*uo)+len);
		//控制后立马更新数据
		event_push(ac->sac->user->callback, UE_INFO_MODIFY, ac->sac->user->handle, ac->sac->user->callback_handle);
	   	event_cancel_merge(ac->sac->user->handle);
	}
	
	return true;
}


static bool _yt_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_yt_info_t* info = ac->com_udp_dev_info.device_info; 
	bool ret = false;

	if( !info || !obj ){
		return false;
	}

	ac->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
		case UCAT_IA_YUETU_WORK:
			if(is_valid_obj_data(obj, sizeof(cl_yt_work_t))){
				ret = _yt_update_work(info, obj);
			}else{
				return false;
			}
			break;
		case UCAT_IA_YUETU_TIMER:
			if(is_valid_obj_data(obj, sizeof(cl_yt_timer_t))){
				ret = _yt_update_timer(info, obj);
			}else{
				return false;
			}
			break;
		case UCAT_IA_YUETU_STAT:
			if(is_valid_obj_data(obj, sizeof(ucp_yt_stat_t))){
				ret = _yt_update_stat(info, obj);
			}else{
				return false;
			}
			break;
		case UCAT_IA_YUETU_ERROR_PROTECT:
			if(is_valid_obj_data(obj, sizeof(ucp_yt_protect_t))){
				ret = _yt_update_protect(info, obj);
			}else{
				return false;
			}
			break;
		case UCAT_IA_YUETU_ELE:
			ret = _yt_update_ele(info, obj);
			break;
		case UCAT_IA_YUETU_SN:
			if(is_valid_obj_data(obj, sizeof(cl_yt_ac_type_t))){
				ret = _yt_update_acinfo(ac, info, obj);
			}else{
				return false;
			}
			break;
		default:
			ret = false;
			break;
	}

	info->sn_err = 0;
	//这里来判断下条形码与实际机型可能不匹配的问题
	if (info->ac_info_valid && 
		info->stat_info_valid && 
		info->work_info_valid) {
		//单冷情况下不能有制热模式，或者自动模式下四通阀开启
		if (info->ac_info.cool_type == 0) {
			if ((info->mode == YT_MODE_HOT) || 
				((info->mode == YT_MODE_AUTO) && 
				info->four_valve)) {
				log_info("send SAE_YT_SN_ERR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				event_push(ac->sac->user->callback, SAE_YT_SN_ERR, ac->sac->user->handle, ac->sac->user->callback_handle);
	   			event_cancel_merge(ac->sac->user->handle);
				info->sn_err = 1;
			}
		}
	}

	
	return ret;	
}

////////////////////////////////////////////////////////////////////////////////////
//澳德绅
static bool _ads_ctrl(user_t *user, u_int8_t action, u_int8_t value, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_ads_info_t* pai = (cl_ads_info_t*)ac->com_udp_dev_info.device_info;
	int len = 0;
	char buf[128];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pdata = (u_int8_t *)(&uo[1]);

	if (!ac || !pai) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memset(buf, 0, sizeof(buf));
	switch(action) {
	case ADS_ACTION_ONOFF:
		if (value != pai->on) {
			pai->on = value;
			if (value == 0) {
				pai->wind = 0;
			}
			len = 8;
			pdata[0] = action;
			pdata[1] = 4;
			pdata[4] = value;
		}
		break;
	case ADS_ACTION_BW_ONOFF:
		if (value != pai->back_water_on) {
			pai->back_water_on = value;
			len = 8;
			pdata[0] = action;
			pdata[1] = 4;
			pdata[4] = value;
		}
		break;
	case ADS_ACTION_TMP:
		if (value != pai->water_box_ctrl_tmp) {
			pai->water_box_ctrl_tmp = value;
			len = 8;
			pdata[0] = action;
			pdata[1] = 4;
			pdata[4] = value;
		}
		break;
	default:
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if (len > 0) {
		//控制后立马更新数据
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	    event_cancel_merge(user->handle);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ADS, UCAT_IA_ADS_CTRL,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
	}

	return true;
}

static void ads_conf_order(cl_ads_conf_t *ptmp_net)
{
	ptmp_net->sys_clock_time = htons(ptmp_net->sys_clock_time);
	ptmp_net->first_start_time = htons(ptmp_net->first_start_time);
	ptmp_net->first_end_time = htons(ptmp_net->first_end_time);
	ptmp_net->second_start_time = htons(ptmp_net->second_start_time);
	ptmp_net->second_end_time = htons(ptmp_net->second_end_time);
	ptmp_net->defrost_time = htons(ptmp_net->defrost_time);
}

static bool _ads_conf(user_t *user, u_int8_t *ptmp, RS *ret)
{
	smart_air_ctrl_t *ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_ads_info_t *pai = (cl_ads_info_t*)ac->com_udp_dev_info.device_info;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_ads_conf_t *ptmp_net = (cl_ads_conf_t *)(&uo[1]);
	cl_ads_conf_t *ptmp_src = (cl_ads_conf_t *)ptmp;
	cl_ads_conf_t *ptmp_local = &pai->conf;
	int len = 0;
	
	if (!ac || !pai) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if (memcmp((void *)ptmp_local, (void *)ptmp_src, sizeof(*ptmp_local)) == 0) {
		return true;
	}
	memcpy((void *)ptmp_local, (void *)ptmp_src, sizeof(*ptmp_local));
	memcpy((void *)ptmp_net, (void *)ptmp_src, sizeof(*ptmp_net));
	len = sizeof(*ptmp_net);
	ads_conf_order(ptmp_net);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_ADS, UCAT_IA_ADS_CONF,len);
	sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, true, 0x1, buf, sizeof(*uo)+len);
	//控制后立马更新数据
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    event_cancel_merge(user->handle);
	
	return true;
}

static bool _ads_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	u_int8_t action, value;
	u_int16_t data16;
	u_int8_t *ptmp = NULL;
	
	info = (cln_common_info_t *)&pkt->data[0];

	switch(info->action){
	case ACT_ADS_CTRL:
		data16 = cci_u16_data(info);
        action = (data16 >> 8)&0xff;
        value = data16 &0xff;
		return _ads_ctrl(user, action, value , ret);
		break;
	case ACT_ADS_CONF:
		ptmp = cci_pointer_data(info);
		_ads_conf(user, ptmp, ret);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;
	}
	
	return true;
}

static bool _ads_update_stat(cl_ads_info_t* info, ucp_obj_t* obj)
{
	bool ret = true;
	u_int16_t tmp16;
	cl_ads_stat_t *ptmp_net = (cl_ads_stat_t *)(&obj[1]);
	cl_ads_stat_t *ptmp_local = &info->stat;

	if (memcmp((void *)ptmp_local, (void *)ptmp_net, sizeof(*ptmp_local)) == 0) {
		return true;
	}

	memcpy((void *)ptmp_local, (void *)ptmp_net, sizeof(*ptmp_local));

	if (info->on != ptmp_local->work_mode) {
		info->on = ptmp_local->work_mode;
		ret = true;
	}

	//快热模式
	if (info->back_water_on != ptmp_local->back_water_status) {
		info->back_water_on = ptmp_local->back_water_status;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->water_box_show_tmp;
	if (info->water_box_show_tmp != tmp16) {
		info->water_box_show_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->water_box_ctrl_tmp;
	if (info->water_box_ctrl_tmp != tmp16) {
		info->water_box_ctrl_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->water_box_set_tmp;
	if (info->water_show_tmp != tmp16) {
		info->water_show_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->out_water_tmp;
	if (info->out_water_tmp != tmp16) {
		info->out_water_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->env_tmp;
	if (info->env_tmp != tmp16) {
		info->env_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->back_diff_tmp;
	if (info->back_diff_tmp != tmp16) {
		info->back_diff_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->compressor_cur;
	if (info->compressor_cur != tmp16) {
		info->compressor_cur = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->scroll_tmp;
	if (info->scroll_tmp != tmp16) {
		info->scroll_tmp = tmp16;
		ret = true;
	}

	tmp16 = (int8_t)ptmp_local->exhaust_tmp;
	if (info->exhaust_tmp != tmp16) {
		info->exhaust_tmp = tmp16;
		ret = true;
	}

	if (info->defrost != ptmp_local->defrost_status) {
		info->defrost = ptmp_local->defrost_status;
		ret = true;
	}
#if 0
	if (info->wind != ptmp_local->wind) {
		info->wind = ptmp_local->defrost_status;
		ret = true;
	}
#endif	

	if (info->fault_code != ptmp_local->fault_code) {
		info->fault_code = ptmp_local->fault_code;
		ret = true;
	}

	if (info->fault_code2 != ptmp_local->fault_code2) {
		info->fault_code2 = ptmp_local->fault_code2;
		ret = true;
	}

	tmp16 = (ptmp_local->pump_clock_h)*60 + ptmp_local->pump_clock_m;
	if (info->pump_clock != tmp16) {
		info->pump_clock = tmp16;
		info->conf.sys_clock_time = tmp16;
		ret = true;
	}

	//conf
	tmp16 = ptmp_local->first_start_time_h*60 + ptmp_local->first_start_time_m;
	if (info->conf.first_start_time != tmp16) {
		info->conf.first_start_time = tmp16;
		ret = true;
	}

	tmp16 = ptmp_local->first_end_time_h*60 + ptmp_local->first_end_time_m;
	if (info->conf.first_end_time != tmp16) {
		info->conf.first_end_time = tmp16;
		ret = true;
	}

	tmp16 = ptmp_local->second_start_time_h*60 + ptmp_local->second_start_time_m;
	if (info->conf.second_start_time != tmp16) {
		info->conf.second_start_time = tmp16;
		ret = true;
	}

	tmp16 = ptmp_local->second_end_time_h*60 + ptmp_local->second_end_time_m;
	if (info->conf.second_end_time != tmp16) {
		info->conf.second_end_time = tmp16;
		ret = true;
	}

	tmp16 = ptmp_local->defrost_time;
	if (info->conf.defrost_time != tmp16) {
		info->conf.defrost_time = tmp16;
		ret = true;
	}
	tmp16 = (int8_t)ptmp_local->defrost_tmp; 
	if (info->conf.defrost_tmp != tmp16) {
		info->conf.defrost_tmp = tmp16;
		ret = true;
	}

	tmp16 = ptmp_local->compressor_cur; 
	if (info->compressor_cur != tmp16) {
		info->compressor_cur = tmp16;
		ret = true;
	}
	
	if (info->compressor_status != ptmp_local->compressor_status) {
		info->compressor_status = ptmp_local->compressor_status;
		ret = true;
	}

	//风机逻辑，开机，压缩机开启，就开启风机转动
	if (info->on && info->compressor_status) {
		info->wind = 1;
	} else {
		info->wind = 0;
	}
	
	return ret;
}

static bool _ads_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	cl_ads_info_t* info = ac->com_udp_dev_info.device_info; 
	bool ret = false;

	if( !info || !obj ){
		return false;
	}

	ac->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
		case UCAT_IA_ADS_CONF:
			if(is_valid_obj_data(obj, sizeof(cl_ads_stat_t))){
				ret = _ads_update_stat(info, obj);
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

///////////////////////////////////////////////////////////////////////////////////
//晶石微波炉
static bool _js_wave_update_data(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    cl_js_wave_info_t* info = ac->com_udp_dev_info.device_info;
    ucp_js_stat_t* js;
    if( !info || !obj ){
        return false;
    }
    
    switch (obj->attr) {
        case UCAT_IA_JS_STATE:
            if (!is_obj_less_than_len(obj, sizeof(*js))) {
                js = OBJ_VALUE(obj, ucp_js_stat_t*);
                
                info->stat.local_refresh_time = (u_int32_t)time(NULL);
                js->stat_flag = ntohs(js->stat_flag);
                info->stat.is_data_valid = true;
                info->stat.work_sub_mode = js->work_sub_mode;
                info->stat.work_mode = js->work_mode;
                info->stat.wave_fire = js->wave_fire;
                info->stat.setting_min = js->t_min;
                info->stat.setting_sec = js->t_sec;
                info->stat.hot_fan_temp = js->hot_fan_temp;
                info->stat.food_weight = ntohs(js->food_weight);
                info->stat.cur_temp = js->cur_temp;
                info->stat.remain_sec = js->cur_sec;
                info->stat.remain_min = js->cur_min;
                info->stat.barbecue_fire = js->barbecue_fire;
                info->stat.is_waiting = !! (js->stat_flag & BIT(0));
                info->stat.is_working = !! (js->stat_flag & BIT(6));
                info->stat.is_pausing = !! (js->stat_flag & BIT(7));
                info->stat.child_lock_onoff = !! (js->stat_flag & BIT(8));
                info->stat.is_door_open = !! (js->stat_flag & BIT(9));
                info->stat.is_fault_stat = !! (js->stat_flag & BIT(10));
                info->stat.is_chain_stat = !! (js->stat_flag & BIT(11));
            }
            break;
            
        default:
            break;
    }
    
    return true;
}

static bool _js_wave_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    cl_js_wave_work_setting_t* ws;
    int len = 0;
    u_int8_t buf[128] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_js_child_lock_ctrl_t* lc = (ucp_js_child_lock_ctrl_t*)(uo+1);
    ucp_js_fast_ctrl_t* fc = (ucp_js_fast_ctrl_t*)(uo+1);
    ucp_js_ctrl_t* jc = (ucp_js_ctrl_t*)(uo+1);
    
    if(!user || !user->smart_appliance_ctrl){
        *ret = RS_OFFLINE;
        return false;
    }
    
    ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    info = (cln_common_info_t *)&pkt->data[0];
    switch (info->action) {
        case ACT_JS_WAVE_CTRL:
            len = sizeof(*jc);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_JS_MICWAVE, UCAT_IA_JS_SETTING,len);
            ws = cci_pointer_data(info);
            
            jc->work_mode = ws->work_mode;
            jc->barbecue_fire = ws->barbecue_fire;
            jc->food_weight = htons(ws->food_weight);
            jc->hot_fan_temp = ws->hot_fan_temp;
            jc->wave_fire = ws->wave_fire;
            jc->work_min = ws->work_min;
            jc->work_sec = ws->work_sec;
            jc->work_sub_mode = ws->work_sub_mode;
            jc->action = ws->action;
            break;
        case ACT_JS_FAST_CTRL:
            len = sizeof(*fc);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_JS_MICWAVE, UCAT_IA_JS_FAST_EXEC,len);
            fc->action = cci_u8_data(info);
            break;
        case ACT_JS_CHILD_LOCK:
            len = sizeof(*lc);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_JS_MICWAVE, UCAT_IA_JS_CHILD_LOCK,len);
            lc->on_off = !! cci_u8_data(info);
            break;
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    
    if (len > 0) {
        sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//晶石微波炉
static bool _kxm_ther_update_data(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    cl_kxm_thermost_info_t* info = ac->com_udp_dev_info.device_info;
    ucp_kxm_ther_ctrl_t* ks = OBJ_VALUE(obj, ucp_kxm_ther_ctrl_t*);
    if( !info || !obj ){
        return false;
    }
    
    switch (obj->attr) {
        case UCAT_IA_KXM_T_ONOFF:
            if (!is_obj_less_than_len(obj, sizeof(*ks))) {
                info->onoff = !! ks->value;
            }
            break;
        case UCAT_IA_KXM_T_MODE:
            if (!is_obj_less_than_len(obj, sizeof(*ks))) {
                info->mode = ks->value;
            }
            break;
        case UCAT_IA_KXM_T_TEMP:
            if (!is_obj_less_than_len(obj, sizeof(*ks))) {
                info->setting_temp = ks->value;
            }
            break;
        case UCAT_IA_KXM_T_ENV_TEMP:
            if (!is_obj_less_than_len(obj, sizeof(*ks))) {
                info->room_temp = (int8_t)ks->value;
            }
            break;
        case UCAT_IA_KXM_T_FAN_SPEED:
            if (!is_obj_less_than_len(obj, sizeof(*ks))) {
                info->fan_speed = ks->value;
            }
            break;
        case UCAT_IA_KXM_T_ENERGY:
            if (!is_obj_less_than_len(obj, sizeof(*ks))) {
                info->energy_cons = !!ks->value;
            }
            break;
            
        default:
            return false;
            break;
    }
    
    return true;
}

static bool _kxm_ther_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    int len = 0;
    u_int8_t buf[128] = {0},attr = 0;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    cl_kxm_thermost_info_t* kti ;
    ucp_kxm_ther_ctrl_t* ks  = (ucp_kxm_ther_ctrl_t*)(uo+1);
    u_int16_t data;
    
    if(!user || !user->smart_appliance_ctrl){
        *ret = RS_OFFLINE;
        return false;
    }
    
    ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    info = (cln_common_info_t *)&pkt->data[0];
    kti = ac->com_udp_dev_info.device_info;
    if (!kti) {
        *ret = RS_OFFLINE;
        return false;
    }
    switch (info->action) {
        case ACT_KXM_THER_COMMON_CTRL:
            data = cci_u16_data(info);
            len = sizeof(*ks);
            switch ((data >> 8)&0xFF) {
                case KXM_CA_ONOFF:
                    kti->onoff = !!(data & 0xFF);
                    ks->value = kti->onoff;
                    attr = UCAT_IA_KXM_T_ONOFF;
                    break;
                case KXM_CA_MODE:
                    kti->mode = data & 0xFF;
					//节能模式下，改变工作模式时，同时设置温度：制冷模式设置28度，制热设置16度
					if (kti->energy_cons) {
						switch(kti->mode) {
						case KXM_WM_COLD:
							kti->setting_temp = 28;
							break;
						default:
							kti->setting_temp = 16;
							break;
						}
						ks->value = kti->setting_temp;
						fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_KXM_THER, UCAT_IA_KXM_T_TEMP,len);
			        	sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
					}
					
                    ks->value = kti->mode;
                    attr = UCAT_IA_KXM_T_MODE;
                    break;
                case KXM_CA_TEMP:
					if (kti->energy_cons) {
						kti->energy_cons = 0;
						ks->value = 0;
						fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_KXM_THER, UCAT_IA_KXM_T_ENERGY,len);
			        	sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
					}
                    kti->setting_temp = data & 0xFF;
                    ks->value = kti->setting_temp;
                    attr = UCAT_IA_KXM_T_TEMP;
                    break;
                case KXM_CA_FS:
                    kti->fan_speed = data & 0xFF;
                    ks->value = kti->fan_speed;
                    attr = UCAT_IA_KXM_T_FAN_SPEED;
                    break;
                case KXM_CA_EC:
                    kti->energy_cons = !!(data & 0xFF);
					if (kti->energy_cons) {
						//设置节能时，制冷28度，制热16度
						switch(kti->mode) {
						case KXM_WM_COLD:
							kti->setting_temp = 28;
							break;
						default:
							kti->setting_temp = 16;
							break;
						}
						ks->value = kti->setting_temp;
				        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_KXM_THER, UCAT_IA_KXM_T_TEMP,len);
				        sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
					}
					
                    ks->value = kti->energy_cons;
                    attr = UCAT_IA_KXM_T_ENERGY;
                    break;
                default:
                    *ret = RS_INVALID_PARAM;
                    return false;
                    break;
            }
            break;
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    
    if (len > 0) {
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_KXM_THER, attr,len);
        sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
    }
    
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
//车载悟空
static bool _car_update_data(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	int i;
	char *buf = NULL;
    cl_car_info_t* ci;
	ucp_car_on_t *pon = NULL;
	ucp_car_temp_t *ptemp = NULL;
	cl_car_alarm_t *palaram = NULL;
	ucp_car_search_t *psearch = NULL;
	ucp_car_powersave_t *power = NULL;
	ucp_car_valcheck_t *pvalcheck = NULL;
	ucp_car_percentage_t *ppercen = NULL;

	if(!ac || !obj || !ac->com_udp_dev_info.device_info){
		return false;
	}
    
    ci = ac->com_udp_dev_info.device_info;	

	switch(obj->attr) {
	case UCAT_IA_CAR_WUKONG_ON:
		if(!is_valid_obj_data(obj,sizeof(*pon))){
			return false;
		}
		
		pon = OBJ_VALUE(obj, ucp_car_on_t*);
		ci->on = pon->on;
		ci->last_on_time = pon->last_on_time;
		ci->on_keep_time = pon->keep_time;
		break;
	case UCAT_IA_CAR_WUKONG_SERCH:
		if(!is_valid_obj_data(obj,sizeof(*psearch))){
			return false;
		}
		
		psearch = OBJ_VALUE(obj, ucp_car_search_t*);
		ci->horn_num = psearch->horn_num;
		ci->horn_time = psearch->horn_time;
		ci->horn_interval = psearch->horn_interval;
		ci->light_num = psearch->light_num;
		ci->light_time = psearch->light_time;
		ci->light_interval = psearch->light_interval;
		break;
	case UCAT_IA_CAR_WUKONG_TEMP:
		if(!is_valid_obj_data(obj,sizeof(*ptemp))){
			return false;
		}
		
		ptemp = OBJ_VALUE(obj, ucp_car_temp_t*);
		ci->temp = ptemp->temp;
		break;
	case UCAT_IA_CAR_WUKONG_VALCHECK:
		if(!is_valid_obj_data(obj,sizeof(*pvalcheck))){
			return false;
		}

		pvalcheck = OBJ_VALUE(obj, ucp_car_valcheck_t*);
		ci->valtage = htons(pvalcheck->valtage);
		ci->val_on = pvalcheck->on;
		break;
	case UCAT_IA_CAR_WUKONG_POWERSAVE:
		if(!is_valid_obj_data(obj,sizeof(*power))){
			return false;
		}

		power = OBJ_VALUE(obj, ucp_car_powersave_t*);
		ci->powersave_on = power->on;
		break;
	case UCAT_IA_CAR_WUKONG_ALARM:
		ci->alarm_num = 0;
		if (ci->car_alarm) {
			SAFE_FREE(ci->car_alarm);
		}
		if (obj->param_len > 0 &&
			(obj->param_len%sizeof(cl_car_alarm_t) == 0)) {
			ci->car_alarm = cl_calloc(obj->param_len, 1);
			if (ci->car_alarm) {
				ci->alarm_num = obj->param_len/sizeof(cl_car_alarm_t);
				memcpy((void *)ci->car_alarm, (void *)(obj+1), obj->param_len);
				palaram = ci->car_alarm;
				for(i = 0; i < ci->alarm_num; i++) {
					palaram[i].time = htonl(palaram[i].time);
				}
				
			}
			
		}
		break;
	case UCAT_IA_CAR_WUKONG_ELE_PERCENTAGE:
		if(!is_valid_obj_data(obj,sizeof(*ppercen))){
			return false;
		}

		ppercen = OBJ_VALUE(obj, ucp_car_percentage_t*);
		ci->ele_percentage = ppercen->ele_per;
		break;
	case UCAT_IA_CAR_WUKONG_DEBUG_INFO:
		if (ci->debug_info) {
			cl_free(ci->debug_info);
			ci->debug_info = NULL;
		}
		buf = (char *)(obj+1);
		buf[obj->param_len - 1] = 0;
		ci->debug_info = cl_strdup(buf);
		break;
	default:
		break;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////
// 808 TV AND STB
static bool _stb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    char buf[256] = {0};
    smart_air_ctrl_t* ac;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_rc_match_t* rcm;
    ucp_rc_info_t* ri;
    ucp_rc_key_ctrl_t* kc;
    ucp_rc_key_oper_t* ko;
    u_int16_t u16_value;
    u_int32_t u32_value;
    int len = 0;
    char* name,*p;
    bool need_query = false;
    priv_rc_manage_info* prc;
    ucp_rc_key_learn_t* kl;
    
    info = (cln_common_info_t *)&pkt->data[0];
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac || !ac->sac->user->online) {
        return false;
    }
    
    prc = &ac->rc_pm;
    
    switch(info->action){
       case ACT_STB_CHANGE_NAME:
        {
            name = cci_pointer_data(info);
            len = sizeof(*ri) ;
            ri = (ucp_rc_info_t*)(uo+1);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_INFO,len);
            
            strncpy((char*)ri->name, name, sizeof(ri->name));
            ri->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            ri->type = RC_TYPE_STB;
            ri->rc_id = prc->pair_rc.stb_info.d_id;
            need_query = true;
        }
            break;
        case ACT_STB_START_MATCH:
        {
            rcm = (ucp_rc_match_t*)(uo+1);
            u32_value = cci_u32_data(info);
            
            rcm->rc_id = (u32_value >> 16) & 0xff;
            if (prc->pair_rc.stb_info.d_id == rcm->rc_id) {
                rcm->type = RC_TYPE_STB;
                rcm->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            }else if(prc->pair_rc.tv_info.d_id == rcm->rc_id){
                rcm->type = RC_TYPE_TV;
                rcm->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            rcm->action = CL_AIR_CODE_MATCH_CLOUD;
            rcm->timeout = u32_value & 0xff;
            
            len = sizeof(*rcm);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_MATCH,len);
        }
            break;
        case ACT_STB_START_NKEY_MATCH:
        {
            rcm = (ucp_rc_match_t*)(uo+1);
            u32_value = cci_u32_data(info);
            
            rcm->rc_id = (u32_value >> 24) & 0xff;
            if (prc->pair_rc.stb_info.d_id == rcm->rc_id) {
                rcm->type = RC_TYPE_STB;
                rcm->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            }else if(prc->pair_rc.tv_info.d_id == rcm->rc_id){
                rcm->type = RC_TYPE_TV;
                rcm->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            rcm->action = 0x7;
            rcm->timeout = u32_value & 0xff;
            rcm->next_except_key = (u32_value >> 16) & 0xff;
            
            len = sizeof(*rcm);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_MATCH,len);
        }
            break;
        case ACT_STB_STOP_MATCH:
        {
            rcm = (ucp_rc_match_t*)(uo+1);
            
            rcm->rc_id = cci_u8_data(info);
            if (prc->pair_rc.stb_info.d_id == rcm->rc_id) {
                rcm->type = RC_TYPE_STB;
                rcm->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            }else if(prc->pair_rc.tv_info.d_id == rcm->rc_id){
                rcm->type = RC_TYPE_TV;
                rcm->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            rcm->action = CL_AIR_CODE_MATCH_STOP;
            
            len = sizeof(*rcm);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_MATCH,len);
        }
            break;
        case ACT_STB_CTRL_KEY:
        {
            u16_value = cci_u16_data(info);
            kc = (ucp_rc_key_ctrl_t*)(uo+1);
            
            kc->rc_id = (u16_value >> 8) & 0xff;
            kc->key_id = u16_value & 0xff;
            
            if (prc->pair_rc.stb_info.d_id == kc->rc_id) {
                kc->type = RC_TYPE_STB;
                kc->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            }else if(prc->pair_rc.tv_info.d_id == kc->rc_id){
                kc->type = RC_TYPE_TV;
                kc->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            
            len = sizeof(*kc);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_CTRL,len);

        }
            break;
        case ACT_STB_QUICK_ONOFF:
        {
            u16_value = cci_u16_data(info);
            len = sizeof(*kc);
            
            kc = (ucp_rc_key_ctrl_t*)(uo+1);
            kc->rc_id = (u16_value >> 8) & 0xff;
            kc->key_id = TV_KEY_ONOFF;
            kc->type = RC_TYPE_TV;
            kc->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_CTRL,len);
            
            sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_CTRL,len);
            kc = (ucp_rc_key_ctrl_t*)(uo+1);
            kc->rc_id = u16_value & 0xff;
            kc->key_id = STV_KEY_ONOFF;
            kc->type = RC_TYPE_STB;
            kc->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            
            return sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo)+len);
            
        }
            break;
        case ACT_STB_START_LEARN:
            u16_value = cci_u16_data(info);
            kl = (ucp_rc_key_learn_t*)(uo+1);
            
            kl->rc_id = (u16_value >>8) & 0xff;
            kl->key_id = u16_value & 0xff;
            
            if (prc->pair_rc.stb_info.d_id == kl->rc_id) {
                kl->type = RC_TYPE_STB;
            }else if(prc->pair_rc.tv_info.d_id == kl->rc_id){
                kl->type = RC_TYPE_TV;
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            kl->action = ACT_RC_KL_START;
            
            len = sizeof(*kl);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_LEARN,len);
            break;
        case ACT_STB_STOP_LEARN:
            u16_value = cci_u16_data(info);
            kl = (ucp_rc_key_learn_t*)(uo+1);
            
            kl->rc_id = (u16_value >>8) & 0xff;
            kl->key_id = u16_value & 0xff;
            
            if (prc->pair_rc.stb_info.d_id == kl->rc_id) {
                kl->type = RC_TYPE_STB;
            }else if(prc->pair_rc.tv_info.d_id == kl->rc_id){
                kl->type = RC_TYPE_TV;
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            kl->action = ACT_RC_KL_STOP;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_LEARN,len);
            len = sizeof(*kl);
            
            return sa_ctrl_obj_value(user->uc_session, UCA_DELETE, false, 0x1, buf, sizeof(*uo)+len);
            break;
        case ACT_STB_DELETE_KEY:
            u16_value = cci_u16_data(info);
            ko = (ucp_rc_key_oper_t*)(uo+1);
            
            ko->rc_id = (u16_value >> 8) & 0xff;
            ko->key_id = u16_value & 0xff;
            if (prc->pair_rc.stb_info.d_id == ko->rc_id) {
                ko->type = RC_TYPE_STB;
                ko->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            }else if(prc->pair_rc.tv_info.d_id == ko->rc_id){
                ko->type = RC_TYPE_TV;
                ko->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_KEY,sizeof(*ko));
            
            return sa_ctrl_obj_value(user->uc_session, UCA_DELETE, false, 0x1, buf, sizeof(*uo)+sizeof(*ko));
            
            break;
        case ACT_STB_MODIFY_KEY:
            p = cci_pointer_data(info);
            ko = (ucp_rc_key_oper_t*)(uo+1);
            
            ko->rc_id = p[0] & 0xff;
            ko->key_id = p[1] & 0xff;
            if (prc->pair_rc.stb_info.d_id == ko->rc_id) {
                ko->type = RC_TYPE_STB;
                ko->ir_id = htons(prc->pair_rc.stb_info.matched_ir_id);
            }else if(prc->pair_rc.tv_info.d_id == ko->rc_id){
                ko->type = RC_TYPE_TV;
                ko->ir_id = htons(prc->pair_rc.tv_info.matched_ir_id);
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            
            strncpy((char*)ko->name, p+2, sizeof(ko->name)-1);
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_KEY,sizeof(*ko));
            
            len = sizeof(*ko);
            
            break;
        default:
            *ret = RS_INVALID_PARAM;
            break;
    }
    
    if(len > 0){
        if (need_query) {
            sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
        }else{
            sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
        }
        
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//车载悟空
static bool _car_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	int len = 0;
	bool need_query = false;
	cln_common_info_t *info;
	cl_car_info_t *ci = NULL;
	u_int8_t u8_value = 0;
	u_int16_t u16_value = 0;
	ucp_car_search_t *psearch_temp = NULL;
    char buf[256] = {0};
    smart_air_ctrl_t* ac;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_car_on_t *pon = (ucp_car_on_t *)&uo[1];
//	ucp_car_temp_t *ptemp = (ucp_car_temp_t *)&uo[1];
	ucp_car_search_t *psearch = (ucp_car_search_t *)&uo[1];
	ucp_car_valcheck_t *pvalcheck = (ucp_car_valcheck_t *)&uo[1];
	ucp_car_powersave_t *power = (ucp_car_powersave_t *)&uo[1];

	info = (cln_common_info_t *)&pkt->data[0];
	
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac || !ac->sac->user->online) {
        return false;
    }

	ci = (cl_car_info_t *)ac->com_udp_dev_info.device_info;

	switch(info->action){
	case ACT_CAR_CONFIG_ON:
		u8_value = cci_u8_data(info);
		ci->on_keep_time = u8_value;

		len = sizeof(ucp_car_on_t);
		pon->on = ci->on;
		pon->keep_time = ci->on_keep_time;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_ON,len);
		break;
	case ACT_CAR_CTRL_ON:
		len = sizeof(ucp_car_on_t);
		u8_value = cci_u8_data(info);
		pon->on = u8_value;
		pon->keep_time = ci->on_keep_time;
		ci->on = u8_value;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_ON,len);
		break;
	case ACT_CAR_CONFIG_SEARCH:
		psearch_temp = (ucp_car_search_t *)cci_pointer_data(info);
		ci->horn_num = psearch_temp->horn_num;
		ci->horn_time = psearch_temp->horn_time;
		ci->horn_interval = psearch_temp->horn_interval;
		ci->light_num = psearch_temp->light_num;
		ci->light_time = psearch_temp->light_time;
		ci->light_interval = psearch_temp->light_interval;

		len = sizeof(ucp_car_search_t);
		psearch->horn_num = ci->horn_num;
		psearch->horn_time = ci->horn_time;
		psearch->horn_interval = ci->horn_interval;
		psearch->light_num = ci->light_num;
		psearch->light_time = ci->light_time;
		psearch->light_interval = ci->light_interval;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_SERCH,len);		
		break;
	case ACT_CAR_CTRL_SEARCH:
		len = sizeof(ucp_car_search_t);
		psearch->horn_num = ci->horn_num;
		psearch->horn_time = ci->horn_time;
		psearch->horn_interval = ci->horn_interval;
		psearch->light_num = ci->light_num;
		psearch->light_time = ci->light_time;
		psearch->light_interval = ci->light_interval;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_SERCH,len);
		break;
	case ACT_CAR_CONFIG_VALTAGE:
		u16_value = cci_u16_data(info);
		ci->valtage = u16_value;

		len = sizeof(ucp_car_valcheck_t);
		u8_value = cci_u8_data(info);
		pvalcheck->valtage = htons(ci->valtage);
		pvalcheck->on = u8_value;
		ci->val_on = u8_value;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_VALCHECK,len);
		break;
	case ACT_CAR_CTRL_VALTAGE:
		len = sizeof(ucp_car_valcheck_t);
		u8_value = cci_u8_data(info);
		pvalcheck->valtage = htons(ci->valtage);
		pvalcheck->on = u8_value;
		ci->val_on = u8_value;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_VALCHECK,len);
		break;
	case ACT_CAR_CTRL_POWERSAVE:
		len = sizeof(ucp_car_powersave_t);
		u8_value = cci_u8_data(info);
		power->on = u8_value;
		ci->powersave_on = u8_value;
		need_query = true;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CAR_WUKONG, UCAT_IA_CAR_WUKONG_POWERSAVE,len);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;
	}

    if(len > 0){
		//控制后立马更新数据
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		
        if (need_query) {
            sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
        }else{
            sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
        }
        
    }
	
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////
// 沙特和晴乐插座
static bool _eplug_oem_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    char buf[256] = {0};
    smart_air_ctrl_t* ac;
    cl_eplug_oem_stat* ci;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    int len = 0;
    bool need_query = true;
    cln_common_info_t *info;
    u_int8_t * dest = (u_int8_t*)(uo+1);
    u_int32_t value;
    ucp_ep_temp_range_t* et;
    
    
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac || !ac->sac->user->online) {
        return false;
    }
    
    ci = (cl_eplug_oem_stat *)ac->com_udp_dev_info.device_info;
    info = (cln_common_info_t *)&pkt->data[0];
    
    switch (info->action) {
        case ACT_EO_SET_ONOFF:
            len = sizeof(u_int32_t);
            ci->onoff = !!cci_u8_data(info);
            *dest = ci->onoff;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EPLUG_OEM, UCAT_IA_EP_OEM_ONOFF,len);
            break;
        case ACT_EO_SET_TEMP_RANGE:
            len = sizeof(ucp_ep_temp_range_t);
            et = (ucp_ep_temp_range_t*)(uo+1);
            value = cci_u32_data(info);
            
            ci->range_enable = !!((value >> 24) & 0xff);
            ci->range_max_temp = ((value >> 16) & 0xff);
            ci->range_min_temp = ((value >> 8) & 0xff);
            
            et->enable = ci->range_enable;
            et->max = ci->range_max_temp;
            et->low = ci->range_min_temp;
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EPLUG_OEM, UCAT_IA_EP_OEM_TEMP_RANGE,len);
            break;
        case ACT_EO_SET_THRESHOLD:
            value = cci_u32_data(info);
            len = sizeof(u_int32_t);
            ci->temp_threshold_enable = !!((value >> 24) & 0xff);
            ci->temp_threshold_value = ((value >> 16) & 0xff);
            *dest++ = ci->temp_threshold_enable;
            *dest = ci->temp_threshold_value;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EPLUG_OEM, UCAT_IA_EP_OEM_MAX_TEMP,len);
            break;
        case ACT_EO_SET_OFFLINE_ENABLE:
            len = sizeof(u_int32_t);
            ci->off_line_close_enable = !!cci_u8_data(info);
            *dest = ci->off_line_close_enable;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EPLUG_OEM, UCAT_IA_EP_OEM_OFFLINE_PROTECT,len);
            break;
        case ACE_EO_SET_PERSON_ENABLE:
            if(!ci->is_support_person_detect){
                *ret = ERR_PARAM_INVALID;
                return false;
            }
            len = sizeof(u_int32_t);
            ci->person_detect_enable = !!cci_u8_data(info);
            *dest = ci->person_detect_enable;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_EPLUG_OEM, UCAT_IA_EP_OEM_PERSION_DETECT,len);
            break;
        default:
            break;
    }
    
    if(len > 0){
        if (need_query) {
            sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
        }else{
            sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
        }
        
    }
    
    return true;
}

static bool _eplug_update_data(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
    cl_eplug_oem_stat * es;
    ucp_ep_temp_range_t* er;
    u_int8_t* value;
    
    if (!ac || !obj || !ac->com_udp_dev_info.device_info) {
        return false;
    }
    
    es = (cl_eplug_oem_stat *)ac->com_udp_dev_info.device_info;
    
    switch (obj->attr) {
        case UCAT_IA_EP_OEM_ONOFF:
            if (!is_obj_less_than_len(obj, sizeof(u_int32_t))) {
                es->onoff = !!obj_u8_value(obj);
            }
            break;
        case UCAT_IA_EP_OEM_ROOM_TEMP:
            if (!is_obj_less_than_len(obj, sizeof(u_int32_t))) {
                es->room_temp = obj_u8_value(obj);
            }
            break;
        case UCAT_IA_EP_OEM_OFFLINE_PROTECT:
            if (!is_obj_less_than_len(obj, sizeof(*er))) {
                es->off_line_close_enable = !!obj_u8_value(obj);
            }
            break;
        case UCAT_IA_EP_OEM_MAX_TEMP:
            if (!is_obj_less_than_len(obj, sizeof(u_int32_t))) {
                value = OBJ_VALUE(obj, u_int8_t*);
                es->temp_threshold_enable = !!(*value);
                value++;
                es->temp_threshold_value = *value;
            }
            break;
        case UCAT_IA_EP_OEM_TEMP_RANGE:
            if (!is_obj_less_than_len(obj, sizeof(u_int32_t))) {
                er = OBJ_VALUE(obj, ucp_ep_temp_range_t*);
                es->range_enable = er->enable;
                es->range_max_temp = er->max;
                es->range_min_temp = er->low;
            }
            break;
        case UCAT_IA_EP_OEM_PERSION_DETECT:
            if (!is_obj_less_than_len(obj, sizeof(u_int32_t))) {
                es->is_support_person_detect = true;
                es->person_detect_enable = !!obj_u8_value(obj);
            }
            break;
            
        default:
            break;
    }
    
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//设置是否支持标志
static void _udp_set_ia_support_flag(smart_air_ctrl_t* ac, uc_sys_support_obj_t* so)
{
    switch (so->sub_obj) {
        case UCSOT_IA_COMMON_STAT://通用电量统计
            ac->com_udp_dev_info.is_suppport_elec_stat = true;
            break;
        case UCSOT_IA_PUBLIC:
		if(so->attr == UCAT_IA_PUBLIC_EXCEPTION || so->attr == UCAT_IA_PUBLIC_EXCEPTION_V2){
			ac->com_udp_dev_info.is_support_dev_err_info = so->attr == UCAT_IA_PUBLIC_EXCEPTION ? 1 : 2;
		}else if(so->attr == UCAT_IA_PUBLIC_PERIOD_TIMER ){
			ac->com_udp_dev_info.is_support_period_timer = true;
			ac->air_info.is_support_peroid_timer = true;
		}else if((so->attr == UCAT_IA_PUBLIC_WIFI_SETTING)){
			ac->com_udp_dev_info.is_support_dev_set_wifi_param = true;
        }else if (so->attr == UCAT_IA_PUBLIC_EXT_PERIOD_TIMER){
            ac->com_udp_dev_info.is_support_ext_period_timer = true;
            ac->com_udp_dev_info.is_support_period_timer = true;
            ac->air_info.is_support_peroid_timer = true;
            ac->air_info.is_support_peroid_ext_timer = true;
        }else if (so->attr == UCAT_IA_TMP_CURVE) {
			ac->air_info.is_support_temp_curve = true;
        }else if(so->attr == UCAT_IA_TMP_UTC_CURVE){
            ac->air_info.is_support_utc_temp_curve = true;
            ac->air_info.is_support_temp_curve = true;
			// 放在通用里面
			ac->com_udp_dev_info.is_support_utc_temp_curve = true;
        } else if (so->attr == UCAT_IA_PUBLIC_UTC_TMP_CTRL) {
        	ac->com_udp_dev_info.is_support_public_utc_temp_ac_ctrl = true;
        } else if (so->attr == UCAT_IA_PUBLIC_CHILD_LOCK) {
        	ac->com_udp_dev_info.is_support_public_child_lock = true;
        } else if (so->attr == UCAT_IA_PUBLIC_TEMP_ALARM) {
        	ac->com_udp_dev_info.is_support_public_temp_alarm = true;
        } else if (so->attr == UCAT_IA_PUBLIC_SMART_ON) {
        	ac->com_udp_dev_info.is_support_public_smart_on = true;
        } else if (so->attr == UCAT_IA_PUBLIC_SHORTCUT_TIMER) {
        	ac->com_udp_dev_info.is_support_public_shortcuts_onoff = true;
        } else if (so->attr == UCAT_IA_PUBLIC_HISTORY_INFO) {
        	ac->com_udp_dev_info.is_support_dev_history = true;
       } else if (so->attr == UCAT_IA_PUBLIC_BOOT_TEMP) {
        	ac->com_udp_dev_info.is_support_boot_temp = true;
       } else if (so->attr == UCAT_IA_PUBLIC_WAN_CONFIG) {
        	ac->com_udp_dev_info.is_support_wan_config = true;
       } else if (so->attr == UCAT_IA_PUBLIC_DHCP_SERVER) {
        	ac->com_udp_dev_info.is_support_dhcp_server = true;
       } else if (so->attr == UCAT_IA_PUBLIC_AP_CONFIG) {
        	ac->com_udp_dev_info.is_support_ap_config= true;
       } else if (so->attr == UCAT_IA_PUBLIC_REPEATER) {
        	ac->com_udp_dev_info.is_support_repeat = true;
       } else if (so->attr == UCAT_IA_PUBLIC_BACKGROUND) {
			ac->sac->user->is_support_background_set = true;
	   } else if (so->attr== UCAT_IA_PUBLIC_TELNET) {
			ac->sac->user->is_support_telnet = true;
	   }
	   
	   	break;
		case UCSOT_IA_AC:
            if (so->attr == UCAT_AC_UTC_TMP_CTRL) {
                ac->air_info.is_support_temp_ac_ctrl = true;
                ac->air_info.is_support_utc_temp_ac_ctrl = true;
            }
            if (so->attr == UCAT_IA_TMP_CTRL) {
                ac->air_info.is_support_temp_ac_ctrl = true;
            }
		break;
        default:
            break;
    }
}

static void _udp_set_system_support_flag(smart_air_ctrl_t* ac, uc_sys_support_obj_t* so)
{
    switch (so->sub_obj) {
        case UCSOT_SYS_HARDWARE://通用单片机升级
		if(so->attr == UCAT_HARDWARE_STM_VERSION){
            ac->com_udp_dev_info.is_support_stm_upgrade = true;
		}
        if (so->attr == UCATT_HARDWARE_LED && (ac->sac->user->sub_type == IJ_101 || ac->sac->user->sub_type == IJ_102)) {
            ac->u.eb_info.is_support_ctrl_led = true;
        }
		if (so->attr == UCAT_HARDWARE_DISK_INFO_GET) {
			ac->com_udp_dev_info.is_support_disk = true;
		}
		if (so->attr == UCAT_HARDWARE_ETH_INFO_GET) {
			ac->com_udp_dev_info.is_support_eth = true;
		}
        break;
	case UCSOT_SYS_SOFTWARE:
		if(so->attr == UCAT_SYS_RESTORY_FACTORY){
			ac->com_udp_dev_info.is_support_dev_restory_factory = true;
		}

		if (so->attr == UCAT_SYS_WIFI_STATE) {
			ac->com_udp_dev_info.is_support_dev_wifi_state = true;
		}	


		if (so->attr == UCAT_SYS_WIFI_STATE_S3) {
			ac->com_udp_dev_info.is_support_dev_wifi_state = true;
		}

		if (so->attr == UCAT_SYS_STM_UPGRADE_PREINFO) {
			ac->com_udp_dev_info.is_support_spe_up= true;
			ac->sac->user->is_support_spe_up = true;
		}
		
		break;
	case UCSOT_SYS_SERVER:
		if(so->attr == UCAT_SERVER_HOMEID){
			ac->com_udp_dev_info.is_support_la = true;
		}
		break;

	case UCSOT_LANUSERS_MANAGE:
		if (so->attr == UCAT_LANUSERS_MANAGE_ENABLE) {
			ac->com_udp_dev_info.support_lanusers_manage = true;
		}
		break;

	default:
        break;
    }
}

void udp_set_support_flag_hook(smart_air_ctrl_t* ac, uc_sys_support_obj_t* so)
{
	if(!ac|| !so){
		return;
	}
	
	switch(so->obj){
		case UCOT_SYSTEM:
			_udp_set_system_support_flag(ac,so);
			break;
        	case UCOT_IA:
			_udp_set_ia_support_flag(ac,so);
			break;
		default:
			break;
	}

}
// 判断命令是否支持离线操作
bool udp_proc_support_offline(u_int32_t notify_type)
{
	switch (notify_type) {
		case CLNE_COMMON_UDP_INDIACAR:
			return true;
	}

	return false;
}

// 判断一个udp_ctrl命令是否在APP SERVER那边支持
bool is_supported_udp_app_server(u_int8_t obj, u_int8_t sub_obj, u_int8_t attr)
{
	if (obj == UCOT_IA) {
		if (sub_obj == UCSOT_IA_INDIA_CAR) {
			return true;
		}
	}

	return false;
}


//udp通用处理，处理APP 上层请求
bool udp_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = false;
	user_t* user ;
	cln_common_info_t *info;

    if (pkt->type > CLNE_RFGW_START  && pkt->type < CLNE_RFGW_END) {
        return rfgw_proc_notify_hook(pkt,ret);
    }
    
	if(pkt->type <= CLNE_COMMON_UDP_START || pkt->type >= CLNE_COMMON_UDP_END){
		return res;
	}
	
	info = (cln_common_info_t *)&pkt->data[0];
	user = lookup_by_handle(HDLT_USER, info->handle);

	// 支持离线操作的UDP设备
	if (user && user->is_udp_ctrl) {
		if (udp_proc_support_offline(pkt->type) && !user->smart_appliance_ctrl) {
			sa_init(user);
		}
		if (user->smart_appliance_ctrl) {
			goto proc;
		}
	}

	if (!user|| !user->is_udp_ctrl || !user->smart_appliance_ctrl ) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

//	if(  !is_supported_udp_device(user->sub_type,user->ext_type) ){
//		*ret = RS_INVALID_PARAM;
//		return res;
//	}

	if (!user->online){
		*ret = RS_OFFLINE;
		return res;
	}

proc:
	switch(pkt->type){
		case CLNE_COMMON_UDP_TIMER_CTRL://通用定时器处理
			res = _udp_proc_timer_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_COMMON_CTRL:
			res = _udp_proc_common_notify(user,pkt,ret);
			break;
		case  CLNE_COMMON_UDP_TB_HEATER_CTRL:
			res = _tb_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_YL_AC_CTRL:
			res = _yl_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_LEDE_LAMP_CTRL:
			res = _lede_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_JNB_DEVICE:
			res = _jnb_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_AMT_FAN_CTRL:
			res = amt_scm_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL:
			res = chiffo_scm_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_EVM_DEVICE:
			res = evm_scm_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_HX_CTRL:
			res = _hx_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_TL_CTRL:
			res = _tl_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_QPCP_CTRL:
			res = _qpcp_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_QP_POT_CTRL:
		    res = qp_pot_proc_notify(user, pkt, ret);
		    break;
		case CLNE_COMMON_UDP_STB_TV_CTRL:
		    res = _stb_proc_notify(user, pkt, ret);
		    break;
		case CLNE_COMMON_UDP_CAR:
		    res = _car_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_EPLUG_OEM:
		    res = _eplug_oem_proc_notify(user, pkt, ret);
		    break;
		case CLNE_COMMON_UDP_THERMOSTAT_XY:
			res = xy_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_BIMAR:
		    res = bimar_scm_proc_notify(user, pkt, ret);
		    break;
		case CLNE_COMMON_UDP_QP_PBJ:
		    res = qp_pbj_proc_notify(user, pkt, ret);
		    break;
		case CLNE_COMMON_UDP_TBB:
			res = tbb_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_HX_YSH:
		    res = hx_ysh_proc_notify(user,pkt,ret);
		    break;
		case CLNE_COMMON_UDP_YT:
			res = _yt_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_ADS:
			res = _ads_proc_notify(user,pkt,ret);
			break;
		case CLNE_COMMON_UDP_ZH_JL_LAMP:
			res = zh_jl_lamp_scm_proc_notify(user,pkt,ret);
			break;
        case CLNE_COMMON_UDP_JS_WAVE:
            res = _js_wave_proc_notify(user, pkt, ret);
            break;
        case CLNE_COMMON_UDP_KXM:
            res = kxm_scm_proc_notify(user, pkt, ret);
            break;
        case CLNE_COMMON_UDP_KXM_THER:
            res = _kxm_ther_proc_notify(user, pkt, ret);
            break;
        case CLNE_COMMON_UDP_SBT_THER:
            res = sbt_ther_scm_proc_notify(user, pkt, ret);
            break;
		case CLNE_COMMON_UDP_YJ_HEATER:
			res = yj_heater_scm_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_ZSSX:
			res = _zssx_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_INDIACAR:
			res = indiacar_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_ZKRSQ:
			res = zkrsq_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_LINKON:
			res = linkon_proc_notify(user, pkt, ret);
			break;
		case CLNE_RFGW_SCM_CTRL:
			res = rfgw_scm_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_ZHCL:
			res = zhcl_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_LUM:
			res = lanusers_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_LEIS:
			res = leis_scm_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_YINSU:
			res = yinsu_scm_proc_notify(user, pkt, ret);
			break;
		case CLNE_COMMON_UDP_ZHDHX:
			res = zhdhx_proc_notify(user, pkt, ret);
			break;
		default:
			*ret = RS_NOT_SUPPORT;
			break;
	}
	
	return res;
}

bool udp_quick_query_info_hook(smart_air_ctrl_t* ac)
{
	bool res = true;
	
	if(!ac){
		return false;
	}

	res = udp_quick_query_dev_priv_info(ac);
	
	if(res){
		// 如果是可以支持的设备，判断是否支持通用电量统计
		if(ac->com_udp_dev_info.is_suppport_elec_stat){
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_COMMON_STAT,ALL_SUB_ATTR);
			//单独获取每天的电量统计
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_COMMON_STAT,UCAT_STAT_DAYS_STAT);
		}
		// 如果支持设备异常
		if(ac->com_udp_dev_info.is_support_dev_err_info){
			if (ac->com_udp_dev_info.is_support_dev_err_info == 1) {
				udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_EXCEPTION);
			} else if (ac->com_udp_dev_info.is_support_dev_err_info == 2) {			
				udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_EXCEPTION_V2);
			}
		}
		udp_query_single_object(ac->sac->user->uc_session, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_DEBUGINFO);
		if(ac->com_udp_dev_info.is_support_stm_upgrade){
			udp_query_single_object(ac->sac->user->uc_session,UCOT_SYSTEM,UCSOT_SYS_HARDWARE,UCAT_HARDWARE_STM_VERSION);
		}
        if (ac->com_udp_dev_info.is_support_ext_period_timer) {
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_EXT_PERIOD_TIMER);
        }else if(ac->com_udp_dev_info.is_support_period_timer){
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_PUBLIC_PERIOD_TIMER);
        }
        if (ac->air_info.is_support_utc_temp_curve) {
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_TMP_UTC_CURVE);
        }else if (ac->air_info.is_support_temp_curve) {
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_TMP_CURVE);
		}
        if (ac->air_info.is_support_utc_temp_ac_ctrl) {
            udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_AC,UCAT_AC_UTC_TMP_CTRL);
        }else if (ac->air_info.is_support_temp_ac_ctrl) {
			udp_query_single_object(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_AC,UCAT_IA_TMP_CTRL);
		}

	}
	
	return res;
}

bool udp_update_data_hook(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	user_t* user;
	bool res = false;

	if (air_ctrl&& air_ctrl->sac) {
		user = air_ctrl->sac->user;
	} else {
		log_err(false, "error %s %d\n", __FUNCTION__, __LINE__);
		return false;
	}

	switch(obj->sub_objct){
		case UCSOT_IA_PUBLIC:
			res = _udp_common_do_2_12_public_attr(air_ctrl,action,obj);
			break;
		case UCSOT_IA_COMMON_STAT:
			res = _udp_common_update_elec_stat_data(air_ctrl,action,obj);
			break;
        case UCSOT_IA_CODE:
            res = _udp_update_rc_info(air_ctrl,action,obj);
            break;
		case UCSOT_IA_JCX:
			res = jcx_power_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_INDIA_CAR:
			res = indiacar_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_TBHEATER:
			switch(user->ext_type) {
			case ETYPE_IJ_824_HTC_BUSINESS:
			case TYPE_IJ_824_YCJ:
				res = _tbb_update_data(air_ctrl,action,obj);
				break;
			default:
				res = _tb_update_data(air_ctrl,action,obj);
				break;
			}
			break;
		case UCSOT_IA_ZKRSQ:
			res = _zkrsq_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_LEDE_LED:
			res = _lede_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_THERMOSTAT_YL:
			res = _yl_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_JNB_THERMOSTAT:
			res = _jnb_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_BREAK_MACHINE:
			res = _hx_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_TL_TEMP:
			res = _tl_update_data(air_ctrl,action,obj);
			break;
		case UCSOT_IA_TEA_TRAY_QP:
			res = _qpcp_update_data(air_ctrl,action,obj);
			break;
        case UCSOT_IA_QP_POT:
            res = qp_pot_update_data(air_ctrl, action, obj);
            break;
		case UCSOT_IA_CAR_WUKONG:
			res = _car_update_data(air_ctrl, action, obj);
			break;
        case UCSOT_IA_EPLUG_OEM:
            res = _eplug_update_data(air_ctrl, action, obj);
            break;
		case UCSOT_IA_THERMOSTAT_XY:
			res = _xy_update_data(air_ctrl,action,obj);
			break;
        case UCSOT_IA_QP_PBJ:
            res = qp_pbj_update_data(air_ctrl, action, obj);
            break;
        case UCSOT_IA_HX_YS_POT:
            res = hx_ysh_update_data(air_ctrl, action, obj);
            break;
		case UCSOT_IA_YUETU:
			res = _yt_update_data(air_ctrl, action, obj);
			break;
		case UCSOT_IA_ADS:
			res = _ads_update_data(air_ctrl, action, obj);
			break;
        case UCSOT_IA_JS_MICWAVE:
            res = _js_wave_update_data(air_ctrl, action, obj);
            break;
        case UCSOT_IA_KXM_THER:
            res = _kxm_ther_update_data(air_ctrl, action, obj);
            break;
		case UCSOT_IA_ELEC_HEATER:
			res = _zssx_update_data(air_ctrl, action, obj);
			break;
		case UCSOT_IA_LINKONWKQ:
			res = linkon_update_data(air_ctrl, action, obj);
			break;
		case UCSOT_IA_ZHCL:
			res = zhcl_update_data(air_ctrl, action, obj);
			break;
		case UCSOT_IA_ZHDHX:
			res = zhdhx_update_data(air_ctrl, action, obj);
			break;
		case UCSOT_IA_HPGW:
			res = hpgw_update_data(air_ctrl, action, obj);
			break;
		default:
			break;
	}

	return res;
}

int udp_proc_ctrl_hook(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;

    if(is_except_attr(obj, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_RC_LEARN)){
        if (error != ERR_NONE) {
            return SAE_LEARN_KEY_DEV_BUSY;
        }else{
            return SAE_LEARN_KEY_DEV_READY_OK;
        }
    }

    if(is_except_attr(obj, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_PLAN)){
        if (error == ERR_SCENE_ID_MAX) {
            return SAE_SCENE_ID_MAX;
        }
    }

	
	// 统一处理，若需要单独event，自行添加
	if(error == ERR_NONE)
		event = SAE_COMMON_CTRL_OK;
	else
		event = SAE_COMMON_CTRL_FAILED;

	return event;
}

static bool udp_session_has_ctrl_pkt(ucc_session_t *s)
{
	pkt_t *pkt, *next;
	int n = 0;
	
	if (stlc_list_empty(&s->send_list)) {
		return false;
	}

	//本身自己还没删除,不理第一个
	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_list, link) {
		if ((n > 0) && 
			(pkt->cmd == CMD_UDP_CTRL) && 
			(pkt->action == UCA_SET)) {
			return true;
		}
		n++;
	}
	
	return false;
}

int udp_proc_ctrl_modify_query(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	user_t* user = NULL;
	ucc_session_t *s = NULL;

	user = ctrl->sac->user;
	s = user->uc_session;
	
	if (!user->last_ctrl) {
		return 0;
	}

	//判断发送链表里还有244控制报文没
	if (udp_session_has_ctrl_pkt(s)) {
		return 0;
	}

	//全查询一下
	//log_debug("udp_proc_ctrl_modify_query all !!!!!!!!11\n");
	udp_quick_query_dev_priv_info(ctrl);
	user->last_ctrl = false;
	
	return 0;
}

int udp_proc_ctrl_modify_hook(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	user_t* user = NULL;


	if (!ctrl || !ctrl->sac || !ctrl->sac->user) {
		return -1;
	}
	
	user = ctrl->sac->user;

	//表示上次没控制过，直接返回
	if (!user->last_ctrl) {
		return 0;
	}
	log_debug("udp_proc_ctrl_modify_hook  go for query !!!!!!!!!!!!!!!!!!!!!!1\n");
	switch(ctrl->sac->user->sub_type) {
	case IJ_838_YT:
	default:
		udp_proc_ctrl_modify_query(ctrl, obj, error);
		break;
	}

	return 0;
}

bool udp_build_objs_hook(user_t* user,cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	bool res = false;
	// 如果私有数据没有带指针的，则设置数据大小就可以传递给APP
	u_int32_t pure_data_size = 0; 

	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return false;
	}

	if (!(ac = sma->sub_ctrl)) {
		return false;
	}

	if(!is_supported_udp_device(user->sub_type,user->ext_type)
		 || !udp_build_common_info(user,ac,ui)){
		return res;
	}
	
	switch(user->sub_type){
		case IJ_SMART_PLUG:
			pure_data_size = 1;
			break;
		case IJ_822:
			pure_data_size = sizeof(cl_jnb_thermotat_info);
			break;
		case IJ_823:
			pure_data_size = sizeof(cl_yl_thermostat_info);
			break;
		case IJ_824:
			switch(user->ext_type) {
			case ETYPE_IJ_824_HTC_BUSINESS:
			case TYPE_IJ_824_YCJ:
				pure_data_size = sizeof(cl_tbb_info_t);
				break;
			case ETYPE_IJ_824_ZKRSQ:
				pure_data_size = sizeof(cl_zkrsq_info_t);
				break;
			default:
				pure_data_size = sizeof(cl_tb_info_t);
				break;
			}
			break;
		case IJ_830:
			res = _lede_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
		case IJ_840:
			res = jcx_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
		case IJ_AMT:
			res = amt_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
		case IJ_CHIFFO:
			res = chiffo_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
		case IJ_EVM:
			res = evm_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
		case IJ_INDIACAR:
			res = indiacar_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
		case IJ_HXPBJ:
            switch(user->ext_type){
                case ETYPE_IJ_HX_POT:
                    res = hx_ysh_bulid_priv_dev_info(ac, ui->com_udp_info);
                    break;
                case ETYPE_IJ_HX_PBJ:
                    pure_data_size = sizeof(cl_hx_info);
                    break;
                default:
                    break;
            }
			
			break;
		case IJ_TL_TEMP:
			pure_data_size = sizeof(cl_tl_info_t);
			break;
		case IJ_QPCP:
			res = _qpcp_bulid_priv_dev_info(ac,ui->com_udp_info);
			break;
        case IJ_KXM_DEVICE:
			switch(user->ext_type) {
                case ETYPE_IJ_KXM_HOST:
				case EYTYP_IJ_KXM_AC:
                    pure_data_size = sizeof(cl_kxm_info_t);
                    break;
                case ETYPE_IJ_KXM_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT:
				case ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB:
                    pure_data_size = sizeof(cl_kxm_thermost_info_t);
                    break;
                case ETYPE_IJ_XY_THERMOSTAT:
                    pure_data_size = sizeof(cl_xy_info_t);
                    break;
                case ETYPE_IJ_SBT_THER:
                    pure_data_size = sizeof(cl_sbt_ther_info_t);
                    break;
				case ETYPE_IJ_YJ_HEATER:
					pure_data_size = sizeof(cl_yj_heater_info_t);
					break;
				case EYTPE_IJ_ZSSX_FURN:
					res = zssx_priv_dev_info(ac, ui->com_udp_info);
					break;
				case ETYPE_IJ_LINKON_THERMOSTAT:
					pure_data_size = sizeof(cl_linkon_info_t);
					break;
                default:
                    break;
			}
            break;
       
		case IJ_RFGW:
			rfgw_build_objs(user, ui);
			res = true;
			break;
        case IJ_QP_POT:
            switch(user->ext_type){
                case ETYPE_IJ_QP_POT:
                    res = qp_pot_bulid_priv_dev_info(ac, ui->com_udp_info);
                    break;
                case ETYPE_IJ_QP_PBJ:
                    res = qp_pbj_bulid_priv_dev_info(ac, ui->com_udp_info);
                    break;
                default:
                    break;
            }
            break;
		case IJ_TEST_DEV:
			switch(user->ext_type) {
			case ETYPE_IJ_TEST_CAR_WK:
				res = _car_bulid_priv_dev_info(ac, ui->com_udp_info);
				break;
			case ETYPE_IJ_TEST_XY:
				pure_data_size = sizeof(cl_xy_info_t);
				break;
            case ETYPE_IJ_TEST_BITMAR:
                pure_data_size = sizeof(cl_bimar_heater_info_t);
                break;
			default:
				break;
			}
			break;
        case IJ_HEATER_DEV:
            switch (user->ext_type) {
                case ETYPE_IJ_HEATER_BIMAR_C2000:
                case ETYPE_IJ_HEATER_AKM_0505L:
                case ETYPE_IJ_HEATER_AKM_5162L:
                    pure_data_size = sizeof(cl_bimar_heater_info_t);
                    break;
                    
                default:
                    break;
            }
            break;
		case IJ_101:
			pure_data_size = 1;
			break;
        case IJ_101_OEM:
            pure_data_size = sizeof(cl_eplug_oem_stat);
            break;
		case IJ_838_YT:
			pure_data_size = sizeof(cl_yt_info_t);
			break;
		case IJ_JL_STAGE_LAMP:
		switch (user->ext_type) {
                case ETYPE_JL_STAGE_LAMP:
                    pure_data_size = sizeof(cl_jl_lamp_info_t);
                    break;
                    
                default:
                    break;
            }
		break;
		case IJ_839_ADS:
			pure_data_size = sizeof(cl_ads_info_t);
			break;
        case IJ_JS_MICWAVE:
            switch (user->ext_type) {
                case ETYPE_IJ_JS_MICWARE:
                case ETYPE_IJ_JS_MIC_BARBECUE:
                case ETYPE_IJ_JS_ONLY_MIC:
                    pure_data_size = sizeof(cl_js_wave_info_t);
                    break;
                    
                default:
                    break;
            }
            break;
		case IJ_ZHCL:
			pure_data_size = sizeof(cl_zhcl_info_t);
			break;
		case IJ_ZHDHX:
			pure_data_size = sizeof(cl_zhdhx_info_t);
			break;
		case IJ_HOMESERVER:
			pure_data_size = sizeof(u_int8_t);
			break;
		case IJ_LEIS:
			switch (user->ext_type) {
				case ETYPE_LEIS_DEFAULT:				
					pure_data_size = sizeof(cl_leis_info_t);
					break;
				case ETYPE_LEIS_YINSU:
					pure_data_size = sizeof(cl_yinsu_info_t);
					break;
			}
			break;
		default:
			break;
	}
	
	if(pure_data_size > 0){
		res = udp_bulid_pure_priv_dev_info(ac,ui->com_udp_info,pure_data_size);
	}

	return res;
}

void udp_free_objs_hook(struct cl_com_udp_device_data_s* udp_dev_info)
{
	if(!udp_dev_info){
		return;
	}
	SAFE_FREE(udp_dev_info->timer_info.timers);
	SAFE_FREE(udp_dev_info->timer_info.period_timers);
	SAFE_FREE(udp_dev_info->dev_err_info.err_items);
	SAFE_FREE(udp_dev_info->dev_err_info.pushed_err);
	SAFE_FREE(udp_dev_info->comm_timer_head.timer);
	SAFE_FREE(udp_dev_info->timer_summary.stat_count);
	//TODO: 根据类型释放
	switch(udp_dev_info->sub_type){
		case IJ_822:
		case IJ_823:
		case IJ_824:
		case IJ_838_YT:
		case IJ_839_ADS:
		case IJ_ZHCL:
		case IJ_ZHDHX:
		case IJ_HOMESERVER:
			break;
		case IJ_830:
			_free_lede_info(udp_dev_info->device_info);
			break;
		case IJ_840:
			_free_jcx_info(udp_dev_info->device_info);
			break;
        case IJ_HXPBJ:
            switch(udp_dev_info->ext_type){
                case ETYPE_IJ_HX_POT:
                    _free_hx_ysh_info(udp_dev_info->device_info);
                    break;
                default:
                    break;
            }
            break;
		case IJ_QPCP:
			_free_qpcp_info(udp_dev_info->device_info);
			break;
		case IJ_QP_POT:
            switch(udp_dev_info->ext_type){
                case ETYPE_IJ_QP_POT:
                    _free_qp_pot_info(udp_dev_info->device_info);
                    break;
                case ETYPE_IJ_QP_PBJ:
                    _free_qp_pbj_info(udp_dev_info->device_info);
                    break;
                default:
                    break;
            }
			break;
		case IJ_RFGW:
			_cl_rfgw_free(udp_dev_info->device_info);
			break;
		case IJ_TEST_DEV:
			switch(udp_dev_info->ext_type) {
			case ETYPE_IJ_TEST_CAR_WK:
				_free_car_info(udp_dev_info->device_info);
				break;
			case ETYPE_IJ_TEST_XY:
				_free_xy_info(udp_dev_info->device_info);
				break;
			default:
				break;
			}
			break;
		default:
			break;
	}
	udp_dev_info->elec_days_info.is_info_valid = false;
	udp_dev_info->elec_days_info.days_count = 0;
	SAFE_FREE(udp_dev_info->elec_days_info.elec_data);
	SAFE_FREE(udp_dev_info->device_info);

	SAFE_FREE(udp_dev_info->lum_info.record);
}

void udp_free_sdk_priv_data(struct cl_com_udp_device_data_s* udp_dev_info)
{
	if(!udp_dev_info){
		return;
	}
	SAFE_FREE(udp_dev_info->timer_info.timers);
	SAFE_FREE(udp_dev_info->timer_info.period_timers);
	SAFE_FREE(udp_dev_info->dev_err_info.err_items);
	SAFE_FREE(udp_dev_info->dev_err_info.pushed_err);	
	SAFE_FREE(udp_dev_info->comm_timer_head.timer);
	SAFE_FREE(udp_dev_info->timer_summary.stat_count);
	//TODO: 根据类型释放
	switch(udp_dev_info->sub_type){
		case IJ_822:
		case IJ_823:
		case IJ_824:
		case IJ_838_YT:
		case IJ_839_ADS:
		case IJ_ZHCL:
		case IJ_ZHDHX:
		case IJ_HOMESERVER:
			break;
		case IJ_830:
			_free_lede_info(udp_dev_info->device_info);
			break;
		case IJ_840:
			_free_jcx_info(udp_dev_info->device_info);
			break;
		case IJ_QPCP:
			_free_qpcp_sdk_info(udp_dev_info->device_info);
			break;
        case IJ_HXPBJ:
            switch (udp_dev_info->ext_type) {
                case ETYPE_IJ_HX_POT:
                    _free_hx_ysh_sdk_info(udp_dev_info->device_info);
                    break;
                    
                default:
                    break;
            }
            break;
		case IJ_RFGW:
			free_rfgw_sdk(udp_dev_info->device_info);
			break;
        case IJ_QP_POT:
            switch(udp_dev_info->ext_type){
                case ETYPE_IJ_QP_POT:
                   _free_qp_pot_sdk_info(udp_dev_info->device_info);
                    break;
                case ETYPE_IJ_QP_PBJ:
                    _free_qp_pbj_sdk_info(udp_dev_info->device_info);
                    break;
                default:
                    break;
            }
            
            break;
		case IJ_TEST_DEV:
			switch(udp_dev_info->ext_type) {
			case ETYPE_IJ_TEST_CAR_WK:
				_free_car_info(udp_dev_info->device_info);
				break;
			case ETYPE_IJ_TEST_XY:
				_free_xy_info(udp_dev_info->device_info);
				break;
			default:
				break;
			}
			break;
		case IJ_INDIACAR:
			indiacar_free(udp_dev_info->device_info);
			break;
		default:
			break;
	}
	udp_dev_info->elec_days_info.is_info_valid = false;
	udp_dev_info->elec_days_info.days_count = 0;
	SAFE_FREE(udp_dev_info->elec_days_info.elec_data);
	SAFE_FREE(udp_dev_info->device_info);
	
}

int udp_fill_ext_peroid_timer_modify_pkt(smart_air_ctrl_t* ac,cl_period_timer_t* t_hdr,void* src_priv,void* dest)
{
    int len = 0;
    ucp_qp_timer_ext_t* qpe;
    ucp_808_timer_ext_t* t8;
    ucp_101_oem_timer_ext_t* oe;
    ucp_qp_pot_timer_ext_t* qp_pot;
    ucp_qp_pbj_timer_ext_t* qp_pbj;
    ucp_hx_ysh_timer_ext_t* hx_ysh;
    
    if (!ac || !src_priv || !dest || !t_hdr) {
        return RS_ERROR;
    }
    
    switch (t_hdr->ext_data_type) {
        case PT_EXT_DT_QPCP: //千帕茶盘
            qpe = (ucp_qp_timer_ext_t*)dest;
            qpe->id = htons(t_hdr->pt_ext_data_u.qp_time_info.id);
            memcpy(qpe+1, src_priv, sizeof(cl_qpcp_scp_t));
            
            len = sizeof(cl_qpcp_scp_t) + sizeof(*qpe);
            break;
        case PT_EXT_DT_808:
            t8 = (ucp_808_timer_ext_t*)dest;
            t8->fan_dir = t_hdr->pt_ext_data_u.air_timer_info.fan_dir;
            t8->fan_speed = t_hdr->pt_ext_data_u.air_timer_info.fan_speed;
            t8->key_id = t_hdr->pt_ext_data_u.air_timer_info.key_id;
            t8->mode = t_hdr->pt_ext_data_u.air_timer_info.mode;
            t8->onOff = t_hdr->pt_ext_data_u.air_timer_info.onOff;
            t8->temp = t_hdr->pt_ext_data_u.air_timer_info.temp;
            if (t8->temp >= AC_TEMP_BASE && t8->temp != 0xFF) {
                t8->temp -= AC_TEMP_BASE;
            }
            
            len = sizeof(*t8);
            break;
        case PT_EXT_DT_101_OEM:
            oe = (ucp_101_oem_timer_ext_t*)dest;
            oe->min_temp = t_hdr->pt_ext_data_u.oem_101_timer_info.min_temp;
            oe->max_temp = t_hdr->pt_ext_data_u.oem_101_timer_info.max_temp;
            len = sizeof(*oe);
            break;
        case PT_EXT_DT_QP_POT:
            qp_pot = (ucp_qp_pot_timer_ext_t*)dest;
            qp_pot->cook_id = htons(t_hdr->pt_ext_data_u.qp_pot_timer_info.cook_id);
            qp_pot->cook_time = htons(t_hdr->pt_ext_data_u.qp_pot_timer_info.cook_time);
            qp_pot->cooking_mode = t_hdr->pt_ext_data_u.qp_pot_timer_info.cooking_mode;
            qp_pot->hot_degress = t_hdr->pt_ext_data_u.qp_pot_timer_info.hot_degress;
            qp_pot->microswitch = t_hdr->pt_ext_data_u.qp_pot_timer_info.microswitch;
            qp_pot->warm_temp = t_hdr->pt_ext_data_u.qp_pot_timer_info.warm_temp;
            len = sizeof(*qp_pot);
            break;
        case PT_EXT_DT_QP_PBJ:
            qp_pbj = (ucp_qp_pbj_timer_ext_t*)dest;
            qp_pbj->scene_id = htons(t_hdr->pt_ext_data_u.qp_pbj_timer_info.scene_id);
            len = sizeof(*qp_pbj);
            break;
        case PT_EXT_DT_HX_YSH:
            hx_ysh = (ucp_hx_ysh_timer_ext_t*)dest;
            hx_ysh->work_time = t_hdr->pt_ext_data_u.hx_ysh_timer_info.work_time;
            hx_ysh->keep_temp = t_hdr->pt_ext_data_u.hx_ysh_timer_info.keep_temp;
            hx_ysh->keep_time = t_hdr->pt_ext_data_u.hx_ysh_timer_info.keep_time;
            hx_ysh->power = t_hdr->pt_ext_data_u.hx_ysh_timer_info.power;
            hx_ysh->scene_id = htons(t_hdr->pt_ext_data_u.hx_ysh_timer_info.scene_id);
            hx_ysh->temp = t_hdr->pt_ext_data_u.hx_ysh_timer_info.temp;
            len = sizeof(*hx_ysh);
            break;
        default:
            break;
    }
    
    return len;
    
}

void udp_updata_ext_peroid_timer_by_ext_data(smart_air_ctrl_t* ac,void* ext_pkt,int len ,cl_period_timer_t* dest)
{
    ucp_qp_timer_ext_t* qpe;
    ucp_808_timer_ext_t* t8;
    ucp_101_oem_timer_ext_t* oe;
    ucp_qp_pot_timer_ext_t* qp_pot;
    ucp_qp_pbj_timer_ext_t* qp_pbj;
    ucp_hx_ysh_timer_ext_t* hx_ysh;
    
    switch (ac->sac->user->sub_type) {
        case IJ_QPCP:
            if (len >= sizeof(*qpe)) {
                qpe = (ucp_qp_timer_ext_t*)ext_pkt;
                dest->ext_data_type = PT_EXT_DT_QPCP;
                dest->pt_ext_data_u.qp_time_info.id = ntohs(qpe->id);
            }
            break;
        case IJ_808:
            if (len >= sizeof(*t8)) {
                t8 = (ucp_808_timer_ext_t*)ext_pkt;
                dest->ext_data_type = PT_EXT_DT_808;
                dest->pt_ext_data_u.air_timer_info.fan_dir = t8->fan_dir;
                dest->pt_ext_data_u.air_timer_info.fan_speed = t8->fan_speed;
                dest->pt_ext_data_u.air_timer_info.key_id = t8->key_id;
                dest->pt_ext_data_u.air_timer_info.mode = t8->mode;
                dest->pt_ext_data_u.air_timer_info.onOff = t8->onOff;
                if(t8->temp != 0xFF){
                    dest->pt_ext_data_u.air_timer_info.temp = t8->temp+AC_TEMP_BASE;
                    if (dest->pt_ext_data_u.air_timer_info.temp > AC_TEMP_MAX) {
                        dest->pt_ext_data_u.air_timer_info.temp = AC_TEMP_MAX;
                    }
                }else{
                    dest->pt_ext_data_u.air_timer_info.temp = 0xFF;
                }
            }
            break;
        case IJ_101_OEM:
            if (len >= sizeof(*oe)) {
                oe = (ucp_101_oem_timer_ext_t*)ext_pkt;
                dest->ext_data_type = PT_EXT_DT_101_OEM;
                dest->pt_ext_data_u.oem_101_timer_info.min_temp = oe->min_temp;
                dest->pt_ext_data_u.oem_101_timer_info.max_temp = oe->max_temp;
            }
            break;
        case IJ_HXPBJ:
            switch (ac->sac->user->ext_type) {
                case ETYPE_IJ_HX_POT:
                    if (len >= sizeof(*hx_ysh)) {
                        hx_ysh = (ucp_hx_ysh_timer_ext_t*)ext_pkt;
                        dest->ext_data_type = PT_EXT_DT_HX_YSH;
                        dest->pt_ext_data_u.hx_ysh_timer_info.keep_temp = hx_ysh->keep_temp;
                        dest->pt_ext_data_u.hx_ysh_timer_info.keep_time = hx_ysh->keep_time;
                        dest->pt_ext_data_u.hx_ysh_timer_info.power = hx_ysh->power;
                        dest->pt_ext_data_u.hx_ysh_timer_info.scene_id = ntohs(hx_ysh->scene_id);
                        dest->pt_ext_data_u.hx_ysh_timer_info.temp = hx_ysh->temp;
                        dest->pt_ext_data_u.hx_ysh_timer_info.work_time = hx_ysh->work_time;
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        case IJ_QP_POT:
            
            switch (ac->sac->user->ext_type) {
                case ETYPE_IJ_QP_POT:
                    if (len >= sizeof(*qp_pot)) {
                        qp_pot = (ucp_qp_pot_timer_ext_t*)ext_pkt;
                        dest->ext_data_type = PT_EXT_DT_QP_POT;
                        dest->pt_ext_data_u.qp_pot_timer_info.cook_id = ntohs(qp_pot->cook_id);
                        dest->pt_ext_data_u.qp_pot_timer_info.cook_time = ntohs(qp_pot->cook_time);
                        dest->pt_ext_data_u.qp_pot_timer_info.cooking_mode = qp_pot->cooking_mode;
                        dest->pt_ext_data_u.qp_pot_timer_info.hot_degress = qp_pot->hot_degress;
                        dest->pt_ext_data_u.qp_pot_timer_info.microswitch = qp_pot->hot_degress;
                        dest->pt_ext_data_u.qp_pot_timer_info.warm_temp = qp_pot->warm_temp;
                    }
                    break;
                case ETYPE_IJ_QP_PBJ:
                    if (len >= sizeof(*qp_pbj)) {
                        qp_pbj = (ucp_qp_pbj_timer_ext_t*)ext_pkt;
                        dest->ext_data_type = PT_EXT_DT_QP_PBJ;
                        dest->pt_ext_data_u.qp_pbj_timer_info.scene_id = ntohs(qp_pbj->scene_id);
                    }
                    break;
                    
                default:
                    break;
            }
            
            break;
        default:
            break;
    }
}

void comm_timer_utc_2_local(cl_comm_timer_t *ptimer)
{
 #ifdef USE_TIME_MINS	
	int time_min = 0;
	u_int8_t tmp = 0;

	time_min = ptimer->hour*60 + ptimer->min + cl_priv->time_diff + 24*60;
	tmp = ptimer->week&BIT(7);
	
	if (time_min < 24*60) {
		ptimer->week = timer_week_right_shift(ptimer->week);
	} else if (time_min >= 48*60) {
		ptimer->week = timer_week_left_shift(ptimer->week);
	}
	ptimer->hour = (time_min/60)%24;
	ptimer->min = time_min%60;
	ptimer->week |= tmp;
#else
	int hour;
	u_int8_t tmp = 0;

	hour = ptimer->hour + 24 + cl_priv->timezone;
	tmp = ptimer->week&BIT(7);
	
	if (hour < 24) {
		ptimer->week = timer_week_right_shift(ptimer->week&(~(BIT(7))));
	} else if (hour >= 48) {
		ptimer->week = timer_week_left_shift(ptimer->week&(~(BIT(7))));
	}
	ptimer->hour = hour%24;
	ptimer->week |= tmp;
#endif	
}

void comm_timer_local_2_utc(cl_comm_timer_t *ptimer)
{
 #ifdef USE_TIME_MINS	
	int time_min = 0;
	u_int8_t tmp = 0;

	time_min = ptimer->hour*60 + ptimer->min - cl_priv->time_diff + 24*60;
	tmp = ptimer->week&BIT(7);
	
	if (time_min < 24*60) {
		ptimer->week = timer_week_right_shift(ptimer->week);
	} else if (time_min >= 48*60) {
		ptimer->week = timer_week_left_shift(ptimer->week);
	}
	ptimer->hour = (time_min/60)%24;
	ptimer->min = time_min%60;
	ptimer->week |= tmp;
#else
	int hour;
	u_int8_t tmp = 0;

	hour = ptimer->hour + 24 - cl_priv->timezone;
	tmp = ptimer->week&BIT(7);
	
	if (hour < 24) {
		ptimer->week = timer_week_right_shift(ptimer->week&(~(BIT(7))));
	} else if (hour >= 48) {
		ptimer->week = timer_week_left_shift(ptimer->week&(~(BIT(7))));
	}
	ptimer->hour = hour%24;
	ptimer->week |= tmp;
#endif	
}

void _ucp_comm_update_timer_onoff(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	u_int8_t *pdata = (u_int8_t *)&ptimer[1];
	
	if (ptimer->extened_len == 0) {
		pctimer->type = 0;
		return;
	}

	switch(ptimer->type) {
	case UT_DEV_TYPE_ONOFF:
		if (*pdata == 1) {
			if (ptimer->duration) {
				pctimer->type = UT_TYPE_PERIOD_ONOFF;
			} else {
				pctimer->type = UT_TYPE_ON;
			}
		} else if (*pdata == 0){
			pctimer->type = UT_TYPE_OFF;
		}
		break;
	default:
		pctimer->type = 0;
		break;
	}
}

static void ucp_comm_time_update(u_int32_t start_time, u_int8_t *hour, u_int8_t *min)
{
	struct tm l_tm;
	time_t time_sec = 0;

	time_sec = start_time *60;
	gmtime_r(&time_sec, &l_tm);

	*hour = l_tm.tm_hour;
	*min = l_tm.tm_min;
}

static void ucp_update_comm_type_pkt(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	ucp_comm_advance_timer_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
		break;
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_ADVANCE_TIMER:
		if (ptimer->extened_len < sizeof(*padvance)) {
			log_debug("type=%u errlen=%u\n", ptimer->type, ptimer->extened_len);
			break;
		}
		padvance = (ucp_comm_advance_timer_t *)ptimer->data;
		pctimer->extended_data_u.zykt_timer.mode = padvance->mode;
		pctimer->extended_data_u.zykt_timer.tmp = (int8_t)padvance->tmp;
		break;
	default:
		break;
	}
}

static void ucp_update_linkon_pkt(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	ucp_linkon_advance_timer_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
		break;
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_ADVANCE_TIMER:
		if (ptimer->extened_len < sizeof(*padvance)) {
			log_debug("type=%u errlen=%u\n", ptimer->type, ptimer->extened_len);
			break;
		}
		padvance = (ucp_linkon_advance_timer_t *)ptimer->data;
		pctimer->extended_data_u.linkon_timer.run_mode = padvance->run_mode;
		pctimer->extended_data_u.linkon_timer.wind_speed = padvance->wind_speed;
		pctimer->extended_data_u.linkon_timer.tmp = htons(padvance->tmp);
		pctimer->extended_data_u.linkon_timer.scene_mode = padvance->scene_mode;
		break;
	default:
		break;
	}
}


static void ucp_update_comm_wkair_pkt(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	ucp_comm_wkair_ext_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TIMER_WKAIR_ON:
	case UT_TIMER_WKAIR_OFF:
	case UT_TIMER_WKAIR_DURATION:
		break;
	case UT_TIMER_WKAIR_ADVANCE_1:
	case UT_TIMER_WKAIR_ADVANCE_2:
		if (ptimer->extened_len < sizeof(*padvance)) {
			log_debug("type=%u errlen=%u\n", ptimer->type, ptimer->extened_len);
			break;
		}
		padvance = (ucp_comm_wkair_ext_t *)ptimer->data;
		pctimer->extended_data_u.wkair_timer.tmp = (int8_t)padvance->tmp;
		pctimer->extended_data_u.wkair_timer.mode = (int8_t)padvance->mode;
		break;
	default:
		break;
	}
}

static void ucp_update_comm_ht_pkt(slave_t *slave, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	u_int8_t *pext_data = ptimer->data;

	switch(ptimer->type) {
	case UT_TIMER_HT_ONOFF:
		pctimer->extended_data_u.hv_timer.tmp_int = pext_data[0];
		pctimer->extended_data_u.hv_timer.tmp_dec = pext_data[1];
		break;
	case UT_TIMER_HT_DURATION:
		pctimer->extended_data_u.hv_timer.start_tmp_int = pext_data[0];
		pctimer->extended_data_u.hv_timer.start_tmp_dec = pext_data[1];
		pctimer->extended_data_u.hv_timer.end_tmp_int = pext_data[2];
		pctimer->extended_data_u.hv_timer.end_tmp_dec = pext_data[3];
		break;
	case UT_TIMER_HT_CONST_TMP:
#if 1	
		pctimer->extended_data_u.hv_timer.max_tmp_int = pext_data[0];
		pctimer->extended_data_u.hv_timer.max_tmp_dec = pext_data[1];
		pctimer->extended_data_u.hv_timer.min_tmp_int = pext_data[2];
		pctimer->extended_data_u.hv_timer.min_tmp_dec = pext_data[3];
#else
		slave->max_tmp_int = pext_data[0];
		slave->max_tmp_dec = pext_data[1];
		slave->min_tmp_int = pext_data[2];
		slave->min_tmp_dec = pext_data[3];
#endif
		break;
	default:
		break;
	}
}

static void ucp_update_comm_zhdj_pkt(slave_t *slave, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	switch(ptimer->type) {
	case UT_TYPE_ON:
		if (ptimer->data[0] > 100) {
			ptimer->data[0] = 100;
		}
		pctimer->extended_data_u.zhdj_timer.location = 100 - ptimer->data[0];
		break;
	default:
		break;
	}
}

static void ucp_update_comm_dhxml_pkt(slave_t *slave, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	ucp_dhxml_advance_timer_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TIMER_WKAIR_ON:
	case UT_TIMER_WKAIR_OFF:
	case UT_TIMER_WKAIR_DURATION:
		break;
	case UT_TIMER_WKAIR_ADVANCE_1:
	case UT_TIMER_WKAIR_ADVANCE_2:
		if (ptimer->extened_len < sizeof(*padvance)) {
			log_debug("type=%u errlen=%u\n", ptimer->type, ptimer->extened_len);
			break;
		}
		padvance = (ucp_dhxml_advance_timer_t *)ptimer->data;
		pctimer->extended_data_u.dhxml_timer.on_off_stat = htonl(padvance->on_off_stat);
		break;
	default:
		break;
	}
}

static void ucp_update_comm_dhxml_lhx_pkt(slave_t *slave, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	ucp_dhxml_advance_timer_2_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TIMER_WKAIR_ON:
	case UT_TIMER_WKAIR_OFF:
	case UT_TIMER_WKAIR_DURATION:
	case UT_TIMER_WKAIR_ADVANCE_1:
	case UT_TIMER_WKAIR_ADVANCE_2:
		if (ptimer->extened_len < sizeof(*padvance)) {
			log_debug("type=%u errlen=%u\n", ptimer->type, ptimer->extened_len);
			break;
		}
		padvance = (ucp_dhxml_advance_timer_2_t *)ptimer->data;
		pctimer->extended_data_u.dhxml_timer.on_off_stat = htons(padvance->on_off_stat);
		break;
	default:
		break;
	}
}

static void ucp_comm_update_timer_modify_slave_pkt(slave_t *slave, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	switch(slave->ext_type) {
    case RF_EXT_TYPE_LIGHT:
    case RF_EXT_TYPE_LED_LAMP:
	case RF_EXT_TYPE_DWHF:
	case RF_EXT_TYPE_DWYKHF:
	case RF_EXT_TYPE_DWYSTGQ:
    case RF_EXT_TYPE_DOOR_LOCK:
    case RF_EXT_TYPE_DOOR_MAGNET:
	case RF_EXT_TYPE_DOOR_MAGNETV2:
    case RF_EXT_TYPE_DHX:
    case RF_EXT_TYPE_YT_DOOR_LOCK:
    case RF_EXT_TYPE_HM_MAGENT:
    case RF_EXT_TYPE_HM_ENV_DETECT:
    case RF_EXT_TYPE_HM_BODY_DETECT:
	case RF_EXT_TYPE_WUANS6:
	case RF_EXT_TYPE_KTCZ:
	case RF_EXT_TYPE_HTLLOCK:
	case RF_EXT_TYPE_GAS:
	case RF_EXT_TYPE_QSJC:
	case RF_EXT_TYPE_HMCO:
	case RF_EXT_TYPE_HMYW:
	case RF_EXT_TYPE_HMQJ:
	case RF_EXT_TYPE_YLLOCK:
	case RF_EXT_TYPE_YLTC:
		//暂时slave和master处理都一样，后面需要自己不同处理
		ucp_update_comm_type_pkt(ptimer, pctimer);
		break;
    case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_WK_AIR2:
		ucp_update_comm_wkair_pkt(ptimer, pctimer);		
		break;
    case RF_EXT_TYPE_HEATING_VALVE:
		ucp_update_comm_ht_pkt(slave, ptimer, pctimer);		
		break;
	case RS_EXT_TYPE_ZHDJ:
		ucp_update_comm_zhdj_pkt(slave, ptimer, pctimer);		
		break;
	case RF_EXT_TYPE_DHXML:
	case RF_EXT_TYPE_DHXZH:
	case RF_EXT_TYPE_DHXCP:
	case RF_EXT_TYPE_LHX:
		ucp_update_comm_dhxml_pkt(slave, ptimer, pctimer);		
		break;
	default:
		//暂时slave和master处理都一样，后面需要自己不同处理
		ucp_update_comm_type_pkt(ptimer, pctimer);
		break;
	}
}

static void ucp_update_comm_zhcl_pkt(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	switch(ptimer->type) {
	case UT_TYPE_ON:
		if (ptimer->data[0] > 100) {
			ptimer->data[0] = 100;
		}
		pctimer->extended_data_u.zhdj_timer.location = ptimer->data[0];
		break;
	default:
		break;
	}
}
static void ucp_update_comm_zhdhx_pkt(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	switch(ptimer->type) {
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		pctimer->extended_data_u.zhdhx_timer.onoff = ptimer->data[0];
		pctimer->extended_data_u.zhdhx_timer.mask = ptimer->data[1];
		break;
	default:
		break;
	}
}


static void ucp_comm_update_timer_modify_master_pkt(user_t *user, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{
	switch(user->sub_type) {
	case IJ_ZHCL:
		ucp_update_comm_zhcl_pkt(ptimer, pctimer);
		break;
	case IJ_KXM_DEVICE:
		if (user->ext_type == ETYPE_IJ_LINKON_THERMOSTAT) {
			ucp_update_linkon_pkt(ptimer, pctimer);
			break;
		}
	case IJ_ZHDHX:
		ucp_update_comm_zhdhx_pkt(ptimer, pctimer);
		break;
	default:
		//暂时slave和master处理都一样，后面需要自己不同处理
		ucp_update_comm_type_pkt(ptimer, pctimer);
		break;
	}
}

static bool ucp_short_timer_proc(user_t *user, u_int64_t slave_sn, ucp_comm_timer_t *ptimer)
{
	slave_t *slave;
	ucp_comm_wkair_ext_t *padvance = (ucp_comm_wkair_ext_t *)ptimer->data;

	slave = _find_slave_at_list(slave_sn, &user->slave);
	if (!slave) {
		return false;
	}

	switch(slave->ext_type) {
	case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_WK_AIR2:
		break;
	default:
		return false;
		break;
	}

	slave->shortcuts_onoff.enable = ptimer->enable;
	slave->shortcuts_onoff.onoff = padvance->onoff;
	slave->shortcuts_onoff.remain_time = htons(ptimer->duration);
#ifdef USE_TIME_MINS
	slave->shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->time_diff * 60;
#else
	slave->shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->timezone * 3600;
#endif
	log_debug("sn=%"PRIu64" get shortcutse=%u o=%u t=%u rt=%u pso->remain_time=%u\n", 
		slave->sn, slave->shortcuts_onoff.enable, slave->shortcuts_onoff.onoff, 
		slave->shortcuts_onoff.time, slave->shortcuts_onoff.remain_time, slave->shortcuts_onoff.remain_time);

	return true;
}

void ucp_comm_update_timer_modify_pkt(user_t *user, bool is_slave, u_int64_t slave_sn, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer)
{	
	slave_t *slave;
	
	if (!user || !pctimer || !ptimer) {
		return ;
	}

#if 0
	// TODO:空调贴rf设备端代码空间紧张，要求用通用定时器结构来承载快捷定时器
	if ((ptimer->start_time == 0) && is_slave) {
		if (!ucp_short_timer_proc(user ,slave_sn, ptimer)) {
			log_debug("ucp_short_timer_proc failed\n");
		}
		
		return;
	}
#endif	

	pctimer->id = ptimer->id;
	pctimer->type = ptimer->type;
	pctimer->enable = ptimer->enable;
	pctimer->week = ptimer->week;
	ptimer->start_time = htonl(ptimer->start_time);
	pctimer->duration = htons(ptimer->duration);
	pctimer->extened_len = ptimer->extened_len;
	pctimer->valid = 1;
	ucp_comm_time_update(ptimer->start_time, &pctimer->hour, &pctimer->min);
	//comm_timer_utc_2_local_weekonly(pctimer);
	comm_timer_utc_2_local(pctimer);
	log_debug("recv id=%u ptimer->start_time=%u pctimer->hour=%u pctimer->min=%u pctimer->week=%02x\n", 
		ptimer->id, ptimer->start_time, pctimer->hour, pctimer->min, pctimer->week);
	pctimer->week_cal = pctimer->week;
	if (pctimer->week&BIT(7)) {
		pctimer->week &= ~(BIT(7));
	} else {
		pctimer->week = 0;
	}

	log_debug("recv id=%u week=%02x\n", pctimer->id, pctimer->week);
	if (is_slave) {
		slave = _find_slave_at_list(slave_sn, &user->slave);
		if (!slave) {
			return;
		}
		ucp_comm_update_timer_modify_slave_pkt(slave, ptimer, pctimer);
	} else {
		ucp_comm_update_timer_modify_master_pkt(user, ptimer, pctimer);
	}
}

static u_int8_t ucp_comm_get_day(u_int8_t week)
{
	int i;

	for(i = 0; i < 7; i++) {
		if (week&BIT(i)) {
			return i;
		}
	}

	return 0;
}

static u_int32_t ucp_comm_time_fill(cl_comm_timer_t *ptimer)
{
	time_t now;
	int start_time;
	struct tm l_tm;
	u_int8_t dst_day;

	time(&now);
	log_debug("now=%u\n", now);
	gmtime_r(&now, &l_tm);

	log_debug("111 l_tm.tm_wday=%u week=%02x l_tm.tm_hour=%u l_tm.tm_min=%u l_tm.tm_mday=%u\n", 
		l_tm.tm_wday, ptimer->week, l_tm.tm_hour, l_tm.tm_min, l_tm.tm_mday);	
	
	l_tm.tm_hour = ptimer->hour;
	l_tm.tm_min = ptimer->min;

	start_time = (int)mktime(&l_tm);

	dst_day = ucp_comm_get_day(ptimer->week);
	if (l_tm.tm_wday < dst_day) {
		start_time += (int)(dst_day - l_tm.tm_wday)*ONE_DAY_SECOND;
	} else {
		start_time -= (int)(l_tm.tm_wday - dst_day)*ONE_DAY_SECOND;
	}

	start_time +=  cl_priv->time_diff*60;
	
	return ((u_int32_t)start_time)/60;
}

u_int8_t timer_add_next_day(cl_comm_timer_t *ptimer)
{
	time_t now;
	struct tm l_tm;
	u_int32_t time1;
	u_int32_t time2;
	u_int8_t week = 0;

	time(&now);
	localtime_r(&now, &l_tm);

	time1 = ptimer->hour*60 + ptimer->min;
	time2 = l_tm.tm_hour*60 + l_tm.tm_min;

	if (time1 > time2) {
		week |= BIT(l_tm.tm_wday);
	} else {
		week |= BIT(((l_tm.tm_wday+1)%7));
	}

	return week;
}

static int ucp_fill_timer_modify_comm_type_pkt(ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	ucp_comm_advance_timer_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_ADVANCE_TIMER:
		puct->extened_len = sizeof(*padvance);
		padvance = (ucp_comm_advance_timer_t *)puct->data;
		padvance->mode = ptimer->extended_data_u.zykt_timer.mode;
		padvance->tmp = (u_int8_t)ptimer->extended_data_u.zykt_timer.tmp;
		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
		puct->extened_len = 0;
		len = sizeof(*puct);
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	return len;
}

static int ucp_fill_timer_modify_linkon_pkt(ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	ucp_linkon_advance_timer_t *padvance = NULL;

	switch(ptimer->type) {
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_ADVANCE_TIMER:
		puct->extened_len = sizeof(*padvance);
		padvance = (ucp_linkon_advance_timer_t *)puct->data;
		padvance->run_mode = ptimer->extended_data_u.linkon_timer.run_mode;
		padvance->wind_speed = ptimer->extended_data_u.linkon_timer.wind_speed;
		padvance->tmp = htons(ptimer->extended_data_u.linkon_timer.tmp);
		padvance->scene_mode = ptimer->extended_data_u.linkon_timer.scene_mode;
		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
		puct->extened_len = 0;
		len = sizeof(*puct);
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	return len;
}


static int ucp_fill_timer_modify_comm_wkair_pkt(slave_t *slave,ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	priv_air_ir_stat_cache* cache;
	ucp_comm_wkair_ext_t *padvance = NULL;
	u_int8_t buf[1024];
	cl_wk_air_work_stat_t *wk_air = &slave->dev_info.rf_stat.dev_priv_data.wk_air_info.stat;

	cache = &slave->match_mana.ir_cache;
    if(!cache->is_ir_info_valid){
        log_debug("cache->is_ir_info_valid is false\n");
		return 0;
    }
	slave_cache_back(cache);
	padvance = (ucp_comm_wkair_ext_t *)puct->data;
	
	padvance->mode = wk_air->mode;
	padvance->tmp = wk_air->temp;
	padvance->win = wk_air->wind;
	padvance->dir = wk_air->wind_direct;
	padvance->key = AC_KEY_POWER;
	
	cache->mode = padvance->mode;
	cache->temp = padvance->tmp;
	cache->wind = padvance->win;
	cache->wind_direct = padvance->dir;
	cache->key_id = padvance->key;
	
	switch(ptimer->type) {
	case UT_TIMER_WKAIR_ON:
		cache->onoff = padvance->onoff = 1;
	    if (!air_ir_get_ir_detail_code(slave, buf, &len)||len <= 0) {
			log_debug("get ir code failed\n");
			len = 0;
	        goto done;
	    }	
		puct->extened_len = (u_int8_t)(sizeof(*padvance) + len);
		memcpy((void *)padvance->code, buf, len);
		
		len = sizeof(*puct) + puct->extened_len;
		//test
		//len = sizeof(*puct) + puct->extened_len + 30;
		break;
	case UT_TIMER_WKAIR_OFF:
		cache->onoff = padvance->onoff = 0;
	    if (!air_ir_get_ir_detail_code(slave, buf, &len)||len <= 0) {
			log_debug("get ir code failed\n");
			len = 0;
	        goto done;
	    }	
		puct->extened_len = (u_int8_t)(sizeof(*padvance) + len);
		memcpy((void *)padvance->code, buf, len);
		
		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TIMER_WKAIR_DURATION:
		padvance->onoff = cache->onoff = 1;
	    if (!air_ir_get_ir_detail_code(slave, buf, &len)||len <= 0) {
			log_debug("get ir code failed\n");
			len = 0;
	        goto done;
	    }	
		puct->extened_len = (u_int8_t)(sizeof(*padvance) + len);
		memcpy((void *)padvance->code, buf, len);

		cache->onoff = !cache->onoff;
	    if (!air_ir_get_ir_detail_code(slave, buf, &len)||len <= 0) {
			log_debug("get ir code failed\n");
			len = 0;
	        goto done;
	    }
		
		puct->extened_len += len;
		memcpy((void *)&padvance->code[len], buf, len);
		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TIMER_WKAIR_ADVANCE_1:
	case UT_TIMER_WKAIR_ADVANCE_2:
		padvance->mode = ptimer->extended_data_u.wkair_timer.mode;
		padvance->tmp = (u_int8_t)ptimer->extended_data_u.wkair_timer.tmp;
	    if (padvance->tmp > 32) {
	        padvance->tmp = 32;
	    }
	    if (padvance->tmp < 16) {
	        padvance->tmp = 16;
	    }
		cache->mode = padvance->mode;
		cache->temp = padvance->tmp;
		padvance->onoff = cache->onoff = 1;
	    if (!air_ir_get_ir_detail_code(slave, buf, &len)||len <= 0) {
			log_debug("get ir code failed\n");
			len = 0;
	        goto done;
	    }	
		puct->extened_len = (u_int8_t)(sizeof(*padvance) + len);
		memcpy((void *)padvance->code, buf, len);

		cache->onoff = !cache->onoff;
	    if (!air_ir_get_ir_detail_code(slave, buf, &len)||len <= 0) {
			log_debug("get ir code failed\n");
			len = 0;
	        goto done;
	    }
		
		puct->extened_len += len;
		memcpy((void *)&padvance->code[len], buf, len);
		len = sizeof(*puct) + puct->extened_len;
		log_debug("ucp_fill_timer_modify_comm_wkair_pkt extlen=%u len=%u\n", puct->extened_len, len);
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

done:
	slave_cache_revert(cache);
	return len;
}

static int ucp_fill_timer_modify_comm_ht_pkt(slave_t *slave, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	u_int8_t *pext_data = NULL;

	switch(ptimer->type) {
	case UT_TIMER_HT_ONOFF:
		puct->extened_len = 2;
		pext_data = puct->data;
		
		pext_data[0] = ptimer->extended_data_u.hv_timer.tmp_int;
		pext_data[1] = ptimer->extended_data_u.hv_timer.tmp_dec;
			
		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TIMER_HT_DURATION:
		puct->extened_len = 4;
		pext_data = puct->data;

		pext_data[0] = ptimer->extended_data_u.hv_timer.start_tmp_int;
		pext_data[1] = ptimer->extended_data_u.hv_timer.start_tmp_dec;
		pext_data[2] = ptimer->extended_data_u.hv_timer.end_tmp_int;
		pext_data[3] = ptimer->extended_data_u.hv_timer.end_tmp_dec;

		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TIMER_HT_CONST_TMP:
		puct->extened_len = 4;
		pext_data = puct->data;

		pext_data[0] = ptimer->extended_data_u.hv_timer.max_tmp_int;
		pext_data[1] = ptimer->extended_data_u.hv_timer.max_tmp_dec;
		pext_data[2] = ptimer->extended_data_u.hv_timer.min_tmp_int;
		pext_data[3] = ptimer->extended_data_u.hv_timer.min_tmp_dec;

		len = sizeof(*puct) + puct->extened_len;
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	log_debug("ucp_fill_timer_modify_comm_ht_pkt timer set len=%u\n", len);

	return len;
}

static int ucp_fill_timer_modify_comm_zhdj_pkt(slave_t *slave, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	u_int8_t *pext_data = NULL;
	u_int8_t pos = 0;

	switch(ptimer->type) {
	case UT_TYPE_ON:
		pos = ptimer->extended_data_u.zhdj_timer.location;
		if (pos > 100) {
			log_debug("err location=%u\n", pos);
			len = 0;
			break;
		}
		pos = 100 - pos;
		
		puct->extened_len = 1;
		puct->data[0] = pos;
			
		len = sizeof(*puct) + puct->extened_len;
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	log_debug("ucp_fill_timer_modify_comm_zhdj_pkt timer set len=%u\n", len);

	return len;
}

static int ucp_fill_timer_modify_comm_dhxml_pkt(slave_t *slave, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	ucp_dhxml_advance_timer_t *padvance = NULL;
	cl_dhx_switch_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);

	if (!info->support_time) {
		log_info("sn=%"PRIu64" not support timer\n", slave->sn);
		return 0;
	}
	switch(ptimer->type) {
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_ADVANCE_TIMER:
		puct->extened_len = sizeof(*padvance);
		padvance = (ucp_dhxml_advance_timer_t *)puct->data;
		padvance->on_off_stat = htonl(ptimer->extended_data_u.dhxml_timer.on_off_stat);
		len = sizeof(*puct) + puct->extened_len;
		break;
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
		puct->extened_len = 0;
		len = sizeof(*puct);
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	return len;
}

static int ucp_fill_timer_modify_comm_dhxml_lhx_pkt(slave_t *slave, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	ucp_dhxml_advance_timer_2_t *padvance = NULL;
	cl_dhx_switch_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);

	if (!info->support_time) {
		log_info("sn=%"PRIu64" not support timer\n", slave->sn);
		return 0;
	}
	switch(ptimer->type) {
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
		puct->extened_len = sizeof(*padvance);
		padvance = (ucp_dhxml_advance_timer_2_t *)puct->data;
		padvance->on_off_stat = htons(ptimer->extended_data_u.dhxml_timer.on_off_stat);
		len = sizeof(*puct) + puct->extened_len;
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	return len;
}

static int _ucp_comm_fill_timer_modify_slave_pkt(slave_t *slave, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;

	log_debug("_ucp_comm_fill_timer_modify_slave_pkt slave->ext_type=%u\n", slave->ext_type);
	
	switch(slave->ext_type) {
    case RF_EXT_TYPE_LIGHT:
    case RF_EXT_TYPE_LED_LAMP:
	case RF_EXT_TYPE_DWHF:
	case RF_EXT_TYPE_DWYKHF:
	case RF_EXT_TYPE_DWYSTGQ:
    case RF_EXT_TYPE_DOOR_LOCK:
    case RF_EXT_TYPE_DOOR_MAGNET:
	case RF_EXT_TYPE_DOOR_MAGNETV2:
    case RF_EXT_TYPE_DHX:
    case RF_EXT_TYPE_YT_DOOR_LOCK:
    case RF_EXT_TYPE_HM_MAGENT:
    case RF_EXT_TYPE_HM_ENV_DETECT:
    case RF_EXT_TYPE_HM_BODY_DETECT:
	case RF_EXT_TYPE_WUANS6:
	case RF_EXT_TYPE_KTCZ:
	case RF_EXT_TYPE_HTLLOCK:
	case RF_EXT_TYPE_GAS:
	case RF_EXT_TYPE_QSJC:
	case RF_EXT_TYPE_HMCO:
	case RF_EXT_TYPE_HMYW:
	case RF_EXT_TYPE_YLLOCK:
	case RF_EXT_TYPE_YLTC:
		//暂时slave和master处理都一样，后面需要自己不同处理
		len = ucp_fill_timer_modify_comm_type_pkt(puct, ptimer);
		break;
    case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_WK_AIR2:
		len = ucp_fill_timer_modify_comm_wkair_pkt(slave, puct, ptimer);
		break;
    case RF_EXT_TYPE_HEATING_VALVE:
		len = ucp_fill_timer_modify_comm_ht_pkt(slave, puct, ptimer);
		break;
	case RS_EXT_TYPE_ZHDJ:
		len = ucp_fill_timer_modify_comm_zhdj_pkt(slave, puct, ptimer);
		break;
	case RF_EXT_TYPE_DHXZH:
	case RF_EXT_TYPE_DHXCP:
	case RF_EXT_TYPE_DHXML:
	case RF_EXT_TYPE_LHX:
		len = ucp_fill_timer_modify_comm_dhxml_pkt(slave, puct, ptimer);
		break;
	default:
		//暂时slave和master处理都一样，后面需要自己不同处理
		len = ucp_fill_timer_modify_comm_type_pkt(puct, ptimer);
		break;
	}
	
	return len;
}

static int ucp_fill_timer_modify_comm_zhcl_pkt(ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	u_int8_t *pext_data = NULL;
	u_int8_t pos = 0;

	switch(ptimer->type) {
	case UT_TYPE_ON:
		pos = ptimer->extended_data_u.zhdj_timer.location;
		if (pos > 100) {
			log_debug("err location=%u\n", pos);
			len = 0;
			break;
		}
		puct->extened_len = 1;
		puct->data[0] = pos;
			
		len = sizeof(*puct) + puct->extened_len;
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	log_debug("ucp_fill_timer_modify_comm_zhcl_pkt timer set len=%u\n", len);

	return len;
}

static int ucp_fill_timer_modify_comm_zhdhx_pkt(ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	u_int8_t *pext_data = NULL;
	u_int8_t pos = 0;

	switch(ptimer->type) {
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		puct->extened_len = 2;
		puct->data[0] = ptimer->extended_data_u.zhdhx_timer.onoff;
		puct->data[1] = ptimer->extended_data_u.zhdhx_timer.mask;
		len = sizeof(*puct) + puct->extened_len;
		break;
	default:
		puct->extened_len = 0;
		len = 0;
		break;
	}

	log_debug("%s timer set len=%u\n", __FUNCTION__, len);

	return len;
}


static int _ucp_comm_fill_timer_modify_master_pkt(user_t *user, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	
	switch(user->sub_type) {
	case IJ_ZHCL:
		len = ucp_fill_timer_modify_comm_zhcl_pkt(puct, ptimer);
		break;
	case IJ_KXM_DEVICE:
		if (user->ext_type == ETYPE_IJ_LINKON_THERMOSTAT) {
			len = ucp_fill_timer_modify_linkon_pkt(puct, ptimer);
			break;
		}
	case IJ_ZHDHX:
		len = ucp_fill_timer_modify_comm_zhdhx_pkt(puct, ptimer);
		break;
	default:
		//暂时slave和master处理都一样，后面需要自己不同处理
		len = ucp_fill_timer_modify_comm_type_pkt(puct, ptimer);
		break;
	}
	
	return len;
}

int ucp_comm_fill_timer_modify_pkt(user_t *user, bool is_slave, u_int64_t slave_sn, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	u_int8_t *pdata = (u_int8_t *)&puct[1];
	u_int32_t time_send;
	slave_t *slave = NULL;
	
	if (!user || !puct || !ptimer) {
		return 0;
	}

	//这里专门处理下，rf定时器需要
	if (ptimer->week) {
		ptimer->week |= BIT(7);
	} else {
	//如果week==0，赋值
		ptimer->week = timer_add_next_day(ptimer);
	}

	comm_timer_local_2_utc(ptimer);
	puct->id = ptimer->id;
	puct->type = ptimer->type;
	puct->enable = ptimer->enable;
	puct->duration = htons(ptimer->duration);
	time_send = ucp_comm_time_fill(ptimer);
	puct->start_time = htonl(time_send);
	
	//comm_timer_local_2_utc(ptimer);
	log_debug("time_send=%u ptimer->week=%02x\n", time_send, ptimer->week);
	puct->week = ptimer->week;

	if (is_slave) {
		slave = _find_slave_at_list(slave_sn, &user->slave);
		if (!slave) {
			log_debug("not found the slave !!!!!!!!!!!!!!!!!!!!\n");
			return 0;
		}
		return _ucp_comm_fill_timer_modify_slave_pkt(slave, puct, ptimer);
	} else {
		return _ucp_comm_fill_timer_modify_master_pkt(user, puct, ptimer);
	}

	return 0;
}


