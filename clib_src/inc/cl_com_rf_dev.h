#ifndef CL_COM_RF_DEV_H
#define CL_COM_RF_DEV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//枚举值还是写上值吧，不然有时候需要查询值的时候老要慢慢数
typedef enum {
    D_T_UNKNOWN = 0,
    D_T_LAMP = 1, // RGB 彩色灯
    D_T_DOOR_LOCK = 2, //门锁
    D_T_DOOR_MAGNET = 3,//门磁
    D_T_DHX_SWITCH = 4, //单火线开关
    D_T_YT_DOOR_LOCK = 5, //友泰门锁
    D_T_HM_MAGNET = 6,//海曼门磁
    D_T_HM_BODY_DETECT = 7,//海曼人体探测
    D_T_HM_TEMP_HM = 8,//海曼温湿度探测
    D_T_KTCZ = 9,	// 凯特插座
    D_T_HEATING_VALVE = 10,	// 暖气阀
    D_T_GAS = 11,		// 气体检测
    D_T_QSJC = 12,		// 水检测
    D_T_HTLLOCK = 13,	// 汇泰龙锁
    D_T_HMCO = 14,	// 一氧化碳检测
    D_T_HMYW = 15,	// 烟雾检测
    D_T_DWYK = 16,	//电微遥控器
    D_T_WK_AIR = 17, //悟空空调贴
    D_T_DOOR_MAGNETV2 = 18,	// V2版本门磁
    D_T_SCENE_CONTROOLER = 19,	// 情景遥控器
	D_T_DWHF = 20,	//电威合封芯片灯
	D_T_HMQJ = 21,	// 海曼求救器  
	D_T_ZHDJ = 22, //智皇电机
	D_T_DWKJ = 23,	//  电王科技
	D_T_YLTC = 24,// 夜狼红外
	D_T_YLWSD = 25,//夜狼温湿度
	D_T_YLSOS = 26,//夜狼sos
	D_T_YLLOCK = 27,//夜狼门磁
	D_T_YLLIGHT = 28,	// 夜狼声光报警器
	D_T_DHXZH_SWITCH = 29,	// 智皇单火线
	D_T_DHXCP_SWITCH = 30,	// 橙朴单火线
	D_T_DWYKHF = 31,//电微合封遥控器
	D_T_DWYSTGQ = 32,	// 电威音速调光器
	D_T_WK_AIR2 = 33, //空调贴2
	D_T_YSD = 34, //音速灯
	D_T_CDQJMB = 35, //橙灯情景面板
	D_T_JQ = 36,	// 甲醛传感器
	D_T_MLDHX = 37,//魔乐单火线
	D_T_LIGHT_SENSE = 38,// 光照传感器
	// 中间不要再加了
	D_T_LAMP_START = 39,	// 电威灯拓展类型起始
	D_T_LAMP_END = 54,		// 电威灯拓展类型结束
	// 中间不要再加了
	D_T_LHX = 55,	// 零火线
	D_T_WUANS6 = 56,	// 悟安S6人体红外
	
	D_T_MAX,
}cl_slave_data_type;
    
    enum{
        LAMP_CTRL_MODE_SWITCH = 0x1,
        LAMP_CTRL_MODE_LAYER
    };

#pragma pack(push,1)


///////////////////////////////////////////////////
//灯
    
typedef struct {
    u_int8_t R;
    /*红 0~255*/
    u_int8_t	B;
    /*绿0-255*/
    u_int8_t 	G;
    /*蓝0~255*/
    u_int8_t	L;
    /*亮度0~100*/
    u_int8_t cold;
    /*色温*/
    u_int8_t	power;
    /*开关，0为关，1为开*/
    u_int8_t	mod_id; //情景模式编号
    /*模式id*/
    u_int8_t	action; //0:设置RGB 1:设置色温
    //行为
    u_int8_t ctrl_mode; //0，控制灯 1：控制开关，2：控制层
    u_int8_t value;
	u_int8_t flag;
    u_int8_t pad;
}cl_rf_lamp_stat_t;

#define MAX_DW_LAMP_REMOTE_CNT 4
#define MAX_DW_LAMP_KEY_ID 8
//电威遥控器相关
// key_id 1-4
typedef struct {
    u_int32_t remote_id;
    u_int8_t key_id;
    u_int8_t pad[3];
}cl_rf_lamp_remote_info_t;


enum {
	LED_UI_WC_UPPER = 8, //WC, WC分层上层
	LED_UI_WC_LOWER,     //WC分层下层
	LED_UI_WC_DOUBLE,    //WC分层两层
	LED_UI_RGB,          //RGB色板，RGB情景模式页面
	LED_UI_SMART_SWITCH, //智能开关页面
	LED_UI_POWER_SWITCH, //电源开关按钮
	LED_UI_WHITE_ONLY,	// 专门控制白光
};

enum{
    RL_LT_WC_OR_RGB = 0x1,
    RL_LT_WC_ONLY = 2,
    RL_LT_WC_AND_RGB = 3,
    RL_LT_LAYER = 4,
    RL_LT_SWITCH = 5,
    RL_LT_C_ONLY = 6,
    RL_LT_C_OR_RGB = 7,
    RL_LT_RGB_ONLY = 8,
    RL_LT_RGB_BELT = 9,	// RGB灯带
    RL_LT_MAX
};
    
typedef struct {
    cl_rf_lamp_stat_t stat;
    cl_rf_lamp_remote_info_t r_info[MAX_DW_LAMP_REMOTE_CNT];
    u_int8_t remote_count; //属于几个遥控器
    u_int8_t is_support_color_temp; //是否支持色温调节
    u_int8_t is_support_rgb; //是否支持rgb调节
    u_int8_t lamp_type; // 0 支持RGB和色温，两者互斥，1:仅支持色温 2：支持RGB和色温，两者并存
}cl_rf_lamp_t;

//////////////////////////

////////////
// 夜狼声光报警器
enum {
	ACT_RF_YLLIGHT_MOD= 1,		// 模式0 照明 1 警笛
	ACT_RF_YLLIGHT_SOUND,	// 控制鸣笛0 停止1开始

	// 下面SDK自己用
	ACT_RF_YLLIGHT_LAMP_CTRL = 100,	
	ACT_RF_YLLIGHT_ALARM_CONFIG,	// 设置默认报警时间
};

enum {
	TLV_YLLIGHT_MODE_SET = 210,
	TLV_YLLIGHT_SOUND = 211,
	TLV_YLLIGHT_ALARM_CONFIG = 212,
	TLV_YLLIGHT_LAMP_SET = 213,
	TLV_YLLIGHT_QUERY = 214,
};

typedef struct {
    u_int8_t R;
    u_int8_t G;
    u_int8_t B;
    u_int8_t W;
    u_int8_t C;
    u_int8_t power;
    u_int8_t mod_id;
    u_int8_t o_wc_l;
    u_int8_t o_r;
    u_int8_t o_g;
    u_int8_t o_b;
    u_int8_t o_l;
    u_int8_t o_c;
    u_int8_t hwconf;
} ucp_yllight_stat_t;

typedef struct {
	cl_rf_lamp_t lamp_stat;	// 灯相关信息
	u_int8_t mode;	// 0 照明模式 1 警笛模式
	u_int8_t is_alarm;	// 警笛模式下表示正在报警
	u_int8_t is_dynamic;	// 警笛模式下表示报警是否为动态模式
	//u_int8_t duration;	// 报警持续时间 1-180秒

		/*
	报警方式：
	    0 -- 持续报警（永久报警）
	    1 -- 报警一次，此时仅total_time有效
	    2 -- 间歇报警，on_time和off_time必须为非0，表示一个周期内的开启和停止时间
	        total_time为0，表示按照上述周期永久性报警，非0则表示在报警total_time以后停止 
	*/
	u_int8_t alarm_mode;
	u_int8_t on_time;	// 一个周期内的开启时间，单位秒，范围0-255
	u_int16_t off_time;	// 一个周期内的停止时间，单位秒，范围0-65535
	u_int16_t total_time;	// 报警时间总长度，单位分钟，范围0-65535
	// 下面数据SDK用
	bool get_cache;
} cl_yllight_info_t;


typedef struct {
	u_int8_t flagbits;
	u_int8_t hislog_count;
	/*
	报警方式：
	    0 -- 持续报警（永久报警）
	    1 -- 报警一次，此时仅total_time有效
	    2 -- 间歇报警，on_time和off_time必须为非0，表示一个周期内的开启和停止时间
	        total_time为0，表示按照上述周期永久性报警，非0则表示在报警total_time以后停止 
	*/
	u_int8_t alarm_mode;
	u_int8_t on_time;	// 一个周期内的开启时间，单位秒，范围0-255
	u_int16_t off_time;	// 一个周期内的停止时间，单位秒，范围0-65535
	u_int16_t total_time;	// 报警时间总长度，单位分钟，范围0-65535
	u_int32_t hislog_index_current;
	//ucp_yllight_stat_t stat;
} yllight_cache_t;

// 报警时间,下标0开始，如果有值，分别表示
// 开始、结束时间，间隔时间
typedef struct {
	u_int32_t time[4];
} cl_alarm_time_t;    

typedef struct {
	u_int8_t alarm_mode;
	u_int8_t on_time;
	u_int16_t off_time;
	u_int16_t total_time;
} cl_yllight_alarm_config_t;

//////////////////////////
//门锁

//门锁控制类型
enum{
    RDL_CTRL_OPEN = 0x1, //控制锁开关
    RDL_CTRL_GRARD //控制布防撤防
};

// 门锁私有的TLV TYPE 
enum {
	DOOR_LOCK_PRIV_TYPE_QUERY_WIFI_LOCK = 212,	//  查询WIFI开关锁信息
	DOOR_LOCK_PRIV_TYPE_SET_WIFI_LOCK = 211, //设置接入WIFI自动开锁,断开WIFI自动关锁
	DOOR_LOCK_PRIV_TYPE_SET_UNLOCK_TIMEOUT = 213, //设置未关锁超时时间
	DOOR_LOCK_PRIV_TYPE_SET_DISTINGUISH_MODE = 214,	// 进入遥控器识别模式的设置和回复
	DOOR_LOCK_PRIV_TYPE_PUSH_CONTROLLER_ID = 215,	// 上报遥控器ID
	DOOR_LOCK_PRIV_TYPE_SET_CONTROLLER_INFO = 216,//遥控器名称设置
	DOOR_LOCK_PRIV_TYPE_QUERY_CONTROLLER_INFO = 217,//遥控器信息查询

	DOOR_LOCK_PRIV_TYPE_ASSOCIATE = 220,	// 友泰关联锁体
};

//门锁遥控器状态类型
enum{
    DOOR_LOCK_REMOTECONTROLLER_NORAML = 0x1, //正常
    DOOR_LOCK_REMOTECONTROLLER_REPORT_LOSS = 0x2, //挂失
    DOOR_LOCK_REMOTECONTROLLER_DELETE = 0x3, //删除
};

typedef struct {
	u_int32_t flagbits;
	u_int32_t hislog_index_current;
	u_int8_t max_hislog_count;
	u_int8_t battery;
	u_int8_t ass_state;
	u_int8_t v_alarm;
} yt_door_lock_tt_cache_t;


typedef struct {
	u_int32_t flagbits;
	u_int32_t hislog_index_current;
	u_int8_t max_hislog_count;
	u_int8_t battery;
	u_int8_t open_timeout_en;
	u_int8_t open_timeout_v;
	u_int8_t v_wifilock;
	u_int8_t v_alarm;
	u_int8_t controller;	// 低4位表示多少个遥控器，高4位表示最近一次按下的遥控器ID
	u_int8_t v_cid;
	u_int8_t controller_idx[5];
	u_int8_t lan_num;
} door_lock_tt_cache_t;



