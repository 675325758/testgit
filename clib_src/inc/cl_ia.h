#ifndef	__CL_IA_H__
#define	__CL_IA_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_smart_appliance.h"

//event
enum {
	IE_BEGIN = 1000,
	/* 设置某个属性成功 */
	IE_CTRL_OK,
	/* 设置某个属性失败 */
	IE_CTRL_FAULT,

	/* 空气净化器属性设置事件 */
	IE_AIRCLEANER_SET_ONOFF_OK,
	IE_AIRCLEANER_SET_SPEED_OK,
	IE_AIRCLEANER_SET_TIMER_OK,
	IE_AIRCLEANER_SET_ULTRA_OK,
	IE_AIRCLEANER_SET_ANION_OK,
	IE_AIRCLEANER_SET_ONOFF_FAULT,
	IE_AIRCLEANER_SET_SPEED_FAULT,
	IE_AIRCLEANER_SET_TIMER_FAULT,
	IE_AIRCLEANER_SET_ULTRA_FAULT,
	IE_AIRCLEANER_SET_ANION_FAULT,

	/* 快热炉 联创  DF-HDW2001RA属性设置事件 */
	IE_AIRHEATER_SET_ONOFF_OK,
	IE_AIRHEATER_SET_GEAR_OK,
	IE_AIRHEATER_SET_TIME_OK,
	IE_AIRHEATER_SET_MODE_OK,
	IE_AIRHEATER_SET_ONOFF_FAULT,
	IE_AIRHEATER_SET_GEAR_FAULT,
	IE_AIRHEATER_SET_TIME_FAULT,
	IE_AIRHEATER_SET_MODE_FAULT,

	/* 热水器属性设置事件 */
	IE_WATERHEATER_SET_WORK_OK,
	IE_WATERHEATER_SET_TEMP_OK,
	IE_WATERHEATER_SET_TIMER_OK,
	IE_WATERHEATER_SET_CAPACTITY_OK,
	IE_WATERHEATER_SET_WORK_FAULT,
	IE_WATERHEATER_SET_TEMP_FAULT,
	IE_WATERHEATER_SET_TIMER_FAULT,
	IE_WATERHEATER_SET_CAPACTITY_FAULT,

	/* 空调属性设置事件 */
	IE_AIRCONDITION_SET_ONOFF_OK,
	IE_AIRCONDITION_SET_MODE_OK,
	IE_AIRCONDITION_SET_TEMP_OK,
	IE_AIRCONDITION_SET_TIMER_OK,
	IE_AIRCONDITION_SET_ONOFF_FAULT,
	IE_AIRCONDITION_SET_MODE_FAULT,
	IE_AIRCONDITION_SET_TEMP_FAULT,
	IE_AIRCONDITION_SET_TIMER_FAULT,

	/* 风扇属性设置事件 */
	IE_ELECTRICFAN_SET_WORK_OK,
	IE_ELECTRICFAN_SET_GEAR_OK,
	IE_ELECTRICFAN_SET_TIMER_OK,
	IE_ELECTRICFAN_SET_SHAKE_OK,
	IE_ELECTRICFAN_SET_WORK_FAULT,
	IE_ELECTRICFAN_SET_GEAR_FAULT,
	IE_ELECTRICFAN_SET_TIMER_FAULT,
	IE_ELECTRICFAN_SET_SHAKE_FAULT,

	/* 设备属性发生变化 */
	IE_UPDATE_STATUS,

	/* 前锋热水器属性设置事件 */
	IE_WATERHEATER_A9_SET_WORK_OK,
	IE_WATERHEATER_A9_SET_TEMP_OK,
	IE_WATERHEATER_A9_SET_MODE_OK,
	IE_WATERHEATER_A9_CLEAR_CNT_OK,
	IE_WATERHEATER_A9_SET_FIRE_LEVEL_OK,
	IE_WATERHEATER_A9_SET_WORK_FAULT,
	IE_WATERHEATER_A9_SET_TEMP_FAULT,
	IE_WATERHEATER_A9_SET_MODE_FAULT,
	IE_WATERHEATER_A9_CLEAR_CNT_FAULT,
	IE_WATERHEATER_A9_SET_FIRE_LEVEL_FAULT,



