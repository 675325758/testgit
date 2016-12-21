/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: xy_priv.h
**  File:    xy_priv.h
**  Author:  liubenlong
**  Date:    07/16/2015
**
**  Purpose:
**    鑫源私有文件.
**************************************************************************/


#ifndef XY_PRIV_H
#define XY_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */


/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
void _free_xy_info(cl_xy_info_t* info);
bool xy_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
bool _xy_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* XY_PRIV_H */

