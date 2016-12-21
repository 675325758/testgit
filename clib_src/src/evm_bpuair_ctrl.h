/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: evm_bpuair_ctrl
**  File:    evm_bpuair_ctrl.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/05/2016
**
**  Purpose:
**    Evm_bpuair_ctrl.
**************************************************************************/


#ifndef EVM_BPUAIR_CTRL_H
#define EVM_BPUAIR_CTRL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"


/* Macro constant definitions. */


/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	bool once_timer_enable;
	bool once_timer_onoff;	// 一次性定时器设置 0：定时关机1：定时开机
	u_int8_t once_timer_hour;	// 一次性定时器小时
	u_int8_t once_timer_min;	// 一次性定时器分钟
} once_timer_t;

typedef struct {
	u_int32_t time;
	bool onoff;
} nearest_timer_t;


typedef struct {
	u_int8_t set;
	u_int8_t hour;
	u_int8_t min;
} bpuair_tt_period_timer_t;

typedef struct {
	/*
		
	远程开关功能：
	Bit1：开命令，0-无效，1-有效；Bit0：关命令，0-无效，1-有效；
	工作模式切换:（单暖只只有制热，冷暖制冷制热都支持）
	Bit2：0-制冷，1-制热；
	节能模式切换:
	Bit3：0-关闭，1-开启；
		
	*/
	u_int8_t onoff;
	u_int16_t cold_temp;
	u_int16_t hot_temp;
	/*
		
	定时设置
	Bit0：定时总开关（0：不使用1：使用） Bit1：备用
	Bit2：备用							  Bit3：备用
	Bit4：备用							  Bit5：备用
	Bit6：备用							  Bit7：一次定时设置（0：定时关机1：定时开机）
	*/
	u_int8_t timer_set;
	u_int8_t once_timer_hour;
	u_int8_t once_timer_min;

	bpuair_tt_period_timer_t period_timer[6];
	u_int8_t pad;
} bpuair_tt_ctrl_t;

typedef struct {
	u_int8_t type;	// 设备机型
	/*
		
		0：待机；
		1：运行；
		2：正在停机；
	*/
	u_int8_t stat1; 
	/*
		0:无
		1：防冻；
		2：除霜；
		3：防冻和除霜；
	*/
	u_int8_t stat2;
	u_int8_t work_mode; // 工作模式 1制冷 2 制热
	
	u_int8_t eco_mode;	// 节能模式0 关闭 1开启
	int16_t cold_temp;	// 制冷设定温度，单位0.1度，10.0 ~ 30.0
	int16_t hot_temp;		// 制热设定温度，单位0.1度，10.0 ~ 85.0
	int16_t backwater_temp; // 回水温度单位0.1度
	int16_t water_temp; // 系统出水温度单位0.1度
	int16_t env_temp;	// 环境温度单位0.1度
	int16_t coiler1_temp;	// 盘管温度单位0.1度
	int16_t coiler2_temp;	// 盘管温度单位0.1度
	int16_t current1;	// 电流1 单位A
	int16_t current2;	// 电流2 单位A
	int16_t eco_cold_temp;	// 制冷节能设定温度（范围：10.0°C~30.0°C）
	int16_t eco_hot_temp;	// 制热节能设定温度（范围：10.0°C~85.0°C）
	u_int8_t pad[3];
	u_int8_t err[8];

	u_int8_t timer_set;
	u_int8_t once_timer_hour;
	u_int8_t once_timer_min;

	bpuair_tt_period_timer_t period_timer[6];
	u_int16_t uptime;	// 累计运行小时
} bpuair_tt_stat_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
bool bpuair_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool bpuair_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool bpuair_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool bpuair_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* EVM_BPUAIR_CTRL_H */

