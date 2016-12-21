#ifndef	__CL_JS_WAVE_H__
#define	__CL_JS_WAVE_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

// ����ģʽ
enum{
    JS_WM_NONE = 0,
    JS_WM_WAVE , //΢��ģʽ
    JS_WM_UNFREE, //�ⶳģʽ
    JS_WM_WAVE_BARBECUE, //΢��+�տ�
    JS_WM_AUTO_MENU //�Զ��˵�ģʽ
};

// ������ģʽ

//�տ�ģʽ����ģʽ
enum{
    JS_SWM_B_TOP_BARBECUE = 0x1, // �����տ�����
    JS_SWM_B_BOTTOM_BARBECUE, // �ײ��տ�����
    JS_SWM_B_TB_BARBECUE // �ײ��Ͷ���������
};

//���ģʽ����ģʽ
enum{
    JS_SWM_WB_TOP_BARBECUE = 0x1, // ΢��+�����տ�����
    JS_SWM_WB_BOTTOM_BARBECUE, // ΢��+�ײ��տ�����
    JS_SWM_WB_TB_BARBECUE, // ΢��+�ײ��Ͷ���������
    JS_SWM_WB_HF_ALL_BARBECUE, // ΢��+�ȷ�+�ײ��Ͷ���������
    JS_SWM_WB_HF // ΢��+�ȷ�
};

//�Զ��˵�����ģʽ
enum{
    JS_SWM_AUTO_RICE = 0x1, //�׷�
    JS_SWM_AUTO_VEGETABLES, //�߲�
    JS_SWM_AUTO_MEAT,// ����
    JS_SWM_AUTO_NOODLE,//����
    JS_SWM_AUTO_FISH, // ����
    JS_SWM_AUTO_TIPPING, // �Զ�����
    JS_SWM_AUTO_POTATO, // ����
    JS_SWM_AUTO_PIZZA, // ������
    JS_SWM_AUTO_CHICKEN, // ����
    JS_SWM_AUTO_DEHYDRATION, // ��ˮ����
    JS_SWM_AUTO_POPCORN, // ���׻�����
    JS_SWM_AUTO_CARROT_MUD, // ���ܲ���
    JS_SWM_AUTO_APPLE_MUD, //ƻ������
    JS_SWM_AUTO_MILK_COFFEE, // ţ�̿���
    JS_SWM_AUTO_INSTANT_NOODLE, //������
    JS_SWM_AUTO_BREAD, //���
    JS_SWM_AUTO_APPLEPAI,//ƻ����
    JS_SWM_AUTO_WIFEPIE,//���ű�
    JS_SWM_AUTO_BIGBEEF,//��ţ�Ϳ���
    JS_SWM_AUTO_GOUGHNUT, //����Ȧ
    JS_SWM_AUTO_FRENCHBIG,//��ʽ��ħ��
};
    
enum{
    JS_UACT_START = 0x1, //����
    JS_UACT_CANCEL, //ȡ��
    JS_UACT_PAUSE // ��ͣ
};
    
typedef struct{
    u_int8_t is_data_valid; // �����Ƿ���Ч
    u_int8_t is_waiting; //�Ƿ����
    u_int8_t is_working; //�Ƿ����ڹ���
    u_int8_t is_pausing; //�Ƿ���������ͣ����״̬
    u_int8_t child_lock_onoff; //ͯ��״̬
    u_int8_t is_door_open; //�豸���Ƿ��
    u_int8_t is_fault_stat; //�豸�Ƿ��ڹ���״̬
    u_int8_t is_chain_stat; //����״̬
    u_int8_t work_mode; //����ģʽ,�� JS_WM_XX()
    u_int8_t work_sub_mode; //������ģʽ
    u_int8_t setting_min; // ʱ�䣺����
    u_int8_t setting_sec; // ʱ�䣺����
    u_int8_t wave_fire; //΢������
    u_int8_t barbecue_fire; // �տ�����
    u_int8_t hot_fan_temp; // �ȷ��¶�
    u_int16_t food_weight; // ʳ������
    u_int8_t cur_temp; // ��ǰ�¶�
    u_int8_t remain_min; //��ǰʱ��
    u_int8_t remain_sec; // ��ǰ����
    u_int32_t local_refresh_time; //ˢ��״̬���ֻ�utcʱ�䣬���Ը��ݸ�ʱ�䣬����ʣ��ķ��Ӻ�����
}cl_js_wave_stat_t;
    
typedef struct {
    u_int8_t work_mode; //����ģʽ
    u_int8_t work_min; //���������� 1-60����
    u_int8_t work_sec; //�������� 0-60 �룬��������0ʱ��������������0
    u_int8_t wave_fire; //΢��������С
    u_int8_t barbecue_fire; // �տ�����
    u_int8_t hot_fan_temp; // �ȷ��¶�
    u_int16_t food_weight; // ʳ�������������Ƿ���Ҫ��
    u_int8_t work_sub_mode; //������ģʽ
    u_int8_t action; // ������ ������ȡ������ͣ
}cl_js_wave_work_setting_t;
    
typedef struct{
    cl_js_wave_stat_t stat;
}cl_js_wave_info_t;

/*
 ���ܣ�
 	�����Զ���ģʽ
 �������:
 	@handle: �豸���
 �������:
 	��
 ���أ�
 	RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_js_wave_ctrl(cl_handle_t dev_handle,cl_js_wave_work_setting_t* setting);


/*
 ���ܣ�
	 ���ƿ���������ֹͣ����ͣ
 �������:
	 @handle: �豸���
 �������:
	 ��
 ���أ�
	 RS_OK:
 	����: ʧ��
 �¼�֪ͨ:
 
 */
CLIB_API RS cl_js_wave_fast_ctrl(cl_handle_t dev_handle,u_int8_t action);
    
/*
 ���ܣ�
 	����ͯ��
 �������:
 	@handle: �豸���
 �������:
 	��
 ���أ�
 	RS_OK:
 	����: ʧ��
 	�¼�֪ͨ:
 
 */
CLIB_API RS cl_js_wave_ctrl_child_lock(cl_handle_t dev_handle,u_int8_t on_off);

#ifdef __cplusplus
}
#endif 

#endif

