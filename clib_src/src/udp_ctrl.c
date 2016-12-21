#include "client_lib.h"
#include "cl_priv.h"
#include "aes.h"
#include "md5.h"

#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "uc_client.h"


/*
	alloc udp ctrl packet.
	user 'pkt_free()' function to free it
*/

static pkt_t * uc_pkt_new_v2(ucc_session_t* s,int cmd, int param_len,
                             bool is_request, bool is_enc, u_int8_t flags,
                             u_int32_t client_sid, u_int32_t device_sid, u_int32_t request_id)
{
    pkt_t *pkt;
    ucph_v2_t *hdr;
    int total_len = sizeof(pkt_t) + ucph_v2_hdr_size + param_len;
    
    // 多申请一块内存，用于加密时候用
    if ((pkt = (pkt_t *)cl_calloc(total_len + AES128_EKY_LEN, 1)) == NULL) {
        log_err(false, "%s alloc %d failed\n", __FUNCTION__, total_len);
        return NULL;
    }
    pkt->total = ucph_v2_hdr_size + param_len;
    pkt->cmd = cmd;
    
    hdr = (ucph_v2_t *)&pkt->data[0];
    
    hdr->ver = PROTO_VER2;
    hdr->hlen = ucph_v2_hdr_size/4;
    hdr->request = is_request;
    hdr->encrypt = is_enc;
    hdr->flags = flags;
    hdr->client_sid = htonl(client_sid);
    hdr->device_sid = htonl(device_sid);
    hdr->request_id = request_id & 0xFF;
    hdr->u32_req_id = htonl(request_id);
    
    hdr->command = htons(cmd);
    hdr->param_len = htons(param_len);
    
    return pkt;
}

static u_int32_t pkt_gen_seq(void)
{
	static u_int32_t seq = 1;

	return seq++;
}

pkt_t *uc_pkt_new(void* s,int cmd, int param_len,
			bool is_request, bool is_enc, u_int8_t flags,
			u_int32_t client_sid, u_int32_t device_sid, u_int32_t request_id)
{
	pkt_t *pkt;
	ucph_t *hdr;
	int total_len ;
    ucc_session_t* us = (ucc_session_t*)s;
    
    
    if (us != NULL && us->has_share_key && us->status == UCCS_ESTABLISH) {
        return uc_pkt_new_v2(us, cmd, param_len, is_request, is_enc, flags, client_sid, device_sid, request_id);
    }

    total_len = sizeof(pkt_t) + ucph_hdr_size + param_len;
	// 多申请一块内存，用于加密时候用
	if ((pkt = (pkt_t *)cl_calloc(total_len + AES128_EKY_LEN, 1)) == NULL) {
		log_err(false, "%s alloc %d failed\n", __FUNCTION__, total_len);
		return NULL;
	}
	pkt->total = ucph_hdr_size + param_len;
	pkt->cmd = cmd;
	pkt->seq = pkt_gen_seq();

	hdr = (ucph_t *)&pkt->data[0];
	
	hdr->ver = PROTO_VER1;
	hdr->hlen = ucph_hdr_size/4;
	hdr->request = is_request;
	hdr->encrypt = is_enc;
	hdr->flags = flags;
	hdr->client_sid = htonl(client_sid);
	hdr->device_sid = htonl(device_sid);
	hdr->request_id = request_id & 0xFF;
	
	hdr->command = htons(cmd);
	hdr->param_len = htons(param_len);

	return pkt;
}



void uc_hdr_order(ucph_t *hdr)
{
    ucph_v2_t* hdr_v2;
    
	hdr->client_sid = ntohl(hdr->client_sid);
	hdr->device_sid = ntohl(hdr->device_sid);
	
	hdr->command = ntohs(hdr->command);
	hdr->param_len = ntohs(hdr->param_len);
    
    if (hdr->ver == 2 && (hdr->hlen << 2) == sizeof(ucph_v2_t)) {
        hdr_v2 = (ucph_v2_t*)hdr;
        hdr_v2->u32_req_id = htonl(hdr_v2->u32_req_id);
    }
    
}


/*
	retyr0、1、2的单位为100毫秒
	keeplive和die的单位为秒
*/
void uc_set_time_param(uc_time_param_item_t *it,
		u_int8_t retry0, u_int8_t retry1, u_int8_t retry2,
		u_int8_t keeplive, u_int16_t die)
{
	it->retry_100ms[0] = retry0;
	it->retry_100ms[1] = retry1;
	it->retry_100ms[2] = retry2;

	it->keeplive = keeplive;
	it->die = die;
}

