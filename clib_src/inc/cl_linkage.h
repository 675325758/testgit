/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_linkage.h
**  File:    cl_linkage.h
**  Author:  liubenlong
**  Date:    12/03/2015
**
**  Purpose:
**    �����ⲿͷ�ļ�.
**************************************************************************/


#ifndef CL_LINKAGE_H
#define CL_LINKAGE_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_user.h"
/* Macro constant definitions. */
#define LA_USER_MAX		10
#define LA_HOME_MAX		20
#define LA_MEM_MAX		30

/* Type definitions. */
//�����
enum {
	LA_ERR_OTHER_HOME = 0,//�豸���ʧ��ԭ��������Ȧ����
	LA_PHONE_ACOUNT_EXIST = 1,//�ֻ��˺��Ѿ����ڣ����ܴ�����
	LA_PHONE_ALL_SESSION_OFFLINE = 2,//����session�������ߵģ����ܴ���
	LA_PHOME_LOGIN_FAILED_LOGINED = 3,//���ֻ��˺��Ѿ���½��
	LA_PHONE_NOT_EXIST = 4,//���ֻ��˺Ų�����
	LA_PHONE_CREATE_FAILED_EXIST = 5,//�����������и��˺�
	LA_PHONE_REPLACE_FAILED = 6,//�������滻ǿ�û�ʧ��
	LA_PHONE_DEFAULT_HOME_SERVER_OFFLINE = 7,//Ĭ��Ȧ�ӷ�����������
	LA_PHONE_LOGIN_PASSWD_ERR = 8,//��½�������
	LA_HOME_DEL_FAILED_DEF = 9,//ɾ��Ȧ��ʧ�ܣ���Ϊ��Ĭ��Ȧ�ӣ�����ɾ��
	LA_HOME_REG_SESSION_IS_OFFLINE = 10,//ע��ʧ�ܣ�����������
	LA_NET_TIMEOUT = 11,	// ��Ϊ��������³�ʱ(����·��������)

	LA_APP_USER_REQ_SHARE_NO_USERID = 12, //pad�����ά���������,û��userid
	LA_APP_USER_REQ_SHARE_USER_ID_ALREAD = 13,//pad�����ά����󣬲����Լ������Լ�ע��
	LA_APP_USER_REQ_SHARE_PARSE_ERROR = 14,//pad�����ά���������
	LA_APP_USER_REQ_SHARE_HOME_NOT_EXIST = 15,//pad�����ά��ע��ʱ���Ҳ���Ҫע��ļ�ͥ
	LA_APP_USER_REQ_SHRAE_HOME_EXIST = 16,//pad�����ά��ע��ʱ����������Ӧ���û��Ѿ��иü�ͥ��

	//��֧�ִ���
	LA_NOT_SUPPORT = 17,
	
	LA_APP_VER_IS_TOO_SMALL = 99,//APP�汾���ͣ����Ӳ��Ϸ�����
};

enum {
	LA_BEGIN = 2100,
	//�����û�Ȧ�ӳ�Ա��Ϣһ���ȡ�䶯�¼�,���ʱ��͸õ���cl_la_info_get������ȡ�ܵ�Ȧ���б���Ϣ
	LA_USER_CHANGED = LA_BEGIN + 1,
	//����Ȧ�ӱ仯 
	LA_HOME_CHANGED = LA_BEGIN + 2,
	//Ȧ��ɾ��ʧ��
	LA_HOME_DEL_FAILED = LA_BEGIN + 3,
	//Ȧ������豸����
	LA_HOME_ADDDEV_FAILED = LA_BEGIN + 4,
	//Ȧ���豸�б��б䶯��������push������homehandle����ѯȦ���豸,�������cl_la_home_info_get����
	LA_HOME_ADDDEV_CHANGED = LA_BEGIN + 5,
	//Ȧ���豸ɾ��ʧ��
	LA_HOME_REMOVEDEV_FAILED = LA_BEGIN + 6,
	//����ע��ɹ�
	LA_HOME_SHARE_REGISTER_SUCCESSED = LA_BEGIN + 7,//�¼�err_no���ľ���home_id
	//���������޸�ʧ��
	LA_HOME_SHARE_DESC_CHANGE_FAILED = LA_BEGIN + 8,
	//����ɾ��ʧ��
	LA_HOME_SHARE_DESC_DELETE_FAILED = LA_BEGIN + 9,
	//���Ȧ��ʧ��
	LA_HOME_CREATE_FAILED = LA_BEGIN + 10,
	//��Ȧ���Ѿ�����ע�����
	LA_HOME_SHARE_REGISTER_ALREADY = LA_BEGIN + 11,
	//����ִ��ʧ��
	LA_LINK_CONF_EXEC_FAILED = LA_BEGIN + 12,
	//���������ɹ�
	LA_SHARE_REQ_SUCCESSED = LA_BEGIN + 13,
	//Ȧ�Ӽ��豸�ƶ�ʧ��
	LA_HOME_CONF_DEV_MOVE_FAILED = LA_BEGIN + 14,
	//����ִ�гɹ�
	LA_LINK_CONF_EXEC_SUCCESSED = LA_BEGIN + 15,
	//Ȧ�������޸�ʧ��
	LA_HOME_CONF_NAME_MODIFY_FAILED = LA_BEGIN + 16,
	//Ȧ�Ӵ����ɹ�
	LA_HOME_CREATE_SUCCESSED = LA_BEGIN + 17,//�¼�err_no���ľ���home_id
	//�÷��������Ȧ�ӱ�ɾ����
	LA_HOME_SHARE_DELETED_BY_CREATOR = LA_BEGIN + 18,
	//����ע��ʧ��
	LA_HOME_SHARE_REGISTOR_FAILED = LA_BEGIN + 19,
	//�ֻ��˺Ŵ���ʧ��
	LA_PHONE_CREATE_FAILED = LA_BEGIN + 20,//�����ж�errno��ľ������
	//�ֻ��˺Ŵ����ɹ�
	LA_PHONE_CREATE_SUCCESS = LA_BEGIN + 21,
	//�ֻ��˺ŵ�½ʧ��
	LA_PHONE_LOGIN_FAILED = LA_BEGIN + 22,
	//�ֻ��˺ŵ�½�ɹ�
	LA_PHONE_LOGIN_SUCCESS = LA_BEGIN + 23,
	//�ֻ��˺ŵǳ�ʧ��
	LA_PHONE_LOGOUT_FAILED = LA_BEGIN + 24,
	//�ֻ��˺ŵǳ��ɹ�
	LA_PHONE_LOGOUT_SUCCESS = LA_BEGIN + 25,
	//�ֻ��˺�ɾ��ʧ��
	LA_PHONE_DEL_FAILED = LA_BEGIN + 26,
	//�ֻ��˺�ɾ���ɹ�
	LA_PHONE_DEL_SUCCESS = LA_BEGIN + 27,
	//�ֻ��˺��л�ʧ��
	LA_PHONE_SWICH_FAILED = LA_BEGIN + 28,
	//�ֻ��˺��л��ɹ�
	LA_PHONE_SWICH_SUCCESS = LA_BEGIN + 29,
	//�ֻ��˺������޸ĳɹ�
	LA_PHONE_PASSWD_MODIFY_SUCCESS = LA_BEGIN + 30,
	//�ֻ��˺������޸�ʧ��
	LA_PHONE_PASSWD_MODIFY_FAILED = LA_BEGIN + 31,
	//����һ���û���½�ߵ���
	LA_KICKED_BY_OTHER_USER = LA_BEGIN + 32,
	//����ע��ʧ�ܣ�����������
	LA_HOME_SHARE_REGISTOR_FAILED_OFFLINE = LA_BEGIN + 33,
	//���������ʧ��
	LA_SHARE_REQ_FAILED = LA_BEGIN + 34,

	//pad�������������ʧ��
	LA_USER_SHARE_REQ_FAILED = LA_BEGIN + 35,
	//pad�������������ɹ�
	LA_USER_SHARE_REQ_SUCCESSED = LA_BEGIN + 36,
	//pad���ͷ�����Ӽ�ͥʧ��
	LA_USER_SHARE_ADD_HOME_FILED = LA_BEGIN + 37,
	//pad���ͷ�����Ӽ�ͥ�ɹ�
	LA_USER_SHARE_ADD_HOME_SUCCESSED = LA_BEGIN + 38,
	//pad���ͷ���pad�����ά����յ��ɹ����Ȧ����Ϣ
	LA_USER_SHARE_ADD_ONE_HOME_SUCCESSED = LA_BEGIN + 39,

