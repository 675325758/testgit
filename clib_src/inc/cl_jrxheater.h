/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 进睿芯光芒热水器
**  File:    cl_jrxheater.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/06/2016
**
**  Purpose:
**    进睿芯光芒热水器.
**************************************************************************/


#ifndef CL_JRXHEATER_H
#define CL_JRXHEATER_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
enum {
	ACT_JRXHEATER_ONOFF,
	ACT_JRXHEATER_MODE,
	ACT_JRXHEATER_TEMP,
	ACT_JRXHEATER_CHILDLOCK,
	ACT_JRXHEATER_PERIOD_TIMER,
};

/* Type definitions. */
typedef struct {
	u_int8_t valid;
	u_int8_t enable;
	u_int8_t id;	// ID 从1开始
	u_int8_t hour;
	u_int8_t min;
	u_int8_t set_temp;	// 定时器到点后设置温度，范围20-80摄氏度
} cl_jrxheater_sample_timer_t;

typedef struct {
	u_int8_t valid;
	u_int8_t enable;
	u_int8_t id;	// ID 从1开始
	u_int8_t start_hour;
	u_int8_t start_min;
	u_int8_t end_hour;
	u_int8_t end_min;
	u_int8_t set_temp;	// 定时器到点后设置温度，范围20-80摄氏度
} cl_jrxheater_period_timer_t;


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
} cl_jrxheater_stat_t;

typedef struct {
	cl_jrxheater_stat_t stat;
	cl_jrxheater_sample_timer_t sample_timer[8];
	cl_jrxheater_period_timer_t period_timer[8];
} cl_jrxheater_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */


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

/*
	功能: 周期定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 操作1 添加2 关闭 3 修改 4 删除
		@valid: 是否存在
		@enable: 是否使能
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jrxheater_period_timer(cl_handle_t dev_handle, u_int8_t action, u_int8_t valid, u_int8_t enable, u_int8_t id, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min, u_int8_t temp);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_JRXHEATER_H */

