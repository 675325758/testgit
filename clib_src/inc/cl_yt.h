/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_yt.h
**  File:    cl_yt.h
**  Author:  liubenlong
**  Date:    08/24/2015
**
**  Purpose:
**    月兔.
**************************************************************************/


#ifndef CL_YT_H
#define CL_YT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
//onoff
#define YT_ON	0
#define YT_OFF	1

#define YT_TMP_BASE		16

//mode
enum {
	YT_MODE_AUTO = 0,//自动
	YT_MODE_COOL,	//制冷
	YT_MODE_AREFACTION,//除湿
	YT_MODE_WIND,	//送风
	YT_MODE_HOT,
	YT_MODE_MAX,
};

enum {
	YT_WIND_SPEED_AUTO = 0,//自动风速
	YT_WIND_SPEED_ONE,	//风速一
	YT_WIND_SPEED_TWO,	//风速二
	YT_WIND_SPEED_THREE,	//风速三
	YT_WIND_SPEED_MAX,
};

enum {
	YT_WIND_DIR_AUTO = 0,//风向自动
	YT_WIND_DIR_ONE,	//风向一
	YT_WIND_DIR_TWO,	//风向二
	YT_WIND_DIR_THREE,	//风向三 
	YT_WIND_DIR_FOUR,	//风向四
	YT_WIND_DIR_FIVE,	//风向五
	//YT_WIND_DIR_UPDOWN,	//上下摆风
	//YT_WIND_DIR_LEFTRIGHT,//左右摆风
	YT_WIND_DIR_MAX,
};

typedef enum {
	YT_ACTION_ONOFF = 0,//开关
	YT_ACTION_MODE,	//模式
	YT_ACTION_TMP,	//温度
	YT_ACTION_WINDSPEED,//风量
	YT_ACTION_WINDDIR,//风向
	YT_ACTION_ELEASSIST,//电辅
	YT_ACTION_SLEEP,//睡眠
	YT_ACTION_SWING,//摆风
	YT_ACTION_MAX,
}YT_ACTION_T;

#define YT_TMP_MAX	31
#define YT_TMP_MIN	16
/* Type definitions. */
#pragma pack(push, 1)
typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t tmp;
	u_int8_t wind_speed;
	u_int8_t wind_dir;
	u_int8_t ele_assist;
	u_int8_t sleep;
	u_int8_t swing;
	u_int8_t action;
	u_int8_t room_tmp;
	u_int8_t extern_tmp;
	u_int8_t pad[5];
}cl_yt_work_t;

typedef struct {
	u_int8_t off_enable;
	u_int8_t pad2;
	u_int16_t off_remain_min;
	u_int8_t on_enable;//是否使能
	u_int8_t pad1;
	u_int16_t on_remain_min;//剩余时间
}cl_yt_timer_t;

typedef struct {
	u_int16_t index;//机型表下标。=0xffff,表示没扫描过，需要扫描
	u_int16_t ele_assist_power;//辅热功率
	u_int8_t freq_type;//0=定频，1=变频。
	u_int8_t cool_type;//0=单冷，1=冷暖。
	u_int8_t rl_swing;//0=无左右摆风，1=有左右摆风。
	u_int8_t pad2;
	u_int16_t cool_power;//制冷功率。
	u_int16_t hot_power;//辅热功率。
	u_int64_t sn;//sn
}cl_yt_ac_type_t;


#pragma pack(pop)

//查询电量时间段
typedef struct {
	u_int16_t begin_year;
	u_int8_t begin_month;
	u_int8_t begin_day;
	u_int16_t end_year;
	u_int8_t end_month;
	u_int8_t end_day;
}cl_query_ele_time_t;


/*
		故障显示数据B2		故障显示数据B12（变频机）		保护显示数据B13（变频机）		保护显示数据B14（变频机）	保护显示数据B18（变频机）

bit0		室温故障			室内EE故障					外环故障					室外AC电流保护停机		制热内盘过热保护停机

bit1		内盘故障			室外EE故障					外盘故障					压缩机相电流保护停机		制冷外环过低保护停机

bit2		反馈故障			内外机通信故障				排气故障					室外AC电压保护停机		制热外环过高保护停机

bit3		过零故障			外主板与驱动板通信故障		保留						直流母线电压保护停机		保留

bit4		保留				压缩机启动异常				保留						IPM模块温度过			保留

bit5		保留				压缩机失步故障				保留						排气温度过高保护停机		保留

bit6		保留				IPM模块故障				保留						制冷内盘防冻结保护停机	保留
	
bit7		保留				保留						保留						制冷外盘过热保护停机		保留

*/
typedef struct {
	//可设置区域
	u_int8_t onoff;//0:关闭，1:开
	u_int8_t mode;//0＝自动 1＝制冷 2＝除湿 3＝送风 4＝制热
	u_int8_t tmp;//16～31,设置温度
	u_int8_t wind_speed;//0＝自动，1＝风速1，2＝风速2，3＝风速3
	u_int8_t wind_dir;//0＝自动，1＝风向1，2＝风向2，3＝风向3，4＝风向4，5 = 风向5，6 = 上下摆风
	u_int8_t ele_assist;//电辅,1=开，0=关
	u_int8_t sleep;//睡眠,1=开，0=关
	u_int8_t swing;//摆风,0=摆风关闭， 1 = 左右摆风
	u_int8_t room_tmp;//室温-30~99℃
	u_int8_t extern_tmp;//室外温度-30~99℃
	u_int8_t sn_err;//0正常，1表示条形码与实际机型不匹配

	//定时器
	cl_yt_timer_t timer;


	//仅显示区域
	u_int8_t compressor_onoff;//压缩机开关，1=开，0=关
	u_int8_t extern_wind_onoff;//外风机开关，0=关, 1=低风，2=高风
	u_int8_t four_valve;//四通阀开关，1=开，0=关
	u_int8_t ele_hot;//电铺热开关，1=开，0=关
	u_int16_t compressor_work_hours;//压缩机运行累计时间，单位小时
	u_int8_t down_reboot;//掉电重启功能，1=开，0=关
	u_int8_t wind_real_spee;//风机实际转速，显示时值*10，单位转
	u_int8_t inner_tmp;//内盘温度，-30~99℃
	u_int8_t dc_busway_val;//直流母线电压,变频机，显示时，值+200，单位：伏
	u_int8_t extern_ac_val;//室外ac电压,变频机，显示时, 值+100，单位：伏
	u_int8_t extern_ac_cur;//室外ac电流，变频机，显示时, 值/10，保留小数点后一位，单位：安
	u_int8_t compressor_cur;//压缩机相电流，变频机，显示时，值/10，保留小数点后一位，单位：安
	u_int8_t compressor_freq;//变频机, 压缩机运转频率，0~255HZ
	u_int8_t outside_tmp;//柜机或变频机, 外盘温度,-30~99℃
	u_int8_t exhaust_tmp;//变频机, 排气温度，-30~127℃
	u_int8_t ipm_tmp;//变频机, ipm温度，-30~127℃
	u_int8_t heat_defrost;// 1 = 制热除霜状态，0 = 制热非除霜状态
	u_int8_t sys_type;//0=定频挂机，1=定频柜机，2=变频挂机,3=变频柜机
	u_int8_t in_fans_gears;//实际风速档位，0=停止，1=微风，2=低档，3=中档，4=高档
	u_int16_t assit_work_hours;//压缩机运行累计时间，单位小时



	//故障保护状态
	u_int8_t fault_b2;//故障显示数据B2
	u_int8_t fault_b12;//故障显示数据B15,实际是b15，避免jni改动，只添加。。。
	u_int8_t protect_b13;//保护显示数据B16
	u_int8_t protect_b14;//保护显示数据B17 ,位表示含义如上图一
	u_int8_t protect_b18;//保护显示数据18

	//电量统计
	u_int32_t ele_total;//可能不用，先放这儿
	
	cl_query_ele_time_t ele_time;//阶段电量时间段。
	u_int32_t ele_phase;//阶段电量。单位，0.1度

	//关于机型的一些数据
	cl_yt_ac_type_t ac_info;
	char name[30];//机型

	//这个字段是sdk用的，app jni可以不管，主要是为了方便sdk处理
	cl_yt_work_t work;
	bool ac_info_valid;
	bool stat_info_valid;
	bool work_info_valid;
}cl_yt_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	功能:
		主界面控制命令
	输入参数:
		@dev_handle: 设备的句柄
		@action:控制行为
		@value:
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yt_ctrl(cl_handle_t dev_handle, YT_ACTION_T action, u_int8_t value);

/*
	功能:
		定时器设置
	输入参数:
		@dev_handle: 设备的句柄
		@on_remain_min:开机剩余时间配置
		@off_remain_min:关机剩余时间配置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yt_timer_config(cl_handle_t dev_handle, cl_yt_timer_t *ptimer);

/*
	功能:
		条形码扫描
	输入参数:
		@dev_handle: 设备的句柄
		@sn:月兔条形码
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yt_scan_sn(cl_handle_t dev_handle, char *sn);


/*
	功能:
		查询电量，界面选择时间段后调用该函数，后面会发modify，到时在info取数据显示
	输入参数:
		@dev_handle: 设备的句柄
		@time:查询电量的时间段
	输出参数:
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yt_query_ele(cl_handle_t dev_handle, cl_query_ele_time_t *time);






#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_YT_H */

