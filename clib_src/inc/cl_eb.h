#ifndef	__CL_EB_H__
#define	__CL_EB_H__

#ifdef __cplusplus
extern "C" {
#endif 


/* E宝控制结果通知给APP的事件 */
enum {
	EBE_BEGIN = 1800,
	EBE_SET_WORK_OK = EBE_BEGIN + 1,
	EBE_SET_WORK_FAULT = EBE_BEGIN + 2,
	EBE_SET_TIMER_OK = EBE_BEGIN + 3,
	EBE_SET_TIMER_FAULT = EBE_BEGIN + 4,
	EBE_END = EBE_BEGIN + 99
};


/*
	功能:
		控制开关
	输入参数:
		@dev_handle: 插座的句柄
		@is_on: 0-关机，1－开机
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_ctrl_work(cl_handle_t dev_handle, bool is_on);

/*
	功能:
		添加或修改定时开关规则
	输入参数:
		@dev_handle: 设备句柄
		@timer: 定时器条目的参数。id为0表示添加，其他的为修改
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_timer_set(cl_handle_t dev_handle, cl_air_timer_t *timer);

/*
	功能:
		删除定时开关规则
	输入参数:
		@dev_handle: 插座的句柄
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	功能:
		添加或修改定时开关规则
	输入参数:
		@dev_handle: 设备句柄
		@timer: 定时器条目的参数。id为0表示添加，其他的为修改
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer);

/*
	功能:
		删除定时开关规则
	输入参数:
		@dev_handle: 插座的句柄
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_period_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	功能:
		设置校正参数
	输入参数:
		@dev_handle: 设备句柄
		@adj: 电流电压校正参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_pt_adj_set(cl_handle_t dev_handle, cl_plug_pt_adkb_t *adj, u_int8_t action);
    

/*
 功能:
    控制杰能宝LED灯
 输入参数:
    @onoff: 0:关闭，1：开启，2：智能
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_eb_ctrl_led_power(cl_handle_t dev_handle, u_int8_t onoff);


#ifdef __cplusplus
}
#endif 


#endif

