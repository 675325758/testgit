#ifndef	__CL_EQUIPMENT_H__
#define	__CL_EQUIPMENT_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

enum{
    CL_RT_BASE,
    CL_RT_SMOKE_DETECTOR = 1, /* 烟雾探测器 */
    CL_RT_GAS_LEAK_DETECTOR = 2, /* 气体感应探测器 */
    CL_RT_INFRARED_DETECTOR = 3, /* 红外探测器 */
    CL_RT_DOOR_SENSOR = 4, /* 门磁感应器 */
    CL_RT_WINDOW_SENSOR = 5, /* 窗户磁感应器 */
    CL_RT_INTELLIGENT_LOCK = 6, /* 智能锁 */
    CL_RT_EMERGENCY_BUTTON = 7, /* 紧急按钮 */
    CL_RT_DB_SMOKE_DETECTOR = 21, /* 双向烟雾探测器 */
    CL_RT_DB_GAS_LEAK_DETECTOR = 22, /* 双向气体感应探测器 */
    CL_RT_DB_INFRARED_DETECTOR = 23, /* 双向红外探测器 */
    CL_RT_DB_DOOR_SENSOR = 24, /* 双向门磁感应器 */
    CL_RT_DB_WINDOW_SENSOR = 25, /* 双向窗户磁感应器 */
    CL_RT_DB_INTELLIGENT_LOCK = 26, /* 双向智能锁 */
    CL_RT_DB_EMERGENCY_BUTTON = 27, /* 双向紧急按钮 */
    CL_RT_DB_MAX = 49,
    
    CL_RT_W_TV = 50,	/*WIFI转红外电视*/
    CL_RT_W_TVBOX = 51,	/*WIFI转红外机顶盒*/
    CL_RT_W_AIRCONDITION = 52,	/*WIFI转红外空调*/
    CL_RT_W_OTHER = 53,	/*WIFI转其他类型红外电器*/
    CL_RT_W_MAX = 99,
    
    CL_RT_CURTAIN =100,/*窗帘*/
    CL_RT_PLUG = 101,/*插座*/
    CL_RT_LAMP = 102,/*灯*/
    CL_RT_RF_CUSTOM = 103,/*自定义无线控制*/
    CL_RT_RF_CUSTOM_TV = 104, /*自定义红外转发控制电视*/
    CL_RT_RF_CUSTOM_TVBOX = 105, /*自定义红外转发控制机顶盒*/
    CL_RT_RF_CUSTOM_AIRCONDITION = 106,/*自定义红外转发控制空调*/
    CL_RT_RF_CUSTOM_INFR_CUSTOM = 107,    /*自定义红转自定义*/
    CL_RT_SOUNDLIGHT = 120, /* 声光报警报警器 */
    CL_RT_SCENE_CONTROLLER = 121, /* 情景遥控器 */
    CL_RT_RF_MAX = 149,
    
    CL_RT_DB_RF_LAMP = 150,	/* RF 双向灯板 */
    CL_RT_DB_RF_PLUG = 151,	/* RF 双向插座 */
    CL_RT_DB_RF_CURTAIN = 152, /* RF 双向窗帘 */
    CL_RT_DB_RF_DIMMING_LAMP = 154,/*RF双向调光灯*/
    CL_RT_DB_RF_SOUNDLIGHT = 155,/*双向声光报警器*/
    CL_RT_DB_RF_MAX = 199,
    
    CL_RT_CLOUD_AIRCONDITION = 200, /* 红外云空调 */
    CL_RT_CLOUD_TV = 201, /* 红外云电视 */
    CL_RT_CLOUD_STB = 202, /* 红外云机顶盒 */
    CL_RT_CLOUD_MAX = 249, /* 红外云最大值 */
    
    CL_RT_MAX,
};

/*电视键值*/
enum{
    RF_TV_KEYID_UP = 0x20, /*频道+*/
    RF_TV_KEYID_DOWN = 0x21,/*频道-*/
    RF_TV_KEYID_VOL_ADD=0x22,/*音量加*/
    RF_TV_KEYID_VOL_DEC = 0x23,/*音量减*/
    RF_TV_KEYID_MUTE = 0x24,/*静音*/
    RF_TV_KEYID_SWITCH = 0x25,/*电视／视频*/
    RF_TV_KEYID_POWER = 0x26/*电源*/
};
/*空调键值*/
enum{
    RF_AIR_KEYID_HEAT = 0x20,/*制热*/
    RF_AIR_KEYID_COOL = 0x21,/*制冷*/
    RF_AIR_KEYID_OFF = 0x22,/*关机*/
};

