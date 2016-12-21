/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: misc client
**  File:    misc_client.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    12/01/2016
**
**  Purpose:
**    Misc client.
**************************************************************************/


/* Include files. */
#include "misc_client.h"

/* Macro constant definitions. */
#define MC_KEY "APP2SERVER"
#define MC_MAGIC 0x6831

/* Type definitions. */
#if 1
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

static mc_mgr_t *mcmgr = NULL;

static void misc_client_pkt_free(mc_pkt_t *pkt)
{
	stlc_list_del(&pkt->link);
	cl_free(pkt);
}

static int misc_client_read(cl_thread_t *t)
{
	mc_mgr_t *mcmgr;
	u_int8_t data[2048] = {0};
	struct sockaddr_in addr;
	int len_addr = sizeof(addr);
	int n, pad_len;
	mchdr_t *hdr;
	mc_pkt_t *pkt;
	
	mcmgr = (mc_mgr_t*)CL_THREAD_ARG(t);

	mcmgr->t_read = NULL;
	CL_THREAD_READ_ON(MASTER, mcmgr->t_read, misc_client_read, mcmgr, mcmgr->sock);

	log_debug("misc client read...\n");

	n = (int)recvfrom(mcmgr->sock, data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, (socklen_t *)&len_addr);
	if (n <= 0) {
		log_err(true, "read failed\n");
		return -1;
	}

	if (n < sizeof(*hdr)) {
		return -1;
	}

	hdr = (mchdr_t *)data;
	hdr->handle = ntohs(hdr->handle);

	log_debug("misc client read len %d from %u.%u.%u.%u port %u seq #%u\n", n, IP_SHOW(ntohl(addr.sin_addr.s_addr)), ntohs(addr.sin_port), hdr->handle);

	if (stlc_list_empty(&mcmgr->send_list)) {
		log_err(false, "get misc client reply pkt, but no local pkt\n");
		return -1;
	}

	pkt = (mc_pkt_t *)stlc_list_entry(mcmgr->send_list.next, mc_pkt_t, link);

	if (pkt->seq != hdr->handle) {
		log_err(false, "ignore seq %u, local is %u\n", hdr->handle, pkt->seq);
		return -1;
	}

	// 解压
	dec_block((u_int8_t *)BOFP(hdr, mc_plain_size), (n - mc_plain_size), mcmgr->key);
	pad_len = ((u_int8_t *)hdr)[n - 1];
	if (pad_len > AES128_EKY_LEN || pad_len <= 0) {
		log_debug("Drop bad aes pkt\n");
		return 0;
	}
	
	n -= pad_len;

	hdr->timestamp = ntohl(hdr->timestamp);
	hdr->magic = ntohl(hdr->magic);
	hdr->type = ntohs(hdr->type);
	hdr->len = ntohs(hdr->len);

	if (hdr->magic != MC_MAGIC) {
		log_err(false, "after dec, magic invalid 0x%x\n", hdr->magic);
	}

	if (pkt->cb) {
		pkt->cb(pkt->user_handle, hdr->type, hdr->len, hdr->data);
	}

	misc_client_pkt_free(pkt);

	return 0;
}

static RS misc_client_init(void)
{
	MD5_CTX ctx;

	if (mcmgr) {
		return RS_OK;
	}
	
	mcmgr = cl_calloc(1, sizeof(*mcmgr));
	if (!mcmgr) {
		return RS_ERROR;
	}

	STLC_INIT_LIST_HEAD(&mcmgr->send_list);

	if (mcmgr->sock <= 0) {
		mcmgr->sock = create_udp_server(0, 0);
		if (mcmgr->sock == INVALID_SOCKET) {
			goto err_out;
		}
		CL_THREAD_READ_ON(MASTER, mcmgr->t_read, misc_client_read, mcmgr, mcmgr->sock);
	}

	MD5Init(&ctx);
	MD5Update(&ctx, MC_KEY, (u_int32_t)strlen(MC_KEY));
	MD5Final(mcmgr->key, &ctx);

	return RS_OK;

err_out:
	SAFE_FREE(mcmgr);

	return RS_ERROR;
}

static u_int16_t misc_pkt_gen_seq(void)
{
	static u_int16_t seq = 0;

	return ++seq;
}


