#ifndef	__AUDIO_PRIV_H__
#define	__AUDIO_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif 

// 压缩前块大小
#define	ILBC_RAW_BLK_SIZE_20MS	320
#define	ILBC_RAW_BLK_SIZE_30MS	480
// 压缩后块大小
#define	ILBC_BLK_SIZE_20MS	38
#define	ILBC_BLK_SIZE_30MS	50

extern int audio_encode(u_int8_t *src, u_int8_t *dst, int raw_len);
extern int audio_decode(u_int8_t *src, u_int8_t *dst, int raw_len);
extern void do_get_sound(video_t *video, pic_ring_t *pr, int idx);
extern RS audio_proc_speek_a(video_t *video, pkt_t *pkt);
extern RS audio_request_speek(video_t *video, bool request);
extern int audio_sample_to_idx(int sample);
extern int audio_idx_to_sample(int idx);
extern RS audio_send_sound(video_t *video, cl_notify_pkt_t *cln_pkt);

#ifdef __cplusplus
}
#endif 

#endif

