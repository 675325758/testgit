/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_car.h
**  File:    cl_car.h
**  Author:  liubenlong
**  Date:    06/23/2015
**
**  Purpose:
**    Cl_car.h.
**************************************************************************/


#ifndef CL_CAR_H
#define CL_CAR_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */

/* Type definitions. */
#pragma pack(push,1)
typedef struct {
	u_int32_t time;//��������ʱ��
	u_int8_t id;//������Ϣid
	u_int8_t pad[3];
}cl_car_alarm_t;
#pragma pack(pop)


typedef struct {
	u_int8_t on;//����������״̬
	//���ò���
	u_int8_t on_keep_time;//����������ʱ�䣬����Ϊ��λ��

	//ʣ������ٷֱȣ���д�ţ���û�ҵ��㷨,0-100
	u_int8_t ele_percentage;

	u_int8_t horn_num;//���������
	u_int8_t horn_time;//���ȳ�����ʱ�䣬��0.1sΪ��λ
	u_int8_t horn_interval;//����������ļ��ʱ��,��sΪ��λ
	u_int8_t light_num;//�ƹ���˸����
	u_int8_t light_time;//�ƹ���˸����ʱ��
	u_int8_t light_interval;//���εƹ���˸���ʱ��

	//״̬����
	u_int8_t temp;//�����¶� 
	u_int8_t val_on;//��ȫ��ѹ��⿪��
	u_int16_t valtage;//��ȫ��ѹ��λ��0.1v
	u_int8_t powersave_on;//���ܿ���
	u_int8_t last_on_time;//�ϴο���������������ʱ�䣬�Է���Ϊ��λ

	//������Ϣ;
	u_int16_t alarm_num;//������Ϣ����
	//������Ϣ���飬��alarm_num��Ͻ�������
	cl_car_alarm_t *car_alarm;
	//debuginfo
	char *debug_info;
}cl_car_info_t;

//Ѱ�����ܲ���
typedef struct {
	u_int8_t horn_num;
	u_int8_t horn_time;
	u_int8_t horn_interval;
	u_int8_t light_num;
	u_int8_t light_time;
	u_int8_t light_interval;
}cl_car_search_t;

//�¼�
enum {
	CAR_CTRL_EVENT_ON = 1,//�����յ�
	CAR_CTRL_EVENT_OFF,//�رտյ�
	CAR_ELE_EVENT_FULL,//�ѳ�����
	CAR_ELE_EVENT_FILLING,//���ڳ��
	CAR_ELE_EVENT_LESS,//�������㣬����
	CAR_CTRL_ALARM_ON_FAILED = 100,//����������ʧ��,������
	CAR_CTRL_ALARM_AC_ON_FAILED,//�����յ�ʧ�ܣ��յ�δ������
	CAR_CTRL_ALARM_VALTAGE,//��ƿ��ѹ����
};

/* External function declarations. */

/* Macro API definitions. */


/* Global variable declarations. */
/*
 ����:
	���÷���������ʱ��
 �������:
    @dev_handle: �豸�ľ��
    @keep_time: ���ÿ�������ʱ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_config_keeptime(cl_handle_t dev_handle, u_int8_t keep_time);

/*
 ����:
	����������
 �������:
    @dev_handle: �豸�ľ��
    @on: 0���رգ�1����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_ctrl_on(cl_handle_t dev_handle, u_int8_t on);   

/*
 ����:
	Ѱ�����ܲ�������
 �������:
    @dev_handle: �豸�ľ��
    @search: ����Ѱ������
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_config_search(cl_handle_t dev_handle, cl_car_search_t *search);

/*
 ����:
	����Ѱ������
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_ctrl_search(cl_handle_t dev_handle);


/*
 ����:
	��ȫ��ѹ����
 �������:
    @dev_handle: �豸�ľ��
    @valtage: ��ȫ��ѹ��λ��0.1v
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_config_valtage(cl_handle_t dev_handle, u_int16_t valtage);


/*
 ����:
	��ȫ��ѹ���ܿ���
 �������:
    @dev_handle: �豸�ľ��
    @on: ��ȫ��ѹ���أ�1����0��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_ctrl_valtage(cl_handle_t dev_handle, u_int8_t on);

/*
 ����:
	���ܹ��ܿ���
 �������:
    @dev_handle: �豸�ľ��
    @on: ���أ�1����0��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_car_ctrl_powersave(cl_handle_t dev_handle, u_int8_t on);



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_CAR_H */

