#ifndef	__CL_RC_H__
#define	__CL_RC_H__

#ifdef __cplusplus
extern "C" {
#endif 

    //��������Ϣ
#define MAX_RC_NAME_LEN 16
    
//���Ӻͻ�����ң�������
#define RC_TYPE_TV 0x1
#define RC_TYPE_STB 0x2

/* ���Ӱ������� */
/* ���ػ�   - 0 */
#define TV_KEY_ONOFF (0)
/* ����     - 1  */
/* ����     - 2  */
/* ��ʽ     - 3  */
/* ˯��     - 4  */
/* 1        - 5  */
/* 2        - 6  */
/* 3        - 7  */
/* 4        - 8  */
/* 5        - 9  */
/* 6        - 10 */
/* 7        - 11 */
/* 8        - 12 */
/* 9        - 13 */
/* --/-     - 14 */
/* 0        - 15 */
/* ��Ŀ���� - 16 */
/* ����     - 17 */
/* ���л�   - 18 */
/* ����     - 19 */
/* ѡ̨     - 20 */
/* ͼ��     - 21 */
/* CH-      - 22 */
/* CH+      - 23 */
/* ����     - 24 */
/* ����     - 25 */
#define TV_KEY_UP (25)
/* ����     - 26 */
#define TV_KEY_DOWN (26)
/* ����     - 27 */
#define TV_KEY_LEFT (27)
/* ����     - 28 */
#define TV_KEY_RIGHT (28)
/* �˵�     - 29 */
/* ����     - 30 */
/* TV/AV    - 31 */
#define TV_KEY_TVAV (31)
/* ���     - 32 */
/* ������   - 33 */
#define TV_KEY_VOLUP (33)
/* ������   - 34 */
#define TV_KEY_VOLDOWN (34)
/* Ƶ����   - 35 */
#define TV_KEY_CHUP   (35)
/* Ƶ����   - 36 */
#define TV_KEY_CHDOWN (36)
/* ����     - 37 */
#define TV_KEY_MUTE   (37)


/* �����а������� */
/* 1               - 0 */
/* 2               - 1 */
/* 3               - 2 */
/* 4               - 3 */
/* 5               - 4 */
/* 6               - 5 */
/* 7               - 6 */
/* 8               - 7 */
/* 9               - 8 */
/* 0               - 9 */
/* �б�            - 10 */
/* ��һ��Ŀ        - 11 */
/* ����            - 12 */
#define STV_KEY_ONOFF (12)
/* Ƶ��+           - 13 */
#define STV_KEY_CHUP  (13)
/* Ƶ��-           - 14 */
#define STV_KEY_CHDOWN (14)
/* ����+           - 15 */
#define STV_KEY_VOLUP  (15)
/* ����-           - 16 */
#define STV_KEY_VOLDOWN (16)
/* ��              - 17 */
#define STV_KEY_UP     (17)
/* ��              - 18 */
#define STV_KEY_DOWN   (18)
/* ��              - 19 */
#define STV_KEY_LEFT   (19)
/* ��              - 20 */
#define STV_KEY_RIGHT  (20)
/* ȷ��            - 21 */
#define STV_KEY_OK     (21)
/* �˳�            - 22 */
/* �˵�            - 23 */
#define STV_KEY_MENU  (23)
/* ��              - 24 */
/* ��              - 25 */
/* ��              - 26 */
/* ��              - 27 */
/* ����            - 28 */
#define STV_KEY_BACK  (28)
/* ��ҳ            - 29 */
#define STV_KEY_PAGEUP (29)
/* ��ҳ            - 30 */
#define STV_KEY_PAGEDOWN (30)
/* ����            - 31 */
/* ��Ϣ            - 32 */
/* ����            - 33 */
/* ϲ��            - 34 */
/* ����            - 35 */
/* ����            - 36 */
/* �㲥            - 37 */
/* ��Ѷ            - 38 */
/* ��Ʊ            - 39 */
/* �㲥            - 40 */
/* �ʼ�            - 41 */
/* ��Ϸ            - 42 */
/* �б�            - 43 */
/* ��һ��Ŀ        - 44 */
/* �趨            - 45 */
/* ��ҳ            - 46 */
#define STV_KEY_HOME   (46)
/* ��¼��Record�� - 47 */
/* ֹͣ��           - 48 */
/* A               - 49 */
/* B               - 50 */
/* C               - 51 */
/* D               - 52 */
/* E               - 53 */
/* F               - 54 */
/* ����            - 55 */
/* ���            - 56 */
/* ����/��ͣ       - 57 */
/* ����            - 58 */
/* ����            - 59 */
/* ����            - 60 */
/* ����            - 61 */
/* ����            - 62 */
/* ����            - 63 */
    
    
typedef struct {
    u_int8_t type; //TV or STB
    u_int8_t rc_id; //ң����id
    u_int8_t action; // ��ǰ״̬ AIR_CODE_XX
    u_int8_t is_cloud_matching; // �Ƿ�������ƥ���У�ȫƥ����ֶ���Ч
    u_int8_t cur_step; // ��ǰ���е��ڼ���
    u_int8_t max_step; //�ܹ����ٲ�
    u_int8_t error; // ƥ����ִ���
    u_int8_t flag; //ƥ��ͬ�������,ע:ֻ��ƥ���������Ч��
    ////////////////////////////////////////////////
    u_int8_t recommon_key_id; // �Ƽ����´ΰ���id
}cl_rc_match_stat_t;

#define MAX_CODE_LEN 256

typedef struct {
    u_int8_t isLearn; //�Ƿ�����ѧϰ��
    u_int8_t rc_id; // ң����id
    u_int8_t type; //TV or STB
    u_int8_t key_id; // ��ǰѧϰ�� ����id
    u_int8_t code_len; // ���볤��
    u_int8_t code[MAX_CODE_LEN]; //ѧϰ���ı���
}cl_rc_key_learn_stat_t;

typedef struct {
    u_int8_t key_id; //����id
    u_int8_t has_code; //�Ƿ��б���
}cl_rc_fixed_key_info;

typedef struct {
    u_int8_t key_id; //����ID
    u_int8_t has_code; //�Ƿ����б���
    u_int8_t pad[2];
    u_int8_t name[MAX_RC_NAME_LEN]; //��������
}cl_rc_user_key_info;

typedef struct {
    u_int8_t d_id; //ң�ذ� id��Ҳ��rc_id
    u_int8_t is_matched; //�Ƿ�ƥ��
    u_int8_t fixed_key_num; //�̶���������
    u_int8_t user_def_key_num; //�û����尴������
    u_int16_t matched_ir_id; //��ƥ�������ID
    u_int8_t name[MAX_RC_NAME_LEN];
    cl_rc_fixed_key_info * fk;
    cl_rc_user_key_info * uk;
}cl_rc_info;

//���ӡ�������ң������ϣ���֧��һ����ϵ�������ṹ
typedef struct {
    u_int8_t name[MAX_RC_NAME_LEN];
    cl_rc_info tv_info;// ����ң�ذ���Ϣ
    cl_rc_info stb_info; //������ң�ذ���Ϣ
}cl_pair_rc_info;
    
typedef struct {
    u_int8_t is_query_key_info;
    u_int8_t pad[3];
    cl_pair_rc_info pair_rc;
    cl_rc_match_stat_t match_stat;
    cl_rc_key_learn_stat_t learn_info;
}priv_rc_manage_info;


#ifdef __cplusplus
}
#endif 

