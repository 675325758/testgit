#ifndef	__CL_CH_BLANKET_H__
#define	__CL_CH_BLANKET_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define AREA_BLANKET_LEFT 0x0
#define AREA_BLANKET_RIGHT 0x1

//温区曲线点数
#define BLANKET_CURVE_PER_HOUR_POINT  24
#define BLANKET_CURVE_HALF_HOUR_POINT  48

//工作模式
enum{
    BLANKET_MODE_KNOWN  = 0x0,
    BLANKET_MODE_KILL_MITES, //除螨
    BLANKET_MODE_DEHUMIDIFY, //除湿，排潮
    BLANKET_MODE_HOT, //加热
    BLANKET_MODE_SLEEP //睡眠模式
};

    
/*
	功能:
		查询温区信息
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_query_info(cl_handle_t dev_handle, u_int8_t area);

/*
	功能:
		开关温区
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@on_off   0x0: 关闭 1:开启
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_set_on_off(cl_handle_t dev_handle, u_int8_t area, u_int8_t on_off);

/*
	功能:
		设置温区温度
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@temp 18-48度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_set_temp(cl_handle_t dev_handle, u_int8_t area, u_int8_t temp);

/*
	功能:
		设置温区曲线是否生效
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@is_enable  0x0:关闭 0x1:开启
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_set_curve_enable(cl_handle_t dev_handle, u_int8_t area, u_int8_t is_enable);

/*
	功能:
		设置温区手动定时
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@time  0x0:关闭 0x1--0x9 定时时间
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_set_timer(cl_handle_t dev_handle, u_int8_t area, u_int8_t time);

/*
	功能:
		设置温区曲线澹(每小时一个点)
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@curve: 从0点至23点,关闭置0,开启时设置温度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_set_hour_curve(cl_handle_t dev_handle, u_int8_t area, u_int8_t curve[BLANKET_CURVE_PER_HOUR_POINT]);

/*
	功能:
		设置温区曲线(每半小时一个点)
	输入参数:
		@dev_handle: 电热毯的句柄
		@area  温区选择
		@curve: 从0点至23:30悻关闭置0,开启时设置温度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_blanket_set_half_hour_curve(cl_handle_t dev_handle, u_int8_t area, u_int8_t curve[BLANKET_CURVE_HALF_HOUR_POINT]);

    
/*
 功能:
    设置温区模式
 输入参数:
    @dev_handle: 电热毯的句柄
    @area  温区选择
    @on_off   BLANKET_MODE_XX
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_blanket_set_work_mode(cl_handle_t dev_handle, u_int8_t area, u_int8_t work_mode);

#ifdef __cplusplus
}
#endif 


#endif