	//��ǩ���
	LA_LABEL_ADD_SUCCESS = LA_BEGIN + 40,
	LA_LABEL_ADD_FAILED = LA_BEGIN + 41,
	LA_LABEL_DEL_SUCCESS = LA_BEGIN + 42,
	LA_LABEL_DEL_FAILED = LA_BEGIN + 43,
	LA_LABEL_BIND_SUCCESS = LA_BEGIN + 44,
	LA_LABEL_BIND_FAILED = LA_BEGIN + 45,
	

	//������������Ϣ��ʾ�¼�
	LA_SERVER_PUSH_MSG = LA_BEGIN + 50,
	//���ӷ�����ʧ���¼�
	LA_SERVER_LINK_FAILED = LA_BEGIN + 51,
	//�����ļ��仯�¼�
	LA_CAP_FILE_CHANGED = LA_BEGIN + 52,
	//rsa����app�����¼�
	LA_RSA_REQ = LA_BEGIN + 53,
	//�ֻ��˺��������
	LA_PHONE_PASSWD_ERR = LA_BEGIN + 54,

	//s3ר��
	//��ѯʧ��
	LA_S3_QUERY_RULE_FAILED = LA_BEGIN + 55,
	//��ѯ�ɹ�
	LA_S3_QUERY_RULE_SUCCESSD = LA_BEGIN + 56,
	//��ѯ�����޸�ʱ��ʧ��
	LA_S3_QUERY_RULE_LAST_MODIFY_TIME_FAILED = LA_BEGIN + 57,
	//��ѯ�����޸�ʱ��ɹ�
	LA_S3_QUERY_RULE_LAST_MODIFY_TIMER_SUCCESSD = LA_BEGIN + 58,

	//��ݰ������
	LA_SHORTCUT_ADD_SUCCESSD = LA_BEGIN + 59,
	LA_SHORTUT_ADD_FAILED = LA_BEGIN + 60,
	LA_SHORTCUT_DEL_SUCCESSD = LA_BEGIN + 61,
	LA_SHORTCUT_DEL_FAILED = LA_BEGIN + 62,

	// �ֵ����
	LA_DICT_MODIFY = LA_BEGIN + 63,	//�ֵ��и���
	LA_DICT_SET_SUCCESS = LA_BEGIN + 64,	//�ֵ���ӳɹ�
	LA_DICT_SET_FAILED = LA_BEGIN + 65,	//�ֵ����ʧ��
	LA_DICT_DEL_SUCCESS = LA_BEGIN + 66,	//�ֵ�ɾ���ɹ�

	LA_SHORTCUT_MOD_SUCCESSD = LA_BEGIN + 67,
	LA_SHORTCUT_MOD_FAILED = LA_BEGIN + 68,

	//��־���
	LA_LOG_RULE_EXCUTE = LA_BEGIN + 69,//��������ִ����־��ѯ�ɹ����յ����¼��󣬴�errno��ȡ��home_id������ȡ��־
	
	LA_LOG_HOME_DEV_CHANGE = LA_BEGIN + 70,//�豸�Ƴ������ͥ��־��ѯ�ɹ����յ����¼��󣬴�errno��ȡ��home_id������ȡ��־

	// �ֵ����
	LA_DICT_DEL_FAILED = LA_BEGIN + 71,	//�ֵ�ɾ��ʧ��
	LA_DICT_QUERY_FAILED = LA_BEGIN + 72,	// �ֵ��ѯʧ��

	//����ִ����־�¼������������������˱仯 ��־���index�������ϴ��¼�ʱerr_no = max_index
	LA_LOG_RULE_EXCUTE_MAX_INDEX = LA_BEGIN + 73,

	LA_WIDGET_KEY_DONAME_CHANGED = LA_BEGIN + 74,//��Կ���������仯�ˣ�app��Ҫȥ��ȡͬ������

	LA_END = 2399,
};

