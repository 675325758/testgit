/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: rfgw_scm_ctrl.h
**  File:    rfgw_sctm_ctrl.h
**  Author:  liubenlong
**  Date:    02/27/2016
**
**  Purpose:
**    Rf网关透传头文件.
**************************************************************************/


#ifndef RFGW_SCTM_CTRL_H
#define RFGW_SCTM_CTRL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

/* Macro constant definitions. */
//夜狼宏定义
#define YL_BOOT_CODE		(0x55)
#define YL_FUNC_CODE_GET	(0x2)
#define YL_FUNC_CODE_SET	(0x3)

#define YL_ACTION_SET_ALARM_TIME	(0x1)
#define YL_ACTION_SET_SOUND			(0x2)
#define YL_ACTION_SET_HORN			(0x3)
#define YL_ACTION_SET_DEFENSE		(0x2)



/* Type definitions. */

#pragma pack(push,1)
//夜狼网络数据结构
typedef struct {
    u_int8_t boot_code;
	u_int8_t func_code;
	u_int8_t len;
	u_int8_t check_sum;
}ucp_rfgw_scm__head_t;

typedef struct {
	//ucp_rfgw_scm__head_t head;
	u_int8_t battery;//电池电量（bit0-6）(0 —100),kBit7：电源选项(0电池，1外接电源)
	u_int8_t sos_alarm;//sos报警指令(0，无；1，SOS报警。)
	u_int8_t key_info;//按键信息(0 — 255，按键逻辑反馈。)
	u_int8_t door_voice;//门铃声音大小(0 — 100)
	u_int8_t alarm_voice;//警报声音大小(0 — 100)
	u_int8_t alarm_time;//警报时间(0 - 255秒)
	u_int8_t is_guard;
	u_int8_t pad[5];
}ucp_rfwg_scm_allstate_t;

//报警时间设置
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t time;
}ucp_rfgw_scm_alarm_time_t;

//喇叭声音大小设置
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t func;//为0x1则设置的是喇叭作为门铃的声音参数。为0x2则设置的是喇叭作为警报的声音参数。
	u_int8_t voice;//声音大小
}ucp_rfgw_scm_horn_voice_t;

//喇叭动作控制报文
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t func;//为0x1选择门铃，0x2选择警笛。
	u_int8_t ctrl;//0 关闭， 1开启。
}ucp_rfgw_scm_horn_action_t;

//一键部防撤防
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t is_defense;
	u_int8_t pad[6];
}ucp_rfgw_scm_defense_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */



extern bool rfgw_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool rfgw_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern void rfgw_do_query_hook(smart_air_ctrl_t* air_ctrl);
extern int rfgw_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);
extern void rf_scm_dev_set_defense_batch(user_t *puser, u_int8_t is_defence);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* RFGW_SCTM_CTRL_H */

