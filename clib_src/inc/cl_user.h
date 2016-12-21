#ifndef	__CL_USER_H__
#define	__CL_USER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_ia.h"
#include "cl_intelligent_forward.h"
#include "cl_rfgw.h"
#include "cl_lanusers.h"
//#include "cl_linkage.h"

/*******************************************

	�û���ز���
	module: user
	level: public
		
********************************************/


/* user callback event */
enum {
	UE_BEGIN = 0,
	UE_LOGOUT = UE_BEGIN + 1,
	UE_LOGIN_OFFLINE = UE_BEGIN + 2,
	UE_LOGIN_ONLINE = UE_BEGIN + 3,
	UE_INFO_MODIFY = UE_BEGIN + 4,
	UE_MODIFY_PASSWD_OK = UE_BEGIN + 5,
	UE_MODIFY_PASSWD_FAIL = UE_BEGIN + 6,
	UE_MODIFY_NICKNAME_OK = UE_BEGIN + 7,
	UE_MODIFY_NICKNAME_FAIL = UE_BEGIN + 8,
	UE_MODIFY_ALARM_PHONE_OK = UE_BEGIN + 9,
	UE_MODIFY_ALARM_PHONE_FAIL = UE_BEGIN + 10,
	UE_LOGIN_ERROR = UE_BEGIN + 11,
	UE_PHONE_USER_REGISTER_OK = UE_BEGIN + 12,
	UE_PHONE_USER_REGISTER_FAILED = UE_BEGIN + 13,
	UE_PHONE_USER_RESET_OK = UE_BEGIN + 14,
	UE_PHONE_USER_RESET_FAILED = UE_BEGIN + 15,
	UE_PHONE_USER_GOOD_VCODE = UE_BEGIN + 16,
	UE_PHONE_USER_BAD_VCODE = UE_BEGIN + 17,
	/*�ύ���ֻ�����ɹ�*/
	UE_BIND_PHONE_REQUEST_OK = UE_BEGIN + 18,
	/*�ύ���ֻ�����ɹ����Ѱ�*/
	UE_BIND_PHONE_OK = UE_BEGIN + 19,
	/*�յ����ֻ������б�*/
	UE_BIND_PHONE_REQUEST_LIST = UE_BEGIN + 20,
	/*�յ��Ѱ��ֻ��б�*/
	UE_BIND_PHONE_LIST = UE_BEGIN + 21, 
	/*�յ��԰�����Ĳ������*/
	UE_BIND_PHONE_OPERATION_RESULT = UE_BEGIN + 22,
	/*�յ�ƻ�����ͷ������ý��*/
	UE_APNS_CONFIG = UE_BEGIN + 23,
	/*�ֻ��˺��ѱ�ע��*/
	UE_PHONE_USER_REGISTER_EXISTED = UE_BEGIN + 24,
	UE_PHONE_NEW_VERSION = UE_BEGIN + 25,

	UE_PHONE_USER_ADD_DEV_FINISH = UE_BEGIN + 26,
	UE_PHONE_USER_DEL_DEV_FINISH = UE_BEGIN + 27,
	
	// �����������仯�ˣ������¶ȡ�PM2.5��
	UE_ENV_MON_MODIFY = UE_BEGIN + 30,
	UE_SET_NAME_OK = UE_BEGIN + 31,
	UE_SET_NAME_FAILED = UE_BEGIN + 32,
	/*�յ�ƻ�����ͷ�������ʧ��*/
	UE_APNS_CONFIG_FAIL = UE_BEGIN + 33,
	/*�յ��豸�п������汾*/
	UE_DEV_UPGRADE_READY = UE_BEGIN + 34,
	UE_DEV_UPGRADE_SET_OK = UE_BEGIN + 35,
	UE_DEV_UPGRADE_SET_FAIL = UE_BEGIN + 36,
	UE_DEV_UPGRADE_NOW_OK = UE_BEGIN + 37,
	UE_DEV_UPGRADE_NOW_FAIL = UE_BEGIN + 38,
	UE_DEV_UPGRADE_PROGRESS = UE_BEGIN + 39,
	/* �յ����ڻ������� */
	UE_ENV_WEATHER_SUGGEST = UE_BEGIN + 40,
	
	UE_DEV_ACTIVE_SUCCESS	 = UE_BEGIN + 45,	
	UE_DEV_ACTIVE_FAILED	 = UE_BEGIN + 46,	
	
	//����805�ɹ�
	UE_CTRL_805_OK			= UE_BEGIN + 47,
	//����805ʧ��
	UE_CTRL_805_FAILED		= UE_BEGIN + 48,
	//RFGW���ط������豸��Ҫȷ�����
	UE_RFGW_DEV_FIND		= UE_BEGIN + 49,
	//RF�豸��͸�������¼�
	UE_RFGW_DEV_TT			= UE_BEGIN + 50,
	//���ö�ʱ��ʱ���ͻ�¼�
	UE_CTRL_YT_TIME_ERR		= UE_BEGIN + 51,
	//rf�豸������ͻ�¼�
	UE_RFGW_DEV_UPGRDE_ERR	= UE_BEGIN + 52,
	//���ù���ʱ���ܲ����¼�
	UE_CTRL_YT_CTRL_FAULT 	= UE_BEGIN + 53,
	// ӡ�ȳ�����ʷ��¼ͳ����Ϣ����
	UE_INDIACAR_HISOTRY_INFO_UPDATE = UE_BEGIN + 54,
	// ӡ�ȳ�����ʷ��¼��ϸ��Ϣ����
	UE_INDIACAR_HISOTRY_DETAIL_UPDATE = UE_BEGIN + 55,
	// ӡ�ȳ����豸����
	UE_INDIACAR_DEV_UPGRADE = UE_BEGIN + 56,

	// �յ�ͨ��SN����ƻ�����͵ĳɹ�
	UE_APNS_CONFIG_BY_SN_OK = UE_BEGIN + 57,
	// �յ�ͨ��SN����ƻ�����͵�ʧ��
	UE_APNS_CONFIG_BY_SN_FAILED = UE_BEGIN + 58,

	// ӡ�ȳ�ʵʱ���͹����У���ǰ�ó̽�����
	UE_INDIACAR_REALTIME_TRIP_OVER = UE_BEGIN + 59,
	// ӡ�ȳ�����ʵʱ�ó����ݣ����ǵ�ǰû�����ó���
	UE_INDIACAR_REALTIME_TRIP_ERR = UE_BEGIN + 60,
	// ӡ�ȳ����յ�ʵʱ����
	UE_INDIACAR_REALTIME_TRIP = UE_BEGIN + 61,
	// ӡ�ȳ��õ��������ó̸���
	UE_INDIACAR_JORNEY_COUNT = UE_BEGIN + 62,
	//��΢ң������Ҫ�����¼�
	UE_DWYK_NEED_UPGRADE = UE_BEGIN + 63,

	// ӡ�ȳ���ȡ����ͼƬ
	UE_INDIACAR_GET_PIC = UE_BEGIN + 64,

	// ӡ�ȳ���Ƶ���������ȶ��Ͽ�
	UE_INDIACAR_AGENT_SERVER_ERROR = UE_BEGIN + 65,

	// ӡ�ȳ��������������Ƶ����õķ���
	UE_INDIACAR_GET_LOCAL_WATCH_INFO = UE_BEGIN + 66,

	// ӡ�ȳ��������������Ƶ����õ��ļ��б�
	UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST = UE_BEGIN + 67,
	//�յ�����鿪�ػ������¼���errno��ʾ���ؿյ���״̬��0 �ɹ� 1 ��ʱ��δ��⵽���� 2��ǰ�Ѿ��ǿ���״̬ 0xff:У����
	UE_WKAIR_CHECK_STATUS = UE_BEGIN + 68,
	// ӡ�ȳ��Ѿ���¼���ˣ��ظ�¼����Ҫ�ȹر�
	UE_INDIACAR_IS_RECORDING = UE_BEGIN + 69,	
	//�յ����𶯼�鿪�ػ������¼���errno��ʾ���ؿյ���״̬��ƥ������0 �ɹ� 1ʧ�� 0xff��У���� 
	UE_WKAIR_AUTO_SHOCK_CHECK = UE_BEGIN + 70,


	UE_DEV_UPGRADE_STM_ERASE = UE_BEGIN + 71,

	// ӡ�ȳ���ȡ��һ����Ƶ
	UE_INDIACAR_GET_AUDIO = UE_BEGIN + 72,

	// ӡ�ȳ�������һ֡MP4֡��Ӧ��BMPͼƬ
	UE_INDIACAR_GET_MP4_PIC = UE_BEGIN + 73,
	// ӡ�ȳ���ȡMP4�ļ�����
	UE_INDIACAR_DECODE_MP4_FINISH = UE_BEGIN + 74,
	//rf���ز���������
	UE_RFGW_NOT_ALLOW_UPGRADE = UE_BEGIN + 75,
	
	UE_IWULINK_CLIENT = UE_BEGIN + 98,
	UE_END = UE_BEGIN + 99
};

////////////////////////////////////////////////////

