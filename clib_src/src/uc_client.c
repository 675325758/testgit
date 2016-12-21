#include "client_lib.h"
#include "cl_priv.h"
#include "cl_sys.h"
#include "md5.h"
#include "cl_thread.h"
#include "aes.h"
#include "tea.h"

#include "udp_ctrl.h"
#include "uc_client.h"
#include "smart_appliance_priv.h"
#include "lan_dev_probe_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "phone_user.h"
#include "app_pkt_priv.h"

#define DEBUG_ON
#define UC_CLIENT_DEBUG

/********************打印调试开关********************打印调试开关***************
*************打印调试开关********************打印调试开关*******************/
#if 0
//文件控制宏,属于app调试用的一些稍微重要的问题件
#define DEBUG_IMPORTANT_FILE
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


#define UC_MAYBE_MUL_PORT 100

#define UC_TRANS_PORT0	51192
#define UC_TRANS_PORT1	42612
#define UC_TRANS_PORT2	1192


extern ucc_proc_t ucc_proc[UCCS_MAX];

static int ucc_keeplive_timer(cl_thread_t *t);
static void ucc_check_need_switch_port(ucc_session_t *s);
extern bool rf_slave_init(user_t *user);
void dev_ver_reset_cal(user_t *user);
void la_do_dev_estab_proc(user_t *puser);
void la_do_dev_error_proc(user_t *puser);
extern void do_la_back_passwd(user_t *puser);
extern bool la_is_valid();
extern void cl_debug_buff_add(void *s, char *buf, int len);

static char cmap[] = "7ijg0tBCzhsxR6HbpqaIdFLEyfTm1knWVcJv";

//二维码添加设备相关函数

static void json_enc(char *src, char *dst)
{
    char c;
    
    // skip   "v":"1",
    while (*src && *src != ',')
        *dst++ = *src++;
    *dst++ = *src;
    if (*src == '\0')
        return;
    src++;
    
    while ((c = *src)) {
        if ('0' <= c && c <= '9') {
            *dst = cmap[c - '0'];
        } else if ('a' <= c && c <= 'z') {
            *dst = cmap[c - 'a' + 10];
        } else {
            *dst = c;
        }
        
        src++;
        dst++;
    }
}

void json_dec(char *src, char *dst)
{
    int i;
    char c;
    
    // skip   "v":"1",
    while (*src && *src != ',')
        *dst++ = *src++;
    *dst++ = *src;
    if (*src == '\0')
        return;
    src++;
    
    while ((c = *src)) {
        for (i = 0; cmap[i] && cmap[i] != c; i++) {
        }
        if (cmap[i]) {
            if (i < 10) {
                *dst = '0' + i;
            } else {
                *dst = 'a' + i - 10;
            }
        } else {
            *dst = c;
        }
        
        src++;
        dst++;
    }
}

void uc_u64_2_char(u_int8_t *pshare, u_int64_t *pkey)
{
	int i;

	for(i = 0; i < 8; i++) {
		pshare[i] = (u_int8_t)((*pkey>>(i*8))&0xff);
	}
}

void uc_share_2_u64(u_int8_t *pshare, u_int64_t *pkey)
{
	int i;
	u_int64_t value = 0;
	u_int64_t tmp = 0;

	for(i = 0; i < 8; i++) {
		tmp = (u_int64_t)pshare[i];
		value += tmp <<(i*8);
	}

	*pkey = value;
}

static void ucc_store_share_code(ucc_session_t* s)
{
    char fn[256];
    FILE *fp;
    
    if (cl_priv->dir == NULL) {
        return;
    }
    
    sprintf(fn, "%s/%012"PRIu64"", cl_priv->dir, s->user->sn);
    MKDIR(fn, 0777);
    
    sprintf(fn, "%s/%012"PRIu64"/s_code.data", cl_priv->dir, s->user->sn);
    if ((fp = fopen(fn, "w")) == NULL) {
        log_err(false, "open file %s failed\n", fn);
        return ;
    }
    
    fwrite(s->share_key, V2_SHARE_KEY_LEN, 1, fp);
    
    fclose(fp);
}

void ucc_del_share_code(ucc_session_t *s)
{
    char fn[256];
    FILE *fp;
    
    if (cl_priv->dir == NULL) {
        return;
    }
    
    sprintf(fn, "%s/%012"PRIu64"/s_code.data", cl_priv->dir, s->user->sn);
    if ((fp = fopen(fn, "w")) == NULL) {
        log_err(false, "open file %s failed\n", fn);
        return ;
    }
    
    fclose(fp);
    
}

static void ucc_read_share_code(ucc_session_t *s)
{
    char fn[256];
    FILE *fp;
    int len;
    
    if (cl_priv->dir == NULL) {
        return;
    }
    
    sprintf(fn, "%s/%012"PRIu64"/s_code.data", cl_priv->dir, s->user->sn);
    if ((fp = fopen(fn, "rb")) == NULL) {
        log_err(false, "open file %s failed\n", fn);
        return ;
    }
    
    len = (int)fread(s->share_key, sizeof(s->share_key), 1, fp);
    if (len <= 0) {
        memset(s->share_key, 0, sizeof(s->share_key));
        goto done;
    }
    
    s->has_share_key = true;
    
done:
    fclose(fp);
    
}

// 二维码登录
static void ucc_proc_qr_code(user_t* user,ucc_session_t* s)
{
	u_int64_t qr_code64 = 0;
	
    s->is_qr_login = true;
	s->v2_client_sid = 0;
	s->v2_device_sid = 0;
	s->v2_login_num = 3;

    if (user->qr_code) {
		sscanf(user->qr_code, "%"PRIu64"", &qr_code64);
		s->qr_code64 = qr_code64;
		//log_debug("ucc_test %"PRIu64"", s->qr_code64);
    }
}

/*****************************************
	报文处理相关的函数
 *****************************************/
//最多ip上下波动10个
#define 	TRANS_IP_RANGE_MAX	(10)

static void ucc_trans_mul_port_send(ucc_session_t *s, pkt_t *pkt, u_int32_t ip)
{
	int i, n;
	struct sockaddr_in addr;
	u_int16_t trans_port[3] = {UC_TRANS_PORT0, UC_TRANS_PORT1, UC_TRANS_PORT2};

	for(i = 0; i < 3; i++) {
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(ip);
		addr.sin_port = htons(trans_port[i]);

		n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
		log_info("%s send to ip=%u.%u.%u.%u port=%u n=%d\n", 
			__FUNCTION__, IP_SHOW(ip), trans_port[i], n);
	}
}

static void ucc_trans_send(ucc_session_t *s, pkt_t *pkt)
{
	u_int8_t i,j;
	u_int32_t ip;
	u_int8_t num;

	for(i = 0; i < cl_priv->trans_ip_num; i++) {
		ip = cl_priv->trans_ip[i];
		num = (u_int8_t)(ip&0xff);
		ip &= 0xffffff00;
		//向下
		for(j = 0; j < TRANS_IP_RANGE_MAX; j++) {
			ucc_trans_mul_port_send(s, pkt, ip|((num+j)&0xff));
		}
		//向上
		for(j = 0; j < TRANS_IP_RANGE_MAX; j++) {
			ucc_trans_mul_port_send(s, pkt, ip|((num-j-1)&0xff));
		}
	}
}

