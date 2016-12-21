#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "linkage_client.h"
#include "lan_dev_probe_priv.h"

#ifdef	WIN32
const char *log_file = "d:\\cl_log.txt";

#define LOG_TO_PHONE_FILE
#elif defined (ANDROID)

//#define ANDROID_TO_FILE

#include <android/log.h>  
const char *log_file = NULL;
// log标签
#define  TAG    "CLibLog"
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG, __VA_ARGS__)
#else
const char *log_file = NULL;
#endif

#define	LOG_BUF_SIZE	(16*1024)

//network dianosis file max
#define ND_LOG_FILE_SIZE		(2*1024*1024)

//debug dir
#define ND_LOG_DIR			"log"

//#define LOG_TO_PHONE_FILE

static bool is_init_dest_file = false;
static char log_dest_file[256] = {0};

static void init_log_file()
{
#ifndef WIN32
    if(is_init_dest_file){
        return;
    }
    
    if(cl_priv != NULL && cl_priv->dir[0] != '\0'){
        sprintf(log_dest_file,"%s/cl_log.txt",cl_priv->dir);
        log_file = log_dest_file;
        is_init_dest_file = true;
    }
#endif
}



static bool log_to_file(char* buf,int len)
{
    FILE *fp;

#ifdef LOG_TO_PHONE_FILE
    init_log_file();
#endif
    
    if (log_file != NULL) {
        if ((fp = cl_fopen(log_file, "a+")) != NULL) {
            fwrite(buf, len, 1, fp);
            fclose(fp);
            
            return true;
        }
    }
    return false;
}


void _log_debug(const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now;

	now = get_msec();
	buf = malloc(LOG_BUF_SIZE+64);
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[DEBUG] %u:%03u: ", now/1000, now%1000);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    if (log_to_file(buf,pos)) {
        free(buf);
        return;
    }
    
#ifdef ANDROID
    LOGD("%s", buf);
#else
    printf("%s",buf);
#endif
    
	free(buf);
}

void _log_info(const char *file, int line, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
    va_list vl;
	u_int32_t now;

	now = get_msec();
	buf = malloc(LOG_BUF_SIZE+64);
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[INFO]  %u:%03u: %s line %u: ", now/1000, now%1000, file, line);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);

    if (log_to_file(buf,pos)) {
        free(buf);
        return;
    }
    
#ifdef ANDROID
    LOGI("%s", buf);
#else
    printf("%s",buf);
#endif

	free(buf);
}

