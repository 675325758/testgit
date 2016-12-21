#include "bindphone_priv.h"
#include "cl_priv.h"
#include "ds_proto.h"
#include "wait_server.h"

static void  copy_bind_info(cl_bind_phone_t *dst, net_phone_bind_t *src)
{
	memcpy(dst->phone_number, src->phone_number, sizeof(dst->phone_number));
	memcpy(dst->phone_model, src->phone_model, sizeof(dst->phone_model));
	memcpy(dst->bind_name, src->bind_name, sizeof(dst->bind_name));
	memcpy(dst->bind_uuid, src->bind_uuid, sizeof(dst->bind_uuid));
	memcpy(dst->bind_message, src->bind_message, sizeof(dst->bind_message));
	memcpy(dst->timestamp, src->timestamp, sizeof(dst->timestamp));
}

void _recv_my_bind_phone_result(user_t *user, net_phone_bind_result_t *br)
{
	user->bp->is_binding = 0;
	if(br->err_code == ERR_NONE){
		if(br->action == BIND_ACTION_ACCEPT){
			if(user->status == CS_ESTABLISH){
				/*已登录情况下提交的申请*/
				user->bp->login_type = LT_BIND;
				event_push(user->callback, UE_BIND_PHONE_OK, user->handle, user->callback_handle);
			}else{
				/*未登录情况下提交的申请*/
				user_set_status(user, CS_IDLE);
			}
		}else{
		}
	} else if(br->err_code == ERR_BIND_FULL){
		user->last_err = ULGE_FULL_BIND;
		user_set_status(user, CS_LOGIN_ERR);	
	}
}

void _recv_other_bind_phone_result(user_t *user, net_phone_bind_result_t *br)
{
	bind_phone_result_node_t *bpr;
	if (user->bp == NULL) {
		return;
	}

	bpr = cl_malloc(sizeof(*bpr));
	if(bpr == NULL)
		return;
	
	if (br->err_code == ERR_BINDLATE) {
		bpr->result.err_code = ULGE_OTHER_BIND;
	} else if (br->err_code == ERR_BIND_FULL) {
		bpr->result.err_code = ULGE_FULL_BIND;
	} else {
		bpr->result.err_code = br->err_code;
	}
	bpr->result.action = br->action;
	memcpy(bpr->result.request_uuid, br->request_uuid, sizeof(bpr->result.request_uuid));
	copy_bind_info(&bpr->result.operator_info, &br->ratify_info);
	stlc_list_add_tail(&bpr->link, &user->bp->result_head);				
	event_push(user->callback, UE_BIND_PHONE_OPERATION_RESULT, user->handle, user->callback_handle);
}

/*
recv_bind_phone_result
收到绑定手机申请结果
*/
void recv_bind_phone_result(user_t *user, pkt_t *pkt)
{
	net_header_t *hdr;
	net_phone_bind_result_t *br;

	hdr = (net_header_t*)pkt->data;
	if(hdr->param_len < sizeof(*br)){
		return;
	}
	
	br = get_pkt_payload(pkt, net_phone_bind_result_t);
	br->err_code = ntohl(br->err_code);
	if (strcmp((char*)cl_priv->uuid, (char*)br->request_uuid) == 0) {
		/*my bind result*/
		_recv_my_bind_phone_result(user, br);
	} else {
		/*other's bind result*/
		_recv_other_bind_phone_result(user, br);		
	}	
}

