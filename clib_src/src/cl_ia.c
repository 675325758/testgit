#include "cl_ia.h"
#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
static bool is_udp_handle(cl_handle_t dev_handle);
static RS cl_udp_lcyt_set(cl_handle_t dev_handle,u_int32_t action, u_int16_t value);
static  RS cl_ia_ctrl_v2(cl_handle_t user_handle, int num, ia_status_t *ia_stat)
{
	cl_notify_pkt_t *pkt;
	cln_ia_t *ci;
	RS ret;
	int len, n;


	CL_CHECK_INIT;

	if (!ia_stat) {
		return RS_ERROR;
	}

	len = num * sizeof(ia_status_t) + sizeof(cln_ia_t) + sizeof(cl_notify_pkt_t);
	n = (len + 1024) / 1024;
	pkt = cl_notify_pkt_new(1024 * n, CLNE_IA_CTRL, CLNPF_ACK);
	if (!pkt) {
		return RS_ERROR;
	}

	pkt->param_len = len -  sizeof(cl_notify_pkt_t);
	ci = (cln_ia_t *)&pkt->data[0];
	ci->user_handle = user_handle;
	ci->ns = num;
	while (num--) {
		ci->ia_stat[num].id = ia_stat[num].id;
		ci->ia_stat[num].value = ia_stat[num].value;
	}
	
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}


static  RS cl_ia_ctrl(cl_handle_t user_handle, ia_status_t *ia_stat)
{
	return cl_ia_ctrl_v2(user_handle, 1, ia_stat);
}


static  RS check_ia_type(cl_handle_t dev_handle, u_int8_t _type)
{
	user_t *user = NULL;
	u_int8_t type = 0;

	cl_lock(&cl_priv->mutex);
	
	user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
	if (!user || !user->ia) {
		log_err(false, "Don`t find out user\n");
	} else {
		type = user->ia->type;
	}
	
	cl_unlock(&cl_priv->mutex);

	if (type == _type) {
		return RS_OK;
	}
	
	return RS_ERROR;
}



static RS  inline  _ia_aircleaner_set(cl_handle_t dev_handle, u_int16_t id, u_int16_t value)
{
	ia_status_t  ia_stat;

	CL_CHECK_INIT;

	if (check_ia_type(dev_handle, IA_TYPE(id)) != RS_OK){
		return RS_ERROR;
	}

	ia_stat.id = id;
	ia_stat.value = value;
	return cl_ia_ctrl(dev_handle, &ia_stat);
}


/**************************************************************************************************
	  空气净化器等相关信息 
 **************************************************************************************************/

CLIB_API RS cl_ia_aircleaner_set_onoff(cl_handle_t dev_handle, bool onoff)
{
	if (onoff > 1) {
		return RS_ERROR;
	}
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_WORK, (u_int16_t)onoff);
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCLEANER_STATUS_WORK, (u_int16_t)onoff); 
}

CLIB_API RS cl_ia_aircleaner_set_speed(cl_handle_t dev_handle, u_int8_t speed)
{
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_SPEED, (u_int16_t)speed);
	}
	if (speed > 4 || speed < 2) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCLEANER_STATUS_SPEED, (u_int16_t)speed); 
}

CLIB_API RS cl_ia_aircleaner_set_timer(cl_handle_t dev_handle, u_int32_t min)
{
	if (min > (24 * 60)) {
		return RS_ERROR;
	}
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_TIMER, (u_int16_t)min);
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCLEANER_STATUS_TIMER, (u_int16_t)min); 
}

CLIB_API RS cl_ia_aircleaner_set_ultra(cl_handle_t dev_handle, bool onoff)
{
	if (onoff > 1) {
		return RS_ERROR;
	}
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_ULTRAVIOLET, (u_int16_t)onoff);
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCLEANER_STATUS_ULTRAVIOLET, (u_int16_t)onoff);
}

CLIB_API RS cl_ia_aircleaner_set_anion(cl_handle_t dev_handle, bool onoff)
{
	if (onoff > 1) {
		return RS_ERROR;
	}
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_ANION, (u_int16_t)onoff);
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCLEANER_STATUS_ANION, (u_int16_t)onoff);
}

CLIB_API RS cl_ia_aircleaner_set_mode(cl_handle_t dev_handle, u_int8_t mode)
{
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_MODE, (u_int16_t)mode);
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCLEANER_STATUS_MODE, (u_int16_t)mode);
}

CLIB_API RS cl_ia_aircleaner_query_all(cl_handle_t dev_handle)
{
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_QUERY, 0);
	}
	return RS_ERROR;
}

CLIB_API RS cl_ia_aircleaner_set_terilize(cl_handle_t dev_handle, u_int8_t is_on, u_int8_t minute)
{
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRCLEANER_STATUS_TERILIZE, BUILD_U16(is_on, minute));
	}
	return RS_ERROR;

}

CLIB_API RS cl_ia_aircleaner_add_periodic_timer(cl_handle_t dev_handle, cl_air_timer_t* time_info)
{
	periodic_timer_t ti;
	if(is_udp_handle(dev_handle)){
		memset(&ti, 0, sizeof(ti));
		ti.id = time_info->id;
		ti.enable = time_info->enable;
		ti.hour = time_info->hour;
		ti.week = time_info->week;
		ti.onoff = time_info->onoff;
		ti.minute = time_info->minute;
		return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL,
			IA_AIRCLEANER_STATUS_PERIODIC_TIMER, (u_int8_t *)&ti, sizeof(ti));
	}
	return RS_ERROR;
}

CLIB_API RS cl_ia_aircleaner_del_periodic_timer(cl_handle_t dev_handle, u_int8_t timer_id)
{
	periodic_timer_t ti;
	if(is_udp_handle(dev_handle)){
		memset(&ti, 0, sizeof(ti));
		ti.is_del = 1;
		ti.id = timer_id;
		return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL,
			IA_AIRCLEANER_STATUS_PERIODIC_TIMER, (u_int8_t *)&ti, sizeof(ti));
	}
	return RS_ERROR;
}

/**************************************************************************************************
	  快热炉等相关信息 
 **************************************************************************************************/

CLIB_API RS cl_ia_airheater_set_onoff(cl_handle_t dev_handle, bool onoff)
{
	if (onoff > 1) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_STATUS_ONOFF, (u_int16_t)onoff); 
}

CLIB_API RS cl_ia_airheater_set_gear(cl_handle_t dev_handle, u_int16_t gear)
{
	if (gear < 1 || gear > 3) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_STATUS_GEAR, (u_int16_t)gear); 
}

CLIB_API RS cl_ia_airheater_set_time(cl_handle_t dev_handle, u_int16_t time)
{
	if (time > (15 * 60)) {
		return RS_ERROR;
	}
	if (time % 60 && time != 0) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_STATUS_TIME, (u_int16_t)time); 
}

CLIB_API RS cl_ia_airheater_set_mode(cl_handle_t dev_handle, u_int16_t mode)
{
	if (mode < 1 || mode > 4) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_STATUS_MODE, (u_int16_t)mode); 

}



/**************************************************************************************************
	  快热炉ycyt等相关信息 
 **************************************************************************************************/

static bool is_udp_handle(cl_handle_t dev_handle)
{
	user_t *user;
	bool ret = false;
	
	cl_lock(&cl_priv->mutex);
	
	user = (user_t *)lookup_by_handle(HDLT_USER, dev_handle);
	if(user && user->is_udp_ctrl){
		ret = true;
	}
	
	cl_unlock(&cl_priv->mutex);

	return RS_ERROR;
}

static RS cl_udp_lcyt_set(cl_handle_t dev_handle,u_int32_t action, u_int16_t value)
{
	return cl_send_u16_notify(&cl_priv->thread_main, dev_handle,CLNE_IA_CTRL, action, value);
}

CLIB_API RS cl_ia_airheater_ycyt_set_temp(cl_handle_t dev_handle, u_int16_t temp)
{
	// 范围为1-35
	if (temp < 1 || temp > 35) {
		return RS_ERROR;
	}
	
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_TEMP_SET,temp);
	}else{
		return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_TEMP_SET, (u_int16_t)temp); 
	}
}


CLIB_API RS cl_ia_airheater_ycyt_set_mode(cl_handle_t dev_handle, u_int16_t mode)
{
	// 1:睡眠 2:省电 3:舒适 4:速热 5:温控
	if (mode < 1 || mode > 5) {
		return RS_ERROR;
	}

	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_MODE,mode);
	}else{
		return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_MODE, (u_int16_t)mode); 
	}
}

