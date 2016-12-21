#ifndef	__AREA_PRIV_H__
#define	__AREA_PRIV_H__

#include "cl_area.h"
#include "cl_priv.h"

enum  {
    AC_AREA_QUERY_ALL = 0x0, //查询所有区域
    AC_AREA_MODIFY = 0x1, //区域添加或修改
    AC_AREA_DELETE = 0x2, //区域删除
    AC_AREA_QUERY_SINGLE =0x3, //查询单个区域
    AC_AREA_MODIFY_SINGLE = 0x4
};

typedef struct{
    struct stlc_list_head link;//自身链表
    cl_handle_t area_handle;
    u_int32_t create_time;
}area_req_store_t;

typedef struct _area_ctrl_s {
	// 回指指针
	user_t *user;
	// 查询定时器
	cl_thread_t *t_query;
    
    u_int32_t query_time_interval;
    
	int prev_reply_len;
	// 缓存上一次查询的结果，用来判断本次查询结果是否改变
	void *prev_reply;
    
	// 区域链表
	struct stlc_list_head area_list;
    //区域添加请求
    struct stlc_list_head area_req_list;
} area_ctrl_t;

typedef struct {
	struct stlc_list_head link;//自身链表
    area_ctrl_t * ac; /*回指指针*/
    u_int32_t query_time_interval; //查询时间间隔
    area_t area_info;
    cl_handle_t area_handle;//区域句柄
    u_int8_t area_id;      //区域ID
    u_int16_t dev_obj_num;
    u_int16_t eq_obj_num;
    int prev_reply_len;    //上次区域查询信息长度
	void *prev_reply;      //上次查询信息回应
    cl_thread_t *t_query;  //单个区域查询定时器
} area_priv_t;

extern bool area_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool area_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern RS area_ctrl_alloc(user_t* user);//分配内存
extern void area_ctrl_free(user_t* user);//释放内存
extern void area_build_objs(user_t* user,cl_dev_info_t* ui);//查询
extern cl_handle_t area_get_handle_by_id(user_t* user,u_int8_t area_id);

#endif

