#ifndef	__LAN_DEV_PROBE_PRIV_H__
#define	__LAN_DEV_PROBE_PRIV_H__

#include "cl_lan_dev_probe.h"
#include "cl_priv.h"

//报文命令类型
#define	CFGPT_RESULT	0
#define	CFGPT_DISCOVERY  1
#define	CFGPT_I_HERE  2
#define	CFGPT_AUTH		3
#define	CFGPT_KEEPLIVE	4
#define	CFGPT_SET_CONFIG	5
#define	CFGPT_GET_CONFIG	6
#define	CFGPT_GET_INFO	7
#define	CFGPT_SET_INFO	8
#define	CFGPT_FRAG		9
#define	CFGPT_UPGRADE_REQ	10
#define	CFGPT_UPGRADE_ACK	11
#define	CFGPT_SWITCH_QUERY	100
#define	CFGPT_SWITCH_STATUS	101
#define	CFGPT_SWITCH_SET		102


#define	SN_BROCAST	0

#define	IJCFG_PORT_DEV	8818 //设备监听的端口
#define	IJCFG_PORT_APP	8819 //客户端监听端口
#define DEV_SERVER_PORT 1181 //局域网直接登录端口

//注意: 这里的flags标志需要和设备端统一，增加删除请双方确认
/* CFGPF_XXX*/
#define	CFGPF_SLAVE_SUPPORT     0x01
#define	CFGPF_MASTER_SUPPORT    0x02
#define	CFGPF_INDPD_SUPPORT     0x04 //独立模式
// 正在升级
#define	CFGPF_UPGRADING		0x08
//增加是否插入了ukey的标记
#define CFGPF_UKEY_PLUGIN    0x10
// 支持UDP控制
#define	CFGPF_UDP_CTRL	0x20
//表示是否用户没修改过wan口配置
#define CFGPF_USER_NO_MODIFY	0x40


#define NO_PASSWD "no password"
#define HAS_PASSWD "password1 %s"

#define MODE_MASTER "master"
#define MODE_INDEPEND "independence"

//配置无线上网参数字符串格式
#define CFGPF_PARAM_FMT \
"!\n\
misc\n\
 mode %s\n\
!\n\
wan\n\
 router_mode\n\
 type_line 2\n\
 type dhcp\n\
!\n\
wifi\n\
 enable\n\
 wifi mode 1\n\
!\n\
ap_client\n\
 ssid1 %s\n\
 %s\n\
!\n\
end\n"

#pragma pack(push,1)

/*Device Probe statue enum*/
enum{
	PS_INIT,
	PS_AUTHING,
	PS_AUTHED,
	PS_CONFIGING,
	PS_CONFIGED,
};

typedef struct cfg_pkt_dhr_s {
    u_int64_t src_sn;   //源sn,扫描时发起方置随机数
    u_int64_t dst_sn;   //目的sn
    u_int16_t cmd;      //命令类型，CFGPT_XXX
    u_int16_t param_len;//参数长度
    u_int16_t req_id;   //事务ID
    u_int16_t flags;    //CFGPF_xxx
    u_int8_t data[0];
} cfg_pkt_dhr_t;

typedef struct cfg_pkt_i_here_s {
    u_int8_t type;          //设备类型,TP_XXX
    u_int8_t sub_type;      //设备子类型,U_XXX
    u_int16_t config_count; //配置计数，每改变一次配置就加1
    u_int8_t auth[16];      //认证随机数
    u_int16_t sys_mode;     //设备运行模式
    u_int8_t ext_type;
    u_int8_t pad;          //保留
} cfg_pkt_i_here_t;

typedef struct {
	u_int16_t cfg_cnt;
	u_int8_t enc;
	u_int8_t pad;
	u_int8_t checksum[16];
	u_int8_t config[0];
}cfg_pkt_set_config_t;

typedef struct {
	u_int16_t req_id;
	u_int16_t err_code;
} cfg_pkt_result_t;

#pragma pack(pop)

/*存储设备发现报文*/
typedef struct {
    struct stlc_list_head link;
    cl_handle_t handle;
    cl_thread_t* time_out;
    u_int8_t type;
    u_int8_t sub_type;
    u_int8_t ext_type;
	u_int8_t developer_id[32];	// 开发者ID
    u_int8_t real_ext_type;
    u_int16_t dev_run_mode;
    u_int64_t dev_sn; /*序列号*/
    u_int32_t peer_ip;/*ip*/
    u_int32_t recv_time;/*收到报文的时间*/
    u_int32_t sm_success_time;//一键配置成功多久了
    u_int8_t evm_is_upgrading;
    u_int8_t auth[16];    
    cl_thread_t *t_timer;
    char md5_pwd[16];
    char *ssid;
    char *wifi_pwd;
    u_int8_t probe_status;
    u_int8_t retry;
    u_int16_t req_id;
    u_int16_t flags;
	cl_la_lan_info_t la_info;//联动局域网扫描数据
}dev_probe_info_t;

typedef struct {
	struct stlc_list_head link;
	// 目的广播地址，网络字节序
	u_int32_t bcast_addr;
} brcdst_t;

typedef struct _lan_dev_probe_ctrl_s{
    struct stlc_list_head dev_info_list;
    cl_callback_t callback;
    void *  callback_handle;
    SOCKET udp_socket;
    u_int64_t random_sn;
    u_int64_t ap_dest_sn;
    cl_thread_t *t_probe;
    cl_thread_t *t_read;
    pkt_t*      packet;
    cl_mutex_t mutex;
    u_int8_t probe_times;
    u_int8_t is_probe_enable;//是否处于扫描状态
    u_int8_t notify_user_times;
	// brcdst_t
	struct stlc_list_head bcast_lst;
	// 最后一次获取广播地址的时间，每隔两秒再重新获取，避免太频繁
	u_int32_t last_get_bcast;
}lan_dev_probe_ctrl_t;

//
extern u_int32_t get_dev_lan_ipaddr(u_int64_t sn);
extern bool lan_dev_is_need_udp_login(u_int64_t sn);
extern void delete_lan_dev_by_sn(u_int64_t sn);
extern bool lan_dev_probe_init(cl_priv_t* cp);
extern void lan_dev_probe_exit(cl_priv_t* cp);
extern bool lan_dev_probe_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern void lan_phone_send_config_ok_to_app(u_int64_t wan_sn);
extern void lan_dev_proc_wifi_switch(int new_net_type);

#endif

