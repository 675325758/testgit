/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: APP上层二次开发通道
**  File:    app_pkt_priv.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    07/26/2016
**
**  Purpose:
**    APP上层二次开发通道.
**************************************************************************/


/* Include files. */
#include "app_pkt_priv.h"

#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"
#include "udp_device_common_priv.h"
#include "cl_qpcp.h"
#include "rfgw_priv.h"
#include "linkage_client.h"
#include "linkage_priv.h"
#include "cl_priv.h"
#include "cl_thread.h"

static u_int8_t g_developer_id[32] = {0};

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

/* Macro constant definitions. */


/* Type definitions. */


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
static cl_app_proc_fun app_proc = NULL;

static RS app_send_pkt_to_wifi_dev(u_int64_t sn, u_int8_t *data, int data_len)
{
	user_t *user;
	ucc_session_t *s;
	pkt_t *pkt;
	int len;
	u_int8_t *hdr;

	log_debug("app send pkt to sn %"PRIu64"\n", sn);

	user = user_lookup_by_sn(sn);
	if (!user) {
		log_err(false, "user not found\n");
		return RS_INVALID_PARAM;
	}

	if ((s = user->uc_session) == NULL) {
		return RS_NOT_INIT;
	}

	len = data_len;
    
    pkt = uc_pkt_new(s, CMD_APP_DEV_USER, len, true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return RS_ERROR;
	}

	hdr = get_ucp_payload(pkt, u_int8_t);

	memcpy(hdr, data, data_len);
    
    ucc_request_add(s, pkt);
	
	return RS_OK;
}

RS app_proc_pkt_from_wifi_dev(u_int64_t sn, u_int8_t *data, int data_len)
{
	log_debug("app_proc_pkt_from_wifi_dev data_len %u\n", data_len);	

	if (app_proc != NULL) {
		app_proc(pt_wifi_dev, sn, (char *)data, data_len);
	}

	return RS_OK;
}

static RS app_send_pkt_to_macbee_dev(u_int64_t sn, u_int8_t *data, int data_len)
{
	user_t *user = NULL; 
	slave_t *slave = NULL;
	bool found = false;
	u_int8_t buf[512] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	log_debug("app send pkt to macbee sn %"PRIu64"\n", sn);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->sn == sn) {
				found = true;
				goto done;
			}
		}
	}

done:
	if (!found) {
		log_err(false, "not found slave sn %"PRIu64"\n", sn);
		return RS_INVALID_PARAM;
	}

	log_debug("found slave %"PRIu64" at user %llu\n", slave->sn, user->sn);

	if (data_len >= 26) {
		rf_slave_send_big_pkt_v2(slave, RF_BIG_PKT_COMMON, data, data_len, NULL);
	} else {
		tlv->type = RF_TT_CMD_RAWDATA;
		tlv->len = data_len;
		memcpy((char*)&tlv[1], data, data_len);
		rfgw_send_tt_packet(slave, buf, (u_int16_t)(data_len + sizeof(*tlv)));
	}
	
	return RS_OK;
}

RS app_proc_pkt_from_macbee_dev(u_int64_t sn, u_int8_t *data, int data_len)
{

	log_debug("app_proc_pkt_from_macbee_dev data_len %u\n", data_len);	

	if (app_proc != NULL) {
		app_proc(pt_macbee_dev, sn, (char *)data, data_len);
	}

	return RS_OK;
}

RS app_proc_pkt_from_macbee_dev_cache(u_int64_t sn, u_int8_t *data, int data_len)
{

	log_debug("app_proc_pkt_from_macbee_dev_cache data_len %u\n", data_len);	

	if (app_proc != NULL) {
		app_proc(pt_macbee_dev_cache, sn, (char *)data, data_len);
	}

	return RS_OK;
}


/**
	到联动服务器
*/
static RS app_send_pkt_to_server(u_int8_t *data, int data_len)
{
	pkt_t *pkt;
	u_int8_t *hdr;
	ucla_session_t *s;
	int len;

	s = ucla_get_any_enstablis_session();
	if (!s) {
		log_debug("linkserver not ready, app_send_pkt_to_server failed\n");
		return RS_ERROR;
	}

	log_debug("app send pkt to server data_len %d\n", data_len);
	
	len = data_len;

	pkt = ucla_pkt_new(CMD_APP_LINKAGE_USER, len, 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	if (!pkt) {
		return RS_ERROR;
	}

	hdr = get_uascp_payload(pkt, u_int8_t);
	memcpy(hdr, data, data_len);

	ucla_request_add(s, pkt);

	return RS_OK;
}


RS app_proc_pkt_from_server(u_int8_t *data, int data_len)
{

	log_debug("app_proc_pkt_from_server  data_len %u\n", data_len);	

	if (app_proc != NULL && data_len) {
		app_proc(pt_server, 0, (char *)data, data_len);
	}

	return RS_OK;
}

RS app_send_pkt(PEER_TYPE_T peer_type, u_int64_t ident, u_int8_t *pkt, int pkt_len)
{
	RS ret = RS_INVALID_PARAM;
	
	switch (peer_type) {
		case pt_wifi_dev:
			ret = app_send_pkt_to_wifi_dev(ident, pkt, pkt_len);
			break;
		case pt_macbee_dev:
			ret = app_send_pkt_to_macbee_dev(ident, pkt, pkt_len);
			break;
		case pt_server:
			ret = app_send_pkt_to_server(pkt, pkt_len);
			break;
		default:
			break;
	}
	
	return 0;
}

u_int8_t* app_get_developer_id(void)
{
	return g_developer_id;
}

CLIB_API RS cl_send_pkt(PEER_TYPE_T peer_type, u_int64_t ident, char* data , int data_len)
{
	RS ret = RS_OK;
	cl_notify_pkt_t *pkt;
	cl_app_pkt_send_t *u;

	CL_CHECK_INIT;
	pkt = cl_notify_pkt_new(4096, CLNE_APP_PKT_SEND, CLNPF_ACK);
	
	u = (cl_app_pkt_send_t *)&pkt->data[0];
	u->type = peer_type;
	u->ident = ident;
	u->pkt_len = data_len;
	pkt->param_len = sizeof(cl_app_pkt_send_t) + data_len;

	memcpy(u->pkt, data, data_len);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;	
}

CLIB_API RS cl_set_proc_pkt_callback(cl_app_proc_fun fun)
{
	cl_lock(&cl_priv->mutex);

	app_proc = fun;

	cl_unlock(&cl_priv->mutex);

	return RS_OK;
}

CLIB_API RS cl_set_developer_id(char *id)
{
	if (strlen(id) < 32) {
		return RS_ERROR;
	}

	memcpy(g_developer_id, id, 32);
	return RS_OK;
}

