#ifndef	__CLIENT_LIB_H__
#define	__CLIENT_LIB_H__


#ifdef __cplusplus
extern "C" {
#endif 

/*
	client libary header.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef INT8 int8_t;
typedef INT16	int16_t;
typedef INT32 int32_t;
typedef UINT8	u_int8_t;
typedef UINT16	u_int16_t;
typedef UINT32	u_int32_t;
typedef UINT64	u_int64_t;
typedef INT32 int32_t;
//#define	LOCALTIME_YEAR_BASE	1900
#define PRIu64	"llu"
#define PRIx64	"llx"
#else
#include <sys/types.h>
#include <inttypes.h>
//#define	LOCALTIME_YEAR_BASE	1970
#endif

#include "cl_type_def.h"

//localtime��ʱ����linux��mac�϶��Ǵ�1900��ʼ
#define	LOCALTIME_YEAR_BASE	1900

#define CONFIG_SN_DICT  1

#if 1 //def __GNUC__
	//typedef unsigned char bool
#ifdef bool
#undef bool
#endif
    #define	bool unsigned char
	#define	true	1
	#define	false	0
#endif

//�Ӹ���������Ǹ�utcת���ĺ�����������
#define USE_TIME_MINS

/*
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;

#define	FALSE	0
#define	TRUE	1
*/

#ifdef WIN32

#if 1
#ifdef CLIB_EXPORTS
#define CLIB_API __declspec(dllexport)
#else
#define CLIB_API __declspec(dllimport)
#endif
#else
#define CLIB_API
#endif

#else

#define CLIB_API

#endif

/*******************************************
	client libary, 2013
	
	module include:
		user
		video
		
	action include: 
		login/logout
		set/get
		init/start/stop
		add/modify/del
		
********************************************/

#ifndef	RS_OK
	/* return status */
	typedef int RS;

	/* �����ɹ���� */
	#define	RS_OK	0
	/* ���� */
	#define	RS_ERROR			-1
	#define	RS_NOT_INIT		-2
	#define	RS_NOT_SUPPORT	-3
	/* ��δ��¼ */
	#define	RS_NOT_LOGIN		-4
	// ��Ч�Ĳ���
	#define	RS_INVALID_PARAM	-5
	// û�п�������
	#define	RS_EMPTY	-6
    //�ڴ����ʧ��
    #define RS_MEMORY_MALLOC_FAIL  -7
	/* ���ʱ�Ѵ��� */
	#define	RS_EXIST	1
	/* δ�ҵ� */
	#define	RS_NOT_FOUND	2
	/*��Ч��Ȩ�ļ�*/
	#define RS_INVALID_LICENSE 3
	/* �豸���� */
	#define RS_OFFLINE 4
	/*����Ȧ��ɾ��ʧ�ܣ���Ϊ��Ĭ��Ȧ��*/
	#define RS_HOME_DEL_FAILED 5
	/*�����Ƿ�ƥ����Ϣ��û��ȡ��=>��ʾ"����ͬ���豸״̬�����Ժ�"*/
	#define RS_IR_SYNING 6
	/*����û��ƥ��=>��ʾ��δƥ����롱*/
	#define RS_IR_HAVE_NO_MATCH 7
	/*�ϲ��·����ݹ���������300k*/
	#define RS_DATA_TOO_BIG		8
#endif

#define	INVALID_HANDLE	0
typedef u_int32_t cl_handle_t;

typedef struct {
	// �¼�
	u_int32_t event;
	// ������
	cl_handle_t obj_handle;
	// �����û�������õ�callback_handle������ش���ȥ
	void *callback_handle;
	// ����š���¼ʧ�ܿ�ULGE_xxx�������Ĵ󲿷ݿɲο�ds_proto.h�е�ERR_XXX
	int err_no;
} cl_event_more_info_t;

/*
	�ص�������
	���¼��ص��У����� callback_handle ����ǿ��ת���� cl_event_more_info_t ָ������ʹ��
*/
typedef void (* cl_callback_t)(u_int32_t event, void *user_handle, void *callback_handle);



/*******************************************

	module: libary
		
********************************************/

/****************************************************
	һЩ�������ݽṹ
 ****************************************************/

typedef enum {
	OT_UNKNOW = 0, /* δ֪���� */
	OT_SLAVE = 1, /* ���豸�������е�һ�������豸 */
	OT_USB_VIDEO = 2, /* ����̨����ͷ */
	OT_EQUIPMENT = 3, /* ���ӡ��յ��ȵ����Ͱ����������豸 */
    OT_KEY = 4,
} object_type_t;

// slave bind status
typedef enum {
	BMS_UNBIND,
	BMS_BINDING,
	BMS_BIND_ONLINE,
	BMS_BIND_OFFLINE,
	// local add: bind error, maybe bad password
	BMS_BIND_ERROR,
	// �Ѿ��󶨹����豸��ԭ�����ز�����
	// �������°󶨣�app��鵽���������󶨹���ֱ�������
	// ������ʾ�������豸���û��һؾ��豸����ʾ
	BMS_REBIND,
	//��½�У���Ҫ������ʾ���ػ����豸�ĳ�ʼ��״̬
	BMS_LOGINING,
	BMS_MAX,
} cl_bms_t;

/*
	һ������Ĺ������֡�
	�ö������������ͷ�������ǲ����������ǵ��ӻ����յ��������ǰ��������������ȵ�
*/
typedef struct {
	// �ö�������: OT_XXX
	object_type_t type;
	
	// ��ǰ״̬, BMS_xxx
	cl_bms_t status;
	
	// �ö�����SDK����Ψһ��ʶ
	cl_handle_t handle;
	// ���ƣ�UTF-8��ʽ
	char *name;
	// �����Ĵ��豸sn
	u_int64_t sn;
	
	// ������������ʹ��
	void *priv;
} cl_obj_t;

// H.264��Ƶ�Զ���ֱ��ʲ�������
typedef struct {
	int width;
	int height;
	// �����0���������� 5 10 15 20 25 0 0 0
	int fps[8];
} cl_video_param_t;

//  һЩϵͳ���ơ��������ƶ�û��������\0������ָ���ݲ���
typedef struct {
	//  �û���������
	int max_user_name_len;
	int max_user_passwd_len;

	// ���豸���֡�ģ������
	int max_mod_name_len;

	// wifi
	int max_wifi_ssid_len;
	int max_wifi_passwd_len;
	
	// ����
	int max_area;
	int max_area_name_len;

	// ����
	int max_scene;
	int max_scene_name_len;

	//  ����
	int max_equipment;
	int max_eq_name_len;
	
	// ÿ�������µİ���
	int max_key_of_eq;
	int max_key_name_len;
	/* ����������Ϣ�ĳ��ȣ���max_key_name_lenһ�� */
	int max_alarm_msg_len;

	// ����
	int max_belter_user;
	int max_belter_user_name_len;
    //��ߣ���λ������
    int min_belter_user_height;
    int max_belter_user_height;
    //���أ���λ��ǧ��
    int min_belter_user_weight;
    int max_belter_user_weight;
    //���䣬��
    int min_belter_user_age;
    int max_belter_user_age;

	/* ���ܲ�����ʱ���� */
	int max_008_timer; 
	int max_008_timer_name_len;

	/* ¼��ʱ�� */
	int max_record_timer; 
	int max_record_timer_name_len;

	/* �龰��ʱ�� */
	int max_scene_timer;
	int max_scene_timer_name_len;

	/* �ֻ��󶨵ļ����������� */
	int max_phone_model_len; /* �ֻ��ͺ� */
	int max_bind_name_len;/*������*/
	int max_bind_message_len;/*������*/
    /*��Ƶ���ͶȲ�������*/
    int max_video_brighness_val;//�������
    int min_video_brighness_val;//��С����
    int max_video_contrast_val;//�Աȶ�
    int max_video_saturation_val;//���Ͷ�
    int max_video_gain_val;//����
    int max_video_roll_speed; //�����Ƶת���ٶ�

	// ��Ƶ�ֱ��ʲ�������
	cl_video_param_t video_param[3];
} cl_limit_t;

#define FLASH_UPGRADE_BLOCK_NUM			(3)
typedef struct flash_block_s{
	u_int32_t flash_addr;
	u_int32_t valid;
	u_int32_t soft_ver;
	u_int32_t svn;
	u_int32_t len;
	u_int32_t crc;	
	u_int32_t run;
}flash_block_t;     

/*********************************************************************************************/


/*
	���ܣ�
		��ʼ���⣬������Ҫ����Դ
	����IN��
		client_type: �ͻ������ͣ���CID_XXX
	���أ�

*/

enum{
	CID_IOS = 0,
	CID_ANDROID = 1,
	CID_WIN32 = 3,
	CID_MAX
};

enum{
	APP_TYPE_DEFAULT, //δ֪
	APP_TYPE_INTEL_HOME,//�ǻۼ�ͥ
	APP_TYPE_SUPER_HOME,//�������ܼҾ�
	APP_TYPE_AIR_PLUG, //�һ���
	APP_TYPE_LINKAGE_SUPPORT = 7,//����֧��app
};

#define APP_TYPE_IWUHOME APP_TYPE_AIR_PLUG
    
#define MAX_UUID_LEN 40
#define MAX_UUID_BIN_LEN 16

#define MAX_TEST_IP_NUM	(10)

#define MAX_TRANS_IP_NUM (50)
    
typedef struct {
	// IN. ������ʱ��
	int timezone;
	// IN. ���ͻ��˵�OEM id. 0-galaxywind, 10-qh
	int vvid;
	//app id
	u_int32_t app_id;
	//oem
	char oem_vendor[16];
	// IN. ������Ψһ��ʶ
	char uuid[MAX_UUID_LEN];
	// ����Ŀ¼����������һЩ��Ϣ������
	char dir[200];
	//��Ӹ������õ�Ŀ¼����Ҫ��Ϊ�˺�ɾ��
	char priv_dir[200];
	// IN. ����
	char *reserved;
	//Ӧ�ó�������
	u_int32_t app_type;
    // �ֻ�����,�ֻ���(�û�ȡ��xxx���ֻ�֮���)���ֻ��ͺ�
    u_int8_t phone_desc[64];

	//IN.app�汾���ϴ����������ж��õ�,a.b.c.d��ʽ major,minor,revise
	u_int8_t app_ver[3];
	
	// OUT. SDK�汾��, a.b.c.d��ʽ
	u_int8_t version[4];
	// OUT. ����
	char desc[64];
	// OUT. SDK��SVN��
	u_int32_t svn;

	// һЩϵͳ����: ���򡢳������������ʺ����볤��...�ȵ�
	cl_limit_t limit;
	// �����ʾ����ģʽ
	int is_pt_mode;
	//test server ip
	u_int8_t test_ip_num;
	u_int32_t test_ip[MAX_TEST_IP_NUM];
} cl_lib_info_t;

CLIB_API RS cl_init(u_int32_t client_type, cl_lib_info_t *info);

/*
	���ܣ�
		��������SDK�Ļ�����Ϣ
	�������:
		@info: 
			@@timezone: ����ʱ��
			@@vvid: ���ҵ�vvid��galaxywindΪ0
			@@uuid: ����Ψһ��ʶ
	�������:
		@info: 
			@@veresion: SDK�汾��
			@@desc: ����
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_set_info(cl_lib_info_t *info);

/*
	���ܣ�
		��ȡSDK�Ļ�����Ϣ
	�������:
		��
	�������:
		@info: ��������Ϣ
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_get_info(cl_lib_info_t *info);

/*
	�ֻ������������ͣ�����: �����ӡ�Wi-Fi��2G/3G����̫
*/
#define	NET_TYPE_NONE	0
#define	NET_TYPE_WIFI	1
#define	NET_TYPE_3G	2
#define	NET_TYPE_ETH	3
/*
	���ܣ�
		���õ�ǰ�ͻ����������������
	�������:
		@net_type: NET_TYPE_xxx���������ͣ�����: �����ӡ�Wi-Fi��2G/3G����̫
		@desc: һЩ����
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_set_net_type(int net_type, char *desc);

/*
	���ܣ�
		����app��ȡ���Ĺ㲥��ַ
	�������:
		@baddr: �㲥��ַ
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�¼�֪ͨ:
		��
*/
CLIB_API RS cl_set_net_baddr(u_int32_t baddr);

/*
	���ܣ�
		�����⣬��ֹ����̣߳��ͷ�������Դ
	������

	���أ�

*/
CLIB_API RS cl_stop();

typedef struct {
	// ���������صĿͻ������°汾��
	char *iphone_newest_version;
	char *android_newest_version;
	
	// ƻ���汾����Ӣ������
	char *desc_iphone_en;
	char *desc_iphone_ch;
	
	// ��׿�汾����Ӣ������
	char *desc_android_en;
	char *desc_android_ch;
} cl_phone_version_t;

/*
	���ܣ�
		��ȡ���������ص��ֻ��汾����Ϣ
	�������:
		��
	�������:
		��
	���أ�
		NULL: ʧ��
		����: �ֻ��汾��Ϣ
	�¼�֪ͨ:
		��
*/
CLIB_API cl_phone_version_t *cl_get_phone_version( );

/*
	���ܣ�
		�ͷ�cl_get_phone_version() �������ص��ڴ�
	�������:
		@info: cl_get_phone_version() �������ص��ڴ�
	�������:
		��
	���أ�
		��
	�¼�֪ͨ:
		��
*/
CLIB_API void cl_free_phone_version(cl_phone_version_t *info);

typedef struct {
    u_int64_t tx_bytes; //�����ֽ���
    u_int64_t rx_bytes; //�հ��ֽ���
    u_int32_t tx_pkts; // ������
    u_int32_t rx_pkts; //�հ���
}cl_traffic_stat_t;

/*
 ���ܣ�
    �����Ƿ����������
 �������:
    @is_enable true������  false:�ر�
 �������:
    ��
 ���أ�
    ��
 �¼�֪ͨ:
    ��
 */
CLIB_API RS cl_set_traffic_stat_enable(bool is_enable);

/*
 ���ܣ�
 
 �������:
    @stat: ״̬ͳ������
    @is_clear_data: �Ƿ����ͳ������
 �������:
    ��
 ���أ�
    ��
 �¼�֪ͨ:
 ��
 */
CLIB_API void cl_get_traffic_stat(cl_traffic_stat_t* stat,bool is_clear_data);

/*
 ���ܣ�
    @�����ֻ������̨
 �������:
 �������:
    ��
 ���أ�
    ��
 �¼�֪ͨ:
    ��
 */
CLIB_API RS cl_set_phone_background(bool is_background);
    CLIB_API cl_traffic_stat_t* cl_get_tcp_cmd_traffic_stat(int* out_num,bool is_clear_data);


#ifdef __cplusplus
}
#endif 

#endif

