#ifndef	__CL_USER_H__
#define	__CL_USER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_ia.h"
#include "cl_intelligent_forward.h"
#include "cl_rfgw.h"
#include "cl_lanusers.h"
//#include "cl_linkage.h"

/*******************************************

	用户相关操作
	module: user
	level: public
		
********************************************/


/* user callback event */
enum {
	UE_BEGIN = 0,
	UE_LOGOUT = UE_BEGIN + 1,
	UE_LOGIN_OFFLINE = UE_BEGIN + 2,
	UE_LOGIN_ONLINE = UE_BEGIN + 3,
	UE_INFO_MODIFY = UE_BEGIN + 4,
	UE_MODIFY_PASSWD_OK = UE_BEGIN + 5,
	UE_MODIFY_PASSWD_FAIL = UE_BEGIN + 6,
	UE_MODIFY_NICKNAME_OK = UE_BEGIN + 7,
	UE_MODIFY_NICKNAME_FAIL = UE_BEGIN + 8,
	UE_MODIFY_ALARM_PHONE_OK = UE_BEGIN + 9,
	UE_MODIFY_ALARM_PHONE_FAIL = UE_BEGIN + 10,
	UE_LOGIN_ERROR = UE_BEGIN + 11,
	UE_PHONE_USER_REGISTER_OK = UE_BEGIN + 12,
	UE_PHONE_USER_REGISTER_FAILED = UE_BEGIN + 13,
	UE_PHONE_USER_RESET_OK = UE_BEGIN + 14,
	UE_PHONE_USER_RESET_FAILED = UE_BEGIN + 15,
	UE_PHONE_USER_GOOD_VCODE = UE_BEGIN + 16,
	UE_PHONE_USER_BAD_VCODE = UE_BEGIN + 17,
	/*提交绑定手机申请成功*/
	UE_BIND_PHONE_REQUEST_OK = UE_BEGIN + 18,
	/*提交绑定手机申请成功，已绑定*/
	UE_BIND_PHONE_OK = UE_BEGIN + 19,
	/*收到绑定手机申请列表*/
	UE_BIND_PHONE_REQUEST_LIST = UE_BEGIN + 20,
	/*收到已绑定手机列表*/
	UE_BIND_PHONE_LIST = UE_BEGIN + 21, 
	/*收到对绑定申请的操作结果*/
	UE_BIND_PHONE_OPERATION_RESULT = UE_BEGIN + 22,
	/*收到苹果推送服务配置结果*/
	UE_APNS_CONFIG = UE_BEGIN + 23,
	/*手机账号已被注册*/
	UE_PHONE_USER_REGISTER_EXISTED = UE_BEGIN + 24,
	UE_PHONE_NEW_VERSION = UE_BEGIN + 25,

	UE_PHONE_USER_ADD_DEV_FINISH = UE_BEGIN + 26,
	UE_PHONE_USER_DEL_DEV_FINISH = UE_BEGIN + 27,
	
	// 环境监测参数变化了，包括温度、PM2.5等
	UE_ENV_MON_MODIFY = UE_BEGIN + 30,
	UE_SET_NAME_OK = UE_BEGIN + 31,
	UE_SET_NAME_FAILED = UE_BEGIN + 32,
	/*收到苹果推送服务配置失败*/
	UE_APNS_CONFIG_FAIL = UE_BEGIN + 33,
	/*收到设备有可升级版本*/
	UE_DEV_UPGRADE_READY = UE_BEGIN + 34,
	UE_DEV_UPGRADE_SET_OK = UE_BEGIN + 35,
	UE_DEV_UPGRADE_SET_FAIL = UE_BEGIN + 36,
	UE_DEV_UPGRADE_NOW_OK = UE_BEGIN + 37,
	UE_DEV_UPGRADE_NOW_FAIL = UE_BEGIN + 38,
	UE_DEV_UPGRADE_PROGRESS = UE_BEGIN + 39,
	/* 收到室内环境建议 */
	UE_ENV_WEATHER_SUGGEST = UE_BEGIN + 40,
	
	UE_DEV_ACTIVE_SUCCESS	 = UE_BEGIN + 45,	
	UE_DEV_ACTIVE_FAILED	 = UE_BEGIN + 46,	
	
	//控制805成功
	UE_CTRL_805_OK			= UE_BEGIN + 47,
	//控制805失败
	UE_CTRL_805_FAILED		= UE_BEGIN + 48,
	//RFGW网关发现新设备需要确认配对
	UE_RFGW_DEV_FIND		= UE_BEGIN + 49,
	//RF设备有透传数据事件
	UE_RFGW_DEV_TT			= UE_BEGIN + 50,
	//月兔定时器时间冲突事件
	UE_CTRL_YT_TIME_ERR		= UE_BEGIN + 51,
	//rf设备升级冲突事件
	UE_RFGW_DEV_UPGRDE_ERR	= UE_BEGIN + 52,
	//月兔故障时不能操作事件
	UE_CTRL_YT_CTRL_FAULT 	= UE_BEGIN + 53,
	// 印度车有历史记录统计信息更新
	UE_INDIACAR_HISOTRY_INFO_UPDATE = UE_BEGIN + 54,
	// 印度车有历史记录详细信息更新
	UE_INDIACAR_HISOTRY_DETAIL_UPDATE = UE_BEGIN + 55,
	// 印度车有设备更新
	UE_INDIACAR_DEV_UPGRADE = UE_BEGIN + 56,

	// 收到通过SN订阅苹果推送的成功
	UE_APNS_CONFIG_BY_SN_OK = UE_BEGIN + 57,
	// 收到通过SN订阅苹果推送的失败
	UE_APNS_CONFIG_BY_SN_FAILED = UE_BEGIN + 58,

	// 印度车实时推送过程中，当前旅程结束了
	UE_INDIACAR_REALTIME_TRIP_OVER = UE_BEGIN + 59,
	// 印度车请求实时旅程数据，但是当前没处于旅程中
	UE_INDIACAR_REALTIME_TRIP_ERR = UE_BEGIN + 60,
	// 印度车有收到实时数据
	UE_INDIACAR_REALTIME_TRIP = UE_BEGIN + 61,
	// 印度车得到了最新旅程个数
	UE_INDIACAR_JORNEY_COUNT = UE_BEGIN + 62,
	//电微遥控器需要升级事件
	UE_DWYK_NEED_UPGRADE = UE_BEGIN + 63,

	// 印度车获取到了图片
	UE_INDIACAR_GET_PIC = UE_BEGIN + 64,

	// 印度车视频服务器不稳定断开
	UE_INDIACAR_AGENT_SERVER_ERROR = UE_BEGIN + 65,

	// 印度车请求局域网看视频，获得的反馈
	UE_INDIACAR_GET_LOCAL_WATCH_INFO = UE_BEGIN + 66,

	// 印度车请求局域网看视频，获得的文件列表
	UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST = UE_BEGIN + 67,
	//空调贴检查开关机返回事件，errno表示返回空调贴状态，0 成功 1 超时，未检测到开机 2当前已经是开机状态 0xff:校正中
	UE_WKAIR_CHECK_STATUS = UE_BEGIN + 68,
	// 印度车已经在录像了，重复录像。需要先关闭
	UE_INDIACAR_IS_RECORDING = UE_BEGIN + 69,	
	//空调贴震动检查开关机返回事件，errno表示返回空调贴状态，匹配结果，0 成功 1失败 0xff：校正中 
	UE_WKAIR_AUTO_SHOCK_CHECK = UE_BEGIN + 70,


	UE_DEV_UPGRADE_STM_ERASE = UE_BEGIN + 71,

	// 印度车获取到一段音频
	UE_INDIACAR_GET_AUDIO = UE_BEGIN + 72,

	// 印度车解析出一帧MP4帧对应的BMP图片
	UE_INDIACAR_GET_MP4_PIC = UE_BEGIN + 73,
	// 印度车读取MP4文件结束
	UE_INDIACAR_DECODE_MP4_FINISH = UE_BEGIN + 74,
	//rf网关不允许升级
	UE_RFGW_NOT_ALLOW_UPGRADE = UE_BEGIN + 75,
	
	UE_IWULINK_CLIENT = UE_BEGIN + 98,
	UE_END = UE_BEGIN + 99
};

////////////////////////////////////////////////////

