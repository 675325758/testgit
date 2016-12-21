#include "client_lib.h"
#include "cl_priv.h"
#include "cl_notify.h"

/*
	timeout:
		0: 立即返回
		< 0: 一直等待
		> 0; 等待毫秒数
*/
#if 0
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif
RS cl_recv_sock(SOCKET sock, char *buf, int *size, int timeout, struct sockaddr_in *from)
{
	int n, addr_len = sizeof(struct sockaddr_in);
	fd_set rfds;
	struct timeval tv, *tvp;
	
	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	if (timeout < 0) {
		tvp = NULL;
	} else {
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout % 1000)*1000;

		tvp = &tv;
	}

	n = select((int)sock + 1, &rfds, NULL, NULL, tvp);
	if (n <= 0)
		return RS_ERROR;

	n = recvfrom(sock, buf, *size, 0, (struct sockaddr *)from, &addr_len);
	if (n <= 0) {
		log_err(true, "cl_recv_sock recv failed\n");
		return RS_ERROR;
	}
	
	if (n > 0)
		*size = n;

	return RS_OK;
}

cl_notify_pkt_t *cl_recv_notify(cl_thread_info_t *ti, int timeout, struct sockaddr_in *from)
{
	int size = ti->udp_buf_size;
	
	if (cl_recv_sock(ti->sock_notify, (char *)ti->udp_buf, &size, timeout, from) == RS_OK)
		return ti->udp_buf;

	return NULL;
}


/*
	如果是不需要应答的，直接发送后返回
	如果是需要应答的，检查自己是不是就是接收事件的线程，如果是，直接处理，否则发送后等待结果
*/
RS cl_send_notify(cl_thread_info_t *ti, cl_notify_pkt_t *request)
{
	int total = request->hdr_len + request->param_len;
	int n, reply_size;
	SOCKET sock;
	RS ret = RS_OK;
	cl_notify_pkt_t reply;
	struct sockaddr_in from;
	int nSendBuf = total + 100;

	if ((request->flags & CLNPF_ACK) != 0 && MY_THREAD_ID == ti->tid) {
		ret = ti->proc_notify(request);
		return ret;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		log_err(true, "cl_send_notify create udp socket failed\n");
		return RS_ERROR;
	}

	setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
	
	memset(&reply, 0, sizeof(cl_notify_pkt_t));

	while (1) {
		n = (int)sendto(sock, (char *)request, total, 0, (struct sockaddr *)&ti->addr_notify, sizeof(struct sockaddr));
		if (n != total) {
			log_err(true, "send notify failed: want %d, but send %d\n", total, n);
			ret = RS_ERROR;
			goto done;
		}

		log_debug("send notify to thread %s: type=%d, request id=%u, len=%d, ret=%d\n",
			ti->name, request->type, request->request_id, total, n);

		if ((request->flags & CLNPF_ACK) == 0)
			break;

		reply_size = sizeof(cl_notify_pkt_t);

		if (cl_recv_sock(sock, (char *)&reply, &reply_size, 10000, &from) == RS_OK && reply.type == request->type) {
			log_info("send request type %d, recv reply %d\n", request->type, reply.err_code);
			ret = reply.err_code;
			break;
		}
	}

done:
	CLOSE_SOCK(sock);
	return ret;
}

int cl_send_notify_wait(cl_thread_info_t *ti, cl_notify_pkt_t *request, int request_buf_size)
{
	int total = request->hdr_len + request->param_len;
	int n, reply_size = -1;
	SOCKET sock;
	struct sockaddr_in from;

	if ((request->flags & CLNPF_ACK) != 0 && MY_THREAD_ID == ti->tid) {
		cl_assert(0);
		return RS_ERROR;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		log_err(true, "cl_send_notify create udp socket failed\n");
		return RS_ERROR;
	}
	
	n = sendto(sock, (char *)request, total, 0, (struct sockaddr *)&ti->addr_notify, sizeof(struct sockaddr));
	if (n != total) {
		log_err(true, "send notify failed: want %d, but send %d\n", total, n);
		goto done;
	}

	log_debug("send notify wait to thread %s: type=%d, request id=%u, len=%d, ret=%d\n",
		ti->name, request->type, request->request_id, total, n);

	reply_size = request_buf_size;
	// 象域名解析这种要等很久，多等点
	if (cl_recv_sock(sock, (char *)request, &reply_size, 20000, &from) != RS_OK) {
		reply_size = -1;
	}

done:
	CLOSE_SOCK(sock);

	return reply_size;
}

/* 
	type = CLNE_xxx
*/
RS cl_send_notify_simple(cl_thread_info_t *thread, u_int32_t type)
{
	cl_notify_pkt_t pkt;

	memset(&pkt, 0, sizeof(cl_notify_pkt_t));
	pkt.type = type;
	pkt.request_id = 0;
	pkt.hdr_len = sizeof(cl_notify_pkt_t);
	pkt.hdr_len = sizeof(cl_notify_pkt_t);

	return cl_send_notify(thread, &pkt);
}

