#include <sys/types.h>
#include <sys/stat.h>
#include "cl_priv.h"
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_common_udp_device.h"
#include "udp_device_common_priv.h"
#include "cl_tb_heater_pump.h"
#include "cl_lede_lamp.h"
#include "cl_jnb_device.h"
#include "cl_yl_thermostat.h"
#include "cl_yj_heater.h"
#include "cl_chiffo.h"
#include "cl_hxpbj.h"
#include "cl_tl_temp.h"
#include "cl_qpcp.h"
#include "qpcp_priv.h"
#include "cl_rc.h"
#include "cl_car.h"
#include "cl_xy.h"
#include "cl_tbb.h"
#include "cl_bimar_dev.h"
#include "cl_yt.h"
#include "cl_ads.h"
#include "cl_js_wave.h"
#include "cl_indiacar.h"
#include "ica_priv.h"
#include "cl_zhdhx.h"

CLIB_API RS cl_com_udp_timer_set(cl_handle_t dev_handle, cl_air_timer_t *timer)
{
	RS ret;
	cl_notify_pkt_t *pkt;
	cln_common_info_t *info;

	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_COMMON_UDP_TIMER_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(*info);
	info = (cln_common_info_t *)pkt->data;
	info->handle = dev_handle;
	info->action = ACT_UDP_COM_TIMER_SET;
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


CLIB_API RS cl_com_udp_timer_del(cl_handle_t dev_handle, u_int8_t id)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TIMER_CTRL, ACT_UDP_COM_TIMER_DEL, (u_int8_t)id);
}

CLIB_API RS cl_com_udp_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer)
{
	RS ret;
	cl_notify_pkt_t *pkt;
	cln_common_info_t *info;

	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_COMMON_UDP_TIMER_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(*info);
	info = (cln_common_info_t *)pkt->data;
	info->handle = dev_handle;
	info->action = ACT_UDP_COM_PEROID_TIMER_SET;
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

CLIB_API RS cl_com_udp_ext_period_timer_set(cl_handle_t dev_handle, cl_period_timer_t *timer,void* other_param,u_int16_t param_len)
{
    cl_period_timer_t* dest;
    u_int8_t buf[256] = {0};
    
    
    CL_CHECK_INIT;
    
    dest = (cl_period_timer_t*)buf;
    memcpy(dest, timer, sizeof(*dest));
    
    if (param_len > 0) {
        memcpy((dest+1), other_param, param_len);
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TIMER_CTRL, ACT_UDP_COM_EXT_PEROID_TIMER_SET,buf,sizeof(*dest)+param_len);
}

CLIB_API RS cl_com_udp_period_timer_del(cl_handle_t dev_handle, u_int8_t id)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TIMER_CTRL, ACT_UDP_COM_PEROID_TIMER_DEL, (u_int8_t)id);
}

CLIB_API void cl_com_udp_refresh_timer(int dev_handle)
{
	cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TIMER_CTRL, ACT_UDP_COM_TIMER_REFRESH, 0);
}

CLIB_API void cl_com_udp_refresh_dev_all_info(int dev_handle)
{
	cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_REFRESH_ALL_STAT, 0);
}

CLIB_API RS cl_com_udp_set_peak_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute)
{
	u_int32_t data = BUILD_U32_FROM_U16(begin_time,last_minute);
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SET_PEAK_TIME, data);
}
    
CLIB_API RS cl_com_udp_set_valley_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute)
{
	u_int32_t data = BUILD_U32_FROM_U16(begin_time,last_minute);
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SET_VALLEY_TIME, data);
}

CLIB_API RS cl_com_udp_set_peak_price(cl_handle_t dev_handle, u_int32_t price)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SET_PEAK_PRICE, price);
}

CLIB_API RS cl_com_udp_set_valley_price(cl_handle_t dev_handle, u_int32_t price)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SET_VALLEY_PRICE, price);
}

CLIB_API RS cl_com_udp_set_flat_price(cl_handle_t dev_handle, u_int32_t price)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SET_FLAT_PRICE, price);
}

CLIB_API RS cl_com_udp_refresh_elec_info(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_REFRESH_ELEC_STAT, 0);
}

CLIB_API RS cl_com_udp_clear_elec_stat_info(cl_handle_t dev_handle,int type)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_CLEAR_ELEC_STAT, type & 0xF);
}

CLIB_API RS cl_com_udp_set_elec_ajust_value(cl_handle_t dev_handle,int16_t value)
{
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_AJUST_ELEC_VALUE,value);
}

CLIB_API RS cl_com_udp_clear_dev_err_info(cl_handle_t dev_handle , u_int32_t err_id)
{
	CL_CHECK_INIT;
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_CLEAR_DEV_ERR_INFO,err_id);
}

CLIB_API RS cl_com_udp_refresh_dev_err_info(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_REFRESH_DEV_ERR_INFO,0 );
}

CLIB_API RS cl_com_udp_set_permit_stm_upgrade(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SET_PEMIT_STM_UPGRADE,0 );
}

CLIB_API RS cl_com_udp_set_dev_restore_factory(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_RESTORY_FACTORY,0 );
}

CLIB_API RS cl_com_udp_reset_dev_ssid_and_passwd(cl_handle_t dev_handle,const char* ssid,const char* password)
{
	u_int8_t buf[256] = {0};
	int ssid_len;
	int pass_len;
	CL_CHECK_INIT;
	
	if(!ssid){
		return RS_INVALID_PARAM;
	}	

	ssid_len = (int)strlen(ssid);
	pass_len = 0;
	if(password != NULL){
		pass_len = (int)strlen(password);
	}
	
	if(ssid_len <= 0 || ssid_len > 128){
		return RS_INVALID_PARAM;
	} 
	
	ssid_len++;
	ssid_len = (ssid_len+3) & 0xFFFFFFFC;

	strcpy((char*)&buf[2],ssid);
	if(pass_len > 0){
		pass_len++;
		pass_len = (pass_len+3) & 0xFFFFFFFC;
		strcpy((char*)&buf[128],password);
	}
	
	buf[0] = ssid_len;
	buf[1] = pass_len;

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_SETTING_SSID_PASSWD,buf,sizeof(buf));
}

CLIB_API RS cl_com_udp_set_env_temp_ajust_value(cl_handle_t dev_handle,int16_t value)
{
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_AJUST_ROOM_TEMP,value);
}

CLIB_API RS cl_com_udp_refresh_24hour_line(cl_handle_t dev_handle,u_int8_t type)
{
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_REFRESH_24HOUR_LINE,type);
}

CLIB_API RS cl_com_udp_request_share_code(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_REQUEST_SHARE_CODE,0);
}

CLIB_API RS cl_com_udp_del_shared_phone(cl_handle_t dev_handle,u_int32_t share_index)
{
    CL_CHECK_INIT;
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_DEL_SHARED_PHONE,share_index);
}


CLIB_API RS cl_com_udp_refresh_shard_list(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_REFRESH_SHARED_LIST,0);
}

CLIB_API RS cl_com_udp_modify_shared_phone(cl_handle_t dev_handle,u_int32_t share_index, char *desc)
{
	u_int8_t buf[256] = {0};
	ucp_share_desc_info_t *psdi = (ucp_share_desc_info_t *)buf;
	
	CL_CHECK_INIT;

	memset(buf, 0, sizeof(buf));
	psdi->index = share_index;
	strncpy((char *)psdi->desc, desc, sizeof(psdi->desc));
	
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_COMMON_CTRL, ACT_UDP_COM_MODFIY_SHARED_PHONE,buf,sizeof(buf));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 拓邦热水泵
CLIB_API RS cl_tb_ctrl_stat(cl_handle_t dev_handle,cl_tb_user_config_t* uconfig)
{
	CL_CHECK_INIT;
	
	if(!uconfig){
		return RS_INVALID_PARAM;
	}	

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TB_HEATER_CTRL, ACT_TB_CTRL_STAT,(u_int8_t*)uconfig,sizeof(*uconfig));
}

