#ifdef WIN32
//#include <Winbase.h>
//#include <Winnls.h>
#include "client_lib.h"
#include "cl_priv.h"
#include "cl_sys.h"
#include "cl_notify.h"
#include <time.h>

// MultiByteToWideChar
//#pragma comment(lib, "Coredll.lib")
 // socket
#pragma comment(lib, "WS2_32.lib")
// mutex
#pragma comment(lib, "Kernel32.lib")

RS cl_create_thread(cl_thread_info_t *ti, const char *name, cl_thread_proc_t proc, void *param)
{
	DWORD tid;

	strcpy(ti->name, name);
	ti->stopping = 0;
	
	if (cl_create_notify_sock(ti) != RS_OK)
		return RS_ERROR;
	
	ti->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, param, 0, &tid);
	if (ti->handle == NULL)
		return RS_ERROR;

	ti->tid = tid;
	while (ti->proc_notify == NULL) {
		msleep(1);
	}

	return RS_OK;
}

u_int32_t calc_begin_time_of_day(u_int32_t time)
{
    time_t t_time = 0;
    struct tm tms = {0};
    time_t src;
    
    if (!time) {
        time = get_sec();
    }
    src = time;
    
    //使用手机时区更贴近用户，所以使用localtime;
    localtime_s(&tms, &src);
    tms.tm_hour = tms.tm_min = tms.tm_sec = 0;
    t_time = mktime(&tms);
    
    return (u_int32_t)t_time;
}

double get_dtime()
{
	SYSTEMTIME st;
	struct tm tm;
	u_int32_t sec;
	double t;

	GetLocalTime(&st);
	memset(&tm, 0, sizeof(tm));
	tm.tm_year = st.wYear - LOCALTIME_YEAR_BASE;
	tm.tm_mon = st.wMonth - 1;
	tm.tm_mday = st.wDay;
	tm.tm_hour = st.wHour;
	tm.tm_min = st.wMinute;
	tm.tm_sec = st.wSecond;
	sec = (u_int32_t)mktime(&tm);
	
	t = sec + st.wMilliseconds/1000.0;

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
	return GetTickCount();
}

u_int32_t get_usec()
{
	LARGE_INTEGER freq;
	LARGE_INTEGER count;
	u_int32_t t;

	if ( ! QueryPerformanceFrequency(&freq) ) {
		log_err(true, "QueryPerformanceFrequency failed\n");
		return 0;
	}
	QueryPerformanceCounter(&count);

	t = (u_int32_t)((double)count.QuadPart/freq.QuadPart*1000000);

	return t;
}

int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = (long)clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;

    return 0;
}

void ansi_to_utf8(const char *str, char *out)
{
	int slen, u8Len, wcsLen;
	wchar_t *wBuf;

	slen = (int)strlen(str);

	//预转换，得到所需空间的大小
	wcsLen = MultiByteToWideChar(CP_ACP, 0, str, slen, NULL, 0);
	wBuf = cl_malloc(sizeof(wchar_t)*(wcsLen + 1));
	MultiByteToWideChar(CP_ACP, 0, str, slen, wBuf, wcsLen);

	u8Len = WideCharToMultiByte(CP_UTF8, 0, wBuf, wcsLen, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, wBuf, wcsLen, out, u8Len, NULL, NULL);
	out[u8Len] = '\0';

	cl_free(wBuf);
}

void utf8_to_ansi(const char *str, char *out)
{
	int wscLen, ansiLen;
	wchar_t* wszString;
	
	//预转换，得到所需空间的大小
	wscLen = MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), NULL, 0);	
	wszString = cl_malloc(sizeof(wchar_t)*(wscLen + 1));
	MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), wszString, wscLen);	
	wszString[wscLen] = '\0';
	
	ansiLen = WideCharToMultiByte(CP_ACP, 0, wszString, wscLen, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wszString, wscLen, out, ansiLen, NULL, NULL);
	out[ansiLen] = '\0';

	cl_free(wszString);
}

RS cl_init_lock(cl_mutex_t *mutex)
{
	*mutex = CreateMutex(NULL, false, NULL);
	if (*mutex == NULL) {
		log_err(true, "cl_init_lock failed\n");
		return RS_ERROR;
	}

	return RS_OK;
}

void cl_destroy_lock(cl_mutex_t *mutex)
{
	CloseHandle(*mutex);
	*mutex = NULL;
}

RS cl_lock(cl_mutex_t *mutex)
{
	if (WaitForSingleObject(*mutex, INFINITE) == WAIT_FAILED) {
		log_err(true, "cl_lock failed\n");
		return RS_ERROR;
	}

	return RS_OK;
}

RS cl_unlock(cl_mutex_t *mutex)
{
	if ( ! ReleaseMutex(*mutex) ) {
		log_err(true, "cl_unlock failed\n");
		return RS_ERROR;
	}

	return RS_OK;
}

FILE *cl_fopen(const char *fn, const char *mode)
{
	FILE *fp;

	if (fopen_s(&fp, fn, mode) == 0)
		return fp;

	return NULL;
}
#endif /*WIN32*/

