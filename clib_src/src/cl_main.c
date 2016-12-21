#include "client_lib.h"
#include "cl_priv.h"
#include "md5.h"
#include "cl_notify.h"
#include "cl_thread.h"
#include "cl_user.h"
#include "net_detect.h"
#include "cmd_misc.h"
#include "user_priv.h"
#include "video_priv.h"
#include "wait_server.h"
#include "community_priv.h"
#include "plug_priv.h"
#include "notify_push_priv.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "lan_dev_probe_priv.h"
#include "phone_user.h"
#include "ia_priv.h"
#include "health_priv.h"
#include "intelligent_forward_priv.h"
#include "cloud_ac_priv.h"

#include "uc_client.h"
#include "smart_appliance_priv.h"
#include "lc_furnace_priv.h"
#include "uasc_priv.h"
#include "uas_client.h"
#include "linkage_client.h"
#include "rfgw_priv.h"

static void reset_event_timer();

// 消息发的太密了，手机扛不住。慢点一个一个的发
#define	EVENT_ONE_BY_ONE	1
#define	TIME_EVENT_ONE_BYE_ONE	20

pkt_t *pkt_new(int cmd, int param_len, u_int8_t ds_type)
{
	pkt_t *pkt;
	net_header_t *hdr;
	int total_len = sizeof(pkt_t) + net_header_size + param_len;

	if ((pkt = (pkt_t *)cl_calloc(total_len, 1)) == NULL) {
		log_err(false, "pkt_new alloc %d failed\n", total_len);
		return NULL;
	}
	pkt->total = net_header_size + param_len;
    pkt->cmd = cmd;

	hdr = (net_header_t *)&pkt->data[0];
	hdr->ver = PROTO_VER1;
	hdr->hlen = net_header_size/4;
	hdr->ds_type = ds_type;
	hdr->command = htons(cmd);
	hdr->param_len = htonl(param_len);

	return pkt;
}

pkt_t *pkt_new_v2(int cmd, int param_len, u_int8_t flags, u_int64_t slave_sn, u_int8_t ds_type)
{
	pkt_t *pkt;
	net_header_t *hdr;
	net_header_v2_t *v2;
	int total_len = sizeof(pkt_t) + net_header_v2_size + param_len;

	if ((pkt = (pkt_t *)cl_calloc(total_len, 1)) == NULL) {
		log_err(false, "pkt_new alloc %d failed\n", total_len);
		return NULL;
	}
	pkt->total = net_header_v2_size + param_len;

	hdr = (net_header_t *)&pkt->data[0];
	v2 = (net_header_v2_t *)&pkt->data[0];
	
	hdr->ver = PROTO_VER2;
	hdr->hlen = net_header_v2_size/4;
	hdr->ds_type = ds_type;
	hdr->command = htons(cmd);
	hdr->param_len = htonl(param_len);
	hdr->handle = 0;

	v2->flags |= flags;
	//v2->flags |= NHF_TRANSPARENT;
	v2->sn = ntoh_ll(slave_sn);

	return pkt;
}

void pkt_free(void *pkt)
{
	if (pkt != NULL)
		cl_free(pkt);
}

void hdr_order(pkt_t *pkt)
{
	net_header_t *hdr;

	hdr = (net_header_t *)&pkt->data[0];
	hdr->command = ntohs(hdr->command);
	hdr->param_len = ntohl(hdr->param_len);
}
// 临时code
typedef struct {
	u_int64_t sn;
	u_int8_t sub_type;
} sn_dict_t;


#ifdef CONFIG_SN_DICT
static sn_dict_t sn_dicts[] = {
	{ 107ULL, IJ_AIRCLEANER},
	{ 180000013545ULL, IJ_AIRCLEANER },
	{ 182000010004ULL, IJ_WATERHEATER },		
	{ 182000010007ULL, IJ_WATERHEATER },
	{ 182000010005ULL, IJ_ELECTRICFAN },
	{ 182000010006ULL, IJ_AIRCONDITION },
	{ 42ULL, IJ_AIRHEATER },
	{ 182000010008ULL, IJ_ELECTRICFAN},
};


u_int8_t parse_sn_for_type(u_int64_t sn)
{
	int i;
	u_int8_t type = 0;
	
	for (i = 0; i < sizeof(sn_dicts)/sizeof(sn_dicts[0]); i++) {
		if (sn == sn_dicts[i].sn) {
			type = sn_dicts[i].sub_type;
			break;
		}
	}

	return type;
}
#endif


