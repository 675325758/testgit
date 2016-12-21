/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zhcl.h
**  File:    cl_zhcl.h
**  Author:  liubenlong
**  Date:    04/19/2016
**
**  Purpose:
**    �ǻʴ���.
**************************************************************************/


#ifndef CL_ZHCL_H
#define CL_ZHCL_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */
//status
enum{
	ZHCL_STATUS_OPEN = 0,
	ZHCL_STATUS_CLOSE = 1,
	ZHCL_STATUS_STOP = 3,
};

//type
enum{
	//����
	ZHCL_INDEX_1 = 0,
	//��ɴ
	ZHCL_INDEX_2 = 1,
};

enum{
	//���ҿ���
	ZHCL_TYPE_1 = 0,
	//���¿���
	ZHCL_TYPE_2 = 1,
};

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
typedef struct {
	u_int32_t magic;//��������
	u_int8_t index;//0������1��ɴ
	u_int8_t status;//Open = 0 :����  STOP =1��ֹͣ  CLOSE = 3���ر�
	u_int8_t percent;//����λ�ðٷְ�	
	u_int8_t type;//�������ͣ�0��������1������
	u_int8_t support_dir;//�Ƿ�֧��ת��,0��֧�֣�1��ʾ֧��
	u_int8_t dir;//0��ʾû����1��ʾ�ѻ���
}cl_zhcl_info_t;


//////////////////////////////////////////////////////////////////////////////////
//// �ǻʴ���API
/*
	 ����:  �ǻʴ���������״̬����
 
	 �������:
	 	status:Open = 0 :����  STOP =1��ֹͣ  CLOSE = 3���ر�
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhcl_status_set(cl_handle_t dev_handle, u_int8_t status);

/*
	 ����:  �ǻʴ���������λ������
 
	 �������:
	 	location: 0~100,���Ǹ��ٷְ�
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhcl_location_set(cl_handle_t dev_handle, u_int8_t location);

/*
	 ����:  �ǻʴ�����������
 
	 �������:
	 	magic:xxx
	 	index:xxx
	 	type:1/2
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhcl_bind(cl_handle_t dev_handle, u_int32_t magic, u_int8_t index, u_int8_t type);

/*
	 ����:  �ǻʴ�����������������
 
	 �������:
	 	type: 0/1
	 	index:xxx
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhcl_type_set(cl_handle_t dev_handle, u_int8_t type, u_int8_t index);

/*
	 ����:  �ǻʴ�������������
 
	 �������:
	 	dir: 0��ʾû�л���1��ʾ�ѻ���
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhcl_dir_set(cl_handle_t dev_handle, u_int8_t dir);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZHCL_H */

