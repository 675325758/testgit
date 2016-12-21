#ifndef __DS_PROTO_HEADER__
#define __DS_PROTO_HEADER__

#ifdef CLIB_HDR
  /*公共库*/
  #include "client_lib.h"
  #pragma warning(disable:4819)
#else
  /*服务器和爱家设备*/
  #include "ds_types.h"
  #include "charge_card.h"
  #include "intelligent_route.h"
#endif

/*role type of DS007 system*/
enum {
	TP_CENTER = 0,		/*生产中心*/
	TP_DISPATCHER = 1,	/*分配服务器*/
	TP_DEV_SRV = 2,		/*设备服务器*/
	TP_WEB_SRV = 3,		/*web服务器*/
	TP_USER = 4,		/*手机或流量器用户*/
	TP_DS007 = 5,		/* DS007设备*/
	TP_CHARGE_SRV = 6,		/* 充值服务器 */
	TP_CHARGE_CLI_RO = 7,	/* 充值客户端，只读 */
	TP_CHARGE_CLI_WR = 8,	/* 充值客户端，读写  */
	TP_MAX
};

/*subtype of 007 device*/
#define IJ_TYPE_NUM 0x04

#define IJ_007 			0x0  	/*i+007*/
#define IJ_001 			0x01  	/*i+001 目前主要是i+001E无线红外转发器*/
#define IJ_002 			0x02  	/*i+002 iTV-C*/
#define IJ_003 			0x03  	/*i+003 wireless camera*/
#define IJ_006 			0x6  	/*i+006*/
#define IJ_008 			0x08  	/* i+008 iTV-C */
#define IJ_803 			0x37  	/*海斯方案摄像头*/
#define IJ_807 			0x09  	/*i+807E*/
#define IJ_FANSBOX 		0x0A 	/* fansbox*/


#define IJ_808			0x10	/*i+808插座*/

#define IJ_UDP_DEVICE_BASE IJ_808
#define IJ_UDP_DEVICE_END 0x1f
#define IS_UDP_DEV(subtype) (((subtype)>=IJ_UDP_DEVICE_BASE)&&((subtype)<=IJ_UDP_DEVICE_END))


#define IJ_OPT 			0x20 	/*openwrt刷机设备*/
#define IJ_ANDROID 		0x21  	/*Andriod刷机设备*/
#define IJ_COMMUNITY 	0x22   /* 小区服务器 */

#define IJ_AIRCONDITION 0x30	/* 智慧家电单品空调 */
#define IJ_WATERHEATER 0X31		/* 智慧家电单品热水器 */
#define IJ_AIRHEATER 0x32		/* 智慧家电单品快热炉 */
#define IJ_AIRCLEANER 0x33		/* 智慧家电单品空气净化器 */
#define IJ_ELECTRICFAN 0x34		/* 智慧家电单品电风扇 */
#define IJ_BATHROOMMASTER 0x35	/* 智慧家电单品浴霸 */
#define IJ_805		0x36		/* IJ805 */
#define	IJ_UNKNOW	0xFF /* 未知的类型 */

/*extented type of IJ_008*/
#define EX_008_SL 1
#define EX_008_SH 2
#define EX_008_EL 3
#define EX_008_EH 4

/*extented type of IJ_003*/
/*下面定义从mu定义中拷贝过来*/
//增加硬件平台版本参数
enum {
	/*  版本1:
		i+003Cam-H
		i+003Cam-M
		以及其他平台目前只有一个硬件版本,例如i+007E ...
	*/
	EX_003_CAMH = 1,
	
	/*  版本2:
		i+003Cam-Y
	*/
	EX_003_CAMY = 2,
	
	/*  版本3:
		 i+003Cam-G(toplink厂家的硬件oem版本) 
	*/
	EX_003_CAMG = 3, 
	
	/*  版本4:
		i+003Cam-H(S)(在i+003Cam-H硬件的基础上，裁剪了usb接口和tf卡接口)
	*/
	EX_003_CAMHS = 4,
	
	/*  版本5:
		i+003Cam-OG(室外枪型，兴华安科技厂家的硬件oem版本)
	*/
	EX_003_CAMOG = 5,
	
	/*  版本6:
		i+003Cam-OB(室外圆球型，兴华安科技厂家的硬件oem版本)
	*/
	EX_003_CAMOB = 6,

	/*  版本7:
		i+003Cam-YH(Cam-YS的增强版本，增加红外控制单片机)
	*/
	EX_003_CAMYH = 7,
	
	/*  版本8:
		i+003Cam-G galaxywind
	*/
	EX_003_CAMYG = 8,
	EX_003_MAX
};

/*extented type of IJ_803*/
enum {
	/*
	* i+803-hie, easyn, 海思摄像头
	*/
	EX_803_HS = 0x01,
	/*
	* i+gepc2, galaxywind, C2摄像头
	*/
	EX_803_C2 = 0x02,
	/*
	* i+gepc3 , galaxywind, C3摄像头
	*/
	EX_803_C3 = 0x03,
	EX_803_MAX,
};

#define STP_MAX 256 /*max sub type*/

enum{/*登录类型*/
	LT_NORMAL = 1, /*普通登录，未启用绑定手机*/
	LT_BIND = 2, /*绑定手机登录*/
	LT_UNBIND = 3 /*未绑定手机登录*/
};

enum{/*网络连接类型*/
	NT_SERVER = 1, /*连接服务器*/
	NT_DEVICE = 2, /*局域网直连设备，设备连接到服务器*/
	NT_DEVICE_OFFLINE = 3, /*局域网直连设备，设备未连接到服务器*/
};

enum{
	DIT_TYPE,
	DIT_NAME,
	DIT_IPHONE_NEWEST_VERSION,
	DIT_ANDRIOD_NEWEST_VERSION,
	DIT_IPHONE_EN,     /*iphone english newest version description*/
	DIT_IPHONE_CH,     /*iphone chinese newest version description*/
	DIT_ANDRIOD_EN,   /*android english newest version description*/
	DIT_ANDRIOD_CH,   /*android chinese newest version description*/
	DIT_LOGINTYPE,      /*LT_NORMAL/LT_BIND/LT_UNBIND*/
	DIT_NETTYPE,          /*NT_SERVER/NT_DEVICE*/
	DIT_MAX
};

#ifndef CLIB_HDR
enum{
	CID_IPHONE = 0,
	CID_ANDROID = 1,
	CID_SYMBIAN = 2,
	CID_PC = 3,
	CID_MAX
};
#endif


#define DEF_WEB_DOMAIN "www.jiazhang007.com"
#define DEF_DISPATCHER_DOMAIN "www.jiazhang008.com"
#define DEF_APPLE_DOMAIN "www.apple.com"

/*分配服务器服务进程端口*/
#define DEF_DISPATCHER_PORT 1180
/*设备服务器服务进程端口*/
#define DEF_DEVSERVER_PORT 1181
/*Web服务器服务进程端口*/
#define DEF_WEBSERVER_PORT 1182
/* 代理服务器UDP监听端口*/
#define DEF_AGENT_UDP_PORT	1183
/*充值服务器缺省端口*/
#define DEF_CHARGE_SRV_PORT     1184
/*设备服务器http端口*/
#define DEF_DEV_HTTP_PORT 880
/*Web服务器http端口*/
#define DEF_WEB_HTTP_PORT 80
/*DISPATCHER服务器http端口*/
#define DEF_DISPATCHER_HTTP_PORT 88
/* 接收udp的echo_q的端口 */
#define DEF_DISPATCHER_UDP_PORT	1190
/* 设备端发送udp echo_q的端口 */
#define DEF_DEV_UDP_PORT	1191
/* 通过UDP控制设备的端口号，提供给客户端的 */
#define	DFL_UDP_CTRL_CLIENT_WAN_PORT	1192
/* 服务器给设备提供服务的UDP端口 */
#define	DFL_UDP_CTRL_DEV_PORT	1193
/* 设备局域网内UDP控制的端口 */
#define	DFL_UDP_CTRL_CLIENT_LAN_PORT	1194

#ifndef CLIB_HDR
/*小区服务器udp端口，设备直连小区服务器*/
#define CMT_PORT 5362
#endif

#define DSFTP_STDIN "/dsserver/dsftp-in"
#define DSFTP_PORT 2345

#undef	BIT
#define	BIT(n)	(1 << (n))
#define DEV_LIST_TMP_FILE "/var/tmp-device-%u"

/*通信协议命令
禁止在已有命令中间插入新命令
只能顺序添加到后面*/
enum {
	CMD_OK = 0,
	CMD_FAIL = 1,
	CMD_ECHO_Q = 2,
	CMD_ECHO_A = 3,
	CMD_AUTH_Q = 4,
	CMD_AUTH_A = 5,
	CMD_AUTH_K = 6,
	CMD_EXCHANG_Q = 7,
	CMD_EXCHANG_A = 8,
	CMD_USER_BIND_Q = 9,
	CMD_USER_BIND_A = 10,
	CMD_USER_AUTH_Q = 11,
	CMD_USER_AUTH_A = 12,
	CMD_USER_AUTH_K = 13,
	CMD_SERVER_Q = 14,
	CMD_SERVER_A = 15,
	CMD_DEV_Q = 16,
	CMD_DEV_A = 17,
	CMD_URL_Q = 18,
	CMD_URL_A = 19,
	CMD_DEV_CONFIG_Q = 20,
	CMD_DEV_CONFIG_A = 21,
	CMD_DEV_STAT_Q = 22,
	CMD_DEV_STAT_A = 23,
	CMD_DEV_STAT_CYCLE = 24,
	CMD_ELECT_MASTER_Q = 25,
	CMD_ELECT_MASTER_A = 26,
	CMD_ELECT_MASTER_C = 27,
	CMD_NICKNAME = 28,
	CMD_PASSWD = 29,
	CMD_LINK_OPTION_Q = 30,
	CMD_LINK_OPTION_A = 31,
	CMD_WEB_AUTH_Q = 32,
	CMD_WEB_AUTH_A = 33,
	CMD_NICKNAME_Q = 34,
	CMD_NICKNAME_A = 35,
	CMD_FTP_Q = 36,
	CMD_FTP_A = 37,
	CMD_YW_SERVER_Q = 38, /*运维查询服务器请求*/
	CMD_KEEP_DATA = 39,
	CMD_NICKNAME_CONFIRM = 40, 	/*修改昵称成功确认命令，服务器内部使用*/
	CMD_NICKNAME_FAIL = 41,   	/*修改昵称失败命令，服务器内部使用*/
	CMD_LOAD_USER_INFO = 42,  	/*加载用户基本信息*/
	CMD_UDP_KEEP_DATA = 43,
	CMD_UDP_DEV_STAT = 44,
	CMD_SERVICE_DATE_Q = 45,  	/*服务套餐查询请求*/
	CMD_SERVICE_DATE_A = 46,  	/*服务套餐查询响应*/
#if 0	
	CMD_SERVICE_CHARGE = 47,  	/* 服务套餐更新*/
#endif	
	CMD_URL_HIT_Q = 48, 		/*查询URL 命中请求*/
	CMD_URL_HIT_A = 49, 		/*查询URL 命中响应*/
	CMD_IPLOCATION_Q	= 50,	/*查询客户端IP地址范围*/
	CMD_IPLOCATION_A = 51,	/*dispatcher 查询客户端IP地址范围*/
	CMD_SELLER_LOGIN_Q	= 52,	/*销售人员登录认证请求*/
	CMD_SELLER_LOGIN_A	= 53,	/*销售人员登录认证应答*/
	CMD_SELLER_USER_Q	= 54,	/*用户账号查询请求*/
	CMD_SELLER_USER_A	= 55,	/*用户账号查询应答*/
	CMD_SELLER_USER_ADD	= 56,	/*添加账号*/
	CMD_SELLER_USER_MODIFY = 57,/*修改账号*/
	CMD_SELLER_USER_DEL	= 58,	/*删除账号*/
	CMD_VEDIO_Q = 59, /* 请求查看摄像头视频 */
	CMD_VEDIO_A = 60, /* 摄像头视频信息应答 */
	CMD_SELLER_PWD = 61,	/*修改账号密码*/
	CMD_VIDEO_SYN_Q = 62,
	CMD_VIDEO_SYN_A = 63,
	CMD_VIDEO_HELLO_REQ = 64,
	CMD_VIDEO_HELLO =65,
	CMD_VIDEO_JPG = 66,
	CMD_VIDEO_ACK = 67,
	CMD_VIDEO_QUALITY = 68,
	CMD_VIDEO_STOP = 69,
	CMD_VEDIO_AGENT_A = 70,
	CMD_VEDIO_AGENT_Q = 71,
	CMD_VEDIO_AGENT_SETUP = 72,
	CMD_MISC_Q = 73,
	CMD_MISC_A = 74,
	CMD_UDP_YW_DISPATCH_Q = 78,//运维--在分配服务器下-向设备服务器发送-取得设备服务器下详细设备列表
	CMD_LOAD_PACKAGE_LOG_Q = 79,//查询充值日志
	CMD_LOAD_PACKAGE_LOG_A = 80,//查询充值日志
	CMD_UDP_YW_DISPATCH_A = 81,////运维--在分配服务器下-向设备服务器发送
	CMD_ADD_RC_CARD_Q = 83,  /*添加充值卡请求*/
	CMD_ADD_RC_CARD_A = 84,  /*添加充值卡响应*/
	#if 0
	CMD_MOD_RC_CARD_Q = 85 , /*更新充值卡状态请求*/
	CMD_MOD_RC_CARD_A = 86,  /*更新充值卡状态响应*/
	CMD_DEL_RC_CARD_Q = 87,  /*删除充值卡请求*/
	CMD_DEL_RC_CARD_A = 88, /*删除充值卡响应*/
	#endif
	CMD_TRANS_RC_CARD_Q = 89, /*转移充值卡请求*/
	CMD_TRANS_RC_CARD_A = 90, /*转移充值卡响应*/
	CMD_QUERY_RC_CARD_Q = 91, /*查询充值卡状态请求*/
	CMD_QUERY_RC_CARD_A = 92, /*查询充值卡状态响应*/
	CMD_CARD_CHARGE_Q = 93, /*充值请求*/
	CMD_CARD_CHARGE_A = 94, /*充值响应*/
	CMD_DEV_CHARGE = 95,  /*充值成功同步通知命令*/
	CMD_YW_SERVER_A = 96, /*运维查询服务器响应*/
	CMD_VEDIO_AGENT_OK = 97,/*代理成功*/
	CMD_ADD_CARD_TYPE_Q = 98,/*添加卡类型请求*/
	CMD_ADD_CARD_TYPE_A = 99,/*添加卡类型响应*/
	CMD_QUERY_CARD_TYPE_Q = 100,/*查询卡类型请求*/
	CMD_QUERY_CARD_TYPE_A = 101,/*查询卡类型响应*/
	CMD_QUERY_USER_PWD_Q = 102,/*取得当前登录DEV密码*/
	#if 0
	CMD_CARD_CHARGE_SYN_Q = 103,/*充值信息同步请求*/
	CMD_CARD_CHARGE_SYN_A = 104,/*充值信息同步应答*/
	CMD_CARD_TYPE_SYN_Q = 105,/*充值卡类型同步请求*/
	CMD_CARD_TYPE_SYN_A = 106,/*充值卡类型同步应答*/
	CMD_CARD_STATUS_SYN_Q = 107,/*充值卡状态同步请求*/
	CMD_CARD_STATUS_SYN_A = 108,/*充值卡状态同步应答*/
	CMD_DEV_CHARGE_CONFIRM = 109,/*充值成功同步通知确认命令*/
	#endif
	CMD_YW_DEVSERVER_Q = 110,/*运维管理--设备服务器查询*/
	CMD_YW_DEVSERVER_A = 111,/*运维管理--设备服务器查询*/
	CMD_BIND = 112, /* 主从设备命令 */
	CMD_SET_NAME = 113, /* 主从设备命令 */
	CMD_MS_CTRL = 114,  /* 主从设备命令，服务器不需要处理 */
	CMD_VIDEO_ROLL = 115, /* 控制云台转动 */
	CMD_FM_CONFIG_Q = 116,  /*配置家庭测量人员请求*/
	CMD_FM_CONFIG_A = 117,  /*配置家庭测量人员响应*/
	CMD_FM_Q = 118,  /*查询家庭测量人员请求*/
	CMD_FM_A = 119,  /*查询家庭测量人员响应*/
	CMD_MESURE_TRANS = 120, /*传输测量结果*/
	CMD_MESURE_Q = 121,      /*查询测量结果请求*/
	CMD_MESURE_A = 122,      /*查询测量结果响应*/
	CMD_MESURE_DEL = 123,   /*删除测量结果*/
	CMD_MESURE_TRANS_CONFIRM = 124, /*设备服务器间同步测量数据确认*/
	CMD_PLUG_TIMER_Q = 125,	/* 定时开关的请求 */
	CMD_PLUG_TIMER_A = 126,	/* 定时开关的回应 */
	CMD_USER_DEBUG = 127,     /*手机上传调试信息*/
	CMD_IR_LIST = 128,		/*手机请求支持列表*/
	CMD_IR_CONFIG = 129,	/*手机配置和获取已配置家电*/
	CMD_IR_DB = 130,		/*设备从服务器获取编码库*/
	CMD_IR_URL = 131,		/*设备获取电器遥控列表的URL */
	CMD_IR_CTRL= 132,		/* 手机测试红外编码库是有有效 */
	CMD_REMOTE_CONFIG = 133, /* CMD_REMOTE_CONFIG */
	CMD_REMOTE_KEY_CONFIG = 134, /* 手机为家庭电器配置一个按键 */
	CMD_REMOTE_CODE = 135, /* 手机为按键配置一个编码 */
	CMD_REMOTE_CTRL= 136, /* 手机向家庭电器发送控制命令 */
	CMD_REMOTE_STATE = 137, /* 手机对单个电器状态的操作 */
	CMD_VIDEO_TEMP_QUALITY = 138, /* 用于云台控制时，临时调试画质，不保存配置 */
	CMD_VIDEO_RECOR_CTRL = 139,/*设备端本地录像命令------已经取消此报文*/
	CMD_ALARM_BIND_PHONE = 140,	/*报警短信绑定手机*/
	CMD_ALARM_MSG = 141,	/*报警信息发送请求*/
	CMD_FAIL_EX = 142,		/* cmd_fail扩展命令 */
	CMD_ALARM_CONFIG_PHONE = 143,
	CMD_SLAVE_HIS = 144, /*从设备通告视屏信息给主设备*/
	CMD_GET_CMDOK = 145,	/* 设备端向服务器查询手机认证成功后需要的相关信息 */
	CMD_ALARM_LOG = 146,/*报警日志相关操作*/
	CMD_ALARM_LOG_SYNC = 147,/*报警日志的同步*/
	CMD_TIME_SYNC = 148,/*设备向手机查询时间同步*/
	CMD_VTAP_LIST_Q = 149,/*查询设备录像列表*/
	CMD_VTAP_LIST_A = 150,/*响应设备录像列表的查询*/
	CMD_VTAP_Q = 151,/*请求设备录像文件*/
	CMD_VTAP_KEEP = 152,/*录像观看保活*/
	CMD_VTAP_A = 153,/*录像请求结果*/
	CMD_VTAP_TIMESTAP = 154, /*请求播放指定时戳*/

	CMD_REMOTE_BROADCAST = 155, /* 广播控制命令给从设备 */
	CMD_VIDEO_SONIX = 156, /*对sonix 设置*/
	CMD_REC_TIMER_Q = 157,/* 视频录制定时开关的请求 */
	CMD_REC_TIMER_A  = 158,/* 视频录制定时开关的回应 */
	CMD_VIDEO_CONTROL_ALARM_CFG = 159, /*侦测报警配置信息*/
	CMD_MS_DEV_INFO = 160, /*主从设备通信告知相互的版本，时区，支持的功能 */

	CMD_DEV_SAHRD_INFO = 161, /*设备共享命令，请求与响应相同*/
	CMD_DEV_SAHRD_COUNT = 162, /*设备分享次数增加命令*/
	CMD_OPEN_TELNET = 163, /*开启从设备telnet服务*/
	CMD_ALARM_SWITCH_CTL = 164, /*安防报警总开关控制*/
	CMD_VTAP_END = 165,/*设备通知客户端，录像播放完毕*/
	CMD_VIDEO_QUALITY_V2 = 166,/* 自定义分辨率设置 */
	CMD_PLUG_ELECTRIC_STAT = 167,/*电量统计功能支持查询命令*/
	CMD_SCENE_CONFIG = 168,/*情景模式操作命令*/
	CMD_OPT_SCAN_Q = 169, /*扫描刷机设备请求*/
	CMD_OPT_SCAN_A = 170, /*扫描刷机设备响应*/
	CMD_DEV_REG_Q = 171,  /*刷机设备注册请求*/
	CMD_DEV_REG_A = 172, /*刷机设备注册响应*/
	CMD_ARIA2C_Q = 173, /*aria2c控制请求*/
	CMD_ARIA2C_A = 174, /*aria2c控制响应*/
	CMD_NETWORK_DETECT_Q = 175, /* 视频网络探测请求包 */
	CMD_NETWORK_DETECT_A = 176, /* 视频网络探测响应包 */
	CMD_SSIDPW = 177, /*手机修改设备wifi SSID和密码*/
	CMD_DEVSERVER_LIST_Q = 178, /*获取设备服务器列表请求*/
	CMD_DEVSERVER_LIST_A = 179, /*获取设备服务器列表响应*/
	CMD_NET_PROBE_Q = 180,  /*设备到服务器网络探测请求*/
	CMD_NET_PROBE_A = 181,  /*设备到服务器网络探测响应*/
	CMD_LOCATION_Q = 182,  /*设备位置请求*/
	CMD_LOCATION_A = 183,  /*设备位置响应*/
	CMD_MACDENY = 184, /*添加wifi mac禁止列表*/
	CMD_MACALLOW = 185, /*删除wifi mac禁止列表*/
	CMD_BIND_SLAVE_INFO = 186,/*获取从设备绑定信息*/
	CMD_MASTER_SLAVE_CTRL = 187,/*主从之间的控制命令，不通过服务器*/
	CMD_REBOOT = 188,/*远程重启命令*/
	CMD_AREA_CONFIG = 189,/*区域操作命令*/	
	CMD_STATIC_PIC_Q = 190,/*最近静态图片请求*/
	CMD_STATIC_PIC_A = 191,/*最近静态图片响应*/
	CMD_POSITION_Q = 192, /*定位信息请求*/
	CMD_POSITION_A = 193, /*定位信息响应*/
	CMD_SPEED_MAX_Q = 194,/*限速阀值请求*/
	CMD_SPEED_MAX_A = 195,/*限速阀值响应*/
	CMD_SCHLBUS_BIND = 196,/*校车绑定*/
	CMD_VOICE = 197,
	CMD_VOICE_ACK = 198,
	CMD_SPEEK_Q = 199,
	CMD_SPEEK_A = 200,
	CMD_VOICE_REG = 201,
	CMD_REMOTE_CODE_UPLOAD = 202,/*上传本地设备学习到的控制编码到服务器*/
	CMD_RECORD_QUALITY_V2 = 203,/*录像质量配置*/
	CMD_NOTIFY_HELLO = 204, /* 设备与小区服务器握手请求 */
	CMD_NOTIFY_HELLO_ACK = 205, /* 设备与小区服务器握手响应 */
	CMD_NOTIFY = 206, /* 消息推送命令，如报警、健康、公告 */
	CMD_NOTIFY_RESULT = 207, /* 消息推送应答 */
	CMD_NOTIFY_EXPECT = 208, /* 消息ID 同步命令*/
	CMD_NOTIFY_CENTER_LIST = 209, /* 小区服务器把自己ip/port上报给服务器，服务器推送给设备*/
	CMD_CMT_OP_DEVICE = 210, /* 对小区所管理的设备进行操作，如添加。参数net_cmt_op_device_t*/
	CMD_VOICE_PROMPT  = 211, /* 播放本地音乐文件 */
	CMD_REMOTE_BD_BIND = 212,	/* 双向RF协议绑定解绑定 */
	CMD_REMOTE_CONFIG_SOUNDLIGHT = 213,	/* 配置安防报警器关联的声光报警器 */
	CMD_REMOTE_TD_CODE = 214, /* 电器二维码添加 */
	CMD_PHONE_BIND_Q = 215, /*绑定手机申请提交，参数net_phone_bind_t*/
	CMD_PHONE_REQUESTLIST_Q =216, /*绑定手机申请列表查询*/
	CMD_PHONE_REQUESTLIST_A =217, /*绑定手机申请列表响应，参数net_phone_bind_list_t*/
	CMD_PHONE_BIND_OPERATION = 218, /*已绑定手机对绑定申请操作，参数net_phone_bind_operation_t*/
	CMD_PHONE_BIND_RESULT = 219, /*已绑定手机对绑定申请操作结果，参数net_phone_bind_result_t*/
	CMD_PHONE_BIND_DEL = 220, /*删除绑定手机，net_phone_bind_uuid_t*/
	CMD_PHONE_UNBINDLOGIN_ALLOW = 221, /*允许未绑定手机登录*/
	CMD_PHONE_UNBINDLOGIN_DENY = 222, /*禁止未绑定手机登录*/
	CMD_PHONE_BINDLIST_Q = 223, /*绑定手机列表查询请求*/
	CMD_PHONE_BINDLIST_A = 224, /*绑定手机列表查询响应，参数net_phone_bind_list_t*/
	CMD_SCENE_TIMER_Q  =  225,	/*配置情景模式的定时器请求命令*/
	CMD_SCENE_TIMER_A  =  226,	/*配置情景模式的定时器的应答命令*/
	CMD_SCENE_LINKAGE = 227,	/* 联动情景配置命令 */
	CMD_PHONE_APN_OPERATION = 228, /*配置手机推送命令，参数net_phone_push_t*/
	CMD_STATIC_PIC_Q_V2 = 229,/*第二版本最近静态图片请求*/
	CMD_STATIC_PIC_A_V2 = 230,/*第二版本最近静态图片响应*/
	CMD_REMOTE_VTY = 231,/* 启动远程telnetd服务 */
	CMD_IA = 232,		/* 智能家居单品控制和查询 */
	CMD_NEWUPVER_Q = 233,	/*手机获取设备最新升级版本请求*/
	CMD_NEWUPVER_A = 234,	/*手机获取设备最新升级版本应答*/
	CMD_SET_NEWUPVER = 235,	/*手机设置设备升级版本*/
	CMD_NOTICE_DEVUP = 236,	/*手机通知设备立即升级*/
	CMD_V4L2_COLOR = 237, /* v4l2 color参数查询和控制 */
	CMD_NOTIFY_QUERY = 238, /* 查询推送消息请求，参数net_notify_query_t，响应CMD_NOTIFY */
	CMD_MOTO_ATTRIBUTE = 239, /* IPC云台属性相关命令 */
	CMD_MOTO_PRE_POSITION = 240, /* IPC云台预置位命令 */
	CMD_MOTO_CRUISE = 241, /* IPC云台巡航命令 */
	CMD_UDP_AUTH = 242,
	CMD_UDP_KEEPLIVE = 243,
	CMD_UDP_CTRL = 244,
	CMD_UDP_NOTIFY = 245,
	CMD_UDP_BIND_PHONE = 246,
	CMD_RF2_CTRL = 247,
	CMD_CLOUD_MATCH = 248,	/* 客户端和设备进行云匹配交互的报文 */
	CMD_CLOUD_MATCH_RESULT = 249,	/* 客户端向服务器请求云匹配结果 */
	CMD_CLOUD_AC_LIB_INFO = 250,	/* 设备向服务器获取云空调编码摘要部分 */
	CMD_CLOUD_AC_LIB_DATA = 251,	/* 设备向服务器获取云空调编码数据部分 */
	CMD_805_CONFIG = 254, /*805点阵屏和蜂鸣器控制*/
	CMD_UDP_APP_REPORT_RUNENV = 256,
	CMD_UDP_APP_REPORT_ERROR = 257,  
	CMD_APP_SERVER_DISP = 258,
	CMD_UDP_APP_REPORT_HABIT = 259,
    CMD_SHARE_REGISTER = 260,
	CMD_APP_USER = 261, /*手机用户注册、认证*/
	CMD_HOME_CONFIG = 262, /*家庭配置*/
	CMD_HOME_SHARE = 263, /*家庭分享、成员管理*/
	CMD_LINKAGE_CONFIG = 264, /*联动配置*/
	CMD_TLV_UDP_CTRL = 265, /* 仿造CMD_UDP_CTRL, 只是内容变成TLV方式*/
	CMD_PUSH_NOTIFY = 266,/*服务器通知命令*/
	
	CMD_HOME_LABEL = 268,/*家庭标签命令*/
	CMD_HOME_DICTIONARY = 269,/*字典*/
	
	CMD_UDP_DNS_PROB = 301, /* 根据设备ip地址获取多国服务器域名 */
	CMD_APP_DEV_USER = 302,	// APP客户的和WIFI设备通道
	CMD_APP_LINKAGE_USER = 303,	// APP和联动服务器
	CMD_SERVER_WIFI_DEV_USER = 304,	// 服务器和WIFI设备通道
	CMD_SERVER_RF_DEV_USER = 305,	// 服务器和RF设备通道
	CMD_UDP_DONAME_PROB = 306, /* app探测多国服务器域名 或者最快服务器命令，服务器不好搞，单独设置命令*/
	//307服务器用了
	CMD_QUERY_HISTORY = 308,/*日志查询命令*/
	CMD_WIDGET_KEY = 309,	/* 获取WIDGET秘钥命令 */
	CMD_MAX
};
/*通信协议命令
禁止在已有命令中间插入新命令
只能顺序添加到后面*/


enum{
	ERR_NONE = 0,
	ERR_SN_INVALID = 1,    		/*DS007序列号无效*/
	ERR_NICKNAME_INVALID = 2,	/*用户昵称无效*/
	ERR_PASSWD_INVALID = 3,   	/*用户口令错误*/
	ERR_CMD_INVALID = 4,     	/* 无效命令*/
	ERR_PARAM_INVALID = 5, 		/*无效参数*/
	ERR_MEMORY = 6,       		/*服务器内部分配内存失败*/
	ERR_SYSTEM = 7,       		/*服务器内部系统还是调用失败*/
	ERR_NICKNAME_CONFLICT = 8,  /*昵称冲突*/
	ERR_NICKNAME_TOO_LONG =9, 	/*昵称过长*/
	ERR_EMAIL_TOO_LONG = 10,    /*email地址过长*/
	ERR_DATABASE = 11,			/*数据库操作失败*/
	ERR_CLIENT_VER = 12, 		/*手机客户端版本过低*/
	ERR_DEV_OFFLINE = 13,		/* 设备离线 */
	ERR_VEDIO_OFF = 14,		/* 未插入摄像头 */
	ERR_DEV_SYS_ERR = 15,		/* 设备系统错误 */
	ERR_SELLER_NAME_CONFLICT = 16,/*销售人员用户名冲突*/
	ERR_TOO_MANY = 17, /* 太多人在观看视频了 */
	ERR_PACKAGE_NAME_CONFLICT = 18, /* 太多人在观看视频了 */
	ERR_OUT_SERVICE = 19, /* 服务到期 */
	ERR_CARD_SN_INVALID = 20, /*充值卡序列号无效*/
	ERR_CARD_PWD_INVALID = 21, /*充值卡密码无效*/
	ERR_CARD_STATE_INVALID = 22, /*充值卡状态无效*/
	ERR_CARD_NOTIME_TRANS = 23, /*设备无服务期限可转移*/
	ERR_TIMEOUT = 24, /*超时失败*/
	ERR_AGENT = 25,		/* 代理失败*/
	ERR_EMAIL_INVALID =26, /*email地址无效*/
	ERR_FM_ID = 27,/* 家庭成员ID无效 */
	ERR_FM_LIMIT = 28, /* 家庭成员配置过多 */
	ERR_DEV_SYS_BUSY = 29, /* 系统忙，可能正在升级系统 */
	ERR_PLUG_TIMER_LIMIT = 30, /* 定时开关策略配置个数已达到最大 */
	ERR_PLUG_TIMER_ID = 31, /* 定时开关策略ID无效 */
	ERR_REMOTE_LIMIT = 32, /* 可控制电器配置已达最大数量 */
	ERR_IR_DB_INVALID = 33, /* 红外编码库错误 */
	ERR_REMOTE_BUTTON_LIMIT = 34, /* 可控电器按键达到上限 */
	ERR_REMOTE_ID_INVALID = 35, /* 可控电器ID无效 */
	ERR_REMOTE_KEY_ID_INVALID = 36, /* 可控电器KEY ID无效 */
	ERR_REMOTE_BUSY = 37, /* 电器正忙，比如处于学习状态 */
	ERR_REMOTE_KEY_VALID = 38, /* 电器按钮无效 */
	ERR_REMOTE_CODE_LEARN_FAILED = 39, /*学习失败*/
	ERR_PHONE_NUM_EXCESS = 40,/*超出支持的最大电话绑定数*/
	ERR_NO_BIND_PHONE = 41,/*该智能网关尚未绑定手机*/
	ERR_DEV_UNLINK = 42,/*设备未连接设备服务器*/
	ERR_ALARM_PHONE_NOT_FOUNT = 43, /*绑定的报警手机号不存在*/
	ERR_ALARM_VIDEO_NOT_FOUNT = 44,/*没有指定的报警录像*/
	ERR_ALARM_LOG = 45,/*报警日志操作出错*/
	ERR_ALARM_LOG_SYNC = 46,/*报警日志同步出错*/
	ERR_REC_TIMER_LIMIT = 47,/*视频录制定时器:已经达到策略最大可配置数*/
	ERR_REC_TIMER_OPT = 48,/*视频录制定时器:操作失败*/
	ERR_REC_TIMER_ID   = 49,/*视频录制定时器:定时器id无效*/
	ERR_REC_TIMER_NTP = 50,/*ntp 未同步 添加失败*/
	ERR_REC_TIMER_DURATION = 51,/*时长太短*/
	ERR_NO_VTAP = 52,/*没有视频录像文件*/
	ERR_SLAVE_OFFLINE = 53, /* 从设备离线 */
	ERR_DPI_FOR_PHONE = 54, /* 手机在线不支持大分辨率、帧率的配置 */
	ERR_CODE_ADJUST = 55, /* 对应编码不支持微调 */
	ERR_VTAP_CLIENT_EXCEED = 56, /*观看录像的人太多了，同时只能有1个人看*/
	ERR_VTAP_DAMAGE = 57,/*录像文件损坏*/
	ERR_SCENE_VERSION = 58,/*版本儿不匹配*/
	ERR_SCENE_ID	=59,/*非法场景id*/
	ERR_SCENE_FAIL	=60,/*执行失败*/
	ERR_SCENE_ACTION	= 61,/*操作非法*/
	ERR_SCENE_ID_MAX	=62,/*场景id已达上限*/
	ERR_SCENE_BUSY		=63,/*执行忙*/
	ERR_AREA_VERSION = 64,/*区域版本儿不匹配*/
	ERR_AREA_ID	=65,/*非法区域id*/
	ERR_AREA_FAIL	=66,/*操作失败*/
	ERR_AREA_ACTION	= 67,/*操作非法*/
	ERR_AREA_ID_MAX	=68,/*区域id已达上限*/
	ERR_AREA_ERR_OBJTYPE =69,/*错误的对象类型*/	
	ERR_NO_SD_DETECTED = 70, /* 没有存储设备插入 */
	ERR_NOT_SUPPORT = 71,/*设备不支持*/
	ERR_BUSY = 72,/*正与其他人通话*/
	ERR_REMOTE_NOT_SUPPORT = 73,/*本设备不支持电器控制,也许是从设备支持*/
	ERR_TF_NOT_INSERT = 74,/*TF卡未插入*/
	ERR_REMOTE_INVALID_TD = 75,/* 添加电器未知的二维码信息 */
	ERR_UNBIND =  76, /*非绑定用户禁止登录*/
	ERR_BIND_FULL =77, /*达到绑定数量限制，不能申请绑定了*/
	ERR_BINDLATE = 78, /*已经有人对绑定申请进行处理了*/
	ERR_SCENE_TIMER_LIMIT = 79, /*情景模式定时器操作最大数*/
	ERR_SCENE_TIMER_ID = 80,	/*情景模式定时器的ID号不对*/
	ERR_SCENE_INVALID_ID = 81, /* 场景联动时无效的场景ID */
	ERR_SCENE_INVALID_REMOTE_ID = 82, /* 场景联动时无效的电器ID */
	ERR_UNBIND_WITH_DEV_OFFLINE = 83, /*未绑定登录失败，且设备离线，不要提示用户申请绑定*/
	ERR_DUPLICATE_REMOTE_CODE = 84,	/* 重复添加按键编码 */
	ERR_IA_NOT_READY = 85, /*智能单品没初始化*/
	ERR_IA_OPERATE_INVALID = 86, /*智能单品操作无效*/
	ERR_UPGRADE_VER_EMPTY = 87, /*手机查询设备最新升级信息为空*/

	ERR_NEED_ENCRPYT = 88, /* 需要加密，却无法协商出一样的加密算法 */
	ERR_CLONE = 89,  /* 设备是克隆机 */
	ERR_WAIT_IR_TIMEOUT = 90,	/* 云匹配时候等待红外超时 */
	ERR_CLOUD_MATCH_FAILED = 91,		/* 云匹配失败 */
	ERR_CLOUD_LIB_TIMEOUT = 92,	/* 从服务器获取编码失败 */
	ERR_CLOUD_NOT_READY = 93,	/* 服务器没有连接上 */
	ERR_CLOUD_MATCHING = 94,	/* 云匹配正在进行 */
	ERR_CLOUD_LIB_MISSING = 95,	/* 查询的编码库ID不存在 */
	ERR_CLOUD_LIB_SET = 96,	/* 设置云电器编码库失败 */
	ERR_SHORTCUT_ONOFF_UTC_PAST = 97,	/* 快捷开关因为手机时间问题设置失败 */
	ERR_SOFT_VER_LOW = 99, /* 手机版本过低 */
	ERR_OLD_EXPLICT = 100,/*表示是服务器迁移手机账号*/
	ERR_USER_EXIST = 102,/*用户已存在*/
	ERR_MAX /* 请在此之前添加错误码 */
};