static void parse_cmd_ok_param(user_t *user, net_header_t *hdr)
{
	net_cmd_ok_t *ok;
	st_tlv *tlv;
	unsigned int remain, index;
	bool pv_modify = false;
#ifdef CONFIG_SN_DICT	
	u_int8_t sub_type = 0;
#endif

	if (hdr->param_len < sizeof(*ok)) {
		return;
	}
	ok = get_pkt_payload(user->tcp_buf, net_cmd_ok_t);
	remain = hdr->param_len - sizeof(*ok);
	index = 0;

#ifdef CONFIG_SN_DICT
	if ((sub_type = parse_sn_for_type(user->sn)) != 0) {
		user->sub_type = sub_type;
	} else {
#endif	
		if (user->sub_type != ok->sub_type) {
			log_debug("%s login ok, parse dev_type from %u to %u\n", user->name, user->sub_type, ok->sub_type);
			user->sub_type = ok->sub_type;
			if (user->parent != NULL && user->parent->is_phone_user) {
				pu_save_dev_type(user);
			}
		}
#ifdef CONFIG_SN_DICT
	}
#endif
	user->ext_type = ok->reserved;
	user->df_flags = ok->df_flags;
	
	while(remain >= sizeof(*tlv)){
		tlv = (st_tlv*)&ok->tlv[index];
		tlv->type = ntohs(tlv->type);
		tlv->len = ntohs(tlv->len);
		if(remain < (sizeof(*tlv) + tlv->len))
			break;
		switch(tlv->type){
			case DIT_TYPE:
				log_debug("DIT_TYPE = %s\n", (char *)tlv->value);
				break;
			case DIT_NAME: /* like: i+003, i+007 */
				log_debug("DIT_NAME = %s\n", (char *)tlv->value);
				break;
			case DIT_IPHONE_NEWEST_VERSION:
				log_debug("DIT_IPHONE_NEWEST_VERSION = %s\n", (char *)tlv->value);
				if ( ! IS_SAME_STR(cl_priv->iphone_newest_version, (char *)tlv->value) ) {
					pv_modify = true;
					STR_REPLACE(cl_priv->iphone_newest_version, (char *)tlv->value);
				}
				break;
			case DIT_ANDRIOD_NEWEST_VERSION:
				log_debug("DIT_ANDRIOD_NEWEST_VERSION = %s\n", (char *)tlv->value);
				if ( ! IS_SAME_STR(cl_priv->android_newest_version, (char *)tlv->value) ) {
					pv_modify = true;
					STR_REPLACE(cl_priv->android_newest_version, (char *)tlv->value);
				}
				break;
			case DIT_IPHONE_EN:
				log_debug("DIT_IPHONE_EN = %s\n", (char *)tlv->value);
				STR_REPLACE(cl_priv->desc_iphone_en, (char *)tlv->value);
				break;
			case DIT_IPHONE_CH:
				log_debug("DIT_IPHONE_CH = %s\n", (char *)tlv->value);
				STR_REPLACE(cl_priv->desc_iphone_ch, (char *)tlv->value);
				break;
			case DIT_ANDRIOD_EN:
				log_debug("DIT_ANDRIOD_EN = %s\n", (char *)tlv->value);
				STR_REPLACE(cl_priv->desc_android_en, (char *)tlv->value);
				break;
			case DIT_ANDRIOD_CH:
				log_debug("DIT_ANDRIOD_CH = %s\n", (char *)tlv->value);
				STR_REPLACE(cl_priv->desc_android_ch, (char *)tlv->value);
				break;
			case DIT_LOGINTYPE:
				if(tlv->len == 1){
					user->bp->login_type = tlv->value[0];
				}else{
					user->bp->login_type = LT_NORMAL;
				}
				log_debug("DIT_LOGINTYPE = %u\n", user->bp->login_type);
				break;
				
			case DIT_NETTYPE:
				if(tlv->len == 1){
					user->bp->net_type = tlv->value[0];
				}else{
					user->bp->net_type = NT_DEVICE;
				}
				log_debug("DIT_NETTYPE = %u\n", user->bp->login_type);
				break;
				
			default:
				break;
		}
		remain -= (sizeof(*tlv) + tlv->len);
		index += (sizeof(*tlv) + tlv->len);
	}	

	if (pv_modify) {
		event_push(user->callback, UE_PHONE_NEW_VERSION, user->handle, user->callback_handle);
	}
}
static void do_cmd_ok(user_t *user, net_header_t *hdr)
{
	wait_t *w;
	
	if (user->status == CS_AUTH) {
		if(user->bp->is_binding){
            user->last_err = ULGE_NONE;
			event_push(user->callback, UE_BIND_PHONE_REQUEST_OK, user->handle, user->callback_handle);
			user_set_status(user, CS_LOGIN_ERR);
			user->bp->is_binding = 0;
		}else{
			log_info("%s Login OK.\n", user->name);
			parse_cmd_ok_param(user, hdr);
			user_set_status(user, CS_ESTABLISH);
			wait_del(hdr->handle);
		}
		return;
	}

	if ((w = wait_lookup(hdr->handle)) == NULL) {
		log_info("Ignore CMD_OK: not found wait handle=0x%08x\n", hdr->handle);
		return;
	}

	wait_callback(w, CMD_OK, true);	
}

