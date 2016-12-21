#ifndef	__SBT_THER_SCM_CTRL_H__
#define	__SBT_THER_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"
#include "cl_yl_thermostat.h"

#define SBT_START_CODE1 0xFF
#define SBT_START_CODE2 0xAA

enum{
    SBT_SCM_CMD_STAT = 0x1,
    SBT_SCM_CMD_FUNC_PARAM ,
    SBT_SCM_CMD_TIME_ADJUST = 0x4,
    SBT_SCM_CMD_AUTO_PARAM
};

#pragma pack(push,1)

typedef struct {
    u_int8_t start_code1;
    u_int8_t start_code2;
    u_int8_t command;
    u_int8_t device_type;
    u_int8_t data_len;
    u_int8_t pad[3];
}ucp_sbt_scm_head_t;

typedef struct {
    u_int8_t temp; //温度 10-32
    u_int8_t fan_speed;
    u_int8_t mode; //工作模式 00制冷模式,01制热模式,2通风模式,3 节能模式
    u_int8_t onoff; //开关机
    //----------------------
    u_int8_t auto_mode; //自动模式状态 0关闭，1执行自动模式
    int8_t temp_adjust; //温度补偿 -5至5
    u_int8_t low_temp; //低温保护温度 3-9
    u_int8_t valve_mode; //(风机开关)阀门模式控制 1、阀关风机不停0、阀关风机停
    u_int8_t return_temp; //回差温度 1~31
    u_int8_t is_low_temp_guard; //低温保护使能
    u_int8_t max_temp; //限制最高温度 10-32
    u_int8_t min_temp; //限制最低温度 10-32
    //以下这段数据APP可以暂时不关心,jni可以不上传
    u_int8_t lock_screen; //是否锁屏，
    u_int8_t sesor_type; //温度传感器类型
    u_int8_t broad_cast_type; //广播类型
    u_int8_t no_paid_mode; //欠费模式
    u_int8_t max_sesor_temp; //外部传感器限制温度
    u_int8_t pad;
    //不关心部分完//
    u_int8_t room_temp_integer; //室温整数部分
    u_int8_t room_temp_decimal; //室温小数部分
    
    u_int8_t scm_hour; //单片机时间,时
    u_int8_t scm_min; //单片机时间,分
    u_int8_t scm_sec; //单片机时间,秒
    u_int8_t scm_weekday; //单片机时间,星期
    cl_smart_smart_ctrl_t smart_info; //智能模式当前信息
}ucp_sbt_scm_info_t;

typedef struct {
    u_int8_t temp; //温度 10-32
    u_int8_t fan_speed; //
    u_int8_t mode; //工作模式 00制冷模式，01制热模式，2通风模式， 3 节能模式
    u_int8_t onoff; //开关机
}ucp_sbt_scm_stat_t;

typedef struct {
    u_int8_t auto_mode; //自动模式状态 0关闭，1执行自动模式
    int8_t temp_adjust; //温度补偿 -5至5
    u_int8_t low_temp; //低温保护温度 3-9
    u_int8_t valve_mode; //(风机开关)阀门模式控制 1、阀关风机不停0、阀关风机停
    u_int8_t return_temp; //回差温度 1~31
    u_int8_t is_low_temp_guard; //低温保护使能
    u_int8_t max_temp; //限制最高温度 10-32
    u_int8_t min_temp; //限制最低温度 10-32
}ucp_sbt_scm_func_t;

typedef struct {
    u_int8_t scm_hour; //单片机时间,时
    u_int8_t scm_min; //单片机时间,分
    u_int8_t scm_sec; //单片机时间,秒
    u_int8_t scm_weekday; //单片机时间,星期
}ucp_sbt_scm_time_adjust_t;

#pragma pack(pop)

extern bool sbt_ther_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool sbt_ther_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

extern int sbt_ther_scm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);

#endif

