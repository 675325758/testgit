#ifndef	__CL_SMART_APPLIANCE_H__
#define	__CL_SMART_APPLIANCE_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_rc.h"
    
//event
enum {
	SAE_BEGIN = 1200,
    SAE_INFO_MODIFY = 1201, //状态发生变化
    SAE_AIR_CTRL_OK = 1202, // 控制成功，
    SAE_MODIFY_TIMER_OK = 1203, //创建和修改定时器
    SAE_MODIFY_PRICE_OK = 1204, //修改电价
    SAE_MODIFY_PERIOD_OK  = 1205, // 修改峰、谷计算周期
    SAE_CTRL_LED_OK  = 1206, // 控制LED
    SAE_DEL_TIMER_OK  = 1207, //删除定时器
    SAE_SMART_CTRL_OK   = 1208,//智能控制
    SAE_CODE_MATCH_DEV_READY_OK  = 1209,//设备准备好匹配 1.云匹配是准备好接收遥控板编码 2.全匹配是已经开始匹配
    SAE_CODE_MATCH_DEV_RECV_CODE  = 1210,//云匹配模式下，已经接收到编码，开始云匹配 10
    SAE_CODE_MATCH_STAT_MODIFY  = 1211, //匹配编码过程中状态有变化
    SAE_CODE_MATCH_STOP_OK  = 1212, //停止编码成功
    SAE_CODE_MATCH_OK  = 1213,//匹配编码成功
    SAE_SET_NICK_NAME_OK  = 1214, //修改昵称成功
    SAE_DEV_POWER_NOTIFY  = 1215, //设备功率变化
    SAE_CTRL_LED_COLOR_OK  = 1216, // 控制LED
    SAE_TT_IR = 1217, //透传到客户端的红外编码
    SAE_TT_SOUND = 1217, //透传到客户端的声音

	SAE_HTLLOCK_ADMIN_LOGIN_OK = 1218,		// 汇泰龙登陆成功
	SAE_HTLLOCK_ADMIN_LOGIN_FAILED = 1219, // 汇泰龙登陆失败
	SAE_HTLLOCK_SET_NAME_OK = 1220,	// 汇泰龙设置用户名字成功
	SAE_HTLLOCK_SET_NAME_FAILED = 1221,// 汇泰龙设置用户名字失败
	SAE_HTLLOCK_SET_PIC_OK = 1222,// 汇泰龙设置用户头像成功
	SAE_HTLLOCK_SET_PIC_FAILED = 1223,// 汇泰龙设置失败
	SAE_HTLLOCK_SET_BIND_OK = 1224,// 汇泰龙关联用户的指纹、密码、扫描卡成功
	SAE_HTLLOCK_SET_BIND_FAILED = 1225, // 汇泰龙关联用户的指纹、密码、扫描卡失败
	SAE_HTLLOCK_SET_UNBIND_OK = 1226,// 汇泰龙设置取消关联成功
	SAE_HTLLOCK_SET_UNBIND_FAILED = 1227,// 汇泰龙设置取消关联失败

	SAE_HTLLOCK_SET_INFO_NOTICE_OK = 1228, // 汇泰龙设置信息提醒成功
	SAE_HTLLOCK_SET_INFO_NOTICE_FAILED = 1229, // 汇泰龙设置信息提醒失败

	SAE_HTLLOCK_SET_REMINDER_ONOFF_OK = 1230,	// 汇泰龙设置提醒开关成功
	SAE_HTLLOCK_SET_REMINDER_ONOFF_FAILED = 1231, // 汇泰龙设置提醒开关失败


	SAE_ZKCLEANNER_DAY_DATA = 1233,	// 中科净化器获取到了当天测量数据
	SAE_ZKCLEANNER_MONTH_DATA = 1234,	// 中科净化器获取到了最近一个月测量数据

	SAE_DWKJ_SET_TIMER_OK = 1235,	// 电王科技设置定时器成功
	SAE_DWKJ_SET_TIMER_FAILD = 1236, // 电王科技设置定时器失败

	SAE_HTLLOCK_SET_PIN_OK = 1237,	// 汇泰龙设置PIN码成功
	SAE_HTLLOCK_SET_PIN_FAILED = 1238,	// 汇泰龙设置PIN码失败

	SAE_HTLLOCK_UNLOCK_OK = 1239,	// 汇泰龙开锁成功
	SAE_HTLLOCK_PIN_REPLY = 1240,	// 查询回来临时PIN

	SAE_DEV_COMM_HISTORY_SUMMARY = 1241,	// 通用历史记录的摘要信息更新
	SAE_DEV_COMM_HISTORY_ITEM = 1242,	// 通用历史记录的数据更新

    SAE_MODIFY_TIMER_FAILED = 1250, //16
    SAE_AIR_CTRL_FAILED = 1251,
    SAE_MODIFY_PRICE_FAILED = 1252,
    SAE_MODIFY_PERIOD_FAILED = 1253,
    SAE_CTRL_LED_FAILED = 1254,
    SAE_DEL_TIMER_FAILED = 1255,
    SAE_SMART_CTRL_FAILED = 1256,
    SAE_CODE_MATCH_START_FAILED = 1257,//23
    SAE_CODE_MATCH_STOP_FAILED = 1258,
    SAE_CODE_MATCH_FAILED = 1259,//匹配编码失败
    SAE_SET_NICK_NAME_FAILED = 1260,
    SAE_AIR_CTRL_NOT_MATCH = 1261, //控制按键时未匹配编码
    SAE_CTRL_LED_COLOR_FAILED  = 1262, // 控制LED
    SAE_SMART_HOME_ON = 1263, //智能回家开启空调
    SAE_SMART_HOME_CANCEL = 1264, //取消智能回家(智能回家开启空调后一定时间开未到家)

	SAE_LEARN_KEY_DEV_READY_OK = 1265, //设备进入学习状态
	SAE_LEARN_KEY_DEV_BUSY = 1266, //设备忙，正在学习中
	SAE_LEARN_KEY_WAIT_TIME_OUT = 1267, //设备等待用户按遥控超时
	SAE_LEARN_KEY_SUCCESSED = 1268, // 设备成功学习到编码

	SAE_SCENE_ID_MAX = 1269,//情景到达个数上限
	SAE_TIMER_TIME_CONFLICT = 1270,//定时器时间冲突
	SAE_TIMER_ID_MAX = 1271,//定时器到达个数上限
    
    SAE_LEARN_KEY_NEED_NEXT_KEY = 1272,//电器匹配时需要匹配另一个按键
    
    SAE_RF_DEV_ALARM_INFO = 1275,//RF设备报警推送
    SAE_YT_LOCK_CTRL_OK = 1276,//开关门成功
    SAE_YT_LOCK_SET_PASSWD_OK = 1277,//设置密码成功
    SAE_YT_LOCK_CHANGE_PASSWD_OK = 1278, //修改密码成功
    SAE_YT_LOCK_CTRL_PASSWD_ERR = 1279,//开关门时密码错误
    SAE_YT_LOCK_PASSWD_ERR = 1280, //修改密码时，原始密码错误
    SAE_YT_LOCK_CTRL_NO_PASSWD = 1281, //操作失败，未设置控制密码

	SAE_SHARE_COEE_GET = 1282,//安全共享获取共享码成功的事件
	SAE_YT_SN_ERR = 1283,//月兔条形码与实际机型不匹配
	SAE_RF_DEV_COMM_ALARM_INFO = 1284, // 通用的RF从设备报警信息，从cl_rfdev_status_t 下cai获取

	SAE_RF_DEV_COMM_HISTORY_SUMMARY = 1285,	// 通用历史记录的摘要信息更新
	SAE_RF_DEV_COMM_HISTORY_ITEM = 1286,	// 通用历史记录的数据更新
    
