#include "cl_priv.h"
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_eb.h"
#include "eb_priv.h"


/*
	功能:
		控制开关
	输入参数:
		@dev_handle: 插座的句柄
		@is_on: 0-关机，1－开机
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_ctrl_work(cl_handle_t dev_handle, bool is_on)
{
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_IA_CTRL, IA_EB_WORK, !(!is_on));
}


/*
	功能:
		添加或修改定时开关规则
	输入参数:
		@dev_handle: 设备句柄
		@timer: 定时器条目的参数。id为0表示添加，其他的为修改
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_timer_set(cl_handle_t dev_handle, cl_air_timer_t *timer)
{
	RS ret;
	cl_notify_pkt_t *pkt;
	cln_common_info_t *info;

	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_IA_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(*info);
	info = (cln_common_info_t *)pkt->data;
	info->handle = dev_handle;
	info->action = IA_EB_TIMER_SET;
	info->u.timer_info.id = timer->id;
	info->u.timer_info.enable = timer->enable;
	info->u.timer_info.week = timer->week;
	info->u.timer_info.hour = timer->hour;
	info->u.timer_info.minute = timer->minute;
 	info->u.timer_info.onoff = timer->onoff;
 	info->u.timer_info.repeat = timer->repeat;
   
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
  
	return ret;
}

/*
	功能:
		删除定时开关规则
	输入参数:
		@dev_handle: 插座的句柄
		@id: 要删除的规则的id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_eb_timer_del(cl_handle_t dev_handle, u_int8_t id)
{
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_IA_CTRL, IA_EB_TIMER_DEL, (u_int8_t)id);
}

CLIB_API RS cl_eb_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t*timer)
{
	RS ret;
	cl_notify_pkt_t *pkt;
	cln_common_info_t *info;

	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_IA_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(*info);
	info = (cln_common_info_t *)pkt->data;
	info->handle = dev_handle;
	info->action = IA_EB_PERIOD_TIMER_SET;
	info->u.period_timer_info.id = timer->id;
	info->u.period_timer_info.enable = timer->enable;
	info->u.period_timer_info.week = timer->week;
	info->u.period_timer_info.hour = timer->hour;
	info->u.period_timer_info.minute = timer->minute;
 	info->u.period_timer_info.onoff = timer->onoff;
 	info->u.period_timer_info.duration= timer->duration;
   
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
  
	return ret;
}

CLIB_API RS cl_eb_period_timer_del(cl_handle_t dev_handle, u_int8_t id)
{
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_IA_CTRL, IA_EB_PERIOD_TIMER_DEL, (u_int8_t)id);
}

CLIB_API RS cl_eb_ctrl_led_power(cl_handle_t dev_handle, u_int8_t onoff)
{
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_IA_CTRL, IA_EB_LED_CTRL, (u_int8_t)onoff);
}

CLIB_API RS cl_eb_pt_adj_set(cl_handle_t dev_handle, cl_plug_pt_adkb_t *adj, u_int8_t action)
{
	CL_CHECK_INIT;
    
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
                                   CLNE_IA_CTRL, (action==UCA_SET)?IA_EB_PT_ADJ_SET:IA_EB_PT_ADJ_DEL, (u_int8_t*)adj, sizeof(*adj));
    
}


