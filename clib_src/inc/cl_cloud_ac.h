#ifndef	__CL_CLOUD_AC_H__
#define	__CL_CLOUD_AC_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"


//event
enum {
	CA_BEGIN = 1600,

	/*准备就绪，等待红外编码*/
	CA_WAIT_IR,
	/*收到红外编码，等待服务器反馈*/
	CA_WAIT_MATCH,
	/*服务器匹配成功，正在下载编码*/
	CA_WAIT_CODE,
	/*下载编码成功，可以控制了*/
	CA_DONE,
	
	/*取消匹配成功*/
	CA_CM_CANNEL,

	/*云匹配失败或还未进行云匹配*/
	CA_NOT_MATCH,
	/*设置成功*/
	CA_SET_OK,
	/* 云空调匹配失败，或者控制失败 */
	CA_ERROR,

	CA_END = CA_BEGIN + 99
};

typedef struct{
	u_int8_t action;
	u_int8_t match_type;
	u_int16_t select_match_id;
	cl_handle_t ac_handle;
}cln_cloud_match_t;

typedef struct{
	u_int32_t key_id;
	cl_handle_t ac_handle;
}cln_ac_ctrl_t;

typedef struct{
	u_int8_t onoff:1;//开关
	u_int8_t mode:3;//模式
	u_int8_t temp:4;//温度
	u_int8_t speed:2;//风速
	u_int8_t dir:2;//风向
	u_int8_t key_v:3;//按键值
	u_int8_t oldkey_v:4;//老式空调按键值
	
}cln_ac_key_id_t;



/*
	功能:
		云空调云匹配
	输入参数:
		@ac_handle:  设备句柄
		@do_match:  1,进行云匹配，0，取消云匹配
		@cloud_dev_id:  云电器ID
		@match_type:  云匹配类型
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_cloud_match(cl_handle_t ac_handle,bool do_match, u_int8_t match_type);

/*
	功能:
		发送云空调控制
	输入参数:
		@ac_handle:  设备句柄
		@onoff:  1,开0, 关
		@temp:  温度,  ＝ 实际温度 - AC_TEMP_BASE
		@mode: 模式, AC_MODE_XXX
		@speed: 风速, AC_WIND_xxx
		@dir: 风向, AC_DIR_xxx
		@presskey 新款空调按下的键, AC_KEY_xxx
		@oldkey:  老式遥控板按下的键，若为老式其他填0。
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_ac_send_ctl(cl_handle_t ac_handle,u_int8_t onoff, u_int8_t temp, u_int8_t mode, u_int8_t speed, u_int8_t dir, u_int8_t presskey, u_int8_t oldkey);


/*
	功能:
		设置云空调匹编码库ID
	输入参数:
		@ac_handle:  设备句柄
		@select_match_id:  选择ID的下标
		@match_type:  云匹配类型
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

CLIB_API RS cl_ac_set_match_id(cl_handle_t ac_handle, u_int16_t select_match_id, u_int8_t match_type);


#ifdef __cplusplus
}
#endif 

#endif