typedef struct {
    u_int8_t battary; //电池剩余电量；,友泰只有是否为低电量数据 101：电量未知，102： 电量充足 103：表示低电量
    u_int8_t is_door_open; //是否开门状态；
    u_int8_t is_look_open; //锁是否开启状态
    u_int8_t is_battary_warn; //电池低电量报警
    u_int8_t has_break_door; //是否正在破坏门（门栓未用到）
    u_int8_t is_guard; //是否处于布防状态
	u_int8_t unlock_timeout_enable; // 是否支持门超时推送报警
	u_int8_t unlock_timeout;	// 门多久没关推送

	u_int8_t has_limit_fault;	// 限位阀故障
	u_int8_t has_moto_fault;	// 电机故障
	u_int8_t has_unlock_timeout;	// 长时间没关门(门栓未用到)

    //////////////////////////////////////
    //有泰门锁数据
    u_int8_t is_break_lock; //是否破坏了锁
    u_int8_t has_open_passwd; //是否已设置了开门密码
    u_int8_t ass_state;	// 关联状态 0 获取485模块ID 1 关联中 2 关联成功3 未知状态

	// 遥控器
	u_int8_t last_controller_id;	// 最近一次按下的遥控器ID，SAE_RF_DEV_DOOR_LOCK_CONTROLLER = 1289 收到这个信号以后读取才有效

	// 是否使用新的电池图标
	// 新图标情况下，battary 只有[5 20 40 60 80 100]这几个值
	u_int8_t is_new_battary_show;

	// 以下是SDK私有结构
	bool has_get_stat;
	u_int8_t wifilock_index;
	u_int8_t alarm_index;
	u_int8_t cid_index;
	u_int8_t controller_index[5];
}cl_rf_door_lock_stat_t;
    
typedef struct {
    u_int8_t value; //开关门动作
    u_int8_t info_type;
    u_int8_t is_valid; //是否有效
    u_int8_t pad;
    u_int32_t time_stamp; //时间戳

	u_int8_t ex_type; //扩展类型
	u_int8_t ex_value;//扩展值
}cl_rf_door_history_t;

//推送信息
typedef struct {
    u_int8_t value; //开关门动作
    u_int8_t info_type; //动作类型 0：开关门 1：撬门 2：撬锁3: 超时未锁门 4:低电量报警
    u_int8_t pad[2];
    u_int32_t time_stamp; //时间戳
}cl_door_alarm_info_t;

typedef struct {
	u_int8_t don_enable;	// WIFI接入自动开锁使能
	u_int8_t doff_enable;   // WIFI断开自动上锁使能
	u_int8_t don_starthour; // WIFI接入自动开锁开始的时间段
	u_int8_t don_endhour;  // WIFI接入自动开锁结束的时间段
	u_int8_t doff_starthour; // WIFI断开上锁开始的时间段
	u_int8_t doff_endhour;   // WIFI断开上锁结束的时间段
} cl_door_lock_wifilock_t;

typedef struct {
	u_int8_t id;	// 对应遥控器id 1-5	
	u_int8_t state;	// 设置遥控器状态，1 - 正常，2 - 挂失， 3 - 删除
	u_int8_t pad[2];// 不用填
	u_int8_t name[16];	// 如果不需要设置名字，填0
} cl_door_lock_controller_set_t;

typedef struct {
	u_int8_t id;
	u_int8_t state;	// 设置遥控器状态，1 - 正常，2 - 挂失， 3 - 删除
	u_int8_t pad[2];
	u_int8_t name[16];
} cl_door_lock_controller_info_t;

	
typedef struct {
    cl_rf_door_lock_stat_t stat;
    cl_rf_door_history_t his[200];
    cl_door_alarm_info_t alarm_info;
	cl_door_lock_wifilock_t wifilock;	// wifi 接入断开控制锁
	u_int32_t controller_num;		// 遥控器个数，最多5个
	cl_door_lock_controller_info_t controller[5];	// 遥控器 
}cl_door_lock_info_t;



////////////////////////////////////////////////////
//门磁
    
//自动布防和撤防
enum{
    MODE_AUTO_GUARD_ON = 0x1,
    MODE_AUTO_GUARD_OFF
};
    
//门磁控制类型 使用 cl_rf_dev_com_ctrl 控制
enum{
    RDM_CTRL_OPEN = 0x1, //控制锁开关
    RDM_CTRL_GRARD //控制布防撤防
};

    
typedef struct {
    u_int8_t enable;
    u_int8_t start_hour;
    u_int8_t end_hour;
    u_int8_t type; //使用 MODE_AUTO_GUARD_XX
}cl_rf_auto_guard_info_t;
    
typedef struct {
	bool stat_valid;	// 是否有取到数据
    u_int8_t battary; //电池剩余电量；
    u_int8_t is_door_open; //是否开门状态；
    u_int8_t is_battary_warn; //电池低电量报警
    u_int8_t is_break_door; //是否正在破坏门磁
    u_int8_t is_guard; //是否处于布防状态
    u_int8_t is_support_new_history;	// 支持新的历史记录查询
}cl_rf_door_magnet_stat_t;
    
typedef struct {
    cl_rf_door_magnet_stat_t stat;
    cl_rf_door_history_t his[200];
    cl_door_alarm_info_t alarm_info; //推送消息
    cl_rf_auto_guard_info_t auto_on; //自动布防信息
    cl_rf_auto_guard_info_t auto_off; //自动撤防信息

	u_int8_t index_alarm;//sdk专用，用来判断是否需要查询的
	u_int8_t index_autodefense;//sdk专用
	u_int32_t max_timestamp;//sdk专用，最小时间戳

	u_int8_t send_num;//sdk专用，发送计数
	u_int32_t send_timeout;//sdk专用，发送计数超时
}cl_door_magnet_info_t;

///////////////////////////////////////////////////
//橙灯情景面板
#define CDQJMB_KEY_MAXNUM	(9)
#define CDQJMB_NAME_MAXLEN	(26)
//cache报文
typedef struct {
	u_int32_t flag_len;//最低字节是len
	u_int32_t hislog_cur_index;//当前日志index
	u_int8_t hislog_count;//日志个数
	u_int16_t pad;
	u_int8_t key_num;//按键个数
	u_int8_t ice_rule_maxnum;//联动规则最大可设置条数，计数从1开始。
	u_int8_t ice_rule_curnum;//设备当前存储的联动规则条数，计数从1开始。
	u_int8_t key_changed[CDQJMB_KEY_MAXNUM];//每个按键的变化数
}cdqjmb_cache_t;

typedef struct {
	char name[CDQJMB_NAME_MAXLEN + 1];
}cl_cdqjmb_key_item_t;

typedef struct {
	u_int32_t flag;//暂时没用，放着
	u_int8_t key_num;//按键个数
	u_int8_t ice_rule_maxnum;//联动规则最大可设置条数，计数从1开始。
	u_int8_t ice_rule_curnum;//设备当前存储的联动规则条数，计数从1开始。
	cl_cdqjmb_key_item_t key_conf[CDQJMB_KEY_MAXNUM];//各按键配置

	//sdk专用，app可以不管
	bool is_valid;
	u_int8_t key_changed[CDQJMB_KEY_MAXNUM];//每个按键的变化数
}cl_cdqjmb_info_t;


typedef struct {
	u_int8_t index;
	u_int8_t len;
	char name[0];
}cdqjmb_set_name_t;

///////////////////////////////////////////////////
// 光照感应器
typedef struct {
	u_int32_t flags;
	u_int8_t battery;
	u_int8_t light_level;
	u_int16_t light_val;	
} light_sense_cache_t;

typedef struct {
	u_int8_t battery;	
	u_int8_t light_level;	// 光照等级
	u_int16_t light_val;		// 光照值
} cl_light_sense_stat_t;

typedef struct {
	cl_light_sense_stat_t stat;
} cl_light_sense_info_t;

///////////////////////////////////////////////////
//单火线开关
// 使用 cl_rf_dev_com_ctrl 控制开关，控制类型为 DHX_CTRL_ON_OFF
#define DHX_CTRL_ON_OFF 0x0

//最大路数
#define DHX_MAX_NUM		(4)

//最大名称长度
#define DHX_MAX_NAME_LEN	(32)

typedef struct {
    u_int8_t group_num; //有多少路
    u_int8_t pad[3];
    u_int32_t on_off_stat; //各路开关状态，第0bit表示第一路,0表示关，1表示开
}cl_dhx_switch_stat_t;

typedef struct {
	u_int8_t valid;
	u_int8_t name[DHX_MAX_NAME_LEN];
} cl_dhx_key_name_item_t;

typedef struct {
    cl_dhx_switch_stat_t stat;

	u_int8_t support_time;//0不支持，1支持
	u_int8_t support_name_set;//0不支持，1支持

	cl_dhx_key_name_item_t keys[DHX_MAX_NUM];

	//sdk专用
	u_int8_t init;
	u_int8_t time_cnum;
	u_int8_t index_key[DHX_MAX_NUM];
}cl_dhx_switch_info_t;

typedef struct {
	cl_dhx_key_name_item_t keys[DHX_MAX_NUM];
	u_int8_t index_key[DHX_MAX_NUM];
}cl_dhx_save_conf_t;

typedef struct {
	u_int32_t flagbits;
	u_int32_t pad;
	u_int8_t pad1[2];
	u_int8_t group_num;
	u_int8_t onoff;
} dhx_cache_old_t;

typedef struct {
	u_int32_t flagbits;
	u_int32_t pad;
	u_int8_t pad1[2];
	u_int8_t group_num;
	u_int8_t onoff;
	u_int8_t time_cnum;//定时器变化计数，从1开始，0忽略
	u_int8_t index_key[DHX_MAX_NUM];
} dhx_cache_t;
////////////////////////////////////////////////////
//温湿度探测
enum {
	RF_TT_CMD_QUERY_CURVE  =  200,
	RF_TT_CMD_PUSH_CURVE  =  201,
};
    
#define MAX_HM_HISTORY_NUM 12
    
typedef struct {
    int8_t temp; //温度 7F表示该点未采集到数据
    u_int8_t hum;//湿度 0-100 ,7F表示该时间点未采集到数据
}cl_hm_history_info_t;
    
typedef struct {
    int8_t cur_temp; //当前温度
    bool support_temp100;	// 支持温度小数显示
    int16_t cur_temp100;	// 如果支持小数显示，那么温度显示用这个数值,单位0.01度，支持负数
    u_int8_t cur_hum;//当前湿度 0-100
    u_int8_t battary; //电量 0-100
    u_int8_t is_low_battary_warn; //是否处于低电量报警状态
    u_int8_t history_hour; //表示最后一次整点采集的utc的时间，倒推，如，10
    u_int8_t is_history_data_valid; //历史数据是否有效
    cl_hm_history_info_t his_data[MAX_HM_HISTORY_NUM];

	// 硬件版本
	u_int8_t hw_ver;
	// 软件版本x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// svn号
	u_int32_t svn;

	// 下面数据SDK使用
	u_int8_t index_curve;
	u_int8_t index_alarm;
	bool stat_valid;
	int history_query_count;
}cl_hm_temp_hum_info_t;

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t temp;
	u_int8_t humi;
	u_int8_t index_curve;
} hm_temp_hum_cache_old_t;    

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t temp;
	u_int8_t humi;
	u_int8_t index_curve;
	// 下面是新增的
	u_int8_t index_alarm;
	int16_t temp100;
} hm_temp_hum_cache_t;    

////////////////////////////////////////////////////
//人体探测
//布防撤防控制类型
#define RDM_HM_CTRL_GUARD 0x1
// 设置报警频率，单位为5秒
#define RDM_HM_CTRL_SET_FREQ 0x2  
typedef struct {
    u_int8_t battary; //电量 0-100 为正常显示电量(悟安才支持) 103 表示电量过低104 表示充电中 105表示充满状态
    u_int8_t is_low_battary_warn; //是否处于低电量报警状态
    u_int8_t is_guard; //是否处于布防状态
    u_int8_t is_break;	// 是否有异常振动
    u_int32_t detectd_num; //最后一次布防至今探测到有多少次报警
    u_int32_t last_guard_time; //UTC的最后一次布防时间
    cl_door_alarm_info_t alarm_info;//推送消息
    cl_alarm_time_t alarm_time;	// 报警开始，结束和间隔时间
    cl_rf_door_history_t his[200]; //复用历史记录，只关注是否有效，如果有效，即探测到有人经过

	u_int8_t is_support_new_history;	// 支持新的历史记录查询

	// 硬件版本
	u_int8_t hw_ver;
	// 软件版本x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// svn号
	u_int32_t svn;
	// 下面数据SDK自己使用
	bool stat_valid;
	u_int8_t index_alarm;
	u_int8_t index_time;
}cl_hm_body_info_t;

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_time;	// 报警开始结束和间隔
	u_int16_t alarm_num;
} hm_body_detect_cache_t;

////////////////////////////////////////////////////
// 凯特插座
typedef enum {
	ACT_KTCZ_ONOFF = 0x43,
} ACT_KTCZ_T;

typedef struct {
	u_int8_t onoff;
} cl_ktcz_stat_t;
// 上层获取到的全部状态信息
typedef struct {
	cl_ktcz_stat_t stat;
} cl_ktcz_info_t;

////////////////////////////////////////////////////
// 暖气阀
// APP到底层控制类型
typedef enum {
	//ACT_HEATING_VALVE_ONOFF = 0x81,
	ACT_HEATING_VALVE_MODE = 0X82,	// value 1 自动 0 手动
	//ACT_HEATING_VALVE_ANTI_LIME = 0x83,
	//ACT_HEATING_VALVE_FROST_PROTECTION = 0x84,
	ACT_HEATING_VALVE_CHILD_LOCK = 0x85,
	ACT_HEATING_VALVE_WINDOW = 0x86,	// 窗口功能 0 取消 1使能
	//ACT_HEATING_VALVE_SUMMER_WINTER = 0X87,
	ACT_HEATING_VALVE_DATE = 0x88,
	ACT_HEATING_VALVE_TEMP = 0x89,
	ACT_HEATING_VALVE_PERIOD = 0x8a,
	
} ACT_HEATING_VALVE_T;

enum {
	UP_TLV_GET_STATUS = 212,
	UP_TLV_GET_TIME = 213,
	UP_TLV_GET_CIRCLE = 214,
};

typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
} cl_heating_valve_param_date_t;

typedef struct {
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;
} cl_heating_valve_param_temp_t;

// 设置加热和经济周期的4个时间点
typedef struct {
	u_int8_t day;	// 星期几（取值范围：1-7）
	u_int8_t resv;	
	u_int8_t hh1;	// 第一加热周期 小时 （取值范围：0 - 24）
	u_int8_t hm1;	// 第一加热周期 分钟 （取值范围：0 - 50，粒度10）
	u_int8_t eh1;	// 第一经济周期 小时（取值范围：0 - 24）
	u_int8_t em1;	// 第一经济周期 分钟（取值范围：0 - 50，粒度10）
	u_int8_t hh2;	// 第二加热周期 小时（取值范围：0 - 24）
	u_int8_t hm2;	// 第二加热周期 分钟（取值范围：0 - 50，粒度10）
	u_int8_t eh2;	// 第二经济周期 小时（取值范围：0 - 24）
	u_int8_t em2;	// 第二经济周期 分钟（取值范围：0 - 50，粒度10）
} cl_heating_valve_param_period_t;

typedef struct {
	u_int8_t hh1;	// 第一加热周期 小时 （取值范围：0 - 24）
	u_int8_t hm1;	// 第一加热周期 分钟 （取值范围：0 - 50，粒度10）
	u_int8_t eh1;	// 第一经济周期 小时（取值范围：0 - 24）
	u_int8_t em1;	// 第一经济周期 分钟（取值范围：0 - 50，粒度10）
	u_int8_t hh2;	// 第二加热周期 小时（取值范围：0 - 24）
	u_int8_t hm2;	// 第二加热周期 分钟（取值范围：0 - 50，粒度10）
	u_int8_t eh2;	// 第二经济周期 小时（取值范围：0 - 24）
	u_int8_t em2;	// 第二经济周期 分钟（取值范围：0 - 50，粒度10）
} cl_heating_valve_day_period_t;

typedef struct {
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t syn3;
	u_int8_t ver;
	u_int8_t cmd;
	u_int8_t plen;
	u_int8_t seq;
	u_int8_t checksum;
	//u_int8_t param[0];
} heating_valve_uart_hd_t;

#if 0
typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t window;
	u_int8_t against;
	u_int8_t frost;
	u_int8_t child_proof;
	u_int8_t summer_winter;
	u_int8_t battery;
	u_int16_t current_temp;
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;
} heating_valve_uart_stat_param_t;
#else
typedef struct {
	u_int8_t error;
	u_int8_t mode;
	u_int8_t windowfun;
	u_int8_t windowopen;
	u_int8_t child_proof;
	u_int8_t battery;
	u_int16_t current_temp;
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;
} heating_valve_uart_stat_param_t;

#endif
typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
} heating_valve_uart_time_t;

typedef struct {
	// 基本信息
	u_int8_t onoff;	// 废弃
	u_int8_t mode;
	u_int8_t window;
	u_int8_t against;	// 废弃
	u_int8_t frost;		// 废弃
	u_int8_t child_proof;
	u_int8_t summer_winter;	// 废弃
	u_int8_t battery;
	u_int16_t current_temp;
	u_int16_t manual_temp;
	u_int16_t heat_temp;
	u_int16_t economy_temp;

	// 下位机时间
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;

	// 
	u_int8_t error;	// 错误号	
	u_int8_t windowfun;	// 是否开启窗户检测功能
	u_int8_t windowopen;	// 窗是否打开
} cl_heating_valve_stat_t;

typedef struct {
	int seq;
	int recv_count;
	int total_count;
	cl_heating_valve_day_period_t day_period[7];
} heating_valve_day_period_cache_t;

typedef struct {
	u_int8_t slice;
	u_int8_t slice_idx;
} heating_valve_period_slice_hd_t;

typedef struct {
	u_int8_t mcu_stat_index;//状态信息摘要
	u_int8_t mcu_time_index;//时间信息摘要
	u_int8_t mcu_circle_index;//加热周期和经济周期 摘要
	u_int8_t quick_timer_index;//快键定时器摘要
} heaing_valve_cache_t;

// 上层获取到的全部状态信息
typedef struct {
	// 基本状态
	cl_heating_valve_stat_t stat;

	// 一个星期每天的加热和经济时间
	cl_heating_valve_day_period_t day_period[7];

	// 内部数据APP上层无视
	heating_valve_day_period_cache_t dpc;
	heaing_valve_cache_t cache;
} cl_heating_valve_info_t;

////////////////////////////////////////////////////
// 通用的探测器，比如气感 水感等
//控制类型 使用 cl_rf_dev_com_ctrl 控制
enum {
    ACT_COM_DETECTOR_CTRL_DEFENSE = 0x1, //布放撤防
    ACT_COM_DETECTOR_CTRL_REPORT_FREQ = 0x2,	// 上报频率，需要乘以5
    ACT_COM_DETECTOR_CTRL_ALARM_TYPE = 0x3,		// 设置报警方式0 :报警就一直推送直到APP 确认 1 只报警一次
    ACT_COM_DETECTOR_CTRL_ALARM_DEMO = 0x4,		// 报警演示
};

enum {
	CD_ALARM_INFO_TYPE_DEV,			// 设备报警
	CD_ALARM_INFO_TYPE_BATTERY,		// 电池过低报警
	CD_ALARM_INFO_TYPE_ALARM_PAUSE, // 报警暂停
} ;

//推送信息
typedef struct {

    u_int8_t value; 
	/*
		 历史信息类型
		 	0: 设备报警/报警恢复，上面value 1表示报警 0表示报警恢复
		 	1: 电量过低报警/电量恢复,上面value 1表示电量过低报警，0表示电量恢复 
		 	2: 报警暂停/报警重新开始,上面value 1表示报警暂停, 0表示报警重新开始
	*/
    u_int8_t info_type;
    u_int8_t pad[2];
    u_int32_t time_stamp; //时间戳
} cl_com_detector_alarm_info_t;

typedef struct {
	// 硬件版本
	u_int8_t hw_ver;
	// 软件版本x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// 电池剩余电量，103表示电量过低，102表示电量正常
	u_int8_t abc_battery;
	u_int8_t is_alarm;	// 是否正在报警 1 正在报警 0 报警恢复
	u_int8_t is_low_battery;	// 电池是否低电量 1表示电池电流低报警，0表示未报警
	u_int8_t is_defence;	// 是否处于布防 1 是 0 否
	u_int8_t is_pause_alarm;	// 是否处于暂停报警 1 暂停 0 未暂停
	u_int8_t is_alarm_once;	// 0 一直推送报警直到APP确认 1 只推送一次

	u_int8_t is_support_new_history;	// 支持新历史记录查询
	u_int32_t svn;	// svn号

	// 下面数据SDK自己使用
	u_int8_t index_alarm;
	u_int8_t index_time;
	u_int8_t stat_valid;
} cl_com_detector_stat_t;

typedef struct {
	u_int8_t value; // 具体类型具体值含义
	/*
		 历史信息类型
		 	0: 设备报警/报警恢复，上面value 1表示报警 0表示报警恢复
		 	1: 电量过低报警/电量恢复,上面value 1表示电量过低报警，0表示电量恢复 
		 	2: 报警暂停/报警重新开始,上面value 1表示报警暂停, 0表示报警重新开始
	*/
    u_int8_t info_type; 
    u_int8_t is_valid; //是否有效
    u_int8_t pad;
    u_int32_t time_stamp; //时间戳
} cl_com_detector_his_t;


// 上层获取到的全部状态信息
typedef struct {
	cl_com_detector_alarm_info_t alarm_info;
	cl_com_detector_stat_t stat;
	cl_alarm_time_t alarm_time;	// 报警时间，包括开始、结束时间，间隔时间
	cl_com_detector_his_t his[200];
} cl_com_detector_info_t;

//#if 1
typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_time;	// 报警开始结束和间隔
} com_detector_cache_t;

// 甲醛传感器
enum {
	ACT_JQ_SET_ALARM_THR,
	ACT_JQ_SET_ALARM_PERIOD,
	ACT_JQ_FLUSH_CH2O,
};

enum {
	UP_TLV_JQ_SET_ALARM_THR = 201,
	UP_TLV_JQ_SET_ALARM_PERIOD = 202,	
	UP_TLV_JQ_FLUSH_CH2O = 203,
	UP_TLV_JQ_GET_CH2O_HISTORY = 204,
};


typedef struct {
	u_int8_t valid;	// 数据是否有效
	
	u_int16_t cur_ch2o;	// 当前浓度，单位ppb (1ppb = 1ppm / 1000 = 0.0007466mg/m? = 0.7466μg/m?)
	u_int8_t battery;	// 电池电量 0、20%、40%、60%、80%、100% 	   101表示电量未知
	u_int8_t std;	// 报警阀值标准：0 = 国标，1 = 欧标，2 = 自定义
	u_int16_t thr_ch2o; // 自定义阀值，单位μg/m3
	u_int16_t period;	// 报警间隔，单位：分钟，默认值为1分钟
	// 下面数据APP不用关心
	u_int8_t v_alarm;	
} cl_jq_stat_t;

