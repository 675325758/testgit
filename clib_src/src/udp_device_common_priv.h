#ifndef	__UDP_DEVICE_COMMON_PRIV_H__
#define	__UDP_DEVICE_COMMON_PRIV_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"

#define DAYS_PER_WEEK 0x7

// udp 通用功能action
enum{
	ACT_UDP_COM_TIMER_SET = 0x0,
	ACT_UDP_COM_TIMER_DEL ,
	ACT_UDP_COM_PEROID_TIMER_SET ,
	ACT_UDP_COM_PEROID_TIMER_DEL ,
	ACT_UDP_COM_TIMER_REFRESH,
	ACT_UDP_COM_REFRESH_ALL_STAT,
	ACT_UDP_COM_SET_PEAK_TIME, 
	ACT_UDP_COM_SET_VALLEY_TIME, 
	ACT_UDP_COM_SET_PEAK_PRICE, 
	ACT_UDP_COM_SET_VALLEY_PRICE, 
	ACT_UDP_COM_SET_FLAT_PRICE,
	ACT_UDP_COM_REFRESH_ELEC_STAT,
	ACT_UDP_COM_CLEAR_ELEC_STAT,
	ACT_UDP_COM_CLEAR_DEV_ERR_INFO,
	ACT_UDP_COM_REFRESH_DEV_ERR_INFO,
	ACT_UDP_COM_SET_PEMIT_STM_UPGRADE,
	ACT_UDP_COM_RESTORY_FACTORY,
	ACT_UDP_COM_SETTING_SSID_PASSWD,
    ACT_UDP_COM_EXT_PEROID_TIMER_SET,
    ACT_UDP_COM_AJUST_ROOM_TEMP,
    ACT_UDP_COM_AJUST_ELEC_VALUE,
    ACT_UDP_COM_REFRESH_24HOUR_LINE,
    ACT_UDP_COM_REQUEST_SHARE_CODE,
    ACT_UDP_COM_DEL_SHARED_PHONE,
    ACT_UDP_COM_REFRESH_SHARED_LIST,
    ACT_UDP_COM_MODFIY_SHARED_PHONE
};
// 杰能宝温控器控制action
enum{
	ACT_JNB_CTRL_ONOFF = 0x0,
	ACT_JNB_CTRL_MODE ,
	ACT_JNB_CTRL_TEMP,
	ACT_JNB_CTRL_SCHED,
	ACT_JNB_CTRL_TEMP_PARAM,
	ACT_JNB_CTRL_HOLIDAY,
	ACT_JNB_CTRL_HOLD_TIME
};

// 拓邦控制action
enum{
	ACT_TB_CTRL_STAT = 0x0,
	ACT_TB_CTRL_SETTING_WORK_PARAM,
	ACT_TB_CTRL_REFRESH_TEMP_INFO,
	ACT_TB_CTRL_REFRESH_OTHER_INFO,
	ACT_TB_CTRL_REFRESH_FAULT_INFO,
	ACT_TB_CTRL_BIND_BAR_CODE
};

// 亿林温控器控制action
enum{
	ACT_YL_CTRL_ONOFF = 0x0,
	ACT_YL_CTRL_MODE,
	ACT_YL_CTRL_GEAR,
	ACT_YL_CTRL_TEMP,
	ACT_YL_CTRL_SCENE,
	ACT_YL_SETTING_TEMP_PARAM
};

//LEDE调色灯控制action
enum{
	ACT_LEDE_CTRL_STAT = 0x0,
	ACT_LEDE_CTRL_TIEMR ,
	ACT_LEDE_DELETE_TIEMR,
	ACT_LEDE_ON_STAT,
};

enum{
	ACT_AMT_CTRL_ONOFF = 0x0,
	ACT_AMT_CTRL_MODE,
	ACT_AMT_CTRL_GEAR,
	ACT_AMT_CTRL_SHAKE,
	ACT_AMT_CTRL_SCREEN_LIGHT,
	ACT_AMT_CTRL_U_DEF_MODE,
	ACT_AMT_CTRL_S_TIMER,
	ACT_AMT_CTRL_ANION,
	ACT_AMT_CTRL_PLASMA,
	ACT_AMT_CTRL_SMART_PARAM
};

