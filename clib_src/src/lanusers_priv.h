/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 用户上下线信息管理
**  File:    lanusers_priv.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    04/18/2016
**
**  Purpose:
**    Xxx.
**************************************************************************/


#ifndef LANUSERS_PRIV_H
#define LANUSERS_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_lanusers.h"

/* Macro constant definitions. */


/* Type definitions. */
typedef struct {
	u_int32_t magic;
	u_int32_t min_idx;
	u_int32_t record_num;
	lanusers_manage_user_record_item_t items[0];
} lanusers_record_hd_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

bool lanusers_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

bool lanusers_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LANUSERS_PRIV_H */

