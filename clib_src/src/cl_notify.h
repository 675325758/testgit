#ifndef	__CL_NOTIFY_H__
#define	__CL_NOTIFY_H__

#ifdef __cplusplus
extern "C" {
#endif 

/**
805 配置
*/
#define ACT_805_SCREEN_CTRL 	0x0
#define ACT_805_BEEP_CTRL		0x1
#define ACT_805_SCREEN_QUERY   0x2
#define ACT_805_BEEP_QUERY   0x3

// 等待应答
#define	CLNPF_ACK	BIT(0)

// client libary notify envent
#define	CLNE_STOP	0
#define	CLNE_WAKEUP	1 /* 唤醒 */
#define	CLNE_RESOLV_DISP_SERVER	2 /* 解析分配服务器的域名 */
#define	CLNE_RESOLV_CUSTOM	3 /* gethostname不是线程安全，统一由一个线程解析 */
#define   CLNE_CLEAR_DNS_CACHE 4
#define CLNE_RESOLV_OEM_DONAME 5/*oem域名解析*/
#define CLNE_DNS_CLEAN	6 /*dns清除*/

#define	CLNE_USER_START	100
#define	CLNE_USER_LOGIN	101 /* 添加或修改用户 */
#define	CLNE_USER_LOGOUT	102 /* 删除用户 */
#define	CLNE_USER_MODIFY_PASSWD	103 /* 修改用户密码 */
#define	CLNE_USER_MODIFY_NICKNAME	104 /* 修改用户昵称 */
#define	CLNE_SLAVE_MODIFY_NAME	105	/* 从设备修改名字 */
#define	CLNE_SLAVE_BIND	106 /* 绑定从设备 */
#define	CLNE_SLAVE_UNBIND	107	/* 解绑定从设备 */
#define CLNE_USER_ADD_ALARM_PHONE 108
#define CLNE_USER_DEL_ALARM_PHONE 109
#define	CLNE_USER_ADD_DEV	110 /* 手机用户添加设备 */
#define	CLNE_USER_DEL_DEV	111 /* 手机用户删除设备 */
#define	CLNE_USER_REGISTER	112 /* 注册一个新的手机用户 */
#define	CLNE_USER_RESET	113 /* 重置手机用户的密码 */
#define	CLNE_USER_SEND_VCODE	114 /* 手机用户发送验证码 */
#define	CLNE_USER_BIND_PHONE_Q 115 /* 提交绑定手机申请 */
#define	CLNE_USER_BIND_PHONE_REQUEST_LIST 116 /*查询绑定申请列表*/
#define	CLNE_USER_BIND_PHONE_LIST 117 /*查询已绑定列表*/
#define	CLNE_USER_BIND_PHONE_NORMAL 118 /*配置普通用户登录权限*/
#define	CLNE_USER_BIND_PHONE_OPERATION 119 /*对绑定申请进行批准、拒绝操作*/
#define	CLNE_USER_MODIFY_LOGIN	120 /* 添加或修改用户 */
#define	CLNE_APNS_CONFIG 121 /*配置苹果推送服务*/
#define	CLNE_SLAVE_QUERY_STAT	122 /* 查询统计信息 */
#define	CLNE_SLAVE_OPEN_TELNET	123	/* 打开telnet */
#define	CLNE_SLAVE_REBOOT	124	/* 重启远程设备 */
#define	CLNE_USER_SET_DIRECT_IP 125 /* 设置直接登录的IP */
#define	CLNE_USER_RELOGIN 126 /* 重新登录 */
#define	CLNE_DEV_UPGRADE_Q 127 /* 查询设备升级包版本 */
#define	CLNE_DEV_UPGRADE_SET 128 /* 告诉服务器允许设备升级 */
#define	CLNE_DEV_UPGRADE_NOW 129 /* 告诉设备立即升级 */
#define	CLNE_DEV_UPGRADE_CLI 130 /* 客户端直接发送升级包到设备升级 */
#define   CLNE_DEV_805_CONFIG 131
#define	CLNE_DEV_UPGRADE_STM 132 /* 客户端直接发送单片机升级包到设备升级 */
#define 	CLNE_DEV_ACTIVE	133	/*激活*/
#define CLNE_QR_CODE_LOGIN 134 //二维码方式添加
#define CLNE_USER_ADD_QR_DEV 135 //手机账号用二维码方式添加设备

#define	CLNE_DEV_UPGRADE_EVM 136 /* 虚拟机升级相关 */
#define	CLNE_DEV_UPGRADE_CLI_NO_HEARD 137 /* 客户端直接发送升级包到设备升级 */
#define	CLNE_APNS_CONFIG_BY_SN 138 /*配置苹果推送服务，只有SN和配置，不带handle */

#define	CLNE_DEV_UPGRADE_FLASH_ERASE 139 /*flash erase*/
#define CLNE_DEV_UPGRADE_FLASH_UPGRADE 140

#define CLNE_APP_PKT_SEND	141	// 二次开发通道的发送命令

#define CLNE_DEV_STM_SPE_UP	142 //网关指定升级

#define	CLNE_USER_END	150

#define	CLNE_CMT_ADD_DEVICE	152 /* 小区添加设备 */
#define	CLNE_CMT_DEL_DEVICE	153 /* 小区删除设备 */
#define	CLNE_CMT_QUERY_DEVICE	154 /* 查询小区设备列表 */
#define	CLNE_CMT_PUBLISH_MSG		155 /*小区发布电子公告 */

#define	CLNE_VIDEO_START	200
#define	CLNE_VIDEO_STOP	201
#define	CLNE_VIDEO_ROLL	202 /* 云台旋转 */
#define	CLNE_VIDEO_QULITY	203 /* 视频质量 */
#define	CLNE_VIDEO_CAL_STAT	204 /* 计算统计 */
#define	CLNE_VIDEO_FLIP	205 /* 图像翻转 */
#define	CLNE_VIDEO_GET_H264	210 /* 收到H264图片 */
#define	CLNE_VIDEO_DECODE_H264	211 /* 成功解码出一张图片 */
#define	CLNE_AUDIO_REQUEST_SPEEK	220 /* 请求发言权 */
#define	CLNE_AUDIO_RELEASE_SPEEK	221 /* 释放发言权 */
#define	CLNE_AUDIO_PUT_SOUND	222	/* 发送声音数据 */
#define CLNE_VIDEO_SET_V4L2     223 /*设置视频饱和度参数*/
#define CLNE_VIDEO_SET_ROLL_SPEED     224 /*设置视频饱和度参数*/

#define	CLNE_VRT_SET_ON	230	/* 切换该摄像头录像的总开关 */
#define	CLNE_VRT_ADD	231		/* 添加一条录像定时器 */
#define	CLNE_VRT_MODIFY	232		/* 修改一条录像定时器 */
#define	CLNE_VRT_DEL	233		/* 删除一条录像定时器 */
#define CLNE_VTAP_QUERY_LIST     234 /*获取录像列表*/
#define	CLNE_VTAP_START     235		/* 开始看单个录像文件*/
#define	CLNE_VTAP_STOP     236		/* 停止看录像*/
#define	CLNE_VIDEO_MAX	299

#define	CLNE_PLUG_START	300
#define	CLNE_PLUG_QUERY_START	301
#define	CLNE_PLUG_QUERY_STOP	302
#define	CLNE_PLUG_SET_ON	303
#define	CLNE_PLUG_SET_TIMER	304
#define	CLNE_PLUG_DEL_TIMER	305
#define	CLNE_PLUG_ELECTRIC_STAT_CLS	306
#define	CLNE_PLUG_MAX	399
    
#define CLNE_NOTIFY_PUSH_START 500
#define CLNE_NOTIFY_PUSH_SET_EXPECT_ID 501
#define CLNE_NOTIFY_PUSH_MAX 599

//电器
#define CLNE_EQ_START 600
#define CLNE_EQ_ADD   601  //添加电器
#define CLNE_EQ_DEL   602  //删除电器
#define CLNE_EQ_CH_NAME 603 //修改电器名称
#define CLNE_EQ_CH_AREA 604 //修改电器区域
#define CLNE_EQ_ALARM_PHONE 605 //报警器电话关联
#define CLNE_EQ_ALARM_ENABLE 606 //报警器开关
#define CLNE_EQ_ALARM_SET_BIND_001E 607 //关联001E
#define CLNE_EQ_DB_RF_SCAN 608 //扫描双向RF电器
#define CLNE_EQ_ADD_BY_JSON   609  //添加电器
#define	CLNE_EQ_BIND_SOUNDLIGHT	610 /* 电器关联声光报警器 */
#define	CLNE_EQ_LINKAGE_SCENE_SET	611	/* 设置电器联动场景 */
#define	CLNE_EQ_LINKAGE_SCENE_DEL	612 /* 删除电器联动场景 */
#define	CLNE_EQ_ALARM_PUSH_ENABLE	613
#define	CLNE_EQ_ALARM_SMS_ENABLE		614
#define	CLNE_EQ_REFRESH				615 /*刷新电器*/
#define	CLNE_EQ_RF_REPEATER_SET		616 /*设置RF中续模式*/


//按键
#define CLNE_EKEY_ADD   630 //添加按键
#define CLNE_EKEY_DEL   631 //删除按键
#define CLNE_EKEY_CH_NAME 632 //修改按键名称
#define CLNE_EKEY_CTRL    633 //按键控制
#define CLNE_EQ_SET_DIMMING_LAMP    634 /*设置双向调光灯数值*/

//按键学习
#define CLNE_KL_SET_CALLBACK 650
#define CLNE_KL_START_LEARN        651 //开始学习
#define CLNE_KL_TRY         652 //尝试
#define CLNE_KL_GEN_CODE    653 //对码之产生编码
#define CLNE_KL_AJUST_CODE  654  //微调
#define CLNE_KL_SET_PW    655  //设置脉宽
#define CLNE_KL_GET_PW      656  //获取脉宽
#define CLNE_KL_SAVE_CODE   657  //保存
#define CLNE_KL_STOP_LEARN  658  //停止
#define CLNE_EQ_END   699

//区域
#define CLNE_AREA_START         700
#define CLNE_AREA_ADD           701 //添加区域
#define CLNE_AREA_DEL           702 //删除区域
#define CLNE_AREA_CHANGE_NAME   703 //修改名称
#define CLNE_AREA_MODIDY_S      704 //区域修改
#define CLNE_AREA_CHANGE_EQ     705 //电器或从设备修改区域
#define CLNE_AREA_CHANGE_IMG    706
#define CLNE_AREA_MODIFY    707
#define CLNE_AREA_END           799

//情景模式
#define CLNE_SCENE_START         800
#define CLNE_SCENE_ADD           801 //添加情景模式
#define CLNE_SCENE_DEL           802 //删除情景模式
#define CLNE_SCENE_CHANGE_NAME   803 //修改情景模式名称
#define CLNE_SCENE_MODIDY_EVENT  804 //修改事件
#define CLNE_SCENE_CHANGE_IMG    805 
#define CLNE_SCENE_MODIDY  806 /* 修改名称、图片、事件 */
#define CLNE_SCENE_EXEC          807
#define	CLNE_SCENE_TIMER_ADD	808
#define	CLNE_SCENE_TIMER_MODIFY	809
#define	CLNE_SCENE_TIMER_DEL	810
#define CLNE_SCENE_END           899
    
//局域网设备扫描
#define CLNE_LAN_DEV_PROBE_START 900
#define CLNE_LDP_SET_CALLBACK 901
#define CLNE_LAN_AUTH 902
#define CLNE_LAN_WIFI_CFG 903
#define	CLNE_START_SMART_CONFIG	910
#define	CLNE_STOP_SMART_CONFIG	911
#define CLNE_START_6621_SMART_CONFIG 912
#define CLNE_START_MBROADCAST_SMART_CONFIG 913
#define CLNE_START_PHONE_WLAN_CONFIG 915
#define CLNE_NETWORK_CHANGED 916
#define CLNE_START_MBROADCAST_SMART_CONFIG_SLOWLY 917
#define CLNE_START_SMART_CONFIG_EXT	918
#define CLNE_START_ADVANCE_CONF	919
#define CLNE_LAN_DEV_PROBE_END 999

// HTTP Get/Post结果通知
#define	CLNE_HTTP_START	1000
#define	CLNE_HTTP_IGNORE	CLNE_HTTP_START
#define	CLNE_HTTP_PU_REGISTER	1001 /* register phone user  */
#define	CLNE_HTTP_PU_RESET_PASSWD	1002 /* reset phone user password  */
#define	CLNE_HTTP_PU_LOGIN	1003 /* phone user login */
#define	CLNE_HTTP_PU_GET_DEV	1004 /* phone user get bind device list */
#define	CLNE_HTTP_PU_ADD_DEV	1005 /* phone user add device */
#define	CLNE_HTTP_PU_DEL_DEV	1006 /* phone user dev device */
#define	CLNE_HTTP_PU_GET_DICT	1007 /* phone user get 字典 */
#define	CLNE_HTTP_PU_PUT_DICT	1008 /* phone user put 字典 */
#define	CLNE_HTTP_PU_SEND_VCODE	1009 /* phone user get 字典 */
#define	CLNE_HTTP_PU_MODIFY_PASSWD	1010 /* modify phone user password */
#define	CLNE_HTTP_PU_PUT_DICT_MODIFY_PASSWD	1011 /* phone user put 字典 */
#define	CLNE_HTTP_PU_GET_DEV_TYPE	1012 /* phone user get device type */
#define	CLNE_HTTP_PU_DICT_ADD_DEV	1013 /* phone user put 字典 - add device */
#define	CLNE_HTTP_PU_DICT_DEL_DEV	1014 /* phone user put 字典 - del device */
#define	CLNE_HTTP_ENV_GET_WEATHER	1020 /* 获取天气 */
#define	CLNE_HTTP_ENV_GET_PM25	1021 /* 获取PM2.5 */
#define	CLNE_HTTP_ENV_GET_CITY_LIST	1022 /* 获取城市列表 */
#define	CLNE_HTTP_ENV_SET_CITY	1023 /* 设置城市 */
#define	CLNE_HTTP_ANPS_CONFIG	1024 /* 手机账号配置苹果推送 */
#define	CLNE_HTTP_MSG_PUSH		1025 /*手机账号收到推送消息*/
#define	CLNE_HTTP_DEV_VERSION	1026 /*查询设备版本号*/
#define	CLNE_HTTP_ENV_GET_SUGGEST	1027	/* 获取室内环境建议 */
#define	CLNE_HTTP_RFDEV_VERSION	1028 /*查询设备版本号*/
#define	CLNE_HTTP_ANPS_CONFIG_V2	1029 /* 手机账号配置苹果推送 */
#define	CLNE_HTTP_ANPS_CONFIG_SN	1030 /* 手机账号配置苹果推送 */
#define CLNE_HTTP_RFDEV_UP_CHECK 	1031 /*网关指定sn升级啥的*/

#define	CLNE_HTTP_END	1099

//智能家居单品
#define CLNE_IA_START 	1100  
#define CLNE_IA_CTRL	1101  /* 控制状态 */
#define CLNE_IA_END		1199 

//健康
#define CLNE_FAMILY_LIST_Q		1200 /* 查询家庭成员列表 */
#define CLNE_FAMILY_CONFIG		1201 /* 配置家庭成员 */
#define CLNE_MESSURE_Q		1202 /* 查询测量结果 */
#define CLNE_MESSURE_DEL		1223 /* 删除测量结果 */

//智能空调单品
#define CLNE_SA_START           1300
#define CLNE_SA_AIR_CTRL_STAT   1301
#define CLNE_SA_AIR_CTRL_POWER  1302
#define CLNE_SA_AIR_CTRL_MODE   1303
#define CLNE_SA_AIR_CTRL_WIND   1304
#define CLNE_SA_AIR_CTRL_DIRECT 1305
#define CLNE_SA_AIR_CTRL_LED    1306
#define CLNE_SA_AIR_PEAK_PERIOD 1307
#define CLNE_SA_AIR_PEAK_PRICE  1308
#define CLNE_SA_AIR_VALLEY_PRICE    1309
#define CLNE_SA_AIR_MODIFY_TIMER   1310
#define CLNE_SA_AIR_DEL_TIMER   1311
#define CLNE_SA_AIR_CTRL_TEMP   1312
#define CLNE_SA_AIR_VALLEY_PERIOD 1313
#define CLNE_SA_CODE_MATCH_CTRL     1314
#define CLNE_SA_SMART_CTRL      1315
#define CLNE_SA_AIR_FLAT_PRICE    1316
#define CLNE_SA_AIR_POWER_START     1317
#define CLNE_SA_AIR_REFRESH_TIMER     1318
#define CLNE_SA_AIR_CTRL_OLD_AIR		1319
#define CLNE_SA_AIR_REFRESH_ELEC	1320
#define CLNE_SA_AIR_CLEAR_ELEC		1321
#define CLNE_SA_AIR_LED_COLOR  1322
#define CLNE_SA_TRANS_SEND  1323
#define CLNE_SA_RESTORE_FACTORY  1324
#define CLNE_SA_PT_SET_ADKB 1325
#define CLNE_SA_MODIFY_PERIOD_TIMER 1326
#define CLNE_SA_PERIOD_DEL_TIMER   1327
#define CLNE_SA_KEY_LEARN   1328
#define CLNE_SA_SEND_LICENSE 1329
#define CLNE_SA_PT_SET_ADJ 1330
#define CLNE_SA_ISC_SETON 1331
#define CLNE_SA_TEMP_CURVE 1332
#define CLNE_SA_SET_TEMP_CTRL 1333
#define CLNE_SA_AJUST_FAN_SPEED 1334
#define CLNE_SA_AJUST_FAN       1335
#define CLNE_SA_SET_CHILD_LOCK  1336
#define CLNE_SA_SET_I8_MSG_CONFIG 1337
#define CLNE_SA_PT_SET_ADJ_EXT 1338

#define CLNE_SA_END             1399

//联创暖风机单品
#define CLNE_AH_START 1400
#define CLNE_AH_POWER 1401
#define CLNE_AH_TEMP 1402
#define CLNE_AH_TIMER 1403
#define CLNE_AH_SHAKE 1404
#define CLNE_AH_ECO 1405
#define CLNE_AH_MODE 1406
#define CLNE_AH_REFRESH_POWER 1407
#define CLNE_AH_TEST    1408
#define CLNE_AH_END 1499


//智能转发器
#define CLNE_IF_START 1500
#define CLNE_IF_QUERY 1501
#define CLNE_IF_END 1599 

//云空调
#define CLNE_CA_START 1600
#define CLNE_CA_MATCH 1601
#define CLNE_CA_CTRL 1602
#define CLNE_CA_SET_MATCH_ID 1603

#define CLNE_CA_END 1699 

//联创暖风机单品
#define CLNE_EB_START 1700
#define CLNE_EB_POWER 1701
#define CLNE_EB_TIMER_SET 1702
#define CLNE_EB_TIMER_DEL 1703
#define CLNE_EB_END 1799

#define CLNE_COMMON_UDP_START 1800
#define CLNE_COMMON_UDP_TIMER_CTRL 1801
#define CLNE_COMMON_UDP_COMMON_CTRL 1802
#define CLNE_COMMON_UDP_JCX_POWER_CTRL 1803
#define CLNE_COMMON_UDP_TB_HEATER_CTRL 1804
#define CLNE_COMMON_UDP_YL_AC_CTRL 1805
#define CLNE_COMMON_UDP_LEDE_LAMP_CTRL 1806
#define CLNE_COMMON_UDP_JNB_DEVICE 1807
#define CLNE_COMMON_UDP_AMT_FAN_CTRL 1808
#define CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL 1809
#define CLNE_UASC_SERVER_CTRL	1810
#define CLNE_COMMON_UDP_HX_CTRL 1811
#define CLNE_COMMON_UDP_TL_CTRL 1812
#define CLNE_COMMON_UDP_QPCP_CTRL	1813
#define CLNE_COMMON_UDP_QP_POT_CTRL 1814
#define CLNE_COMMON_UDP_STB_TV_CTRL 1815
#define CLNE_COMMON_UDP_CAR 1816
#define CLNE_COMMON_UDP_EPLUG_OEM 1817
#define CLNE_COMMON_UDP_THERMOSTAT_XY 1818
#define CLNE_COMMON_UDP_BIMAR   1819
#define CLNE_COMMON_UDP_TBB		1820
#define CLNE_COMMON_UDP_QP_PBJ  1821
#define CLNE_COMMON_UDP_HX_YSH  1822
#define CLNE_COMMON_UDP_YT		1823
#define CLNE_COMMON_UDP_ZH_JL_LAMP 1824
#define CLNE_COMMON_UDP_ADS		1825
#define CLNE_COMMON_UDP_JS_WAVE	1826
#define CLNE_COMMON_UDP_KXM		1827
#define CLNE_COMMON_UDP_KXM_THER 1828
#define CLNE_COMMON_UDP_SBT_THER 1829
#define CLNE_COMMON_UDP_ZSSX	1830
//#define CLNE_COMMON_UDP_YUYUAN_CTRL 1831
#define CLNE_COMMON_UDP_YJ_HEATER 1832
#define CLNE_COMMON_UDP_INDIACAR 1833
//#define CLNE_COMMON_UDP_EVM_TT 1834
#define CLNE_COMMON_UDP_ZKRSQ 1835
#define CLNE_COMMON_UDP_EVM_DEVICE 1836
#define CLNE_COMMON_UDP_LINKON 1837
#define CLNE_RFGW_SCM_CTRL 	1838
#define CLNE_COMMON_UDP_ZHCL 1839
#define CLNE_COMMON_UDP_LUM	 1840
#define CLNE_COMMON_UDP_LEIS	 1841
#define CLNE_COMMON_UDP_YINSU	 1842
#define CLNE_COMMON_UDP_ZHDHX	1843

#define  CLNE_COMMON_UDP_END 1899

#define CLNE_RFGW_START 1900
#define CLNE_RFGW_GATEWAY       1901
#define CLNE_RF_LIGHT       1902
#define CLNE_RF_DOOR_LOCK   1903
#define CLNE_RF_LAMP    1904
#define CLNE_RF_COM_DEV 1905 //通用控制指令
    
#define CLNE_RFGW_END   1999



//通用控制指令
#define CLNE_MISC_START		2000
#define CLNE_COMM_TIMER_ADD	2001
#define CLNE_COMM_TIMER_DEL	2002
#define CLNE_COMM_SMART_ON  2003
#define CLNE_COMM_CHILD_LOCK 2004
#define CLNE_COMM_SHORTCUTS_ONOFF 2005
#define CLNE_COMM_TEMP_CTRL 2006
#define CLNE_COMM_TEMP_CURVE 2007
#define CLNE_COMM_TMP_24HOUR_LINE_IMPORT 2008
#define CLNE_COMM_HUMI_24HOUR_LINE_IMPORT 2009
#define CLNE_COMM_IRID_SET	2010
#define CLNE_COMM_TEMP_ALARM 2011
#define CLNE_COMM_HISTORY_QUERY 2012
#define CLNE_COMM_BOOT_TEMP 2013
#define CLNE_COMM_TIMEZONE_SET 2014
#define CLNE_COMM_SCC_ONOFF_SET 2015
#define CLNE_COMM_STM_ERASE 2016
#define CLNE_COMM_WAN_CONFIG 2017
#define CLNE_COMM_DHCP_CONFIG 2018
#define CLNE_COMM_AP_CONFIG 2019
#define CLNE_COMM_REPEAT_CONFIG 2020
#define CLNE_COMM_DEBUG_CONF 2021
#define CLNE_COMM_DAYS_ELE_IMPORT 2022



#define CLNE_MISC_END		2099


//联动相关控制指令
#define CLNE_LA_START 2100
#define CLNE_LA_CTRL 2101
#define CLNE_LA_PHONE_CTRL 2102
#define CLNE_LA_COMM_CTRL 2103


#define CLNE_LA_END 2199

/// packet type define

#pragma pack(push,1)

typedef struct {
	cl_handle_t user_handle;
	int ns;
	ia_status_t ia_stat[0];
} cln_ia_t;


typedef struct cl_notify_pkt_s {
	// 事件类型，CLNE_XXX
	u_int32_t type;
	// 请求ID
	u_int32_t request_id;
	// CLNPF_XXX
	u_int16_t flags;
	// 头部长度
	u_int16_t hdr_len;
	// 参数长度
	u_int32_t param_len;
	// 处理结果，0为成功，其它为失败
	int32_t err_code;
	u_int8_t data[0];
} cl_notify_pkt_t;

typedef struct {
	cl_handle_t user_handle;
	int len;
	u_int16_t len_name;
	u_int16_t len_passwd;
	u_int16_t len_license;
	u_int32_t ip;
	u_int16_t port;
	u_int16_t pad;
	u_int32_t num;
	bool b_proc;
	cl_callback_t callback;
	void *callback_handle;
	char data[0];
} cln_user_t;

enum{
	ROLL_TYPE_ONCE,
	ROLL_TYPE_START,
	ROLL_TYPE_STOP,	
};

typedef struct {
	cl_handle_t slave_halde;
	cl_callback_t callback;
	void *callback_handle;
	union {
		struct {
			int left_right;
			int up_down;
			int roll_type;
		} roll;
		cl_video_quality_t qulity;
		cl_sound_data_t sound_data;
        cl_video_saturation_t vs;
        net_vtap_list_query_t query;
        struct {
            u_int32_t begin_time;
            u_int16_t offset_sec;
        }vtap_req;
        int roll_speed; //暂时先这样，后续添加参数时形成数据结构
	} u;
} cln_video_t;

// video record timer info
typedef struct {
	cl_handle_t slave_halde;
	cl_callback_t callback;
	void *callback_handle;
	// 使能/禁止录像功能时用
	u_int8_t on;
	// 删除定时录像时用
	u_int8_t id;
	u_int8_t pad[2];
	void *timer;
} cln_vrt_t;

typedef struct{
	cl_handle_t user_handle;
	u_int64_t sn;
	u_int8_t phone[128];
	u_int8_t name[64];
}cln_device_t;

typedef struct {
	cl_handle_t slave_halde;
	cl_callback_t callback;
	void *callback_handle;
	u_int32_t query_seconds;
	u_int32_t index;
	u_int8_t on;
	u_int8_t pad;
	u_int16_t id;
	void *timer;
} cln_plug_t;
    
typedef struct {
    cl_handle_t user_haldle;
    cl_callback_t callback;
    void *callback_handle;
    u_int64_t dev_sn;		
    u_int64_t expect_report_id;
}cln_notify_push_t;

typedef struct {
	cl_handle_t slave_handle;
	cl_handle_t eq_handle;
	cl_handle_t user_handle;
	cl_handle_t scene_handle; /* 联动的情景 */
	cl_handle_t* add_handle;
	cl_handle_t s_001e_handle;
	cl_handle_t area_handle;
	char name[64];
	u_int8_t group_num;/*灯或者排插路数*/
	u_int8_t eq_type;
	u_int8_t is_more_ctrl;
	u_int8_t enableAlarm;
	u_int8_t numofphone;
	u_int8_t soundline_num;
	u_int8_t soundline_on;
	u_int8_t rf_repeater_on;
    u_int8_t db_dimming_value;
	char** phonelist;
	char *json;
	cl_handle_t *soundline;
}cln_equipment_t;
    
typedef struct {
    cl_handle_t eq_handle;
    u_int32_t key_id;
    char key_name[64];
    u_int8_t db_dimm_value;
}cln_key_info_t;
    
typedef struct {
    cl_handle_t user_handle;
    cl_handle_t eq_handle;
    u_int32_t key_id;
	cl_callback_t callback;
    void *callback_handle;
    u_int8_t learn_mode;
    int  ajust_value;
    int  ajust_pw_value;
}cln_key_learn_t;

typedef struct {
    cl_handle_t area_hand;
    cl_handle_t* req_hand;
    cl_handle_t user_hand;
    char name[64];
    u_int8_t img_id;
    u_int8_t item_count;
    u_int8_t pad[2];
    cl_handle_t eq_hands[0];
}cln_area_t;
    
typedef struct {
	cl_handle_t scene_hand;
	cl_handle_t* req_hand;
	cl_handle_t user_hand;
	char name[64];
	u_int8_t img_id;
	u_int8_t item_count;
	u_int8_t pad[2];
	void *timer; /* cl_scene_timer_t */
	void* events[0]; //需要强制转换才能用
} cln_scene_t;

typedef struct {
	cl_handle_t handle;
	// 结果是否成功。0表示成功
	int result;
	// 请求的链接
	char *url;
	// 返回的结果
	char *page;
	u_int8_t index;
} cln_http_t;

typedef struct {
    cl_callback_t callback;
    void *handle;
}cln_lan_dev_probe_t;

typedef struct {
	u_int64_t dev_sn;
	char md5_pwd[16];
}cln_lan_dev_auth_t;

typedef struct{
	u_int64_t dev_sn;
	u_int16_t len_ssid;
	u_int16_t len_pwd;
	/*data : ssid + password*/
	char data[0];
}cln_lan_wifi_cfg_t;

typedef struct{
	cl_handle_t user_handle;
	char phone_number[16]; /*手机号码*/
	char phone_model[32]; /*手机型号*/
	char bind_name[16];/*绑定名称*/
	char bind_uuid[40]; /*绑定uuid*/
	char bind_message[40];/*绑定描述*/
}cln_bind_phone_t;

typedef struct{
	cl_handle_t user_handle;
	int allow;
}cln_bind_phone_normal_allow_t;

typedef struct{
	cl_handle_t user_handle;
	int reserved;
}cln_bind_phone_query_t;

 typedef struct{
 	cl_handle_t user_handle;
	char action;
	char reserved[3];
	char request_uuid[40];
 }cln_bind_phone_operation_t;

 typedef struct{
 	cl_handle_t user_handle;
	cl_apns_config_t cfg;
 }cln_apns_config_t;

typedef struct{
   u_int64_t sn;
   cl_apns_config_t cfg;
} cln_apns_config_v2_t;


typedef struct {
	const char *ssid;
	const char *passwd;
	int auth_mode;
	u_int8_t m_time;
	u_int8_t m_i_time;
	u_int8_t b_time;
	u_int8_t b_i_time;
} cln_smart_config_t;

typedef struct {
	u_int8_t id; //定时器ID
	u_int8_t enable; //是否启用
	u_int8_t week; // 星期几重复
	u_int8_t hour; //小时
	u_int8_t minute; //分钟
	u_int8_t onoff;
	u_int8_t is_del;
	u_int8_t repeat; // 是否是周期重复的定时器
}periodic_timer_t;

typedef struct {
    cl_handle_t dev_handle;
    u_int32_t action;
    u_int8_t led_on_off;
    u_int8_t old_key;
    u_int16_t begin_time;
    u_int16_t last_time;
    u_int32_t temp_value;
    struct {
        u_int8_t onoff;
        u_int8_t mode;
        u_int8_t temp;
        u_int8_t wind;
        u_int8_t wind_direct;
		u_int8_t key;
    } stat;
    struct {
        u_int8_t id; //定时器ID
        u_int8_t enable; //是否启用
        u_int8_t week; // 星期几重复
        u_int8_t hour; //小时
        u_int8_t minute; //分钟
        u_int8_t onoff;
	  u_int16_t duration;
    } timer_info;
    struct{
        u_int32_t time_out;
        u_int8_t mode;
    }code_match;
    char share_struct[128];
    struct{
        u_int16_t trans_len;
        u_int8_t trans_buf[1024];
    }trans_data;
}cln_sa_air_info_t;

typedef struct{//联创暖风机
    cl_handle_t dev_handle;
    u_int32_t action;
    u_int8_t action_param;
    u_int8_t pad[15];
}cln_sa_ah_info_t;

typedef struct{//misc ctrl
    cl_handle_t dev_handle;
    u_int32_t data_len;
	u_int8_t data[0];
}cln_misc_info_t;

typedef struct{//misc ctrl
    cl_handle_t handle;
	u_int32_t type;
	u_int32_t action;
    u_int32_t data_len;
	u_int8_t data[0];
}cln_la_info_t;

typedef struct {
	u_int8_t id; //定时器ID
	u_int8_t enable; //是否启用
	u_int8_t week; // 星期几重复
	u_int8_t hour; //小时
	u_int8_t minute; //分钟
	u_int8_t on_off;
} week_timer_info_t;

typedef struct{
	u_int8_t rgb;
	u_int8_t cnt;
	u_int8_t action;
	u_int8_t group_id;
	cl_handle_t handle[128];
}batch_sn_handle_t;

typedef struct{//通用简单数据传输
	cl_handle_t handle;
	u_int32_t action;
	u_int16_t single_param_len; //单个数据长度，以此判断是哪种类型
	u_int16_t param_count;
	u_int16_t data_len; /*参数长度超过本数据结构的长度,没超过请填0*/
	u_int16_t pad;
	union{
		u_int64_t u64_data;
		u_int32_t u32_data[0];
		u_int16_t u16_data[0];
		u_int8_t   u8_data[0];
		periodic_timer_t timer_info;
		cl_period_timer_t period_timer_info;
        //此处尽量不要添加数据结构，可以使用指针方式转换 cci_pointer_data
	}u;
}cln_common_info_t;

#pragma pack(pop)

extern cl_notify_pkt_t *cl_recv_notify(cl_thread_info_t *ti, int timeout, struct sockaddr_in *from);
extern RS cl_send_notify(cl_thread_info_t *ti, cl_notify_pkt_t *request);
extern int cl_send_notify_wait(cl_thread_info_t *ti, cl_notify_pkt_t *request, int request_buf_size);

/* 
	type = CLNE_xxx
*/
extern RS cl_send_notify_simple(cl_thread_info_t *thread, u_int32_t type);
extern RS cl_create_notify_sock(cl_thread_info_t *info);
extern cl_notify_pkt_t *cl_notify_pkt_new(int size, u_int32_t type, u_int16_t flags);
extern void cl_notify_pkt_free(cl_notify_pkt_t *pkt);
/*
通用简单数据发送接口
*/
extern RS cl_send_notify_common(cl_thread_info_t *thread, u_int32_t type,cln_common_info_t* ci);
extern RS cl_send_simple_data_notify(cl_thread_info_t *thread, cl_handle_t handle,u_int32_t type,u_int16_t action,u_int64_t value,int val_size);
#define cl_send_u64_notify(thread,handle,type,action,value) cl_send_simple_data_notify(thread,handle,type,action,value,sizeof(u_int64_t))
#define cl_send_u32_notify(thread,handle,type,action,value) cl_send_simple_data_notify(thread,handle,type,action,value,sizeof(u_int32_t))
#define cl_send_u16_notify(thread,handle,type,action,value) cl_send_simple_data_notify(thread,handle,type,action,value,sizeof(u_int16_t))
#define cl_send_u8_notify(thread,handle,type,action,value) cl_send_simple_data_notify(thread,handle,type,action,value,sizeof(u_int8_t))
extern RS cl_send_var_data_notify(cl_thread_info_t *thread, cl_handle_t handle,u_int32_t type,u_int16_t action,u_int8_t *value,int val_size);
extern RS cl_la_send_var_data_notify(cl_thread_info_t *thread, cl_handle_t handle,u_int32_t type,u_int16_t action,u_int8_t *value,int val_size);


#define cci_u64_data(ci) ci->u.u64_data
#define cci_u32_data(ci) ci->u.u32_data[0]
#define cci_u16_data(ci) ci->u.u16_data[0]
#define cci_u8_data(ci) ci->u.u8_data[0]
#define cci_pointer_data(ci) (void*)(&ci->u.u8_data[0])

#define is_valid_single_cci_data(ci,except_dlen) ((ci->single_param_len == except_dlen)?true:false)


#ifdef __cplusplus
}
#endif 

#endif