enum{
	ACT_CHIFFO_CTRL_ONOFF = 0x0,
	ACT_CHIFFO_CTRL_WATER_TEMP,
    ACT_CHIFFO_ADD_DEC_WATER_TEMP,
	ACT_CHIFFO_CTRL_HEATER_TEMP,
    ACT_CHIFFO_ADD_DEC_HEATER_TEMP,
	ACT_CHIFFO_SET_MODE,
	ACT_CHIFFO_SET_TIMER,
	ACT_CHIFFO_REFRESH_TIMER,
	ACT_CHIFFO_SET_SCENE,
	ACT_CHIFFO_SET_LOOP,
	ACT_CHIFFO_SET_CLOCK,
	ACT_CHIFFO_SET_WATER_CAP_UP_OR_DOWN,
	ACT_CHIFFO_SET_WATER_CAP
};

//海迅action操作
enum {
	ACT_HX_MODE_CMD = 0X0,
	ACT_HX_DIY_NAME = 0X1,
	ACT_HX_FINISH_CLEAR = 0X2,
};

enum{
	ACT_TL_CTRL_ONOFF = 0,
	ACT_TL_CTRL_MODE,
	ACT_TL_CTRL_FAN_SPEED,
	ACT_TL_CTRL_TEMP,
	ACT_TL_CTRL_ECO,
	ACT_TL_CTRL_LEARN,
	ACT_TL_CTRL_TIMER,
    ACT_TL_CTRL_TIME_AUTO_SYNC,
    ACT_TL_CTRL_TIME_SYNC,
    ACT_TL_CTRL_REFRESH_TIME
};

//千帕茶盘
enum {
	ACT_QPCP_CTRL_ONOFF = 0X0,
	ACT_QPCP_CTRL_ADD_WATER,
	ACT_QPCP_CTRL_HANDLE_CTRL,
	ACT_QPCP_CTRL_SCENE_DEL,
	ACT_QPCP_CTRL_SCENE_MODIFY,
	ACT_QPCP_CTRL_SCENE_EXECUTE,
    ACT_QPCP_CTRL_RESET_FAULT
};

enum{
    ACT_STB_CHANGE_NAME = 0x0,
    ACT_STB_START_MATCH,
    ACT_STB_STOP_MATCH,
    ACT_STB_CTRL_KEY,
    ACT_STB_QUICK_ONOFF,
    ACT_STB_START_LEARN,
    ACT_STB_STOP_LEARN,
    ACT_STB_DELETE_KEY,
    ACT_STB_MODIFY_KEY,
    ACT_STB_START_NKEY_MATCH
};

//千帕锅
enum{
    ACT_QP_POT_CTRL = 0x0,
    ACT_QP_POT_EXEC_SCENE,
    ACT_QP_POT_DEL_SCENE,
    ACT_QP_POT_MODIFY_SCENE
};

//车载悟空
enum {
	ACT_CAR_CONFIG_ON = 0X0,
	ACT_CAR_CTRL_ON = 0X1,
	ACT_CAR_CONFIG_SEARCH = 0X2,
	ACT_CAR_CTRL_SEARCH = 0X3,
	ACT_CAR_CONFIG_VALTAGE = 0X4,
	ACT_CAR_CTRL_VALTAGE = 0X5,
	ACT_CAR_CTRL_POWERSAVE = 0X6,
};

//沙特插座
enum{
    ACT_EO_SET_ONOFF,
    ACT_EO_SET_TEMP_RANGE,
    ACT_EO_SET_THRESHOLD,
    ACT_EO_SET_OFFLINE_ENABLE,
    ACE_EO_SET_PERSON_ENABLE
};

//鑫源温控器
enum {
	ACT_XY_CTRL_ONOFF = 0X0,
	ACT_XY_CTRL_TEMP,
	ACT_XY_CTRL_MODE,
	ACT_XY_CTRL_TIME,
	ACT_XY_CTRL_ADJUST,
	ACT_XY_CTRL_LOCK_ONOFF,
	ACT_XY_CONFIG_SMART_MODE,
	ACT_XY_CTRL_EXTERN_TEMP,
	ACT_XY_CTRL_SMARTHOME_ONOFF,
	ACT_XY_CTRL_SMART_MODE,
};

//bimar暖风机
enum{
    ACT_BIMAR_NONE = 0x0,
    ACT_BIMAR_COMMON_CTRL
};

//千帕破壁机
enum{
    ACT_QP_PBJ_CTRL_ONOFF = 0x0,
    ACT_QP_PBJ_CTRL_EXEC_SCENE,
    ACT_QP_PBJ_CTRL_MODIFY_SCENE,
    ACT_QP_PBJ_CTRL_FAULT_STAT
};

