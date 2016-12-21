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

// 在FLASH保存的内容
#define INDIACAR_HISTORY_MAGIC 0x115588
#define INDIACAR_HITORY_MAX_FILE_SIZE (1024*1024*10)
#define INDIACAR_REATIME_TRIP_REALLOC_NUM	(1000)
#define INDIACAR_REALTIME_ITEM_NUM 16000
//#define INDIACAR_REALTIME_ONE_PACKET_ITEM	50	// 一次请求50个
#define INDIACAR_REALTIME_ONE_PACKET_ITEM	50	// 一次请求50个



#pragma pack(push,1)


// 经纬度是8字节数据
typedef struct {
	u_int32_t high;
	u_int32_t low;
} latitude_longitud_data_t;
	
typedef struct {
	u_int16_t id;	// 当前处于当天的第几段旅程，0表示未处于任何旅程
	u_int8_t engine_stat;	// 发动机状态0 关闭 1 启动 2位置
	u_int8_t vol;	// 车内音量 DB

	u_int8_t temp_now_degree;	// 车内温度(摄氏度)
	u_int8_t temp_avg_degree;	// 平均温度(摄氏度)
	u_int8_t temp_max_degree;	// 当前旅程最高温度(摄氏度)

	u_int8_t temp_now_fahrenheit;	// 车内温度(华氏度)
	u_int8_t temp_avg_fahrenheit;	// 平均温度(华氏度)
	u_int8_t temp_max_fahrenheit;	// 当前旅程最高温度(华氏度)

	u_int8_t speed_now;	// 车速
	u_int8_t speed_avg;	// 平均车速
	u_int8_t speed_max;	// 最大车速
	u_int8_t is_runfree;	// 是否空转
	u_int16_t runfree_time;	// 当前已经空转时长(分钟)

	u_int16_t total_time;	// 旅程累计用时 分钟

	u_int8_t start_hour;		// 当前旅程开始时间 小时 设备时间
	u_int8_t start_min;			// 当前旅程开始分钟 设备时间
	u_int16_t total_distance;	// 当前旅程总里程 (0.1km)
	u_int8_t pad[2];

	latitude_longitud_data_t longitude;	// 经度数值
	latitude_longitud_data_t latitude;	// 纬度数值

	u_int8_t power;	// 经度纬度次方数
	u_int8_t pad1[3];
	u_int32_t journey_date;	// 旅程开始时间
} india_car_stat_t;

typedef struct {
	u_int8_t gps;		// GPS 状态0：关闭 1：定位中 2：定位成功 101：开启失败
	u_int8_t hotspot;	// WIFI热点状态0：关闭 1：开启 101：开启失败
	u_int8_t battery_left;// 剩余电量百分比 0-100
	u_int8_t front_camera;	// 前置摄像头 0：关闭 1：开启 2：摄像中 101：开启失败
	u_int8_t rear_camera;	// 后置摄像头0：关闭 1：开启 2：摄像中 101：开启失败
	u_int8_t microphone;	// 0：关闭 1：开启 2：录音中 101：开启失败
	u_int8_t power_supply_mode;	// 供电方式0：未知 1：设备自带电源 2：车载电源
	u_int8_t voltage;		// 当前电压
	u_int8_t tree_axis_accelerometer;	// 三轴加速器状态：0：关闭 1：开启 101：开启失败
	u_int8_t temp; // 0：读取数据中 1：正常 101：异常 （正常和异常区别是是否能读出数据）
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
	
	
	功能开关标志位：
		总开关：bit0
		车速告警开关： bit	1
		行程重置时间开关: bit 2
		旅程距离上限开关: bit 3 
		旅程时间上限开关: bit 4
		空转时间上限开关: bit5
		音量上限开关：	 bit6
		报警定时提醒开关:	bit7
		地理围栏报警开关:	bit8
		防止暴力破坏车辆开关：bit9
	
	*/
	u_int32_t onoff; // 功能开关标志位
	u_int8_t max_speed;  // 车速告警上限(km/h)
	u_int8_t reset_time; // 行程重置时间(分钟)
	u_int16_t max_distance; // 旅程距离上限(km)
	u_int16_t max_time; // 旅程时间上限(分钟)
	u_int16_t max_freerun_time; // 空转时间上限(分钟)
	u_int8_t max_vol; // 音量上限(dB)
	u_int8_t report_period; // 报警定期提醒(N*半小时)
	u_int8_t geo_fencing_radius; // 地理围栏半径(km)
	u_int8_t auto_upgrade_onoff; // 设备自动升级开关

	latitude_longitud_data_t longitude;	// 地理围栏中心经度数值
	latitude_longitud_data_t latitude;	// 地理围栏中心纬度数值

	u_int8_t break_level;	// 防爆等级
	u_int8_t pad[3];
} india_car_warn_t;

