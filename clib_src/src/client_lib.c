#include "client_lib.h"
#include "cl_priv.h"
#include "cl_dns.h"

#include "wait_server.h"
#include "ir_lib.h"
#include "lan_dev_probe_priv.h"
#include "smart_config_priv.h"
#include "env_mon_priv.h"
#include "cl_env_mon.h"
#include "uc_client.h"
#include "cl_server.h"
#include "uas_client.h"

#include "sdk_version.h"	// 这个头文件只能包含一次
#include "h264_decode.h"

#ifdef IOS_COMPILE
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifndef	NO_COMMUNITY
#include "ds_key.h"
#endif

/************************************************

	一些库接口，主要是库初始化和终止
	
*************************************************/

////////////////////////////////////////////

cl_priv_t *cl_priv = NULL;
extern u_int32_t cl_main_thread(void *param);
bool type_def_init();

void init_limit(cl_limit_t *lm)
{
	int i;
	
	cl_video_param_t *vp;
	
	lm->max_user_name_len = 31;
	lm->max_user_passwd_len = 32;
	lm->max_mod_name_len = 255;

	lm->max_wifi_ssid_len = 32;
	lm->max_wifi_passwd_len = 63;
	
	lm->max_area = 31; /* AREA_MAX_ID in mu */
	lm->max_area_name_len = 63;

	lm->max_scene = 32; /* SCENE_ID_MAX in mu */
	lm->max_scene_name_len = 63;

	lm->max_equipment = 200; /* MAX_REMOTE_NUM in mu */
	lm->max_eq_name_len = 63;

	lm->max_key_of_eq = 32; /* MAX_KEY_NUM in mu */
	lm->max_key_name_len = 63;
	lm->max_alarm_msg_len = 63;

	lm->max_belter_user = 8;
	lm->max_belter_user_name_len = 15; /* net_fm_t->name in mu */
    lm->max_belter_user_height = 300;
    lm->min_belter_user_height = 50;
    lm->max_belter_user_weight = 200;
    lm->min_belter_user_weight = 3;
    lm->max_belter_user_age = 160;
    lm->min_belter_user_age = 1;
    

	lm->max_008_timer = 31; /* MAX_PLUG_TIMER in mu */
	lm->max_008_timer_name_len = 63; /* MAX_PLUG_TIMER_NAME in mu */
	
	lm->max_record_timer = 31; /* MAX_REC_TIMER_NUM in mu */
	lm->max_record_timer_name_len = 63; /* MAX_REC_TIMER_NAME in mu */
	
	lm->max_scene_timer = 32; /* MAX_SCENE_TIMER */
	lm->max_scene_timer_name_len = 63; /* net_scene_timer_t in ds_proto.h */

	lm->max_phone_model_len = 31;
	lm->max_bind_name_len = 15;
	lm->max_bind_message_len = 39;
    //视频饱和度
    lm->max_video_brighness_val = 64;
    lm->min_video_brighness_val = -64;
    lm->max_video_contrast_val = 64;
    lm->max_video_saturation_val = 48;
    lm->max_video_gain_val = 100;
    //视频转速
    lm->max_video_roll_speed = 100;

	memset(lm->video_param, 0, sizeof(lm->video_param));
	vp = &lm->video_param[0];
	vp->width = 320;
	vp->height = 240;
	i = 0;
	vp->fps[i++] = 5;
	vp->fps[i++] = 10;
	vp->fps[i++] = 15;
	vp->fps[i++] = 20;
	vp->fps[i++] = 25;
	
	vp = &lm->video_param[1];
	vp->width = 640;
	vp->height = 480;
	i = 0;
	vp->fps[i++] = 5;
	vp->fps[i++] = 10;
	vp->fps[i++] = 15;
	vp->fps[i++] = 20;
	vp->fps[i++] = 25;

	vp = &lm->video_param[2];
	vp->width = 1280;
	vp->height = 720;
	i = 0;
	vp->fps[i++] = 5;
	vp->fps[i++] = 10;
	vp->fps[i++] = 15;
	vp->fps[i++] = 20;
	// no 25 fps
}

void uuid_str_to_bin(char *uuid, u_int8_t *bin)
{
	int i, k;
	char buf[3], *endp;
	
	buf[2] = '\0';
	memset(bin, 0, MAX_UUID_BIN_LEN);
	for (i = 0, k = 0; uuid[i] != '\0' && k < MAX_UUID_BIN_LEN; i++) {
		if (uuid[i] == '-' || uuid[i] == '_')
			continue;

		buf[0] = uuid[i++];
		buf[1] = uuid[i];

		bin[k++] = (u_int8_t)strtoul(buf, &endp, 16);
	}
}

static void set_uuid(char *uuid)
{
	memcpy(cl_priv->uuid, uuid, MAX_UUID_LEN);
	uuid_str_to_bin(uuid, cl_priv->uuid_bin);
}

// 域名解析
static clib_dns_server_info_t dns_servers[] = {
//	{"kentcentosvm.cloudapp.net",0,{0}},//印度服务器
    {"cn.ice.galaxywind.com",0,{0}}, //中国
    {"ie.ice.galaxywind.com",0,{0}}, //爱尔兰
    {"de.ice.galaxywind.com",0,{0}}, //德国
//    {"test1.ice.jiazhang008.com",0,{0}}, //美国
    {"us.ice.galaxywind.com",0,{0}}, //美国
 //   {"test2.ice.jiazhang008.com",0,{0}}, //美国
    {"jp.ice.galaxywind.com",0,{0}}, //日本
    {"sg.ice.galaxywind.com",0,{0}}, //新加坡
    {"br.ice.galaxywind.com",0,{0}}, //巴西
    {"au.ice.galaxywind.com",0,{0}} //澳大利亚
};

static clib_dns_server_info_t dns_servers_v6[] = {
//	{"kentcentosvm.cloudapp.net",0,{0}},//印度服务器
    {"cn.ice.galaxywind.com",0,{0}}, //中国
    {"ie.ice.galaxywind.com",0,{0}}, //爱尔兰
    {"de.ice.galaxywind.com",0,{0}}, //德国
//    {"test1.ice.jiazhang008.com",0,{0}}, //美国
    {"us.ice.galaxywind.com",0,{0}}, //美国
 //   {"test2.ice.jiazhang008.com",0,{0}}, //美国
    {"jp.ice.galaxywind.com",0,{0}}, //日本
    {"sg.ice.galaxywind.com",0,{0}}, //新加坡
    {"br.ice.galaxywind.com",0,{0}}, //巴西
    {"au.ice.galaxywind.com",0,{0}} //澳大利亚
};
static void init_dns_servers()
{
    cl_priv->clib_dns_servers = &dns_servers[0];
    cl_priv->dns_count = sizeof(dns_servers)/sizeof(clib_dns_server_info_t);

    cl_priv->clib_dns_servers_v6 = &dns_servers_v6[0];
    cl_priv->dns_count_v6 = sizeof(dns_servers_v6)/sizeof(clib_dns_server_info_t);
}

static int cl_time_init()
{
	time_t now = 0;
	int time_diff = 0;
	int time_dst_diff = 0;
	int time1 = 0;
	int time2 = 0;
	struct tm tmgm;
	struct tm tmgm2;
	struct tm tmgm3;
	struct tm tmlm;

	now = time(NULL);
	gmtime_r(&now, &tmgm);
	localtime_r(&now, &tmlm);
	memcpy((void *)&tmgm2, (void *)&tmgm, sizeof(tmgm2));
	memcpy((void *)&tmgm3, (void *)&tmgm, sizeof(tmgm2));

	time_diff = (int)now - (int)mktime(&tmgm);

	//用本地localtime来判断靠谱些
	if (tmlm.tm_isdst) {
		tmgm2.tm_isdst = 0;
		time1 = (int)mktime(&tmgm2);
		tmgm3.tm_isdst = 1;
		time2 = (int)mktime(&tmgm3);
		time_dst_diff = time2 - time1;
	}
	
	//_log_debug("get time1=%d time2=%d time_dst_diff=%d tmgm.tm_isdst=%u tmlm.tm_isdst=%u\n", 
		//time1, time2, time_dst_diff, tmgm.tm_isdst, tmlm.tm_isdst);

	return ((time_diff - time_dst_diff)/60);
}

#if 0
void test_dst(void)
{
	struct tm a, a1, b, b1;
	struct tm *m;
	time_t now, n1;
	char *buf1[64];

	memset(&a, 0x00, sizeof(struct tm));
	memset(&a1, 0x00, sizeof(struct tm));
	memset(&b, 0x00, sizeof(struct tm));
	memset(&b1, 0x00, sizeof(struct tm));

	//now = time(NULL);
	//m = gmtime(&now);

	//_log_debug("year %u month %u day %u %u:%u:%u\n", 1900 + m->tm_year, m->tm_mon + 1, m->tm_mday, m->tm_hour, m->tm_min, m->tm_sec);	
	n1 = mktime(m);

	strftime(buf1, sizeof(buf1), "%Y-%m-%d %H:%M:%S", &n1);

	_log_debug("[%s]\n", buf1);
}
#endif

#if 0
static int test_audio_decode(void)
{
	FILE *fp;
	char path[64], out_path[64];
	u_int8_t buf[170000], decode[170000];
	int len;
	
	sprintf(path, "%s/pcm.encode", cl_priv->dir);
	sprintf(out_path, "%s/out.pcm", cl_priv->dir);

	fp = fopen(path, "rb");
	if (!fp) {
		return -1;
	}

	len = fread(buf, 1, sizeof(buf), fp);
	if (len <= 0) {
		return -1;
	}
	fclose(fp);
	
	len = audio_decode1(buf, decode, len);

	_log_debug("xxx len %d\n", len);

	fp = fopen(out_path, "wb+");
	if (!fp) {
		return -1;
	}

	len = fwrite(decode, 1, len, fp);
	if (len <= 0) {
		return -1;
	}
	fclose(fp);
}

static int test_audio_encode(void)
{
	FILE *fp;
	char path[64], out_path[64];
	u_int8_t buf[170000], decode[170000];
	int len;
	
	sprintf(path, "%s/in.pcm", cl_priv->dir);
	sprintf(out_path, "%s/pcm.encode", cl_priv->dir);

	fp = fopen(path, "rb");
	if (!fp) {
		return -1;
	}

	len = fread(buf, 1, sizeof(buf), fp);
	if (len <= 0) {
		return -1;
	}
	fclose(fp);
	
	len = audio_encode1(buf, decode, len);

	_log_debug("xxx len %d\n", len);

	fp = fopen(out_path, "wb+");
	if (!fp) {
		return -1;
	}

	len = fwrite(decode, 1, len, fp);
	if (len <= 0) {
		return -1;
	}
	fclose(fp);
}

#endif

#ifdef IOS_COMPILE
static void fd_init()
{
    struct rlimit rlim;

    
    getrlimit(RLIMIT_NOFILE, &rlim);
    log_debug("1 RLIMIT_NOFILE rlim_cur=%"PRIu64" rlim_max=%llu\n", rlim.rlim_cur, rlim.rlim_max);
    rlim.rlim_cur = 4096;
    setrlimit(RLIMIT_NOFILE, &rlim);
    memset(&rlim, 0, sizeof(rlim));
    getrlimit(RLIMIT_NOFILE, &rlim);
    log_debug("2 rlim_cur=%"PRIu64" rlim_max=%llu\n", rlim.rlim_cur, rlim.rlim_max);
}
#else
static void fd_init()
{
	
}
#endif

