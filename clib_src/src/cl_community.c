#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "cl_priv.h"
#include "cl_user.h"
#include "cl_notify.h"
#include "openssl/md5.h"
#include "cl_community.h"
#include "community_priv.h"
#include "ds_key.h"
#include "cl_mem.h"
#include "notify_push_priv.h"
#include "health_priv.h"

#define SENDTO_SLAVE(user, slave, pkt)\
	sendto((user)->sock_udp, (pkt)->data, (pkt)->total, 0, \
	(struct sockaddr *)&((slave)->community->addr), sizeof(struct sockaddr_in ))

#define CHECK_EXPIRE(msg) \
do{\
	if ((msg)->expire == 0) \
		(msg)->expire = EXPIRE_DEFAULT; \
	else if ((msg)->expire > EXPIRE_MAX) \
		(msg)->expire = EXPIRE_MAX; \
}while(0)

static RS check_msg_valid(notify_msg_t *msg)
{
	if (msg == NULL) {
		return RS_ERROR;
	}
	if(msg->msg_format > FMT_STRING)
		return RS_ERROR;
	if (msg->msg_len > 0 && msg->msg == NULL)
		return RS_ERROR;
	if (msg->content_len > 0 && msg->content == NULL)
		return RS_ERROR;
	if ((msg->content_len > MAX_NOTIFY_CONTENT) || (msg->msg_len > MAX_NOTIFY_MSG))
		return RS_ERROR;
	if ((msg->msg_type >= TNOTIFY_EMERGENCY) && (msg->msg_type <= TNOTIFY_AD)) {
		if (msg->msg_format != FMT_HTTP)
			return RS_ERROR;		
	}
	
	CHECK_EXPIRE(msg);
	
	return RS_OK;
}

static RS check_send_to_slave(user_t *user, pkt_t *pkt, slave_t *slave, notify_msg_t *msg)
{
	int r = 0;
		
	/* 到指定设备的单播或广播才发送 */
	if ((slave->sn == msg->dev_sn) || (0 == msg->dev_sn)) {
		r = SENDTO_SLAVE(user, slave, pkt);
		if (r == pkt->total ) {
			return RS_OK;
		}
	}
	
	return RS_ERROR;
}

static RS check_send_to_server(user_t *user, pkt_t *pkt, notify_msg_t *msg, int is_new)
{
	if (is_new) {
		/* 新公告，单播或广播就发送 */
		user_add_pkt(user, pkt);
		return RS_OK;
	} else {
		/* 同步旧公告 */
		if ((user->sn == msg->dev_sn) || (0 == msg->dev_sn)){
			user_add_pkt(user, pkt);
			return RS_OK;
		}
	}	
	return RS_ERROR;
}

static pkt_t *build_notify_msg_old(user_t *user, notify_msg_t *msg, int is_new)
{
	int n;
	pkt_t *pkt;
	net_notify_t *notify;
	net_notify_tlv_t *tlv;
	net_notify_value_t *value;
	
	n = sizeof(*notify) + sizeof(*tlv) + sizeof(*value) + msg->msg_len;
	pkt = pkt_new_v2(CMD_NOTIFY, n, 0, msg->dev_sn, user->ds_type);
	if(pkt == NULL){
		return NULL;
	}
	
	notify = get_pkt_payload(pkt, net_notify_t);
	notify->first_report_id = ntohl((u_int32_t)msg->report_id);
	notify->report_count = 1;
	notify->report_expire = msg->expire;
	notify->reserved = 0;

	tlv = (net_notify_tlv_t*)notify->tlv_data;
	tlv->type = ntohs(msg->msg_type);
	tlv->length = ntohl(sizeof(*value) + msg->msg_len);

	value = (net_notify_value_t *)tlv->value;
	value->timestamp = ntohl(msg->msg_time);
	value->len = ntohl(msg->msg_len);
	if (msg->msg_len )
		memcpy(value->notify_msg, msg->msg, msg->msg_len);
	return pkt;
}

