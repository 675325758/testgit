#include "client_lib.h"
#include "cl_priv.h"
#include "cl_plug.h"
#include "video_record_timer.h"
#include "wait_server.h"


void vrt_do_timer_a(video_t *video, pkt_t *pkt)
{
	net_header_t *hdr;
	net_record_timer_config_t *nrtc;
	slave_t *slave = video->slave;
	user_t *user = video->slave->user;

	hdr = (net_header_t *)pkt->data;
	nrtc = get_pkt_payload(pkt, net_record_timer_config_t);

	log_debug("vrt_do_timer_a, user name=%s, action=%u\n", video->slave->name, nrtc->action);

	if (nrtc->action == e_REC_ADD || nrtc->action == e_REC_MOD 
		|| nrtc->action == e_REC_DEL || nrtc->action == e_REC_SWITCH)
	{
		if (nrtc->err_code == ERR_NONE) {
			event_push(user->callback, VE_REC_TIMER_SET_OK, slave->handle, video->callback_handle);
		} else {
			log_err(false, "%s vrt_do_timer_a action=%d, err=%u\n", video->slave->str_sn, nrtc->action, nrtc->err_code);
			event_push_err(user->callback, VE_REC_TIMER_SET_FAILED, slave->handle, video->callback_handle, nrtc->err_code);
		}
		return;
	}
	
	if (nrtc->action != e_REC_QUERY)
		return;
	if (video->timer != NULL) {
		log_debug("OLD: action=%u, itme_num=%u, switch=%u, last_id=%u, err=%u\n", 
			video->timer->action, video->timer->item_num,
			video->timer->switch_val, video->timer->last_id,
			video->timer->err_code);
	}
	log_debug("NEW: action=%u, itme_num=%u, switch=0x%02x, last_id=%u, err=%u, user->callback=%p\n", 
		nrtc->action, nrtc->item_num,
		nrtc->switch_val, nrtc->last_id,
		nrtc->err_code, user->callback);

	// 判断是否改变
	if (video->timer != NULL && memcmp(video->timer, nrtc, hdr->param_len) == 0)
		return;
	
	SAFE_FREE(video->timer);
	video->timer = cl_malloc(hdr->param_len);
	memcpy(video->timer, nrtc, hdr->param_len);

	if (user->callback != NULL) {
		event_push(user->callback, VE_REC_TIMER_MODIFY, slave->handle, video->callback_handle);
	}	
}

