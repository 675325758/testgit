#include "client_lib.h"
#include "cl_priv.h"
#include "cl_notify.h"
#include "md5.h"

/**************************************************************************************

	client libary private functions
	һЩ�ڲ��õĹ�����������

 **************************************************************************************/


err_no_str_t glb_error_str[ERR_MAX] = {
	{ERR_SN_INVALID, "i+�豸���к���Ч"},
	{ERR_SN_INVALID,    	"�豸���к���Ч"},
	{ERR_NICKNAME_INVALID,	"�û��ǳ���Ч"},
	{ERR_PASSWD_INVALID,   	"�û��������"},
	{ERR_CMD_INVALID,     	"��Ч����"},
	{ERR_PARAM_INVALID,		"��Ч����"},
	{ERR_MEMORY,       		"�������ڲ������ڴ�ʧ��"},
	{ERR_SYSTEM,       		"�������ڲ�ϵͳ���ǵ���ʧ��"},
	{ERR_NICKNAME_CONFLICT,  "�ǳƳ�ͻ"},
	{ERR_NICKNAME_TOO_LONG, 	"�ǳƹ���"},
	{ERR_EMAIL_TOO_LONG,    "email��ַ����"},
	{ERR_DATABASE,			"���ݿ����ʧ��"},
	{ERR_CLIENT_VER, 		"�ֻ��ͻ��˰汾����"},
	{ERR_DEV_OFFLINE,		" �豸���� "},
	{ERR_VEDIO_OFF,		" δ��������ͷ "},
	{ERR_DEV_SYS_ERR,		" �豸ϵͳ���� "},
	{ERR_SELLER_NAME_CONFLICT,"������Ա�û�����ͻ"},
	{ERR_TOO_MANY, " ̫�����ڹۿ���Ƶ�� "},
	{ERR_PACKAGE_NAME_CONFLICT, " ̫�����ڹۿ���Ƶ�� "},
	{ERR_OUT_SERVICE, " ������ "},
	{ERR_CARD_SN_INVALID, "��ֵ�����к���Ч"},
	{ERR_CARD_PWD_INVALID, "��ֵ��������Ч"},
	{ERR_CARD_STATE_INVALID, "��ֵ��״̬��Ч"},
	{ERR_CARD_NOTIME_TRANS, "�豸�޷������޿�ת��"},
	{ERR_TIMEOUT, "��ʱʧ��"},
	{ERR_AGENT,		" ����ʧ��"},
	{ERR_EMAIL_INVALID, "email��ַ��Ч"},
	{ERR_FM_ID,"��ͥ��ԱID��Ч "},
	{ERR_FM_LIMIT, " ��ͥ��Ա���ù���"},
	{ERR_DEV_SYS_BUSY, " ϵͳæ��������������ϵͳ "},
	{ERR_PLUG_TIMER_LIMIT, " ��ʱ���ز������ø����Ѵﵽ���"},
	{ERR_PLUG_TIMER_ID, " ��ʱ���ز���ID��Ч"},
	{ERR_REMOTE_LIMIT, " �ɿ��Ƶ��������Ѵ��������"},
	{ERR_IR_DB_INVALID, " �����������"},
	{ERR_REMOTE_BUTTON_LIMIT, " �ɿص��������ﵽ����"},
	{ERR_REMOTE_ID_INVALID, " �ɿص���ID��Ч"},
	{ERR_REMOTE_KEY_ID_INVALID, " �ɿص���KEY ID��Ч"},
	{ERR_REMOTE_BUSY, " ������æ�����紦��ѧϰ״̬"},
	{ERR_REMOTE_KEY_VALID, " ������ť��Ч"},
	{ERR_REMOTE_CODE_LEARN_FAILED, "ѧϰʧ��"},
	{ERR_PHONE_NUM_EXCESS,"����֧�ֵ����绰����"},
	{ERR_NO_BIND_PHONE,"������������δ���ֻ�"},
	{ERR_DEV_UNLINK,"�豸δ�����豸������"},
	{ERR_ALARM_PHONE_NOT_FOUNT, "�󶨵ı����ֻ��Ų�����"},
	{ERR_ALARM_VIDEO_NOT_FOUNT, "û��ָ���ı���¼��"},
	{ERR_ALARM_LOG,"������־��������"},
	{ERR_ALARM_LOG_SYNC,"������־ͬ������"},
	{ERR_REC_TIMER_LIMIT,"��Ƶ¼�ƶ�ʱ��:�Ѿ��ﵽ��������������"},
	{ERR_REC_TIMER_OPT,"��Ƶ¼�ƶ�ʱ��:����ʧ��"},
	{ERR_REC_TIMER_ID,"��Ƶ¼�ƶ�ʱ��:��ʱ��id��Ч"},
	{ERR_REC_TIMER_NTP,"ntp δͬ�� ���ʧ��"},
	{ERR_REC_TIMER_DURATION,"ʱ��̫��"},
	{ERR_NO_VTAP,"û����Ƶ¼���ļ�"},
	{ERR_SLAVE_OFFLINE,"���豸����"},
	{ERR_DPI_FOR_PHONE,"�ֻ����߲�֧�ִ�ֱ��ʡ�֡�ʵ�����"}
};

char * get_err_str(u_int32_t err_no)
{
	int i;

	for(i = 1; i < ERR_MAX; i++){
		if(glb_error_str[i].err_code == err_no)
			return glb_error_str[i].err_str;
	}
	return "δ֪����";
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
	֪ͨ�߳����٣��ⲿ���ý�����
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
	�����߳�һЩ��Ϣ���߳��ڲ����õ�
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

// check if a > b���������
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
