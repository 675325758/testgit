#ifndef	__COMMON_UDP_DEVICE__
#define	__COMMON_UDP_DEVICE__
#include "cl_smart_appliance.h"
#include "cl_lanusers.h"

enum {
    COMMON_UE_BEGIN = 2000,
    //设备异常信息push 
    COMMON_UE_DEV_ERR_PUSH_MSG = COMMON_UE_BEGIN + 1,
    COMMON_UE_DEV_COMM_TIMER_ADD_FAILED = COMMON_UE_BEGIN + 2,
    COMMON_UE_DEV_COMM_TIMER_DEL_FAILED = COMMON_UE_BEGIN + 3,
    //定时器时间冲突事件
    COMMON_UE_DEV_COMM_TIMER_TIME_CONFLICT = COMMON_UE_BEGIN + 4,

    COMMON_UE_END = COMMON_UE_BEGIN + 99
};

typedef struct{
	u_int32_t err_id;
	u_int32_t err_time; 
	u_int16_t err_obj_type;
	u_int16_t err_type;
	u_int32_t err_data;
}cl_dev_err_item_t;

typedef struct{
	u_int8_t err_count;
	u_int8_t pushed_err_count;
	u_int8_t pad[2];
	cl_dev_err_item_t * err_items;
	cl_dev_err_item_t* pushed_err;
}cl_dev_err_info_t;


// 通用的WIFI设备历史记录
typedef struct {
	u_int32_t id;
	u_int32_t time;
	u_int8_t type;
	u_int8_t condition;
	u_int8_t action;
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t temp;
	u_int8_t wind;
	u_int8_t winddir;
	u_int8_t key;
	u_int8_t valid;
} cl_wukong_history_item_t;

typedef struct {
	u_int32_t index;
	u_int32_t num;
} cl_dev_history_item_hd_t;

typedef struct {
	union {
		cl_wukong_history_item_t wukong;
	} u;
} cl_dev_history_item_t;

typedef struct {
	
	// 下面2个数据在收到SAE_DEV_COMM_HISTORY_SUMMARY = 1241 后读取
	u_int32_t index_current;
	u_int32_t max_count;

	// 下面2个数据在收到SAE_DEV_COMM_HISTORY_ITEM = 1242后读取
	u_int32_t n;
	u_int32_t index;	// 下面item 的第一个index，提出来是方便IOS同学使用
	cl_dev_history_item_t *item;
} cl_dev_history_t;

