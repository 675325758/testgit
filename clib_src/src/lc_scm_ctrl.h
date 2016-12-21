#ifndef	__LC_SCM_CTRL_H__
#define	__LC_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"

//Bit0：0关机，1开机
#define FHF_LC_UART_CMD_FLAG_STATE (0x1 << 0)
//Bit1：0低档，1高档
#define FHF_LC_UART_CMD_FLAG_GEAR (0x1 << 1)
//Bit2：0不摇头，1摇头
#define FHF_LC_UART_CMD_FLAG_SHAKE_HEAD (0x1 << 2)
//Bit3：0不在ECO，1在EC0模式
#define FHF_LC_UART_CMD_FLAG_ECO (0x1 << 3)
//Bit4：0发热体不工作,在过温保护状态，1发热体在工作
#define FHF_LC_UART_CMD_FLAG_HEATWORK (0x1 << 4)
//串口发包队列最大值
#define FHF_UART_SEND_LIST_MAX 25
//串口发包错误最大重试次数
#define FHF_UART_SEND_TRY_MAX 3
//串口发包超时时间,单位秒
#define FHF_UART_SEND_TIMEOUT 1
//串口发包超时时间,单位毫秒
#define FHF_UART_SEND_TIMEOUT_MS	500
//状态刷新定时器时间,单位秒
#define FHF_STATE_REFRESH_TIME 3

enum {
	FHF_LC_OPERATION_SUB = 0,
	FHF_LC_OPERATION_ADD = 1
};

enum {
	//命令类型：温度设定值
	FHF_LC_UART_CMD_TYPE_SET_TEMP = 0x1,
	//命令类型：定时设定值
	FHF_LC_UART_CMD_TYPE_SET_TIMER = 0x2
};

enum {
	//获取参数命令类型环所有
	FHF_LC_UART_CMD_TYPE_GET_ALL = 0x0,
	//获取参数命令类型环境温度值
	FHF_LC_UART_CMD_TYPE_GET_ENV_TEMP = 0x1,
	//获取参数命令类型发热体1温度值
	FHF_LC_UART_CMD_TYPE_GET_HEATING_UNIT1_TEMP = 0x2,
	//获取参数命令类型发热体2温度值
	FHF_LC_UART_CMD_TYPE_GET_HEATING_UNIT2_TEMP = 0x3,
	//获取参数命令类型剩余定时时间
	FHF_LC_UART_CMD_TYPE_GET_REMAIN_MINUTE = 0x4,
	//获取参数命令类型当前消耗电量值
	FHF_LC_UART_CMD_TYPE_GET_POWER = 0x5,
	//获取参数命令类型设置温度值
	FHF_LC_UART_CMD_TYPE_GET_SET_TEMP = 0x6
};

enum {
	//设置参数命令
	FHF_LC_UART_CMD_SET_PARAM = 0x1,
	//按键动作命令
	FHF_LC_UART_CMD_SET_PRESS_KEY = 0x2,
	//获取参数命令
	FHF_LC_UART_CMD_GET_PARAM = 0x3,
	//获取工作状态命令
	FHF_LC_UART_CMD_GET_STATE = 0x4,
	//增减参数命令
	FHF_LC_UART_CMD_ADD_OR_SUB_PARAM = 0x5,
	//获取产品序列号命令
	FHF_LC_UART_CMD_GET_SN = 0x6,
	//错误数据包返回命令
	FHF_LC_UART_CMD_ERROR_REPLY = 0x80,
	//设置参数命令的应答命令
	FHF_LC_UART_CMD_SET_PARAM_REPLY = 0x81,
	//按键动作的应答命令
	FHF_LC_UART_CMD_SET_PRESS_KEY_REPLY = 0x82,
	//获取参数的应答命令
	FHF_LC_UART_CMD_GET_PARAM_REPLY = 0x83,
	//获取工作状态的应答命令
	FHF_LC_UART_CMD_GET_STATE_REPLY = 0x84,
	//增减参数的应答命令
	FHF_LC_UART_CMD_ADD_OR_SUB_PARAM_REPLY = 0x85,
	//获取产品序列号的应答命令
	FHF_LC_UART_CMD_GET_SN_REPLY = 0x86,
	//激活处理
	FHF_LC_UART_CMD_ACTIVE = 0x87
};

enum {
	//设置命令关机
	FHF_LC_UART_SET_COMMAND_SHUTDOWN = 0x0,
	//设置命令开机
	FHF_LC_UART_SET_COMMAND_STARTING_UP = 0x1,
	//设置命令低档
	FHF_LC_UART_SET_COMMAND_GEAR_LOW = 0x2,
	//设置命令高档
	FHF_LC_UART_SET_COMMAND_GEAR_HIGH = 0x3,
	//设置命令摇头
	FHF_LC_UART_SET_COMMAND_SHAKE_HEAD = 0x4,
	//设置命令ECO
	FHF_LC_UART_SET_COMMAND_ECO = 0x5,
	//设置命令设置
	FHF_LC_UART_SET_COMMAND_SETTING = 0x6,
	//设置命令增加
	FHF_LC_UART_SET_COMMAND_ADD = 0x7,
	//设置命令减少
	FHF_LC_UART_SET_COMMAND_SUB = 0x8,
	//设置命令档位切换
	FHF_LC_UART_SET_COMMAND_SWITCH_GEAR = 0x9,
	//设置命令开关切换
	FHF_LC_UART_SET_COMMAND_SWITCH_ON_OFF = 0xA
};

enum {
	//就绪状态，可以发送命令
	FHF_LC_UART_STATE_READY = 0x0,
	//就绪状态，等待回包或超时后在发送
	FHF_LC_UART_STATE_WORKING = 0x1
};

/* Type definitions. */
#pragma pack(push,1)

typedef struct lc_ucd_pkt_request_head_s{
	u_int8_t len;
	u_int8_t flag;
	u_int8_t cmd;
	u_int8_t data_sub_type;
	u_int8_t data[0];
}lc_ucd_pkt_request_head_t;

typedef struct lc_ucd_pkt_response_head_s{
	u_int8_t len;
	u_int8_t flag;
	u_int8_t cmd;
	u_int8_t data_sub_type;
	u_int8_t err_no;
	u_int8_t data[0];
}lc_ucd_pkt_response_head_t;

//83 00
typedef struct {
	u_int16_t pad;
	//环境温度
	u_int8_t env_temp;
	//发热体1温度值
	u_int8_t heating_unit1_temp;
	//发热体2温度值
	u_int8_t heating_unit2_temp;
	//剩余定时时间
	u_int8_t remain_hour;
	//当前消耗电量值
	u_int32_t power_low;
	u_int32_t power_high;
	//设置温度
	u_int8_t set_temp;
}fhf_uart_get_param_data_t;

typedef struct {
	u_int32_t power_low;
	u_int32_t power_high;
}fhf_uart_get_power_t;
//84 00
typedef struct {
	/*当前的工作状态:
	*Bit0：0关机,1开机
	*Bit1：0低档,1高档
	*Bit2：0不摇头,1摇头
	*Bit3：0不在ECO,1在EC0模式*/
	u_int8_t state;
	/*故障状态,0：代表正常,非0代表故障,故障具体定义如下
	*Bit0：倾倒保护
	*Bit1：环境温度过高保护
	*Bit2：发热体温度过高保护
	*Bit3：程序执行错误*/
	u_int8_t fault_state;
}fhf_uart_get_state_data_t;

#pragma pack(pop)

extern bool lc_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool lc_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern int lc_scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
extern int lc_get_ext_type_by_ident(u_int8_t* ident,int ident_len);

#endif

