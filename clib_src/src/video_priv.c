#include "client_lib.h"
#include "cl_priv.h"
#include "md5.h"
#include "cl_notify.h"
#include "cl_thread.h"
#include "cl_user.h"
#include "net_detect.h"
#include "user_priv.h"
#include "video_priv.h"
#include "aes.h"
#include "h264_decode.h"
#include "audio_priv.h"
#include "wait_server.h"
#include "video_record_timer.h"

#define TCP_AGENT
/////////////////////////////////////////////////////////////////////////

#define	TIME_HELLO_REQ	500

/* p2p TCP连接的超时时间 */
#define	TIME_VIDEO_P2P_TCP_CONN_TIMEOUT	4000
/* p2p UDP连接的超时时间 */
#define	TIME_VIDEO_P2P_UDP_CONN_TIMEOUT	5000
/* syn_a中LAN地址连接的超时时间 */
#define	TIME_VIDEO_LAN_UDP_CONN_TIMEOUT	4000
/* syn_a中WAN地址连接的超时时间 */
#define	TIME_VIDEO_WAN_UDP_CONN_TIMEOUT	10000

#define	HELLO_REQ_COUNT	10
//视频饱和度超时
#define MAX_VIDEO_SATURTAION_INTERVAL 12*1000
//视频饱和度定时器
#define MAX_VIDEO_SATURTAION_TIMER 5*1000

/////////////////////////////////////////////////////////////////////////


void video_set_last_error(video_t *video, const char *err);
void video_restart(video_t *video, int delay);
void video_update_die(video_t *video);
RS video_set_stat_err(video_t *video, const char *err);
void video_retry(video_t *video);
void video_on_tcp_connect(video_t *video);
video_t *video_lookup_by_status(user_t *user, int status);
int video_tcp_read(cl_thread_t *t);
int video_tcp_write(cl_thread_t *t);
static void video_get_static_pic(video_t *video);


/////////////////////////////////////////////////////////////////////////

bool OpenH264Decode(video_t *video)
{
	if (video->ti_h264_decode.proc_notify != NULL)
		return true;

	if (video->data_h264 == NULL) {
		video->data_h264 = de_alloc(10, 1024, false);
	}
	
	if (start_h264_decode(video) != RS_OK)
		return false;

	return true;
}


void video_set_status(video_t *video, int status)
{
	int prev_status;
	video_stat_t *vs = &video->stat;
	
   // TRACE("%s : status before = %d, after = %d\n", m_strTitle, m_nStatus, status);

	prev_status = video->status;
	video->status = status;

	// 从成功到失败
	if (prev_status == VS_ESTABLISH && status != prev_status) {
		time_t now;
		
   		 video->is_open_success = false;

		now = get_sec();
		
		vs->failed[vs->fr_write].begin = vs->this_begin;
		vs->failed[vs->fr_write].duration = (u_int32_t)now - (u_int32_t)vs->this_begin;
		strcpy(vs->failed[vs->fr_write].reson, vs->reson);
		vs->reson[0] = '\0';
		vs->fr_write = (vs->fr_write + 1)%MAX_FAILED_RECORD;
		vs->retry++;
		
		vs->prev_see += ((u_int32_t)now - (u_int32_t)vs->this_begin);
		vs->this_begin = 0;

		vs->width = 0;
		vs->height = 0;

		CL_THREAD_OFF(video->t_roll);
	}

	if ((prev_status == VS_VIDEO_REQ || prev_status == VS_SYN_A_RCV) && status - prev_status != 1) {
		vs->failed[vs->fr_write].begin = get_sec();
		vs->failed[vs->fr_write].duration = 0;
		if (prev_status == VS_VIDEO_REQ) {
			strcpy(vs->failed[vs->fr_write].reson, "等待SYN A超时");
		} else {
			strcpy(vs->failed[vs->fr_write].reson, "建立视频连接超时");
		}
		vs->fr_write = (vs->fr_write + 1)%MAX_FAILED_RECORD;
		vs->retry++;
	}
	
	if (status == VS_ESTABLISH) {
		// 从失败到成功
		if (prev_status != VS_ESTABLISH) {
			video_try_one_t *vt_top = vt_get_this_try(&video->vtry);
			
			vs->see_count++;
			vs->this_begin = get_sec();
			vs->is_tcp = vt_top->is_tcp;
			vs->dev_ip = vt_top->ip;
			vs->dev_port = vt_top->port;
			
			vt_set_die(&video->vtry, TIME_VIDEO_DIE);
			
			event_push(video->callback, VE_ESTABLISH, video->handle, video->callback_handle);	

			video->pic_ring->type = video->is_h264 ? PRT_H264 : PRT_JPG;
			if (video->sound_ring == NULL) {
				video->sound_ring = pr_alloc();
				video->sound_ring->type = PRT_VOICE;
			}
			if (video->data_sound == NULL) {
				video->data_sound = de_alloc(NUM_VOICE_DB, VOICE_BUF_SIZE, false);
			}
			
			// 启动解码进程
			if (video->is_h264 && ( ! OpenH264Decode(video))) {
				video_set_last_error(video, "启动解码线程失败");
				video_restart(video, TIME_DELAY_SYS_ERROR);
				return;
			}
		}
	} else if (status == VS_IDLE || status == VS_ERROR) {
		if (status == VS_ERROR && video->last_err == NULL) {
			video_set_last_error(video, "等待重连。。。");
		}
	}
}
//出错时使用
void vtap_close(video_t *video);