/*可控电器类型*/
enum{
	REMOTE_TYPE_TV = 1,/*电视*/
	REMOTE_TYPE_TVBOX = 2,/*机顶盒*/
	REMOTE_TYPE_AIRCONDITION = 3,/*空调*/
	REMOTE_TYPE_W_TV = 10,	/*WIFI转红外电视*/
	REMOTE_TYPE_W_TVBOX = 11,	/*WIFI转红外机顶盒*/
	REMOTE_TYPE_W_AIRCONDITION = 12,	/*WIFI转红外空调*/
	REMOTE_TYPE_W_OTHER = 13,	/*WIFI转其他类型电器*/

	REMOTE_TYPE_CLOUD_AIRCONDITION = 20,	/* 云空调 */
	REMOTE_TYPE_CLOUD_TV = 21,	/* 云电视 */
	REMOTE_TYPE_CLOUD_STB = 22,	/* 云机顶盒 */

	REMOTE_TYPE_WIRELESS_BASE = 128,
	REMOTE_TYPE_CURTAIN =129,/*窗帘*/
	REMOTE_TYPE_DOOR = 130,/*门*/
	REMOTE_TYPE_WINDOW = 131,/*窗*/
	REMOTE_TYPE_PLUG = 132,/*插座*/
	REMOTE_TYPE_LAMP = 133,/*灯*/
	REMOTE_TYPE_ALARM = 134,/*报警*/
	REMOTE_TYPE_SOUNDLIGHT = 135,/*声光报警*/
	REMOTE_TYPE_SCENE_CONTROLLER = 136,	/* 情景遥控器 */
	REMOTE_TYPE_BD_LAMP = 150,	/* 双向灯板 */
	REMOTE_TYPE_BD_PLUG = 151,	/* 双向插座 */
	REMOTE_TYPE_BD_CURTAIN = 152, /* 双向窗帘 */
	REMOTE_TYPE_BD_ALARM = 153,	/* 双向报警器 */
	REMOTE_TYPE_BD_DIMMING_LAMP = 154,	/* 双向调光灯 */
	REMOTE_TYPE_BD_SOUND_LIGHT = 155,	/* 双向声光报警器 */
	REMOTE_TYPE_DIY = 254,/*自定义*/
	REMOTE_TYPE_OTHER = 255/*其他*/
};
/*编码类型*/
enum{
	CODE_TYPE_LEARN = 0,/*通过学习得到的编码*/
	CODE_TYPE_CUSTOM = 1/*根据电阻芯片类型自定义的编码*/
};

#define REMOTE_KEY_TEMP 	0x80000000	/* 空调温度掩码 */
#define REMOTE_ALARM_MASK	0x40000000  /* 按键名称为数字掩码 */
#define REMOTE_DEV_ABILITY_LEARN    0x1
#define REMOTE_DEV_ABILITY_VALID    0x2	/*至少有一个有效按键时候，才置位*/

/* charge card sync operation type */
enum{
	OP_INSERT = 0,	/*插入*/
	OP_UPDATE = 1,  /*更新*/
	OP_QUERY =  2,  /*查询*/
	OP_DELE = 3,   /*删除*/
};

/* proto version control */
enum {
	PROTO_VER1 = 1,
	PROTO_VER2 = 2,
	/* further version add here */
};

#define PROTO_SUPPORT PROTO_VER2
#define PROTO_VERSION_INDEX(v)  ((v)-1)
#define PROTO_VERSION_VALID(v)	(((v) >= PROTO_VER1 && \
								(v) <= PROTO_SUPPORT) ? 1 : 0)

#define PROTO_MAX PROTO_VER2
#define MAX_PARAM_LEN (1024*1024*64)
#define SN_LEN 12	/*DS007设备序列号字符串长度*/
#define MAX_NICKNAME 16  /*用户昵称最大长度*/
#define MAX_HOSTNAME 64 /*最长的用户名称，udp设备用*/
#define MAX_PLUG_TIMER_NAME	64	/* 定时开关名称最大长度 */
#define MAX_EMAIL 32
#define MAX_PHONE_SINGLE 16
#define MAX_PHONE_NUM 10
#define MAX_SL_NUM (MAX_REMOTE_NUM - 1)
#define MAX_PHONE (MAX_PHONE_SINGLE * MAX_PHONE_NUM)
#define MAX_WEB_ROOT 64
#define MAX_HANDLE 0xFFFFFFFF
#define MAX_SERVER_HANDLE 0xEE6B2800 /*保证设备端与服务器产生的handle不能重复*/
#define MAX_SSIDPW_SSID_LEN 33
#define MAX_SSIDPW_PW_LEN 64

#pragma pack(push,1)

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	encrypt:1,
		compress:1,
		hlen:3,
		ver:3;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t ver:3,
		hlen:3,
		compress:1,
		encrypt:1;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t ds_type;
	u_int16_t command;
	u_int32_t param_len;
	u_int32_t handle;
}net_header_t;

#define  NHF_TRANSPARENT	0x01
#define  NHF_WAIT_REPLY	0x02

typedef struct {
	// hdr.ver = 2, hdr.hlen = sizeof(net_header_v2_t)/4
	net_header_t hdr;
	// NHF_XXX
	u_int8_t flags;
	// must be 0
	u_int8_t resv[3];
	u_int64_t sn;
} net_header_v2_t;

#define	net_hdr_len(net_hdr)	(((net_header_t *)net_hdr)->hlen<<2)
#define net_header_v2_size (sizeof(net_header_v2_t))

#define net_header_size (sizeof(net_header_t))
#ifdef CLIB_HDR
#define net_header_real_size(hdr) (((net_header_t *)hdr)->ver == PROTO_VER1 ? net_header_size : net_header_v2_size)
#else
#define net_header_real_size(v) ((v) == PROTO_VER1 ? net_header_size : net_header_v2_size)
#endif
#define	net_param_len(hdr) (((net_header_t *)hdr)->param_len)

/* 用原始接收到的协议头部填充新构造协议头部V2部分(注意SRC中V2的SN在接收时候被转换了字节序),并且填充v1的handle */
#define net_header_v2_wraper(dst, src) do {\
	memcpy((char*)(dst)+net_header_size, (char*)(src)+net_header_size, net_header_v2_size - net_header_size);\
	((net_header_v2_t*)(dst))->sn = ntoh_ll(((net_header_v2_t*)(src))->sn);\
	((net_header_t*)(dst))->handle = ((net_header_t*)(src))->handle;\
} while (0);

#ifndef CLIB_HDR
typedef struct{
	u_int32_t global_id;
	u_int32_t wan_ip[ISP_MAX];
	u_int32_t lan_ip;
	u_int16_t ds_port;
	u_int16_t http_port;
	u_int16_t ndev_now; /*目前有多少台设备连接在该服务器上*/
	u_int16_t ndev_next;/*还能继续接收多少个设备，由前台服务器自己估算一个，并可动态调整*/
	u_int16_t asid;
	u_int8_t ds_type;
	u_int8_t isp;
	u_int8_t is_master; /*主服务器，对分配服务器有效*/
	u_int8_t ds_action; /*DSAC_XXX*/
	u_int8_t can_encrypt;  /*压缩能力，当两端都为true时开启压缩*/
	u_int8_t can_compress /*加密能力，当两端都为true时开启加密*/;
	u_int8_t srv_status;
	u_int8_t is_linked; /*建立连接标志*/
	u_int8_t pad3;
	u_int8_t pad4;
}net_para_server_t;
#endif

typedef struct{
	u_int64_t sn;
	u_int32_t ip;
	u_int32_t local_ip;
	u_int16_t dev_flags; /* come from device, KF_XXX */
	u_int8_t ver_master; /* come from device */
	u_int8_t ver_slave; /* come from device */
	u_int8_t ver_build;
	u_int8_t pad2;
}net_para_device_t;

typedef struct{
	u_int32_t dev_server_id;
}net_dev_server_q;

typedef struct{
	u_int8_t file_name[32];//文件名
	u_int8_t file_content[0];//文件内容
}net_dev_list_file_name_t;

typedef struct{
	u_int8_t username[MAX_NICKNAME];
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t client_id; /*CID_IPHONE*/
	u_int8_t isp; /*client isp, available between servers*/
}net_para_user_bind_q;

typedef struct{
	u_int64_t sn;
	u_int32_t devserver_id;
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t is_udp;	/*是否走udp ctrl protocol*/
	u_int8_t flag;
}net_device_location_t;

typedef struct{
	u_int64_t sn;
	u_int16_t stat_len;
	u_int8_t stat_data[0];
}net_pkg_stat_t;

typedef struct{
	u_int32_t time;
	u_int8_t passwd[16];
}net_device_pwd_t;

typedef struct{
	u_int64_t sn;
	u_int32_t time;
	u_int8_t passwd[16];
}net_pkg_pwd_t;

typedef struct{
	u_int64_t dev_sn;
	u_int8_t password[33];
}net_query_user_pwd;

typedef struct{
	u_int64_t sn;
	u_int32_t callback_id;
	u_int8_t url[0];
}net_para_urlq_t;

typedef struct{
	u_int32_t callback_id;
	u_int8_t query_fail;
	u_int8_t hit_class;
	u_int8_t url_type;
	u_int8_t game_id;
}net_para_urla_t;

typedef struct{
	u_int64_t sn;
	u_int8_t md5[16];
}net_para_user_auth_t;

typedef struct{
	u_int64_t sn;
	u_int8_t md5[16];
	u_int8_t uuid[40];
	u_int8_t reserved[4];
}net_para_user_auth_v2_t;

typedef struct {
	u_int8_t uuid[40];
}net_phone_bind_uuid_t;

typedef struct{
	u_int8_t phone_number[16]; /*手机号码*/
	u_int8_t phone_model[32]; /*手机型号*/
	u_int8_t bind_name[16];/*绑定名称*/
	u_int8_t bind_uuid[40]; /*绑定uuid*/
	u_int8_t bind_message[40];/*绑定描述*/
	u_int8_t timestamp[20]; /*时间戳2014-02-20 13:56:12*/
}net_phone_bind_t;

typedef struct {/*绑定申请*/
	u_int64_t sn;
	net_phone_bind_t phone;
	u_int8_t reserved[4];
}net_phone_bind_request_t;

typedef struct{/*已绑定列表，对应数据库t_bind_phone*/
	u_int8_t count; /*绑定列表个数*/
	u_int8_t unbind_login;
	u_int8_t reserved[2];
	net_phone_bind_t binds[0]; /*绑定列表*/
}net_phone_bind_list_t;

typedef struct{/*未处理绑定申请列表，对应数据库t_bind_phone_q*/
	u_int8_t count; /*绑定列表个数*/
	u_int8_t reserved[3];
	net_phone_bind_t binds[0]; /*绑定列表*/
}net_phone_bind_q_list_t;


enum{
	BIND_ACTION_ACCEPT = 1, /*批准绑定*/
	BIND_ACTION_DENY = 2 /*拒绝绑定*/
};

typedef struct{
	u_int8_t action; /*BIND_ACTION_ACCEPT , BIND_ACTION_DENY*/
	u_int8_t reserved[3];
	u_int8_t request_uuid[40]; /*申请者uuid*/
	u_int8_t operation_uuid[40]; /*操作者uuid*/
}net_phone_bind_operation_t;


typedef struct{
	u_int32_t err_code; /*ERR_NONE, ERR_BINDLATE*/
	u_int8_t action; /*BIND_ACTION_ACCEPT , BIND_ACTION_DENY*/
	u_int8_t reserved[3];
	u_int8_t request_uuid[40]; /*申请者uuid*/
	net_phone_bind_t ratify_info; /*绑定申请处理者信息*/
	u_int8_t reserved1[4];
}net_phone_bind_result_t;

typedef struct{
	char ip[64];
	char location[128];
}net_para_iplocation_t;


typedef struct{
	u_int64_t sn;
	u_int8_t nickname_len;
	u_int8_t email_len;
	u_int8_t data[0];	/* 后面指向昵称和email地址，长度为nickname_len+email_len */
}net_para_nickname_t;

/*CMD_NICKNAME_FAIL*/
typedef struct{
	u_int64_t sn;
	u_int32_t errorcode;
}net_nickname_fail_t;

/* CMD_WEB_AUTH_Q CMD_SELLER_LOGIN_Q */
typedef struct{
	u_int8_t name[MAX_NICKNAME];
	u_int8_t rand[8];
	u_int8_t md5[33];
}net_para_web_auth_t;

/* CMD_WEB_AUTH_Q CMD_SELLER_LOGIN_Q */
typedef struct{
	u_int8_t name[MAX_NICKNAME];
	u_int8_t rand[8];
	u_int8_t md5[33];
	u_int32_t ip; /*用户浏览器客户端IP*/
}net_para_web_auth_v1_t;


/* CMD_SELLER_MASTER_A */
typedef struct{
	u_int32_t master_ip;
	u_int16_t master_http_port;
}net_master_location_t;

/* CMD_SELLER_USER_Q */
typedef struct{
	u_int8_t role;
	u_int8_t is_username;
	u_int8_t is_creator;
	u_int8_t username[MAX_NICKNAME];
	u_int8_t creator[MAX_NICKNAME];
}net_seller_user_query_t;

#ifndef CLIB_HDR
/* CMD_SYS_LOG_Q */
typedef struct{
	u_int8_t oper_name[MAX_NICKNAME];
	u_int64_t dev_sn;
	u_int8_t old_service_time[MAX_DATE];
	u_int8_t service_time[MAX_DATE];
	u_int8_t old_service_type;
	u_int8_t service_type;
	u_int8_t oper_time[MAX_DATE];
}net_admin_charge_t;

/* CMD_SYS_LOG_Q */
typedef struct{
	u_int64_t dev_sn;
	u_int8_t  oper_time_start[MAX_DATE];
	u_int8_t  oper_time_end[MAX_DATE];
}net_query_admin_charge_log_t;//查询管理员手工更改服务期日志
#endif


/* CMD_PACKAGE_ADD_QUERY_Q */
typedef struct{
	u_int8_t op_type;
	u_int8_t packe_name[33];
	u_int32_t package_money;
	u_int8_t service_time;
	u_int8_t service_unit[4];
	u_int8_t packe_desc[180];
	u_int8_t service_state;
}net_package_add_query_t;





/* CMD_SYS_LOG_Q */
typedef struct{
	u_int64_t dev_sn;//设备序列号
	u_int64_t card_sn;//充值卡号
	u_int8_t charge_time[20];//充值时间
	u_int8_t service_time[20];//服务时间
}net_charge_log_t;


/* CMD_SYS_LOG_Q */
typedef struct{
	u_int8_t oper_name[MAX_NICKNAME];
	u_int8_t oper_time_start[20];
	u_int8_t oper_time_end[20];
	u_int8_t oper_reuslt;
	u_int32_t curent_num;
	u_int32_t page_size;
}net_query_sys_log_t;

/* CMD_SYS_LOG_Q */
typedef struct{
	u_int8_t oper_name[MAX_NICKNAME];
	u_int8_t charge_time_start[20];
	u_int8_t charge_time_end[20];
	u_int64_t dev_sn;
	u_int64_t card_sn;
	u_int32_t curent_num;
	u_int32_t page_size;
}net_query_charge_log_t;


/* CMD_SELLER_LOGIN_A CMD_SELLER_USER_A CMD_SELLER_USER_MODIFY*/
typedef struct{
	u_int8_t username[MAX_NICKNAME];
	u_int8_t role;
	u_int32_t dispatchserver_id;
	u_int16_t dispatchserver_port;
}net_seller_user_info_t;

/* CMD_SELLER_USER_ADD */
typedef struct{
	net_seller_user_info_t user_info;
	u_int8_t pwd[33];
	u_int8_t creator[MAX_NICKNAME];
}net_seller_user_t;

/* CMD_SELLER_PWD */
typedef struct{
	u_int8_t username[MAX_NICKNAME];
	u_int8_t new_pwd[33];
}net_seller_pwd_t;

enum{
	FT_USERDB = 0, /*用户数据库导入文件*/
	FT_USERUP = 1, /*用户数据修改日志文件,已废弃*/
	FT_URLLIB = 2, /*URL识别库文件*/
	FT_007BIN = 3, /*007设备升级文件*/
	FT_DEVICE = 4, /*设备修改日志记录文件*/
	FT_MAX
};

enum{
	FTP_GET,
	FTP_PUT
};

typedef struct{
	u_int8_t ft_type;  /*要查询文件类型，如FT_USERDB*/
	u_int32_t version; /*要查询文件版本，0表示取最新版本*/
}net_ftp_q;

typedef struct{
	u_int8_t ft_type;
	u_int32_t version;
	u_int32_t length;
	u_int32_t ip;
	u_int16_t port;
	u_int8_t filename[0];
}net_ftp_a;

typedef struct{
	u_int8_t ft_type;
	u_int8_t is_data; /*true表示data为同步数据，否则为同步列表*/
	u_int8_t is_compress; /*true表示数据为压缩*/
	u_int8_t is_last;
	u_int32_t version;
	u_int32_t data_len;
	u_int8_t data[0];
}net_server_sync_t;

#define	MAX_VIDEO_HIS	 20

// 观看视频的历史记录
typedef struct {
	// 客户端IP地址
	u_int32_t ip;
	// 开始观看时间
	u_int32_t begin;
	// 时长
	u_int32_t take;
} video_his_ele_t;

// 环形缓冲区
typedef struct {
	// 第一个节点
	u_int8_t begin;
	// 有多少个有效节点
	u_int8_t num;
	// 一个节点结构多大，主要用于从服务器取回后的版本兼容
	u_int8_t struct_sz;
	// = MAX_VIDEO_HIS
	u_int8_t max_video_his;
	video_his_ele_t e[MAX_VIDEO_HIS];
} video_his_t;

typedef struct {
	video_his_t his;
} video_info_v0_t;


#define	KID_DEV_BUF	0
#define	KID_CFG_IF	1
#define	KID_CFG_GNET	2
#define	KID_CFG_OPEN	3
#define	KID_VIDEO_STAT	4
#define	KID_VIDEO_CFG	5
#define	KID_MAX	10

#define	KOP_WRITE	0
#define	KOP_READ	1

typedef struct{
	u_int64_t sn;
	u_int32_t time;
	u_int8_t kid;
	u_int8_t op;
	u_int16_t data_len;
	u_int8_t data[0];
}net_keep_data_t;

typedef struct{
	u_int32_t config_time;
	u_int16_t config_len;
	u_int8_t *config_data;
}net_device_config_t;

typedef struct{
	u_int64_t sn;
	u_int32_t server_id;
	u_int32_t time;
	u_int8_t kid;
	u_int8_t op;
	u_int16_t data_len;
	u_int8_t data[0];
}net_sync_keep_data;

typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
	u_int8_t second;
	u_int8_t resv;
	u_int32_t wan_ip;
}net_time_t;

#define ONE_HOUR_SECOND	(60*60)
#define ONE_DAY_SECOND (ONE_HOUR_SECOND*24)
#define ONE_YEAR_SEDOND (ONE_DAY_SECOND*365)
#define VIP_YEAR 5
#define VIP_SECOND (VIP_YEAR*ONE_YEAR_SEDOND)
#define ONE_DAY_MINS (24*60)
#define ONE_HOUR_MINS (60)
#define ONE_MIN_SECS (60)


#define SERVICE_TASTE_MASK 0x01  /*免费体验标志位,true表示已经体验*/
#define SERVICE_VIP_MASK 0x02    /*终身套餐标志位,true表示是终身套餐*/
#define SERVICE_CHARGE_MASK 0x04 /*已经充值标志位*/

