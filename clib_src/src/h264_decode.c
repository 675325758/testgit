#include "client_lib.h"
#include "cl_priv.h"
#include "pic_ring.h"

#include "h264_decode.h"

static void notify_video_proc(video_t *video)
{
	cl_notify_pkt_t *pkt;
	cln_video_t *v;

	pkt = cl_notify_pkt_new(1024, CLNE_VIDEO_DECODE_H264, 0);
	
	v = (cln_video_t *)&pkt->data[0];
	v->slave_halde = video->slave->handle;
	v->callback = NULL;
	v->callback_handle = NULL;
	
	pkt->param_len = sizeof(cln_video_t);

	cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);
}

#ifndef REMOVE_H264_LIB

static void hd_decode(video_t *video)
{
	int bmp_len;
	data_buf_t *db_raw, *db_pic;
	
	while ((db_raw = de_get_read(video->data_h264)) != NULL) {
		db_pic = de_get_write(video->data_pic, BMP_BUF_SIZE);
		bmp_len = Gw_m264_deocde((u_int8_t *)db_pic->data, db_raw->data, db_raw->data_size, video->h264_handle);
#ifdef	DEBUG_PIC_DATA
		log_debug("decode thread: raw=%u bytes, bmp_len=%d\n", db_raw->data_size, bmp_len);
#endif
		de_move_read(video->data_h264);

		if (bmp_len > 0) {
			db_pic->data_size = bmp_len;
			de_move_write(video->data_pic);
			notify_video_proc(video);
		}
	}
}

#endif


static RS hd_proc_notify(video_t *video, cl_thread_info_t *ti, cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	
	log_debug("%s proc notfiy: type=%d, request id=%u, parame len=%d\n",
		ti->name, pkt->type, pkt->request_id, pkt->param_len);
	
	switch (pkt->type) {
	case CLNE_STOP:
		ti->stopping = true;
		break;

	case CLNE_VIDEO_GET_H264:
	case CLNE_WAKEUP:
	default:
#ifndef REMOVE_H264_LIB
		hd_decode(video);
#endif
		break;
	}

	return ret;
}

RS hd_proc_notify_self(cl_notify_pkt_t *pkt)
{
	log_err(false, "解码线程不应该自己处理自己的通告。类型=%u\n", pkt->type);
	return RS_ERROR;
}

u_int32_t h264_decode_thread(void *param)
{
	RS ret;
	cl_notify_pkt_t *notify;
	video_t *video = (video_t *)param;
	cl_thread_info_t *ti = &video->ti_h264_decode;
	struct sockaddr_in from;

	log_info("start decode thread\n");
	
	ti->proc_notify = hd_proc_notify_self;
    
#ifndef REMOVE_H264_LIB
	video->h264_handle = Gw_m264_new(180);


	while ( ! ti->stopping ) {
		while ((notify = cl_recv_notify(ti, 1000, &from)) != NULL) {
			ret = hd_proc_notify(video, ti, notify);
			if (ti->stopping)
				break;

			if ((notify->flags & CLNPF_ACK) != 0) {
				notify->err_code = ret;
				notify->param_len = 0;
				sendto(ti->sock_notify, (char *)notify, notify->hdr_len, 0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
			}
		}
	}

	Gw_m264_del(video->h264_handle);
	video->h264_handle = NULL;

	log_info("H264 decode thread exit now!\n");
#endif
	cl_destroy_thread(ti);
	
	return 0;
}

RS start_h264_decode(video_t *video)
{
	cl_create_thread(&video->ti_h264_decode, "h264_decode", h264_decode_thread, (void *)video);

	return RS_OK;
}

void stop_h264_decode(video_t *video)
{
	cl_notify_destroy_thread(&video->ti_h264_decode);
}

