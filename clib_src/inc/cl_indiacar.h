#ifndef CL_INDIACAR_H
#define CL_INDIACAR_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


typedef enum {
	HDS_IDLE,
	HDS_WAIT_REPLY,
	HDS_RECV,
} HDS_T;

/* Type definitions. */

// ��FLASH���������
#define INDIACAR_HISTORY_MAGIC 0x115588
#define INDIACAR_HITORY_MAX_FILE_SIZE (1024*1024*10)
#define INDIACAR_REATIME_TRIP_REALLOC_NUM	(1000)
#define INDIACAR_REALTIME_ITEM_NUM 16000
//#define INDIACAR_REALTIME_ONE_PACKET_ITEM	50	// һ������50��
#define INDIACAR_REALTIME_ONE_PACKET_ITEM	50	// һ������50��



#pragma pack(push,1)


// ��γ����8�ֽ�����
typedef struct {
	u_int32_t high;
	u_int32_t low;
} latitude_longitud_data_t;
	
typedef struct {
	u_int16_t id;	// ��ǰ���ڵ���ĵڼ����ọ́�0��ʾδ�����κ��ó�
	u_int8_t engine_stat;	// ������״̬0 �ر� 1 ���� 2λ��
	u_int8_t vol;	// �������� DB

	u_int8_t temp_now_degree;	// �����¶�(���϶�)
	u_int8_t temp_avg_degree;	// ƽ���¶�(���϶�)
	u_int8_t temp_max_degree;	// ��ǰ�ó�����¶�(���϶�)

	u_int8_t temp_now_fahrenheit;	// �����¶�(���϶�)
	u_int8_t temp_avg_fahrenheit;	// ƽ���¶�(���϶�)
	u_int8_t temp_max_fahrenheit;	// ��ǰ�ó�����¶�(���϶�)

	u_int8_t speed_now;	// ����
	u_int8_t speed_avg;	// ƽ������
	u_int8_t speed_max;	// �����
	u_int8_t is_runfree;	// �Ƿ��ת
	u_int16_t runfree_time;	// ��ǰ�Ѿ���תʱ��(����)

	u_int16_t total_time;	// �ó��ۼ���ʱ ����

	u_int8_t start_hour;		// ��ǰ�ó̿�ʼʱ�� Сʱ �豸ʱ��
	u_int8_t start_min;			// ��ǰ�ó̿�ʼ���� �豸ʱ��
	u_int16_t total_distance;	// ��ǰ�ó������ (0.1km)
	u_int8_t pad[2];

	latitude_longitud_data_t longitude;	// ������ֵ
	latitude_longitud_data_t latitude;	// γ����ֵ

	u_int8_t power;	// ����γ�ȴη���
	u_int8_t pad1[3];
	u_int32_t journey_date;	// �ó̿�ʼʱ��
} india_car_stat_t;

typedef struct {
	u_int8_t gps;		// GPS ״̬0���ر� 1����λ�� 2����λ�ɹ� 101������ʧ��
	u_int8_t hotspot;	// WIFI�ȵ�״̬0���ر� 1������ 101������ʧ��
	u_int8_t battery_left;// ʣ������ٷֱ� 0-100
	u_int8_t front_camera;	// ǰ������ͷ 0���ر� 1������ 2�������� 101������ʧ��
	u_int8_t rear_camera;	// ��������ͷ0���ر� 1������ 2�������� 101������ʧ��
	u_int8_t microphone;	// 0���ر� 1������ 2��¼���� 101������ʧ��
	u_int8_t power_supply_mode;	// ���緽ʽ0��δ֪ 1���豸�Դ���Դ 2�����ص�Դ
	u_int8_t voltage;		// ��ǰ��ѹ
	u_int8_t tree_axis_accelerometer;	// ���������״̬��0���ر� 1������ 101������ʧ��
	u_int8_t temp; // 0����ȡ������ 1������ 101���쳣 ���������쳣�������Ƿ��ܶ������ݣ�
	u_int8_t pad[2];
} india_car_dev_stat_t;

