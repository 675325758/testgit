/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: yt_priv.h
**  File:    yt_priv.h
**  Author:  liubenlong
**  Date:    09/07/2015
**
**  Purpose:
**    ‘¬Õ√.
**************************************************************************/


#ifndef YT_PRIV_H
#define YT_PRIV_H


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
bool _yt_sn(user_t *user, u_int8_t *ptmp, RS *ret);
bool _yt_query_phase_ele(user_t *user, u_int8_t *ptmp, RS *ret) ;
void ac_type_order(cl_yt_ac_type_t *ptmp);
char *ac_type_get_name(u_int16_t index);
u_int8_t ac_sys_type_get(u_int16_t index);
bool yt_get_ac_info(u_int8_t *psn, cl_yt_ac_type_t *ptmp);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* YT_PRIV_H */

