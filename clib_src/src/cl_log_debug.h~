/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_log_debug.h
**  File:    cl_log_debug.h
**  Author:  liubenlong
**  Date:    03/08/2016
**
**  Purpose:
**    调试控制头文件.
**************************************************************************/


#ifndef CL_LOG_DEBUG_H
#define CL_LOG_DEBUG_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */


/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
//***************************打印总开关，如果没有这个就关闭所有打印*******************
//#define LOG_DEBUG_ON

//重要文件调试开关，主要是给app童鞋调试用的
#define LOG_IMPORTANG_FILE




#ifdef LOG_DEBUG_ON
#if (!defined(DEBUG_IMPORTANT_FILE) || defined(LOG_IMPORTANG_FILE))
#if (DEBUG_LOCAL_LEVEL == 0)
#define	log_debug(fmt, ...)	
#define	log_info(fmt, ...)	
#define	log_err(print_err, fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)
    
#elif (DEBUG_LOCAL_LEVEL == 1)
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err

#define	log_debug(fmt, ...)	
#define	log_info(fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)

#define	log_err(print_err, ...) _log_err(__FILE__, __LINE__, print_err, __VA_ARGS__)
extern void _log_err(const char *file, int line, bool print_err, const char *fmt, ...);

#elif (DEBUG_LOCAL_LEVEL == 2)
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err

#define	log_debug(fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)

#define	log_err(print_err, ...) _log_err(__FILE__, __LINE__, print_err, __VA_ARGS__)
extern void _log_err(const char *file, int line, bool print_err, const char *fmt, ...);

#define	log_info( ...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
extern void _log_info(const char *file, int line, const char *fmt, ...);

#elif (DEBUG_LOCAL_LEVEL == 3)
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err

#define	log_debug(...) _log_debug(__VA_ARGS__)
extern void _log_debug(const char *fmt, ...);

#define	log_info( ...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
extern void _log_info(const char *file, int line, const char *fmt, ...);

#define	log_err(print_err, ...) _log_err(__FILE__, __LINE__, print_err, __VA_ARGS__)
extern void _log_err(const char *file, int line, bool print_err, const char *fmt, ...);

#define	cl_assert(b) _cl_assert(!(!(b)), __FILE__, __LINE__)
extern void _cl_assert(bool b, const char *file, int line);

#define mem_dump(pre,dest,len) memdump(pre,dest,len);
extern void memdump(char* pre,void* dest, u_int32_t len);

#else
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err

#define	log_debug(fmt, ...)	
#define	log_info(fmt, ...)	
#define	log_err(print_err, fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)
#endif
#else
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err

#define	log_debug(fmt, ...)	
#define	log_info(fmt, ...)	
#define	log_err(print_err, fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)
#endif
#else
#undef log_debug
#undef log_info
#undef cl_assert
#undef mem_dump
#undef log_err

#define	log_debug(fmt, ...)	
#define	log_info(fmt, ...)	
#define	log_err(print_err, fmt, ...)	
#define	cl_assert(b)
#define mem_dump(pre,dest,len)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_LOG_DEBUG_H */