typedef struct {
	u_int16_t tf_total;
	u_int16_t tf_left;
	u_int16_t flash_total;
	u_int16_t pad;
} india_car_store_stat_t;

typedef struct {
	/*
	
	
	���ܿ��ر�־λ��
		�ܿ��أ�bit0
		���ٸ澯���أ� bit	1
		�г�����ʱ�俪��: bit 2
		�ó̾������޿���: bit 3 
		�ó�ʱ�����޿���: bit 4
		��תʱ�����޿���: bit5
		�������޿��أ�	 bit6
		������ʱ���ѿ���:	bit7
		����Χ����������:	bit8
		��ֹ�����ƻ��������أ�bit9
	
	*/
	u_int32_t onoff; // ���ܿ��ر�־λ
	u_int8_t max_speed;  // ���ٸ澯����(km/h)
	u_int8_t reset_time; // �г�����ʱ��(����)
	u_int16_t max_distance; // �ó̾�������(km)
	u_int16_t max_time; // �ó�ʱ������(����)
	u_int16_t max_freerun_time; // ��תʱ������(����)
	u_int8_t max_vol; // ��������(dB)
	u_int8_t report_period; // ������������(N*��Сʱ)
	u_int8_t geo_fencing_radius; // ����Χ���뾶(km)
	u_int8_t auto_upgrade_onoff; // �豸�Զ���������

	latitude_longitud_data_t longitude;	// ����Χ�����ľ�����ֵ
	latitude_longitud_data_t latitude;	// ����Χ������γ����ֵ

	u_int8_t break_level;	// �����ȼ�
	u_int8_t pad[3];
} india_car_warn_t;

typedef struct {
	u_int8_t stat;
	u_int8_t ssid_len;
	u_int8_t pwd_len;
	u_int8_t pad;
	// �����Ǳ䳤��SSID�����룬������0����
	u_int8_t data[256];
} india_car_wifi_config_t;

typedef struct {
	/*
	
	�����������ͣ�
			  1������ĳ�ó���ϸ��Ϣ
			  2��������ó���ϸ��Ϣ 
		         3������ĳ���ж��ٸ��ó�
			  4��ֹͣ�����ó�����
	*/
	u_int8_t type;
	u_int8_t id;
	u_int8_t pad[2];
	/*
	
	���ڣ����<<16|�·�<<8|���� ,�·ݣ�1-12�������ڣ�1-31�����꣨2000-3000��
	����ǰ�ó���Ϣʱ����������Ϊ1���ó�IDΪ0������Ϊ0��
	����ĳ��ĳ���ó���Ϣʱ����������Ϊ1���ó�ID����0��С��0xFF��
	����ĳ�������ó���Ϣʱ����������Ϊ2���ó�IDΪ0xFF��
	����ĳ��ĳ���ó̼���֮����ó���Ϣʱ����������Ϊ2���ó�IDΪ���ȡ����С�ó�ID������0��.
	*/
	u_int32_t date;
} india_car_history_request_t;

typedef struct {
	u_int8_t type;
	u_int8_t id;
	u_int8_t pad[2];
	u_int32_t date;
	u_int8_t err; // ���� 1����Ч��ѯ���� 2�������� 3. �ó�δ����
	u_int8_t journey_count;
	u_int8_t pad1[2];
} india_car_history_reply_t;

typedef struct {
	/*
		
	���ͣ� 
		   1���ó�ͳ������
		   2���ó���ϸ����
	*/
	u_int8_t type;
	u_int8_t id;
	u_int8_t ver;
	u_int8_t pad;
	u_int32_t date;
	u_int32_t data_len;
	u_int32_t data_idx;
	//u_int8_t data[0];
} india_car_history_notify_t;

// �ó�ͳ������
typedef struct {
	u_int8_t temp_avg_degree;	// ƽ���¶�(���϶�)
	u_int8_t temp_min_degree;	// �ó�����¶�(���϶�)
	u_int8_t temp_max_degree;	// �ó�����¶�(���϶�)

	u_int8_t temp_avg_fahrenheit;	// ƽ���¶�(���϶�)
	u_int8_t temp_max_fahrenheit;	// ��ǰ�ó�����¶�(���϶�)
	u_int8_t temp_min_fahrenheit;	// �ó�����¶�(���϶�)

	
	u_int8_t speed_max;	// �ó������
	u_int8_t speed_avg;	// �ó�ƽ������

	u_int16_t max_freerun_time;	// �ó���߿�תʱ�������ӣ�
	u_int16_t total_time;		// �ó��ۼ���ʱ�����ӣ�
	u_int16_t total_distance;	// ��ǰ�ó������(0.1 km)
	u_int8_t start_hour;		// �ó̿�ʼʱ�䣨Сʱ
	u_int8_t start_mins;		// �ó̿�ʼʱ�䣨����

	u_int8_t pad[2];
	u_int8_t dev_time1;
	u_int8_t dev_time2;

	latitude_longitud_data_t start_longitude;	// �ó���㣨���ȣ�
	latitude_longitud_data_t start_latitude;	// �ó���㣨���ȣ�

	latitude_longitud_data_t end_longitude;	// �ó��յ㣨���ȣ�
	latitude_longitud_data_t end_latitude;	// �ó��յ㣨���ȣ�

	u_int8_t power;
	u_int8_t pad1[3];
	u_int8_t pad2[4];
} india_car_journey_statistics_t;

// �ó���ϸ����
typedef struct {
	u_int8_t temp;	// �¶ȣ����϶ȣ�
	u_int8_t vol;
	u_int8_t speed;
	/*
	
		״̬��
		�������Ƿ����� bit0
				     �Ƿ��ת��bit1
	*/
	u_int8_t stat;
	latitude_longitud_data_t longitude;
	latitude_longitud_data_t latitude;
} india_car_journey_detail_t;

typedef struct {
	u_int8_t major;	/* ���汾 */
    u_int8_t minor;	/* �ΰ汾 */
	u_int16_t url_len;
	u_int32_t svn;
	u_int8_t url[0];
} india_car_dev_upgrade_set_t;

typedef struct {
	u_int8_t major;	/* ���汾 */
    u_int8_t minor;	/* �ΰ汾 */
	u_int16_t pad;
	u_int32_t svn;

	u_int32_t data_len;	/* ��������С */
	/*
	
	����״̬�� 
		   1.����������������
		   2.������������
		   3.׼����װ����������ʱ��Ͽ������ˣ�
	*/
	u_int8_t stat;
	u_int8_t process; // ���ذٷֱ�
	u_int8_t err;
	/*
	
	�����룺
		0��һ������
		1�����ӷ�����ʧ��
		2������������ʧ��
		3��������У�����
	*/
	u_int8_t pad1;
} india_car_dev_upgrade_push_t;

typedef struct {
	u_int8_t type;
	u_int8_t pad[3];
	u_int16_t start_idx;
	u_int16_t end_idx;
} india_car_dev_realtime_requst_t;

typedef struct {
	u_int8_t temp;	// �¶ȣ����϶ȣ�
	u_int8_t vol;
	u_int8_t speed;
	/*
	
		״̬��
		�������Ƿ����� bit0
				     �Ƿ��ת��bit1
	*/
	u_int8_t stat;
	latitude_longitud_data_t longitude;
	latitude_longitud_data_t latitude;
} india_car_dev_realtime_trip_item_t;

typedef struct {
	u_int8_t type;	// ����1 ʵʱ��Ϣ
	u_int8_t jid;
	u_int16_t len;
	u_int16_t start_idx;
	u_int16_t end_idx;
	u_int32_t date;	// �ó̿�ʼʱ��
	india_car_dev_realtime_trip_item_t data[0];
} india_car_dev_realtime_trip_hd_t;


