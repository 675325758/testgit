/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: hxpbj
**  File:    cl_hxpbj.h
**  Author:  liubenlong
**  Date:    05/06/2015
**
**  Purpose:
**    海迅破壁机头.
**************************************************************************/


#ifndef CL_HXPBJ_H
#define CL_HXPBJ_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
#define HX_DIY_NAME_MAX		13
#define CL_HXYSH_CUSTOMIZE_SCENE_MIN_ID 101 // 自定义情景起始ID号
#define CL_HXYSH_BOILING_TIME 255 //煮沸需要的时间
    
enum{
	HX_MODE_GZ = 0,	//果汁模式
	HX_MODE_MH,		//米糊
	HX_MODE_SB,		//沙冰
	HX_MODE_MZ,		//绵粥
	HX_MODE_ZT,		//中汤
	HX_MODE_DJ,		//豆浆
	HX_MODE_XT,		//西汤
	HX_MODE_FR,		//翻热 
	HX_MODE_DIY,	//DIY
	HX_MODE_STOP,	//STOP
    HX_MODE_DIY1,   //DIY1
    HX_MODE_DIY2,
    HX_MODE_DIY3,
    HX_MODE_DIY4,
    HX_MODE_DIY5,
    HX_MODE_DIY6,
    HX_MODE_DIY7,
    HX_MODE_DIY8,
    HX_MODE_DIY9,
	HX_MODE_MAX
};

/* Type definitions. */
typedef struct {
	//工作模式
	u_int8_t cur_mode;
	//当前温度
	u_int8_t cur_tmp;
	//当前转速
	u_int8_t cur_speed;
	//发送完成报告
	u_int8_t cur_send_finsh;
    //暂停功能
    u_int8_t cur_pause;
    //工作时间
    u_int32_t work_time;
	//发送机故障
	u_int8_t cur_send_err;
	//空闲状态
	u_int8_t idle_status;
	//保温温度
	u_int8_t keep_warm;
	//自定义名称
	char name[HX_MODE_MAX][HX_DIY_NAME_MAX];
}cl_hx_info;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	功能:
		控制模式
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_hx_ctrl_work_mode(cl_handle_t dev_handle, u_int8_t work_mode);


/*
	功能:
		自定义名称修改函数
	输入参数:
		@dev_handle: 设备的句柄
		@id:自定义id
		@name:以/0结束的name字符串
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_hx_ctrl_diy_name(cl_handle_t dev_handle, u_int8_t id, char *name);


/*
	功能:
		清除finish状态
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_hx_finish_clear(cl_handle_t dev_handle);
    

////////////////////////////////////////////////////////////////////
//海迅养生壶

/*
 0x00:正常状态
 0x01提壶操作或温度探头开路
 0x02温度探头短路操作
 0x03温度探头高温操作
 */
enum{
    HX_YSH_ERR_NONE = 0x0,
    HX_YSH_ERR_NO_POT,
    HX_YSH_ERR_TEMP_DETECT,
    HX_YSH_ERR_HIGH_TEMP
};

//情景操作
enum {
    //删除
    HXYSH_CTRL_SCENE_ACTION_DELETE = 0X01,
    //停止
    HXYSH_CTRL_SCENE_ACTION_STOP = 0X02,
};

//work_state
enum {
    HXYSH_WORK_STATE_STANDBY = 0X00,     //待机
    
    HXYSH_WORK_STATE_HEATING = 0X01,   //加热中
    HXYSH_WORK_STATE_HEAT_DONE = 0X02, //加热结束
    
    HXYSH_WORK_STATE_KEEPING = 0X03,     //保温中
    HXYSH_WORK_STATE_KEEP_DONE = 0X04//保温结束
};
    
// timer 换算因子
enum {
    HXYSH_TIMEFACTOR_MCU_TIMER = 2,         //2倍
    HXYSH_TIMEFACTOR_WORK_REMAIN_TIME = 5,   //5倍
    HXYSH_TIMEFACTOR_HEAT_TIME = 5,   //5倍
    HXYSH_TIMEFACTOR_KEEP_TIME = 5   //5倍
};
    
typedef struct {
    u_int8_t is_data_valid; //数据是否有效
    u_int8_t on_off; //开关
    u_int16_t cur_exec_id; //当前执行的情景，如果是待机，且预约时间不为0，则改id，表示预约的情景id
    u_int8_t work_stat; // 0:待机: 1. 制作中 2:制作完成
    u_int8_t work_remain_time; //如果当前在制作中，该时间表示制作完成的倒计时
    u_int8_t cur_power; //当前功率 1-8 单位 100瓦
    u_int8_t temp; //温度
    u_int8_t mcu_timer; //如果有预约，该时间表示倒计时执行
    u_int8_t is_hot; //是否在加热中
    u_int8_t err_no; //错误状态
    u_int16_t wifi_timer_exec_id;
	u_int16_t wifi_timer;
}cl_hx_ysh_stat_info_t;

typedef struct {
    u_int8_t temp;// 温度 35-100
    u_int8_t time; // 加热时间 单位 10分钟
    u_int8_t power; //加热功率 1-8 （表示100-800瓦）
    u_int8_t keep_temp; //保温温度 30-90
    u_int8_t keep_time; //保温时间
    u_int8_t pad;
    u_int16_t scene_id; //情景id;
    u_int32_t create_time;
    char name[32];
}cl_hx_ysh_scene_t;

typedef struct{
    cl_hx_ysh_stat_info_t stat;
    u_int32_t scene_num;
    cl_hx_ysh_scene_t* scene;
}cl_hx_ysh_info_t;

/*
 功能:
    控制养生壶启动
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hx_ysh_ctrl_onoff(cl_handle_t dev_handle, u_int8_t on_off);

/*
 功能:
    控制养生壶执行或删除情景
 输入参数:
    @dev_handle: 设备的句柄
    @action : 1：删除 : 2:停止
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hx_ysh_ctrl_scene(cl_handle_t dev_handle, u_int8_t aciton, u_int16_t scene_id);

    
/*
 功能:
    控制养生壶执行或删除情景
 输入参数:
    @dev_handle: 设备的句柄
    @action : 1：删除 : 2:停止
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hx_ysh_exec_scene(cl_handle_t dev_handle, cl_hx_ysh_scene_t* scene);
/*
 功能:
    控制养生壶修改情景
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hx_ysh_modify_scene(cl_handle_t dev_handle, cl_hx_ysh_scene_t* scene);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CL_HXPBJ_H */

