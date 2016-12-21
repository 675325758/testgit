#ifndef	 __IR_LIB_H__
#define	__IR_LIB_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _ir_lib_s {
	u_int32_t next_check;
	// json_t
	void *json;
} ir_lib_t;

extern void check_query_ir_url(user_t *user);


#ifdef __cplusplus
}
#endif 

#endif

