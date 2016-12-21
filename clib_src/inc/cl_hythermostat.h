#ifndef CL_HYTHERMOSTAT_H
#define CL_HYTHERMOSTAT_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

// cl_hythermostat_ctrl��action 
enum {
	ACT_HYTHERMOSTAT_ONOFF = 1,// ���ػ�1 �� 0 �ر�  (ͨ��)
	ACT_HYTHERMOSTAT_TEMP,// 0.1��  50~600 (ͨ��)
	ACT_HYTHERMOSTAT_MODE,// �յ�ģʽ 1�����䣻2�����ȣ�3���ͷ� // ˮůģʽ 1��ˮ���䣻2��ˮ��ů��
    ACT_HYTHERMOSTAT_WIND,// ������λ 0: ͣ�磻1���ͷ磻2���з磻3��ǿ�磻(���յ���)

    ACT_HYTHERMOSTAT_RHFUN,//��¶��⹦�� 0: �رգ� 1: �򿪣�(���յ���)
    ACT_HYTHERMOSTAT_RHVAL,//��¶�����ֵ 1%  10~90 (���յ���)
    
};

// cl_hythermostat
typedef struct {
    u_int16_t mcuver;   //OR  ��λ������汾
    u_int16_t type;	    //OR  �豸���ͣ�0-�յ����ͣ�1-ˮů����
    u_int16_t temp;	    //OR  ��ǰ�����¶� 0.1���϶�
    u_int16_t valve;    //OR  �յ���0-���رգ�1-��������ˮů��0-ִ�����أ�1-ִ��������
    u_int16_t onoff;	//WR ���ػ�1 �� 0 �ر� (ͨ��)========
    u_int16_t settemp;  //WR �����¶� 0.1��  50~600 (ͨ��)=========
    u_int16_t mode;	    //WR �յ�ģʽ 1�����䣻2�����ȣ�3���ͷ� // ˮůģʽ 1��ˮ���䣻2��ˮ��ů��============
    u_int16_t wind;  	//WR ������λ 0: ͣ�磻1���ͷ磻2���з磻3��ǿ�磻(���յ���)=========
    u_int16_t RHfun;	//WR ��¶��⹦�� 0: �رգ� 1: �򿪣�(���յ���)=====
    u_int16_t RHval;    //WR ��¶�����ֵ 1%  10~90 (���յ���)========
    u_int16_t RHstate;  //OR ��¶���״̬ 0: δ������1: ���ڱ���״̬

} cl_hythermostat_stat_t;


typedef struct {
	cl_hythermostat_stat_t stat;

} cl_hythermostat_info_t;



/*
	����: ���¿������ñ���
		
	�������:
		@dev_handle: �豸�ľ��
		@action: ��������ACT_HYTHERMOSTAT_XXX����value���ʹ��
			
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_hythermostat_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);



#ifdef __cplusplus
}
#endif 


#endif


