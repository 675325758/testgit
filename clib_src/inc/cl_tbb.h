/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_tbb.h
**  File:    cl_tbb.h
**  Author:  liubenlong
**  Date:    08/10/2015
**
**  Purpose:
**    拓邦商用.
**************************************************************************/


#ifndef CL_TBB_H
#define CL_TBB_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/*
support_mode:
	Bit0：自动    （0:无   1:有）
	Bit1：制冷    （0:无   1:有）
	Bit2：制热    （0:无   1:有）
	Bit3：供暖    （0:无   1:有）
*/
/*
pump_info:
	Bit0:1#压缩机。
	Bit1:2#压缩机。
	Bit2::3#压缩机。
	Bit3::4#压缩机。
	Bit4:电加热。
	Bit5:底盘电加热。
	Bit6:曲轴电加热 。
	Bit7:水泵。
	Bit8:直流变频水泵。
	Bit9:高风。
	Bit10:低风 。
	Bit11:1#四通阀。
	Bit12:2#四通阀。
	Bit13:3#四通阀。
	Bit14:4#四通阀。 
	Bit15:循环阀。
*/
/*
setup_on:
都是（0:无/1:有）。
	Bit0:1#高压开关。
	Bit1:1#低压开关。
	Bit2:2#高压开关。
	Bit3:2#低压开关。
	Bit4:3#高压开关。
	Bit5:3#低压开关。
	Bit6:4#高压开关。
	Bit7:4#低压开关。
	Bit8:水流开关。
	Bit9:应急开关。
	Bit10:相序开关。
	Bit11:空调需求信号开关。
*/
/*
sys_select:
   默认 1
   0 单冷
   1 热泵
   2 电热
   3 热水
*/

/*
tmp:
	制冷温度设定值(10~25)
	制热温度设定值 (20~60)
	自动温度设定值 (8~40)	    
*/
//需要对齐的
#pragma pack(push, 1)
//硬件信息
/*
sys_type:
	系统机型
	0x01         0 为 家用机系列    1 家用机1型
	0x31         3 为 泳池机系列    1 泳池机1型
	0x81         8 为 商用机系列    1 商用机1型
ele_band_mcu:
电控板MCU型号
	0x1051    105为STM8S105    1 为 32K
	0x2071    207为STM8S207    1 为 32K
	0x2072                     2 为 64K
	0x2073                     3 为 128K
ele_band_ver:
	电控板固件版本号默认0x10 即固件版本号为V1.0
line_band_mcu:
	线控器MCU型号
	0x1051    105为STM8S105    1 为 32K
	0x2071    207为STM8S207    1 为 32K
	0x2072                     2 为 64K
	0x2073                     3 为 128K
*/
/*
set_on1:
	设备安装状态 1：
	Bit0：循环水泵
	Bit1：循环阀
	Bit2：冲洗阀
	Bit3：电加热
	Bit4: 低风开关
	Bit5: 直流变频水泵
	Bit6: 底盘电加热
	Bit7：曲轴电加热
	Bit8：进水温度传感器
	Bit9：出水温度传感器
	Bit10：回水温度传感器
	Bit11：水箱温度传感器
	Bit12：环境温度传感器
	Bit13~ Bit15：保留
*/

/*
set_on2:
	设备安装状态 2：
	Bit0：1#电子膨胀阀
	Bit1：1#盘管温度
	Bit2：1#排气温度
	Bit3：1#回气温度
	Bit4：2#电子膨胀阀
	Bit5：2#盘管温度
	Bit6：2#排气温度
	Bit6：2#回气温度
	Bit8：3#电子膨胀阀
	Bit9：3#盘管温度
	Bit10：3#排气温度
	Bit11：3#回气温度
	Bit12：4#电子膨胀阀
	Bit13：4#盘管温度
	Bit14：4#排气温度
	Bit15：4#回气温度
*/

