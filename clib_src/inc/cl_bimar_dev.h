#ifndef	__CL_BIMAR_DEV_H__
#define	__CL_BIMAR_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif 
  
//////////////////////////////////
// BIMAR C2000
#define BIM_MAX_C_TEMP 37
#define BIM_MIN_C_TEMP 5
    
#define BIM_MAX_F_TEMP 99
#define BIM_MIN_F_TEMP 41
    
#define BIM_PREVENTFREEZEMODE_C_TEMP 7
#define BIM_PREVENTFREEZEMODE_F_TEMP 45
    
#define BIM_DEV_HC2000L 0x0
#define BIM_DEV_0505L 0x1
#define BIM_DEV_5162L 0x2

#define BIM_MAX_SET_TIME_HOUR 15
////////////////////////////////////////
// �Ŀ��� 0505L
#define AKM_0505L_MAX_C_TEMP 49
#define AKM_0505L_MIN_C_TEMP 10
#define AUCMA_MAX_SET_TIME_HOUR 8
    
#define AKM_5162L_MAX_C_TEMP 49
#define AKM_5162L_MIN_C_TEMP 10
    
    
enum{
    AUCMA_GEAR_WIND = 0x1, //�������ȴ���
    AUCMA_GEAR_LOW, //һ��
    AUCMA_GEAR_HIGH //����
};
    
enum{
    BIM_FAN_SPEED_UNKNOWN = 0x0,
    BIM_FAN_SPEED_LOW, //�ͷ�
    BIM_FAN_SPEED_MID, //�з�
    BIM_FAN_SPEED_HIGH //�߷�
};
    
enum{
    BIM_TEMP_UNIT_UNKNOWN = 0x0,
    BIM_TEMP_UNIT_FAHR,  //���϶�
    BIM_TEMP_UNIT_CELS //���϶�
};
    
enum{
    BIM_GEAR_UNKNOWN = 0x0,
    BIM_GEAR_LOW, //��
    BIM_GEAR_MID, //��
    BIM_GEAR_HIGH //��
};
    
enum{
    BIM_GEAR_800W = 0x1, //
    BIM_GEAR_1200W, //
    BIM_GEAR_2000W, //
    BIM_GEAR_800W_EXT //800W�ӷ�˪��
};
    
typedef enum{
    BIM_CTRL_TYPE_UNKNOWN,
    BIM_CTRL_TYPE_ONOFF,
    BIM_CTRL_TYPE_ANION,
    BIM_CTRL_TYPE_SHAKE,
    BIM_CTRL_TYPE_TEMP_UNIT,
    BIM_CTRL_TYPE_GEAR,
    BIM_CTRL_TYPE_CELS_TEMP,
    BIM_CTRL_TYPE_FAHR_TEMP,
    BIM_CTRL_TYPE_TIMER
}BIM_CTRL_ACTION_E;

typedef struct{
    u_int8_t is_data_valid; //�����Ƿ���Ч����SDK�Ƿ��ȡ���豸��״̬��
    u_int8_t is_on; //����״̬
    u_int8_t is_shake; //ҡͷ״̬
    u_int8_t is_anion_enable; //������״̬
    u_int8_t fan_speed; //����
    u_int8_t temp_unit; // �¶ȵ�λ
    u_int8_t power_gear; //���ʵ�λ
    u_int8_t cur_set_cels_temp; //��ǰ�����¶� ���϶� 5-37
    u_int8_t cur_set_fahr_temp; //��ǰ�����¶� ���϶� 41-99
    u_int8_t cur_set_timer; //���õĶ�ʱʱ��,Сʱ
    u_int8_t cur_remain_min;//��cur_set_timer���ʹ�ã���ʾʣ����ٷ���
    int8_t cels_room_temp; //���϶�����
    int8_t fahr_room_temp; //���϶�����
    u_int8_t machine_type; // APP ���ô˲���
    u_int32_t loc_update_gnu_time; //���ظ���ʱ�䣬�����������ʱ�������ʣ����ٷ���
   	u_int8_t scm_run_mode;//�����������Ų��� AUCMA_RUN_MODE_NONE  AUCMA_RUN_MODE_TEMP  AUCMA_RUN_MODE_GEAR
    //SDKʹ�ã��ϲ��������
    u_int8_t old_set_cels_temp;
	u_int8_t old_gear;
	u_int8_t pad;
}cl_bimar_heater_info_t;
    

/*
 ����:
    ���ø��ֲ���
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
    
// ע�⣬��������Ч��is_data_valid ���� false��ʱ,���ô˺����᷵��ʧ��
CLIB_API RS cl_bimar_ctrl(cl_handle_t dev_handle,BIM_CTRL_ACTION_E type,u_int8_t value);
    
#ifdef __cplusplus
}
#endif 

#endif

