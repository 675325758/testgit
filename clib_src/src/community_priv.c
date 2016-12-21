#include "client_lib.h"
#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_user.h"
#include "net_detect.h"
#include "user_priv.h"
#include "cl_community.h"
#include "community_priv.h"
#include "notify_push_priv.h"
#include "ds_proto.h"

#ifdef WIN32
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#endif

extern slave_t *slave_alloc();
extern void slave_free(slave_t *slave);

void get_ip_list(u_int32_t *ip, int *count)
{
	int limit = *count;

#ifdef WIN32
	ULONG len;
	int i;	
	IP_ADAPTER_INFO info[100];
	PIP_ADAPTER_INFO pAdapter = NULL;
	IP_ADDR_STRING *pip;
	
	*count = 0;
	len = sizeof(info);
	GetAdaptersInfo(info,&len);

	i = 0;
	pAdapter = info;
	for (pAdapter = info; pAdapter != NULL; pAdapter = pAdapter->Next) {
		if(pAdapter->Type == MIB_IF_TYPE_LOOPBACK)
			continue;
		pip = &pAdapter->IpAddressList;
		for(; i < limit && pip; ){			
			ip[i] = ntohl(inet_addr(pip->IpAddress.String));	
			if(ip[i])
				i++;
			pip = pip->Next;
		}
	}
	*count = i;
#else
	*count = 0;
#endif
}

static void fill_center_info(community_center_info_t *center_info, 
		u_int64_t sn, u_int32_t ip, u_int16_t port)
{
	center_info->sn = ntoh_ll(sn);
	center_info->ip = ntohl(ip);
	center_info->port = ntohs(port);
	center_info->reserved[0] = 0;
	center_info->reserved[1] = 0;	
}

void send_notify_center_list(user_t *user)
{
	net_notify_center_list_t *center_head;
	community_center_info_t *center_info;
	int param_len ; 
	pkt_t *pkt;
	u_int32_t local_ip = user->local_ip;
	int ip_count, i;

	/* 
	  local 和global ip不同，设备不能发送到global ip，
	  所以这里只通告local ip	  
	*/
	ip_count = sizeof(user->user_cmt->service_ip)/sizeof(u_int32_t);
	get_ip_list(user->user_cmt->service_ip, &ip_count);	
	for( i = 0; i < ip_count; i++){
		if(user->user_cmt->service_ip[i] == user->local_ip){
			local_ip = 0;
			break;
		}
	}
	param_len = sizeof(net_notify_center_list_t) + sizeof(community_center_info_t) * ip_count;
	if(local_ip){
		param_len += sizeof(community_center_info_t);
	}
	pkt = pkt_new_v2(CMD_NOTIFY_CENTER_LIST, param_len, 0, user->sn, user->ds_type);
	if(pkt == NULL)
		return;
	center_head = get_pkt_payload(pkt, net_notify_center_list_t);
	center_head->count = ip_count;
	center_head->element_size = sizeof(community_center_info_t);
	center_head->reserved [0] = 0;
	center_head->reserved [1] = 0;
	
	center_info = (community_center_info_t*)center_head->element_data;
	if(local_ip){
		fill_center_info(center_info, user->sn, local_ip, user->sock_udp_port);
		center_info++;
		center_head->count++;
	}
	for(i = 0; i < ip_count; i++){
		fill_center_info(center_info, user->sn, user->user_cmt->service_ip[i], user->sock_udp_port);
		center_info++;
	}	
	user_add_pkt(user, pkt);
}

