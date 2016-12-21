#include "client_lib.h"
#include "cl_priv.h"
#include "cl_smart_config.h"
#include "smart_config_priv.h"
#include "lan_dev_probe_priv.h"
#ifdef WIN32
#include "Iphlpapi.h"
#else
#include <net/if.h>
#endif

#define	SCT_WIFI_SSID		0x10
#define	SCT_WIFI_PASSWD	0x11

// smart config 一个报文重复发送多少次
#define	SMART_CONFIG_PKT_COUNT	10
// 避免上面没调用stop，下面设置个最长超时时间
#define	SMART_CONFIG_MAX_TIMEOUT	(5*60)

#define MAX_SC_6621_BYTE_SEND_COUNT (20)
#define SC_PHONE_CONFIG_TIME (500)

//////////////////////////////////////////////////////////////
//for android wlan phone config

#define 	SNIFFER_CMD_SMARTCONFIG			1

#pragma pack(push,1)

typedef struct {
	u_int64_t src_sn;
	u_int64_t dst_sn;
	//命令=>SNIFFER_CMD_SMARTCONFIG
	u_int16_t cmd;
	// 参数长度
	u_int16_t param_len;
	//id,重复发送什么的，现在可以不用
	u_int16_t id;
	u_int16_t flags;
	//后面紧跟数据
	//u_int8_t data[0];
} sniffer_hdr_t;

typedef struct sniffer_tlv_s{
	u_int16_t type;
	u_int16_t len;
}sniffer_tlv_t;

#pragma pack(pop)


//设备监听端口
#define 	SNIFFER_PORT				9919

//type定义
#define 	SNIFFER_TLV_TYPE_OK			1
#define 	SNIFFER_TLV_TYPE_ACK		2
#define 	SNIFFER_TLV_TYPE_SSID		3
#define 	SNIFFER_TLV_TYPE_PASSWD		4
#define 	SNIFFER_TLV_TYPE_AUTHMODE		5


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

//////////////////////////////////////////////////////////////
typedef struct {
	struct stlc_list_head link;

	u_int32_t addr;//先暂时够用了
}smart_inf_info_t;

//airkiss
#define AK_ARRAY_LEN	(4)
//ak 发送状态机
enum{
	AK_STATUS_PREAMBEL = 0,
	AK_STATUS_MAGIC,
	AK_STATUS_PRE,
	AK_STATUS_DATA,
	AK_STATUS_MAX,
};

//sc通用发送状态机
/*
*时间戳	----2.4s----|----0.4s----|----0.4s----|----2.2s----|----4s----|----0.6s----|
*广播		preambel 	  |	magic		|pre		  | data		|idle		| idle
*组播		idle		  | data		|data 	| 	data 		|data		| idle
*/

enum {
	SC_SD_0_STATUS_2_4S = 0,
	SC_SD_1_STATUS_0_2s,
	SC_SD_2_STATUS_0_2s,
	SC_SD_3_STATUS_2_6s,
	SC_SD_4_STATUS_4_0s,
	SC_SD_5_STATUS_0_6s,
	SC_SD_STATUS_MAX,
};

#define SC_SD_TIME_2_4S (3500)
#define SC_SD_TIME_0_2S (200)
#define SC_SD_TIME_2_6S (2600)
#define SC_SD_TIME_4_0s (100)
#define SC_SD_TIME_0_6S	(100)

//先发20s的组播
#define SC_SD_MUL_SEND_TIME	(20000)

typedef struct {
	cl_thread_t *t_timer;
	// 开始配置的时间
	u_int32_t begin;
	u_int8_t *pkt;
	u_int16_t *pkt_ak;
	// 报文缓冲区大小
	int pkt_buf_size;
	// 报文数据长度
	int pkt_len;
	// 当前发到那个字节了
	int send_index;
	// 当前那个字节发送了多少次了
	int send_count;
	// 发送报文的套接字
	SOCKET sock;
	struct sockaddr_in addr;
	//手机做热点使用
	cl_thread_t* t_recv;
	u_int8_t *recv_pkt;
	int recv_buff_size;

	u_int32_t m_time;
	u_int32_t m_i_time;
	u_int32_t b_time;
	u_int32_t b_i_time;

	//airkiss
	u_int16_t ak_preamble[AK_ARRAY_LEN];
	u_int16_t ak_magic[AK_ARRAY_LEN];
	u_int16_t ak_pre[AK_ARRAY_LEN];
	int ak_status;
	int ak_send_num;//每个状态发送次数
	int ak_last_send_time;
	u_int32_t ak_send_a[AK_STATUS_MAX];
	int ak_pkt_len;

	//通用发送时间戳
	u_int32_t sc_sd_time[SC_SD_STATUS_MAX];
	u_int8_t sc_sd_status;
	u_int32_t sc_mul_20s_send;
	int sc_sd_all_num;//发送轮次

	//高级一键配置用，发送方案
	u_int8_t send_mode;
	bool is_mul;

	//发送累计时间
	u_int32_t send_time_total;
	u_int32_t last_send_time;

	//或者接口的时间
	u_int8_t addr_num;
	u_int32_t get_addr_time;
	struct stlc_list_head addr_list;
} smart_config_t;


typedef struct {
    smart_config_t gala_config;
    smart_config_t sc_6621_config;
    smart_config_t sc_phone_config;	
}smart_config_manager_t;



//高级一键配置
typedef struct {
	u_int8_t send_diff_ms;//发送间隔时间毫秒
}sc_advance_mode_t;

//前一半是组播，后一半是广播
#define ADVANCE_SC_MODE_MAX (12)

sc_advance_mode_t 
sc_mode[ADVANCE_SC_MODE_MAX] = {
	{5},
	{10},
	{15},
	{20},
	{25},
	{30},
	{5},
	{10},
	{15},
	{20},
	{25},
	{30},
};




static void clean_all_info(smart_config_t *sc);
void get_all_inf(smart_config_t *sc, bool brcast);

static int init_single_smart_config(smart_config_t* sc)
{
	if (!sc) {
        return -1;
    }
    
    sc->pkt_buf_size = 2048;
	sc->sock = INVALID_SOCKET;
	
	sc->pkt = cl_calloc(sc->pkt_buf_size, 1);
    if (!sc->pkt) {
        return -1;
    }

	sc->pkt_ak = cl_calloc(sc->pkt_buf_size, 1);
    if (!sc->pkt_ak) {
        return -1;
    }

	STLC_INIT_LIST_HEAD(&sc->addr_list);
	
    return 0;
}

static int init_smart_config_read_buffer(smart_config_t* sc)
{
	if (!sc) {
        return -1;
    }
    
    sc->recv_buff_size= 2048;
	
	sc->recv_pkt= cl_calloc(sc->recv_buff_size, 1);
    if (!sc->recv_pkt) {
        return -1;
    }
    
    return 0;
}

void smart_config_init()
{
	smart_config_manager_t* scm;
    
    scm = cl_calloc(sizeof(*scm), 1);
    if (!scm) {
        return;
    }
    
    init_single_smart_config(&scm->gala_config);
    init_single_smart_config(&scm->sc_6621_config);
	init_single_smart_config(&scm->sc_phone_config);

	//只有手机热点使用，节约点
	init_smart_config_read_buffer(&scm->sc_phone_config);

	cl_priv->smart_config = (void *)scm;
}


static void clean_single_smart_config(smart_config_t *sc)
{
    if (!sc) {
        return;
    }

    if(sc->recv_buff_size && sc->recv_pkt){
        SAFE_FREE(sc->recv_pkt);
        sc->recv_buff_size = 0;
    }

    cl_free(sc->pkt);
    cl_free(sc->pkt_ak);
    sc->pkt_buf_size = 0x0;
	CL_THREAD_OFF(sc->t_timer);
	CLOSE_SOCK(sc->sock);
    
}

void smart_config_clean()
{
	smart_config_manager_t* scm;

	if ((scm = (smart_config_manager_t *)cl_priv->smart_config) == NULL)
		return;
	
    clean_single_smart_config(&scm->gala_config);
    clean_single_smart_config(&scm->sc_6621_config);
	clean_single_smart_config(&scm->sc_phone_config);
	
	cl_free(scm);
	cl_priv->smart_config = NULL;
}