static pkt_t *build_notify_msg(user_t *user, notify_msg_t *msg, int is_new)
{
	int n;
	pkt_t *pkt;
	net_notify_t *notify;
	net_notify_tlv_t *tlv;
	net_notify_value_v2_t *value;

	if(msg->msg_type <= NOTIFY_AD)
		return build_notify_msg_old(user, msg, is_new);
	
	n = sizeof(*notify) + sizeof(*tlv) + sizeof(*value) + msg->msg_len + msg->content_len;
	pkt = pkt_new_v2(CMD_NOTIFY, n, 0, msg->dev_sn, user->ds_type);
	if(pkt == NULL){
		return NULL;
	}
	
	notify = get_pkt_payload(pkt, net_notify_t);
	notify->first_report_id = ntohl((u_int32_t)msg->report_id);
	notify->report_count = 1;
	notify->report_expire = msg->expire;
	notify->reserved = 0;

	tlv = (net_notify_tlv_t*)notify->tlv_data;
	tlv->type = ntohs(msg->msg_type);
	tlv->length = ntohl(sizeof(*value) + msg->msg_len + msg->content_len);

	value = (net_notify_value_v2_t *)tlv->value;
	value->timestamp = ntohl(msg->msg_time);
	value->len = ntohl(msg->msg_len);
	value->title_len = msg->content_len;
	value->msg_fmt = msg->msg_format;
	value->pad1 = 0;
	value->pad2 = 0;
	if (msg->msg_len )
		memcpy(value->notify_msg, msg->msg, msg->msg_len);
	if (msg->content_len)
		memcpy(&value->notify_msg[msg->msg_len], msg->content, msg->content_len);
	return pkt;
}

static pkt_t *build_hello_ack(user_t *user, notify_hello_ack_t *ack)
{
	pkt_t *pkt = NULL;
	net_notify_hello_ack_t *net_ack;
	notify_hello_tlv_t *src, *dst;
	u_int16_t index, left, current;
	
	
	pkt = pkt_new_v2(CMD_NOTIFY_HELLO_ACK, sizeof(*net_ack) + ack->tlv_data_len,
					0, ack->dst_sn, user->ds_type);
	if(pkt == NULL){
		return NULL;
	}

	net_ack = get_pkt_payload(pkt, net_notify_hello_ack_t);
	net_ack->mysn = ntoh_ll(user->sn);
	net_ack->versiona = ack->versiona;
	net_ack->versionb = ack->versionb;
	net_ack->versionc = ack->versionc;
	net_ack->versiond = ack->versiond;
	net_ack->expect_report_id = ntohl(ack->expect_report_id);
	net_ack->hello_timer = ack->reserved[0]; /*握手时间间隔，缺省10秒*/
	if(net_ack->hello_timer < 10)
		net_ack->hello_timer = 10;
	net_ack->reserved[0] = 0;
	net_ack->reserved[1] = 0;
	net_ack->reserved[2] = 0;

	index = 0;
	left = ack->tlv_data_len;
	while(ack->tlv_count && left >= sizeof(notify_hello_tlv_t)){
		src = (notify_hello_tlv_t*)&ack->tlv_data[index];
		dst = (notify_hello_tlv_t*)&net_ack->tlv_data[index];
		dst->type = ntohs(src->type);
		dst->len = ntohs(src->len);
		current = sizeof(notify_hello_tlv_t) + src->len;
		if(left < current)
			break;
		if(src->len){
			memcpy(dst->value, src->value, src->len);			
		}
		left -= current;
		index += current;
		ack->tlv_count--;		
	}
	if (index != ack->tlv_data_len) {
		net_header_t *hdr = (net_header_t*)pkt->data;
		hdr->param_len = ntohl(sizeof(*net_ack) + index);
		pkt->total = net_header_real_size(hdr) + (sizeof(*net_ack) + index);		
	}
	return pkt;
}
#if 0
static pkt_t *build_notify(user_t *user, notify_msg_t *msg)
{
	pkt_t *pkt = NULL;
	net_notify_t *notify;   /*网络层*/
	net_notify_tlv_t *dst;         /*网络层*/
	notify_t *src;              /*应用层*/
	int param_len;
	u_int32_t left, isrc, idst, current;

	param_len = sizeof(net_notify_t) + msg->tlv_data_len;
	pkt = pkt_new_v2(CMD_NOTIFY, param_len, 0, user->sn, user->ds_type);
	if(pkt == NULL)
		return NULL;
	notify = get_pkt_payload(pkt, net_notify_t);
	notify->first_report_id = ntohl(msg->first_id);
	notify->report_count = 0;
	notify->reserved[0] = 0;
	notify->reserved[1] = 0;
	notify->reserved[2] = 0;

	/*
	目前网络层和应用层tlv一致，
	就只判断应用层tlv数据边界了
	*/
	left = msg->tlv_data_len;
	isrc = 0; idst = 0; 
	while(notify->report_count < msg->tlv_count && left >= msg->tlv_data_len){
		dst = (net_notify_tlv_t*)&notify->tlv_data[idst];
		src = (notify_t*)&msg->tlv_data[isrc];
		current = sizeof(notify_t) + src->len;
		if (left < current)
			break;
		dst->type = ntohs(src->type);
		dst->length = ntohl(src->len);
		if(src->len > 0)
			memcpy(dst->value, src->value, src->len);
		left -= current;
		isrc += current;
		idst += (sizeof(*dst) + src->len);
		notify->report_count++;		
	}

	return pkt;
}
#endif