RS cl_send_notify_common(cl_thread_info_t *thread, u_int32_t type,cln_common_info_t* ci)
{
	 cl_notify_pkt_t *pkt;
	cln_common_info_t *s;
	RS ret;
	int len = sizeof(*pkt) + sizeof(*ci);
    
	CL_CHECK_INIT;

	if(type == 0 || !thread || !ci){
		return RS_INVALID_PARAM;
	}
    
	pkt = cl_notify_pkt_new(len, type, CLNPF_ACK);
	pkt->param_len = sizeof(cln_common_info_t);
	s = (cln_common_info_t*)pkt->data;

	memcpy(s,ci,sizeof(*ci));
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

RS cl_send_simple_data_notify(cl_thread_info_t *thread, cl_handle_t handle,u_int32_t type,u_int16_t action,u_int64_t value,int val_size)
{
	cln_common_info_t ci = {0};
	
	if(!thread || type == 0 || val_size <= 0){
		return RS_INVALID_PARAM;
	}

	ci.action = action;
	ci.handle = handle;
	ci.param_count = 0x1;
	ci.single_param_len = val_size&0xffff;
	ci.data_len = val_size;

	switch(val_size){
		case sizeof(u_int64_t):
		{
			ci.u.u64_data = value;
		}
			break;
		case sizeof(u_int32_t):
		{
			ci.u.u32_data[0] = (u_int32_t)(value & 0xffffffff);
		}
			break;
		case sizeof(u_int16_t):
		{
			ci.u.u16_data[0] = (u_int16_t)(value & 0xffff);
		}
			break;
		case sizeof(u_int8_t):
		{
			ci.u.u8_data[0] = (u_int8_t)(value & 0xff);
		}
			break;
		default:
			return RS_INVALID_PARAM;
			break;
	}

	return cl_send_notify_common(thread,type,&ci);
}

RS cl_send_var_data_notify(cl_thread_info_t *thread, cl_handle_t handle,u_int32_t type,u_int16_t action,u_int8_t *value,int val_size)
{
	cl_notify_pkt_t *pkt;
	cln_common_info_t *s,temp;
	RS ret;
	int len = sizeof(cln_common_info_t);
	int pkt_len = 1024;
    
	CL_CHECK_INIT;

	if(type == 0 || !thread ){
		return RS_INVALID_PARAM;
	}

	if(val_size > (int)sizeof(temp.u)){
		len+=(val_size-(int)sizeof(temp.u));
		if(val_size > (pkt_len - (int)sizeof(*pkt))){
			pkt_len = ((val_size + sizeof(*pkt))+1023)/1024*1024;
		}
	}
	
	pkt = cl_notify_pkt_new(pkt_len, type, CLNPF_ACK);
	pkt->param_len = (u_int32_t)len;
	s = (cln_common_info_t*)pkt->data;
	s->action = action;
	s->handle = handle;
	s->param_count = 0x1;
	s->single_param_len =  val_size&0xffff;
	s->data_len = (u_int32_t)val_size;
	
	memcpy(&s->u.u8_data[0],value,val_size);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}


RS cl_la_send_var_data_notify(cl_thread_info_t *thread, cl_handle_t handle,u_int32_t type,u_int16_t action,u_int8_t *value,int val_size)
{
	cl_notify_pkt_t *pkt;
	cln_la_info_t *s;
	RS ret;
	int len = sizeof(cln_la_info_t);
	int pkt_len = 1024;
    
	CL_CHECK_INIT;

	if(type == 0 || !thread ){
		return RS_INVALID_PARAM;
	}

	while((val_size + 100) > pkt_len) {
		pkt_len *= 2;
	}

	
	pkt = cl_notify_pkt_new(pkt_len, type, CLNPF_ACK);
	pkt->param_len = (u_int32_t)len + (u_int32_t)val_size;
	s = (cln_la_info_t*)pkt->data;
	s->action = action;
	s->handle = handle;
	s->type = type;
	s->data_len = (u_int32_t)val_size;
	
	memcpy(&s->data[0],value,val_size);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}


RS cl_create_notify_sock(cl_thread_info_t *info)
{
	struct sockaddr_in addr;
	int addr_len;
	SOCKET sock;
	int nRecvBuf = MAX_UDP_PKT*40;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		log_err(true, "create udp socket failed\n");
		return RS_ERROR;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int)) != 0) {
		log_err(true, "setsockopt failed !!!!!!!!!!!!!!!!!!!!!\n");
	}
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP_LOOPBACK);
	if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0) {
		log_err(true, "bind notify socket failed\n");
		goto err;
	}

	addr_len = sizeof(info->addr_notify);
	if (getsockname(sock, (struct sockaddr *)&info->addr_notify, &addr_len) != 0) {
		log_err(true, "getsockname failed\n");
		goto err;
	}

	info->sock_notify = sock;

	//info->udp_buf_size = MAX_UDP_PKT;
	//这里因为快捷按键规则可能过大，这里改成最大256k。
	info->udp_buf_size = MAX_UDP_PKT*40;
	info->udp_buf = cl_malloc(info->udp_buf_size);
	
	return RS_OK;

err:
	CLOSE_SOCK(sock);
	return RS_ERROR;
}

cl_notify_pkt_t *cl_notify_pkt_new(int size, u_int32_t type, u_int16_t flags)
{
	cl_notify_pkt_t *pkt;

	if ((pkt = (cl_notify_pkt_t *)cl_malloc(size)) == NULL) {
		log_err(false, "cl_notify_pkt_new out of memory: size=%d, type=%u\n", size, type);
		return NULL;
	}

	memset(pkt, 0, size);
	pkt->type = type;
	pkt->flags = flags;
	if ((flags & CLNPF_ACK) != 0)
		pkt->request_id = cl_get_request_id();
	pkt->hdr_len = sizeof(cl_notify_pkt_t);
	pkt->param_len = size - sizeof(cl_notify_pkt_t);

	return pkt;
}

void cl_notify_pkt_free(cl_notify_pkt_t *pkt)
{
	cl_free(pkt);
}


