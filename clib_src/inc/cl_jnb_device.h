#ifndef	__CL_JNB_DEVICE_H__
#define	__CL_JNB_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum {
	JT_MODE_OFF = 0x0,
	JT_MODE_DEFORST, //��˪
	JT_MODE_HOLD_COMFORTABLE, //HOLD ����ģʽ
	JT_MODE_HOLD_ECONOMY,//HOLD ����ģʽ
	JT_MODE_AUTO, //�Զ�ģʽ
	JT_MODE_HOLIDAY // ����ģʽ
};

typedef struct {
	u_int8_t work_status;		// ָ����ģʽ�����ϡ�
	u_int8_t env_temperature;	// �����¶�
	u_int8_t temp_temperature;	// ��ʱ�����¶�
	u_int8_t vacation_remain_days; 
	u_int8_t vacation_temperature;// �������õ��¶�
	u_int8_t vacation_days;	// �����趨������
	u_int8_t comfort_temperaute;	// ����ģʽ���õ��¶�
	u_int8_t comfort_hold_time;	// ��ʱHOLDʱ�䣬��λСʱ
	u_int8_t economic_temperature;// ����ģʽ���õ��¶�
	u_int8_t economic_hold_time;	// ��ʱHOLDʱ�䣬��λСʱ
	u_int32_t scheduler[7];	// scheduler[0]��ʾ�����죬��������Ϊ����1������6��
				// ����1��24λ�ֱ��ʾÿ��ʱ��ε�ģʽѡ��25λ��ʾ�Ƿ�����:0-δ���ã�1-������
} cl_jnb_thermotat_info;


/*
	����:
		���ÿ���
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_ctrl_onoff(cl_handle_t dev_handle,u_int8_t onoff);

/*
	����:
		����ģʽ
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode);

/*
	����:
		��ʱ�����¶�
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_ctrl_temp(cl_handle_t dev_handle,u_int8_t temp);

/*
	����:
		����������ģʽ
	�������:
		@dev_handle: �����ľ��
		@schedluer, 7��u32����ֵ������ÿ���24Сʱ��// scheduler[0]��ʾ�����죬��������Ϊ����1������6��
				// ����1��24λ�ֱ��ʾÿ��ʱ��ε�ģʽѡ��25λ��ʾ�Ƿ�����:0-δ���ã�1-������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_set_schedluer(cl_handle_t dev_handle,u_int32_t* schedluer);

/*
	����:
		���ø�ģʽ�µ�Ĭ���¶Ȳ���
		// ��Ҫ���Զ�ģʽ�ͼ���ģʽ
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_set_temp_param(cl_handle_t dev_handle,u_int8_t mode,u_int8_t temp);

/*
	����:
		����hold ��ʱ�䣬0-24Сʱ
	�������:
		@dev_handle: �����ľ��
		@mode JT_MODE_HOLD_ECONOMY��JT_MODE_HOLD_COMFORTABLE��
			�������ã���ǰΪ���ʣ����þ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_set_holiday_days(cl_handle_t dev_handle,u_int8_t temp,u_int8_t days);

/*
	����:
		���ü�������
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_jnb_th_set_hold_time(cl_handle_t dev_handle,u_int8_t mode,u_int8_t hours);

#ifdef __cplusplus
}
#endif 


#endif

