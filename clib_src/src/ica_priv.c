/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: indiacar server client
**  File:    ica_priv.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    12/30/2015
**
**  Purpose:
**    Indiacar server client.
**************************************************************************/


/* Include files. */
#include "ica_priv.h"
#include "cl_thread.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "md5.h"
#include "h264_decode.h"
#include "client_lib.h"
#include "cl_priv.h"
#include "mp4_fmt.h"
#include "audio_priv.h"
#ifndef REMOVE_ILBC_LIB
#include "ilbc.h"
#endif

#if 0
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif


/* Macro constant definitions. */
// 720P需要的大小，RGB 24位
#define	BUF_SIZE_720P (1280*720*3)
#define	BUF_SIZE_1080P (1920*1080*3)
#define BMP_BUF_SIZE	(BUF_SIZE_720P + 2048)

#define	NUM_VOICE_DB	6
#define	VOICE_BUF_SIZE	8192

#define	NUM_PIC_DB	3

/* Type definitions. */
enum {
	ICAC_IDLE,
	ICAC_CONNECTING,
	ICAC_AUTHING,
	ICAC_ESTAB,
	ICAC_ERROR,
	ICAC_MAX
};

typedef void (*ica_func_t)();
typedef struct {
	char *name;
	ica_func_t on_into;
	ica_func_t proc_pkt;
} icac_proc_t;


typedef struct {
	struct stlc_list_head link;
	int total_len;
	int send_len;
	u_int16_t command;
	u_int8_t data[0];	// hd + param
} icac_pkt_t;


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
static int audio_decode1(u_int8_t *src, u_int8_t mode, u_int8_t *dst, int raw_len)
{
#ifndef REMOVE_ILBC_LIB
	int frameLen, pos, len, total;
	iLBC_decinst_t *Dec_Inst;
	WebRtc_Word16 speechType = 0;

	/* Create structs */
	WebRtcIlbcfix_DecoderCreate(&Dec_Inst);

	/* Initialization */
	WebRtcIlbcfix_DecoderInit(Dec_Inst, mode);

	if (mode == 30) {
		frameLen = ILBC_BLK_SIZE_30MS;
	} else if (mode == 20) {
		frameLen = ILBC_BLK_SIZE_20MS;
	} else {
		return 0;
	}

	// 必须一块一块压缩，最后不完整的都不能要了
	raw_len -= frameLen;
	/*
		x86下测试看，20ms每块len=38, 30ms每块len=50
	*/
	for (pos = 0, total = 0; pos <= raw_len; pos += frameLen) {
		len = WebRtcIlbcfix_Decode(Dec_Inst, (WebRtc_Word16 *)(src+pos), 
				(WebRtc_Word16)frameLen, (WebRtc_Word16 *)(dst+total), &speechType);
		len *= sizeof(WebRtc_Word16);
		total += len;
	}
	
	/* Free structs */
	WebRtcIlbcfix_DecoderFree(Dec_Inst);

	return total;
#else
	return 0;
#endif
}



static void ica_notify_video_proc(video_t *video)
{
	event_push(video->callback, UE_INDIACAR_GET_PIC, video->handle, video->callback_handle);
}

static void ica_notify_audio_proc(video_t *video)
{
	event_push(video->callback, UE_INDIACAR_GET_AUDIO, video->handle, video->callback_handle);
}

static void ica_hd_decode(video_t *video)
{
	int bmp_len;
	data_buf_t *db_raw, *db_pic;
    
#ifndef REMOVE_H264_LIB
	
	while ((db_raw = de_get_read(video->data_h264)) != NULL) {
		db_pic = de_get_write(video->data_pic, BMP_BUF_SIZE);
		bmp_len = Gw_m264_deocde((u_int8_t *)db_pic->data, db_raw->data, db_raw->data_size, video->h264_handle);
#if 1
		log_debug("decode thread: raw=%u bytes, bmp_len=%d\n", db_raw->data_size, bmp_len);
#endif
		de_move_read(video->data_h264);

		if (bmp_len > 0) {
			db_pic->data_size = bmp_len;
			de_move_write(video->data_pic);
			ica_notify_video_proc(video);
		} else if (bmp_len < 0) {
			log_err(false, "decode h264 failed, reinit\n");

			if (video->h264_handle) {
				Gw_m264_del(video->h264_handle);
			}
			video->h264_handle = Gw_m264_new(180);
		}
	}
    
#endif
}

static RS ica_hd_proc_notify(video_t *video, cl_thread_info_t *ti, cl_notify_pkt_t *pkt)
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
		ica_hd_decode(video);
		break;
	}

	return ret;
}

RS ica_hd_proc_notify_self(cl_notify_pkt_t *pkt)
{
	log_err(false, "解码线程不应该自己处理自己的通告。类型=%u\n", pkt->type);
	return RS_ERROR;
}