/*
	hd + data + magic
*/
typedef struct {
	u_int32_t magic;
	u_int8_t type; // 1 ͳ����Ϣ 2 ��ϸ��Ϣ
	u_int8_t id;	// �ó�ID��
	u_int8_t ver;	//  �汾��
	u_int8_t pad;
	u_int32_t date;// ���� ���<<16|�·�<<8|���� ,�·ݣ�1-12�������ڣ�1-31�����꣨2000-3000��
	u_int32_t data_len; // �������ݵĳ���
	u_int8_t data[0];
} india_car_history_flash_hd_t;

typedef struct {
	u_int32_t last_time;	// ���һ���յ���ʷ��Ϣ��ʱ���
	u_int8_t type;
	u_int8_t id;	
	u_int8_t journey_count;
	u_int8_t ver;
	u_int32_t date;
	u_int32_t total_size;
	u_int32_t last_idx;
	u_int8_t *data;
} history_download_stat_t;

// ĳ���ж��ٸ��ọ́���Ҫ�ȵ���������ʷ��¼���������3
typedef struct {
	u_int32_t date;	// ��һ��
	u_int8_t count;	// �ж��ٸ��ó�
	u_int8_t pad[3];
} cl_indiacar_jorney_num_t;

typedef struct {
	u_int32_t date;	// ʵʱ���ݵ�����
	u_int8_t jid;	// �ó�ID
	
	u_int32_t max_write_idx;
	u_int32_t min_request_idx;	// ������Ҫ���ĸ�������ʼ��������
	india_car_dev_realtime_trip_item_t items[0];
} indiacar_reatime_record_t;


// �����ʵʱ����
typedef struct {
	u_int32_t num;	// �ж��ٸ�india_car_dev_realtime_trip_item_t �����ṹ��	
	u_int32_t max_items_end_idx;	// ����������ൽ�����±���Ч�����ֵС��num
	u_int32_t last_start_idx;	// ���һ�θ��´����￪ʼ��
	u_int32_t last_end_idx;		// ���һ�θ��´����������
	india_car_dev_realtime_trip_item_t *items;	// ʵʱ�������飬ע����������λ��Ϊ0����ʾ��Ч

	// ��������SDK�Լ�ʹ��
	bool realtime_trip_config_read;
	bool need_realtime;	// �Ƿ�ʼ����ʵʱ����
	
	u_int32_t date;	// ʵʱ���ݵ�����
	u_int8_t jid;	// �ó�ID
	
	u_int32_t min_request_idx;	// ������Ҫ���ĸ�������ʼ��������
	u_int32_t max_write_idx;	// ���һ��push ���ݵĿ�ʼλ��
	bool is_geting;	// ���ڻ�ȡǰ���ʵʱ����
} cl_indiacar_realtime_trip_t;

typedef struct {
	/**���
            1.���ò�����ʹ������Ч����������Ϊʵ�ʳ���
            2.����Զ�̵��ԣ���������Ϊ0������������Ϣ��
            3.�ر�Զ�̵��ԣ���������Ϊ0������������Ϣ��
            4.�ϴ���ǰ���ļ�������־����������Ϊ0������������Ϣ��
            5.�ϴ���ǰ���쳣������־����������Ϊ0������������Ϣ��
     */
	u_int8_t cmd;
	u_int8_t onoff;
	u_int16_t cmd_len;
	u_int16_t gps_time_inv;	// GPS�ɼ����ʱ�� ����
	u_int16_t remote_port;	// Զ�̵��Զ˿�
	u_int32_t remote_ip;	// Զ�̵���IP��ַ
	u_int16_t gps_len_inv;	// GPS�ɼ�������� ��
	u_int16_t file_debug_enable;	// �ļ������������ģ��
	u_int16_t file_debug_level;	// �ļ�������������ȼ�
	u_int16_t file_debug_url_len;	// �ļ��ϴ�URL����
	u_int16_t bps;			// ��Ƶ������(8K 14.4K)
	u_int8_t video_rotate;	// ��Ƶ��ת����0 ����ת 1 2 3�ֱ��ʾ˳ʱ����ת90 180 270 ��
	u_int8_t pad;
	u_int16_t moto_threshold;	// �����жϷ��ͻ���ֵ
	u_int16_t detail_save_inv;	// ��ϸ�ó̱���ʱ���� ����
	u_int8_t power;
	u_int8_t realtime_inv;
	u_int16_t pad1;
	u_int32_t pad2;
	u_int8_t url[256];
} india_car_debug_config_t;

typedef struct {
	u_int8_t action;	// �������� 1 ��ѯĳ���б� 2 �ر�HTTP������
	u_int8_t pad[3];
	u_int32_t date;	// ���<<16|�·�<<8|���� ,�·ݣ�1-12�������ڣ�1-31�����꣨2000-3000��	
} india_car_local_watch_get_t;

typedef struct {
	u_int8_t action;
	u_int8_t err;	// 0 �ɹ�1 ���ھ����� 2 ����Ƶ�ļ�
	u_int16_t port;
	u_int32_t ip;
} india_car_local_watch_reply_t;

typedef struct {
	u_int8_t total;
	u_int8_t index;
	u_int16_t file_num;
	u_int32_t date;
	u_int32_t file_name[0];
} india_car_local_watch_push_t;


typedef struct {
	u_int32_t agent_ip;
	u_int16_t agent_port;
	u_int16_t select_enc;
	u_int8_t key[16]; 
} india_car_video_agent_reply_t;

typedef struct {

	// �յ�UE_INDIACAR_GET_LOCAL_WATCH_INFO = UE_BEGIN + 66,�Ժ��ȡ������������
	u_int8_t errcode;	// ����� 0 �ɹ�1 ���ھ����� 2 ����Ƶ�ļ� 3 ����������ʧ��
	u_int32_t ip;
	u_int32_t port;


	// �յ�UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST = UE_BEGIN + 67 �Ժ��ȡ���漸��
	u_int32_t year;
	u_int32_t month;
	u_int32_t day;
	u_int32_t num;	// �ж��ٸ��ļ���
	u_int32_t name_list[2048];	// ÿ���ļ���4�ֽڣ�Сʱ << 16|����<<8|��

	// ���漸��SDK�Լ���
	u_int8_t _total;
	u_int8_t _index;

	int _write_index;
	u_int32_t _name[2048];
} india_car_local_watch_info_t;

typedef struct {
	// ʵʱ����״̬
	india_car_stat_t car_stat;
	//  ʵʱ�豸״̬
	india_car_dev_stat_t dev_stat;
	// �洢״̬
	india_car_store_stat_t store_stat;
	// ��������
	india_car_warn_t warn;
	// WIFI������Ϣ
	india_car_wifi_config_t wifi_config;

	// ����״̬
	india_car_dev_upgrade_push_t upgrade_stat;

	// ĳ���ж��ٸ��ó�
	cl_indiacar_jorney_num_t jn;

	// ʵʱ����
	cl_indiacar_realtime_trip_t rt;

	// ������Ϣ
	india_car_debug_config_t dc;

	// ������Ƶ���
	india_car_local_watch_info_t wi;

	// ������Щ�ϲ�����
	bool car_stat_init;

	// ����ʱ���õ��Ľṹ�壬�ϲ�����
	history_download_stat_t ds;

	// ��Ƶ���
	void *icc;	// ica_client_t 

	// ����MP4���
	void *mp4_decode;	// ica_mp4_decode_t
} cl_indiacar_info_t;


typedef struct {
	int8_t ids[256];
} cl_indiacar_journey_id_list_t;

typedef struct {
	india_car_history_flash_hd_t hd;
	
} cl_indiacar_history_info_t;


typedef struct {
	u_int8_t action;
	u_int8_t pad[3];
	u_int64_t seek;
	u_int8_t path[256];
} cl_indiancar_mp4_decode_request_t;

typedef struct {
	u_int32_t duration;	// MP4������ʱ�䣬��λ����
	
} cl_indiacar_mp4_info;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	����:���������豸
		
	�������:
		@dev_handle: �豸�ľ��
		@major: ���汾��
		@minor: �ΰ汾��
		@svn: SVN��
		@url: URL�ַ���������С��256
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
	��ע:
