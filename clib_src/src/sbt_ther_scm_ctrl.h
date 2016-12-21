#ifndef	__SBT_THER_SCM_CTRL_H__
#define	__SBT_THER_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"
#include "udp_device_common_priv.h"
#include "cl_common_udp_device.h"
#include "cl_yl_thermostat.h"

#define SBT_START_CODE1 0xFF
#define SBT_START_CODE2 0xAA

enum{
    SBT_SCM_CMD_STAT = 0x1,
    SBT_SCM_CMD_FUNC_PARAM ,
    SBT_SCM_CMD_TIME_ADJUST = 0x4,
    SBT_SCM_CMD_AUTO_PARAM
};

#pragma pack(push,1)

typedef struct {
    u_int8_t start_code1;
    u_int8_t start_code2;
    u_int8_t command;
    u_int8_t device_type;
    u_int8_t data_len;
    u_int8_t pad[3];
}ucp_sbt_scm_head_t;

typedef struct {
    u_int8_t temp; //�¶� 10-32
    u_int8_t fan_speed;
    u_int8_t mode; //����ģʽ 00����ģʽ,01����ģʽ,2ͨ��ģʽ,3 ����ģʽ
    u_int8_t onoff; //���ػ�
    //----------------------
    u_int8_t auto_mode; //�Զ�ģʽ״̬ 0�رգ�1ִ���Զ�ģʽ
    int8_t temp_adjust; //�¶Ȳ��� -5��5
    u_int8_t low_temp; //���±����¶� 3-9
    u_int8_t valve_mode; //(�������)����ģʽ���� 1�����ط����ͣ0�����ط��ͣ
    u_int8_t return_temp; //�ز��¶� 1~31
    u_int8_t is_low_temp_guard; //���±���ʹ��
    u_int8_t max_temp; //��������¶� 10-32
    u_int8_t min_temp; //��������¶� 10-32
    //�����������APP������ʱ������,jni���Բ��ϴ�
    u_int8_t lock_screen; //�Ƿ�������
    u_int8_t sesor_type; //�¶ȴ���������
    u_int8_t broad_cast_type; //�㲥����
    u_int8_t no_paid_mode; //Ƿ��ģʽ
    u_int8_t max_sesor_temp; //�ⲿ�����������¶�
    u_int8_t pad;
    //�����Ĳ�����//
    u_int8_t room_temp_integer; //������������
    u_int8_t room_temp_decimal; //����С������
    
    u_int8_t scm_hour; //��Ƭ��ʱ��,ʱ
    u_int8_t scm_min; //��Ƭ��ʱ��,��
    u_int8_t scm_sec; //��Ƭ��ʱ��,��
    u_int8_t scm_weekday; //��Ƭ��ʱ��,����
    cl_smart_smart_ctrl_t smart_info; //����ģʽ��ǰ��Ϣ
}ucp_sbt_scm_info_t;

typedef struct {
    u_int8_t temp; //�¶� 10-32
    u_int8_t fan_speed; //
    u_int8_t mode; //����ģʽ 00����ģʽ��01����ģʽ��2ͨ��ģʽ�� 3 ����ģʽ
    u_int8_t onoff; //���ػ�
}ucp_sbt_scm_stat_t;

typedef struct {
    u_int8_t auto_mode; //�Զ�ģʽ״̬ 0�رգ�1ִ���Զ�ģʽ
    int8_t temp_adjust; //�¶Ȳ��� -5��5
    u_int8_t low_temp; //���±����¶� 3-9
    u_int8_t valve_mode; //(�������)����ģʽ���� 1�����ط����ͣ0�����ط��ͣ
    u_int8_t return_temp; //�ز��¶� 1~31
    u_int8_t is_low_temp_guard; //���±���ʹ��
    u_int8_t max_temp; //��������¶� 10-32
    u_int8_t min_temp; //��������¶� 10-32
}ucp_sbt_scm_func_t;

typedef struct {
    u_int8_t scm_hour; //��Ƭ��ʱ��,ʱ
    u_int8_t scm_min; //��Ƭ��ʱ��,��
    u_int8_t scm_sec; //��Ƭ��ʱ��,��
    u_int8_t scm_weekday; //��Ƭ��ʱ��,����
}ucp_sbt_scm_time_adjust_t;

#pragma pack(pop)

extern bool sbt_ther_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool sbt_ther_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);

extern int sbt_ther_scm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv);

#endif

