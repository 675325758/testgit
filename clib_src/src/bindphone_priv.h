#ifndef	__BINDPHONE_PRIV_H__
#define	__BINDPHONE_PRIV_H__

#include "cl_priv.h"

void send_bind_phone_q(user_t *user);
void recv_bind_phone_result(user_t *user, pkt_t *pkt);
void recv_bind_phone_request_list(user_t *user, pkt_t *pkt);
void recv_bind_phone_list(user_t *user, pkt_t *pkt);


RS user_bind_phone_q(cl_notify_pkt_t *cln_pkt);
RS user_bind_phone_request_list_query(cl_notify_pkt_t *cln_pkt);
RS user_bind_phone_list_query(cl_notify_pkt_t *cln_pkt);
RS user_bind_phone_normal_allow(cl_notify_pkt_t *cln_pkt);
RS user_bind_phone_operate(cl_notify_pkt_t *cln_pkt);

RS user_bind_phone_alloc(user_t *user);
void user_bind_phone_free(user_t *user);


#endif

