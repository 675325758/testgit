/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 进睿芯光芒热水器
**  File:    evm_jrxheater_ctrl.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/06/2016
**
**  Purpose:
**    进睿芯光芒热水器.
**************************************************************************/


#ifndef EVM_JRXHEATER_CTRL_H
#define EVM_JRXHEATER_CTRL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"
#include "cl_jrxheater.h"


/* Macro constant definitions. */
#define JRX_UART_CMD_PUSH_STAT 52
#define JRX_UART_CMD_STAT 53
#define JRX_UART_CMD_ONCE_TIMER 54
#define JRX_UART_CMD_PERIOD_TIMER 55



/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t cmd;
	u_int8_t dev_type;
	u_int8_t param_len;
} jrxheater_uart_hd_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t set_temp;
	u_int8_t childlock;
} jrxheater_uart_set_stat_t;

typedef struct {
	u_int8_t status;
	u_int8_t id;
	u_int8_t hour;
	u_int8_t min;
	u_int8_t temp;
} jrxheater_uart_once_timer_t;

typedef struct {
	u_int8_t status;
	u_int8_t id;
	u_int8_t start_hour;
	u_int8_t start_min;
	u_int8_t end_hour;
	u_int8_t end_min;
	u_int8_t onoff;
	u_int8_t temp;
} jrxheater_uart_period_timer_t;


typedef struct {
	u_int8_t onoff;	// 设备开关：0-关机 1-开机
	u_int8_t mode;	// 0-经济模式 1-舒适模式 2-全天候模式 3-个性化模式
	u_int8_t set_temp; // 用户设置热水器工作的温度，范围20-80摄氏度
	u_int8_t child_lock; // 童锁开关：0-关闭 1-开启
	u_int8_t work_state; // 状态：1-加热 2-保温
	u_int8_t temp;	// 水箱温度：当前水箱的温度，范围20-80摄氏度
	u_int8_t capacity;	// 容量：水箱大小，单位L
	u_int8_t power; // 热水器功率，单位W

	// APP页面需要显示热水器达到水箱温度的时间
	u_int8_t hour;
	u_int8_t min;
} jrxheater_uart_stat_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

bool jrxheater_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool jrxheater_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool jrxheater_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool jrxheater_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
/*
	功能: 基本设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_JRXHEATER_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jrxheater_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* EVM_JRXHEATER_CTRL_H */

