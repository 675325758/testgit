/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: indiacar server client
**  File:    ica_priv.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    12/30/2015
**
**  Purpose:
**    Indiacar server client.
**************************************************************************/


#ifndef ICA_PRIV_H
#define ICA_PRIV_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "client_lib.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "client_lib.h"
#include "cl_priv.h"
#include "cl_sys.h"
#include "cl_thread.h"
#include "cl_notify.h"
#include "mp4_fmt.h"

/* Macro constant definitions. */
//#define MAX_ICA_PKT_PARAM_SIZE 8196
#define MAX_ICA_PKT_PARAM_SIZE (1024*64)


/* Type definitions. */
typedef enum {
        ICA_CMD_VIDEO = 2,
        ICA_CMD_AGENT_ADDR = 3,
        ICA_CMD_RESET = 4,
        ICA_CMD_AUDIO = 5,
} ica_proto_t;


// 通讯协议
#pragma pack(push,1)

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
		u_int8_t ver:3,
			hlen:3,
			compress:1,
			encrypt:1;
#elif __BYTE_ORDER == __BIG_ENDIAN
		u_int8_t	encrypt:1,
			compress:1,
			hlen:3,
			ver:3;

# error "Please fix <bits/endian.h>"
#endif
	u_int8_t ds_type;
	u_int16_t command;
	u_int32_t param_len;
	u_int8_t flags;
	u_int8_t resv[3];
	u_int64_t sn;
} ica_header_t;
#define ica_header_real_size(h)	(((h)->hlen) << 2)
#define ica_header_size	(sizeof(ica_header_t))

#define BOFP1(buf, ofs) (&((char *)(buf))[ofs])
#define	get_ica_pkt_payload(pkt, type) (type *)BOFP1((pkt)->data, ica_header_size)

typedef struct {
	// ART_XXX
	u_int8_t action;
	u_int8_t reserved[3];
	// 随机数
	u_int8_t rand1[4];
	u_int64_t sn;
	// 客户端唯一标识
	u_int8_t my_uuid[MAX_UUID_BIN_LEN];
} auth_request_t;

typedef struct {
	// ART_XXX
	u_int8_t action;
	u_int8_t reserved[3];
	// 随机数
	u_int8_t rand2[4];
	u_int8_t check_type;	
	u_int8_t resv2[3];
} auth_question_t;

typedef struct {
	u_int8_t action;
	u_int8_t resv;
	u_int16_t support_method;
	u_int8_t answer[16];
} auth_answer_t;;

typedef struct {
	u_int8_t action;
	u_int8_t resv;
	u_int16_t select_enc;
	u_int8_t err;
	u_int8_t keeplive_inv;
	u_int16_t resv2;
} auth_result_t;

typedef struct {
	u_int8_t type;
	u_int8_t pad[3];
	u_int8_t data[0];
} video_data_t;

typedef struct {
	u_int8_t channel;
	u_int8_t bit;
	u_int8_t mode;
	u_int8_t pad;
	u_int32_t time;
	u_int8_t data[0];
} audio_data_t;


#pragma pack(pop)

typedef struct {
	cl_thread_info_t ti_mp4_decode;

	// user 回调
	cl_handle_t handle;
	cl_callback_t callback;
	void *callback_handle;
	
	data_exchg_t *data_pic; /* 组装解码出来的图片缓冲区 */

	u_int8_t need_seek;
	u_int64_t seek;	// 跳转到的地方，毫秒
	u_int32_t start_time;	// 开始播放时间，毫秒(绝对时间)
	u_int32_t next_show_time;	// 预计下一次开始展示图片时间(从0开始)
	u_int8_t need_puse;	// 需要暂停下

	void *h264_handle;	// 解析H264到bmp的handle
	
	mp4_handle_t *mp4_decode_handle;	// MP4读handle
	char path[256];	// mp4文件地址
	u_int32_t mp4_duration;	// mp4播放总时间，单位毫秒
} ica_mp4_decode_t;

typedef struct ica_client_s {
	u_int32_t is_valid;

	u_int8_t has_get_first_i;
	
	//cl_handle_t user_handle;	// 绑定在哪个user下面
	u_int64_t sn;

	//
	cl_handle_t handle;
	cl_callback_t callback;
	void *callback_handle;

	video_t *video;
	
	SOCKET sock;
	u_int32_t agent_ip;
	u_int16_t agent_port;

	// 加密相关
	u_int16_t agent_select_enc;
	u_int8_t agent_key[16];
	
	int state;	// ICAS_XXX

	cl_thread_t *t_read;
	cl_thread_t *t_write;
	cl_thread_t *t_timer;

	struct stlc_list_head pkt_list;

	// 加密相关
	u_int8_t rand1[4];
	u_int8_t rand2[4];
	u_int8_t key[16];

	ica_header_t hd;
	u_int8_t param[MAX_ICA_PKT_PARAM_SIZE];
	int recv_len;
	int param_len;

	// 写录像
	mp4_handle_t *mp4_handle;
	bool mp4_get_first_i;
	u_int8_t record_path[512];
	u_int8_t audio_record_path[512];
} ica_client_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
int icac_start(user_t *user, ica_client_t **pp);
void icac_stop(ica_client_t *c);
bool ica_video_open(user_t *user, video_t *video);
void icac_set_agent_info(ica_client_t *client, u_int32_t agent_ip, u_int16_t agent_port, u_int16_t select_enc, u_int8_t key[16]);
bool icac_is_working(ica_client_t *c);
bool icac_is_record(ica_client_t *c);
extern    void icac_set_record(ica_client_t *c, u_int8_t *path, bool onoff);
extern void ica_do_mp4_decode(user_t *user, ica_mp4_decode_t **decode, u_int64_t seek, u_int8_t *path, u_int8_t action);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ICA_PRIV_H */

