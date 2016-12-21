#ifndef CL_DRKZQ_H
#define CL_DRKZQ_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

// cl_drkzq_ctrl的action 
enum {
	act_drkzq_humi_threshold1 = 1,	// 区域1湿度阀值控制0-100
	act_drkzq_humi_threshold2,	// 区域1湿度阀值控制0-100
	act_drkzq_humi_threshold3,	// 区域1湿度阀值控制0-100
	act_drkzq_humi_threshold4,	// 区域1湿度阀值控制0-100
	act_drkzq_safe_temp_threshold,	// 土壤安全温度阀值0-60摄氏度
	act_drkzq_cycling_water_temp_threshold,	// 回水池安全温度阀值 0-60摄氏度
	act_drkzq_fertilzer_hour,	// 施肥时间 小时
	act_drkzq_sunshine_threshold,	// 开光照阀值
	act_drkzq_safe_ph_threshold,	// 安全PH阀值0-100
	act_drkzq_watering_onoff1,// 区域1浇灌控制
	act_drkzq_watering_onoff2,// 区域2浇灌控制
	act_drkzq_watering_onoff3,// 区域3浇灌控制
	act_drkzq_watering_onoff4,// 区域4浇灌控制
	act_drkzq_sunshine_onoff,	// 光照控制
	act_drkzq_feretilize_onoff,	// 施肥控制
	act_drkzq_light_onoff,	// 照明控制
	act_drkzq_water_pump_onoff,	// 水泵控制
	act_drkzq_resv_onoff1,	// 保留控制1
	act_drkzq_resv_onoff2,	// 保留控制1
	act_drkzq_resv_onoff3,	// 保留控制1
	act_drkzq_resv_onoff4,	// 保留控制1
	act_drkzq_work_mode,	// 工作模式1全自动模式，2定时模式，3手动模式，4定时自动模式
	act_drkzq_sys_time, // 系统时间 用2字节表示，低8位为小时，高8为分钟

	// 下面的SDK自己使用
	act_drkzq_timer,
	act_drkzq_name,
};

enum {
	// 4个区域的名字
	DRKZQ_NAME_ZERO1,
	DRKZQ_NAME_ZERO2,
	DRKZQ_NAME_ZERO3,
	DRKZQ_NAME_ZERO4,

	DRKZQ_NAME_LIGHT,	// 照明控制名字
	DRKZQ_NAME_WATER_PUMP,	// 水池水泵水泵控制

	// 4个保留控制名字
	DRKZQ_NAME_RESV1,
	DRKZQ_NAME_RESV2,
	DRKZQ_NAME_RESV3,
	DRKZQ_NAME_RESV4,

	DRKZQ_NAME_MAX,
};

typedef struct {
	// 4个区域的温湿度值
	u_int8_t humi1;
	u_int8_t temp1;
	u_int8_t humi2;
	u_int8_t temp2;
	u_int8_t humi3;
	u_int8_t temp3;
	u_int8_t humi4;
	u_int8_t temp4;

	u_int8_t ph;
	u_int8_t cycling_water_temp;	// 回水池水温
	u_int8_t analog;	// 模拟采集值

	// 四个区域浇灌控制0 关 1 开
	u_int8_t watering_onoff1;
	u_int8_t watering_onoff2;
	u_int8_t watering_onoff3;
	u_int8_t watering_onoff4;

	u_int8_t sunshine_onoff;	// 光照控制
	u_int8_t feretilize_onoff;	// 施肥控制
	u_int8_t light_onoff;	// 照明控制
	u_int8_t water_pump_onoff;	// 水泵控制

	// 4个备用控制
	u_int8_t resv_onoff1;
	u_int8_t resv_onoff2;
	u_int8_t resv_onoff3;
	u_int8_t resv_onoff4;

	u_int8_t work_mode;	// 工作模式1全自动模式，2定时模式，3手动模式，4定时自动模式

	// 四个区域浇灌湿度阀值
	u_int8_t humi_threshold1;	
	u_int8_t humi_threshold2;
	u_int8_t humi_threshold3;
	u_int8_t humi_threshold4;

	u_int8_t safe_temp_threshold;	// 土壤安全温度阀值
	u_int8_t cycling_water_temp_threshold;	// 回水池安全温度阀值
	u_int8_t fertilzer_second;	// 施肥时间4~120,单位为秒，打开施肥开关时间
	u_int8_t sunshine_threshold;	// 开光照阀值
	u_int8_t safe_ph_threshold;	// 安全PH阀值
	u_int8_t sys_min;	// 系统时间 分钟
	u_int8_t sys_hour;	// 系统时间小时

	// 第一组定时器
	u_int8_t timer1_start_min;
	u_int8_t timer1_start_hour;
	u_int8_t timer1_end_min;
	u_int8_t timer1_end_hour;
	u_int8_t timer1_valid;	// 是否有效

	// 第二组定时器
	u_int8_t timer2_start_min;
	u_int8_t timer2_start_hour;
	u_int8_t timer2_end_min;
	u_int8_t timer2_end_hour;
	u_int8_t timer2_valid;	// 是否有效
} cl_drkzq_stat_t;

// 当前异常
typedef struct {
	u_int8_t temp_sensor;	// 温度探头异常
	u_int8_t humi_sensor;	// 湿度探头异常
	u_int8_t ph_sensor;	// ph探头异常
	u_int8_t sunshine_sensor; // 光照探头异常
	u_int8_t high_temp;	// 温度过高异常
	u_int8_t lack_water;	// 缺水异常
	u_int8_t ph;	// ph异常
	u_int8_t humi;	// 湿度异常
} cl_drkzq_fault_t;

typedef struct {
	char name[64];
} cl_drkzq_name_item_t;

typedef struct {
	cl_drkzq_stat_t stat;
	cl_drkzq_fault_t fault;
	// 目前一共有10个地方需要设置名字，如果为空用默认
	cl_drkzq_name_item_t name[DRKZQ_NAME_MAX];
} cl_drkzq_info_t;

/*
	功能: 对净化器设置报文
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型act_drkzq_xxx，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_drkzq_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

/*
	功能: 对净化器设置某个模块的名字
		
	输入参数:
		@dev_handle: 设备的句柄
		@type: 名字DRKZQ_NAME_XXX
		@name: 名字，长度需要小于64字节
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_drkzq_set_name(cl_handle_t dev_handle, u_int8_t type, char *name);


/*
	功能: 对净化器设置某个定时器
		
	输入参数:
		@dev_handle: 设备的句柄
		@id: 目前只支持定时器1和2
		@valid:开启还是关闭定时器
		@start_hour start_min: 开始的小时分钟
		@end_hour end_min: 结束的小时分钟
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_drkzq_set_timer(cl_handle_t dev_handle, u_int8_t id, u_int8_t valid, u_int8_t start_hour, u_int8_t start_min, u_int8_t end_hour, u_int8_t end_min);

#ifdef __cplusplus
}
#endif 


#endif



