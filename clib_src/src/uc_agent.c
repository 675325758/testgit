/**************************************************************************
**  Copyright (c) 2013 GalaxyWind, Ltd.
**
**  Project: 简单的UDP代理服务器
**  File:    uc_agent.c
**  Author:  yuanchao.wong@gmail.com
**  Date:    05/31/2016
**
**  Purpose:
**    简单的UDP代理服务器.
**************************************************************************/


/* Include files. */
#include "uc_agent.h"


/* Macro constant definitions. */
#define ENTER()		log_debug("enter %s line=%d\n", __FUNCTION__, __LINE__)
#define EXIT()		log_debug("exit %s\n", __FUNCTION__)

#if 0
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

/* Type definitions. */
#define UC_SELECTED_ENC UA_ENC_TYPE_AES128

/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
extern RS user_proc_udp(user_t *user, pkt_t *pkt, struct sockaddr_in *peer_addr, bool is_uc_agent);
extern void user_set_status(user_t *user, u_int32_t status);

void uc_hex_dump(char *msg, u_int8_t *data, int len)
{
	char str[512] = {0};
	int n = 0, i;

	for (i = 0; i < len; i++) {
		n += sprintf(&str[n], "0x%02x,", data[i]);
	}

	log_debug("%s:\n %s\n", msg, str);
}

static u_int16_t ua_gen_handle(void)
{
	static u_int16_t handle = 10000;

	return ++handle;
}

static int ua_enc_block(ua_node_t *node, u_int8_t *data, int len)
{
	u_int8_t pad;
	
	u_int8_t key[4][16] = {
		{0x15,0xe2,0x9a,0x6b,0x64,0xb4,0xee,0x1f,0xae,0x9f,0x0d,0x75,0x42,0x35,0x3c,0xe7},
		{0x51,0x89,0x6d,0xf0,0xf9,0x52,0x86,0x57,0xe1,0x1f,0xa0,0xa5,0x02,0xf7,0x2d,0x35},
		{0x31,0x58,0x49,0x41,0x8b,0x5d,0x91,0xf9,0x8c,0x16,0x27,0x64,0xeb,0x3c,0x56,0x26},
		{0x4e,0xdb,0xc5,0xf3,0x13,0xfb,0xe0,0xd2,0x23,0x6a,0xe7,0xf9,0xfe,0xba,0x9b,0x69},
	};

	if (node->enc_type != UA_ENC_TYPE_AES128) {
		return 0;
	}

	pad = (len & (AES128_EKY_LEN - 1)) == 0 ? 0 : (AES128_EKY_LEN - (len & (AES128_EKY_LEN - 1)));
	len += pad;

	if (pad) {
		memset(data + len, pad, pad);
	}

	uc_hex_dump("in enc block key", &key[node->handle % 4][0], 16);

	enc_block(data, len, &key[node->handle % 4][0]);

	return len;
}

static RS ua_dec_block(ua_node_t *node, u_int8_t *enc_data, int *len)
{
//	u_int8_t pad_len;

	u_int8_t key[4][16] = {
			{0x15,0xe2,0x9a,0x6b,0x64,0xb4,0xee,0x1f,0xae,0x9f,0x0d,0x75,0x42,0x35,0x3c,0xe7},
			{0x51,0x89,0x6d,0xf0,0xf9,0x52,0x86,0x57,0xe1,0x1f,0xa0,0xa5,0x02,0xf7,0x2d,0x35},
			{0x31,0x58,0x49,0x41,0x8b,0x5d,0x91,0xf9,0x8c,0x16,0x27,0x64,0xeb,0x3c,0x56,0x26},
			{0x4e,0xdb,0xc5,0xf3,0x13,0xfb,0xe0,0xd2,0x23,0x6a,0xe7,0xf9,0xfe,0xba,0x9b,0x69},
		};

	if (node->enc_type != UA_ENC_TYPE_AES128) {
		return RS_ERROR;
	}

	if ((*len % AES128_EKY_LEN) != 0) {
		return RS_ERROR;
	}

	dec_block(enc_data, *len, &key[node->handle % 4][0]);

#if 0
	pad_len = enc_data[*len - 1];

	if (pad_len > AES128_EKY_LEN) {
		return 0;
	}

	*len -= pad_len;
#endif
	
	return RS_OK;
}

