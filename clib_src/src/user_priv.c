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
#include "h264_decode.h"
#include "wait_server.h"
#include "cl_community.h"
#include "community_priv.h"
#include "equipment_priv.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "http.h"
#include "phone_user.h"
#include "lan_dev_probe_priv.h"
#include "bindphone_priv.h"
#include "cmd_misc.h"
#include "env_mon_priv.h"
#include "cl_env_mon.h"
#include "ia_priv.h"
#include "health_priv.h"
#include "notify_push_priv.h"
#include "uc_client.h"
#include "smart_appliance_priv.h"
#include "lbs.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "rfgw_priv.h"
#include "evm_priv.h"
#include "cl_dns.h"
#include "linkage_client.h"
#include "uc_agent.h"
#include "cl_app_pkt.h"
#include "app_pkt_priv.h"
#include "misc_client.h"
/********************打印调试开关********************打印调试开关***************
*************打印调试开关********************打印调试开关*******************/
#if 0
//文件控制宏,属于app调试用的一些稍微重要的问题件
#define DEBUG_IMPORTANT_FILE/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif


#define USER_DUMP_PKT


//分配服务器回复设备在不在线的标志
#define DEV_IS_ONLINE(x)		((x)&BIT(0))

static int timer_disp_die(cl_thread_t *t);
static void send_auth_q(user_t *user);
static void send_auth_k(user_t *user);
static void send_device_auth_k(user_t *user, pkt_t *pkt, net_header_t *hdr);
static void do_nickname_a(user_t *user, pkt_t *pkt);
static void do_apns_config_a(user_t *user, pkt_t *pkt);
static void do_bind_slave_info(user_t *user, pkt_t *pkt);
static void do_dev_newupver_a(user_t *user, pkt_t *pkt);
static void do_dev_upgrade_now_a(user_t *user, pkt_t *pkt);
static void do_805_config_cmd(user_t *user, pkt_t *pkt);
extern void ucc_del_share_code(ucc_session_t *s);
extern void user_qr_sync(user_t *user, char* qr_code);
extern void build_udp_sn_http_key(u_int64_t sn, char pu_key[64]);
extern bool app_is_la();
extern void la_modify_passwd_timer_reset();
bool rf_slave_init(user_t *user);
extern void do_passwd_sync_to_server(user_t *user);
extern void la_phone_login(char *name, char *passwd, bool is_ctrl);
extern bool get_dev_lan_sub_type(u_int64_t sn, u_int8_t *psub_type, u_int8_t *pext_type);
extern cl_slave_data_type _get_rf_dev_index_by_ext(u_int8_t ext_type);
extern bool can_send_disp();

void disp_stat_send(u_int32_t ip, u_int16_t port);
void disp_stat_recv(u_int32_t ip, u_int16_t port);
void disp_stat_dump();
u_int16_t disp_stat_get_valid_port(u_int32_t ip);

//RS pu_on_dev_upgrade_notify(pkt);

/**
	通过设备，来找到设备连接的服务器地址，如果实在找不到
	选择一个域名解析出来的ip
*/
u_int32_t user_select_ip(user_t *user)
{
	u_int32_t nip = 0;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	
	//  非直连，直接取得服务器IP
	if (user->devserver_ip && !user->direct_ip) {
		nip = user->devserver_ip;
		log_debug("use user's devserver_ip");
	} else if ((sac = user->smart_appliance_ctrl) != NULL && (air_ctrl = (smart_air_ctrl_t* )sac->sub_ctrl)) {
		if (air_ctrl->stat.server_ip) {
			nip = air_ctrl->stat.server_ip;
			log_debug("use stat's server_ip\n");
		}
	}

	if (nip == 0) {
		if (cl_priv->num_ip_disp_server == 0) {
			return nip;
		}

		// 随便找一个
		nip = cl_priv->ip_disp_server[0];
	}

	return nip;
}

static void user_free_send_list(user_t *user)
{
	pkt_t *pkt, *next;
	
	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &user->tcp_send_list, link) {
		stlc_list_del(&pkt->link);
		pkt_free(pkt);
	}

	user->tcp_send_len = 0;
}

static void do_disp_err(user_t *user, int err)
{
	if (err != ERR_SN_INVALID
		&& err != ERR_NICKNAME_INVALID
		&& err != ERR_PASSWD_INVALID
        && err != ERR_DEV_OFFLINE )
	{
		return;
	}
    
    
	user->last_err = err;
	user_set_status(user, CS_LOGIN_ERR);
}

static bool net_type_is_wifi()
{
	return (NET_TYPE_WIFI == cl_priv->net_type);
}

static void do_disp_err_proc(user_t *user)
{
	int i;
	int sn_invalid_num = 0;
	int sn_offline_num = 0;
	int err = ERR_DEV_OFFLINE;
	cl_disp_map_t *pmap = &user->disp_map;

	//判断逻辑，只有所有服务器为无效序列号时才为无效
	for(i = 0; i < pmap->num; i++) {
		log_debug("do_disp_err_proc pmap->last_err[%d]=%u pmap->num=%i\n", i, pmap->last_err[i], pmap->num);
		switch(pmap->last_err[i]) {
		case ERR_SN_INVALID:
			sn_invalid_num++;
			break;
		case ERR_NICKNAME_INVALID:
			err = ERR_NICKNAME_INVALID;
			goto done;
		case ERR_PASSWD_INVALID:
			err = ERR_PASSWD_INVALID;
			goto done;
		case ERR_DEV_OFFLINE:
			sn_offline_num++;
			break;
		}
	}

done:
	if ((pmap->num > 0) && (sn_invalid_num == pmap->num)) {
		err = ERR_SN_INVALID;
	} else if ((pmap->num > 0) && (sn_offline_num == pmap->num)) {
		err = ERR_DEV_OFFLINE;
	}

	user->last_err = err;

	//这里判断一下是否需要中转登陆
	if (!is_user_in_background_mode(user) && 
		(err == ERR_DEV_OFFLINE) &&
		net_type_is_wifi() &&
		can_send_disp() &&
		(cl_priv->trans_ip_num > 0)) {
		user->back_last_err = err;
		user->devserver_ip = cl_priv->trans_ip[0];
		user->devserver_port = 51192;
		user->need_trans_login = true;
		//只支持udp
		user->is_udp_ctrl = true;
		user_set_status(user, CS_DEV_CONNECTING);
		return;
	}

	user->need_trans_login = false;
	user_set_status(user, CS_LOGIN_ERR);
}

static void do_disp_err_sync(user_t *user, int err, u_int32_t ip)
{
	int i;
	bool all_recv = false;
	cl_disp_map_t *pmap = &user->disp_map;

	for(i = 0; i < pmap->num; i++) {
		if (ip == pmap->ip[i]) {
			pmap->last_err[i] = (u_int8_t)err;
		}
	}

	//判断下是否所有分配服务器回应全收到了错误
	for(i = 0; i < pmap->num; i++) {
		if (pmap->last_err[i] == 0) {
			break;
		}
	}
	if (i == pmap->num) {
		do_disp_err_proc(user);
	}
}

static void do_cmd_dev_oem_dn(user_t *user, uc_tlv_t* tlv)
{
	char *token, *p;
	char oem_dn[8][64] = {0};
	char (*dn_ptr)[64] = oem_dn;
	int num = 0;

	log_debug("get oem dn, tlv len %u\n", tlv->len);

	if (tlv->len == 0) {
		return;
	}

	p = (u_int8_t*)&tlv[1];
	for ((token = strtok(p, "\t")); token != NULL && num < 8; (token = strtok(NULL, "\t"))) {
		log_debug("oem dn [%s]\num", token);

		strncpy((u_int8_t*)&dn_ptr[num++], token, 64);
	}

	disp_resolv_doname(num, (const char (* const)[64])oem_dn);
}

static void do_cmd_bind_a_tlv(user_t *user, int length,uc_tlv_t* tlv)
{
	int total_len = length;
	tlv_devtype_t* td;
	
	if(length <= 0){
		return;
	}

	while(total_len >= (int)sizeof(uc_tlv_t)){
		tlv->len = htons(tlv->len);
		tlv->type = htons(tlv->type);

		log_debug("	sn %llu dispatcher tlv type %u len %u\n", tlv->type, tlv->len);

		switch(tlv->type){
			case DISP_TLV_DEV_TYPE:
				// 如果已经固化类型，不再被证书类型修改回去
				if (user->type_fixed) {
					break;
				}
				if(tlv->len >= sizeof(tlv_devtype_t)){
					td = (tlv_devtype_t*)(tlv+1);
					user->sub_type = td->subtype;
					user->ext_type = td->ext_type;
                    user->real_ext_type = td->ext_type;
                    user->real_sub_type = td->subtype;
					log_info("name %s sub_type %u ds_type %u ext_type %u\n",user->name,user->sub_type,
						user->ds_type,user->ext_type);
					sa_init(user);
				}
				break;
			case UCT_SCM_DEV_TYPE:
			case UCT_SCM_FACTORY_DATA:
			case UCT_DEVICE_NEW_DEV_EXT_TYPE:
			case UCT_DEVELOPER_ID:
				scm_do_dispatch_tlv(user,tlv);
				break;

			case UCT_DEV_OEM_DN:
				do_cmd_dev_oem_dn(user, tlv);
				break;
				
			default:
				break;
		}

		total_len -= (tlv->len + sizeof(*tlv));
		tlv = tlv_next(tlv);
	}
	
}

static void user_sync_reply_ip(user_t *user, u_int32_t ip)
{
	int i;
	
	cl_disp_map_t *pmap = &user->disp_map;

	for (i = 0; i < pmap->n_reply_ip; i++) {
		if (pmap->reply_ip[i] == ip) {
			return;
		}
	}

	if (pmap->n_reply_ip + 1 > MAX_DISP_SERVER_IP) {
		return;
	}

	log_debug("	add reply ip %u.%u.%u.%u\n", IP_SHOW(ip));

	pmap->reply_ip[pmap->n_reply_ip++] = ip;
}

static bool user_has_get_reply_ip(user_t *user, u_int32_t ip)
{
	int i;
	
	cl_disp_map_t *pmap = &user->disp_map;

	//log_debug("	n reply ip %u, query %u.%u.%u.%u\n", pmap->n_reply_ip, IP_SHOW(ip));

	for (i = 0; i < pmap->n_reply_ip; i++) {
		//log_debug("	idx %d %u.%u.%u.%u\n", i, IP_SHOW(pmap->reply_ip[i]));
		if (pmap->reply_ip[i] == ip) {
			return true;
		}
	}

	return false;
}

bool is_fobident_ip(u_int32_t ip)
{
	#if 0
	if (inet_addr("115.29.198.74") == ip)
		return true;

	if (inet_addr("114.55.7.155") == ip)
		return true;
#endif
	if (inet_addr("47.88.188.100") == ip)
		return true;

	return false;
}

RS user_proc_udp(user_t *user, pkt_t *pkt, struct sockaddr_in *peer_addr, bool is_uc_agent)
{
	net_header_t *hdr;
	net_device_location_t *loc;
	bool need_query_env = false;
	u_int8_t sub_type;
    u_int32_t ip;
    
	hdr = (net_header_t *)pkt->data;

    ip = ntohl(peer_addr->sin_addr.s_addr);
	log_info("user %s udp_read from %u.%u.%u.%u : command=%d, param len=%d\n",
		user->name, IP_SHOW(ip),hdr->command, hdr->param_len);

	nd_login_debug(NULL, "user %s udp_read from %u.%u.%u.%u : command=%d, param len=%d\n",
		user->name, IP_SHOW(ip),hdr->command, hdr->param_len);

	user->udp_recv_pkts++;

	// 记录返回的IP
	user_sync_reply_ip(user, ip);

	disp_stat_recv(ip, ntohs(peer_addr->sin_port));

	switch (hdr->command) {
	case CMD_USER_BIND_A:
		cl_priv->disp_recv = true;
		if (user->status != CS_DISP) {
			log_info("%s Ignore dispacher packet: in connecting now. stat %u\n", user->name, user->status);
			break;
		}
		
		if (hdr->param_len < sizeof(net_device_location_t)) {
			log_err(false, "%s Bad CMD_USER_BIND_A lenght, except %d bytes\n", user->name, sizeof(net_device_location_t));
			break;
		}
		loc = get_pkt_payload(pkt, net_device_location_t);
		loc->sn = ntoh_ll(loc->sn);
		loc->devserver_ip = ntohl(loc->devserver_ip);
		loc->devserver_port = ntohs(loc->devserver_port);
#ifdef USER_DUMP_PKT		
		log_info("%012"PRIu64" dev server ip=%u.%u.%u.%u, port=%u, loc->flag=%02x, version=%u.%u param len = %u\n", 
			loc->sn, IP_SHOW(loc->devserver_ip), loc->devserver_port,
			loc->flag,loc->major_ver, loc->minor_ver,hdr->param_len);
#endif
		nd_login_debug(NULL, "%012"PRIu64" dev server ip=%u.%u.%u.%u, port=%u, loc->flag=%02x, version=%u.%u param len = %u\n", 
			loc->sn, IP_SHOW(loc->devserver_ip), loc->devserver_port,
			loc->flag,loc->major_ver, loc->minor_ver,hdr->param_len);

//		if (user->sn != loc->sn)
//			need_query_env = true;
        //防止局域网扫描已更改了
        if (!user->is_udp_ctrl) {
            user->is_udp_ctrl = loc->is_udp;
        }
		sub_type = user->sub_type;
		do_cmd_bind_a_tlv(user,hdr->param_len-sizeof(net_device_location_t),(uc_tlv_t*)(loc+1));
        
            ajust_dev_sub_type_by_sn(user->sn,&user->sub_type,&user->ext_type);
            map_test_dev_sub_type(&user->sub_type,&user->ext_type);
		if(user->is_udp_ctrl && user->sub_type!= sub_type){
			//设备类型变化
			pu_save_dev_type(user);
		}
        
		user->sn = loc->sn;

#if 1
		//这里判断下设备在不在线
		if (!user->is_udp_ctrl &&
			!DEV_IS_ONLINE(loc->flag)) {
			//log_err(false, "UDP Dev[%012"PRIu64"] offline\n",user->sn);
			log_debug("UDP Dev[%012"PRIu64"] offline\n",user->sn);
			break;
		}
#endif

#ifdef UC_AGENT
		// 如果来自代理，且回复了有效IP和端口，那么这里先不忙去连接，等透传通道建立
		if (user->is_udp_ctrl && loc->devserver_ip && is_uc_agent == true) {
			nd_login_debug(NULL, "user %"PRIu64" get dev's server addr by agent, real ip %u.%u.%u.%u %u\n", 
				user->sn, IP_SHOW(loc->devserver_ip), loc->devserver_port);
			// 超时时间弄长一点
			CL_THREAD_TIMER_OFF(user->t_disp_die_timer);
	        CL_THREAD_TIMER_ON(&cl_priv->master, user->t_disp_die_timer, timer_disp_die, (void *)user, 2*1000);
			
			ua_request_transparent_agent(user->umgr, loc->devserver_ip, loc->devserver_port);
			break;
		}
#endif
		
#ifndef MUT_SERVER_ADAPT
		// 设备不在线
		user->devserver_ip = loc->devserver_ip;
		user->devserver_port = loc->devserver_port;
        if (user->is_udp_ctrl && (!user->devserver_ip || !user->devserver_port)) {
            log_err(false, "UDP Dev[%012"PRIu64"] offline\n",user->sn);
            do_disp_err(user,ERR_DEV_OFFLINE);
            break;
        }
#else
        if (user->is_udp_ctrl && (!loc->devserver_ip || !loc->devserver_port)) {
            //多服务器，设备离线先不处理，全部超时才报离线
            log_err(false, "UDP Dev[%012"PRIu64"] offline\n",user->sn);
            break;
        }
		user->devserver_ip = loc->devserver_ip;
		user->devserver_port = loc->devserver_port;
#endif


        CL_THREAD_TIMER_OFF(user->t_disp_die_timer);
		user_set_status(user, CS_DEV_CONNECTING);
		if (need_query_env)
			em_start(user);
		break;
		
	case CMD_FAIL:
		if (user->status == CS_DISP) {
			int err = 0;
			if (net_param_len(hdr) >= 4) {
				err = *get_pkt_payload(pkt, u_int32_t);
				err = ntohl(err);
#ifndef MUT_SERVER_ADAPT
				do_disp_err(user, err);
#else
				do_disp_err_sync(user, err, ip);
#endif
			}
			log_err(false, "%s Dispactch failed: param_len=%d, err=%d\n", user->name, net_param_len(hdr), err);
		}
		break;

	case CMD_NETWORK_DETECT_A:
		if (user->status == CS_ESTABLISH) {
			recv_net_detect_a(user, pkt);
		}
		break;
#ifndef	NO_COMMUNITY		
	case CMD_NOTIFY_HELLO:
		recv_notify_hello(user, pkt, peer_addr);
		break;
	case CMD_NOTIFY:
		recv_notify(user, pkt, peer_addr);
		break;
	case CMD_NOTIFY_RESULT:
		recv_notify_result(user, pkt, peer_addr);
		break;
#endif		
	}

	return RS_OK;
}

static int udp_read(cl_thread_t *thread)
{
	user_t *user = (user_t *)CL_THREAD_ARG(thread);
	net_header_t *hdr;
	pkt_t *pkt;
	struct sockaddr_in addr;
	socklen_t len_addr;

	user->t_read_udp = NULL;

	// 只要收到一个包，恢复外网离线标志
	cl_priv->net_offline = false;
    
    if (user->sock_udp > 0) {
        CL_THREAD_READ_ON(&cl_priv->master, user->t_read_udp, udp_read, (void *)user, user->sock_udp);
        log_debug("udp_read add udp read %s %d\n",__FUNCTION__,__LINE__);
    }else{
        log_debug("udp_read no udp read %s %d\n",__FUNCTION__,__LINE__);
    }

	len_addr = sizeof(addr);
	pkt = user->udp_buf;
	pkt->total = (int)recvfrom(user->sock_udp, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, &len_addr);
	if (pkt->total < (int)net_header_size) {
		log_err(false, "%012"PRIu64" ignore bad udp packet: sock=%u, len=%d\n", user->sn, user->sock_udp, pkt->total);
		return 0;
	}

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
    
#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(true, pkt->total);
#endif

	cl_lock(&cl_priv->mutex);
	user_proc_udp(user, pkt, &addr, false);
	cl_unlock(&cl_priv->mutex);

	return 0;
}

/*
	Windows: 比较麻烦，还要看exceptfds，算了，等超时
		If the client uses the select function, success is reported in the writefds set and failure
		is reported in the exceptfds set
	Linux:
		It is possible to select(2) or poll(2) for completion by  selecting
		the  socket  for  writing.  After select(2) indicates writability, use getsockopt(2)
		to read the SO_ERROR option at  level  SOL_SOCKET  to  determine
		whether  connect()  completed successfully (SO_ERROR is zero) or unsuccess-
		fully (SO_ERROR is one of the usual error codes listed here, explaining the
		reason for the failure).	
*/
RS tcp_check_connecting(user_t *user)
{
	int status;
	int slen;
	int ret;

	CL_THREAD_OFF(user->t_read);
	CL_THREAD_OFF(user->t_write);

	slen = sizeof(status);
	ret = (int)getsockopt(user->sock_dev_server, SOL_SOCKET, SO_ERROR, (void *)&status, &slen);
	if (ret < 0) {
		log_err(true, "tcp_check_connecting %s getsockopt failed\n", user->name);
		user_set_status(user, CS_IDLE);
		return RS_ERROR;
	}
	if (status != 0) {
		log_err(false, "tcp_check_connecting %s status=%d\n", user->name, status);
		user_set_status(user, CS_IDLE);
		return RS_ERROR;
	}

	user_set_status(user, CS_AUTH);

	return RS_OK;
}



