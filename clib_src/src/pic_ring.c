#include "cl_priv.h"
#include "pic_ring.h"
#include "aes.h"
#include "mp4_fmt.h"
#include "h264_decode.h"
#include "audio_priv.h"

pic_ring_t *pr_alloc()
{
	int i;
	pic_ring_t *pr;

	pr = cl_calloc(sizeof(pic_ring_t), 1);
	
	pr->is_first_pic = true;
	pr->first_seq = 0;
	pr->win = DFL_WIN;
	pr->first_idx = 0;
	pr->cache = (pic_cache_t **)cl_calloc(sizeof(void *)*pr->win, 1);

	pr->quality = -1;

	for (i = 0; i < pr->win; i++) {
		pr->cache[i] = (pic_cache_t *)cl_calloc(sizeof(pic_cache_t), 1);
	}

	return pr;
}

void pr_free(pic_ring_t *pr)
{
	int i;

	if (pr == NULL)
		return;

	for (i = 0; i < pr->win; i++) {
		pr_free_pkt(pr->cache[i]);
		cl_free(pr->cache[i]);
	}

	cl_free(pr->cache);
	cl_free(pr);
}

void pr_free_pkt(pic_cache_t *c)
{
	int i;

	for (i = 0; i < MAX_FRAGS_PER_PIC; i++) {
		SAFE_FREE(c->pkts[i]);
	}
}

void pr_reset(pic_ring_t *pr)
{
	int i;

	if (pr == NULL)
		return;

	// 一些统计
	pr->quality = -1;

	for (i = 0; i < pr->win; i++) {
		pr_free_pkt(pr->cache[i]);
		memset(pr->cache[i], 0, sizeof(pic_cache_t));
	}

	pr->is_first_pic = true;
	pr->first_idx = pr->first_seq = 0;
}

pic_cache_t *pr_get_cache(pic_ring_t *pr, int idx)
{
	return pr->cache[(pr->first_idx + idx)%pr->win];
}

void pr_drop_head(pic_ring_t *pr, u_int16_t n)
{
	int i, drop;
	pic_cache_t *c;

	drop = MIN(n, pr->win);

	for (i = 0; i < drop; i++) {
		c = pr_get_cache(pr, i);
		
		pr_free_pkt(c);
		memset(c, 0, sizeof(pic_cache_t));
	}
	
	pr->first_seq += n;
    pr->current_seq = pr->first_seq;
	pr->first_idx = (pr->first_idx + n)%pr->win;
	log_debug("pr->first_seq %d\n",pr->first_seq);
#if DEBUG_PIC_DATA
	log_debug("DropHead(%d), m_nFirstSeq=%u, m_nFirstIdx=%u\n", n, pr->first_seq, pr->first_idx);
#endif
}

int get_pic_size(pic_ring_t *pr, int idx)
{
	int i, total, hdr_size;
	pic_cache_t *c;

	c = pr->cache[idx];
	if (pr->type == PRT_VOICE) {
		hdr_size = sizeof(net_voice_t);
	} else {
		hdr_size = sizeof(net_video_jpg_v1_t);
	}

	for (i = 0, total = 0; i < c->total; i++) {
		total += net_param_len(c->pkts[i]->data) - hdr_size;
	}

	return total;
}

int pr_get_pic_data(pic_ring_t *pr, int idx, u_int8_t *buf, u_int8_t *pwd_md5)
{
	int i, n, total;
	pic_cache_t *c;
	net_video_jpg_v1_t *jpg;

	c = pr->cache[idx];
	cl_assert(c->recv_count == c->total);


	for (i = 0, total = 0; i < c->total; i++) {
		jpg = get_pkt_payload(c->pkts[i], net_video_jpg_v1_t);
		n = net_param_len(c->pkts[i]->data) - sizeof(net_video_jpg_v1_t);

		if (pwd_md5 == NULL) {
			memcpy(buf + total, jpg->data, n);
		} else {
			dec_block2(buf + total, jpg->data, n, pwd_md5);
		}
		total += n - jpg->pad_len;
	}
	 pr->quality = jpg->quality;

#if DEBUG_PIC_DATA
	log_debug("pr_get_pic_data #%u total=%d bytes\n", jpg->seq, total);
#endif

	return total;
}