    SAE_CODE_MATCH_START_DOWNLOAD_CODE = 1287, //匹配编码：开始从服务器下载编码；
    
    SAE_CODE_MATCH_START_NOTIFY_DEV = 1288, //匹配编码: 通知设备学习编码成功

	SAE_RF_DEV_DOOR_LOCK_CONTROLLER = 1289,	// 有按下遥控器，在设备信息里面的last_controller_id里面获取
    
    SAE_CODE_MATCH_START_RECV_IR_CODE = 1290, //匹配编码： 设备已接收到遥控器编码，手机准备接收

	SAE_RF_DEV_COMM_HISTORY_ITEM_V2 = 1291, // 更高级的历史记录返回事件

	SAE_COMMON_CTRL_OK= 1297, //通用的控制成功消息
	SAE_COMMON_CTRL_FAILED = 1298, //通用的控制失败消息
    
	SAE_END = SAE_BEGIN + 99
};
    
//匹配编码过程出错
enum{
    ERR_CM_WAIT_IR_SINGLE = 0x1, //等待用户按遥控板超时
    ERR_CM_PROCESS_MATCHING = 0x2, // 目前设备正在处理匹配
    ERR_CM_WAIT_SERVER_CODE =0x3, //等待从服务器获取编码超时
    ERR_CM_NO_RESULT = 0x4,             //匹配所有编码，无法匹配
    ERR_CM_SEVER_NO_CONNECT = 0x5, //设备未连接服务器
    ERR_CM_TO_DEV_TIME_OUT = 0x10

};
// LED 调色灯
typedef enum {
	AIR_LED_DEFAULT_COLOR = 0,		// 熄灭
	AIR_LED_RED_COLOR,			//红色
	AIR_LED_GREEN_COLOR,		//绿色
	AIR_LED_BLUE_COLOR,			//蓝色 
	AIR_LED_YELLOW_COLOR,		//黄色
	AIR_LED_PURPLE_COLOR,		//紫色
	AIR_LED_LIGHT_BLUE_COLOR,     //浅蓝色
	AIR_LED_WHITE_COLOR			//白色
}COLOR_T;


//一年12个月
#define MONTH_PER_YEAR  (12)

// 编码匹配
#define CL_AIR_CODE_MATCH_STOP       0x0
#define CL_AIR_CODE_MATCH_CLOUD 0x1
#define CL_AIR_CODE_MATCH_ALL      0x2

// 模式
#define	CL_AC_MODE_AUTO	0
#define	CL_AC_MODE_COLD	1
#define	CL_AC_MODE_AREFACTION 2
#define	CL_AC_MODE_WIND	3
#define	CL_AC_MODE_HOT	4

// 风速
#define	CL_AC_WIND_AUTO	0
#define	CL_AC_WIND_HIGH	1
#define	CL_AC_WIND_MIDD	2
#define	CL_AC_WIND_LOW	3
    
// 风向
#define	CL_AC_DIR_AUTO	0
#define	CL_AC_DIR_1	1
#define	CL_AC_DIR_2	2
#define	CL_AC_DIR_3	3

//老式样空调键值
#define AC_CKEY_POWER 0
#define AC_CKEY_MODE_AUTO 1
#define AC_CKEY_MODE_COLD 2
#define AC_CKEY_MODE_AREFACTION 3
#define AC_CKEY_MODE_WIND 4
#define AC_CKEY_MODE_HOT 5
#define AC_CKEY_TEMP_UP 6
#define AC_CKEY_TEMP_DOWN 7
#define AC_CKEY_WIND_AUTO 8
#define AC_CKEY_WIND_LOW 9
#define AC_CKEY_WIND_MID 10
#define AC_CKEY_WIND_HIGH 11
#define AC_CKEY_DIR_MANUAL 12
#define AC_CKEY_DIR_AUTO 13



//设备端支持定时器最多20个
#define ID_TIMER_MAX	20

#pragma pack(push, 1)
typedef struct {
	u_int16_t ad;
	u_int16_t ad2;
}ad_data_t;
#pragma pack(pop)


//智能终端设备(UDP通讯模式的)
typedef struct{
    // AC_POWER_xxx
	u_int8_t onoff;
	// AC_MODE_XXX
	u_int8_t mode;
	// 空调温度
	u_int8_t temp;
	// 风速，AC_WIND_xxx
	u_int8_t wind;
	// 风向，AC_DIR_xxx
	u_int8_t wind_direct;
}cl_air_work_stat_t;

typedef struct {
    u_int8_t id; //定时器ID
    u_int8_t enable; //是否启用
    u_int8_t week; // 星期几重复
    u_int8_t hour; //小时
    u_int8_t minute; //分钟
    // APP看到的是true/false(true为开), CLIB内部看到的是AC_POWER_ON / AC_POWER_OFF
    u_int8_t onoff; //开关
	u_int8_t repeat;
	u_int8_t reserved;
} cl_air_timer_t;
    
enum{
    PT_EXT_DT_UNKNOWN = 0x0,
    PT_EXT_DT_QPCP, /*千帕茶盘定时器*/
    PT_EXT_DT_808,
    PT_EXT_DT_101_OEM ,/*沙特、晴乐插座*/
    PT_EXT_DT_QP_POT, //千帕锅定时器
    PT_EXT_DT_QP_PBJ, //千帕破壁机定时器
    PT_EXT_DT_HX_YSH, //海迅养生壶定时器
    PT_EXT_DT_HTC_POOL, //华天成泳池机
};
    
typedef struct {
    u_int8_t min_temp;
    u_int8_t max_temp;
    u_int8_t pad[2];
}cl_101_oem_timer_t;
    
/*千帕茶盘定时器私有数据*/
typedef struct {
    u_int16_t id;
}cl_qp_timer_ext_t;
    
typedef struct {
    u_int8_t onOff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t fan_speed;
    u_int8_t fan_dir;
    u_int8_t key_id;
    u_int16_t pad;
}cl_808_timer_ext_t;

typedef struct {
    u_int16_t cook_id;
    u_int16_t cook_time;
    u_int8_t hot_degress;//加热强度（等级）
    u_int8_t microswitch; //微动开关次数
    u_int8_t warm_temp; //保温温度
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
}cl_qp_pot_timer_t;

typedef struct {
    u_int16_t scene_id;
    u_int8_t pad;
}cl_qp_pbj_timer_t;
    
typedef struct {
    u_int16_t scene_id;
    u_int8_t temp;
    u_int8_t work_time;
    u_int8_t power;
    u_int8_t keep_temp;
    u_int8_t keep_time;
    u_int8_t pad;
}cl_hx_ysh_timer_t;
    
typedef struct {
	u_int8_t id;				/* 策略ID */
	u_int8_t hour;			/* 小时 0-23 */
	u_int8_t minute;			/* 分钟 0-59 */
	u_int8_t week;				/* bit 0-6位对应星期天到星期六 */
	u_int8_t enable;			/* 是否生效(手机设置) 或者已经无效(设备返回) */
    u_int8_t onoff;				/* 是否是定时开机 */
	u_int16_t duration;			/* 持续分钟数 */
    u_int16_t ext_data_type;    /*扩展数据类型 */
    union {
        cl_qp_timer_ext_t qp_time_info; //千帕茶盘扩展数据
        cl_808_timer_ext_t air_timer_info; //808空调扩展
        cl_101_oem_timer_t oem_101_timer_info; //沙特、晴乐扩展
        cl_qp_pot_timer_t qp_pot_timer_info; //千帕锅定时器
        cl_qp_pbj_timer_t qp_pbj_timer_info; //千帕破壁机定时器
        cl_hx_ysh_timer_t hx_ysh_timer_info; //海迅养生壶定时器
    }pt_ext_data_u;
}cl_period_timer_t;