void callback_video_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	slave_t *slave;
    user_t * user;

	if ((slave = (slave_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_video_request, not found user type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

    user = slave->user;
	switch (w->cmd) {
	case CMD_VIDEO_SYN_Q:
		if (slave->video->status == VS_VIDEO_REQ) {
			log_err(false, "callback_video_request, CMD_VIDEO_SYN_Q result=%u, restart video\n", result);
			video_restart(slave->video, TIME_DELAY_SYS_ERROR);
		}
		break;
    case CMD_V4L2_COLOR:
        if (slave->video) {
            u_int32_t action = (u_int32_t)w->param;
            if (action == e_V4L2_CTRL) {
                event_push(slave->video->callback, VE_SET_V4L2_FAILED, slave->video->handle, slave->video->callback_handle);
            }
        }
        break;
    case CMD_MOTO_ATTRIBUTE:
        if (slave->video) {
            u_int32_t action = (u_int32_t)w->param;
            if (action == e_V4L2_CTRL) {
                event_push(slave->video->callback, VE_SET_ROLL_SPEED_FAILED, slave->video->handle, slave->video->callback_handle);
            }
            
        }
        break;
    case CMD_VTAP_LIST_Q:
        {
            event_push(user->callback, VE_VTAP_GET_LIST_FAILED,slave->handle,user->callback_handle);
            if (slave->video) {
                vtap_close(slave->video);
            }
        }
            break;
	default:
		event_push(slave->video->callback, VE_VTAP_CHECK_ERR, slave->video->handle, slave->video->callback_handle);
		event_cancel_merge(slave->video->handle);
		log_err(false, "callback_video_request, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}

bool video_send_syn_q(video_t *video, int vtapId)
{
	net_video_syn_q_t *p;
	pkt_t *pkt;
	u_int8_t em[] = {ENC_FRAG_NONE, ENC_FRAG_AES128, ENC_FRAG_AES128_UDP_PTZ, ENC_FRAG_AES128_H264,
		ENC_FRAG_AES128_H264_AUDIO_ONEWAY, ENC_FRAG_AES128_H264_AUDIO_TWOWAY};
	int ec = sizeof(em);
	slave_t *slave = video->slave;
	video_try_t *vt = &video->vtry;
	
	video_set_status(video, VS_VIDEO_REQ);
	video_update_die(video);

	pkt = pkt_new_v2(CMD_VIDEO_SYN_Q, sizeof(net_video_syn_q_t) + ec, 0, slave->sn, TP_USER);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VIDEO_SYN_Q, NULL, callback_video_request);
	
	p = get_pkt_payload(pkt, net_video_syn_q_t);

	p->local_ip = htonl(slave->user->local_ip);
	p->global_ip = htonl(slave->user->global_ip);
	p->local_port = htons(vt->local_port);
	p->global_port = 0;
	p->request_agent = vt->is_agent;// | ((vtapId & 0x0f) << 4);
	p->vtap_id = vtapId;
	p->encrypt_count = ec;
	memcpy(&p->encrypt_method[0], em, ec);

	log_debug("%s video_send_syn_q: local ip=%u.%u.%u.%u, global ip=%u.%u.%u.%u, local port=%u, agent=%d\n",
		slave->str_sn,
		IP_SHOW(slave->user->local_ip), IP_SHOW(slave->user->global_ip),
		vt->local_port, vt->is_agent);

	vt_reset_try(vt);
	vt_add_timeout(vt, TIME_VIDEO_WAIT_SYN_A);

	user_add_pkt(slave->user, pkt);

	return true;
}

bool video_send_stop(video_t *video)
{
	u_int8_t *p;
	pkt_t *pkt;
	slave_t *slave = video->slave;

	if (slave->user->status != CS_ESTABLISH) {
		log_err(false, "%s status=%u, ignore send video stop\n", slave->str_sn, slave->user->status);
		return false;
	}
	
	pkt = pkt_new_v2(CMD_VIDEO_STOP, sizeof(u_int8_t), 0, slave->sn, TP_USER);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VIDEO_STOP, NULL, callback_video_request);
	
	p = get_pkt_payload(pkt, u_int8_t);
	*p = video->m_Syna.client_id;

	log_info("video_send_stop=%d\n", video->m_Syna.client_id);

	user_add_pkt(slave->user, pkt);

	return true;
}

RS video_tcp_try_write(video_t *video)
{
	int len;
	pkt_t *pkt;

	pkt = stlc_list_first_entry(&video->tcp_pkt_list, pkt_t, link);
	len = send(video->vtry.sock_tcp, pkt->data + video->tcp_send_len, pkt->total - video->tcp_send_len, 0);
	if (len < 0) {
		if ( ! NORMAL_ERRNO(GET_SOCK_ECODE()) ) {
			log_err(true, "video_tcp_try_write failed, errno=%d, total=%d, prev_send=%d\n",
				GET_SOCK_ECODE(), pkt->total, video->tcp_send_len);
			return RS_ERROR;
		}

		return RS_OK;
	}

	log_debug("video_tcp_try_write prev_send=%d, send %d of %d bytes\n", video->tcp_send_len, len, pkt->total);

	video->tcp_send_len += len;
	if (video->tcp_send_len == pkt->total) {
		video->tcp_send_len = 0;
		stlc_list_del(&pkt->link);
		pkt_free(pkt);
	}
	
	return RS_OK;
}

bool video_udp_send(video_t *video, pkt_t *pkt, int ip, int port)
{
	bool ret = false;
	struct sockaddr_in addr;
	int len;
	
	if ( ( ! VALID_SOCK(video->vtry.sock_udp)) || ip == 0 || port == 0)
		goto err;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	len = sizeof(addr);


	
	if ((len = (int)sendto(video->vtry.sock_udp, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len)) != pkt->total) {
		log_err(true, "1. 发送UDP数据失败, ip=%u.%u.%u.%u, port=%u, len=%d", IP_SHOW(ip), port, len);
	}
#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, pkt->total);
#endif
	
	ret = true;
	
err:
	cl_free(pkt);
	return ret;
}

bool video_send(video_t *video, pkt_t *pkt)
{
	bool ret = true;
	
	if (VALID_SOCK(video->vtry.sock_tcp)) {
		stlc_list_add_tail(&pkt->link, &video->tcp_pkt_list);
		CL_THREAD_WRITE_ON(&cl_priv->master, video->t_tcp_write, video_tcp_write, (void *)video, video->vtry.sock_tcp);
	} else {
		ret = video_udp_send(video, pkt, video->last_recv_ip, video->last_recv_port);
	}

	return ret;
}

bool video_send_nack(video_t *video, u_int16_t idx, u_int16_t seq)
{
	int i, count;
	u_int8_t nack[MAX_FRAGS_PER_PIC];
	pkt_t *pkt;
	net_video_ack_v1_t *ack;
	pic_cache_t *c;

	c = pr_get_cache(video->pic_ring, idx);
	if (c->nack_send)
		return true;

	for (i = 0, count = 0; i < c->total; i++) {
		if (c->pkts[i] == NULL) {
			nack[count++] = i;
		}
	}

	pkt = pkt_new(CMD_VIDEO_ACK, sizeof(net_video_ack_v1_t) + count, TP_USER);
	ack = get_pkt_payload(pkt, net_video_ack_v1_t);
	ack->v0.client_id = video->m_Syna.client_id;
	ack->nack_number = count;
	ack->nack_seq = ntohs(seq);
	for (i = 0; i < count; i++) {
		ack->nack[i] = nack[i];
	}

	video_send(video, pkt);
    log_debug("send nack, seq %d count %d\n", seq, count);

	return true;
}

bool video_send_hello(video_t *video)
{
	pkt_t *pkt;
	net_vedio_hello_t *p;
	MD5_CTX ctx;

	if (VALID_SOCK(video->vtry.sock_tcp) && (!video->vtry.is_connect_ok))
		return false;

	pkt = pkt_new(CMD_VIDEO_HELLO, sizeof(net_vedio_hello_t) + (video->status >= VS_SYN_A_RCV ? 16 : 0), TP_USER);
	p = get_pkt_payload(pkt, net_vedio_hello_t);

	p->sn = ntoh_ll(video->slave->sn);
	p->client_id = video->m_Syna.client_id;
	p->ack = 1;
	p->client_type = cl_priv->cleint_type;

	if (video->status >= VS_SYN_A_RCV) {
		p->client_id = video->m_Syna.client_id;
		p->auth_len = 16;

		MD5Init(&ctx);
		MD5Update(&ctx, &p->client_id, 1);
		
		MD5Update(&ctx, video->m_Syna.auth_rand, 16);
		MD5Update(&ctx, video->slave->user->passwd_md5, 16);
		MD5Final(p->auth, &ctx);
	}

	video_send(video, pkt);

	return true;
}

static void vtap_proc_jpg_offset(video_t* video);
int timer_video_nack(cl_thread_t *t)
{
    u_int16_t seq;
    int i,count;
    u_int8_t nack[MAX_FRAGS_PER_PIC];
    pkt_t *pkt;
    net_video_ack_v1_t *ack;
    pic_cache_t *c;
    video_t *video = (video_t *)CL_THREAD_ARG(t);
    video->t_nack = NULL;
	for (seq = video->pic_ring->first_seq;
		 u16_is_bigger(video->pic_ring->first_seq + video->pic_ring->win, seq);
		 seq++) {
		c = pr_get_cache(video->pic_ring, seq - video->pic_ring->first_seq);
        if (c->seq == seq) {
            for (i = 0, count = 0; i < c->total; i++) {
                if (c->pkts[i] == NULL) {
                    nack[count++] = i;
                }
            }
        } else {
            count = MAX_FRAGS_PER_PIC; //完全丢帧的情况
        }
        pkt = pkt_new(CMD_VIDEO_ACK, sizeof(net_video_ack_v1_t) + count, TP_USER);
        ack = get_pkt_payload(pkt, net_video_ack_v1_t);
        ack->v0.client_id = video->m_Syna.client_id;
        ack->nack_number = count;
        ack->nack_seq = htons(seq);
        for (i = 0; i < count; i++) {
            ack->nack[i] = nack[i];
        }
        video_send(video, pkt);
        log_debug("resend nack, seq %d c->seq %d count %d\n", seq, c->seq, count);
    }
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_nack, timer_video_nack, video, 1000);
	return 0;
}

bool video_proc_jpg(video_t *video, net_header_t *hdr)
{
	net_video_jpg_v1_t *jpg;
	int total;
	pkt_t *pkt;
	
	if (video->status == VS_ERROR) {
		return false;
	}
    
    if (video->is_vtap && video->status <= VS_VTAP) {
        return true;
    }

	if (video->status < VS_ESTABLISH) {
		video_set_status(video, VS_ESTABLISH);
        video->is_open_success = true;
		log_info("video %s ******** video establish **************(video)\n", video->slave->str_sn);
	}

	jpg = get_net_pkt_payload(hdr, net_video_jpg_v1_t);
	jpg->seq = ntohs(jpg->seq);
	jpg->timestamp = ntohl(jpg->timestamp);
    if (video->is_vtap) {
        video->cur_vtap_time_offset = jpg->timestamp;
        vtap_proc_jpg_offset(video);
    }

	if (jpg->quality != video->pic_quality) {
		log_info("video %s jpg quality modify from %u to %u\n",
			video->slave->str_sn, video->pic_quality, jpg->quality);
		video->pic_quality = jpg->quality;
		vrt_quick_query(video);
	}

	// MU有BUG，没有进行字节序转化，只有通过标志来判定字节序了
	if (jpg->resv & 0x80) {
		video->stat.client_count = jpg->resv & 0x7F;
	} else if (jpg->resv & 0x8000) {
		video->stat.client_count = (jpg->resv & 0x7F00) >> 8;
	} else {
		video->stat.client_count = 0;
	}

#if DEBUG_PIC_DATA
	log_debug("video %s, SEQ #%u frag %u/%u, size=%d bytes\n",
		video->slave->str_sn, jpg->seq, jpg->index, jpg->total, hdr->param_len - sizeof(net_video_jpg_v1_t));
#endif

	total = hdr->param_len + net_hdr_len(hdr);
	pkt = cl_malloc(total + sizeof(pkt_t));
	pkt->total = total;
	memcpy(pkt->data, hdr, total);
	pr_put_jpg(video, video->pic_ring, pkt, jpg);

	video_update_die(video);

	return true;
}

bool video_proc_voice(video_t *video, net_header_t *hdr)
{
	net_voice_t *vc;
	int total;
	pkt_t *pkt;
	
	if (video->status == VS_ERROR) {
		return false;
	}

	if (video->status < VS_ESTABLISH) {
		video_set_status(video, VS_ESTABLISH);
        video->is_open_success = true;
		log_info("video %s ******** video establish **************(audio)\n", video->slave->str_sn);
	}

	vc = get_net_pkt_payload(hdr, net_voice_t);
	vc->seq = ntohs(vc->seq);
	vc->timestamp = ntohl(vc->timestamp);

#if DEBUG_PIC_DATA
	log_debug("audio %s, SEQ #%u frag %u/%u, size=%d bytes\n",
		video->slave->str_sn, vc->seq, vc->index, vc->total, hdr->param_len - sizeof(net_voice_t));
#endif

	total = hdr->param_len + net_hdr_len(hdr);
	pkt = cl_malloc(total + sizeof(pkt_t));
	pkt->total = total;
	memcpy(pkt->data, hdr, total);
	pr_put_sound(video, video->sound_ring, pkt, vc);

	video_update_die(video);

	return true;
}

int video_udp_recv(cl_thread_t *t)
{
	int addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	video_t *video = (video_t *)CL_THREAD_ARG(t);
	net_header_t *hdr;
	pkt_t *pkt;

	video->t_udp = NULL;
	
	CL_THREAD_READ_ON(&cl_priv->master, video->t_udp, video_udp_recv, video, video->vtry.sock_udp);

	pkt = video->slave->user->udp_buf;
	pkt->total = (int)recvfrom(video->vtry.sock_udp, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, &addr_len);
	if (pkt->total < (int)net_header_size) {
		log_err(false, "Ignore bad udp packet: len=%d, %u\n", pkt->total, video->vtry.sock_udp);
		return 0;
	}

	video->last_recv_ip = ntohl(addr.sin_addr.s_addr);
	video->last_recv_port = ntohs(addr.sin_port);

	hdr_order(pkt);
	hdr = (net_header_t *)pkt->data;
	if ((u_int32_t)pkt->total < hdr->param_len + net_hdr_len(hdr)) {
		log_err(false, "Ignore bad udp packet: len=%d, hdr->param_len=%u, net_hdr_len(hdr)=%u\n",
			pkt->total, hdr->param_len, net_hdr_len(hdr));
		return 0;
	}
    
#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(true, pkt->total);
#endif

	log_debug("video %s recv from %u.%u.%u.%u:%u, cmd=%u, len=%u\n",
		video->slave->str_sn, IP_SHOW(video->last_recv_ip), video->last_recv_port, 
		hdr->command, pkt->total);

	switch (hdr->command) {
	case CMD_VIDEO_HELLO:
		video_send_hello(video);
		break;
		
	case CMD_VIDEO_JPG:
		video_proc_jpg(video, hdr);
		break;
	case CMD_VOICE:
		video_proc_voice(video, hdr);
		break;
	}
	
	return 0;
}

/*
	Windows: 比较麻烦，还要看exceptfds，算了，等超时
		If the client uses the select function, success is reported in the writefds set and failure
		is reported in the exceptfds set
	Linux:
		It is possible to select(2) or poll(2) for completion by  selecting
		the  socket  for  writing.  After select(2) indicates writability, use getsockopt(2)
		to read the SO_ERROR option at  level  SOL_SOCKET  to  determine
		whether  connect()  completed successfully (SO_ERROR is zero) or unsuccess-
		fully (SO_ERROR is one of the usual error codes listed here, explaining the
		reason for the failure).	
*/
static RS video_tcp_check_connecting(video_t *video)
{
	int status;
	int slen;
	int ret;

	slen = sizeof(status);
	ret = (int)getsockopt(video->vtry.sock_tcp, SOL_SOCKET, SO_ERROR, (void *)&status, &slen);
	if (ret < 0) {
		log_err(true, "video_tcp_check_connecting %s getsockopt failed\n", video->slave->str_sn);
		video_retry(video);
		return RS_ERROR;
	}
	if (status != 0) {
		log_err(false, "video_tcp_check_connecting %s status=%d\n", video->slave->str_sn, status);
		video_retry(video);
		return RS_ERROR;
	}

	video_on_tcp_connect(video);

	CL_THREAD_OFF(video->t_tcp_write);
	CL_THREAD_OFF(video->t_tcp_read);
	CL_THREAD_READ_ON(&cl_priv->master, video->t_tcp_read, video_tcp_read, (void *)video, video->vtry.sock_tcp);

	return RS_OK;
}

int video_tcp_read(cl_thread_t *t)
{
	buff_t *bf;
	int want, n;
	video_t *video = (video_t *)CL_THREAD_ARG(t);
	net_header_t *hdr;
	net_header_v2_t *hdr_v2;

	video->t_tcp_read = NULL;
	CL_THREAD_READ_ON(&cl_priv->master, video->t_tcp_read, video_tcp_read, (void *)video, video->vtry.sock_tcp);

	if ( ! video->vtry.is_connect_ok ) {
		log_info("video_tcp_read, 检查是否连接成功\n");
		video_tcp_check_connecting(video);
		return 0;
	}

	bf = video->tcp_buff;

	if (bf->read_pos < net_header_size) {
		want = net_header_size - bf->read_pos;
	} else {
		want = bf->pkt_size - bf->read_pos;
	}

	n = recv(video->vtry.sock_tcp, bf->buf + bf->read_pos, want, 0);
	if (n == 0) {
		log_err(false, "video %s close by peer\n", video->slave->str_sn);
		goto err;
	} else if (n < 0) {
		if (NORMAL_ERRNO(GET_SOCK_ECODE()))
			return 0;
		
		log_err(true, "video %s read tcp failed\n", video->slave->str_sn);
		goto err;
	}

	bf->read_pos += n;
	if (bf->read_pos < net_header_size) {
		goto done;
	} else if (bf->read_pos == net_header_size) {
		hdr = (net_header_t *)bf->buf;
		hdr->command = ntohs(hdr->command);
		hdr->param_len = ntohl(hdr->param_len);

		bf->pkt_size = net_header_size + hdr->param_len;
#if 0
if (hdr->param_len > 1500)	{
	u_int32_t *p;
	log_debug("pkt_size=%d, hdr->param_len=%d\n", bf->pkt_size, hdr->param_len);
	p = (u_int32_t *)hdr;
	log_debug("%08x %08x %08x\n", p[0], p[1], p[2]);
	cl_assert(0);
}
#endif
		if (bf->pkt_size > bf->buf_size) {
			if ( ! bf_expand(bf, bf->pkt_size ) ) {
				log_err(false, "video %s read too big pkt: cmd=%u, len=%u, but buf size=%u\n",
					video->slave->str_sn, hdr->command, bf->pkt_size, bf->buf_size);
				goto err;
			}
		}
	}

	if (bf->read_pos < bf->pkt_size) 
		goto done;

	hdr = (net_header_t *)bf->buf;
	hdr_v2 = (net_header_v2_t *)hdr;
	if (hdr->ver >= 2) {
		if (net_hdr_len(hdr) < net_header_v2_size || bf->pkt_size < net_header_v2_size) {
			log_err(false, "video %s Bad packet: ver=%d, hlen=%d\n", video->slave->str_sn, hdr->ver, net_hdr_len(hdr));
			goto err;
		}
		hdr_v2->sn = ntoh_ll(hdr_v2->sn);
	}
    
#ifdef SUPPORT_TRAFFIC_STAT
    TCP_CMD_PKT_STAT(ntohs(hdr->command),true, n);
#endif

	log_debug("video %s Tcp recv packet: commond=%u, param_len=%u, handle=%x\n",
		video->slave->str_sn, hdr->command, hdr->param_len, hdr->handle);

	// 完整报文，处理之
	switch (hdr->command) {
	case CMD_VIDEO_HELLO:
		video_send_hello(video);
		break;
		
	case CMD_VIDEO_JPG:
		video_proc_jpg(video, hdr);
		break;
	case CMD_VOICE:
		video_proc_voice(video, hdr);
		break;
	default:
		log_err(false, "tcp cmd not support, cmd = %hu\n", hdr->command);
		goto err;
	}

	bf->pkt_size = bf->read_pos = 0;

done:
	return 0;

err:
	event_push(video->callback, VE_VTAP_CHECK_ERR, video->handle, video->callback_handle);
	event_cancel_merge(video->handle);
	video_retry(video);
	return 0;
}

// 平时都是直接发，只有连接时候会到这里
int video_tcp_write(cl_thread_t *t)
{
	video_t *video = (video_t *)CL_THREAD_ARG(t);

	video->t_tcp_write = NULL;

	if ( ! video->vtry.is_connect_ok ) {
		log_info("video_tcp_write, 检查是否连接成功\n");
		video_tcp_check_connecting(video);
		return 0;
	}

	if (stlc_list_empty(&video->tcp_pkt_list))
		return 0;

	if (video_tcp_try_write(video) != RS_OK) {
		video_retry(video);
		return 0;
	}
	
	if ( ! stlc_list_empty(&video->tcp_pkt_list) ) {
		CL_THREAD_WRITE_ON(&cl_priv->master, video->t_tcp_write, video_tcp_write, (void *)video, video->vtry.sock_tcp);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////

void video_reply_hello_req(video_t *video)
{
	pkt_t *pkt;
	net_vedio_hello_t *p;
	hello_q_t *hq;
	struct sockaddr_in addr;
	int len;

	hq = &video->hello_q;

	pkt = pkt_new(CMD_VIDEO_HELLO, sizeof(net_vedio_hello_t), TP_USER);
	p = get_pkt_payload(pkt, net_vedio_hello_t);
	PKT_HANDLE(pkt) = hq->handle;

	p->sn = ntoh_ll(video->slave->sn);
	p->callback_id = hq->callback_id;
	p->callback_ip = hq->callback_ip;
	p->callback_port = hq->callback_port;
	p->client_id = 0;
	p->ack = 0;
	p->client_type = cl_priv->cleint_type;
	p->auth_len = 0;


	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(hq->dst_ip);
	addr.sin_port = htons(hq->dst_port);
	len = sizeof(addr);
	
	if ((len = (int)sendto(video->vtry.sock_udp, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len)) != pkt->total) {
		log_err(true, "video_reply_hello_req 发送UDP数据失败, ip=%u.%u.%u.%u, port=%u, len=%d",
			IP_SHOW(hq->dst_ip), hq->dst_port, len);
	}
#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, pkt->total);
#endif

	pkt_free(pkt);
}

int timer_video_hello(cl_thread_t *t)
{
	video_t *video = (video_t *)CL_THREAD_ARG(t);

	video->t_hello = NULL;
	video->hello_req_count++;

	if (video->hello_req_count < HELLO_REQ_COUNT && video->status == VS_VIDEO_REQ) {
		video_reply_hello_req(video);
		CL_THREAD_TIMER_ON(&cl_priv->master, video->t_hello, timer_video_hello, video, TIME_HELLO_REQ);
	}

	return 0;
}

bool video_proc_hello_req(video_t *video, pkt_t *pkt)
{
	net_vedio_hello_q_t *req;
	hello_q_t *hq;

	req = get_pkt_payload(pkt, net_vedio_hello_q_t);
	
	hq = &video->hello_q;
	memset(hq, 0, sizeof(hello_q_t));
	hq->handle = PKT_HANDLE(pkt);
	hq->callback_id = req->callback_id;
	hq->callback_ip = req->callback_ip;
	hq->callback_port = req->callback_port;
	hq->dst_ip = ntohl(req->dst_ip);
	hq->dst_port = ntohs(req->dst_port);

	video_reply_hello_req(video);
	video->hello_req_count = 0;
	CL_THREAD_OFF(video->t_hello);
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_hello, timer_video_hello, video, TIME_HELLO_REQ);
	
	return true;
}

void video_on_tcp_connect(video_t *video)
{
	log_info("%s connect tcp ok\n", video->slave->str_sn);

	video->vtry.is_connect_ok = true;
	video_send_hello(video);
}

void video_set_last_error(video_t *video, const char *err)
{
	SAFE_FREE(video->last_err);
	video->last_err = cl_strdup(err);
}

RS video_set_stat_err(video_t *video, const char *err)
{
	if (video->stat.reson[0] != '\0' || video->status != VS_ESTABLISH)
		return RS_ERROR;

	strncpy(video->stat.reson, err, sizeof(video->stat.reson));
	video->stat.reson[sizeof(video->stat.reson) - 1] = '\0';

	return RS_OK;
}

void video_update_die(video_t *video)
{
	vt_update_die(&video->vtry);
}

void video_update_stat_pos(video_t *video)
{
	int i;
	u_int32_t diff;
	time_t now;

	now = get_sec();
	if (video->stat.last_stat == 0) {
		video->stat.first_stat = video->stat.last_stat = now;
		return;
	}
	
	if ((diff = (u_int32_t)now - (u_int32_t)video->stat.last_stat) == 0)
		return;

	for (i = 1; i <= (int)diff && i <= NUM_STAT_POOL; i++) {
		video->stat.sec_bytes[(video->stat.sec_pos + i)%NUM_STAT_POOL] = 0;
		video->stat.sec_frames[(video->stat.sec_pos + i)%NUM_STAT_POOL] = 0;
	}
	
	video->stat.sec_pos = (video->stat.sec_pos + diff)%NUM_STAT_POOL;
	video->stat.last_stat = now;
}

void video_update_stat_bytes(video_t *video, int bytes)
{
	video_update_stat_pos(video);
	video->stat.sec_bytes[video->stat.sec_pos] += bytes;

	video->stat.bytes += bytes;
}

void video_update_stat_frames(video_t *video)
{
	video_update_stat_pos(video);
	video->stat.sec_frames[video->stat.sec_pos]++;

	video->stat.frames++;
}

static void video_saturation_quick_query(video_t *video,bool force_stop);

void video_cal_sec_stat(video_t *video)
{
	int i, sec;
	u_int32_t bytes, frames;

	video_update_stat_pos(video);
	if (video->stat.first_stat == video->stat.last_stat)
		return;

	for (i = 0, bytes = 0, frames = 0; i < NUM_STAT_POOL; i++) {
		// 最后一秒的不要计算
		if (i == video->stat.sec_pos)
			continue;
		
		bytes += video->stat.sec_bytes[i];
		frames += video->stat.sec_frames[i];
	}

	sec = min(NUM_STAT_POOL - 1, (u_int32_t)video->stat.first_stat - (u_int32_t)video->stat.last_stat);
	video->stat.sec_rate = bytes*8/sec;
	video->stat.sec_fps = frames/sec;
    //检查是否需要获取视频参数
    video_saturation_quick_query(video,false);
}

int video_timer(cl_thread_t *t)
{
	video_t *video = (video_t *)CL_THREAD_ARG(t);

	log_debug("video %s timer, is_open=%d\n", video->slave->str_sn, video->is_open);

	video->t_timer = NULL;
	if ( ! video->is_open )
		return 0;
	
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_timer, video_timer, (void *)video, 1000);

	switch (video->status) {
	case VS_IDLE: 
		// 如果是第一个，发起请求。因为HelloReq没有handle，无法区分
		if (video_lookup_by_status(video->slave->user, VS_VIDEO_REQ) == NULL) {
			video_get_static_pic(video);
			video_send_syn_q(video, video->cur_vtap_client_id);
		}
		break;
		
	case VS_VTAP: /* 检查是否告下一个 */
//		cl_assert(0);
		break;
		
	case VS_ESTABLISH:
	case VS_VIDEO_REQ:
	case VS_SYN_A_RCV:
	case VS_ERROR:
		if (vt_check_timer(&video->vtry)) {
			video_retry(video);
		} else if (video->status == VS_SYN_A_RCV) {
			video_send_hello(video);
		}
		break;
	}

	return 0;
}

static void video_try_next_method(video_t *video)
{
	video_try_one_t *op = vt_get_this_try(&video->vtry);

	op->begin = get_msec();
	
	log_info("%u: %s video_try_next_method, proto=%s, ip=%u.%u.%u.%u, port=%u, timeout=%u\n",
		get_msec(), video->slave->str_sn, op->is_tcp ? "TCP" : "UDP", IP_SHOW(op->ip), op->port, op->timeout);
	
	if (op->ip == 0 || op->port == 0) {
		// IP地址和端口为0的，纯粹是拿来做延时的
		log_debug("%s delay %u ms\n", video->slave->str_sn, op->timeout);
		return;
	}

	video->stat.retry++;
	if (video->tcp_buff == NULL)
		video->tcp_buff = bf_alloc(DFL_VIDEO_TCP_BUFF_SIZE);

	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_timer, video_timer, (void *)video, 1000);

	video->last_recv_ip = op->ip;
	video->last_recv_port = op->port;
	
	if (op->is_tcp) { /* TCP, 相对复杂些 */
		bool inprogress;
		bf_reset(video->tcp_buff);

		video->vtry.sock_tcp = connect_tcp(op->ip, op->port, &inprogress);
		video->connect_type = C_TCP;

		if (VALID_SOCK(video->vtry.sock_tcp)) {
			//CL_THREAD_OFF(user->t_read_udp);
			CL_THREAD_READ_ON(&cl_priv->master, video->t_tcp_read, video_tcp_read, (void *)video, video->vtry.sock_tcp);

			if ( inprogress ) {
				CL_THREAD_WRITE_ON(&cl_priv->master, video->t_tcp_write, video_tcp_write, (void *)video, video->vtry.sock_tcp);
			} else {
				video_on_tcp_connect(video);
			}	
		} else {
			// 通过强制超时来触发下一个尝试
			op->begin = op->begin - op->timeout - 1;
		}
	} else { /* UDP */
		if (video->vtry.is_agent)
			video->connect_type = C_AGENT;
		else 
			video->connect_type = C_UDP;

		video_send_hello(video);
	}
}

void video_reset_data(video_t *video)
{
	video_off_all_thread(video, false);
	log_debug("%s open retry, m_nStatus=%d\n", video->slave->str_sn, video->status);

	video->is_recoding = false;
	video->is_open_success = false;

	pr_reset(video->pic_ring);
	pr_reset(video->sound_ring);
	
	// TCP肯定要删除。UDP可能需要保留
	CLOSE_SOCK(video->vtry.sock_tcp);
	video->vtry.is_connect_ok = false;

	if (video->buf_sound_recv_raw != NULL)
		bf_reset(video->buf_sound_recv_raw);
	if (video->buf_sound_send_remain != NULL)
		bf_reset(video->buf_sound_send_remain);

	video->	timestamp_base = (get_sec()*100 + get_msec()/10);
}

/* 等待一定毫秒数后重新开始 */
void video_restart(video_t *video, int delay)
{
	memset(&video->m_Syna, 0, sizeof(video->m_Syna));
	video_reset_data(video);    
	vt_reset(&video->vtry, true);
	vt_add_timeout(&video->vtry, delay);
	video_set_status(video, VS_ERROR);
}

video_t *video_lookup_by_status(user_t *user, int status)
{
	slave_t *slave;
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->video->status == status)
			return slave->video;
	}

	return NULL;
}


