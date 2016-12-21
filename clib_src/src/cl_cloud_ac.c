#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
#include "cl_cloud_ac.h"


CLIB_API RS cl_cloud_match(cl_handle_t ac_handle, bool do_match, u_int8_t match_type)
{
	cl_notify_pkt_t *pkt;
	cln_cloud_match_t *match;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_CA_MATCH, CLNPF_ACK);
	
	match = (cln_cloud_match_t *)&pkt->data[0];

	if (do_match) {
		match->action = 2;
	} else {
		match->action = 3;
	}
	match->ac_handle = ac_handle;
	match->match_type = match_type;
	
	pkt->param_len = sizeof(*match);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;


}

CLIB_API RS cl_ac_set_match_id(cl_handle_t ac_handle, u_int16_t select_match_id, u_int8_t match_type)
{
	cl_notify_pkt_t *pkt;
	cln_cloud_match_t *match;
	RS ret;

	//if(select_match_id >= 4 ) {
		//return RS_INVALID_PARAM;
	//}
	
	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_CA_SET_MATCH_ID, CLNPF_ACK);
	
	match = (cln_cloud_match_t *)&pkt->data[0];

	match->action = 4;
	match->ac_handle = ac_handle;
	match->match_type = match_type;
	match->select_match_id = select_match_id;
	pkt->param_len = sizeof(*match);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;

}

CLIB_API RS cl_ac_send_ctl(cl_handle_t ac_handle,u_int8_t onoff, u_int8_t temp, u_int8_t mode, u_int8_t speed, u_int8_t dir, u_int8_t presskey, u_int8_t oldkey)
{
	cln_ac_ctrl_t *ac_ctrl;
	cl_notify_pkt_t *pkt;
	RS ret;
	cln_ac_key_id_t *key_id;
	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_CA_CTRL, CLNPF_ACK);
	
	ac_ctrl = (cln_ac_ctrl_t *)&pkt->data[0];

	key_id = (cln_ac_key_id_t *)&ac_ctrl->key_id;
	key_id->onoff = onoff;
	key_id->mode = mode;
	key_id->temp = temp;
	key_id->speed = speed;
	key_id->dir = dir;
	key_id->key_v = presskey;
	key_id->oldkey_v = oldkey;
	
	ac_ctrl->ac_handle = ac_handle;
	
	pkt->param_len = sizeof(*ac_ctrl);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}


