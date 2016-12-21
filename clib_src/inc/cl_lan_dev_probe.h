#ifndef	__CL_LAN_DEV_PROBE_H__
#define	__CL_LAN_DEV_PROBE_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "client_lib.h"

//设备当前运行模式
typedef enum enum_dev_run_mode{
    DR_MODE_SLAVE, //从设备
    DR_MODE_MASTER, //主设备模式
    DR_MODE_INDPD  //独立模式、仅摄像头
}dev_run_mode_t;

//局域网扫描tlv信息，是否支持联动，homeid是多少?
typedef struct {
	u_int8_t is_valid;//是否有效，为1时后面数据才有效
	u_int8_t is_la_new; //是否为新设备
	u_int8_t back_is_la_new;
	u_int32_t home_id;
}cl_la_lan_info_t;
    
typedef struct _cl_lan_dev_info_s{
    cl_handle_t handle; //device unique handle
    u_int64_t dev_sn; //设备序列号
    u_int32_t dev_type; //IJ_XXX
    u_int32_t last_alive_time; //最后一次存活时间
    u_int32_t ip_addr;//网络序ip地址
    u_int32_t sm_success_time;//一键配置成功多久了
    dev_run_mode_t dev_run_mode; //当前运行模式
    u_int8_t is_upgrading; //正在升级标志
    u_int8_t evm_is_upgrading;
    u_int8_t is_udp_ctrl; // 是否支持UDP控制
    u_int8_t exp_type;
    u_int8_t real_ext_type;
	u_int8_t developer_id[32];	// 开发者ID
	cl_la_lan_info_t la_info;//联动局域网扫描数据
}cl_lan_dev_info;
    
typedef struct _cl_lan_dev_list_s{
    u_int32_t dev_count;
    cl_lan_dev_info info[0];
}cl_lan_dev_list;
    
// 局域网设备扫描 event
enum {
    LDPE_BEGIN = 900,
    LDPE_DEVICE_CHANGED,
    LDPE_DEV_AUTH_OK,
    LDPE_DEV_AUTH_FAIL,
    LDPE_WIFI_CFG_OK,
    LDPE_WIFI_CFG_FAIL,
    LDPE_TIMEOUT_FAIL,
    LDPE_PHONE_CONFIG_DEV_OK,
    LDPE_END = LDPE_BEGIN + 99
};

CLIB_API RS cl_set_probe_callback(cl_callback_t callback, void *handle);

CLIB_API RS cl_get_probe_dev_list(cl_lan_dev_list** list);
    
CLIB_API void cl_free_probe_dev_list(cl_lan_dev_list* list);

CLIB_API RS cl_reset_probe_dev_list(void);

/*
	功能：
		一键配置设备认证
	输入参数:
		@dev: 设备信息
		@pwd: 设备密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知:	LDPE_DEV_AUTH_OK LDPE_DEV_AUTH_FAIL	
*/
CLIB_API RS cl_dev_auth(cl_lan_dev_info *dev, const char *pwd);

/*
	功能：
		一键配置设备上网wifi参数
	输入参数:
		@dev: 设备信息
		@ssid: wifi ssid
		@pwd: wifi密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知:	LDPE_WIFI_CFG_OK LDPE_WIFI_CFG_FAIL	
*/
CLIB_API RS cl_wifi_config(cl_lan_dev_info *dev, const char *ssid, const char *pwd);

/*
	功能：
		获取手机热点一键配置设备的目的sn
	输入参数:
		无
	输出参数:
		无
	返回：
	   手机做为热点时的目的sn
*/
CLIB_API u_int64_t cl_get_ap_dest_sn();
    
#ifdef __cplusplus
}
#endif 


#endif