void _log_err(const char *file, int line, bool print_err, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	int ecode;
    va_list vl;
	u_int32_t now;
	
	ecode = GET_SOCK_ECODE();

	now = get_msec();
	buf = malloc(LOG_BUF_SIZE+64);
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[ERROR] %u:%03u: %s line %d: ", now/1000, now%1000, file, line);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
	if (print_err && ecode != 0) {
#ifdef WIN32			
		char *lpMsgBuf;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			ecode,
			0,
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			);

		if (lpMsgBuf != NULL) {
			pos += sprintf(buf + pos, "        [ERROR] %s\n", lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
#else
		pos += sprintf(buf + pos, "        [ERROR] %s\n", strerror(ecode));
#endif
	} else {
		pos += sprintf(buf + pos, "\n");
	}
	
    if (log_to_file(buf,pos)) {
        free(buf);
        return;
    }
    
#ifdef ANDROID
    LOGE("%s", buf);
#else
    printf("%s",buf);
#endif

	free(buf);	
}

void _cl_assert(bool b, const char *file, int line)
{
	if ( ! b ) {
		log_err(false, "%s line %d assert failed!!!\n", file, line);
		*(char *)0 = 0;
	}
}

#ifdef ANDROID

void memdump(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;

    if (!p) {
        return;
    }

    if (pre) {
        LOGI("%s:\n",pre);
    }

    for (i = 0; i<len; i++) {
        if (i>0 && i%16 == 0) {
            LOGI("\n");
        }

        LOGI("%02X ",p[i]);
    }

    LOGI("\n");

}

#else

void memdumpone(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;
	int index = 0;
	int line = 0;
	char buff[1024*10];
	u_int16_t *pdata = (u_int16_t *)dest;
    
    if (!p) {
        return;
    }
    
    if (pre) {
        index += sprintf(buff+index, "%s:\n",pre);
    }

	index += sprintf(buff + index, "%-5d  ", line++);
    for (i = 0; i<len; i++) {
        if (i>0 && i%16 == 0) {
           index += sprintf(buff + index, "\n");
		   index += sprintf(buff + index, "%d  ", line++);
        }
        
        index += sprintf(buff+index, "%02x ",p[i]);
    }
    
    _log_debug("%s\n", buff);
    
}


void memdump(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;
	int index = 0;
	int line = 0;
	char buff[1024*10];
	u_int16_t *pdata = (u_int16_t *)dest;
    
    if (!p) {
        return;
    }
    
    if (pre) {
        index += sprintf(buff+index, "%s:\n",pre);
    }

	index += sprintf(buff + index, "%-5d  ", line++);
    for (i = 0; i<len/2; i++) {
        if (i>0 && i%16 == 0) {
           index += sprintf(buff + index, "\n");
		   index += sprintf(buff + index, "%-5d  ", line++);
        }
        
        index += sprintf(buff+index, "%-5u ",htons(pdata[i]));
    }
    
    _log_debug("%s\n", buff);
    
}

#endif

static void nd_file_clean(char *file)
{
	FILE *fp = NULL;

	fp = fopen(file, "w");

	if (fp) {
		fclose(fp);
	}
}

static FILE *nd_init_output(char *file_prefix)
{
	struct stat stat1,stat2;
	int exist1 = 0,exist2 = 0, fnum = 0;
	char buf[300];
	char file1[300];
	char file2[300];
	char log_dir[300];
	FILE *fp = NULL;
	u_int32_t debug_size_limit = ND_LOG_FILE_SIZE;

	if (!cl_priv->priv_dir) {
		return NULL;
	}
	if (debug_size_limit < cl_priv->nd_debug_size) {
		debug_size_limit = cl_priv->nd_debug_size;
	}

	//dir create
	sprintf(log_dir, "%s/%s", cl_priv->priv_dir, ND_LOG_DIR);
	MKDIR(log_dir, 0777);
	
	sprintf(file1, "%s/%s/%s%d.txt", cl_priv->priv_dir, ND_LOG_DIR, file_prefix, 0);
	sprintf(file2, "%s/%s/%s%d.txt", cl_priv->priv_dir, ND_LOG_DIR, file_prefix, 1);
	exist1 = cl_access(file1,0)?0:1;
	exist2 = cl_access(file2,0)?0:1;
	if(exist1){
		if(stat(file1,&stat1)!=0){
			nd_file_clean(file1);
			exist1 =0;
		}
	}
	if(exist2){
		if(stat(file2,&stat2)){
			nd_file_clean(file2);
			exist2 =0;
		}
	}

	if(!exist1 && !exist2) {
		goto end;
	}
	if(exist1){
		if(exist2){
			if((u_int32_t)stat1.st_size < debug_size_limit) {
				goto end;
			}
			if((u_int32_t)stat2.st_size < debug_size_limit){
				fnum = 1;
				goto end;
			}

			if((u_int32_t)stat2.st_mtime <= stat1.st_mtime){
				fnum = 1;
				nd_file_clean(file2);
				goto end;
			}else{
				nd_file_clean(file1);
				goto end;
			}

		}

		if((u_int32_t)stat1.st_size >= debug_size_limit) {
			fnum = 1;
		}
		goto end;
	}

	if((u_int32_t)stat2.st_size >= debug_size_limit) {
		fnum =0;
	} else {
		fnum = 1;
	}

end:
	sprintf(buf,"%s/%s/%s%d.txt",cl_priv->priv_dir , ND_LOG_DIR, file_prefix ,fnum&0x1);
	fp = fopen(buf, "a");

	return fp;
}

static void _nd_log_to_file(char* buf,int len, char *file_prefix)
{
    FILE *fp;

	fp = nd_init_output(file_prefix);
    if (!fp) {
		return;
	}

    fwrite(buf, len, 1, fp);
    fclose(fp);
}

void _nd_login_debug(void *in_s, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now_msec;
	time_t now_s;
	char time_str[100] = {0};
	ucc_session_t *s = (ucc_session_t *)in_s;

	// TODO:这里判断下是否开启打印
	if (!cl_priv->nd_debug) {
		return;
	}
	now_msec = get_msec();
	now_s = time(NULL);
	buf = malloc(LOG_BUF_SIZE+64);

	sprintf(time_str, "%s", ctime(&now_s));
	time_str[strlen(time_str)-1] = 0;
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[sn=%"PRIu64"] time=%s %u:%03u: ", 
		s?s->user->sn:0, time_str, now_msec/1000, now_msec%1000);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    _nd_log_to_file(buf,pos, "login_debug");
    
	free(buf);
}

void _nd_login_info(const char *file, int line, void *in_s, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now_msec;
	time_t now_s;
	char time_str[100] = {0};
	ucc_session_t *s = (ucc_session_t *)in_s;

	// TODO:这里判断下是否开启打印
	if (!cl_priv->nd_debug) {
		return;
	}
	now_msec = get_msec();
	now_s = time(NULL);
	buf = malloc(LOG_BUF_SIZE+64);

	sprintf(time_str, "%s", ctime(&now_s));
	time_str[strlen(time_str)-1] = 0;
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[sn=%"PRIu64"] time=%s %u:%03u: %s line=%u:", 
		s?s->user->sn:0, time_str, now_msec/1000, now_msec%1000, 
		file, line);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    _nd_log_to_file(buf,pos, "login_debug");
    
	free(buf);
}

void _nd_la_debug(void *in_s, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now_msec;
	time_t now_s;
	char time_str[100] = {0};
	ucla_session_t *s = (ucla_session_t *)in_s;

	// TODO:这里判断下是否开启打印
	if (!cl_priv->nd_debug) {
		return;
	}
	now_msec = get_msec();
	now_s = time(NULL);
	buf = malloc(LOG_BUF_SIZE+64);

	sprintf(time_str, "%s", ctime(&now_s));
	time_str[strlen(time_str)-1] = 0;
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[doname=%s] time=%s %u:%03u: ", 
		s?s->doname:"null", time_str, now_msec/1000, now_msec%1000);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    _nd_log_to_file(buf,pos, "la_debug");
    
	free(buf);
}

