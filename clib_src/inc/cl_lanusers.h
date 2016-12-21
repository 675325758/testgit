/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: �������û�����(�����ߣ����S3����)
**  File:    cl_lanusers.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    04/18/2016
**
**  Purpose:
**    Xx.
**************************************************************************/


#ifndef CL_LANUSERS_H
#define CL_LANUSERS_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
enum {
	ACT_LUM_SET_ENABLE = 1,
	
 	ACT_LUM_SET_MODEL = 100,	// 
};

/* Type definitions. */

#pragma pack(push,1)

typedef struct {
	u_int8_t enable;
	u_int8_t pad[3];
} ucp_lanusers_manage_enable_t;

typedef struct {
	u_int32_t user_id;
	u_int8_t model[128];	// ����
} ucp_lanusers_manage_user_info_set_t;

typedef struct {
	u_int8_t event_type;
	u_int8_t pad[3];
	u_int32_t user_id;
	u_int32_t timestamp;
} ucp_lanusers_manage_user_status_t;

typedef struct {
	u_int8_t count;
	u_int8_t pad[3];
	u_int32_t start_index;
} ucp_lanusers_manage_user_records_request_t;

typedef struct {
	u_int8_t event_type;
	u_int8_t pad[3];
	u_int32_t user_id;
	u_int32_t timestamp;
} lanusers_manage_user_record_item_t;

typedef struct {
	u_int8_t count;
	u_int8_t pad[3];
	u_int32_t start_index;
	lanusers_manage_user_record_item_t items[0];
} ucp_lanusers_manage_user_records_reply_t;

typedef struct {
	u_int16_t count;
	u_int16_t pad;
	u_int32_t current_index;
} ucp_lanusers_manage_user_records_num_t;

// ������ʷ����ʱ�����ʱ�ṹ
typedef struct {
	u_int16_t total;
	u_int32_t start_index;
	u_int32_t end_index;
	u_int32_t request_start_index;	// ��ǰ�����������ʼ��ַ
	lanusers_manage_user_record_item_t *items;	// total ��items
} user_record_cache_t;

typedef struct {
	u_int8_t enable;	// �Ƿ�ʹ���û�������
	
	lanusers_manage_user_record_item_t last_record;

	// �û��ؼ������ʷ��¼
	int record_num;
	lanusers_manage_user_record_item_t *record;

	// ��������SDK��		
	bool load_config;
	u_int32_t min_index;	// ��ǰ���ص���С��¼ֵ
	user_record_cache_t rc;
} cl_lanusers_manage_info_t;
#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	����: �������ƽӿ�
		
	�������:
		@handle: �豸�ľ��
		@action: ACT_LUM_SET_ENABLE
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_lanusers_ctrl(cl_handle_t handle, u_int8_t action, u_int32_t value);

/*
	����: ���������û�ID���ֻ��ͺ�
		
	�������:
		@handle: �豸�ľ��
		@request: �����û�ID���ֻ��ͺ�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_lanusers_set_user_info(cl_handle_t handle, ucp_lanusers_manage_user_info_set_t *request);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_LANUSERS_H */