/*机顶盒键值*/
enum{
    RF_TVBOX_KEYID_UP = 0x20, /*频道+*/
    RF_TVBOX_KEYID_DOWN = 0x21,/*频道-*/
    RF_TVBOX_KEYID_VOL_ADD=0x22,/*音量加*/
    RF_TVBOX_KEYID_VOL_DEC = 0x23,/*音量减*/
    RF_TVBOX_KEYID_MUTE = 0x24,/*静音*/
    RF_TVBOX_KEYID_SWITCH = 0x25,/*电视／视频*/
    RF_TVBOX_KEYID_POWER = 0x26,/*电源*/
    RF_TVBOX_KEYID_EXIT = 0x27,/*退出*/
    RF_TVBOX_KEYID_MENU = 0x28,/*菜单*/
    RF_TVBOX_KEYID_OK = 0x29,/*退出*/
    RF_TVBOX_KEYID_ARROW_UP = 0x2A,/*上*/
    RF_TVBOX_KEYID_ARROW_DOWN = 0x2B,/*下*/
    RF_TVBOX_KEYID_ARROW_LEFT = 0x2C,/*左*/
    RF_TVBOX_KEYID_ARROW_RIGHT = 0x2D /*右*/
};
/*窗帘键值*/
enum{
    RF_CURTAIN_KEYID_ON = 0x1,
    RF_CURTAIN_KEYID_OFF = 0x2,
    RF_CURTAIN_KEYID_STOP = 0x3
};
/*灯和排插键值*/
#define SWITCH_ON_KEY_ID (101) /*总开*/
#define SWITCH_OFF_KEY_ID (102) /*总关*/
    /*开关拥有独立的开和关指令base*/

/*单开单关键值51-98  如: 
id=51   第一路开
id=52   第一路关
id= 53  第二路开，依次类推
*/

#define SWITCH_KEYID_M_BASE (51) 
#define SWITCH_KEYID_M_MAX  (98)

#define SWITCH_KEYID_S_MIN  (1)
#define SWITCH_KEYID_S_MAX  (50) /*反转控制 1-50*/
    
#define MAX_DIMMING_VALUE   (100)
#define MIN_DIMMING_VALUE   (0)

typedef struct {
    cl_obj_t obj;
    cl_handle_t equipment_handle; /*所属电器句柄*/
    u_int32_t key_id;
    bool had_learned; /*该按键是否学习*/
}cl_key_t;
    
typedef struct {
	bool push_enable; /* 是否使能消息推送 */
	bool sms_enable; /* 是否开启短消息推送 */
	bool isLearned; /*是否已经学习到报警信号*/
	u_int8_t phone_num; /*关联电话数量*/
	u_int8_t soundline_num; /* 关联的声光报警器个数 */
	u_int8_t soundline_on; /* 使能声光报警器 */
	char* alarm_msg; /*报警信息*/
	char** phone_list; /*关联的电话*/
	cl_handle_t *soundline; /* 关联的声光报警器 */
	cl_handle_t scene; /* 联动的情景，0为没有 */
}cl_alarm_info_t;
    
/* electrical equipment，电器、安防设备等 */
typedef struct {
    cl_obj_t obj;
    cl_handle_t area_handle; /*区域句柄*/
    cl_handle_t eq_001e_handle;/*关联从设备句柄*/
    u_int8_t dev_type;		/* CL_RT_XXX */
    u_int8_t key_count;     //按键数量
    u_int8_t is_more_ctrl;   //是否反转式控制 0：反转 1：单开单关
    u_int8_t is_rf_repeater; //1表示支持RF中续
    u_int8_t rf_repeater_on; //is_rf_repeater非0时有效，1表示处于中续状态，
    u_int8_t db_dimming_lamp_value;       //双向调光灯数值 /*25-100*/
    u_int16_t group_num;     //灯或排插路数或双向窗帘全开时间
    u_int16_t group_state;	//双向RF电器状态（每个bit对应一路状态）或双向窗帘当前位置
    u_int16_t local_id; //电器local_id
	u_int8_t match_id_num;	// 云空调匹配到多少个编码，最多4
    u_int8_t ac_type;	//云空调类型
	u_int16_t match_id[4];	//最多4个匹配到的ID号
	u_int16_t last_presskey;//新云空调上一次按下的键
    u_int16_t pad;
    cl_key_t** keys;            /*按键列表*/
    cl_alarm_info_t* alarm_info;    /*报警器信息*/
} cl_equipment_t;
    