typedef struct {
	u_int8_t stat;
	u_int8_t ssid_len;
	u_int8_t pwd_len;
	u_int8_t pad;
	// 后面是变长的SSID和密码，各自以0结束
	u_int8_t data[256];
} india_car_wifi_config_t;

typedef struct {
	/*
	
	请求数据类型：
			  1：请求某旅程详细信息
			  2：请求多旅程详细信息 
		         3：请求某天有多少个旅程
			  4：停止传输旅程数据
	*/
	u_int8_t type;
	u_int8_t id;
	u_int8_t pad[2];
	/*
	
	日期：年份<<16|月份<<8|日期 ,月份（1-12），日期（1-31），年（2000-3000）
	请求当前旅程信息时，请求类型为1，旅程ID为0，日期为0；
	请求某天某个旅程信息时，请求类型为1，旅程ID大于0，小于0xFF；
	请求某天所有旅程信息时，请求类型为2，旅程ID为0xFF；
	请求某天某个旅程及其之后的旅程信息时，请求类型为2，旅程ID为想获取的最小旅程ID（大于0）.
	*/
	u_int32_t date;
} india_car_history_request_t;

typedef struct {
	u_int8_t type;
	u_int8_t id;
	u_int8_t pad[2];
	u_int32_t date;
	u_int8_t err; // 错误： 1：无效查询参数 2：无数据 3. 旅程未结束
	u_int8_t journey_count;
	u_int8_t pad1[2];
} india_car_history_reply_t;

typedef struct {
	/*
		
	类型： 
		   1：旅程统计数据
		   2：旅程详细数据
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

// 旅程统计数据
typedef struct {
	u_int8_t temp_avg_degree;	// 平均温度(摄氏度)
	u_int8_t temp_min_degree;	// 旅程最低温度(摄氏度)
	u_int8_t temp_max_degree;	// 旅程最高温度(摄氏度)

	u_int8_t temp_avg_fahrenheit;	// 平均温度(华氏度)
	u_int8_t temp_max_fahrenheit;	// 当前旅程最高温度(华氏度)
	u_int8_t temp_min_fahrenheit;	// 旅程最低温度(华氏度)

	
	u_int8_t speed_max;	// 旅程最大车速
	u_int8_t speed_avg;	// 旅程平均车速

	u_int16_t max_freerun_time;	// 旅程最高空转时长（分钟）
	u_int16_t total_time;		// 旅程累计用时（分钟）
	u_int16_t total_distance;	// 当前旅程总里程(0.1 km)
	u_int8_t start_hour;		// 旅程开始时间（小时
	u_int8_t start_mins;		// 旅程开始时间（分钟

	u_int8_t pad[2];
	u_int8_t dev_time1;
	u_int8_t dev_time2;

	latitude_longitud_data_t start_longitude;	// 旅程起点（经度）
	latitude_longitud_data_t start_latitude;	// 旅程起点（经度）

	latitude_longitud_data_t end_longitude;	// 旅程终点（经度）
	latitude_longitud_data_t end_latitude;	// 旅程终点（经度）

	u_int8_t power;
	u_int8_t pad1[3];
	u_int8_t pad2[4];
} india_car_journey_statistics_t;

// 旅程详细数据
typedef struct {
	u_int8_t temp;	// 温度（摄氏度）
	u_int8_t vol;
	u_int8_t speed;
	/*
	
		状态：
		发动机是否工作： bit0
				     是否空转：bit1
	*/
	u_int8_t stat;
	latitude_longitud_data_t longitude;
	latitude_longitud_data_t latitude;
} india_car_journey_detail_t;