typedef struct {
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t df_flags;
	u_int8_t reserved;
	u_int8_t tlv[0];
}net_cmd_ok_t;

/*df_flags为设备状态标，目前支持如下标志：*/
#define DF_ONLINE 0x01 /*设备在线标志，true表示设备在线*/
#define DF_SERVICE_EXPIRED 0x02 /*设备服务到期标志，true表示服务已到期*/
#define DF_SERVICE_VIP 0x04 /*设备VIP标志，true表示是VIP用户*/
#define DF_BELTER 0x08 /*倍泰健康测量设备标志*/

typedef struct {
	u_int64_t sn;
	u_int8_t service_type;
	u_int8_t service_date[20];
	u_int32_t dev_type;//003/006/007
}net_service_date_t;

typedef struct {
	net_service_date_t v1;
	u_int16_t minute;
	u_int8_t times;
}net_service_date_v2_t;

// 付费服务
#define	MPF_MONEY	0x0001

typedef struct {
	u_int16_t mid;
	// MPF_XXX
	u_int16_t flags;
	// 保留以后扩展，必须填写为0
	u_int32_t resv;
} mod_policy_t;

typedef struct {
	net_service_date_v2_t v2;
	u_int16_t mod_num;
	mod_policy_t mod[0];
} net_service_date_v3_t;

typedef struct{
	u_int32_t study;
	u_int32_t game;
	u_int32_t other;
	u_int32_t nolimit;
	u_int32_t black;
}net_url_hit_t;


typedef struct {
	u_int8_t md5[16];
	u_int32_t local_ip;
}net_auth_k_t;

typedef struct {
	u_int64_t sn;
	// 只有本项由服务器从客户端收到报文后填写
	u_int32_t callback_id;
	// TP_WEB_SRV/TP_USER/TP_DS007
	u_int8_t ds_type;
}net_vedio_q_t;

typedef struct {
	u_int64_t sn;
	// 本项由设备从net_vedio_q_t中提取，拷贝过来
	u_int32_t callback_id;
	u_int32_t dev_global_ip;
	u_int32_t dev_local_ip;
	u_int16_t url_len;
	/*
		本链接可以直接访问，例：
		http://132.68.7.69:1399/z87zbcde0918.html
	*/
	char url[0];
}net_vedio_a_t;

typedef struct {
	u_int32_t local_ip;
	u_int32_t global_ip;
	u_int16_t local_port;
	u_int16_t global_port;
	u_int8_t request_agent;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t encrypt_count:4,
			vtap_id:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t vtap_id:4,
		      encrypt_count:4;
#else
# error "Please fix <bits/endian.h>"
#endif	
	u_int8_t encrypt_method[0];	
}net_video_syn_q_t;

typedef struct {
	u_int32_t local_ip;
	u_int32_t global_ip;
	u_int16_t local_port;
	u_int16_t global_port;
	u_int8_t encrypt_method;
	u_int8_t ack_period;
	u_int8_t client_id;
	u_int8_t err_number;
	u_int8_t key_rand[16];
	u_int8_t auth_rand[16];
	u_int8_t md5[16];
}net_video_syn_a_t;

typedef struct {
	u_int32_t callback_id;
	u_int32_t dst_ip;
	u_int16_t dst_port;
	u_int16_t callback_port;
	u_int32_t callback_ip;
	u_int8_t reserved[4];
}net_vedio_hello_q_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t callback_ip;
	u_int16_t callback_port;
	u_int16_t reserved1;
	u_int32_t reserved2;
	u_int8_t client_id;
	u_int8_t ack;
	u_int8_t client_type;
	u_int8_t auth_len;
	u_int8_t auth[0];
}net_vedio_hello_t;

typedef struct {
	u_int16_t seq;
	u_int8_t pad_len;
	u_int8_t quality;
	u_int8_t data[0];
}net_vedio_jpg_t;

typedef struct {
	u_int16_t seq;
	u_int8_t pad_len;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	quality:4,
		ver:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t 	ver:4,
		quality:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t total;
	u_int8_t index;
	u_int16_t resv;
	// 时间戳，单位为10毫秒
	u_int32_t timestamp;
	u_int8_t data[0];
} net_video_jpg_v1_t;

typedef struct {
	u_int16_t last_seq;
	u_int16_t rcv_count;
	u_int16_t drop_count;
	u_int8_t client_id;
}net_vedio_ack_t;

typedef struct {
	net_vedio_ack_t v0;
	u_int8_t nack_number;
	u_int16_t nack_seq;
	u_int8_t nack[0];
} net_video_ack_v1_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t device_global_ip;
	u_int32_t user_global_ip;
	u_int16_t cer_length;
	u_int16_t reserve1;
	u_int32_t reserver2;
	u_int8_t  cert[0];
}net_vedio_agent_q_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t device_global_ip;
	u_int32_t user_global_ip;
	u_int16_t cer_length;
	// 0表示失败，其它为代理使用的端口
	u_int16_t  device_global_port;
	u_int32_t reserver2;
	u_int8_t  cert[0];
}net_vedio_agent_a_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t user_global_ip;
	u_int32_t device_global_ip;
	u_int16_t user_global_port;
	u_int16_t device_global_port;
}net_vedio_agent_setup_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
}net_vedio_agent_ok_t;

typedef struct {
	u_int64_t sn;
	u_int8_t dev_pass_md5[16];
}net_bind_v2_t;

typedef struct {
	u_int64_t sn;
	u_int16_t module_id;
	u_int8_t   name_len;
	u_int8_t   pad;
	u_int8_t  name[0];
}net_set_name_v2_t;

typedef struct {
	// 左正右负，0为停止
	char left_right;
	// 上正下负, 0为停止
	char up_down;
	u_int16_t resv;
} net_video_roll_t;

typedef struct {
	u_int8_t client_id;
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t resolved;
	u_int32_t info_len;
	u_int8_t info[0];
} net_user_debug_t;

typedef struct {
	u_int8_t cookie[16];
} network_detect_q_t;

typedef struct {
	u_int8_t cookie[16];
	// 总共回应多少个报文
	u_int16_t total;
	// 这个是第几个，从0开始递增
	u_int16_t index;
	u_int8_t data[0];
} network_detect_a_t;

/* CMD_MISC_Q的MT_VIDEO_IP_LIST子类型 */
typedef struct {
	u_int32_t ip;
	u_int16_t port;
	u_int8_t type;
	u_int8_t priority;
} vil_ele_t;

typedef struct {
	u_int8_t count;
	// 每个成员大小，乘以4
	u_int8_t ele_size;
	u_int16_t resv;
	vil_ele_t ele[0];
} video_ip_list_t;


#define	MF_DEV_LAST	0
#define	MF_DEV_ONLY	1
#define	MF_SVR_ONLY	2

#define	MT_MK(mf, nr)	(((mf)&0xF) << 12 | ((nr)&0xFFF))
#define	MT_FLAG(v) (((v) >> 12) & 0xF)


enum {
	MT_AD =              MT_MK(MF_SVR_ONLY, 0),
	MT_TOTAL_FILTER =    MT_MK(MF_SVR_ONLY, 1)	,
	MF_SVR_ONLY_MAX
};

enum {
	MT_HIS_STAT =        MT_MK(MF_DEV_LAST, 0),
	MT_DEV_FILTER =      MT_MK(MF_DEV_LAST, 1) ,
	MF_DEV_LAST_MAX
};

/* 使用枚举是需要取最大值上报给手机 */
enum {
	MT_DEV_ONLINE =      MT_MK(MF_DEV_ONLY, 0),
	MT_CHILD_ONLINE =    MT_MK(MF_DEV_ONLY, 1),
	MT_REMAIN_TIME =     MT_MK(MF_DEV_ONLY, 2),
	MT_REMAIN_APP =      MT_MK(MF_DEV_ONLY, 3),
	MT_CUR_STAT	=        MT_MK(MF_DEV_ONLY, 4),
	MT_DEV_URL =         MT_MK(MF_DEV_ONLY, 5),
	MT_PLUG_AC =         MT_MK(MF_DEV_ONLY, 6),
	MT_PLUG_V =          MT_MK(MF_DEV_ONLY, 7),
	MT_PLUG_T =          MT_MK(MF_DEV_ONLY, 8),
	MT_PLUG_ON =         MT_MK(MF_DEV_ONLY, 9),
	MT_PLUG_SET_ON =     MT_MK(MF_DEV_ONLY, 10),
	MT_PLUG_SET_OFF =    MT_MK(MF_DEV_ONLY, 11),
	MT_SLAVE_LIST =      MT_MK(MF_DEV_ONLY, 12),
	MT_ENABLE_ITV =      MT_MK(MF_DEV_ONLY, 13),
	MT_DEV_MEM =         MT_MK(MF_DEV_ONLY, 14),
 	MT_DEV_CLIENT =      MT_MK(MF_DEV_ONLY, 15),
	MT_DEV_NI =          MT_MK(MF_DEV_ONLY, 16),
	MT_DEV_SSIDPW  =     MT_MK(MF_DEV_ONLY, 17),
 	MT_DEV_CPU =         MT_MK(MF_DEV_ONLY, 18),
	MT_DISABLE_ITV =     MT_MK(MF_DEV_ONLY, 19),
	MT_VENDOR_ID =       MT_MK(MF_DEV_ONLY, 20),
	MT_VENDOR_URL =      MT_MK(MF_DEV_ONLY, 21),
	MT_SLAVE_LIST_V2 =   MT_MK(MF_DEV_ONLY, 22),
	MT_DEV_VERSION =     MT_MK(MF_DEV_ONLY, 23),
	MT_DEV_RUN_TIME	=    MT_MK(MF_DEV_ONLY, 24),
	MT_DEV_ONLINE_TIME = MT_MK(MF_DEV_ONLY, 25),
	MT_DEV_CONNECT_INTERNET_TIME = MT_MK(MF_DEV_ONLY,26), /* wan口生效后时间 */
	MT_VIDEO_IP_LIST =   MT_MK(MF_DEV_ONLY, 27), 		 /* 可以看视频的地址端口列表 */
	MT_DEV_WIFI =        MT_MK(MF_DEV_ONLY, 28),
	MT_DEV_ENABLE_WIFI = MT_MK(MF_DEV_ONLY, 29),
	MT_DEV_DISABLE_WIFI= MT_MK(MF_DEV_ONLY, 30),
	MT_DEV_ARIA2c =      MT_MK(MF_DEV_ONLY, 31),
	MT_DEV_ENABLE_ARIA2c=MT_MK(MF_DEV_ONLY, 32),
	MT_DEV_DISABLE_ARIA2c=MT_MK(MF_DEV_ONLY,33),
	MT_DEV_MACDENY =     MT_MK(MF_DEV_ONLY, 34),
	MT_DEV_STORAGE_DEVICE=MT_MK(MF_DEV_ONLY,35),
	MT_DEV_MAXCMD_GET =   MT_MK(MF_DEV_ONLY,36),       /*用于手机查询最大的misc命令和最大的网络命令*/
	MT_DEV_GET_EXSUBTYPE =   MT_MK(MF_DEV_LAST,37),       /*查询设备扩展子类型命令，例如003Cam-YH 003Cam-G*/
	MT_DEV_SENSOR = MT_MK(MF_DEV_ONLY, 38),	/*008SH传感器信息*/	
	MT_DEV_GET_TMEP = MT_MK(MF_DEV_ONLY, 39),	/*查询805M获取的温度值*/
	MT_DEV_GET_RH = MT_MK(MF_DEV_ONLY, 40),	/*查询805M获取的湿度值*/
	MT_DEV_GET_PM25 = MT_MK(MF_DEV_ONLY, 41),	/*查询805M获取的PM2.5值*/
	MT_DEV_GET_VOC = MT_MK(MF_DEV_ONLY, 42),	/*查询805M获取的VOC值*/
	MF_DEV_ONLY_MAX
};


/* IP地址类型 */
#define	IPT_UNKNOW	0
#define	IPT_LAN	1
#define	IPT_WAN	2
#define	IPT_MASTER	3
#define	IPT_UPNP	4
#define	IPT_GLOBAL	5

/* 不同类型IP地址的探测优先级 */
#define	PRIO_LAN	10
#define	PRIO_WAN	20
#define	PRIO_MASTER	30
#define	PRIO_UPNP	50
#define	PRIO_GLOBAL	70

#define	MTF_USB_VIDEO_IN	0x01
#define	MTF_ITV_ENABLE	0x01
//倍泰健康设备是否插入
#define MTF_USB_BELTER_IN	0x01
// 支持电流电压温度探测
#define	MTF_VC_DETECT	0x01
//支持电量统计功能
#define	MTF_PLUG_ELECTRIC_STAT	0x02
// 支持003视频录制
#define MTF_VIDEO_RECORD  0x01
//支持视频翻转
#define MTF_VIDEO_FLIP  0x02
//支持视频侦测
#define MTF_VIDEO_DETECT  0x04
//支持自定义分辨率
#define MTF_VIDEO_CUSTOM_DPI  0x08
//不支持云台
#define MTF_VIDEO_UN_SUPPORT_PTZ 0x10

//支持color参数调整
#define MTF_V4L2_COLOR  0x01

//支持云台属性的一系列设置
#define MTF_MOTO_ATTRI_SPEED  0x01 //支持云台旋转速度调整
#define MTF_MOTO_ATTRI_PRE_POSITION  0x02//支持云台预置位
#define MTF_MOTO_ATTRI_CRUISE  0x04//支持云台巡航

// IA子类型
#define MTF_IA_WATERHEATER_SUBTYPE_PROTO	0x00 //热水器原型(万家乐 D40-HG7WF)
#define MTF_IA_WATERHEATER_SUBTYPE_A9		0x01 //前锋热水器

// 需要充值才能使用的功能模块
#define MTF_MONEY	0x80
// 支持取名字的模块
#define	MTF_RENAME	0x40
// 无线可以学习
#define MTF_REMOTE_RF_LEARN 0x01	
// 红外可以学习
#define MTF_REMOTE_IR_LEARN 0x02	
// 至少配置有一个电器
#define MTF_REMOTE_VALID    0x04
// 支持对码功能
#define MTF_REMOTE_GENCODE  0x08
// 支持无线微调
#define MTF_REMOTE_ADJUST 	0x10
// 支持对码脉宽设置
#define MTF_REMOTE_WIDTH   0x20
//支持001E相关功能
#define MTF_REMOTE_001E 0x40
//支持二维码添加电器
#define MTF_REMOTE_TD 0x80

/* 支持一键布防和一键撤防 */
#define MTF_ALARM_SWITCH	0x01

/* 当是i+007E时,在MID_SYSTEM的flag中这个标志位表示是WS-HC-1001设备*/
#define MTF_SYSTEM_WSHC1001 0x01

// 设备模块ID定义
#define	MID_DEV	0
// USB摄像头
#define	MID_USB_VIDEO_MIN	1
#define	MID_USB_VIDEO_MAX	10
// 遥控开关
#define	MID_PLUG_MIN	11
#define	MID_PLUG_MAX	50
// 网络管理、防蹭网等
#define	MID_NET_ADMIN	60
// 绿色上网，家长控制
#define	MID_GNET	100
// 无线摄像头
#define	MID_MJPG_VIDEO_MIN	101
#define	MID_MJPG_VIDEO_MAX	120
#define	MID_H264_VIDEO_MIN	121
#define	MID_H264_VIDEO_MAX	140
#define	MID_ITV	200
// 红外遥控发射
#define	MID_RCONTROL	210
//倍泰健康测量设备
#define	MID_BELTER	280
// 爱家E系列控制模块
#define MID_REMOTE 281
#define MID_ALARM   282
// TO DO: openwrt分支继续使用该宏，下次代码合并时处理
#if (CUSTOM_VENDOR_NAME == OPENWRT)
#define MID_REMOTE_DEV 283
#endif
//情景模式功能支持
#define MID_SCENE_SUPPORT	284
#define MFK_SCENE_TIMER    BIT(0)
#define MFK_SCENE_LINKAGE_ALARM BIT(1)

/*区域功能支持*/
#define AREA_FUNC_SUPPORT 285
#define AREA_ID_SUPPORT 286

#define MID_SYSTEM 287
#define MID_COLOR 288

#define MID_MOTO_ATTRIBUTE 289 //IPC云台属性调整支持 

#define MID_IA_ID 1003 // ia子类型


/* 红外无线控制模块取名字,必须大于1000，否者iphone会虚拟出来主机图标 */
#define MID_REMOTE_IR 1001
#define MID_REMOTE_RF 1002

/* 主机接入的是双向单片机 */
#define MID_REMOTE_BD 290

#define	KF_UKEY_IN	BIT(0)
// 支持视频代理现在
#define	KF_VIDEO_AGENT_SUPPORT	BIT(1)
//upnpc 隧道打通上传服务器
#define	KF_UPNPC_SUPPORT 	BIT(2)

typedef struct {
	u_int32_t flags;
}net_keeplive_v1_t;

typedef struct {
	net_keeplive_v1_t v1;
	// 版本号，m.s格式，设备v0.7开始支持
	u_int16_t ver_master;
	u_int16_t ver_slave;
}net_keeplive_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t passwd[16];
} net_bind_t;

typedef struct {
	u_int64_t sn;
	u_int16_t module_id;
	u_int8_t name_len;
	u_int8_t resv;
	u_int8_t name[0];
} net_set_name_t;

typedef struct {
	u_int8_t old_ssid_len;
	u_int8_t ssid_len;
	u_int8_t pw_len;
	u_int8_t data[0];
} ssidpw_opt_t;

typedef struct {
	u_int16_t num;
	u_int8_t mac[0];
} maclist_t;

#define SX_WOMEN 1 /*女*/
#define SX_MAN 0  /*男*/
#define CAREER_NORMAL 1 /*一般人员*/
#define CARRER_SPORT 2 /*运动员*/
#define CARRER_PROFESSIONAL 0 /*职业人士*/
#define AC_QUERY 0 /* 查询 */
#define AC_ADD 1 /*添加测量人员*/
#define AC_MOD 2 /*修改测量人员*/
#define AC_DEL 3  /*删除测量人员*/


/*测量信息常量定义，参考《2.4G_Hz通讯协议v1.7.1.doc》*/
/*健康测量设备类型*/
#define MDT_WEIGTH 0x01  /*体重计*/
#define MDT_FAT 0x02      /*脂肪仪*/
#define MDT_BLOOD_PRESSURE 0x03  /*血压计*/
#define MDT_PEDOMETER 0x04      /*计步器*/
#define MDT_BLOOD_SUGAR 0x05   /*血糖仪*/
#define MDT_BLOOD_OXYGN 0x06   /*血氧仪*/
#define MDT_EAR_THERMOMETER 0x7 /* 耳温枪 */

