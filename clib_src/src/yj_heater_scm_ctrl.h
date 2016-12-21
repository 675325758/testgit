#ifndef	__YJ_HEATER_SCM_CTRL_H
#define	__YJ_HEATER_SCM_CTRL_H

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"
#include "cl_yj_heater.h"

#define YJ_START_CODE1 0xFF
#define YJ_START_CODE2 0xAA


#pragma pack(push,1)

typedef struct {
    u_int8_t start_code1;
    u_int8_t start_code2;
    u_int8_t command;
    u_int8_t device_type;
    u_int8_t data_len;
    u_int8_t pad[3];
} ucp_yj_scm_head_t;

#pragma pack(pop)

extern bool yj_heater_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool yj_heater_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

extern int yj_heater_scm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);

#endif