#define IJ_007 0x0  /*i+006*/
#define IJ_006 0x6  /*i+006*/
#define IJ_001 0x01  /*i+001 Ŀǰ��Ҫ��i+001E���ߺ���ת����*/
#define IJ_002 0x02  /*i+002 iTV-C*/
#define IJ_003 0x03  /*i+003 wireless camera*/
#define IJ_008 0x08  /* i+008 iTV-C */
#define IJ_803 0x37  /*��˹��������ͷ*/
#define IJ_805 0x36 /*����ת����*/
#define IJ_807 0x09  /*i+807*/
#define IJ_808 0x10  /*808����*/
#define IJ_812 0x11  /*udpů���*/
#define IJ_816 0x12 /*udp ��͡*/
#define IJ_813 0x13 /*udp ����������*/
#define IJ_820 0x14 /*udp��*/
#define	IJ_101	0x15	/* E�� */
#define	IJ_102	0x16	/* E��-II�� */
#define IJ_821 0x17 /*�ʺ����̺*/
#define IJ_822 0x18  /*���ܱ��¿���*/
#define IJ_823 0x19 /*�����¿���*/
#define IJ_830 0x1A  /*LEDE ��ɫ��*/
#define IJ_840 0x1B /*���������*/
#define IJ_824 0x1C /*�������ˮ��*/
#define IJ_AMT 0x1D /*�������豸*/
#define IJ_RFGW 0x1E /*2.4G RF ����*/
#define IJ_CHIFFO  0x1F/*ǰ���Ʒ*/

    
#define IJ_FANSBOX		0x0A 	/* fansbox*/
#define IJ_OPT			0x20 	/*openwrtˢ���豸*/
#define IJ_ANDROID		0x21  	/*Andriodˢ���豸*/
#define IJ_COMMUNITY	0x22   /* С�������� */

#define IJ_AIRCONDITION 0x30	/* �ǻۼҵ絥Ʒ�յ� */
#define IJ_WATERHEATER 0X31		/* �ǻۼҵ絥Ʒ��ˮ�� */
#define IJ_AIRHEATER 0x32		/* �ǻۼҵ絥Ʒ����¯ */
#define IJ_AIRCLEANER 0x33		/* �ǻۼҵ絥Ʒ���������� */
#define IJ_ELECTRICFAN 0x34		/* �ǻۼҵ絥Ʒ���� */
#define IJ_815			0x35     /*����ԡ������*/
#define IJ_805			0x36     /*����ת����*/

#define IJ_TL_TEMP	0x50 //�����¿���
    
///////////////////////////////////////////////////////////
#define IJ_HXPBJ	0X51/*��Ѹ�豸*/

#define ETYPE_IJ_HX_PBJ 0x1 //��Ѹ�Ʊڻ�
#define ETYPE_IJ_HX_POT 0x2 //��Ѹ������
////////////////////////////////////////////////////////////
#define IJ_QPCP		0x52/*ǧ������*/
#define IJ_QP_POT   0x53/*ǧ������*/
#define IJ_TEST_DEV 0x54 /*����DEV*/
#define IJ_101_OEM  0x55 /*ɳ�غ����ֲ���*/
#define IJ_BIMAR_HEATER  0x57 /*bimarů���*/
#define IJ_JL_STAGE_LAMP 0x58 /*������̨��*/
#define IJ_JS_MICWAVE   0x59    /*��ʯ΢��¯*/
#define IJ_KXM_DEVICE	0x60 /*��ϣ��*/
#define IJ_838_YT	0X61	/*����*/
#define IJ_839_ADS	0X62	/*�ĵ���*/

#define IJ_EVM		0x70	/* ֤��Ϊ������豸���ͣ���չ���� IJ_EVM_EXT_TYPE*/
#define IJ_LEIS		0x71	/* ��ʿ���� */
#define IJ_ZHDHX	0x91	/*�ǻʵ�����*/
#define IJ_HOMESERVER 0x92	/*��ͥ������*/

enum {
	ETYPE_LEIS_DEFAULT = 1,
	ETYPE_LEIS_YINSU = 2,	// ���ٵ�	
};

#define IJ_INDIACAR 0x64 	/* ӡ�ȳ���׷���� */
#define IJ_ZHCL		0x65	/*�ǻʴ���*/
////////////////////////////////////////////
#define ETYPE_INDIACAR_DEFAULT 0x1	// ӡ�ȳ���չ����
////////////////////////////////////////////

////////////////////////////////////////////
#define ETYPE_IJ_ZHCL 0x1	// �ǻʴ�����չ����
////////////////////////////////////////////

#define ETYPE_IJ_LEIS	0x1	// ��ʿ����Ĭ����չ����
////////////////////////////////////////////
typedef enum {
	EYPE_EVM_DEFAULT = 1,		// ֤������
	EYPE_EVM_YUYUAN_WATER_CLEANNER = 2,	// ��Դ��ˮ��
	EYPE_EVM_ZKCLEANNER = 3,	// �пƾ�����
	//EYPE_EVM_HYTHERMOSTAT = 6,	// �����¿��� �ϳ�ʹ��
	EYPE_EVM_BPUAIR_1 = 0X6,	// ���ݰ������ܿյ�������
	EYPE_EVM_BPUAIR_2 = 0X7,	// ���ݰ������ܿյ�������
	EYPE_EVM_BPUAIR_3 = 0X8,	// ���ݰ������ܿյ�������
	EYPE_EVM_BPUAIR_4 = 0X9,	// ���ݰ������ܿյ�������
	EYPE_EVM_HYTHERMOSTAT_AC = 0x0a, // �����¿��� �յ�
    EYPE_EVM_HYTHERMOSTAT_HT = 0x0b, // �����¿��� ˮů
    EYPE_EVM_JRXHEATER = 0xc,	// ���ս��о��â��ˮ��	
    EYPE_EVM_CJTHERMOSTAT = 0xd,	// ���������¿���
    EYPE_EVM_DRKZQ = 0xe,	// һƷ����ֲ���̻����ܿ�����
} IJ_EVM_EXT_TYPE;
////////////////////////////////////////////


////////////////////////////////////////////
#define ETYPE_IJ_KXM_HOST 0x1 //��ϣ���߿���
#define ETYPE_IJ_KXM_THERMOSTAT 0x2 //��ϣ���¿���
#define ETYPE_IJ_XY_THERMOSTAT 0x3 //��Դ�¿���
#define ETYPE_IJ_SBT_THER	0x4	//˼�����¿���
#define EYTPE_IJ_ZSSX_FURN	0X5 //��ɽ���͵���¯
#define EYTYP_IJ_KXM_AC		0X6 //��ϣ���յ���
#define ETYPE_IJ_YJ_HEATER	0x7	// ��ɽ������ܵ�ů¯
#define ETYPE_IJ_GALAXYWIND_THERMOSTAT 0x8	// ���ӷ����¿���(��ϣ���¿�������)
#define ETYPE_IJ_LINKON_THERMOSTAT 0X9	// linkon�¿���
#define ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB 0xa	// ���ӷ����¿���(��ϣ���¿�������) ��ҵ���п���

/////////////////////////////////////////////


#define ETYPE_IJ_JS_MICWARE 0x1 //��ʯ��չ�ͺţ�΢����
#define ETYPE_IJ_JS_MIC_BARBECUE 0x2 //΢��+�տ�
#define ETYPE_IJ_JS_ONLY_MIC	0x3 //��΢��
//////////////////////////////////////////////////////////////
#define IJ_HEATER_DEV 0x57  //ů���ͨ��sub_type
    
#define ETYPE_IJ_HEATER_BIMAR_C2000 0x1 //BIMAR C2000ů���
    
#define ETYPE_IJ_HEATER_AKM_0505L 0x6//�Ŀ���ů���0505L
#define ETYPE_IJ_HEATER_AKM_5162L 0x7//�Ŀ���ů���5162L
/////////////////////////////////////////////////////////////

#define IJ_SMART_PLUG   IJ_808  /*���ܲ���v3��Ʒ*/

//�����չ����
#define ETYPE_IJ_SMP_H_16A   0x1 // i+808-CN2.0 i+808-CN2.1 ����16A��գ������Űס�������ɫ��
#define ETYPE_IJ_SMP_B_16A   0x2 //i+828-CN1.0 ��ҵ16A��� �����Űף�
#define ETYPE_IJ_SMP_H_WX_10A   0x3 //i+807-CN2.0 i+807-CN2.1 ����10A���΢�ţ������Űס�������ɫ��
#define ETYPE_IJ_SMP_B_10A      0x4 //i+827-CN1.0 ���i8��ҵ�һ��յ����ܰ��±�׼��(10A�ſ��)
#define ETYPE_IJ_SMP_H_GALA_10A 0x5 //i+807-CN1.0  ���i8���ùһ��յ����ܰ��±�׼��(10A�ſ��)
#define ETYPE_IJ_SMP_H_GALA_16A 0x6 //i+808-CN1.0 ���i8���ùһ��յ����ܰ��±�׼��(16A�ſ��)
#define ETYPE_IJ_SMP_H_ENH_16A 0x7 //i+818-CN2.0 ���i8���ùһ��յ����ܰ�����ǿ��(16A΢���ſ��)
#define ETYPE_IJ_WK_KSA 0x0a		// ɳ����գ�ȥ���˵����������Ӻ���ѧϰ

#define ETYPE_IJ_SMP_H_ENH_16A_NO_INPUT 0xb	// //i+818-CN2.0 ���i8���ùһ��յ����ܰ�����ǿ��(16A΢���ſ��) (�����������)
#define ETYPE_IJ_WK_AJ	0xc // ���i8���ùһ��յ����ܰ��±�׼��(16A�ſ��)	�������    
#define ETYPE_IJ_WK_GD_H_16A 0x31 // i+808-CN2.0 i+808-CN2.1 ����16A��գ�GD��Ƭ�������Űס�������ɫ��
#define ETYPE_IJ_WK_GD_B_16A 0x32 // i+828-CN1.0 ��ҵ16A��� GD��Ƭ�� �����Űף�

#define IJ_UNKNOWN 0xFF  /*δ֪����*/

//!!!!ע��!!!!
#define ETYPE_IJ_MIN 0x01 /* �豸��չ���͵���Сֵ */
#define ETYPE_IJ_MAX 0xFF /* �豸��չ���͵����ֵ */

//�����豸��չ����
#define ETYPE_IJ_812_AH_5331P  0x1 //������ͨů�����֧��ҡͷ
#define ETYPE_IJ_812_AH_HT5C05P  0x2 //��֧��ҡͷ��ION
#define ETYPE_IJ_812_AH_9505R	0x3 //��֧��ҡͷ��֧��ION
#define ETYPE_IJ_812_HT_5325P   0x4//֧��ҡͷ��ECO,ͬ5331P
#define ETYPE_IJ_812_HT_5326P   0x5 //֧��ҡͷ��ECO,ͬ5331P
#define ETYPE_IJ_812_HT_5250P   0x6 //֧��ҡͷ����֧��ECO


#define ETYPE_IJ_816_LCYT 0x1


#define ETYPE_IJ_813_HK 0x1  /* ���ƿ��������� */
#define ETYPE_IJ_813_NB 0x2  /* �ϰؿ��������� */
#define ETYPE_IJ_820_GX 0x1	/* ��ѶLED����� */
#define	ETYPE_IJ_101_EB	0x1
#define ETYPE_IJ_821_CH 0x1

/*E����չ����*/
#define ETYPE_IJ_EB_USB    0x1 /*E����USB*/
#define ETYPE_IJ_EB_NO_USB 0x2 /*E������USB*/
#define ETYPE_IJ_EB_JNB_GLOBAL     0x3 /*E����USB,�ӵ�����ѹ��⹦�ܣ����ʰ�*/
#define ETYPE_IJ_EB_JNB_WATER_DROP 0x4 /*E������USB,ˮ�α���*/
#define ETYPE_IJ_EB_FOXCONN  0x5 /*��ʿ������*/
#define ETYPE_IJ_EB_GXLAXYWIND_GLOBAL  0x7 /*���ӷ���-WIFI���������ʰ棩*/
#define ETYPE_IJ_EB_GXLAXYWIND  0x8        /*���ӷ���-WIFI���������ڰ棩*/
#define ETYPE_IJ_EB_JNB_OVERSEA 0x9        /*���ܱ�Wi-Fi����,����*/
#define ETYPE_IJ_EB_KT		0x10				/* ����E�����ڰ� */
#define ETYPE_IJ_EB_KT_OVERSEA	0x11					/* ����E�����ʰ� */

//���ܱ��¿�����չ����
#define ETYPE_IJ_822_JNB 0x1 

//�����¿�����չ����
#define ETYPE_IJ_823_YL 0x1

//LEDE ��ɫ����չ����
#define ETYPE_IJ_830_LEDE 0x1

//�����������չ����
#define ETYPE_IJ_840_JCX  0x1

//�������ˮ��
#define ETYPE_IJ_824_HTC 0x1
//���û����
#define TYPE_IJ_824_YCJ	0X2
#define ETYPE_IJ_824_HTC_BUSINESS 0X3
//�ǿ���ˮ��
#define ETYPE_IJ_824_ZKRSQ	0X4

//�����ز�Ʒ
#define ETYPE_IJ_AMT_FAN 0x1

/*��������չ����*/
#define ETYPE_IJ_AMT_FAN    0x1 /*�����ط���*/

/*2.4G RF ������չ����*/
#define ETYPE_IJ_RFGW_6621 0x01 /*����6621ģ��*/

#define ETYPE_IJ_RFGW_S2	0x02 /*2.4G RF S2����*/
#define ETYPE_IJ_RFGW_S3	0x03 /*2.4G RF S3����*/
#define ETYPE_IJ_RFGW_S4	0x04 /*2.4G RF S4����*/
#define ETYPE_IJ_RFGW_YL	0x05 /*ҹ������*/
#define ETYPE_IJ_RFGW_S9	0x68 /*��ƿ����*/


#define ETYPE_IJ_RFDEV_STM32 0x21 /*STM32��Ƭ��RF�豸*/
//ǰ���Ʒ
#define ETYPE_IJ_CHIFFO_FlOOR_HEATER  0x01 /*ǰ���ů*/
#define ETYPE_IJ_CHIFFO_WATER_HEATER  0x02	/* ǰ����ˮ��QFM0591A */

//���ֲ�Ʒ
#define ETYPE_IJ_TL_HEATER 	0x1 //���ֲ�ů�豸
//ǧ����Ʒ�����̺͹���subtype���ֵ�
#define ETYPE_IJ_QP_CP 0x1 //ǧ������
//ǧ��
#define ETYPE_IJ_QP_POT 0x1 //ǧ����
#define ETYPE_IJ_QP_PBJ 0x2 //�Ʊڻ���չ����

// 0x54 ����֤��
#define ETYPE_IJ_TEST_CAR_WK 0x1 //�������
#define ETYPE_IJ_TEST_8266_LED 0x2 //esp8266 LED
#define ETYPE_IJ_TEST_XY	0X6/*��Դ�¿���*/
#define ETYPE_IJ_TEST_BITMAR 0x7 //bimarů���

//0x55 ����OEM
#define ETYPE_IJ_101_QL 0x1 //���ֲ���
#define ETYPE_IJ_101_SA 0x2 //ɳ��30A����

//0x57 bimarů���
#define ETYPE_BIMAR_HEATER 0x01
#define ETYPE_AUCMA_HEATER 0x06

//������չ����
#define ETYPE_IJ838_YT	0X1

//0x58 ������̨��
#define ETYPE_JL_STAGE_LAMP 0x01


//�ĵ�����չ����
#define ETYPE_IJ839_ADS	0X01

//803����ͷ��չ����
#define ETYPE_IJ803_HS  0x01 //i+803-hie, easyn, ��˼����ͷ
#define ETYPE_IJ803_C2 0x02 //i+gepc2, galaxywind, C2����ͷ
#define ETYPE_IJ803_C3 0x03 //i+gepc3 , galaxywind, C3����ͷ

enum{/*��¼����*/
	LTYPE_NORMAL = 1, /*��ͨ��¼��δ���ð��ֻ�*/
	LTYPE_BIND = 2, /*���ֻ���¼*/
	LTYPE_UNBIND = 3 /*δ���ֻ���¼*/
};

enum{/*������������*/
	NTYPE_SERVER = 1, /*���ӷ�����*/
	NTYPE_DEVICE = 2, /*������ֱ���豸���豸���ӵ�������*/
	NTYPE_DEVICE_OFFLINE = 3, /*������ֱ���豸���豸δ���ӵ�������*/
};

enum{
	BIND_PHONE_ACCEPT = 1, /*��׼������*/
	BIND_PHONE_DENY = 2, /*�ܾ�������*/
};

enum{
	ACTION_QUERY = 0, /* ��ѯ */
	ACTION_ADD = 1,  /*���*/
	ACTION_MOD = 2, /*�޸�*/
	ACTION_DEL = 3,  /*ɾ��*/
};

/* ���Ӱ󶨵Ĵ���� */
enum{
	BIND_ERR_NONE = 0,
	BIND_ERR_PASSWORD = 1,		//������֤�������
	BIND_ERR_OTHER_MASTER  	//�������豸�󶨣�����¼�󶨵��豸SN��
};

/* ���� */
enum{
	LANG_BEGIN = 0,
	LANG_CH = 1, /* �������� */
	LANG_EN = 2, /* Ӣ�� */
	LANG_MAX
};

/* �豸�汾��Ϣ */
typedef struct {
	u_int8_t major;		/* ���汾 */
	u_int8_t minor;		/* �ΰ汾 */
	u_int8_t revise;	/* �޶��汾 */
	u_int8_t pad;	/* ����ֽ� */
} cl_version_t;

/* �쳣����� */
// ͨ��̽����
enum {
	CAT_TYPE_COM_DETECTOR_LOW_BATTERY = 10,	// ��ص�������
	CAT_TYPE_COM_DETECTOR_ALARM = 14,	// ��������
};

// ��̩��
enum {
	CAT_TYPE_HTLLOCK_LOCK = 20,				// ��̩������
	CAT_TYPE_HTLLOCK_UNLOCK = 21,			// ��̩������
	CAT_TYPE_HTLLOCK_NOT_LOCK = 22,			// ��̩����δ��
	CAT_TYPE_HTLLOCK_ERR_PWD = 23,			// ��̩��������������ָ�ƻ�Ƭ����10��
	CAT_TYPE_HTLLOCK_HIJACK = 24,			// ��̩��������ٳ��������ٳ�ָ�ƿ���
	CAT_TYPE_HTLLOCK_MENCHANNICAL_KEY = 25,	// ��̩��ʹ�û�еԿ�׿���
};

// ����
enum {
	CAT_TYPE_DOOR_BREAK = 8,	// ����
	CAT_TYPE_DOOR_LOCK_BREAK = 9,	// ����
	CAT_TYPE_DOOR_LOW_BATTERY = 10,	// ��ع���
	CAT_TYPE_DOOR_OPEN = 11,	// �ſ�
	CAT_TYPE_DOOR_CLOSE = 12,	// �Ź�
	CAT_TYPE_DOOR_UNLOCK_TIMEOUT = 13,	// ��ʱ��δ����
};

// ��ȩ
enum {
	CAT_TYPE_CH2O_VALUE_HIGH = 29,	// ̽����ֵ������ֵ
	CAT_TYPE_CH2O_VALUE_SAFE = 30,	// ̽����ֵ�ظ�����
};

// ƽ����ʾ���豸״̬
enum {
	DISPLAY_STAT_NOT_INITED,		// ��û��ʼ����״̬,APP���ù�
	DISPLAY_STAT_VIRTUAL_ONLINE,	// ������
	DISPLAY_STAT_REAL_ONLINE ,	// ��ʵ���ߣ��ɲ鿴״̬������
	DISPLAY_STAT_OFF_LINE,	// �ֻ�����ͨ����
	DISPLAY_STAT_CONNECTING,	// ������
	DISPLAY_STAT_REAL_STAT,	// ������ʵ��״̬
};

/* cl_handle_t
 * slave handle��ȡֵ��Χ��[0x2000000, 0x3000000);
 */
enum {
    CL_SLAVEHANDLE_MIN = 0x2000000,
    CL_SLAVEHANDLE_MAX = 0x3000000
};

typedef struct {
	cl_obj_t obj;

	// IJ_XXX
	u_int32_t dev_type;

	/* �󶨲��������Ĵ����, BIND_ERR_XXX, 0δ����1�������,2�ѱ������豸�� */
	u_int32_t bind_error;
	/* bind_errnoΪBIND_ERR_OTHER_MASTER��Ч����¼���󶨵����豸SN */
	u_int64_t other_master_sn;

	 /* �̼��汾��Ϣ������ 1.2.3 */
	cl_version_t soft_version;
	/* �������汾��Ϣ������ 1.2.4 */
	cl_version_t upgrade_version;
	//rf�豸�µ�Ƭ���汾��
	cl_version_t rf_stm_ver;
	// �豸�����೤ʱ���ˣ���λ��
	u_int32_t uptime;
	// �豸����������Ӷ೤ʱ���ˣ���λ��
	u_int32_t online;
	// �豸�ϻ������೤ʱ���ˣ���λ��
	u_int32_t conn_internet;

	/*
		dev_type == IJ_008��ʱ��ʹ��
	*/
	// ��ң�ز����Ƿ���������
	bool has_current_detect;
	//֧�ֵ���ͳ�ƹ���
	bool has_electric_stat;


	/*
		dev_type == IJ_003����IJ_803��ʱ��ʹ��
	*/
	// �Ƿ�֧��¼����
	bool has_video_record;
	// �Ƿ�֧��ͼ��ת
	bool has_video_flip;
    // �Ƿ�֧�ֱ��Ͷ�
    bool has_v4l2_color_setting;
    // �Ƿ���̨ת�ٿ���
    bool has_roll_speed_ctrl;
    //�Ƿ�֧����̨����
    bool has_ptz;

	bool has_recv_flag_pkt;//�Ƿ��־��Ч
	u_int8_t is_support_la;//�Ƿ�֧����������	
    

	// Eϵ�����豸����豸
	// �Ƿ�֧�ֺ��⹦��(���ӻ����յ��ȵ�����֧�ֿ��ƺ�ѧϰ)��281ģ���еı�־
	bool has_ir;
	// �Ƿ�֧�����߹���(��忪�ء��������ȣ�֧�ַ��ͺ�ѧϰ)��281ģ���еı�־
	bool has_rf;
	// �Ƿ�֧�ָ澯(���ⱨ�������ŴŸ�Ӧ�ȣ�֧�ַ��ͺ�ѧϰ)��282ģ��
	bool has_alarm;
	// �Ƿ�֧�ֶ��źͱ������͵�������
	bool has_alarm_swich;
	// ֧�ֶ�ά��ɨ��json��ӵ���
	bool has_eq_add_by_json;
	//�Ƿ�����ʧ��
	bool is_upgrade_err;

	//��Ļ�Ƿ�
	bool is_805_screen_on;
	//�����Ƿ�
	bool is_805_beep_on;
	//UDP���豸
	bool is_udp;
	//���豸��չ����
	u_int8_t ext_type;
	// �Ƿ��ȡ��������Ϣ
	u_int8_t status_valid;
	// ���豸������ID
	u_int8_t developer_id[32];

	//rfͨ�ö�ʱ��
	cl_comm_timer_head_t comm_timer;
	cl_dev_timer_summary_t timer_summary;
	/* ������, 0Ϊ��Ч */
	cl_handle_t area_handle;
	/* ���豸��handle */
	cl_handle_t master_handle;

	u_int8_t is_support_public_shortcuts_onoff;		// ֧��ͨ�õĿ�ݿ��ػ�
	cl_shortcuts_onoff_t shortcuts_onoff;			// ͨ�õĿ�ݿ��ػ� 

	//�Ƿ�֧������һ����������
	bool is_support_dbc;

	//stat
	u_int32_t run_time;//�豸����ʱ��
	
	cl_rfdev_status_t rfdev;
} cl_slave_t;

#define	ULGE_NONE	0
// ��������к�
#define	ULGE_BAD_SN	1
// ������ǳ�
#define	ULGE_BAD_NICKNAME	2
// �������
#define	ULGE_BAD_PASSWORD	3
#define	ULGE_BAD_PHONE_NUM	4
// ��Ҫ���ֻ����ܵ�¼
#define	ULGE_NEED_BIND		5
// ��Ҫ���ֻ����Ѿ�������
#define	ULGE_FULL_BIND		6
// ���ֻ������Ѿ��������˴�����
#define ULGE_OTHER_BIND		7
// ���粻����
#define	ULGE_NETWORK_ERROR	8
// �������е�æ
#define	ULGE_SERVER_BUSY	9
//�ͻ��˰汾����
#define ERR_BAD_VERSION
//�豸����
#define ULGE_DEV_OFF_LINE 13

#define ULGE_DEV_CLONE 14
//v2Э��δע��
#define ERR_V2_UNBIND	15


#define ERROR_IMAGE				(100)


typedef struct {/*�豸���ֻ�����*/
	char phone_number[16]; /*�ֻ�����*/
	char phone_model[32]; /*�ֻ��ͺ�*/
	char bind_name[16];/*������*/
	char bind_uuid[40]; /*��uuid*/
	char bind_message[40];/*������*/
	char timestamp[20]; /*ʱ���2014-02-20 13:56:12*/
}cl_bind_phone_t;

typedef struct{
	u_int8_t count;
	u_int8_t reserved[3];
	cl_bind_phone_t *request_list;
}cl_bind_phone_request_list_t;

typedef struct{
	u_int8_t count;
	u_int8_t allow_normal;
	u_int8_t reserved[2];
	cl_bind_phone_t *bind_array;
}cl_bind_phone_list_t;

typedef struct{
	u_int32_t err_code; 
	u_int8_t action; 
	u_int8_t reserved[3];
	u_int8_t request_uuid[40]; /*������uuid*/
	cl_bind_phone_t operator_info; /*�����봦������Ϣ*/
}cl_bind_phone_result_t;

typedef struct{
	char action;		/*ACTION_QUERY ACTION_ADD*/
	char need_push;	/*�Ƿ��ģ�0��ʾ������*/
	u_int8_t cert_id;		/*����֤��id*/
	u_int8_t token_len;	/*token����*/
	u_int8_t msg_len;		/*������Ϣǰ׺����*/
	u_int8_t reserved[3];	/*����*/	
	char phone_ver[8];	/*�ֻ�ϵͳ�汾����"7.1"*/
	char token[64];		/*token�������ƣ�Ŀǰ32�ֽ�*/
	char msg_prefix[64];	/*������Ϣǰ׺��UTF8��ʽ*/

	// ��������
	/*
		������д�����������Ϣ����ʽ���¡�����Ϊ��
		languages=EN&push_music=whatdoesfoxsay.mp3 
	*/
	char language[64];
	char push_music[256];

	// С��������,IOS����
	char mipush_packname[64];
	char regid[256];
}cl_apns_config_t;

typedef struct {
	// ���û��ڹ������ڲ���Ψһ��ʶ
	cl_handle_t handle;

	//��Ӹ�is_udp_ctrl�ֶΣ���app���Ż�ʹ��
	bool is_udp_ctrl;

	// �û���������ƣ����������кţ�Ҳ�������ǳ�
	char *name;
	// ���豸�����к�
	u_int64_t sn;
	// �ǳ�, UTF-8
	char *nickname;

	// �û����������
	char *passwd;

	// �Ƿ��¼�ɹ�
	bool is_login;
	// �Ƿ�����
	bool is_online;
	// ƽ����ʾ��״̬,DISPLAY_STAT_XXX
	u_int8_t display_stat;
	// �Ƿ�֧�ְ��ֻ�
	bool can_bind_phone;
	char login_type;
	char net_type;
	
	// ��¼ʧ��ʱ�Ĵ���ULGE_XXXX
	int last_err;

	//****pc��ʾ��
	u_int32_t dev_ip;
	u_int32_t home_id;
	u_int32_t upgrade_version;
	//****

	// ������豸���豸�����ͣ�IJ_XXX
	u_int8_t sub_type;
	// ������豸���豸��չ����
	u_int8_t ext_type;
	// ������ID
	u_int8_t developer_id[32];
	// ������豸���豸�ĳ���ID
	char *vendor_id;
	// ������豸����ȡ�豸�ĳ�����Դ��URL
	char *vendor_url;

	// �Ƿ�֧�ֳ���ģʽ
	bool has_scene;
	// ֧���龰��ʱ��
	bool has_scene_timer;
	// ֧�ֱ����������龰ģʽ
	bool has_scene_linkage_alarm;
	// �Ƿ�֧������ģʽ
	bool has_area;

	// ȫ�ֱ�־����������̨���豸��֧�֣���Ҫ��������
	// �Ƿ�֧�ֺ��⹦��(���ӻ����յ��ȵ�����֧�ֿ��ƺ�ѧϰ)
	bool has_ir;
	// �Ƿ�֧�����߹���(��忪�ء��������ȣ�֧�ַ��ͺ�ѧϰ)
	bool has_rf;
	// �Ƿ�֧�ָ澯(���ⱨ�������ŴŸ�Ӧ�ȣ�֧�ַ��ͺ�ѧϰ)
	bool has_alarm;
	// �Ƿ�֧�ֶ��źͱ������͵�������
	bool has_alarm_swich;
	//֧��˫��RF����
	bool has_db_rf;
	// ֧�ֶ��빦��
	bool has_eq_gencode;
	// ֧�ֶ�ά��ɨ��json��ӵ���
	bool has_eq_add_by_json;

	//�豸���룬���������ֻ��˺�Ҫ�� 
	u_int8_t dev_passwd[16];

	//�жϸ��豸�汾�Ƿ����
	bool dev_ver_is_valid;//�Ƿ���Ч��Ϊ��ʱdev_ver_is_too_low����Ч
	bool dev_ver_is_too_low;

	u_int8_t is_support_telnet;//�Ƿ�֧��telnet����
	
	// �Ƿ�֧����ɫ����
	bool has_green_net;
	// �Ƿ��б�̩����ģ��
	bool has_belter;

	// objsָ�������ж��ٸ�ָ��
	u_int32_t num_objs;
	// ����ͷ��������ָ������
	cl_obj_t **objs;

	// �ж��ٸ�usb����ͷ
	u_int32_t num_usb_video;
	// USB����ͷ��objs�дӵڼ�����ʼ
	u_int32_t idx_usb_video;

	// �ж��ٸ����豸
	u_int32_t num_slave;
	// ���豸��objs�дӵڼ�����ʼ
	u_int32_t idx_slave;

	// �ж��ٸ�����
	u_int32_t num_equipment;
	// ������objs�дӵڼ�����ʼ
	u_int32_t idx_equipment;
    /*�豸�м��������ֻ�����*/
    u_int32_t num_alarm_conf_phone;
    //�����ֻ������б�
    char** phone_list;
    //�ж��ٸ�����
    u_int16_t num_area;
    //�����б�
    struct _cl_area_s_** areas;
    //�ж��ٸ��龰ģʽ
    u_int16_t num_scene;
    //�龰ģʽ�б�
    struct _cl_scene_s_** scenes;
    //���ܲ���v3״̬��Ϣ
    struct _cl_air_info_s *air_info;
    //����ů���״̬
    struct _cl_ah_info_s *ah_info;
	//�ǻۼҾ�״̬��Ϣ
	struct _cl_ia_info_s_ ia_info;
    //805M����ת������Ϣ
    cl_if_info_t if_info;
	// E�������Ϣ
	cl_ia_eb_info_t *eb_info;
      //ͨ��UDP �豸��Ϣ
	struct cl_com_udp_device_data_s* com_udp_info;
	// ������������ʹ��
	void *priv;
} cl_dev_info_t;

//�������ݽṹ
#define APP_USER_UUID_NAME_LEN		16
//��ͥ��ǩ
#define LA_LABEL_NAME_MAX_LEN	(64)

typedef struct {
	u_int8_t state;//��ӵĹ���״̬��1��ʾִ���У�0��ʾδִ��
	u_int8_t enable;//ʹ��״̬��1��ʾʹ��
	u_int32_t rule_len;//rule_len
	u_int8_t *rule;//��ӹ����ַ���\0�������ַ���
	u_int32_t rule_id;//��ӹ������������id
	u_int32_t last_exec_time;//�����ϴ�ִ��ʱ���
}cl_rule_desc_t;

typedef struct {
	u_int32_t user_id;//�ü�ͥӵ�е��û�id
	u_int32_t join_time;//���û������ͥ��ʱ��
	u_int32_t lastuse_time;//��Ա�����½ʱ�䣬�������Ǹ���Ա��ʾ
	u_int8_t role_id;
	u_int8_t desc[64];
}la_share_desc_t;

typedef struct {
	u_int16_t id;
	char label_name[LA_LABEL_NAME_MAX_LEN];
	u_int16_t sn_num;
	u_int64_t *sn;
}la_label_desc_t;

//��ݰ���
#define LA_SC_NAME_MAX (64)

#define LA_SC_A_NUM	(6)

typedef struct{
	u_int8_t valid;//�Ƿ���Ч
	char name[LA_SC_NAME_MAX];
	u_int32_t rule_id;
}la_sc_key_t;

// �ֵ�
#define MAX_LA_DICT_NUM 64

typedef struct {
	u_int8_t *key;
	u_int16_t key_len;
	u_int8_t *value;
	u_int16_t value_len;
} cl_la_dict_t;

typedef struct {
	u_int32_t home_id;	// ��ͥID
	int ndict;	// �ж��ٸ��ֵ�
	cl_la_dict_t dict[MAX_LA_DICT_NUM];
} cl_la_dict_info_t;

typedef struct {
	cl_handle_t handle;
	bool is_def_home;//�Ƿ���Ĭ��Ȧ��
	u_int8_t online;//�Ƿ������Ϸ������ģ�1��ʾ������
	u_int32_t home_id; //�����������ȫ��Ψһid
	char home_name[64];	//��ͥ����
	u_int32_t last_rule_time;//�ϴι����޸�ʱ��
	u_int32_t last_template_time;//�ϴ�ģ���޸�ʱ��

	u_int16_t url_num;//ģ�����
	u_int8_t **url_array;//ģ��url����

	bool rules_is_cache;//��ʾ�����ǲ��ǻ����
	u_int16_t rule_num;//��ӵĹ���
	cl_rule_desc_t *rule_array;//��ӵĹ����ַ���

	u_int8_t *share;//������,\0�������ַ���

	u_int16_t share_desc_num;//�ü�ͥ������ʹ�õĸ�������һ���Ǽ�ͥ������
	la_share_desc_t *share_desc_array;	
	
	u_int8_t home_passwd[APP_USER_UUID_NAME_LEN];//��ͥ����

	//��ͥ��ǩ��
	u_int16_t label_num;
	la_label_desc_t *label;

	//��ݰ���
	la_sc_key_t la_sc_key[LA_SC_A_NUM]; 

	// �ֵ�
	cl_la_dict_info_t *dict;
}cl_la_home_info_t;

typedef struct cl_user_s {
	cl_handle_t user_handle;

	// �û�������û�����������SN��Ҳ�������ǳƣ�Ҳ�������ֻ���
	char *username;
	// �Ƿ����ֻ�ע����ʺ�
	bool is_phone_user;
	//�Ƿ���������ͥ
	bool is_la;
	// �û����������
	char *passwd;
	// �û�������������MD5����󣬴�ӡ���ַ���
	char *passwd_md5_str;

	// �Ƿ��¼�ɹ�
	bool is_login;
	// ��¼ʧ��ʱ�Ĵ���ULGE_XXXX
	int last_err;

	// ������豸���豸�����ͣ�IJ_XXX
	u_int8_t sub_type;
	//������豸���豸��չ����
	u_int8_t ext_type;
	// ������豸���豸�ĳ���ID
	char *vendor_id;
	// ������豸����ȡ�豸�ĳ�����Դ��URL
	char *vendor_url;

	u_int8_t is_support_spe_up;//�Ƿ�֧��ָ������
	//������ͥ����
	cl_la_home_info_t home;

	u_int32_t num_dev;
	// ���ʺŻ��ͥ�������豸��ָ�����顣
	cl_dev_info_t **dev;
} cl_user_t;


// ����ӿ���Ϣ
typedef struct {
	// �ӿ�����
	char *name;

	bool is_up;
	bool is_ip_valid;
	
	u_int32_t ip;
	int mtu;
	u_int64_t rx_byte;
	u_int64_t tx_byte;
} cl_ni_t;

// ���������ӵĿͻ���
typedef struct {
	// ���������
	char *name;
	u_int32_t ip;
	u_int8_t mac[6];
	// ���뷽ʽ: ���ߡ�����
	u_int8_t is_from_wifi;
	u_int8_t pad;
} cl_lan_client_t;

#define DAY_HOUR 24
typedef struct light_smp{
	u_int32_t yesterday;
	u_int32_t today;
	u_int32_t sample[DAY_HOUR];
}light_smp_t;

enum {
	ACTIVE_STATUS_NOACTIVE = 0,//δ�����
	ACTIVE_STATUS_ACTIVE = 1,//�Ѽ����
};

typedef struct{
	u_int32_t active_status; //����״̬ACTIVE_STATUS_XXX
	u_int64_t active_sn;//������ʱ���伤���sn��
}active_st_t;

#define DISK_MAX_NUM	(10)
#define DISK_MAX_MODEL_LEN	(64)

typedef struct {
	short temp;//�¶�,��
	u_int32_t use_time;//ʹ��ʱ������λСʱ
	u_int8_t model[DISK_MAX_MODEL_LEN];//�ͺ�
	u_int8_t serial[DISK_MAX_MODEL_LEN];//���к�
	u_int32_t capacity;//��������λM
}cl_disk_item_info_t;

typedef struct {
	u_int8_t valid;//Ӳ����Ϣ�Ƿ���Ч.
	
	u_int8_t mode;//Ӳ�̿లװģʽ�������Ȳ��ܰ�

	u_int32_t total_capacity;//����������λM
	u_int32_t used_capacity;//��ʹ����������λM

	u_int8_t num;//���豸��������Ӳ��
	cl_disk_item_info_t disk_item[DISK_MAX_NUM];
}cl_disk_info_t;


//�������֧�ָ���
#define ETH_MAX_NUM	(10)

typedef struct {
	u_int8_t index;//��������
	u_int32_t ip;//����ip��ַ
	u_int8_t name[16];//��������
	u_int32_t tx_rate;//�������ʣ���λ bytes/s,�ֽ�ÿ�룬app�Ͽ��Ը����������1024��ʾm/s��
	u_int32_t rx_rate;//�������ʣ���λ bytes/s��
}cl_eth_item_info_t;

typedef struct {
	u_int8_t valid;//������Ϣ�Ƿ���Ч

	u_int8_t num;//���ڸ���
	cl_eth_item_info_t eth_item[ETH_MAX_NUM];
}cl_eth_info_t;

