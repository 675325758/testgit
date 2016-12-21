#ifndef	__USER_PRIV_H__
#define	__USER_PRIV_H__


#define USER_BACKGROUND_RETURN_CHECK(user) \
{ \
if (is_user_in_background_mode(user)){\
    return 0;\
}\
}

#ifdef __cplusplus
extern "C" {
#endif 

extern RS tcp_check_connecting(user_t *user);
extern bool user_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
extern bool user_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern const char *err_2_str(int ecode);
extern void user_set_status(user_t *user, u_int32_t status);
extern void user_set_status_uc(user_t *user, u_int32_t status, bool from_uc);
extern int timer_die(cl_thread_t *t);
extern void user_del_all();
extern user_t *user_create(bool is_phone_dev, 
			char *username, char *passwd, char* qr_code,char *license, int len_license,
			cl_callback_t callback, void *callback_handle, bool authing, bool is_home);
extern void user_destroy(user_t *user);
extern void user_free(user_t *user);
extern void user_get_nickname(user_t *user);
void callback_user_requst(u_int32_t result, void *none, void *waitp);
extern RS user_set_direct_login(user_t *user, u_int32_t ip);
extern RS user_create_udp(user_t *user);
extern bool is_user_in_background_mode(user_t* user);
    
extern RS ajust_dev_sub_type_by_sn(u_int64_t sn, u_int8_t* sub_type ,u_int8_t* ext_type);
    
extern RS map_test_dev_sub_type(u_int8_t* sub_type ,u_int8_t* ext_type);

extern void user_request_display_info_save(void);
extern int disp_stat_timeout(cl_thread_t *t);
extern  u_int32_t user_select_ip(user_t *user);
#ifdef __cplusplus
}
#endif 

#endif