typedef struct {
    char name[64]; //最多63字节，多余的自动截断
    cl_handle_t eq_001e_handle; /*创建红外电器时必须设置*/
    cl_handle_t area_handle;
    u_int8_t dev_type; /*电器或报警设备类型 CL_RT_XX */
    u_int8_t group_num; //灯和插座的路数，最大支持8路,
    u_int8_t is_more_ctrl; //是否反转式控制 0：反转 1：单开单关
    u_int8_t pad; 
}cl_equipment_add_info_t;
 
typedef struct {
    cl_handle_t eq_handle; /*电器或报警设备handle*/
    u_int32_t key_id; /*电器的按键handle,报警类忽略*/
    u_int32_t remain_time; /*剩余倒数时间，仅EE_KL_HOST_WAIT_TIME，EE_KL_TRY_WAIT_TIME有效*/
    int last_error; /*出现错误时，标志上次错误是什么原因*/
    int max_ajust_value; /*可微调射频信号最低值,仅EE_KL_RF_SUPPORT_AJUST有效*/
    int min_ajust_value; /*可微调射频信号最高值,仅EE_KL_RF_SUPPORT_AJUST有效*/
    bool is_support_ajust_code;/*是否支持微调,仅EE_KL_RF_SUPPORT_AJUST有效*/
    bool is_support_ajust_plus_width;/*是否脉宽调整,仅EE_KL_RF_SUPPORT_PLUS_WIDTH有效*/
    bool is_narrow_plus_width; /*当前脉宽调整值,是否是窄脉宽，否则就是宽脉宽*/
}cl_key_learn_t;

/*无线编码学习模式*/
typedef enum {
    KLM_LEARN_CODE_BY_HOST = 0x0,  /*智慧家庭主机学习电器的编码*/
    KLM_LEARN_CODE_BY_EQUIPMENT = 0x1 /*电器学习主机的编码，即通常说的对码*/
}KL_LEARN_MODE_T;

enum{
    ERR_KL_SAVE_FAIL = 660, /*保持学习按钮信息失败*/
    ERR_KL_NOTIFY_DEV_TO_LEARN_TIME_OUT, /*通知设备端进入学习状态超时*/
    ERR_KL_WAIT_USER_INPUT_TIME_OUT,/*用户按遥控器超时*/
    ERR_KL_RECV_ALARM_TIME_OUT,/*尝试接收学习到的报警信号超时，有可能未学习成功或者用户未触发报警信号*/
    ERR_KL_WAIT_TRY_RES_TIME_OUT,/*等待设备返回尝试结果超时*/
    ERR_KL_GEN_CODE_TIME_OUT,/*等待设备返回产生编码结果超时*/
    ERR_KL_STOP_TIME_OUT, //等待设备回应停止学习报文超时
    ERR_KL_PLUS_WIDTH_QUERY_TIME_OUT, //等待设备回应脉宽查询报文超时
    ERR_KL_PLUS_WIDTH_SET_TIME_OUT, //等待设备回应脉宽设置报文超时
    ERR_KL_RF_AJUST_TIMEOUT, //等待设备回应射频微调报文超时
    ERR_KL_DEVICE_BUSY_OR_OFFLINE, //设备忙，正在学习或者处理控制信号
    ERR_KL_DUPLICATE_REMOTE_CODE //学习到的报警编码重复了
};
    
/*
 注意: 电器和按键的增、删、改操作，统一在用户回调函数中通知.消息序列如下
 1->.收到对应操作是否成功的消息
 2->.收到设备信息变动的消息.UE_INFO_MODIFY
 */
    
