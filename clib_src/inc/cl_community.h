#ifndef	__CL_COMMUNITY_H__
#define	__CL_COMMUNITY_H__
/*
�ļ���: cl_community.h
�ǻ�С���ӿں���ͷ�ļ�*/

/*
������ģʽ��ָС�����豸���ܹ����ӵ���������ͨ��������Э��С�����豸��ͨ��
������ģʽ��Ҫʹ�����̣�
1����ʼ��SDK�����ú����� cl_init
2��С����¼�����ú����� cl_cmt_login 

3��С�������������״̬����Ҫ��Ҫ���������¼�
	UE_LOGIN_OFFLINE�����ߵ�¼�ɹ���û�����ӵ�����������һ�ε�¼����������Ͽ����ӻ��յ����¼�����
					��һ�ε�¼��Ҫ��ѯ�������ݿ��豸�б�������cl_cmt_add_device���豸��ӵ�SDK��

	UE_LOGIN_ONLINE�����ߵ�¼�ɹ����ɹ����ӵ��������������������ѯ��С��������豸�б����ú�����cl_cmt_query_device_list

	CE_QUERY_DEVICE������������ѯ�豸�б��������ˣ����ú���cl_cmt_get_device_list��ȡ�������ϵ��豸�б�
					�����������б�ͱ������ݿⱣ����豸�б���бȽϣ��Բ��첿�ֽ���ͬ�������ú�����cl_cmt_add_device��cl_cmt_del_device��

	CE_ADD_DEVICE����ӱ�С�������豸����������������ˣ�����cl_cmt_get_add_result��ȡ��ӽ��
	CE_DEL_DEVICE���ӷ�����ɾ����С�������豸��������ˣ�����cl_cmt_get_del_result��ȡɾ�����
	
	CE_NOTIFY_HELLO���յ��豸�������󣬵���cl_cmt_get_notify_hello��ȡhello��Ϣ��
					����С�������յ��豸����һ����Ϣid������cl_cmt_send_notify_hello_ack����ȷ��
					Ȼ������豸hello����expect_report_id�����������ݿ��ѯ��С��������֪ͨ��Ϣ��
					���������cl_cmt_send_notify���豸��������Ϣ���͹�ȥ
	
	CE_NOTIFY_RESULT���յ��豸�Ա�С��������Ϣ��ȷ���¼������Է�����һ����Ϣ�ˡ�

	CE_NOTIFY���յ��豸֪ͨ��Ϣ������cl_cmt_get_notify��ȡ��Ϣ���ݲ�չʾ�����档
	CE_ALARM���յ��豸������Ϣ������cl_cmt_get_alarm��ȡ�������ݲ�չʾ�����档
	

4��С���ǳ������ú�����cl_cmt_logout
5����ֹSDK�����ú�����cl_stop
6��С�������˳�
*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_notify_def.h"
#include "cl_health.h"

/*
CMT_PORT С�������������˿�
�����������£�С����������Ҫ�̶��˿ڣ�
�����豸ֱ�Ӻ�С��������ͨ��
*/
#define CMT_PORT 5362

// community event
enum {
	CE_BEGIN = 300,

	// �յ��豸�����¼�, 
	// �ȵ���cl_cmt_get_notify_hello ��ȡhello����
	// �ٵ���cl_cmt_send_notify_hello_ack ����ȷ��
	// ������hello����expect_report_id�ж��Ƿ����µ���Ϣ��Ҫ����
	CE_NOTIFY_HELLO,
	
	// �յ���Ϣͨ��,
	// Ŀǰ��TNOTIFY_EMERGENCY, TNOTIFY_NORMAL, TNOTIFY_AD
	// Ŀǰֻ��С�����͸��豸	
	// �ȵ���cl_cmt_get_notify ��ȡ֪ͨ��Ϣ
	// �ٵ���cl_cmt_send_notify_result ����ȷ��
	CE_NOTIFY,
	
	// �յ�����
	// ����cl_cmt_get_alarm��ȡ������
	// ʹ����ɺ󣬵���cl_cmt_free_alarm �ͷ��ڴ�
	// Ŀǰֻ���豸���͸�С��
	CE_ALARM,

