#ifndef __CL_EPLUG_OEM_H__
#define __CL_EPLUG_OEM_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    

typedef struct{
    u_int8_t onoff; //����
    u_int8_t room_temp; //��ǰ����
    ///////////////////
    u_int8_t range_enable; //���·�Χenable
    u_int8_t range_max_temp; //���¸�ֵ
    u_int8_t range_min_temp;
    ///////////////////////////
    u_int8_t temp_threshold_enable; //�¶ȷ�ֵ����
    u_int8_t temp_threshold_value; //�¶ȷ�ֵ
    //////////////////////////
    u_int8_t off_line_close_enable; //���߹رչ���
    //////////////////////////
    u_int8_t is_support_person_detect; //�Ƿ�֧������̽��
    u_int8_t person_detect_enable; //����̽���Ƿ���
}cl_eplug_oem_stat;
    
/*
 ����:
    ���ƿ���
 �������:
    @dev_handle: �����ľ��
    @is_on: 0-�ػ���1������
    �������:
 ��
    ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_eo_set_onoff(cl_handle_t dev_handle, bool is_on);
    
    
/*
 ����:
    ���ƺ��·�Χ
 �������:
    @dev_handle: �����ľ��
 
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_eo_set_temp_range(cl_handle_t dev_handle, bool enable,u_int8_t max_temp,u_int8_t min_temp);

/*
 ����:
    ���ƹرղ��������·�ֵ
 �������:
    @dev_handle: �����ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_eo_set_threshold(cl_handle_t dev_handle, bool enable,u_int8_t max_temp);
    
/*
 ����:
    �������߹ػ�����
 �������:
    @dev_handle: �����ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_eo_set_off_line_close_enable(cl_handle_t dev_handle, bool enable);

/*
 ����:
    ��������̽�⹦��
 �������:
    @dev_handle: �����ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_eo_set_person_detect_enable(cl_handle_t dev_handle, bool enable);
    
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_CAR_H */