typedef struct {
	u_int32_t time;	// 采集的时间戳
	u_int16_t ch2o;	// 浓度，PPB
} cl_jq_history_item_t;

#define MAX_JQ_HISTORY_ITEM_NUM 31

typedef struct {
	cl_jq_stat_t stat;
	int n_item;	// 有多少天历史平均甲醛浓度记录
	cl_jq_history_item_t items[MAX_JQ_HISTORY_ITEM_NUM];	// 平均甲醛浓度记录
	
	// 下面数据APP不用关心
	u_int8_t v_ch2o_history;//历史记录变化次数
	u_int8_t query_map[MAX_JQ_HISTORY_ITEM_NUM];	// 平均甲醛浓度记录
} cl_jq_info_t;

typedef struct {
	u_int8_t len;
	u_int8_t battery;
	u_int16_t cur_ch2o;
	u_int8_t std;
	u_int8_t v_alarm;
	
	u_int16_t thr_ch2o;
	u_int16_t alarm_period;

	u_int16_t hislog_count;
	u_int32_t hislog_index_current;

	u_int8_t v_ch2o_history;
	u_int8_t n_ch2o_history;
} jq_cache_t;



//#endif
////////////////////////
// 汇泰龙智能锁
//#define HTLLOCK_MAX_USER_NUM 32
//用户数量变大了
#define HTLLOCK_MAX_USER_NUM 210
#define HTLLOCK_MAX_NOTICE_TYPE_NUM 16
#define HLLLOCK_MAX_TRY_QUERY 10

// 网关透传的TLV类型枚举
enum {
	HTLLOCK_RAW_DATA = 4,		// 透传数据
	HTLLOCK_USER_MANAGE = 50,	// 用户管理
	HTLLOCK_INFO_NOTICE = 51,	// 信息提醒	
};


enum {
	ACT_HTLLOCK_ADMIN_LOGIN = 51,
    ACT_HTLLOCK_USER_MANAGE_SET_NAME,
    ACT_HTLLOCK_USER_MANAGE_SET_PIC,
    ACT_HTLLOCK_USER_MANAGE_SET_BIND,
    ACT_HTLLOCK_USER_MANAGE_SET_UNBIND,
    ACT_HTLLOCK_USER_MANAGE_SET_REMIND_ONOFF,
    ACT_HTLLOCK_SET_INTICE_INFO,
    ACT_HTLLOCK_GET_HISTORY,    
    ACT_HTLLOCK_SET_PIN,	// 临时PIN码设置
    ACT_HTLLOCK_SET_VOL = 100,	// 设置音量1 高音 2 中音3低音 4 静音
    ACT_HTLLOCK_SET_LANG,	// 设置语言1 中文 2英文
    ACT_HTLLOCK_LOCAL_OPEN,	// 局域网开锁
    ACT_HTLLOCK_QUERY_PIN,	// 查询临时PIN
};

// cache报文
typedef struct {
	u_int32_t devstat_bits;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t index_notice;
	u_int8_t index_alarm;
	u_int8_t resv1;
	u_int16_t index_user;
	u_int16_t user_count;
} htllock_tt_cache_t;

//智能门磁flag位定义
#define 	DORR_MANAGE_V2_FLAG_ONOFF		(BIT(0))
#define 	DORR_MANAGE_V2_FLAG_ALARM		(BIT(1))
#define 	DORR_MANAGE_V2_FLAG_BREAK_DOOR	(BIT(2))
#define 	DORR_MANAGE_V2_FLAG_SECURITY	(BIT(3))

//智能门磁cache报文
typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_autodefense;
}door_manage_v2_cache_t;

typedef struct {
	u_int8_t start;
	u_int8_t pid;
	u_int8_t len;
	u_int8_t mcmd;
	u_int8_t scmd;
} htllock_uart_hdr_t;

typedef struct {
	u_int8_t checksum;
	u_int8_t end;
} htllock_uart_tail_t;

// 管理员登陆协议
typedef struct {
	u_int8_t start;
	u_int8_t pid;
	u_int8_t len;
	u_int8_t mcmd;
	u_int8_t scmd;

	u_int16_t id;
	u_int8_t pwd[6];
	u_int8_t checksum;
	u_int8_t end;
} htllock_uart_cmd_login_t;

typedef struct {
	u_int8_t start;
	u_int8_t pid;
	u_int8_t len;
	u_int8_t mcmd;
	u_int8_t scmd;

	u_int8_t err;
	u_int8_t checksum;
	u_int8_t end;
} htllock_uart_cmd_login_reply_t;

typedef struct {
	u_int8_t doorstate;
	u_int8_t lockstate;
	u_int8_t deadbolt;	// 方舌状态0 缩回 1 弹出
	u_int8_t latchbolt;	// 斜舌状态0 缩回 1 弹出
	u_int8_t fingerermain;
	u_int8_t cardremain;
	u_int8_t coderemain;
	u_int8_t battery;
	u_int8_t vol;
	u_int8_t lang;
} htllock_uart_cmd_stat_reply_t;

typedef struct {
	u_int8_t op; // 用户管理和信息管理的reply
	u_int8_t err; // 0-操作成功 1-操作失败 
	u_int16_t index;// 用户编号
} htllock_tt_set_reply_t;

typedef struct {
	u_int16_t id;
	u_int8_t pwd[6];
} htllock_tt_admin_login_t;


/*  用户管理*/

// 第一个字节OP的定义
typedef enum {
	HTLLOCK_USER_MANAGE_OPT_TYPE_QUERY = 1,	// 查询用户属性
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_NAME = 2,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_PIC = 3,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_BIND = 4,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_UNBIND = 5,
	HTLLOCK_USER_MANAGE_OPT_TYPE_SET_REMIND_ONOFF = 8,
} HTLLOCK_USER_MANAGE_OPT_TYPE_T;

//query type
enum {
	HTLLOCK_USER_QUERY_BATCH = 1,
	HTLLOCK_USER_QUERY_ONE = 2,
	HTLLOCK_USER_QUERY_SUMMARY = 3,
};

typedef struct {
	u_int8_t op; // 填1
	u_int8_t resv;
} htllock_tt_user_manage_get_t;


#if 0
typedef struct {
	u_int8_t op; // 用户管理的动作，1-查询用户属性
	u_int8_t index_user;	// 当前系统维护的用户变化数目
	u_int16_t slice; // 查询用户时候的序号保证：从0开始，严格按序增加，最高bit位为1表示传送完成。

	u_int16_t index; // 用户编号
	u_int16_t pindex; // 该用户的父亲用户的编号。如果是0，表示该用户就是父亲用户。 手机app会根据该值对应关系展示树形用户的界面
	/*
		
	创建该用户原始的id，位0-11表示用户编号，
		位12-15表示用户类型，1表示指纹类型，2表示密码类型，3表示感应卡类型
	*/
	u_int16_t create_id;
	u_int8_t pic_id; // 用户头像对应的编号。 app内置的每张图片，对应的图片编号固定，为0显示"无头像"
	/*
		
	功能属性
		从右到左依次
		bit0：该用户进行开关锁的状态提醒是否关闭。 1-关闭状态提醒 0-不关闭
	*/
	u_int8_t flagbit;
	u_int8_t name[12]; // 用户名称，限制在12字节，为0，显示"未命名"
} htllock_tt_user_manage_push_t;
#else
typedef struct {
	u_int8_t op; // 用户管理的动作，1-查询用户属性
	u_int8_t resv;
	
	
	u_int16_t index_user;	// 当前系统维护的用户配置变化次数
	u_int16_t get_useridex;	// 用户期望获取的用户序号，1开始。 和下面的用户编号index用途不一样。

	u_int16_t index; // 用户编号
	u_int16_t pindex; // 该用户的父亲用户的编号。如果是0，表示该用户就是父亲用户。 手机app会根据该值对应关系展示树形用户的界面
	/*
		
	创建该用户原始的id，位0-11表示用户编号，
		位12-15表示用户类型，1表示指纹类型，2表示密码类型，3表示感应卡类型
	*/
	u_int16_t create_id;
	u_int8_t pic_id; // 用户头像对应的编号。 app内置的每张图片，对应的图片编号固定，为0显示"无头像"
	/*
		
	功能属性
		从右到左依次
		bit0：该用户进行开关锁的状态提醒是否关闭。 1-关闭状态提醒 0-不关闭
	*/
	u_int8_t flagbit;
	u_int8_t name[12]; // 用户名称，限制在12字节，为0，显示"未命名"
} htllock_tt_user_manage_push_t;

#endif

typedef struct {
	u_int8_t is_close_stat_reminder;	// 该用户进行开关锁的状态提醒是否关闭。 1-关闭状态提醒 0-不关闭
	
	u_int16_t index; // 用户编号
	u_int16_t pindex; // 该用户的父亲用户的编号。如果是0，表示该用户就是父亲用户。 手机app会根据该值对应关系展示树形用户的界面
	/*
		
	创建该用户原始的id，位0-11表示用户编号，
		位12-15表示用户类型，1表示指纹类型，2表示密码类型，3表示感应卡类型
		create_id 唯一性
	*/
	u_int16_t create_id;
	u_int8_t pic_id; // 用户头像对应的编号。 app内置的每张图片，对应的图片编号固定，为0显示"无头像"
	u_int8_t pad[3];
	u_int8_t name[12]; // 用户名称，限制在12字节，为0，显示"未命名"
} htllock_user_manage_stat_t;

typedef struct {
	u_int8_t op; // 用户管理的动作，2-修改用户名
	u_int8_t resv;
	u_int16_t index;// 用户编号
	u_int8_t name[12];
} htllock_tt_user_manage_set_name_t;

typedef struct {
	u_int64_t sn;
	u_int16_t index;// 用户编号
	u_int16_t resv;
	u_int8_t name[12];
} htllock_tt_user_manage_set_name_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t index_type;
	u_int8_t num;
	u_int16_t get_index;
}httlock_bid_user_query_v2_t;

typedef struct {
	u_int64_t sn;
	u_int16_t id_index;
	u_int8_t pic_id;
	u_int8_t pad;
}httlock_bid_user_picid_set_v2_t;

typedef struct {
	u_int64_t sn;
	u_int16_t id_index;
	u_int16_t cindex;
}httlock_bid_user_bind_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t isclose;
	u_int8_t resv;
	u_int16_t id_index;
}httlock_bid_notice_v2_t;

typedef struct {
	u_int16_t id_index;
	u_int16_t pid_index;
	u_int16_t create_id;
	u_int8_t pic_id; 
	u_int8_t flagbit;
	u_int8_t name[12];
}httlock_bid_cache_user_item_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t index_type;
	u_int8_t resv;
	u_int16_t index_userconf;
	u_int16_t all_usernum;
	u_int16_t tmp_index;
	httlock_bid_cache_user_item_v2_t item[0];
}httlock_bid_cache_hd_v2_t;


typedef struct {
	u_int64_t sn;
	u_int8_t op;//op：用户变化的动作，1-增加用户  2-删除用户
	u_int8_t resv;
	u_int16_t create_id;
	u_int16_t id_index;
	u_int16_t pad;
}httlock_bid_user_adddel_v2_t;

typedef struct {
	u_int8_t op; // 用户管理的动作，3-修改用户头像
	u_int8_t pic_id;
	u_int16_t index;// 用户编号
} htllock_tt_user_manage_set_pic_t;

typedef struct {
	u_int8_t op; // 用户管理的动作，4-关联用户
	u_int8_t resv;
	u_int16_t index;// 用户编号
	u_int16_t cindex;// 待关联的孩子用户的编号 
} htllock_tt_user_manage_set_bind_t;