	// �յ���������
	// Ŀǰֻ���豸���͸�С��
	CE_MESURE,
	
	// ������Ϣ���豸���յ���Ϣȷ���¼�
	// ������ػ����µ���Ϣ������cl_cmt_send_notify����
	CE_NOTIFY_RESULT,
	
	// �յ�����豸����¼�
	// ����cl_cmt_get_add_result ��ȡ��ӽ��
	CE_ADD_DEVICE,	
	
	// �յ�ɾ���豸����¼�
	// ����cl_cmt_get_del_result ��ȡɾ�����
	CE_DEL_DEVICE,	
	
	// �յ���ѯ�豸�б����¼�
	// ����cl_cmt_get_device_list ��ȡ��ѯ���
	CE_QUERY_DEVICE,

	// ����
	CE_ERROR = CE_BEGIN + 50,
	// ��ʱʧ��
	CE_ERR_TIMEOUT,
	CE_END = CE_BEGIN + 99
};

#pragma warning(disable: 4200)

#pragma pack(push,1)

typedef struct{                      /* С����������Ϣ���ݽṹ*/
	u_int64_t sn;                     /* ���к�*/
	u_int32_t service_ip[16];  /* ����ip�б�*/
	u_int16_t service_port;    /* ����˿�*/
	u_int8_t ip_count;            /* ��Ч����ip����*/
	u_int8_t reserved;            /* ����*/	
}cmt_info_t;

typedef struct{         /* �豸��Ϣ���ݽṹ */
	u_int64_t sn;        /* �豸���к�*/
	char phone[128]; /* �豸ӵ���ߵ绰�������'\t'����*/
	char name[64];    /* �豸���ƣ�����: 3��3��Ԫ202*/
}device_info_t;

typedef struct{
	RS result;	       /* �豸�������:RS_OK, RS_NOT_FOUND, RS_XXX*/
	device_info_t info;
}device_status_t;

typedef struct{
	int total;	/* �豸�ܸ���*/
	int count;	/* �����յ����豸����*/
	device_status_t *device; /* �豸����*/
}device_list_t;

typedef struct{      /* ������ϢTLV */
	u_int16_t type;
	u_int16_t len;
	u_int8_t value[0];
}notify_hello_tlv_t;

typedef struct{          /* �����������ݽṹ*/
	u_int64_t dst_sn;  /* ���������к� */
	u_int64_t src_sn;  /* ���������к� */
	u_int8_t versiona;  /*  �����߰汾��Ϣ*/
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int16_t tlv_count;        /* TLV��������*/
	u_int16_t tlv_data_len;   /* TLV�����ܳ���*/
	u_int8_t tlv_data[0];       /* TLV���ݣ�notify_hello_tlv_t��Ŀǰδʹ��*/
}notify_hello_t;

typedef struct{    /* ������Ӧ���ݽṹ*/
	u_int64_t dst_sn;    /* ���������к� */
	u_int64_t src_sn;    /* ���������к� */
	u_int8_t versiona;  /*  �����߰汾��Ϣ*/
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int8_t reserved[3];      /* ����*/
	u_int16_t tlv_count;        /* TLV��������*/
	u_int16_t tlv_data_len;   /* TLV�����ܳ���*/
	u_int8_t tlv_data[0];       /* TLV���ݣ�notify_hello_tlv_t��Ŀǰδʹ��*/
}notify_hello_ack_t;

typedef struct{ /* ����������¼ */
	u_int64_t dev_sn;
	u_int64_t report_id;
	family_t member;		/* �������� */
	measure_t measure;	/* ����ֵ */
}cl_measure_t;

typedef struct{
	int count;
	cl_measure_t **list;
}cl_measure_list_t;

#pragma pack(pop)

/*
	������cl_cmt_login
	���ܣ�
		�ǻ�С����¼

	������
		user_handle: ��¼�ɹ�����Ψһ�û����
		filename: �ǻ�С����Ȩ�ļ���
				ÿ���ǻ�С�����������в�ͬ����Ȩ
		callback: �ص�����, �¼�ΪUE_XXX , CE_XXX
		callback_handle: �ص�ʱ��ش����ص�����
		
	���أ�
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_login(cl_handle_t *user_handle, const char *filename,
						cl_callback_t callback, void *callback_handle);

/*
	������cl_cmt_logout
	���ܣ�
		�ǻ�С���ǳ�

	������
		user_handle: ���Ӿ��
				
	���أ�
		RS_OK , RS_XXX
*/						
CLIB_API RS cl_cmt_logout(cl_handle_t user_handle);

