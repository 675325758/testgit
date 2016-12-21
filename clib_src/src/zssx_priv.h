/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: zssx_priv.h
**  File:    zssx_priv.h
**  Author:  liubenlong
**  Date:    09/30/2015
**
**  Purpose:
**    中山商贤.
**************************************************************************/


#ifndef ZSSX_PRIV_H
#define ZSSX_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_zssx.h"

/* Macro constant definitions. */


/* Type definitions. */
#pragma pack(push, 1)
//开关机和屏显，指令53

typedef struct {
#if 0
#if __BYTE_ORDER == __LITTLE_ENDIAN  
	u_int8_t pad:2,
		tmp_onoff:1,
		led_onoff:1, 
		tmp_type:1,
		anion_onoff:1,
		lock_onoff:1,
		power_onoff:1;

#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t power_onoff:1,
		lock_onoff:1, 
		anion_onoff:1,
		tmp_type:1,
		led_onoff:1,
		tmp_onoff:1,
		pad:2;
#else
# error "Please fix <bits/endian.h>"
#endif
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN  
	u_int8_t pad:2,
		led_onoff:1,
		tmp_onoff:1, 
		tmp_type:1,
		anion_onoff:1,
		lock_onoff:1,
		power_onoff:1;

#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t power_onoff:1,
		lock_onoff:1, 
		anion_onoff:1,
		tmp_type:1,
		tmp_onoff:1,
		led_onoff:1,
		pad:2;
#else
# error "Please fix <bits/endian.h>"
#endif

#endif

}zssx_on_info_t;

typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN  
	u_int8_t fake_firewood:4,
		fire_level:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t fire_level:4,
		fake_firewood:4;

#else
# error "Please fix <bits/endian.h>"
#endif
}zssx_set_status1_t;

typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN  
u_int8_t speed_gears:4,
	pad:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
u_int8_t pad:4,
	speed_gears:4;
#else
# error "Please fix <bits/endian.h>"
#endif
}zssx_set_status2_t;

typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN  
	u_int8_t timer_hour:5,
		pad:1,
		timer_off:1,
		timer_on:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t timer_on:1,
		timer_off:1,
		pad:1,
		timer_hour:5;
#else
# error "Please fix <bits/endian.h>"
#endif
}zssx_timer_hour_t;

typedef struct {
	zssx_on_info_t onoff;
	zssx_set_status1_t set_status1;
	zssx_set_status2_t set_status2;
	u_int8_t tmp;
	zssx_timer_hour_t timer_hour;
	u_int8_t timer_min;
	u_int8_t ntc_fault;
	u_int8_t thermostat_fault;
	u_int8_t work_status;
	u_int8_t pad[3];
}ucp_zssx_work_t;

typedef struct {
	u_int8_t ssid[36];
	u_int8_t pswd[68];
}ucp_zssx_wifi_t;


#pragma pack(pop)

typedef struct {
	cl_zssx_info_t info;
	ucp_zssx_work_t set_work;
}ucp_zssx_priv_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
bool _zssx_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
bool _zssx_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj);
bool zssx_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ZSSX_PRIV_H */

