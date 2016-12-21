/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: 网络调试
**  File:    cl_debug_agent.c
**  Author:  liubenlong
**  Date:    10/14/2014
**
**  Purpose:
**    网络调试.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_debug_agent.h"
#include "cl_sys.h"
/* Macro constant definitions. */
#if 0
#define	log_debug(...) _log_debug(__VA_ARGS__)
extern void _log_debug(const char *fmt, ...);

#define	log_info( ...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
extern void _log_info(const char *file, int line, const char *fmt, ...);

#define	log_err(print_err, ...) _log_err(__FILE__, __LINE__, print_err, __VA_ARGS__)
extern void _log_err(const char *file, int line, bool print_err, const char *fmt, ...);

#define	cl_assert(b) _cl_assert(!(!(b)), __FILE__, __LINE__)
extern void _cl_assert(bool b, const char *file, int line);

#define mem_dump(pre,dest,len) memdump(pre,dest,len);
extern void memdump(char* pre,void* dest, u_int32_t len);
#endif

#define ENTER()		log_debug("enter %s line=%d\n", __FUNCTION__, __LINE__)
#define EXIT()		log_debug("exit %s\n", __FUNCTION__)

extern void sa_force_refresh_uptime(user_t* user);
extern void sa_systime_set(user_t* user, struct tm *ptm);
extern void cl_net_debug(ucc_session_t *s, bool debug_on);

/* Type definitions. */

//会话数据结构
typedef struct cl_debug_server_s{
	bool init;
	int tcp_sock;
	int tcp_port;
	//session 锁定
	void *session;
	//客户端链表
	struct stlc_list_head client_list;
	cl_thread_t *t_accept;
	//检查客户端定时器
	cl_thread_t *t_timer;	
}cl_debug_server_t;

typedef struct cl_debug_client_s{
	//客户端链表
	struct stlc_list_head link;
	//客户端发送数据链表
	struct stlc_list_head send_list;
	//客户端发送时间
	u_int32_t send_time;
	//发送字节数
	int snd_len;
	//客户端sock
	int sock;
	//客户端读缓存
	char buff[1024];
	//接收长度
	int rcv_len;
	//客户端读
	cl_thread_t *t_read;
	//客户端写
	cl_thread_t *t_write;
}cl_debug_client_t;

typedef struct debug_pkt_s{
	struct stlc_list_head link;
	int total;
	char buff[0];
}debug_pkt_t;


/* Local function declarations. */


/* Macro API definitions. */
#define CLIENT_SIZE		sizeof(cl_debug_client_t)
#define DEBUG_PKT_SIZE	sizeof(debug_pkt_t)

//5分钟超时
#define DEBUG_CLIENT_SESSION_TIMEOUT		(5*60)

/* Global variable declarations. */
cl_debug_server_t debug_server;

//colour array
char *colour[DEBUG_ALL] = {
	"",
	"\033[31m\033[1m",
	"\033[33m\033[1m",
	"\033[37m\033[1m"
};

int grep_local[DEBUG_ALL] = {1, 1, 1, 1};

static debug_pkt_t *cl_debug_pkt_new(char *buff, u_int32_t len)
{
	debug_pkt_t *pkt = NULL;
	
	if (!buff) {
		return NULL;
	}

	//加两个字符，\r\n
	pkt = (debug_pkt_t *)cl_calloc(len + 2 + DEBUG_PKT_SIZE, 1);
	if (!pkt) {
		log_err(true, "calloc failed\n");
		return NULL;
	}
	pkt->total = len + 2;
	STLC_INIT_LIST_HEAD(&pkt->link);
	memcpy(pkt->buff, buff, len);
	pkt->buff[len] = '\r';
	pkt->buff[len+1] = '\n';	

	return pkt;
}

	
static void cl_debug_client_free(cl_debug_client_t *pclient)
{
	debug_pkt_t *pkt, *next;
	
	if (!pclient) {
		return;
	}

	CL_THREAD_OFF(pclient->t_read);
	CL_THREAD_OFF(pclient->t_write);
	if (pclient->sock > 0) {
		CLOSE_SOCK(pclient->sock);
	}
	
	stlc_list_for_each_entry_safe(debug_pkt_t, pkt, next, &pclient->send_list, link) {
		stlc_list_del(&pkt->link);
		cl_free(pkt);
	}	

	cl_free(pclient);
}


static void cl_debug_client_close(cl_debug_client_t * pclient)
{
	if (!pclient) {
		return;
	}
	
	stlc_list_del(&pclient->link);
	cl_debug_client_free(pclient);
}

static int cl_debug_write(cl_thread_t *t)
{
	int n, want;
	debug_pkt_t *pkt = NULL;
	cl_debug_client_t * pclient = (cl_debug_client_t *)CL_THREAD_ARG(t);

	pclient->t_write = NULL;
	if (stlc_list_empty(&pclient->send_list)) {
		log_err(false, "Big bug: %s but send list is empty\n", __FUNCTION__);
		return 0;
	}

	pkt = (debug_pkt_t *)stlc_list_entry(pclient->send_list.next, debug_pkt_t, link);
	want = pkt->total - pclient->snd_len;
	cl_assert(want > 0);
	n = send(pclient->sock, &pkt->buff[pclient->snd_len], want, 0);

	if (n < 0) {
		if (NORMAL_ERRNO(errno)) {
			CL_THREAD_WRITE_ON(MASTER, pclient->t_write,cl_debug_write, (void *)pclient, pclient->sock);
			return 0;
		}
		log_err(true, "write socket error");
		goto err;
	}	
	log_debug("write n=%d\n", n);
	pclient->snd_len += n;
	pclient->send_time = (u_int32_t)time(NULL);
	if (n == want) {
		pclient->snd_len = 0;
		stlc_list_del(&pkt->link);
		cl_free(pkt);
		if (stlc_list_empty(&pclient->send_list)) {
			return 0;
		}		
	}	

	CL_THREAD_WRITE_ON(MASTER, pclient->t_write,cl_debug_write, (void *)pclient, pclient->sock);
	
	return 0;
	
err:
	cl_debug_client_close(pclient);
	
	return -1;
}

static void cl_debug_add_pkt(cl_debug_client_t *pclient, debug_pkt_t *pkt)
{
	bool need_send = false;
	
	if (!pclient || !pkt) {
		return;
	}

	if (stlc_list_empty(&pclient->send_list)) {
		need_send = true;
	}

	stlc_list_add_tail(&pkt->link, &pclient->send_list);

	if (need_send) {
		CL_THREAD_WRITE_ON(MASTER, pclient->t_write,cl_debug_write, (void *)pclient, pclient->sock);
	}
}

void cl_debug_buff_add(void *s, char *buf, int len)
{
	int i, j;
	int big_len = 0;	
	u_int32_t now = 0;	
	char big_buff[1024];
	char big_buff2[1024];	
	debug_pkt_t *pkt = NULL;	
	cl_debug_client_t *pclient, *next;
	cl_debug_server_t *pserver = &debug_server;

	//session 锁定
	if (!s || s != pserver->session) {
		return;
	}

	now = (u_int32_t)time(NULL);
	if (!buf || len <= 0) {
		return;
	}
	//debug_info("%s\n", buf);
	//加上颜色及过滤	
	if (buf[0] >= DEBUG_ERROR && buf[0] <= DEBUG_DEBUG) {
		if (grep_local[buf[0]] == 0) {
			return;
		}
		
		big_len = sprintf(big_buff , "%s", colour[buf[0]]);
		buf++;
		len--;
	}

	memcpy(big_buff + big_len, buf, len);
	big_len += len;

	//替换下\n=>\r\n
	for(i = 0, j = 0; i < sizeof(big_buff) && j < sizeof(big_buff2); i++, j++) {
		if (big_buff[i] == '\n') {
			big_buff2[j++] = '\r';
		}
		big_buff2[j] = big_buff[i];
	}
	
	
	stlc_list_for_each_entry_safe(cl_debug_client_t, pclient, next, &pserver->client_list, link) {
		pkt = cl_debug_pkt_new(big_buff2, big_len);
		if (pkt) {
			cl_debug_add_pkt(pclient, pkt);
		}		
	}	
	//超时检查
	stlc_list_for_each_entry_safe(cl_debug_client_t, pclient, next, &pserver->client_list, link) {
		if (pclient->send_time + DEBUG_CLIENT_SESSION_TIMEOUT < now) {
			log_debug("some one timeout now\n");
			stlc_list_del(&pclient->link);
			cl_debug_client_free(pclient);
		}	
	}		
}

static void cl_debug_agent_reset(cl_debug_server_t *pserver)
{
	cl_debug_client_t *pclient, *next;

	if (!pserver) {
		return;
	}

	CL_THREAD_OFF(pserver->t_accept);
	CL_THREAD_OFF(pserver->t_timer);

	CLOSE_SOCK(pserver->tcp_sock);
	
	stlc_list_for_each_entry_safe(cl_debug_client_t, pclient, next, &pserver->client_list, link) {
		stlc_list_del(&pclient->link);
		cl_debug_client_free(pclient);
	}	
}

static void client_init(cl_debug_client_t * pclient, int sock)
{
	if (!pclient) {
		return;
	}

	pclient->sock = sock;
	pclient->snd_len = 0;
	pclient->rcv_len = 0;	
	pclient->send_time = 0;
	pclient->t_read = NULL;
	pclient->t_write = NULL;
	STLC_INIT_LIST_HEAD(&pclient->link);		
	STLC_INIT_LIST_HEAD(&pclient->send_list);	
}

static void cl_debug_client_recv_clear(cl_debug_client_t * pclient)
{
	if (!pclient) {
		return;
	}

	pclient->rcv_len = 0;
	memset(pclient->buff, 0, sizeof(pclient->buff));	
}

static void cl_debug_client_recv_proc(cl_debug_client_t * pclient)
{
	if (!pclient) {
		return;
	}

	if (strncmp(pclient->buff, "exit", 4) == 0) {
		log_debug("someone exit \n");
		stlc_list_del(&pclient->link);
		cl_debug_client_free(pclient);		
	} else {
		cl_debug_client_recv_clear(pclient);
	}
}


static int debug_client_read(cl_thread_t *t)
{
	int n;
	char value;
	cl_debug_client_t * pclient = (cl_debug_client_t *)CL_THREAD_ARG(t);

	pclient->t_read = NULL;	
	CL_THREAD_READ_ON(MASTER, pclient->t_read, debug_client_read, (void *)pclient, pclient->sock);		

	value = 0;
	n = recv(pclient->sock, &value, 1, 0);
	if (n > 0) {
		if (pclient->rcv_len + 2 < sizeof(pclient->buff)) {
			pclient->buff[pclient->rcv_len] = value;
			pclient->rcv_len++;
			if (value == '\r' || value == '\n') {
				pclient->buff[pclient->rcv_len] = 0;
				log_debug("recv:%s\n",pclient->buff);
				cl_debug_client_recv_proc(pclient);
			}
		} else {
			cl_debug_client_recv_clear(pclient);
		}
	}

	//对端关闭
	if (n == 0) {
		log_debug("one client closed now\n");
		stlc_list_del(&pclient->link);
		cl_debug_client_free(pclient);
	}

	return 0;
}

