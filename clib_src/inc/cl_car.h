/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_car.h
**  File:    cl_car.h
**  Author:  liubenlong
**  Date:    06/23/2015
**
**  Purpose:
**    Cl_car.h.
**************************************************************************/


#ifndef CL_CAR_H
#define CL_CAR_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */

/* Type definitions. */
#pragma pack(push,1)
typedef struct {
	u_int32_t time;//报警产生时间
	u_int8_t id;//报警信息id
	u_int8_t pad[3];
}cl_car_alarm_t;
#pragma pack(pop)


typedef struct {
	u_int8_t on;//发动机开关状态
	//配置参数
	u_int8_t on_keep_time;//开启发动机时间，分钟为单位。

	//剩余电量百分比，先写着，还没找到算法,0-100
	u_int8_t ele_percentage;

	u_int8_t horn_num;//喇叭响次数
	u_int8_t horn_time;//喇叭持续响时间，以0.1s为单位
	u_int8_t horn_interval;//两次喇叭响的间隔时间,以s为单位
	u_int8_t light_num;//灯光闪烁次数
	u_int8_t light_time;//灯光闪烁持续时间
	u_int8_t light_interval;//两次灯光闪烁间隔时间

	//状态参数
	u_int8_t temp;//车内温度 
	u_int8_t val_on;//安全电压检测开关
	u_int16_t valtage;//安全电压单位是0.1v
	u_int8_t powersave_on;//节能开关
	u_int8_t last_on_time;//上次开启发动机以来的时间，以分钟为单位

	//警告信息;
	u_int16_t alarm_num;//报警消息个数
	//警告信息数组，与alarm_num结合解析数据
	cl_car_alarm_t *car_alarm;
	//debuginfo
	char *debug_info;
}cl_car_info_t;

//寻车功能参数
typedef struct {
	u_int8_t horn_num;
	u_int8_t horn_time;
	u_int8_t horn_interval;
	u_int8_t light_num;
	u_int8_t light_time;
	u_int8_t light_interval;
}cl_car_search_t;

//事件
enum {
	CAR_CTRL_EVENT_ON = 1,//开启空调
	CAR_CTRL_EVENT_OFF,//关闭空调
	CAR_ELE_EVENT_FULL,//已充满电
	CAR_ELE_EVENT_FILLING,//正在充电
	CAR_ELE_EVENT_LESS,//电量不足，请充电
	CAR_CTRL_ALARM_ON_FAILED = 100,//开启发动机失败,警告类
	CAR_CTRL_ALARM_AC_ON_FAILED,//开启空调失败，空调未开或坏了
	CAR_CTRL_ALARM_VALTAGE,//电瓶电压错误
};

/* External function declarations. */

/* Macro API definitions. */


/* Global variable declarations. */
/*
 功能:
	配置发动机开启时间
 输入参数:
    @dev_handle: 设备的句柄
    @keep_time: 配置开机持续时间
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_config_keeptime(cl_handle_t dev_handle, u_int8_t keep_time);

/*
 功能:
	发动机开关
 输入参数:
    @dev_handle: 设备的句柄
    @on: 0，关闭，1开启
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_ctrl_on(cl_handle_t dev_handle, u_int8_t on);   

/*
 功能:
	寻车功能参数配置
 输入参数:
    @dev_handle: 设备的句柄
    @search: 配置寻车功能
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_config_search(cl_handle_t dev_handle, cl_car_search_t *search);

/*
 功能:
	开启寻车功能
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_ctrl_search(cl_handle_t dev_handle);


/*
 功能:
	安全电压配置
 输入参数:
    @dev_handle: 设备的句柄
    @valtage: 安全电压单位是0.1v
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_config_valtage(cl_handle_t dev_handle, u_int16_t valtage);


/*
 功能:
	安全电压功能开关
 输入参数:
    @dev_handle: 设备的句柄
    @on: 安全电压开关，1开，0关
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_ctrl_valtage(cl_handle_t dev_handle, u_int8_t on);

/*
 功能:
	节能功能开关
 输入参数:
    @dev_handle: 设备的句柄
    @on: 开关，1开，0关
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_car_ctrl_powersave(cl_handle_t dev_handle, u_int8_t on);



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_CAR_H */

