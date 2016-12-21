#ifndef	__RFGW_PRIV_H__
#define	__RFGW_PRIV_H__

enum{
    ACT_RFGW_JOIN = 0,
    ACT_RFGW_JOIN_ACTION,
    ACT_RFGW_GROUP,
    ACT_RFGW_TT,
    ACT_RFGW_DEVLIST,
    ACT_RFGW_TT_BATCH,
    ACT_RFGW_DEV_DEL,
    ACT_RFGW_WORK_QUERY,
    ACT_RFGW_DEV_GROUP,
    ACT_RFGW_DEV_GROUP_MEMBER,
    ACT_RFGW_DEV_GROUP_TT,
    ACT_RFGW_DEV_NAME,
    ACT_RFGW_DEV_UPGRADE,
    ACT_RFGW_DEV_DEL_ALL,
    ACT_RFGW_DEV_COMMPAT,
    ACT_RFGW_DEV_UP_QUERY,
    ACT_RFGW_DEV_TIMER_ADD,
    ACT_RFGW_DEV_TIMER_DEL,
    ACT_RFGW_DEV_TIMER_QUERY,
    ACT_RFGW_DEV_COMM_HISTORY_QUERY,
    ACT_RFGW_DEV_DEFENSE_BATCH_CONFIG,
    ACT_RFGW_DEV_UP_CHECK,
    ACT_RFGW_DEV_IMG_CACHE_QUERY,
    ACT_RFGW_DEV_IMG_CACHE_DEL,

	ACT_RFGW_HPGW_APPINFO,
	ACT_RFGW_HPGW_SMS,
	ACT_RFGW_HPGW_CONFIG_USER,
	ACT_RFGW_HPGW_DEL_USER,
	ACT_RFGW_HPGW_LAMP_CTRL,

	ACT_RFGW_RF_RUNTIME_QUERY,

	ACT_RFGW_ONEKEY,
};

///// RF灯
enum{
    ACT_RF_LAMP_KNOWN,
    ACT_RF_LAMP_SET_COLOR,
    ACT_RF_LAMP_SET_SW_ONOFF
};
//灯控制
enum{
    ACT_RF_DOOR_LOCK_CTRL,
    ACT_RF_DOOR_LOCK_REMOTE_CTRL,
    ACT_RF_QUERY_HISTORY,
    ACT_YT_RF_DOOR_LOCK_CTRL,
    ACT_YT_RF_DOOR_LOCK_MODIFY_PASSWD,
    ACT_YT_RF_DOOR_LOCK_CREATE_PASSWD,
    ACT_RF_DOOR_LOCK_UNLOCK_TIMEOUT,
    ACT_RF_DOOR_LOCK_WIFI_LOCK,
    ACT_RF_DOOR_LOCK_SET_CONTROLLER_INFO,
    ACT_YT_RF_DOOR_LOCK_ASSOCIATE,
    
};

//
enum{
    ACT_RF_COM_NONE = 0x0,
    ACT_RF_COM_DIRECT_CTRL,
    ACT_RF_COM_AUTO_GUARD,
    ACT_RF_COM_QUERY_HISTORY,
    ACT_RF_COM_ALARM_TIME,	// 设置报警时间
    ACT_RF_COM_ALARM_CLC,	// 给设备回复收到报警了
    ACT_RF_COM_AIR_IR_CTRL, // 通用红外控制
    ACT_RF_COM_SHORTCUTS_ONOFF_QUERY,//通用快捷开关查询
    ACT_RF_COM_SHORTCUTS_ONFF_SET,//通用快捷开关设置
	ACT_RF_COM_TMP_ADJUST,//通用温度矫正
	ACT_RF_COM_LED_MODE,//通用led灯模式设置
    ACT_RF_COM_END = 50,		// 通用结束，后面自己设备字节决定
};

//每种类型是否已查询最新版本号
enum{
	UP_QUERY_IDLE = 0,
	UP_QUERY_NEED,
	UP_QUERYED,
};

//夜狼网关action
enum {
	ACT_RF_YL_ALARM_TIME = 1,
	ACT_RF_YL_VOICE,
	ACT_RF_YL_SIREN_OFF,
};

//网关大量数据命令tlv
enum {
	GW_HTL_BD_QUERY = 1,
	GW_HTL_BD_NAME_SET = 2,
	GW_HTL_BD_PID_SET = 3,
	GW_HTL_BD_ADD_LINK = 4,
	GW_HTL_BD_DEL_LINK = 5,
	GW_HTL_BD_ADDDEL_PUSH = 6,
	GW_HTL_BD_NOTICE = 7,
}GW_BIG_DATA_T;

