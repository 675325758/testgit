#ifndef __HELTH_PRIV_H__
#define __HELTH_PRIV_H__
#include "cl_health.h"

typedef struct{
	cl_handle_t handle;
	family_t fm;
}cln_family_t;

typedef struct{
	cl_handle_t handle;
	measure_query_t q;
}cln_measure_query_t;

typedef struct{
	cl_handle_t handle;
	int del_all;
	measure_del_t del;
}cln_measure_del_t;


typedef struct{
	struct stlc_list_head node;
	void *p;
}list_link_t;

typedef struct{
	struct stlc_list_head measure_hd;
	family_list_t *fl;
	family_t config_a;
}user_health_t;

bool health_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
bool health_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);

user_health_t *user_health_init();
void user_health_clean(user_health_t* h);
RS parse_measure_data(measure_t *m, net_messure_t *n);
void fm_to_local(net_fm_t *net, family_t *up);

#endif

