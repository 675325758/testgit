#ifndef	__CA_PRIV_H__
#define	__CA_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif 
#define REMOTE_CLOUD_IR_STATUS_VALID 1	// �����Ƿ�ɹ����ر���
#define REMOTE_CLOUD_IR_STATUS_OLD_AC 2 // �Ƿ�����ʽ�յ�
#define REMOTE_CLOUD_IR_STATUS_LAST 3	// �ϴο��Ƶļ�


extern bool ca_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern void do_cmd_cm(user_t *user, pkt_t *pkt, net_header_t *hdr);




#ifdef __cplusplus
}
#endif 

#endif





