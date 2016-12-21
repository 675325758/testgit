/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_xy.h
**  File:    cl_xy.h
**  Author:  liubenlong
**  Date:    07/16/2015
**
**  Purpose:
**    鑫源项目外部头文件.
**************************************************************************/


#ifndef CL_XY_H
#define CL_XY_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
#define XY_SMART_WEEK_MAX			7	//一周7天
#define XY_SMART_WEEK_TIMEPOINT		48	//一天分成48个时间点，半小时一个点

#define XY_SMART_DAY_TIMEPOINT_MAX	6	//一天可以分为最多6个时间段

#define XY_SMARTHOME_SUB_MODE_NUM	3	//智能模式子模式个数

//主动丢包
#define XY_DROP_UPDATE_NUM			1

/* Type definitions. */
//mode
enum {
	XY_MODE_CONSTTEMP = 1,//恒温模式
	XY_MODE_SMART = 2,//智能模式
	XY_MODE_HOLIDAY = 3,//休假模式
};

//智能模式下的子模式
enum {
	XY_SSM_ONE_SERVEN = 0X0,// 1*7
	XY_SSM_FIVE_TWO,		// 5+2
	XY_SSM_FIVE_ONE_ONE,	// 5+1+1
};

//开关机
enum {
	XY_STATUS_OFF = 0x0,
	XY_STATUS_ON,
};

//串口设置开关
enum {
	XY_UART_STATUS_ON = 0X1,
	XY_UART_STATUS_OFF,
};

//加热状态
enum {
	XY_HEAT_OFF = 0x0,
	XY_HEAT_ON,
};

//锁定状态
enum {
	XY_LOCK_OFF = 0x0,
	XY_LOCK_ON,
};

//串口锁定设置
enum {
	XY_UART_LOCK_OFF = 0x1,
	XY_UART_LOCK_ON,
};

//探头状态
enum {
	XY_PROBE_OK = 0X0,
	XY_PROBE_ERR,
};


//时间段 
#pragma pack(push, 1)
typedef struct {
	u_int8_t valid;//(0/1)
	u_int8_t start_index;//(0~48)
	u_int8_t end_index;//(0~48)
	u_int8_t temp;
}cl_xy_tp_t;

//mode
typedef struct {
	u_int8_t mode;//XY_SSM_ONE_SERVEN , XY_SSM_FIVE_TWO, XY_SSM_FIVE_ONE_ONE
	u_int8_t pad[3];
}cl_xy_st_mode_t;

typedef struct {
	//智能模式配置参数,两个字段配合取得一周7*48个节点的温度配置
	cl_xy_st_mode_t sub_mode;
	cl_xy_tp_t timepoint[XY_SMARTHOME_SUB_MODE_NUM][XY_SMART_WEEK_MAX][XY_SMART_DAY_TIMEPOINT_MAX];//一周时间段分配配置
}cl_xy_smartmode_info_t;

typedef struct {
	//智能模式配置参数,两个字段配合取得一周7*48个节点的温度配置
	cl_xy_st_mode_t sub_mode;
	//一周时间段分配配置,用一维数组，jni处理方便
	cl_xy_tp_t timepoint[XY_SMARTHOME_SUB_MODE_NUM*XY_SMART_WEEK_MAX*XY_SMART_DAY_TIMEPOINT_MAX];
}cl_xy_smartmode_com_info_t;

#pragma pack(pop)


typedef struct {
	u_int8_t temp_adj;//温度校正值+20，例如温度校正值为-3，则Adj=17。Adj取值范围11~29
	u_int8_t temp_top;//可设定温度上限（5~85）
	u_int8_t temp_tolr;//温控容差(1-9)
	u_int8_t temp_otemp;//过热保护温度（15~85）
	u_int8_t temp_prottrig;//加热器保护触发时间+1（1~100），例如触发时间5小时，则ProtTrig=6
	u_int8_t temp_protlen;//加热器保护时长（10~90）
}cl_xy_adjust_t;

typedef struct {
	//状态参数
	u_int8_t onoff;//0:关闭，1:开启
	u_int8_t mode;// 1:恒温，2:智能，3:休假
	u_int16_t root_temp;//室温，单位0.1℃
	u_int16_t di_temp;//地温，发热体温度
	u_int8_t cur_dst_temp;//当前目标温度
	u_int8_t heat;//0:未加热，1:加热
	u_int8_t lock;//0:未锁定，1:锁定
	u_int8_t err;//0-失败 1-成功 2-超出允许取值范围
	u_int8_t cons_temp;//恒温模式目标温度
	u_int8_t holiday_temp;//休假模式目标温度
	u_int16_t remain_time;//高8位表示小时，低8位表示分钟

	//上次发送命令，用来过滤返回的数据更新
	u_int8_t last_cmd;

	//本地区室外温度
	u_int8_t extern_temp;

	//探头故障
	u_int8_t probe_err;

	//智能回家开关
	u_int8_t smarthome_onoff;

	//后台操作数据
	cl_xy_adjust_t adjust;//校正参数
	
	//配置智能模式参数等。
	cl_xy_smartmode_com_info_t smart_mode;
}cl_xy_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	功能:
		开关
	输入参数:
		@dev_handle: 设备的句柄
		@onoff:开关，1开, 0关
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_onoff(cl_handle_t dev_handle, u_int8_t onoff);

/*
	功能:
		恒温模式下的温度设置
	输入参数:
		@dev_handle: 设备的句柄
		@temp:温度值，负数补码形式
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_temp(cl_handle_t dev_handle, u_int8_t temp);

/*
	功能:
		按键锁定控制
	输入参数:
		@dev_handle: 设备的句柄
		@onoff:开关，1开, 0关
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_lock_onoff(cl_handle_t dev_handle, u_int8_t onoff);

/*
	功能:
		模式设置
	输入参数:
		@dev_handle: 设备的句柄
		@mode:1-恒温 2-智能 3-休假
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
	功能:
		休假模式时间设置设置
	输入参数:
		@dev_handle: 设备的句柄
		@time:99*24小时
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_time(cl_handle_t dev_handle, u_int16_t time);


/*
	功能:
		智能模式参数配置
	输入参数:
		@dev_handle: 设备的句柄
		@st_info:智能模式参数配置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_config_smart_mode(cl_handle_t dev_handle, cl_xy_smartmode_com_info_t *pst_info);

/*
	功能:
		后台校正参数
	输入参数:
		@dev_handle: 设备的句柄
		@st_info:智能模式参数配置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_adjust(cl_handle_t dev_handle, cl_xy_adjust_t *padjust);

/*
	功能:
		外部温度设置
	输入参数:
		@dev_handle: 设备的句柄
		@temp:本地区的室外温度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_extern_temp(cl_handle_t dev_handle, u_int8_t temp);

/*
	功能:
		智能回家开关
	输入参数:
		@dev_handle: 设备的句柄
		@onoff:0关，1开
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_smarthome_onoff(cl_handle_t dev_handle, u_int8_t onoff);

/*
	功能:
		智能子模式设置
	输入参数:
		@dev_handle: 设备的句柄
		@mode:
			//智能模式下的子模式
			enum {
				XY_SSM_ONE_SERVEN = 0X0,// 1*7
				XY_SSM_FIVE_TWO,		// 5+2
				XY_SSM_FIVE_ONE_ONE,	// 5+1+1
			};		
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_xy_ctrl_smarthome_mode(cl_handle_t dev_handle, u_int8_t mode);




#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_XY_H */