//商用华天成
enum {
	ACT_TBB_ON = 0X0,
	ACT_TBB_MODE,
	ACT_TBB_TMP,
	ACT_TBB_CONFIG,
	ACT_TBB_BIND,
};

//千帕破壁机
enum{
    ACT_HX_YSH_CTRL_ONOFF = 0x0,
    ACT_HX_YSH_CTRL_EXEC_SCENE,
    ACT_HX_YSH_CTRL_MODIFY_SCENE,
    ACT_HX_YSH_CTRL_DEL_OR_STOP_SCENE
};

//月兔
enum {
	ACT_YT_CTRL = 0X0,
	ACT_YT_TIMER,
	ACT_YT_SN,
	ACT_YT_ELE_INFO,
	ACT_YT_QUERY_ELE,
	ACT_YT_QUERY_POWER,
};
//珠海集利灯
enum{
	ACT_JL_UNKNOWN = 0x0,
	ACT_JL_3200_CTRL,
    ACT_JL_3200_AL_CTRL
};


//澳德绅
enum {
	ACT_ADS_CTRL = 0X0,
	ACT_ADS_CONF,
};

//晶石微波炉
enum{
    ACT_JS_WAVE_CTRL = 0x0,
    ACT_JS_AUTO_MENU,
    ACT_JS_CHILD_LOCK,
    ACT_JS_FAST_CTRL
};

//科希曼
enum{
    ACT_KXM_CTRL_ONOFF,
    ACT_KXM_CTRL_MODE,
    ACT_KXM_CTRL_TIMER,
    ACT_KXM_CTRL_DEV_TIME,
    ACT_KXM_CTRL_ALL_TIMER,
    ///////////////////////////////
    ACT_KXM_THER_COMMON_CTRL
};

//思博特
enum{
    ACT_SBT_CTRL_STAT,
    ACT_SBT_SETTINT_PARAM,
    ACT_SBT_AJUST_TIME,
    ACT_SBT_SMART_MODE_PARAM,
    ACT_SBT_SMART_MODE_ENABLE
};

//中山商贤
enum {
	ACT_ZSSX_CTRL_ON,
	ACT_ZSSX_TIMER,
	ACT_ZSSX_WIFI_CONF,
};

// 中山益佳电暖炉
enum {
    ACT_YJ_HEATER_CTRL
};

// 印度车载追踪器
enum {
	ACT_INDIACAR_REQUEST_HISTORY,		// 请求历史记录
	ACT_INDIACAR_REQUEST_UPGRADE,		// 请求升级
	ACT_INDIACAR_REQUEST_WARN_SETTING,	// 告警设置
	ACT_INDIACAR_REQUEST_WIFI_CONFIG,	// 请求WIFI配置
	ACT_INDIACAR_REQUEST_REALTIME_TRIP,	// 请求实时数据
	ACT_INDIACAR_REQUEST_DEBUG_CONFIG,	//  调试信息
	ACT_INDIACAR_REQUEST_VIDEO,	// 开始或者停止视频
	ACT_INDIACAR_REQUEST_LOCAL_VIDEO_WATCH,	// 局域网查看视频
	ACT_INDIACAR_REQUEST_RECORD,	// 请求或者停止录像
	ACT_INDIACAR_REQUEST_DECODE_MP4,	// 请求播放MP4
};

//智科热水器
enum {
	ACT_ZKRSQ_CTRL,
	ACT_ZKRSQ_CONFIG,
	ACT_ZKRSQ_TIMER,
};

//智皇窗帘
enum{
	ACT_ZHCL_STATUS,
	ACT_ZHCL_LOCATION,
	ACT_ZHCL_BIND,
	ACT_ZHCL_TYPE,
	ACT_ZHCL_DIR,
};

//智皇单火线
enum {
	ACT_ZHDHX_ONOFF,
	ACT_ZHDHX_KEY_NAME,
};
///////////////////////////////////////////////////////////////////////////
//各设备私有通讯协议
#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
//拓邦华天成

typedef struct{
	/* 从机下控ID */    
	u_int8_t cid;    
	/* 开关机 */    
	u_int8_t onoff;   
	/* 工作模式 */   
	u_int8_t work_mode;   
	/* 温度值 */    
	u_int8_t temp;    
	/* 预留 */    
	u_int8_t reserved[4];
}ucp_tb_user_config_t;


typedef struct  {
	 /* 从机下控ID */    
	u_int8_t cid;    
	/* 冷媒回开关 */    
	u_int8_t return_cold_switch;    
	/* 设备安装状态 */    
	u_int16_t facility_state;  
	/* 系统功能选择 */   
	u_int16_t sysfunc;    
	/* 回差温度 */    
	u_int8_t return_diff_temp;    
	/* 制热除霜周期 */    
	u_int8_t heat_defrost_circle;    
	/* 制热进入除霜温度 */   
	u_int8_t start_heat_defrost_temp;    
	/* 制热退出除霜温度 */    
	u_int8_t stop_heat_defrost_temp;    
	/* 制热退出除霜时间 */    
	u_int8_t stop_heat_defrost_time;    
	/* 电加热启动设定值 */    
	u_int8_t eheat_value;    
	/* 背光延时关闭时间 */    
	u_int8_t backlight_delay_time;    
	/*风向模式*/
	u_int8_t fan_mode;
	u_int8_t reserved[2];
}ucp_tb_work_config_t;
	

typedef struct  {
	/*从机下控ID*/
	u_int8_t cid;
	u_int8_t mode;   /* 0：手动，1：自动 */
	u_int8_t year;   /* 0 ~ 99 (2000 ~ 2099)*/
	u_int8_t month;  /* 1 ~ 12 */
	u_int8_t mday;   /* 1 ~ 31 */
	u_int8_t hour;   /* 0 ~ 23 */
	u_int8_t minute; /* 0 ~ 59 */
	u_int8_t second; /* 0 ~ 59 */
}ucp_tb_rtc_t;

typedef struct {
	/* 从机下控ID */    
	u_int8_t cid;   
	/* 环境温度 */    
	u_int8_t env_temp;    
	/* 水箱下部温度 */    
	u_int8_t tankbottom_temp;    
	/* 水箱上部温度 */   
	u_int8_t tanktop_temp;   
	/* 盘管温度 */  
	u_int8_t coil_temp;   
	/* 排气温度 */  
	u_int8_t air_temp;    
	/* 回气温度 */    
	u_int8_t returnair_temp;  
	/* 出水温度 */   
	u_int8_t outwater_temp;  
	/* 进水温度 */   
	u_int8_t inwater_temp;   
	/* 回水温度 */    
	u_int8_t returnwater_temp;   
	/* 制热量 */    
	u_int16_t heat_capacity;   
	/* 制热时间 */   
	u_int16_t heat_time;   
	/* 耗电量 */   
	u_int16_t consumption_power
	;    /* 节省电量 */   
	u_int16_t saving_power;  
	u_int16_t reserved;
}ucp_tb_temp_info_t;

typedef struct {
	/* 从机下控ID */  
	u_int8_t cid;   
	/* 电子阀开度 */  
	u_int8_t valve_expansion; 
	/* 从机设备上电状态 */   
	u_int16_t slave_onoff;   
	/* 设备/故障1 */  
	u_int16_t dev_fault;   
	/* 设备/故障2 */   
	u_int16_t dev_guard; 
	u_int16_t load_state;
	u_int16_t reserved;
}ucp_tb_fault_stat;

typedef struct {
	/*从机下控ID*/
	u_int8_t cid;
	/* 机型信息 */
	u_int8_t dev_info;
	/* 模式选择 */
	u_int8_t dev_mode;
	/* 固件版本号 */
	u_int8_t fw_version;
	/* 主控板固件版本号 */
	u_int8_t mb_version;
	u_int8_t svn_version;
	u_int8_t stm_up_state;
	u_int8_t pad;
}ucp_tb_other_info;

typedef struct{
	/* 设备状态 */  
	u_int16_t dev_state;   
	/* 绑定状态 */  
	u_int16_t  bind_state;  
	/* 拓邦SN，17字符 */ 
	u_int8_t tb_sn[24]; 
	u_int8_t reserved[8];
}ucp_tb_bind_info;

///////////////////////////////////////////////////////////////////////////
// 金长信配电箱
typedef struct {
	u_int16_t voltage; //电压 单位0.001V
	u_int16_t elec;  //电流 单位0.001A
	int32_t active_power;	// 有功功率 单位0.1W
	int32_t reactive_power; // 无功功能 单位 0.1 乏尔
	int16_t power_factor;	// 功率因素，单位0.001
	u_int16_t frequency; //频率
	u_int32_t active_degree;	// 有效电度 单位千瓦时
	u_int32_t reactive_degree;	// 无效电度 单位千乏尔时
	u_int32_t sn;				// 序列号
	u_int16_t soft_ver;			// 软件版本
	u_int16_t hardware_ver;		// 硬件版本
} ucp_jcx_power_box_info;

/////////////////////////////////////////////////////////////////////////////////////
// LEDE 灯
typedef struct{	
	u_int8_t R;			
	/*红 0~255*/	
	u_int8_t 	G; 			
	/*蓝0~255*/	
	u_int8_t	B;		
	/*绿0-255*/
	u_int8_t	L;	
	/*亮度0~100*/	
	u_int8_t cold;
	/*色温 0~100*/
	u_int8_t	power;		
	/*开关，0为关，1为开*/
	u_int8_t	mod_id;		
	/*模式id*/	
	u_int8_t	action;
	//行为
}ucp_lede_led_state_t;

typedef struct {
	u_int8_t enable;
	u_int8_t type;
	u_int8_t pad[2];
	
	u_int8_t R;			
	/*红 0~255*/	
	u_int8_t 	G; 			
	/*蓝0~255*/	
	u_int8_t	B;		
	/*绿0-255*/
	u_int8_t	L;	
	/*亮度0~100*/	
	u_int8_t cold;
	/*色温 0~100*/
	u_int8_t	power;		
	/*开关，0为关，1为开*/
	u_int8_t	mod_id;		
	/*模式id*/	
	u_int8_t	action;
	//行为
} ucp_lede_led_on_stat_t;


typedef struct {	
	u_int8_t	id;				
	/*定时器id,有效值从1开始*/	
	u_int8_t	flags;			
	/*bit0:enable*/	
	u_int8_t	week_loop;		
	/*bit0代表星期天，bit1代表星期一，等等。为0代表不循环，到期后自动删除*/	
	u_int8_t	hour;			
	/*小时*/	
	u_int8_t	min;			
	/*分*/	
	u_int8_t	pad1;	
	u_int16_t pad2;	
	ucp_lede_led_state_t	config;	
	/*定时器到期后，使用该设置更新led状态*/
} ucp_lede_led_timer_t;

/////////////////////////////////////////////////////////////////////////////////////////
// 杰能宝温控器

enum{
	U_JNB_OP_DEV_STATUS = 0,
	U_JNB_OP_MODE_CHANGE  ,
	U_JNB_OP_MODE_CONFIG ,
	U_JNB_OP_TEMP_CHANGE
};

typedef struct {
	u_int8_t version;		// 版本。
	u_int8_t operaton_type;	// 操作类型，见上。
	u_int8_t work_status;		// 指定的模式，见上。
	u_int8_t vacation_remain_days; // 启用假期模式后，的剩余天数。

	u_int8_t env_temperature;	// 环境温度
	u_int8_t temp_temperature;	// 临时设置温度
	u_int8_t vacation_temperature;// 假期配置的温度
	u_int8_t vacation_days;	// 假期设定的天数

	u_int8_t comfort_temperaute;	// 舒适模式配置的温度
	u_int8_t comfort_hold_time;	// 临时HOLD时间，单位小时
	u_int8_t economic_temperature;// 经济模式配置的温度
	u_int8_t economic_hold_time;	// 临时HOLD时间，单位小时
	
	u_int32_t scheduler[7];	// scheduler[0]表示星期天，往后依次为星期1到星期6，
				// 其中1～24位分别表示每个时间段的模式选择。25位表示是否配置:0-未配置，1-有配置
} ucp_jnb_thermotat_info;
////////////////////////////////////////////////
//亿林 温控器

typedef struct ts_data_work_s{
	u_int8_t onoff;
	u_int8_t pad[3];
}ucp_yl_work_stat;

typedef struct ts_data_mode_s{
	u_int8_t mode;
	u_int8_t scene;
	u_int8_t gear;
	u_int8_t tmp;
}ucp_yl_data_mode_t;

typedef struct ucp_yl_scene_s{
	u_int8_t set_tmp;
	u_int8_t gear;
}ucp_yl_scene_t;

////////////////////////////////////////////////////////////////////////////////
//海迅通用数据结构
typedef struct ucp_hx_data_s{
	u_int8_t value;
	u_int8_t pad[3];
}ucp_hx_data_t;

typedef struct ucp_hx_diy_name_s{
	u_int8_t id;
	u_int8_t len;
	//u_int8_t name[0];
}ucp_hx_diy_name_t;

//////////////////////////////////////////////////////////////////////////////////
//特林温控器和采暖通用