void save_bind_phone_request(user_t *user, net_phone_bind_t *q)
{
	bind_phone_node_t *b, *b_next;

	stlc_list_for_each_entry_safe(bind_phone_node_t, b, b_next, &user->bp->request_head, link) {
		if(strcmp(q->bind_uuid, b->bind.bind_uuid) == 0){
			/*绑定申请没有被及时处理情况下
			经常会反复申请
			*/
			copy_bind_info(&b->bind, q);
			return;
		}		
	}
	b = cl_malloc(sizeof(*b));
	if(b == NULL)
		return;
	copy_bind_info(&b->bind, q);
	user->bp->request_count++;
	stlc_list_add(&b->link, &user->bp->request_head);
}
/*
recv_bind_phone_request_list
收到绑定手机申请列表
*/
void recv_bind_phone_request_list(user_t *user, pkt_t *pkt)
{
	net_header_t *hdr;
	net_phone_bind_q_list_t *list;
	u_int8_t i;

	hdr = (net_header_t*)pkt->data;
	if(hdr->param_len < sizeof(*list)){
		log_err(false, "invalid param len %d < %d\n",
			hdr->param_len , sizeof(*list));
		return;
	}
	list = get_pkt_payload(pkt, net_phone_bind_q_list_t);
	if(list->count == 0)
		return;
	if(hdr->param_len < (sizeof(*list) + list->count * sizeof(net_phone_bind_t))){
		log_err(false, "invalid param len %d < %d\n", hdr->param_len ,
			(sizeof(*list) + list->count * sizeof(net_phone_bind_t)));
		return;
	}

	for(i = 0; i < list->count; i++){
		save_bind_phone_request(user, &list->binds[i]);
	}
	event_push(user->callback, UE_BIND_PHONE_REQUEST_LIST, user->handle, user->callback_handle);	
}

/*
recv_bind_phone_list
收到已绑定手机列表
*/
void recv_bind_phone_list(user_t *user, pkt_t *pkt)
{
	net_header_t *hdr;
	net_phone_bind_list_t *list;
	cl_bind_phone_list_t *bl;
	u_int8_t i;
	
	hdr = (net_header_t*)pkt->data;
	if(hdr->param_len < sizeof(*list)){
		log_err(false, "invalid param len %d < %d\n",
			hdr->param_len , sizeof(*list));
		return;
	}
	list = get_pkt_payload(pkt, net_phone_bind_list_t);
	if(hdr->param_len < (sizeof(*list) + list->count * sizeof(net_phone_bind_t))){
		log_err(false, "invalid param len %d < %d\n", hdr->param_len ,
			(sizeof(*list) + list->count * sizeof(net_phone_bind_t)));
		return;
	}

	SAFE_FREE(user->bp->bpl.bind_array);
	bl = &user->bp->bpl;
	bl->count = list->count;
	bl->allow_normal = list->unbind_login;
	
	if(list->count){
		bl->bind_array = cl_malloc(list->count * sizeof(cl_bind_phone_t));
		if(bl->bind_array){
			for(i = 0; i < list->count; i++){
				copy_bind_info(&bl->bind_array[i], &list->binds[i]);
			}
		}
	}
	event_push(user->callback, UE_BIND_PHONE_LIST, user->handle, user->callback_handle);
}

void send_bind_phone_q(user_t *user)
{
	pkt_t *pkt;
	net_phone_bind_request_t *netq;
	cl_bind_phone_t *src;
	
	if(user->bp == NULL)
		return;
	src = &user->bp->my_bind;
	
	pkt = pkt_new(CMD_PHONE_BIND_Q, sizeof(*netq), user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, user->handle, CMD_PHONE_BIND_Q, NULL, callback_user_requst);
	netq = get_pkt_payload(pkt, net_phone_bind_request_t);
	netq->sn = ntoh_ll(user->sn);
	strcpy(netq->phone.phone_number, src->phone_number);
	strcpy(netq->phone.phone_model, src->phone_model);
	strcpy(netq->phone.bind_name, src->bind_name);
	strcpy(netq->phone.bind_uuid, src->bind_uuid);
	strcpy(netq->phone.bind_message, src->bind_message);
	netq->phone.timestamp[0] = 0;
	netq->reserved[0] = 0;
	netq->reserved[1] = 0;
	netq->reserved[2] = 0;
	netq->reserved[3] = 0;	
	user_add_pkt(user, pkt);
	log_info("xxx send bind request for %012"PRIu64", phone_number=%s, phone_model=%s, bind_name=%s, bind_uuid=%02x%02x\n", 
		user->sn, src->phone_number, src->phone_model, src->bind_name, src->bind_uuid[0], src->bind_uuid[1]);
}

