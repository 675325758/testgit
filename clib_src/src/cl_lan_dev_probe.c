/*
 本文件实现局域网设备扫描的程序接口
 */
#include "lan_dev_probe_priv.h"
#include "cl_lan_dev_probe.h"
#include "cl_priv.h"

//static RS cl_do_lan_probe_action(u_int32_t type)
//{
//    cl_notify_pkt_t *pkt;
//	RS ret;
//    
//	CL_CHECK_INIT;
//   
//	pkt = cl_notify_pkt_new(128, type, CLNPF_ACK);
//	pkt->param_len = 0x0;
//    
//	ret = cl_send_notify(&cl_priv->thread_main, pkt);
//	cl_notify_pkt_free(pkt);
//    
//	return ret;
//}

CLIB_API RS cl_set_probe_callback(cl_callback_t callback, void *handle)
{
    cl_notify_pkt_t *pkt;
    cln_lan_dev_probe_t* ldp;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(128, CLNE_LDP_SET_CALLBACK, CLNPF_ACK);
    ldp = (cln_lan_dev_probe_t*)pkt->data;
	pkt->param_len = sizeof(cln_lan_dev_probe_t);
    ldp->callback = callback;
    ldp->handle = handle;
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_reset_probe_dev_list(void)
{
	dev_probe_info_t* dpi, *n;	
    lan_dev_probe_ctrl_t* ldpc;
	
	CL_CHECK_INIT;

	ldpc = cl_priv->ldpc;
    if (!ldpc) {
        return RS_OK;
    }

	cl_lock(&ldpc->mutex);

	stlc_list_for_each_entry_safe(dev_probe_info_t, dpi, n, &ldpc->dev_info_list, link) {
		CL_THREAD_TIMER_OFF(dpi->time_out);
		CL_THREAD_TIMER_OFF(dpi->t_timer);

		stlc_list_del(&dpi->link);

		cl_free(dpi);
	}

	cl_unlock(&ldpc->mutex);

	return RS_OK;
}


CLIB_API RS cl_get_probe_dev_list(cl_lan_dev_list** list)
{
    cl_lan_dev_info* item;
    lan_dev_probe_ctrl_t* ldpc;
    cl_lan_dev_list* tlist;
    dev_probe_info_t* dpi;
    int count = 0;
    
    CL_CHECK_INIT;
    
    if (!list) {
        return RS_INVALID_PARAM;
    }
    *list = NULL;
    tlist = NULL;
    
    ldpc = cl_priv->ldpc;
    if (!ldpc) {
        return RS_OK;
    }
    
    cl_lock(&ldpc->mutex);
    
    stlc_list_count(count, &ldpc->dev_info_list);
    if (count<=0) {
        goto unlock_out;
    }
    
    tlist = cl_calloc(sizeof(*tlist)+count*sizeof(cl_lan_dev_info), 1);
    if (!tlist) {
        goto unlock_out;
    }
    
    stlc_list_for_each_entry(dev_probe_info_t, dpi, &ldpc->dev_info_list, link){
        if ((int)tlist->dev_count >= count) {
            break;
        }
        item = (cl_lan_dev_info*)&tlist->info[tlist->dev_count++];
        item->handle = dpi->handle;
        item->dev_sn = dpi->dev_sn;
        item->dev_type = dpi->sub_type;
        item->exp_type = dpi->ext_type;
		memcpy(item->developer_id, dpi->developer_id, sizeof(item->developer_id));
        item->real_ext_type = dpi->real_ext_type;
        item->dev_run_mode = dpi->dev_run_mode;
        item->last_alive_time = dpi->recv_time;
        item->ip_addr = dpi->peer_ip;
        item->sm_success_time = dpi->sm_success_time;
        item->is_upgrading = (dpi->flags & CFGPF_UPGRADING) ? 1 : 0;
		item->is_udp_ctrl = (dpi->flags & CFGPF_UDP_CTRL) ? 1 : 0;
		item->evm_is_upgrading = dpi->evm_is_upgrading;
		memcpy((void *)&item->la_info, (void *)&dpi->la_info, sizeof(item->la_info));
    }
    
    *list = tlist;
    tlist = NULL;
    
unlock_out:
    cl_unlock(&ldpc->mutex);
    
    if (tlist) {
        cl_free_probe_dev_list(tlist);
    }
    
    return RS_OK;
}

CLIB_API void cl_free_probe_dev_list(cl_lan_dev_list* list)
{
    if (list) {
        cl_free(list);
    }
}

CLIB_API RS cl_dev_auth(cl_lan_dev_info *dev, const char *pwd)
{
	cl_notify_pkt_t *pkt;
	cln_lan_dev_auth_t *up;
	RS ret;

	if(dev == NULL || pwd == NULL)
		return RS_INVALID_PARAM;
	
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_LAN_AUTH, CLNPF_ACK);
	if (pkt == NULL)
		return RS_MEMORY_MALLOC_FAIL;

	up = (cln_lan_dev_auth_t*)pkt->data;
	up->dev_sn = dev->dev_sn;	
	hash_passwd((u_int8_t*)up->md5_pwd, (char*)pwd);
	pkt->param_len = sizeof(*up);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_wifi_config(cl_lan_dev_info *dev, const char *ssid, const char *pwd)
{
	cl_notify_pkt_t *pkt;
	cln_lan_wifi_cfg_t *up;
	RS ret;

	if(dev == NULL || ssid == NULL || pwd == NULL)
		return RS_INVALID_PARAM;
	
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(4096, CLNE_LAN_WIFI_CFG, CLNPF_ACK);
	if (pkt == NULL)
		return RS_MEMORY_MALLOC_FAIL;

	up = (cln_lan_wifi_cfg_t*)pkt->data;
	up->dev_sn = dev->dev_sn;
	up->len_ssid = (u_int16_t)strlen(ssid) + 1;
	up->len_pwd = (u_int16_t)strlen(pwd) + 1;
	memcpy(up->data, ssid, up->len_ssid);
	memcpy(&up->data[up->len_ssid], pwd, up->len_pwd);
	
	pkt->param_len = sizeof(*up) + up->len_ssid + up->len_pwd;
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API u_int64_t cl_get_ap_dest_sn()
{
	if(cl_priv->ldpc != NULL){
		return cl_priv->ldpc->ap_dest_sn;
	}
	return 0;
}

