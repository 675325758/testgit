#ifndef	__CL_NOTIFY_DEF_H__
#define	__CL_NOTIFY_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum{
    TNOTIFY_EMERGENCY = 0, /* 紧急通知，消息格式固定为HTTP */
    TNOTIFY_NORMAL = 1,       /* 普通通知，消息格式固定为HTTP */
    TNOTIFY_AD = 2,                /* 广告，消息格式固定为HTTP*/
    TNOTIFY_FANS_SYSTEM = 5, /* FansSRV system notify msg */
    TNOTIFY_FANS_BUSSINESS = 6, /* FansSRV bussiness notify msg */
    TNOTIFY_EMERGENCY_V2 = 7,		/* 紧急通知，可以指定消息格式 */
    TNOTIFY_NORMAL_V2 = 8,		 /* 普通通知，可以指定消息格式 */
    TNOTIFY_AD_V2 = 9,			/* 广告，可以指定消息格式 */
};

enum{
	FMT_HTTP = 0, /* 消息格式为http */
	FMT_URL = 1, /* 消息格式为url */
	FMT_STRING = 2,  /* 消息格式为字符串 */
};
    
enum  { /*报警设备类型定义*/
    SAFETY_SMOKE_DETECTOR = 0x1,	/* 烟雾感应探测器 */
    SAFETY_GAS_LEAK_DETECTOR = 0x2, /* 气体感应探测器 */
    SAFETY_INFRARED_DETECTOR = 0x3,	/* 广角红外探测器 */
    SAFETY_DOOR_SENSOR = 0x4,		/* 门磁探测器 */
    SAFETY_WINDOW_SENSOR = 0x5,		/* 幕帘红外探测器 */
    SAFETY_INTELLIGENT_LOCK = 0x6,	/* 智能门锁 */
    SAFETY_EMERGENCY_BUTTON = 0x7,	/* 紧急按钮 */
    SAFETY_003H_MOTION = 100,		/* 摄像头移动侦测 */
};

/* 消息缺省有效期，天 */
#define EXPIRE_DEFAULT 14
/* 消息最长有效期，天 */
#define EXPIRE_MAX	90
/*发送通知消息正文的最大长度*/
#define MAX_NOTIFY_MSG 7800
/*发送通知消息摘要的最大长度*/
#define MAX_NOTIFY_CONTENT 128

#pragma warning(disable: 4200)
    
#pragma pack(push,1)
    
typedef struct{		/* 报警内容数据结构 */
    u_int32_t alarm_time;           /* 报警触发时间戳，UTC*/
    u_int16_t alarm_duration;     /* 报警持续时间，单位秒*/
    u_int16_t  alarm_type;		/* 报警器类型， 如 SAFETY_SMOKE_DETECTOR*/
    u_int16_t  alarm_id;			/* 报警器编号，一个家庭可能有多个烟雾探测，用编号区分 */
    u_int8_t len_name;              /* 报警设备名称长度*/
    u_int8_t arlam_name[64];               /* 报警设备名称，UTF8字符集，如：厨房烟雾探测器*/
    u_int8_t len_msg;		    /* 报警消息内容长度*/
    u_int8_t alarm_msg[256];                 /* 报警消息内容，UTF8字符集，如：厨房冒烟啦*/
}alarm_msg_t;

typedef struct{	/* 报警头数据结构 */
    u_int64_t dev_sn;		/* 设备序列号 */
    u_int64_t first_report_id;  /* 报警消息起始id */
    u_int16_t count;			/* 报警消息条数 */
    alarm_msg_t *msg;		/* 报警消息数组首地址*/
}alarm_msg_list_t;

typedef struct{ 			/* 接收通知消息内容*/
    u_int64_t dev_sn;            /* 设备序列号 */
    u_int32_t msg_time;	/* 消息时间戳，UTC*/
    u_int64_t report_id;	/* 消息id */
    u_int16_t msg_type;    /* 消息类型，TNOTIFY_NORMAL, TNOTIFY_XXX*/
    u_int8_t msg_format; /* 消息格式，FMT_HTTP，FMT_URL */
    u_int8_t expire;	/* 有效期，单位天 */
    u_int8_t content_len;  /* 消息摘要长度 */
    u_int8_t reserved;  /* 保留*/
    u_int32_t msg_len;	/* 消息长度*/
    char *msg;			/* 消息内容, msg_type为TNOTIFY_URL时msg为URL*/
    char *content;		/*消息摘要，UTF8字符串，可能为NULL*/
}notify_msg_t;

typedef struct{
	int count;
	notify_msg_t **list;
}notify_msg_list_t;

typedef struct{                     /* 通知消息确认数据结构*/
    u_int64_t dev_sn;             /* 设备序列号 */
    u_int64_t first_report_id; /* 起始消息id*/
    u_int8_t report_count;     /* 消息个数*/
    u_int8_t result;
    u_int8_t reserved[2];
}notify_msg_result_t;

typedef struct{			/* 通知公网服务器小区期望收到设备消息id数据结构 */
    u_int64_t dev_sn;		/* 设备序列号*/
    u_int64_t expect_report_id;	/* 期望收到指定设备的消息id */
}notify_expect_t;

//一台设备最多加入4个小区
#define MAX_CMT 4

typedef struct{
	u_int64_t cmt_sn;	/*小区序列号*/
	u_int64_t max_report_id; /*小区最大消息id*/
}cmt_notify_info_t;

typedef struct{
	u_int64_t cmt_sn; /*小区序列号*/
	u_int64_t report_id_begin; /*查询消息起始id，0表示不限制*/
	u_int64_t report_id_end; /*查询消息结束id，0表示不限制*/
	u_int8_t is_descending; /*0表示升序查询，1表示降序查询*/
	u_int8_t query_cnt; /*查询条数*/
	u_int8_t reserved[2]; /*保留*/
}cmt_notify_query_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 


#endif



