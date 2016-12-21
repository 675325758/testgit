#include "client_lib.h"
#include "cl_priv.h"
#include "ioext.h"


/*
	fd - which to wait
	wait_read - wait read or write
	timeout - how many seconds to wait
*/
RS wait_io_ready(SOCKET fd, bool wait_read, bool wait_write, int timeout)
{
	int	n;
	fd_set	rfds, wfds;
	unsigned int	begin, now;
	struct timeval	tv;

	now = begin = get_sec();

	while (true) {
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		if (wait_read)
			FD_SET(fd, &rfds);
		if (wait_write)
			FD_SET(fd, &wfds);
		
		memset(&tv, 0, sizeof(tv));
		tv.tv_sec = timeout;

		n = select((int)fd + 1, &rfds, &wfds, NULL, &tv);
		
		if (n > 0) { /* if ok */
			break;
		} else if (n == 0) { /* if timeout */
			return RS_ERROR;
		} else {
			if ( ! NORMAL_ERRNO(GET_SOCK_ECODE()))
				return RS_ERROR;
			now = get_sec();
			timeout -= (now - begin);
			begin = now;
			if (timeout < 0)
				return RS_ERROR;
		}
	}

	return RS_OK;
}

/*
	带timeout功能的TCP连接
	ip和port都是主机序
*/
SOCKET connect_tmo(u_int32_t ip, int port, int timeout)
{
	SOCKET fd;
	bool inprogress;
	int ret, slen, status;
	
	fd = connect_tcp(ip, port, &inprogress);
	if (fd == INVALID_SOCKET)
		return INVALID_SOCKET;

	if (wait_io_ready(fd, true, true, timeout) != RS_OK) {
		log_err(false, "connect_tmo %u.%u.%u.%u port %u timeout\n", IP_SHOW(ip), port);
		goto err;
	}

	slen = sizeof(status);
	ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&status, &slen);
	if (ret < 0) {
		log_err(true, "connect_tmo %u.%u.%u.%u port %u failed\n", IP_SHOW(ip), port);
		goto err;
	}
	if (status != 0) {
		log_err(false, "connect_tmo %u.%u.%u.%u port %u failed: status=%d\n", IP_SHOW(ip), port, status);
		goto err;
	}

	return fd;

err:
	CLOSE_SOCK(fd);
	return INVALID_SOCKET;
}

/*
	带timeout功能的读socket
*/
int recv_tmo(SOCKET fd, char *buf, int to_read, int timeout)
{
	if (wait_io_ready(fd, true, false, timeout) != RS_OK) {
		log_err(false, "recv_tmo timeout. timeout=%u, to_read=%d\n", timeout, to_read);
		return -1;
	}

	return recv(fd, buf, to_read, 0);
}

/*
	带timeout功能的写socket
*/
int send_tmo(SOCKET fd, const char *buf, int to_write, int timeout)
{
	if (wait_io_ready(fd, false, true, timeout) != RS_OK) {
		log_err(false, "send_tmo timeout. timeout=%u, to_write=%d\n", timeout, to_write);
		return -1;
	}

	return send(fd, (char *)buf, to_write, 0);
}