#define IJ_007 0x0  /*i+006*/
#define IJ_006 0x6  /*i+006*/
#define IJ_001 0x01  /*i+001 目前主要是i+001E无线红外转发器*/
#define IJ_002 0x02  /*i+002 iTV-C*/
#define IJ_003 0x03  /*i+003 wireless camera*/
#define IJ_008 0x08  /* i+008 iTV-C */
#define IJ_803 0x37  /*海斯方案摄像头*/
#define IJ_805 0x36 /*智能转发器*/
#define IJ_807 0x09  /*i+807*/
#define IJ_808 0x10  /*808插座*/
#define IJ_812 0x11  /*udp暖风机*/
#define IJ_816 0x12 /*udp 油汀*/
#define IJ_813 0x13 /*udp 空气进化器*/
#define IJ_820 0x14 /*udp灯*/
#define	IJ_101	0x15	/* E宝 */
#define	IJ_102	0x16	/* E宝-II代 */
#define IJ_821 0x17 /*彩虹电热毯*/
#define IJ_822 0x18  /*杰能宝温控器*/
#define IJ_823 0x19 /*忆林温控器*/
#define IJ_830 0x1A  /*LEDE 调色灯*/
#define IJ_840 0x1B /*金长信配电箱*/
#define IJ_824 0x1C /*华天成热水泵*/
#define IJ_AMT 0x1D /*艾美特设备*/
#define IJ_RFGW 0x1E /*2.4G RF 网关*/
#define IJ_CHIFFO  0x1F/*前锋产品*/

    
#define IJ_FANSBOX		0x0A 	/* fansbox*/
#define IJ_OPT			0x20 	/*openwrt刷机设备*/
#define IJ_ANDROID		0x21  	/*Andriod刷机设备*/
#define IJ_COMMUNITY	0x22   /* 小区服务器 */

#define IJ_AIRCONDITION 0x30	/* 智慧家电单品空调 */
#define IJ_WATERHEATER 0X31		/* 智慧家电单品热水器 */
#define IJ_AIRHEATER 0x32		/* 智慧家电单品快热炉 */
#define IJ_AIRCLEANER 0x33		/* 智慧家电单品空气净化器 */
#define IJ_ELECTRICFAN 0x34		/* 智慧家电单品风扇 */
#define IJ_815			0x35     /*奥普浴霸样机*/
#define IJ_805			0x36     /*智能转发器*/

#define IJ_TL_TEMP	0x50 //特林温控器
    
///////////////////////////////////////////////////////////
#define IJ_HXPBJ	0X51/*海迅设备*/

#define ETYPE_IJ_HX_PBJ 0x1 //海迅破壁机
#define ETYPE_IJ_HX_POT 0x2 //海迅养生壶
////////////////////////////////////////////////////////////
#define IJ_QPCP		0x52/*千帕茶盘*/
#define IJ_QP_POT   0x53/*千帕共用*/
#define IJ_TEST_DEV 0x54 /*测试DEV*/
#define IJ_101_OEM  0x55 /*沙特和晴乐插座*/
#define IJ_BIMAR_HEATER  0x57 /*bimar暖风机*/
#define IJ_JL_STAGE_LAMP 0x58 /*集利舞台灯*/
#define IJ_JS_MICWAVE   0x59    /*晶石微波炉*/
#define IJ_KXM_DEVICE	0x60 /*科希曼*/
#define IJ_838_YT	0X61	/*月兔*/
#define IJ_839_ADS	0X62	/*澳德绅*/

#define IJ_EVM		0x70	/* 证书为虚拟机设备类型，扩展类型 IJ_EVM_EXT_TYPE*/
#define IJ_LEIS		0x71	/* 雷士照明 */
#define IJ_ZHDHX	0x91	/*智皇单火线*/
#define IJ_HOMESERVER 0x92	/*家庭服务器*/

enum {
	ETYPE_LEIS_DEFAULT = 1,
	ETYPE_LEIS_YINSU = 2,	// 音速灯	
};

#define IJ_INDIACAR 0x64 	/* 印度车载追踪器 */
#define IJ_ZHCL		0x65	/*智皇窗帘*/
////////////////////////////////////////////
#define ETYPE_INDIACAR_DEFAULT 0x1	// 印度车扩展类型
////////////////////////////////////////////

////////////////////////////////////////////
#define ETYPE_IJ_ZHCL 0x1	// 智皇窗帘扩展类型
////////////////////////////////////////////

#define ETYPE_IJ_LEIS	0x1	// 雷士照明默认扩展类型
////////////////////////////////////////////
typedef enum {
	EYPE_EVM_DEFAULT = 1,		// 证书类型
	EYPE_EVM_YUYUAN_WATER_CLEANNER = 2,	// 御源净水器
	EYPE_EVM_ZKCLEANNER = 3,	// 中科净化器
	//EYPE_EVM_HYTHERMOSTAT = 6,	// 华钓温控器 废除使用
	EYPE_EVM_BPUAIR_1 = 0X6,	// 广州邦普智能空调控制器
	EYPE_EVM_BPUAIR_2 = 0X7,	// 广州邦普智能空调控制器
	EYPE_EVM_BPUAIR_3 = 0X8,	// 广州邦普智能空调控制器
	EYPE_EVM_BPUAIR_4 = 0X9,	// 广州邦普智能空调控制器
	EYPE_EVM_HYTHERMOSTAT_AC = 0x0a, // 华钓温控器 空调
    EYPE_EVM_HYTHERMOSTAT_HT = 0x0b, // 华钓温控器 水暖
    EYPE_EVM_JRXHEATER = 0xc,	// 江苏进睿芯光芒热水器	
    EYPE_EVM_CJTHERMOSTAT = 0xd,	// 沈阳创佳温控器
    EYPE_EVM_DRKZQ = 0xe,	// 一品德仁植物绿化智能控制器
} IJ_EVM_EXT_TYPE;
////////////////////////////////////////////


////////////////////////////////////////////
#define ETYPE_IJ_KXM_HOST 0x1 //科希曼线控器
#define ETYPE_IJ_KXM_THERMOSTAT 0x2 //科希曼温控器
#define ETYPE_IJ_XY_THERMOSTAT 0x3 //鑫源温控器
#define ETYPE_IJ_SBT_THER	0x4	//思博特温控器
#define EYTPE_IJ_ZSSX_FURN	0X5 //中山商贤电热炉
#define EYTYP_IJ_KXM_AC		0X6 //科希曼空调机
#define ETYPE_IJ_YJ_HEATER	0x7	// 中山益佳智能电暖炉
#define ETYPE_IJ_GALAXYWIND_THERMOSTAT 0x8	// 银河风云温控器(科希曼温控器方案)
#define ETYPE_IJ_LINKON_THERMOSTAT 0X9	// linkon温控器
#define ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB 0xa	// 银河风云温控器(科希曼温控器方案) 商业集中控制

/////////////////////////////////////////////


#define ETYPE_IJ_JS_MICWARE 0x1 //晶石扩展型号，微波型
#define ETYPE_IJ_JS_MIC_BARBECUE 0x2 //微波+烧烤
#define ETYPE_IJ_JS_ONLY_MIC	0x3 //仅微波
//////////////////////////////////////////////////////////////
#define IJ_HEATER_DEV 0x57  //暖风机通用sub_type
    
#define ETYPE_IJ_HEATER_BIMAR_C2000 0x1 //BIMAR C2000暖风机
    
#define ETYPE_IJ_HEATER_AKM_0505L 0x6//澳柯玛暖风机0505L
#define ETYPE_IJ_HEATER_AKM_5162L 0x7//澳柯玛暖风机5162L
/////////////////////////////////////////////////////////////

#define IJ_SMART_PLUG   IJ_808  /*智能插座v3单品*/

//悟空扩展类型
#define ETYPE_IJ_SMP_H_16A   0x1 // i+808-CN2.0 i+808-CN2.1 家用16A悟空，（酷炫白、土豪金色）
#define ETYPE_IJ_SMP_B_16A   0x2 //i+828-CN1.0 商业16A悟空 （酷炫白）
#define ETYPE_IJ_SMP_H_WX_10A   0x3 //i+807-CN2.0 i+807-CN2.1 家用10A悟空微信，（酷炫白、土豪金色）
#define ETYPE_IJ_SMP_B_10A      0x4 //i+827-CN1.0 悟空i8商业挂机空调智能伴侣标准版(10A炫酷白)
#define ETYPE_IJ_SMP_H_GALA_10A 0x5 //i+807-CN1.0  悟空i8家用挂机空调智能伴侣标准版(10A炫酷白)
#define ETYPE_IJ_SMP_H_GALA_16A 0x6 //i+808-CN1.0 悟空i8家用挂机空调智能伴侣标准版(16A炫酷白)
#define ETYPE_IJ_SMP_H_ENH_16A 0x7 //i+818-CN2.0 悟空i8家用挂机空调智能伴侣增强版(16A微信炫酷白)
#define ETYPE_IJ_WK_KSA 0x0a		// 沙特悟空，去掉了电量管理，增加红外学习

#define ETYPE_IJ_SMP_H_ENH_16A_NO_INPUT 0xb	// //i+818-CN2.0 悟空i8家用挂机空调智能伴侣增强版(16A微信炫酷白) (不带输入插座)
#define ETYPE_IJ_WK_AJ	0xc // 悟空i8家用挂机空调智能伴侣标准版(16A炫酷白)	傲基悟空    
#define ETYPE_IJ_WK_GD_H_16A 0x31 // i+808-CN2.0 i+808-CN2.1 家用16A悟空，GD单片机（酷炫白、土豪金色）
#define ETYPE_IJ_WK_GD_B_16A 0x32 // i+828-CN1.0 商业16A悟空 GD单片机 （酷炫白）

#define IJ_UNKNOWN 0xFF  /*未知类型*/

//!!!!注意!!!!
#define ETYPE_IJ_MIN 0x01 /* 设备扩展类型的最小值 */
#define ETYPE_IJ_MAX 0xFF /* 设备扩展类型的最大值 */

//联创设备扩展类型
#define ETYPE_IJ_812_AH_5331P  0x1 //联创普通暖风机，支持摇头
#define ETYPE_IJ_812_AH_HT5C05P  0x2 //不支持摇头和ION
#define ETYPE_IJ_812_AH_9505R	0x3 //不支持摇头，支持ION
#define ETYPE_IJ_812_HT_5325P   0x4//支持摇头和ECO,同5331P
#define ETYPE_IJ_812_HT_5326P   0x5 //支持摇头和ECO,同5331P
#define ETYPE_IJ_812_HT_5250P   0x6 //支持摇头，不支持ECO


#define ETYPE_IJ_816_LCYT 0x1


#define ETYPE_IJ_813_HK 0x1  /* 海科空气净化器 */
#define ETYPE_IJ_813_NB 0x2  /* 南柏空气净化器 */
#define ETYPE_IJ_820_GX 0x1	/* 高讯LED调光灯 */
#define	ETYPE_IJ_101_EB	0x1
#define ETYPE_IJ_821_CH 0x1

/*E宝扩展类型*/
#define ETYPE_IJ_EB_USB    0x1 /*E宝带USB*/
#define ETYPE_IJ_EB_NO_USB 0x2 /*E宝不带USB*/
#define ETYPE_IJ_EB_JNB_GLOBAL     0x3 /*E宝带USB,加电流电压检测功能，国际版*/
#define ETYPE_IJ_EB_JNB_WATER_DROP 0x4 /*E宝不带USB,水滴爆款*/
#define ETYPE_IJ_EB_FOXCONN  0x5 /*富士康插座*/
#define ETYPE_IJ_EB_GXLAXYWIND_GLOBAL  0x7 /*银河风云-WIFI插座（国际版）*/
#define ETYPE_IJ_EB_GXLAXYWIND  0x8        /*银河风云-WIFI插座（国内版）*/
#define ETYPE_IJ_EB_JNB_OVERSEA 0x9        /*杰能宝Wi-Fi插座,海外*/
#define ETYPE_IJ_EB_KT		0x10				/* 凯特E宝国内版 */
#define ETYPE_IJ_EB_KT_OVERSEA	0x11					/* 凯特E宝国际版 */

//杰能宝温控器扩展类型
#define ETYPE_IJ_822_JNB 0x1 

//忆林温控器扩展类型
#define ETYPE_IJ_823_YL 0x1

//LEDE 调色灯扩展类型
#define ETYPE_IJ_830_LEDE 0x1

//金长信配电箱扩展类型
#define ETYPE_IJ_840_JCX  0x1

//华天成热水泵
#define ETYPE_IJ_824_HTC 0x1
//商用华天成
#define TYPE_IJ_824_YCJ	0X2
#define ETYPE_IJ_824_HTC_BUSINESS 0X3
//智科热水器
#define ETYPE_IJ_824_ZKRSQ	0X4

//艾美特产品
#define ETYPE_IJ_AMT_FAN 0x1

/*艾美特扩展类型*/
#define ETYPE_IJ_AMT_FAN    0x1 /*艾美特风扇*/

/*2.4G RF 网关扩展类型*/
#define ETYPE_IJ_RFGW_6621 0x01 /*采用6621模组*/

#define ETYPE_IJ_RFGW_S2	0x02 /*2.4G RF S2主机*/
#define ETYPE_IJ_RFGW_S3	0x03 /*2.4G RF S3主机*/
#define ETYPE_IJ_RFGW_S4	0x04 /*2.4G RF S4主机*/
#define ETYPE_IJ_RFGW_YL	0x05 /*夜狼网关*/
#define ETYPE_IJ_RFGW_S9	0x68 /*花瓶网关*/


#define ETYPE_IJ_RFDEV_STM32 0x21 /*STM32单片机RF设备*/
//前锋产品
#define ETYPE_IJ_CHIFFO_FlOOR_HEATER  0x01 /*前锋地暖*/
#define ETYPE_IJ_CHIFFO_WATER_HEATER  0x02	/* 前锋热水器QFM0591A */

//特林产品
#define ETYPE_IJ_TL_HEATER 	0x1 //特林采暖设备
//千帕产品，茶盘和锅用subtype区分的
#define ETYPE_IJ_QP_CP 0x1 //千帕茶盘
//千帕
#define ETYPE_IJ_QP_POT 0x1 //千帕锅
#define ETYPE_IJ_QP_PBJ 0x2 //破壁机扩展类型

// 0x54 测试证书
#define ETYPE_IJ_TEST_CAR_WK 0x1 //车载悟空
#define ETYPE_IJ_TEST_8266_LED 0x2 //esp8266 LED
#define ETYPE_IJ_TEST_XY	0X6/*鑫源温控器*/
#define ETYPE_IJ_TEST_BITMAR 0x7 //bimar暖风机

//0x55 插座OEM
#define ETYPE_IJ_101_QL 0x1 //晴乐插座
#define ETYPE_IJ_101_SA 0x2 //沙特30A插座

//0x57 bimar暖风机
#define ETYPE_BIMAR_HEATER 0x01
#define ETYPE_AUCMA_HEATER 0x06

//月兔拓展类型
#define ETYPE_IJ838_YT	0X1

//0x58 集利舞台灯
#define ETYPE_JL_STAGE_LAMP 0x01


//澳德绅扩展类型
#define ETYPE_IJ839_ADS	0X01

//803摄像头扩展类型
#define ETYPE_IJ803_HS  0x01 //i+803-hie, easyn, 海思摄像头
#define ETYPE_IJ803_C2 0x02 //i+gepc2, galaxywind, C2摄像头
#define ETYPE_IJ803_C3 0x03 //i+gepc3 , galaxywind, C3摄像头

enum{/*登录类型*/
	LTYPE_NORMAL = 1, /*普通登录，未启用绑定手机*/
	LTYPE_BIND = 2, /*绑定手机登录*/
	LTYPE_UNBIND = 3 /*未绑定手机登录*/
};

enum{/*网络连接类型*/
	NTYPE_SERVER = 1, /*连接服务器*/
	NTYPE_DEVICE = 2, /*局域网直连设备，设备连接到服务器*/
	NTYPE_DEVICE_OFFLINE = 3, /*局域网直连设备，设备未连接到服务器*/
};

enum{
	BIND_PHONE_ACCEPT = 1, /*批准绑定申请*/
	BIND_PHONE_DENY = 2, /*拒绝绑定申请*/
};

enum{
	ACTION_QUERY = 0, /* 查询 */
	ACTION_ADD = 1,  /*添加*/
	ACTION_MOD = 2, /*修改*/
	ACTION_DEL = 3,  /*删除*/
};

/* 主从绑定的错误号 */
enum{
	BIND_ERR_NONE = 0,
	BIND_ERR_PASSWORD = 1,		//三次认证密码错误
	BIND_ERR_OTHER_MASTER  	//被其他设备绑定，并记录绑定的设备SN号
};

/* 语言 */
enum{
	LANG_BEGIN = 0,
	LANG_CH = 1, /* 简体中文 */
	LANG_EN = 2, /* 英文 */
	LANG_MAX
};

/* 设备版本信息 */
typedef struct {
	u_int8_t major;		/* 主版本 */
	u_int8_t minor;		/* 次版本 */
	u_int8_t revise;	/* 修订版本 */
	u_int8_t pad;	/* 填充字节 */
} cl_version_t;

/* 异常错误号 */
// 通用探测器
enum {
	CAT_TYPE_COM_DETECTOR_LOW_BATTERY = 10,	// 电池电量过低
	CAT_TYPE_COM_DETECTOR_ALARM = 14,	// 报警触发
};

// 汇泰龙
enum {
	CAT_TYPE_HTLLOCK_LOCK = 20,				// 汇泰龙开锁
	CAT_TYPE_HTLLOCK_UNLOCK = 21,			// 汇泰龙关锁
	CAT_TYPE_HTLLOCK_NOT_LOCK = 22,			// 汇泰龙门未锁
	CAT_TYPE_HTLLOCK_ERR_PWD = 23,			// 汇泰龙输入错误密码或指纹或卡片超过10次
	CAT_TYPE_HTLLOCK_HIJACK = 24,			// 汇泰龙输入防劫持密码或防劫持指纹开锁
	CAT_TYPE_HTLLOCK_MENCHANNICAL_KEY = 25,	// 汇泰龙使用机械钥匙开门
};

// 门锁
enum {
	CAT_TYPE_DOOR_BREAK = 8,	// 撬门
	CAT_TYPE_DOOR_LOCK_BREAK = 9,	// 撬锁
	CAT_TYPE_DOOR_LOW_BATTERY = 10,	// 电池过低
	CAT_TYPE_DOOR_OPEN = 11,	// 门开
	CAT_TYPE_DOOR_CLOSE = 12,	// 门关
	CAT_TYPE_DOOR_UNLOCK_TIMEOUT = 13,	// 长时间未锁门
};

// 甲醛
enum {
	CAT_TYPE_CH2O_VALUE_HIGH = 29,	// 探测数值超过阀值
	CAT_TYPE_CH2O_VALUE_SAFE = 30,	// 探测数值回复正常
};

