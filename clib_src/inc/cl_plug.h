#ifndef	__CL_PLUG_H__
#define	__CL_PLUG_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

// plug event
enum {
	PE_BEGIN = 400,
	// 从断电变成通电
	PE_ON = PE_BEGIN + 1,
	// 从通电变成断电
	PE_OFF = PE_BEGIN + 2,
	// 定时器列表发生变化。查询定时器列表的时间间隔约为统计信息查询间隔的3倍
	PE_TIMER_MODIFY = PE_BEGIN + 3,
	// 定时查询电流、电压、温度、累积电量、阶段电量返回了结果
	PE_QUERY = PE_BEGIN + 4,
	// 添加、删除、修改定时通电规则成功
	PE_TIMER_SET_OK = PE_BEGIN + 5,
	// 添加、删除、修改定时通电规则失败
	PE_TIMER_SET_FAIL = PE_BEGIN + 6,
	PE_END = PE_BEGIN + 99
};

typedef struct {
	u_int8_t id;			/* 策略ID */
	u_int8_t hour;			/* 小时 0-23 */
	u_int8_t minute;		/* 分钟 0-59 */
	u_int8_t week;			/* bit 0-6位对应星期天、星期1到星期六 */
	u_int8_t enable;		/* 是否生效(手机设置) 或者已经无效(设备返回) */
	u_int8_t pad;			/* 保留 */
	u_int16_t last;			/* 持续多久(分钟) */
	u_int32_t sort;			/* 当前开始，按触发时间排序。0表示禁止的，不参与排序。其他的从1开始 */
	char *name;		/* 策略名字, UTF-8格式 */
} cl_plug_timer_t;


typedef struct {
	// 是否通电
	u_int16_t is_on;
	// 电流，单位: 1/10 安培
	u_int16_t current;
	// 电压，单位: 伏特
	u_int16_t voltage;
	// 温度，单位: 摄氏度
	u_int16_t temperature;

	/* 从开机到现在的总电量,单位 W */
	u_int32_t electric_total; 
	/* 从time指定时文到当前的阶段电量,单位 W */
	u_int32_t electric_section; 
	/* 阶段电量从什么时候开始的 */
	u_int32_t section_begin_time;

	// 定时器有多少条
	u_int32_t num_timer;
	// 定时器指针数组
	cl_plug_timer_t **timer;

	// 表明next_on和next_time是否有效，用来表示定时器下一次的动作是开还是关，要等多久
	bool next_effect;
	// 下一次是通电还是断电
	bool next_on;
	// 还要多久发生，单位分钟
	u_int32_t next_minute;
} cl_plug_info_t;



/*
	功能:
		开始定时轮询某个遥控插座的状态
	输入参数:
		@slave_handle: 遥控插座的句柄
		@seconds: 多少秒查询一次
		@callback: 回调函数
		@handle: 回调参数，给调用者自己使用，SDK不关心该参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_query_start(cl_handle_t slave_handle, u_int32_t seconds, cl_callback_t callback, void *handle);

/*
	功能:
		停止定时轮询某个遥控插座的状态
	输入参数:
		@slave_handle: 遥控插座的句柄
	输出参数:
		无
	返回:
		无
*/
CLIB_API RS cl_plug_query_stop(cl_handle_t slave_handle);

/*
	功能:
		开关遥控插座
	输入参数:
		@slave_handle: 遥控插座的句柄
		@index: 控制哪个插孔。0表示所有。目前忽略该参数
		@on: 1表示通电，0表示断电
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_turn_on(cl_handle_t slave_handle, u_int32_t index, bool on);

/*
	功能:
		添加或修改定时通电规则
	输入参数:
		@slave_handle: 遥控插座的句柄
		@index: 控制哪个插孔。0表示所有。目前忽略该参数
		@plug_timer: 要设置的定时通电规则。
			plug_timer->name必须为UTF-8格式，长度小于64字节。
			如果plug_timer->id有效，为修改操作
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_timer_set(cl_handle_t slave_handle, u_int32_t index, cl_plug_timer_t *plug_timer, int32_t tz);

/*
	功能:
		把阶梯电量清0
	输入参数:
		@slave_handle: 遥控插座的句柄
		@index: 控制哪个插孔。0表示所有。目前忽略该参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_clear_electric_stat(cl_handle_t slave_handle, u_int32_t index);

/*
	功能:
		删除定时通电规则
	输入参数:
		@slave_handle: 遥控插座的句柄
		@index: 控制哪个插孔。0表示所有。目前忽略该参数
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_timer_del(cl_handle_t slave_handle, u_int32_t index, int id);

	
/*
	功能:
		获取该遥控插座的当前一些信息
	输入参数:
		@slave_handle: 遥控插座的句柄
		@index: 具体哪个插孔。目前忽略该参数
	输出参数:
		无
	返回:
		NULL: 失败
		其他: 成功, 返回的 cl_plug_info_t * 指向的内存块，需要用cl_plug_free_info函数释放
*/
CLIB_API cl_plug_info_t *cl_plug_get_info(cl_handle_t slave_handle, u_int32_t index, int32_t tz);

/*
	功能:
		释放 cl_plug_get_info() 函数返回的内存块
	输入参数:
		@info: cl_plug_get_info() 函数返回的内存块
	输出参数:
		无
	返回:
		无
*/
CLIB_API void cl_plug_free_info(cl_plug_info_t *info);

#ifdef __cplusplus
}
#endif 


#endif



