#ifndef	__CL_NOTIFY_PUSH_H__
#define	__CL_NOTIFY_PUSH_H__

#ifdef __cplusplus
extern "C" {
#endif 
 
#include "cl_notify_def.h"
#include "client_lib.h"

    
enum {
    NE_BEGIN = 500,

    NE_NOTIFY,
    
    // ��ѯ������Ϣ���յ��ı�����Ϣ
    // ����cl_get_alarm��ȡ������
    // ʹ����ɺ󣬵���cl_cmt_free_alarm �ͷ��ڴ�
    NE_ALARM_LOG,
    // �յ����������豸���������͵ı���
    // ����cl_get_alarm��ȡ������
    // ʹ����ɺ󣬵���cl_cmt_free_alarm �ͷ��ڴ�
    NE_ALARM_PUSH_LOG,
    //�յ��豸����С���б�
    //����cl_get_community��ȡС���б�
    //�ٸ����ֻ������С��report_id����Ϣ���report_id
    //����cl_notify_query�����ѯС������
    NE_CMT_LIST,

    NE_END = NE_BEGIN + 99
};

/*
     ����:
        1���豸�˺������������ձ�����Ϣ
        2���ֻ��˺������������յ�������Ϣ
     �������:
         @user_handle: �豸���
         @expect: �������յ��豸���к��Լ������յ��������Ϣ���
         @callback: �ص�������ַ
         @handle: �ص���������
     �������:
         ��
     ����:
         RS_OK: �ɹ�
     ����: ʧ��
     ��ע�¼�:NE_NOTIFY��NE_ALARM_LOG��NE_ALARM_PUSH_LOG
     
     ע������:
 */ 
CLIB_API RS cl_set_notify_expect(cl_handle_t user_handle, notify_expect_t* expect,cl_callback_t callback, void *handle);

/*
     ����:
        ��ȡ������Ϣ�б�
     �������:
         @user_handle: �豸���
     �������:
         ��
     ����:
         �ǿ�: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 
CLIB_API alarm_msg_list_t *cl_get_alarm(cl_handle_t user_handle);

/*
     ����:
        �ͷű�����Ϣ�б��ڴ�
     �������:
         @ptr: cl_get_alarm ���صı�����Ϣ�б�
     �������:
         ��
     ����:
         �ǿ�: �ɹ�
     ����: ʧ��
     
     ע������:
 */ 
CLIB_API void cl_free_alarm(alarm_msg_list_t *ptr);

/*
     ����:
         ��ȡһ��������Ϣ
     �������:
         @user_handle: �û����
     �������:
         ��
     ����:
         �ǿ�: �ɹ�
     ����: ʧ��
     
     ע������:
     NE_NOTIFY�¼����ñ�������ȡһ��������Ϣ
     ʹ����ɺ󣬵���cl_free_notify�ͷ��ڴ�
 */ 
CLIB_API notify_msg_t *cl_get_notify(cl_handle_t user_handle);
CLIB_API void cl_free_notify(notify_msg_t *ptr);

/*
     ����:
         ��ȡ����������Ϣ
     �������:
         @user_handle: �û����
     �������:
         ��
     ����:
         �ǿ�: �ɹ�
     ����: ʧ��
     
     ע������:
     NE_NOTIFY�¼����ñ�������ȡ������������Ϣ
     ʹ����ɺ󣬵���cl_free_notify_list�ͷ��ڴ�
 */ 
CLIB_API notify_msg_list_t *cl_get_notify_list(cl_handle_t user_handle);
CLIB_API void cl_free_notify_list(notify_msg_list_t *ptr);

/*
     ����:
        ��ȡ�豸����С���б�
     �������:
         @dev_handle: �豸���
         @cmt:  ���С���б��ڴ�
         @cnt:  ���С�������ڴ�ָ��
     �������:
         @cmt: С���б�����
         @cnt: С������
         ��
     ����:
         RS_OK: �ɹ� 
         ����: ʧ��
     
     ע������:
     ��NE_CMT_LILST�¼�����cl_get_community����
     ����С����max_report_id�뱾�ر���Ľ��бȽ�
     ������صĸ�С������Ҫ����cl_query_cmt_notify�������в�ѯ��ȡ
 */
CLIB_API RS cl_get_community(cl_handle_t dev_handle, cmt_notify_info_t cmt[MAX_CMT], int *cnt);

/*
     ����:
        ��ѯС��������Ϣ
     �������:
         @dev_handle: �豸���
         @query: ��ѯ�����׵�ַ��һ�ο��Բ�ѯ���С��
         @cnt: query���� 
     �������:
         ��
     ����:
         RS_OK : �ɹ�
         ����: ʧ��     
     ע������:
     1��ʹ���ֻ��˺ŵ�¼ʱ�������ж�̨�豸��ÿ̨�豸���Լ���С���б�
            һ�β�ѯ�Ķ��С����������ͬһ̨�豸
     2����ѯ���������NE_NOTIFY�¼�֪ͨ�ϲ�
 */ 
CLIB_API RS cl_query_cmt_notify(cl_handle_t dev_handle, cmt_notify_query_t *query, int cnt);

#ifdef __cplusplus
}
#endif 


#endif



