#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"


CLIB_API RS cl_sa_air_ctrl(cl_handle_t dev_handle, cl_air_work_stat_t* stat)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_STAT, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->stat.onoff = stat->onoff?AC_POWER_ON:AC_POWER_OFF;
    cln_sa->stat.mode = stat->mode;
    cln_sa->stat.wind_direct = stat->wind_direct;
    cln_sa->stat.wind = stat->wind;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_air_ctrl_power(cl_handle_t dev_handle, bool onoff)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_POWER, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->stat.onoff = onoff;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_air_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_MODE, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->stat.mode = mode;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_air_ctrl_temp(cl_handle_t dev_handle, u_int8_t temp)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (temp < AC_TEMP_BASE || temp > AC_TEMP_BASE+16) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_TEMP, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->stat.temp = temp;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_air_ctrl_wind(cl_handle_t dev_handle, u_int8_t wind)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (wind > CL_AC_WIND_LOW) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_WIND, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->stat.wind = wind;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_air_ctrl_direct(cl_handle_t dev_handle, u_int8_t direct)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (direct > CL_AC_DIR_3) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_DIRECT, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->stat.wind_direct = direct;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_sa_old_air_ctrl(cl_handle_t dev_handle, u_int8_t key_id)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_OLD_AIR, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->old_key= key_id;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;

}


CLIB_API RS cl_sa_ctrl_led_power(cl_handle_t dev_handle, u_int8_t onoff)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CTRL_LED, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->led_on_off = onoff;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;

}

CLIB_API RS cl_sa_set_air_peak_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_PEAK_PERIOD, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->begin_time = begin_time;
    cln_sa->last_time = last_minute;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_sa_set_air_valley_time(cl_handle_t dev_handle, u_int16_t begin_time,u_int16_t last_minute)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_VALLEY_PERIOD, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->begin_time = begin_time;
    cln_sa->last_time = last_minute;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_sa_set_air_peak_price(cl_handle_t dev_handle, u_int32_t price)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_PEAK_PRICE, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->temp_value = price;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_set_air_valley_price(cl_handle_t dev_handle, u_int32_t price)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_VALLEY_PRICE, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->temp_value = price;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_sa_set_air_flat_price(cl_handle_t dev_handle, u_int32_t price)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_FLAT_PRICE, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->temp_value = price;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_add_air_timer(cl_handle_t dev_handle, cl_air_timer_t* time_info)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!time_info) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_MODIFY_TIMER, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->timer_info.id = time_info->id;
    cln_sa->timer_info.enable = time_info->enable;
    cln_sa->timer_info.hour = time_info->hour;
    cln_sa->timer_info.week = time_info->week;
    cln_sa->timer_info.onoff = time_info->onoff;
    cln_sa->timer_info.minute = time_info->minute;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_sa_add_air_period_timer(cl_handle_t dev_handle, cl_period_timer_t* time_info)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!time_info) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_MODIFY_PERIOD_TIMER, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->timer_info.id = time_info->id;
    cln_sa->timer_info.enable = time_info->enable;
    cln_sa->timer_info.hour = time_info->hour;
    cln_sa->timer_info.week = time_info->week;
    cln_sa->timer_info.onoff = time_info->onoff;
    cln_sa->timer_info.minute = time_info->minute;
    cln_sa->dev_handle = dev_handle;
    cln_sa->timer_info.duration = time_info->duration;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_del_air_timer(cl_handle_t dev_handle, u_int8_t timer_id)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!timer_id) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_DEL_TIMER, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->timer_info.id = timer_id;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_sa_del_period_timer(cl_handle_t dev_handle, u_int8_t timer_id)
{
	 cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!timer_id) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_PERIOD_DEL_TIMER, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->timer_info.id = timer_id;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}


CLIB_API RS cl_sa_start_code_match(cl_handle_t dev_handle, bool all_match,u_int32_t timeout)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!timeout) {
        //默认120秒
        timeout = 120;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_CODE_MATCH_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_CODE_MATCH_START;
    cln_sa->code_match.mode = all_match?CL_AIR_CODE_MATCH_ALL:CL_AIR_CODE_MATCH_CLOUD;
    cln_sa->code_match.time_out = timeout;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_get_code_match_stat(cl_handle_t dev_handle,cl_air_code_match_stat_t* stat)
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
		log_err(false, "cl_sa_get_code_match_stat request 0x%08x failed: not found\n", dev_handle);
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
    
    memcpy(stat, &ac->match_stat, sizeof(*stat));
    
    
done:
	cl_unlock(&cl_priv->mutex);
    
    return RS_OK;
}

CLIB_API RS cl_sa_stop_code_match(cl_handle_t dev_handle)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_CODE_MATCH_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_CODE_MATCH_STOP;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}


CLIB_API RS cl_sa_set_smart_power_on(cl_handle_t dev_handle, bool enable)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_SMART_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_SMART_POWER_ON;
    cln_sa->temp_value = !!enable;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;

}


CLIB_API RS cl_sa_set_smart_power_off(cl_handle_t dev_handle, bool enable)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_SMART_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_SMART_POWER_OFF;
    cln_sa->temp_value = !!enable;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}


CLIB_API RS cl_sa_set_smart_sleep(cl_handle_t dev_handle, bool enable)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_SMART_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_SMART_SLEEP;
    cln_sa->temp_value = !!enable;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;

}

CLIB_API RS cl_sa_air_get_real_time_data(cl_handle_t dev_handle,cl_air_real_time_data_t* rdata)
{
    RS ret = ERR_NONE;
    user_t* user;
    smart_appliance_ctrl_t* sac;
    smart_air_ctrl_t* ac;
    
    CL_CHECK_INIT;
    
    if (!rdata) {
        return RS_INVALID_PARAM;
    }
    
    memset(rdata, 0, sizeof(*rdata));
    
    cl_lock(&cl_priv->mutex);
    
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "cl_get_real_time_data 0x%08x failed: not found\n", dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
    
    if (!(sac = user->smart_appliance_ctrl)) {
        ret = RS_NOT_FOUND;
        goto done;
    }
	
    if ( !(ac = sac->sub_ctrl)) {
        goto done;
    }
    
    ac->last_get_power_time = get_sec();
    
    rdata->cur_power = ac->air_info.cur_power;
    
done:
	cl_unlock(&cl_priv->mutex);
    
    return RS_OK;
}

CLIB_API RS cl_sa_air_start_get_cur_power(cl_handle_t dev_handle,u_int8_t time_interval)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_POWER_START, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->temp_value = time_interval;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_air_stop_get_cur_power(cl_handle_t dev_handle)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_POWER_START, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_STOP_CUR_POWER;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_set_smart_power_on_detail(cl_handle_t dev_handle,cl_smart_air_on_param_t* ao)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!ao) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_SMART_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_SMART_POWER_ON_DETAIL;
    cln_sa->dev_handle = dev_handle;
    memcpy(&cln_sa->share_struct, ao, sizeof(*ao));
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;

}

CLIB_API RS cl_sa_set_smart_power_off_detail(cl_handle_t dev_handle,cl_smart_air_off_param_t* af)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!af) {
        return RS_INVALID_PARAM;
    }
	pkt = cl_notify_pkt_new(1024, CLNE_SA_SMART_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_SMART_POWER_OFF_DETAIL;
    cln_sa->dev_handle = dev_handle;
    memcpy(&cln_sa->share_struct, af, sizeof(*af));
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;

}


CLIB_API RS cl_sa_set_smart_sleep_detail(cl_handle_t dev_handle,cl_smart_air_sleep_param_t* as)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!as) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_SMART_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->action = SA_ACT_SMART_SLEEP_DETAIL;
    cln_sa->dev_handle = dev_handle;
    memcpy(&cln_sa->share_struct, as, sizeof(*as));
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;

}

CLIB_API RS cl_sa_air_refresh_timer_info(cl_handle_t dev_handle)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_REFRESH_TIMER, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_air_refresh_elec_info(cl_handle_t dev_handle)
{
	 cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_REFRESH_ELEC, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_air_clear_elec_stat_info(cl_handle_t dev_handle,int type)
{
	 cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_CLEAR_ELEC, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
	cln_sa->action = (u_int32_t)type;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_air_set_led_color(cl_handle_t dev_handle,int on_color,int off_color)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_SA_AIR_LED_COLOR, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
	cln_sa->share_struct[0] = on_color;
	cln_sa->share_struct[1] = off_color;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_sa_air_trans_send(cl_handle_t dev_handle, u_int8_t *buf, int len)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	if(len > MAX_TRANS_LEN)
		return RS_INVALID_PARAM;

	pkt = cl_notify_pkt_new(sizeof(*pkt) + sizeof(*cln_sa), CLNE_SA_TRANS_SEND, CLNPF_ACK);
	pkt->param_len = sizeof(*cln_sa);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->trans_data.trans_len = len;
	memcpy(cln_sa->trans_data.trans_buf, buf, len);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API int cl_sa_get_trans_ir(cl_handle_t dev_handle, u_int8_t *buf, int len)
{
	int ret = 0;
	user_t* user;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* ac;

	CL_CHECK_INIT;

	if (!buf || len <= 0) {
		return 0;
	}

	cl_lock(&cl_priv->mutex);
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "cl_get_real_time_data 0x%08x failed: not found\n", dev_handle);
		goto done;
	}

	if (!(sac = user->smart_appliance_ctrl)) {
		goto done;
	}
	
	if ( !(ac = sac->sub_ctrl)) {
		goto done;
	}
	
	if(ac->ir_len){
		ret = ac->ir_len < len ? ac->ir_len : len;
		memcpy(buf, ac->ir_buf, ret);
	}
	
done:
	cl_unlock(&cl_priv->mutex);
		return ret;
}

CLIB_API int cl_sa_get_trans_sound(cl_handle_t dev_handle, u_int8_t *buf, int len)
{
	int ret = 0;
	user_t* user;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* ac;
	sound_link_data_t *sound;
	

	CL_CHECK_INIT;

	if (!buf || len <= 0) {
		return 0;
	}

	cl_lock(&cl_priv->mutex);
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "cl_get_real_time_data 0x%08x failed: not found\n", dev_handle);
		goto done;
	}

	if (!(sac = user->smart_appliance_ctrl)) {
		goto done;
	}
	
	if ( !(ac = sac->sub_ctrl)) {
		goto done;
	}

	if(stlc_list_empty(&ac->pt_sound))
		goto done;
	sound = stlc_list_first_entry(&ac->pt_sound, sound_link_data_t, link);
	ret = sound->len < len ? sound->len: len;
	memcpy(buf, sound->data, ret);
	stlc_list_del(&sound->link);
	cl_free(sound);	
	
done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}
CLIB_API RS cl_sa_air_restore_factory(cl_handle_t dev_handle)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_RESTORE_FACTORY, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_sa_pt_set_adkb(cl_handle_t dev_handle,  cl_air_pt_adkb_t *v)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_PT_SET_ADKB, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	memcpy(cln_sa->share_struct, v, sizeof(*v));
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_sa_pt_set_adkb_ext(cl_handle_t dev_handle,  cl_air_pt_adkb_t *v)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_PT_SET_ADJ_EXT, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	memcpy(cln_sa->share_struct, v, sizeof(*v));
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_sa_pt_set_adj(cl_handle_t dev_handle,  cl_plug_pt_adkb_t *v)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_PT_SET_ADJ, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	memcpy(cln_sa->share_struct, v, sizeof(*v));
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_sa_air_reset_ir_code_id(cl_handle_t dev_handle,  u_int16_t new_code_id)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_CODE_MATCH_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->temp_value = new_code_id;
	cln_sa->action = SA_ACT_RESET_IR_CODE;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
	
}

CLIB_API RS cl_ac_set_pan_type(cl_handle_t dev_handle,u_int8_t pan_type)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->temp_value = pan_type;
	cln_sa->action = SA_ACT_SET_PAN_TYPE;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_ac_refresh_key_info(cl_handle_t dev_handle)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_REFRESH_KEY_INFO;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_ac_set_key_info(cl_handle_t dev_handle,u_int8_t key_id,char* key_name)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_SET_KEY_INFO;
	cln_sa->temp_value = key_id;
       if(key_name != NULL){
		strncpy(cln_sa->share_struct,key_name,MAX_KEY_NAME_LEN);
		cln_sa->share_struct[MAX_KEY_NAME_LEN-1] = '\0';
	}
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_ac_set_key_info_v2(cl_handle_t dev_handle,u_int8_t key_id,char* key_name, u_int8_t flag, air_key_stat_t air_stat)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_SET_KEY_INFO_V2;
	cln_sa->temp_value = key_id;
	cln_sa->begin_time = (u_int16_t)flag;
    if (key_name != NULL){
		strncpy(cln_sa->share_struct,key_name,MAX_KEY_NAME_LEN);
		cln_sa->share_struct[MAX_KEY_NAME_LEN-1] = '\0';
	}

	if (air_stat.temp >= 16 || air_stat.temp <= 30) {
		air_stat.temp -= AC_TEMP_BASE;
	}
	
	memcpy(&cln_sa->stat, (char*)&air_stat, sizeof(cln_sa->stat));
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}