/*
	������cl_cmt_self_info
	���ܣ�
		��ȡ�ǻ�С����Ϣ

	������
		user_handle: ���Ӿ��
		info: �����С����Ϣ

	���أ�
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_self_info(cl_handle_t user_handle, cmt_info_t *info);

/*
	������cl_cmt_query_device_list
	���ܣ�
		����������ѯ��С������İ����豸�б�
		ֻ����С�����ӵ���������(�¼�UE_LOGIN_ONLINE)�����ܷ��ͱ�����
		������Ϊ�첽��ʽ��
		�ڻص�CE_QUERY_DEVICE�¼�ʱ��
		����cl_cmt_get_device_list��ȡʵ���б�
		ʹ����ɺ󣬵���cl_cmt_free_device_list�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
		sn: 0��ʾ��ѯ���е��豸����0��ʾ��ѯ����ĳ̨�豸
				
	���أ�
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_query_device_list(cl_handle_t user_handle, u_int64_t sn);

/*
	������cl_cmt_get_device_list
	���ܣ�
		��ȡ��ѯ��С������İ����豸�б�
		ʹ����ɺ󣬵���cl_cmt_free_device_list�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
						
	���أ�
		�ɹ������豸�б�device_list_t 
		ʧ�ܷ���NULL

*/
CLIB_API device_list_t *cl_cmt_get_device_list(cl_handle_t user_handle);

/*
	������cl_cmt_free_device_list
	���ܣ�
		�ͷ��豸�б��ڴ棬���ڴ���cl_cmt_get_device_list ����
		
	������
		ptr: Ҫ�ͷŵ��豸�б�
						
	���أ�
		��

*/
CLIB_API void cl_cmt_free_device_list(device_list_t *ptr);

/*
	������cl_cmt_add_device
	���ܣ�
		�ǻ�С������豸����С��
		������Ϊ�첽��ʽ��
		�ڻص�CE_ADD_DEVICE �¼�ʱ����cl_cmt_get_add_result��ȡ��ӽ��

	������
		user_handle: ���Ӿ��
		device_info_t: Ҫ��ӵ��豸��Ϣ				
	���أ�
		RS_OK , RS_XXX
		�������RS_OK����ʾ�ɹ���ӵ��ӿڿ⣬������ģʽ���Թ����ˡ�
		��������������ȴ��첽�ص�֪ͨ
*/
CLIB_API RS cl_cmt_add_device(cl_handle_t user_handle, device_info_t *device);

/*
	������cl_cmt_del_device
	���ܣ�
		�ǻ�С��ɾ���豸
		������Ϊ�첽��ʽ��
		�ڻص�CE_DEL_DEVICE ���¼�ʱ����cl_cmt_get_del_result ��ȡɾ�����

	������
		user_handle: ���Ӿ��
		device_info_t: Ҫɾ�����豸��Ϣ				
	���أ�
		RS_OK , RS_XXX
		�������RS_OK, ��ʾ�ӽӿڿ���ɾ���ɹ���
		��������������ȴ��첽�ص�֪ͨ		
*/
CLIB_API RS cl_cmt_del_device(cl_handle_t user_handle, device_info_t *device);

/*
	������cl_cmt_get_add_result
	���ܣ�
		��ȡ����豸���
		�յ��ص��¼�CE_ADD_DEVICEʱ���ñ�������
		ʹ����ɺ󣬵���cl_cmt_free_dev_result�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
		
	���أ�
		�ɹ���������豸���device_status_t
		ʧ�ܷ���NULL
		
*/
CLIB_API device_status_t *cl_cmt_get_add_result(cl_handle_t user_handle);