void ResetAudio(video_t *video)
{
	log_err(false, "请填写代码  ResetAudio()\n");
}

/*
	每一种状态的超时或者失败，都可以调用到这里
*/
void video_retry(video_t *video)
{
	log_info("%u: %s Retry Next Step, status=%u now\n", get_msec(), video->slave->str_sn, video->status);
	
	video_reset_data(video);

	// 这段代码暂时还只考虑看视频的
    // 看录像暂时不处理
	switch (video->status) {
	case VS_IDLE: /* 原理上这个状态不会调用过来 */
		cl_assert(0);
		break;
	case VS_VTAP:
		break;
	case VS_VIDEO_REQ:
		vt_reset(&video->vtry, true);
		vt_add_timeout(&video->vtry, TIME_VIDEO_NEXT_TRY);
		video_set_status(video, VS_ERROR);
		break;
	case VS_SYN_A_RCV:
		if (vt_move_next_try(&video->vtry)) { /*  尝试下一个连接方式 */
			video_try_next_method(video);
		} else { /* 所有方法都尝试完毕了 */
			video_send_stop(video);
			//SAFE_DEL(m_sockServer);
			//SAFE_DEL(m_sockForward);
			
			if (video->vtry.is_agent) { /* 代理都尝试失败，等待下一次尝试 */
				// 处理同 "case VS_ESTABLISH:"
				vt_reset(&video->vtry, true);
				video_set_status(video, VS_ERROR);
			} else { /* 尝试代理 */
				vt_reset(&video->vtry, false);
				video->vtry.is_agent = true;
				video_set_status(video, VS_IDLE);
			}
			vt_add_timeout(&video->vtry, TIME_VIDEO_NEXT_TRY);
		}
		break;
	case VS_ESTABLISH:
		video_send_stop(video);
		//SAFE_DEL(m_sockServer);
		//SAFE_DEL(m_sockForward);
		
		vt_reset(&video->vtry, true);
		vt_add_timeout(&video->vtry, TIME_VIDEO_NEXT_TRY);
		ResetAudio(video);
		video_set_status(video, VS_ERROR);
		break;
	case VS_ERROR:
		vt_reset(&video->vtry, true);
		video_set_status(video, VS_IDLE);
		break;
	}

	CL_THREAD_READ_ON(&cl_priv->master, video->t_udp, video_udp_recv, video, video->vtry.sock_udp);
}
	

bool video_proc_sonix(video_t *video, pkt_t *pkt)
{
	sonix_opt_t *so;

	so = get_pkt_payload(pkt, sonix_opt_t);

	log_err(false, "video %s recv sonix cmd: type=%u, cmd=%u, flip=%u, detect_enabled=%u, detect=%u\n",
		video->slave->str_sn, so->opt_type, so->cmd, so->flip, so->detect_enabled, so->detect);

	return true;
}

bool video_proc_qv2(video_t *video, pkt_t *pkt)
{
	video_quality_v2_t *vq;

	vq = get_pkt_payload(pkt, video_quality_v2_t);
	if (video->is_custom_quality != vq->is_custom || video->quality != vq->v1_hd.q) {
		log_info("video %s modify custom from %u to %u, qulity from %u to %u\n", 
			video->slave->str_sn, video->is_custom_quality, vq->is_custom,
			video->quality, vq->v1_hd.q);
	}
	video->is_custom_quality = vq->is_custom;
	video->quality = vq->v1_hd.q;

	return true;
}

bool video_do_syn_a(video_t *video, pkt_t *pkt)
{
	net_video_syn_a_t *p;
 
	if (video->status != VS_VIDEO_REQ) {
	 	log_info("Ignore VideoSynA, video %s status = %d\n", video->slave->str_sn, video->status);
		return false;
	}

	p = get_pkt_payload(pkt, net_video_syn_a_t);
	if (p->err_number) {
		video_set_last_error(video, err_2_str(p->err_number));
		video_retry(video);
		return false;
	}

	memcpy(&video->m_Syna, p, sizeof(net_video_syn_a_t));
	
	video->m_Syna.local_ip = ntohl(p->local_ip);
	video->m_Syna.global_ip = ntohl(p->global_ip);
	video->m_Syna.local_port = ntohs(p->local_port);
	video->m_Syna.global_port = ntohs(p->global_port);
	
	log_debug("video_do_syn_a: local_ip=%u.%u.%u.%u, local_port=%u, global_ip=%u.%u.%u.%u, global_port=%u, client id=%u, encrypt_method=%u\n",
		IP_SHOW(video->m_Syna.local_ip), video->m_Syna.local_port,
		IP_SHOW(video->m_Syna.global_ip), video->m_Syna.global_port,
		video->m_Syna.client_id, video->m_Syna.encrypt_method);

	video_set_status(video, VS_SYN_A_RCV);

	if (video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264
		|| video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264_AUDIO_ONEWAY
		|| video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264_AUDIO_TWOWAY) {
		video->is_h264 = true;
	} else {
		video->is_h264 = false;
	}

	if (video->m_Syna.encrypt_method == ENC_FRAG_AES128
		|| video->m_Syna.encrypt_method == ENC_FRAG_AES128_UDP_PTZ
		|| video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264
		|| video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264_AUDIO_ONEWAY
		|| video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264_AUDIO_TWOWAY)
	{
		video->is_aes_enc = true;
		if(video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264_AUDIO_ONEWAY)
		{
			video->has_audio = AUDIO_ONEWAY;
		}
		if(video->m_Syna.encrypt_method == ENC_FRAG_AES128_H264_AUDIO_TWOWAY)
		{
			video->has_audio = AUDIO_TWOWAY;
		}
		dec_block(video->m_Syna.key_rand, sizeof(video->m_Syna.key_rand), video->slave->user->passwd_md5);
		log_info("video encypt method = 0x%02x, has_audio=%d\n", video->m_Syna.encrypt_method, video->has_audio);
	} else {
		video->is_aes_enc = false;
	}

	// 按优先级，组织尝试队列
	vt_reset_try(&video->vtry);

	if ( ( ! video->vtry.is_agent) && video->slect_p2p >= 0) { /* p2p探测成功 */
		net_detect_t *nd;

		// 添加p2p的
		stlc_list_by_index(net_detect_t, nd, &video->net_detect_list, link, video->slect_p2p);
		vt_add_try(&video->vtry, true, nd->m_nIp, nd->m_nPort + video->m_Syna.client_id - 1, TIME_VIDEO_P2P_TCP_CONN_TIMEOUT);
		vt_add_try(&video->vtry, false, nd->m_nIp, nd->m_nPort, TIME_VIDEO_P2P_UDP_CONN_TIMEOUT);

		// 添加应答中返回的
		if (video->m_Syna.local_ip != 0 
			&& video->m_Syna.local_port != 0
			&& video->m_Syna.local_ip != nd->m_nIp)
		{
			vt_add_try(&video->vtry, false, video->m_Syna.local_ip, video->m_Syna.local_port, TIME_VIDEO_LAN_UDP_CONN_TIMEOUT);
		}
		if (video->m_Syna.global_ip != 0
			&& video->m_Syna.global_port != 0
			&& video->m_Syna.global_ip != nd->m_nIp 
			&& video->m_Syna.global_ip != video->m_Syna.local_ip)
		{
			vt_add_try(&video->vtry, false, video->m_Syna.global_ip, video->m_Syna.global_port, TIME_VIDEO_WAN_UDP_CONN_TIMEOUT);
		}
	} else if (( ! video->vtry.is_agent) && video->slave->user->m_bLanLoginOk) { /* 局域网直连登录 */
		vt_add_try(&video->vtry, true, video->m_Syna.local_ip, video->m_Syna.local_port + video->m_Syna.client_id - 1, TIME_VIDEO_P2P_TCP_CONN_TIMEOUT);
		if (video->m_Syna.local_ip != video->m_Syna.global_ip) {
			vt_add_try(&video->vtry, true, video->m_Syna.global_ip, video->m_Syna.global_port + video->m_Syna.client_id - 1, TIME_VIDEO_P2P_TCP_CONN_TIMEOUT);
		}
		vt_add_try(&video->vtry, false, video->m_Syna.local_ip, video->m_Syna.local_port, TIME_VIDEO_LAN_UDP_CONN_TIMEOUT);
		if (video->m_Syna.local_ip != video->m_Syna.global_ip) {
			vt_add_try(&video->vtry, false, video->m_Syna.global_ip, video->m_Syna.global_port, TIME_VIDEO_WAN_UDP_CONN_TIMEOUT);
		}
	} else { /* 服务器登录 */
		if (video->m_Syna.local_ip != 0 && video->m_Syna.local_port != 0) {
#ifdef TCP_AGENT
			vt_add_try(&video->vtry, true, video->m_Syna.local_ip, video->m_Syna.local_port, TIME_VIDEO_P2P_TCP_CONN_TIMEOUT);
#else
			vt_add_try(&video->vtry, false, video->m_Syna.local_ip, video->m_Syna.local_port, TIME_VIDEO_LAN_UDP_CONN_TIMEOUT);
#endif
		}
		if (video->m_Syna.global_ip != 0
			&& video->m_Syna.global_port != 0
			&& video->m_Syna.global_ip != video->m_Syna.local_ip)
		{
#ifdef TCP_AGENT
			vt_add_try(&video->vtry, true, video->m_Syna.global_ip, video->m_Syna.global_port, TIME_VIDEO_P2P_TCP_CONN_TIMEOUT);
#else
			vt_add_try(&video->vtry, false, video->m_Syna.global_ip, video->m_Syna.global_port, TIME_VIDEO_WAN_UDP_CONN_TIMEOUT);
#endif
		}
	}

	video_retry(video);

	return true;
}

