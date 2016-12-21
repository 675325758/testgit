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
	����: ����һ����WIFIģ��͸���Ĵ�������
		
	�������:
		@dev_handle: �豸�ľ��
		@cmd: �Զ��崮������
		@cmd_len: �����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_evm_tt_send_uart_cmd(cl_handle_t dev_handle, u_int8_t *cmd, u_int8_t cmd_len);
CLIB_API int cl_get_tt_cmd_info(cl_handle_t dev_handle, void *info);


#ifdef __cplusplus
}
#endif 


#endif

