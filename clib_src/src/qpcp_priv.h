/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: qpcp_priv.h
**  File:    qpcp_priv.h
**  Author:  liubenlong
**  Date:    05/13/2015
**
**  Purpose:
**    千帕茶盘.
**************************************************************************/


#ifndef QPCP_PRIV_H
#define QPCP_PRIV_H

#include "cl_hxpbj.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */

/* Macro constant definitions. */
#define BOIL_DESC		"蒸煮"
#define DISINFECT_DESC	"烧水"

//茶盘个数限制
#define TEA_TRAY_QP_PLAN_MAX	50
/* Type definitions. */
typedef struct {
    //千帕茶盘用
	cl_qpcp_info_t cl_info;
    //千帕锅用
    cl_qp_pot_info_t qp_pot_info;
    //千帕破壁机用
    cl_qp_pbj_info_t qp_pbj_info;
    //海迅养生壶
    cl_hx_ysh_info_t hx_ysh_info;
	//情景链表
	struct stlc_list_head scene_list;
}cl_qpcp_priv_t;

#pragma pack(push, 1)
typedef struct {
	u_int8_t work;
	u_int8_t pad[3];
}cl_qpcp_pri_work_t;

typedef struct {
	u_int8_t action;
	u_int8_t water_time;
	u_int8_t pad[2];
}cl_qpcp_pri_water_t;

typedef struct {
	u_int16_t id;
	u_int16_t pad;
}cl_qpcp_pri_sdel_t;

typedef struct {
	u_int16_t id;
	u_int16_t pad;
	cl_qpcp_scp_t scp;
}cl_qpcp_pri_exe_t;
    
typedef struct {
    u_int8_t is_rest;
    u_int8_t pad[3];
}usp_qpcp_reset_pkt_t;

typedef struct {
	u_int8_t work;
	u_int8_t mode;
	u_int8_t disinfect_temp;
	u_int8_t disinfect_power;
	u_int8_t disinfect_time;
	u_int8_t disinfect_thermal_temp;
	u_int8_t disinfect_thermal_time;
	u_int8_t boil_temp;
	u_int8_t boil_power;
	u_int8_t boil_time;
	u_int8_t boil_thermal_temp;
	u_int8_t boil_thermal_time;
	u_int8_t water_temp;
	u_int8_t water_time;
	u_int8_t water_state;
	u_int8_t error;
	u_int8_t remain_water_time;
	u_int8_t product_status;
    u_int16_t disinfect_plan_id;
	u_int16_t boil_plan_id;
    u_int8_t disinfect_index;
    u_int8_t boil_index;
    u_int8_t work_mode; //工作模式
    u_int8_t water_warning;
    u_int8_t pad[2];
}cl_cp_wg_t;
    
    //情景链表处理数据结构
typedef struct {
	u_int16_t scene_id;
	u_int16_t modify;
}cp_id_modify_t;

typedef struct {
	u_int16_t count;
	u_int16_t pad;
	cp_id_modify_t id_modify[0];
}cp_id_hd_t;
    
//千帕茶盘情景定时器附加参数报文
typedef struct {
    u_int16_t id;
    u_int16_t pad;
}ucp_qp_timer_ext_t;

typedef struct {
    u_int16_t scene_id;
    u_int16_t pad;
}ucp_qp_pbj_timer_ext_t;
    
typedef struct {
    u_int8_t onOff;
    u_int8_t mode;
    u_int8_t temp;
    u_int8_t fan_speed;
    u_int8_t fan_dir;
    u_int8_t key_id;
    u_int16_t pad;
}ucp_808_timer_ext_t;

typedef struct {
    u_int8_t min_temp;
    u_int8_t max_temp;
    u_int8_t pad[2];
}ucp_101_oem_timer_ext_t;
    
typedef struct {
    u_int16_t cook_id;
    u_int16_t cook_time;
    u_int8_t hot_degress;//加热强度（等级）
    u_int8_t microswitch; //微动开关次数
    u_int8_t warm_temp; //保温温度
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
    u_int8_t pad[4];
}ucp_qp_pot_timer_ext_t;
    
typedef struct {
    u_int16_t scene_id;
    u_int8_t act; //收包时用
    u_int8_t pad;
    u_int8_t temp;
    u_int8_t work_time;
    u_int8_t power;
    u_int8_t keep_temp;
    u_int8_t keep_time;
    u_int8_t pad1[3];
}ucp_hx_ysh_timer_ext_t;
    
/////////////////////////////
//千帕锅报文
typedef struct {
    u_int16_t cook_id;
    u_int16_t cook_time;
    u_int8_t work_stat;
    u_int8_t pot_flag;
    u_int8_t warm_temp; //保温温度
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
    u_int8_t pot_temp;
    u_int8_t food_degress;
    u_int8_t err_code;
    u_int8_t pad;
    u_int32_t pad1;
}ucp_qp_pot_stat_t;
    
