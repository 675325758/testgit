#include "cl_priv.h"
#include "cl_video.h"
#include "cl_notify.h"
#include "audio_priv.h"


CLIB_API RS cl_video_start(cl_handle_t slave_handle, cl_callback_t callback, void *handle)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_START, CLNPF_ACK);
	
	v = (cln_video_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->callback = callback;
	v->callback_handle = handle;
	
	pkt->param_len = sizeof(cln_video_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

static RS cl_video_notify_simple(cl_handle_t slave_handle, u_int32_t clne, bool ack)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, clne, ack ? CLNPF_ACK : 0);
	
	u = (cln_video_t *)&pkt->data[0];
	u->slave_halde = slave_handle;
	
	pkt->param_len = sizeof(cln_video_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_video_stop(cl_handle_t slave_handle)
{
	CL_CHECK_INIT;

	return cl_video_notify_simple(slave_handle, CLNE_VIDEO_STOP, true);
}

CLIB_API RS cl_video_get_picture(cl_handle_t slave_handle, void **pic, u_int32_t *size)
{
	RS ret = RS_OK;
	video_t *video;
	data_buf_t *db;

	CL_CHECK_INIT;

	*pic = NULL;
	*size = 0;
	
	cl_lock(&cl_priv->mutex);
	
	video = (video_t *)lookup_by_handle(HDLT_VIDEO, slave_handle);
	if (video == NULL) {
		log_err(false, "cl_video_get_picture: lookup slave handle=0x%08x failed\n", slave_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (video->data_pic == NULL) {
		ret = RS_NOT_INIT;
		goto done;
	}
	
	db = de_get_read(video->data_pic);
    if (!db) {
        *pic = NULL;
        *size = 0;
        goto done;
    }
	*pic = (void *)&db->data[0];
	*size = db->data_size;

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

static RS __cl_video_ptz_roll(cl_handle_t slave_handle, ptz_roll_t left_right, ptz_roll_t up_down, int roll_type)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_ROLL, 0);
	
	u = (cln_video_t *)&pkt->data[0];
	u->slave_halde = slave_handle;
	u->u.roll.roll_type = roll_type;
	if (left_right == ptz_roll_left) {
		u->u.roll.left_right = 1;
	} else if (left_right == ptz_roll_right) {
		u->u.roll.left_right = -1;
	} else {
		u->u.roll.left_right = 0;
	}
	if (up_down == ptz_roll_up) {
		u->u.roll.up_down = 1;
	} else if (up_down == ptz_roll_down) {
		u->u.roll.up_down = -1;
	} else {
		u->u.roll.up_down = 0;
	}
	
	pkt->param_len = sizeof(cln_video_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}
CLIB_API RS cl_video_ptz_roll(cl_handle_t handle, ptz_roll_t left_right, ptz_roll_t up_down)
{
	return __cl_video_ptz_roll(handle, left_right, up_down, ROLL_TYPE_ONCE);
}
CLIB_API RS cl_video_ptz_roll_start(cl_handle_t handle, ptz_roll_t left_right, ptz_roll_t up_down)
{
	return __cl_video_ptz_roll(handle, left_right, up_down, ROLL_TYPE_START);
}
CLIB_API RS cl_video_ptz_roll_stop(cl_handle_t handle)
{
	return __cl_video_ptz_roll(handle, ptz_roll_stop, ptz_roll_stop, ROLL_TYPE_STOP);
}

CLIB_API RS cl_video_set_quality(cl_handle_t slave_handle, cl_video_quality_t *q)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_QULITY, 0);
	
	u = (cln_video_t *)&pkt->data[0];
	u->slave_halde = slave_handle;
	u->u.qulity = *q;
	
	pkt->param_len = sizeof(cln_video_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_video_flip(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_FLIP, 0);
	
	u = (cln_video_t *)&pkt->data[0];
	u->slave_halde = slave_handle;
	
	pkt->param_len = sizeof(cln_video_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_video_get_stat(cl_handle_t handle, cl_video_stat_t *st)
{
	RS ret = RS_OK;
	video_t *video;
	video_stat_t *vs;
	video_stat_priv_t *vsp;

	CL_CHECK_INIT;

	cl_video_notify_simple(handle, CLNE_VIDEO_CAL_STAT, true);
	vsp = (video_stat_priv_t *)&st->resv[0];

	cl_lock(&cl_priv->mutex);
	
	video = lookup_by_handle(HDLT_VIDEO, handle);
	if (video == NULL) {
		log_err(false, "cl_video_get_stat: lookup slave handle=0x%08x failed\n", handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	vs = &video->stat;
	memset(st, 0, sizeof(cl_video_stat_t));

	st->is_custom_quality = video->is_custom_quality;
	st->quality = video->pic_quality;
	
	st->width = vs->width;
	st->height = vs->height;
	st->fps = vs->sec_fps;
	st->bitrate = (u_int32_t)vs->sec_rate;
	st->client_count = vs->client_count;
    st->vs_stat.brightness_val = vs->brightness_val;
    st->vs_stat.contrast_val = vs->contrast_val;
    st->vs_stat.saturation_val = vs->saturation_val;
    st->vs_stat.gain_val = vs->gain_val;
    st->roll_speed = vs->roll_speed;
	
	vsp->is_tcp = (u_int8_t)vs->is_tcp;
	if (video->status == VS_ESTABLISH) {
		vsp->dev_ip = vs->dev_ip;
		vsp->dev_port = vs->dev_port;
	}

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

CLIB_API RS cl_audio_get_sound(cl_handle_t handle, cl_sound_data_t *sd)
{
	RS ret = RS_OK;
	video_t *video;
	data_buf_t *db;

	CL_CHECK_INIT;

	cl_lock(&cl_priv->mutex);
	
	video = lookup_by_handle(HDLT_VIDEO, handle);
	if (video == NULL) {
		log_err(false, "cl_audio_get_sound: lookup slave handle=0x%08x failed\n", handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	
	if ((db = de_get_read(video->data_sound)) == NULL) {
		ret = RS_EMPTY;
		goto done;
	}
	sd->data = (void *)&db->data[0];
	sd->len = db->data_size;
	sd->bits = video->stat.sound_bit;
	sd->channels = video->stat.sound_channel;
	sd->samples = audio_idx_to_sample(video->stat.sample);

	de_move_read(video->data_sound);

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

CLIB_API RS cl_audio_put_sound(cl_handle_t slave_handle, cl_sound_data_t *sd)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *u;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_AUDIO_PUT_SOUND, CLNPF_ACK);
	
	u = (cln_video_t *)&pkt->data[0];
	u->slave_halde = slave_handle;
	u->u.sound_data = *sd;
	
	pkt->param_len = sizeof(cln_video_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	请求发言权
*/
CLIB_API RS cl_audio_request_speek(cl_handle_t slave_handle)
{
	return cl_video_notify_simple(slave_handle, CLNE_AUDIO_REQUEST_SPEEK, true);
}

/*
	释放发言权
*/
CLIB_API RS cl_audio_release_speek(cl_handle_t slave_handle)
{
	return cl_video_notify_simple(slave_handle, CLNE_AUDIO_RELEASE_SPEEK, true);
}

////////////////////////////////////////////////////////////
// 录像相关的一些东西
////////////////////////////////////////////////////////////


static void vrt_utc_2_local(cl_vrt_item_t *timer, int zone)
{
	int hour;

	hour = timer->hour + 24 + zone;
	if (hour < 24) {
		timer->wday = timer_week_right_shift(timer->wday);
	} else if (hour >= 48) {
		timer->wday = timer_week_left_shift(timer->wday);
	}
	timer->hour = hour%24;
}

static void vrt_local_2_utc(cl_vrt_item_t *timer, int zone)
{	
	int hour;
	
	hour = timer->hour + 24 - zone;
	if (hour < 24) {
		timer->wday = timer_week_right_shift(timer->wday);
	} else if (hour >= 48) {
		timer->wday = timer_week_left_shift(timer->wday);
	}
	timer->hour = hour%24;
}

CLIB_API RS cl_video_rec_timer_turn_on(cl_handle_t slave_handle, bool on)
{
	cl_notify_pkt_t *pkt;
	cln_vrt_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_VRT_SET_ON, CLNPF_ACK);
	
	v = (cln_vrt_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->on = on;
	
	pkt->param_len = sizeof(cln_vrt_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_video_rec_timer_add(cl_handle_t slave_handle, cl_vrt_item_t *timer, int32_t tz)
{
	cl_notify_pkt_t *pkt;
	cln_vrt_t *v;
	RS ret;

	CL_CHECK_INIT;

	vrt_local_2_utc(timer, tz);

	pkt = cl_notify_pkt_new(1024, CLNE_VRT_ADD, CLNPF_ACK);
	
	v = (cln_vrt_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->timer = timer;
	
	pkt->param_len = sizeof(cln_vrt_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_video_rec_timer_modify(cl_handle_t slave_handle, cl_vrt_item_t *timer, int32_t tz)
{
	cl_notify_pkt_t *pkt;
	cln_vrt_t *v;
	RS ret;

	CL_CHECK_INIT;

	vrt_local_2_utc(timer, tz);

	pkt = cl_notify_pkt_new(1024, CLNE_VRT_MODIFY, CLNPF_ACK);
	
	v = (cln_vrt_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->timer = timer;
	
	pkt->param_len = sizeof(cln_vrt_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_video_rec_timer_del(cl_handle_t slave_handle, int id)
{
	cl_notify_pkt_t *pkt;
	cln_vrt_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_VRT_DEL, CLNPF_ACK);
	
	v = (cln_vrt_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->id = id;
	
	pkt->param_len = sizeof(cln_vrt_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API cl_video_info_t *cl_video_info_get(cl_handle_t slave_handle, int32_t tz)
{
	int i, count;
	slave_t *slave;
	video_t *video;
	cl_video_info_t *vi = NULL;
	net_record_timer_config_t *config;
	record_timer_item_t *ti;
	cl_vrt_item_t *vrti;
	
	if (cl_priv == NULL) {
		log_err(false, "client libary is not init now. please call cl_init first!!!!\n");
		return NULL; 
	}
	
	cl_lock(&cl_priv->mutex);

	video = (video_t *)lookup_by_handle(HDLT_VIDEO, slave_handle);
	if (video == NULL) {
		log_err(false, "cl_video_info_get: lookup slave handle=0x%08x failed\n", slave_handle);
		goto done;
	}
	slave = video->slave;
	
	vi = (cl_video_info_t *)cl_calloc(sizeof(cl_video_info_t), 1);

	vi->has_establish = video->is_open_success;
	vi->has_ptz = slave->has_ptz;
	vi->is_h264 = video->is_h264;
	vi->has_audio = (video->has_audio > AUDIO_NONE);
	vi->has_audio_speek = (video->has_audio == AUDIO_TWOWAY);

	if ((config = video->timer) == NULL)
		goto done;
	
	vi->record_enable = !(!(config->switch_val & 0x80));
	vi->record_status = (config->switch_val & 0x7F);
	
	vi->num_timer = config->item_num;
	vi->timer = cl_calloc(sizeof(void *), vi->num_timer);
	for (i = 0, count = 0; i < (int)vi->num_timer; i++) {
		ti = &video->timer->item[i];
		if (ti->stat == eITEM_INVALID)
			continue;

		vrti = vi->timer[count] = (cl_vrt_item_t *)cl_calloc(sizeof(cl_vrt_item_t), 1);
		count++;
		
		
		vrti->id = ti->id;
		vrti->wday = ti->wday;
		vrti->hour = ti->hours;
		vrti->minute = ti->minute;
		vrti->enable = (ti->stat == eITEM_ENABLE);
		vrti->is_once = (ti->item_type == ePOLICY_ONCE);
		vrti->duration = ntohs(ti->duration);
		vrti->location_time = ntohl(ti->location_time);
		vrti->name = cl_strdup((char*)ti->name);
		
		vrt_utc_2_local(vrti, tz);
	}

	vi->num_timer = count;
	
done:
	cl_unlock(&cl_priv->mutex);

	return vi;
}

CLIB_API void cl_video_info_free(cl_video_info_t *info)
{
	int i;
	
	if (info == NULL)
		return;
	
	for (i = 0; i < (int)info->num_timer; i++) {
		SAFE_FREE(info->timer[i]->name);
		SAFE_FREE(info->timer[i]);
	}
	
	SAFE_FREE(info->timer);
	cl_free(info);
}

CLIB_API RS cl_video_set_saturation(cl_handle_t slave_handle,cl_video_saturation_t* vs)
{
    cl_notify_pkt_t *pkt;
	cln_video_t* v;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!vs) {
        return ERR_PARAM_INVALID;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_SET_V4L2, CLNPF_ACK);
	
	v = (cln_video_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
    memcpy(&v->u.vs, vs, sizeof(cl_video_saturation_t));
	
	pkt->param_len = sizeof(cln_video_t);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_video_set_roll_speed(cl_handle_t slave_handle,u_int8_t speed)
{
    cl_notify_pkt_t *pkt;
	cln_video_t* v;
	RS ret;
    
	CL_CHECK_INIT;
   
    
	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_SET_ROLL_SPEED, CLNPF_ACK);
	
	v = (cln_video_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
    v->u.roll_speed = speed;
	
	pkt->param_len = sizeof(cln_video_t);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_query_vtap_list(cl_handle_t slave_handle,u_int32_t begin_time)
{
    cl_notify_pkt_t *pkt;
	cln_video_t* v;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!begin_time) {
        begin_time = (u_int32_t)time(NULL);
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_VTAP_QUERY_LIST, CLNPF_ACK);
	
	v = (cln_video_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
    v->u.query.begin_time = begin_time;
    v->u.query.want_num = 0;
	pkt->param_len = sizeof(cln_video_t);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

//延迟处理数据,手机取的时候归并
CLIB_API cl_vtap_list_t* cl_get_vtap_list_data(cl_handle_t slave_handle)
{
    cl_vtap_list_t* list = NULL;
    video_t *video;
    slave_t* slave;
    cl_vtap_t* vtap;
    u_int32_t count = 0;
    vtap_record_item_t* vcp;
	
	if (cl_priv == NULL) {
		log_err(false, "client libary is not init now. please call cl_init first!!!!\n");
		return NULL;
	}
	
	cl_lock(&cl_priv->mutex);
    
	video = (video_t *)lookup_by_handle(HDLT_VIDEO, slave_handle);
	if (video == NULL) {
		log_err(false, "cl_get_vtap_list_data: lookup slave handle=0x%08x failed\n", slave_handle);
		goto done;
	}
	slave = video->slave;
    stlc_list_count(count, &video->vtap_list);
    
    if (count == 0) {
        goto done;
    }
    
    list = cl_malloc(sizeof(cl_vtap_t)*count+sizeof(*list));
    if (!list) {
        log_err(false, "cl_get_vtap_list_data malloc memory failed");
        goto done;
    }
    
    list->total_num = 0;
    vtap = list->vtap;
    memset(vtap, 0, sizeof(*vtap));
    stlc_list_for_each_entry(vtap_record_item_t, vcp, &video->vtap_list, link){
        //容忍1秒的断开？？
        log_debug(" begin %u ,duration %u ,next begin %u du %u\n",vtap->begin_time,vtap->duration,vcp->tap_info.begin_time,
               vcp->tap_info.duration);
        if ((vtap->begin_time+vtap->duration /* +1 */) >= vcp->tap_info.begin_time) {
            vtap->duration = vcp->tap_info.begin_time + vcp->tap_info.duration - vtap->begin_time;
            log_debug("to old begin %u duration %u\n",vtap->begin_time,vtap->duration);
        }else{
            //第一次不用移动指针
            if (list->total_num > 0) {
                vtap++;
            }
            
            vtap->begin_time = vcp->tap_info.begin_time;
            vtap->duration = vcp->tap_info.duration;
            
            list->total_num++;
            log_debug("new ------------------- begin %u duration %u count %u\n",vtap->begin_time,vtap->duration,list->total_num);
        }
    }
    
done:
	cl_unlock(&cl_priv->mutex);
    
	return list;
}

CLIB_API void cl_free_vtap_list_data(cl_vtap_list_t* list)
{
    if (list) {
        free(list);
    }
}

CLIB_API RS cl_vtap_start(cl_handle_t slave_handle,u_int32_t begin_time,
                          cl_callback_t callback, void *handle)
{
    cl_notify_pkt_t *pkt;
	cln_video_t *v;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_VTAP_START, CLNPF_ACK);
	
	v = (cln_video_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->callback = callback;
	v->callback_handle = handle;
    v->u.vtap_req.begin_time = begin_time;
    v->u.vtap_req.offset_sec = 0;
	
	pkt->param_len = sizeof(cln_video_t);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_vtap_stop(cl_handle_t handle)
{
    CL_CHECK_INIT;
    
	return cl_video_notify_simple(handle, CLNE_VTAP_STOP, true);
}

CLIB_API RS cl_vtap_get_picture(cl_handle_t handle, u_int32_t* pic_time,void **pic, u_int32_t *size)
{
    RS ret = RS_OK;
	video_t *video;
	data_buf_t *db;
    
	CL_CHECK_INIT;
    
    if (!pic_time || !pic || !size) {
        return RS_INVALID_PARAM;
    }
    
	*pic = NULL;
	*size = 0;
    *pic_time = 0;
	
	cl_lock(&cl_priv->mutex);
	
	video = (video_t *)lookup_by_handle(HDLT_VIDEO, handle);
	if (video == NULL) {
		log_err(false, "cl_vtap_get_picture: lookup slave handle=0x%08x failed\n", handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
    
	if (video->data_pic == NULL) {
		ret = RS_NOT_INIT;
		goto done;
	}
	
	db = de_get_read(video->data_pic);
	if (db == NULL) {
		ret = RS_ERROR;
		goto done;
	}
	*pic = (void *)&db->data[0];
	*size = db->data_size;
    *pic_time = (u_int32_t)(video->cur_vtap_time_offset/100.0+
                            video->cur_vtap_begin_time );
    
done:
	cl_unlock(&cl_priv->mutex);
    
	return ret;
}
