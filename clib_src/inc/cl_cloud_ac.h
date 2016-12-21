#ifndef	__CL_CLOUD_AC_H__
#define	__CL_CLOUD_AC_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"


//event
enum {
	CA_BEGIN = 1600,

	/*׼���������ȴ��������*/
	CA_WAIT_IR,
	/*�յ�������룬�ȴ�����������*/
	CA_WAIT_MATCH,
	/*������ƥ��ɹ����������ر���*/
	CA_WAIT_CODE,
	/*���ر���ɹ������Կ�����*/
	CA_DONE,
	
	/*ȡ��ƥ��ɹ�*/
	CA_CM_CANNEL,

	/*��ƥ��ʧ�ܻ�δ������ƥ��*/
	CA_NOT_MATCH,
	/*���óɹ�*/
	CA_SET_OK,
	/* �ƿյ�ƥ��ʧ�ܣ����߿���ʧ�� */
	CA_ERROR,

	CA_END = CA_BEGIN + 99
};

typedef struct{
	u_int8_t action;
	u_int8_t match_type;
	u_int16_t select_match_id;
	cl_handle_t ac_handle;
}cln_cloud_match_t;

typedef struct{
	u_int32_t key_id;
	cl_handle_t ac_handle;
}cln_ac_ctrl_t;

typedef struct{
	u_int8_t onoff:1;//����
	u_int8_t mode:3;//ģʽ
	u_int8_t temp:4;//�¶�
	u_int8_t speed:2;//����
	u_int8_t dir:2;//����
	u_int8_t key_v:3;//����ֵ
	u_int8_t oldkey_v:4;//��ʽ�յ�����ֵ
	
}cln_ac_key_id_t;



/*
	����:
		�ƿյ���ƥ��
	�������:
		@ac_handle:  �豸���
		@do_match:  1,������ƥ�䣬0��ȡ����ƥ��
		@cloud_dev_id:  �Ƶ���ID
		@match_type:  ��ƥ������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_cloud_match(cl_handle_t ac_handle,bool do_match, u_int8_t match_type);

/*
	����:
		�����ƿյ�����
	�������:
		@ac_handle:  �豸���
		@onoff:  1,��0, ��
		@temp:  �¶�,  �� ʵ���¶� - AC_TEMP_BASE
		@mode: ģʽ, AC_MODE_XXX
		@speed: ����, AC_WIND_xxx
		@dir: ����, AC_DIR_xxx
		@presskey �¿�յ����µļ�, AC_KEY_xxx
		@oldkey:  ��ʽң�ذ尴�µļ�����Ϊ��ʽ������0��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ac_send_ctl(cl_handle_t ac_handle,u_int8_t onoff, u_int8_t temp, u_int8_t mode, u_int8_t speed, u_int8_t dir, u_int8_t presskey, u_int8_t oldkey);


/*
	����:
		�����ƿյ�ƥ�����ID
	�������:
		@ac_handle:  �豸���
		@select_match_id:  ѡ��ID���±�
		@match_type:  ��ƥ������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/

CLIB_API RS cl_ac_set_match_id(cl_handle_t ac_handle, u_int16_t select_match_id, u_int8_t match_type);


#ifdef __cplusplus
}
#endif 

#endif