	/* 快热炉 联创油汀 属性设置事件 */
	IE_AIRHEATER_YCYT_SET_TEMP_OK,
	IE_AIRHEATER_YCYT_SET_MODE_OK,
	IE_AIRHEATER_YCYT_SET_GEAR_OK,
	IE_AIRHEATER_YCYT_SET_TIMER_OK,
    IE_AIRHEATER_YCYT_SET_ONOFF_OK,
    IE_AIRHEATER_YCYT_SET_ORDER_TIMER_OK,
    
	IE_AIRHEATER_YCYT_SET_TEMP_FAULT,
	IE_AIRHEATER_YCYT_SET_MODE_FAULT,
	IE_AIRHEATER_YCYT_SET_GEAR_FAULT,
	IE_AIRHEATER_YCYT_SET_TIMER_FAULT,
    IE_AIRHEATER_YCYT_SET_ONOFF_FAULT,
    IE_AIRHEATER_YCYT_SET_ORDER_TIMER_FAULT,
    /*奥普浴霸*/
	IE_BATHHEATER_SET_WORK_OK,
	IE_BATHHEATER_SET_LIGHT_OK,
	IE_BATHHEATER_SET_ANION_OK,
	IE_BATHHEATER_SET_BREATH_OK,
	IE_BATHHEATER_SET_DRY_OK,
	IE_BATHHEATER_SET_TRONIC_OK,
	IE_BATHHEATER_SET_TIME_OK,
	IE_BATHHEATER_SET_WORK_FAULT,
	IE_BATHHEATER_SET_LIGHT_FAULT,
	IE_BATHHEATER_SET_ANION_FAULT,
	IE_BATHHEATER_SET_BREATH_FAULT,
	IE_BATHHEATER_SET_DRY_FAULT,
	IE_BATHHEATER_SET_TRONIC_FAULT,
	IE_BATHHEATER_SET_TIME_FAULT,

	/* 空气净化器属性设置事件(海科) */
	IE_AIRCLEANER_SET_MODE_OK,
	IE_AIRCLEANER_SET_MODE_FAULT,
	/* 空气进化器臭氧杀菌事件(南柏) */
	IE_AIRCLEANER_SET_TERILIZE_OK,
	IE_AIRCLEANER_SET_TERILIZE_FAULT,

	IE_END = IE_BEGIN + 99
};

/**********************************************
	  空调按键的宏定义
 **********************************************/

// 开关
#define	AC_POWER_ON	0
#define	AC_POWER_OFF	1

// 模式
#define	AC_MODE_AUTO	0
#define	AC_MODE_COLD	1
#define	AC_MODE_AREFACTION 2
#define	AC_MODE_WIND	3
#define	AC_MODE_HOT	4

#define	AC_TEMP_BASE	16
#define AC_TEMP_MAX     32

// 风速
#define	AC_WIND_AUTO	0
#define	AC_WIND_1	1
#define	AC_WIND_2	2
#define	AC_WIND_3	3

// 风向
#define	AC_DIR_AUTO	0
#define	AC_DIR_1	1
#define	AC_DIR_2	2
#define	AC_DIR_3	3

#define	AC_KEY_POWER	0
#define	AC_KEY_MODE     1
#define	AC_KEY_TEMP     2
#define	AC_KEY_WIND     3
#define	AC_KEY_DIR		4


/**************************************************************************************************
	  空气净化器等相关信息 
 **************************************************************************************************/