static void callback_vrt_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	slave_t *slave;
	video_t *video;

	if ((slave = (slave_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_vrt_request, not found user type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	video = slave->video;

	switch (w->cmd) {
	case CMD_REC_TIMER_Q:
		log_err(false, "%s callback_vrt_request, CMD_REC_TIMER_Q, err=%u\n", slave->str_sn, result);
		if (w->param != (void *)e_REC_QUERY) {
			event_push_err(slave->user->callback, VE_REC_TIMER_SET_FAILED, video->handle, video->callback_handle, result);
		}
		break;

	case CMD_VIDEO_QUALITY_V2:
		log_err(false, "%s callback_vrt_request, CMD_VIDEO_QUALITY_V2, err=%u\n", slave->str_sn, result);
		break;

	default:
		log_err(false, "callback_vrt_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}

static int timer_vrt_query(cl_thread_t *t)
{
	video_t *video = (video_t *)CL_THREAD_ARG(t);
	user_t *user = video->slave->user;
	pkt_t *pkt;
	net_record_timer_config_t *nrtc;
	video_quality_v2_t *vq;

	video->t_query = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_query, timer_vrt_query, (void *)video, TIME_QUERY_VIDEO_TIMER);
    
    USER_BACKGROUND_RETURN_CHECK(user);

	// query A: 定时器
	if (video->slave->has_video_record) {
		pkt = pkt_new_v2(CMD_REC_TIMER_Q, sizeof(net_record_timer_config_t), 
				NHF_TRANSPARENT|NHF_WAIT_REPLY, video->slave->sn, user->ds_type);
		nrtc = get_pkt_payload(pkt, net_record_timer_config_t);
		memset(nrtc, 0, sizeof(net_record_timer_config_t));
		nrtc->action = e_REC_QUERY;
		
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_REC_TIMER_Q, (void *)(u_int32_t)nrtc->action, callback_vrt_request);
		user_add_pkt(user, pkt);
	}

	// query B: 视频质量
	pkt = pkt_new_v2(CMD_VIDEO_QUALITY_V2, sizeof(video_quality_v2_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, video->slave->sn, user->ds_type);
	vq = get_pkt_payload(pkt, video_quality_v2_t);
	vq->action = VIDEO_CUSTOM_READ;
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VIDEO_QUALITY_V2, (void *)(u_int32_t)vq->action, callback_vrt_request);
	user_add_pkt(user, pkt);
	

	video->quick_query = false;

	return 0;
}

void vrt_quick_query(video_t *video)
{
	video->quick_query = true;
	
	CL_THREAD_OFF(video->t_query);
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_query, timer_vrt_query, (void *)video, 0);
}

///////////////////////////////////////////////////////


static void vrt_set_on(video_t *video, int on)
{
	pkt_t *pkt;
	net_record_timer_config_t *nrtc;
	user_t *user = video->slave->user;

	log_info("vrt_set_on video=%s, on=%d\n", video->slave->name, on);
	
	pkt = pkt_new_v2(CMD_REC_TIMER_Q, sizeof(net_record_timer_config_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, video->slave->sn, user->ds_type);
	nrtc = get_pkt_payload(pkt, net_record_timer_config_t);

	nrtc->action = e_REC_SWITCH;
	nrtc->switch_val = on;
	
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_REC_TIMER_Q, (void *)(u_int32_t)nrtc->action, callback_vrt_request);
	user_add_pkt(user, pkt);

	vrt_quick_query(video);
}

static void vrt_set_timer(video_t *video, cln_vrt_t *vrt, bool add)
{
	pkt_t *pkt;
	net_record_timer_config_t *nrtc;
	record_timer_item_t *item;
	user_t *user = video->slave->user;
	cl_vrt_item_t *vrti = (cl_vrt_item_t *)vrt->timer;

	pkt = pkt_new_v2(CMD_REC_TIMER_Q, sizeof(net_record_timer_config_t) + sizeof(record_timer_item_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, video->slave->sn, user->ds_type);
	nrtc = get_pkt_payload(pkt, net_record_timer_config_t);

	nrtc->action = (add ? e_REC_ADD : e_REC_MOD);
	nrtc->item_num = 1;
	item = &nrtc->item[0];
	item->id = vrti->id;
	item->wday = vrti->wday;
	item->hours = vrti->hour;
	item->minute = vrti->minute;
	item->stat = (vrti->enable ? eITEM_ENABLE : eITEM_DISABLE);
	item->item_type = (vrti->is_once ? ePOLICY_ONCE : ePOLICY_WDAY);
	item->duration = htons(vrti->duration);
	item->location_time = htonl(vrti->location_time);
	strncpy((char*)item->name, vrti->name, sizeof(item->name));
	item->name[sizeof(item->name) - 1] = '\0';
	
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_REC_TIMER_Q, (void *)(u_int32_t)nrtc->action, callback_vrt_request);
	user_add_pkt(user, pkt);

	vrt_quick_query(video);
}

static void vrt_del_timer(video_t *video, cln_vrt_t *vrt)
{
	pkt_t *pkt;
	net_record_timer_config_t *nrtc;
	record_timer_item_t *item;
	user_t *user = video->slave->user;

	pkt = pkt_new_v2(CMD_REC_TIMER_Q, sizeof(net_record_timer_config_t) + sizeof(record_timer_item_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY, video->slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_REC_TIMER_Q, NULL, callback_vrt_request);
	nrtc = get_pkt_payload(pkt, net_record_timer_config_t);

	nrtc->action = e_REC_DEL;
	nrtc->item_num = 1;
	item = &nrtc->item[0];
	item->id = vrt->id;
	
	user_add_pkt(user, pkt);

	vrt_quick_query(video);
}

bool vrt_proc_notify(video_t *video, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_vrt_t *vrt;

	vrt = (cln_vrt_t *)&pkt->data[0];
	
	switch (pkt->type) {
	case CLNE_VRT_SET_ON:
		vrt_set_on(video, vrt->on);
		*ret = RS_OK;
		break;

	case CLNE_VRT_ADD:
		vrt_set_timer(video, vrt, true);
		*ret = RS_OK;
		break;
		
	case CLNE_VRT_MODIFY:
		vrt_set_timer(video, vrt, false);
		*ret = RS_OK;
		break;
		
	case CLNE_VRT_DEL:
		vrt_del_timer(video, vrt);
		*ret = RS_OK;
		break;
		
	default:
		return false;
	}

	return true;
}