typedef struct {
    u_int8_t on_effect; //下次开启是否有效
    u_int8_t off_effect; //下次关闭是否有效
    u_int8_t timer_count; //多少个定时器
    u_int8_t next_on_day;
    u_int8_t next_on_hour;
    u_int8_t next_on_min;
    u_int8_t next_off_day;
    u_int8_t next_off_hour;
    u_int8_t next_off_min;
    u_int8_t pad[3];
    u_int16_t on_minute; //距离下次开启的分钟数
    u_int16_t off_minute; //距离下次关闭的分钟数
    cl_air_timer_t* timers; // 定时器列表
    cl_period_timer_t* period_timers; //周期性时间
}cl_air_timer_info_t;

typedef struct {
    // 开始时间，单位：分钟（0:0算起）
    u_int16_t begin_minute;
    // 持续多长，单位分钟
    u_int16_t last_minute;
}cl_peak_time_t;
    
typedef struct{
    u_int32_t month_peak[MONTH_PER_YEAR]; //最近12月峰值
    u_int32_t month_valley[MONTH_PER_YEAR]; //最近12月谷值
    u_int32_t month_normal[MONTH_PER_YEAR]; //最近12月平值
    cl_peak_time_t peak_time; //每天的峰值计量时间段
    cl_peak_time_t valley_time; //每天的谷电计量时间段
    cl_peak_time_t flat_time; //每天平电时间段
    u_int32_t peak_price; // 峰值价格:度/分
    u_int32_t valley_price; // 谷值价格; 度/分
    u_int32_t flat_price; //平电价格: 度/分
}cl_elec_stat_info_t;

typedef struct {
    //智能开关
    u_int8_t on;
    //推送开关
    u_int8_t push_on;
    //夏天温度高则智能制冷开关
    u_int8_t sum_on ;
    //夏天温度上限,默认１７
    u_int8_t sum_tmp;
    //冬天温度高于xx度则制热开关
    u_int8_t win_on;
    //制热xx度，默认３０
    u_int8_t win_tmp;
    //到家开还是到家前５分钟开空调的开关
    u_int8_t home_on;
    u_int8_t pad[1];
}cl_smart_air_on_param_t;
    
typedef struct {
    //智能关功能开关
    u_int8_t on;
    //推送开关
    u_int8_t push_on;
    // 1小时内没人，自动关机(可改时间0.5-3小时，以0.5小时为单位，即范围为１－６，默认为２)
    u_int8_t off_time;
    u_int8_t pad[1];
}cl_smart_air_off_param_t;

typedef struct {
    u_int8_t on;
    u_int8_t pad[3];
}cl_smart_air_sleep_param_t;

typedef struct{
	u_int32_t begin_time; //UTC 时间
	u_int32_t elec; //统计的电量
}cl_air_elec_item_info;

typedef struct{
	u_int8_t air_on_color;
	u_int8_t air_off_color;
	u_int16_t pad;
}cl_air_led_color_info;

typedef struct {
	u_int8_t is_info_valid;
	u_int8_t pad;
	u_int16_t days_count; //有多少条数据
	u_int32_t nearest_data_time; //最近一条记录UTC 时间
	u_int16_t* elec_data; //电量数据
}cl_elec_days_stat_info;

typedef struct{
	u_int16_t c_id; //红外编码ID
	u_int8_t is_on; //开关机 
	u_int8_t mode;//模式
	u_int8_t temp; //温度
	u_int8_t fan; //风量
	u_int8_t fan_dir; //风向
	u_int8_t key; //关键字，设备用
}cl_ac_code_item_t;

typedef struct{
	u_int16_t cur_match_id; // 当前使用的id
	u_int16_t code_num; //可切换编码数量
	cl_ac_code_item_t* items;
}cl_ac_code_match_info_t;

#define AC_PAN_CLOUD		0x0 //云匹配面板
#define AC_PAN_WINDOW 	0x1 //窗机面板，固定10个按键 AC_KEY_ID_POWER---AC_KEY_ID_TEMP_MODE_AUTO
#define AC_PAN_ONLY_ONOFF 0x2 //窗机面板，固定2个按键 开关

enum{
	AC_KEY_ID_UN_KNOWN = 0x0, //未知保留按键ID
	AC_KEY_ID_POWER = 0x1,//电源键
	AC_KEY_ID_TEMP_ADD  = 0x2, // 温度加
	AC_KEY_ID_TEMP_DEC  = 0x3, //温度减
	AC_KEY_ID_FAN_ADD  = 0x4, //风量+
	AC_KEY_ID_FAN_DEC  = 0x5,//风量-
	AC_KEY_ID_MODE_COLD  = 0x6, //制冷
	AC_KEY_ID_MODE_HOT  = 0x7, //制热
	AC_KEY_ID_MODE_FAN  = 0x8, //送风
	AC_KEY_ID_MODE_ENERGY_SAVE  = 0x9 //节能
};

//沙特带LED按钮 type = AC_PAN_ONLY_ONOFF
enum{
    AC_KEY_ID_SAUDI_KNOWN = 0x0,
    AC_KEY_ID_SAUDI_OFF = 0x1, //关机
    AC_KEY_ID_SAUDI_ON = 0x2 //开机
};

typedef struct {
	// AC_POWER_xxx
	u_int8_t onoff;
	// AC_MODE_XXX
	u_int8_t mode;
	// 16 - 30
	u_int8_t temp;
	// 风速，AC_WIND_xxx
	u_int8_t wind;
	// 风向，AC_DIR_xxx
	u_int8_t wind_direct;
	// 键值，AC_KEY_xxx
	u_int8_t key;
} air_key_stat_t;


#define MAX_KEY_NAME_LEN 16
typedef struct{
	u_int8_t key_id;
	u_int8_t is_support_learn; //是否支持学习
	u_int8_t is_support_change_name; //是否支持修改名称
	u_int8_t is_support_delete; //是否支持删除该按键
	u_int8_t is_learn_code; //是否已经学习到编码
	u_int8_t is_snapshot_key;	// 这个按键是否是快照
	u_int8_t is_need_decode;	// 学习的按键信息是否需要在控制的时候反解析出内容
	u_int8_t name[MAX_KEY_NAME_LEN];
}cl_air_key;

typedef struct{
	u_int8_t key_num; //按键个数
	cl_air_key* keys;	//按键信息

	// 下面两个收到 SAE_LEARN_KEY_SUCCESSED 事件后去读取
	u_int8_t stat_valid;	// 下面的状态是否有效。状态只在学习的时候，设备学到后返回才会赋值
	air_key_stat_t stat;	// 状态，对应开关机模式温度风量风向
}cl_no_screen_key_info;

//*************************************温度曲线数据结构********************************
//温度曲线flag标志含义
enum{
    ACT_TC_INVALID, //无效
    ACT_TC_OFF, //待机
    ACT_TC_COLD, //制冷
    ACT_TC_HOT, //制热
    ACT_TC_AUTO, //自动
    ACT_TC_AREFACTION,//除湿
    ACT_TC_WIND //送风
};
    
typedef  struct {
	u_int8_t  flag; //模式
	u_int8_t  tmp;//温度
	u_int8_t   wind; //风量
    u_int8_t dir; //风向
}tmp_curve_t;

typedef struct {
	u_int8_t id; //曲线id，用以支持多曲线
	u_int8_t enable;//是否使能该曲线
	u_int8_t week;//周期0-6位表示星期日到星期六
	u_int8_t begin_hour;//24小时中的开始时间，从0开始
	u_int8_t end_hour;//24小时中的结束时间
	u_int8_t time_period;//时间间隔，即分割1小时，如30，则将1小时分为2个节点，需要60能整除
	u_int8_t count;//由time_period,being_hour, end_hour算出的时间点数，算法:count=(end_hour - begin_hour + 1 + 24)%24*60/time_period
	u_int8_t pad;
	//tmp_curve_t tmp_curve[0];
}cl_temp_curve_t;

