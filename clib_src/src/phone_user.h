#ifndef	__PHONE_USER_H__
#define	__PHONE_USER_H__

#ifdef __cplusplus
extern "C" {
#endif 

//CGI�ӿ�ͨ�ô�����
#define PUE_NONE 0 //�ɹ�
#define PUE_OTHER 1 //��������
#define PUE_KEY_EXPIRE 2 //key����
//ע����һ�����
#define PUE_REG_HAS_EXIST 2 //ע��ʱ���û��Ѿ�����
#define PUE_REG_NOT_EXIST 3 //�һ�����ʱ���û�������
//�ύ��֤��
#define PUE_VCODE_INVALID 2 //��֤�����
#define PUE_VCODE_EXPIRE 3	//��֤�����
//��¼
#define PUE_AUTH_NOT_EXIST 2 //�û���������
#define PUE_AUTH_BAD_PWD 3 //�������


#define	DEV_DICT_KEY_V1	"devinfo_"
#define	DEV_DICT_KEY_V2	"dinf_v2_"

extern RS pu_ignore_notify(cl_notify_pkt_t *pkt);
extern RS pu_on_get_dev_notify(cl_notify_pkt_t *pkt);
extern void pu_get_dev(user_t *user);
extern RS pu_on_add_dev_notify(cl_notify_pkt_t *pkt);
extern RS pu_add_dev(cl_notify_pkt_t *pkt);
extern RS pu_on_del_dev_notify(cl_notify_pkt_t *pkt);
extern RS pu_del_dev(cl_notify_pkt_t *pkt);

extern void pu_init_passwd(user_t *user);
extern RS pu_on_login_notify(cl_notify_pkt_t *pkt);
extern void pu_auth(user_t *user);
extern int pu_get_timeout(user_t *user);
extern RS pu_on_get_dict(cl_notify_pkt_t *pkt);
extern void pu_on_dev_login(user_t *dev, bool is_ok);

extern RS pu_on_reset_notify(cl_notify_pkt_t *pkt);
extern RS pu_on_register_notify(cl_notify_pkt_t *pkt);
extern RS pu_register(cl_notify_pkt_t *pkt);
extern RS pu_reset(cl_notify_pkt_t *pkt);
extern RS pu_on_send_vcode_notify(cl_notify_pkt_t *pkt);
extern RS pu_send_vcode(cl_notify_pkt_t *pkt);
extern RS pu_on_modify_passwd_notify(cl_notify_pkt_t *pkt);
extern void pu_modify_user_pwd(user_t *user, char *pwd);
extern void pu_modify_dev_pwd(user_t *user, char *pwd);
extern RS pu_on_put_dict_notify(cl_notify_pkt_t *pkt);
extern RS pu_on_dict_add_dev_notify(cl_notify_pkt_t *pkt);
extern RS pu_on_dict_del_dev_notify(cl_notify_pkt_t *pkt);
extern RS pu_on_dict_modify_passwd_notify(cl_notify_pkt_t *pkt);
extern void pu_save_dev_type(user_t *user);
extern RS pu_on_get_devtype_notify(cl_notify_pkt_t *pkt);
extern void pu_get_devtype(user_t *user);
extern void pu_save_dev_type(user_t *user);
extern RS pu_apns_config(user_t *user, cl_notify_pkt_t *pkt);
extern RS pu_on_apns_config_notify(cl_notify_pkt_t *pkt);
extern RS pu_on_apns_config_notify_v2(cl_notify_pkt_t *pkt);
extern RS pu_on_apns_config_notify_sn(cl_notify_pkt_t *pkt);

extern RS pu_set_notify_expect_id(user_t *user, cln_notify_push_t *param);
extern RS pu_on_msg_push_notify(cl_notify_pkt_t *pkt);
#ifdef __cplusplus
}
#endif 

#endif


