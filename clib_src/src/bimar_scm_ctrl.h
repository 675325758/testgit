#ifndef	__BIMAR_SCM_CTRL_H__
#define	__BIMAR_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "cl_bimar_dev.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

#define BIMAR_ACT_QUERY 0x1
#define BIMAR_ACT_SET 0x2
#define BIMAR_ACT_PUSH 0x3

#define BIMAR_START_CODE 0xF1
#define BIMAR_RSP_S_CODE 0xF2
#define BIMAR_END_CODE 0x7E

//开关
#define BIMAR_ON 0x1
#define BIMAR_OFF 0x2
//SCM底层运行测试
#define AUCMA_RUN_MODE_NONE          0x00
#define AUCMA_RUN_MODE_TEMP          0x01
#define AUCMA_RUN_MODE_GEAR          0x02


#define BIMAR_UN_SUPPORT_FUNC 0x0


#pragma pack(push,1)

typedef struct {
    u_int8_t start_1;
    u_int8_t start_2;
    u_int8_t cmd;
    u_int8_t len;
}stm_bimar_head_t;

typedef struct {
    u_int8_t end;
}stm_bimar_tail_t;


typedef struct {
    u_int8_t onoff;
    u_int8_t shake_stat;
    u_int8_t auto_valid;
    u_int8_t anion_onoff;
    u_int8_t fan_speed;
    u_int8_t temp_unit;
    u_int8_t gear;
    u_int8_t set_c_temp;
    u_int8_t set_f_temp;
    u_int8_t timer;
    u_int8_t c_room_temp;
    u_int8_t f_room_temp;
    u_int8_t setting_min;
    u_int8_t pad;
    u_int8_t machine_type;
    u_int8_t crc;
}stm_bimar_body_t;

#pragma pack(pop)

extern bool bimar_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool bimar_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

extern int bimar_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);

#endif