/*
����:
    �޸�ң�����������
�������:
    @key
�������:
    ��
����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_change_name(cl_handle_t dev_handle, u_int8_t* name);

/*
 ����:
    ��ʼƥ����룬�����ɹ��󣬸����¼�����
 �������:
    @rc_id ���ӻ������id
    @time_out ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_start_match(cl_handle_t dev_handle,u_int8_t rc_id,u_int16_t timeout);

/*
 ����:
    ���ƥ��ĵڶ���ƥ��
 �������:
    @rc_id ���ӻ������id
    @time_out ��λ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_rc_start_next_key_match(cl_handle_t dev_handle,u_int8_t rc_id,u_int16_t timeout,u_int8_t key_id);

/*
 ����:
    ֹͣƥ�����
 �������:
    @rc_id ���ӻ������id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_stop_match(cl_handle_t dev_handle,u_int8_t rc_id);

/*
 ����:
    ��ȡ����ƥ��״̬
 �������:
    @key
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */

CLIB_API RS cl_rc_get_match_stat(cl_handle_t dev_handle,cl_rc_match_stat_t* stat);

/*
 ����:
    ��ȡ����ѧϰ����״̬
 �������:
    @key
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_get_learn_stat(cl_handle_t dev_handle,cl_rc_key_learn_stat_t* stat);

/*
 ����:
    ���Ϳ���ָ��
 �������:
    @rc_id ���ӻ������id
    @key_id ����id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_ctrl_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 ����:
    ���ٷ��͵��Ӻͻ����п��ؿ���ָ��
    �����û�һ���������رյ��Ӻͻ�����
 �������:
    @key
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_quick_ctrl_onoff(cl_handle_t dev_handle,u_int8_t tv_rc_id,u_int8_t stb_rc_id);

/*
 ����:
    ��ʼѧϰ����
 �������:
    @rc_id ���ӻ������id
    @key_id ����ID
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_start_learn(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 ����:
    ֹͣѧϰ����
 �������:
    @rc_id ���ӻ������id
    @key_id ����ID
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_stop_learn(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 ����:
    ɾ������
 �������:
    @rc_id ���ӻ������id
    @key_id ����id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_delete_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 ����:
    ���/�޸İ���
 �������:
    @rc_id ���ӻ������id
    @key_id ����id
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_rc_modify_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id,char* name);


#endif

