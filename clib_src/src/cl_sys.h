#ifndef	__CL_SYS_H__
#define	__CL_SYS_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

/***********************************************************

	跟平台相关的一些函数，实现在cl_win.c/cl_linux.c
	
************************************************************/


#ifdef WIN32

#include <Windows.h>
//#include <Winsock2.h>
#include <direct.h>
#include <io.h>

#define	inline __forceinline

typedef HANDLE cl_thread_hadle_t;
typedef HANDLE cl_mutex_t;

#define	EINPROGRESS	WSAEWOULDBLOCK
#define	EINTR	WSAEINTR
#define	EWOULDBLOCK	WSAEWOULDBLOCK
#define	EAGAIN	WSAEWOULDBLOCK

#define	VALID_SOCK(sock)	((sock) != INVALID_SOCKET && (sock) != 0)
#define socklen_t int

#define	SET_SOCK_NBLOCK(sock) \
	do { \
		unsigned long on = 1; \
		ioctlsocket((sock), FIONBIO, &on); \
	} while (0)
	
#define	CLOSE_SOCK(sock)		\
	do { \
		if (VALID_SOCK(sock)) { \
			closesocket(sock); \
			sock = INVALID_SOCKET; \
		} \
	} while (0)

#define	GET_SOCK_ECODE()	WSAGetLastError()

#define atoll _atoi64
#define	strdup _strdup
#define strcasecmp stricmp
#define	MKDIR(__dir, __mask)	_mkdir(__dir)
#define RMDIR(__dir)	_rmdir(__dir)
#define	localtime_r(__t, __tm) localtime_s(__tm, __t)
#define	gmtime_r(__t, __tm) gmtime_s(__tm, __t)
#define cl_access(f,m) _access(f,m)
        
#define	MY_THREAD_ID	GetCurrentThreadId()
#define	msleep(ms) Sleep(ms)
extern FILE *cl_fopen(const char *fn, const char *mode);
extern int gettimeofday(struct timeval *tp, void *tzp);

#else /* below is LINUX */

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/ioctl.h>

#define SOCKET int
#define	INVALID_SOCKET	-1

#define	VALID_SOCK(sock)	((sock) != INVALID_SOCKET && (sock) != 0)

#define	SET_SOCK_NBLOCK(sock) \
	do { \
		int on = 1; \
		ioctl((sock), FIONBIO, (void *)&on); \
	} while (0)

#define	CLOSE_SOCK(fd)		\
	do { \
		if (VALID_SOCK(fd)) { \
			close(fd); \
			fd = INVALID_SOCKET; \
		} \
	} while (0)

#define	GET_SOCK_ECODE()	errno

typedef pthread_t cl_thread_hadle_t;
typedef pthread_mutex_t * cl_mutex_t;

#define	MY_THREAD_ID	pthread_self()
#define	msleep(ms) usleep((ms)*1000)
#define	cl_fopen fopen
#define	MKDIR(__dir, __mask)	mkdir(__dir, __mask)
#define RMDIR(__dir)	rmdir(__dir)
#define cl_access(f,m) access(f,m)

#endif /* end WIN32 */
    
#ifdef IOS_COMPILE
// for mkdir
#include <sys/stat.h>
#endif

extern  u_int32_t calc_begin_time_of_day(u_int32_t time);
extern double get_dtime();
extern u_int32_t get_sec();
extern u_int32_t get_msec();
extern u_int32_t get_usec();
extern void ansi_to_utf8(const char *str, char *out);
extern void utf8_to_ansi(const char *str, char *out);
extern RS cl_init_lock(cl_mutex_t *mutex);
extern void cl_destroy_lock(cl_mutex_t *mutex);
extern RS cl_lock(cl_mutex_t *mutex);
extern RS cl_unlock(cl_mutex_t *mutex);


#ifdef __cplusplus
}
#endif 

#endif