typedef struct {
	 /* 开关 0：待机; 1: 工作 (默认高风速) */
	u_int16_t onoff;

	/* 紫外线 0: 关闭; 1: 开启 */
	u_int16_t ultra;

	/* 负离子 0: 关闭; 1: 开启  */
	u_int16_t anion;

	/* 风速 2: 低风速; 3: 中风速; 4: 高风速 */
	u_int16_t speed;
	
	/* 定时时间(分钟)， 0表示取消定时，其他为设置的定时分钟数，最多24小时(24*60)
	* 注意：
	*	如果当前处于待机状态，定时表示定时开始工作
	*	如果当前处于工作状态，定时表示定时关闭，进入待机模式
	*	海科净化器表示定时剩余分钟数，并参考timer_type	
	*/
	u_int16_t timer;

	/* PM2.5 数值对应PM2.5数值 */
	u_int16_t pm25;

	/* 温度 实际温度 * 10 */
	u_int16_t temp;

	/* 湿度 实际湿度 */
	u_int16_t rh;

	/* 功率 */
	u_int16_t power;

	/*海科空气净化器工作模式*/
	u_int16_t work_mode;
	/*海科空气净化器定时器类型: 1 定时开，2 定时关*/
	u_int16_t timer_type; 
	/*海科空气净化器定设定的定时小时*/
	u_int16_t set_hour;	

	/*南柏臭氧杀菌 0: 关闭; 1: 开启 */
	u_int16_t terilize;
	/*南柏臭氧杀菌时间*/
	u_int16_t terilize_minute;
	/*南柏滤网使用寿命分钟*/
	u_int32_t rosebox_life;
	cl_air_timer_info_t periodic_timer; //周期定时器
	
	u_int16_t pad;
} cl_ia_aircleaner_info_t;






/*
	功能:
		开关空气净化器
	输入参数:
		@dev_handle:  空气净化器的句柄
		@onoff:  1,开，0，关
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_onoff(cl_handle_t dev_handle, bool onoff);




/*
	功能:
		设置空气净化器风速
	输入参数:
		@dev_handle:  空气净化器的句柄
		@speed:  1: 低风速; 2: 中风速; 3: 高风速; 4: 超高风速(海科)
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_speed(cl_handle_t dev_handle, u_int8_t speed);





/*
	功能:
		设置空气净化器定时器
	注意：
　　		如果当前处于待机状态，定时表示定时开始工作
		如果当前处于工作状态，定时表示定时关闭，进入待机模式
	输入参数:
		@dev_handle:  空气净化器的句柄
		@min:  0取消定时，其他为设置的定时分钟数，最多24小时(24*60)
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_timer(cl_handle_t dev_handle, u_int32_t min);




/*
	功能:
		设置空气净化器的紫外线功能
	输入参数:
		@dev_handle:  空气净化器的句柄
		@onoff:  0: 关闭; 1: 开启
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_ultra(cl_handle_t dev_handle, bool onoff);






/*
	功能:
		设置空气净化器的负离子功能
	输入参数:
		@dev_handle:  空气净化器的句柄
		@onoff:  0: 关闭; 1: 开启
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_anion(cl_handle_t dev_handle, bool onoff);


/*
	功能:
		设置空气净化器的工作模式
		海科空气净化器特有
	输入参数:
		@dev_handle:  空气净化器的句柄
		@mode: 1: 自动 2: 手动 3: 睡眠
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
	功能:
		查询空气净化器的所有工作状态
		目前仅udp协议支持
	输入参数:
		@dev_handle:  空气净化器的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_query_all(cl_handle_t dev_handle);

/*
	功能:
		设置空气净化器的臭氧杀菌功能
		南柏空气净化器特有
	输入参数:
		@dev_handle:  空气净化器的句柄
		@is_on:  0: 关闭 1: 开启
		@minute: 臭氧杀菌时间，支持10分钟、30分钟、60分钟
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircleaner_set_terilize(cl_handle_t dev_handle, u_int8_t is_on, u_int8_t minute);

/*
 功能:
    添加周期定时器
 输入参数:
    @time_info: 定时器信息 id若存在，则为修改
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ia_aircleaner_add_periodic_timer(cl_handle_t dev_handle, cl_air_timer_t* time_info);

/*
 功能:
    删除周期定时器
 输入参数:
    @timer_id : 定时器ID
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ia_aircleaner_del_periodic_timer(cl_handle_t dev_handle, u_int8_t timer_id);


/**************************************************************************************************
	  快热炉/暖风机等相关信息 联创  DF-HDW2001RA
 **************************************************************************************************/

