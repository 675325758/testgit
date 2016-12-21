#ifndef EVM_CJTHERMOSTAT_H
#define EVM_CJTHERMOSTAT_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_cjthermostat.h"

#pragma pack(push, 1)

// 串口头部
typedef struct {
	u_int8_t syn;
	u_int8_t cmd;
	u_int16_t addr;
} cjthermostat_uart_hdr_t;


typedef struct {
	u_int16_t outtime_hour;	// 输出时长，小时
	u_int8_t outtime_min;	// 输出时长，分钟
	u_int8_t ver;
	u_int8_t is_heat;	// 是否启动加热 0 未启动 1 启动
	u_int8_t week;	// 温控器星期
	u_int8_t time;	// 当前时间0-240,单位为6分钟
	u_int8_t stat;	// 0键无操作1 一般操作 2 后台设定 3 上述两种  “0：键无操作”是指温控器上电后或接到过清键指令后，没有再对按键进行过操作， 也可以理解为暂无操作。 这个UI上现在没有用到。
	u_int8_t set_temp;	// 设定温度，正数
	u_int8_t inside_temp;		// 内探头温度，整数部分。正数
	u_int8_t inside_temp1;	// 内抬头温度，小数部分
	u_int8_t outside_temp;	// 外抬头温度，整数部分正数
	u_int8_t outside_temp1;	// 外抬头温度，小数部分
	u_int8_t mode;	// 模式1 工作 2 手动3 休息4 自动 5 除霜 6 预置
	u_int8_t power;	// 0 关机1 开机屏幕显示2开机屏幕关闭
	u_int8_t key_lock;	// 0 键开放 1 键锁定
	u_int8_t fault;	// 错误 0 正常 1 测温探头短2测温探头开3保护探头短4保护过热
	int8_t temp_adjust;	// 温校:-9～9
	u_int8_t set_temp_upper_limit;	// 温度设置上限
	u_int8_t set_temp_lower_limit;	// 温度设置下限
	u_int8_t temp_allowance;	// 温控容差
	u_int8_t defrost_temp;	// 除霜温度
	u_int8_t overtemp;	// 过热温度
	u_int8_t overtemp_allowance;	// 过热容差
	u_int8_t flag;	// bit 0 型号:0: S型1:D型  bit 1 预置标志: 0 住宅模式1办公模式
	u_int8_t timer_week;	// 自动周期设置bit0-bit7表示星期7。1 ~6
	u_int8_t manual_temp;	// 手动模式温度设定
} cjthermostat_uart_stat_t;

#pragma pack(pop)


bool cjthermostat_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool cjthermostat_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool cjthermostat_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool cjthermostat_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif



