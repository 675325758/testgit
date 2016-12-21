/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: �򵥵�UDP���������
**  File:    uc_agent.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    05/31/2016
**
**  Purpose:
**    �򵥵�UDP���������.
**************************************************************************/


#ifndef UC_AGENT_H
#define UC_AGENT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "client_lib.h"
#include "cl_priv.h"
#include "cl_sys.h"
#include "md5.h"
#include "cl_thread.h"
#include "aes.h"
#include "tea.h"

#include "udp_ctrl.h"
#include "uc_client.h"


/* Macro constant definitions. */
#define MAX_UA_NODE_NUM 64
//#define UA_MASTER_PORT 49666
#define UA_MASTER_PORT 49668




enum {
	UA_NODE_STAT_IDLE = 0,
	UA_NODE_STAT_REGISTING = 1,
	UA_NODE_STAT_ESTB,
};


/* Type definitions. */
#pragma pack(push, 1)

enum {
	AF_IPV4 = 4,
	AF_IPV6 = 6,
};

enum {
	UA_CMD_REQUEST = 1,
	UA_CMD_RELEASE = 2,
};

enum {
	UA_PROXY_TYPE_RANSPAREND = 1,	// ͸�������ֻ����豸ͨ��ʹ�ñ����ͣ�ÿ���豸����һ			����������ͨ��udp�غ������ڱ��ֲ���
	UA_PROXY_TYPE_IP_IN_IP = 2,	// ��͸�������ֻ���������ͨ��ʹ�ñ����ͣ�����ͨ��udp�غɷ�			װĿ��ip���˿ڵ�
};

enum {
	UA_ENC_TYPE_NONE = 0,
	UA_ENC_TYPE_AES128 = 1,
	UA_ENC_TYPE_RSA1024  = 4,
};

// ��������
typedef struct {
	#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	ver:4,
				af:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t 	af:4,
				ver:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t enc_type;
	u_int16_t random1;
	u_int8_t param[0];
} uc_request_hdr_t;

typedef struct {
	u_int16_t random2;
	u_int16_t dest_port;
	u_int8_t cmd;
	u_int8_t proxy_type;
	u_int16_t pad;
	u_int32_t timestamp;
	u_int32_t ip;
} uc_request_t;

typedef struct {
	u_int16_t random2;
	u_int16_t proxy_port;
	u_int8_t cmd;
	u_int8_t proxy_type;
	u_int16_t pad;
	u_int32_t timestamp;
	u_int32_t proxy_ip;
} uc_request_reply_t;

typedef struct {
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		u_int8_t	ver:4,
					af:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
		u_int8_t	af:4,
					ver:4;
#else
# error "Please fix <bits/endian.h>"
#endif

	u_int8_t proxy_type;
	u_int16_t dest_port;
	u_int32_t dest_ip;
	u_int8_t payload[0];
} uc_ip_hdr_t;

#pragma pack(pop)


typedef struct {
	u_int16_t handle;	// ͬ���������random
	u_int8_t stat;	// ״̬ UA_NODE_STAT_XXX
	u_int8_t enc_type; //  UC_ENC_TYPE_XXX
	u_int8_t proxy_type;	// �������� UA_TRANS_TYPE_XXX
	int request_count;
	struct sockaddr dst_addr;	// ת������Ŀ�ĵ�ַ��������
	struct sockaddr agent_addr;		// ת����������ַ��������
	struct sockaddr master_addr;
	cl_thread_t *t_timer;
} ua_node_t;


typedef struct {
	ua_node_t node[MAX_UA_NODE_NUM];	// ��ʵֻ��Ҫ2������Ϊ������ת���ķ�͸��������Ϊ�豸��͸������
	struct sockaddr master_addr;	// �������ķ�������ַ�����Իظ����ķ�����

	SOCKET agent_sock;		// ר�������������ͨ����socket
	cl_thread_t *t_agent_read;	// ����sock�Ķ�

	// ��Ϊ���뱨�ĺͷ�͸��ת�����Ĳ�̫�����֣��ɴ����2��sock
	// ���ڷ�͸��ת����sock
	SOCKET sock;
	cl_thread_t *t_read;

	void *user;
} ua_mgr_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
void ua_init(ua_mgr_t *uamgr);
RS ua_request_agent(ua_mgr_t *uamgr, struct sockaddr dst_addr, u_int8_t type);
void ua_set_master(ua_mgr_t *uamgr, struct sockaddr *addr);
void ua_create(ua_mgr_t **ppuamgr, void *user);
void ua_request_transparent_agent(ua_mgr_t *uamgr, u_int32_t ip, u_int16_t port);
RS ua_trans_send(ua_mgr_t *uamgr, u_int32_t ip, u_int16_t port, u_int8_t *data, int len);
void ua_request_ipinip_agent(ua_mgr_t *uamgr);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* UC_AGENT_H */