// 平滑显示的设备状态
enum {
	DISPLAY_STAT_NOT_INITED,		// 还没初始化的状态,APP不用管
	DISPLAY_STAT_VIRTUAL_ONLINE,	// 假在线
	DISPLAY_STAT_REAL_ONLINE ,	// 真实在线，可查看状态，操作
	DISPLAY_STAT_OFF_LINE,	// 手机连不通外网
	DISPLAY_STAT_CONNECTING,	// 连接中
	DISPLAY_STAT_REAL_STAT,	// 返回真实的状态
};

/* cl_handle_t
 * slave handle的取值范围是[0x2000000, 0x3000000);
 */
enum {
    CL_SLAVEHANDLE_MIN = 0x2000000,
    CL_SLAVEHANDLE_MAX = 0x3000000
};

typedef struct {
	cl_obj_t obj;

	// IJ_XXX
	u_int32_t dev_type;

	/* 绑定操作产生的错误号, BIND_ERR_XXX, 0未出错，1密码错误,2已被其它设备绑定 */
	u_int32_t bind_error;
	/* bind_errno为BIND_ERR_OTHER_MASTER生效，记录被绑定的主设备SN */
	u_int64_t other_master_sn;

	 /* 固件版本信息，类似 1.2.3 */
	cl_version_t soft_version;
	/* 升级包版本信息，类似 1.2.4 */
	cl_version_t upgrade_version;
	//rf设备下单片机版本，
	cl_version_t rf_stm_ver;
	// 设备启动多长时间了，单位秒
	u_int32_t uptime;
	// 设备与服务器连接多长时间了，单位秒
	u_int32_t online;
	// 设备上互联网多长时间了，单位秒
	u_int32_t conn_internet;

	/*
		dev_type == IJ_008的时候使用
	*/
	// 该遥控插座是否带电流检测
	bool has_current_detect;
	//支持电量统计功能
	bool has_electric_stat;


	/*
		dev_type == IJ_003或者IJ_803的时候使用
	*/
	// 是否支持录像功能
	bool has_video_record;
	// 是否支持图像翻转
	bool has_video_flip;
    // 是否支持饱和度
    bool has_v4l2_color_setting;
    // 是否云台转速控制
    bool has_roll_speed_ctrl;
    //是否支持云台控制
    bool has_ptz;

	bool has_recv_flag_pkt;//是否标志有效
	u_int8_t is_support_la;//是否支持联动控制	
    

	// E系列主设备或从设备
	// 是否支持红外功能(电视机、空调等电器，支持控制和学习)。281模块中的标志
	bool has_ir;
	// 是否支持无线功能(面板开关、窗帘机等，支持发送和学习)。281模块中的标志
	bool has_rf;
	// 是否支持告警(声光报警器、门磁感应等，支持发送和学习)。282模块
	bool has_alarm;
	// 是否支持短信和报警推送单独配置
	bool has_alarm_swich;
	// 支持二维码扫描json添加电器
	bool has_eq_add_by_json;
	//是否升级失败
	bool is_upgrade_err;

	//屏幕是否开
	bool is_805_screen_on;
	//蜂鸣是否开
	bool is_805_beep_on;
	//UDP从设备
	bool is_udp;
	//从设备扩展类型
	u_int8_t ext_type;
	// 是否获取到在线信息
	u_int8_t status_valid;
	// 从设备开发者ID
	u_int8_t developer_id[32];

	//rf通用定时器
	cl_comm_timer_head_t comm_timer;
	cl_dev_timer_summary_t timer_summary;
	/* 区域句柄, 0为无效 */
	cl_handle_t area_handle;
	/* 主设备的handle */
	cl_handle_t master_handle;

	u_int8_t is_support_public_shortcuts_onoff;		// 支持通用的快捷开关机
	cl_shortcuts_onoff_t shortcuts_onoff;			// 通用的快捷开关机 

	//是否支持批量一键布防撤防
	bool is_support_dbc;

	//stat
	u_int32_t run_time;//设备运行时间
	
	cl_rfdev_status_t rfdev;
} cl_slave_t;

#define	ULGE_NONE	0
// 错误的序列号
#define	ULGE_BAD_SN	1
// 错误的昵称
#define	ULGE_BAD_NICKNAME	2
// 密码错误
#define	ULGE_BAD_PASSWORD	3
#define	ULGE_BAD_PHONE_NUM	4
// 需要绑定手机才能登录
#define	ULGE_NEED_BIND		5
// 需要绑定手机，已经绑定满了
#define	ULGE_FULL_BIND		6
// 绑定手机申请已经被其他人处理了
#define ULGE_OTHER_BIND		7
// 网络不给力
#define	ULGE_NETWORK_ERROR	8
// 服务器有点忙
#define	ULGE_SERVER_BUSY	9
//客户端版本过低
#define ERR_BAD_VERSION
//设备离线
#define ULGE_DEV_OFF_LINE 13

#define ULGE_DEV_CLONE 14
//v2协议未注册
#define ERR_V2_UNBIND	15


#define ERROR_IMAGE				(100)


typedef struct {/*设备绑定手机参数*/
	char phone_number[16]; /*手机号码*/
	char phone_model[32]; /*手机型号*/
	char bind_name[16];/*绑定名称*/
	char bind_uuid[40]; /*绑定uuid*/
	char bind_message[40];/*绑定描述*/
	char timestamp[20]; /*时间戳2014-02-20 13:56:12*/
}cl_bind_phone_t;

typedef struct{
	u_int8_t count;
	u_int8_t reserved[3];
	cl_bind_phone_t *request_list;
}cl_bind_phone_request_list_t;

typedef struct{
	u_int8_t count;
	u_int8_t allow_normal;
	u_int8_t reserved[2];
	cl_bind_phone_t *bind_array;
}cl_bind_phone_list_t;

typedef struct{
	u_int32_t err_code; 
	u_int8_t action; 
	u_int8_t reserved[3];
	u_int8_t request_uuid[40]; /*申请者uuid*/
	cl_bind_phone_t operator_info; /*绑定申请处理者信息*/
}cl_bind_phone_result_t;

typedef struct{
	char action;		/*ACTION_QUERY ACTION_ADD*/
	char need_push;	/*是否订阅，0表示不订阅*/
	u_int8_t cert_id;		/*推送证书id*/
	u_int8_t token_len;	/*token长度*/
	u_int8_t msg_len;		/*推送消息前缀长度*/
	u_int8_t reserved[3];	/*保留*/	
	char phone_ver[8];	/*手机系统版本，如"7.1"*/
	char token[64];		/*token，二进制，目前32字节*/
	char msg_prefix[64];	/*推送消息前缀，UTF8格式*/

	// 新增内容
	/*
		这里填写额外的配置信息，格式如下。可以为空
		languages=EN&push_music=whatdoesfoxsay.mp3 
	*/
	char language[64];
	char push_music[256];

	// 小米推送用,IOS不管
	char mipush_packname[64];
	char regid[256];
}cl_apns_config_t;

typedef struct {
	// 该用户在公共库内部的唯一标识
	cl_handle_t handle;

	//添加个is_udp_ctrl字段，给app做优化使用
	bool is_udp_ctrl;

	// 用户输入的名称，可能是序列号，也可能是昵称
	char *name;
	// 该设备的序列号
	u_int64_t sn;
	// 昵称, UTF-8
	char *nickname;

	// 用户输入的密码
	char *passwd;

	// 是否登录成功
	bool is_login;
	// 是否在线
	bool is_online;
	// 平滑显示的状态,DISPLAY_STAT_XXX
	u_int8_t display_stat;
	// 是否支持绑定手机
	bool can_bind_phone;
	char login_type;
	char net_type;
	
	// 登录失败时的错误，ULGE_XXXX
	int last_err;

	//****pc显示用
	u_int32_t dev_ip;
	u_int32_t home_id;
	u_int32_t upgrade_version;
	//****

	// 如果是设备，设备子类型，IJ_XXX
	u_int8_t sub_type;
	// 如果是设备，设备扩展类型
	u_int8_t ext_type;
	// 开发者ID
	u_int8_t developer_id[32];
	// 如果是设备，设备的厂家ID
	char *vendor_id;
	// 如果是设备，获取设备的厂家资源的URL
	char *vendor_url;

	// 是否支持场景模式
	bool has_scene;
	// 支持情景定时器
	bool has_scene_timer;
	// 支持报警器触发情景模式
	bool has_scene_linkage_alarm;
	// 是否支持区域模式
	bool has_area;

	// 全局标志。具体是哪台从设备上支持，需要遍历查找
	// 是否支持红外功能(电视机、空调等电器，支持控制和学习)
	bool has_ir;
	// 是否支持无线功能(面板开关、窗帘机等，支持发送和学习)
	bool has_rf;
	// 是否支持告警(声光报警器、门磁感应等，支持发送和学习)
	bool has_alarm;
	// 是否支持短信和报警推送单独配置
	bool has_alarm_swich;
	//支持双向RF功能
	bool has_db_rf;
	// 支持对码功能
	bool has_eq_gencode;
	// 支持二维码扫描json添加电器
	bool has_eq_add_by_json;

	//设备密码，联动兼容手机账号要用 
	u_int8_t dev_passwd[16];

	//判断改设备版本是否过低
	bool dev_ver_is_valid;//是否有效，为真时dev_ver_is_too_low才有效
	bool dev_ver_is_too_low;

	u_int8_t is_support_telnet;//是否支持telnet设置
	
	// 是否支持绿色上网
	bool has_green_net;
	// 是否有倍泰健康模块
	bool has_belter;

	// objs指针数组有多少个指针
	u_int32_t num_objs;
	// 摄像头、电器等指针数组
	cl_obj_t **objs;

	// 有多少个usb摄像头
	u_int32_t num_usb_video;
	// USB摄像头在objs中从第几个开始
	u_int32_t idx_usb_video;

	// 有多少个从设备
	u_int32_t num_slave;
	// 从设备在objs中从第几个开始
	u_int32_t idx_slave;

	// 有多少个电器
	u_int32_t num_equipment;
	// 电器在objs中从第几个开始
	u_int32_t idx_equipment;
    /*设备有几个报警手机号码*/
    u_int32_t num_alarm_conf_phone;
    //报警手机号码列表
    char** phone_list;
    //有多少个区域
    u_int16_t num_area;
    //区域列表
    struct _cl_area_s_** areas;
    //有多少个情景模式
    u_int16_t num_scene;
    //情景模式列表
    struct _cl_scene_s_** scenes;
    //智能插座v3状态信息
    struct _cl_air_info_s *air_info;
    //联创暖风机状态
    struct _cl_ah_info_s *ah_info;
	//智慧家居状态信息
	struct _cl_ia_info_s_ ia_info;
    //805M智能转发器信息
    cl_if_info_t if_info;
	// E宝相关信息
	cl_ia_eb_info_t *eb_info;
      //通用UDP 设备信息
	struct cl_com_udp_device_data_s* com_udp_info;
	// 保留给调用者使用
	void *priv;
} cl_dev_info_t;

