#ifndef	__CL_LOOKUP_H__
#define	__CL_LOOKUP_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

// handle type
enum {
	HDLT_USER = 1,
	HDLT_SLAVE,
	HDLT_VIDEO,
	HDLT_EQUIPMENT,
    HDLT_AREA,
    HDLT_SCENE,
    HDLT_TMP,
    HDLT_LINKAGE,
	HDLT_MAX = 256
};

#define	HANDLE_MAX	0x1000000

extern cl_handle_t handle_create(int type);
extern user_t *user_lookup_by_sn(u_int64_t sn);
extern slave_t *slave_lookup_by_sn(user_t *user, u_int64_t sn);
extern slave_t *slave_lookup_by_ident(u_int64_t sn);
extern void *lookup_by_handle(u_int8_t lookup_type, cl_handle_t handle);
extern slave_t *slave_lookup_by_handle(user_t *user, cl_handle_t handle);
extern user_t *user_lookup_by_name(user_t *user, const char *name);

#define HANDLE_TYPE(handle) ((handle>>24)&0xFF)

#define IS_SAME_HANDLE_TYPE(handle,type) (HANDLE_TYPE(handle)==type)

#ifdef __cplusplus
}
#endif 


#endif

