/*
 ���ļ�ʵ�������ܵ��û�����Ľӿ�
 */
#include "cl_area.h"
#include "cl_priv.h"

CLIB_API RS cl_area_add(cl_handle_t user_handle,cl_handle_t* area_handle,u_int8_t img_resv,const char* area_name)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!area_name||strlen(area_name)<=0||!IS_SAME_HANDLE_TYPE(user_handle, HDLT_USER)) {
        return RS_INVALID_PARAM;
    }
    
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_ADD, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t);
    a = (cln_area_t*)pkt->data;
    a->user_hand = user_handle;
    a->req_hand = area_handle;
    a->img_id = img_resv;
    strncpy(a->name, area_name, MAX_AREA_NAME_LENGTH-1);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_area_del(cl_handle_t area_handle)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)) {
        return RS_INVALID_PARAM;
    }

	pkt = cl_notify_pkt_new(1024, CLNE_AREA_DEL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t);
    a = (cln_area_t*)pkt->data;
    a->area_hand = area_handle;
   
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_area_change_name(cl_handle_t area_handle,const char* name)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)||!name||strlen(name)<=0) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_CHANGE_NAME, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t);
    a = (cln_area_t*)pkt->data;
    a->area_hand = area_handle;
    strncpy(a->name, name, MAX_AREA_NAME_LENGTH-1);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_area_change_imge_resv(cl_handle_t area_handle,u_int8_t img_resv)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_CHANGE_IMG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t);
    a = (cln_area_t*)pkt->data;
    a->area_hand = area_handle;
    a->img_id = img_resv;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_area_modify_appliances(cl_handle_t area_handle,u_int8_t item_count,cl_handle_t* handles)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)||(item_count>0 && !handles)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_MODIDY_S, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t)+sizeof(cl_handle_t)*item_count;
    a = (cln_area_t*)pkt->data;
    a->area_hand = area_handle;
    a->item_count = item_count;
    
    for (item_count=0; item_count<a->item_count; item_count++) {
        a->eq_hands[item_count]=handles[item_count];
    }
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_area_add_3(cl_handle_t user_handle, cl_handle_t *area_handle, 
				const char* name, u_int8_t img_resv, u_int8_t item_count,cl_handle_t* handles)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (user_handle == 0 
		|| !IS_SAME_HANDLE_TYPE(user_handle, HDLT_USER) 
		|| (item_count > 0 && handles == NULL)) 
	{
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_MODIFY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t)+sizeof(cl_handle_t)*item_count;
    a = (cln_area_t*)pkt->data;
    a->user_hand = user_handle;
    a->req_hand = area_handle;

    strncpy(a->name, name, MAX_AREA_NAME_LENGTH - 1);
    a->img_id = img_resv;
	
    a->item_count = item_count;
    
    for (item_count=0; item_count<a->item_count; item_count++) {
        a->eq_hands[item_count]=handles[item_count];
    }
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_area_modify(cl_handle_t area_handle, const char* name, 
				u_int8_t img_resv, u_int8_t item_count,cl_handle_t* handles)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (area_handle == 0
		|| ! IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)
		|| (item_count>0 && !handles)) 
	{
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_MODIFY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t)+sizeof(cl_handle_t)*item_count;
    a = (cln_area_t*)pkt->data;
    a->area_hand = area_handle;

    strncpy(a->name, name, MAX_AREA_NAME_LENGTH - 1);
    a->img_id = img_resv;
	
    a->item_count = item_count;
    
    for (item_count=0; item_count<a->item_count; item_count++) {
        a->eq_hands[item_count]=handles[item_count];
    }
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_appliance_change_area(cl_handle_t appli_handle,cl_handle_t area_handle)
{
    cl_notify_pkt_t *pkt;
	cln_area_t *a;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_AREA_CHANGE_EQ, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t)+sizeof(cl_handle_t);
    a = (cln_area_t*)pkt->data;
    a->area_hand = area_handle;
    a->item_count = 0x1;
    a->eq_hands[0] = appli_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}