static void do_cmd_fail(user_t *user, net_header_t *hdr)
{
	wait_t *w;
	const char *e;
	u_int32_t *ep, ecode = 0;

	if (hdr->command == CMD_FAIL_EX) {
		direct_link_reply_t *dlr;
		dlr = get_net_pkt_payload(hdr, direct_link_reply_t);
		ecode = ntohl(dlr->error);
		e = get_err_str(ecode);
	} else {
		if (hdr->param_len >= 4) {
			ep = (unsigned int *)((char *)hdr + net_header_real_size(hdr));
			ecode = ntohl(*ep);

			e = get_err_str(ecode);
		} else {
			e = "未知的错误";
		}
	}

	// 收到CMD_FAIL和CMD_FAIL_EX，必为错误
	if (ecode == 0)
		ecode = ERR_SYSTEM;
	
	log_err(false, "**** %s server reply error, error code is %d, current status is %d(%s)\n", user->name, ecode, user->status, e);

	if (user->status == CS_AUTH) {
		if (ecode == ERR_PASSWD_INVALID) {
			/*密码错*/
			user->last_err = ULGE_BAD_PASSWORD;
			user_set_status(user, CS_LOGIN_ERR);
		} else if (ecode == ERR_UNBIND) {
			/*需要绑定手机*/
			user->last_err = ULGE_NEED_BIND;
			user_set_status(user, CS_LOGIN_ERR);
		} else if (ecode == ERR_BIND_FULL) {
			/*已经绑定满了*/
			user->last_err = ULGE_FULL_BIND;
			user_set_status(user, CS_LOGIN_ERR);
		} else {
			user_set_status(user, CS_IDLE);
		}
		return;
	}

	if ((w = wait_lookup(hdr->handle)) == NULL) {
		log_info("Ignore CMD_FAIL: not found wait handle=0x%08x\n", hdr->handle);
		return;
	}

	wait_callback(w, ecode, true);

}

int tcp_read(cl_thread_t *thread)
{
	int	n, want, total_len;
	user_t *user = (user_t *)CL_THREAD_ARG(thread);
	net_header_t *hdr;
	pkt_t *pkt;
	bool too_big = false;

	user->t_read = NULL;
	CL_THREAD_READ_ON(&cl_priv->master, user->t_read, tcp_read, (void *)user, user->sock_dev_server);

	if (user->status == CS_DEV_CONNECTING) {
		tcp_check_connecting(user);
		return 0;
	}

	// real read
	pkt = user->tcp_buf;
	hdr = (net_header_t *)pkt->data;
	if (user->tcp_recv_len >= net_header_size) {
		want = (hdr->param_len + net_hdr_len(hdr)) - user->tcp_recv_len;
		if (hdr->param_len + net_hdr_len(hdr) > MAX_TCP_PKT)
			too_big = true;
	} else {
		want = net_header_size - user->tcp_recv_len;
	}

	if (too_big) {
		n = recv(user->sock_dev_server, BOFP(hdr, net_header_size), min(MAX_TCP_PKT - net_header_size, want), 0);
	} else {
		n = recv(user->sock_dev_server, BOFP(hdr, user->tcp_recv_len), want, 0);
	}
	if (n == 0) {
		log_err(false, "%s TCP connect close by peer\n", user->name);
		goto err;
	} else if (n < 0) {
		if (NORMAL_ERRNO(GET_SOCK_ECODE())) {
			return 0;
		}
		log_err(true, "%s read socket failed\n", user->name);
		goto err;
	}

	// reset die
	CL_THREAD_OFF(user->t_timer);
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void *)user, TIME_CLIENT_DIE);

	user->tcp_recv_len += n;
	if (n < want)
		return 0;
	// 刚刚接收完毕头部
	if (user->tcp_recv_len == net_header_size) {
		hdr_order(pkt);
	}
	total_len = hdr->param_len + net_hdr_len(hdr);
	if ((int)user->tcp_recv_len < total_len){
		return 0;
	}
	if (too_big) {
		log_err(false, "%s ignore too big packet total_len=%d, cmd=%u\n", user->name, total_len, hdr->command);
		goto done;
	}

	if (hdr->ver >= 2) {
		net_header_v2_t *hdr_v2;

		if (net_hdr_len(hdr) < net_header_v2_size) {
			log_err(false, "%s Bad packet: ver=%d, hlen=%d\n", user->name, hdr->ver, net_hdr_len(hdr));
			goto done;
		}
		hdr_v2 = (net_header_v2_t *)hdr;
		hdr_v2->sn = ntoh_ll(hdr_v2->sn);
	}
#ifdef SUPPORT_TRAFFIC_STAT
    TCP_CMD_PKT_STAT(hdr->command,true,total_len);
