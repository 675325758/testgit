#ifndef	__CL_JNB_DEVICE_H__
#define	__CL_JNB_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum {
	JT_MODE_OFF = 0x0,
	JT_MODE_DEFORST, //除霜
	JT_MODE_HOLD_COMFORTABLE, //HOLD 舒适模式
	JT_MODE_HOLD_ECONOMY,//HOLD 经济模式
	JT_MODE_AUTO, //自动模式
	JT_MODE_HOLIDAY // 假期模式
};

typedef struct {
	u_int8_t work_status;		// 指定的模式，见上。
	u_int8_t env_temperature;	// 环境温度
	u_int8_t temp_temperature;	// 临时设置温度
	u_int8_t vacation_remain_days; 
	u_int8_t vacation_temperature;// 假期配置的温度
	u_int8_t vacation_days;	// 假期设定的天数
	u_int8_t comfort_temperaute;	// 舒适模式配置的温度
	u_int8_t comfort_hold_time;	// 临时HOLD时间，单位小时
	u_int8_t economic_temperature;// 经济模式配置的温度
	u_int8_t economic_hold_time;	// 临时HOLD时间，单位小时
	u_int32_t scheduler[7];	// scheduler[0]表示星期天，往后依次为星期1到星期6，
				// 其中1～24位分别表示每个时间段的模式选择。25位表示是否配置:0-未配置，1-有配置
} cl_jnb_thermotat_info;


/*
	功能:
		设置开关
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_ctrl_onoff(cl_handle_t dev_handle,u_int8_t onoff);

/*
	功能:
		设置模式
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode);

/*
	功能:
		临时设置温度
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_ctrl_temp(cl_handle_t dev_handle,u_int8_t temp);

/*
	功能:
		设置周期性模式
	输入参数:
		@dev_handle: 插座的句柄
		@schedluer, 7个u32的数值，代表每天的24小时，// scheduler[0]表示星期天，往后依次为星期1到星期6，
				// 其中1～24位分别表示每个时间段的模式选择。25位表示是否配置:0-未配置，1-有配置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_set_schedluer(cl_handle_t dev_handle,u_int32_t* schedluer);

/*
	功能:
		配置各模式下的默认温度参数
		// 主要是自动模式和假期模式
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_set_temp_param(cl_handle_t dev_handle,u_int8_t mode,u_int8_t temp);

/*
	功能:
		配置hold 的时间，0-24小时
	输入参数:
		@dev_handle: 插座的句柄
		@mode JT_MODE_HOLD_ECONOMY或JT_MODE_HOLD_COMFORTABLE，
			反向配置，当前为舒适，配置经济
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_set_holiday_days(cl_handle_t dev_handle,u_int8_t temp,u_int8_t days);

/*
	功能:
		配置假期天数
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_jnb_th_set_hold_time(cl_handle_t dev_handle,u_int8_t mode,u_int8_t hours);

#ifdef __cplusplus
}
#endif 


#endif

