#ifndef	__UASC_PRIV_H__
#define	__UASC_PRIV_H__

// 处理用户请求
enum{
	ACT_UASC_START_CONNECT,
	ACT_UASC_PUSH_APP_STAT,
	ACT_MAX
};

// APP 状态上传TLV 取值
enum{
	AS_TT_UUID = 0x1, // uuid 16字节
	AS_TT_APP_VERSION , // app 版本
	AS_TT_HW_TYPE, // 数字:  硬件类型，pad还是手机
	AS_TT_HW_DESC, // 硬件描述信息，华为、iphone4s等
	AS_TT_OS_TYPE, //  数字:  Android or iphone or PC  
	AS_TT_OS_VERSION, // 操作系统版本，字符串
	AS_TT_APP_TYPE, //数字,4字节
	AS_TT_SN,
	AS_TT_UDP_CTRL
};


extern bool uasc_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret);

#endif

