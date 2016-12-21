#ifndef	__COMMUNITY_PRIV_H__
#define	__COMMUNITY_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif 

void send_notify_center_list(user_t *user);
void recv_notify_hello(user_t *user, pkt_t *pkt, struct sockaddr_in *addr);
void recv_cmt_center_list(user_t *user, pkt_t *pkt);
void recv_notify(user_t *user, pkt_t *pkt, struct sockaddr_in *addr);
void recv_notify_result(user_t *user, pkt_t *pkt, struct sockaddr_in *addr);
void recv_notify_expect(user_t *user, pkt_t *pkt, struct sockaddr_in *addr);
bool cmt_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
RS do_cmt_op_device(user_t *user, pkt_t *pkt);
void get_ip_list(u_int32_t *ip, int *count);
void user_cmt_free(user_t *user);


#ifdef __cplusplus
}
#endif 

#endif

