#ifndef	__CL_BIMAR_DEV_H__
#define	__CL_BIMAR_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif 
  
//////////////////////////////////
// BIMAR C2000
#define BIM_MAX_C_TEMP 37
#define BIM_MIN_C_TEMP 5
    
#define BIM_MAX_F_TEMP 99
#define BIM_MIN_F_TEMP 41
    
#define BIM_PREVENTFREEZEMODE_C_TEMP 7
#define BIM_PREVENTFREEZEMODE_F_TEMP 45
    
#define BIM_DEV_HC2000L 0x0
#define BIM_DEV_0505L 0x1
#define BIM_DEV_5162L 0x2

#define BIM_MAX_SET_TIME_HOUR 15
////////////////////////////////////////
// 澳柯玛 0505L
#define AKM_0505L_MAX_C_TEMP 49
#define AKM_0505L_MIN_C_TEMP 10
#define AUCMA_MAX_SET_TIME_HOUR 8
    
#define AKM_5162L_MAX_C_TEMP 49
#define AKM_5162L_MIN_C_TEMP 10
    
    
enum{
    AUCMA_GEAR_WIND = 0x1, //开启风扇吹风
    AUCMA_GEAR_LOW, //一档
    AUCMA_GEAR_HIGH //二档
};
    
enum{
    BIM_FAN_SPEED_UNKNOWN = 0x0,
    BIM_FAN_SPEED_LOW, //低风
    BIM_FAN_SPEED_MID, //中风
    BIM_FAN_SPEED_HIGH //高风
};
    
enum{
    BIM_TEMP_UNIT_UNKNOWN = 0x0,
    BIM_TEMP_UNIT_FAHR,  //华氏度
    BIM_TEMP_UNIT_CELS //摄氏度
};
    
enum{
    BIM_GEAR_UNKNOWN = 0x0,
    BIM_GEAR_LOW, //低
    BIM_GEAR_MID, //中
    BIM_GEAR_HIGH //高
};
    
enum{
    BIM_GEAR_800W = 0x1, //
    BIM_GEAR_1200W, //
    BIM_GEAR_2000W, //
    BIM_GEAR_800W_EXT //800W加防霜冻
};
    
typedef enum{
    BIM_CTRL_TYPE_UNKNOWN,
    BIM_CTRL_TYPE_ONOFF,
    BIM_CTRL_TYPE_ANION,
    BIM_CTRL_TYPE_SHAKE,
    BIM_CTRL_TYPE_TEMP_UNIT,
    BIM_CTRL_TYPE_GEAR,
    BIM_CTRL_TYPE_CELS_TEMP,
    BIM_CTRL_TYPE_FAHR_TEMP,
    BIM_CTRL_TYPE_TIMER
}BIM_CTRL_ACTION_E;

typedef struct{
    u_int8_t is_data_valid; //数据是否有效，即SDK是否获取到设备的状态了
    u_int8_t is_on; //开关状态
    u_int8_t is_shake; //摇头状态
    u_int8_t is_anion_enable; //负离子状态
    u_int8_t fan_speed; //风速
    u_int8_t temp_unit; // 温度单位
    u_int8_t power_gear; //功率档位
    u_int8_t cur_set_cels_temp; //当前设置温度 摄氏度 5-37
    u_int8_t cur_set_fahr_temp; //当前设置温度 华氏度 41-99
    u_int8_t cur_set_timer; //设置的定时时间,小时
    u_int8_t cur_remain_min;//和cur_set_timer配合使用，表示剩余多少分钟
    int8_t cels_room_temp; //摄氏度室温
    int8_t fahr_room_temp; //华氏度室温
    u_int8_t machine_type; // APP 不用此参数
    u_int32_t loc_update_gnu_time; //本地更新时间，可以利用这个时间推算出剩余多少分钟
   	u_int8_t scm_run_mode;//机器运行哪张策略 AUCMA_RUN_MODE_NONE  AUCMA_RUN_MODE_TEMP  AUCMA_RUN_MODE_GEAR
    //SDK使用，上层无须关心
    u_int8_t old_set_cels_temp;
	u_int8_t old_gear;
	u_int8_t pad;
}cl_bimar_heater_info_t;
    

/*
 功能:
    设置各种参数
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
    
// 注意，当数据无效（is_data_valid ＝＝ false）时,调用此函数会返回失败
CLIB_API RS cl_bimar_ctrl(cl_handle_t dev_handle,BIM_CTRL_ACTION_E type,u_int8_t value);
    
#ifdef __cplusplus
}
#endif 

#endif

