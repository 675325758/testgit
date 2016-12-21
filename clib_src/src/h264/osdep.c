
#ifdef WIN32
#include "windows.h"
#else
#include <pthread.h>
#endif

#ifdef WIN32
static HANDLE 
#else
static pthread_mutex_t 
#endif
Gw_g_avlock;


void Gw_init_avlock()
{
#ifdef WIN32
	Gw_g_avlock = CreateMutex(NULL, 0 /*false*/, NULL);
#else
	pthread_mutex_init(&Gw_g_avlock, NULL);
#endif
}

void Gw_avlock()
{
#ifdef WIN32
	WaitForSingleObject(Gw_g_avlock, INFINITE);
#else
	pthread_mutex_lock(&Gw_g_avlock);
#endif
}

void Gw_avunlock()
{
#ifdef WIN32
	ReleaseMutex(Gw_g_avlock);
#else
	pthread_mutex_unlock(&Gw_g_avlock);
#endif
}

void Gw_destroy_avlock()
{
#ifdef WIN32
	CloseHandle(Gw_g_avlock);
#else
	pthread_mutex_destroy(&Gw_g_avlock);
#endif

}

