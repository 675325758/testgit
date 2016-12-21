#ifndef	 __NOTIFY_PUSH_PRIV_H__
#define	__NOTIFY_PUSH_PRIV_H__

#include "cl_notify_push.h"
#include "notify_push_priv.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _user_cmt_event_s{
	struct stlc_list_head link;
	int event;
	int len_msg;
	void *msg;	
}user_cmt_event_t;

typedef struct _notify_push_s {
	u_int64_t expect_report_id;
    int is_user_has_set_id;
    int need_notify_user;
    u_int64_t android_push_id;
    
    cl_callback_t callback;
	void *callback_handle;
    
    alarm_msg_list_t* msglist;
} notify_push_t;

extern bool notify_push_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool notify_push_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern notify_push_t *notify_push_alloc(void);
extern void notify_push_free(notify_push_t *np);
extern void notify_push_proc_android_msg(user_t* user);

#ifdef __cplusplus
}
#endif 

#endif