static int misc_client_send_timer(cl_thread_t *t)
{
	int n = 1000;
	struct sockaddr_in addr;
	mc_pkt_t *pkt;
	mc_mgr_t *mcmgr;
	u_int16_t port;
	static u_int16_t send_ports[3] ={
		51185,
		31185,
    	1185,
	};

	mcmgr = (mc_mgr_t *)CL_THREAD_ARG(t);
	mcmgr->t_send = NULL;

	log_debug("xxxxxxxxx\n");

	if (stlc_list_empty(&mcmgr->send_list)) {
		log_err(false, "Big bug: %s but send list is empty\n", __FUNCTION__);
		return 0;
	}

	pkt = (mc_pkt_t *)stlc_list_entry(mcmgr->send_list.next, mc_pkt_t, link);

	// 发送6次都没回应，放弃
	if (pkt->try_count >= 6) {
		misc_client_pkt_free(pkt);

		// 队列里面还有数据，继续发送
		if (!stlc_list_empty(&mcmgr->send_list)) {
			CL_THREAD_TIMER_ON(MASTER, mcmgr->t_send, misc_client_send_timer, (void *)mcmgr, 0);
		}
		
		return 0;
	}

	port = send_ports[pkt->try_count % 3];

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(pkt->ip);
	addr.sin_port = htons(port);

	pkt->try_count++;
	
	n = (int)sendto(mcmgr->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (n <= 0) {
		log_err(true, "misc client: send ip %u.%u.%u.%u port %u failed, data_len %u seq #%u\n", 
			IP_SHOW(pkt->ip), port, pkt->total, pkt->seq);
	} else {
		log_debug("misc client: send ip %u.%u.%u.%u port %u success, data_len %u seq #%u\n", 
			IP_SHOW(pkt->ip), port, pkt->total, pkt->seq);
	}

	CL_THREAD_TIMER_ON(MASTER, mcmgr->t_send, misc_client_send_timer, (void *)mcmgr, n);

	return 0;
}

static RS misc_client_enc_pkt(mc_pkt_t *pkt)
{
	mchdr_t *hdr = (mchdr_t *)pkt->data;
	int len;
	u_int8_t pad;

	len = pkt->total - mc_plain_size;
	pad = AES128_EKY_LEN - (len & (AES128_EKY_LEN - 1));
	len += pad;
	
	memset(pkt->data + pkt->total, pad, pad);

	enc_block((u_int8_t *)BOFP(pkt->data, mc_plain_size), len, mcmgr->key);
	pkt->total = len + mc_plain_size;
	
	return RS_OK;
}

RS misc_client_do_request(cl_handle_t user_handle, u_int16_t type, u_int16_t len, u_int8_t *data, u_int32_t ip, mc_pkt_callback cb)
{
	mc_pkt_t *pkt = NULL;
	mchdr_t *hdr;
	u_int32_t total = 0;

	log_debug("misc client do request: type %u len %u ip %u.%u.%u.%u\n", type, len, IP_SHOW(ip));

	if (ip == 0) {
		return RS_ERROR;
	}
	
	if (misc_client_init() != RS_OK) {
		return RS_ERROR;
	}

	total = mc_hdr_size + len;
	
	pkt = cl_calloc(1, sizeof(*pkt)+ total + AES128_EKY_LEN);
	if (!pkt) {
		return RS_ERROR;
	}
	pkt->seq = misc_pkt_gen_seq();
	pkt->total = total;
	pkt->cb = cb;
	pkt->ip = ip;
	pkt->user_handle = user_handle;
	
	hdr = (mchdr_t *)pkt->data;
	hdr->ver = 1;
	hdr->handle = ntohs(pkt->seq);
	hdr->magic = ntohl(MC_MAGIC);
	hdr->timestamp = ntohl((u_int32_t)time(NULL));
	hdr->type = ntohs(type);
	hdr->len = ntohs(len);
	memcpy(hdr->data, data, len);

	misc_client_enc_pkt(pkt);
	if (stlc_list_empty(&mcmgr->send_list)) {
		CL_THREAD_TIMER_ON(MASTER, mcmgr->t_send, misc_client_send_timer, (void *)mcmgr, 0);
	}

	stlc_list_add_tail(&pkt->link, &mcmgr->send_list);

	return 0;
}