//联动数据结构
#define APP_USER_UUID_NAME_LEN		16
//家庭标签
#define LA_LABEL_NAME_MAX_LEN	(64)

typedef struct {
	u_int8_t state;//添加的规则状态，1表示执行中，0表示未执行
	u_int8_t enable;//使能状态，1表示使能
	u_int32_t rule_len;//rule_len
	u_int8_t *rule;//添加规则字符串\0结束的字符串
	u_int32_t rule_id;//添加规则服务器分配id
	u_int32_t last_exec_time;//规则上次执行时间戳
}cl_rule_desc_t;

typedef struct {
	u_int32_t user_id;//该家庭拥有的用户id
	u_int32_t join_time;//该用户加入家庭的时间
	u_int32_t lastuse_time;//成员最近登陆时间，正好做那个成员显示
	u_int8_t role_id;
	u_int8_t desc[64];
}la_share_desc_t;

typedef struct {
	u_int16_t id;
	char label_name[LA_LABEL_NAME_MAX_LEN];
	u_int16_t sn_num;
	u_int64_t *sn;
}la_label_desc_t;

//快捷按键
#define LA_SC_NAME_MAX (64)

#define LA_SC_A_NUM	(6)

typedef struct{
	u_int8_t valid;//是否有效
	char name[LA_SC_NAME_MAX];
	u_int32_t rule_id;
}la_sc_key_t;

// 字典
#define MAX_LA_DICT_NUM 64

typedef struct {
	u_int8_t *key;
	u_int16_t key_len;
	u_int8_t *value;
	u_int16_t value_len;
} cl_la_dict_t;

typedef struct {
	u_int32_t home_id;	// 家庭ID
	int ndict;	// 有多少个字典
	cl_la_dict_t dict[MAX_LA_DICT_NUM];
} cl_la_dict_info_t;

typedef struct {
	cl_handle_t handle;
	bool is_def_home;//是否是默认圈子
	u_int8_t online;//是否连接上服务器的，1表示连接上
	u_int32_t home_id; //服务器分配的全局唯一id
	char home_name[64];	//家庭名称
	u_int32_t last_rule_time;//上次规则修改时间
	u_int32_t last_template_time;//上次模板修改时间

	u_int16_t url_num;//模板个数
	u_int8_t **url_array;//模板url数组

	bool rules_is_cache;//表示规则是不是缓存的
	u_int16_t rule_num;//添加的规则
	cl_rule_desc_t *rule_array;//添加的规则字符串

	u_int8_t *share;//分享码,\0结束的字符串

	u_int16_t share_desc_num;//该家庭被分享使用的个数，第一个是家庭创建者
	la_share_desc_t *share_desc_array;	
	
	u_int8_t home_passwd[APP_USER_UUID_NAME_LEN];//家庭密码

	//家庭标签组
	u_int16_t label_num;
	la_label_desc_t *label;

	//快捷按键
	la_sc_key_t la_sc_key[LA_SC_A_NUM]; 

	// 字典
	cl_la_dict_info_t *dict;
}cl_la_home_info_t;

typedef struct cl_user_s {
	cl_handle_t user_handle;

	// 用户输入的用户名，可能是SN，也可能是昵称，也可能是手机号
	char *username;
	// 是否是手机注册的帐号
	bool is_phone_user;
	//是否是联动家庭
	bool is_la;
	// 用户输入的密码
	char *passwd;
	// 用户输入的密码进行MD5运算后，打印成字符串
	char *passwd_md5_str;

	// 是否登录成功
	bool is_login;
	// 登录失败时的错误，ULGE_XXXX
	int last_err;

	// 如果是设备，设备子类型，IJ_XXX
	u_int8_t sub_type;
	//如果是设备，设备扩展类型
	u_int8_t ext_type;
	// 如果是设备，设备的厂家ID
	char *vendor_id;
	// 如果是设备，获取设备的厂家资源的URL
	char *vendor_url;

	u_int8_t is_support_spe_up;//是否支持指定升级
	//联动家庭数据
	cl_la_home_info_t home;

	u_int32_t num_dev;
	// 该帐号或家庭下所有设备的指针数组。
	cl_dev_info_t **dev;
} cl_user_t;


// 网络接口信息
typedef struct {
	// 接口名字
	char *name;

	bool is_up;
	bool is_ip_valid;
	
	u_int32_t ip;
	int mtu;
	u_int64_t rx_byte;
	u_int64_t tx_byte;
} cl_ni_t;

// 内网上连接的客户端
typedef struct {
	// 计算机名字
	char *name;
	u_int32_t ip;
	u_int8_t mac[6];
	// 接入方式: 无线、网线
	u_int8_t is_from_wifi;
	u_int8_t pad;
} cl_lan_client_t;

#define DAY_HOUR 24
typedef struct light_smp{
	u_int32_t yesterday;
	u_int32_t today;
	u_int32_t sample[DAY_HOUR];
}light_smp_t;

enum {
	ACTIVE_STATUS_NOACTIVE = 0,//未激活的
	ACTIVE_STATUS_ACTIVE = 1,//已激活的
};

typedef struct{
	u_int32_t active_status; //激活状态ACTIVE_STATUS_XXX
	u_int64_t active_sn;//当激活时，其激活的sn号
}active_st_t;

#define DISK_MAX_NUM	(10)
#define DISK_MAX_MODEL_LEN	(64)

typedef struct {
	short temp;//温度,度
	u_int32_t use_time;//使用时长，单位小时
	u_int8_t model[DISK_MAX_MODEL_LEN];//型号
	u_int8_t serial[DISK_MAX_MODEL_LEN];//序列号
	u_int32_t capacity;//容量，单位M
}cl_disk_item_info_t;

typedef struct {
	u_int8_t valid;//硬盘信息是否有效.
	
	u_int8_t mode;//硬盘苦安装模式，现在先不管吧

	u_int32_t total_capacity;//总容量，单位M
	u_int32_t used_capacity;//已使用容量，单位M

	u_int8_t num;//该设备包括几个硬盘
	cl_disk_item_info_t disk_item[DISK_MAX_NUM];
}cl_disk_info_t;


//网口最大支持个数
#define ETH_MAX_NUM	(10)

typedef struct {
	u_int8_t index;//网口索引
	u_int32_t ip;//网口ip地址
	u_int8_t name[16];//网口名称
	u_int32_t tx_rate;//发送速率，单位 bytes/s,字节每秒，app上可以根据情况除以1024显示m/s。
	u_int32_t rx_rate;//接收速率，单位 bytes/s。
}cl_eth_item_info_t;

typedef struct {
	u_int8_t valid;//网口信息是否有效

	u_int8_t num;//网口个数
	cl_eth_item_info_t eth_item[ETH_MAX_NUM];
}cl_eth_info_t;

