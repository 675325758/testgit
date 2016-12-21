#include "client_lib.h"
#include "cl_priv.h"
#ifndef REMOVE_ILBC_LIB
#include "ilbc.h"
#endif
#include "audio_priv.h"
#include "video_priv.h"
#include "aes.h"
#include "wait_server.h"

 /* 采样率映射表，VOICE_RETE_xxx -> 值 */
static int sample_map[] = {
	8000,
	16000,
	32000,
	44100,
	48000
};

int audio_sample_to_idx(int sample)
{
	int i;
	
	for (i = 0; i < sizeof(sample_map)/sizeof(sample_map[0]); i++) {
		if (sample_map[i] == sample)
			return i;
	}

	return 0;
}

int audio_idx_to_sample(int idx)
{
	if (idx < sizeof(sample_map)/sizeof(sample_map[0])) {  /* 采样率 */
		return sample_map[idx];
	} 

	return sample_map[0];
}


int audio_encode(u_int8_t *src, u_int8_t *dst, int raw_len)
{
#ifndef REMOVE_ILBC_LIB
	int frameLen, pos, len, total;
	int mode = 20; /* 20 ms */
	iLBC_encinst_t *Enc_Inst;

	/* Create structs */
	WebRtcIlbcfix_EncoderCreate(&Enc_Inst);

	/* Initialization */
	WebRtcIlbcfix_EncoderInit(Enc_Inst, mode);

	frameLen = mode*8;

	// 必须一块一块压缩，最后不完整的都不能要了
	raw_len -= sizeof(WebRtc_Word16)*frameLen;
	/*
		x86下测试看，20ms每块len=38, 30ms每块len=50
	*/
	for (pos = 0, total = 0; pos <= raw_len; pos += sizeof(WebRtc_Word16)*frameLen) {
		len = WebRtcIlbcfix_Encode(Enc_Inst, (WebRtc_Word16 *)(src + pos), 
				(WebRtc_Word16)frameLen, (WebRtc_Word16 *)(dst + total));
		total += len;
	}
	
	/* Free structs */
	WebRtcIlbcfix_EncoderFree(Enc_Inst);

	return total;
#else
	return 0;
#endif
}

