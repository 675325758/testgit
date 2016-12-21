#ifndef	__CA_PRIV_H__
#define	__CA_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif 
#define REMOTE_CLOUD_IR_STATUS_VALID 1	// 电器是否成功下载编码
#define REMOTE_CLOUD_IR_STATUS_OLD_AC 2 // 是否是老式空调
#define REMOTE_CLOUD_IR_STATUS_LAST 3	// 上次控制的键


extern bool ca_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern void do_cmd_cm(user_t *user, pkt_t *pkt, net_header_t *hdr);




#ifdef __cplusplus
}
#endif 

#endif