int pr_get_sound_data(pic_ring_t *pr, int idx, u_int8_t *buf, u_int8_t *pwd_md5)
{
	int i, n, total;
	pic_cache_t *c;
	net_voice_t *vc;

	c = pr->cache[idx];
	cl_assert(c->recv_count == c->total);


	for (i = 0, total = 0; i < c->total; i++) {
		vc = get_pkt_payload(c->pkts[i], net_voice_t);
		n = net_param_len(c->pkts[i]->data) - sizeof(net_voice_t);

		if (pwd_md5 == NULL) {
			memcpy(buf + total, vc->data, n);
		} else {
			dec_block2(buf + total, vc->data, n, pwd_md5);
		}
		total += n - vc->pad_len;
	}

#if DEBUG_PIC_DATA
	log_debug("pr_get_sound_data #%u total=%d bytes\n", vc->seq, total);
#endif

	return total;
}

void do_get_jpg(video_t *video, pic_ring_t *pr, int idx)
{
	int total, width, height;
	data_buf_t *db;

	total = get_pic_size(pr, idx);
	if (video->data_pic == NULL)
		video->data_pic = de_alloc(NUM_PIC_DB, total, true);
	
	db = de_get_write(video->data_pic, total);
	db->data_size = pr_get_pic_data(pr, idx, db->data, video->is_aes_enc ? video->m_Syna.key_rand : NULL);
	de_move_write(video->data_pic);

	if (parse_jpg_wh(db->data, db->data_size, &width, &height) == RS_OK) {
		video->stat.width = width;
		video->stat.height = height;
	}
	
	event_push(video->callback, VE_GET_PICTURE, video->handle, video->callback_handle);
}

void do_get_h264(video_t *video, pic_ring_t *pr, int idx)
{
	int total, width, height;
	data_buf_t *db;

	if (video->data_pic == NULL)
		video->data_pic = de_alloc(NUM_PIC_DB, BMP_BUF_SIZE, true);

	total = get_pic_size(pr, idx);
	
	db = de_get_write(video->data_h264, total);
	db->data_size = pr_get_pic_data(pr, idx, db->data, video->is_aes_enc ? video->m_Syna.key_rand : NULL);
	de_move_write(video->data_h264);

	if (h264_get_pic_size(db->data, db->data_size, &width, &height) == 0) {
		if (width != video->stat.width || height != video->stat.height) {
//			log_info("视频分辨率变化: 从%ux%u 调整为 %ux%u, 重启解码线程\n", 
//				video->stat.width, video->stat.height, width, height);
			if (video->stat.width != 0) {
				stop_h264_decode(video);
				start_h264_decode(video);
			}

			video->stat.width = width;
			video->stat.height = height;
		}
	}

	cl_send_notify_simple(&video->ti_h264_decode, CLNE_VIDEO_GET_H264);
}
extern int timer_video_nack(cl_thread_t *t);
bool pr_put_jpg(video_t *video, pic_ring_t *pr, pkt_t *pkt, net_video_jpg_v1_t *jpg)
{
	u_int16_t idx;
	pic_cache_t *c;

	if (jpg->total == 0 || jpg->total > MAX_FRAGS_PER_PIC || jpg->index >= jpg->total) {
		log_err(false, "********** FAILED: jpg->total=%u, seq=#%u, idx=%u, total=%u ************\n",
			jpg->total, jpg->seq, jpg->index, jpg->total);
		cl_free(pkt);
		return false;
	}

	video_update_stat_bytes(video, pkt->total);

	if (u16_is_bigger(pr->first_seq, jpg->seq) && ( ! pr->is_first_pic) ) {
		log_info("Ignore old picture(#%u, now first seq=#%u).\n", jpg->seq, pr->first_seq);
		video->stat.old_frags++;
		cl_free(pkt);
		return false;
	}
	
	if (pr->is_first_pic) {
		pr->is_first_pic = false;
		pr->first_seq = jpg->seq;
		pr->current_seq = jpg->seq;
		log_debug("pr->first_seq %d\n",pr->first_seq);
	}

	log_debug("current_seq %d, jpg seq %d [%d/%d]\n",pr->current_seq, jpg->seq, jpg->index, jpg->total);
	
	//新的一组数据过来，全部缓冲区顶出
	if (u16_is_bigger(jpg->seq, pr->first_seq + pr->win - 1)) {
		log_debug("new group\n");
		pr_drop_head(pr, pr->win);
	}

	idx = ((u_int16_t)(pr->first_idx + jpg->seq - pr->first_seq))%pr->win;
	c = pr->cache[idx];
	if (c->total == 0) {
		c->total = jpg->total;
		c->seq = jpg->seq;
	}
	if (c->pkts[jpg->index] != NULL) {
		log_info("*** duplicate, ignore ***\n");
		cl_free(pkt);
		return true;
	}

	// cache pkt
	c->pkts[jpg->index] = pkt;
	c->recv_count++;
	if (c->nack_send)
		c->recv_after_nack++;

	// 立即发送，显示统计
	//vw->SendMessage(WM_PIC_FRAG_RECV, jpg->seq, 0);

	//UDP 1s没收到视频报文，发送本组数据的nack报文
	CL_THREAD_TIMER_OFF(video->t_nack);
	if (!VALID_SOCK(video->vtry.sock_tcp)) {
        CL_THREAD_TIMER_ON(&cl_priv->master, video->t_nack, timer_video_nack, video, 1000);
    }

	if (c->seq == pr->current_seq && c->recv_count == c->total) {
		video_update_stat_frames(video);
		
		log_debug("recieve pic total  = %d frags, timestap = %d\n", c->total, jpg->timestamp);
		if (video->is_h264) {
			//完整报文依次解码
			do{
				do_get_h264(video, pr, idx);
				pr->current_seq++;
				idx = ((u_int16_t)(pr->first_idx + pr->current_seq - pr->first_seq))%pr->win;
				c = pr->cache[idx];
			}while (c->seq == pr->current_seq && c->total == c->recv_count);
		} else { /* jpg */
			do_get_jpg(video, pr, idx);
		}
		
#if DEBUG_PIC_DATA
		log_debug("Show pic #%u done.\n", jpg->seq);
#endif		
		//完整报文单独发送nack，在网络比较好的情况下可以避免nack批量发送，nack可能需要重发，这里不释放缓刑缓冲区，由新的一组帧发送过来时统一一起释放
		video_send_nack(video, (u_int16_t)(jpg->seq - pr->first_seq), jpg->seq);
	}

	return true;
}