#endif

	// 完整报文，处理之
	log_info("user %s tcp_read: command=%d, param len=%d\n", 
		user->name,	hdr->command, hdr->param_len);
	
	cl_lock(&cl_priv->mutex);
	if (hdr->command == CMD_OK) {
		do_cmd_ok(user, hdr);
	} else if (hdr->command == CMD_FAIL || hdr->command == CMD_FAIL_EX) {
		do_cmd_fail(user, hdr);
	} else {
		if (user_proc_tcp(user, user->tcp_buf, hdr)
			|| video_proc_tcp(user, user->tcp_buf, hdr)
			|| plug_proc_tcp(user, user->tcp_buf, hdr)
            || notify_push_proc_tcp(user, user->tcp_buf, hdr)
            || eq_proc_tcp(user, user->tcp_buf, hdr)
            || area_proc_tcp(user, user->tcp_buf, hdr)
            || scene_proc_tcp(user, user->tcp_buf, hdr)
            || ia_proc_tcp(user, user->tcp_buf, hdr)
            || health_proc_tcp(user, user->tcp_buf, hdr)
			);
	}
	// 删除对应的等待
	if (hdr->command != CMD_VIDEO_HELLO_REQ && hdr->command != CMD_CLOUD_MATCH) {
		wait_del(hdr->handle);
	}
	cl_unlock(&cl_priv->mutex);

done:
	user->tcp_recv_len = 0;

	return 0;

err:
	user_set_status(user, CS_IDLE);
	
	return 0;
}
	
int tcp_write(cl_thread_t *thread)
{
	int n, want;
	pkt_t *pkt;
	net_header_t *hdr;
	user_t *user = (user_t *)CL_THREAD_ARG(thread);

	user->t_write = NULL;
	if (user->status == CS_DEV_CONNECTING) {
		tcp_check_connecting(user);
		return 0;
	}

	if (stlc_list_empty(&user->tcp_send_list)) {
		log_err(false, "%s tcp_write, but send list is empty\n", user->name);
		return 0;
	}
	
	pkt = (pkt_t *)stlc_list_entry(user->tcp_send_list.next, pkt_t, link);
	hdr = (net_header_t *)pkt->data;
	want = pkt->total - user->tcp_send_len;
	cl_assert(want > 0);
	n = send(user->sock_dev_server, &pkt->data[user->tcp_send_len], want, 0);
	if (n < 0) {
		if (NORMAL_ERRNO(GET_SOCK_ECODE())) {
			CL_THREAD_WRITE_ON(&cl_priv->master, user->t_write, tcp_write, (void *)user, user->sock_dev_server);
			return 0;
		}
		log_err(true, "%s write socket error\n", user->name);
		goto err;
	}

	user->tcp_send_len += n;

	if (n == want) {
		user->tcp_send_len = 0;
		stlc_list_del(&pkt->link);

		log_debug("send pkt done: type=%u, total=%u\n", ntohs(((net_header_t *)pkt->data)->command), pkt->total);
#ifdef SUPPORT_TRAFFIC_STAT
        //发包统计
        TCP_CMD_PKT_STAT(ntohs(hdr->command),false,pkt->total);
#endif
		pkt_free(pkt);
		
		if (stlc_list_empty(&user->tcp_send_list)) {
			return 0;
		}
	}

	CL_THREAD_WRITE_ON(&cl_priv->master, user->t_write, tcp_write, (void *)user, user->sock_dev_server);

	return 0;

err:
	user_set_status(user, CS_IDLE);
	
	return 0;
}

void user_add_pkt(user_t *user, pkt_t *pkt)
{
    
	if ( ! VALID_SOCK(user->sock_dev_server)) {
		log_err(false, "%s user_add_pkt command %u, but sock is invalid\n", user->name,pkt->cmd);
		pkt_free(pkt);
		return;
	}
	
	stlc_list_add_tail(&pkt->link, &user->tcp_send_list);
	if (user->t_write == NULL) {
		CL_THREAD_WRITE_ON(&cl_priv->master, user->t_write, tcp_write, (void *)user, user->sock_dev_server);
	}
}

int event_callback(cl_thread_t *t)
{
	event_info_t *ei, *next;
	u_int32_t now = get_msec();

	cl_priv->t_event = NULL;

	stlc_list_for_each_entry_safe(event_info_t, ei, next, &cl_priv->events, link) {
		if (u32_is_bigger(ei->merge_timeout, now)) {
#ifdef	EVENT_ONE_BY_ONE			
			// 一个一个发，中间加间隔
			break;
#else
			continue;
#endif			
		}
		
		log_debug("@@ Begin call event: func=%p, event=%u, obj_handle=0x%08x, more=%p, errno=%d\n", 
			ei->func, ei->event, ei->obj_handle, ei->more_info, ei->more_info->info.err_no);
		
		ei->func(ei->event, (void *)ei->obj_handle, (void *)&ei->more_info->info);

		log_debug("@@ ...End call event: func=%p, event=%u, obj_handle=0x%08x, more=%p, errno=%d\n", 
			ei->func, ei->event, ei->obj_handle, ei->more_info, ei->more_info->info.err_no);

		stlc_list_del(&ei->link);
		cl_free(ei);
#ifdef	EVENT_ONE_BY_ONE			
		// 一个一个发，中间加间隔
		break;
#endif
	}

	reset_event_timer();

	return 0;
}

static int event_timer(cl_thread_t *t)
{
	cl_priv->t_event_timer = NULL;
	CL_THREAD_EVENT_ON(&cl_priv->master, cl_priv->t_event, event_callback, NULL, 0);

	return 0;
}

#define	TIMEOUT_MERGE_LOGIN	5000

