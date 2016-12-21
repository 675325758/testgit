
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

	// 发送请求报文的时间, 单位微秒
	u_int32_t m_liQTime;
	// 收到第一个回应报文的时间, 单位微秒
	u_int32_t m_liFirstATime;
	// 计算出来的速率, 单位 bps
	double m_Rate;
	// 时延，单位秒
	double m_Delay;
	// 收到的报文数目
	int m_nPkts;
	// 收到的字节数目
	int m_nBytes;
} net_detect_t;


extern int timer_net_detect(cl_thread_t *t);
extern RS recv_net_detect_a(user_t *user, pkt_t *pkt);

#ifdef __cplusplus
}
#endif 


#endif



