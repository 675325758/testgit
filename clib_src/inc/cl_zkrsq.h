/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zkrsq.h
**  File:    cl_zkrsq.h
**  Author:  liubenlong
**  Date:    11/16/2015
**
**  Purpose:
**    智科热水器.
**************************************************************************/


#ifndef CL_ZKRSQ_H
#define CL_ZKRSQ_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */

typedef enum {
	ZKRSQ_ACTION_ONOFF,//开关
	ZKRSQ_ACTION_MODE,// 模式
	ZKRSQ_ACTION_TMP,//温度
}ZKRSQ_ACTION_T;

/*
	fault:
		BIT0	0	RESERVED
		BIT1	1	通讯故障
		BIT2	1	水箱温度故障
		BIT3	1	环境温度故障
		BIT4	1	盘管温度故障
		BIT5	1	回水温度故障
		BIT6	1	高压保护
		BIT7	1	低压保护
*/

typedef struct {
	int8_t back_tmp;//回差温度 1℃ —10℃
	int8_t compensation_tmp;//补偿温度 0℃ —10℃
	u_int8_t defrost_time;//除霜时间，范围 20MIN – 90MIN 默认45MIN
	int8_t defrost_in_tmp;//除霜进入温度 -9℃—5℃
	u_int8_t defrost_continue_time;//除霜持续时间 3MIN – 12MIN 默认8MIN
	int8_t defrost_out_tmp;//除霜退出温度	5℃ —25℃
	int8_t back_water_tmp;//回水温度 5℃—60℃
	u_int8_t back_water_mode;//回水模式 0:自动模式/1:普通模式；
}cl_zk_confset_t;

typedef struct{
	u_int8_t timer1_valid;//是否有效
	u_int8_t timer1_onoff;//1:on, 0:off
	u_int8_t timer1_hour;//定时时间，小时
	u_int8_t timer1_min;//定时时间，分钟
	u_int8_t timer1_hour_end;//定时时间，结束小时
	u_int8_t timer1_min_end;//定时时间，结束分钟
	
	u_int8_t timer2_valid;//是否有效
	u_int8_t timer2_onoff;//1:on, 0:off
	u_int8_t timer2_hour;//定时时间，小时
	u_int8_t timer2_min;//定时时间，分钟表示
	u_int8_t timer2_hour_end;//定时时间，结束小时
	u_int8_t timer2_min_end;//定时时间，结束分钟
	
	u_int8_t timer3_valid;//是否有效
	u_int8_t timer3_onoff;//1:on, 0:off
	u_int8_t timer3_hour;//定时时间，小时
	u_int8_t timer3_min;//定时时间，分钟表示
	u_int8_t timer3_hour_end;//定时时间，结束小时
	u_int8_t timer3_min_end;//定时时间，结束分钟
}cl_zkrsq_tiemr_t;

typedef struct {
	u_int8_t onoff;//开关，1：表示ON  , 0：表示OFF.
	u_int8_t back_water_onoff;//开关，1：表示ON  , 0：表示OFF.
	int8_t hot_water_tmp;//设置温度 15℃—60℃ 
	u_int8_t mode;//工作模式 0：空调制冷；1：空调制热；2：热水；3：空调制冷+热水；4：空调制热+热水；5：自动
	//设置参数
	cl_zk_confset_t conf;

	//定时
	cl_zkrsq_tiemr_t timer;

	//故障
	u_int8_t fault;//如上

	//状态,设备复位
	u_int8_t reset_status;// 1:设备被复位
	int8_t hot_water_tmp_cur;//当前热水温度 显示范围(-9℃—99℃)
	int8_t coilpipe_tmp;//盘管温度 显示范围(-9℃—99℃)
	int8_t room_tmp;//环境温度 显示范围(-9℃—99℃)
	int8_t back_water_tmp_cur;//当前回水温度 显示范围(-9℃—99℃)
	
}cl_zkrsq_info_t;

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
CLIB_API RS cl_zkrsq_ctrl(cl_handle_t dev_handle, ZKRSQ_ACTION_T action, u_int8_t value);

/*
	功能:
		详情参数设置
	输入参数:
		@dev_handle: 设备的句柄
		@pconfig:详情设置参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zkrsq_config(cl_handle_t dev_handle, cl_zk_confset_t *pconfig);

/*
	功能:
		定时器设置
	输入参数:
		@dev_handle: 设备的句柄
		@ptimer:定时配置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zkrsq_timer(cl_handle_t dev_handle, cl_zkrsq_tiemr_t *ptimer);



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZKRSQ_H */

