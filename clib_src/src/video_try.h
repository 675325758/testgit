#ifndef	__VIDEO_TRY_H__
#define	__VIDEO_TRY_H__

#ifdef __cplusplus
extern "C" {
#endif 



typedef struct {
	struct stlc_list_head link;
	bool is_tcp;
	u_int32_t ip;
	int port;
	// 尝试时超时时间，单位毫秒
	int timeout;
	// 开始时间
	u_int32_t begin;
} video_try_one_t;

typedef struct {
	// video_try_one_t
	struct stlc_list_head try_list;

	void *video;

	// 是否走了视频代理
	bool is_agent;
	
	// 本次尝试哪个
	int try_idx;

	// 视频用的UDP socket，跟设备通信的
	SOCKET sock_udp;
	// 视频用的TCP socket，跟设备通信的
	SOCKET sock_tcp;
	// tcp 连接是否成功
	bool is_connect_ok;
	u_int32_t local_port;
} video_try_t;

extern void vt_init(video_try_t *vt);
extern void vt_destroy(video_try_t *vt);
extern bool vt_add_try(video_try_t *vt, bool bTcp, u_int32_t ip, int port, int timeout);
extern bool vt_add_timeout(video_try_t *vt, int delay);
extern int vt_count(video_try_t *vt);
extern bool vt_check_timer(video_try_t *vt);
extern bool vt_set_die(video_try_t *vt, int timeout);
extern bool vt_update_die(video_try_t *vt);
extern RS vt_reset(video_try_t *vt, bool bResetAgent);
extern void vt_reset_try( video_try_t *vt);
extern bool vt_move_next_try(video_try_t *vt);
extern video_try_one_t *vt_get_this_try(video_try_t *vt);
extern int vt_get_timeout(video_try_t *vt);



#ifdef __cplusplus
}
#endif 

#endif

