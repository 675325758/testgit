#ifndef	__VIDEO_PRIV_H__
#define	__VIDEO_PRIV_H__

#include "buffer.h"
#include "video_try.h"
#include "pic_ring.h"
#include "data_exchg.h"

#ifdef __cplusplus
extern "C" {
#endif 


#define	ENC_NONE	0
#define	ENC_AES128	1
#define	ENC_AES128_UDP_PTZ	2
#define	ENC_AES128_H264	3
#define	ENC_AES128_H264_AUDIO_ONEWAY    4
#define	ENC_AES128_H264_AUDIO_TOWWAY    5
#define	MAX_ENC	6

#define	FRAG_FLAG	0x80
#define	ENC_FRAG_NONE	(ENC_NONE | FRAG_FLAG)
#define	ENC_FRAG_AES128	(ENC_AES128 | FRAG_FLAG)
#define	ENC_FRAG_AES128_UDP_PTZ	(ENC_AES128_UDP_PTZ | FRAG_FLAG)
#define	ENC_FRAG_AES128_H264	(ENC_AES128_H264 | FRAG_FLAG)
#define	ENC_FRAG_AES128_H264_AUDIO_ONEWAY	(ENC_AES128_H264_AUDIO_ONEWAY | FRAG_FLAG)
#define	ENC_FRAG_AES128_H264_AUDIO_TWOWAY	(ENC_AES128_H264_AUDIO_TOWWAY | FRAG_FLAG)

#define	SUPPORT_FRAG(e)	(((e) & FRAG_FLAG) != 0)

#define	AUDIO_NONE		0
// 只能听，不能说
#define	AUDIO_ONEWAY		1
// 对讲
#define   AUDIO_TWOWAY		2
    
#define SECOND_OF_DAY (60*60*24)
#define SECOND_OF_HOUR (60*60)

enum {
	VS_IDLE = 0,
	VS_VTAP,
	VS_VIDEO_REQ,
	VS_SYN_A_RCV,
	VS_ESTABLISH,
	VS_ERROR
};

//网络连接类型
enum {
	C_UDP,
	C_TCP,
	C_AGENT
};

typedef struct {
	u_int32_t handle;
	u_int32_t callback_id;
	u_int32_t dst_ip;
	u_int16_t dst_port;
	u_int16_t callback_port;
	u_int32_t callback_ip;
} hello_q_t;

#define	MAX_FAILED_RECORD	10

typedef struct {
	u_int32_t begin;
	u_int32_t duration;
	char reson[64];
} failed_record_t;

#define	NUM_STAT_POOL	6
    
typedef struct {
    struct stlc_list_head link;
    vtap_t tap_info;
}vtap_record_item_t;

typedef struct {
	// 用户点鼠标开始观看时间，time()获取的。并不到一定成功看到图像
	u_int32_t begin;
	// 本次开始成功看到视频的时间，time()获取。如果为0表示本次还没开始
	u_int32_t this_begin;
	// 本次除外的累计观看时间，单位秒。
	u_int32_t prev_see;
	// 图象分辨率，以显示为准
	u_int16_t width;
	u_int16_t height;
	
	u_int32_t dev_ip;
	u_int16_t dev_port;
	u_int16_t is_tcp;

	// 观看视频的客户端数目
	int client_count;

	/* 声音一些信息 */
	u_int8_t sound_format;
	// 声道数
	u_int8_t sound_channel;
	// 位数
	u_int8_t sound_bit;
	// 采样率， VOICE_RETE_xxx
	u_int8_t sample;
    /*视频饱和度相关信息*/
    int32_t brightness_val;//亮度
	u_int32_t contrast_val; //对比度
	u_int32_t saturation_val; //饱和度
	u_int32_t gain_val; //补偿
    /*云台转速*/
    u_int8_t roll_speed;
    u_int8_t pad[3];
	
	// 累计收到的字节数
	u_int64_t bytes;
	// 统计两种帧的长度
	u_int64_t bytes_i_frame;
	u_int64_t bytes_p_frame;
	u_int32_t max_i_frame;
	u_int32_t min_i_frame;
	u_int32_t max_p_frame;
	u_int32_t min_p_frame;

	// 5秒循环统计
	int sec_pos;
	// 第一次统计时间和最后一次统计时间
	time_t first_stat;
	time_t last_stat;
	u_int32_t sec_bytes[NUM_STAT_POOL];
	u_int32_t sec_frames[NUM_STAT_POOL];
	u_int32_t sec_fps;
	double sec_rate;
	
	// 累计收到的帧数
	u_int32_t frames;
	// 其中I帧个数
	u_int32_t i_frames;
	// 传输累计丢的帧数
	u_int32_t drop_frames;
	// 收到过期碎片
	u_int32_t old_frags;
	// 失败重连次数
	u_int32_t retry;
	// 成功看起视频的次数
	u_int32_t see_count;

	// 失败记录，环形缓冲区。写从fr_write写起。
	/// 读: 如果see_count<MAX_FAILED_RECORD， 从0开始，否则从 fr_write
	int fr_write;
	failed_record_t failed[MAX_FAILED_RECORD];
	// 本次失败原因
	char reson[64];
} video_stat_t;

typedef struct _video_s {
	// 回指指针
	slave_t *slave;

	// 不同于slave->handle
	cl_handle_t handle;

	cl_callback_t callback;
	void *callback_handle;
	
	// 观看视频时候的状态变迁。考虑到一个摄像头可能看视频，也可以看录像，后面应该多实例化
	// TODO: 多实例化
	int status;
	video_stat_t stat;
	video_try_t vtry;
	net_video_syn_a_t m_Syna;
	// 发送了多少个
	int hello_req_count;
	hello_q_t hello_q;
	// 控制TCP视频的
	cl_thread_t *t_tcp_read;
	cl_thread_t *t_tcp_write;
	cl_thread_t *t_udp;
	// 定时发送hello
	cl_thread_t *t_hello;
	cl_thread_t *t_timer;
	cl_thread_t *t_nack;
    // 视频饱和度、云台等查询timer
    cl_thread_t *t_vs_timer;

	// 本次是否是快速查询
	bool quick_query;
	// 查询定时器等信息
	cl_thread_t *t_query;
	// 定时器配置
	net_record_timer_config_t *timer;

	// 快速推出的第一帧图片
	int static_pic_size;
	int static_pic_type;
	void *static_pic; /* static_pic_a_t */
	
	// TCP发送队列
	struct stlc_list_head tcp_pkt_list;
	// 首报文发送了多少字节
	int tcp_send_len;
	
	// 转动云台的定时器
	cl_thread_t *t_roll;
	// 连续发送了多少个旋转命令了
	int num_roll_send;
	int roll_type;
	int roll_left_right;
	int roll_up_down;

	bool is_vtap;
	bool is_h264;
	// 是否是aes加密的
	bool is_aes_enc;
	int has_audio;
	char *last_err;

	// 查询回来的画质
	u_int8_t is_custom_quality; /* 是否是自定义的画质 */
	u_int8_t quality; /* 画质等级: QUALITY_XXX */
	// 图片报文中附带的质量
	u_int8_t pic_quality;
	u_int8_t pad;

	cl_thread_info_t ti_h264_decode;
	void *h264_handle;

	u_int32_t last_recv_ip;
	u_int16_t last_recv_port;
	int connect_type;
	
	// 是处于视频打开观看状态吗
	bool is_open;
	// 是否打开成功。以收到第一帧图片为准
	bool is_open_success;
	// 是否录像
	bool is_recoding;

	buff_t *tcp_buff;
	pic_ring_t *pic_ring; /* 组装图片分片用的 */
	data_exchg_t *data_pic; /* 组装解码出来的图片缓冲区 */
	data_exchg_t *data_h264; /* 原始的H264数据 */

	pic_ring_t *sound_ring; /* 组装声音分片用的 */
	data_exchg_t *data_sound; /* 组装解码出来的声音缓冲区 */
	buff_t *buf_sound_recv_raw; /* 从设备收到的原始数据，已解密，未解码 */
	buff_t *buf_sound_send_remain; /* 上次发送给设备的残留数据，未编码，未加密 */
	u_int16_t sound_seq; /* 发送给设备端的声音，不过设备端忽略该序号，问题不大 */
	u_int32_t timestamp_base;
	

	// 网络状况探测
	// 下一次发送CMD_NETWORK_DETECT_Q查询报文的时间, 单位毫秒。使用GetTickCount, 避免系统时间调整带来问题。
	cl_thread_t *t_net_detect;
	// 下一次探测哪个
	u_int32_t pos_next_query;
	// 保存最后一次列表，用于判断该从设备网络情况是否改变
	video_ip_list_t *last_video_ip_list;
	// 每一对IP:PORT做一个探测。net_detect_t
	struct stlc_list_head net_detect_list;

	// 探测成功的IP:PORT在数组m_arNetDetect中的下标。如果小于0，表示没有可用的
	int slect_p2p;
    
    //录像相关数据
    //是否是预取数据回包
    bool is_pre_get_vtap;
    //当前查询时间缓存
    u_int32_t cur_vtap_query_begin_time;
    u_int32_t cur_vtap_query_end_time;
    //当前看录像id缓存
    u_int8_t cur_vtap_client_id;
    //当前看到哪段录像
    u_int32_t cur_vtap_begin_time;
    //当前看到哪段录像的哪个时间
    u_int16_t cur_want_offset;
    u_int16_t cur_vtap_time_offset;
    //当前看的录像总长度
    u_int16_t cur_vtap_duration;
    
	// 录像列表, cl_vtap_t
	struct stlc_list_head vtap_list;
} video_t;

// cl_video_stat_t 中的resv部分
typedef struct {
	/* 连接方式 */
	u_int8_t is_tcp; /* tcp or udp */
	u_int8_t pad1;
	u_int16_t dev_port;	
	u_int32_t dev_ip;
} video_stat_priv_t;

extern void video_off_all_thread(video_t *video, bool off_net_detect);
extern void video_free_tcp_send(video_t *video);
extern void video_update_stat_pos(video_t *video);
extern void video_update_stat_bytes(video_t *video, int bytes);
extern void video_update_stat_frames(video_t *video);
extern void video_cal_sec_stat(video_t *video);

extern bool video_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
extern bool video_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool video_send_nack(video_t *video, u_int16_t idx, u_int16_t seq);
extern video_t *video_alloc(slave_t *slave);
extern void video_free(video_t *video);
extern bool video_send(video_t *video, pkt_t *pkt);
extern void callback_video_request(u_int32_t result, void *none, void *waitp);


#ifdef __cplusplus
}
#endif 

#endif