static int ucc_send_timer(cl_thread_t *t)
{
	int n;
	ucc_session_t *s;
	struct sockaddr_in addr;
	pkt_t *pkt;

	s = (ucc_session_t *)CL_THREAD_ARG(t);
	s->t_send = NULL;
	if (stlc_list_empty(&s->send_list)) {
		log_err(false, "Big bug: %s but send list is empty\n", __FUNCTION__);
		return 0;
	}

	if (s->send_retry >= 3)
		n = 2;
	else
		n = s->send_retry;
	n = s->time_param_cur->retry_100ms[n] * 100;

	if(s->send_retry > MAX_SEND_RETRY_NUM){
        log_debug("%s send_retry over, to IDLE direct ip %u.%u.%u.%u\n",s->name,IP_SHOW(s->user->direct_ip));
        //稳当处理
        if (!s->user->last_err) {
            s->user->last_err = ULGE_NETWORK_ERROR;
            event_push(s->user->callback, UE_LOGIN_ERROR, s->user->handle, s->user->callback_handle);
        }
        
		ucc_set_status(s, UCCS_IDLE);
		return 0;
	}
	
	CL_THREAD_TIMER_ON(MASTER, s->t_send, ucc_send_timer, (void *)s, n);
	s->send_retry++;
    s->send_pkts++;

	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);

	//这里处理一下，是否中转登陆
	if ((pkt->action == UC_MAYBE_MUL_PORT) && 
		s->user && s->user->need_trans_login) {
		ucc_trans_send(s, pkt);
		return 0;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(s->ip);
	addr.sin_port = htons(s->port);
	n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (n <= 0) {
		log_err(true, "%s send pkt failed: ip=%u.%u.%u.%u, port=%d, want=%d, send=%d errno[%d]\n",
			s->name, IP_SHOW(s->ip), s->port, pkt->total, n,errno);

		nd_login_info(s, "%s send pkt failed: ip=%u.%u.%u.%u, port=%d, want=%d, send=%d errno[%d]\n",
			s->name, IP_SHOW(s->ip), s->port, pkt->total, n,errno);
	} else {
#ifdef	DBG_UDP_CTRL_PKT	
		ucph_t *hdr;
        ucph_v2_t* v2_hdr;
		char upg[128] = "";
        
		if(pkt->up_total){
			sprintf(upg, " upgrade %hu / %hu ", pkt->up_current, pkt->up_total);
		}

		hdr = (ucph_t *)pkt->data;
        if (hdr->ver <= PROTO_VER1) {
            log_debug("%s send ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, action=%u, req=%d, enc=%d, csid=%08x, dsid=%08x, rid=%u %s\n",
                      s->name, IP_SHOW(s->ip), s->port, n,
                      pkt->cmd, *((u_int8_t *)(hdr + 1)), hdr->request, hdr->encrypt,
                      ntohl(hdr->client_sid), ntohl(hdr->device_sid), hdr->request_id, upg);

			if (s->status != UCCS_ESTABLISH) {
				nd_login_debug(s, "%s send ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, action=%u, req=%d, enc=%d, csid=%08x, dsid=%08x, rid=%u %s\n",
	                      s->name, IP_SHOW(s->ip), s->port, n,
	                      pkt->cmd, *((u_int8_t *)(hdr + 1)), hdr->request, hdr->encrypt,
	                      ntohl(hdr->client_sid), ntohl(hdr->device_sid), hdr->request_id, upg);
			}
        }else if(hdr->ver == PROTO_VER2){
            v2_hdr = (ucph_v2_t*)pkt->data;
            log_debug("%s send v2 pkt ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, action=%u, req=%d, enc=%d, csid=%08x, dsid=%08x, rid=%u hash=%08x %s\n",
                      s->name, IP_SHOW(s->ip), s->port, n,
                      pkt->cmd, *((u_int8_t *)(hdr + 1)), hdr->request, hdr->encrypt,
                      ntohl(hdr->client_sid), ntohl(hdr->device_sid), ntohl(v2_hdr->u32_req_id),v2_hdr->pkt_hash, upg);

			if (s->status != UCCS_ESTABLISH) {
				nd_login_debug(s, "%s send v2 pkt ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, action=%u, req=%d, enc=%d, csid=%08x, dsid=%08x, rid=%u hash=%08x %s\n",
										  s->name, IP_SHOW(s->ip), s->port, n,
										  pkt->cmd, *((u_int8_t *)(hdr + 1)), hdr->request, hdr->encrypt,
										  ntohl(hdr->client_sid), ntohl(hdr->device_sid), ntohl(v2_hdr->u32_req_id),v2_hdr->pkt_hash, upg);
			}
		}
		
//        mem_dump("pkt content", pkt->data, pkt->total);
#endif
	}

	return 0;
}

static RS _ucc_enc_aes128(ucc_session_t *s, pkt_t *pkt)
{
	ucph_t *hdr = (ucph_t *)pkt->data;
	int len;
	u_int8_t pad;

	len = pkt->total - ucph_hdr_plain_size;
	pad = AES128_EKY_LEN - (len & (AES128_EKY_LEN - 1));
	len += pad;

	memset(pkt->data + pkt->total, pad, pad);
	enc_block((u_int8_t *)BOFP(pkt->data, ucph_hdr_plain_size), len, s->key);
	pkt->total = len + ucph_hdr_plain_size;
	hdr->encrypt = 1;
	return RS_OK;
}

static RS _ucc_dec_aes128(ucc_session_t *s, pkt_t *pkt)
{
	int pad_len;

	if (((pkt->total - ucph_hdr_plain_size)%AES128_EKY_LEN) != 0) {
		log_debug("Drop bad packet: n=%d, but encrypt=1\n", pkt->total);
		return RS_ERROR;
	}
	dec_block((u_int8_t *)BOFP(pkt->data, ucph_hdr_plain_size), (pkt->total - ucph_hdr_plain_size), s->key);
	pad_len = ((u_int8_t *)pkt->data)[pkt->total - 1];
	if (pad_len > AES128_EKY_LEN || pad_len <= 0) {
	log_debug("Drop bad packet: encrypt pad=%d\n", pad_len);
		return RS_ERROR;
	}
	pkt->total -= pad_len;
	return RS_OK;
}
static RS _ucc_enc_xxtea(ucc_session_t *s, pkt_t *pkt)
{
	ucph_t *hdr = (ucph_t *)pkt->data;
	int len;
	u_int8_t pad;
	
	len = pkt->total - ucph_hdr_plain_size;
	pad = XXTEA_BLOCK_SIZE - (len & (XXTEA_BLOCK_SIZE - 1));
	len += pad;

	memset(pkt->data + pkt->total, pad, pad);
	tea_enc_block((u_int8_t *)BOFP(pkt->data, ucph_hdr_plain_size), len, s->key);
	pkt->total = len + ucph_hdr_plain_size;
	hdr->encrypt = 1;
	return RS_OK;
}
static RS _ucc_dec_xxtea(ucc_session_t *s, pkt_t *pkt)
{
	int pad_len;

	if (((pkt->total - ucph_hdr_plain_size)%XXTEA_BLOCK_SIZE) != 0) {
		log_debug("Drop bad packet: n=%d, but encrypt=1\n", pkt->total);
		return RS_ERROR;
	}
	tea_dec_block((u_int8_t *)BOFP(pkt->data, ucph_hdr_plain_size), (pkt->total - ucph_hdr_plain_size), s->key);
	pad_len = ((u_int8_t *)pkt->data)[pkt->total - 1];
	if (pad_len > XXTEA_BLOCK_SIZE || pad_len <= 0) {
	log_debug("Drop bad packet: encrypt pad=%d\n", pad_len);
		return RS_ERROR;
	}
	pkt->total -= pad_len;
	return RS_OK;
}
static RS ucc_enc_pkt(ucc_session_t *s, pkt_t *pkt)
{
	ucph_t *hdr = (ucph_t *)pkt->data;
	
	if (s->select_enc == UC_ENC_AES128) {
		_ucc_enc_aes128(s, pkt);
	} else if(s->select_enc == UC_ENC_XXTEA){
		_ucc_enc_xxtea(s, pkt);
	}else{
		hdr->encrypt = 0;
	}

	return RS_OK;
}

static RS ucc_dec_pkt(ucc_session_t *s, pkt_t *pkt)
{
	if(s->select_enc == UC_ENC_AES128){
		return _ucc_dec_aes128(s, pkt);
	}else if(s->select_enc == UC_ENC_XXTEA){
		return _ucc_dec_xxtea(s, pkt);
	}else if(s->select_enc == UC_ENC_NONE){
		return RS_OK;
	}else{
		return RS_ERROR;
	}
}

static void uc_do_v2_pkt_hash(ucc_session_t *s, pkt_t *pkt)
{
    ucph_v2_t* v2_hdr;
    MD5_CTX ctx;
    u_int8_t result[16] = {0};
    
    v2_hdr = (ucph_v2_t*)&pkt->data[0];
    if(v2_hdr->ver != PROTO_VER2){
        return;
    }

	if (s->select_enc == UC_ENC_AES128) {
		v2_hdr->encrypt = 1;
	} else {
		v2_hdr->encrypt = 0;		
	}
	
    v2_hdr->pkt_hash = 0;
    
    mem_dump("pkt", pkt->data, pkt->total);
    
    mem_dump("uuid", s->uuid, sizeof(s->uuid));
    
    mem_dump("rand1", s->rand1, sizeof(s->rand1));
    
    mem_dump("rand2", s->rand2, sizeof(s->rand2));
    
    mem_dump("share key", s->share_key, V2_SHARE_KEY_LEN);
    
    MD5Init(&ctx);
    MD5Update(&ctx, (u_int8_t *)&pkt->data[0], pkt->total);
    MD5Update(&ctx, (u_int8_t *)&s->uuid[0], (u_int32_t)sizeof(s->uuid));
    MD5Update(&ctx, (u_int8_t *)&s->rand1[0], (u_int32_t)sizeof(s->rand1));
    MD5Update(&ctx, (u_int8_t *)&s->rand2[0], (u_int32_t)sizeof(s->rand2));
    MD5Update(&ctx, s->share_key, V2_SHARE_KEY_LEN);
    MD5Final(result, &ctx);
    memcpy(&v2_hdr->pkt_hash, &result[12], 4);
    
    mem_dump("all hash  key", result, sizeof(result));
//
    mem_dump("pkt", pkt->data, pkt->total);
}

/*
	发送请求报文，需要定时重发，重发时间递增
*/
RS ucc_request_add(ucc_session_t *s, pkt_t *pkt)
{
	ucp_ctrl_t* uc;
	
	if (s->status == UCCS_ESTABLISH) {
		uc = get_ucp_payload(pkt, ucp_ctrl_t);
		pkt->action = uc->action;
		
        if (s->has_share_key) {
            uc_do_v2_pkt_hash(s,pkt);
        }
		if (ucc_enc_pkt(s, pkt) != RS_OK) {
			pkt_free(pkt);
			return RS_ERROR;
		}
		s->my_request_id++;
	}

	if (stlc_list_empty(&s->send_list)) {
		s->send_retry = 0;
		CL_THREAD_TIMER_ON(MASTER, s->t_send, ucc_send_timer, (void *)s, 0);
	}
	
	stlc_list_add_tail(&pkt->link, &s->send_list);
    

	CL_THREAD_OFF(s->t_keeplive);

	return RS_OK;
}