enum {
    EE_BEGIN = 600,
    /*电器或安防设备添加成功*/
    EE_EQ_ADD_OK = EE_BEGIN+1,
    /*电器或安防设备添加失败*/
    EE_EQ_ADD_FAIL = EE_BEGIN+2,
    /*电器或安防设备删除成功*/
    EE_EQ_DEL_OK = EE_BEGIN+3,
    /*电器或安防设备删除失败*/
    EE_EQ_DEL_FAIL = EE_BEGIN+4,
    /*电器或安防设修改成功*/
    EE_EQ_MODIFY_OK = EE_BEGIN+5,
    /*电器或安防设修改失败*/
    EE_EQ_MODIFY_FAIL = EE_BEGIN+6,
    /*双向RF扫描成功*/
    EE_EQ_DB_RF_SCAN_OK = EE_BEGIN+7,
    /*双向RF扫描失败*/
    EE_EQ_DB_RF_SCAN_FAIL = EE_BEGIN+8,
    
    /*按键添加成功*/
    EE_EKEY_ADD_OK = EE_BEGIN+20,
    /*按键添加失败*/
    EE_EKEY_ADD_FAIL = EE_BEGIN+21,
    /*按键删除成功*/
    EE_EKEY_DEL_OK = EE_BEGIN+22,
    /*按键删除失败*/
    EE_EKEY_DEL_FAIL = EE_BEGIN+23,
    /*按键修改成功*/
    EE_EKEY_MODIFY_OK = EE_BEGIN+24,
    /*按键修改失败*/
    EE_EKEY_MODIFY_FAIL = EE_BEGIN+25,
    /*按键信号发送成功*/
    EE_EKEY_SEND_SINGAL_OK = EE_BEGIN+26,
    /*按键信号发送失败*/
    EE_EKEY_SEND_SINGAL_FAIL = EE_BEGIN+27,
    
    /*开始学习,等待用户按下学习的按钮,30s倒计时*/
    EE_KL_HOST_WAIT_TIME = EE_BEGIN+41,
    /*信号学习成功*/
    EE_KL_LEARN_OK = EE_BEGIN+42,
    /*主机生成信号成功*/
    EE_KL_GEN_CODE_OK = EE_BEGIN+43,
    /*确认信号倒计时,仅报警类设备有此消息*/
    EE_KL_TRY_WAIT_TIME =     EE_BEGIN+44,
    /*报警类设备用，表示尝试时识别到了报警信号*/
    EE_KL_RECV_ALARM_OK = EE_BEGIN+45,
    /*发送学习到或产生的信号成功*/
    EE_KL_TRY_SEND_OK = EE_BEGIN+46,
    /*保存学习(经过微调或调整脉宽后)到的信号成功*/
    EE_KL_SAVE_CODE_OK = EE_BEGIN+47,
    /*该电器无线信号支持微调*/
    EE_KL_RF_SUPPORT_AJUST = EE_BEGIN+48,
    /*微调成功*/
    EE_KL_RF_AJUST_OK = EE_BEGIN+49,
    /*支持脉宽*/
    EE_KL_RF_SUPPORT_PLUS_WIDTH = EE_BEGIN+50,
    /*脉宽调整成功*/
    EE_KL_PLUS_WIDTH_AJUST_OK = EE_BEGIN+51,
    /*停止学习成功*/
    EE_KL_STOP_OK = EE_BEGIN+52,
    /*出现错误*/
    EE_KL_ERROR = EE_BEGIN+53,

	// 情景联动
    EE_LINKAGE_SCENE_DEL_OK = EE_BEGIN + 60,
    EE_LINKAGE_SCENE_DEL_FAIL = EE_BEGIN + 61,
    EE_LINKAGE_SCENE_MODIFY_OK = EE_BEGIN + 62,
    EE_LINKAGE_SCENE_MODIFY_FAIL = EE_BEGIN + 63,
	
    EE_END = EE_BEGIN + 99
};
/***************************************************************************************************/
//每个设备最多支持配置10个手机号码
/*
     功能:
         设置主机报警手机号码
     输入参数:
         @user_handle: 的设备句柄
         @phone: 添加的手机号码
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败
     注意事项:

        收到 UE_MODIFY_ALARM_PHONE_OK 设备端添加成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
        收到 UE_MODIFY_ALARM_PHONE_FAIL 设备端添加失败
 */

CLIB_API RS cl_user_add_alarm_phone(cl_handle_t user_handle, const char* phone);

/*
     功能:
         删除主机报警手机号码
     输入参数:
         @user_handle: 的设备句柄
         @phone: 删除的手机号码
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败
     注意事项:
     
         收到 UE_MODIFY_ALARM_PHONE_OK 设备端添加成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
         收到 UE_MODIFY_ALARM_PHONE_FAIL 设备端添加失败
 */
