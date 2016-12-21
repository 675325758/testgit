#ifndef	__CL_PLUG_H__
#define	__CL_PLUG_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

// plug event
enum {
	PE_BEGIN = 400,
	// �Ӷϵ���ͨ��
	PE_ON = PE_BEGIN + 1,
	// ��ͨ���ɶϵ�
	PE_OFF = PE_BEGIN + 2,
	// ��ʱ���б����仯����ѯ��ʱ���б��ʱ����ԼΪͳ����Ϣ��ѯ�����3��
	PE_TIMER_MODIFY = PE_BEGIN + 3,
	// ��ʱ��ѯ��������ѹ���¶ȡ��ۻ��������׶ε��������˽��
	PE_QUERY = PE_BEGIN + 4,
	// ��ӡ�ɾ�����޸Ķ�ʱͨ�����ɹ�
	PE_TIMER_SET_OK = PE_BEGIN + 5,
	// ��ӡ�ɾ�����޸Ķ�ʱͨ�����ʧ��
	PE_TIMER_SET_FAIL = PE_BEGIN + 6,
	PE_END = PE_BEGIN + 99
};

typedef struct {
	u_int8_t id;			/* ����ID */
	u_int8_t hour;			/* Сʱ 0-23 */
	u_int8_t minute;		/* ���� 0-59 */
	u_int8_t week;			/* bit 0-6λ��Ӧ�����졢����1�������� */
	u_int8_t enable;		/* �Ƿ���Ч(�ֻ�����) �����Ѿ���Ч(�豸����) */
	u_int8_t pad;			/* ���� */
	u_int16_t last;			/* �������(����) */
	u_int32_t sort;			/* ��ǰ��ʼ��������ʱ������0��ʾ��ֹ�ģ����������������Ĵ�1��ʼ */
	char *name;		/* ��������, UTF-8��ʽ */
} cl_plug_timer_t;


typedef struct {
	// �Ƿ�ͨ��
	u_int16_t is_on;
	// ��������λ: 1/10 ����
	u_int16_t current;
	// ��ѹ����λ: ����
	u_int16_t voltage;
	// �¶ȣ���λ: ���϶�
	u_int16_t temperature;

	/* �ӿ��������ڵ��ܵ���,��λ W */
	u_int32_t electric_total; 
	/* ��timeָ��ʱ�ĵ���ǰ�Ľ׶ε���,��λ W */
	u_int32_t electric_section; 
	/* �׶ε�����ʲôʱ��ʼ�� */
	u_int32_t section_begin_time;

	// ��ʱ���ж�����
	u_int32_t num_timer;
	// ��ʱ��ָ������
	cl_plug_timer_t **timer;

	// ����next_on��next_time�Ƿ���Ч��������ʾ��ʱ����һ�εĶ����ǿ����ǹأ�Ҫ�ȶ��
	bool next_effect;
	// ��һ����ͨ�绹�Ƕϵ�
	bool next_on;
	// ��Ҫ��÷�������λ����
	u_int32_t next_minute;
} cl_plug_info_t;



/*
	����:
		��ʼ��ʱ��ѯĳ��ң�ز�����״̬
	�������:
		@slave_handle: ң�ز����ľ��
		@seconds: �������ѯһ��
		@callback: �ص�����
		@handle: �ص����������������Լ�ʹ�ã�SDK�����ĸò���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_plug_query_start(cl_handle_t slave_handle, u_int32_t seconds, cl_callback_t callback, void *handle);

/*
	����:
		ֹͣ��ʱ��ѯĳ��ң�ز�����״̬
	�������:
		@slave_handle: ң�ز����ľ��
	�������:
		��
	����:
		��
*/
CLIB_API RS cl_plug_query_stop(cl_handle_t slave_handle);

/*
	����:
		����ң�ز���
	�������:
		@slave_handle: ң�ز����ľ��
		@index: �����ĸ���ס�0��ʾ���С�Ŀǰ���Ըò���
		@on: 1��ʾͨ�磬0��ʾ�ϵ�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_plug_turn_on(cl_handle_t slave_handle, u_int32_t index, bool on);

/*
	����:
		��ӻ��޸Ķ�ʱͨ�����
	�������:
		@slave_handle: ң�ز����ľ��
		@index: �����ĸ���ס�0��ʾ���С�Ŀǰ���Ըò���
		@plug_timer: Ҫ���õĶ�ʱͨ�����
			plug_timer->name����ΪUTF-8��ʽ������С��64�ֽڡ�
			���plug_timer->id��Ч��Ϊ�޸Ĳ���
		@tz: ʱ������λΪСʱ�����綫����Ϊ8
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_plug_timer_set(cl_handle_t slave_handle, u_int32_t index, cl_plug_timer_t *plug_timer, int32_t tz);

/*
	����:
		�ѽ��ݵ�����0
	�������:
		@slave_handle: ң�ز����ľ��
		@index: �����ĸ���ס�0��ʾ���С�Ŀǰ���Ըò���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_plug_clear_electric_stat(cl_handle_t slave_handle, u_int32_t index);

/*
	����:
		ɾ����ʱͨ�����
	�������:
		@slave_handle: ң�ز����ľ��
		@index: �����ĸ���ס�0��ʾ���С�Ŀǰ���Ըò���
		@id: Ҫɾ���Ĺ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_plug_timer_del(cl_handle_t slave_handle, u_int32_t index, int id);

	
/*
	����:
		��ȡ��ң�ز����ĵ�ǰһЩ��Ϣ
	�������:
		@slave_handle: ң�ز����ľ��
		@index: �����ĸ���ס�Ŀǰ���Ըò���
	�������:
		��
	����:
		NULL: ʧ��
		����: �ɹ�, ���ص� cl_plug_info_t * ָ����ڴ�飬��Ҫ��cl_plug_free_info�����ͷ�
*/
CLIB_API cl_plug_info_t *cl_plug_get_info(cl_handle_t slave_handle, u_int32_t index, int32_t tz);

/*
	����:
		�ͷ� cl_plug_get_info() �������ص��ڴ��
	�������:
		@info: cl_plug_get_info() �������ص��ڴ��
	�������:
		��
	����:
		��
*/
CLIB_API void cl_plug_free_info(cl_plug_info_t *info);

#ifdef __cplusplus
}
#endif 


#endif



