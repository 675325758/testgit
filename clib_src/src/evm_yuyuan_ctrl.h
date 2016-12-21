#ifndef EVM_YUYUAN_CTRL_H
#define EVM_YUYUAN_CTRL_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_yuyuan.h"


enum {
	// "AT+WATER_BOX=1":����ˮ��ΪСˮ�䣬0-��ˮ�䣬1-Сˮ��
	ACT_YUYUAN_WATER_BOX = 0x1,
	// "AT+INLET_TIMEOUT=20"����ˮ��ʱ����λS����Χ0-100
	ACT_YUYUAN_INLET_TIMEOUT,
	// "AT+IMPULSE_COUNT=3"���������������
	ACT_YUYUAN_IMPULSE_COUNT,
	// "AT+IMPULSE_PERIOD=500"����������������
	ACT_YUYUAN_IMPULSE_PERIOD,
	// "AT+MCDELAY=90": ΢����λ��ʱ����λS��Ĭ��90����Χ0-300
	ACT_YUYUAN_MCDELAY,
	//"AT+ NM_VALVE_DELAY=90": ���׷���λ��ʱ����λS��Ĭ��90����Χ0-300
	ACT_YUYUAN_NM_VALVE_DELAY,
	// "AT+ FUNC_VALVE_TIMEOUT": ���ܷ���λ��ʱʱ�䣬Ĭ��120����Χ0-300
	ACT_YUYUAN_FUNC_VALE_TIMEOUT,
	// "AT+SPEED&SPEED1=10, SPEED2=11, SPEED3=12": 3��������ת�٣���Χ0-100
	ACT_YUYUAN_SPEED,
	// "AT+ LOOP_ONOFF=1"��ѭ�����أ�1-����0-��
	ACT_YUYUAN_LOOP_ONOFF,
	// "AT+ LOOP&LOOPD=10,LOOPH=10,LOOPM=30,LOOPT=5"��ѭ��ʱ��, �ڴﵽѭ������LPD��LPHʱLPM��ִ��LPT����ѭ��
	ACT_YUYUAN_LOOP,
	// "AT+ MCCLEAN&MCCLEAN_D=3, MCCLEAN_H=5, MCCLEAN_M=30, MCCLEAN_RT=14, MCCLEAN_DT=14"��΢�����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
	ACT_YUYUAN_MCCLEAN,
	// "AT+ NMCLEAN&NMCLEAN_D=3, NMCLEAN_H=6, NMCLEAN_M=30, NMCLEAN_RT=14, NMCLEAN_DT=14"�������Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
	ACT_YUYUAN_NMCLEAN,
	// "AT+ BEGIN_MICRO_CLEAN"��������ʼ΢������ϴ
	ACT_YUYUAN_BEGIN_MICRO_CLEAN,
	// "AT+ BEGIN_NM_CLEAN"��������ʼ������ϴ
	ACT_YUYUAN_NM_CLEAN,
	// "AT+ CHECK_SELF"��������ʼ�Լ졣�Լ������ģ����ȡ��ERROR�����ϱ���APP��
	ACT_YUYUAN_CHECK_SELF,
	// "AT+ REBOOT_CLEANER"������������ˮ��
	ACT_YUYUAN_REBOOT_CLEANER,
	// "AT+CONFIG=VALUE":����ģʽ���APP����һ������ģʽ�����������ڵ������滻��VALUE�����͸���λ�����������䡣VALUE�ĳ�������Ϊ50�ֽ��ڡ�
	ACT_YUYUAN_CONFIG,

	// ����
	ACT_YUYUAN_PWD,
	// ������Ϣ
	ACT_YUYUAN_REMIND,
};


bool yuyuan_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool yuyuan_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool yuyuan_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool yuyuan_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif
