#ifndef EVM_SCM_CTRL_H
#define EVM_SCM_CTRL_H


#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

#endif


bool evm_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool evm_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
void evm_set_type_by_tlv(u_int8_t *sub_type, u_int8_t *ext_type, uc_tlv_t* tlv);
bool evm_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool evm_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

