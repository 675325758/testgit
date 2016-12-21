#include "cl_ia.h"
#include "cl_priv.h"
#include "cl_notify.h"
#include "ia_priv.h"
#include "wait_server.h"
#include "lc_furnace_priv.h"
#include "eb_priv.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"

typedef struct {
	u_int16_t ns;
	ia_status_t *is_head;
} cl_ia_data_t;

typedef struct {
	u_int16_t id;
	int event_ok;
	int event_fault;
} id_event_map_t;


static id_event_map_t  id_aircleaner_dict[] = {
	/* 空气净化器 */
	{ IA_AIRCLEANER_STATUS_WORK, IE_AIRCLEANER_SET_ONOFF_OK, IE_AIRCLEANER_SET_ONOFF_FAULT },
	{ IA_AIRCLEANER_STATUS_SPEED, IE_AIRCLEANER_SET_SPEED_OK, IE_AIRCLEANER_SET_SPEED_FAULT },
	{ IA_AIRCLEANER_STATUS_TIMER, IE_AIRCLEANER_SET_TIMER_OK, IE_AIRCLEANER_SET_TIMER_FAULT },
	{ IA_AIRCLEANER_STATUS_ULTRAVIOLET, IE_AIRCLEANER_SET_ULTRA_OK, IE_AIRCLEANER_SET_ULTRA_FAULT },
	{ IA_AIRCLEANER_STATUS_ANION, IE_AIRCLEANER_SET_ANION_OK, IE_AIRCLEANER_SET_ANION_FAULT },

};

static id_event_map_t  id_airheater_dict[] = {

	/* 快热炉 */
	{ IA_AIRHEATER_STATUS_ONOFF, IE_AIRHEATER_SET_ONOFF_OK, IE_AIRHEATER_SET_ONOFF_FAULT },
	{ IA_AIRHEATER_STATUS_GEAR, IE_AIRHEATER_SET_GEAR_OK, IE_AIRHEATER_SET_GEAR_FAULT },
	{ IA_AIRHEATER_STATUS_TIME, IE_AIRHEATER_SET_TIME_OK, IE_AIRHEATER_SET_TIME_FAULT },
	{ IA_AIRHEATER_STATUS_MODE, IE_AIRHEATER_SET_MODE_OK, IE_AIRHEATER_SET_MODE_FAULT },
};


static id_event_map_t  id_airheater_ycyt_dict[] = {

	/* 快热炉 ycyt */
	{ IA_AIRHEATER_YCYT_STATUS_TEMP_SET, IE_AIRHEATER_YCYT_SET_TEMP_OK, IE_AIRHEATER_YCYT_SET_TEMP_FAULT },
	{ IA_AIRHEATER_YCYT_STATUS_MODE, IE_AIRHEATER_YCYT_SET_MODE_OK, IE_AIRHEATER_YCYT_SET_MODE_FAULT },
	{ IA_AIRHEATER_YCYT_STATUS_GEAR, IE_AIRHEATER_YCYT_SET_GEAR_OK, IE_AIRHEATER_YCYT_SET_GEAR_FAULT },
	{ IA_AIRHEATER_YCYT_STATUS_TIME, IE_AIRHEATER_YCYT_SET_TIMER_OK, IE_AIRHEATER_YCYT_SET_TIMER_FAULT },
    { IA_AIRHEATER_YCYT_STATUS_WORK, IE_AIRHEATER_YCYT_SET_ONOFF_OK, IE_AIRHEATER_YCYT_SET_ONOFF_FAULT },
    { IA_AIRHEATER_YCYT_STATUS_ORDER_TIME, IE_AIRHEATER_YCYT_SET_ORDER_TIMER_OK, IE_AIRHEATER_YCYT_SET_ORDER_TIMER_FAULT }
};


