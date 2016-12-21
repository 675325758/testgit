#include "cl_priv.h"
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_ch_blanket.h"
#include "lc_furnace_priv.h"

#define MIN_TEMP 18
#define MAX_TEMP 48

#define MAX_TIMER 10


#define CHECK_HANDLE_AND_AREA(handle,area) \
	do { \
		if (handle == INVALID_HANDLE|| area > AREA_BLANKET_RIGHT) { \
			log_err(false, "The handle[%08x] or area[%u] error\n",handle,area); \
			return RS_INVALID_PARAM; \
		} \
	} while (0)


CLIB_API RS cl_blanket_query_info(cl_handle_t dev_handle, u_int8_t area)
{
	CL_CHECK_INIT;

	CHECK_HANDLE_AND_AREA(dev_handle,area);
	
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_IA_CTRL, CH_BLANKET_QUERY_ALL, area);
}


RS cl_blanket_set_on_off(cl_handle_t dev_handle, u_int8_t area, u_int8_t on_off)
{
	u_int8_t buf[2] = {0};
	
	CL_CHECK_INIT;
	CHECK_HANDLE_AND_AREA(dev_handle,area);

	buf[0] = area;
	buf[1] = !!on_off;

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_ON_OFF,buf ,sizeof(buf));
	
}

CLIB_API RS cl_blanket_set_work_mode(cl_handle_t dev_handle, u_int8_t area, u_int8_t work_mode)
{
    u_int8_t buf[2] = {0};
    
    CL_CHECK_INIT;
    CHECK_HANDLE_AND_AREA(dev_handle,area);
    
    buf[0] = area;
    buf[1] = work_mode;
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_SET_MODE,buf ,sizeof(buf));
}

CLIB_API RS cl_blanket_set_temp(cl_handle_t dev_handle, u_int8_t area, u_int8_t temp)
{
	u_int8_t buf[2] = {0};
	
	CL_CHECK_INIT;
	CHECK_HANDLE_AND_AREA(dev_handle,area);

	if(temp < MIN_TEMP || temp > MAX_TEMP){
		return RS_INVALID_PARAM;
	}

	buf[0] = area;
	buf[1] = temp;

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_AREA_TEMP,buf ,sizeof(buf));
}


CLIB_API RS cl_blanket_set_curve_enable(cl_handle_t dev_handle, u_int8_t area, u_int8_t is_enable)
{
	u_int8_t buf[2] = {0};
	
	CL_CHECK_INIT;
	CHECK_HANDLE_AND_AREA(dev_handle,area);

	buf[0] = area;
	buf[1] = !!is_enable;

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_CURVE_ON_OFF,buf ,sizeof(buf));
}


CLIB_API RS cl_blanket_set_timer(cl_handle_t dev_handle, u_int8_t area, u_int8_t time)
{
	u_int8_t buf[2] = {0};
	
	CL_CHECK_INIT;
	CHECK_HANDLE_AND_AREA(dev_handle,area);

	if(time > MAX_TIMER ){
		return RS_INVALID_PARAM;
	}

	buf[0] = area;
	buf[1] = time;

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_CONFIG_TIMER,buf ,sizeof(buf));
}

CLIB_API RS cl_blanket_set_hour_curve(cl_handle_t dev_handle, u_int8_t area, u_int8_t curve[BLANKET_CURVE_PER_HOUR_POINT])
{
	u_int8_t buf[BLANKET_CURVE_PER_HOUR_POINT+1];
	
	CL_CHECK_INIT;
	CHECK_HANDLE_AND_AREA(dev_handle,area);

	buf[0] = area;
	memcpy(&buf[1],curve,BLANKET_CURVE_PER_HOUR_POINT);

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_CONFIG_CURVE ,buf ,sizeof(buf));
}

CLIB_API RS cl_blanket_set_half_hour_curve(cl_handle_t dev_handle, u_int8_t area, u_int8_t curve[BLANKET_CURVE_HALF_HOUR_POINT])
{
	u_int8_t buf[BLANKET_CURVE_HALF_HOUR_POINT+1];
	
	CL_CHECK_INIT;
	CHECK_HANDLE_AND_AREA(dev_handle,area);

	buf[0] = area;
	memcpy(&buf[1],curve,BLANKET_CURVE_HALF_HOUR_POINT);

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_IA_CTRL, CH_BLANKET_CONFIG_CURVE ,buf ,sizeof(buf));
}

