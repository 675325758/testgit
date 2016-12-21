#ifndef	__LEIS_SCM_CTRL_H__
#define	__LEIS_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"
#include "cl_leis.h"

#pragma pack(push,1)

typedef struct{
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t rid;
	u_int8_t cmd;
	u_int16_t param_len;
	u_int8_t ret;
	u_int8_t checksum;
} ucp_leis_pkt_hdr_t;

typedef struct {
    u_int8_t R;
    u_int8_t G;
    u_int8_t B;
    u_int8_t W;
    u_int8_t C;
    u_int8_t power;
    u_int8_t mod_id;
    u_int8_t o_wc_l;
    u_int8_t o_r;
    u_int8_t o_g;
    u_int8_t o_b;
    u_int8_t o_l;
    u_int8_t o_c;
    u_int8_t hwconf;
    u_int32_t r_id;
} leis_uart_lamp_ctrl_param_t;

#pragma pack(pop)

extern bool leis_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool leis_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern bool leis_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);

extern int leis_get_ext_type_by_tlv(uc_tlv_t* tlv);

#endif


