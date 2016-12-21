#ifndef	__EQUIPMENT_PRIV_H__
#define	__EQUIPMENT_PRIV_H__

#include "cl_equipment.h"

#ifdef __cplusplus
extern "C" {
#endif 

#pragma pack(push, 1)

// �����������net_proto.h�У�����vc��������0�����б���0���飬��
typedef struct{
	u_int32_t err;
	u_int8_t action; // 0:��ѯ 1: ��� 2: �޸� 3:ɾ��
	u_int8_t version;
	u_int16_t count;	// remote_attri_t �ṹ�����
	//remote_atrri_t ctrldev[0];	// remote_attri_t �ṹ��
}net_remote_t;

#pragma pack(pop)

enum  {
    AC_KEYCODE_LEARN = 0x0, /*��ʼѧϰ*/
    AC_KEYCODE_ADD,  /*��ӱ���*/
    AC_KEYCODE_DELETE, /*ɾ������*/
    AC_KEYCODE_MODIFY, /*�޸ı���*/
    AC_KEYCODE_TRY,    /*����*/
    AC_KEYCODE_STUDYOK,  /*ѧϰ�ɹ�,�豸���ͱ���*/
    AC_KEYCODE_STUDYSTOP, /*ֹͣѧϰ*/
    AC_KEYCODE_GEN_CODE,       /*��������*/
    AC_KEYCODE_PROBE,     /*̽���Ƿ����΢��*/
    AC_KEYCODE_ADJUST,    /*΢��*/
    AC_KEYCODE_PLUS_WIDTH_AJUST,  /*�������*/
};
    
enum  { /*REMOTE_TYPE_DIY�Զ�������������Ͷ���*/
    RDEVSUBTYPE_CUSTOM = 0,  /*�Զ���RF����*/
    RDEVSUBTYPE_RF_TO_INFR_TV = 0x1, /*����ת�����Ƶ���*/
    RDEVSUBTYPE_RF_TO_INFR_TVBOX = 0x2,/*����ת�����ƻ�����*/
    RDEVSUBTYPE_RF_TO_INFR_AIRCONDITION = 0x3,/*����ת�����ƿյ�*/
    RDEVSUBTYPE_RF_TO_INFR_CUSTOM =0x4/*��ת�Զ���*/
};

#define ALARM_PHONE_LENGTH (16)

#define KEYID_ALARM_MSG_MASK (0x40000000)
#define KEYID_ALARM_KEY ((0x40000000)|0x1)
    
#define STATEID_OUTLET_GROUPNUM (16)/*�ж���·*/
#define STATEID_OUTLET_FUCTION  (15)/*�������ػ��Ƿ�ת*/
    
#define STATEID_CURTAIN_TYPE    (16)/*��Ҷ����������������*/
#define STATEID_CURTAIN_STATE   (15)/*��ǰ״̬*/

#define STATEID_DB_RF_CURTAIN_TIME (15) /*˫����ȫ����ȫ��ʱ��*/
#define STATEID_DB_RF_CURTAIN_POSTION (12) /*˫������ǰλ��*/
#define STSTEID_DB_RF_CTRL (11) /*˫��ƿ��ƿ����ǹ�*/
    
#define STATEID_LIGHT_GROUPNUM  (16)/*�ж���·*/
#define STATEID_LIGHT_FUCTION    (15)/*�������ػ��Ƿ�ת*/
#define STATEID_LIGHT_STATE     (12)/*�Ƶ�ǰ״̬*/
#define STATEID_RF_REPEATER    (17)/*RF����*/
#define STATEID_DB_DIMMING      (14)/*�����*/

#define STATEID_FUNC_AJUST_LIGHT     BIT(0)  /*��1��ʾ����*/
#define STATEID_FUNC_BTN_ONOFF_CTRL BIT(1)  /*��1��������*/

#define IS_KEY_STATE_ON(keyid, state)  ((BIT((keyid))>>1) & (state))

enum{//action of double rf equipment
	AC_RF2_BIND = 1,
	AC_RF2_UNBIND = 2,
	AC_RF2_REPEATER_ON = 3,
	AC_RF2_REPEATER_OFF = 4,
};

typedef struct {
    struct stlc_list_head link;
    cl_handle_t handle;
}handle_backup_t;

typedef struct _equipment_ctrl_s {
	// ��ָָ��
	slave_t *slave;
	//��ƥ�����ѯ
	cl_thread_t *t_cm_query;
	// ��ѯ��ʱ��
	cl_thread_t *t_query;

	int prev_reply_len;
	// ������һ�β�ѯ�Ľ���������жϱ��β�ѯ����Ƿ�ı�
	void *prev_reply;

	// ��������, equipment_t
	struct stlc_list_head eq_list;
    
    struct stlc_list_head eq_new_eq_hand_list;

} equipment_ctrl_t;

typedef struct {
	struct stlc_list_head link;
	cl_handle_t handle;
    equipment_ctrl_t* ec;
    
    /*�����б�*/
    struct stlc_list_head keylist;
    cl_thread_t *t_key_query;
	 /* ��ѯ�绰��Ϣ�͹��������ⱨ���� */
    cl_thread_t *t_alarm_query;
    u_int32_t nbp_len;
    net_alarm_bind_phone_t* nbp;

	// �ñ��������������ⱨ����
	u_int32_t soundlight_len;
	// ������������棬list���ֽ���δת�����������Ѿ�ת������
    net_remote_config_soundlight *soundlight;

    int prev_reply_len;
	// ������һ�ΰ�����ѯ�Ľ���������жϱ��β�ѯ����Ƿ�ı�
	void *prev_reply;
	u_int8_t match_id_num;	// �ƿյ�ƥ�䵽���ٸ����룬���4
	u_int16_t match_id[4];	//���4��ƥ�䵽��ID��
	remote_atrri_v2_t attr;
} equipment_t;
    
typedef struct {
    struct stlc_list_head link;
    /*��ָ����ָ��*/
    equipment_t *eq;
    remote_key_attri_t key_attr;
}remote_key_t;

/*���ð����ͱ���ѧϰ*/
typedef struct remote_key_learn_s{
    u_int32_t key_id; /*ѧϰ��key���*/
    cl_handle_t eq_handle; /*�������*/
    u_int8_t learn_mode; /*ѧϰģʽ���Ƿ��Ƕ���*/
    u_int8_t learn_stat; /*��ǰѧϰ״̬*/
    u_int8_t learn_remain_time; /*����ʱʣ��ʱ��*/
    u_int8_t is_support_plw; /*�Ƿ�֧���������*/
    u_int8_t is_support_ajust; /*�Ƿ�֧���ź�΢��*/
    int last_err;
    u_int16_t ajust_pw_max;
    u_int16_t ajust_current_value;
    
    cl_callback_t callback;  /*�ص�����*/
	void *callback_handle; /*�ص�����*/
    
    u_int16_t learn_code_len; /*ѧϰ���ı��볤��*/
    u_int16_t learn_code_type; /*ѧϰ���ı�������*/
    u_int8_t* learn_code; /*ѧϰ���ı���*/
    
    u_int16_t host_gen_code_len; /*���������ı��볤��*/
    u_int16_t host_gen_code_type; /*���������ı�������*/
    u_int8_t* host_gen_code;  /*���������ı���*/
    
    u_int16_t ajust_code_len;  /*΢�������ı��볤��*/
    u_int16_t ajust_code_type; /*΢�������ı�������*/
    u_int8_t* ajust_code;      /*΢�������ı���*/
    int ajust_range;     /*΢����Χ*/
    
    u_int16_t base_code_len;  /*΢��ʱ�õĻ�׼���볤��*/
    u_int16_t base_code_type; /*΢��ʱ�õĻ�׼��������*/
    u_int8_t* base_code;      /*΢��ʱ�õĻ�׼����*/
    
    cl_thread_t *t_learn;
}remote_key_learn_t;
    
typedef struct _alarm_phone_assist_s{
    user_t* user; //��ָָ��
    cl_thread_t* t_phone_query; /*��ѯphone�б�*/
    u_int32_t nacp_len;
    net_alarm_config_phone_t* nacp; /*phone�б�*/
}alarm_phone_assist_t;

bool isAlarmDevice(equipment_t* eq);
// ����; BOOL: �����˸ñ���. false: ��Ҫ����ģ�����������
extern bool eq_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool eq_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern RS eq_alloc(slave_t *slave);
extern void eq_free(slave_t *slave);
extern void eq_free_objs(cl_equipment_t *eq);
extern void eq_build_objs(cl_dev_info_t *ui, slave_t *slave, int *idx_eq);
extern RS key_learn_alloc(user_t* user);
extern void key_learn_free(user_t* user);
extern RS alarm_phone_alloc(user_t* user);
extern void alarm_phone_free(user_t* user);
extern equipment_t *eq_lookup(equipment_ctrl_t *ec, u_int16_t id);
extern equipment_t *eq_lookup_by_handle(equipment_ctrl_t *ec, cl_handle_t eq_handle);
extern remote_key_t *rk_lookup(equipment_t *eq, u_int32_t id);
extern void eq_force_update_all_info(equipment_ctrl_t *ec);



#ifdef __cplusplus
}
#endif 

#endif