static id_event_map_t  id_waterheater_dict[] = {
	/* 热水器 */
	{ IA_WATERHEATER_STATUS_WORK, IE_WATERHEATER_SET_WORK_OK, IE_WATERHEATER_SET_WORK_FAULT },
	{ IA_WATERHEATER_STATUS_TEMP_SET, IE_WATERHEATER_SET_TEMP_OK, IE_WATERHEATER_SET_TEMP_FAULT },
	{ IA_WATERHEATER_STATUS_TIMER, IE_WATERHEATER_SET_TIMER_OK, IE_WATERHEATER_SET_TIMER_FAULT },
	{ IA_WATERHEATER_STATUS_CAPACTITY, IE_WATERHEATER_SET_CAPACTITY_OK, IE_WATERHEATER_SET_CAPACTITY_FAULT },
};

static id_event_map_t  id_waterheater_a9_dict[] = {
	/* 前锋热水器 */
	{ IA_WATERHEATER_A9_STATUS_WORK, IE_WATERHEATER_A9_SET_WORK_OK, IE_WATERHEATER_A9_SET_WORK_FAULT },
	{ IA_WATERHEATER_A9_STATUS_TEMP_SET, IE_WATERHEATER_A9_SET_TEMP_OK, IE_WATERHEATER_A9_SET_TEMP_FAULT },
	{ IA_WATERHEATER_A9_STATUS_MODE, IE_WATERHEATER_A9_SET_MODE_OK, IE_WATERHEATER_A9_SET_MODE_FAULT },
	{ IA_WATERHEATER_A9_STATUS_FIRE_LEVEL, IE_WATERHEATER_A9_SET_FIRE_LEVEL_OK, IE_WATERHEATER_A9_SET_FIRE_LEVEL_FAULT },
	{ IA_WATERHEATER_A9_STATUS_COUNT, IE_WATERHEATER_A9_CLEAR_CNT_OK, IE_WATERHEATER_A9_CLEAR_CNT_FAULT },
};

static id_event_map_t  id_aircondition_dict[] = {
	/* 空调 */
	{ IA_AIRCONDITION_STATUS_ONOFF, IE_AIRCONDITION_SET_ONOFF_OK, IE_AIRCONDITION_SET_ONOFF_FAULT },
	{ IA_AIRCONDITION_STATUS_MODE, IE_AIRCONDITION_SET_MODE_OK, IE_AIRCONDITION_SET_MODE_FAULT },
	{ IA_AIRCONDITION_STATUS_TEMP, IE_AIRCONDITION_SET_TEMP_OK, IE_AIRCONDITION_SET_TEMP_FAULT },
	{ IA_AIRCONDITION_STATUS_TIMER, IE_AIRCONDITION_SET_TIMER_OK, IE_AIRCONDITION_SET_TIMER_FAULT },

};

static id_event_map_t  id_electricfan_dict[] = {
	/* 风扇 */
	{ IA_ELECTRICFAN_STATUS_WORK, IE_ELECTRICFAN_SET_WORK_OK, IE_ELECTRICFAN_SET_WORK_FAULT },
	{ IA_ELECTRICFAN_STATUS_GEAR, IE_ELECTRICFAN_SET_GEAR_OK, IE_ELECTRICFAN_SET_GEAR_FAULT },
	{ IA_ELECTRICFAN_STATUS_TIMER, IE_ELECTRICFAN_SET_TIMER_OK, IE_ELECTRICFAN_SET_TIMER_FAULT },
	{ IA_ELECTRICFAN_STATUS_SHAKE, IE_ELECTRICFAN_SET_SHAKE_OK, IE_ELECTRICFAN_SET_SHAKE_FAULT },

};


static id_event_map_t  id_bathheater_dict[] = {
	/* 浴霸 */
	{ IA_BATHROOMMASTER_STATUS_ONOFF, IE_BATHHEATER_SET_WORK_OK, IE_BATHHEATER_SET_WORK_FAULT},
	{ IA_BATHROOMMASTER_STATUS_NEGATIVEIONS, IE_BATHHEATER_SET_ANION_OK, IE_BATHHEATER_SET_ANION_FAULT},
	{ IA_BATHROOMMASTER_STATUS_LIGHT, IE_BATHHEATER_SET_LIGHT_OK, IE_BATHHEATER_SET_LIGHT_FAULT},
	{ IA_BATHROOMMASTER_STATUS_AIR, IE_BATHHEATER_SET_BREATH_OK, IE_BATHHEATER_SET_BREATH_FAULT},
	{ IA_BATHROOMMASTER_STATUS_DRY, IE_BATHHEATER_SET_DRY_OK, IE_BATHHEATER_SET_DRY_FAULT},
	{ IA_BATHROOMMASTER_STATUS_WARNM, IE_BATHHEATER_SET_TRONIC_OK, IE_BATHHEATER_SET_TRONIC_FAULT},
	{ IA_BATHROOMMASTER_STATUS_TIMER, IE_BATHHEATER_SET_TIME_OK, IE_BATHHEATER_SET_TIME_FAULT}
};