void send_disp(user_t *user)
{
	pkt_t *pkt;
	u_int16_t valid_port = 0;
	net_para_user_bind_q *ub;
	cl_disp_map_t *pmap = &user->disp_map;

#ifndef MUT_SERVER_ADAPT
    int i,len,r;
#else
    int i, len, r , k , cnt;
	static u_int16_t disp_ports[3] ={
    	DISP_SERVER_PORT2,
    	DISP_SERVER_PORT1,
    	DISP_SERVER_PORT,
	};
#endif
	
	struct sockaddr_in addr;
    
	if (user->last_run_in_background != cl_priv->run_in_background) {
		user->last_run_in_background = cl_priv->run_in_background;
		//变到前台时清除下失败记录
		if (!user->last_run_in_background) {
			user->disp_die_num = 0;
		}
	}    
    
    if (is_user_in_background_mode(user)) {
        return;
    }
	
	pkt = pkt_new(CMD_USER_BIND_Q, sizeof(net_para_user_bind_q),user->ds_type);
	if (pkt == NULL) {
		return;
	}
	
	((net_header_t *)pkt->data)->handle = user->handle;
	ub = get_pkt_payload(pkt, net_para_user_bind_q);
    if (user->is_udp_ctrl) {
        sprintf((char*)ub->username, "%012"PRIu64"",user->sn);
        
        log_info("user dispatch %s is udp sn[%012"PRIu64"]\n", user->name,user->sn);
    }else{
        strncpy((char *)ub->username, user->name, sizeof(ub->username));
    }
	ub->major_ver = VERSION_MAJOR;
	ub->minor_ver = VERSION_MINOR;
	ub->client_id = cl_priv->cleint_type;

    if (user_create_udp(user) != RS_OK){
        goto err;
    }

	memset(&addr, 0, sizeof(addr));
#ifdef MUT_SERVER_ADAPT
    cnt = sizeof(disp_ports)/sizeof(u_int16_t);
#endif

	pmap->num = cl_priv->num_ip_disp_server;
	for (i = 0; i < (int)cl_priv->num_ip_disp_server; i++) {
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(cl_priv->ip_disp_server[i]);
#ifndef MUT_SERVER_ADAPT
		addr.sin_port = htons(DISP_SERVER_PORT);
#endif
		len = sizeof(addr);
        pmap->ip[i] = cl_priv->ip_disp_server[i];

		// 已经回复的服务器，不需要再发了
		if (user_has_get_reply_ip(user, pmap->ip[i]) == true) {
			log_debug("has get reply ip %u.%u.%u.%u\n", IP_SHOW(pmap->ip[i]));
			continue;
		}
				
#ifndef MUT_SERVER_ADAPT
		r = (int)sendto(user->sock_udp, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len);
#ifdef USER_DUMP_PKT
		log_debug("'%s' socket %d send dispacher request to %u.%u.%u.%u port %d, ret=%d\n",
			user->name, user->sock_udp,IP_SHOW(cl_priv->ip_disp_server[i]), DISP_SERVER_PORT, r);
#endif
#else
		valid_port = disp_stat_get_valid_port(cl_priv->ip_disp_server[i]);
		for (k = 0; k < cnt; k++) {
	        addr.sin_port = htons(disp_ports[k]);
			//只允许接收到回复的端口再次发送
			if (valid_port && valid_port != disp_ports[k]) {
				continue;
			}
			disp_stat_send(cl_priv->ip_disp_server[i], disp_ports[k]);

#ifdef UC_AGENT
			// 这里需要判断一下,如果对应服务器没回复，且代理申请好了。就用代理发
			if (ua_trans_send(user->umgr, cl_priv->ip_disp_server[i], disp_ports[k], pkt->data, pkt->total) == RS_OK) {
				continue;
			}
#endif
      	  r = (int)sendto(user->sock_udp, pkt->data, pkt->total, 0, (struct sockaddr *)&addr, len);
#ifdef USER_DUMP_PKT
     	   log_debug("'%s' socket %d send dispacher request to %u.%u.%u.%u port %d, ret=%d\n",
    	              user->name, user->sock_udp,IP_SHOW(cl_priv->ip_disp_server[i]), ntohs(addr.sin_port), r);
#endif
      }
#endif
        
#ifdef SUPPORT_TRAFFIC_STAT
        UDP_PKT_STAT(false, pkt->total);
#endif
	}
err:
	pkt_free(pkt);
}

int timer_die(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);

	user->t_timer = NULL;
	if (user->is_phone_user) {
        if (is_user_in_background_mode(user)) {
            CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void*)user, 5000);
        }else{
            pu_auth(user);
            CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void*)user, pu_get_timeout(user));
        }
		
	} else {
		user_set_status(user, CS_IDLE);
	}

	return 0;
}

static int timer_idle(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);

	user->t_timer = NULL;
	if (user->is_phone_user) {
		user_set_status(user, CS_AUTH);
	} else {
        if (user->is_udp_ctrl && user->direct_ip != 0) {
            user_set_status(user, CS_AUTH);
        }else{
            user_set_status(user, CS_DISP);
        }
	}

	return 0;
}
static int timer_disp_die(cl_thread_t *t)
{
    user_t *user = (user_t *)CL_THREAD_ARG(t);
    
    user->t_disp_die_timer = NULL;

#ifndef MUT_SERVER_ADAPT
    do_disp_err(user,ERR_DEV_OFFLINE);
#else
	do_disp_err_proc(user);
#endif

	user->disp_die_num++;

	// 一个包都不回，外网不通
	if (user->disp_map.n_reply_ip == 0 && !(is_user_in_background_mode(user))) {
		cl_priv->net_offline = true;
	}

	return 0;
}

static int timer_uc_agent(cl_thread_t *t)
{
	struct sockaddr_in addr;
	user_t *user = CL_THREAD_ARG(t);
	cl_disp_map_t *disp_map = &user->disp_map;

	user->t_uc_agent = NULL;

	log_debug("timer_uc_agent\n");
	
	// 看是否有超时的服务器

	if (disp_map->num == disp_map->n_reply_ip) {
		return 0;
	}

	log_debug("there has server no reply, total %u reply %u\n", disp_map->num, disp_map->n_reply_ip);

	// 找不到做代理的服务器，网络不通
	if (disp_map->n_reply_ip == 0) {
		log_err(false, "has no server ready for agent\n");
		return 0;
	}

	ua_create((ua_mgr_t**)&user->umgr, user);
	if (user->umgr == NULL) {
		return 0;
	}

	log_debug("create uamgr %p\n", user->umgr);

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	//addr.sin_addr.s_addr = inet_addr("10.133.3.2");
	addr.sin_addr.s_addr = ntohl(disp_map->reply_ip[0]);
	
	ua_set_master(user->umgr, (struct sockaddr *)&addr);
	
	ua_request_ipinip_agent(user->umgr);
	
	return 0;
}

//每秒速度限制
#define DISP_SPEED_LIMIT_NUM	(20)
static bool disp_speed_is_limit()
{
	static u_int32_t last_time = 0;
	static u_int32_t num = 0;
	u_int32_t now = get_sec();

	if (last_time != now) {
		last_time = now;
		if (num) {
			log_debug("now=%u num=%u\n", now, num);
		}
		num = 0;
	}

	num++;
	if (num >= DISP_SPEED_LIMIT_NUM) {
		return true;
	}

	return false;
}
	
static int timer_send_disp(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);

	user->t_timer = NULL;

	if (user->direct_ip != 0) {
		user_set_direct_login(user, user->direct_ip);
		return 0;
	}
	
	//CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_send_disp, (void *)user, range_rand(TIME_TRY_DISP, 1000));
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_send_disp, (void *)user, TIME_TRY_DISP*2);
	cl_lock(&cl_priv->mutex);
	if ((cl_priv->num_ip_disp_server > 0) && 
		can_send_disp() && 
		!disp_speed_is_limit()) {
		send_disp(user);
		user->send_disp_num++;
	} else {
		user->dis_send_disp_num++;
	}
	cl_unlock(&cl_priv->mutex);

	return 0;
}

static int timer_keeplive(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);
	pkt_t *pkt;

	user->t_timer_keeplive = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_keeplive, timer_keeplive, (void *)user, TIME_KEEPLIVE);

	pkt = pkt_new(CMD_ECHO_Q, 0, user->ds_type);
	user_add_pkt(user, pkt);

	return 0;	
}


///////////////////////////////////////////////////////////////////////////

const char *err_2_str(int ecode)
{
	const char *e;
	static char buf[64];
	
	switch (ecode) {
	case ERR_DEV_OFFLINE:
		e = "设备离线";
		break;
	case ERR_VEDIO_OFF:
		e = "未插入摄像头";
		break;
	case ERR_DEV_SYS_ERR:
		e = "设备系统错误";
		break;
	case ERR_TOO_MANY:
		e = "太多人在同时观看该视频了";
		break;
	case ERR_OUT_SERVICE:
		e = "服务到期";
		break;
	case ERR_TIMEOUT:
		e = "超时失败";
		break;
	case ERR_AGENT:
		e = "代理失败";
		break;
	case ERR_NO_VTAP:
		e = "找不到录像文件";
		break;
	case ERR_VTAP_CLIENT_EXCEED:
		e = "太多人同时观看录像了";
		break;
	case ERR_SN_INVALID:
		e = "无效的序列号";
		break;
	default:
		sprintf(buf, "错误号为%d", ecode);
		e = buf;
		break;
	}	

	return e;
}

static void do_805_config_cmd(user_t *user, pkt_t *pkt)
{
	misc_hdr_t* mh;
	net_805_screen_t* ns;
	net_805_beep_t* nb;
	net_805_config_hdr_t* c_hdr;
	wait_t *w;
	slave_t* slave;
	int event = 0;

	w = wait_lookup(PKT_HANDLE(pkt));
	if (w == NULL || w->obj_type != HDLT_SLAVE) {
		log_err(false, "do_805_config_cmd ignore not found slave by handle=0x%08x\n", PKT_HANDLE(pkt));
		return ;
	}

	slave = slave_lookup_by_handle(user, w->obj_handle);
	if(slave == NULL){
		log_err(false, "do_805_config_cmd ignore not found slave by handle=0x%08x\n", PKT_HANDLE(pkt));
		return;
	}

	if (net_param_len(pkt->data) < sizeof(net_805_config_hdr_t)) {
		log_err(false, "do_remote_config failed: param len=%d\n", net_param_len(pkt->data));
		return;
	}

	c_hdr = get_pkt_payload(pkt, net_805_config_hdr_t);
	c_hdr->err = ntohl(c_hdr->err);
	
	if(c_hdr->err!= ERR_NONE){
		event = UE_CTRL_805_FAILED;
	}else{
		mh = (misc_hdr_t*)(c_hdr+1);
		mh->type = ntohs(mh->type);
		mh->len = ntohs(mh->len);
		if(mh->type == NET_805_BEEP){
			if(mh->len >= sizeof(*nb)){
				nb = (net_805_beep_t*)(mh+1);
				slave->is_805_beep_on = !!nb->onoff;
				event = UE_INFO_MODIFY;
			}
		}else if(mh->type == NET_805_SCREEN){
			if(mh->len >= sizeof(*ns)){
				ns = (net_805_screen_t*)(mh+1);
				slave->is_805_screen_on= !!ns->onoff;
				event = UE_INFO_MODIFY;
			}
		}
		
	}

	if(event !=0){
		event_push(user->callback, event, user->handle, user->callback_handle);
	}
	
}


// 返回; bool: 处理了该报文. false: 需要其他模块继续处理处理
bool user_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	u_int32_t err_code = 0;

	user->tcp_recv_pkts++;

	switch (hdr->command) {
	case CMD_USER_AUTH_A:
		if(user->bp->is_binding == 0)
			send_auth_k(user);
		else 
			send_bind_phone_q(user);
		break;
	case CMD_MISC_A:
		do_misc_a(user, pkt);
		break;
	case CMD_NOTIFY:
		recv_notify(user, pkt, NULL);
		break;	
	case CMD_NOTIFY_CENTER_LIST:
		recv_cmt_center_list(user, pkt);
		break;

	case CMD_NICKNAME_A:
		do_nickname_a(user, pkt);
		break;
		
	case CMD_PHONE_BIND_RESULT:
		recv_bind_phone_result(user, pkt);
		break;
		
	case CMD_PHONE_REQUESTLIST_A:
		recv_bind_phone_request_list(user, pkt);
		break;
		
	case CMD_PHONE_BINDLIST_A:
		recv_bind_phone_list(user, pkt);
		break;
		
	case CMD_PHONE_APN_OPERATION:
		do_apns_config_a(user, pkt);
		break;

	case CMD_BIND_SLAVE_INFO:
		do_bind_slave_info(user, pkt);
		break;
		
	case CMD_NEWUPVER_A:
		do_dev_newupver_a(user, pkt);
		break;
		
	case CMD_NOTICE_DEVUP:
		do_dev_upgrade_now_a(user, pkt);
		break;
	case CMD_805_CONFIG:
		do_805_config_cmd(user,pkt);
		break;

#ifndef	NO_COMMUNITY
	case CMD_AUTH_A:
		send_device_auth_k(user, pkt, hdr);
		break;
	case CMD_CMT_OP_DEVICE:
		do_cmt_op_device(user, pkt);
		break;
	case CMD_NOTIFY_RESULT:
		recv_notify_result(user, pkt, NULL);
		break;
	case CMD_NOTIFY_EXPECT:
		recv_notify_expect(user, pkt, NULL);
		break;
#endif

	default:
		return false;
	}

	return true;
}


static RS connect_dev_server(user_t *user)
{
	bool inprogress;
	
	CLOSE_SOCK(user->sock_dev_server);

	user->sock_dev_server = connect_tcp(user->devserver_ip, user->devserver_port, &inprogress);
	if (user->sock_dev_server == INVALID_SOCKET) {
		user_set_status(user, CS_IDLE);
		return RS_ERROR;
	}

	//CL_THREAD_OFF(user->t_read_udp);
	CL_THREAD_READ_ON(&cl_priv->master, user->t_read, tcp_read, (void *)user, user->sock_dev_server);
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void *)user, TIME_CONNECT);

	if ( inprogress ) {
		CL_THREAD_WRITE_ON(&cl_priv->master, user->t_write, tcp_write, (void *)user, user->sock_dev_server);
	} else {
		user_set_status(user, CS_AUTH);
	}
		
	return RS_OK;
}

static void send_user_auth_q(user_t *user)
{
	pkt_t *pkt;
	u_int32_t *ip;
	struct sockaddr_in addr;
	int slen = sizeof(struct sockaddr_in);

	memset(&addr, 0, sizeof(addr));
	getsockname(user->sock_dev_server, (struct sockaddr *)&addr, &slen);
	user->local_ip = ntohl(addr.sin_addr.s_addr);
	
	pkt = pkt_new(CMD_USER_AUTH_Q, sizeof(u_int32_t), user->ds_type);
	PKT_HANDLE(pkt) = user->handle;
	ip = get_pkt_payload(pkt, u_int32_t);
	*ip = addr.sin_addr.s_addr;

	user_add_pkt(user, pkt);
}

#ifndef	NO_COMMUNITY
static void send_device_auth_q(user_t *user)
{
	pkt_t *pkt;
	u_int8_t *cert;
	struct sockaddr_in addr;
	int slen = sizeof(struct sockaddr_in);

	memset(&addr, 0, sizeof(addr));
	getsockname(user->sock_dev_server, (struct sockaddr *)&addr, &slen);
	user->local_ip = ntohl(addr.sin_addr.s_addr);
	
	pkt = pkt_new(CMD_AUTH_Q, user->dskey->license_hd.cert_len, user->ds_type);
	PKT_HANDLE(pkt) = user->handle;
	cert = get_pkt_payload(pkt, u_int8_t);
	memcpy(cert, user->dskey->cert, user->dskey->license_hd.cert_len);
	user_add_pkt(user, pkt);
}
#endif	

static void send_auth_q(user_t *user)
{
#ifndef	NO_COMMUNITY		
	if(user->dskey == NULL){
#endif		
		send_user_auth_q(user);
#ifndef	NO_COMMUNITY		
	}else{
		send_device_auth_q(user);
	}
#endif	
}
static void send_auth_k(user_t *user)
{
	pkt_t *pkt;
	auth_a_t *auth_a;
	net_para_user_auth_v2_t *p;
	MD5_CTX md5_ctx;

	auth_a = get_pkt_payload(user->tcp_buf, auth_a_t);
	user->global_ip = ntohl(auth_a->global_ip);

	pkt = pkt_new(CMD_USER_AUTH_K, sizeof(*p), user->ds_type);
	p = get_pkt_payload(pkt, net_para_user_auth_v2_t);

	PKT_HANDLE(pkt) = user->handle;
	p->sn = ntoh_ll(user->sn);

	MD5Init(&md5_ctx);
	MD5Update(&md5_ctx, user->passwd_md5, 16);
	MD5Update(&md5_ctx, auth_a->auth, 32);
	MD5Final(p->md5, &md5_ctx);
	memcpy(p->uuid, cl_priv->uuid, sizeof(p->uuid));
	p->reserved[0] = 0;
	p->reserved[1] = 0;
	p->reserved[2] = 0;
	p->reserved[3] = 0;

	user_add_pkt(user, pkt);
}

#ifndef	NO_COMMUNITY
static void send_device_auth_k(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	pkt_t *pkt_reply;
	u_int8_t *secure_data;
	u_int8_t skey[256];
	net_auth_k_t *p;
	MD5_CTX md5_ctx;	
	int ret;

	secure_data = get_pkt_payload(user->tcp_buf, u_int8_t);

	ret = RSA_private_decrypt(hdr->param_len, secure_data, skey, 
		user->dskey->rsa_private, RSA_PKCS1_PADDING); /*RSA_private_decrypt is not thread safe*/
	if(ret != SKEY_SIZE){
		log_err(false, "RSA_private_decrypt skey failed, %d\n", ret);
		goto error_out;
	}
	memcpy(user->skey, skey, SKEY_SIZE);
	
	pkt_reply = pkt_new(CMD_AUTH_K, sizeof(net_auth_k_t), user->ds_type);
	if(pkt_reply == NULL)
		goto error_out;
	p = get_pkt_payload(pkt_reply, net_auth_k_t);

	MD5Init(&md5_ctx);
	MD5Update(&md5_ctx, skey, SKEY_SIZE);
	MD5Final(p->md5, &md5_ctx);
	p->local_ip = htonl(user->local_ip);

	PKT_HANDLE(pkt_reply) = user->handle;

	user_add_pkt(user, pkt_reply);

error_out:
	return;
}
#endif

static void user_off_all_timer(user_t *user)
{
	slave_t *slave;
	video_t *video;

	log_debug("user off all timer\n");
	
	CL_THREAD_OFF(user->t_timer);
	CL_THREAD_OFF(user->t_timer_keeplive);
	CL_THREAD_OFF(user->t_timer_query_master);
	CL_THREAD_OFF(user->t_timer_query_slave);
	CL_THREAD_OFF(user->t_timer_query_link_scene);
	CL_THREAD_OFF(user->t_disp_die_timer);
	CL_THREAD_OFF(user->t_uc_agent);
	//CL_THREAD_OFF(user->t_timer_query_env);

	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		video = slave->video;
		video_off_all_thread(video, true);
	}
}

static void user_do_offline(user_t *user)
{
#if 0	
	slave_t *slave, *next;
	
	user_off_all_timer(user);
	
	cl_lock(&cl_priv->mutex);

	// 从user_free中抽取部分出来
	//user_free(user);

	CL_THREAD_OFF(user->t_read_udp);
	CL_THREAD_OFF(user->t_read);
	CL_THREAD_OFF(user->t_write);

	user_free_send_list(user);

	stlc_list_for_each_entry_safe(slave_t, slave, next, &user->slave, link) {
		stlc_list_del(&slave->link);
		slave_free(slave);
	}
    key_learn_free(user);
	alarm_phone_free(user);
	
	cl_unlock(&cl_priv->mutex);
	
	event_push(user->callback, UE_INFO_MODIFY, (void *)user->handle, user->callback_handle);
#endif	
}

void user_set_status_uc(user_t *user, u_int32_t status, bool from_uc)
{
	u_int32_t next_timer;
	u_int32_t prev_satus = user->status;

	user->status = status;
	log_info("'%s'[uc] modify status from %d to %d last_err=%u err_backup=%u\n", 
		user->name, prev_satus, status, user->last_err, user->err_backup);

	// 
	if (prev_satus != CS_ESTABLISH && status == CS_ESTABLISH) {
		event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
		event_push(user->callback, user->online ? UE_LOGIN_ONLINE : UE_LOGIN_OFFLINE, user->handle, user->callback_handle);
		event_cancel_merge(user->handle);
		user->last_enstablish = false;
		user->err_backup = 0;
	} else	if (prev_satus == CS_ESTABLISH && status != CS_ESTABLISH) {
		user->online = false;
//		event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
//		user->last_err = ULGE_NETWORK_ERROR;
		user->last_enstablish = true;
        user->udp_recv_pkts = 0;
		//如果是联动，且是设备主动reset的，就不报这个离线事件了
//		if (!app_is_la() 
//			|| (!user->is_reset_active)) {
//			//这里，暂时的超时离线不报离线，报网络不给力，后面设备重新登陆，当收到服务器确认离线消息后再报离线
//			event_push_err(user->callback, UE_LOGIN_ERROR, user->handle, user->callback_handle, ULGE_NETWORK_ERROR);
//			event_cancel_merge(user->handle);
//		}
		
		user_do_offline(user);
	} else if (status == CS_LOGIN_ERR) {
        if(user->last_err != user->err_backup){
            user->err_backup = user->last_err;
            event_push_err(user->callback, UE_LOGIN_ERROR, user->handle, user->callback_handle, user->last_err);
            //		event_cancel_merge(user->handle);
            user->notify_login_err_count++;
        }
	}

	user->is_reset_active = false;

	switch (status) {
	case CS_IDLE:
		user_off_all_timer(user);
		CL_THREAD_READ_OFF(user->t_read_udp);
		CLOSE_SOCK(user->sock_udp);
        CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_IDLE);
#ifdef UC_AGENT		
		ua_init(user->umgr);
#endif
        if ( ! from_uc )
                ucc_set_status(user->uc_session, UCCS_ERROR);
		break;

	case CS_LOGIN_ERR:
		user_off_all_timer(user);
		CL_THREAD_READ_OFF(user->t_read_udp);
		CLOSE_SOCK(user->sock_udp);
            
        if ( ! from_uc ){
            ucc_set_status(user->uc_session, UCCS_ERROR);
            if (user->last_err != ULGE_DEV_CLONE &&
                user->last_err != ULGE_BAD_PASSWORD &&
                user->last_err != ULGE_BAD_SN) {
                if (user->direct_ip != 0) {
                    CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(3));
                }else{
       	            log_debug("sn=%"PRIu64" user->send_disp_num=%u dis_send_disp_num=%u\n", 
						user->sn, user->send_disp_num, user->dis_send_disp_num);
					next_timer = min(60, (20 + ((user->disp_die_num > 3)?(user->disp_die_num - 3):0) * 10));
		            if (user->send_disp_num < 3) {
						CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(5));
					} else {
						if (!net_type_is_wifi()) {
		            		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(next_timer));
						} else {
							CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(20));
						}
					}
                }
            }
        }
		break;
		
	case CS_DISP:
		user_off_all_timer(user);
        CL_THREAD_READ_OFF(user->t_read_udp);
        CLOSE_SOCK(user->sock_udp);
		//清除一下disp分配服务器返回的错误值，主要是为了处理多国服务器报错的问题
		memset((void *)&user->disp_map, 0, sizeof(user->disp_map));
		user->send_disp_num = 0;
		user->dis_send_disp_num = 0;
		user->need_trans_login = false;

		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_IDLE);
#ifdef UC_AGENT		
		ua_init(user->umgr);
	log_debug("	start timer_uc_agent\n");
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_uc_agent, timer_uc_agent,
				   (void *)user, TIME_N_SECOND(2));

#endif
        if ( ! from_uc )
            ucc_set_status(user->uc_session, UCCS_ERROR);
        CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_send_disp,
                           (void *)user, range_rand(0, 500));
        CL_THREAD_TIMER_ON(&cl_priv->master, user->t_disp_die_timer, timer_disp_die,
                           (void *)user, 5*1000 + range_rand(0, 1000));
		
		break;

	case CS_DEV_CONNECTING:
		user_off_all_timer(user);
		if ( ! from_uc )
			ucc_set_status(user->uc_session, UCCS_AUTH_REQ);
		break;

	case CS_AUTH:
		// 
		user_off_all_timer(user);
		if ( ! from_uc )
			ucc_set_status(user->uc_session, UCCS_AUTH_REQ);
		break;

	case CS_ESTABLISH:
		user->last_err = 0;
		user->never_establish = false;
        if (user->is_udp_ctrl) {
            sa_init(user);
        }
		break;
		
	default:
		log_err(false, "%s is udp control, unknow status=%d.\n", user->name, status);
		break;
	}
}

void user_set_status(user_t *user, u_int32_t status)
{
	u_int32_t next_timer;
	u_int32_t prev_satus = user->status;

	// 
	if (user->is_udp_ctrl) {
		user_set_status_uc(user, status, false);
		return;
	}
	
	user->status = status;
	log_info("'%s' modify status from %d to %d\n", user->name, prev_satus, status);

	// 设置事件来回调，避免加锁回调
	if (prev_satus != CS_ESTABLISH && status == CS_ESTABLISH) {
		if(user->ds_type == TP_DS007) {
			event_push(user->callback, UE_LOGIN_ONLINE, user->handle, user->callback_handle);
		} else {
			event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
			event_push(user->callback, user->online ? UE_LOGIN_ONLINE : UE_LOGIN_OFFLINE, user->handle, user->callback_handle);
		}
	} else	if (prev_satus == CS_ESTABLISH && status != CS_ESTABLISH) {
		user->online = false;
		user->last_err = 0;
		if(user->ds_type == TP_DS007) { /* 小区，模拟设备登录 */
			event_push(user->callback, UE_LOGIN_OFFLINE, user->handle, user->callback_handle);
		} else { /* 客户端用户 */
			event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
			event_push(user->callback, UE_LOGOUT, user->handle, user->callback_handle);
			user_do_offline(user);
		}
		event_cancel_merge(user->handle);
	} else if (status == CS_LOGIN_ERR) {
		event_push_err(user->callback, UE_LOGIN_ERROR, user->handle, user->callback_handle, user->last_err);
		event_cancel_merge(user->handle);
		user->notify_login_err_count++;
	}

	switch (status) {
	case CS_IDLE:
#ifdef UC_AGENT		
		ua_init(user->umgr);
#endif
		user_off_all_timer(user);
		CL_THREAD_OFF(user->t_read);
		user->tcp_recv_len = 0;
		CL_THREAD_OFF(user->t_write);
		user_free_send_list(user);

		if(user->ds_type != TP_DS007){
			CL_THREAD_OFF(user->t_read_udp);
			CLOSE_SOCK(user->sock_udp);
		}
		CLOSE_SOCK(user->sock_dev_server);
		
		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_IDLE);
		break;

	case CS_LOGIN_ERR:
		user_off_all_timer(user);
		CL_THREAD_OFF(user->t_read);
		user->tcp_recv_len = 0;
		CL_THREAD_OFF(user->t_write);
		user_free_send_list(user);
		
		if (user->ds_type != TP_DS007) {
			CL_THREAD_OFF(user->t_read_udp);
			CLOSE_SOCK(user->sock_udp);
		}
		CLOSE_SOCK(user->sock_dev_server);
		if (user->is_pud_authing) {
			pu_on_dev_login(user, false);
		}
		//避免直接挂了
        if (user->last_err != ULGE_BAD_PASSWORD &&
            user->last_err != ULGE_BAD_SN) {
            log_debug("sn=%"PRIu64" user->send_disp_num=%u dis_send_disp_num=%u\n", 
				user->sn, user->send_disp_num, user->dis_send_disp_num);
			next_timer = min(60, (20 + ((user->disp_die_num > 3)?(user->disp_die_num - 3):0) * 10));
            if (user->send_disp_num < 3) {
				CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(5));
			} else {
				if (!net_type_is_wifi()) {
            		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(next_timer));
				} else {
					CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_idle, (void *)user, TIME_N_SECOND(20));
				}
			}
        }
		break;
		
	case CS_DISP:	
		user_off_all_timer(user);
		// 第一次快一点
		//清除一下disp分配服务器返回的错误值，主要是为了处理多国服务器报错的问题
		memset((void *)&user->disp_map, 0, sizeof(user->disp_map));
		user->send_disp_num = 0;
		user->dis_send_disp_num = 0;
		user->need_trans_login = false;

		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_send_disp,
			(void *)user, range_rand(0, 500));
        CL_THREAD_TIMER_ON(&cl_priv->master, user->t_disp_die_timer, timer_disp_die,
                           (void *)user, 5*1000 + range_rand(0, 1000));

#ifdef UC_AGENT		
		ua_init(user->umgr);
		log_debug(" start timer_uc_agent in user_set_status\n");
		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_uc_agent, timer_uc_agent,
				  (void *)user, TIME_N_SECOND(2));
#endif	

		user->tcp_recv_len = 0;
		user_free_send_list(user);
		break;

	case CS_DEV_CONNECTING:
		user_off_all_timer(user);
		connect_dev_server(user);
		break;

	case CS_AUTH:
		// 重设超时
		user_off_all_timer(user);
		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void *)user, TIME_CONNECT);
		if (user->is_phone_user) {
			pu_auth(user);
		} else {
			send_auth_q(user);
			CL_THREAD_READ_ON(&cl_priv->master, user->t_read, tcp_read, (void *)user, user->sock_dev_server);
		}
		break;

	case CS_ESTABLISH:
		user->last_err = 0;
		user->never_establish = false;
		if (user->is_phone_user) {
			CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void *)user, pu_get_timeout(user));
		} else {
			CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer, timer_die, (void *)user, TIME_CLIENT_DIE);
			CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_keeplive, timer_keeplive, (void *)user, TIME_KEEPLIVE);
			// 第一次快点
			if(user->ds_type == TP_USER)
				CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_master, timer_query_master, (void *)user, 1);
#ifndef	NO_COMMUNITY		
			else
				send_notify_center_list(user);
#endif		
			if (user->is_pud_authing) {
				pu_on_dev_login(user, true);
			}
            
            if (!user->is_udp_ctrl) {
                ia_init(user);
            }
            
            notify_push_proc_android_msg(user);
		}
		break;
		
	default:
		break;
	}
}

RS user_create_udp(user_t *user)
{
	int len;
	struct sockaddr_in addr;
	
	if (IS_INVALID_SOCK(user->sock_udp)) {
		user->sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		if(user->ds_type == TP_DS007)
			addr.sin_port = ntohs(CMT_PORT);
		if (bind(user->sock_udp, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0) {
			log_err(true, "bind notify socket failed\n");
			CLOSE_SOCK(user->sock_udp);
			goto err;
		}
		len = sizeof(addr);
		if (getsockname(user->sock_udp, (struct sockaddr *)&addr, &len) != 0) {
			log_err(true, "bind notify socket failed\n");
			CLOSE_SOCK(user->sock_udp);
			goto err;
		}
		user->sock_udp_port = ntohs(addr.sin_port);
		SET_SOCK_NBLOCK(user->sock_udp);
        
        CL_THREAD_READ_ON(&cl_priv->master, user->t_read_udp, udp_read, (void *)user, user->sock_udp);
		
		log_info("'%s' create udp sock fd = %d, port = %d, max_fd = %d @t_read_udp = %p udp_read=%p \n", user->name, user->sock_udp, user->sock_udp_port, cl_priv->master.max_fd,user->t_read_udp,udp_read);
        
        log_debug("udp_read add udp read %s %d\n",__FUNCTION__,__LINE__);
	}

	return RS_OK;
	
err:
	CLOSE_SOCK(user->sock_udp);
	return RS_ERROR;
}

bool is_user_in_background_mode(user_t* user)
{
    //手机运行在后台
    if (cl_priv->run_in_background) {
        return true;
    }
    //手机用户下的用户
    if (user->parent) {
        return user->parent->background|user->background;
    }
    
    return user->background;
}

static user_t *user_alloc()
{
	user_t *user;

	user = cl_calloc(sizeof(user_t), 1);
	if(user == NULL)
		return NULL;
	user->ext_type = 0xFF;
	user->user_cmt = cl_calloc(sizeof(user_cmt_t), 1);
	if(user->user_cmt == NULL){
		cl_free(user);
		return NULL;
	}		
	
	user->handle = handle_create(HDLT_USER);

	user->never_establish = true;
	
	STLC_INIT_LIST_HEAD(&user->link);
	STLC_INIT_LIST_HEAD(&user->dev);
	STLC_INIT_LIST_HEAD(&user->slave);
	STLC_INIT_LIST_HEAD(&user->tcp_send_list);
	
	STLC_INIT_LIST_HEAD(&user->user_cmt->hello_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->alarm_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->mesure_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->notify_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->notify_ack_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->dev_add_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->dev_del_head);
	STLC_INIT_LIST_HEAD(&user->user_cmt->dev_query_head);
	user->user_cmt->my_notify_id = 1;
	user->user_cmt->ip_count = 0;
    
	key_learn_alloc(user);
    
	user_bind_phone_alloc(user);

	user->env_mon = cl_calloc(sizeof(cl_env_mon_t), 1);
	user->health = user_health_init();

	return user;
}

void user_free(user_t *user)
{
	slave_t *slave, *slave_next;
	user_t *dev, *dev_next;

	if (user->link.next != NULL)
		stlc_list_del(&user->link);
	
	CLOSE_SOCK(user->sock_udp);
	CLOSE_SOCK(user->sock_dev_server);
	
	CL_THREAD_OFF(user->t_read_udp);
    log_debug("CL_THREAD_READ_OFF %s\n",__FUNCTION__);
	CL_THREAD_OFF(user->t_read);
	CL_THREAD_OFF(user->t_write);
	CL_THREAD_OFF(user->t_timer_comm);
	user_off_all_timer(user);
	CL_THREAD_OFF(user->t_timer_login_timeout);
	CL_THREAD_OFF(user->t_timer_query_env);
	CL_THREAD_OFF(user->t_timer_pu_msg);

	SAFE_FREE(user->vendor_id);
	SAFE_FREE(user->vendor_url);
	SAFE_FREE(user->nickname);
	SAFE_FREE(user->name);
	SAFE_FREE(user->passwd);
	SAFE_FREE(user->new_passwd);
	SAFE_FREE(user->scm_dev_desc);
	SAFE_FREE(user->rf_up_filepath);
	SAFE_FREE(user->stm_spe_up_file);

	cl_em_free_info((cl_env_mon_t *)user->env_mon);

	pkt_free(user->udp_buf);
	pkt_free(user->tcp_buf);
	user_free_send_list(user);

	stlc_list_for_each_entry_safe(user_t, dev, dev_next, &user->dev, link) {
		stlc_list_del(&dev->link);
		user_free(dev);
	}

	stlc_list_for_each_entry_safe(slave_t, slave, slave_next, &user->slave, link) {
		stlc_list_del(&slave->link);
		slave_free(slave);
	}
#ifndef	NO_COMMUNITY
	free_licence(user->dskey);
	user_cmt_free(user);
#endif
	ia_free(user);
	SAFE_FREE(user->np);
	key_learn_free(user);
	alarm_phone_free(user);
	area_ctrl_free(user);
	scene_ctrl_free(user);
	user_bind_phone_free(user);
	if (user->json != NULL)
		json_free(user->json);
	SAFE_FREE(user->apns_cfg);
	SAFE_FREE(user->prev_scene_linkage);
	user_health_clean((user_health_t *)user->health);

	ucc_free(user);

    sa_free(user);
	lbs_user_free(user);
    SAFE_FREE(user->qr_code);

	CL_THREAD_OFF(user->t_display_stat_timer);
    
	cl_free(user);
}

void user_stop_h264_decode(user_t *user)
{
	slave_t *slave;
	user_t *dev;

	stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
		user_stop_h264_decode(dev);
	}
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		stop_h264_decode(slave->video);
	}
}

void user_destroy(user_t *user)
{
	log_info("destroy user 0x%08x\n", user->handle);

	user_stop_h264_decode(user);
	
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	
	cl_lock(&cl_priv->mutex);
	user_free(user);
	cl_unlock(&cl_priv->mutex);
	log_info("destroy user OK\n");
}

static RS user_del(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_user_t *up;
	ucc_session_t *s;


	up = (cln_user_t *)&pkt->data[0];

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "del user 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if(!user->is_phone_user){
		delete_lan_dev_by_sn(user->sn);
	}

	if (up->b_proc && (s = (ucc_session_t *)user->uc_session)) {
		ucc_del_share_code(s);
	}
	
	user_destroy(user);
	
done:
	cl_unlock(&cl_priv->mutex);
	return ret;

}

void user_del_all()
{
	user_t *user, *next;

	// 先停止解码线程
	stlc_list_for_each_entry_safe(user_t, user, next, &cl_priv->user, link) {
		user_stop_h264_decode(user);
	}

	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry_safe(user_t, user, next, &cl_priv->user, link) {
		user_free(user);
	}
	
	cl_unlock(&cl_priv->mutex);
}

static void callback_set_name(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	slave_t *slave;

	if ((slave = (slave_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_set_name, not found slave type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	log_debug("callback_set_name %s, handle=0x%08x, result=%d\n", slave->name, slave->handle, result);

	switch (w->cmd) {
	case CMD_SET_NAME:
		if (result == CMD_OK) {
			event_push(slave->user->callback, UE_SET_NAME_OK, slave->handle, slave->user->callback_handle);
		} else {
			event_push_err(slave->user->callback, UE_SET_NAME_FAILED, slave->handle, slave->user->callback_handle, result);
		}
		break;

	default:
		log_err(false, "callback_set_name, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;		
	}
}

static void callback_slave_result(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	slave_t *slave;

	if ((slave = (slave_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_slave_result, not found slave type=%d handle=0x%08x, cmd=%u, result=%u\n",
			w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	log_debug("callback_slave_result %s, handle=0x%08x, result=%d\n", slave->name, slave->handle, result);

	switch (w->cmd) {
	case CMD_SET_NEWUPVER:
		
		log_debug("agree upgrade ret=%d\n", result);
		if (result == CMD_OK) {
			event_push(slave->user->callback, UE_DEV_UPGRADE_SET_OK, slave->handle, slave->user->callback_handle);
		} else {
			event_push_err(slave->user->callback, UE_DEV_UPGRADE_SET_FAIL, slave->handle, slave->user->callback_handle, result);
		}
		break;
		
	case CMD_NOTICE_DEVUP:
		if (result == CMD_OK) {
			log_debug("upgrade now ret=%d\n", result);
			event_push(slave->user->callback, UE_DEV_UPGRADE_NOW_OK, slave->handle, slave->user->callback_handle);
		} else {
			event_push_err(slave->user->callback, UE_DEV_UPGRADE_NOW_FAIL, slave->handle, slave->user->callback_handle, result);
		}

	default:
		log_err(false, "callback_set_name, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;
	}
}

static RS slave_modify_name(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	slave_t *slave;
	cln_user_t *up;
	char *name;
	pkt_t *pkt;
	net_set_name_t *nsn;
	video_t *video;
	int mod_id = 0;

	up = (cln_user_t *)&cln_pkt->data[0];
	name = up->data;
	
	cl_lock(&cl_priv->mutex);

	slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle);
	if (slave == NULL) {
		mod_t *mod;
		
		video = (video_t *)lookup_by_handle(HDLT_VIDEO, up->user_handle);
		if (video == NULL) {
			log_err(false, "slave_modify_name failed: not found slave handle=0x%08x\n", up->user_handle);
			ret = RS_NOT_FOUND;
			goto done;
		}
		slave = video->slave;
		stlc_list_for_each_entry(mod_t, mod, &slave->mod_list, link) {
			if (MID_USB_VIDEO_MIN <= mod->mod_id && mod->mod_id <= MID_USB_VIDEO_MAX) { /* USB摄像头 */
				mod_id = mod->mod_id;
				break;
			}
		}
	}

	if (slave->user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new_v2(CMD_SET_NAME, sizeof(net_set_name_t) + up->len_name, NHF_TRANSPARENT, slave->user->sn, slave->user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_SET_NAME, NULL, callback_set_name);
	nsn = get_pkt_payload(pkt, net_set_name_t);
	nsn->sn = ntoh_ll(slave->sn);
	nsn->module_id = htons(mod_id);
	nsn->name_len = up->len_name - 1;
	nsn->resv = 0;
	strncpy(nsn->name, up->data, nsn->name_len);
	nsn->name[nsn->name_len] = '\0';

	user_add_pkt(slave->user, pkt);
	log_info("尝试修改%s名字为%s, name len=%u\n", slave->str_sn, nsn->name, nsn->name_len);

done:
	cl_unlock(&cl_priv->mutex);

	// 触发一次快速查询
	if (ret == RS_OK) {
		CL_THREAD_OFF(slave->user->t_timer_query_master);
		CL_THREAD_TIMER_ON(&cl_priv->master, slave->user->t_timer_query_master, timer_query_master, (void *)slave->user, 1);
	}
	
	return ret;
}

static RS slave_open_telnet(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	slave_t *slave;
	user_t *user;
	cln_user_t *up;
	pkt_t *pkt;
	remote_vty_t *nb;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
		 if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) != NULL && user->is_udp_ctrl) {
            ret = sa_user_telnet_device(user, up);
            goto done;
        }
		log_err(false, "open-telnet slave handle = 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (slave->user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new_v2(CMD_REMOTE_VTY, sizeof(remote_vty_t), NHF_TRANSPARENT, slave->sn, slave->user->ds_type);
	nb = get_pkt_payload(pkt, remote_vty_t);

	nb->ip = htonl(up->ip);
	nb->port = htons(up->port);

	user_add_pkt(slave->user, pkt);
	log_info("Try open %s telnet, ip=%u.%u.%u.%u, port=%u\n", slave->name, IP_SHOW(up->ip), up->port);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static RS slave_reboot(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	slave_t *slave;
	cln_user_t *up;
	pkt_t *pkt;
	net_reboot_t *nb;
    user_t * user;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
        if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) != NULL && user->is_udp_ctrl) {
            ret = sa_user_reboot_device(user);
            goto done;
        }
        
		log_err(false, "open-telnet slave handle = 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (slave->user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new_v2(CMD_REBOOT, sizeof(net_reboot_t), NHF_TRANSPARENT, slave->sn, slave->user->ds_type);
	nb = get_pkt_payload(pkt, net_reboot_t);
	nb->action = REBOOT_REQUEST;

	user_add_pkt(slave->user, pkt);
	log_info("Try reboot %s\n", slave->name);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static void do_dev_upgrade_now_a(user_t *user, pkt_t *pkt)
{
	net_header_v2_t *hdr = (net_header_v2_t *)pkt->data;
	slave_t *slave;
	u_int64_t sn;

	if (hdr->hdr.ver == PROTO_VER1)
		sn = user->sn;
	else if (hdr->hdr.ver >= PROTO_VER2)
		sn = hdr->sn?hdr->sn:user->sn;
	else
		return;
			
	slave = slave_lookup_by_sn(user, sn);
	if (slave == NULL)
		return;
	
	event_push(user->callback, UE_DEV_UPGRADE_NOW_OK, slave->handle, user->callback_handle);
}

static void do_dev_newupver_a(user_t *user, pkt_t *pkt)
{
	net_newupver_a_t *a;
	net_header_v2_t *hdr = (net_header_v2_t *)pkt->data;
	u_int32_t vs, vd;
	slave_t *slave;
	ms_dev_version_t *ver;
	u_int8_t *pdesc;

	if (hdr->hdr.ver < PROTO_VER2)
		return;
	if (hdr->hdr.param_len < sizeof(*a)) {
		log_info("device %s, param len %d < %d \n", user->name, hdr->hdr.param_len, sizeof(*a));
		return;
	}
	
	a = get_pkt_payload(pkt, net_newupver_a_t);
	a->errorcode = ntohl(a->errorcode);
	a->desc_len = ntohs(a->desc_len);
	a->url_len = ntohs(a->url_len);
	if (hdr->hdr.param_len < (sizeof(*a) + a->desc_len + a->url_len)) {
		log_info("device %s, param len %d < %d \n", user->name, hdr->hdr.param_len, (sizeof(*a) + a->desc_len + a->url_len));
		return;
	}
	if (a->errorcode) {
		log_info("device %s, errorcode=%u\n", user->name, a->errorcode);
		return;
	}
	
	slave = slave_lookup_by_sn(user, hdr->sn?hdr->sn:user->sn);
	if (slave == NULL)
		return;
	pdesc = (u_int8_t*)(a + 1);
	SAFE_FREE(slave->dev_info.release_desc);
	SAFE_FREE(slave->dev_info.release_url);
	SAFE_FREE(slave->dev_info.release_date);
	if (a->desc_len) {
		slave->dev_info.release_desc = cl_calloc(a->desc_len + 1, 1);
		if (slave->dev_info.release_desc) {
			memcpy(slave->dev_info.release_desc, pdesc, a->desc_len);
		}
		pdesc = pdesc+ a->desc_len;
	}
	
	if (a->url_len) {
		slave->dev_info.release_url = cl_calloc(a->url_len + 1, 1);
		if (slave->dev_info.release_url) {
			memcpy(slave->dev_info.release_url, pdesc, a->url_len);
		}
	}
	a->date[UP_DATE_LEN - 1] = 0;
	slave->dev_info.release_date = cl_strdup(a->date);

	vs = BUILD_U32(a->fmajor, a->fminor, a->frevise, 0);
	ver = &slave->dev_info.version;
	vd = BUILD_U32(ver->soft_version.major, ver->soft_version.minor, ver->soft_version.revise, 0);
	if (vd < vs) {
		//need upgrade firmware
		slave->dev_info.new_version.major = a->umajor;
		slave->dev_info.new_version.minor = a->uminor;
		slave->dev_info.new_version.revise = a->urevise;
		slave->dev_info.new_version.pad = 0;
		slave->dev_info.can_update = 1;
		slave->dev_info.can_auto_update = 0;
		event_push(user->callback, UE_DEV_UPGRADE_READY, slave->handle, user->callback_handle);
		return;
	}

	vs = BUILD_U32(a->umajor, a->uminor, a->urevise, 0);
	vd = BUILD_U32(ver->upgrade_version.major, ver->upgrade_version.minor, ver->upgrade_version.revise, 0);
	if (vd < vs ) {
		//need upgrade software
		slave->dev_info.new_version.major = a->umajor;
		slave->dev_info.new_version.minor = a->uminor;
		slave->dev_info.new_version.revise = a->urevise;
		slave->dev_info.new_version.pad = 0;
		slave->dev_info.can_update = 1;
		slave->dev_info.can_auto_update = 1;
		event_push(user->callback, UE_DEV_UPGRADE_READY, slave->handle, user->callback_handle);
		return;
	}
	slave->dev_info.can_update = 0;
	slave->dev_info.can_auto_update = 0;
}

static RS __dev_upgrade_q(user_t *dev, u_int8_t lang)
{
	pkt_t *pkt;
	net_newupver_q_t *q;
	slave_t *slave;
	ms_dev_version_t *ver;
			
	if (dev->status != CS_ESTABLISH) {
		return RS_NOT_LOGIN;
	}

	stlc_list_for_each_entry(slave_t, slave, &dev->slave, link) {
		ver = &slave->dev_info.version;
		//忽略不在线的或扩展类型无效的
		if (slave->status != BMS_BIND_ONLINE )// || ver->soft_version.pad == 0)
			continue;
		if (slave->sn == dev->sn) {
			if (dev->vendor_id == NULL || dev->ext_type == 0xFF)
				continue;
		} else {
			if (slave->vendor_id == NULL || slave->ext_type == 0xFF)
				continue;
		}
		
		pkt = pkt_new_v2(CMD_NEWUPVER_Q, sizeof(*q), 0, slave->sn, dev->ds_type);
		if (pkt == NULL) {
			return RS_MEMORY_MALLOC_FAIL;
		}
		q = get_pkt_payload(pkt, net_newupver_q_t);
		q->type = TP_DS007;
		if (slave->sn == dev->sn) {
			q->sub_type = dev->sub_type;
			q->ext_type = dev->ext_type;
			strncpy(q->oem_id, dev->vendor_id, sizeof(q->oem_id));
		} else {
			q->sub_type = slave->sub_type;
			q->ext_type = slave->ext_type;
			strncpy(q->oem_id, slave->vendor_id, sizeof(q->oem_id));
		}		
		q->lang = lang;

		log_info("%s will send cmd[%d] \n", __FUNCTION__,CMD_NEWUPVER_Q);
		
		user_add_pkt(dev, pkt);	
	}
	
	return RS_OK;	
}

static void dev_upgrade_parse(user_t *user, misc_upgrade_query_reply_t *pr, u_int16_t len)
{
	bool ret = false;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	cl_dev_stat_t *st;
	u_int32_t vnew, vold;
	char *tmp;
	char *url;
	u_int16_t dst_len = 0;

	tmp = (char *)pr;
	pr->sn = ntoh_ll(pr->sn);
	pr->url_ver_len = ntohl(pr->url_ver_len);
	pr->url_stm_ver_len = ntohl(pr->url_stm_ver_len);
	pr->url_rfstm_ver_len = ntohl(pr->url_rfstm_ver_len);

	dst_len = (u_int16_t)(pr->url_ver_len + pr->url_stm_ver_len + pr->url_rfstm_ver_len + sizeof(*pr));
	tmp = (char *)pr;
	log_err(false, "len=%u url_ver_len=%u url_stm_ver_len=%u url_rfstm_ver_len=%u dst_len=%u\n", 
		len, pr->url_ver_len, pr->url_stm_ver_len, pr->url_rfstm_ver_len, dst_len);
	//len check
	if (len < dst_len) {
		return;
	}
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		return;
    }
	air_ctrl = sac->sub_ctrl;
	st = &air_ctrl->stat;
	st->can_update = false;
	st->stm_can_update = false;

	st->new_version.major = pr->ver.major;
	st->new_version.minor = pr->ver.minor;
	st->new_version.revise = pr->ver.revise;

	st->stm_newest_version.major = pr->stm_ver.major;
	st->stm_newest_version.minor = pr->stm_ver.minor;
	st->stm_newest_version.revise = pr->stm_ver.revise;

    SAFE_FREE(st->stm_release_url);
    SAFE_FREE(st->release_desc);
    SAFE_FREE(st->release_url);
    SAFE_FREE(st->release_date);

	if (pr->url_ver_len) {
		url = cl_calloc((pr->url_ver_len + 10), 1);
		if (!url) {
			return;
		}
		memcpy(url, &tmp[sizeof(*pr)], pr->url_ver_len);
		st->release_url = url;
        vnew = BUILD_U32(st->new_version.major, st->new_version.minor, st->new_version.revise, 0);
        vold = BUILD_U32(st->upgrade_version.major, st->upgrade_version.minor, st->upgrade_version.revise, 0);
        log_err(false ,"release_url=%s vold=%08x vnew=%08x\n", st->release_url, vold, vnew);
        if((vold != 0) && (vnew > vold)){
            st->can_update = true;
        }
	}

	pr->desc[sizeof(pr->desc)-1] = 0;
	st->release_desc = cl_strdup(pr->desc);
	
	if (pr->url_stm_ver_len) {
		url = cl_calloc((pr->url_stm_ver_len + 10), 1);
		if (!url) {
			return;
		}
		memcpy(url, &tmp[sizeof(*pr) + pr->url_ver_len], pr->url_stm_ver_len);
		st->stm_release_url = url;
        vnew = BUILD_U32(st->stm_newest_version.major, st->stm_newest_version.minor, st->stm_newest_version.revise, 0);
        vold = BUILD_U32(st->stm_cur_version.major, st->stm_cur_version.minor, st->stm_cur_version.revise, 0);
		log_err(false ,"stm_release_url=%s vold=%08x vnew=%08x\n", st->stm_release_url, vold, vnew);
        if(vnew > vold){
            st->stm_can_update = true;
        }
	}
	
	if(st->can_update || st->stm_can_update){
		event_push(user->callback, UE_DEV_UPGRADE_READY, user->handle, user->callback_handle);
	}
}

static void dev_upgrade_cb(cl_handle_t user_handle, u_int16_t type, u_int16_t len, u_int8_t *data)
{
	user_t *user;
	misc_upgrade_query_reply_t *pr = (misc_upgrade_query_reply_t *)data;
	
	if (MCT_UPGRADE != type) {
		log_err(false, "err type=%u\n", type);
		return;
	}
	if (pr->result != 0) {
		log_err(false, "err result=%u\n", pr->result);
		return;
	}
	if (len < sizeof(*pr)) {
		log_err(false, "err len=%u\n", len);
		return;
	}

	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, user_handle);
		goto done;
	}

	dev_upgrade_parse(user, pr, len);
	
done:
	cl_unlock(&cl_priv->mutex);
}


static RS do_dev_upgrade_build(user_t *user, u_int8_t lang)
{
	misc_upgrade_query_t query; 
	u_int32_t ip = user_select_ip(user);

	if (ip == 0) {
		log_err(false, "err ip=0\n");
		return RS_ERROR;
	}
	memset((void *)&query, 0, sizeof(query));
	query.type = TP_DS007;
	query.sub_type = user->sub_type;
	query.ext_type = user->ext_type;
	query.lang = lang;
	if (user->vendor_id) {
		strcpy((char *)query.vendor, user->vendor_id);
	}
	memcpy(query.developer_id, user->developer_id, sizeof(query.developer_id));
	query.mastersn = 0;
	query.upstm = 1;
	query.sn = ntoh_ll(user->sn);
	
	misc_client_do_request(user->handle, MCT_UPGRADE, sizeof(query), (u_int8_t *)&query, ip, dev_upgrade_cb);

	return RS_OK;
}


/*
http_dev_upgrade_q
通过cgi查询设备新版本
*/
static RS http_dev_upgrade_q(user_t *user, int lang)
{
	char url[1024];
    u_int8_t sub_type = user->ext_type;
	
    if (user->real_sub_type !=0) {
        sub_type = user->real_sub_type;
    }
	sprintf(url, "http://%s/cgi-bin/updev?sn=%012"PRIu64"&type=%hhu&subtype=%hhu&exttype=%hhu&vendor=%s&lang=%d&upstm=1",
		DEFAULT_DOMAIN, user->sn, TP_DS007, user->sub_type, user->real_ext_type, user->vendor_id, lang);

	//log_debug("http_dev_upgrade_q url=%s\n", url);
	http_get(url, CLNE_HTTP_DEV_VERSION, user->handle);
	return RS_OK;
}

static void parse_upgrade_data(user_t *user, jvalue_t *node)
{
	jvalue_t *jv;
	bool ret = false;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	cl_dev_stat_t *st;
	u_int32_t vnew, vold;
    bool dev_data_valid = false,stm_data_valid = false;
		
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		return;
    }
    
	air_ctrl = sac->sub_ctrl;
	st = &air_ctrl->stat;
	st->can_update = false;
	st->stm_can_update = false;

	if ((jv = jvalue_lookup_by_str(node, "ver")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "ver type=%u\n", jv->type);
		} else {
			if(sscanf(jv->value.str, "%hhu.%hhu.%hhu", &st->new_version.major, 
				&st->new_version.minor, &st->new_version.revise)==3){
                dev_data_valid = true;
			}			

		}
	}
	
	if ((jv = jvalue_lookup_by_str(node, "stm_ver")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "stm ver type=%u\n", jv->type);
		} else {
			if(sscanf(jv->value.str, "%hhu.%hhu.%hhu", &st->stm_newest_version.major, 
				&st->stm_newest_version.minor, &st->stm_newest_version.revise)==3){
                stm_data_valid = true;
			}			

		}
	}
    
    SAFE_FREE(st->stm_release_url);
    SAFE_FREE(st->release_desc);
    SAFE_FREE(st->release_url);
    SAFE_FREE(st->release_date);
    
    if ((jv = jvalue_lookup_by_str(node, "download_stmurl")) != NULL) {
        if (jv->type == JT_S) {
            st->stm_release_url = jv->value.str?cl_strdup(jv->value.str):NULL;
        } 
    }

	
	if ((jv = jvalue_lookup_by_str(node, "desc")) != NULL) {
		if (jv->type == JT_S) {
            st->release_desc = jv->value.str?cl_strdup(jv->value.str):NULL;
		} 
	}

	if ((jv = jvalue_lookup_by_str(node, "release_time")) != NULL) {
		if (jv->type == JT_S) {
            st->release_date = jv->value.str?cl_strdup(jv->value.str):NULL;
		} 
	}

	if ((jv = jvalue_lookup_by_str(node, "download_url")) != NULL) {
		if (jv->type == JT_S) {
            st->release_url = jv->value.str?cl_strdup(jv->value.str):NULL;
		} 
	}
    
    if (dev_data_valid && st->release_url && strlen(st->release_url) > 0) {
        
        vnew = BUILD_U32(st->new_version.major, st->new_version.minor, st->new_version.revise, 0);
        vold = BUILD_U32(st->upgrade_version.major, st->upgrade_version.minor, st->upgrade_version.revise, 0);
        
        if((vold != 0) && (vnew > vold)){
            st->can_update = true;
        }
    }
	
    
    if (stm_data_valid && st->stm_release_url && strlen(st->stm_release_url) > 0) {
       
        vnew = BUILD_U32(st->stm_newest_version.major, st->stm_newest_version.minor, st->stm_newest_version.revise, 0);
        vold = BUILD_U32(st->stm_cur_version.major, st->stm_cur_version.minor, st->stm_cur_version.revise, 0);
       // if((vold != 0) && (vnew > vold)){
        if(vnew > vold){
            st->stm_can_update = true;
        }
    }

	if(st->can_update || st->stm_can_update){
		event_push(user->callback, UE_DEV_UPGRADE_READY, user->handle, user->callback_handle);
	}

}

static void rf_parse_upgrade_data(user_t *user, jvalue_t *node, u_int8_t index)
{
	jvalue_t *jv;
	bool ret = false;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	u_int32_t vnew, vold;
	rfgw_priv_t *p;
	bool upgrade_ready = false;
	u_int32_t v1,v2,v3;
	static cl_version_t newest_version;

	index %= D_T_MAX;
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		return;
    }
	air_ctrl = sac->sub_ctrl;

	p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	p->upgrade_query[index] = 0;
	p->server_queryed[index] = 1;

	memset((void *)&newest_version, 0, sizeof(newest_version));
	//判断是否app通讯镜像需要升级
	if ((jv = jvalue_lookup_by_str(node, "stm_ver")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "stm ver type=%u\n", jv->type);
		} else {
			if(sscanf(jv->value.str, "%u.%u.%u", &v1, 
				&v2, &v3) != 3){
				p->upgrade_query[index] = 0;
				return ;
			}			
			newest_version.major = (u_int8_t)v1;
			newest_version.minor = (u_int8_t)v2;
			newest_version.revise = (u_int8_t)v3;
		}
	}
    SAFE_FREE(p->upgrade_url[index]);
    if ((jv = jvalue_lookup_by_str(node, "download_stmurl")) != NULL) {
        if (jv->type == JT_S) {
            p->upgrade_url[index] = cl_strdup(jv->value.str);
			p->server_ver[index] = 
				BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
        } 
    }
	
	if(p->upgrade_url[index] == NULL) {
		goto rf_up ;
	}

	vnew = BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
	vold = p->min_ver[index];
	log_debug("ver index=%u vold=%08x vnew-=%08x upgradestatus=%u\n", 
		index, vold, vnew, p->upgrade_status[index]);
	if((vnew > vold)){
		p->upgrade_query[index] = UP_QUERYED;
		p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
		upgrade_ready = true;
		log_debug("app need upgrade index=%u url=%s\n", index, p->upgrade_url[index]);
		goto done;
	} else {
		p->upgrade_status[index] = 0;
	}

rf_up:	
	//判断是否rf需要升级
	memset((void *)&newest_version, 0, sizeof(newest_version));
	if ((jv = jvalue_lookup_by_str(node, "ver")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "ver type=%u\n", jv->type);
		} else {
			if(sscanf(jv->value.str, "%u.%u.%u", &v1, 
				&v2, &v3) != 3){
				return ;
			}			
			newest_version.major = (u_int8_t)v1;
			newest_version.minor = (u_int8_t)v2;
			newest_version.revise = (u_int8_t)v3;
		}
	}
    SAFE_FREE(p->upgrade_url[index]);
    if ((jv = jvalue_lookup_by_str(node, "download_url")) != NULL) {
        if (jv->type == JT_S) {
            p->upgrade_url[index] = cl_strdup(jv->value.str);
			p->server_ver_rf[index] = 
				BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
        } 
    }
	
	if(p->upgrade_url[index] == NULL) {
		goto rf_stm_up ;
	}

	vnew = BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
	vold = p->min_ver_rf[index];
	log_debug("rf index=%u vold=%08x vnew-=%08x upgradestatus=%u\n", 
		index, vold, vnew, p->upgrade_status[index]);
	if((vnew > vold)){
		p->upgrade_query[index] = UP_QUERYED;
		p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
		upgrade_ready = true;
		log_debug("rf need upgrade index=%u url=%s\n", index, p->upgrade_url[index]);
		goto done;
	} else {
		p->upgrade_status[index] = 0;
	}

rf_stm_up:
	//判断是否rf下单片机需要升级
	memset((void *)&newest_version, 0, sizeof(newest_version));
	if ((jv = jvalue_lookup_by_str(node, "rfstm_ver")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "ver type=%u\n", jv->type);
		} else {
			if(sscanf(jv->value.str, "%u.%u.%u", &v1, 
				&v2, &v3) != 3){
				return ;
			}			
			newest_version.major = (u_int8_t)v1;
			newest_version.minor = (u_int8_t)v2;
			newest_version.revise = (u_int8_t)v3;
		}
	}
    SAFE_FREE(p->upgrade_url[index]);
    if ((jv = jvalue_lookup_by_str(node, "download_rfstmurl")) != NULL) {
        if (jv->type == JT_S) {
            p->upgrade_url[index] = cl_strdup(jv->value.str);
			p->server_ver_rf_stm[index] = 
				BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
        } 
    }
	
	if(p->upgrade_url[index] == NULL) {
		goto done ;
	}

	vnew = BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
	vold = p->min_ver_rf_stm[index];
	log_debug("rf index=%u vold=%08x vnew-=%08x upgradestatus=%u\n", 
		index, vold, vnew, p->upgrade_status[index]);
	if(vnew > vold){
		p->upgrade_query[index] = UP_QUERYED;
		p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
		upgrade_ready = true;
		log_debug("rf need upgrade index=%u url=%s\n", index, p->upgrade_url[index]);
	} else {
		p->upgrade_status[index] = 0;
	}

done:	
	if (upgrade_ready) {
		event_push(user->callback, UE_DEV_UPGRADE_READY, user->handle, user->callback_handle);
	}
	_slave_upgrade_check(user, air_ctrl);
}

/*
pu_on_dev_upgrade_notify
通过cgi查询设备新版本结果处理
cgi应答：
{"result":0, "errordesc":"OK", "data":{"ver":"2.5.1","fver":"2.5.1","desc":"Fix bug. Add new functions","release_time":"2014-09-03","download_url":"http://www.jiazhang007.com/download/IPLUS808_upgrade_V2.5.1.bin"}}
{"result":1, "errordesc":"(失败原因)"}

*/
RS pu_on_dev_upgrade_notify(cl_notify_pkt_t *pkt)
{
#if 1
	user_t *user;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv, *node;
	int result = -1;
	int event = UE_APNS_CONFIG_FAIL;
	RS ret = RS_OK;

	h = (cln_http_t *)pkt->data;

	log_debug("pu_on_dev_upgrade_notify: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
				if ((node = jvalue_lookup_by_str(json->m_data, "data")) != NULL && node->type == JT_O) {
					parse_upgrade_data(user, node);
				}				
			} else {
				log_err(false, "register phone user failed: result=%s\n", jv->value.str);
				if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
					&& jv->type == JT_S) {
					log_err(false, "%s result=%d, errordesc=%s\n", __FUNCTION__,  result, jv->value.str);
				}
			}
		}
		json_free(json);
	}	

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return ret;
	#endif
	return 0;
}

static void rf_http_next(user_t *user, u_int8_t index)
{
	bool ret = false;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	rfgw_priv_t *p;
	
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		return;
    }
	air_ctrl = sac->sub_ctrl;

	p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;

	p->upgrade_query[index] = 0;
	p->server_queryed[index] = 1;
	_slave_upgrade_check(user, air_ctrl);
}

RS pu_on_rfdev_upgrade_notify(cl_notify_pkt_t *pkt)
{
#if 1
	user_t *user;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv, *node;
	int result = -1;
	int event = UE_APNS_CONFIG_FAIL;
	RS ret = RS_OK;

	h = (cln_http_t *)pkt->data;

	log_debug(" ^^^^^^^^^^^^^^^^^^^^^^ pu_on_rfdev_upgrade_notify: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		log_debug("not found\n");
		ret = RS_NOT_FOUND;
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
				if ((node = jvalue_lookup_by_str(json->m_data, "data")) != NULL && node->type == JT_O) {
					rf_parse_upgrade_data(user, node, h->index);
				}				
			} else {
				log_err(false, "register phone user failed: result=%s\n", jv->value.str);
				if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
					&& jv->type == JT_S) {
					log_err(false, "%s result=%d, errordesc=%s\n", __FUNCTION__,  result, jv->value.str);
				}

				rf_http_next(user, h->index);
			}
		} else {
			rf_http_next(user, h->index);
		}
		json_free(json);
	}
	
done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return ret;
	#endif
	return 0;
}

static void rfdev_up_check_parse_upgrade_data(user_t *user, jvalue_t *node)
{
	jvalue_t *jv;
	jvalue_t *node_dev;
	int i,count;
	bool ret = false;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	u_int32_t vnew, vold;
	rfgw_priv_t *p = NULL;
	u_int8_t ext_type = 0;
	u_int8_t sub_type = 0;
	u_int8_t index = 0;
	char dev_name[10];
	u_int32_t v1,v2,v3;
	cl_version_t newest_version;
	char *url = NULL;
	bool upgrade_ready = false;
		
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		return;
    }
    
	air_ctrl = sac->sub_ctrl;
	p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;

	//get count
	count = 0;
	if ((jv = jvalue_lookup_by_str(node, "count")) == NULL) {
		log_err(false, "there is no count\n");
		return;
	}
	if (jv->type != JT_S) {
		log_err(false, "count type error node up check type=%u\n", jv->type);
		return;
	} else {
		count = atoi(jv->value.str);
		if ((count <= 0) || (count > D_T_MAX)) {
			log_err(false, "count=%d not need upgrade\n", count);
			return;
		}
	}

	for(i = 0; i < count; i++) {
		sprintf(dev_name, "dev%d", i);
		//get devx node
		node_dev = jvalue_lookup_by_str(node, dev_name);
		if (!node_dev) {
			log_err(false, "not found dev_name=%s\n", dev_name);
			continue;
		}
		//sub_type check
		jv = jvalue_lookup_by_str(node_dev, "sub_type");
		if (!jv || (jv->type != JT_S)) {
			log_err(false, "err subtype\n");
			continue;
		}
		sub_type = (u_int8_t)atoi(jv->value.str);
		if (sub_type != IJ_RFGW) {
			log_err(false, "err sub_type=%u\n", sub_type);
			continue;
		}
		//ext_type check
		jv = jvalue_lookup_by_str(node_dev, "ext_type");
		if (!jv || (jv->type != JT_S)) {
			log_err(false, "err ext_type\n");
			continue;
		}
		ext_type = (u_int8_t)atoi(jv->value.str);
		index = _get_rf_dev_index_by_ext(ext_type);
		//get version
		memset((void *)&newest_version, 0, sizeof(newest_version));
		jv = jvalue_lookup_by_str(node_dev, "ver");
		
		if (!jv || (jv->type != JT_S)) {
			log_err(false, "err ver\n");
			continue;
		}
		if(sscanf(jv->value.str, "%u.%u.%u", &v1, 
			&v2, &v3) != 3){
			log_err(false, "err ver str=%s\n", jv->value.str);
			return ;
		}
		newest_version.major = v1;
		newest_version.minor = v2;
		newest_version.revise = v3;
		//get url
		jv = jvalue_lookup_by_str(node_dev, "fup_url");
		if (!jv || (jv->type != JT_S)) {
			log_err(false, "err url\n");
			continue;
		}
		
		vnew = BUILD_U32(newest_version.major, newest_version.minor, newest_version.revise, 0);
		vold = p->min_ver[index];
		if (vnew <= vold) {
			continue;
		}
		p->upgrade_query[index] = UP_QUERYED;
		p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
		SAFE_FREE(p->upgrade_url[index]);
		p->upgrade_url[index] = cl_strdup(jv->value.str);
		upgrade_ready = true;
	}	

	if (upgrade_ready) {
		event_push(user->callback, UE_DEV_UPGRADE_READY, user->handle, user->callback_handle);
	}
}


/*
*{
    "ver":"1.0",
    "mastersn":808000010001,
    "count":2,
    "dev0":{
        "sub_type":30,
        "ext_type":47,
        "image":"xx.bin",
        "ver":"1.5.0",
        "fup_url":"http:www.dl.jiazhang007.com/xx.bin"
    },
    "dev1":{
        "sub_type":30,
        "ext_type":49,
        "image":"xx1.bin",
        "ver":"1.3.0",
        "fup_url":"http:www.dl.jiazhang007.com/xx1.bin"
    }
}
*/
RS pu_on_rfdev_upgrade_check_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	int result = -1;
	RS ret = RS_OK;

	h = (cln_http_t *)pkt->data;
	log_debug("pu_on_rfdev_upgrade_check_notify: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
				rfdev_up_check_parse_upgrade_data(user, json->m_data);
			} else {
				log_err(false, "register phone user failed: result=%s\n", jv->value.str);
				if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
					&& jv->type == JT_S) {
					log_err(false, "%s result=%d, errordesc=%s\n", __FUNCTION__,  result, jv->value.str);
				}
			}
		}
		json_free(json);
	}	

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return ret;
}


static RS dev_upgrade_q(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	user_t *dev;
	u_int8_t lang;

	up = (cln_user_t *)&cln_pkt->data[0];
	lang = (u_int8_t)up->ip;
	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (user->is_udp_ctrl) {
#ifdef MISC_CLIENT_USER	
		ret = do_dev_upgrade_build(user, lang);
#else
		ret = http_dev_upgrade_q(user, lang);
#endif
		goto done;
	}

	if (user->is_phone_user) {
		stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
			__dev_upgrade_q(dev, lang);
		}
	} else {
		__dev_upgrade_q(user, lang);
	}
	
done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static RS dev_upgrade_set(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	slave_t *slave;
	pkt_t *pkt;
	net_set_newupver_t *q;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	user = slave->user;

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}
	if (slave->status != BMS_BIND_ONLINE ) {
		ret = RS_OFFLINE;
		goto done;
	}

	pkt = pkt_new_v2(CMD_SET_NEWUPVER, sizeof(*q), 0, slave->sn, user->ds_type);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_SET_NEWUPVER, NULL, callback_slave_result);
	q = get_pkt_payload(pkt, net_set_newupver_t);
	q->type = TP_DS007;
	if (user->sn == slave->sn ) {
		q->sub_type = user->sub_type;
		q->ext_type = user->ext_type;
		strncpy(q->oem_id, user->vendor_id, sizeof(q->oem_id));
	} else {
		q->sub_type = slave->sub_type;
		q->ext_type = slave->ext_type;
		strncpy(q->oem_id, slave->vendor_id, sizeof(q->oem_id));
	}

	log_info("%s handle = 0x%08x will send cmd[%d] \n", __FUNCTION__, up->user_handle, CMD_SET_NEWUPVER);

	user_add_pkt(user, pkt);
	
done:
	cl_unlock(&cl_priv->mutex);	
	return ret;
}

static RS dev_upgrade_now(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	slave_t *slave;
	pkt_t *pkt;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	user = slave->user;

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}
	if (slave->status != BMS_BIND_ONLINE ) {
		ret = RS_OFFLINE;
		goto done;
	}

	pkt = pkt_new_v2(CMD_NOTICE_DEVUP, 0, NHF_TRANSPARENT|NHF_WAIT_REPLY, slave->sn, user->ds_type);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_NOTICE_DEVUP, NULL, callback_slave_result);

	log_info("%s handle = 0x%08x will send cmd[%d] \n", __FUNCTION__, up->user_handle, CMD_NOTICE_DEVUP);
	user_add_pkt(user, pkt);
	
done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}


static RS slave_bind(cl_notify_pkt_t *cln_pkt)	
{
	RS ret = RS_OK;
	slave_t *slave;
	cln_user_t *up;
	pkt_t *pkt;
	net_bind_t *nb;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
		log_err(false, "bind slave handle = 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (slave->user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new_v2(CMD_BIND, sizeof(net_bind_t), NHF_TRANSPARENT, slave->user->sn, slave->user->ds_type);
	nb = get_pkt_payload(pkt, net_bind_t);

	nb->sn = ntoh_ll(slave->sn);
	hash_passwd(nb->passwd, (char *)up->data);


	user_add_pkt(slave->user, pkt);
	log_info("%012"PRIu64" 尝试绑定从设备sn=%s，密码为%s\n", slave->user->sn, slave->str_sn, (char *)up->data);

done:
	cl_unlock(&cl_priv->mutex);

	// 触发一次快速查询
	if (ret == RS_OK) {
		CL_THREAD_OFF(slave->user->t_timer_query_master);
		CL_THREAD_TIMER_ON(&cl_priv->master, slave->user->t_timer_query_master, timer_query_master, (void *)slave->user, 1);
		slave->user->quick_query_master = 5;
		/*
			这个不能太快，互联网上一个查询可能需要1秒多，即使停止了，
			也会可能收到上次的报文，就会导致check_quick_query()不好使
		*/
		slave->user->quick_query_time = 3;
	}
	
	return ret;
}

static RS slave_unbind(cl_notify_pkt_t *cln_pkt)	
{
	RS ret = RS_OK;
	slave_t *slave;
	cln_user_t *up;
	pkt_t *pkt;
	net_bind_t *nb;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
		log_err(false, "bind slave handle = 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (slave->user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new_v2(CMD_BIND, sizeof(net_bind_t), NHF_TRANSPARENT, slave->user->sn, slave->user->ds_type);
	nb = get_pkt_payload(pkt, net_bind_t);

	nb->sn = ntoh_ll(slave->sn);
	// 通过把绑定密码置成全0，来表示解绑定


	user_add_pkt(slave->user, pkt);
	log_info("%012"PRIu64" 尝试解绑定从设备sn=%s\n", slave->user->sn, slave->str_sn);

done:
	cl_unlock(&cl_priv->mutex);

	// 触发一次快速查询
	if (ret == RS_OK) {
		CL_THREAD_OFF(slave->user->t_timer_query_master);
		CL_THREAD_TIMER_ON(&cl_priv->master, slave->user->t_timer_query_master, timer_query_master, (void *)slave->user, 1);
		slave->user->quick_query_master = 10;
		slave->user->quick_query_time = 1;
	}
	
	return ret;
}

void callback_user_requst(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	user_t *user;

	if ((user = (user_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "callback_user_requst, not found user type=%d handle=0x%08x\n", w->obj_type, w->obj_handle);
		return;
	}

	switch (w->cmd) {
	case CMD_PASSWD:
		if (result == CMD_OK) {
			memcpy(user->passwd_md5, user->new_passwd_md5, sizeof(user->passwd_md5));
			user->passwd = user->new_passwd;
			user->new_passwd = NULL;
			fmt_hex(user->passwd_md5_str, user->passwd_md5, 16);
			user->modify_passwd_flags &= ~MPF_DEVICE;
			if (user->modify_passwd_flags == 0) {
				event_push(user->callback, UE_MODIFY_PASSWD_OK, user->handle, user->callback_handle);
			}
			//把修改密码命令交给服务器来做，这样好同步。放在密码修改成功的地方去
			do_passwd_sync_to_server(user);
		} else {
			SAFE_FREE(user->new_passwd);
			event_push_err(user->callback, UE_MODIFY_PASSWD_FAIL, user->handle, user->callback_handle, result);
		}
		break;
	case CMD_NICKNAME:
		event_push_err(user->callback, (result == CMD_OK ? UE_MODIFY_NICKNAME_OK : UE_MODIFY_NICKNAME_FAIL),
				user->handle, user->callback_handle, result);
		break;
	case CMD_NICKNAME_Q:
		log_err(false, "%s get nickname failed\n", user->name);
		break;
	case CMD_PHONE_BIND_Q:
		if (user->bp->is_binding)
			user->bp->is_binding = 0;
		if (result == CMD_OK) {
			event_push(user->callback, UE_BIND_PHONE_REQUEST_OK, user->handle, user->callback_handle);
		} else {
			log_err(false, "%s bind phone request failed\n", user->name);
		}
		break;
	default:
		log_err(false, "callback_user_requst, unknow cmd=%d. result=%d\n", w->cmd, result);
		break;
	}
}

static RS user_modify_passwd(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_user_t *up;
	char *pwd;
	pkt_t *pkt;
	net_pkg_pwd_t *npp;
	la_home_t *phome = NULL;
	la_member_t *pmem = NULL;

	up = (cln_user_t *)&cln_pkt->data[0];
//	if (up->len_passwd >= sizeof(npp->passwd)) {
//		return RS_INVALID_PARAM;
//	}
	pwd = up->data;
	pwd[up->len_passwd - 1] = '\0';
	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "modify user pwd 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	//这里加些判断，如果是联动圈子设备，如果没添加到服务器则可以修改密码，如果已经添加到服务器了，在服务器不通的情况下不允许修改密码
	if (user->home_id != 0) {
		phome = ucla_find_home_by_id(user->home_id);
		if (phome) {
			pmem = ucla_find_member_by_sn_from_home(phome, user->sn);
			if (pmem && (pmem->conf.flag == 0) && (!ucla_get_any_enstablis_session())) {
				ret = RS_ERROR;
				goto done;		
			}
		}
	}

	STR_REPLACE(user->new_passwd, pwd);
	hash_passwd(user->new_passwd_md5, pwd);

	if (user->is_phone_user) {
		user->modify_passwd_flags = MPF_HTTP_DEV;
		pu_modify_user_pwd(user, pwd);
		goto done;
	} else if (user->parent != NULL && user->parent->is_phone_user) {
		user->modify_passwd_flags = MPF_HTTP_DEV | MPF_DEVICE | MPF_HTTP_DICT;
		pu_modify_dev_pwd(user, pwd);
		// don't goto, continue to modify deivce password
	} else {
		user->modify_passwd_flags = MPF_DEVICE;
	}

#if 0
	//把修改密码命令交给服务器来做，这样好同步。放在密码修改成功的地方去
	do_passwd_sync_to_server(user);
#endif

	if(user->is_udp_ctrl){
		ret = sa_user_modify_passwd(user,user->new_passwd_md5);
		goto done;
	}

	pkt = pkt_new(CMD_PASSWD, sizeof(net_pkg_pwd_t), user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, user->handle, CMD_PASSWD, NULL, callback_user_requst);
	npp = get_pkt_payload(pkt, net_pkg_pwd_t);
	npp->sn = ntoh_ll(user->sn);
	npp->time = htonl(get_sec());
	hash_passwd(npp->passwd, pwd);

	user_add_pkt(user, pkt);
	log_info("尝试修改%012"PRIu64"密码, pwd len=%u\n", user->sn, up->len_passwd - 1);
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

// 第一次登录成功，获取当前昵称
void user_get_nickname(user_t *user)
{
	pkt_t *pkt;
	u_int64_t *sn;

	pkt = pkt_new(CMD_NICKNAME_Q, sizeof(u_int64_t), user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, user->handle, CMD_NICKNAME_Q, NULL, callback_user_requst);
	sn = get_pkt_payload(pkt, u_int64_t);
	*sn = ntoh_ll(user->sn);

	user_add_pkt(user, pkt);
}

static void do_nickname_a(user_t *user, pkt_t *pkt)
{
	u_int8_t c;
	net_para_nickname_t *nick_email;

	nick_email = get_pkt_payload(pkt, net_para_nickname_t);

	c = nick_email->data[nick_email->nickname_len];
	nick_email->data[nick_email->nickname_len] = '\0';
	if (user->nickname == NULL || strcmp(user->nickname, nick_email->data) != 0) {
		STR_REPLACE(user->nickname, nick_email->data);
		event_push(user->callback, UE_INFO_MODIFY, 
			(user->parent != NULL ?  user->parent->handle : user->handle), 
			user->callback_handle);
	}

	log_debug("do_nickname_a, sn=%012"PRIu64", nickname=%s\n", user->sn, nick_email->data);
}

static RS user_modify_nickname(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_user_t *up;
	char *nickname;
	pkt_t *pkt;
	net_para_nickname_t *npn;

	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "modify user nickname 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}
	
	if (user->is_udp_ctrl) {
		if (up->len_name > MAX_HOSTNAME) {
			ret =  RS_INVALID_PARAM;
			goto done;
		}	
	} else {
		if (up->len_name > MAX_NICKNAME) {
			ret = RS_INVALID_PARAM;
			goto done;
		}
	}
	nickname = up->data;
	up->len_name -=  1;
	nickname[up->len_name] = '\0';
	
    if (user->is_udp_ctrl) {
        ret = sa_user_modify_nick_name(user,nickname,up->len_name);
        goto done;
    }

	pkt = pkt_new(CMD_NICKNAME, sizeof(net_para_nickname_t) + up->len_name, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, user->handle, CMD_NICKNAME, NULL, callback_user_requst);
	npn = get_pkt_payload(pkt, net_para_nickname_t);
	npn->sn = ntoh_ll(user->sn);
	npn->nickname_len = (u_int8_t)up->len_name;
	npn->email_len = 0;
	memcpy(npn->data, nickname, up->len_name);

	user_add_pkt(user, pkt);
	log_info("尝试修改%012"PRIu64"昵称, nickname len=%u， %s\n", user->sn, up->len_passwd, nickname);

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

static RS create_cmt_service(user_t *user)
{
	int len;
	struct sockaddr_in addr;
	
	if (IS_INVALID_SOCK(user->sock_udp)) {
		user->sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;		
		addr.sin_port = ntohs(CMT_PORT);
		if (bind(user->sock_udp, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0) {
			log_err(true, "bind notify socket port=%u failed\n", CMT_PORT);
			CLOSE_SOCK(user->sock_udp);
			return RS_ERROR;
		}
		len = sizeof(addr);
		if (getsockname(user->sock_udp, (struct sockaddr *)&addr, &len) != 0) {
			log_err(true, "bind notify socket failed\n");
			CLOSE_SOCK(user->sock_udp);
			return RS_ERROR;
		}
		user->sock_udp_port = ntohs(addr.sin_port);
		SET_SOCK_NBLOCK(user->sock_udp);
		CL_THREAD_READ_ON(&cl_priv->master, user->t_read_udp, udp_read, (void *)user, user->sock_udp);
		log_info("'%s' create udp sock fd = %d, port = %d\n", user->name, user->sock_udp, user->sock_udp_port);
        log_debug("udp_read add udp read %s %d\n",__FUNCTION__,__LINE__);
	}
	return RS_OK;
}

static int user_login_timeout(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);

	user->t_timer_login_timeout = NULL;

	if (user->never_establish && user->notify_login_err_count == 0) {
		if (user->udp_recv_pkts == 0 && user->tcp_recv_pkts == 0) {
			user->last_err = ULGE_NETWORK_ERROR;
		} else {
			user->last_err = ULGE_NETWORK_ERROR;
		}
		user->notify_login_err_count++;
		event_push_err(user->callback, UE_LOGIN_ERROR, user->handle, user->callback_handle, user->last_err);
		event_cancel_merge(user->handle);
	}

	return 0;
}

static bool is_udp_ctrl_device(u_int64_t sn)
{
	if(sn >= 600000000000ul && sn < 700000000000ul)
		return true;

	if(sn >= 808000000000ul && sn < 809000000000ul)
		return true;

	return false;
}

static user_t *udp_user_lookup(const char *username)
{
	user_t *user, *ret = NULL,*dev;
	bool is_sn = cl_is_sn(username);
	u_int64_t sn;

	if (is_sn)
		sn = atoll(username);

	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (is_sn) {
			if (user->sn == sn) {
				ret = user;
				goto done;
			}

			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == sn){
					ret = dev;
					goto done;
				}
			}
		} else {
			if (user->nickname != NULL && strcmp(user->nickname, username) == 0) {
				ret = user;
				goto done;
			}
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);

	return ret;
}

int char2hex(char *in, u_int8_t *out, int *outlen)
{
	int inlen = (int)strlen(in), i, c = 0;

	if (inlen % 2) {
		return -1;
	}

	inlen /= 2;

	if (*outlen < inlen) {
		return -1;
	}
	*outlen = inlen;

	for (i = 0; i < inlen; i++) {
		if (sscanf(&in[i * 2], "%02x", &c) != 1) {
			return -1;
		}

		out[i] = c;
	}

	return 0;
}

static int user_passwd_is_md5(user_t *user, char *passwd)
{
	char key_str[] = "ismd5str";
	int key_len = (int)strlen(key_str), len, outlen = sizeof(user->passwd_md5);

	len = (int)strlen(passwd);

	if (len != 32 + key_len) {
		return 0;
	}

	if (memcmp((u_int8_t*)&passwd[32], key_str, key_len) != 0) {
		return false;
	}

	passwd[32] = 0;
	len = 16;

	if (char2hex(passwd, user->passwd_md5, &outlen) < 0) {
		return -1;
	}

	user->passwd = cl_strdup(passwd);

	return 1;
}

u_int8_t user_build_display_stat(user_t *user, u_int8_t is_net_ok, u_int32_t last_click_time)
{
	time_t now = time(NULL);
	dev_probe_info_t *dev = NULL;
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;

//	log_debug("user[%p] %"PRIu64" build display stat, is_net_ok %u last_click_time %u [%u]\n", user, user->sn, is_net_ok, last_click_time, now - last_click_time);

	if (user->online) {
		//log_debug("is online\n");
		return DISPLAY_STAT_REAL_ONLINE;
	}

	// 局域网列表有
	if (get_dev_lan_ipaddr(user->sn) != 0) {
		log_debug("is lan dev\n");
		return DISPLAY_STAT_REAL_STAT;
	}

	if (!is_net_ok) {
		log_debug("net offline\n");
		return DISPLAY_STAT_OFF_LINE;
	}

	// 如果密码错误，直接让上面返回真实状态
	if (user->last_err) {
		if (user->last_err == ULGE_BAD_PASSWORD) {
			log_debug("user %"PRIu64" bad paswd\n", user->sn);
			return DISPLAY_STAT_REAL_STAT;
		}
	}

	// 没有点击过
	if (last_click_time == 0) {
		if ((u_int32_t)now - (u_int32_t)cl_priv->start_time < 60 * 5) {
			log_debug("< 5 min, virtual online\n");
			return DISPLAY_STAT_VIRTUAL_ONLINE;
		} else {
			log_debug("real stat\n");
			return DISPLAY_STAT_REAL_STAT;
		}
	}

	if ((u_int32_t)now - last_click_time > 60) {
		log_debug("clicked after 1 min, return real stat\n");
		return DISPLAY_STAT_REAL_STAT;
	}

	log_debug("return connecting stat\n");

	return DISPLAY_STAT_CONNECTING;
}

char *display_stat_string(u_int8_t stat)
{
	switch (stat) {
		case DISPLAY_STAT_REAL_ONLINE:
			return "DISPLAY_STAT_REAL_ONLINE";
		case DISPLAY_STAT_VIRTUAL_ONLINE:
			return "DISPLAY_STAT_VIRTUAL_ONLINE";
		case DISPLAY_STAT_OFF_LINE:
			return "DISPLAY_STAT_OFF_LINE";
		case DISPLAY_STAT_CONNECTING:
			return "DISPLAY_STAT_CONNECTING";
		case DISPLAY_STAT_REAL_STAT:
			return "DISPLAY_STAT_REAL_STAT";
		default:
			return "UNKNOWN DISPLAY STAT";
	}
}


static cl_thread_t *t_user_display_info_save = NULL;
static int user_display_info_save(cl_thread_t *t)
{
	FILE *fp = NULL;
	char path[256], buf[256];
	user_t *user;
//	slave_t *slave;
	int len, ret;

	t_user_display_info_save = NULL;

	log_debug("user_display_info_save\n");

	sprintf(path, "%s/dinfo.conf", cl_priv->dir);

	if ((fp = fopen(path, "a")) == NULL) {
		log_err(true, "fopen path[%s] failed\n", path);
		return 0;
	}

	/*
		sn1=xxx,sn2=xxx,click=xxx
	*/
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
	

		//log_debug("user %p %"PRIu64" sn2 0=%u click time %u\n", user, user->sn, 0, (u_int32_t)user->last_click_time);
		len = sprintf(buf, "sn1=%"PRIu64",sn2=%u,click=%u\r\n", user->sn, 0, (u_int32_t)user->last_click_time);
		ret = (int)fwrite(buf, 1, len, fp);
		log_debug("config [%s] ret %d\n", buf, ret);

#if 0		
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->ext_type != IJ_RFGW) {
				continue;
			}

			len = sprintf(buf, "sn1=%"PRIu64",sn2=%"PRIu64",click=%u,bms_stat=%d\r\n", user->sn, slave->sn, (u_int32_t)slave->last_click_time, slave->status);
			ret = (int)fwrite(buf, 1, len, fp);
			log_debug("config [%s] ret %d\n", buf, ret);
		}
#endif
	}

	fclose(fp);

	return 0;
}

void user_request_display_info_save(void)
{
	CL_THREAD_TIMER_OFF(t_user_display_info_save);

	CL_THREAD_TIMER_ON(&cl_priv->master, t_user_display_info_save, user_display_info_save, NULL, TIME_N_SECOND(1));
}

#if 0
void user_display_info_load_all(void)
{
	FILE *fp = NULL;
	char path[256], line[256];
	user_t *user;
	slave_t *slave;
	int ret;
	u_int64_t sn1, sn2;
	u_int32_t click, bms_stat;

	sprintf(path, "%s/dinfo.conf", cl_priv->dir);

	if ((fp = fopen(path, "r")) == NULL) {
		log_err(true, "fopen path[%s] failed\n");
		return;
	}

	while(fgets(line, sizeof(line), fp)) {
		user = slave = NULL;
		
		ret = sscanf(line, "sn1=%"PRIu64",sn2=%"PRIu64",click=%u,bms_stat=%d", &sn1, &sn2, &click, &bms_stat);

		if (ret < 3) {
			continue;
		}

		log_debug("load display info sn1=%"PRIu64",sn2=%"PRIu64",click=%u\n", sn1, sn2, click);
		if (ret >=4) {
			log_debug("		bms_stat=%d\n", bms_stat);
		}

		user = user_lookup_by_sn(sn1);
		if (!user) {
			log_err(false, "user sn %"PRIu64" not found\n", sn1);
			continue;
		}

		// 从设备
		if (ret >= 4) {
			slave = slave_lookup_by_sn(user, sn2);
			if (!slave) {
				log_err(false, "slave sn %"PRIu64" not found\n", sn2);
				continue;
			}

			slave->last_click_time = click;
			slave->status = bms_stat;

			continue;
		}

		// 主设备
		user->last_click_time = click;
	}

	fclose(fp);
}

void slave_display_info_load(slave_t *slave)
{
	FILE *fp = NULL;
	char path[256], line[256];
	user_t *user;
	int ret;
	u_int64_t sn1, sn2;
	u_int32_t click, bms_stat;

	sprintf(path, "%s/dinfo.conf", cl_priv->dir);

	if ((fp = fopen(path, "r")) == NULL) {
		log_err(true, "fopen path[%s] failed\n");
		return;
	}

	while (fgets(line, sizeof(line), fp)) {		
		ret = sscanf(line, "sn1=%"PRIu64",sn2=%"PRIu64",click=%u,bms_stat=%d", &sn1, &sn2, &click, &bms_stat);

		if (ret < 3) {
			continue;
		}

		log_debug("load display info sn1=%"PRIu64",sn2=%"PRIu64",click=%u\n", sn1, sn2, click);
		if (ret >= 4) {
			log_debug("		bms_stat=%d\n", bms_stat);
		}

		//主设备或者不是这个从设备
		if (sn2 == 0 || sn2 != slave->sn) {
			continue;
		}
		
		user = user_lookup_by_sn(sn1);
		if (!user) {
			log_err(false, "user sn %"PRIu64" not found\n", sn1);
			break;
		}

		slave->last_click_time = click;
		slave->status = bms_stat;

		log_debug("	update slave %"PRIu64" last_click_time %u stat %u\n", click, bms_stat);
		break;
	}

	fclose(fp);
}
#endif

void user_display_info_load(user_t *user)
{
	FILE *fp = NULL;
	char path[256], line[256];
//	slave_t *slave;
	int ret;
	u_int64_t sn1, sn2;
	u_int32_t click, bms_stat;

	sprintf(path, "%s/dinfo.conf", cl_priv->dir);

	if ((fp = fopen(path, "r")) == NULL) {
		log_err(true, "fopen path[%s] failed\n", path);
		return;
	}

	while(fgets(line, sizeof(line), fp)) {		
		ret = sscanf(line, "sn1=%"PRIu64",sn2=%"PRIu64",click=%u,bms_stat=%d", &sn1, &sn2, &click, &bms_stat);

		if (ret < 3) {
			continue;
		}

		log_debug("load display info sn1=%"PRIu64",sn2=%"PRIu64",click=%u\n", sn1, sn2, click);

		if (sn1 != user->sn) {
			continue;
		}

		// 不处理从设备
		if (sn2 != 0) {
			continue;
		}

#if 0
		// 从设备
		if (sn2 != 0 && ret >= 4) {
			slave = slave_lookup_by_sn(user, sn2);
			if (slave != NULL) {
				slave->last_click_time = click;
				slave->status = bms_stat;
			}
			
			continue;
		}
#endif
		// 主设备
		user->last_click_time = click;

		log_debug("update user %"PRIu64" last_click_time %u\n", click);
	}

	fclose(fp);
}

/**
	用于读取本地配置中设备上次点击时间
	以及开启定时器，定时计算展示用的设备状态
*/
static int user_display_stat_timer(cl_thread_t *t)
{
	u_int8_t new_stat;
	slave_t *slave;
	bool modify = false;
	time_t now = time(NULL);
	
	user_t *user = CL_THREAD_ARG(t);

	user->t_display_stat_timer = NULL;

	// 从配置文件读取上次点击时间，以及从设备最近一次状态
	if (!user->has_load_display_stat) {
		user->has_load_display_stat = 1;
		user_display_info_load(user);
	}

	new_stat = user_build_display_stat(user, !cl_priv->net_offline, (u_int32_t)user->last_click_time);
	if (new_stat != user->last_display_stat) {
		log_debug("user %"PRIu64" display stat %s => %s\n", user->sn, display_stat_string(user->last_display_stat), display_stat_string(new_stat));
		user->last_display_stat = new_stat;
		modify = true;
	}

	// 如果网关不处于在线或者假在线。在线的从设备状态改成登陆中
	if (!(user->online || new_stat == DISPLAY_STAT_VIRTUAL_ONLINE)) {
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if (slave->status == BMS_BIND_ONLINE) {
				slave->status = BMS_LOGINING;
				modify = true;
			}
		}
	}
	
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_display_stat_timer, user_display_stat_timer, user, TIME_N_MSECOND(500));

	// 一旦在线，清除点击标志
	if (user->online && user->last_click_time) {
		user->last_click_time = 0;
		user_request_display_info_save();
	}

	if (modify) {		
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	}

	//log_debug("				xxx user %"PRIu64" display_stat %u user->last_err %u\n", user->sn, user->last_display_stat, user->last_err);
	
	return 0;
}


user_t *user_create(bool is_phone_dev,
			char *username, char *passwd, char* qr_code, char *license, int len_license,
			cl_callback_t callback, void *callback_handle, bool authing, bool is_home)
{
	user_t *user = NULL;
	bool is_sn;
    u_int32_t dev_lan_ip=0;
	u_int64_t t_sn;
	bool is_udp_ctrl = false;
	u_int8_t sub_type = 0;
	u_int8_t ext_type = 0;

	if ( ! is_phone_dev ) {
		user = cl_user_lookup(username);
		if (user != NULL) {
			log_err(false, "create user %s failed: exist\n", username);
			return NULL;
		}
	}

	is_sn = cl_is_sn(username);
	if(is_sn){
		t_sn = atoll(username);
		is_udp_ctrl = is_udp_ctrl_device(t_sn);
	}

	if(is_sn && is_udp_ctrl)
	{
		user = udp_user_lookup(username);
		if (user != NULL) {
			log_err(false, "create udp user %s failed: exist\n", username);
			return NULL;
		}
	}
	
	if (user == NULL) {
		user = user_alloc();
		user->is_udp_ctrl = is_udp_ctrl;
		
#ifndef	NO_COMMUNITY		
		if (len_license) {
			user->dskey = parse_licence(license, &len_license);
			user->ds_type = TP_DS007;
			create_cmt_service(user);
		}else{
			user->dskey = NULL;
#endif		
			user->ds_type = TP_USER;
#ifndef	NO_COMMUNITY		
		}
#endif		

		user->sub_type = IJ_UNKNOW;
		
		if (is_sn){
			user->sn = t_sn;
			dev_lan_ip = get_dev_lan_ipaddr(user->sn);
			
			if (get_dev_lan_sub_type(user->sn, &sub_type, &ext_type)) {
				user->sub_type = sub_type;
				user->ext_type = ext_type;
			}
		}else
			user->nickname = cl_strdup(username);

		user->name = cl_strdup(username);
		user->udp_buf = pkt_new(0, MAX_UDP_PKT, user->ds_type);
		user->tcp_buf = pkt_new(0, MAX_TCP_PKT, user->ds_type);

		STLC_INIT_LIST_HEAD(&user->tcp_send_list);

		log_info("add user '%s'\n", username);

		if ( ! is_phone_dev && stlc_list_empty(&user->link)) {
			stlc_list_add_tail(&user->link, &cl_priv->user);
		}
		
		user->is_pud_authing = authing;
		user_create_udp(user);

		// 用户第一次添加的时候，也触发一个事件回调
		if(user->ds_type == TP_DS007)
			event_push(callback, UE_LOGIN_OFFLINE, user->handle, callback_handle);
		else
			event_push(callback, UE_LOGOUT, user->handle, callback_handle);
		event_cancel_merge(user->handle);
	} else {
		log_info("modify user '%s'\n", username);
	}

	if (user_passwd_is_md5(user, passwd)) {
		// md5测试登陆
	} else if (is_home) {
		SAFE_FREE(user->passwd);
		user->passwd = cl_strdup("");
		memcpy(user->passwd_md5, (u_int8_t *)passwd, 16);
		fmt_hex(user->passwd_md5_str, user->passwd_md5, 16);
	} else {
		SAFE_FREE(user->passwd);
		user->passwd = cl_strdup(passwd);
		hash_passwd(user->passwd_md5, passwd);
		fmt_hex(user->passwd_md5_str, user->passwd_md5, 16);
	}

	user->callback = callback;
	user->callback_handle = callback_handle;

	user->is_phone_user = cl_is_phone(username);
	if (user->is_phone_user) {
		pu_init_passwd(user);
		user_set_status(user, CS_AUTH);
	} else {
#ifdef	UCC_TEST
		user->is_udp_ctrl = true;
#endif
		user->direct_ip = dev_lan_ip;
        if (!user->is_udp_ctrl) {
            user->is_udp_ctrl = lan_dev_is_need_udp_login(user->sn);
        }
        //必须放在创建session的前面，创建的时候要处理
        if (qr_code != NULL) {
            user->qr_code = strdup(qr_code);
        }
        
		ucc_new(user);
        if (!user->is_udp_ctrl) {
            alarm_phone_alloc(user);
        }
        
        sa_init(user);
        if (dev_lan_ip!=0) {
            user_set_direct_login(user, ntohl(dev_lan_ip));
        }else{
            user_set_status(user, CS_DISP);
        }
	}
	
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_login_timeout, user_login_timeout, (void *)user, TIME_LOGIN_TIMEOUT);
//	if (user->sn != 0 || user->is_phone_user) {
//		em_start(user);
//	}
	
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_display_stat_timer, user_display_stat_timer, user, 0);

	rf_slave_init(user);
	//初始化user备份密码，默认为123456
	hash_passwd(user->back_passwd_md5, "123456");

	return user;
}

RS user_set_direct_login(user_t *user, u_int32_t ip)
{
    user->devserver_ip = ip;
	if (user->is_udp_ctrl) {
        user->direct_ip = ip;
#ifdef UCC_TEST		
	    user->devserver_port = DFL_UDP_CTRL_CLIENT_WAN_PORT;
#else
	    user->devserver_port = DFL_UDP_CTRL_CLIENT_LAN_PORT;
#endif		
	} else {
        user->direct_ip = ip;
	    user->devserver_port = DEV_SERVER_PORT;
	}
	
    log_info("user %s in LAN, ip %u.%u.%u.%u direct login!\n", user->name, IP_SHOW(ip));
    
    user_set_status(user, CS_DEV_CONNECTING);

	return RS_OK;
}

static RS user_login(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_user_t *up;
	char *username, *passwd, *license;

	up = (cln_user_t *)&pkt->data[0];
	username = up->data;
	passwd = username + up->len_name;
	license = passwd + up->len_passwd;

	//如果是支持联动切是手机用户登陆，则直接跳到手机用户登陆去
	if ((up->len_name > 0) && 
		app_is_la() && 
		(!cl_is_sn(username))) {
		la_phone_login(username, passwd, false);
		return RS_OK;
	}

	cl_lock(&cl_priv->mutex);
	user = user_create(false, username, passwd, NULL, license, up->len_license, up->callback, up->callback_handle, false, false);
	if (user == NULL) {
		ret = RS_ERROR;
	} 
	//else {
		//rf_slave_init(user);
//	}
	cl_unlock(&cl_priv->mutex);

	if (up->b_proc) {
		user->is_smartconf = true;
	}

	if ((ret == RS_OK) && la_is_valid()) {
		la_comm_timer_reset();
	}

	return ret;

}

static RS QR_user_login(cl_notify_pkt_t *pkt)
{
    RS ret = RS_OK;
    user_t *user;
    cln_user_t *up;
    char *username, *passwd, *license;
    
    up = (cln_user_t *)&pkt->data[0];
    username = up->data;
    passwd = username + up->len_name;
    license = passwd + up->len_passwd;
    
    cl_lock(&cl_priv->mutex);

    user = user_create(false, username, "", passwd, license, up->len_license, up->callback, up->callback_handle, false, false);
    if (user == NULL)
        ret = RS_ERROR;
    
    cl_unlock(&cl_priv->mutex);
    
    return ret;
    
}

user_t *user_modify(
			cl_handle_t handle, char *passwd,
			cl_callback_t callback, void *callback_handle, bool authing, bool is_la)
{
	user_t *user;
    u_int32_t dev_lan_ip=0;

	user = (user_t *)lookup_by_handle(HDLT_USER, handle);
	if (user == NULL) {
		log_err(false, "modify user %s failed: not exist\n", user->name);
		return NULL;
	}

	if (is_la) {
		memcpy(user->passwd_md5, passwd, 16);
		//这里可能有时差，有可能服务器还没修改设备密码成功
		return user;
	} else {
		if (user_passwd_is_md5(user, passwd)) {
			// md5测试登陆
		} else {
			STR_REPLACE(user->passwd, passwd);
			hash_passwd(user->passwd_md5, passwd);
			memcpy(user->new_passwd_md5, user->passwd_md5, 16);
		}
	}
	
	fmt_hex(user->passwd_md5_str, user->passwd_md5, 16);
	
	user->callback = callback;
	user->callback_handle = callback_handle;

	if (user->is_phone_user) {
		pu_init_passwd(user);
		user_set_status(user, CS_AUTH);
	} else {
		if (user->parent != NULL && user->parent->is_phone_user) {
			user->modify_passwd_flags = 0xFF; /* 故意多置起，免得通知密码修改成功的消息 */
			pu_modify_dev_pwd(user, passwd);
		}
        dev_lan_ip = user->direct_ip;
        if (dev_lan_ip != 0) {
            user_set_direct_login(user, dev_lan_ip);
        }else{
            user_set_status(user, CS_DISP);
        }
	}
    
	return user;
}

static RS user_modify_login(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_user_t *up;
	char *passwd, *license;

	up = (cln_user_t *)&pkt->data[0];
	passwd = up->data + up->len_name;
	license = passwd + up->len_passwd;

	cl_lock(&cl_priv->mutex);

	user = user_modify(up->user_handle, passwd, up->callback, up->callback_handle, false, false);
	if (user == NULL)
		ret = RS_ERROR;

	cl_unlock(&cl_priv->mutex);

	//do_passwd_sync_to_server(user);
	//这里要判断一下，如果设备是添加过圈子大的，那可能是非联动设备添加过错误密码了，在登陆成功后是需要同步密码的
	if (user) {
		user->maybe_need_pd_sync = true;
	}

	return ret;

}

static RS user_set_direct_ip(cl_notify_pkt_t *pkt)
{
	cln_user_t *up;
	user_t *user;
	RS ret = RS_OK;
	
	up = (cln_user_t *)&pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "user_set_direct_ip user 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	user->direct_ip = up->ip;
	user_set_direct_login(user, user->direct_ip);

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

static RS user_relogin(cl_notify_pkt_t *pkt)
{
	cln_user_t *up;
	user_t *user;
	RS ret = RS_OK;
	
	up = (cln_user_t *)&pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "user_relogin user 0x%08x failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	user->bp->is_binding = false;
    user->last_err = 0;
    event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
	
	if (user->status != CS_IDLE) {
        user_set_status(user, CS_IDLE);
	}

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

static void do_bind_slave_info(user_t *user, pkt_t *pkt)
{
	int i;
	net_bind_info_ctl_t *bic;
	bind_slave_info_t *bsi;
	slave_t *slave;
	net_header_t *hdr = (net_header_t*)pkt->data;

	bic = get_pkt_payload(pkt, net_bind_info_ctl_t);
	
	if (hdr->param_len < sizeof(net_bind_info_ctl_t) + sizeof(bind_slave_info_t)*bic->count) {
		log_err(false, "%s param len=%d, net_bind_info_ctl_t=%d, bind_slave_info_t=%d, count=%d\n",
			__FUNCTION__, hdr->param_len, sizeof(net_bind_info_ctl_t), sizeof(bind_slave_info_t), bic->count);
		return;
	}
	if (bic->action != BIND_INFO_A) {
		log_err(false, "%s action=%d, not BIND_INFO_A\n", __FUNCTION__, bic->action);
		return;
	}
	
	for (i = 0, bsi = (bind_slave_info_t *)&bic->data[0]; i < bic->count; i++, bsi++) {
		bsi->sn = ntoh_ll(bsi->sn);
		bsi->ip = ntohl(bsi->ip);
		bsi->other_master_sn = ntoh_ll(bsi->other_master_sn);

		slave = slave_lookup_by_sn(user, bsi->sn);
		if (slave == NULL) {
			log_err(false, "%s ignore slave sn=%012"PRIu64", bind_err=%d, other_master_sn=%012"PRIu64"\n",
				__FUNCTION__, bsi->sn, bsi->bind_errno, bsi->other_master_sn);
			continue;
		}

		if (slave->bind_error != bsi->bind_errno || slave->other_master_sn != bsi->other_master_sn) {
			slave->bind_error = bsi->bind_errno;
			slave->other_master_sn = bsi->other_master_sn;
			event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
		}
	}

}
	
/*
do_apns_config_a
收到苹果推送服务配置结果
*/
static void do_apns_config_a(user_t *user, pkt_t *pkt)
{
	net_phone_push_t *p;
	cl_apns_config_t *up;
	net_header_t *hdr = (net_header_t*)pkt->data;

	
	if (hdr->param_len < sizeof(*p)) {
		log_err(false, "%s param len %d < %d\n", __FUNCTION__, hdr->param_len, sizeof(*up));
		return;
	}
	p = get_pkt_payload(pkt, net_phone_push_t);
	p->token_len = ntohs(p->token_len);
	p->sn = ntoh_ll(p->sn);
	if (hdr->param_len < (sizeof(*p) + p->msg_len + p->token_len)) {
		log_err(false, "%s param len %d < %d\n", __FUNCTION__, hdr->param_len, (sizeof(*p) + p->msg_len + p->token_len));
		return;
	}
	
	if (user->apns_cfg == NULL) {
		user->apns_cfg = cl_malloc(sizeof(*up));
	}
	up = user->apns_cfg;
	if (up == NULL) {
		log_err(false, "%s malloc %d bytes failed\n", __FUNCTION__, sizeof(*up));
		return;
	}
	up->action = p->action;
	up->need_push = p->need_push;
	up->cert_id = p->reserved[0];
	up->reserved[0] = 0;
	up->reserved[1] = 0;
	up->reserved[2] = 0;
	up->token_len = (u_int8_t)p->token_len;
	up->msg_len = p->msg_len;
	strcpy(up->phone_ver, p->phone_ver);
	memcpy(up->token, p->token, up->token_len);
	memcpy(up->msg_prefix, &p->token[up->token_len], up->msg_len);
	event_push(user->callback, UE_APNS_CONFIG, user->handle, user->callback_handle);
	
}

static void test_cb(u_int16_t type, u_int16_t len, u_int8_t *data)
{
	char buf[32] = {0};

	memcpy(buf, data, len);

	log_err(false, "test_cb [%s]\n", buf);
}

static RS user_anps_config(cl_notify_pkt_t *clnpkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_apns_config_t *up;
	net_phone_push_t *p;
	pkt_t *pkt;
	
	up = (cln_apns_config_t *)&clnpkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	
	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (user->is_phone_user || user->is_udp_ctrl) {
		ret = pu_apns_config(user, clnpkt);
		goto done;
	}

	pkt = pkt_new(CMD_PHONE_APN_OPERATION, sizeof(*p) + up->cfg.token_len + up->cfg.msg_len, user->ds_type);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	p = get_pkt_payload(pkt, net_phone_push_t);
	p->action = up->cfg.action;
	p->msg_len = up->cfg.msg_len;
	p->token_len = ntohs(up->cfg.token_len);
	p->need_push = up->cfg.need_push;
	p->reserved[0]= up->cfg.cert_id;
	p->reserved[1]= 0;
	p->reserved[2]= 0;
	strcpy(p->phone_ver, up->cfg.phone_ver);
	p->sn = ntoh_ll(user->sn);

	if(up->cfg.token_len)
		memcpy(p->token, up->cfg.token, up->cfg.token_len);
	if(up->cfg.msg_len)
		memcpy(&p->token[up->cfg.token_len], up->cfg.msg_prefix, up->cfg.msg_len);
	user_add_pkt(user, pkt);	
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

/**
	通过SN和订阅配置文件来配置，HTTP方式
*/
static RS user_anps_config_by_sn(cl_notify_pkt_t *clnpkt)
{
	char url[1024];
	char prefix[512];
	unsigned char token[128];
	int i,n;
	cln_apns_config_v2_t *up;
//	u_int32_t nip;
	char pu_key[64];

	up = (cln_apns_config_v2_t *)&clnpkt->data[0];
		
	for(n = 0, i = 0; i < up->cfg.token_len; i++){
		n = n + sprintf((char*)(&token[n]), "%02x", (u_int8_t)up->cfg.token[i]);
	}
	if (up->cfg.msg_prefix[0])
		http_sprintf(prefix, "%s", up->cfg.msg_prefix);
	else
		prefix[0] = 0;	

	build_udp_sn_http_key(up->sn, pu_key);

	sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&cid=%d&token=%s&action=%d&need=%d&iver=%s&prefix=%s",
		DFL_CGI_SERVER, pu_key, up->cfg.cert_id, token, up->cfg.action, up->cfg.need_push, up->cfg.phone_ver, prefix);

	http_get(url, CLNE_HTTP_ANPS_CONFIG_SN, (cl_handle_t)(up->sn % 0xffffffff));

	log_debug("pu_apns_config url \n%s\n", url);

	return RS_OK;
}


static RS user_query_slave_stat(cl_notify_pkt_t *clnpkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	slave_t *slave;
	
	up = (cln_user_t *)&clnpkt->data[0];
	
	cl_lock(&cl_priv->mutex);

	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, up->user_handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	query_slave_stat(slave);

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

static void relog()
{
	user_t *user;

	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->status != CS_IDLE)
			user_set_status(user, CS_IDLE);
	}

	cl_unlock(&cl_priv->mutex);
}

static void relogin_when_network_changed(){
        
	user_t *user;
	return;

	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->direct_ip == 0 && user->status != CS_IDLE )
			user_set_status(user, CS_IDLE);
	}

	cl_unlock(&cl_priv->mutex);
}

RS ajust_dev_sub_type_by_sn(u_int64_t sn, u_int8_t* sub_type ,u_int8_t* ext_type)
{
    if (!sub_type || !ext_type) {
        return RS_ERROR;
    }

    
//    if (sn >= 825000010000ULL && sn <= 825000020000){//sn >= 808000000001ULL && sn <= 808000000009ULL) {
//        *sub_type = IJ_101;
//        *ext_type = ETYPE_IJ_EB_JNB_GLOBAL;
//        return RS_OK;
//    }
    
    return RS_OK;
}

static RS _map_test_dev_to_app_dev(u_int8_t* sub_type ,u_int8_t* ext_type)
{
    switch (*ext_type) {
        case ETYPE_IJ_TEST_8266_LED:
            *sub_type = IJ_830;
            *ext_type = ETYPE_IJ_830_LEDE;
            break;
            
        default:
            break;
    }
    return RS_OK;
}

static RS _map_app_dev_to_test_dev(u_int8_t* sub_type ,u_int8_t* ext_type)
{
    return RS_OK;
}

RS map_test_dev_sub_type(u_int8_t* sub_type ,u_int8_t* ext_type)
{
    u_int8_t t_sub_type;
    
    if (!sub_type || !ext_type) {
        return RS_ERROR;
    }
    
    t_sub_type = *sub_type;
    
    if (t_sub_type == IJ_TEST_DEV) {
        return _map_test_dev_to_app_dev(sub_type, ext_type);
    }
    
    return _map_app_dev_to_test_dev(sub_type,ext_type);
}

static void do_action_when_network_changed()
{
	switch(cl_priv->net_type){
		case NET_TYPE_NONE:
			break;
		default :
			{
				//切换网络重新log
				relogin_when_network_changed();
			}
			break;
	}
}

static RS user_do_805_config(cl_notify_pkt_t *clnpkt)
{
	RS ret = RS_OK;
	slave_t *slave;
	cln_common_info_t* com_info;
	misc_hdr_t* mh;
	net_805_config_hdr_t* config_hdr;
	pkt_t *pkt;
	int pkt_len = 0;
	net_805_screen_t* ns;
	net_805_beep_t* nb;
	
	com_info = (cln_common_info_t *)&clnpkt->data[0];
	pkt_len = sizeof(net_805_config_hdr_t)+sizeof(*mh);

	if(com_info->action == ACT_805_BEEP_CTRL){
		pkt_len += sizeof(net_805_beep_t);
	}else if(com_info->action == ACT_805_SCREEN_CTRL){
		pkt_len += sizeof(net_805_screen_t);
	}
	
	cl_lock(&cl_priv->mutex);
	if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, com_info->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found slave device\n", __FUNCTION__, com_info->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	

	pkt = pkt_new_v2(CMD_805_CONFIG,pkt_len , NHF_TRANSPARENT|NHF_WAIT_REPLY, slave->user->sn, slave->user->ds_type);
	if (pkt == NULL) {
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	config_hdr= get_pkt_payload(pkt, net_805_config_hdr_t);
	mh = (misc_hdr_t*)(config_hdr+1);
	if(com_info->action == ACT_805_BEEP_CTRL){
		config_hdr->cmd = NET_805_CONFIG_CTL;
		nb = (net_805_beep_t*)(mh+1);
		mh->type = htons(NET_805_BEEP);
		mh->len = htons(sizeof(*nb));
		nb->onoff = cci_u8_data(com_info);
	}else if(com_info->action == ACT_805_SCREEN_CTRL){
		config_hdr->cmd = NET_805_CONFIG_CTL;
		ns = (net_805_screen_t*)(mh+1);
		mh->type = htons(NET_805_SCREEN);
		mh->len = htons(sizeof(*ns));
		ns->onoff = cci_u8_data(com_info);
	}else if(com_info->action == ACT_805_BEEP_QUERY){
		config_hdr->cmd = NET_805_CONFIG_QUERY;
		mh->type = htons(NET_805_BEEP);
		mh->len = 0;
	}else if(com_info->action == ACT_805_SCREEN_QUERY){
		config_hdr->cmd = NET_805_CONFIG_QUERY;
		mh->type = htons(NET_805_SCREEN);
		mh->len = 0 ;
	}else{
		pkt_free(pkt);
		ret = RS_INVALID_PARAM;
		goto done;
	}
	 PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_805_CONFIG, NULL, callback_user_requst);

	user_add_pkt(slave->user, pkt);	
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

pkt_t *sa_dev_upgrade_flash_upgrade_pkt(ucc_session_t *s, u_int8_t num)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	u_int8_t *pdata = NULL;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + sizeof(num),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_UPGRADE, UCSOT_UPGRADE_FLASH, UCAT_FLASH_UPGRADE, sizeof(num));
	pdata = (u_int8_t *)(uo+1);
	*pdata = num;	
	
	return pkt;
}

RS sa_dev_upgrade_upgrade(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	pkt_t *pkt = NULL;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	pkt = sa_dev_upgrade_flash_upgrade_pkt(user->uc_session, up->num);
	if (pkt) {
		log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);		
		ucc_request_add(user->uc_session, pkt);
	} 

done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}

RS user_app_pkt_send(cl_notify_pkt_t *cln_pkt)
{
	cl_app_pkt_send_t *request = (cl_app_pkt_send_t *)cln_pkt->data;

	return app_send_pkt(request->type, request->ident, request->pkt, request->pkt_len);
}


///////////////////////////////////////////////////////////////////
bool user_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	if ( ! ((CLNE_USER_START <= pkt->type && pkt->type < CLNE_USER_END) 
		|| (CLNE_HTTP_START <= pkt->type && pkt->type < CLNE_HTTP_END)||
		(CLNE_NETWORK_CHANGED == pkt->type)))
		return false;
	
	switch (pkt->type) {
	case CLNE_USER_LOGIN:
		*ret = user_login(pkt);
		break;
    case CLNE_QR_CODE_LOGIN:
        *ret = QR_user_login(pkt);
        break;

	case CLNE_USER_MODIFY_LOGIN:
		*ret = user_modify_login(pkt);
		break;

	case CLNE_USER_SET_DIRECT_IP:
		*ret = user_set_direct_ip(pkt);
		break;

	case CLNE_USER_RELOGIN:
		*ret = user_relogin(pkt);
		break;

	case CLNE_USER_LOGOUT:
		*ret = user_del(pkt);
		break;

	case CLNE_RESOLV_DISP_SERVER:
		relog();
		*ret = RS_OK;
		break;

	case CLNE_USER_MODIFY_PASSWD:
		*ret = user_modify_passwd(pkt);
		break;

	case CLNE_USER_MODIFY_NICKNAME:
		*ret = user_modify_nickname(pkt);
		break;

	case CLNE_SLAVE_MODIFY_NAME:
		*ret = slave_modify_name(pkt);
		break;

	case CLNE_SLAVE_BIND:
		*ret = slave_bind(pkt);
		break;

	case CLNE_SLAVE_UNBIND:
		*ret = slave_unbind(pkt);
		break;

	case CLNE_USER_ADD_DEV:
    case CLNE_USER_ADD_QR_DEV:
		*ret = pu_add_dev(pkt);
		break;

	case CLNE_USER_DEL_DEV:
		*ret = pu_del_dev(pkt);
		break;

	case CLNE_USER_REGISTER:
		*ret = pu_register(pkt);
		break;

	case CLNE_USER_RESET:
		*ret = pu_reset(pkt);
		break;	

	case CLNE_USER_SEND_VCODE:
		*ret = pu_send_vcode(pkt);
		break;

	case CLNE_HTTP_IGNORE:
		*ret = pu_ignore_notify(pkt);
		break;

	case CLNE_HTTP_PU_REGISTER:
		*ret = pu_on_register_notify(pkt);
		break;

	case CLNE_HTTP_PU_RESET_PASSWD:
		*ret = pu_on_reset_notify(pkt);
		break;

	case CLNE_HTTP_PU_SEND_VCODE:
		*ret = pu_on_send_vcode_notify(pkt);
		break;

	case CLNE_HTTP_PU_MODIFY_PASSWD:
		*ret = pu_on_modify_passwd_notify(pkt);
		break;

	case CLNE_HTTP_PU_PUT_DICT_MODIFY_PASSWD:
		*ret = pu_on_dict_modify_passwd_notify(pkt);
		break;

	case CLNE_HTTP_PU_LOGIN:
		*ret = pu_on_login_notify(pkt);
		break;

	case CLNE_HTTP_PU_GET_DEV:
		*ret = pu_on_get_dev_notify(pkt);
		break;
		
	case CLNE_HTTP_PU_GET_DEV_TYPE:
		*ret = pu_on_get_devtype_notify(pkt);
		break;

	case CLNE_HTTP_PU_GET_DICT:
		*ret = pu_on_get_dict(pkt);
		break;

	case CLNE_HTTP_PU_PUT_DICT:
		*ret = pu_on_put_dict_notify(pkt);
		break;

	case CLNE_HTTP_PU_DICT_ADD_DEV:
		*ret = pu_on_dict_add_dev_notify(pkt);
		break;

	case CLNE_HTTP_PU_DICT_DEL_DEV:
		*ret = pu_on_dict_del_dev_notify(pkt);
		break;

	case CLNE_HTTP_PU_ADD_DEV:
		*ret = pu_on_add_dev_notify(pkt);
		break;

	case CLNE_HTTP_PU_DEL_DEV:
		*ret = pu_on_del_dev_notify(pkt);
		break;
	case CLNE_HTTP_DEV_VERSION:
		*ret = pu_on_dev_upgrade_notify(pkt);
		break;
	case CLNE_HTTP_RFDEV_VERSION:
		*ret = pu_on_rfdev_upgrade_notify(pkt);
		break;
	case CLNE_HTTP_RFDEV_UP_CHECK:
		*ret = pu_on_rfdev_upgrade_check_notify(pkt);
		break;
	case CLNE_USER_BIND_PHONE_Q:
		*ret = user_bind_phone_q(pkt);
		break;

	case CLNE_USER_BIND_PHONE_REQUEST_LIST:
		*ret = user_bind_phone_request_list_query(pkt);
		break;

	case CLNE_USER_BIND_PHONE_LIST:
		*ret = user_bind_phone_list_query(pkt);
		break;

	case CLNE_USER_BIND_PHONE_NORMAL:
		*ret = user_bind_phone_normal_allow(pkt);
		break;

	case CLNE_USER_BIND_PHONE_OPERATION:
		*ret = user_bind_phone_operate(pkt);
		break;

	case CLNE_HTTP_ANPS_CONFIG:
		*ret = pu_on_apns_config_notify(pkt);
		break;
		
	case CLNE_HTTP_ANPS_CONFIG_V2:
		*ret = pu_on_apns_config_notify_v2(pkt);
		break;
		
	case CLNE_HTTP_ANPS_CONFIG_SN:
		*ret = pu_on_apns_config_notify_sn(pkt);
		break;

	case CLNE_HTTP_MSG_PUSH:
		*ret = pu_on_msg_push_notify(pkt);
		break;

	case CLNE_APNS_CONFIG:
		*ret = user_anps_config(pkt);
		break;
		
	case CLNE_APNS_CONFIG_BY_SN:
		*ret = user_anps_config_by_sn(pkt);
		break;

	case CLNE_SLAVE_QUERY_STAT:
		*ret = user_query_slave_stat(pkt);
		break;

	case CLNE_HTTP_ENV_GET_WEATHER:
		*ret = em_on_get_weather(pkt);
		break;

	case CLNE_HTTP_ENV_GET_SUGGEST:
		*ret = em_on_get_suggest(pkt);
		break;

	case CLNE_HTTP_ENV_GET_PM25:
		*ret = em_on_get_pm25(pkt);
		break;

	case CLNE_HTTP_ENV_SET_CITY:
		*ret = em_on_set_city(pkt);
		break;
		
	case CLNE_HTTP_ENV_GET_CITY_LIST:
		*ret = em_on_get_city_list(pkt);
		break;

	case CLNE_SLAVE_OPEN_TELNET:
		*ret = slave_open_telnet(pkt);
		break;

	case CLNE_SLAVE_REBOOT:
		*ret = slave_reboot(pkt);
		break;

	case CLNE_DEV_UPGRADE_Q:
		*ret = dev_upgrade_q(pkt);
		break;

	case CLNE_DEV_UPGRADE_SET:
		*ret = dev_upgrade_set(pkt);
		break;
		
	case CLNE_DEV_UPGRADE_NOW:
		*ret = dev_upgrade_now(pkt);
		break;
		
	case CLNE_DEV_UPGRADE_CLI:
		*ret = sa_dev_upgrade_cli(pkt);
		break;
	case CLNE_DEV_UPGRADE_STM:
		*ret = sa_dev_stm_upgrade_cli(pkt);
		break;
	case CLNE_DEV_STM_SPE_UP:
		*ret = sa_dev_stm_upgrade_spe_cli(pkt);
		break;
	case CLNE_DEV_UPGRADE_EVM:
		*ret = sa_dev_upgrade_evm(pkt);
		break;
	case CLNE_DEV_805_CONFIG:
		*ret = user_do_805_config(pkt);
		break;

	case CLNE_NETWORK_CHANGED:
		do_action_when_network_changed();
		return false;
		break;
		
	case CLNE_DEV_ACTIVE:
		*ret = sa_dev_active(pkt);
		break;
	case CLNE_DEV_UPGRADE_CLI_NO_HEARD:
		*ret = sa_dev_upgrade_cli_no_head(pkt);
		break;
	case CLNE_DEV_UPGRADE_FLASH_ERASE:
		*ret = sa_dev_upgrade_erase(pkt);
		break;
	case CLNE_DEV_UPGRADE_FLASH_UPGRADE:
		*ret = sa_dev_upgrade_upgrade(pkt);
		break;
	case CLNE_APP_PKT_SEND:
		*ret = user_app_pkt_send(pkt);
		break;
	default:
		return false;
	}

	return true;
}

/*******************************************static***********************************************************************/
static u_int16_t const disp_stat_ports[STAT_PORT_COUNT] ={
	DISP_SERVER_PORT2,
	DISP_SERVER_PORT1,
	DISP_SERVER_PORT,
};

static int disp_get_port_index(u_int16_t const port)
{
	int i;
	
	for(i = 0; i < STAT_PORT_COUNT; i++) {
		if (port == disp_stat_ports[i]) {
			return i;
		}
	}

	return -1;
}

static disp_pkt_stat_port_t *disp_stat_find_by_ip(disp_pkt_stat_t *pdisp_stat, u_int32_t ip, u_int16_t port)
{
	int i, index;

	//get index
	index = disp_get_port_index(port);
	if (index < 0) {
		return NULL;
	}
	//get ip
	for(i = 0; i < (int)pdisp_stat->valid_count; i++) {
		if (ip == pdisp_stat->stat_ip[i].ip) {
			break;
		}
	}
	if (i == pdisp_stat->valid_count) {
		return NULL;
	}

	return &pdisp_stat->stat_ip[i].stat_port[index];
}

static disp_pkt_stat_port_t *disp_stat_get(u_int32_t ip, u_int16_t port)
{
	int index = 0;
	disp_pkt_stat_port_t *pr = NULL;

	if (cl_priv->disp_stat.valid_count >= MAX_DISP_SERVER_IP) {
		return NULL;
	}
	index = disp_get_port_index(port);
	if (index < 0) {
		return NULL;
	}
	cl_priv->disp_stat.stat_ip[cl_priv->disp_stat.valid_count].ip = ip;
	
	return &cl_priv->disp_stat.stat_ip[cl_priv->disp_stat.valid_count++].stat_port[index];
}

void disp_stat_send(u_int32_t ip, u_int16_t port)
{
	disp_pkt_stat_port_t *ps_port = NULL;

	log_debug("enter %s ip=%u.%u.%u.%u port=%u\n", __FUNCTION__, IP_SHOW(ip), port);
	ps_port = disp_stat_find_by_ip(&cl_priv->disp_stat, ip, port);
	if (!ps_port) {
		ps_port = disp_stat_get(ip, port);
	}
	if (!ps_port) {
		log_debug("%s ip=%u.%u.%u.%u port=%u not foud\n", __FUNCTION__, IP_SHOW(ip), port);
		return;
	}

	ps_port->send_pkt++;
}

void disp_stat_recv(u_int32_t ip, u_int16_t port)
{
	disp_pkt_stat_port_t *ps_port = NULL;

	ps_port = disp_stat_find_by_ip(&cl_priv->disp_stat, ip, port);
	if (!ps_port) {
		return;
	}

	ps_port->recv_pkt++;
}

u_int16_t disp_stat_get_valid_port(u_int32_t ip)
{
	int i,j;
	u_int16_t port_ret = 0;
	disp_pkt_stat_port_t *ps_port = NULL;
	disp_pkt_stat_ip_t *pip = NULL;

	for(i = 0; i < (int)cl_priv->disp_stat.valid_count; i++) {
		pip = &cl_priv->disp_stat.stat_ip[i];
		if (pip->ip != ip) {
			continue;
		}
		for(j = 0; j < STAT_PORT_COUNT; j++) {
			ps_port = &pip->stat_port[j];
			if (ps_port->recv_pkt != 0) {
				port_ret = disp_stat_ports[j];
				break;
			}
		}
	}

	return port_ret;
}

void disp_stat_dump()
{
	int i,j;
	u_int32_t send;
	u_int32_t recv;
	disp_pkt_stat_ip_t *pip = NULL;
	disp_pkt_stat_port_t *pport = NULL;
	u_int32_t now = get_sec();
	
	if (cl_priv->last_time == 0) {
		cl_priv->last_time = now;
	}
	for(i = 0; i < (int)cl_priv->disp_stat.valid_count; i++) {
		pip = &cl_priv->disp_stat.stat_ip[i];
		for(j = 0; j < STAT_PORT_COUNT; j++) {
			pport = &pip->stat_port[j];
			send = pport->send_pkt*74;
			recv = pport->recv_pkt*84;
			cl_priv->total_send += send;
			cl_priv->total_recv += recv;
			log_debug("ip=%u.%u.%u.%u port=%u send_pkt=%u recv_pkt=%u "
				"send_bytes=%u recv_bytes=%u\n", 
				IP_SHOW(pip->ip), disp_stat_ports[j], 
				pport->send_pkt, pport->recv_pkt, send,
				recv);
		}
	}
	cl_priv->total_num++;
	log_debug("disp_stat.valid_count=%u\n", cl_priv->disp_stat.valid_count);
	log_debug("now(s/m)=(%u/%u) last_time=%u time_diff=%u \n", 
		now,now/60, cl_priv->last_time, now - cl_priv->last_time);
	log_debug("average total_send(%u)/send_num(%u)=%u\n", 
		cl_priv->total_send, cl_priv->total_num, cl_priv->total_send/cl_priv->total_num);
	log_debug("average total_recv(%u)/send_num(%u)=%u\n", 
		cl_priv->total_recv, cl_priv->total_num, cl_priv->total_recv/cl_priv->total_num);
	cl_priv->last_time = now;
	log_debug("\n");
}

#ifdef ELE_STAT_CHECK
#define DIFF_CAL_TIME	(3600)
//离线时间超1小时
#define OFFLINE_MAX_TIME	(3600)

static void do_door_magnet_test()
{
	user_t *user, *usern;
	slave_t *slave, *slaven;
	cl_door_magnet_info_t* rd;
	u_int32_t ele_ave = 0;
	int num;
	static int cal_num = 0;
	u_int32_t now = get_sec();
	static u_int32_t last_time = 0;
	u_int8_t buff[1024*10];
	int index;
	int i;
	bool err = false;
	static bool first = true;
	u_int32_t diff = 0;

	cl_lock(&cl_priv->mutex);
	//记录第一次电量
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (user->sub_type != IJ_RFGW) {
			continue;
		}
		stlc_list_for_each_entry_safe(slave_t, slave, slaven, &user->slave, link) {
			if (slave->status == BMS_UNBIND) {
				continue;
			}
			if (slave->ext_type != RF_EXT_TYPE_DOOR_MAGNETV2) {
				continue;
			}
			if (slave->ele_stat.first_battary != 0) {
				continue;
			}
			rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
			slave->ele_stat.first_battary = rd->stat.battary;
		}
	}

	//多久计算一次
	if (last_time == 0) {
		last_time = now;
	}

	if (!first && (last_time + DIFF_CAL_TIME > now)) {
		goto end;
	}
	last_time = now;
	//计算均值
	num = 0;
	ele_ave = 0;
	first = false;
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (user->sub_type != IJ_RFGW) {
			continue;
		}
		stlc_list_for_each_entry_safe(slave_t, slave, slaven, &user->slave, link) {
			if (slave->status == BMS_UNBIND) {
				continue;
			}
			if (slave->ext_type != RF_EXT_TYPE_DOOR_MAGNETV2) {
				continue;
			}
			rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
			if (slave->ele_stat.first_battary >= rd->stat.battary) {
				ele_ave += slave->ele_stat.first_battary - rd->stat.battary;
			}
			num++;
			if (slave->ele_stat.first_battary >= rd->stat.battary) {
				slave->ele_stat.battary_diff[slave->ele_stat.index++] = slave->ele_stat.first_battary - rd->stat.battary;
			}
			slave->ele_stat.index %= ELE_STAT_MAX_NUM;
		}
	}

	if (num > 0) {
		ele_ave *= 100;
		ele_ave /= num;
	}
	//printf
	log_debug("cal_num=%u ele_diff_ave=%u ******************************\n\n\n\n\n\n", 
	cal_num++, ele_ave);
	stlc_list_for_each_entry_safe(user_t, user, usern, &cl_priv->user, link) {
		if (user->sub_type != IJ_RFGW) {
			continue;
		}
		stlc_list_for_each_entry_safe(slave_t, slave, slaven, &user->slave, link) {
			if (slave->status == BMS_UNBIND) {
				continue;
			}
			if (slave->ext_type != RF_EXT_TYPE_DOOR_MAGNETV2) {
				continue;
			}
			rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
			diff = (u_int32_t)(100*slave->ele_stat.battary_diff[(slave->ele_stat.index + ELE_STAT_MAX_NUM - 1)%ELE_STAT_MAX_NUM]);
			if (diff > (2*ele_ave)) {
				err = true;
			} else {
				err = false;
			}
			index = 0;
			index += sprintf(buff+index, "sn=%llu status[%s] first_battary=%u cur_battary=%u status=%u", 
				slave->sn, err?"warning":"normal", slave->ele_stat.first_battary, rd->stat.battary, slave->status);
			//离线太久
			if ((slave->ele_stat.offline_time > 0) && 
				(slave->ele_stat.offline_time + OFFLINE_MAX_TIME) < now) {
				index += sprintf(buff+index, "rf offline %u(s) warning", now - slave->ele_stat.offline_time);
			}
			for(i = 0; i < slave->ele_stat.index; i++) {
				if (i%5 == 0) {
					index += sprintf(buff+index, "\n");
				}
				index += sprintf(buff+index, "diff[%d]=%u ", i, slave->ele_stat.battary_diff[i]);
			}
			
			log_debug("%s\n\n", buff);
		}
	}

end:
	cl_unlock(&cl_priv->mutex);
}
#endif

static void disp_stat_clean_check()
{
	int i;

	for(i = 0; i < (int)cl_priv->disp_stat.valid_count; i++) {
		if (memcmp((void *)&cl_priv->disp_stat.stat_ip[i], 
			(void *)&cl_priv->disp_stat_back.stat_ip[i], 
			sizeof(disp_pkt_stat_ip_t))) {
			continue;
		}
		//清空端口接收数据
		memset((void *)&cl_priv->disp_stat.stat_ip[i].stat_port, 0, sizeof(disp_pkt_stat_port_t));
	}
	memcpy((void *)&cl_priv->disp_stat_back, (void *)&cl_priv->disp_stat, sizeof(cl_priv->disp_stat));
}

int disp_stat_timeout(cl_thread_t *t)
{
	cl_priv->t_disp_time_out = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, cl_priv->t_disp_time_out, disp_stat_timeout, NULL, TIME_N_SECOND(DISP_STAT_TIME_OUT));

	//disp_stat_dump();
	//这里简单处理下，这个超时跟上次的比较，如果一样，可以表示这个超时内没接收到报文，可以清空了
	disp_stat_clean_check();

	//门磁电量统计测试
#ifdef ELE_STAT_CHECK	
	do_door_magnet_test();
#endif

	
	return 0;
}


/******************************************************************************************************************/