CLIB_API RS cl_ac_delete_key(cl_handle_t dev_handle,u_int8_t key_id)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_DELETE_KEY;
	cln_sa->temp_value = key_id;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_start_learn_key(cl_handle_t dev_handle,u_int8_t key_id)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_START_LEARN_KEY;
	cln_sa->temp_value = key_id;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_stop_learn_key(cl_handle_t dev_handle,u_int8_t key_id)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_STOP_LEARN_KEY;
	cln_sa->temp_value = key_id;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_ctrl_key(cl_handle_t dev_handle,u_int8_t key_id)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_SA_KEY_LEARN, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->action = SA_ACT_CTRL_KEY;
	cln_sa->temp_value = key_id;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_query_samecode(cl_handle_t dev_handle,u_int8_t *status)
{
	user_t *user;
	RS ret = RS_ERROR;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	
	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, dev_handle)) == NULL) {
		log_err(false, "%s: lookup user handle=0x%08x failed\n", __FUNCTION__, dev_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
        goto done;
    }
	
	air_ctrl = (smart_air_ctrl_t*)sac->sub_ctrl;

	*status = air_ctrl->match_stat.flag&0x01;
	
	ret = RS_OK;
	
done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}

CLIB_API RS cl_ac_modify_onoff_status(cl_handle_t dev_handle,u_int8_t on)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_ISC_SETON, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;
	cln_sa->stat.onoff = on;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_sa_modify_temp_curve(cl_handle_t dev_handle,cl_temp_curve_t *ptmp_curve)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
    tmp_curve_t* tc;
	u_int16_t len,i;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_TEMP_CURVE, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;

	//计算长度
	len = sizeof(*ptmp_curve) + ptmp_curve->count*sizeof(tmp_curve_t);
	if (len > sizeof(cln_sa->trans_data.trans_buf)) {
		return RS_ERROR;
	}
	
    tc = (tmp_curve_t*)(ptmp_curve+1);
    for (i = 0; i< ptmp_curve->count; i++,tc++) {
        tc->dir = 0xFF;
        tc->wind = 0xFF;
    }
    
	cln_sa->trans_data.trans_len = len;
	memcpy((void *)cln_sa->trans_data.trans_buf, (void *)ptmp_curve, len);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_ac_set_temp_ctrl(cl_handle_t dev_handle,cl_temp_ac_ctrl_t *ptmp_ctrl)
{
	cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	u_int16_t len;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_SET_TEMP_CTRL, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;

	//计算长度
	len = sizeof(*ptmp_ctrl);
	
	cln_sa->trans_data.trans_len = len;
	memcpy((void *)cln_sa->trans_data.trans_buf, (void *)ptmp_ctrl, len);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;	
}

CLIB_API RS cl_ac_set_fan_speed_opposite(cl_handle_t dev_handle,u_int8_t is_opposite)
{
    cl_notify_pkt_t *pkt;
    cln_sa_air_info_t *cln_sa;
    RS ret;
    
    CL_CHECK_INIT;
    
    pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_AJUST_FAN_SPEED, CLNPF_ACK);
    pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->temp_value = is_opposite;
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_set_fan_stat(cl_handle_t dev_handle,u_int8_t fan_stat)
{
    cl_notify_pkt_t *pkt;
    cln_sa_air_info_t *cln_sa;
    RS ret;
    
    CL_CHECK_INIT;
    
    pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_AJUST_FAN, CLNPF_ACK);
    pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->temp_value = fan_stat;

	log_debug("xxxddd fan stat = %d\n", fan_stat);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_set_child_lock_stat(cl_handle_t dev_handle,u_int8_t lock_stat)
{
    cl_notify_pkt_t *pkt;
    cln_sa_air_info_t *cln_sa;
    RS ret;
    
    CL_CHECK_INIT;
    
    pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_SET_CHILD_LOCK, CLNPF_ACK);
    pkt->param_len = sizeof(cln_sa_air_info_t);
    cln_sa = (cln_sa_air_info_t*)pkt->data;
    cln_sa->dev_handle = dev_handle;
    cln_sa->temp_value = lock_stat;
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_ac_set_msg_config(cl_handle_t dev_handle, cl_ac_msg_config_t *config)
{
    cl_notify_pkt_t *pkt;
	cln_sa_air_info_t *cln_sa;
	u_int16_t len;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(sizeof(*pkt)+sizeof(*cln_sa), CLNE_SA_SET_I8_MSG_CONFIG, CLNPF_ACK);
	pkt->param_len = sizeof(cln_sa_air_info_t);
	cln_sa = (cln_sa_air_info_t*)pkt->data;
	cln_sa->dev_handle = dev_handle;

	//计算长度
	len = sizeof(*config);

	cln_sa->trans_data.trans_len = len;
	memcpy((void *)cln_sa->trans_data.trans_buf, (void *)config, len);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	
	cl_notify_pkt_free(pkt);
	
	return ret;
}