CLIB_API RS cl_user_del_alarm_phone(cl_handle_t user_handle, const char* phone);

/*
     功能:
         报警器关联声光报警器
     输入参数:
         @eq_handle: 报警器的设备句柄
         @on: 是否开关
         @soundline: 要关联的声光报警器的句柄数组
         @soundline_count: 要关联的声光报警器的个数
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败
     注意事项:
     
         收到 UE_MODIFY_ALARM_PHONE_OK 设备端添加成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
         收到 UE_MODIFY_ALARM_PHONE_FAIL 设备端添加失败
 */
CLIB_API RS cl_eq_bind_soundlight(cl_handle_t eq_handle, bool on, 
				cl_handle_t *soundline, u_int8_t soundline_count);

/***************************************************************************************************/
// 电器管理
    
//1.只能在主设备上添加电器  2.报警设备不能发控制信号 3.可能需要预留立刻刷新电器和按键的接口
/*
     功能:
         添加电器或报警器
     输入参数:
         @slave_handle: 主设备句柄
         @eq_handle: 存储电器句柄的地址指针
         @info: 添加电器时需要的信息
     输出参数:
         @eq_handle: 新的电器设备句柄
     返回:
         RS_OK: 成功
     其他: 失败
     注意事项:
         目前红外转发器只支持001e
         收到 EE_EQ_ADD_OK 设备端添加成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
         收到 EE_EQ_ADD_FAIL 设备端添加失败
 
 */
CLIB_API RS cl_eq_add(cl_handle_t slave_handle,cl_handle_t* eq_handle,cl_equipment_add_info_t* info);

/*
     功能:
         修改电器所在区域
     输入参数:
         @eq_handle: 电器句柄
         @area_handle: 区域句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     	其他: 失败

	事件: EE_EQ_MODIFY_OK、EE_EQ_MODIFY_FAIL
 */
CLIB_API RS cl_eq_modify_area(cl_handle_t eq_handle, cl_handle_t area_handle);


/*
     功能:
         添加电器或报警器(使用二维码扫描来添加，不需要学习)
     输入参数:
         @slave_handle: 主设备句柄
         @eq_handle: 存储电器句柄的地址指针
         @json: 添加电器时需要的信息, json格式
     输出参数:
         @eq_handle: 新的电器设备句柄
     返回:
         RS_OK: 成功
     其他: 失败
     注意事项:
         收到 EE_EQ_ADD_OK 设备端添加成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
         收到 EE_EQ_ADD_FAIL 设备端添加失败
 
 */
CLIB_API RS cl_eq_add_by_json(cl_handle_t slave_handle,cl_handle_t* eq_handle, const char *json);

/*
     功能:
         删除电器或报警器
     输入参数:
         @eq_handle: 电器设备句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     注意事项:
     收到 EE_EQ_DEL_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EQ_DEL_FAIL 设备端处理失败
     
 */
CLIB_API RS cl_eq_del(cl_handle_t eq_handle);


/*
     功能:
         修改电器或报警器名称
     输入参数:
         @eq_handle: 电器设备句柄
         @new_name: 新的电器名称最长63字节
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EQ_MODIFY_FAIL 设备端处理失败
     
 */

CLIB_API RS cl_eq_modify_name(cl_handle_t eq_handle, const char* new_name);

/*
 功能:
    设置双向调光灯数据
 输入参数:
    @eq_handle: 电器设备句柄
    @key_id: 调光灯按键ID
    @value: 调光值(25-100之间)
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 
注意事项:
 收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
 收到 EE_EQ_MODIFY_FAIL 设备端处理失败
 
 */
CLIB_API RS cl_set_db_dimming_value(cl_handle_t eq_handle,u_int8_t value);

/*
     功能:
         设置报警器短信 (推送消息)
     输入参数:
         @eq_handle: 电器设备句柄
         @msg: 新的报警消息，最长63字节
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EQ_MODIFY_FAIL 设备端处理失败
     
 */
CLIB_API RS cl_eq_set_alarm_msg(cl_handle_t eq_handle, const char* msg);

/*
     功能:
         设置红外设备绑定到001E 红外转发器
     输入参数:
         @eq_handle: 电器设备句柄
         @slave_handle: 001E 设备句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EQ_MODIFY_FAIL 设备端处理失败
     
 */    
CLIB_API RS cl_eq_set_alarm_assist_001e(cl_handle_t eq_handle,cl_handle_t slave_handle);

