/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zhcl.h
**  File:    cl_zhcl.h
**  Author:  liubenlong
**  Date:    04/19/2016
**
**  Purpose:
**    智皇窗帘.
**************************************************************************/


#ifndef CL_ZHCL_H
#define CL_ZHCL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */
//status
enum{
	ZHCL_STATUS_OPEN = 0,
	ZHCL_STATUS_CLOSE = 1,
	ZHCL_STATUS_STOP = 3,
};

//type
enum{
	//窗帘
	ZHCL_INDEX_1 = 0,
	//窗纱
	ZHCL_INDEX_2 = 1,
};

enum{
	//左右开合
	ZHCL_TYPE_1 = 0,
	//上下开合
	ZHCL_TYPE_2 = 1,
};

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
typedef struct {
	u_int32_t magic;//分组类型
	u_int8_t index;//0窗帘，1窗纱
	u_int8_t status;//Open = 0 :开启  STOP =1：停止  CLOSE = 3：关闭
	u_int8_t percent;//窗帘位置百分百	
	u_int8_t type;//窗帘类型，0横着拉，1竖着拉
	u_int8_t support_dir;//是否支持转向,0不支持，1表示支持
	u_int8_t dir;//0表示没换向，1表示已换向
}cl_zhcl_info_t;


//////////////////////////////////////////////////////////////////////////////////
//// 智皇窗帘API
/*
	 功能:  智皇窗帘，窗帘状态设置
 
	 输入参数:
	 	status:Open = 0 :开启  STOP =1：停止  CLOSE = 3：关闭
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhcl_status_set(cl_handle_t dev_handle, u_int8_t status);

/*
	 功能:  智皇窗帘，窗帘位置设置
 
	 输入参数:
	 	location: 0~100,就是个百分百
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhcl_location_set(cl_handle_t dev_handle, u_int8_t location);

/*
	 功能:  智皇窗帘，窗帘绑定
 
	 输入参数:
	 	magic:xxx
	 	index:xxx
	 	type:1/2
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhcl_bind(cl_handle_t dev_handle, u_int32_t magic, u_int8_t index, u_int8_t type);

/*
	 功能:  智皇窗帘，窗帘类型设置
 
	 输入参数:
	 	type: 0/1
	 	index:xxx
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhcl_type_set(cl_handle_t dev_handle, u_int8_t type, u_int8_t index);

/*
	 功能:  智皇窗帘，换向设置
 
	 输入参数:
	 	dir: 0表示没有换向，1表示已换向
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhcl_dir_set(cl_handle_t dev_handle, u_int8_t dir);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZHCL_H */