/*
	发送请求报文，requestid 不需要递增
*/
RS ucc_request_add2(ucc_session_t *s, pkt_t *pkt)
{
	if (s->status == UCCS_ESTABLISH) {
		if (ucc_enc_pkt(s, pkt) != RS_OK) {
			pkt_free(pkt);
			return RS_ERROR;
		}
	}

	if (stlc_list_empty(&s->send_list)) {
		s->send_retry = 0;
		CL_THREAD_TIMER_ON(MASTER, s->t_send, ucc_send_timer, (void *)s, 0);
	}
	
	stlc_list_add_tail(&pkt->link, &s->send_list);

	CL_THREAD_OFF(s->t_keeplive);

	return RS_OK;
}


RS ucc_response_send(ucc_session_t *s, pkt_t *pkt)
{
    if (s->status != UCCS_ESTABLISH){
        pkt_free(pkt);
        return RS_ERROR;
    }
    
    if (s->has_share_key) {
        uc_do_v2_pkt_hash(s,pkt);
    }
    
	if (ucc_enc_pkt(s, pkt) != RS_OK) {
        pkt_free(pkt);
        return RS_ERROR;
    }
    
	uc_send_pkt_raw(s->sock, s->ip, s->port, pkt);
    s->send_pkts++;
    
    pkt_free(pkt);
    
	return RS_OK;
}

/*
	删除头节点，触发下一节点的发送
*/
void ucc_request_del(ucc_session_t *s)
{
	pkt_t *pkt;

	if (stlc_list_empty(&s->send_list))
		return;
	
	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	stlc_list_del(&pkt->link);
	pkt_free(pkt);

	CL_THREAD_OFF(s->t_send);
	
	if (stlc_list_empty(&s->send_list)) {
		if (s->status == UCCS_ESTABLISH) {
			CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, ucc_keeplive_timer, (void *)s, TIME_N_SECOND(s->time_param_cur->keeplive));
		}
		return;
	}

	s->send_retry = 0;
	CL_THREAD_TIMER_ON(MASTER, s->t_send, ucc_send_timer, (void *)s, 0);
}

/*
	释放整个队列的所有报文
*/
void ucc_reset_send(ucc_session_t *s)
{
	pkt_t *pkt, *next;

	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_list, link) {
		stlc_list_del(&pkt->link);
		pkt_free(pkt);
	}
	s->send_retry = 0;
	s->up_total = 0;
	s->last_rid = 0xff;
	s->drop_update_num = 0;
	s->droped_num = 0;
	CL_THREAD_OFF(s->t_send);
}

static int ucc_read(cl_thread_t *t)
{
	ucc_session_t *s;
	struct sockaddr_in addr;
	int n;
	u_int32_t addr_len;
	pkt_t *pkt;
	ucph_t *hdr;
	
	s = (ucc_session_t *)CL_THREAD_ARG(t);
	s->t_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_recv, ucc_read, (void *)s, s->sock);

	addr_len = sizeof(struct sockaddr_in);
	pkt = s->rcv_buf;

	n = pkt->total = (int)recvfrom(s->sock, (char *)pkt->data, MAX_UDP_PKT, 0,
			(struct sockaddr *)&addr, &addr_len);
	if (n < (int)ucph_hdr_size) {
		log_err(true, "%s read udp failed\n", s->name);
		return 0;
	}

    s->recv_pkts++;
	hdr = (ucph_t *)pkt->data;
    s->rcv_hdr = hdr;

	// 1. 解密、检查长度合法性
	if (hdr->encrypt) {
		if(ucc_dec_pkt(s, pkt) != RS_OK)
			return 0;
	}

	uc_hdr_order(hdr);
	
	if (ucph_hdr_len(hdr) < ucph_hdr_size 
		|| pkt->total != hdr->param_len + ucph_hdr_len(hdr))
	{
		log_debug("Drop bad packet: total=%d, ucph_hdr_len=%d, param_len=%d\n", 
			pkt->total, ucph_hdr_len(hdr), hdr->param_len);
		return 0;
	}

	// 2. 检查会话ID
	if (s->client_sid != 0 && hdr->client_sid != s->client_sid) {
		log_debug("Drop bad packet: packet session id=%u, but now is %u\n",
			hdr->client_sid, s->client_sid);
		return 0;
	}

	// update ip address and port
	s->ip = ntohl(addr.sin_addr.s_addr);
	s->port = ntohs(addr.sin_port);

#ifdef	DBG_UDP_CTRL_PKT	
	log_debug("%s recv from %u.%u.%u.%u:%u: version=%u cmd=%u, action=%u, param_len=%u, csid=%08x, dsid=%08x, request id=%u(is %s)\n",
		s->name, IP_SHOW(s->ip), s->port,hdr->ver, hdr->command, *((u_int8_t *)(hdr + 1)), hdr->param_len,
		hdr->client_sid, hdr->device_sid, hdr->request_id, hdr->request ? "request" : "reply");
#endif

	if (s->status != UCCS_ESTABLISH) {
		nd_login_debug(s, "%s recv from %u.%u.%u.%u:%u: version=%u cmd=%u, action=%u, param_len=%u, csid=%08x, dsid=%08x, request id=%u(is %s)\n",
			s->name, IP_SHOW(s->ip), s->port,hdr->ver, hdr->command, *((u_int8_t *)(hdr + 1)), hdr->param_len,
			hdr->client_sid, hdr->device_sid, hdr->request_id, hdr->request ? "request" : "reply");
	}

	// 处理报文
	cl_lock(&cl_priv->mutex);
	ucc_proc[s->status].proc_pkt(s);
	cl_unlock(&cl_priv->mutex);
#ifdef DBG_UDP_CTRL_PKT
	log_debug("The Packet Hand completed!\n");
#endif 
	
	return 0;
}

static void ucc_off_all_timer(ucc_session_t *s)
{
	//CL_THREAD_OFF(s->t_recv);
	CL_THREAD_OFF(s->t_send);
	CL_THREAD_OFF(s->t_keeplive);
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_OFF(s->t_timer);
}

static bool is_reset_pkt(ucph_t *hdr)
{
	uc_keeplive_reply_t *kr;
    
	if (hdr->command != CMD_UDP_KEEPLIVE)
		return false;
	
	kr = get_net_ucp_payload(hdr, uc_keeplive_reply_t);
    
	return (kr->action == UCA_RESET);
}

/***************************
	IDLE
 ***************************/

static int idle_timer(cl_thread_t *t)
{
	ucc_session_t *s = (ucc_session_t *)CL_THREAD_ARG(t);
	
	s->t_timer = NULL;
	ucc_set_status(s, UCCS_AUTH_REQ);

	return 0;
}

static void ucc_idle_into(ucc_session_t *s)
{
	ucc_reset_send(s);

	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;
	s->user->auth_err_num = 0;
	
	ucc_off_all_timer(s);
	
	CL_THREAD_TIMER_ON(MASTER, s->t_timer, idle_timer, (void *)s, TIME_N_SECOND(s->idle_time));
	s->idle_time = DFL_UC_IDLE_TIME;

	nd_login_debug(s, "ucc_idle_into idle_time %u\n", s->idle_time);
}

static void ucc_idle_out(ucc_session_t *s)
{
	// do none
}

static void ucc_idle_proc(ucc_session_t *s)
{
	// do none
}

/***************************
	AUTH REQUEST
 ***************************/