typedef struct {
	u_int16_t sys_type;//单片机机型
	u_int16_t ele_band_mcu;//电控板mcu型号
	u_int16_t ele_band_ver;//电控板版本号
	u_int16_t line_band_mcu;//线控版mcu型号
	u_int16_t line_band_ver;//线控板版本号
	u_int16_t pad;
}cl_tbb_hd_ver_t;
//除霜参数设置
typedef struct {
	u_int16_t in_defrost_time;//制热进入除霜时间周期(10~90min)
	u_int16_t in_defrost_tmp;//制热进入除霜温度( -30~0℃)
	u_int16_t out_defrost_tmp;// 制热退出除霜温度条件(2~30℃)
	u_int16_t out_defrost_time;//制热退出除霜时间条件(1~12min)
	u_int16_t env_tmp;//除霜环境温度条件
	u_int16_t dif;//除霜盘管温度和环境温度差值
	u_int16_t defrost_continue_time;//除霜条件持续时间 
	u_int16_t pad;
}cl_tbb_defrost_t;

typedef struct {
	u_int16_t dst_tmp_pro;//目标过热度( -15~15℃)
	u_int16_t cool_out_water;//制冷出水过冷保护值(3~20℃)
	u_int16_t heat_out_water;//制热出水过热保护值(20~90℃)
	u_int16_t in_out_tmp;//进出水温差过大值(5~20℃)
	u_int16_t pump_begin_time;//水泵提前启动时间(5~99 s)
	u_int16_t pump_delay_time;//水泵延时关闭时间(5~99 s)
	u_int16_t wind_ordor_tmp;//风速转换温度(5~40℃)
	u_int16_t env_tmp;//环境温度防冻值(0~15℃)
	u_int16_t in_water_tmp;//进水温度防冻值(2~14℃)
	u_int16_t pad;
}cl_tbb_protect_t;
/*
work:
	BIT1：有无掉电保护 （0（无）~1（有））。
	Bit3：水泵工作方式（0：普通，1：特殊）。
	Bit8：电子膨胀阀调节方式（0：手动，1：自动）。
	Bit9：四通阀正反向（0（正向）~1（反向））。
	Bit10：摄氏度/华氏度转换 （0：摄氏度， 1：华氏度） 默认0
	Bit15：恢复出厂默认值（0：关，1：开）。
*/
typedef struct {
	u_int8_t sys_num;//系统数量(1-4)
	u_int8_t sys_select;//机型选择
	u_int16_t work;
	u_int16_t heat_pump_diff;//热泵重启回差值(1~20℃)
	u_int16_t cool_diff;//制冷回差温度
	u_int16_t hot_diff;//供暖回差温度
	u_int16_t pad;
}cl_tbb_misc_t;

typedef struct {
	u_int16_t ele_cycle;//电子膨胀阀调节周期数
	u_int16_t hand_ctrl_step;//手动调节电子膨胀阀步数 (10~50)
	u_int16_t cool_ele_valve;//制冷电子膨胀阀开度(10~50)
	u_int16_t limit_day;//限时天数(0~999)
}cl_tbb_eev_t;

typedef struct {
	u_int16_t cool_tmp;//制冷回水温度设定值 
	u_int16_t heat_tmp;//制热回水温度设定值   
	u_int16_t auto_tmp;// 自动回水温度设定值    
	u_int16_t pad;
}cl_tbb_tmp_t;

typedef struct {
	cl_tbb_defrost_t defrost;//除霜设置
	cl_tbb_misc_t misc;
	cl_tbb_protect_t protect;//保护参数
	cl_tbb_eev_t eev;
	cl_tbb_tmp_t auto_tmp;//
	u_int16_t bottom_ele_heat_tmp;//底盘电加热启动温度(0~20℃)
	u_int16_t pad;
}cl_tbb_config_set_t;
//商用华天成
typedef struct {
	u_int16_t scroll_tmp;//盘管温度
	u_int16_t out_air_tmp;//排气温度
	u_int16_t back_air_tmp;//回气温度
}cl_tbb_scroll_t;