CLIB_API RS cl_ia_airheater_ycyt_set_gear(cl_handle_t dev_handle, u_int16_t gear)
{
	// 1:关闭 2:低档 3:中档 4:高档
	if (gear < 1 || gear > 4) {
		return RS_ERROR;
	}
	
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_GEAR,gear);
	}else{
		return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_GEAR, (u_int16_t)gear);
	}
	
}

CLIB_API RS cl_ia_airheater_ycyt_set_timer(cl_handle_t dev_handle, u_int16_t time)
{
	if (time > (24 * 60)) {
		return RS_ERROR;
	}
	
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_TIME,time);
	}else{
		return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_TIME,  (u_int16_t)time);
	}
	
}

CLIB_API RS cl_ia_airheater_ycyt_set_onoff(cl_handle_t dev_handle, bool onoff)
{
    onoff = !!onoff;

	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_WORK,onoff);
	}else{
		return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_WORK,  (u_int16_t)onoff);
	}
}

CLIB_API RS cl_ia_airheater_ycyt_set_order_timer(cl_handle_t dev_handle, u_int16_t time)
{
	if (time > (24 * 60)) {
		return RS_ERROR;
	}
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_ORDER_TIME,time);
	}else{
		return _ia_aircleaner_set(dev_handle, IA_AIRHEATER_YCYT_STATUS_ORDER_TIME,  (u_int16_t)time);
	}
	
}

CLIB_API RS cl_ia_airheater_ycyt_refresh_timer(cl_handle_t dev_handle)
{
	if(is_udp_handle(dev_handle)){
		return cl_udp_lcyt_set(dev_handle, IA_AIRHEATER_YCYT_REFRESH_TIMER,0);
	}

	return RS_OK;
}

/**************************************************************************************************
	  热水器等相关信息 
 **************************************************************************************************/

CLIB_API RS cl_ia_waterheater_set_work(cl_handle_t dev_handle, bool work)
{
	if (work > 1) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_STATUS_WORK, (u_int16_t)work); 

}

CLIB_API RS cl_ia_waterheater_set_temp(cl_handle_t dev_handle, u_int16_t temp)
{	
	if (temp < 35 || temp > 75) {
		return RS_ERROR;//35-75
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_STATUS_TEMP_SET, (u_int16_t)temp); 
}

CLIB_API RS cl_ia_waterheater_set_timer(cl_handle_t dev_handle, u_int16_t timer)
{
	if (timer > (24 * 60)) {
		return RS_ERROR;
	}
	if (timer % 60 && timer != 0) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_STATUS_TIMER, (u_int16_t)timer); 
}

CLIB_API RS cl_ia_waterheater_set_capactity(cl_handle_t dev_handle, u_int16_t capactity)
{
	if (capactity < 1 || capactity > 2) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_STATUS_CAPACTITY, (u_int16_t)capactity); 
}

/* 前锋热水器 */
CLIB_API RS cl_ia_waterheater_a9_set_fire_level(cl_handle_t dev_handle, u_int16_t level)
{
	//0：不燃烧
	//1：左燃烧
	//2：右燃烧
	//3：全燃烧
	if (level > 3) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_A9_STATUS_FIRE_LEVEL, (u_int16_t)level); 
}

CLIB_API RS cl_ia_waterheater_a9_clear_cnt(cl_handle_t dev_handle)
{
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_A9_STATUS_COUNT, 0); 
}

CLIB_API RS cl_ia_waterheater_a9_set_mode(cl_handle_t dev_handle, u_int16_t mode)
{
	// 1 自动 2 浴缸 3 洗碗 4 洗菜 5 洗衣
	if (mode == 0 || mode > 5) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_A9_STATUS_MODE, (u_int16_t)mode); 
}



CLIB_API RS cl_ia_waterheater_a9_set_temp(cl_handle_t dev_handle, u_int16_t temp)
{
	if (temp < 35 || temp > 65) {
		return RS_ERROR;//35-65
	}
	return _ia_aircleaner_set(dev_handle, IA_WATERHEATER_A9_STATUS_TEMP_SET, (u_int16_t)temp); 
}

/**************************************************************************************************
	  空调等相关信息 
 **************************************************************************************************/

