/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: ���о��â��ˮ��
**  File:    evm_jrxheater_ctrl.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    01/06/2016
**
**  Purpose:
**    ���о��â��ˮ��.
**************************************************************************/


#ifndef EVM_JRXHEATER_CTRL_H
#define EVM_JRXHEATER_CTRL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"
#include "cl_jrxheater.h"


/* Macro constant definitions. */
#define JRX_UART_CMD_PUSH_STAT 52
#define JRX_UART_CMD_STAT 53
#define JRX_UART_CMD_ONCE_TIMER 54
#define JRX_UART_CMD_PERIOD_TIMER 55



/* Type definitions. */
#pragma pack(push,1)

typedef struct {
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t cmd;
	u_int8_t dev_type;
	u_int8_t param_len;
} jrxheater_uart_hd_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t set_temp;
	u_int8_t childlock;
} jrxheater_uart_set_stat_t;

typedef struct {
	u_int8_t status;
	u_int8_t id;
	u_int8_t hour;
	u_int8_t min;
	u_int8_t temp;
} jrxheater_uart_once_timer_t;

typedef struct {
	u_int8_t status;
	u_int8_t id;
	u_int8_t start_hour;
	u_int8_t start_min;
	u_int8_t end_hour;
	u_int8_t end_min;
	u_int8_t onoff;
	u_int8_t temp;
} jrxheater_uart_period_timer_t;


typedef struct {
	u_int8_t onoff;	// �豸���أ�0-�ػ� 1-����
	u_int8_t mode;	// 0-����ģʽ 1-����ģʽ 2-ȫ���ģʽ 3-���Ի�ģʽ
	u_int8_t set_temp; // �û�������ˮ���������¶ȣ���Χ20-80���϶�
	u_int8_t child_lock; // ͯ�����أ�0-�ر� 1-����
	u_int8_t work_state; // ״̬��1-���� 2-����
	u_int8_t temp;	// ˮ���¶ȣ���ǰˮ����¶ȣ���Χ20-80���϶�
	u_int8_t capacity;	// ������ˮ���С����λL
	u_int8_t power; // ��ˮ�����ʣ���λW

	// APPҳ����Ҫ��ʾ��ˮ���ﵽˮ���¶ȵ�ʱ��
	u_int8_t hour;
	u_int8_t min;
} jrxheater_uart_stat_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

bool jrxheater_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool jrxheater_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool jrxheater_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool jrxheater_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
/*
	����: ��������
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������ACT_JRXHEATER_XXX����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jrxheater_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* EVM_JRXHEATER_CTRL_H */