CLIB_API RS cl_cmt_login(cl_handle_t *user_handle, const char *filename,
							cl_callback_t callback, void *callback_handle)
{
	cl_notify_pkt_t *pkt = NULL;
	cln_user_t *u;
	RS ret;
	char passwd[32];
	u_int8_t license[MAX_LICENCE];
	int len_license;
	ds_key_t *dskey = NULL;

	CL_CHECK_INIT;

	*user_handle = INVALID_HANDLE;	
	len_license = read_licence(filename, license);
	if(license == 0){
		ret = RS_INVALID_LICENSE;
		goto done;
	}
	dskey = parse_licence(license, &len_license);
	if(dskey == NULL){
		ret = RS_INVALID_LICENSE;
		goto done;
	}
	if(dskey->cert_hd.ds_type != TP_DS007){
		ret = RS_INVALID_LICENSE;
		goto done;
	}
	
	sprintf(passwd, "%06d", dskey->cert_hd.global_id);
	pkt = cl_notify_pkt_new(4096, CLNE_USER_LOGIN, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->len_name = (u_int16_t)strlen(dskey->cert_hd.sn) + 1;
	u->len_passwd = (u_int16_t)strlen(passwd) + 1;
	u->len_license = (u_int16_t)len_license;
	u->callback = callback;
	u->callback_handle = callback_handle;
	strcpy(u->data, dskey->cert_hd.sn);
	strcpy(u->data + u->len_name, passwd);
	memcpy(u->data + u->len_name + u->len_passwd, license, len_license);
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd + u->len_license;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	if (ret == RS_OK) {
		user_t *user;
		int count;
		cl_lock(&cl_priv->mutex);
		if ((user = cl_user_lookup(dskey->cert_hd.sn)) != NULL){
			*user_handle = user->handle;
			count = sizeof(user->user_cmt->service_ip)/sizeof(u_int32_t);
			get_ip_list(user->user_cmt->service_ip, &count);
			user->user_cmt->ip_count = count;
		}
		cl_unlock(&cl_priv->mutex);
	}
done:
	if(dskey)
		free_licence(dskey);
	return ret;
}
CLIB_API RS cl_cmt_logout(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cln_user_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(4096, CLNE_USER_LOGOUT, CLNPF_ACK);
	
	u = (cln_user_t *)&pkt->data[0];
	u->user_handle = user_handle;
	
	pkt->param_len = sizeof(cln_user_t) + u->len_name + u->len_passwd;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_cmt_self_info(cl_handle_t user_handle, cmt_info_t *info)
{
	user_t *user;
	RS ret = RS_OK;
	
	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		ret = RS_NOT_LOGIN;
		goto done;
	}
	info->sn = user->sn;
	info->service_port = user->sock_udp_port;
	info->ip_count = user->user_cmt->ip_count;
	memcpy(info->service_ip, user->user_cmt->service_ip, sizeof(info->service_ip));	

done:	
	cl_unlock(&cl_priv->mutex);
	return ret;
}

CLIB_API RS cl_cmt_query_device_list(cl_handle_t user_handle, u_int64_t sn)
{
	cl_notify_pkt_t *pkt;
	cln_device_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(512, CLNE_CMT_QUERY_DEVICE, CLNPF_ACK);
	
	u = (cln_device_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->sn = sn;
	u->phone[0] = 0;
	u->name[0] = 0;
	
	pkt->param_len = sizeof(cln_user_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API device_list_t *cl_cmt_get_device_list(cl_handle_t user_handle)
{
	user_cmt_event_t *evt;
	device_list_t *list = NULL;
	user_t *user;

	if (cl_priv == NULL) { 
		log_err(false, "client libary is not init now. please call cl_init first!!!!\n"); 
		return NULL; 
	}
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}
	if(stlc_list_empty(&user->user_cmt->dev_query_head))
		goto done;
	
	evt = stlc_list_first_entry(&user->user_cmt->dev_query_head, user_cmt_event_t, link);
	list = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);

	return list;
}

CLIB_API RS cl_cmt_add_device(cl_handle_t user_handle, device_info_t *device)
{
	cl_notify_pkt_t *pkt;
	cln_device_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(512, CLNE_CMT_ADD_DEVICE, CLNPF_ACK);
	
	u = (cln_device_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->sn = device->sn;
	strncpy((char*)u->phone, device->phone, sizeof(u->phone));
	strncpy((char*)u->name, device->name, sizeof(u->name));
	pkt->param_len = sizeof(cln_device_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}
CLIB_API RS cl_cmt_del_device(cl_handle_t user_handle, device_info_t *device)
{
	cl_notify_pkt_t *pkt;
	cln_device_t *u;
	RS ret;

	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(512, CLNE_CMT_DEL_DEVICE, CLNPF_ACK);
	
	u = (cln_device_t *)&pkt->data[0];
	u->user_handle = user_handle;
	u->sn = device->sn;
	u->phone[0] = 0;
	u->name[0] = 0;
	pkt->param_len = sizeof(cln_device_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API device_status_t *cl_cmt_get_add_result(cl_handle_t user_handle)
{
	device_status_t *devst = NULL;
	user_t *user;
	user_cmt_event_t *evt;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}
	
	if(stlc_list_empty(&user->user_cmt->dev_add_head))
		goto done;
	evt = stlc_list_first_entry(&user->user_cmt->dev_add_head, user_cmt_event_t, link);
	devst = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);
	return devst;
}

CLIB_API device_status_t *cl_cmt_get_del_result(cl_handle_t user_handle)
{
	device_status_t *devst = NULL;
	user_t *user;
	user_cmt_event_t *evt;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_dev_get_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}
	
	if(stlc_list_empty(&user->user_cmt->dev_del_head))
		goto done;
	evt = stlc_list_first_entry(&user->user_cmt->dev_del_head, user_cmt_event_t, link);
	devst = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);
	return devst;
}

CLIB_API RS cl_cmt_send_notify_expect(cl_handle_t user_handle, notify_expect_t* expect, int count)
{
	RS ret = RS_OK;
	user_t *user;
	net_notify_expect_t *p;
	pkt_t *pkt = NULL;
	int i;

	if(count <= 0 ||  count > 100 || expect == NULL)
		return RS_INVALID_PARAM;

	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if(user->status != CS_ESTABLISH){
		ret = RS_OFFLINE;
		goto done;
	}
	
	i = sizeof(*p) * count;
	pkt = pkt_new_v2(CMD_NOTIFY_EXPECT, i, 0, user->sn, user->ds_type);
	if(pkt == NULL){
		ret = RS_ERROR;
		goto done;
	}
	p = get_pkt_payload(pkt, net_notify_expect_t);
	for(i = 0; i < count; i++){
		p[i].sn = ntoh_ll(expect[i].dev_sn);
		p[i].expect_report_id = ntohl((u_int32_t)expect[i].expect_report_id);
	}
	
	user_add_pkt(user, pkt);
	pkt = NULL;

done:
	if(pkt)
		pkt_free(pkt);		
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

CLIB_API RS cl_cmt_send_notify_new(cl_handle_t user_handle, notify_msg_t *msg, int *send_count)
{
	RS ret = RS_OK;
	user_t *user;
	slave_t *slave;
	pkt_t *pkt = NULL;
	int cnt = 0;

	if(send_count)
		*send_count = 0;
	if (check_msg_valid(msg) != RS_OK)
		return RS_INVALID_PARAM;

	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = build_notify_msg(user, msg, 1);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
		
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if(slave->community==NULL)
			continue;
		if (check_send_to_slave(user, pkt, slave, msg) == RS_OK)
			cnt++;
	}
	
	/* 还要发送给服务器 */
	if(user->status == CS_ESTABLISH) {
		user_add_pkt(user, pkt);
		/*放入发送队列，发送完成再释放*/
		pkt = NULL;
		cnt++;
	}
	if(cnt == 0)
		ret = RS_OFFLINE;
	if(send_count)
		*send_count = cnt;

done:
	if(pkt)
		pkt_free(pkt);		
	cl_unlock(&cl_priv->mutex);
	log_debug("send notify id=%u to %d peers\n", msg->report_id, cnt);

	return ret;
}

CLIB_API RS cl_cmt_send_notify_old(cl_handle_t user_handle, notify_msg_t *msg, int *send_count, u_int64_t peer_sn)
{
	RS ret = RS_OK;
	user_t *user;
	slave_t *slave;
	pkt_t *pkt = NULL;
	int cnt = 0;

	if(send_count)
		*send_count = 0;
	if (check_msg_valid(msg) != RS_OK)
		return RS_INVALID_PARAM;

	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = build_notify_msg(user, msg, 0);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}

	if (peer_sn == 0 || peer_sn == user->sn) {
		if(user->status == CS_ESTABLISH) {
			user_add_pkt(user, pkt);
			/*放入发送队列，发送完成再释放*/
			pkt = NULL; 
			cnt++;		
		}
	}else{
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->sn != peer_sn)
				continue;
			if(slave->community==NULL)
				break;
			if (check_send_to_slave(user, pkt, slave, msg) == RS_OK)
				cnt++;
			break;
		}
	
	}
		
	if(cnt == 0)
		ret = RS_OFFLINE;
	if(send_count)
		*send_count = cnt;