CLIB_API RS cl_ia_aircondition_set(cl_handle_t dev_handle, bool onoff, u_int16_t mode, u_int16_t temp)
{
	ia_status_t  ia_stat_arr[3];

	
	if (onoff > 1) {
		return RS_ERROR;
	}

	if (mode > 4) {
		return RS_ERROR; //0-4
	}

	if (temp < 16 || temp > 32) {
		return RS_ERROR;//16-32 
	}

	CL_CHECK_INIT;

	if (check_ia_type(dev_handle, IA_TYPE(IA_AIRCONDITION_STATUS_ONOFF)) != RS_OK){
		return RS_ERROR;
	}

	ia_stat_arr[0].id = IA_AIRCONDITION_STATUS_ONOFF;
	ia_stat_arr[0].value = onoff;


	ia_stat_arr[1].id = IA_AIRCONDITION_STATUS_MODE;
	ia_stat_arr[1].value = mode;

	ia_stat_arr[2].id = IA_AIRCONDITION_STATUS_TEMP;
	ia_stat_arr[2].value = temp;

	return cl_ia_ctrl_v2(dev_handle, 3, ia_stat_arr); 
}


CLIB_API RS cl_ia_aircondition_set_timer(cl_handle_t dev_handle, u_int16_t timer)
{
	if (timer > (24 * 60)) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_AIRCONDITION_STATUS_TIMER, (u_int16_t)timer); 
}



/**************************************************************************************************
	  风扇等相关信息 
 **************************************************************************************************/

CLIB_API RS cl_ia_electricfan_set_work(cl_handle_t dev_handle, bool work)
{
	if (work > 1) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_ELECTRICFAN_STATUS_WORK, (u_int16_t)work); 
}

CLIB_API RS cl_ia_electricfan_set_gear(cl_handle_t dev_handle, u_int16_t gear)
{
	if (gear < 1 || gear > 4) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_ELECTRICFAN_STATUS_GEAR, (u_int16_t)gear); 
}

CLIB_API RS cl_ia_electricfan_set_timer(cl_handle_t dev_handle, u_int16_t timer)
{
	if (timer > (7 * 60 + 30)) { //7.5 小时
		return RS_ERROR;
	}
	if (timer % 30 && timer != 0) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_ELECTRICFAN_STATUS_TIMER, (u_int16_t)timer); 
}

CLIB_API RS cl_ia_electricfan_set_shake(cl_handle_t dev_handle, u_int16_t shake)
{
	if (shake > 1) {
		return RS_ERROR;
	}
	return _ia_aircleaner_set(dev_handle, IA_ELECTRICFAN_STATUS_SHAKE, (u_int16_t)shake); 
}

CLIB_API RS cl_ia_bath_heater_set_work(cl_handle_t dev_handle, u_int16_t work)
{
	work = !!work;
	return _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_ONOFF, (u_int16_t)work); 
}

CLIB_API RS cl_ia_bath_heater_set_anion(cl_handle_t dev_handle, u_int16_t anion)
{
	anion = !!anion;
	return _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_NEGATIVEIONS, (u_int16_t)anion); 
}

CLIB_API RS cl_ia_bath_heater_set_light(cl_handle_t dev_handle, u_int16_t light)
{
	light = !!light;
	return _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_LIGHT, (u_int16_t)light); 
}

CLIB_API RS cl_ia_bath_heater_set_breath(cl_handle_t dev_handle, u_int16_t breath)
{
	breath = !!breath;
	return  _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_AIR, (u_int16_t)breath);  
}

CLIB_API RS cl_ia_bath_heater_set_dry(cl_handle_t dev_handle, u_int16_t dry)
{
	dry = !!dry;
	return  _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_DRY, (u_int16_t)dry); 
}

CLIB_API RS cl_ia_bath_heater_set_tronic(cl_handle_t dev_handle, u_int16_t tronic)
{
	if(tronic > 2)
		return RS_INVALID_PARAM;
	return  _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_WARNM, (u_int16_t)tronic); 
}

CLIB_API RS cl_ia_bath_heater_set_timer(cl_handle_t dev_handle, u_int16_t timer)
{
	return  _ia_aircleaner_set(dev_handle, IA_BATHROOMMASTER_STATUS_TIMER, (u_int16_t)timer); 
}


