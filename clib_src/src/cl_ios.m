#include <time.h>
#include "cl_priv.h"
#include "cl_sys.h"

#ifdef IOS_COMPILE
#include <sys/time.h>
#import <Foundation/NSProgress.h>
#endif

double get_dtime()
{
	struct timeval tv;
	double t;
    
	gettimeofday(&tv, NULL);
	t = tv.tv_sec + tv.tv_usec/1000000.0;
    
	return t;
}

u_int32_t get_sec()
{
	time_t now;

	time(&now);

	return (u_int32_t)now;
}

// 单位毫秒
u_int32_t get_msec()
{
	
	u_int64_t t;
    
#ifdef IOS_COMPILE
	t = (u_int64_t)([NSProcessInfo processInfo].systemUptime*1000);
#else
    struct timespec tp;
	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
		t = tp.tv_sec*1000 + tp.tv_nsec/1000000;
	} else {
		log_err(true, "clock_gettime failed\n");
		t = 0;
	}
#endif


	return (u_int32_t)t;
}

// 单位毫秒
u_int32_t get_usec()
{
	
	u_int64_t t;
#ifdef IOS_COMPILE
	t = (u_int64_t)([NSProcessInfo processInfo].systemUptime*1000000);
#else
    struct timespec tp;
    
	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
		t = tp.tv_sec*1000000 + tp.tv_nsec/1000;
	} else {
		log_err(true, "clock_gettime failed\n");
		t = 0;
	}
#endif


	return (u_int32_t)t;
}

u_int32_t calc_begin_time_of_day(u_int32_t time)
{
    u_int32_t t_time = 0;
    struct tm tms = {0};
    time_t src;
    
    if (!time) {
        time = get_sec();
    }
    src = time;
    
    // use localtime here
    localtime_r(&src, &tms);
    tms.tm_hour = tms.tm_min = tms.tm_sec = 0;
    t_time = (u_int32_t)mktime(&tms);
    
    return t_time;
}

RS cl_init_lock(cl_mutex_t *mutex)
{
	pthread_mutex_t *m;
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
    
#ifdef IOS_COMPILE
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
	
	m = (pthread_mutex_t *)cl_calloc(sizeof(pthread_mutex_t), 1);
	pthread_mutex_init(m, &attr);
	*mutex = m;

	pthread_mutexattr_destroy(&attr);
	
	return RS_OK;
}

void cl_destroy_lock(cl_mutex_t *mutex)
{
	pthread_mutex_destroy(*mutex);
	*mutex = NULL;
}

RS cl_lock(cl_mutex_t *mutex)
{
	return pthread_mutex_lock(*mutex);
}

RS cl_unlock(cl_mutex_t *mutex)
{
	return pthread_mutex_unlock(*mutex);
}

typedef void *(* LPTHREAD_START_ROUTINE) (void *);

RS cl_create_thread(cl_thread_info_t *ti, const char *name, cl_thread_proc_t proc, void *param)
{
	pthread_attr_t attr;

	strcpy(ti->name, name);
	ti->stopping = 0;
	
	if (cl_create_notify_sock(ti) != RS_OK)
		return RS_ERROR;

	pthread_attr_init(&attr);
	pthread_create(&ti->handle, &attr, (LPTHREAD_START_ROUTINE)proc, param);
	pthread_attr_destroy(&attr);

	pthread_detach(ti->handle);
	
	ti->tid = (u_int32_t)ti->handle;
	
	while (ti->proc_notify == NULL) {
		msleep(1);
	}

	return RS_OK;
}