/*测量参数类型*/
#define MT_HELLO 0x01 /*握手（Data:0x5a 0xa5 0x5a 0xa5  上位机反馈Data： 0xa5 0x5a 0xa5 0x5a)*/
#define MT_ERROR 0x02 /*错误提示（Data待定）*/
#define MT_FB    0x03 /*反馈信息（Data  0x01：正确。 0x02：错误。 0x03：重发。）*/
#define MT_MESSURE_ING 0x04 /*测量中*/
#define MT_READY 0x05 /*准备就绪可以测量(开始测量)*/
#define MT_POWER_OFF 0x06  /*关机信号（设备关机前发给主机）*/
#define MT_MATCH_OK 0x07  /*对码成功*/
#define MT_POWER_LOW 0x08  /*低电压信号*/
#define MT_MESSURE_STOP 0x09 /*停止测量*/
#define MT_DATE 0x10   /*日期：年（2Byte）、月（1Byte）、日（1Byte）*/
#define MT_TIME 0x11   /*时间：小时（1Byte）、分（1Byte）*/
#define MT_WEIGHT 0x12  /*实时体重（2Byte 0.0kg）、单位（1Byte：0x00 kg，0x01 lb， 0x02 st）*/
#define MT_USER 0x13    /*用户资料（年龄1Byte、身高1Byte、性别（bit7女=1 男=0）&模式（bit6~0）、步距1Byte（0~255cm) ）*/
#define MT_FAT  0x14    /*脂肪率(2Byte 0.0%)*/
#define MT_WATER 0x15   /*水分（2Byte 0.0%）*/
#define MT_MUSCLE 0x16  /*肌肉（2Byte 0.0kg）*/
#define MT_BONE 0x17    /*骨骼（2Byte 0.0%）*/
#define MT_BMI 0x18     /*身体质量指数（Body Mass Index),BMI(2Byte 0.0)*/
#define MT_BM 0x19     /*basic metabolism, 基础代谢KCAl（2Byte 1KCAl）*/
#define MT_M 0x1A      /*metabolism，新陈代谢KCAl（2Byte 1KCAl）*/
#define MT_VISCERAL_FAT 0x1B  /*内脏脂肪等级（2Byte）*/
#define MT_WEIGHT_CF 0x1C /*确定体重（2Byte 0.0kg）、单位（1Byte：0x00 kg，0x01 lb， 0x02 st）体重确定信号在秤LCD闪烁前发送至主机*/
#define MT_PRESSURE_H 0x1D  /*高压值（2Byte 0mmHg）、单位（1Byte：0x00 mmHg，0x01 kPa）*/
#define MT_PRESSURE_l 0x1E  /*低压值（2Byte 0mmHg）、单位（1Byte：0x00 mmHg，0x01 kPa）*/
#define MT_PLUS 0x1F        /*脉搏数（1Byte ）*/
#define MT_WALK_CNT 0x20    /*运动步数,高4Byte（最大步数为99999步）*/
#define MT_WALK_DISTANCE 0x21    /*运动距离高4Byte（最大距离为999.99kms）*/
#define MT_CALORIE 0x22     /*消耗能量KCAL  4Byte（1 KCAL）*/
#define MT_BLOOD_SUGAR 0x23  /*血糖值（2Byte）例如28.5mmol/L 直接发送285   ------血糖仪*/
#define MT_BLOOD_OXYGN 0x24  /*---'血氧饱和度（1Byte）                     ------血氧仪*/
#define MT_BLOOD_PLUS  0x25  /*脉率（1Byte）                               ------血氧仪*/

#define MT_PEDOMETER_1	0x26	/* 计步器第一天数据 */
#define MT_PEDOMETER_2	0x27	/* 计步器第二天数据 */
#define MT_PEDOMETER_3	0x28	/* 计步器第三天数据 */
#define MT_PEDOMETER_4	0x29	/* 计步器第四天数据 */
#define MT_PEDOMETER_5	0x2a	/* 计步器第五天数据 */
#define MT_PEDOMETER_6	0x2b	/* 计步器第六天数据 */
#define MT_PEDOMETER_7	0x2c	/* 计步器第七天数据 */

#define MT_EAR_TEMP			0x44 /* 温度 -- 耳温枪*/

/*家庭成员信息，Family Member info*/
typedef struct{
	u_int16_t    bd_year;   /*出生年，例如1980*/
	u_int16_t    weight;   /*体重，kg*10*/
	u_int8_t    bd_month;  /*出生月, 1到12*/
	u_int8_t    height;     /*身高，cm*/
	u_int8_t    sex;       /*性别，SX_WOMEN or SX_MAN*/
	u_int8_t    career;    /*职业，CAREER_NORMAL */
	u_int8_t    step;      /*步距，cm*/
	u_int8_t    id;        /*家庭成员id，由服务器生成，手机修改/删除成员时要填写*/
	u_int8_t    action;    /*AC_ADD*/
	u_int8_t    is_current;  /*true表示当前测量人员*/
	u_int8_t    reserved[4];  /*保留*/
	u_int8_t    name[MAX_NICKNAME];
}net_fm_t;


/*测量信息数据结构，Messure info*/
typedef struct{
	u_int8_t    mesure_data[4]; /*测量内容，依赖测量类型而定*/
	u_int8_t    mesure_type;   /*测量类型，MT_XXX*/
}messure_para_t;
typedef struct{
	u_int32_t    mtime;   /*测量时间戳*/
	u_int8_t    fm_id;   /*被测量家庭成员id*/
	u_int8_t    md_id;     /*测量设备id，MDT_XXX*/
	u_int8_t    messure_cnt;     /*测量参数个数*/
	u_int8_t    reserved;      /*保留*/
	messure_para_t  messure_para[0];	/*测量参数*/
}net_messure_t;

typedef struct{
	u_int8_t op;	/*MSYN_XX*/
	u_int64_t sn;
	u_int32_t server_id;
	net_messure_t data;
}net_sync_messure_t;

/*查询测量结果条件数据结构，Messure Query*/
typedef struct{
	u_int32_t    begin_time;  /*测量开始时间，0表示不限制*/
	u_int32_t    end_time;   /*测量结束时，0表示不限制*/
	u_int8_t    fm_id;       /*测量人员id，0表示不限制*/
	u_int8_t    mdt;         /*测量类型，MDT_XXX，0表示不限制*/
    u_int8_t    count;       /*查询测量条数，0表示不限制*/
	u_int8_t    reserved[5];  /*保留*/
}net_messure_q_t;

typedef struct {
	u_int8_t id;			/* 策略ID */
	u_int8_t hours;			/* 小时 0-23 */
	u_int8_t minute;		/* 分钟 0-59 */
	u_int8_t week;			/* bit 0-6位对应星期1到星期天 */
	u_int8_t enable;		/* 是否生效(手机设置) 或者已经无效(设备返回) */
	u_int8_t pad;			/* 保留 */
	u_int16_t last;			/* 持续多久(分钟) */
	u_int32_t errorcode;	/* 每个策略的错误信息的 */
	u_int8_t name[MAX_PLUG_TIMER_NAME];		/* 策略名字 */
} net_plug_timer_t;

#define	PTCF_EFFECT	(1<<15)
#define	PTCF_ON	(1<<14)

typedef struct {
	u_int8_t action;		/* 查询添加修改删除 AC_XXX */
	u_int8_t num;			/* 如果是非查询，需要填上个数 */
	u_int16_t next_action;	/* bit15: effect, bit14: on, bt13~bit0: minutes */
	u_int8_t pad[4]; 	/* 保留以后使用 */
	net_plug_timer_t plug_timer[0];	/* 具体策略，个数由num决定 */
} net_plug_timer_config_t;

/*定时录制视频相关数据结构定义*/
#define MAX_REC_TIMER_NAME MAX_PLUG_TIMER_NAME
typedef struct {
	u_int8_t id;	/* 策略ID */
	u_int8_t wday; /* bit 0-6位对应星期天到星期六 */
	u_int8_t hours; /* 小时 0-23 */
	u_int8_t minute; /* 分钟 0-59 */
	u_int8_t stat ; /* 策略的状态 e_rec_item_stat_t */
	u_int8_t item_type;/* 策略类型 由e_rec_item_type_t控制*/
	u_int16_t duration;/* 持续多久(分钟) */
	u_int32_t location_time;/*定位时间，item_type = ePOLICY_ONCE填充，代表执行时间*/
	u_int8_t name[MAX_REC_TIMER_NAME];		/* 策略名字 */
}record_timer_item_t;

typedef struct {
	u_int8_t action;/* 这一次操作的类型 */
	u_int8_t item_num;/* 这一次操作的策略数量 */
	u_int8_t switch_val;/* 录制状态和开关 */
	u_int8_t last_id;/* 用于A报文回馈信息，这次操作的id */
	u_int32_t err_code;/* 用于A报文回馈信息，这一次操作的错误信息 */
	u_int32_t pad;/* 扩展 */
	record_timer_item_t item[0];/* 具体策略，个数由num决定 */
}net_record_timer_config_t;
#define 	MAX_REC_TIMER_NUM   31 /* 策略的最大个数 */

typedef enum {
	REC_STA_INIT = 0,//0: 待机
	REC_STA_ERROR1,//1:挂载失败或者tf卡未找到
	REC_STA_ERROR2,//2:video进程未启动
	REC_STA_READY,//3:准备启动ffmpeg
	REC_STA_RUN,//4:ffmpeg已经启动
}e_record_status;


typedef enum {
	e_REC_QUERY,
	e_REC_ADD,
	e_REC_MOD,
	e_REC_DEL,
	e_REC_SWITCH
}e_rec_timer_action;

typedef enum {
	eITEM_INVALID,
	eITEM_DISABLE,
	eITEM_ENABLE
}e_rec_item_stat_t;

typedef enum {
	ePOLICY_ONCE,
	ePOLICY_WDAY
}e_rec_item_type_t;

/*008电量统计相关数据结构*/
#define 		PLUG_ELECTRIC_STAT_CLS			0
#define 		PLUG_ELECTRIC_STAT_QUERY		1
#define 		PLUG_ELECTRIC_STAT_VERSION		0

typedef struct {
	u_int32_t electric_stat_total;/*从开机到现在的总电量,单位 W*/
	u_int32_t electric_stat_section;/*从time指定时文到当前的阶段电量,单位 W*/
	u_int32_t time;/*指定的某一时刻,好用以记算到当前时间累积的电量*/	
}plug_electric_stat_t;
typedef struct {
	u_int8_t version;/*版本儿号*/
	u_int8_t action;/*执行动作,清除和查询*/	
	u_int16_t pad;/*先放这儿,拿来做对齐用*/
	u_int32_t err_code;/* 用于报文回馈信息，这一次操作的错误与否的信息 */		
	plug_electric_stat_t item;/*统计数据*/
}net_plug_electric_stat_t;


/*遥控相关数据结构定义*/

#define IR_LIST_DIR "soft/ir_config/"

typedef struct {
	u_int32_t id;
	u_int8_t name[256];	//中文/t英文
}ir_list_member_t;

typedef struct {
	u_int32_t errorno;     	// 错误号
	u_int32_t classes_id;  	// 类型ID
	u_int32_t brand_id;  	// 厂商ID
	u_int32_t ir_id;		// 具体遥控设备id
	u_int32_t ver;			// 遥控按键版本号
	u_int32_t n_member;
	ir_list_member_t member[0];
}ir_list_t;

typedef struct {
	u_int32_t err;
	u_int32_t ver;			// 版本信息
	u_int32_t ir_id; 		// 要遥控哪个遥控板
	u_int8_t name[128]; 	// 哪个按键，手机端传英文的名字
}net_ir_test_t;

#define IR_CONFIG_ACT_QUERY 0
#define IR_CONFIG_ACT_ADD 1
#define IR_CONFIG_ACT_MODITY 2
#define IR_CONFIG_ACT_DEL 3

typedef struct {
	u_int32_t ir_id;		// 具体遥控设备id
	u_int32_t action;	// 0:查询 1: 添加 2: 修改 3:删除
	u_int32_t err;
	u_int8_t name[128];
} ir_config_t;

#define MAX_IR_DB_SEG 8000

typedef struct {
	u_int32_t errorno;
    u_int32_t ir_id;
    u_int32_t ver;
    u_int32_t compress_type;	// 0 为非压缩 1 为 gzip 压缩
    u_int32_t index;
    u_int32_t db_len;
    u_int8_t  db[0];   			// 遥控配置文件
}ir_db_t;

typedef struct {
	u_int32_t err;
	u_int32_t version;
	u_int32_t url_len;
	u_int8_t  url[0];
}net_ir_list_url_t;

typedef struct{
	u_int16_t state_id;
	u_int16_t state_value;
}remote_state_t;

typedef struct{
	u_int16_t local_id;		// id for home由设备填写，不可修改
	// 属性
	u_int8_t dev_type;		// REMOTE_TYPE_XXX，见4.2，有手机填写，不可修改
	u_int8_t area_id;
	u_int32_t ability;		// 电器的一些能力，见4.5，由设备填写，不可修改
	u_int32_t ir_id;		// 红外数据库ID
	u_int32_t factory_id;
	u_int32_t time_stamp_id;  // 创建电器的时间戳，由设备填写，不可修改
	u_int8_t name[64];	// nick name
	u_int8_t ir_addr;	//红外转发地址，暂时填0
	u_int8_t alarm_flag;//0无效1打开2关闭
	u_int8_t pad;
	u_int8_t n_state;
	remote_state_t state[0];/*创建时使用，修改报文可以忽略此字段*/
}remote_atrri_t;

typedef struct{
	u_int16_t local_id;		// id for home由设备填写，不可修改
	// 属性
	u_int8_t dev_type;		// REMOTE_TYPE_XXX，见4.2，有手机填写，不可修改
	u_int8_t area_id;
	u_int32_t ability;		// 电器的一些能力，见4.5，由设备填写，不可修改
	u_int32_t ir_id;		// 红外数据库ID
	u_int32_t factory_id;
	u_int32_t time_stamp_id;  // 创建电器的时间戳，由设备填写，不可修改
	u_int8_t name[64];	// nick name
	u_int8_t ir_addr;	//红外转发地址，暂时填0
	u_int8_t alarm_flag;//0无效1打开2关闭
	u_int8_t pad;		// 传递class_id，记录添加的电器类别
	u_int8_t n_state;
	u_int64_t bind_sn;	// 绑定的从设备序列号
	u_int8_t model_name[32];	//型号名称:3D50A3700iD
	remote_state_t state[0];/*创建时使用，修改报文可以忽略此字段*/
}remote_atrri_v2_t;

#ifndef CLIB_HDR
typedef struct{
	u_int32_t err;
	u_int8_t action; // 0:查询 1: 添加 2: 修改 3:删除
	u_int8_t version;
	u_int16_t count;	// remote_attri_t 结构体个数
	remote_atrri_t ctrldev[0];	// remote_attri_t 结构体
}net_remote_t;
#endif

#define REMOTE_KEY_NAME_SIZE 64
typedef struct{
	u_int32_t key_id;		// ID 高位可以设置特殊值,key_id 不能为0
	u_int8_t valid;
	u_int8_t pad[3];
	u_int8_t name[REMOTE_KEY_NAME_SIZE];
}remote_key_attri_t;

typedef struct{
	u_int32_t err;
	u_int8_t action;  // 0:查询 1: 添加 2: 修改 3:删除
	u_int8_t count;  // remote_key_attri_t 结构体个数
	u_int16_t local_id;
	remote_key_attri_t attri[0];	// remote_key_attri_t 结构体
}net_remote_key_t;

typedef struct{
	u_int32_t err;
	u_int8_t action; // 0:学习 1:添加 2:删除 3:修改 4:测试 5:学到编码 6:停止学习
	u_int8_t time_out;	//学习超时时间，单位:10秒
	u_int16_t local_id;
	u_int32_t key_id;	// key_id 为0表示只设置状态
	u_int16_t code_type;	//填写0,表示当前的压缩算法 => 编码类型，红外、无线啥的
	u_int16_t code_len;
	u_int8_t code[0];
}net_remote_code_t;

typedef struct{
	u_int32_t err;
	u_int32_t key_id;
	u_int16_t local_id;
	u_int8_t repeat;	// 重复按键次数
	u_int8_t state_num;
	remote_state_t states[0];
}net_remote_ctrl_t;

typedef struct{
	u_int32_t err;
	u_int8_t action;  // 0:查询 1:添加 2:修改 3:删除
	u_int8_t state_num;
	u_int16_t local_id;
	remote_state_t states[0];
}net_remote_state_t;

typedef struct {
	u_int16_t local_id;
	u_int16_t pad;
} remote_soundlight_list_t;

typedef struct {
	u_int32_t err;
	u_int8_t action;
	u_int8_t onoff;
	u_int8_t count;
	u_int8_t pad;
	u_int16_t local_id;
	u_int16_t timeout;
	remote_soundlight_list_t list[0];
} net_remote_config_soundlight; 

typedef struct {
	u_int32_t err;
	u_int8_t len;
	u_int8_t pad[3];
	u_int8_t value[0];
} net_remote_td_code;

/* CMD_MISC_A */
typedef struct {
	u_int16_t type;
	u_int16_t len;
	u_int8_t data[0];
} misc_hdr_t;

typedef struct {
	u_int64_t sn;
	u_int8_t serials_type;
	u_int8_t dev_type;
	u_int8_t status;
	u_int8_t mod_count;
	u_int16_t name_len;
	u_int16_t resv;
	char name[0];
} slave_hdr_t;

typedef struct {
	u_int16_t mod_id;
	u_int8_t name_len;
	u_int8_t flags;
	char name[0];
} module_hdr_t;


typedef struct{
	u_int32_t err;
	u_int8_t action;//0 查询1添加2修改3删除
	u_int8_t count;
	u_int8_t replace;
	u_int8_t pad;
	u_int8_t phones[0];
}net_alarm_config_phone_t;

typedef struct{
	u_int32_t error;
	u_int16_t local_id;
	u_int8_t action;//0查询1添加2修改
	u_int8_t count;
	u_int8_t phones[0];
}net_alarm_bind_phone_t;

typedef struct{
	u_int32_t error;
	u_int32_t key_id;
	u_int16_t local_id;
	u_int8_t pad[3];
	u_int8_t phone_count;
	u_int16_t msg_len;
	u_int8_t data[0];
}net_alarm_msg_t;

enum
{
	ALARM_LOG_ACT_QUERY = 0,
	ALARM_LOG_ACT_ADD = 1,
	ALARM_LOG_ACT_MOD = 2,
	ALARM_LOG_ACT_DEL = 3,
	ALARM_LOG_ACT_QUERY_VER = 4,
	ALARM_LOG_ACT_DEL_VIDEO = 5,
	ALARM_LOG_ACT_QUERY_VIDEO = 6,
	ALARM_LOG_ACT_PUSH = 7,
	ALARM_LOG_ACT_QUERY_UUIDS = 8,
};

#define MAX_ALARM_LOG_RECORD 100

typedef struct{
	u_int8_t version; /*版本，从0开始*/
	u_int8_t action; /*
					0:查询 
					1:添加 只由网关上传至服务器
					3:删除 
					4: 查询网关最大版本支持 
					5: 删除 同时删除相关录像；alarm_log_t内填写ID
					6: 查询 录像文件是否存在；alarm_log_t内填写ID
					7：推送 由网关或者服务器主动推送至手机端
					8: 查询所有UUID
					*/
	u_int16_t log_num; /*返回报警信息数量*/
	u_int32_t err;
}alarm_log_head_t;

typedef struct{
	alarm_log_head_t alarm_header;
	u_int32_t query_start_time; /*查询记录开始时间*/
	u_int16_t query_start_index; /*从结果中第N条记录返回，用以支持翻页*/
	u_int16_t query_want_num;/*希望返回日志最大数 0:不限制*/
}net_alarm_log_query_t;

