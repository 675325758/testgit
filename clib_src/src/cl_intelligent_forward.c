#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
#include "cl_intelligent_forward.h"
#include "intelligent_forward_priv.h"


CLIB_API RS cl_intelligent_forward_query(cl_handle_t dev_handle)
{
	cl_notify_pkt_t *pkt;
	cln_if_query_t *q;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(128, CLNE_IF_QUERY, CLNPF_ACK);
	
	q = (cln_if_query_t *)&pkt->data[0];
	q->dev_handle = dev_handle;
	
	pkt->param_len = sizeof(*q);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;


}