//***********************************根据室温控制空调*******************************************************
typedef struct {
	u_int8_t enable;
	u_int8_t mode;
	u_int8_t temp_min;
	u_int8_t temp_max;
	u_int8_t week;
	u_int8_t begin_hour;
	u_int8_t end_hour;
	u_int8_t pad;
}cl_temp_ac_ctrl_t;
    
typedef struct {
    u_int8_t is_same_onoff_code; //是否开关机同码
    u_int8_t is_same_fan; // 是否扫风同码
    u_int8_t is_fan_speed_opposite; //是否风速相反
    u_int8_t pad;
}cl_808_param_ajust_info;
    
typedef struct {
    u_int8_t is_valid;
    u_int8_t num;
    u_int8_t pad[2];
    u_int8_t data[24*6]; //144个点，10分钟一个
}cl_24hour_line;

enum{
    CHILD_LOCK_NONE = 0, //无锁
    CHILD_LOCK_ALL, //锁住所有按键
    CHILD_LOCK_NO_ONOFF //锁住除开关外的所有按键
};
    
typedef struct {
    u_int32_t phone_index; //分享出的手机序号
    u_int32_t phone_share_time;//分享出去的时间
    u_int32_t phone_operate_num;//被分享手机操作了多少次设备
    u_int32_t phone_last_operate_time; //被分享手机最后一次操作设备的时间
    u_int8_t phone_desc[16];//被分享手机的描述信息
}cl_share_record_t;
    
typedef struct {
    u_int8_t is_share_data_valid; //分享数据是否有效
    bool is_super_user;//是否是超级手机
    u_int8_t record_num; //分享记录条数
    u_int8_t v1_remain_days; //过渡期剩余时间
    u_int32_t cur_phone_index; //当前登陆手机index
    cl_share_record_t* records; //分享记录
}cl_share_info_t;

// 快捷开关机
typedef struct {
	u_int8_t enable; // 功能开关 0-关闭 1-开启
	u_int8_t onoff; // 开关机， 0-关机 1-开机
	
	u_int32_t time;	// local时间基值，秒数单位
	u_int32_t remain_time;//剩余时间，秒数单位，由app根据time,remain_time计算出到点执行时间
} cl_shortcuts_onoff_t;

typedef struct {
	u_int8_t onoff;	// 总开关
	u_int8_t timer;	// 定时器推送
	u_int8_t sync;	// 遥控器同步
	u_int8_t temp_ctrl;	//  智能恒温
	u_int8_t curve;	//  温度曲线
	u_int8_t sleep;	// 智能睡眠
	u_int8_t smart_on;	// 智能开机
	u_int8_t poweron_restore;	// 掉电后自动恢复开关
	u_int8_t linkage_ctrl;	// 联动控制
	u_int8_t resv[3];
} cl_ac_msg_config_t;


typedef struct{	 
	u_int32_t ck;	/*current k*/
	u_int32_t cad;	/*current ad*/
	u_int32_t cad2;	/*current ad2*/
	u_int32_t vk;	/*voltage k*/
	u_int32_t vb;	/*voltage b*/
}cl_air_pt_adkb_t;


//空调插座信息
typedef struct _cl_air_info_s{
    cl_handle_t handle; //设备句柄
	
    u_int8_t air_led_on_off; /*LED*/
    u_int8_t room_temp;/*室温*/
    u_int8_t smart_on_enable; /*智能开机*/
    u_int8_t smart_off_enable; /*智能关机*/
    
    u_int8_t smart_sleep_enable; /*智能睡眠*/
	u_int8_t is_match_code_valid;//是否已经匹配了编码，是否有效
    u_int8_t is_match_code; //是否已经匹配了编码
    u_int8_t is_old_air; // 是否是老空调
    u_int8_t is_support_led_color;
   //窗机控制面板
    u_int8_t current_pan_type; //面板类型
    u_int8_t is_support_switch_pan; //是否支持切换面板
    u_int8_t is_support_key_learn;	// 是否悟空上面支持按键学习功能，傲基方案
    u_int8_t is_work_stat_data_valid; //工作状态数据是否有效
    u_int8_t is_smart_on_data_valid; //智能开启数据是否有效


	u_int8_t is_support_learn_snapshort;	// 表面支持学习和快照功能，用于悟空上
    
    u_int8_t is_support_peroid_timer; //是否支持周期性时间
    u_int8_t is_support_temp_curve;		//是否支持温度曲线
    u_int8_t is_support_temp_ac_ctrl;//是否支持根据温度控制空调
    u_int8_t is_support_peroid_ext_timer; //是否支持周期性扩展定时器
    
    u_int8_t is_support_utc_temp_curve;		//是否支持温度曲线
    u_int8_t is_support_utc_temp_ac_ctrl;//是否支持根据温度控制空调
    u_int8_t has_utc_temp_curve_data;
    u_int8_t has_utc_temp_ctrl_data;
    
    u_int8_t is_support_param_ajust;//参数微调
    u_int8_t is_support_room_temp_ajust;//室温校正
    u_int8_t is_support_elec_ajust;//电量校正
    u_int8_t temp_humidity; //湿度，增强型悟空才有效
    
    int16_t env_room_temp_low; //最高校正室温
    int16_t env_room_temp_high; //最低校正室温
    int16_t  env_temp_ajust_value; //环境温度校正值 放大10倍，例：界面显示 -5-5度，传值 -50-50
    int16_t  elec_ajust_value; //电量校正系数 放大100倍 例:界面显示0.5-1.5，传值为 50-150
    
    u_int8_t is_support_child_lock;//是否支持童锁
    u_int8_t child_lock_value; //CHILD_LOCK_XX
    
    cl_24hour_line room_temp_line; // 24小时温度曲线
    cl_24hour_line humi_line; //24小时湿度曲线
    
    cl_808_param_ajust_info ajust_info;
    //增强型悟空支持电视和机顶盒信息
    cl_pair_rc_info priv_rc;
   //////////////////////////
    u_int32_t cur_power; //当前功率
    ad_data_t ad;//adjust
    u_int32_t cur_milli_power; //当前功率，毫瓦
    cl_air_elec_item_info total_elec; //总电量
    cl_air_elec_item_info period_elec; //阶段电量
    cl_air_elec_item_info last_on_elec; //上次开机电量
    cl_air_work_stat_t air_work_stat; //空调当前工作状态
    cl_air_timer_info_t air_timer_info; //空调定时器信息
    cl_elec_stat_info_t air_elec_stat_info; //空调电力统计信息
    cl_elec_days_stat_info elec_days_info; //电量统计之365天统计数据
    cl_smart_air_on_param_t smart_on_info; //智能开启参数
    cl_smart_air_off_param_t smart_off_info; //智能关闭参数
    cl_smart_air_sleep_param_t smart_sleep_info; //智能睡眠参数
    cl_ac_code_match_info_t last_match_info; //编码匹配信息
    cl_air_led_color_info led_color; //智能灯颜色
    cl_no_screen_key_info key_info;
    cl_share_info_t share_info;
	u_int8_t* requested_share_code;//超级手机请求的分享码
	//通用温度曲线数据
	int temp_curve_len;	//通用温度曲线数据长度，因为可以支持多个曲线，目前估计只用一个,len配合count可以解析出多个曲线
	cl_temp_curve_t *temp_curve;//通用温度曲线数据
	//根据温度控制空调
	cl_temp_ac_ctrl_t tac;
	u_int32_t id;
	//小电流开关
	u_int8_t scc_onoff_valid;
	u_int8_t scc_onoff;
	//电流电压校正值
	cl_air_pt_adkb_t adkb;
	u_int32_t smt_hard_ver; //单片机硬件版本

	// 推送配置
	u_int8_t is_support_msg_config;	// 支持配置推送
	cl_ac_msg_config_t msg_config;
}cl_air_info_t;