static int auth_req_die(cl_thread_t *t)
{
	ucc_session_t *s = (ucc_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	cl_assert(s->status == UCCS_AUTH_REQ);

	log_info("%s UCCS_AUTH_REQ die\n", s->name);
	nd_login_debug(s, "UCCS_AUTH_REQ die\n");
	//这里处理一下，是否中转登陆失败
	if (s->user && s->user->need_trans_login) {
		s->user->last_err = s->user->back_last_err;
		nd_login_debug(s, "need_trans_login ULGE_NETWORK_ERROR\n");
		user_set_status(s->user, CS_LOGIN_ERR);
		return 0;
	}

    //认证超时，还是从头再来
    if (s->user->direct_ip != 0) {
        ucc_set_status(s, UCCS_IDLE);
		//如果设备掉电这种情况肯定最后是要报离线的，不然局域网重连肯定是可以再次连接成功的
		if (s->user->last_err != ULGE_DEV_OFF_LINE) {
            s->user->last_err = ULGE_DEV_OFF_LINE;
			event_push(s->user->callback, UE_LOGIN_OFFLINE, s->user->handle, s->user->callback_handle);
			event_cancel_merge(s->user->handle);
		}
		nd_login_debug(s, "direct_ip %u.%u.%u.%u, ULGE_DEV_OFF_LINE\n", IP_SHOW(s->user->direct_ip));
        
    }else{
        if (s->user->udp_recv_pkts == 0) {
            //没有收到报文，网络错误
            if (s->user->last_err!= ULGE_NETWORK_ERROR) {
                s->user->last_err = ULGE_NETWORK_ERROR;
                event_push(s->user->callback, UE_LOGIN_ERROR, s->user->handle, s->user->callback_handle);
                event_cancel_merge(s->user->handle);
            }
            s->user->udp_recv_pkts = 0;
            user_set_status(s->user, CS_IDLE);
			
			nd_login_debug(s, "no recv pkt, ULGE_NETWORK_ERROR\n");
        }else{
            //收到有报文，至少分配服务器或设备服务器是通的，离线
            if (s->user->last_err != ULGE_DEV_OFF_LINE) {
                s->user->last_err = ULGE_DEV_OFF_LINE;
                event_push(s->user->callback, UE_LOGIN_OFFLINE, s->user->handle, s->user->callback_handle);
                event_cancel_merge(s->user->handle);
            }
            s->user->udp_recv_pkts = 0;
            ucc_set_status(s, UCCS_IDLE);

			nd_login_debug(s, "has recv pkt, but ULGE_DEV_OFF_LINE\n");
        }
        
    }
	return 0;
}

static void fill_local_time(uc_time_t *uct)
{
	time_t now;
	struct tm *tmp;

	time(&now);
	// 避免多线程同时调用localtime, 加锁保护
	CL_LOCK;
	tmp = gmtime(&now);
	uct->year = tmp->tm_year + LOCALTIME_YEAR_BASE;
	uct->month = tmp->tm_mon + 1;
	uct->day = tmp->tm_mday;
	uct->hour = tmp->tm_hour;
	uct->minute = tmp->tm_min;
	uct->second = tmp->tm_sec;
	uct->timezone = cl_priv->timezone;
	CL_UNLOCK;

	uct->year = htons(uct->year);
}

static void ucc_copy_user_info(ucc_session_t *s)
{
	user_t *user = s->user;
	
	s->sn = user->sn;
	s->ip = user->devserver_ip;
	s->port = user->devserver_port;

	//进入后台

	
	if (s->user->direct_ip != 0) {
		s->time_param_cur = &s->time_param.lan;
	} else {
		if (is_user_in_background_mode(s->user)) {
			s->time_param_cur = &s->time_param.wan_background;
		}else{
			s->time_param_cur = &s->time_param.wan_front;
		}
	}
    
    log_debug("%s ucc_copy_user_info [direct_ip %u.%u.%u.%u] update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
              s->name, IP_SHOW(s->user->direct_ip),
              s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
              s->time_param_cur->keeplive, s->time_param_cur->die);

	nd_login_debug(s, "ucc_copy_user_info [direct_ip %u.%u.%u.%u] update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
              IP_SHOW(s->user->direct_ip),
              s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
              s->time_param_cur->keeplive, s->time_param_cur->die);

}

static void ucc_auth_req_into(ucc_session_t *s)
{
	pkt_t *pkt;
	uc_auth_request_t *ar;
    uc_share_register_t* sr;
	char rand1_str[32] = {0};
	char my_uuid_str[256] = {0};

	nd_login_debug(s, "ucc_auth_req_into... direct_ip %u.%u.%u.%u\n", IP_SHOW(s->user->direct_ip));

	// reset
	ucc_reset_send(s);
	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;

	ucc_copy_user_info(s);
    
    //要重置超时时间
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, auth_req_die, (void *)s, TIME_N_SECOND(s->die_for_auth));
	
	if(s->ip == 0){
		if(s->user->direct_ip != 0 ){
			user_set_direct_login(s->user, s->user->direct_ip);
			return;
		}
		user_set_status(s->user, CS_DISP);
		return;
	}

    ucc_check_need_switch_port(s);

    //二维码登陆，先注册
    if(s->v2_client_sid && s->is_qr_login){
        pkt = uc_pkt_new(s,CMD_SHARE_REGISTER, sizeof(uc_share_register_t),
                         true, false, 0,s->v2_client_sid, s->v2_device_sid, 0);
        sr = get_ucp_payload(pkt, uc_share_register_t);
        // TODO:手机描述
        memcpy(sr->phone_desc, cl_priv->phone_desc, sizeof(sr->phone_desc));
        memcpy(sr->phone_uuid, cl_priv->uuid_bin, sizeof(sr->phone_uuid));
        sr->phone_desc[15] = '\0';
		sr->sc = ntoh_ll(s->qr_code64);
        ucc_request_add(s,pkt);

        return;
    }
	// create new rand
	memset(s->rand2, 0, sizeof(s->rand2));
	// send auth.request
	pkt = uc_pkt_new(s,CMD_UDP_AUTH, sizeof(uc_auth_request_t),
				true, false, 0,	0, 0, 0);
	ar = get_ucp_payload(pkt, uc_auth_request_t);
	ar->action = UCA_REQUEST;
    ar->version = PROTO_VER2;
	ar->auth_flag |= (BIT(0) & 0xf);
	memcpy(ar->rand1, s->rand1, sizeof(ar->rand1));
	ar->sn = ntoh_ll(s->sn);
	memcpy(ar->my_uuid, cl_priv->uuid_bin, sizeof(ar->my_uuid));

	fill_local_time(&ar->time);

    s->recv_pkts = 0;
    s->send_pkts = 0;

	pkt->action = UC_MAYBE_MUL_PORT;
	ucc_request_add(s, pkt);

	fmt_hex(rand1_str, ar->rand1, sizeof(ar->rand1));
	fmt_hex(my_uuid_str, ar->my_uuid, sizeof(ar->my_uuid));
	
	nd_login_debug(s, "auth request into, rand1[%s] uuid[%s]\n", rand1_str, my_uuid_str);
}

static void ucc_auth_req_out(ucc_session_t *s)
{
	ucc_reset_send(s);
}

static void do_auth_redirect(ucc_session_t *s, uc_auth_redirect_t *rd)
{
	
	rd->ip = ntohl(rd->ip);
	rd->port = ntohs(rd->port);
	log_info("%s redirect to ip=%u.%u.%u.%u port=%u\n",
		s->name, IP_SHOW(rd->ip), rd->port);

	nd_login_debug(s, "redirect to ip=%u.%u.%u.%u port=%u\n",
		IP_SHOW(rd->ip), rd->port);

	if(!rd->ip && !rd->port){
		// 设备离线,通知上层的话会停止登录，
		//设置用户为 IDLE ，会不断连接分配服务器，就这样，大网上20秒一次也没问题
        log_debug("%s at switch to idle direct ip %u.%u.%u.%u \n",s->name,IP_SHOW(s->user->direct_ip));
        nd_login_debug(s, "switch to idle direct ip %u.%u.%u.%u \n", IP_SHOW(s->user->direct_ip));
			
		if(s->user->direct_ip == 0){
            //如果是公网，则设备真正的离线了
            if (s->user->last_err != ULGE_DEV_OFF_LINE) {
                s->user->last_err = ULGE_DEV_OFF_LINE;
                event_push(s->user->callback, UE_INFO_MODIFY, s->user->handle, s->user->callback_handle);
            }
	           // s->idle_time = 20; //公网，休息20秒后再连接
            user_set_status(s->user, CS_IDLE);

			
			nd_login_debug(s, "direct_ip == 0, ULGE_DEV_OFF_LINE\n");
        }else{
        	ucc_set_status(s, UCCS_IDLE);
        }
		
		return;
	}
	
	s->ip = rd->ip;
	s->port = rd->port;

	ucc_set_status(s, UCCS_AUTH_REQ);
}

bool mem_is_all_zero(u_int8_t *data, int len)
{
	int i;

	for(i = 0; i < len; i++) {
		if (data[i]) {
			return false;
		}
	}

	return true;
}

static void log_hex_array(char *name, u_int8_t *buf, int size) {
	int i = 0;

	log_debug("ucc_test %s ", name);
	for (i = 0; i < size; ++i) {
		log_debug(" %02x", buf[i]);
	}
	log_debug("\n");	
}


static void do_auth_question(ucc_session_t *s, uc_auth_question_t *qs)
{
	uc_auth_answer_t *ans;
	pkt_t *pkt;
	MD5_CTX ctx;
    uc_auth_question_v2_t* v2_qs = (uc_auth_question_v2_t*)qs;
	char r1[128], r2[128], md5[128];

	ucc_set_status(s, UCCS_AUTH_ANSWER);

	memcpy(s->rand2, qs->rand2, sizeof(s->rand2));
    // V2 ,带了PSK,以设备带的为准，问题，如果设备只分配一次，如何保证此报文OK
    if (s->rcv_hdr->param_len  >=  sizeof(uc_auth_question_v2_t)) {
		if (v2_qs->remain_days == 0 &&
			v2_qs->version == PROTO_VER2 &&
			!mem_is_all_zero(v2_qs->share_key, sizeof(v2_qs->share_key))) {
	        memcpy(&s->share_key, &v2_qs->share_key, V2_SHARE_KEY_LEN);
	        s->has_share_key = true;
	        s->v1_remain_days = v2_qs->remain_days;
	        mem_dump("do_auth_question received shared key", s->share_key, V2_SHARE_KEY_LEN);
			//log_debug("do_auth_question received shared key sizeof(v2_qs->share_key)=%u\n", sizeof(v2_qs->share_key));
	        //TODO: 存储共享密钥，
	        ucc_store_share_code(s);
		}
    }
    
	pkt = uc_pkt_new(s,CMD_UDP_AUTH, sizeof(uc_auth_answer_t),
				true, false, 0, s->client_sid, s->device_sid, 0);
	ans = get_ucp_payload(pkt, uc_auth_answer_t);
	ans->action = UCA_ANSWER;
	ans->support_enc = htons(UC_ENC_AES128 | UC_ENC_XXTEA);

	// MD5(token1 + token2 + password_md5)
	MD5Init(&ctx);
	MD5Update(&ctx, s->rand1, sizeof(s->rand1));
	MD5Update(&ctx, s->rand2, sizeof(s->rand2));
    if (s->has_share_key) {
        ans->flags |= 0x1;
        MD5Update(&ctx, s->share_key, sizeof(s->share_key));
    }else{
    	if (la_is_valid() && s->user->auth_err_num > 0) {
        	MD5Update(&ctx, s->user->back_passwd_md5, sizeof(s->user->passwd_md5));
		} else {
        	MD5Update(&ctx, s->user->passwd_md5, sizeof(s->user->passwd_md5));
		}
    }
	MD5Final(ans->answer, &ctx);

	fill_local_time(&ans->time);

	ucc_request_add(s, pkt);

	fmt_hex(r1, s->rand1, sizeof(s->rand1));
	fmt_hex(r2, s->rand2, sizeof(s->rand2));
	fmt_hex(md5, ans->answer, sizeof(ans->answer));
	nd_login_debug(s, "do_auth_question r1[%s] r2[%s] md5[%s]\n", r1, r2, md5);
}

static void ucc_auth_do_share_register(ucc_session_t*s)
{
    uc_share_reg_res_t* sr = get_ucp_payload(s->rcv_buf, uc_share_reg_res_t);

	//log_debug("enter ucc_auth_do_share_register sr->result=%u\n", sr->result);
    sr->result = ntohs(sr->result);
    if (sr->result != ERR_NONE) {
        s->user->last_err = sr->result;
        user_set_status(s->user, CS_LOGIN_ERR);
        return;
    }
	sr->share_code = ntoh_ll(sr->share_code);
	//log_debug("sr->share_code=%"PRIu64"\n", sr->share_code);
    uc_u64_2_char(s->share_key, &sr->share_code);
	//log_debug("ucc_test share_code %"PRIu64"\n", sr->share_code);
	//log_hex_array("share_key", s->share_key,sizeof(s->share_key));
	//log_mem_dump("sr->share_code", s->share_key, sizeof(s->share_key));
    s->has_share_key = true;
	s->is_qr_login = false;
    ucc_store_share_code(s);
    user_set_status(s->user, CS_AUTH);
}

static void ucc_auth_req_proc(ucc_session_t *s)
{
	ucph_t *hdr;
	uc_auth_redirect_t *rd;
	uc_auth_question_t *qs;

	hdr = (ucph_t *)s->rcv_buf->data;
    
    // 二维码登录时，未得到共享key
    if(hdr->command == CMD_SHARE_REGISTER && s->is_qr_login && !s->has_share_key){
        ucc_auth_do_share_register(s);
        return;
    }

	nd_login_debug(s, "do auth req proc, cmd %u\n", hdr->command);
    
	if (hdr->command != CMD_UDP_AUTH) {
		log_debug("%s at %s ignore pkt cmd=%u\n", s->name, ucc_proc[s->status].name, hdr->command);
		return;
	}

	rd = get_ucp_payload(s->rcv_buf, uc_auth_redirect_t);
	switch (rd->action) {
	case UCA_REDIRECT:
		do_auth_redirect(s, rd);
		break;
	case UCA_QUESTION:
		s->client_sid = hdr->client_sid;
		s->device_sid = hdr->device_sid;
		s->v2_client_sid = hdr->client_sid;
		s->v2_device_sid = hdr->device_sid;
		qs = get_ucp_payload(s->rcv_buf, uc_auth_question_t);
		do_auth_question(s, qs);
		break;
	default:
		log_debug("%s at %s ignore pkt action=%u\n", s->name, ucc_proc[s->status].name, rd->action);
		return;
	}
}

/***************************
	AUTH ANSWER
 ***************************/

static int auth_answer_die(cl_thread_t *t)
{
	ucc_session_t *s = (ucc_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	cl_assert(s->status == UCCS_AUTH_ANSWER);

	log_info("%s UCCS_AUTH_ANSWER die\n", s->name);
	nd_login_debug(s, "UCCS_AUTH_ANSWER\n");

    //回应超时了，可以不走分配服务器
	ucc_set_status(s, UCCS_AUTH_REQ);
	return 0;
}

static void ucc_auth_answer_into(ucc_session_t *s)
{
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, auth_answer_die, (void *)s, TIME_N_SECOND(s->die_for_auth));

	nd_login_debug(s, "ucc_auth_answer_into die_for_auth %u\n", s->die_for_auth);
}

static void ucc_auth_answer_out(ucc_session_t *s)
{
	ucc_reset_send(s);
	nd_login_debug(s, "ucc_auth_answer_out\n");
}

static RS do_tlv_vendor(ucc_session_t *s, uc_tlv_t *tlv)
{
	if (tlv->len < 1) {
		log_err(false, "%s ignore vendor len=%u\n", s->name, tlv->len);
		return RS_ERROR;
	}
	tlv_val(tlv)[tlv->len - 1] = '\0';
	STR_REPLACE(s->user->vendor_id, tlv_val(tlv));

	nd_login_debug(s, "get vendor %s\n", s->user->vendor_id);

	return RS_OK;
}

static RS do_tlv_local_time(ucc_session_t *s, uc_tlv_t *tlv)
{
	if (tlv->len < sizeof(uc_time_t)) {
		log_err(false, "%s ignore local time len=%u\n", s->name, tlv->len);
		return RS_ERROR;
	}

	// do nothing.

	return RS_OK;
}

static RS do_tlv_client_timezone(ucc_session_t *s, uc_tlv_t *tlv)
{
	if (tlv->len < sizeof(u_int32_t)) {
		log_err(false, "%s ignore timezone len=%u\n", s->name, tlv->len);
		return RS_ERROR;
	}

	s->timezone = htonl(*(u_int32_t *)(tlv+1));
	nd_login_debug(s, "client timezone %u\n", s->timezone);

	return RS_OK;
}

static RS do_tlv_time_param(ucc_session_t *s, uc_tlv_t *tlv)
{
	if (tlv->len < sizeof(uc_time_param_all_t)) {
		log_err(false, "%s ignore time param len=%u\n", s->name, tlv->len);
		return RS_ERROR;
	}

	if (memcmp(tlv_val(tlv), &s->time_param_net, sizeof(uc_time_param_all_t)) == 0) {
		return RS_OK;
	}

	if (cl_priv->is_pt_mode) { /* 产测模式 */
		s->time_param_cur = &s->time_param.lan;
	} else {
		memcpy(&s->time_param_net, tlv_val(tlv), sizeof(uc_time_param_all_t));
		memcpy(&s->time_param, tlv_val(tlv), sizeof(uc_time_param_all_t));
		s->time_param.dev.die = ntohs(s->time_param.dev.die);
		s->time_param.lan.die = ntohs(s->time_param.lan.die);
		s->time_param.wan_front.die = ntohs(s->time_param.wan_front.die);
		s->time_param.wan_background.die = ntohs(s->time_param.wan_background.die);

		if (s->user->direct_ip != 0) {
			s->time_param_cur = &s->time_param.lan;
		} else {
			if (is_user_in_background_mode(s->user)) {
                s->time_param_cur = &s->time_param.wan_background;
            } else{
                s->time_param_cur = &s->time_param.wan_front;
            }
		}
	}

	log_info("%s do_tlv_time_param [direct_ip %u.%u.%u.%u] update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
		s->name, IP_SHOW(s->user->direct_ip),
		s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
		s->time_param_cur->keeplive, s->time_param_cur->die);

	nd_login_debug(s, "%s do_tlv_time_param [direct_ip %u.%u.%u.%u] update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
		s->name, IP_SHOW(s->user->direct_ip),
		s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
		s->time_param_cur->keeplive, s->time_param_cur->die);

	return RS_OK;
}

static void do_auth_result(ucc_session_t *s, ucph_t *hdr, uc_auth_result_t *rs)
{
	MD5_CTX ctx;
	user_t *user = s->user;
	uc_tlv_t *tlv;
	int remain;
	u_int8_t sub_type;
	char pwd[256];
    
    if (hdr->param_len < sizeof(uc_auth_result_t)) {
        log_err(false, "bad auth result pkt: param_len=%u\n", hdr->param_len);
        return;
    }

	rs->select_enc = ntohs(rs->select_enc);
	rs->err_num = ntohs(rs->err_num);

	sub_type = user->sub_type;
	user->sub_type = rs->dev_type;
	user->ext_type = rs->ext_type;
    user->real_sub_type = rs->dev_type;
    user->real_ext_type = rs->ext_type;
	user->login_type = rs->login_type;
	user->net_type = rs->net_type;

    ajust_dev_sub_type_by_sn(user->sn,&user->sub_type,&user->ext_type);
    map_test_dev_sub_type(&user->sub_type,&user->ext_type);
	
	if( user->sub_type!= sub_type){
		pu_save_dev_type(user);
	}
	
	if (rs->err_num != 0) {
		log_err(false, "%s auth failed, result=%d\n", s->name, rs->err_num);
		nd_login_debug(s, "auth failed, result=%d\n", rs->err_num);
		if(rs->err_num == ERR_CLONE){
			s->user->last_err = ULGE_DEV_CLONE;
		}else if(rs->err_num == ERR_PASSWD_INVALID){
			if (s->v2_login_num > 0) {
				user_set_status(user, CS_AUTH);
				s->v2_login_num--;
				return;
			} else {
				s->user->last_err = ULGE_BAD_PASSWORD;
			}
		}else if(rs->err_num == ERR_UNBIND){
			if (s->v2_login_num > 0) {
				user_set_status(user, CS_AUTH);
				s->v2_login_num--; 
				return;
			} else {
				s->user->last_err = ERR_V2_UNBIND;
			}
		}else{
			s->user->last_err = rs->err_num;
		}
		//换个密码试下
		if (la_is_valid() && (s->user->auth_err_num == 0) &&
			memcmp(s->user->passwd_md5, s->user->back_passwd_md5, sizeof(s->user->back_passwd_md5)) &&
			!mem_is_all_zero(s->user->back_passwd_md5, sizeof(s->user->back_passwd_md5))) {
			s->user->auth_err_num++;
			user_set_status(user, CS_AUTH);

			fmt_hex(pwd, s->user->back_passwd_md5, sizeof(s->user->back_passwd_md5));
			nd_login_info(s, "try swith pwd[%s]\n", pwd);
		} else {
			la_do_dev_error_proc(s->user);
	        user_set_status(user, CS_LOGIN_ERR);
			s->user->auth_err_num = 0;
		}
		return;
	}

	s->my_request_id = s->peer_request_id = 0;

	s->select_enc = rs->select_enc;
	// 认证请求方的加密秘钥使用 MD5(rand1 + rand2 + 验证凭证 + 验证凭证)
	MD5Init(&ctx);
	MD5Update(&ctx, s->rand1, sizeof(s->rand1));
	MD5Update(&ctx, s->rand2, sizeof(s->rand2));
	MD5Update(&ctx, s->user->passwd_md5, sizeof(s->user->passwd_md5));
	MD5Update(&ctx, s->user->passwd_md5, sizeof(s->user->passwd_md5));
	MD5Final(s->key, &ctx);

	remain = hdr->param_len - sizeof(uc_auth_result_t);
	tlv = &rs->tlv[0];
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
		case UCT_VENDOR:
			do_tlv_vendor(s, tlv);
			break;
		case UCT_LOCAL_TIME:
			do_tlv_local_time(s, tlv);
			break;
		case UCT_TIME_PARAM:
			do_tlv_time_param(s, tlv);
			break;
		case UCT_SCM_FACTORY_DATA:
		case UCT_SCM_DEV_TYPE:
		case UCT_808_PAN_TYPE:
		case UCT_DEVICE_NEW_DEV_EXT_TYPE:
		case UCT_DEVELOPER_ID:
			scm_do_dispatch_tlv(s->user,tlv);
			break;
		case UCT_UCDS_TIMEZONE_PUT:
			do_tlv_client_timezone(s, tlv);
			break;
        case UCT_PHONE_INDEX:
            s->phone_index = htonl(*(u_int32_t *)(tlv+1));
            log_debug("phone index = %"PRIu64"\n",s->phone_index);
            break;
        case UCT_PHONE_PSK:
            if (tlv->len >= V2_SHARE_KEY_LEN) {
                memcpy(s->share_key, tlv+1, V2_SHARE_KEY_LEN);
                mem_dump("do_auth_result TLV received shared key", s->share_key, V2_SHARE_KEY_LEN);
                s->has_share_key = true;
                ucc_store_share_code(s);
            }
        
            break;
		case UCT_IA_IS_AP_MODE:
			s->wifi_mode = *(u_int8_t *)(tlv+1);
			break;

		case UCT_LED_COLOR_VER:
			{
				u_int8_t ver = *(u_int8_t *)(tlv+1);
				smart_appliance_ctrl_t *sac = NULL;
				smart_air_ctrl_t *sa = NULL;

				if (tlv->len > 0 && sa_init(user) == RS_OK) {
					sac = user->smart_appliance_ctrl;
					if (sac && sac->sub_ctrl) {
						sa = sac->sub_ctrl;
						sa->com_udp_dev_info.hardware_led_ver = ver;
					}
				}
			}
			break;
			
        default:
            break;
		}

		tlv = tlv_next(tlv);
		if (remain >= sizeof(uc_tlv_t)) {
			tlv->type = ntohs(tlv->type);
			tlv->len = ntohs(tlv->len);
		}
	}
	
	log_info("%s auth result ok: enc=0x%04x, dev_type=%u, ext_type=%u, login_type=%u, net_type=%u\n",
		s->name, s->select_enc, rs->dev_type, rs->ext_type, rs->login_type, rs->net_type);

	nd_login_debug(s, "auth result ok: enc=0x%04x, dev_type=%u, ext_type=%u, login_type=%u, net_type=%u\n",
		s->select_enc, rs->dev_type, rs->ext_type, rs->login_type, rs->net_type);

	//认证成功后检查一下
	//局域网还是公网
	if(s->user->direct_ip != 0){ //局域网
		if(s->ip != s->user->direct_ip){
			//局网网登录成功，但是direct_ip 不对,删除局域网的，如果重新扫描到，会重新登录
			s->user->direct_ip = 0;
			delete_lan_dev_by_sn(user->sn);
			nd_login_debug(s, "is direct login but user direct_ip %u.%u.%u.%u != %u.%u.%u.%u\n", 
				IP_SHOW(s->user->direct_ip), IP_SHOW(s->ip));
		}
	}

	ucc_set_status(s, UCCS_ESTABLISH);
    user_set_status(s->user, CS_ESTABLISH);
}

static void ucc_auth_answer_proc(ucc_session_t *s)
{
	ucph_t *hdr;
	uc_auth_result_t *rs;

	hdr = (ucph_t *)s->rcv_buf->data;

	nd_login_debug(s, "ucc_auth_answer_proc cmd %u\n", hdr->command);
	
    //回应的时候要处理reset报文
    if (is_reset_pkt(hdr)) {
		log_info("%s reset by peer\n", s->name);
		nd_login_info(s, "reset by peer in answer proc\n");
		ucc_set_status(s, UCCS_AUTH_REQ);
		return;
	}
    
	if (hdr->command != CMD_UDP_AUTH) {
		log_debug("%s at %s ignore pkt cmd=%u\n", s->name, ucc_proc[s->status].name, hdr->command);
		return;
	}

	rs = get_ucp_payload(s->rcv_buf, uc_auth_result_t);
	switch (rs->action) {
	case UCA_RESULT:
		do_auth_result(s, hdr, rs);
		break;
	default:
		log_debug("%s at %s ignore pkt action=%u\n", s->name, ucc_proc[s->status].name, rs->action);
		return;
	}
}


/***************************
	ESTABLISH
 ***************************/

static int ucc_estab_die(cl_thread_t *t)
{
	ucc_session_t *s = (ucc_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	cl_assert(s->status == UCCS_ESTABLISH);

	log_info("%s UCCS_ESTABLISH die\n", s->name);
	nd_login_debug(s, "UCCS_ESTABLISH die\n");

	ucc_set_status(s, UCCS_AUTH_REQ);
	
	return 0;
}

static void ucc_estab_com_init(ucc_session_t *s)
{
	user_t *user = s->user;

	//迫使设备登录成功后强行设置一次app状态
	user->last_background_status = !cl_priv->run_in_background;
}

static void ucc_estab_into(ucc_session_t *s)
{
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, ucc_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));

    s->user->last_err = 0;
	s->user->last_ctrl = false;
    s->user->online = true;
	s->v2_login_num = 0;
	user_set_status_uc(s->user, CS_ESTABLISH, true);
    
    // 先快速发一个keeplive，获取一些参数
    CL_THREAD_OFF(s->t_keeplive);
    CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, ucc_keeplive_timer, (void *)s, 0);
    
    sa_force_refresh_data(s->user);

#if 0
	if (s->user->sub_type == IJ_RFGW) {
		rf_slave_init(s->user);
	}
#endif	

	//这里处理一下，如果没有圈子给手机报个添加圈子设备成功啥的，目的是避免app在登陆界面转
	la_do_dev_estab_proc(s->user);
	//保存正常密码
	do_la_back_passwd(s->user);

	ucc_estab_com_init(s);
	
	s->user->auth_err_num = 0;
	s->user->establish_num++;
	s->user->disp_die_num = 0;

	if (s->debug_on) {
		s->need_debug_on = true;
	}
	
	nd_login_debug(s, "ucc_estab_into...\n");
}

static void ucc_estab_out(ucc_session_t *s)
{
	ucc_reset_send(s);
	ucc_off_all_timer(s);
	CL_THREAD_OFF(s->user->t_timer_comm);
	sa_do_action_when_estab_out(s->user);
	user_set_status_uc(s->user, CS_AUTH, true);
	nd_login_debug(s, "ucc_estab_out\n");
}

void cl_net_debug(ucc_session_t *s, bool debug_on)
{
	int n = 0;		
	u_int8_t on = 0;
	pkt_t *pkt = NULL;		
	u_int8_t *data_buff = NULL;	

	if (s->status != UCCS_ESTABLISH) {
		return;
	}	

	pkt = uc_pkt_new(s, CMD_USER_DEBUG, sizeof(on), 
				true, true, 0, s->client_sid,	s->device_sid, s->my_request_id);
	data_buff = get_ucp_payload(pkt, u_int8_t);

	if (debug_on) {
		on = 1;
	} else {
		on = 0;
	}
	*data_buff = on;
	log_debug("cl_net_debug send on=%u\n", on);	
	ucc_request_add(s, pkt);
}

static int ucc_keeplive_timer(cl_thread_t *t)
{
	ucc_session_t *s;
	pkt_t *pkt;
	uc_keeplive_request_t *kr;

	s = (ucc_session_t *)CL_THREAD_ARG(t);
	s->t_keeplive = NULL;
	// 不着急设置下一个定时器，等本次的发完毕后，在ucc_request_del中设置
	//CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, ucc_keeplive_timer, (void *)s, 0);

	// 发送队列目前还有报文等待发送，没必要再发保活报文了
	if ( ! stlc_list_empty(&s->send_list) )
		return 0;

	pkt = uc_pkt_new(s,CMD_UDP_KEEPLIVE, sizeof(uc_keeplive_request_t),
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	kr = get_ucp_payload(pkt, uc_keeplive_request_t);
	kr->action = UCA_REQUEST;
	
	ucc_request_add(s, pkt);

#ifdef	WIN32
	//开启debug
	log_debug("ucc_keeplive_timer need_debug_on=%u\n", s->need_debug_on);
	if (s->need_debug_on) {
		s->need_debug_on = false;
		cl_net_debug(s, true);
	}
#endif	

	nd_login_debug(s, "send keeplive\n");

	return 0;
}

static void do_keeplive(ucc_session_t *s, ucph_t *hdr)
{
	uc_keeplive_reply_t *kr;
	uc_tlv_t *tlv;
	int remain;

	if (hdr->param_len < sizeof(uc_keeplive_reply_t)) {
		log_err(false, "bad keeplive packet: param_len=%u\n", hdr->param_len);
		return;
	}
	
	kr = get_net_ucp_payload(hdr, uc_keeplive_reply_t);
	if (kr->action == UCA_RESET) {
		log_info("%s reset by peer\n", s->name);
		ucc_set_status(s, UCCS_AUTH_REQ);
		nd_login_debug(s, "get keeplive reset\n");
		return;
	}

	if (kr->action != UCA_REPLY) {
		log_debug("%s ignore keeplive action=%u\n", kr->action);
		return;
	}

	tlv = &kr->tlv[0];

	remain = hdr->param_len - sizeof(uc_keeplive_reply_t);
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		switch (tlv->type) {
		case UCT_TIME_PARAM:
			do_tlv_time_param(s, tlv);
			break;
		}

		remain -= (sizeof(uc_tlv_t) + tlv->len);
		tlv = tlv_next(tlv);
		if (remain >= sizeof(uc_tlv_t)) {
			tlv->type = ntohs(tlv->type);
			tlv->len = ntohs(tlv->len);
		}
	}
}

static RS ucc_check_request_id(ucc_session_t *s, ucph_t *request)
{
	
	nd_login_debug(s, "request id 0x%x last 0x%x flag 0x%x\n", request->request_id, (s->peer_request_id) & 0xff, request->flags);

	if (request->request_id == (s->peer_request_id &0xFF)) {
		return RS_OK;
	} else if (request->request_id == (u_int8_t)(s->peer_request_id + 1)) {
		// good, update to next
		log_info("%s update rquest id to %u\n", s->name, request->request_id);
		s->peer_request_id++;
		return RS_OK;
	}

	//这里判断一下如果是rfgw发送的push报文就放过
	if (request->flags == 0x01) {
		return RS_OK;
	}

	log_err(false, "%s ignore request pkt: reply id=%u, but local is %u.\n",
		s->name, request->request_id, s->peer_request_id);

	return RS_ERROR;
}
static void ucc_check_upgrade_progress(ucc_session_t *s, pkt_t *pkt)
{
	int ps;
	u_int32_t now = get_sec();

	//log_debug("pkt->up_total =%d pkt->up_current =%d \n", pkt->up_total, pkt->up_current);
	nd_login_debug(s, "pkt->up_total =%d pkt->up_current =%d \n", pkt->up_total, pkt->up_current);
	ps = (pkt->up_current*100)/pkt->up_total;
	
	//这里需要处理一下，升级大镜像如s3 3m时可能导致事件太多，卡死app上层，处理策略是1s一个事件等等。。。
	if ((s->up_event_recv_last_time != now) || (ps == 100)) {
		s->up_event_recv_last_time = now;
		event_push_err(s->user->callback, UE_DEV_UPGRADE_PROGRESS, s->user->handle, s->user->callback_handle, ps);
		event_cancel_merge(s->user->handle);
	}
	
	if(pkt->up_current >= pkt->up_total){
		s->up_total = 0;
		//s->user->rf_up_type = 0;
		s->user->rf_need_up = 0;
		//升级完了通知一下
		event_push(s->user->callback, UE_INFO_MODIFY, s->user->handle, s->user->callback_handle);
        event_cancel_merge(s->user->handle);
		nd_login_debug(s, "uprade done...\n");
	}
	
}
static RS ucc_check_reply_id(ucc_session_t *s, ucph_t *reply)
{
	pkt_t *pkt;
	ucph_t *request;
	
	if (stlc_list_empty(&s->send_list)) {
		log_err(false, "%s ignore reply pkt: no request in send list.\n", s->name);
		nd_login_debug(s, "ignore reply pkt: no request in send list.\n");
		return RS_ERROR;
	}

	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	request = (ucph_t *)&pkt->data[0];
	if (request->request_id != reply->request_id) {
		log_err(false, "%s ignore reply pkt: my request id=%u, but %u in pkt.\n",
			s->name, request->request_id, reply->request_id);

		nd_login_debug(s, "ignore reply pkt: my request id=%u, but %u in pkt.\n",
			s->name, request->request_id, reply->request_id);
		
		return RS_ERROR;
	}
	
	if(s->up_total && pkt->up_total){
		ucc_check_upgrade_progress(s, pkt);
	}

	return RS_OK;
}

static void ucc_do_user_dev(ucc_session_t *s, ucph_t *hdr)
{
	u_int8_t *ptr;
	pkt_t *pkt;

	if (hdr->param_len < sizeof(ucp_app_data_hdr_t)) {
		log_debug( "bad ucc_do_user_dev packet: param_len=%u\n", hdr->param_len);
		return;
	}

	ptr = get_net_ucp_payload(hdr, u_int8_t);

	app_proc_pkt_from_wifi_dev(s->sn, ptr, hdr->param_len);

	//先回答
	pkt = uc_pkt_new(s, CMD_APP_DEV_USER, 0, 
				false, true, 0,	s->client_sid, s->device_sid, s->peer_request_id);
	if (!pkt) {
		return;
	}
	
	ucc_enc_pkt(s, pkt);
	uc_send_pkt_raw(s->sock, s->ip, s->port, pkt);
	pkt_free(pkt);
}

static void ucc_estab_do_request(ucc_session_t *s, ucph_t *hdr)
{
	if (ucc_check_request_id(s, hdr) != RS_OK)
		return;

	nd_login_debug(s, "get request cmd %u\n", hdr->command);

    //预处理
	switch (hdr->command) {
	case CMD_UDP_KEEPLIVE:
		do_keeplive(s, hdr);
            return;
		break;
	case CMD_APP_DEV_USER:		
		ucc_do_user_dev(s, hdr);
		break;
	case CMD_UDP_NOTIFY:
		break;
	case CMD_UDP_BIND_PHONE:
		break;
    default:
		break;
	}
    sa_do_uc_request_pkt(s->user, hdr);
}

static void ucc_estab_proc(ucc_session_t *s)
{
	ucph_t *hdr;
#ifdef	WIN32
	char *ptemp = NULL;	
#endif
	
	hdr = (ucph_t *)s->rcv_buf->data;

	// check if it's keeplive.reset
	if (is_reset_pkt(hdr)) {
		log_debug("%s reset by peer\n", s->name);
		
		nd_login_debug(s, "reset by peer\n");
		if (s->user) {
			s->user->is_reset_active = true;
		}
		ucc_set_status(s, UCCS_AUTH_REQ);
		return;
	}


	// process request packet
	if (hdr->request) {
		ucc_estab_do_request(s, hdr);
		return;
	}

	//hdr->command, hdr->flags, hdr->param_len);
	//用hdr->flag来表示到底是回的调试开关命令响应包还是普通push包。flag为0表示响应包，为1表示push包。
#ifdef	WIN32
	if ((hdr->flags == 1) &&
		(CMD_USER_DEBUG == hdr->command)) {
		ptemp = (char *)get_net_ucp_payload(hdr, u_int8_t);
		ptemp[hdr->param_len - 1] = '\0';
		cl_debug_buff_add(s, ptemp, hdr->param_len);		
		return;
	}
#endif

	/*
		bellow process reply packet
	*/
	
	if (ucc_check_reply_id(s, hdr) != RS_OK) {
		return;
	}

	switch (hdr->command) {
	
	case CMD_UDP_KEEPLIVE:
		do_keeplive(s, hdr);
		break;
	case CMD_APP_DEV_USER:
		app_proc_pkt_from_wifi_dev(s->sn, get_net_ucp_payload(hdr, u_int8_t), hdr->param_len);
		break;
	case CMD_UDP_NOTIFY:
    case CMD_UDP_CTRL:
	case CMD_UDP_BIND_PHONE:
        sa_do_uc_reply_pkt(s->user, hdr);
		break;
	default:
		log_debug("%s at %s ignore reply pkt cmd=%u\n", s->name, ucc_proc[s->status].name, hdr->command);
        return;
		break;
	}

	// 删除掉已经应答的报文
	ucc_request_del(s);

	// 在处理完报文再设置死亡时间。因为可能修改了死亡时间参数
	if (s->status == UCCS_ESTABLISH) {
		CL_THREAD_OFF(s->t_die);
		CL_THREAD_TIMER_ON(MASTER, s->t_die, ucc_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
	}
}

/***************************
	ERROR
 ***************************/
static void ucc_error_into(ucc_session_t *s)
{
	ucc_reset_send(s);
	ucc_off_all_timer(s);

	s->user->auth_err_num = 0;
}

static void ucc_error_out(ucc_session_t *s)
{
	// do nothing
}

static void ucc_error_proc(ucc_session_t *s)
{
	ucph_t *hdr;

	hdr = (ucph_t *)s->rcv_buf->data;

	// do nothing
	log_debug("%s at %s ignore reply pkt cmd=%u\n", s->name, ucc_proc[s->status].name, hdr->command);
}

/*
	FSM
*/
ucc_proc_t ucc_proc[UCCS_MAX] = {
	{"IDLE", ucc_idle_into, ucc_idle_out, ucc_idle_proc},
	{"AUTH_REQ", ucc_auth_req_into, ucc_auth_req_out, ucc_auth_req_proc},
	{"AUTH_ANSWER", ucc_auth_answer_into, ucc_auth_answer_out, ucc_auth_answer_proc},
	{"ESTAB", ucc_estab_into, ucc_estab_out, ucc_estab_proc},
	{"ERROR", ucc_error_into, ucc_error_out, ucc_error_proc},
};

void ucc_set_status(ucc_session_t *s, int status)
{
	int prev_satus = s->status;
    
	if (status >= UCCS_MAX) {
		log_err(false, "ucc_set_status unknow new status = %d\n", status);
		return;
	}

	s->status = status;
    
	log_info("%s modify status from %s to %s\n",
		s->name, ucc_proc[prev_satus].name, ucc_proc[status].name);

	nd_login_debug(s, "%s modify status from %s to %s\n",
		s->name, ucc_proc[prev_satus].name, ucc_proc[status].name);

	ucc_proc[prev_satus].on_out(s);
	ucc_proc[status].on_into(s);
}

/*
	实际环境中发现，如果每次退出又进来、试用新的UDP端口，可能会登录不成功，或者登录
	很慢，要一两分钟，抓包看，服务器转给客户端的报文掉了。
	后来使用固定端口，该问题解决。测试了50次，出现过几次10秒的，其他都在5秒以内
	所以这里使用固定端口，根据序列号计算一个出来
*/
#define	UCC_PORT_BEGIN	15001
#define	UCC_PORT_NUM	800

// 先简单点用一个字节表示一个端口是否被用
static u_int8_t ucc_port_bitmap[UCC_PORT_NUM] = {0};

void ucc_port_init()
{
	MD5_CTX ctx;
	u_int8_t result[16];
	int num;
	
	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)&cl_priv->vvid, (u_int32_t)sizeof(cl_priv->vvid));
	MD5Update(&ctx, (u_int8_t *)&cl_priv->uuid_bin, (u_int32_t)sizeof(cl_priv->uuid_bin));
	MD5Final(result, &ctx);

	num = result[0]%16;
	
	cl_priv->ucc_port_base = UCC_PORT_BEGIN + UCC_PORT_NUM*num;;
}

