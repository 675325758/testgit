#ifndef	__CLIENT_LIB_H__
#define	__CLIENT_LIB_H__


#ifdef __cplusplus
extern "C" {
#endif 

/*
	client libary header.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef INT8 int8_t;
typedef INT16	int16_t;
typedef INT32 int32_t;
typedef UINT8	u_int8_t;
typedef UINT16	u_int16_t;
typedef UINT32	u_int32_t;
typedef UINT64	u_int64_t;
typedef INT32 int32_t;
//#define	LOCALTIME_YEAR_BASE	1900
#define PRIu64	"llu"
#define PRIx64	"llx"
#else
#include <sys/types.h>
#include <inttypes.h>
//#define	LOCALTIME_YEAR_BASE	1970
#endif

#include "cl_type_def.h"

//localtime的时间在linux和mac上都是从1900开始
#define	LOCALTIME_YEAR_BASE	1900

#define CONFIG_SN_DICT  1

#if 1 //def __GNUC__
	//typedef unsigned char bool
#ifdef bool
#undef bool
#endif
    #define	bool unsigned char
	#define	true	1
	#define	false	0
#endif

//加个宏控制下那个utc转换的函数。。。。
#define USE_TIME_MINS

/*
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;

#define	FALSE	0
#define	TRUE	1
*/

#ifdef WIN32

#if 1
#ifdef CLIB_EXPORTS
#define CLIB_API __declspec(dllexport)
#else
#define CLIB_API __declspec(dllimport)
#endif
#else
#define CLIB_API
#endif

#else

#define CLIB_API

#endif

/*******************************************
	client libary, 2013
	
	module include:
		user
		video
		
	action include: 
		login/logout
		set/get
		init/start/stop
		add/modify/del
		
********************************************/

#ifndef	RS_OK
	/* return status */
	typedef int RS;

	/* 操作成功完成 */
	#define	RS_OK	0
	/* 出错 */
	#define	RS_ERROR			-1
	#define	RS_NOT_INIT		-2
	#define	RS_NOT_SUPPORT	-3
	/* 尚未登录 */
	#define	RS_NOT_LOGIN		-4
	// 无效的参数
	#define	RS_INVALID_PARAM	-5
	// 没有可用数据
	#define	RS_EMPTY	-6
    //内存分配失败
    #define RS_MEMORY_MALLOC_FAIL  -7
	/* 添加时已存在 */
	#define	RS_EXIST	1
	/* 未找到 */
	#define	RS_NOT_FOUND	2
	/*无效授权文件*/
	#define RS_INVALID_LICENSE 3
	/* 设备离线 */
	#define RS_OFFLINE 4
	/*联动圈子删除失败，因为是默认圈子*/
	#define RS_HOME_DEL_FAILED 5
	/*红外是否匹配信息还没获取到=>提示"正在同步设备状态，请稍候"*/
	#define RS_IR_SYNING 6
	/*红外没有匹配=>提示“未匹配编码”*/
	#define RS_IR_HAVE_NO_MATCH 7
	/*上层下发数据过长，超哥300k*/
	#define RS_DATA_TOO_BIG		8
#endif

#define	INVALID_HANDLE	0
typedef u_int32_t cl_handle_t;

typedef struct {
	// 事件
	u_int32_t event;
	// 对象句柄
	cl_handle_t obj_handle;
	// 来自用户最初设置的callback_handle，这里回传回去
	void *callback_handle;
	// 错误号。登录失败看ULGE_xxx，其他的大部份可参看ds_proto.h中的ERR_XXX
	int err_no;
} cl_event_more_info_t;

/*
	回调函数。
	在事件回调中，参数 callback_handle 可以强制转化成 cl_event_more_info_t 指针类型使用
*/
typedef void (* cl_callback_t)(u_int32_t event, void *user_handle, void *callback_handle);



/*******************************************

	module: libary
		
********************************************/

/****************************************************
	一些公共数据结构
 ****************************************************/

typedef enum {
	OT_UNKNOW = 0, /* 未知类型 */
	OT_SLAVE = 1, /* 从设备。数组中第一个是主设备 */
	OT_USB_VIDEO = 2, /* 带云台摄像头 */
	OT_EQUIPMENT = 3, /* 电视、空调等电器和安防报警等设备 */
    OT_KEY = 4,
} object_type_t;

// slave bind status
typedef enum {
	BMS_UNBIND,
	BMS_BINDING,
	BMS_BIND_ONLINE,
	BMS_BIND_OFFLINE,
	// local add: bind error, maybe bad password
	BMS_BIND_ERROR,
	// 已经绑定过得设备，原来网关不可用
	// 尝试重新绑定，app检查到本地曾经绑定过，直接允许绑定
	// 否则不提示发现新设备，用户找回旧设备才显示
	BMS_REBIND,
	//登陆中，主要用来表示本地缓存设备的初始化状态
	BMS_LOGINING,
	BMS_MAX,
} cl_bms_t;

/*
	一个对象的公共部分。
	该对象可能是摄像头，可能是插座，可能是电视机、空调，可能是安防报警电器，等等
*/
typedef struct {
	// 该对象类型: OT_XXX
	object_type_t type;
	
	// 当前状态, BMS_xxx
	cl_bms_t status;
	
	// 该对象在SDK库内唯一标识
	cl_handle_t handle;
	// 名称，UTF-8格式
	char *name;
	// 所属的从设备sn
	u_int64_t sn;
	
	// 保留给调用者使用
	void *priv;
} cl_obj_t;

// H.264视频自定义分辨率参数限制
typedef struct {
	int width;
	int height;
	// 最后以0结束，比如 5 10 15 20 25 0 0 0
	int fps[8];
} cl_video_param_t;

//  一些系统限制。长度限制都没包含最后的\0，仅仅指数据部分
typedef struct {
	//  用户名、密码
	int max_user_name_len;
	int max_user_passwd_len;

	// 从设备名字、模块名字
	int max_mod_name_len;

	// wifi
	int max_wifi_ssid_len;
	int max_wifi_passwd_len;
	
	// 区域
	int max_area;
	int max_area_name_len;

	// 场景
	int max_scene;
	int max_scene_name_len;

	//  电器
	int max_equipment;
	int max_eq_name_len;
	
	// 每个电器下的按键
	int max_key_of_eq;
	int max_key_name_len;
	/* 报警推送消息的长度，与max_key_name_len一样 */
	int max_alarm_msg_len;

	// 健康
	int max_belter_user;
	int max_belter_user_name_len;
    //身高，单位：厘米
    int min_belter_user_height;
    int max_belter_user_height;
    //体重，单位：千克
    int min_belter_user_weight;
    int max_belter_user_weight;
    //年龄，岁
    int min_belter_user_age;
    int max_belter_user_age;

	/* 智能插座定时开关 */
	int max_008_timer; 
	int max_008_timer_name_len;

	/* 录像定时器 */
	int max_record_timer; 
	int max_record_timer_name_len;

	/* 情景定时器 */
	int max_scene_timer;
	int max_scene_timer_name_len;

	/* 手机绑定的几个参数限制 */
	int max_phone_model_len; /* 手机型号 */
	int max_bind_name_len;/*绑定名称*/
	int max_bind_message_len;/*绑定描述*/
    /*视频饱和度参数限制*/
    int max_video_brighness_val;//最大亮度
    int min_video_brighness_val;//最小亮度
    int max_video_contrast_val;//对比度
    int max_video_saturation_val;//饱和度
    int max_video_gain_val;//补偿
    int max_video_roll_speed; //最大视频转动速度

	// 视频分辨率参数限制
	cl_video_param_t video_param[3];
} cl_limit_t;

#define FLASH_UPGRADE_BLOCK_NUM			(3)
typedef struct flash_block_s{
	u_int32_t flash_addr;
	u_int32_t valid;
	u_int32_t soft_ver;
	u_int32_t svn;
	u_int32_t len;
	u_int32_t crc;	
	u_int32_t run;
}flash_block_t;     

/*********************************************************************************************/


/*
	功能：
		初始化库，申请需要的资源
	参数IN：
		client_type: 客户端类型，见CID_XXX
	返回：

*/

enum{
	CID_IOS = 0,
	CID_ANDROID = 1,
	CID_WIN32 = 3,
	CID_MAX
};

enum{
	APP_TYPE_DEFAULT, //未知
	APP_TYPE_INTEL_HOME,//智慧家庭
	APP_TYPE_SUPER_HOME,//超级智能家居
	APP_TYPE_AIR_PLUG, //挂机宝
	APP_TYPE_LINKAGE_SUPPORT = 7,//联动支持app
};

#define APP_TYPE_IWUHOME APP_TYPE_AIR_PLUG
    
#define MAX_UUID_LEN 40
#define MAX_UUID_BIN_LEN 16

#define MAX_TEST_IP_NUM	(10)

#define MAX_TRANS_IP_NUM (50)
    
typedef struct {
	// IN. 本机的时区
	int timezone;
	// IN. 本客户端的OEM id. 0-galaxywind, 10-qh
	int vvid;
	//app id
	u_int32_t app_id;
	//oem
	char oem_vendor[16];
	// IN. 本机的唯一标识
	char uuid[MAX_UUID_LEN];
	// 工作目录，用来保存一些信息到本地
	char dir[200];
	//添加个联动用的目录，主要是为了好删除
	char priv_dir[200];
	// IN. 保留
	char *reserved;
	//应用程序类型
	u_int32_t app_type;
    // 手机描述,手机名(用户取的xxx的手机之类的)或手机型号
    u_int8_t phone_desc[64];

	//IN.app版本，上传服务器做判断用的,a.b.c.d形式 major,minor,revise
	u_int8_t app_ver[3];
	
	// OUT. SDK版本号, a.b.c.d形式
	u_int8_t version[4];
	// OUT. 描述
	char desc[64];
	// OUT. SDK的SVN号
	u_int32_t svn;

	// 一些系统限制: 区域、场景、电器、帐号密码长度...等等
	cl_limit_t limit;
	// 非零表示产测模式
	int is_pt_mode;
	//test server ip
	u_int8_t test_ip_num;
	u_int32_t test_ip[MAX_TEST_IP_NUM];
} cl_lib_info_t;

CLIB_API RS cl_init(u_int32_t client_type, cl_lib_info_t *info);

/*
	功能：
		重新设置SDK的基本信息
	输入参数:
		@info: 
			@@timezone: 本机时区
			@@vvid: 厂家的vvid，galaxywind为0
			@@uuid: 本机唯一标识
	输出参数:
		@info: 
			@@veresion: SDK版本号
			@@desc: 描述
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_set_info(cl_lib_info_t *info);

/*
	功能：
		获取SDK的基本信息
	输入参数:
		无
	输出参数:
		@info: 填充相关信息
	返回：
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_get_info(cl_lib_info_t *info);

/*
	手机网络连接类型，包括: 无连接、Wi-Fi、2G/3G、以太
*/
#define	NET_TYPE_NONE	0
#define	NET_TYPE_WIFI	1
#define	NET_TYPE_3G	2
#define	NET_TYPE_ETH	3
/*
	功能：
		设置当前客户端连接网络的类型
	输入参数:
		@net_type: NET_TYPE_xxx，网络类型，包括: 无连接、Wi-Fi、2G/3G、以太
		@desc: 一些描述
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_set_net_type(int net_type, char *desc);

/*
	功能：
		设置app获取到的广播地址
	输入参数:
		@baddr: 广播地址
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	事件通知:
		无
*/
CLIB_API RS cl_set_net_baddr(u_int32_t baddr);

/*
	功能：
		结束库，终止相关线程，释放所有资源
	参数：

	返回：

*/
CLIB_API RS cl_stop();

typedef struct {
	// 服务器返回的客户端最新版本号
	char *iphone_newest_version;
	char *android_newest_version;
	
	// 苹果版本的中英文描述
	char *desc_iphone_en;
	char *desc_iphone_ch;
	
	// 安卓版本的中英文描述
	char *desc_android_en;
	char *desc_android_ch;
} cl_phone_version_t;

/*
	功能：
		获取服务器返回的手机版本号信息
	输入参数:
		无
	输出参数:
		无
	返回：
		NULL: 失败
		其他: 手机版本信息
	事件通知:
		无
*/
CLIB_API cl_phone_version_t *cl_get_phone_version( );

/*
	功能：
		释放cl_get_phone_version() 函数返回的内存
	输入参数:
		@info: cl_get_phone_version() 函数返回的内存
	输出参数:
		无
	返回：
		无
	事件通知:
		无
*/
CLIB_API void cl_free_phone_version(cl_phone_version_t *info);

typedef struct {
    u_int64_t tx_bytes; //发包字节数
    u_int64_t rx_bytes; //收包字节数
    u_int32_t tx_pkts; // 发包数
    u_int32_t rx_pkts; //收包述
}cl_traffic_stat_t;

/*
 功能：
    设置是否开启流量监测
 输入参数:
    @is_enable true：开启  false:关闭
 输出参数:
    无
 返回：
    无
 事件通知:
    无
 */
CLIB_API RS cl_set_traffic_stat_enable(bool is_enable);

/*
 功能：
 
 输入参数:
    @stat: 状态统计数据
    @is_clear_data: 是否清空统计数据
 输出参数:
    无
 返回：
    无
 事件通知:
 无
 */
CLIB_API void cl_get_traffic_stat(cl_traffic_stat_t* stat,bool is_clear_data);

/*
 功能：
    @设置手机进入后台
 输入参数:
 输出参数:
    无
 返回：
    无
 事件通知:
    无
 */
CLIB_API RS cl_set_phone_background(bool is_background);
    CLIB_API cl_traffic_stat_t* cl_get_tcp_cmd_traffic_stat(int* out_num,bool is_clear_data);


#ifdef __cplusplus
}
#endif 

#endif

