#ifndef	__WAIT_SERVER_H__
#define	__WAIT_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_priv.h"

#define	SERVER_WAIT_TIMEOUT	10

typedef struct {
	struct stlc_list_head link;

	// 超时删除的时间
	u_int32_t timeout;
	
	u_int32_t server_handle;
	
	int obj_type;
	cl_handle_t obj_handle;
	int cmd;
	void *param;
	// 回调时候，参数event为CMD_OK/CMD_FAIL，user_handle为空，callback_handle为 (wait_t *)
	cl_callback_t callback;
} wait_t;

extern u_int32_t wait_add(int handle_type, cl_handle_t handle, int cmd, void *param, cl_callback_t callback);
extern wait_t *wait_lookup(u_int32_t server_handle);
extern void wait_del(u_int32_t server_handle);
extern int wait_check_timeout(cl_thread_t *t);
extern void wait_callback(wait_t *w, int cmd, bool del);


#ifdef __cplusplus
}
#endif 

#endif


