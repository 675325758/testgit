/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: linkon温控器
**  File:    cl_linkon.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    02/17/2016
**
**  Purpose:
**    Linkon温控器.
**************************************************************************/


#ifndef CL_LINKON_H
#define CL_LINKON_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
// LINKON温控器
enum {
	ACT_LINKON_POWER,	// 0 关机 1 开机
	ACT_LINKON_LOCK,	//  0 关闭童锁 1 开启童锁
	ACT_LINKON_WORK_MODE,	// 0 恒温模式 1 节能模式2离家模式
	ACT_LINKON_RUNNING_MODE,	// 0 加热 1制冷2换气
	ACT_LINKON_WIND_SPEED,	// 0低风 2 中风 3 高风
	ACT_LINKON_TEMP,	// 设置温度，单位0.1度
};


/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	u_int8_t power;	// 0:关机 1：开机
	u_int8_t lock;	// 0-童锁关 1-童锁开
	int16_t house_temp;	// 室温，可能为负数,单位0.1度
	u_int16_t const_temp;	// 恒温温度,单位0.1度
	u_int16_t go_out_temp;	// 离家温度,单位0.1度
	u_int16_t save_temp;	// 节能温度,单位0.1度
	u_int8_t humidity;	// 湿度
	u_int8_t work_mode;	// 工作模式，0-恒温，1-节能，2-离家
	u_int8_t running_mode;	// 运行模式，0-加热，1-制冷，2-换气
	u_int8_t wind_speed;	// 风速，0-低风，1-中风，2-高风
} cl_linkon_stat_t;

typedef struct {
	cl_linkon_stat_t stat;
} cl_linkon_info_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	功能:LINKON温控器基本控制接口
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型,ACT_LINKON_XXX
		@value: 控制值
	输出参数:
		无
	返回:

		
*/
CLIB_API RS cl_linkon_sample_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_LINKON_H */

