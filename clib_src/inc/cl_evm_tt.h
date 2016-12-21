#ifndef CL_EVM_TT_H
#define CL_EVM_TT_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_EVM_TT_BUF_SIZE 1024
#define MAX_EVM_TT_BUF_NUM 16

typedef struct {
	u_int32_t widx;
	u_int8_t buf[MAX_EVM_TT_BUF_SIZE];
} evm_tt_buffer_t;

typedef struct {
	u_int32_t num;
	evm_tt_buffer_t caches[MAX_EVM_TT_BUF_NUM];
} cl_evm_tt_info_t;

/*
	功能: 发送一个让WIFI模组透传的串口命令
		
	输入参数:
		@dev_handle: 设备的句柄
		@cmd: 自定义串口命令
		@cmd_len: 命令长度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_evm_tt_send_uart_cmd(cl_handle_t dev_handle, u_int8_t *cmd, u_int8_t cmd_len);
CLIB_API int cl_get_tt_cmd_info(cl_handle_t dev_handle, void *info);


#ifdef __cplusplus
}
#endif 


#endif