enum {
	CAP_FILE_TYPE_NONE = 0,
	CAP_FILE_TYPE_COM,//��ͨ�����ļ�
	CAP_FILE_TYPE_CUSTOM,//�Զ��������ļ�
};

typedef struct {
	u_int16_t type;//CAP_FILE_TYPE_COM ....
	//�豸�����ļ��ϴ��޸�ʱ��
	u_int32_t last_cap_time;
	u_int16_t cap_num;//�����ļ�����
	u_int8_t **cap_array;//�����ļ�url����
}cap_file_type_t;

typedef struct {
	u_int8_t file_type_num;//�����������
	cap_file_type_t *pfile_type;
}cap_file_t;

typedef struct {
	u_int8_t user_num;//�Ժ�����������ж��������ֻ��һ�����ȷ������
	u_int32_t user_id[10];//����user_num��ȡuser_id

	u_int8_t phone_num;//�ֻ��˺Ÿ���,����0����Ч
	u_int8_t cur_phone;//��ǰ�ֻ��˺�
	char **phone_name;//�ֻ��˺���

	u_int8_t language;// 1-���ģ�2-Ӣ��
/*
	//�豸�����ļ��ϴ��޸�ʱ��
	u_int32_t last_cap_time;
	u_int16_t cap_num;//�����ļ�����
	u_int8_t **cap_array;//�����ļ�url����
*/
	cap_file_t cap_file;

	//�Ƿ�֧�ֱ�ǩ�������п��ܷ�����û������ԭ��
	bool support_label;
	
	u_int8_t home_num;
	cl_user_t **home;
}cl_la_info_t;

//��������Ϣ�ϱ��ṹ
typedef struct {
	u_int16_t type;//��Ϣ����
	char *cn_msg;
	char *en_msg;
}cl_la_notify_msg_t;

#define LA_USE_RSA

//rsa�������ݽṹ
typedef void (*cl_rsa_callback)(int priv_len, u_int8_t *priv, int in_len, u_int8_t *in, u_int8_t *out);

typedef struct {
	cl_rsa_callback dec;
	cl_rsa_callback enc;
}cln_la_rsa_t;

//s3ר��
typedef struct {
	u_int32_t home_id;
	u_int16_t rule_num;//��ӵĹ���
	u_int32_t last_modify_time;//�ü�ͥ�ϴι��������޸�ʱ��
	cl_rule_desc_t *rule_array;//��ӵĹ����ַ���
}cl_home_rule_info_t;


//��־���
//����ִ�б仯��־
typedef struct {
	u_int32_t index;

	u_int8_t is_test;//�Ƿ�����һ��ִ�й���
	u_int8_t is_timer;//�Ƿ���timer��
	
	u_int16_t monitor_count;
	u_int64_t *pmonitor_sn;
	
	u_int16_t action_count;
	u_int64_t *paction_sn;

	u_int32_t ruld_id;
	u_int32_t time_stamp;
}home_log_rule_info_t;

typedef struct {
	u_int32_t home_id;
	u_int32_t max_index;
	
	u_int16_t log_count;
	home_log_rule_info_t *plog;
}cl_home_log_rule_change_t;

//��ͥ�豸�仯��־
#define HOME_MEM_CHANGE_ACTION_OUT 	0
#define HOME_MEM_CHANGE_ACTION_IN 	1

typedef struct {
	u_int64_t sn;
	u_int8_t action;
	u_int8_t resaon;
	u_int8_t username[APP_USER_UUID_NAME_LEN];
	u_int32_t user_id;
	u_int32_t time_stamp;
}home_log_member_info_t;

typedef struct {
	u_int32_t home_id;
	
	u_int16_t log_count;
	home_log_member_info_t *plog;
}cl_home_log_member_change_t;



//��Կ������ȡ
#define WIDGET_KEY_MAX_LEN (32)
typedef struct {
	u_int8_t key[WIDGET_KEY_MAX_LEN];
	char doname[64];
}cl_widget_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
����: 
    ��ȡsdk������������Ϣ    
 ����:
    cl_la_info_t
    ����: ʧ��
 */
CLIB_API cl_la_info_t *cl_la_info_get();
CLIB_API RS cl_la_info_free(cl_la_info_t * info);
/*
	��������:
	��ͥ����������û��ã�����ʱ�ù����ĸ��������ϣ�������
	����:
	@name, ��ͥ��
*/
CLIB_API RS cl_la_home_create(cl_handle_t *homehandle, char *name);

/*
	��������:
	��ͥɾ��
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
*/
CLIB_API RS cl_la_home_del(cl_handle_t homehandle, u_int32_t home_id);


/*
	��������:
	��ͥ��Ա���
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@num,��ӵ��豸����
	@*sn,��ӵ��豸sn����
*/
CLIB_API RS cl_la_home_adddev(cl_handle_t homehandle, u_int32_t home_id, u_int8_t num, u_int64_t *sn);

/*
	��������:
	��ͥ��Աɾ��
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@num,��ӵ��豸����
	@*sn,��ӵ��豸sn����
	@flag: bit 0
			Ϊ0��ʾ��Ҫ�ָ����� ��Ϊ1��ʾ����Ҫ�ָ�����
*/
CLIB_API RS cl_la_home_removedev(cl_handle_t homehandle, u_int32_t home_id, u_int8_t num, u_int64_t *sn, u_int8_t flag);
/*
	��������:
	��������ģ���ѯ
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
*/
CLIB_API RS cl_la_config_template_query(cl_handle_t homehandle, u_int32_t home_id);
/*
	��������:
	�������ù����ѯ
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@rule_id, ����id
*/
//  ��ѯһ������
CLIB_API RS cl_la_config_rule_query(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id);
/*
	��������:
	�������ù������
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@rule_id, ����id
	@rule, ��������
*/
// ���һ������
CLIB_API RS cl_la_config_rule_add(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id, char *rule, u_int8_t enable);
/*
	��������:
	�������ù���ɾ��
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@rule_id, ����id
*/
// ɾ��һ������
CLIB_API RS cl_la_config_rule_del(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id);
/*
	��������:
	�����¼�����
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@rule_id, ����id
*/
// �����¼�
CLIB_API RS cl_la_config_event(cl_handle_t homehandle, u_int8_t event, u_int32_t home_id, u_int32_t rule_id);
/*
	��������:
	��ͥ��������
	����:
	@homehandle,��ͥhandle
	@home_id, ��ͥid
*/
/*
	��ͥ��Ա������Ա����
**/
// App������������ͥ�ķ����룬������ȷ������Ч�ڣ�Ĭ��10���ӡ�
CLIB_API RS cl_la_home_share_create(cl_handle_t homehandle, u_int32_t home_id);
// ע��
/*
	��������:
	��ͥ����ע��
	����:
	@homehandle,��ͥhandle
	@home_id, ��ͥid
	@share,��ά�����ɵĹ������ַ���
	@desc,����
*/
CLIB_API RS cl_la_home_share_register(cl_handle_t homehandle, u_int32_t home_id, u_int8_t *share, u_int8_t *desc);
// ��ѯ����
/*
	��������:
	��ͥ�����ѯ
	����:
	@homehandle,��ͥhandle
	@home_id, ��ͥid
*/
CLIB_API RS cl_la_home_share_query(cl_handle_t homehandle, u_int32_t home_id);
/*
	��������:
	��ͥ����༭�޸�
	����:
	@homehandle,��ͥhandle
	@home_id, ��ͥid
	@desc, ����
*/
// ����༭
CLIB_API RS cl_la_home_share_edit(cl_handle_t homehandle, u_int32_t home_id, u_int32_t user_id, u_int8_t role_id, u_int8_t *desc);
/*
	��������:
	��ͥ����༭�޸�
	����:
	@homehandle,�û�handle
	@home_id, ��ͥid
	@user_id, �û�id
*/
// ɾ������
CLIB_API RS cl_la_home_share_del(cl_handle_t homehandle, u_int32_t home_id, u_int32_t user_id);

/*
	��������:
	app���ã�Ҫ��sdk�Զ���½��sn�豸
	����:
	@homehandle,�û�handle
	@sn,���ɸ�sn������
	@num,num��sn
*/
// ɾ������
CLIB_API RS cl_la_sdk_login_set(cl_handle_t homehandle, u_int64_t *sn, u_int16_t num);

