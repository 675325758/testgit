#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
#include "wait_server.h"
#include "ds_proto.h"
#include "cloud_ac_priv.h"
#include "cl_cloud_ac.h"
#include "equipment_priv.h"


static void callback_ca_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	user_t *user;

	if ((user = (user_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_ca_request, not found user type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	switch (w->cmd) {
		
	default:
		log_err(false, "callback_ca_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
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

	default:
		log_err(false, "callback_eq_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}

static bool is_cloud_ac_equipment(equipment_t* eq)
{
    return (eq->attr.dev_type == REMOTE_TYPE_CLOUD_AIRCONDITION)?true:false;

}

static int timer_cm_query(cl_thread_t *t)
{
	equipment_ctrl_t *ec;
	user_t *user;

	ec = (equipment_ctrl_t *)CL_THREAD_ARG(t);
	user = ec->slave->user;
	
	ec->t_cm_query = NULL;
	//推送匹配成功
	event_push(user->callback, CA_DONE, user->handle, user->callback_handle);

	
	return 0;
}
static void do_match_ok(equipment_t *eq, net_cloud_match_t *match)
{
	u_int8_t i = 0;
	log_debug("do_match_ok\n");
	//匹配成功做一次快速查询
	eq_force_update_all_info(eq->ec);
	
	eq->match_id_num = match->match_id_num;
	//获取编码库ID
	for (i = 0; i < eq->match_id_num; i++) {
		eq->match_id[i] = htons(match->match_id[i]);

	}
	//等先查询一下电器的信息再推送匹配成功
    CL_THREAD_TIMER_ON(&cl_priv->master, eq->ec->t_cm_query, timer_cm_query, (void *)eq->ec, 500);
	
}

static RS ca_proc_match(cl_notify_pkt_t *notify_pkt)
{
	cln_cloud_match_t *cc;
	user_t *user;
	equipment_t *eq;
	pkt_t *pkt;
	net_cloud_match_t *match;
	cc = (cln_cloud_match_t *)&notify_pkt->data[0];
	eq = (equipment_t *)lookup_by_handle(HDLT_EQUIPMENT, cc->ac_handle);
	if (!eq) {
		log_debug("not find eq\n");
		return RS_ERROR;
	}
	user = eq->ec->slave->user;
	
	pkt = pkt_new_v2(CMD_CLOUD_MATCH, sizeof(net_cloud_match_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (!pkt) {
		return RS_ERROR;
	}
	match = get_pkt_payload(pkt, net_cloud_match_t);

	match->err = 0;
	match->action = cc->action;
	match->local_id = htons(eq->attr.local_id);;
	match->type = cc->match_type;
	
	
    PKT_HANDLE(pkt) = wait_add(HDLT_EQUIPMENT, cc->ac_handle, CMD_CLOUD_MATCH, NULL, callback_ca_request);
	
	log_debug("ready send pkt for ca_proc_match, handle:0x%08x\n", cc->ac_handle);

	user_add_pkt(user, pkt);
	return RS_OK;
}

static RS ca_proc_ctrl(cl_notify_pkt_t *notify_pkt)
{
	cln_ac_ctrl_t *cc;
	user_t *user;
	net_remote_ctrl_t* nrc;
	equipment_t *eq;
	pkt_t *pkt;
	u_int8_t i = 0;
	bool is_match = false;
	
	cc = (cln_ac_ctrl_t *)&notify_pkt->data[0];
	eq = (equipment_t *)lookup_by_handle(HDLT_EQUIPMENT, cc->ac_handle);
	if (!eq) {
		log_debug("not find eq\n");
		return RS_ERROR;
	}

	user = eq->ec->slave->user;

	for (i = 0; i < eq->attr.n_state; i++) {
		if ((eq->attr.state[i].state_id == REMOTE_CLOUD_IR_STATUS_VALID) && (eq->attr.state[i].state_value != 0)) {
			is_match = true;
		}
	}

	if (!is_match) {
		event_push(user->callback, CA_NOT_MATCH, user->handle, user->callback_handle);
		log_debug("have not match\n");
		return RS_OK;
	}
	
	pkt = pkt_new_v2(CMD_REMOTE_CTRL, sizeof(net_remote_ctrl_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (!pkt) {
		return RS_ERROR;
	}
	
	nrc = get_pkt_payload(pkt, net_remote_ctrl_t);
    nrc->key_id = htonl((cc->key_id));
	
    nrc->local_id = htons(eq->attr.local_id);

	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, eq->ec->slave->handle, CMD_REMOTE_CTRL, NULL, callback_eq_request);
	log_debug("ready send pkt for ca_proc_ctrl\n");

	user_add_pkt(user, pkt);

	if(is_cloud_ac_equipment(eq)) {
		eq_force_update_all_info(eq->ec);
	}
	
	return RS_OK;
}

static RS ca_proc_set(cl_notify_pkt_t *notify_pkt)
{
	cln_cloud_match_t *cc;
	user_t *user;
	equipment_t *eq;
	pkt_t *pkt;
	net_cloud_match_t *match;
	u_int8_t i = 0;
	bool is_match = false;
	
	cc = (cln_cloud_match_t *)&notify_pkt->data[0];
	eq = (equipment_t *)lookup_by_handle(HDLT_EQUIPMENT, cc->ac_handle);
	if (!eq) {
		log_debug("not find eq\n");
		return RS_ERROR;
	}
	user = eq->ec->slave->user;
	
	
	pkt = pkt_new_v2(CMD_CLOUD_MATCH, sizeof(net_cloud_match_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (!pkt) {
		return RS_ERROR;
	}
	match = get_pkt_payload(pkt, net_cloud_match_t);

	match->err = 0;
	match->action = cc->action;
	match->local_id = htons(eq->attr.local_id);
	match->type = cc->match_type;
	match->set_id = cc->select_match_id;
	
	
    PKT_HANDLE(pkt) = wait_add(HDLT_EQUIPMENT, cc->ac_handle, CMD_CLOUD_MATCH, NULL, callback_ca_request);
	
	log_debug("ready send pkt for ca_proc_set, handle:0x%08x\n", cc->ac_handle);

	user_add_pkt(user, pkt);
	return RS_OK;
}

bool ca_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;

	switch(pkt->type){
		case CLNE_CA_MATCH:
			*ret = ca_proc_match(pkt);
			break;
		case CLNE_CA_CTRL:
			*ret = ca_proc_ctrl(pkt);
			break;
		case CLNE_CA_SET_MATCH_ID:
			*ret = ca_proc_set(pkt);
			break;
		default:
			res =false;
			break;
	}
	return res;
}

 void do_cmd_cm(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	net_cloud_match_t *match;
	wait_t *w;
	equipment_t *eq;
	//查找等待
	w = wait_lookup(PKT_HANDLE(pkt));
	if (w == NULL) {
		return;
	}
	//每收到一次加一下超时时间
	w->timeout += 5;
	eq = (equipment_t *)lookup_by_handle(HDLT_EQUIPMENT, w->obj_handle);
	if (!eq) {
		log_debug("not find eq\n");
		return;
	}
	
	match = get_pkt_payload(pkt, net_cloud_match_t);
	match->err = ntohl(match->err);

	if (match->err != 0) {
		event_push_err(user->callback, CA_ERROR, user->handle, user->callback_handle, match->err);
		return;
	}
	
	switch (match->action) {
	case CMA_REPORT:
		if ((match->status > 0) && (match->status <= CMS_DONE)) {
			if ((CA_BEGIN + match->status) == CA_DONE) {
				do_match_ok(eq, match);
				//通知更新一下数据
				event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
				// 删除对应的等待
				wait_del(hdr->handle);
			} else {
				event_push(user->callback, (CA_BEGIN + match->status), user->handle, user->callback_handle);
			}
		}
		break;
	case CMA_REQUEST:
		event_push(user->callback, CA_WAIT_IR, user->handle, user->callback_handle);
		break;
	case CMA_CANCLE:
		event_push(user->callback, CA_CM_CANNEL, user->handle, user->callback_handle);
		break;
	case CMA_SET:
		// 删除对应的等待
		wait_del(hdr->handle);
		event_push(user->callback, CA_SET_OK, user->handle, user->callback_handle);
		break;
	default:
		return;
	}
}



