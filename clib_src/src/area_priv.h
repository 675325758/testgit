#ifndef	__AREA_PRIV_H__
#define	__AREA_PRIV_H__

#include "cl_area.h"
#include "cl_priv.h"

enum  {
    AC_AREA_QUERY_ALL = 0x0, //��ѯ��������
    AC_AREA_MODIFY = 0x1, //������ӻ��޸�
    AC_AREA_DELETE = 0x2, //����ɾ��
    AC_AREA_QUERY_SINGLE =0x3, //��ѯ��������
    AC_AREA_MODIFY_SINGLE = 0x4
};

typedef struct{
    struct stlc_list_head link;//��������
    cl_handle_t area_handle;
    u_int32_t create_time;
}area_req_store_t;

typedef struct _area_ctrl_s {
	// ��ָָ��
	user_t *user;
	// ��ѯ��ʱ��
	cl_thread_t *t_query;
    
    u_int32_t query_time_interval;
    
	int prev_reply_len;
	// ������һ�β�ѯ�Ľ���������жϱ��β�ѯ����Ƿ�ı�
	void *prev_reply;
    
	// ��������
	struct stlc_list_head area_list;
    //�����������
    struct stlc_list_head area_req_list;
} area_ctrl_t;

typedef struct {
	struct stlc_list_head link;//��������
    area_ctrl_t * ac; /*��ָָ��*/
    u_int32_t query_time_interval; //��ѯʱ����
    area_t area_info;
    cl_handle_t area_handle;//������
    u_int8_t area_id;      //����ID
    u_int16_t dev_obj_num;
    u_int16_t eq_obj_num;
    int prev_reply_len;    //�ϴ������ѯ��Ϣ����
	void *prev_reply;      //�ϴβ�ѯ��Ϣ��Ӧ
    cl_thread_t *t_query;  //���������ѯ��ʱ��
} area_priv_t;

extern bool area_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool area_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern RS area_ctrl_alloc(user_t* user);//�����ڴ�
extern void area_ctrl_free(user_t* user);//�ͷ��ڴ�
extern void area_build_objs(user_t* user,cl_dev_info_t* ui);//��ѯ
extern cl_handle_t area_get_handle_by_id(user_t* user,u_int8_t area_id);

#endif