void _nd_la_info(const char *file, int line, void *in_s, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now_msec;
	time_t now_s;
	char time_str[100] = {0};
	ucla_session_t *s = (ucla_session_t *)in_s;

	// TODO:这里判断下是否开启打印
	if (!cl_priv->nd_debug) {
		return;
	}
	now_msec = get_msec();
	now_s = time(NULL);
	buf = malloc(LOG_BUF_SIZE+64);

	sprintf(time_str, "%s", ctime(&now_s));
	time_str[strlen(time_str)-1] = 0;
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[doname=%s] time=%s %u:%03u: %s line=%u:", 
		s?s->doname:"null", time_str, now_msec/1000, now_msec%1000, file, line);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    _nd_log_to_file(buf,pos, "la_debug");
    
	free(buf);
}

void _nd_lan_debug(void *in_dev, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now_msec;
	time_t now_s;
	char time_str[100] = {0};
	dev_probe_info_t* dev = (dev_probe_info_t *)in_dev;

	// TODO:这里判断下是否开启打印
	if (!cl_priv->nd_debug) {
		return;
	}
	now_msec = get_msec();
	now_s = time(NULL);
	buf = malloc(LOG_BUF_SIZE+64);

	sprintf(time_str, "%s", ctime(&now_s));
	time_str[strlen(time_str)-1] = 0;
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[lan sn=%"PRIu64"] time=%s %u:%03u: ", 
		dev?dev->dev_sn:0, time_str, now_msec/1000, now_msec%1000);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    _nd_log_to_file(buf,pos, "lan_debug");
    
	free(buf);
}

void _nd_lan_info(const char *file, int line, void *in_dev, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	va_list vl;
	u_int32_t now_msec;
	time_t now_s;
	dev_probe_info_t* dev = (dev_probe_info_t *)in_dev;
	char time_str[100] = {0};

	// TODO:这里判断下是否开启打印
	if (!cl_priv->nd_debug) {
		return;
	}
	now_msec = get_msec();
	now_s = time(NULL);
	buf = malloc(LOG_BUF_SIZE+64);

	sprintf(time_str, "%s", ctime(&now_s));
	time_str[strlen(time_str)-1] = 0;
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "[lan sn=%"PRIu64"] time=%s %u:%03u: %s line=%u: ", 
		dev?dev->dev_sn:0, time_str, now_msec/1000, now_msec%1000, file, line);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
    va_end(vl);
    
    _nd_log_to_file(buf,pos, "lan_debug");
    
	free(buf);
}



void nd_login_memdump(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;
	int index = 0;
	int line = 0;
	char buff[1024*10];
	u_int16_t *pdata = (u_int16_t *)dest;

	if (!cl_priv->nd_debug) {
		return;
	}
    if (!p) {
        return;
    }
    if (pre) {
        index += sprintf(buff+index, "%s:\n",pre);
    }
	index += sprintf(buff + index, "%-5d  ", line++);
    for (i = 0; i<len; i++) {
        if (i>0 && i%16 == 0) {
           index += sprintf(buff + index, "\n");
		   index += sprintf(buff + index, "%d  ", line++);
        }
        
        index += sprintf(buff+index, "%02x ",p[i]);
    }
	_nd_log_to_file(buff,index, "login_debug");
}

void nd_la_memdump(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;
	int index = 0;
	int line = 0;
	char buff[1024*10];
	u_int16_t *pdata = (u_int16_t *)dest;

	if (!cl_priv->nd_debug) {
		return;
	}
    if (!p) {
        return;
    }
    if (pre) {
        index += sprintf(buff+index, "%s:\n",pre);
    }
	index += sprintf(buff + index, "%-5d  ", line++);
    for (i = 0; i<len; i++) {
        if (i>0 && i%16 == 0) {
           index += sprintf(buff + index, "\n");
		   index += sprintf(buff + index, "%d  ", line++);
        }
        
        index += sprintf(buff+index, "%02x ",p[i]);
    }

	index += sprintf(buff+index, "\n");
	_nd_log_to_file(buff,index, "la_debug");
}


void nd_lan_memdump(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;
	int index = 0;
	int line = 0;
	char buff[1024*10];
	u_int16_t *pdata = (u_int16_t *)dest;

	if (!cl_priv->nd_debug) {
		return;
	}
    if (!p) {
        return;
    }
    if (pre) {
        index += sprintf(buff+index, "%s:\n",pre);
    }
	index += sprintf(buff + index, "%-5d  ", line++);
    for (i = 0; i<len; i++) {
        if (i>0 && i%16 == 0) {
           index += sprintf(buff + index, "\n");
		   index += sprintf(buff + index, "%d  ", line++);
        }
        
        index += sprintf(buff+index, "%02x ",p[i]);
    }
	_nd_log_to_file(buff,index, "lan_debug");
}



