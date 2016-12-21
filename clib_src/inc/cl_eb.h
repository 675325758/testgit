#ifndef	__CL_EB_H__
#define	__CL_EB_H__

#ifdef __cplusplus
extern "C" {
#endif 


/* E�����ƽ��֪ͨ��APP���¼� */
enum {
	EBE_BEGIN = 1800,
	EBE_SET_WORK_OK = EBE_BEGIN + 1,
	EBE_SET_WORK_FAULT = EBE_BEGIN + 2,
	EBE_SET_TIMER_OK = EBE_BEGIN + 3,
	EBE_SET_TIMER_FAULT = EBE_BEGIN + 4,
	EBE_END = EBE_BEGIN + 99
};


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
CLIB_API RS cl_eb_ctrl_work(cl_handle_t dev_handle, bool is_on);

/*
	����:
		��ӻ��޸Ķ�ʱ���ع���
	�������:
		@dev_handle: �豸���
		@timer: ��ʱ����Ŀ�Ĳ�����idΪ0��ʾ��ӣ�������Ϊ�޸�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_eb_timer_set(cl_handle_t dev_handle, cl_air_timer_t *timer);

/*
	����:
		ɾ����ʱ���ع���
	�������:
		@dev_handle: �����ľ��
		@id: Ҫɾ���Ĺ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_eb_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	����:
		��ӻ��޸Ķ�ʱ���ع���
	�������:
		@dev_handle: �豸���
		@timer: ��ʱ����Ŀ�Ĳ�����idΪ0��ʾ��ӣ�������Ϊ�޸�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_eb_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer);

/*
	����:
		ɾ����ʱ���ع���
	�������:
		@dev_handle: �����ľ��
		@id: Ҫɾ���Ĺ����id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_eb_period_timer_del(cl_handle_t dev_handle, u_int8_t id);

/*
	����:
		����У������
	�������:
		@dev_handle: �豸���
		@adj: ������ѹУ������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_eb_pt_adj_set(cl_handle_t dev_handle, cl_plug_pt_adkb_t *adj, u_int8_t action);
    

/*
 ����:
    ���ƽ��ܱ�LED��
 �������:
    @onoff: 0:�رգ�1��������2������
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_eb_ctrl_led_power(cl_handle_t dev_handle, u_int8_t onoff);


#ifdef __cplusplus
}
#endif 


#endif