/*
     功能:
         设置报警器关联的手机号码列表
     输入参数:
         @eq_handle: 电器设备句柄
         @phonelist: 设置的手机号码列表,字符串指针数组
         @phone_count：要设置多少个手机号码,最多支持10个
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     phonelist中的手机列表必须是cl_user_set_alarm_phones 设置的子集
     
     收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EQ_MODIFY_FAIL 设备端处理失败
     
 */ 

CLIB_API RS cl_eq_set_alarm_phones(cl_handle_t eq_handle,char** phonelist,u_int8_t phone_count);

/*
	这里需要注意的是: 短信不能单独开启! 开短信必开消息推送；关消息推送必关短信。
*/

/*
	!!!!!!!! 注意 !!!!!!!!
		本接口废弃，请使用cl_eq_enable_alarm_push 和 cl_eq_enable_alarm_sms

     功能:
         报警器开启或关闭短信推送/消息推送
     输入参数:
         @eq_handle: 电器设备句柄
         @enable: false 关闭true 开启
     输出参数:
         无
     返回:
		RS_OK: 成功
		其他: 失败
     
     注意事项:
		收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
		收到 EE_EQ_MODIFY_FAIL 设备端处理失败
     
		cl_dev_info_t->has_alarm_swich为false时，本接口控制的是短信和消息推送这两个
		cl_dev_info_t->has_alarm_swich为true时，本接口控制的是短信
 */    
CLIB_API RS cl_eq_enable_alarm(cl_handle_t eq_handle,bool enable);

/*
     功能:
         报警器开启或关闭消息推送
     输入参数:
         @eq_handle: 电器设备句柄
         @enable: false 关闭true 开启
     输出参数:
         无
     返回:
		RS_OK: 成功
		其他: 失败
     
     注意事项:
		收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
		收到 EE_EQ_MODIFY_FAIL 设备端处理失败
*/    
CLIB_API RS cl_eq_enable_alarm_push(cl_handle_t eq_handle,bool enable);

/*
     功能:
         报警器开启或关闭短信推送
     输入参数:
         @eq_handle: 电器设备句柄
         @enable: false 关闭true 开启
     输出参数:
         无
     返回:
		RS_OK: 成功
		其他: 失败
     
     注意事项:
		收到 EE_EQ_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
		收到 EE_EQ_MODIFY_FAIL 设备端处理失败

		cl_dev_info_t->has_alarm_swich为true时，才支持本功能。
*/    
CLIB_API RS cl_eq_enable_alarm_sms(cl_handle_t eq_handle,bool enable);

/*
     功能:
       释放报警器信息数据
     输入参数:
         @ai, 报警器信息指针
     输出参数:
         无
     返回:
         RS_OK: 无
     其他: 失败
     
     注意事项:
     无
     
 */  

CLIB_API void cl_alarm_info_free(cl_alarm_info_t* ai);

/***************************************************************************************************/
// 电器按键管理
/*
     功能:
         添加按键
     输入参数:
         @eq_handle: 电器设备句柄
         @key_id: 用户自由指定,建议各平台定义相同
         @key_name: 按键名称，最长63字节
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     key_id 定义可以参考本头文件上端key定义
     
     收到 EE_EKEY_ADD_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EKEY_ADD_FAIL 设备端处理失败
     
 */   

CLIB_API RS cl_key_add(cl_handle_t eq_handle,u_int32_t key_id, const char* key_name);

/*
     功能:
         删除按键
     输入参数:
         @eq_handle: 电器设备句柄
         @key_id: 需要删除的按键id
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     收到 EE_EKEY_DEL_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EKEY_DEL_FAIL 设备端处理失败
     
 */   
CLIB_API RS cl_key_del(cl_handle_t eq_handle,u_int32_t key_id);

/*
     功能:
         修改按键名称
     输入参数:
         @eq_handle: 电器设备句柄
         @key_id: 需要修改的按键id
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     
     收到 EE_EKEY_MODIFY_OK 设备端处理成功 ＝》接收到 UE_INFO_MODIFY 后可更新信息
     收到 EE_EKEY_MODIFY_FAIL 设备端处理失败
     
 */ 
CLIB_API RS cl_key_modify_name(cl_handle_t eq_handle,u_int32_t key_id, const char* new_name);

