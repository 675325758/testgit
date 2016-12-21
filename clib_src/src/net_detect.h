
#ifndef	__CL_NET_DETECT_H__
#define	__CL_NET_DETECT_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 


typedef struct {
	u_int32_t nd_ptr;
	u_int32_t time;
	u_int8_t pad[8];
} nd_cookie_t;

#define	ETH_IP_UDP_LEN	(14 + 20 + 8)

typedef struct {
	struct stlc_list_head link;
	
	u_int32_t m_nIp;
	int m_nPort;
	int m_nType;
	int m_nPriority;
	nd_cookie_t m_Cookie;

	// ���������ĵ�ʱ��, ��λ΢��
	u_int32_t m_liQTime;
	// �յ���һ����Ӧ���ĵ�ʱ��, ��λ΢��
	u_int32_t m_liFirstATime;
	// �������������, ��λ bps
	double m_Rate;
	// ʱ�ӣ���λ��
	double m_Delay;
	// �յ��ı�����Ŀ
	int m_nPkts;
	// �յ����ֽ���Ŀ
	int m_nBytes;
} net_detect_t;


extern int timer_net_detect(cl_thread_t *t);
extern RS recv_net_detect_a(user_t *user, pkt_t *pkt);

#ifdef __cplusplus
}
#endif 


#endif



