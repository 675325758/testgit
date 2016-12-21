#ifndef	__AMT_SCM_CTRL_H__
#define	__AMT_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "cl_amt_device.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

// 艾美特产品类型
enum {
	//内销排风扇
	AMT_PRODUCT_TYPE_VENTILATIN_FAN = 0x9,
	//内销加湿机
	AMT_PRODUCT_TYPE_HUMIDIFIER = 0xA,
	//内销除湿机
	AMT_PRODUCT_TYPE_DEHUMIDIFIER = 0xB,
	// 内销电风扇
	AMT_PRODUCT_TYPE_FAN = 0xE
};

//数据类型掩码
#define AMT_DATA_TYPE_MASK	(0x0F)

//-------------------------------------------------------
//设备状态十分变化掩码
#define AMT_DATA_CHANGE_MASK (0xF0)
#define AMT_CTRL_CMD_MASK (0xA0)

enum{
	//产品种类
	AMT_DATA_TYPE_PRODUCT_CATEGORY = 0x1,
	//产品型号
	AMT_DATA_TYPE_PRODUCT_MODEL = 0x2,
	//开关机状态
	AMT_DATA_TYPE_STATUS = 0x3,
	//模式和档位
	AMT_DATA_TYPE_GEAR_INFO = 0x4,
	//定时器
	AMT_DATA_TYPE_TIMER = 0x5,
	//摇头
	AMT_DATA_TYPE_SHAKE = 0x6,
	//其他杂项信息
	AMT_DATA_TYPE_OTHER_INFO = 0x7,
	//环境信息
	AMT_DATA_TYPE_ENV = 0x8 ,
	//设备运行其他参数
	AMT_DATA_TYPE_DEV_RUN_INFO = 0x9,
	//耗电信息
	AMT_DATA_TYPE_ELEC_STAT = 0xA
};

enum{
	// 云端控制开关及显示状态
	AMT_CTRL_CODE_ONOFF = 0xA2,
	// 云端控制产品模式和档位
	AMT_CTRL_CODE_GEAR = 0xA3, 
	// 云端控制定时器
	AMT_CTRL_CODE_TIMER = 0xA4,
	// 云端控制摇头
	AMT_CTRL_CODE_SHAKE = 0xA5,
	// 云端控制其他信息
	AMT_CTRL_CODE_OTHER = 0xA6,
	//云端控制环境信息
	AMT_CTRL_CODE_ENV = 0xA7,
	//云端控制情景
	AMT_CTRL_CODE_SCENE = 0xAB
};

//数据变化后的数据
#define AMT_CHANGE_DATA 		(0x5)
//定时发送给wifi模组的数据
#define AMT_TIMER_DATA 			(0xD)

#define AMT_SMT_ERROR			(0xE1)

//有哪些按键
enum{
	AMT_KEY_NO = 0x0, //没有开关键
	AMT_KEY_ONLY_OFF = 0x1, //只有关按键
	AMT_KEY_ONLY_ON = 0x2, //只有开按键
	AMT_KEY_ON_OFF = 0x3,//独立的开和关
	AMT_KEY_ONOFF = 0x4, //开关按键合一  
	AMT_KEY_GEARON_OFF = 0x5, // "开/档位"、"关"两个按键
	AMT_KEY_GEARONOFF = 0x6 // 只有"开/档位/关"一个按键
};

#define AMT_FUNC_NO (0x0)
#define AMT_FUNC_YES (0x3)

#define AMT_FAN_USER_DEF_MASK 0x8

#define AMT_FIX_START_CODE 0xAAAA
#define AMT_FIX_END_CODE 0x55

#define AMT_DATA_SRC_MASK 0xF0
#define DEV_NO_CHANGE_TO_PHONE 0xD0
#define DEV_HAS_CHANGE_TO_PHONE 0x50
#define DEV_PHONE_CTRL_RESPONSE 0xA0


//定时类型
enum{
	AMT_TIME_TYPE_DAY, //天
	AMT_TIME_TYPE_HOUR,//小时
	AMT_TIME_TYPE_MIN,//分
	AMT_TIME_TYPE_SECOND //秒
};

enum{
	AMT_ENV_TEMP = 0x0,
	AMT_P1_TEMP,
	AMT_P2_TEMP,
	AMT_P3_TEMP
};

enum{
	AMT_TIMER_DT_REMAIN = 0x0,
	AMT_TIMER_DT_OFF_SETTING,
	AMT_TIMER_DT_ON_SETTING
};

enum{
	AMT_TIMER_MODE_NO,
	AMT_TIMER_MODE_OFF,
	AMT_TIMER_MODE_ON,
	AMT_TIMER_MODE_ON_OFF
};