/*
	������cl_cmt_get_del_result
	���ܣ�
		��ȡɾ���豸���
		�յ��ص��¼�CE_DEL_DEVICEʱ���ñ�������
		ʹ����ɺ󣬵���cl_cmt_free_dev_result�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
		
	���أ�
		�ɹ�����ɾ���豸���device_status_t
		ʧ�ܷ���NULL
		
*/
CLIB_API device_status_t *cl_cmt_get_del_result(cl_handle_t user_handle);

/*
	������cl_cmt_free_dev_result
	���ܣ�
		�ͷ��豸��ӡ�ɾ������ڴ�
		���ڴ���cl_cmt_get_del_result ��cl_cmt_get_del_result ����
		
	������
		ptr: Ҫ�ͷŵ��ڴ�
		
	���أ�
		��
		
*/
CLIB_API void cl_cmt_free_dev_result(device_status_t *ptr);

/*-----------------------------------------------------------------------------------*/
/*
	������cl_cmt_send_notify_expect
	���ܣ�
		֪ͨ����������С�������յ��豸��Ϣid��
		��С�����ӵ����������������(�¼�UE_LOGIN_ONLINE)��
		���Һͷ���������豸�б�ͬ����
		���ñ�����ͷ�����ͬ���豸��Ϣ

	������
		user_handle: ���Ӿ��
		expect: ����ͬ����Ϣ�豸��Ϣ
		count: ���η���ͬ���豸��Ϣ������
			����С��������豸�ɴ���ǧ����
			��Ҫ��η��ͣ�ÿ����෢��100����
				
	���أ�
		�ɹ�����RS_OK������ʧ��

*/
CLIB_API RS cl_cmt_send_notify_expect(cl_handle_t user_handle, notify_expect_t* expect, int count);

/*
	������cl_cmt_get_notify_hello
	���ܣ�
		��ȡnotify hello ��Ϣ
		�ڻص� CE_NOTIFY_HELLO ���¼�ʱ���ñ�������ȡ������Ϣ��������
		ʹ����ɺ󣬵���cl_cmt_free_notify_hello�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
				
	���أ�
		��Ϣ���� notify_hello_t

*/
CLIB_API notify_hello_t *cl_cmt_get_notify_hello(cl_handle_t user_handle);

/*
	������cl_cmt_free_notify_hello
	���ܣ�
		�ͷ��ڴ棬���ڴ���cl_cmt_get_notify_hello����
	������	
		ptr: Ҫ�ͷŵ��ڴ�
		
	���أ�
		��

*/
CLIB_API void cl_cmt_free_notify_hello(notify_hello_t *ptr);

/*
	������cl_cmt_send_notify_hello_ack
	���ܣ�
		����notify hello ack ��Ϣ
		�ڻص� CE_NOTIFY_HELLO ���¼�ʱ���ñ���������ack
		
	������
		user_handle: ���Ӿ��
		ack: notify hello ack��Ϣ����
				
	���أ�
		RS_OK, RS_XXX
*/
CLIB_API RS cl_cmt_send_notify_hello_ack(cl_handle_t user_handle, notify_hello_ack_t *ack);

/*
	������cl_cmt_get_alarm
	���ܣ�
		��ȡ������Ϣ
		�ڻص� CE_ALARM ���¼�ʱ���ñ�������ȡ������Ϣ��������
		ʹ����ɺ󣬵���cl_cmt_free_alarm�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
				
	���أ�
		�������� alarm_msg_list_t

*/
CLIB_API alarm_msg_list_t *cl_cmt_get_alarm(cl_handle_t user_handle);
CLIB_API void cl_cmt_free_alarm(alarm_msg_list_t *ptr);

/*
	������cl_cmt_get_notify
	���ܣ�
		��ȡ������Ϣ
		�ڻص� CE_NOTIFY ���¼�ʱ���ñ�������ȡ������Ϣ��������
		ʹ����ɺ󣬵���cl_cmt_free_notify�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
				
	���أ�
		��Ϣ���� notify_msg_t

*/
CLIB_API notify_msg_t *cl_cmt_get_notify(cl_handle_t user_handle);

/*
	������cl_cmt_free_notify
	���ܣ�
		�ͷ��ڴ棬���ڴ���cl_cmt_get_notify����
	����IN��	
		ptr: Ҫ�ͷŵ��ڴ�
		
	���أ�
		��

*/
CLIB_API void cl_cmt_free_notify(notify_msg_t *ptr);


