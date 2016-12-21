#ifndef	__PIC_RING_H__
#define	__PIC_RING_H__

#ifdef __cplusplus
extern "C" {
#endif 

// 一个分片最大多少字节
#define	MAX_FRAG_SIZE	1500
#define	DFL_WIN	30
#define	MAX_FRAGS_PER_PIC	250

// 720P需要的大小，RGB 24位
#define	BUF_SIZE_720P (1280*720*3)
#define	BUF_SIZE_1080P (1920*1080*3)
#define BMP_BUF_SIZE	(BUF_SIZE_720P + 2048)

#define	VOICE_BUF_SIZE	8192

#define	NUM_PIC_DB	3
#define	NUM_VOICE_DB	6

typedef struct {
	int total;
	int recv_count;
	// 发送nack_send后收到多少片
	int recv_after_nack;
	int size;
	// 是否已经发送了NACK，只发一次
	bool nack_send;
    u_int16_t seq;
	// 256 * 1500 = 384 KBytes
	pkt_t *pkts[MAX_FRAGS_PER_PIC];
} pic_cache_t;

typedef enum {
	PRT_JPG, /* JPG视频 */
	PRT_H264, /* H264视频 */
	PRT_VOICE /* 声音数据 */
} pr_type_t;

typedef struct {
	pr_type_t type;
	
	bool is_first_pic;
	u_int16_t win;
	u_int16_t first_seq;
	u_int16_t first_idx;
	u_int16_t current_seq;
	pic_cache_t **cache;

	// 统计相关
	int quality;
} pic_ring_t;

extern pic_ring_t *pr_alloc();
extern void pr_free(pic_ring_t *pr);
extern void pr_free_pkt(pic_cache_t *c);
extern void pr_reset(pic_ring_t *pr);
extern pic_cache_t *pr_get_cache(pic_ring_t *pr, int idx);
extern void pr_drop_head(pic_ring_t *pr, u_int16_t nRecv);
extern int pr_get_pic_data(pic_ring_t *pr, int idx, u_int8_t *buf, u_int8_t *pwd_md5);
extern int pr_get_sound_data(pic_ring_t *pr, int idx, u_int8_t *buf, u_int8_t *pwd_md5);
extern bool pr_put_jpg(video_t *video, pic_ring_t *pr, pkt_t *pkt, net_video_jpg_v1_t *jpg);
extern bool pr_put_sound(video_t *video, pic_ring_t *pr, pkt_t *pkt, net_voice_t *vc);
extern int get_pic_size(pic_ring_t *pr, int idx);


#ifdef __cplusplus
}
#endif 

#endif

