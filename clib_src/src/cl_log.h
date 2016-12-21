#ifndef	__CL_LOG_H__
#define	__CL_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif 
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err
#undef nd_login_debug

#define	log_debug(fmt, ...)	
#define	log_info(fmt, ...)	
#define	log_err(print_err, fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)


//登陆流程打印
extern void _nd_login_debug(void *in_s, const char *fmt, ...);
#define nd_login_debug(s, ...) _nd_login_debug(s, __VA_ARGS__)
extern void _nd_login_info(const char *file, int line, void *in_s, const char *fmt, ...);
#define nd_login_info(s, ...) _nd_login_info(__FUNCTION__, __LINE__, s, __VA_ARGS__)

//联动流程打印
extern void _nd_la_debug(void *in_s, const char *fmt, ...);
#define nd_la_debug(s, ...) _nd_la_debug(s, __VA_ARGS__)
extern void _nd_la_info(const char *file, int line, void *in_s, const char *fmt, ...);
#define nd_la_info(s, ...) _nd_la_info(__FUNCTION__, __LINE__, s, __VA_ARGS__)

//局域网流程打印
extern void _nd_lan_debug(void *in_dev, const char *fmt, ...);
#define nd_lan_debug(s, ...) _nd_lan_debug(s, __VA_ARGS__)
extern void _nd_lan_info(const char *file, int line, void *in_dev, const char *fmt,...);
#define nd_lan_info(s, ...) _nd_lan_info(__FUNCTION__, __LINE__, s, __VA_ARGS__)


//hex dump
extern void nd_login_memdump(char* pre,void* dest, u_int32_t len);
extern void nd_la_memdump(char* pre,void* dest, u_int32_t len);
extern void nd_lan_memdump(char* pre,void* dest, u_int32_t len);

#ifdef __cplusplus
}
#endif 


#endif

