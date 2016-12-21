#ifndef	__CHIFFO_SCM_CTRL_H__
#define	__CHIFFO_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"
#include "cl_chiffo.h"

#define CH_DTYPE_PHONE_2_STM 0x0
#define CH_DTYPE_PHONE_CONTINUE_DATA	0x2

#define CH_STM_START_CODE 		0xAA
#define CH_STM_END_CODE		0x55

#define CH_STM_EXCEPT_PKT_LEN  24

// 1bit 掩码
#define BIT_MASK 0x1
//两bit 掩码
#define DBIT_MASK 0x3
// 3 bit 掩码
#define TBIT_MASK 0x7

//第2字节标志
#define B2_BIT_SHIFT_FIRE 4
#define B2_BIT_SHIFT_WATER 3
#define B2_BIT_SHIFT_FAN 2
//第3字节标志
#define B3_BIT_SHIFT_HEATER_FUNC 3
//第4字节标志
#define B4_BIT_SHIFT_HEATER_MODE  0
#define B4_BIT_SHIFT_PRESS_WANG	1
#define B4_BIT_SHIFT_PUMP			3
#define B4_BIT_SHIFT_ON_OFF		4
#define B4_BIT_SHIFT_WIFI			6
//第8字节标志
#define B8_BIT_SHIFT_DTYPE			0

// 命令号
#define CH_CMD_MODE_AUTO	0X3	// 自动模式
#define CH_CMD_MODE_BATH	0x5	// 浴缸模式
#define CH_CMD_MODE_DISHES	0x6	// 洗碗模式
#define CH_CMD_MODE_VEGETABLES	0x7	// 洗菜模式
#define CH_CMD_MODE_CLOTHES	0xA	// 洗衣模式


#define CH_CMD_FH_ON 	0x8 //开机
#define CH_CMD_FH_OFF 	0x9 //关机

#define CH_CMD_W_TEMP_ADD   0x4
#define CH_CMD_W_TEMP_DEC 0x0

#define CH_CMD_H_TEMP_ADD   0x11
#define CH_CMD_H_TEMP_DEC 0x12

#define CH_CMD_FH_H_ON	0x10
#define CH_CMD_FH_H_OFF	0x16

#define CH_CMD_SINGLE_LOOP 0x1A		// 单循环
#define CH_CMD_SMART_LOOP	0x1B	// 智能循环

#define CH_CMD_WATER_SET_TEMP 	0x76
#define CH_CMD_HEATER_SET_TEMP	0x77
#define CH_CMD_REFRESH_TIMER		0x79

#define CH_CMD_WATER_CAP_UP 0x24		// 水量升
#define CH_CMD_WATER_CAP_DOWN	0x25	// 水量降
#define CH_CMD_WATER_CAP	0x75		// 快速设置水量





#define CH_CMD_CONTINU_TYPE	0x62
#define CH_CMD_CONTINU_COUNT	0x63
#define CH_CMD_CONTINU_DATA	0x64

enum{
	CH_HS_FUNC_NO = 0x0, //无供暖功能
	CH_HS_FUNC_OFF,//供暖开启
	CH_HS_FUNC_ON //供暖关闭
};

enum{
    CH_ALTER_DT_DEV_TYPE = 0x0,
    CH_ALTER_DT_FAULT = 0x3,
    CH_ALTER_DT_HOUR = 0x4,
    CH_ALTER_DT_MIN = 0x5,
    CH_ALTER_DT_MAX = 0x8
};

enum{
	CH_DTYPE_REAL_TIME = 0x1,
	CH_DTYPE_WEEK_7_TIMER,
	CH_DTYPE_WEEK_1_TIMER,
	CH_DTYPE_WEEK_2_TIMER,
	CH_DTYPE_WEEK_3_TIMER,
	CH_DTYPE_WEEK_4_TIMER,
	CH_DTYPE_WEEK_5_TIMER,
	CH_DTYPE_WEEK_6_TIMER
};

#pragma pack(push,1)

typedef struct{
	u_int8_t s_code1;
	u_int8_t s_code2;
	u_int8_t len_low;
	u_int8_t len_high;
	u_int8_t d_type;
}ucp_chiffo_pkt_hdr_t;

typedef struct{
	u_int8_t crc_low;
	u_int8_t crc_high;
	u_int8_t e_code1;
	u_int8_t e_code2;
}ucp_chiffo_pkt_tail_t;

typedef struct{
	u_int8_t cmd_bit_len;
	u_int8_t cmd_low;
	u_int8_t cmd_high;
}ucp_chiffo_ctrl_key_t;

typedef struct{
	u_int8_t cmd;
	u_int8_t data;
}ucp_chiffo_continu_cmd_item_t;

#pragma pack(pop)

typedef struct{
    u_int8_t disp_fault;
	cl_chiffo_floor_heater_info_t heater_info;
}chiffo_priv_data_t;


extern bool chiffo_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool chiffo_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern bool chiffo_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);

extern int chiffo_get_ext_type_by_tlv(uc_tlv_t* tlv);

#endif