void uc_set_default_time_param(uc_time_param_all_t *time_param, uc_time_param_all_t *time_param_net)
{
	// 设备与服务器通信	1秒	3秒	3秒，20秒	30秒
	uc_set_time_param(&time_param->dev, 10, 30, 30, 20, 30);
	// 手机与设备局域网连接	0.3秒	1秒	1秒，3秒	5秒
	if (cl_priv->is_pt_mode) { /* 产测模式 */
		uc_set_time_param(&time_param->lan, 1, 3, 3, 3, 10);
	} else {
		uc_set_time_param(&time_param->lan, 3, 10, 10, 3, 10);
	}
	// 手机与设备广域网前台	1秒	2秒	3秒，20秒	5分钟
	uc_set_time_param(&time_param->wan_front, 10, 20, 30, 20, 300);
	// 手机与设备广域网后台	1秒	5秒	20秒，2分钟	5分钟
	uc_set_time_param(&time_param->wan_background, 10, 50, 100, 120, 300);

	memcpy(time_param_net, time_param, sizeof(uc_time_param_all_t));
	time_param_net->dev.die = htons(time_param_net->dev.die);
	time_param_net->lan.die = htons(time_param_net->lan.die);
	time_param_net->wan_front.die = htons(time_param_net->wan_front.die);
	time_param_net->wan_background.die = htons(time_param_net->wan_background.die);
}

int uc_send_pkt_raw(SOCKET sock, u_int32_t ip, u_int16_t port, pkt_t *pkt)
{
	int n;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	n = (int)sendto(sock, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, sizeof(addr));

	return n;
}

int uc_send_data_raw(SOCKET sock, u_int32_t ip, u_int16_t port, void *data, int len)
{
	int n;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	n = (int)sendto(sock, data, len, 0, (struct sockaddr *)&addr, sizeof(addr));

	return n;
}

void order_uc_time(uc_time_t *t)
{
	t->year = ntohs(t->year);
}

void gen_rand_block(u_int8_t *dst, int size)
{
	int i;
	MD5_CTX ctx;
	u_int32_t v;
	u_int8_t block[16];

	for (i = 0; i < (size + 15)/16; i++, dst += 16) {
		MD5Init(&ctx);
		
		v = get_msec();
		MD5Update(&ctx, (u_int8_t *)&v, sizeof(v));

		v = rand();
		MD5Update(&ctx, (u_int8_t *)&v, sizeof(v));

		v = (u_int32_t)&v;
		MD5Update(&ctx, (u_int8_t *)&v, sizeof(v));
		
		MD5Final(block, &ctx);

		v = min(16, size-(i<<4));
		memcpy(dst, block, v);
	}
}

RS tlvm_init(tlv_manger_t* m)
{
	if(!m){
		return RS_ERROR;
	}

	memset(m,0x0,sizeof(*m));
	m->tlv_data = cl_calloc(MAX_TLVM_LENGTH, 1);
	if(!m->tlv_data){
		return RS_ERROR;
	}
	
	m->total = MAX_TLVM_LENGTH;
	m->cur = m->tlv_data;

	return RS_OK;
}

RS tlvm_free_data(tlv_manger_t* m)
{
	if(!m){
		return RS_ERROR;
	}
	if(m->tlv_data){
		cl_free(m->tlv_data);
	}	

	memset(m,0,sizeof(*m));	

	return RS_OK;
}

RS tlvm_add_tlv(tlv_manger_t* m,u_int16_t type,u_int16_t len,void* data)
{
	uc_tlv_t* tlv;
	void* dest;

	if(!m || !m->tlv_data || !m->cur || !data || len == 0){
		return RS_ERROR;
	}
	
	if(m->total - m->used_len - sizeof(*tlv) < len){
		return RS_ERROR;
	}

	tlv = (uc_tlv_t*)m->cur;
	tlv->type = htons(type);
	tlv->len = htons(len);

	dest = (void*)(tlv+1);
	memcpy(dest,data,len);
	
	m->cur = m->cur+(len+sizeof(*tlv));
	m->used_len+=(sizeof(*tlv)+len);
	
	return RS_OK;
}

RS tlvm_add_u8_tlv(tlv_manger_t* m,u_int16_t type,u_int8_t u8_data)
{
	return tlvm_add_tlv(m,type,sizeof(u8_data),&u8_data);
}

RS tlvm_add_u16_tlv(tlv_manger_t* m,u_int16_t type,u_int16_t u16_data,bool sw_bit_order)
{
	if(sw_bit_order){
		u16_data = htons(u16_data);
	}
	return tlvm_add_tlv(m,type,sizeof(u16_data),&u16_data);
}

RS tlvm_add_u32_tlv(tlv_manger_t* m,u_int16_t type,u_int32_t u32_data,bool sw_bit_order)
{
	if(sw_bit_order){
		u32_data = htonl(u32_data);
	}
	return tlvm_add_tlv(m,type,sizeof(u32_data),&u32_data);
}

RS tlvm_add_string_tlv(tlv_manger_t* m,u_int16_t type,char* string)
{
	int len;
	
	if(!m || !m->tlv_data || !m->cur || !string){
		return RS_ERROR;
	}
	
	len = (int)strlen(string);
	if(len == 0){
		return RS_ERROR;
	}else{
		len = (len+3)&0xFFFFFFFC;//4字节对齐
	}

	return tlvm_add_tlv(m,type,len,string);	
}

