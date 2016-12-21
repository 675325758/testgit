#ifndef __DS_PROTO_HEADER__
#define __DS_PROTO_HEADER__

#ifdef CLIB_HDR
  /*������*/
  #include "client_lib.h"
  #pragma warning(disable:4819)
#else
  /*�������Ͱ����豸*/
  #include "ds_types.h"
  #include "charge_card.h"
  #include "intelligent_route.h"
#endif

/*role type of DS007 system*/
enum {
	TP_CENTER = 0,		/*��������*/
	TP_DISPATCHER = 1,	/*���������*/
	TP_DEV_SRV = 2,		/*�豸������*/
	TP_WEB_SRV = 3,		/*web������*/
	TP_USER = 4,		/*�ֻ����������û�*/
	TP_DS007 = 5,		/* DS007�豸*/
	TP_CHARGE_SRV = 6,		/* ��ֵ������ */
	TP_CHARGE_CLI_RO = 7,	/* ��ֵ�ͻ��ˣ�ֻ�� */
	TP_CHARGE_CLI_WR = 8,	/* ��ֵ�ͻ��ˣ���д  */
	TP_MAX
};

/*subtype of 007 device*/
#define IJ_TYPE_NUM 0x04

#define IJ_007 			0x0  	/*i+007*/
#define IJ_001 			0x01  	/*i+001 Ŀǰ��Ҫ��i+001E���ߺ���ת����*/
#define IJ_002 			0x02  	/*i+002 iTV-C*/
#define IJ_003 			0x03  	/*i+003 wireless camera*/
#define IJ_006 			0x6  	/*i+006*/
#define IJ_008 			0x08  	/* i+008 iTV-C */
#define IJ_803 			0x37  	/*��˹��������ͷ*/
#define IJ_807 			0x09  	/*i+807E*/
#define IJ_FANSBOX 		0x0A 	/* fansbox*/


#define IJ_808			0x10	/*i+808����*/

#define IJ_UDP_DEVICE_BASE IJ_808
#define IJ_UDP_DEVICE_END 0x1f
#define IS_UDP_DEV(subtype) (((subtype)>=IJ_UDP_DEVICE_BASE)&&((subtype)<=IJ_UDP_DEVICE_END))


#define IJ_OPT 			0x20 	/*openwrtˢ���豸*/
#define IJ_ANDROID 		0x21  	/*Andriodˢ���豸*/
#define IJ_COMMUNITY 	0x22   /* С�������� */

#define IJ_AIRCONDITION 0x30	/* �ǻۼҵ絥Ʒ�յ� */
#define IJ_WATERHEATER 0X31		/* �ǻۼҵ絥Ʒ��ˮ�� */
#define IJ_AIRHEATER 0x32		/* �ǻۼҵ絥Ʒ����¯ */
#define IJ_AIRCLEANER 0x33		/* �ǻۼҵ絥Ʒ���������� */
#define IJ_ELECTRICFAN 0x34		/* �ǻۼҵ絥Ʒ����� */
#define IJ_BATHROOMMASTER 0x35	/* �ǻۼҵ絥Ʒԡ�� */
#define IJ_805		0x36		/* IJ805 */
#define	IJ_UNKNOW	0xFF /* δ֪������ */

/*extented type of IJ_008*/
#define EX_008_SL 1
#define EX_008_SH 2
#define EX_008_EL 3
#define EX_008_EH 4

/*extented type of IJ_003*/
/*���涨���mu�����п�������*/
//����Ӳ��ƽ̨�汾����
enum {
	/*  �汾1:
		i+003Cam-H
		i+003Cam-M
		�Լ�����ƽ̨Ŀǰֻ��һ��Ӳ���汾,����i+007E ...
	*/
	EX_003_CAMH = 1,
	
	/*  �汾2:
		i+003Cam-Y
	*/
	EX_003_CAMY = 2,
	
	/*  �汾3:
		 i+003Cam-G(toplink���ҵ�Ӳ��oem�汾) 
	*/
	EX_003_CAMG = 3, 
	
	/*  �汾4:
		i+003Cam-H(S)(��i+003Cam-HӲ���Ļ����ϣ��ü���usb�ӿں�tf���ӿ�)
	*/
	EX_003_CAMHS = 4,
	
	/*  �汾5:
		i+003Cam-OG(����ǹ�ͣ��˻����Ƽ����ҵ�Ӳ��oem�汾)
	*/
	EX_003_CAMOG = 5,
	
	/*  �汾6:
		i+003Cam-OB(����Բ���ͣ��˻����Ƽ����ҵ�Ӳ��oem�汾)
	*/
	EX_003_CAMOB = 6,

	/*  �汾7:
		i+003Cam-YH(Cam-YS����ǿ�汾�����Ӻ�����Ƶ�Ƭ��)
	*/
	EX_003_CAMYH = 7,
	
	/*  �汾8:
		i+003Cam-G galaxywind
	*/
	EX_003_CAMYG = 8,
	EX_003_MAX
};

/*extented type of IJ_803*/
enum {
	/*
	* i+803-hie, easyn, ��˼����ͷ
	*/
	EX_803_HS = 0x01,
	/*
	* i+gepc2, galaxywind, C2����ͷ
	*/
	EX_803_C2 = 0x02,
	/*
	* i+gepc3 , galaxywind, C3����ͷ
	*/
	EX_803_C3 = 0x03,
	EX_803_MAX,
};

#define STP_MAX 256 /*max sub type*/

enum{/*��¼����*/
	LT_NORMAL = 1, /*��ͨ��¼��δ���ð��ֻ�*/
	LT_BIND = 2, /*���ֻ���¼*/
	LT_UNBIND = 3 /*δ���ֻ���¼*/
};

enum{/*������������*/
	NT_SERVER = 1, /*���ӷ�����*/
	NT_DEVICE = 2, /*������ֱ���豸���豸���ӵ�������*/
	NT_DEVICE_OFFLINE = 3, /*������ֱ���豸���豸δ���ӵ�������*/
};

enum{
	DIT_TYPE,
	DIT_NAME,
	DIT_IPHONE_NEWEST_VERSION,
	DIT_ANDRIOD_NEWEST_VERSION,
	DIT_IPHONE_EN,     /*iphone english newest version description*/
	DIT_IPHONE_CH,     /*iphone chinese newest version description*/
	DIT_ANDRIOD_EN,   /*android english newest version description*/
	DIT_ANDRIOD_CH,   /*android chinese newest version description*/
	DIT_LOGINTYPE,      /*LT_NORMAL/LT_BIND/LT_UNBIND*/
	DIT_NETTYPE,          /*NT_SERVER/NT_DEVICE*/
	DIT_MAX
};

#ifndef CLIB_HDR
enum{
	CID_IPHONE = 0,
	CID_ANDROID = 1,
	CID_SYMBIAN = 2,
	CID_PC = 3,
	CID_MAX
};
#endif


#define DEF_WEB_DOMAIN "www.jiazhang007.com"
#define DEF_DISPATCHER_DOMAIN "www.jiazhang008.com"
#define DEF_APPLE_DOMAIN "www.apple.com"

/*���������������̶˿�*/
#define DEF_DISPATCHER_PORT 1180
/*�豸������������̶˿�*/
#define DEF_DEVSERVER_PORT 1181
/*Web������������̶˿�*/
#define DEF_WEBSERVER_PORT 1182
/* ���������UDP�����˿�*/
#define DEF_AGENT_UDP_PORT	1183
/*��ֵ������ȱʡ�˿�*/
#define DEF_CHARGE_SRV_PORT     1184
/*�豸������http�˿�*/
#define DEF_DEV_HTTP_PORT 880
/*Web������http�˿�*/
#define DEF_WEB_HTTP_PORT 80
/*DISPATCHER������http�˿�*/
#define DEF_DISPATCHER_HTTP_PORT 88
/* ����udp��echo_q�Ķ˿� */
#define DEF_DISPATCHER_UDP_PORT	1190
/* �豸�˷���udp echo_q�Ķ˿� */
#define DEF_DEV_UDP_PORT	1191
/* ͨ��UDP�����豸�Ķ˿ںţ��ṩ���ͻ��˵� */
#define	DFL_UDP_CTRL_CLIENT_WAN_PORT	1192
/* ���������豸�ṩ�����UDP�˿� */
#define	DFL_UDP_CTRL_DEV_PORT	1193
/* �豸��������UDP���ƵĶ˿� */
#define	DFL_UDP_CTRL_CLIENT_LAN_PORT	1194

#ifndef CLIB_HDR
/*С��������udp�˿ڣ��豸ֱ��С��������*/
#define CMT_PORT 5362
#endif

#define DSFTP_STDIN "/dsserver/dsftp-in"
#define DSFTP_PORT 2345

#undef	BIT
#define	BIT(n)	(1 << (n))
#define DEV_LIST_TMP_FILE "/var/tmp-device-%u"

/*ͨ��Э������
��ֹ�����������м����������
ֻ��˳����ӵ�����*/
enum {
	CMD_OK = 0,
	CMD_FAIL = 1,
	CMD_ECHO_Q = 2,
	CMD_ECHO_A = 3,
	CMD_AUTH_Q = 4,
	CMD_AUTH_A = 5,
	CMD_AUTH_K = 6,
	CMD_EXCHANG_Q = 7,
	CMD_EXCHANG_A = 8,
	CMD_USER_BIND_Q = 9,
	CMD_USER_BIND_A = 10,
	CMD_USER_AUTH_Q = 11,
	CMD_USER_AUTH_A = 12,
	CMD_USER_AUTH_K = 13,
	CMD_SERVER_Q = 14,
	CMD_SERVER_A = 15,
	CMD_DEV_Q = 16,
	CMD_DEV_A = 17,
	CMD_URL_Q = 18,
	CMD_URL_A = 19,
	CMD_DEV_CONFIG_Q = 20,
	CMD_DEV_CONFIG_A = 21,
	CMD_DEV_STAT_Q = 22,
	CMD_DEV_STAT_A = 23,
	CMD_DEV_STAT_CYCLE = 24,
	CMD_ELECT_MASTER_Q = 25,
	CMD_ELECT_MASTER_A = 26,
	CMD_ELECT_MASTER_C = 27,
	CMD_NICKNAME = 28,
	CMD_PASSWD = 29,
	CMD_LINK_OPTION_Q = 30,
	CMD_LINK_OPTION_A = 31,
	CMD_WEB_AUTH_Q = 32,
	CMD_WEB_AUTH_A = 33,
	CMD_NICKNAME_Q = 34,
	CMD_NICKNAME_A = 35,
	CMD_FTP_Q = 36,
	CMD_FTP_A = 37,
	CMD_YW_SERVER_Q = 38, /*��ά��ѯ����������*/
	CMD_KEEP_DATA = 39,
	CMD_NICKNAME_CONFIRM = 40, 	/*�޸��ǳƳɹ�ȷ������������ڲ�ʹ��*/
	CMD_NICKNAME_FAIL = 41,   	/*�޸��ǳ�ʧ������������ڲ�ʹ��*/
	CMD_LOAD_USER_INFO = 42,  	/*�����û�������Ϣ*/
	CMD_UDP_KEEP_DATA = 43,
	CMD_UDP_DEV_STAT = 44,
	CMD_SERVICE_DATE_Q = 45,  	/*�����ײͲ�ѯ����*/
	CMD_SERVICE_DATE_A = 46,  	/*�����ײͲ�ѯ��Ӧ*/
#if 0	
	CMD_SERVICE_CHARGE = 47,  	/* �����ײ͸���*/
#endif	
	CMD_URL_HIT_Q = 48, 		/*��ѯURL ��������*/
	CMD_URL_HIT_A = 49, 		/*��ѯURL ������Ӧ*/
	CMD_IPLOCATION_Q	= 50,	/*��ѯ�ͻ���IP��ַ��Χ*/
	CMD_IPLOCATION_A = 51,	/*dispatcher ��ѯ�ͻ���IP��ַ��Χ*/
	CMD_SELLER_LOGIN_Q	= 52,	/*������Ա��¼��֤����*/
	CMD_SELLER_LOGIN_A	= 53,	/*������Ա��¼��֤Ӧ��*/
	CMD_SELLER_USER_Q	= 54,	/*�û��˺Ų�ѯ����*/
	CMD_SELLER_USER_A	= 55,	/*�û��˺Ų�ѯӦ��*/
	CMD_SELLER_USER_ADD	= 56,	/*����˺�*/
	CMD_SELLER_USER_MODIFY = 57,/*�޸��˺�*/
	CMD_SELLER_USER_DEL	= 58,	/*ɾ���˺�*/
	CMD_VEDIO_Q = 59, /* ����鿴����ͷ��Ƶ */
	CMD_VEDIO_A = 60, /* ����ͷ��Ƶ��ϢӦ�� */
	CMD_SELLER_PWD = 61,	/*�޸��˺�����*/
	CMD_VIDEO_SYN_Q = 62,
	CMD_VIDEO_SYN_A = 63,
	CMD_VIDEO_HELLO_REQ = 64,
	CMD_VIDEO_HELLO =65,
	CMD_VIDEO_JPG = 66,
	CMD_VIDEO_ACK = 67,
	CMD_VIDEO_QUALITY = 68,
	CMD_VIDEO_STOP = 69,
	CMD_VEDIO_AGENT_A = 70,
	CMD_VEDIO_AGENT_Q = 71,
	CMD_VEDIO_AGENT_SETUP = 72,
	CMD_MISC_Q = 73,
	CMD_MISC_A = 74,
	CMD_UDP_YW_DISPATCH_Q = 78,//��ά--�ڷ����������-���豸����������-ȡ���豸����������ϸ�豸�б�
	CMD_LOAD_PACKAGE_LOG_Q = 79,//��ѯ��ֵ��־
	CMD_LOAD_PACKAGE_LOG_A = 80,//��ѯ��ֵ��־
	CMD_UDP_YW_DISPATCH_A = 81,////��ά--�ڷ����������-���豸����������
	CMD_ADD_RC_CARD_Q = 83,  /*��ӳ�ֵ������*/
	CMD_ADD_RC_CARD_A = 84,  /*��ӳ�ֵ����Ӧ*/
	#if 0
	CMD_MOD_RC_CARD_Q = 85 , /*���³�ֵ��״̬����*/
	CMD_MOD_RC_CARD_A = 86,  /*���³�ֵ��״̬��Ӧ*/
	CMD_DEL_RC_CARD_Q = 87,  /*ɾ����ֵ������*/
	CMD_DEL_RC_CARD_A = 88, /*ɾ����ֵ����Ӧ*/
	#endif
	CMD_TRANS_RC_CARD_Q = 89, /*ת�Ƴ�ֵ������*/
	CMD_TRANS_RC_CARD_A = 90, /*ת�Ƴ�ֵ����Ӧ*/
	CMD_QUERY_RC_CARD_Q = 91, /*��ѯ��ֵ��״̬����*/
	CMD_QUERY_RC_CARD_A = 92, /*��ѯ��ֵ��״̬��Ӧ*/
	CMD_CARD_CHARGE_Q = 93, /*��ֵ����*/
	CMD_CARD_CHARGE_A = 94, /*��ֵ��Ӧ*/
	CMD_DEV_CHARGE = 95,  /*��ֵ�ɹ�ͬ��֪ͨ����*/
	CMD_YW_SERVER_A = 96, /*��ά��ѯ��������Ӧ*/
	CMD_VEDIO_AGENT_OK = 97,/*����ɹ�*/
	CMD_ADD_CARD_TYPE_Q = 98,/*��ӿ���������*/
	CMD_ADD_CARD_TYPE_A = 99,/*��ӿ�������Ӧ*/
	CMD_QUERY_CARD_TYPE_Q = 100,/*��ѯ����������*/
	CMD_QUERY_CARD_TYPE_A = 101,/*��ѯ��������Ӧ*/
	CMD_QUERY_USER_PWD_Q = 102,/*ȡ�õ�ǰ��¼DEV����*/
	#if 0
	CMD_CARD_CHARGE_SYN_Q = 103,/*��ֵ��Ϣͬ������*/
	CMD_CARD_CHARGE_SYN_A = 104,/*��ֵ��Ϣͬ��Ӧ��*/
	CMD_CARD_TYPE_SYN_Q = 105,/*��ֵ������ͬ������*/
	CMD_CARD_TYPE_SYN_A = 106,/*��ֵ������ͬ��Ӧ��*/
	CMD_CARD_STATUS_SYN_Q = 107,/*��ֵ��״̬ͬ������*/
	CMD_CARD_STATUS_SYN_A = 108,/*��ֵ��״̬ͬ��Ӧ��*/
	CMD_DEV_CHARGE_CONFIRM = 109,/*��ֵ�ɹ�ͬ��֪ͨȷ������*/
	#endif
	CMD_YW_DEVSERVER_Q = 110,/*��ά����--�豸��������ѯ*/
	CMD_YW_DEVSERVER_A = 111,/*��ά����--�豸��������ѯ*/
	CMD_BIND = 112, /* �����豸���� */
	CMD_SET_NAME = 113, /* �����豸���� */
	CMD_MS_CTRL = 114,  /* �����豸�������������Ҫ���� */
	CMD_VIDEO_ROLL = 115, /* ������̨ת�� */
	CMD_FM_CONFIG_Q = 116,  /*���ü�ͥ������Ա����*/
	CMD_FM_CONFIG_A = 117,  /*���ü�ͥ������Ա��Ӧ*/
	CMD_FM_Q = 118,  /*��ѯ��ͥ������Ա����*/
	CMD_FM_A = 119,  /*��ѯ��ͥ������Ա��Ӧ*/
	CMD_MESURE_TRANS = 120, /*����������*/
	CMD_MESURE_Q = 121,      /*��ѯ�����������*/
	CMD_MESURE_A = 122,      /*��ѯ���������Ӧ*/
	CMD_MESURE_DEL = 123,   /*ɾ���������*/
	CMD_MESURE_TRANS_CONFIRM = 124, /*�豸��������ͬ����������ȷ��*/
	CMD_PLUG_TIMER_Q = 125,	/* ��ʱ���ص����� */
	CMD_PLUG_TIMER_A = 126,	/* ��ʱ���صĻ�Ӧ */
	CMD_USER_DEBUG = 127,     /*�ֻ��ϴ�������Ϣ*/
	CMD_IR_LIST = 128,		/*�ֻ�����֧���б�*/
	CMD_IR_CONFIG = 129,	/*�ֻ����úͻ�ȡ�����üҵ�*/
	CMD_IR_DB = 130,		/*�豸�ӷ�������ȡ�����*/
	CMD_IR_URL = 131,		/*�豸��ȡ����ң���б��URL */
	CMD_IR_CTRL= 132,		/* �ֻ����Ժ�������������Ч */
	CMD_REMOTE_CONFIG = 133, /* CMD_REMOTE_CONFIG */
	CMD_REMOTE_KEY_CONFIG = 134, /* �ֻ�Ϊ��ͥ��������һ������ */
	CMD_REMOTE_CODE = 135, /* �ֻ�Ϊ��������һ������ */
	CMD_REMOTE_CTRL= 136, /* �ֻ����ͥ�������Ϳ������� */
	CMD_REMOTE_STATE = 137, /* �ֻ��Ե�������״̬�Ĳ��� */
	CMD_VIDEO_TEMP_QUALITY = 138, /* ������̨����ʱ����ʱ���Ի��ʣ����������� */
	CMD_VIDEO_RECOR_CTRL = 139,/*�豸�˱���¼������------�Ѿ�ȡ���˱���*/
	CMD_ALARM_BIND_PHONE = 140,	/*�������Ű��ֻ�*/
	CMD_ALARM_MSG = 141,	/*������Ϣ��������*/
	CMD_FAIL_EX = 142,		/* cmd_fail��չ���� */
	CMD_ALARM_CONFIG_PHONE = 143,
	CMD_SLAVE_HIS = 144, /*���豸ͨ��������Ϣ�����豸*/
	CMD_GET_CMDOK = 145,	/* �豸�����������ѯ�ֻ���֤�ɹ�����Ҫ�������Ϣ */
	CMD_ALARM_LOG = 146,/*������־��ز���*/
	CMD_ALARM_LOG_SYNC = 147,/*������־��ͬ��*/
	CMD_TIME_SYNC = 148,/*�豸���ֻ���ѯʱ��ͬ��*/
	CMD_VTAP_LIST_Q = 149,/*��ѯ�豸¼���б�*/
	CMD_VTAP_LIST_A = 150,/*��Ӧ�豸¼���б�Ĳ�ѯ*/
	CMD_VTAP_Q = 151,/*�����豸¼���ļ�*/
	CMD_VTAP_KEEP = 152,/*¼��ۿ�����*/
	CMD_VTAP_A = 153,/*¼��������*/
	CMD_VTAP_TIMESTAP = 154, /*���󲥷�ָ��ʱ��*/

	CMD_REMOTE_BROADCAST = 155, /* �㲥������������豸 */
	CMD_VIDEO_SONIX = 156, /*��sonix ����*/
	CMD_REC_TIMER_Q = 157,/* ��Ƶ¼�ƶ�ʱ���ص����� */
	CMD_REC_TIMER_A  = 158,/* ��Ƶ¼�ƶ�ʱ���صĻ�Ӧ */
	CMD_VIDEO_CONTROL_ALARM_CFG = 159, /*��ⱨ��������Ϣ*/
	CMD_MS_DEV_INFO = 160, /*�����豸ͨ�Ÿ�֪�໥�İ汾��ʱ����֧�ֵĹ��� */

	CMD_DEV_SAHRD_INFO = 161, /*�豸���������������Ӧ��ͬ*/
	CMD_DEV_SAHRD_COUNT = 162, /*�豸���������������*/
	CMD_OPEN_TELNET = 163, /*�������豸telnet����*/
	CMD_ALARM_SWITCH_CTL = 164, /*���������ܿ��ؿ���*/
	CMD_VTAP_END = 165,/*�豸֪ͨ�ͻ��ˣ�¼�񲥷����*/
	CMD_VIDEO_QUALITY_V2 = 166,/* �Զ���ֱ������� */
	CMD_PLUG_ELECTRIC_STAT = 167,/*����ͳ�ƹ���֧�ֲ�ѯ����*/
	CMD_SCENE_CONFIG = 168,/*�龰ģʽ��������*/
	CMD_OPT_SCAN_Q = 169, /*ɨ��ˢ���豸����*/
	CMD_OPT_SCAN_A = 170, /*ɨ��ˢ���豸��Ӧ*/
	CMD_DEV_REG_Q = 171,  /*ˢ���豸ע������*/
	CMD_DEV_REG_A = 172, /*ˢ���豸ע����Ӧ*/
	CMD_ARIA2C_Q = 173, /*aria2c��������*/
	CMD_ARIA2C_A = 174, /*aria2c������Ӧ*/
	CMD_NETWORK_DETECT_Q = 175, /* ��Ƶ����̽������� */
	CMD_NETWORK_DETECT_A = 176, /* ��Ƶ����̽����Ӧ�� */
	CMD_SSIDPW = 177, /*�ֻ��޸��豸wifi SSID������*/
	CMD_DEVSERVER_LIST_Q = 178, /*��ȡ�豸�������б�����*/
	CMD_DEVSERVER_LIST_A = 179, /*��ȡ�豸�������б���Ӧ*/
	CMD_NET_PROBE_Q = 180,  /*�豸������������̽������*/
	CMD_NET_PROBE_A = 181,  /*�豸������������̽����Ӧ*/
	CMD_LOCATION_Q = 182,  /*�豸λ������*/
	CMD_LOCATION_A = 183,  /*�豸λ����Ӧ*/
	CMD_MACDENY = 184, /*���wifi mac��ֹ�б�*/
	CMD_MACALLOW = 185, /*ɾ��wifi mac��ֹ�б�*/
	CMD_BIND_SLAVE_INFO = 186,/*��ȡ���豸����Ϣ*/
	CMD_MASTER_SLAVE_CTRL = 187,/*����֮��Ŀ��������ͨ��������*/
	CMD_REBOOT = 188,/*Զ����������*/
	CMD_AREA_CONFIG = 189,/*�����������*/	
	CMD_STATIC_PIC_Q = 190,/*�����̬ͼƬ����*/
	CMD_STATIC_PIC_A = 191,/*�����̬ͼƬ��Ӧ*/
	CMD_POSITION_Q = 192, /*��λ��Ϣ����*/
	CMD_POSITION_A = 193, /*��λ��Ϣ��Ӧ*/
	CMD_SPEED_MAX_Q = 194,/*���ٷ�ֵ����*/
	CMD_SPEED_MAX_A = 195,/*���ٷ�ֵ��Ӧ*/
	CMD_SCHLBUS_BIND = 196,/*У����*/
	CMD_VOICE = 197,
	CMD_VOICE_ACK = 198,
	CMD_SPEEK_Q = 199,
	CMD_SPEEK_A = 200,
	CMD_VOICE_REG = 201,
	CMD_REMOTE_CODE_UPLOAD = 202,/*�ϴ������豸ѧϰ���Ŀ��Ʊ��뵽������*/
	CMD_RECORD_QUALITY_V2 = 203,/*¼����������*/
	CMD_NOTIFY_HELLO = 204, /* �豸��С���������������� */
	CMD_NOTIFY_HELLO_ACK = 205, /* �豸��С��������������Ӧ */
	CMD_NOTIFY = 206, /* ��Ϣ��������籨�������������� */
	CMD_NOTIFY_RESULT = 207, /* ��Ϣ����Ӧ�� */
	CMD_NOTIFY_EXPECT = 208, /* ��ϢID ͬ������*/
	CMD_NOTIFY_CENTER_LIST = 209, /* С�����������Լ�ip/port�ϱ��������������������͸��豸*/
	CMD_CMT_OP_DEVICE = 210, /* ��С����������豸���в���������ӡ�����net_cmt_op_device_t*/
	CMD_VOICE_PROMPT  = 211, /* ���ű��������ļ� */
	CMD_REMOTE_BD_BIND = 212,	/* ˫��RFЭ��󶨽�� */
	CMD_REMOTE_CONFIG_SOUNDLIGHT = 213,	/* ���ð������������������ⱨ���� */
	CMD_REMOTE_TD_CODE = 214, /* ������ά����� */
	CMD_PHONE_BIND_Q = 215, /*���ֻ������ύ������net_phone_bind_t*/
	CMD_PHONE_REQUESTLIST_Q =216, /*���ֻ������б��ѯ*/
	CMD_PHONE_REQUESTLIST_A =217, /*���ֻ������б���Ӧ������net_phone_bind_list_t*/
	CMD_PHONE_BIND_OPERATION = 218, /*�Ѱ��ֻ��԰��������������net_phone_bind_operation_t*/
	CMD_PHONE_BIND_RESULT = 219, /*�Ѱ��ֻ��԰�����������������net_phone_bind_result_t*/
	CMD_PHONE_BIND_DEL = 220, /*ɾ�����ֻ���net_phone_bind_uuid_t*/
	CMD_PHONE_UNBINDLOGIN_ALLOW = 221, /*����δ���ֻ���¼*/
	CMD_PHONE_UNBINDLOGIN_DENY = 222, /*��ֹδ���ֻ���¼*/
	CMD_PHONE_BINDLIST_Q = 223, /*���ֻ��б��ѯ����*/
	CMD_PHONE_BINDLIST_A = 224, /*���ֻ��б��ѯ��Ӧ������net_phone_bind_list_t*/
	CMD_SCENE_TIMER_Q  =  225,	/*�����龰ģʽ�Ķ�ʱ����������*/
	CMD_SCENE_TIMER_A  =  226,	/*�����龰ģʽ�Ķ�ʱ����Ӧ������*/
	CMD_SCENE_LINKAGE = 227,	/* �����龰�������� */
	CMD_PHONE_APN_OPERATION = 228, /*�����ֻ������������net_phone_push_t*/
	CMD_STATIC_PIC_Q_V2 = 229,/*�ڶ��汾�����̬ͼƬ����*/
	CMD_STATIC_PIC_A_V2 = 230,/*�ڶ��汾�����̬ͼƬ��Ӧ*/
	CMD_REMOTE_VTY = 231,/* ����Զ��telnetd���� */
	CMD_IA = 232,		/* ���ܼҾӵ�Ʒ���ƺͲ�ѯ */
	CMD_NEWUPVER_Q = 233,	/*�ֻ���ȡ�豸���������汾����*/
	CMD_NEWUPVER_A = 234,	/*�ֻ���ȡ�豸���������汾Ӧ��*/
	CMD_SET_NEWUPVER = 235,	/*�ֻ������豸�����汾*/
	CMD_NOTICE_DEVUP = 236,	/*�ֻ�֪ͨ�豸��������*/
	CMD_V4L2_COLOR = 237, /* v4l2 color������ѯ�Ϳ��� */
	CMD_NOTIFY_QUERY = 238, /* ��ѯ������Ϣ���󣬲���net_notify_query_t����ӦCMD_NOTIFY */
	CMD_MOTO_ATTRIBUTE = 239, /* IPC��̨����������� */
	CMD_MOTO_PRE_POSITION = 240, /* IPC��̨Ԥ��λ���� */
	CMD_MOTO_CRUISE = 241, /* IPC��̨Ѳ������ */
	CMD_UDP_AUTH = 242,
	CMD_UDP_KEEPLIVE = 243,
	CMD_UDP_CTRL = 244,
	CMD_UDP_NOTIFY = 245,
	CMD_UDP_BIND_PHONE = 246,
	CMD_RF2_CTRL = 247,
	CMD_CLOUD_MATCH = 248,	/* �ͻ��˺��豸������ƥ�佻���ı��� */
	CMD_CLOUD_MATCH_RESULT = 249,	/* �ͻ����������������ƥ���� */
	CMD_CLOUD_AC_LIB_INFO = 250,	/* �豸���������ȡ�ƿյ�����ժҪ���� */
	CMD_CLOUD_AC_LIB_DATA = 251,	/* �豸���������ȡ�ƿյ��������ݲ��� */
	CMD_805_CONFIG = 254, /*805�������ͷ���������*/
	CMD_UDP_APP_REPORT_RUNENV = 256,
	CMD_UDP_APP_REPORT_ERROR = 257,  
	CMD_APP_SERVER_DISP = 258,
	CMD_UDP_APP_REPORT_HABIT = 259,
    CMD_SHARE_REGISTER = 260,
	CMD_APP_USER = 261, /*�ֻ��û�ע�ᡢ��֤*/
	CMD_HOME_CONFIG = 262, /*��ͥ����*/
	CMD_HOME_SHARE = 263, /*��ͥ������Ա����*/
	CMD_LINKAGE_CONFIG = 264, /*��������*/
	CMD_TLV_UDP_CTRL = 265, /* ����CMD_UDP_CTRL, ֻ�����ݱ��TLV��ʽ*/
	CMD_PUSH_NOTIFY = 266,/*������֪ͨ����*/
	
	CMD_HOME_LABEL = 268,/*��ͥ��ǩ����*/
	CMD_HOME_DICTIONARY = 269,/*�ֵ�*/
	
	CMD_UDP_DNS_PROB = 301, /* �����豸ip��ַ��ȡ������������� */
	CMD_APP_DEV_USER = 302,	// APP�ͻ��ĺ�WIFI�豸ͨ��
	CMD_APP_LINKAGE_USER = 303,	// APP������������
	CMD_SERVER_WIFI_DEV_USER = 304,	// ��������WIFI�豸ͨ��
	CMD_SERVER_RF_DEV_USER = 305,	// ��������RF�豸ͨ��
	CMD_UDP_DONAME_PROB = 306, /* app̽�������������� ������������������������ø㣬������������*/
	//307����������
	CMD_QUERY_HISTORY = 308,/*��־��ѯ����*/
	CMD_WIDGET_KEY = 309,	/* ��ȡWIDGET��Կ���� */
	CMD_MAX
};
/*ͨ��Э������
��ֹ�����������м����������
ֻ��˳����ӵ�����*/


enum{
	ERR_NONE = 0,
	ERR_SN_INVALID = 1,    		/*DS007���к���Ч*/
	ERR_NICKNAME_INVALID = 2,	/*�û��ǳ���Ч*/
	ERR_PASSWD_INVALID = 3,   	/*�û��������*/
	ERR_CMD_INVALID = 4,     	/* ��Ч����*/
	ERR_PARAM_INVALID = 5, 		/*��Ч����*/
	ERR_MEMORY = 6,       		/*�������ڲ������ڴ�ʧ��*/
	ERR_SYSTEM = 7,       		/*�������ڲ�ϵͳ���ǵ���ʧ��*/
	ERR_NICKNAME_CONFLICT = 8,  /*�ǳƳ�ͻ*/
	ERR_NICKNAME_TOO_LONG =9, 	/*�ǳƹ���*/
	ERR_EMAIL_TOO_LONG = 10,    /*email��ַ����*/
	ERR_DATABASE = 11,			/*���ݿ����ʧ��*/
	ERR_CLIENT_VER = 12, 		/*�ֻ��ͻ��˰汾����*/
	ERR_DEV_OFFLINE = 13,		/* �豸���� */
	ERR_VEDIO_OFF = 14,		/* δ��������ͷ */
	ERR_DEV_SYS_ERR = 15,		/* �豸ϵͳ���� */
	ERR_SELLER_NAME_CONFLICT = 16,/*������Ա�û�����ͻ*/
	ERR_TOO_MANY = 17, /* ̫�����ڹۿ���Ƶ�� */
	ERR_PACKAGE_NAME_CONFLICT = 18, /* ̫�����ڹۿ���Ƶ�� */
	ERR_OUT_SERVICE = 19, /* ������ */
	ERR_CARD_SN_INVALID = 20, /*��ֵ�����к���Ч*/
	ERR_CARD_PWD_INVALID = 21, /*��ֵ��������Ч*/
	ERR_CARD_STATE_INVALID = 22, /*��ֵ��״̬��Ч*/
	ERR_CARD_NOTIME_TRANS = 23, /*�豸�޷������޿�ת��*/
	ERR_TIMEOUT = 24, /*��ʱʧ��*/
	ERR_AGENT = 25,		/* ����ʧ��*/
	ERR_EMAIL_INVALID =26, /*email��ַ��Ч*/
	ERR_FM_ID = 27,/* ��ͥ��ԱID��Ч */
	ERR_FM_LIMIT = 28, /* ��ͥ��Ա���ù��� */
	ERR_DEV_SYS_BUSY = 29, /* ϵͳæ��������������ϵͳ */
	ERR_PLUG_TIMER_LIMIT = 30, /* ��ʱ���ز������ø����Ѵﵽ��� */
	ERR_PLUG_TIMER_ID = 31, /* ��ʱ���ز���ID��Ч */
	ERR_REMOTE_LIMIT = 32, /* �ɿ��Ƶ��������Ѵ�������� */
	ERR_IR_DB_INVALID = 33, /* ����������� */
	ERR_REMOTE_BUTTON_LIMIT = 34, /* �ɿص��������ﵽ���� */
	ERR_REMOTE_ID_INVALID = 35, /* �ɿص���ID��Ч */
	ERR_REMOTE_KEY_ID_INVALID = 36, /* �ɿص���KEY ID��Ч */
	ERR_REMOTE_BUSY = 37, /* ������æ�����紦��ѧϰ״̬ */
	ERR_REMOTE_KEY_VALID = 38, /* ������ť��Ч */
	ERR_REMOTE_CODE_LEARN_FAILED = 39, /*ѧϰʧ��*/
	ERR_PHONE_NUM_EXCESS = 40,/*����֧�ֵ����绰����*/
	ERR_NO_BIND_PHONE = 41,/*������������δ���ֻ�*/
	ERR_DEV_UNLINK = 42,/*�豸δ�����豸������*/
	ERR_ALARM_PHONE_NOT_FOUNT = 43, /*�󶨵ı����ֻ��Ų�����*/
	ERR_ALARM_VIDEO_NOT_FOUNT = 44,/*û��ָ���ı���¼��*/
	ERR_ALARM_LOG = 45,/*������־��������*/
	ERR_ALARM_LOG_SYNC = 46,/*������־ͬ������*/
	ERR_REC_TIMER_LIMIT = 47,/*��Ƶ¼�ƶ�ʱ��:�Ѿ��ﵽ��������������*/
	ERR_REC_TIMER_OPT = 48,/*��Ƶ¼�ƶ�ʱ��:����ʧ��*/
	ERR_REC_TIMER_ID   = 49,/*��Ƶ¼�ƶ�ʱ��:��ʱ��id��Ч*/
	ERR_REC_TIMER_NTP = 50,/*ntp δͬ�� ���ʧ��*/
	ERR_REC_TIMER_DURATION = 51,/*ʱ��̫��*/
	ERR_NO_VTAP = 52,/*û����Ƶ¼���ļ�*/
	ERR_SLAVE_OFFLINE = 53, /* ���豸���� */
	ERR_DPI_FOR_PHONE = 54, /* �ֻ����߲�֧�ִ�ֱ��ʡ�֡�ʵ����� */
	ERR_CODE_ADJUST = 55, /* ��Ӧ���벻֧��΢�� */
	ERR_VTAP_CLIENT_EXCEED = 56, /*�ۿ�¼�����̫���ˣ�ͬʱֻ����1���˿�*/
	ERR_VTAP_DAMAGE = 57,/*¼���ļ���*/
	ERR_SCENE_VERSION = 58,/*�汾����ƥ��*/
	ERR_SCENE_ID	=59,/*�Ƿ�����id*/
	ERR_SCENE_FAIL	=60,/*ִ��ʧ��*/
	ERR_SCENE_ACTION	= 61,/*�����Ƿ�*/
	ERR_SCENE_ID_MAX	=62,/*����id�Ѵ�����*/
	ERR_SCENE_BUSY		=63,/*ִ��æ*/
	ERR_AREA_VERSION = 64,/*����汾����ƥ��*/
	ERR_AREA_ID	=65,/*�Ƿ�����id*/
	ERR_AREA_FAIL	=66,/*����ʧ��*/
	ERR_AREA_ACTION	= 67,/*�����Ƿ�*/
	ERR_AREA_ID_MAX	=68,/*����id�Ѵ�����*/
	ERR_AREA_ERR_OBJTYPE =69,/*����Ķ�������*/	
	ERR_NO_SD_DETECTED = 70, /* û�д洢�豸���� */
	ERR_NOT_SUPPORT = 71,/*�豸��֧��*/
	ERR_BUSY = 72,/*����������ͨ��*/
	ERR_REMOTE_NOT_SUPPORT = 73,/*���豸��֧�ֵ�������,Ҳ���Ǵ��豸֧��*/
	ERR_TF_NOT_INSERT = 74,/*TF��δ����*/
	ERR_REMOTE_INVALID_TD = 75,/* ��ӵ���δ֪�Ķ�ά����Ϣ */
	ERR_UNBIND =  76, /*�ǰ��û���ֹ��¼*/
	ERR_BIND_FULL =77, /*�ﵽ���������ƣ������������*/
	ERR_BINDLATE = 78, /*�Ѿ����˶԰�������д�����*/
	ERR_SCENE_TIMER_LIMIT = 79, /*�龰ģʽ��ʱ�����������*/
	ERR_SCENE_TIMER_ID = 80,	/*�龰ģʽ��ʱ����ID�Ų���*/
	ERR_SCENE_INVALID_ID = 81, /* ��������ʱ��Ч�ĳ���ID */
	ERR_SCENE_INVALID_REMOTE_ID = 82, /* ��������ʱ��Ч�ĵ���ID */
	ERR_UNBIND_WITH_DEV_OFFLINE = 83, /*δ�󶨵�¼ʧ�ܣ����豸���ߣ���Ҫ��ʾ�û������*/
	ERR_DUPLICATE_REMOTE_CODE = 84,	/* �ظ���Ӱ������� */
	ERR_IA_NOT_READY = 85, /*���ܵ�Ʒû��ʼ��*/
	ERR_IA_OPERATE_INVALID = 86, /*���ܵ�Ʒ������Ч*/
	ERR_UPGRADE_VER_EMPTY = 87, /*�ֻ���ѯ�豸����������ϢΪ��*/

	ERR_NEED_ENCRPYT = 88, /* ��Ҫ���ܣ�ȴ�޷�Э�̳�һ���ļ����㷨 */
	ERR_CLONE = 89,  /* �豸�ǿ�¡�� */
	ERR_WAIT_IR_TIMEOUT = 90,	/* ��ƥ��ʱ��ȴ����ⳬʱ */
	ERR_CLOUD_MATCH_FAILED = 91,		/* ��ƥ��ʧ�� */
	ERR_CLOUD_LIB_TIMEOUT = 92,	/* �ӷ�������ȡ����ʧ�� */
	ERR_CLOUD_NOT_READY = 93,	/* ������û�������� */
	ERR_CLOUD_MATCHING = 94,	/* ��ƥ�����ڽ��� */
	ERR_CLOUD_LIB_MISSING = 95,	/* ��ѯ�ı����ID������ */
	ERR_CLOUD_LIB_SET = 96,	/* �����Ƶ��������ʧ�� */
	ERR_SHORTCUT_ONOFF_UTC_PAST = 97,	/* ��ݿ�����Ϊ�ֻ�ʱ����������ʧ�� */
	ERR_SOFT_VER_LOW = 99, /* �ֻ��汾���� */
	ERR_OLD_EXPLICT = 100,/*��ʾ�Ƿ�����Ǩ���ֻ��˺�*/
	ERR_USER_EXIST = 102,/*�û��Ѵ���*/
	ERR_MAX /* ���ڴ�֮ǰ��Ӵ����� */
};

/*�ɿص�������*/
enum{
	REMOTE_TYPE_TV = 1,/*����*/
	REMOTE_TYPE_TVBOX = 2,/*������*/
	REMOTE_TYPE_AIRCONDITION = 3,/*�յ�*/
	REMOTE_TYPE_W_TV = 10,	/*WIFIת�������*/
	REMOTE_TYPE_W_TVBOX = 11,	/*WIFIת���������*/
	REMOTE_TYPE_W_AIRCONDITION = 12,	/*WIFIת����յ�*/
	REMOTE_TYPE_W_OTHER = 13,	/*WIFIת�������͵���*/

	REMOTE_TYPE_CLOUD_AIRCONDITION = 20,	/* �ƿյ� */
	REMOTE_TYPE_CLOUD_TV = 21,	/* �Ƶ��� */
	REMOTE_TYPE_CLOUD_STB = 22,	/* �ƻ����� */

	REMOTE_TYPE_WIRELESS_BASE = 128,
	REMOTE_TYPE_CURTAIN =129,/*����*/
	REMOTE_TYPE_DOOR = 130,/*��*/
	REMOTE_TYPE_WINDOW = 131,/*��*/
	REMOTE_TYPE_PLUG = 132,/*����*/
	REMOTE_TYPE_LAMP = 133,/*��*/
	REMOTE_TYPE_ALARM = 134,/*����*/
	REMOTE_TYPE_SOUNDLIGHT = 135,/*���ⱨ��*/
	REMOTE_TYPE_SCENE_CONTROLLER = 136,	/* �龰ң���� */
	REMOTE_TYPE_BD_LAMP = 150,	/* ˫��ư� */
	REMOTE_TYPE_BD_PLUG = 151,	/* ˫����� */
	REMOTE_TYPE_BD_CURTAIN = 152, /* ˫���� */
	REMOTE_TYPE_BD_ALARM = 153,	/* ˫�򱨾��� */
	REMOTE_TYPE_BD_DIMMING_LAMP = 154,	/* ˫������ */
	REMOTE_TYPE_BD_SOUND_LIGHT = 155,	/* ˫�����ⱨ���� */
	REMOTE_TYPE_DIY = 254,/*�Զ���*/
	REMOTE_TYPE_OTHER = 255/*����*/
};
/*��������*/
enum{
	CODE_TYPE_LEARN = 0,/*ͨ��ѧϰ�õ��ı���*/
	CODE_TYPE_CUSTOM = 1/*���ݵ���оƬ�����Զ���ı���*/
};

#define REMOTE_KEY_TEMP 	0x80000000	/* �յ��¶����� */
#define REMOTE_ALARM_MASK	0x40000000  /* ��������Ϊ�������� */
#define REMOTE_DEV_ABILITY_LEARN    0x1
#define REMOTE_DEV_ABILITY_VALID    0x2	/*������һ����Ч����ʱ�򣬲���λ*/

/* charge card sync operation type */
enum{
	OP_INSERT = 0,	/*����*/
	OP_UPDATE = 1,  /*����*/
	OP_QUERY =  2,  /*��ѯ*/
	OP_DELE = 3,   /*ɾ��*/
};

/* proto version control */
enum {
	PROTO_VER1 = 1,
	PROTO_VER2 = 2,
	/* further version add here */
};

#define PROTO_SUPPORT PROTO_VER2
#define PROTO_VERSION_INDEX(v)  ((v)-1)
#define PROTO_VERSION_VALID(v)	(((v) >= PROTO_VER1 && \
								(v) <= PROTO_SUPPORT) ? 1 : 0)

#define PROTO_MAX PROTO_VER2
#define MAX_PARAM_LEN (1024*1024*64)
#define SN_LEN 12	/*DS007�豸���к��ַ�������*/
#define MAX_NICKNAME 16  /*�û��ǳ���󳤶�*/
#define MAX_HOSTNAME 64 /*����û����ƣ�udp�豸��*/
#define MAX_PLUG_TIMER_NAME	64	/* ��ʱ����������󳤶� */
#define MAX_EMAIL 32
#define MAX_PHONE_SINGLE 16
#define MAX_PHONE_NUM 10
#define MAX_SL_NUM (MAX_REMOTE_NUM - 1)
#define MAX_PHONE (MAX_PHONE_SINGLE * MAX_PHONE_NUM)
#define MAX_WEB_ROOT 64
#define MAX_HANDLE 0xFFFFFFFF
#define MAX_SERVER_HANDLE 0xEE6B2800 /*��֤�豸���������������handle�����ظ�*/
#define MAX_SSIDPW_SSID_LEN 33
#define MAX_SSIDPW_PW_LEN 64

#pragma pack(push,1)

typedef struct{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	encrypt:1,
		compress:1,
		hlen:3,
		ver:3;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t ver:3,
		hlen:3,
		compress:1,
		encrypt:1;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t ds_type;
	u_int16_t command;
	u_int32_t param_len;
	u_int32_t handle;
}net_header_t;

#define  NHF_TRANSPARENT	0x01
#define  NHF_WAIT_REPLY	0x02

typedef struct {
	// hdr.ver = 2, hdr.hlen = sizeof(net_header_v2_t)/4
	net_header_t hdr;
	// NHF_XXX
	u_int8_t flags;
	// must be 0
	u_int8_t resv[3];
	u_int64_t sn;
} net_header_v2_t;

#define	net_hdr_len(net_hdr)	(((net_header_t *)net_hdr)->hlen<<2)
#define net_header_v2_size (sizeof(net_header_v2_t))

#define net_header_size (sizeof(net_header_t))
#ifdef CLIB_HDR
#define net_header_real_size(hdr) (((net_header_t *)hdr)->ver == PROTO_VER1 ? net_header_size : net_header_v2_size)
#else
#define net_header_real_size(v) ((v) == PROTO_VER1 ? net_header_size : net_header_v2_size)
#endif
#define	net_param_len(hdr) (((net_header_t *)hdr)->param_len)

/* ��ԭʼ���յ���Э��ͷ������¹���Э��ͷ��V2����(ע��SRC��V2��SN�ڽ���ʱ��ת�����ֽ���),�������v1��handle */
#define net_header_v2_wraper(dst, src) do {\
	memcpy((char*)(dst)+net_header_size, (char*)(src)+net_header_size, net_header_v2_size - net_header_size);\
	((net_header_v2_t*)(dst))->sn = ntoh_ll(((net_header_v2_t*)(src))->sn);\
	((net_header_t*)(dst))->handle = ((net_header_t*)(src))->handle;\
} while (0);

#ifndef CLIB_HDR
typedef struct{
	u_int32_t global_id;
	u_int32_t wan_ip[ISP_MAX];
	u_int32_t lan_ip;
	u_int16_t ds_port;
	u_int16_t http_port;
	u_int16_t ndev_now; /*Ŀǰ�ж���̨�豸�����ڸ÷�������*/
	u_int16_t ndev_next;/*���ܼ������ն��ٸ��豸����ǰ̨�������Լ�����һ�������ɶ�̬����*/
	u_int16_t asid;
	u_int8_t ds_type;
	u_int8_t isp;
	u_int8_t is_master; /*�����������Է����������Ч*/
	u_int8_t ds_action; /*DSAC_XXX*/
	u_int8_t can_encrypt;  /*ѹ�������������˶�Ϊtrueʱ����ѹ��*/
	u_int8_t can_compress /*���������������˶�Ϊtrueʱ��������*/;
	u_int8_t srv_status;
	u_int8_t is_linked; /*�������ӱ�־*/
	u_int8_t pad3;
	u_int8_t pad4;
}net_para_server_t;
#endif

typedef struct{
	u_int64_t sn;
	u_int32_t ip;
	u_int32_t local_ip;
	u_int16_t dev_flags; /* come from device, KF_XXX */
	u_int8_t ver_master; /* come from device */
	u_int8_t ver_slave; /* come from device */
	u_int8_t ver_build;
	u_int8_t pad2;
}net_para_device_t;

typedef struct{
	u_int32_t dev_server_id;
}net_dev_server_q;

typedef struct{
	u_int8_t file_name[32];//�ļ���
	u_int8_t file_content[0];//�ļ�����
}net_dev_list_file_name_t;

typedef struct{
	u_int8_t username[MAX_NICKNAME];
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t client_id; /*CID_IPHONE*/
	u_int8_t isp; /*client isp, available between servers*/
}net_para_user_bind_q;

typedef struct{
	u_int64_t sn;
	u_int32_t devserver_id;
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t is_udp;	/*�Ƿ���udp ctrl protocol*/
	u_int8_t flag;
}net_device_location_t;

typedef struct{
	u_int64_t sn;
	u_int16_t stat_len;
	u_int8_t stat_data[0];
}net_pkg_stat_t;

typedef struct{
	u_int32_t time;
	u_int8_t passwd[16];
}net_device_pwd_t;

typedef struct{
	u_int64_t sn;
	u_int32_t time;
	u_int8_t passwd[16];
}net_pkg_pwd_t;

typedef struct{
	u_int64_t dev_sn;
	u_int8_t password[33];
}net_query_user_pwd;

typedef struct{
	u_int64_t sn;
	u_int32_t callback_id;
	u_int8_t url[0];
}net_para_urlq_t;

typedef struct{
	u_int32_t callback_id;
	u_int8_t query_fail;
	u_int8_t hit_class;
	u_int8_t url_type;
	u_int8_t game_id;
}net_para_urla_t;

typedef struct{
	u_int64_t sn;
	u_int8_t md5[16];
}net_para_user_auth_t;

typedef struct{
	u_int64_t sn;
	u_int8_t md5[16];
	u_int8_t uuid[40];
	u_int8_t reserved[4];
}net_para_user_auth_v2_t;

typedef struct {
	u_int8_t uuid[40];
}net_phone_bind_uuid_t;

typedef struct{
	u_int8_t phone_number[16]; /*�ֻ�����*/
	u_int8_t phone_model[32]; /*�ֻ��ͺ�*/
	u_int8_t bind_name[16];/*������*/
	u_int8_t bind_uuid[40]; /*��uuid*/
	u_int8_t bind_message[40];/*������*/
	u_int8_t timestamp[20]; /*ʱ���2014-02-20 13:56:12*/
}net_phone_bind_t;

typedef struct {/*������*/
	u_int64_t sn;
	net_phone_bind_t phone;
	u_int8_t reserved[4];
}net_phone_bind_request_t;

typedef struct{/*�Ѱ��б���Ӧ���ݿ�t_bind_phone*/
	u_int8_t count; /*���б����*/
	u_int8_t unbind_login;
	u_int8_t reserved[2];
	net_phone_bind_t binds[0]; /*���б�*/
}net_phone_bind_list_t;

typedef struct{/*δ����������б���Ӧ���ݿ�t_bind_phone_q*/
	u_int8_t count; /*���б����*/
	u_int8_t reserved[3];
	net_phone_bind_t binds[0]; /*���б�*/
}net_phone_bind_q_list_t;


enum{
	BIND_ACTION_ACCEPT = 1, /*��׼��*/
	BIND_ACTION_DENY = 2 /*�ܾ���*/
};

typedef struct{
	u_int8_t action; /*BIND_ACTION_ACCEPT , BIND_ACTION_DENY*/
	u_int8_t reserved[3];
	u_int8_t request_uuid[40]; /*������uuid*/
	u_int8_t operation_uuid[40]; /*������uuid*/
}net_phone_bind_operation_t;


typedef struct{
	u_int32_t err_code; /*ERR_NONE, ERR_BINDLATE*/
	u_int8_t action; /*BIND_ACTION_ACCEPT , BIND_ACTION_DENY*/
	u_int8_t reserved[3];
	u_int8_t request_uuid[40]; /*������uuid*/
	net_phone_bind_t ratify_info; /*�����봦������Ϣ*/
	u_int8_t reserved1[4];
}net_phone_bind_result_t;

typedef struct{
	char ip[64];
	char location[128];
}net_para_iplocation_t;


typedef struct{
	u_int64_t sn;
	u_int8_t nickname_len;
	u_int8_t email_len;
	u_int8_t data[0];	/* ����ָ���ǳƺ�email��ַ������Ϊnickname_len+email_len */
}net_para_nickname_t;

/*CMD_NICKNAME_FAIL*/
typedef struct{
	u_int64_t sn;
	u_int32_t errorcode;
}net_nickname_fail_t;

/* CMD_WEB_AUTH_Q CMD_SELLER_LOGIN_Q */
typedef struct{
	u_int8_t name[MAX_NICKNAME];
	u_int8_t rand[8];
	u_int8_t md5[33];
}net_para_web_auth_t;

/* CMD_WEB_AUTH_Q CMD_SELLER_LOGIN_Q */
typedef struct{
	u_int8_t name[MAX_NICKNAME];
	u_int8_t rand[8];
	u_int8_t md5[33];
	u_int32_t ip; /*�û�������ͻ���IP*/
}net_para_web_auth_v1_t;


/* CMD_SELLER_MASTER_A */
typedef struct{
	u_int32_t master_ip;
	u_int16_t master_http_port;
}net_master_location_t;

/* CMD_SELLER_USER_Q */
typedef struct{
	u_int8_t role;
	u_int8_t is_username;
	u_int8_t is_creator;
	u_int8_t username[MAX_NICKNAME];
	u_int8_t creator[MAX_NICKNAME];
}net_seller_user_query_t;

#ifndef CLIB_HDR
/* CMD_SYS_LOG_Q */
typedef struct{
	u_int8_t oper_name[MAX_NICKNAME];
	u_int64_t dev_sn;
	u_int8_t old_service_time[MAX_DATE];
	u_int8_t service_time[MAX_DATE];
	u_int8_t old_service_type;
	u_int8_t service_type;
	u_int8_t oper_time[MAX_DATE];
}net_admin_charge_t;

/* CMD_SYS_LOG_Q */
typedef struct{
	u_int64_t dev_sn;
	u_int8_t  oper_time_start[MAX_DATE];
	u_int8_t  oper_time_end[MAX_DATE];
}net_query_admin_charge_log_t;//��ѯ����Ա�ֹ����ķ�������־
#endif


/* CMD_PACKAGE_ADD_QUERY_Q */
typedef struct{
	u_int8_t op_type;
	u_int8_t packe_name[33];
	u_int32_t package_money;
	u_int8_t service_time;
	u_int8_t service_unit[4];
	u_int8_t packe_desc[180];
	u_int8_t service_state;
}net_package_add_query_t;





/* CMD_SYS_LOG_Q */
typedef struct{
	u_int64_t dev_sn;//�豸���к�
	u_int64_t card_sn;//��ֵ����
	u_int8_t charge_time[20];//��ֵʱ��
	u_int8_t service_time[20];//����ʱ��
}net_charge_log_t;


/* CMD_SYS_LOG_Q */
typedef struct{
	u_int8_t oper_name[MAX_NICKNAME];
	u_int8_t oper_time_start[20];
	u_int8_t oper_time_end[20];
	u_int8_t oper_reuslt;
	u_int32_t curent_num;
	u_int32_t page_size;
}net_query_sys_log_t;

/* CMD_SYS_LOG_Q */
typedef struct{
	u_int8_t oper_name[MAX_NICKNAME];
	u_int8_t charge_time_start[20];
	u_int8_t charge_time_end[20];
	u_int64_t dev_sn;
	u_int64_t card_sn;
	u_int32_t curent_num;
	u_int32_t page_size;
}net_query_charge_log_t;


/* CMD_SELLER_LOGIN_A CMD_SELLER_USER_A CMD_SELLER_USER_MODIFY*/
typedef struct{
	u_int8_t username[MAX_NICKNAME];
	u_int8_t role;
	u_int32_t dispatchserver_id;
	u_int16_t dispatchserver_port;
}net_seller_user_info_t;

/* CMD_SELLER_USER_ADD */
typedef struct{
	net_seller_user_info_t user_info;
	u_int8_t pwd[33];
	u_int8_t creator[MAX_NICKNAME];
}net_seller_user_t;

/* CMD_SELLER_PWD */
typedef struct{
	u_int8_t username[MAX_NICKNAME];
	u_int8_t new_pwd[33];
}net_seller_pwd_t;

enum{
	FT_USERDB = 0, /*�û����ݿ⵼���ļ�*/
	FT_USERUP = 1, /*�û������޸���־�ļ�,�ѷ���*/
	FT_URLLIB = 2, /*URLʶ����ļ�*/
	FT_007BIN = 3, /*007�豸�����ļ�*/
	FT_DEVICE = 4, /*�豸�޸���־��¼�ļ�*/
	FT_MAX
};

enum{
	FTP_GET,
	FTP_PUT
};

typedef struct{
	u_int8_t ft_type;  /*Ҫ��ѯ�ļ����ͣ���FT_USERDB*/
	u_int32_t version; /*Ҫ��ѯ�ļ��汾��0��ʾȡ���°汾*/
}net_ftp_q;

typedef struct{
	u_int8_t ft_type;
	u_int32_t version;
	u_int32_t length;
	u_int32_t ip;
	u_int16_t port;
	u_int8_t filename[0];
}net_ftp_a;

typedef struct{
	u_int8_t ft_type;
	u_int8_t is_data; /*true��ʾdataΪͬ�����ݣ�����Ϊͬ���б�*/
	u_int8_t is_compress; /*true��ʾ����Ϊѹ��*/
	u_int8_t is_last;
	u_int32_t version;
	u_int32_t data_len;
	u_int8_t data[0];
}net_server_sync_t;

#define	MAX_VIDEO_HIS	 20

// �ۿ���Ƶ����ʷ��¼
typedef struct {
	// �ͻ���IP��ַ
	u_int32_t ip;
	// ��ʼ�ۿ�ʱ��
	u_int32_t begin;
	// ʱ��
	u_int32_t take;
} video_his_ele_t;

// ���λ�����
typedef struct {
	// ��һ���ڵ�
	u_int8_t begin;
	// �ж��ٸ���Ч�ڵ�
	u_int8_t num;
	// һ���ڵ�ṹ�����Ҫ���ڴӷ�����ȡ�غ�İ汾����
	u_int8_t struct_sz;
	// = MAX_VIDEO_HIS
	u_int8_t max_video_his;
	video_his_ele_t e[MAX_VIDEO_HIS];
} video_his_t;

typedef struct {
	video_his_t his;
} video_info_v0_t;


#define	KID_DEV_BUF	0
#define	KID_CFG_IF	1
#define	KID_CFG_GNET	2
#define	KID_CFG_OPEN	3
#define	KID_VIDEO_STAT	4
#define	KID_VIDEO_CFG	5
#define	KID_MAX	10

#define	KOP_WRITE	0
#define	KOP_READ	1

typedef struct{
	u_int64_t sn;
	u_int32_t time;
	u_int8_t kid;
	u_int8_t op;
	u_int16_t data_len;
	u_int8_t data[0];
}net_keep_data_t;

typedef struct{
	u_int32_t config_time;
	u_int16_t config_len;
	u_int8_t *config_data;
}net_device_config_t;

typedef struct{
	u_int64_t sn;
	u_int32_t server_id;
	u_int32_t time;
	u_int8_t kid;
	u_int8_t op;
	u_int16_t data_len;
	u_int8_t data[0];
}net_sync_keep_data;

typedef struct {
	u_int16_t year;
	u_int8_t month;
	u_int8_t day;
	u_int8_t hour;
	u_int8_t minute;
	u_int8_t second;
	u_int8_t resv;
	u_int32_t wan_ip;
}net_time_t;

#define ONE_HOUR_SECOND	(60*60)
#define ONE_DAY_SECOND (ONE_HOUR_SECOND*24)
#define ONE_YEAR_SEDOND (ONE_DAY_SECOND*365)
#define VIP_YEAR 5
#define VIP_SECOND (VIP_YEAR*ONE_YEAR_SEDOND)
#define ONE_DAY_MINS (24*60)
#define ONE_HOUR_MINS (60)
#define ONE_MIN_SECS (60)


#define SERVICE_TASTE_MASK 0x01  /*��������־λ,true��ʾ�Ѿ�����*/
#define SERVICE_VIP_MASK 0x02    /*�����ײͱ�־λ,true��ʾ�������ײ�*/
#define SERVICE_CHARGE_MASK 0x04 /*�Ѿ���ֵ��־λ*/

typedef struct {
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t df_flags;
	u_int8_t reserved;
	u_int8_t tlv[0];
}net_cmd_ok_t;

/*df_flagsΪ�豸״̬�꣬Ŀǰ֧�����±�־��*/
#define DF_ONLINE 0x01 /*�豸���߱�־��true��ʾ�豸����*/
#define DF_SERVICE_EXPIRED 0x02 /*�豸�����ڱ�־��true��ʾ�����ѵ���*/
#define DF_SERVICE_VIP 0x04 /*�豸VIP��־��true��ʾ��VIP�û�*/
#define DF_BELTER 0x08 /*��̩���������豸��־*/

typedef struct {
	u_int64_t sn;
	u_int8_t service_type;
	u_int8_t service_date[20];
	u_int32_t dev_type;//003/006/007
}net_service_date_t;

typedef struct {
	net_service_date_t v1;
	u_int16_t minute;
	u_int8_t times;
}net_service_date_v2_t;

// ���ѷ���
#define	MPF_MONEY	0x0001

typedef struct {
	u_int16_t mid;
	// MPF_XXX
	u_int16_t flags;
	// �����Ժ���չ��������дΪ0
	u_int32_t resv;
} mod_policy_t;

typedef struct {
	net_service_date_v2_t v2;
	u_int16_t mod_num;
	mod_policy_t mod[0];
} net_service_date_v3_t;

typedef struct{
	u_int32_t study;
	u_int32_t game;
	u_int32_t other;
	u_int32_t nolimit;
	u_int32_t black;
}net_url_hit_t;


typedef struct {
	u_int8_t md5[16];
	u_int32_t local_ip;
}net_auth_k_t;

typedef struct {
	u_int64_t sn;
	// ֻ�б����ɷ������ӿͻ����յ����ĺ���д
	u_int32_t callback_id;
	// TP_WEB_SRV/TP_USER/TP_DS007
	u_int8_t ds_type;
}net_vedio_q_t;

typedef struct {
	u_int64_t sn;
	// �������豸��net_vedio_q_t����ȡ����������
	u_int32_t callback_id;
	u_int32_t dev_global_ip;
	u_int32_t dev_local_ip;
	u_int16_t url_len;
	/*
		�����ӿ���ֱ�ӷ��ʣ�����
		http://132.68.7.69:1399/z87zbcde0918.html
	*/
	char url[0];
}net_vedio_a_t;

typedef struct {
	u_int32_t local_ip;
	u_int32_t global_ip;
	u_int16_t local_port;
	u_int16_t global_port;
	u_int8_t request_agent;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t encrypt_count:4,
			vtap_id:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t vtap_id:4,
		      encrypt_count:4;
#else
# error "Please fix <bits/endian.h>"
#endif	
	u_int8_t encrypt_method[0];	
}net_video_syn_q_t;

typedef struct {
	u_int32_t local_ip;
	u_int32_t global_ip;
	u_int16_t local_port;
	u_int16_t global_port;
	u_int8_t encrypt_method;
	u_int8_t ack_period;
	u_int8_t client_id;
	u_int8_t err_number;
	u_int8_t key_rand[16];
	u_int8_t auth_rand[16];
	u_int8_t md5[16];
}net_video_syn_a_t;

typedef struct {
	u_int32_t callback_id;
	u_int32_t dst_ip;
	u_int16_t dst_port;
	u_int16_t callback_port;
	u_int32_t callback_ip;
	u_int8_t reserved[4];
}net_vedio_hello_q_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t callback_ip;
	u_int16_t callback_port;
	u_int16_t reserved1;
	u_int32_t reserved2;
	u_int8_t client_id;
	u_int8_t ack;
	u_int8_t client_type;
	u_int8_t auth_len;
	u_int8_t auth[0];
}net_vedio_hello_t;

typedef struct {
	u_int16_t seq;
	u_int8_t pad_len;
	u_int8_t quality;
	u_int8_t data[0];
}net_vedio_jpg_t;

typedef struct {
	u_int16_t seq;
	u_int8_t pad_len;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t 	quality:4,
		ver:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t 	ver:4,
		quality:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t total;
	u_int8_t index;
	u_int16_t resv;
	// ʱ�������λΪ10����
	u_int32_t timestamp;
	u_int8_t data[0];
} net_video_jpg_v1_t;

typedef struct {
	u_int16_t last_seq;
	u_int16_t rcv_count;
	u_int16_t drop_count;
	u_int8_t client_id;
}net_vedio_ack_t;

typedef struct {
	net_vedio_ack_t v0;
	u_int8_t nack_number;
	u_int16_t nack_seq;
	u_int8_t nack[0];
} net_video_ack_v1_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t device_global_ip;
	u_int32_t user_global_ip;
	u_int16_t cer_length;
	u_int16_t reserve1;
	u_int32_t reserver2;
	u_int8_t  cert[0];
}net_vedio_agent_q_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t device_global_ip;
	u_int32_t user_global_ip;
	u_int16_t cer_length;
	// 0��ʾʧ�ܣ�����Ϊ����ʹ�õĶ˿�
	u_int16_t  device_global_port;
	u_int32_t reserver2;
	u_int8_t  cert[0];
}net_vedio_agent_a_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
	u_int32_t user_global_ip;
	u_int32_t device_global_ip;
	u_int16_t user_global_port;
	u_int16_t device_global_port;
}net_vedio_agent_setup_t;

typedef struct {
	u_int64_t sn;
	u_int32_t callback_id;
}net_vedio_agent_ok_t;

typedef struct {
	u_int64_t sn;
	u_int8_t dev_pass_md5[16];
}net_bind_v2_t;

typedef struct {
	u_int64_t sn;
	u_int16_t module_id;
	u_int8_t   name_len;
	u_int8_t   pad;
	u_int8_t  name[0];
}net_set_name_v2_t;

typedef struct {
	// �����Ҹ���0Ϊֹͣ
	char left_right;
	// �����¸�, 0Ϊֹͣ
	char up_down;
	u_int16_t resv;
} net_video_roll_t;

typedef struct {
	u_int8_t client_id;
	u_int8_t major_ver;
	u_int8_t minor_ver;
	u_int8_t resolved;
	u_int32_t info_len;
	u_int8_t info[0];
} net_user_debug_t;

typedef struct {
	u_int8_t cookie[16];
} network_detect_q_t;

typedef struct {
	u_int8_t cookie[16];
	// �ܹ���Ӧ���ٸ�����
	u_int16_t total;
	// ����ǵڼ�������0��ʼ����
	u_int16_t index;
	u_int8_t data[0];
} network_detect_a_t;

/* CMD_MISC_Q��MT_VIDEO_IP_LIST������ */
typedef struct {
	u_int32_t ip;
	u_int16_t port;
	u_int8_t type;
	u_int8_t priority;
} vil_ele_t;

typedef struct {
	u_int8_t count;
	// ÿ����Ա��С������4
	u_int8_t ele_size;
	u_int16_t resv;
	vil_ele_t ele[0];
} video_ip_list_t;


#define	MF_DEV_LAST	0
#define	MF_DEV_ONLY	1
#define	MF_SVR_ONLY	2

#define	MT_MK(mf, nr)	(((mf)&0xF) << 12 | ((nr)&0xFFF))
#define	MT_FLAG(v) (((v) >> 12) & 0xF)


enum {
	MT_AD =              MT_MK(MF_SVR_ONLY, 0),
	MT_TOTAL_FILTER =    MT_MK(MF_SVR_ONLY, 1)	,
	MF_SVR_ONLY_MAX
};

enum {
	MT_HIS_STAT =        MT_MK(MF_DEV_LAST, 0),
	MT_DEV_FILTER =      MT_MK(MF_DEV_LAST, 1) ,
	MF_DEV_LAST_MAX
};

/* ʹ��ö������Ҫȡ���ֵ�ϱ����ֻ� */
enum {
	MT_DEV_ONLINE =      MT_MK(MF_DEV_ONLY, 0),
	MT_CHILD_ONLINE =    MT_MK(MF_DEV_ONLY, 1),
	MT_REMAIN_TIME =     MT_MK(MF_DEV_ONLY, 2),
	MT_REMAIN_APP =      MT_MK(MF_DEV_ONLY, 3),
	MT_CUR_STAT	=        MT_MK(MF_DEV_ONLY, 4),
	MT_DEV_URL =         MT_MK(MF_DEV_ONLY, 5),
	MT_PLUG_AC =         MT_MK(MF_DEV_ONLY, 6),
	MT_PLUG_V =          MT_MK(MF_DEV_ONLY, 7),
	MT_PLUG_T =          MT_MK(MF_DEV_ONLY, 8),
	MT_PLUG_ON =         MT_MK(MF_DEV_ONLY, 9),
	MT_PLUG_SET_ON =     MT_MK(MF_DEV_ONLY, 10),
	MT_PLUG_SET_OFF =    MT_MK(MF_DEV_ONLY, 11),
	MT_SLAVE_LIST =      MT_MK(MF_DEV_ONLY, 12),
	MT_ENABLE_ITV =      MT_MK(MF_DEV_ONLY, 13),
	MT_DEV_MEM =         MT_MK(MF_DEV_ONLY, 14),
 	MT_DEV_CLIENT =      MT_MK(MF_DEV_ONLY, 15),
	MT_DEV_NI =          MT_MK(MF_DEV_ONLY, 16),
	MT_DEV_SSIDPW  =     MT_MK(MF_DEV_ONLY, 17),
 	MT_DEV_CPU =         MT_MK(MF_DEV_ONLY, 18),
	MT_DISABLE_ITV =     MT_MK(MF_DEV_ONLY, 19),
	MT_VENDOR_ID =       MT_MK(MF_DEV_ONLY, 20),
	MT_VENDOR_URL =      MT_MK(MF_DEV_ONLY, 21),
	MT_SLAVE_LIST_V2 =   MT_MK(MF_DEV_ONLY, 22),
	MT_DEV_VERSION =     MT_MK(MF_DEV_ONLY, 23),
	MT_DEV_RUN_TIME	=    MT_MK(MF_DEV_ONLY, 24),
	MT_DEV_ONLINE_TIME = MT_MK(MF_DEV_ONLY, 25),
	MT_DEV_CONNECT_INTERNET_TIME = MT_MK(MF_DEV_ONLY,26), /* wan����Ч��ʱ�� */
	MT_VIDEO_IP_LIST =   MT_MK(MF_DEV_ONLY, 27), 		 /* ���Կ���Ƶ�ĵ�ַ�˿��б� */
	MT_DEV_WIFI =        MT_MK(MF_DEV_ONLY, 28),
	MT_DEV_ENABLE_WIFI = MT_MK(MF_DEV_ONLY, 29),
	MT_DEV_DISABLE_WIFI= MT_MK(MF_DEV_ONLY, 30),
	MT_DEV_ARIA2c =      MT_MK(MF_DEV_ONLY, 31),
	MT_DEV_ENABLE_ARIA2c=MT_MK(MF_DEV_ONLY, 32),
	MT_DEV_DISABLE_ARIA2c=MT_MK(MF_DEV_ONLY,33),
	MT_DEV_MACDENY =     MT_MK(MF_DEV_ONLY, 34),
	MT_DEV_STORAGE_DEVICE=MT_MK(MF_DEV_ONLY,35),
	MT_DEV_MAXCMD_GET =   MT_MK(MF_DEV_ONLY,36),       /*�����ֻ���ѯ����misc�����������������*/
	MT_DEV_GET_EXSUBTYPE =   MT_MK(MF_DEV_LAST,37),       /*��ѯ�豸��չ�������������003Cam-YH 003Cam-G*/
	MT_DEV_SENSOR = MT_MK(MF_DEV_ONLY, 38),	/*008SH��������Ϣ*/	
	MT_DEV_GET_TMEP = MT_MK(MF_DEV_ONLY, 39),	/*��ѯ805M��ȡ���¶�ֵ*/
	MT_DEV_GET_RH = MT_MK(MF_DEV_ONLY, 40),	/*��ѯ805M��ȡ��ʪ��ֵ*/
	MT_DEV_GET_PM25 = MT_MK(MF_DEV_ONLY, 41),	/*��ѯ805M��ȡ��PM2.5ֵ*/
	MT_DEV_GET_VOC = MT_MK(MF_DEV_ONLY, 42),	/*��ѯ805M��ȡ��VOCֵ*/
	MF_DEV_ONLY_MAX
};


/* IP��ַ���� */
#define	IPT_UNKNOW	0
#define	IPT_LAN	1
#define	IPT_WAN	2
#define	IPT_MASTER	3
#define	IPT_UPNP	4
#define	IPT_GLOBAL	5

/* ��ͬ����IP��ַ��̽�����ȼ� */
#define	PRIO_LAN	10
#define	PRIO_WAN	20
#define	PRIO_MASTER	30
#define	PRIO_UPNP	50
#define	PRIO_GLOBAL	70

#define	MTF_USB_VIDEO_IN	0x01
#define	MTF_ITV_ENABLE	0x01
//��̩�����豸�Ƿ����
#define MTF_USB_BELTER_IN	0x01
// ֧�ֵ�����ѹ�¶�̽��
#define	MTF_VC_DETECT	0x01
//֧�ֵ���ͳ�ƹ���
#define	MTF_PLUG_ELECTRIC_STAT	0x02
// ֧��003��Ƶ¼��
#define MTF_VIDEO_RECORD  0x01
//֧����Ƶ��ת
#define MTF_VIDEO_FLIP  0x02
//֧����Ƶ���
#define MTF_VIDEO_DETECT  0x04
//֧���Զ���ֱ���
#define MTF_VIDEO_CUSTOM_DPI  0x08
//��֧����̨
#define MTF_VIDEO_UN_SUPPORT_PTZ 0x10

//֧��color��������
#define MTF_V4L2_COLOR  0x01

//֧����̨���Ե�һϵ������
#define MTF_MOTO_ATTRI_SPEED  0x01 //֧����̨��ת�ٶȵ���
#define MTF_MOTO_ATTRI_PRE_POSITION  0x02//֧����̨Ԥ��λ
#define MTF_MOTO_ATTRI_CRUISE  0x04//֧����̨Ѳ��

// IA������
#define MTF_IA_WATERHEATER_SUBTYPE_PROTO	0x00 //��ˮ��ԭ��(����� D40-HG7WF)
#define MTF_IA_WATERHEATER_SUBTYPE_A9		0x01 //ǰ����ˮ��

// ��Ҫ��ֵ����ʹ�õĹ���ģ��
#define MTF_MONEY	0x80
// ֧��ȡ���ֵ�ģ��
#define	MTF_RENAME	0x40
// ���߿���ѧϰ
#define MTF_REMOTE_RF_LEARN 0x01	
// �������ѧϰ
#define MTF_REMOTE_IR_LEARN 0x02	
// ����������һ������
#define MTF_REMOTE_VALID    0x04
// ֧�ֶ��빦��
#define MTF_REMOTE_GENCODE  0x08
// ֧������΢��
#define MTF_REMOTE_ADJUST 	0x10
// ֧�ֶ�����������
#define MTF_REMOTE_WIDTH   0x20
//֧��001E��ع���
#define MTF_REMOTE_001E 0x40
//֧�ֶ�ά����ӵ���
#define MTF_REMOTE_TD 0x80

/* ֧��һ��������һ������ */
#define MTF_ALARM_SWITCH	0x01

/* ����i+007Eʱ,��MID_SYSTEM��flag�������־λ��ʾ��WS-HC-1001�豸*/
#define MTF_SYSTEM_WSHC1001 0x01

// �豸ģ��ID����
#define	MID_DEV	0
// USB����ͷ
#define	MID_USB_VIDEO_MIN	1
#define	MID_USB_VIDEO_MAX	10
// ң�ؿ���
#define	MID_PLUG_MIN	11
#define	MID_PLUG_MAX	50
// ���������������
#define	MID_NET_ADMIN	60
// ��ɫ�������ҳ�����
#define	MID_GNET	100
// ��������ͷ
#define	MID_MJPG_VIDEO_MIN	101
#define	MID_MJPG_VIDEO_MAX	120
#define	MID_H264_VIDEO_MIN	121
#define	MID_H264_VIDEO_MAX	140
#define	MID_ITV	200
// ����ң�ط���
#define	MID_RCONTROL	210
//��̩���������豸
#define	MID_BELTER	280
// ����Eϵ�п���ģ��
#define MID_REMOTE 281
#define MID_ALARM   282
// TO DO: openwrt��֧����ʹ�øú꣬�´δ���ϲ�ʱ����
#if (CUSTOM_VENDOR_NAME == OPENWRT)
#define MID_REMOTE_DEV 283
#endif
//�龰ģʽ����֧��
#define MID_SCENE_SUPPORT	284
#define MFK_SCENE_TIMER    BIT(0)
#define MFK_SCENE_LINKAGE_ALARM BIT(1)

/*������֧��*/
#define AREA_FUNC_SUPPORT 285
#define AREA_ID_SUPPORT 286

#define MID_SYSTEM 287
#define MID_COLOR 288

#define MID_MOTO_ATTRIBUTE 289 //IPC��̨���Ե���֧�� 

#define MID_IA_ID 1003 // ia������


/* �������߿���ģ��ȡ����,�������1000������iphone�������������ͼ�� */
#define MID_REMOTE_IR 1001
#define MID_REMOTE_RF 1002

/* �����������˫��Ƭ�� */
#define MID_REMOTE_BD 290

#define	KF_UKEY_IN	BIT(0)
// ֧����Ƶ��������
#define	KF_VIDEO_AGENT_SUPPORT	BIT(1)
//upnpc �����ͨ�ϴ�������
#define	KF_UPNPC_SUPPORT 	BIT(2)

typedef struct {
	u_int32_t flags;
}net_keeplive_v1_t;

typedef struct {
	net_keeplive_v1_t v1;
	// �汾�ţ�m.s��ʽ���豸v0.7��ʼ֧��
	u_int16_t ver_master;
	u_int16_t ver_slave;
}net_keeplive_v2_t;

typedef struct {
	u_int64_t sn;
	u_int8_t passwd[16];
} net_bind_t;

typedef struct {
	u_int64_t sn;
	u_int16_t module_id;
	u_int8_t name_len;
	u_int8_t resv;
	u_int8_t name[0];
} net_set_name_t;

typedef struct {
	u_int8_t old_ssid_len;
	u_int8_t ssid_len;
	u_int8_t pw_len;
	u_int8_t data[0];
} ssidpw_opt_t;

typedef struct {
	u_int16_t num;
	u_int8_t mac[0];
} maclist_t;

#define SX_WOMEN 1 /*Ů*/
#define SX_MAN 0  /*��*/
#define CAREER_NORMAL 1 /*һ����Ա*/
#define CARRER_SPORT 2 /*�˶�Ա*/
#define CARRER_PROFESSIONAL 0 /*ְҵ��ʿ*/
#define AC_QUERY 0 /* ��ѯ */
#define AC_ADD 1 /*��Ӳ�����Ա*/
#define AC_MOD 2 /*�޸Ĳ�����Ա*/
#define AC_DEL 3  /*ɾ��������Ա*/


/*������Ϣ�������壬�ο���2.4G_HzͨѶЭ��v1.7.1.doc��*/
/*���������豸����*/
#define MDT_WEIGTH 0x01  /*���ؼ�*/
#define MDT_FAT 0x02      /*֬����*/
#define MDT_BLOOD_PRESSURE 0x03  /*Ѫѹ��*/
#define MDT_PEDOMETER 0x04      /*�Ʋ���*/
#define MDT_BLOOD_SUGAR 0x05   /*Ѫ����*/
#define MDT_BLOOD_OXYGN 0x06   /*Ѫ����*/
#define MDT_EAR_THERMOMETER 0x7 /* ����ǹ */

