#ifndef	__CL_SERVER_H__
#define	__CL_SERVER_H__


#ifdef __cplusplus
extern "C" {
#endif 


#include "client_lib.h"

enum{
	UASC_BEGIN = 2200,
	UASC_START_CONNECT = 2201,
	UASC_CONNECT_OK = 2202,
	UASC_PUSH_STAT_OK = 2203
};

enum{
    APP_HARDWARE_TYPE_IPHONE = 0x00,
    APP_HARDWARE_TYPE_IPAD = 0x01,
    APP_HARDWARE_TYPE_IPOD = 0x02,
    APP_HARDWARE_TYPE_SIMULATOR = 0x03,
};
    
typedef struct{
	u_int8_t hard_ware_type; // 0x0:手机0x1:平板
	char manufacturer_info[64]; //生产厂商信息,如手机生产厂家，型号
	char os_info[64]; //操作系统版本信息
	char app_version_info[32];//app版本信息
}cl_app_stat_info_t;

typedef struct{
	u_int8_t is_establish; //是否建立连接
	// SDK 调试用
	u_int8_t cur_stat; 
	u_int32_t server_ip;
	u_int16_t server_port;
}cl_app_server_connect_info_t;

/*******************************************

	module: server
	level: private
		
********************************************/

/*
	功能：
		设置分配服务器的地址。
		不调用本函数，缺省是 www.jiazhang008.com
	参数IN：
		server：可以是IP地址的字符串，也可以是域名。
	返回：
		RS_OK: 成功
		其他：失败
*/
CLIB_API RS cl_disp_server_set(const char *server);
/*
	功能：
		获取分配服务器的IP地址
	参数IN：
		ip: 地址数组的首地址
		count: ip数组个数
	参数OUT：
		ip: 存放解析出来的IP地址列表
		count: 解析出来的ip地址个数
	返回：
		RS_OK: 成功
		其他：失败
*/
CLIB_API RS cl_disp_server_get_ip(u_int32_t *ip, u_int32_t *count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/********************************************

应用服务器相关接口函数



*********************************************/
/*
	功能：
		启动连接APP_服务器的任务
	参数：
		
	返回：
		RS_OK: 成功
		其他：失败
*/
CLIB_API RS cl_start_connect_to_app_server();

/*
	功能：
		查询APP 服务器状态信息
	参数：
		
	返回：
		RS_OK: 成功
		其他：失败
*/
CLIB_API RS cl_get_app_server_info(cl_app_server_connect_info_t* info);

/*
	功能：
		发送APP 状态信息
	参数：
		
	返回：
		RS_OK: 成功
		其他：失败
*/
CLIB_API RS cl_push_app_stat_info(cl_app_stat_info_t* stat);


#ifdef __cplusplus
}
#endif 

#endif