/**
	IP v4的。。
*/
void ua_gen_addr(struct sockaddr *addr, u_int32_t ip, u_int16_t port)
{
	struct sockaddr_in *a = (struct sockaddr_in *)(addr);
	
	if (!addr) {
		return;
	}

	a->sin_family = AF_INET;
	a->sin_addr.s_addr = ntohl(ip);
	a->sin_port = ntohs(port);
}

void ua_addr_get_ip_port(struct sockaddr *addr, u_int32_t *ip, u_int16_t *port)
{
	struct sockaddr_in *a = (struct sockaddr_in *)(addr);
	
	if (!addr) {
		return;
	}

	a->sin_family = AF_INET;
	*ip = ntohl(a->sin_addr.s_addr);
	*port = ntohs(a->sin_port);
}

static RS ua_set_addr_port(struct sockaddr *addr, u_int16_t port)
{
	struct sockaddr_in *paddr;
	
	if (addr->sa_family != AF_INET) {
		return RS_ERROR;
	}

	paddr = (struct sockaddr_in *)addr;
	paddr->sin_port = ntohs(port);

	return RS_OK;
}

static void ua_node_free(ua_node_t *node)
{
	CL_THREAD_OFF(node->t_timer);

	memset(node, 0x00, sizeof(*node));
}

static ua_node_t *ua_node_get_by_handle(ua_mgr_t *uamgr, u_int16_t handle)
{
	int i;

	for (i = 0; i < MAX_UA_NODE_NUM; i++) {

		if (uamgr->node[i].handle == handle) {
			return &uamgr->node[i];
		}
	}

	return NULL;
}
static ua_node_t *ua_node_get_by_type(ua_mgr_t *uamgr, u_int8_t type)
{
	int i;

	for (i = 0; i < MAX_UA_NODE_NUM; i++) {
		if (uamgr->node[i].proxy_type == type) {
			return &uamgr->node[i];
		}
	}

	return NULL;
}

static void ua_addr_dump(char *msg, struct sockaddr addr)
{
//	char ipstr[64];
	struct sockaddr_in *a;
	
	if (addr.sa_family != AF_INET) {
		//log_err(false, "only support ipv4 yet, but family is %u\n", addr.sa_family);
		return;
	}

	a = (struct sockaddr_in *)&addr;

	log_debug("%s: %u.%u.%u.%u (%u)\n", msg, IP_SHOW(ntohl(a->sin_addr.s_addr)), ntohs(a->sin_port));
}

/*
static void ua_node_dump(ua_node_t *node)
{
	log_debug("handle %u, type %u stat %u\n", node->handle, node->proxy_type, node->stat);

	ua_addr_dump("agent addr", node->agent_addr);
}
*/

