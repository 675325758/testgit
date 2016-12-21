#ifndef	__CL_VIDEO_H__
#define	__CL_VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"


// video event
enum {
	VE_BEGIN = 200,
	// 视频连接建立，并收到图象。需要告知是否支持声音
	VE_ESTABLISH = VE_BEGIN + 1,
	// 获取到一个图片
	VE_GET_PICTURE = VE_BEGIN + 2,
	// 获取到一个声音片段
	VE_GET_SOUND = VE_BEGIN + 3,
	// 获取发言权成功
	VE_TALK_REQ_SUCCESS = VE_BEGIN + 4,

	VE_REC_TIMER_SET_OK = VE_BEGIN + 5,
	VE_REC_TIMER_SET_FAILED = VE_BEGIN + 6,
	VE_REC_TIMER_MODIFY = VE_BEGIN + 7,
    //设置视频饱和度成功
    VE_SET_V4L2_OK = VE_BEGIN + 8,
    //设置云台转速成功
    VE_SET_ROLL_SPEED_OK = VE_BEGIN + 9,
	
	// 视频出错
	VE_ERROR = VE_BEGIN + 50,
	// 获取发言权失败
	VE_TALK_REQ_FAILED,
    //设置视频饱和度失败
    VE_SET_V4L2_FAILED,
    //设置云台转速失败
    VE_SET_ROLL_SPEED_FAILED,
    //获取录像列表成功
    VE_VTAP_GET_LIST_OK,
    //获取录像列表失败
    VE_VTAP_GET_LIST_FAILED,
    VE_VTAP_SET_POS_FAILED,
    //视频查看出现错误
    VE_VTAP_CHECK_ERR,
	VE_END = VE_BEGIN + 99
};


/*
	USB摄像头相关信息
*/
typedef struct {
	cl_obj_t obj;
} cl_usb_video_t;



/*
	功能：
		开始视频连接
	参数IN：
		slave: 要观看视频的从设备的指针
		callback：回调函数
	参数OUT：
		无
	返回：
		VIDEO_XXX
*/
CLIB_API RS cl_video_start(cl_handle_t video_handle, cl_callback_t callback, void *handle);

CLIB_API RS cl_video_stop(cl_handle_t handle);

/*
	功能：
		获取看视频时的最后一帧图片。H264返回的是BMP格式的，MJPG的返回的是JPG格式的。
*/
CLIB_API RS cl_video_get_picture(cl_handle_t handle, void **pic, u_int32_t *size);

/**************************************************************************************************/

typedef struct {
	void *data;
	u_int32_t len;
	// 通道数，一般为1
	u_int8_t channels;
	// 位宽，一般为16位
	u_int8_t bits;
	/* 采样率 */
	u_int16_t samples;
	// 是否是重复的数据
	u_int32_t repeat;
} cl_sound_data_t;

/*
	取回下一段声音
*/
CLIB_API RS cl_audio_get_sound(cl_handle_t handle, cl_sound_data_t *sd);

/*
	发送声音
*/
CLIB_API RS cl_audio_put_sound(cl_handle_t handle, cl_sound_data_t *sd);

/**************************************************************************************************/

/*
	请求发言权
*/
CLIB_API RS cl_audio_request_speek(cl_handle_t handle);
/*
	释放发言权
*/
CLIB_API RS cl_audio_release_speek(cl_handle_t handle);

/**************************************************************************************************/

#ifndef	QUALITY_SMART
	// 智能(jpeg), 普清(h264)
	#define	QUALITY_SMART	0
	// 流畅(jpeg)，标清(h264)
	#define	QUALITY_SMOOTH	1
	// 清晰(jpeg)，高清(h264)
	#define	QUALITY_CLEAR	2
	//720p(h264)
	#define QUALITY_720P 3
#endif


typedef struct {
	// QUALITY_xxx
	u_int16_t quality;
	u_int16_t width;
	u_int16_t height;
	u_int16_t fps;
} cl_video_quality_t;

/*
	功能：
		设置看视频时的当前画质
	参数IN：
		slave: 相关从设备
		qulity: 质量，见QUALITY_xxx
		width: 视频宽度像素。合法的有0，320，640，1280。0表示由设备端根据qulity选择
		height: 视频高度像素。合法的有0，240，480，720。0表示由设备端根据qulity选择
		fps: 帧率，合法的为0，5，10，15，20，25，30。0表示由设备端根据qulity选择
	参数OUT：
		无
	返回：
		RS_OK: 成功
		其它：失败		
*/
CLIB_API RS cl_video_set_quality(cl_handle_t handle, cl_video_quality_t *q);

/**************************************************************************************************/

