/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zhdhx.h
**  File:    cl_zhdhx.h
**  Author:  liubenlong
**  Date:    10/17/2016
**
**  Purpose:
**    智皇单火线.
**************************************************************************/


#ifndef CL_ZHDHX_H
#define CL_ZHDHX_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */
#define 	ZHDHX_NAME_MAX		(64)
#define 	ZHDHX_KEY_NAME_MAX 	(32)

typedef struct {
	u_int8_t valid;//是否有效，当为1时name才有效
	u_int8_t name[ZHDHX_NAME_MAX];
} cl_zhdhx_key_name_t;

typedef struct {
	u_int32_t dhx_type;//单火线类型，值是多少就是多少路
	u_int32_t on_off_stat; //各路开关状态，第0bit表示第一路,0表示关，1表示开
	cl_zhdhx_key_name_t key_name[ZHDHX_KEY_NAME_MAX];
}cl_zhdhx_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	 功能:  智皇单火线开关设置
 
	 输入参数:
	 	@dev_handle,设备handle
	 	@num:0-ff, 注意，0表示全部，比如num=0,onoff=1,表示全开，0,0表示全关。
		@onoff:0表示关，1表示开。
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdhx_onoff(cl_handle_t dev_handle, u_int8_t num, u_int8_t onoff);

/*
	 功能:  智皇单火线各路名称设置
 
	 输入参数:
	 	@dev_handle,设备handle
	 	@num:1-ff, 表示第几路
		@name:/0结束的字符串，长度最长63.
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdhx_key_name_set(cl_handle_t dev_handle, u_int8_t num, char *name);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZHDHX_H */