typedef struct {
	/* 通断电情况 0表示关机， 1表示开机 */
	u_int16_t onoff;
	
	/* 档位 0: 无 1:低 2:中 3:高 */
	u_int16_t gear;
	
	/* 预约定时 
	*	0: 取消
	*	60:1h      120:2h      180:3h
	*	240:4h     300:5h      360:6h
	*	420:7h     480:8h      540:9h
	*	600:10h    660:11h     720:12h
	*	780:13h    840:14h     900:15h
	*	注意：
	*	1.当前为关机状态，表示什么时候开机
	*
	*	2.当前为开机状态，表示什么时候关机
	*
	*	离预约时间还剩下多少分钟。注意
	*	3.只有在预约时间不为0情况下有效
	*	4.当前为关机状态，表示什么时候开机
	*	5.当前为开机状态，表示什么时候关机
	*	预约时间到后，执行相应开/关机动作，同时把预约定时状态变为取消
	*/
	u_int16_t time;

	/* 智能 0: 无 1:睡眠 2:省电 3:舒适 4:速热 */
	u_int16_t mode;

	/* 功率 */
	u_int16_t power;

	/* 当前温度 */
	u_int16_t temp;
} cl_ia_airheater_info_t;



/*
	功能:
		开关快热炉
	输入参数:
		@dev_handle:  快热炉的句柄
		@onoff:  1,开，0，关
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_set_onoff(cl_handle_t dev_handle, bool onoff);



/*
	功能:
		快热炉档位设置
	输入参数:
		@dev_handle:  快热炉的句柄
		@gear:  1:低 2:中 3:高
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_set_gear(cl_handle_t dev_handle, u_int16_t gear);



/*
	功能:
		快热炉预约时间设置
	输入参数:
		@dev_handle:  快热炉的句柄
		@time: 
			*	0: 取消
			*	60:1h      120:2h      180:3h
			*	240:4h     300:5h      360:6h
			*	420:7h     480:8h      540:9h
			*	600:10h    660:11h     720:12h
			*	780:13h    840:14h     900:15h
			*	注意：
			*	1.当前为关机状态，表示什么时候开机
			*
			*	2.当前为开机状态，表示什么时候关机
			*
			*	离预约时间还剩下多少分钟。注意
			*	3.只有在预约时间不为0情况下有效
			*	4.当前为关机状态，表示什么时候开机
			*	5.当前为开机状态，表示什么时候关机
			*	预约时间到后，执行相应开/关机动作，同时把预约定时状态变为取消
	
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_set_time(cl_handle_t dev_handle, u_int16_t time);


/*
	功能:
		快热炉模式设置
	输入参数:
		@dev_handle:  快热炉的句柄
		@mode:  1:睡眠 2:省电 3:舒适 4:速热
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_set_mode(cl_handle_t dev_handle, u_int16_t mode);


/**************************************************************************************************
	  快热炉/暖风机等相关信息 联创油汀
 **************************************************************************************************/


typedef struct {
	/* 设置温度 1-35 */
	u_int16_t set_temp;

	/* 当前温度 0-51 */
	u_int16_t cur_temp;

	/* 1:睡眠 2:省电 3:舒适 4:速热 5:温控 */
	u_int16_t mode;
	
	/* 1:关闭 2:低档 3:中档 4:高档 */
	u_int16_t gear;
	
	/* 预约定时时间 
	*	设置时候，取60分钟(小时)整数倍。
	*	查询回来的时间，表示执行倒计时。
	*/
	u_int16_t time; //定时关机剩余时间

	u_int16_t onoff;
    
    	u_int16_t time_on;/*预约开机剩余时间*/

	u_int16_t time_set_off; /*设置的多久关机*/
	u_int16_t time_set_on; /*设置的多久开机*/
} cl_ia_airheater_ycyt_info_t;




/*
	功能:
		设置快热炉/暖风机工作温度
	输入参数:
		@dev_handle:  热水器的句柄
		@temp:  范围为1-35
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_ycyt_set_temp(cl_handle_t dev_handle, u_int16_t temp);




/*
	功能:
		控制设置快热炉/暖风机工作模式
	输入参数:
		@dev_handle:  热水器的句柄
		@mode:  	 1:睡眠 2:省电 3:舒适 4:速热 5:温控
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_ycyt_set_mode(cl_handle_t dev_handle, u_int16_t mode);




/*
	功能:
		控制设置快热炉/暖风机工作档位
	输入参数:
		@dev_handle:  热水器的句柄
		@gear:  	  1:关闭 2:低档 3:中档 4:高档
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_ycyt_set_gear(cl_handle_t dev_handle, u_int16_t gear);





/*
	功能:
		控制设置快热炉/暖风机工作档位
	输入参数:
		@dev_handle:  热水器的句柄
		@time:  	  单位小时
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_airheater_ycyt_set_timer(cl_handle_t dev_handle, u_int16_t time);
    
/*
 功能:
  设置开启或者关闭油汀
 输入参数:
 @dev_handle:  热水器的句柄
 @on_off:  	  开启/关闭
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ia_airheater_ycyt_set_onoff(cl_handle_t dev_handle, bool onoff);

/*
 功能:
 设置油汀预约开机时间
 输入参数:
 @dev_handle:  热水器的句柄
 @time:  	  单位小时
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ia_airheater_ycyt_set_order_timer(cl_handle_t dev_handle, u_int16_t time);

/*
 功能:
刷新油汀定时时间 和倒计时时间
 @dev_handle:  热水器的句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ia_airheater_ycyt_refresh_timer(cl_handle_t dev_handle);


/**************************************************************************************************
	  热水器等相关信息 
 **************************************************************************************************/