typedef struct {
    u_int8_t action; // 当前状态 AIR_CODE_XX
    u_int8_t is_cloud_matching; // 是否正在云匹配中，全匹配此字段无效
    u_int8_t cur_step; // 当前进行到第几步
    u_int8_t max_step; //总共多少步
    u_int8_t error; // 匹配出现错误
    u_int8_t flag; //匹配同码错误标等,注:只在匹配过程中有效。
}cl_air_code_match_stat_t;
    
typedef struct {
    u_int32_t cur_power; /*当前功率*/
}cl_air_real_time_data_t;

enum{
	ELEC_CLEAR_PERIOD,
	ELEC_CLEAR_LAST_ON,
	ELEC_CLEAR_TOTAL,
    ELEC_CLEAR_ALL_ELEC
};

//智能插座的电流电压频率
typedef struct{	 
	u_int32_t freq_current;
	u_int32_t k_current;
	u_int32_t freq_voltage;
	u_int32_t k_voltage;
}cl_plug_pt_adkb_t;



/***********通用定时器************/
//通用定时器类型定义，这个是跟app协商的类型
enum {
	//开关类(旧定时器页面简约)
	UT_TYPE_ON = 1, //定时开
	UT_TYPE_PERIOD_ONOFF = 3, //时间段 开始时开启，时间段结束时关闭
    
    
    /*新定时器页面只有简约关,高级开,阶段*/
    UT_TYPE_OFF = 2, //定时关
	UT_TYPE_ADVANCE_TIMER = 4,//时间段 开始时开启，时间段结束时关闭, 高级定时器,持续时间内报错温度模式不变化(针对中央空调)
	UT_TYPE_ON_ADVANCE_TIMER = 5,//定时开(高级)

    
	//恒温类
	UT_TYPE_COMM_TYPE_CONST_TMP = 6,//时间段 恒温定时器
	
	UT_TYPE_COMM_MAX = 100,
};

//定时器分类
enum {
	UT_TIMER_TYPE_ONOFF = 1,//开关定时器
	UT_TIMER_TYPE_TMP = 2,//温度类定时器等，以后用
};
/******************用大于100的type表示各自用自己的，小于100定义为通用********************************************/
//暖气阀定时器类型
enum {
	UT_TIMER_HT_ONOFF = 1,//暖气阀单点开定时器
	UT_TIMER_HT_DURATION = 3,//暖气阀时间段定时器
	UT_TIMER_HT_CONST_TMP = 6,//暖气阀恒温定时器
};

//空调贴定时器类型
enum {
	UT_TIMER_WKAIR_ON = 1,//单开
	UT_TIMER_WKAIR_OFF = 2,//单关
	UT_TIMER_WKAIR_DURATION = 3,//时间段
	UT_TIMER_WKAIR_ADVANCE_1 = 4,//时间段
	UT_TIMER_WKAIR_ADVANCE_2 = 5,//时间段
};

typedef struct {
	u_int8_t mode;
	int8_t tmp;
}cl_comm_timer_zykt_t;

//空调贴定时器参数
typedef struct {
	u_int8_t mode;
	int8_t tmp;
}cl_comm_timer_wkair_t;

//智皇电机定时器参数
typedef struct {
	u_int8_t location;
}cl_comm_timer_zhdj_t;

//linkon温控器定时器参数
typedef struct {
	u_int8_t run_mode;//运行模式,0-加热，1-制冷，2-换气
	u_int8_t wind_speed;//风速，0-低风，1-中风，2-高风
	u_int16_t tmp;//设置温度，350表示35度,范围50-350
	u_int8_t scene_mode;//情景模式，0-恒温，1-节能，2-离家
}cl_comm_timer_linkon_t;

//魔乐单火线定时器参数
typedef struct {
    u_int32_t on_off_stat; //注:高16位为mask，bit为1表示无效，后16位是值，bit为1表示开，//各路开关状态，第0bit表示第一路,0表示关，1表示开
}cl_comm_timer_dhxml_t;

//智皇单火线定时器参数
typedef struct {
	u_int8_t onoff;//bit表示，1为开，0为关
	u_int8_t mask;//bit表示，相应位为1表示有效
}cl_comm_timer_zhdhx_t;

typedef struct {
	//单点定时器设置温度
	u_int8_t tmp_int;//整数部分
	u_int8_t tmp_dec;//小数部分

	//时间段定时器设置开始温度，结束温度
	u_int8_t start_tmp_int;//整数部分
	u_int8_t start_tmp_dec;//小数部分
	u_int8_t end_tmp_int;//整数部分
	u_int8_t end_tmp_dec;//小数部分

	//恒温定时器最大，最小设置温度
	u_int8_t max_tmp_int;//整数部分
	u_int8_t max_tmp_dec;//小数部分
	u_int8_t min_tmp_int;//整数部分
	u_int8_t min_tmp_dec;//小数部分	
}cl_ct_heating_valve_t;

typedef struct {
	u_int8_t id;//定时器id
	u_int8_t enable;//定时器是否使能
	u_int8_t type;//定时器，类型，这个很重要，app，sdk靠它来判断定时器的实际逻辑业务
	u_int8_t hour;
	u_int8_t min;
	u_int8_t week;//Week: 如果是周期循环定时器，bit7为1，bit0-6代表星期天、星期一到星期六，表示每星期那几天生效,如果是一次性定时器，bit7为0，bit0-6代表星期天、星期一到星期六，表示哪一天生效
	u_int16_t duration;//时间段定时器持续时间
	
	u_int8_t extened_len;//后面扩展数据长度，app好像可以不用
	u_int8_t valid;//这个是sdk内部使用

	u_int32_t min_time;//下次执行定时时间，sdk专用
	u_int8_t week_cal;//sdk专用，用来计算下次执行定时器的
	union {
		u_int8_t data[10];
		cl_comm_timer_zykt_t zykt_timer;
		cl_comm_timer_wkair_t wkair_timer;
		cl_ct_heating_valve_t hv_timer;
		cl_comm_timer_zhdj_t zhdj_timer;//智皇电机，智皇窗帘用的扩展数据
		cl_comm_timer_linkon_t linkon_timer;
		cl_comm_timer_dhxml_t dhxml_timer;//魔乐单火线
		cl_comm_timer_zhdhx_t zhdhx_timer;//智皇单火线
    }extended_data_u;
}cl_comm_timer_t;

typedef struct {
	u_int8_t ip[16];	// 字符串
	u_int8_t mask[16];	// 字符串
	u_int8_t getway_ip[16];	// 字符串
	u_int8_t main_dns[16];	// 字符串
	u_int8_t sub_dns[16];	// 字符串，可选，如果用户不填这里全填0
} cl_wan_static_t;

typedef struct {
	u_int8_t main_dns[16];	// 字符串
	u_int8_t sub_dns[16];	// 字符串，可选，如果用户不填这里全填0
	u_int8_t ip[16];	// 字符串，显示用
	u_int8_t mask[16];	// 字符串，显示用
	u_int8_t getway_ip[16];	// 网关IP，显示用
} cl_wan_dhcp_t;

typedef struct {
	u_int8_t name[32];		// 账户
	u_int8_t pwd[32];		// 密码
	u_int8_t main_dns[16];	// 字符串
	u_int8_t sub_dns[16];	// 字符串，可选，如果用户不填这里全填0
	u_int8_t ip[16];	// 字符串，显示用
	u_int8_t peer_ip[16];	// 字符串，对端IP，显示用
} cl_wan_pppoe_t;