typedef struct cl_com_udp_device_data_s{
	/*备份一次类型，用于App 层确认数据正确性*/
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t is_lan_connect; //是否局域网连接0: 未知, 1 局网连接，2 公网连接
	u_int8_t is_all_data_update; //是否数据更新完毕, 0, 更新中, 1 更新完成

	u_int8_t is_system_info_valid; //系统版本状态是否获取回来了
	u_int8_t is_stat_info_valid;// 状态是否获取完成
	u_int8_t is_support_period_timer; //是否支持周期性定时器
	u_int8_t is_suppport_elec_stat; // 是否支持电量统计
    u_int8_t is_support_ext_period_timer; //是否支持周期性扩展定时器 
	
	u_int8_t is_support_dev_err_info; //是否支持设备异常信息
	u_int8_t is_support_stm_upgrade; //支持单片机升级功能
	u_int8_t is_support_dev_restory_factory; //是否支持恢复出厂功能
	u_int8_t is_support_dev_set_wifi_param; //支持设置wifi参数功能

	u_int8_t is_support_disk;//是否支持硬盘信息获取
	u_int8_t is_support_eth;//是否支持网口信息获取
    
    u_int8_t is_support_room_temp_ajust;//室温校正
    u_int8_t is_support_elec_ajust;//电量校正
    u_int8_t is_support_rc; //支持电视、机顶盒控制
    u_int8_t pad;
    
    int16_t env_room_temp_low; //最高校正室温
    int16_t env_room_temp_high; //最低校正室温
    int16_t  env_temp_ajust_value; //环境温度校正值 放大10倍，例：界面显示 -5-5度，传值 -50-50
    int16_t  elec_ajust_value; //电量校正系数 放大100倍 例:界面显示0.5-1.5，传值为 50-150

	u_int8_t is_support_dev_wifi_state;	// 支持设备端显示WIFI连接状态
	/*
	      0: WIFI未配置
	      1：连接成功   //表示设备成功连接到wifi 且获取到ip地址，切上行链路通
	      2：连接中。。。
	      3：连接成功，正在获取ip地址
	      4：上行链路不通
	      5：连接ap失败
	*/
	u_int8_t dev_wifi_state;		// 设备端WIFI状态
	//timezone
	u_int8_t timezone_valid;
	u_int32_t timezone;

	u_int8_t is_support_wan_config;	// 是否支持外网口配置
	cl_wan_config_t wan_config;		// 外网口配置

	u_int8_t is_support_dhcp_server;	// 支持DHCP服务器配置
	cl_dhcp_server_config_t dhcp_config;	// dhcp服务器配置

	u_int8_t is_support_ap_config;	// 是否支持AP配置
	cl_ap_config_t ap_config;		// AP配置

	u_int8_t is_support_repeat;		// 是否支持中继器
	u_int8_t repeat_onoff;			// 中继器开关 0 关 1 开
	///////////////////////////////////////////////////////////////////////
	//  通用定时器信息
	cl_air_timer_info_t timer_info; // 定时器信息
	cl_dev_timer_summary_t timer_summary;//通用定时器的摘要信息
	cl_comm_timer_head_t comm_timer_head;//通用定时器，针对所以设备
	//////////////////////////////////////////////////////////////////////////
	//通用电量统计
	u_int32_t current_power; // 当前功率
	u_int32_t cur_milli_power; //当前功率，毫瓦
	cl_elec_stat_info_t elec_stat_info; //电量统计信息
	cl_air_elec_item_info total_elec; //总电量
	cl_air_elec_item_info period_elec; //阶段电量
	cl_air_elec_item_info last_on_elec; //上次开机电量
	cl_elec_days_stat_info elec_days_info; //电量统计之365天统计数据
	/////////////////////////////////////////////////////////////////////////////////////////////
	//设备异常信息
	cl_dev_err_info_t dev_err_info;

	// 设备历史记录
	u_int8_t is_support_dev_history;
	cl_dev_history_t dev_history;

	// 通用的开机温度
	u_int8_t is_support_boot_temp;
	u_int8_t boot_temp_enable;	// 是否开启自动开机
	u_int8_t boot_temp;			// 自动开机温度

	// 用户上下线信息
	bool support_lanusers_manage;
	cl_lanusers_manage_info_t lum_info;
	
	// //////////////////////////////////////////////////////////////////////////////////////////
	// 通用其他信息
	u_int8_t is_support_utc_temp_curve;		// 支持通用命令下的温度曲线
	u_int8_t has_utc_temp_curve_data;		// 有温度曲线数据
	int temp_curve_len;	//通用温度曲线数据长度，因为可以支持多个曲线，目前估计只用一个,len配合count可以解析出多个曲线
	cl_temp_curve_t *temp_curve;//通用温度曲线数据

    cl_24hour_line room_temp_line; // 24小时温度曲线
    cl_24hour_line humi_line; //24小时湿度曲线
	u_int8_t room_temp;/*室温*/
	u_int8_t temp_humidity; //湿度，增强型悟空才有效
	
	u_int8_t is_support_public_utc_temp_ac_ctrl;	// 支持通用命令下的温度控制，结构在air_ctrl->air_info.ta
	u_int8_t has_utc_temp_ctrl_data;	// 用温度控制数据
	cl_temp_ac_ctrl_t tac; //根据温度控制空调
	
	u_int8_t is_support_public_smart_on;			// 支持通用的智能开机配置
	u_int8_t smart_on;								// 通用的智能开关机
	
	u_int8_t is_support_public_child_lock;			// 支持通用命令下的童锁
	u_int8_t child_lock_value;						// 通用的童锁

	u_int8_t is_support_public_temp_alarm;			// 支持通用命令下的温度阀值报警
	u_int8_t temp_alarm_onoff;						// 通用温度阀值报警开关
	u_int8_t temp_alarm_min;						// 通用温度阀值报警最小温度	
	u_int8_t temp_alarm_max;						// 通用温度阀值报警最大温度
	
	u_int8_t is_support_public_shortcuts_onoff;		// 支持通用的快捷开关机
	cl_shortcuts_onoff_t shortcuts_onoff;			// 通用的快捷开关机 

	bool has_recv_flag_pkt;//是否标志有效
	u_int8_t is_support_la;//是否支持联动控制

	u_int8_t flag_stat_update_ok;	// 支持标志已经刷新过

	u_int8_t hardware_led_ver;	// 节能宝LED硬件版本

	u_int8_t is_support_spe_up;//是否支持指导升级
	//////////////////////////////////////////////////////////////////////////////////////////
	//  设备私有信息
	void * device_info; //私有设备信息，各类设备不同
}cl_com_udp_device_data;

#define UTC_TIME_TO_LOCAL(time) (time+cl_priv->zone*3600)

#ifdef __cplusplus
extern "C" {
#endif 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      通用定时器处理
/*
	功能:
		删除定时开关规则
	输入参数:
		@dev_handle: 设备的句柄
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_com_udp_timer_set(cl_handle_t dev_handle, cl_air_timer_t *timer);

/*
	功能:
		删除定时开关规则
	输入参数:
		@dev_handle: 设备的句柄
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_com_udp_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	功能:
		添加或修改定时开关规则
	输入参数:
		@dev_handle: 设备句柄
		@timer: 定时器条目的参数。id为0表示添加，其他的为修改
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_com_udp_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer);

    
/*
 功能:
    添加或修改定时开关规则
 输入参数:
    @dev_handle: 设备句柄
    @timer: 定时器条目的参数。id为0表示添加，其他的为修改
    @other_param,扩展参数，用于某设备需要的特殊参数，格式由APP和SDK协商
    @param_len，扩展参数长度
 /////////////////
    千帕茶盘 other_param == cl_qpcp_scp_t; param_len == sizeof(cl_qpcp_scp_t)
    悟空  other_param == cl_808_timer_ext_t,param_len = sizeof(cl_808_timer_ext_t)
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_ext_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer,void* other_param,u_int16_t param_len);
/*
	功能:
		删除定时开关规则
	输入参数:
		@dev_handle: 设备的句柄
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_com_udp_period_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	功能:
		刷新定时器
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API void cl_com_udp_refresh_timer(int dev_handle);

/*
	功能:
		刷新设备所有数据
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API void cl_com_udp_refresh_dev_all_info(int dev_handle);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//     通用电量统计处理

/*
 功能:
    设置峰电计量时间
 输入参数:
    @begin_time: 开始时间 单位：分钟  0： 0点0分  1439：23时59分
    @last_time:  持续时间 单位：分钟  max： 1440
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_set_peak_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
/*
 功能:
    设置谷电计量时间
 输入参数:
    @begin_time: 开始时间 单位：分钟  0： 0点0分  1439：23时59分
    @last_time:  持续时间 单位：分钟  max： 1440
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_set_valley_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute);
    
    
/*
 功能:
    设置峰电价格
 输入参数:
    @price: 单位：分
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_set_peak_price(cl_handle_t dev_handle, u_int32_t price);

/*
 功能:
    设置谷电价格
 输入参数:
    @price: 单位：分
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_set_valley_price(cl_handle_t dev_handle, u_int32_t price);

/*
 功能:
    设置平电价格
 输入参数:
    @price: 单位：分
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_set_flat_price(cl_handle_t dev_handle, u_int32_t price);

/*
 功能:
    刷新电量统计数据
 输入参数:
    @dev_handle: 设备句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_refresh_elec_info(cl_handle_t dev_handle);

/*
 功能:
    清空周期电量统计信息
 输入参数:
    @dev_handle 设备句柄
    @type: 0x0: 清空周期统计信息
               0x1: 清空最近开机的电量统计信息；
               0x2: 清空累计统计信息
               0x3: 清空所有电量信息
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_clear_elec_stat_info(cl_handle_t dev_handle,int type);
    
/*
 功能:
    设置电量校正系数
 输入参数:
    @dev_handle 设备句柄
    @value: 50-150
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_set_elec_ajust_value(cl_handle_t dev_handle,int16_t value);

/*
 功能:
    清空设备异常信息
 输入参数:
    @dev_handle 设备句柄
    @err_id ,异常ID 号，0:表示清空
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_clear_dev_err_info(cl_handle_t dev_handle , u_int32_t err_id);

/*
 功能:
    刷新设备异常信息
 输入参数:
    @dev_handle 设备句柄
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_refresh_dev_err_info(cl_handle_t dev_handle);

/*
 功能:
    设置允许升级单片机
 输入参数:
    @dev_handle 设备句柄
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_set_permit_stm_upgrade(cl_handle_t dev_handle);

/*
 功能: //谨慎使用
    让设备恢复出厂
 输入参数:
    @dev_handle 设备句柄
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_set_dev_restore_factory(cl_handle_t dev_handle);


/*
 功能: 
    重设设备的ssid和密码
 输入参数:
    @dev_handle 设备句柄
 @as
    输出参数:
 无
    返回:
 RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_reset_dev_ssid_and_passwd(cl_handle_t dev_handle,const char* ssid,const char* password);
    
/*
 功能:
    设置室温校正的参数
 输入参数:
    @dev_handle 设备句柄
    @value -50 -- 150
    @value
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_com_udp_set_env_temp_ajust_value(cl_handle_t dev_handle,int16_t value);
    
/*
 功能:
    刷新温湿度曲线
 输入参数:
    @type 0x0 温度
          0x1 湿度
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
    
CLIB_API RS cl_com_udp_refresh_24hour_line(cl_handle_t dev_handle,u_int8_t type);
    
/*
 功能:
 	从设备端获取一个分享码，必须具备超级权限的客户端才可以获取
 输入参数:
 输出参数:
 	无
 返回:
 	RS_OK: 成功
 	其他: 失败
 */
CLIB_API RS cl_com_udp_request_share_code(cl_handle_t dev_handle);
    
/*
 功能:
 	删除一个已分享的手机
 输入参数:
 	@share_index 分享index
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_del_shared_phone(cl_handle_t dev_handle,u_int32_t share_index);

/*
 功能:
 	刷新已分享手机列表
 输入参数:
 输出参数:
 	无
 返回:
 	RS_OK: 成功
 	其他: 失败
 */
CLIB_API RS cl_com_udp_refresh_shard_list(cl_handle_t dev_handle);

/*
 功能:
 	修改一个手机的描述
 输入参数:
 	@share_index 分享index
 	@desc 手机描述字符串
 输出参数:
 无
 返回:
 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_com_udp_modify_shared_phone(cl_handle_t dev_handle,u_int32_t share_index, char *desc);

#ifdef __cplusplus
}
#endif 


#endif

