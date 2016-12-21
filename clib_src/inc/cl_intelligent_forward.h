#ifndef	__CL_IF_H__
#define	__CL_IF_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"


//event
enum {
	IF_BEGIN = 1500,
	/*获取信息成功 */
	IF_QUERY_OK = 1501,
	/*获取信息失败*/
	IF_QUERY_FAIL = 1502,
	IF_END = IF_BEGIN + 99
};

typedef struct cl_if_info_s{

	/*  实际温度 */
	int16_t temp;

	/*  实际湿度 */
	u_int8_t rh;

	/* PM2.5数值 */
	u_int16_t pm25;

	/* VOC 数值 */
	u_int16_t voc;
} cl_if_info_t;



/*
	功能:
		查询智能转发信息
	输入参数:
		@dev_handle: 智能转发器句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_intelligent_forward_query(cl_handle_t dev_handle);




#ifdef __cplusplus
}
#endif 

#endif