notify_hello_t *dump_notify_hello(net_notify_hello_t *hello, int len, u_int64_t dst_sn)
{
	notify_hello_t *up;
	notify_hello_tlv_t *src, *dst;
	int left, current, index;
	
	up = cl_malloc(len + sizeof(*up));
	if(up == NULL){
		return NULL;
	}
	up->dst_sn = dst_sn;
	up->src_sn = hello->mysn;
	up->versiona = hello->versiona;
	up->versionb = hello->versionb;
	up->versionc = hello->versionc;
	up->versiond = hello->versiond;
	up->expect_report_id = hello->expect_report_id;
	up->tlv_count = 0;

	left = len - sizeof(net_notify_hello_t);	
	index = 0;
	while(left >= sizeof(notify_hello_tlv_t)){
		src = (notify_hello_tlv_t*)&hello->tlv_data[index];
		dst = (notify_hello_tlv_t*)&up->tlv_data[index]; 
		dst->type = ntohs(src->type);
		dst->len = ntohs(src->len);
		current = sizeof(notify_hello_tlv_t) + dst->len;
		if(left < current){
			dst->len = 0;
			break;
		}
		if(dst->len > 0){
			memcpy(dst->value, src->value, dst->len);
		}
		up->tlv_count++;
		index += current;
		left -= current;	
	}
	
	return up;
}