/*������������*/
#define MT_HELLO 0x01 /*���֣�Data:0x5a 0xa5 0x5a 0xa5  ��λ������Data�� 0xa5 0x5a 0xa5 0x5a)*/
#define MT_ERROR 0x02 /*������ʾ��Data������*/
#define MT_FB    0x03 /*������Ϣ��Data  0x01����ȷ�� 0x02������ 0x03���ط�����*/
#define MT_MESSURE_ING 0x04 /*������*/
#define MT_READY 0x05 /*׼���������Բ���(��ʼ����)*/
#define MT_POWER_OFF 0x06  /*�ػ��źţ��豸�ػ�ǰ����������*/
#define MT_MATCH_OK 0x07  /*����ɹ�*/
#define MT_POWER_LOW 0x08  /*�͵�ѹ�ź�*/
#define MT_MESSURE_STOP 0x09 /*ֹͣ����*/
#define MT_DATE 0x10   /*���ڣ��꣨2Byte�����£�1Byte�����գ�1Byte��*/
#define MT_TIME 0x11   /*ʱ�䣺Сʱ��1Byte�����֣�1Byte��*/
#define MT_WEIGHT 0x12  /*ʵʱ���أ�2Byte 0.0kg������λ��1Byte��0x00 kg��0x01 lb�� 0x02 st��*/
#define MT_USER 0x13    /*�û����ϣ�����1Byte�����1Byte���Ա�bit7Ů=1 ��=0��&ģʽ��bit6~0��������1Byte��0~255cm) ��*/
#define MT_FAT  0x14    /*֬����(2Byte 0.0%)*/
#define MT_WATER 0x15   /*ˮ�֣�2Byte 0.0%��*/
#define MT_MUSCLE 0x16  /*���⣨2Byte 0.0kg��*/
#define MT_BONE 0x17    /*������2Byte 0.0%��*/
#define MT_BMI 0x18     /*��������ָ����Body Mass Index),BMI(2Byte 0.0)*/
#define MT_BM 0x19     /*basic metabolism, ������лKCAl��2Byte 1KCAl��*/
#define MT_M 0x1A      /*metabolism���³´�лKCAl��2Byte 1KCAl��*/
#define MT_VISCERAL_FAT 0x1B  /*����֬���ȼ���2Byte��*/
#define MT_WEIGHT_CF 0x1C /*ȷ�����أ�2Byte 0.0kg������λ��1Byte��0x00 kg��0x01 lb�� 0x02 st������ȷ���ź��ڳ�LCD��˸ǰ����������*/
#define MT_PRESSURE_H 0x1D  /*��ѹֵ��2Byte 0mmHg������λ��1Byte��0x00 mmHg��0x01 kPa��*/
#define MT_PRESSURE_l 0x1E  /*��ѹֵ��2Byte 0mmHg������λ��1Byte��0x00 mmHg��0x01 kPa��*/
#define MT_PLUS 0x1F        /*��������1Byte ��*/
#define MT_WALK_CNT 0x20    /*�˶�����,��4Byte�������Ϊ99999����*/
#define MT_WALK_DISTANCE 0x21    /*�˶������4Byte��������Ϊ999.99kms��*/
#define MT_CALORIE 0x22     /*��������KCAL  4Byte��1 KCAL��*/
#define MT_BLOOD_SUGAR 0x23  /*Ѫ��ֵ��2Byte������28.5mmol/L ֱ�ӷ���285   ------Ѫ����*/
#define MT_BLOOD_OXYGN 0x24  /*---'Ѫ�����Ͷȣ�1Byte��                     ------Ѫ����*/
#define MT_BLOOD_PLUS  0x25  /*���ʣ�1Byte��                               ------Ѫ����*/

#define MT_PEDOMETER_1	0x26	/* �Ʋ�����һ������ */
#define MT_PEDOMETER_2	0x27	/* �Ʋ����ڶ������� */
#define MT_PEDOMETER_3	0x28	/* �Ʋ������������� */
#define MT_PEDOMETER_4	0x29	/* �Ʋ������������� */
#define MT_PEDOMETER_5	0x2a	/* �Ʋ������������� */
#define MT_PEDOMETER_6	0x2b	/* �Ʋ������������� */
#define MT_PEDOMETER_7	0x2c	/* �Ʋ������������� */

#define MT_EAR_TEMP			0x44 /* �¶� -- ����ǹ*/

/*��ͥ��Ա��Ϣ��Family Member info*/
typedef struct{
	u_int16_t    bd_year;   /*�����꣬����1980*/
	u_int16_t    weight;   /*���أ�kg*10*/
	u_int8_t    bd_month;  /*������, 1��12*/
	u_int8_t    height;     /*��ߣ�cm*/
	u_int8_t    sex;       /*�Ա�SX_WOMEN or SX_MAN*/
	u_int8_t    career;    /*ְҵ��CAREER_NORMAL */
	u_int8_t    step;      /*���࣬cm*/
	u_int8_t    id;        /*��ͥ��Աid���ɷ��������ɣ��ֻ��޸�/ɾ����ԱʱҪ��д*/
	u_int8_t    action;    /*AC_ADD*/
	u_int8_t    is_current;  /*true��ʾ��ǰ������Ա*/
	u_int8_t    reserved[4];  /*����*/
	u_int8_t    name[MAX_NICKNAME];
}net_fm_t;


/*������Ϣ���ݽṹ��Messure info*/
typedef struct{
	u_int8_t    mesure_data[4]; /*�������ݣ������������Ͷ���*/
	u_int8_t    mesure_type;   /*�������ͣ�MT_XXX*/
}messure_para_t;
typedef struct{
	u_int32_t    mtime;   /*����ʱ���*/
	u_int8_t    fm_id;   /*��������ͥ��Աid*/
	u_int8_t    md_id;     /*�����豸id��MDT_XXX*/
	u_int8_t    messure_cnt;     /*������������*/
	u_int8_t    reserved;      /*����*/
	messure_para_t  messure_para[0];	/*��������*/
}net_messure_t;

typedef struct{
	u_int8_t op;	/*MSYN_XX*/
	u_int64_t sn;
	u_int32_t server_id;
	net_messure_t data;
}net_sync_messure_t;

/*��ѯ��������������ݽṹ��Messure Query*/
typedef struct{
	u_int32_t    begin_time;  /*������ʼʱ�䣬0��ʾ������*/
	u_int32_t    end_time;   /*��������ʱ��0��ʾ������*/
	u_int8_t    fm_id;       /*������Աid��0��ʾ������*/
	u_int8_t    mdt;         /*�������ͣ�MDT_XXX��0��ʾ������*/
    u_int8_t    count;       /*��ѯ����������0��ʾ������*/
	u_int8_t    reserved[5];  /*����*/
}net_messure_q_t;

typedef struct {
	u_int8_t id;			/* ����ID */
	u_int8_t hours;			/* Сʱ 0-23 */
	u_int8_t minute;		/* ���� 0-59 */
	u_int8_t week;			/* bit 0-6λ��Ӧ����1�������� */
	u_int8_t enable;		/* �Ƿ���Ч(�ֻ�����) �����Ѿ���Ч(�豸����) */
	u_int8_t pad;			/* ���� */
	u_int16_t last;			/* �������(����) */
	u_int32_t errorcode;	/* ÿ�����ԵĴ�����Ϣ�� */
	u_int8_t name[MAX_PLUG_TIMER_NAME];		/* �������� */
} net_plug_timer_t;

#define	PTCF_EFFECT	(1<<15)
#define	PTCF_ON	(1<<14)

typedef struct {
	u_int8_t action;		/* ��ѯ����޸�ɾ�� AC_XXX */
	u_int8_t num;			/* ����Ƿǲ�ѯ����Ҫ���ϸ��� */
	u_int16_t next_action;	/* bit15: effect, bit14: on, bt13~bit0: minutes */
	u_int8_t pad[4]; 	/* �����Ժ�ʹ�� */
	net_plug_timer_t plug_timer[0];	/* ������ԣ�������num���� */
} net_plug_timer_config_t;

/*��ʱ¼����Ƶ������ݽṹ����*/
#define MAX_REC_TIMER_NAME MAX_PLUG_TIMER_NAME
typedef struct {
	u_int8_t id;	/* ����ID */
	u_int8_t wday; /* bit 0-6λ��Ӧ�����쵽������ */
	u_int8_t hours; /* Сʱ 0-23 */
	u_int8_t minute; /* ���� 0-59 */
	u_int8_t stat ; /* ���Ե�״̬ e_rec_item_stat_t */
	u_int8_t item_type;/* �������� ��e_rec_item_type_t����*/
	u_int16_t duration;/* �������(����) */
	u_int32_t location_time;/*��λʱ�䣬item_type = ePOLICY_ONCE��䣬����ִ��ʱ��*/
	u_int8_t name[MAX_REC_TIMER_NAME];		/* �������� */
}record_timer_item_t;

typedef struct {
	u_int8_t action;/* ��һ�β��������� */
	u_int8_t item_num;/* ��һ�β����Ĳ������� */
	u_int8_t switch_val;/* ¼��״̬�Ϳ��� */
	u_int8_t last_id;/* ����A���Ļ�����Ϣ����β�����id */
	u_int32_t err_code;/* ����A���Ļ�����Ϣ����һ�β����Ĵ�����Ϣ */
	u_int32_t pad;/* ��չ */
	record_timer_item_t item[0];/* ������ԣ�������num���� */
}net_record_timer_config_t;
#define 	MAX_REC_TIMER_NUM   31 /* ���Ե������� */

typedef enum {
	REC_STA_INIT = 0,//0: ����
	REC_STA_ERROR1,//1:����ʧ�ܻ���tf��δ�ҵ�
	REC_STA_ERROR2,//2:video����δ����
	REC_STA_READY,//3:׼������ffmpeg
	REC_STA_RUN,//4:ffmpeg�Ѿ�����
}e_record_status;


typedef enum {
	e_REC_QUERY,
	e_REC_ADD,
	e_REC_MOD,
	e_REC_DEL,
	e_REC_SWITCH
}e_rec_timer_action;

typedef enum {
	eITEM_INVALID,
	eITEM_DISABLE,
	eITEM_ENABLE
}e_rec_item_stat_t;

typedef enum {
	ePOLICY_ONCE,
	ePOLICY_WDAY
}e_rec_item_type_t;

/*008����ͳ��������ݽṹ*/
#define 		PLUG_ELECTRIC_STAT_CLS			0
#define 		PLUG_ELECTRIC_STAT_QUERY		1
#define 		PLUG_ELECTRIC_STAT_VERSION		0

typedef struct {
	u_int32_t electric_stat_total;/*�ӿ��������ڵ��ܵ���,��λ W*/
	u_int32_t electric_stat_section;/*��timeָ��ʱ�ĵ���ǰ�Ľ׶ε���,��λ W*/
	u_int32_t time;/*ָ����ĳһʱ��,�����Լ��㵽��ǰʱ���ۻ��ĵ���*/	
}plug_electric_stat_t;
typedef struct {
	u_int8_t version;/*�汾����*/
	u_int8_t action;/*ִ�ж���,����Ͳ�ѯ*/	
	u_int16_t pad;/*�ȷ����,������������*/
	u_int32_t err_code;/* ���ڱ��Ļ�����Ϣ����һ�β����Ĵ���������Ϣ */		
	plug_electric_stat_t item;/*ͳ������*/
}net_plug_electric_stat_t;


/*ң��������ݽṹ����*/

#define IR_LIST_DIR "soft/ir_config/"

typedef struct {
	u_int32_t id;
	u_int8_t name[256];	//����/tӢ��
}ir_list_member_t;

typedef struct {
	u_int32_t errorno;     	// �����
	u_int32_t classes_id;  	// ����ID
	u_int32_t brand_id;  	// ����ID
	u_int32_t ir_id;		// ����ң���豸id
	u_int32_t ver;			// ң�ذ����汾��
	u_int32_t n_member;
	ir_list_member_t member[0];
}ir_list_t;

typedef struct {
	u_int32_t err;
	u_int32_t ver;			// �汾��Ϣ
	u_int32_t ir_id; 		// Ҫң���ĸ�ң�ذ�
	u_int8_t name[128]; 	// �ĸ��������ֻ��˴�Ӣ�ĵ�����
}net_ir_test_t;

#define IR_CONFIG_ACT_QUERY 0
#define IR_CONFIG_ACT_ADD 1
#define IR_CONFIG_ACT_MODITY 2
#define IR_CONFIG_ACT_DEL 3

typedef struct {
	u_int32_t ir_id;		// ����ң���豸id
	u_int32_t action;	// 0:��ѯ 1: ��� 2: �޸� 3:ɾ��
	u_int32_t err;
	u_int8_t name[128];
} ir_config_t;

#define MAX_IR_DB_SEG 8000

typedef struct {
	u_int32_t errorno;
    u_int32_t ir_id;
    u_int32_t ver;
    u_int32_t compress_type;	// 0 Ϊ��ѹ�� 1 Ϊ gzip ѹ��
    u_int32_t index;
    u_int32_t db_len;
    u_int8_t  db[0];   			// ң�������ļ�
}ir_db_t;

typedef struct {
	u_int32_t err;
	u_int32_t version;
	u_int32_t url_len;
	u_int8_t  url[0];
}net_ir_list_url_t;

typedef struct{
	u_int16_t state_id;
	u_int16_t state_value;
}remote_state_t;

typedef struct{
	u_int16_t local_id;		// id for home���豸��д�������޸�
	// ����
	u_int8_t dev_type;		// REMOTE_TYPE_XXX����4.2�����ֻ���д�������޸�
	u_int8_t area_id;
	u_int32_t ability;		// ������һЩ��������4.5�����豸��д�������޸�
	u_int32_t ir_id;		// �������ݿ�ID
	u_int32_t factory_id;
	u_int32_t time_stamp_id;  // ����������ʱ��������豸��д�������޸�
	u_int8_t name[64];	// nick name
	u_int8_t ir_addr;	//����ת����ַ����ʱ��0
	u_int8_t alarm_flag;//0��Ч1��2�ر�
	u_int8_t pad;
	u_int8_t n_state;
	remote_state_t state[0];/*����ʱʹ�ã��޸ı��Ŀ��Ժ��Դ��ֶ�*/
}remote_atrri_t;

typedef struct{
	u_int16_t local_id;		// id for home���豸��д�������޸�
	// ����
	u_int8_t dev_type;		// REMOTE_TYPE_XXX����4.2�����ֻ���д�������޸�
	u_int8_t area_id;
	u_int32_t ability;		// ������һЩ��������4.5�����豸��д�������޸�
	u_int32_t ir_id;		// �������ݿ�ID
	u_int32_t factory_id;
	u_int32_t time_stamp_id;  // ����������ʱ��������豸��д�������޸�
	u_int8_t name[64];	// nick name
	u_int8_t ir_addr;	//����ת����ַ����ʱ��0
	u_int8_t alarm_flag;//0��Ч1��2�ر�
	u_int8_t pad;		// ����class_id����¼��ӵĵ������
	u_int8_t n_state;
	u_int64_t bind_sn;	// �󶨵Ĵ��豸���к�
	u_int8_t model_name[32];	//�ͺ�����:3D50A3700iD
	remote_state_t state[0];/*����ʱʹ�ã��޸ı��Ŀ��Ժ��Դ��ֶ�*/
}remote_atrri_v2_t;

#ifndef CLIB_HDR
typedef struct{
	u_int32_t err;
	u_int8_t action; // 0:��ѯ 1: ��� 2: �޸� 3:ɾ��
	u_int8_t version;
	u_int16_t count;	// remote_attri_t �ṹ�����
	remote_atrri_t ctrldev[0];	// remote_attri_t �ṹ��
}net_remote_t;
#endif

#define REMOTE_KEY_NAME_SIZE 64
typedef struct{
	u_int32_t key_id;		// ID ��λ������������ֵ,key_id ����Ϊ0
	u_int8_t valid;
	u_int8_t pad[3];
	u_int8_t name[REMOTE_KEY_NAME_SIZE];
}remote_key_attri_t;

typedef struct{
	u_int32_t err;
	u_int8_t action;  // 0:��ѯ 1: ��� 2: �޸� 3:ɾ��
	u_int8_t count;  // remote_key_attri_t �ṹ�����
	u_int16_t local_id;
	remote_key_attri_t attri[0];	// remote_key_attri_t �ṹ��
}net_remote_key_t;

typedef struct{
	u_int32_t err;
	u_int8_t action; // 0:ѧϰ 1:��� 2:ɾ�� 3:�޸� 4:���� 5:ѧ������ 6:ֹͣѧϰ
	u_int8_t time_out;	//ѧϰ��ʱʱ�䣬��λ:10��
	u_int16_t local_id;
	u_int32_t key_id;	// key_id Ϊ0��ʾֻ����״̬
	u_int16_t code_type;	//��д0,��ʾ��ǰ��ѹ���㷨 => �������ͣ����⡢����ɶ��
	u_int16_t code_len;
	u_int8_t code[0];
}net_remote_code_t;

typedef struct{
	u_int32_t err;
	u_int32_t key_id;
	u_int16_t local_id;
	u_int8_t repeat;	// �ظ���������
	u_int8_t state_num;
	remote_state_t states[0];
}net_remote_ctrl_t;

typedef struct{
	u_int32_t err;
	u_int8_t action;  // 0:��ѯ 1:��� 2:�޸� 3:ɾ��
	u_int8_t state_num;
	u_int16_t local_id;
	remote_state_t states[0];
}net_remote_state_t;

typedef struct {
	u_int16_t local_id;
	u_int16_t pad;
} remote_soundlight_list_t;

typedef struct {
	u_int32_t err;
	u_int8_t action;
	u_int8_t onoff;
	u_int8_t count;
	u_int8_t pad;
	u_int16_t local_id;
	u_int16_t timeout;
	remote_soundlight_list_t list[0];
} net_remote_config_soundlight; 

typedef struct {
	u_int32_t err;
	u_int8_t len;
	u_int8_t pad[3];
	u_int8_t value[0];
} net_remote_td_code;

/* CMD_MISC_A */
typedef struct {
	u_int16_t type;
	u_int16_t len;
	u_int8_t data[0];
} misc_hdr_t;

typedef struct {
	u_int64_t sn;
	u_int8_t serials_type;
	u_int8_t dev_type;
	u_int8_t status;
	u_int8_t mod_count;
	u_int16_t name_len;
	u_int16_t resv;
	char name[0];
} slave_hdr_t;

typedef struct {
	u_int16_t mod_id;
	u_int8_t name_len;
	u_int8_t flags;
	char name[0];
} module_hdr_t;


typedef struct{
	u_int32_t err;
	u_int8_t action;//0 ��ѯ1���2�޸�3ɾ��
	u_int8_t count;
	u_int8_t replace;
	u_int8_t pad;
	u_int8_t phones[0];
}net_alarm_config_phone_t;

typedef struct{
	u_int32_t error;
	u_int16_t local_id;
	u_int8_t action;//0��ѯ1���2�޸�
	u_int8_t count;
	u_int8_t phones[0];
}net_alarm_bind_phone_t;

typedef struct{
	u_int32_t error;
	u_int32_t key_id;
	u_int16_t local_id;
	u_int8_t pad[3];
	u_int8_t phone_count;
	u_int16_t msg_len;
	u_int8_t data[0];
}net_alarm_msg_t;

enum
{
	ALARM_LOG_ACT_QUERY = 0,
	ALARM_LOG_ACT_ADD = 1,
	ALARM_LOG_ACT_MOD = 2,
	ALARM_LOG_ACT_DEL = 3,
	ALARM_LOG_ACT_QUERY_VER = 4,
	ALARM_LOG_ACT_DEL_VIDEO = 5,
	ALARM_LOG_ACT_QUERY_VIDEO = 6,
	ALARM_LOG_ACT_PUSH = 7,
	ALARM_LOG_ACT_QUERY_UUIDS = 8,
};

#define MAX_ALARM_LOG_RECORD 100

typedef struct{
	u_int8_t version; /*�汾����0��ʼ*/
	u_int8_t action; /*
					0:��ѯ 
					1:��� ֻ�������ϴ���������
					3:ɾ�� 
					4: ��ѯ�������汾֧�� 
					5: ɾ�� ͬʱɾ�����¼��alarm_log_t����дID
					6: ��ѯ ¼���ļ��Ƿ���ڣ�alarm_log_t����дID
					7������ �����ػ��߷����������������ֻ���
					8: ��ѯ����UUID
					*/
	u_int16_t log_num; /*���ر�����Ϣ����*/
	u_int32_t err;
}alarm_log_head_t;

typedef struct{
	alarm_log_head_t alarm_header;
	u_int32_t query_start_time; /*��ѯ��¼��ʼʱ��*/
	u_int16_t query_start_index; /*�ӽ���е�N����¼���أ�����֧�ַ�ҳ*/
	u_int16_t query_want_num;/*ϣ��������־����� 0:������*/
}net_alarm_log_query_t;

typedef struct {
	u_int32_t log_uuid; /*��־ȫ��ID�����ǻ����ؾ�������������ѯ��־¼������,ɾ����ֻ����ֶ�, ɾ��ʱ:0:���������־*/
	u_int32_t alarm_create_time; /*������������ʱ��*/
	u_int32_t log_time; /*�����źŴ���ʱ��*/
	u_int16_t log_last_time; /*�����źų���ʱ�䣬��λ:��*/ 
	u_int16_t alarm_type; /*���������ͣ�Ŀǰ����134*/
	u_int16_t alarm_factoryid; /*�������������ͣ��ֻ�������ӵ���ʱ��д��*/
	u_int16_t local_id;/*�������ֲ�ID*/
	u_int8_t has_video; /*�Ƿ���¼�� 0:û�� 1:�� 2:δ֪ */
	u_int8_t alarm_name_len; /*�����������Ƴ��� �����ַ�����β����,��ֹ�û�ɾ�����ٿ���־*/
	u_int8_t alarm_msg_len;/*�û����õı�����Ϣ����, �����ַ�����β����*/
	u_int8_t alarm_phone_count;/*�����ֻ����б�*/
	u_int8_t alarm_name[0]; /* ������������,�������������Ϣ,��������ֻ����б�ÿ���ֻ���ռ16�ֽ�*/
} alarm_log_t;

#ifndef CLIB_HDR
typedef struct{
	alarm_log_head_t alarm_header;
	alarm_log_t log_data[0];
}net_alarm_log_t;

typedef struct{
	u_int8_t version; /*�汾����0��ʼ*/
	u_int8_t action; /*
					1:��� ֻ�������ϴ���������
					3:ɾ�� 
					5: ɾ�� ͬʱɾ�����¼��alarm_log_t����дID
					*/
	u_int16_t log_num; /*���ر�����Ϣ����*/
	u_int32_t confirm;
	u_int32_t err;
	alarm_log_t log_data[0];
}net_alarm_log_sync_t;
#endif
typedef struct {
	u_int32_t err;
	u_int32_t time_stamp;
}net_time_sync_t;


enum{
	ALARM_SWITCH_QUERY = 0,		/*��ѯ����������ѯ��alarm switch�Լ��豸�ظ�ʱʹ��*/
	ALARM_SWITCH_MODIFY = 1,	/*�л�alarm switch״̬*/
	ALARM_SWITCH_SYNC_QUERY = 2,/*���豸�����豸����ͬ�������*/
	ALARM_SWITCH_SYNC = 3,		/*���豸��󶨵Ĵ��豸�����ͬ��ͨ��*/
	ALARM_SWITCH_UNDEFINE
};
/*���������ܿ��ؿ���*/
typedef struct{
	u_int8_t action;	
	u_int8_t value;		/*�豸�ϵİ�����������״̬:0�رգ�1����*/
	u_int16_t pad;
	u_int8_t data[0];
}net_alarm_switch_ctl_t;

/* begin�ǻ�С�� */
/*
���ݽṹ�� net_notify_hello_t
CMD_NOTIFY_HELLO �������
���豸���͸�С��������������v2Э�飬
v2ͷsn��С��sn������mysn���豸�Լ�sn
*/
typedef struct{
	u_int64_t mysn;
	u_int8_t versiona;
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int8_t tlv_data[0];
}net_notify_hello_t;

/*
���ݽṹ�� net_notify_hello_ack_t
CMD_NOTIFY_HELLO_ACK �������
��С�����������͸��豸������v2Э�飬
v2ͷsn���豸sn������mysn��С���Լ�sn
*/
typedef struct{
	u_int64_t mysn;
	u_int8_t versiona;
	u_int8_t versionb;
	u_int8_t versionc;
	u_int8_t versiond;
	u_int32_t expect_report_id;
	u_int8_t hello_timer; /*����ʱ������ȱʡ10��*/
	u_int8_t reserved[3];
	u_int8_t tlv_data[0];
}net_notify_hello_ack_t;

#define NF_IS_PUSH 0x01		/*�ñ�־��ʾ�Ƿ������������͵ģ����ǲ�ѯӦ��*/
#define NF_HAS_MORE 0x02	/*��ʾ���滹����Ϣ*/
/*
���ݽṹ�� net_notify_t
CMD_NOTIFY �������
����v2Э�飬v2ͷsn��д�Լ�sn
*/
typedef struct{
	u_int32_t first_report_id;
	u_int8_t report_count;
	u_int8_t report_expire;		/*��Ϣ����ʱ�䣬��λ��*/
	u_int8_t report_nf_flag;	/*NF_IS_PUSH | NF_HAS_MORE*/
	u_int8_t reserved;
	u_int8_t tlv_data[0]; 		/*net_notify_tlv_t*/
}net_notify_t;

/*
���ݽṹ��net_notify_tlv_t
ͨ����ϢTLV��net_notify_t -> tlv_data
*/
typedef struct{
	u_int16_t type; /*NOTIFY_xxx such as NOTIFY_ALARM_LOG*/
	u_int32_t length;
	u_int8_t  value[0];
}net_notify_tlv_t;

enum{/* ͨ����Ϣ���Ͷ��� */
	NOTIFY_EMERGENCY = 0,		/* value��ʽ: net_notify_value_t*/
	NOTIFY_NORMAL = 1,		/*value��ʽ: net_notify_value_t*/
	NOTIFY_AD = 2,				/*value��ʽ: net_notify_value_t*/
	NOTIFY_ALARM_LOG = 3 ,	/* value��ʽ�ο�CMD_ALARM_LOG*/
	NOTIFY_MESURE_TRANS = 4,	/* value��ʽ�ο�CMD_MESURE_TRANS*/
	NOTIFY_FANS_SYSTEM = 5,	/* FansSRV system notify msg */
	NOTIFY_FANS_BUSINESS = 6,	/* FansSRV bussiness notify msg */
	NOTIFY_EMERGENCY_V2 = 7,		/* value��ʽ: net_notify_value_t*/
	NOTIFY_NORMAL_V2 = 8,		/*value��ʽ: net_notify_value_t*/
	NOTIFY_AD_V2 = 9,			/*value��ʽ: net_notify_value_t*/
};

#ifndef CLIB_HDR /* NOT in SDK */
enum{
	FMT_HTTP = 0, /* ��Ϣ��ʽΪhttp */
	FMT_URL = 1, /* ��Ϣ��ʽΪurl */
	FMT_STRING = 2,  /* ��Ϣ��ʽΪ�ַ��� */
};
#endif

/*
���ݽṹ�� net_notify_value_t
notify_tlv_t ��typeΪNOTIFY_EMERGENCY,NOTIFY_NORMAL,	NOTIFY_AD
ʱ��value��ʽΪ�����ݽṹ
*/
typedef struct{
	u_int32_t timestamp;
	u_int32_t len;
	u_int8_t notify_msg[0];
}net_notify_value_t;

/*
���ݽṹ�� net_notify_value_v2_t
notify_tlv_t ��typeΪNOTIFY_EMERGENCY_V2,NOTIFY_NORMAL_V2,	NOTIFY_AD_V2
ʱ��value��ʽΪ�����ݽṹ
*/
typedef struct{
	u_int32_t timestamp;
	u_int32_t len;
	u_int8_t title_len; /* ��ϢժҪ���� */
	u_int8_t msg_fmt; /* ��Ϣ��ʽ */
	u_int8_t pad1;
	u_int8_t pad2;
	u_int8_t notify_msg[0]; /* ��Ϣ��ժҪ */
}net_notify_value_v2_t;

/*
���ݽṹ�� net_notify_result_t
CMD_NOTIFY_RESULT �������
����v2Э�飬v2ͷsn�Լ�sn
*/
typedef struct{
	u_int32_t first_report_id;
	u_int8_t report_count;
	u_int8_t result;
	u_int8_t reserved[2];
}net_notify_result_t;

/*
���ݽṹ�� net_notify_expect_t
CMD_NOTIFY_EXPECT �������
����v2Э�飬v2ͷsn��д�Լ�sn
*/
typedef struct{
	u_int64_t sn;
	u_int32_t expect_report_id;
}net_notify_expect_t;

/*
���ݽṹ: net_notify_query_t
CMD_NOTIFY_QUERY�������
*/
typedef struct{
	u_int64_t sn; /*��Ϣ��Դ�����к�*/
	u_int64_t report_begin; /*��ѯ��Ϣ��ʼid��0��ʾ������*/
	u_int64_t report_end; /*��ѯ��Ϣ����id��0��ʾ������*/
	u_int8_t is_descending; /*0��ʾ�����ѯ��1��ʾ�����ѯ*/
	u_int8_t query_cnt; /*��ѯ����*/
	u_int8_t reserved[2]; /*����*/	
}net_notify_query_t;

/*
���ݽṹ�� net_notify_center_list_t
CMD_NOTIFY_CENTER_LIST �������
����v2Э�飬v2ͷsn��д�Լ�sn
1��ͨ�������С����ҵ�����Լ���ip/port�б��ϱ����������������������͸��豸
2���豸���½ӵ����������������Ѹ�С����������ҵ���������͸��豸
*/
typedef struct{
	u_int8_t count;
	u_int8_t element_size;
	u_int8_t reserved[2];
	u_int8_t element_data[0];/*community_center_info_t*/
}net_notify_center_list_t;

/*
���ݽṹ��net_srv_notify_center_list_t
CMD_NOTIFY_CENTER_LIST���豸������֮��֪ͨС��ip port�б����
upd���Ĵ�һ̨�豸���������͵�����һ̨�豸������
*/
#define SYNC_NOTIFY_CENTER_LIST 1
#define SYNC_CMT_ADD_DEVICE 2
#define SYNC_CMT_DEL_DEVICE 3
#define NOTIFY_SYNC_MAGIC 0x83f105ad
typedef struct{
	u_int32_t src_srv_id; /*source devserver id*/
	u_int32_t dst_srv_id; /*destination devserver id*/
	u_int32_t dst_ip;  /*destination devserver ip*/
	u_int32_t rand; /*random*/
	u_int32_t magic; /*random ^ NOTIFY_SYNC_MAGIC*/	
	u_int64_t cmt_sn; /*community sn*/
	u_int64_t dev_sn; /*device sn*/
	u_int8_t sync_type; /*SYNC_NOTIFY_CENTER_LIST, SYNC_CMT_ADD_DEVICE...*/
	u_int8_t reserved[3];
	u_int32_t context; 
	u_int8_t up_date[20];
}net_srv_notify_center_list_t;

/*
���ݽṹ�� community_center_info_t
С����ҵ��������Ϣ����Ӧnet_notify_center_list_t -> element_data
*/
typedef struct{
	u_int64_t sn;
	u_int32_t ip;  /*С����Ϣ���͸��ֻ�ʱ�����ֶα�ʾС�����report_id*/
	u_int16_t port;
	u_int8_t  reserved[2];
}community_center_info_t;

/*
���ݽṹ�� net_cmt_op_device_t
CMD_CMT_OP_DEVICE ����������豸��Ϣ����
*/
typedef struct{
	u_int64_t device_sn;
	u_int8_t action;            /* AC_ADD / AC_DEL / AC_MOD */
	u_int8_t len_phone;     /* �绰���볤�ȣ�����绰��'\t' ����*/
	u_int8_t len_name;      /*  �豸���Ƴ���*/
	u_int8_t result;        /* �������, ERR_NONE��ʾ�ɹ�*/
	u_int8_t op_data[0];     /* phone + name */
}net_cmt_op_device_t;

/*
���ݽṹ�� net_cmt_op_device_t
CMD_CMT_OP_DEVICE �������ͷ
*/
typedef struct{
	u_int32_t total;		/* �ܹ��ж���̨�豸*/
	u_int16_t current;       /* ��ǰ�����ж���̨�豸*/
	u_int8_t action;            /* AC_ADD / AC_DEL / AC_MOD */
	u_int8_t result;		
	u_int8_t op_device[0];   /* �豸��Ϣnet_cmt_op_device_t */
}net_cmt_op_device_hdr_t;

/* end�ǻ�С�� */
 
/*�ֻ�ֱ���豸ʱ�����豸δ���豸����������ʱ���豸����ذ��ṹ*/
typedef struct{
	u_int32_t cmd;	/*�ֻ����������*/
	u_int32_t error;
	u_int8_t pad[4];
}direct_link_reply_t;


/************��Ƶ�����߼����á���ѯ�����չ*************/
typedef struct {
	u_int8_t q;
} video_quality_v1_t;

enum{
	VIDEO_CUSTOM_READ = 0,
	VIDEO_CUSTOM_WRITE = 1,
};

/*
Ŀǰ֧�ֱַ���:320X240��640X480��1280X720
֡��:5��10��15��20��25
*/
typedef struct {
	video_quality_v1_t v1_hd;
	u_int8_t is_custom;	/*1�����Զ�������,��Ч�������漸��ֵ��0:�����ã���Ч��video_quality_v1_t*/
	u_int8_t action;		/*VIDEO_CUSTOM_READ/VIDEO_CUSTOM_WRITE*/
	u_int8_t fps;/*֡��*/
	u_int16_t width;/*�ֱ���:��,��:640*/
	u_int16_t height;/*�ֱ���:��,��:480*/
	u_int32_t errorno;
} video_quality_v2_t;


/************�����������ͨ��Э��ṹ*************/
//��sonix �����ľ���ѡ��
enum {
	CMD_SONIX_OK = 0,
	CMD_SONIX_FLIP = 1,		//��ת
	CMD_SONIX_SENSIT = 2,	//������
};

/*���ڴ������ʾ��������ֵ*/
#define CLIENT_SENSIT_LOW		(10)		//��
#define CLIENT_SENSIT_MIDDLE		(20)		//��
#define CLIENT_SENSIT_HIGH		(30)		//��

typedef struct {
	u_int32_t sensit;	//������
	u_int8_t vcr;		//��ʼ¼��
	u_int8_t message;	//������Ϣ
	u_int8_t sms;		//���Ͷ���
	u_int8_t warning;	//��������
}detect_t;

typedef struct {
	u_int8_t opt_type;			//opt_type=KOP_WRITE Ϊ���ò�����ֵΪ0����opt_type= KOP_READ Ϊ��ȡ������ֵΪ1����
	u_int8_t cmd;				//ָ����sonix �Ǹ����ܽ������á�	
	u_int8_t flip;				//������ת��1��ת��0����ת
	u_int8_t detect_enabled;	//������⿪��
	detect_t detect;			//�������
}sonix_opt_t;


/************������ⱨ�����ͨ��Э��ṹ*************/
#define MAX_ALARM_MSG_LEN 64
typedef struct {
	u_int8_t action;			/* 0 -- ��ѯ��1 --�޸� */
	u_int8_t count;			/* �������ֻ��� */
	u_int8_t pad[2];
	u_int8_t alarm_msg[MAX_ALARM_MSG_LEN];	/* ���źͶ��͵�����,���63���ֽ� */
	u_int8_t phones[0];			/* һ���ֻ�ʹ��16���ֽ� */
}vca_cfg_t;

#define VCA_QUERY 0
#define VCA_MODIFY 1
#define VCA_DEL 2
#define VCA_QUERY_Q 3	/* ��������ͨ�ţ�����dev_cfg���� */
#define VCA_QUERY_A 4   /* ��������ͨ�ţ�Ӧ��dev_cfg */


enum
{
	VTAP_FORMAT_MP4,
};

typedef struct {
	u_int32_t begin_time;
	u_int16_t duration;
	u_int8_t  video_format;
	u_int8_t pad;
}vtap_t;

typedef struct {
	u_int32_t begin_time;
	u_int16_t want_num;
	u_int8_t pad[2];
}net_vtap_list_query_t;

typedef struct {
	u_int16_t seg_num;//�б���ܺܳ��������ܳ��ȿ��ܳ���10K���������ջ���������˶Ա�//����Ľ��з�Ƭ����
	u_int16_t seg_index;//��0��ʼ
	u_int32_t num;
	u_int32_t err;
	vtap_t  tap_list[0];
}net_vtap_list_t;

typedef struct {
	u_int32_t err;
	u_int32_t begin_time;
	u_int16_t duration;
	u_int8_t vtap_id;
	u_int8_t pad;
}net_vtap_req_t;

typedef struct {
	u_int8_t client_id;
	u_int8_t pad[3];
}net_vtap_keepalive_t;

typedef struct {
	u_int32_t err;
	u_int8_t vtap_id;
	u_int8_t pad[3];
	u_int32_t timestap;//��λ10����
}net_vtap_timestap_t;

typedef struct {
	u_int32_t err;
	u_int8_t vtap_id;
	u_int8_t pad[3];
}net_vtap_end_t;

typedef struct {
	u_int16_t type;
	u_int16_t len;
	u_int8_t value[0];
}st_tlv;

#define DTYPE_SLAVE_SHARE_NAME (0x1)
#define DTYPE_SLAVE_DESC        (0x2)
#ifndef CLIB_HDR
typedef struct {
	u_int32_t shared_num;  /*�Ѿ������ķ���Ĵ���*/
	u_int16_t total_length; /*һ��slave��Ϣ�ĳ���*/
	u_int8_t dev_type; /*��ʾ����ͼ��ʹ��*/
	u_int8_t pad;
	u_int64_t dev_sn; /*����Ĵ��豸���к�*/
	u_int16_t province_id;
	u_int16_t town_id;
	u_int16_t category_id;
	u_int16_t pad2;
	st_tlv slave_info[0];
}slave_dev_info;

typedef struct {
	u_int8_t action; /*0:��ѯ 1.���� 2��ɾ��*/
	u_int8_t version; /*�汾����չ��*/
	u_int8_t slave_dev_count; /*���豸����*/
	u_int8_t pad;
	u_int32_t error; /*�����*/
	u_int64_t master_sn; /*���豸���*/
	slave_dev_info sdev_info[0]; /*����Ĵ��豸��Ϣ*/
}dev_shard_info;
#endif
typedef struct {
	u_int64_t dev_sn;
	u_int8_t dev_type;
	u_int8_t pad[3];
}dev_shard_count;

typedef struct {
	u_int64_t sn;
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t status;
	u_int8_t module_cnt;
	u_int16_t name_len;
	u_int16_t pad;
	u_int8_t name[0];
}misc_slave_list;

typedef struct {
	u_int16_t module_id;
	u_int8_t name_len;
	u_int8_t flags;
	u_int8_t name[0];
}misc_slave_module;

/* ��������ͨ������ʵ�ֵ��豸���� */
typedef struct code_obj_s{
	u_int16_t local_id;
	u_int8_t  key_num;
	u_int8_t  pad;
	u_int32_t key_ids[1];
}code_obj_t;

/* �������� */
typedef enum obj_type_e{
	OBJ_TYPE_SWITCH = 0x1,
	OBJ_TYPE_CODE,
}obj_type_t;

#define SCENE_MAX_NAME_LENGTH 64

/* �������Խṹ */
typedef struct scene_s{
	u_int8_t 	scene_id;		/* ����id,ע��,id�����5��ʼ,1-4��Ԥ��ռ�� */
	u_int8_t 	image_id;		/* ������Ӧ��ͼƬid */
	u_int8_t 	flag;/*����flag*/
	u_int8_t 	pad;
	u_int32_t create_time;	/* ��������ʱ�� */
	u_int8_t  name[SCENE_MAX_NAME_LENGTH]; /* �������� */
}scene_t;

#ifndef CLIB_HDR /* NOT in SDK */

/*�龰ģʽ�������ݽṹ*/
/* ֻ�п��ع��ܵ��豸���� */
typedef struct switch_obj_s{
	u_int64_t sn;			/* �豸sn�� */
	u_int16_t mode_id;	/* ģ��id��*/
	u_int8_t flag;		/*flag*/
	u_int8_t  action;		/* �豸���ܿ��� */
	u_int16_t  param_length; /* �����ĳ��� */
	u_int16_t pad;/*���*/
	u_int8_t  param[0];		/* ��������ֵ */
}switch_obj_t;

/* �¼� */
typedef struct event_s{
	u_int16_t  id;			/* �¼�id ,���豸��Ҫ��һ�޶�,Ŀǰ���900��*/
	u_int8_t	pad[2];
	u_int8_t	name[SCENE_MAX_NAME_LENGTH];
	u_int16_t	obj_type;		/*�¼������Ķ������ͣ�OBJ_TYPE_XXX*/
	u_int16_t obj_data_size;	/* �������ݽṹ�Ĵ�С */
	union{
		switch_obj_t switch_obj[0];
		code_obj_t code_obj[0];
	}obj;
}event_t;

/* ������������� */
typedef struct scene_config_s{
	u_int8_t version;		/* ����汾 */
	u_int8_t action;		/* ��������: 0;��ѯ�����龰,��Ҫ�¼���1:�޸ĺ���ӣ�2:ɾ��,3:��ѯĳ�龰ģʽ�������¼�, 9:ִ��*/
	u_int8_t  scene_id;	/* �龰id ,Ϊ0ʱ��ʾ���*/
	u_int8_t item_num; /* �龰��Ŀ����Ҫ�����ڲ�ѯ���������龰����ǰ�龰���¼���, 0��ʾ���������������ֵ */
	u_int32_t err;		/* Ӧ������룬��ȷ���ֵ��0 */
	union{
		scene_t scene[0];		/* ��ѯ�����龰ģʽʱ�ķ���ֵ */
		event_t event[0];		/* �����¼� */
	}item;
}scene_config_t;

/* �龰ģʽ���¼��Ĳ�������� */
typedef struct event_config_s{
	u_int8_t version;		/* ����汾 */
	u_int8_t action;		/* ��������: 0;��ѯ(Ŀǰֻ���������) */
	u_int16_t total;	/*���ص��¼�����*/
	u_int16_t	 item_num;   /* ����ʱ�������,��Ӧʱ����Ӧ����*/
	u_int16_t  index; /*��ѯ�����¼��Ŀ�ʼindex*/
	u_int32_t  err;		/* Ӧ������룬��ȷ���ֵ��0 */
	event_t event_item[0];
}event_config_t;

#else /* IN SDK */

typedef struct switch_obj_s{
	u_int64_t sn;			/* �豸sn�� */
	u_int16_t mode_id;	/* ģ��id��*/
	u_int8_t flag;		/*flag*/
	u_int8_t  action;		/* �豸���ܿ��� */
	u_int16_t  param_length; /* �����ĳ��� */
	u_int16_t pad;/*���*/
	//u_int8_t  param[0];		/* ��������ֵ */
}switch_obj_t;

/* ������������� */
typedef struct scene_config_s{
	u_int8_t version;		/* ����汾 */
	u_int8_t action;		/* ��������: 0;��ѯ�����龰,��Ҫ�¼���1:�޸ĺ���ӣ�2:ɾ��,3:��ѯĳ�龰ģʽ�������¼�, 9:ִ��*/
	u_int8_t  scene_id;	/* �龰id ,Ϊ0ʱ��ʾ���*/
	u_int8_t item_num; /* �龰��Ŀ����Ҫ�����ڲ�ѯ���������龰����ǰ�龰���¼���, 0��ʾ���������������ֵ */
	u_int32_t err;		/* Ӧ������룬��ȷ���ֵ��0 */
	scene_t scenes[0];
}scene_config_t;

typedef struct event_s{
	u_int16_t  id;			/* �¼�id ,���豸��Ҫ��һ�޶�,Ŀǰ���900��*/
	u_int8_t	pad[2];
	u_int8_t	name[SCENE_MAX_NAME_LENGTH];
	u_int16_t	obj_type;		/*�¼������Ķ������ͣ�OBJ_TYPE_XXX*/
	u_int16_t obj_data_size;	/* �������ݽṹ�Ĵ�С */
	union{
		switch_obj_t switch_obj;
		code_obj_t code_obj;
	}obj[0];
}event_t;

/* �龰ģʽ���¼��Ĳ�������� */
typedef struct event_config_s{
	u_int8_t version;		/* ����汾 */
	u_int8_t action;		/* ��������: 0;��ѯ(Ŀǰֻ���������) */
	u_int16_t total;	/*���ص��¼�����*/
	u_int16_t	 item_num;   /* ����ʱ�������,��Ӧʱ����Ӧ����*/
	u_int16_t  index; /*��ѯ�����¼��Ŀ�ʼindex*/
	u_int32_t  err;		/* Ӧ������룬��ȷ���ֵ��0 */
	// event_t event_item[0]; ��Ϊevent_t->obj.switch_obj������0��������������0��VC�±��벻��
}event_config_t;

#endif


typedef struct{
	u_int8_t member_count;
	u_int8_t member_size;
	u_int8_t pad[2];
}net_udp_echo_hd_t;

typedef struct {
	u_int64_t master_sn;
	u_int64_t slave_sn;
	u_int32_t uptime;
	u_int32_t networktime; 
	u_int32_t connect_time; 
	u_int8_t soft_major;
	u_int8_t soft_minor;
	u_int8_t soft_revison;
	u_int8_t pad1;
	u_int8_t update_major;
	u_int8_t update_minor;
	u_int8_t update_revison;
	u_int8_t pad2;
}net_udp_echo_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int64_t sn;
	char pad[4];
}devserver_list_q_t;

typedef struct{
	u_int32_t id;
	u_int32_t ip;
	u_int16_t port;
	u_int16_t net_delay;
	u_int16_t net_speed;
	u_int8_t isp;
	u_int8_t pad;
}devserver_info_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int32_t device_ip;
	u_int16_t device_port;
	u_int8_t devcie_isp;
	u_int8_t devserver_cnt;
	u_int8_t pad[4];
	devserver_info_t devservers[0];
}devserver_list_a_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int64_t sn;
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int16_t probe_size;
	u_int16_t probe_interval;
	u_int8_t probe_count;
	u_int8_t pad;
}net_probe_q_t;

typedef struct{
	u_int32_t magic;
	u_int32_t time;
	u_int32_t devserver_ip;
	u_int16_t devserver_port;
	u_int16_t probe_size;
	u_int16_t probe_interval;
	u_int8_t probe_count;
	u_int8_t probe_index;
	u_int8_t pad[4];
	u_int8_t probe_data[0];
}net_probe_a_t;

typedef struct{
	u_int64_t sn;
	u_int8_t devserver_cnt;
	u_int8_t pad[7];
	devserver_info_t devservers[0];	
}dev_location_t;

typedef struct {
	u_int64_t sn;			/*���豸��SN*/
	u_int32_t ip;			/*���豸��IP,���û����Ϊ0*/
	u_int8_t mac_addr[6];	/*���豸��MAC��ַ,�ù�����ʱδʵ����0*/
	u_int8_t serials_type;	/*���豸��serials_type*/
	u_int8_t dev_type;		/*���豸��dev_type*/
	u_int8_t bind_errno;	/*�󶨲��������Ĵ����,0δ����1�������,2�ѱ������豸��*/
	u_int8_t status;		/*0��δ�󶨣�1�����У�2�������ߣ�3��������*/
	u_int8_t pad[2];		/*���λ*/
	u_int64_t other_master_sn;/*bind_errnoΪ2ʱ��Ч����¼���󶨵����豸SN*/
}bind_slave_info_t;

enum {
	BIND_INFO_Q,	/*������豸�İ���Ϣ*/
	BIND_INFO_A		/*��Ӧ����*/
};

typedef struct {
	u_int8_t action;	/*ȡֵ������*/
	u_int8_t count;		/*actionΪBIND_INFO_Aʱ��Ч��ָʾ���ص�slave_bind_info_t����*/
	u_int8_t version;	/*�汾�ţ�ĿǰΪ0*/
	u_int8_t pad;		/*���λ*/
	u_int64_t sn;		/*���ָ��SN,�豸��ֻ����ָ��SN����Ϣ��Ϊ0ʱ��ʾ��ȡ���豸ɨ�赽�����д��豸��Ϣ*/
	u_int8_t data[0];	/*ָʾ�����豸���صĴ��豸��Ϣ��ʵ��ֵΪbind_slave_info_t*/
}net_bind_info_ctl_t;

enum {
	REBOOT_REQUEST,		/*��������*/
	REBOOT_ANSWER,		/*����Ӧ��*/
	REBOOT_REQUEST_FROM_MASTER	/*���豸������豸����*/
};

typedef struct {
	u_int8_t version;	/*�汾�ţ�ĿǰΪ0*/
	u_int8_t action;	/*0��ʾ�����ģ�1ΪӦ��*/
	u_int8_t flag;		/*����������豸Ϊ���豸�������flagֵΪ1�����ʾ���������豸�����⻹��Ҫ�������а󶨵Ĵ��豸*/
	u_int8_t pad;		/*���λ*/
	u_int8_t data[0];	
}net_reboot_t;
/************************************************������֧��*************************/
#define AREA_MAX_NAME_LENGTH 64

/* �������Խṹ */
typedef struct area_s{
	u_int8_t area_id;		/* ��1��ʼ��0��ʾ���*/
	u_int8_t image_id;		/* ������Ӧ��ͼƬid */
	u_int8_t flag;		
	u_int8_t pad; 
	u_int32_t create_time;	/* ��������ʱ�� */
	u_int8_t  name[AREA_MAX_NAME_LENGTH]; /* �������� */
}area_t;

/* �������� */
typedef enum area_obj_type_e{
	AREA_OBJ_TYPE_DEV = 0x1,
	AREA_OBJ_TYPE_REMOTE,
}area_obj_type_t;

/* ���豸���� */
typedef struct area_dev_obj_s{
	u_int64_t sn;			/* �豸sn�� */
}area_dev_obj_t;

/* ��������ͨ������ʵ�ֵ��豸���� */
typedef struct  area_remote_obj_s{
	u_int16_t local_id;
	u_int16_t  pad;
}area_remote_obj_t;

#ifndef CLIB_HDR /* NOT in SDK */

/* ���ݶ��� */
typedef struct area_obj_s{
   u_int8_t obj_type;
   u_int8_t pad;
   u_int16_t obj_data_len;
	union{
		area_dev_obj_t dev_obj[0];
		area_remote_obj_t  remote_obj[0];
	}obj_sub;
}area_obj_t;


/* ������������ */
typedef struct area_config_s{
	u_int8_t version;		/* ����汾 */
	u_int8_t action;		
	/* ��������: 
	0:��ѯ��������
	1:�޸ĺ��������
	2:ɾ������
	3:��ѯָ�������������¼���
	 */
	u_int8_t  area_id;	/*����id ,Ϊ0ʱ��ʾ���*/
	u_int8_t item_num;
 	/* ������Ŀ����Ҫ�����ڲ�ѯ����������������ǰ������豸�͵�����, 0��ʾ���������������ֵ */
	u_int32_t err;		/* Ӧ������룬��ȷ���ֵ��0 */
	union{
		area_t area[0];		/* ��ѯ��������ʱ�ķ���ֵ */
		area_obj_t obj[0];		/* �����豸�͵����б� */
	}item;
}area_config_t;

#else /* IN SDK */

/* ���ݶ��� */
typedef struct area_obj_s{
   u_int8_t obj_type;
   u_int8_t pad;
   u_int16_t obj_data_len;
	union{
		area_dev_obj_t dev_obj;
		area_remote_obj_t  remote_obj;
	}obj_sub[0];
}area_obj_t;

// SDK���õ����ݽṹ����Ҫ��VC�Ƚ��ϸ�
typedef struct area_config_s{
	u_int8_t version;		/* ����汾 */
	u_int8_t action;
	/* ��������:
     0:��ѯ��������
     1:�޸ĺ��������
     2:ɾ������
     3:��ѯָ�������������¼���
	 */
	u_int8_t  area_id;	/*����id ,Ϊ0ʱ��ʾ���*/
	u_int8_t item_num;
 	/* ������Ŀ����Ҫ�����ڲ�ѯ����������������ǰ������豸�͵�����, 0��ʾ���������������ֵ */
	u_int32_t err;		/* Ӧ������룬��ȷ���ֵ��0 */
	area_t areas[0];
}area_config_t;
#endif

typedef struct{
	u_int32_t client_ip;
	u_int16_t client_port;
	u_int8_t interval;
	u_int8_t reserved[5];
}static_pic_q_t;

typedef struct{
	u_int16_t width;
	u_int16_t height;
	u_int8_t pad_len;
	u_int8_t pic_type;
	u_int8_t reserved[6];
	u_int8_t pic_data[0];	
}static_pic_a_t;


typedef struct {
	u_int32_t ip;
	u_int16_t port;
	u_int8_t res[2];
} remote_vty_t;



typedef struct{
	u_int32_t begin_time;
	u_int32_t end_time;
	u_int16_t max_entry;
	u_int16_t reserved[2];
}gps_position_q_t;

typedef struct{
	u_int32_t longitude;
	u_int32_t latitude;
	u_int16_t speed;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t e_w:1,/*��������0��ʾ������1��ʾ����*/
		s_n:1, /*�ϱ�γ��0��ʾ��γ��1��ʾ��γ*/
		pad:6;
#elif __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t pad:6,
		s_n:1, /*�ϱ�γ��0��ʾ��γ��1��ʾ��γ*/
		e_w:1;/*��������0��ʾ������1��ʾ����*/
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t reserved;
	u_int32_t timesamp;
}gps_position_a_t;

#define SPEED_CTRL_SET 0 /*�������ٷ�ֵ*/
#define SPEED_CTRL_GET 1 /*��ȡ���ٷ�ֵ*/
#define SPEED_CTRL_CLEAR 2 /*ȡ������*/
typedef struct{
	u_int8_t maxspeed;
	u_int8_t ctrl; /*SPEED_CTRL_XXX*/
	u_int8_t reserved[2];
}speed_max_q_t;

typedef struct{
	u_int8_t maxspeed;
	u_int8_t error;
	u_int8_t reserved[2];
}speed_max_a_t;


#define BUS_NUM_MAX 16
#define DRIVER_PHONE 15
#define DRIVER_NAME 64

#define OP_BUSBIND_QUERY 0
#define OP_BUSBIND_SET 1
typedef struct{
	u_int8_t op;
	u_int8_t reserved;
	u_int16_t errorcode;
	u_int8_t bus_num[BUS_NUM_MAX];
	u_int8_t driver_name[DRIVER_NAME];//UTF-8
	u_int8_t driver_phone[DRIVER_PHONE];
	u_int8_t cam_count;
	u_int64_t cam_sn[0];
}schlbus_bind_t;

/********************** ����֧��  **********************/

typedef struct {
	u_int16_t seq;
	u_int8_t pad_len;
#if __BYTE_ORDER == __BIG_ENDIAN
	u_int8_t ver:4,
		rate:4;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	u_int8_t rate:4,
		ver:4;
#else
# error "Please fix <bits/endian.h>"
#endif
	u_int8_t total;
	u_int8_t index;
	u_int8_t format;
	u_int8_t client_id;
	// ������
	u_int8_t channel;
	// λ��
	u_int8_t bit;
	u_int16_t resv;
	u_int32_t timestamp;
	u_int8_t data[0];
} net_voice_t;

#define	AUDIO_FMT_PCM	0
#define	AUDIO_FMT_ILBC 1

#define	VOICE_RETE_8000	0
#define	VOICE_RETE_16000	1
#define	VOICE_RETE_32000	2
#define	VOICE_RETE_44100	3
#define	VOICE_RETE_48000	4

typedef struct {
	u_int8_t resv[6];
	u_int8_t client_id;
	u_int8_t nack_number;
	u_int16_t nack_seq;
	u_int8_t nack[0];
} net_voice_ack_t;

typedef struct {
	u_int8_t client_id;
	u_int8_t operate;
	u_int16_t resv;
} net_speek_q_t;

typedef struct {
	u_int32_t err_number;
	u_int8_t operate;
	u_int8_t timeout;
	u_int16_t resv;
} net_speek_a_t;

// �ͷŷ���Ȩ
#define	SPEEK_OP_RELEASE	0
// ���뷢��Ȩ
#define	SPEEK_OP_REQUEST	1
#define	SPEEK_OP_UNKNOW	0xFF

typedef struct {
	u_int32_t code_id;
	u_int32_t brand_id;		/*��������id*/
	u_int8_t model_name[32]; 	/*�����ͺ�����*/
	u_int8_t class_id;			/*�������id������ӣ������У��յ���*/
	u_int8_t flag;				/*�ϴ���־:�Զ��ϴ��ı�־Ϊ0,��̨�ֶ��ϴ�Ϊ1*/
	u_int16_t has_learn;				/*pad*/
	u_int32_t len;				/*�������볤��*/
	u_int8_t data[0];	
}net_remote_upload_t;

typedef struct {
	u_int32_t cmd_max;/*�����������*/
	u_int32_t misc_max;/*���misc����*/
}cmd_max_t;

typedef struct {
	u_int32_t err;
	u_int8_t action;	/* 1: �� 2:��� */
	u_int8_t pad;
	u_int16_t local_id;	/* ������ID */ 
} net_remote_bd_bind;
/*************************************************************************************/

/********************************dns ����****************************************************/
typedef struct dns_header //DNS���ݱ���
{
	u_int16_t id; //��ʶ��ͨ�����ͻ��˿��Խ�DNS��������Ӧ����ƥ�䣻
	u_int16_t flags; //��־��[ QR | opcode | AA| TC| RD| RA | zero | rcode ]
	u_int16_t quests; //������Ŀ��
	u_int16_t answers; //��Դ��¼��Ŀ��
	u_int16_t author; //��Ȩ��Դ��¼��Ŀ��
	u_int16_t addition; //������Դ��¼��Ŀ��
}DNS_HEADER; //��16λ�ı�־�У�QRλ�ж��ǲ�ѯ/��Ӧ���ģ�opcode�����ѯ���ͣ�AA�ж��Ƿ�Ϊ��Ȩ�ش�TC�ж��Ƿ�ɽضϣ�RD�ж��Ƿ������ݹ��ѯ��RA�ж��Ƿ�Ϊ���õݹ飬zero����Ϊ0��rcodeΪ�������ֶΡ�


typedef struct dns_query //DNS��ѯ���ݱ���
{
	u_int16_t type; //��ѯ���ͣ���Լ��20����ͬ������
	u_int16_t classes; //��ѯ��,ͨ����A��Ȳ�ѯIP��ַ��
}DNS_QUERY;

typedef struct dns_response //DNS��Ӧ���ݱ���
{
	u_int16_t name; //��ѯ������
	u_int16_t type; //��ѯ����
	u_int16_t classes; //������
	u_int32_t ttl; //����ʱ��
	u_int16_t length; //��Դ���ݳ���
}DNS_RESPONSE;
/*************************************************************************************/

/*����Ϊ�龰ģʽ��ʱ����ز��������籨��*/
enum {
	SCENE_TIMER_QUERY = 0,	/*��ѯ����龰ģʽ���ж�ʱ��*/
	SCENE_TIMER_ADD = 1,	/*���һ����ʱ��*/
	SCENE_TIMER_MODIFY = 2,	/*�޸Ķ�ʱ��������*/
	SCENE_TIMER_DEL = 3,	/*ɾ����ʱ��*/
	SCENE_TIMER_COVER = 4	/*���ǲ�����ɾ��֮ǰ�Ķ�ʱ�������ֻ���������Ϊ׼*/
};

typedef struct {
	u_int8_t id;			/*��ʱ��ID�����豸ͳһά��*/
	u_int8_t hour;			/*ÿСʱ*/
	u_int8_t minute;		/*ÿ����*/
	u_int8_t week;			/*ÿ��*/
	u_int8_t enable;		/*�Ƿ����øö�ʱ��*/
	u_int8_t pad[3];		/*���λ*/
	u_int8_t name[MAX_PLUG_TIMER_NAME];	/*��ʱ������*/
}net_scene_timer_t;

typedef struct {
	u_int32_t errorcode;		/*����ţ���ERR_SCENE_TIMER_LIMIT */
	u_int8_t version;			/*��0��ʼ��ĿǰΪ0*/
	u_int8_t action;			/*ȡֵ�������ö��ֵ��:SCENE_TIMER_QUERY*/
	u_int16_t scene_id;			/*�龰ģʽ�е�һ���龰��ID*/
	u_int8_t num;				/*�������timer����*/
	u_int8_t pad;				/*���λ*/
	u_int16_t next_execute_time;/*���龰�´ν�Ҫִ�е�ʱ������ڵ�ǰ��time_t��ֵ,����ж����ʱ���������Ҫִ�е�ʱ��*/
	net_scene_timer_t timer[0];	/*��ʱ����һ���龰�����ж����ʱ��*/	
}net_scene_timer_hd_t;

#define SCENE_LINKAGE_TYPE_ALARM 1

typedef struct {
	u_int16_t local_id;
	u_int16_t flag;
} scene_alarm_t;

#ifndef CLIB_HDR
typedef struct {
	u_int8_t scene_id;
	u_int8_t type; // SCENE_LINKAGE_TYPE_XXX
	u_int8_t len;
	u_int8_t pad;

	u_int8_t priv[0];	// scene_alarm_t
} scene_linkage_t;
#else /* IN SDK */
typedef struct {
	u_int8_t scene_id;
	u_int8_t type; // SCENE_LINKAGE_TYPE_XXX
	u_int8_t len;
	u_int8_t pad;
	//u_int8_t priv[0];	// scene_alarm_t
} scene_linkage_t;
#endif

typedef struct {
	u_int32_t err;
	u_int8_t action;
	u_int8_t count;
	u_int8_t query_type;
	u_int8_t pad;
	scene_linkage_t sl[0];
} net_scene_linkage_t;

typedef struct{
	u_int8_t action; /*AC_ADD AC_DEL*/
	u_int8_t msg_len; /*������ʾ���ݳ���*/
	u_int16_t token_len; /*token����*/
	u_int8_t need_push; /*�Ƿ��ģ�0��ʾ������*/
	u_int8_t reserved[3];
	u_int8_t  phone_ver[8];/*�ֻ�ϵͳ�汾����"7.1"*/
	u_int64_t sn;
	u_int8_t token[0]; /*token���ݣ����msg_len����0��token������msg����*/
}net_phone_push_t;