typedef struct{
	u_int8_t is_stat_valid; //״̬�����Ƿ���Ч������uptime��online
	////
	u_int8_t dev_to_server_stat; //�豸���ӷ�����״̬
	///
	u_int8_t pad[2];
	u_int32_t devSvnVersion; //�豸svn�汾��
	u_int32_t dev_cur;//�յ�����ǰ����
	u_int32_t dev_cur_ad;//�յ�����ǰ����adֵ
	u_int32_t dev_cur_k;//�յ�����ǰ����kֵ
	u_int32_t dev_cur_b;//�յ�����ǰ����bֵ
	u_int32_t dev_vol;//�յ�����ǰ��ѹ
	u_int32_t dev_vol_ad;//�յ��������ѹdֵ
	u_int32_t dev_vol_k;//�յ�����ǰ��ѹkֵ
	u_int32_t dev_vol_b;//�յ�����ǰ��ѹbֵ
	u_int32_t dev_light_ad;//�յ�����ǰ����adֵ
	u_int8_t  dev_domain[32]; //�豸���ӵ�����
	u_int32_t dev_cur_phone_num; 
	light_smp_t light_study;
	u_int32_t avg_ad; //����ƽ����?
	u_int32_t max_ad; //����������
	u_int32_t delay_power_on_time; //��������ͻ��ʱ��
	u_int32_t delay_power_off_time;// �ػ�����ͻ��ʱ��
	u_int32_t no_load_ad; //���ص�����AD ֵ
	u_int32_t smt_soft_ver; // ��Ƭ������汾
	u_int32_t smt_hard_ver; //��Ƭ��Ӳ���汾
	u_int16_t ir_lib_id; //��������ID
	u_int16_t pad1;
	u_int32_t cold_delay_pn_time;//���俪������ͻ��ʱ��(s
	u_int32_t cold_delay_pf_time;//����ػ�����ͻ��ʱ��(s
	u_int32_t hot_delay_pn_time;//���ȿ�������ͻ��ʱ��(s
	u_int32_t hot_delay_pf_time;//���ȹػ�����ͻ��ʱ��(s
	char* stm_32_dbg_info; //��Ƭ������ʱ��
	u_int32_t adjust_pressed; //�����ʾ�Ѿ�����reset��������У��
	u_int32_t pt_stat_flag; //��ҵ��ղ����־λ
	////////////////////////////////////////////////////////
	u_int32_t dispatch_ip; //���������ip
	u_int32_t dev_server_ip; //�豸������ip
	u_int32_t dev_cur_time; //�豸��ǰʱ��
	u_int8_t dev_img_compile_data[32]; //�豸�����������
	u_int8_t dev_img_compile_time[32]; //�豸�������ʱ��
	active_st_t active_st;
	//timezone
	u_int8_t timezone_valid;
	u_int32_t timezone;
	//С��������
	u_int8_t scc_onoff_valid;
	u_int8_t scc_onoff;

	u_int32_t wifi_conn_time;	// WIFI����ʱ��
	int8_t wifi_rssi;	// WIFI�ź�ǿ�ȣ�����Ϊ��
	u_int8_t wifi_phy_mode;	// WIFI��ģʽ
	cl_version_t wifi_version;	//  WIFI�汾��

	cl_version_t kernel_image_version;	// Kernel����汾��
	u_int32_t kernel_image_svn;		// Kernel ����SVN��
	cl_version_t user_image_version;	// User����汾��
	u_int32_t user_image_svn;		// User����汾��
//	u_int32_t wireless_wan_ip;		// ����WAN��IP
	u_int32_t wired_wan_ip;			// ����WAN��IP

	u_int8_t wifi_mac[6];	// WIFI��MAC��ַ

	//Ӳ����Ϣ
	cl_disk_info_t disk_info;
	//������Ϣ
	cl_eth_info_t eth_info;
}cl_udp_dev_stat;

typedef struct {
	cl_handle_t handle;
	u_int64_t sn;
	// CPUʹ���ʣ�С������λ
	u_int16_t cpu;
	// �ڴ�ʹ���ʣ�С������λ
	u_int16_t mem;
	 /* �̼��汾��Ϣ������ 1.2.3 */
	cl_version_t soft_version;
	/* �������汾��Ϣ������ 1.2.4 */
	cl_version_t upgrade_version;
	/* �°汾��Ϣ ��can_updateΪtrueʱ��Ч */
	cl_version_t new_version;
	//Ӳ���汾
	cl_version_t hardware_version;
	/* true��ʾ�豸���°汾�������� */
	bool can_update;
	/* true��ʾ�����Զ�������false��ʾ��Ҫ�ֹ������̼� */
	bool can_auto_update;
	//��Ƭ���Ƿ���������������
	bool stm_can_update;
	/* �°汾������Ϣ */
	char *release_desc;
	/* �°汾URL */
	char *release_url;
	/* �°汾�������� */
	char *release_date;
    //�µ�Ƭ���汾���ص�ַ
    char* stm_release_url;
	////////////////////////////////////////////////////////////////////
	//��Ƭ����ǰ�汾
	cl_version_t stm_cur_version;
	//��Ƭ�������������°汾
	cl_version_t stm_newest_version;
	/////////////////////////////////////////////////////////////////////
	// �豸�����೤ʱ���ˣ���λ��
	u_int32_t uptime;
	// �豸����������Ӷ೤ʱ���ˣ���λ��
	u_int32_t online;
	// �豸�ϻ������೤ʱ���ˣ���λ��
	u_int32_t conn_internet;

	// ��������
	char *ap_ssid;
	// ��������
	char *ap_passwd;

	// ����ӿ���Ϣ
	cl_ni_t wan;
	cl_ni_t lan;

	// ����������Ϣ
	int client_num;
	cl_lan_client_t **clients;
	cl_udp_dev_stat udp_dev_stat;
	
	//���Щͳ����Ϣ
	u_int32_t server_ip;
	char server_domainname[50];
	u_int32_t svn_num;
	u_int8_t dev_status;
	//new add
	u_int32_t DevServerIp;
	u_int32_t StmSoftVersion;
	u_int32_t StmHardVersion;
	u_int32_t LightCur;
	u_int32_t LightYes;
	u_int32_t LightNext;
	u_int16_t IrId;
	u_int32_t AcCur;
	u_int32_t ACVal;
	//��������
	char *compile_date;
	//����ʱ��
	char *compile_time;
		//systime;
	u_int32_t systime;

} cl_dev_stat_t;

typedef struct{
	u_int32_t vol_ad;
	u_int32_t cur_ad;
	u_int32_t cur_ad2;
}cl_pt_stat_t;


typedef struct {
	cl_version_t ver;
	cl_version_t upver;
	cl_version_t hardver;
	u_int32_t svn;
}cl_dev_debug_info_t;


/*
	���ܣ�
		���һ���û�(�豸)
	�������:
		@user_handle: ���ش���豸����ĵ�ַ
		@username: �豸���кŻ��豸�ǳ�
		@passwd: �豸����
		@callback: �ص��������ص��¼�Ϊ UE_xxx
			״̬��Ǩʱ��ص���������¼�ɹ����ɹ�->ʧ�ܡ�����->���ߡ�����->����
		@callback_handle: �ص�ʱ�ش�������
	�������:
		@user_handle: ������豸�ľ����Ӧ�ó������ں���ʹ��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_login(cl_handle_t *user_handle, const char *username, const char *passwd,
							cl_callback_t callback, void *callback_handle);

/*
	���ܣ�
		һ�����ú����һ���û�(�豸)
	�������:
		@user_handle: ���ش���豸����ĵ�ַ
		@username: �豸���кŻ��豸�ǳ�
		@passwd: �豸����
		@callback: �ص��������ص��¼�Ϊ UE_xxx
			״̬��Ǩʱ��ص���������¼�ɹ����ɹ�->ʧ�ܡ�����->���ߡ�����->����
		@callback_handle: �ص�ʱ�ش�������
	�������:
		@user_handle: ������豸�ľ����Ӧ�ó������ں���ʹ��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_smartconf_login(cl_handle_t *user_handle, const char *username, const char *passwd,
							cl_callback_t callback, void *callback_handle);

/*
 ���ܣ�
 	ʹ�ö�ά�����һ���û�(�豸)
 �������:
 	@user_handle: ���ش���豸����ĵ�ַ
 	@callback: �ص��������ص��¼�Ϊ UE_xxx
 ״̬��Ǩʱ��ص���������¼�ɹ����ɹ�->ʧ�ܡ�����->���ߡ�����->����
 	@callback_handle: �ص�ʱ�ش�������
 �������:
 	@user_handle: ������豸�ľ����Ӧ�ó������ں���ʹ��
 ���أ�
 	RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_user_QR_login(cl_handle_t *user_handle, char* sn, const char *qr_code,
                          cl_callback_t callback, void *callback_handle);

/*
	���ܣ�
		�޸�һ���û�(�豸)
	�������:
		@user_handle: ���ش���豸����ĵ�ַ
		@username: �豸���кŻ��豸�ǳ�
		@passwd: �豸����
		@callback: �ص��������ص��¼�Ϊ UE_xxx
			״̬��Ǩʱ��ص���������¼�ɹ����ɹ�->ʧ�ܡ�����->���ߡ�����->����
		@callback_handle: �ص�ʱ�ش�������
	�������:
		@user_handle: ������豸�ľ����Ӧ�ó������ں���ʹ��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_modify_login(cl_handle_t user_handle, const char *passwd,
							cl_callback_t callback, void *callback_handle);

/*
	���ܣ�
		�˳���¼
	�������:
		@user_handle: Ҫ�˳���¼���豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_logout(cl_handle_t user_handle);

/*
	���ܣ�
		ע�⣬���ֻ�����ֶ�ɾ��app���б��豸ʱ����Ҫ���������ԭ������Ҫɾ����ά����Կ��app�����˳�ʱ��cl_user_logout�����˳�
		�˳���¼
	�������:
		@user_handle: Ҫ�˳���¼���豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_qr_logout(cl_handle_t user_handle);

/*
	���ܣ�
		ֱ�������豸��¼��IP��ַ
	�������:
		@user_handle: Ҫ���õ��豸�ľ��
		@ip: ���õ�¼��IP��ַ�������ֽ���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_set_direct_login(cl_handle_t user_handle, u_int32_t ip);

/*
	���ܣ�
		���µ�¼һ̨�豸��һ���û�
	�������:
		@user_handle: Ҫ���µ�¼���豸���û�
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_relogin(cl_handle_t user_handle);


/*
	���ܣ�
		�ֻ��ʺ����һ���豸
	�������:
		@user_handle: �ֻ��ʺž��
		@dev_name: Ҫ��ӵ��豸���ǳƻ�SN��
		@passwd: �豸������
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_add_dev(cl_handle_t user_handle, const char *dev_name, const char *passwd);
    
/*
 ���ܣ�
 	�ֻ��ʺ����һ����ά���豸
 �������:
 	@user_handle: �ֻ��ʺž��
 	@dev_name: Ҫ��ӵ��豸���ǳƻ�SN��
 	@QR_code: �豸����Ķ�ά��
 �������:
 	��
 ���أ�
 	RS_OK: �ɹ�
 	����: ʧ��
 */
