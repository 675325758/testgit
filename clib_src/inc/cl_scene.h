#ifndef	__CL_SCENE_H__
#define	__CL_SCENE_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "client_lib.h"

#define MAX_SCENE_NAME_LENGTH (64)
#define MAX_EVENT_NAME_LENGTH (64)
    
//事件类型
typedef enum _enum_scene_event_type{
    EM_ET_UNKOWN,/*未知*/
    EM_ET_EQUIPMENT_KEY, //电器按键
    EM_ET_DEVICE_FUNC   //设备功能
}cl_scene_event_type_t;

//事件动作
typedef enum {
    CEA_UNKONWN = 0x0,
    CEA_ON = 0x1,  //开启，
    CEA_OFF = 0x2  //关闭
}cl_event_action;
    
//虚拟事件类型
typedef enum {
    CVE_UNKOWN = 0x0,
    CVE_008_CTRL = 0x1, //008开关
    CVE_003_RECORD = 0x2, //003录像
    CVE_003_MOTION = 0x3, //003移动侦测
    CVE_ALARM_SWITCH = 0x4 //报警设备开关
}cl_vitual_event_type_t;

//事件数据结构
typedef struct _cl_scene_event_s_{
    cl_scene_event_type_t enent_type; //事件类型
    cl_vitual_event_type_t ev_type; //虚拟事件类型,用户虚拟功能
    cl_handle_t obj_handle;  //事件所属设备或电器
    char event_name[MAX_EVENT_NAME_LENGTH]; //时间名称
    u_int32_t action_num; //多少个动作
    u_int32_t ac_values[1]; //动作数组,最少有一个动作
}cl_scene_event_t;

// 情景定时器
typedef struct {
	u_int8_t id;			/* 策略ID */
	u_int8_t hour;			/* 小时 0-23 */
	u_int8_t minute;		/* 分钟 0-59 */
	u_int8_t week;			/* bit 0-6位对应星期天、星期1到星期六 */
	u_int8_t enable;		/* 是否生效(手机设置) 或者已经无效(设备返回) */
	u_int8_t pad;			/* 保留 */
	u_int16_t sort;			/* 当前开始，按触发时间排序。0表示禁止的，不参与排序。其他的从1开始 */
	char *name;		/* 名字, UTF-8格式 */
} cl_scene_timer_t;

typedef struct _cl_scene_s_{
    cl_handle_t scene_handle;
    u_int8_t img_resv;//用户私有标志，可用于不同手机显示相同图片
    u_int8_t event_count;//该情景模式配置了多少个事件
    u_int8_t last_executed; // 最后被执行的情景
	u_int8_t timer_num; // 情景定时器个数
    u_int8_t scene_id;
    u_int32_t create_time;//创建时间,可以用于某台手机定制图片
    u_int8_t scene_name[MAX_SCENE_NAME_LENGTH];
	u_int32_t next_time; /* 情景定时器下一个执行的时间 */
	cl_scene_timer_t **timer;
    cl_scene_event_t* events[0];
} cl_scene_t;

// 情景模式 event
enum {
    SE_BEGIN = 800,
    SE_SCENE_INFO_HAS_CHANGED = SE_BEGIN+1,

	// 情景
    SE_SCENE_ADD_OK = SE_BEGIN+2,
    SE_SCENE_ADD_FAIL = SE_BEGIN+3,
    SE_SCENE_DEL_OK = SE_BEGIN+4,
    SE_SCENE_DEL_FAIL = SE_BEGIN+5,
    SE_SCENE_CHANGE_OK = SE_BEGIN+6,
    SE_SCENE_CHANGE_FAIL = SE_BEGIN+7,
    SE_SCENE_EXEC_OK = SE_BEGIN+8,
    SE_SCENE_EXEC_FAIL = SE_BEGIN+9,

	// 情景定时器
    SE_SCENE_TIMER_ADD_OK = SE_BEGIN + 10,
    SE_SCENE_TIMER_ADD_FAIL = SE_BEGIN + 11,
    SE_SCENE_TIMER_DEL_OK = SE_BEGIN + 12,
    SE_SCENE_TIMER_DEL_FAIL = SE_BEGIN + 13,
    SE_SCENE_TIMER_MODIFY_OK = SE_BEGIN + 14,
    SE_SCENE_TIMER_MODIFY_FAIL = SE_BEGIN + 15,

    SE_END = SE_BEGIN + 99
};
    
CLIB_API RS cl_scene_add(cl_handle_t user_handle,cl_handle_t* s_handle,u_int8_t img_resv,const char* scene_name);
    
CLIB_API RS cl_scene_del(cl_handle_t s_handle);

CLIB_API RS cl_scene_change_name(cl_handle_t s_handle,const char* name);
    
CLIB_API RS cl_scene_change_img_resv(cl_handle_t s_handle,u_int8_t img_resv);

CLIB_API RS cl_scene_modify_events(cl_handle_t s_handle,u_int16_t event_count,cl_scene_event_t** event);

CLIB_API RS cl_scene_modify(cl_handle_t s_handle, const char* name, u_int8_t img_resv, u_int16_t event_count,cl_scene_event_t** event);

CLIB_API RS cl_scene_add_3(cl_handle_t user_handle, cl_handle_t* s_handle,
				const char* name, u_int8_t img_resv, u_int16_t event_count,cl_scene_event_t** event);
    
CLIB_API RS cl_scene_exec(cl_handle_t s_handle);

CLIB_API RS cl_scene_timer_add(cl_handle_t scene_handle, cl_scene_timer_t *timer);
CLIB_API RS cl_scene_timer_modify(cl_handle_t scene_handle, cl_scene_timer_t *timer);
CLIB_API RS cl_scene_timer_del(cl_handle_t scene_handle, u_int8_t tid);

#ifdef __cplusplus
}
#endif


#endif