static void callback_ia_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	user_t *user;

	if ((user = (user_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_ia_request, not found user type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	switch (w->cmd) {
		
	default:
		log_err(false, "callback_ia_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}


static int timer_ia_query(cl_thread_t *t)
{
	ia_t *ia = (ia_t *)CL_THREAD_ARG(t);
	user_t *user = ia->user;
	pkt_t *pkt;
	net_ia_t *net_ia;
	
	ia->t_query = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, ia->t_query, timer_ia_query, (void *)ia, TIME_QUERY_IA_TIMER);
    
    USER_BACKGROUND_RETURN_CHECK(user);

	pkt = pkt_new_v2(CMD_IA, sizeof(net_ia_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (!pkt) {
		return RS_ERROR;
	}

	PKT_HANDLE(pkt) = wait_add(HDLT_USER, ia->user->handle, CMD_IA, NULL, callback_ia_request);
	log_debug("ready send pkt for query ia status\n");

	net_ia = get_pkt_payload(pkt, net_ia_t);
	net_ia->action = IA_ACTION_QUERY;
	user_add_pkt(user, pkt);
	return RS_OK;
}

static void ia_quick_query(ia_t *ia) 
{
	CL_THREAD_OFF(ia->t_query);
	CL_THREAD_TIMER_ON(&cl_priv->master, ia->t_query, timer_ia_query, (void *)ia, 0);
}


void ia_free(user_t *user)
{
	if (user->ia) {
		CL_THREAD_OFF(user->ia->t_query);
		cl_free(user->ia->is_head);
	}
	SAFE_FREE(user->ia);
	log_info("free ia moudle!!\n");
}


RS ia_init(user_t *user)
{
	u_int8_t type = 0;

	switch (user->sub_type) {
	case IJ_AIRCLEANER:
		type = IA_TYPE_AIRCLEANER;
		break;
	case IJ_AIRCONDITION:
		type = IA_TYPE_AIRCONDITION;
		break;
	case IJ_AIRHEATER:
		type = IA_TYPE_AIRHEATER;
		break;
	case IJ_WATERHEATER:
		type = IA_TYPE_WATERHEATER;
		break;
	case IJ_ELECTRICFAN:
		type = IA_TYPE_ELECTRICFAN;
		break;
	case IJ_815:
		type = IA_TYPE_BATHROOMMASTER;
		break;
	case IJ_101:
		type = IA_TYPE_EB;
		break;
	case IJ_102:
		type = IA_TYPE_EB_II;
		break;
	default:
		log_err(false, "This isn`t a ia dev; sub_type==0x%02x", user->sub_type);
		return RS_ERROR;
	}


	if (!user->ia) {
		user->ia = cl_calloc(sizeof(ia_t), 1);
		if (!user->ia) {
			log_err(true, "calloc fault!!");
			return RS_ERROR;
		}
	}


	log_info("This is a ia dev; sub_type==0x%02x", user->sub_type);
	user->ia->type = type;
	user->ia->user = user;
	ia_quick_query(user->ia);
	return RS_OK;
}




static RS ia_ctrl(cl_notify_pkt_t *notify_pkt)
{
	cln_ia_t *ci;
	user_t *user;
	pkt_t *pkt;
	net_ia_t *net_ia;
	int ns;

	ci = (cln_ia_t *)&notify_pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->user_handle);
	if (!user) {
		return RS_ERROR;
	}

	pkt = pkt_new_v2(CMD_IA, sizeof(net_ia_t) + sizeof(ia_status_t) * ci->ns, NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (!pkt) {
		return RS_ERROR;
	}

	PKT_HANDLE(pkt) = wait_add(HDLT_USER, ci->user_handle, CMD_IA, NULL, callback_ia_request);
	log_debug("ready send pkt for ctrl ia status\n");

	net_ia = get_pkt_payload(pkt, net_ia_t);
	net_ia->action = IA_ACTION_CTRL;
	net_ia->ns = ns = ci->ns;
	while (ns--) {
		net_ia->is[ns].id = htons(ci->ia_stat[ns].id);
		net_ia->is[ns].value = htons(ci->ia_stat[ns].value);
	}

	user_add_pkt(user, pkt);
	ia_quick_query(user->ia);
	return RS_OK;
}


bool ia_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	user_t* user;
	cln_ia_t *ci;
	
	if (pkt->type < CLNE_IA_START || pkt->type > CLNE_IA_END) {
		return false;
	}

	ci = (cln_ia_t *)&pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->user_handle);
	if(user && user->is_udp_ctrl){
		if(udp_dev_type_equl(user,IJ_816,ETYPE_IJ_816_LCYT)){
			return lcyt_proc_notify(pkt, ret);
		}
		
		if(udp_dev_type_equl(user, IJ_813, ETYPE_IJ_813_HK)){
			return hkac_proc_notify(pkt, ret);
		}

		if(udp_dev_type_equl(user, IJ_813, ETYPE_IJ_813_NB)){
			return nbac_proc_notify(pkt, ret);
		}

		if(udp_dev_type_equl(user, IJ_820, ETYPE_IJ_820_GX)){
			return lamp_proc_notify(pkt, ret);
		}

		if(user->sub_type == IJ_101 || user->sub_type == IJ_102) {
			return eb_proc_notify(user, pkt, ret);
		}

		if(udp_dev_type_equl(user,IJ_821,ETYPE_IJ_821_CH)){
			return ch_blanket_proc_notify(pkt, ret);
		}

//		if(user->sub_type == IJ_RFGW){
//			return rfgw_proc_notify(user, pkt, ret);
//		}
		
		return false;
	}
	
	switch (pkt->type) {
	case CLNE_IA_CTRL:
		*ret = ia_ctrl(pkt);
		break;

	default:
		return false;
	}

	return true;
}

static int  ia_get_ctrl_event(net_ia_t *net_ia, u_int8_t ia_sub_type)
{
	id_event_map_t *map;
	int n = 0;
	
	if (net_ia->ns == 0 || net_ia->ns > 1) {
		return net_ia->err ? IE_CTRL_FAULT : IE_CTRL_OK;
	}

	switch (IA_TYPE(net_ia->is[0].id)) {
	case IA_TYPE_AIRCLEANER:
		map = id_aircleaner_dict;
		n = sizeof(id_aircleaner_dict)/sizeof(id_event_map_t);
		break;
		
	case IA_TYPE_AIRCONDITION:
		map = id_aircondition_dict;
		n = sizeof(id_aircondition_dict)/sizeof(id_event_map_t);
		break;
		
	case IA_TYPE_AIRHEATER:
		if (ia_sub_type == IA_AIRHEATER_SUBTYPE_YCYT) {
			map = id_airheater_ycyt_dict;
			n = sizeof(id_airheater_ycyt_dict)/sizeof(id_event_map_t);
		}
		else {
			map = id_airheater_dict;
			n = sizeof(id_airheater_dict)/sizeof(id_event_map_t);
		}
		break;
		
	case IA_TYPE_ELECTRICFAN:
		map = id_electricfan_dict;
		n = sizeof(id_electricfan_dict)/sizeof(id_event_map_t);
		break;
		
	case IA_TYPE_WATERHEATER:
		if (ia_sub_type == IA_WATERHEATER_SUBTYPE_A9) {
			map = id_waterheater_a9_dict;
			n = sizeof(id_waterheater_a9_dict)/sizeof(id_event_map_t);
		}
		else {
			map = id_waterheater_dict;
			n = sizeof(id_waterheater_dict)/sizeof(id_event_map_t);
		}
		break;
	case IA_TYPE_BATHROOMMASTER:
		map = id_bathheater_dict;
		n = sizeof(id_bathheater_dict)/sizeof(id_event_map_t);
		break;

	case IA_TYPE_EB:
		break;
		
	default:
		return IE_CTRL_FAULT;
	}

	while (n--) {
		if (map[n].id == net_ia->is[0].id) {
			return net_ia->err ? map[n].event_fault : map[n].event_ok;
		}
	}
	
	return IE_CTRL_FAULT;
}

static void do_cmd_ia(ia_t *ia, pkt_t *pkt, net_header_t *hdr)
{
	net_ia_t *net_ia;
	int ns, event;


	if (hdr->param_len < sizeof(net_ia_t)) {
		log_err(false, "Invalid pkt!! drop it!!");
		return;
	}
	

	net_ia = get_pkt_payload(pkt, net_ia_t);
	if ((net_ia->ns * sizeof(ia_status_t) + sizeof(net_ia_t)) != hdr->param_len) {
		log_err(false, "Invalid pkt!! drop it!!");
		return;
	}

	
	net_ia->err = ntohl(net_ia->err);
	ns = net_ia->ns;
	while (ns--) {
		net_ia->is[ns].id = ntohs(net_ia->is[ns].id);
		net_ia->is[ns].value = ntohs(net_ia->is[ns].value);
	}

	log_info("recv ia reply,  err_number=%u, ns=%d, action=%d, param_len=%d\n", net_ia->err, net_ia->ns, net_ia->action, hdr->param_len);

	if (net_ia->action == IA_ACTION_CTRL) {
		event = ia_get_ctrl_event(net_ia, ia->ia_sub_type);
		event_push_err(ia->user->callback, event, ia->user->handle, ia->user->callback_handle, net_ia->err);
		return;
	}

	if (net_ia->err != RS_OK) {
		log_err(false, "Query ia stat fault\n");
		return;
	}

	//是否更新
	if (ia->ns == 0 && net_ia->ns == 0) {
		log_info("Noting change, num is zero!!");
		return;
	}
	if (ia->ns == net_ia->ns
		&& memcmp(ia->is_head, net_ia->is, net_ia->ns * sizeof(ia_status_t)) == 0) {
		log_info("Noting change!");
		return;
	}

	SAFE_FREE(ia->is_head);
	ia->is_head = cl_calloc(net_ia->ns * sizeof(ia_status_t), 1);
	if (!ia->is_head) {
		ia->ns = 0;
		log_err(true, "calloc fault");
		return;
	}

	ia->ns = net_ia->ns;
	memcpy(ia->is_head, net_ia->is, net_ia->ns * sizeof(ia_status_t));
	event_push(ia->user->callback, IE_UPDATE_STATUS, ia->user->handle, ia->user->callback_handle);
	event_push(ia->user->callback, UE_INFO_MODIFY, ia->user->handle, ia->user->callback_handle);
}


bool ia_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	u_int32_t err_code = 0;

	
	switch (hdr->command) {
	case CMD_IA:
		do_cmd_ia(user->ia, pkt, hdr);
		break;
	default:
		return false;
	}

	return true;
}


static u_int16_t  parse_ia_array(cl_ia_data_t *ia_arry, u_int16_t id)
{
	int i;
	for (i = 0; i < ia_arry->ns; i++) {
		if (ia_arry->is_head[i].id == id) {
			return ia_arry->is_head[i].value;
		}
	}
	return 0;
}

static cl_ia_aircleaner_info_t* ia_build_aircleaner_info(cl_ia_data_t *ia_data)
{
	cl_ia_aircleaner_info_t  *info;

	info = cl_calloc(sizeof(cl_ia_aircleaner_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->onoff = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_WORK);
	info->ultra = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_ULTRAVIOLET);
	info->anion = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_ANION);
	info->speed = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_SPEED);
	info->timer = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_TIMER);
	info->pm25 = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_PM25);
	info->temp = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_TEMP);
	info->rh = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_RH);
	info->power = parse_ia_array(ia_data, IA_AIRCLEANER_STATUS_POWER);

	

	return info;
}