int audio_decode(u_int8_t *src, u_int8_t *dst, int raw_len)
{
#ifndef REMOVE_ILBC_LIB
	int frameLen, pos, len, total, mode = 20;
	iLBC_decinst_t *Dec_Inst;
	WebRtc_Word16 speechType = 0;

	/* Create structs */
	WebRtcIlbcfix_DecoderCreate(&Dec_Inst);

	/* Initialization */
	WebRtcIlbcfix_DecoderInit(Dec_Inst, mode);

	frameLen = ILBC_BLK_SIZE_20MS;

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

void do_get_sound(video_t *video, pic_ring_t *pr, int idx)
{
	int enc_total, dec_total, total, n;
	data_buf_t *db;

	enc_total = get_pic_size(pr, idx);
	
	if (video->data_sound == NULL) {
		video->data_sound = de_alloc(NUM_VOICE_DB, VOICE_BUF_SIZE, false);
	}

	// 检查原始数据缓冲区大小
	if (video->buf_sound_recv_raw == NULL) {
		video->buf_sound_recv_raw = bf_alloc(enc_total + 1024);
	}
	if (bf_get_write_free(video->buf_sound_recv_raw) < enc_total) {
		bf_expand(video->buf_sound_recv_raw, enc_total + 1024);
	}

	// 解密
	dec_total = pr_get_sound_data(pr, idx, 
		(u_int8_t*)(video->buf_sound_recv_raw->buf + video->buf_sound_recv_raw->write_pos),
		video->is_aes_enc ? video->m_Syna.key_rand : NULL);
	video->buf_sound_recv_raw->write_pos += dec_total;

    if (video->is_vtap) {
        db = de_get_write(video->data_sound, dec_total);
        memcpy(&db->data[0], video->buf_sound_recv_raw->buf, dec_total);
        db->data_size = dec_total;
        de_move_write(video->data_sound);
        video->buf_sound_recv_raw->write_pos = 0x0;
    }else{
        // 解码
        n = video->buf_sound_recv_raw->write_pos%ILBC_BLK_SIZE_20MS;
        total = (video->buf_sound_recv_raw->write_pos - n)/ILBC_BLK_SIZE_20MS * ILBC_RAW_BLK_SIZE_20MS;
        
        db = de_get_write(video->data_sound, total);
        db->data_size = audio_decode((u_int8_t*)video->buf_sound_recv_raw->buf, &db->data[0], video->buf_sound_recv_raw->write_pos - n);
        de_move_write(video->data_sound);
        
        log_debug("decode sound, encrypt %u bytes, decrypt %u bytes, decompress %u(%u) from %u, remain %u to next\n",
                  enc_total, dec_total, total, db->data_size, video->buf_sound_recv_raw->write_pos - n, n);
        
        // 有剩余的，要留到下次来过
        if (n) {
            memcpy(video->buf_sound_recv_raw->buf, video->buf_sound_recv_raw->buf + video->buf_sound_recv_raw->write_pos - n, n);
        }
        video->buf_sound_recv_raw->write_pos = n;
    }
	
	event_push(video->callback, VE_GET_SOUND, video->handle, video->callback_handle);
}

// 处理设备返回的应答
RS audio_proc_speek_a(video_t *video, pkt_t *pkt)
{
	net_speek_a_t *sa;

	sa = get_pkt_payload(pkt, net_speek_a_t);
	sa->err_number = ntohl(sa->err_number);
	
	log_info("video %s recv speek reply, op=%u, timeout=%u, err_number=%u\n",
		video->slave->str_sn, sa->operate, sa->timeout, sa->err_number);
	
	if (sa->operate == SPEEK_OP_REQUEST) {
		if (sa->err_number == RS_OK) {
			event_push(video->callback, VE_TALK_REQ_SUCCESS, video->handle, video->callback_handle);
		} else {
			event_push_err(video->callback, VE_TALK_REQ_FAILED, video->handle, video->callback_handle, sa->err_number);
		}
	}

	return RS_OK;
}

// 处理用户过来的请求。向设备发送相应请求
RS audio_request_speek(video_t *video, bool request)
{
	pkt_t *pkt;
	net_speek_q_t *sq;

	if (video->status != VS_ESTABLISH) {
		log_err(false, "请求对讲，但是当前视频状态是 %d\n", video->status);
		return RS_NOT_INIT;
	}
	if (video->m_Syna.encrypt_method != ENC_FRAG_AES128_H264_AUDIO_TWOWAY) {
		log_err(false, "请求对讲，但是当前协商的视频格式是 0x%02x\n", video->m_Syna.encrypt_method);
		return RS_NOT_SUPPORT;
	}
	
	pkt = pkt_new_v2(CMD_SPEEK_Q, sizeof(net_speek_q_t), NHF_WAIT_REPLY | NHF_TRANSPARENT, video->slave->sn, video->slave->user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, video->handle, CMD_SPEEK_Q, NULL, callback_video_request);
	sq = get_pkt_payload(pkt, net_speek_q_t);
	sq->client_id = video->m_Syna.client_id;
	sq->operate = request ? SPEEK_OP_REQUEST : SPEEK_OP_RELEASE;
	
	user_add_pkt(video->slave->user, pkt);

	return RS_OK;
}

RS audio_send_sound(video_t *video, cl_notify_pkt_t *cln_pkt)
{
	cl_sound_data_t *sd = &((cln_video_t *)&cln_pkt->data[0])->u.sound_data;
	pkt_t *pkt;
	int len_raw, len_encode, len_encrypt;
	net_voice_t *nv;
	buff_t *bf;
	int src_ofs = 0, dst_ofs = 0;
	u_int32_t ts_now, ts;

	if ( ! video->is_open) {
		log_err(false, "video %s audio_send_sound failed: not open", video->slave->str_sn);
		return RS_ERROR;
	}
	if (video->status != VS_ESTABLISH) {
		log_err(false, "video %s audio_send_sound failed: status=%d", video->slave->str_sn, video->status);
		return RS_ERROR;
	}
	if (sd->len < ILBC_RAW_BLK_SIZE_20MS) {
		log_err(false, "audio_send_sound, too short sound = %d\n", sd->len);
		return RS_ERROR;
	}

	if (video->buf_sound_send_remain == NULL) {
		video->buf_sound_send_remain = bf_alloc(ILBC_RAW_BLK_SIZE_20MS);
	}
	bf = video->buf_sound_send_remain;

	// 计算几个长度
	len_raw = bf->write_pos + sd->len;
	len_encode = len_raw / ILBC_RAW_BLK_SIZE_20MS * ILBC_BLK_SIZE_20MS;
	len_encrypt = (len_encode + AES128_EKY_LEN - 1) & (~(AES128_EKY_LEN - 1));

	pkt = pkt_new(CMD_VOICE, sizeof(net_voice_t) + len_encrypt, video->slave->user->ds_type);
	nv = get_pkt_payload(pkt, net_voice_t);

	// 先编码成ILBC格式的声音
	if (bf->write_pos > 0) {
		src_ofs = ILBC_RAW_BLK_SIZE_20MS - bf->write_pos;
		memcpy(bf->buf + bf->write_pos, sd->data, src_ofs);
		dst_ofs = audio_encode((u_int8_t*)bf->buf, nv->data, ILBC_RAW_BLK_SIZE_20MS);
	}
	audio_encode((u_int8_t *)sd->data + src_ofs, nv->data + dst_ofs, sd->len - src_ofs);

	// 再进行AES128加密，传的长度必须是填充后的
	enc_block(nv->data, len_encrypt, video->m_Syna.key_rand);

	ts_now = (get_sec()*100 + get_msec()/10);
	ts = ts_now - video->timestamp_base;

	// 填写其他信息
	video->sound_seq++;
	nv->seq = htons(video->sound_seq);
	nv->pad_len = (u_int8_t)(len_encrypt - len_encode);
	nv->ver = 0;
	nv->rate = audio_sample_to_idx(sd->samples);
	nv->total = 1;
	nv->index = 0;
	nv->format = AUDIO_FMT_ILBC;
	nv->client_id = video->m_Syna.client_id;
	nv->channel = sd->channels;
	nv->resv = 0;
	nv->timestamp = htonl(ts);		

	log_debug("send voice to %s, seq=#%u, pad_len=%u, rate=%u, client_id=%u, channel=%u, timestamp=%u\n", 
		video->slave->str_sn, ntohs(nv->seq), nv->pad_len, nv->rate, nv->client_id, nv->channel, ntohl(nv->timestamp));
	// 发到发送队列
	video_send(video, pkt);

	// 残留的部分保存起来
	if ((bf->write_pos = len_raw%ILBC_RAW_BLK_SIZE_20MS) != 0) {
		memcpy(bf->buf, ((char *)sd->data) + (sd->len - bf->write_pos), bf->write_pos);
		log_debug("video %s, audio_send_sound len=%d, remain=%d\n", video->slave->str_sn, sd->len, bf->write_pos);
	}

	return RS_OK;
}

