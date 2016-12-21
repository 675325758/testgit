#ifndef	__CL_JS_WAVE_H__
#define	__CL_JS_WAVE_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

// 工作模式
enum{
    JS_WM_NONE = 0,
    JS_WM_WAVE , //微波模式
    JS_WM_UNFREE, //解冻模式
    JS_WM_WAVE_BARBECUE, //微波+烧烤
    JS_WM_AUTO_MENU //自动菜单模式
};

// 工作子模式

//烧烤模式的子模式
enum{
    JS_SWM_B_TOP_BARBECUE = 0x1, // 顶部烧烤工作
    JS_SWM_B_BOTTOM_BARBECUE, // 底部烧烤工作
    JS_SWM_B_TB_BARBECUE // 底部和顶部都工作
};

//组合模式的子模式
enum{
    JS_SWM_WB_TOP_BARBECUE = 0x1, // 微波+顶部烧烤工作
    JS_SWM_WB_BOTTOM_BARBECUE, // 微波+底部烧烤工作
    JS_SWM_WB_TB_BARBECUE, // 微波+底部和顶部都工作
    JS_SWM_WB_HF_ALL_BARBECUE, // 微波+热风+底部和顶部都工作
    JS_SWM_WB_HF // 微波+热风
};

//自动菜单的子模式
enum{
    JS_SWM_AUTO_RICE = 0x1, //米饭
    JS_SWM_AUTO_VEGETABLES, //蔬菜
    JS_SWM_AUTO_MEAT,// 肉类
    JS_SWM_AUTO_NOODLE,//面类
    JS_SWM_AUTO_FISH, // 鱼类
    JS_SWM_AUTO_TIPPING, // 自动翻热
    JS_SWM_AUTO_POTATO, // 土豆
    JS_SWM_AUTO_PIZZA, // 披萨饼
    JS_SWM_AUTO_CHICKEN, // 鸡肉
    JS_SWM_AUTO_DEHYDRATION, // 脱水功能
    JS_SWM_AUTO_POPCORN, // 爆米花功能
    JS_SWM_AUTO_CARROT_MUD, // 胡萝卜泥
    JS_SWM_AUTO_APPLE_MUD, //苹果署泥
    JS_SWM_AUTO_MILK_COFFEE, // 牛奶咖啡
    JS_SWM_AUTO_INSTANT_NOODLE, //方便面
    JS_SWM_AUTO_BREAD, //面包
    JS_SWM_AUTO_APPLEPAI,//苹果派
    JS_SWM_AUTO_WIFEPIE,//老婆饼
    JS_SWM_AUTO_BIGBEEF,//大牛油可酥
    JS_SWM_AUTO_GOUGHNUT, //甜甜圈
    JS_SWM_AUTO_FRENCHBIG,//法式大魔杖
};
    
enum{
    JS_UACT_START = 0x1, //启动
    JS_UACT_CANCEL, //取消
    JS_UACT_PAUSE // 暂停
};
    
typedef struct{
    u_int8_t is_data_valid; // 数据是否有效
    u_int8_t is_waiting; //是否待机
    u_int8_t is_working; //是否正在工作
    u_int8_t is_pausing; //是否正处于暂停工作状态
    u_int8_t child_lock_onoff; //童锁状态
    u_int8_t is_door_open; //设备门是否打开
    u_int8_t is_fault_stat; //设备是否处于故障状态
    u_int8_t is_chain_stat; //连锁状态
    u_int8_t work_mode; //工作模式,见 JS_WM_XX()
    u_int8_t work_sub_mode; //工作子模式
    u_int8_t setting_min; // 时间：分钟
    u_int8_t setting_sec; // 时间：秒数
    u_int8_t wave_fire; //微波火力
    u_int8_t barbecue_fire; // 烧烤火力
    u_int8_t hot_fan_temp; // 热风温度
    u_int16_t food_weight; // 食物重量
    u_int8_t cur_temp; // 当前温度
    u_int8_t remain_min; //当前时间
    u_int8_t remain_sec; // 当前秒数
    u_int32_t local_refresh_time; //刷新状态的手机utc时间，可以根据该时间，倒推剩余的分钟和秒数
}cl_js_wave_stat_t;
    
typedef struct {
    u_int8_t work_mode; //工作模式
    u_int8_t work_min; //工作分钟数 1-60分钟
    u_int8_t work_sec; //工作秒数 0-60 秒，当分钟是0时，秒数不能设置0
    u_int8_t wave_fire; //微波火力大小
    u_int8_t barbecue_fire; // 烧烤火力
    u_int8_t hot_fan_temp; // 热风温度
    u_int16_t food_weight; // 食物重量，？？是否需要呢
    u_int8_t work_sub_mode; //工作子模式
    u_int8_t action; // 动作： 启动、取消、暂停
}cl_js_wave_work_setting_t;
    
typedef struct{
    cl_js_wave_stat_t stat;
}cl_js_wave_info_t;

/*
 功能：
 	控制自定义模式
 输入参数:
 	@handle: 设备句柄
 输出参数:
 	无
 返回：
 	RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_js_wave_ctrl(cl_handle_t dev_handle,cl_js_wave_work_setting_t* setting);


/*
 功能：
	 控制快速启动、停止、暂停
 输入参数:
	 @handle: 设备句柄
 输出参数:
	 无
 返回：
	 RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_js_wave_fast_ctrl(cl_handle_t dev_handle,u_int8_t action);
    
/*
 功能：
 	控制童锁
 输入参数:
 	@handle: 设备句柄
 输出参数:
 	无
 返回：
 	RS_OK:
 	其他: 失败
 	事件通知:
 
 */
CLIB_API RS cl_js_wave_ctrl_child_lock(cl_handle_t dev_handle,u_int8_t on_off);

#ifdef __cplusplus
}
#endif 

#endif