typedef struct {
	const char *name;
	int event;
	u_int32_t timeout_min;
	u_int32_t timeout_max;
} event_merge_info_t;

static event_merge_info_t event_merge_info[] = {
	{"UE_INFO_MODIFY", UE_INFO_MODIFY, 100, 2000},
	{"LDPE_DEVICE_CHANGED", LDPE_DEVICE_CHANGED, 100, 1000},
	// 环境变量，干等一会儿，便于登录成功
	{"UE_ENV_MON_MODIFY", UE_ENV_MON_MODIFY, TIMEOUT_MERGE_LOGIN, TIMEOUT_MERGE_LOGIN},
	{"NE_NOTIFY", NE_NOTIFY, 10, 10},
	{"", -1, 0, 0}
};

static bool event_is_login_logout(int event) 
{
	return (event == UE_LOGOUT 
		|| event == UE_LOGIN_OFFLINE 
		|| event == UE_LOGIN_ONLINE 
		|| event == UE_LOGIN_ERROR);	
}

static bool event_can_merge(int event)
{
	event_merge_info_t *emi;

	for (emi = &event_merge_info[0]; emi->event >= 0; emi++) {
		if (emi->event == event)
			return true;
	}
	return event_is_login_logout(event);
}

static int event_merge_get_timeout(int event)
{
	event_merge_info_t *emi;

	for (emi = &event_merge_info[0]; emi->event >= 0; emi++) {
		if (emi->event == event)
			return emi->timeout_min;
	}

	return TIMEOUT_MERGE_LOGIN;
}

cl_handle_t get_parent_handle(cl_handle_t h)
{
	void *obj;
	cl_handle_t ret = INVALID_HANDLE;
	user_t *user;
	slave_t *slave;
	video_t *video;
    equipment_t* eq;
    area_priv_t* area_priv;
    scene_priv_t* scene_priv;

	if (h == INVALID_HANDLE)
		return INVALID_HANDLE;

	if ((obj = lookup_by_handle(HANDLE_TYPE(h), h)) == NULL)
		return INVALID_HANDLE;
	
	switch (HANDLE_TYPE(h)) {
	case HDLT_USER:
		user = (user_t *)obj;
		ret = (user->parent == NULL ? INVALID_HANDLE : user->parent->handle);
		break;
	case HDLT_SLAVE:
		slave = (slave_t *)obj;
		ret = slave->user->handle;
		break;
	case HDLT_VIDEO:
		video = (video_t *)obj;
		ret = video->slave->handle;
		break;
	case HDLT_EQUIPMENT:
		eq = (equipment_t *)obj;
		ret = eq->ec->slave->handle;
		break;
	case HDLT_AREA:
		area_priv = (area_priv_t *)obj;
		ret = area_priv->ac->user->handle;
		break;
	case HDLT_SCENE:
		scene_priv = (scene_priv_t *)obj;
		ret = scene_priv->sc->user->handle;
		break;
	default:
		break;
	}

	return ret;
}

void event_cancel_merge(cl_handle_t handle)
{
	event_info_t *ei;
	u_int32_t now = get_msec();
	cl_handle_t parent_handle;

	stlc_list_for_each_entry(event_info_t, ei, &cl_priv->events, link) {
		if (handle == ei->obj_handle) {
			log_debug("@@ Cancel event merge for handle=0x%08x, event=%u, timeout remain=%u ms\n",
				(cl_handle_t)handle, ei->event, ei->merge_timeout - now);
			ei->merge_timeout = now;
		}
	}

	parent_handle = get_parent_handle((cl_handle_t)handle);
	if (parent_handle != INVALID_HANDLE) {
		event_cancel_merge(parent_handle);
	}
	
	CL_THREAD_EVENT_ON(&cl_priv->master, cl_priv->t_event, event_callback, NULL, 0);
}

static void reset_event_timer()
{
	event_info_t *ei;
	u_int32_t now, min_timeout = 10000, n;
	
	CL_THREAD_TIMER_OFF(cl_priv->t_event_timer);
	if (stlc_list_empty( &cl_priv->events))
		return;
	
	now = get_msec();

	// 其实现在可以只看头结点了，反正发消息是从头开始慢慢发
	stlc_list_for_each_entry(event_info_t, ei, &cl_priv->events, link) {
		if (u32_is_bigger(now, ei->merge_timeout))
			n = 0;
		else
			n = ei->merge_timeout - now;

		min_timeout = min(min_timeout, n);
	}

	if (min_timeout == 0) {
		min_timeout = TIME_EVENT_ONE_BYE_ONE;
	}
	CL_THREAD_TIMER_ON(&cl_priv->master, cl_priv->t_event_timer, event_timer, NULL, min_timeout);
}

event_more_info_t *event_more_info_get()
{
	event_more_info_t *emi;

	// 从链表头取出来，放到链表尾
	emi = stlc_list_entry(cl_priv->event_more_info.next, event_more_info_t, link);
	stlc_list_del(&emi->link);

	memset(&emi->info, 0, sizeof(cl_event_more_info_t));
	emi->count++;
	emi->last_used = get_msec();

	stlc_list_add_tail(&emi->link, &cl_priv->event_more_info);

	return emi;
}


