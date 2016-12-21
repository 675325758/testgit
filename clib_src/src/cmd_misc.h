#ifndef __CMD_MISC_H__
#define	__CMD_MISC_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

extern slave_t *slave_alloc();
extern void slave_free(slave_t *slave);
extern int timer_query_master(cl_thread_t *t);
extern int timer_query_slave(cl_thread_t *t);
extern void slave_list_free(struct stlc_list_head *slave);
extern RS do_misc_a(user_t *user, pkt_t *pkt);
extern int query_slave_stat(slave_t *slave);
extern bool slave_replace(user_t *user, struct stlc_list_head *slave_list);


#ifdef __cplusplus
}
#endif 


#endif