*/

CLIB_API RS cl_indiacar_do_dev_upgrade_request(cl_handle_t dev_handle, u_int8_t major, u_int8_t minor, u_int32_t svn, char *url);

 /*
	����:��ѯ��ʷ��¼
		
	�������:
		@dev_handle: �豸�ľ��
		@request_type: ��������
			1 	 ����ĳ�ó���ϸ��Ϣ
			2��������ó���ϸ��Ϣ 
			3������ĳ���ж��ٸ��ó�
			4��ֹͣ�����ó�����
			5���������ó̵�ͳ����Ϣ
		@journey_id:�ó�ID
		@year: ��20xx
		@month: ��1-12
		@day: ��1-31
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
	��ע:
	
	����ǰ�ó���Ϣʱ����������Ϊ1���ó�IDΪ0������Ϊ0��
	����ĳ��ĳ���ó���Ϣʱ����������Ϊ1���ó�ID����0��С��0xFF��
	����ĳ�������ó���Ϣʱ����������Ϊ2���ó�IDΪ0xFF��
	����ĳ��ĳ���ó̼���֮����ó���Ϣʱ����������Ϊ2���ó�IDΪ���ȡ����С�ó�ID������0��.
	����ĳ��ĳ���ó̼���֮����ó�ͳ����Ϣ����������Ϊ5���ó�IDΪ���ȡ����С�ó�ID������0����
*/
CLIB_API RS cl_indiacar_do_history_request(cl_handle_t dev_handle, u_int8_t request_type, u_int8_t journey_id, u_int32_t year, u_int32_t month, u_int32_t day);


// ����ĳ�������ص��ó�ID�Լ����ó�ID��Ӧ���غõ����
/**
	value: -1 : ��û�и��ó�ID�ļ�¼ 
	1: �и��ó�ID��Ӧ��ͳ����Ϣ
	2: �и��ó�ID��Ӧ����ϸ��Ϣ
	3: ͳ�ƺ���ϸ��Ϣ����

		·��: base_path/sn/indiacar_history/date/jorney_id/info.dat
	               base_path/sn/indiacar_history/date/jorney_id/detail.dat
*/
/*
	���� ����ĳ�������ص��ó�ID�Լ����ó�ID��Ӧ���غõ����
		
	�������:
		@dev_handle: �豸�ľ��
		@year: ��20xx
		@month: ��1-12
		@day: ��1-31
	����:
		cl_indiacar_journey_id_list_t: ����ÿ��value��ֵ����һ���ó�ID�������±�Ϊ2��ֵ��ʾ�ó�IDΪ2��״̬
	       0 : ��û�и��ó�ID�ļ�¼ 
		1: �и��ó�ID��Ӧ��ͳ����Ϣ
		2: �и��ó�ID��Ӧ����ϸ��Ϣ
		3: ͳ�ƺ���ϸ��Ϣ����
	��ע:
	*/
CLIB_API cl_indiacar_journey_id_list_t *cl_indiacar_journey_id_stat_get(cl_handle_t dev_handle, u_int32_t year, u_int32_t month, u_int32_t day);
CLIB_API void cl_indiacar_journey_id_list_free(cl_indiacar_journey_id_list_t *list);

/*
	����:��ѯĳ���ĳ���ó�ID��ͳ����Ϣ������ϸ��Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@type: 1Ϊ��ȡͳ����Ϣ 2Ϊ��ȡ��ϸ��Ϣ
		@id:�ó�ID
		@year: ��20xx
		@month: ��1-12
		@day: ��1-31
	�������:
		��
	����:
		
*/
CLIB_API india_car_history_flash_hd_t *cl_indiacar_journey_infomation_get(cl_handle_t dev_handle, u_int8_t type, u_int8_t id, u_int32_t year, u_int32_t month, u_int32_t day);
CLIB_API void cl_indiacar_journey_infomation_free(india_car_history_flash_hd_t *hd);


