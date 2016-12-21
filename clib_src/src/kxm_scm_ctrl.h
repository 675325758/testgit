#ifndef	__KXM_SCM_CTRL_H__
#define	__KXM_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

#define KXM_START_CODE1 0x5A
#define KXM_START_CODE2 0xA5
#define KXM_MIN_BODY_LEN	73

#pragma pack(push,1)

typedef struct {
    u_int8_t s_code1;
    u_int8_t s_code2;
    u_int8_t dev_index;
    u_int8_t command;
    u_int8_t start_addr;
    u_int8_t d_len;
}ucp_kxm_smc_head;

#pragma pack(pop)

extern bool kxm_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool kxm_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern void kxm_do_query_hook(smart_air_ctrl_t* air_ctrl);
extern int kxm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);

#endif

