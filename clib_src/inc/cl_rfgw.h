#ifndef	__CL_RFGW_H__
#define	__CL_RFGW_H__

#include "cl_com_rf_dev.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define PSK_LEN 8
#define MAX_GW_COUNT 16
#define RF_MAX_NAME 16
#define MAX_SLAVE_PER_GW 128
    
//扩展类型
enum{
    RF_EXT_TYPE_GW = 0x01, //网关扩展类型
    RF_EXT_TYPE_LIGHT = 0x21,//七色灯0x21
    RF_EXT_TYPE_LED_LAMP = 0x22,//LED灯
    RF_EXT_TYPE_DOOR_LOCK = 0x23, //智能门锁/门闩
    RF_EXT_TYPE_DOOR_MAGNET = 0x24, //智能门磁
    RF_EXT_TYPE_DHX = 0x25, //单火线开关
    RF_EXT_TYPE_YT_DOOR_LOCK = 0x26, //友泰门锁
    RF_EXT_TYPE_HM_MAGENT = 0x27,//海曼门磁
    RF_EXT_TYPE_HM_BODY_DETECT = 0x28,//海曼人体探测
    RF_EXT_TYPE_HM_ENV_DETECT = 0x29,//海曼温湿度探测

	RF_EXT_TYPE_DWHF = 0x2A,//电威合封芯片灯
	
    RF_EXT_TYPE_DOOR_MAGNETV2 = 0x2b,	// V2硬件版本的门磁
    RF_EXT_TYPE_KTCZ = 0x30,	// 凯特插座
    RF_EXT_TYPE_HTLLOCK = 0x31,	// 汇泰龙云锁
    RF_EXT_TYPE_GAS = 0x32,		// 气体检测
    RF_EXT_TYPE_QSJC = 0x33,	// 水检测
    RF_EXT_TYPE_DWYK = 0X34,	//电微遥控器
    RF_EXT_TYPE_HEATING_VALVE = 0x35,	// 暖气阀
    RF_EXT_TYPE_HMCO = 0x36,	// 一氧化碳检测    
    RF_EXT_TYPE_JQ = 0X37,	// 甲醛传感器
    RF_EXT_TYPE_HMYW = 0x38,	// 烟雾检测
    RF_EXT_TYPE_HMQJ = 0x39,	// 海曼智能求救呼叫器
    RF_EXT_TYPE_SCENE_CONTROLLER = 0x40,	// 情景遥控器
    RF_EXT_TYPE_WK_AIR = 0x41, //空调贴
    RS_EXT_TYPE_ZHDJ = 0x42,//智皇电机
    RF_EXT_TYPE_YLLOCK = 0x43,//夜狼门磁
    RF_EXT_TYPE_DWKJ = 0x44,	// 电王科技
    RF_EXT_TYPE_YLTC = 0x45,//夜狼红外
    RF_EXT_TYPE_YLWSD = 0x46, //夜狼温湿度
    RF_EXT_TYPE_YLSOS = 0x47,//夜狼sos
    RF_EXT_TYPE_DWYKHF = 0x48,//电微合封遥控器
    RF_EXT_TYPE_YLLIGHT = 0x4a,	// 夜狼声光报警器
    RF_EXT_TYPE_DHXZH = 0x4b, //智皇单火线
    RF_EXT_TYPE_DHXCP = 0x4c, // 橙朴单火线
    RF_EXT_TYPE_DWYSTGQ = 0x4d,	// 电威音速调光器

	RF_EXT_TYPE_LAMP_START = 0x50,	// 电威灯系列开始类型
	RF_EXT_TYPE_LAMP_END = 0x5f,	// 电威灯系列结束类型

	
    RF_EXT_TYPE_WK_AIR2 = 0x61, //格力中央空调
    RS_EXT_TYPE_YSD = 0x62, //音速灯
    RS_EXT_TYPE_CDQJMB = 0x63, //橙灯情景面板
    RF_EXT_TYPE_LIGHT_SENSE = 0X64,	// 光照感应
    RF_EXT_TYPE_DHXML = 0x66, //魔乐单火线
    RF_EXT_TYPE_DEMO = 0x67,	// 演示用从设备
    RF_EXT_TYPE_S9 = 0x68,		// S9网关(花瓶)
    RF_EXT_TYPE_LHX = 0x6a,		// RF零火线
    RF_EXT_TYPE_LHX_CONTROLLER = 0x6b,	// RF零火线遥控器
    RF_EXT_TYPE_WUANS6 = 0x6c,	// 悟安S6智能红外人体感应
};
//组数据类型
enum{
    GDT_NONE_SPCAIL = 0,
    GDT_LAMP_COLOR_CTRL
};

//每种类型
enum {
	UP_STATUS_NONE = 0,
	UP_STATUS_NEED_UPGRADE,
	UP_STATUS_UPGRADING,
};

typedef struct{
	u_int64_t sn;
	u_int8_t subtype;
	u_int8_t extype;
	u_int8_t pad[2];
}cl_rfgw_dev_find_t;

//RGB三色灯状态位
#define	BIT(n)	(1 << (n))
#define RGB_WORK_R BIT(0)
#define RGB_WORK_G BIT(1)
#define RGB_WORK_B BIT(2)
//门磁状态比特位，1表示开门，0表示关门
#define MC_WORK_OPEN BIT(3)

//分组类型
enum{
    RF_GPT_KNOWN,
    RF_GPT_LAMP, //灯组
    RF_GPT_DOOR_LOCK, //门锁组
    RF_GPT_DOOR_MAGNET,//门磁组
    RF_GPT_DHX_SWITCH, //单火线开关组
    RF_GFT_HM_MAGNET,//海曼门磁组
    RF_GFT_DWKJ_LAMP,	// 电王科技灯组
};

typedef struct{
	u_int8_t work;
	u_int32_t time;
}cl_rfdev_work_t;
    
/* 设备版本信息 */
typedef struct {
    u_int8_t major;	/* 主版本 */
    u_int8_t minor;	/* 次版本 */
    u_int8_t revise;	/* 修订版本 */
    u_int8_t pad;		/* 填充字节 */
} ud_version_t;

typedef struct{
	u_int8_t group_id;
	u_int8_t group_type;
	u_int8_t dev_cnt;
	u_int8_t query_dev;//非0表示查询过分组成员
	u_int8_t reserved;
	u_int8_t pad[3];
	u_int8_t name[RF_MAX_NAME];
	u_int64_t dev_sn[128];
}cl_dev_group_t;
    
typedef struct {
    u_int8_t slave_count; //灯数量
    u_int8_t real_count; //实际数量
    u_int8_t lamp_type; //灯类型，按键控制按照这个显示页面
    u_int8_t pad;
    u_int32_t* slave_handle;
}cl_lamp_remote_key_info;

//电威灯，遥控器信息
typedef struct {
    u_int32_t handle; //网关句柄
    u_int32_t r_id; //遥控器 id；
    u_int8_t lamp_type; //灯类型，遥控器控制按照这个显示页面
    u_int8_t pad[3];
    cl_lamp_remote_key_info key[4];
}cl_lamp_remote_info_t;

//网关外设数据结构
//夜狼数据定义
enum {
	RFDEV_SCM_TYPE_YL = 1,//夜狼网关外设类型
};
typedef struct {
	u_int8_t is_guard; //是否处于布防状态
	u_int8_t battery;//0 —100
	u_int8_t power;//0电池，1外接电源
	u_int8_t sos_on;//SOS报警指令 ：0，无；1，SOS报警。
	u_int8_t key_info;//按键信息：0 — 255，按键逻辑反馈
	u_int8_t door_voice;//门铃声音大小：0 — 100
	u_int8_t alarm_voice;//门铃声音大小：0 — 100
	u_int8_t time;//警报时间：0 - 255秒
}cl_rfdev_scm_yl_t;

typedef struct {
	u_int8_t valid;
	union {
		cl_rfdev_scm_yl_t yl_info;
	}rfdev_scm_dev_data;
}cl_rfdev_scm_t;

// 花瓶网关
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
    u_int32_t r_id;
} hpgw_lamp_ctrl_param_t;


// 花瓶网关的一些配置信息
typedef struct {
	u_int8_t name[24];		// 用户名
	u_int64_t phome_number;	// 手机号
} cl_hpgw_phone_user_t;

typedef struct {
	u_int8_t support_appinfo;	// 是否支持推送断网断电的APP信息
	u_int8_t support_sms;		// 是否支持短信
	u_int8_t sms_lang;			// 短信语言 0 中文 1 英文
	u_int8_t phone_user_num;	// 手机用户个数，最多16个
	cl_hpgw_phone_user_t users[16];

	cl_rf_lamp_t lamp_stat;	// 灯状态
} cl_hpgw_info_t;

typedef struct{
	// 花瓶网关
	cl_hpgw_info_t hpinfo;
	u_int8_t support_hpinfo;	// 支持花瓶网关相关配置
	
	u_int8_t commpat;//0默认兼容级别；1~255，兼容级别，数字越大，越不兼容老版本，而使用新特性。
	u_int8_t channel;//0默认信道；1~255为信道号，2400+信道号为网关使用频率。
	u_int8_t is_upgrade;//0没有升级中，1在升级中
	
	u_int8_t dev_group_cnt;
	cl_dev_group_t *dev_group;
    u_int8_t lamp_remote_cnt;
    cl_lamp_remote_info_t* lr_info;
	u_int8_t upgrade_status[D_T_MAX];	//rt升级用数据
	char *upgrade_url[D_T_MAX];//升级url

	u_int8_t img_cache_num;
	cl_rfdev_img_cache_info_t *pimg_cache;

	//网关透传外设数据
	cl_rfdev_scm_t rfdef_scm_dev;
}cl_gw_info_t;

typedef struct {
	u_int32_t id;			// 不用关心
	u_int32_t record_time;	// 记录时间
	u_int16_t object;	// 不用关心
	u_int16_t type; 	// 报警类型，具体见CAT_TYPE_XXX
	u_int32_t value;	// 不同类型意思不一样
} cl_rf_dev_comm_alarm_info_t;

typedef struct {
	u_int8_t type;
	u_int8_t value;
	u_int8_t ex_type;
	u_int8_t ex_value;
	u_int32_t timestamp;
	u_int8_t valid;	// 是否有效
} cl_rf_dev_comm_history_item_t;

typedef struct {
	// 下面两个数据当SAE_RF_DEV_COMM_HISTORY_SUMMARY 事件产生时更新
	u_int32_t index_current;	// 历史记录当前索引，例如：index_current == 400；那么当前历史记录最大索引为400
	/*
		
		历史记录条数，例如：max_count == 100；那么设备当前存有的历史记录为100条
		例如：index_current = 1000， max_count = 100，那么有效的历史记录索引为：
			  901 -- 1000
	*/
	u_int32_t max_count;			

	// 下面三个数据当SAE_RF_DEV_COMM_HISTORY_ITEM事件产生时更新
	u_int16_t index;	// 表示该条历史记录的索引值低16位（高16位被砍掉）
	u_int16_t n;		// 本次一共多少条历史记录，最多3条
	cl_rf_dev_comm_history_item_t items[3];
} cl_rf_dev_comm_history_info_t;

