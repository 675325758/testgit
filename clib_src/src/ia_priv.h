#ifndef	__IA_PRIV_H__
#define	__IA_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif 

extern bool ia_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern bool ia_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern RS ia_init(user_t *user);
extern void ia_free(user_t *user);
extern void ia_build_objs(user_t* user,cl_dev_info_t* ui);


#ifdef __cplusplus
}
#endif 

#endif
