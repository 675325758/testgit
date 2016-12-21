#ifndef	__CL_MEM_H__
#define	__CL_MEM_H__


#ifdef __cplusplus
extern "C" {
#endif 

#define	MEM_DEBUG 0
extern void cl_dump_data_debug(const char *fn, int line, void *data, int len);


#if	MEM_DEBUG
extern void *cl_malloc_debug(const char *fn, int line, size_t size);
extern void *cl_calloc_debug(const char *fn, int line, size_t size, int n);
extern char *cl_strdup_debug(const char *fn, int line, const char *str);
extern void *cl_realloc_debug(const char *fn, int line, void *ptr, size_t size);
extern void cl_free_debug(const char *fn, int line, void *ptr);


#define	cl_malloc(size) cl_malloc_debug(__FILE__, __LINE__, size)
#define	cl_calloc(size, n) cl_calloc_debug(__FILE__, __LINE__, size, n)
#define	cl_realloc(ptr, size) cl_realloc_debug(__FILE__, __LINE__, ptr, size)
#define cl_strdup(str) cl_strdup_debug(__FILE__, __LINE__, str)
#define	cl_free(ptr) cl_free_debug(__FILE__, __LINE__, ptr)

#define	dump_data(data, size) cl_dump_data_debug(__FILE__, __LINE__, data, size)

extern void extern_mem_check(const char *fn, int line);
#define cl_mem_check() 	extern_mem_check(__FILE__, __LINE__)

#else
#define	cl_malloc(size) malloc(size)
#define	cl_calloc(size, n) calloc(size, n)
#define	cl_realloc(ptr, size) realloc(ptr, size)
#define cl_strdup(str) strdup(str)
#define	cl_free(ptr) free(ptr)
#define	dump_data(data, size) cl_dump_data_debug(__FILE__, __LINE__, data, size)

#define cl_mem_check() 	

#endif


#ifdef __cplusplus
}
#endif 


#endif

