#ifndef	__CL_SERVER_H__
#define	__CL_SERVER_H__


#ifdef __cplusplus
extern "C" {
#endif 


#include "client_lib.h"

enum{
	UASC_BEGIN = 2200,
	UASC_START_CONNECT = 2201,
	UASC_CONNECT_OK = 2202,
	UASC_PUSH_STAT_OK = 2203
};

enum{
    APP_HARDWARE_TYPE_IPHONE = 0x00,
    APP_HARDWARE_TYPE_IPAD = 0x01,
    APP_HARDWARE_TYPE_IPOD = 0x02,
    APP_HARDWARE_TYPE_SIMULATOR = 0x03,
};
    
typedef struct{
	u_int8_t hard_ware_type; // 0x0:�ֻ�0x1:ƽ��
	char manufacturer_info[64]; //����������Ϣ,���ֻ��������ң��ͺ�
	char os_info[64]; //����ϵͳ�汾��Ϣ
	char app_version_info[32];//app�汾��Ϣ
}cl_app_stat_info_t;

typedef struct{
	u_int8_t is_establish; //�Ƿ�������
	// SDK ������
	u_int8_t cur_stat; 
	u_int32_t server_ip;
	u_int16_t server_port;
}cl_app_server_connect_info_t;

/*******************************************

	module: server
	level: private
		
********************************************/

/*
	���ܣ�
		���÷���������ĵ�ַ��
		�����ñ�������ȱʡ�� www.jiazhang008.com
	����IN��
		server��������IP��ַ���ַ�����Ҳ������������
	���أ�
		RS_OK: �ɹ�
		������ʧ��
*/
CLIB_API RS cl_disp_server_set(const char *server);
/*
	���ܣ�
		��ȡ�����������IP��ַ
	����IN��
		ip: ��ַ������׵�ַ
		count: ip�������
	����OUT��
		ip: ��Ž���������IP��ַ�б�
		count: ����������ip��ַ����
	���أ�
		RS_OK: �ɹ�
		������ʧ��
*/
CLIB_API RS cl_disp_server_get_ip(u_int32_t *ip, u_int32_t *count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/********************************************

Ӧ�÷�������ؽӿں���



*********************************************/
/*
	���ܣ�
		��������APP_������������
	������
		
	���أ�
		RS_OK: �ɹ�
		������ʧ��
*/
CLIB_API RS cl_start_connect_to_app_server();

/*
	���ܣ�
		��ѯAPP ������״̬��Ϣ
	������
		
	���أ�
		RS_OK: �ɹ�
		������ʧ��
*/
CLIB_API RS cl_get_app_server_info(cl_app_server_connect_info_t* info);

/*
	���ܣ�
		����APP ״̬��Ϣ
	������
		
	���أ�
		RS_OK: �ɹ�
		������ʧ��
*/
CLIB_API RS cl_push_app_stat_info(cl_app_stat_info_t* stat);


#ifdef __cplusplus
}
#endif 

#endif