bool pr_put_sound(video_t *video, pic_ring_t *pr, pkt_t *pkt, net_voice_t *vc)
{
	u_int16_t idx;
	pic_cache_t *c;

	if (vc->total == 0 || vc->total > MAX_FRAGS_PER_PIC || vc->index >= vc->total) {
		log_err(false, "********** FAILED: vc->total=%u, seq=#%u, idx=%u, total=%u ************\n",
			vc->total, vc->seq, vc->index, vc->total);
		cl_free(pkt);
		return false;
	}

	video->stat.sound_format = vc->format;
	video->stat.sound_channel = vc->channel;
	video->stat.sound_bit = vc->bit;
	video->stat.sample = vc->rate;

	video_update_stat_bytes(video, pkt->total);

	if (u16_is_bigger(pr->first_seq, vc->seq) && ( ! pr->is_first_pic) ) {
		log_info("Ignore old voice(#%u, now first seq=#%u).\n", vc->seq, pr->first_seq);
		video->stat.old_frags++;
		cl_free(pkt);
		return false;
	}
	
	if (pr->is_first_pic) {
		pr->is_first_pic = false;
		pr->first_seq = vc->seq;
	}

	// 越界，顶出
	if (u16_is_bigger(vc->seq, pr->first_seq + pr->win - 1)) {
		video->stat.drop_frames += vc->seq - (pr->first_seq + pr->win - 1);
		pr_drop_head(pr, vc->seq - (pr->first_seq + pr->win - 1));
	}

	idx = ((u_int16_t)(pr->first_idx + vc->seq - pr->first_seq))%pr->win;
	c = pr->cache[idx];
	if (c->total == 0) {
		c->total = vc->total;
	}
	if (c->pkts[vc->index] != NULL) {
		log_info("*** duplicate audio, ignore ***\n");
		cl_free(pkt);
		return true;
	}

	// cache pkt
	c->pkts[vc->index] = pkt;
	c->recv_count++;
	if (c->nack_send)
		c->recv_after_nack++;

	// 立即发送，显示统计
	//vw->SendMessage(WM_PIC_FRAG_RECV, vc->seq, 0);

	if (c->recv_count == c->total) {
		//video_update_stat_frames(video);
		
		log_debug("recieve voice total  = %d frags, timestap = %d\n", c->total, vc->timestamp);
		do_get_sound(video, pr, idx);
		
#if DEBUG_PIC_DATA
		log_debug("Play voice #%u done.\n", vc->seq);
#endif		

		//video_send_nack(video, (u_int16_t)(vc->seq - pr->first_seq), vc->seq);
		pr_drop_head(pr, (idx + pr->win - pr->first_idx)%pr->win + 1);
	} else if (vc->seq != pr->first_seq) {
		// 集中火力发上一帧，再前面的就算了
		//video_send_nack(video, (u_int16_t)(vc->seq - pr->first_seq - 1), (u_int16_t)(vc->seq - 1));
	}

	return true;
}

