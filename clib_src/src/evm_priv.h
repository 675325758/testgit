#ifndef EVM_PRIV_H
#define EVM_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif 

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"
#include "uc_client.h"

enum {
	ACT_EVM_QUERY_INFO = 1,
	ACT_EVM_ERASE = 2,
	ACT_EVM_UPGRADE = 3,
};

typedef struct {
	u_int8_t start_flag[4];
	u_int32_t fw_size;	// include hdr data and tail
	u_int32_t entry_addr;	
	u_int32_t create_time;
	u_int32_t soft_ver;
	u_int32_t svn;
	u_int8_t vendor[16];
} evm_fw_hdr_t;


void sys_get_evm_info(smart_air_ctrl_t* air_ctrl, ucp_obj_t* uobj);

RS sa_dev_upgrade_evm(cl_notify_pkt_t *cln_pkt);


#ifdef __cplusplus
}
#endif


#endif