static void video_get_static_pic(video_t *video)
{
	pkt_t *pkt;
	slave_t *slave = video->slave;

	log_debug("Try video_get_static_pic\n");
	
	pkt = pkt_new_v2(CMD_STATIC_PIC_Q_V2, sizeof(static_pic_q_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, slave->sn, TP_USER);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_STATIC_PIC_Q_V2, NULL, callback_video_request);
	
	user_add_pkt(slave->user, pkt);
}

static void video_show_static_pic(video_t *video)
{
	data_buf_t *db;

	if (video->static_pic == NULL)
		return;

	if (video->data_pic == NULL)
		video->data_pic = de_alloc(NUM_PIC_DB, video->static_pic_size, true);
	
	db = de_get_write(video->data_pic, video->static_pic_size);
	memcpy(db->data, video->static_pic, video->static_pic_size);
	db->data_size = video->static_pic_size;
	de_move_write(video->data_pic);

	log_debug("Show static pic: type=%d data size=%d\n",
		video->static_pic_type, video->static_pic_size);
	
	event_push(video->callback, VE_GET_PICTURE, video->handle, video->callback_handle);
}

static void video_proc_static_pic_a(video_t *video, pkt_t *pkt)
{
	int pic_len;
	static_pic_a_t *a;
	
	if (net_param_len(pkt->data) <= sizeof(static_pic_a_t)) {
		log_err(false, "bad static pic a: total=%d\n", pkt->total);
		return;
	}

	a = get_pkt_payload(pkt, static_pic_a_t);

	pic_len = net_param_len(pkt->data) - sizeof(static_pic_a_t);
	dec_block(a->pic_data, pic_len, video->slave->user->passwd_md5);
	/* 原始数据格式: 0x11223344 jpg-data pad */
	pic_len = pic_len - 4 - a->pad_len;
	SAFE_FREE(video->static_pic);
	video->static_pic_size = pic_len;
	video->static_pic_type = a->pic_type;
	video->static_pic = cl_malloc(video->static_pic_size);
	memcpy(video->static_pic, &a->pic_data[4], video->static_pic_size);

	video_show_static_pic(video);
}

bool video_open(video_t *video, cl_notify_pkt_t *pkt)
{
	cln_video_t *v = (cln_video_t *)&pkt->data[0];
	
	if (video->is_open) {
		log_err(false, "video %s is already opened befor", video->slave->str_sn);
		return true;
	}

	video->callback = v->callback;
	video->callback_handle = v->callback_handle;
	
	video->is_open = true;

	memset(&video->stat, 0, sizeof(video->stat));
	video->vtry.video = video;
	SAFE_FREE(video->last_err);
	video_set_status(video, VS_IDLE);
	vt_reset(&video->vtry, true);
	
	// 如果是第一个，发起请求。因为HelloReq没有handle，无法区分
	if (video_lookup_by_status(video->slave->user, VS_VIDEO_REQ) == NULL) {
		video_get_static_pic(video);
		video_send_syn_q(video, 0);
	}

	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_timer, video_timer, (void *)video, 1000);

	video_show_static_pic(video);

	return true;
}

void video_close(video_t *video)
{
	if ( ! video->is_open ) {
		log_err(false, "video %s is already close befor", video->slave->str_sn);
		return;
	}
	
	video->is_open = false;

	if (video->is_h264) {
		stop_h264_decode(video);
		de_free(video->data_h264);
		video->data_h264 = NULL;
        video->is_h264 = false;
	}
	de_free(video->data_pic);
	video->data_pic = NULL;
    
    CL_THREAD_TIMER_OFF(video->t_vs_timer);

	pr_free(video->sound_ring);
	video->sound_ring = NULL;

	bf_free(video->buf_sound_recv_raw);
	video->buf_sound_recv_raw = NULL;

	bf_free(video->buf_sound_send_remain);
	video->buf_sound_send_remain = NULL;
	
	video_off_all_thread(video, false);

	if (video->status >= VS_SYN_A_RCV  && video->m_Syna.client_id > 0) {
		video_send_stop(video);
	}

	video_set_status(video, VS_IDLE);
	vt_reset(&video->vtry, true);
	
	memset(&video->m_Syna, 0, sizeof(net_video_syn_a_t));
	pr_reset(video->pic_ring);
	
	ResetAudio(video);
}

void video_free_tcp_send(video_t *video)
{
	video->tcp_send_len = 0;
	stlc_list_free(pkt_t, &video->tcp_pkt_list, link);
}

void video_off_all_thread(video_t *video, bool off_net_detect)
{
	if (off_net_detect)
		CL_THREAD_OFF(video->t_net_detect);
	CL_THREAD_OFF(video->t_hello);
	CL_THREAD_OFF(video->t_tcp_read);
	CL_THREAD_OFF(video->t_tcp_write);
	CL_THREAD_OFF(video->t_udp);
	CL_THREAD_OFF(video->t_query);
    CL_THREAD_TIMER_OFF(video->t_vs_timer);
	if ( ! video->is_open )
		CL_THREAD_OFF(video->t_timer);
	CL_THREAD_OFF(video->t_roll);

	video_free_tcp_send(video);
}


int timer_video_roll(cl_thread_t *t)
{
	video_t *video = (video_t *)CL_THREAD_ARG(t);
	pkt_t *pkt;
	net_video_roll_t *vr;

	video->t_roll = NULL;
	if (video->status != VS_ESTABLISH) {
		log_err(false, "video %s roll timer failed: status=%d", video->slave->str_sn, video->status);
		return 0;
	}

	pkt = pkt_new_v2(CMD_VIDEO_ROLL, sizeof(net_video_roll_t), NHF_TRANSPARENT, video->slave->sn, TP_USER);
	vr = get_pkt_payload(pkt, net_video_roll_t);
	vr->left_right = video->roll_left_right;
	vr->up_down = video->roll_up_down;
	video_send(video, pkt);

	video->num_roll_send++;
	if (video->roll_type == ROLL_TYPE_ONCE) {
		if (video->num_roll_send < ROLL_SEND_COUNT) {
			CL_THREAD_TIMER_ON(&cl_priv->master, video->t_roll, timer_video_roll, video, TIME_ROLL);
		}
	} else if (video->roll_type == ROLL_TYPE_START) {
		if ( (video->num_roll_send * TIME_ROLL) < TIME_N_SECOND(60) ) {
			CL_THREAD_TIMER_ON(&cl_priv->master, video->t_roll, timer_video_roll, video, TIME_ROLL);
		}
	}
	
	return 0;
}

static bool do_video_roll(video_t *video, cl_notify_pkt_t *pkt)
{
	cln_video_t *v = (cln_video_t *)&pkt->data[0];
	
	if ( ! video->is_open) {
		log_err(false, "video %s roll failed: not open", video->slave->str_sn);
		return false;
	}
	if (video->status != VS_ESTABLISH) {
		log_err(false, "video %s roll failed: status=%d", video->slave->str_sn, video->status);
		return false;
	}

	if (v->u.roll.roll_type == ROLL_TYPE_STOP) {
		CL_THREAD_OFF(video->t_roll);
		return true;
	} else if (v->u.roll.roll_type == ROLL_TYPE_ONCE) {
	} else if (v->u.roll.roll_type == ROLL_TYPE_START) {
	} else {
		log_err(false, "video %s roll failed: invalid roll type %d", video->slave->str_sn, v->u.roll.roll_type);
		return false;
	}

	video->num_roll_send = 0;
	video->roll_type = v->u.roll.roll_type;
	video->roll_left_right = v->u.roll.left_right;
	video->roll_up_down = v->u.roll.up_down;
	
	CL_THREAD_OFF(video->t_roll);
	// 第一个快速发
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_roll, timer_video_roll, video, 1);

	return true;
}

int cal_qulity(cl_video_quality_t *vq)
{
	int q = 0;
	
	if (vq->width == 0 || vq->height == 0 || vq->fps == 0 || vq->quality != 0)
		return vq->quality;
	
	if (vq->width == 320) {
		q = 1;
	} else if (vq->width == 640) {
		q = 2;
	} else if (vq->width == 1280) {
		q = 3;
	}

	return q;
}

static bool do_video_qulity(video_t *video, cl_notify_pkt_t *cln_pkt)
{
	cln_video_t *v = (cln_video_t *)&cln_pkt->data[0];
	pkt_t *pkt;
	
	if ( ! video->is_open) {
		log_err(false, "video %s set qulity failed: not open", video->slave->str_sn);
		return false;
	}
	/*
	if (video->status != VS_ESTABLISH) {
		log_err(false, "video %s set qulity failed: status=%d", video->slave->str_sn, video->status);
		return false;
	}
	*/

	if (video->slave->has_video_custom && video->is_h264) {
		video_quality_v2_t *vq;
		
		pkt = pkt_new_v2(CMD_VIDEO_QUALITY_V2, sizeof(video_quality_v2_t), NHF_TRANSPARENT, video->slave->sn, TP_USER);
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VIDEO_QUALITY_V2, NULL, callback_video_request);
		
		vq = get_pkt_payload(pkt, video_quality_v2_t);
		vq->action = VIDEO_CUSTOM_WRITE;
		vq->v1_hd.q = cal_qulity(&v->u.qulity);
		vq->is_custom = (v->u.qulity.width != 0);
		vq->fps = (u_int8_t)v->u.qulity.fps;
		vq->width = htons(v->u.qulity.width);
		vq->height = htons(v->u.qulity.height);
		vq->errorno = 0;

		video->is_custom_quality = vq->is_custom;
		video->quality = vq->v1_hd.q;
	} else {
		u_int8_t *p;
		
		pkt = pkt_new_v2(CMD_VIDEO_QUALITY, 1, NHF_TRANSPARENT, video->slave->sn, TP_USER);
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VIDEO_QUALITY, NULL, callback_video_request);
		p = get_pkt_payload(pkt, u_int8_t);
		*p = cal_qulity(&v->u.qulity);
		
		video->quality = *p;
	}
	
	user_add_pkt(video->slave->user, pkt);
	vrt_quick_query(video);

	return true;
}

static bool do_video_flip(video_t *video, cl_notify_pkt_t *cln_pkt)
{
//	cln_video_t *v = (cln_video_t *)&cln_pkt->data[0];
	pkt_t *pkt;
	sonix_opt_t *flip;
	
	if ( ! video->is_open) {
		log_err(false, "video %s flip failed: not open", video->slave->str_sn);
		return false;
	}

	pkt = pkt_new_v2(CMD_VIDEO_SONIX, sizeof(sonix_opt_t), NHF_TRANSPARENT, video->slave->sn, TP_USER);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VIDEO_SONIX, NULL, callback_video_request);
	
	flip = get_pkt_payload(pkt, sonix_opt_t);
	flip->opt_type = KOP_WRITE;
	flip->cmd = CMD_SONIX_FLIP;
	flip->flip = 1;
	
	user_add_pkt(video->slave->user, pkt);

	return true;
}
/////////////////////////////////////////////////////////////////////////
// 视频云台旋转相关
static void video_proc_roll_ctrl_packet(video_t *video, pkt_t *pkt)
{
    net_moto_attri_t* nma;
    
	nma = get_pkt_payload(pkt, net_moto_attri_t);
    if (!nma||!video) {
        return;
    }
    //目前只有两个action，就直接处理了
    if (nma->err!=ERR_NONE) {
        if (nma->action == e_MOTO_ATTRI_CTRL) {
            //TODO 发送设置失败消息
            event_push(video->callback, VE_SET_ROLL_SPEED_FAILED, video->handle, video->callback_handle);
        }
        return;
    }
    
    if (nma->action == e_MOTO_ATTRI_CTRL) {
        //TODO 通知控制成功
        event_push(video->callback, VE_SET_ROLL_SPEED_OK, video->handle, video->callback_handle);
    }else if (nma->action == e_MOTO_ATTRI_QUERY){
        video->stat.roll_speed = nma->roll_speed;
    }
}

static bool do_video_set_roll_ctrl(video_t *video, cl_notify_pkt_t *cln_pkt)
{
    cln_video_t *v = (cln_video_t *)&cln_pkt->data[0];
	pkt_t *pkt;
    slave_t* slave;
    net_moto_attri_t* nma;
	user_t *user;
    
    slave = video->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_MOTO_ATTRIBUTE, sizeof(net_moto_attri_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_MOTO_ATTRIBUTE, (void*)e_MOTO_ATTRI_CTRL, callback_video_request);
    nma = get_pkt_payload(pkt, net_moto_attri_t);
    nma->action = e_MOTO_ATTRI_CTRL;
    nma->roll_speed = v->u.roll_speed;
	user_add_pkt(user, pkt);
    //强制刷新
    video_saturation_quick_query(video,true);
    
    return true;
}

/////////////////////////////////////////////////////////////////////////
// 视频饱和度相关
static int video_saturation_query_timer(cl_thread_t *t)
{
    cl_handle_t slave_handle;
    slave_t* slave;
    video_t* video;
    net_v4l2_color_t* nc;
    net_moto_attri_t* nma;
    pkt_t *pkt;
	user_t *user;
    u_int32_t now;

    
    slave_handle = (cl_handle_t)CL_THREAD_ARG(t);
    slave = lookup_by_handle(HDLT_SLAVE, slave_handle);
    if (!slave || !slave->video) {
        return ERR_PARAM_INVALID;
    }
    
    user = slave->user;
    video = slave->video;
    video->t_vs_timer = NULL;
    
    //离线就算了
    if (slave->status != BMS_BIND_ONLINE) {
        return ERR_PARAM_INVALID;
    }
    
    //如果支持饱和度，发送饱和度报文
    if (video->slave->has_v4l2_color_setting && !is_user_in_background_mode(user)) {
        pkt = pkt_new_v2(CMD_V4L2_COLOR, sizeof(net_v4l2_color_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                         slave->sn, user->ds_type);
        PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_V4L2_COLOR, NULL, callback_video_request);
        nc = get_pkt_payload(pkt, net_v4l2_color_t);
        nc->action = e_V4L2_QUERY;
        user_add_pkt(user, pkt);
    }
    //视频云台控制，发送云台控制查询报文
    if (video->slave->has_roll_speed_ctrl && !is_user_in_background_mode(user)) {
        pkt = pkt_new_v2(CMD_MOTO_ATTRIBUTE, sizeof(net_moto_attri_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                         slave->sn, user->ds_type);
        PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_MOTO_ATTRIBUTE, NULL, callback_video_request);
        nma = get_pkt_payload(pkt, net_moto_attri_t);
        nma->action = e_MOTO_ATTRI_QUERY;
        user_add_pkt(user, pkt);
    }
    
    //未超时启动下次探测
    now = get_sec();
    if (now - video->stat.last_stat < MAX_VIDEO_SATURTAION_INTERVAL) {
        CL_THREAD_TIMER_ON(&cl_priv->master, video->t_vs_timer , video_saturation_query_timer, (void*)video->slave->handle, MAX_VIDEO_SATURTAION_TIMER);
    }
    
    return 0;
}

static bool video_support_one_function_for_query(video_t* video)
{
    return video?(video->slave->has_v4l2_color_setting
                  || video->slave->has_roll_speed_ctrl):false;
}

//立刻查询一次
static void video_saturation_quick_query(video_t *video,bool force_stop)
{
    //不支持就算了
    if (!video||!video->slave|| !video_support_one_function_for_query(video) || (!force_stop && video->t_vs_timer)) {
        return;
    }
    CL_THREAD_TIMER_OFF(video->t_vs_timer);
    CL_THREAD_TIMER_ON(&cl_priv->master, video->t_vs_timer , video_saturation_query_timer, (void*)video->slave->handle, 0);
}

static void video_proc_v4l2_packet(video_t *video, pkt_t *pkt)
{
    net_v4l2_color_t* nc;
    
	nc = get_pkt_payload(pkt, net_v4l2_color_t);
    if (!nc||!video) {
        return;
    }
    //目前只有两个action，就直接处理了
    if (nc->err!=ERR_NONE) {
        if (nc->action == e_V4L2_CTRL) {
            //TODO 发送设置失败消息
            event_push(video->callback, VE_SET_V4L2_FAILED, video->handle, video->callback_handle);
        }
        return;
    }
    
    if (nc->action == e_V4L2_CTRL) {
        //TODO 通知控制成功
        event_push(video->callback, VE_SET_V4L2_OK, video->handle, video->callback_handle);
    }else if (nc->action == e_V4L2_QUERY){
        video->stat.brightness_val = (int32_t)ntohl(nc->brightness_val);
        video->stat.contrast_val = ntohl(nc->contrast_val);
        video->stat.saturation_val = ntohl(nc->saturation_val);
        video->stat.gain_val = ntohl(nc->gain_val);
    }
}

static bool do_video_set_v4l2(video_t *video, cl_notify_pkt_t *cln_pkt)
{
    cln_video_t *v = (cln_video_t *)&cln_pkt->data[0];
	pkt_t *pkt;
    slave_t* slave;
    net_v4l2_color_t* nc;
	user_t *user;
    
    slave = video->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_V4L2_COLOR, sizeof(net_v4l2_color_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_V4L2_COLOR, (void*)e_V4L2_CTRL, callback_video_request);
    nc = get_pkt_payload(pkt, net_v4l2_color_t);
    nc->action = e_V4L2_CTRL;
    nc->brightness_val = htonl((u_int32_t)v->u.vs.brightness_val);
    nc->contrast_val = htonl(v->u.vs.contrast_val);
    nc->saturation_val = htonl(v->u.vs.saturation_val);
    nc->gain_val = htonl(v->u.vs.gain_val);
	user_add_pkt(user, pkt);
    //查询一次
    video_saturation_quick_query(video,true);
    
    return true;
}

/////////////////////////////////////////////////////////////////////////
// 看远程录像相关,

//根据begintime，获取最适合的录像，vtap_list是升序排列
static vtap_record_item_t* lookup_record_by_begin_time(video_t *video,u_int32_t begin_time)
{
    vtap_record_item_t* rec,*tmp;
    stlc_list_for_each_entry(vtap_record_item_t, rec, &video->vtap_list, link){
        
        if ( begin_time >= rec->tap_info.begin_time &&
            begin_time <= (rec->tap_info.begin_time+rec->tap_info.duration-2)) {
            //设备端列表持续时间是300，但实际传回有可能小于此数，因此只能相信begin_time
            //要直到取到最后的一个准确的为止
            tmp = (vtap_record_item_t*)rec->link.next;
            while (&(tmp->link) != &video->vtap_list) {
                if (begin_time > tmp->tap_info.begin_time &&
                    begin_time <= (tmp->tap_info.begin_time+tmp->tap_info.duration-2)) {
                    rec = tmp;
                }else{
                    break;
                }
                tmp = (vtap_record_item_t*)tmp->link.next;
            }
            return rec;
        }
        // 如果是无效的begin，自动取下一个
        if (rec->tap_info.begin_time+rec->tap_info.duration > begin_time) {
            return rec;
        }
    }
    
    return NULL;
}

static bool do_vtap_query_list(video_t *video, cln_video_t* v)
{
    net_vtap_list_query_t* vlq;
	pkt_t *pkt;
    slave_t* slave;
	user_t *user;
    
    slave = video->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_VTAP_LIST_Q, sizeof(net_vtap_list_query_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VTAP_LIST_Q, (void*)NULL, callback_video_request);
    vlq = get_pkt_payload(pkt, net_vtap_list_query_t);
    vlq->begin_time = calc_begin_time_of_day(v->u.query.begin_time);
    //缓存开始和结束时间，回包时使用
    log_debug("vlq->begin_time %u\n",vlq->begin_time);
    video->cur_vtap_query_begin_time = vlq->begin_time;
    video->cur_vtap_query_end_time = vlq->begin_time + SECOND_OF_DAY;
    vlq->begin_time = htonl(vlq->begin_time);
    vlq->want_num = htons(v->u.query.want_num);
    
	user_add_pkt(user, pkt);
    
    stlc_list_free(vtap_record_item_t, &video->vtap_list, link);
    
    return true;
}

static bool do_vtap_request(video_t *video, u_int32_t begin_time, u_int16_t offset_sec)
{
    net_vtap_req_t* vrq;
	pkt_t *pkt;
    slave_t* slave;
	user_t *user;
    
    slave = video->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_VTAP_Q, sizeof(net_vtap_req_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VTAP_Q, (void*)NULL, callback_video_request);
    vrq = get_pkt_payload(pkt, net_vtap_req_t);
    vrq->begin_time = htonl(begin_time);
    vrq->duration = htons(offset_sec);
    vrq->vtap_id = 0x0;
    vrq->err = vrq->pad = 0x0;

	user_add_pkt(user, pkt);
    
    return true;
}

static bool time_in_current_record(video_t * video,vtap_record_item_t* vrec,u_int32_t time)
{
    if (vrec->tap_info.begin_time != video->cur_vtap_begin_time) {
        return false;
    }
    
    if (time >= video->cur_vtap_begin_time && time <= video->cur_vtap_begin_time+video->cur_vtap_duration) {
        return true;
    }
    
    return false;
}


static void vtap_add_list_to_local(video_t* video,net_vtap_list_t* vlist,u_int32_t* max_time)
{
    vtap_record_item_t* v_item;
    vtap_t* vtap;
    u_int32_t m_time=0;
    u_int32_t i;
    u_int32_t limit_time;
    
    if (max_time) {
        *max_time = 0;
    }
    if (!vlist || !video || vlist->num <= 0) {
        return;
    }
    //按时间过滤结果,暂时不缓存
    limit_time = video->cur_vtap_query_end_time;
    vtap = (vtap_t*)vlist->tap_list;
    for (i = 0; i<vlist->num; i++,vtap++) {
        if (limit_time>0 && ntohl(vtap->begin_time) > limit_time) {
            break;
        }
        v_item = cl_malloc(sizeof(vtap_record_item_t));
        if (!v_item) {
            break;
        }
        v_item->tap_info.begin_time = ntohl(vtap->begin_time);
        v_item->tap_info.duration = ntohs(vtap->duration);
        v_item->tap_info.video_format = vtap->video_format;
        v_item->tap_info.pad = vtap->pad;
        if (v_item->tap_info.begin_time > m_time) {
            m_time = v_item->tap_info.begin_time;
        }
        
        stlc_list_add_tail(&v_item->link, &video->vtap_list);
    }
    
    if (max_time) {
        *max_time = m_time;
    }
}

static void vtap_query_vtap_list(video_t* video,u_int32_t begin_time)
{
    cln_video_t cln_video = {0};
    cln_video.u.query.begin_time = begin_time;
    do_vtap_query_list(video,&cln_video);
}

//static int vtap_pre_query_event(cl_thread_t *t)
//{
//    u_int32_t handle = CL_THREAD_VAL(t);
//    slave_t* slave = lookup_by_handle(HDLT_SLAVE, handle);
//    video_t* video;
//    
//    if (!slave || !slave->video ) {
//        return 0;
//    }
//    
//    video = slave->video;
//    
//    video->is_pre_get_vtap = true;
//    vtap_query_vtap_list(video, 0);
//    
//    return 0;
//}

static void vtap_proc_query_list_answer(video_t *video, pkt_t *pkt)
{
    net_vtap_list_t* vack;
    user_t* user;
    u_int32_t max_time;
    
    video->is_pre_get_vtap = false;
    if (net_param_len(pkt->data) < sizeof(*vack)) {
        log_err(false, "%s video list answer packet length error[%u]", video->slave->str_sn,net_param_len(pkt->data));
        return;
    }
    user = video->slave->user;
	vack = get_pkt_payload(pkt, net_vtap_list_t);
    vack->err = ntohl(vack->err);
    vack->num = ntohl(vack->num);
    vack->seg_index = ntohs(vack->seg_index);
    vack->seg_num = ntohs(vack->seg_num);
    
    log_debug("query list packet err [%u] vtap_num[%u] seg_num[%u] seg_index[%u]\n",
              vack->err,vack->num,vack->seg_num,vack->seg_index);
    if (vack->err) {
        if (video->is_pre_get_vtap) {
            return;
        }
        if (vack->err == ERR_NO_VTAP) {
            event_push(user->callback, VE_VTAP_GET_LIST_OK,video->slave->handle,user->callback_handle);
        }else{
            event_push(user->callback, VE_VTAP_GET_LIST_FAILED,video->slave->handle,user->callback_handle);
        }
        return;
    }
    
    // 只支持按天获取录像
    vtap_add_list_to_local(video,vack,&max_time);
    
    if (video->cur_vtap_query_begin_time == 0) {
        //非预取、用户设置的开始时间为0，还有剩余的就取完
        if (!video->is_pre_get_vtap && vack->seg_num > 1 && max_time > 0) {
            vtap_query_vtap_list(video,max_time);
        }
        
        if (!video->is_pre_get_vtap && (vack->seg_num == 1 || max_time == 0)) {
            event_push(user->callback, VE_VTAP_GET_LIST_OK,video->slave->handle,user->callback_handle);
        }
    }else{
        event_push(user->callback, VE_VTAP_GET_LIST_OK,video->slave->handle,user->callback_handle);
    }
}

static bool vtap_real_open(video_t *video);

static void vtap_proc_req_answer(video_t *video, pkt_t *pkt)
{
    net_vtap_req_t* vreq;
    user_t* user;
    
    video->is_pre_get_vtap = false;
    if (net_param_len(pkt->data) < sizeof(*vreq)) {
        log_err(false, "%s video vtap_proc_req_answer packet length error[%u]", video->slave->str_sn,net_param_len(pkt->data));
        return;
    }
    user = video->slave->user;
	vreq = get_pkt_payload(pkt, net_vtap_req_t);
    vreq->err = ntohl(vreq->err);
    vreq->begin_time = ntohl(vreq->begin_time);
    vreq->duration = ntohs(vreq->duration);
    
    if (vreq->begin_time != video->cur_vtap_begin_time) {
        log_err(false, "%s video vtap_proc_req_answer answer time error[%u] except[%u]", video->slave->str_sn,
                vreq->begin_time,video->cur_vtap_begin_time);
        return;
    }
    
    video->cur_vtap_client_id = vreq->vtap_id;
    video->cur_vtap_duration = vreq->duration;
    
    log_debug("video id %u begin_time %u",video->cur_vtap_client_id,video->cur_vtap_begin_time);
    vtap_real_open(video);
}

static void vtap_request_offset(video_t* video,vtap_record_item_t* vrec,u_int32_t offset)
{
    net_vtap_timestap_t* vt;
	pkt_t *pkt;
    slave_t* slave;
	user_t *user;
    
    slave = video->slave;
    user = slave->user;
    
    pkt = pkt_new_v2(CMD_VTAP_TIMESTAP, sizeof(net_vtap_req_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     slave->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->slave->handle, CMD_VTAP_TIMESTAP, (void*)NULL, callback_video_request);
    vt = get_pkt_payload(pkt, net_vtap_timestap_t);
    offset*=100;//单位是10毫秒
    vt->timestap = htonl(offset);
    vt->vtap_id = video->cur_vtap_client_id;
    
	user_add_pkt(user, pkt);
}

static void vtap_proc_request_offset(video_t *video, pkt_t *pkt)
{
    net_vtap_timestap_t *vtta;
    user_t* user =  video->slave->user;
    
	vtta = get_pkt_payload(pkt, net_vtap_timestap_t);
    if (net_param_len(pkt->data) < sizeof(*vtta)) {
        log_err(false, "%s video vtap_proc_request_offset packet length error[%u]", video->slave->str_sn,
                net_param_len(pkt->data));
        return;
    }
    //报告失败就可以了
    if (vtta->err != ERR_NONE) {
        event_push(user->callback, VE_VTAP_SET_POS_FAILED,video->slave->handle,user->callback_handle);
    }
}

//真正开始录像协商
static bool vtap_real_open(video_t *video)
{
    if (!video || !video->cur_vtap_begin_time || !video->cur_vtap_client_id) {
        return true;
    }
    
	if (video_lookup_by_status(video->slave->user, VS_VIDEO_REQ) == NULL) {
		video_send_syn_q(video, video->cur_vtap_client_id);
	}
    
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_timer, video_timer, (void *)video, 1000);
    
	return true;
}

static bool vtap_open_file(video_t* video,vtap_record_item_t* vrec)
{
    video->is_open = true;
	
    memset(&video->stat, 0, sizeof(video->stat));
	video->vtry.video = video;
	SAFE_FREE(video->last_err);
	video_set_status(video, VS_IDLE);
	vt_reset(&video->vtry, true);
    
    //请求录像,以查找的为准
    video->is_vtap = true;
    video->cur_vtap_begin_time = vrec->tap_info.begin_time;
    video->cur_vtap_client_id = 0x0;
    video->cur_vtap_time_offset = 0x0;
    video->cur_vtap_duration = 0;
    
    do_vtap_request(video,vrec->tap_info.begin_time,vrec->tap_info.duration);
    video_set_status(video, VS_VTAP);
    
    return true;
}