CLIB_API RS cl_misc_comm_timer_modify_add(cl_handle_t dev_handle, cl_comm_timer_t *ptimer)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(*ptimer);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_TIMER_ADD, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(*ptimer);
	memcpy(cln_mi->data, (void *)ptimer, sizeof(*ptimer));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_misc_comm_timer_del(cl_handle_t dev_handle, u_int8_t id)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	cl_comm_timer_t timer;
	int len = 0;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(timer);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_TIMER_DEL, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(timer);
	timer.id = id;
	memcpy(cln_mi->data, (void *)&timer, sizeof(timer));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;	
}

/*
 功能:
    通用的设置温度曲线
    air_ctrl->com_udp_dev_info.temp_curve
 输入参数:
    @dev_handle 设备句柄
    @ptmp_ctrl: 控制参数
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_modify_temp_curve(cl_handle_t dev_handle, cl_temp_curve_t *ptmp_curve)
{
	cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(*ptmp_curve) + ptmp_curve->count * sizeof(tmp_curve_t);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_TEMP_CURVE, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(*ptmp_curve) + ptmp_curve->count * sizeof(tmp_curve_t);
	memcpy(cln_mi->data, (void *)ptmp_curve, cln_mi->data_len);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_sa_public_history_query(cl_handle_t dev_handle, u_int32_t index, u_int32_t num)
{
	cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
	cl_dev_history_item_hd_t request;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	request.index = index;
	request.num = num;

	len = sizeof(cln_misc_info_t) + sizeof(request);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_HISTORY_QUERY, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(request);
	memcpy(cln_mi->data, (void *)&request, cln_mi->data_len);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);

	return ret;
}

/*
 功能:
    通用的设置恒温
    air_ctrl->com_udp_dev_info.tac
 输入参数:
    @dev_handle 设备句柄
    @ptmp_ctrl: 控制参数
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */

CLIB_API RS cl_sa_public_set_temp_ctrl(cl_handle_t dev_handle, cl_temp_ac_ctrl_t *ptmp_ctrl)
{
	cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(*ptmp_ctrl);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_TEMP_CTRL, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(ptmp_ctrl);
	memcpy(cln_mi->data, (void *)ptmp_ctrl, sizeof(*ptmp_ctrl));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

/*
 功能:
    通用的设置智能开机，查询在
    air_ctrl->smart_home_enable
 输入参数:
    @dev_handle 设备句柄
    @enable: 是否使能
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_smart_on(cl_handle_t dev_handle, u_int8_t enable)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(enable);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_SMART_ON, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(enable);
	memcpy(cln_mi->data, (void *)&enable, sizeof(enable));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

/*
 功能:
    通用的设置童锁，查询在
    air_ctrl->com_udp_dev_info.child_lock_value
 输入参数:
    @dev_handle 设备句柄
    @type: 童锁类型:0-关闭童锁 1-锁所有误操作 2-锁开关操作
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_child_lock(cl_handle_t dev_handle, u_int8_t type)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(type);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_CHILD_LOCK, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(type);
	memcpy(cln_mi->data, (void *)&type, sizeof(type));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

/*
 功能:
    通用的设置开机温度
    air_ctrl->com_udp_dev_info.boot_temp
 输入参数:
    @dev_handle 设备句柄
    @type: 开机温度，摄氏度
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_boot_temp(cl_handle_t dev_handle, u_int8_t enable, u_int8_t temp)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
	u_int16_t value = enable | (temp << 8) & 0xff00;
    
    CL_CHECK_INIT;

	len = sizeof(cln_misc_info_t) + sizeof(value);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_BOOT_TEMP, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(value);
	memcpy(cln_mi->data, (void *)&value, sizeof(value));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}


/*
 功能:
    通用的设置设备外网口配置
    air_ctrl->com_udp_dev_info.wan_config
 输入参数:
    @dev_handle 设备句柄
    @config: 配置信息
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_wan_config(cl_handle_t dev_handle, cl_wan_request_config_t *config)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0, plen;
    
    CL_CHECK_INIT;

	plen = sizeof(*config);
	
	len = sizeof(cln_misc_info_t) + plen;
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_WAN_CONFIG, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = plen;
	memcpy(cln_mi->data, (void *)config, plen);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

/*
 功能:
    通用的设置设备DHCP服务器
    air_ctrl->com_udp_dev_info.dhcp_config
 输入参数:
    @dev_handle 设备句柄
    @config: 配置信息
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_dhcp_server_config(cl_handle_t dev_handle, cl_dhcp_server_config_t *config)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0, plen;
    
    CL_CHECK_INIT;

	plen = sizeof(*config);
	
	len = sizeof(cln_misc_info_t) + plen;
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_DHCP_CONFIG, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = plen;
	memcpy(cln_mi->data, (void *)config, plen);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

/*
 功能:
    通用的设置设备AP
    air_ctrl->com_udp_dev_info.ap_config
 输入参数:
    @dev_handle 设备句柄
    @config: 配置信息
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_ap_config(cl_handle_t dev_handle, cl_ap_config_t *config)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0, plen;
    
    CL_CHECK_INIT;

	plen = sizeof(*config);
	
	len = sizeof(cln_misc_info_t) + plen;
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_AP_CONFIG, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = plen;
	memcpy(cln_mi->data, (void *)config, plen);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

/*
 功能:
    通用的设置设备中继器开关
    air_ctrl->com_udp_dev_info.repeat_onoff
 输入参数:
    @dev_handle 设备句柄
    @onoff: 开关
    
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_repeat_onoff(cl_handle_t dev_handle, u_int8_t onoff)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0, plen;
	u_int8_t request[4] = {0};
    
    CL_CHECK_INIT;

	request[0] = onoff;
	plen = sizeof(request);
	
	len = sizeof(cln_misc_info_t) + plen;
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_REPEAT_CONFIG, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = plen;
	memcpy(cln_mi->data, (void *)&request, plen);
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}




/*
 功能:
    通用的设置温度阀值
    air_ctrl->com_udp_dev_info.temp_alarm_xxx
 输入参数:
    @dev_handle 设备句柄
    @onoff: 开关
    @min: 最小温度
    @max: 最大温度
    
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_temp_alarm(cl_handle_t dev_handle, u_int8_t onoff, u_int8_t min, u_int8_t max)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
    RS ret;
	int len = 0;
	ucp_temp_alarm_t ta;
    
    CL_CHECK_INIT;

	ta.onoff = onoff;
	ta.min = min;
	ta.max = max;

	len = sizeof(cln_misc_info_t) + sizeof(ta);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_TEMP_ALARM, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(ta);
	memcpy(cln_mi->data, (void *)&ta, sizeof(ta));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}


/*
 功能:
    通用的设置童锁，查询在
    air_ctrl->com_udp_dev_info.shortcuts_onoff
 输入参数:
    @dev_handle 设备句柄
    @enable: 是否使能
    @onoff: 0: 关机 1开机
    @time: 时间戳，传入本地时间，SDK负责转为UTC
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_sa_public_set_shortcuts_onoff(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int32_t time)
{
    cl_notify_pkt_t *pkt;
    cln_misc_info_t *cln_mi;
	ucp_shortcuts_onoff_t request;
    RS ret;
	int len = 0;
    
    CL_CHECK_INIT;

	request.onoff = onoff;
	request.enable = enable;
	request.remain_time = (u_int16_t)time;

	len = sizeof(cln_misc_info_t) + sizeof(request);
    pkt = cl_notify_pkt_new(sizeof(*pkt)+ len , CLNE_COMM_SHORTCUTS_ONOFF, CLNPF_ACK);
    pkt->param_len = len;
    cln_mi = (cln_misc_info_t*)pkt->data;
    cln_mi->dev_handle = dev_handle;
	cln_mi->data_len = sizeof(request);
	memcpy(cln_mi->data, (void *)&request, sizeof(request));
    
    ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
    cl_notify_pkt_free(pkt);
    
    return ret;
}

