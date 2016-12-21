#ifndef EVM_YUYUAN_CTRL_H
#define EVM_YUYUAN_CTRL_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_yuyuan.h"


enum {
	// "AT+WATER_BOX=1":设置水箱为小水箱，0-大水箱，1-小水箱
	ACT_YUYUAN_WATER_BOX = 0x1,
	// "AT+INLET_TIMEOUT=20"：进水超时，单位S，范围0-100
	ACT_YUYUAN_INLET_TIMEOUT,
	// "AT+IMPULSE_COUNT=3"：设置脉冲个数，
	ACT_YUYUAN_IMPULSE_COUNT,
	// "AT+IMPULSE_PERIOD=500"：设置脉冲检测周期
	ACT_YUYUAN_IMPULSE_PERIOD,
	// "AT+MCDELAY=90": 微处理定位延时，单位S，默认90，范围0-300
	ACT_YUYUAN_MCDELAY,
	//"AT+ NM_VALVE_DELAY=90": 纳米阀定位延时，单位S，默认90，范围0-300
	ACT_YUYUAN_NM_VALVE_DELAY,
	// "AT+ FUNC_VALVE_TIMEOUT": 功能阀定位超时时间，默认120，范围0-300
	ACT_YUYUAN_FUNC_VALE_TIMEOUT,
	// "AT+SPEED&SPEED1=10, SPEED2=11, SPEED3=12": 3个部件的转速，范围0-100
	ACT_YUYUAN_SPEED,
	// "AT+ LOOP_ONOFF=1"：循环开关，1-开，0-关
	ACT_YUYUAN_LOOP_ONOFF,
	// "AT+ LOOP&LOOPD=10,LOOPH=10,LOOPM=30,LOOPT=5"：循环时间, 在达到循环周期LPD天LPH时LPM分执行LPT分钟循环
	ACT_YUYUAN_LOOP,
	// "AT+ MCCLEAN&MCCLEAN_D=3, MCCLEAN_H=5, MCCLEAN_M=30, MCCLEAN_RT=14, MCCLEAN_DT=14"：微处理自动清洗：天-时-分-反冲时长（MIN）-直洗时长（MIN）
	ACT_YUYUAN_MCCLEAN,
	// "AT+ NMCLEAN&NMCLEAN_D=3, NMCLEAN_H=6, NMCLEAN_M=30, NMCLEAN_RT=14, NMCLEAN_DT=14"：纳米自动清洗：天-时-分-反冲时长（MIN）-直洗时长（MIN）
	ACT_YUYUAN_NMCLEAN,
	// "AT+ BEGIN_MICRO_CLEAN"：立即开始微处理清洗
	ACT_YUYUAN_BEGIN_MICRO_CLEAN,
	// "AT+ BEGIN_NM_CLEAN"：立即开始纳米清洗
	ACT_YUYUAN_NM_CLEAN,
	// "AT+ CHECK_SELF"：立即开始自检。自检过程中模组会获取到ERROR，并上报到APP。
	ACT_YUYUAN_CHECK_SELF,
	// "AT+ REBOOT_CLEANER"：立即重启净水器
	ACT_YUYUAN_REBOOT_CLEANER,
	// "AT+CONFIG=VALUE":工程模式命令，APP上有一个工程模式输入框，输入框内的内容替换掉VALUE，发送给下位机，方便扩充。VALUE的长度限制为50字节内。
	ACT_YUYUAN_CONFIG,

	// 密码
	ACT_YUYUAN_PWD,
	// 提醒信息
	ACT_YUYUAN_REMIND,
};


bool yuyuan_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool yuyuan_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool yuyuan_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool yuyuan_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif
