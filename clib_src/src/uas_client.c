#include "client_lib.h"
#include "cl_priv.h"
#include "md5.h"
#include "cl_thread.h"
#include "aes.h"

#include "uc_client.h"
#include "udp_ctrl.h"
#include "uas_client.h"
#include "lan_dev_probe_priv.h"
#include "cl_server.h"
#include "smart_appliance_priv.h"
#include "uasc_priv.h"
#include "cl_log.h"
#include "udp_device_common_priv.h"
#include "rfgw_priv.h"

#undef MAX_SEND_RETRY_NUM
#define MAX_SEND_RETRY_NUM  (50)
#define MAX_PARAM_LEN_PER_PKT (1200)

#undef MAX_AUTH_LOST
#define MAX_AUTH_LOST (3)
static const char* share_key = "airCloudServerPwd";

#define UAS_TEST_IP "10.133.17.56"
#define UAS_TEST_PORT 51949

/********************打印调试开关********************打印调试开关***************
*************打印调试开关********************打印调试开关*******************/
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



extern ucc_proc_t uasc_proc[UASCS_MAX];
RS uasc_udp_socket(uasc_session_t *s);

static int uasc_keeplive_timer(cl_thread_t *t);

extern bool air_update_data_with_mib(void* ctrl,u_int8_t action,ucp_obj_t* obj);


/*
	发送一个已经封装好的udp ctrl报文
*/
RS uasc_ctrl_obj_value(u_int64_t sn, u_int8_t action, u_int8_t obj_count, void* content, int content_len)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
	uc_tlv_t *tlv;
	uasc_session_t *s = cl_priv->uasc_session;
	int param_len = 0;

	param_len = sizeof(*tlv) + sizeof(sn) +
				sizeof(*tlv) + sizeof(*uc) + content_len;

	if (s == NULL || s->status != UASCS_ESTABLISH) {
		return RS_ERROR;
	}
    
    if (!content ||content_len <= 0) {
        return RS_ERROR;
    }
    pkt = uasc_pkt_new(CMD_TLV_UDP_CTRL, param_len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	tlv = get_ucp_payload(pkt, uc_tlv_t);

	tlv->type = ntohs(AS_TT_SN);
	tlv->len = ntohs(sizeof(sn));
	sn = ntoh_ll(sn);
	memcpy(tlv_val(tlv), &sn, sizeof(sn));

	tlv = tlv_n_next(tlv);
	tlv->type = ntohs(AS_TT_UDP_CTRL);
	tlv->len = ntohs((u_int16_t)(sizeof(* uc) + content_len));
	uc = (ucp_ctrl_t *)tlv_val(tlv);
	
	uc->action = action;
    uc->count = obj_count;
    memcpy((void*)(uc+1), content, content_len);
    
    uasc_request_add(cl_priv->uasc_session, pkt);

    return RS_OK;
}

pkt_t* uasc_mk_ctrl_pkt(u_int64_t sn, u_int8_t action, u_int8_t obj_count, void* content, int content_len)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    uc_tlv_t *tlv;
    uasc_session_t *s = cl_priv->uasc_session;
    int param_len = 0;
    
    param_len = sizeof(*tlv) + sizeof(sn) +
				sizeof(*tlv) + sizeof(*uc) + content_len;
    
    if (!s || !content ||content_len <= 0) {
        return NULL;
    }
    pkt = uasc_pkt_new(CMD_TLV_UDP_CTRL, param_len,
                       true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
    tlv = get_ucp_payload(pkt, uc_tlv_t);
    
    tlv->type = ntohs(AS_TT_SN);
    tlv->len = ntohs(sizeof(sn));
    sn = ntoh_ll(sn);
    memcpy(tlv_val(tlv), &sn, sizeof(sn));
    
    tlv = tlv_n_next(tlv);
    tlv->type = ntohs(AS_TT_UDP_CTRL);
    tlv->len = ntohs((u_int16_t)(sizeof(* uc) + content_len));
    uc = (ucp_ctrl_t *)tlv_val(tlv);
    
    uc->action = action;
    uc->count = obj_count;
    memcpy((void*)(uc+1), content, content_len);
    
    return pkt;
}

//////////////////////////////////
//fragment

static uasc_defrag_cache_t* new_defrag_cache()
{
    uasc_defrag_cache_t * cache;
    
    cache = cl_calloc(sizeof(*cache), 1);
    if (!cache) {
        return NULL;
    }
    
    STLC_INIT_LIST_HEAD(&cache->link);
    
    return cache;
}

static uasc_defrag_cache_t* lookup_cache_by_ident(u_int32_t ident,struct stlc_list_head* list)
{
    uasc_defrag_cache_t* c;
    if (!list) {
        return NULL;
    }
    
    stlc_list_for_each_entry(uasc_defrag_cache_t, c, list, link){
        if (c->frag_ident == ident) {
            return c;
        }
    }
    
    return NULL;
}

static bool is_all_frag_ok(uasc_defrag_cache_t* cache)
{
    if(!cache){
        return false;
    }
    
    if (cache->total > 0 && (cache->frag_recved == cache->total)) {
        return true;
    }
    
    return false;
}


static void _uasc_frag_free(uasc_defrag_cache_t* dc)
{
    int i;
    
    if (!dc) {
        return;
    }
    
    for (i = 0; i < UASC_MAX_FRAG_PET_PKT && i < dc->total; i++) {
        if (dc->pkts[i] != NULL) {
            pkt_free(dc->pkts[i]);
            dc->pkts[i] = NULL;
        }
    }
    
    if (dc->whole_pkt != NULL) {
        pkt_free(dc->whole_pkt);
        dc->whole_pkt = NULL;
    }
    
    cl_free(dc);
}

static void _uasc_reset_frag_list(uasc_session_t *s)
{
    uasc_defrag_cache_t* cur,*next;
    
    stlc_list_for_each_entry_safe(uasc_defrag_cache_t, cur, next, &s->defrag_list, link){
        stlc_list_del(&cur->link);
        _uasc_frag_free(cur);
    }
}

static void uasc_respose_frag_packet(uasc_session_t* s,uascph_t* shdr,u_int8_t error)
{
    pkt_t *pkt;
    uascp_report_result_t* rs;
    
    pkt = uasc_pkt_new(shdr->command, sizeof(uascp_report_result_t)
                     ,false, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->peer_request_id);
    rs = get_uascp_payload(pkt, uascp_report_result_t);
    rs->errorcode = error;

    uasc_response_send(s,pkt);
}

/*****************************************
	报文处理相关的函数
 *****************************************/

static RS uasc_event_push(int event, cl_handle_t obj_handle)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;

    if (ldpc != NULL) {
        event_push(ldpc->callback, event, obj_handle, ldpc->callback_handle);
    }
    return RS_OK;
}

static int uasc_send_timer(cl_thread_t *t)
{
	int n;
	uasc_session_t *s;
	struct sockaddr_in addr;
	pkt_t *pkt;

	s = (uasc_session_t *)CL_THREAD_ARG(t);
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
	
	// 太久没有连接上...
	if(s->send_retry > MAX_SEND_RETRY_NUM){
		log_info("uasc_send_timer send pkt %u failed! to IDLE stat!\n",MAX_SEND_RETRY_NUM);
		uasc_set_status(s, UASCS_ERROR);
		return 0;
	}

	
	
	CL_THREAD_TIMER_ON(MASTER, s->t_send, uasc_send_timer, (void *)s, n);
	s->send_retry++;
	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(s->ip);
    addr.sin_port = htons(s->port);
	n = (int)sendto(s->sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (n <= 0) {
		log_err(true, "%s send pkt failed: ip=%u.%u.%u.%u, port=%d, want=%d, send=%d errno[%d]\n",
			s->name, IP_SHOW(s->ip), s->port, pkt->total, n,errno);
	} else {
#ifdef	DBG_UDP_CTRL_PKT	
		uascph_t *hdr;
		char upg[128] = "";
		if(pkt->up_total){
			sprintf(upg, " upgrade %hu / %hu ", pkt->up_current, pkt->up_total);
		}
		hdr = (uascph_t *)pkt->data;
		log_debug("%s send ip=%u.%u.%u.%u, port=%d, len=%d: cmd=%d, req=%d, enc=%d, csid=%08x,  rid=%u %s\n",
			s->name, IP_SHOW(s->ip), s->port, n, 
			pkt->cmd, hdr->request, hdr->encrypt,
			ntohl(hdr->client_sid), hdr->request_id, upg);
//        mem_dump("pkt content", pkt->data, pkt->total);
#endif
	}

	return 0;
}

static RS uasc_enc_pkt(uasc_session_t *s, pkt_t *pkt)
{
	uascph_t *hdr = (uascph_t *)pkt->data;
	
	if (s->select_enc == UC_ENC_AES128) {
		int len;	
		u_int8_t pad;

		len = pkt->total - uascph_hdr_plain_size;
		pad = AES128_EKY_LEN - (len & (AES128_EKY_LEN - 1));
		len += pad;
		
		memset(pkt->data + pkt->total, pad, pad);

		enc_block((u_int8_t *)BOFP(pkt->data, uascph_hdr_plain_size), len, s->key);
		pkt->total = len + uascph_hdr_plain_size;

		hdr->encrypt = 1;
	} else {
		hdr->encrypt = 0;
	}

	return RS_OK;
}


static bool  _is_over_keepalive_time(u_int8_t cur_id,u_int8_t last_id)
{
	u_int8_t max = 15;
	u_int8_t sub = cur_id - last_id;
	
	if((cur_id == last_id) || sub > 0x7F){
		return false;
	}	

	if(sub >= max){
		return true;
	}

	return false;
}

/*
	发送请求报文，需要定时重发，重发时间递增
*/

RS uasc_request_add(uasc_session_t *s, pkt_t *pkt)
{
	if (s->status == UASCS_ESTABLISH) {
		// 超过一段时间，断开连接
		//保活报文，一定是列表是空才添加
		if(pkt->cmd == CMD_UDP_KEEPLIVE ){
			if(s->last_keep_id == 0 && s->my_request_id != 0){
				s->last_keep_id = s->my_request_id;
			}
			//保活报文超过数量
			if(_is_over_keepalive_time(s->my_request_id,s->last_keep_id)){
				uasc_set_status(s,UASCS_ERROR);
				return RS_ERROR;
			}
			
		}else{
			s->last_keep_id = 0;
		}

		if (uasc_enc_pkt(s, pkt) != RS_OK) {
			pkt_free(pkt);
			return RS_ERROR;
		}
		s->my_request_id++;
    }

	if (stlc_list_empty(&s->send_list)) {
		s->send_retry = 0;
		CL_THREAD_TIMER_ON(MASTER, s->t_send, uasc_send_timer, (void *)s, 0);
	}
	
	stlc_list_add_tail(&pkt->link, &s->send_list);

	CL_THREAD_OFF(s->t_keeplive);

	return RS_OK;
}

RS uasc_request_add_by_data(uasc_session_t *s,int cmd, void* param, int param_len,
                                   bool is_request, bool is_enc, u_int8_t flags,
                                   u_int32_t client_sid)
{
    pkt_t * pkt;
    int len,remain,num,index;
    u_int8_t* pdata;
    uascph_t* hdr;
    
    if (param_len <= 0 || !param || !s) {
        return RS_INVALID_PARAM;
    }
    
    if (param_len <= MAX_PARAM_LEN_PER_PKT) {
        pkt = uasc_pkt_new(cmd, param_len, is_request, is_enc, flags, client_sid, s->device_sid, s->my_request_id);
        if (pkt) {
            memcpy(&pkt->data[sizeof(uascph_t)], param, param_len);
            return uasc_request_add(s,pkt);
        }
        return RS_MEMORY_MALLOC_FAIL;
    }
    
    remain = param_len;
    pdata = (u_int8_t*)param;
    index = 0;
    
    s->rand_num++;
    num = (remain + MAX_PARAM_LEN_PER_PKT-1)/MAX_PARAM_LEN_PER_PKT;
    
    while (remain > 0) {
        len = (remain >= MAX_PARAM_LEN_PER_PKT)?MAX_PARAM_LEN_PER_PKT:remain;
        pkt = uasc_pkt_new(cmd, len, is_request, is_enc, flags, client_sid, s->device_sid, s->my_request_id);
        if (!pkt) {
            return RS_MEMORY_MALLOC_FAIL;
        }
        
        hdr = (uascph_t*)&pkt->data[0];
        hdr->frag_total = htons(num);
        hdr->frag_offset = htons(index);

        index++;
        hdr->frag_ident = htons(s->rand_num);
        hdr->is_frag = true;
        
        
        memcpy(&pkt->data[sizeof(uascph_t)], pdata, len);
        uasc_request_add(s,pkt);
        remain -= len;
        pdata += len;
    }
    
    return RS_OK;
}

RS uasc_remove_wait_pkt_by_ident(uasc_session_t *s, u_int64_t ident)
{
    pkt_t *pkt, *next;
    
    if (!s) {
        return RS_OK;
    }
    
    //发送队列,第一个数据包不能删除
    stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_list, link) {
        if (pkt->pkt_ident == ident) {
            if ((pkt->link.prev == &s->send_list) && (s->status== UASCS_ESTABLISH)) {
                continue;
            }
            
            stlc_list_del(&pkt->link);
            pkt_free(pkt);
            
        }
    }
    
    //发送等待队列
    stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_wait_list, link) {
        if (pkt->pkt_ident == ident) {
            stlc_list_del(&pkt->link);
            pkt_free(pkt);
        }
    }
    
    return RS_OK;
}

RS uasc_try_connect_to_server(uasc_session_t *s)
{
    if (s && s->status >= UASCS_ERROR) {
		s->try_count = 0;
        uasc_set_status(s, UASCS_DISPATCH);
    }
    return RS_OK;
}

RS uasc_wait_request_add(uasc_session_t *s, pkt_t *pkt,u_int64_t sn)
{
    if (!s || !pkt) {
        pkt_free(pkt);
        return RS_ERROR;
    }
    
    pkt->pkt_ident = sn;
    if (s->status == UASCS_ESTABLISH) {
        uasc_request_add(s, pkt);
    }else{
        stlc_list_add_tail(&pkt->link, &s->send_wait_list);
        uasc_try_connect_to_server(s);
    }
    
    return RS_OK;
}

RS uasc_response_send(uasc_session_t *s, pkt_t *pkt)
{
    if (s->status != UASCS_ESTABLISH){
        pkt_free(pkt);
        return RS_ERROR;
    }
    
    if (uasc_enc_pkt(s, pkt) != RS_OK) {
        pkt_free(pkt);
        return RS_ERROR;
    }
    
    uc_send_pkt_raw(s->sock, s->ip, s->port, pkt);
    
    pkt_free(pkt);
    
    return RS_OK;
}

/*
	删除头节点，触发下一节点的发送
*/
static void uasc_request_del(uasc_session_t *s)
{
	pkt_t *pkt;

	if (stlc_list_empty(&s->send_list))
		return;
	
	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	stlc_list_del(&pkt->link);
	pkt_free(pkt);

	CL_THREAD_OFF(s->t_send);
	
	if (stlc_list_empty(&s->send_list)) {
		if (s->status == UASCS_ESTABLISH) {
			CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, uasc_keeplive_timer, (void *)s, TIME_N_SECOND(s->time_param_cur->keeplive));
		}
		return;
	}

	s->send_retry = 0;
	CL_THREAD_TIMER_ON(MASTER, s->t_send, uasc_send_timer, (void *)s, 0);
}

static void uasc_hdr_order(uascph_t *hdr)
{
	hdr->client_sid = ntohl(hdr->client_sid);
	
	hdr->command = ntohs(hdr->command);
	hdr->param_len = ntohl(hdr->param_len);
    hdr->frag_total = ntohs(hdr->frag_total);
    hdr->frag_offset = ntohs(hdr->frag_offset);
    hdr->frag_ident = ntohs(hdr->frag_ident);
}

static pkt_t* pkt_clone(pkt_t* src)
{
    pkt_t* dest;
    int total_len;
    
    if (!src) {
        return NULL;
    }
    
    total_len = src->total+sizeof(pkt_t)+AES128_EKY_LEN;
    
    if ((dest = (pkt_t *)cl_calloc(total_len , 1)) == NULL) {
        log_err(false, "%s alloc %d failed\n", __FUNCTION__, total_len);
        return NULL;
    }
    
    memcpy(dest, src, sizeof(pkt_t)+src->total);
    
    STLC_INIT_LIST_HEAD(&dest->link);
    
    return dest;
}

pkt_t *uasc_pkt_new(int cmd, int param_len, 
			bool is_request, bool is_enc, u_int8_t flags,
			u_int32_t client_sid, u_int32_t device_sid, u_int8_t request_id)
{
	pkt_t *pkt;
	uascph_t *hdr;
	int total_len = sizeof(pkt_t) + uascph_hdr_size + param_len;

	// 多申请一块内存，用于加密时候用
	if ((pkt = (pkt_t *)cl_calloc(total_len + AES128_EKY_LEN, 1)) == NULL) {
		log_err(false, "%s alloc %d failed\n", __FUNCTION__, total_len);
		return NULL;
	}
	pkt->total = uascph_hdr_size + param_len;
	pkt->cmd = cmd;

	hdr = (uascph_t *)&pkt->data[0];
	
	hdr->ver = PROTO_VER1;
	hdr->hlen = uascph_hdr_size/4;
	hdr->request = is_request;
	hdr->encrypt = is_enc;
	hdr->flags = flags;
	hdr->client_sid = htonl(client_sid);
	hdr->request_id = request_id;
	
	hdr->command = htons(cmd);
	hdr->param_len = htonl(param_len);

	return pkt;
}

/*
	释放整个队列的所有报文
*/
void uasc_reset_send(uasc_session_t *s)
{
	pkt_t *pkt, *next;

	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_list, link) {
		stlc_list_del(&pkt->link);
		pkt_free(pkt);
	}
	s->send_retry = 0;
	CL_THREAD_OFF(s->t_send);
}

void uasc_reset_send_wait(uasc_session_t*s)
{
    pkt_t *pkt, *next;
    
    stlc_list_for_each_entry_safe(pkt_t, pkt, next, &s->send_wait_list, link) {
        stlc_list_del(&pkt->link);
        pkt_free(pkt);
    }
}

static int uasc_read(cl_thread_t *t)
{
	uasc_session_t *s;
	struct sockaddr_in addr;
	int n, pad_len;
	int addr_len;
	pkt_t *pkt;
	uascph_t *hdr;
	
	s = (uasc_session_t *)CL_THREAD_ARG(t);
	s->t_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_recv, uasc_read, (void *)s, s->sock);

	addr_len = sizeof(struct sockaddr_in);
	pkt = s->rcv_buf;

	n = pkt->total = (int)recvfrom(s->sock, (char *)pkt->data, MAX_UDP_PKT, 0,
			(struct sockaddr *)&addr, (socklen_t*)&addr_len);
	if (n <= (int)uascph_hdr_size) {
		log_err(true, "%s read udp failed n=%d\n", s->name,n);
		return 0;
	}

	hdr = (uascph_t *)pkt->data;

	// 1. 解密、检查长度合法性
	if (hdr->encrypt) {
		if (((n - uascph_hdr_plain_size)%AES128_EKY_LEN) != 0) {
			log_debug("Drop bad packet: n=%d, but encrypt=1\n", n);
			return 0;
		}
		dec_block((u_int8_t *)BOFP(hdr, uascph_hdr_plain_size), (n - uascph_hdr_plain_size), s->key);
		pad_len = ((u_int8_t *)hdr)[n - 1];
		if (pad_len > AES128_EKY_LEN || pad_len <= 0) {
			log_debug("Drop bad packet: encrypt pad=%d\n", pad_len);
			return 0;
		}
		pkt->total -= pad_len;
	}

	uasc_hdr_order(hdr);
	
	if (uascph_hdr_len(hdr) < uascph_hdr_size 
		|| pkt->total != hdr->param_len + uascph_hdr_len(hdr))
	{
		log_debug("Drop bad packet: total=%d, uascph_hdr_len=%d, param_len=%d\n", 
			pkt->total, uascph_hdr_len(hdr), hdr->param_len);
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
	log_debug("%s recv from %u.%u.%u.%u:%u: cmd=%u, param_len=%u, csid=%08x,is_enc = %u, is_frag = %s,frag_total=%u, frag_ident=%u, frag_offset=%u ,request id=%u(is %s)\n",s->name, IP_SHOW(s->ip), s->port, hdr->command, hdr->param_len,
		hdr->client_sid, hdr->encrypt,hdr->is_frag?"true":"false",hdr->frag_total,hdr->frag_ident,hdr->frag_offset,hdr->request_id,hdr->request ? "request" : "reply");
#endif
    //
	// 处理报文
	uasc_proc[s->status].proc_pkt(s);
#ifdef DBG_UDP_CTRL_PKT
	log_debug("The Packet Hand completed!\n");
#endif 
	
	return 0;
}

static void uasc_off_all_timer(uasc_session_t*s)
{
	//CL_THREAD_OFF(s->t_recv);
	CL_THREAD_OFF(s->t_send);
	CL_THREAD_OFF(s->t_keeplive);
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_OFF(s->t_timer);
}

static bool uasc_is_reset_pkt(uascph_t *hdr)
{
	uc_keeplive_reply_t *kr;
    
	if (hdr->command != CMD_UDP_KEEPLIVE)
		return false;
	
	kr = get_net_uascp_payload(hdr, uc_keeplive_reply_t);
    
	return (kr->action == UCA_RESET);
}

/***************************
	IDLE
 ***************************/

static int uasc_idle_timer(cl_thread_t *t)
{
	uasc_session_t *s = (uasc_session_t *)CL_THREAD_ARG(t);
	
	s->t_timer = NULL;
#if 1
	uasc_set_status(s, UASCS_DISPATCH);
#else
    s->ip = 0xA860047;
    s->port = 0xCAEB;
    uasc_set_status(s, UASCS_AUTH_REQ);
#endif

	return 0;
}
/************************************
	IDLE STAT
************************************/
static void uasc_idle_into(uasc_session_t *s)
{
	uasc_reset_send(s);

	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;

	uasc_off_all_timer(s);
	
	CL_THREAD_TIMER_ON(MASTER, s->t_timer, uasc_idle_timer, (void *)s, TIME_N_SECOND(s->idle_time));
	s->idle_time = DFL_UC_IDLE_TIME;
}

static void uasc_idle_out(uasc_session_t *s)
{
	// do none
}

static void uasc_idle_proc(uasc_session_t *s)
{
	// do none
}

/************************************
	DISPATCH STAT
************************************/

static void uasc_send_disp(uasc_session_t *s)
{
	pkt_t *pkt;
	uasc_req_disp_hdr_t *dh;
	int i, len, r;
	struct sockaddr_in addr;
    
	if(cl_priv->num_ip_disp_server == 0){
		log_info("uasc try send dispatch pkt,but disp dns ip is zero!\n");
		return;
	}

	// 统计查询次数
	s->try_count += 1;
    	
	//暂时直接产生数据包，后续有TLV数据时增加函数
	pkt = pkt_new(CMD_APP_SERVER_DISP, sizeof(uasc_req_disp_hdr_t),TP_USER);
	if (pkt == NULL) {
		return;
	}
	
	((net_header_t *)pkt->data)->handle = 0;
	
	dh = get_pkt_payload(pkt, uasc_req_disp_hdr_t);
	
//	dh->apptype = cl_priv->app_type & 0xF;
	dh->apptype = 3;	// 因为服务器那边只认类型3，so ...
	dh->time_xor = (u_int32_t)time(NULL);
	//时间取反
	dh->time_xor^=0xFFFFFFFF;
    dh->time_xor = htonl(dh->time_xor);
    dh->apptype = htons(dh->apptype);

	memset(&addr, 0, sizeof(addr));
	for (i = 0; i < (int)cl_priv->num_ip_disp_server; i++) {
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(cl_priv->ip_disp_server[i]);
		addr.sin_port = htons(DISP_SERVER_PORT);
		len = sizeof(addr);

		r = (int)sendto(s->disp_sock , pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len);
		log_debug("'%s' send dispacher request to %u.%u.%u.%u port %d, ret=%d\n",
			s->name, IP_SHOW(cl_priv->ip_disp_server[i]), DISP_SERVER_PORT, r);
	}

	pkt_free(pkt);
}

static int uasc_disp_timer(cl_thread_t *t)
{
	uasc_session_t *s = (uasc_session_t *)CL_THREAD_ARG(t);

	s->t_timer = NULL;

	if (s->try_count >= 20) {
		log_err(false, "try query disp count >= 10 failed\n");
		uasc_set_status(s, UASCS_ERROR);
		return 0;
	}
	
	CL_THREAD_TIMER_ON(&cl_priv->master, s->t_timer, uasc_disp_timer, (void *)s, TIME_TRY_DISP);

	uasc_send_disp(s);

	return 0;
}

static RS uasc_renew_disp_socket(uasc_session_t *s)
{
	if(!s){
		return RS_ERROR;
	}

	if(s->disp_sock != INVALID_SOCKET){
		CLOSE_SOCK(s->disp_sock);
	}
	
	s->disp_sock = create_udp_server(0, 0);
	
	if (s->disp_sock == INVALID_SOCKET) {
		if ((s->disp_sock = create_udp_server(0, 0)) == INVALID_SOCKET) {
			return RS_ERROR;
		}
	}
	
	return RS_OK;

}

static RS uasc_proc_disp_udp(uasc_session_t *s, pkt_t *pkt, struct sockaddr_in *peer_addr)
{
	net_header_t *hdr;
	uasc_disp_res_hdr_t* rh;
	int len;
	uc_tlv_t* tlv;

	hdr = (net_header_t *)pkt->data;
	rh = (uasc_disp_res_hdr_t*)(hdr+1);
	len = hdr->param_len;
	
	if(len < sizeof(*rh)){
		return RS_INVALID_PARAM;
	}
	
    rh->errorno = ntohl(rh->errorno);

	log_err(false, "get disp reply: %u.%u.%u.%u err=%u\n", IP_SHOW(ntohl(peer_addr->sin_addr.s_addr)), rh->errorno);
	
	if(rh->errorno != ERR_NONE){

#ifndef MUT_SERVER_ADAPT
	//TODO: 判断致命错误，告知APP
    log_err(false,"dispatch return error %u\n",rh->errorno);
    uasc_set_status(s,UASCS_ERROR);
	return RS_INVALID_PARAM;
#else
	return RS_OK;
#endif
	}

	if(rh->ip == 0 || rh->port == 0){
		//所有服务器都离线??
#ifndef MUT_SERVER_ADAPT        
		return RS_INVALID_PARAM;
#else
		return RS_OK;
#endif
	}

	s->ip = ntohl(rh->ip);
	s->port = ntohs(rh->port);

	len-=sizeof(*rh);
	
	//处理TLV
	if(len >=  sizeof(*tlv)){
		tlv = &rh->tlv[0];
		tlv->type = ntohs(tlv->type);
		tlv->len = ntohs(tlv->len);
		while (len >= sizeof(uc_tlv_t) && (u_int32_t)len >= sizeof(uc_tlv_t) + tlv->len) {
			len -= (sizeof(uc_tlv_t) + tlv->len);
			
			switch (tlv->type) {
			case 0:
				break;
				default:
					break;
			}
		}
	}	
	// 有目的IP 和端口了，连接
	uasc_set_status(s,UASCS_AUTH_REQ);
	
	return RS_OK;
}

static int uasc_disp_read(cl_thread_t *thread)
{
	uasc_session_t *s = (uasc_session_t *)CL_THREAD_ARG(thread);
	net_header_t *hdr;
	pkt_t *pkt;
	struct sockaddr_in addr;
	socklen_t len_addr;

	s->t_disp_recv = NULL;
	CL_THREAD_READ_ON(MASTER, s->t_disp_recv, uasc_disp_read, (void *)s, s->disp_sock);

	len_addr = sizeof(addr);
	pkt = s->rcv_buf;
	pkt->total = (int)recvfrom(s->disp_sock, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, &len_addr);
	if (pkt->total < (int)net_header_size) {
		return 0;
	}

	hdr_order(pkt);
	hdr = (net_header_t *)pkt->data;
	if (hdr->ver >= 2) {
		//分配服务器没有v2报文
		return 0;
	}	

	if( pkt->total < (int)(hdr->param_len + net_header_real_size(hdr))){
		log_err(false, "Ignore too short udp packet: %d  < %d\n", 
			pkt->total, (hdr->param_len + net_header_real_size(hdr)));
		return 0;
	}
    
	switch(hdr->command){
		case CMD_APP_SERVER_DISP:
			uasc_proc_disp_udp(s,pkt,&addr);
			break;
		default:
			break;
	}

	return 0;
}

static void uasc_disp_into(uasc_session_t *s)
{
	uasc_reset_send(s);

	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;

	uasc_off_all_timer(s);

	if(uasc_renew_disp_socket(s) == RS_OK){
		CL_THREAD_READ_ON(MASTER, s->t_disp_recv, uasc_disp_read, (void *)s, s->disp_sock);
		CL_THREAD_TIMER_ON(MASTER, s->t_timer, uasc_disp_timer,
			(void *)s, range_rand(0, 100));	
	}else{
		uasc_set_status(s,UASCS_IDLE);
	}
}

static void uasc_disp_out(uasc_session_t *s)
{
	uasc_off_all_timer(s);
	CL_THREAD_OFF(s->t_disp_recv);
	CLOSE_SOCK(s->disp_sock);
}

static void uasc_disp_proc(uasc_session_t *s)
{
	// do none
}

/***************************
	AUTH REQUEST
 ***************************/


static int auth_req_die(cl_thread_t *t)
{
	uasc_session_t *s = (uasc_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	cl_assert(s->status == UASCS_AUTH_REQ);

	log_info("%s UASCS_AUTH_REQ die\n", s->name);
	s->auth_lost++;

	if(s->auth_lost >= MAX_AUTH_LOST){
		if(uasc_udp_socket(s) != RS_OK){
			return 0;
		}
	}

    	uasc_set_status(s, UASCS_IDLE);
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

static void uasc_copy_user_info(uasc_session_t *s)
{

      s->time_param_cur = &s->time_param.wan_front;

    
    log_debug("%s uasc_copy_user_info  update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
              s->name,
              s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
              s->time_param_cur->keeplive, s->time_param_cur->die);

}

static void uasc_auth_req_into(uasc_session_t *s)
{
	pkt_t *pkt;
	uc_auth_request_t *ar;
	u_int32_t r1;

	// reset
	uasc_reset_send(s);
	s->client_sid = 0;
	s->device_sid = 0;
	s->my_request_id = 0;
	s->peer_request_id = 0;

	uasc_copy_user_info(s);
    
    //要重置超时时间
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, auth_req_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
    _uasc_reset_frag_list(s);

	// 地址和端口错误
	if(s->ip == 0 || s->port == 0){
		uasc_set_status(s,UASCS_IDLE);
		return;
	}

	// create new rand
	srand(get_sec());
	r1 = rand();
	memcpy(&s->rand1, &r1, sizeof(s->rand1));
	memset(s->rand2, 0, sizeof(s->rand2));
	// send auth.request
	pkt = uasc_pkt_new(CMD_UDP_AUTH, sizeof(uc_auth_request_t), 
				true, false, 0,	0, 0, 0);
	ar = get_uascp_payload(pkt, uc_auth_request_t);
	ar->action = UCA_REQUEST;
	ar->auth_flag |= (BIT(0) & 0xf);
	memcpy(ar->rand1, s->rand1, sizeof(ar->rand1));
	memcpy(ar->my_uuid, cl_priv->uuid_bin, sizeof(ar->my_uuid));

	fill_local_time(&ar->time);

	uasc_request_add(s, pkt);
}

static void uasc_auth_req_out(uasc_session_t *s)
{
	uasc_reset_send(s);
}

static void uasc_do_auth_redirect(uasc_session_t *s, uc_auth_redirect_t *rd)
{
	
	rd->ip = ntohl(rd->ip);
	rd->port = ntohs(rd->port);
	log_info("%s redirect to ip=%u.%u.%u.%u port=%u\n",
		s->name, IP_SHOW(rd->ip), rd->port);

	if(!rd->ip && !rd->port){
		// 设备离线,通知上层的话会停止登录，
		//设置用户为 IDLE ，会不断连接分配服务器，就这样，大网上20秒一次也没问题
        log_debug("%s at switch to idle direct ip %u.%u.%u.%u \n",s->name,IP_SHOW(s->ip));
		uasc_set_status(s, UASCS_IDLE);
  
		return;
	}
	
	s->ip = rd->ip;
	s->port = rd->port;

	uasc_set_status(s, UASCS_AUTH_REQ);
}

static void uasc_do_auth_question(uasc_session_t *s, uc_auth_question_t *qs)
{
	uc_auth_answer_t *ans;
	pkt_t *pkt;
	MD5_CTX ctx;

	uasc_set_status(s, UASCS_AUTH_ANSWER);

	memcpy(s->rand2, qs->rand2, sizeof(s->rand2));
    
	pkt = uasc_pkt_new(CMD_UDP_AUTH, sizeof(uc_auth_answer_t), 
				true, false, 0, s->client_sid, s->device_sid, 0);
	ans = get_uascp_payload(pkt, uc_auth_answer_t);
	ans->action = UCA_ANSWER;
	ans->support_enc =  htons(UC_ENC_AES128);

	// MD5(token1 + token2 + password_md5)
	MD5Init(&ctx);
	MD5Update(&ctx, s->rand1, sizeof(s->rand1));
	MD5Update(&ctx, s->rand2, sizeof(s->rand2));
	MD5Update(&ctx, s->md5_passwd, sizeof(s->md5_passwd));
	MD5Final(ans->answer, &ctx);
    
    mem_dump("s->rand1",s->rand1,sizeof(s->rand1));
    mem_dump("s->rand2",s->rand2, sizeof(s->rand2));
    
    mem_dump("passwd_md5",s->md5_passwd, sizeof(s->md5_passwd));
    mem_dump("ans->answer",ans->answer, sizeof(ans->answer));

	fill_local_time(&ans->time);

	uasc_request_add(s, pkt);
}

static void uasc_auth_req_proc(uasc_session_t *s)
{
	uascph_t *hdr;
	uc_auth_redirect_t *rd;
	uc_auth_question_t *qs;

	hdr = (uascph_t *)s->rcv_buf->data;
	if (hdr->command != CMD_UDP_AUTH) {
		log_debug("%s at %s ignore pkt cmd=%u\n", s->name, uasc_proc[s->status].name, hdr->command);
		return;
	}

	rd = get_uascp_payload(s->rcv_buf, uc_auth_redirect_t);
	switch (rd->action) {
	case UCA_REDIRECT:
		uasc_do_auth_redirect(s, rd);
		break;
	case UCA_QUESTION:
		s->client_sid = hdr->client_sid;
		qs = get_uascp_payload(s->rcv_buf, uc_auth_question_t);
		uasc_do_auth_question(s, qs);
		break;
	default:
		log_debug("%s at %s ignore pkt action=%u\n", s->name, uasc_proc[s->status].name, rd->action);
		return;
	}
}

/***************************
	AUTH ANSWER
 ***************************/

static int auth_answer_die(cl_thread_t *t)
{
	uasc_session_t *s = (uasc_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	cl_assert(s->status == UASCS_AUTH_ANSWER);

	log_info("%s UASCS_AUTH_ANSWER die\n", s->name);

    //回应超时了，可以不走分配服务器
	uasc_set_status(s, UASCS_AUTH_REQ);
	return 0;
}

static void uasc_auth_answer_into(uasc_session_t *s)
{
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, auth_answer_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
}

static void uasc_auth_answer_out(uasc_session_t *s)
{
	uasc_reset_send(s);
}

static RS do_tlv_time_param(uasc_session_t *s, uc_tlv_t *tlv)
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
		//和 服务器连接，只有前台
		s->time_param_cur = &s->time_param.wan_front;
		
	}

	log_debug("%s do_tlv_time_param [dev server %u.%u.%u.%u] update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
		s->name, IP_SHOW(s->ip),
		s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
		s->time_param_cur->keeplive, s->time_param_cur->die);

	return RS_OK;
}

static void do_auth_result(uasc_session_t *s, uascph_t *hdr, uc_auth_result_t *rs)
{
	MD5_CTX ctx;
	uc_tlv_t *tlv;
	int remain;
    
    if (hdr->param_len < sizeof(uc_auth_result_t)) {
        log_err(false, "bad auth result pkt: param_len=%u\n", hdr->param_len);
        return;
    }

	rs->select_enc = ntohs(rs->select_enc);
	rs->err_num = ntohs(rs->err_num);
	
	if (rs->err_num != 0) {
		log_err(false, "%s auth failed, result=%d\n", s->name, rs->err_num);
		
        	uasc_set_status(s, UASCS_ERROR);
		return;
	}

	s->my_request_id = s->peer_request_id = 0;

	s->select_enc = rs->select_enc;
//	// 认证请求方的加密秘钥使用 MD5(rand1 + rand2 + 验证凭证 + 验证凭证)
	MD5Init(&ctx);
	MD5Update(&ctx, s->rand1, sizeof(s->rand1));
	MD5Update(&ctx, s->rand2, sizeof(s->rand2));
	MD5Update(&ctx, s->md5_passwd, sizeof(s->md5_passwd));
	MD5Update(&ctx, s->md5_passwd, sizeof(s->md5_passwd));
	MD5Final(s->key, &ctx);
    
    mem_dump("key", s->key, sizeof(s->key));

	remain = hdr->param_len - sizeof(uc_auth_result_t);
	tlv = &rs->tlv[0];
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
		case UCT_TIME_PARAM:
			do_tlv_time_param(s, tlv);
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
	


	uasc_set_status(s, UASCS_ESTABLISH);
}

static void uasc_auth_answer_proc(uasc_session_t *s)
{
	uascph_t *hdr;
	uc_auth_result_t *rs;

	hdr = (uascph_t *)s->rcv_buf->data;
    //回应的时候要处理reset报文
    if (uasc_is_reset_pkt(hdr)) {
		log_info("%s reset by peer\n", s->name);
		uasc_set_status(s, UASCS_AUTH_REQ);
		return;
	}
    
	if (hdr->command != CMD_UDP_AUTH) {
		log_debug("%s at %s ignore pkt cmd=%u\n", s->name, uasc_proc[s->status].name, hdr->command);
		return;
	}

	rs = get_uascp_payload(s->rcv_buf, uc_auth_result_t);
	switch (rs->action) {
	case UCA_RESULT:
		do_auth_result(s, hdr, rs);
		break;
	default:
		log_debug("%s at %s ignore pkt action=%u\n", s->name, uasc_proc[s->status].name, rs->action);
		return;
	}
}


/***************************
	ESTABLISH
 ***************************/

static int uasc_estab_die(cl_thread_t *t)
{
	uasc_session_t *s = (uasc_session_t *)CL_THREAD_ARG(t);
	
	s->t_die = NULL;
	cl_assert(s->status == UASCS_ESTABLISH);

	log_info("%s UASCS_ESTABLISH die\n", s->name);

	uasc_set_status(s, UASCS_ERROR);
	
	return 0;
}

static void uasc_check_wait_queue(uasc_session_t* s)
{
    pkt_t* pkt;
    uascph_t* hdr;
    
    while (! stlc_list_empty(&s->send_wait_list)) {
        pkt = stlc_list_first_entry(&s->send_wait_list, pkt_t, link);
        if (pkt != NULL) {
            stlc_list_del_init(&pkt->link);
            
            hdr = (uascph_t *)&pkt->data[0];
            hdr->request_id = s->my_request_id;
            hdr->client_sid = htonl(s->client_sid);
            
            uasc_request_add(s, pkt);
        }
    }
}

static void uasc_estab_into(uasc_session_t *s)
{
//    void* pkt;
//    int len;
    
	CL_THREAD_OFF(s->t_die);
	CL_THREAD_TIMER_ON(MASTER, s->t_die, uasc_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));

	// 先快速发一个keeplive，获取一些参数
	CL_THREAD_OFF(s->t_keeplive);
	CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, uasc_keeplive_timer, (void *)s, 0);
    
//    len = 16384;
//    pkt = cl_calloc(len, 1);
//    if (pkt) {
//        uasc_request_add_by_data(s,CMD_UDP_APP_REPORT_ERROR,pkt,len,true,s->select_enc?true:false,0,s->client_sid);
//        cl_free(pkt);
//    }
//    
//
    uasc_check_wait_queue(s);

	uasc_event_push(UASC_CONNECT_OK,0);
}

static void uasc_estab_out(uasc_session_t *s)
{
	uasc_reset_send(s);
	uasc_off_all_timer(s);
    _uasc_reset_frag_list(s);
}

static int uasc_keeplive_timer(cl_thread_t *t)
{
	uasc_session_t *s;
	pkt_t *pkt;
	uc_keeplive_request_t *kr;

	s = (uasc_session_t *)CL_THREAD_ARG(t);
	s->t_keeplive = NULL;
	// 不着急设置下一个定时器，等本次的发完毕后，在uasc_request_del中设置
	//CL_THREAD_TIMER_ON(MASTER, s->t_keeplive, uasc_keeplive_timer, (void *)s, 0);

	// 发送队列目前还有报文等待发送，没必要再发保活报文了
	if ( ! stlc_list_empty(&s->send_list) )
		return 0;

	pkt = uasc_pkt_new(CMD_UDP_KEEPLIVE, sizeof(uc_keeplive_request_t), 
				true, true, 0,	s->client_sid, s->device_sid, s->my_request_id);
	kr = get_uascp_payload(pkt, uc_keeplive_request_t);
	kr->action = UCA_REQUEST;
	
	uasc_request_add(s, pkt);

	return 0;
}



static void uasc_send_push_response(uasc_session_t *s, ucp_ctrl_t* uc)
{
    pkt_t *pkt;
    
    ucp_obj_t* obj;
    
    pkt = uasc_pkt_new(CMD_TLV_UDP_CTRL, sizeof(ucp_ctrl_t) + sizeof(ucp_obj_t)
                     ,false, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->peer_request_id);
	uc = get_uascp_payload(pkt, ucp_ctrl_t);
    obj = (ucp_obj_t*)(uc + 1);
	uc->action = uc->action;
    if (uc->count > 0 ) {
        uc->count = 1;
        memcpy(obj, uc + 1, sizeof(*obj));
        obj->param_len = 0;
    } else {
        uc->count = 0;
    }
	
    log_info("uasc send PUSH PACKET to peer\n");

	uasc_response_send(s,pkt);
}


void uasc_do_udp_ctrl(user_t *user, u_int8_t *param, u_int16_t param_len)
{
	ucp_ctrl_t* uc;
    ucp_obj_t* obj;
    int total = param_len;
    int index;
    smart_appliance_ctrl_t* sac;
//    u_int16_t error;
	bool is_valid_dev = false;

    
    //不是智能终端
    if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
        return;
    }

	uc = (ucp_ctrl_t* )param;

	if (uc->count == 0) {
		log_err(false, "uasc udp ctrl :count == 0\n");
		return;
	}

	 total -= sizeof(*uc);
	 obj = (ucp_obj_t*)(uc + 1);

	 for (index = 0; index < uc->count && total >= sizeof(ucp_obj_t); index++) {
        ucp_obj_order(obj);		


		if (is_supported_udp_app_server((u_int8_t)obj->objct, (u_int8_t)obj->sub_objct, (u_int8_t)obj->attr) && (uc->action == UCA_GET || uc->action == UCA_PUSH)) {

            air_update_data_with_mib(sac->sub_ctrl, uc->action, obj);
        }
        
        obj = (ucp_obj_t*)((char*)(obj+1) + obj->param_len);
    }
}

