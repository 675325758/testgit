/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: ��������
**  File:    cl_yinsu.h
**  Author:  yuanchao.wong@gmail.com
**  Date:    05/04/2016
**
**  Purpose:
**    ��������.
**************************************************************************/


#ifndef CL_YINSU_H
#define CL_YINSU_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_com_rf_dev.h"


/* Macro constant definitions. */


/* Type definitions. */
enum {
	ACT_YINSU_CTRL_LAMP,
};

typedef struct {
	cl_rf_lamp_t lamp_stat;
} cl_yinsu_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	 ����:  ��ʿ�������Ƶ�״̬
 
	 �������:
	 	@dev_handle:�豸handle
	 	@stat: ���忴�ṹ�嶨��
	 �������:
 		��
 	����:
 		RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_yinsu_lamp_ctrl(cl_handle_t dev_handle, cl_rf_lamp_stat_t* stat);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_YINSU_H */

