#ifndef	__CL_YL_THERMOSTAT_H__
#define	__CL_YL_THERMOSTAT_H__

#ifdef __cplusplus
extern "C" {
#endif 

//温度限制
#define YL_SET_TMP_MAX 	(30)
#define YL_SET_TMP_MIN 	(10)
#define YL_SET_TMP_DEF 	(23)

/* Type definitions. */
//开关状态
enum {
	YL_TS_OFF = 0X0,
	YL_TS_ON = 0X1,
};

//模式
enum {
	YL_TS_MODE_FAN = 0x0,
	YL_TS_MODE_COOL = 0x1,
	YL_TS_MODE_HEAT = 0x2,
	YL_TS_MODE_MAX,
};

//档位
enum{
	YL_TS_GEAR_AUTO = 0x0,
	YL_TS_GEAR_LOW = 0x1,
	YL_TS_GEAR_MIDDLE = 0x2,
	YL_TS_GEAR_HIGH = 0x3
};

//功能模式
enum {
	//手动模式
	YL_TS_FUNC_MODE_HAND = 0X0,
	//编程模式
	YL_TS_FUNC_MODE_PROG = 0x1,
	//舒适模式
	YL_TS_FUNC_MODE_COMF = 0X2,
	//节能模式
	YL_TS_FUNC_MODE_SAVE = 0X3,
	//离家模式
	YL_TS_FUNC_MODE_LEAVE = 0X4,
	//智能模式
	YL_TS_FUNC_MODE_SMART = 0X5,
	YL_TS_FUNC_MODE_MAX,
};

typedef struct ts_scene_s{
	u_int8_t set_tmp;
	u_int8_t gear;
}ts_scene_t;

typedef struct {
	u_int8_t onoff; //开关
	u_int8_t work_mode; //工作模式
	u_int8_t gear;  //档位
	u_int8_t temp; //温度
	u_int8_t room_temp; //室温10-30度，默认23度
	u_int8_t cur_scene; //当前功能模式
	ts_scene_t scene[YL_TS_FUNC_MODE_MAX*YL_TS_MODE_MAX]; 
} cl_yl_thermostat_info;


/*
	功能:
		控制开关
	输入参数:
		@dev_handle: 设备的句柄
		@is_on: 0-关机，1－开机
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yl_thermostat_ctrl_onoff(cl_handle_t dev_handle, bool is_on);


/*
	功能:
		控制工作模式
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yl_thermostat_ctrl_work_mode(cl_handle_t dev_handle, u_int8_t work_mode);

/*
	功能:
		控制档位，仅通风下有效，有work_mode，防止扩展
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yl_thermostat_ctrl_gear(cl_handle_t dev_handle,u_int8_t work_mode, u_int8_t gear);

/*
	功能:
		控制温度
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yl_thermostat_ctrl_temp(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t temp);

/*
	功能:
		控制切换情景
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yl_thermostat_ctrl_scene(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t scene);

/*
	功能:
		设置各模式下的温度参数
	输入参数:
		@dev_handle: 设备的句柄
		@temp_param: 一维数值大小: YL_TS_MODE_MAX*YL_TS_FUNC_MODE_MAX
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yl_thermostat_set_temp_param(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t scene_count,ts_scene_t* temp_param);
    
/////////////////////////////////////////////////////////////////
// 科希曼温控器

#define MAX_KXM_SUB_PUMP_NUM 8
#define MAX_KXM_TIMER_CNT 3
#define MAX_KXM_SUB_SYSTEM 4

#define KOCHEM_THERMOSTAT_COOLMODE_TEMPERATURE_MIN 5
#define KOCHEM_THERMOSTAT_COOLMODE_TEMPERATURE_MAX 35
#define KOCHEM_THERMOSTAT_HEATMODE_TEMPERATURE_MIN 5
#define KOCHEM_THERMOSTAT_HEATEMODE_TEMPERATURE_MAX 35
#define KOCHEM_THERMOSTAT_ENERGY_COOL_TEMPERATURE 28
#define KOCHEM_THERMOSTAT_ENERGY_HOT_TEMPERATURE 16
    
#define KOCHEM_DEVICE_COOLMODE_TEMPERATURE_MIN 10
#define KOCHEM_DEVICE_COOLMODE_TEMPERATURE_MAX 30
#define KOCHEM_DEVICE_HEATMODE_TEMPERATURE_MIN 20
#define KOCHEM_DEVICE_HEATMODE_TEMPERATURE_MAX 55
    
typedef struct {
    u_int8_t scoll_temp; //盘管温度
    u_int8_t inhale_temp; //吸气温度
    u_int8_t exhaust_temp; //排气温度
    u_int8_t exv_value; //EXV开度
    u_int8_t compress_stat; //压缩机状态
    u_int8_t spray_stat; //喷淋阀状态
    u_int8_t is_low_press_fault; //低压故障
    u_int8_t is_high_press_fault; //高压故障
    u_int8_t is_over_curr_fault; //过流故障
    u_int8_t is_exhault_fault; //排气故障
    u_int8_t pad[2];
}cl_kxm_sub_system_stat_t;
    
//机组状态
typedef struct {
    u_int8_t is_online; //是否存在
    u_int8_t machine_type; //机器型号 0：热水 1： 空调
    u_int8_t back_water_temp; //回水温度
    u_int8_t water_temp; //供水温度
    u_int8_t env_temp; // 环境温度
    u_int8_t water_box_temp; // 水箱温度
    u_int8_t in_water_temp; // 进水温度
    u_int8_t out_water_temp; // 出水温度
    u_int8_t run_stat;// 运行状态 0保温;1制热;2制冷;3除霜;4防冻
    u_int8_t water_pos;//水位状态 百分比
    u_int8_t is_fan_high; // 风机高状态 0：关闭  1：开启
    u_int8_t is_fan_low; // 风机低状态 0：关闭  1：开启
    u_int8_t cir_pump_run; // 循环泵状态 0：关闭  1：开启
    u_int8_t back_pump_run; // 回水泵状态 0：关闭  1：开启
    u_int8_t in_water_pump_run; // 进水泵状态 0：关闭  1：开启
    u_int8_t water_pump_run; // 供水泵状态 0：关闭  1：开启
    u_int8_t is_elec_hot_run; // 电加热状态 0：关闭  1：开启
    u_int8_t sw_dir_tap_run; // 换向阀状态 0：关闭  1：开启
//    -------------------------------------------------
    u_int8_t sensor_back_water_fault; //回水传感器故障
    u_int8_t sensor_water_fault; //供水传感器故障
    u_int8_t sensor_env_fault; //环境温度传感器故障
    u_int8_t sensor_water_box_fault; //水箱传感器故障
    u_int8_t sensor_in_water_fault; //进水传感器故障
    u_int8_t sensor_out_water_fault; //出水传感器故障
    u_int8_t is_out_water_temp_low_fault; //出水温度低故障
    u_int8_t is_out_water_temp_high_fault; //出水温度高故障
    u_int8_t is_in_out_temp_big_fault; //进出水温差大故障
    u_int8_t is_anti_phase_fault; //逆相故障
    u_int8_t is_no_phase_L2_fault; //缺相L2故障
    u_int8_t is_no_phase_L3_fault; // 缺相L3故障
    u_int8_t is_ctrl_comu_fault; //控制器通信故障
    u_int8_t pad;
    cl_kxm_sub_system_stat_t sub_system_info[MAX_KXM_SUB_SYSTEM];
}cl_kxm_pump_stat_info_t;

//定时器
typedef struct {
    u_int8_t on_hour;
    u_int8_t on_min;
    u_int8_t off_hour;
    u_int8_t off_min;
}cl_kxm_timer_info_t;

//模式
enum{
    KXM_MODE_HOT_WATER,
    KXM_MODE_HOT,
    KXM_MODE_COLD
};
    
//线控器(空调机/热水器)控制信息和状态
typedef struct {
    u_int8_t on_off;
    u_int8_t mode; // 0:热水 1：制热 2： 制冷
    u_int8_t hot_water_setting_temp; //热水设置温度 20-60
    u_int8_t hot_setting_temp; //制热设定温度 20-60
    u_int8_t cold_setting_temp; // 制冷设定温度 10-30
    u_int8_t t_hour; //设备上的时钟：小时
    u_int8_t t_min; //设备上的时钟: 分钟
    u_int8_t pad;
    cl_kxm_timer_info_t timer[MAX_KXM_TIMER_CNT];
}cl_kxm_host_info_t;
    
typedef struct {
    u_int8_t is_data_valid;
    u_int8_t has_receive_data;
    u_int8_t pad[2];
    cl_kxm_pump_stat_info_t pump[MAX_KXM_SUB_PUMP_NUM];
    cl_kxm_host_info_t hinfo;
    
    u_int8_t sdk_priv_data[128]; //SDK使用，APP不要填充数据
}cl_kxm_info_t;

/*
 功能：
	 控制线控器开关
 输入参数:
	 @handle: 设备句柄
 输出参数:
	 无
 返回：
	 RS_OK:
	 其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_kxm_ctrl_host_onoff(cl_handle_t dev_handle,u_int8_t on_off);
    
/*
 功能：
	 控制线控器定时器
 输入参数:
	 @handle: 设备句柄
 	@timer_index: 1-3
 输出参数:
	 无
 返回：
	 RS_OK:
	 其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_kxm_ctrl_timer(cl_handle_t dev_handle,u_int8_t timer_index,cl_kxm_timer_info_t* timer);


/*
 功能：
	 控制线控器模式和温度
 输入参数:
	 @handle: 设备句柄
 输出参数:
	 无
 返回：
	 RS_OK:
	 其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_kxm_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode,u_int8_t temp);
    
/*
 功能：
	 控制线控器模式和温度
 输入参数:
	 @handle: 设备句柄
 输出参数:
	 无
 返回：
	 RS_OK:
	 其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_kxm_set_dev_time(cl_handle_t dev_handle,u_int8_t min,u_int8_t sec);
    

/*
 功能：
 	控制线控器定时器
 输入参数:
	 @handle: 设备句柄
 输出参数:
	 无
 返回：
	 RS_OK:
 	其他: 失败
	 事件通知:
 
 */
CLIB_API RS cl_kxm_ctrl_all_timer(cl_handle_t dev_handle,cl_kxm_timer_info_t timer[MAX_KXM_TIMER_CNT]);

/////////////////////////////////////////////////////////////////////
//科希曼温控器
    
enum{
    KXM_WM_UNKNOWN = 0x00,
    KXM_WM_COLD, //制冷模式
    KXM_WM_FAN_HOT, //风盘制热
    KXM_WM_WATER_HOT, // 水暖制热
    KXM_WM_FAN_WATER_HOT //风盘和水暖制热
};
    
enum{
    KXM_FS_UNKNOWN,
    KXM_FS_LOW, //低风
    KXM_FS_MIDDLE, //中风
    KXM_FS_HIGH, //高风
    KXM_FS_AUTO //自动风
};
    
typedef struct {
    u_int8_t onoff; //开关状态
    u_int8_t mode; //工作模式
    u_int8_t setting_temp; //温度
    int8_t room_temp; //室温
    u_int8_t fan_speed; //风速
    u_int8_t energy_cons;//节能
}cl_kxm_thermost_info_t;

enum{
    KXM_CA_NONE = 0,
    KXM_CA_ONOFF, //控制开关
    KXM_CA_MODE, //控制模式
    KXM_CA_TEMP, //控制温度
    KXM_CA_FS, //控制风速
    KXM_CA_EC //控制节能
};
    
/*
 功能：
 	控制科希曼温控器
 输入参数:
 	@handle: 设备句柄
 输出参数:
 	无
 返回：
 	RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_kxm_ther_ctrl(cl_handle_t dev_handle,u_int8_t action,u_int8_t value);

/////////////////////////////////////////////////////////////////////////////
//思博特温控器

//状态控制
typedef struct {
    u_int8_t onoff; //开关机
    u_int8_t temp; //温度 10-32
    u_int8_t mode; //工作模式 00制冷模式，01制热模式，2通风模式， 3 节能模式
    u_int8_t fan_speed; //风速 0—自动；1—低档，2---中档， 3—高档；
}cl_sbt_work_ctrl_t;

//高级参数配置
typedef struct {
    u_int8_t auto_mode; //自动模式状态 0关闭，1执行自动模式
    int8_t temp_adjust; //温度补偿 -5至5
    u_int8_t low_temp; //低温保护温度 3-9
    u_int8_t valve_mode; //(风机开关)阀门模式控制 1、阀关风机不停0、阀关风机停
    u_int8_t return_temp; //回差温度 1~31
    u_int8_t is_low_temp_guard; //低温保护使能
    u_int8_t max_temp; //限制最高温度 10-32
    u_int8_t min_temp; //限制最低温度 10-32
}cl_sbt_func_setting_t;

//时间校正
typedef struct {
    u_int8_t scm_hour; //单片机时间,时
    u_int8_t scm_min; //单片机时间,分
    u_int8_t scm_sec; //单片机时间,秒
    u_int8_t scm_weekday; //单片机时间,星期
}cl_sbt_time_adjust_t;

#pragma pack(push,1)
    
// 智能模式配置相关
typedef struct {
    u_int8_t hour;
    u_int8_t temp;
}cl_sbt_smart_item_t;
    
typedef struct {
    cl_sbt_smart_item_t si[4];
}cl_sbt_smart_day_t;
    
typedef struct {
    cl_sbt_smart_day_t work_day;
    cl_sbt_smart_day_t sat_day;
    cl_sbt_smart_day_t sun_day;
}cl_smart_smart_ctrl_t;

#pragma pack(pop)
    
typedef struct {
    u_int8_t onoff; //开关机
    u_int8_t temp; //温度 10-32
    u_int8_t mode; //工作模式 00制冷模式，01制热模式，2通风模式， 3 节能模式
    u_int8_t fan_speed; //风速 0—自动；1—低档，2---中档， 3—高档；
    //----------------------//
    u_int8_t auto_mode; //自动模式状态 0关闭，1执行自动模式
    int8_t temp_adjust; //温度补偿 -5至5
    u_int8_t low_temp; //低温保护温度 3-9
    u_int8_t valve_mode; //(风机开关)阀门模式控制 1、阀关风机不停0、阀关风机停
    u_int8_t return_temp; //回差温度 1~31
    u_int8_t is_low_temp_guard; //低温保护使能
    u_int8_t max_temp; //限制最高温度 10-32
    u_int8_t min_temp; //限制最低温度 10-32
    /*以下这段数据APP可以暂时不关心,jni可以不上传*/
    u_int8_t lock_screen; //是否锁屏，
    u_int8_t sesor_type; //温度传感器类型
    u_int8_t broad_cast_type; //广播类型
    u_int8_t no_paid_mode; //欠费模式
    u_int8_t max_sesor_temp; //外部传感器限制温度
    u_int8_t pad;
    //不关心部分完//
    int16_t room_temp; //室温*10，使用的时候需要除以10
    u_int8_t scm_hour; //单片机时间,时
    u_int8_t scm_min; //单片机时间,分
    u_int8_t scm_sec; //单片机时间,秒
    u_int8_t scm_weekday; //单片机时间,星期
    cl_smart_smart_ctrl_t smart_info; //智能模式当前信息
    u_int8_t is_data_valid;//数据是否有效
    u_int8_t pad2[3];
}cl_sbt_ther_info_t;
    
