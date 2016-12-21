#ifndef	 __WGET_H__
#define	__WGET_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 


extern RS wget(char *url, char **ret_buf, int *ret_len, bool is_post, char *post_data);



#ifdef __cplusplus
}
#endif 

#endif

