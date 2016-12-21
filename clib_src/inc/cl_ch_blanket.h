#ifndef	__CL_CH_BLANKET_H__
#define	__CL_CH_BLANKET_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define AREA_BLANKET_LEFT 0x0
#define AREA_BLANKET_RIGHT 0x1

//�������ߵ���
#define BLANKET_CURVE_PER_HOUR_POINT  24
#define BLANKET_CURVE_HALF_HOUR_POINT  48

//����ģʽ
enum{
    BLANKET_MODE_KNOWN  = 0x0,
    BLANKET_MODE_KILL_MITES, //����
    BLANKET_MODE_DEHUMIDIFY, //��ʪ���ų�
    BLANKET_MODE_HOT, //����
    BLANKET_MODE_SLEEP //˯��ģʽ
};

    
/*
	����:
		��ѯ������Ϣ
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_query_info(cl_handle_t dev_handle, u_int8_t area);

/*
	����:
		��������
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@on_off   0x0: �ر� 1:����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_set_on_off(cl_handle_t dev_handle, u_int8_t area, u_int8_t on_off);

/*
	����:
		���������¶�
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@temp 18-48��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_set_temp(cl_handle_t dev_handle, u_int8_t area, u_int8_t temp);

/*
	����:
		�������������Ƿ���Ч
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@is_enable  0x0:�ر� 0x1:����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_set_curve_enable(cl_handle_t dev_handle, u_int8_t area, u_int8_t is_enable);

/*
	����:
		���������ֶ���ʱ
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@time  0x0:�ر� 0x1--0x9 ��ʱʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_set_timer(cl_handle_t dev_handle, u_int8_t area, u_int8_t time);

/*
	����:
		�������������(ÿСʱһ����)
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@curve: ��0����23��,�ر���0,����ʱ�����¶�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_set_hour_curve(cl_handle_t dev_handle, u_int8_t area, u_int8_t curve[BLANKET_CURVE_PER_HOUR_POINT]);

/*
	����:
		������������(ÿ��Сʱһ����)
	�������:
		@dev_handle: ����̺�ľ��
		@area  ����ѡ��
		@curve: ��0����23:30㬹ر���0,����ʱ�����¶�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_blanket_set_half_hour_curve(cl_handle_t dev_handle, u_int8_t area, u_int8_t curve[BLANKET_CURVE_HALF_HOUR_POINT]);

    
/*
 ����:
    ��������ģʽ
 �������:
    @dev_handle: ����̺�ľ��
    @area  ����ѡ��
    @on_off   BLANKET_MODE_XX
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_blanket_set_work_mode(cl_handle_t dev_handle, u_int8_t area, u_int8_t work_mode);

#ifdef __cplusplus
}
#endif 


#endif