// 网口的某个具体配置
typedef struct {	
	u_int8_t network_type;	// 0 没有配置 1 静态IP 2 DHCP 3 PPPOE
	u_int8_t pad[3];
	union {
		cl_wan_static_t config_static;	// 如果wan_type 为1，用这个接口
		cl_wan_dhcp_t config_dhcp;		// 如果wan_type 为2，用这个接口
		cl_wan_pppoe_t config_pppoe;	// 如果wan_type 为3，用这个接口
		u_int8_t config_pad[128];		// 无视
	} config;
} cl_wan_config_item_t;

// 网口的信息
typedef struct {
	u_int8_t index;		// 下标，第几个WAN口
	u_int8_t wan_phy_type;	// 网口类型 1 有线 2 . 2.4G无线，对于设置可以不管
	u_int8_t select_network_type;	// 选择的配置0-未配置  1-静态IP  2-DHCP  3-pppoe
	u_int8_t pad;
	cl_wan_config_item_t config_item[3];	// 具体的配置
} cl_wan_phy_item_t;

typedef struct {
	u_int8_t wan_num;	// 有几个网口可以配置(有线、无线)
	u_int8_t pad[3];
	cl_wan_phy_item_t config[2];	// 最多2个网口
} cl_wan_config_t;

// 配置网口用
typedef struct {
	u_int8_t index;	// 网口下标
	u_int8_t pad;
	u_int8_t wan_type;	// 需要配置的类型1-静态IP  2-DHCP  3-pppoe
	u_int8_t pad1;
	union {
		cl_wan_static_t config_static;	// 如果wan_type 为1，用这个接口
		cl_wan_dhcp_t config_dhcp;		// 如果wan_type 为2，用这个接口
		cl_wan_pppoe_t config_pppoe;	// 如果wan_type 为3，用这个接口
		u_int8_t config_pad[128];		// 无视
	} config;
} cl_wan_request_config_t;

typedef struct {
	u_int8_t getway_ip[16];	// 网关IP
	u_int8_t start_ip[16];	// 开始IP
	u_int8_t end_ip[16];	// 结束IP
	u_int8_t mask[16];	// 掩码
	u_int32_t time;	// 租期，单位分钟
} cl_dhcp_server_config_t;

typedef struct {
	u_int8_t ssid[36];
	u_int8_t pwd[68];
	u_int8_t is_enc;	// 是否加密0 不加密 1 加密(WAP2)
	u_int8_t pow;		// 信号强度	1 环保模式 2 普通模式 3 穿墙模式
	u_int8_t channle_mode;	// 信道选择模式 0 自动选择 1 手动选择
	u_int8_t channel;	// 信道1-13
	u_int8_t enable;	// 开关 
} cl_ap_config_t;
/*
app定时器显示逻辑:

timer_next_count 有值的情况下证明有下次执行定时器显示，至于是在app上的哪个界面显示就根据界面逻辑来了,
比如，打算在主控制界面显示开关定时器，就要把所有下次定时器遍历一下，找出type=
UT_TYPE_ON 
UT_TYPE_OFF
UT_TYPE_PERIOD_ONOFF 
这三类定时器来显示

根据这种类型来判断改如何显示时间:
	
type = 	UT_TYPE_ON						UT_TYPE_OFF					UT_TYPE_PERIOD_ONOFF 这个可以显示开关
		如果next_start_valid=1			如果next_start_valid=1			如果next_start_valid=1	如果next_finish_valid=1
		显示下次开						显示下次关						显示下次开				显示下次关
		next_start_day					next_start_day					next_start_day			next_finish_day
		next_start_hour					next_start_hour					next_start_hour			next_finish_hour
		next_start_min					next_start_min					next_start_min			next_finish_min

这里理论上只会有其中一种(设置时做时间冲突检查)

其他类型，以后再说 	

*/
/*
	老版本备份
    u_int8_t timer_next_count; //下次要执行的有多少个定时器
    u_int8_t next_start_valid;//下次开始定时器时间是否有效
    u_int8_t next_start_day;
    u_int8_t next_start_hour;
    u_int8_t next_start_min;
	
    u_int8_t next_finish_valid;//下次结束定时器时间是否有效
    u_int8_t next_finish_day;
    u_int8_t next_finish_hour;
    u_int8_t next_finish_min;
    u_int8_t id[32];//下一次定时器的id数组,为1表示有效,设备端不支持超过32个定时器

	u_int32_t min_time;//sdk专用
	u_int8_t type;//sdk专用
	u_int8_t id_tmp;//sdk专用


*/
typedef struct {
    u_int8_t next_valid;
    u_int8_t next_day;
    u_int8_t next_hour;
    u_int8_t next_min;
}next_exec_time_t;

typedef struct {
	bool comm_timer_valid;//为真时才表示下面的数据有效
	//这里为了表现通用定时器的概念太麻烦了，还是修改下，下次关定时器，下次开定时器，或者下次设置温度曲线什么的
	//下次执行开机定时器的时间
    next_exec_time_t next_on;
	//下次执行关机定时器的时间
	next_exec_time_t next_off;

	//下次温度曲线开始设置时间什么的
	next_exec_time_t next_temp_start;
	//下次温度曲线结束设置时间什么的
    next_exec_time_t next_temp_finish;

	//以后需要什么添加什么吧，要不然没法搞成通用的了。。。。。。。。。。。。。。。。

	u_int32_t min_time_start;//sdk专用
	u_int32_t min_time_finish;//sdk专用
	u_int32_t min_time;//sdk专用
	bool is_slave;//sdk专用
	u_int8_t sub_type;//sdk专用
	u_int8_t ext_type;//sdk专用
	
	u_int8_t timer_count;
	u_int8_t real_count;
	cl_comm_timer_t *timer;
}cl_comm_timer_head_t;

/*
备注:
support_type支持类型是表示设备端支持的类型，目前定时器在udp_ctrl.h文件，有3种
enum {
	UT_DEV_TYPE_ONOFF = 1, //开关定时器
	UT_DEV_TYPE_PERIOD_CONSTANT_TEMP = 2, //恒温
	UT_DEV_TYPE_PERIOD_TEMP_CURVE = 3, //温度曲线
	UT_DEV_TYPE_MAX
};

*/
typedef struct {
	u_int8_t max_timer_count; //app用
	u_int8_t max_type_count;//app用,这里是为了凯特兼容的，以后用后面的support_type
	u_int8_t max_data_len;//app用
	u_int16_t support_type;//app用，bit0-bit15分别表示支持type1-type16的类型

	//下面是sdk专用，app不用管
	u_int8_t reserve;
	u_int32_t map;
	u_int64_t sn;
	u_int32_t need_query;
	u_int16_t *stat_count;
}cl_dev_timer_summary_t;

/***********************/

typedef struct {
	u_int32_t light;
	u_int32_t temp;
	u_int32_t power;
	u_int32_t humidity;
}cl_air_debug_info_set_t;


/*
 功能:
    控制老空调
 输入参数:
    @key
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_old_air_ctrl(cl_handle_t dev_handle, u_int8_t key_id);

/*
 功能:
    控制空调
 输入参数:
    @stat, 各参数，如果是0，则使用当前状态值
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_air_ctrl(cl_handle_t dev_handle, cl_air_work_stat_t* stat);

/*
 功能:
    控制空调开关机
 输入参数:
    @onoff, false:OFF true:ON
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_air_ctrl_power(cl_handle_t dev_handle, bool onoff);
    
/*
 功能:
    控制空调
 输入参数:
    @mode: CL_AC_MOD_XXX
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_air_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
 功能:
    控制空调温度
 输入参数:
    @temp： 温度: 16-30
 输出参数:
 无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_air_ctrl_temp(cl_handle_t dev_handle, u_int8_t temp);
    
/*
功能:
    控制空调风力
输入参数:
    @wind: 风力 CL_AC_WIND_XX
输出参数:
    无
返回:
 RS_OK: 成功
其他: 失败
*/
CLIB_API RS cl_sa_air_ctrl_wind(cl_handle_t dev_handle, u_int8_t wind);