static cl_ia_airheater_info_t* ia_build_airheater_info(cl_ia_data_t *ia_data)
{
	cl_ia_airheater_info_t	 *info;
		info = cl_calloc(sizeof(cl_ia_airheater_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->onoff = parse_ia_array(ia_data, IA_AIRHEATER_STATUS_ONOFF);
	info->gear = parse_ia_array(ia_data, IA_AIRHEATER_STATUS_GEAR);
	info->time = parse_ia_array(ia_data, IA_AIRHEATER_STATUS_TIME);
	info->mode = parse_ia_array(ia_data, IA_AIRHEATER_STATUS_MODE);
	info->power = parse_ia_array(ia_data, IA_AIRHEATER_STATUS_POWER);
	info->temp = parse_ia_array(ia_data, IA_AIRHEATER_STATUS_TEMP);
	return info;
}


static cl_ia_airheater_ycyt_info_t* ia_build_airheater_ycyt_info(cl_ia_data_t *ia_data)
{
	cl_ia_airheater_ycyt_info_t	 *info;
	info = cl_calloc(sizeof(cl_ia_airheater_ycyt_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->set_temp = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_TEMP_SET);
	info->cur_temp = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_TEMP_CURRENT);
	info->mode = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_MODE);
	info->gear = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_GEAR);
	info->time = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_TIME);
    info->onoff = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_WORK);
    info->time_on = parse_ia_array(ia_data, IA_AIRHEATER_YCYT_STATUS_ORDER_TIME);

	return info;
}

