#ifndef EVM_ZKCLEANNER_H
#define EVM_ZKCLEANNER_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_zkcleanner.h"


enum {
	ACT_ZKCLEANNER = 1,
};

#define ZKCLEANNER_UART_CMD_QUERY 0x55
#define ZKCLEANNER_UART_CMD_SET 0x66
#define ZKCLEANNER_UART_CMD_REPLY 0x77
#define ZKCLEANNER_UART_CMD_PT_SET 0x88
#define ZKCLEANNER_UART_CMD_PT_SET_REPLY 0x89

#pragma pack(push, 1)

typedef struct {
	u_int8_t syn;
	u_int8_t checksum;
	u_int8_t len;
	u_int8_t cmd;
} zkcleanner_uart_hdr_t;

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
} zkcleanner_uart_ctrl_t;

typedef struct {
	u_int8_t ontime_l;
	u_int8_t ontime_h;
	u_int8_t offtime_l;
	u_int8_t offtime_h;
} zkcleanner_uart_set_time_t;

#pragma pack(pop)

typedef struct {
	u_int8_t type;	// 型号1、2、3：分别表示三种机型
	u_int8_t onoff;
	u_int8_t temp;	// 温度0-40度
	u_int8_t pm25_l;
	u_int8_t pm25_h;
	u_int8_t co2_l;
	u_int8_t co2_h;
	u_int8_t hcho_l;
	u_int8_t hcho_h;
	u_int8_t voc_l;
	u_int8_t voc_h;
	u_int8_t wind;
	u_int8_t aqi;	// 空气质量
	u_int8_t mode;
	u_int8_t antibiosis;	// 抗菌
	u_int8_t fresh;	// 清新
	u_int8_t maintain; // 保养标志
	u_int8_t uptime_l;
	u_int8_t uptime_h;
	u_int8_t ontime_l;
	u_int8_t ontime_h;
	u_int8_t offtime_l;
	u_int8_t offtime_h;
} zkcleanner_uart_query_reply_t;

typedef struct {
	u_int8_t type;	// 型号1、2、3：分别表示三种机型
	u_int8_t onoff;
	u_int8_t temp;	// 温度0-40度
	u_int8_t pm25_l;
	u_int8_t pm25_h;
	u_int8_t co2_l;
	u_int8_t co2_h;
	u_int8_t hcho_l;
	u_int8_t hcho_h;
	u_int8_t voc_l;
	u_int8_t voc_h;
	u_int8_t wind;
	u_int8_t aqi;	// 空气质量
	u_int8_t mode;
	u_int8_t antibiosis;	// 抗菌
	u_int8_t fresh;	// 清新
	u_int8_t maintain; // 保养标志
	u_int8_t uptime_l;
	u_int8_t uptime_h;
	u_int8_t ontime_l;
	u_int8_t ontime_h;
	u_int8_t offtime_l;
	u_int8_t offtime_h;

	// 新添加的
	u_int8_t flag;	// bit 0 - 3: 版本号 bit 4 是否定时器改变
} zkcleanner_uart_query_reply_v2_t;

bool zkcleanner_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool zkcleanner_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool zkcleanner_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool zkcleanner_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif


