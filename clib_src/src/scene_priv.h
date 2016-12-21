#ifndef	__SCENE_PRIV_H__
#define	__SCENE_PRIV_H__

#include "cl_scene.h"
#include "cl_priv.h"


enum  {
    AC_SCENE_QUERY_ALL = 0x0,
    AC_SCENE_MODIFY = 0x1,
    AC_SCENE_DELETE = 0x2,
    AC_SCENE_QUERY_SINGLE= 0x3,
    AC_SCENE_EXEC = 0x9
};

#define SCENE_DEV_ACTION_ON     (0x0)
#define SCENE_DEV_ACTION_OFF    (0x1)

typedef struct{
    struct stlc_list_head link;//自身链表
    cl_handle_t scene_handle;
    u_int32_t create_time;
}scene_req_store_t;

typedef struct _scent_ctrl_s {
    user_t *user;
	// 查询定时器
	cl_thread_t *t_query;
    
	int prev_reply_len;
	// 缓存上一次查询的结果，用来判断本次查询结果是否改变
	void *prev_reply;
    
    u_int32_t query_time_interval;
	// 情景模式链表
	struct stlc_list_head scene_list;
    
    struct stlc_list_head scene_req_list;
} scene_ctrl_t;

// scene_t->flags
#define FLAG_SCENE_LAST_ID			0x1
#define FLAG_SCENE_EXECUTED			0x2
#define FLAG_SCENE_HAVE_EVENTS		0x4

typedef struct {
	struct stlc_list_head link;//自身链表
    
    cl_handle_t scene_handle;//情景模式句柄
    scene_t scene_info;
    int prev_reply_len;    //上次查询信息长度
	void *prev_reply;      //上次查询信息回应

	int prev_timer_len; /* 保存上一次查询的情景定时器 */
	void *prev_timer;
	
    cl_thread_t *t_query;  //单个情景模式查询定时器
    scene_ctrl_t * sc; /*回指指针*/
    u_int32_t query_time_interval; //查询时间间隔
    u_int8_t event_count;
    u_int8_t scene_id;      //ID
} scene_priv_t;

extern bool scene_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool scene_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern RS scene_ctrl_alloc(user_t* user);//分配内存
extern void scene_ctrl_free(user_t* user);//释放内存
extern void scene_build_objs(user_t* user,cl_dev_info_t* ui);//查询
extern void scene_free_timer(cl_scene_t* scene);
extern void quick_query_alarm_link_scene(user_t *user);
extern scene_priv_t* scene_lookup_by_id(scene_ctrl_t* sc,u_int8_t scene_id);
RS scene_proc_exec(cl_handle_t s_handle);

#endif

