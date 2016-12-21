/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: APP上层二次开发通道
**  File:    app_pkt_priv.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    07/26/2016
**
**  Purpose:
**    APP上层二次开发通道.
**************************************************************************/


#ifndef APP_PKT_PRIV_H
#define APP_PKT_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_app_pkt.h"
#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "udp_ctrl.h"
#include "linkage_client.h"

/* Macro constant definitions. */


/* Type definitions. */


/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

RS app_send_pkt(PEER_TYPE_T peer_type, u_int64_t ident, u_int8_t *pkt, int pkt_len);

RS app_proc_pkt_from_server(u_int8_t *data, int data_len);
RS app_send_pkt(PEER_TYPE_T peer_type, u_int64_t ident, u_int8_t *pkt, int pkt_len);

u_int8_t *app_get_developer_id(void);

RS app_proc_pkt_from_wifi_dev(u_int64_t sn, u_int8_t *data, int data_len);
RS app_proc_pkt_from_macbee_dev(u_int64_t sn, u_int8_t *data, int data_len);
RS app_proc_pkt_from_macbee_dev_cache(u_int64_t sn, u_int8_t *data, int data_len);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* APP_PKT_PRIV_H */