/*
	功能:
		图像翻转
	参数IN:
		@slave_handle: 要翻转的摄像头
	参数OUT：
		无
	返回：
		RS_OK: 成功
		其它：失败		
*/
CLIB_API RS cl_video_flip(cl_handle_t handle);

/**************************************************************************************************/

typedef enum {
	ptz_roll_stop = 0,
	ptz_roll_left = 1,
	ptz_roll_right = 2,
	ptz_roll_up = 3,
	ptz_roll_down = 4
} ptz_roll_t;
	
/*
	功能:
		旋转云台
	参数:
		@left_right: 
			ptz_roll_stop: 停止左右转动
			ptz_roll_right: 右转
			ptz_roll_left: 左转
		@up_down:
			ptz_roll_stop: 停止上下转动
			ptz_roll_down: 下转
			ptz_roll_up: 上转
*/
CLIB_API RS cl_video_ptz_roll(cl_handle_t handle, ptz_roll_t left_right, ptz_roll_t up_down);

/*
	功能:
		开始旋转云台
	参数:
		@left_right: 
			ptz_roll_stop: 停止左右转动
			ptz_roll_right: 右转
			ptz_roll_left: 左转
		@up_down:
			ptz_roll_stop: 停止上下转动
			ptz_roll_down: 下转
			ptz_roll_up: 上转
*/
CLIB_API RS cl_video_ptz_roll_start(cl_handle_t handle, ptz_roll_t left_right, ptz_roll_t up_down);

/*
	功能:
		停止旋转云台，与cl_video_ptz_roll_start配套使用
	参数:
	
*/
CLIB_API RS cl_video_ptz_roll_stop(cl_handle_t handle);


/**************************************************************************************************/
    
typedef struct {
    int32_t brightness_val;//亮度
    u_int32_t contrast_val; //对比度
    u_int32_t saturation_val; //饱和度
    u_int32_t gain_val; //补偿
}cl_video_saturation_t;

typedef struct {
	/* 画质 */
	u_int8_t is_custom_quality; /* 是否是自定义的画质 */
	u_int8_t quality; /* 画质等级: QUALITY_XXX */
	u_int16_t width;
	u_int16_t height;
	u_int16_t fps;
	// 观看视频的客户端数
	u_int16_t client_count;
	u_int8_t pad[2];
	/* 码率 */
	u_int32_t bitrate;
    /*云台转速*/
    u_int8_t roll_speed;
    /* 视频饱和度 */
    cl_video_saturation_t vs_stat;
	/* 保留 */
	u_int8_t resv[128];
} cl_video_stat_t;
    
typedef struct {
    u_int32_t begin_time;
    u_int32_t duration;
}cl_vtap_t;

typedef struct {
    u_int32_t total_num;
    cl_vtap_t vtap[0];
}cl_vtap_list_t;
/*
	功能：
		获取看视频时的当前统计
*/
CLIB_API RS cl_video_get_stat(cl_handle_t handle, cl_video_stat_t *st);

/**************************************************************************************************
	  定时器等相关信息 
 **************************************************************************************************/


typedef struct {
	u_int8_t id;		/* 策略ID */
	u_int8_t enable;	/* 使能/禁用 本规则 */
	u_int8_t is_once;	/* 是一次性规则还是周期性规则。一次性规则忽略wday、hours、minute, 周期性规则忽略location_time */
	u_int8_t wday;		/* bit 0-6位对应星期天到星期六 */
	u_int8_t hour;		/* 小时 0-23 */
	u_int8_t minute;	/* 分钟 0-59 */
	u_int16_t duration;/* 持续多久(分钟) */
	u_int32_t location_time; /* 一次性规则的触发时间 */
	char *name;		/* 策略名字, UTF-8格式 */
} cl_vrt_item_t;


// 0: 待机
#define	REC_STA_INIT	0
// 1:挂载失败或者tf卡未找到
#define	REC_STA_ERROR1	1
// 2:video进程未启动
#define	REC_STA_ERROR2	2
// 3:准备启动ffmpeg
#define	REC_STA_READY	3
// 4:ffmpeg已经启动
#define	REC_STA_RUN	4

typedef struct {
	// 视频是否建立成功，只有建立成功，下面的is_h264、has_audio、has_audio_speek才准确，否则勿用
	bool has_establish;
	// 是否支持云台
	bool has_ptz;
	// 是H26编码还是MJPG的
	bool is_h264;
	// 是否支持声音
	bool has_audio;
	// 是否支持对讲
	bool has_audio_speek;

	// 是否使能录像功能
	u_int8_t record_enable;
	// 录像状态: REC_STA_XXX
	u_int8_t record_status;
	u_int8_t pad[1];
	
	// 定时录像规则有多少条
	u_int32_t num_timer;
	// 定时录像规则指针数组
	cl_vrt_item_t **timer;
} cl_video_info_t;

