#include "client_lib.h"
#include "cl_priv.h"
#include "cl_notify.h"
#include "md5.h"

/**************************************************************************************

	client libary private functions
	一些内部用的公共基础函数

 **************************************************************************************/


err_no_str_t glb_error_str[ERR_MAX] = {
	{ERR_SN_INVALID, "i+设备序列号无效"},
	{ERR_SN_INVALID,    	"设备序列号无效"},
	{ERR_NICKNAME_INVALID,	"用户昵称无效"},
	{ERR_PASSWD_INVALID,   	"用户口令错误"},
	{ERR_CMD_INVALID,     	"无效命令"},
	{ERR_PARAM_INVALID,		"无效参数"},
	{ERR_MEMORY,       		"服务器内部分配内存失败"},
	{ERR_SYSTEM,       		"服务器内部系统还是调用失败"},
	{ERR_NICKNAME_CONFLICT,  "昵称冲突"},
	{ERR_NICKNAME_TOO_LONG, 	"昵称过长"},
	{ERR_EMAIL_TOO_LONG,    "email地址过长"},
	{ERR_DATABASE,			"数据库操作失败"},
	{ERR_CLIENT_VER, 		"手机客户端版本过低"},
	{ERR_DEV_OFFLINE,		" 设备离线 "},
	{ERR_VEDIO_OFF,		" 未插入摄像头 "},
	{ERR_DEV_SYS_ERR,		" 设备系统错误 "},
	{ERR_SELLER_NAME_CONFLICT,"销售人员用户名冲突"},
	{ERR_TOO_MANY, " 太多人在观看视频了 "},
	{ERR_PACKAGE_NAME_CONFLICT, " 太多人在观看视频了 "},
	{ERR_OUT_SERVICE, " 服务到期 "},
	{ERR_CARD_SN_INVALID, "充值卡序列号无效"},
	{ERR_CARD_PWD_INVALID, "充值卡密码无效"},
	{ERR_CARD_STATE_INVALID, "充值卡状态无效"},
	{ERR_CARD_NOTIME_TRANS, "设备无服务期限可转移"},
	{ERR_TIMEOUT, "超时失败"},
	{ERR_AGENT,		" 代理失败"},
	{ERR_EMAIL_INVALID, "email地址无效"},
	{ERR_FM_ID,"家庭成员ID无效 "},
	{ERR_FM_LIMIT, " 家庭成员配置过多"},
	{ERR_DEV_SYS_BUSY, " 系统忙，可能正在升级系统 "},
	{ERR_PLUG_TIMER_LIMIT, " 定时开关策略配置个数已达到最大"},
	{ERR_PLUG_TIMER_ID, " 定时开关策略ID无效"},
	{ERR_REMOTE_LIMIT, " 可控制电器配置已达最大数量"},
	{ERR_IR_DB_INVALID, " 红外编码库错误"},
	{ERR_REMOTE_BUTTON_LIMIT, " 可控电器按键达到上限"},
	{ERR_REMOTE_ID_INVALID, " 可控电器ID无效"},
	{ERR_REMOTE_KEY_ID_INVALID, " 可控电器KEY ID无效"},
	{ERR_REMOTE_BUSY, " 电器正忙，比如处于学习状态"},
	{ERR_REMOTE_KEY_VALID, " 电器按钮无效"},
	{ERR_REMOTE_CODE_LEARN_FAILED, "学习失败"},
	{ERR_PHONE_NUM_EXCESS,"超出支持的最大电话绑定数"},
	{ERR_NO_BIND_PHONE,"该智能网关尚未绑定手机"},
	{ERR_DEV_UNLINK,"设备未连接设备服务器"},
	{ERR_ALARM_PHONE_NOT_FOUNT, "绑定的报警手机号不存在"},
	{ERR_ALARM_VIDEO_NOT_FOUNT, "没有指定的报警录像"},
	{ERR_ALARM_LOG,"报警日志操作出错"},
	{ERR_ALARM_LOG_SYNC,"报警日志同步出错"},
	{ERR_REC_TIMER_LIMIT,"视频录制定时器:已经达到策略最大可配置数"},
	{ERR_REC_TIMER_OPT,"视频录制定时器:操作失败"},
	{ERR_REC_TIMER_ID,"视频录制定时器:定时器id无效"},
	{ERR_REC_TIMER_NTP,"ntp 未同步 添加失败"},
	{ERR_REC_TIMER_DURATION,"时长太短"},
	{ERR_NO_VTAP,"没有视频录像文件"},
	{ERR_SLAVE_OFFLINE,"从设备离线"},
	{ERR_DPI_FOR_PHONE,"手机在线不支持大分辨率、帧率的配置"}
};

char * get_err_str(u_int32_t err_no)
{
	int i;

	for(i = 1; i < ERR_MAX; i++){
		if(glb_error_str[i].err_code == err_no)
			return glb_error_str[i].err_str;
	}
	return "未知错误";
}

bool cl_is_sn(const char *sn)
{
	int i;

	for (i = 0; i < 12 && sn[i] != '\0'; i++) {
		if (sn[i] < '0' || sn[i] > '9')
			return false;
	}

	if (i == 12 && sn[i] == '\0')
		return true;

	return false;
}

/*
	1xx xxxx xxxx
*/
bool cl_is_phone(const char *str)
{
	int i;

	for (i = 0; i < 11 && str[i] != '\0'; i++) {
		if (str[i] < '0' || str[i] > '9')
			return false;
	}

	if (str[0] == 0)
		return false;

	if (i == 11 && str[i] == '\0')
		return true;

	return false;
}

