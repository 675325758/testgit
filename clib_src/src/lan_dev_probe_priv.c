#include "lan_dev_probe_priv.h"
#include "cl_lan_dev_probe.h"
#include "cl_priv.h"
#include "md5.h"
#include "smart_config_priv.h"
#include "lbs.h"
#ifdef WIN32
#include "Iphlpapi.h"
#else
#include <net/if.h>
#endif
#include "uc_client.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "evm_scm_ctrl.h"
#include "cl_user.h"
#include "linkage_client.h"

#define DEF_PROBE_TIMES (3)
#define DEF_DEVICE_TIME_OUT (16)

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

static void ldp_del_and_free_dev_info(dev_probe_info_t*dpi)
{
    if (!dpi) {
        return;
    }
    CL_THREAD_TIMER_OFF(dpi->time_out);
    CL_THREAD_TIMER_OFF(dpi->t_timer);
    SAFE_FREE(dpi->ssid);
    SAFE_FREE(dpi->wifi_pwd);
    stlc_list_del_init(&dpi->link);
    cl_free(dpi);
}

static void ldp_clear_all_devices(lan_dev_probe_ctrl_t* ldpc)
{
    dev_probe_info_t* dpi,*next;
    
    stlc_list_for_each_entry_safe(dev_probe_info_t, dpi, next,&ldpc->dev_info_list, link){
        ldp_del_and_free_dev_info(dpi);
    }
}

static int ldp_time_out_timer(cl_thread_t *t)
{
    dev_probe_info_t* dpi = (dev_probe_info_t*)CL_THREAD_ARG(t);
    lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    user_t* user;
    
    dpi->time_out = NULL;
    log_debug("Device [%12"PRIu64"] timer out ! last recv time[%u] cur_time[%u] Delete it!\n",dpi->dev_sn,
              dpi->recv_time,get_sec());
    ldp_del_and_free_dev_info(dpi);
    
    if ((user = user_lookup_by_sn(dpi->dev_sn)) != NULL){
	 user->direct_ip = 0;
	 user->devserver_ip = 0;
	 lbs_update_lan_detect(user, 0);
        //登录未出错，就走服务器
        if (user->status != CS_LOGIN_ERR &&  user->status!=CS_ESTABLISH) {
            user_set_status(user, CS_IDLE);
        }
    }
    
    if (ldpc && ldpc->callback) {
        event_push(ldpc->callback, LDPE_DEVICE_CHANGED, 0, ldpc->callback_handle);
    }
    
    
    return 0;
}

static void ldp_reset_time_out(dev_probe_info_t* dpi,u_int32_t sec)
{
    
    if (!dpi) {
        return;
    }
    CL_THREAD_TIMER_OFF(dpi->time_out);
    
    if (sec>0) {
        CL_THREAD_TIMER_ON(&cl_priv->master, dpi->time_out, ldp_time_out_timer, dpi, TIME_N_SECOND(sec));
    }
    
}

static void ldp_stop_probe_device(lan_dev_probe_ctrl_t* ldpc)
{
    if (!ldpc->is_probe_enable) {
        return;
    }
    ldpc->callback = ldpc->callback_handle = NULL;
    CL_THREAD_OFF(ldpc->t_probe);
    CL_THREAD_OFF(ldpc->t_read);
    CLOSE_SOCK(ldpc->udp_socket);
    
    ldpc->is_probe_enable = false;    
}

static void bcast_list_free(lan_dev_probe_ctrl_t* ldpc)
{
	brcdst_t* node, *next;

	stlc_list_for_each_entry_safe(brcdst_t, node, next, &ldpc->bcast_lst, link){
		stlc_list_del_init(&node->link);
		cl_free(node);
	}

	STLC_INIT_LIST_HEAD(&ldpc->bcast_lst);
}

/*
	获取该手机/PC机所有接口的局部广播地址。
	把255.255.255.255插到第一个
	不同系统需要实现不同的
*/
#ifdef WIN32
static void bcast_list_get(lan_dev_probe_ctrl_t* ldpc)
{
	ULONG len;
	IP_ADAPTER_INFO info[100];
	PIP_ADAPTER_INFO pAdapter = NULL;
	IP_ADDR_STRING *pip;
	u_int32_t ip, mask;
	u_int32_t now;
	DWORD addr;
	int rev = 0;
	brcdst_t *node;

	now = get_sec();
	if (now - ldpc->last_get_bcast < 2)
		return;

	bcast_list_free(ldpc);

	node = cl_calloc(sizeof(brcdst_t), 1);
	node->bcast_addr = 0xFFFFFFFF;
	stlc_list_add_tail(&node->link, &ldpc->bcast_lst);
	
	len = sizeof(info);
	rev = GetAdaptersInfo(info, &len);
	if(rev != ERROR_SUCCESS)
		return;

	pAdapter = info;
	for (pAdapter = info; pAdapter != NULL; pAdapter = pAdapter->Next) {
		for (pip = &pAdapter->IpAddressList; pip != NULL; pip = pip->Next) {
			ip = inet_addr(pip->IpAddress.String);
			mask = inet_addr(pip->IpMask.String);

			if (ip != 0 && ip != 0xFFFFFFFF && mask != 0 && mask != 0xFFFFFFFF) {
				addr = (ip | (~mask));
				
				node = cl_calloc(sizeof(brcdst_t), 1);
				node->bcast_addr = addr;
				stlc_list_add_tail(&node->link, &ldpc->bcast_lst);
			}
		}
	}

	ldpc->last_get_bcast = now;
}

#else /* iOS、Android */

static void ios_bcast_list_get(lan_dev_probe_ctrl_t* ldpc)
{
	u_int32_t now;
	u_int32_t addr;
	brcdst_t *node;
	int i=0;
	int sockfd = -1;
	struct ifconf ifconf;
	unsigned char buf[1024];
	struct ifreq *ifreq;
	struct ifreq ifr;

	now = get_sec();
	if (now - ldpc->last_get_bcast < 2)
		return;

	bcast_list_free(ldpc);

	node = cl_calloc(sizeof(brcdst_t), 1);
	node->bcast_addr = 0xFFFFFFFF;
	stlc_list_add_tail(&node->link, &ldpc->bcast_lst);

	//获取接口信息
	ifconf.ifc_len = 1024;
	ifconf.ifc_buf = buf;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		ldpc->last_get_bcast = now;
		return;
	}  

	ioctl(sockfd, SIOCGIFCONF, &ifconf); 

	ifreq = (struct ifreq*)buf;  
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
//		log_debug("name = [%s]\n", ifreq->ifr_name);
//		log_debug("local addr = [%s]\n", 
//		inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));

		if(ioctl(sockfd,SIOCGIFBRDADDR,ifreq)==0){
//			log_debug("broad addr = [%s]\n", 
//			inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_broadaddr))->sin_addr));
			addr = ((struct sockaddr_in*)&(ifreq->ifr_broadaddr))->sin_addr.s_addr;
			//不是0，也不是全f，发吧
			if(addr !=0 && addr != 0xFFFFFFFF){
				node = cl_calloc(sizeof(brcdst_t), 1);
				if(node != NULL){
					node->bcast_addr = addr;
					stlc_list_add_tail(&node->link, &ldpc->bcast_lst);
				}
			}
		}
		ifreq++;
	}

	ldpc->last_get_bcast = now;
	CLOSE_SOCK(sockfd);
}

static void andriod_bcast_list_get(lan_dev_probe_ctrl_t* ldpc)
{
	u_int32_t now;
	u_int32_t addr;
	brcdst_t *node;
	int i=0;
	int sockfd = -1;
	struct ifconf ifconf;
	unsigned char buf[1024];
	struct ifreq *ifreq;

	now = get_sec();
	if (now - ldpc->last_get_bcast < 2)
		return;

	bcast_list_free(ldpc);
	switch(cl_priv->net_type){
		//以太或wifi网络才进行局域网扫描
		case NET_TYPE_NONE:
		case NET_TYPE_ETH:
		case NET_TYPE_WIFI:
			break;
		//其它网络就不扫描了
		default:
			return;
	}

	
	//获取接口信息
	ifconf.ifc_len = sizeof(buf);
	ifconf.ifc_buf = buf;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		ldpc->last_get_bcast = now;
		return;
	}  

	ioctl(sockfd, SIOCGIFCONF, &ifconf);   

	ifreq = (struct ifreq*)buf;  
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
#if 0
		log_debug("name = [%s]\n", ifreq->ifr_name);
		log_debug("local addr = [%s]\n", 
		inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
#endif
		if(ioctl(sockfd,SIOCGIFBRDADDR,ifreq)==0){
#if 0
			log_debug("broad addr = [%s]\n", 
			inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_broadaddr))->sin_addr));
#endif
			addr = ((struct sockaddr_in*)&(ifreq->ifr_broadaddr))->sin_addr.s_addr;
			//不是0，也不是全f，发吧
			if(addr !=0 && addr != 0xFFFFFFFF){
				node = cl_calloc(sizeof(brcdst_t), 1);
				if(node != NULL){
					node->bcast_addr = addr;
					stlc_list_add_tail(&node->link, &ldpc->bcast_lst);
				}
			}
		}
		ifreq++;
	}
	
	node = cl_calloc(sizeof(brcdst_t), 1);
	if(node){
		node->bcast_addr = 0xFFFFFFFF;
		stlc_list_add_tail(&node->link, &ldpc->bcast_lst);
	}			

	ldpc->last_get_bcast = now;
	CLOSE_SOCK(sockfd);
}

static void bcast_list_get(lan_dev_probe_ctrl_t* ldpc)
{
	if(cl_priv->cleint_type == CID_ANDROID){
		andriod_bcast_list_get(ldpc);
	}else{
		ios_bcast_list_get(ldpc);
	}
}
#endif

static void lan_send_bcast(lan_dev_probe_ctrl_t* ldpc, void *pkt, int len)
{
	brcdst_t *node;
    struct sockaddr_in addr;
    int r, addr_len;
	u_int32_t app_baddr = 0;

	if (cl_priv) {
		app_baddr = htonl(cl_priv->app_baddr);
	}
	
	bcast_list_get(ldpc);
	
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(IJCFG_PORT_DEV);
    addr_len = sizeof(addr);

	stlc_list_for_each_entry(brcdst_t, node, &ldpc->bcast_lst, link) {
	    addr.sin_addr.s_addr = node->bcast_addr;
	    r = (int)sendto(ldpc->udp_socket, pkt, len, 0, (struct sockaddr *)&addr, addr_len);
		if (node->bcast_addr == app_baddr) {
			app_baddr = 0;
		}
	}

	//app上层可能也下发了个广播地址，也发一遍
	if (app_baddr != 0) {
		addr.sin_addr.s_addr = app_baddr;
		//log_debug("send %u.%u.%u.%u\n", IP_SHOW(app_baddr));
		r = (int)sendto(ldpc->udp_socket, pkt, len, 0, (struct sockaddr *)&addr, addr_len);
	}
}

static void lan_dev_dump_timer()
{
	dev_probe_info_t* dest;
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	static u_int32_t last_time = 0;
	u_int32_t now = get_sec();

	brcdst_t *node;

	if (last_time + 30 > now) {
		return;
	}
	last_time = now;

    if (!stlc_list_empty(&ldpc->dev_info_list)) {
		nd_lan_debug(NULL, "enter lan_dev_dump_timer lan dev info dump\n");
		stlc_list_for_each_entry(dev_probe_info_t, dest, &ldpc->dev_info_list, link){
			nd_lan_debug(dest, "sn=%"PRIu64" sub_type=%u ext_type=%u peer_ip=%u.%u.%u.%u recv_time=%u "
			"sm_success_time=%u la_is_valid=%u is_la_new=%u home_id=%u\n", 
			dest->dev_sn, dest->sub_type, dest->ext_type, IP_SHOW(dest->peer_ip), dest->recv_time, 
			dest->sm_success_time, dest->la_info.is_valid, dest->la_info.is_la_new, dest->la_info.home_id);
		}
	}

	bcast_list_get(ldpc);
	if (!stlc_list_empty(&ldpc->bcast_lst)) {
		nd_lan_debug(NULL, "enter lan_dev_dump_timer bcast_list dump\n");
		stlc_list_for_each_entry(brcdst_t, node, &ldpc->bcast_lst, link) {
		    nd_lan_debug(NULL, "ip=%u.%u.%u.%u\n", IP_SHOW(htonl(node->bcast_addr)));
		}
	}

}

static int timer_lan_dev_probe(cl_thread_t *t)
{
    lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    cfg_pkt_dhr_t pkt = {0};
    int r;
    
    if (!ldpc) {
        return 0;
    }
    ldpc->t_probe = NULL;
    
    /*如果是第一次启动，则300毫秒1次，连续发3个包，后续3秒发1次报文*/
    if (ldpc->probe_times<DEF_PROBE_TIMES) {
        CL_THREAD_TIMER_ON(&cl_priv->master, ldpc->t_probe, timer_lan_dev_probe, NULL, TIME_N_MSECOND(300));
    }else{
    	if(cl_priv->run_in_background){
		//前台2-3秒
       	 r = rand()%1000 + 2000;
    	}else{
    		//后台 3-4秒
    		 r = rand()%1000 + 3000;
    	}
		if(cl_priv->is_pt_mode)
			r = 200; //产测模式
        CL_THREAD_TIMER_ON(&cl_priv->master, ldpc->t_probe, timer_lan_dev_probe, NULL, TIME_N_MSECOND(r));
    }
    
    ldpc->probe_times++;
    if (ldpc->probe_times>50) {
        ldpc->probe_times = DEF_PROBE_TIMES+1;
    }
    
    PHONE_BACKGROUND_RETURN_CHECK();

    
    pkt.cmd = htons(CFGPT_DISCOVERY);
    pkt.src_sn = ntoh_ll(ldpc->random_sn);
	lan_send_bcast(ldpc, (void*)&pkt, sizeof(cfg_pkt_dhr_t));

#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, sizeof(pkt));
#endif

	// 定时打印下局域网扫描设备信息
	lan_dev_dump_timer();
    
	return 0;
}
static void send_lan_udp(dev_probe_info_t *dev, char *param, int param_len, u_int16_t cmd)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	cfg_pkt_dhr_t *hdr;
	char *data;
	int total;
	struct sockaddr_in addr;

	total = sizeof(*hdr) + param_len;
	data = cl_malloc(total);
	if(data == NULL)
		return;
	hdr = (cfg_pkt_dhr_t *)data;
	hdr->src_sn = ntoh_ll(ldpc->random_sn);
	hdr->dst_sn = ntoh_ll(dev->dev_sn);
	hdr->cmd = ntohs(cmd);
	hdr->param_len = ntohs((u_int16_t)param_len);
	hdr->req_id = ntohs(dev->req_id);
	hdr->flags = ntohs(0);
	if(param_len)
		memcpy(hdr->data, param, param_len);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = (dev->peer_ip);
	addr.sin_port = htons(IJCFG_PORT_DEV);
	sendto(ldpc->udp_socket, data, total, 0, (struct sockaddr *)&addr, sizeof(addr));

#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, total);
#endif
    
	cl_free(data);
}

static int timer_lan_dev_auth(cl_thread_t *t)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	dev_probe_info_t *dev;
	u_int8_t param[16];
	MD5_CTX md5_ctx;

	cl_lock(&ldpc->mutex);
	dev = CL_THREAD_ARG(t);
	if(dev == NULL)
		goto done;
	dev->t_timer = NULL;
	if(++dev->retry > 3){
		if(ldpc->callback)
			event_push_err(ldpc->callback, LDPE_TIMEOUT_FAIL, dev->handle, ldpc->callback_handle, ERR_TIMEOUT);
		dev->probe_status = PS_INIT;
		goto done;
	}
	CL_THREAD_TIMER_ON(&cl_priv->master, dev->t_timer, timer_lan_dev_auth, dev, TIME_N_SECOND(2));
	MD5Init(&md5_ctx);
	MD5Update(&md5_ctx, dev->auth, 16);
	MD5Update(&md5_ctx, (u_int8_t*)dev->md5_pwd, 16);
	MD5Final(param, &md5_ctx);
	send_lan_udp(dev, (char*)param, sizeof(param), CFGPT_AUTH);

done:
	cl_unlock(&ldpc->mutex);
	return 0;
}

static int timer_lan_wifi_config(cl_thread_t *t)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	dev_probe_info_t *dev;
	cfg_pkt_set_config_t *cfg;
	char param[1024];
	char pwds[512];
	int len;

	cl_lock(&ldpc->mutex);
	dev = CL_THREAD_ARG(t);
	if(dev == NULL)
		goto done;
	dev->t_timer = NULL;
	if(++dev->retry > 3){
		if(ldpc->callback)
			event_push_err(ldpc->callback, LDPE_TIMEOUT_FAIL, dev->handle, ldpc->callback_handle, ERR_TIMEOUT);
		dev->probe_status = PS_INIT;
		goto done;
	}
	CL_THREAD_TIMER_ON(&cl_priv->master, dev->t_timer, timer_lan_wifi_config, dev, TIME_N_SECOND(2));
	if (dev->wifi_pwd == NULL || dev->wifi_pwd[0] == 0) {
		strcpy(pwds, NO_PASSWD);
	} else {
		sprintf(pwds, HAS_PASSWD, dev->wifi_pwd);
	}
	
	cfg = (cfg_pkt_set_config_t*)param;
	cfg->cfg_cnt = ntohs(0);
	cfg->enc = 0;
	cfg->pad = 0;
	memset(cfg->checksum, 0, sizeof(cfg->checksum));	
	if((dev->sub_type == IJ_001) || (dev->sub_type == IJ_003) || (dev->sub_type == IJ_803))
		len = sprintf((char*)cfg->config, CFGPF_PARAM_FMT, MODE_INDEPEND, dev->ssid, pwds);
	else
		len = sprintf((char*)cfg->config, CFGPF_PARAM_FMT, MODE_MASTER, dev->ssid, pwds);
	send_lan_udp(dev, param, sizeof(*cfg) + len +1, CFGPT_SET_CONFIG);
	
done:
	cl_unlock(&ldpc->mutex);
	return 0;
}

static dev_probe_info_t* find_dpi_by_sn(lan_dev_probe_ctrl_t* ldpc,u_int64_t sn)
{
    dev_probe_info_t* dest;
    
    stlc_list_for_each_entry(dev_probe_info_t, dest, &ldpc->dev_info_list, link){
        if (dest->dev_sn == sn) {
            return dest;
        }
    }
    
    return NULL;
}

static void update_device_info(lan_dev_probe_ctrl_t* ldpc,dev_probe_info_t* dpi)
{
	user_t *user;
	dev_probe_info_t* dest = find_dpi_by_sn(ldpc,dpi->dev_sn);
    
    if (dest) {
        u_int32_t t_ip = ntohl(dpi->peer_ip);
        ldp_reset_time_out(dest, DEF_DEVICE_TIME_OUT);
//        log_debug("Update Device [%012"PRIu64"] timer! ip[%u.%u.%u.%u] time interval[%u]\n",dpi->dev_sn,
//                  IP_SHOW(t_ip),get_sec()-dest->recv_time);
    }
    
    if(dpi->peer_ip == 0){
        return;
    }
    
    if (dest) {
        dest->recv_time = dpi->recv_time;
        dest->sub_type = dpi->sub_type;
        dest->type = dpi->type;
        dest->peer_ip = dpi->peer_ip;
	  	dest->ext_type = dpi->ext_type;
	  	memcpy(dest->developer_id, dpi->developer_id, sizeof(dest->developer_id));
        dest->sm_success_time = dpi->sm_success_time;
        dest->dev_run_mode = dpi->dev_run_mode;
        memcpy(dest->auth, dpi->auth, sizeof(dest->auth));
        dest->flags = dpi->flags;
        //没通知过用户，立刻通知
        if (ldpc->callback && !ldpc->notify_user_times) {
            ldpc->notify_user_times = 0x1;
            event_push(ldpc->callback, LDPE_DEVICE_CHANGED, 0, ldpc->callback_handle);
        } else {
			//如果联动标志变化了也要通知一下
			if (memcmp((void *)&dest->la_info, (void *)&dpi->la_info, sizeof(dest->la_info))) {
				event_push(ldpc->callback, LDPE_DEVICE_CHANGED, 0, ldpc->callback_handle);
			}
		}
		memcpy((void *)&dest->la_info, (void *)&dpi->la_info, sizeof(dest->la_info));
    }else {
        dest = cl_calloc(sizeof(*dest), 1);
        if (!dest) {
            return;
        }
        dest->handle = handle_create(HDLT_TMP);
        dest->dev_sn = dpi->dev_sn;
        dest->recv_time = dpi->recv_time;
        dest->sub_type = dpi->sub_type;
        dest->type = dpi->type;
        dest->peer_ip = dpi->peer_ip;
		dest->ext_type = dpi->ext_type;
		memcpy(dest->developer_id, dpi->developer_id, sizeof(dest->developer_id));
        dest->dev_run_mode = dpi->dev_run_mode;
        dest->sm_success_time = dpi->sm_success_time;
        dest->flags = dpi->flags;
		memcpy((void *)&dest->la_info, (void *)&dpi->la_info, sizeof(dest->la_info));
        memcpy(dest->auth, dpi->auth, sizeof(dest->auth));
        stlc_list_add_tail(&dest->link, &ldpc->dev_info_list);
        
        log_debug("Discover Device %012"PRIu64"  ip [%u.%u.%u.%u] type [%x]!\n",dpi->dev_sn,IP_SHOW(dpi->peer_ip),dpi->type);
        ldp_reset_time_out(dest, DEF_DEVICE_TIME_OUT);
        //有变化，立刻通知
        if (ldpc->callback) {
            ldpc->notify_user_times = 0x1;
            event_push(ldpc->callback, LDPE_DEVICE_CHANGED, 0, ldpc->callback_handle);
        }
    }

	if ((user = user_lookup_by_sn(dest->dev_sn)) == NULL)
		return;

	//局域网扫描到设备后更新下类型
	if (user->sub_type != dest->sub_type) {
		user->sub_type = dest->sub_type;
		user->ext_type = dest->ext_type;
	}
	if (user->lbs != NULL)
		lbs_update_lan_detect(user, 1);

	if ((user->status == CS_ESTABLISH && user->online) || user->devserver_ip == dest->peer_ip || dest->peer_ip == 0){
		if(!user->is_udp_ctrl ){
			return;
		}
		
		if(user->direct_ip!=0){
			return;
		}
		
	}
		
	if (user->status == CS_LOGIN_ERR 
		&& (user->last_err != ULGE_NETWORK_ERROR && user->last_err != ULGE_SERVER_BUSY 
		&& user->last_err !=ULGE_DEV_OFF_LINE && user->last_err != ERR_NONE))
		return;
	
	// 未(成功建立连接并在线)，并且未尝试局域网连接的，切换为局域网连接
	log_info("%s status=%u, online=%d, devserver_ip=%u.%u.%u.%u, last_err=%d,peer_ip %u.%u.%u.%u\n",
		user->name, user->status, user->online, IP_SHOW(user->devserver_ip), user->last_err,IP_SHOW(dest->peer_ip));
    
    if (!user->is_udp_ctrl) {
        user->is_udp_ctrl = !(!(dpi->flags & CFGPF_UDP_CTRL));
    }
    
    if (ntohl(dest->peer_ip) == user->direct_ip ) {
        return;
    }
    
//    if (user->is_udp_ctrl && (dest->peer_ip == user->direct_ip) && (user->status == CS_AUTH || user->status == CS_DEV_CONNECTING)) {
//        //已经在认证，不打断了
//        return;
//    }
	user_set_direct_login(user, ntohl(dest->peer_ip));
}

static void timer_quick_probe(lan_dev_probe_ctrl_t* ldpc)
{
    if (ldpc) {
        CL_THREAD_OFF(ldpc->t_probe);
        CL_THREAD_TIMER_ON(&cl_priv->master, ldpc->t_probe, timer_lan_dev_probe, NULL, 0);
    }
}

static void header_bytes_order(cfg_pkt_dhr_t* header)
{
    header->cmd = ntohs(header->cmd);
    header->dst_sn = ntoh_ll(header->dst_sn);
    header->src_sn = ntoh_ll(header->src_sn);
    header->flags = ntohs(header->flags);
    header->param_len = ntohs(header->param_len);
    header->req_id = ntohs(header->req_id);
}

static void do_i_here_ext_tlv(dev_probe_info_t* info,uc_tlv_t* tlv,int tlv_len)
{
	int remain = tlv_len,ext_type;

	log_debug("do i here tlv: sn %"PRIu64"\n", info->dev_sn);

	if(!info || !tlv || remain < sizeof(*tlv)){
		return;
	}
	
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
		remain -= (sizeof(uc_tlv_t) + tlv->len);

		log_debug("	tlv type %u\n", tlv->type);
		switch (tlv->type) {
		case UCT_SCM_DEV_TYPE:
			if(tlv->len > 0){
				ext_type = scm_get_ext_type_by_ident(info->sub_type,(char*)(tlv+1),tlv->len);
				if(ext_type >= 0){
					info->ext_type = ext_type;
				}
			}
			break;
		case UCT_DEVICE_NEW_DEV_EXT_TYPE:
			evm_set_type_by_tlv(&info->sub_type, &info->ext_type, tlv);
			
			break;
        case UCT_SCM_FACTORY_DATA:		
			if(tlv->len > 0){
				ext_type = scm_get_ext_type_by_tlv(info->sub_type,info->ext_type,tlv);
				if(ext_type >= 0){
					info->ext_type = ext_type;
				}
			}
                break;
        case UCT_SM_CONFIG_TIME:
                if (tlv->len >= sizeof(u_int32_t)) {
                    info->sm_success_time = tlv_ntoh_u32_value(tlv);
//                    log_debug("dev %012"PRIu64" smart config time = %08X!\n",info->dev_sn,info->sm_success_time);
                }
                break;
		case UCT_DEVICE_EVM_IS_UPGRADING:
			if (tlv->len >= 1) {
				info->evm_is_upgrading = tlv_u8_value(tlv);
			}
			break;
		case UCT_DEV_NEW_LA_SUPPORT:
			info->la_info.is_la_new = 1;
			info->la_info.home_id = tlv_ntoh_u32_value(tlv);
			break;
		case UCT_DEVELOPER_ID:
			if (tlv->len >= 32) {
				memcpy(info->developer_id, tlv_val(tlv), 32);
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
	
}

static int recv_i_here(pkt_t *pkt, lan_dev_probe_ctrl_t* ldpc, struct sockaddr_in* peer_addr)
{
    cfg_pkt_i_here_t* here_res;
    dev_probe_info_t dpi = {0};
    cfg_pkt_dhr_t *header = (cfg_pkt_dhr_t*)pkt->data;

    if (header->param_len < sizeof(*here_res)) {
        return 0;
    }
    
    here_res = (cfg_pkt_i_here_t*)(header+1);
    dpi.dev_sn = header->src_sn;
    dpi.recv_time = get_sec();
    dpi.sub_type = here_res->sub_type;
    dpi.type = here_res->type;
    dpi.dev_run_mode = ntohs(here_res->sys_mode);
    dpi.peer_ip = peer_addr->sin_addr.s_addr;
    memcpy(dpi.auth, here_res->auth, sizeof(dpi.auth));
    dpi.flags = header->flags;
    dpi.ext_type = here_res->ext_type;
    dpi.real_ext_type = here_res->ext_type;

	dpi.la_info.is_valid = true;
	dpi.la_info.back_is_la_new = dpi.la_info.is_la_new;
	dpi.la_info.is_la_new = 0;

	log_debug("xxxxxxxxxxxxxx lan proc sn %"PRIu64"\n", dpi.dev_sn);
    if(header->param_len > sizeof(*here_res)){
		do_i_here_ext_tlv(&dpi,(uc_tlv_t*)(here_res+1),header->param_len-sizeof(*here_res));
    }

    ajust_dev_sub_type_by_sn(dpi.dev_sn,&dpi.sub_type,&dpi.ext_type);
    map_test_dev_sub_type(&dpi.sub_type,&dpi.ext_type);
    
    update_device_info(ldpc, &dpi);
   // dpi.la_info.back_is_la_new = dpi.la_info.is_la_new;
	
    return 0;
}

bool lan_user_is_home_added(user_t *puser)
{
	bool ret = false;
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	dev_probe_info_t *dev = NULL;

	if (!puser) {
		return ret;
	}
	
	cl_lock(&ldpc->mutex);
	dev = find_dpi_by_sn(ldpc, puser->sn);
	if (dev == NULL) {
		//非局域网可以直接返回false；
		goto done;
	}

	if (!dev->la_info.is_valid) {
		goto done;
	}

	if (dev->la_info.home_id) {
		ret = true;
	}	

done:	
	cl_unlock(&ldpc->mutex);
	return ret;
}

bool lan_user_is_la_support(user_t *puser)
{
	bool ret = false;
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	dev_probe_info_t *dev = NULL;

	if (!puser) {
		return ret;
	}
	cl_lock(&ldpc->mutex);
	dev = find_dpi_by_sn(ldpc, puser->sn);
	if (dev == NULL) {
		//非局域网可以直接返回false；
		goto done;
	}
	if (!dev->la_info.is_valid) {
		goto done;
	}
	if (dev->la_info.is_la_new) {
		ret = true;
	}	

done:	
	cl_unlock(&ldpc->mutex);
	return ret;
}


static int recv_result(pkt_t *pkt, lan_dev_probe_ctrl_t* ldpc, struct sockaddr_in* peer_addr)
{
	cfg_pkt_result_t *rt;
	dev_probe_info_t* dev;
	cfg_pkt_dhr_t *header = (cfg_pkt_dhr_t*)pkt->data;
	int event = 0;

	if (header->dst_sn != ldpc->random_sn)
		return 0;
	if(header->param_len < sizeof(*rt)){
		return 0;
	}
	dev = find_dpi_by_sn(ldpc, header->src_sn);
	if(dev == NULL)
		return 0;
	
	rt = (cfg_pkt_result_t*)(header+1);
	rt->err_code = ntohs(rt->err_code);
	rt->req_id = ntohs(rt->req_id);
	if(rt->req_id != dev->req_id)
		return 0;
	//收到正确响应，关闭重发定时器
	CL_THREAD_TIMER_OFF(dev->t_timer);
	
	if (dev->probe_status == PS_AUTHING) {
		if (rt->err_code == 0) {
			event = LDPE_DEV_AUTH_OK;
			dev->probe_status = PS_AUTHED;
		} else {
			event = LDPE_DEV_AUTH_FAIL;
			dev->probe_status = PS_INIT;	
		}
	} else if (dev->probe_status == PS_CONFIGING) {
		if (rt->err_code == 0) {
			event = LDPE_WIFI_CFG_OK;
			dev->probe_status = PS_CONFIGED;
		} else {
			event = LDPE_WIFI_CFG_FAIL;
			dev->probe_status = PS_INIT;	
		}
	}

	if(event && ldpc->callback)
		event_push(ldpc->callback, event, dev->handle, ldpc->callback_handle);
	return 0;
}

static int lan_dev_hand_packet(pkt_t* packet,lan_dev_probe_ctrl_t* ldpc,cfg_pkt_dhr_t* header,
                               struct sockaddr_in* peer_addr)
{
	header_bytes_order(header);
	if (header->dst_sn != ldpc->random_sn)
		return 0;

	switch(header->cmd){
		case CFGPT_I_HERE:
			return recv_i_here(packet, ldpc, peer_addr);
			break;
			
		case CFGPT_RESULT:
			return recv_result(packet, ldpc, peer_addr);
			break;
			
		default:
			break;
	}
	return 0;
}

static int lan_dev_probe_read(cl_thread_t *t)
{
    lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    struct sockaddr_in addr;
    socklen_t len_addr;
    pkt_t* pkt ;
    cfg_pkt_dhr_t* cfg_hdr;
    int ret;
  
    if (!ldpc) {
        return 0;
    }
    
    ldpc->t_read = NULL;
    if (ldpc->udp_socket>0) {
         CL_THREAD_READ_ON(&cl_priv->master, ldpc->t_read, lan_dev_probe_read, NULL, ldpc->udp_socket);
    }else{
        return 0;
    }
    len_addr = sizeof(addr);
    pkt = ldpc->packet;
    pkt->total = (int)recvfrom(ldpc->udp_socket, pkt->data, MAX_UDP_PKT, 0, (struct sockaddr *)&addr, &len_addr);
    
    if (pkt->total < (int)sizeof(*cfg_hdr)) {
        return -1;
    }
    
#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(true, pkt->total);
#endif
    
    cfg_hdr = (cfg_pkt_dhr_t*)pkt->data;
    cl_lock(&ldpc->mutex);
    ret = lan_dev_hand_packet(pkt,ldpc,cfg_hdr,&addr);
    cl_unlock(&ldpc->mutex);
    return ret;
}

static u_int64_t rand_sn()
{
	int r;
	u_int64_t sn;

	srand(get_usec());
	r = rand();
	
	sn = 999900000000 + r%100000000;

	return sn;
}

static RS ldp_try_create_socket(lan_dev_probe_ctrl_t* ldpc)
{
	u_int16_t port = IJCFG_PORT_APP;
	int max_try_num = 50;
	 struct sockaddr_in addr;
    int permit_broadcast = 0x1;
	
	 if (IS_INVALID_SOCK(ldpc->udp_socket)) {
        ldpc->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

	while(bind(ldpc->udp_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0){
		port++;
		addr.sin_port = htons(port);
		max_try_num--;
		if(max_try_num<=0){
			log_err(true, "bind lan dev scan socket failed\n");
			  CLOSE_SOCK(ldpc->udp_socket);
            		return RS_ERROR;
		}
	}
	
        setsockopt(ldpc->udp_socket, SOL_SOCKET, SO_BROADCAST,(char*)&permit_broadcast,sizeof(int));
        SET_SOCK_NBLOCK(ldpc->udp_socket);
    }

	 return RS_OK;
}

static RS ldp_start_probe_device(lan_dev_probe_ctrl_t* ldpc)
{
   

    if (ldpc->is_probe_enable) {
        return RS_OK;
    }
    
  if( ldp_try_create_socket(ldpc)!=RS_OK)
  	return RS_ERROR;
    
    ldpc->random_sn  = rand_sn();
    ldpc->is_probe_enable = true;
    ldpc->probe_times = 0;
    CL_THREAD_READ_ON(&cl_priv->master, ldpc->t_read, lan_dev_probe_read, NULL, ldpc->udp_socket);
    timer_quick_probe(ldpc);
    
    return RS_OK;
}

static RS ldp_set_callback(cl_notify_pkt_t *pkt)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	cln_lan_dev_probe_t *cln_ldp = (cln_lan_dev_probe_t*)pkt->data;

	if(pkt->param_len < sizeof(cln_lan_dev_probe_t))
		return RS_INVALID_PARAM;
	
	ldpc->callback = cln_ldp->callback;
	ldpc->callback_handle = cln_ldp->handle;
    ucla_set_callback(cln_ldp->callback, cln_ldp->handle);
	//设置回调后再快速扫描一次
	timer_quick_probe(ldpc);
    
	return RS_OK;
}

static RS lan_dev_auth(cl_notify_pkt_t *pkt)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	cln_lan_dev_auth_t *up;
	dev_probe_info_t *dev;
	RS ret = RS_OK;

	if(pkt->param_len < sizeof(*up))
		return RS_INVALID_PARAM;
	up = (cln_lan_dev_auth_t*)pkt->data;
	
	cl_lock(&ldpc->mutex);
	
	dev = find_dpi_by_sn(ldpc, up->dev_sn);
	if (dev == NULL) {
		ret = RS_NOT_FOUND;
		goto done;
	}
	memcpy(dev->md5_pwd, up->md5_pwd, 16);
	dev->req_id++;
	dev->retry = 0;
	dev->probe_status = PS_AUTHING;
	CL_THREAD_TIMER_OFF(dev->t_timer);
	CL_THREAD_TIMER_ON(&cl_priv->master, dev->t_timer, timer_lan_dev_auth, dev, 1);
	
done:	
	cl_unlock(&ldpc->mutex);
	return ret;
}

static RS lan_wifi_config(cl_notify_pkt_t *pkt)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	cln_lan_wifi_cfg_t *up;
	dev_probe_info_t *dev;
	char *ssid, *pwd;
	RS ret = RS_OK;

	if(pkt->param_len < sizeof(*up))
		return RS_INVALID_PARAM;
	up = (cln_lan_wifi_cfg_t*)pkt->data;
	if(pkt->param_len < (sizeof(*up) + up->len_ssid + up->len_pwd))
		return RS_INVALID_PARAM;
	ssid = up->data;
	pwd = &up->data[up->len_ssid];
	
	cl_lock(&ldpc->mutex);
	dev = find_dpi_by_sn(ldpc, up->dev_sn);
	if (dev == NULL) {
		ret = RS_NOT_FOUND;
		goto done;
	}
	if (dev->probe_status != PS_AUTHED) {
		ret = RS_NOT_LOGIN;
		goto done;
	}
	
	STR_REPLACE(dev->ssid, ssid);
	STR_REPLACE(dev->wifi_pwd, pwd);
	dev->req_id++;
	dev->retry = 0;
	dev->probe_status = PS_CONFIGING;
	CL_THREAD_OFF(dev->t_timer);
	CL_THREAD_TIMER_ON(&cl_priv->master, dev->t_timer, timer_lan_wifi_config, dev, 1);
	
	
done:	
	cl_unlock(&ldpc->mutex);
	return ret;
}

bool lan_dev_probe_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;

	if (!ldpc) {
		*ret = RS_INVALID_PARAM;
		return true;
	}
	switch (pkt->type) {
	case CLNE_LDP_SET_CALLBACK:
		*ret = ldp_set_callback(pkt);
		break;

	case CLNE_LAN_AUTH:
		*ret = lan_dev_auth(pkt);
		break;
		
	case CLNE_LAN_WIFI_CFG:
		*ret = lan_wifi_config(pkt);
		break;

	case CLNE_START_SMART_CONFIG_EXT:
		*ret = smart_config_start_ext(pkt);
		break;
	case CLNE_START_SMART_CONFIG:
		*ret = smart_config_start(pkt);
		break;
    case CLNE_START_6621_SMART_CONFIG:
        *ret = smart_6621_config_start(pkt);
        break;
	case CLNE_START_MBROADCAST_SMART_CONFIG:
		*ret = smart_config_mbroadcast_start(pkt);
		break;
	case CLNE_START_MBROADCAST_SMART_CONFIG_SLOWLY:
		*ret = smart_config_mbroadcast_start_hotspot(pkt);
		break;
	case CLNE_START_PHONE_WLAN_CONFIG:
		*ret = smart_phone_wlan_config_start(pkt);
		break;
	case CLNE_NETWORK_CHANGED:
		lan_dev_proc_wifi_switch(cl_priv->net_type);
		//其他地方也要处理这个报文
		return false;
		break;		
	case CLNE_STOP_SMART_CONFIG:
		*ret = smart_config_stop();
		break;
	case CLNE_START_ADVANCE_CONF:
		*ret = smart_config_start_advance(pkt);
		break; 
	default:
		return false;
		break;
	}

	return true;
}

u_int32_t get_dev_lan_ipaddr(u_int64_t sn)
{
    lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    dev_probe_info_t* dest;
    
    if (!ldpc) {
        return 0;
    }
    
    dest = find_dpi_by_sn(ldpc,sn);
    if (dest) {
        return dest->peer_ip;
    }
    return 0;
}

bool get_dev_lan_sub_type(u_int64_t sn, u_int8_t *psub_type, u_int8_t *pext_type)
{
	bool ret = false;
    lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    dev_probe_info_t* dest;
    
    if (!ldpc) {
        return ret;
    }
    
    dest = find_dpi_by_sn(ldpc,sn);
    if (dest) {
        *psub_type = dest->sub_type;
        *pext_type = dest->ext_type;
		ret = true;
    }


	return ret;
}

bool lan_dev_is_need_udp_login(u_int64_t sn)
{
    lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    dev_probe_info_t* dest;
    
    if (!ldpc) {
        return false;
    }
    
    dest = find_dpi_by_sn(ldpc,sn);
    if (dest) {
        return !(!(dest->flags & CFGPF_UDP_CTRL));
    }
    return false;
}

void lan_phone_send_config_ok_to_app(u_int64_t wan_devsn)
{
	 lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;

    if (ldpc != NULL) {
	  ldpc->ap_dest_sn = ntoh_ll(wan_devsn);
        event_push(ldpc->callback, LDPE_PHONE_CONFIG_DEV_OK, 0, ldpc->callback_handle);
    }
	
}

void lan_dev_proc_wifi_switch(int new_net_type)
{
	dev_probe_info_t* dpi,*next;
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
	//网络切换了，加快局域网
	if(new_net_type == NET_TYPE_NONE){
		 stlc_list_for_each_entry_safe(dev_probe_info_t, dpi, next,&ldpc->dev_info_list, link){
	         ldp_reset_time_out(dpi, 3);
	    }
	}
}

void delete_lan_dev_by_sn(u_int64_t sn)
{
	 lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;
    dev_probe_info_t* dest;
    
    if (!ldpc) {
        return ;
    }
    
    dest = find_dpi_by_sn(ldpc,sn);
    if (dest) {
         ldp_del_and_free_dev_info(dest);
    }

}

bool lan_dev_probe_init(cl_priv_t* cp)
{
    lan_dev_probe_ctrl_t* ldpc;
    
    if (cp->ldpc ) {
        return true;
    }
    
    ldpc = cl_calloc(sizeof(*ldpc), 1);
    if (!ldpc) {
        return false;
    }
    
    if (!ldpc->packet) {
        ldpc->packet = cl_calloc(MAX_UDP_PKT, 1);
        if (!ldpc->packet) {
            cl_free(ldpc);
            return false;
        }
    }
    
    cl_init_lock(&ldpc->mutex);
    STLC_INIT_LIST_HEAD(&ldpc->dev_info_list);
	STLC_INIT_LIST_HEAD(&ldpc->bcast_lst);
    cp->ldpc = ldpc;
    
    ldp_start_probe_device(ldpc);
    
    return true;
}

void lan_dev_probe_exit(cl_priv_t* cp)
{
    lan_dev_probe_ctrl_t* ldpc = cp->ldpc;
    
    cp->ldpc = NULL;
    if (!ldpc) {
        return;
    }
    ldp_stop_probe_device(ldpc);
    ldp_clear_all_devices(ldpc);
	bcast_list_free(ldpc);
    cl_destroy_lock(&ldpc->mutex);
    pkt_free(ldpc->packet);
    
    cl_free(ldpc);
}

