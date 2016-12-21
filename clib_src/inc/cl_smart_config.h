#ifndef	__CL_SMART_CONFIG_H__
#define	__CL_SMART_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

/*
	功能：
		开始一键配置。
	输入参数:
		@ssid: 手机连接的网关的ssid
		@passwd: 无线密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_smart_config_start(const char *ssid, const char *passwd);

/*
	功能：
		开始一键配置，混合配置方式，先组播，后广播啥的。
	输入参数:
		@ssid: 手机连接的网关的ssid
		@passwd: 无线密码
		@m_time:组播时间，0-255 s 注意是秒
		@m_i_time:组播发包完后多久发送广播,0-255s
		@b_time:广播时间,0-255 s 注意是秒
		@b_i_time:广播发送完后多久发送组播,0-255 s 
		注:m_time与b_time不能同时为0
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_smart_config_start_ext(const char *ssid, const char *passwd, 
	u_int8_t m_time, u_int8_t m_i_time, u_int8_t b_time, u_int8_t b_i_time);
    
/*
 功能：
    开始6621一键配置。
 输入参数:
    @ssid: 手机连接的网关的ssid
    @passwd: 无线密码
 输出参数:
    无
 返回：
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_smart_6621_config_start(const char *ssid, const char *passwd);

/*
 功能：
    开始组播一键配置
 输入参数:
    @ssid: 手机连接的网关的ssid
    @passwd: 无线密码
 输出参数:
    无
 返回：
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_smart_mbroadcast_config_start(const char *ssid, const char *passwd);


/*
 功能：
    开始组播一键配置，热点模式组播发送
 输入参数:
    @ssid: 手机连接的网关的ssid
    @passwd: 无线密码
 输出参数:
    无
 返回：
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_smart_mbroadcast_config_start_hotspot(const char *ssid, const char *passwd);

/*
	功能：
		结束一键配置。
	输入参数:
		无
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_smart_config_stop( );

/*
 功能：
    高级一键配置接口
 输入参数:
    @ssid: 手机连接的网关的ssid
    @passwd: 无线密码
    @mode:方案，1-12, 1-6表示组播，7-12表广播，对应每个方案递增5ms，即组播，广播的发送时间间隔范围是5-30ms
 输出参数:
    无
 返回：
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_advance_smart_config_start(const char *ssid, const char *passwd, u_int8_t mode);


#ifdef __cplusplus
}
#endif 


#endif


