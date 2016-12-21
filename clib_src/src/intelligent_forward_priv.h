#ifndef	__IF_PRIV_H__
#define	__IF_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct{
	cl_handle_t dev_handle;
}cln_if_query_t;


extern bool if_proc_notify(cl_notify_pkt_t *pkt, RS *ret);



#ifdef __cplusplus
}
#endif 

#endif




