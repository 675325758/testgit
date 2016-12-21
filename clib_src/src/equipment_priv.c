/*
	远程控制: 红外、无线
*/

#include "client_lib.h"
#include "cl_priv.h"
#include "equipment_priv.h"
#include "wait_server.h"
#include "cl_notify_def.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "cloud_ac_priv.h"

/*用于按键列表更新*/
typedef struct {
    equipment_ctrl_t *ec;
    u_int16_t local_id;
}key_timer_param_t;

#define ALARM_ENABLED (0x1)
#define ALARM_DISABLED (0x2)
#define	ALARM_PUSH	(0x4)

enum{
    KL_STAT_INIT = 0x0, /*初始化,上层未设置学习参数或停止学习*/
    KL_STAT_READY,              /*准备就绪,出错或完成后回到该状态*/
    KL_STAT_WAIT_LEARN_RES,     /*等待开始学习报文结果*/
    KL_STAT_WAIT_LEARN_USER_INPUT, /*等待用户按下学习的按钮,30秒倒计时*/
    KL_STAT_WAIT_TRY_USER_INPUT,       /*等待尝试结果,倒计时*/
    KL_STAT_WAIT_TRY_PACKET,
    KL_STAT_WAIT_GEN_RES,
    KL_STAT_WAIT_STOP_RES,
    KL_STAT_WAIT_AJUST_RES,
    KL_STAT_WAIT_PLW_RES,
    KL_STAT_WAIT_SAVE,
};

#define KL_STAT_EQ(ustat,stat) ((ustat)==(stat))

/******************************************************************/
#define KLC_LEARN_CODE   BIT(0)
#define KLC_AJUST_CODE   BIT(1)
#define KLC_GEN_CODE     BIT(2)
#define KLC_BASE_CODE    BIT(3)
/*学习超时时间*/
#define LEARNING_TIME_OUT (30)

static void kl_reset_timer(user_t* user,u_int16_t time_out);
static int timer_key_query(cl_thread_t *t);
static void ekey_do_query_immediately(equipment_t* eq);
static void do_alarm_phone_quick_query(equipment_t* eq);
static void alarm_phone_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
static void alarm_phone_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
static u_int16_t get_rf_2_state(equipment_t* eq);
static void eq_proc_bind_soundlight(cl_notify_pkt_t *pkt, RS *ret);
static RS eq_linkage_scene(cl_notify_pkt_t *pkt, RS *ret);

/*内存释放函数放顶部了*/
static remote_key_learn_t* _key_learn_alloc(void)
{
    remote_key_learn_t* kl = cl_calloc(sizeof(remote_key_learn_t), 1);
    
    return kl;
}

static void _key_learn_free(remote_key_learn_t* kl,bool free_kl)
{
    if (!kl) {
        return;
    }
    CL_THREAD_OFF(kl->t_learn);
    SAFE_FREE(kl->ajust_code);
    SAFE_FREE(kl->base_code);
    SAFE_FREE(kl->learn_code);
    SAFE_FREE(kl->host_gen_code);
    if (free_kl) {
        cl_free(kl);
    }
}

static void rk_free(remote_key_t* rk)
{
    if (rk) {
        cl_free(rk);
    }
}

static void _eq_free(equipment_t* eq,bool is_free_eq)
{
    remote_key_t* rk,*next;
    key_timer_param_t* param;
    
    if (!eq) {
        return;
    }
    
    SAFE_FREE(eq->prev_reply);
    eq->prev_reply_len = 0x0;
    
    SAFE_FREE(eq->nbp);
    eq->nbp_len = 0x0;

	SAFE_FREE(eq->soundlight);
	eq->soundlight_len = 0;
    
    if (eq->t_key_query) {
        param = CL_THREAD_ARG(eq->t_key_query);
        if (param != NULL) {
            cl_free(param);
        }
        CL_THREAD_OFF(eq->t_key_query);
    }
    
    if (eq->t_alarm_query) {
        param = CL_THREAD_ARG(eq->t_alarm_query);
        if (param) {
            cl_free(param);
        }
        CL_THREAD_OFF(eq->t_alarm_query);
    }
    
    stlc_list_for_each_entry_safe(remote_key_t, rk, next, &eq->keylist, link){
        stlc_list_del(&rk->link);
        rk_free(rk);
    }
    
    if (is_free_eq) {
        cl_free(eq);
    }
}

static void _eqc_free(equipment_ctrl_t *ec,int is_free_ec)
{
    equipment_t* eq,*next;
    if (!ec) {
        return;
    }
    
    stlc_list_for_each_entry_safe(equipment_t, eq, next, &ec->eq_list, link){
        stlc_list_del(&eq->link);
        _eq_free(eq,true);
    }
    
	if (is_free_ec) {
        SAFE_FREE(ec->prev_reply);
        ec->prev_reply_len = 0x0;
        CL_THREAD_OFF(ec->t_query);
        cl_free(ec);
    }
}
/**********************************************************************************/

static handle_backup_t* hb_lookup_by_handle(struct stlc_list_head* head,u_int32_t handle)
{
    handle_backup_t* hb;
    
    stlc_list_for_each_entry(handle_backup_t, hb, head , link){
        if (hb->handle == handle) {
            return hb;
        }
    }
    return NULL;
}

static handle_backup_t* hb_get_first(struct stlc_list_head* head)
{
    handle_backup_t* hb;
    if (!stlc_list_empty(head)) {
        hb = stlc_list_first_entry(head, handle_backup_t, link);
        return hb;
    }
    return NULL;
}
/**********************************************************************************/
// 电器设备处理

bool isAlarmDevice(equipment_t* eq)
{
    return !!(eq->attr.dev_type == REMOTE_TYPE_ALARM || eq->attr.dev_type == REMOTE_TYPE_BD_ALARM);
}

static bool is_rf_equipment(equipment_t* eq)
{
    return (eq->attr.dev_type >= REMOTE_TYPE_CURTAIN &&
            eq->attr.dev_type<= REMOTE_TYPE_OTHER &&
            eq->attr.dev_type!=REMOTE_TYPE_ALARM);
}

static bool is_rf2_equipment(equipment_t* eq)
{
    return ((eq->attr.dev_type == REMOTE_TYPE_BD_LAMP)||
            (eq->attr.dev_type == REMOTE_TYPE_BD_PLUG) ||
            (eq->attr.dev_type == REMOTE_TYPE_BD_DIMMING_LAMP));
}

static bool is_rf2_stat_equipment(equipment_t* eq)
{
    return ((eq->attr.dev_type == REMOTE_TYPE_BD_LAMP)||
            (eq->attr.dev_type == REMOTE_TYPE_BD_PLUG) ||
            (eq->attr.dev_type == REMOTE_TYPE_BD_CURTAIN) ||
            (eq->attr.dev_type == REMOTE_TYPE_BD_DIMMING_LAMP));
}

static bool is_infr_equipment(equipment_t* eq)
{
    return ((eq->attr.dev_type >= REMOTE_TYPE_W_TV &&
             eq->attr.dev_type<= REMOTE_TYPE_W_OTHER)||
            (eq->attr.dev_type >= REMOTE_TYPE_TV &&
             eq->attr.dev_type<= REMOTE_TYPE_AIRCONDITION));
}

static bool is_scene_remote_ctrl(equipment_t* eq)
{
    return (eq->attr.dev_type == REMOTE_TYPE_SCENE_CONTROLLER)?true:false;
}


#if 0
static bool is_infr_type(u_int8_t dev_type)
{
    return ((dev_type >= REMOTE_TYPE_W_TV &&
             dev_type<= REMOTE_TYPE_W_OTHER)||
            (dev_type >= REMOTE_TYPE_TV &&
             dev_type<= REMOTE_TYPE_AIRCONDITION));
}
#endif

static bool is_001e_infr_type(u_int8_t dev_type)
{
    return (dev_type >= REMOTE_TYPE_W_TV &&
             dev_type<= REMOTE_TYPE_W_OTHER);
}

static bool cl_remote_type_2_our_remote_type(u_int8_t cl_type,u_int8_t* our_type,u_int32_t* factory_id)
{
    if (cl_type<=CL_RT_BASE||cl_type>=CL_RT_MAX||!our_type||!factory_id) {
        return false;
    }
    
    *factory_id = 0;
    *our_type = 0;
    
    switch (cl_type) {
        case CL_RT_SMOKE_DETECTOR:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_SMOKE_DETECTOR;
            break;
        case CL_RT_GAS_LEAK_DETECTOR:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_GAS_LEAK_DETECTOR;
            break;
        case CL_RT_INFRARED_DETECTOR:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_INFRARED_DETECTOR;
            break;
        case CL_RT_DOOR_SENSOR:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_DOOR_SENSOR;
            break;
        case CL_RT_WINDOW_SENSOR:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_WINDOW_SENSOR;
            break;
        case CL_RT_INTELLIGENT_LOCK:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_INFRARED_DETECTOR;
            break;
        case CL_RT_EMERGENCY_BUTTON:
            *our_type = REMOTE_TYPE_ALARM;
            *factory_id = SAFETY_EMERGENCY_BUTTON;
            break;

        case CL_RT_DB_SMOKE_DETECTOR:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_SMOKE_DETECTOR;
            break;
        case CL_RT_DB_GAS_LEAK_DETECTOR:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_GAS_LEAK_DETECTOR;
            break;
        case CL_RT_DB_INFRARED_DETECTOR:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_INFRARED_DETECTOR;
            break;
        case CL_RT_DB_DOOR_SENSOR:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_DOOR_SENSOR;
            break;
        case CL_RT_DB_WINDOW_SENSOR:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_WINDOW_SENSOR;
            break;
        case CL_RT_DB_INTELLIGENT_LOCK:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_INFRARED_DETECTOR;
            break;
        case CL_RT_DB_EMERGENCY_BUTTON:
            *our_type = REMOTE_TYPE_BD_ALARM;
            *factory_id = SAFETY_EMERGENCY_BUTTON;
            break;

        case CL_RT_W_TV:
            *our_type = REMOTE_TYPE_W_TV;
            break;
        case CL_RT_W_TVBOX:
            *our_type = REMOTE_TYPE_W_TVBOX;
            break;
        case CL_RT_W_AIRCONDITION:
            *our_type = REMOTE_TYPE_W_AIRCONDITION;
            break;
        case CL_RT_W_OTHER:
            *our_type = REMOTE_TYPE_W_OTHER;
            break;
		case CL_RT_CLOUD_AIRCONDITION:
			*our_type = REMOTE_TYPE_CLOUD_AIRCONDITION;
			break;
        case CL_RT_CURTAIN:
            *our_type = REMOTE_TYPE_CURTAIN;
            break;
        case CL_RT_PLUG:
            *our_type = REMOTE_TYPE_PLUG;
            break;
        case CL_RT_LAMP:
            *our_type = REMOTE_TYPE_LAMP;
            break;
        case CL_RT_RF_CUSTOM:
            *our_type = REMOTE_TYPE_DIY;
            *factory_id = RDEVSUBTYPE_CUSTOM;
            break;
        case CL_RT_RF_CUSTOM_TV:
            *our_type = REMOTE_TYPE_DIY;
            *factory_id = RDEVSUBTYPE_RF_TO_INFR_TV;
            break;
        case CL_RT_RF_CUSTOM_TVBOX:
            *our_type = REMOTE_TYPE_DIY;
            *factory_id = RDEVSUBTYPE_RF_TO_INFR_TVBOX;
            break;
        case CL_RT_RF_CUSTOM_AIRCONDITION:
            *our_type = REMOTE_TYPE_DIY;
            *factory_id = RDEVSUBTYPE_RF_TO_INFR_AIRCONDITION;
            break;
        case CL_RT_RF_CUSTOM_INFR_CUSTOM:
            *our_type = REMOTE_TYPE_DIY;
            *factory_id = RDEVSUBTYPE_RF_TO_INFR_CUSTOM;
            break;

		case CL_RT_SOUNDLIGHT:
            *our_type = REMOTE_TYPE_SOUNDLIGHT;
			break;
		case CL_RT_SCENE_CONTROLLER:
            *our_type = REMOTE_TYPE_SCENE_CONTROLLER;
			break;
        case CL_RT_DB_RF_LAMP:
            *our_type = REMOTE_TYPE_BD_LAMP;
            break;
        case CL_RT_DB_RF_PLUG:
            *our_type = REMOTE_TYPE_BD_PLUG;
            break;
        case CL_RT_DB_RF_CURTAIN:
            *our_type = REMOTE_TYPE_BD_CURTAIN;
            break;
        case CL_RT_DB_RF_DIMMING_LAMP:
            *our_type = REMOTE_TYPE_BD_DIMMING_LAMP;
            break;
        case CL_RT_DB_RF_SOUNDLIGHT:
            *our_type = REMOTE_TYPE_BD_SOUND_LIGHT;
            break;
            
        default:
            return false;
            break;
    }
    
    return true;
}

static bool our_remote_type_2_cl_remote_type(u_int8_t our_type,u_int32_t factory_id,u_int8_t* cl_type)
{
    switch (our_type) {
        case REMOTE_TYPE_W_TV:
            *cl_type = CL_RT_W_TV;
            break;
        case REMOTE_TYPE_W_TVBOX:
            *cl_type = CL_RT_W_TVBOX;
            break;
        case REMOTE_TYPE_W_AIRCONDITION:
            *cl_type = CL_RT_W_AIRCONDITION;
            break;
        case REMOTE_TYPE_W_OTHER:
            *cl_type = CL_RT_W_OTHER;
            break;
		case REMOTE_TYPE_CLOUD_AIRCONDITION:
            *cl_type = CL_RT_CLOUD_AIRCONDITION;
			break;
        case REMOTE_TYPE_CURTAIN:
            *cl_type = CL_RT_CURTAIN;
            break;
        case REMOTE_TYPE_PLUG:
            *cl_type = CL_RT_PLUG;
            break;
        case REMOTE_TYPE_LAMP:
            *cl_type = CL_RT_LAMP;
            break;
        case REMOTE_TYPE_ALARM:
            if (factory_id >= SAFETY_SMOKE_DETECTOR &&
                factory_id <= SAFETY_EMERGENCY_BUTTON) {
                *cl_type = (u_int8_t)factory_id;
            }else{
                return false;
            }
            break;

        case REMOTE_TYPE_BD_ALARM:
            if (factory_id >= SAFETY_SMOKE_DETECTOR &&
                factory_id <= SAFETY_EMERGENCY_BUTTON) {
                *cl_type = (u_int8_t)factory_id+20;
            }else{
                return false;
            }
            break;

        case REMOTE_TYPE_DIY:
            if (factory_id == RDEVSUBTYPE_CUSTOM) 
                *cl_type = CL_RT_RF_CUSTOM;
            else if (factory_id == RDEVSUBTYPE_RF_TO_INFR_TV) 
                *cl_type = CL_RT_RF_CUSTOM_TV;
            else if (factory_id == RDEVSUBTYPE_RF_TO_INFR_TVBOX) 
                *cl_type = CL_RT_RF_CUSTOM_TVBOX;
            else if (factory_id == RDEVSUBTYPE_RF_TO_INFR_AIRCONDITION) 
                *cl_type = CL_RT_RF_CUSTOM_AIRCONDITION;
            else if (factory_id == RDEVSUBTYPE_RF_TO_INFR_CUSTOM) 
                *cl_type = CL_RT_RF_CUSTOM_INFR_CUSTOM;
            else
                return false;
            break;
		case REMOTE_TYPE_SOUNDLIGHT:
			*cl_type = CL_RT_SOUNDLIGHT;
			break;
		case REMOTE_TYPE_SCENE_CONTROLLER:
			*cl_type = CL_RT_SCENE_CONTROLLER;
			break;
        case REMOTE_TYPE_BD_LAMP:
            *cl_type = CL_RT_DB_RF_LAMP;
            break;
        case REMOTE_TYPE_BD_PLUG:
            *cl_type = CL_RT_DB_RF_PLUG;
            break;
        case REMOTE_TYPE_BD_CURTAIN:
            *cl_type = CL_RT_DB_RF_CURTAIN;
            break;
        case REMOTE_TYPE_BD_DIMMING_LAMP:
            *cl_type = CL_RT_DB_RF_DIMMING_LAMP;
            break;
        case REMOTE_TYPE_BD_SOUND_LIGHT:
            *cl_type = CL_RT_DB_RF_SOUNDLIGHT;
            break;
            
        default:
            return false;
            break;
    }
    return true;
}

