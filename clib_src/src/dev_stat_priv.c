#include "client_lib.h"
#include "cl_priv.h"
#include "cl_user.h"
#include "smart_appliance_priv.h"

static void copy_ni(cl_ni_t *cni, misc_ni_item_t *nii)
{
	cni->name = cl_strdup(nii->name == NULL ? "" : nii->name);
	cni->is_up = !(!(nii->flags & MNIF_CONNECT));
	cni->is_ip_valid = !(!(nii->flags & MNIF_IP_INVALID));
	cni->ip = ntohl(nii->ip);
	cni->mtu = ntohs(nii->mtu);
	cni->rx_byte = ntoh_ll(nii->rx_bytes);
	cni->tx_byte = ntoh_ll(nii->tx_bytes);
}

static cl_lan_client_t *copy_client(misc_client_item_t *ci)
{
	cl_lan_client_t *lc;

	lc = cl_calloc(sizeof(cl_lan_client_t), 1);
	lc->name = cl_strdup(ci->name);
	lc->ip = ntohl(ci->ip);
	memcpy(lc->mac, ci->mac, 6);
	lc->is_from_wifi = ci->is_from_wifi;

	return lc;
}

static RS request_query_slave_stat(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_SLAVE_QUERY_STAT, CLNPF_ACK);
	
	v = (cln_user_t *)&pkt->data[0];
	v->user_handle = slave_handle;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}


// 获取一次，下面立即触发一次查询
CLIB_API cl_dev_stat_t *cl_get_dev_stat_info(cl_handle_t dev_handle)
{
	int i;
	cl_dev_stat_t *ds = NULL;
	user_t *user;
	slave_t *slave;
	cl_handle_t slave_handle = INVALID_HANDLE;
	u_int32_t now;
	dev_info_t *di;

	cl_lock(&cl_priv->mutex);

	// 查找
	if (IS_SAME_HANDLE_TYPE(dev_handle, HDLT_USER)) {
		user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
		if (user == NULL) {
			log_err(false, "cl_get_dev_stat_info not found user 0x%08x\n", dev_handle);
			goto done;
		}
        
        if (user->is_udp_ctrl) {
            ds = sa_get_dev_stat_info(user);
            cl_unlock(&cl_priv->mutex);
            return ds;
        }
        
		if (stlc_list_empty(&user->slave)) {
			log_err(false, "cl_get_dev_stat_info %s have no slave\n", user->name);
			goto done;
		}
		slave = stlc_list_entry(user->slave.next, slave_t, link);
	} else {
		slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, dev_handle);
	}

	if (slave == NULL) {
		log_err(false, "cl_get_dev_stat_info not found slave 0x%08x\n", dev_handle);
		goto done;
	}

	slave_handle = slave->handle;
	di = &slave->dev_info;

	ds = cl_calloc(sizeof(cl_dev_stat_t), 1);
	
	/// 拷贝信息
	ds->handle = slave->handle;
	ds->sn = slave->sn;

	ds->cpu = di->cpu;
	ds->mem = di->mem;
	
	now = get_sec();
	di = &slave->dev_info;
	if (di->query_uptime != 0)
		ds->uptime = di->uptime + (now - di->query_uptime);
	if (di->query_online != 0)
		ds->online = di->online + (now - di->query_online);
	if (di->query_conn_internet != 0)
		ds->conn_internet = di->conn_internet + (now - di->query_conn_internet);
	memcpy(&ds->soft_version, &di->version.soft_version, sizeof(ds->soft_version));
	memcpy(&ds->upgrade_version, &di->version.upgrade_version, sizeof(ds->upgrade_version));
	if (di->can_update ) {
		ds->can_update = true;
		ds->can_auto_update = di->can_auto_update?true:false;
		memcpy(&ds->new_version, &di->new_version, sizeof(ds->new_version));
		if (di->release_desc)
			ds->release_desc = cl_strdup(di->release_desc);
		if (di->release_url)
			ds->release_url = cl_strdup(di->release_url);
		if (di->release_date)
			ds->release_date = cl_strdup(di->release_date);
	} else {
		ds->can_update = false;
		ds->can_auto_update = false;
	}
	
	ds->ap_ssid = cl_strdup(di->ap_ssid);
	ds->ap_passwd = cl_strdup(di->ap_passwd);

	// 内外网接口统计信息
	copy_ni(&ds->wan, &di->wan);
	copy_ni(&ds->lan, &di->lan);

	if (di->clients != NULL) {
		misc_client_item_t *ci;
		
		ds->client_num = di->clients->count;
		ds->clients = cl_calloc(sizeof(void *), ds->client_num);
		
		for (i = 0; i < di->clients->count; i++) {
			ci = (misc_client_item_t *)BOFP(&di->clients->items[0], di->clients->item_len*i);
			ds->clients[i] = copy_client(ci);
		}
	}

done:
	cl_unlock(&cl_priv->mutex);

	if (slave_handle != INVALID_HANDLE && slave->user != NULL && !slave->user->is_udp_ctrl)
		request_query_slave_stat(slave_handle);
	
	return ds;
}

// 获取产测参数，下面立即触发一次查询
CLIB_API cl_pt_stat_t *cl_get_pt_stat_info(cl_handle_t dev_handle)
{
	cl_pt_stat_t *ds = NULL;
	user_t *user;

	cl_lock(&cl_priv->mutex);

	user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
		if (user == NULL) {
		log_err(false, "cl_get_dev_stat_info not found user 0x%08x\n", dev_handle);
		goto done;
	}

	if (!user->is_udp_ctrl) {
		goto done;
	}
	ds = sa_get_pt_stat_info(user);

done:
	cl_unlock(&cl_priv->mutex);	
	return ds;
}

CLIB_API void cl_free_pt_stat_info(cl_pt_stat_t * p)
{
	SAFE_FREE(p);
}

CLIB_API void cl_free_dev_stat_info(cl_dev_stat_t * info)
{
	int i;
	cl_lan_client_t *lc;

	SAFE_FREE(info->ap_ssid);
	SAFE_FREE(info->ap_passwd);

	for (i = 0; i < info->client_num; i++) {
		lc = info->clients[i];
		SAFE_FREE(lc->name);
		cl_free(lc);
	}
	SAFE_FREE(info->clients);
	SAFE_FREE(info->wan.name);
	SAFE_FREE(info->lan.name);
	SAFE_FREE(info->release_desc);
	SAFE_FREE(info->release_url);
    SAFE_FREE(info->stm_release_url);
	SAFE_FREE(info->release_date);
	SAFE_FREE(info->udp_dev_stat.stm_32_dbg_info);
	cl_free(info);
}


