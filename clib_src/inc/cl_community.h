#ifndef	__CL_COMMUNITY_H__
#define	__CL_COMMUNITY_H__
/*
文件名: cl_community.h
智慧小区接口函数头文件*/

/*
广域网模式是指小区和设备都能够连接到服务器，通过服务器协调小区和设备的通信
广域网模式主要使用流程：
1、初始化SDK，调用函数： cl_init
2、小区登录，调用函数： cl_cmt_login 

3、小区程序进入运行状态，主要需要处理以下事件
	UE_LOGIN_OFFLINE：离线登录成功（没有连接到服务器，第一次登录或与服务器断开连接会收到本事件），
					第一次登录需要查询本地数据库设备列表，并调用cl_cmt_add_device把设备添加到SDK。

	UE_LOGIN_ONLINE：在线登录成功（成功连接到服务器），向服务器查询本小区管理的设备列表，调用函数：cl_cmt_query_device_list

	CE_QUERY_DEVICE：到服务器查询设备列表结果回来了，调用函数cl_cmt_get_device_list获取服务器上的设备列表
					并将服务器列表和本机数据库保存的设备列表进行比较，对差异部分进行同步（调用函数：cl_cmt_add_device，cl_cmt_del_device）

	CE_ADD_DEVICE：添加本小区管理设备到服务器结果回来了，调用cl_cmt_get_add_result获取添加结果
	CE_DEL_DEVICE：从服务器删除本小区管理设备结果回来了，调用cl_cmt_get_del_result获取删除结果
	
	CE_NOTIFY_HELLO：收到设备握手请求，调用cl_cmt_get_notify_hello获取hello消息，
					查找小区期望收到设备的下一条消息id，调用cl_cmt_send_notify_hello_ack发送确认
					然后根据设备hello里面expect_report_id，到本地数据库查询本小区发布的通知消息，
					并按需调用cl_cmt_send_notify把设备期望的消息发送过去
	
	CE_NOTIFY_RESULT：收到设备对本小区发送消息的确认事件，可以发送下一条消息了。

	CE_NOTIFY：收到设备通知消息，调用cl_cmt_get_notify获取消息内容并展示到界面。
	CE_ALARM：收到设备报警消息，调用cl_cmt_get_alarm获取报警内容并展示到界面。
	

4、小区登出，调用函数：cl_cmt_logout
5、终止SDK，调用函数：cl_stop
6、小区程序退出
*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_notify_def.h"
#include "cl_health.h"

/*
CMT_PORT 小区服务器监听端口
局域网环境下，小区服务器需要固定端口，
便与设备直接和小区服务器通信
*/
#define CMT_PORT 5362

// community event
enum {
	CE_BEGIN = 300,

	// 收到设备握手事件, 
	// 先调用cl_cmt_get_notify_hello 获取hello内容
	// 再调用cl_cmt_send_notify_hello_ack 发送确认
	// 最后根据hello里面expect_report_id判断是否有新的消息需要发送
	CE_NOTIFY_HELLO,
	
	// 收到消息通告,
	// 目前有TNOTIFY_EMERGENCY, TNOTIFY_NORMAL, TNOTIFY_AD
	// 目前只是小区发送给设备	
	// 先调用cl_cmt_get_notify 获取通知消息
	// 再调用cl_cmt_send_notify_result 发送确认
	CE_NOTIFY,
	
	// 收到报警
	// 调用cl_cmt_get_alarm获取报警，
	// 使用完成后，调用cl_cmt_free_alarm 释放内存
	// 目前只是设备发送给小区
	CE_ALARM,

	// 收到健康测量
	// 目前只是设备发送给小区
	CE_MESURE,
	
	// 发送消息给设备后，收到消息确认事件
	// 如果本地还有新的消息，调用cl_cmt_send_notify发送
	CE_NOTIFY_RESULT,
	
	// 收到添加设备结果事件
	// 调用cl_cmt_get_add_result 获取添加结果
	CE_ADD_DEVICE,	
	
	// 收到删除设备结果事件
	// 调用cl_cmt_get_del_result 获取删除结果
	CE_DEL_DEVICE,	
	
	// 收到查询设备列表结果事件
	// 调用cl_cmt_get_device_list 获取查询结果
	CE_QUERY_DEVICE,

	// 出错
	CE_ERROR = CE_BEGIN + 50,
	// 超时失败
	CE_ERR_TIMEOUT,
	CE_END = CE_BEGIN + 99
};

#pragma warning(disable: 4200)

#pragma pack(push,1)

typedef struct{                      /* 小区服务器信息数据结构*/
	u_int64_t sn;                     /* 序列号*/
	u_int32_t service_ip[16];  /* 服务ip列表*/
	u_int16_t service_port;    /* 服务端口*/
	u_int8_t ip_count;            /* 有效服务ip个数*/
	u_int8_t reserved;            /* 保留*/	
}cmt_info_t;

typedef struct{         /* 设备信息数据结构 */
	u_int64_t sn;        /* 设备序列号*/
	char phone[128]; /* 设备拥有者电话，多个用'\t'隔开*/
	char name[64];    /* 设备名称，例如: 3栋3单元202*/
}device_info_t;

typedef struct{
	RS result;	       /* 设备操作结果:RS_OK, RS_NOT_FOUND, RS_XXX*/
	device_info_t info;
}device_status_t;

typedef struct{
	int total;	/* 设备总个数*/
	int count;	/* 本次收到的设备个数*/
	device_status_t *device; /* 设备数组*/
}device_list_t;

typedef struct{      /* 握手消息TLV */
	u_int16_t type;
	u_int16_t len;
	u_int8_t value[0];
}notify_hello_tlv_t;

typedef struct{          /* 握手请求数据结构*/
	u_int64_t dst_sn;  /* 接收者序列号 */
	u_int64_t src_sn;  /* 发送者序列号 */
	u_int8_t versiona;  /*  发送者版本信息*/
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int16_t tlv_count;        /* TLV参数个数*/
	u_int16_t tlv_data_len;   /* TLV参数总长度*/
	u_int8_t tlv_data[0];       /* TLV数据，notify_hello_tlv_t，目前未使用*/
}notify_hello_t;

typedef struct{    /* 握手响应数据结构*/
	u_int64_t dst_sn;    /* 接收者序列号 */
	u_int64_t src_sn;    /* 发送者序列号 */
	u_int8_t versiona;  /*  发送者版本信息*/
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int8_t reserved[3];      /* 保留*/
	u_int16_t tlv_count;        /* TLV参数个数*/
	u_int16_t tlv_data_len;   /* TLV参数总长度*/
	u_int8_t tlv_data[0];       /* TLV数据，notify_hello_tlv_t，目前未使用*/
}notify_hello_ack_t;

typedef struct{ /* 健康测量记录 */
	u_int64_t dev_sn;
	u_int64_t report_id;
	family_t member;		/* 被测量者 */
	measure_t measure;	/* 测量值 */
}cl_measure_t;

typedef struct{
	int count;
	cl_measure_t **list;
}cl_measure_list_t;

#pragma pack(pop)

/*
	函数：cl_cmt_login
	功能：
		智慧小区登录

	参数：
		user_handle: 登录成功分配唯一用户句柄
		filename: 智慧小区授权文件名
				每个智慧小区服务器具有不同的授权
		callback: 回调函数, 事件为UE_XXX , CE_XXX
		callback_handle: 回调时候回传给回调函数
		
	返回：
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_login(cl_handle_t *user_handle, const char *filename,
						cl_callback_t callback, void *callback_handle);

/*
	函数：cl_cmt_logout
	功能：
		智慧小区登出

	参数：
		user_handle: 连接句柄
				
	返回：
		RS_OK , RS_XXX
*/						
CLIB_API RS cl_cmt_logout(cl_handle_t user_handle);

/*
	函数：cl_cmt_self_info
	功能：
		获取智慧小区信息

	参数：
		user_handle: 连接句柄
		info: 输出本小区信息

	返回：
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_self_info(cl_handle_t user_handle, cmt_info_t *info);

/*
	函数：cl_cmt_query_device_list
	功能：
		到服务器查询本小区管理的爱家设备列表
		只有在小区连接到服务器后(事件UE_LOGIN_ONLINE)，才能发送本请求
		本过程为异步方式，
		在回调CE_QUERY_DEVICE事件时，
		调用cl_cmt_get_device_list获取实际列表，
		使用完成后，调用cl_cmt_free_device_list释放内存

	参数：
		user_handle: 连接句柄
		sn: 0表示查询所有的设备，非0表示查询具体某台设备
				
	返回：
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_query_device_list(cl_handle_t user_handle, u_int64_t sn);

/*
	函数：cl_cmt_get_device_list
	功能：
		获取查询本小区管理的爱家设备列表
		使用完成后，调用cl_cmt_free_device_list释放内存

	参数：
		user_handle: 连接句柄
						
	返回：
		成功返回设备列表device_list_t 
		失败返回NULL

*/
CLIB_API device_list_t *cl_cmt_get_device_list(cl_handle_t user_handle);

/*
	函数：cl_cmt_free_device_list
	功能：
		释放设备列表内存，该内存由cl_cmt_get_device_list 分配
		
	参数：
		ptr: 要释放的设备列表
						
	返回：
		无

*/
CLIB_API void cl_cmt_free_device_list(device_list_t *ptr);

/*
	函数：cl_cmt_add_device
	功能：
		智慧小区添加设备到本小区
		本过程为异步方式，
		在回调CE_ADD_DEVICE 事件时调用cl_cmt_get_add_result获取添加结果

	参数：
		user_handle: 连接句柄
		device_info_t: 要添加的设备信息				
	返回：
		RS_OK , RS_XXX
		如果返回RS_OK，表示成功添加到接口库，局域网模式可以工作了。
		服务器操作结果等待异步回调通知
*/
CLIB_API RS cl_cmt_add_device(cl_handle_t user_handle, device_info_t *device);

/*
	函数：cl_cmt_del_device
	功能：
		智慧小区删除设备
		本过程为异步方式，
		在回调CE_DEL_DEVICE 调事件时调用cl_cmt_get_del_result 获取删除结果

	参数：
		user_handle: 连接句柄
		device_info_t: 要删除的设备信息				
	返回：
		RS_OK , RS_XXX
		如果返回RS_OK, 表示从接口库中删除成功。
		服务器操作结果等待异步回调通知		
*/
CLIB_API RS cl_cmt_del_device(cl_handle_t user_handle, device_info_t *device);

/*
	函数：cl_cmt_get_add_result
	功能：
		获取添加设备结果
		收到回调事件CE_ADD_DEVICE时调用本函数，
		使用完成后，调用cl_cmt_free_dev_result释放内存

	参数：
		user_handle: 连接句柄
		
	返回：
		成功返回添加设备结果device_status_t
		失败返回NULL
		
*/
CLIB_API device_status_t *cl_cmt_get_add_result(cl_handle_t user_handle);

/*
	函数：cl_cmt_get_del_result
	功能：
		获取删除设备结果
		收到回调事件CE_DEL_DEVICE时调用本函数，
		使用完成后，调用cl_cmt_free_dev_result释放内存

	参数：
		user_handle: 连接句柄
		
	返回：
		成功返回删除设备结果device_status_t
		失败返回NULL
		
*/
CLIB_API device_status_t *cl_cmt_get_del_result(cl_handle_t user_handle);

/*
	函数：cl_cmt_free_dev_result
	功能：
		释放设备添加、删除结果内存
		该内存由cl_cmt_get_del_result ，cl_cmt_get_del_result 分配
		
	参数：
		ptr: 要释放的内存
		
	返回：
		无
		
*/
CLIB_API void cl_cmt_free_dev_result(device_status_t *ptr);

/*-----------------------------------------------------------------------------------*/
/*
	函数：cl_cmt_send_notify_expect
	功能：
		通知公网服务器小区期望收到设备消息id。
		在小区连接到公网服务器情况下(事件UE_LOGIN_ONLINE)，
		并且和服务器完成设备列表同步后，
		利用本命令和服务器同步设备消息

	参数：
		user_handle: 连接句柄
		expect: 期望同步消息设备信息
		count: 本次发送同步设备信息个数，
			由于小区管理的设备可达上千个，
			需要多次发送，每次最多发送100个。
				
	返回：
		成功返回RS_OK，其它失败

*/
CLIB_API RS cl_cmt_send_notify_expect(cl_handle_t user_handle, notify_expect_t* expect, int count);

/*
	函数：cl_cmt_get_notify_hello
	功能：
		获取notify hello 消息
		在回调 CE_NOTIFY_HELLO 调事件时调用本函数获取推送消息具体内容
		使用完成后，调用cl_cmt_free_notify_hello释放内存

	参数：
		user_handle: 连接句柄
				
	返回：
		消息内容 notify_hello_t

*/
CLIB_API notify_hello_t *cl_cmt_get_notify_hello(cl_handle_t user_handle);

/*
	函数：cl_cmt_free_notify_hello
	功能：
		释放内存，该内存由cl_cmt_get_notify_hello分配
	参数：	
		ptr: 要释放的内存
		
	返回：
		无

*/
CLIB_API void cl_cmt_free_notify_hello(notify_hello_t *ptr);

/*
	函数：cl_cmt_send_notify_hello_ack
	功能：
		发送notify hello ack 消息
		在回调 CE_NOTIFY_HELLO 调事件时调用本函数发送ack
		
	参数：
		user_handle: 连接句柄
		ack: notify hello ack消息内容
				
	返回：
		RS_OK, RS_XXX
*/
CLIB_API RS cl_cmt_send_notify_hello_ack(cl_handle_t user_handle, notify_hello_ack_t *ack);

/*
	函数：cl_cmt_get_alarm
	功能：
		获取报警消息
		在回调 CE_ALARM 调事件时调用本函数获取推送消息具体内容
		使用完成后，调用cl_cmt_free_alarm释放内存

	参数：
		user_handle: 连接句柄
				
	返回：
		报警内容 alarm_msg_list_t

*/
CLIB_API alarm_msg_list_t *cl_cmt_get_alarm(cl_handle_t user_handle);
CLIB_API void cl_cmt_free_alarm(alarm_msg_list_t *ptr);

/*
	函数：cl_cmt_get_notify
	功能：
		获取推送消息
		在回调 CE_NOTIFY 调事件时调用本函数获取推送消息具体内容
		使用完成后，调用cl_cmt_free_notify释放内存

	参数：
		user_handle: 连接句柄
				
	返回：
		消息内容 notify_msg_t

*/
CLIB_API notify_msg_t *cl_cmt_get_notify(cl_handle_t user_handle);

/*
	函数：cl_cmt_free_notify
	功能：
		释放内存，该内存由cl_cmt_get_notify分配
	参数IN：	
		ptr: 要释放的内存
		
	返回：
		无

*/
CLIB_API void cl_cmt_free_notify(notify_msg_t *ptr);


/*
	函数：cl_cmd_send_notify_new
	功能：
		智慧小区新发布电子公告
		本过程为异步方式
		回调 CE_NOTIFY_RESULT 事件时确认某台设备已经收到
		注意：目前与设备通信只支持UDP协议，一次最多能发送约8KB数据，
			建议把电子公共主要内容放在超链接里面。
	参数 IN：	
		user_handle: 连接句柄
		msg: 电子公共内容，dev_sn 字段为0表示广播发送给所有设备
				dev_sn 为具体某台设备序列号，则只发送给该设备
	参数OUT：
		send_count: 输出具体发给了多少台设备(包括服务器)

	返回：
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_send_notify_new(cl_handle_t user_handle, notify_msg_t *msg, int *send_count);

/*
	函数：cl_cmd_send_notify_old
	功能：
		智慧小区收到同步请求，把历史公告发送过去
		本过程为异步方式
		回调 CE_NOTIFY_RESULT 事件时确认某台设备已经收到
		注意：目前与设备通信只支持UDP协议，一次最多能发送约8KB数据，
			建议把电子公共主要内容放在超链接里面。
	参数 IN：	
		user_handle: 连接句柄
		msg: 电子公共内容，dev_sn 字段为0表示广播发送给所有设备
				dev_sn 为具体某台设备序列号，则只发送给该设备
		peer_sn: 如果对端是设备，peer_sn为设备序列号，0表示对端是服务器
	参数OUT：
		send_count: 输出具体发给了多少份，同步历史公告为点到点，应该只发送一份

	返回：
		RS_OK , RS_XXX
*/
CLIB_API RS cl_cmt_send_notify_old(cl_handle_t user_handle, notify_msg_t *msg, int *send_count, u_int64_t peer_sn);

/*-----------------------------------------------------------------------------------*/
/*
	函数：cl_cmt_get_notify_result
	功能：
		获取消息确认
		回调 CE_NOTIFY_RESULT 调事件时调用本函数		
		使用完成后，调用cl_cmt_free_notify_reslut释放内存
	参数IN：	
		user_handle: 连接句柄
		

	返回：
		消息确认内容notify_msg_result_t

*/
CLIB_API notify_msg_result_t *cl_cmt_get_notify_result(cl_handle_t user_handle);

/*
	函数：cl_cmt_free_notify_reslut
	功能：
		释放内存，该内存由cl_cmt_get_notify_result 分配
		
	参数：	
		ptr: 要释放的内存
		
	返回：
		无

*/
CLIB_API void cl_cmt_free_notify_reslut(notify_msg_result_t *ptr);

/*
	函数：cl_cmt_get_measure
	功能：
		获取健康测量记录
		在回调 CE_MESURE 调事件时调用本函数获取健康测量记录
		使用完成后，调用cl_cmt_free_measure释放内存

	参数：
		user_handle: 连接句柄
				
	返回：
		健康测量记录 cl_measure_list_t

*/
CLIB_API cl_measure_list_t *cl_cmt_get_measure(cl_handle_t user_handle);
CLIB_API void cl_cmt_free_measure(cl_measure_list_t *list);

#ifdef __cplusplus
}
#endif 


#endif



