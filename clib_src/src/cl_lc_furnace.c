#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "lc_furnace_priv.h"

RS static cl_sa_ah_ctrl_common(cl_handle_t dev_handle, u_int32_t action, u_int8_t param)
{
	cl_notify_pkt_t *pkt;
	cln_sa_ah_info_t *cln_ah;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, action, CLNPF_ACK);
	pkt->param_len = sizeof(*cln_ah);
	cln_ah = (cln_sa_ah_info_t*)pkt->data;
	cln_ah->dev_handle = dev_handle;
	cln_ah->action_param = param;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_ah_ctrl_power(cl_handle_t dev_handle, bool is_on)
{
	return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_POWER, !!is_on);
}
    
CLIB_API RS cl_sa_ah_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode)
{
	if(mode == AH_HEAT_MODE_LOW 
		|| mode == AH_HEAT_MODE_HIGH)
		return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_MODE, mode);
	else return RS_INVALID_PARAM;
}

CLIB_API RS cl_sa_ah_ctrl_temp(cl_handle_t dev_handle, u_int8_t is_add)
{
	return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_TEMP, !!is_add);
}
    
CLIB_API RS cl_sa_ah_ctrl_timer(cl_handle_t dev_handle, u_int8_t is_add)
{
	return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_TIMER, !!is_add);
}

CLIB_API RS cl_sa_ah_ctrl_eco(cl_handle_t dev_handle, u_int8_t is_eco)
{
	return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_ECO, !!is_eco);
}

CLIB_API RS cl_sa_ah_ctrl_shake(cl_handle_t dev_handle, bool is_shake)
{
	return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_SHAKE, !!is_shake);
}

CLIB_API void cl_sa_ah_refresh_power_and_timer(cl_handle_t dev_handle)
{
	cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_REFRESH_POWER,0);
}

CLIB_API RS cl_sa_ah_ctrl_test(cl_handle_t dev_handle, u_int8_t timeout)
{
    return cl_sa_ah_ctrl_common(dev_handle, CLNE_AH_TEST, timeout);
}

