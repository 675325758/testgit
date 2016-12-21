#ifndef	__CL_UDP_LAMP_H__
#define	__CL_UDP_LAMP_H__

/*
LP : LAMP
GX: GaoXun
��ѶLED�����
�����ṩ6621ģ�飬ͨ��ʹ��udpЭ��
*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

//event of lamp
enum {
	LPE_BEGIN = 1700,
	LPE_CTRL_OK = LPE_BEGIN + 1, //���Ƴɹ���������Դ��ҡͷ����λ��ECO
	LPE_CTRL_FAIL = LPE_BEGIN + 2, //����ʧ��
	LPE_END = LPE_BEGIN + 99
};

/*
 ����:
    ���Ƶ�Դ����
 �������:
    @dev_handle:  �豸���
    @is_on:  true:ON  false:OFF
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_power(cl_handle_t dev_handle, u_int8_t is_on);

/*
 ����:
    ����ȫ��
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_full_light(cl_handle_t dev_handle);

/*
 ����:
    ���ư׹�
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_white(cl_handle_t dev_handle);

/*
 ����:
    ����ů��
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_warm(cl_handle_t dev_handle);

/*
 ����:
    ���ƻ��
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_mix(cl_handle_t dev_handle);

/*
 ����:
    ��������
 �������:
    @dev_handle:  �豸���
    @brightness:  ����ֵ��ȡֵ��Χ1-100
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_brightness(cl_handle_t dev_handle, u_int8_t brightness);

/*
 ����:
    ����ů��
 �������:
    @dev_handle:  �豸���
    @warmness:  ů��ֵ��ȡֵ��Χ1-100
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_warmness(cl_handle_t dev_handle, u_int8_t warmness);

/*
 ����:
    ���ƺ�������ɫ
 �������:
    @dev_handle:  �豸���
    @r: ��ɫ��ȡֵ��Χ0x00-0xFF
    @g: ��ɫ��ȡֵ��Χ0x00-0xFF
    @b: ��ɫ��ȡֵ��Χ0x00-0xFF
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_rgb(cl_handle_t dev_handle, u_int8_t r, u_int8_t g, u_int8_t b);

/*
 ����:
    �����Զ�ģʽ
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_auto(cl_handle_t dev_handle);

/*
 ����:
    ���ƽ���ģʽ
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_energy_saving(cl_handle_t dev_handle);

/*
 ����:
    ����ҹ��
 �������:
    @dev_handle:  �豸���
    @is_on: true:ON  false:OFF
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_night(cl_handle_t dev_handle, u_int8_t is_on);

/*
 ����:
    ����ģʽ1
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_model1(cl_handle_t dev_handle);

/*
 ����:
    ����ģʽ2
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_ctrl_model2(cl_handle_t dev_handle);

/*
 ����:
    ��ѯ���й���״̬
 �������:
    @dev_handle:  �豸���
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_lamp_query_all(cl_handle_t dev_handle);

#ifdef __cplusplus
}
#endif 

#endif