/*
	尝试合并事件，减少设备刚登录时候的众多事件
*/
static bool event_merge(cl_callback_t func, int event, cl_handle_t handle, void *callback_handle, int err)
{
	event_merge_info_t *merge;
	event_info_t *ei;
	u_int32_t now;
	user_t *user;
	bool found = false;

	// 收到其他类型的消息，不但不能合并，还要取消之前合并后的等待，触发立即发送
	if ( ! event_can_merge(event) )	{
		stlc_list_for_each_entry(event_info_t, ei, &cl_priv->events, link) {
			if (event_can_merge(ei->event) && ei->obj_handle == handle) {
				ei->merge_timeout = get_msec();
			}
		}
		return false;
	}

	now = get_msec();

	user = lookup_by_handle(HDLT_USER, handle);
	if (event == UE_INFO_MODIFY && user != NULL && user->parent != NULL) {
		user = user->parent;

		func = user->callback;
		handle = user->handle;
		callback_handle = user->callback_handle;
	}

	/*
		合并规则:
		1、LDPE_DEVICE_CHANGED消息有最小间隔和最大间隔两个时间参数，取最近的来触发
		2、UE_INFO_MODIFY消息有最小间隔和最大间隔两个时间参数，取最近的来触发
		3、LONGIN相关的3个消息，靠一个超时和其他地方调用event_cancel_merge()来触发
	*/
	for (merge = &event_merge_info[0]; merge->event >= 0; merge++) {
		if (merge->event == event)
			break;
	}
	if (merge->event >= 0) {
		stlc_list_for_each_entry(event_info_t, ei, &cl_priv->events, link) {
			if (handle != ei->obj_handle || ei->event != event)
				continue;

			log_debug("@@ Merge %s event, create time=%u ms prev, remain timeout = %u ms\n",
				merge->name, now - ei->create_time, ei->merge_timeout - now);
			
			// Yes, we find it.
			if (now - ei->create_time >= merge->timeout_max) {
				ei->merge_timeout = now;
			} else {
				ei->merge_timeout = now + merge->timeout_min;
			}
			found = true;
			break;
		}
	} else { /* UE_LOGXXX */
		stlc_list_for_each_entry(event_info_t, ei, &cl_priv->events, link) {
			if (handle != ei->obj_handle || ! event_is_login_logout(ei->event))
				continue;

			log_debug("@@ Merge LOGIN/LONGOUT event %u, create time=%u ms prev, remain timeout = %u ms\n",
				event, now - ei->create_time, ei->merge_timeout - now);

			// Yes, we find it.
			ei->merge_timeout = now + TIMEOUT_MERGE_LOGIN;
			ei->event = event;
			ei->more_info->info.err_no = err;
			found = true;
			break;
		}
	}

	// not found, we create new node
	if ( ! found ) {
		ei = cl_calloc(sizeof(event_info_t) , 1);
		ei->create_time = now;
		ei->merge_timeout = now + event_merge_get_timeout(event);
		ei->func = func;
		ei->event = event;
		ei->obj_handle = handle;
		ei->callback_handle = callback_handle;
		
		ei->more_info = event_more_info_get();
		ei->more_info->info.event = event;
		ei->more_info->info.obj_handle = handle;
		ei->more_info->info.callback_handle = callback_handle;
		ei->more_info->info.err_no = err;
		
		stlc_list_add_tail(&ei->link, &cl_priv->events);
	}

	reset_event_timer();

	return true;
}

RS event_more_info_init()
{
	int i;
	event_more_info_t *info;
	
	STLC_INIT_LIST_HEAD(&cl_priv->event_more_info);
	for (i = 0; i < NUM_EVENT_MORE_INFO; i++) {
		info = cl_calloc(sizeof(event_more_info_t), 1);
		stlc_list_add(&info->link, &cl_priv->event_more_info);
	}

	return RS_OK;
}

RS event_push_err(cl_callback_t func, int event, cl_handle_t handle, void *callback_handle, int err)
{
	event_info_t *ei;
	event_more_info_t *emi;
	
	if (func == NULL) {
		log_err(false, "push event failed: func=%p, event=%u\n", func, event);
		return RS_ERROR;
	}

	log_debug("@@ event_push: event=%u, func=%p, handle=0x%08x, callback_handle=%p, err=%d\n",
		event, func, handle, callback_handle, err);

	if (event_merge(func, event, handle, callback_handle, err)) {
	} else {
		ei = cl_calloc(sizeof(event_info_t) , 1);
		ei->create_time = ei->merge_timeout = get_msec();
		ei->func = func;
		ei->event = event;
		ei->obj_handle = handle;
		ei->callback_handle = callback_handle;

		emi = event_more_info_get();
		ei->more_info = emi;
		emi->info.event = event;
		emi->info.obj_handle = (cl_handle_t)handle;
		emi->info.callback_handle = callback_handle;
		emi->info.err_no = err;
		
		stlc_list_add_tail(&ei->link, &cl_priv->events);
		
		CL_THREAD_EVENT_ON(&cl_priv->master, cl_priv->t_event, event_callback, NULL, 0);

		// 逐级向上取消合并，立即发送
		event_cancel_merge(handle);
	}

	return RS_OK;
}

