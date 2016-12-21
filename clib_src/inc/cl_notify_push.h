#ifndef	__CL_NOTIFY_PUSH_H__
#define	__CL_NOTIFY_PUSH_H__

#ifdef __cplusplus
extern "C" {
#endif 
 
#include "cl_notify_def.h"
#include "client_lib.h"

    
enum {
    NE_BEGIN = 500,

    NE_NOTIFY,
    
    // 查询报警信息后收到的报警信息
    // 调用cl_get_alarm获取报警，
    // 使用完成后，调用cl_cmt_free_alarm 释放内存
    NE_ALARM_LOG,
    // 收到服务器或设备端主动推送的报警
    // 调用cl_get_alarm获取报警，
    // 使用完成后，调用cl_cmt_free_alarm 释放内存
    NE_ALARM_PUSH_LOG,
    //收到设备所属小区列表
    //调用cl_get_community获取小区列表
    //再根据手机保存的小区report_id和消息最大report_id
    //调用cl_notify_query按需查询小区公告
    NE_CMT_LIST,

    NE_END = NE_BEGIN + 99
};

/*
     功能:
        1、设备账号设置期望接收报警消息
        2、手机账号设置期望接收的推送消息
     输入参数:
         @user_handle: 设备句柄
         @expect: 期望接收的设备序列号以及期望收到的最低消息序号
         @callback: 回调函数地址
         @handle: 回调函数参数
     输出参数:
         无
     返回:
         RS_OK: 成功
     其他: 失败
     关注事件:NE_NOTIFY，NE_ALARM_LOG，NE_ALARM_PUSH_LOG
     
     注意事项:
 */ 
CLIB_API RS cl_set_notify_expect(cl_handle_t user_handle, notify_expect_t* expect,cl_callback_t callback, void *handle);

/*
     功能:
        获取报警消息列表
     输入参数:
         @user_handle: 设备句柄
     输出参数:
         无
     返回:
         非空: 成功
     其他: 失败
     
     注意事项:
 */ 
CLIB_API alarm_msg_list_t *cl_get_alarm(cl_handle_t user_handle);

/*
     功能:
        释放报警消息列表内存
     输入参数:
         @ptr: cl_get_alarm 返回的报警消息列表
     输出参数:
         无
     返回:
         非空: 成功
     其他: 失败
     
     注意事项:
 */ 
CLIB_API void cl_free_alarm(alarm_msg_list_t *ptr);

/*
     功能:
         获取一条推送消息
     输入参数:
         @user_handle: 用户句柄
     输出参数:
         无
     返回:
         非空: 成功
     其他: 失败
     
     注意事项:
     NE_NOTIFY事件调用本函数获取一条推送消息
     使用完成后，调用cl_free_notify释放内存
 */ 
CLIB_API notify_msg_t *cl_get_notify(cl_handle_t user_handle);
CLIB_API void cl_free_notify(notify_msg_t *ptr);

/*
     功能:
         获取多条推送消息
     输入参数:
         @user_handle: 用户句柄
     输出参数:
         无
     返回:
         非空: 成功
     其他: 失败
     
     注意事项:
     NE_NOTIFY事件调用本函数获取多条条推送消息
     使用完成后，调用cl_free_notify_list释放内存
 */ 
CLIB_API notify_msg_list_t *cl_get_notify_list(cl_handle_t user_handle);
CLIB_API void cl_free_notify_list(notify_msg_list_t *ptr);

/*
     功能:
        获取设备所属小区列表
     输入参数:
         @dev_handle: 设备句柄
         @cmt:  存放小区列表内存
         @cnt:  存放小区数量内存指针
     输出参数:
         @cmt: 小区列表内容
         @cnt: 小区个数
         无
     返回:
         RS_OK: 成功 
         其他: 失败
     
     注意事项:
     在NE_CMT_LILST事件调用cl_get_community函数
     并用小区的max_report_id与本地保存的进行比较
     如果本地的更小，则需要调用cl_query_cmt_notify函数进行查询获取
 */
CLIB_API RS cl_get_community(cl_handle_t dev_handle, cmt_notify_info_t cmt[MAX_CMT], int *cnt);

/*
     功能:
        查询小区推送消息
     输入参数:
         @dev_handle: 设备句柄
         @query: 查询条件首地址，一次可以查询多个小区
         @cnt: query个数 
     输出参数:
         无
     返回:
         RS_OK : 成功
         其它: 失败     
     注意事项:
     1、使用手机账号登录时，可能有多台设备，每台设备有自己的小区列表
            一次查询的多个小区必须属于同一台设备
     2、查询结果回来后，NE_NOTIFY事件通知上层
 */ 
CLIB_API RS cl_query_cmt_notify(cl_handle_t dev_handle, cmt_notify_query_t *query, int cnt);

#ifdef __cplusplus
}
#endif 


#endif