/*
 功能：
	 思博特温控器状态控制
 输入参数:
	 @handle: 设备句柄
 输出参数:
 	无
 返回：
	 RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_sbt_ther_ctrl_stat(cl_handle_t dev_handle,cl_sbt_work_ctrl_t* wc);
    

/*
 功能：
 	思博特温控器参数控制
 输入参数:
 	@handle: 设备句柄
 输出参数:
 无
 返回：
 RS_OK:
 其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_sbt_ther_set_param(cl_handle_t dev_handle,cl_sbt_func_setting_t* fs);
    
/*
 功能：
 	思博特温控器校正时间
 输入参数:
 	@handle: 设备句柄
 输出参数:
	 无
 返回：
 	RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_sbt_ther_set_ajust_scm_time(cl_handle_t dev_handle,cl_sbt_time_adjust_t* st);

/*
 功能：
 	思博特温控器设置智能模式的配置
 输入参数:
 	@handle: 设备句柄
 输出参数:
 	无
 返回：
 	RS_OK:
    其他: 失败
 	事件通知:
 
 */
CLIB_API RS cl_sbt_ther_set_smart_config(cl_handle_t dev_handle,cl_smart_smart_ctrl_t* ss);
    

/*
 功能：
 	思博特温控器启用和停用智能模式
 输入参数:
 	@handle: 设备句柄
 输出参数:
 	无
 返回：
 	RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_sbt_ther_set_smart_config_mode(cl_handle_t dev_handle,u_int8_t onoff);


    
#ifdef __cplusplus
}
#endif 


#endif

