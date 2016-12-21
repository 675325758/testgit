#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
#include "wait_server.h"
#include "ds_proto.h"
#include "cmd_misc.h"
#include "intelligent_forward_priv.h"


static RS if_proc_q(cl_notify_pkt_t *notify_pkt)
{
	cln_if_query_t *ci;
	user_t *user;
	pkt_t *pkt;
	u_int8_t i = 0;
	u_int16_t *req;
	u_int16_t req_list[] = {MT_DEV_GET_TMEP, MT_DEV_GET_RH, MT_DEV_GET_PM25, MT_DEV_GET_VOC};

	ci = (cln_if_query_t *)&notify_pkt->data[0];
	user = (user_t *)lookup_by_handle(HDLT_USER, ci->dev_handle);
	if (!user) {
		return RS_ERROR;
	}

	pkt = pkt_new_v2(CMD_MISC_Q, sizeof(req_list), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	if (!pkt) {
		return RS_ERROR;
	}
	req = get_pkt_payload(pkt, u_int16_t);
	
	for (i = 0; i < sizeof(req_list)/sizeof(u_int16_t); i++, req++) {
		*req = htons(req_list[i]);
	}
	
	PKT_HANDLE(pkt) = ci->dev_handle;
	log_debug("ready send pkt for if_proc_q\n");

	user_add_pkt(user, pkt);
	return RS_OK;
}

bool if_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;

	switch(pkt->type){
		case CLNE_IF_QUERY:
			*ret = if_proc_q(pkt);
			break;
		default:
			res =false;
			break;
	}
	return res;
}