/*
	��������:
	app�������
	����:
	@homehandle,�û�handle
	@sn,���ɸ�sn������
	@num,num��sn
*/
// ɾ������
CLIB_API void cl_la_conf_clean();
/*
	��������:
	�������ù���ִ��
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@rule_id, ����id
*/
// ɾ��һ������
CLIB_API RS cl_la_config_rule_exec(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id);

// ��ѯ����
/*
	��������:
	���õ�ǰ��ͥid
	����:
	@homehandle,��ͥhandle
	@home_id, ��ͥid
*/
CLIB_API RS cl_la_cur_homeid_set(u_int32_t home_id);

/*
	��������:
	��ȡ��ǰȦ��id
	����:
	��
	����:
	��Ϣ����
*/
CLIB_API u_int32_t cl_la_cur_homeid_get();

/*
	��������:
	��ͥ���豸�ƶ��ӿ�
	����:
	@homehandle,Ŀ�ļ�ͥhandle
	@src_home_id���Ƴ��豸�ļ�ͥid
	@dst_home_id���ƽ��豸�ļ�ͥid
	@num,�ƶ����豸����
	@*sn,�ƶ����豸sn����
*/
CLIB_API RS cl_la_home_movedev(cl_handle_t homehandle, u_int32_t src_home_id, u_int32_t dst_home_id, u_int8_t num, u_int64_t *sn);

/*
	��������:
	���������޸ĳ��Ƿ�ʹ��
	����:
	@homehandle,��ͥhandle
	@home_id����ͥid
	@rule_id, ����id
	@enable, �Ƿ�ʹ�ܣ�1ʹ��
*/
// ɾ��һ������
CLIB_API RS cl_la_config_rule_modify(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id, u_int8_t enable);

/*
	��������:
	�޸ļ�ͥ����
	����:
	@home_id,�޸ĵļ�ͥid
	@name, ��ͥ��
*/
CLIB_API RS cl_la_home_name_modify(cl_handle_t homehandle, u_int32_t home_id, char *name);

/**************************************************�ֻ��˺Ų����ӿ�**********************************************************************************/
/*
	��������:
	�����ֻ��û�
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API RS cl_la_phone_create(char *name, char *passwd);

/*
	��������:
	��½�ֻ��û�
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API RS cl_la_phone_login(const char *name, const char *passwd);


/*
	��������:
	��ǰ����ȥ˯�ߵ�session���µ�½
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API RS cl_la_phone_relogin();

/*
	��������:
	�ǳ��ֻ��û�
	����:
	@name,�û���
*/
CLIB_API RS cl_la_phone_logout(char *name);

/*
	��������:
	ɾ���ֻ��û������޸ķ�����
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API RS cl_la_phone_del(char *name);
/*
	��������:
	�л��ֻ��û�
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API RS cl_la_phone_swich(char *name);
/*
	��������:
	�ֻ��û������޸�
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API RS cl_la_phone_passwd_modify(char *name, char *passwd);

/*
	��������:
	�ֻ��û�����Ƚ�
	����:
	@name,�û���
	@passwd, ����
*/
CLIB_API bool cl_la_phone_is_same_passwd(char *name, char *passwd);

//��������Ϣ֪ͨ
/*
	��������:
	�յ� LA_SERVER_PUSH_MSG �¼�����û�ȡ��Ϣ����
	����:
	��
	����:
	��Ϣ����
*/
CLIB_API cl_la_notify_msg_t *cl_la_get_notify_msg();

/*
	��������:
	�Ƿ���Ϣ
	����:
	��
	����:
	��Ϣ����
*/
CLIB_API void cl_la_free_notify_msg(cl_la_notify_msg_t *msg);

/*
	��������:
	������������
	����:
	lang@:1-���ģ�2-Ӣ��
*/
CLIB_API RS cl_la_cur_lang_set(u_int8_t lang);

/*
	��������:
	��ȡ��������
	����:
	lang@:1-���ģ�2-Ӣ��
*/
CLIB_API RS cl_la_cur_lang_get(u_int8_t *lang);

/*
	��������:
	��ȡ�豸�����ļ���ѯ�ӿ�
	����:
	lang@:1-���ģ�2-Ӣ��
*/
CLIB_API RS cl_la_cap_file_query();

