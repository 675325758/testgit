#include "client_lib.h"
#include "cl_priv.h"
#include "cl_dns.h"

#ifdef __GNUC__
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif
 
static char *disp_server = NULL;

#define	MAX_RESOLV_IP	1000


#define DNS_CACHE_FILE	"dns_cache.txt"


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


typedef struct {
	struct stlc_list_head link;
	u_int32_t timeout;
	// like: www.jiazhang008.com
	char *dn;
	int count;
	u_int32_t *ips;
} dns_cache_t;

// 本地缓存超时时间: 30分钟
#define	DNS_CACHE_TIMEOUT	(30*60)
struct stlc_list_head dns_cache;


static bool dns_resolv_ip_comm(char *ds);
static void add_cache_dns();
static void dns_cache_save();
static void dns_cache_init();

extern void do_la_dns_cache_clean();

static void free_dns_cache()
{
	dns_cache_t *node, *next;
	
	stlc_list_for_each_entry_safe(dns_cache_t, node, next, &dns_cache, link) {
		stlc_list_del(&node->link);
		SAFE_FREE(node->dn);
		SAFE_FREE(node->ips);
		cl_free(node);
	}
}

typedef struct {
	struct stlc_list_head link;
	u_int32_t timeout;
	char *dn;
	int count;
	ipc_sock_t *ipc;
} ipc_dns_cache_t;

struct stlc_list_head ipc_dns_cache;
static bool ipc_init = false;

static void ipc_dns_cache_init()
{
	cl_lock(&cl_priv->mutex);
	
	if (!ipc_init) {
		STLC_INIT_LIST_HEAD(&ipc_dns_cache);
	}

	ipc_init = true;

	cl_unlock(&cl_priv->mutex);
}

static void free_ipc_dns_cache()
{
	ipc_dns_cache_t *node, *next;
	
	stlc_list_for_each_entry_safe(ipc_dns_cache_t, node, next, &ipc_dns_cache, link) {
		stlc_list_del(&node->link);
		SAFE_FREE(node->dn);
		SAFE_FREE(node->ipc);
		cl_free(node);
	}
}

static ipc_dns_cache_t* look_up_ipc_dns_cache_by_name(char* dn)
{
    ipc_dns_cache_t *node, *next;
    
    if (!dn) {
        return NULL;
    }
    
    stlc_list_for_each_entry_safe(ipc_dns_cache_t, node, next, &ipc_dns_cache, link) {
        if (!strcmp(dn, node->dn)) {
            return node;
        }
    }
    
    return NULL;
}

static dns_cache_t* look_up_dns_cache_by_name(char* dn)
{
    dns_cache_t *node, *next;
    
    if (!dn) {
        return NULL;
    }
    
    stlc_list_for_each_entry_safe(dns_cache_t, node, next, &dns_cache, link) {
        if (!strcmp(dn, node->dn)) {
            return node;
        }
    }
    
    return NULL;
}

/**
	通过IP反找域名
*/
char *cl_lookup_dn_by_ip(u_int32_t ip)
{
	dns_cache_t *node, *next;
	int i;
    
    log_info("call cl_lookup_dn_by_ip\n");
    
    stlc_list_for_each_entry_safe(dns_cache_t, node, next, &dns_cache, link) {
        log_info("call cl_lookup_dn_by_ip dn %s count %d\n",node->dn,node->count);
		for (i = 0; i < node->count; i++) {
            log_info("dn %s ip %u.%u.%u.%u ,dest %u.%u.%u.%u\n",node->dn,IP_SHOW(node->ips[i]),IP_SHOW(ip));
			if (node->ips[i] == ip || node->ips[i] == htonl(ip)) {
				return node->dn;
			}
		}
    }
    
    return NULL;
}

u_int32_t cl_get_dns_timeout(char *doname)
{
	u_int32_t timeout = 0;
	ipc_dns_cache_t *pd = NULL;
	
	cl_lock(&cl_priv->mutex);
	pd = look_up_ipc_dns_cache_by_name(doname);
	if (!pd) {
		goto done;
	}

	timeout = pd->timeout;

done:
	cl_unlock(&cl_priv->mutex);

	return timeout;
}

void cl_set_dns_cache(char *doname, ipc_sock_t *ipc, int count)
{
	int len = 0;
	ipc_dns_cache_t *node = NULL;

	if (!ipc || !doname) {
		return;
	}

	//以防线程启动先后问题
	ipc_dns_cache_init();
	cl_lock(&cl_priv->mutex);
	node = look_up_ipc_dns_cache_by_name(doname);
	if (node) {
		goto done;
	}

	node = cl_calloc(sizeof(ipc_dns_cache_t), 1);
	if (!node) {
		goto done;
	}
	log_debug("ipc_dns_cache.next=%p ipc_dns_cache.prev=%p &ipc_dns_cache=%p doname=%s\n", 
		ipc_dns_cache.next, ipc_dns_cache.prev, &ipc_dns_cache, doname);
	stlc_list_add_tail(&node->link, &ipc_dns_cache);
	node->dn = cl_strdup(doname);

	node->count = count;
	len = sizeof(ipc_sock_t)*node->count;
	node->ipc = cl_calloc(len, 1);

	if (node->ipc) {
		memcpy((void *)node->ipc, (void *)ipc, len);
	}

done:
	cl_unlock(&cl_priv->mutex);
}

/*
	通过域名查找ip
*/
bool cl_get_ip_by_doname(char *doname, u_int32_t *ip, int *num)
{
	int i;
	bool ret = false;
	dns_cache_t *pd = NULL;

	if (!doname || !ip || !num) {
		return false;
	}

	cl_lock(&cl_priv->mutex);
	pd = look_up_dns_cache_by_name(doname);
	if (!pd) {
		goto done;
	}

	for(i = 0; i < pd->count; i++) {
		ip[i] = pd->ips[i];
	}
	
	*num = i;
	ret = true;
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

static void set_test_dns_cache(const char *dn, u_int32_t ip)
{
	dns_cache_t *node;
	node = cl_calloc(sizeof(dns_cache_t), 1);
	if (node == NULL)
		return;
	
	node->dn = cl_strdup(dn);
	node->count = 1;
	node->ips = cl_calloc(sizeof(u_int32_t), node->count);
	node->ips[0] = ip;
		
	log_info("ADD DNS: %s[0] = %u.%u.%u.%u\n", dn, IP_SHOW(node->ips[0]));
	
	node->timeout = get_sec() + range_rand(DNS_CACHE_TIMEOUT, DNS_CACHE_TIMEOUT*3/2);
	stlc_list_add(&node->link, &dns_cache);	
}

static dns_cache_t *update_dns_cache(const char *dn, struct hostent *ent)
{
	int i;
	dns_cache_t *node;
	u_int32_t *ipp;

	stlc_list_for_each_entry(dns_cache_t, node, &dns_cache, link) {
		if (strcmp(dn, node->dn) != 0)
			continue;
		
		// 找到，更新信息
		SAFE_FREE(node->ips);
		for (i = 0; ent->h_addr_list[i] != NULL && i < MAX_RESOLV_IP; i++) {
		}
		node->count = i;
		node->ips = cl_calloc(sizeof(u_int32_t), node->count);
		for (i = 0; i < node->count; i++) {
			ipp = (u_int32_t *)ent->h_addr_list[i];
			node->ips[i] = ntohl(*ipp);
			log_info("UPDATE DNS: %s[%d] = %u.%u.%u.%u\n", dn, i, IP_SHOW(node->ips[i]));
		}

		node->timeout = get_sec() + range_rand(DNS_CACHE_TIMEOUT/2, DNS_CACHE_TIMEOUT/2*3);
		return node;
	}

	// 没找到，是新节点
	node = cl_calloc(sizeof(dns_cache_t), 1);
	stlc_list_add(&node->link, &dns_cache);

	node->dn = cl_strdup(dn);
	
	for (i = 0; ent->h_addr_list[i] != NULL && i < MAX_RESOLV_IP; i++) {
	}
	node->count = i;
	node->ips = cl_calloc(sizeof(u_int32_t), node->count);
	for (i = 0; i < node->count; i++) {
		ipp = (u_int32_t *)ent->h_addr_list[i];
		node->ips[i] = ntohl(*ipp);
		log_info("ADD DNS: %s[%d] = %u.%u.%u.%u\n", dn, i, IP_SHOW(node->ips[i]));
	}

	node->timeout = get_sec() + range_rand(DNS_CACHE_TIMEOUT/2, DNS_CACHE_TIMEOUT/2*3);

	return node;
}

static dns_cache_t *resolv_dns(const char *dn)
{
	// 先找本地缓存
	dns_cache_t *node;
	struct hostent *ent;
	
	stlc_list_for_each_entry(dns_cache_t, node, &dns_cache, link) {
		// 找到
		if (strcmp(dn, node->dn) == 0 && node->timeout > get_sec()) {
			log_debug("DNS: %s use cache\n", dn);
			return node;
		}
	}

    ent = gethostbyname(dn);
	if (ent == NULL) {
		log_err(true, "gethostbyname failed for %s\n", dn);
		return NULL;
	}

	return update_dns_cache(dn, ent);
}

static bool dns_ip_look_up(u_int32_t *ip_list, u_int8_t num, u_int32_t ip)
{
	int i;

	for(i = 0; i < num; i++) {
		if (ip_list[i] == ip) {
			return true;
		}
	}

	return false;
}

static void add_cache_dns()
{
	int i;

	cl_lock(&cl_priv->mutex);

	//testip
	for(i = 0; i < cl_priv->test_ip_num; i++) {
		if ((cl_priv->num_ip_disp_server < MAX_DISP_SERVER_IP) &&
			!dns_ip_look_up(cl_priv->ip_disp_server, cl_priv->num_ip_disp_server, cl_priv->test_ip[i])) {
			cl_priv->ip_disp_server[cl_priv->num_ip_disp_server++] = cl_priv->test_ip[i];
		}
	}

	//cacheip
	for(i = 0; i < cl_priv->num_ip_disp_server_cache; i++) {
		if ((cl_priv->num_ip_disp_server < MAX_DISP_SERVER_IP) &&
			!dns_ip_look_up(cl_priv->ip_disp_server, cl_priv->num_ip_disp_server, cl_priv->ip_disp_server_cache[i])) {
			cl_priv->ip_disp_server[cl_priv->num_ip_disp_server++] = cl_priv->ip_disp_server_cache[i];
		}
	}
	
	cl_unlock(&cl_priv->mutex);
}

/*
	域名解析，主要解析分配服务器。
	如果是IP，就不用解析
	如果解析成功，不再解析，直到有新的解析请求
*/

static void build_disp_ips()
{
    dns_cache_t *node;
    int i,k;
    
    //先清0
    cl_priv->num_ip_disp_server = 0;
    memset(cl_priv->ip_disp_server, 0, sizeof(cl_priv->ip_disp_server));

    stlc_list_for_each_entry(dns_cache_t, node, &dns_cache, link) {
        for (i = 0; i < node->count; i++) {
            for (k = 0; k < MAX_DISP_SERVER_IP; k++) {
                if (node->ips[i] == cl_priv->ip_disp_server[k]) {
                    break;
                }
            }
            
            if (k >= MAX_DISP_SERVER_IP ) {
                cl_priv->ip_disp_server[cl_priv->num_ip_disp_server++] = node->ips[i];
            } 
        }
    }

	//先用解析的
	add_cache_dns();
}

static RS cl_resolve_disp_server(char *ds)
{
	int i, k;
	bool must_relog = true;
	dns_cache_t *node;

	if ((node = resolv_dns(ds)) == NULL) {
		return RS_ERROR;
	}

	//printf
	nd_login_debug(NULL, "cl_resolve_disp_server dn=%s count=%u\n", node->dn, node->count);
	for (i = 0; i < node->count; i++)  {
		nd_login_debug(NULL, "cl_resolve_disp_server ip=%u.%u.%u.%u\n", IP_SHOW(htonl(node->ips[i])));
	}
	
	cl_lock(&cl_priv->mutex);

	// 检查是否有相同的服务器，如果有，就不重新登录
	for (i = 0; i < node->count && must_relog; i++) {
		for (k = 0; k < (int)cl_priv->num_ip_disp_server; k++) {
			if (cl_priv->ip_disp_server[k] == node->ips[i]) {
				must_relog = false;
				break;
			}
		}
	}
#ifndef MUT_SERVER_ADAPT
	// 覆盖
	for (i = 0; i < node->count && i < MAX_DISP_SERVER_IP; i++) {
		cl_priv->ip_disp_server[i] = node->ips[i];
	}
	cl_priv->num_ip_disp_server = i;
#else
    //重新build一次
    build_disp_ips();
#endif
    
	
	cl_unlock(&cl_priv->mutex);

	// 通知主线程重新开始登陆
	if (must_relog)
		cl_send_notify_simple(&cl_priv->thread_main, CLNE_RESOLV_DISP_SERVER);

	// 缓存下dns need.....
	cl_priv->disp_need_save = true;

	return RS_OK;
}

RS resolv_custom(cl_notify_pkt_t *pkt)
{
	int i;
	dns_cache_t *node;

	pkt->param_len = 0;

	log_debug("resolve %s ...\n", pkt->data);
	if ((node = resolv_dns((char *)pkt->data)) == NULL) {
		return RS_ERROR;
	}
	
	for (i = 0; i < node->count; i++) {
		*((u_int32_t *)(&pkt->data[i*4])) = node->ips[i];
	}
	pkt->param_len = i*4;

	return RS_OK;
}

RS resolv_oem_doname(cl_notify_pkt_t *pkt)
{
	int i;
	char (*doname)[64] = NULL;

	if(!pkt) {
		return RS_ERROR;
	}

	doname = (char (*)[64])pkt->data;
	for (i = 0; i < (int)(pkt->param_len/sizeof(*doname)); i++, doname++) {
		((char *)doname)[63] = 0;
		//log_debug("resolv_oem_doname!!!!!!!!!!!!!!!!!!!!!!!!!!!!doname=%s\n", (char *)doname);
		cl_resolve_disp_server((char *)doname);
		dns_resolv_ip_comm((char *)doname);
	}

	return RS_OK;
}

static void do_dns_cache_clean()
{
	char path[512];

	log_info("enter");
	sprintf(path, "%s/%s", cl_priv->priv_dir, DNS_CACHE_FILE);
	remove(path);

	cl_priv->num_ip_disp_server_cache = 0;

	//这里还是清除下联动部分的缓存文件吧
	do_la_dns_cache_clean();
}

static RS dns_resolv_proc_notify(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cl_thread_info_t *ti = &cl_priv->thread_dns_resolv;
#ifdef MUT_SERVER_ADAPT
    int i;
    clib_dns_server_info_t* si;
#endif
	
	log_debug("DNS-resolve proc notfiy: type=%d, request id=%u, parame len=%d\n",
		pkt->type, pkt->request_id, pkt->param_len);
	
	switch (pkt->type) {
	case CLNE_STOP:
		ti->stopping = true;
		break;

	case CLNE_RESOLV_DISP_SERVER:
#ifndef MUT_SERVER_ADAPT
		cl_lock(&cl_priv->mutex);
		if (cl_priv->disp_server != NULL) {
			cl_priv->num_ip_disp_server = 0;
			disp_server = cl_strdup(cl_priv->disp_server);
		}
		cl_unlock(&cl_priv->mutex);
#endif
		break;
	case CLNE_CLEAR_DNS_CACHE:
		cl_lock(&cl_priv->mutex);
		if (cl_priv->disp_server != NULL) {
			cl_priv->num_ip_disp_server = 0;
			disp_server = cl_strdup(cl_priv->disp_server);
		}
#ifdef MUT_SERVER_ADAPT
        if (cl_priv && cl_priv->clib_dns_servers != NULL && cl_priv->dns_count != 0){
            si = cl_priv->clib_dns_servers;
            for (i = 0; i < cl_priv->dns_count; i++,si++) {
                si->failed_cnt = 0;
            }
        }
#endif
            
		cl_unlock(&cl_priv->mutex);

		if(disp_server != NULL){
			free_dns_cache();
		}
		
		break;

	case CLNE_RESOLV_CUSTOM:
		resolv_custom(pkt);
		break;
	case CLNE_RESOLV_OEM_DONAME:
		resolv_oem_doname(pkt);
		break;
	case CLNE_DNS_CLEAN:
		do_dns_cache_clean();
		break;
	case CLNE_WAKEUP:
	default:
		break;
	}

	return ret;
}


static ipc_dns_cache_t *update_ipc_dns_cache(const char *dn, struct addrinfo *res)
{
	int i;
	ipc_dns_cache_t *node = NULL, *n;
	struct addrinfo *a = NULL;
	ipc_sock_t *psock = NULL;

	if (!res) {
		return NULL;
	}
	log_debug("enter dn=%s %s %d\n", dn, __FUNCTION__, __LINE__);	
	stlc_list_for_each_entry_safe(ipc_dns_cache_t, node, n, &ipc_dns_cache, link) {
		if (strcmp(dn, node->dn) != 0) {
			continue;
		}
		SAFE_FREE(node->ipc);
		for (i = 0, a = res; a != NULL && i < MAX_RESOLV_IP; i++, a = a->ai_next) {
		}
		node->count = i;
		node->ipc = cl_calloc(sizeof(ipc_sock_t), node->count);
		for (i = 0, a = res; i < node->count && a != NULL && a->ai_addr; i++, a = a->ai_next) {
			psock = &node->ipc[i];
			psock->ai_family = a->ai_family;
			psock->ai_flags = a->ai_flags;
			psock->ai_socktype = a->ai_socktype;
			psock->ai_protocol = a->ai_protocol;
			psock->ai_addrlen = a->ai_addrlen;
			//psock->addr = *a->ai_addr;
			memcpy((void *)psock->addr.sockaddrc, a->ai_addr, a->ai_addrlen);
			log_debug("addrinfo dn=%s ai_family=%u AF_INET=%u AF_INET6a=%u "
			"ai_addrlen=%u sizeof(struct sockaddr)=%u sizeof(struct sockaddr_in)=%u "
			"sizeof(struct sockaddr_in6)=%u count=%u\n", 
				dn, a->ai_family, AF_INET, AF_INET6, a->ai_addrlen, 
				sizeof(struct sockaddr), sizeof(struct sockaddr_in), 
				sizeof(struct sockaddr_in6), node->count);
		}

		node->timeout = get_sec() + range_rand(DNS_CACHE_TIMEOUT/2, DNS_CACHE_TIMEOUT/2*3);
		return node;
	}

	// 没找到，是新节点
	node = cl_calloc(sizeof(ipc_dns_cache_t), 1);
	stlc_list_add_tail(&node->link, &ipc_dns_cache);

	node->dn = cl_strdup(dn);
	
	for (i = 0, a = res; a != NULL && i < MAX_RESOLV_IP; i++, a = a->ai_next) {
	}
	node->count = i;
	node->ipc = cl_calloc(sizeof(ipc_sock_t), node->count);
	for (i = 0, a = res; i < node->count && a != NULL && a->ai_addr; i++, a = a->ai_next) {
		psock = &node->ipc[i];
		psock->ai_family = a->ai_family;
		psock->ai_flags = a->ai_flags;
		psock->ai_socktype = a->ai_socktype;
		psock->ai_protocol = a->ai_protocol;
		psock->ai_addrlen = a->ai_addrlen;
		//psock->addr = *a->ai_addr;
		memcpy((void *)psock->addr.sockaddrc, a->ai_addr, a->ai_addrlen);
		log_debug("addrinfo dn=%s ai_family=%u AF_INET=%u AF_INET6a=%u "
		"ai_addrlen=%u sizeof(struct sockaddr)=%u sizeof(struct sockaddr_in)=%u "
		"sizeof(struct sockaddr_in6)=%u count=%u\n", 
			dn, a->ai_family, AF_INET, AF_INET6, a->ai_addrlen, 
			sizeof(struct sockaddr), sizeof(struct sockaddr_in), 
			sizeof(struct sockaddr_in6), node->count);
	}

	node->timeout = get_sec() + range_rand(DNS_CACHE_TIMEOUT/2, DNS_CACHE_TIMEOUT/2*3);

	return node;
}


static ipc_dns_cache_t *resolv_ipc_dns(const char *dn)
{
	int ret = 0;
	ipc_dns_cache_t *node = NULL, *n;
	struct addrinfo hints, *res = NULL;

	stlc_list_for_each_entry_safe(ipc_dns_cache_t, node, n, &ipc_dns_cache, link) {
		if (strcmp(dn, node->dn) == 0 && node->timeout > get_sec()) {
			log_debug("IPC_DNS: %s use cache\n", dn);
			return node;
		}
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;

	ret = getaddrinfo(dn, "53", &hints, &res);
	if (ret != 0) {
		log_info("getaddrinfo failed for %s err_ret=%d\n", dn, ret);
		return NULL;
	}
	
	cl_lock(&cl_priv->mutex);
	node = update_ipc_dns_cache(dn, res);
	cl_unlock(&cl_priv->mutex);
	
	freeaddrinfo(res); 

	return node;
}

static bool dns_resolv_ip_comm(char *ds)
{
	if (resolv_ipc_dns(ds) == NULL) {
		return false;
	}

	return true;
}

bool cl_get_addr_by_doname(char *dn, ipc_sock_t *sock_addr, int *num)
{
	bool ret = false;
	ipc_dns_cache_t *node = NULL, *n;

	if (!dn || !sock_addr || !num) {
		return false;
	}

	cl_lock(&cl_priv->mutex);
	*num = 0;
	stlc_list_for_each_entry_safe(ipc_dns_cache_t, node, n, &ipc_dns_cache, link) {
		if (!strcmp(dn, node->dn)) {
			if (node->count) {
				*num = node->count;
				memcpy((void *)sock_addr, (void *)node->ipc, sizeof(ipc_sock_t)*node->count);
				ret = true;
			}
			break;
		}
	}
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

u_int32_t cl_dns_thread(void *param)
{
	RS ret;
	cl_notify_pkt_t *notify;
	cl_thread_info_t *ti = &cl_priv->thread_dns_resolv;
	struct sockaddr_in from;
#ifdef MUT_SERVER_ADAPT
    int i = 0 , pos = 0, pos2 = 0;
    clib_dns_server_info_t* si;
#endif
	ti->proc_notify = dns_resolv_proc_notify;
	STLC_INIT_LIST_HEAD(&dns_cache);
	ipc_dns_cache_init();

// 改dns多国版，不预解析了
#ifndef MUT_SERVER_ADAPT
    
	cl_lock(&cl_priv->mutex);
	if (cl_priv->disp_server != NULL)
		disp_server = cl_strdup(cl_priv->disp_server);

	// 先预填几个，加快登录速度
	if (cl_priv->testdns) {
		cl_priv->ip_disp_server[cl_priv->num_ip_disp_server++] = ntohl(inet_addr(cl_priv->testdns));
	} else {
	//如果是国外服务器，不连接阿里云
#ifndef AWS_SERVER
//		cl_priv->ip_disp_server[cl_priv->num_ip_disp_server++] = ntohl(inet_addr("115.28.222.148"));
#endif
		//cl_priv->ip_disp_server[cl_priv->num_ip_disp_server++] = ntohl(inet_addr("61.139.124.118"));
	}
	cl_unlock(&cl_priv->mutex);

	// 预解析几个要用到的，本地缓存到。这样可以加快后续处理
	if (cl_priv->testdns) {
		set_test_dns_cache(DFL_CGI_SERVER,ntohl(inet_addr(cl_priv->testdns)));
		set_test_dns_cache(DFL_DIS_SERVER,ntohl(inet_addr(cl_priv->testdns)));
	} else {
		resolv_dns(DFL_CGI_SERVER);
		resolv_dns(DFL_DIS_SERVER);
	}
#endif

	dns_cache_init();

	add_cache_dns();

	while ( ! ti->stopping ) {
#ifndef MUT_SERVER_ADAPT
        if (disp_server != NULL) {
            if (cl_resolve_disp_server(disp_server) == RS_OK)
                SAFE_FREE(disp_server);
        }
#else
        
        if (cl_priv && cl_priv->clib_dns_servers != NULL && cl_priv->dns_count != 0) {
            for (i = 0; i < cl_priv->dns_count; i++,pos = (pos+1)%cl_priv->dns_count) {
                si = &cl_priv->clib_dns_servers[pos];
                //已经解析出来的，后续考虑在某些地区解析不到域名咋办？？
                if (si->dns[pos] != '\0' && look_up_dns_cache_by_name(si->dns) != NULL) {
                    continue;
                }
                
                if (si->failed_cnt > MAX_DNS_RESLOVE_4G_TRY_CNT && cl_priv->net_type == NET_TYPE_3G) {
                    //超过次数限制，不再解析了，避免4G下耗流量
                    pos = (pos+1)%cl_priv->dns_count;
                    break;
                }
                //阻塞解析，成功就跳出
                if (cl_resolve_disp_server(si->dns)) {
                    si->failed_cnt++;
                    //解析失败，下次再来，要保证大家都有机会
                    pos = (pos+1)%cl_priv->dns_count;
                    break;
                }
            }
        }

        if (cl_priv && cl_priv->clib_dns_servers_v6 != NULL && cl_priv->dns_count_v6 != 0) {
            for (i = 0; i < cl_priv->dns_count_v6; i++,pos2 = (pos2+1)%cl_priv->dns_count_v6) {
                si = &cl_priv->clib_dns_servers_v6[pos2];
                //已经解析出来的，后续考虑在某些地区解析不到域名咋办？？
                if (si->dns[pos] != '\0' && look_up_ipc_dns_cache_by_name(si->dns) != NULL) {
                    continue;
                }
                if (si->failed_cnt > MAX_DNS_RESLOVE_4G_TRY_CNT && cl_priv->net_type == NET_TYPE_3G) {
                    //超过次数限制，不再解析了，避免4G下耗流量
                    pos2 = (pos2+1)%cl_priv->dns_count_v6;
                    break;
                }
                //阻塞解析，成功就跳出
                if (!dns_resolv_ip_comm(si->dns)) {
                    si->failed_cnt++;
                    //解析失败，下次再来，要保证大家都有机会
                    pos2 = (pos2+1)%cl_priv->dns_count_v6;
                    break;
                }
            }
        }
#endif
		dns_cache_save();

		while ((notify = cl_recv_notify(ti, 1000, &from)) != NULL) {
			ret = dns_resolv_proc_notify(notify);
			if (ti->stopping)
				break;

			if ((notify->flags & CLNPF_ACK) != 0) {
				notify->err_code = ret;
				if (CLNE_RESOLV_CUSTOM != notify->type)
					notify->param_len = 0;
				sendto(ti->sock_notify, (char *)notify, notify->hdr_len + notify->param_len, 
					0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
			}
		}

		//for (i = 0; i < 100 && ! ti->stopping; i++)
			msleep(10);
	}

	SAFE_FREE(disp_server);
	free_dns_cache();
	free_ipc_dns_cache();
	log_info("DNS resolve thread exit now!\n");

	cl_destroy_thread(&cl_priv->thread_dns_resolv);
	
	return 0;
}

/*
	返回解析了多少个IP
*/
int my_gethostbyname(const char *name, u_int32_t *ipp, int ipp_buf_size)
{
	int ret, buf_size = 4096, len;
	cl_notify_pkt_t *pkt;
	
	pkt = cl_notify_pkt_new(buf_size, CLNE_RESOLV_CUSTOM, CLNPF_ACK);
	strcpy((char *)pkt->data, name);
	pkt->param_len = (int)strlen(name) + 1;

	ret = cl_send_notify_wait(&cl_priv->thread_dns_resolv, pkt, buf_size);
	if (ret < 0)
		len = 0;
	else
		len = min(ipp_buf_size, (int)pkt->param_len);
	memcpy(ipp, pkt->data, len);

	cl_notify_pkt_free(pkt);

	return len/4;
}

int disp_resolv_doname(int num, const char (* const name)[64])
{
    cl_notify_pkt_t *pkt;
	int len = 0;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (num == 0 ||
		!name) {
        return RS_INVALID_PARAM;
    }
    
    len = num*64;
	pkt = cl_notify_pkt_new(2048, CLNE_RESOLV_OEM_DONAME, 0);
	pkt->param_len = len;
    memcpy((void *)pkt->data, name, len);
	ret = cl_send_notify(&cl_priv->thread_dns_resolv, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

static void dns_cache_save()
{
	int i, n, m;
	char path[512];
	char buf[512];
	FILE *fp = NULL;

	if (!cl_priv || !cl_priv->priv_dir) {
		return;
	}
	
	if (!cl_priv->disp_need_save) {
		return;
	}

	log_info("enter\n");
	//表示收到disp回复才保存，避免网络不通dns解析出错
	if (!cl_priv->disp_recv) {
		return;
	}

	cl_priv->disp_need_save = false;
	
	sprintf(path, "%s/%s", cl_priv->priv_dir, DNS_CACHE_FILE);
	fp = fopen(path, "w");
	if (!fp) {
		return;
	}

	/*这里要处理一下，为了避免dns越积累越多，比如多登陆过其他oem的设备导致很多其他dns
	*cache就保存我们内置域名个数num*2 + oemnum(2)*2，先这样定义吧
	*/
	m = cl_priv->dns_count*2 + 2*2;
	n = (int)cl_priv->num_ip_disp_server;
	n = min(n, m);
	for(i = 0; i < n; i++) {
		sprintf(buf, "%u.%u.%u.%u\n", IP_SHOW(cl_priv->ip_disp_server[i]));
		fputs(buf, fp);
		log_info("save %s\n", buf);
	}

	fclose(fp);
}

static void dns_cache_init()
{
	char path[512];
	char buf[512];
	u_int32_t ip;
	FILE *fp = NULL;

	if (!cl_priv || !cl_priv->priv_dir) {
		return;
	}
	log_info("enter");
	sprintf(path, "%s/%s", cl_priv->priv_dir, DNS_CACHE_FILE);
	fp = fopen(path, "r");
	if (!fp) {
		log_info("path=%s fopen failed\n", path);
		return;
	}

	cl_priv->num_ip_disp_server_cache = 0;
	
	while(fgets(buf, sizeof(buf), fp)) {
		ip = inet_addr(buf);
		if (INADDR_NONE == ip) {
			continue;
		}
		ip = ntohl(ip);
		if (cl_priv->num_ip_disp_server_cache < MAX_DISP_SERVER_IP) {
			cl_priv->ip_disp_server_cache[cl_priv->num_ip_disp_server_cache++] = ip;
			log_info("ip=%u.%u.%u.%u\n", IP_SHOW(ip));
		}
	}

	fclose(fp);
}

CLIB_API RS cl_dns_cache_clean()
{
    cl_notify_pkt_t *pkt;
	RS ret;
    
	pkt = cl_notify_pkt_new(2048, CLNE_DNS_CLEAN, 0);
	pkt->param_len = 0;
	ret = cl_send_notify(&cl_priv->thread_dns_resolv, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