static cl_ia_waterheater_info_t * ia_build_waterheater_info(cl_ia_data_t *ia_data)
{
	cl_ia_waterheater_info_t *info;
	info = cl_calloc(sizeof(cl_ia_waterheater_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->work = parse_ia_array(ia_data, IA_WATERHEATER_STATUS_WORK);
	info->temp_set = parse_ia_array(ia_data, IA_WATERHEATER_STATUS_TEMP_SET);
	info->temp_current = parse_ia_array(ia_data, IA_WATERHEATER_STATUS_TEMP_CURRENT);
	info->timer = parse_ia_array(ia_data, IA_WATERHEATER_STATUS_TIMER);
	info->capactity = parse_ia_array(ia_data, IA_WATERHEATER_STATUS_CAPACTITY);
	info->power = parse_ia_array(ia_data, IA_WATERHEATER_STATUS_POWER);
	return info;
}


static cl_ia_waterheater_a9_info_t * ia_build_waterheater_a9_info(cl_ia_data_t *ia_data)
{
	cl_ia_waterheater_a9_info_t *info;
	info = cl_calloc(sizeof(cl_ia_waterheater_a9_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->work = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_WORK);
	info->temp_set = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_TEMP_SET);
	info->temp_current = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_TEMP_CURRENT);
	info->mode = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_MODE);
	info->count = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_COUNT);
	info->fire_level = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_FIRE_LEVEL);
	info->gas = parse_ia_array(ia_data, IA_WATERHEATER_A9_STATUS_GAS);
	
	return info;
}