typedef struct{
	u_int8_t is_stat_valid; //状态数据是否有效，包括uptime和online
	////
	u_int8_t dev_to_server_stat; //设备连接服务器状态
	///
	u_int8_t pad[2];
	u_int32_t devSvnVersion; //设备svn版本号
	u_int32_t dev_cur;//空调宝当前电流
	u_int32_t dev_cur_ad;//空调宝当前电流ad值
	u_int32_t dev_cur_k;//空调宝当前电流k值
	u_int32_t dev_cur_b;//空调宝当前电流b值
	u_int32_t dev_vol;//空调宝当前电压
	u_int32_t dev_vol_ad;//空调宝当轻电压d值
	u_int32_t dev_vol_k;//空调宝当前电压k值
	u_int32_t dev_vol_b;//空调宝当前电压b值
	u_int32_t dev_light_ad;//空调宝当前光敏ad值
	u_int8_t  dev_domain[32]; //设备连接的域名
	u_int32_t dev_cur_phone_num; 
	light_smp_t light_study;
	u_int32_t avg_ad; //待机平均电?
	u_int32_t max_ad; //待机最大电流
	u_int32_t delay_power_on_time; //开机电流突变时间
	u_int32_t delay_power_off_time;// 关机电流突变时间
	u_int32_t no_load_ad; //空载电流的AD 值
	u_int32_t smt_soft_ver; // 单片机软件版本
	u_int32_t smt_hard_ver; //单片机硬件版本
	u_int16_t ir_lib_id; //红外编码库ID
	u_int16_t pad1;
	u_int32_t cold_delay_pn_time;//制冷开机电流突变时间(s
	u_int32_t cold_delay_pf_time;//制冷关机电流突变时间(s
	u_int32_t hot_delay_pn_time;//制热开机电流突变时间(s
	u_int32_t hot_delay_pf_time;//制热关机电流突变时间(s
	char* stm_32_dbg_info; //单片机编译时间
	u_int32_t adjust_pressed; //非零表示已经按过reset按键进行校正
	u_int32_t pt_stat_flag; //商业悟空产测标志位
	////////////////////////////////////////////////////////
	u_int32_t dispatch_ip; //分配服务器ip
	u_int32_t dev_server_ip; //设备服务器ip
	u_int32_t dev_cur_time; //设备当前时间
	u_int8_t dev_img_compile_data[32]; //设备镜像编译日期
	u_int8_t dev_img_compile_time[32]; //设备镜像编译时间
	active_st_t active_st;
	//timezone
	u_int8_t timezone_valid;
	u_int32_t timezone;
	//小电流开关
	u_int8_t scc_onoff_valid;
	u_int8_t scc_onoff;

	u_int32_t wifi_conn_time;	// WIFI连接时间
	int8_t wifi_rssi;	// WIFI信号强度，可以为负
	u_int8_t wifi_phy_mode;	// WIFI的模式
	cl_version_t wifi_version;	//  WIFI版本号

	cl_version_t kernel_image_version;	// Kernel镜像版本号
	u_int32_t kernel_image_svn;		// Kernel 镜像SVN号
	cl_version_t user_image_version;	// User镜像版本号
	u_int32_t user_image_svn;		// User镜像版本号
//	u_int32_t wireless_wan_ip;		// 无线WAN口IP
	u_int32_t wired_wan_ip;			// 有线WAN口IP

	u_int8_t wifi_mac[6];	// WIFI的MAC地址

	//硬盘信息
	cl_disk_info_t disk_info;
	//网口信息
	cl_eth_info_t eth_info;
}cl_udp_dev_stat;

typedef struct {
	cl_handle_t handle;
	u_int64_t sn;
	// CPU使用率，小数点两位
	u_int16_t cpu;
	// 内存使用率，小数点两位
	u_int16_t mem;
	 /* 固件版本信息，类似 1.2.3 */
	cl_version_t soft_version;
	/* 升级包版本信息，类似 1.2.4 */
	cl_version_t upgrade_version;
	/* 新版本信息 ，can_update为true时有效 */
	cl_version_t new_version;
	//硬件版本
	cl_version_t hardware_version;
	/* true表示设备有新版本可以升级 */
	bool can_update;
	/* true表示可以自动升级，false表示需要手工升级固件 */
	bool can_auto_update;
	//单片机是否有最新升级镜像
	bool stm_can_update;
	/* 新版本描述信息 */
	char *release_desc;
	/* 新版本URL */
	char *release_url;
	/* 新版本发布日期 */
	char *release_date;
    //新单片机版本下载地址
    char* stm_release_url;
	////////////////////////////////////////////////////////////////////
	//单片机当前版本
	cl_version_t stm_cur_version;
	//单片机服务器上最新版本
	cl_version_t stm_newest_version;
	/////////////////////////////////////////////////////////////////////
	// 设备启动多长时间了，单位秒
	u_int32_t uptime;
	// 设备与服务器连接多长时间了，单位秒
	u_int32_t online;
	// 设备上互联网多长时间了，单位秒
	u_int32_t conn_internet;

	// 无线网络
	char *ap_ssid;
	// 无线密码
	char *ap_passwd;

	// 网络接口信息
	cl_ni_t wan;
	cl_ni_t lan;

	// 内网主机信息
	int client_num;
	cl_lan_client_t **clients;
	cl_udp_dev_stat udp_dev_stat;
	
	//添加些统计信息
	u_int32_t server_ip;
	char server_domainname[50];
	u_int32_t svn_num;
	u_int8_t dev_status;
	//new add
	u_int32_t DevServerIp;
	u_int32_t StmSoftVersion;
	u_int32_t StmHardVersion;
	u_int32_t LightCur;
	u_int32_t LightYes;
	u_int32_t LightNext;
	u_int16_t IrId;
	u_int32_t AcCur;
	u_int32_t ACVal;
	//编译日期
	char *compile_date;
	//编译时间
	char *compile_time;
		//systime;
	u_int32_t systime;

} cl_dev_stat_t;

typedef struct{
	u_int32_t vol_ad;
	u_int32_t cur_ad;
	u_int32_t cur_ad2;
}cl_pt_stat_t;


typedef struct {
	cl_version_t ver;
	cl_version_t upver;
	cl_version_t hardver;
	u_int32_t svn;
}cl_dev_debug_info_t;


/*
	功能：
		添加一个用户(设备)
	输入参数:
		@user_handle: 本地存放设备句柄的地址
		@username: 设备序列号或设备昵称
		@passwd: 设备密码
		@callback: 回调函数。回调事件为 UE_xxx
			状态变迁时会回调，包括登录成功、成功->失败、离线->在线、在线->离线
		@callback_handle: 回调时回传回来的
	输出参数:
		@user_handle: 保存该设备的句柄，应用程序用于后续使用
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_login(cl_handle_t *user_handle, const char *username, const char *passwd,
							cl_callback_t callback, void *callback_handle);

/*
	功能：
		一键配置后添加一个用户(设备)
	输入参数:
		@user_handle: 本地存放设备句柄的地址
		@username: 设备序列号或设备昵称
		@passwd: 设备密码
		@callback: 回调函数。回调事件为 UE_xxx
			状态变迁时会回调，包括登录成功、成功->失败、离线->在线、在线->离线
		@callback_handle: 回调时回传回来的
	输出参数:
		@user_handle: 保存该设备的句柄，应用程序用于后续使用
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_smartconf_login(cl_handle_t *user_handle, const char *username, const char *passwd,
							cl_callback_t callback, void *callback_handle);

/*
 功能：
 	使用二维码添加一个用户(设备)
 输入参数:
 	@user_handle: 本地存放设备句柄的地址
 	@callback: 回调函数。回调事件为 UE_xxx
 状态变迁时会回调，包括登录成功、成功->失败、离线->在线、在线->离线
 	@callback_handle: 回调时回传回来的
 输出参数:
 	@user_handle: 保存该设备的句柄，应用程序用于后续使用
 返回：
 	RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_user_QR_login(cl_handle_t *user_handle, char* sn, const char *qr_code,
                          cl_callback_t callback, void *callback_handle);

/*
	功能：
		修改一个用户(设备)
	输入参数:
		@user_handle: 本地存放设备句柄的地址
		@username: 设备序列号或设备昵称
		@passwd: 设备密码
		@callback: 回调函数。回调事件为 UE_xxx
			状态变迁时会回调，包括登录成功、成功->失败、离线->在线、在线->离线
		@callback_handle: 回调时回传回来的
	输出参数:
		@user_handle: 保存该设备的句柄，应用程序用于后续使用
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_modify_login(cl_handle_t user_handle, const char *passwd,
							cl_callback_t callback, void *callback_handle);

/*
	功能：
		退出登录
	输入参数:
		@user_handle: 要退出登录的设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_logout(cl_handle_t user_handle);

/*
	功能：
		注意，这个只是在手动删除app上列表设备时才需要调用这个，原因是需要删除二维码密钥，app正常退出时用cl_user_logout正常退出
		退出登录
	输入参数:
		@user_handle: 要退出登录的设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_qr_logout(cl_handle_t user_handle);

/*
	功能：
		直接设置设备登录的IP地址
	输入参数:
		@user_handle: 要设置的设备的句柄
		@ip: 设置登录的IP地址，主机字节序
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_set_direct_login(cl_handle_t user_handle, u_int32_t ip);

/*
	功能：
		重新登录一台设备或一个用户
	输入参数:
		@user_handle: 要重新登录的设备或用户
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_relogin(cl_handle_t user_handle);


/*
	功能：
		手机帐号添加一个设备
	输入参数:
		@user_handle: 手机帐号句柄
		@dev_name: 要添加的设备的昵称或SN号
		@passwd: 设备的密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_add_dev(cl_handle_t user_handle, const char *dev_name, const char *passwd);
    
/*
 功能：
 	手机帐号添加一个二维码设备
 输入参数:
 	@user_handle: 手机帐号句柄
 	@dev_name: 要添加的设备的昵称或SN号
 	@QR_code: 设备分享的二维码
 输出参数:
 	无
 返回：
 	RS_OK: 成功
 	其他: 失败
 */
