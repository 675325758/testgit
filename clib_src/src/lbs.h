#ifndef	__LBS_H__
#define	__LBS_H__

#include "cl_lbs.h"

#pragma pack(push,1)

#define LAC_GET 1
#define LAC_PUT 2
#define LAC_SMART_ON 3
#define LAC_SMART_OFF 4
#define LAC_BACKHOME	5

typedef struct{
	u_int8_t is_east;
	u_int8_t is_north;
	u_int8_t action;
	u_int8_t pad;
	u_int16_t distance;
	u_int16_t speed;
	u_int8_t latitude[16];
	u_int8_t longitude[16];
}sa_net_location_t;

#pragma pack(pop)

enum {
	LBS_INIT = 0,
	LBS_IDLE = 1,
	LBS_GOING_HOME = 2,
	LBS_ARRIVE_HOME = 3,
	LBS_LEAVE_HOME = 4,
	LBS_MAX = 5
};

enum{
	BASE_INIT = 0,
	BASE_IN_HOME = 1,
	BASE_NEAR_HOME = 2,
	BASE_FAR_HOME = 3
};

typedef void (* lbs_func_t)();

typedef struct {
	char *name;
	// 进入本状态调用的函数
	lbs_func_t on_into;
	// 离开本状态调用的函数
	lbs_func_t on_out;
	// 处理定位
	lbs_func_t proc_pos;
} lbs_proc_t;

extern void lbs_user_free(user_t *user);
extern void lbs_update_lan_detect(user_t *user, int is_in_lan);
RS query_home_location(user_t *user);
bool on_get_home_location(user_t *user, void *uo);
RS do_smart_home(user_t *user, u_int8_t action);

#endif