typedef struct {
	u_int8_t op; // 用户管理的动作，5-解除关联用户
	u_int8_t resv;
	u_int16_t index;// 用户编号
	u_int16_t cindex;// 待解除关联的孩子用户的编号  
} htllock_tt_user_manage_set_unbind_t;

typedef struct {
	u_int8_t op;
	u_int8_t isclose;
	u_int16_t index;
} htllock_tt_user_manage_set_remind_onoff_t;

typedef struct {
	u_int8_t op;
	u_int8_t resv;
	u_int16_t get_userindex;
} htllock_tt_user_manage_get_info_t;

/*  信息提醒*/
// 第一个字节OP的定义
typedef enum {
	HTLLOCK_INFO_NOTICE_OPT_TYPE_QUERY = 1,	// 查询提醒信息设置
	HTLLOCK_INFO_NOTICE_OPT_TYPE_SET = 2,	// 设置提醒信息
	HTLLOCK_INFO_NOTICE_OPT_TYPE_GET_HISTORY = 3,	// 获取历史信息
} HTLLOCK_INFO_NOTICE_OPT_TYPE_T;


typedef struct {
	u_int8_t op; // 填1
	u_int8_t resv;
} htllock_tt_info_notice_get_t;

/*
	
op：信息提醒的动作，1-查询信息提醒设置
	sbit_xx 信息提醒设置bit统一格式，
	  从右到左依次排，
	  提醒内容（bit0-bit3）：
		0-"app通知"、1-开关锁、2-低电量、3-撬锁、4-门未锁、5-错误报警、6-劫持报警、7-				机械锁报警
	  提醒方式（bit4-bit7）：
		全0-不要提醒、bit4-为1表示支持通知提醒、bit5-为1表示支持在家免打扰、bit6-为1		  		表示支持短信提醒、bit7-为1表示支持电话提醒
*/
typedef struct {
	u_int8_t op;	// 
	u_int8_t index_notice;	// 当前设备维护的通知设置变化数
	u_int8_t sbit[8];
} htllock_tt_info_notice_push_t;

typedef struct {
	u_int8_t op;
	u_int8_t sbit_temp;
} htllock_tt_info_notice_set_t;

typedef struct {
	u_int8_t op;	// 信息提醒的动作，2-设置信息提醒
	u_int8_t err;
	u_int8_t sbit_tmp;
	u_int8_t resv;
} htllock_tt_info_notice_set_reply_t;


typedef struct {
	u_int8_t op;
	u_int8_t pad[3];
	u_int32_t timestamp;
} htllock_tt_info_notice_get_history_t;

enum {
	HTLLOCK_HISTORY_TYPE_LOCKONOFF = 1,
	HTLLOCK_HISTORY_TYPE_LOWAC,
	HTLLOCK_HISTORY_TYPE_FORCELOCK,
	HTLLOCK_HISTORY_TYPE_DOORISOPEN,
	HTLLOCK_HISTORY_TYPE_ERRORINPUT,
	HTLLOCK_HISTORY_TYPE_HIJACK,
	HTLLOCK_HISTORY_TYPE_MANDLE
};

typedef struct {
	u_int8_t op;
	/*
	
	0  bit: 表示该字段和后面的时间戳是否有效。1-表示有效，0-表示无效
	1  bit: 根据2-7bit表示的不同类型，0和1表示的含义不同
	2-7 bit: 历史信息类型
	   1：开/关锁动作， 上面bit1的值含义为：1-表示开锁 0-表示关锁
	   2：低电量，上面bit1的值含义为：1-表示低电量 0-表示正常
	   3：撬锁动作， 上面bit1的值含义为：1-表示撬锁 0-表示正常
	   4：门未锁， 上面bit1的值含义为：1-表示门未锁 0-表示正常
	   5：错误报警， 上面bit1的值含义为：1-表示错误报警 0-表示正常
	   6：劫持报警， 上面bit1的值含义为：1-表示劫持报警0-表示正常
	   7：机械锁报警， 上面bit1的值含义为：1-表示机械锁报警0-表示正常
	*/
	u_int8_t info;
	u_int16_t id;
	u_int32_t timestamp;
} htllock_tt_info_notice_get_history_reply_t;

// 设置临时PIN
typedef struct {
	u_int16_t time;	// 有效时间，单位分钟(0xffff表示不限制)
	u_int8_t cnt;	// 密码可用次数(0xff表示不限制)
	u_int8_t pwd_len;	// 密码长度，只能是6字节
	u_int8_t pwd[6];	// 密码
} htllock_tt_set_pin_t;

enum {
	HTLLOCK_INFO_NOTICE_TYPE_APP = 1,		// APP通知
	HTLLOCK_INFO_NOTICE_TYPE_LOCKONOFF = 2,	// 开关锁
	HTLLOCK_INFO_NOTICE_TYPE_LOWAC = 3,		// 低电量
	HTLLOCK_INFO_NOTICE_TYPE_FORCELOCK = 4,	// 撬锁
	HTLLOCK_INFO_NOTICE_TYPE_DOORISOPEN = 5,	// 门未锁
	HTLLOCK_INFO_NOTICE_TYPE_ERRORINPUT = 6,	// 错误报警
	HTLLOCK_INFO_NOTICE_TYPE_HIJACK = 7,	// 劫持报警
	HTLLOCK_INFO_NOTICE_TYPE_MANDLE = 8,	// 机械锁报警
};

typedef struct {
	u_int8_t type;	// HTLLOCK_INFO_NOTICE_TYPE_XXX 报警类型
	u_int8_t support_remind;	// 支持通知提醒
	u_int8_t support_trouble_free;	// 支持在家免打扰
	u_int8_t support_msg_remind;	// 支持短信提醒
	u_int8_t support_tel_remind;	//  支持电话提醒
} htllock_info_notice_stat_t;



typedef struct {
	u_int8_t onoff;	// 锁开关状态 1 开 0 关
	u_int8_t vol;	// 音量1 高2 中 3低4 静音
	u_int8_t lang;	// 语言 1 中文2 英文
	u_int8_t battery;	// 电量0-100
} htllock_lock_stat_t;

typedef struct {	
	
    u_int8_t is_valid; //是否有效
    u_int8_t value; 
	/*
	  报警类型，用枚举值 HTLLOCK_HISTORY_TYPE_XXX
	  1：开/关锁动作， 上面value的值含义为：1-表示开锁 0-表示关锁
	  2：低电量，上面value的值含义为：1-表示低电量 0-表示正常
	  3：撬锁动作， 上面value的值含义为：1-表示撬锁 0-表示正常
	  3：撬锁动作， 上面value的值含义为：1-表示撬锁 0-表示正常
	  4：门未锁， 上面value的值含义为：1-表示门未锁 0-表示正常
	  5：错误报警， 上面value的值含义为：1-表示错误报警 0-表示正常
	  6：劫持报警， 上面value的值含义为：1-表示劫持报警0-表示正常
	  7：机械锁报警， 上面value的值含义为：1-表示机械锁报警0-表示正常
	*/
    u_int8_t info_type;
    u_int8_t pad;
    u_int32_t time_stamp; //时间戳
    /*
		用户原始的id，
		位0-11表示用户编号，
		位12-15表示用户类型，1表示指纹类型，2表示密码类型，3表示感应卡类型
	*/
   	u_int16_t create_id; 
} httlock_history_t;

// 上层获取到的全部状态信息
typedef struct {
	htllock_lock_stat_t lock_stat;		// 锁状态
	htllock_tt_set_pin_t last_pin;		// 上次设置的临时PIN，收到 SAE_HTLLOCK_PIN_REPLY 才去获取
	// 用户信息
	htllock_user_manage_stat_t user_manage[HTLLOCK_MAX_USER_NUM]; // 用户管理信息	
	htllock_info_notice_stat_t info_notice[HTLLOCK_MAX_NOTICE_TYPE_NUM]; // 信息提醒信息
	httlock_history_t history[200];
	
	// APP无视下面这些
	u_int8_t get_cache;
	htllock_tt_cache_t cache;	// cache 信息

	u_int16_t index_user;	// 从cache获取的最新user 变化数
	u_int16_t user_count;	// 从cache获取的最新user个数
	u_int16_t user_write_index;
	htllock_tt_user_manage_push_t user_slice[HTLLOCK_MAX_USER_NUM];
} cl_htllock_info_t;

#define USER_INFO_FLASH_MAGIC	0x6831
// 保持在文件里面的结构
typedef struct {
	u_int32_t magic;	// 0x6831
	u_int16_t index_user;
	htllock_user_manage_stat_t user_manage[HTLLOCK_MAX_USER_NUM]; // 用户管理信息
} htllock_user_info_flash_t;

//////////////////////////////////////////////////
//悟空空调贴
    
//使用通用控制
enum{
    CT_WK_ONOFF = 0x0, //控制开关
    CT_WK_MODE, //控制模式
    CT_WK_TEMP, //控制temp
    CT_WK_WIND, //控制风力
    CT_WK_WIND_DIR, //控制风向
    CT_WK_START_MATCH, //启动编码匹配
    CT_WK_STOP_MATCH //停止编码匹配
};
    
enum {
    MS_WK_ERR_NONE = 0x0,
    MS_WK_ERR_DEV_NETWORK, //到设备网络不通
    MS_WK_ERR_DEV_BUSY, //设备忙，正在匹配其他中
    MS_WK_ERR_WAIT_DEV_RESP, //等待设备响应超时
    MS_WK_ERR_DEV_WAIT_IR_TIME_OUT, //等待按遥控器超时
    MS_WK_ERR_DEV_TRANS_IR, //IR信号传输失败
    MS_WK_ERR_SERVER_NETWORK, //到服务器网络不通
    MS_WK_ERR_SERVER_RESP, //等待服务器响应超时
    MS_WK_ERR_SERVER_IR, //服务器说无效信号或匹配失败
    MS_WK_ERR_SERVER_DOWNLOAD_CODE, //下载红外编码失败
    MS_WK_ERR_IR_TO_DEVICE_TIMEOUT //红外传输到设备失败
};

//空调贴tlv ,从210开始
enum {
	UP_TLV_WKAIR_SHORTCUTS_QUERY = 210,
	UP_TLV_WKAIR_SHORTCUTS_SET = 211,
	UP_TLV_WKAIR_TEMP_ADJUST  =  212,
	UP_TLV_WKAIR_LED_MODE = 213,
	UP_TLV_WKAIR_DIR = 214,
	UP_TLV_WKAIR_CHECK = 215,
	UP_TLV_WKAIR_ADJUST = 216,
	UP_TLV_WKAIR_IR_QUERY = 217,
	UP_TLV_WKAIR_SHOCK_QUERY = 218,
	UP_TLV_WKAIR_STATUS_SYNC = 219,
	UP_TLV_WKAIR_ADDR_SET = 220,
	UP_TLV_WKAIR_GET_IR_CODE = 223,
	UP_TLV_WKAIR_SYNC_CTRL = 224,
};

//空调贴action
enum {
	ACT_WKAIR_DIR = 50,
	ACT_WKAIR_CHECK,
	ACT_WKAIR_DIR_ADJUST,
	ACT_WKAIR_SHOCK_ADJUST,
	ACT_WKAIR_SHOCK_QUERY,
	ACT_WKAIR_IR_QUERY,
	ACT_WKAIR_ADDR_SET,
	ACT_WKAIR_IR_SET,
	ACT_WKAIR_IR_SYNC,
};

typedef struct {
    u_int8_t cur_step; // 当前进行到第几步，服务器匹配时有效
    u_int8_t max_step; //总共多少步，服务器匹配时有效
    u_int8_t error; // 匹配出现错误
    u_int8_t pad; //
    u_int16_t matched_id; // 启动后非0有效
    u_int16_t num; // 匹配空调编码使用
    u_int16_t matched_ids[32]; //匹配空调编码使用
}cl_rf_air_code_match_stat_t;
    
