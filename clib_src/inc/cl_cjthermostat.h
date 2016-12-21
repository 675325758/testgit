/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 创佳温控器
**  File:    cl_cjthermostat.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    02/18/2016
**
**  Purpose:
**    创佳温控器.
**************************************************************************/


#ifndef CL_CJTHERMOSTAT_H
#define CL_CJTHERMOSTAT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */

enum {
	ACT_CJTHERMOSTAT_LOCK,	// 0  开放面板 1 锁面板
	ACT_CJTHERMOSTAT_POWER,	//  0 关机 1 开机面板显示2 开机面板熄灭
	ACT_CJTHERMOSTAT_TIMER_WEEK,	// 自动周期，bit0 星期天 bit1-6 星期1到6
	ACT_CJTHERMOSTAT_TEMP_ADJUST,	// 温校-9 到 9
	ACT_CJTHERMOSTAT_TEMP_UPPER_LIMIT,	// 温设上限
	ACT_CJTHERMOSTAT_TEMP_LOWER_LIMIT,	// 温设下限
	ACT_CJTHERMOSTAT_TEMP_ALLOWANCE,	// 温控容差
	ACT_CJTHERMOSTAT_DEFROST_TEMP,	// 除霜温度
	ACT_CJTHERMOSTAT_OVERTEMP,	// 过热温度
	ACT_CJTHERMOSTAT_OVERTEMP_ALLOWANCE,	// 过热容差
	ACT_CJTHERMOSTAT_FLAG,				// bit 0 型号:0: S型1:D型  bit 1 预置标志: 0 住宅模式1办公模式
	ACT_CJTHERMOSTAT_MODE,	// 工作模式1 工作 2 手动3 休息4 自动 5 除霜 6 预置
	ACT_CJTHERMOSTAT_MANUAL_TEMP,	// 手动模式温度设定
	ACT_CJTHERMOSTAT_RESET,	// 复位
	// 上面部分使用cl_zkcleanner_ctrl 接口
	
	ACT_CJTHERMOSTAT_PERIOD_TEMP_WORKDAY,
	ACT_CJTHERMOSTAT_PERIOD_TEMP_OFFDAY,
};

enum {
    CJ_WORKMODE_TYPE_WORK = 1,
    CJ_WORKMODE_TYPE_MANUAL,
    CJ_WORKMODE_TYPE_REST,
    CJ_WORKMODE_TYPE_AUTO,
    CJ_WORKMODE_TYPE_DEFORST,
    CJ_WORKMODE_TYPE_PRESET,
};

/* Type definitions. */
typedef struct {
	u_int16_t outtime_hour;	// 输出时长，小时
	u_int8_t outtime_min;	// 输出时长，分钟
	u_int8_t ver;	// 版本号
	u_int8_t is_heat;	// 是否启动加热 0 未启动 1 启动
	u_int8_t week;	// 温控器星期
	u_int8_t time;	// 当前时间0-240,单位为6分钟
	u_int8_t stat;	// 0键无操作1 一般操作 2 后台设定 3 上述两种  “0：键无操作”是指温控器上电后或接到过清键指令后，没有再对按键进行过操作， 也可以理解为暂无操作。 这个UI上现在没有用到。
	u_int8_t set_temp;	// 设定温度，正数
	u_int8_t inside_temp;		// 内探头温度，整数部分。正数
	u_int8_t inside_temp1;	// 内抬头温度，小数部分
	u_int8_t outside_temp;	// 外抬头温度，整数部分正数
	u_int8_t outside_temp1;	// 外抬头温度，小数部分
	u_int8_t mode;	// 模式1 工作 2 手动3 休息4 自动 5 除霜 6 预置
	u_int8_t power;	// 0 关机1 开机屏幕显示2开机屏幕关闭
	u_int8_t key_lock;	// 0 键开放 1 键锁定
	u_int8_t fault;	// 错误 0 正常 1 测温探头短2测温探头开3保护探头短4保护过热
	int8_t temp_adjust;	// 温校:-9～9
	u_int8_t set_temp_upper_limit;	// 温度设置上限
	u_int8_t set_temp_lower_limit;	// 温度设置下限
	u_int8_t temp_allowance;	// 温控容差
	u_int8_t defrost_temp;	// 除霜温度
	u_int8_t overtemp;	// 过热温度
	u_int8_t overtemp_allowance;	// 过热容差
	u_int8_t flag;	// bit 0 型号:0: S型1:D型  bit 1 预置标志: 0 住宅模式1办公模式
	u_int8_t timer_week;	// 自动周期设置bit0-bit7表示星期7。1 ~6
	u_int8_t manual_temp;	// 手动模式温度设定
} cl_cjthermostat_stat_t;

typedef struct {
	cl_cjthermostat_stat_t stat;
	u_int8_t work_period_temp[48];		// 工作日48个时段温度设置，每半小时一个点
	u_int8_t offday_period_temp[48];	// 休息日48个时段温度设置，每半小时一个点
} cl_cjthermostat_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	功能: 对创佳温控器的基本状态设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_CJTHERMOSTAT_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_cjthermostat_ctrl(cl_handle_t dev_handle, u_int8_t action, int8_t value);

/*
	功能: 对创佳温控器的周期时间进行设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@type: 0 设置工作日温度 1 设置休息日温度
		@temp: 48个温度点，每个点表示半小时
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_cjthermostat_period_temp(cl_handle_t dev_handle, u_int8_t type, u_int8_t temp[48]);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_CJTHERMOSTAT_H */