typedef struct {
	/* 工作状态 
	*  对于查询：0表示待机中，1工作中
	*  对于控制：0表示让热水器停止工作 1 表示让热水器开始工作(加热) 
	*/
	u_int16_t work;
	
	/* 设置水温 范围为35-75，表示具体设置的摄氏度 */
	u_int16_t temp_set;

	/* 当前水温 实际的摄氏度 */
	u_int16_t temp_current;

	/* 预约时间(分钟) 
	* 0取消定时，表示设置的定时分钟数，最多为24小时(24*60)
	* 注意
	*    由于设计要求，传入的时间必须是60的倍数，也就是小时。
	*　　如果当前处于待机状态，定时表示定时开始工作
	*    如果当前处于工作状态，定时表示定时关闭，进入待机模式
	*/
	u_int16_t timer;
	
	/* 容量 1表示半胆， 2表示全胆 */
	u_int16_t capactity;

	/* 功率 100W为单位 */
	u_int16_t power;
} cl_ia_waterheater_info_t;






/*
	功能:
		控制热水器是否工作
	输入参数:
		@dev_handle:  热水器的句柄
		@work:  0表示让热水器停止工作 1 表示让热水器开始工作(加热) 
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_set_work(cl_handle_t dev_handle, bool work);




/*
	功能:
		设置热水器加热温度
	输入参数:
		@dev_handle:  热水器的句柄
		@temp:  范围为35-75
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_set_temp(cl_handle_t dev_handle, u_int16_t temp);



/*
	功能:
		设置热水器的预约时间
	输入参数:
		@dev_handle:  热水器的句柄
		@timer:  	* 0取消定时，表示设置的定时分钟数，最多为24小时(24*60)
				* 注意
				*    由于设计要求，传入的时间必须是60的倍数，也就是小时。
				*　　如果当前处于待机状态，定时表示定时开始工作
				*    如果当前处于工作状态，定时表示定时关闭，进入待机模式
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_set_timer(cl_handle_t dev_handle, u_int16_t timer);


/*
	功能:
		设置热水器容量
	输入参数:
		@dev_handle:  热水器的句柄
		@capactity:  1表示半胆， 2表示全胆 
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_set_capactity(cl_handle_t dev_handle, u_int16_t capactity);


/**************************************************************************************************
	  前锋热水器等相关信息 
 **************************************************************************************************/
typedef struct {
	/* 设置水温 范围为35-65，表示具体设置的摄氏度 */
	u_int16_t temp_set;

	/* 当前水温 实际的摄氏度 */
	u_int16_t temp_current;

	/* 功能: 1 自动 2 浴缸 3 洗碗 4 洗菜 5 洗衣 */
	u_int16_t mode;

	/* 工作状态 
	* bit 0: 风机是否开启(0 关闭 1 开启)    
	* bit 1: 水流是否开启(0 关闭 1 开启)
	* bit 2: 是否在燃烧(0 否 1 是)   
	* bit 3: 是否设置为了优先(0 否 1 是)   
	* bit 4: 是否超量小(0 否 1 是)
	* bit 5: 是否为超量大(0 否 1 是)
	* bit 6: 是否T1(0 否 1 是)
	* bit 7: 是否T2(0 否 1 是)
	*/
	u_int16_t work;

	/* 燃烧分段
	*  0：不燃烧
	*  1：左燃烧
	*  2：右燃烧
	*  3：全燃烧
	*/
	u_int16_t fire_level;
	
	/* 累计水流量 单位:L */
	u_int16_t count;

	/* 耗气量 0-6 */
	u_int16_t gas;
} cl_ia_waterheater_a9_info_t;