/*
	������cl_cmd_send_notify_new
	���ܣ�
		�ǻ�С���·������ӹ���
		������Ϊ�첽��ʽ
		�ص� CE_NOTIFY_RESULT �¼�ʱȷ��ĳ̨�豸�Ѿ��յ�
		ע�⣺Ŀǰ���豸ͨ��ֻ֧��UDPЭ�飬һ������ܷ���Լ8KB���ݣ�
			����ѵ��ӹ�����Ҫ���ݷ��ڳ��������档
	���� IN��	
		user_handle: ���Ӿ��
		msg: ���ӹ������ݣ�dev_sn �ֶ�Ϊ0��ʾ�㲥���͸������豸
				dev_sn Ϊ����ĳ̨�豸���кţ���ֻ���͸����豸
	����OUT��
		send_count: ������巢���˶���̨�豸(����������)

	���أ�
		RS_OK , RS_XXX

*/
CLIB_API RS cl_cmt_send_notify_new(cl_handle_t user_handle, notify_msg_t *msg, int *send_count);

/*
	������cl_cmd_send_notify_old
	���ܣ�
		�ǻ�С���յ�ͬ�����󣬰���ʷ���淢�͹�ȥ
		������Ϊ�첽��ʽ
		�ص� CE_NOTIFY_RESULT �¼�ʱȷ��ĳ̨�豸�Ѿ��յ�
		ע�⣺Ŀǰ���豸ͨ��ֻ֧��UDPЭ�飬һ������ܷ���Լ8KB���ݣ�
			����ѵ��ӹ�����Ҫ���ݷ��ڳ��������档
	���� IN��	
		user_handle: ���Ӿ��
		msg: ���ӹ������ݣ�dev_sn �ֶ�Ϊ0��ʾ�㲥���͸������豸
				dev_sn Ϊ����ĳ̨�豸���кţ���ֻ���͸����豸
		peer_sn: ����Զ����豸��peer_snΪ�豸���кţ�0��ʾ�Զ��Ƿ�����
	����OUT��
		send_count: ������巢���˶��ٷݣ�ͬ����ʷ����Ϊ�㵽�㣬Ӧ��ֻ����һ��

	���أ�
		RS_OK , RS_XXX
*/
CLIB_API RS cl_cmt_send_notify_old(cl_handle_t user_handle, notify_msg_t *msg, int *send_count, u_int64_t peer_sn);

/*-----------------------------------------------------------------------------------*/
/*
	������cl_cmt_get_notify_result
	���ܣ�
		��ȡ��Ϣȷ��
		�ص� CE_NOTIFY_RESULT ���¼�ʱ���ñ�����		
		ʹ����ɺ󣬵���cl_cmt_free_notify_reslut�ͷ��ڴ�
	����IN��	
		user_handle: ���Ӿ��
		

	���أ�
		��Ϣȷ������notify_msg_result_t

*/
CLIB_API notify_msg_result_t *cl_cmt_get_notify_result(cl_handle_t user_handle);

/*
	������cl_cmt_free_notify_reslut
	���ܣ�
		�ͷ��ڴ棬���ڴ���cl_cmt_get_notify_result ����
		
	������	
		ptr: Ҫ�ͷŵ��ڴ�
		
	���أ�
		��

*/
CLIB_API void cl_cmt_free_notify_reslut(notify_msg_result_t *ptr);

/*
	������cl_cmt_get_measure
	���ܣ�
		��ȡ����������¼
		�ڻص� CE_MESURE ���¼�ʱ���ñ�������ȡ����������¼
		ʹ����ɺ󣬵���cl_cmt_free_measure�ͷ��ڴ�

	������
		user_handle: ���Ӿ��
				
	���أ�
		����������¼ cl_measure_list_t

*/
CLIB_API cl_measure_list_t *cl_cmt_get_measure(cl_handle_t user_handle);
CLIB_API void cl_cmt_free_measure(cl_measure_list_t *list);

#ifdef __cplusplus
}
#endif 


#endif



