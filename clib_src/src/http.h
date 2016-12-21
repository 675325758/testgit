#ifndef	 __HTTP_H__
#define	__HTTP_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

extern char *http_sprintf(char *buf, char *fmt, ...);
extern RS http_get(const char *url, int event, cl_handle_t handle);
extern RS http_post(const char *url, const char *post_data, int event, cl_handle_t handle);
extern RS http_get_ext(const char *url, int event, cl_handle_t handle, u_int8_t index);

#ifdef __cplusplus
}
#endif 

#endif