CLIB_API RS cl_user_add_QR_dev(cl_handle_t user_handle, const char *dev_sn, const char *QR_code);
/*
	���ܣ�
		�ֻ��ʺ����һ���豸
	�������:
		@user_handle: �ֻ��ʺž��
		@dev_handle: ���汾����ӵ��豸�ľ��
		@dev_name: Ҫ��ӵ��豸���ǳƻ�SN��
		@passwd: �豸������
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_add_dev_v2(cl_handle_t user_handle, cl_handle_t *dev_handle, const char *dev_name, const char *passwd);

/*
	���ܣ�
		�ֻ��ʺ�ɾ��һ���豸
	�������:
		@dev_handle: Ҫɾ�����豸�ľ��
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_del_dev(cl_handle_t dev_handle);

/*
	���ܣ�
		��ȡ���豸��Ϣ(Ŀǰֻ������RF�����´��豸)
	�������:
		@handle: ���豸���
	�������:
		��
	���أ�
		NULL: ʧ��
*/
CLIB_API cl_slave_t *cl_get_slave_info(cl_handle_t handle);

/*
	���ܣ�
		�ͷŴ��豸��Ϣ
	�������:
		@slave: ���豸��Ϣָ��
	�������:
		��
	���أ�
		��
*/
CLIB_API void cl_free_slave_info(cl_slave_t *slave);
#if 1
/*
	���ܣ�
		��ȡ�豸��Ϣ
	�������:
		@user_handle: �豸���
	�������:
		��
	���أ�
		NULL: ʧ��
		����: ָ���豸��Ϣ��ָ��
*/
CLIB_API cl_dev_info_t *cl_user_get_dev_info(cl_handle_t dev_handle);

/*
	���ܣ�
		�ͷ�cl_user_get_dev_info()�������ص��豸��Ϣ
	�������:
		@di: cl_user_get_dev_info()�������ص��豸��Ϣ
	�������:
		��
	���أ�
		��
*/
CLIB_API void cl_user_free_dev_info(cl_dev_info_t *di);
#endif

/*
	���ܣ�
		��ȡ�ʺ���Ϣ
	�������:
		@user_handle: �ʺž��
	�������:
		��
	���أ�
		NULL: ʧ��
		����: �ʺ���Ϣָ�� 
*/
CLIB_API cl_user_t *cl_user_get_info(cl_handle_t user_handle);

/*
	���ܣ�
		��ȡ�ʺ���Ϣ��ֻ��ȡuser������ȡ�豸
	�������:
		@user_handle: �ʺž��
	�������:
		��
	���أ�
		NULL: ʧ��
		����: �ʺ���Ϣָ�� 
*/
CLIB_API cl_user_t *clsdk_get_user_info(cl_handle_t user_handle) ;

/*
	���ܣ�
		�ͷ�cl_user_get_info()�������ص��ʺ���Ϣ
	�������:
		@info: cl_user_get_info()�������ص��ʺ���Ϣ
	�������:
		��
	���أ�
		��
*/
CLIB_API void cl_user_free_info(cl_user_t *info);

/*
	���ܣ�
		���ĳ���豸��ʱ�򣬵�������ӿ�
	�������:
		@dev_handle: ���豸���ߴ��豸��handle 
		@stat: ���ص���ʾ״̬
	�������:
		��
	���أ�
		������ط�RS_OK�����ܽ����豸�����б�
*/
CLIB_API RS cl_user_set_click(cl_handle_t dev_handle, u_int8_t *stat);

/*
	���ܣ�
		�޸��豸�ǳ�(��¼�ɹ������޸�)
		���������첽���ûᴥ��UE_MODIFY_NICKNAME_OK �� UE_MODIFY_NICKNAME_FAIL�¼�

	�������:
		@user_handle: Ҫ�޸��ǳƵ��豸���
		@new_nickname: �µ��ǳ�
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
*/
CLIB_API RS cl_user_modify_nickname(cl_handle_t user_handle, const char *new_nickname);

/*
	���ܣ�
		���������������Ƿ���ͬ
		��������ͬ������
	�������:
		@user_handle: �û�/�豸���
		@passwd: Ҫ��������
	�������:
		��
	���أ�
		true: ��������ȷ
		false: ���������
*/
CLIB_API bool cl_user_same_passwd(cl_handle_t user_handle, const char *passwd);

/*
	���ܣ�
		�޸��豸/�ʺŵ�����(��¼�ɹ������޸�)
		���������첽���ûᴥ�� UE_MODIFY_PASSWD_OK �� UE_MODIFY_PASSWD_FAIL �¼�
	�������:
		@user_handle: Ҫ�޸��ǳƵ��豸���
		@new_passwd: �µ�����
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_modify_passwd(cl_handle_t user_handle, const char *new_passwd);

/*
	���ܣ�
		�޸Ĵ��豸����(��¼�ɹ������޸�)
	�������:
		@user_handle: Ҫ�޸��ǳƵ��豸���
		@new_name: �µ�����
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		UE_SET_NAME_OK
		UE_SET_NAME_FAILED
*/
CLIB_API RS cl_slave_modify_name(cl_handle_t slave_handle, const char *new_name);

/*
	���ܣ�
		�󶨴��豸
	�������:
		@slave_handle: ���豸���
		@passwd: ������
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
*/
CLIB_API RS cl_slave_bind(cl_handle_t slave_handle, const char *passwd);

/*
	���ܣ�
		��󶨴��豸
	�������:
		@slave_handle: ���豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
*/
CLIB_API RS cl_slave_unbind(cl_handle_t slave_handle);

/*
	���ܣ�
		ע��һ���ֻ��û��˺�
	�������:
		@phone_number: �ֻ�����
		@callback: �ص��������ص��¼�Ϊ 
			UE_PHONE_USER_REGISTER_OK��UE_PHONE_USER_REGISTER_FAILED
		@callback_handle: �ص�ʱ�ش�������
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_register(const char *phone_number, 
				cl_callback_t callback, void *callback_handle);
/*
	���ܣ�
		�����ֻ��˺�����
	�������:
		@phone_number: �ֻ�����
		@callback: �ص��������ص��¼�Ϊ 
		@callback_handle: �ص�ʱ�ش�������
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_PHONE_USER_RESET_OK/UE_PHONE_USER_RESET_FAILED��
		�����UE_PHONE_USER_RESET_OK����Ҫ�û�������֤�롢������
*/
CLIB_API RS cl_user_reset_passwd(const char *phone_number, 
				cl_callback_t callback, void *callback_handle);

/*
	���ܣ�
		�ֻ��û�������֤��
	�������:
		@phone_number: �ֻ�����
		@password: ����
		@vcode: ��֤��
		@callback: �ص��������ص��¼�Ϊ 
			UE_PHONE_USER_GOOD_VCODE��UE_PHONE_USER_BAD_VCODE
		@callback_handle: �ص�ʱ�ش�������
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_send_vcode(const char *phone_number, 
				const char *password, const char *vcode,
				cl_callback_t callback, void *callback_handle);

/*
	���ܣ�
		���ֻ�����
	�������:
		@request: ���ֻ���ز���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_BIND_PHONE_REQUEST_OK �ύ������ɹ�
		UE_BIND_PHONE_REQUEST_OK �ύ�����뱻��׼
*/
CLIB_API RS cl_user_bind_phone(cl_handle_t user_handle, cl_bind_phone_t *request);

/*
	���ܣ�
		����/��ֹ��ͨ�û���¼
	�������:
		allow@ ��0��ʾ������ͨ�û���¼��0��ʾ��ֹ��ͨ�û���¼
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: 
*/
CLIB_API RS cl_user_bind_phone_allow_normal(cl_handle_t user_handle, int allow);

/*
	���ܣ�
		��ѯ�Ѱ��ֻ��б�
	�������:
		��
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_BIND_PHONE_LIST �յ��Ѱ��б�
	
*/
CLIB_API RS cl_user_bind_phone_query(cl_handle_t user_handle);
CLIB_API cl_bind_phone_list_t *cl_user_get_bind_phone_list(cl_handle_t user_handle);
CLIB_API void cl_user_free_bind_phone_list(cl_bind_phone_list_t *);
/*
	���ܣ�
		��ѯ���ֻ������б�
	�������:
		��
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_BIND_PHONE_REQUEST_LIST �յ��������б�

*/
CLIB_API RS cl_user_bind_phone_request_query(cl_handle_t user_handle);
CLIB_API cl_bind_phone_request_list_t *cl_user_get_bind_phone_request_list(cl_handle_t user_handle);
CLIB_API void cl_user_free_bind_phone_request_list(cl_bind_phone_request_list_t *list);
/*
	���ܣ�
		�԰��ֻ����������׼/�ܾ�����
	�������:
		action@: BIND_PHONE_ACCEPT ��׼BIND_PHONE_DEN �ܾ�
		uuid@: ������uuid
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_BIND_PHONE_OPERATION_RESULT �������

*/
CLIB_API RS cl_user_bind_phone_operation(cl_handle_t user_handle, char action, const char *uuid);
CLIB_API cl_bind_phone_result_t *cl_user_get_bind_phone_operation_result(cl_handle_t user_handle);
CLIB_API void cl_user_free_bind_phone_operation_result(cl_bind_phone_result_t *result);

