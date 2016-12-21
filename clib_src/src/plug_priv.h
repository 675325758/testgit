#ifndef	 __PLUG_PRIV_H__
#define	__PLUG_PRIV_H__

#include "cl_plug.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define	DFL_PLUG_TIMER_Q_BASE	5
#define	DFL_PLUG_TIMER_Q_STAT	5
#define	DFL_PLUG_TIMER_Q_TIMER	10

typedef struct _plug_s {
	// 回指指针
	slave_t *slave;

	// 不同于slave->handle
	cl_handle_t handle;

	// 本次是否是快速查询
	bool quick_query;

	// 查询基础信息，包括开关、电流、电压、温度
	int timer_q_base;
	int count_q_base;
	// 查询电量总量和阶段电量
	int timer_q_stat;
	int count_q_stat;
	// 查询定时开关的配置，这个可以较长时间查询一次
	int timer_q_timer;
	int count_q_timer;

	cl_callback_t callback;
	void *callback_handle;

	// 查询回来的结果
	bool on; /* 开关 */
	int ac; /* 电流，单位: 0.1A */
	int v; /* 电压，单位: V  */
	int t; /* 温度, 单位: C */

	u_int32_t electric_stat_total; /*从开机到现在的总电量,单位 W*/
	u_int32_t electric_stat_section; /*从time指定时文到当前的阶段电量,单位 W*/
	u_int32_t section_time; /*指定的某一时刻,好用以记算到当前时间累积的电量*/	

	/* 定时开关策略 */
	net_plug_timer_config_t *timer;

	cl_thread_t *t_query;
} plug_t;

#define	HANDLE_QUERY_PLUG 0x11330000


extern void plug_quick_query(plug_t *plug);
extern bool plug_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern plug_t *plug_alloc(slave_t *slave);
extern void plug_free(plug_t *plug);
extern bool plug_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);


#ifdef __cplusplus
}
#endif 

#endif

