#ifndef	__CL_SCENE_H__
#define	__CL_SCENE_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "client_lib.h"

#define MAX_SCENE_NAME_LENGTH (64)
#define MAX_EVENT_NAME_LENGTH (64)
    
//�¼�����
typedef enum _enum_scene_event_type{
    EM_ET_UNKOWN,/*δ֪*/
    EM_ET_EQUIPMENT_KEY, //��������
    EM_ET_DEVICE_FUNC   //�豸����
}cl_scene_event_type_t;

//�¼�����
typedef enum {
    CEA_UNKONWN = 0x0,
    CEA_ON = 0x1,  //������
    CEA_OFF = 0x2  //�ر�
}cl_event_action;
    
//�����¼�����
typedef enum {
    CVE_UNKOWN = 0x0,
    CVE_008_CTRL = 0x1, //008����
    CVE_003_RECORD = 0x2, //003¼��
    CVE_003_MOTION = 0x3, //003�ƶ����
    CVE_ALARM_SWITCH = 0x4 //�����豸����
}cl_vitual_event_type_t;

//�¼����ݽṹ
typedef struct _cl_scene_event_s_{
    cl_scene_event_type_t enent_type; //�¼�����
    cl_vitual_event_type_t ev_type; //�����¼�����,�û����⹦��
    cl_handle_t obj_handle;  //�¼������豸�����
    char event_name[MAX_EVENT_NAME_LENGTH]; //ʱ������
    u_int32_t action_num; //���ٸ�����
    u_int32_t ac_values[1]; //��������,������һ������
}cl_scene_event_t;

// �龰��ʱ��
typedef struct {
	u_int8_t id;			/* ����ID */
	u_int8_t hour;			/* Сʱ 0-23 */
	u_int8_t minute;		/* ���� 0-59 */
	u_int8_t week;			/* bit 0-6λ��Ӧ�����졢����1�������� */
	u_int8_t enable;		/* �Ƿ���Ч(�ֻ�����) �����Ѿ���Ч(�豸����) */
	u_int8_t pad;			/* ���� */
	u_int16_t sort;			/* ��ǰ��ʼ��������ʱ������0��ʾ��ֹ�ģ����������������Ĵ�1��ʼ */
	char *name;		/* ����, UTF-8��ʽ */
} cl_scene_timer_t;

typedef struct _cl_scene_s_{
    cl_handle_t scene_handle;
    u_int8_t img_resv;//�û�˽�б�־�������ڲ�ͬ�ֻ���ʾ��ͬͼƬ
    u_int8_t event_count;//���龰ģʽ�����˶��ٸ��¼�
    u_int8_t last_executed; // ���ִ�е��龰
	u_int8_t timer_num; // �龰��ʱ������
    u_int8_t scene_id;
    u_int32_t create_time;//����ʱ��,��������ĳ̨�ֻ�����ͼƬ
    u_int8_t scene_name[MAX_SCENE_NAME_LENGTH];
	u_int32_t next_time; /* �龰��ʱ����һ��ִ�е�ʱ�� */
	cl_scene_timer_t **timer;
    cl_scene_event_t* events[0];
} cl_scene_t;

// �龰ģʽ event
enum {
    SE_BEGIN = 800,
    SE_SCENE_INFO_HAS_CHANGED = SE_BEGIN+1,

	// �龰
    SE_SCENE_ADD_OK = SE_BEGIN+2,
    SE_SCENE_ADD_FAIL = SE_BEGIN+3,
    SE_SCENE_DEL_OK = SE_BEGIN+4,
    SE_SCENE_DEL_FAIL = SE_BEGIN+5,
    SE_SCENE_CHANGE_OK = SE_BEGIN+6,
    SE_SCENE_CHANGE_FAIL = SE_BEGIN+7,
    SE_SCENE_EXEC_OK = SE_BEGIN+8,
    SE_SCENE_EXEC_FAIL = SE_BEGIN+9,

	// �龰��ʱ��
    SE_SCENE_TIMER_ADD_OK = SE_BEGIN + 10,
    SE_SCENE_TIMER_ADD_FAIL = SE_BEGIN + 11,
    SE_SCENE_TIMER_DEL_OK = SE_BEGIN + 12,
    SE_SCENE_TIMER_DEL_FAIL = SE_BEGIN + 13,
    SE_SCENE_TIMER_MODIFY_OK = SE_BEGIN + 14,
    SE_SCENE_TIMER_MODIFY_FAIL = SE_BEGIN + 15,

    SE_END = SE_BEGIN + 99
};
    
CLIB_API RS cl_scene_add(cl_handle_t user_handle,cl_handle_t* s_handle,u_int8_t img_resv,const char* scene_name);
    
CLIB_API RS cl_scene_del(cl_handle_t s_handle);

CLIB_API RS cl_scene_change_name(cl_handle_t s_handle,const char* name);
    
CLIB_API RS cl_scene_change_img_resv(cl_handle_t s_handle,u_int8_t img_resv);

CLIB_API RS cl_scene_modify_events(cl_handle_t s_handle,u_int16_t event_count,cl_scene_event_t** event);

CLIB_API RS cl_scene_modify(cl_handle_t s_handle, const char* name, u_int8_t img_resv, u_int16_t event_count,cl_scene_event_t** event);

CLIB_API RS cl_scene_add_3(cl_handle_t user_handle, cl_handle_t* s_handle,
				const char* name, u_int8_t img_resv, u_int16_t event_count,cl_scene_event_t** event);
    
CLIB_API RS cl_scene_exec(cl_handle_t s_handle);

CLIB_API RS cl_scene_timer_add(cl_handle_t scene_handle, cl_scene_timer_t *timer);
CLIB_API RS cl_scene_timer_modify(cl_handle_t scene_handle, cl_scene_timer_t *timer);
CLIB_API RS cl_scene_timer_del(cl_handle_t scene_handle, u_int8_t tid);

#ifdef __cplusplus
}
#endif


#endif