u_int32_t ica_h264_decode_thread(void *param)
{
	RS ret;
	cl_notify_pkt_t *notify;
	video_t *video = (video_t *)param;
	cl_thread_info_t *ti = &video->ti_h264_decode;
	struct sockaddr_in from;

	log_info("start ica decode thread\n");
	
	ti->proc_notify = ica_hd_proc_notify_self;
#ifndef REMOVE_H264_LIB
	video->h264_handle = Gw_m264_new(180);

	while ( ! ti->stopping ) {
		while ((notify = cl_recv_notify(ti, 1000, &from)) != NULL) {
			ret = ica_hd_proc_notify(video, ti, notify);
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
#endif
	video->h264_handle = NULL;

	log_info("H264 decode thread exit now!\n");

	cl_destroy_thread(ti);
	
	return 0;
}

static RS ica_start_h264_decode(video_t *video)
{
	cl_create_thread(&video->ti_h264_decode, "ica_h264_decode", ica_h264_decode_thread, (void *)video);

	return RS_OK;
}

void ica_stop_h264_decode(video_t *video)
{
	cl_notify_destroy_thread(&video->ti_h264_decode);
}


////////////////////////////////////////////////////////////////////////////////////////
void icac_set_state(ica_client_t *c, int state);
int icac_on_read(cl_thread_t *t);
int icac_on_write(cl_thread_t *t);


void ica_header_order(ica_header_t *hd)
{
	hd->param_len = ntohl(hd->param_len);
	hd->command = ntohs(hd->command);
	hd->sn = ntoh_ll(hd->sn);
}

bool ica_header_check(ica_header_t *hd)
{
	if (hd->ver != 1) {
		log_debug("ver %u != 1\n", hd->ver);
		return false;
	}

	if (net_header_real_size(hd) != net_header_size) {
		log_err(false, "hlen %d invalid\n", hd->hlen);
		return false;
	}


	if (hd->param_len > MAX_ICA_PKT_PARAM_SIZE) {
		log_err(false, "param len %d too big\n", hd->param_len);
		return false;
	}
	
	return true;
}

static void icac_notify_server_err(ica_client_t *c)
{	
	log_debug("  === icac_notify_server_err\n");
	event_push(c->callback, UE_INDIACAR_AGENT_SERVER_ERROR, c->handle, c->callback_handle);
}

static RS icac_check_connecting(ica_client_t *c)
{
	int status;
	int slen;
	int ret;

	slen = sizeof(status);
	ret = (int)getsockopt(c->sock, SOL_SOCKET, SO_ERROR, (void *)&status, (socklen_t *)&slen);
	if (ret < 0) {
		icac_set_state(c, ICAC_IDLE);
		return RS_ERROR;
	}
	if (status != 0) {
		log_err(false, "icac_check_connecting state %u != 0\n", status);
		icac_set_state(c, ICAC_IDLE);
		return RS_ERROR;
	}

	log_debug("check connect ok\n");

	
	icac_set_state(c, ICAC_AUTHING);

	return RS_OK;
}

int icac_on_write(cl_thread_t *t)
{
	int n, want;
	ica_client_t *c = CL_THREAD_ARG(t);
	icac_pkt_t *pkt;

	c->t_write = NULL;

	if (c->state == ICAC_CONNECTING) {
		icac_check_connecting(c);
		return 0;
	}

	if (stlc_list_empty(&c->pkt_list)) {
		return 0;
	}

	pkt = stlc_list_entry(c->pkt_list.next, icac_pkt_t,  link);
	want = pkt->total_len- pkt->send_len;

	if (want == 0) {
		stlc_list_del(&pkt->link);
		cl_free(pkt);
		return 0;
	}

	n = (int)send(c->sock, &pkt->data[pkt->send_len], want, 0);
	if (n < 0) {
		if (NORMAL_ERRNO(GET_SOCK_ECODE())) {
			CL_THREAD_WRITE_ON(&cl_priv->master, c->t_write, icac_on_write, c, c->sock);
			return 0;
		}
		goto err_out;
	} else if (n == 0) {
		goto err_out;
	}

	if (n == want) {
		stlc_list_del(&pkt->link);
		free(pkt);

		if (stlc_list_empty(&c->pkt_list)) {
			return 0;
		}
	}

	CL_THREAD_WRITE_ON(&cl_priv->master, c->t_write, icac_on_write, c, c->sock);
	
	return 0;
err_out:
	icac_notify_server_err(c);
	icac_set_state(c, ICAC_ERROR);
	return -1;
}

icac_pkt_t *icac_pkt_new(u_int64_t sn, u_int16_t command, int data_len)
{
	icac_pkt_t *pkt;
	ica_header_t *hd;
	int total_len = sizeof(icac_pkt_t) + sizeof(*hd) + data_len;

	if ((pkt = cl_calloc(1, total_len)) == NULL) {
		log_err(true, "calloc pkt len %d failed\n");
		return NULL;
	}

	pkt->total_len = sizeof(*hd) + data_len;
	pkt->command = command;

	hd = (ica_header_t*)pkt->data;
	hd->ver = 1;
	hd->hlen = ica_header_size/4;
	hd->ds_type = 2;
	hd->command = ntohs(command);
	hd->param_len = ntohl(data_len);
	hd->sn = ntoh_ll(sn);

	return pkt;
}

void icac_pkt_add(ica_client_t *c, icac_pkt_t *pkt)
{
	stlc_list_add_tail(&pkt->link, &c->pkt_list);

	if (c->t_write == NULL) {
		CL_THREAD_WRITE_ON(&cl_priv->master, c->t_write, icac_on_write, c, c->sock);
	}
}


int icac_idle_timer(cl_thread_t *t)
{
	ica_client_t *c;
	bool inprogress = false;

	c = CL_THREAD_ARG(t);

	c->t_timer = NULL;


#if 0
	c->agent_ip = ntohl(inet_addr("10.11.7.117"));
	c->agent_port = 10000;
#endif

	log_debug("icac idle timer: ip %u.%u.%u.%u\n", IP_SHOW(c->agent_ip));

	if (c->agent_ip == 0) {
		CL_THREAD_TIMER_ON(&cl_priv->master, c->t_timer, icac_idle_timer, c, TIME_N_SECOND(3));
		return 0;
	}

	CLOSE_SOCK(c->sock);

	c->sock = connect_tcp(c->agent_ip, c->agent_port, &inprogress);

	if (c->sock == INVALID_SOCKET) {
		icac_set_state(c, ICAC_IDLE);
		return 0;
	}

	CL_THREAD_READ_ON(&cl_priv->master, c->t_read, icac_on_read, c, c->sock);

	if ( inprogress ) {
		log_debug("is inprogress, wait...\n");
		icac_set_state(c, ICAC_CONNECTING);		
		CL_THREAD_WRITE_ON(&cl_priv->master, c->t_write, icac_on_write, c, c->sock);
	} else {
		log_debug("connect to %u.%u.%u.%u:%u success\n", IP_SHOW(c->agent_ip), c->agent_port);		
		//CL_THREAD_READ_ON(&cl_priv->master, c->t_read, icac_on_read, c, c->sock);
		icac_set_state(c, ICAC_AUTHING);
	}

	return 0;
}

int icac_error_timer(cl_thread_t *t)
{
	ica_client_t *client = CL_THREAD_ARG(t);

	client->t_timer = NULL;
	
	icac_set_state(client, ICAC_IDLE);

	return 0;
}


static void icac_reset_send_list(ica_client_t *c)
{
	icac_pkt_t *pkt, *n;
	
	stlc_list_for_each_entry_safe(icac_pkt_t, pkt, n, &c->pkt_list, link) {
		stlc_list_del(&pkt->link);
		free(pkt);
	}
	
	STLC_INIT_LIST_HEAD(&c->pkt_list);
}

static void icac_reset(ica_client_t *c)
{
	CLOSE_SOCK(c->sock);

	CL_THREAD_OFF(c->t_timer);
	CL_THREAD_OFF(c->t_write);
	CL_THREAD_OFF(c->t_read);

	c->recv_len = c->param_len = 0;

	icac_reset_send_list(c);

	c->agent_ip = c->agent_port = 0;
	c->has_get_first_i = 0;

	if (c->mp4_handle) {
		mp4_close(c->mp4_handle);
		c->mp4_handle = NULL;
		c->mp4_get_first_i = false;
	}
}

static void icac_idle_into(ica_client_t *c)
{
	icac_reset(c);
	
	CL_THREAD_TIMER_ON(&cl_priv->master, c->t_timer, icac_idle_timer, c, 0);
}

static void icac_idle_proc(ica_client_t *c)
{}

static void icac_authing_into(ica_client_t *c)
{
	auth_request_t *ar;
	u_int32_t rand1;
	icac_pkt_t *pkt;
	srand(get_sec());
    rand1 = rand();
	
	pkt = icac_pkt_new(c->sn, CMD_UDP_AUTH, sizeof(*ar));
	if (!pkt) {
		log_err(false, "calloc pkt failed\n");
		goto err_out;
	}

	ar = get_ica_pkt_payload(pkt, auth_request_t);
	ar->action = UCA_REQUEST;
	memcpy(ar->rand1, &rand1, sizeof(rand1));
	memcpy(c->rand1, &rand1, sizeof(rand1));
	
	icac_pkt_add(c, pkt);   
 
	return; 
err_out:
	icac_notify_server_err(c);
	icac_set_state(c, ICAC_ERROR);
}

static void icac_do_question(ica_client_t *c)
{
	MD5_CTX ctx;
	auth_question_t *aq = (auth_question_t *)c->param;
	auth_answer_t *aa;
	icac_pkt_t *pkt;

	log_debug("do question\n");

	memcpy(c->rand2, aq->rand2, sizeof(c->rand2));

	MD5Init(&ctx);
	MD5Update(&ctx, c->rand1, sizeof(c->rand1));
	MD5Update(&ctx, c->rand2, sizeof(c->rand2));

	MD5Final(c->key, &ctx);

	pkt = icac_pkt_new(c->sn, CMD_UDP_AUTH, sizeof(*aa));
	if (!pkt) {
		log_err(false, "pkt new failed\n");
		goto err_out;
	}
	aa = get_ica_pkt_payload(pkt, auth_answer_t);
	aa->action = UCA_ANSWER;

	memcpy(aa->answer, c->key, sizeof(aa->answer));

	icac_pkt_add(c, pkt);

	return;
err_out:
	icac_notify_server_err(c);
	icac_set_state(c, ICAC_ERROR);
}

static void icac_do_result(ica_client_t *c)
{
	auth_result_t *ar;

	ar = (auth_result_t *)c->param;

	log_debug("icac do result err %u\n", ar->err);

	if (ar->err == 0) {
		log_debug("ica estab\n");
		icac_set_state(c, ICAC_ESTAB);
	} else {
		log_debug("auth failed\n");
		icac_notify_server_err(c);
		icac_set_state(c, ICAC_ERROR);
	}
}

static void icac_authing_proc(ica_client_t *c)
{
	u_int8_t *op;

	op = c->param;

	log_debug("enter authing proc, action %u\n", *op);

	if (c->hd.command != CMD_UDP_AUTH) {
		log_err(false, "in authing proc, but cmd not auth\n");
		return;
	}

	switch (*op) {
		case UCA_QUESTION:
			icac_do_question(c);
			break;
		case UCA_RESULT:
			icac_do_result(c);
			break;
		default:
			log_err(false, "invalid op %u in authing proc\n");
			break;
	}
}

static void icac_estab_into(ica_client_t *c)
{
	// TODO:超时，多久内没有转发数据，断开
}

static void icac_do_video(ica_client_t *c)
{
	int total, width, height;
	data_buf_t *db;
	video_t *video = c->video;
	//static int n = 0;
	video_data_t *vd = (video_data_t *)c->param;
	//static int count = 0;
	RS ret;
	
	if (c->param_len < sizeof(*vd)) {
		return;
	}

	if (c->has_get_first_i == 0) {
		if (vd->type == 1) {
			log_debug("get first i frame\n");
			c->has_get_first_i = 1;		
		} else {
			return;
		}
	}
	
	log_debug("estab get pkt, cmd %u len %u [0x%x 0x%x 0x%x 0x%x 0x%x] %d\n", c->hd.command, c->param_len, 
		c->param[0], c->param[1], c->param[2], c->param[3], c->param[4], n++);


	total = c->param_len - sizeof(*vd);

	if (total == 0) {
		return;
	}

	//log_debug("get type %u len %u [0x%x 0x%x 0x%x 0x%x]\n", vd->type, total, vd->data[0], vd->data[1], vd->data[2], vd->data[3]);

#if 1
	db = de_get_write(video->data_h264, total);
	db->data_size = total;
	memcpy(db->data, vd->data, total);
	de_move_write(video->data_h264);

	if (h264_get_pic_size(db->data, db->data_size, &width, &height) == 0) {
		if (width != video->stat.width || height != video->stat.height) {
			log_info("视频分辨率变化: 从%ux%u 调整为 %ux%u, 重启解码线程\n", 
				video->stat.width, video->stat.height, width, height);
			if (video->stat.width != 0) {
				ica_stop_h264_decode(video);
				ica_start_h264_decode(video);
			}

			video->stat.width = width;
			video->stat.height = height;
		}
	}

	cl_send_notify_simple(&video->ti_h264_decode, CLNE_VIDEO_GET_H264);
#endif

	// 没有在录制
	if (c->mp4_handle == NULL) {
		return;
	}

	// 录制也要从I帧开始
	if (c->mp4_get_first_i == false) {
		if (vd->type != 1) {
			return;
		}

		c->mp4_get_first_i = true;
	}

	ret = mp4_video_write_frame(c->mp4_handle, vd->data, total);
	log_debug("write mp4 frame len %d ret %d\n", total, ret);
}

/*
static int  icac_test_write(char *path, u_int8_t *data, int len)
{
	FILE *fp = NULL;

	fp = fopen(path, "a+b");
	if (!fp) {
		return 0;
	}

	fwrite(data, 1, len, fp);

	fclose(fp);

	return 0;
}
*/

static void icac_do_audio(ica_client_t *c)
{
	int total;
	data_buf_t *db;
	video_t *video = c->video;
	//static int n = 0;
	audio_data_t *ad = (audio_data_t *)c->param;
	//static int count = 0;
	FILE *fp = NULL;
	//char raw_path[256], pcm_path[256];


	log_debug("do audio param_len %u \n", c->param_len);
	
	if (c->param_len < sizeof(*ad)) {
		return;
	}

	total = c->param_len - sizeof(*ad);

	ad->time = ntohl(ad->time);

	log_debug("estab get pkt audio mode %u audio len %u time %u\n", ad->mode, total, ad->time);

	if (total == 0) {
		return;
	}

	db = de_get_write(video->data_sound, total);
    db->data_size = audio_decode1(ad->data, ad->mode, db->data, total);
    de_move_write(video->data_sound);

	log_debug("decode audio len %d\n", db->data_size);

	ica_notify_audio_proc(video);

	if (ad->time == 0) {
		return;
	}

#if 0	
	sprintf(raw_path, "%s/audio/%d.enc", cl_priv->dir, ad->time);
	sprintf(pcm_path, "%s/audio/%d.pcm", cl_priv->dir, ad->time);


	icac_test_write(raw_path, ad->data, total);
	icac_test_write(pcm_path, db->data, db->data_size);
#endif	
	// 没有录制
	if (c->mp4_handle == NULL) {
		return;
	}

	if (c->has_get_first_i == 0) {
		return;
	}

	fp = fopen((char *)c->audio_record_path, "ab");
	if (!fp) {
		return;
	}

	fwrite(db->data, 1, db->data_size, fp);

	fclose(fp);
}


static void icac_estab_proc(ica_client_t *c)
{
	log_debug("icac_estab_proc cmd %u param_len %u\n", c->hd.command, c->param_len);

	switch (c->hd.command) {
		case ICA_CMD_VIDEO:
			icac_do_video(c);
			break;
		case ICA_CMD_AUDIO:
			icac_do_audio(c);
			break;
	}
}

static void icac_error_into(ica_client_t *c)
{
	icac_reset(c);

	//CL_THREAD_TIMER_ON(&cl_priv->master, c->t_timer, icac_error_timer, c, TIME_N_SECOND(5));
	log_debug("icac enter error state\n");
}

static void icac_error_proc(ica_client_t *c)
{
}



static icac_proc_t icac_proc[ICAC_MAX] = {
	{"ICAC_IDLE", icac_idle_into, icac_idle_proc},
	{"ICAC_CONNECTING", NULL, NULL},
	{"ICAC_AUTHING", icac_authing_into, icac_authing_proc},
	{"ICAC_ESTAB", icac_estab_into, icac_estab_proc},
	{"ICAC_ERROR", icac_error_into, icac_error_proc}
};

int icac_on_read(cl_thread_t *t)
{
	int len, want;
	u_int8_t *ptr;
	ica_client_t *c = CL_THREAD_ARG(t);

	c->t_read = NULL;

	if (c->state == ICAC_CONNECTING) {
		icac_check_connecting(c);
		return 0;
	}

	// 先接收头部
	if (c->recv_len < ica_header_size) {
		//log_debug("recv header..\n");
		ptr = (u_int8_t*)&c->hd;
		want = ica_header_size - c->recv_len;
		len = (int)recv(c->sock, ptr + c->recv_len, want, 0);
	} else {
		//log_debug("recv param %p.\n", s->param);
		want = ica_header_real_size(&c->hd) + c->hd.param_len - c->recv_len;
		len = (int)recv(c->sock, &c->param[c->recv_len - ica_header_size], want, 0);
	}

	if (len == 0) {
		log_err(false, "session disconnect...\n");
		goto err_out;
	}

	if (len < 0) {
		log_err(false, "read len %d < 0\n", len);
		if (NORMAL_ERRNO(GET_SOCK_ECODE())) {
			log_debug("normal tcp err\n");			
			CL_THREAD_READ_ON(&cl_priv->master, c->t_read, icac_on_read, c, c->sock);
			
			return 0;
		}

		goto err_out;
	}

//	log_debug("recv len %d\n", len);
	
	c->recv_len += len;
	
	if (c->recv_len == ica_header_size) {
		ica_header_order(&c->hd);
		if (ica_header_check(&c->hd) == false) {
			log_debug("check header failed\n");
			goto err_out;
		}
		
		c->param_len = c->hd.param_len;
	}


	// 判断参数是否接收完成
	if (c->recv_len < (int)(ica_header_size + c->hd.param_len)) {
		CL_THREAD_READ_ON(&cl_priv->master, c->t_read, icac_on_read, c, c->sock);
		return 0;
	}

	//log_debug("param len %d recv done, proc pkt now, cmd %u state %s\n", c->hd.param_len, c->hd.command, icac_proc[c->state].name);

	// 处理数据
	if (icac_proc[c->state].proc_pkt) {
		icac_proc[c->state].proc_pkt(c);
	}

	c->recv_len = 0;

	CL_THREAD_READ_ON(&cl_priv->master, c->t_read, icac_on_read, c, c->sock);
	
	return 0;

err_out:
	log_debug("read errout: now stat %u\n", c->state);
	icac_notify_server_err(c);
	icac_set_state(c, ICAC_ERROR);

	return -1;
}

void icac_set_state(ica_client_t *c, int state)
{
	int prev_state;
	
	if (state >= ICAC_MAX)
		return;


	prev_state = c->state;

	log_debug("set state from %s to %s\n", icac_proc[prev_state].name, icac_proc[state].name);

	c->state = state;

	if (icac_proc[state].on_into) {
		icac_proc[state].on_into(c);
	}
}

static video_t *ica_video_new(user_t *user)
{
	video_t *video;
	// 视频结构体初始化
	video = cl_calloc(1, sizeof(video_t));
	if (video == NULL) {
		return NULL;
	}

	log_debug("create ica video handle 0x%x\n", video->handle);
	
	
	if (video->data_h264 == NULL) {
		video->data_h264 = de_alloc(10, 2048, false);
	}
	
	if (video->data_pic == NULL) {
		video->data_pic = de_alloc(NUM_PIC_DB, BMP_BUF_SIZE, true);
	}

	if (video->data_sound == NULL) {
		video->data_sound = de_alloc(NUM_VOICE_DB, VOICE_BUF_SIZE, false);
	}

	// 使用user的回调
	video->handle = user->handle;
	video->callback = user->callback;
	video->callback_handle = user->callback_handle;
	
	ica_start_h264_decode(video);
	
	return video;
}

#if 1
u_int8_t h264[1024*1024] = {0};
//

cl_thread_t *t_input = NULL;
FILE *fp = NULL;;

#if 0
static int timer_input(cl_thread_t *t)
{
	u_int8_t frame[512];
	char path[64];
	int total, width, height;
	data_buf_t *db;
	static int idx = 0;
	video_t *video = CL_THREAD_ARG(t);
	static int count = 0;

	t_input = NULL;

	if (fp == NULL) {
		sprintf(path, "%s/h264.txt", cl_priv->dir);
		fp = fopen(path, "rb");
		if (!fp) {
			return -1;
		}
		log_debug("open video ok\n");
	}


	total = fread(frame, 1, sizeof(frame), fp);
	if (total == 0) {
		log_debug("read total == 0\n");
		fclose(fp);
		return -1;
	}

	log_debug("[%d] get %u bytes\n", idx++, total);
	db = de_get_write(video->data_h264, total);
	db->data_size = total;
	memcpy(db->data, frame, total);
	de_move_write(video->data_h264);

	if (h264_get_pic_size(db->data, db->data_size, &width, &height) == 0) {
		if (width != video->stat.width || height != video->stat.height) {
			log_info("视频分辨率变化: 从%ux%u 调整为 %ux%u, 重启解码线程\n", 
				video->stat.width, video->stat.height, width, height);
			if (video->stat.width != 0) {
				ica_stop_h264_decode(video);
				ica_start_h264_decode(video);
			}

			video->stat.width = width;
			video->stat.height = height;
		}
	}

	cl_send_notify_simple(&video->ti_h264_decode, CLNE_VIDEO_GET_H264);

done:
	CL_THREAD_TIMER_ON(&cl_priv->master, t_input, timer_input, video, TIME_N_MSECOND(120));

	return 0;
}
#endif
static int timer_input(cl_thread_t *t)
{
	u_int8_t frame[1240];
	char path[64], mp4_path[256];
	int total;// width, height;
	//data_buf_t *db;
	//static int idx = 0;
	video_t *video = CL_THREAD_ARG(t);
	int ret = 0;
	static mp4_handle_t *mp4_handle = NULL;
	static FILE *fp = NULL;

	t_input = NULL;

	sprintf(mp4_path, "%s/output.mp4", cl_priv->dir);

	if (fp == NULL) {
		sprintf(path, "%s/input.h264", cl_priv->dir);
		fp = fopen(path, "rb");
		if (!fp) {
			return -1;
		}
		log_debug("open h264 input ok\n");
	}


	if (mp4_handle == NULL) {
		mp4_handle = mp4_open(mp4_path, MP4OF_WRITE);
	}

	if ((total = (int)fread(frame, 1, sizeof(frame), fp)) > 0) {
		ret = mp4_video_write_frame(mp4_handle, frame, total);
		log_debug("write frame ret %d\n", ret);
	} else {
		fclose(fp);
		ret = mp4_close(mp4_handle);		
		log_debug("convert h264 to mp4 done[%s] ret %d\n", mp4_path, ret);

		return 0;
	}

	CL_THREAD_TIMER_ON(&cl_priv->master, t_input, timer_input, video, TIME_N_MSECOND(33));

	return 0;
}

void test_input(video_t *video)
{
	CL_THREAD_TIMER_ON(&cl_priv->master, t_input, timer_input, video, TIME_N_MSECOND(120));
}
#endif



void icac_set_agent_info(ica_client_t *client, u_int32_t agent_ip, u_int16_t agent_port, u_int16_t select_enc, u_int8_t key[16])
{
	if (!client) {
		return;
	}
	
	client->agent_ip = agent_ip;
	client->agent_port = agent_port;
	client->agent_select_enc = select_enc;
	memcpy(client->agent_key, key, 16);
}

void test_mp4_2_h264(void)
{
	mp4_handle_t *handle;
	char mp4_path[256] = {0};

	strcpy(mp4_path, "%s/mp4.mp4");
	
	handle = mp4_open(mp4_path, 0);
	if (!handle) {
		return;
	}

	
}

/**
	初始化客户端
*/
int icac_start(user_t *user, ica_client_t **pp)
{	
	ica_client_t *c;

	if (*pp != NULL) {
		c = *pp;
		goto start;
	}
	c = cl_calloc(1, sizeof(*c));
	if (!c) {
		return -1;
	}
	
	log_err(false, "icaic init\n");
	c->sn = user->sn;

	c->handle = user->handle;
	c->callback = user->callback;
	c->callback_handle = user->callback_handle;

	STLC_INIT_LIST_HEAD(&c->pkt_list);

	c->video = ica_video_new(user);
	if (c->video == NULL) {
		cl_free(c);
		return -1;
	}

	*pp = c;
	
start:
	icac_set_state(c, ICAC_IDLE);
//test_input(c->video);

	return 0;
}

void icac_stop(ica_client_t *c)
{
	if (c == NULL) {
		return;
	}
	
	icac_set_state(c, ICAC_ERROR);
}

// 是否正在正常看视频
bool icac_is_working(ica_client_t *c)
{	
	if (c == NULL) {
		return false;
	}

	return c->state == ICAC_ESTAB;
}

bool icac_is_record(ica_client_t *c)
{
	if (!c) {
		return false;
	}
	
	return c->mp4_handle != NULL;
}

void icac_set_record(ica_client_t *c, u_int8_t *path, bool onoff)
{
	int len;
	const char *audio = "pcm";
	
	if (c == NULL) {
		log_err(false, "ica client is null\n");
		return;
	}
	
	log_debug("set record to %u, before record onoff %u path[%s]\n", onoff, c->mp4_handle ? 1 : 0, path);
	if (onoff == true) {
		if (c->mp4_handle != NULL) {
			log_err(false, "want mp4 record, but is recording now...\n");
			event_push(c->callback, UE_INDIACAR_IS_RECORDING, c->handle, c->callback_handle);			
			return;
		}

		memcpy(c->record_path, path, sizeof(c->record_path));
		c->mp4_handle = mp4_open((char *)path, MP4OF_WRITE);

		// PCM路径，直接替换mp4后缀
		len = (int)strlen((char *)path);
		memcpy(c->audio_record_path, path, sizeof(c->record_path));
		memcpy(&c->audio_record_path[len - 3], audio, 3);		

		log_debug("open mp4 handle done\n");
	} else {
		// 正在录制的话就停止
		if (c->mp4_handle != NULL) {
			log_debug("stop record, path[%s]\n", c->record_path);
			// 
			mp4_close(c->mp4_handle);
			c->mp4_handle = NULL;
		}
	}
}

u_int32_t ica_mp4_decode_thread(void *param)
{
#ifdef REMOVE_H264_LIB
	return 0;
#else
	u_int32_t msec;
	int n, bmp_len;
	
	ica_mp4_decode_t *decode = (ica_mp4_decode_t *)param;
	mp4_video_frame_t vf;
	cl_thread_info_t *ti = &decode->ti_mp4_decode;
	u_int8_t *vdata = NULL;
	int vdata_size = 1024*1024;
	data_buf_t *db_pic;
	static u_int8_t last_is_puse = 0;

	log_info("start ica_mp4_decode_thread\n");
	
	ti->proc_notify = ica_hd_proc_notify_self;

	decode->h264_handle = Gw_m264_new(180);

	if (decode->data_pic == NULL) {
		decode->data_pic = de_alloc(NUM_PIC_DB, BMP_BUF_SIZE, true);
		if (decode->data_pic == NULL) {
			goto done;
		}
	}

	decode->mp4_decode_handle = mp4_open(decode->path, 0);
	if (!decode->mp4_decode_handle) {
		log_err(false, "open mp4 path[%s] failed\n", decode->path);
		goto done;
	}
	
	decode->mp4_duration = decode->mp4_decode_handle->boxes.mvhd->duration;

	if (decode->seek) {
		mp4_seek(decode->mp4_decode_handle, decode->seek);
	}

	decode->start_time = get_msec();
	decode->next_show_time = 0;

	if ((vdata = cl_calloc(1, vdata_size)) == NULL) {
		goto done;
	}
	
	vf.buf_size = vdata_size;
	vf.buf = vdata;
	
	while ( ! ti->stopping ) {
		// check if need seek
		if (decode->need_seek) {
			decode->need_seek = 0;
			mp4_seek(decode->mp4_decode_handle, decode->seek);
		}

		if (decode->need_puse) {
			last_is_puse = 1;
			msleep(1);
			continue;
		}

		if (last_is_puse == 1) {
			last_is_puse = 0;
			
			decode->start_time = get_msec();
			decode->next_show_time = 0;
		}

		// read the frame
		if ((n = mp4_video_read_frame(decode->mp4_decode_handle, &vf)) <= 0) {
			log_err(false, "mp4 read frame failed\n");
			goto done;
		}
		mp4_frame_2_h264(vf.buf, n);
		
		// calc sleep time
		decode->next_show_time += vf.duration;

		log_debug("	mp4 next show time %u\n", decode->next_show_time);

		// decode fram to bmp and put bmp to ring
		db_pic = de_get_write(decode->data_pic, BMP_BUF_SIZE);

		
		bmp_len = Gw_m264_deocde((u_int8_t *)db_pic->data, vf.buf, n, decode->h264_handle);
		if (bmp_len < 0) {
			log_err(false, "h264 decode failed\n");
			goto done;
		}

		db_pic->data_size = bmp_len;

		de_move_write(decode->data_pic);
		event_push(decode->callback, UE_INDIACAR_GET_MP4_PIC, decode->handle, decode->callback_handle);

		// sleep
		msec = get_msec() - decode->start_time;
		
		// decode too slow, 
		if (msec >= decode->next_show_time) {
			continue;
		}
		
		msleep(decode->next_show_time - msec);
		
		log_debug("	bmp len %d sleep %d ms\n", bmp_len, decode->next_show_time - msec);
	}

done:
	log_info("MP4 decode thread exit now!\n");

	if (decode->data_pic) {
		de_free(decode->data_pic);
		decode->data_pic = NULL;
	}
	
	SAFE_FREE(vdata);
	Gw_m264_del(decode->h264_handle);
	decode->h264_handle = NULL;

	cl_destroy_thread(ti);
	
	event_push(decode->callback, UE_INDIACAR_DECODE_MP4_FINISH, decode->handle, decode->callback_handle);
	
	return 0;
#endif	
}

void ica_do_mp4_decode(user_t *user, ica_mp4_decode_t **ppdecode, u_int64_t seek, u_int8_t *path, u_int8_t  action)
{
	ica_mp4_decode_t *decode = *ppdecode;

	if (*ppdecode == NULL) {
		decode = cl_calloc(1, sizeof(ica_mp4_decode_t));
		*ppdecode = decode;
	}

	// 使用user的回调
	decode->handle = user->handle;
	decode->callback = user->callback;
	decode->callback_handle = user->callback_handle;
	
	decode->seek = seek;

	log_debug("ica_start_mp4_decode path[%s] seek %u\n", path, seek);

	
	// 处理停止播放
	if (action == 0) {
		if (decode->h264_handle != NULL) {
			cl_notify_destroy_thread(&decode->ti_mp4_decode);
		}
		
		return;
	}

	if (action == 2) {
		decode->need_puse = 1;
		return;
	} 
	if (action == 3) {
		decode->need_puse = 0;
		return;
	}
	

	// 已经开始播放了，应该就是调下进度条
	if (decode->h264_handle != NULL) {
		if (seek) {
			decode->need_seek = 1;
			decode->seek = seek;
		}
		return;
	}

	// 重新开始播放
	strcpy(decode->path, (char *)path);
	decode->need_puse = decode->need_seek = 0;
	
	cl_create_thread(&decode->ti_mp4_decode, "ica_mp4_decode", ica_mp4_decode_thread, (void *)decode);
}