static smart_config_t *smart_config_get()
{
    smart_config_manager_t* scm;
    
	if ((scm = (smart_config_manager_t *)cl_priv->smart_config) == NULL)
		return NULL;
    
	return &scm->gala_config;
}

static smart_config_t * sc_6621_config_get()
{
    smart_config_manager_t* scm;
    
	if ((scm = (smart_config_manager_t *)cl_priv->smart_config) == NULL)
		return NULL;
    
	return &scm->sc_6621_config;
}

static smart_config_t * sc_phone_config_get()
{
    smart_config_manager_t* scm;
    
	if ((scm = (smart_config_manager_t *)cl_priv->smart_config) == NULL)
		return NULL;
    
	return &scm->sc_phone_config;
}


// DirectConfigDemo.cpp : Defines the entry point for the console application.
//



const unsigned char __crc8_tbl[256]=
{
	0x00,0x91,0xe3,0x72,0x07,0x96,0xe4,0x75,
	0x0e,0x9f,0xed,0x7c,0x09,0x98,0xea,0x7b,
	0x1c,0x8d,0xff,0x6e,0x1b,0x8a,0xf8,0x69,
	0x12,0x83,0xf1,0x60,0x15,0x84,0xf6,0x67,
	0x38,0xa9,0xdb,0x4a,0x3f,0xae,0xdc,0x4d,
	0x36,0xa7,0xd5,0x44,0x31,0xa0,0xd2,0x43,
	0x24,0xb5,0xc7,0x56,0x23,0xb2,0xc0,0x51,
	0x2a,0xbb,0xc9,0x58,0x2d,0xbc,0xce,0x5f,
	0x70,0xe1,0x93,0x02,0x77,0xe6,0x94,0x05,
	0x7e,0xef,0x9d,0x0c,0x79,0xe8,0x9a,0x0b,
	0x6c,0xfd,0x8f,0x1e,0x6b,0xfa,0x88,0x19,
	0x62,0xf3,0x81,0x10,0x65,0xf4,0x86,0x17,
	0x48,0xd9,0xab,0x3a,0x4f,0xde,0xac,0x3d,
	0x46,0xd7,0xa5,0x34,0x41,0xd0,0xa2,0x33,
	0x54,0xc5,0xb7,0x26,0x53,0xc2,0xb0,0x21,
	0x5a,0xcb,0xb9,0x28,0x5d,0xcc,0xbe,0x2f,
	0xe0,0x71,0x03,0x92,0xe7,0x76,0x04,0x95,
	0xee,0x7f,0x0d,0x9c,0xe9,0x78,0x0a,0x9b,
	0xfc,0x6d,0x1f,0x8e,0xfb,0x6a,0x18,0x89,
	0xf2,0x63,0x11,0x80,0xf5,0x64,0x16,0x87,
	0xd8,0x49,0x3b,0xaa,0xdf,0x4e,0x3c,0xad,
	0xd6,0x47,0x35,0xa4,0xd1,0x40,0x32,0xa3,
	0xc4,0x55,0x27,0xb6,0xc3,0x52,0x20,0xb1,
	0xca,0x5b,0x29,0xb8,0xcd,0x5c,0x2e,0xbf,
	0x90,0x01,0x73,0xe2,0x97,0x06,0x74,0xe5,
	0x9e,0x0f,0x7d,0xec,0x99,0x08,0x7a,0xeb,
	0x8c,0x1d,0x6f,0xfe,0x8b,0x1a,0x68,0xf9,
	0x82,0x13,0x61,0xf0,0x85,0x14,0x66,0xf7,
	0xa8,0x39,0x4b,0xda,0xaf,0x3e,0x4c,0xdd,
	0xa6,0x37,0x45,0xd4,0xa1,0x30,0x42,0xd3,
	0xb4,0x25,0x57,0xc6,0xb3,0x22,0x50,0xc1,
	0xba,0x2b,0x59,0xc8,0xbd,0x2c,0x5e,0xcf
};

/*   sc_crc8   */
/*-------------------------------------------------------------------------
	Description:	
		calculate crc8 for input data
		
	Arguments:
        	ptr - input data buffer
        	len - input data length
        	
	Return Value:
		crc8 result
		
	Note: 
-------------------------------------------------------------------------*/
static unsigned char sc_crc8(unsigned char *ptr, unsigned int len)
{
	unsigned char crc8,data;

	crc8=0;
	while(len--!=0)
	{
		data = *ptr++;
    	crc8=__crc8_tbl[crc8^data];
    }
    return crc8;
}