typedef struct {
	u_int8_t major;	/* 主版本 */
    u_int8_t minor;	/* 次版本 */
	u_int16_t url_len;
	u_int32_t svn;
	u_int8_t url[0];
} india_car_dev_upgrade_set_t;

typedef struct {
	u_int8_t major;	/* 主版本 */
    u_int8_t minor;	/* 次版本 */
	u_int16_t pad;
	u_int32_t svn;

	u_int32_t data_len;	/* 升级包大小 */
	/*
	
	升级状态： 
		   1.连接升级服务器中
		   2.下载升级包中
		   3.准备安装升级包（此时会断开连接了）
	*/
	u_int8_t stat;
	u_int8_t process; // 下载百分比
	u_int8_t err;
	/*
	
	错误码：
		0：一切正常
		1：连接服务器失败
		2：下载升级包失败
		3：升级包校验出错
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
	u_int8_t temp;	// 温度（摄氏度）
	u_int8_t vol;
	u_int8_t speed;
	/*
	
		状态：
		发动机是否工作： bit0
				     是否空转：bit1
	*/
	u_int8_t stat;
	latitude_longitud_data_t longitude;
	latitude_longitud_data_t latitude;
} india_car_dev_realtime_trip_item_t;

typedef struct {
	u_int8_t type;	// 类型1 实时信息
	u_int8_t jid;
	u_int16_t len;
	u_int16_t start_idx;
	u_int16_t end_idx;
	u_int32_t date;	// 旅程开始时间
	india_car_dev_realtime_trip_item_t data[0];
} india_car_dev_realtime_trip_hd_t;


/*
	hd + data + magic
*/
typedef struct {
	u_int32_t magic;
	u_int8_t type; // 1 统计信息 2 详细信息
	u_int8_t id;	// 旅程ID号
	u_int8_t ver;	//  版本号
	u_int8_t pad;
	u_int32_t date;// 日期 年份<<16|月份<<8|日期 ,月份（1-12），日期（1-31），年（2000-3000）
	u_int32_t data_len; // 后续数据的长度
	u_int8_t data[0];
} india_car_history_flash_hd_t;

typedef struct {
	u_int32_t last_time;	// 最后一次收到历史信息的时间戳
	u_int8_t type;
	u_int8_t id;	
	u_int8_t journey_count;
	u_int8_t ver;
	u_int32_t date;
	u_int32_t total_size;
	u_int32_t last_idx;
	u_int8_t *data;
} history_download_stat_t;

// 某天有多少个旅程，需要先调用请求历史记录里面的类型3
typedef struct {
	u_int32_t date;	// 哪一天
	u_int8_t count;	// 有多少个旅程
	u_int8_t pad[3];
} cl_indiacar_jorney_num_t;

typedef struct {
	u_int32_t date;	// 实时数据的日期
	u_int8_t jid;	// 旅程ID
	
	u_int32_t max_write_idx;
	u_int32_t min_request_idx;	// 现在需要从哪个索引开始请求数据
	india_car_dev_realtime_trip_item_t items[0];
} indiacar_reatime_record_t;


// 请求的实时数据
typedef struct {
	u_int32_t num;	// 有多少个india_car_dev_realtime_trip_item_t 这样结构体	
	u_int32_t max_items_end_idx;	// 数组里面最多到哪里下标有效，这个值小于num
	u_int32_t last_start_idx;	// 最后一次更新从哪里开始的
	u_int32_t last_end_idx;		// 最后一次更新从哪里结束的
	india_car_dev_realtime_trip_item_t *items;	// 实时数据数组，注意如果里面的位置为0，表示无效

	// 下面数据SDK自己使用
	bool realtime_trip_config_read;
	bool need_realtime;	// 是否开始请求实时数据
	
	u_int32_t date;	// 实时数据的日期
	u_int8_t jid;	// 旅程ID
	
	u_int32_t min_request_idx;	// 现在需要从哪个索引开始请求数据
	u_int32_t max_write_idx;	// 最后一次push 数据的开始位置
	bool is_geting;	// 正在获取前面的实时数据
} cl_indiacar_realtime_trip_t;

typedef struct {
	/**命令：
            1.配置参数，使参数生效，参数长度为实际长度
            2.启用远程调试，参数长度为0，以下配置信息无
            3.关闭远程调试，参数长度为0，以下配置信息无
            4.上传当前的文件调试日志，参数长度为0，以下配置信息无
            5.上传当前的异常崩溃日志，参数长度为0，以下配置信息无
     */
	u_int8_t cmd;
	u_int8_t onoff;
	u_int16_t cmd_len;
	u_int16_t gps_time_inv;	// GPS采集间隔时间 毫秒
	u_int16_t remote_port;	// 远程调试端口
	u_int32_t remote_ip;	// 远程调试IP地址
	u_int16_t gps_len_inv;	// GPS采集间隔距离 米
	u_int16_t file_debug_enable;	// 文件调试输出允许模块
	u_int16_t file_debug_level;	// 文件调试允许输出等级
	u_int16_t file_debug_url_len;	// 文件上传URL长度
	u_int16_t bps;			// 音频比特率(8K 14.4K)
	u_int8_t video_rotate;	// 视频旋转方向0 不旋转 1 2 3分别表示顺时针旋转90 180 270 度
	u_int8_t pad;
	u_int16_t moto_threshold;	// 三轴判断发送机阀值
	u_int16_t detail_save_inv;	// 详细旅程保存时间间隔 毫秒
	u_int8_t power;
	u_int8_t realtime_inv;
	u_int16_t pad1;
	u_int32_t pad2;
	u_int8_t url[256];
} india_car_debug_config_t;

typedef struct {
	u_int8_t action;	// 请求类型 1 查询某天列表 2 关闭HTTP服务器
	u_int8_t pad[3];
	u_int32_t date;	// 年份<<16|月份<<8|日期 ,月份（1-12），日期（1-31），年（2000-3000）	
} india_car_local_watch_get_t;

typedef struct {
	u_int8_t action;
	u_int8_t err;	// 0 成功1 不在局域网 2 无视频文件
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

	// 收到UE_INDIACAR_GET_LOCAL_WATCH_INFO = UE_BEGIN + 66,以后读取下面三个数据
	u_int8_t errcode;	// 错误号 0 成功1 不在局域网 2 无视频文件 3 服务器启动失败
	u_int32_t ip;
	u_int32_t port;


	// 收到UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST = UE_BEGIN + 67 以后读取下面几个
	u_int32_t year;
	u_int32_t month;
	u_int32_t day;
	u_int32_t num;	// 有多少个文件名
	u_int32_t name_list[2048];	// 每个文件名4字节，小时 << 16|分钟<<8|秒

	// 下面几个SDK自己用
	u_int8_t _total;
	u_int8_t _index;

	int _write_index;
	u_int32_t _name[2048];
} india_car_local_watch_info_t;

typedef struct {
	// 实时车辆状态
	india_car_stat_t car_stat;
	//  实时设备状态
	india_car_dev_stat_t dev_stat;
	// 存储状态
	india_car_store_stat_t store_stat;
	// 报警参数
	india_car_warn_t warn;
	// WIFI配置信息
	india_car_wifi_config_t wifi_config;

	// 升级状态
	india_car_dev_upgrade_push_t upgrade_stat;

	// 某天有多少个旅程
	cl_indiacar_jorney_num_t jn;

	// 实时数据
	cl_indiacar_realtime_trip_t rt;

	// 调试信息
	india_car_debug_config_t dc;

	// 本地视频相关
	india_car_local_watch_info_t wi;

	// 下面这些上层无视
	bool car_stat_init;

	// 下载时候用到的结构体，上层无视
	history_download_stat_t ds;

	// 视频相关
	void *icc;	// ica_client_t 

	// 播放MP4相关
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
	u_int32_t duration;	// MP4播放总时间，单位毫秒
	
} cl_indiacar_mp4_info;

