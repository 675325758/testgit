#ifndef	__SCENE_PRIV_H__
#define	__SCENE_PRIV_H__

#include "cl_scene.h"
#include "cl_priv.h"


enum  {
    AC_SCENE_QUERY_ALL = 0x0,
    AC_SCENE_MODIFY = 0x1,
    AC_SCENE_DELETE = 0x2,
    AC_SCENE_QUERY_SINGLE= 0x3,
    AC_SCENE_EXEC = 0x9
};

#define SCENE_DEV_ACTION_ON     (0x0)
#define SCENE_DEV_ACTION_OFF    (0x1)

typedef struct{
    struct stlc_list_head link;//��������
    cl_handle_t scene_handle;
    u_int32_t create_time;
}scene_req_store_t;

typedef struct _scent_ctrl_s {
    user_t *user;
	// ��ѯ��ʱ��
	cl_thread_t *t_query;
    
	int prev_reply_len;
	// ������һ�β�ѯ�Ľ���������жϱ��β�ѯ����Ƿ�ı�
	void *prev_reply;
    
    u_int32_t query_time_interval;
	// �龰ģʽ����
	struct stlc_list_head scene_list;
    
    struct stlc_list_head scene_req_list;
} scene_ctrl_t;

// scene_t->flags
#define FLAG_SCENE_LAST_ID			0x1
#define FLAG_SCENE_EXECUTED			0x2
#define FLAG_SCENE_HAVE_EVENTS		0x4

typedef struct {
	struct stlc_list_head link;//��������
    
    cl_handle_t scene_handle;//�龰ģʽ���
    scene_t scene_info;
    int prev_reply_len;    //�ϴβ�ѯ��Ϣ����
	void *prev_reply;      //�ϴβ�ѯ��Ϣ��Ӧ

	int prev_timer_len; /* ������һ�β�ѯ���龰��ʱ�� */
	void *prev_timer;
	
    cl_thread_t *t_query;  //�����龰ģʽ��ѯ��ʱ��
    scene_ctrl_t * sc; /*��ָָ��*/
    u_int32_t query_time_interval; //��ѯʱ����
    u_int8_t event_count;
    u_int8_t scene_id;      //ID
} scene_priv_t;

extern bool scene_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool scene_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern RS scene_ctrl_alloc(user_t* user);//�����ڴ�
extern void scene_ctrl_free(user_t* user);//�ͷ��ڴ�
extern void scene_build_objs(user_t* user,cl_dev_info_t* ui);//��ѯ
extern void scene_free_timer(cl_scene_t* scene);
extern void quick_query_alarm_link_scene(user_t *user);
extern scene_priv_t* scene_lookup_by_id(scene_ctrl_t* sc,u_int8_t scene_id);
RS scene_proc_exec(cl_handle_t s_handle);

#endif