static bool vtap_reopen(video_t* video,vtap_record_item_t* rec,u_int32_t want_time)
{
    if (video->is_open) {
        video_close(video);
    }
    //不能清除当前查询的数据
    video->cur_want_offset =
    video->cur_vtap_duration =
    video->cur_vtap_begin_time =
    video->cur_vtap_client_id = 0x0;
    video->cur_vtap_time_offset = 0x0;
    if (want_time > rec->tap_info.begin_time && want_time < rec->tap_info.begin_time+rec->tap_info.duration) {
        video->cur_want_offset = want_time-rec->tap_info.begin_time;
    }
    
    return vtap_open_file(video,rec);
}

static void vtap_proc_jpg_offset(video_t* video)
{
    vtap_record_item_t* vrec;
    
    if (!video->cur_vtap_begin_time || !video || !video->is_vtap ) {
        return;
    }
    
    log_debug("begin %08x du %u cur_off %u\n",video->cur_vtap_begin_time,video->cur_vtap_duration,video->cur_vtap_time_offset);
    // 处理等待跳转的问题
    if (video->cur_want_offset) {
        vrec = lookup_record_by_begin_time(video, video->cur_vtap_begin_time);
        if (vrec) {
            vtap_request_offset(video,vrec,video->cur_want_offset);
        }
        video->cur_want_offset = 0;
        return;
    }
    
    if(video->cur_vtap_duration > 0 && ((int)video->cur_vtap_duration - (int)(video->cur_vtap_time_offset/100.0))<=0x1)
    {
        log_debug("reopen want %u %u xxxxxxxxx******************************************\n",video->cur_vtap_begin_time,video->cur_vtap_duration);
        vrec = lookup_record_by_begin_time(video, video->cur_vtap_begin_time+video->cur_vtap_duration+10);
        if (vrec) {
            vtap_record_item_t* prev,*next;
            
            prev = (vtap_record_item_t*)vrec->link.prev;
            next  = (vtap_record_item_t*)vrec->link.next;
            
            log_debug("selected record %u %u \n",vrec->tap_info.begin_time,vrec->tap_info.duration);
            log_debug("prev record %u %u \n",prev->tap_info.begin_time,prev->tap_info.duration);
            log_debug("next record %u %u \n",next->tap_info.begin_time,next->tap_info.duration);
            
            if (video->cur_vtap_begin_time+video->cur_vtap_duration+3 > vrec->tap_info.begin_time) {
                vtap_reopen(video, vrec, video->cur_vtap_begin_time+video->cur_vtap_duration+3);
            }else{
                vtap_reopen(video, vrec, 0);
            }
            
        }
    }
}

//开始看录像，先请求
bool vtap_open(video_t *video, cl_notify_pkt_t *pkt, RS *ret)
{
    cln_video_t *v = (cln_video_t *)&pkt->data[0];
    vtap_record_item_t* vrec;
    u_int32_t t_time;
    
    if (v->u.vtap_req.begin_time == 0) {
        *ret = RS_INVALID_PARAM;
        return true;
    }
    
    //先转换为格林时间,检查在查询的范围内
    t_time = v->u.vtap_req.begin_time;
    if (t_time < video->cur_vtap_query_begin_time ||
        t_time > video->cur_vtap_query_end_time) {
        *ret = RS_INVALID_PARAM;
        return true;
    }
	//取出一个最近的
    vrec = lookup_record_by_begin_time(video, t_time);
    if (!vrec) {
        *ret = RS_NOT_FOUND;
        log_err(false, "vtap %s can not found time [%08x] ", video->slave->str_sn,v->u.vtap_req.begin_time);
        return true;
    }
    
    //如果是open的,重开？？
	if (video->is_open) {
        //当前正在播放
        if (time_in_current_record(video,vrec,t_time)) {
            vtap_request_offset(video,vrec,t_time-vrec->tap_info.begin_time);
            return true;
        }
        //需要跳到另一个文件
		return vtap_reopen(video, vrec,t_time);
	}
    
    video->callback = v->callback;
	video->callback_handle = v->callback_handle;
    
//    return vtap_open_file(video,vrec);
    return vtap_reopen(video, vrec, t_time);
}

void vtap_close(video_t *video)
{
	if ( ! video->is_open ) {
		log_err(false, "video %s is already close befor", video->slave->str_sn);
		return;
	}
	
    video->cur_want_offset =
    video->cur_vtap_duration =
    video->cur_vtap_begin_time =
    video->cur_vtap_client_id = 0x0;
    video->cur_vtap_time_offset = 0x0;
    video->is_vtap = false;
    
	video_close(video);
}

/////////////////////////////////////////////////////////////////////////

// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
bool video_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
//	u_int32_t err_code = 0;
	slave_t *slave;
	video_t *video;
	wait_t *w;

	w = wait_lookup(PKT_HANDLE(pkt));
	if (w == NULL || w->obj_type != HDLT_SLAVE) {
		return false;
	}
	
	slave = slave_lookup_by_handle(user, w->obj_handle);
	if (slave == NULL) {
		log_debug("video ignore cmd=%u, not found slave by handle=0x%08x\n", hdr->command, PKT_HANDLE(pkt));
		return false;
	}

	video = slave->video;
	log_debug("video_proc_tcp sn=%s, cmd=%u, handle=0x%08x\n", slave->str_sn, hdr->command, PKT_HANDLE(pkt));

	switch (hdr->command) {
	case CMD_VIDEO_SYN_A:
		video_do_syn_a(video, pkt);
		break;

	case CMD_VIDEO_HELLO_REQ:
		video_proc_hello_req(video, pkt);
		break;

	case CMD_VIDEO_SONIX:
		video_proc_sonix(video, pkt);
		break;

	case CMD_SPEEK_A:
		audio_proc_speek_a(video, pkt);
		break;
		
	case CMD_REC_TIMER_A:
		vrt_do_timer_a(video, pkt);
		break;

	case CMD_STATIC_PIC_A:
	case CMD_STATIC_PIC_A_V2:
		video_proc_static_pic_a(video, pkt);
		break;

	case CMD_VIDEO_QUALITY_V2:
		video_proc_qv2(video, pkt);
		break;
    case CMD_V4L2_COLOR:
        video_proc_v4l2_packet(video,pkt);
        break;
    case CMD_MOTO_ATTRIBUTE:
        video_proc_roll_ctrl_packet(video, pkt);
        break;
    case CMD_VTAP_LIST_A:
        vtap_proc_query_list_answer(video, pkt);
        break;
    case CMD_VTAP_A:
        vtap_proc_req_answer(video, pkt);
        break;
    case CMD_VTAP_TIMESTAP:
        vtap_proc_request_offset(video,pkt);
        break;

/* 录像相关
	case CMD_VTAP_TIMESTAP:
		DoVtapTimestampA();
		break;
	case CMD_VIDEO_CONTROL_ALARM_CFG:
		DoMsgPhone();
		break;
	case CMD_ALARM_CONFIG_PHONE:
		DoAlarmPhone();
		break;
*/
	
	default:
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////

bool video_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	cln_video_t *cv;
	slave_t *slave;
	video_t *video;

	if (pkt->type < CLNE_VIDEO_START || pkt->type > CLNE_VIDEO_MAX) {
		return false;
	}
	
	cv = (cln_video_t *)&pkt->data[0];
	slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, cv->slave_halde);
	if (slave == NULL) {
		video = (video_t *)lookup_by_handle(HDLT_VIDEO, cv->slave_halde);
		if (video == NULL) {
			log_err(false, "video_proc_notify failed: not found slave handle=0x%08x\n", cv->slave_halde);
			*ret = RS_NOT_FOUND;
			return true;
		}
	} else {
		video = slave->video;
	}
	
	switch (pkt->type) {
	case CLNE_VIDEO_START:
		video_open(video, pkt);
		*ret = RS_OK;
		break;

	case CLNE_VIDEO_STOP:
        video_close(video);
		*ret = RS_OK;
		break;

	case CLNE_VIDEO_DECODE_H264:
		event_push(video->callback, VE_GET_PICTURE, video->handle, video->callback_handle);
		break;

	case CLNE_VIDEO_ROLL:
		do_video_roll(video, pkt);
		break;

	case CLNE_VIDEO_QULITY:
		do_video_qulity(video, pkt);
		break;

	case CLNE_VIDEO_FLIP:
		do_video_flip(video, pkt);
		break;

	case CLNE_VIDEO_CAL_STAT:
		video_cal_sec_stat(video);
		break;

	case CLNE_AUDIO_REQUEST_SPEEK:
		*ret = audio_request_speek(video, true);
		break;

	case CLNE_AUDIO_RELEASE_SPEEK:
		*ret = audio_request_speek(video, false);
		break;

	case CLNE_AUDIO_PUT_SOUND:
		*ret = audio_send_sound(video, pkt);
		break;
    case CLNE_VIDEO_SET_V4L2:
        do_video_set_v4l2(video, pkt);
        break;
    case CLNE_VIDEO_SET_ROLL_SPEED:
        do_video_set_roll_ctrl(video, pkt);
        break;
    case CLNE_VTAP_QUERY_LIST:
        do_vtap_query_list(video,(cln_video_t*)pkt->data);
        break;
    case CLNE_VTAP_START:
        vtap_open(video, pkt,ret);
        break;
    case CLNE_VTAP_STOP:
        vtap_close(video);
        break;

	default:
		if ( vrt_proc_notify(video, pkt, ret) )
			return true;
		return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////

video_t *video_alloc(slave_t *slave)
{
	video_t *video;
	
	video = cl_calloc(sizeof(video_t), 1);
	video->slave = slave;
	
	video->pic_ring = pr_alloc();

	STLC_INIT_LIST_HEAD(&video->tcp_pkt_list);
    STLC_INIT_LIST_HEAD(&video->vtap_list);
    video->is_pre_get_vtap = true;

	video->vtry.video = video;
	vt_init(&video->vtry);
	STLC_INIT_LIST_HEAD(&video->net_detect_list);
	video->slect_p2p = -1;
	
	return video;
}

void video_free(video_t *video)
{
	if (video == NULL)
		return;
	
	video->is_open = false;
	
	stop_h264_decode(video);
	
	if (video->tcp_buff != NULL)
		bf_free(video->tcp_buff);

	stlc_list_free(net_detect_t, &video->net_detect_list, link);
    CL_THREAD_TIMER_OFF(video->t_vs_timer);
    CL_THREAD_TIMER_OFF(video->t_nack);

	video_off_all_thread(video, true);
	vt_destroy(&video->vtry);
	pr_free(video->pic_ring);
	
	pr_free(video->sound_ring);
	de_free(video->data_sound);
	bf_free(video->buf_sound_recv_raw);

	bf_free(video->buf_sound_send_remain);
	video->buf_sound_send_remain = NULL;
    //清除录像请求
    stlc_list_free(vtap_record_item_t, &video->vtap_list, link);
	
	SAFE_FREE(video->last_video_ip_list);
	SAFE_FREE(video->last_err);
	de_free(video->data_pic);
	de_free(video->data_h264);
	SAFE_FREE(video->timer);
	SAFE_FREE(video->static_pic);
	cl_free(video);
}