CLIB_API RS cl_user_add_QR_dev(cl_handle_t user_handle, const char *dev_sn, const char *QR_code);
/*
	功能：
		手机帐号添加一个设备
	输入参数:
		@user_handle: 手机帐号句柄
		@dev_handle: 保存本次添加的设备的句柄
		@dev_name: 要添加的设备的昵称或SN号
		@passwd: 设备的密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_add_dev_v2(cl_handle_t user_handle, cl_handle_t *dev_handle, const char *dev_name, const char *passwd);

/*
	功能：
		手机帐号删除一个设备
	输入参数:
		@dev_handle: 要删除的设备的句柄
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_del_dev(cl_handle_t dev_handle);

/*
	功能：
		获取从设备信息(目前只适用于RF网关下从设备)
	输入参数:
		@handle: 从设备句柄
	输出参数:
		无
	返回：
		NULL: 失败
*/
CLIB_API cl_slave_t *cl_get_slave_info(cl_handle_t handle);

/*
	功能：
		释放从设备信息
	输入参数:
		@slave: 从设备信息指针
	输出参数:
		无
	返回：
		无
*/
CLIB_API void cl_free_slave_info(cl_slave_t *slave);
#if 1
/*
	功能：
		获取设备信息
	输入参数:
		@user_handle: 设备句柄
	输出参数:
		无
	返回：
		NULL: 失败
		其他: 指向设备信息的指针
*/
CLIB_API cl_dev_info_t *cl_user_get_dev_info(cl_handle_t dev_handle);

/*
	功能：
		释放cl_user_get_dev_info()函数返回的设备信息
	输入参数:
		@di: cl_user_get_dev_info()函数返回的设备信息
	输出参数:
		无
	返回：
		无
*/
CLIB_API void cl_user_free_dev_info(cl_dev_info_t *di);
#endif

/*
	功能：
		获取帐号信息
	输入参数:
		@user_handle: 帐号句柄
	输出参数:
		无
	返回：
		NULL: 失败
		其他: 帐号信息指针 
*/
CLIB_API cl_user_t *cl_user_get_info(cl_handle_t user_handle);

/*
	功能：
		获取帐号信息，只获取user，不获取设备
	输入参数:
		@user_handle: 帐号句柄
	输出参数:
		无
	返回：
		NULL: 失败
		其他: 帐号信息指针 
*/
CLIB_API cl_user_t *clsdk_get_user_info(cl_handle_t user_handle) ;

/*
	功能：
		释放cl_user_get_info()函数返回的帐号信息
	输入参数:
		@info: cl_user_get_info()函数返回的帐号信息
	输出参数:
		无
	返回：
		无
*/
CLIB_API void cl_user_free_info(cl_user_t *info);

/*
	功能：
		点击某个设备的时候，调用这个接口
	输入参数:
		@dev_handle: 主设备或者从设备的handle 
		@stat: 返回的显示状态
	输出参数:
		无
	返回：
		如果返回非RS_OK，不能进入设备控制列表
*/
CLIB_API RS cl_user_set_click(cl_handle_t dev_handle, u_int8_t *stat);

/*
	功能：
		修改设备昵称(登录成功才能修改)
		本函数是异步调用会触发UE_MODIFY_NICKNAME_OK 或 UE_MODIFY_NICKNAME_FAIL事件

	输入参数:
		@user_handle: 要修改昵称的设备句柄
		@new_nickname: 新的昵称
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
*/
CLIB_API RS cl_user_modify_nickname(cl_handle_t user_handle, const char *new_nickname);

/*
	功能：
		检查密码与旧密码是否相同
		本函数是同步调用
	输入参数:
		@user_handle: 用户/设备句柄
		@passwd: 要检查的密码
	输出参数:
		无
	返回：
		true: 旧密码正确
		false: 旧密码错误
*/
CLIB_API bool cl_user_same_passwd(cl_handle_t user_handle, const char *passwd);

/*
	功能：
		修改设备/帐号的密码(登录成功才能修改)
		本函数是异步调用会触发 UE_MODIFY_PASSWD_OK 或 UE_MODIFY_PASSWD_FAIL 事件
	输入参数:
		@user_handle: 要修改昵称的设备句柄
		@new_passwd: 新的密码
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_modify_passwd(cl_handle_t user_handle, const char *new_passwd);

/*
	功能：
		修改从设备名称(登录成功才能修改)
	输入参数:
		@user_handle: 要修改昵称的设备句柄
		@new_name: 新的名称
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		UE_SET_NAME_OK
		UE_SET_NAME_FAILED
*/
CLIB_API RS cl_slave_modify_name(cl_handle_t slave_handle, const char *new_name);

/*
	功能：
		绑定从设备
	输入参数:
		@slave_handle: 从设备句柄
		@passwd: 绑定密码
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
*/
CLIB_API RS cl_slave_bind(cl_handle_t slave_handle, const char *passwd);

/*
	功能：
		解绑定从设备
	输入参数:
		@slave_handle: 从设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
*/
CLIB_API RS cl_slave_unbind(cl_handle_t slave_handle);

/*
	功能：
		注册一个手机用户账号
	输入参数:
		@phone_number: 手机号码
		@callback: 回调函数。回调事件为 
			UE_PHONE_USER_REGISTER_OK、UE_PHONE_USER_REGISTER_FAILED
		@callback_handle: 回调时回传回来的
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_register(const char *phone_number, 
				cl_callback_t callback, void *callback_handle);
/*
	功能：
		重置手机账号密码
	输入参数:
		@phone_number: 手机号码
		@callback: 回调函数。回调事件为 
		@callback_handle: 回调时回传回来的
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_PHONE_USER_RESET_OK/UE_PHONE_USER_RESET_FAILED。
		如果是UE_PHONE_USER_RESET_OK，需要用户输入验证码、新密码
*/
CLIB_API RS cl_user_reset_passwd(const char *phone_number, 
				cl_callback_t callback, void *callback_handle);

/*
	功能：
		手机用户发送验证码
	输入参数:
		@phone_number: 手机号码
		@password: 密码
		@vcode: 认证码
		@callback: 回调函数。回调事件为 
			UE_PHONE_USER_GOOD_VCODE、UE_PHONE_USER_BAD_VCODE
		@callback_handle: 回调时回传回来的
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_send_vcode(const char *phone_number, 
				const char *password, const char *vcode,
				cl_callback_t callback, void *callback_handle);

/*
	功能：
		绑定手机申请
	输入参数:
		@request: 绑定手机相关参数
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_BIND_PHONE_REQUEST_OK 提交绑定申请成功
		UE_BIND_PHONE_REQUEST_OK 提交绑定申请被批准
*/
CLIB_API RS cl_user_bind_phone(cl_handle_t user_handle, cl_bind_phone_t *request);

/*
	功能：
		允许/禁止普通用户登录
	输入参数:
		allow@ 非0表示允许普通用户登录，0表示禁止普通用户登录
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: 
*/
CLIB_API RS cl_user_bind_phone_allow_normal(cl_handle_t user_handle, int allow);

/*
	功能：
		查询已绑定手机列表
	输入参数:
		无
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_BIND_PHONE_LIST 收到已绑定列表
	
*/
CLIB_API RS cl_user_bind_phone_query(cl_handle_t user_handle);
CLIB_API cl_bind_phone_list_t *cl_user_get_bind_phone_list(cl_handle_t user_handle);
CLIB_API void cl_user_free_bind_phone_list(cl_bind_phone_list_t *);
/*
	功能：
		查询绑定手机申请列表
	输入参数:
		无
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_BIND_PHONE_REQUEST_LIST 收到绑定申请列表

*/
CLIB_API RS cl_user_bind_phone_request_query(cl_handle_t user_handle);
CLIB_API cl_bind_phone_request_list_t *cl_user_get_bind_phone_request_list(cl_handle_t user_handle);
CLIB_API void cl_user_free_bind_phone_request_list(cl_bind_phone_request_list_t *list);
/*
	功能：
		对绑定手机申请进行批准/拒绝操作
	输入参数:
		action@: BIND_PHONE_ACCEPT 批准BIND_PHONE_DEN 拒绝
		uuid@: 申请者uuid
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_BIND_PHONE_OPERATION_RESULT 操作结果

*/
CLIB_API RS cl_user_bind_phone_operation(cl_handle_t user_handle, char action, const char *uuid);
CLIB_API cl_bind_phone_result_t *cl_user_get_bind_phone_operation_result(cl_handle_t user_handle);
CLIB_API void cl_user_free_bind_phone_operation_result(cl_bind_phone_result_t *result);