#define	RFDEV_FLAG_ONLINE BIT(0)
#define	RFDEV_FLAG_NEW BIT(1)
#define	RFDEV_FLAG_BINDING BIT(2)
#define	RFDEV_FLAG_REBIND BIT(3)
#define RFDEV_FLAG_UPGRADING 	BIT(4)
#define RFDEV_FLAG_LINKAGE	BIT(5)
#define RFDEV_FLAG_DBC		BIT(6)//表示支持一键布防撤防
#define	RFDEV_FLAG_INVALID 	BIT(7)

typedef struct{
	struct stlc_list_head link;
	u_int32_t time;
	u_int8_t work;
}rfdev_work_t;

typedef struct _rfdev_status_s{
	struct stlc_list_head work_list;
	u_int64_t ctrl_total;
	u_int32_t ctrl_min;
	u_int32_t ctrl_max;
	u_int32_t ctrl_ok;
	u_int32_t ctrl_fail;
	u_int32_t ctrl_msec;
    
	u_int32_t rfrx; // from RF device
	u_int16_t linkretry; // from RF device
	u_int8_t work;  // from RF device
    
	u_int8_t is_ctrl;
    //stat; more dev union
}rfdev_status_t;

//缓存从设备定时器的数据结构
typedef struct {
	u_int8_t max_timer_count;
	u_int8_t max_type_count;
	u_int8_t max_data_len;
	u_int16_t support_type;
	u_int32_t map;
	u_int16_t stat_count[32];
	u_int8_t timer_count;
	u_int8_t real_count;
	cl_comm_timer_t timer[32];
}rf_comm_timer_save_t;

#pragma pack(push,1)
/////////////////SDK internal param////////////////////
typedef struct{
	u_int64_t dev_sn;
	u_int16_t accept;
}sdk_join_act_t;

typedef struct{
	u_int8_t gw_cnt;
	u_int8_t pad[3];
	u_int8_t psk[PSK_LEN];
	cl_handle_t gw_handle[MAX_GW_COUNT];	
}sdk_group_t;

typedef struct{
	struct stlc_list_head link;
	cl_rfgw_dev_find_t cl_dev;	
}sdk_dev_find_t;

typedef struct {
	u_int8_t flag;
	u_int8_t changed_num;
	u_int8_t cache_changed_num;
	u_int8_t pad;
}net_dev_list_info_t;
typedef struct {
	u_int32_t seq;
	net_dev_list_info_t dev[0];
}net_dev_list_digest_t;


typedef struct{
	struct stlc_list_head new_dev_head;
	struct stlc_list_head new_slave_head;
	// 花瓶网关
	cl_hpgw_info_t hpinfo;
	u_int8_t support_hpinfo;	// 支持花瓶网关相关配置
	
	cl_dev_group_t *devgroup[256];
	u_int8_t upgrade_status[D_T_MAX];//是否需要升级或者升级中状态
	char *upgrade_url[D_T_MAX];//升级url
	u_int32_t max_ver[D_T_MAX];//每种类型最最大的版本号,主要是为了解决那个1.4.0一下的设备不支持升级的问题。
	u_int32_t min_ver[D_T_MAX];//每种类型最小的版本号
	u_int32_t min_ver_rf[D_T_MAX];//rf主通讯框架版本
	u_int32_t min_ver_rf_stm[D_T_MAX];//rf设备下单片机版本
	u_int8_t min_ver_rf_valid[D_T_MAX];//用来判断该类型下是否有单片机存在
	u_int32_t server_ver[D_T_MAX];//服务器上保存的版本
	u_int32_t server_ver_rf[D_T_MAX];//服务器上保存的版本
	u_int32_t server_ver_rf_stm[D_T_MAX];//服务器上保存的版本
	u_int32_t server_queryed[D_T_MAX];//服务器已经查询过了
	u_int8_t upgrade_query[D_T_MAX];//是否查询过最新版本	
	u_int8_t cur_query_ext;
	u_int32_t cur_query_index;

	u_int8_t commpat;//0默认兼容级别；1~255，兼容级别，数字越大，越不兼容老版本，而使用新特性。
	u_int8_t channel;//0默认信道；1~255为信道号，2400+信道号为网关使用频率。
	u_int8_t is_upgrade;//0没有升级中，1在升级中

	u_int32_t list_len;//从设备摘要缓存长度
	net_dev_list_digest_t *plist;//从设备摘要缓存 

	u_int8_t img_cache_num;
	cl_rfdev_img_cache_info_t *pimg_cache;

	//网关透传外设数据
	cl_rfdev_scm_t rfdef_scm_dev;
}rfgw_priv_t;

typedef struct {
	u_int32_t type;
	char filepath[512];
}rfgw_upgrade_t;


///////////////param to device////////////////////////
typedef struct{
	u_int16_t timeout;
	u_int8_t reserved[2];
}net_rfgw_join_t;

typedef struct{
	u_int64_t sn;
	u_int8_t subtype;
	u_int8_t extype;
	u_int8_t reserved[2];
}net_rfgw_join_find_t;

typedef struct{
	u_int64_t sn;
	u_int8_t action;
	u_int8_t reserved[3];
}net_rfgw_join_action_t;

typedef struct{
	u_int8_t count;
	u_int8_t total;
	u_int8_t reserved[2];
	//net_rfgw_dev_t
}net_rfgw_list_t;

typedef struct{
	u_int64_t sn;
	u_int8_t subtype;
	u_int8_t extype;
	u_int8_t flags;
	u_int8_t tlv_count;
	u_int8_t tlvdata[0];
}net_rfgw_dev_t;

typedef struct{
	u_int8_t psk[PSK_LEN];
	u_int8_t gw_cnt;
	u_int8_t reserved[3];
	u_int64_t gw_sn[0];	
}net_rfgw_group_t;

typedef struct{
	u_int64_t sn; //device sn
	u_int16_t len;
	u_int8_t data[0];	
}net_rfgw_tt_t;

typedef struct{
    u_int8_t group_id;
    u_int8_t len;
    u_int8_t sub_type;
    u_int8_t ext_type;
    u_int8_t data[0];
}net_rfgw_group_tt_t;

typedef struct{
	u_int8_t rf_major;
	u_int8_t rf_minor;
	u_int8_t app_major;
	u_int8_t app_minor;
}rf_stm_ver_t;

typedef struct{
	u_int8_t stm_major;
	u_int8_t stm_minor;
	u_int8_t stm_flag;//BIT0为真表示单片机有效
	u_int8_t reserve;
}rf_stm_ver2_t;

typedef struct {
    u_int8_t type;
    u_int8_t len;
    u_int8_t value[0];
}rf_tlv_t;

#define	rf_tlv_val(tlv) ((void *)((rf_tlv_t *)tlv + 1))

typedef struct{
	u_int8_t group_id;
	u_int8_t dev_count;
	u_int8_t group_type;
	u_int8_t group_count;
	u_int8_t name[16];
	u_int64_t sn[0];
}net_dev_group_t;

typedef struct{
	u_int8_t group_id;
	u_int8_t dev_count;
	u_int8_t pad[2];
	u_int64_t sn[0];
}net_dev_group_member_t;

typedef struct{
	u_int64_t sn;
	u_int8_t name[16];
}net_dev_name_t;

typedef struct{
	u_int64_t sn;
	cl_comm_timer_t timer;
}net_dev_timer_t;

typedef struct {
	u_int64_t sn;
	u_int32_t index;
} net_dev_comm_history_query_t;

typedef struct {
	u_int8_t commpat;//0默认兼容级别；1~255，兼容级别，数字越大，越不兼容老版本，而使用新特性。
	u_int8_t channel;//0默认信道；1~255为信道号，2400+信道号为网关使用频率。
	u_int8_t is_upgrade;//0没有升级中，1在升级中
	u_int8_t support;//支持功能位
	u_int32_t reserve[7];
}net_dev_param_t;

typedef struct {
    u_int8_t err;
    u_int8_t pad;
    u_int16_t ir_id;
    u_int16_t len;
    u_int16_t pad1;
    u_int32_t pos;
    u_int8_t data[0];
}ucp_ir_code_data2_t;

#define IR_CODE_BYTE_PER_PKT 1200

typedef struct {
    u_int16_t ir_id;
    u_int16_t len;
    u_int32_t pos;
}ucp_ir_code_dl_req_t;

typedef struct {
	u_int8_t num;
	u_int8_t pad[3];
	u_int64_t sn[0];
}ucp_rfgw_dbc_hd_t;

//typedef struct{
//	u_int8_t group_id;
//	u_int8_t pad;
//	u_int16_t len;
//	u_int8_t data[0];	
//}net_rfgw_group_tt_t;

#pragma pack(pop)



//红外id查询列表
typedef struct {
	struct stlc_list_head link;
	u_int16_t ir_id;
	u_int64_t user_sn;
	u_int64_t slave_sn;
}ir_ql_t;


void *init_rfgw_sdk();
void free_rfgw_sdk(void *ptr);

rfdev_status_t *rfdev_alloc();
void rfdev_free(rfdev_status_t *p);
void _cl_rfdev_free(cl_slave_t *slave);
void _cl_rfgw_free(cl_gw_info_t *info);

/*
	处理设备端过来的通知
*/
extern bool rfgw_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

/*
	处理APP下来的用户请求
*/


// APP能看见的状态数据
extern void rfgw_build_objs(user_t* user, cl_dev_info_t* ui);

extern int rfgw_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);

extern void rfgw_quick_query_info(smart_air_ctrl_t* ac);

//////////////////////////////////////////////////
extern bool rfgw_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret);

extern void rfgw_proc_gw_offline_hook(user_t *user);

extern void rfgw_send_tt_packet(slave_t* slave,void* param ,u_int16_t len);
extern void rfgw_send_tt_query_packet(slave_t* slave,void* param ,u_int16_t len);

//按固定的大小发送大报文
extern void rf_slave_send_big_pkt(slave_t* slave,u_int8_t data_type,u_int8_t* param ,u_int16_t len);
//RF 大报文分片组包
extern rf_b_pkt_defrag_t* rf_slave_recv_big_pkt(slave_t* slave, rf_tlv_t* tlv);
extern bool rf_slave_do_big_pkt_ret(slave_t* slave,rf_tlv_t* tlv);
// RF 处理分片获取
extern void rf_slave_proc_frame_get(slave_t* slave, rf_tlv_t* tlv);

extern const unsigned char wk_ir_detail[];
extern const unsigned char wk_ir_info[];

extern bool rf_slave_send_big_pkt_v2(slave_t* slave,u_int8_t data_type,u_int8_t* param ,u_int16_t len, rbpi_callback finish_func);
extern bool rf_slave_recv_big_pkt_v2(slave_t* slave, u_int8_t data_type, rbpi_callback finish_func);
extern void rbmp_callback(slave_t *slave, u_int8_t data_type, u_int8_t errcode);

//红外学习超时30s
#define AIR_IR_CODE_LEARN_TIMEOUT		(30)
//红外学习命令重发定时器
#define AIR_IR_RESEND_TIME	(3)

enum{
    MS_WK_IDLE, //空闲
    MS_WK_WAIT_DEV_START, //等待设备启动学习
    MS_WK_DEV_WAIT_SING, //等待用户按红外遥控器
    MS_WK_SERVER_MATCH, // 等待服务器匹配
    MS_WK_PHONE_DOWN_SING, // 下载红外编码
    MS_WK_NOTIFY_DEV_CODE_ID //将红外数据库信息传给设备
};
//RF 空调贴编码学习
extern void air_ir_code_match_set_status(slave_t* slave,u_int32_t status);

extern void air_ir_start_to_server_match(slave_t* slave);

extern bool reset_ir_info(slave_t* slave,u_int16_t new_ir_id);

extern bool air_rf_ir_send_signal(slave_t* slave);

//////////////////////////////////////////////////////////////////
/////////////           RF终端设备处理               ///////////////
//////////////////////////////////////////////////////////////////

//终端设备更新状态
extern bool udp_rf_dev_update_date(slave_t* slave,rf_tlv_t* tlv, u_int8_t aciton);

extern bool udp_rf_dev_update_raw_date(slave_t* slave,u_int8_t* buf,u_int16_t len, bool cache);
extern bool udp_rf_dev_update_cache_date(slave_t* slave, u_int8_t action, u_int8_t* buf, u_int16_t len);
extern bool udp_rf_update_server_ia_code_subobj(slave_t* slave,u_int8_t action,ucp_obj_t* obj);
//从服务器过来的报文
extern bool udp_rf_dev_do_server_uc(slave_t* slave,ucp_ctrl_t* uc,int len);

//填充查询报文
extern u_int8_t udp_rf_dev_mk_stat_query_pkt(slave_t* slave,rf_tlv_t* tlv);
//填充查询报文
extern u_int8_t udp_rf_dev_mk_raw_stat_query_pkt(slave_t* slave,u_int8_t* buf);
//释放APP层slave结构数据
extern void udp_rf_dev_free_cl_data(cl_slave_t* slave);
//释放slave结构数据
extern void udp_rf_dev_free_slave_data(slave_t* slave);
// 处理APP接口
extern bool udp_rf_dev_proc_notify(slave_t* slave,cl_notify_pkt_t *pkt, RS *ret);
//
extern bool udp_rf_dev_bulid_slave(slave_t* slave,cl_slave_t* info);
//组操作
bool udp_rf_dev_group_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern void _slave_upgrade_check(user_t *user, smart_air_ctrl_t* air_ctrl);
extern u_int64_t _get_rf_sn_by_ext(user_t *user, rfgw_priv_t *p);
bool comm_timer_is_conflict(user_t *user, cl_comm_timer_head_t *pcthd, cl_comm_timer_t *timer);
void slave_cache_back(priv_air_ir_stat_cache* cache);
void slave_cache_revert(priv_air_ir_stat_cache* cache);
void ir_ql_init();
void ir_ql_exit();
void ir_ql_add(u_int64_t user_sn, u_int64_t slave_sn, u_int16_t ir_id);
ir_ql_t *ir_ql_find_by_id(u_int16_t ir_id);
extern bool do_comm_gw_big_data_send(slave_t *slave, u_int8_t action, u_int8_t *pdata, int len);

#endif

