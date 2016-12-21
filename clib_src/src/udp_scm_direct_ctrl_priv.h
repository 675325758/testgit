#ifndef	__UDP_SCM_DIRECT_CTRL_PRIV_H__
#define	__UDP_SCM_DIRECT_CTRL_PRIV_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"

typedef struct{
	u_int8_t ident;
	u_int8_t pad[3];
}tlv_dev_ident_t;

extern bool scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern void scm_quick_query_info(smart_air_ctrl_t* ac);
extern bool scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern int scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
/*发包*/
extern RS scm_send_single_get_pkt(ucc_session_t *s,void* content,int content_len);
extern bool scm_send_single_set_pkt(ucc_session_t *s,void* content,int content_len);

//-1，表示未识别到
extern int scm_get_ext_type_by_ident(u_int8_t dev_sub_type,u_int8_t* ident,int ident_len);

extern int scm_do_dispatch_tlv(user_t* user,uc_tlv_t* tlv);

extern int scm_get_ext_type_by_tlv(u_int8_t dev_sub_type,u_int8_t real_ext_type,uc_tlv_t* tlv);

#endif