typedef struct _cl_rfdev_status_s{
	u_int64_t ctrl_total;
	u_int32_t ctrl_min;
	u_int32_t ctrl_max;
	u_int32_t ctrl_ok;
	u_int32_t ctrl_fail;
	u_int32_t ctrl_msec;
	cl_rfdev_work_t *work_list;
	u_int32_t rfrx; // from RF device
	u_int16_t linkretry; // from RF device
	u_int16_t work_list_cnt;
	u_int8_t work;  // from RF device
	u_int8_t is_ctrl;
    cl_slave_data_type d_type; //数据类型
    //////////////////////////////////////
    ud_version_t soft_ver; //终端软件版本
    ud_version_t hardware_ver; //终端硬件版本
    ////////////////////////////////////
    // 通用的报警信息，流程一般是SDK通过cache知道有报警消息，然后主动去查询
    cl_rf_dev_comm_alarm_info_t cai;

	// 通用的历史记录
	cl_rf_dev_comm_history_info_t chi;

	/*
		
	相应能力bit位值为1表示支持，0表示不支持
	bit0:布防/撤防控制能力
	bit1:布防/撤防状态上报能力
	bit2:触发报警能力
	bit3:执行报警能力
	bit4:开/关控制能力
	bit5:开/关状态上报能力
	bit6:智能报警能力
	*/
	u_int8_t dev_support;
	
    union{
        cl_door_lock_info_t door_lock_info;
        cl_rf_lamp_t lamp_info;
        cl_door_magnet_info_t door_magnet_info;
        cl_dhx_switch_info_t dhx_info;
        cl_hm_body_info_t hb_info; //海曼人体探测
        cl_hm_temp_hum_info_t ht_info; //海曼温湿度
        cl_heating_valve_info_t hv_info;	// 悟空暖气阀
        cl_ktcz_info_t kt_info;				// 凯特插座
        cl_com_detector_info_t cd_info;		// 通用的探测设备
        cl_htllock_info_t hl_info;	// 汇泰龙锁
        cl_wk_air_info_t wk_air_info; //悟空空调贴
        cl_zhdj_info_t zhdj_info;//智皇电机
        cl_dwkj_info_t dwkj_info;	// 电王科技
        cl_scene_controller_info_t sc_info;	// 情景遥控器
        cl_yllight_info_t yllight_info;	// 夜狼声光报警器
        cl_cdqjmb_info_t cdqjmb_info;//橙灯情景模板
        cl_jq_info_t jq_info;	// 甲醛传感器
        cl_light_sense_info_t ls;	// 光感
    }dev_priv_data;
}cl_rfdev_status_t;


typedef struct {
	ud_version_t ver;
	ud_version_t upver;
}cl_rf_dev_debug_info_t;


/*
	功能:
		指示网关进入配对模式
	输入参数:
		@gw_handle: 网关的句柄
		@timeout: 进入配对模式后的超时时间
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rfgw_join(cl_handle_t gw_handle, u_int16_t timeout);
//UE_RFGW_DEV_FIND
CLIB_API cl_rfgw_dev_find_t *cl_rfgw_get_join_dev(cl_handle_t gw_handle, u_int8_t *cnt);
CLIB_API void cl_rfgw_free_join_dev(cl_rfgw_dev_find_t *dev);
CLIB_API RS cl_rfgw_join_action(cl_handle_t gw_handle, u_int64_t dev_sn, u_int16_t accept);
CLIB_API RS cl_rfgw_group(cl_handle_t *gw_handle, u_int8_t gw_count, u_int8_t *psk);

/*
	功能:
		删除已经配对的设备
	输入参数:
		@gw_handle: 网关句柄
		@dev_handle: 设备句柄数组
		@cnt: 设备句柄数量，最多32个
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rfgw_dev_delete(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt);

/*
	功能:
		删除已经配对的所有设备
	输入参数:
		@gw_handle: 网关句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rfgw_dev_delete_all(cl_handle_t gw_handle);


/*
	功能:
		从网关查询设备列表
	输入参数:
		@gw_handle: 网关的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/

CLIB_API RS cl_rfgw_query_devlist(cl_handle_t gw_handle);
/*
	功能:
		发送透传数据到设备
	输入参数:
		@dev_handle: 设备的句柄
		@data: 透传数据
		@len: 透传数据长度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rfgw_set_tt(cl_handle_t dev_handle, u_int8_t *data, u_int16_t len);

/*
	功能:
		接收设备透传数据
	输入参数:
		@dev_handle: 设备的句柄		
	输出参数:
		@len: 透传数据长度
		无
	返回:
		数据地址: 成功
		NULL: 失败
	说明:
		UE_RFGW_DEV_TT 事件时调用本函数

*/
CLIB_API u_int8_t *cl_rfgw_get_tt(cl_handle_t dev_handle, u_int16_t *len);
CLIB_API void cl_rfgw_free_tt(u_int8_t *data);

/*
	功能:
		控制设备电源
	输入参数:
		@handle: 设备的句柄	控制单个设备，网关句柄控制网关下所有设备
		@r: true表示红灯开，false表示电源关
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
	说明:
	

*/
CLIB_API RS cl_rfdev_rgb(cl_handle_t handle, bool r, bool g, bool b);

/*
	功能:
		批量控制设备电源
	输入参数:
		@gw_handle: 网关句柄
		@dev_handle: 设备句柄数组
		@cnt: 设备句柄数量，最多32个
		@rgb: 三色灯控制，参考RGB_WORK_R
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
	说明:	

*/
CLIB_API RS cl_rfdev_rgb_batch(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt, u_int8_t rgb);

/*
	功能:
		批量查询设备工作状态
	输入参数:
		@gw_handle: 网关句柄
		@dev_handle: 设备句柄数组
		@cnt: 设备句柄数量，最多32个
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
	说明:	

*/
CLIB_API RS cl_rfdev_work_query(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt);

/*
	功能:
		查询分组
	输入参数:
		@gw_handle: 网关句柄
		@group_id: 分组id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_dev_group_query(cl_handle_t gw_handle, u_int8_t group_id);

/*
	功能:
		创建或更新分组
	输入参数:
		@gw_handle: 网关句柄
		@group_id: 分组id
		@group_name: 分组名称
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_dev_group_set(cl_handle_t gw_handle, cl_dev_group_t *group);

/*
	功能:
		删除分组
	输入参数:
		@gw_handle: 网关句柄
		@group_id: 分组id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_dev_group_delete(cl_handle_t gw_handle, u_int8_t group_id);


/*
 功能:
    分组控制指令
 输入参数:
    @gw_handle: 网关句柄
    @group_id: 分组id
    @flag:默认填GDT_NONE_SPCAIL，如果是灯组的颜色那些控制，填GDP_LAMP_COLOR_CTRL（因为要转换数据）
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rf_dev_group_ctrl(cl_handle_t gw_handle, u_int8_t group_id,u_int32_t flag,u_int8_t* data,int len);
    
/*
	功能:
		修改从设备名称
	输入参数:
		@dev_handle: 从设备句柄
		@name: 从设备名称，最长15字节
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_dev_name_set(cl_handle_t dev_handle, u_int8_t *name);

/*
	功能:
		修改从设备名称
	输入参数:
		@dev_handle: 网关句柄
		@upgrade_type: 从设备升级类型
		@filepath:升级文件路径
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rfdev_upgrade(cl_handle_t gw_handle, u_int32_t upgrade_type, u_int8_t *filepath);

/*
	功能:
		兼容模式设置
	输入参数:
		@gw_handle: 网关句柄
		@commpat: 网关兼容模式0,1,2
		@channel:0默认信道；1~255为信道号，2400+信道号为网关使用频率。
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_dev_commpat(cl_handle_t gw_handle, u_int8_t commpat, u_int8_t channel);

/*
	功能:
		升级状态查询
	输入参数:
		@gw_handle: 网关句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_dev_up_query(cl_handle_t gw_handle);


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
CLIB_API RS cl_rfdef_comm_timer_modify_add(cl_handle_t dev_handle, cl_comm_timer_t *ptimer);

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
CLIB_API RS cl_rfdev_comm_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
功能: 
    通用定时器查询，如登陆时查询，切换进定时器页面时查询
    @id: 定时器id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rfdev_comm_timer_query(cl_handle_t dev_handle);


/*
功能: 
    通用查询从设备历史记录
    @index: 历史记录索引
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rfdef_comm_history_query(cl_handle_t dev_handle, u_int32_t index);

/*
功能: 
    查询rf从设备版本号，主要给pc软件测试用，app不用管
    @id: 定时器id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rfdev_debug_info_query(cl_handle_t dev_handle, cl_rf_dev_debug_info_t *ver);

/*
 功能:
    rf从设备快捷开关机
 输入参数:
    @dev_handle 设备句柄
    @enable: 是否使能
    @onoff: 0: 关机 1开机
    @time: 剩余时间，秒数单位
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rfdev_public_set_shortcuts_onoff(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int32_t time);

/*
 功能:
    rf从设备快捷开关机查询，估计需要定时查询
 输入参数:
    @dev_handle 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rfdev_public_query_shortcuts_onoff(cl_handle_t dev_handle);

/**************************************网关透传外设控制接口**********************************************************************************/
//网关透传外设控制接口
/*
 功能:
    夜狼警报时间设置
 输入参数:
    @gw_handle:网关句柄
    @time:报警时间:0-255
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rfdev_set_yl_time(cl_handle_t gw_handle, u_int8_t time);

/*
 功能:
    夜狼喇叭声音大小设置
 输入参数:
    @gw_handle:网关句柄
    @type:1-表示喇叭作为门铃声音参数，2-表示喇叭作为警报的声音参数
    @voice:0-100
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rfdev_set_yl_voice(cl_handle_t gw_handle, u_int8_t type, u_int8_t voice);

/*
 功能:
    夜狼警笛关闭
 输入参数:
    @gw_handle:网关句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rfdev_set_yl_siren_off(cl_handle_t gw_handle);

/*
 功能:
    夜狼警笛开关设置
 输入参数:
    @gw_handle:网关句柄
    @onoff:1,开启，0关闭
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rfdev_set_yl_siren_onoff(cl_handle_t gw_handle, u_int8_t onoff);

/*
*指定网关sn查询是否从设备允许升级
*/
CLIB_API RS cl_rfdev_up_check(cl_handle_t gw_handle);

/*
功能: 
	rf设备缓存镜像查询
	@gw_handle
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rfdev_img_cache_query(cl_handle_t gw_handle);

/*
功能: 
	rf设备缓存镜像删除
	@gw_handle
	@pindex, 删除开始的缓存下标数组
	@num, 删除个数
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rfdev_img_cache_del(cl_handle_t gw_handle, u_int8_t num, u_int8_t *pindex);


/*
功能: 
	花瓶网关配置是否推送断网断电信息
	@gw_handle
	@onoff: 开关
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hpgw_config_appinfo_onoff(cl_handle_t gw_handle, u_int8_t onoff);

/*
功能: 
	花瓶网关配置短信
	@gw_handle
	@sms_onoff: 是否开启短信
	@lang: 短信语言
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hpgw_config_sms(cl_handle_t gw_handle, u_int8_t sms_onoff, u_int8_t lang);

/*
功能: 
	花瓶网关配置短信用户，注意，如果网关已经有这个手机号，那就是修改名字
	@gw_handle
	@name: 用户自定义名字
	@phone_number: 手机号
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hpgw_config_phone_user(cl_handle_t gw_handle, u_int8_t *name, u_int64_t phone_number);

/*
功能: 
	花瓶网关删除一个手机用户
	@gw_handle
	@sms_onoff: 是否开启短信
	@phone_number: 手机号
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hpgw_del_phone_user(cl_handle_t gw_handle, u_int64_t phone_number);

/*
功能: 
	花瓶网关灯控制
	@gw_handle
	@stat: 控制参数
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_hpgw_lamp_ctrl(cl_handle_t gw_handle, cl_rf_lamp_stat_t *stat);

#ifdef __cplusplus
}
#endif 


#endif

