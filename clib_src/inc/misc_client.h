/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: misc client
**  File:    misc_client.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    12/01/2016
**
**  Purpose:
**    Misc client.
**************************************************************************/


#ifndef MISC_CLIENT_H
#define MISC_CLIENT_H


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
#include "stlc_list.h"
#include "udp_ctrl.h"
#include "uc_client.h"


/* Macro constant definitions. */


/* Type definitions. */

#pragma pack(push,1)

typedef struct {
	u_int8_t ver;
	u_int8_t flag;
	u_int16_t handle;
	u_int32_t timestamp;
	u_int32_t magic;
	u_int16_t type;
	u_int16_t len;
	u_int8_t data[0];
} mchdr_t;
#define mc_hdr_size sizeof(mchdr_t)

#define mc_plain_size 4

#pragma pack(pop)

typedef enum {
	MCT_APNS = 1,
	MCT_UPGRADE = 2,
} MC_TYPE_T;

typedef void (*mc_pkt_callback)(cl_handle_t user_handle, u_int16_t type, u_int16_t len, u_int8_t *data);

typedef struct {
	struct stlc_list_head link;

	cl_handle_t user_handle;
	u_int16_t seq;
	u_int8_t try_count;
	u_int32_t ip;
	mc_pkt_callback cb;
	u_int32_t total;
	u_int8_t data[0];
} mc_pkt_t;

typedef struct {
	SOCKET sock;
	struct stlc_list_head send_list;
	cl_thread_t *t_send;
	cl_thread_t *t_read;
	u_int8_t key[16];
} mc_mgr_t;
/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

extern RS misc_client_do_request(cl_handle_t user_handle, u_int16_t type, u_int16_t len, u_int8_t *data, u_int32_t ip, mc_pkt_callback cb);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* MISC_CLIENT_H */

