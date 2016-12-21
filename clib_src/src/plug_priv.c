#include "client_lib.h"
#include "cl_priv.h"
#include "cl_plug.h"
#include "plug_priv.h"
#include "wait_server.h"


static void callback_plug_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	slave_t *slave;
	plug_t *plug;

	if ((slave = (slave_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_plug_request, not found user type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	plug = slave->plug;

	switch (w->cmd) {
	case CMD_PLUG_TIMER_Q:
		log_err(false, "%s callback_plug_request, err=%u\n", slave->str_sn, result);
		event_push_err(plug->callback, PE_TIMER_SET_FAIL, plug->handle, plug->callback_handle, result);
		break;

	default:
		log_err(false, "callback_plug_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}

int timer_plug_query(cl_thread_t *t)
{
	plug_t *plug = (plug_t *)CL_THREAD_ARG(t);
	user_t *user = plug->slave->user;
	int i;
	pkt_t *pkt;
	u_int16_t *req;
	u_int16_t req_list[] = {MT_PLUG_ON, MT_PLUG_AC, MT_PLUG_V, MT_PLUG_T};
	net_plug_electric_stat_t *es;	
	net_plug_timer_config_t *tc;

	plug->t_query = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, plug->t_query, timer_plug_query, (void *)plug, 1000);
    
    USER_BACKGROUND_RETURN_CHECK(user);

	if (plug->slave->status != BMS_BIND_ONLINE) {
		log_err(false, "timer_plug_query, %s status=%d, not query\n", plug->slave->str_sn, plug->slave->status);
		return 0;
	}

	// query A: 电量统计
	plug->count_q_stat++;
	if (plug->slave->has_electric_stat && (plug->quick_query || plug->count_q_stat >= plug->timer_q_stat)) {
		plug->count_q_stat = 0;

		pkt = pkt_new_v2(CMD_PLUG_ELECTRIC_STAT, sizeof(net_plug_electric_stat_t), 
				NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_PLUG_ELECTRIC_STAT, NULL, callback_plug_request);
		es = get_pkt_payload(pkt, net_plug_electric_stat_t);
		memset(es, 0, sizeof(net_plug_electric_stat_t));
		es->action = PLUG_ELECTRIC_STAT_QUERY;
		
		user_add_pkt(user, pkt);
	}

	// query B: base infomation
	plug->count_q_base++;
	if (plug->quick_query || plug->count_q_base >= plug->timer_q_base) {
		int count;
		
		plug->count_q_base = 0;

		if (plug->slave->has_current_detect) {
			count = sizeof(req_list)/sizeof(u_int16_t);
		} else {
			count = 1;
		}
		
		pkt = pkt_new_v2(CMD_MISC_Q, sizeof(u_int16_t)*count, NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_MISC_Q, NULL, callback_plug_request);
		req = get_pkt_payload(pkt, u_int16_t);

		for (i = 0; i < count; i++, req++) {
			*req = htons(req_list[i]);
		}
		
		user_add_pkt(user, pkt);
	}

	// query C: 定时器
	plug->count_q_timer++;
	if (plug->quick_query || plug->count_q_timer >= plug->timer_q_timer) {
		plug->count_q_timer = 0;

		pkt = pkt_new_v2(CMD_PLUG_TIMER_Q, sizeof(net_plug_timer_config_t), 
				NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_PLUG_TIMER_Q, NULL, callback_plug_request);
		tc = get_pkt_payload(pkt, net_plug_timer_config_t);
		memset(tc, 0, sizeof(*tc));
		tc->action = AC_QUERY;
		
		user_add_pkt(user, pkt);
	}

	plug->quick_query = false;

	return 0;
}

void plug_query_start(plug_t *plug, cln_plug_t *cp)
{
	log_info("Plug query start, callback=%p, timer=%d\n", cp->callback, cp->query_seconds);
	
	plug->callback = cp->callback;
	plug->callback_handle = cp->callback_handle;
	
	plug->timer_q_stat = cp->query_seconds;
	if (plug->timer_q_stat < 0)
		plug->timer_q_stat = 1;
	plug->timer_q_base = plug->timer_q_stat;
}

void plug_query_stop(plug_t *plug)
{
	log_info("Plug query stop\n");
	
	plug->callback = NULL;
	plug->callback_handle = NULL;

	plug->timer_q_base = DFL_PLUG_TIMER_Q_BASE;
	plug->timer_q_stat = DFL_PLUG_TIMER_Q_STAT;
	plug->timer_q_timer = DFL_PLUG_TIMER_Q_TIMER;	
}

void plug_quick_query(plug_t *plug)
{
	plug->quick_query = true;
	
	CL_THREAD_OFF(plug->t_query);
	CL_THREAD_TIMER_ON(&cl_priv->master, plug->t_query, timer_plug_query, (void *)plug, 0);
}

void plug_set_on(plug_t *plug, int index, int on)
{
	pkt_t *pkt;
	u_int16_t *req;
	user_t *user = plug->slave->user;

	pkt = pkt_new_v2(CMD_MISC_Q, sizeof(u_int16_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_MISC_Q, NULL, callback_plug_request);
	req = get_pkt_payload(pkt, u_int16_t);

	*req = htons(on ? MT_PLUG_SET_ON : MT_PLUG_SET_OFF);
	
	user_add_pkt(user, pkt);

	plug_quick_query(plug);
}

void plug_set_timer(plug_t *plug, int index, cl_plug_timer_t *pt)
{
	pkt_t *pkt;
	user_t *user = plug->slave->user;
	net_plug_timer_config_t *nptc;

	pkt = pkt_new_v2(CMD_PLUG_TIMER_Q, sizeof(net_plug_timer_config_t) + sizeof(net_plug_timer_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_PLUG_TIMER_Q, NULL, callback_plug_request);
	nptc = get_pkt_payload(pkt, net_plug_timer_config_t);

	memset(nptc, 0, sizeof(net_plug_timer_config_t) + sizeof(net_plug_timer_t));
	nptc->action = pt->id == 0 ? AC_ADD : AC_MOD;
	nptc->num = 1;
	nptc->plug_timer[0].id = pt->id;
	nptc->plug_timer[0].hours = pt->hour;
	nptc->plug_timer[0].minute = pt->minute;
	nptc->plug_timer[0].week = pt->week;
	nptc->plug_timer[0].enable = pt->enable;
	nptc->plug_timer[0].last = htons(pt->last);
	strncpy(nptc->plug_timer[0].name, pt->name, sizeof(nptc->plug_timer[0].name));
	
	user_add_pkt(user, pkt);

	plug_quick_query(plug);
}

void plug_del_timer(plug_t *plug, int index, int id)
{
	pkt_t *pkt;
	user_t *user = plug->slave->user;
	net_plug_timer_config_t *nptc;

	pkt = pkt_new_v2(CMD_PLUG_TIMER_Q, sizeof(net_plug_timer_config_t) + sizeof(net_plug_timer_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_PLUG_TIMER_Q, NULL, callback_plug_request);
	nptc = get_pkt_payload(pkt, net_plug_timer_config_t);

	memset(nptc, 0, sizeof(net_plug_timer_config_t) + sizeof(net_plug_timer_t));
	nptc->action = AC_DEL;
	nptc->num = 1;
	nptc->plug_timer[0].id = (u_int8_t)id;
	
	user_add_pkt(user, pkt);

	plug_quick_query(plug);
}

/////////////////////////////////////////////////////////////////////////

static void do_electric_stat(plug_t *plug, pkt_t *pkt)
{
	net_plug_electric_stat_t *es;

	es = get_pkt_payload(pkt, net_plug_electric_stat_t);
	if (es->action != PLUG_ELECTRIC_STAT_QUERY)
		return;

	plug->electric_stat_total = ntohl(es->item.electric_stat_total);
	plug->electric_stat_section = ntohl(es->item.electric_stat_section);
	plug->section_time = ntohl(es->item.time);

	// 这里不单独触发事件，由处理MISC报文中的电流、电压、温度来触发
	/*
	if (plug->callback != NULL) {
		event_push(plug->callback, PE_QUERY, (void *)slave->handle, plug->callback_handle);
	}
	*/
}

void plug_clear_electric_stat(plug_t *plug, int idx)
{
	user_t *user = plug->slave->user;
	pkt_t *pkt;
	net_plug_electric_stat_t *es;	

	pkt = pkt_new_v2(CMD_PLUG_ELECTRIC_STAT, sizeof(net_plug_electric_stat_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, plug->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, plug->slave->handle, CMD_PLUG_ELECTRIC_STAT, NULL, callback_plug_request);
	es = get_pkt_payload(pkt, net_plug_electric_stat_t);
	memset(es, 0, sizeof(net_plug_electric_stat_t));
	es->action = PLUG_ELECTRIC_STAT_CLS;
	es->item.time = htonl(get_sec());
	
	user_add_pkt(user, pkt);

	plug_quick_query(plug);
}

static void do_timer_a(plug_t *plug, pkt_t *pkt)
{
	net_header_t *hdr;
	net_plug_timer_config_t *tc;

	hdr = (net_header_t *)pkt->data;
	
	tc = get_pkt_payload(pkt, net_plug_timer_config_t);
	if (tc->action == AC_ADD || tc->action == AC_MOD|| tc->action == AC_DEL) {
		if (tc->num == 0) {
			log_err(false, "%s do_timer_a action=%d, but num=0\n", plug->slave->str_sn, tc->action);
			return;
		}
		if (tc->plug_timer[0].errorcode == ERR_NONE) {
			event_push(plug->callback, PE_TIMER_SET_OK, plug->handle, plug->callback_handle);
		} else {
			log_err(false, "%s do_timer_a action=%d, err=%u\n", plug->slave->str_sn, tc->action, tc->plug_timer[0].errorcode);
			event_push_err(plug->callback, PE_TIMER_SET_FAIL, plug->handle, plug->callback_handle, tc->plug_timer[0].errorcode);
		}
		return;
	}
	
	if (tc->action != AC_QUERY)
		return;

	// 判断是否改变
	if (plug->timer != NULL && memcmp(plug->timer, tc, hdr->param_len) == 0)
		return;
	
	SAFE_FREE(plug->timer);
	plug->timer = cl_malloc(hdr->param_len);
	memcpy(plug->timer, tc, hdr->param_len);

	if (plug->callback != NULL) {
		event_push(plug->callback, PE_TIMER_MODIFY, plug->handle, plug->callback_handle);
	}
}

// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
bool plug_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	slave_t *slave;
	plug_t *plug;
	wait_t *w;

	w = wait_lookup(PKT_HANDLE(pkt));
	if (w == NULL || w->obj_type != HDLT_SLAVE) {
		return false;
	}
	
	slave = slave_lookup_by_handle(user, w->obj_handle);
	if (slave == NULL) {
		log_debug("plug ignore cmd=%u, not found slave by handle=0x%08x\n", hdr->command, PKT_HANDLE(pkt));
		return false;
	}

	if ((plug = slave->plug) == NULL)
		return false;
	log_debug("plug_proc_tcp sn=%s, cmd=%u, handle=0x%08x\n", slave->str_sn, hdr->command, PKT_HANDLE(pkt));

	switch (hdr->command) {
	case CMD_PLUG_ELECTRIC_STAT:
		do_electric_stat(plug, pkt);
		break;

	case CMD_PLUG_TIMER_A:
		do_timer_a(plug, pkt);
		break;

	default:
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////

bool plug_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	cln_plug_t *cp;
	slave_t *slave;

	if (pkt->type < CLNE_PLUG_START || pkt->type > CLNE_PLUG_MAX) {
		return false;
	}
	
	cp = (cln_plug_t *)&pkt->data[0];
	slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, cp->slave_halde);
	if (slave == NULL) {
		log_err(false, "plug_proc_notify failed: not found slave handle=0x%08x\n", cp->slave_halde);
		*ret = RS_NOT_FOUND;
		return true;
	}

	if ( ! slave->has_plug ) {
		log_err(false, "plug_proc_notify failed: %s not spport plug slave handle=0x%08x\n",
			slave->str_sn, cp->slave_halde);
		*ret = RS_NOT_SUPPORT;
		return true;
	}
	
	switch (pkt->type) {
	case CLNE_PLUG_QUERY_START:
		plug_query_start(slave->plug, cp);
		*ret = RS_OK;
		break;

	case CLNE_PLUG_QUERY_STOP:
		plug_query_stop(slave->plug);
		*ret = RS_OK;
		break;

	case CLNE_PLUG_SET_ON:
		plug_set_on(slave->plug, cp->index, cp->on);
		break;

	case CLNE_PLUG_SET_TIMER:
		plug_set_timer(slave->plug, cp->index, cp->timer);
		break;

	case CLNE_PLUG_DEL_TIMER:
		plug_del_timer(slave->plug, cp->index, cp->id);
		break;

	case CLNE_PLUG_ELECTRIC_STAT_CLS:
		plug_clear_electric_stat(slave->plug, cp->index);
		break;

	default:
		return false;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////


plug_t *plug_alloc(slave_t *slave)
{
	plug_t *plug;
	
	plug = cl_calloc(sizeof(plug_t), 1);
	plug->slave = slave;

	plug->timer_q_base = DFL_PLUG_TIMER_Q_BASE;
	plug->timer_q_stat = DFL_PLUG_TIMER_Q_STAT;
	plug->timer_q_timer = DFL_PLUG_TIMER_Q_TIMER;

	return plug;
}

void plug_free(plug_t *plug)
{
	if (plug == NULL)
		return;

	CL_THREAD_OFF(plug->t_query);

	if (plug->timer != NULL)
		cl_free(plug->timer);
	cl_free(plug);
}