RS user_bind_phone_q(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	
	cln_bind_phone_t *clnq;
	cl_bind_phone_t *dst;
	
	
	clnq = (cln_bind_phone_t *)&cln_pkt->data[0];
		
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, clnq->user_handle)) == NULL) {
		log_err(false, "user bind phone request 0x%08x failed: not found\n", clnq->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if(user->bp == NULL || user->bp->login_type == LT_BIND){
		ret = RS_ERROR;
		goto done;
	}
	dst = &user->bp->my_bind;	
	if ((user->status == CS_ESTABLISH) || (user->status == CS_LOGIN_ERR)) {
		strcpy(dst->phone_number, clnq->phone_number);
		strcpy(dst->phone_model, clnq->phone_model);
		strcpy(dst->bind_name, clnq->bind_name);
		strcpy(dst->bind_uuid, clnq->bind_uuid);
		strcpy(dst->bind_message, clnq->bind_message);
		user->bp->is_binding = 1;
		if(user->status == CS_ESTABLISH)
			send_bind_phone_q(user);
		else
			user_set_status(user, CS_DISP);
	}else{
		ret = RS_ERROR;
	}

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

RS user_bind_phone_request_list_query(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	pkt_t *pkt = NULL;
	
	cln_bind_phone_query_t *clnq;
	
	
	clnq = (cln_bind_phone_query_t *)&cln_pkt->data[0];
		
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, clnq->user_handle)) == NULL) {
		log_err(false, "user bind phone request 0x%08x failed: not found\n", clnq->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if(user->bp->login_type != LT_BIND){
		ret = RS_ERROR;
		goto done;
	}
	if ((user->status != CS_ESTABLISH)){
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new(CMD_PHONE_REQUESTLIST_Q, 0, user->ds_type);
	
	if(pkt){
		user_add_pkt(user, pkt);
		log_info("查询绑定手机申请列表012"PRIu64"\n", user->sn);
	}
	else
		ret = RS_MEMORY_MALLOC_FAIL;

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}
RS user_bind_phone_list_query(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	pkt_t *pkt = NULL;
	
	cln_bind_phone_query_t *clnq;
	
	
	clnq = (cln_bind_phone_query_t *)&cln_pkt->data[0];
		
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, clnq->user_handle)) == NULL) {
		log_err(false, "user bind phone request 0x%08x failed: not found\n", clnq->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if(user->bp->login_type != LT_BIND){
		ret = RS_ERROR;
		goto done;
	}
	if ((user->status != CS_ESTABLISH)){
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new(CMD_PHONE_BINDLIST_Q, 0, user->ds_type);
	
	if(pkt){
		user_add_pkt(user, pkt);
		log_info("查询已绑定手机列表012"PRIu64"\n", user->sn);
	}
	else
		ret = RS_MEMORY_MALLOC_FAIL;

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}
RS user_bind_phone_normal_allow(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	pkt_t *pkt = NULL;
	
	cln_bind_phone_normal_allow_t *clnq;
	
	
	clnq = (cln_bind_phone_normal_allow_t *)&cln_pkt->data[0];
		
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, clnq->user_handle)) == NULL) {
		log_err(false, "user bind phone request 0x%08x failed: not found\n", clnq->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if(user->bp->login_type != LT_BIND){
		ret = RS_ERROR;
		goto done;
	}
	if ((user->status != CS_ESTABLISH)){
		ret = RS_NOT_LOGIN;
		goto done;
	}
	if(clnq->allow){
		pkt = pkt_new(CMD_PHONE_UNBINDLOGIN_ALLOW, 0, user->ds_type);

	}else{
		pkt = pkt_new(CMD_PHONE_UNBINDLOGIN_DENY, 0, user->ds_type);
	}
	if(pkt)
		user_add_pkt(user, pkt);
	else
		ret = RS_MEMORY_MALLOC_FAIL;

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}

RS user_bind_phone_operate(cl_notify_pkt_t *cln_pkt)
{
	cln_bind_phone_operation_t *clnq;
	net_phone_bind_operation_t *netq;
	RS ret = RS_OK;
	user_t *user;
	pkt_t *pkt = NULL;
	
	
	clnq = (cln_bind_phone_operation_t *)&cln_pkt->data[0];
		
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, clnq->user_handle)) == NULL) {
		log_err(false, "user bind phone request 0x%08x failed: not found\n", clnq->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if (user->bp->login_type != LT_BIND) {
		ret = RS_ERROR;
		goto done;
	}
	if ((user->status != CS_ESTABLISH)) {
		ret = RS_NOT_LOGIN;
		goto done;
	}
	
	
	pkt = pkt_new(CMD_PHONE_BIND_OPERATION, sizeof(*netq), user->ds_type);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	netq = get_pkt_payload(pkt, net_phone_bind_operation_t);
	netq->action = clnq->action;
	netq->reserved[0] = 0;
	netq->reserved[1] = 0;
	netq->reserved[2] = 0;
	strcpy((char*)netq->request_uuid , clnq->request_uuid);
	strcpy((char*)netq->operation_uuid, (char*)cl_priv->uuid);
//	
//	log_info("操作绑定手机申请%012"PRIu64", action=%d, uuid=%s\n",
//		user->sn, netq->action, netq->request_uuid);
	user_add_pkt(user, pkt);

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
	
}


/***************user level bind phone routine bellow*****************/
CLIB_API RS cl_user_bind_phone(cl_handle_t user_handle, cl_bind_phone_t *request)
{
	cl_notify_pkt_t *pkt;
	cln_bind_phone_t *q;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_BIND_PHONE_Q, CLNPF_ACK);
	
	q = (cln_bind_phone_t *)&pkt->data[0];
	q->user_handle = user_handle;
	request->phone_number[sizeof(request->phone_number)-1] = 0;
	request->phone_model[sizeof(request->phone_model)-1] = 0;
	request->bind_name[sizeof(request->bind_name)-1] = 0;
	request->bind_uuid[sizeof(request->bind_uuid)-1] = 0;
	request->bind_message[sizeof(request->bind_message)-1] = 0;
	strcpy(q->phone_number, request->phone_number);
	strcpy(q->phone_model, request->phone_model);
	strcpy(q->bind_name, request->bind_name);
	strcpy(q->bind_uuid, request->bind_uuid);
	strcpy(q->bind_message, request->bind_message);

	pkt->param_len = sizeof(*q);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_bind_phone_allow_normal(cl_handle_t user_handle, int allow)
{
	cl_notify_pkt_t *pkt;
	cln_bind_phone_normal_allow_t *q;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_BIND_PHONE_NORMAL, CLNPF_ACK);
	
	q = (cln_bind_phone_normal_allow_t *)&pkt->data[0];
	q->user_handle = user_handle;
	q->allow = allow;

	pkt->param_len = sizeof(*q);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_bind_phone_request_query(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cln_bind_phone_query_t *q;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_BIND_PHONE_REQUEST_LIST, CLNPF_ACK);
	
	q = (cln_bind_phone_query_t *)&pkt->data[0];
	q->user_handle = user_handle;
	
	pkt->param_len = sizeof(*q);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_bind_phone_query(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cln_bind_phone_query_t *q;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_BIND_PHONE_LIST, CLNPF_ACK);
	
	q = (cln_bind_phone_query_t *)&pkt->data[0];
	q->user_handle = user_handle;
	
	pkt->param_len = sizeof(*q);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_user_bind_phone_operation(cl_handle_t user_handle, char action, const char *uuid)
{
	//net_phone_bind_operation_t *op;
	cl_notify_pkt_t *pkt;
	cln_bind_phone_operation_t *op;
	RS ret;

	if((action != 	BIND_PHONE_ACCEPT) && (action != BIND_PHONE_DENY))
		return RS_INVALID_PARAM;
	if((uuid ==NULL) || (uuid[0] == 0))
		return RS_INVALID_PARAM;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(4096, CLNE_USER_BIND_PHONE_OPERATION, CLNPF_ACK);
	
	op = (cln_bind_phone_operation_t *)&pkt->data[0];
	op->user_handle = user_handle;
	op->action = action;
	op->reserved[0] = 0;
	op->reserved[1] = 0;
	op->reserved[2] = 0;
	strcpy(op->request_uuid , uuid);
	
	pkt->param_len = sizeof(*op);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;

}

CLIB_API cl_bind_phone_list_t *cl_user_get_bind_phone_list(cl_handle_t user_handle)
{	
	user_t *user;
	cl_bind_phone_list_t *list = NULL;
	cl_bind_phone_list_t *src;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if (user->bp == NULL || user->bp->bpl.bind_array == NULL) {
		goto done;
	}
	src = &user->bp->bpl;
	
	list = cl_malloc(sizeof(*list));
	if (list == NULL) {
		goto done;
	}
	list->bind_array = cl_malloc(sizeof(cl_bind_phone_t) * src->count);
	if(list->bind_array == NULL){
		cl_free(list);
		list = NULL;
		goto done;
	}
		
	list->count = src->count;
	list->allow_normal = src->allow_normal;
	list->reserved[0] = 0;
	list->reserved[1] = 0;
	memcpy(list->bind_array, src->bind_array, sizeof(cl_bind_phone_t) * src->count);	

done:
	cl_unlock(&cl_priv->mutex);
	return list;
}

CLIB_API void cl_user_free_bind_phone_list(cl_bind_phone_list_t *list)
{
	if(list){
		if(list->bind_array)
			cl_free(list->bind_array);
		cl_free(list);		
	}
}

CLIB_API cl_bind_phone_request_list_t *cl_user_get_bind_phone_request_list(cl_handle_t user_handle)
{	
	user_t *user;
	cl_bind_phone_request_list_t *list = NULL;
	bind_phone_node_t *b, *b_next;
	u_int8_t i;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if (user->bp == NULL || user->bp->request_count == 0) {
		goto done;
	}
	

	list = cl_malloc(sizeof(*list));
	if(list == NULL)
		goto done;
	list->request_list = cl_malloc(sizeof(cl_bind_phone_t) * user->bp->request_count);
	if(list->request_list == NULL){
		cl_free(list);
		list = NULL;
		goto done;
	}
		
	list->count = 0;
	list->reserved[0] = 0;
	list->reserved[1] = 0;
	list->reserved[2] = 0;
	i = 0;
	stlc_list_for_each_entry_safe(bind_phone_node_t, b, b_next, &user->bp->request_head, link) {
		memcpy(&list->request_list[i], &b->bind, sizeof(cl_bind_phone_t));
		user->bp->request_count--;
		i++; list->count++;
		stlc_list_del(&b->link);
		cl_free(b);
		/*上层未处理申请的话，会累积在sdk
		不要超过list->count表示范围
		*/
		if(i >= 254)
			break;
	}

done:
	cl_unlock(&cl_priv->mutex);
	return list;
}

CLIB_API void cl_user_free_bind_phone_request_list(cl_bind_phone_request_list_t *list)
{
	if(list){
		if(list->request_list)
			cl_free(list->request_list);
		cl_free(list);
	}
}

CLIB_API cl_bind_phone_result_t *cl_user_get_bind_phone_operation_result(cl_handle_t user_handle)
{	
	user_t *user;
	cl_bind_phone_result_t *result = NULL;
	bind_phone_result_node_t *bpr, *bpr_next;
			
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if (user->bp == NULL) {
		goto done;
	}
	if(stlc_list_empty(&user->bp->result_head))
		goto done;

	result = cl_malloc(sizeof(*result));
	if (result == NULL) {
		goto done;
	}
	stlc_list_for_each_entry_safe(bind_phone_result_node_t, bpr, bpr_next, &user->bp->result_head, link) {
		memcpy(result, &bpr->result, sizeof(*result));
		stlc_list_del(&bpr->link);
		cl_free(bpr);
		break;
	}

done:
	cl_unlock(&cl_priv->mutex);
	return result;
}

CLIB_API void cl_user_free_bind_phone_operation_result(cl_bind_phone_result_t *result)
{
	if(result)
		cl_free(result);
}

RS user_bind_phone_alloc(user_t *user)
{
	user->bp = cl_malloc(sizeof(user_bind_phone_t));
	if(user->bp == NULL)
		return RS_MEMORY_MALLOC_FAIL;
	memset(user->bp, 0, sizeof(user_bind_phone_t));
	STLC_INIT_LIST_HEAD(&user->bp->request_head);
	STLC_INIT_LIST_HEAD(&user->bp->result_head);
	return RS_OK;	
}
void user_bind_phone_free(user_t *user)
{
	bind_phone_result_node_t *bpr, *bpr_next;
	bind_phone_node_t *b, *b_next;
	
	stlc_list_for_each_entry_safe(bind_phone_result_node_t, bpr, bpr_next, &user->bp->result_head, link) {
		stlc_list_del(&bpr->link);
		cl_free(bpr);
	}
	stlc_list_for_each_entry_safe(bind_phone_node_t, b, b_next, &user->bp->request_head, link) {
		stlc_list_del(&b->link);
		cl_free(b);
	}
	SAFE_FREE(user->bp->bpl.bind_array);
	SAFE_FREE(user->bp);		
}