CLIB_API RS cl_init(u_int32_t client_type, cl_lib_info_t *info)
{
	info->version[0] = 0;
	info->version[1] = VERSION_LIB;
	info->version[2] = VERSION_MAJOR;
	info->version[3] = VERSION_MINOR;
	strcpy(info->desc, CL_DESC);

	info->svn = SDK_SVN;

	fd_init();

	init_limit(&info->limit);

	if (cl_priv != NULL) {
		log_info("You have recall cl_init. It's init before.\n");
		return RS_OK;
	}

	//必须注释掉，否则define ANDROID_TO_FILE会导致崩溃
	//log_info("\n\n\n\n\n\n==================================================================\n");
	//log_info("client libary init.\n");
	
#ifdef	WIN32	
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		//log_info("WSAStartup=%d\n", iResult);
	}
#endif	

	cl_priv = (cl_priv_t *)cl_calloc(sizeof(cl_priv_t), 1);

	// 启动时间
	cl_priv->start_time = time(NULL);

	cl_priv->is_pt_mode = info->is_pt_mode;
	cl_priv->cleint_type = client_type;
	if (client_type == CID_WIN32) {
		cl_priv->flags |= CLIF_MULTI_USER;
	}
    memcpy(cl_priv->phone_desc, info->phone_desc, sizeof(cl_priv->phone_desc));

	memcpy(cl_priv->app_ver, info->app_ver, sizeof(cl_priv->app_ver));

#ifdef USE_TIME_MINS
	cl_priv->time_diff = cl_time_init();
#endif

    cl_priv->timezone = info->timezone;
	cl_priv->vvid = info->vvid;
	cl_priv->app_type = info->app_type;
	cl_priv->app_id = info->app_id;
	memcpy(cl_priv->oem_vendor, info->oem_vendor, sizeof(cl_priv->oem_vendor));

	//testip
	cl_priv->test_ip_num = info->test_ip_num;
	memcpy((void *)cl_priv->test_ip, (void *)info->test_ip, sizeof(cl_priv->test_ip));
	
	if (info->dir[0] != '\0') {
		info->dir[sizeof(info->dir) - 1] = '\0';
		STR_REPLACE(cl_priv->dir, info->dir);
	}

	if (info->priv_dir[0] != '\0') {
		info->priv_dir[sizeof(info->priv_dir) - 1] = '\0';
		STR_REPLACE(cl_priv->priv_dir, info->priv_dir);
	}

	cl_init_lock(&cl_priv->mutex);
	cl_init_lock(&cl_priv->mutex_request);
	
	STLC_INIT_LIST_HEAD(&cl_priv->wait_list);
	STLC_INIT_LIST_HEAD(&cl_priv->events);
	STLC_INIT_LIST_HEAD(&cl_priv->user);
	STLC_INIT_LIST_HEAD(&cl_priv->tmp_obj);

	event_more_info_init();
	
	cl_thread_init(&cl_priv->master);

	if (info->reserved) 
		cl_priv->testdns = cl_strdup(info->reserved);
	else
		cl_priv->testdns = NULL;
#ifndef MUT_SERVER_ADAPT
	cl_priv->disp_server = cl_strdup(DFL_DIS_SERVER);
#else
    init_dns_servers();
#endif
	cl_priv->ir_lib = cl_calloc(sizeof(ir_lib_t), 1);
	lan_dev_probe_init(cl_priv);
	set_uuid(info->uuid);

	ucc_port_init();

#ifndef REMOVE_H264_LIB
    // h264解码初始化
    Gw_m264_init();
#endif

	smart_config_init();
	type_def_init();
	
	/* 创建主线程 */
	cl_create_thread(&cl_priv->thread_main, "Thread-Main", cl_main_thread, NULL);

	/* 创建DNS解析线程 */
	cl_create_thread(&cl_priv->thread_dns_resolv, "Thread-DNS-Resolve", cl_dns_thread, NULL);

	return RS_OK;
}

