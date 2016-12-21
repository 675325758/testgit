#ifndef	__EB_PRIV_H__
#define	__EB_PRIV_H__


#define	IA_EB_WORK		IA_MK(IA_TYPE_EB, 1)
#define	IA_EB_TIMER_SET	IA_MK(IA_TYPE_EB, 2)
#define	IA_EB_TIMER_DEL	IA_MK(IA_TYPE_EB, 3)
#define	IA_EB_PERIOD_TIMER_SET	IA_MK(IA_TYPE_EB, 4)
#define	IA_EB_PERIOD_TIMER_DEL	IA_MK(IA_TYPE_EB, 5)
#define	IA_EB_PT_ADJ_SET	IA_MK(IA_TYPE_EB, 6)
#define	IA_EB_PT_ADJ_GET	IA_MK(IA_TYPE_EB, 7)
#define	IA_EB_PT_ADJ_DEL	IA_MK(IA_TYPE_EB, 8)
#define	IA_EB_LED_CTRL	IA_MK(IA_TYPE_EB, 9)

/*
	处理设备端过来的通知
*/
extern bool eb_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

/*
	处理APP下来的用户请求
*/
extern bool eb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);

// APP能看见的状态数据
extern void eb_build_objs(user_t* user, cl_dev_info_t* ui);

extern int eb_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

extern void eb_quick_query_info(smart_air_ctrl_t* ac);

#endif