#pragma pack(pop)

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	功能:请求升级设备
		
	输入参数:
		@dev_handle: 设备的句柄
		@major: 主版本号
		@minor: 次版本号
		@svn: SVN号
		@url: URL字符串，必须小于256
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
	备注:
*/

CLIB_API RS cl_indiacar_do_dev_upgrade_request(cl_handle_t dev_handle, u_int8_t major, u_int8_t minor, u_int32_t svn, char *url);

 /*
	功能:查询历史记录
		
	输入参数:
		@dev_handle: 设备的句柄
		@request_type: 请求类型
			1 	 请求某旅程详细信息
			2：请求多旅程详细信息 
			3：请求某天有多少个旅程
			4：停止传输旅程数据
			5：请求多个旅程的统计信息
		@journey_id:旅程ID
		@year: 年20xx
		@month: 月1-12
		@day: 日1-31
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
	备注:
	
	请求当前旅程信息时，请求类型为1，旅程ID为0，日期为0；
	请求某天某个旅程信息时，请求类型为1，旅程ID大于0，小于0xFF；
	请求某天所有旅程信息时，请求类型为2，旅程ID为0xFF；
	请求某天某个旅程及其之后的旅程信息时，请求类型为2，旅程ID为想获取的最小旅程ID（大于0）.
	请求某天某个旅程及其之后的旅程统计信息，请求类型为5，旅程ID为想获取的最小旅程ID（大于0）。
*/
CLIB_API RS cl_indiacar_do_history_request(cl_handle_t dev_handle, u_int8_t request_type, u_int8_t journey_id, u_int32_t year, u_int32_t month, u_int32_t day);


// 请求某天已下载的旅程ID以及该旅程ID对应下载好的情况
/**
	value: -1 : 还没有该旅程ID的记录 
	1: 有该旅程ID对应的统计信息
	2: 有该旅程ID对应的详细信息
	3: 统计和详细信息都有

		路径: base_path/sn/indiacar_history/date/jorney_id/info.dat
	               base_path/sn/indiacar_history/date/jorney_id/detail.dat
*/
/*
	功能 请求某天已下载的旅程ID以及该旅程ID对应下载好的情况
		
	输入参数:
		@dev_handle: 设备的句柄
		@year: 年20xx
		@month: 月1-12
		@day: 日1-31
	返回:
		cl_indiacar_journey_id_list_t: 里面每个value的值表述一个旅程ID，比如下标为2的值表示旅程ID为2的状态
	       0 : 还没有该旅程ID的记录 
		1: 有该旅程ID对应的统计信息
		2: 有该旅程ID对应的详细信息
		3: 统计和详细信息都有
	备注:
	*/
CLIB_API cl_indiacar_journey_id_list_t *cl_indiacar_journey_id_stat_get(cl_handle_t dev_handle, u_int32_t year, u_int32_t month, u_int32_t day);
CLIB_API void cl_indiacar_journey_id_list_free(cl_indiacar_journey_id_list_t *list);

/*
	功能:查询某天的某个旅程ID的统计信息或者详细信息
		
	输入参数:
		@dev_handle: 设备的句柄
		@type: 1为获取统计信息 2为获取详细信息
		@id:旅程ID
		@year: 年20xx
		@month: 月1-12
		@day: 日1-31
	输出参数:
		无
	返回:
		
*/
CLIB_API india_car_history_flash_hd_t *cl_indiacar_journey_infomation_get(cl_handle_t dev_handle, u_int8_t type, u_int8_t id, u_int32_t year, u_int32_t month, u_int32_t day);
CLIB_API void cl_indiacar_journey_infomation_free(india_car_history_flash_hd_t *hd);


/*
	功能:设置告警信息
		
	输入参数:
		@dev_handle: 设备的句柄
		@request: 配置参数
		@id:旅程ID
		@year: 年20xx
		@month: 月1-12
		@day: 日1-31
	输出参数:
		无
	返回:
		
*/
CLIB_API RS cl_indiacar_warn_set(cl_handle_t dev_handle, india_car_warn_t *request);

/*
	功能:配置wifi信息
		
	输入参数:
		@dev_handle: 设备的句柄
		@request: 配置参数
	输出参数:
		无
	返回:
		
*/
CLIB_API RS cl_indiacar_wifi_config(cl_handle_t dev_handle, india_car_wifi_config_t *request);

/*
	功能:请求实时数据
		
	输入参数:
		@dev_handle: 设备的句柄
		@type: 1:开始 0: 结束
	输出参数:
		无
	返回:
		
*/
CLIB_API RS cl_indiacar_reatime_trip_request(cl_handle_t dev_handle, u_int8_t type);

/*
	功能:获取最近一次请求的某天旅程个数信息
		
	输入参数:
		@dev_handle: 设备的句柄
		@jn: 个数信息在里面
	输出参数:
		无
	返回:
		
*/
CLIB_API RS cl_indiacar_get_jorney_count(cl_handle_t dev_handle, cl_indiacar_jorney_num_t *jn);

/*
	功能:配置一些调试信息
		
	输入参数:
		@dev_handle: 设备的句柄
		@request: 配置参数
	输出参数:
		无
	返回:
		
*/
CLIB_API RS cl_indiacar_debug_config(cl_handle_t dev_handle, india_car_debug_config_t *request);

CLIB_API RS cl_indiacar_video_start(cl_handle_t dev_handle, bool onoff);

/*
	功能:播放MP4文件
			
		输入参数:
			@dev_handle: 设备的句柄
			@action: 1 开始播放 0 结束2 暂停 3 恢复播放
			@path: MP4路径
			@seek: 跳转到的播放时间，毫秒单位
		输出参数:
		返回:

		备注:

			会有两个事件
		// 印度车解析出一帧MP4帧对应的BMP图片
		UE_INDIACAR_GET_MP4_PIC = UE_BEGIN + 73,
		// 印度车读取MP4文件结束
		UE_INDIACAR_DECODE_MP4_FINISH = UE_BEGIN + 74,
*/
CLIB_API RS cl_indiacar_mp4_decode(cl_handle_t dev_handle, u_int8_t action, char *path, u_int64_t seek);

CLIB_API RS cl_indiaocar_video_get_picture(cl_handle_t dev_handle, void **pic, u_int32_t *size);
CLIB_API RS cl_indiaocar_video_get_audio(cl_handle_t dev_handle, void **audio, u_int32_t *size);




/*
	功能:读取MP4文件转换成的一帧BMP图片
		
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		@info: MP4文件的参数
	返回:

	备注:
		
*/
CLIB_API RS cl_indiaocar_video_get_mp4_picture(cl_handle_t dev_handle, cl_indiacar_mp4_info *info, void **pic, u_int32_t *size);

/*
	功能:请求局域网看视频的通道
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 1 请求某天的视频文件名列表 2 关闭HTTP服务器
		@year: 年2000-3000
		@month: 月1-12
		@day: 日1-31
	输出参数:
		无
	返回:
		收到 UE_INDIACAR_GET_LOCAL_WATCH_INFO
			      UE_INDIACAR_GET_LOCAL_WATCH_INFO_LIST 两个事件
		
*/
CLIB_API RS cl_indiaocar_request_local_watch(cl_handle_t dev_handle, u_int8_t action, u_int32_t year, u_int32_t month, u_int32_t day);


/*
	功能:请求录制录像(MP4格式)
		
	输入参数:
		@dev_handle: 设备的句柄
		@mp4_path: 录像文件保存路径
		@onoff: 开启还是关闭录像
	输出参数:
		无
	返回:
		UE_INDIACAR_IS_RECORDING 如果正在录制，就返回这个错误
*/
CLIB_API RS cl_indiacar_video_record(cl_handle_t dev_handle, char *mp4_path, bool onoff);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CL_INDIACAR_H */


