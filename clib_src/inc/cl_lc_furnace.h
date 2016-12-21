#ifndef	__CL_LC_FURNACE_H__
#define	__CL_LC_FURNACE_H__

/*
ah : air heater
联创暖风机DF-HT5313P
我们提供6621模块，通信使用udp协议
*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

//event of air heater
enum {
	AHE_BEGIN = 1400,
	AHE_CTRL_OK = AHE_BEGIN + 1, //控制成功，包含电源、摇头、档位、ECO
	AHE_CTRL_FAIL = AHE_BEGIN + 2, //控制失败
	AHE_CTRL_TEMP_OK  = AHE_BEGIN + 3, // 设置温度成功
	AHE_CTRL_TEMP_FAIL  = AHE_BEGIN + 4, // 设置温度失败
	AHE_CTRL_TIMER_OK  = AHE_BEGIN + 5, // 设置定时成功
	AHE_CTRL_TIMER_FAIL  = AHE_BEGIN + 6, // 设置定时失败
	AHE_END = AHE_BEGIN + 99
};

#define AH_HEAT_MODE_LOW 0x2
#define AH_HEAT_MODE_HIGH 0x3

//暖风机运行状态
typedef struct{
	// true表示电源开
	u_int8_t is_on;
	//  AH_HEAT_MODE_XX 暖风机档位: 低档、高档
	u_int8_t heat_mode; 
	//true表示摇头
	u_int8_t is_shake; 
	//true表示工作在ECO模式
	u_int8_t is_eco;
	//ture表示发热体出于工作状态，正在加热
	u_int8_t is_heating;
	
	//倾倒保护
	u_int8_t is_topple_protect;
	// 环境温度过高保护
	u_int8_t is_temp_high_protect;
	//发热体温度过高保护
	u_int8_t is_furnace_high_protect;
	//暖风机自身故障
	u_int8_t is_furnace_error;

	//设定的温度
	u_int8_t set_temp;
	//环境温度值
	u_int8_t room_temp;
	//发热体1温度值
	u_int8_t thermode_1_temp;
	//发热体2温度值
	u_int8_t thermode_2_temp;	
	//剩余定时时间，单位分钟
	u_int16_t remain_minute;
	//设置下去的小时
	u_int8_t set_hour;
	u_int8_t timer_type;
	//当前消耗电量值
	u_int32_t epower;		
}cl_ah_work_stat_t;

     
    
//联创暖风机信息
typedef struct _cl_ah_info_s{
	cl_handle_t handle; //设备句柄
	cl_ah_work_stat_t ah_work_stat;
}cl_ah_info_t;


/*
 功能:
    控制电源开关
 输入参数:
    @is_on,  true:ON  false:OFF
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_ah_ctrl_power(cl_handle_t dev_handle, u_int8_t is_on);
    
/*
 功能:
    控制加热档位
 输入参数:
    @mode: AH_HEAT_MODE_LOW AH_HEAT_MODE_HIGH
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_ah_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
 功能:
    控制温度
 输入参数:
    @is_add, true: 增加温度 false: 减少温度
 输出参数:
 无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_ah_ctrl_temp(cl_handle_t dev_handle, u_int8_t is_add);
    
/*
功能:
    控制定时器
输入参数:
    @is_add: true: 增加定时器 false: 减少定时器
输出参数:
    无
返回:
 RS_OK: 成功
其他: 失败
*/
CLIB_API RS cl_sa_ah_ctrl_timer(cl_handle_t dev_handle, u_int8_t is_add);

/*
 功能:
    控制ECO
 输入参数:
    @is_eco: true: 开启ECO模式 false: 关闭ECO模式
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_ah_ctrl_eco(cl_handle_t dev_handle, u_int8_t is_eco);

/*
 功能:
    控制摇头
 输入参数:
    @@is_shake: true: 开启摇头 false: 关闭摇头
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_ah_ctrl_shake(cl_handle_t dev_handle, u_int8_t is_shake);

CLIB_API void cl_sa_ah_refresh_power_and_timer(cl_handle_t dev_handle);
    
/*
 功能:
    联创认证用接口
    用于定时发送时间增加指令
 输入参数:
    @@timeout: 超时时间 0：停止 1-10 表示100-1000毫秒时间间隔
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_sa_ah_ctrl_test(cl_handle_t dev_handle, u_int8_t timeout);

    
#ifdef __cplusplus
}
#endif 

#endif