// 统一在事件中调用，避免死锁
RS event_push(cl_callback_t func, int event, cl_handle_t obj_handle, void *callback_handle)
{
	return event_push_err(func, event, obj_handle, callback_handle, 0);
}


/* ip和port都是主机序，向该地址、端口发起TCP连接 */
SOCKET connect_tcp(u_int32_t ip, int port, bool *inprogress)
{
	SOCKET sock;
	int	addr_len;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		log_err(true, "create tcp socket failed!\n");
		return INVALID_SOCKET;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;

	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);
	addr_len = sizeof(addr);

	SET_SOCK_NBLOCK(sock);
	*inprogress = false;

	if (0 !=  connect(sock, (struct sockaddr *)&addr, addr_len)) {
		if (GET_SOCK_ECODE() == EINPROGRESS) {
			*inprogress = true;
			return sock;
		}
		log_err(true, "connect tcp to %u.%u.%u.%u port %u failed!\n", IP_SHOW(ip), port);
		CLOSE_SOCK(sock);
		return INVALID_SOCKET;
	}

	log_info("connect tcp to %u.%u.%u.%u port %u 直接成功了!\n", IP_SHOW(ip), port);
	return sock;
}

/* ip和port都是主机序，向该地址、端口发起udp连接 */
SOCKET connect_udp(u_int32_t ip, int port)
{
	SOCKET sock;
	int	addr_len;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		log_err(true, "create udp socket failed!\n");
		return INVALID_SOCKET;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;

	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);
	addr_len = sizeof(addr);

	SET_SOCK_NBLOCK(sock);

	if (0 !=  connect(sock, (struct sockaddr *)&addr, addr_len)) {
		log_err(true, "connect udp to %u.%u.%u.%u port %u failed!\n", IP_SHOW(ip), port);
		CLOSE_SOCK(sock);
		return INVALID_SOCKET;
	}

	return sock;
}

/* ip和port都是主机序 */
SOCKET create_udp_server(u_int32_t ip, int port)
{
	SOCKET sock;
	int	addr_len;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		log_err(true, "create udp socket failed!\n");
		return INVALID_SOCKET;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;

	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);
	addr_len = sizeof(addr);

	SET_SOCK_NBLOCK(sock);
	if (bind(sock, (struct sockaddr *)&addr, addr_len) < 0) {
		log_err(true, "bind udp to %u.%u.%u.%u port %u failed!\n", IP_SHOW(ip), port);
		CLOSE_SOCK(sock);
		return INVALID_SOCKET;
	}

	return sock;
}


SOCKET create_udp_server6()
{
	SOCKET sock;
	struct sockaddr_in6 addr6;

	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock < 0) {
		log_err(true, "create udp socket ipv6 failed!\n");
		return INVALID_SOCKET;
	}
	
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = 0;
	addr6.sin6_flowinfo = 0;
	addr6.sin6_addr = in6addr_any;
	addr6.sin6_scope_id = 0;

	SET_SOCK_NBLOCK(sock);
	if (bind(sock, (struct sockaddr *)&addr6, sizeof(addr6)) < 0) {
		log_err(true, "bind udp to  failed!\n");
		CLOSE_SOCK(sock);
		return INVALID_SOCKET;
	}

	return sock;
}

/**************************************************************************
**  Function Name: create_tcp_server
**  Purpose:
**    U_int32_t ip.
**  Parameters:
**    u_int32_t ip---server ip, int port---server port
**  Return:
**    RS_ERROR--create socket error
**	sock-- create tcp socket fd
**  Notes:
**    Server ip address.
**************************************************************************/
/* ip和port都是主机序 */
int
sockopt_reuseaddr (int sock)
{
  int ret;
  int on = 1;

  ret = setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, 
		    (void *) &on, sizeof (on));
  if (ret < 0)
    {
      log_err(true, "setsockopt failed\n");
      return -1;
    }
  return 0;
}

SOCKET create_tcp_server(u_int32_t ip, int port)
{
	int	sock, ret;
	struct sockaddr_in addr;

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		log_err(true, "create_tcp_server socket failed!");
		return RS_ERROR;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);

	//sockopt_reuseaddr(sock);

	ret = bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		log_err(true, "create_tcp_server Can't bind to socket(ip=0x%08x, port=%d)",
			ip, port);
		CLOSE_SOCK(sock);
		return RS_ERROR;
	}

	ret = listen(sock, SOMAXCONN);
	if (ret < 0) {
		log_err(true, "create_tcp_server Can't listen to socket(port=%d)", port);
		CLOSE_SOCK(sock);
		return RS_ERROR;
	}
	//sockopt_reuseaddr(sock);
	
	return sock;
}


