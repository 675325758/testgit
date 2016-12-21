/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_zhdhx.h
**  File:    cl_zhdhx.h
**  Author:  liubenlong
**  Date:    10/17/2016
**
**  Purpose:
**    �ǻʵ�����.
**************************************************************************/


#ifndef CL_ZHDHX_H
#define CL_ZHDHX_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/* Type definitions. */
#define 	ZHDHX_NAME_MAX		(64)
#define 	ZHDHX_KEY_NAME_MAX 	(32)

typedef struct {
	u_int8_t valid;//�Ƿ���Ч����Ϊ1ʱname����Ч
	u_int8_t name[ZHDHX_NAME_MAX];
} cl_zhdhx_key_name_t;

typedef struct {
	u_int32_t dhx_type;//���������ͣ�ֵ�Ƕ��پ��Ƕ���·
	u_int32_t on_off_stat; //��·����״̬����0bit��ʾ��һ·,0��ʾ�أ�1��ʾ��
	cl_zhdhx_key_name_t key_name[ZHDHX_KEY_NAME_MAX];
}cl_zhdhx_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	 ����:  �ǻʵ����߿�������
 
	 �������:
	 	@dev_handle,�豸handle
	 	@num:0-ff, ע�⣬0��ʾȫ��������num=0,onoff=1,��ʾȫ����0,0��ʾȫ�ء�
		@onoff:0��ʾ�أ�1��ʾ����
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdhx_onoff(cl_handle_t dev_handle, u_int8_t num, u_int8_t onoff);

/*
	 ����:  �ǻʵ����߸�·��������
 
	 �������:
	 	@dev_handle,�豸handle
	 	@num:1-ff, ��ʾ�ڼ�·
		@name:/0�������ַ����������63.
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_zhdhx_key_name_set(cl_handle_t dev_handle, u_int8_t num, char *name);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_ZHDHX_H */

