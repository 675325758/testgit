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
// ֻ����������˵
#define	AUDIO_ONEWAY		1
// �Խ�
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

//������������
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
	// �û�����꿪ʼ�ۿ�ʱ�䣬time()��ȡ�ġ�������һ���ɹ�����ͼ��
	u_int32_t begin;
	// ���ο�ʼ�ɹ�������Ƶ��ʱ�䣬time()��ȡ�����Ϊ0��ʾ���λ�û��ʼ
	u_int32_t this_begin;
	// ���γ�����ۼƹۿ�ʱ�䣬��λ�롣
	u_int32_t prev_see;
	// ͼ��ֱ��ʣ�����ʾΪ׼
	u_int16_t width;
	u_int16_t height;
	
	u_int32_t dev_ip;
	u_int16_t dev_port;
	u_int16_t is_tcp;

	// �ۿ���Ƶ�Ŀͻ�����Ŀ
	int client_count;

	/* ����һЩ��Ϣ */
	u_int8_t sound_format;
	// ������
	u_int8_t sound_channel;
	// λ��
	u_int8_t sound_bit;
	// �����ʣ� VOICE_RETE_xxx
	u_int8_t sample;
    /*��Ƶ���Ͷ������Ϣ*/
    int32_t brightness_val;//����
	u_int32_t contrast_val; //�Աȶ�
	u_int32_t saturation_val; //���Ͷ�
	u_int32_t gain_val; //����
    /*��̨ת��*/
    u_int8_t roll_speed;
    u_int8_t pad[3];
	
	// �ۼ��յ����ֽ���
	u_int64_t bytes;
	// ͳ������֡�ĳ���
	u_int64_t bytes_i_frame;
	u_int64_t bytes_p_frame;
	u_int32_t max_i_frame;
	u_int32_t min_i_frame;
	u_int32_t max_p_frame;
	u_int32_t min_p_frame;

	// 5��ѭ��ͳ��
	int sec_pos;
	// ��һ��ͳ��ʱ������һ��ͳ��ʱ��
	time_t first_stat;
	time_t last_stat;
	u_int32_t sec_bytes[NUM_STAT_POOL];
	u_int32_t sec_frames[NUM_STAT_POOL];
	u_int32_t sec_fps;
	double sec_rate;
	
	// �ۼ��յ���֡��
	u_int32_t frames;
	// ����I֡����
	u_int32_t i_frames;
	// �����ۼƶ���֡��
	u_int32_t drop_frames;
	// �յ�������Ƭ
	u_int32_t old_frags;
	// ʧ����������
	u_int32_t retry;
	// �ɹ�������Ƶ�Ĵ���
	u_int32_t see_count;

	// ʧ�ܼ�¼�����λ�������д��fr_writeд��
	/// ��: ���see_count<MAX_FAILED_RECORD�� ��0��ʼ������� fr_write
	int fr_write;
	failed_record_t failed[MAX_FAILED_RECORD];
	// ����ʧ��ԭ��
	char reson[64];
} video_stat_t;