static RS main_proc_notify(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cl_thread_info_t *ti = &cl_priv->thread_main;

	log_debug("MAIN-thread proc notfiy: type=%d, request id=%u, parame len=%d\n",
		pkt->type, pkt->request_id, pkt->param_len);

	if ( user_proc_notify(pkt, &ret) )
		return ret;
		
	if ( video_proc_notify(pkt, &ret) )
		return ret;

	if ( plug_proc_notify(pkt, &ret) )
		return ret;
    
    if (notify_push_proc_notify(pkt, &ret)) {
        return ret;
    }
    
    if (eq_proc_notify(pkt, &ret)) {
        return ret;
    }
    
    if (area_proc_notify(pkt, &ret)) {
        return ret;
    }
    
    if (scene_proc_notify(pkt, &ret)) {
        return ret;
    }
    
    if (lan_dev_probe_proc_notify(pkt, &ret)) {
        return ret;
    }

    if(uasc_proc_notify_hook(pkt,&ret)){
	return ret;
    }
    
    if (sa_proc_notify(pkt, &ret)) {
        return ret;
    }

    if (ah_proc_notify(pkt, &ret)) {
        return ret;
    }

    if (misc_proc_notify(pkt, &ret)) {
        return ret;
    }

    if (la_proc_notify(pkt, &ret)) {
        return ret;
    }	

#ifndef	NO_COMMUNITY
	if( cmt_proc_notify(pkt, &ret) )
		return ret;
#endif

	if (ia_proc_notify(pkt, &ret)) {
		return ret;
	}

	if (health_proc_notify(pkt, &ret)) {
		return ret;
	}

	if (if_proc_notify(pkt, &ret)) {
		return ret;
	}

	if (ca_proc_notify(pkt, &ret)) {
		return ret;
	}
	
	switch (pkt->type) {
	case CLNE_STOP:
		ti->stopping = true;
		break;

	case CLNE_WAKEUP:
	default:
		break;
	}

	return ret;
}

static int read_notify(cl_thread_t *t)
{
	RS ret;
	int n, addr_len = sizeof(struct sockaddr_in);
	cl_thread_info_t *ti = &cl_priv->thread_main;
	struct sockaddr_in addr;

	ti->t_read = NULL;
	CL_THREAD_READ_ON(&cl_priv->master, ti->t_read, read_notify, NULL, ti->sock_notify);

	n = (int)recvfrom(CL_THREAD_FD(t), (char *)ti->udp_buf, ti->udp_buf_size, 0, (struct sockaddr *)&addr, &addr_len);
	if (n < (int)sizeof(cl_notify_pkt_t)) {
		log_err(false, "read notify ret=%d, fd=%u\n", n, CL_THREAD_FD(t));
		return 0;
	}

	if (n != ti->udp_buf->hdr_len + ti->udp_buf->param_len) {
		log_err(false, "main read notify, ret=%d, hdr_len=%d, param_len=%d\n",
			n, ti->udp_buf->hdr_len, ti->udp_buf->param_len);
		return 0;
	}

	cl_lock(&cl_priv->mutex);
	ret = main_proc_notify(ti->udp_buf);
	cl_unlock(&cl_priv->mutex);

	if ((ti->udp_buf->flags & CLNPF_ACK) != 0) {
		ti->udp_buf->err_code = ret;
		ti->udp_buf->param_len = 0;
		sendto(CL_THREAD_FD(t), (char *)ti->udp_buf, ti->udp_buf->hdr_len, 0,
				(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	}

	return 0;
}

/*
	每个用户维护一个状态机，与服务器进行连接
*/

u_int32_t cl_main_thread(void *param)
{
	cl_thread_t thread;
	cl_thread_info_t *ti = &cl_priv->thread_main;
	ti->proc_notify = main_proc_notify;
	CL_THREAD_READ_ON(&cl_priv->master, ti->t_read, read_notify, NULL, ti->sock_notify);

	CL_THREAD_TIMER_ON(&cl_priv->master, cl_priv->t_check_wait, wait_check_timeout, NULL, 1000);
	CL_THREAD_TIMER_ON(&cl_priv->master, cl_priv->t_disp_time_out, disp_stat_timeout, NULL, TIME_N_SECOND(DISP_STAT_TIME_OUT));


	cl_priv->uasc_session = uasc_new();
	linkage_init();
	ir_ql_init();
	
	while (cl_thread_fetch(&cl_priv->master, &thread) != NULL) {
		//log_debug("cl_thread_fetch @%p\n", thread.func);
		cl_thread_call(&thread);
		
		if ( ti->stopping)
			break;
	}
	linkage_exit();
	ir_ql_exit();
	uasc_free(cl_priv->uasc_session);
    lan_dev_probe_exit(cl_priv);
	user_del_all();
	tmp_obj_del_all();
	CL_THREAD_OFF(cl_priv->t_check_wait);
	CL_THREAD_OFF(cl_priv->t_disp_time_out);
	cl_thread_stop(&cl_priv->master);
	log_info("Main thread exit now!\n");
	cl_destroy_thread(&cl_priv->thread_main);
	
	return 0;
}


