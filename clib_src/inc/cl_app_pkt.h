/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: APP上层二次开发通道
**  File:    cl_app_pkt.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    07/26/2016
**
**  Purpose:
**    APP上层二次开发通道.
**************************************************************************/


#ifndef CL_APP_PKT_H
#define CL_APP_PKT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "client_lib.h"
#include "cl_app_pkt.h"

/* Macro constant definitions. */


/* Type definitions. */

typedef enum {
	pt_server = 1,	// 服务器
	pt_app,
	pt_all_online_app,
	pt_all_app,
	pt_wifi_dev,
	pt_macbee_dev,			// macbee设备发来的数据
	pt_macbee_dev_cache,	// macbee缓存在网关的数据，自动获取
} PEER_TYPE_T;

typedef enum {
	PI_DETAIL_TYPE = 1,	// 从设备的具体类型，2字节
} PEER_INFO_T;

typedef struct {
	PEER_TYPE_T type;
	u_int64_t ident;
	int pkt_len;
	u_int8_t pkt[0];
} cl_app_pkt_send_t;

typedef void (*cl_app_proc_fun)(PEER_TYPE_T peer_type, u_int64_t ident, char* pkt , int pkt_len);

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	功能：
		发送一个数据报文
	输入参数:
		@peer_type:发送目的地类型
		@ident: 对端识别标志，如果发送给设备，这里填SN
		@pkt: 发送的数据
		@pkt_len: 发送的数据长度
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_send_pkt(PEER_TYPE_T peer_type, u_int64_t ident, char* pkt , int pkt_len);

/*
	功能：
		设置一个接收数据回调的接口
	输入参数:
		@fun: 回调
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_set_proc_pkt_callback(cl_app_proc_fun fun);

/*
	功能：
		设置开发者ID
	输入参数:
		@id: 开发者ID
	输出参数:
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_set_developer_id(char *id);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_APP_PKT_H */