typedef struct {
	u_int16_t water_box_tmp;//水箱温度（ -300~990(0.1℃)）
	u_int16_t in_water_tmp;//进水温度（ -300~990(0.1℃)）
	u_int16_t out_water_tmp;//出水温度（ -300~990(0.1℃)）
	u_int16_t env_tmp;//环境温度（ -300~990(0.1℃)）
	u_int16_t back_water_tmp;//回水温度（ -300~990(0.1℃)）
	u_int16_t support_mode;//支持模式
	cl_tbb_scroll_t scroll_tmp[4];
	u_int16_t ele_valve[4];//电子膨胀阀开度,电子膨胀阀开度, 电子膨胀阀开度, 电子膨胀阀开度
	u_int16_t pump_info;//热泵工作详情(
	u_int16_t set_on;//各安装开关
	u_int16_t set_on1;//安装1
	u_int16_t set_on2;//安装2
	u_int16_t slave_status;
	u_int16_t fault1;// 30033
	u_int16_t fault2;// 30034
	u_int16_t fault3;// 30035
	u_int16_t sys_run_days;
	u_int16_t pad16;
	u_int32_t pad32;
}cl_tbb_status_t;

typedef struct{
	/* 设备状态 */  
	u_int16_t dev_state;   
	/* 绑定状态 */  
	u_int16_t  bind_state;  
	/* 拓邦SN，17字符 */ 
	u_int8_t tb_sn[24]; 
	u_int8_t reserved[8];
}cl_tbb_bindinfo_t;

/*
upgrade_state:
//升级错误
#define TBB_ERR_NONE   (0)
	//升级成功 
#define TBB_ERR_OK    (127)
	未联网
#define TBB_ERR_NET   (128)
	镜像下载失败
#define TBB_ERR_DOWN  (129)
	镜像校验失败
#define TBB_ERR_IMG   (130)
	以上的错误可以重试, 下面的错误联系厂商处理
	升级线控失败
#define TBB_ERR_MCU0  (131)
	升级电控板失败
#define TBB_ERR_MCU1  (132)
*/
typedef struct {
    u_int32_t upgradeing;//升级中
    u_int32_t upgrade_role;//升级角色，2=>mcu0,3=>mcu1
    u_int32_t upgrade_state;//升级状态
    u_int32_t up_state;//升级百分比
}cl_stm_upgrade_info_t;

#pragma pack(pop)

typedef struct {
	//只显示区域
	u_int8_t on;
	u_int8_t mode;//模式根据support_mode 设置
	u_int16_t tmp;//设置温度

	cl_stm_upgrade_info_t upgrade_info;//升级信息
	cl_tbb_status_t status;
	cl_tbb_hd_ver_t hd_ver;//单片机硬件信息，有些根据硬件信息显示
	cl_tbb_bindinfo_t bindinfo;//绑定信息
	//可设置区域
	cl_tbb_config_set_t config;
}cl_tbb_info_t;

/* Type definitions. */


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
CLIB_API RS cl_tbb_ctrl_onoff(cl_handle_t dev_handle, u_int8_t onoff);
/*
	功能:
		模式设置
	输入参数:
		@dev_handle: 设备的句柄
		@mode:(1-4)
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tbb_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
	功能:
		温度设置
	输入参数:
		@dev_handle: 设备的句柄
		@tmp:看情况 
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tbb_ctrl_tmp(cl_handle_t dev_handle, u_int16_t tmp);


/*
	功能:
		除霜设置
	输入参数:
		@dev_handle: 设备的句柄
		@pconfig:配置参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tbb_config(cl_handle_t dev_handle, cl_tbb_config_set_t *pconfig);

/*
	功能:
		sn绑定
	输入参数:
		@dev_handle: 设备的句柄
		@bar_code:sn字符串
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tbb_bind_dev_bar_code(cl_handle_t dev_handle,u_int8_t* bar_code);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_TBB_H */

