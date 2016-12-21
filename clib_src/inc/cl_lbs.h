#ifndef	__CL_LBS_H__
#define	__CL_LBS_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

typedef struct {
	u_int8_t is_east;
	u_int8_t is_north;
	u_int8_t is_gps;
	u_int8_t pad;
	u_int16_t speed;
	// 纬度
	double latitude;
	// 经度
	double longitude;
} lbs_pos_t;

#define MAX_BASE_CNT 8
typedef struct{
	int count; //基站个数，每次可能获取到多个基站
	int base[MAX_BASE_CNT];
}lbs_base_t;

typedef struct{
	u_int64_t sn;
	double speed;
	double distance;
	u_int8_t is_in_lan;
	u_int8_t has_mark_home;
	u_int8_t status;
}lbs_gdb_info_t;

typedef struct {
	int count;
	lbs_gdb_info_t list[10];
} lbs_gdb_list_t;

// 标记当前位置
CLIB_API RS lbs_mark(lbs_pos_t *pos);
// 标记基站位置
CLIB_API RS lbs_mark_base(lbs_base_t *base);
// 设置位置精度，单位米
CLIB_API void lbs_set_precision(int precision);
// 测试用，表示当前位置是家的位置。只能设置一次，第二次要删除home.txt文件、杀掉程序
CLIB_API RS lbs_mark_home_test();
/* 纬度距离 */
CLIB_API double lbs_get_latitude_diff();
/* 经度距离 */
CLIB_API double lbs_get_longitude_diff();

/*查看调试信息*/
CLIB_API void lbs_get_info(lbs_gdb_list_t *info);

/*以下接口给ios使用
局域网扫描到空调宝时，标记或修正中心位置
3公里以外 -- > 进入3公里以内 --> 进入1公里以内时调用lbs_going_home_on
智能回家开启空调*/
CLIB_API RS lbs_going_home_on(cl_handle_t handle);

#ifdef __cplusplus
}
#endif 


#endif