/*
	��������:
	��ȡ�豸�Զ��������ļ���ѯ�ӿ�
	����:
	lang@:1-���ģ�2-Ӣ��
*/
CLIB_API RS cl_la_cap_custom_file_query();

/*
	��������:
	rsa��ʼ���������˳�����ע��
	����:
	@rsa_enc:rsa���ܺ���
	@rsa_dec:rsa���ܺ���
*/
CLIB_API RS cl_la_rsa_callback_set(cl_rsa_callback rsa_enc, cl_rsa_callback rsa_dec);


// App������������ͥ�ķ����룬������ȷ������Ч�ڣ�Ĭ��10���ӡ�Ϊ��pad��ӣ��������
CLIB_API RS cl_la_app_share_create();
// ע��
/*
	��������:
	��ͥ����ע��
	����:
	@home_id, ��ͥid
	@share,��ά�����ɵĹ������ַ���
	@desc,����
*/
CLIB_API RS cl_la_app_share_register(u_int32_t home_id, char *share, char *desc);

/*
*��app�յ� ����ɹ��¼�LA_USER_SHARE_REQ_SUCCESSED �󣬵�������ӿڻ�ȡshare�ַ���
*/
CLIB_API char *cl_la_app_get_share();


//��ǩ����
/*
	��������:
	����޸ı�ǩ,������豸���뵽��Ӧ��ǩ�У��򴴽��ı�ǩ�С�
	����:
	@home_id, ��ͥid
	@label_id,��ǩid��0��ʾ������������ʾ�޸�
	@name:��ǩ���ƣ����63�ֽ�
	@sn_num:��ǩ������sn����,����Ϊ0, 0-n����
	@*sn:ָ����ӵ�sn����
*/
CLIB_API RS cl_la_label_add_modify(u_int32_t home_id, u_int16_t label_id, char *name, u_int16_t sn_num, u_int64_t *sn);

/*
	��������:
	ɾ����ǩ
	����:
	@home_id, ��ͥid
	@label_id,��ǩid
*/
CLIB_API RS cl_la_label_del(u_int32_t home_id, u_int16_t label_id);

/*
	��������:
	���豸��Ӹ������ǩ��
	����:
	@home_id, ��ͥid
	@sn��Ҫ��ӵ�������ǩ�е�sn
	@label_id_num:Ҫ��ӵı�ǩ��,������Ϊ0
	@*lable_id:Ҫ��ӵı�ǩ����
*/
CLIB_API RS cl_la_bind_devs_to_label(u_int32_t home_id, u_int64_t sn, u_int16_t label_id_num, u_int16_t *label_id);

// �ֵ�

/*
*	��������:
*	������ѯ��ͥ�µ�ĳ���ֵ�(������ȡ)����ѯ����LA_DICT_MODIFY�¼�
*	����:
*	@home_id,����ȡ�ֵ��homeid
	@key: �ֵ�
	@key_len: �ֵ䳤��
*/
CLIB_API RS cl_la_dict_query(u_int32_t home_id, u_int8_t *key, u_int16_t key_len);

/*
*	��������:
*	���ü�ͥ�µ�ĳ���ֵ䣬��LA_DICT_SET_SUCCESS��LA_DICT_SET_FAILED�¼�
*	����:
*	@home_id,����ȡ�ֵ��homeid
	@key: �ֵ�
	@key_len: �ֵ䳤��
	@value: �ֵ���ֵ
	@value_len: �ֵ����ݳ���
*/
CLIB_API RS cl_la_dict_set(u_int32_t home_id, u_int8_t *key, u_int8_t *value, u_int16_t key_len, u_int16_t value_len);

/*
*	��������:
*	���ü�ͥ�µ�ĳ���ֵ�
*	����:
*	@home_id,����ȡ�ֵ��homeid����LA_DICT_DEL_SUCCESS�¼�
	@key: �ֵ�
	@key_len: �ֵ䳤��
	@value: �ֵ���ֵ
	@value_len: �ֵ����ݳ���
*/
CLIB_API RS cl_la_dict_del(u_int32_t home_id, u_int8_t *key, u_int16_t key_len);

//��ݰ�������

