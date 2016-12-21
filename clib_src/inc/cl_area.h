#ifndef	__CL_AREA_H__
#define	__CL_AREA_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

#define MAX_AREA_NAME_LENGTH (64)
    
typedef struct _cl_area_s_{
    cl_handle_t area_handle;
    u_int8_t img_resv;//�û�˽�б�־�������ڲ�ͬ�ֻ���ʾ��ͬͼƬ
    u_int8_t item_count;//��������ӵ�ж��ٸ������ʹ��豸
    u_int8_t area_id; //�����ֻ�����ͼƬ
    u_int8_t pad;
    u_int32_t create_time;//����ʱ��,��������ĳ̨�ֻ�����ͼƬ
    u_int8_t area_name[MAX_AREA_NAME_LENGTH];
    cl_handle_t items[0]; //�����ڵ����ʹ��豸���
}cl_area_t;
    
// AREA event
enum {
	AE_BEGIN = 700,
    AE_AREA_ADD_OK = AE_BEGIN+1,
    AE_AREA_ADD_FAIL = AE_BEGIN+2,
    AE_AREA_DEL_OK = AE_BEGIN+3,
    AE_AREA_DEL_FAIL = AE_BEGIN+4,
    AE_AREA_CHANGE_OK = AE_BEGIN+5,
    AE_AREA_CHANGE_FAIL = AE_BEGIN+6,
	AE_END = AE_BEGIN + 99
};
    
CLIB_API RS cl_area_add(cl_handle_t user_handle,cl_handle_t* area_handle,
                        u_int8_t img_resv,const char* area_name);
    
CLIB_API RS cl_area_del(cl_handle_t area_handle);

CLIB_API RS cl_area_change_name(cl_handle_t area_handle,const char* name);

CLIB_API RS cl_area_change_imge_resv(cl_handle_t area_handle,u_int8_t img_resv);
    
CLIB_API RS cl_area_modify_appliances(cl_handle_t area_handle,u_int8_t item_count,cl_handle_t* handles);

CLIB_API RS cl_area_modify(cl_handle_t area_handle, const char* name, 
				u_int8_t img_resv, u_int8_t item_count,cl_handle_t* handles);

CLIB_API RS cl_area_add_3(cl_handle_t user_handle, cl_handle_t *area_handle, 
				const char* name, u_int8_t img_resv, u_int8_t item_count,cl_handle_t* handles);

CLIB_API RS cl_appliance_change_area(cl_handle_t appli_handle,cl_handle_t area_handle);

#ifdef __cplusplus
}
#endif 


#endif