/**
	处理是TLV的UDP CTRL报文
*/
static void uasc_do_tlv_udp_ctrl(uasc_session_t *s, uascph_t *hdr)
{
	uc_tlv_t* tlv = get_net_uascp_payload(hdr, uc_tlv_t);
	int remain = hdr->param_len;
	u_int64_t sn = 0;
	user_t *user = NULL;
	ucp_ctrl_t* uc = NULL;
    slave_t* slave = NULL;
    
//    udp_rf_update_server_obj_data

	if (remain < sizeof(*tlv)){
		return;
	}

	log_debug("uasc_do_tlv_udp_ctrl obj param len %u\n", hdr->param_len);

	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		
		switch (tlv->type) {
		case AS_TT_SN:
			if (tlv->len != sizeof(sn)) {
				break;
			}
			
			sn = ntoh_ll(*(u_int64_t*)tlv_val(tlv));
			log_debug("get tlv udp ctrl's sn %"PRIu64"\n", sn);

			if ((user = user_lookup_by_sn(sn)) == NULL) {
				log_err(false, "not found user by sn %"PRIu64"\n", sn);
                if ((slave = slave_lookup_by_ident(sn)) == NULL) {
                    goto done;
                }
			}

			
			
			break;
			
		case AS_TT_UDP_CTRL:
			if (!user&& !slave) {
				log_err(false, "not found user by sn %"PRIu64"\n", sn);
				break;
			}
			uc = (ucp_ctrl_t*)tlv_val(tlv);

			log_debug("uc count %u total len %u\n", uc->count, tlv->len);
            if (user) {
                uasc_do_udp_ctrl(user, tlv_val(tlv), tlv->len);
            }else if(slave){
                udp_rf_dev_do_server_uc(slave,(ucp_ctrl_t*)tlv_val(tlv),tlv->len);
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

done:
	// 如果是请求报文，一定要回复
	if (uc && hdr->request) {
		uasc_send_push_response(s, uc);
	}
}


static void uasc_do_keeplive(uasc_session_t *s, uascph_t *hdr)
{
	uc_keeplive_reply_t *kr;
//	uc_tlv_t *tlv;
//	int remain;

	if (hdr->param_len < sizeof(uc_keeplive_reply_t)) {
		log_err(false, "bad keeplive packet: param_len=%u\n", hdr->param_len);
		return;
	}
	
	kr = get_net_uascp_payload(hdr, uc_keeplive_reply_t);
	if (kr->action == UCA_RESET) {
		log_info("%s reset by peer\n", s->name);
		uasc_set_status(s, UASCS_AUTH_REQ);
		return;
	}

	if (kr->action != UCA_REPLY) {
		log_debug("%s ignore keeplive action=%u\n", kr->action);
		return;
	}

//	tlv = &kr->tlv[0];

//	remain = hdr->param_len - sizeof(uc_keeplive_reply_t);
//	tlv->type = ntohs(tlv->type);
//	tlv->len = ntohs(tlv->len);
//	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
//		switch (tlv->type) {
//		case UCT_TIME_PARAM:
//			do_tlv_time_param(s, tlv);
//			break;
//		}
//
//		remain -= (sizeof(uc_tlv_t) + tlv->len);
//		tlv = tlv_next(tlv);
//		if (remain >= sizeof(uc_tlv_t)) {
//			tlv->type = ntohs(tlv->type);
//			tlv->len = ntohs(tlv->len);
//		}
//	}
}

static RS uasc_check_request_id(uasc_session_t *s, uascph_t *request)
{
	if (request->request_id == s->peer_request_id) {
		return RS_OK;
	} else if (request->request_id == (u_int8_t)(s->peer_request_id + 1)) {
		// good, update to next
		log_info("%s update rquest id to %u\n", s->name, request->request_id);
		s->peer_request_id++;
		return RS_OK;
	}

	log_err(false, "%s ignore request pkt: reply id=%u, but local is %u.\n",
		s->name, request->request_id, s->peer_request_id);

	return RS_ERROR;
}

static RS uasc_check_reply_id(uasc_session_t *s, uascph_t *reply)
{
	pkt_t *pkt;
	uascph_t *request;
	
	if (stlc_list_empty(&s->send_list)) {
		log_err(false, "%s ignore reply pkt: no request in send list.\n", s->name);
		return RS_ERROR;
	}

	pkt = (pkt_t *)stlc_list_entry(s->send_list.next, pkt_t, link);
	request = (uascph_t *)&pkt->data[0];
	if (request->request_id != reply->request_id) {
		log_err(false, "%s ignore reply pkt: my request id=%u, but %u in pkt.\n",
			s->name, request->request_id, reply->request_id);
		return RS_ERROR;
	}

	return RS_OK;
}


static void uasc_estab_do_request(uasc_session_t *s, uascph_t *hdr)
{
	if (uasc_check_request_id(s, hdr) != RS_OK)
		return;

    //预处理
	switch (hdr->command) {
        case CMD_UDP_KEEPLIVE:
            uasc_do_keeplive(s, hdr);
                return;
		case CMD_TLV_UDP_CTRL:
			uasc_do_tlv_udp_ctrl(s, hdr);
            break;
    default:
        uasc_respose_frag_packet(s,hdr,ERR_NONE);
		break;
	}
    
}

static uasc_defrag_cache_t* check_add_defrag(uasc_defrag_cache_t* cache)
{
    int i;
    int len;
    uascph_t* hdr,*dhdr;
    pkt_t* dest,*cur;
    u_int8_t* content;
    
    if (!cache || !is_all_frag_ok(cache)) {
        return NULL;
    }
    
    dest = (pkt_t*)cl_calloc(cache->total_len + sizeof(pkt_t)+AES128_EKY_LEN, 1);
    if (!dest) {
        return NULL;
    }
    
    dhdr = (uascph_t*)&dest->data[0];
    content = (u_int8_t*)(dhdr+1);
    
    STLC_INIT_LIST_HEAD(&dest->link);
    
    for (i = 0; i < cache->total; i++) {
        cur = cache->pkts[i];
        hdr = (uascph_t*)cur->data;
        
        //copy head
        if (i == 0) {
            // pkt
            dest->cmd = cur->cmd;
            dest->up_current = cur->up_current;
            dest->up_total = cur->up_total;
            //hdr
            memcpy(dhdr, hdr, sizeof(*dhdr));
            dhdr->is_frag = false;
            // init header len
            dhdr->param_len = 0;
        }
        
        if(i == cache->total - 1){
            dhdr->request_id = hdr->request_id;
        }
        
        len = cur->total-sizeof(*hdr);
        log_debug("frag [%u] len = %u\n",i,len);
        // content
        memcpy(content, hdr+1, len);
        content = content + len;
        dhdr->param_len+=len;
    }
    
    cache->whole_pkt = dest;
    
    cache->is_defrag_successed = true;
    
    return cache;
}

static uasc_defrag_cache_t* uasc_add_frag_to_list(uasc_session_t* s,uascph_t *hdr)
{
    uasc_defrag_cache_t* cache,* whole;
    pkt_t * pkt;
    
    cache = lookup_cache_by_ident(hdr->frag_ident, &s->defrag_list);
    if (!cache) {
        cache = new_defrag_cache();
        if (!cache) {
            // rsponse packet ??
            uasc_respose_frag_packet(s,hdr,ERR_NONE);
            return NULL;
        }
        
        cache->frag_ident = hdr->frag_ident;
        cache->total = hdr->frag_total;
        stlc_list_add(&cache->link, &s->defrag_list);
    }
    
    
    if(cache->pkts[hdr->frag_offset] != NULL){
        //repeat packet, rsponse packet ??
        uasc_respose_frag_packet(s,hdr,ERR_NONE);
        return NULL;
    }
    
    pkt = pkt_clone(s->rcv_buf);
    if (!pkt) {
        uasc_respose_frag_packet(s,hdr,ERR_NONE);
        return NULL;
    }
    
    cache->pkts[hdr->frag_offset] = pkt;
    cache->frag_recved++;
    cache->total_len+=pkt->total;
    
    whole = check_add_defrag(cache);
    
    if (!whole) {
        uasc_respose_frag_packet(s,hdr,ERR_NONE);
        return NULL;
    }
    
    if (whole && !whole->is_defrag_successed) {
        // err fragment cache
        stlc_list_del(&whole->link);
        _uasc_frag_free(whole);
        //
        uasc_respose_frag_packet(s,hdr,ERR_NONE);
        return NULL;
    }
    
    return whole;
}

static void uasc_proc_resp_pkt(uasc_session_t *s,uascph_t *hdr)
{
    
    switch (hdr->command) {
            
        case CMD_UDP_KEEPLIVE:
            uasc_do_keeplive(s, hdr);
            break;
        case CMD_UDP_APP_REPORT_RUNENV:
            uasc_event_push(UASC_PUSH_STAT_OK, 0);
            break;
		case CMD_TLV_UDP_CTRL:
			uasc_do_tlv_udp_ctrl(s, hdr);
			break;
        default:
            log_debug("%s at %s ignore reply pkt cmd=%u\n", s->name, uasc_proc[s->status].name, hdr->command);
            return;
            break;
    }
    
    // 删除掉已经应答的报文
    uasc_request_del(s);
    
    // 在处理完报文再设置死亡时间。因为可能修改了死亡时间参数
    if (s->status == UASCS_ESTABLISH) {
        CL_THREAD_OFF(s->t_die);
        CL_THREAD_TIMER_ON(MASTER, s->t_die, uasc_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
    }
}


static void uasc_proc_frag_pkt(uasc_session_t *s,uascph_t *hdr)
{
    uasc_defrag_cache_t * dc;
    
    if (hdr->frag_offset > hdr->frag_total) {
        log_err(false,"Error frag packet ! cmd[%u] total[%u] offset[%u]\n",hdr->command,hdr->frag_total,hdr->frag_offset);
        return;
    }
    
    if (hdr->request) {
        // check request id
        if (uasc_check_request_id(s, hdr) != RS_OK)
            return ;
    }else{
        // id error
        if (uasc_check_reply_id(s, hdr) != RS_OK) {
            return ;
        }
    }
    
    dc = uasc_add_frag_to_list(s,hdr);
    if (!dc) {
        return;
    }
    
    
    stlc_list_del(&dc->link);
    hdr = (uascph_t*)(&dc->whole_pkt->data[0]);
    log_debug("defrag a pkt! command=%u total len = %u frag total = %u\n",hdr->command,hdr->param_len,hdr->frag_total);
    if (hdr->request) {
        uasc_estab_do_request(s, hdr);
        goto end;
    }
    
    uasc_proc_resp_pkt(s,hdr);
    
    
end:
    _uasc_frag_free(dc);
}

static void uasc_estab_proc(uasc_session_t *s)
{
	uascph_t *hdr;
    uasc_error_t* ue;

	hdr = (uascph_t *)s->rcv_buf->data;
    ue = (uasc_error_t*)(hdr+1);

	// check if it's keeplive.reset
	if (uasc_is_reset_pkt(hdr)) {
		log_info("%s reset by peer\n", s->name);
		uasc_set_status(s, UASCS_AUTH_REQ);
		return;
	}
    
    if(hdr->is_frag_resp){
        uasc_request_del(s);
        
        if (s->status == UASCS_ESTABLISH) {
            CL_THREAD_OFF(s->t_die);
            CL_THREAD_TIMER_ON(MASTER, s->t_die, uasc_estab_die, (void *)s, TIME_N_SECOND(s->time_param_cur->die));
        }
        return;
    }

    //process fragment packet
    if (hdr->is_frag ) {
        if (hdr->frag_total > UASC_MAX_FRAG_PET_PKT) {
            // too big,not support
            return;
        }
        
        if((hdr->frag_total > 1) && (hdr->frag_total <= UASC_MAX_FRAG_PET_PKT)){
            uasc_proc_frag_pkt(s,hdr);
            return;
        }
        
        // remain frag_total == 0 or frag_total == 0x1  equal single packet
        
    }
	// process request packet
	if (hdr->request) {
		uasc_estab_do_request(s, hdr);
		return;
	}

	/*
		bellow process reply packet
	*/
	
	if (uasc_check_reply_id(s, hdr) != RS_OK) {
		return;
	}
    
    uasc_proc_resp_pkt(s,hdr);
	
}

/***************************
	ERROR
 ***************************/
static void uasc_error_into(uasc_session_t *s)
{
	uasc_reset_send(s);
    uasc_reset_send_wait(s);
	uasc_off_all_timer(s);
}

static void uasc_error_out(uasc_session_t *s)
{
	// do nothing
}

static void uasc_error_proc(uasc_session_t *s)
{
	uascph_t *hdr;

	hdr = (uascph_t *)s->rcv_buf->data;

	// do nothing
	log_debug("%s at %s ignore reply pkt cmd=%u\n", s->name, uasc_proc[s->status].name, hdr->command);
}

/*
	FSM
*/
ucc_proc_t uasc_proc[UASCS_MAX] = {
	{"IDLE", uasc_idle_into, uasc_idle_out, uasc_idle_proc},
	{"DISPATCH", uasc_disp_into, uasc_disp_out, uasc_disp_proc},
	{"AUTH_REQ", uasc_auth_req_into, uasc_auth_req_out, uasc_auth_req_proc},
	{"AUTH_ANSWER", uasc_auth_answer_into, uasc_auth_answer_out, uasc_auth_answer_proc},
	{"ESTAB", uasc_estab_into, uasc_estab_out, uasc_estab_proc},
	{"ERROR", uasc_error_into, uasc_error_out, uasc_error_proc},
};

void uasc_set_status(uasc_session_t *s, int status)
{
	int prev_satus = s->status;
    
	if (status >= UASCS_MAX) {
		log_err(false, "uasc_set_status unknow new status = %d\n", status);
		return;
	}

	s->status = status;
    
	log_info("%s modify status from %s to %s\n",
		s->name, uasc_proc[prev_satus].name, uasc_proc[status].name);

	uasc_proc[prev_satus].on_out(s);
	uasc_proc[status].on_into(s);
}

RS uasc_udp_socket(uasc_session_t *s)
{
	if(!s){
		return RS_ERROR;
	}

	s->auth_lost = 0;
	if(s->sock != INVALID_SOCKET){
		CLOSE_SOCK(s->sock);
		ucc_free_port(s->my_port);
	}
	
	s->my_port = ucc_get_port(0, s->name);
	s->sock = create_udp_server(0, s->my_port);

	log_info("UASC %s  try use local udp port=%u\n", s->name, s->my_port);
	
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
uasc_session_t*  uasc_new()
{
	uasc_session_t *s;
    MD5_CTX ctx;
	
	s = (uasc_session_t *)cl_calloc(sizeof(uasc_session_t), 1);

	s->status = UASCS_IDLE;
	s->idle_time = 0;

	// init default time parament
	uc_set_default_time_param(&s->time_param, &s->time_param_net);
    
	s->time_param_cur = &s->time_param.wan_front;
    MD5Init(&ctx);
    MD5Update(&ctx, (u_int8_t*)share_key, (unsigned int)strlen(share_key));
    MD5Final(s->md5_passwd, &ctx);
    
	
    
    log_debug("%s update time paramt: retry = %u, %u, %u (100ms), keeplive=%us, die=%us\n",
              s->name, 
              s->time_param_cur->retry_100ms[0], s->time_param_cur->retry_100ms[1], s->time_param_cur->retry_100ms[2],
              s->time_param_cur->keeplive, s->time_param_cur->die);
	
	s->port = DFL_UDP_CTRL_CLIENT_WAN_PORT;
	s->sock = INVALID_SOCKET;
	s->disp_sock = INVALID_SOCKET;
    s->rand_num = rand()%100000;

	if (uasc_udp_socket(s) != RS_OK) {
		cl_free(s->name);
		cl_free(s);
		return NULL;
	}

	STLC_INIT_LIST_HEAD(&s->send_list);
    STLC_INIT_LIST_HEAD(&s->send_wait_list);
    STLC_INIT_LIST_HEAD(&s->defrag_list);
	s->rcv_buf = pkt_new(0, MAX_UDP_PKT, 0);

#if 1
	uasc_set_status(s, UASCS_ERROR);

#else
	s->ip = ntohl(inet_addr(UAS_TEST_IP));
	s->port = UAS_TEST_PORT;

	uasc_set_status(s, UASCS_AUTH_REQ);
#endif
	CL_THREAD_READ_ON(MASTER, s->t_recv, uasc_read, (void *)s, s->sock);

	return s;
}

void uasc_free(uasc_session_t *s)
{

	if (!s)
		return;

	uasc_reset_send(s);
    uasc_reset_send_wait(s);
	uasc_off_all_timer(s);
    _uasc_reset_frag_list(s);
	CL_THREAD_OFF(s->t_recv);
	
	pkt_free(s->rcv_buf);
	CLOSE_SOCK(s->sock);
	CLOSE_SOCK(s->disp_sock);
	ucc_free_port(s->my_port);

	cl_free(s);
}