typedef struct {
    u_int16_t ir_id; // 当前红外库id
    u_int8_t battary; //电量，1-100，120为未知
    int8_t room_temp; //室温，120为未知
    u_int8_t room_humi; //湿度，120为未知
    u_int8_t charge;//充电标记，1：充电中 0：没有充电
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
	int8_t tmp_adjust;//矫正温度
	u_int8_t led_mode;//led灯模式
	//空调地址
	u_int8_t addr;//1-16
	// 红外同步开关
	u_int8_t support_ir_sync_ctrl;	// 支持同步开关控制
	u_int8_t ir_sync_ctrl;	// 0 关闭 1开启
}cl_wk_air_work_stat_t;
    
//悟空 空调贴
typedef struct {
    cl_wk_air_work_stat_t stat; // 状态信息
    cl_rf_air_code_match_stat_t match_stat; //匹配信息
	
	u_int8_t ir_valid;//表示是否有效，空调贴至少要收到tt后才能表示有效
	u_int8_t shortcuts_count;//1-255,0表示不支持快速定时器
	u_int8_t status_count;//1-255
	u_int8_t comm_timer_count;//1-255,0表示不支持

	// 下面是SDK自己使用
	u_int8_t sync_num; // 1-255
	u_int8_t code[256];
	u_int8_t code_widx;
}cl_wk_air_info_t;

//空调贴cache报文
typedef struct {
	u_int8_t shortcuts_count;//1-255,0表示不支持快速定时器
	u_int8_t status_count;//1-255
	u_int8_t comm_timer_count;//1-255,0表示不支持
	u_int8_t ir_num;//1-255，红外学习编码编码

    u_int8_t battary; //电量
    u_int8_t room_temp;
    u_int8_t room_humi;
    u_int8_t onoff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t wind;
    u_int8_t wind_direct;

	u_int8_t addr;

	u_int8_t channel;
	u_int8_t power;
	u_int8_t sync_num;
	
	u_int8_t ir_sync_ctrl;
}wk_air_cache_t;

typedef struct {
	u_int16_t ir_id;
	u_int8_t sync_num;
	u_int8_t offset;
	u_int8_t pad[2];
} wk_air_ir_code_get_t;

typedef struct {
	u_int8_t ret;
	u_int8_t sync_num;
	u_int8_t offset;
	u_int8_t data[0];
} wk_air_ir_code_get_reply_t;


//橙灯情景面板
enum {
	ACT_CDQJMB_NAME_SET = 1,
};

enum {
	UP_TLV_CDQJMB_NAME_SET = 200,
	UP_TLV_CDQJMB_NAME_QUERY = 201,
	UP_TLV_CDQJMB_NAME_PUSH = 202,
};

//智皇电机控制
enum{
	ACT_ZHDJ_STATUS_SET = 1,
	ACT_ZHDJ_LOCATION_SET,
	ACT_ZHDJ_LOCATION_QUERY,
	ACT_ZHDJ_BIND,
	ACT_ZHDJ_TYPE,
	ACT_ZHDJ_DIR,
};

//智皇电机tlv
enum {
	UP_TLV_ZHDJ_STATUS_SET = 210,
	UP_TLV_ZHDJ_LOCATION_SET = 211,
	UP_TLV_ZHDJ_LOCATION_GET  =  212,
	UP_TLV_ZHDJ_BIND = 213,
	UP_TLV_ZHDJ_TYPE = 214,
	UP_TLV_ZHDJ_DIR = 215,
};
//智皇电机开关状态
enum{
	ZHCJ_STATUS_OPEN = 0,
	ZHCJ_STATUS_CLOSE = 1,
	ZHCJ_STATUS_STOP = 3,
};

typedef struct {
	u_int32_t magic;
	u_int8_t index;
	u_int8_t type;
	u_int8_t pad[2];
}zhdj_bind_t;

//智皇电机cache报文
typedef struct {
	u_int32_t magic;//分组类型
	u_int8_t index;//0窗帘，1窗纱
	u_int8_t status;//Open = 0 :开启  STOP =1：停止  CLOSE = 3：关闭
	u_int8_t percent;//窗帘位置百分百
	u_int8_t type;
	u_int8_t max_timer_count;
	u_int8_t max_data_len;
	u_int8_t support_dir;//是否支持转向,0不支持，1表示支持
	u_int8_t dir;//0表示没换向，1表示已换向
	u_int32_t map;
	u_int8_t change_num[0];
}zhdj_cache_t;

typedef struct {
	u_int32_t magic;//分组类型
	u_int8_t index;//0窗帘，1窗纱
	u_int8_t status;//Open = 0 :开启  STOP =1：停止  CLOSE = 3：关闭
	u_int8_t percent;//窗帘位置百分百	
	u_int8_t type;//窗帘类型，0横着拉，1竖着拉
	u_int8_t support_dir;//是否支持转向,0不支持，1表示支持
	u_int8_t dir;//0表示没换向，1表示已换向
}cl_zhdj_info_t;

// 电王科技

enum {
	RF_DEV_DWKJ_TT_TYPE_SET_TIMER_POINT = 211,
	RF_DEV_DWKJ_TT_TYPE_QUERY_TIMER_POINT = 212,
} ;

enum {
	ACT_DWKJ_ONOFF, // 0 关机 1开机
	ACT_DWKJ_PERCENT,	// 调光
	// 上面ACTION 在接口cl_dwkj_sample_ctrl 使用

	ACT_DWKJ_TIMER,	// 定时器设置
};

typedef struct {
	u_int8_t syn;
	u_int8_t len;
	u_int8_t cmd;
} dwkj_uart_hdr_t;

typedef struct {
	u_int8_t onoff;	// 0 关机 1开机
	u_int16_t vol;	// 电压，单位V
	u_int16_t current;	// 电流，单位mA
	u_int16_t power;	// 功率，单位W
	u_int8_t percent;	// 调光百分比值，比如50就是50%
	u_int32_t degree;	//用电量，单位瓦时

	/*		
		故障描述
		B5；过温状态：1=过温；0=OK；
		B4：过压状态：1=过压；0=OK；
		B3：欠压状态：1=欠压；0=OK；
		B2：风机状态：1=风机卡死；0=OK；
		B1：短路状态：1=短路；0=OK；
		B0：开路状态：1=开路；0=OK；
		注：开路和短路都显示为灯故障。
	*/
	u_int8_t error;	

	// 下面数据SDK自己用
	u_int8_t v_timer;
} cl_dwkj_stat_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t count;
	u_int16_t point[12];
} ucp_dwkj_timer_t;

typedef struct {
	u_int16_t point;	// 表示一天内某一时间点，粒度为5分钟（例如287，时间点为287*5=1435，表示每天的23:55），有效取值范围为0-287（0:00 - 23:55），超出有效取值范围的值视为无效
	u_int8_t level;		// 表示调整灯的档位值，有效范围为1、2（1表示开机，2表示关机）或者50-110（比如50，对应50%档），超出有效范围的值视为无效
} cl_dwkj_timer_item_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t count; // 有多少个定时器，比如count 为5，表示下标0 - 4 数据有效
	cl_dwkj_timer_item_t item[12];
} cl_dwkj_timer_t;

typedef struct {
	cl_dwkj_stat_t stat;	// 基本状态

	// 定时器
	cl_dwkj_timer_t timer;	// 一共12个定时器

	// 下面数据SDK自己用
	cl_dwkj_timer_t timer_bk;	// 备份timer，收到设备发来的设置失败后，还原之前的
} cl_dwkj_info_t;

typedef struct {
	dwkj_uart_hdr_t hd;
	u_int8_t pad1[3];	// 暂时保留
	u_int8_t onoff;	// 0 关机 1开机
	u_int16_t vol;	// 电压，单位V
	u_int16_t current;	// 电流，单位mA
	u_int16_t power;	// 功率，单位W
	u_int8_t percent;	// 调光百分比值，比如50就是50%
	u_int32_t degree;	//用电量，单位瓦时
	u_int8_t pad2;	// 暂时保留
	u_int8_t error;	
	u_int8_t pad3[5];	// 暂时保留
	u_int8_t tail;
	
	u_int8_t v_timer;	
	u_int8_t pad;
} dwkj_cache_t;

// 情景遥控器

enum {
	RF_TT_SC_TYPE_SET_KEY = 200,
	RF_TT_SC_TYPE_QUERY_KEY = 201,
	RF_TT_SC_TYPE_PUSH_KEY = 202,
	RF_TT_SC_TYPE_SET_LOSS = 203,
};

enum {
	ACT_SC_SET_NAME,
	ACT_SC_SET_LOSS,	// 挂失value填1表示挂失 填0表示解锁恢复
};

enum {
	DWKJ_CONF_ID_NAME = 1,	// 按键ID配置
	COMM_TIMER_CONFI_ID,
	CDQJMB_CONF_ID,
	DHX_CONF_ID,
};

//魔乐单火线
enum {
	RF_TT_DHX_TYPE_SET_KEY = 200,
	RF_TT_DHX_TYPE_QUERY_KEY = 201,
	RF_TT_DHX_TYPE_PUSH_KEY = 202,
};

enum {
	ACT_DHX_SET_NAME,
};

typedef struct {
	u_int8_t valid;
	u_int8_t id;
	u_int8_t idx;
	u_int8_t name[32];
} dwkj_flash_key_t;


typedef struct {
	u_int8_t id;
	u_int8_t len;
	u_int8_t name[0];
} ucp_scene_controller_key_set_t;


typedef struct {
	u_int8_t id;
	u_int8_t len;
	u_int8_t name[0];
} ucp_scene_controller_key_push_t;

typedef struct {
	u_int8_t valid;	// 是否有效
	char name[32];
} scene_controller_key_item_t;

typedef struct {
	u_int8_t id;
	u_int8_t name[24];
} cl_scene_controller_key_set_t;

typedef struct {
	u_int8_t hw_ver;
	u_int8_t soft_ver;
	u_int16_t resv;
	u_int32_t svn;
} ucp_scene_controller_version_t;


typedef struct {
	u_int8_t is_low_battery;
	u_int8_t abc_battery;
	// 按键信息，分别对应ID 1 2 3 4
	scene_controller_key_item_t keys[4];

	// 硬件版本
	u_int8_t hw_ver;
	// 软件版本x.x.x
	u_int8_t soft_ver_mar;
	u_int8_t soft_ver_min;
	u_int8_t soft_ver_rev;
	// svn号
	u_int32_t svn;

	// 是否处于挂失状态
	u_int8_t is_loss;

	// 下面数据SDK自己用
	u_int8_t stat_valid;
	u_int8_t index_alarm;
	u_int8_t index_key[4];
} cl_scene_controller_info_t;

typedef struct {
	u_int32_t flag;
	u_int32_t hislog_index_current;
	u_int8_t hislog_count;
	u_int8_t abc_battery;
	u_int8_t index_alarm;
	u_int8_t index_key[4];
} scene_controller_cache_t;

//网关缓存镜像
typedef struct {
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t img_type;//1-macbee子设备app，2-macbee子设备rf  3下位机
	u_int8_t up_action;// 1-指定升级镜像,2-强制升级镜像。
	u_int8_t index;//下标，1开始
	u_int8_t reserve;
}cl_rfdev_img_cache_info_t;

typedef struct {
	u_int8_t img_num;
	u_int8_t reserve[3];
	cl_rfdev_img_cache_info_t cache[0];
}cl_rfdev_img_cache_query_t;