/*
	功能:
		录像的总开关
	输入参数:
		@slave_handle: 遥控插座的句柄
		@on: 1表示使能录像规则，0表示禁止录像规则
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_video_rec_timer_turn_on(cl_handle_t slave_handle, bool on);

/*
	功能:
		添加定时定时录像规则
	输入参数:
		@slave_handle: 遥控插座的句柄
		@timer: 要添加的定时录像规则。
			timer->name必须为UTF-8格式，长度小于64字节。
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_video_rec_timer_add(cl_handle_t slave_handle, cl_vrt_item_t *timer, int32_t tz);

/*
	功能:
		修改定时录像规则
	输入参数:
		@slave_handle: 遥控插座的句柄
		@timer: 要修改的定时录像规则。
			timer->name必须为UTF-8格式，长度小于64字节。
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_video_rec_timer_modify(cl_handle_t slave_handle, cl_vrt_item_t *timer, int32_t tz);

/*
	功能:
		删除定时录像规则
	输入参数:
		@slave_handle: 遥控插座的句柄
		@id: 要删除的规则的id
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_video_rec_timer_del(cl_handle_t slave_handle, int id);

/*
	功能:
		获取该摄像头的所有定时器
	输入参数:
		@slave_handle: 遥控插座的句柄
		@tz: 时区，单位为小时，比如东八区为8
	输出参数:
		无
	返回:
		NULL: 失败
		其他: 成功, 返回的 cl_plug_info_t * 指向的内存块，需要用cl_plug_free_info函数释放
*/
CLIB_API cl_video_info_t *cl_video_info_get(cl_handle_t slave_handle, int32_t tz);

/*
	功能:
		释放 cl_video_info_get() 函数返回的内存块
	输入参数:
		@info: cl_video_info_get() 函数返回的内存块
	输出参数:
		无
	返回:
		无
*/
CLIB_API void cl_video_info_free(cl_video_info_t *info);
    
/*
    功能:
        设置视频饱和度
    输入参数:
        @vs: 视频饱和度参数配置
     输出参数:
     无
     返回:
     无
*/
CLIB_API RS cl_video_set_saturation(cl_handle_t slave_handle,cl_video_saturation_t* vs);
    
/*
 功能:
    设置视频云台转动速度
 输入参数:
    @speed:视频云台转速，0-100
 输出参数:
    无
 返回:
    无
 */
CLIB_API RS cl_video_set_roll_speed(cl_handle_t slave_handle,u_int8_t speed);

/*
 功能:
    查询视频有多少录像(只支持按天获取录像）
 输入参数:
    @begin_time 开始时间,0表示当天
 输出参数:
    无
 返回:
    无
 */
CLIB_API RS cl_query_vtap_list(cl_handle_t slave_handle,u_int32_t begin_time);
/*
 功能：
    获取上次cl_query_vtap_list的结果
 参数IN：
    slave_handle:录像设备句柄。
 参数OUT：
    无
 返回：
    cl_vtap_list_t：视频列表
 */
CLIB_API cl_vtap_list_t* cl_get_vtap_list_data(cl_handle_t slave_handle);

/*
 功能：
    释放cl_get_vtap_list_data获取到的数据。
 参数IN：
    list:cl_get_vtap_list_data获取到的数据。
 参数OUT：
    无
 返回：
    无
 */
CLIB_API void cl_free_vtap_list_data(cl_vtap_list_t* list);
    
/*
 功能：
    开始看录像
 参数IN：
    begin_time: 开始时间
    slave: 要观看视频的从设备的指针
    callback：回调函数
 参数OUT：
    无
 返回：
    VIDEO_XXX
 */
CLIB_API RS cl_vtap_start(cl_handle_t slave_handle,u_int32_t begin_time,
                          cl_callback_t callback, void *handle);
/*
 功能：
    停止看录像
 参数IN：
    handle:录像设备句柄。
 参数OUT：
    无
 返回：
    VIDEO_XXX
 */
CLIB_API RS cl_vtap_stop(cl_handle_t handle);
    
/*
 功能：
 获取看视频时的最后一帧图片。H264返回的是BMP格式的，MJPG的返回的是JPG格式的。pic_time是该图像的时间
 */
CLIB_API RS cl_vtap_get_picture(cl_handle_t handle, u_int32_t* pic_time,void **pic, u_int32_t *size);


#ifdef __cplusplus
}
#endif 

#endif


