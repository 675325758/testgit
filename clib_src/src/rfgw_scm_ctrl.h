/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: rfgw_scm_ctrl.h
**  File:    rfgw_sctm_ctrl.h
**  Author:  liubenlong
**  Date:    02/27/2016
**
**  Purpose:
**    Rf����͸��ͷ�ļ�.
**************************************************************************/


#ifndef RFGW_SCTM_CTRL_H
#define RFGW_SCTM_CTRL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"

/* Macro constant definitions. */
//ҹ�Ǻ궨��
#define YL_BOOT_CODE		(0x55)
#define YL_FUNC_CODE_GET	(0x2)
#define YL_FUNC_CODE_SET	(0x3)

#define YL_ACTION_SET_ALARM_TIME	(0x1)
#define YL_ACTION_SET_SOUND			(0x2)
#define YL_ACTION_SET_HORN			(0x3)
#define YL_ACTION_SET_DEFENSE		(0x2)



/* Type definitions. */

#pragma pack(push,1)
//ҹ���������ݽṹ
typedef struct {
    u_int8_t boot_code;
	u_int8_t func_code;
	u_int8_t len;
	u_int8_t check_sum;
}ucp_rfgw_scm__head_t;

typedef struct {
	//ucp_rfgw_scm__head_t head;
	u_int8_t battery;//��ص�����bit0-6��(0 ��100),kBit7����Դѡ��(0��أ�1��ӵ�Դ)
	u_int8_t sos_alarm;//sos����ָ��(0���ޣ�1��SOS������)
	u_int8_t key_info;//������Ϣ(0 �� 255�������߼�������)
	u_int8_t door_voice;//����������С(0 �� 100)
	u_int8_t alarm_voice;//����������С(0 �� 100)
	u_int8_t alarm_time;//����ʱ��(0 - 255��)
	u_int8_t is_guard;
	u_int8_t pad[5];
}ucp_rfwg_scm_allstate_t;

//����ʱ������
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t time;
}ucp_rfgw_scm_alarm_time_t;

//����������С����
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t func;//Ϊ0x1�����õ���������Ϊ���������������Ϊ0x2�����õ���������Ϊ����������������
	u_int8_t voice;//������С
}ucp_rfgw_scm_horn_voice_t;

//���ȶ������Ʊ���
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t func;//Ϊ0x1ѡ�����壬0x2ѡ�񾯵ѡ�
	u_int8_t ctrl;//0 �رգ� 1������
}ucp_rfgw_scm_horn_action_t;

//һ����������
typedef struct {
	ucp_rfgw_scm__head_t head;
	u_int8_t action;
	u_int8_t is_defense;
	u_int8_t pad[6];
}ucp_rfgw_scm_defense_t;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */



extern bool rfgw_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool rfgw_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern void rfgw_do_query_hook(smart_air_ctrl_t* air_ctrl);
extern int rfgw_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);
extern void rf_scm_dev_set_defense_batch(user_t *puser, u_int8_t is_defence);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* RFGW_SCTM_CTRL_H */

