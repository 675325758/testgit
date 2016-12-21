#ifndef	__CL_USER_PRIV_H__
#define	__CL_USER_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_user.h"

#pragma pack(push,1)

/* 设备版本信息 */
typedef struct ms_version_s{
	u_int8_t major;	/* 主版本 */
	u_int8_t minor;	/* 次版本 */
	u_int8_t revise;	/* 修订版本 */
	u_int8_t pad;		/* 填充字节 */
} ms_version_t;

/* 设备的固件版本和升级版本信息 */
typedef struct ms_dev_version_s{
	ms_version_t soft_version;		/* 固件版本信息 */
	ms_version_t upgrade_version;	/* 升级包版本信息 */
} ms_dev_version_t;

typedef struct {
	u_int16_t ssid_len;
	u_int16_t psswd_len;
	char ssid[32];
	char psswd[64];
} misc_ssid_pwd_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t pad[3];
	misc_ssid_pwd_item_t item[0];
} misc_ssid_pwd_t;

#define	MNIF_CONNECT	BIT(0)
#define	MNIF_WAN	BIT(1)
#define	MNIF_IP_INVALID	BIT(2)

typedef struct {
	char name[16];
	// MNIF_XXX
	u_int8_t flags;
	u_int8_t pad;
	u_int16_t mtu;
	u_int32_t ip;
	u_int64_t rx_bytes;
	u_int64_t tx_bytes;
} misc_ni_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t item_len;
	u_int8_t pad[2];
	misc_ni_item_t item[1];
} misc_ni_t;

typedef struct {
	u_int32_t ip;
	u_int8_t mac[6];
	u_int8_t is_from_wifi;
	u_int8_t name_len;
	char name[64];
} misc_client_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t item_len;
	u_int8_t pad[2];
	misc_client_item_t items[0];
} misc_client_t;

// 用于处理RF设备大报文
typedef struct {
    struct stlc_list_head link;
    u_int8_t frame_index;
    u_int8_t buf[1024];
}rf_dev_defrag_info;

typedef struct {
	// 设备启动多长时间了
	u_int32_t uptime;
	// 什么时候查询回来的，单位毫秒
	u_int32_t query_uptime;
	
	// 设备与服务器连接多长时间了
	u_int32_t online;
	// 什么时候查询回来的，单位毫秒
	u_int32_t query_online;
	
	// 设备上互联网多长时间了
	u_int32_t conn_internet;
	// 什么时候查询回来的，单位毫秒
	u_int32_t query_conn_internet;
	
	// 软件版本号
	ms_dev_version_t version;
	//rf设备下单片机版本，
	ms_version_t rf_stm_ver;
	// 新版本信息 ，can_update为非0时有效
	ms_version_t new_version;
	// 非0表示设备有新版本可以升级
	char can_update;
	// 非0表示可以自动升级，0表示需要手工升级固件
	char can_auto_update;
	char pad[2];
	// 设备新版本描述信息
	char *release_desc;
	// 设备新版本url
	char *release_url;
	// 设备新版本发布日期
	char *release_date;

	// CPU使用率，小数点两位
	u_int16_t cpu;
	// 内存使用率，小数点两位
	u_int16_t mem;
	char ap_ssid[32 + 4];
	char ap_passwd[64 + 4];

	// 网络序
	misc_ni_item_t wan;
	misc_ni_item_t lan;

	// 网络序
	misc_client_t *clients;
    
    cl_rfdev_status_t rf_stat;
	rfdev_status_t *rfdev;
    
} dev_info_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 


#endif

