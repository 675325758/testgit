#ifndef	__CL_IF_H__
#define	__CL_IF_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"


//event
enum {
	IF_BEGIN = 1500,
	/*��ȡ��Ϣ�ɹ� */
	IF_QUERY_OK = 1501,
	/*��ȡ��Ϣʧ��*/
	IF_QUERY_FAIL = 1502,
	IF_END = IF_BEGIN + 99
};

typedef struct cl_if_info_s{

	/*  ʵ���¶� */
	int16_t temp;

	/*  ʵ��ʪ�� */
	u_int8_t rh;

	/* PM2.5��ֵ */
	u_int16_t pm25;

	/* VOC ��ֵ */
	u_int16_t voc;
} cl_if_info_t;



/*
	����:
		��ѯ����ת����Ϣ
	�������:
		@dev_handle: ����ת�������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_intelligent_forward_query(cl_handle_t dev_handle);




#ifdef __cplusplus
}
#endif 

#endif