/*
 功能:
    控制空调风力
 输入参数:
    @wind: 风力 CL_AC_DIRECT_XX
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_air_ctrl_direct(cl_handle_t dev_handle, u_int8_t direct);

/*
 功能:
    控制空调LED灯
 输入参数:
    @onoff: 0:关闭，1：开启，2：智能
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_ctrl_led_power(cl_handle_t dev_handle, u_int8_t onoff);
    
/*
 功能:
    设置空调峰电计量时间
 输入参数:
    @begin_time: 开始时间 单位：分钟  0： 0点0分  1439：23时59分
    @last_time:  持续时间 单位：分钟  max： 1440
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_air_peak_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
/*
 功能:
    设置空调谷电计量时间
 输入参数:
    @begin_time: 开始时间 单位：分钟  0： 0点0分  1439：23时59分
    @last_time:  持续时间 单位：分钟  max： 1440
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_air_valley_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
    
/*
 功能:
    设置空调峰电价格
 输入参数:
    @price: 单位：分
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_air_peak_price(cl_handle_t dev_handle, u_int32_t price);

/*
 功能:
    设置空调谷电价格
 输入参数:
    @price: 单位：分
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_air_valley_price(cl_handle_t dev_handle, u_int32_t price);

/*
 功能:
    设置空调平电价格
 输入参数:
    @price: 单位：分
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_air_flat_price(cl_handle_t dev_handle, u_int32_t price);
/*
 功能:
    添加空调定时器
 输入参数:
    @time_info: 定时器信息 id若存在，则为修改
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_add_air_timer(cl_handle_t dev_handle, cl_air_timer_t* time_info);

/*
 功能:
    添加空调周期定时器
 输入参数:
    @time_info: 定时器信息 id若存在，则为修改
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_add_air_period_timer(cl_handle_t dev_handle, cl_period_timer_t* time_info);

/*
 功能:
    删除空调定时器
 输入参数:
    @timer_id : 定时器ID
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_del_air_timer(cl_handle_t dev_handle, u_int8_t timer_id);


/*
 功能:
    删除空调定时器
 输入参数:
    @timer_id : 定时器ID
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_del_period_timer(cl_handle_t dev_handle, u_int8_t timer_id);
    
/*
 功能:
    开始编码匹配
 输入参数:
    @all_match true：全匹配  false:云匹配
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_start_code_match(cl_handle_t dev_handle, bool all_match,u_int32_t timeout);
    
/*
 功能:
    停止编码匹配
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_stop_code_match(cl_handle_t dev_handle);

/*
 功能:
    获取当前编码匹配状态
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    @stat: 状态值
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_get_code_match_stat(cl_handle_t dev_handle,cl_air_code_match_stat_t* stat);

/*
 功能:
    设置智能开机
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_smart_power_on(cl_handle_t dev_handle,bool enable);

/*
 功能:
    设置智能关机
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */

CLIB_API RS cl_sa_set_smart_power_off(cl_handle_t dev_handle,bool enable);

/*
 功能:
    设置智能睡眠
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_smart_sleep(cl_handle_t dev_handle,bool enable);

/*
 功能:
    获取
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    @rdata 实时数据
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_air_get_real_time_data(cl_handle_t dev_handle,cl_air_real_time_data_t* rdata);

/*
 功能:
    获取空调当前功率
 输入参数:
    @time_interval
 输出参数:
    无
 返回:
    RS_OK: 成功
 */
CLIB_API RS cl_sa_air_start_get_cur_power(cl_handle_t dev_handle,u_int8_t time_interval);
    
/*
 功能:
    停止获取空调当前功率
 输入参数:
    @time_interval
 输出参数:
    无
 返回:
    RS_OK: 成功
 */
CLIB_API RS cl_sa_air_stop_get_cur_power(cl_handle_t dev_handle);

/*
 功能:
    设置智能开机详细参数
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_smart_power_on_detail(cl_handle_t dev_handle,cl_smart_air_on_param_t* ao);

/*
 功能:
 设置智能关机详细参数
 输入参数:
 @dev_handle 设备句柄
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */

CLIB_API RS cl_sa_set_smart_power_off_detail(cl_handle_t dev_handle,cl_smart_air_off_param_t* af);

/*
 功能:
    设置智能睡眠详细参数
 输入参数:
    @dev_handle 设备句柄
    @as
 输出参数:
 无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_set_smart_sleep_detail(cl_handle_t dev_handle,cl_smart_air_sleep_param_t* as);

/*
 功能:
    刷新定时器信息
 输入参数:
    @dev_handle 设备句柄
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_sa_air_refresh_timer_info(cl_handle_t dev_handle);

/*
 功能:
    刷新电量统计信息
 输入参数:
    @dev_handle 设备句柄
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_sa_air_refresh_elec_info(cl_handle_t dev_handle);

/*
 功能:
    刷新电量统计信息
 输入参数:
    @dev_handle 设备句柄
    @type: 0x0: 清空周期统计信息
               0x1: 清空最近开机的电量统计信息；
               0x2: 清空累计统计信息
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_sa_air_clear_elec_stat_info(cl_handle_t dev_handle,int type);

/*
 功能:
    设置led 灯颜色
    @ on_color  空调开启时灯的颜色
    @off_color  空调关闭时灯的颜色

 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_sa_air_set_led_color(cl_handle_t dev_handle,int on_color,int off_color);

/*
	透传串口命令到单片机
	注意: 产测程序用的
*/
#define MAX_TRANS_LEN 1024
CLIB_API RS cl_sa_air_trans_send(cl_handle_t dev_handle, u_int8_t *buf, int len);

/*
	获取设备透传给客户端的红外编码
	SAE_TT_IR事件时调用本函数
	返回:红外编码长度
	
*/
CLIB_API int cl_sa_get_trans_ir(cl_handle_t dev_handle, u_int8_t *buf, int len);

/*
	获取设备透传给客户端的声音
	SAE_TT_SOUND事件时调用本函数
	返回:声音编码长度，大于0成功，小于等于0失败
	
*/
CLIB_API int cl_sa_get_trans_sound(cl_handle_t dev_handle, u_int8_t *buf, int len);

/*
	恢复出厂
	注意: 产测程序用的，慎用
*/
CLIB_API RS cl_sa_air_restore_factory(cl_handle_t dev_handle);

/*
	设置电压电流校准值
	注意: 产测程序用的，慎用
*/
CLIB_API RS cl_sa_pt_set_adkb(cl_handle_t dev_handle,  cl_air_pt_adkb_t *v);

/*
	设置电压电流校准值
	注意: 产测程序用的，慎用
*/
CLIB_API RS cl_sa_pt_set_adkb_ext(cl_handle_t dev_handle,  cl_air_pt_adkb_t *v);

/*
	设置电压电流校准值(V7单点校正)
	注意: 产测程序用的，慎用
*/
CLIB_API RS cl_sa_pt_set_adj(cl_handle_t dev_handle,  cl_plug_pt_adkb_t *v);


/*
 功能:切换已学习的编码
	@new_code_id; 切换至新的id
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_sa_air_reset_ir_code_id(cl_handle_t dev_handle,  u_int16_t new_code_id);


///////////////////////////////////////////////////////////////
// 窗机控制面板接口
/*
	设置面板类型 pan_type 见AC_PAN_WINDOW等定义
*/
CLIB_API RS cl_ac_set_pan_type(cl_handle_t dev_handle,u_int8_t pan_type);