/**
	非透明转发通道
*/
static int ua_agent_forward_read(cl_thread_t *t)
{
	pkt_t *pkt;
	user_t *user;
	u_int8_t data[1024] = {0};
	uc_ip_hdr_t *uihdr = (uc_ip_hdr_t *)data;
	net_header_t *hdr;
	ua_mgr_t *uamgr = CL_THREAD_ARG(t);
	int total;
	struct sockaddr addr;
	struct sockaddr_in *in_addr = (struct sockaddr_in*)&addr;
	int len_addr = sizeof(addr);

	uamgr->t_read = NULL;
	CL_THREAD_READ_ON(&cl_priv->master, uamgr->t_read, ua_agent_forward_read, uamgr, uamgr->sock);

	user = uamgr->user;

	if (!user) {
		log_err(false, "have no user\n");
		return 0;
	}

	log_debug("user %llu, now stat %u\n", user->sn, user->status);

	pkt = user->udp_buf;

	total = (int)recvfrom(uamgr->sock, data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, (socklen_t *)&len_addr);
	if (total <= 0) {
		log_err(true, "read failed\n");
		return -1;
	}

	ua_addr_dump("read udp from forward channle:", addr);

	if (total <= sizeof(*uihdr)) {
		log_debug("forward total len %u invalid\n", total);
		return 0;
	}

	in_addr->sin_port = uihdr->dest_port;
	in_addr->sin_addr.s_addr = uihdr->dest_ip;

	uihdr->dest_ip = ntohl(uihdr->dest_ip);
	uihdr->dest_port = ntohs(uihdr->dest_port);

	log_debug("proxy_type %u, real ip %u.%u.%u.%u %u total %d\n", uihdr->proxy_type, IP_SHOW(uihdr->dest_ip), uihdr->dest_port, total);

	pkt->total = total - sizeof(*uihdr);
	memcpy(pkt->data, uihdr->payload, pkt->total);

	hdr_order(pkt);
	hdr = (net_header_t *)pkt->data;
	if (hdr->ver >= 2) {
		net_header_v2_t *hdr_v2;
		if (net_hdr_len(hdr) < net_header_v2_size) {
			log_err(false, "Ignore bad udp v2 packet: len=%d\n", pkt->total);
			return 0;
		}
		hdr_v2 = (net_header_v2_t *)hdr;
		hdr_v2->sn = ntoh_ll(hdr_v2->sn);
	}	

	if( pkt->total < (int)(hdr->param_len + net_header_real_size(hdr))){
		log_err(false, "Ignore too short udp packet: %d  < %d\n", 
			pkt->total, (hdr->param_len + net_header_real_size(hdr)));
		return 0;
	}

	cl_lock(&cl_priv->mutex);
	user_proc_udp(user, pkt, in_addr, true);
	cl_unlock(&cl_priv->mutex);
		
	return 0;
}


/**
	申请代理通道
*/
static int ua_agent_read(cl_thread_t *t)
{
	u_int8_t data[1024] = {0};
	uc_request_hdr_t *hdr;
	uc_request_reply_t *reply;
	ua_node_t *node = NULL;
	ua_mgr_t *uamgr = CL_THREAD_ARG(t);
	user_t *user = uamgr->user;
	int total, len = 0;
	struct sockaddr addr;
	struct sockaddr_in *in_addr;
	int len_addr = sizeof(addr);

	uamgr->t_agent_read = NULL;
	CL_THREAD_READ_ON(&cl_priv->master, uamgr->t_agent_read, ua_agent_read, uamgr, uamgr->agent_sock);

	log_debug("ua agent read uamgr %p\n", uamgr);

	// 这里过来的数据肯定是带头部的	
	total = (int)recvfrom(uamgr->agent_sock, data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, (socklen_t *)&len_addr);
	if (total <= 0) {
		log_err(true, "read failed\n");
		return -1;
	}

	ua_addr_dump("read udp from regiest channle\n", addr);

	hdr = (uc_request_hdr_t *)data;
	if (total < sizeof(*hdr)) {
		return -1;
	}

	hdr->random1 = ntohs(hdr->random1);

	if ((node = ua_node_get_by_handle(uamgr, hdr->random1)) == NULL) {
		log_err(false, "can not get node handle %u\n", hdr->random1);
		return 0;
	}

	if (node->stat != UA_NODE_STAT_REGISTING) {
		log_err(false, "handle %u's node not in registing stat, but in %u\n", hdr->random1, node->stat);
		return 0;
	}

	if (hdr->enc_type != 0) {
		len = total - sizeof(*hdr);
		
		if (ua_dec_block(node, hdr->param, &len) != RS_OK) {
			log_err(false, "dec failed\n");
			return -1;
		}
	}

	if (len < sizeof(*reply)) {
		log_err(false, "invalid len %u\n", len);
		return -1;
	}

	reply = (uc_request_reply_t*)hdr->param;

	reply->random2 = ntohs(reply->random2);
	reply->timestamp = ntohl(reply->timestamp);
	reply->proxy_ip = ntohl(reply->proxy_ip);
	reply->proxy_port = ntohs(reply->proxy_port);
	
	
	log_debug("get uc agent reply, enc_type %u cmd %u proxy_type %u proxy_ip %u.%u.%u.%u proxy_port %u\n", 
		hdr->enc_type, reply->cmd, reply->proxy_type, IP_SHOW(reply->proxy_ip), reply->proxy_port);

	if (reply->random2 != hdr->random1) {
		log_err(false, "random not same %u - %u\n", reply->random2, hdr->random1);
		return 0;
	}

	// 申请代理失败
	if (reply->proxy_port == 0) {
		ua_node_free(node);
		log_debug("proxy port zero\n");
		return 0;
	}

	in_addr = (struct sockaddr_in*)&node->agent_addr;
	in_addr->sin_family = AF_INET;
	in_addr->sin_addr.s_addr = ntohl(reply->proxy_ip);
	in_addr->sin_port = ntohs(reply->proxy_port);

	log_debug("agent create ok\n");
	ua_addr_dump("dst addr", node->dst_addr);

	log_debug("node %u set to estb\n", node->handle);
	node->stat = UA_NODE_STAT_ESTB;
	CL_THREAD_TIMER_OFF(node->t_timer);

	// 如果是透明转发，需要通知设备去连接了
	if (reply->proxy_type == UA_PROXY_TYPE_RANSPAREND) {
		log_debug("have get agent for uc dev proxy\n");
		nd_login_debug(NULL, "%llu have get agent for uc dev proxy agent ip %u.%u.%u.%u : %u\n", 
			user->sn, IP_SHOW(reply->proxy_ip), reply->proxy_port);
		
		user->devserver_ip = reply->proxy_ip;
		user->devserver_port = reply->proxy_port;
		
		CL_THREAD_TIMER_OFF(user->t_disp_die_timer);
		user_set_status(user, CS_DEV_CONNECTING);
	}

	return 0;
}

