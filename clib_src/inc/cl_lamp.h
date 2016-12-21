#ifndef	__CL_UDP_LAMP_H__
#define	__CL_UDP_LAMP_H__

/*
LP : LAMP
GX: GaoXun
高讯LED调光灯
我们提供6621模块，通信使用udp协议
*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

//event of lamp
enum {
	LPE_BEGIN = 1700,
	LPE_CTRL_OK = LPE_BEGIN + 1, //控制成功，包含电源、摇头、档位、ECO
	LPE_CTRL_FAIL = LPE_BEGIN + 2, //控制失败
	LPE_END = LPE_BEGIN + 99
};

/*
 功能:
    控制电源开关
 输入参数:
    @dev_handle:  设备句柄
    @is_on:  true:ON  false:OFF
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_power(cl_handle_t dev_handle, u_int8_t is_on);

/*
 功能:
    控制全光
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_full_light(cl_handle_t dev_handle);

/*
 功能:
    控制白光
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_white(cl_handle_t dev_handle);

/*
 功能:
    控制暖光
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_warm(cl_handle_t dev_handle);

/*
 功能:
    控制混光
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_mix(cl_handle_t dev_handle);

/*
 功能:
    控制亮度
 输入参数:
    @dev_handle:  设备句柄
    @brightness:  亮度值，取值范围1-100
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_brightness(cl_handle_t dev_handle, u_int8_t brightness);

/*
 功能:
    控制暖光
 输入参数:
    @dev_handle:  设备句柄
    @warmness:  暖光值，取值范围1-100
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_warmness(cl_handle_t dev_handle, u_int8_t warmness);

/*
 功能:
    控制红绿蓝颜色
 输入参数:
    @dev_handle:  设备句柄
    @r: 红色，取值范围0x00-0xFF
    @g: 绿色，取值范围0x00-0xFF
    @b: 蓝色，取值范围0x00-0xFF
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_rgb(cl_handle_t dev_handle, u_int8_t r, u_int8_t g, u_int8_t b);

/*
 功能:
    控制自动模式
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_auto(cl_handle_t dev_handle);

/*
 功能:
    控制节能模式
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_energy_saving(cl_handle_t dev_handle);

/*
 功能:
    控制夜灯
 输入参数:
    @dev_handle:  设备句柄
    @is_on: true:ON  false:OFF
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_night(cl_handle_t dev_handle, u_int8_t is_on);

/*
 功能:
    控制模式1
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_model1(cl_handle_t dev_handle);

/*
 功能:
    控制模式2
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_ctrl_model2(cl_handle_t dev_handle);

/*
 功能:
    查询所有工作状态
 输入参数:
    @dev_handle:  设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_lamp_query_all(cl_handle_t dev_handle);

#ifdef __cplusplus
}
#endif 

#endif