/*
	功能:
		控制热水器燃烧分段
	输入参数:
		@dev_handle:  热水器的句柄
		@level:  	0：不燃烧
	   			1：左燃烧
	   			2：右燃烧
	   			3：全燃烧
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_a9_set_fire_level(cl_handle_t dev_handle, u_int16_t level);


/*
	功能:
		清楚热水器水流量记录
	输入参数:
		@dev_handle:  热水器的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_a9_clear_cnt(cl_handle_t dev_handle);


/*
	功能:
		控制热水器工作模式
	输入参数:
		@dev_handle:  热水器的句柄
		@mode:  	 1 自动 2 浴缸 3 洗碗 4 洗菜 5 洗衣
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_a9_set_mode(cl_handle_t dev_handle, u_int16_t mode);





/*
	功能:
		设置热水器加热温度
	输入参数:
		@dev_handle:  热水器的句柄
		@temp:  范围为35-65
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_waterheater_a9_set_temp(cl_handle_t dev_handle, u_int16_t temp);









/**************************************************************************************************
	  空调等相关信息 
 **************************************************************************************************/

typedef struct {
	/* 通断电情况 0表示关机， 1表示开机 */
	u_int16_t onoff;

	/* 模式 0 自动 1 制热 2 制冷 3 除湿 4 通风 */
	u_int16_t mode;

	/* 温度 16-32 表示对应温度 */
	u_int16_t temp;

	/* 功率 */
	u_int16_t power;

	/* 定时 */
	u_int16_t timer;

	/* 当前温度 */
	u_int16_t cur_temp;
} cl_ia_aircondition_info_t;



/*
	功能:
		设置空调开关、模式和温度
	输入参数:
		@dev_handle:  空调的句柄
		@onoff:  0表示关机， 1表示开机
		@mode:  0 自动 1 制热 2 制冷 3 除湿 4 通风
		@temp:  16-32 表示对应温度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircondition_set(cl_handle_t dev_handle, bool onoff, u_int16_t mode, u_int16_t temp);



/*
	功能:
		设置空调定时器
	输入参数:
		@dev_handle:  空调的句柄
		@timer:  0表示取消定时，其他为设置的定时分钟数，最大(24 * 60)分钟
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_aircondition_set_timer(cl_handle_t dev_handle, u_int16_t timer);



/**************************************************************************************************
	  风扇等相关信息 
 **************************************************************************************************/

typedef struct {
	/* 工作状态: 0 待机 1 工作 */
	u_int16_t work;

	/* 风量 1 睡眠 2 风量弱 3 风量中 4 风量高*/
	u_int16_t gear;

	/* 设置定时器，分钟为单位 */
	u_int16_t timer;

	/* 风扇摆头 0 关闭 1开启 */
	u_int16_t shake;

	/* 功率 */
	u_int16_t power;
	
	u_int16_t pad;
} cl_ia_electricfan_info_t;



