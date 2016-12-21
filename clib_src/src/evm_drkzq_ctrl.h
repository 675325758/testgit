#ifndef EVM_DRKZQ_H
#define EVM_DRKZQ_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_drkzq.h"


enum {
	ACT_DRKZQ = 1,
};

#define DRKZQ_UART_CMD_QUERY 0x55
#define DRKZQ_UART_CMD_SET 0x66
#define DRKZQ_UART_CMD_REPLY 0x77
#define DRKZQ_UART_CMD_PT_SET 0x88
#define DRKZQ_UART_CMD_PT_SET_REPLY 0x89

#pragma pack(push, 1)

typedef struct {
	u_int8_t syn;
	u_int8_t cmd1;
	u_int8_t len;
} drkzq_uart_hdr_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t wind;
	u_int8_t antibiosis;	// 抗菌
	u_int8_t fresh;	// 清新
	u_int8_t maintain; // 保养标志

	u_int8_t ontime_l;
	u_int8_t ontime_h;
	u_int8_t offtime_l;
	u_int8_t offtime_h;
} drkzq_uart_ctrl_t;

typedef struct {
	u_int8_t ontime_l;
	u_int8_t ontime_h;
	u_int8_t offtime_l;
	u_int8_t offtime_h;
} drkzq_uart_set_time_t;

#pragma pack(pop)

typedef struct {
	u_int8_t humi1;
	u_int8_t temp1;
	u_int8_t humi2;
	u_int8_t temp2;
	u_int8_t humi3;
	u_int8_t temp3;
	u_int8_t humi4;
	u_int8_t temp5;
	
} drkzq_uart_query_reply_t;

typedef struct {
	u_int8_t type;
	char name[64];
} drkzq_set_name_t;

typedef struct {
	u_int8_t id;
	u_int8_t valid;
	u_int8_t start_hour;
	u_int8_t start_min;
	u_int8_t end_hour;
	u_int8_t end_min;
} drkzq_set_timer_t;

typedef struct {
	u_int8_t start_min;
	u_int8_t start_hour;
	u_int8_t end_min;
	u_int8_t end_hour;
	u_int8_t valid;
} drkzq_timer_set_param_t;

bool drkzq_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool drkzq_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool drkzq_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool drkzq_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif


