#ifndef	 __PLUG_PRIV_H__
#define	__PLUG_PRIV_H__

#include "cl_plug.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define	DFL_PLUG_TIMER_Q_BASE	5
#define	DFL_PLUG_TIMER_Q_STAT	5
#define	DFL_PLUG_TIMER_Q_TIMER	10

typedef struct _plug_s {
	// ��ָָ��
	slave_t *slave;

	// ��ͬ��slave->handle
	cl_handle_t handle;

	// �����Ƿ��ǿ��ٲ�ѯ
	bool quick_query;

	// ��ѯ������Ϣ���������ء���������ѹ���¶�
	int timer_q_base;
	int count_q_base;
	// ��ѯ���������ͽ׶ε���
	int timer_q_stat;
	int count_q_stat;
	// ��ѯ��ʱ���ص����ã�������Խϳ�ʱ���ѯһ��
	int timer_q_timer;
	int count_q_timer;

	cl_callback_t callback;
	void *callback_handle;

	// ��ѯ�����Ľ��
	bool on; /* ���� */
	int ac; /* ��������λ: 0.1A */
	int v; /* ��ѹ����λ: V  */
	int t; /* �¶�, ��λ: C */

	u_int32_t electric_stat_total; /*�ӿ��������ڵ��ܵ���,��λ W*/
	u_int32_t electric_stat_section; /*��timeָ��ʱ�ĵ���ǰ�Ľ׶ε���,��λ W*/
	u_int32_t section_time; /*ָ����ĳһʱ��,�����Լ��㵽��ǰʱ���ۻ��ĵ���*/	

	/* ��ʱ���ز��� */
	net_plug_timer_config_t *timer;

	cl_thread_t *t_query;
} plug_t;

#define	HANDLE_QUERY_PLUG 0x11330000


extern void plug_quick_query(plug_t *plug);
extern bool plug_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern plug_t *plug_alloc(slave_t *slave);
extern void plug_free(plug_t *plug);
extern bool plug_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);


#ifdef __cplusplus
}
#endif 

#endif

