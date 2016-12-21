#ifndef	__CL_TL_TEMP_H__
#define	__CL_TL_TEMP_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum{
	MODE_TL_FAN = 0x0,
	MODE_TL_AUTO,
	MODE_TL_COOL,
	MODE_TL_HOT
};

enum{
	FAN_TL_AUTO = 0x0,
	FAN_TL_LOW,
	FAN_TL_MID,
	FAN_TL_HIGH
};

typedef struct{
	/* 开关：关、开 */
	u_int8_t onoff;
	/* 运行模式：通风、自动、制冷、制热 */
	u_int8_t mode;
	/* 风速设定：低、中、高 */
	u_int8_t speed;
	/* 温度设定：5 - 35 */
	u_int8_t temp;
	/* 测量温度 */
	u_int16_t room_temp;
	/*状态标志、APP 界面可不关注*/
	u_int16_t state;
	/*阀门状态*/
	u_int8_t valve_stat;
	//制冷阀状态
	u_int8_t cool_valve_stat;
	//制热阀门状态
	u_int8_t hot_valve_stat;
	/*计费状态*/
	u_int8_t charge_enable;
}cl_tl_ctrl_stat_t;

typedef struct  {
	/* 位操作 */
	u_int16_t lock_flags;
	/* 温度带宽设定 */
	u_int16_t temp_bandwidth;
	/* 低档计费因子 */
	u_int16_t charge_factor_low;
	/* 中档计费因子 */
	u_int16_t charge_factor_mid;
	/* 风机是否受控 */
	u_int8_t fan_under_control;
	/* 计时允许 */
	u_int8_t time_on;
	/* ECO节能 */
	u_int8_t eco_mode;
	/* 自学习 */
	u_int8_t self_learn;
    /*当前运行在哪个时段*/
    u_int8_t run_timer;
}cl_tl_adv_info_t;

typedef struct{
	/* 累计折算高档时间 */
	u_int32_t high_gear_time_cal;
	/* 累计低档时间 */
	u_int32_t low_gear_time;
	/* 累计中档时间 */
	u_int32_t mid_gear_time;
	/* 累计高档时间 */
	u_int32_t high_gear_time;
}cl_tl_total_time_t;

typedef struct{
	u_int16_t temp;
	u_int16_t hour;
	u_int16_t min;
}cl_tl_timer_item_t;
    
typedef struct {
    u_int8_t is_data_valid; //数据是否有效
    u_int8_t is_auto_sync; // 是否自动同步
    u_int8_t pad[2];
    u_int32_t dev_time; //设备时间 UTC
    u_int32_t local_update_time; //本地更新时间,UTC
}cl_tl_time_sync_t;

#define TL_TIME_CNT_PER_DAY 4

typedef struct{
	cl_tl_timer_item_t time[TL_TIME_CNT_PER_DAY];
}cl_tl_timer_info_t;

typedef struct{
	cl_tl_ctrl_stat_t ctrl_stat;
	cl_tl_adv_info_t adv_info;
	cl_tl_total_time_t total_info;
	cl_tl_timer_info_t time_info;
    cl_tl_time_sync_t dev_time_sync_info;
}cl_tl_info_t;
    
enum{
    TLH_M_HAND = 0, //特林采暖器手工控制模式
    TLH_M_PROGRAM//特林采暖器编程控制模式
};

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
CLIB_API RS cl_tl_ctrl_off(cl_handle_t dev_handle,bool onoff);

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
CLIB_API RS cl_tl_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode);


/*
	功能:
		设置风速
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tl_ctrl_fan_speed(cl_handle_t dev_handle,u_int8_t fan_speed);

/*
	功能:
		设置温度
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tl_ctrl_temp(cl_handle_t dev_handle,u_int8_t temp);


/*
	功能:
		设置ECO 模式
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tl_ctrl_eco(cl_handle_t dev_handle,bool is_enable);

/*
	功能:
		设置learn 开启
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tl_ctrl_learn(cl_handle_t dev_handle,bool is_enable);


/*
	功能:
		设置时段参数
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tl_setting_timer(cl_handle_t dev_handle,cl_tl_timer_info_t* timer);

/*
 功能:
    设置自动同步时钟
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_tl_ctrl_clock_auto_sync(cl_handle_t dev_handle,bool is_auto_sync);

/*
 功能:
    手动同步时钟
 输入参数:
    @dev_handle: 设备句柄
 输出参数:
    无
返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_tl_ctrl_clock_sync(cl_handle_t dev_handle);
    
/*
 功能:
    刷新设备当前时间
 输入参数:
    @dev_handle: 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_tl_refresh_dev_time(cl_handle_t dev_handle);


#ifdef __cplusplus
}
#endif 


#endif

