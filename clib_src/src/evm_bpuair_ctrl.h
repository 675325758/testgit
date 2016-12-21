/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: evm_bpuair_ctrl
**  File:    evm_bpuair_ctrl.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/05/2016
**
**  Purpose:
**    Evm_bpuair_ctrl.
**************************************************************************/


#ifndef EVM_BPUAIR_CTRL_H
#define EVM_BPUAIR_CTRL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"


/* Macro constant definitions. */


/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	bool once_timer_enable;
	bool once_timer_onoff;	// һ���Զ�ʱ������ 0����ʱ�ػ�1����ʱ����
	u_int8_t once_timer_hour;	// һ���Զ�ʱ��Сʱ
	u_int8_t once_timer_min;	// һ���Զ�ʱ������
} once_timer_t;

typedef struct {
	u_int32_t time;
	bool onoff;
} nearest_timer_t;


typedef struct {
	u_int8_t set;
	u_int8_t hour;
	u_int8_t min;
} bpuair_tt_period_timer_t;

typedef struct {
	/*
		
	Զ�̿��ع��ܣ�
	Bit1�������0-��Ч��1-��Ч��Bit0�������0-��Ч��1-��Ч��
	����ģʽ�л�:����ůֻֻ�����ȣ���ů�������ȶ�֧�֣�
	Bit2��0-���䣬1-���ȣ�
	����ģʽ�л�:
	Bit3��0-�رգ�1-������
		
	*/
	u_int8_t onoff;
	u_int16_t cold_temp;
	u_int16_t hot_temp;
	/*
		
	��ʱ����
	Bit0����ʱ�ܿ��أ�0����ʹ��1��ʹ�ã� Bit1������
	Bit2������							  Bit3������
	Bit4������							  Bit5������
	Bit6������							  Bit7��һ�ζ�ʱ���ã�0����ʱ�ػ�1����ʱ������
	*/
	u_int8_t timer_set;
	u_int8_t once_timer_hour;
	u_int8_t once_timer_min;

	bpuair_tt_period_timer_t period_timer[6];
	u_int8_t pad;
} bpuair_tt_ctrl_t;

typedef struct {
	u_int8_t type;	// �豸����
	/*
		
		0��������
		1�����У�
		2������ͣ����
	*/
	u_int8_t stat1; 
	/*
		0:��
		1��������
		2����˪��
		3�������ͳ�˪��
	*/
	u_int8_t stat2;
	u_int8_t work_mode; // ����ģʽ 1���� 2 ����
	
	u_int8_t eco_mode;	// ����ģʽ0 �ر� 1����
	int16_t cold_temp;	// �����趨�¶ȣ���λ0.1�ȣ�10.0 ~ 30.0
	int16_t hot_temp;		// �����趨�¶ȣ���λ0.1�ȣ�10.0 ~ 85.0
	int16_t backwater_temp; // ��ˮ�¶ȵ�λ0.1��
	int16_t water_temp; // ϵͳ��ˮ�¶ȵ�λ0.1��
	int16_t env_temp;	// �����¶ȵ�λ0.1��
	int16_t coiler1_temp;	// �̹��¶ȵ�λ0.1��
	int16_t coiler2_temp;	// �̹��¶ȵ�λ0.1��
	int16_t current1;	// ����1 ��λA
	int16_t current2;	// ����2 ��λA
	int16_t eco_cold_temp;	// ��������趨�¶ȣ���Χ��10.0��C~30.0��C��
	int16_t eco_hot_temp;	// ���Ƚ����趨�¶ȣ���Χ��10.0��C~85.0��C��
	u_int8_t pad[3];
	u_int8_t err[8];

	u_int8_t timer_set;
	u_int8_t once_timer_hour;
	u_int8_t once_timer_min;

	bpuair_tt_period_timer_t period_timer[6];
	u_int16_t uptime;	// �ۼ�����Сʱ
} bpuair_tt_stat_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
bool bpuair_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool bpuair_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool bpuair_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool bpuair_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* EVM_BPUAIR_CTRL_H */