typedef struct {
	u_int32_t log_uuid; /*日志全局ID，由智慧网关决定，可用作查询日志录像依据,删除是只填该字段, 删除时:0:清空所有日志*/
	u_int32_t alarm_create_time; /*创建报警器的时间*/
	u_int32_t log_time; /*报警信号触发时间*/
	u_int16_t log_last_time; /*报警信号持续时间，单位:秒*/ 
	u_int16_t alarm_type; /*报警器类型，目前都是134*/
	u_int16_t alarm_factoryid; /*报警器具体类型，手机端在添加电器时填写了*/
	u_int16_t local_id;/*报警器局部ID*/
	u_int8_t has_video; /*是否有录像 0:没有 1:有 2:未知 */
	u_int8_t alarm_name_len; /*报警电器名称长度 包括字符串结尾符号,防止用户删除后再看日志*/
	u_int8_t alarm_msg_len;/*用户配置的报警信息长度, 包括字符串结尾符号*/
	u_int8_t alarm_phone_count;/*报警手机号列表*/
	u_int8_t alarm_name[0]; /* 报警电器名称,后面紧跟报警信息,后面紧跟手机号列表，每个手机号占16字节*/
} alarm_log_t;

#ifndef CLIB_HDR
typedef struct{
	alarm_log_head_t alarm_header;
	alarm_log_t log_data[0];
}net_alarm_log_t;

typedef struct{
	u_int8_t version; /*版本，从0开始*/
	u_int8_t action; /*
					1:添加 只由网关上传至服务器
					3:删除 
					5: 删除 同时删除相关录像；alarm_log_t内填写ID
					*/
	u_int16_t log_num; /*返回报警信息数量*/
	u_int32_t confirm;
	u_int32_t err;
	alarm_log_t log_data[0];
}net_alarm_log_sync_t;
#endif
typedef struct {
	u_int32_t err;
	u_int32_t time_stamp;
}net_time_sync_t;


enum{
	ALARM_SWITCH_QUERY = 0,		/*查询动作，主动询问alarm switch以及设备回复时使用*/
	ALARM_SWITCH_MODIFY = 1,	/*切换alarm switch状态*/
	ALARM_SWITCH_SYNC_QUERY = 2,/*从设备向主设备发起同步请求的*/
	ALARM_SWITCH_SYNC = 3,		/*主设备向绑定的从设备发起的同步通告*/
	ALARM_SWITCH_UNDEFINE
};
/*安防报警总开关控制*/
typedef struct{
	u_int8_t action;	
	u_int8_t value;		/*设备上的安防报警开关状态:0关闭，1开启*/
	u_int16_t pad;
	u_int8_t data[0];
}net_alarm_switch_ctl_t;

/* begin智慧小区 */
/*
数据结构： net_notify_hello_t
CMD_NOTIFY_HELLO 命令参数
由设备发送给小区服务器，采用v2协议，
v2头sn填小区sn，参数mysn填设备自己sn
*/
typedef struct{
	u_int64_t mysn;
	u_int8_t versiona;
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int8_t tlv_data[0];
}net_notify_hello_t;

/*
数据结构： net_notify_hello_ack_t
CMD_NOTIFY_HELLO_ACK 命令参数
由小区服务器发送给设备，采用v2协议，
v2头sn填设备sn，参数mysn填小区自己sn
*/
typedef struct{
	u_int64_t mysn;
	u_int8_t versiona;
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int8_t hello_timer; /*握手时间间隔，缺省10秒*/
	u_int8_t reserved[3];
	u_int8_t tlv_data[0];
}net_notify_hello_ack_t;

#define NF_IS_PUSH 0x01		/*该标志表示是服务器主动推送的，不是查询应答*/
#define NF_HAS_MORE 0x02	/*表示后面还有消息*/
/*
数据结构： net_notify_t
CMD_NOTIFY 命令参数
采用v2协议，v2头sn填写自己sn
*/
typedef struct{
	u_int32_t first_report_id;
	u_int8_t report_count;
	u_int8_t report_expire;		/*消息过期时间，单位天*/
	u_int8_t report_nf_flag;	/*NF_IS_PUSH | NF_HAS_MORE*/
	u_int8_t reserved;
	u_int8_t tlv_data[0]; 		/*net_notify_tlv_t*/
}net_notify_t;

/*
数据结构：net_notify_tlv_t
通告消息TLV，net_notify_t -> tlv_data
*/
typedef struct{
	u_int16_t type; /*NOTIFY_xxx such as NOTIFY_ALARM_LOG*/
	u_int32_t length;
	u_int8_t  value[0];
}net_notify_tlv_t;

enum{/* 通告消息类型定义 */
	NOTIFY_EMERGENCY = 0,		/* value格式: net_notify_value_t*/
	NOTIFY_NORMAL = 1,		/*value格式: net_notify_value_t*/
	NOTIFY_AD = 2,				/*value格式: net_notify_value_t*/
	NOTIFY_ALARM_LOG = 3 ,	/* value格式参考CMD_ALARM_LOG*/
	NOTIFY_MESURE_TRANS = 4,	/* value格式参考CMD_MESURE_TRANS*/
	NOTIFY_FANS_SYSTEM = 5,	/* FansSRV system notify msg */
	NOTIFY_FANS_BUSINESS = 6,	/* FansSRV bussiness notify msg */
	NOTIFY_EMERGENCY_V2 = 7,		/* value格式: net_notify_value_t*/
	NOTIFY_NORMAL_V2 = 8,		/*value格式: net_notify_value_t*/
	NOTIFY_AD_V2 = 9,			/*value格式: net_notify_value_t*/
};

#ifndef CLIB_HDR /* NOT in SDK */
enum{
	FMT_HTTP = 0, /* 消息格式为http */
	FMT_URL = 1, /* 消息格式为url */
	FMT_STRING = 2,  /* 消息格式为字符串 */
};
#endif

/*
数据结构： net_notify_value_t
notify_tlv_t 的type为NOTIFY_EMERGENCY,NOTIFY_NORMAL,	NOTIFY_AD
时，value格式为本数据结构
*/
typedef struct{
	u_int32_t timestamp;
	u_int32_t len;
	u_int8_t notify_msg[0];
}net_notify_value_t;

/*
数据结构： net_notify_value_v2_t
notify_tlv_t 的type为NOTIFY_EMERGENCY_V2,NOTIFY_NORMAL_V2,	NOTIFY_AD_V2
时，value格式为本数据结构
*/
typedef struct{
	u_int32_t timestamp;
	u_int32_t len;
	u_int8_t title_len; /* 消息摘要长度 */
	u_int8_t msg_fmt; /* 消息格式 */
	u_int8_t pad1;
	u_int8_t pad2;
	u_int8_t notify_msg[0]; /* 消息和摘要 */
}net_notify_value_v2_t;

/*
数据结构： net_notify_result_t
CMD_NOTIFY_RESULT 命令参数
采用v2协议，v2头sn自己sn
*/
typedef struct{
	u_int32_t first_report_id;
	u_int8_t report_count;
	u_int8_t result;
	u_int8_t reserved[2];
}net_notify_result_t;

/*
数据结构： net_notify_expect_t
CMD_NOTIFY_EXPECT 命令参数
采用v2协议，v2头sn填写自己sn
*/
typedef struct{
	u_int64_t sn;
	u_int32_t expect_report_id;
}net_notify_expect_t;

/*
数据结构: net_notify_query_t
CMD_NOTIFY_QUERY命令参数
*/
typedef struct{
	u_int64_t sn; /*消息来源者序列号*/
	u_int64_t report_begin; /*查询消息起始id，0表示不限制*/
	u_int64_t report_end; /*查询消息结束id，0表示不限制*/
	u_int8_t is_descending; /*0表示升序查询，1表示降序查询*/
	u_int8_t query_cnt; /*查询条数*/
	u_int8_t reserved[2]; /*保留*/	
}net_notify_query_t;

/*
数据结构： net_notify_center_list_t
CMD_NOTIFY_CENTER_LIST 命令参数
采用v2协议，v2头sn填写自己sn
1、通过本命令，小区物业发把自己的ip/port列表上报给服务器，服务器再推送给设备
2、设备连新接到服务器，服务器把该小区完整的物业服务器推送给设备
*/
typedef struct{
	u_int8_t count;
	u_int8_t element_size;
	u_int8_t reserved[2];
	u_int8_t element_data[0];/*community_center_info_t*/
}net_notify_center_list_t;

/*
数据结构：net_srv_notify_center_list_t
CMD_NOTIFY_CENTER_LIST：设备服务器之间通知小区ip port列表参数
upd报文从一台设备服务器发送到另外一台设备服务器
*/
#define SYNC_NOTIFY_CENTER_LIST 1
#define SYNC_CMT_ADD_DEVICE 2
#define SYNC_CMT_DEL_DEVICE 3
#define NOTIFY_SYNC_MAGIC 0x83f105ad
typedef struct{
	u_int32_t src_srv_id; /*source devserver id*/
	u_int32_t dst_srv_id; /*destination devserver id*/
	u_int32_t dst_ip;  /*destination devserver ip*/
	u_int32_t rand; /*random*/
	u_int32_t magic; /*random ^ NOTIFY_SYNC_MAGIC*/	
	u_int64_t cmt_sn; /*community sn*/
	u_int64_t dev_sn; /*device sn*/
	u_int8_t sync_type; /*SYNC_NOTIFY_CENTER_LIST, SYNC_CMT_ADD_DEVICE...*/
	u_int8_t reserved[3];
	u_int32_t context; 
	u_int8_t up_date[20];
}net_srv_notify_center_list_t;

/*
数据结构： community_center_info_t
小区物业服务器信息，对应net_notify_center_list_t -> element_data
*/
typedef struct{
	u_int64_t sn;
	u_int32_t ip;  /*小区信息发送给手机时，本字段表示小区最大report_id*/
	u_int16_t port;
	u_int8_t  reserved[2];
}community_center_info_t;

/*
数据结构： net_cmt_op_device_t
CMD_CMT_OP_DEVICE 命令参数，设备信息部分
*/
typedef struct{
	u_int64_t device_sn;
	u_int8_t action;            /* AC_ADD / AC_DEL / AC_MOD */
	u_int8_t len_phone;     /* 电话号码长度，多个电话用'\t' 隔开*/
	u_int8_t len_name;      /*  设备名称长度*/
	u_int8_t result;        /* 操作结果, ERR_NONE表示成功*/
	u_int8_t op_data[0];     /* phone + name */
}net_cmt_op_device_t;

/*
数据结构： net_cmt_op_device_t
CMD_CMT_OP_DEVICE 命令参数头
*/
typedef struct{
	u_int32_t total;		/* 总共有多少台设备*/
	u_int16_t current;       /* 当前报文有多少台设备*/
	u_int8_t action;            /* AC_ADD / AC_DEL / AC_MOD */
	u_int8_t result;		
	u_int8_t op_device[0];   /* 设备信息net_cmt_op_device_t */
}net_cmt_op_device_hdr_t;

/* end智慧小区 */
 
/*手机直连设备时，当设备未与设备服务器连接时，设备代理回包结构*/
typedef struct{
	u_int32_t cmd;	/*手机请求的命令*/
	u_int32_t error;
	u_int8_t pad[4];
}direct_link_reply_t;


/************视频质量高级配置、查询相关扩展*************/
typedef struct {
	u_int8_t q;
} video_quality_v1_t;

enum{
	VIDEO_CUSTOM_READ = 0,
	VIDEO_CUSTOM_WRITE = 1,
};

/*
目前支持分辨率:320X240、640X480、1280X720
帧率:5、10、15、20、25
*/
typedef struct {
	video_quality_v1_t v1_hd;
	u_int8_t is_custom;	/*1：是自定义配置,有效域是下面几个值，0:老配置，有效域video_quality_v1_t*/
	u_int8_t action;		/*VIDEO_CUSTOM_READ/VIDEO_CUSTOM_WRITE*/
	u_int8_t fps;/*帧率*/
	u_int16_t width;/*分辨率:宽,如:640*/
	u_int16_t height;/*分辨率:高,如:480*/
	u_int32_t errorno;
} video_quality_v2_t;


/************视屏控制相关通信协议结构*************/
//对sonix 操作的具体选项
enum {
	CMD_SONIX_OK = 0,
	CMD_SONIX_FLIP = 1,		//翻转
	CMD_SONIX_SENSIT = 2,	//灵敏度
};

/*用于传输和显示的灵敏度值*/
#define CLIENT_SENSIT_LOW		(10)		//低
#define CLIENT_SENSIT_MIDDLE		(20)		//中
#define CLIENT_SENSIT_HIGH		(30)		//高

typedef struct {
	u_int32_t sensit;	//灵敏度
	u_int8_t vcr;		//开始录像
	u_int8_t message;	//推送消息
	u_int8_t sms;		//发送短信
	u_int8_t warning;	//声音报警
}detect_t;

typedef struct {
	u_int8_t opt_type;			//opt_type=KOP_WRITE 为设置操作（值为0），opt_type= KOP_READ 为获取操作（值为1）。
	u_int8_t cmd;				//指明对sonix 那个工能进行设置。	
	u_int8_t flip;				//视屏翻转；1翻转，0不翻转
	u_int8_t detect_enabled;	//视屏侦测开关
	detect_t detect;			//视屏侦测
}sonix_opt_t;


/************视屏侦测报警相关通信协议结构*************/
#define MAX_ALARM_MSG_LEN 64
typedef struct {
	u_int8_t action;			/* 0 -- 查询，1 --修改 */
	u_int8_t count;			/* 关联的手机数 */
	u_int8_t pad[2];
	u_int8_t alarm_msg[MAX_ALARM_MSG_LEN];	/* 短信和对送的内容,最多63个字节 */
	u_int8_t phones[0];			/* 一个手机使用16个字节 */
}vca_cfg_t;

#define VCA_QUERY 0
#define VCA_MODIFY 1
#define VCA_DEL 2
#define VCA_QUERY_Q 3	/* 用于主从通信，请求dev_cfg配置 */
#define VCA_QUERY_A 4   /* 用于主从通信，应答dev_cfg */


enum
{
	VTAP_FORMAT_MP4,
};

typedef struct {
	u_int32_t begin_time;
	u_int16_t duration;
	u_int8_t  video_format;
	u_int8_t pad;
}vtap_t;

typedef struct {
	u_int32_t begin_time;
	u_int16_t want_num;
	u_int8_t pad[2];
}net_vtap_list_query_t;

typedef struct {
	u_int16_t seg_num;//列表可能很长，报文总长度可能超过10K，超过接收缓冲区，因此对本//命令报文进行分片处理
	u_int16_t seg_index;//从0开始
	u_int32_t num;
	u_int32_t err;
	vtap_t  tap_list[0];
}net_vtap_list_t;

typedef struct {
	u_int32_t err;
	u_int32_t begin_time;
	u_int16_t duration;
	u_int8_t vtap_id;
	u_int8_t pad;
}net_vtap_req_t;

typedef struct {
	u_int8_t client_id;
	u_int8_t pad[3];
}net_vtap_keepalive_t;

typedef struct {
	u_int32_t err;
	u_int8_t vtap_id;
	u_int8_t pad[3];
	u_int32_t timestap;//单位10毫秒
}net_vtap_timestap_t;

typedef struct {
	u_int32_t err;
	u_int8_t vtap_id;
	u_int8_t pad[3];
}net_vtap_end_t;

typedef struct {
	u_int16_t type;
	u_int16_t len;
	u_int8_t value[0];
}st_tlv;

#define DTYPE_SLAVE_SHARE_NAME (0x1)
#define DTYPE_SLAVE_DESC        (0x2)
#ifndef CLIB_HDR
typedef struct {
	u_int32_t shared_num;  /*已经看过改分享的次数*/
	u_int16_t total_length; /*一个slave信息的长度*/
	u_int8_t dev_type; /*显示分享图标使用*/
	u_int8_t pad;
	u_int64_t dev_sn; /*分享的从设备序列号*/
	u_int16_t province_id;
	u_int16_t town_id;
	u_int16_t category_id;
	u_int16_t pad2;
	st_tlv slave_info[0];
}slave_dev_info;

typedef struct {
	u_int8_t action; /*0:查询 1.设置 2：删除*/
	u_int8_t version; /*版本，扩展用*/
	u_int8_t slave_dev_count; /*从设备个数*/
	u_int8_t pad;
	u_int32_t error; /*错误号*/
	u_int64_t master_sn; /*主设备序号*/
	slave_dev_info sdev_info[0]; /*分享的从设备信息*/
}dev_shard_info;
#endif
typedef struct {
	u_int64_t dev_sn;
	u_int8_t dev_type;
	u_int8_t pad[3];
}dev_shard_count;

typedef struct {
	u_int64_t sn;
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t status;
	u_int8_t module_cnt;
	u_int16_t name_len;
	u_int16_t pad;
	u_int8_t name[0];
}misc_slave_list;

typedef struct {
	u_int16_t module_id;
	u_int8_t name_len;
	u_int8_t flags;
	u_int8_t name[0];
}misc_slave_module;

/* 按键功能通过编码实现的设备对象 */
typedef struct code_obj_s{
	u_int16_t local_id;
	u_int8_t  key_num;
	u_int8_t  pad;
	u_int32_t key_ids[1];
}code_obj_t;

/* 对象类型 */
typedef enum obj_type_e{
	OBJ_TYPE_SWITCH = 0x1,
	OBJ_TYPE_CODE,
}obj_type_t;

#define SCENE_MAX_NAME_LENGTH 64

/* 场景属性结构 */
typedef struct scene_s{
	u_int8_t 	scene_id;		/* 场景id,注意,id必须从5开始,1-4是预置占用 */
	u_int8_t 	image_id;		/* 场景对应的图片id */
	u_int8_t 	flag;/*场景flag*/
	u_int8_t 	pad;
	u_int32_t create_time;	/* 场景创建时间 */
	u_int8_t  name[SCENE_MAX_NAME_LENGTH]; /* 场景名称 */
}scene_t;

#ifndef CLIB_HDR /* NOT in SDK */

/*情景模式所需数据结构*/
/* 只有开关功能的设备对象 */
typedef struct switch_obj_s{
	u_int64_t sn;			/* 设备sn号 */
	u_int16_t mode_id;	/* 模块id号*/
	u_int8_t flag;		/*flag*/
	u_int8_t  action;		/* 设备功能开关 */
	u_int16_t  param_length; /* 参数的长度 */
	u_int16_t pad;/*填充*/
	u_int8_t  param[0];		/* 参数具体值 */
}switch_obj_t;

/* 事件 */
typedef struct event_s{
	u_int16_t  id;			/* 事件id ,在设备上要独一无二,目前最多900多*/
	u_int8_t	pad[2];
	u_int8_t	name[SCENE_MAX_NAME_LENGTH];
	u_int16_t	obj_type;		/*事件操作的对象类型，OBJ_TYPE_XXX*/
	u_int16_t obj_data_size;	/* 对象数据结构的大小 */
	union{
		switch_obj_t switch_obj[0];
		code_obj_t code_obj[0];
	}obj;
}event_t;

/* 场景操作命令报文 */
typedef struct scene_config_s{
	u_int8_t version;		/* 命令版本 */
	u_int8_t action;		/* 操作动作: 0;查询所有情景,不要事件，1:修改和添加，2:删除,3:查询某情景模式下所有事件, 9:执行*/
	u_int8_t  scene_id;	/* 情景id ,为0时表示添加*/
	u_int8_t item_num; /* 情景数目，主要是用于查询返回所有情景数或当前情景的事件数, 0表示所有这个类型所有值 */
	u_int32_t err;		/* 应答错误码，正确这个值填0 */
	union{
		scene_t scene[0];		/* 查询所有情景模式时的返回值 */
		event_t event[0];		/* 所有事件 */
	}item;
}scene_config_t;

/* 情景模式下事件的操作命令报文 */
typedef struct event_config_s{
	u_int8_t version;		/* 命令版本 */
	u_int8_t action;		/* 操作动作: 0;查询(目前只有这个操作) */
	u_int16_t total;	/*返回的事件总算*/
	u_int16_t	 item_num;   /* 请求时请求个数,响应时是响应个数*/
	u_int16_t  index; /*查询所有事件的开始index*/
	u_int32_t  err;		/* 应答错误码，正确这个值填0 */
	event_t event_item[0];
}event_config_t;

#else /* IN SDK */

typedef struct switch_obj_s{
	u_int64_t sn;			/* 设备sn号 */
	u_int16_t mode_id;	/* 模块id号*/
	u_int8_t flag;		/*flag*/
	u_int8_t  action;		/* 设备功能开关 */
	u_int16_t  param_length; /* 参数的长度 */
	u_int16_t pad;/*填充*/
	//u_int8_t  param[0];		/* 参数具体值 */
}switch_obj_t;

/* 场景操作命令报文 */
typedef struct scene_config_s{
	u_int8_t version;		/* 命令版本 */
	u_int8_t action;		/* 操作动作: 0;查询所有情景,不要事件，1:修改和添加，2:删除,3:查询某情景模式下所有事件, 9:执行*/
	u_int8_t  scene_id;	/* 情景id ,为0时表示添加*/
	u_int8_t item_num; /* 情景数目，主要是用于查询返回所有情景数或当前情景的事件数, 0表示所有这个类型所有值 */
	u_int32_t err;		/* 应答错误码，正确这个值填0 */
	scene_t scenes[0];
}scene_config_t;

typedef struct event_s{
	u_int16_t  id;			/* 事件id ,在设备上要独一无二,目前最多900多*/
	u_int8_t	pad[2];
	u_int8_t	name[SCENE_MAX_NAME_LENGTH];
	u_int16_t	obj_type;		/*事件操作的对象类型，OBJ_TYPE_XXX*/
	u_int16_t obj_data_size;	/* 对象数据结构的大小 */
	union{
		switch_obj_t switch_obj;
		code_obj_t code_obj;
	}obj[0];
}event_t;

/* 情景模式下事件的操作命令报文 */
typedef struct event_config_s{
	u_int8_t version;		/* 命令版本 */
	u_int8_t action;		/* 操作动作: 0;查询(目前只有这个操作) */
	u_int16_t total;	/*返回的事件总算*/
	u_int16_t	 item_num;   /* 请求时请求个数,响应时是响应个数*/
	u_int16_t  index; /*查询所有事件的开始index*/
	u_int32_t  err;		/* 应答错误码，正确这个值填0 */
	// event_t event_item[0]; 因为event_t->obj.switch_obj是数组0，这里又是数组0，VC下编译不过
}event_config_t;

#endif


typedef struct{
	u_int8_t member_count;
	u_int8_t member_size;
	u_int8_t pad[2];
}net_udp_echo_hd_t;

typedef struct {
	u_int64_t master_sn;
	u_int64_t slave_sn;
	u_int32_t uptime;
	u_int32_t networktime; 
	u_int32_t connect_time; 
	u_int8_t soft_major;
	u_int8_t soft_minor;
	u_int8_t soft_revison;
	u_int8_t pad1;
	u_int8_t update_major;
	u_int8_t update_minor;
	u_int8_t update_revison;
	u_int8_t pad2;
}net_udp_echo_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int64_t sn;
	char pad[4];
}devserver_list_q_t;

typedef struct{
	u_int32_t id;
	u_int32_t ip;
	u_int16_t port;
	u_int16_t net_delay;
	u_int16_t net_speed;
	u_int8_t isp;
	u_int8_t pad;
}devserver_info_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int32_t device_ip;
	u_int16_t device_port;
	u_int8_t devcie_isp;
	u_int8_t devserver_cnt;
	u_int8_t pad[4];
	devserver_info_t devservers[0];
}devserver_list_a_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int64_t sn;
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int16_t probe_size;
	u_int16_t probe_interval;
	u_int8_t probe_count;
	u_int8_t pad;
}net_probe_q_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int16_t probe_size;
	u_int16_t probe_interval;
	u_int8_t probe_count;
	u_int8_t probe_index;
	u_int8_t pad[4];
	u_int8_t probe_data[0];
}net_probe_a_t;

typedef struct{
	u_int64_t sn;
	u_int8_t devserver_cnt;
	u_int8_t pad[7];
	devserver_info_t devservers[0];	
}dev_location_t;

typedef struct {
	u_int64_t sn;			/*从设备的SN*/
	u_int32_t ip;			/*从设备的IP,如果没有则为0*/
	u_int8_t mac_addr[6];	/*从设备的MAC地址,该功能暂时未实现填0*/
	u_int8_t serials_type;	/*从设备的serials_type*/
	u_int8_t dev_type;		/*从设备的dev_type*/
	u_int8_t bind_errno;	/*绑定操作产生的错误号,0未出错，1密码错误,2已被其它设备绑定*/
	u_int8_t status;		/*0：未绑定，1：绑定中，2：绑定在线，3：绑定离线*/
	u_int8_t pad[2];		/*填充位*/
	u_int64_t other_master_sn;/*bind_errno为2时生效，记录被绑定的主设备SN*/
}bind_slave_info_t;

enum {
	BIND_INFO_Q,	/*请求从设备的绑定信息*/
	BIND_INFO_A		/*回应请求*/
};

typedef struct {
	u_int8_t action;	/*取值见上面*/
	u_int8_t count;		/*action为BIND_INFO_A时生效，指示返回的slave_bind_info_t数量*/
	u_int8_t version;	/*版本号，目前为0*/
	u_int8_t pad;		/*填充位*/
	u_int64_t sn;		/*如果指定SN,设备将只返回指定SN的信息，为0时表示获取主设备扫描到的所有从设备信息*/
	u_int8_t data[0];	/*指示从主设备返回的从设备信息，实际值为bind_slave_info_t*/
}net_bind_info_ctl_t;

enum {
	REBOOT_REQUEST,		/*重启请求*/
	REBOOT_ANSWER,		/*重启应答*/
	REBOOT_REQUEST_FROM_MASTER	/*主设备请求从设备重启*/
};

typedef struct {
	u_int8_t version;	/*版本号，目前为0*/
	u_int8_t action;	/*0表示请求报文，1为应答*/
	u_int8_t flag;		/*如果操作的设备为主设备，并则该flag值为1，则表示除了重启设备本身外还需要重启所有绑定的从设备*/
	u_int8_t pad;		/*填充位*/
	u_int8_t data[0];	
}net_reboot_t;
/************************************************区域功能支持*************************/
#define AREA_MAX_NAME_LENGTH 64

/* 区域属性结构 */
typedef struct area_s{
	u_int8_t area_id;		/* 从1开始，0表示添加*/
	u_int8_t image_id;		/* 场景对应的图片id */
	u_int8_t flag;		
	u_int8_t pad; 
	u_int32_t create_time;	/* 场景创建时间 */
	u_int8_t  name[AREA_MAX_NAME_LENGTH]; /* 区域名称 */
}area_t;

/* 对象类型 */
typedef enum area_obj_type_e{
	AREA_OBJ_TYPE_DEV = 0x1,
	AREA_OBJ_TYPE_REMOTE,
}area_obj_type_t;

/* 从设备对象 */
typedef struct area_dev_obj_s{
	u_int64_t sn;			/* 设备sn号 */
}area_dev_obj_t;

/* 按键功能通过编码实现的设备对象 */
typedef struct  area_remote_obj_s{
	u_int16_t local_id;
	u_int16_t  pad;
}area_remote_obj_t;

#ifndef CLIB_HDR /* NOT in SDK */

/* 数据对象 */
typedef struct area_obj_s{
   u_int8_t obj_type;
   u_int8_t pad;
   u_int16_t obj_data_len;
	union{
		area_dev_obj_t dev_obj[0];
		area_remote_obj_t  remote_obj[0];
	}obj_sub;
}area_obj_t;


/* 区域操作命令报文 */
typedef struct area_config_s{
	u_int8_t version;		/* 命令版本 */
	u_int8_t action;		
	/* 操作动作: 
	0:查询所有区域。
	1:修改和添加区域。
	2:删除区域。
	3:查询指定区域下所有事件。
	 */
	u_int8_t  area_id;	/*区域id ,为0时表示添加*/
	u_int8_t item_num;
 	/* 区域数目，主要是用于查询返回所有区域数或当前区域的设备和电器数, 0表示所有这个类型所有值 */
	u_int32_t err;		/* 应答错误码，正确这个值填0 */
	union{
		area_t area[0];		/* 查询所有区域时的返回值 */
		area_obj_t obj[0];		/* 所有设备和电器列表 */
	}item;
}area_config_t;

#else /* IN SDK */

/* 数据对象 */
typedef struct area_obj_s{
   u_int8_t obj_type;
   u_int8_t pad;
   u_int16_t obj_data_len;
	union{
		area_dev_obj_t dev_obj;
		area_remote_obj_t  remote_obj;
	}obj_sub[0];
}area_obj_t;

// SDK库用的数据结构，主要是VC比较严格
typedef struct area_config_s{
	u_int8_t version;		/* 命令版本 */
	u_int8_t action;
	/* 操作动作:
     0:查询所有区域。
     1:修改和添加区域。
     2:删除区域。
     3:查询指定区域下所有事件。
	 */
	u_int8_t  area_id;	/*区域id ,为0时表示添加*/
	u_int8_t item_num;
 	/* 区域数目，主要是用于查询返回所有区域数或当前区域的设备和电器数, 0表示所有这个类型所有值 */
	u_int32_t err;		/* 应答错误码，正确这个值填0 */
	area_t areas[0];
}area_config_t;
#endif

typedef struct{
	u_int32_t client_ip;
	u_int16_t client_port;
	u_int8_t interval;
	u_int8_t reserved[5];
}static_pic_q_t;

typedef struct{
	u_int16_t width;
	u_int16_t height;
	u_int8_t pad_len;
	u_int8_t pic_type;
	u_int8_t reserved[6];
	u_int8_t pic_data[0];	
}static_pic_a_t;


typedef struct {
	u_int32_t ip;
	u_int16_t port;
	u_int8_t res[2];
} remote_vty_t;



typedef struct{
	u_int32_t begin_time;
	u_int32_t end_time;
	u_int16_t max_entry;
	u_int16_t reserved[2];
}gps_position_q_t;

typedef struct{
	u_int32_t longitude;
	u_int32_t latitude;
	u_int16_t speed;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t e_w:1,/*东西经，0表示东经，1表示西经*/
		s_n:1, /*南北纬，0表示北纬，1表示南纬*/
		pad:6;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t pad:6,
		s_n:1, /*南北纬，0表示北纬，1表示南纬*/
		e_w:1;/*东西经，0表示东经，1表示西经*/
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t reserved;
	u_int32_t timesamp;
}gps_position_a_t;

#define SPEED_CTRL_SET 0 /*设置限速阀值*/
#define SPEED_CTRL_GET 1 /*获取限速阀值*/
#define SPEED_CTRL_CLEAR 2 /*取消限速*/
typedef struct{
	u_int8_t maxspeed;
	u_int8_t ctrl; /*SPEED_CTRL_XXX*/
	u_int8_t reserved[2];
}speed_max_q_t;

typedef struct{
	u_int8_t maxspeed;
	u_int8_t error;
	u_int8_t reserved[2];
}speed_max_a_t;


#define BUS_NUM_MAX 16
#define DRIVER_PHONE 15
#define DRIVER_NAME 64

#define OP_BUSBIND_QUERY 0
#define OP_BUSBIND_SET 1
typedef struct{
	u_int8_t op;
	u_int8_t reserved;
	u_int16_t errorcode;
	u_int8_t bus_num[BUS_NUM_MAX];
	u_int8_t driver_name[DRIVER_NAME];//UTF-8
	u_int8_t driver_phone[DRIVER_PHONE];
	u_int8_t cam_count;
	u_int64_t cam_sn[0];
}schlbus_bind_t;

/********************** 语音支持  **********************/

typedef struct {
	u_int16_t seq;
	u_int8_t pad_len;
#if __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t ver:4,
		rate:4;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t rate:4,
		ver:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t total;
	u_int8_t index;
	u_int8_t format;
	u_int8_t client_id;
	// 声道数
	u_int8_t channel;
	// 位数
	u_int8_t bit;
	u_int16_t resv;
	u_int32_t timestamp;
	u_int8_t data[0];
} net_voice_t;

#define	AUDIO_FMT_PCM	0
#define	AUDIO_FMT_ILBC 1

#define	VOICE_RETE_8000	0
#define	VOICE_RETE_16000	1
#define	VOICE_RETE_32000	2
#define	VOICE_RETE_44100	3
#define	VOICE_RETE_48000	4

typedef struct {
	u_int8_t resv[6];
	u_int8_t client_id;
	u_int8_t nack_number;
	u_int16_t nack_seq;
	u_int8_t nack[0];
} net_voice_ack_t;

typedef struct {
	u_int8_t client_id;
	u_int8_t operate;
	u_int16_t resv;
} net_speek_q_t;

typedef struct {
	u_int32_t err_number;
	u_int8_t operate;
	u_int8_t timeout;
	u_int16_t resv;
} net_speek_a_t;

// 释放发言权
#define	SPEEK_OP_RELEASE	0
// 申请发言权
#define	SPEEK_OP_REQUEST	1
#define	SPEEK_OP_UNKNOW	0xFF

typedef struct {
	u_int32_t code_id;
	u_int32_t brand_id;		/*电器厂商id*/
	u_int8_t model_name[32]; 	/*电器型号名称*/
	u_int8_t class_id;			/*电器类别id，如电视，机顶盒，空调等*/
	u_int8_t flag;				/*上传标志:自动上传的标志为0,后台手动上传为1*/
	u_int16_t has_learn;				/*pad*/
	u_int32_t len;				/*后续编码长度*/
	u_int8_t data[0];	
}net_remote_upload_t;

typedef struct {
	u_int32_t cmd_max;/*最大网络命令*/
	u_int32_t misc_max;/*最大misc命令*/
}cmd_max_t;

typedef struct {
	u_int32_t err;
	u_int8_t action;	/* 1: 绑定 2:解绑定 */
	u_int8_t pad;
	u_int16_t local_id;	/* 电器的ID */ 
} net_remote_bd_bind;
/*************************************************************************************/

/********************************dns 报文****************************************************/
typedef struct dns_header //DNS数据报：
{
	u_int16_t id; //标识，通过它客户端可以将DNS的请求与应答相匹配；
	u_int16_t flags; //标志：[ QR | opcode | AA| TC| RD| RA | zero | rcode ]
	u_int16_t quests; //问题数目；
	u_int16_t answers; //资源记录数目；
	u_int16_t author; //授权资源记录数目；
	u_int16_t addition; //额外资源记录数目；
}DNS_HEADER; //在16位的标志中：QR位判断是查询/响应报文，opcode区别查询类型，AA判断是否为授权回答，TC判断是否可截断，RD判断是否期望递归查询，RA判断是否为可用递归，zero必须为0，rcode为返回码字段。


typedef struct dns_query //DNS查询数据报：
{
	u_int16_t type; //查询类型，大约有20个不同的类型
	u_int16_t classes; //查询类,通常是A类既查询IP地址。
}DNS_QUERY;

typedef struct dns_response //DNS响应数据报：
{
	u_int16_t name; //查询的域名
	u_int16_t type; //查询类型
	u_int16_t classes; //类型码
	u_int32_t ttl; //生存时间
	u_int16_t length; //资源数据长度
}DNS_RESPONSE;
/*************************************************************************************/

/*以下为情景模式定时器相关操作的网络报文*/
enum {
	SCENE_TIMER_QUERY = 0,	/*查询这个情景模式所有定时器*/
	SCENE_TIMER_ADD = 1,	/*添加一个定时器*/
	SCENE_TIMER_MODIFY = 2,	/*修改定时器的属性*/
	SCENE_TIMER_DEL = 3,	/*删除定时器*/
	SCENE_TIMER_COVER = 4	/*覆盖操作，删除之前的定时器，以手机传过来的为准*/
};

typedef struct {
	u_int8_t id;			/*定时器ID，由设备统一维护*/
	u_int8_t hour;			/*每小时*/
	u_int8_t minute;		/*每分钟*/
	u_int8_t week;			/*每周*/
	u_int8_t enable;		/*是否启用该定时器*/
	u_int8_t pad[3];		/*填充位*/
	u_int8_t name[MAX_PLUG_TIMER_NAME];	/*定时器名称*/
}net_scene_timer_t;

typedef struct {
	u_int32_t errorcode;		/*错误号，如ERR_SCENE_TIMER_LIMIT */
	u_int8_t version;			/*从0开始，目前为0*/
	u_int8_t action;			/*取值见上面的枚举值如:SCENE_TIMER_QUERY*/
	u_int16_t scene_id;			/*情景模式中的一个情景的ID*/
	u_int8_t num;				/*后面跟的timer数量*/
	u_int8_t pad;				/*填充位*/
	u_int16_t next_execute_time;/*本情景下次将要执行的时间相对于当前的time_t差值,如果有多个定时器返回最近要执行的时间*/
	net_scene_timer_t timer[0];	/*定时器，一个情景可能有多个定时器*/	
}net_scene_timer_hd_t;

#define SCENE_LINKAGE_TYPE_ALARM 1

typedef struct {
	u_int16_t local_id;
	u_int16_t flag;
} scene_alarm_t;

#ifndef CLIB_HDR
typedef struct {
	u_int8_t scene_id;
	u_int8_t type; // SCENE_LINKAGE_TYPE_XXX
	u_int8_t len;
	u_int8_t pad;

	u_int8_t priv[0];	// scene_alarm_t
} scene_linkage_t;
#else /* IN SDK */
typedef struct {
	u_int8_t scene_id;
	u_int8_t type; // SCENE_LINKAGE_TYPE_XXX
	u_int8_t len;
	u_int8_t pad;
	//u_int8_t priv[0];	// scene_alarm_t
} scene_linkage_t;
#endif

typedef struct {
	u_int32_t err;
	u_int8_t action;
	u_int8_t count;
	u_int8_t query_type;
	u_int8_t pad;
	scene_linkage_t sl[0];
} net_scene_linkage_t;

typedef struct{
	u_int8_t action; /*AC_ADD AC_DEL*/
	u_int8_t msg_len; /*推送提示内容长度*/
	u_int16_t token_len; /*token长度*/
	u_int8_t need_push; /*是否订阅，0表示不订阅*/
	u_int8_t reserved[3];
	u_int8_t  phone_ver[8];/*手机系统版本，如"7.1"*/
	u_int64_t sn;
	u_int8_t token[0]; /*token内容，如果msg_len大于0，token后面是msg内容*/
}net_phone_push_t;