static int debug_agent_accept(cl_thread_t *t)
{
	int sock;
	int ip, port, addrlen;
	debug_pkt_t *pkt = NULL;
	struct sockaddr_in	addr;	
	cl_debug_client_t * pclient = NULL;	
	cl_debug_server_t *pserver = &debug_server;
	char *welcome = "welcome to debug!!!!!!\r\n";

	pserver->t_accept = NULL;
	CL_THREAD_READ_ON(MASTER, pserver->t_accept, debug_agent_accept, NULL, pserver->tcp_sock);

	addrlen = sizeof(addr);
	sock = (int)accept(pserver->tcp_sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
	if (sock < 0) {
		log_err(true, "accept bind socket failed");
		return 0;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	log_debug("Accept client from %u.%u.%u.%u port %u sock=%d\n", IP_SHOW(ip), port, sock);
	
	pclient = cl_calloc(CLIENT_SIZE, 1);
	if(NULL == pclient){
		log_err(true, "calloc failed\n");
		return -1;
	}
	client_init(pclient,sock);
	stlc_list_add_tail(&pclient->link, &pserver->client_list);
	//test
	pkt = cl_debug_pkt_new(welcome, (u_int32_t)strlen(welcome));
	if (pkt) {
		cl_debug_add_pkt(pclient, pkt);
	}
	
	CL_THREAD_READ_ON(MASTER, pclient->t_read, debug_client_read, (void *)pclient, pclient->sock);	

	return 0;
}



static void cl_debug_timer_init(cl_debug_server_t *pserver)
{
	//accept
	pserver->t_accept = NULL;
	CL_THREAD_READ_ON(MASTER, pserver->t_accept, debug_agent_accept, NULL, pserver->tcp_sock);

}

static RS cl_debug_sock_create(cl_debug_server_t *pserver, int port)
{
	if (!pserver) {
		return RS_ERROR;
	}

	
	pserver->tcp_sock = (int)create_tcp_server(INADDR_ANY, port);
	
	if (pserver->tcp_sock > 0) {
		SET_SOCK_NBLOCK(pserver->tcp_sock);
		pserver->tcp_port = port;
		log_debug("tcp bind port=%d\n", pserver->tcp_port);
		return RS_OK;
	}

	return RS_ERROR;
}

static void cl_debug_server_clear(cl_debug_server_t *pserver)
{
	if (!pserver) {
		return;
	}
	
	if (pserver->init) {
		cl_debug_agent_reset(pserver);
	}
	
	memset((void *)pserver, 0, sizeof(*pserver));
	//初始化链表
	STLC_INIT_LIST_HEAD(&pserver->client_list);
}

static void cl_debug_session_lock(cl_debug_server_t *pserver, cl_handle_t dev_handle)
{
	user_t *user = NULL;
	ucc_session_t *s;
	
	if (!pserver) {
		return ;
	}
	
	if (IS_SAME_HANDLE_TYPE(dev_handle, HDLT_USER)) {
		user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);	
		pserver->session = user->uc_session;
		s = user->uc_session;
		s->debug_on = true;
		cl_net_debug(s, true);
		log_debug("sn=%llu debug on\n", s->user->sn);
	} else {
		pserver->session = NULL;
	}
}

CLIB_API RS cl_debug_agent_init(cl_handle_t dev_handle, int port)
{
	RS ret = RS_OK;
	cl_debug_server_t *pserver = &debug_server;

	if (pserver->init && pserver->tcp_port == port) {
		//session锁定一下.
		cl_debug_session_lock(pserver, dev_handle);		
		return RS_OK;
	}
	//重置一下.
	cl_debug_server_clear(pserver);
	//sock重创建一下.
	ret = cl_debug_sock_create(pserver, port);
	if (ret != RS_OK) {
		return ret;
	}
	//session锁定一下.
	cl_debug_session_lock(pserver, dev_handle);
	//初始化功能定时器
	cl_debug_timer_init(pserver);	
	pserver->init = true;
	
	return RS_OK;
}

#if 1
// 获取一次，下面立即触发一次查询
CLIB_API cl_debug_info_t *cl_get_debug_info_fuzhu(cl_handle_t dev_handle)
{
	user_t *user;
	smart_air_ctrl_t* ac = NULL;
	smart_appliance_ctrl_t* sac = NULL;
	cl_debug_info_t *cl_debug_info = NULL;


	cl_debug_info = cl_calloc(sizeof(cl_debug_info_t), 1);
	if (cl_debug_info == NULL) {
		return NULL;
	}

	cl_lock(&cl_priv->mutex);

	// 查找
	if (IS_SAME_HANDLE_TYPE(dev_handle, HDLT_USER)) {
		user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
		if (user == NULL || !user->is_udp_ctrl) {
			log_err(false, "cl_get_dev_stat_info not found user 0x%08x\n", dev_handle);
			goto done;
		}
		sac = (smart_appliance_ctrl_t*)user->smart_appliance_ctrl;
		if (sac && sac->sub_ctrl) {
    		ac = sac->sub_ctrl;
	    }
		if (ac) {
			//serverip
			cl_debug_info->ip = ac->stat.server_ip;
			//version
			cl_debug_info->soft_version = ac->stat.soft_version;
			//ssid
			if (ac->stat.ap_ssid) {
				strncpy(cl_debug_info->ssid, ac->stat.ap_ssid, sizeof(cl_debug_info->ssid));
				cl_debug_info->ssid[sizeof(cl_debug_info->ssid)-1] = '\0';				
			}
			//passwd
			if (ac->stat.ap_passwd) {
				strncpy(cl_debug_info->passwd, ac->stat.ap_passwd, sizeof(cl_debug_info->passwd));
				cl_debug_info->passwd[sizeof(cl_debug_info->passwd)-1] = '\0';			
			}
			//server domain name
			strncpy(cl_debug_info->domainname, ac->stat.server_domainname, sizeof(cl_debug_info->domainname));
			cl_debug_info->domainname[sizeof(cl_debug_info->domainname)-1] = '\0';		
			//dev uptime
			cl_debug_info->dev_uptime = ac->stat.uptime;
			//server uptime
			cl_debug_info->server_uptime = ac->stat.online;
			//svn
			cl_debug_info->svn = ac->stat.svn_num;
			//dev status
			cl_debug_info->dev_status = (u_int32_t)ac->stat.dev_status;
			//compile date
			if (ac->stat.compile_date) {
				strncpy(cl_debug_info->com_data, ac->stat.compile_date, sizeof(cl_debug_info->com_data));
				cl_debug_info->com_data[sizeof(cl_debug_info->com_data)-1] = '\0';			
			}
			//compile time
			if (ac->stat.compile_time) {
				strncpy(cl_debug_info->com_time, ac->stat.compile_time, sizeof(cl_debug_info->com_time));
				cl_debug_info->com_time[sizeof(cl_debug_info->com_time)-1] = '\0';			
			}		
			//systime
			cl_debug_info->systime = ac->stat.systime;
			//cpu per
			cl_debug_info->cpuper = ac->stat.cpu;
			//mem per
			cl_debug_info->memper = ac->stat.mem;			
			//wan ip
			cl_debug_info->wan_ip = ac->stat.wan.ip;
			//new
			cl_debug_info->DevServerIp = ac->stat.DevServerIp;
			cl_debug_info->StmSoftVersion = ac->stat.StmSoftVersion;
			cl_debug_info->StmHardVersion = ac->stat.StmHardVersion;
			cl_debug_info->LightCur = ac->stat.LightCur;
			cl_debug_info->LightYes = ac->stat.LightYes;
			cl_debug_info->LightNext = ac->stat.LightNext;
			cl_debug_info->IrId = ac->stat.IrId;
			cl_debug_info->AcCur = ac->stat.AcCur;
			cl_debug_info->ACVal = ac->stat.ACVal;		

			//type
			cl_debug_info->ds_type = user->ds_type;
			cl_debug_info->dev_type = user->sub_type;
			cl_debug_info->ext_type = user->ext_type;
			//power
			cl_debug_info->power = ac->air_info.cur_power;
			memcpy((void *)&cl_debug_info->ad, (void *)&ac->air_info.ad, sizeof(cl_debug_info->ad));
			
			if (user->vendor_id) {
				strcpy(cl_debug_info->vendor, user->vendor_id);
			}
		}		
	}

done:
	cl_unlock(&cl_priv->mutex);

	
	return (void *)cl_debug_info;
}
#endif
CLIB_API void cl_get_debug_info_free(cl_debug_info_t *debug_info)
{
	cl_free(debug_info);
}

CLIB_API void cl_debug_show_flush(cl_handle_t dev_handle)
{
	user_t *user = NULL;

	cl_lock(&cl_priv->mutex);

	// 查找
	if (IS_SAME_HANDLE_TYPE(dev_handle, HDLT_USER)) {
		user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
		if (user == NULL || !user->is_udp_ctrl) {
			log_err(false, "cl_get_dev_stat_info not found user 0x%08x\n", dev_handle);
			goto done;
		}
		if (user->status == CS_ESTABLISH) {
			sa_force_refresh_uptime(user);
		}		
	}
	
done:
	cl_unlock(&cl_priv->mutex);
}

CLIB_API void cl_set_debug_level(int level, int value)
{
	if (level >= DEBUG_ERROR && level <= DEBUG_DEBUG) {
		grep_local[level] = value;
	}
}

CLIB_API void cl_set_debug_time(cl_handle_t dev_handle, struct tm *ptm)
{
	user_t *user = NULL;

	cl_lock(&cl_priv->mutex);

	// 查找
	if (IS_SAME_HANDLE_TYPE(dev_handle, HDLT_USER)) {
		user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
		if (user == NULL || !user->is_udp_ctrl) {
			log_err(false, "cl_get_dev_stat_info not found user 0x%08x\n", dev_handle);
			goto done;
		}
		if (user->status == CS_ESTABLISH) {
			sa_systime_set(user, ptm);
		}		
	}
	
done:
	cl_unlock(&cl_priv->mutex);	
}


