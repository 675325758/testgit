
#ifndef	__CL_IOEXT_H__
#define	__CL_IOEXT_H__

#ifdef __cplusplus
extern "C" {
#endif 

/*
	fd - which to wait
	wait_read - wait read or write
	timeout - how many seconds to wait
*/
extern RS wait_io_ready(SOCKET fd, bool wait_read, bool wait_write, int timeout);

/*
	带timeout功能的TCP连接
	ip和port都是主机序
*/
extern SOCKET connect_tmo(u_int32_t ip, int port, int timeout);

/*
	带timeout功能的读socket
*/
extern int recv_tmo(SOCKET fd, char *buf, int to_read, int timeout);

/*
	带timeout功能的写socket
*/
extern int send_tmo(SOCKET fd, const char *buf, int to_write, int timeout);


#ifdef __cplusplus
}
#endif 


#endif