/*
	通知线程销毁，外部调用进来的
*/
void cl_notify_destroy_thread(cl_thread_info_t *info)
{
	info->stopping = true;
	while (info->handle != 0 && ! IS_INVALID_SOCK(info->sock_notify)) {
		cl_send_notify_simple(info, CLNE_STOP);
		msleep(100);
	}
	info->stopping = 0; 
}

/*
	销毁线程一些信息，线程内部调用的
*/
void cl_destroy_thread(cl_thread_info_t *ti)
{
	//cl_lock(&cl_priv->mutex);
	
	CLOSE_SOCK(ti->sock_notify);

	SAFE_FREE(ti->udp_buf);
	ti->udp_buf_size = 0;
	
	ti->tid = 0;
	ti->proc_notify = NULL;
	ti->handle = 0;
	
	//cl_unlock(&cl_priv->mutex);
}

// check if a > b，允许回绕
bool u8_is_bigger(u_int8_t a, u_int8_t b)
{
	u_int8_t sub = a - b;

	return (a != b && sub < 0x7F);
}

bool u16_is_bigger(u_int16_t a, u_int16_t b)
{
	u_int16_t sub = a - b;

	return (a != b && sub < 0x7FFF);
}

bool u32_is_bigger(u_int32_t a, u_int32_t b)
{
	u_int32_t sub = a - b;

	return (a != b && sub < 0x7FFFFFFF);
}

u_int32_t cl_get_request_id()
{
	u_int32_t id;
	
	cl_lock(&cl_priv->mutex_request);
	id = ++cl_priv->request_id;
	if (id == 0)
		id = ++cl_priv->request_id;
	cl_unlock(&cl_priv->mutex_request);

	return id;
}

int range_rand(int begin, int end)
{
	int n;

	n = rand();
	n = (int)((double)n*(end-begin+1)/RAND_MAX + begin);

	return n;
}

void hash_passwd(u_int8_t *result, char *passwd)
{
	MD5_CTX ctx;

	MD5Init(&ctx);
	MD5Update(&ctx, (u_int8_t *)passwd, (u_int32_t)strlen(passwd));
	MD5Final(result, &ctx);
}

char *fmt_hex(char *result, u_int8_t *data, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		sprintf(result + i*2, "%02x", data[i]);
	}

	return result;
}

#define ADDR_EQUAL(addr1, addr2) \
	( (addr1)->sin_addr.s_addr == (addr2)->sin_addr.s_addr \
	&&(addr1)->sin_port == (addr2)->sin_port )

void update_slave_addr(slave_t *slave, struct sockaddr_in *addr)
{
	if(ADDR_EQUAL(addr, &slave->community->addr)){
		return;
	}

	log_debug("%s, device %012llu addr changed from %u.%u.%u.%u (%u) to %u.%u.%u.%u (%u)\n",
		__FUNCTION__, slave->sn, 
		IP_SHOW(ntohl(slave->community->addr.sin_addr.s_addr)), 
		ntohs(slave->community->addr.sin_port),
		IP_SHOW(ntohl(addr->sin_addr.s_addr)), ntohs(addr->sin_port));
	memcpy(&slave->community->addr, addr, sizeof(*addr));
}

tmp_obj_t *tmp_obj_alloc(int ex_size, void (*free_func)(void *))
{
	tmp_obj_t *to;

	to = (tmp_obj_t *)cl_calloc(sizeof(tmp_obj_t) + ex_size, 1);
	to->handle = handle_create(HDLT_TMP);
	to->free_obj = free_func;

	cl_lock(&cl_priv->mutex);
	stlc_list_add_tail(&to->link, &cl_priv->tmp_obj);
	cl_unlock(&cl_priv->mutex);

	return to;
}

void tmp_obj_free(tmp_obj_t *to)
{
	cl_lock(&cl_priv->mutex);
	if (to->link.next != NULL)
		stlc_list_del(&to->link);
	cl_unlock(&cl_priv->mutex);

	if (to->free_obj == NULL) {
		cl_free(to);
	} else {
		to->free_obj((void *)to);
	}
}

void tmp_obj_del_all()
{
	tmp_obj_t *to, *next;
	
	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry_safe(tmp_obj_t, to, next, &cl_priv->user, link) {
		tmp_obj_free(to);
	}
	
	cl_unlock(&cl_priv->mutex);
}

unsigned char timer_week_right_shift(unsigned char week)
{
	return  ((week&0x7F) >> 1) | ((week&1) << 6);
}

unsigned char timer_week_left_shift(unsigned char week)
{
	return  ((week << 1)&0x7F) | ((week >> 6) & 0x1);
}

void add_traffic_to_item(traffic_stat_item_t* item, bool is_rx,u_int32_t bytes)
{
    if (is_rx) {
        item->rx_pkts++;
        item->rx_bytes+=bytes;
    }else{
        item->tx_pkts++;
        item->tx_bytes+=bytes;
    }
}

void clib_traffic_stat(u_int32_t data_type, bool is_rx ,u_int32_t bytes)
{
    if ( !cl_priv->traffic_enable || data_type >= TRAFFIC_MAX ) {
        return;
    }
    
    if (data_type == TRAFFIC_UDP) {
        add_traffic_to_item(&cl_priv->traffic_stat.udp_stat,is_rx,bytes);
    }else if(data_type == TRAFFIC_HTTP) {
        add_traffic_to_item(&cl_priv->traffic_stat.http_stat,is_rx,bytes);
    }else {
        add_traffic_to_item(&cl_priv->traffic_stat.tcp_stat,is_rx,bytes);
        add_traffic_to_item(&cl_priv->traffic_stat.tcp_cmd_stat[data_type],is_rx,bytes);
    }
}