typedef struct _video_s {
	// ��ָָ��
	slave_t *slave;

	// ��ͬ��slave->handle
	cl_handle_t handle;

	cl_callback_t callback;
	void *callback_handle;
	
	// �ۿ���Ƶʱ���״̬��Ǩ�����ǵ�һ������ͷ���ܿ���Ƶ��Ҳ���Կ�¼�񣬺���Ӧ�ö�ʵ����
	// TODO: ��ʵ����
	int status;
	video_stat_t stat;
	video_try_t vtry;
	net_video_syn_a_t m_Syna;
	// �����˶��ٸ�
	int hello_req_count;
	hello_q_t hello_q;
	// ����TCP��Ƶ��
	cl_thread_t *t_tcp_read;
	cl_thread_t *t_tcp_write;
	cl_thread_t *t_udp;
	// ��ʱ����hello
	cl_thread_t *t_hello;
	cl_thread_t *t_timer;
	cl_thread_t *t_nack;
    // ��Ƶ���Ͷȡ���̨�Ȳ�ѯtimer
    cl_thread_t *t_vs_timer;

	// �����Ƿ��ǿ��ٲ�ѯ
	bool quick_query;
	// ��ѯ��ʱ������Ϣ
	cl_thread_t *t_query;
	// ��ʱ������
	net_record_timer_config_t *timer;

	// �����Ƴ��ĵ�һ֡ͼƬ
	int static_pic_size;
	int static_pic_type;
	void *static_pic; /* static_pic_a_t */
	
	// TCP���Ͷ���
	struct stlc_list_head tcp_pkt_list;
	// �ױ��ķ����˶����ֽ�
	int tcp_send_len;
	
	// ת����̨�Ķ�ʱ��
	cl_thread_t *t_roll;
	// ���������˶��ٸ���ת������
	int num_roll_send;
	int roll_type;
	int roll_left_right;
	int roll_up_down;

	bool is_vtap;
	bool is_h264;
	// �Ƿ���aes���ܵ�
	bool is_aes_enc;
	int has_audio;
	char *last_err;

	// ��ѯ�����Ļ���
	u_int8_t is_custom_quality; /* �Ƿ����Զ���Ļ��� */
	u_int8_t quality; /* ���ʵȼ�: QUALITY_XXX */
	// ͼƬ�����и���������
	u_int8_t pic_quality;
	u_int8_t pad;

	cl_thread_info_t ti_h264_decode;
	void *h264_handle;

	u_int32_t last_recv_ip;
	u_int16_t last_recv_port;
	int connect_type;
	
	// �Ǵ�����Ƶ�򿪹ۿ�״̬��
	bool is_open;
	// �Ƿ�򿪳ɹ������յ���һ֡ͼƬΪ׼
	bool is_open_success;
	// �Ƿ�¼��
	bool is_recoding;

	buff_t *tcp_buff;
	pic_ring_t *pic_ring; /* ��װͼƬ��Ƭ�õ� */
	data_exchg_t *data_pic; /* ��װ���������ͼƬ������ */
	data_exchg_t *data_h264; /* ԭʼ��H264���� */

	pic_ring_t *sound_ring; /* ��װ������Ƭ�õ� */
	data_exchg_t *data_sound; /* ��װ������������������� */
	buff_t *buf_sound_recv_raw; /* ���豸�յ���ԭʼ���ݣ��ѽ��ܣ�δ���� */
	buff_t *buf_sound_send_remain; /* �ϴη��͸��豸�Ĳ������ݣ�δ���룬δ���� */
	u_int16_t sound_seq; /* ���͸��豸�˵������������豸�˺��Ը���ţ����ⲻ�� */
	u_int32_t timestamp_base;
	

	// ����״��̽��
	// ��һ�η���CMD_NETWORK_DETECT_Q��ѯ���ĵ�ʱ��, ��λ���롣ʹ��GetTickCount, ����ϵͳʱ������������⡣
	cl_thread_t *t_net_detect;
	// ��һ��̽���ĸ�
	u_int32_t pos_next_query;
	// �������һ���б������жϸô��豸��������Ƿ�ı�
	video_ip_list_t *last_video_ip_list;
	// ÿһ��IP:PORT��һ��̽�⡣net_detect_t
	struct stlc_list_head net_detect_list;

	// ̽��ɹ���IP:PORT������m_arNetDetect�е��±ꡣ���С��0����ʾû�п��õ�
	int slect_p2p;
    
    //¼���������
    //�Ƿ���Ԥȡ���ݻذ�
    bool is_pre_get_vtap;
    //��ǰ��ѯʱ�仺��
    u_int32_t cur_vtap_query_begin_time;
    u_int32_t cur_vtap_query_end_time;
    //��ǰ��¼��id����
    u_int8_t cur_vtap_client_id;
    //��ǰ�����Ķ�¼��
    u_int32_t cur_vtap_begin_time;
    //��ǰ�����Ķ�¼����ĸ�ʱ��
    u_int16_t cur_want_offset;
    u_int16_t cur_vtap_time_offset;
    //��ǰ����¼���ܳ���
    u_int16_t cur_vtap_duration;
    
	// ¼���б�, cl_vtap_t
	struct stlc_list_head vtap_list;
} video_t;

// cl_video_stat_t �е�resv����
typedef struct {
	/* ���ӷ�ʽ */
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
// ����; BOOL: �����˸ñ���. false: ��Ҫ����ģ�����������
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