static cl_ia_aircondition_info_t * ia_build_aircondition_info(cl_ia_data_t *ia_data)
{
	cl_ia_aircondition_info_t *info;
	info = cl_calloc(sizeof(cl_ia_aircondition_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->onoff = parse_ia_array(ia_data, IA_AIRCONDITION_STATUS_ONOFF);
	info->mode = parse_ia_array(ia_data, IA_AIRCONDITION_STATUS_MODE);
	info->temp = parse_ia_array(ia_data, IA_AIRCONDITION_STATUS_TEMP);
	info->power = parse_ia_array(ia_data, IA_AIRCONDITION_STATUS_POWER);
	info->timer = parse_ia_array(ia_data, IA_AIRCONDITION_STATUS_TIMER);
	info->cur_temp = parse_ia_array(ia_data, IA_AIRCONDITION_STATUS_CUR_TEMP);
	return info;	
}

static cl_ia_electricfan_info_t  * ia_build_electricfan_info(cl_ia_data_t *ia_data)
{
	cl_ia_electricfan_info_t  *info;
	info = cl_calloc(sizeof(cl_ia_electricfan_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->work = parse_ia_array(ia_data, IA_ELECTRICFAN_STATUS_WORK);
	info->gear = parse_ia_array(ia_data, IA_ELECTRICFAN_STATUS_GEAR);
	info->timer = parse_ia_array(ia_data, IA_ELECTRICFAN_STATUS_TIMER);
	info->shake = parse_ia_array(ia_data, IA_ELECTRICFAN_STATUS_SHAKE);
	info->power = parse_ia_array(ia_data, IA_ELECTRICFAN_STATUS_POWER);

	return info;	
}

static cl_ia_bath_heater_info_t* ia_build_bathheater_info(cl_ia_data_t *ia_data)
{
	cl_ia_bath_heater_info_t  *info;
	info = cl_calloc(sizeof(cl_ia_bath_heater_info_t), 1);
	if (!info) {
		return NULL;
	}

	info->power_on_off = parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_ONOFF);
	info->anion_on_off = parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_NEGATIVEIONS);
	info->light_on_off = parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_LIGHT);
	info->breath_on_off = parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_AIR);
	info->dry_on_off = parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_DRY);
	info->tronic_on_off = parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_WARNM);
	info->next_time= parse_ia_array(ia_data, IA_BATHROOMMASTER_STATUS_TIMER);

	return info;	
}


