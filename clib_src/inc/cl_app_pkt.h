/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: APP�ϲ���ο���ͨ��
**  File:    cl_app_pkt.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    07/26/2016
**
**  Purpose:
**    APP�ϲ���ο���ͨ��.
**************************************************************************/


#ifndef CL_APP_PKT_H
#define CL_APP_PKT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "client_lib.h"
#include "cl_app_pkt.h"

/* Macro constant definitions. */


/* Type definitions. */

typedef enum {
	pt_server = 1,	// ������
	pt_app,
	pt_all_online_app,
	pt_all_app,
	pt_wifi_dev,
	pt_macbee_dev,			// macbee�豸����������
	pt_macbee_dev_cache,	// macbee���������ص����ݣ��Զ���ȡ
} PEER_TYPE_T;

typedef enum {
	PI_DETAIL_TYPE = 1,	// ���豸�ľ������ͣ�2�ֽ�
} PEER_INFO_T;

typedef struct {
	PEER_TYPE_T type;
	u_int64_t ident;
	int pkt_len;
	u_int8_t pkt[0];
} cl_app_pkt_send_t;

typedef void (*cl_app_proc_fun)(PEER_TYPE_T peer_type, u_int64_t ident, char* pkt , int pkt_len);

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	���ܣ�
		����һ�����ݱ���
	�������:
		@peer_type:����Ŀ�ĵ�����
		@ident: �Զ�ʶ���־��������͸��豸��������SN
		@pkt: ���͵�����
		@pkt_len: ���͵����ݳ���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_send_pkt(PEER_TYPE_T peer_type, u_int64_t ident, char* pkt , int pkt_len);

/*
	���ܣ�
		����һ���������ݻص��Ľӿ�
	�������:
		@fun: �ص�
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_set_proc_pkt_callback(cl_app_proc_fun fun);

/*
	���ܣ�
		���ÿ�����ID
	�������:
		@id: ������ID
	�������:
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_set_developer_id(char *id);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_APP_PKT_H */