typedef struct{
	/* 开关：关、开 */
	u_int8_t onoff;
	/* 运行模式：通风、自动、制冷、制热 */
	u_int8_t mode;
	/* 风速设定：低、中、高 */
	u_int8_t speed;
	/* 温度设定：5 - 35 */
	u_int8_t temp;
	/* 测量温度 */
	u_int16_t room_temp;
	/* 位标志 */
	u_int16_t state;
	u_int8_t reserved[4];
}ucp_tl_conf_t;

typedef struct  {
	/* 位操作 */
	u_int16_t lock_flags;
	/* 温度带宽设定 */
	u_int16_t temp_bandwidth;
	/* 低档计费因子 */
	u_int16_t charge_factor_low;
	/* 中档计费因子 */
	u_int16_t charge_factor_mid;
	/* 风机是否受控 */
	u_int8_t fan_under_control;
	/* 计时允许 */
	u_int8_t time_on;
	/* ECO节能 */
	u_int8_t eco_mode;
	/* 自学习 */
	u_int8_t self_learn;
    u_int8_t run_timer;
    u_int8_t reserved[3];
}ucp_tl_adv_t;

typedef struct{
	/* 累计折算高档时间 */
	u_int32_t high_gear_time_cal;
	/* 累计低档时间 */
	u_int32_t low_gear_time;
	/* 累计中档时间 */
	u_int32_t mid_gear_time;
	/* 累计高档时间 */
	u_int32_t high_gear_time;
}ucp_tl_total_time_t;

typedef struct{
	u_int16_t temp;
	u_int16_t hour;
	u_int16_t min;
}ucp_tl_timer_item_t;

typedef struct{
	ucp_tl_timer_item_t time[4];
}ucp_tl_timer_info_t;

typedef struct {
    u_int32_t utc_time;
    u_int8_t is_auto;
    u_int8_t just_sync;
    u_int8_t pad[2];
}ucp_tl_time_sync_t;

#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////

/*是否支持设备，新增的时候填写*/
extern bool is_supported_udp_device(u_int8_t sub_type, u_int8_t ext_type);
/*初始化AC 时的回调，主要是分配内存*/
extern bool udp_init_ac_hook(user_t* user,smart_air_ctrl_t* ac);
/*初始化AC 时的回调，主要是分配内存*/
extern void udp_set_support_flag_hook(smart_air_ctrl_t* ac, uc_sys_support_obj_t* so);
// APP 发送指令到SDK后的处理
extern bool udp_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret);
// 判断APP发送的命令是否支持离线操作
extern bool udp_proc_support_offline(u_int32_t notify_type);
// 查询系统信息后，紧跟着需要查询的设备状态信息
extern bool udp_quick_query_info_hook(smart_air_ctrl_t* ac);
// 更新除系统信息外的私有信息
extern bool udp_update_data_hook(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
//控制后的反馈结果
extern int udp_proc_ctrl_hook(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
// 给APP 的数据反馈
extern bool udp_build_objs_hook(user_t* user,cl_dev_info_t* ui);
//释放外部私有数据 
extern void udp_free_objs_hook(struct cl_com_udp_device_data_s* udp_dev_info);
//释放内部私有数据 
extern void udp_free_sdk_priv_data(struct cl_com_udp_device_data_s* udp_dev_info);
//可以看情况控制是否查询一下设备所有状态，是否给模组发modify
int udp_proc_ctrl_modify_hook(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
void do_rfgw_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
// 这个命令是否支持在APP服务器上面
bool is_supported_udp_app_server(u_int8_t obj, u_int8_t sub_obj, u_int8_t attr);

//////////////////////////////////////////////////////
//支持通用扩展定时的，且定时参数不仅是开关操作，需要多参数的，请按照设备协议填写如下函数
//根据上层数据填扩展部分报文，返回填充的长度
extern int udp_fill_ext_peroid_timer_modify_pkt(smart_air_ctrl_t* ac,cl_period_timer_t* t_hdr,void* src_priv,void* dest);
//根据数据包参数填充给APP的数据,
//ext_pkt ，指向扩展数据，各类型设备不同，
//len,扩展数据长度
extern void udp_updata_ext_peroid_timer_by_ext_data(smart_air_ctrl_t* ac,void* ext_pkt,int len ,cl_period_timer_t* dest);
//另一个通用定时器
void ucp_comm_update_timer_modify_pkt(user_t *user, bool is_slave, u_int64_t slave_sn, ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer);
int ucp_comm_fill_timer_modify_pkt(user_t *user, bool is_slave, u_int64_t slave_sn, ucp_comm_timer_t *puct, cl_comm_timer_t *ptimer);

#endif