int ucc_get_port(u_int64_t sn, const char *name)
{
	int i, port;
	MD5_CTX ctx;
	u_int8_t result[16];
	u_int16_t *p, sum;

	MD5Init(&ctx);
	if (sn == 0) {
		MD5Update(&ctx, (u_int8_t *)name, (u_int32_t)strlen(name));
	} else {
		MD5Update(&ctx, (u_int8_t *)&sn, (u_int32_t)sizeof(u_int64_t));
	}
	MD5Final(result, &ctx);

	for (p = (u_int16_t *)result, i = 0, sum = 0; i < 8; i++, p++) {
		sum += *p;
	}

	port = sum%UCC_PORT_NUM;
	for (i = 0; i < UCC_PORT_NUM; i++) {
		if ( ! ucc_port_bitmap[port] )
			break;
		
		port = (port + i)%UCC_PORT_NUM;
	}

	if (i == UCC_PORT_NUM) {
		log_err(false, "no ucc port, use random\n");
		return 0;
	}
	ucc_port_bitmap[port] = true;

	return (port + cl_priv->ucc_port_base);
}

void ucc_free_port(int port)
{
	if (port == 0)
		return;

	port -= cl_priv->ucc_port_base;
	if (port >= UCC_PORT_NUM)
		return;
	ucc_port_bitmap[port] = false;
}

/*
 路由器某些端口被限制，丢包严重
 重新选择端口，
 */
static int ucc_re_select_port(int old_port)
{
    int port,i;
    
    if (old_port >= cl_priv->ucc_port_base && old_port <= cl_priv->ucc_port_base+UCC_PORT_NUM) {
        port = old_port - cl_priv->ucc_port_base;
        for (i = 1; i < UCC_PORT_NUM; i++) {
            port = (port + i)%UCC_PORT_NUM;
            if ( ! ucc_port_bitmap[port] )
                break;
        }
        if (i >= UCC_PORT_NUM) {
            return 0;
        }
        log_debug("ucc_re_select_port port  = %u\n",port);
        ucc_port_bitmap[port] = true;
        
        return (port + cl_priv->ucc_port_base);
    }
    return old_port;
}

static void ucc_check_need_switch_port(ucc_session_t *s)
{
    int port,count;
    if(!s){
        return;
    }
    
    //局域网，不用转换端口
    if (s->user->direct_ip != 0) {
        return;
    }
    
    //第一次发包，不换端口
    if (s->send_pkts == 0 || s->recv_pkts>=s->send_pkts) {
        return;
    }
    
    //收包大于15个,不换端口
    if (s->recv_pkts > 15) {
        return;
    }
    
    //正常的一问一答，告知离线，密码错这些，不用换端口
    if(s->send_pkts - s->recv_pkts < 5){
        return;
    }
  
    
    count = 0;
    while (count < 128 ) {
        
        port = ucc_re_select_port(s->my_port);
        if(s->sock != INVALID_SOCKET){
            CLOSE_SOCK(s->sock);
            ucc_free_port(s->my_port);
        }
        log_debug("port %u my_port %u\n",port,s->my_port);
        s->my_port = port;
        s->sock = create_udp_server(0, s->my_port);
        if (s->sock >= 0) {
            CL_THREAD_READ_OFF(s->t_recv);
            CL_THREAD_READ_ON(MASTER, s->t_recv, ucc_read, (void *)s, s->sock);
            break;
        }
    }
    
    log_info("%s (sn=%012"PRIu64") switch port try use local udp port=%u\n", s->name, s->sn, s->my_port);
   	nd_login_debug(s, "switch port try use local udp port=%u\n", s->name, s->sn, s->my_port);
}

RS ucc_udp_socket(ucc_session_t *s)
{
    int port = s->my_port;
	s->auth_lost = 0;
    
	if(s->sock != INVALID_SOCKET){
        return RS_OK;
	}
	//先get再释放，否则永远都是一个端口
	s->my_port = ucc_get_port(s->sn, s->name);
    ucc_free_port(port);
	s->sock = create_udp_server(0, s->my_port);
	log_info("%s (sn=%012"PRIu64") try use local udp port=%u\n", s->name, s->sn, s->my_port);
	
	if (s->sock == INVALID_SOCKET) {
		if ((s->sock = create_udp_server(0, 0)) == INVALID_SOCKET) {
			return RS_ERROR;
		}
		s->my_port = 0;
	}
	return RS_OK;

}

/*
	新建一个局域网连接的UDP控制设备
*/
RS ucc_new(user_t *user)
{
	ucc_session_t *s;
    u_int32_t r1;
	
	s = (ucc_session_t *)cl_calloc(sizeof(ucc_session_t), 1);
	s->name = cl_malloc(strlen(user->name) + 32);
	sprintf(s->name, "'UCC[%s]'", user->name);

	s->status = UCCS_IDLE;
	s->idle_time = 0;

	// init default time parament
	uc_set_default_time_param(&s->time_param, &s->time_param_net);
    
	if (user->direct_ip != 0) {
		s->time_param_cur = &s->time_param.lan;
	} else {
		s->time_param_cur = &s->time_param.wan_front;
	}
    //
    srand(get_sec());
    r1 = rand();
    
    memcpy(&s->rand1, &r1, sizeof(s->rand1));
    log_debug("%s[direct_ip %u.%u.%u.%u] update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
              s->name, IP_SHOW(user->direct_ip),
              s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
              s->time_param_cur->keeplive, s->time_param_cur->die);
	
	s->user = user;
	s->sn = user->sn;
	s->ip = user->direct_ip;
	s->port = DFL_UDP_CTRL_CLIENT_WAN_PORT;
    s->die_for_auth = DIE_TIME_FOR_AUTH;
    memcpy(s->uuid, cl_priv->uuid_bin, sizeof(s->uuid));

	s->sock = INVALID_SOCKET;
	if (ucc_udp_socket(s) != RS_OK) {
		cl_free(s->name);
		cl_free(s);
		return RS_ERROR;
	}

	STLC_INIT_LIST_HEAD(&s->send_list);
	s->rcv_buf = pkt_new(0, MAX_UDP_PKT, 0);

	user->uc_session = s;
    s->phone_index = 0xFFFFFFFF;
	ucc_set_status(s, UCCS_ERROR);
    
    
    if(user->qr_code != NULL){
        //用户用新的二维码添加设备，则不读本地的了
        ucc_proc_qr_code(user,s);
        //删除本地保存到文件中的预共享key
        ucc_del_share_code(s);
    }else{
        //读取本地的预共享key
        ucc_read_share_code(s);
    }    

	CL_THREAD_READ_ON(MASTER, s->t_recv, ucc_read, (void *)s, s->sock);

	return RS_OK;
}

void ucc_free(user_t *user)
{
	ucc_session_t *s;

	if ((s = (ucc_session_t *)user->uc_session) == NULL)
		return;

    
	ucc_reset_send(s);
	ucc_off_all_timer(s);
	CL_THREAD_OFF(s->t_recv);
	SAFE_FREE(s->name);
	pkt_free(s->rcv_buf);
	CLOSE_SOCK(s->sock);
	ucc_free_port(s->my_port);

	cl_free(s);
	user->uc_session = NULL;
    
}


