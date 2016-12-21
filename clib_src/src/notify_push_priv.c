#include "cl_priv.h"
#include "notify_push_priv.h"
#include "wait_server.h"
#include "cl_community.h"
#include "community_priv.h"
#include "phone_user.h"
#include "health_priv.h"

static void msglist_free(alarm_msg_list_t* list);

static RS notify_push_set_expect_id(user_t * user,cln_notify_push_t* param)
{
    pkt_t *pkt;
    net_alarm_log_query_t* query;
    notify_push_t* np = user->np;
    
    cl_assert(np && param);
    
    if (user->is_udp_ctrl) {
        return RS_OK;
    }
    
    np->expect_report_id = param->expect_report_id;
    np->callback = param->callback;
    np->is_user_has_set_id = 0x1;
    np->callback_handle = param->callback_handle;
    if (np->expect_report_id >= np->android_push_id) {
        np->android_push_id = np->expect_report_id + 1;
    }
    
    /*允许取消*/
    if (np->callback == NULL) {
        return RS_OK;
    }
	if(user->is_phone_user)
		return pu_set_notify_expect_id(user,param);
    
    pkt = pkt_new_v2(CMD_ALARM_LOG, sizeof(net_alarm_log_query_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
    if (!pkt) {
        /*内存分配失败，需要修改错误值*/
        return RS_OK;
    }
    query = get_pkt_payload(pkt, net_alarm_log_query_t);
    query->alarm_header.action = ALARM_LOG_ACT_QUERY;
    
    user_add_pkt(user, pkt);
    
    return RS_OK;
}

static alarm_msg_list_t* alarm_log_pkt2msglist(notify_push_t * np,pkt_t *pkt)
{
    alarm_log_head_t* head;
    alarm_log_t* log;
    alarm_msg_list_t* list = NULL;
    int length,count,i,tmplen,k;
    alarm_msg_t * msg;
    u_int64_t expect_id;
    u_int32_t uuid = 0;
    u_int64_t max_uuid = 0;
    
    cl_assert(np && pkt);
    
    head = get_pkt_payload(pkt, alarm_log_head_t);
    if (head->log_num == 0) {
        return list;
    }
    
    /*先查找有多少个符合要求的*/
    if (np->expect_report_id == 0) {
        count = ntohs(head->log_num);
        goto output;
    }else{
        count = 0;
    }
    if (np->need_notify_user) {
        expect_id = np->android_push_id;
    }else{
        expect_id = np->expect_report_id;
    }
    
    length = pkt->total-sizeof(alarm_log_head_t);
    log = (alarm_log_t*)(head+1);
    for (i=0; i < ntohs(head->log_num) && length>=sizeof(alarm_log_t); i++) {
        if (ntohl(log->log_uuid) >= expect_id) {
            count++;
        }
        tmplen = log->alarm_msg_len+log->alarm_name_len+log->alarm_phone_count*16;
        length-=tmplen;
        log = (alarm_log_t*)(log->alarm_name+tmplen);
    }
    
    /*没有符合要求的*/
    if (count == 0) {
        return list;
    }

output:
    length = pkt->total-sizeof(alarm_log_head_t);
    log = (alarm_log_t*)(head+1);
    
    list = cl_calloc(sizeof(*list), 1);
    if (!list) {
        log_err(false, "%s: malloc memory failed\n", __FUNCTION__);
        return list;
    }
    
    msg = cl_calloc(sizeof(alarm_msg_t), count);
    if (!msg) {
        log_err(false, "%s: malloc memory failed\n", __FUNCTION__);
        cl_free(list);
        return NULL;
    }
    
    k=0;
    expect_id = np->expect_report_id;
    length = pkt->total-sizeof(alarm_log_head_t);
    log = (alarm_log_t*)(head+1);
    for (i=0; i < ntohs(head->log_num) && length>=sizeof(alarm_log_t); i++) {
        uuid = ntohl(log->log_uuid);
        if (uuid >= expect_id) {
            msg[k].alarm_id = ntohs(log->local_id);
            msg[k].alarm_time = ntohl(log->log_time);
            msg[k].alarm_duration = ntohs(log->log_last_time);
            msg[k].alarm_type = ntohs(log->alarm_factoryid);
            msg[k].len_name = log->alarm_name_len;
            msg[k].len_msg = log->alarm_msg_len;
            
            if (msg[k].len_name) {
                tmplen = log->alarm_name_len>=sizeof(msg[k].arlam_name)?sizeof(msg[k].arlam_name)-1:log->alarm_name_len;
                memcpy(msg[k].arlam_name, log->alarm_name,tmplen);
            }
            /*单字节最多25
             6，不检查长度了*/
            if (msg[k].len_msg) {
                memcpy(msg[k].alarm_msg, log->alarm_name+log->alarm_name_len, log->alarm_msg_len);
            }
            /*有多条报警情况下，服务器发过来的是升序，设备的是降序*/
            if (k==0) {
                list->first_report_id = uuid;
            } else {
                if (list->first_report_id > uuid)
                    list->first_report_id = uuid;
            }
            if ((u_int32_t)np->expect_report_id < uuid)
                np->expect_report_id = (u_int64_t)uuid;
            list->count++;
            k++;
            if (uuid > max_uuid) {
                max_uuid = uuid;
            }
        }
        tmplen = log->alarm_msg_len+log->alarm_name_len+log->alarm_phone_count*16;
        length-=tmplen;
        log = (alarm_log_t*)(log->alarm_name+tmplen);
    }
    
    list->msg = msg;
    np->android_push_id = max_uuid + 1;
    
    return list;
}

// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
bool notify_push_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
    switch (hdr->command) {
	case CMD_ALARM_LOG:
        {
            alarm_log_head_t* head = get_pkt_payload(pkt, alarm_log_head_t);
            alarm_msg_list_t* ml;
            if (pkt->total<sizeof(alarm_log_head_t)||
                head->err!=ERR_NONE) {
                /*出错了,要不要回调呢??*/
                return true;
            }
            
            if (head->action == ALARM_LOG_ACT_QUERY||
                head->action == ALARM_LOG_ACT_PUSH) {
                
                //推送或查询，若应用层设置了回调函数，就处理报文，否则丢弃
                if (user->np && user->np->callback) {
                    ml = alarm_log_pkt2msglist(user->np,pkt);
			if(ml)
				ml->dev_sn = user->sn;
                    msglist_free(user->np->msglist);
                    user->np->msglist = ml;
                    /*推送消息事件，即使日志列表是空也推送*/
                    if (head->action == ALARM_LOG_ACT_PUSH||user->np->need_notify_user) {
                        event_push(user->np->callback, NE_ALARM_PUSH_LOG, user->handle, user->np->callback_handle);
                    }else{
                        event_push(user->np->callback, NE_ALARM_LOG, user->handle, user->np->callback_handle);
                    }
                }
            }
            
            if (user->np) {
                user->np->need_notify_user = 0;
            }
        }
		break;

    default:
		return false;
	}

	return true;
}

static void send_notify_ack(user_t *user, net_notify_t *notify, struct sockaddr_in *addr, u_int8_t result)
{
	net_notify_result_t *nr;
	pkt_t *pkt = NULL;

	if(addr == NULL)
		return;/*tcp不发送notify result*/
	pkt = pkt_new_v2(CMD_NOTIFY_RESULT, sizeof(*nr), 0, user->sn, user->ds_type);
	if(pkt == NULL)
		return;
	nr = get_pkt_payload(pkt, net_notify_result_t);
	nr->first_report_id = ntohl(notify->first_report_id);
	nr->report_count = notify->report_count;
	nr->result = result;
	nr->reserved[0] = 0;
	nr->reserved[1] = 0;

	if(addr){
		sendto(user->sock_udp, pkt->data, pkt->total, 0, (struct sockaddr *)addr, sizeof(*addr ));
#ifdef SUPPORT_TRAFFIC_STAT
        UDP_PKT_STAT(false, pkt->total);
#endif
		pkt_free(pkt);
	}else{
		user_add_pkt(user, pkt);
	}
		
}

static RS _cmt_recv_mesure_trans(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	net_fm_t *fm;
	net_messure_t *m;
	cl_measure_t *clm = NULL;
	user_cmt_event_t *evt = NULL;
	
	clm = cl_calloc(sizeof(*clm), 1);
	evt = cl_calloc(sizeof(*evt), 1);
	if (clm == NULL || evt == NULL)
		goto errorout;
	
	fm = (net_fm_t*)tlv->value;
	m = (net_messure_t*)&tlv->value[sizeof(*fm)];
	clm->report_id = report_id;
	clm->dev_sn = ((net_header_v2_t*)hdr)->sn;
	fm_to_local(fm, &clm->member);
	if(parse_measure_data(&clm->measure, m) != RS_OK)
		goto errorout;
	
	evt->msg = clm;
	evt->event = CE_MESURE;
	stlc_list_add_tail(&evt->link, &user->user_cmt->mesure_head);
	if(event_push(user->callback, evt->event, user->handle, user->callback_handle) != RS_OK){
		stlc_list_del(&evt->link);
		goto errorout;
	}
	
	return RS_OK;
	
errorout:
	SAFE_FREE(clm);
	SAFE_FREE(evt);
	return RS_ERROR;
}

static RS _cli_recv_mesure_trans(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	net_fm_t *fm;
	net_messure_t *m;
	measure_list_t *ml = NULL;
	list_link_t *lk = NULL;

	user_health_t *uh = (user_health_t*)user->health;

	if (uh == NULL)
		goto errorout;
	
	ml = cl_calloc(sizeof(*ml), 1);
	lk = cl_calloc(sizeof(*lk), 1);
	if (ml == NULL || lk == NULL)
		goto errorout;
	ml->count = 1;
	ml->list = cl_calloc(sizeof(measure_t), 1);
	if(ml->list == NULL)
		goto errorout;
	
	fm = (net_fm_t*)tlv->value;
	m = (net_messure_t*)&tlv->value[sizeof(*fm)];

	if(parse_measure_data(ml->list, m) != RS_OK)
		goto errorout;
	
	lk->p = ml;
	stlc_list_add_tail(&lk->node, &uh->measure_hd);
	if (event_push(user->callback, HE_MEASURE_QUERY_OK,  user->handle, user->callback_handle) != RS_OK) {
		stlc_list_del(&lk->node);
		goto errorout;
	}	

	return RS_OK;
	
errorout:
	if (ml)
		SAFE_FREE(ml->list);
	SAFE_FREE(ml);
	SAFE_FREE(lk);
	return RS_ERROR;
}

static RS _recv_mesure_trans(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	if (tlv->length < (sizeof(net_fm_t) + sizeof(net_messure_t))){
		return RS_ERROR;
	}
	if (user->ds_type == TP_DS007) 
		return _cmt_recv_mesure_trans(user, tlv, report_id, hdr);
	else
		return _cli_recv_mesure_trans(user, tlv, report_id, hdr);
}

static RS _recv_alarm_log(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	u_int32_t left, current, index;
	alarm_log_head_t *alarm_hd;	
	alarm_log_t *alarm;

	alarm_msg_list_t *list = NULL;
	alarm_msg_t *msg = NULL;
	user_cmt_event_t *evt = NULL;

	if(tlv->length < sizeof(*alarm_hd)){
		log_debug("%s, device %012llu,drop for tlv length is less than alarm log head: %u < %u\r\n",
			__FUNCTION__, ((net_header_v2_t*)hdr)->sn, tlv->length, sizeof(*alarm_hd));
		return RS_OK;
	}
	alarm_hd = (alarm_log_head_t*)tlv->value;
	alarm_hd->log_num = ntohs(alarm_hd->log_num);
	alarm_hd->err = ntohl(alarm_hd->err);
	if(alarm_hd->log_num == 0){
		log_debug("%s, device %012llu, drop for alarm log num is 0\r\n",
			__FUNCTION__, ((net_header_v2_t*)hdr)->sn);
		return RS_OK;
	}
	if( alarm_hd->action != ALARM_LOG_ACT_PUSH &&alarm_hd->action != ALARM_LOG_ACT_ADD){
		log_debug("%s, device %012llu, drop for alarm log num is 0\r\n",
			__FUNCTION__, ((net_header_v2_t*)hdr)->sn);
		return RS_OK;
	}
	if(tlv->length < (sizeof(*alarm_hd)+sizeof(*alarm)) ){
		log_debug("%s, device %012llu,drop for tlv length is less : %u < %u\r\n",
			__FUNCTION__, ((net_header_v2_t*)hdr)->sn, tlv->length,(sizeof(*alarm_hd)+sizeof(*alarm)));
		return RS_OK;
	}
	
	evt = cl_calloc(sizeof(*evt), 1);
	list = cl_calloc(sizeof(*list), 1);
	msg = cl_calloc(sizeof(*msg), alarm_hd->log_num);
	if(evt == NULL || list == NULL || msg == NULL)
		goto errorout;

#ifndef	NO_COMMUNITY
	if(user->ds_type == TP_DS007)
		evt->event = CE_ALARM;
	else
#endif
		evt->event = NE_ALARM_LOG;
	evt->msg = list;
	list->dev_sn = ((net_header_v2_t*)hdr)->sn;
	list->first_report_id = report_id;
	list->count = 0;
	list->msg = msg;

	left = tlv->length - sizeof(*alarm_hd);
	index = sizeof(*alarm_hd);
	while(left >= sizeof(*alarm) && list->count < alarm_hd->log_num){
		alarm = (alarm_log_t*)&tlv->value[index];
		msg[list->count].alarm_time = ntohl(alarm->log_time);
		msg[list->count].alarm_duration = ntohs(alarm->log_last_time);
		msg[list->count].alarm_type = ntohs(alarm->alarm_factoryid);
		msg[list->count].alarm_id = ntohs(alarm->local_id);
		msg[list->count].len_name = alarm->alarm_name_len;
		msg[list->count].len_msg = alarm->alarm_msg_len;
		current = sizeof(*alarm) + alarm->alarm_name_len 
			+ alarm->alarm_msg_len 
			+ alarm->alarm_phone_count * MAX_PHONE_SINGLE;
		if(left < current){
			log_debug("%s, device %012llu,drop for remain tlv length is less: %u < %u\r\n",
				__FUNCTION__, ((net_header_v2_t*)hdr)->sn, left, current);
			break;
		}
		if(alarm->alarm_name_len)
			memcpy(msg[list->count].arlam_name, alarm->alarm_name, 
					alarm->alarm_name_len);
		if(alarm->alarm_msg_len)
			memcpy(msg[list->count].alarm_msg, 
					&alarm->alarm_name[alarm->alarm_name_len], alarm->alarm_msg_len);

		left -= current;
		index += current;
		list->count++;		
	}
	
	stlc_list_add_tail(&evt->link, &user->user_cmt->alarm_head);
	if(event_push(user->callback, evt->event, user->handle, user->callback_handle) != RS_OK){
		stlc_list_del(&evt->link);
		goto errorout;
	}	
			
	return RS_OK;
errorout:
	if(evt)
		cl_free(evt);
	if(list)
		cl_free(list);
	if(msg)
		cl_free(msg);
	return RS_ERROR;	
}

static RS _recv_common_notify(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	net_notify_value_t *net_msg;  /*网络层*/
	notify_msg_t *up_msg = NULL;	/*应用层*/
	user_cmt_event_t *evt = NULL;
	
	if(tlv->length < sizeof(*net_msg))
		return RS_OK;
	net_msg = (net_notify_value_t *)tlv->value;
	net_msg->timestamp = ntohl(net_msg->timestamp);
	net_msg->len = ntohl(net_msg->len);
	if(tlv->length < (sizeof(*net_msg) + net_msg->len)){
		return RS_OK;
	}
	up_msg = cl_malloc(sizeof(*up_msg));
	if(up_msg == NULL)
		goto errorout;
	
	up_msg->msg = cl_malloc(net_msg->len + 1);
	if(up_msg->msg == NULL){
		cl_free(up_msg);
		return RS_ERROR;
	}
	
	evt = cl_malloc(sizeof(*evt));
	if(evt == NULL){
		goto errorout;
	}
	up_msg->content = NULL;
	up_msg->dev_sn = ((net_header_v2_t*)hdr)->sn;
	up_msg->msg_time = net_msg->timestamp;
	up_msg->report_id = report_id;
	up_msg->msg_type = tlv->type;
	up_msg->msg_format = FMT_HTTP;
	up_msg->reserved = 0;
	up_msg->msg_len = net_msg->len;
	
	memcpy(up_msg->msg, net_msg->notify_msg, net_msg->len);
	up_msg->msg[net_msg->len] = 0;

	evt->msg = (void*)up_msg;
#ifndef	NO_COMMUNITY
	if(user->ds_type == TP_DS007)
		evt->event = CE_NOTIFY;
	else
#endif
		evt->event = NE_NOTIFY;
	stlc_list_add_tail(&evt->link, &user->user_cmt->notify_head);
	if(event_push(user->callback, evt->event, user->handle, user->callback_handle) != RS_OK){
		stlc_list_del(&evt->link);
		goto errorout;		
	}
		
	return RS_OK;

errorout:
	if(evt)
		cl_free(evt);
	if(up_msg){
		if(up_msg->msg)
			cl_free(up_msg->msg);
		cl_free(up_msg);
	}
	return RS_ERROR;
}

static RS _recv_common_notify_v2(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	net_notify_value_v2_t *net_msg;  /*网络层*/
	notify_msg_t *up_msg = NULL;	/*应用层*/
	user_cmt_event_t *evt = NULL;
	
	if(tlv->length < sizeof(*net_msg))
		return RS_OK;
	net_msg = (net_notify_value_v2_t *)tlv->value;
	net_msg->timestamp = ntohl(net_msg->timestamp);
	net_msg->len = ntohl(net_msg->len);
	if(tlv->length < (sizeof(*net_msg) + net_msg->len + net_msg->title_len)){
		return RS_OK;
	}
	up_msg = cl_malloc(sizeof(*up_msg));
	if(up_msg == NULL)
		goto errorout;
	
	up_msg->msg = cl_malloc(net_msg->len + 1);
	if(up_msg->msg == NULL){
		cl_free(up_msg);
		return RS_ERROR;
	}
	
	evt = cl_malloc(sizeof(*evt));
	if(evt == NULL){
		goto errorout;
	}
	up_msg->content = NULL;
	up_msg->dev_sn = ((net_header_v2_t*)hdr)->sn;
	up_msg->msg_time = net_msg->timestamp;
	up_msg->report_id = report_id;
	up_msg->msg_type = tlv->type;
	up_msg->msg_format = net_msg->msg_fmt;
	up_msg->reserved = 0;
	up_msg->msg_len = net_msg->len;
		
	memcpy(up_msg->msg, net_msg->notify_msg, net_msg->len);
	up_msg->msg[net_msg->len] = 0;
	if (net_msg->title_len) {
		up_msg->content = cl_malloc(net_msg->title_len + 1);
		if (up_msg->content) {
			memcpy(up_msg->content, &net_msg->notify_msg[net_msg->len], net_msg->title_len);
			up_msg->content[net_msg->title_len] = 0;
		}
	} else {
		up_msg->content = NULL;
	}

	evt->msg = (void*)up_msg;
#ifndef	NO_COMMUNITY
	if(user->ds_type == TP_DS007)
		evt->event = CE_NOTIFY;
	else
#endif
		evt->event = NE_NOTIFY;
	stlc_list_add_tail(&evt->link, &user->user_cmt->notify_head);
	if(event_push(user->callback, evt->event, user->handle, user->callback_handle) != RS_OK){
		stlc_list_del(&evt->link);
		goto errorout;		
	}
		
	return RS_OK;

errorout:
	if(evt)
		cl_free(evt);
	if(up_msg){
		if(up_msg->msg)
			cl_free(up_msg->msg);
		cl_free(up_msg);
	}
	return RS_ERROR;
}

static RS _recv_notify_tlv(user_t *user, net_notify_tlv_t *tlv, u_int32_t report_id, net_header_t *hdr)
{
	RS ret = RS_OK;
	switch(tlv->type){
		case NOTIFY_ALARM_LOG:
			ret = _recv_alarm_log(user, tlv, report_id, hdr);
			break;
		case NOTIFY_MESURE_TRANS:
			ret = _recv_mesure_trans(user, tlv, report_id, hdr);
			break;
		case NOTIFY_EMERGENCY:
		case NOTIFY_NORMAL:
		case NOTIFY_AD:
			ret = _recv_common_notify(user, tlv, report_id, hdr);
			break;

		case NOTIFY_EMERGENCY_V2:
		case NOTIFY_NORMAL_V2:
		case NOTIFY_AD_V2:
			ret = _recv_common_notify_v2(user, tlv, report_id, hdr);
			break;
			
		default:
			/* 未知类型，忽略 */
			break;
	}
	return ret;
}
void send_expect_notify_id(user_t *user, notify_expect_t *exp, u_int8_t cnt)
{
	pkt_t *exp_pkt;
	net_notify_expect_t *expect;
	u_int8_t i;
	
	exp_pkt = pkt_new_v2(CMD_NOTIFY_EXPECT, cnt*sizeof(*expect), 0, 0, user->ds_type);
	if(exp_pkt == NULL)
		return;
	expect = get_pkt_payload(exp_pkt, net_notify_expect_t);
	for(i = 0; i < cnt; i++)
	{
		expect[i].sn = ntoh_ll(exp[i].dev_sn);
		expect[i].expect_report_id = ntohl((u_int32_t)exp[i].expect_report_id);
	}	
	user_add_pkt( user, exp_pkt);
}
u_int8_t check_add_expect_sn(notify_expect_t *exp, u_int8_t cnt, u_int64_t sn, u_int32_t id)
{	u_int8_t i;
	for(i = 0; i <  cnt; i++){
		if(exp[i].dev_sn == sn)
			return cnt;
	}
	exp[cnt].dev_sn = sn;
	exp[cnt].expect_report_id = id;
	return (cnt+1);
}

void check_add_cmt(user_cmt_t *cmt, u_int64_t sn, u_int32_t report_id)
{
	u_int8_t i;
	for (i = 0; i < cmt->cmt_cnt; i++) {
		if (cmt->cmt_list[i] == sn){
			cmt->cmt_report_id[i] = report_id;
			return;
		}
	}
	if (cmt->cmt_cnt < sizeof(cmt->cmt_list)/sizeof(cmt->cmt_list[0])){
		cmt->cmt_list[cmt->cmt_cnt] = sn;
		cmt->cmt_report_id[cmt->cmt_cnt] = report_id;
		cmt->cmt_cnt++;
	}
}

void recv_cmt_center_list(user_t *user, pkt_t *pkt)
{
	net_notify_center_list_t *list;
	community_center_info_t  *info;
	net_header_t *hdr;
	int remain, index;
	u_int8_t i,n, cnt;
	user_cmt_t *cmt = user->user_cmt;

	hdr = (net_header_t*)pkt->data;
	if(hdr->param_len < sizeof(*list)){
		log_err(false, "%s param len too short %d < %d\n", 
			__FUNCTION__, hdr->param_len, sizeof(*list));
		return;
	}
	list = get_pkt_payload(pkt, net_notify_center_list_t);
	if(list->element_size < sizeof(*info)){
		log_err(false, "%s element_size too short %d < %d\n", 
			__FUNCTION__, list->element_size, sizeof(*info));
		return;
	}

	remain = hdr->param_len - sizeof(*list);
	n = remain / list->element_size;
	log_debug("%012llu %s , center count %d (%d)\n", user->sn, __FUNCTION__, list->count, n);
	n = min(n, list->count);

#if 1
	/*收到设备所属小区列表，通知app，由app决定同步小区的推送消息*/
	if(n < 1)
		return;
	cnt = 0;
	cmt->cmt_cnt = 0;
	for (i = 0, index = 0; i < n; i++) {
		info = (community_center_info_t*)&list->element_data[index];
		log_debug("%012llu %s , cmt %012llu, ip %u.%u.%u.%u, port %u\n", user->sn, 
			__FUNCTION__, ntoh_ll(info->sn), IP_SHOW(ntohl(info->ip)), ntohs(info->port));
		check_add_cmt(cmt, ntoh_ll(info->sn), ntohl(info->ip));	
		index += list->element_size;	
	}
	if (user->status == CS_AUTH)
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	event_push(user->callback, NE_CMT_LIST, user->handle, user->callback_handle);
	
#else
	/*期望设备的*/
	expect[0].dev_sn = user->sn;
	expect[0].expect_report_id = 1;	
	send_expect_notify_id(user, expect, 1);
	
	/*期望小区的*/
	if(n < 1)
		return;
	cnt = 0;
	for(i = 0, index = 0; i < n; i++){
		info = (community_center_info_t*)&list->element_data[index];
		log_debug("%012llu %s , cmt %012llu, ip %u.%u.%u.%u, port %u\n", user->sn, 
			__FUNCTION__, ntoh_ll(info->sn), IP_SHOW(ntohl(info->ip)), ntohs(info->port));
		cnt = check_add_expect_sn(expect, cnt, ntoh_ll(info->sn), 1);
		index += list->element_size;	
	}
	send_expect_notify_id(user, expect, cnt);
#endif
}

void notify_push_proc_android_msg(user_t* user)
{
    pkt_t *pkt;
    net_alarm_log_query_t* query;
    notify_push_t* np = user->np;
    //for android
    if (!user||user->is_phone_user||!user->np||!user->np->is_user_has_set_id
        ||cl_priv->cleint_type != CID_ANDROID || user->is_udp_ctrl) {
        return;
    }
    
    np->need_notify_user = 0x1;
    
    pkt = pkt_new_v2(CMD_ALARM_LOG, sizeof(net_alarm_log_query_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
    if (!pkt) {
        /*内存分配失败，需要修改错误值*/
        return;
    }
    query = get_pkt_payload(pkt, net_alarm_log_query_t);
    query->alarm_header.action = ALARM_LOG_ACT_QUERY;
    
    user_add_pkt(user, pkt);
    
}

void recv_notify(user_t *user, pkt_t *pkt, struct sockaddr_in *addr)
{
	net_header_t *hdr;
	net_header_v2_t *hv2;
	net_notify_t *notify;
	net_notify_tlv_t *tlv;	
	slave_t *slave;
	u_int32_t left, index;
	u_int8_t count;
	u_int32_t ip;
	u_int16_t port;
	RS ret = RS_ERROR;
	
	hdr = (net_header_t *)(pkt->data);
	if (hdr->ver < 2) 
		return;
	if(hdr->param_len < sizeof(*notify)){
		log_debug("%s, param_len is less %u < %u\n", 
			__FUNCTION__, hdr->param_len, sizeof(*notify));
		return;
	}
	hv2 = (net_header_v2_t *)hdr;
	notify = get_pkt_payload(pkt, net_notify_t);
	notify->first_report_id = ntohl(notify->first_report_id);	

	if(addr){
		ip = ntohl(addr->sin_addr.s_addr);
		port = ntohs(addr->sin_port);
	}else{
		ip = user->devserver_ip;
		port = user->devserver_port;
	}
	log_debug("%s, peer sn=%012llu, ip=%u.%u.%u.%u, port=%u, first_report_id=%u\n", 
		__FUNCTION__, hv2->sn, IP_SHOW(ip), port, notify->first_report_id);
		
	slave = slave_lookup_by_sn(user, hv2->sn);
	if(slave == NULL){
		/* 出于安全考虑，先从小区添加才能处理 */
		if(addr)
			return;
	}else if(addr){
		update_slave_addr(slave, addr);
	}

	if(notify->report_count == 0){
		log_debug("%s, report_count is 0\n", __FUNCTION__);
		goto send_ack;
	}

	if(hdr->param_len < (sizeof(*notify) + sizeof(*tlv))){
		/*至少应该包含一个TLV 长度*/
		log_debug("%s, param_len is less %u < %u\n", 
			__FUNCTION__, hdr->param_len, (sizeof(*notify) + sizeof(*tlv)));
		return;
	}

	left = hdr->param_len - sizeof(*notify);
	index = 0;count = 0;
	while(left >= sizeof(*tlv) && count < notify->report_count){
		tlv = (net_notify_tlv_t*)&notify->tlv_data[index];
		tlv->type = ntohs(tlv->type);
		tlv->length = ntohl(tlv->length);
		if(left < (sizeof(*tlv) + tlv->length)){
			log_debug("%s, remain is less %u < %u\n", 
				__FUNCTION__, left, (sizeof(*tlv) + tlv->length));
			break;
		}
		
		ret = _recv_notify_tlv(user, tlv, notify->first_report_id + count, hdr);
		if(ret != RS_OK)
			break;
		count++;		
		left -= (sizeof(*tlv) + tlv->length);
		index += (sizeof(*tlv) + tlv->length);		
	}

	
	if(ret == RS_OK)
		goto send_ack;
	return;
	
send_ack:
	send_notify_ack(user, notify, addr, RS_OK);
	
}
/////////////////////////////////////////////////////////////////////////

bool notify_push_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_notify_push_t * np;
    user_t * user;
    
    if (pkt->type < CLNE_NOTIFY_PUSH_START || pkt->type > CLNE_NOTIFY_PUSH_MAX) {
		return false;
	}

    np = (cln_notify_push_t *)&pkt->data[0];
    if ((user = lookup_by_handle(HDLT_USER, np->user_haldle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, np->user_haldle);
		return false;
	}
	
    if (!user->np) {
        user->np = notify_push_alloc();
        if (!user->np) {
            return false;
        }
    }
	
	switch (pkt->type) {
	case CLNE_NOTIFY_PUSH_SET_EXPECT_ID:
        *ret = notify_push_set_expect_id(user,np);
		break;

	default:
		return false;
	}

	return true;
}

notify_push_t *notify_push_alloc(void)
{
    notify_push_t * np = cl_calloc(sizeof(notify_push_t), 1);
    
    return np;
}

static void msglist_free(alarm_msg_list_t* list)
{
    if (list) {
        if (list->msg) {
            cl_free(list->msg);
        }
        cl_free(list);
    }
}

void notify_push_free(notify_push_t *np)
{
    if (np) {
        msglist_free(np->msglist);
        cl_free(np);
    }
}