// 智能报警开关
typedef struct {
	u_int8_t enable;
	u_int8_t pad[3];
	/*
		
	网关sn，在智能报警开启的情况下，触发报警的设备，需要把该SN封装到广播报警报文中，
	执行报警设备收到广播报文后，与设备内部保存的gw_sn进行比较，一致则执行报警。 
	*/
	u_int64_t gw_sn;
} cl_rfdev_onekey_smart_alarm_t;

// 一键布放撤防
typedef struct {
	u_int8_t defense;	// 1 设置布防 0 撤防
	u_int8_t pad;		// 历史问题，必须填0xff。SDK来填
} cl_rfdev_onekey_set_defense_t;

// 一键开关报警执行设备
typedef struct {
	u_int8_t enable;	// 0 报警结束 1 报警开始
	/*
		
	Bit0：为0 -- 默认报警模式（本地配置），此时data_valid字段以后的数据无效
	Bit0：为1 -- 动态报警模式（根据报文），此时data_valid字段以后的数据有效
	*/
	u_int8_t data_valid;
	/*
		
	报警方式：
	0 -- 持续报警（永久报警）
	1 -- 报警一次，此时仅total_time有效
	2 -- 间歇报警，on_time和off_time必须为非0，表示一个周期内的开启和停止时间
			total_time为0，表示按照上述周期永久性报警，非0则表示在报警total_time以后停止 
	*/
	u_int8_t alarm_mode;

	u_int8_t on_time;	// 一个周期内的开启时间，单位秒，范围0-255
	u_int16_t off_time;	// 一个周期内的停止时间，单位秒，范围0-65535
	u_int16_t total_time;	// 报警时间总长度，单位秒，范围0-65535
} cl_rfdev_onekey_set_alarm_mode_t;

// 开关RF子设备
typedef struct {
	u_int8_t onoff;
	u_int8_t pad[3];
} cl_rfdev_onekey_set_onoff_t;

typedef enum {	
	OKT_SET_DEFENSE = 37,		
	OKT_ONOFF = 52,
	OKT_SMART_ALARM = 61,
	OKT_ALARM_MODE = 62,
//	OKT_ALARM_MODE = 211,
} ONEKEY_TYPE_T;

// 一键控制
typedef struct {
	u_int8_t type;	// 控制类型 OKT_XXX
	u_int8_t len;
	union {
		cl_rfdev_onekey_smart_alarm_t smart_alarm;
		cl_rfdev_onekey_set_defense_t set_defense;
		cl_rfdev_onekey_set_alarm_mode_t alarm_mode;
		cl_rfdev_onekey_set_onoff_t onoff;
	} ctrl;
} cl_rfdev_onekey_ctrl_t;

#pragma pack(pop)
    
/*
 功能:
	 控制门磁自动布防
 输入参数:
	 @dev_handle: 设备句柄
 输出参数:
	 无
 返回:
	 RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_ctrl_auto_guard(cl_handle_t slave_handle,cl_rf_auto_guard_info_t* stat);

/*
 功能:
    控制单灯状态
 输入参数:
    @dev_handle: 插座的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rf_lamp_ctrl_stat(cl_handle_t slave_handle,cl_rf_lamp_stat_t* stat);


/*
 功能:
 	控制灯
 输入参数:
	 @dev_handle: 插座的句柄
 输出参数:
	 无
 返回:
	 RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_lamp_remote_ctrl_stat(cl_handle_t gw_handle,u_int32_t remote_id,u_int8_t key_id,cl_rf_lamp_stat_t* stat);

/*
 功能:
    控制门锁
 输入参数:
    @dev_handle: 门锁的句柄
    @group_id: 和handle为二选一关系,分别表示组或设备操作，group_id不为0时，dev_handle填网关的id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rf_door_lock_ctrl(cl_handle_t dev_handle,u_int8_t group_id,u_int8_t type,u_int8_t action);

/////////////////////////////////////////////////
//有泰门锁
    
/*
 功能:
	 控制门锁
 输入参数:
	 @dev_handle: 门锁的句柄
	 @group_id: 和handle为二选一关系,分别表示组或设备操作，group_id不为0时，dev_handle填网关的id
 	 @passwd，关锁时密码置为全F，即：-1
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_yt_rf_door_lock_ctrl_lock(cl_handle_t dev_handle,u_int8_t group_id,u_int8_t on_off, u_int32_t passwd);

/*
 功能:
	 修改密码
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @old_passwd: 旧密码 new_passwd: 新密码
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_yt_rf_door_lock_modify_lock_passwd(cl_handle_t dev_handle,u_int32_t old_passwd, u_int32_t new_passwd);

/*
 功能:
	 新建密码
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @new_passwd: 新建的密码
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_yt_rf_door_lock_create_lock_passwd(cl_handle_t dev_handle, u_int32_t new_passwd);

/*
 功能:
	 友泰设置门锁遥控器
 输入参数:
	 @dev_handle: 门锁的句柄
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_yt_rf_door_lock_set_associate(cl_handle_t dev_handle);

/*
 功能:
	 设置门多久没关就报警
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @enable: 是否开启报警
 	 @timeout: 超时时间，单位分钟
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_door_lock_set_unlock_timeout(cl_handle_t dev_handle, u_int8_t enable, u_int8_t timeout);


enum {
    RFDOORLOCK_SETTINGS_WIFIUNLOCK = 0x1,
    RFDOORLOCK_SETTINGS_WIFILOCK = 0x2,
    RFDOORLOCK_SETTINGS_SUPPORTPUSHNOTIFY = 0x3,
    RFDOORLOCK_SETTINGS_SUPPORTNOTIFDOOROPEN = 0x4
};
/*
 功能:
	 设置wifi接入或者断开以后自动解锁或者上锁
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @type: 1 设置自动解锁 2 设置自动上锁
 	 @enable:  使能
 	 @starthour: 开始的时间
 	 @endhour: 结束的时间
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_door_lock_set_wifilock(cl_handle_t dev_handle, u_int8_t type, u_int8_t enable, u_int8_t starthour, u_int8_t endhour);

/*
 功能:
	 设置门锁遥控器
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @request: 设置参数，具体看cl_door_lock_controller_set_t 解释
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_door_lock_set_controller_info(cl_handle_t dev_handle, cl_door_lock_controller_set_t *request);

///*
// 功能:
//    设置门锁遥控器名称
// 输入参数:
//    @dev_handle: 门锁的句柄
// 输出参数:
// 无
// 返回:
// RS_OK: 成功
// 其他: 失败
// */
//CLIB_API RS cl_rf_door_lock_rc_ctrl(cl_handle_t slave_handle,cl_rf_door_lock_remote_t* lr);
    
/////////////////////////////////////////////////////////
    