/*
	����:���ø澯��Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@request: ���ò���
		@id:�ó�ID
		@year: ��20xx
		@month: ��1-12
		@day: ��1-31
	�������:
		��
	����:
		
*/
CLIB_API RS cl_indiacar_warn_set(cl_handle_t dev_handle, india_car_warn_t *request);

/*
	����:����wifi��Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@request: ���ò���
	�������:
		��
	����:
		
*/
CLIB_API RS cl_indiacar_wifi_config(cl_handle_t dev_handle, india_car_wifi_config_t *request);

/*
	����:����ʵʱ����
		
	�������:
		@dev_handle: �豸�ľ��
		@type: 1:��ʼ 0: ����
	�������:
		��
	����:
		
*/
CLIB_API RS cl_indiacar_reatime_trip_request(cl_handle_t dev_handle, u_int8_t type);

/*
	����:��ȡ���һ�������ĳ���ó̸�����Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@jn: ������Ϣ������
	�������:
		��
	����:
		
*/
CLIB_API RS cl_indiacar_get_jorney_count(cl_handle_t dev_handle, cl_indiacar_jorney_num_t *jn);

/*
	����:����һЩ������Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@request: ���ò���
	�������:
		��
	����:
		
*/
CLIB_API RS cl_indiacar_debug_config(cl_handle_t dev_handle, india_car_debug_config_t *request);

CLIB_API RS cl_indiacar_video_start(cl_handle_t dev_handle, bool onoff);

/*
	����:����MP4�ļ�
			
		�������:
			@dev_handle: �豸�ľ��
			@action: 1 ��ʼ���� 0 ����2 ��ͣ 3 �ָ�����
			@path: MP4·��
			@seek: ��ת���Ĳ���ʱ�䣬���뵥λ
		�������:
		����:

		��ע:

			���������¼�
		// ӡ�ȳ�������һ֡MP4֡��Ӧ��BMPͼƬ
		UE_INDIACAR_GET_MP4_PIC = UE_BEGIN + 73,
		// ӡ�ȳ���ȡMP4�ļ�����
		UE_INDIACAR_DECODE_MP4_FINISH = UE_BEGIN + 74,
*/
CLIB_API RS cl_indiacar_mp4_decode(cl_handle_t dev_handle, u_int8_t action, char *path, u_int64_t seek);

CLIB_API RS cl_indiaocar_video_get_picture(cl_handle_t dev_handle, void **pic, u_int32_t *size);
CLIB_API RS cl_indiaocar_video_get_audio(cl_handle_t dev_handle, void **audio, u_int32_t *size);




/*
	����:��ȡMP4�ļ�ת���ɵ�һ֡BMPͼƬ
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		@info: MP4�ļ��Ĳ���
	����:

	��ע:
		
*/
CLIB_API RS cl_indiaocar_video_get_mp4_picture(cl_handle_t dev_handle, cl_indiacar_mp4_info *info, void **pic, u_int32_t *size);

/*
	����:�������������Ƶ��ͨ��
		
	�������:
		@dev_handle: �豸�ľ��
		@action: 1 ����ĳ�����Ƶ�ļ����б� 2 �ر�HTTP������
		@year: ��2000-3000
		@month: ��1-12
		@day: ��1-31
	�������:
		��
	����:
		�յ� UE_INDIACAR_GET_LOCAL_WATCH_INFO
			      UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST �����¼�
		
*/
CLIB_API RS cl_indiaocar_request_local_watch(cl_handle_t dev_handle, u_int8_t action, u_int32_t year, u_int32_t month, u_int32_t day);


/*
	����:����¼��¼��(MP4��ʽ)
		
	�������:
		@dev_handle: �豸�ľ��
		@mp4_path: ¼���ļ�����·��
		@onoff: �������ǹر�¼��
	�������:
		��
	����:
		UE_INDIACAR_IS_RECORDING �������¼�ƣ��ͷ����������
*/
CLIB_API RS cl_indiacar_video_record(cl_handle_t dev_handle, char *mp4_path, bool onoff);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CL_INDIACAR_H */