/*
     功能:
         发送控制信号
     输入参数:
         @eq_handle: 电器设备句柄
         @key_id: 需要控制的按键id
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     
     收到 EE_EKEY_SEND_SINGAL_OK 设备端处理成功
     收到 EE_EKEY_SEND_SINGAL_FAIL 设备端处理失败
     
 */ 
CLIB_API RS cl_key_send_ctrl_singal(cl_handle_t eq_handle,u_int32_t key_id);


/***************************************************************************************************/
// 电器按键学习
/*
*******************************************
回调消息通用处理:
{
    cl_key_learn_t* kl = cl_key_learn_get_stat(xx_u_handle);
    
    switch(event){
       case EE_KL_HOST_WAIT_TIME:
         显示倒计时时间
         break;
         default:
           break;
    }
}
********************************************
红外转发学习流程
1. 添加好绑定001e的红外转发电器。（仅支持001e）
2. 调用cl_set_key_learn_callback
3. 调用cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_EQUIPMENT
4.调用cl_kl_gen_code 产生编码，让红外转发器学习到RF 信号,(将收到EE_KL_GEN_CODE_OK事件)
5. 按红外遥控器上对应按钮，让红外转发器学到红外信号
6. 调用cl_kl_try_ctrl 尝试控制(可选)，(将收到EE_KL_TRY_SEND_OK 事件)
7.调用cl_kl_save_learn_code 保存(将收到EE_KL_SAVE_CODE_OK 事件)
**************************************************************
报警器学习流程
1. 先调用cl_eq_set_alarm_msg 设置好报警消息.(建议在创建完成后，后台自动设置)
2. 调用cl_set_key_learn_callback
3. 调用cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_HOST key_id =0 (将收到EE_KL_HOST_WAIT_TIME 事件)
4. 等待用户触发报警器(收到EE_KL_LEARN_OK 表示学习成功)
5. 收到EE_KL_SAVE_CODE_OK 事件(报警类设备自动保存信号)
6. 调用cl_kl_try_ctrl 尝试控制(可选)，(收到EE_KL_TRY_WAIT_TIME 表示等待用户再次
触发报警器，收到EE_KL_RECV_ALARM_OK 事件表示主机匹配报警信号成功)
************************************************************************************
RF 对码学习流程
1. 调用cl_set_key_learn_callback  
2. 调用cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_EQUIPMENT 
3.调用cl_kl_gen_code 产生编码(将收到EE_KL_GEN_CODE_OK事件)
4. 调用cl_kl_try_ctrl 尝试控制(将收到EE_KL_TRY_SEND_OK 事件)
5.调用cl_kl_save_learn_code 保存(将收到EE_KL_SAVE_CODE_OK 事件)
************************************************************************************
RF主机学习编码流程
1. 调用cl_set_key_learn_callback  
2. 调用cl_key_learn_start learn_mode=KLM_LEARN_CODE_BY_HOST (将收到EE_KL_HOST_WAIT_TIME 事件)
3. 等待用户按电器遥控器(收到EE_KL_LEARN_OK 表示学习成功)
4. 调用cl_kl_try_ctrl 尝试控制(可选)，(收到EE_KL_TRY_SEND_OK 事件表示主机发送控制信号成功)
5.调用cl_kl_save_learn_code 保存(将收到EE_KL_SAVE_CODE_OK 事件)
************************************************************************************
RF 信号微调
1.学习成功，收到EE_KL_RF_SUPPORT_AJUST，表示支持信号微调。
2.在cl_key_learn_t 中取得微调范围。
3. 调用cl_kl_ajust_code 进行微调
************************************************************************************
脉宽调整
1. 对码模式中，收到EE_KL_RF_SUPPORT_PLUS_WIDTH，表示支持脉宽调整
2.在cl_key_learn_t 中取得当前脉宽
3.  调用cl_kl_set_plus_width 设置脉宽
*/


/*
     功能:
         设置按键学习回调参数，每用户在程序生存期调用1次就可以
     输入参数:
         @user_handle: 设备句柄
         @callback: 回调函数地址
         @callback_param: 回调函数参数
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     无
 */ 

CLIB_API RS cl_set_key_learn_callback(cl_handle_t user_handle,cl_callback_t callback,void* callback_param);

/*
     功能:
         设置学习参数并开始
     输入参数:
         @user_handle: 设备句柄
         @eq_handle; 电器或报警器句柄
         @key_id: 电器key_id  ,报警器时置0
         @learn_mode : 学习模式
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     每次学习必须调用
 */ 
