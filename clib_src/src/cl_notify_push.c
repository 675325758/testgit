#include "client_lib.h"
#include "cl_priv.h"
#include "cl_notify_push.h"
#include "notify_push_priv.h"

CLIB_API RS cl_set_notify_expect(cl_handle_t user_handle, notify_expect_t* expect,cl_callback_t callback, void *handle)
{
	cl_notify_pkt_t *pkt;
	cln_notify_push_t *n;
	RS ret;
    user_t *user;
    
    CL_CHECK_INIT;
    
    
    if (!expect) {
        return RS_INVALID_PARAM;
    }
    
    cl_lock(&cl_priv->mutex);
    if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
        ret = RS_INVALID_PARAM;
		goto done;
	}
    
    if(user->status != CS_ESTABLISH){
        ret = RS_NOT_LOGIN;
		goto done;
	}
	
    if(expect->dev_sn == 0)
		expect->dev_sn = user->sn;
#if 0
    if (user->sn!=expect->dev_sn) {
        ret = RS_INVALID_PARAM;
        goto done;
    }
#endif
    
    cl_unlock(&cl_priv->mutex);
	
    
	pkt = cl_notify_pkt_new(1024, CLNE_NOTIFY_PUSH_SET_EXPECT_ID, 0);
	
	n = (cln_notify_push_t *)&pkt->data[0];
	n->user_haldle = user_handle;
    n->callback = callback;
    n->callback_handle = handle;
    n->expect_report_id = expect->expect_report_id;
    n->dev_sn = expect->dev_sn;
	
	pkt->param_len = sizeof(cln_notify_push_t);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
    return ret;
  
done:
	cl_unlock(&cl_priv->mutex);
	return ret;
	
}

CLIB_API alarm_msg_list_t *cl_cmt_get_alarm(cl_handle_t user_handle)
{	
	user_t *user;
	user_cmt_event_t *evt;
	alarm_msg_list_t *alarm = NULL;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if(!user->user_cmt){
		goto done;
	}

	if(stlc_list_empty(&user->user_cmt->alarm_head))
		goto done;
	evt = stlc_list_first_entry(&user->user_cmt->alarm_head, user_cmt_event_t, link);
	alarm = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);
	return alarm;
}

CLIB_API alarm_msg_list_t *cl_get_alarm(cl_handle_t user_handle)
{	
	user_t *user;
	alarm_msg_list_t *alarm = NULL;

	alarm = cl_cmt_get_alarm(user_handle);
	if(alarm)
		return alarm;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, user_handle);
		goto done;
	}
    
	if(user->status != CS_ESTABLISH){
		goto done;
	}
	
	if (user->np && user->np->msglist) {
		alarm = user->np->msglist;
		user->np->msglist = NULL;
	}    

done:
	cl_unlock(&cl_priv->mutex);
	return alarm;
}


CLIB_API void cl_free_alarm(alarm_msg_list_t *ptr)
{
	if(ptr){
		if(ptr->msg)
			cl_free(ptr->msg);
		cl_free(ptr);
	}
}

CLIB_API notify_msg_t *cl_get_notify(cl_handle_t user_handle)
{	
	user_t *user;
	user_cmt_event_t *evt;
	notify_msg_t *notify = NULL;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if(!user->user_cmt){
		goto done;
	}

	if(stlc_list_empty(&user->user_cmt->notify_head))
		goto done;
	evt = stlc_list_first_entry(&user->user_cmt->notify_head, user_cmt_event_t, link);
	notify = evt->msg;
	stlc_list_del(&evt->link);
	cl_free(evt);

done:
	cl_unlock(&cl_priv->mutex);
	return notify;
}


 CLIB_API void cl_free_notify(notify_msg_t *ptr)
{
	if(ptr){
		SAFE_FREE(ptr->content);
		SAFE_FREE(ptr->msg);
		cl_free(ptr);
	}	
}
CLIB_API notify_msg_list_t *cl_get_notify_list(cl_handle_t user_handle)
{	
	user_t *user;
	user_cmt_event_t *evt, *next;
	notify_msg_list_t *list = NULL;
	int cnt = 0;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_user_get_user_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}

	if(!user->user_cmt){
		goto done;
	}

	stlc_list_for_each_entry(user_cmt_event_t, evt, &user->user_cmt->notify_head, link){
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
	stlc_list_for_each_entry_safe(user_cmt_event_t, evt, next, &user->user_cmt->notify_head, link) {		
		list->list[cnt++] = evt->msg;
		stlc_list_del(&evt->link);
		cl_free(evt);
	}
	list->count = cnt;

done:
	cl_unlock(&cl_priv->mutex);
	return list;
}

CLIB_API void cl_free_notify_list(notify_msg_list_t *ptr)
{
	int i;
	if(!ptr){
		return;
	}
	for (i = 0; i < ptr->count; i++){
		cl_free_notify(ptr->list[i]);
	}
	SAFE_FREE(ptr->list);
	cl_free(ptr);
}

CLIB_API RS cl_get_community(cl_handle_t dev_handle, cmt_notify_info_t cmt[MAX_CMT], int *cnt)
 {
 	user_t *user;
	RS ret = RS_ERROR;
	u_int8_t i;
	
	 CL_CHECK_INIT;
	 
	 cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "cl_get_community: lookup user handle=0x%08x failed\n", dev_handle);
		goto done;
	}

	if(!user->user_cmt){
		*cnt = 0;
		goto done;
	}

	for (i = 0; i < user->user_cmt->cmt_cnt && i < MAX_CMT; i++){
		cmt[i].cmt_sn = user->user_cmt->cmt_list[i];
		cmt[i].max_report_id = user->user_cmt->cmt_report_id[i];
	}
	*cnt = (int)i;
	ret = RS_OK;

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
 }

static void _query_cmt_notify(user_t *user, cmt_notify_query_t *query)
{
	pkt_t *pkt;
	net_notify_query_t *q;

	pkt = pkt_new_v2(CMD_NOTIFY_QUERY, sizeof(*q), 0, user->sn, user->ds_type);
	if (pkt == NULL)
		return;
	q = get_pkt_payload(pkt, net_notify_query_t);
	q->sn = ntoh_ll(query->cmt_sn);
	q->report_begin = ntoh_ll(query->report_id_begin);
	q->report_end = ntoh_ll(query->report_id_end);
	q->is_descending = query->is_descending;
	q->query_cnt = query->query_cnt;
	q->reserved[0] = 0;
	q->reserved[1] = 0;
	user_add_pkt(user, pkt);
}

 CLIB_API RS cl_query_cmt_notify(cl_handle_t dev_handle, cmt_notify_query_t *query, int cnt)
{
 	user_t *user;
	RS ret = RS_ERROR;
	int i;
	
	 CL_CHECK_INIT;
	 if ((query == NULL) || (cnt <= 0) || (cnt > MAX_CMT))
	 	return RS_INVALID_PARAM;
	 
	 cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "cl_get_community: lookup user handle=0x%08x failed\n", dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH || user->is_udp_ctrl) {
		ret = RS_NOT_LOGIN;
		goto done;
	}	

	for (i = 0; i < cnt ; i++){
		_query_cmt_notify(user, &query[i]);
	}
	
	ret = RS_OK;

done:
	cl_unlock(&cl_priv->mutex);
	return ret;
 }
