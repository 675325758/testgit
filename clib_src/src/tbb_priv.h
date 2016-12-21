/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: tbb_priv.h
**  File:    tbb_priv.h
**  Author:  liubenlong
**  Date:    08/10/2015
**
**  Purpose:
**    Õÿ∞Ó…Ã”√.
**************************************************************************/


#ifndef TBB_PRIV_H
#define TBB_PRIV_H


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
bool tbb_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
bool _tbb_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TBB_PRIV_H */