static notify_msg_t *dump_notify(net_notify_t *notify, int len, u_int64_t dev_sn)
{
#if 0
	in_notify_msg_t *msg;
	net_notify_tlv_t *tlv;
	int left, current, isrc;
	list = cl_malloc(sizeof(*list));
	if(list == NULL)
		return NULL;
	msg = cl_malloc(sizeof(*msg) * notify->report_count);
	if(msg == NULL){
		cl_free(list);
		return NULL;
	}
	memset(msg, 0, sizeof(*msg) * notify->report_count);
	
	list->dev_sn = dev_sn;
	list->first_report_id = notify->first_report_id;
	list->reserved[0] = notify->reserved[0];
	list->reserved[1] = notify->reserved[1];
	list->count = 0;
	list->notify = msg;

	left = len - sizeof(net_notify_t);
<<<<<<< .mine	isrc = 0;
	while( left >= sizeof(*tlv) && list->count < notify->report_count){
		tlv = (net_notify_tlv_t*)&notify->tlv_data[isrc];
		msg[list->count].msg_type = ntohs(tlv->type);
		msg[list->count].msg_len = ntohl(tlv->length);
		current = sizeof(*tlv) + msg[list->count].msg_len;
=======	isrc = 0; idst = 0;
	while( left >= sizeof(*src) && msg->tlv_count < notify->report_count){
		src = (net_notify_tlv_t*)&notify->tlv_data[isrc];
		dst = (notify_t*)&msg->tlv_data[idst];
		dst->type = ntohs(src->type);
		dst->len = ntohl(src->length);
		current = sizeof(*src) + dst->len;
>>>>>>> .theirs		if(left < current)
			break;
		if(msg[list->count].msg_len){
			msg[list->count].msg = cl_malloc(msg[list->count].msg_len);
			if(msg[list->count].msg == NULL)
				break;
			memcpy(msg[list->count].msg, tlv->value, msg[list->count].msg_len);
		}
		isrc += current;
		left -= current;
		list->count++;
	}
	
	return list;
#endif	
}



void recv_notify_hello(user_t *user, pkt_t *pkt, struct sockaddr_in *addr)
{
	net_header_t *hdr;
	net_header_v2_t *hv2;
	net_notify_hello_t *hello;
	notify_hello_t *up_hello;
	slave_t *slave;
	user_cmt_event_t *evt;
	
	hdr = (net_header_t *)(pkt->data);
	if (hdr->ver < 2) 
		return;
	if(hdr->param_len < sizeof(*hello))
		return;
	hv2 = (net_header_v2_t *)hdr;
	hello = get_pkt_payload(pkt, net_notify_hello_t);
	hello->mysn = ntoh_ll(hello->mysn);
	hello->expect_report_id = ntohl(hello->expect_report_id);
		
	log_debug("%s, center sn = %012llu, device sn=%012llu, ip=%u.%u.%u.%u, port=%u\n", 
		__FUNCTION__, hv2->sn, hello->mysn, 
		IP_SHOW(ntohl(addr->sin_addr.s_addr)), ntohs(addr->sin_port));
	if(hv2->sn != user->sn)
		return;
	
	slave = slave_lookup_by_sn(user, hello->mysn);
	if(slave == NULL){
		/* 出于安全考虑，先从小区添加才能处理 */
		return;
	}

	update_slave_addr(slave, addr);
	
	up_hello = dump_notify_hello(hello, hdr->param_len, hv2->sn);		
	if(up_hello == NULL)
		return;
	slave->community->my_notify_id = up_hello->expect_report_id;
	evt = cl_malloc(sizeof(*evt));
	if(evt == NULL){
		cl_free(up_hello);
		return;
	}
	evt->msg = (void*)up_hello;
	evt->event = CE_NOTIFY_HELLO;
	stlc_list_add_tail(&evt->link, &user->user_cmt->hello_head);
	if(event_push(user->callback, CE_NOTIFY_HELLO, user->handle, user->callback_handle) != RS_OK){
		cl_free(up_hello);
		stlc_list_del(&evt->link);
		cl_free(evt);
	}	
}

void recv_notify_result(user_t *user, pkt_t *pkt, struct sockaddr_in *addr)
{
	net_header_t *hdr;
	net_header_v2_t *hv2;
	net_notify_result_t *result;
	notify_msg_result_t *up_result;
	slave_t *slave;
	user_cmt_event_t *evt;
	u_int32_t ip;
	u_int16_t port;

	hdr = (net_header_t *)(pkt->data);
	if (hdr->ver < 2) 
		return;
	if(hdr->param_len < sizeof(*result))
		return;
	hv2 = (net_header_v2_t *)hdr;
	result = get_pkt_payload(pkt, net_notify_result_t);
	result->first_report_id = ntohl(result->first_report_id);
	if(addr){
		ip = ntohl(addr->sin_addr.s_addr);
		port = ntohs(addr->sin_port);
	}else{
		ip = user->devserver_ip;
		port = user->devserver_port;
	}			
	log_debug("%s, peer sn=%012llu, ip=%u.%u.%u.%u, port=%u, first_report_id=%u\n", 
		__FUNCTION__, hv2->sn, 
		IP_SHOW(ip), ntohs(port), result->first_report_id);

	slave = slave_lookup_by_sn(user, hv2->sn);
	if(slave == NULL){
		if(addr){
			/*udp 发过来的*/
			/* 出于安全考虑，先从小区添加才能处理 */
			return;
		}
		/*tcp 发过来的，目前只有与服务器使用tcp连接*/
		/*服务器发过来，v2头sn填写的小区sn*/
		if(hv2->sn != user->sn){
			/*不是服务器发过来的，先丢弃*/
			return;
		}
		user->user_cmt->my_notify_id = (result->first_report_id + result->report_count);
	}else{
		if(addr){
			update_slave_addr(slave, addr);
			slave->community->my_notify_id = (result->first_report_id + result->report_count);
		}
	}
	
	up_result = cl_malloc(sizeof(*up_result));
	if(up_result == NULL)
		return;
	evt = cl_malloc(sizeof(*evt));
	if(evt == NULL){
		cl_free(up_result);
		return;
	}
	if(user->sn == hv2->sn)
		up_result->dev_sn = 0;
	else
		up_result->dev_sn = hv2->sn;
	up_result->first_report_id = result->first_report_id;
	up_result->report_count = result->report_count;
	up_result->result = result->result;
	up_result->reserved[0] = result->reserved[0];
	up_result->reserved[1] = result->reserved[1];
	
	evt->msg = (void*)up_result;
	evt->event = CE_NOTIFY_RESULT;
	stlc_list_add_tail(&evt->link, &user->user_cmt->notify_ack_head);
	if(event_push(user->callback, CE_NOTIFY_RESULT, user->handle, user->callback_handle) != RS_OK){
		cl_free(up_result);
		stlc_list_del(&evt->link);
		cl_free(evt);
	}
}

void recv_notify_expect(user_t *user, pkt_t *pkt, struct sockaddr_in *addr)
{
	/* 服务器期望收到本小区的notify，
	构造notify result通知应用层，触发应用层发送notify */
	net_header_t *hdr;
	net_header_v2_t *hv2;
	net_notify_expect_t *expect;
	notify_msg_result_t *up_result;
	user_cmt_event_t *evt;

	hdr = (net_header_t *)(pkt->data);
	if (hdr->ver < 2) 
		return;
	if(hdr->param_len < sizeof(*expect))
		return;
	hv2 = (net_header_v2_t *)hdr;
	expect = get_pkt_payload(pkt, net_notify_expect_t);
	expect->sn = ntoh_ll(expect->sn);
	expect->expect_report_id = ntohl(expect->expect_report_id);
	if(expect->sn != user->sn){
		return;
	}	
	if(expect->expect_report_id < 1)
		return;
	user->user_cmt->my_notify_id = expect->expect_report_id;
	
	
	up_result = cl_malloc(sizeof(*up_result));
	if(up_result == NULL)
		return;
	evt = cl_malloc(sizeof(*evt));
	if(evt == NULL){
		cl_free(up_result);
		return;
	}
	
	up_result->dev_sn = 0;
	up_result->first_report_id = expect->expect_report_id - 1;
	up_result->report_count = 1;
	up_result->result = RS_OK;
	up_result->reserved[0] = 0;
	up_result->reserved[1] = 0;
	
	evt->msg = (void*)up_result;
	evt->event = CE_NOTIFY_RESULT;
	stlc_list_add_tail(&evt->link, &user->user_cmt->notify_ack_head);
	if(event_push(user->callback, CE_NOTIFY_RESULT, user->handle, user->callback_handle) != RS_OK){
		cl_free(up_result);
		stlc_list_del(&evt->link);
		cl_free(evt);
	}
}

void send_op_device(user_t *user, cln_device_t *device, u_int8_t action)
{
	pkt_t *pkt;
	net_cmt_op_device_hdr_t *qh;
	net_cmt_op_device_t *op;
	int param_len;
	u_int8_t *phone, *name;

	param_len = sizeof(*qh) + sizeof(*op);
	if(action == AC_ADD || action == AC_MOD){
		param_len += (int)(strlen(device->phone) + strlen(device->name) + 2);
	}		

	pkt = pkt_new_v2(CMD_CMT_OP_DEVICE, param_len, 0, 0, user->ds_type);
	if(pkt == NULL)
		return;
	
	qh = get_pkt_payload(pkt, net_cmt_op_device_hdr_t);
	qh->total = ntohl(1);
	qh->current = ntohs(1);
	qh->action = action;
	qh->result = 0;
	op = (net_cmt_op_device_t*)qh->op_device;
	op->action = action;
	op->device_sn = ntoh_ll(device->sn);
	op->result = 0;
	if(action == AC_ADD || action == AC_MOD){
		op->len_phone = (u_int8_t)(strlen(device->phone) + 1);
		op->len_name = (u_int8_t)strlen(device->name) + 1;
		phone = op->op_data;
		name = phone + op->len_phone;
		memcpy(phone, device->phone, op->len_phone);
		memcpy(name, device->name, op->len_name);
	}else{
		op->len_phone = 0;
		op->len_name = 0;
	}
	user_add_pkt(user, pkt);	
}

static RS device_add(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_device_t *device;
	slave_t *slave;
	user_t *user;
	
	device = (cln_device_t*)pkt->data;
	user = lookup_by_handle(HDLT_USER, device->user_handle);
	if(user == NULL){
		ret = RS_NOT_LOGIN;
		goto done;
	}
		
	slave = slave_lookup_by_sn(user, device->sn);
	if(slave){
		/*本地已经存在，添加到服务器*/
		goto add_to_server;
	}
	slave = slave_alloc();
	if(slave == NULL){
		ret = RS_ERROR;
		goto done;
	}
	slave->user = user;
	slave->sn = device->sn;
	if(device->name[0])
		slave->name = cl_strdup(device->name);
	else
		slave->name = NULL;
	if(device->phone[0])
		slave->phone = cl_strdup(device->phone);
	else
		slave->phone = NULL;
	stlc_list_add_tail(&slave->link, &user->slave);
add_to_server:
	if(user->status == CS_ESTABLISH){
		send_op_device(user, device, AC_ADD);
	}

done:
	return ret;
}

static RS device_del(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_device_t *device;
	slave_t *slave;
	user_t *user;
	
	device = (cln_device_t*)pkt->data;
	user = lookup_by_handle(HDLT_USER, device->user_handle);
	if(user == NULL){
		ret = RS_ERROR;
		goto done;
	}
		
	slave = slave_lookup_by_sn(user, device->sn);
	if(slave == NULL){
		ret = RS_ERROR;
		goto done;
	}
	
	if(user->status == CS_ESTABLISH){
		send_op_device(user, device, AC_DEL);
	}
	stlc_list_del(&slave->link);
	slave_free(slave);	
	return ret;

done:
	return ret;
}

static RS device_query(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_device_t *device;
	user_t *user;
	
	device = (cln_device_t*)pkt->data;
	user = lookup_by_handle(HDLT_USER, device->user_handle);
	if(user == NULL){
		ret = RS_ERROR;
		goto done;
	}
			
	if(user->status == CS_ESTABLISH){
		send_op_device(user, device, AC_QUERY);
	}	
	return ret;

done:	
	return ret;
}

bool cmt_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{

	switch (pkt->type) {
	case CLNE_CMT_ADD_DEVICE:
		*ret = device_add(pkt);
		break;

	case CLNE_CMT_DEL_DEVICE:
		*ret = device_del(pkt);
		break;
	case CLNE_CMT_QUERY_DEVICE:
		*ret = device_query(pkt);
		break;

	default:
		return false;
	}

	return true;
}

static void update_user_slave(user_t *user, device_status_t *device)
{
	slave_t *slave;

	slave = slave_lookup_by_sn(user, device->info.sn);
	if(slave == NULL){
		slave = slave_alloc();
		if(slave == NULL)
			return;
		stlc_list_add_tail(&slave->link, &user->slave);
		slave->sn = device->info.sn;
		if(device->info.name[0])
			slave->name = cl_strdup(device->info.name);
		if(device->info.phone[0])
			slave->phone = cl_strdup(device->info.phone);
			
	}	
}

static void copy_op_result(device_status_t *device, net_cmt_op_device_t *op)
{
	u_int8_t len;
	int index = 0;
	if(op->result == ERR_NONE)
		device->result = RS_OK;
	else if(op->result == ERR_SN_INVALID)
		device->result = RS_NOT_FOUND;
	else
		device->result = RS_ERROR;
	device->info.sn =  ntoh_ll(op->device_sn);
	if(op->len_phone){
		len = op->len_phone;
		if(len >= sizeof(device->info.phone))
			len = sizeof(device->info.phone) - 1;
		memcpy(device->info.phone, op->op_data, len);
		device->info.phone[len]=0;	
		index += op->len_phone;
	}else{
		device->info.phone[0] = 0;
	}
	if(op->len_name){
		len = op->len_name;
		if(len >= sizeof(device->info.name))
			len = sizeof(device->info.name) -1;
		memcpy(device->info.name, &op->op_data[index], len);
		device->info.name[len] = 0;
	}else{
		device->info.name[0] = 0;
	}
}

device_list_t *dump_query_result(net_cmt_op_device_hdr_t *qh, u_int32_t len, user_t *user)
{
	u_int32_t left, index = 0, count = 0, current;
	net_cmt_op_device_t *op;
	device_list_t *list;
	device_status_t *device;
	
	list = cl_malloc(sizeof(*list));
	if(list == NULL)
		return NULL;
	list->total = qh->total;
	list->count = 0;
	list->device = NULL;

	/* 计算个数 */
	left = len;
	current = qh->current;
	while(left >= sizeof(*op) && count < qh->current){
		op = (net_cmt_op_device_t *)&qh->op_device[index];		
		current = sizeof(*op) + op->len_name + op->len_phone;
		if(left < current)
			break;
		count++;
		index += current;
		left -= current;
	}
	if(count == 0){
		return list;
	}
	
	list->device = cl_malloc(count * sizeof(device_status_t));
	if(list->device == NULL){
		cl_free(list);
		return NULL;
	}

	left = len;
	index = 0; count = 0;
	while(left >= sizeof(*op)){
		op = (net_cmt_op_device_t *)&qh->op_device[index];
		device = &list->device[count];
		current = sizeof(*op) + op->len_name + op->len_phone;		
		if(left < current)
			break;
		copy_op_result(device, op);
		if(device->result == RS_OK){
			update_user_slave(user, device);
		}
		count++;
		index += current;
		left -= current;
	}
	list->count = count;
	return list;	
}

RS do_cmt_op_device(user_t *user, pkt_t *pkt)
{
	net_cmt_op_device_t *op ;
	net_cmt_op_device_hdr_t *qh;
	net_header_t *hdr;
	device_status_t *device = NULL;
	device_list_t *list = NULL;
	user_cmt_event_t *evt = NULL;
	u_int16_t i;
	int remain, index, current;
	RS ret = RS_OK;

	hdr = (net_header_t *)pkt->data;
	if(hdr->param_len < sizeof(*qh))
		return RS_ERROR;
	qh = get_pkt_payload(pkt, net_cmt_op_device_hdr_t);
	qh->total = ntohl(qh->total);
	qh->current = ntohs(qh->current);
	
	if(qh->action == AC_ADD || qh->action == AC_DEL){
		index = 0;
		remain = hdr->param_len - sizeof(*qh);		
		for(i = 0; i < qh->current; i++){
			if(remain < sizeof(*op))
				break;			
			op = (net_cmt_op_device_t*)&qh->op_device[index];
			current = sizeof(*op) + op->len_phone + op->len_name;
			if(remain < current)
				break;
			device = cl_malloc(sizeof(*device));
			evt = cl_malloc(sizeof(*evt));
			if(device == NULL || evt == NULL){
				ret = RS_ERROR;
				goto done;
			}
			evt->msg = device;
			copy_op_result(device, op);
			if(op->action == AC_DEL){
				evt->event = CE_DEL_DEVICE;
				stlc_list_add_tail(&evt->link, &user->user_cmt->dev_del_head);
			}else{
				evt->event = CE_ADD_DEVICE;
				stlc_list_add_tail(&evt->link, &user->user_cmt->dev_add_head);
			}
			event_push(user->callback, evt->event, user->handle, user->callback_handle);			
		}		
		return ret;	
	}else if(qh->action == AC_QUERY){
		list = dump_query_result(qh, hdr->param_len - sizeof(*qh), user);
		if(list == NULL){
			ret = RS_ERROR;
			goto done;			
		}
		evt = cl_malloc(sizeof(*evt));
		if(evt == NULL){
			ret = RS_ERROR;
			goto done;
		}
		evt->msg = list;
		evt->event = CE_QUERY_DEVICE;
		stlc_list_add_tail(&evt->link, &user->user_cmt->dev_query_head);
		event_push(user->callback, evt->event, user->handle, user->callback_handle);
		return ret;
	}

done:
	if(device)
		cl_free(device);
	if(evt)
		cl_free(evt);
	if(list){
		if(list->device)
			cl_free(list->device);
		cl_free(list);
	}
	return ret;
	
}

void user_cmt_free(user_t *user)
{
	user_cmt_t *cmt = user->user_cmt;
	user_cmt_event_t *evt,*next;

	if(cmt == NULL){
		return;
	}
	
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->hello_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_notify_hello((notify_hello_t *)evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->alarm_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_alarm((alarm_msg_list_t*)evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->mesure_head, link) {
		stlc_list_del(&evt->link);
		SAFE_FREE(evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->notify_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_notify((notify_msg_t*)evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->notify_ack_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_notify_reslut((notify_msg_result_t*)evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->dev_add_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_dev_result((device_status_t *)evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->dev_del_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_dev_result((device_status_t *)evt->msg);
		cl_free(evt);
	}
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &cmt->dev_query_head, link) {
		stlc_list_del(&evt->link);
		cl_cmt_free_device_list((device_list_t *)evt->msg);
		cl_free(evt);
	}
	SAFE_FREE(user->user_cmt);
}