/*
	功能:
		控制风扇是否工作
	输入参数:
		@dev_handle:  风扇的句柄
		@work:  0 待机 1 工作
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_electricfan_set_work(cl_handle_t dev_handle, bool work);


/*
	功能:
		控制风扇的风量
	输入参数:
		@dev_handle:  风扇的句柄
		@gear:  风量 1: 睡眠档 2: 低风档 3: 中风档 4: 强风档
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_electricfan_set_gear(cl_handle_t dev_handle, u_int16_t gear);


/*
	功能:
		控制风扇的定时器
	输入参数:
		@dev_handle:  风扇的句柄
		@timer:  0取消定时，其他为设置的定时分钟数，最多7.5小时，30分钟的倍数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_electricfan_set_timer(cl_handle_t dev_handle, u_int16_t timer);



/*
	功能:
		控制风扇摆头
	输入参数:
		@dev_handle:  风扇的句柄
		@shake:  风扇摆头 0 关闭 1开启

	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_electricfan_set_shake(cl_handle_t dev_handle, u_int16_t shake);

/////////////////////////////////////////////////////////////
//  浴霸数据

typedef struct{
	u_int16_t power_on_off; // 开启、关闭
	u_int16_t anion_on_off; // 负离子
	u_int16_t light_on_off; //照明
	u_int16_t breath_on_off; //换气
	u_int16_t dry_on_off; //干燥
	u_int16_t tronic_on_off; //风暖
	u_int16_t next_time; //下次开时间
}cl_ia_bath_heater_info_t;

/*
	功能:
		控制浴霸的工作
	输入参数:
		@dev_handle:  浴霸的句柄
		@work: 是否工作
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_work(cl_handle_t dev_handle, u_int16_t work);

/*
	功能:
		控制负离子
	输入参数:
		@dev_handle:  浴霸的句柄
		@anion: 是否工作
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_anion(cl_handle_t dev_handle, u_int16_t anion);

/*
	功能:
		控制灯光
	输入参数:
		@dev_handle:  浴霸的句柄
		@light: 是否工作
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_light(cl_handle_t dev_handle, u_int16_t light);

/*
	功能:
		控制换气
	输入参数:
		@dev_handle:  浴霸的句柄
		@breath: 换气是否工作
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_breath(cl_handle_t dev_handle, u_int16_t breath);

/*
	功能:
		控制干燥模式
	输入参数:
		@dev_handle:  浴霸的句柄
		@dry: 干燥是否工作
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_dry(cl_handle_t dev_handle, u_int16_t dry);

/*
	功能:
		控制风暖
	输入参数:
		@dev_handle:  浴霸的句柄
		@tronic: 0 关闭风暖 1.风暖弱，2风暖强
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_tronic(cl_handle_t dev_handle, u_int16_t tronic);

/*
	功能:
		控制定时器
	输入参数:
		@dev_handle:  浴霸的句柄
		@timer:  0取消定时，其他为设置的定时分钟数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ia_bath_heater_set_timer(cl_handle_t dev_handle, u_int16_t timer);

/* 智能家居类型 空调 */ 
#define IA_AIRCONDITION 1
/* 智能家居类型 热水器 */ 
#define IA_WATERHEATER 2
/* 智能家居类型 快热炉 */ 
#define IA_AIRHEATER 3
/* 智能家居类型 空气净化器 */ 
#define IA_AIRCLEANER 4
/* 智能家居类型 风扇 */ 
#define IA_ELECTRICFAN 5
/* 智能家居类型 奥普浴霸 */ 
#define IA_BATHHEATER_AUPU 6

#define IA_UDP_AIR_HEATER 0x12
#define IA_UDP_AIR_CLEAR 0x13 // UDP油汀


#define IA_EXT_TYPE_LCYT_OIL 0x1 
#define IA_EXT_TYPE_813_HK 0x1 
#define IA_EXT_TYPE_813_NB 0x2 




/* 空调子类型 */
#define IA_AIRCONDITION_SUBTYPE_PROTO  0
/* 热水器子类型 */
#define IA_WATERHEATER_SUBTYPE_PROTO  0
#define IA_WATERHEATER_SUBTYPE_A9	  1
/* 快热炉子类型 */
#define IA_AIRHEATER_SUBTYPE_PROTO  0
#define IA_AIRHEATER_SUBTYPE_YCYT	  1

/* 空气净化器子类型 */
#define IA_AIRCLEANER_SUBTYPE_PROTO  0
/* 风扇子类型 */
#define IA_ELECTRICFAN_SUBTYPE_PROTO  0

//高讯LED调光灯
enum {	
	GX_LED_STATUS_OFF = 0,	 	 // 关灯	
	GX_LED_STATUS_ON = 1,	  	 // 开灯	
	GX_LED_STATUS_FULL_LIGHT = 2,// 全光
	GX_LED_STATUS_WHITE = 3,	 // 白光	
	GX_LED_STATUS_WARM = 4, 	 // 暖光	
	GX_LED_STATUS_MIX = 5,  	 // 混光	
	GX_LED_STATUS_AUTO = 6, 	 // 自动	
	GX_LED_STATUS_SAVE = 7, 	 // 节能	
	GX_LED_STATUS_MODEL1 = 8, 	 // MODEL1	
	GX_LED_STATUS_MODEL2 = 9, 	 // MODEL2	
	GX_LED_STATUS_MAX
};

