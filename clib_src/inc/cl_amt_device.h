#ifndef	__CL_AMT_DEVICE_H__
#define	__CL_AMT_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_USER_DEFINE_MODE_POINT 7

enum{
	AMT_FAN_STAND = 0x0, //标准风
	AMT_FAN_NATURAL = 0x1,//自然风
	AMT_FAN_SLEEP = 0x2,//睡眠风
	AMT_FAN_SMART = 0x3,//智能风
	AMT_FAN_UD_NATURAL = 0x9//自定义自然风
};

#define AMT_MAX_U_GEAR 32
#define AMT_MIN_GEAR 1

typedef struct{
	u_int8_t onoff; //开关
	u_int8_t cur_mode; //当前模式
	u_int8_t cur_gear; //当前档位
	u_int8_t screen_light; //屏显
	u_int8_t is_shake; //是否处在摇头阶段
	int8_t cur_temp;//当前温度
	u_int8_t is_anion_on;//负离子是否开启
	u_int8_t is_plasma_on;
	u_int8_t cur_user_define_mode[MAX_USER_DEFINE_MODE_POINT]; //用户自定义模式的档位
	u_int8_t is_timer_on_valid; //预约开机是否有效
	u_int8_t is_timer_off_valid; //定时关机是否有效
	u_int8_t cur_power; //当前功率
	u_int32_t dev_on_remain_time;//预约开机剩余时间
	u_int32_t dev_off_remain_time; // 定时关机剩余时间
}cl_amt_fan_device_t;

#ifdef __cplusplus
}
#endif 

/*
	功能:
		控制开关机
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_on_off(cl_handle_t dev_handle,u_int8_t is_on);

/*
	功能:
		切换模式
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode);

/*
	功能:
		切换档位
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_gear(cl_handle_t dev_handle,u_int8_t mode,u_int8_t gear);

/*
	功能:
		控制摇头
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_shake(cl_handle_t dev_handle,u_int8_t is_shake);

/*
	功能:
		控制负离子
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_anion(cl_handle_t dev_handle,u_int8_t is_on);

/*
	功能:
		控制等离子
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_plasma(cl_handle_t dev_handle,u_int8_t is_on);

/*
	功能:
		控制屏显
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_screen_light(cl_handle_t dev_handle,u_int8_t is_on);

/*
	功能:
		设置用户自定义模式参数
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_config_fan_user_mode(cl_handle_t dev_handle,u_int8_t time_interval,u_int8_t gear_num,u_int8_t* gears);

/*
	功能:
		定时关机和预约开机
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_ctrl_dev_onoff_timer(cl_handle_t dev_handle,u_int8_t onoff,u_int16_t time);

/*
	功能:
		设置智能模式参数
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_amt_config_smart_param(cl_handle_t dev_handle,u_int8_t temp,u_int8_t gear);


#endif

