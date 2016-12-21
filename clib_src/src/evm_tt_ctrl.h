#ifndef EVM_TT_CTRL_H
#define EVM_TT_CTRL_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_yuyuan.h"


enum {
	ACT_EVM_TT_UART = 1,
};


bool tt_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool tt_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool tt_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif

