#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
#include "health_priv.h"

CLIB_API RS cl_family_list_query(cl_handle_t handle)
{
	cl_notify_pkt_t *pkt;
	cln_family_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_FAMILY_LIST_Q, CLNPF_ACK);
	
	v = (cln_family_t *)&pkt->data[0];
	v->handle = handle;
	
	pkt->param_len = sizeof(*v);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_family_config(cl_handle_t handle, family_t *fm)
{
	cl_notify_pkt_t *pkt;
	cln_family_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_FAMILY_CONFIG, CLNPF_ACK);
	
	v = (cln_family_t *)&pkt->data[0];
	v->handle = handle;
	memcpy(&v->fm, fm, sizeof(*fm));
	
	pkt->param_len = sizeof(*v);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_measure_query(cl_handle_t handle, measure_query_t *query)
{
	cl_notify_pkt_t *pkt;
	cln_measure_query_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_MESSURE_Q, CLNPF_ACK);
	
	v = (cln_measure_query_t *)&pkt->data[0];
	v->handle = handle;
	memcpy(&v->q, query, sizeof(*query));
	
	pkt->param_len = sizeof(*v);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_measure_delete(cl_handle_t handle, measure_del_t *del)
{
	cl_notify_pkt_t *pkt;
	cln_measure_del_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_MESSURE_DEL, CLNPF_ACK);
	
	v = (cln_measure_del_t *)&pkt->data[0];
	v->handle = handle;
	if (del) {
		if (del->mtime == 0)
			return RS_INVALID_PARAM;
		memcpy(&v->del, del, sizeof(*del));
		v->del_all = 0;
	} else {
		v->del_all = 1;
	}	
	pkt->param_len = sizeof(*v);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API family_list_t *cl_family_list_get(cl_handle_t handle)
{
	user_t *user;
	family_list_t *fl = NULL;
	user_health_t *uh;
	
	CL_CHECK_INIT_RP;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, handle);
		goto done;
	}
	
	uh = (user_health_t*)user->health;
	if (uh && uh->fl) {
		fl = uh->fl;
		uh->fl = NULL;
	}

done:
	cl_unlock(&cl_priv->mutex);
	return fl;
}

CLIB_API void cl_family_list_free(family_list_t *fl)
{
	if (fl) {
		SAFE_FREE(fl->list);
		cl_free(fl);
	}
}

CLIB_API measure_list_t *cl_measure_list_get(cl_handle_t handle)
{
	user_t *user;
	measure_list_t *ml = NULL;
	user_health_t *uh;
	list_link_t *lk;
	
	CL_CHECK_INIT_RP;
	
	cl_lock(&cl_priv->mutex);
	if ((user = lookup_by_handle(HDLT_USER, handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, handle);
		goto done;
	}
    	
	uh = (user_health_t*)user->health;
	if (!uh) 
		goto done;
	if(stlc_list_empty(&uh->measure_hd))
		goto done;
	lk = stlc_list_first_entry(&uh->measure_hd, list_link_t, node);
	stlc_list_del(&lk->node);
	ml = lk->p;
	cl_free(lk);

done:
	cl_unlock(&cl_priv->mutex);
	return ml;
}
CLIB_API void cl_measure_list_free(measure_list_t *ml)
{
	if (ml) {
		SAFE_FREE(ml->list);
		cl_free(ml);
	}
}