static RS ua_node_request_agent(ua_node_t *node)
{
	if (++node->request_count > 3) {
		node->stat = UA_NODE_STAT_IDLE;
		log_err(false, "request count > 3, goto err\n");
		return RS_ERROR;
	}

	// 组请求报文，发送
//	ua_regist_agent(ua_mgr_t * uamgr,ua_node_t * node)
	
	return RS_OK;
}

static int ua_node_timer(cl_thread_t *t)
{
	ua_node_t *node = (ua_node_t *)CL_THREAD_ARG(t);

	node->t_timer = NULL;

	switch (node->stat) {
		case UA_NODE_STAT_REGISTING:
			ua_node_request_agent(node);
			break;		
	}

	
	CL_THREAD_TIMER_ON(&cl_priv->master, node->t_timer, ua_node_timer, node, TIME_N_SECOND(1));

	return 0;
}

static ua_node_t *ua_node_new(ua_mgr_t *uamgr, struct sockaddr dst_addr, u_int8_t type)
{
	int i;
	ua_node_t *node = NULL;

	for (i = 0; i < MAX_UA_NODE_NUM; i++) {
		if (uamgr->node[i].stat == UA_NODE_STAT_IDLE) {
			node = (ua_node_t *)&uamgr->node[i];
			break;
		}
	}

	if (!node) {
		return NULL;
	}	
	
	memcpy(&node->dst_addr, &dst_addr, sizeof(struct sockaddr));
	memcpy(&node->master_addr, &uamgr->master_addr, sizeof(struct sockaddr));

	node->stat = UA_NODE_STAT_REGISTING;
	node->handle = ua_gen_handle();
	node->proxy_type = type;
	node->enc_type = UC_SELECTED_ENC;

	CL_THREAD_TIMER_ON(&cl_priv->master, node->t_timer, ua_node_timer, node, TIME_N_MSECOND(2));

	return node;
}