CLIB_API RS cl_key_learn_start(cl_handle_t user_handle,cl_handle_t eq_handle,u_int32_t key_id,
                               KL_LEARN_MODE_T learn_mode);

/*
     功能:
         通知设备发送学习到(包括微调后)的信号
         或通知设备准备接收报警器信号
     输入参数:
         @user_handle: 设备句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     无
 */ 

CLIB_API RS cl_kl_try_ctrl(cl_handle_t user_handle);

/*
     功能:
         对码模式或红外转发模式下产生编码
     输入参数:
         @user_handle: 设备句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     无
 */ 
CLIB_API RS cl_kl_gen_code(cl_handle_t user_handle);

 /*
     功能:
         停止学习
     输入参数:
         @user_handle: 设备句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
     学习完成后必须调用(无论失败还是成功)
 */ 
CLIB_API RS cl_key_learn_stop(cl_handle_t user_handle);

/*
     功能:
         微调RF 信号
     输入参数:
         @user_handle: 设备句柄
         @ajust_value: 需要微调的值
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
 */ 
CLIB_API RS cl_kl_ajust_code(cl_handle_t user_handle,int ajust_value);

/*
     功能:
         设置脉宽
     输入参数:
         @user_handle: 设备句柄
         @is_narrow:  false:设置为宽脉宽
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
 */ 
CLIB_API RS cl_kl_set_plus_width(cl_handle_t user_handle,bool is_narrow);

/*
     功能:
         查询当前脉宽
     输入参数:
         @user_handle: 设备句柄
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
 */ 

CLIB_API RS cl_kl_save_learn_code(cl_handle_t user_handle);

/*
     功能:
         获取当前学习状态和参数
     输入参数:
         @user_handle: 设备句柄
     输出参数:
         无
     返回:
         非空: 成功
     其他: 失败
     
     注意事项:
 */ 
CLIB_API cl_key_learn_t* cl_key_learn_get_stat(cl_handle_t user_handle);

/*
     功能:
        释放cl_key_learn_get_stat 得到的数据
     输入参数:
         @stat: cl_key_learn_get_stat 返回指针
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
 */ 

CLIB_API void cl_kl_stat_free(cl_key_learn_t* stat);

/*
     功能:
        主设备主动对周围双向电器进行一次绑定尝试
     输入参数:
         无
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     
     注意事项:
 */ 
CLIB_API RS cl_user_scan_db_rf(cl_handle_t user_handle);

/*
     功能:
         设置报警器联动情景
     输入参数:
         @eq_handle: 报警器的设备句柄
         @scene_handle: 要联动的情景。
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败

     可能收到事件:
     	EE_LINKAGE_SCENE_MODIFY_OK 修改情景联动成功
     	EE_LINKAGE_SCENE_MODIFY_FAIL 修改情景联动失败
 */
CLIB_API RS cl_eq_linkage_scene_set(cl_handle_t eq_handle, cl_handle_t scene_handle);

/*
     功能:
         删除报警器联动情景
     输入参数:
         @eq_handle: 报警器的设备句柄
         @scene_handle: 之前联动的情景。
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败

     可能收到事件:
     	EE_LINKAGE_SCENE_DEL_OK 删除情景联动成功
     	EE_LINKAGE_SCENE_DEL_FAIL 删除情景联动失败
 */
CLIB_API RS cl_eq_linkage_scene_del(cl_handle_t eq_handle, cl_handle_t scene_handle);

/*
     功能:
         刷新电器状态
     输入参数:
         @eq_handle: 电器设备句柄
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败

     可能收到事件:
     	UE_INFO_MODIFY 
 */
CLIB_API RS cl_eq_refresh(cl_handle_t eq_handle);

/*
     功能:
         RF中续模式开关
     输入参数:
         @eq_handle: 电器设备句柄
         @repeater_on: 1表示设置为中续模式，0表示取消中续模式
     输出参数:
         无
     返回:
         RS_OK: 发送请求报文成功
     其他: 失败

     可能收到事件:
     	UE_INFO_MODIFY 
 */
CLIB_API RS cl_eq_rf_repeater_set(cl_handle_t eq_handle, u_int8_t repeater_on);
/***************************************************************************************************/

#ifdef __cplusplus
}
#endif 

#endif