/*
	��������:
	��ݰ����������������
	����:
	@home_id, ��ͥid
	@index, 1-6,6���Զ����ݼ�
	@name,�Զ�������
	@rule_id,����id
*/
CLIB_API RS cl_la_shortcut_rule_bind(u_int32_t home_id, u_int8_t index, u_int8_t enabel, char *name, u_int32_t rule_id, char *rule);

/*
	��������:
	��ݰ������޸ġ�
	����:
	@index, 1-6,6���Զ����ݼ�
	@name,�Զ�������
*/
CLIB_API RS cl_la_shortcut_modify(u_int32_t home_id, u_int8_t index, char *name);

/*
	��������:
	��ݿ��أ��������
	����:
	@home_id, ��ͥid
	@index, 1-6,6���Զ����ݼ�
*/
CLIB_API RS cl_la_shortcut_del(u_int32_t home_id, u_int8_t index);


//s3����ר�ýӿ�
/*
*	��������:
*	ͨ��homeid��ѯ�����б�
*	����:
*	@home_id,����ѯhomeid
*/
CLIB_API RS cl_la_home_rule_query(u_int32_t home_id);

/*
*	��������:
*	ͨ��homeid��ȡsdk�ڵĹ����б�
*	����:
*	@home_id,����ȡ�����homeid
*/
CLIB_API cl_home_rule_info_t *cl_la_home_rule_get(u_int32_t home_id);

/*
*	��������:
*	�ͷ�
*	����:
*/
CLIB_API RS cl_la_home_rule_free(cl_home_rule_info_t *pinfo);

/*
*	��������:
*	��ѯ��ͥ�¹���������޸�ʱ��
*	����:
*	@home_id,����ѯ��homeid
*/
CLIB_API RS cl_la_home_rule_query_last_modify_time(u_int32_t home_id);

/*
*	��������:
*	pad��Ҫ����ʱ��ִ�й���
*	����:
*	@home_id,
*	@rule, ��Ҫִ�еĹ���
*/
CLIB_API RS cl_la_home_rule_excute(u_int32_t home_id, char *rule);


/*
*	��������:
*	��ѯ��ͥ����ִ����־
*	����:
*	@home_id,
*	@index:��־index >=0
*	@count:��ѯ��־����
*/
CLIB_API RS cl_la_home_rules_logs_query(u_int32_t home_id, u_int32_t index, u_int32_t count);

/*
*	��������:
*	��ȡ��ͥ����ִ����־
*	����:
*	@home_id,
*/
CLIB_API cl_home_log_rule_change_t *cl_la_home_rules_logs_get(u_int32_t home_id);

/*
*	��������:
*	��ȡ��ͥ����ִ����־
*	����:
*	@home_id,
*/
CLIB_API RS cl_la_home_rules_logs_free(cl_home_log_rule_change_t *pinfo);

/*
*	��������:
*	��ѯ��ͥ���豸�䶯���
*	����:
*	@home_id,
*/
CLIB_API RS cl_la_home_members_logs_query(u_int32_t home_id);

/*
*	��������:
*	��ȡ��ͥ���豸�䶯���
*	����:
*	@home_id,
*/
CLIB_API cl_home_log_member_change_t *cl_la_home_members_logs_get(u_int32_t home_id);

/*
*	��������:
*	��ȡ��ͥ���豸�䶯���
*	����:
*	@home_id,
*/
CLIB_API RS cl_la_home_members_logs_free(cl_home_log_member_change_t *pinfo);

/*
*	��������:
*	widget��ȡ��Կ��ѯ����https��ѯ����Կ��ʱ������µ��ã�����ֻ�а�׿��Ҫ���ã���Ϊ�������ں�̨��פ��������������
*	����������Զ���ȡһ���µģ�����ios���ܲ���Ҫ��ѯ
*	����:
*	@home_id,
*/
CLIB_API RS cl_la_widget_key_query();

/*
*	��������:
*	���յ���Կ�䶯�¼��󣬵��øú�����ȡ��Կ������Ϣ
*	����:
*/
CLIB_API cl_widget_info_t *cl_la_widget_info_get();

/*
*	��������:
*	�ͷ�����
*	����:
*/
CLIB_API RS cl_la_widget_info_free(cl_widget_info_t *pinfo);




#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_LINKAGE_H */