typedef struct {
	u_int8_t led_status; // LED状态， 取值见上,如 GX_LED_STATUS_ON	
	u_int8_t brightness;		 // 亮度值	
	u_int8_t warmness;		 // 暖光值	
	u_int8_t red;		 // RED	
	u_int8_t green;      // GREEN	
	u_int8_t blue;       // BLUE	
	u_int8_t night_status;// 夜灯状态 0-关闭，1-开启	
	u_int8_t reserved;	 // 保留
} cl_ia_gx_led_info_t;

/*
	E宝
*/
typedef struct{
	u_int8_t is_support_elec_info;
	u_int8_t pad[3];
	u_int32_t current_power; // 当前功率
	u_int32_t current_mil_power; // 当前功率
	cl_elec_stat_info_t elec_stat_info; //电量统计信息
	cl_air_elec_item_info total_elec; //总电量
	cl_air_elec_item_info period_elec; //阶段电量
	cl_air_elec_item_info last_on_elec; //上次开机电量
	cl_elec_days_stat_info elec_days_info; //电量统计之365天统计数据
}cl_common_elec_info;

typedef struct {
	u_int8_t on_off_valid;	// 开关机状态是否有效
	// 开启、关闭
	u_int8_t on_off; 
	//是否支持时间段定时器
	u_int8_t is_support_period_timer;
    //是否支持控制LED
    u_int8_t is_support_ctrl_led;
    //led是否开启
    u_int8_t led_onoff;
	// 定时器
	cl_air_timer_info_t timer;
	//电量统计相关功能
	cl_common_elec_info elec_info;
} cl_ia_eb_info_t;

/*
*  彩虹电热毯
*/
#define MAX_CURVE_DATA_NUM 48
typedef struct {
	u_int8_t work_stat; //开关
	u_int8_t set_temperature;//用户设置温度
	u_int8_t current_temperature; //温区当前温度
	u_int8_t off_timer; //手工定时剩余时间
	u_int8_t curve_enable; //曲线是否启用
	u_int8_t curve_week; //曲线启用周期
	u_int16_t curve_time_interval;//曲线间隔时间粒度, 60代表一个小时
	u_int16_t curve_next_work_time;//下次开启或者关闭时间
	u_int8_t curve_data_len;
	u_int8_t work_mode; //工作模式
	u_int8_t curve_data[MAX_CURVE_DATA_NUM];
}cl_ia_ch_area_info_t;

typedef struct {
	cl_ia_ch_area_info_t left_area_info;
	cl_ia_ch_area_info_t right_area_info;
}cl_ia_ch_blanket_info_t;


/*
* 获取cl_ia_info_t信息，需要调用cl_user_get_dev_info接口返回cl_dev_info_t*，然后再从cl_dev_info_t中提取cl_ia_info_t。
*/
typedef struct _cl_ia_info_s_{
	/* 智能家居类型 */ 
	u_int8_t ia_type;
	/* 智能家居子类型 */
	u_int8_t ia_sub_type;
	u_int8_t pad[2];
	/* 根据类型选项不同句柄 */
	union {
		/* 空气净化器 */ 
		cl_ia_aircleaner_info_t  *aircleaner_info;
		/* 快热炉 */ 
		cl_ia_airheater_info_t	 *airheater_info;
		/* 快热炉 ycyt */ 
		cl_ia_airheater_ycyt_info_t	 *airheater_ycyt_info;
		/* 热水器 */ 
		cl_ia_waterheater_info_t *waterheater_info;
		/* 前锋热水器 */
		cl_ia_waterheater_a9_info_t *waterheater_a9_info;
		/* 空调 */ 
		cl_ia_aircondition_info_t *aircondition_info;
		/* 风扇 */ 
		cl_ia_electricfan_info_t  *electricfan_info;
		/*浴霸*/
		cl_ia_bath_heater_info_t* bath_heater_info;
		/* 高讯调光灯 */
		cl_ia_gx_led_info_t *gx_led_info;
		/* E宝 */
		cl_ia_eb_info_t *eb;
		//彩虹电热毯
		cl_ia_ch_blanket_info_t* ch_blanket;
		
		void *ptr;
	} u;
} cl_ia_info_t;


#ifdef __cplusplus
}
#endif 

#endif

