/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: zkrsq_priv.h
**  File:    zkrsq_priv.h
**  Author:  liubenlong
**  Date:    11/16/2015
**
**  Purpose:
**    E:\µÍ¹¦ÂÊwifi\new-win-sdk\inc.
**************************************************************************/


#ifndef ZKRSQ_PRIV_H
#define ZKRSQ_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_zkrsq.h"

/* Macro constant definitions. */


/* Type definitions. */
#pragma pack(push, 1)
typedef struct {
	u_int8_t onoff;
	u_int8_t pad[3];
}ucp_zkrsq_on_t;

typedef struct {
	u_int8_t mode;
	u_int8_t pad[3];
}ucp_zkrsq_mode_t;

typedef struct {
	u_int8_t tmp;
	u_int8_t pad[3];
}ucp_zkrsq_tmp_t;

typedef struct {
	u_int8_t time;
	u_int8_t pad[3];
}ucp_zkrsq_time_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t hour;
	u_int8_t min;
	u_int8_t hour_end;
	u_int8_t min_end;
	u_int8_t pad[3];
}ucp_zkrsq_timer_t;

typedef struct {
	u_int8_t fault;
	u_int8_t pad[3];
}ucp_zkrsq_fault_t;

typedef struct {
	u_int8_t reset;
	u_int8_t pad[3];
}ucp_zkrsq_reset_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
bool _zkrsq_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool zkrsq_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ZKRSQ_PRIV_H */