static RS ua_regist_agent(ua_mgr_t *uamgr, ua_node_t *node)
{
	u_int8_t buf[256] = {0};
	uc_request_hdr_t *hd = (uc_request_hdr_t*)buf;
	uc_request_t *request = (uc_request_t *)hd->param;
	u_int32_t ip = 0, dest_ip = 0;
	u_int16_t port = 0, dest_port = 0;
	int n = 0, len = sizeof(*request);

	if (uamgr == NULL) {
		return RS_ERROR;
	}

	node->stat = UA_NODE_STAT_REGISTING;

	ua_addr_get_ip_port(&node->master_addr, &ip, &port);
	ua_addr_get_ip_port(&node->dst_addr, &dest_ip, &dest_port);

	hd->ver = 1;
	hd->af = AF_IPV4;
	hd->enc_type = node->enc_type;
	hd->random1 = ntohs(node->handle);

	
	request->random2 = hd->random1;
	request->ip = ntohl(dest_ip);
	request->dest_port = ntohs(dest_port);

	request->cmd = UA_CMD_REQUEST;
	request->proxy_type = node->proxy_type;
	request->timestamp = (u_int32_t)time(NULL);


	// 这里可能需要加密
	if (hd->enc_type) {
		len = ua_enc_block(node, (u_int8_t*)request, len);
		if (len == 0) {
			log_err(false, "enc block failed\n");
			return RS_ERROR;
		}
	}

	n = (int)sendto(uamgr->agent_sock, buf, sizeof(*hd) + len, 0, &node->master_addr, (socklen_t)sizeof(node->master_addr));
	if (n <= 0) {
		log_err(true, "send failed sock %d\n", uamgr->agent_sock);
		return RS_ERROR;
	}

	log_debug("send request to %u.%u.%u.%u - %u ok for regiest proxxy type %u dst %u.%u.%u.%u (%u)\n", IP_SHOW(ip), port, node->proxy_type, IP_SHOW(dest_ip), dest_port);

	return RS_OK;
}

/**
	指定IP和端口的转发
*/
RS ua_trans_send(ua_mgr_t *uamgr, u_int32_t ip, u_int16_t port, u_int8_t *data, int len)
{
	u_int8_t buf[2096] = {0};
	uc_ip_hdr_t *hd = (uc_ip_hdr_t *)buf;
	ua_node_t *node;
	int n;
	user_t *user;
	u_int32_t agent_addr = 0;
	u_int16_t agent_port = 0;

	if (!uamgr) {
		return RS_ERROR;
	}

	user = uamgr->user;

	node = ua_node_get_by_type(uamgr, UA_PROXY_TYPE_IP_IN_IP);
	if (!node) {
		//log_err(false, "not found ip in ip agent\n");
		return RS_ERROR;
	}

	if (node->stat != UA_NODE_STAT_ESTB) {
		log_err(false, "agent stat not estb\n");
		return RS_ERROR;
	}

	ua_addr_get_ip_port(&node->agent_addr, &agent_addr, &agent_port);

	hd->af = AF_IPV4;
	hd->ver = 1;

	hd->dest_ip = ntohl(ip);
	hd->dest_port = ntohs(port);
	hd->proxy_type = UA_PROXY_TYPE_IP_IN_IP;

	memcpy(hd->payload, data, len);

	n = (int)sendto(uamgr->sock, buf, sizeof(*hd) + len, 0, &node->agent_addr, (socklen_t)sizeof(node->agent_addr));
	if (n <= 0) {
		log_err(true, "send failed\n");
		return RS_ERROR;
	}

	log_debug("'%s' socket %d send dispacher request to %u.%u.%u.%u port %d, ret=%d by agent %u.%u.%u.%u (%u)\n",
                      user->name, uamgr->sock, IP_SHOW(ip), port, n, IP_SHOW(agent_addr), agent_port);
	
	return RS_OK;
}


