#ifndef	__SMART_CONFIG_PRIV_H__
#define	__SMART_CONFIG_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

extern void smart_config_init();
extern void smart_config_clean();
extern RS smart_config_start(cl_notify_pkt_t *pkt);
extern RS smart_config_start_ext(cl_notify_pkt_t *pkt);
extern RS smart_config_start_advance(cl_notify_pkt_t *pkt);
extern RS smart_6621_config_start(cl_notify_pkt_t *pkt);
extern RS smart_phone_wlan_config_start(cl_notify_pkt_t *pkt);
extern RS smart_config_mbroadcast_start(cl_notify_pkt_t *pkt);
extern RS smart_config_stop( );
extern RS smart_config_mbroadcast_start_hotspot(cl_notify_pkt_t *pkt);


#ifdef __cplusplus
}
#endif 


#endif