#define	MAX_OEM_STR	16
#define UP_DATE_LEN 12
enum {
	UPDESC_LANG_ZH = 1,
	UPDESC_LANG_EN = 2
};

typedef struct{
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t lang;		/*新特性介绍语言，1为中文，2为英文*/
	u_int8_t pad[4];
	u_int8_t oem_id[MAX_OEM_STR];
}net_newupver_q_t;

typedef struct{
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t pad;
	u_int8_t oem_id[MAX_OEM_STR];
}net_set_newupver_t;

typedef struct{
	u_int8_t umajor;
	u_int8_t uminor;
	u_int8_t urevise;
	u_int8_t pad1;
	u_int8_t fmajor;
	u_int8_t fminor;
	u_int8_t frevise;
	u_int8_t pad2;
	u_int32_t errorcode;
	u_int8_t date[UP_DATE_LEN];/*新版本发布时间，如：2014-07-10*/
	u_int16_t desc_len;	/*新版本特性描述长度，最大长度为1000，为0表示不提供新特性描述*/
	u_int16_t url_len;	/*固件升级url地址长度，最大长度为1000，为0表示不提供url地址*/
	/*
	desc	新版本特性描述，语言由net_newupver_q_t的lang决定，中文为utf-8编码,长度为desc_len
	url		提示用户固件升级的url地址，长度为url_len
	*/
}net_newupver_a_t;


/* v4l2 color 相关数据 */
#define V4L2_COLOR_VERSION_V1   0

typedef enum {
	e_V4L2_QUERY,
	e_V4L2_CTRL,
} e_v4l2_action_t;

typedef struct {
	u_int8_t ver;
	u_int8_t action; //类型由e_v4l2_action_t赋值
	u_int8_t pad[2];
	u_int32_t err;
	u_int32_t brightness_val;
	u_int32_t contrast_val;
	u_int32_t saturation_val;
	u_int32_t gain_val;
} net_v4l2_color_t;


/* 云台属性相关数据 */
#define MOTO_ATTRI_VERSION_V1   0

typedef enum {
	e_MOTO_ATTRI_QUERY  =  0,
	e_MOTO_ATTRI_CTRL  =  1,
} e_moto_attri_action;

typedef struct net_moto_attri_s{
	u_int8_t ver;
	u_int8_t action; 
	u_int8_t pad[2];
	u_int32_t err;
	u_int8_t roll_speed;
	u_int8_t  resv8b;
	u_int16_t  resv16b;
	u_int32_t  resv32b;
} net_moto_attri_t;
/**/

/* 云台预置位相关数据 */
#define MOTO_PREPOSITION_VERSION_V1   0

typedef enum {
	e_MOTO_PREPOSITION_CALL  =  0,
	e_MOTO_PREPOSITION_CONFIG  =  1,
} e_moto_prepostion_action;


typedef struct net_moto_prepostion_s{
	u_int8_t ver;
	u_int8_t action; 
	u_int8_t point_index;
	u_int8_t pad;
	u_int32_t err;
	u_int32_t  resv32b;
} net_moto_prepostion_t;
/**/

/* 云台巡航相关数据 */
#define MOTO_CRUISE_VERSION_V1   0

typedef enum {
	e_MOTO_CRUISE_QUERY  =  0,
	e_MOTO_CRUISE_CTRL  =  1,
} e_moto_cruise_action;

typedef enum {
	e_MOTO_CRUISE_TYPE_NOT_ROLL  =  0,
	e_MOTO_CRUISE_TYPE_LR_ROLL  =  1,
	e_MOTO_CRUISE_TYPE_UD_ROLL  =  2,
	e_MOTO_CRUISE_TYPE_LINE_1_ROLL  =  3,
	e_MOTO_CRUISE_TYPE_LINE_2_ROLL  =  4,
	e_MOTO_CRUISE_TYPE_NOT_LR_ROLL  =  5,
	e_MOTO_CRUISE_TYPE_NOT_UD_ROLL  =  6,
} e_moto_cruise_type;

typedef struct prepos_point_node_s{
	u_int8_t  index;
	u_int8_t  point_index; 
	u_int16_t  stop_time;
} prepos_point_node_t;

typedef struct net_moto_cruise_s{
	u_int8_t ver;
	u_int8_t action; 
	/*巡航的类型:关闭巡航、左右巡航、上下巡航、自定义巡航线路一、自定义巡航线路二*/
	u_int8_t type;
	/*两条自定义巡航路径所配置预置点占用的长度和*/
	u_int8_t len; 
	u_int32_t err;
	u_int8_t line_1_point_num;
	u_int8_t line_2_point_num;
	u_int16_t  pad;
	prepos_point_node_t  prepos_point[0];
	
} net_moto_cruise_t;
/**/


enum {
	IA_ACTION_QUERY,
	IA_ACTION_CTRL
};

typedef struct {
	u_int16_t id;
	u_int16_t value;
} ia_status_t;

typedef struct {
	u_int32_t err;
	u_int8_t action;	//  IA_ACTION_XXX
	u_int8_t ns;
	u_int16_t pad;

	ia_status_t is[0];
} net_ia_t;

// 智能家居状态ID16位，分成两部分，高8位表示类型，比如属于空调遥控器啥的，低8位表示具体ID
#define	IA_MK(type, nr)	(((type)&0xFF) << 8 | ((nr)&0xFF))
#define	IA_TYPE(type) (((type) >> 8) & 0xFF)

// 智能家居类型
#define IA_TYPE_AIRCONDITION 1
#define IA_TYPE_WATERHEATER 2
#define IA_TYPE_AIRHEATER 3
#define IA_TYPE_AIRCLEANER 4
#define IA_TYPE_ELECTRICFAN 5
#define IA_TYPE_BATHROOMMASTER 6
#define	IA_TYPE_EB	7 /* E宝 */
#define	IA_TYPE_EB_II	8 /* E宝II */
#define	IA_TYPE_RFGW	9 /* 2.4G RF Gateway*/


//电风扇状态枚举
typedef enum {
	IA_ELECTRICFAN_STATUS_WORK = IA_MK(IA_TYPE_ELECTRICFAN, 1),	// 工作状态: 0 待机 1 工作
	IA_ELECTRICFAN_STATUS_GEAR = IA_MK(IA_TYPE_ELECTRICFAN, 2),	// 风量 1: 睡眠档 2: 低风档 3: 中风档 4: 强风档
	IA_ELECTRICFAN_STATUS_TIMER = IA_MK(IA_TYPE_ELECTRICFAN, 3),// 设置定时器，分钟为单位
	IA_ELECTRICFAN_STATUS_SHAKE = IA_MK(IA_TYPE_ELECTRICFAN, 4),// 风扇摆头 0 关闭 1开启
	IA_ELECTRICFAN_STATUS_POWER = IA_MK(IA_TYPE_ELECTRICFAN, 5), // 当前功率
} IA_ELECTRICFAN_STATUS_T;


// 空调状态枚举
typedef enum {
	IA_AIRCONDITION_STATUS_ONOFF = IA_MK(IA_TYPE_AIRCONDITION, 1),	// 工作状态 0 待机 1 工作中
	IA_AIRCONDITION_STATUS_MODE = IA_MK(IA_TYPE_AIRCONDITION, 2),	// 模式
	IA_AIRCONDITION_STATUS_TEMP = IA_MK(IA_TYPE_AIRCONDITION, 3),	// 设置温度
	IA_AIRCONDITION_STATUS_POWER = IA_MK(IA_TYPE_AIRCONDITION, 4),	// 显示功率
	IA_AIRCONDITION_STATUS_TIMER = IA_MK(IA_TYPE_AIRCONDITION, 5),	// 定时分钟
	IA_AIRCONDITION_STATUS_CUR_TEMP = IA_MK(IA_TYPE_AIRCONDITION, 6),	// 当前温度
} IA_AIRCONDITION_STATUS_T;

/* 		热水器状态枚举 		*/
typedef enum {
	IA_WATERHEATER_STATUS_WORK = IA_MK(IA_TYPE_WATERHEATER, 1),			// 工作状态 0 待机 1 工作中
	IA_WATERHEATER_STATUS_TEMP_SET = IA_MK(IA_TYPE_WATERHEATER, 2),		// 设置水温 范围35-75
	IA_WATERHEATER_STATUS_TEMP_CURRENT = IA_MK(IA_TYPE_WATERHEATER, 3),	// 当前水温
	IA_WATERHEATER_STATUS_TIMER = IA_MK(IA_TYPE_WATERHEATER, 4),	// 预约定时分钟(剩余)
	IA_WATERHEATER_STATUS_CAPACTITY = IA_MK(IA_TYPE_WATERHEATER, 5), // 容量 1 半胆 2 全胆
	IA_WATERHEATER_STATUS_POWER	= IA_MK(IA_TYPE_WATERHEATER, 6),	// 功率
} IA_WATERHEATER_STATUS_T;

// 热水器状态枚举数值意义
typedef enum {
	IA_WATERHEATER_STATUS_WORK_VALUE_SLEEP,
	IA_WATERHEATER_STATUS_WORK_VALUE_WORK,
} IA_WATERHEATER_STATUS_WORK_VALUE_T;

typedef enum {
	IA_WATERHEATER_A9_STATUS_TEMP_SET = IA_MK(IA_TYPE_WATERHEATER, 1),			// 设置水温 35-65
	IA_WATERHEATER_A9_STATUS_TEMP_CURRENT = IA_MK(IA_TYPE_WATERHEATER, 2),		// 当前水温
	IA_WATERHEATER_A9_STATUS_MODE = IA_MK(IA_TYPE_WATERHEATER, 3),	// 功能: 1 自动 2 浴缸 3 洗碗 4 洗菜 5 洗衣
	IA_WATERHEATER_A9_STATUS_WORK = IA_MK(IA_TYPE_WATERHEATER, 4),	// 工作状态 bit 表示 bit0 风机是否开启 1 水流是否开启 2 是否燃烧 3 是否超量小 4 是否超量大 
	IA_WATERHEATER_A9_STATUS_FIRE_LEVEL = IA_MK(IA_TYPE_WATERHEATER, 5), // 燃烧分段 0 不燃烧 1 左然然 2 右燃烧 3 全燃烧
	IA_WATERHEATER_A9_STATUS_COUNT	= IA_MK(IA_TYPE_WATERHEATER, 6),	// 累计水流量，单位L
	IA_WATERHEATER_A9_STATUS_GAS	= IA_MK(IA_TYPE_WATERHEATER, 7),	// 耗气量
} IA_WATERHEATER_A9_STATUS_T;



//快热炉状态枚举
typedef enum {
	IA_AIRHEATER_STATUS_ONOFF = IA_MK(IA_TYPE_AIRHEATER, 1),
	IA_AIRHEATER_STATUS_GEAR = IA_MK(IA_TYPE_AIRHEATER, 2),
	IA_AIRHEATER_STATUS_TIME = IA_MK(IA_TYPE_AIRHEATER, 3),
	IA_AIRHEATER_STATUS_MODE = IA_MK(IA_TYPE_AIRHEATER, 4),
	IA_AIRHEATER_STATUS_POWER = IA_MK(IA_TYPE_AIRHEATER, 5),
	IA_AIRHEATER_STATUS_TEMP = IA_MK(IA_TYPE_AIRHEATER, 6),		// 当前温度，单位0.1摄氏度
} IA_AIRHEATER_STATUS_T;



// 联创油汀
typedef enum {
	IA_AIRHEATER_YCYT_STATUS_TEMP_SET = IA_MK(IA_TYPE_AIRHEATER, 1), 	// 设置温度 1-35
	IA_AIRHEATER_YCYT_STATUS_TEMP_CURRENT = IA_MK(IA_TYPE_AIRHEATER, 2), 	// 当前温度 0-51
	IA_AIRHEATER_YCYT_STATUS_MODE = IA_MK(IA_TYPE_AIRHEATER, 3),		// 模式 1 睡眠 2 省电 3 舒适 4 速热 5 温控
	IA_AIRHEATER_YCYT_STATUS_GEAR = IA_MK(IA_TYPE_AIRHEATER, 4),		//  档位 1 关闭 2 低 3 中 4 高
	IA_AIRHEATER_YCYT_STATUS_ORDER_TIME = IA_MK(IA_TYPE_AIRHEATER, 5),		// 预约开机
	IA_AIRHEATER_YCYT_STATUS_WORK = IA_MK(IA_TYPE_AIRHEATER, 6),		//开关状态
	IA_AIRHEATER_YCYT_STATUS_TIME = IA_MK(IA_TYPE_AIRHEATER, 7),		// 定时关机
	IA_AIRHEATER_YCYT_REFRESH_TIMER = IA_MK(IA_TYPE_AIRHEATER, 8),		// 定时关机
} IA_AIRHEATER_YCYT_STATUS_T;


// 空气净化器状态枚举
typedef enum {
	IA_AIRCLEANER_STATUS_WORK = IA_MK(IA_TYPE_AIRCLEANER, 1),	// 工作状态: 0 待机 1 工作
	IA_AIRCLEANER_STATUS_SPEED = IA_MK(IA_TYPE_AIRCLEANER, 2),	// 风速 1低 2 中 3 高
	IA_AIRCLEANER_STATUS_TIMER = IA_MK(IA_TYPE_AIRCLEANER, 3),// 设置定时器，分钟为单位
	IA_AIRCLEANER_STATUS_ULTRAVIOLET = IA_MK(IA_TYPE_AIRCLEANER, 4),// 紫外线 0 关闭 1开启
	IA_AIRCLEANER_STATUS_ANION = IA_MK(IA_TYPE_AIRCLEANER, 5),		// 负离子 0 关闭 1开启
	IA_AIRCLEANER_STATUS_PM25 = IA_MK(IA_TYPE_AIRCLEANER, 6),		// 当前pm2.5
	IA_AIRCLEANER_STATUS_TEMP = IA_MK(IA_TYPE_AIRCLEANER, 7),		// 当前温度
	IA_AIRCLEANER_STATUS_RH = IA_MK(IA_TYPE_AIRCLEANER, 8),			// 当前湿度
	IA_AIRCLEANER_STATUS_POWER = IA_MK(IA_TYPE_AIRCLEANER, 9),		// 当前功率
	IA_AIRCLEANER_STATUS_MODE = IA_MK(IA_TYPE_AIRCLEANER, 10),		// 设置工作模式
	IA_AIRCLEANER_STATUS_QUERY = IA_MK(IA_TYPE_AIRCLEANER, 11),		// 查询所有工作状态
	IA_AIRCLEANER_STATUS_TERILIZE = IA_MK(IA_TYPE_AIRCLEANER, 12),	// 臭氧杀菌
	IA_AIRCLEANER_STATUS_PERIODIC_TIMER = IA_MK(IA_TYPE_AIRCLEANER, 13),	// 周期定时器
} IA_AIRCLEANER_STATUS_T;

// 浴霸样机
typedef enum {
	IA_BATHROOMMASTER_STATUS_ONOFF = IA_MK(IA_TYPE_BATHROOMMASTER, 1),			// 通电开关 0-关，1-开
	IA_BATHROOMMASTER_STATUS_NEGATIVEIONS = IA_MK(IA_TYPE_BATHROOMMASTER, 2),	// 负离子   0-关，1-开
	IA_BATHROOMMASTER_STATUS_LIGHT = IA_MK(IA_TYPE_BATHROOMMASTER, 3),			// 照明     0-关，1-开
	IA_BATHROOMMASTER_STATUS_AIR = IA_MK(IA_TYPE_BATHROOMMASTER, 4), 			// 换气     0-关，1-开
	IA_BATHROOMMASTER_STATUS_DRY = IA_MK(IA_TYPE_BATHROOMMASTER, 5),			// 除湿     0-关，1-开
	IA_BATHROOMMASTER_STATUS_WARNM = IA_MK(IA_TYPE_BATHROOMMASTER, 6), 			// 风暖     0-关，1-开
	IA_BATHROOMMASTER_STATUS_TIMER = IA_MK(IA_TYPE_BATHROOMMASTER, 7),			// 定时 (单位分钟)
}IA_BATHROOMMASTER_STATUS_T;


typedef enum {
	CMS_IDLE = 0,
	CMS_WAIT_IR = 1,
	CMS_WAIT_MATCH = 2,
	CMS_WAIT_CODE = 3,
	CMS_DONE = 4
} CMS_T;

typedef enum {
	CMA_REPORT = 1,
	CMA_REQUEST,
	CMA_CANCLE,
	CMA_SET,
} CMA_T;

typedef enum {
	CMT_AC = 0,		// 云空调
	CMT_TV = 1,		// 云电视
	CMT_STB = 2,	// 云机顶盒
} CMT_T;

// 云识别相关
typedef struct {
	u_int32_t err;
	u_int8_t action;	// 1: 通知 2: 请求云匹配 3: 取消云匹配
	u_int8_t status;	// 当前状态 CMS_XXX
	u_int16_t local_id;
	u_int8_t type;	// 0: 云空调匹配 ...
	u_int8_t match_id_num;	// 匹配到多少个，最多4
	u_int16_t set_id;	// 手机端用，用来设置一个ID号
	u_int16_t match_id[4];	// 返回的最多4个匹配到的ID号
} net_cloud_match_t;

typedef struct {
	u_int16_t id;
	u_int16_t start_idx;
	u_int16_t end_idx;
	u_int16_t pad;
} net_cloud_match_ac_data_t;

typedef struct {
	u_int8_t err;
	u_int8_t pad;
	u_int16_t lib_id;
	u_int8_t lib_name_len;
} net_cloud_match_ac_info_t;


typedef struct {
	u_int8_t on_off;
	u_int8_t resv[3];
} net_eb_work_t;

/*Dispatch 报文添加的TLV*/

#define	DISP_TLV_DEV_TYPE	1

typedef struct{
	u_int8_t type;
	u_int8_t subtype;
	u_int8_t ext_type;
	u_int8_t pad;
}tlv_devtype_t;

typedef enum {
	NET_805_CONFIG_QUERY,//查询
	NET_805_CONFIG_CTL//控制
} net_805_config_cmd_t;

typedef enum {
	NET_805_BEEP,//蜂鸣器
	NET_805_SCREEN//点阵屏
} net_805_config_type_t;

typedef struct {
	u_int32_t err;//错误号
	u_int8_t cmd;//命令
	u_int8_t pad;
} net_805_config_hdr_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t pad;
	u_int16_t pad2;
} net_805_beep_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t pad;
	u_int16_t pad2;
} net_805_screen_t;


/*通信协议数据结构1字节对齐，新数据结构添加在本行之前*/
#pragma pack(pop)

#ifdef WIN32
static __inline u_int64_t ntoh_ll(u_int64_t n)
#else
static inline u_int64_t ntoh_ll(u_int64_t n)
#endif
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	return (((n>>56)&0xFFULL)
			|((n>>40)&0xFF00ULL)
			|((n>>24)&0xFF0000ULL)
			|((n>>8)&0xFF000000ULL)
			|((n<<8)&0xFF00000000ULL)
			|((n<<24)&0xFF0000000000ULL)
			|((n<<40)&0xFF000000000000ULL)
			|((n<<56)&0xFF00000000000000ULL));
#elif __BYTE_ORDER == __BIG_ENDIAN
	return n;
#else
# error "Please fix <bits/endian.h>"
#endif
}
#ifdef CLIB_HDR
#else
#include "diy_device.h"
#endif

#endif