/**
	申请一个透明
*/
void ua_request_transparent_agent(ua_mgr_t *uamgr, u_int32_t ip, u_int16_t port)
{
	struct sockaddr addr;
	ua_node_t *node = NULL;

	
	if (uamgr == NULL) {
		return;
	}

	memset(&addr, 0x00, sizeof(addr));

	log_debug("request transparent agent for devserver ip %u.%u.%u.%u port %u\n", IP_SHOW(ip), port);

	ua_gen_addr(&addr, ip, port);

	node = ua_node_get_by_type(uamgr, UA_PROXY_TYPE_RANSPAREND);
	if (!node) {
		node = ua_node_new(uamgr, addr, UA_PROXY_TYPE_RANSPAREND);
		if (!node) {
			log_err(false, "calloc node failed\n");
			return;
		}
	}

	if (node->stat == UA_NODE_STAT_ESTB) {
		log_debug("has estb ip in ip node\n");
		return;
	}

	//ua_request_agent(uamgr, addr, UA_PROXY_TYPE_RANSPAREND);
	ua_regist_agent(uamgr, node);
}

/**
	申请一个非透明转发
*/
void ua_request_ipinip_agent(ua_mgr_t *uamgr)
{
	struct sockaddr addr;
	ua_node_t *node = NULL;

	if (uamgr == NULL) {
		return;
	}

	memset(&addr, 0x00, sizeof(addr));

	ENTER();

	node = ua_node_get_by_type(uamgr, UA_PROXY_TYPE_IP_IN_IP);
	if (!node) {
		node = ua_node_new(uamgr, addr, UA_PROXY_TYPE_IP_IN_IP);
		if (!node) {
			log_err(false, "calloc node failed\n");
			return;
		}
	}

	if (node->stat == UA_NODE_STAT_ESTB) {
		log_debug("has estb ip in ip node\n");
		return;
	}

	//ua_request_agent(uamgr, addr, UA_PROXY_TYPE_IP_IN_IP);
	ua_regist_agent(uamgr, node);
}

void ua_create(ua_mgr_t **ppuamgr, void *user)
{	
	ua_mgr_t *uamgr = NULL;

	if (*ppuamgr != NULL) {
		log_debug("uc agent had inited\n");
		return;
	}

	if ((uamgr = cl_calloc(1, sizeof(*uamgr))) == NULL) {
		return;
	}
	
	uamgr->user = user;
	
	if (uamgr->agent_sock <= 0) {
		log_debug("create agent sock\n");
		uamgr->agent_sock = create_udp_server(0, 0);
		if (uamgr->agent_sock == INVALID_SOCKET) {
			goto err_out;
		}
	}

	CL_THREAD_OFF(uamgr->t_agent_read);
	CL_THREAD_READ_ON(&cl_priv->master, uamgr->t_agent_read, ua_agent_read, uamgr, uamgr->agent_sock);

	if (uamgr->sock <= 0) {
		log_debug("create regiest sock\n");
		uamgr->sock = create_udp_server(0, 0);
		if (uamgr->sock == INVALID_SOCKET) {
			goto err_out;;
		}
	}

	CL_THREAD_OFF(uamgr->t_read);
	CL_THREAD_READ_ON(&cl_priv->master, uamgr->t_read, ua_agent_forward_read, uamgr, uamgr->sock);

	log_debug("ua init done agent_sock %d sock %d\n", uamgr->agent_sock, uamgr->sock);

	*ppuamgr = uamgr;

	return;

err_out:
	if (uamgr) {
		cl_free(uamgr);
	}	
}

void ua_init(ua_mgr_t *uamgr)
{
	int i;

	if (!uamgr) {
		return;
	}

	for (i = 0; i < MAX_UA_NODE_NUM; i++) {
		ua_node_free(&uamgr->node[i]);
	}
	
	memset(uamgr->node, 0x00, sizeof(uamgr->node));
}

/**
	设置申请代理的服务器地址
*/
void ua_set_master(ua_mgr_t *uamgr, struct sockaddr *addr)
{
	u_int32_t ip = 0;
	u_int16_t port = 0;

	if (!uamgr) {
		return;
	}

	memcpy(&uamgr->master_addr, addr, sizeof(*addr));

	if (ua_set_addr_port(&uamgr->master_addr, UA_MASTER_PORT) != RS_OK) {
		log_err(false, "set addr port failed\n");
		return;
	}

	ua_addr_get_ip_port(addr, &ip, &port);
	
	log_debug("set addr %u.%u.%u.%u port %u as master agent addr\n", IP_SHOW(ip), port);
}


