#ifndef	__UASC_PRIV_H__
#define	__UASC_PRIV_H__

// �����û�����
enum{
	ACT_UASC_START_CONNECT,
	ACT_UASC_PUSH_APP_STAT,
	ACT_MAX
};

// APP ״̬�ϴ�TLV ȡֵ
enum{
	AS_TT_UUID = 0x1, // uuid 16�ֽ�
	AS_TT_APP_VERSION , // app �汾
	AS_TT_HW_TYPE, // ����:  Ӳ�����ͣ�pad�����ֻ�
	AS_TT_HW_DESC, // Ӳ��������Ϣ����Ϊ��iphone4s��
	AS_TT_OS_TYPE, //  ����:  Android or iphone or PC  
	AS_TT_OS_VERSION, // ����ϵͳ�汾���ַ���
	AS_TT_APP_TYPE, //����,4�ֽ�
	AS_TT_SN,
	AS_TT_UDP_CTRL
};


extern bool uasc_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret);

#endif

