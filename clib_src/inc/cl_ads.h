/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_ads.h
**  File:    cl_ads.h
**  Author:  liubenlong
**  Date:    08/25/2015
**
**  Purpose:
**    澳德绅热水器.
**************************************************************************/


#ifndef CL_ADS_H
#define CL_ADS_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
typedef enum {
	ADS_ACTION_ONOFF = 1,//热水器开关
	ADS_ACTION_BW_ONOFF,//回水设置开关
	ADS_ACTION_TMP,//温度设置
	ADS_ACTION_MAX,
}ADS_ACTION_T;

/* Type definitions. */
#pragma pack(push, 1)

typedef struct{
	u_int8_t water_box_show_tmp;
	u_int8_t water_box_ctrl_tmp;
	u_int8_t scroll_tmp;
	u_int8_t exhaust_tmp;

	u_int8_t out_water_tmp;
	u_int8_t env_tmp;
	u_int8_t back_diff_tmp;
	u_int8_t back_water_tmp;

	u_int8_t compressor_cur;
	u_int8_t compressor_work_time_l;
	u_int8_t compressor_work_time_h;
	u_int8_t sys_uptime_l;
	
	u_int8_t sys_uptime_h;
	u_int8_t defrost_time;
	u_int8_t defrost_tmp;
	u_int8_t water_box_set_tmp;

	u_int8_t first_start_time_h;
	u_int8_t first_start_time_m;
	u_int8_t first_end_time_h;
	u_int8_t first_end_time_m;
	
	u_int8_t second_start_time_h;
	u_int8_t second_start_time_m;
	u_int8_t second_end_time_h;
	u_int8_t second_end_time_m;

	u_int8_t back_water_status;//当成快热开关状态
	u_int8_t defrost_status;
	u_int8_t compressor_status;//压缩机开关
	u_int8_t fault_code;

	u_int8_t work_mode;//模式当做开关状态
	u_int8_t sys_type;
	u_int8_t water_level;
	u_int8_t water_box_tmp2;

	u_int8_t water_box_tmp3;
	u_int8_t water_box_tmp4;
	u_int8_t water_pump_arg1;
	u_int8_t water_pump_arg2;

	u_int8_t fault_code2;
	u_int8_t ele_valve_l;
	u_int8_t ele_valve_h;
	u_int8_t pump_clock_h;

	u_int8_t pump_clock_m;
	u_int8_t wind_status;
	u_int8_t pad[6];
}cl_ads_stat_t;

typedef struct {
	u_int16_t sys_clock_time;//系统时间(分钟)
	u_int16_t first_start_time;//第一次定时器加热开始时间(分钟)
	u_int16_t first_end_time;//第一次定时器加热结束时间(分钟)
	u_int16_t second_start_time;//第二次定时器加热开始时间(分钟)
	u_int16_t second_end_time;//第二次定时器加热结束时间(分钟)

	u_int16_t defrost_time;//除霜间隔时间，0-240分钟
	u_int16_t defrost_tmp;//除霜进入温度，-15~5度
	u_int8_t pad[2];
}cl_ads_conf_t;

#pragma pack(pop)
/*
除霜温度：0°~-9°C
排气温度：-300°C--130°C
盘管温度，环境温度，水箱温度，出水温度，集热温度等：-30°C--90°C.
运行时间：0--99999小时

东东 2015/9/22 14:17:41
只是除霜，其他是-30-90，排气温度是-30-130
14:20:43
大时空 2015/9/22 14:20:43
哦，那除霜这个怎么算，0-15，-30是-30~-15，这个是除霜范围吗

东东 2015/9/22 14:21:21
设置范围是0--9.

错误码:
		fault_code								fault_code2
bit0		E7：0无故障，1故障						不是故障，不管//循环泵 0停止,1为工作（盘管机时恒为0）

bit1		E5：0无故障，1故障						不是故障，不管//四通阀 0停止,1为工作

bit2		E4：0无故障，1故障						不是故障，不管//定时：0无效,1为有效(定时显示)

bit3		E3：0无故障，1故障						不是故障，不管//低压开关：0为无效，1有效

bit4		E2：0无故障，1故障						E12：0无故障，1故障 四通阀故障

bit5		E1：0无故障，1故障						E10：0无故障，1故障

bit6		E0：0无故障，1故障						E9：0无故障，1故障

bit7		不可运行故障; 0为未出现1为出现.			E8：0无故障，1故障

注意:
E7,E8,E10故障不影响运行。即只要出现e0-e6,e12就不能控制设备了。

*/
typedef struct {
	//可设置区域
	u_int8_t on;//热水器开关，1表示开，0表示关
	u_int8_t back_water_on;//直接当成快热开关好了，避免改jni，直接用
	//u_int8_t tmp;//设置温度，30~60 摄氏度
	cl_ads_conf_t conf;
	

	//显示区域
	u_int16_t water_box_show_tmp;//水箱显示温度，-30-90 摄氏度
	u_int16_t water_box_ctrl_tmp;//设定温度,-30-90 摄氏度
	u_int16_t out_water_tmp;//出水问题，-30-90 摄氏度
	u_int16_t env_tmp;//环境温度，-30-90 摄氏度
	u_int16_t back_diff_tmp;//回差温度，-30-90 摄氏度
	u_int16_t compressor_cur;//压缩机电流，单位还不知道，对方还没说。0.1a
	u_int16_t scroll_tmp;//盘管温度
	u_int16_t exhaust_tmp;//排气温度
	u_int8_t defrost;//除霜状态，0表示没有除霜，1表示除霜
	u_int8_t wind;//风机状态，0表示关，1表示开
	u_int8_t fault_code;//错误代码
	u_int16_t pump_clock;//热泵系统时间，分钟，可换算成时分，显示热泵当前系统时间
	u_int8_t fault_code2;//错误代码
	u_int16_t water_show_tmp;//水箱控制温度显示-30-90 摄氏度

	u_int8_t compressor_status;//压缩机状态，app可以不管


	//一下保证数据，sdk用，app可以不管。。。。。。。。。。。
	cl_ads_stat_t stat;
}cl_ads_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	功能:
		主界面控制命令
	输入参数:
		@dev_handle: 设备的句柄
		@action:控制行为
		@value:
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ads_ctrl(cl_handle_t dev_handle, ADS_ACTION_T action, u_int8_t value);

/*
	功能:
		主界面控制命令
	输入参数:
		@dev_handle: 设备的句柄
		@pconf:配置设置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ads_conf(cl_handle_t dev_handle,  cl_ads_conf_t *pconf);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ADS_H */

