#ifndef CL_EVM_H_
#define CL_EVM_H_

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push ,1)

typedef struct {
	u_int32_t flash_addr;
	u_int32_t valid;
	u_int32_t soft_ver;
	u_int32_t svn;
	u_int32_t len;
	u_int32_t crc;	
	u_int32_t run;
	u_int8_t pack_name[32];
	u_int8_t base_vm_name[32];
} evm_block_item_t;

typedef struct {
	u_int8_t n;
	u_int8_t pad[3];
	evm_block_item_t item[8];
} evm_block_t;

#pragma pack(pop)


CLIB_API RS cl_dev_update_evm(cl_handle_t handle, char *filename);
CLIB_API RS cl_dev_update_evm_info(cl_handle_t handle);
CLIB_API RS cl_dev_evm_info_get(cl_handle_t handle, evm_block_t *block);
CLIB_API RS cl_evm_flash_erase(cl_handle_t handle, u_int32_t num);


#ifdef __cplusplus
}
#endif


#endif
