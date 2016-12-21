#ifndef	__CL_YJ_HEADER_H
#define	__CL_YJ_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)

typedef struct {
	// �ϲ�����
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t cmd;
	u_int8_t dev_type;
	u_int8_t param_len;
	// �ϲ�����

	// ״ֻ̬������
	u_int8_t onoff; // 0 off 1 on
	u_int8_t temp_set;// �����¶�5 - 36
	u_int8_t temp_type; // �¶����� 0 ���϶� 1 ���϶�
	u_int8_t anion;	// ������
	u_int8_t power;// 0 low 1 high
	u_int8_t child_lock; // ͯ�� 0  �� 1 ��
	int8_t temp_now;	// ��ǰ���£�����Ϊ����
	u_int8_t timer;   // ��ʱ�� �ﵽ����ʱ����Զ��ػ� 0-24
	u_int8_t  err_code;	
	u_int8_t process;	//  0---����˸  1---������˸  2---�Ͽ���˸
	// ״ֻ̬������
	
	// �ϲ�����
	u_int8_t pad1[4];
	u_int8_t  check_sum;
} cl_yj_heater_info_t;

typedef struct {
	// �ϲ�����
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t cmd;
	u_int8_t pad;
	u_int8_t param_len;
	// �ϲ�����

	// ״ֻ̬������
	u_int8_t onoff;	// 0 off 1 on
	u_int8_t temp_set;	// �����¶�5 - 36
	u_int8_t temp_type;	// �¶����� 0 ���϶� 1 ���϶�
	u_int8_t anion;	// ������1 ��
	u_int8_t power;// ���� 0 low 1 high
	u_int8_t child_lock; // ͯ�� 0  �� 1 ��
	u_int8_t timer;		// ��ʱ�� �ﵽ����ʱ����Զ��ػ� 0-24
	// ״ֻ̬������

	u_int8_t pad1[3];
	u_int8_t checksum;
} cl_yj_heater_set_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 



/*
 ���ܣ�
 	����¿�������
 �������:
 	@handle: �豸���
 	@ctrl: ���Ʋ���
 �������:
 	��
 ���أ�
 	RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_yj_heater_set_ctrl(cl_handle_t dev_handle, cl_yj_heater_set_t *ctrl);


#endif