// 通用接口
/*
 功能:
    查询历史消息
 输入参数:
    @dev_handle: 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_dev_query_history(cl_handle_t slave_handle,u_int32_t last_time);
    
/*
 功能:
    控制门锁
 输入参数:
    @dev_handle: RF设备的句柄
    @group_id: 和handle为二选一关系,分别表示组或设备操作，group_id不为0时，dev_handle填网关的id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rf_dev_com_ctrl(cl_handle_t slave_handle,u_int8_t group_id,u_int8_t group_type,u_int8_t ctrl_type,u_int8_t value);

/*
	功能: 设置报警时间，比如啥时候再开始啥的
		
	输入参数:
		@slave_handle: 设备的句柄
		@type: 1 : 设置报警开始时间0 设置报警结束时间
		@time: 到达对应type状态间隔时间，单位秒
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_dev_set_alarm_time(cl_handle_t slave_handle, u_int8_t type, u_int32_t time);

/*
	功能: 对于有些一旦报警后就会一直响应的设备，
	需要发送一个报警取消命令
		
	输入参数:
		@slave_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_dev_set_alarm_clc(cl_handle_t slave_handle);

/*
 功能: 空调红外控制相关
 
 输入参数:
 	@slave_handle: 设备的句柄
 	@type CT_WK_XXX
 输出参数:
 	无
 返回:
 	RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_dev_air_ir_ctrl(cl_handle_t slave_handle,u_int8_t type,u_int8_t value);

//////////////////////////////////////////////////////////////////////////////////
// 暖气阀
/*
	功能: 暖气阀的简单控制接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@action: 控制类型(ACT_HEATING_VALVE_T)
			ACT_HEATING_VALVE_ONOFF 控制开关机0 关机 1开机
			ACT_HEATING_VALVE_MODE 控制模式0 自动 1手动
			ACT_HEATING_VALVE_ANTI_LIME  防石灰 0 取消 1使能
			ACT_HEATING_VALVE_FROST_PROTECTION 霜冻保护 0 取消 1使能
			ACT_HEATING_VALVE_CHILD_LOCK 童锁 0 取消 1使能
			ACT_HEATING_VALVE_WINDOW 0 取消 1使能
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_simple_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int8_t value);


/*
	功能: 暖气阀的日期控制接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@date: 日期设置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_date_ctrl(cl_handle_t slave_handle, cl_heating_valve_param_date_t *date);


/*
	功能: 暖气阀的温度设置接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@date: 日期设置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_temp_ctrl(cl_handle_t slave_handle, cl_heating_valve_param_temp_t *temp);

/*
	功能: 设置某天的加热和经济时间点
		
	输入参数:
		@slave_handle: 设备的句柄
		@day: 	 星期几(取值1-7)
		@hh1;	 第一加热周期 小时 （取值范围：0 - 24）
		@hm1;	 第一加热周期 分钟 （取值范围：0 - 50，粒度10）
		@eh1;	 第一经济周期 小时（取值范围：0 - 24）
		@em1;	 第一经济周期 分钟（取值范围：0 - 50，粒度10）
		@hh2;	 第二加热周期 小时（取值范围：0 - 24）
		@hm2;	 第二加热周期 分钟（取值范围：0 - 50，粒度10）
		@eh2;	 第二经济周期 小时（取值范围：0 - 24）
		@em2;	 第二经济周期 分钟（取值范围：0 - 50，粒度10）
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_period_ctrl(cl_handle_t slave_handle, u_int8_t day, 
	u_int8_t hh1, u_int8_t hm1, 
	u_int8_t eh1, u_int8_t em1, 
	u_int8_t hh2, u_int8_t hm2, 
	u_int8_t eh2, u_int8_t em2);

////////////////////////////////////////////////////////////////////////////////////
// 凯特插座
/*
	功能:  凯特插座的简单控制接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@action: 控制类型(ACT_KTCZ__T)
			ACT_KTCZ_ONOFF 控制通断电0 断电 1通电
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_ktcz_simple_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int8_t value);

//////////////////////////////////////////////////////////////////////////////////// 通用探测器接口
// 



/*
	功能:  通用探测器控制接口，沿用cl_rf_dev_com_ctrl
		
	输入参数:
		@slave_handle: 设备的句柄
		@group_id: 暂时不用
		@group_type: 暂时不用
		@type: 控制类型，和value联合使用
			ACT_COM_DETECTOR_CTRL_DEFENSE 0: 撤防 1 布放
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

//CLIB_API RS cl_rf_dev_com_ctrl(cl_handle_t slave_handle,u_int8_t group_id,u_int8_t group_type,u_int8_t type,u_int8_t value)


/*
	功能:  一键布放撤防接口
		
	输入参数:
		@master_handle:网关的句柄
		@is_defense: 1 布放 0 撤防
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_dev_set_defense_batch(cl_handle_t master_handle, u_int8_t is_defense);

/*
	功能:  一键布放撤防设备配置
		
	输入参数:
		@master_handle:网关的句柄
		@num: 配置设备个数
		@sn: 配置的sn数组
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_dev_config_defense_batch(cl_handle_t master_handle, u_int8_t num, u_int64_t *sn);

/////////////////////////////////////////////////////////////////////////////////
// 汇泰龙
/*
	功能:  管理用户登陆
	需要根据随后的事件判断是否成功
	SAE_HTLLOCK_ADMIN_LOGIN_OK = 1218,		// 汇泰龙登陆成功
	SAE_HTLLOCK_ADMIN_LOGIN_FAILED = 1219, // 汇泰龙登陆失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_admin_login(cl_handle_t slave_handle, htllock_tt_admin_login_t *request);

/*
	功能:  设置临时PIN码
	需要根据随后的事件判断是否成功
	SAE_HTLLOCK_SET_PIN_OK = 1237,	// 汇泰龙设置PIN码成功
	SAE_HTLLOCK_SET_PIN_FAILED = 1238,	// 汇泰龙设置PIN码失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_set_pin(cl_handle_t slave_handle, htllock_tt_set_pin_t *request);

/*
	功能:  用户设置名字
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_NAME_OK = 1220,	// 汇泰龙设置用户名字成功
	SAE_HTLLOCK_SET_NAME_FAILED = 1221,// 汇泰龙设置用户名字失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_name(cl_handle_t slave_handle, htllock_tt_user_manage_set_name_t *request);

/*
	功能:  用户设置头像
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_PIC_OK = 1222,// 汇泰龙设置用户头像成功
	SAE_HTLLOCK_SET_PIC_FAILED = 1223,// 汇泰龙设置失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_pic(cl_handle_t slave_handle, htllock_tt_user_manage_set_pic_t *request);

/*
	功能:  用户设置关联
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_BIND_OK = 1224,// 汇泰龙关联用户的指纹、密码、扫描卡成功
	SAE_HTLLOCK_SET_BIND_FAILED = 1225, // 汇泰龙关联用户的指纹、密码、扫描卡失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_bind(cl_handle_t slave_handle, htllock_tt_user_manage_set_bind_t *request);

/*
	功能:  用户设置关联取消
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_UNBIND_OK = 1226,// 汇泰龙设置取消关联成功
	SAE_HTLLOCK_SET_UNBIND_FAILED = 1227,// 汇泰龙设置取消关联失败
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_unbind(cl_handle_t slave_handle, htllock_tt_user_manage_set_bind_t *request);

/*
	功能:  修改用户开关门提醒通知
	输入参数:
		@slave_handle: 设备的句柄
		@is_close: 是否关闭提醒
		@user_index:用户编号
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_remind_onoff(cl_handle_t slave_handle, u_int8_t isclose, u_int16_t user_index);
/*
	功能:  用户设置消息
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_INFO_NOTICE_OK = 1228, // 汇泰龙设置信息提醒成功
	SAE_HTLLOCK_SET_INFO_NOTICE_FAILED = 1229, // 汇泰龙设置信息提醒失败

	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_notice_info_set(cl_handle_t slave_handle, htllock_tt_info_notice_set_t *request);


/*
	功能:  一些基础的控制，音量

	输入参数:
		@request: 控制参数,op不用填
		@action:
			    ACT_HTLLOCK_SET_VOL,	// 设置音量1 高音 2 中音3低音 4 静音
			    ACT_HTLLOCK_SET_LANG,	// 设置语言
			    ACT_HTLLOCK_LOCAL_OPEN // 局域网开锁
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);

/*
	功能:  请求历史数据，返回的信息保存在 cl_htllock_info_t的history里面

	输入参数:
		@slave_handle: 设备的句柄
		@request: 请求参数,op 不用填，只需要填timestamp，使用本地时间戳
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_request_history(cl_handle_t slave_handle, htllock_tt_info_notice_get_history_t *request);
    
    
//////////////////////////////////////////////////////////////////////////////////
//// 空调贴API

/*
	 功能:  获取空调贴编码匹配状态信息
 
	 输入参数:
	 	stat
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_get_air_code_match_stat(cl_handle_t dev_handle,cl_rf_air_code_match_stat_t* stat);
/*
	 功能:  设置空调贴温度矫正值
 
	 输入参数:
	 	tmp:-5~5摄氏度
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_code_tmp_adjust(cl_handle_t slave_handle, int8_t tmp);

/*
	 功能:  设置空调贴led灯模式
 
	 输入参数:
	 	mode:1 智能模式 ,2 关闭led灯
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_code_led_mode(cl_handle_t slave_handle, u_int8_t mode);

/*
	 功能:  设置空调贴红外方向，sdk透传，app定义，大概4个方向，4个bit，每个bit用位来表示意义。
	 Bit0:1发射红外 0 不发送红外
 	 Bit1:1 大功率发射 0 小功率发射	 
 
	 输入参数:
	 	dir:方向参数，数组
	 	len:数组大小
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_code_dir(cl_handle_t slave_handle, u_int8_t *dir, u_int8_t len);

CLIB_API RS cl_set_air_ir_id(cl_handle_t slave_handle, u_int16_t id);

/*
	 功能:  设置空调贴红外方向自适应	 
 
	 输入参数:
	 	send_num:单方向发送次数，一共4个方向
	 	send_inter:发送间隔，秒
	 	send_timeout:单方向发送超时，一共超时要*4
	 	len:数组大小
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_code_dir_auto_adjust(cl_handle_t slave_handle, u_int8_t send_num, u_int8_t send_inter, u_int8_t send_timeout);

/*
	 功能:  检查开机功能 
 
	 输入参数:
	 	on:1启动，0停止
	 	timeout:检查超时，秒
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_code_check(cl_handle_t slave_handle, u_int8_t on, u_int8_t timeout);

/*
	 功能:  震动自动检查开关功能 
 
	 输入参数:
	 	step: 0-通知设备第一次检查，1-通知设备第二次检查
	 	onoff:1-代表当前空调处于开机状态，0-代表当前空调处于关机状态
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_shock_auto_check(cl_handle_t slave_handle, u_int8_t step, u_int8_t onoff);


/*
	 功能:  震动自动检查中间状态查询
 
	 输入参数:
		@slave_handle,从设备handle
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_shock_status_query(cl_handle_t slave_handle);

/*
	 功能:  红外自动检查中间状态查询
 
	 输入参数:
		@slave_handle,从设备handle
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_ir_status_query(cl_handle_t slave_handle);

/*
	 功能:  空调贴控制地址设置
 
	 输入参数:
		@slave_handle,从设备handle
		@addr:1-16
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_ir_addr(cl_handle_t slave_handle, u_int8_t addr);

/*
	 功能:  空调贴控制红外同步开关
 
	 输入参数:
		@slave_handle,从设备handle
		@onoff: 开关 0 关 1 开
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_air_ir_sync_onoff(cl_handle_t slave_handle, u_int8_t onoff);

//////////////////////////////////////////////////////////////////////////////////
//橙灯面板

/*
	 功能:  橙灯情景面板按键名称设置
 
	 输入参数:
		@slave_handle,从设备handle
		@index:按键index，1-6
		@name:以/0结束的字符串，可以为空，最长26字节
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_set_cdqjmb_name(cl_handle_t slave_handle, u_int8_t index, char *name);

//////////////////////////////////////////////////////////////////////////////////
// 夜狼声光报警器
/*
	 功能:  夜狼声光报警器灯状态控制
 
	 输入参数:
	 	@slave_hande: 从设备handle
	 	@stat: 具体看结构体定义
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_yllight_lamp_ctrl(cl_handle_t slave_hande, cl_rf_lamp_stat_t* stat);


/*
	 功能:  夜狼声光报警器配置默认报警方式
 
	 输入参数:
	 	@slave_hande: 从设备handle
	 	@config: 具体看结构体定义
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_yllight_alarm_config(cl_handle_t slave_hande, cl_yllight_alarm_config_t* config);



/*
	 功能:  夜狼声光报警器通用控制
 
	 输入参数:
	 	@slave_hande: 从设备handle
	 	@action: ACT_RF_YLLIGHT_XXX
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_yllight_com_ctrl(cl_handle_t slave_hande, u_int8_t action, u_int32_t value);


 
//////////////////////////////////////////////////////////////////////////////////
//// 智皇电机API
/*
	 功能:  智皇电机，窗帘状态设置
 
	 输入参数:
	 	status:Open = 0 :开启  Close =1：停止  Stop = 3：关闭
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_status_set(cl_handle_t slave_handle, u_int8_t status);

/*
	 功能:  智皇电机，窗帘位置设置
 
	 输入参数:
	 	location: 0~100,就是个百分百
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_location_set(cl_handle_t slave_handle, u_int8_t location);

/*
	 功能:  智皇电机，窗帘位置查询
 
	 输入参数:
		无
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_location_query(cl_handle_t slave_handle);

/*
	 功能:  智皇电机，窗帘绑定
 
	 输入参数:
	 	magic:xxx
	 	index:xxx
	 	type:0/1
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_bind(cl_handle_t slave_handle, u_int32_t magic, u_int8_t index, u_int8_t type);

/*
	 功能:  智皇电机，窗帘类型设置
 
	 输入参数:
	 	type: 0/1
	 	index:xxx
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_type_set(cl_handle_t slave_handle, u_int8_t type, u_int8_t index);

/*
	 功能:  智皇电机，窗帘类型设置
 
	 输入参数:
	 	dir: 0表示没有换向，1表示已换向
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_dir_set(cl_handle_t slave_handle, u_int8_t dir);


// 电王科技灯

/*
	 功能:  电王科技灯的基础控制
 
	 输入参数:
	 	type: 0/1
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);


/*
	 功能:  电王科技灯的定时器设置
 
	 输入参数:
	 	type: 0/1
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_zhdj_timer_ctrl(cl_handle_t slave_handle, cl_dwkj_timer_t *request);



// 电王科技灯

/*
	 功能:  电王科技灯的基础控制
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@action: ACT_DWKJ_ONOFF ACT_DWKJ_PERCENT
	 	@value: 配合action使用
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_dwkj_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);


/*
	 功能:  电王科技灯的定时器设置
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@request: 定时器设置
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_dwkj_timer_ctrl(cl_handle_t slave_handle, cl_dwkj_timer_t *request);


/*
	 功能:  情景遥控器设置按键名称
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@id: 情景遥控器按键ID，1-4
	 	@name: 情景遥控器名称，加上\0长度最多24
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_scene_controller_set_key(cl_handle_t slave_handle, u_int8_t id, char *name);

/*
	 功能:  情景遥控器的一些基本控制，目前支持挂失
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@action: ACT_SC_SET_LOSS value 填1表示挂失，0表示恢复解锁
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_scene_controller_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value);



/*
	 功能:  甲醛传感器设置阀值
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@type:阀值类型 0 国标 1 欧标2 自定义
	 	@threshold:  自定义阀值，单位μg/m3
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_jq_set_threshold(cl_handle_t slave_handle, u_int8_t type, u_int16_t threshold);

/*
	 功能:  甲醛传感器设置报警间隙
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@peroid: 报警间隙，单位分钟
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_jq_set_alarm_period(cl_handle_t slave_handle, u_int16_t period);

/*
	 功能:  甲醛传感器立即检测一次当前值
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_jq_flush_ch2o(cl_handle_t slave_handle);

/*
	 功能:  魔乐单火线设置路数自定义名称
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 	@id:路数，1-3 
	 	@name: 自定义名称，加上\0长度最多24
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_dhx_ml_set_key(cl_handle_t slave_handle, u_int8_t id, char *name);

/*
	 功能:  从设备运行时间查询
 
	 输入参数:
	 	@slave_handle: 从设备handle
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rf_com_runtime_query(cl_handle_t slave_handle);

/*
	 功能:  对从设备进行一键控制
	 输入参数:
	 	@gw_handle: 网关handle
	 	@ctrl: 控制参数
	 输出参数:
 		无
 	返回:
 		RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_rfgw_onekey_ctrl(cl_handle_t gw_handle, cl_rfdev_onekey_ctrl_t *ctrl);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_COM_RF_DEV_H */

