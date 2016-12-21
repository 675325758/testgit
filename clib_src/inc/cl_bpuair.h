/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 广州邦普空气控制器
**  File:    cl_bpuair.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/05/2016
**
**  Purpose:
**    广州邦普空气控制器.
**************************************************************************/


#ifndef CL_BPUAIR_H
#define CL_BPUAIR_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
// cl_bpuair_ctrl的action 
enum {
	ACT_BPUAIR_ONOFF,	// 开关机
	ACT_BPUAIR_MODE,    // 工作模式 1制冷 2 制热
	ACT_BPUAIR_ECO,		
	ACT_BPUAIR_COLD_TEMP,
	ACT_BPUAIR_HOT_TEMP,
	ACT_BPUAIR_TIMER_ENABLE,	// 定时器总开关
	ACT_BPUAIR_ONCE_TIMER,	// 一次性定时器
	ACT_BPUAIR_PERIOD_TIMER,	// 周期定时器
};


/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	bool valid;	// 是否有效
	u_int8_t week;	// 星期几，bit 0 - 6对应星期7 1 2 3 4 5 6
	u_int8_t onoff;	// 定时开还是关
	u_int8_t hour;	// 定时小时
	u_int8_t mins;	// 定时分钟
} cl_bpuair_timer_t;

typedef struct {
	u_int8_t id;
	bool valid;	// 是否有效
	u_int8_t week;	// 星期几，bit 0 - 6对应星期1-7
	u_int8_t onoff;	// 定时开还是关
	u_int8_t hour;	// 定时小时
	u_int8_t mins;	// 定时分钟
} cl_bpuair_timer_set_t;


typedef struct {
	u_int8_t type;	// 设备机型
	/*
		
		0：待机；
		1：运行；
		2：正在停机；
	*/
	u_int8_t stat1;	
	/*
		0:无
		1：防冻；
		2：除霜；
		3：防冻和除霜；
	*/
	u_int8_t stat2;
	u_int8_t work_mode;	// 工作模式 1制冷 2 制热
	u_int8_t eco_mode;	// 节能模式0 关闭 1开启
	int16_t cold_temp;	// 制冷设定温度，单位0.1度，10.0 ~ 30.0
	int16_t hot_temp;		// 制热设定温度，单位0.1度，10.0 ~ 85.0
	int16_t backwater_temp;	// 回水温度单位0.1度
	int16_t water_temp;	// 系统出水温度单位0.1度
	int16_t env_temp;	// 环境温度单位0.1度
	int16_t coiler1_temp;	// 盘管温度单位0.1度
	int16_t coiler2_temp;	// 盘管温度单位0.1度
	int16_t current1;	// 电流1 单位A
	int16_t current2;	// 电流2 单位A

	int16_t eco_cold_temp;
	int16_t eco_hot_temp;

	// 压机1的错误
	bool low_vol_1;	// 压机低压
	bool high_vol_1;	// 压机高压
	bool high_current_1;	// 压机电流过大
	bool low_current_1;	// 压机电流过小
	bool coiler_high_temp_1;	// 盘管温度探头故障
	bool exhaust_temp_sensor_1; // 排气温度探头故障
	bool exhaust_temp_high_1;	// 排气温度过高

	bool phase_protection_1;	// 缺相保护
	bool anti_phase_protection_1;	// 逆相保护

	// 压机2的错误
	bool low_vol_2;	// 压机低压
	bool high_vol_2;	// 压机高压
	bool high_current_2;	// 压机电流过大
	bool low_current_2;	// 压机电流过小
	bool coiler_high_temp_2;	// 盘管温度探头故障
	bool exhaust_temp_sensor_err_2; // 排气温度探头故障
	bool exhaust_temp_high_2;	// 排气温度过高

	bool fault_phase_protection_2;	// 错相保护
	bool eeprom_data_2;	// EEPROM数据错误
	bool const_temp_sensor_2; // 环温探头故障
	bool sys_temp_return_2;	// 系统回温故障
	bool sys_temp_out_2;	// 系统出温故障
	bool phase_protection_2;	// 缺相保护
	bool anti_phase_protection_2;	// 逆相保护

	bool lack_water;	// 水流不足
	bool high_temp_out_water;	// 出水温度过高
	bool low_temp_out_water;	// 出水温度过低

	// 定时器
	bool timer_enable;	// 定时器是否使能

	// 一次性定时器
	bool once_timer_enable;	// 一次性定时器使能
	bool once_timer_onoff;	// 一次性定时器设置 0：定时关机1：定时开机
	u_int8_t once_timer_hour;	// 一次性定时器小时
	u_int8_t once_timer_min;	// 一次性定时器分钟

	u_int16_t uptime;	// 运行天数

	char unit_code[64];// 机组编码
	char soft_ver1[64]; // 控制板软件版本号
	char soft_ver2[64]; // 手操器软件版本号

	
	/*
		错误数组
		[0]
		故障：某一位为1时存在该故障，0时无故障
		Bit0：1#压机低压	   （24） Bit1：1#压机高压		 （25）
		Bit2：1#压机电流过大   （26） Bit3：1#压机电流过小	（27）
		Bit4：1#盘管温度探头故障（28）Bit5：1#排气温度探头故障（29）
		Bit6：1#排气温度过高   （30） Bit7：备用			 （31）
		[1]
		故障：某一位为1时存在该故障，0时无故障
		Bit0：备用				（32）	Bit1：备用			（33）
		Bit2：备用				（34）	Bit3：备用			（35）
		Bit4：备用				（36）	Bit5：备用			 （37）
		Bit6：缺相保护			（38） Bit7：逆相保护		 （39） 			 
		[2]
		预留
		[3]
		故障：某一位为1时存在该故障，0时无故障
		Bit0：2#压机低压		 （48） Bit1：2#压机高压	 （49）
		Bit2：2#压机电流过大	（50） Bit3：1#压机电流过小 （51）
		Bit4：2#盘管温度探头故障（52）Bit5：2#排气温度探头故障（53）
		Bit6：2#排气温度过高	（54）		 Bit7：备用 	 （55）
		[4]
		预留
		[5]
		故障：某一位为1时存在该故障，0时无故障
		Bit0：备用		   （0）		  Bit1：错相保护	（1）
		Bit2：EEPROM 数据错（2）		  Bit3：环温探头故障（3）
		Bit4：系统回温故障 （4）		  Bit5：系统出温故障（5）
		Bit6：备用		   （6）		  Bit7：缺相保护	（7）
		[6]
		故障：某一位为1时存在该故障，0时无故障
		Bit0：水流不足	   （8）		   Bit1：备用		 （9）
		Bit2：出水温度过高（10）		   Bit3：出水温度过低（11）
		Bit4：备用		   （12）			Bit5：备用		   （13）
		Bit6：备用		   （14）			Bit7：备用			（15）
		
	*/
	u_int8_t fault_array[7];
	
	// 下个定时器的时间
	u_int32_t next_timer_stamp;	// 下个定时器开始的时间，本地时间
	bool next_timer_onoff;		// 下个定时器是开还是关
} cl_bpuair_stat_t;

typedef struct {
	u_int32_t timestamp;
	u_int32_t fault_number;
} cl_bpuair_fault_item_t;

typedef struct {
	u_int32_t valid;
	u_int32_t num;
	cl_bpuair_fault_item_t fault[10];
} cl_bpuair_fault_t;

typedef struct {
	u_int32_t num;
	cl_bpuair_fault_item_t fault[10];
} cl_bpuair_current_fault_t;
	
typedef struct {
	cl_bpuair_stat_t stat;
	cl_bpuair_timer_t timers[6];	// 周期定时器

	cl_bpuair_fault_t fault_history;

	cl_bpuair_current_fault_t fault_current;	// 当前故障

	// 下面SDK私有数据
	u_int32_t last_ctrl;
} cl_bpuair_info_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	功能: 对空调控制器进行设置
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_BPUAIR_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_bpuair_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

/*
	功能:一次性定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@enable: 是否使能
		@onoff: 开还是关 0 关 1 开
		@hour: 定时小时
		@min: 定时分钟
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_bpuair_once_timer(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int8_t hour, u_int8_t min);

/*
	功能: 周期定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@id: 1 - 7
		@enable: 是否使能
		@week:  星期几，bit 0 - 6对应星期1-7
		@onoff: 0 关 1 开
		@hour: 定时小时
		@min: 定时分钟
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_bpuair_period_timer(cl_handle_t dev_handle, u_int8_t id, u_int8_t enable, u_int8_t week, u_int8_t onoff, u_int8_t hour, u_int8_t min);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_BPUAIR_H */