/*
	���ܣ�
		��ѯ������ƻ�����ͷ���
	�������:
		cfg@: �������ò���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_APNS_CONFIG UE_APNS_CONFIG_FAIL
*/
CLIB_API RS cl_user_apns_config(cl_handle_t dev_handle, cl_apns_config_t *cfg);

/*
	���ܣ�
		��ѯ������ƻ�����ͷ���
	�������:
		cfg@: �������ò���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ: UE_APNS_CONFIG UE_APNS_CONFIG_FAIL
*/
CLIB_API RS cl_user_apns_config_by_sn(u_int64_t sn, cl_apns_config_t *cfg);


/*
	���ܣ�
		��ȡƻ�����ͷ������ý��
		UE_APNS_CONFIG�¼�ʱ���ñ�����
	�������:
		cfg@: ������Ϣ��������ַ
	�������:
		cfg@ д��������Ϣ����
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_apns_config_get(cl_handle_t dev_handle, cl_apns_config_t *cfg);

/*
	���ܣ�
		���ú�̨ģʽ
	�������:
		background@:true��ʾ���óɺ�̨ģʽ
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_user_background(cl_handle_t user_handle, bool background);

/*
	���ܣ�
		��ȡĳ����豸��ͳ����Ϣ
	�������:
		@slave_handle: ���豸���
	�������:
		��
	���أ�
		NULL: ʧ��
		����: �ɹ���ָ��cl_dev_stat_t�ṹ����Ҫ��cl_free_dev_stat_info�������ͷ�

	�¼�֪ͨ:
		��
*/
CLIB_API cl_dev_stat_t *cl_get_dev_stat_info(cl_handle_t slave_handle);

/*
	���ܣ�
		�ͷ���cl_get_dev_stat_info()�������ص��ڴ���Դ
	�������:
		@info: ��cl_get_dev_stat_info()�������ص��ڴ���Դ
	�������:
		��
	���أ�
		��

	�¼�֪ͨ:
		��
*/
CLIB_API void cl_free_dev_stat_info(cl_dev_stat_t * info);

/*
	���ܣ�
		��һ���豸��Զ��telnet
	�������:
		@slave_handle: ���豸���
		@ip: ��������IP��ַ�������ֽ���
		@port: �������Ķ˿ںţ������ֽ���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_slave_open_telnet(cl_handle_t slave_handle, u_int32_t ip, int port);

/*
	���ܣ�
		Զ������һ���豸
	�������:
		@slave_handle: ���豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_slave_reboot(cl_handle_t slave_handle);

/*
	���ܣ�
		���ÿ�����������Ļ����
	�������:
		@slave_handle: ���豸���
		@on_off: �����
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_805_set_screen_on_off(cl_handle_t slave_handle,bool on_off);

/*
	���ܣ�
		���ÿ�������������
	�������:
		@slave_handle: ���豸���
		@on_off: �����
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_805_set_beep_on_off(cl_handle_t slave_handle,bool on_off);


/*
	���ܣ�
		��ѯ������������Ļ����
	�������:
		@slave_handle: ���豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_805_query_screen_on_off(cl_handle_t slave_handle);

/*
	���ܣ�
		��ѯ��������������
	�������:
		@slave_handle: ���豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_805_query_beep_on_off(cl_handle_t slave_handle);

/*
	���ܣ�
		��ѯ�豸�������汾
	�������:
		@handle: �û����豸�����������ֻ��û���sdk����ѯ�󶨵������豸
		@lang: ���ԣ�Ŀǰ֧��LANG_CH LANG_EN
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����ѯ����
		����: ʧ��
	�¼�֪ͨ:
		UE_DEV_UPGRADE_READY���¼�handleΪslave_handle(���豸Ҳģ����һ��slave)
		����cl_get_dev_stat_info�������Ի�ȡ�°汾��Ϣ
*/
CLIB_API RS cl_dev_upgrade_check(cl_handle_t handle, int lang);

/*
	���ܣ�
		���߷����������豸�Զ�����
	�������:
		@slave_handle: (��)�豸���������UE_DEV_UPGRADE_READY�¼�
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ�����޸�����
		����: ʧ��
	�¼�֪ͨ:
		UE_DEV_UPGRADE_SET_OK��UE_DEV_UPGRADE_SET_FAIL
*/
CLIB_API RS cl_dev_update_set(cl_handle_t slave_handle);

/*
	���ܣ�
		�����豸�����Զ�����
	�������:
		@slave_handle: (��)�豸���������UE_DEV_UPGRADE_READY�¼�
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ��������
		����: ʧ��
	�¼�֪ͨ:
		UE_DEV_UPGRADE_NOW_OK��UE_DEV_UPGRADE_NOW_FAIL
*/
CLIB_API RS cl_dev_update_now(cl_handle_t slave_handle);

/*
	���ܣ�
		�ͻ��˰ѱ����������ļ�ֱ�ӷ��͸��豸����
	�������:
		@handle: �豸���������UE_DEV_UPGRADE_READY�¼�
		@filename: �������ļ���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ��������
		����: ʧ��
	�¼�֪ͨ:
		UE_DEV_UPGRADE_NOW_OK��UE_DEV_UPGRADE_NOW_FAIL
*/
CLIB_API RS cl_dev_update_cli(cl_handle_t handle, char *filename);

/*
	���ܣ�
		�ͻ��˰ѱ����������ļ�ֱ�ӷ��͸��豸����,����Ҫͷ���ļ�
	�������:
		@handle: �豸���������UE_DEV_UPGRADE_READY�¼�
		@filename: �������ļ���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ��������
		����: ʧ��
	�¼�֪ͨ:
		UE_DEV_UPGRADE_NOW_OK��UE_DEV_UPGRADE_NOW_FAIL
*/

CLIB_API RS cl_dev_update_cli_no_head(cl_handle_t handle,char *filename);


/*
	����
*/
CLIB_API cl_pt_stat_t *cl_get_pt_stat_info(cl_handle_t dev_handle);
CLIB_API void cl_free_pt_stat_info(cl_pt_stat_t *p);

/*
	���ܣ�
		������Ƭ������
	�������:
		@handle: �豸���������UE_DEV_UPGRADE_READY�¼�
		@filename: �������ļ���
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ��������
		����: ʧ��
	�¼�֪ͨ:

*/
CLIB_API RS cl_dev_stm_update_cli(cl_handle_t handle, char *filename);

/*
	���ܣ�
		����ָ��sn����
	�������:
		@handle: �豸���������UE_DEV_UPGRADE_READY�¼�
		@filename: �������ļ���
		@num,ָ����sn����
		@sn,ָ����sn����
		@force, �Ƿ�ǿ��������Ĭ��ǿ��
	�������:
		��
	���أ�
		RS_OK: �ɹ�����SDK��SDK��ʼ��������
		����: ʧ��
	�¼�֪ͨ:
		UE_DEV_UPGRADE_NOW_OK��UE_DEV_UPGRADE_NOW_FAIL
*/
CLIB_API RS cl_dev_stm_update_spe_cli(cl_handle_t handle, char *filename, u_int8_t num, u_int64_t *sn, u_int8_t force);

CLIB_API RS cl_dev_active(cl_handle_t handle, u_int8_t *pdata, int len);
CLIB_API RS cl_get_timezone(cl_handle_t dev_handle,u_int32_t *timezone);


CLIB_API RS cl_get_debug_info(cl_handle_t dev_handle,cl_dev_debug_info_t *debug_info);
CLIB_API RS cl_misc_tmp_24hour_line_import(cl_handle_t dev_handle, char *file);
CLIB_API RS cl_misc_humi_24hour_line_import(cl_handle_t dev_handle, char *file);
CLIB_API RS cl_dev_set_irid(cl_handle_t handle, int id);
CLIB_API RS cl_dev_flash_upgrade_query(cl_handle_t handle, flash_block_t *block);
CLIB_API RS cl_dev_flash_upgrade(cl_handle_t handle, u_int32_t num);
CLIB_API RS cl_dev_flash_erase(cl_handle_t handle, u_int32_t num);
/*
* ������ϵ��Դ�ӡ���ؿ���
* @debug_on: 1������0�ر�
* @size:�����ļ���С���ƣ��ֽ�Ϊ��λ��1m=1024*1024
*/
CLIB_API RS cl_dev_nd_debug_set(bool debug_on, u_int32_t size);
/*
* ���������أ��ϲ������л����ã���ӡ��Ӧ��Ϣ��
* @desc:�����л���һЩ���������appզд�ɡ�
*/
CLIB_API RS cl_dev_nd_wan_switch(char *desc);
/*
*ʱ������
*/
CLIB_API RS cl_dev_timezone_set(cl_handle_t handle, int value);

/*
*С������������
*/
CLIB_API RS cl_dev_scc_onoff_set(cl_handle_t handle, int value);
/*
*������Ƭ����������
*/
CLIB_API RS cl_dev_stm_erase(cl_handle_t handle);

/*
*���dns��������
*/
CLIB_API RS cl_dns_cache_clean();

/*
*����������¹������õ�
*/
CLIB_API RS cl_dev_debug_info_set(cl_handle_t handle, cl_air_debug_info_set_t *pinfo);
/*
*���������
*/
CLIB_API RS cl_dev_day_ele_import(cl_handle_t handle, char *filename);

/*
	���ܣ�
		��ȡ�û�����������
	�������:
		@handle: �豸���
		@type: 0����Ҫ��Ϊ�˷�ֹһ���豸���ܳ��ֶ��ʹ�ò�ͬ����user_name�ĵط���Ĭ��Ϊ0��
	�������:
		��
	���أ�
		������Ƴ��ȡ���Чֵ��>0
	�¼�֪ͨ:

*/
CLIB_API int cl_user_name_len_limit_get(cl_handle_t handle, int type);

#ifdef __cplusplus
}
#endif 

#endif