const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/************************************************************************************************/
const char crc8_ak_table[]={
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

static unsigned char sc_ak_crc8(unsigned char *ptr, unsigned int len)
{
	unsigned char crc8,data;

	crc8=0;
	while(len--!=0)
	{
		data = *ptr++;
    	crc8=crc8_ak_table[crc8^data];
    }
    return crc8;
}
/************************************************************************************************/


static int base64_enc(const char *str, char *buf)
{
	int i, len, m, v, out;
	unsigned char *ptr;

	len = (int)strlen(str);
	m = len/3;
	ptr = (unsigned char *)str;
	for (i = 0, out = 0; i < m; i++) {
		v = ((int)ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
		ptr += 3;
		
		buf[out++] = base64_table[(v >> 18) & 0x3F];
		buf[out++] = base64_table[(v >> 12) & 0x3F];
		buf[out++] = base64_table[(v >> 6) & 0x3F];
		buf[out++] = base64_table[v & 0x3F];
	}
	
	m = len%3;
	if (m == 1) {
		buf[out++] = base64_table[(*ptr >> 2) & 0x3f];
		buf[out++] = base64_table[(*ptr << 4) & 0x3f];
		
		buf[out++] = '=';
		buf[out++] = '=';
	} else if (m == 2) {
		v = ((int)ptr[0] << 8) | ptr[1];
		buf[out++] = base64_table[(v >> 10) & 0x3f];
		buf[out++] = base64_table[(v >> 4) & 0x3f];
		buf[out++] = base64_table[(v << 2) & 0x3f];
		
		buf[out++] = '=';
	}
	
	buf[out] = '\0';

	return out;
}

static RS sc_build_pkt(smart_config_t *sc, const char *ssid, const char *passwd)
{
	int n;
	char b64[256];
	u_int8_t Crc8;
	unsigned char * ptr, *ptr_total;


	ptr = sc->pkt;

	memset(ptr, 0x04, 8);
	ptr += 8;

	*ptr++ = 0x05;
	*ptr++ = 0x06;
	*ptr++ = 0x07;
	*ptr++ = 0x08;

	ptr_total = ptr;
	ptr++;

	// tlv: ssid 
	base64_enc(ssid, b64);
	n = (int)strlen(b64);
	*ptr++ = SCT_WIFI_SSID;
	*ptr++ = (unsigned char)n;
	memcpy(ptr, b64, n);
	ptr += n;

	// tlv: passwd
	base64_enc(passwd, b64);
	n = (int)strlen(b64);
	*ptr++ = SCT_WIFI_PASSWD;
	*ptr++ = (unsigned char)n;
	memcpy(ptr, b64, n);
	ptr += n;

	// fill total len
	*ptr_total = (u_int8_t)(ptr - ptr_total - 1);
	
	// crc8
	Crc8 = (unsigned char)sc_crc8((sc->pkt + 12), (unsigned int)(ptr - sc->pkt - 12));
	*ptr = (Crc8 & 0xF0) >> 4;
	ptr++;
	*ptr = Crc8 & 0x0F;
	ptr++;

	*ptr++ = 0x09;
	*ptr++ = 0x0A;
	*ptr++ = 0x0B;
	*ptr++ = 0x0C;

	sc->pkt_len = (unsigned char)(ptr - sc->pkt);

	return RS_OK;
}

static u_int16_t ak_get_o_data(u_int8_t *o_data, const char *ssid, const char *passwd, u_int8_t r_data)
{
	int i;
	int len = 0;
	int index = 0;

	len = (int)strlen(passwd);
	for(i = 0; i < len; i++) {
		o_data[index++] = (u_int8_t)passwd[i];
	}
	//rand
	o_data[index++] = r_data;
	//psd
	len = (int)strlen(ssid);
	for(i = 0; i < len; i++) {
		o_data[index++] = (u_int8_t)ssid[i];
	}
	
	return index;
}

static void ak_get_m_data(u_int16_t *m_data, const char *ssid, u_int16_t o_data_len)
{
	u_int8_t crc = 0;
	
	if (o_data_len < 16) {
		o_data_len += 128;
	}

	crc = (u_int8_t)sc_crc8((u_int8_t *)ssid, (u_int8_t)strlen(ssid)+1);
	m_data[0] = (o_data_len>>4)&0xf;
	m_data[1] = 0x10|(o_data_len&0xf);
	m_data[2] = 0x20|((crc>>4)&0xf);
	m_data[3] = 0x30|(crc&0xf);
}

static void ak_get_p_data(u_int16_t *p_data, u_int8_t psd_len)
{
	u_int8_t crc = 0;
	u_int8_t len_value[2] = {0, 0};

	len_value[0] = psd_len;
	crc = (u_int8_t)sc_ak_crc8(len_value, 1);
	p_data[0] = 0x40|((psd_len>>4)&0xf);
	p_data[1] = 0x50|(psd_len&0xf);
	p_data[2] = 0x60|((crc>>4)&0xf);
	p_data[3] = 0x70|crc&0xf;
}

static u_int16_t ak_get_s_data(u_int16_t *s_data, u_int8_t *o_data, u_int16_t o_data_len)
{
	int i = 0;
	int j = 0;
	int n = 0;
	int index = 0;
	u_int8_t crc = 0;
	u_int8_t a[5];
	u_int8_t remain_len = 0;
	u_int16_t ret_len = 0;

	n = o_data_len/4;
	remain_len = o_data_len%4;
	for(index = 0; index < n; index++) {
		a[0]= index;
		memcpy(&a[1], &o_data[index*4], 4);
		crc = (u_int8_t)sc_ak_crc8(a, 5);
		//这里跟java反解析带不太一致，代码是左移一位，应该跟协议不一样
		s_data[ret_len++] = BIT(7)|crc;
		s_data[ret_len++] = BIT(7)|index;
		log_debug("index=%u o_data_len=%u crc=%u index=%u\n", index, o_data_len, BIT(7)|crc, BIT(7)|index);
		for(j = 0; j < 4; j++, ret_len++) {
			s_data[ret_len] = BIT(8)|(u_int16_t)a[j+1];
		}
	}
	if (remain_len) {
		a[0] = index;
		memcpy(&a[1], &o_data[o_data_len - remain_len], remain_len);
		crc = (u_int8_t)sc_ak_crc8(a, remain_len+1);
		s_data[ret_len++] = BIT(7)|crc;
		s_data[ret_len++] = BIT(7)|index;
		for(j = 0; j < remain_len; j++, ret_len++) {
			s_data[ret_len] = BIT(8)|(u_int16_t)a[j+1];
		}
	}
	
	return ret_len;
}

static RS sc_build_airkiss_pkt(smart_config_t *sc, const char *ssid, const char *passwd)
{
	u_int8_t o_data[1024];
	u_int16_t s_data[1024];
	u_int16_t o_data_len = 0;
	u_int16_t s_data_len = 0;
	u_int8_t r_data = rand()%0xff;
	u_int16_t m_data[AK_ARRAY_LEN];
	u_int16_t p_data[AK_ARRAY_LEN];
	u_int16_t pre_data[AK_ARRAY_LEN] = {0x1,0x2,0x3,0x4};

	memset(o_data, 0, sizeof(o_data));
	memset(s_data, 0, sizeof(s_data));
	memset(m_data, 0, sizeof(m_data));
	memset(p_data, 0, sizeof(p_data));
	
	o_data_len = ak_get_o_data(o_data, ssid, passwd, r_data);
	ak_get_m_data(m_data, ssid, o_data_len);
	log_debug("%s m_data %u:%u:%u:%u\n", __FUNCTION__, 
		m_data[0], m_data[1], m_data[2], m_data[3]);
	ak_get_p_data(p_data, (u_int8_t)strlen(passwd));
	log_debug("%s p_data %u:%u:%u:%u\n", __FUNCTION__, 
		p_data[0], p_data[1], p_data[2], p_data[3]);
	s_data_len = ak_get_s_data(s_data, o_data, o_data_len);
	
	//组合
	//先发{1,2,3,4},发个10组
	memcpy((void *)sc->ak_preamble, (void *)pre_data, sizeof(sc->ak_preamble));
	//magic 5组
	memcpy((void *)sc->ak_magic, (void *)m_data, sizeof(sc->ak_magic));
	//pre
	memcpy((void *)sc->ak_pre, (void *)p_data, sizeof(sc->ak_pre));
	//data
	sc->ak_pkt_len = s_data_len;
	memcpy((void *)sc->pkt_ak, (void *)s_data, s_data_len*sizeof(u_int16_t));

	log_debug("sc->ak_pkt_len=%u\n", sc->ak_pkt_len);

	return RS_OK;
}

static RS sc_create_sock(smart_config_t *sc)
{
	int so_broadcast = 1;
	
	sc->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sc->sock == INVALID_SOCKET) {
		log_err(true, "creaet smart config socket failed!\n");
		return RS_ERROR;
	}

	setsockopt(sc->sock, SOL_SOCKET, SO_BROADCAST,( char *)&so_broadcast, sizeof(so_broadcast));
	
    sc->addr.sin_family = AF_INET;
    sc->addr.sin_port = htons(58888);
    //sc->addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    sc->addr.sin_addr.s_addr = 0;

	return RS_OK;
}

int sc_mutil_broadcast_timer_hotspot(cl_thread_t *t)
{
	int len;
	int ret;
	static unsigned char ipaddr[4] = {0xeb, 0x55, 0x0, 0x0};
	smart_config_t *sc = smart_config_get();
	smart_inf_info_t *p, *pn;
	bool get_addr = false;
	struct in_addr interface_addr;
	u_int32_t now = (u_int32_t)time(NULL);

	sc->t_timer = NULL;

	//这里要判断一下看是否需要重新获取接口地址，先定义成3s
	if (now > (sc->get_addr_time + 3)) {
		clean_all_info(sc);
		get_all_inf(sc, false);
		sc->get_addr_time = now;
		//这里防止sock被设置过不能用了，就重生成一下
		CLOSE_SOCK(sc->sock);
		sc_create_sock(sc);
		//log_debug("goto reget addr !!!!!!!!!!!!!!!!!!!!!!!!!sc->sock=%u !!!\n", sc->sock);
	}	

	if (sc->sock == INVALID_SOCKET) {
		log_err(false, "creaet smart config socket failed!\n");
		return 0;
	}

	len = sc->pkt[sc->send_index];

	//序列号０用来做结束使用
	if (sc->send_index == sc->pkt_len) {
		ipaddr[2] = 0;
		ipaddr[3] = 0;
	} else {
		ipaddr[2] = sc->send_index + 1;
		ipaddr[3] = sc->pkt[sc->send_index];		
	}

	sc->addr.sin_addr.s_addr = *(int *)ipaddr;
	//这里绑定下接口
	stlc_list_for_each_entry_safe(smart_inf_info_t, p, pn, &sc->addr_list, link) {
		memset((void *)&interface_addr, 0, sizeof(interface_addr));
		interface_addr.s_addr = p->addr;
		if (setsockopt(sc->sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&interface_addr, sizeof(interface_addr)) != 0) {
			log_err(true, "IP_MULTICAST_IF faled addr=%u.%u.%u.%u\n", IP_SHOW(interface_addr.s_addr));
		} else {
			get_addr = true;
		}
		ret = (int)sendto(sc->sock, sc->pkt, 10, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
		if (ret < 0) {
			log_err(true, "send faled\n");
		}
	}

	if (!get_addr) {
		ret = (int)sendto(sc->sock, sc->pkt, 10, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
	}

	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	}

// 每个字节发送3 次
	if (++sc->send_count >=  0x3 ) {
		sc->send_count = 0;
		sc->send_index++;
		
		if (sc->send_index > sc->pkt_len) {
			sc->send_index = 0;
			// 发完1轮休息15 ms
			CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer_hotspot, NULL, 15);
			return 0;
		}
		// 发完一个字节，休息15 毫秒
            CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer_hotspot, NULL, 15);
	}else{
		// 每次发包休息3毫秒
	 	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer_hotspot, NULL, 3);
	}
	
	return 0;
	
}

int sc_mutil_broadcast_timer(cl_thread_t *t)
{
	int len;
	int ret;
	static unsigned char ipaddr[4] = {0xeb, 0x55, 0x0, 0x0};
	smart_config_t *sc = smart_config_get();

	sc->t_timer = NULL;

	len = sc->pkt[sc->send_index];

	//序列号０用来做结束使用
	if (sc->send_index == sc->pkt_len) {
		ipaddr[2] = 0;
		ipaddr[3] = 0;
	} else {
		ipaddr[2] = sc->send_index + 1;
		ipaddr[3] = sc->pkt[sc->send_index];		
	}

	sc->addr.sin_addr.s_addr = *(int *)ipaddr;
	
	ret = (int)sendto(sc->sock, sc->pkt, 10, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));

	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	}

// 每个字节发送3 次
	if (++sc->send_count >=  0x3 ) {
		sc->send_count = 0;
		sc->send_index++;
		
		if (sc->send_index > sc->pkt_len) {
			sc->send_index = 0;
			// 发完1轮休息15 ms
			CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer, NULL, 15);
			return 0;
		}
		// 发完一个字节，休息15 毫秒
            CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer, NULL, 15);
	}else{
		// 每次发包休息3毫秒
	 	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer, NULL, 3);
	}
	
	return 0;
	
}


static int sc_timer(cl_thread_t *t)
{
	int len;
	smart_config_t *sc = smart_config_get();

	sc->t_timer = NULL;

	len = sc->pkt[sc->send_index];
	if (len == 0)
		len = 0x81;
	if ((sc->send_index & 1) == 0) {
		len = 0x9c + len;
	} else {
		len = 0x9c - len;
	}

	sendto(sc->sock, sc->pkt, len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));

#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, len);
#endif

	if (++sc->send_count >= SMART_CONFIG_PKT_COUNT) {
		sc->send_count = 0;
		sc->send_index++;

		if (sc->send_index >= sc->pkt_len) {
			sc->send_index = 0;
		}
	}

	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	} else {
		CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_timer, NULL, TIME_SMART_CONFIG_PKT);
	}
	
	return 0;
}

static int sc_airkiss_send(smart_config_t *sc, int len)
{
	int ret = 0;
	smart_inf_info_t *p, *pn;
	
	//先发255.255.255.255，失败再发下面地址列表
	sc->addr.sin_addr.s_addr = 0xffffffff;
	ret = (int)sendto(sc->sock, (u_int8_t *)sc->pkt_ak, len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
	if (ret > 0) {
		return 0;
	} else {
		log_err(true, "%s 255.255.255.255 send failed\n", __FUNCTION__);
	}

	//再尝试app设置的广播地址
	if (cl_priv->app_baddr != 0) {
		sc->addr.sin_addr.s_addr = htonl(cl_priv->app_baddr);
		ret = (int)sendto(sc->sock, (u_int8_t *)sc->pkt_ak, len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
		if (ret > 0) {
			return 0;
		} else {
			log_err(true, "%s %u.%u.%u.%u send failed\n", __FUNCTION__, IP_SHOW(cl_priv->app_baddr));
		}
	}

	//最后尝试地址列表
	stlc_list_for_each_entry_safe(smart_inf_info_t, p, pn, &sc->addr_list, link) {
		sc->addr.sin_addr.s_addr = p->addr;
		ret = (int)sendto(sc->sock, (u_int8_t *)sc->pkt_ak, len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
		if (ret < 0) {
			log_err(true, "smart send failed\n");
		}
	}

	return 0;
}

static void sc_airkiss_pre_send(smart_config_t *sc)
{
	int i;

	for(i = 0; i < 4; i++) {
		sc_airkiss_send(sc, (int)sc->ak_preamble[i]);
	}
}

static int sc_airkiss_timer()
{
	int len = 0;
	u_int32_t now = get_msec();
	
	smart_config_t *sc = smart_config_get();

	if (sc->ak_last_send_time == 0) {
		sc->ak_last_send_time = now;
	}

	switch(sc->ak_status) {
	case AK_STATUS_PREAMBEL:
		len = sc->ak_preamble[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case AK_STATUS_MAGIC:
		len = sc->ak_magic[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case AK_STATUS_PRE:
		len = sc->ak_pre[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case AK_STATUS_DATA:
		len = sc->pkt_ak[sc->ak_send_num++%sc->ak_pkt_len];
		break;
	default:
		log_debug("sc_airkiss_timer error zk_status=%u\n", sc->ak_status);
		sc->ak_status = AK_STATUS_PREAMBEL;
		return -1;
	}
	log_debug("%s num=%u status=%u len=%u\n", __FUNCTION__, sc->ak_send_num, sc->ak_status, len);
	sc_airkiss_send(sc, len);
	
	if ((now - sc->ak_last_send_time) >= sc->ak_send_a[sc->ak_status]) {
		sc->ak_last_send_time = now;
		sc->ak_send_num = 0;
		sc->ak_status++;
		sc->ak_status = sc->ak_status%AK_STATUS_MAX;
	}
	
	return 0;
}

static void sc_time_mul_timer()
{
	int ret;
	static unsigned char ipaddr[4] = {0xeb, 0x55, 0x0, 0x0};
	smart_config_t *sc = smart_config_get();

	//序列号０用来做结束使用
	if (sc->send_index == sc->pkt_len) {
		ipaddr[2] = 0;
		ipaddr[3] = 0;
	} else {
		ipaddr[2] = sc->send_index + 1;
		ipaddr[3] = sc->pkt[sc->send_index];		
	}
	sc->addr.sin_addr.s_addr = *(int *)ipaddr;
	ret = (int)sendto(sc->sock, sc->pkt, 1, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));

	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	}

	log_debug("enter %s count=%u index=%u ret=%u\n", __FUNCTION__, sc->send_count, sc->send_index, ret);
	
	sc->send_index++;
	if (sc->send_index > sc->pkt_len) {
		sc->send_index = 0;
	}
}

static int sc_time_ex(cl_thread_t *t)
{
	u_int32_t diff_time = 0;
	u_int32_t now_msec = get_msec();
	smart_config_t *sc = smart_config_get();

	sc->t_timer = NULL;
	
	sc->send_time_total += now_msec - sc->last_send_time;
	sc->last_send_time = now_msec;

	//先组播，再广播
	if (sc->send_time_total < sc->m_time) {
		sc_time_mul_timer();
	} else if ((sc->send_time_total - sc->m_time) < sc->m_i_time) {
		;//null
	} else if ((sc->send_time_total - sc->m_time - sc->m_i_time) < sc->b_time) {
		sc_airkiss_timer();
	} else if ((sc->send_time_total - sc->m_time - sc->m_i_time - sc->b_time) < sc->b_i_time) {
		;//null
	}else {
		sc->send_time_total = 0;
		sc->ak_status = AK_STATUS_PREAMBEL;
		sc->ak_send_num = 0;
		sc->ak_last_send_time = 0;
	}

	log_debug("enter %s total=%u sc->m_time=%u sc->b_time==%u\n", 
		__FUNCTION__, sc->send_time_total, sc->m_time, sc->b_time);
	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	} else {
		CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex, NULL, 5);
	}

	return 0;
}

static int sc_time_ex2(cl_thread_t *t)
{
	u_int32_t diff_time = 0;
	u_int32_t now_msec = get_msec();
	smart_config_t *sc = smart_config_get();

	sc->t_timer = NULL;
	
	sc->send_time_total += now_msec - sc->last_send_time;
	sc->last_send_time = now_msec;

	sc_airkiss_timer();
	sc_time_mul_timer();

	log_debug("enter %s total=%u sc->m_time=%u sc->b_time==%u\n", 
		__FUNCTION__, sc->send_time_total, sc->m_time, sc->b_time);
	
	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	} else {
		CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex2, NULL, 5);
	}

	return 0;
}

static void do_sc_time_send()
{
	u_int32_t now_msec = get_msec();
	smart_config_t *sc = smart_config_get();
	bool mul_send = false;
	bool br_send = false;
	int len = 0;

	//这里先处理下，头20s先发组播，完成后再发混合报文
	if ((sc->sc_mul_20s_send + SC_SD_MUL_SEND_TIME) > now_msec) {
		sc_time_mul_timer();
		return;
	}

	if (sc->ak_last_send_time == 0) {
		sc->ak_last_send_time = now_msec;
	}

	switch(sc->sc_sd_status) {
	case SC_SD_0_STATUS_2_4S:
		br_send = true;
		len = sc->ak_preamble[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case SC_SD_1_STATUS_0_2s:
		br_send = true;
		mul_send = true;
		len = sc->ak_magic[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case SC_SD_2_STATUS_0_2s:
		br_send = true;
		mul_send = true;
		len = sc->ak_pre[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case SC_SD_3_STATUS_2_6s:
		br_send = true;
		mul_send = true;
		len = sc->pkt_ak[sc->ak_send_num++%sc->ak_pkt_len];
		break;
	case SC_SD_4_STATUS_4_0s:
		mul_send = true;
		break;
	case SC_SD_5_STATUS_0_6s:
		break;
	default:
		sc->sc_sd_status = SC_SD_0_STATUS_2_4S;
		return;
	}

	if (br_send) {
		sc_airkiss_send(sc, len);
	}
	//组播发两轮停两轮
	if (mul_send && ((sc->sc_sd_all_num == 2) || (sc->sc_sd_all_num == 3))) {
		sc_time_mul_timer();
	}
	
	log_debug("%s status=%u mul_send=%u br_send=%u len=%u now_msec=%u lasttime=%u\n", 
		__FUNCTION__, sc->sc_sd_status, mul_send, br_send, len, now_msec, sc->ak_last_send_time);

	if ((now_msec - sc->ak_last_send_time) >= sc->sc_sd_time[sc->sc_sd_status]) {
		sc->ak_last_send_time = now_msec;
		sc->ak_send_num = 0;
		sc->sc_sd_status++;
		sc->sc_sd_status %= SC_SD_STATUS_MAX;
		
		sc->sc_sd_all_num++;
		sc->sc_sd_all_num %= 4;
	}
}

static int sc_time_ex3(cl_thread_t *t)
{
	smart_config_t *sc = smart_config_get();

	sc->t_timer = NULL;

	log_debug("enter %s \n", __FUNCTION__);
	do_sc_time_send();
	
	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	} else {
		//发送速率修改，一轮5ms，一轮10s
		if ((sc->sc_sd_all_num%2) == 0)  {
			CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex3, NULL, 5);
		} else {
			CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex3, NULL, 10);
		}
	}

	return 0;
}

static void do_sc_advance_send()
{
	u_int32_t now_msec = get_msec();
	smart_config_t *sc = smart_config_get();
	int len = 0;

	if (sc->is_mul) {
		sc_time_mul_timer();
		return;
	}

	if (sc->ak_last_send_time == 0) {
		sc->ak_last_send_time = now_msec;
	}

	switch(sc->sc_sd_status) {
	case SC_SD_0_STATUS_2_4S:
		len = sc->ak_preamble[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case SC_SD_1_STATUS_0_2s:
		len = sc->ak_magic[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case SC_SD_2_STATUS_0_2s:
		len = sc->ak_pre[sc->ak_send_num++%AK_ARRAY_LEN];
		break;
	case SC_SD_3_STATUS_2_6s:
		len = sc->pkt_ak[sc->ak_send_num++%sc->ak_pkt_len];
		break;
	case SC_SD_4_STATUS_4_0s:
		sc->ak_last_send_time = now_msec;
		sc->ak_send_num = 0;
		sc->sc_sd_status = (sc->sc_sd_status + 1)%SC_SD_STATUS_MAX;
		return;
	case SC_SD_5_STATUS_0_6s:
		sc->ak_last_send_time = now_msec;
		sc->ak_send_num = 0;
		sc->sc_sd_status = (sc->sc_sd_status + 1)%SC_SD_STATUS_MAX;
		return;
	default:
		sc->sc_sd_status = SC_SD_0_STATUS_2_4S;
		return;
	}

	log_debug("enter %s sc->sc_sd_status=%u len=%u\n", 
		__FUNCTION__, sc->sc_sd_status, len);
	
	sc_airkiss_send(sc, len);

	if ((now_msec - sc->ak_last_send_time) >= sc->sc_sd_time[sc->sc_sd_status]) {
		sc->ak_last_send_time = now_msec;
		sc->ak_send_num = 0;
		sc->sc_sd_status = (sc->sc_sd_status + 1)%SC_SD_STATUS_MAX;
	}
}

static int sc_time_ex4(cl_thread_t *t)
{
	smart_config_t *sc = smart_config_get();

	sc->t_timer = NULL;

	log_debug("enter %s timediff=%u\n", __FUNCTION__, sc_mode[sc->send_mode].send_diff_ms);
	do_sc_advance_send();
	
	if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
		log_err(false, "!!!@@@@### SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n", 
			sc->begin, get_sec());
	} else {
		CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex4, NULL, sc_mode[sc->send_mode].send_diff_ms);
	}

	return 0;
}

static RS gala_smart_config_start(const char* ssid,const char* password)
{
    smart_config_t *sc;
    sc = smart_config_get();
    
    sc_build_pkt(sc, ssid, password);
	if (sc_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_timer, NULL, TIME_SMART_CONFIG_PKT);
    
	return RS_OK;
}

static void sc_send_time_init(smart_config_t *sc)
{
	sc->ak_status = AK_STATUS_PREAMBEL;
	sc->ak_send_num = 0;

	//AK_STATUS_PREAMBEL发送70遍,时间
	sc->ak_send_a[AK_STATUS_PREAMBEL] = 2*60*4*5;
	//AK_STATUS_MAGIC 20遍
	sc->ak_send_a[AK_STATUS_MAGIC] = 10*4*5;
	//AK_STATUS_PRE 20遍
	sc->ak_send_a[AK_STATUS_PRE] = 10*4*5;
	//AK_STATUS_DATA 20遍
	sc->ak_send_a[AK_STATUS_DATA] = 20*sc->ak_pkt_len*5;
}

static void sc_sd_time_init(smart_config_t *sc)
{
	sc->sc_sd_status = SC_SD_0_STATUS_2_4S;

	sc->sc_sd_time[SC_SD_0_STATUS_2_4S] = SC_SD_TIME_2_4S;
	sc->sc_sd_time[SC_SD_1_STATUS_0_2s] = SC_SD_TIME_0_2S;
	sc->sc_sd_time[SC_SD_2_STATUS_0_2s] = SC_SD_TIME_0_2S;
	sc->sc_sd_time[SC_SD_3_STATUS_2_6s] = SC_SD_TIME_2_6S;
	sc->sc_sd_time[SC_SD_4_STATUS_4_0s] = SC_SD_TIME_4_0s;
	sc->sc_sd_time[SC_SD_5_STATUS_0_6s] = SC_SD_TIME_0_6S;
}

static RS gala_smart_config_start_ext(const char* ssid,const char* password, 
	u_int8_t m_time, u_int8_t m_i_time, u_int8_t b_time, u_int8_t b_i_time)
{
    smart_config_t *sc;
    sc = smart_config_get();

	log_debug("enter %s %u.%u.%u.%u\n", __FUNCTION__, m_time, m_i_time, b_time, b_i_time);
	//时间控制
	sc->m_time = (u_int32_t)m_time*1000;
	sc->m_i_time = (u_int32_t)m_i_time*1000;
	sc->b_time = (u_int32_t)b_time*1000;
	sc->b_i_time = (u_int32_t)b_i_time*1000;
	sc->send_time_total = 0;
	sc->send_index = 0;
	sc->last_send_time = get_msec();
	sc->ak_last_send_time = 0;
	sc->sc_mul_20s_send= get_msec();
	sc->sc_sd_all_num = 0;
	//airkiss数据构造
    sc_build_airkiss_pkt(sc, ssid, password);
	sc_send_time_init(sc);
	//通用发送时间戳
	sc_sd_time_init(sc);
	//组播数据构造
	sc_build_pkt(sc, ssid, password);
	//获取接口
	clean_all_info(sc);
	get_all_inf(sc, true);
	//获取sock
	if (sc_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	//CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex, NULL, 0);
	//CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex2, NULL, 0);
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex3, NULL, 0);
    
	return RS_OK;
}

static RS gala_smart_config_start_advance(const char* ssid,const char* password, int mode)
{
    smart_config_t *sc;
    sc = smart_config_get();

	log_debug("enter %s %s %s %d\n", 
		__FUNCTION__, ssid, password, mode);
	//时间控制
	sc->send_time_total = 0;
	sc->send_index = 0;
	sc->last_send_time = get_msec();
	sc->ak_last_send_time = 0;
	sc->sc_mul_20s_send= get_msec();
	sc->sc_sd_all_num = 0;
	
	sc->send_mode = (u_int8_t)mode;
	sc->send_mode = (((u_int8_t)mode - 1))%ADVANCE_SC_MODE_MAX;
	sc->is_mul = (sc->send_mode < (ADVANCE_SC_MODE_MAX/2));
	//airkiss数据构造
    sc_build_airkiss_pkt(sc, ssid, password);
	sc_send_time_init(sc);
	//通用发送时间戳
	sc_sd_time_init(sc);
	//组播数据构造
	sc_build_pkt(sc, ssid, password);
	//获取接口
	clean_all_info(sc);
	get_all_inf(sc, true);
	//获取sock
	if (sc_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	//CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex, NULL, 0);
	//CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex2, NULL, 0);
	//CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex3, NULL, 0);
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_time_ex4, NULL, 0);

	return RS_OK;
}

#ifdef WIN32
void get_all_inf(smart_config_t *sc, bool brcast)
{
	ULONG len;
	IP_ADAPTER_INFO info[100];
	PIP_ADAPTER_INFO pAdapter = NULL;
	IP_ADDR_STRING *pip;
	u_int32_t ip, mask;
	DWORD addr;
	int rev = 0;
	smart_inf_info_t *pinf;
	
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
				pinf = cl_calloc(sizeof(*pinf), 1);
				if (pinf) {
					STLC_INIT_LIST_HEAD(&pinf->link);
					stlc_list_add_tail(&pinf->link, &sc->addr_list);
					pinf->addr = addr;
					log_err(false, "get addr=%u.%u.%u.%u \n", IP_SHOW(htonl(addr)));
				}
			}
		}
	}

}
#else
void andriod_get_all_inf(smart_config_t *sc , bool brcast)
{
	u_int32_t addr;
	int i=0;
	int index = 0;
	int sockfd = -1;
	struct ifconf ifconf;
	unsigned char buf[1024];
	struct ifreq *ifreq;
	smart_inf_info_t *pinf;
	int flag = SIOCGIFADDR;

	if (brcast) {
		flag = SIOCGIFBRDADDR;
	}
	//获取接口信息
	ifconf.ifc_len = sizeof(buf);
	ifconf.ifc_buf = (char*)buf;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		return;
	}  

	ioctl(sockfd, SIOCGIFCONF, &ifconf);   

	ifreq = (struct ifreq*)buf;  
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--, index++)
	{
		if(ioctl(sockfd,flag,ifreq)==0){
			addr = ((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr.s_addr;
			if(addr !=0 && addr != 0xFFFFFFFF && addr != htonl(0x7F000001)){
				pinf = cl_calloc(sizeof(*pinf), 1);
				if (pinf) {
					STLC_INIT_LIST_HEAD(&pinf->link);
					stlc_list_add_tail(&pinf->link, &sc->addr_list);
					pinf->addr = addr;
					log_err(false, "get addr=%u.%u.%u.%u name=%s\n", IP_SHOW(htonl(addr)), ifreq->ifr_name);
				}
			}
		}

		ifreq++;
	}
	
	CLOSE_SOCK(sockfd);
}

void ios_get_all_inf(smart_config_t *sc, bool brcast)
{
	u_int32_t addr;
	int i=0;
	int sockfd = -1;
	struct ifconf ifconf;
	unsigned char buf[1024];
	struct ifreq *ifreq;
	smart_inf_info_t *pinf;

	log_err(false, "**************************\n");
	
	//获取接口信息
	ifconf.ifc_len = 1024;
	ifconf.ifc_buf = buf;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		return;
	}  

	ioctl(sockfd, SIOCGIFCONF, &ifconf); 

	ifreq = (struct ifreq*)buf;  
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--, ifreq++)
	{	
		if(ioctl(sockfd,SIOCGIFBRDADDR,ifreq)==0){
			addr = ((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr.s_addr;
			if(addr !=0 && addr != 0xFFFFFFFF && addr != htonl(0x7F000001)){
				pinf = cl_calloc(sizeof(*pinf), 1);
				if (pinf) {
					STLC_INIT_LIST_HEAD(&pinf->link);
					stlc_list_add_tail(&pinf->link, &sc->addr_list);
					pinf->addr = addr;
					log_err(false, "get addr=%u.%u.%u.%u name=%s\n", IP_SHOW(htonl(addr)), ifreq->ifr_name);
				}
			}
		}
	}
	
	CLOSE_SOCK(sockfd);
}

void get_all_inf(smart_config_t *sc, bool brcast)
{
	int i;
	
	if(cl_priv->cleint_type == CID_ANDROID){
		andriod_get_all_inf(sc, brcast);
	}else{
		ios_get_all_inf(sc, brcast);
	}
}
#endif

static void clean_all_info(smart_config_t *sc)
{
	smart_inf_info_t *p, *pn;

	stlc_list_for_each_entry_safe(smart_inf_info_t, p, pn, &sc->addr_list, link) {
		stlc_list_del(&p->link);
		cl_free(p);
	}
}

static RS gala_smart_config_mbroadcast_start(const char* ssid,const char* password)
{
    smart_config_t *sc;
    sc = smart_config_get();

	log_err(false, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    sc_build_pkt(sc, ssid, password);
	if (sc_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer, NULL, TIME_SMART_CONFIG_PKT);
    
	return RS_OK;
}

static RS gala_smart_config_mbroadcast_start_hotspot(const char* ssid,const char* password)
{
    smart_config_t *sc;
    sc = smart_config_get();

	//获取手机接口地址等信息
	get_all_inf(sc, false);
	sc->get_addr_time = (u_int32_t)time(NULL);
	
    sc_build_pkt(sc, ssid, password);
	if (sc_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_mutil_broadcast_timer_hotspot, NULL, TIME_SMART_CONFIG_PKT);
    
	return RS_OK;
}


// 6621一键配置

static RS sc_6621_build_pkt(smart_config_t *sc, const char *ssid, const char *passwd)
{
	u_int8_t Crc8;
	unsigned char * ptr;
    u_int8_t ssid_len =0,keylen = 0;
    
    
	ptr = sc->pkt;
	memset(ptr, 0x04, 8);
	ptr += 8;
    
	*ptr++ = 0x05;
	*ptr++ = 0x06;
	*ptr++ = 0x07;
	*ptr++ = 0x08;
    
    if (ssid != NULL) {
        ssid_len = (u_int8_t)strlen(ssid);
    }
    *ptr++ = ssid_len;
    
	if (ssid_len>0) {
        memcpy(ptr, ssid, ssid_len);
        ptr+=ssid_len;
    }
    
	if (passwd != NULL) {
        keylen = (u_int8_t)strlen(passwd);
    }
    *ptr++ = keylen;
    
	if (keylen > 0) {
        memcpy(ptr, passwd, keylen);
        ptr+=keylen;
    }
    
    Crc8 = (unsigned char)sc_crc8((sc->pkt + 12), (unsigned int)(ptr - sc->pkt - 12));
	*ptr = (Crc8 & 0xF0) >> 4;
	ptr++;
	*ptr = Crc8 & 0x0F;
	ptr++;
    
	*ptr++ = 0x09;
	*ptr++ = 0x0A;
	*ptr++ = 0x0B;
	*ptr++ = 0x0C;
    
	sc->pkt_len = (unsigned char)(ptr - sc->pkt);
    
	return RS_OK;
}

static RS sc_6621_create_sock(smart_config_t *sc)
{
	int so_broadcast = 1;
	
	sc->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sc->sock == INVALID_SOCKET) {
		log_err(true, "creaet smart config socket failed!\n");
		return RS_ERROR;
	}
    
	setsockopt(sc->sock, SOL_SOCKET, SO_BROADCAST,( char *)&so_broadcast, sizeof(so_broadcast));
	
    sc->addr.sin_family = AF_INET;
    sc->addr.sin_port = htons(60001);
    sc->addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
	return RS_OK;
}

static int sc_6621_timer(cl_thread_t *t)
{
	int len;
	smart_config_t *sc = sc_6621_config_get();
    
	sc->t_timer = NULL;
    
	len = sc->pkt[sc->send_index];
	if (len == 0)
		len = 0x81;
	if ((sc->send_index & 1) == 0) {
		len = 0x9c + len;
	} else {
		len = 0x9c - len;
	}
    
	sendto(sc->sock, sc->pkt, len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
    
#ifdef SUPPORT_TRAFFIC_STAT
    UDP_PKT_STAT(false, len);
#endif
    
    
    //每字节count
	if (++sc->send_count >= MAX_SC_6621_BYTE_SEND_COUNT) {
		sc->send_count = 0;
		sc->send_index++;
        
        //发送完成1轮就停，6621Demo如此，以后根据情况看是否不停发
		if (sc->send_index >= sc->pkt_len) {
			sc->send_index = 0;
            
            if (get_sec() - sc->begin > SMART_CONFIG_MAX_TIMEOUT) {
                log_err(false, "!!!@@@@### TS6621  SMART CONFIG timeout !!!@@@@, begin=%u, now=%u\n",
                        sc->begin, get_sec());
            } else {
                CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_6621_timer, NULL, TIME_SC_6621_SEND_BYTE_IDLE);
            }
            return 0;
		}
        //发送完一个字节，休息30ms
        CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_6621_timer, NULL, TIME_SC_6621_SEND_BYTE_IDLE);
	}else{
        //单个字节未发送完成，3毫秒一个
        CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_6621_timer, NULL, TIME_SC_6621_SEND_BYTE_INTER);
    }
    
    return 0;
}

static RS sc_6621_config_start(const char* ssid,const char* password)
{
    smart_config_t *sc;
    sc = sc_6621_config_get();
    
    sc_6621_build_pkt(sc, ssid, password);
	if (sc_6621_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_6621_timer, NULL, TIME_SC_6621_SEND_BYTE_IDLE);
    
	return RS_OK;
}

#ifdef	ANDROID

static int  send_bcast_packet(smart_config_t *sc)
{
	u_int32_t addr;
	brcdst_t *node;
	int i=0;
	int sockfd = -1;
	struct ifconf ifconf;
	unsigned char buf[1024];
	struct ifreq *ifreq;
	struct ifreq ifr;
//	int ret;

	//获取接口信息
	ifconf.ifc_len = 1024;
	ifconf.ifc_buf = buf;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		return -1;
	}  

	ioctl(sockfd, SIOCGIFCONF, &ifconf);   

	ifreq = (struct ifreq*)buf;  
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
		if(ioctl(sockfd,SIOCGIFBRDADDR,ifreq)==0){
			addr = ((struct sockaddr_in*)&(ifreq->ifr_broadaddr))->sin_addr.s_addr;
			if(addr !=0 && addr != 0xFFFFFFFF){
				sc->addr.sin_addr.s_addr = addr;
				 sendto(sc->sock, sc->pkt, sc->pkt_len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
//				log_info("sc_phone_wlan_config_timer send packet len = %d",ret);
			}
		}
		ifreq++;
	}
	CLOSE_SOCK(sockfd);
	return 0;
}

static int sc_phone_wlan_config_timer(cl_thread_t *t)
{
	smart_config_t *sc = sc_phone_config_get();
	
	sc->t_timer = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_timer, sc_phone_wlan_config_timer, NULL, SC_PHONE_CONFIG_TIME);
	send_bcast_packet(sc);
	
//	ret = sendto(sc->sock, sc->pkt, sc->pkt_len, 0, (struct sockaddr *)&sc->addr, sizeof(sc->addr));
//	log_info("sc_phone_wlan_config_timer send packet ret = %d",ret);
    
    return 0;
}

static void sc_phone_send_ack_to_dev(smart_config_t *sc,struct sockaddr_in *addr,sniffer_hdr_t* sh)
{
	char send_buff[256] = {0};
	sniffer_hdr_t* d_sh = (sniffer_hdr_t*)send_buff;
	sniffer_tlv_t* tlv = (sniffer_tlv_t*)(d_sh+1);
	int send_len = sizeof(*d_sh)+sizeof(*tlv);
	int i;

	d_sh->cmd = htons(SNIFFER_CMD_SMARTCONFIG);
	d_sh->param_len =htons( sizeof(*tlv));
	tlv->type = htons(SNIFFER_TLV_TYPE_ACK);

	for(i = 0 ; i< 3; i++){
		 sendto(sc->sock, send_buff,send_len , 0,(struct sockaddr *)addr, sizeof(struct sockaddr));
	}
}

static void sc_phone_do_smart_res_packet(smart_config_t *sc,struct sockaddr_in *addr, pkt_t *pkt,sniffer_hdr_t* sh)
{
	int total_len = sh->param_len;
	sniffer_tlv_t* tlv = (sniffer_tlv_t*)(sh+1);
	int len;

	while(total_len >=sizeof(sniffer_tlv_t)){
		tlv->len = ntohs(tlv->len);
		tlv->type = ntohs(tlv->type);

		if(tlv->type == SNIFFER_TLV_TYPE_OK){
			sc_phone_send_ack_to_dev(sc,addr,sh);
			lan_phone_send_config_ok_to_app(sh->src_sn);
		}

		len =  (sizeof(*tlv)+tlv->len);
		total_len -= len;
		tlv = (sniffer_tlv_t*)((char*)tlv + len);
	}
}

static int sc_phone_read(cl_thread_t *t)
{
	smart_config_t *sc;
	sniffer_hdr_t* sh;
	u_int16_t len;
	pkt_t *pkt;
	struct sockaddr_in addr;
	int addr_len;
	
	sc = (smart_config_t *)CL_THREAD_ARG(t);
	sc->t_recv = NULL;
	CL_THREAD_READ_ON(MASTER, sc->t_recv, sc_phone_read, (void *)sc, sc->sock);

	pkt = (pkt_t*)sc->recv_pkt;
	
	len = pkt->total = (int)recvfrom(sc->sock, (char *)pkt->data, sc->recv_buff_size - sizeof(*pkt), 0, 
			(struct sockaddr *)&addr, &addr_len);

	//log_info("sc_phone_read n = %d",len);
	if(len<sizeof(sniffer_hdr_t)){
		log_err(true, "sc_phone_read read udp failed\n");
		return 0;
	}
	sh = (sniffer_hdr_t*)pkt->data;
	sh->cmd = ntohs(sh->cmd);
	sh->param_len = ntohs(sh->param_len);

	//log_info("sc_phone_read n = %d,param_len = %d,sh->cmd = %d",len,sh->param_len,sh->cmd);
	
	switch(sh->cmd){
		case SNIFFER_CMD_SMARTCONFIG:
			sc_phone_do_smart_res_packet(sc,&addr,pkt,sh);
			break;
		default:
			break;
			
	}

	return 0;
	
}

static void sc_phone_config_build_packet(const char* ssid,const char* password,int auth_mode)
{
	smart_config_t *sc;
	sniffer_hdr_t* sh;
	sniffer_tlv_t* tlv;
	u_int16_t len;
	
    	sc = sc_phone_config_get();
	sh = (sniffer_hdr_t*)sc->pkt;

	memset(sc->pkt,0,sc->pkt_len);
	// fill header
	sh->cmd = htons(SNIFFER_CMD_SMARTCONFIG);
	sc->pkt_len = sizeof(*sh);

	// fill ssid
	len = (u_int16_t)strlen(ssid)+1;
	tlv = (sniffer_tlv_t*)(sh+1);
	tlv->len = htons(len);
	tlv->type = htons(SNIFFER_TLV_TYPE_SSID);
	strcpy((char*)(tlv+1),ssid);
	tlv = (sniffer_tlv_t*)((char*)tlv + len +sizeof(*tlv));
	sc->pkt_len += (len + sizeof(*tlv));

	tlv->len = htons(sizeof(u_int32_t));
	tlv->type = htons(SNIFFER_TLV_TYPE_AUTHMODE);
	*((u_int32_t*)(tlv+1)) = (u_int32_t)htonl(auth_mode);
	tlv = (sniffer_tlv_t*)((char*)tlv + sizeof(u_int32_t) +sizeof(*tlv));
	sc->pkt_len += (sizeof(u_int32_t) + sizeof(*tlv));
	// fill password,
	len = 0;
	if(password != NULL){
		len = (u_int16_t)strlen(password);
		//添加尾部\0 结尾
		if(len >0)
			len+=1;
	}

	tlv->len = htons(len);
	tlv->type = htons(SNIFFER_TLV_TYPE_PASSWD);
	if(len >0){
		strcpy((char*)(tlv+1),password);
	}
	tlv = (sniffer_tlv_t*)((char*)tlv + len +sizeof(*tlv));
	sc->pkt_len += (len + sizeof(*tlv));

	sh->param_len = sc->pkt_len - (u_int16_t)sizeof(sniffer_hdr_t);
	sh->param_len = htons(sh->param_len);
	
}

static RS sc_phone_wlan_create_sock(smart_config_t *sc)
{
	int so_broadcast = 1;
	
	sc->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sc->sock == INVALID_SOCKET) {
		log_err(true, "creaet smart config socket failed!\n");
		return RS_ERROR;
	}
    
    setsockopt(sc->sock, SOL_SOCKET, SO_BROADCAST,( char *)&so_broadcast, sizeof(so_broadcast));
	
    sc->addr.sin_family = AF_INET;
    sc->addr.sin_port = htons(SNIFFER_PORT);
    sc->addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
	return RS_OK;
}

static RS sc_phone_config_start(const char* ssid,const char* password,int auth_mode)
{
    smart_config_t *sc;
    sc = sc_phone_config_get();
    
    sc_phone_config_build_packet(ssid, password,auth_mode);
	if (sc_phone_wlan_create_sock(sc) != RS_OK) {
		return RS_ERROR;
	}
	sc->begin = get_sec();
	CL_THREAD_TIMER_ON(MASTER, sc->t_timer, sc_phone_wlan_config_timer, NULL, 0);
	CL_THREAD_READ_ON(MASTER, sc->t_recv, sc_phone_read, (void *)sc, sc->sock);
    
	return RS_OK;
}

#endif


RS smart_config_start(cl_notify_pkt_t *pkt)
{
	cln_smart_config_t *csc;

	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
    //风云广播配置
    gala_smart_config_start(csc->ssid,csc->passwd);
	
	return RS_OK;
}

RS smart_config_start_ext(cl_notify_pkt_t *pkt)
{
	cln_smart_config_t *csc;

	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
    //风云扩展组播广播配置
    gala_smart_config_start_ext(csc->ssid,csc->passwd, 
    csc->m_time, csc->m_i_time, csc->b_time, csc->b_i_time);
	
	return RS_OK;
}

RS smart_config_start_advance(cl_notify_pkt_t *pkt)
{
	cln_smart_config_t *csc;

	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
    //风云扩展组播广播配置
    gala_smart_config_start_advance(csc->ssid,csc->passwd,csc->auth_mode);
	
	return RS_OK;
}

RS smart_config_mbroadcast_start(cl_notify_pkt_t *pkt)
{
	cln_smart_config_t *csc;
	
	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
    //风云组播配置
    gala_smart_config_mbroadcast_start(csc->ssid,csc->passwd);
	
	return RS_OK;
}

RS smart_config_mbroadcast_start_hotspot(cl_notify_pkt_t *pkt)
{
	cln_smart_config_t *csc;
	
	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
    //风云组播配置，慢速发送
    gala_smart_config_mbroadcast_start_hotspot(csc->ssid,csc->passwd);
	
	return RS_OK;
}

RS smart_6621_config_start(cl_notify_pkt_t *pkt)
{
    cln_smart_config_t *csc;
	
	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
    //6621原厂广播配置
    sc_6621_config_start(csc->ssid,csc->passwd);
	
	return RS_OK;
}

#ifdef	ANDROID

RS smart_phone_wlan_config_start(cl_notify_pkt_t *pkt)
{
	cln_smart_config_t *csc;
	
	// 保险起见，先停止老的
	smart_config_stop();
	csc = (cln_smart_config_t *)pkt->data;
	if(csc->ssid == NULL || strlen(csc->ssid)== 0){
		return RS_INVALID_PARAM;
	}
    //手机热点配置模式
     sc_phone_config_start(csc->ssid,csc->passwd,csc->auth_mode);
	
	return RS_OK;
}
#else
RS smart_phone_wlan_config_start(cl_notify_pkt_t *pkt)
{
	
	return RS_NOT_SUPPORT;
}
#endif

static void single_smart_config_stop(smart_config_t *sc)
{
	smart_inf_info_t *p, *pn;
	
    if (!sc) {
        return;
    }
	CL_THREAD_OFF(sc->t_timer);
	CL_THREAD_OFF(sc->t_recv);
	CLOSE_SOCK(sc->sock);;
	sc->pkt_len = 0;
	sc->send_count = 0;
	sc->send_index = 0;

	//释放地址链表
	stlc_list_for_each_entry_safe(smart_inf_info_t, p, pn, &sc->addr_list, link) {
		stlc_list_del(&p->link);
		cl_free(p);
	}
}

RS smart_config_stop( )
{
	single_smart_config_stop(smart_config_get());
    single_smart_config_stop(sc_6621_config_get());
	single_smart_config_stop(sc_phone_config_get());
	
	return RS_OK;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


CLIB_API RS cl_smart_config_start(const char *ssid, const char *passwd)
{
    cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;
	
	if (ssid == NULL || passwd == NULL)
		return RS_INVALID_PARAM;
    
	pkt = cl_notify_pkt_new(1024, CLNE_START_SMART_CONFIG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_smart_config_start_ext(const char *ssid, const char *passwd, 
	u_int8_t m_time, u_int8_t m_i_time, u_int8_t b_time, u_int8_t b_i_time)
{
   cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;
	if (m_time == 0 &&
		b_time == 0) {
		return RS_INVALID_PARAM;
	}
	
	if (ssid == NULL || passwd == NULL) {
		return RS_INVALID_PARAM;
	}
    
	pkt = cl_notify_pkt_new(1024, CLNE_START_SMART_CONFIG_EXT, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);
	csc->m_time = m_time;
	csc->m_i_time = m_i_time;
	csc->b_time = b_time;
	csc->b_i_time = b_i_time;

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_smart_6621_config_start(const char *ssid, const char *passwd)
{
    cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;

	if (ssid == NULL || passwd == NULL)
		return RS_INVALID_PARAM;
    
	pkt = cl_notify_pkt_new(1024, CLNE_START_6621_SMART_CONFIG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_smart_mbroadcast_config_start(const char *ssid, const char *passwd)
{
    cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;
    
	if (ssid == NULL || passwd == NULL)
		return RS_INVALID_PARAM;

	pkt = cl_notify_pkt_new(1024, CLNE_START_MBROADCAST_SMART_CONFIG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_smart_mbroadcast_config_start_hotspot(const char *ssid, const char *passwd)
{
    cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;
	
	if (ssid == NULL || passwd == NULL)
		return RS_INVALID_PARAM;
    
	pkt = cl_notify_pkt_new(1024, CLNE_START_MBROADCAST_SMART_CONFIG_SLOWLY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_phone_wlan_config_start(const char *ssid, const char *passwd,int auth_mode)
{
    cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;
    
	if (ssid == NULL || passwd == NULL)
		return RS_INVALID_PARAM;
    
	pkt = cl_notify_pkt_new(1024, CLNE_START_PHONE_WLAN_CONFIG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);
	csc->auth_mode = auth_mode;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_smart_config_stop( )
{
    cl_notify_pkt_t *pkt;
	RS ret;
    
	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_STOP_SMART_CONFIG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_advance_smart_config_start(const char *ssid, const char *passwd, u_int8_t mode)
{
    cl_notify_pkt_t *pkt;
	cln_smart_config_t *csc;
	RS ret;
    
	CL_CHECK_INIT;
    
	if (ssid == NULL || passwd == NULL) {
		return RS_INVALID_PARAM;
	}
    
	pkt = cl_notify_pkt_new(1024, CLNE_START_ADVANCE_CONF, CLNPF_ACK);
	pkt->param_len = sizeof(cln_smart_config_t);
    csc = (cln_smart_config_t*)pkt->data;
	csc->ssid = cl_strdup(ssid);
	csc->passwd = cl_strdup(passwd);
	csc->auth_mode = mode;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_free((char *)csc->ssid);
	cl_free((char *)csc->passwd);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

