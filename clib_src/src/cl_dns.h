#ifndef	__CL_DNS_H__
#define	__CL_DNS_H__

#ifdef __cplusplus
extern "C" {
#endif 

extern u_int32_t cl_dns_thread(void *param);
extern int my_gethostbyname(const char *name, u_int32_t *ipp, int buf_size);
extern char *cl_lookup_dn_by_ip(u_int32_t ip);
extern int disp_resolv_doname(int num, const char (* const name)[64]);
bool cl_get_addr_by_doname(char *dn, ipc_sock_t *sock_addr, int *num);


#ifdef __cplusplus
}
#endif 


#endif