#define	MAX_OEM_STR	16
#define UP_DATE_LEN 12
enum {
	UPDESC_LANG_ZH = 1,
	UPDESC_LANG_EN = 2
};

typedef struct{
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t lang;		/*�����Խ������ԣ�1Ϊ���ģ�2ΪӢ��*/
	u_int8_t pad[4];
	u_int8_t oem_id[MAX_OEM_STR];
}net_newupver_q_t;

typedef struct{
	u_int8_t type;
	u_int8_t sub_type;
	u_int8_t ext_type;
	u_int8_t pad;
	u_int8_t oem_id[MAX_OEM_STR];
}net_set_newupver_t;

typedef struct{
	u_int8_t umajor;
	u_int8_t uminor;
	u_int8_t urevise;
	u_int8_t pad1;
	u_int8_t fmajor;
	u_int8_t fminor;
	u_int8_t frevise;
	u_int8_t pad2;
	u_int32_t errorcode;
	u_int8_t date[UP_DATE_LEN];/*�°汾����ʱ�䣬�磺2014-07-10*/
	u_int16_t desc_len;	/*�°汾�����������ȣ���󳤶�Ϊ1000��Ϊ0��ʾ���ṩ����������*/
	u_int16_t url_len;	/*�̼�����url��ַ���ȣ���󳤶�Ϊ1000��Ϊ0��ʾ���ṩurl��ַ*/
	/*
	desc	�°汾����������������net_newupver_q_t��lang����������Ϊutf-8����,����Ϊdesc_len
	url		��ʾ�û��̼�������url��ַ������Ϊurl_len
	*/
}net_newupver_a_t;


/* v4l2 color ������� */
#define V4L2_COLOR_VERSION_V1   0

typedef enum {
	e_V4L2_QUERY,
	e_V4L2_CTRL,
} e_v4l2_action_t;

typedef struct {
	u_int8_t ver;
	u_int8_t action; //������e_v4l2_action_t��ֵ
	u_int8_t pad[2];
	u_int32_t err;
	u_int32_t brightness_val;
	u_int32_t contrast_val;
	u_int32_t saturation_val;
	u_int32_t gain_val;
} net_v4l2_color_t;


/* ��̨����������� */
#define MOTO_ATTRI_VERSION_V1   0

typedef enum {
	e_MOTO_ATTRI_QUERY  =  0,
	e_MOTO_ATTRI_CTRL  =  1,
} e_moto_attri_action;

typedef struct net_moto_attri_s{
	u_int8_t ver;
	u_int8_t action; 
	u_int8_t pad[2];
	u_int32_t err;
	u_int8_t roll_speed;
	u_int8_t  resv8b;
	u_int16_t  resv16b;
	u_int32_t  resv32b;
} net_moto_attri_t;
/**/

/* ��̨Ԥ��λ������� */
#define MOTO_PREPOSITION_VERSION_V1   0

typedef enum {
	e_MOTO_PREPOSITION_CALL  =  0,
	e_MOTO_PREPOSITION_CONFIG  =  1,
} e_moto_prepostion_action;


typedef struct net_moto_prepostion_s{
	u_int8_t ver;
	u_int8_t action; 
	u_int8_t point_index;
	u_int8_t pad;
	u_int32_t err;
	u_int32_t  resv32b;
} net_moto_prepostion_t;
/**/

/* ��̨Ѳ��������� */
#define MOTO_CRUISE_VERSION_V1   0

typedef enum {
	e_MOTO_CRUISE_QUERY  =  0,
	e_MOTO_CRUISE_CTRL  =  1,
} e_moto_cruise_action;

typedef enum {
	e_MOTO_CRUISE_TYPE_NOT_ROLL  =  0,
	e_MOTO_CRUISE_TYPE_LR_ROLL  =  1,
	e_MOTO_CRUISE_TYPE_UD_ROLL  =  2,
	e_MOTO_CRUISE_TYPE_LINE_1_ROLL  =  3,
	e_MOTO_CRUISE_TYPE_LINE_2_ROLL  =  4,
	e_MOTO_CRUISE_TYPE_NOT_LR_ROLL  =  5,
	e_MOTO_CRUISE_TYPE_NOT_UD_ROLL  =  6,
} e_moto_cruise_type;

typedef struct prepos_point_node_s{
	u_int8_t  index;
	u_int8_t  point_index; 
	u_int16_t  stop_time;
} prepos_point_node_t;

typedef struct net_moto_cruise_s{
	u_int8_t ver;
	u_int8_t action; 
	/*Ѳ��������:�ر�Ѳ��������Ѳ��������Ѳ�����Զ���Ѳ����·һ���Զ���Ѳ����·��*/
	u_int8_t type;
	/*�����Զ���Ѳ��·��������Ԥ�õ�ռ�õĳ��Ⱥ�*/
	u_int8_t len; 
	u_int32_t err;
	u_int8_t line_1_point_num;
	u_int8_t line_2_point_num;
	u_int16_t  pad;
	prepos_point_node_t  prepos_point[0];
	
} net_moto_cruise_t;
/**/


enum {
	IA_ACTION_QUERY,
	IA_ACTION_CTRL
};

typedef struct {
	u_int16_t id;
	u_int16_t value;
} ia_status_t;

typedef struct {
	u_int32_t err;
	u_int8_t action;	//  IA_ACTION_XXX
	u_int8_t ns;
	u_int16_t pad;

	ia_status_t is[0];
} net_ia_t;

// ���ܼҾ�״̬ID16λ���ֳ������֣���8λ��ʾ���ͣ��������ڿյ�ң����ɶ�ģ���8λ��ʾ����ID
#define	IA_MK(type, nr)	(((type)&0xFF) << 8 | ((nr)&0xFF))
#define	IA_TYPE(type) (((type) >> 8) & 0xFF)

// ���ܼҾ�����
#define IA_TYPE_AIRCONDITION 1
#define IA_TYPE_WATERHEATER 2
#define IA_TYPE_AIRHEATER 3
#define IA_TYPE_AIRCLEANER 4
#define IA_TYPE_ELECTRICFAN 5
#define IA_TYPE_BATHROOMMASTER 6
#define	IA_TYPE_EB	7 /* E�� */
#define	IA_TYPE_EB_II	8 /* E��II */
#define	IA_TYPE_RFGW	9 /* 2.4G RF Gateway*/


//�����״̬ö��
typedef enum {
	IA_ELECTRICFAN_STATUS_WORK = IA_MK(IA_TYPE_ELECTRICFAN, 1),	// ����״̬: 0 ���� 1 ����
	IA_ELECTRICFAN_STATUS_GEAR = IA_MK(IA_TYPE_ELECTRICFAN, 2),	// ���� 1: ˯�ߵ� 2: �ͷ絵 3: �з絵 4: ǿ�絵
	IA_ELECTRICFAN_STATUS_TIMER = IA_MK(IA_TYPE_ELECTRICFAN, 3),// ���ö�ʱ��������Ϊ��λ
	IA_ELECTRICFAN_STATUS_SHAKE = IA_MK(IA_TYPE_ELECTRICFAN, 4),// ���Ȱ�ͷ 0 �ر� 1����
	IA_ELECTRICFAN_STATUS_POWER = IA_MK(IA_TYPE_ELECTRICFAN, 5), // ��ǰ����
} IA_ELECTRICFAN_STATUS_T;


// �յ�״̬ö��
typedef enum {
	IA_AIRCONDITION_STATUS_ONOFF = IA_MK(IA_TYPE_AIRCONDITION, 1),	// ����״̬ 0 ���� 1 ������
	IA_AIRCONDITION_STATUS_MODE = IA_MK(IA_TYPE_AIRCONDITION, 2),	// ģʽ
	IA_AIRCONDITION_STATUS_TEMP = IA_MK(IA_TYPE_AIRCONDITION, 3),	// �����¶�
	IA_AIRCONDITION_STATUS_POWER = IA_MK(IA_TYPE_AIRCONDITION, 4),	// ��ʾ����
	IA_AIRCONDITION_STATUS_TIMER = IA_MK(IA_TYPE_AIRCONDITION, 5),	// ��ʱ����
	IA_AIRCONDITION_STATUS_CUR_TEMP = IA_MK(IA_TYPE_AIRCONDITION, 6),	// ��ǰ�¶�
} IA_AIRCONDITION_STATUS_T;

/* 		��ˮ��״̬ö�� 		*/
typedef enum {
	IA_WATERHEATER_STATUS_WORK = IA_MK(IA_TYPE_WATERHEATER, 1),			// ����״̬ 0 ���� 1 ������
	IA_WATERHEATER_STATUS_TEMP_SET = IA_MK(IA_TYPE_WATERHEATER, 2),		// ����ˮ�� ��Χ35-75
	IA_WATERHEATER_STATUS_TEMP_CURRENT = IA_MK(IA_TYPE_WATERHEATER, 3),	// ��ǰˮ��
	IA_WATERHEATER_STATUS_TIMER = IA_MK(IA_TYPE_WATERHEATER, 4),	// ԤԼ��ʱ����(ʣ��)
	IA_WATERHEATER_STATUS_CAPACTITY = IA_MK(IA_TYPE_WATERHEATER, 5), // ���� 1 �뵨 2 ȫ��
	IA_WATERHEATER_STATUS_POWER	= IA_MK(IA_TYPE_WATERHEATER, 6),	// ����
} IA_WATERHEATER_STATUS_T;

// ��ˮ��״̬ö����ֵ����
typedef enum {
	IA_WATERHEATER_STATUS_WORK_VALUE_SLEEP,
	IA_WATERHEATER_STATUS_WORK_VALUE_WORK,
} IA_WATERHEATER_STATUS_WORK_VALUE_T;

typedef enum {
	IA_WATERHEATER_A9_STATUS_TEMP_SET = IA_MK(IA_TYPE_WATERHEATER, 1),			// ����ˮ�� 35-65
	IA_WATERHEATER_A9_STATUS_TEMP_CURRENT = IA_MK(IA_TYPE_WATERHEATER, 2),		// ��ǰˮ��
	IA_WATERHEATER_A9_STATUS_MODE = IA_MK(IA_TYPE_WATERHEATER, 3),	// ����: 1 �Զ� 2 ԡ�� 3 ϴ�� 4 ϴ�� 5 ϴ��
	IA_WATERHEATER_A9_STATUS_WORK = IA_MK(IA_TYPE_WATERHEATER, 4),	// ����״̬ bit ��ʾ bit0 ����Ƿ��� 1 ˮ���Ƿ��� 2 �Ƿ�ȼ�� 3 �Ƿ���С 4 �Ƿ����� 
	IA_WATERHEATER_A9_STATUS_FIRE_LEVEL = IA_MK(IA_TYPE_WATERHEATER, 5), // ȼ�շֶ� 0 ��ȼ�� 1 ��ȻȻ 2 ��ȼ�� 3 ȫȼ��
	IA_WATERHEATER_A9_STATUS_COUNT	= IA_MK(IA_TYPE_WATERHEATER, 6),	// �ۼ�ˮ��������λL
	IA_WATERHEATER_A9_STATUS_GAS	= IA_MK(IA_TYPE_WATERHEATER, 7),	// ������
} IA_WATERHEATER_A9_STATUS_T;



//����¯״̬ö��
typedef enum {
	IA_AIRHEATER_STATUS_ONOFF = IA_MK(IA_TYPE_AIRHEATER, 1),
	IA_AIRHEATER_STATUS_GEAR = IA_MK(IA_TYPE_AIRHEATER, 2),
	IA_AIRHEATER_STATUS_TIME = IA_MK(IA_TYPE_AIRHEATER, 3),
	IA_AIRHEATER_STATUS_MODE = IA_MK(IA_TYPE_AIRHEATER, 4),
	IA_AIRHEATER_STATUS_POWER = IA_MK(IA_TYPE_AIRHEATER, 5),
	IA_AIRHEATER_STATUS_TEMP = IA_MK(IA_TYPE_AIRHEATER, 6),		// ��ǰ�¶ȣ���λ0.1���϶�
} IA_AIRHEATER_STATUS_T;



// ������͡
typedef enum {
	IA_AIRHEATER_YCYT_STATUS_TEMP_SET = IA_MK(IA_TYPE_AIRHEATER, 1), 	// �����¶� 1-35
	IA_AIRHEATER_YCYT_STATUS_TEMP_CURRENT = IA_MK(IA_TYPE_AIRHEATER, 2), 	// ��ǰ�¶� 0-51
	IA_AIRHEATER_YCYT_STATUS_MODE = IA_MK(IA_TYPE_AIRHEATER, 3),		// ģʽ 1 ˯�� 2 ʡ�� 3 ���� 4 ���� 5 �¿�
	IA_AIRHEATER_YCYT_STATUS_GEAR = IA_MK(IA_TYPE_AIRHEATER, 4),		//  ��λ 1 �ر� 2 �� 3 �� 4 ��
	IA_AIRHEATER_YCYT_STATUS_ORDER_TIME = IA_MK(IA_TYPE_AIRHEATER, 5),		// ԤԼ����
	IA_AIRHEATER_YCYT_STATUS_WORK = IA_MK(IA_TYPE_AIRHEATER, 6),		//����״̬
	IA_AIRHEATER_YCYT_STATUS_TIME = IA_MK(IA_TYPE_AIRHEATER, 7),		// ��ʱ�ػ�
	IA_AIRHEATER_YCYT_REFRESH_TIMER = IA_MK(IA_TYPE_AIRHEATER, 8),		// ��ʱ�ػ�
} IA_AIRHEATER_YCYT_STATUS_T;


// ����������״̬ö��
typedef enum {
	IA_AIRCLEANER_STATUS_WORK = IA_MK(IA_TYPE_AIRCLEANER, 1),	// ����״̬: 0 ���� 1 ����
	IA_AIRCLEANER_STATUS_SPEED = IA_MK(IA_TYPE_AIRCLEANER, 2),	// ���� 1�� 2 �� 3 ��
	IA_AIRCLEANER_STATUS_TIMER = IA_MK(IA_TYPE_AIRCLEANER, 3),// ���ö�ʱ��������Ϊ��λ
	IA_AIRCLEANER_STATUS_ULTRAVIOLET = IA_MK(IA_TYPE_AIRCLEANER, 4),// ������ 0 �ر� 1����
	IA_AIRCLEANER_STATUS_ANION = IA_MK(IA_TYPE_AIRCLEANER, 5),		// ������ 0 �ر� 1����
	IA_AIRCLEANER_STATUS_PM25 = IA_MK(IA_TYPE_AIRCLEANER, 6),		// ��ǰpm2.5
	IA_AIRCLEANER_STATUS_TEMP = IA_MK(IA_TYPE_AIRCLEANER, 7),		// ��ǰ�¶�
	IA_AIRCLEANER_STATUS_RH = IA_MK(IA_TYPE_AIRCLEANER, 8),			// ��ǰʪ��
	IA_AIRCLEANER_STATUS_POWER = IA_MK(IA_TYPE_AIRCLEANER, 9),		// ��ǰ����
	IA_AIRCLEANER_STATUS_MODE = IA_MK(IA_TYPE_AIRCLEANER, 10),		// ���ù���ģʽ
	IA_AIRCLEANER_STATUS_QUERY = IA_MK(IA_TYPE_AIRCLEANER, 11),		// ��ѯ���й���״̬
	IA_AIRCLEANER_STATUS_TERILIZE = IA_MK(IA_TYPE_AIRCLEANER, 12),	// ����ɱ��
	IA_AIRCLEANER_STATUS_PERIODIC_TIMER = IA_MK(IA_TYPE_AIRCLEANER, 13),	// ���ڶ�ʱ��
} IA_AIRCLEANER_STATUS_T;

// ԡ������
typedef enum {
	IA_BATHROOMMASTER_STATUS_ONOFF = IA_MK(IA_TYPE_BATHROOMMASTER, 1),			// ͨ�翪�� 0-�أ�1-��
	IA_BATHROOMMASTER_STATUS_NEGATIVEIONS = IA_MK(IA_TYPE_BATHROOMMASTER, 2),	// ������   0-�أ�1-��
	IA_BATHROOMMASTER_STATUS_LIGHT = IA_MK(IA_TYPE_BATHROOMMASTER, 3),			// ����     0-�أ�1-��
	IA_BATHROOMMASTER_STATUS_AIR = IA_MK(IA_TYPE_BATHROOMMASTER, 4), 			// ����     0-�أ�1-��
	IA_BATHROOMMASTER_STATUS_DRY = IA_MK(IA_TYPE_BATHROOMMASTER, 5),			// ��ʪ     0-�أ�1-��
	IA_BATHROOMMASTER_STATUS_WARNM = IA_MK(IA_TYPE_BATHROOMMASTER, 6), 			// ��ů     0-�أ�1-��
	IA_BATHROOMMASTER_STATUS_TIMER = IA_MK(IA_TYPE_BATHROOMMASTER, 7),			// ��ʱ (��λ����)
}IA_BATHROOMMASTER_STATUS_T;


typedef enum {
	CMS_IDLE = 0,
	CMS_WAIT_IR = 1,
	CMS_WAIT_MATCH = 2,
	CMS_WAIT_CODE = 3,
	CMS_DONE = 4
} CMS_T;

typedef enum {
	CMA_REPORT = 1,
	CMA_REQUEST,
	CMA_CANCLE,
	CMA_SET,
} CMA_T;

typedef enum {
	CMT_AC = 0,		// �ƿյ�
	CMT_TV = 1,		// �Ƶ���
	CMT_STB = 2,	// �ƻ�����
} CMT_T;

// ��ʶ�����
typedef struct {
	u_int32_t err;
	u_int8_t action;	// 1: ֪ͨ 2: ������ƥ�� 3: ȡ����ƥ��
	u_int8_t status;	// ��ǰ״̬ CMS_XXX
	u_int16_t local_id;
	u_int8_t type;	// 0: �ƿյ�ƥ�� ...
	u_int8_t match_id_num;	// ƥ�䵽���ٸ������4
	u_int16_t set_id;	// �ֻ����ã���������һ��ID��
	u_int16_t match_id[4];	// ���ص����4��ƥ�䵽��ID��
} net_cloud_match_t;

typedef struct {
	u_int16_t id;
	u_int16_t start_idx;
	u_int16_t end_idx;
	u_int16_t pad;
} net_cloud_match_ac_data_t;

typedef struct {
	u_int8_t err;
	u_int8_t pad;
	u_int16_t lib_id;
	u_int8_t lib_name_len;
} net_cloud_match_ac_info_t;


typedef struct {
	u_int8_t on_off;
	u_int8_t resv[3];
} net_eb_work_t;

/*Dispatch ������ӵ�TLV*/

#define	DISP_TLV_DEV_TYPE	1

typedef struct{
	u_int8_t type;
	u_int8_t subtype;
	u_int8_t ext_type;
	u_int8_t pad;
}tlv_devtype_t;

typedef enum {
	NET_805_CONFIG_QUERY,//��ѯ
	NET_805_CONFIG_CTL//����
} net_805_config_cmd_t;

typedef enum {
	NET_805_BEEP,//������
	NET_805_SCREEN//������
} net_805_config_type_t;

typedef struct {
	u_int32_t err;//�����
	u_int8_t cmd;//����
	u_int8_t pad;
} net_805_config_hdr_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t pad;
	u_int16_t pad2;
} net_805_beep_t;

typedef struct {
	u_int8_t onoff;
	u_int8_t pad;
	u_int16_t pad2;
} net_805_screen_t;


/*ͨ��Э�����ݽṹ1�ֽڶ��룬�����ݽṹ����ڱ���֮ǰ*/
#pragma pack(pop)

#ifdef WIN32
static __inline u_int64_t ntoh_ll(u_int64_t n)
#else
static inline u_int64_t ntoh_ll(u_int64_t n)
#endif
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	return (((n>>56)&0xFFULL)
			|((n>>40)&0xFF00ULL)
			|((n>>24)&0xFF0000ULL)
			|((n>>8)&0xFF000000ULL)
			|((n<<8)&0xFF00000000ULL)
			|((n<<24)&0xFF0000000000ULL)
			|((n<<40)&0xFF000000000000ULL)
			|((n<<56)&0xFF00000000000000ULL));
#elif __BYTE_ORDER == __BIG_ENDIAN
	return n;
#else
# error "Please fix <bits/endian.h>"
#endif
}
#ifdef CLIB_HDR
#else
#include "diy_device.h"
#endif

#endif