#define AMT_A10_CUR_POWER 0xFF

enum{
	AMT_A7_DB_ANION = 0, //负离子
	AMT_A7_DB_PLASMA //等离子
};

/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	u_int16_t fix_s_code;
	u_int8_t len;
	u_int8_t product_type;
	u_int8_t data_type;
}amt_pkt_header_t;

//开关机和屏显，指令53
typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN  
	u_int8_t is_on:1,
		on_off_key:3, 
		is_screen_display:2, // 0x00,关，0x11 开
		has_screen:2; // 0x00,无，0x11 有
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t has_screen:2,
		is_screen_display:2, // 0x00,关，0x11 开
		on_off_key:3, 
		is_on:1;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_53h_info_t;

//产品模式和档位,0x54
typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t cur_mode:4, //现在运行在哪个模式下 
		total_mode:4;//总共支持多少种模式
		
	u_int8_t max_gear:6, //产品调速方式
		speed_mode:2; // 产品最大档位
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t total_mode:4,//总共支持多少种模式
		cur_mode:4; //现在运行在哪个模式下
	u_int8_t speed_mode:2, //产品调速方式
		max_gear:6; // 产品最大档位
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t cur_gear; // 产品当前工作档位
}amt_54h_info_t;

//用户自定义模式

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t group:4, //组别
		index:4; //组内index
	u_int8_t total_mode:4,//总共支持多少种模式
		config_mode:4; //现在运行在哪个模式下
	u_int8_t gear;
	u_int8_t time_type:2, //时间类型
		time:6; //定时时间
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t index:4, //组别
		group:4; //组内index
	u_int8_t config_mode:4,//总共支持多少种模式
		total_mode:4; //现在运行在哪个模式下
	u_int8_t gear;
	u_int8_t time:6, //时间类型
		time_type:2; //定时时间
#else
# error "Please fix <bits/endian.h>"
#endif
	
}amt_user_define_mode;

typedef struct{
	u_int8_t crc;
	u_int8_t fix_end_code1;
	u_int8_t fix_end_code2;
}amt_pkt_tail_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t env_dir:4,
		env_type:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t env_type:4,
		env_dir:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t env_val;
}amt_env_item_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t time:6,
		time_type:2;
	
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t time_type:2,
		time:6;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_time_item_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t timer_index:4,
			timer_type:2,
			timer_mode:2;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t timer_mode:2,
			timer_type:2,
		timer_index:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	amt_time_item_t item[4];	
}amt_machine_timer_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	
	u_int8_t shake_type:4,
		pad:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t pad:4,
		shake_type:4;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_shake_type_t;

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
	u_int8_t cur_angle:4,
		max_angle:4;
	
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t max_angle:4,
		cur_angle:4;
#else
# error "Please fix <bits/endian.h>"
#endif
}amt_shake_angle_t;

typedef struct{

	u_int8_t d_type;
	u_int16_t d_value;
}amt_elec_item_t;

#pragma pack(pop)

typedef struct{
	amt_54h_info_t cmd_54_info;
	u_int8_t cur_user_define_mode[MAX_USER_DEFINE_MODE_POINT];
}amt_mode_info_t;

typedef struct{
	amt_shake_type_t shake_type;
	amt_shake_angle_t shake_angle;
}amt_shake_info_t;

typedef struct{
	u_int8_t onoff; //开关
	u_int8_t cur_mode; //当前模式
	u_int8_t cur_gear; //当前档位
	u_int8_t screen_light; //屏显
	u_int8_t is_shake; //是否处在摇头阶段
	int8_t cur_temp;//当前温度
	u_int8_t is_anion_on; //负离子
	u_int8_t is_plasma_on; //等离子
	u_int16_t cur_power; //当前功率
	
	u_int8_t is_timer_on_valid; //预约开机是否有效
	u_int8_t is_timer_off_valid; //定时关机是否有效
	u_int32_t dev_on_remain_time;//预约开机剩余时间
	u_int32_t dev_off_remain_time; // 定时关机剩余时间

	u_int8_t product_type;
	u_int8_t product_category[32];
	amt_53h_info_t cmd_53_info;
	amt_mode_info_t mode_info;
	amt_shake_info_t shake_info;
}amt_fan_priv_data_t;

extern bool amt_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool amt_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern int amt_scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
extern bool amt_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
extern bool amt_send_set_timer_ctrl_cmd(smart_air_ctrl_t* ac);

extern int amt_get_ext_type_by_tlv(uc_tlv_t* tlv);

#endif

