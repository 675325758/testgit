#include "wait_server.h"

#define	WAIT_HANDLE_MIN	0xFF000000
#define	WAIT_HANDLE_MAX	1000000

// 添加等待队列，返回句柄用于查找
u_int32_t wait_add(int handle_type, cl_handle_t handle, int cmd, void *param, cl_callback_t callback)
{
	wait_t *w;
	
	cl_priv->server_handle++;
	if (cl_priv->server_handle == 0 || cl_priv->server_handle >= WAIT_HANDLE_MAX)
		cl_priv->server_handle = 1;

	w = cl_calloc(sizeof(wait_t), 1);
	w->timeout = get_sec() + SERVER_WAIT_TIMEOUT;
	w->server_handle = WAIT_HANDLE_MIN + cl_priv->server_handle;
	w->obj_type = handle_type;
	w->obj_handle = handle;
	w->cmd = cmd;
	w->param = param;
	w->callback = callback;

	stlc_list_add_tail(&w->link, &cl_priv->wait_list);

	return w->server_handle;
}

wait_t *wait_lookup(u_int32_t server_handle)
{
	wait_t *w;
	
	stlc_list_for_each_entry(wait_t, w, &cl_priv->wait_list, link) {
		if (w->server_handle == server_handle)
			return w;
	}

	return (wait_t *)NULL;
}

void wait_del(u_int32_t server_handle)
{
	wait_t *w;
	
	if ((w = wait_lookup(server_handle)) != NULL) {
		stlc_list_del(&w->link);
		cl_free(w);
	}
}

int wait_check_timeout(cl_thread_t *t)
{
	wait_t *w, *next;
	u_int32_t now;

	cl_priv->t_check_wait = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, cl_priv->t_check_wait, wait_check_timeout, NULL, 1000);
	
	now = get_sec();
	
	stlc_list_for_each_entry_safe(wait_t, w, next, &cl_priv->wait_list, link) {
		if (u32_is_bigger(now, w->timeout)) {
			log_err(false, "wait server reply timeout: server handle=0x%08x, object handle type=%d, object handle=0x%08x, command=%u\n",
				w->server_handle, w->obj_type, w->obj_handle, w->cmd);
			stlc_list_del(&w->link);
			w->callback(CMD_FAIL, NULL, w);
			cl_free(w);
		}
	}

	return 0;
}

void wait_callback(wait_t *w, int cmd, bool del)
{
	w->callback(cmd, NULL, w);
	
	if (del) {
		stlc_list_del(&w->link);
		cl_free(w);
	}
}

