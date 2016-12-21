#ifndef	__CL_THREAD_H__
#define	__CL_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "stlc_list.h"

/* Master of the theads. */
typedef struct {
	struct stlc_list_head read;
	struct stlc_list_head write;
	struct stlc_list_head timer;
	struct stlc_list_head event;
	struct stlc_list_head ready;
	struct stlc_list_head unuse;
	fd_set readfd;
	fd_set writefd;
	fd_set exceptfd;
	SOCKET max_fd;
} cl_thread_master_t;

typedef struct cl_thread_s {
	u_int8_t type; /* thread type , THREAD_XXX*/
	u_int8_t pad[3];
	struct stlc_list_head link;
	cl_thread_master_t *master; /* pointer to the struct thread_master. */
	int (*func) (struct cl_thread_s *); /* event function */
	void *arg;			/* event argument */
	union {
		int val;			/* second argument of the event. */
		SOCKET fd;			/* file descriptor in case of read/write. */
		u_int32_t timeout;	/* 超时时间，uptime，毫秒 */
	} u;
} cl_thread_t;


/* Thread types. */
#define CL_THREAD_UNUSED	0
#define CL_THREAD_READ	1
#define CL_THREAD_WRITE	2
#define CL_THREAD_TIMER	3
#define CL_THREAD_EVENT	4
#define CL_THREAD_READY	5

typedef int (* cl_thread_func_t)(cl_thread_t *);



/* Macros. */
#define CL_THREAD_ARG(X) ((X)->arg)
#define CL_THREAD_FD(X)  ((X)->u.fd)
#define CL_THREAD_VAL(X) ((X)->u.val)

#define CL_THREAD_READ_ON(master,thread,func,arg,sock) \
  do { \
    if (! thread) \
      thread = cl_thread_add_read (master, func, arg, sock); \
  } while (0)

#define CL_THREAD_WRITE_ON(master,thread,func,arg,sock) \
  do { \
    if (! thread) \
      thread = cl_thread_add_write (master, func, arg, sock); \
  } while (0)

// 时间单位: 毫秒
#define CL_THREAD_TIMER_ON(master,thread,func,arg,time) \
	do { \
		if (thread != NULL) \
			cl_thread_cancel(thread); \
		thread = cl_thread_add_timer (master, func, arg, time); \
	} while (0)

#define CL_THREAD_EVENT_ON(master,thread,func,arg,val) \
  do { \
    if (! thread) \
      thread = cl_thread_add_event (master, func, arg, val); \
  } while (0)

#define CL_THREAD_OFF(thread) \
  do { \
    if (thread) \
      { \
        cl_thread_cancel (thread); \
        thread = NULL; \
      } \
  } while (0)

#define CL_THREAD_READ_OFF(thread)  CL_THREAD_OFF(thread)
#define CL_THREAD_WRITE_OFF(thread)  CL_THREAD_OFF(thread)
#define CL_THREAD_TIMER_OFF(thread)  CL_THREAD_OFF(thread)


extern cl_thread_t *cl_thread_add_read(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, SOCKET sock);
extern cl_thread_t *cl_thread_add_write(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, SOCKET sock);
extern cl_thread_t *cl_thread_add_timer(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, u_int32_t timer);
extern cl_thread_t *cl_thread_add_event(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, u_int32_t val);
extern void cl_thread_cancel(cl_thread_t *t);



extern cl_thread_t *cl_thread_fetch(cl_thread_master_t *m, cl_thread_t *fetch);
extern void cl_thread_call(cl_thread_t *thread);
extern RS cl_thread_init(cl_thread_master_t *m);
extern void cl_thread_stop(cl_thread_master_t *m);


#ifdef __cplusplus
}
#endif 

#endif

