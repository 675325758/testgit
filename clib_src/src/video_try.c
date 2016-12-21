#include "client_lib.h"
#include "cl_priv.h"
#include "cl_notify.h"
#include "video_try.h"
#include "video_priv.h"

void vt_init(video_try_t *vt)
{
	vt->local_port = 0;
	vt->sock_udp = INVALID_SOCKET;
	vt->sock_tcp = INVALID_SOCKET;
	STLC_INIT_LIST_HEAD(&vt->try_list);
	
	vt_reset(vt, true);
}

void vt_destroy(video_try_t *vt)
{
	CLOSE_SOCK(vt->sock_udp);
	CLOSE_SOCK(vt->sock_tcp);
	stlc_list_free(video_try_one_t, &vt->try_list, link);
}

/*
	添加一个尝试或者延时
*/
bool vt_add_try(video_try_t *vt, bool bTcp, u_int32_t ip, int port, int timeout)
{
	video_try_one_t *op;

	op = cl_calloc(sizeof(video_try_one_t), 1);
	op->is_tcp = bTcp;
	op->ip = ip;
	op->port = port;
	op->timeout = timeout;

	stlc_list_add_tail(&op->link, &vt->try_list);

	return true;
}

/*
	单位: ms
*/
bool vt_add_timeout(video_try_t *vt, int delay)
{
	return vt_add_try(vt, false, 0, 0, delay);
}

int vt_count(video_try_t *vt)
{
	int count;
	
	stlc_list_count(count, &vt->try_list);

	return count;
}


/*
	检查是否超时，超时返回TRUE
*/
bool vt_check_timer(video_try_t *vt)
{
	int count = 0;
	video_try_one_t *op;

	stlc_list_count(count, &vt->try_list);
	
	if (vt->try_idx < 0) {
		vt->try_idx = 0;
		if (count > 0) {
			stlc_list_by_index(video_try_one_t, op, &vt->try_list, link, vt->try_idx);
			op->begin = get_msec();
		}
	}
	
	if (vt->try_idx >= count)
		return false;

	stlc_list_by_index(video_try_one_t, op, &vt->try_list, link, vt->try_idx);
	return u32_is_bigger(get_msec(), op->begin + op->timeout);
}

/*
	修改当前的超时时间
*/
bool vt_set_die(video_try_t *vt, int timeout)
{
	video_try_one_t *op;

	if (vt->try_idx < 0 || vt->try_idx >= vt_count(vt))
		return false;

	stlc_list_by_index(video_try_one_t, op, &vt->try_list, link, vt->try_idx);
	op->timeout = timeout;

	return true;
}

/*
	用于更新时间，避免超时
*/
bool vt_update_die(video_try_t *vt)
{
	video_try_one_t *op;
	
	if (vt->try_idx < 0 || vt->try_idx >= vt_count(vt))
		return false;

	stlc_list_by_index(video_try_one_t, op, &vt->try_list, link, vt->try_idx);
	op->begin = get_msec();

	return true;
}

/*
	复位并准备重新开始
*/
RS vt_reset(video_try_t *vt, bool bResetAgent)
{
	int addr_len;
	struct sockaddr_in addr;
	video_t *v = vt->video;
	
	vt_reset_try(vt);
	
	if (bResetAgent)
		vt->is_agent = false;

	CL_THREAD_OFF(v->t_udp);
	CL_THREAD_OFF(v->t_tcp_read);
	CL_THREAD_OFF(v->t_tcp_write);
	video_free_tcp_send(v);
	
	CLOSE_SOCK(vt->sock_udp);
	CLOSE_SOCK(vt->sock_tcp);

	vt->sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if ( ! VALID_SOCK(vt->sock_udp)) {
		log_err(true, "create udp socket failed\n");
		return RS_ERROR;
	}

	addr_len = sizeof(addr);
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(vt->sock_udp, (struct sockaddr *)&addr, addr_len) != 0) {
		log_err(true, "bind error");
	}
	
	if (getsockname(vt->sock_udp, (struct sockaddr *)&addr, &addr_len) != 0) {
		log_err(true, "getsockname failed\n");
		vt->local_port = 0;
		return RS_ERROR;
	}
	vt->local_port = ntohs(addr.sin_port);

	return RS_OK;
}

/*
	仅仅清空尝试的数组
*/
void vt_reset_try( video_try_t *vt)
{
	stlc_list_free(video_try_one_t, &vt->try_list, link);

	vt->try_idx = -1;
}

/*
	移动到下一个尝试
	返回:
		true: 还有下一个
		false: 到头了
*/
bool vt_move_next_try(video_try_t *vt)
{
	video_try_one_t *op;
	
	if (vt->try_idx >= 0 && VALID_SOCK(vt->sock_tcp)) {
		CLOSE_SOCK(vt->sock_tcp);
		vt->is_connect_ok = false;
		stlc_list_by_index(video_try_one_t, op, &vt->try_list, link, vt->try_idx);
		// ...
	}

	vt->try_idx++;

	return (vt->try_idx < vt_count(vt));
}

video_try_one_t *vt_get_this_try(video_try_t *vt)
{
	video_try_one_t *op;

	if (vt->try_idx < 0 || vt->try_idx >= vt_count(vt)) {
		log_debug("GetThisTry failed: m_nTry=%d, array count=%d\n", vt->try_idx, vt_count(vt));
		return NULL;
	}

	stlc_list_by_index(video_try_one_t, op, &vt->try_list, link, vt->try_idx);

	return op;
}

// 返回当前尝试还剩下多少秒超时
int vt_get_timeout(video_try_t *vt)
{
	u_int32_t timeout;
	u_int32_t now;
	video_try_one_t *op;

	if ((op = vt_get_this_try(vt)) == NULL)
		return 0;

	now = get_msec();
	timeout = op->begin + op->timeout;

	if (u32_is_bigger(now, timeout))
		return 0;
	
	return (timeout - now) / 1000;
}