CLIB_API RS cl_tb_setting_work_param(cl_handle_t dev_handle,cl_tb_work_config_t* wconfig)
{
	CL_CHECK_INIT;

	if(!wconfig){
		return RS_INVALID_PARAM;
	}
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TB_HEATER_CTRL, ACT_TB_CTRL_SETTING_WORK_PARAM,(u_int8_t*)wconfig,sizeof(*wconfig));
}

CLIB_API RS cl_tb_refresh_temp_info(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TB_HEATER_CTRL, ACT_TB_CTRL_REFRESH_TEMP_INFO, 0x0);
}

CLIB_API RS cl_tb_refresh_other_info(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TB_HEATER_CTRL, ACT_TB_CTRL_REFRESH_OTHER_INFO, 0x0);
}

CLIB_API RS cl_tb_bind_dev_bar_code(cl_handle_t dev_handle,u_int8_t* bar_code)
{ 
	u_int8_t buf[32] = {0};
	CL_CHECK_INIT;
	
	if(bar_code != NULL){
		strncpy((char*)buf,(char*)bar_code,sizeof(buf));
	}

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TB_HEATER_CTRL, ACT_TB_CTRL_BIND_BAR_CODE,buf,sizeof(buf));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 杰能宝温控器

CLIB_API RS cl_jnb_th_ctrl_onoff(cl_handle_t dev_handle,u_int8_t onoff)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_ONOFF, onoff);
}

CLIB_API RS cl_jnb_th_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_MODE, mode);
}

CLIB_API RS cl_jnb_th_ctrl_temp(cl_handle_t dev_handle,u_int8_t temp)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_TEMP, temp);
}

CLIB_API RS cl_jnb_th_set_schedluer(cl_handle_t dev_handle,u_int32_t* schedluer)
{
	u_int32_t buf[DAYS_PER_WEEK] = {0};
	CL_CHECK_INIT;
	
	if(!schedluer){
		return RS_INVALID_PARAM;
	}
	
	memcpy(buf,schedluer,sizeof(buf));

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_SCHED,(u_int8_t*)(&buf[0]),sizeof(buf));
}

CLIB_API RS cl_jnb_th_set_temp_param(cl_handle_t dev_handle,u_int8_t mode,u_int8_t temp)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(mode,temp,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_TEMP_PARAM, data);
}

CLIB_API RS cl_jnb_th_set_holiday_days(cl_handle_t dev_handle,u_int8_t temp,u_int8_t days)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(temp,days,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_HOLIDAY, data);
}

CLIB_API RS cl_jnb_th_set_hold_time(cl_handle_t dev_handle,u_int8_t mode,u_int8_t hours)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(mode,hours,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JNB_DEVICE, ACT_JNB_CTRL_HOLD_TIME, data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LEDE 调色灯

CLIB_API RS cl_lede_ctrl_stat(cl_handle_t dev_handle,cl_lede_led_state_t* uconfig)
{
	u_int8_t buf[64] = {0};
	CL_CHECK_INIT;
	
	if(!uconfig){
		return RS_INVALID_PARAM;
	}
	
	memcpy(buf,uconfig,sizeof(cl_lede_led_state_t));

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_LEDE_LAMP_CTRL, ACT_LEDE_CTRL_STAT,buf,sizeof(cl_lede_led_state_t));
}

CLIB_API RS cl_lede_ctrl_timer(cl_handle_t dev_handle,cl_lede_led_timer_t* tinfo)
{
	u_int8_t buf[64] = {0};
	CL_CHECK_INIT;
	
	if(!tinfo){
		return RS_INVALID_PARAM;
	}
	
	memcpy(buf,tinfo,sizeof(cl_lede_led_timer_t));

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_LEDE_LAMP_CTRL, ACT_LEDE_CTRL_TIEMR,buf,sizeof(cl_lede_led_timer_t));
}

CLIB_API RS cl_lede_delete_timer(cl_handle_t dev_handle,u_int8_t timer_id)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_LEDE_LAMP_CTRL, ACT_LEDE_DELETE_TIEMR, timer_id);
}

CLIB_API RS cl_lede_on_state_config(cl_handle_t dev_handle, cl_lede_led_on_stat_t *uconfig)
{
	u_int8_t buf[64] = {0};
	CL_CHECK_INIT;
	
	if (!uconfig){
		return RS_INVALID_PARAM;
	}
	
	memcpy(buf, uconfig, sizeof(cl_lede_led_state_t));

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_LEDE_LAMP_CTRL, ACT_LEDE_ON_STAT, buf, sizeof(cl_lede_led_on_stat_t));
}


CLIB_API RS cl_jl_ctrl_3200_lamp(cl_handle_t dev_handle,u_int8_t on_off,u_int8_t color,u_int8_t bright)
{
    u_int32_t value = BUILD_U32(on_off, color, bright, 0);
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZH_JL_LAMP, ACT_JL_3200_CTRL, value);
}

CLIB_API RS cl_jl_ctrl_3200_total_bright_lamp(cl_handle_t dev_handle,u_int8_t on_off,u_int8_t color,u_int8_t bright,u_int8_t total_bright)
{
    u_int32_t value = BUILD_U32(on_off, color, bright, total_bright);
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZH_JL_LAMP, ACT_JL_3200_AL_CTRL, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//亿林温控器

CLIB_API RS cl_yl_thermostat_ctrl_onoff(cl_handle_t dev_handle, bool is_on)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YL_AC_CTRL, ACT_YL_CTRL_ONOFF, is_on);
}

CLIB_API RS cl_yl_thermostat_ctrl_work_mode(cl_handle_t dev_handle, u_int8_t work_mode)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YL_AC_CTRL, ACT_YL_CTRL_MODE, work_mode);
}

CLIB_API RS cl_yl_thermostat_ctrl_gear(cl_handle_t dev_handle,u_int8_t work_mode, u_int8_t gear)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(work_mode,gear,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YL_AC_CTRL, ACT_YL_CTRL_GEAR, data);
}

CLIB_API RS cl_yl_thermostat_ctrl_temp(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t temp)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(work_mode,temp,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YL_AC_CTRL, ACT_YL_CTRL_TEMP, data);
}

CLIB_API RS cl_yl_thermostat_ctrl_scene(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t scene)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(work_mode,scene,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YL_AC_CTRL, ACT_YL_CTRL_SCENE, data);
}

CLIB_API RS cl_yl_thermostat_set_temp_param(cl_handle_t dev_handle,u_int8_t work_mode,u_int8_t scene_count,ts_scene_t* temp_param)
{
	u_int8_t buf[128] = {0};
	
	CL_CHECK_INIT;

	if(!temp_param || scene_count > 18){
		return RS_INVALID_PARAM;
	}
	buf[0] = work_mode;
	buf[1] = scene_count;
	memcpy(&buf[2],temp_param,sizeof(ts_scene_t)*scene_count);

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_YL_AC_CTRL, ACT_YL_SETTING_TEMP_PARAM,buf,sizeof(buf));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//艾美特
CLIB_API RS cl_amt_ctrl_on_off(cl_handle_t dev_handle,u_int8_t is_on)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_ONOFF, is_on);
}

CLIB_API RS cl_amt_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_MODE, mode);
}

CLIB_API RS cl_amt_ctrl_gear(cl_handle_t dev_handle,u_int8_t mode,u_int8_t gear)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(mode,gear,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_GEAR, data);
}

CLIB_API RS cl_amt_ctrl_shake(cl_handle_t dev_handle,u_int8_t is_shake)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_SHAKE, is_shake);
}

CLIB_API RS cl_amt_ctrl_anion(cl_handle_t dev_handle,u_int8_t is_on)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_ANION, is_on);
}

CLIB_API RS cl_amt_ctrl_plasma(cl_handle_t dev_handle,u_int8_t is_on)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_PLASMA, is_on);
}

CLIB_API RS cl_amt_ctrl_screen_light(cl_handle_t dev_handle,u_int8_t is_on)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_SCREEN_LIGHT, is_on);
}

CLIB_API RS cl_amt_config_fan_user_mode(cl_handle_t dev_handle,u_int8_t time_interval,u_int8_t gear_num,u_int8_t* gears)
{
	u_int8_t buf[64] = {0};
	
	CL_CHECK_INIT;

	if(!gears|| gear_num == 0){
		return RS_INVALID_PARAM;
	}
	buf[0] = time_interval;
	buf[1] = gear_num;
	memcpy(&buf[2],gears,sizeof(u_int8_t)*gear_num);

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_U_DEF_MODE,buf,sizeof(buf));
}

CLIB_API RS cl_amt_ctrl_dev_onoff_timer(cl_handle_t dev_handle,u_int8_t onoff,u_int16_t time)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32_FROM_U16(!!onoff ,time);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_S_TIMER, data);
}

CLIB_API RS cl_amt_config_smart_param(cl_handle_t dev_handle,u_int8_t temp,u_int8_t gear)
{
	u_int32_t data;
	CL_CHECK_INIT;
	data = BUILD_U32(temp,gear,0,0);
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_AMT_FAN_CTRL, ACT_AMT_CTRL_SMART_PARAM, data);
}
////////////////////////////////////////////////////////////////////
//前锋

CLIB_API RS cl_chiffo_ctrl_on_off(cl_handle_t dev_handle,u_int8_t is_on)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_CTRL_ONOFF, is_on);
}

CLIB_API RS cl_chiffo_setting_water_mode_temp(cl_handle_t dev_handle,u_int8_t temp)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_CTRL_WATER_TEMP, temp);
}

CLIB_API RS cl_chiffo_add_dec_water_mode_temp(cl_handle_t dev_handle,bool is_add)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_ADD_DEC_WATER_TEMP, is_add);
}

CLIB_API RS cl_chiffo_add_dec_heater_mode_temp(cl_handle_t dev_handle,bool is_add)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_ADD_DEC_HEATER_TEMP, is_add);
}

CLIB_API RS cl_chiffo_setting_heater_mode_temp(cl_handle_t dev_handle,u_int8_t temp)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_CTRL_HEATER_TEMP, temp);
}

CLIB_API RS cl_chiffo_setting_mode(cl_handle_t dev_handle,u_int8_t water_on_off,u_int8_t heater_on_off)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_MODE, heater_on_off);
}

CLIB_API RS cl_chiffo_setting_timer(cl_handle_t dev_handle,cl_chiffo_one_day_timer_t* day_info,u_int8_t day_index)
{
	u_int8_t buf[128] = {0};
	
	CL_CHECK_INIT;

	if( day_index >= 7){
		return RS_INVALID_PARAM;
	}
	buf[0] = day_index;
	memcpy(&buf[1],day_info,sizeof(*day_info));

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_TIMER,buf,sizeof(buf));
}

CLIB_API RS cl_chiffo_set_clock(cl_handle_t dev_handle)
{
	u_int8_t buf[8] = {0};
	u_int8_t idx = 0;
	time_t timestamp;
	struct tm *tm;
	u_int32_t year, month, day, week, hour, min, sec;
	
	CL_CHECK_INIT;

	timestamp = time(NULL);
	tm = localtime(&timestamp);

	year = tm->tm_year + 1900;
	month = tm->tm_mon + 1;
	day = tm->tm_mday;
	week = tm->tm_wday;
	hour = tm->tm_hour;
	min = tm->tm_min;
	sec = tm->tm_sec;
	

	buf[idx++] = (year/100) & 0xff;
	buf[idx++] = (year % 100) & 0xff;

	buf[idx++] = month;
	buf[idx++] = day;
	buf[idx++] = week;
	buf[idx++] = hour;
	buf[idx++] = min;
	buf[idx++] = sec;

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_CLOCK, buf, sizeof(buf));
}


CLIB_API RS cl_chiffo_refresh_timer_by_day(cl_handle_t dev_handle,u_int8_t day_index)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_REFRESH_TIMER, day_index);
}

CLIB_API RS cl_chiffo_set_scene(cl_handle_t dev_handle, u_int8_t scene)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_SCENE, scene);
}

CLIB_API RS cl_chiffo_set_loop(cl_handle_t dev_handle,u_int8_t loop_type)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_LOOP, loop_type);
}

CLIB_API RS cl_chiffo_add_dec_water_capacity(cl_handle_t dev_handle,bool is_add)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_WATER_CAP_UP_OR_DOWN, is_add);
}

CLIB_API RS cl_chiffo_setting_water_capacity_temp(cl_handle_t dev_handle,u_int16_t capacity)
{
	CL_CHECK_INIT;

	if (capacity > 0xff) {
		return RS_INVALID_PARAM;
	}

	if (capacity % 10) {
		return RS_INVALID_PARAM;
	}
	
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL, ACT_CHIFFO_SET_WATER_CAP, (u_int8_t)(capacity/10));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//海迅库接口
CLIB_API RS cl_hx_ctrl_work_mode(cl_handle_t dev_handle, u_int8_t work_mode)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_HX_CTRL, ACT_HX_MODE_CMD, work_mode);
}

CLIB_API RS cl_hx_ctrl_diy_name(cl_handle_t dev_handle, u_int8_t id, char *name)
{
	u_int8_t buf[257] = {0};
	
	CL_CHECK_INIT;

	memset(buf, 0, sizeof(buf));
	buf[0] = id;
	strcpy((char *)&buf[1], name);
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_HX_CTRL, ACT_HX_DIY_NAME, buf, sizeof(buf));	
}

CLIB_API RS cl_hx_finish_clear(cl_handle_t dev_handle)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_HX_CTRL, ACT_HX_FINISH_CLEAR, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//特林温控器

CLIB_API RS cl_tl_ctrl_off(cl_handle_t dev_handle,bool onoff)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_ONOFF, onoff);
}

CLIB_API RS cl_tl_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_MODE, mode);
}

CLIB_API RS cl_tl_ctrl_fan_speed(cl_handle_t dev_handle,u_int8_t fan_speed)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_FAN_SPEED, fan_speed);
}

CLIB_API RS cl_tl_ctrl_temp(cl_handle_t dev_handle,u_int8_t temp)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_TEMP, temp);
}

CLIB_API RS cl_tl_ctrl_eco(cl_handle_t dev_handle,bool is_enable)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_ECO, is_enable);
}

CLIB_API RS cl_tl_ctrl_learn(cl_handle_t dev_handle,bool is_enable)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_LEARN, is_enable);
}

CLIB_API RS cl_tl_setting_timer(cl_handle_t dev_handle,cl_tl_timer_info_t* timer)
{
	u_int8_t buf[128] = {0};
	
	CL_CHECK_INIT;

	if( ! timer ){
		return RS_INVALID_PARAM;
	}
	memcpy(&buf[0],timer,sizeof(*timer));

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_TIMER,buf,sizeof(buf));
}

CLIB_API RS cl_tl_ctrl_clock_auto_sync(cl_handle_t dev_handle,bool is_auto_sync)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_TIME_AUTO_SYNC, is_auto_sync);
}

CLIB_API RS cl_tl_ctrl_clock_sync(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_TIME_SYNC, 0);
}

CLIB_API RS cl_tl_refresh_dev_time(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TL_CTRL, ACT_TL_CTRL_REFRESH_TIME, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//千帕茶盘
CLIB_API RS cl_qpcp_ctrl_on(cl_handle_t dev_handle, u_int8_t onoff)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_ONOFF, onoff);
}

CLIB_API RS cl_qpcp_add_water(cl_handle_t dev_handle, u_int8_t action, u_int8_t time)
{
	u_int8_t buf[2];
	
	CL_CHECK_INIT;
	buf[0] = action;
	buf[1] = time;
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_ADD_WATER,buf,sizeof(buf));
}

CLIB_API RS cl_qpcp_handle_ctrl(cl_handle_t dev_handle, cl_qpcp_scp_t *param)
{
	u_int8_t buff[128];
	cl_qpcp_scp_t *php;
	
	CL_CHECK_INIT;
	php = (cl_qpcp_scp_t *)buff;
	memcpy(php, param, sizeof(*php));
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_HANDLE_CTRL,buff,sizeof(buff));
}

CLIB_API RS cl_qpcp_scene_del(cl_handle_t dev_handle, u_int16_t id)
{
	CL_CHECK_INIT;
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_SCENE_DEL, id);
}

CLIB_API RS cl_qpcp_scene_modify(cl_handle_t dev_handle, cl_qpcp_sp_t *param)
{
	u_int8_t buff[128];
	cl_qpcp_sp_t *psp;
	
	CL_CHECK_INIT;
	psp = (cl_qpcp_sp_t *)buff;
	memcpy(psp, param, sizeof(*psp));
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_SCENE_MODIFY,buff,sizeof(buff));
}

CLIB_API RS cl_qpcp_scene_execute(cl_handle_t dev_handle, u_int16_t id, cl_qpcp_scp_t *param)
{
	u_int8_t buff[128];	
	u_int16_t *pid = NULL;
	cl_qpcp_scp_t *pscp = NULL;
	
	CL_CHECK_INIT;
	pid = (u_int16_t *)buff;
	pscp = (cl_qpcp_scp_t *)(&pid[1]);
	*pid = id;
	memcpy(pscp, param, sizeof(*pscp));
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_SCENE_EXECUTE,buff,sizeof(buff));
}

CLIB_API RS cl_qpcp_reset_fault(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QPCP_CTRL, ACT_QPCP_CTRL_RESET_FAULT, 0);
}

//////////////////////////////////////////////////
//千帕锅


CLIB_API RS cl_qp_pot_ctrl(cl_handle_t dev_handle, cl_qp_pot_scene_param_t* param)
{
    u_int8_t buff[128] = {0};
    
    CL_CHECK_INIT;

    memcpy(buff, param, sizeof(*param));
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QP_POT_CTRL, ACT_QP_POT_CTRL,buff,sizeof(buff));
}


CLIB_API RS cl_qp_pot_exec_scene(cl_handle_t dev_handle, cl_qp_pot_scene_param_t* param)
{
    u_int8_t buff[128] = {0};
    
    CL_CHECK_INIT;
    
    memcpy(buff, param, sizeof(*param));
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QP_POT_CTRL, ACT_QP_POT_EXEC_SCENE,buff,sizeof(buff));
}

CLIB_API RS cl_qp_pot_del_scene(cl_handle_t dev_handle, u_int16_t s_id)
{
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QP_POT_CTRL, ACT_QP_POT_DEL_SCENE, s_id);
}

CLIB_API RS cl_qp_pot_modify_scene(cl_handle_t dev_handle, cl_qp_pot_scene_t *scene)
{
    u_int8_t buff[256] = {0};
    
    CL_CHECK_INIT;
    
    memcpy(buff, scene , sizeof(*scene));
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QP_POT_CTRL, ACT_QP_POT_MODIFY_SCENE,buff,sizeof(buff));
}

//////////////////////////////////////////
//千帕破壁机
CLIB_API RS cl_qp_pbj_ctrl_onoff(cl_handle_t dev_handle, u_int8_t on_off)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QP_PBJ, ACT_QP_PBJ_CTRL_ONOFF, on_off);
}

CLIB_API RS cl_qp_pbj_ctrl_scene(cl_handle_t dev_handle, u_int8_t aciton, u_int16_t scene_id)
{
    u_int32_t value = BUILD_U32_FROM_U16(aciton, scene_id);
    
    CL_CHECK_INIT;
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QP_PBJ, ACT_QP_PBJ_CTRL_EXEC_SCENE , value);
}

CLIB_API RS cl_qp_pbj_modify_scene(cl_handle_t dev_handle, cl_qp_pbj_scene_t* scene)
{
    CL_CHECK_INIT;
    
    if (!scene) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_QP_PBJ, ACT_QP_PBJ_CTRL_MODIFY_SCENE,(u_int8_t*)scene,sizeof(*scene));
}

CLIB_API RS cl_qp_pbj_reset_fault(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_QP_PBJ, ACT_QP_PBJ_CTRL_FAULT_STAT, 0);
}

//////////////////////////////////////////////////////////
//电视、机顶盒控制


CLIB_API RS cl_rc_change_name(cl_handle_t dev_handle, u_int8_t* name)
{
    u_int8_t buff[128] = {0};
    int len;
    
    CL_CHECK_INIT;
    
    if (!name) {
        return RS_INVALID_PARAM;
    }
    
    len = (int)strlen((char*)name);
    
    if (len <= 0 || len >= MAX_RC_NAME_LEN) {
        return RS_INVALID_PARAM;
    }
    
    strcpy((char*)buff, (char*)name);
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_STB_TV_CTRL,
                                   ACT_STB_CHANGE_NAME,buff,sizeof(buff));
}


CLIB_API RS cl_rc_start_match(cl_handle_t dev_handle,u_int8_t rc_id,u_int16_t timeout)
{
    u_int32_t value;
    
    CL_CHECK_INIT;
    
    value = BUILD_U32_FROM_U16(rc_id, timeout);
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_START_MATCH, value);
}

CLIB_API RS cl_rc_start_next_key_match(cl_handle_t dev_handle,u_int8_t rc_id,u_int16_t timeout,u_int8_t key_id)
{
    u_int32_t value;
    
    CL_CHECK_INIT;
    
    value = (rc_id << 24) | (key_id << 16) | timeout;
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_START_NKEY_MATCH, value);
}


CLIB_API RS cl_rc_stop_match(cl_handle_t dev_handle,u_int8_t rc_id)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_STOP_MATCH, rc_id);
}


CLIB_API RS cl_rc_get_match_stat(cl_handle_t dev_handle,cl_rc_match_stat_t* stat)
{
    RS ret = ERR_NONE;
    user_t* user;
    smart_appliance_ctrl_t* sac;
    smart_air_ctrl_t* ac;
    
    CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
    memset(stat, 0, sizeof(*stat));
    
    cl_lock(&cl_priv->mutex);
    
    if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
        log_err(false, "cl_rc_get_match_stat request 0x%08x failed: not found\n", dev_handle);
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    if (!(sac = user->smart_appliance_ctrl)) {
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    if ( !(ac = sac->sub_ctrl)) {
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    memcpy(stat, &ac->rc_pm.match_stat, sizeof(*stat));
    
    
done:
    cl_unlock(&cl_priv->mutex);
    
    return RS_OK;
}


CLIB_API RS cl_rc_get_learn_stat(cl_handle_t dev_handle,cl_rc_key_learn_stat_t* stat)
{
    RS ret = ERR_NONE;
    user_t* user;
    smart_appliance_ctrl_t* sac;
    smart_air_ctrl_t* ac;
    
    CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
    memset(stat, 0, sizeof(*stat));
    
    cl_lock(&cl_priv->mutex);
    
    if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
        log_err(false, "cl_rc_get_learn_stat request 0x%08x failed: not found\n", dev_handle);
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    if (!(sac = user->smart_appliance_ctrl)) {
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    if ( !(ac = sac->sub_ctrl)) {
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    memcpy(stat, &ac->rc_pm.learn_info, sizeof(*stat));
    
done:
    cl_unlock(&cl_priv->mutex);
    
    return RS_OK;
}


CLIB_API RS cl_rc_ctrl_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id)
{
    u_int16_t value;
    CL_CHECK_INIT;
    
    value = BUILD_U16(rc_id, key_id);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_CTRL_KEY, value);
}


CLIB_API RS cl_rc_quick_ctrl_onoff(cl_handle_t dev_handle,u_int8_t tv_rc_id,u_int8_t stb_rc_id)
{
    u_int16_t value;
    CL_CHECK_INIT;
    
    value = BUILD_U16(tv_rc_id , stb_rc_id);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_QUICK_ONOFF, value);
}

CLIB_API RS cl_rc_start_learn(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id)
{
    u_int16_t value;
    CL_CHECK_INIT;
    
    value = BUILD_U16(rc_id, key_id);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_START_LEARN, value);
}

CLIB_API RS cl_rc_stop_learn(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id)
{
    u_int16_t value;
    CL_CHECK_INIT;
    
    value = BUILD_U16(rc_id, key_id);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_STOP_LEARN, value);
}


CLIB_API RS cl_rc_delete_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id)
{
    u_int16_t value;
    CL_CHECK_INIT;
    
    value = BUILD_U16(rc_id, key_id);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_STB_TV_CTRL, ACT_STB_DELETE_KEY, value);
}

CLIB_API RS cl_rc_modify_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id,char* name)
{
    u_int8_t buff[128] = {0};
    int len;
    
    CL_CHECK_INIT;
    
    if (!name) {
        return RS_INVALID_PARAM;
    }
    
    len = (int)strlen((char*)name);
    
    if (len <= 0 || len >= MAX_RC_NAME_LEN) {
        return RS_INVALID_PARAM;
    }
    
    buff[0] = rc_id;
    buff[1] = key_id;
    strcpy((char*)&buff[2], (char*)name);
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_STB_TV_CTRL,
                                   ACT_STB_MODIFY_KEY,buff,sizeof(buff));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//车载悟空
CLIB_API RS cl_car_config_keeptime(cl_handle_t dev_handle, u_int8_t keep_time)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CAR, ACT_CAR_CONFIG_ON, keep_time);
}

CLIB_API RS cl_car_ctrl_on(cl_handle_t dev_handle, u_int8_t on)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CAR, ACT_CAR_CTRL_ON, on);
}

CLIB_API RS cl_car_config_search(cl_handle_t dev_handle, cl_car_search_t *search)
{
    CL_CHECK_INIT;
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_CAR,
                                   ACT_CAR_CONFIG_SEARCH,(u_int8_t *)search,sizeof(*search));
}

CLIB_API RS cl_car_ctrl_search(cl_handle_t dev_handle)
{
	int on = 1;
	
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CAR, ACT_CAR_CTRL_SEARCH, on);
}

CLIB_API RS cl_car_config_valtage(cl_handle_t dev_handle, u_int16_t valtage)
{
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CAR, ACT_CAR_CONFIG_VALTAGE, valtage);	
}

CLIB_API RS cl_car_ctrl_valtage(cl_handle_t dev_handle, u_int8_t on)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CAR, ACT_CAR_CTRL_VALTAGE, on);	
}

CLIB_API RS cl_car_ctrl_powersave(cl_handle_t dev_handle, u_int8_t on)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_CAR, ACT_CAR_CTRL_POWERSAVE, on);	
}

///////////////////////////////////////////////////////////////////////////////////////

CLIB_API RS cl_eo_set_onoff(cl_handle_t dev_handle, bool is_on)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EPLUG_OEM, ACT_EO_SET_ONOFF, is_on);
}

CLIB_API RS cl_eo_set_temp_range(cl_handle_t dev_handle, bool enable,u_int8_t max_temp,u_int8_t min_temp)
{
    u_int32_t value = BUILD_U32(enable, max_temp, min_temp, 0);
    CL_CHECK_INIT;
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EPLUG_OEM, ACT_EO_SET_TEMP_RANGE, value);
}

CLIB_API RS cl_eo_set_threshold(cl_handle_t dev_handle, bool enable,u_int8_t max_temp)
{
    u_int32_t value = BUILD_U32(enable, max_temp , 0, 0);
    CL_CHECK_INIT;
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EPLUG_OEM, ACT_EO_SET_THRESHOLD, value);
}

CLIB_API RS cl_eo_set_off_line_close_enable(cl_handle_t dev_handle, bool enable)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EPLUG_OEM, ACT_EO_SET_OFFLINE_ENABLE, enable);
}

CLIB_API RS cl_eo_set_person_detect_enable(cl_handle_t dev_handle, bool enable)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_EPLUG_OEM, ACE_EO_SET_PERSON_ENABLE, enable);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//鑫源温控器
CLIB_API RS cl_xy_ctrl_onoff(cl_handle_t dev_handle, u_int8_t onoff)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_ONOFF, onoff);
}

CLIB_API RS cl_xy_ctrl_temp(cl_handle_t dev_handle, u_int8_t temp)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY,
		ACT_XY_CTRL_TEMP, temp);
}

CLIB_API RS cl_xy_ctrl_lock_onoff(cl_handle_t dev_handle, u_int8_t onoff)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_LOCK_ONOFF, onoff);
}

CLIB_API RS cl_xy_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_MODE, mode);
}

CLIB_API RS cl_xy_ctrl_time(cl_handle_t dev_handle, u_int16_t time)
{
    CL_CHECK_INIT;
	
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_TIME, time);	
}

CLIB_API RS cl_xy_config_smart_mode(cl_handle_t dev_handle, cl_xy_smartmode_com_info_t *pst_info)
{
	CL_CHECK_INIT;
	
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_THERMOSTAT_XY,
		ACT_XY_CONFIG_SMART_MODE,(u_int8_t *)pst_info,sizeof(*pst_info));
}

CLIB_API RS cl_xy_ctrl_adjust(cl_handle_t dev_handle, cl_xy_adjust_t *padjust)
{
	CL_CHECK_INIT;
	
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_THERMOSTAT_XY,
		ACT_XY_CTRL_ADJUST,(u_int8_t *)padjust,sizeof(*padjust));
}

CLIB_API RS cl_xy_ctrl_extern_temp(cl_handle_t dev_handle, u_int8_t temp)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_EXTERN_TEMP, temp);
}
CLIB_API RS cl_xy_ctrl_smarthome_onoff(cl_handle_t dev_handle, u_int8_t onoff)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_SMARTHOME_ONOFF, onoff);
}
CLIB_API RS cl_xy_ctrl_smarthome_mode(cl_handle_t dev_handle, u_int8_t mode)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_THERMOSTAT_XY, 
		ACT_XY_CTRL_SMART_MODE, mode);	
}

//////////////////////////////////////////////////////////////////////////////////
//bimar暖风机
CLIB_API RS cl_bimar_ctrl(cl_handle_t dev_handle,BIM_CTRL_ACTION_E type,u_int8_t value)
{
    u_int16_t data;
    
    CL_CHECK_INIT;
    data = BUILD_U16((u_int8_t)type, value);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_BIMAR,
                             ACT_BIMAR_COMMON_CTRL, data);
    
}

//商用华天成
CLIB_API RS cl_tbb_ctrl_onoff(cl_handle_t dev_handle, u_int8_t onoff)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TBB, 
		ACT_TBB_ON, onoff);
}

CLIB_API RS cl_tbb_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode)
{
    CL_CHECK_INIT;
	
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TBB, 
		ACT_TBB_MODE, mode);
}

CLIB_API RS cl_tbb_ctrl_tmp(cl_handle_t dev_handle, u_int16_t tmp)
{
    CL_CHECK_INIT;
	
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_TBB, 
		ACT_TBB_TMP, tmp);
}

CLIB_API RS cl_tbb_config(cl_handle_t dev_handle, cl_tbb_config_set_t *pconfig)
{
	CL_CHECK_INIT;
	
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TBB,
		ACT_TBB_CONFIG,(u_int8_t *)pconfig,sizeof(*pconfig));	
}

CLIB_API RS cl_tbb_bind_dev_bar_code(cl_handle_t dev_handle,u_int8_t* bar_code)
{ 
	u_int8_t buf[32] = {0};
	
	CL_CHECK_INIT;
	
	if(bar_code != NULL){
		strncpy((char*)buf,(char*)bar_code,sizeof(buf));
	}

	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_TBB, 
		ACT_TBB_BIND,buf,sizeof(buf));
}

////////////////////////////////
//海迅养生壶
CLIB_API RS cl_hx_ysh_ctrl_onoff(cl_handle_t dev_handle, u_int8_t on_off)
{
    CL_CHECK_INIT;
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_HX_YSH, ACT_HX_YSH_CTRL_ONOFF, on_off);
}

CLIB_API RS cl_hx_ysh_ctrl_scene(cl_handle_t dev_handle, u_int8_t aciton, u_int16_t scene_id)
{
    u_int32_t value = BUILD_U32_FROM_U16(aciton, scene_id);
    
    CL_CHECK_INIT;
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_HX_YSH, ACT_HX_YSH_CTRL_DEL_OR_STOP_SCENE , value);
}

CLIB_API RS cl_hx_ysh_exec_scene(cl_handle_t dev_handle, cl_hx_ysh_scene_t* scene)
{
    CL_CHECK_INIT;
    
    if (!scene) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_HX_YSH, ACT_HX_YSH_CTRL_EXEC_SCENE,(u_int8_t*)scene,sizeof(*scene));
}

CLIB_API RS cl_hx_ysh_modify_scene(cl_handle_t dev_handle, cl_hx_ysh_scene_t* scene)
{
    CL_CHECK_INIT;
    
    if (!scene) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_HX_YSH, ACT_HX_YSH_CTRL_MODIFY_SCENE,(u_int8_t*)scene,sizeof(*scene));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//月兔
CLIB_API RS cl_yt_ctrl(cl_handle_t dev_handle, YT_ACTION_T action, u_int8_t value)
{
	u_int16_t data;

	CL_CHECK_INIT;
	data = BUILD_U16((u_int8_t)action, value);

	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YT,
	                         ACT_YT_CTRL, data);
}

CLIB_API RS cl_yt_timer_config(cl_handle_t dev_handle, cl_yt_timer_t *ptimer)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_YT,
		ACT_YT_TIMER,(u_int8_t *)ptimer,sizeof(*ptimer));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//澳德绅
CLIB_API RS cl_ads_ctrl(cl_handle_t dev_handle, ADS_ACTION_T action, u_int8_t value)
{
	u_int16_t data;

	CL_CHECK_INIT;
	data = BUILD_U16((u_int8_t)action, value);

	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ADS,
	                         ACT_ADS_CTRL, data);
}

CLIB_API RS cl_ads_conf(cl_handle_t dev_handle,  cl_ads_conf_t *pconf)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_ADS,
		ACT_ADS_CONF,(u_int8_t *)pconf,sizeof(*pconf));	
}

/////////////////////////////////////////////////////////
//晶石微波炉

CLIB_API RS cl_js_wave_ctrl(cl_handle_t dev_handle,cl_js_wave_work_setting_t* setting)
{
    CL_CHECK_INIT;
    return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_JS_WAVE,
                                   ACT_JS_WAVE_CTRL,(u_int8_t *)setting,sizeof(*setting));
}

CLIB_API RS cl_js_wave_ctrl_auto_menu(cl_handle_t dev_handle,u_int8_t action,u_int8_t menu_id)
{
    u_int16_t value = BUILD_U16(action, menu_id);
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JS_WAVE,
                              ACT_JS_AUTO_MENU, value);
}

CLIB_API RS cl_js_wave_fast_ctrl(cl_handle_t dev_handle,u_int8_t action)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JS_WAVE,
                             ACT_JS_FAST_CTRL, action);
}

CLIB_API RS cl_js_wave_ctrl_child_lock(cl_handle_t dev_handle,u_int8_t on_off)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_JS_WAVE,
                              ACT_JS_CHILD_LOCK, on_off);
}

////////////////////////////////////////////////////////////////////////////////////////
// 科希曼温控器

CLIB_API RS cl_kxm_ctrl_host_onoff(cl_handle_t dev_handle,u_int8_t on_off)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_KXM,
                             ACT_KXM_CTRL_ONOFF, on_off);
}

CLIB_API RS cl_kxm_ctrl_timer(cl_handle_t dev_handle,u_int8_t timer_index,cl_kxm_timer_info_t* timer)
{
    u_int8_t buff[64] = {0};
    CL_CHECK_INIT;
    
    buff[0] = timer_index;
    memcpy(&buff[4], timer, sizeof(*timer));
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_KXM,
                             ACT_KXM_CTRL_TIMER, buff,sizeof(buff));
}

CLIB_API RS cl_kxm_ctrl_mode(cl_handle_t dev_handle,u_int8_t mode,u_int8_t temp)
{
    u_int16_t value = BUILD_U16(mode, temp);
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_KXM,
                             ACT_KXM_CTRL_MODE, value);
}

CLIB_API RS cl_kxm_set_dev_time(cl_handle_t dev_handle,u_int8_t min,u_int8_t sec)
{
    u_int16_t value = BUILD_U16(min,sec);
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_KXM,
                              ACT_KXM_CTRL_DEV_TIME, value);
}

CLIB_API RS cl_kxm_ctrl_all_timer(cl_handle_t dev_handle,cl_kxm_timer_info_t timer[3])
{
    CL_CHECK_INIT;
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_KXM,
                                   ACT_KXM_CTRL_ALL_TIMER, (u_int8_t*)(&timer[0]),sizeof(cl_kxm_timer_info_t)*3);
}

CLIB_API RS cl_kxm_ther_ctrl(cl_handle_t dev_handle,u_int8_t action,u_int8_t value)
{
    u_int16_t data = BUILD_U16(action, value);
    CL_CHECK_INIT;
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_KXM_THER,
                              ACT_KXM_THER_COMMON_CTRL, data);
}

///////////////////////////////////////////////////////////////
 //思博特温控器

CLIB_API RS cl_sbt_ther_ctrl_stat(cl_handle_t dev_handle,cl_sbt_work_ctrl_t* wc)
{
    CL_CHECK_INIT;
    
    if (!wc) {
        return RS_INVALID_PARAM;
    }
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_SBT_THER,
                                   ACT_SBT_CTRL_STAT, (u_int8_t*)(wc),sizeof(*wc));
}

CLIB_API RS cl_sbt_ther_set_param(cl_handle_t dev_handle,cl_sbt_func_setting_t* fs)
{
    CL_CHECK_INIT;
    
    if (!fs) {
        return RS_INVALID_PARAM;
    }
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_SBT_THER,
                                   ACT_SBT_SETTINT_PARAM, (u_int8_t*)(fs),sizeof(*fs));
}

CLIB_API RS cl_sbt_ther_set_ajust_scm_time(cl_handle_t dev_handle,cl_sbt_time_adjust_t* st)
{
    CL_CHECK_INIT;
    
    if (!st) {
        return RS_INVALID_PARAM;
    }
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_SBT_THER,
                                   ACT_SBT_AJUST_TIME, (u_int8_t*)(st),sizeof(*st));
}

CLIB_API RS cl_sbt_ther_set_smart_config(cl_handle_t dev_handle,cl_smart_smart_ctrl_t* ss)
{
    CL_CHECK_INIT;
    
    if (!ss) {
        return RS_INVALID_PARAM;
    }
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_SBT_THER,
                                   ACT_SBT_SMART_MODE_PARAM, (u_int8_t*)(ss),sizeof(*ss));
}

CLIB_API RS cl_sbt_ther_set_smart_config_mode(cl_handle_t dev_handle,u_int8_t onoff)
{
    CL_CHECK_INIT;
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_SBT_THER, ACT_SBT_SMART_MODE_ENABLE, onoff);
}

///////////////////////////////////////////////////////////////
 // 益佳智能电暖炉
CLIB_API RS cl_yj_heater_set_ctrl(cl_handle_t dev_handle, cl_yj_heater_set_t *ctrl)
{
	CL_CHECK_INIT;
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YJ_HEATER, ACT_YJ_HEATER_CTRL, (u_int8_t*)(ctrl), sizeof(*ctrl));
}

///////////////////////////////////////////////////////////////
 // 印度车载追踪器


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
CLIB_API RS cl_indiacar_do_dev_upgrade_request(cl_handle_t dev_handle, u_int8_t major, u_int8_t minor, u_int32_t svn, char *url)
{
	u_int8_t buf[1024] = {0};
	india_car_dev_upgrade_set_t *request = (india_car_dev_upgrade_set_t *)&buf[0];
		
	CL_CHECK_INIT;

	if (strlen(url) > 256) {
		return RS_INVALID_PARAM;
	}

	request->major = major;
	request->minor = minor;
	request->svn = svn;
	request->url_len = (u_int16_t)strlen(url);

	memcpy(request->url, url, request->url_len);
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_UPGRADE, (u_int8_t*)(request), sizeof(*request) + request->url_len);
}

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
CLIB_API RS cl_indiacar_do_history_request(cl_handle_t dev_handle, u_int8_t request_type, u_int8_t journey_id, u_int32_t year, u_int32_t month, u_int32_t day)
{
	india_car_history_request_t request;
		
	CL_CHECK_INIT;

	if (request_type > 5) {
		return RS_INVALID_PARAM;
	}

	request.type = request_type;
	request.id = journey_id;
	request.date = year << 16 | month << 8 | day;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_HISTORY, (u_int8_t*)(&request), sizeof(request));
}