done:
	if(pkt)
		pkt_free(pkt);		
	cl_unlock(&cl_priv->mutex);
	log_debug("send notify id=%u to %d peers\n", msg->report_id, cnt);

	return ret;
}

CLIB_API notify_msg_result_t *cl_cmt_get_notify_result(cl_handle_t user_handle)
{	
	user_t *user;
	user_cmt_event_t *evt;
	notify_msg_result_t *nrt = NULL;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if(stlc_list_empty(&user->user_cmt->notify_ack_head))
		goto done;
	evt = stlc_list_first_entry(&user->user_cmt->notify_ack_head, user_cmt_event_t, link);	
	nrt = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);
	return nrt;
}


CLIB_API notify_msg_t *cl_cmt_get_notify(cl_handle_t user_handle)
{	
	return cl_get_notify(user_handle);
}

CLIB_API notify_hello_t *cl_cmt_get_notify_hello(cl_handle_t user_handle)
{
	notify_hello_t *hello = NULL;
	user_t *user;
	user_cmt_event_t *evt;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}
	
	if(stlc_list_empty(&user->user_cmt->hello_head))
		goto done;
	evt = stlc_list_first_entry(&user->user_cmt->hello_head, user_cmt_event_t, link);
	hello = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);
	return hello;
}

CLIB_API RS cl_cmt_send_notify_hello_ack(cl_handle_t user_handle, notify_hello_ack_t *ack)
{
	RS ret = RS_OK;
	user_t *user;
	slave_t *slave;
	pkt_t *pkt = NULL;
	int r;

	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if ((slave = slave_lookup_by_sn(user, ack->dst_sn)) == NULL) {
		log_err(false, "%s: lookup device sn=%012llu failed\n", __FUNCTION__, ack->dst_sn);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if(slave->community == NULL){
		log_err(false, "%s: device sn=%012llu is offline\n", __FUNCTION__, ack->dst_sn);
		ret = RS_OFFLINE;
		goto done;
	}
	pkt = build_hello_ack(user, ack);
	if(pkt == NULL){
		ret =  RS_ERROR;
		goto done;
	}
	r = SENDTO_SLAVE(user, slave, pkt);	
	if (r != pkt->total){
		ret = RS_ERROR;
	}
	log_debug("send %d of %d  bytes hello ack to device %012llu\n",r, pkt->total, ack->dst_sn);
	
done:
	if(pkt)
		pkt_free(pkt);		
	cl_unlock(&cl_priv->mutex);
	return ret;
}

CLIB_API void cl_cmt_free_device_list(device_list_t *ptr)
{
	if(ptr){
		if(ptr->device)
			cl_free(ptr->device);
		cl_free(ptr);		
	}
}
CLIB_API void cl_cmt_free_dev_result(device_status_t *ptr)
{
	if(ptr)
		cl_free(ptr);
}

CLIB_API void cl_cmt_free_notify_reslut(notify_msg_result_t *ptr)
{
	if(ptr)
		cl_free(ptr);
}

CLIB_API void cl_cmt_free_notify(notify_msg_t *ptr)
{
	cl_free_notify(ptr);
}

CLIB_API void cl_cmt_free_notify_hello(notify_hello_t *ptr)
{
	if(ptr)
		cl_free(ptr);
}
CLIB_API void cl_cmt_free_alarm(alarm_msg_list_t *ptr)
{
	cl_free_alarm(ptr);
}

CLIB_API cl_measure_list_t *cl_cmt_get_measure(cl_handle_t user_handle)
{	
	user_t *user;
	user_cmt_event_t *evt, *next;
	cl_measure_list_t *list = NULL;
	int cnt = 0;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	stlc_list_for_each_entry(user_cmt_event_t, evt, &user->user_cmt->mesure_head, link) {
		cnt++;
	}
	if(cnt == 0)
		goto done;
	list = cl_malloc(sizeof(*list));
	if (list == NULL)
		goto done;
	list->list = cl_calloc(sizeof(void *), cnt);
	if (list->list == NULL){
		SAFE_FREE(list);
		goto done;		
	}
	cnt = 0;
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next,  &user->user_cmt->mesure_head, link) {		
		list->list[cnt++] = evt->msg;
		stlc_list_del(&evt->link);
		cl_free(evt);		
	}
	list->count = cnt;

done:
	cl_unlock(&cl_priv->mutex);
	return list;
}

CLIB_API void cl_cmt_free_measure(cl_measure_list_t *list)
{
	int i;
	if (list) {
		if (list->list ){
			for (i = 0; i < list->count; i++) {
				SAFE_FREE(list->list[i]);
			}
			cl_free(list->list);
		}
		cl_free(list);
	}		
}