/*
	功能：
		查询、配置苹果推送服务
	输入参数:
		cfg@: 推送配置参数
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_APNS_CONFIG UE_APNS_CONFIG_FAIL
*/
CLIB_API RS cl_user_apns_config(cl_handle_t dev_handle, cl_apns_config_t *cfg);

/*
	功能：
		查询、配置苹果推送服务
	输入参数:
		cfg@: 推送配置参数
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知: UE_APNS_CONFIG UE_APNS_CONFIG_FAIL
*/
CLIB_API RS cl_user_apns_config_by_sn(u_int64_t sn, cl_apns_config_t *cfg);


/*
	功能：
		获取苹果推送服务配置结果
		UE_APNS_CONFIG事件时调用本函数
	输入参数:
		cfg@: 配置信息缓冲区地址
	输出参数:
		cfg@ 写入配置信息内容
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_apns_config_get(cl_handle_t dev_handle, cl_apns_config_t *cfg);

/*
	功能：
		设置后台模式
	输入参数:
		background@:true表示设置成后台模式
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_user_background(cl_handle_t user_handle, bool background);

/*
	功能：
		获取某天从设备的统计信息
	输入参数:
		@slave_handle: 从设备句柄
	输出参数:
		无
	返回：
		NULL: 失败
		其他: 成功，指向cl_dev_stat_t结构。需要用cl_free_dev_stat_info函数来释放

	事件通知:
		无
*/
CLIB_API cl_dev_stat_t *cl_get_dev_stat_info(cl_handle_t slave_handle);

/*
	功能：
		释放由cl_get_dev_stat_info()函数返回的内存资源
	输入参数:
		@info: 由cl_get_dev_stat_info()函数返回的内存资源
	输出参数:
		无
	返回：
		无

	事件通知:
		无
*/
CLIB_API void cl_free_dev_stat_info(cl_dev_stat_t * info);

/*
	功能：
		打开一个设备的远程telnet
	输入参数:
		@slave_handle: 从设备句柄
		@ip: 服务器的IP地址，主机字节序
		@port: 服务器的端口号，主机字节序
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_slave_open_telnet(cl_handle_t slave_handle, u_int32_t ip, int port);

/*
	功能：
		远程重启一个设备
	输入参数:
		@slave_handle: 从设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_slave_reboot(cl_handle_t slave_handle);

/*
	功能：
		设置空气净化器屏幕开关
	输入参数:
		@slave_handle: 从设备句柄
		@on_off: 开或关
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_805_set_screen_on_off(cl_handle_t slave_handle,bool on_off);

/*
	功能：
		设置空气蜂鸣器开关
	输入参数:
		@slave_handle: 从设备句柄
		@on_off: 开或关
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_805_set_beep_on_off(cl_handle_t slave_handle,bool on_off);


/*
	功能：
		查询空气进化器屏幕开关
	输入参数:
		@slave_handle: 从设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_805_query_screen_on_off(cl_handle_t slave_handle);

/*
	功能：
		查询空气蜂鸣器开关
	输入参数:
		@slave_handle: 从设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_805_query_beep_on_off(cl_handle_t slave_handle);

/*
	功能：
		查询设备升级包版本
	输入参数:
		@handle: 用户或设备句柄，如果是手机用户，sdk将查询绑定的所有设备
		@lang: 语言，目前支持LANG_CH LANG_EN
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起查询请求
		其他: 失败
	事件通知:
		UE_DEV_UPGRADE_READY，事件handle为slave_handle(主设备也模拟了一个slave)
		调用cl_get_dev_stat_info函数可以获取新版本信息
*/
CLIB_API RS cl_dev_upgrade_check(cl_handle_t handle, int lang);

/*
	功能：
		告诉服务器允许设备自动升级
	输入参数:
		@slave_handle: (从)设备句柄，来自UE_DEV_UPGRADE_READY事件
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起修改请求
		其他: 失败
	事件通知:
		UE_DEV_UPGRADE_SET_OK，UE_DEV_UPGRADE_SET_FAIL
*/
CLIB_API RS cl_dev_update_set(cl_handle_t slave_handle);

/*
	功能：
		告诉设备立即自动升级
	输入参数:
		@slave_handle: (从)设备句柄，来自UE_DEV_UPGRADE_READY事件
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起请求
		其他: 失败
	事件通知:
		UE_DEV_UPGRADE_NOW_OK，UE_DEV_UPGRADE_NOW_FAIL
*/
CLIB_API RS cl_dev_update_now(cl_handle_t slave_handle);

/*
	功能：
		客户端把本地升级包文件直接发送给设备升级
	输入参数:
		@handle: 设备句柄，来自UE_DEV_UPGRADE_READY事件
		@filename: 升级包文件名
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起请求
		其他: 失败
	事件通知:
		UE_DEV_UPGRADE_NOW_OK，UE_DEV_UPGRADE_NOW_FAIL
*/
CLIB_API RS cl_dev_update_cli(cl_handle_t handle, char *filename);

/*
	功能：
		客户端把本地升级包文件直接发送给设备升级,不需要头部文件
	输入参数:
		@handle: 设备句柄，来自UE_DEV_UPGRADE_READY事件
		@filename: 升级包文件名
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起请求
		其他: 失败
	事件通知:
		UE_DEV_UPGRADE_NOW_OK，UE_DEV_UPGRADE_NOW_FAIL
*/

CLIB_API RS cl_dev_update_cli_no_head(cl_handle_t handle,char *filename);


/*
	产测
*/
CLIB_API cl_pt_stat_t *cl_get_pt_stat_info(cl_handle_t dev_handle);
CLIB_API void cl_free_pt_stat_info(cl_pt_stat_t *p);

/*
	功能：
		升级单片机镜像
	输入参数:
		@handle: 设备句柄，来自UE_DEV_UPGRADE_READY事件
		@filename: 升级包文件名
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起请求
		其他: 失败
	事件通知:

*/
CLIB_API RS cl_dev_stm_update_cli(cl_handle_t handle, char *filename);

/*
	功能：
		网关指导sn升级
	输入参数:
		@handle: 设备句柄，来自UE_DEV_UPGRADE_READY事件
		@filename: 升级包文件名
		@num,指导的sn个数
		@sn,指导的sn数组
		@force, 是否强制升级，默认强制
	输出参数:
		无
	返回：
		RS_OK: 成功传给SDK。SDK开始发起请求
		其他: 失败
	事件通知:
		UE_DEV_UPGRADE_NOW_OK，UE_DEV_UPGRADE_NOW_FAIL
*/
CLIB_API RS cl_dev_stm_update_spe_cli(cl_handle_t handle, char *filename, u_int8_t num, u_int64_t *sn, u_int8_t force);

CLIB_API RS cl_dev_active(cl_handle_t handle, u_int8_t *pdata, int len);
CLIB_API RS cl_get_timezone(cl_handle_t dev_handle,u_int32_t *timezone);


CLIB_API RS cl_get_debug_info(cl_handle_t dev_handle,cl_dev_debug_info_t *debug_info);
CLIB_API RS cl_misc_tmp_24hour_line_import(cl_handle_t dev_handle, char *file);
CLIB_API RS cl_misc_humi_24hour_line_import(cl_handle_t dev_handle, char *file);
CLIB_API RS cl_dev_set_irid(cl_handle_t handle, int id);
CLIB_API RS cl_dev_flash_upgrade_query(cl_handle_t handle, flash_block_t *block);
CLIB_API RS cl_dev_flash_upgrade(cl_handle_t handle, u_int32_t num);
CLIB_API RS cl_dev_flash_erase(cl_handle_t handle, u_int32_t num);
/*
* 网络诊断调试打印开关控制
* @debug_on: 1开启，0关闭
* @size:调试文件大小限制，字节为单位，1m=1024*1024
*/
CLIB_API RS cl_dev_nd_debug_set(bool debug_on, u_int32_t size);
/*
* 网络诊断相关，上层网络切换调用，打印相应信息。
* @desc:网络切换的一些描述，随便app咋写吧。
*/
CLIB_API RS cl_dev_nd_wan_switch(char *desc);
/*
*时区设置
*/
CLIB_API RS cl_dev_timezone_set(cl_handle_t handle, int value);

/*
*小电流开关设置
*/
CLIB_API RS cl_dev_scc_onoff_set(cl_handle_t handle, int value);
/*
*擦除单片机分区镜像
*/
CLIB_API RS cl_dev_stm_erase(cl_handle_t handle);

/*
*清除dns缓存配置
*/
CLIB_API RS cl_dns_cache_clean();

/*
*调试悟空室温功率设置等
*/
CLIB_API RS cl_dev_debug_info_set(cl_handle_t handle, cl_air_debug_info_set_t *pinfo);
/*
*天电量导入
*/
CLIB_API RS cl_dev_day_ele_import(cl_handle_t handle, char *filename);

/*
	功能：
		获取用户名长度限制
	输入参数:
		@handle: 设备句柄
		@type: 0。主要是为了防止一个设备可能出现多个使用不同长度user_name的地方，默认为0。
	输出参数:
		无
	返回：
		最大限制长度。有效值是>0
	事件通知:

*/
CLIB_API int cl_user_name_len_limit_get(cl_handle_t handle, int type);

#ifdef __cplusplus
}
#endif 

#endif