CLIB_API RS cl_set_info(cl_lib_info_t *info)
{
	CL_CHECK_INIT;
	
	info->version[0] = 0;
	info->version[1] = VERSION_LIB;
	info->version[2] = VERSION_MAJOR;
	info->version[3] = VERSION_MINOR;
	strcpy(info->desc, CL_DESC);
	init_limit(&info->limit);

	cl_priv->timezone = info->timezone;
	cl_priv->vvid = info->vvid;
	if (info->dir[0] != '\0') {
		info->dir[sizeof(info->dir) - 1] = '\0';
		STR_REPLACE(cl_priv->dir, info->dir);
	}
	set_uuid(info->uuid);

	return RS_OK;
}

CLIB_API RS cl_get_info(cl_lib_info_t *info)
{
	CL_CHECK_INIT;
	
	info->version[0] = 0;
	info->version[1] = VERSION_LIB;
	info->version[2] = VERSION_MAJOR;
	info->version[3] = VERSION_MINOR;
	info->svn = SDK_SVN;
	strcpy(info->desc, CL_DESC);
	init_limit(&info->limit);

	info->timezone = cl_priv->timezone;
	info->vvid = cl_priv->vvid;
	memcpy(info->uuid, cl_priv->uuid, MAX_UUID_LEN);
	if (cl_priv->dir != NULL)
		strcpy(info->dir, cl_priv->dir);

	return RS_OK;
}

CLIB_API RS cl_set_net_type(int net_type, char *desc)
{
	int old_net_type = cl_priv->net_type;
	CL_CHECK_INIT;

	if(net_type != cl_priv->net_type){
		cl_priv->net_type = net_type;
		nd_login_debug(NULL, "net_type=%u:desc=%s\n", net_type, desc?desc:"NULL");
        if (net_type == NET_TYPE_NONE) {
            cl_disp_server_set(NULL);
        }
		cl_send_notify_simple(&cl_priv->thread_main, CLNE_NETWORK_CHANGED);
	}
	
	return RS_OK;
}

CLIB_API RS cl_set_net_baddr(u_int32_t baddr)
{
	CL_CHECK_INIT;

	cl_priv->app_baddr = baddr;
	
	return RS_OK;
}

CLIB_API cl_phone_version_t *cl_get_phone_version( )
{
	cl_phone_version_t *info;

	info = cl_calloc(sizeof(cl_phone_version_t), 1);
	
	info->iphone_newest_version = cl_strdup(cl_priv->iphone_newest_version  ? cl_priv->iphone_newest_version : "");
	info->android_newest_version = cl_strdup(cl_priv->android_newest_version ? cl_priv->android_newest_version  : "");
	
	info->desc_iphone_en = cl_strdup(cl_priv->desc_iphone_en ? cl_priv->desc_iphone_en : "");
	info->desc_iphone_ch = cl_strdup(cl_priv->desc_iphone_ch ? cl_priv->desc_iphone_ch : "");

	info->desc_android_en = cl_strdup(cl_priv->desc_android_en ? cl_priv->desc_android_en : "");
	info->desc_android_ch = cl_strdup(cl_priv->desc_android_ch ? cl_priv->desc_android_ch : "");

	return info;
}

CLIB_API void cl_free_phone_version(cl_phone_version_t *info)
{
	SAFE_FREE(info->iphone_newest_version);
	SAFE_FREE(info->android_newest_version);
	SAFE_FREE(info->desc_iphone_en);
	SAFE_FREE(info->desc_iphone_ch);
	SAFE_FREE(info->desc_android_en);
	SAFE_FREE(info->desc_android_ch);

	SAFE_FREE(info);
}

RS cl_stop()
{
	if (cl_priv == NULL)
		return RS_OK;

	cl_notify_destroy_thread(&cl_priv->thread_main);
	cl_notify_destroy_thread(&cl_priv->thread_dns_resolv);

	stlc_list_free(wait_t, &cl_priv->wait_list, link);
	stlc_list_free(event_info_t, &cl_priv->events, link);
	stlc_list_free(event_more_info_t, &cl_priv->event_more_info, link);

	cl_destroy_lock(&cl_priv->mutex);
	cl_destroy_lock(&cl_priv->mutex_request);

	SAFE_FREE(cl_priv->disp_server);
	SAFE_FREE(cl_priv->testdns);
	SAFE_FREE(cl_priv->ir_lib);
#ifndef REMOVE_H264_LIB
	Gw_m264_clean();
#endif
	smart_config_clean();

	SAFE_FREE(cl_priv->iphone_newest_version);
	SAFE_FREE(cl_priv->android_newest_version);
	SAFE_FREE(cl_priv->desc_iphone_en);
	SAFE_FREE(cl_priv->desc_iphone_ch);
	SAFE_FREE(cl_priv->desc_android_en);
	SAFE_FREE(cl_priv->desc_android_ch);
	
	SAFE_FREE(cl_priv->dir);
	SAFE_FREE(cl_priv->priv_dir);

	cl_em_free_city_list(cl_priv->city_list);
	
	SAFE_FREE(cl_priv);
	
#ifdef	WIN32	
	WSACleanup();
#endif

	log_info("client libary stopped.\n");
#ifndef	NO_COMMUNITY
	rsa_clean();
#endif

	return RS_OK;
}

CLIB_API RS cl_set_traffic_stat_enable(bool is_enable)
{
    CL_CHECK_INIT;
	
    //设置个变量，就不加锁了
	cl_priv->traffic_enable = !!is_enable;
    
    return RS_OK;
}

CLIB_API RS cl_set_phone_background(bool is_background)
{
    CL_CHECK_INIT;
	
    //设置个变量，就不加锁了
	cl_priv->run_in_background = !!is_background;
    
    return RS_OK;
}

static void add_data_2_traffic(cl_traffic_stat_t* stat,traffic_stat_item_t* item)
{
    stat->tx_pkts+=item->tx_pkts;
    stat->tx_bytes+=item->tx_bytes;
    stat->rx_pkts+=item->rx_pkts;
    stat->rx_bytes+=item->rx_bytes;
}

CLIB_API void cl_get_traffic_stat(cl_traffic_stat_t* stat,bool is_clear_data)
{
    //状态统计，不加锁了
//    int i;
//    traffic_stat_item_t* item;
//    static traffic_stat_item_t tcp_stat = {0};
//    static traffic_stat_item_t udp_stat = {0};
//    static traffic_stat_item_t http_stat = {0};
//    
    if (!stat || !cl_priv) {
        return;
    }
    
    memset(stat, 0, sizeof(*stat));
    
    add_data_2_traffic(stat,&cl_priv->traffic_stat.tcp_stat);
    add_data_2_traffic(stat,&cl_priv->traffic_stat.udp_stat);
    add_data_2_traffic(stat,&cl_priv->traffic_stat.http_stat);
// 和上次统计比较，代码不删除了
//    if (memcmp(&tcp_stat, &cl_priv->traffic_stat.tcp_stat, sizeof(tcp_stat))) {
//         printf("tcp add tx %"PRIu64" rx %llu\n",cl_priv->traffic_stat.tcp_stat.tx_bytes - tcp_stat.tx_bytes,
//                cl_priv->traffic_stat.tcp_stat.rx_bytes - tcp_stat.rx_bytes);
//        memcpy(&tcp_stat, &cl_priv->traffic_stat.tcp_stat, sizeof(tcp_stat));
//    }
//    
//    if (memcmp(&udp_stat, &cl_priv->traffic_stat.udp_stat, sizeof(udp_stat))) {
//        printf("udp add tx %"PRIu64" rx %llu\n",cl_priv->traffic_stat.udp_stat.tx_bytes - udp_stat.tx_bytes,
//               cl_priv->traffic_stat.udp_stat.rx_bytes - udp_stat.rx_bytes);
//        memcpy(&udp_stat, &cl_priv->traffic_stat.udp_stat, sizeof(udp_stat));
//    }
//    
//    if (memcmp(&http_stat, &cl_priv->traffic_stat.http_stat, sizeof(http_stat))) {
//        printf("http add tx %"PRIu64" rx %llu\n",cl_priv->traffic_stat.http_stat.tx_bytes - http_stat.tx_bytes,
//               cl_priv->traffic_stat.http_stat.rx_bytes - http_stat.rx_bytes);
//        memcpy(&http_stat, &cl_priv->traffic_stat.http_stat, sizeof(http_stat));
//    }
//

// 明细，暂时放这里
//    item = cl_priv->traffic_stat.tcp_cmd_stat;
//    for (i=0; i<sizeof(cl_priv->traffic_stat.tcp_cmd_stat)/sizeof(cl_priv->traffic_stat.tcp_cmd_stat[0]);
//         i++,item++) {
//        if (item->tx_pkts||item->rx_pkts) {
//            printf("index[%d] tx_byte [%"PRIu64"] tx_pkt[%u] rx_byte [%llu] rx_pkt[%u]\n",
//                   i,item->tx_bytes,item->tx_pkts,item->rx_bytes,item->rx_pkts);
//        }
//        
//    }
    
    //此操作在界面线程中，特殊情况有可能导致数据错乱，先这样，后续处理
    if (is_clear_data) {
        memset(&cl_priv->traffic_stat, 0x0, sizeof(cl_priv->traffic_stat));
    }
    
}

