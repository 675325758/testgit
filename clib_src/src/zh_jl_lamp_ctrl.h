#ifndef	__ZH_JL_LAMP_CTRL_H__
#define	__ZH_JL_LAMP_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

#pragma pack(push,1)

typedef struct{
	u_int8_t start_code;
	u_int8_t on_off;
	u_int8_t color;
	u_int8_t bright;
    u_int8_t total_bright;
}ucp_zh_jl_3200_pkt;

#pragma pack(pop)

extern bool zh_jl_lamp_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool zh_jl_lamp_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

extern int zh_jl_lamp_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);

#endif