void ia_build_objs(user_t* user, cl_dev_info_t* ui)
{

	cl_ia_data_t *ia_data;

	if(user && user->is_udp_ctrl){
		if(udp_dev_type_equl(user,IJ_816,ETYPE_IJ_816_LCYT)){
			lcyt_build_objs(user,ui);
		}
		if(udp_dev_type_equl(user,IJ_813,ETYPE_IJ_813_HK)){
			hkac_build_objs(user,ui);
		}
		if(udp_dev_type_equl(user,IJ_813,ETYPE_IJ_813_NB)){
			nbac_build_objs(user,ui);
		}
		if(udp_dev_type_equl(user, IJ_820, ETYPE_IJ_820_GX)){
			lamp_build_objs(user, ui);
		}

		if(udp_dev_type_equl(user,IJ_821,ETYPE_IJ_821_CH)){
			ch_blanket_build_objs( user,ui);
		}
		
		return ;
	}

	if (!user || !user->ia || user->ia->ns == 0 || !ui) {
		return;
	}
	ia_data = (cl_ia_data_t *)&user->ia->ns;


	switch (user->ia->type) {
	case IA_TYPE_AIRCLEANER:
		ui->ia_info.u.aircleaner_info = ia_build_aircleaner_info(ia_data);
		break;
	case IA_TYPE_AIRCONDITION:
		ui->ia_info.u.aircondition_info = ia_build_aircondition_info(ia_data);
		break;
	case IA_TYPE_AIRHEATER:
		if (user->ia->ia_sub_type == IA_AIRHEATER_SUBTYPE_YCYT) {
			ui->ia_info.u.airheater_ycyt_info = ia_build_airheater_ycyt_info(ia_data);
		}
		else {
			ui->ia_info.u.airheater_info = ia_build_airheater_info(ia_data);
		}
		break;
	case IA_TYPE_ELECTRICFAN:
		ui->ia_info.u.electricfan_info = ia_build_electricfan_info(ia_data);
		break;
	case IA_TYPE_WATERHEATER:
		if (user->ia->ia_sub_type == IA_WATERHEATER_SUBTYPE_A9) {
			ui->ia_info.u.waterheater_a9_info = ia_build_waterheater_a9_info(ia_data);
		}
		else  {
			ui->ia_info.u.waterheater_info = ia_build_waterheater_info(ia_data);
		}
		break;
	case IA_TYPE_BATHROOMMASTER:
		ui->ia_info.u.bath_heater_info = ia_build_bathheater_info(ia_data);
		break;
	default:
		return;
	}

	ui->ia_info.ia_type = user->ia->type;
	ui->ia_info.ia_sub_type = user->ia->ia_sub_type;
}