CLIB_API cl_traffic_stat_t* cl_get_tcp_cmd_traffic_stat(int* out_num,bool is_clear_data)
{
    int i;
    traffic_stat_item_t* item;
    cl_traffic_stat_t* out_traffic;
    
    if (!out_num || !cl_priv) {
        return NULL;
    }
    
    *out_num = 0;
    
    out_traffic = cl_calloc(sizeof(cl_traffic_stat_t),CMD_MAX+2);
    if (!out_traffic) {
        return NULL;
    }

    item = cl_priv->traffic_stat.tcp_cmd_stat;
    for (i=0; i<CMD_MAX;i++,item++) {
        out_traffic[i].rx_pkts = item->rx_pkts;
        out_traffic[i].rx_bytes = item->rx_bytes;
        out_traffic[i].tx_pkts = item->tx_pkts;
        out_traffic[i].tx_bytes = item->tx_bytes;
    }
    
    item = &cl_priv->traffic_stat.udp_stat;
    out_traffic[CMD_MAX].rx_pkts = item->rx_pkts;
    out_traffic[CMD_MAX].rx_bytes = item->rx_bytes;
    out_traffic[CMD_MAX].tx_pkts = item->tx_pkts;
    out_traffic[CMD_MAX].tx_bytes = item->tx_bytes;
    
    item = &cl_priv->traffic_stat.http_stat;
    out_traffic[CMD_MAX+1].rx_pkts = item->rx_pkts;
    out_traffic[CMD_MAX+1].rx_bytes = item->rx_bytes;
    out_traffic[CMD_MAX+1].tx_pkts = item->tx_pkts;
    out_traffic[CMD_MAX+1].tx_bytes = item->tx_bytes;
    
    *out_num = i+2;
    
    //此操作在界面线程中，特殊情况有可能导致数据错乱，先这样，后续处理
    if (is_clear_data) {
        memset(&cl_priv->traffic_stat.tcp_cmd_stat, 0x0, sizeof(cl_priv->traffic_stat.tcp_cmd_stat));
    }
    
    return out_traffic;
}


enum {
	APP_ID_LOCAL_HTL = 100,
	
};

static void app_id_map_init()
{
	switch(cl_priv->app_id) {
	case APP_ID_HTL:
		cl_priv->app_id = APP_ID_LOCAL_HTL;
		break;
	default:
		cl_priv->app_id = 0;
		break;
	}
}

bool type_def_init()
{
	//app_id map
	app_id_map_init();

	return true;
}