/*
	功能 请求某天已下载的旅程ID以及该旅程ID对应下载好的情况
		
	输入参数:
		@dev_handle: 设备的句柄
		@year: 年20xx
		@month: 月1-12
		@day: 日1-31
	返回:
		cl_indiacar_journey_id_list_t: 里面每个value的值表述一个旅程ID，比如下标为2的值表示旅程ID为2的状态
	       0: 还没有该旅程ID的记录 
		1: 有该旅程ID对应的统计信息
		2: 有该旅程ID对应的详细信息
		3: 统计和详细信息都有
	备注:
	*/

CLIB_API cl_indiacar_journey_id_list_t *cl_indiacar_journey_id_stat_get(cl_handle_t dev_handle, u_int32_t year, u_int32_t month, u_int32_t day)
{
	cl_indiacar_journey_id_list_t *list;
	user_t *user;
	FILE *fp  = NULL;
	u_int8_t id;
	char path[256];
	u_int32_t date = 0, magic = 0;
	india_car_history_flash_hd_t hd;

	date = year << 16 | month << 8 | day;

	// 获取本地当天已经下载的旅程情况
	user = lookup_by_handle(HDLT_USER, dev_handle);
	if (!user) {
		return NULL;
	}

	list = cl_calloc(1, sizeof(*list));
	if (!list) {
		return NULL;
	}

	memset(list, 0, sizeof(*list));

	for (id = 1; id < 254; id++) {
		// 尝试打开统计信息
		sprintf(path, "%s/%012"PRIu64"/indiacar_history/%u/%u/info.dat", cl_priv->dir, user->sn, date, id);
		if ((fp = fopen(path, "r")) != NULL) {
			fread(&hd, sizeof(hd), 1, fp);

			if (hd.magic == INDIACAR_HISTORY_MAGIC) {
				list->ids[id] |= BIT(0);
			}
			
			fclose(fp);
		}

		// 尝试打开详细信息
		sprintf(path, "%s/%012"PRIu64"/indiacar_history/%u/%u/detail.dat", cl_priv->dir, user->sn, date, id);
		if ((fp = fopen(path, "r")) != NULL) {
			fread(&hd, sizeof(hd), 1, fp);

			if (hd.magic == INDIACAR_HISTORY_MAGIC) {
				list->ids[id] |= BIT(1);
			}
			
			fclose(fp);
		}
	}
	
	return list;
}

CLIB_API void cl_indiacar_journey_id_list_free(cl_indiacar_journey_id_list_t *list)
{
	cl_free(list);
}

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
CLIB_API india_car_history_flash_hd_t *cl_indiacar_journey_infomation_get(cl_handle_t dev_handle, u_int8_t type, u_int8_t id, u_int32_t year, u_int32_t month, u_int32_t day)
{
	india_car_history_flash_hd_t *hd = NULL;
	u_int32_t date;
	user_t *user;
	char path[256];
	int file_size;
	FILE *fp = NULL;
	struct stat st;

	date = year << 16 | month << 8 | day;

	// 获取本地当天已经下载的旅程情况
	user = lookup_by_handle(HDLT_USER, dev_handle);
	if (!user) {
		return NULL;
	}

	if (type == 1) {
		sprintf(path, "%s/%012"PRIu64"/indiacar_history/%u/%u/info.dat", cl_priv->dir, user->sn, date, id);
	} else if (type == 2) {
		sprintf(path, "%s/%012"PRIu64"/indiacar_history/%u/%u/detail.dat", cl_priv->dir, user->sn, date, id);
	} else {
		return NULL;
	}
	
	if ((fp = fopen(path, "r")) == NULL) {
		log_err(true, "open %s failed\n", path);
		return NULL;
	}

	if (stat(path, &st) != 0) {
		log_err(true, "stat %s failed\n", path);
		goto failed;
	}

	file_size = (int)st.st_size;

	if (file_size < sizeof(*hd) || file_size >= INDIACAR_HITORY_MAX_FILE_SIZE) {
		log_err(false, "file %s size %u invalid\n", path, file_size);
		goto failed;
	}

	if ((hd = cl_calloc(1, file_size)) == NULL) {
		goto failed;
	}

	if (fread(hd, file_size, 1, fp) <= 0) {
		log_err(true, "fread %s failed\n", path);
		goto failed;
	}

	if (hd->magic != INDIACAR_HISTORY_MAGIC) {
		log_err(true, "magic 0x%u invalid\n", hd->magic);
		goto failed;
	}

	return hd;

failed:
	if (fp) {
		fclose(fp);
	}

	if (hd) {
		cl_free(hd);
	}
	
	return NULL;
}

CLIB_API void cl_indiacar_journey_infomation_free(india_car_history_flash_hd_t *hd)
{
	cl_free(hd);
}


CLIB_API RS cl_indiacar_warn_set(cl_handle_t dev_handle, india_car_warn_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_WARN_SETTING, (u_int8_t*)(request), sizeof(*request));
}

CLIB_API RS cl_indiacar_wifi_config(cl_handle_t dev_handle, india_car_wifi_config_t *request)
{
	CL_CHECK_INIT;

	if (request->pwd_len + request->ssid_len > 256) {
		return RS_INVALID_PARAM;
	}
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_WIFI_CONFIG, (u_int8_t*)(request), sizeof(*request) - sizeof(request->data)+ request->pwd_len + request->ssid_len + 2);
}

CLIB_API RS cl_indiacar_reatime_trip_request(cl_handle_t dev_handle, u_int8_t type)
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_REALTIME_TRIP, type);
}

CLIB_API RS cl_indiacar_get_jorney_count(cl_handle_t dev_handle, cl_indiacar_jorney_num_t *jn)
{
	user_t *user;
	smart_air_ctrl_t* ac;
	cl_indiacar_info_t *priv_info;

	user = lookup_by_handle(HDLT_USER, dev_handle);
	
	if (!user || !user->is_udp_ctrl || !user->smart_appliance_ctrl ) {
		log_err(false, "cl_indiacar_get_jorney_count error handle %08x\n", dev_handle);
		return RS_INVALID_PARAM;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;

	priv_info = ac->com_udp_dev_info.device_info;

	memcpy(jn, &priv_info->jn, sizeof(*jn));
	
	return RS_OK;
}

CLIB_API RS cl_indiacar_debug_config(cl_handle_t dev_handle, india_car_debug_config_t *request)
{
	CL_CHECK_INIT;

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_DEBUG_CONFIG, (u_int8_t*)(request), sizeof(*request));
}

CLIB_API RS cl_indiacar_video_start(cl_handle_t dev_handle, bool onoff)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_VIDEO, (u_int8_t)onoff);
}

CLIB_API RS cl_indiacar_mp4_decode(cl_handle_t dev_handle, u_int8_t action, char *path, u_int64_t seek)
{
	cl_indiancar_mp4_decode_request_t request;

	request.action = action;
	request.seek = seek;
	
	if (strlen(path) >= sizeof(request.path) + 1) {
		return RS_INVALID_PARAM;
	}

	if (action > 3) {
		return RS_INVALID_PARAM;
	}
	
	strcpy(request.path, path);
	
	CL_CHECK_INIT;

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_DECODE_MP4, (u_int8_t*)&request, sizeof(request));
}




