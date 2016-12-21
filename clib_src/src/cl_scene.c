/*
 本文件实现情景模式的接口
 */
#include "cl_scene.h"
#include "cl_priv.h"

CLIB_API RS cl_scene_add(cl_handle_t user_handle,cl_handle_t* s_handle,u_int8_t img_resv,const char* scene_name)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!scene_name||strlen(scene_name)<=0||!IS_SAME_HANDLE_TYPE(user_handle, HDLT_USER)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_ADD, CLNPF_ACK);
	pkt->param_len = sizeof(cln_area_t);
    s = (cln_scene_t*)pkt->data;
    s->user_hand = user_handle;
    s->req_hand = s_handle;
    s->img_id = img_resv;
    strncpy(s->name, scene_name, MAX_SCENE_NAME_LENGTH-1);
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_del(cl_handle_t s_handle)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(s_handle, HDLT_SCENE)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_DEL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = s_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_change_name(cl_handle_t s_handle,const char* name)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(s_handle, HDLT_SCENE)||!name||strlen(name)<=0) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_CHANGE_NAME, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = s_handle;
    strncpy(s->name, name, MAX_SCENE_NAME_LENGTH-1);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_change_img_resv(cl_handle_t s_handle,u_int8_t img_resv)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(s_handle, HDLT_SCENE)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_CHANGE_IMG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = s_handle;
    s->img_id = img_resv;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_modify_events(cl_handle_t s_handle,u_int16_t event_count,cl_scene_event_t** event)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(s_handle, HDLT_SCENE)||(event_count>0 && !event)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_MODIDY_EVENT, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t)+sizeof(cl_scene_event_t*)*event_count;
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = s_handle;
    if (event_count>0 && event) {
        s->item_count = (u_int8_t)event_count;
        memcpy(s->events, event, sizeof(cl_scene_event_t*)*event_count);
    }
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_add_3(cl_handle_t user_handle, cl_handle_t* s_handle,
				const char* name, u_int8_t img_resv, u_int16_t event_count,cl_scene_event_t** event)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (user_handle == 0 
		|| !IS_SAME_HANDLE_TYPE(user_handle, HDLT_USER)
		|| (event_count>0 && !event)) 
	{
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_MODIDY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t)+sizeof(cl_scene_event_t*)*event_count;
    s = (cln_scene_t*)pkt->data;
    s->user_hand = user_handle;
    s->req_hand = s_handle;

    strncpy(s->name, name, MAX_SCENE_NAME_LENGTH - 1);
    s->img_id = img_resv;
	
    if (event_count>0 && event) {
        s->item_count = (u_int8_t)event_count;
        memcpy(s->events, event, sizeof(cl_scene_event_t*)*event_count);
    }
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_modify(cl_handle_t s_handle, const char* name, u_int8_t img_resv, u_int16_t event_count,cl_scene_event_t** event)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(s_handle, HDLT_SCENE)||(event_count>0 && !event)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_MODIDY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t)+sizeof(cl_scene_event_t*)*event_count;
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = s_handle;

    strncpy(s->name, name, MAX_SCENE_NAME_LENGTH - 1);
    s->img_id = img_resv;
	
    if (event_count>0 && event) {
        s->item_count = (u_int8_t)event_count;
        memcpy(s->events, event, sizeof(cl_scene_event_t*)*event_count);
    }
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_exec(cl_handle_t s_handle)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(s_handle, HDLT_SCENE)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_EXEC, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = s_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

// 情景定时器
CLIB_API RS cl_scene_timer_add(cl_handle_t scene_handle, cl_scene_timer_t *timer)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(scene_handle, HDLT_SCENE)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_TIMER_ADD, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = scene_handle;
	s->timer = timer;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_timer_modify(cl_handle_t scene_handle, cl_scene_timer_t *timer)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(scene_handle, HDLT_SCENE)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_TIMER_MODIFY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = scene_handle;
	s->timer = timer;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_scene_timer_del(cl_handle_t scene_handle, u_int8_t tid)
{
    cl_notify_pkt_t *pkt;
	cln_scene_t *s;
	RS ret;
	cl_scene_timer_t t;
    
	CL_CHECK_INIT;
    
    if (!IS_SAME_HANDLE_TYPE(scene_handle, HDLT_SCENE)) {
        return RS_INVALID_PARAM;
    }

	memset(&t, 0, sizeof(t));
 	t.id = tid;
   
	pkt = cl_notify_pkt_new(1024, CLNE_SCENE_TIMER_DEL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_scene_t);
    s = (cln_scene_t*)pkt->data;
    s->scene_hand = scene_handle;
	s->timer = &t;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