/*
 功能:刷新所有按键信息
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_refresh_key_info(cl_handle_t dev_handle);

/*
	 功能:设置按键信息
	@key_id,若设备没有该id，则视为添加，否则视为修改
	@key_name,按键名称15字节
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_set_key_info(cl_handle_t dev_handle,u_int8_t key_id,char* key_name);

/*
	 功能:设置按键信息
	@key_id,若设备没有该id，则视为添加，否则视为修改
	@key_name,按键名称15字节
	@flag: 标志，目前使用bit 4: 设置KEY为快照 bit5: 设置KEY支持状态解析
	@air_stat: 当flag置上快照模式时候,air_stat带上状态。状态来自学习成功后返回的air_info->key_info->stat
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_set_key_info_v2(cl_handle_t dev_handle, u_int8_t key_id, char* key_name, u_int8_t flag, air_key_stat_t air_stat);

/*
	 功能:删除按键
	@key_id, 已创建的key id
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_delete_key(cl_handle_t dev_handle,u_int8_t key_id);

/*
	 功能: 开始按键编码学习
	@key_id, 已创建的key id
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_start_learn_key(cl_handle_t dev_handle,u_int8_t key_id);

/*
	 功能: 停止按键编码学习
	@key_id, 已创建的key id
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_stop_learn_key(cl_handle_t dev_handle,u_int8_t key_id);

/*
	 功能: 按键控制
	@key_id, 已创建的key id
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_ctrl_key(cl_handle_t dev_handle,u_int8_t key_id);

//************************************************************红外同码错误处理接口**************************************************************
/*
	 功能: 查询是否有同码
    输出参数:	@status, 查询同码状态，1表示同码。
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_query_samecode(cl_handle_t dev_handle,u_int8_t *status);

/*
	 功能: 设置空调开关状态值
    输出参数:	@on, 开关设置状态，0关闭，1开启。
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_modify_onoff_status(cl_handle_t dev_handle,u_int8_t on);

//*********************************************************温度曲线处理接口****************************************************************************************
/*
 功能: 添加修改温度曲线配置
 输出参数:	@ptmp_curve,温度曲线配置参数
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_sa_modify_temp_curve(cl_handle_t dev_handle,cl_temp_curve_t *ptmp_curve);

//***********************************根据室温控制空调*******************************************************
/*
 功能: 添加设置根据温度控制空调接口
 输出参数:	@ptmp_ctrl,配置参数
 无
    返回:
 RS_OK: 成功
    其他: 失败
*/
CLIB_API RS cl_ac_set_temp_ctrl(cl_handle_t dev_handle,cl_temp_ac_ctrl_t *ptmp_ctrl);
    
/*
 功能: 设置空调风速是否反了，检测到同码才能调用此函数
 输出参数:	is_opposite 0: 正常，1：反了
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ac_set_fan_speed_opposite(cl_handle_t dev_handle,u_int8_t is_opposite);
    
/*
 功能: 设置空调扫风工作状态
 输出参数:	is_opposite 0: 扫风关闭，1：扫风开启
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_ac_set_fan_stat(cl_handle_t dev_handle,u_int8_t fan_stat);

/*
功能: 
    设置童锁状态
    @lock_stat: CHILD_LOCK_XX
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_ac_set_child_lock_stat(cl_handle_t dev_handle,u_int8_t lock_stat);

/*
功能: 
    设置I8消息推送开关
    @config: 配置
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_ac_set_msg_config(cl_handle_t dev_handle, cl_ac_msg_config_t *config);

/*
功能: 
    通用定时器设置
    @ptimer: 定时器参数
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_misc_comm_timer_modify_add(cl_handle_t dev_handle, cl_comm_timer_t *ptimer);

/*
功能: 
    通用定时器设置
    @id: 定时器id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_misc_comm_timer_del(cl_handle_t dev_handle, u_int8_t id);


/*
 功能:
    通用的设置智能开机，查询在
    air_ctrl->com_udp_dev_info.smart_on
 输入参数:
    @dev_handle 设备句柄
    @enable: 是否使能
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_smart_on(cl_handle_t dev_handle, u_int8_t enable);

/*
 功能:
    通用的设置童锁，查询在
    air_ctrl->com_udp_dev_info.child_lock_value
 输入参数:
    @dev_handle 设备句柄
    @type: 童锁类型:0-关闭童锁 1-锁所有误操作 2-锁开关操作
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_child_lock(cl_handle_t dev_handle, u_int8_t type);


/*
 功能:
    通用的设置开机温度
    air_ctrl->com_udp_dev_info.boot_temp
 输入参数:
    @dev_handle 设备句柄
    @enable: 是否开启
    @temp: 开机温度，摄氏度
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_boot_temp(cl_handle_t dev_handle, u_int8_t enable, u_int8_t temp);

/*
 功能:
    通用的设置温度阀值
    air_ctrl->com_udp_dev_info.temp_alarm_xxx
 输入参数:
    @dev_handle 设备句柄
    @onoff: 开关
    @min: 最小温度
    @max: 最大温度
    
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_temp_alarm(cl_handle_t dev_handle, u_int8_t onoff, u_int8_t min, u_int8_t max);

/*
 功能:
    通用的设置童锁，查询在
    air_ctrl->com_udp_dev_info.shortcuts_onoff
 输入参数:
    @dev_handle 设备句柄
    @enable: 是否使能
    @onoff: 0: 关机 1开机
    @time: 剩余时间，秒数
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_shortcuts_onoff(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int32_t time);

/*
 功能:
    通用的设置恒温
    air_ctrl->com_udp_dev_info.tac
 输入参数:
    @dev_handle 设备句柄
    @ptmp_ctrl: 控制参数
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_temp_ctrl(cl_handle_t dev_handle, cl_temp_ac_ctrl_t *ptmp_ctrl);

/*
 功能:
    通用的设置温度曲线
    air_ctrl->com_udp_dev_info.temp_curve
 输入参数:
    @dev_handle 设备句柄
    @ptmp_ctrl: 控制参数
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_modify_temp_curve(cl_handle_t dev_handle, cl_temp_curve_t *ptmp_curve);

/*
 功能:
    通用的查询WIFI设备的历史记录
 输入参数:
    @dev_handle 设备句柄
    @index: 起始ID
    @num: 查询的个数
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 备注: 当收到SAE_DEV_COMM_HISTORY_ITEM = 1242，表示查询回来结果，从
                com_udp_dev_info.dev_history去读取数据
 */
CLIB_API RS cl_sa_public_history_query(cl_handle_t dev_handle, u_int32_t index, u_int32_t num);

/*
 功能:
    通用的设置设备外网口配置
    air_ctrl->com_udp_dev_info.wan_config
 输入参数:
    @dev_handle 设备句柄
    @config: 配置信息
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_wan_config(cl_handle_t dev_handle, cl_wan_request_config_t *config);

/*
 功能:
    通用的设置设备DHCP服务器
    air_ctrl->com_udp_dev_info.dhcp_config
 输入参数:
    @dev_handle 设备句柄
    @config: 配置信息
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_dhcp_server_config(cl_handle_t dev_handle, cl_dhcp_server_config_t *config);

/*
 功能:
    通用的设置设备AP
    air_ctrl->com_udp_dev_info.ap_config
 输入参数:
    @dev_handle 设备句柄
    @config: 配置信息
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_ap_config(cl_handle_t dev_handle, cl_ap_config_t *config);
/*
 功能:
    通用的设置设备中继器开关
    air_ctrl->com_udp_dev_info.repeat_config
 输入参数:
    @dev_handle 设备句柄
    @onoff: 开关
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_repeat_onoff(cl_handle_t dev_handle, u_int8_t onoff);

#ifdef __cplusplus
}
#endif 

#endif

