#ifndef	__CL_JCX_POWER_BOX_H__
#define	__CL_JCX_POWER_BOX_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_JCX_CHANNEL  0x10

typedef struct {
	u_int16_t voltage; //��ѹ ��λ0.001V
	u_int16_t elec;  //���� ��λ0.001A
	int32_t active_power;	// �й����� ��λ0.1W, �з�������APP ��JNIת��
	int32_t reactive_power; // �޹����� ��λ 0.1 ����,�з�������APP ��JNIת��
	int16_t power_factor;	// �������أ���λ0.001  , �з�������APP ��JNIת��
	u_int16_t frequency; //Ƶ��
	u_int32_t active_degree;	// ��Ч��� ��λǧ��ʱ
	u_int32_t reactive_degree;	// ��Ч��� ��λǧ����ʱ
	u_int32_t jcx_sn;				// ���к�
	u_int16_t jcx_soft_ver;			// ����汾
	u_int16_t jcx_hardware_ver;		// Ӳ���汾
	u_int8_t channel_num; //ͨ����
	u_int8_t pad;
	u_int16_t channelstat; //ͨ��ͨ��״̬
	char* channel_names[MAX_JCX_CHANNEL]; //ͨ������
} cl_jcx_power_box_info;

#ifdef __cplusplus
}
#endif 


#endif