typedef struct {
    u_int16_t cook_id;
    u_int16_t cook_time;
    u_int8_t hot_degress;//加热强度（等级）
    u_int8_t microswitch; //微动开关次数
    u_int8_t warm_temp; //保温温度
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
    u_int8_t action; // 取消：开始 1：开始
    u_int8_t pad[3];
    u_int8_t pad1[4];
}ucp_qp_pot_stat_setting_t;

typedef struct {
    u_int16_t cook_id;
    u_int16_t cook_time;
    u_int8_t hot_degress;//加热强度（等级）
    u_int8_t microswitch; //微动开关次数
    u_int8_t warm_temp; //保温温度
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
    u_int8_t action;
    u_int8_t rice_degress;
    u_int8_t pad[2];
    u_int32_t create_time;
    u_int8_t name[CL_QPCP_NAME_MAX_LEN];
}ucp_qp_pot_scene_t;
    
typedef struct {
    u_int16_t count;
    u_int16_t pad;
    u_int16_t ids[0];
}ucp_qp_pot_scene_list_t;

/////////////////////////////////
//千帕破壁机
typedef struct {
    u_int8_t on_off;
    u_int8_t stat;
    u_int16_t cur_exe_id;
    u_int8_t mode;
    u_int8_t mix_power;
    u_int8_t temp;
    u_int8_t err_num;
}ucp_qp_pbj_stat_t;

typedef struct {
    u_int16_t scene_id;
    u_int16_t pad;
}ucp_qp_pbj_scene_exec_t;

typedef struct {
    u_int8_t action;
    u_int8_t p1;
    u_int8_t p2;
    u_int8_t p3;
}ucp_qp_pbj_scene_action_t;

typedef struct {
    u_int16_t scene_id;
    u_int8_t act; //收包时用
    u_int8_t pad;
    u_int8_t name[32];
    u_int32_t create_time;
    u_int8_t step_count;
    u_int8_t len;
    u_int8_t warm_time;
    u_int8_t warm_temp;
    ucp_qp_pbj_scene_action_t action[0];
}ucp_qp_pbj_scene_t;
    
typedef struct {
    u_int16_t count;
    u_int16_t pad;
    u_int16_t ids[0];
}ucp_qp_pbj_scene_list_t;

////////////////////////////////////
//海迅养生壶
typedef struct {
    u_int8_t cur_exe_id;
    u_int8_t flag;
    u_int8_t err_num;
    u_int8_t power;
    u_int8_t temp;
    u_int8_t mcu_timer;
    u_int8_t work_remain_time;
    u_int8_t pad[1];
    u_int8_t wifi_timer_exec_id;
    u_int8_t work_stat;
    u_int16_t wifi_timer;
}ucp_hx_ysh_stat_t;

typedef struct {
    u_int16_t scene_id;
    u_int8_t act; //收包时用
    u_int8_t pad;
    u_int8_t name[32];
    u_int32_t create_time;
    u_int8_t temp;
    u_int8_t work_time;
    u_int8_t power;
    u_int8_t keep_temp;
    u_int8_t keep_time;
    u_int8_t pad1[3];
}ucp_hx_ysh_scene_t;
    
////////////////////////////////////
    
#pragma pack(pop)
    
typedef struct {
    struct stlc_list_head link;
    u_int16_t modify;
    cl_qpcp_sp_t scene;
}cp_scene_node_t;
    
typedef struct {
    struct stlc_list_head link;
    cl_qp_pot_scene_t scene;
}qp_pot_scene_node_t;

typedef struct {
    struct stlc_list_head link;
    cl_qp_pbj_scene_t scene;
}qp_pbj_scene_node_t;

typedef struct {
    struct stlc_list_head link;
    cl_hx_ysh_scene_t scene;
}hx_ysh_scene_node_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
//千帕茶盘
void *qpcp_priv_init();
bool _qpcp_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
void _free_qpcp_info(cl_qpcp_info_t* info);
void _free_qpcp_sdk_info(cl_qpcp_priv_t* priv);
bool _qpcp_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
bool _qpcp_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
    
//千帕锅
extern bool qp_pot_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
extern bool qp_pot_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
extern bool qp_pot_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern void _free_qp_pot_sdk_info(cl_qpcp_priv_t* priv);
extern void _free_qp_pot_info(cl_qp_pot_info_t* info);

//千帕破壁机
extern bool qp_pbj_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
extern bool qp_pbj_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
extern bool qp_pbj_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern void _free_qp_pbj_sdk_info(cl_qpcp_priv_t* priv);
extern void _free_qp_pbj_info(cl_qp_pbj_info_t* info);
    
//海迅养生壶 因养生壶和千帕产品类似，所以放在这里实现。
extern bool hx_ysh_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
extern bool hx_ysh_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret);
extern bool hx_ysh_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern void _free_hx_ysh_sdk_info(cl_qpcp_priv_t* priv);
extern void _free_hx_ysh_info(cl_hx_ysh_info_t* info);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* QPCP_PRIV_H */