CLIB_API RS cl_indiaocar_video_get_picture(cl_handle_t dev_handle, void **pic, u_int32_t *size)
{
	RS ret = RS_OK;
	user_t *user;
	smart_air_ctrl_t* ac;
	cl_indiacar_info_t *priv_info;
	ica_client_t *icc;
	video_t *video;
	data_buf_t *db;

	CL_CHECK_INIT;

	*pic = NULL;
	*size = 0;
	
	cl_lock(&cl_priv->mutex);
	
	user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
	if (user == NULL) {
		log_err(false, "cl_indiaocar_video_get_picture: lookup dev_handle=0x%08x failed\n", dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->sub_type != IJ_INDIACAR) {
		log_err(false, "cl_indiaocar_video_get_picture: sub_type %u != IJ_INDIACAR\n", user->sub_type);
		ret = RS_INVALID_PARAM;
		goto done;
	}

	if (!user->smart_appliance_ctrl) {
		log_err(false, "cl_indiaocar_video_get_picture: user->smart_appliance_ctrl is null\n");
		ret = RS_INVALID_PARAM;
		goto done;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
//		log_err(false, "indiacar_proc_notify error handle %08x\n",info->handle);
		ret = RS_INVALID_PARAM;
		goto done;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;
	
	icc = priv_info->icc;
	if (!icc) {
		log_err(false, "icc is null\n");
		ret = RS_NOT_INIT;
		goto done;
		
	}

	video = icc->video;
	
	if (video->data_pic == NULL) {
		ret = RS_NOT_INIT;
		goto done;
	}
	
	db = de_get_read(video->data_pic);
    if (!db) {
        *pic = NULL;
        *size = 0;
        goto done;
    }
	*pic = (void *)&db->data[0];
	*size = db->data_size;

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

CLIB_API RS cl_indiaocar_video_get_mp4_picture(cl_handle_t dev_handle, cl_indiacar_mp4_info *info, void **pic, u_int32_t *size)
{
	RS ret = RS_OK;
	user_t *user;
	smart_air_ctrl_t* ac;
	cl_indiacar_info_t *priv_info;
	ica_mp4_decode_t *decode;
	data_buf_t *db;

	CL_CHECK_INIT;

	*pic = NULL;
	*size = 0;
	
	cl_lock(&cl_priv->mutex);
	
	user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
	if (user == NULL) {
		log_err(false, "cl_indiaocar_video_get_picture: lookup dev_handle=0x%08x failed\n", dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->sub_type != IJ_INDIACAR) {
		log_err(false, "cl_indiaocar_video_get_picture: sub_type %u != IJ_INDIACAR\n", user->sub_type);
		ret = RS_INVALID_PARAM;
		goto done;
	}

	if (!user->smart_appliance_ctrl) {
		log_err(false, "cl_indiaocar_video_get_picture: user->smart_appliance_ctrl is null\n");
		ret = RS_INVALID_PARAM;
		goto done;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
//		log_err(false, "indiacar_proc_notify error handle %08x\n",info->handle);
		ret = RS_INVALID_PARAM;
		goto done;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;
	
	decode = (ica_mp4_decode_t *)priv_info->mp4_decode;
	
	if (decode->data_pic == NULL) {
		ret = RS_NOT_INIT;
		goto done;
	}
	
	db = de_get_read(decode->data_pic);
    if (!db) {
        *pic = NULL;
        *size = 0;
        goto done;
    }
	*pic = (void *)&db->data[0];
	*size = db->data_size;

	info->duration = decode->mp4_duration;

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}


CLIB_API RS cl_indiaocar_video_get_audio(cl_handle_t dev_handle, void **audio, u_int32_t *size)
{
	RS ret = RS_OK;
	user_t *user;
	smart_air_ctrl_t* ac;
	cl_indiacar_info_t *priv_info;
	ica_client_t *icc;
	video_t *video;
	data_buf_t *db;

	CL_CHECK_INIT;

	*audio = NULL;
	*size = 0;
	
	cl_lock(&cl_priv->mutex);
	
	user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
	if (user == NULL) {
		log_err(false, "cl_indiaocar_video_get_audio: lookup dev_handle=0x%08x failed\n", dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->sub_type != IJ_INDIACAR) {
		log_err(false, "cl_indiaocar_video_get_audio: sub_type %u != IJ_INDIACAR\n", user->sub_type);
		ret = RS_INVALID_PARAM;
		goto done;
	}

	if (!user->smart_appliance_ctrl) {
		log_err(false, "cl_indiaocar_video_get_picture: user->smart_appliance_ctrl is null\n");
		ret = RS_INVALID_PARAM;
		goto done;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
//		log_err(false, "indiacar_proc_notify error handle %08x\n",info->handle);
		ret = RS_INVALID_PARAM;
		goto done;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;
	
	icc = priv_info->icc;
	if (!icc) {
		log_err(false, "icc is null\n");
		ret = RS_NOT_INIT;
		goto done;
		
	}

	video = icc->video;
	
	if (video->data_pic == NULL) {
		ret = RS_NOT_INIT;
		goto done;
	}
	
	db = de_get_read(video->data_sound);
    if (!db) {
		ret = RS_EMPTY;
        *audio = NULL;
        *size = 0;
        goto done;
    }
	*audio = (void *)&db->data[0];
	*size = db->data_size;

	de_move_read(video->data_sound);

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

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
CLIB_API RS cl_indiaocar_request_local_watch(cl_handle_t dev_handle, u_int8_t action, u_int32_t year, u_int32_t month, u_int32_t day)
{
	india_car_local_watch_get_t request;

	CL_CHECK_INIT;

	memset(&request, 0x00, sizeof(request));

	if (action == 0 || action > 2) {
		return RS_INVALID_PARAM;
	}

	request.action = action;
	request.date = year << 16 | month << 8 | day;
	request.date = ntohl(request.date);

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_LOCAL_VIDEO_WATCH, (u_int8_t*)(&request), sizeof(request));
}

CLIB_API RS cl_indiacar_video_record(cl_handle_t dev_handle, char *mp4_path, bool onoff)
{
	char path[1] = {0};
	
	CL_CHECK_INIT;

	if (onoff == true) {
		return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_RECORD, mp4_path, (int)(strlen(mp4_path) + 1));
	}

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_INDIACAR, ACT_INDIACAR_REQUEST_RECORD, path, 1);
}

// LINKON温控器
/*
	功能:LINKON温控器基本控制接口
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型,ACT_LINKON_XXX
		@value: 控制值
	输出参数:
		无
	返回:
		
*/
CLIB_API RS cl_linkon_sample_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;

	
	return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, 
		CLNE_COMMON_UDP_LINKON, action, value);
	
}

//智皇窗帘
CLIB_API RS cl_zhcl_status_set(cl_handle_t dev_handle, u_int8_t status)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHCL, ACT_ZHCL_STATUS, status);
}

CLIB_API RS cl_zhcl_location_set(cl_handle_t dev_handle, u_int8_t location)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHCL, 
		ACT_ZHCL_LOCATION, location);
}

CLIB_API RS cl_zhcl_bind(cl_handle_t dev_handle, u_int32_t magic, u_int8_t index, u_int8_t type)
{
	u_int8_t buf[100];
	ucp_zhcl_bind_t *pbind = (ucp_zhcl_bind_t *)buf;

	CL_CHECK_INIT;
	pbind->magic = magic;
	pbind->index = index;
	pbind->type = type;

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHCL, 
		ACT_ZHCL_BIND, buf, sizeof(*pbind));
}

CLIB_API RS cl_zhcl_type_set(cl_handle_t dev_handle, u_int8_t type, u_int8_t index)
{
	u_int8_t buf[100];
	ucp_zhcl_type_t *ptype = (ucp_zhcl_type_t *)buf;

	CL_CHECK_INIT;
	ptype->index = index;
	ptype->type = type;

	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHCL, 
		ACT_ZHCL_TYPE, buf, sizeof(*ptype));
}

CLIB_API RS cl_zhcl_dir_set(cl_handle_t dev_handle, u_int8_t dir)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHCL, ACT_ZHCL_DIR, dir);
}

CLIB_API RS cl_zhdhx_onoff(cl_handle_t dev_handle, u_int8_t num, u_int8_t onoff)
{
	int len = 0;
	u_int8_t buf[100];
	
	CL_CHECK_INIT;

	len = 0;
	buf[len++] = num;
	buf[len++] = onoff;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHDHX, 
		ACT_ZHDHX_ONOFF, buf, len);
}

CLIB_API RS cl_zhdhx_key_name_set(cl_handle_t dev_handle, u_int8_t num, char *name)
{
	int len = 0;
	int str_len = 0;
	u_int8_t buf[100];
	
	CL_CHECK_INIT;

	if (!name) {
		return RS_INVALID_PARAM;
	}

	len = 0;
	str_len = (int)strlen(name) + 1;
	buf[len++] = num;
	buf[len++] = str_len;
	memcpy(&buf[len], (void *)name, str_len);
	len += str_len;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_ZHDHX, 
		ACT_ZHDHX_KEY_NAME, buf, len);
}



