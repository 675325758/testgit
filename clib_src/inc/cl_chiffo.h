#ifndef	__CL_CHIFFO_H__
#define	__CL_CHIFFO_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define CHIFFO_TIMER_DAYS_CNT 7
#define CHIFFO_TIMER_SECTION_PER_DAY 8

typedef enum {
	SCENE_AUTO = 1,	// 自动模式
	SCENE_BATH,// 浴缸模式
	SCENE_DISHES,// 洗碗模式
	SCENE_VEGETABLES,// 洗菜模式
	SCENE_CLOTHES,	// 洗衣模式
} CHIFFO_SCENE_T;

typedef struct{
	u_int8_t is_enable;
	u_int8_t temp;
	u_int8_t pad[2];
}cl_chiffo_timer_item_t;

typedef struct{
	cl_chiffo_timer_item_t items[CHIFFO_TIMER_SECTION_PER_DAY];
}cl_chiffo_one_day_timer_t;

typedef struct{
	u_int8_t is_on; //开关
    u_int8_t fault_code;//故障编码
	u_int8_t is_fan_working; //风机是否工作
	u_int8_t is_fire_working;//燃烧是否工作
	u_int8_t is_floor_heat_working;//地热工作
	u_int8_t is_radiator_working;//散热器是否工作
	u_int8_t is_pump_working;//水泵是否工作
	u_int8_t is_water_working;//水流是否工作
	u_int8_t next_heat_time; //下个工作时段
	u_int8_t is_water_mode_on; //热水模式是否开启
	u_int8_t is_heater_mode_on; //供暖模式是否开启
	//热水模式数据
	u_int8_t water_setting_temp; //热水设置温度
	u_int8_t water_current_temp; //热水当前温度
	u_int32_t cur_water_pressure; //当前水压
	//供暖模式数据
	u_int8_t heater_setting_temp; //供暖设置温度
	u_int8_t heater_current_temp; //供暖当前温度
	cl_chiffo_one_day_timer_t timer_info[CHIFFO_TIMER_DAYS_CNT];

	// 情景 自动浴缸洗衣洗菜。。
	CHIFFO_SCENE_T scene;
	// 循环
	u_int8_t loop_type;	// 0 循环关闭1 单循环 2 智能循环

	// 无线连接信号强度，分为1-4级
	u_int8_t rssi;

	// 设备时间
	u_int8_t hour;
	u_int8_t min;

	// 浴缸模式对应的水量，单位L
	u_int16_t water_capacity;
	u_int8_t need_show_water_capacity;	// 是否显示水量设置
}cl_chiffo_floor_heater_info_t;


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
CLIB_API RS cl_chiffo_ctrl_on_off(cl_handle_t dev_handle,u_int8_t is_on);

/*
	功能:
		控制热水模式温度
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_setting_water_mode_temp(cl_handle_t dev_handle,u_int8_t temp);

/*
	功能:
 单步增加或减少热水模式温度
	输入参数:
 @dev_handle: 设备的句柄
	输出参数:
 无
	返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_chiffo_add_dec_water_mode_temp(cl_handle_t dev_handle,bool is_add);

/*
	功能:
 单步增加或减少制热模式温度
	输入参数:
 @dev_handle: 设备的句柄
	输出参数:
 无
	返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_chiffo_add_dec_heater_mode_temp(cl_handle_t dev_handle,bool is_add);

/*
	功能:
		控制热水模式温度
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_setting_heater_mode_temp(cl_handle_t dev_handle,u_int8_t temp);


/*
	功能:
		控制模式
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_setting_mode(cl_handle_t dev_handle,u_int8_t water_on_off,u_int8_t heater_on_off);


/*
	功能:
		控制定时器
	输入参数:
		@dev_handle: 设备的句柄
		@day_index: 1-7 星期1-7
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_setting_timer(cl_handle_t dev_handle,cl_chiffo_one_day_timer_t* day_info,u_int8_t day_index);

/*
	功能:
		刷新某天的定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		day_index 1-7 ，星期1-7
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_refresh_timer_by_day(cl_handle_t dev_handle,u_int8_t day_index);

/*
	功能:
		设置场景
		
	输入参数:
		@dev_handle: 设备的句柄
		@scene: 参考CHIFFO_SCENE_T 定义
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_set_scene(cl_handle_t dev_handle, u_int8_t scene);


/*
	功能:
		设置循环方式
		
	输入参数:
		@dev_handle: 设备的句柄
		@loop_type: 方式0 循环关闭1 单循环 2 智能循环
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

CLIB_API RS cl_chiffo_set_loop(cl_handle_t dev_handle,u_int8_t loop_type);

/*
	功能:
		设置前锋系统的时间
		
	输入参数:
		@dev_handle: 设备的句柄
		
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_set_clock(cl_handle_t dev_handle);

/*
	功能:
		加减前锋水量
		
	输入参数:
		@dev_handle: 设备的句柄
		@is_add: 1为加 0 为减
		
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_chiffo_add_dec_water_capacity(cl_handle_t dev_handle,bool is_add);

/*
	功能:
		设置前锋出水量
		
	输入参数:
		@dev_handle: 设备的句柄
		@capacity: 水量，单位10L
		
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

CLIB_API RS cl_chiffo_setting_water_capacity_temp(cl_handle_t dev_handle,u_int16_t capacity);


#ifdef __cplusplus
}
#endif


#endif

