#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_server.h"
#include "uasc_priv.h"
#include "uas_client.h"

static bool _uasc_start_connet_to_server(cln_common_info_t *info,RS* ret)
{
	uasc_session_t * s = (uasc_session_t*)cl_priv->uasc_session;

	if(s != NULL){
		if(s->status == UASCS_ERROR){
			uasc_set_status(s, UASCS_IDLE);
		}
	}

	return true;
}

static bool _uasc_push_app_stat_info(cln_common_info_t *info,RS* ret)
{
	tlv_manger_t tm = {0};
	cl_app_stat_info_t* as = (cl_app_stat_info_t*)(&info->u.u8_data[0]);
	pkt_t* pkt;
	void * dest;
    uasc_session_t * s = (uasc_session_t*)cl_priv->uasc_session;
    if (!s) {
        *ret = RS_INVALID_PARAM;
        return false;
    }

	
	tlvm_init(&tm);
	
	tlvm_add_tlv(&tm,AS_TT_UUID,sizeof(cl_priv->uuid_bin),cl_priv->uuid_bin);
	tlvm_add_string_tlv(&tm,AS_TT_APP_VERSION,as->app_version_info);
	tlvm_add_u16_tlv(&tm,AS_TT_HW_TYPE,as->hard_ware_type,true);
	tlvm_add_string_tlv(&tm,AS_TT_HW_DESC,as->manufacturer_info);
	tlvm_add_u16_tlv(&tm,AS_TT_OS_TYPE,cl_priv->cleint_type,true);
	tlvm_add_string_tlv(&tm,AS_TT_OS_VERSION,as->os_info);
	tlvm_add_u32_tlv(&tm,AS_TT_APP_TYPE,cl_priv->app_type,true);
	
	pkt = uasc_pkt_new(CMD_UDP_APP_REPORT_RUNENV, tm.used_len, 
				true, false, 0, s->client_sid, 0, s->my_request_id);

	dest = get_uascp_payload(pkt, void);
	memcpy(dest,tm.tlv_data,tm.used_len);

	uasc_request_add(cl_priv->uasc_session,pkt);

	tlvm_free_data(&tm);
	return true;
}

bool uasc_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = false;
	cln_common_info_t *info;

	if(pkt->type != CLNE_UASC_SERVER_CTRL|| cl_priv->uasc_session == NULL){
		return false;
	}
	
	info = (cln_common_info_t *)&pkt->data[0];
	
	switch(info->action){
		case ACT_UASC_START_CONNECT:
			res = _uasc_start_connet_to_server(info,ret);
			break;
		case ACT_UASC_PUSH_APP_STAT:
			res = _uasc_push_app_stat_info(info,ret);
			break;
		default:
			break;
	}

	return res;	
}


