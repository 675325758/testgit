/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zssx.h
**  File:    cl_zssx.h
**  Author:  liubenlong
**  Date:    09/30/2015
**
**  Purpose:
**    中山商贤电烤炉.
**************************************************************************/


#ifndef CL_ZSSX_H
#define CL_ZSSX_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */
typedef enum {
	ZSSX_ACTION_POWER_ONOFF,//开关
	ZSSX_ACTION_LOCK_ONOFF,//锁开关
	ZSSX_ACTION_ANION_ONOFF,//负离子开关
	ZSSX_ACTION_LED_ONOFF,//灯光开关
	ZSSX_ACTION_FIRE_LEVEL,//火焰级别
	ZSSX_ACTION_FAKE_LEVEL,//加柴级别
	ZSSX_ACTION_GEARS,//档位设定
	ZSSX_ACTION_TMP_TYPE,//温度类型设置
	ZSSX_ACTION_TMP_SET,//温度设置
	ZSSX_ACTION_TMP_ONOFF,//温度开关
}ZSSX_ACTION_T;

typedef struct {
	char ssid[33];
	char pswd[65];
}cl_zssx_ssid_t;

typedef struct {
	//开关
	u_int8_t power_onoff;//0-关，1-开
	u_int8_t lock_onoff;//童锁[App界面上没有]，0-关，1-开
	u_int8_t anion_onoff;//负离子开关,0-关，1-开
	u_int8_t tmp_type;//温度类型，0-摄氏度，1-华氏度
	u_int8_t led_onoff;//灯光[App界面上没有控制]，0-关，1-开；
	u_int8_t tmp_onoff;//温度开关，0-关，1-开
	//工作设定状态1
	u_int8_t fire_level;//火焰级别 0:关;01:L1;02:L2-------09:L9，共9级
	u_int8_t fake_firewood;//假柴级别 0:关;01:L1;02:L2-------09:L9，共9级
	//工作设定状态2
	u_int8_t speed_gears;//无极调速:00:D1;02:D2----08:D9 共9档
	u_int8_t timer_on;//定时开，0-无，1-定开
	u_int8_t timer_off;//定时关，0-无，1-定关
	u_int8_t timer_hour;//定时小时，0-23
	u_int8_t timer_min;//定时分钟，0-59
	//设定温度
	u_int8_t tmp;//设定温度，没有负数
	//故障
	u_int8_t ntc_fault;//NTC故障
	u_int8_t thermostat_fault;//温控器故障
	//工作显示状态
	u_int8_t work_status;//Bit0：电热工作：0-关，1-开
	//ap模式
	u_int8_t wifi_mode;//0=sta,1=ap

	//wifi config
	cl_zssx_ssid_t wifi_conf;
}cl_zssx_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	功能:
		主界面控制命令
	输入参数:
		@dev_handle: 设备的句柄
		@action:控制行为
		@value:值
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zssx_ctrl(cl_handle_t dev_handle, ZSSX_ACTION_T action, u_int8_t value);

/*
	功能:
		定时器
	输入参数:
		@dev_handle: 设备的句柄
		@on:定时器开关，0-定时关，1-定时开
		@min:定时器的设置时间，用分钟表示
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zssx_timer(cl_handle_t dev_handle, u_int8_t on, u_int16_t min);

/*
	功能:
		wifi配置
	输入参数:
		@dev_handle: 设备的句柄
		@wifi:wifi ssid, pswd配置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zssx_wifi_conf(cl_handle_t dev_handle, cl_zssx_ssid_t *wifi);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZSSX_H */