static void callback_eq_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	slave_t *slave;
	equipment_ctrl_t *ec;

	if ((slave = (slave_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_eq_request, not found user type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	ec = slave->equipment;

	switch (w->cmd) {
	case CMD_REMOTE_CONFIG:
	case CMD_REMOTE_TD_CODE:
	case CMD_REMOTE_CONFIG_SOUNDLIGHT:
	case CMD_SCENE_LINKAGE:
		log_err(false, "%s callback_eq_request, cmd=%d, err=%u\n", slave->str_sn, w->cmd, result);
		break;

	default:
		log_err(false, "callback_eq_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}

static int timer_eq_query(cl_thread_t *t)
{
	equipment_ctrl_t *ec;
	pkt_t *pkt;
	user_t *user;
	net_remote_t *nr;

	ec = (equipment_ctrl_t *)CL_THREAD_ARG(t);
	user = ec->slave->user;
	
	ec->t_query = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, ec->t_query, timer_eq_query, (void *)ec, TIME_EQUIPMENT_QUERY);
    
    USER_BACKGROUND_RETURN_CHECK(user);

	pkt = pkt_new_v2(CMD_REMOTE_CONFIG, sizeof(net_remote_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, 
		ec->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_CONFIG, NULL, callback_eq_request);

	nr = get_pkt_payload(pkt, net_remote_t);
	memset(nr, 0, sizeof(net_remote_t));
	nr->version = 1;
	nr->action = AC_QUERY;

	user_add_pkt(user, pkt);
	
	return 0;
}

/***************************************************************************************/

equipment_t *eq_lookup(equipment_ctrl_t *ec, u_int16_t id)
{
	equipment_t *eq;
	
	stlc_list_for_each_entry(equipment_t, eq, &ec->eq_list, link) {
		if (eq->attr.local_id == id)
			return eq;
	}

	return NULL;
}

equipment_t *eq_lookup_by_handle(equipment_ctrl_t *ec, cl_handle_t eq_handle)
{
    equipment_t *eq;
	
	stlc_list_for_each_entry(equipment_t, eq, &ec->eq_list, link) {
		if (eq->handle == eq_handle)
			return eq;
	}
    
	return NULL;
}

static void replace_remote_config(equipment_ctrl_t *ec, struct stlc_list_head *ar)
{
	equipment_t *eq, *src;
    handle_backup_t* hb;

	if (ar == NULL||stlc_list_empty(ar)) {
        //停止定时器
        _eqc_free(ec, false);
		return;
	}

	stlc_list_for_each_entry(equipment_t, eq, ar, link) {
		src = eq_lookup(ec, eq->attr.local_id);
		if (src == NULL) {
            hb = hb_get_first(&ec->eq_new_eq_hand_list);
            if (hb) {
                eq->handle = hb->handle;
                stlc_list_del(&hb->link);
                cl_free(hb);
            }else{
                eq->handle = handle_create(HDLT_EQUIPMENT);
            }
            eq->t_alarm_query = NULL;
            eq->t_key_query = NULL;
		} else {
            stlc_list_replace_init(&src->keylist, &eq->keylist);
			eq->handle = src->handle;
            eq->t_key_query = src->t_key_query;
            eq->t_alarm_query = src->t_alarm_query;
            eq->nbp = src->nbp;
            eq->nbp_len = src->nbp_len;
			eq->soundlight_len = src->soundlight_len;
			eq->soundlight = src->soundlight;
			eq->match_id_num = src->match_id_num;
			memcpy(eq->match_id, src->match_id, sizeof(src->match_id));
			
            src->nbp = NULL;
            src->nbp_len = 0;
            src->t_alarm_query =
            src->t_key_query = NULL;
			src->soundlight = NULL;
		}
        /*每个key必定有定时刷新*/
        if (!eq->t_key_query) {
            ekey_do_query_immediately(eq);
        }
        
        if (isAlarmDevice(eq)&&!eq->t_alarm_query) {
            do_alarm_phone_quick_query(eq);
        }
        
	}
    stlc_list_for_each_entry_safe(equipment_t, eq, src, &ec->eq_list, link){
        stlc_list_del(&eq->link);
        _eq_free(eq, true);
    }
	stlc_list_replace_init(ar, &ec->eq_list);
}

static RS parse_remote_config(equipment_ctrl_t *ec, net_remote_t *nr)
{
	int i, k, st_sz;
	remote_atrri_v2_t *ra, *ra_src;
	equipment_t *e;
	struct stlc_list_head ar;

	STLC_INIT_LIST_HEAD(&ar);

	ra_src = (remote_atrri_v2_t *)(nr + 1);
	for (i = 0; i < nr->count; i++, ra_src = (remote_atrri_v2_t *)BOFP(ra_src, sizeof(remote_atrri_v2_t) + st_sz)) {
		st_sz = ra_src->n_state*sizeof(remote_state_t);
		
		e = (equipment_t *)cl_calloc(sizeof(equipment_t) + st_sz, 1);
		STLC_INIT_LIST_HEAD(&e->keylist);
		ra = &e->attr;
		
		memcpy(ra, ra_src, sizeof(remote_atrri_v2_t) + st_sz);
		ra->local_id = ntohs(ra->local_id);
		ra->ability = ntohl(ra->ability);
		ra->ir_id = ntohl(ra->ir_id);
		ra->factory_id = ntohl(ra->factory_id);
		ra->time_stamp_id = ntohl(ra->time_stamp_id);
		ra->bind_sn = ntoh_ll(ra->bind_sn);
        e->ec = ec;
		
		for (k = 0; k < ra->n_state; k++) {
			ra->state[k].state_id = ntohs(ra->state[k].state_id);
			ra->state[k].state_value = ntohs(ra->state[k].state_value);
			log_debug("parse_remote_config ra->n_state:%d ra->state[j].state_id:%d, ra->state[j].state_value:%d\n", ra->n_state, ra->state[k].state_id, ra->state[k].state_value);
		
		}
		stlc_list_add_tail(&e->link, &ar);
	}

	replace_remote_config(ec, &ar);

	return RS_OK;
}

static RS do_query_reply(equipment_ctrl_t *ec, pkt_t *pkt, net_remote_t *nr)
{
	user_t *user = ec->slave->user;
	unsigned int plen;

	plen = net_param_len(pkt->data);
	if (plen < sizeof(net_remote_t) + nr->count*sizeof(remote_atrri_v2_t)) {
		log_err(false, "bad CMD_REMOTE_CONFIG pkt: pare len=%u, but count=%u\n", plen, nr->count);
		return RS_ERROR;
	}
	
	if (ec->prev_reply != NULL && ec->prev_reply_len == plen) {
		if (memcmp(ec->prev_reply, nr, plen) == 0) {
			log_debug("same equipment, ignore\n");
			return RS_OK;
		}
	}
	SAFE_FREE(ec->prev_reply);
	ec->prev_reply_len = plen;
	ec->prev_reply = cl_malloc(plen);
	memcpy(ec->prev_reply, nr, plen);

	parse_remote_config(ec, nr);
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);

	return RS_OK;
}

static RS do_remote_config(equipment_ctrl_t *ec, pkt_t *pkt,u_int32_t param)
{
	net_remote_t *nr;
	RS ret = RS_OK;
    user_t *user = ec->slave->user;
    handle_backup_t* hb;

	if (net_param_len(pkt->data) < sizeof(net_remote_t)) {
		log_err(false, "do_remote_config failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
	
	nr = get_pkt_payload(pkt, net_remote_t);
	nr->err = ntohl(nr->err);
	nr->count = ntohs(nr->count);
	
	if (nr->version < 1) {
		log_err(false, "do_remote_config failed: version=%d\n", nr->version);
		return RS_ERROR;
	}
	log_debug("do_remote_config, err=%u, action=%d, version=%u, count=%u\n", nr->err, nr->action, nr->version, nr->count);
	
	switch (nr->action) {
	case AC_QUERY:
		ret = do_query_reply(ec, pkt, nr);
		break;
	case AC_ADD:
        if (nr->err != ERR_NONE) {
            hb = hb_lookup_by_handle(&ec->eq_new_eq_hand_list,param);
            if (hb) {
                stlc_list_del(&hb->link);
                cl_free(hb);
            }
            event_push_err(user->callback, EE_EQ_ADD_FAIL, user->handle, user->callback_handle, nr->err);
        }else{
            event_push(user->callback, EE_EQ_ADD_OK, user->handle, user->callback_handle);
        }
		break;
	case AC_MOD:
        if (nr->err != ERR_NONE) {
            event_push_err(user->callback, EE_EQ_MODIFY_FAIL, user->handle, user->callback_handle, nr->err);
        }else{
            event_push(user->callback, EE_EQ_MODIFY_OK, user->handle, user->callback_handle);
        }
		break;
	case AC_DEL:
        if (nr->err != ERR_NONE) {
            event_push_err(user->callback, EE_EQ_DEL_FAIL, user->handle, user->callback_handle, nr->err);
        }else{
            event_push(user->callback, EE_EQ_DEL_OK, user->handle, user->callback_handle);
        }
		break;
	default:
		ret = RS_ERROR;
		break;
	}

	return ret;
}

static void eq_proc_add(slave_t* slave,cln_equipment_t* eql,RS * ret)
{
    user_t* user;
    net_remote_t* nr;
    remote_atrri_v2_t* attr;
    equipment_ctrl_t *ec;
    pkt_t *pkt;
    u_int8_t dev_type = 0;
    u_int32_t factory_id = 0;
    handle_backup_t* hand_bk;
    int stat_count = 0;
    slave_t* s_001e = NULL;
	area_priv_t *area = NULL;
    
    ec = slave->equipment;
    user = slave->user;
    
    if (slave->sn!=slave->user->sn) {
        *ret = RS_INVALID_PARAM;
        log_err(false, "eq_proc_other The slave must be master slave sn %s user sn %012llu\n",slave->str_sn,user->sn);
        return;
    }
    
    if (!cl_remote_type_2_our_remote_type(eql->eq_type, &dev_type, &factory_id)) {
        *ret = RS_NOT_FOUND;
        log_err(false, "eq_proc_add can not support type %u\n",eql->eq_type);
        return;
    }
    
    if ( eql->s_001e_handle) {
        s_001e = lookup_by_handle(HDLT_SLAVE, eql->s_001e_handle);
        if (!s_001e) {
            *ret = RS_NOT_FOUND;
            log_err(false, "eq_proc_add can not find 001e slave  handle = %u\n",eql->s_001e_handle);
            return;
        }
    }

	if (eql->area_handle != 0 && (area = (area_priv_t *)lookup_by_handle(HDLT_AREA, eql->area_handle)) == NULL) {
            *ret = RS_NOT_FOUND;
            log_err(false, "eq_proc_add can not find area=0x%08x\n", eql->area_handle);
            return;
	}
    
    if (eql->group_num<1) {
        eql->group_num = 0x1;
    }else if(eql->group_num>8){
        eql->group_num = 8;
    }
    if (dev_type == REMOTE_TYPE_LAMP||
        dev_type == REMOTE_TYPE_PLUG) {
        stat_count = 0x2;
    }
    
    
    /*缓存新handle*/
    hand_bk = cl_calloc(sizeof(handle_backup_t), 1);
    hand_bk->handle = handle_create(HDLT_EQUIPMENT);
    *(eql->add_handle) = hand_bk->handle;
    stlc_list_add_tail(&hand_bk->link, &ec->eq_new_eq_hand_list);
    
    pkt = pkt_new_v2(CMD_REMOTE_CONFIG, sizeof(net_remote_t)+sizeof(remote_atrri_v2_t)+stat_count*sizeof(remote_state_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_CONFIG, (void*)hand_bk->handle, callback_eq_request);
    nr = get_pkt_payload(pkt, net_remote_t);
    attr = (remote_atrri_v2_t*)(nr+1);
    nr->count = htons(1);
    nr->version = 0x1;
    nr->action = AC_ADD;

    attr->area_id = (area == NULL ? 0 : area->area_id);
    attr->dev_type = dev_type;
    attr->factory_id = htonl(factory_id);
    attr->alarm_flag = ALARM_DISABLED;
    strncpy((char*)attr->name, eql->name, sizeof(attr->name)-1);
    if (s_001e) {
        attr->bind_sn = ntoh_ll(s_001e->sn);
    }
    
    if (dev_type == REMOTE_TYPE_LAMP||
        dev_type == REMOTE_TYPE_PLUG) {
        attr->n_state = 0x2;
        attr->state[0].state_id = htons(STATEID_OUTLET_GROUPNUM);
        attr->state[0].state_value = htons(eql->group_num);
        attr->state[1].state_id = htons(STATEID_OUTLET_FUCTION);
        if (eql->is_more_ctrl) {
            attr->state[1].state_value = htons(STATEID_FUNC_BTN_ONOFF_CTRL);
        }else{
            attr->state[1].state_value = 0x0;
        }
        
    }
    
    user_add_pkt(user, pkt);
    CL_THREAD_OFF(ec->t_query);
    CL_THREAD_TIMER_ON(&cl_priv->master, ec->t_query, timer_eq_query, (void *)ec, 0);
}

static RS do_remote_td_code(equipment_ctrl_t *ec, pkt_t *pkt,u_int32_t param)
{
	net_remote_td_code *nr;
	RS ret = RS_OK;
    user_t *user = ec->slave->user;
    handle_backup_t* hb;

	if (net_param_len(pkt->data) < sizeof(net_remote_td_code)) {
		log_err(false, "do_remote_td_code failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
	
	nr = get_pkt_payload(pkt, net_remote_td_code);
	nr->err = ntohl(nr->err);
	
	log_debug("do_remote_td_code, err=%u\n", nr->err);
	
    if (nr->err != ERR_NONE) {
        hb = hb_lookup_by_handle(&ec->eq_new_eq_hand_list, param);
        if (hb) {
            stlc_list_del(&hb->link);
            cl_free(hb);
        }
        event_push_err(user->callback, EE_EQ_ADD_FAIL, user->handle, user->callback_handle, nr->err);
    }else{
        event_push(user->callback, EE_EQ_ADD_OK, user->handle, user->callback_handle);
    }

	return ret;
}

static void eq_proc_add_by_json(slave_t* slave, cln_equipment_t* eql, RS * ret)
{
    user_t* user;
    pkt_t *pkt;
    u_int8_t dev_type = 0;
    u_int32_t factory_id = 0;
    handle_backup_t* hand_bk;
	net_remote_td_code *td;
	int json_len;
    equipment_ctrl_t *ec;
    
    ec = slave->equipment;
    user = slave->user;
    
    if (slave->sn != slave->user->sn) {
        *ret = RS_INVALID_PARAM;
        log_err(false, "eq_proc_add_by_json The slave must be master slave sn %s user sn %012llu\n",slave->str_sn,user->sn);
        return;
    }
    
    /*缓存新handle*/
    hand_bk = cl_calloc(sizeof(handle_backup_t), 1);
    hand_bk->handle = handle_create(HDLT_EQUIPMENT);
    *(eql->add_handle) = hand_bk->handle;
    stlc_list_add_tail(&hand_bk->link, &ec->eq_new_eq_hand_list);

	json_len = (int)((strlen(eql->json) + 1 + 3)&(~3));
    pkt = pkt_new_v2(CMD_REMOTE_TD_CODE, sizeof(net_remote_td_code) + json_len, NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_REMOTE_CONFIG, (void*)hand_bk->handle, callback_eq_request);
    td = get_pkt_payload(pkt, net_remote_td_code);
	memset(td, 0, sizeof(net_remote_td_code));
	td->len = (u_int8_t)(json_len >> 2);
	strcpy((char*)td->value, (char*)eql->json);
    
    user_add_pkt(user, pkt);
	
    CL_THREAD_OFF(ec->t_query);
    CL_THREAD_TIMER_ON(&cl_priv->master, ec->t_query, timer_eq_query, (void *)ec, 0);
}

static void eq_proc_db_rf_scan(cl_notify_pkt_t *cln_pkt, RS *ret)
{
	user_t* user;
	cl_handle_t *handle;
	net_remote_bd_bind *p;
	pkt_t *pkt;

	handle = (cl_handle_t *)cln_pkt->data;

	cl_lock(&cl_priv->mutex);

	user = lookup_by_handle(HDLT_USER, *handle);
	if (user == NULL) {
		*ret = RS_NOT_FOUND;
		goto done;
	}
	if (user->status != CS_ESTABLISH) {
		*ret = RS_NOT_LOGIN;
		goto done;
	}
	pkt = pkt_new_v2(CMD_REMOTE_BD_BIND,  sizeof(*p), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (pkt == NULL) {
		*ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	p = get_pkt_payload(pkt, net_remote_bd_bind);
	p->err = ntohl(0);
	p->action = AC_RF2_BIND;
	p->pad = 0;
	p->local_id = 0;

	user_add_pkt(user, pkt);
	*ret = RS_OK;
		
done:
	cl_unlock(&cl_priv->mutex);
	return;
}

static void eq_proc_rf_repeater_set(equipment_t *eq, cln_equipment_t * eql, RS *ret)
{
	user_t* user;
	slave_t* slave;
	equipment_ctrl_t *ec;
	pkt_t *pkt;
	net_remote_bd_bind *p;
    
	ec = eq->ec;
	slave= eq->ec->slave;
	user = slave->user;

	pkt = pkt_new_v2(CMD_REMOTE_BD_BIND,  sizeof(*p), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (pkt == NULL) {
		*ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	p = get_pkt_payload(pkt, net_remote_bd_bind);
	p->err = ntohl(0);
	if(eql->rf_repeater_on)
		p->action = AC_RF2_REPEATER_ON;
	else
		p->action = AC_RF2_REPEATER_OFF;
	p->pad = 0;
	p->local_id = ntohs(eq->attr.local_id);

	user_add_pkt(user, pkt);
	*ret = RS_OK;

done:
	return;
}

static void eq_proc_eq_refresh(equipment_t* eq, RS * ret)
{
	if (eq && eq->ec) {
		eq_force_update_all_info(eq->ec);
		*ret = RS_OK;
		return;
	}
	*ret = RS_NOT_LOGIN;
}

static void eq_tcp_remote_db_rf(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	net_remote_bd_bind *p;
	
	if (hdr->param_len < sizeof(*p)) {
		return;
	}
	p = get_pkt_payload(pkt, net_remote_bd_bind);
	p->err = ntohl(p->err);
	if (user->callback == NULL)
		return;
	if ( p->err == ERR_NONE ) {
		event_push(user->callback, EE_EQ_DB_RF_SCAN_OK, user->handle, user->callback_handle);		
	} else {
		event_push_err(user->callback, EE_EQ_DB_RF_SCAN_FAIL, user->handle, user->callback_handle, p->err);		
	}
}

static void eq_proc_other(equipment_t* eq,slave_t* dest_slave,u_int32_t type,cln_equipment_t* eql,RS * ret)
{
    user_t* user;
    slave_t* slave;
    net_remote_t* nr;
    remote_atrri_v2_t* attr;
    equipment_ctrl_t *ec;
    pkt_t *pkt;
    
    
    ec = eq->ec;
    slave= eq->ec->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_REMOTE_CONFIG, sizeof(net_remote_t)+sizeof(remote_atrri_v2_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_CONFIG, NULL, callback_eq_request);
    nr = get_pkt_payload(pkt, net_remote_t);
    attr = (remote_atrri_v2_t*)(nr+1);
    nr->count = htons(1);
    nr->version = 0x1;
    
    memcpy(attr, &eq->attr, sizeof(*attr));
    attr->ability = 0x0;
    attr->bind_sn = ntoh_ll(attr->bind_sn);
    attr->factory_id = ntohl(attr->factory_id);
    attr->local_id = htons(attr->local_id);
    attr->n_state = 0x0;
    attr->ir_id = htonl(attr->ir_id);
    attr->time_stamp_id = htonl(attr->time_stamp_id);
    
    switch (type) {
        case CLNE_EQ_DEL:
        {
            nr->action = AC_DEL;
        }
            break;
        case CLNE_EQ_CH_NAME:
        {
            nr->action = AC_MOD;
            strncpy((char*)attr->name, eql->name, sizeof(attr->name)-1);
            attr->name[sizeof(attr->name)-1]=0x0;
        }
            break;
        case CLNE_EQ_CH_AREA:
        {
			area_priv_t *area;
			if (eql->area_handle == 0 || (area = (area_priv_t *)lookup_by_handle(HDLT_AREA, eql->area_handle)) == NULL) {
		            *ret = RS_NOT_FOUND;
		            log_err(false, "eq_proc_other can not find area=0x%08x\n", eql->area_handle);
		            return;
			}
			nr->action = AC_MOD;
            attr->area_id = area->area_id;
        }
            break;
        case CLNE_EQ_ALARM_SET_BIND_001E:
        {
            nr->action = AC_MOD;
            attr->bind_sn = ntoh_ll(dest_slave->sn);
            break;
        }
        case CLNE_EQ_ALARM_ENABLE:
        {
            nr->action = AC_MOD;
            attr->alarm_flag &= 0xfc;
            if (eql->enableAlarm) {
                attr->alarm_flag|=ALARM_ENABLED;
            }else{
                attr->alarm_flag|=ALARM_DISABLED;
            }
            break;
        }
		case CLNE_EQ_ALARM_PUSH_ENABLE: /* 消息推送开关 */
            nr->action = AC_MOD;
			attr->alarm_flag &= 0xf8;
			/* 双向报警器默认为推送和短信可以分开控制 */
			if (slave->has_alarm_swich || attr->dev_type == REMOTE_TYPE_BD_ALARM) { /* 新版本 */
				if (eql->enableAlarm) {
	                attr->alarm_flag |= ALARM_PUSH;
				} else { /* 必须全关 */
				}
			} else { /* 老版本 */
				if (eql->enableAlarm) {
	                attr->alarm_flag |= ALARM_ENABLED;
				} else {
	                attr->alarm_flag |= ALARM_DISABLED;
				}
			}
			break;
 		case CLNE_EQ_ALARM_SMS_ENABLE: /* 短消息开关 */
            nr->action = AC_MOD;
			attr->alarm_flag &= 0xf8;
			if (eql->enableAlarm) { /* 开，必须全开 */
				if (slave->has_alarm_swich || attr->dev_type == REMOTE_TYPE_BD_ALARM) { /* 新版本 */
	                attr->alarm_flag |= (ALARM_ENABLED | ALARM_PUSH);
				} else { /* 老版本 */
	                attr->alarm_flag |= ALARM_ENABLED;
				}
			} else { /* 关，新版本的可以单独关 */
 				if (slave->has_alarm_swich || attr->dev_type == REMOTE_TYPE_BD_ALARM) { /* 新版本 */
	               attr->alarm_flag |= (ALARM_DISABLED | (eq->attr.alarm_flag & ALARM_PUSH));
				} else { /* 老版本，全关 */
	               attr->alarm_flag |= ALARM_DISABLED;
 				}
			}
			break;
           
        default:
            *(int*)0=0;
            return;
            break;
    }
    
    user_add_pkt(user, pkt);
    
    CL_THREAD_OFF(ec->t_query);
    CL_THREAD_TIMER_ON(&cl_priv->master, ec->t_query, timer_eq_query, (void *)ec, 0);
}

static int timer_alarm_phone_query(cl_thread_t *t)
{
    key_timer_param_t* param;
	equipment_ctrl_t *ec;
    equipment_t* eq;
	pkt_t *pkt;
	user_t *user;
	net_alarm_bind_phone_t* nbp;
	net_remote_config_soundlight *slp;
    
    param = (key_timer_param_t*)CL_THREAD_ARG(t);
	ec = param->ec;
	user = ec->slave->user;
	eq = eq_lookup(ec, param->local_id);
    
    /*查找电器*/
    
    if (!eq) {
        log_debug("timer_alarm_phone_query, find remote id[%u] failed!\n",param->local_id);
        cl_free(param);
        return 0;
    }
    eq->t_alarm_query = NULL;
	
	CL_THREAD_TIMER_ON(&cl_priv->master, eq->t_alarm_query, timer_alarm_phone_query, (void *)param, TIME_ALARM_PHONE_QUERY);
    USER_BACKGROUND_RETURN_CHECK(user);
    
	pkt = pkt_new_v2(CMD_ALARM_BIND_PHONE, sizeof(net_alarm_bind_phone_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     ec->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_ALARM_BIND_PHONE,
                               NULL, callback_eq_request);
    nbp = get_pkt_payload(pkt, net_alarm_bind_phone_t);
    nbp->action = AC_QUERY;
    nbp->local_id = htons(param->local_id);
	user_add_pkt(user, pkt);

	// 查询关联的声光报警器
	pkt = pkt_new_v2(CMD_REMOTE_CONFIG_SOUNDLIGHT, sizeof(net_remote_config_soundlight), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     ec->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_CONFIG_SOUNDLIGHT,
                               NULL, callback_eq_request);
    slp = get_pkt_payload(pkt, net_remote_config_soundlight);
    slp->action = AC_QUERY;
    slp->local_id = htons(param->local_id);
	user_add_pkt(user, pkt);
	
	return 0;
}

static void do_alarm_phone_quick_query(equipment_t* eq)
{
    key_timer_param_t* param;
    
    CL_THREAD_OFF(eq->t_alarm_query);
    param = cl_malloc(sizeof(key_timer_param_t));
    if (param) {
        param->ec = eq->ec;
        param->local_id = eq->attr.local_id;
        /*直接刷新*/
        CL_THREAD_TIMER_ON(&cl_priv->master, eq->t_alarm_query, timer_alarm_phone_query, (void *)param, 0);
    }
}

static void eq_proc_alarm_phone(equipment_t* eq,slave_t* dest_slave,u_int32_t type,
                                cln_equipment_t* eql,RS * ret)
{
    user_t* user;
    slave_t* slave;
    equipment_ctrl_t *ec;
    pkt_t *pkt;
    net_alarm_bind_phone_t* nbp;
    int i;
    char* phone,*p;
    
    
    ec = eq->ec;
    slave= eq->ec->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_ALARM_BIND_PHONE, sizeof(*nbp)+eql->numofphone*16, NHF_TRANSPARENT|NHF_WAIT_REPLY,slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_ALARM_BIND_PHONE, NULL, callback_eq_request);
    nbp = get_pkt_payload(pkt, net_alarm_bind_phone_t);
    nbp->count = eql->numofphone>10?10:eql->numofphone;
    nbp->local_id = htons(eq->attr.local_id);
    nbp->action = AC_MOD;
    p = (char*)nbp->phones;
    for (i=0; i<nbp->count; i++,p+=16) {
        phone = eql->phonelist[i];
        strncpy(p, phone, 15);
        p[15]='\0';
    }
    user_add_pkt(user, pkt);
    do_alarm_phone_quick_query(eq);
}


static bool _eq_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_equipment_t * eql;
    equipment_t* eq = NULL;
    slave_t* slave = NULL;
    bool need_ck_slave = false;
    bool need_ck_eq = false;
    
    if (pkt->type == CLNE_EQ_ADD || pkt->type == CLNE_EQ_ADD_BY_JSON) {
        need_ck_slave = true;
    }else{
        need_ck_eq = true;
        if (pkt->type == CLNE_EQ_ALARM_SET_BIND_001E) {
            need_ck_slave = true;
        }
    }
    
    eql = (cln_equipment_t *)&pkt->data[0];
    if (need_ck_slave) {
        slave = lookup_by_handle(HDLT_SLAVE, eql->slave_handle);
        if (!slave) {
            *ret = RS_NOT_FOUND;
            log_err(false, "_eq_proc_notify can not found slave handle 0X%08X\n", eql->slave_handle);
            return true;
        }
        
        if ( pkt->type == CLNE_EQ_ALARM_SET_BIND_001E &&
            !slave->has_ir) {
            *ret = RS_NOT_FOUND;
            log_err(false, "_eq_proc_notify The slave is not 001E 0X%08X\n", eql->slave_handle);
            return true;
        }
    }
    
    if(need_ck_eq)
    {
        eq = lookup_by_handle(HDLT_EQUIPMENT, eql->eq_handle);
        if (!eq) {
            *ret = RS_NOT_FOUND;
            log_err(false, "_eq_proc_notify can not found equipment handle 0X%08X\n", eql->eq_handle);
            return true;
        }
    }
    
    if (pkt->type == CLNE_EQ_ALARM_PHONE
        || pkt->type == CLNE_EQ_ALARM_ENABLE
        || pkt->type == CLNE_EQ_ALARM_PUSH_ENABLE
        || pkt->type == CLNE_EQ_ALARM_SMS_ENABLE) 
	{
        if (!isAlarmDevice(eq)) {
            *ret = RS_INVALID_PARAM;
            log_err(false, "_eq_proc_notify The equipment is not alarm device. handle 0X%08X\n", eql->eq_handle);
            return true;
        }
	}
    
    switch (pkt->type) {
        case CLNE_EQ_ADD:
            eq_proc_add(slave,eql,ret);
            break;
        case CLNE_EQ_ADD_BY_JSON:
            eq_proc_add_by_json(slave, eql, ret);
            break;
        case CLNE_EQ_REFRESH:
            eq_proc_eq_refresh(eq ,ret);
            break;
        case CLNE_EQ_RF_REPEATER_SET:
            eq_proc_rf_repeater_set(eq, eql, ret);
            break;
        case CLNE_EQ_DEL:
        case CLNE_EQ_CH_NAME:
        case CLNE_EQ_CH_AREA:
        case CLNE_EQ_ALARM_ENABLE:
		case CLNE_EQ_ALARM_PUSH_ENABLE:
		case CLNE_EQ_ALARM_SMS_ENABLE:
        case CLNE_EQ_ALARM_SET_BIND_001E:
            eq_proc_other(eq, slave,pkt->type, eql, ret);
            break;
        case CLNE_EQ_ALARM_PHONE:
            eq_proc_alarm_phone(eq, slave, pkt->type, eql, ret);
            break;
            
        default:
            return false;
            break;
    }

    return true;
}

static void do_alarm_phone_replace(slave_t* slave,equipment_t* eq,pkt_t *pkt)
{
    net_alarm_bind_phone_t* nbp;
    int len = net_param_len(pkt->data)-sizeof(net_alarm_bind_phone_t);
    
    nbp = get_pkt_payload(pkt, net_alarm_bind_phone_t);
    if (len<0||nbp->error||len!=nbp->count*16) {
        log_err(false, "do_alarm_phone_relpace failed: err %u\n", nbp->error);
        return;
    }
    
    if (eq->nbp &&
        net_param_len(pkt->data) == eq->nbp_len &&
        !memcmp(eq->nbp, nbp, eq->nbp_len)) {
        log_debug("eq_do_alarm_phone_replace same packet ignore! local_id %u\n",eq->attr.local_id);
        return;
    }

    SAFE_FREE(eq->nbp);
    eq->nbp_len = 0;
    
    eq->nbp = cl_calloc(net_param_len(pkt->data), 1);
    if (eq->nbp) {
        memcpy(eq->nbp, nbp, net_param_len(pkt->data));
        eq->nbp_len = net_param_len(pkt->data);
    }

	event_push(slave->user->callback, UE_INFO_MODIFY, slave->user->handle, slave->user->callback_handle);
}

static RS do_alarm_phone_config(slave_t* slave, pkt_t *pkt)
{
    net_alarm_bind_phone_t* nbp;
    user_t* user = slave->user;
    equipment_t* eq;
    
    if (net_param_len(pkt->data) < sizeof(net_alarm_bind_phone_t)) {
		log_err(false, "do_alarm_phone_config failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
    
    nbp = get_pkt_payload(pkt, net_alarm_bind_phone_t);
    nbp->error = ntohl(nbp->error);
    nbp->local_id = ntohs(nbp->local_id);
    
    eq = eq_lookup(slave->equipment, nbp->local_id);
    if (!eq) {
        log_debug("do_alarm_phone_config failed: can not found eq localid=%u err %u\n", nbp->local_id,nbp->error);
		return RS_ERROR;
    }
    
    switch (nbp->action) {
        case AC_QUERY:
            do_alarm_phone_replace(slave, eq, pkt);
            break;
        case AC_MOD:
            if (nbp->error) {
                event_push_err(user->callback, EE_EQ_MODIFY_FAIL, user->handle, user->callback_handle, nbp->error);
            }else{
                event_push(user->callback, EE_EQ_MODIFY_OK, user->handle, user->callback_handle);
            }
            break;
            
        default:
            return RS_ERROR;
            break;
    }
    
    return RS_OK;
}

/////////////////////////////////////////////////////

static void do_soundlight_replace(slave_t* slave, equipment_t* eq, pkt_t *pkt)
{
    net_remote_config_soundlight *slp;
    int item_len = net_param_len(pkt->data) - sizeof(net_remote_config_soundlight);
    
    slp = get_pkt_payload(pkt, net_remote_config_soundlight);
    if (item_len < 0 || slp->err || item_len != slp->count*sizeof(remote_soundlight_list_t)) {
        log_err(false, "do_soundlight_replace failed: err %u, item_len=%d, count=%d\n", slp->err, item_len, slp->count);
        return;
    }
    
    if (eq->soundlight != NULL
        && net_param_len(pkt->data) == eq->soundlight_len
        && memcmp(eq->soundlight, slp, eq->soundlight_len) == 0)
	{
        log_debug("do_soundlight_replace same packet ignore! local_id %u\n", eq->attr.local_id);
        return;
	}

    SAFE_FREE(eq->soundlight);
    eq->soundlight_len = 0;
    
    eq->soundlight = cl_malloc(net_param_len(pkt->data));
    if (eq->soundlight) {
        memcpy(eq->soundlight, slp, net_param_len(pkt->data));
        eq->soundlight_len = net_param_len(pkt->data);
		log_debug("Detect soundlight modify: on=%d, count=%d, loca_id=%u\n",
			eq->soundlight->onoff, eq->soundlight->count, eq->soundlight->local_id);
    }

	event_push(slave->user->callback, UE_INFO_MODIFY, slave->user->handle, slave->user->callback_handle);
}

static RS do_remote_soundlight(slave_t* slave, pkt_t *pkt)
{
    net_remote_config_soundlight *slp;
    user_t* user = slave->user;
    equipment_t* eq;
    
    if (net_param_len(pkt->data) < sizeof(net_remote_config_soundlight)) {
		log_err(false, "do_remote_soundlight failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
    
    slp = get_pkt_payload(pkt, net_remote_config_soundlight);
    slp->err = ntohl(slp->err);
    slp->local_id = ntohs(slp->local_id);
    slp->timeout = ntohs(slp->timeout);
    
    eq = eq_lookup(slave->equipment, slp->local_id);
    if (!eq) {
        log_err(false, "do_remote_soundlight failed: can not found eq localid=%u err %u, action=%u\n",
			slp->local_id, slp->err, slp->action);
		return RS_ERROR;
    }
    log_debug("do_remote_soundlight eq handle=0x%08x, localid=%u err %u, action=%u\n",
		eq->handle, slp->local_id, slp->err, slp->action);
    
    switch (slp->action) {
    case AC_QUERY:
        do_soundlight_replace(slave, eq, pkt);
        break;
    case AC_MOD:
        if (slp->err) {
            event_push_err(user->callback, EE_EQ_MODIFY_FAIL, user->handle, user->callback_handle, slp->err);
        }else{
            event_push(user->callback, EE_EQ_MODIFY_OK, user->handle, user->callback_handle);
        }
        break;
        
    default:
        return RS_ERROR;
        break;
    }
    
    return RS_OK;
}

/*********************************************************************************/
static int timer_key_query(cl_thread_t *t)
{
    key_timer_param_t* param;
	equipment_ctrl_t *ec;
    equipment_t* eq;
	pkt_t *pkt;
	user_t *user;
	net_remote_key_t *kr;
    
    param = (key_timer_param_t*)CL_THREAD_ARG(t);
	ec = param->ec;
	user = ec->slave->user;
	eq = eq_lookup(ec, param->local_id);
    /*查找电器*/
    if (!eq) {
        log_debug("timer_key_query, find remote id[%u] failed!\n",param->local_id);
        cl_free(param);
        return 0;
    }
    //TODO：如果有两个相同ID的，就找错了
	eq->t_key_query = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, eq->t_key_query, timer_key_query, (void *)param, TIME_REMOTE_KEY_QUERY);
    
    USER_BACKGROUND_RETURN_CHECK(user);
    
	pkt = pkt_new_v2(CMD_REMOTE_KEY_CONFIG, sizeof(net_remote_key_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     ec->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_KEY_CONFIG, NULL, callback_eq_request);
    
	kr = get_pkt_payload(pkt, net_remote_key_t);
	memset(kr, 0, sizeof(net_remote_key_t));
	kr->local_id = htons(eq->attr.local_id);
	kr->action = AC_QUERY;
    
	user_add_pkt(user, pkt);
	
	return 0;
}

remote_key_t *rk_lookup(equipment_t *eq, u_int32_t id)
{
	remote_key_t *rk;
	
	stlc_list_for_each_entry(remote_key_t, rk, &eq->keylist, link) {
		if (rk->key_attr.key_id == id)
			return rk;
	}
    
	return NULL;
}

static remote_key_t* look_up_db_first_key(equipment_t* eq)
{
    remote_key_t *rk;
    
    if (!is_rf2_equipment(eq) || stlc_list_empty(&eq->keylist)) {
        return NULL;
    }
    
    rk = stlc_list_first_entry(&eq->keylist, remote_key_t, link);
    
    return rk;
}

void eq_force_update_all_info(equipment_ctrl_t *ec)
{
    if (!ec) {
        return;
    }
    CL_THREAD_OFF(ec->t_query);
    CL_THREAD_TIMER_ON(&cl_priv->master, ec->t_query, timer_eq_query, (void *)ec, 0);
}

static RS parse_key_config(equipment_t *eq, net_remote_key_t *nk)
{
    int i;
    remote_key_attri_t* ksrc,*kdest;
    remote_key_t* rk;

	stlc_list_free(remote_key_t, &eq->keylist, link);

    ksrc = nk->attri;
    for (i=0; i<nk->count; i++,ksrc++) {
        rk = cl_calloc(sizeof(remote_key_t),1);
        rk->eq = eq;
        memcpy(&rk->key_attr, ksrc, sizeof(remote_key_attri_t));
        kdest = &rk->key_attr;
        kdest->key_id = ntohl(kdest->key_id);
        stlc_list_add_tail(&rk->link, &eq->keylist);
    }
	
    return RS_OK;
}

static RS do_key_query_reply(equipment_ctrl_t *ec, pkt_t *pkt, net_remote_key_t *nr)
{
	user_t *user = ec->slave->user;
	unsigned int plen;
    equipment_t* eq;
    
	plen = net_param_len(pkt->data);
	if (plen < sizeof(net_remote_key_t) + nr->count*sizeof(remote_key_attri_t)) {
		log_err(false, "bad CMD_REMOTE_KEY_CONFIG pkt: pare len=%u, but count=%u\n", plen, nr->count);
		return RS_ERROR;
	}
    /*local id的查找是用的本机序*/
    eq = eq_lookup(ec,ntohs(nr->local_id));
    if (!eq) {
        log_err(false, "bad CMD_REMOTE_KEY_CONFIG pkt: local id=%u lookup failed!\n", nr->local_id);
        return RS_ERROR;
    }

	if (eq->prev_reply != NULL && eq->prev_reply_len == plen) {
		if (memcmp(eq->prev_reply, nr, plen) == 0) {
			log_debug("same remote key, ignore\n");
			return RS_OK;
		}
	}
	SAFE_FREE(eq->prev_reply);
	eq->prev_reply_len = plen;
	eq->prev_reply = cl_malloc(plen);
	memcpy(eq->prev_reply, nr, plen);
    
	parse_key_config(eq, nr);
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    
	return RS_OK;
}

static RS do_key_operation_reply(equipment_ctrl_t *ec, pkt_t *pkt, net_remote_key_t *nr)
{
    net_remote_key_t *kr;
    user_t *user = ec->slave->user;
    
    kr = get_pkt_payload(pkt, net_remote_key_t);
    switch (kr->action) {
        case AC_ADD:
            if (kr->err) {
                event_push_err(user->callback,EE_EKEY_ADD_FAIL, user->handle, user->callback_handle, kr->err);
            }else{
                event_push(user->callback,EE_EKEY_ADD_OK, user->handle, user->callback_handle);
            }
            break;
        case AC_DEL:
            if (kr->err) {
                event_push_err(user->callback,EE_EKEY_DEL_FAIL, user->handle, user->callback_handle, kr->err);
            }else{
                event_push(user->callback,EE_EKEY_DEL_OK, user->handle, user->callback_handle);
            }
            break;
        case AC_MOD:
            if (kr->err) {
                event_push_err(user->callback,EE_EKEY_MODIFY_FAIL, user->handle, user->callback_handle, kr->err);
            }else{
                event_push(user->callback,EE_EKEY_MODIFY_OK, user->handle, user->callback_handle);
            }
            break;
        default:
            return RS_ERROR;
            break;
    }
    return RS_OK;
}

static RS do_key_config(equipment_ctrl_t *ec, pkt_t *pkt)
{
	net_remote_key_t *kr;
	RS ret = RS_OK;
    
	if (net_param_len(pkt->data) < sizeof(net_remote_key_t)) {
		log_err(false, "do_key_config failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
	
	kr = get_pkt_payload(pkt, net_remote_key_t);
	kr->err = ntohl(kr->err);
	
	log_debug("do_key_config, err=%u, action=%d, count=%u, localid=%u\n", kr->err, kr->action, kr->count,
              ntohs(kr->local_id));
	
	switch (kr->action) {
        case AC_QUERY:
            ret = do_key_query_reply(ec,pkt,kr);
            break;
        case AC_ADD:
        case AC_MOD:
        case AC_DEL:
            ret = do_key_operation_reply(ec, pkt, kr);
            break;
        default:
            ret = RS_ERROR;
            break;
	}
    
	return ret;
}

static RS do_key_ctrl(equipment_ctrl_t *ec, pkt_t *pkt)
{
    net_remote_ctrl_t* nrc;
    user_t* user = ec->slave->user;
    
    if (net_param_len(pkt->data) < sizeof(net_remote_ctrl_t)) {
		log_err(false, "do_key_ctrl failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
    nrc = get_pkt_payload(pkt, net_remote_ctrl_t);
    
    log_debug("do_key_ctrl, err=%lu, localid=%u,key_id=%lu\n", ntohl(nrc->err),ntohs(nrc->local_id),
              ntohl(nrc->key_id));
	
	if (nrc->err) {
        event_push_err(user->callback,EE_EKEY_SEND_SINGAL_FAIL, user->handle, user->callback_handle, nrc->err);
    }else{
        event_push(user->callback,EE_EKEY_SEND_SINGAL_OK, user->handle, user->callback_handle);
    }

    return RS_OK;
}

static void ekey_do_query_immediately(equipment_t* eq)
{
    key_timer_param_t* param = NULL;
	
    if (eq->t_key_query){
        param = CL_THREAD_ARG(eq->t_key_query);
    }
    
    CL_THREAD_OFF(eq->t_key_query);
    
    if (!param){
        param = cl_malloc(sizeof(key_timer_param_t));
    }
    
    if (param != NULL) {
        param->ec = eq->ec;
        param->local_id = eq->attr.local_id;
        /*第一次，直接刷新*/
        CL_THREAD_TIMER_ON(&cl_priv->master, eq->t_key_query, timer_key_query, (void *)param, 0);
    }
}

static void ekey_do_action(equipment_t* eq,remote_key_t* key,u_int32_t type,
                           cln_key_info_t* ki,RS* ret)
{
    net_remote_key_t * nk;
    remote_key_attri_t* rk;
    pkt_t * pkt;
    equipment_ctrl_t* ec = eq->ec;
    
    
    
    pkt = pkt_new_v2(CMD_REMOTE_KEY_CONFIG, sizeof(*nk)+sizeof(*rk), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     ec->slave->sn, ec->slave->user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_KEY_CONFIG, NULL, callback_eq_request);
	nk = get_pkt_payload(pkt, net_remote_key_t);
    rk = nk->attri;
    
	nk->count = 0x1;
    nk->local_id = htons(eq->attr.local_id);
    
    if (type == CLNE_EKEY_ADD) {
        nk->action = AC_ADD;
    }else if(type == CLNE_EKEY_DEL){
        nk->action = AC_DEL;
    }else if(type == CLNE_EKEY_CH_NAME){
        nk->action = AC_MOD;
        // 报警设备单独处理
        if (ki->key_id==KEYID_ALARM_KEY && !key) {
            nk->action = AC_ADD;
        }
    }
    
	rk->key_id = htonl(ki->key_id);
    strncpy((char*)rk->name, ki->key_name, sizeof(rk->name)-1);

	user_add_pkt(ec->slave->user, pkt);
    
    ekey_do_query_immediately(eq);
}

static void ekey_send_ctrl_singal(equipment_t* eq,remote_key_t* key,u_int32_t type,
                           cln_key_info_t* ki,RS* ret)
{
  
    pkt_t * pkt;
    equipment_ctrl_t* ec = eq->ec;
    net_remote_ctrl_t* nrc;
    int plen = sizeof(*nrc);
    u_int16_t state;
    scene_priv_t* scene;

   if(isAlarmDevice(eq)){
   	*ret = RS_INVALID_PARAM;
        log_err(false, "ekey_send_ctrl_singal the alarm device can not ctrl  handle=%u\n", eq->handle);
        return ;
   }

    if(!key->key_attr.valid){
	*ret = RS_INVALID_PARAM;
        log_err(false, "ekey_send_ctrl_singal the key has not learn key_id =%u\n", key->key_attr.key_id);
        return ;
    }
    
    if (is_scene_remote_ctrl(eq)) {
        scene = scene_lookup_by_id(ec->slave->user->sc, ki->key_id);
        if (scene) {
            *ret = scene_proc_exec(scene->scene_handle);
        }
        return;
    }
    
    if (is_rf2_equipment(eq))
        plen += sizeof(remote_state_t);
    
    pkt = pkt_new_v2(CMD_REMOTE_CTRL, plen, NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     ec->slave->sn, ec->slave->user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_CTRL, NULL, callback_eq_request);
    nrc = get_pkt_payload(pkt, net_remote_ctrl_t);
    nrc->key_id = htonl(ki->key_id);
    nrc->local_id = htons(eq->attr.local_id);
    /*双向RF 灯和插座，state参数sid 11， value 1表示开*/
    if (is_rf2_equipment(eq)) {
        nrc->state_num = 1;
        nrc->states[0].state_id = ntohs(STSTEID_DB_RF_CTRL);
        state = get_rf_2_state(eq);
        if (IS_KEY_STATE_ON(ki->key_id, state)) 
            nrc->states[0].state_value = ntohs(0);
        else
            nrc->states[0].state_value = ntohs(1);
    }
    user_add_pkt(ec->slave->user, pkt);
    if(is_rf2_stat_equipment(eq)){
        eq_force_update_all_info(ec);
    }
	
}

static void ekey_set_db_dimming_lamp(equipment_t* eq,remote_key_t* key,u_int32_t type,
                                  cln_key_info_t* ki,RS* ret)
{
    
    pkt_t * pkt;
    equipment_ctrl_t* ec = eq->ec;
    net_remote_ctrl_t* nrc;
    int plen = sizeof(*nrc) + sizeof(remote_state_t);
    u_int16_t state;
    
   //有效性在入口已经检查了
    pkt = pkt_new_v2(CMD_REMOTE_CTRL, plen, NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     ec->slave->sn, ec->slave->user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, ec->slave->handle, CMD_REMOTE_CTRL, NULL, callback_eq_request);
    nrc = get_pkt_payload(pkt, net_remote_ctrl_t);
    nrc->key_id = htonl(ki->key_id);
    nrc->local_id = htons(eq->attr.local_id);
    /*双向RF 灯和插座，state参数sid 11， value 1表示开*/
    nrc->state_num = 1;
    nrc->states[0].state_id = ntohs(STATEID_DB_DIMMING);
    state = ki->db_dimm_value;
    nrc->states[0].state_value = htons(state);
    
    user_add_pkt(ec->slave->user, pkt);
    // TODO: 有无必要，是否可以更新单个?
    eq_force_update_all_info(ec);
}

static bool ekey_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_key_info_t * ki;
    equipment_t* eq = NULL;
    remote_key_t* key = NULL;
    
    ki = (cln_key_info_t *)&pkt->data[0];
    eq = lookup_by_handle(HDLT_EQUIPMENT, ki->eq_handle);
    if (!eq) {
        *ret = RS_NOT_FOUND;
        log_err(false, "ekey_proc_notify can not found equipment handle 0X%08X\n", ki->eq_handle);
        return true;
    }
    
    if (pkt->type == CLNE_EQ_SET_DIMMING_LAMP) {
        if (eq->attr.dev_type != REMOTE_TYPE_BD_DIMMING_LAMP) {
            *ret = RS_INVALID_PARAM;
            log_err(false, "ekey_proc_notify CLNE_EQ_SET_DIMMING_LAMP invalid eq 0X%08X; type error!\n", ki->eq_handle);
            return true;
        }
        key = look_up_db_first_key(eq);
        if (!key || !key->key_attr.valid) {
            log_err(false, "ekey_proc_notify CLNE_EQ_SET_DIMMING_LAMP invalid eq 0X%08X; has no key!\n", ki->eq_handle);
            return true;
        }
        ki->key_id = key->key_attr.key_id;
    }else if (pkt->type!=CLNE_EKEY_ADD) {
        key = rk_lookup(eq, ki->key_id);
        if (!key && ki->key_id != KEYID_ALARM_KEY) {
            *ret = RS_NOT_FOUND;
            log_err(false, "ekey_proc_notify can not found key  id= 0X%08X\n", ki->key_id);
            return true;
        }
    }
    
    if (pkt->type == CLNE_EKEY_ADD && isAlarmDevice(eq) && ki->key_id!= KEYID_ALARM_KEY) {
        *ret = RS_INVALID_PARAM;
        log_err(false, "ekey_proc_notify alarm device can not create key!");
        return true;
    }
    
    /*SDK API 入口处已检查了名字长度,后续不考虑*/
    switch (pkt->type) {
        case CLNE_EKEY_ADD:
        case CLNE_EKEY_DEL:
        case CLNE_EKEY_CH_NAME:
            ekey_do_action(eq,key,pkt->type,ki,ret);
            break;
        case CLNE_EKEY_CTRL:
            ekey_send_ctrl_singal(eq,key,pkt->type,ki,ret);
            break;
        case CLNE_EQ_SET_DIMMING_LAMP:
            ekey_set_db_dimming_lamp(eq, key, pkt->type, ki, ret);
            break;
            
        default:
            return false;
            break;
    }

    return true;
}
/*********************************************************************************/


static void kl_enter_stat(user_t* user,u_int16_t stat)
{
    u_int16_t time_out = 0;
    switch (stat) {
        case KL_STAT_INIT:
        case KL_STAT_READY:
            break;
        case KL_STAT_WAIT_LEARN_USER_INPUT:
        case KL_STAT_WAIT_TRY_USER_INPUT:
            time_out = user->kl->learn_remain_time = LEARNING_TIME_OUT;
            break;
        case KL_STAT_WAIT_STOP_RES:
            time_out = 1;
            break;
        default:
            time_out = 5;
            break;
    }
    user->kl->learn_stat = (u_int8_t)stat;
    kl_reset_timer(user,time_out);
}

static void callback_kl_request(u_int32_t result, void *none, void *waitp)
{
    
}

int kl_timer_handler(cl_thread_t *t)
{
    user_t* user = (user_t*)CL_THREAD_ARG(t);
    u_int32_t event = EE_KL_HOST_WAIT_TIME;
    int lasterr;
	
    user->kl->t_learn = NULL;
    
    if (user->kl->learn_stat == KL_STAT_WAIT_TRY_USER_INPUT||
        user->kl->learn_stat == KL_STAT_WAIT_LEARN_USER_INPUT) {
        if (--user->kl->learn_remain_time>0) {
            
            if (user->kl->learn_stat == KL_STAT_WAIT_TRY_USER_INPUT) {
                event = EE_KL_TRY_WAIT_TIME;
            }
            event_push(user->kl->callback, event, user->handle, user->kl->callback_handle);
            CL_THREAD_TIMER_ON(&cl_priv->master, user->kl->t_learn, kl_timer_handler, (void*)user, 1000);
            return RS_OK;
        }
    }
    
    if (user->kl->learn_stat == KL_STAT_WAIT_STOP_RES) {
        event_push(user->kl->callback, EE_KL_STOP_OK, user->handle, user->kl->callback_handle);
    }else{
        lasterr = ERR_NONE;
        switch (user->kl->learn_stat) {
            case KL_STAT_WAIT_LEARN_RES:
                lasterr = ERR_KL_NOTIFY_DEV_TO_LEARN_TIME_OUT;
                break;
            case KL_STAT_WAIT_LEARN_USER_INPUT:
                lasterr = ERR_KL_WAIT_USER_INPUT_TIME_OUT;
                break;
            case KL_STAT_WAIT_TRY_USER_INPUT:
                lasterr = ERR_KL_RECV_ALARM_TIME_OUT;
                break;
            case KL_STAT_WAIT_TRY_PACKET:
                lasterr = ERR_KL_WAIT_TRY_RES_TIME_OUT;
                break;
            case KL_STAT_WAIT_GEN_RES:
                lasterr = ERR_KL_GEN_CODE_TIME_OUT;
                break;
            case KL_STAT_WAIT_STOP_RES:
                lasterr = ERR_KL_STOP_TIME_OUT;
                break;
            case KL_STAT_WAIT_AJUST_RES:
                lasterr = ERR_KL_RF_AJUST_TIMEOUT;
                break;
            case KL_STAT_WAIT_PLW_RES:
                lasterr = ERR_KL_PLUS_WIDTH_SET_TIME_OUT;
                break;
            case KL_STAT_WAIT_SAVE:
                lasterr = ERR_KL_SAVE_FAIL;
                break;
            default:
                break;
        }
        if (lasterr!=ERR_NONE) {
            user->kl->last_err = lasterr;
            event_push_err(user->kl->callback, EE_KL_ERROR, user->handle, user->kl->callback_handle, lasterr);
        }
    }

    kl_enter_stat(user, KL_STAT_READY);
    return RS_OK;
}

static void kl_reset_timer(user_t* user,u_int16_t time_out)
{
    CL_THREAD_OFF(user->kl->t_learn);
    if (time_out>0) {
        if (user->kl->learn_stat == KL_STAT_WAIT_TRY_USER_INPUT||
            user->kl->learn_stat == KL_STAT_WAIT_LEARN_USER_INPUT) {
            CL_THREAD_TIMER_ON(&cl_priv->master, user->kl->t_learn, kl_timer_handler, (void*)user, 0);
        }else{
            CL_THREAD_TIMER_ON(&cl_priv->master, user->kl->t_learn, kl_timer_handler, (void*)user,
                               time_out*1000);
        }
    }
}

static void kl_clear_code(user_t* user,u_int32_t mask)
{
    remote_key_learn_t* kl = user->kl;
    
    cl_assert(kl);
    if (mask & KLC_LEARN_CODE) {
        kl->is_support_ajust = false;
        kl->learn_code_len =
        kl->learn_code_type = 0x0;
        SAFE_FREE(kl->learn_code);
    }
    
    if (mask & KLC_AJUST_CODE) {
        
        kl->ajust_range =
        kl->ajust_code_len =
        kl->ajust_code_type = 0x0;
        SAFE_FREE(kl->ajust_code);
        
    }
    
    if (mask & KLC_BASE_CODE) {
        kl->base_code_len =
        kl->base_code_type = 0x0;
        SAFE_FREE(kl->base_code);
    }
    
    if (mask & KLC_GEN_CODE) {
        kl->host_gen_code_len =
        kl->host_gen_code_type = 0x0;
        SAFE_FREE(kl->host_gen_code);
    }
}

pkt_t* kl_mk_packet(equipment_t* eq,remote_key_t* rk,u_int8_t action,
                    u_int8_t* code,u_int16_t code_len,u_int16_t code_type)
{
    pkt_t *pkt;
    net_remote_code_t *nrc;
    
    /*因为报文里面有电器id和keyid,所以统一使用电器作为回调句柄*/
    pkt = pkt_new_v2(CMD_REMOTE_CODE, sizeof(net_remote_code_t)+code_len,
                     NHF_TRANSPARENT|NHF_WAIT_REPLY, eq->ec->slave->sn, eq->ec->slave->user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, eq->ec->slave->handle, CMD_REMOTE_CODE, NULL, callback_kl_request);
    nrc = get_pkt_payload(pkt, net_remote_code_t);
    memset(nrc, 0, sizeof(*nrc));
    nrc->action = action;
    nrc->key_id = htonl(rk->key_attr.key_id);
    nrc->local_id = htons(eq->attr.local_id);
    
    if (action == AC_KEYCODE_LEARN) {
        nrc->time_out = LEARNING_TIME_OUT/10;
    }
    
    if (code_len == 0 && action == AC_KEYCODE_TRY) {
        nrc->time_out = LEARNING_TIME_OUT/10;
    }
    
    if (code && code_len>0) {
        nrc->code_len = htons(code_len);
        nrc->code_type = htons(code_type);
        memcpy(nrc->code, code, code_len);
    }
    
    return pkt;
}

pkt_t* kl_mk_ajust_plus_width_pkt(equipment_t* eq,remote_key_t* rk,u_int8_t is_query,u_int16_t pw)
{
    pkt_t* pkt;
    net_remote_code_t* nrc;
    
    pkt = kl_mk_packet(eq, rk, AC_KEYCODE_PLUS_WIDTH_AJUST, NULL, 0, 0);
    if (pkt && !is_query) {
        nrc = (net_remote_code_t*)get_pkt_payload(pkt, net_remote_code_t);
        nrc->time_out = 0x1;
        nrc->code_len = htons(pw);
    }
    return pkt;
}

static void kl_do_start_learn(user_t* user,equipment_t* eq,remote_key_t* rk)
{
    pkt_t* pkt;
    cl_assert(KL_STAT_EQ(user->kl->learn_stat,KL_STAT_READY));
    
    /*开始学习,以前的数据都没有用了*/
    kl_clear_code(user,KLC_LEARN_CODE|KLC_AJUST_CODE|KLC_GEN_CODE|KLC_BASE_CODE);
    /*主机主动学习*/
    if (user->kl->learn_mode == KLM_LEARN_CODE_BY_HOST) {
        pkt = kl_mk_packet(eq,rk,AC_KEYCODE_LEARN,NULL,0,0);
        if (pkt) {
            user_add_pkt(user, pkt);
            kl_enter_stat(user, KL_STAT_WAIT_LEARN_RES);
        }
    }
}

static void kl_proc_start_learn(user_t* user,cln_key_learn_t * klp,RS* ret)
{
    equipment_t* eq;
    remote_key_t* rk;
    
    if (user->kl->learn_stat!=KL_STAT_READY) {
        log_err(false, "kl_proc_start_learn must call cl_key_learn_stop at first!\n");
		*ret = RS_NOT_SUPPORT;
		return;
    }
    
    eq = (equipment_t*)lookup_by_handle(HDLT_EQUIPMENT, klp->eq_handle);
    if (!eq) {
        log_err(false, "kl_proc_start_learn failed: not found equipment handle=0x%08x\n", klp->eq_handle);
		*ret = RS_NOT_FOUND;
		return ;
    }
    
    if (isAlarmDevice(eq)) {
        klp->key_id = KEYID_ALARM_KEY;
    }
    
    rk = rk_lookup(eq, klp->key_id);
    if (!rk) {
        log_err(false, "kl_proc_start_learn failed: not found key id=0x%08x\n", klp->key_id);
        *ret = RS_NOT_FOUND;
        return ;
    }
    /*检测不允许对码的电器*/
    if(klp->learn_mode == KLM_LEARN_CODE_BY_EQUIPMENT){
        /*不允许对码*/
        if (eq->attr.dev_type<=REMOTE_TYPE_WIRELESS_BASE||
            eq->attr.dev_type == REMOTE_TYPE_ALARM) {
            log_err(false, "kl_proc_start_learn can not support EQ learn code mode when dev_type= %u\n", eq->attr.dev_type);
            *ret = RS_INVALID_PARAM;
            return;
        }
    }
    
    user->kl->eq_handle = klp->eq_handle;
    user->kl->key_id = klp->key_id;
    user->kl->learn_mode = klp->learn_mode;
    /*初始化完成,可以学习了*/
    kl_enter_stat(user,KL_STAT_READY);
    kl_do_start_learn(user, eq, rk);
}

static void kl_proc_save_code(user_t* user,equipment_t* eq,remote_key_t* rk,cln_key_learn_t * klp,RS* ret)
{
    pkt_t* pkt;
    u_int8_t* code;
    u_int16_t code_type;
    u_int16_t code_len;
    
    if (user->kl->learn_mode == KLM_LEARN_CODE_BY_EQUIPMENT) {
        code = user->kl->host_gen_code;
        code_len = user->kl->host_gen_code_len;
        code_type = user->kl->host_gen_code_type;
    }else{
        code_len = user->kl->learn_code_len;
        code_type = user->kl->learn_code_type;
        code = user->kl->learn_code;
        if (user->kl->ajust_code && user->kl->ajust_code_len>0) {
            code = user->kl->ajust_code;
            code_type = user->kl->ajust_code_type;
            code_len = user->kl->ajust_code_len;
        }
    }
    
    if (!code||code_len==0||!rk) {
        log_err(false, "kl_proc_save failed: NO code! code %p code_len %d\n",code,code_len);
        if (ret) {
            *ret = RS_INVALID_PARAM;
        }
        return;
    }
    
    if (rk->key_attr.valid) {
        pkt = kl_mk_packet(eq,rk,AC_KEYCODE_MODIFY,code,code_len,code_type);
    }else{
        pkt = kl_mk_packet(eq,rk,AC_KEYCODE_ADD,code,code_len,code_type);
    }
    
    user_add_pkt(user, pkt);
    kl_enter_stat(user, KL_STAT_WAIT_SAVE);
    
}

/*
 如果是报警器，需要倒计时，否者设备发尝试报文
 */
static void kl_proc_try(user_t* user,equipment_t* eq,remote_key_t* rk,cln_key_learn_t * klp,RS* ret)
{
    pkt_t* pkt;
    u_int8_t* code;
    u_int16_t code_type;
    u_int16_t code_len;
    
    
    /*直接发,主机等待收报警信号*/
    if (user->kl->learn_mode == KLM_LEARN_CODE_BY_EQUIPMENT) {
        code = user->kl->host_gen_code;
        code_len = user->kl->host_gen_code_len;
        code_type = user->kl->host_gen_code_type;
    }else{
        code_len = user->kl->learn_code_len;
        code_type = user->kl->learn_code_type;
        code = user->kl->learn_code;
        if (user->kl->ajust_code && user->kl->ajust_code_len>0) {
            code = user->kl->ajust_code;
            code_type = user->kl->ajust_code_type;
            code_len = user->kl->ajust_code_len;
        }
    }
    
    if (!code||code_len==0) {
        log_err(false, "kl_proc_try failed: NO code! code %p code_len %d\n",code,code_len);
        *ret = RS_INVALID_PARAM;
        return;
    }
    
    
    if (isAlarmDevice(eq)) {
        pkt = kl_mk_packet(eq,rk,AC_KEYCODE_TRY,NULL,0,0);
        user_add_pkt(user, pkt);
        kl_enter_stat(user, KL_STAT_WAIT_TRY_USER_INPUT);
    }else{
        pkt = kl_mk_packet(eq,rk,AC_KEYCODE_TRY,code,code_len,code_type);
        user_add_pkt(user, pkt);
        kl_enter_stat(user, KL_STAT_WAIT_TRY_PACKET);
    }
}

static void kl_proc_gen_code(user_t* user,equipment_t* eq,remote_key_t* rk,cln_key_learn_t * klp,RS* ret)
{
    pkt_t* pkt;
    
    if (user->kl->learn_mode!=KLM_LEARN_CODE_BY_EQUIPMENT) {
        log_err(false, "kl_proc_gen_code failed: learn mode error\n",user->kl->learn_mode);
        *ret = RS_INVALID_PARAM;
        return;
    }
    
    pkt = kl_mk_packet(eq,rk,AC_KEYCODE_GEN_CODE,NULL,0,0);
    if (pkt) {
        user_add_pkt(user, pkt);
        kl_enter_stat(user, KL_STAT_WAIT_GEN_RES);
    }
   
}

static void kl_proc_ajust_code(user_t* user,equipment_t* eq,remote_key_t* rk,cln_key_learn_t * klp,RS* ret)
{
    pkt_t *pkt;
    net_remote_code_t* nrc;
    remote_key_learn_t* kl = user->kl;
    
    //必须支持调整,发探测是否支持调整时已判断电器类型
    if (kl->learn_mode!=KLM_LEARN_CODE_BY_HOST||
        !kl->is_support_ajust||
        !kl->base_code||
        kl->base_code_len==0||
        klp->ajust_value<(int)-kl->ajust_range||
        klp->ajust_value>(int)kl->ajust_range) {
        log_err(false, "kl_proc_ajust_code failed: learn mode %u,is_support_ajust %u base code %p, base code len %u\n",kl->learn_mode,kl->is_support_ajust,kl->base_code,kl->base_code_len);
        *ret = RS_INVALID_PARAM;
        return;
    }
    
    
    pkt = kl_mk_packet(eq,rk,AC_KEYCODE_ADJUST,user->kl->base_code,user->kl->base_code_len,user->kl->base_code_type);
    if (!pkt) {
        return;
    }
    nrc = (net_remote_code_t*)get_pkt_payload(pkt, net_remote_code_t);
    nrc->err = (u_int32_t)(klp->ajust_value);
    nrc->err = htonl(nrc->err);
    user_add_pkt(user, pkt);
    kl_enter_stat(user, KL_STAT_WAIT_AJUST_RES);
}


static void kl_proc_ajust_plus_width(user_t* user,equipment_t* eq,remote_key_t* rk,
                                     cln_key_learn_t * klp,RS* ret)
{
    pkt_t *pkt;
    u_int16_t pw_value = 0;
    
    //仅无线 对码模式支持脉宽
    if (isAlarmDevice(eq)||is_infr_equipment(eq)||(user->kl->learn_mode == KLM_LEARN_CODE_BY_HOST)) {
        log_err(false, "kl_proc_ajust_plus_width failed: do not support plus width fuction\n");
        *ret = RS_INVALID_PARAM;
        return;
    }
    
    pw_value = klp->ajust_pw_value;
    pkt = kl_mk_ajust_plus_width_pkt(eq,rk,false,pw_value);
    if (pkt) {
        user_add_pkt(user, pkt);
        kl_enter_stat(user, KL_STAT_WAIT_PLW_RES);
    }
}

static void kl_proc_query_plus_width(user_t* user,equipment_t* eq,remote_key_t* rk,
                                     cln_key_learn_t * klp,RS* ret)
{
    pkt_t *pkt;
    
    //仅无线 对码模式支持脉宽
    if (isAlarmDevice(eq)||is_infr_equipment(eq)||(user->kl->learn_mode == KLM_LEARN_CODE_BY_HOST)) {
        log_err(false, "kl_proc_query_plus_width failed: do not support plus width fuction\n");
        *ret = RS_INVALID_PARAM;
        return;
    }
    
    pkt = kl_mk_ajust_plus_width_pkt(eq,rk,true,0);
    if (pkt) {
        user_add_pkt(user, pkt);
        kl_enter_stat(user, KL_STAT_WAIT_PLW_RES);
    }
}

static void kl_direct_query_plus_width(user_t* user,equipment_t*eq,u_int32_t key_id)
{
    pkt_t *pkt;
    remote_key_t* rk;
    
    rk = rk_lookup(eq, key_id);
    if (!rk) {
        return;
    }
    
    pkt = kl_mk_ajust_plus_width_pkt(eq,rk,true,0);
    if (pkt) {
        user_add_pkt(user, pkt);
    }
}

static void kl_proc_set_callback(user_t* user,cln_key_learn_t * klp,RS* ret)
{
    user->kl->callback = klp->callback;
    user->kl->callback_handle = klp->callback_handle;
    if (user->kl->learn_stat == KL_STAT_INIT) {
        user->kl->learn_stat = KL_STAT_READY;
    }
}

static void kl_proc_stop_learn(user_t* user,equipment_t* eq,remote_key_t* rk,cln_key_learn_t * klp,RS* ret)
{
    pkt_t * pkt;
    cl_callback_t callback = user->kl->callback;
    void* calp = user->kl->callback_handle;
    
    if (user->kl->learn_stat == KL_STAT_INIT) {
        return;
    }
    
    _key_learn_free(user->kl,false);
    memset(user->kl, 0, sizeof(remote_key_learn_t));
    user->kl->callback = callback;
    user->kl->callback_handle = calp;
    
    pkt = kl_mk_packet(eq,rk,AC_KEYCODE_STUDYSTOP,NULL,0,0);
    if (!pkt) {
        return;
    }

    user_add_pkt(user, pkt);
    kl_enter_stat(user, KL_STAT_READY);
}

static bool kl_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_key_learn_t * klp;
    user_t* user;
    equipment_t* eq = NULL;
    remote_key_t* rk = NULL;
    
    klp = (cln_key_learn_t*)&pkt->data[0];
    user = (user_t *)lookup_by_handle(HDLT_USER, klp->user_handle);
	if (user == NULL) {
		log_err(false, "kl_proc_notify failed: not found user handle=0x%08x\n", klp->user_handle);
		*ret = RS_NOT_FOUND;
		return true;
	}
    log_debug("kl_proc_notify %s %u",__FUNCTION__,pkt->type);
    /*统一检查回调函数*/
    if (pkt->type!=CLNE_KL_SET_CALLBACK && !user->kl->callback) {
        log_err(false, "kl_proc_notify must set callback at first!\n");
		*ret = RS_NOT_SUPPORT;
		return true;
    }
    
    /*除去初始化的两个函数，其他均需要找到对应的电器和key*/
    if (!(pkt->type==CLNE_KL_SET_CALLBACK||
        pkt->type == CLNE_KL_START_LEARN))
    {
        
        eq = (equipment_t*)lookup_by_handle(HDLT_EQUIPMENT, user->kl->eq_handle);
        if (!eq) {
            log_err(false, "kl_proc_notify failed: not found equipment handle=0x%08x\n", klp->eq_handle);
            *ret = RS_NOT_FOUND;
            return true;
        }
        
        rk = rk_lookup(eq, user->kl->key_id);
        if (!rk) {
            log_err(false, "kl_proc_notify failed: not found key key_id=0x%08x\n", klp->key_id);
            *ret = RS_NOT_FOUND;
            return true;
        }
    }
    
    if (user->kl) {
        user->kl->last_err = ERR_NONE;
    }
    
    switch (pkt->type) {
        case CLNE_KL_SET_CALLBACK:
            kl_proc_set_callback(user,klp,ret);
            break;
        case CLNE_KL_START_LEARN:
            kl_proc_start_learn(user, klp, ret);
            break;
        case CLNE_KL_TRY:
            kl_proc_try(user,eq,rk, klp, ret);
            break;
        case CLNE_KL_GEN_CODE:
            kl_proc_gen_code(user,eq,rk,klp, ret);
            break;
        case CLNE_KL_AJUST_CODE:
            kl_proc_ajust_code(user,eq,rk, klp, ret);
            break;
        case CLNE_KL_SET_PW:
            kl_proc_ajust_plus_width(user,eq,rk, klp, ret);
            break;
        case CLNE_KL_GET_PW:
            kl_proc_query_plus_width(user,eq,rk, klp, ret);
            break;
        case CLNE_KL_STOP_LEARN:
            kl_proc_stop_learn(user,eq,rk, klp, ret);
            break;
        case CLNE_KL_SAVE_CODE:
            kl_proc_save_code(user, eq, rk, klp, ret);
            break;
        default:
            return false;
            break;
    }
    return true;
}



static void kl_send_probe_packet(user_t* user,equipment_t* eq)
{
    pkt_t* pkt;
    remote_key_t* ekey;
    remote_key_learn_t* kl = user->kl;
    
    ekey =rk_lookup(eq, user->kl->key_id);
    if (!ekey||!kl||!kl->learn_code||kl->learn_code_len<=0) {
        return;
    }
    
    pkt = kl_mk_packet(eq, ekey, AC_KEYCODE_PROBE, kl->learn_code,kl->learn_code_len, kl->learn_code_type);
    if (pkt) {
        user_add_pkt(user, pkt);
    }
    
}

static bool do_kl_learn_ok_pkt(user_t* user,equipment_t* eq,pkt_t *pkt,
                               net_remote_code_t *nrc,remote_key_learn_t * kl)
{
    remote_key_t* rk;
    if (nrc->code_len>0 && net_param_len(pkt->data)>=sizeof(net_remote_code_t)+nrc->code_len) {
        kl_clear_code(user, KLC_LEARN_CODE);
        kl->learn_code = cl_calloc(nrc->code_len, 1);
        if (kl->learn_code) {
            memcpy(kl->learn_code, nrc->code, nrc->code_len);
            kl->learn_code_len = nrc->code_len;
            kl->learn_code_type = nrc->code_type;
            //只有无线，主机学习电器编码模式下才支持微调
            if (is_rf_equipment(eq) &&
                !is_rf2_equipment(eq)
                && kl->learn_mode == KLM_LEARN_CODE_BY_HOST) {
                kl_send_probe_packet(user, eq);
            }
            //如果是报警类设备，后台存储
            if (isAlarmDevice(eq)) {
                rk = rk_lookup(eq, nrc->key_id);
                if (rk) {
                    kl_proc_save_code(user,eq,rk,NULL,NULL);
                }
            }
            return true;
        }
    }
    return false;
}

static bool do_kl_gen_code_ok_pkt(user_t* user,equipment_t* eq,pkt_t *pkt,
                               net_remote_code_t *nrc,remote_key_learn_t * kl)
{
    if (nrc->code_len>0 && net_param_len(pkt->data)>=sizeof(net_remote_code_t)+nrc->code_len) {
        kl_clear_code(user, KLC_GEN_CODE);
        kl->host_gen_code = cl_calloc(nrc->code_len, 1);
        if (kl->host_gen_code) {
            memcpy(kl->host_gen_code, nrc->code, nrc->code_len);
            kl->host_gen_code_len = nrc->code_len;
            kl->host_gen_code_type = nrc->code_type;
            return true;
        }
        
    }
    return false;
}

static bool do_kl_probe_ok_pkt(user_t* user,equipment_t* eq,pkt_t *pkt,
                               net_remote_code_t *nrc,remote_key_learn_t * kl)
{
    if (nrc->code_len>0 && net_param_len(pkt->data)>=sizeof(net_remote_code_t)+nrc->code_len) {
        kl_clear_code(user, KLC_BASE_CODE|KLC_AJUST_CODE);
        kl->is_support_ajust = false;
        kl->base_code = cl_calloc(nrc->code_len, 1);
        if (kl->base_code) {
            kl->ajust_range = nrc->time_out;
            memcpy(kl->base_code, nrc->code, nrc->code_len);
            kl->base_code_len = nrc->code_len;
            kl->base_code_type = nrc->code_type;
            if (kl->ajust_range>0) {
                kl->is_support_ajust = true;
                return true;
            }
        }
    }
    return false;
}

static bool do_kl_ajust_ok_pkt(user_t* user,equipment_t* eq,pkt_t *pkt,
                               net_remote_code_t *nrc,remote_key_learn_t * kl)
{
    if (nrc->code_len>0 && net_param_len(pkt->data)>=sizeof(net_remote_code_t)+nrc->code_len) {
        kl_clear_code(user, KLC_AJUST_CODE);
        kl->ajust_code = cl_calloc(nrc->code_len, 1);
        if (kl->ajust_code) {
            memcpy(kl->ajust_code, nrc->code, nrc->code_len);
            kl->ajust_code_len = nrc->code_len;
            kl->ajust_code_type = nrc->code_type;
            return true;
        }
    }
    return false;
}


static RS do_key_learn_config(equipment_ctrl_t *ec, pkt_t *pkt)
{
    net_remote_code_t *nrc;
	RS ret = RS_OK;
    user_t* user = ec->slave->user;
    equipment_t* eq = NULL;
    u_int32_t event = 0;
    remote_key_learn_t * kl = user->kl;
    bool is_ok;
    
	if (net_param_len(pkt->data) < sizeof(net_remote_code_t)) {
		log_err(false, "do_key_learn_config failed: param len=%d\n", net_param_len(pkt->data));
		return RS_ERROR;
	}
    
    nrc = get_pkt_payload(pkt, net_remote_code_t);
    nrc->err = ntohl(nrc->err);
    nrc->key_id = ntohl(nrc->key_id);
    nrc->local_id = ntohs(nrc->local_id);
    nrc->code_len = ntohs(nrc->code_len);
    nrc->code_type = ntohs(nrc->code_type);
    
    if (nrc->action!=AC_KEYCODE_ADJUST) {
        eq = eq_lookup(ec, nrc->local_id);
        if (!eq) {
            log_err(false, "do_key_learn_config can not find equipment local_id=%d action %u\n",
                    nrc->local_id,nrc->action);
            return RS_NOT_FOUND;
        }
    }
    
    log_debug("do_key_learn_config err %u action %u key_id %u local_id %u code len %u\n",
              nrc->err,nrc->action,nrc->key_id,nrc->local_id,nrc->code_type);
    if (nrc->action!=AC_KEYCODE_PROBE && nrc->err!=ERR_NONE) {
        //出错了,设备忙，通知上层
        kl->last_err = ERR_KL_DEVICE_BUSY_OR_OFFLINE;
        if (nrc->err == ERR_DUPLICATE_REMOTE_CODE) {
            kl->last_err = ERR_KL_DUPLICATE_REMOTE_CODE;
        }

        kl_enter_stat(ec->slave->user, KL_STAT_READY);
        event_push_err(user->kl->callback, EE_KL_ERROR,user->handle, user->kl->callback_handle, nrc->err);
        return RS_OK;
    }
    
    
    switch (nrc->action) {
        case AC_KEYCODE_LEARN:
            kl_enter_stat(user, KL_STAT_WAIT_LEARN_USER_INPUT);
            return RS_OK;
            break;
        case AC_KEYCODE_ADD:
        case AC_KEYCODE_MODIFY:
        {
            if (eq) {
                ekey_do_query_immediately(eq);
            }
            event = EE_KL_SAVE_CODE_OK;
        }
            break;
        case AC_KEYCODE_DELETE:
            break;
        case AC_KEYCODE_GEN_CODE:
        {
            if (do_kl_gen_code_ok_pkt(user,eq,pkt,nrc,kl)) {
                event = EE_KL_GEN_CODE_OK;
                kl_direct_query_plus_width(user,eq,nrc->key_id);
            }else{
                user->kl->last_err = ERR_KL_GEN_CODE_TIME_OUT;
                event = EE_KL_ERROR;
            }
        }
            break;
        case AC_KEYCODE_STUDYOK:/*学习到编码*/
        {
            event = EE_KL_LEARN_OK;
            do_kl_learn_ok_pkt(user,eq,pkt,nrc,kl);
        }
            break;
        case AC_KEYCODE_STUDYSTOP:/*停止学习*/
            event = EE_KL_STOP_OK;
            break;
        case AC_KEYCODE_TRY:
            if (isAlarmDevice(eq)) {
                event = EE_KL_RECV_ALARM_OK;
            }else{
                event = EE_KL_TRY_SEND_OK;
            }
            break;
        case AC_KEYCODE_PROBE:
        {
            if (do_kl_probe_ok_pkt(user,eq,pkt,nrc,kl))
            {
                event = EE_KL_RF_SUPPORT_AJUST;
            }
        }
            break;
        case AC_KEYCODE_ADJUST:
        {
            is_ok = do_kl_ajust_ok_pkt(user,eq,pkt,nrc,kl);
            if (is_ok) {
                event = EE_KL_RF_AJUST_OK;
            }else{
                kl->last_err = ERR_KL_RF_AJUST_TIMEOUT;
                event = EE_KL_ERROR;
            }
        }
            break;
        case AC_KEYCODE_PLUS_WIDTH_AJUST:
        {
            if (!nrc->time_out) {
                kl->ajust_pw_max = nrc->code_type;
                kl->ajust_current_value = nrc->code_len;
                event = EE_KL_RF_SUPPORT_PLUS_WIDTH;
            }else{
                kl->ajust_current_value = nrc->code_len;
                event = EE_KL_PLUS_WIDTH_AJUST_OK;
            }
            break;
        }
        default:
            break;
    }
    
    kl_enter_stat(ec->slave->user, KL_STAT_READY);
    
    if (event>0) {
        event_push(user->kl->callback, event, user->handle, user->kl->callback_handle);
    }
    
    return ret;
}

static u_int32_t eq_get_hand_by_kl_packet(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
    net_remote_code_t *nrc;
    
    if (hdr->command == CMD_REMOTE_CODE &&
        net_param_len(pkt->data) >= sizeof(net_remote_code_t)) {
        nrc = get_pkt_payload(pkt, net_remote_code_t);
        if (nrc->action == AC_KEYCODE_STUDYOK) {
            if (ntohl(nrc->err) == ERR_DUPLICATE_REMOTE_CODE||
                user->kl->key_id == ntohl(nrc->key_id)) {
                return user->kl->eq_handle;
            }
        }
    }
    return 0;
}

/*********************************************************************************/
// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
bool eq_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
//	u_int32_t err_code = 0;
	slave_t *slave;
	equipment_ctrl_t *ec;
    equipment_t* eq;
	wait_t *w;
    cl_handle_t eq_handle;
    u_int32_t param =0;
    
    if (hdr->command == CMD_ALARM_CONFIG_PHONE) {
        alarm_phone_proc_tcp(user, pkt, hdr);
        return true;
    }
    if (hdr->command == CMD_REMOTE_BD_BIND) {
        eq_tcp_remote_db_rf(user, pkt, hdr);
        return true;
    }
	if (hdr->command == CMD_CLOUD_MATCH) {
        do_cmd_cm(user, pkt, hdr);
        return true;
    }

	w = wait_lookup(PKT_HANDLE(pkt));
	if (w == NULL || w->obj_type != HDLT_SLAVE) {
        if ((eq_handle = eq_get_hand_by_kl_packet(user,pkt,hdr))==INVALID_HANDLE) {
            return false;
        }
	}else{
        param = (u_int32_t)w->param;
    }
    
	if (w) {
        slave = slave_lookup_by_handle(user, w->obj_handle);
    }else{
        eq =lookup_by_handle(HDLT_EQUIPMENT, eq_handle);
        if (!eq) {
            return true;
        }
        slave = eq->ec->slave;
    }
	if (slave == NULL) {
		log_err(false, "equipment ignore cmd=%u, not found slave by handle=0x%08x\n", hdr->command, PKT_HANDLE(pkt));
		return false;
	}
    
	ec = slave->equipment;
	log_debug("eq_proc_tcp sn=%s, cmd=%u, handle=0x%08x\n", slave->str_sn, hdr->command, PKT_HANDLE(pkt));

	switch (hdr->command) {
	case CMD_REMOTE_CONFIG:
		do_remote_config(ec, pkt,param);
        break;
	case CMD_REMOTE_TD_CODE:
		do_remote_td_code(ec, pkt,param);
		break;
    case CMD_REMOTE_KEY_CONFIG:
        do_key_config(ec, pkt);
		break;
    case CMD_REMOTE_CODE:
        do_key_learn_config(ec, pkt);
        break;
    case CMD_REMOTE_CTRL:
        do_key_ctrl(ec, pkt);
        break;
    case CMD_REMOTE_STATE:
        break;
    case CMD_ALARM_BIND_PHONE:
        do_alarm_phone_config(slave, pkt);
        break;
	case CMD_REMOTE_CONFIG_SOUNDLIGHT:
		do_remote_soundlight(slave, pkt);
		break;
	
	default:
		return false;
	}

	return true;
}

bool eq_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    bool res = false;
    
    /*利用数据结构，在这里处理了*/
    if (pkt->type == CLNE_USER_ADD_ALARM_PHONE||
        pkt->type == CLNE_USER_DEL_ALARM_PHONE) {
        alarm_phone_proc_notify(pkt,ret);
        return true;
    }
    if (pkt->type == CLNE_EQ_DB_RF_SCAN ) {
        eq_proc_db_rf_scan(pkt, ret);
        return true;
    }
	if (pkt->type == CLNE_EQ_BIND_SOUNDLIGHT) {
        eq_proc_bind_soundlight(pkt, ret);
		return true;
	}
	if (pkt->type == CLNE_EQ_LINKAGE_SCENE_SET || pkt->type == CLNE_EQ_LINKAGE_SCENE_DEL) {
        eq_linkage_scene(pkt, ret);
		return true;
	}
	if (pkt->type <= CLNE_EQ_START || pkt->type >= CLNE_EQ_END) {
		return false;
	}
    
    if (pkt->type>=CLNE_KL_SET_CALLBACK) {
        res = kl_proc_notify(pkt, ret);
    }else if (pkt->type>=CLNE_EKEY_ADD) {
        res = ekey_proc_notify(pkt,ret);
    }else if(pkt->type>=CLNE_EQ_ADD){
        res = _eq_proc_notify(pkt, ret);
    }
    
	return res;
}

static void rk_build_keys(slave_t *slave, equipment_t *eq,cl_equipment_t *e)
{
    remote_key_t* rk;
    cl_key_t* key,**keys;
    int count = 0,index=0;
    
    stlc_list_count(count, &eq->keylist);
    if (count<=0) {
        return;
    }
    
    keys = cl_calloc(sizeof(cl_key_t*), count);
    if (!keys) {
        return;
    }
    
    stlc_list_for_each_entry(remote_key_t, rk, &eq->keylist, link){
        key = cl_calloc(sizeof(cl_key_t), 1);
        if (!key) {
            break;
        }
        key->obj.name = cl_strdup((const char*)rk->key_attr.name);
        key->obj.sn = slave->sn;
        key->obj.status = slave->status;
        key->obj.type = OT_KEY;
        key->had_learned = rk->key_attr.valid;
        key->equipment_handle = eq->handle;
        key->key_id = rk->key_attr.key_id;
        keys[index++] = key;
    }
    e->key_count = index;
    e->keys = keys;
}

static cl_handle_t eq_lookup_linkage_scene(equipment_t *eq)
{
	int i, k;
	user_t *user = eq->ec->slave->user;
	cl_handle_t handle = 0;
	net_scene_linkage_t *nslp;
	scene_linkage_t *slp;
	scene_alarm_t *sa;

	if (user->prev_scene_linkage_len == 0 || user->prev_scene_linkage == NULL)
		goto done;

	nslp = (net_scene_linkage_t *)user->prev_scene_linkage;
	if (nslp->count == 0)
		goto done;
	slp = &nslp->sl[0];
	for (i = 0; i < nslp->count; i++) {
		if (slp->type == SCENE_LINKAGE_TYPE_ALARM && slp->len != 0) {
			for (k = 0; k < slp->len*4; k += sizeof(scene_alarm_t)) {
				sa = (scene_alarm_t *)BOFP(slp, sizeof(*slp) + k);

				log_debug("eq_lookup_linkage_scene eq local_id=%u, <eq=%u <-> scene=%u>\n",
					eq->attr.local_id, ntohs(sa->local_id), slp->scene_id);
				if (ntohs(sa->local_id) == eq->attr.local_id) {
					scene_priv_t* sp;
					if ((sp = scene_lookup_by_id(user->sc, slp->scene_id)) != NULL) {
						handle = sp->scene_handle;
					} else {
						log_err(false, "eq_lookup_linkage_scene, not found scene id=%u\n", slp->scene_id);
					}
					goto done;
				}
			}
		}

		slp = (scene_linkage_t *)BOFP(slp, slp->len*4 + sizeof(*slp));
	}
	
done:
	return handle;
}

static void rk_build_alarm_info(slave_t *slave, equipment_t *eq,cl_equipment_t *e)
{
    cl_alarm_info_t *ai;
    remote_key_t* rk;
    u_int8_t i;
    char* p;

	ai = cl_calloc(sizeof(cl_alarm_info_t), 1);
	e->alarm_info = ai;
    
	if ( ! stlc_list_empty(&eq->keylist) ) {
		rk = stlc_list_first_entry(&eq->keylist, remote_key_t, link);
        if (rk) {
            ai->alarm_msg = cl_strdup((const char*)rk->key_attr.name);

            if (rk->key_attr.valid) {
                ai->isLearned = true;
            }else{
                ai->isLearned = false;
            }
        }
		/* 双向报警器默认为推送和短信可以分开控制 */
		if (slave->has_alarm_swich || eq->attr.dev_type == REMOTE_TYPE_BD_ALARM) { /* 新版本，两个开关，但是又很绞 */
			ai->push_enable = !(!(eq->attr.alarm_flag & ALARM_PUSH));
			ai->sms_enable = ((eq->attr.alarm_flag & 0x3)==ALARM_ENABLED);
		} else { /* 老版本，只有一个开关 */
	        if ((eq->attr.alarm_flag & 0x3)==ALARM_ENABLED) {
	            ai->push_enable = true;
	            ai->sms_enable = true;
	        }else{
	            ai->push_enable = false;
	            ai->sms_enable = false;
	        }
		}
        
        ai->phone_num = 0x0;
        ai->phone_list = NULL;
        if (eq->nbp && eq->nbp->count>0) {
            ai->phone_list = cl_calloc(sizeof(char*)*eq->nbp->count, 1);
            if (ai->phone_list) {
                p = (char*)eq->nbp->phones;
                for (i=0; i<eq->nbp->count; i++,p+=ALARM_PHONE_LENGTH) {
                    ai->phone_list[i] = cl_calloc(ALARM_PHONE_LENGTH, 1);
                    strncpy(ai->phone_list[i], p,ALARM_PHONE_LENGTH-1);
                }
                ai->phone_num = eq->nbp->count;
            }
        }
		
        if (eq->soundlight != NULL && eq->soundlight_len > 0) {
			u_int8_t count;
			equipment_t *eq_sl;

			ai->soundline_on = eq->soundlight->onoff;

			ai->soundline = cl_calloc(sizeof(cl_handle_t), eq->soundlight->count);
			for (i = 0, count = 0; i < eq->soundlight->count; i++) {
				if ((eq_sl = eq_lookup(slave->equipment, ntohs(eq->soundlight->list[i].local_id))) != NULL) {
					ai->soundline[count++] = eq_sl->handle;
				} else {
					log_err(false, "rk_build_alarm_info, not found sound line local id=%u\n", ntohs(eq->soundlight->list[i].local_id));
				}
			}
			ai->soundline_num = count;
        }
	}

	ai->scene = eq_lookup_linkage_scene(eq);
}

static void rk_build_objs(slave_t *slave, equipment_t *eq,cl_equipment_t *e)
{
    switch (eq->attr.dev_type) {
        case REMOTE_TYPE_ALARM:
        case REMOTE_TYPE_BD_ALARM:
            rk_build_alarm_info(slave, eq, e);
            break;
        default:
            rk_build_keys(slave,eq,e);
            break;
    }
}

/***************************************************************************************/

void eq_free_objs(cl_equipment_t *eq)
{
	int i;
	cl_key_t *ck;
	
	for (i = 0; i < eq->key_count; i++) {
		ck = eq->keys[i];
		SAFE_FREE(ck->obj.name);
		SAFE_FREE(ck->obj.priv);
		cl_free(ck);
	}
	SAFE_FREE(eq->keys);

	if (eq->alarm_info != NULL) {
		SAFE_FREE(eq->alarm_info->alarm_msg);
		
		for (i = 0; i < eq->alarm_info->phone_num; i++) {
			SAFE_FREE(eq->alarm_info->phone_list[i]);
		}
		SAFE_FREE(eq->alarm_info->phone_list);
		SAFE_FREE(eq->alarm_info->soundline);
		SAFE_FREE(eq->alarm_info);
	}
}
#if 0
//用于调试，强制中续有功能
#define FORCE_RF_REPEATER_ON(eq) \
do{\
	if (!(eq)->is_rf_repeater) {\
		(eq)->is_rf_repeater = 1;\
		(eq)->rf_repeater_on = 0;\
	}\
}while(0)
#else
#define FORCE_RF_REPEATER_ON(eq) \
do{;}while(0)
#endif
static void eq_set_state(cl_equipment_t *e, remote_atrri_v2_t *attr)
{
	int index;
	u_int16_t value, sid;
	if (e->dev_type == CL_RT_LAMP||
		e->dev_type == CL_RT_PLUG) {
		for (index=0; index<attr->n_state; index++) {
			value = attr->state[index].state_value;
			sid = attr->state[index].state_id;
			switch (sid) {
				case STATEID_OUTLET_GROUPNUM:
					e->group_num = (u_int8_t)value;
					break;
				case STATEID_OUTLET_FUCTION:
					e->is_more_ctrl = !!value;
					break;

				default:
				break;
			}
		}
	} else if (e->dev_type == CL_RT_DB_RF_LAMP ||
			e->dev_type == CL_RT_DB_RF_PLUG ||
            e->dev_type == CL_RT_DB_RF_DIMMING_LAMP) {
		for (index=0; index<attr->n_state; index++) {
			value = attr->state[index].state_value;
			sid = attr->state[index].state_id;
			switch (sid) {
				case STATEID_LIGHT_STATE:
					e->group_state = value;
					break;
				case STATEID_OUTLET_GROUPNUM:
					e->group_num = value;
					break;
				
				case STATEID_OUTLET_FUCTION:
					e->is_more_ctrl = !!value;
					break;
				
				case STATEID_RF_REPEATER:
					e->is_rf_repeater = 1;
					e->rf_repeater_on = !!value;
					break;
                case STATEID_DB_DIMMING:
                    e->db_dimming_lamp_value = value & 0xff;
                    break;

				default:
					break;
			}
		}
		FORCE_RF_REPEATER_ON(e);
	} else if (e->dev_type == CL_RT_DB_RF_CURTAIN) {
		for (index=0; index<attr->n_state; index++) {
			value = attr->state[index].state_value;
			sid = attr->state[index].state_id;
			switch (sid) {
				case STATEID_DB_RF_CURTAIN_TIME:
					e->group_num = value;
					break;
				case STATEID_DB_RF_CURTAIN_POSTION:
					e->group_state = value;
					break;	
				case STATEID_RF_REPEATER:
					e->is_rf_repeater = 1;
					e->rf_repeater_on = !!value;

				default:
					break;
			}
		}
		FORCE_RF_REPEATER_ON(e);
	}
	

}

static u_int16_t get_rf_2_state(equipment_t* eq)
{
	u_int8_t index;
	remote_atrri_v2_t *attr = &eq->attr; 
	
	for (index = 0; index < attr->n_state; index++) {
		if (attr->state[index].state_id == STATEID_LIGHT_STATE)
			return attr->state[index].state_value;
	}
	return 0;
}


void eq_build_objs(cl_dev_info_t *ui, slave_t *slave, int *idx_eq)
{
	cl_equipment_t *e;
	equipment_t *eq;
	remote_atrri_v2_t *attr;
	u_int8_t dev_type, i = 0;
	slave_t *bind_slave = NULL;

	if (slave->equipment == NULL || stlc_list_empty(&slave->equipment->eq_list))
		return;

	
	stlc_list_for_each_entry(equipment_t, eq, &slave->equipment->eq_list, link) {
		attr = &eq->attr;
		e = cl_calloc(sizeof(cl_equipment_t), 1);
		
		dev_type = 0x0;
		if (our_remote_type_2_cl_remote_type(attr->dev_type, attr->factory_id, &dev_type)) {
			e->dev_type = dev_type;
		}else{
			e->dev_type = attr->dev_type;
		}
		e->obj.type = OT_EQUIPMENT;
		e->obj.status = slave->status;
		e->obj.handle = eq->handle;
		e->obj.name = cl_strdup((const char*)attr->name);
		e->obj.sn = slave->sn;
		e->area_handle = area_get_handle_by_id(slave->user,attr->area_id);
        e->local_id = attr->local_id;
		e->match_id_num = eq->match_id_num;
		for (i = 0; i < eq->attr.n_state; i++) {
			//云空调类型
			if ((eq->attr.state[i].state_id == REMOTE_CLOUD_IR_STATUS_OLD_AC) && (eq->attr.state[i].state_value != 0)) {
				e->ac_type = 1;
			}
			//新式云空调上一次按下的键
			if ((eq->attr.state[i].state_id == REMOTE_CLOUD_IR_STATUS_LAST) && (eq->attr.state[i].state_value != 0)) {
				e->last_presskey = eq->attr.state[i].state_value;
			} else {
				e->last_presskey = 0;
			}
		}
		
		memcpy(e->match_id, eq->match_id, sizeof(eq->match_id));
		if (attr->bind_sn)
			bind_slave = slave_lookup_by_sn(slave->user, attr->bind_sn);
		if (bind_slave)
			e->eq_001e_handle = bind_slave->handle;
		eq_set_state(e, attr);
		rk_build_objs(slave, eq, e);

		ui->objs[ui->idx_equipment + (*idx_eq)] = &e->obj;
		(*idx_eq)++;
	}
}

RS eq_alloc(slave_t *slave)
{
	equipment_ctrl_t *ec;

	if ( ! slave->has_eq || slave->equipment != NULL)
		return RS_OK;

	slave->equipment = ec = cl_calloc(sizeof(equipment_ctrl_t), 1);

	ec->slave = slave;
	STLC_INIT_LIST_HEAD(&ec->eq_list);
    STLC_INIT_LIST_HEAD(&ec->eq_new_eq_hand_list);

	// 第一次快速查询
    if (slave->sn == slave->user->sn) {
        CL_THREAD_TIMER_ON(&cl_priv->master, ec->t_query, timer_eq_query, (void *)ec, 0);
    }
	

	return RS_OK;
}



void eq_free(slave_t *slave)
{
	equipment_ctrl_t *ec;
    
	if ((ec = slave->equipment) == NULL)
		return;
    
    _eqc_free(ec,true);
    slave->equipment = NULL;
    
}

RS key_learn_alloc(user_t* user)
{
    _key_learn_free(user->kl,true);
    user->kl = _key_learn_alloc();
    
    return RS_OK;
}

void key_learn_free(user_t* user)
{
    _key_learn_free(user->kl,true);
    user->kl = NULL;
}

/***********************************************************************/
//设备报警电话管理

static int timer_user_ap_query(cl_thread_t *t)
{
	pkt_t *pkt;
	user_t *user;
	net_alarm_config_phone_t* nbp;
    alarm_phone_assist_t* ap;
    
    user = (user_t*)CL_THREAD_ARG(t);
	ap = user->ap;
    if (!ap) {
        return RS_ERROR;
    }
    ap->t_phone_query = NULL;

	if (user->is_phone_user)
		return RS_OK;
    
    /*设备不在线，两秒探测一次*/
    if (!user->online) {
        CL_THREAD_TIMER_ON(&cl_priv->master, ap->t_phone_query, timer_user_ap_query, (void *)user, TIME_N_SECOND(2));
        return RS_OK;
    }
    CL_THREAD_TIMER_ON(&cl_priv->master, ap->t_phone_query, timer_user_ap_query, (void *)user, TIME_ALARM_PHONE_QUERY);
    
    USER_BACKGROUND_RETURN_CHECK(user);
    
	pkt = pkt_new_v2(CMD_ALARM_CONFIG_PHONE, sizeof(net_alarm_config_phone_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = user->handle;
    nbp = get_pkt_payload(pkt, net_alarm_config_phone_t);
    nbp->action = AC_QUERY;
	user_add_pkt(user, pkt);
    
	return 0;
}

static void timer_user_ap_quick_query(user_t* user)
{
    alarm_phone_assist_t* ap = user->ap;
	
    if (ap) {
        CL_THREAD_OFF(ap->t_phone_query);
        CL_THREAD_TIMER_ON(&cl_priv->master, ap->t_phone_query, timer_user_ap_query, (void *)user, 0);
    }
}
#if 0
static void alarm_phone_proc_modify(user_t* user,cln_equipment_t* ce ,RS *ret)
{
    pkt_t* pkt;
    net_alarm_config_phone_t* nacp;
    u_int8_t i;
    char* p;
	int len;
    
    if (ce->numofphone>10) {
        ce->numofphone=10;
    }
    
    len = ce->numofphone*16;
    pkt = pkt_new_v2(CMD_ALARM_CONFIG_PHONE, len+sizeof(*nacp), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    if (!pkt) {
        return;
    }
    PKT_HANDLE(pkt)= user->handle;
    nacp = get_pkt_payload(pkt, net_alarm_config_phone_t);
    nacp->action = AC_ADD;
    nacp->count = ce->numofphone;
    p = (char*)nacp->phones;
    for (i=0; i< ce->numofphone; i++,p+=ALARM_PHONE_LENGTH) {
        strncpy(p, ce->phonelist[i], 15);
    }
    user_add_pkt(user, pkt);
    timer_user_ap_quick_query(user);
}
#endif

static void alarm_phone_proc_add(user_t* user,cln_equipment_t* ce ,RS *ret)
{
    pkt_t* pkt;
    net_alarm_config_phone_t* nacp;
    
    pkt = pkt_new_v2(CMD_ALARM_CONFIG_PHONE, sizeof(*nacp)+ALARM_PHONE_LENGTH,
                     NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    if (!pkt) {
        return;
    }
    PKT_HANDLE(pkt)= user->handle;
    nacp = get_pkt_payload(pkt, net_alarm_config_phone_t);
    nacp->action = AC_ADD;
    nacp->count = 0x1;
    strcpy((char*)nacp->phones, ce->name);
    user_add_pkt(user, pkt);
    timer_user_ap_quick_query(user);
}

static void alarm_phone_proc_del(user_t* user,cln_equipment_t* ce ,RS *ret)
{
    pkt_t* pkt;
    net_alarm_config_phone_t* nacp;
   
    pkt = pkt_new_v2(CMD_ALARM_CONFIG_PHONE, ALARM_PHONE_LENGTH+sizeof(*nacp),
                     NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    if (!pkt) {
        return;
    }
    PKT_HANDLE(pkt)= user->handle;
    nacp = get_pkt_payload(pkt, net_alarm_config_phone_t);
    nacp->action = AC_DEL;
    nacp->count = 0x1;
    strcpy((char*)nacp->phones, ce->name);
    
    user_add_pkt(user, pkt);
    timer_user_ap_quick_query(user);
}

static void alarm_phone_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    user_t* user;
    cln_equipment_t* ce;
    
    ce = (cln_equipment_t*)&pkt->data[0];
    user = lookup_by_handle(HDLT_USER, ce->user_handle);
    if (!user||!user->ap) {
        log_err(false, "do_key_learn_config can not find user handlue=%u\n",ce->user_handle);
        *ret = RS_NOT_FOUND;
        return;
    }
    
    switch (pkt->type) {
        case CLNE_USER_ADD_ALARM_PHONE:
            alarm_phone_proc_add(user,ce,ret);
            break;
        case CLNE_USER_DEL_ALARM_PHONE:
            alarm_phone_proc_del(user,ce,ret);
        default:
            break;
    }
    
}

static void eq_bind_soundlight(equipment_t *eq, cln_equipment_t *ce, RS *ret)
{
	int i;
    pkt_t *pkt;
    net_remote_config_soundlight *slp;
    user_t *user = eq->ec->slave->user;
	equipment_t *eq_sl;
	
    pkt = pkt_new_v2(CMD_REMOTE_CONFIG_SOUNDLIGHT, 
				sizeof(*slp) + sizeof(remote_soundlight_list_t)*ce->soundline_num,
				NHF_TRANSPARENT|NHF_WAIT_REPLY,
				user->sn, user->ds_type);
    if (pkt == NULL) {
		*ret = RS_MEMORY_MALLOC_FAIL;
        return;
    }
    slp = get_pkt_payload(pkt, net_remote_config_soundlight);
	memset(slp, 0, sizeof(*slp));
	for (i = 0; i < ce->soundline_num; i++) {
		if ((eq_sl = eq_lookup_by_handle(eq->ec, ce->soundline[i])) == NULL) {
			log_err(false, "eq_bind_soundlight failed: not found soundlight handle=0x%08x\n", ce->soundline[i]);
			pkt_free(pkt);
			*ret = RS_INVALID_PARAM;
			return;
		}
		slp->list[i].local_id = htons(eq_sl->attr.local_id);
		slp->list[i].pad = 0;
	}
    slp->action = AC_MOD;
	slp->onoff = ce->soundline_on;
    slp->count = ce->soundline_num;
	slp->local_id = htons(eq->attr.local_id);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, eq->ec->slave->handle, CMD_REMOTE_CONFIG_SOUNDLIGHT, NULL, callback_eq_request);
	
    user_add_pkt(user, pkt);
	do_alarm_phone_quick_query(eq);
}

static void eq_proc_bind_soundlight(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_equipment_t *ce;
	equipment_t *eq;
    
    ce = (cln_equipment_t *)&pkt->data[0];
    eq = (equipment_t *)lookup_by_handle(HDLT_EQUIPMENT, ce->eq_handle);
    if (eq == NULL) {
        log_err(false, "eq_proc_bind_soundlight can not find eqment handlue=%u\n", ce->user_handle);
        *ret = RS_NOT_FOUND;
        return;
    }

	eq_bind_soundlight(eq, ce, ret);
}

static RS eq_linkage_scene(cl_notify_pkt_t *cln_pkt, RS *ret)
{
	int action;
	cln_equipment_t *clne = (cln_equipment_t *)cln_pkt->data;
	pkt_t *pkt;
	net_scene_linkage_t *nslp;
	scene_linkage_t *slp;
	scene_alarm_t *sa;
	scene_priv_t *scene = NULL;
	equipment_t *eq;
	user_t *user;

	eq = (equipment_t *)lookup_by_handle(HDLT_EQUIPMENT, clne->eq_handle);
	if (eq == NULL) {
		log_err(false, "eq_linkage_scene failed: not found equipment handle=0x%08x\n", clne->eq_handle);
        return RS_INVALID_PARAM;
	}
	user = eq->ec->slave->user;

	action = (cln_pkt->type == CLNE_EQ_LINKAGE_SCENE_DEL ? AC_DEL : AC_MOD);

    scene = lookup_by_handle(HDLT_SCENE, clne->scene_handle);
    if (scene == NULL) {
		log_err(false, "eq_linkage_scene failed: not found scene handle=0x%08x\n", clne->scene_handle);
        return RS_INVALID_PARAM;
    }

    pkt = pkt_new_v2(CMD_SCENE_LINKAGE, 
				sizeof(net_scene_linkage_t) + sizeof(scene_linkage_t) + sizeof(scene_alarm_t),
				NHF_TRANSPARENT|NHF_WAIT_REPLY,
                user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, eq->ec->slave->handle, CMD_SCENE_LINKAGE, NULL, callback_eq_request);
	
    nslp = get_pkt_payload(pkt, net_scene_linkage_t);
	slp = &nslp->sl[0];
	sa = (scene_alarm_t *)(slp + 1);
	
	memset(nslp, 0, sizeof(*nslp));
	memset(slp, 0, sizeof(*slp));
	memset(sa, 0, sizeof(*sa));
	
	nslp->action = action;
	nslp->count = 1;
	nslp->query_type = SCENE_LINKAGE_TYPE_ALARM;

	slp->type = SCENE_LINKAGE_TYPE_ALARM;
	slp->scene_id = scene->scene_id;
	slp->len = sizeof(*sa)/4;

	sa->local_id = htons(eq->attr.local_id);

	user_add_pkt(user, pkt);
	quick_query_alarm_link_scene(user);

	return RS_OK;
}
	
static void alarm_phone_proc_replace(user_t* user,pkt_t* pkt,net_alarm_config_phone_t* na)
{
    net_alarm_config_phone_t* nacp;
    int len = net_param_len(pkt->data)-sizeof(net_alarm_config_phone_t);
    
    if (!user->ap) {
        log_err(false, "alarm_phone_proc_replace failed: user ap invalid handle=%u\n",user->handle);
        return;
    }
    
    
    if (len<0||na->err||len!=na->count*ALARM_PHONE_LENGTH) {
        log_err(false, "alarm_phone_proc_replace failed: err %u count na->count %u\n", na->err,na->count);
        return;
    }
    
    if ( user->ap->nacp &&
        net_param_len(pkt->data) == user->ap->nacp_len &&
        !memcmp(user->ap->nacp, na, user->ap->nacp_len)) {
        log_debug("alarm_phone_proc_replace same packet ignore!\n");
        return;
    }
    
    SAFE_FREE(user->ap->nacp);
    user->ap->nacp_len = 0;
    
    nacp = cl_calloc(net_param_len(pkt->data), 1);
    if (nacp) {
        memcpy(nacp, na, net_param_len(pkt->data));
        user->ap->nacp = nacp;
        user->ap->nacp_len = net_param_len(pkt->data);
    }
    
    event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    
}

static void alarm_phone_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
    net_alarm_config_phone_t* na;
    
    if (net_param_len(pkt->data) < sizeof(net_alarm_config_phone_t)) {
		log_err(false, "alarm_phone_proc_tcp failed: param len=%d\n", net_param_len(pkt->data));
		return;
	}
    
    na = get_pkt_payload(pkt, net_alarm_config_phone_t);
    na->err = ntohl(na->err);
    log_debug("alarm_phone_proc_tcp action = %d err= %d\n",na->action,na->err);
    switch (na->action) {
        case AC_QUERY:
            alarm_phone_proc_replace(user,pkt,na);
            break;
        case AC_MOD:
        case AC_DEL:
        case AC_ADD:
        {
            if (na->err) {
                event_push_err(user->callback, UE_MODIFY_ALARM_PHONE_FAIL,
                           user->handle, user->callback_handle, na->err);
            }else{
                event_push(user->callback, UE_MODIFY_ALARM_PHONE_OK,
                           user->handle, user->callback_handle);
            }
        }
            break;
        default:
            break;
    }
    
}

RS alarm_phone_alloc(user_t* user)
{
    alarm_phone_assist_t* ap;
    
    ap = cl_calloc(sizeof(*ap), 1);
    if (ap) {
        user->ap = ap;
        ap->user = user;
        
        timer_user_ap_quick_query(user);
        
        return RS_OK;
    }
    return RS_ERROR;
}

void alarm_phone_free(user_t* user)
{
    alarm_phone_assist_t* ap;
    
    ap = user->ap;
    user->ap = NULL;
    if (ap) {
        CL_THREAD_OFF(ap->t_phone_query);
        SAFE_FREE(ap->nacp);
        cl_free(ap);
    }
}
