#include "cl_priv.h"
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"
#include "rfgw_rftt.h"
#include "rfgw_scm_ctrl.h"

/*
	功能:
		控制开关
	输入参数:
		@dev_handle: 网关的句柄
		@timeout: 网关进入配对模式的时间长度秒
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rfgw_join(cl_handle_t gw_handle, u_int16_t timeout)
{
	return cl_send_u16_notify(&cl_priv->thread_main, gw_handle, CLNE_RFGW_GATEWAY, ACT_RFGW_JOIN, timeout);
}

CLIB_API cl_rfgw_dev_find_t *
	cl_rfgw_get_join_dev(cl_handle_t gw_handle, u_int8_t *cnt)
{
	user_t *user;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* ac;
	rfgw_priv_t *p;
	sdk_dev_find_t *sdev, *next;	
	cl_rfgw_dev_find_t *cdev = NULL;
		
	if(!cnt)
		return NULL;
	
	cl_lock(&cl_priv->mutex);
	
	user = lookup_by_handle(HDLT_USER, gw_handle);
	if(user == NULL)
		goto done;

	//不是智能终端
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		goto done;
	}
	ac = (smart_air_ctrl_t*)sac->sub_ctrl;
	p = (rfgw_priv_t*)ac->com_udp_dev_info.device_info;
	
	stlc_list_for_each_entry(sdk_dev_find_t, sdev, &p->new_dev_head, link){
		*cnt += 1;
	}
	if(*cnt == 0)
		goto done;
	cdev = cl_calloc(sizeof(*cdev), *cnt);
	if(cdev == NULL)
		goto done;
	*cnt  = 0;
	stlc_list_for_each_entry_safe(sdk_dev_find_t, sdev, next, &p->new_dev_head, link){
		cdev[*cnt].sn = sdev->cl_dev.sn;
		cdev[*cnt].subtype = sdev->cl_dev.subtype;
		cdev[*cnt].extype = sdev->cl_dev.extype;
		*cnt += 1;
		stlc_list_del(&sdev->link);
		cl_free(sdev);
	}

done:
	cl_unlock(&cl_priv->mutex);
	return cdev;

}

CLIB_API void cl_rfgw_free_join_dev(cl_rfgw_dev_find_t *dev)
{
	if(dev)
		cl_free(dev);
}

CLIB_API RS cl_rfgw_join_action(cl_handle_t gw_handle, u_int64_t dev_sn, u_int16_t accept)
{
	sdk_join_act_t act;
	act.dev_sn = dev_sn;
	act.accept = !!accept;
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_JOIN_ACTION,
		(u_int8_t*)&act, sizeof(act));
		
}

CLIB_API RS cl_rfgw_group(cl_handle_t *gw_handle, u_int8_t gw_count, u_int8_t *psk)
{
	u_int8_t i;	
	sdk_group_t group;

	if(gw_handle == NULL || gw_count == 0 || psk == NULL)
		return RS_INVALID_PARAM;
	
	memcpy(group.psk, psk, PSK_LEN);
	group.gw_cnt = gw_count;
	for(i = 0; i < gw_count && i < MAX_GW_COUNT; i++){
		group.gw_handle[i] = gw_handle[i];
	}
	for(i = 0; i < gw_count && i < MAX_GW_COUNT; i++){
		cl_send_var_data_notify(&cl_priv->thread_main, gw_handle[i], 
		CLNE_RFGW_GATEWAY, ACT_RFGW_GROUP,
		(u_int8_t*)&group, sizeof(group));
	}
	return RS_OK;	
}

CLIB_API RS cl_rfgw_query_devlist(cl_handle_t gw_handle)
{
	return cl_send_u16_notify(&cl_priv->thread_main, gw_handle, CLNE_RFGW_GATEWAY, ACT_RFGW_DEVLIST, 0);
}

CLIB_API RS cl_rfgw_set_tt(cl_handle_t dev_handle, u_int8_t *data, u_int16_t len)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	u_int8_t buf[1024];	
	net_rfgw_tt_t *tt = (net_rfgw_tt_t*)buf;	
	
	if(dev_handle == INVALID_HANDLE || data == NULL || len == 0){
		return RS_INVALID_PARAM;		
	}
	
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if(slave && slave->user){
		tt->sn = slave->sn;
		gw_handle = slave->user->handle;
		if(data[0] == RFTT_WORK_REQ){
			if(slave->dev_info.rfdev->is_ctrl){
				slave->dev_info.rfdev->ctrl_fail++;
			}
			slave->dev_info.rfdev->is_ctrl = 1;
			slave->dev_info.rfdev->ctrl_msec = get_msec();
		}
	}
	cl_unlock(&cl_priv->mutex);

	if(gw_handle == INVALID_HANDLE)
		return RS_INVALID_PARAM;
	tt->len = len;
	memcpy(tt->data, data, len);
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_TT, buf, sizeof(*tt)+len);
}

CLIB_API u_int8_t *cl_rfgw_get_tt(cl_handle_t dev_handle, u_int16_t *len)
{
	return NULL;
}
CLIB_API void cl_rfgw_free_tt(u_int8_t *data)
{
	if(data)
		cl_free(data);
}

CLIB_API RS cl_rfdev_rgb(cl_handle_t handle, bool r, bool g, bool b)
{
	user_t *user;
	slave_t *slave;
	cl_handle_t *dev_handle = NULL;
	u_int8_t type, i;
	u_int8_t data[2];

	data[0] = RFTT_WORK_REQ;
	data[1] = 0;
	if(r)
		data[1] |= RGB_WORK_R;
	if(g)
		data[1] |= RGB_WORK_G;
	if(b)
		data[1] |= RGB_WORK_B;

	type = (handle >> 24) & 0xFF;
	if(type == HDLT_SLAVE){
		return cl_rfgw_set_tt(handle, data, sizeof(data));
	}

	cl_lock(&cl_priv->mutex);
	user = lookup_by_handle(HDLT_USER, handle);
	if(user){
		type = 0;
		stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
			if(slave->status == BMS_BIND_ONLINE)
				type++;
		}
		if(type > 0){
			dev_handle = cl_malloc(sizeof(cl_handle_t)*type);
			if(dev_handle){
				type = 0;
				stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
					if(slave->status == BMS_BIND_ONLINE)
						dev_handle[type++] = slave->handle;
				}
			}
		}
	}	
	cl_unlock(&cl_priv->mutex);
	if(dev_handle){
		for(i = 0; i < type; i++){
			cl_rfgw_set_tt(dev_handle[i], data, sizeof(data));
		}
		cl_free(dev_handle);
	}
	return RS_OK;	
}
CLIB_API RS cl_rfdev_rgb_batch(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt, u_int8_t rgb)
{
	u_int8_t i;
	batch_sn_handle_t batch;
	if(dev_handle == NULL || cnt < 1)
		return RS_INVALID_PARAM;
	if(cnt > ARRAY_SIZE(batch.handle))
		cnt = ARRAY_SIZE(batch.handle);
	
	batch.rgb = rgb;
	batch.cnt = cnt;
	for(i = 0; i < cnt; i++){
		batch.handle[i] = dev_handle[i];
	}
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_TT_BATCH, (u_int8_t*)&batch, sizeof(batch));
}
CLIB_API RS cl_rfdev_work_query(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt)
{
	u_int8_t i;
	batch_sn_handle_t batch;
	if(dev_handle == NULL || cnt < 1)
		return RS_INVALID_PARAM;
    
	if(cnt > ARRAY_SIZE(batch.handle))
		cnt = ARRAY_SIZE(batch.handle);
	
	batch.cnt = cnt;
	for(i = 0; i < cnt; i++){
		batch.handle[i] = dev_handle[i];
	}
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_WORK_QUERY, (u_int8_t*)&batch, sizeof(batch));
}

CLIB_API RS cl_rfgw_dev_delete(cl_handle_t gw_handle, cl_handle_t *dev_handle, u_int8_t cnt)
{
	u_int8_t i;
	batch_sn_handle_t batch;
	if(dev_handle == NULL || cnt < 1)
		return RS_INVALID_PARAM;
	if(cnt > ARRAY_SIZE(batch.handle))
		cnt = ARRAY_SIZE(batch.handle);
	
	batch.cnt = cnt;
	for(i = 0; i < cnt; i++){
		batch.handle[i] = dev_handle[i];
	}
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_DEL, (u_int8_t*)&batch, sizeof(batch));
}

CLIB_API RS cl_rfgw_dev_delete_all(cl_handle_t gw_handle)
{
	return cl_send_u16_notify(&cl_priv->thread_main, gw_handle, CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_DEL_ALL, 0);
}

CLIB_API RS cl_dev_name_set(cl_handle_t dev_handle, u_int8_t *name)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	net_dev_name_t t;
	int len;
	
	if(dev_handle == INVALID_HANDLE || name == NULL ){
		return RS_INVALID_PARAM;
	}
	len = (int)strlen((char*)name);
	if(len >= sizeof(t.name))
		return RS_INVALID_PARAM;
		
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if(slave && slave->user){
		t.sn = slave->sn;
		gw_handle = slave->user->handle;
	}
	cl_unlock(&cl_priv->mutex);

	if(gw_handle == INVALID_HANDLE)
		return RS_INVALID_PARAM;
	
	memcpy(t.name, name, len);
	t.name[len] = 0;
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_NAME, (u_int8_t*)&t, sizeof(t));
}

CLIB_API RS cl_rfdef_comm_timer_modify_add(cl_handle_t dev_handle, cl_comm_timer_t *ptimer)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	net_dev_timer_t t;
	int len;
	
	if(dev_handle == INVALID_HANDLE || ptimer == NULL ){
		return RS_INVALID_PARAM;
	}
	len = sizeof(*ptimer);
	if(len > sizeof(t.timer)) {
		return RS_INVALID_PARAM;
	}
		
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if(slave && slave->user){
		t.sn = slave->sn;
		gw_handle = slave->user->handle;
	}
	cl_unlock(&cl_priv->mutex);

	if(gw_handle == INVALID_HANDLE) {
		return RS_INVALID_PARAM;
	}
	
	memcpy((void *)&t.timer, (void *)ptimer, len);
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_TIMER_ADD, (u_int8_t*)&t, sizeof(t));	
}

CLIB_API RS cl_rfdev_comm_timer_del(cl_handle_t dev_handle, u_int8_t id)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	net_dev_timer_t t;
	cl_comm_timer_t timer;
	int len;

	memset((void *)&timer, 0, sizeof(timer));
	timer.id = id;
	if(dev_handle == INVALID_HANDLE){
		return RS_INVALID_PARAM;
	}
	len = sizeof(timer);
	if(len > sizeof(t.timer))
		return RS_INVALID_PARAM;
		
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if(slave && slave->user){
		t.sn = slave->sn;
		gw_handle = slave->user->handle;
	}
	cl_unlock(&cl_priv->mutex);

	if(gw_handle == INVALID_HANDLE)
		return RS_INVALID_PARAM;
	
	memcpy((void *)&t.timer, (void *)&timer, len);
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_TIMER_DEL, (u_int8_t*)&t, sizeof(t));	
}

CLIB_API RS cl_rfdev_comm_timer_query(cl_handle_t dev_handle)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	net_dev_timer_t t;

	if(dev_handle == INVALID_HANDLE){
		return RS_INVALID_PARAM;
	}
		
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if(slave && slave->user){
		t.sn = slave->sn;
		gw_handle = slave->user->handle;
	}
	cl_unlock(&cl_priv->mutex);

	if(gw_handle == INVALID_HANDLE) {
		return RS_INVALID_PARAM;
	}

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_TIMER_QUERY, (u_int8_t*)&t, sizeof(t));		
}

CLIB_API RS cl_rfdef_comm_history_query(cl_handle_t dev_handle, u_int32_t index)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	net_dev_comm_history_query_t t;
	
	if(dev_handle == INVALID_HANDLE){
		return RS_INVALID_PARAM;
	}

		
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if (slave && slave->user){
		t.sn = slave->sn;
		t.index = index;
		gw_handle = slave->user->handle;
	}
	cl_unlock(&cl_priv->mutex);

	if (gw_handle == INVALID_HANDLE) {
		return RS_INVALID_PARAM;
	}
	
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_COMM_HISTORY_QUERY, (u_int8_t*)&t, sizeof(t));	
}


CLIB_API RS cl_rfdev_debug_info_query(cl_handle_t dev_handle, cl_rf_dev_debug_info_t *ver)
{
	slave_t *slave;
	RS ret = RS_ERROR;

	if(dev_handle == INVALID_HANDLE){
		return RS_INVALID_PARAM;
	}
		
	cl_lock(&cl_priv->mutex);
	
	slave = lookup_by_handle(HDLT_SLAVE, dev_handle);
	if(slave){
		ret = RS_OK;
		ver->ver.major = slave->dev_info.version.soft_version.major;
		ver->ver.minor = slave->dev_info.version.soft_version.minor;
		ver->ver.revise = slave->dev_info.version.soft_version.revise;		

		ver->upver.major = slave->dev_info.version.upgrade_version.major;
		ver->upver.minor = slave->dev_info.version.upgrade_version.minor;
		ver->upver.revise = slave->dev_info.version.upgrade_version.revise;		
	}
	
	cl_unlock(&cl_priv->mutex);

	return ret;
}

CLIB_API RS cl_rfdev_upgrade(cl_handle_t gw_handle, u_int32_t upgrade_type, u_int8_t *filepath)
{
	rfgw_upgrade_t upgrade;

	upgrade.type = upgrade_type;
	strncpy(upgrade.filepath, filepath, sizeof(upgrade.filepath));
	
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_UPGRADE, (u_int8_t*)&upgrade, sizeof(upgrade));
}

CLIB_API RS cl_dev_commpat(cl_handle_t gw_handle, u_int8_t commpat, u_int8_t channel)
{
	u_int8_t buf[10] = {commpat, channel};
	
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_COMMPAT, buf, 2);
}

CLIB_API RS cl_dev_up_query(cl_handle_t gw_handle)
{
	u_int8_t data = 0;
	
	return cl_send_u8_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_UP_QUERY, data);	
}

CLIB_API RS cl_dev_group_query(cl_handle_t gw_handle, u_int8_t group_id)
{

	cl_dev_group_t group;

	group.group_id = group_id;
	group.reserved = ACTION_QUERY;
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_GROUP_MEMBER, (u_int8_t*)&group, sizeof(group));
}

CLIB_API RS cl_dev_group_set(cl_handle_t gw_handle, cl_dev_group_t *group)
{
	group->reserved = ACTION_ADD;
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_GROUP, (u_int8_t*)group, sizeof(*group));
}


CLIB_API RS cl_dev_group_delete(cl_handle_t gw_handle, u_int8_t group_id)
{
	cl_dev_group_t group;

	group.group_id = group_id;
	group.reserved = ACTION_DEL;
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_GROUP, (u_int8_t*)&group, sizeof(group));

}

#if 0
static u_int32_t _trans_group_data(u_int32_t flag,u_int8_t* data,int len,u_int8_t* buf)
{
    ucp_rf_led_lamp_t* dest = (ucp_rf_led_lamp_t*)buf;
    cl_rf_lamp_stat_t* src = (cl_rf_lamp_stat_t*)data;
    
    if (flag == GDT_LAMP_COLOR_CTRL) {
        dest->o_wc_l = src->action;
        dest->o_r = src->R;
        dest->o_g = src->G;
        dest->o_b = src->B;
        dest->o_l = src->L;
        dest->o_c = src->cold;
		
        dest->hwconf = src->ctrl_mode;
        
        dest->R = (u_int8_t)(((int)src->R*src->L)/100.0);
        dest->G = (u_int8_t)(((int)src->G*src->L)/100.0);
        dest->B = (u_int8_t)(((int)src->B*src->L)/100.0);
        
        if (dest->o_c < 50) {
            dest->W = (u_int8_t)(255 * (int)dest->o_wc_l / 100.0);
            dest->C= (u_int8_t)((int)dest->W * (int)dest->o_c / 50.0);
        } else {
            dest->C= (u_int8_t)(255 * (int)dest->o_wc_l / 100.0);
            dest->W = (u_int8_t)((int)dest->C * (100 - (int)dest->o_c) / 50.0);
        }
        dest->mod_id = src->mod_id;
        dest->power = src->power;
        dest->sub_cmd = SC_SET;
        dest->type = 0x0;
        
        len = sizeof(*dest);
    }else{
        memcpy(buf, data, len);
    }
    return len;
}

#endif

CLIB_API RS cl_rf_dev_group_ctrl(cl_handle_t gw_handle, u_int8_t group_id,u_int32_t flag,u_int8_t* data,int len)
{
	u_int8_t buf[1024] = {0};
    net_rfgw_group_tt_t* gt = (net_rfgw_group_tt_t*)buf;
    ucp_rf_led_lamp_t* dest = (ucp_rf_led_lamp_t*)(gt+1);
    cl_rf_lamp_stat_t* src = (cl_rf_lamp_stat_t*)data;
    
    if (!data || !group_id) {
        return RS_INVALID_PARAM;
    }
    
    dest->o_wc_l = src->action;
    dest->o_r = src->R;
    dest->o_g = src->G;
    dest->o_b = src->B;
    dest->o_l = src->L;
    dest->o_c = src->cold;
    
    
    dest->R = (u_int8_t)(((int)src->R*src->L)/100.0);
    dest->G = (u_int8_t)(((int)src->G*src->L)/100.0);
    dest->B = (u_int8_t)(((int)src->B*src->L)/100.0);
    
    if (dest->o_c < 50) {
        dest->W = (u_int8_t)(255 * (int)dest->o_wc_l / 100.0);
        dest->C= (u_int8_t)((int)dest->W * (int)dest->o_c / 50.0);
    } else {
        dest->C= (u_int8_t)(255 * (int)dest->o_wc_l / 100.0);
        dest->W = (u_int8_t)((int)dest->C * (100 - (int)dest->o_c) / 50.0);
    }

    dest->hwconf = src->ctrl_mode;

	if ( dest->hwconf == LED_UI_WC_DOUBLE || dest->hwconf == LED_UI_WC_LOWER) {
		dest->o_r = src->R;
		if (dest->o_b < 50) {
            dest->B = (u_int8_t)(255 * (int)dest->o_l / 100.0);
            dest->G = (u_int8_t)((int)dest->B * (int)dest->o_b / 50.0);
        } else {
            dest->G= (u_int8_t)(255 * (int)dest->o_l / 100.0);
            dest->B = (u_int8_t)((int)dest->G * (100 - (int)dest->o_b) / 50.0);
        }

		if (dest->G == 2)
			dest->G = 1;

		if (dest->B == 2)
			dest->B = 1;
	}
    
    dest->mod_id = src->mod_id;
    dest->power = src->power;
    dest->sub_cmd = SC_SET;
    //dest->type = 0x0;
	dest->type = src->flag;
	dest->r_id = 0;
    
    gt->len = sizeof(*dest);
    gt->group_id = group_id;
    gt->sub_type = IJ_RFGW;
    gt->ext_type = RF_EXT_TYPE_LED_LAMP;
    
    return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle,
                                   CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_GROUP_TT, buf, sizeof(*gt) + gt->len);
}


///////////////////////////////////////////
//RFdeng
CLIB_API RS cl_rf_ctrl_auto_guard(cl_handle_t slave_handle,cl_rf_auto_guard_info_t* stat)
{
    CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle,
                                   CLNE_RF_COM_DEV, ACT_RF_COM_AUTO_GUARD, (u_int8_t*)stat, sizeof(*stat));
}

CLIB_API RS cl_rf_lamp_ctrl_stat(cl_handle_t dev_handle,cl_rf_lamp_stat_t* stat)
{
    
    CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
                                   CLNE_RF_LAMP, ACT_RF_LAMP_SET_COLOR, (u_int8_t*)stat, sizeof(*stat));
}

CLIB_API RS cl_rf_lamp_remote_ctrl_stat(cl_handle_t gw_handle,u_int32_t remote_id,u_int8_t key_id,cl_rf_lamp_stat_t* stat)
{
    u_int8_t buf[1024] = {0};
    net_rfgw_group_tt_t* gt = (net_rfgw_group_tt_t*)buf;
    ucp_rf_led_lamp_t* dest = (ucp_rf_led_lamp_t*)(gt+1);
    cl_rf_lamp_stat_t* src = (cl_rf_lamp_stat_t*)stat;
    
    if (!stat || !remote_id) {
        return RS_INVALID_PARAM;
    }
    
    dest->o_wc_l = src->action;
    dest->o_r = src->R;
    dest->o_g = src->G;
    dest->o_b = src->B;
    dest->o_l = src->L;
    dest->o_c = src->cold;
    
    
    dest->R = (u_int8_t)(((int)src->R*src->L)/100.0);
    dest->G = (u_int8_t)(((int)src->G*src->L)/100.0);
    dest->B = (u_int8_t)(((int)src->B*src->L)/100.0);
    
    if (dest->o_c < 50) {
        dest->W = (u_int8_t)(255 * (int)dest->o_wc_l / 100.0);
        dest->C= (u_int8_t)((int)dest->W * (int)dest->o_c / 50.0);
    } else {
        dest->C= (u_int8_t)(255 * (int)dest->o_wc_l / 100.0);
        dest->W = (u_int8_t)((int)dest->C * (100 - (int)dest->o_c) / 50.0);
    }

    dest->hwconf = src->ctrl_mode;

	if ( dest->hwconf == LED_UI_WC_DOUBLE || dest->hwconf == LED_UI_WC_LOWER) {
		dest->o_r = src->R;
		if (dest->o_b < 50) {
            dest->B = (u_int8_t)(255 * (int)dest->o_l / 100.0);
            dest->G = (u_int8_t)((int)dest->B * (int)dest->o_b / 50.0);
        } else {
            dest->G= (u_int8_t)(255 * (int)dest->o_l / 100.0);
            dest->B = (u_int8_t)((int)dest->G * (100 - (int)dest->o_b) / 50.0);
        }

		if (dest->G == 2)
			dest->G = 1;

		if (dest->B == 2)
			dest->B = 1;
	}
    
    dest->mod_id = src->mod_id;
    dest->power = src->power;
    dest->sub_cmd = SC_SET;
    dest->type = src->flag;
    dest->r_id = (remote_id << 3)|key_id;
    dest->r_id = htonl(dest->r_id);
    
    gt->len = sizeof(*dest);
    gt->group_id = 0;
    gt->sub_type = IJ_RFGW;
    gt->ext_type = RF_EXT_TYPE_LED_LAMP;
    
    log_debug("send remote light ctrl R=[%u] G=[%u] B=[%u] L=[%u] cold=[%u] group_id =[%u] remote_id = [%u] key_id=[%u]",
             dest->o_r,dest->o_g,dest->o_b,dest->o_l,dest->o_c,gt->group_id,remote_id,key_id);
    
    return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle,
                                   CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_GROUP_TT, buf, sizeof(*gt)+sizeof(*dest));
}

CLIB_API RS cl_rf_door_lock_ctrl(cl_handle_t dev_handle,u_int8_t group_id,u_int8_t type,u_int8_t action)
{
    u_int32_t value;

    CL_CHECK_INIT;
    
    value = BUILD_U32(group_id, type, 0, action);
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_RF_DOOR_LOCK_CTRL, value);
}

CLIB_API RS cl_yt_rf_door_lock_ctrl_lock(cl_handle_t dev_handle,u_int8_t group_id,u_int8_t on_off, u_int32_t passwd)
{
    u_int64_t value;
    u_int16_t t_v;
    
    CL_CHECK_INIT;
    
    t_v = BUILD_U16(group_id, on_off);
    value = BUILD_U64_FROM_U32(t_v,passwd);
    
    return cl_send_u64_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_YT_RF_DOOR_LOCK_CTRL, value);
}

CLIB_API RS cl_yt_rf_door_lock_modify_lock_passwd(cl_handle_t dev_handle,u_int32_t old_passwd, u_int32_t new_passwd)
{
    u_int64_t value;
    
    CL_CHECK_INIT;
    
    value = BUILD_U64_FROM_U32(old_passwd,new_passwd);
    
    return cl_send_u64_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_YT_RF_DOOR_LOCK_MODIFY_PASSWD, value);
}

CLIB_API RS cl_yt_rf_door_lock_create_lock_passwd(cl_handle_t dev_handle, u_int32_t new_passwd)
{
    u_int64_t value;
    
    CL_CHECK_INIT;
    
    value = BUILD_U64_FROM_U32(0, new_passwd);
    
    return cl_send_u64_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_YT_RF_DOOR_LOCK_CREATE_PASSWD, value);
}


/*
 功能:
	 友泰设置门锁关联
 输入参数:
	 @dev_handle: 门锁的句柄
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_yt_rf_door_lock_set_associate(cl_handle_t dev_handle)
{
    CL_CHECK_INIT;
    
    return cl_send_u8_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_YT_RF_DOOR_LOCK_ASSOCIATE, 0);
}

/*
 功能:
	 设置门多久没关就报警
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @enable: 是否开启报警
 	 @timeout: 超时时间，单位分钟
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_door_lock_set_unlock_timeout(cl_handle_t dev_handle, u_int8_t enable, u_int8_t timeout)
{
    u_int16_t value;
    
    CL_CHECK_INIT;
    
    value = BUILD_U16(enable, timeout);
    
    return cl_send_u16_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_RF_DOOR_LOCK_UNLOCK_TIMEOUT, value);
}

/*
 功能:
	 设置wifi接入或者断开以后自动解锁或者上锁
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @type: 1 设置自动解锁 2 设置自动上锁
 	 @enable:  使能
 	 @starthour: 开始的时间
 	 @endhour: 结束的时间
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_door_lock_set_wifilock(cl_handle_t dev_handle, u_int8_t type, u_int8_t enable, u_int8_t starthour, u_int8_t endhour)
{
    u_int32_t value;
    
    CL_CHECK_INIT;

	if (type == 0 || type > 2) {
		return RS_INVALID_PARAM;
	}
    
    value = BUILD_U32(type, enable, starthour, endhour);
    
    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_RF_DOOR_LOCK_WIFI_LOCK, value);
}

/*
 功能:
	 设置门锁遥控器
 输入参数:
	 @dev_handle: 门锁的句柄
 	 @request: 设置参数，具体看cl_door_lock_controller_set_t 解释
 输出参数:
	 无
 返回:
	 RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rf_door_lock_set_controller_info(cl_handle_t dev_handle, cl_door_lock_controller_set_t *request)
{
    int len;
    
    CL_CHECK_INIT;

	if (request->id == 0 || request->id > 5) {
		return RS_INVALID_PARAM;
	}

	len = sizeof(*request);
	if (request->name[0] == 0) {
		len -= sizeof(request->name);
	}
    
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_RF_DOOR_LOCK, ACT_RF_DOOR_LOCK_SET_CONTROLLER_INFO, (u_int8_t*)request, len);
}


//CLIB_API RS cl_rf_door_lock_rc_ctrl(cl_handle_t dev_handle,cl_rf_door_lock_remote_t* lr)
//{
//    CL_CHECK_INIT;
//
//    if (!lr) {
//        return RS_INVALID_PARAM;
//    }
//
//    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
//                                   CLNE_RF_DOOR_LOCK, ACT_RF_DOOR_LOCK_REMOTE_CTRL, (u_int8_t*)lr, sizeof(*lr));
//}

CLIB_API RS cl_rf_dev_query_history(cl_handle_t slave_handle,u_int32_t last_time)
{
    u_int32_t value;
    
    CL_CHECK_INIT;

    value = last_time;
    
    return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_QUERY_HISTORY, value);
}

CLIB_API RS cl_rf_dev_com_ctrl(cl_handle_t slave_handle,u_int8_t group_id,u_int8_t group_type,u_int8_t type,u_int8_t value)
{
    u_int32_t t_value;
    
    CL_CHECK_INIT;
    
    t_value = BUILD_U32(group_id, type, group_type, value);
    
    return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_DIRECT_CTRL, t_value);
}

/*
	功能: 设置报警时间，比如啥时候再开始啥的
		
	输入参数:
		@slave_handle: 设备的句柄
		@type: 1 : 设置报警开始时间0 设置报警结束时间
		@time: 到达对应type状态间隔时间，单位秒
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_dev_set_alarm_time(cl_handle_t slave_handle, u_int8_t type, u_int32_t t)
{
    ucp_rf_alarm_time_t value;
    
    CL_CHECK_INIT;

    value.ctrl = type & 1;

	if (t == 0) {
		value.time = 0;
	} else {
		value.time = (u_int32_t)time(NULL) + t;
	}
    
    return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_ALARM_TIME, (u_int8_t*)&value, sizeof(value));
}

/*
	功能: 对于有些一旦报警后就会一直响应的设备，
	需要发送一个报警取消命令
		
	输入参数:
		@slave_handle: 设备的句柄

	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_dev_set_alarm_clc(cl_handle_t slave_handle)
{
    u_int8_t value = 0;
    
    CL_CHECK_INIT;

    return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_ALARM_CLC, value);
}


CLIB_API RS cl_rf_dev_air_ir_ctrl(cl_handle_t slave_handle,u_int8_t type,u_int8_t value)
{
    u_int32_t t_value;
    
    CL_CHECK_INIT;
    
    t_value = BUILD_U32(0, 0, type, value);
    
    return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_AIR_IR_CTRL, t_value);
}


/*
	功能: 暖气阀的简单控制接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@action: 控制类型(ACT_HEATING_VALVE_T)
			ACT_HEATING_VALVE_ONOFF 控制开关机0 关机 1开机
			ACT_HEATING_VALVE_MODE 控制模式0 自动 1手动
			ACT_HEATING_VALVE_ANTI_LIME  防石灰 0 取消 1使能
			ACT_HEATING_VALVE_FROST_PROTECTION 霜冻保护 0 取消 1使能
			ACT_HEATING_VALVE_CHILD_LOCK 童锁 0 取消 1使能
			ACT_HEATING_VALVE_WINDOW 0 取消 1使能
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_simple_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int8_t value)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, action, value);
}

/*
	功能: 暖气阀的日期控制接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@date: 日期设置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_date_ctrl(cl_handle_t slave_handle, cl_heating_valve_param_date_t *date)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HEATING_VALVE_DATE, (u_int8_t*)date, sizeof(*date));
}

/*
	功能: 暖气阀的温度设置接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@date: 日期设置
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_temp_ctrl(cl_handle_t slave_handle, cl_heating_valve_param_temp_t *temp)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HEATING_VALVE_TEMP, (u_int8_t*)temp, sizeof(*temp));
}

/*
	功能: 设置某天的加热和经济时间点
		
	输入参数:
		@slave_handle: 设备的句柄
		@day: 	 星期几(取值1-7)
		@hh1;	 第一加热周期 小时 （取值范围：0 - 24）
		@hm1;	 第一加热周期 分钟 （取值范围：0 - 50，粒度10）
		@eh1;	 第一经济周期 小时（取值范围：0 - 24）
		@em1;	 第一经济周期 分钟（取值范围：0 - 50，粒度10）
		@hh2;	 第二加热周期 小时（取值范围：0 - 24）
		@hm2;	 第二加热周期 分钟（取值范围：0 - 50，粒度10）
		@eh2;	 第二经济周期 小时（取值范围：0 - 24）
		@em2;	 第二经济周期 分钟（取值范围：0 - 50，粒度10）
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_heating_valve_period_ctrl(cl_handle_t slave_handle, u_int8_t day, 
	u_int8_t hh1, u_int8_t hm1, 
	u_int8_t eh1, u_int8_t em1, 
	u_int8_t hh2, u_int8_t hm2, 
	u_int8_t eh2, u_int8_t em2)
{
	cl_heating_valve_param_period_t request;

	CL_CHECK_INIT;

	request.day = day;

	request.hh1 = hh1;
	request.hm1 = hm1;

	request.eh1 = eh1;
	request.em1 = em1;

	request.hh2 = hh2;
	request.hm2 = hm2;
	
	request.eh2 = eh2;
	request.em2 = em2;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HEATING_VALVE_PERIOD, (u_int8_t*)&request, sizeof(request));
}



////////////////////////////////////////////////////////////////////////////////////
// 凯特插座
/*
	功能:  凯特插座的简单控制接口
		
	输入参数:
		@slave_handle: 设备的句柄
		@action: 控制类型(ACT_KTCZ__T)
			ACT_KTCZ_ONOFF 控制通断电0 断电 1通电
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_ktcz_simple_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int8_t value)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, action, value);
}


/*
	功能:  一键布放撤防接口
		
	输入参数:
		@master_handle:网关的句柄
		@is_defense: 1 布放 0 撤防
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
#if 1
CLIB_API RS cl_rf_dev_set_defense_batch(cl_handle_t master_handle, u_int8_t is_defense)
{
	user_t *user;
	slave_t *slave;
	char buf[2048] = {0};
	u_int16_t pos = 0,n = 0;
    u_int16_t max_pkt_len = 1200;
	u_int8_t param_len = 0;
    ucp_obj_t* uo ;
    net_rfgw_tt_t* nf;
    rf_tlv_t* tlv;
	ucp_rf_com_detector_ctrl_t *dc;
	RS ret = RS_OK;
	
	CL_CHECK_INIT;

	cl_lock(&cl_priv->mutex);

	user = lookup_by_handle(HDLT_USER, master_handle);
    if (!user) {
        ret = RS_INVALID_PARAM;
		goto done;
    }

	//这里，一键部防撤防有可能是网关外设也需要，如夜狼网关
	rf_scm_dev_set_defense_batch(user, is_defense);
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->sub_type != IJ_RFGW)
			continue;

		if (user->is_support_dbc && !slave->is_support_dbc) {
			continue;
		}

		uo = (ucp_obj_t*)&buf[pos];
		nf = (net_rfgw_tt_t*)(uo+1);
		tlv = (rf_tlv_t*)(nf+1);
		switch (slave->ext_type) {
			case RF_EXT_TYPE_DOOR_LOCK:
				slave->dev_info.rf_stat.dev_priv_data.door_lock_info.stat.is_guard = is_defense;
				break;
			case RF_EXT_TYPE_DOOR_MAGNET:
			case RF_EXT_TYPE_DOOR_MAGNETV2:
			case RF_EXT_TYPE_HM_MAGENT:
			case RF_EXT_TYPE_YLLOCK:
				slave->dev_info.rf_stat.dev_priv_data.door_magnet_info.stat.is_guard = is_defense;
				break;
			case RF_EXT_TYPE_HM_BODY_DETECT:
			case RF_EXT_TYPE_YLTC:
			case RF_EXT_TYPE_WUANS6:
				slave->dev_info.rf_stat.dev_priv_data.hb_info.is_guard = is_defense;
				break;
			case RF_EXT_TYPE_GAS:
			case RF_EXT_TYPE_QSJC:
			case RF_EXT_TYPE_HMCO:
			case RF_EXT_TYPE_HMYW:
			case RF_EXT_TYPE_HMQJ:
			case RF_EXT_TYPE_YLSOS:
				slave->dev_info.rf_stat.dev_priv_data.cd_info.stat.is_defence = is_defense;
				break;
			default:
				//认不到的设备也布防撤防一下
				break;
		}

        param_len = sizeof(*tlv) + sizeof(u_int32_t) + sizeof(ucp_rf_com_detector_ctrl_t);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*nf)+param_len);
		nf->sn = ntoh_ll(slave->sn);
		nf->len = ntohs(param_len);
		pos += (sizeof(*uo)+sizeof(*nf)+param_len);
		n++;
		// 发送布放撤防命令
		tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
        tlv->len = sizeof(u_int32_t);
		dc = (ucp_rf_com_detector_ctrl_t *)rf_tlv_val(tlv);
		dc->value = !!is_defense;
		dc->pad = 0xff;
        //一个数据包完成
        if (pos >= max_pkt_len) {
			sa_ctrl_obj_value(user->uc_session,UCA_SET,false, (u_int8_t)n, buf, pos);
            pos = 0;
            n = 0;
        }
	}
	
    // 不够一个数据包的
    if (n>0 && pos >0) {
		sa_ctrl_obj_value(user->uc_session,UCA_SET,false, (u_int8_t)n, buf, pos);
    }
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}
#else
CLIB_API RS cl_rf_dev_set_defense_batch(cl_handle_t master_handle, u_int8_t is_defense)
{
	user_t *user;
	slave_t *slave;
	int len = 0;
	char buf[256] = {0};
	rf_tlv_t* tlv = (rf_tlv_t*)buf;
	ucp_rf_com_detector_ctrl_t *dc;
	RS ret = RS_OK;
	bool need_send = false;
	
	CL_CHECK_INIT;

	cl_lock(&cl_priv->mutex);

	user = lookup_by_handle(HDLT_USER, master_handle);
    if (!user) {
        ret = RS_INVALID_PARAM;
		goto done;
    }

	//这里，一键部防撤防有可能是网关外设也需要，如夜狼网关
	rf_scm_dev_set_defense_batch(user, is_defense);
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->sub_type != IJ_RFGW)
			continue;

		if (user->is_support_dbc && !slave->is_support_dbc) {
			continue;
		}

		need_send = false;
		switch (slave->ext_type) {
			case RF_EXT_TYPE_DOOR_LOCK:
				slave->dev_info.rf_stat.dev_priv_data.door_lock_info.stat.is_guard = is_defense;
				need_send = true;
				break;
			case RF_EXT_TYPE_DOOR_MAGNET:
			case RF_EXT_TYPE_DOOR_MAGNETV2:
			case RF_EXT_TYPE_HM_MAGENT:
				slave->dev_info.rf_stat.dev_priv_data.door_magnet_info.stat.is_guard = is_defense;
				need_send = true;
				break;
			case RF_EXT_TYPE_HM_BODY_DETECT:
				slave->dev_info.rf_stat.dev_priv_data.hb_info.is_guard = is_defense;
				need_send = true;
				break;
			case RF_EXT_TYPE_GAS:
			case RF_EXT_TYPE_QSJC:
			case RF_EXT_TYPE_HMCO:
			case RF_EXT_TYPE_HMYW:
			case RF_EXT_TYPE_HMQJ:
				slave->dev_info.rf_stat.dev_priv_data.cd_info.stat.is_defence = is_defense;
				need_send = true;
				break;
		}

		if (need_send) {
			// 发送布放撤防命令
			tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
	        tlv->len = sizeof(u_int32_t);
	        len = sizeof(*tlv) + sizeof(u_int32_t);

			dc = (ucp_rf_com_detector_ctrl_t *)rf_tlv_val(tlv);
			dc->value = !!is_defense;
			dc->pad = 0xff;

			rfgw_send_tt_packet(slave, buf, len);
		}
	}

done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

#endif
CLIB_API RS cl_rf_dev_config_defense_batch(cl_handle_t master_handle, u_int8_t num, u_int64_t *sn)
{
	int len = 0;
	u_int8_t buf[1024*3];
	ucp_rfgw_dbc_hd_t *phd = (ucp_rfgw_dbc_hd_t *)buf;

	CL_CHECK_INIT;
	
	if (!sn) {
		return RS_ERROR;
	}

	len = num*sizeof(*sn);
	if ((len + sizeof(*phd)) > sizeof(buf)) {
		return RS_ERROR;
	}
	phd->num = num;
	
	memcpy((void *)phd->sn, (void *)sn, len);
	len += sizeof(*phd);
	
	return cl_send_var_data_notify(&cl_priv->thread_main, master_handle, CLNE_RFGW_GATEWAY, 
		ACT_RFGW_DEV_DEFENSE_BATCH_CONFIG, buf, len);
}

////////////////////////////////////////////////////////////////////////////////////
// 汇泰龙锁
/*
	功能:  管理用户登陆
	需要根据随后的事件判断是否成功
	SAE_HTLLOCK_ADMIN_LOGIN_OK = 1218,		// 汇泰龙登陆成功
	SAE_HTLLOCK_ADMIN_LOGIN_FAILED = 1219, // 汇泰龙登陆失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_admin_login(cl_handle_t slave_handle, htllock_tt_admin_login_t *request)
{
	int i;
	
	CL_CHECK_INIT;

	// 上层传ASCII码的数，SDK转为
	for (i = 0; i < 6; i++) {
		if (request->pwd[i] < 0x30 || request->pwd[i] > 0x39) {
			return RS_INVALID_PARAM;
		}

		request->pwd[i] -= 0x30;
	}

	request->id = 0x2000;
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_ADMIN_LOGIN, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  设置临时PIN码
	需要根据随后的事件判断是否成功
	SAE_HTLLOCK_SET_PIN_OK = 1237,	// 汇泰龙设置PIN码成功
	SAE_HTLLOCK_SET_PIN_FAILED = 1238,	// 汇泰龙设置PIN码失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_set_pin(cl_handle_t slave_handle, htllock_tt_set_pin_t *request)
{
//	int i;
	
	CL_CHECK_INIT;

	if (request->pwd_len != 6) {
		return RS_INVALID_PARAM;
	}

	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_SET_PIN, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  用户设置名字
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_NAME_OK = 1220,	// 汇泰龙设置用户名字成功
	SAE_HTLLOCK_SET_NAME_FAILED = 1221,// 汇泰龙设置用户名字失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_name(cl_handle_t slave_handle, htllock_tt_user_manage_set_name_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_USER_MANAGE_SET_NAME, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  用户设置头像
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_PIC_OK = 1222,// 汇泰龙设置用户头像成功
	SAE_HTLLOCK_SET_PIC_FAILED = 1223,// 汇泰龙设置失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_pic(cl_handle_t slave_handle, htllock_tt_user_manage_set_pic_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_USER_MANAGE_SET_PIC, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  用户设置关联
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_BIND_OK = 1224,// 汇泰龙关联用户的指纹、密码、扫描卡成功
	SAE_HTLLOCK_SET_BIND_FAILED = 1225, // 汇泰龙关联用户的指纹、密码、扫描卡失败
		
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_bind(cl_handle_t slave_handle, htllock_tt_user_manage_set_bind_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_USER_MANAGE_SET_BIND, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  用户设置关联取消
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_UNBIND_OK = 1226,// 汇泰龙设置取消关联成功
	SAE_HTLLOCK_SET_UNBIND_FAILED = 1227,// 汇泰龙设置取消关联失败
	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_unbind(cl_handle_t slave_handle, htllock_tt_user_manage_set_bind_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_USER_MANAGE_SET_UNBIND, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  修改用户开关门提醒通知
	输入参数:
		@slave_handle: 设备的句柄
		@is_close: 是否关闭提醒
		@user_index:用户编号
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_user_manage_set_remind_onoff(cl_handle_t slave_handle, u_int8_t isclose, u_int16_t user_index)
{
	htllock_tt_user_manage_set_remind_onoff_t request;
	
	CL_CHECK_INIT;

	request.op = HTLLOCK_USER_MANAGE_OPT_TYPE_SET_REMIND_ONOFF;
	request.isclose = isclose;
	request.index = user_index;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_USER_MANAGE_SET_REMIND_ONOFF, (u_int8_t*)&request, sizeof(request));
}


/*
	功能:  用户设置消息
	需要根据随后的事件来判断是否成功
	SAE_HTLLOCK_SET_INFO_NOTICE_OK = 1228, // 汇泰龙设置信息提醒成功
	SAE_HTLLOCK_SET_INFO_NOTICE_FAILED = 1229, // 汇泰龙设置信息提醒失败

	输入参数:
		@slave_handle: 设备的句柄
		@request: 控制参数,op不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_notice_info_set(cl_handle_t slave_handle, htllock_tt_info_notice_set_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_SET_INTICE_INFO, (u_int8_t*)request, sizeof(*request));
}

/*
	功能:  一些基础的控制，音量

	输入参数:
		@request: 控制参数,op不用填
		@action:
			    ACT_HTLLOCK_SET_VOL,	// 设置音量1 高音 2 中音3低音 4 静音
			    ACT_HTLLOCK_SET_LANG,	// 设置语言
			    ACT_HTLLOCK_LOCAL_OPEN,	// 局域网开锁
			    ACT_HTLLOCK_QUERY_PIN, // 查询临时PIN
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;

	if (action < ACT_HTLLOCK_SET_VOL) {
		return RS_INVALID_PARAM;
	}
	
	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, action, value);
}



/*
	功能:  请求历史数据，返回的信息保存在 cl_htllock_info_t的history里面

	输入参数:
		@slave_handle: 设备的句柄
		@request: 请求参数,op 不用填
 	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_rf_htllock_request_history(cl_handle_t slave_handle, htllock_tt_info_notice_get_history_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_HTLLOCK_GET_HISTORY, (u_int8_t*)request, sizeof(*request));
}

////////////////////////////////////////////////////////////////////////////////////
// 空调贴
CLIB_API RS cl_get_air_code_match_stat(cl_handle_t dev_handle,cl_rf_air_code_match_stat_t* stat)
{
    RS ret = ERR_NONE;
    slave_t* slave;
    
    CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
    memset(stat, 0, sizeof(*stat));
    
    cl_lock(&cl_priv->mutex);
    
    if ((slave = (slave_t *)lookup_by_handle(HDLT_SLAVE, dev_handle)) == NULL) {
        log_err(false, "cl_get_air_code_match_stat request 0x%08x failed: not found\n", dev_handle);
        ret = RS_NOT_FOUND;
        goto done;
    }
    
    stat->error = slave->match_mana.rf_ir_match.error;
    stat->cur_step = slave->match_mana.rf_ir_match.cur_step;
    stat->matched_id = slave->match_mana.rf_ir_match.matched_id;
    stat->max_step = slave->match_mana.rf_ir_match.max_step;
    stat->num = slave->match_mana.rf_ir_match.num;
    memcpy(&stat->matched_ids, &slave->match_mana.rf_ir_match.matched_ids, sizeof(stat->matched_ids));
    
    
done:
    cl_unlock(&cl_priv->mutex);
    
    return RS_OK;
}

CLIB_API RS cl_set_air_code_tmp_adjust(cl_handle_t slave_handle, int8_t tmp)
{
	u_int8_t local_tmp = (u_int8_t)tmp*10;

	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_TMP_ADJUST, local_tmp);
}

CLIB_API RS cl_set_air_code_led_mode(cl_handle_t slave_handle, u_int8_t mode)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_RF_COM_LED_MODE, mode);
}

CLIB_API RS cl_set_air_code_dir(cl_handle_t slave_handle, u_int8_t *dir, u_int8_t len)
{
	u_int8_t buf[257];
	
	CL_CHECK_INIT;

	//清空一下
	memset(buf, 0, sizeof(buf));
	
	if (!dir ||
		len < 4) {
		return RS_ERROR;
	}

	buf[0] = len;
	memcpy((void *)&buf[1], (void *)dir, len);
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_DIR, (u_int8_t*)buf, len+1);
}

CLIB_API RS cl_set_air_ir_id(cl_handle_t slave_handle, u_int16_t id)
{	
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_IR_SET, (u_int8_t*)&id, sizeof(id));
}


CLIB_API RS cl_set_air_code_dir_auto_adjust(cl_handle_t slave_handle, u_int8_t send_num, u_int8_t send_inter, u_int8_t send_timeout)
{
	u_int8_t buf[257];
	
	CL_CHECK_INIT;

	//清空一下
	memset(buf, 0, sizeof(buf));
	
	buf[0]= send_num;
	buf[1]= send_inter;
	buf[2]= send_timeout;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_CHECK, (u_int8_t*)buf, 4);	
}

CLIB_API RS cl_set_air_code_check(cl_handle_t slave_handle, u_int8_t on, u_int8_t timeout)
{
	u_int8_t buf[257];
	
	CL_CHECK_INIT;

	//清空一下
	memset(buf, 0, sizeof(buf));
	
	buf[0]= on;
	buf[1]= timeout;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_DIR_ADJUST, (u_int8_t*)buf, 4);	
}

CLIB_API RS cl_set_air_shock_auto_check(cl_handle_t slave_handle, u_int8_t step, u_int8_t onoff)
{	
	u_int8_t buf[257];
	
	CL_CHECK_INIT;
	//清空一下
	memset(buf, 0, sizeof(buf));

	buf[0] = step;
	buf[1] = onoff;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_SHOCK_ADJUST, (u_int8_t*)buf, 4);		
}

CLIB_API RS cl_set_air_shock_status_query(cl_handle_t slave_handle)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_SHOCK_QUERY, 0);	
}

CLIB_API RS cl_set_air_ir_status_query(cl_handle_t slave_handle)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_IR_QUERY, 0);	
}

CLIB_API RS cl_set_air_ir_addr(cl_handle_t slave_handle, u_int8_t addr)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_ADDR_SET, addr);	
}

CLIB_API RS cl_set_air_ir_sync_onoff(cl_handle_t slave_handle, u_int8_t onoff)
{
	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_WKAIR_IR_SYNC, onoff);	
}

////////////////////////////////////////////////////////////////////////////////////
//橙灯情景面板
CLIB_API RS cl_set_cdqjmb_name(cl_handle_t slave_handle, u_int8_t index, char *name)
{
	u_int8_t buf[100];
	cdqjmb_set_name_t *pconf = (cdqjmb_set_name_t *)buf;
	
	CL_CHECK_INIT;

	pconf->index = index;
	if (!name) {
		pconf->len = 0;
	} else {
		pconf->len = (u_int8_t)strlen(name);
	}

	memcpy((void *)pconf->name, (void *)name, pconf->len);

	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, 
		ACT_CDQJMB_NAME_SET, (u_int8_t*)buf, sizeof(*pconf) + pconf->len);		
}


////////////////////////////////////////////////////////////////////////////////////
// 夜狼声光报警
CLIB_API RS cl_rf_yllight_lamp_ctrl(cl_handle_t dev_handle, cl_rf_lamp_stat_t* stat)
{    
    CL_CHECK_INIT;
    
    if (!stat) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
                                   CLNE_RF_COM_DEV, ACT_RF_YLLIGHT_LAMP_CTRL, (u_int8_t*)stat, sizeof(*stat));
}

CLIB_API RS cl_rf_yllight_alarm_config(cl_handle_t dev_handle, cl_yllight_alarm_config_t* config)
{	
    CL_CHECK_INIT;
    
    if (!config) {
        return RS_INVALID_PARAM;
    }
    
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
                                   CLNE_RF_COM_DEV, ACT_RF_YLLIGHT_ALARM_CONFIG, (u_int8_t*)config, sizeof(*config));
}

CLIB_API RS cl_rf_yllight_com_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value)
{    
    CL_CHECK_INIT;

    return cl_send_u32_notify(&cl_priv->thread_main, dev_handle,
                                   CLNE_RF_COM_DEV, action, value);
}

////////////////////////////////////////////////////////////////////////////////////
// 智皇电机
CLIB_API RS cl_zhdj_status_set(cl_handle_t slave_handle, u_int8_t status)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_ZHDJ_STATUS_SET, status);
}

CLIB_API RS cl_zhdj_location_set(cl_handle_t slave_handle, u_int8_t location)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_ZHDJ_LOCATION_SET, location);
}

CLIB_API RS cl_zhdj_location_query(cl_handle_t slave_handle)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_ZHDJ_LOCATION_QUERY, 0);
}

CLIB_API RS cl_zhdj_bind(cl_handle_t slave_handle, u_int32_t magic, u_int8_t index, u_int8_t type)
{
	u_int8_t buff[100];
	zhdj_bind_t *bind = (zhdj_bind_t *)buff;
	
	CL_CHECK_INIT;

	bind->magic = magic;
	bind->index = index;
	bind->type = type;

	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, 
		CLNE_RF_COM_DEV, ACT_ZHDJ_BIND, (u_int8_t*)bind, sizeof(*bind));
}

CLIB_API RS cl_zhdj_type_set(cl_handle_t slave_handle, u_int8_t type, u_int8_t index)
{
	u_int8_t buff[100];
	
	CL_CHECK_INIT;

	buff[0] = type;
	buff[1] = index;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, 
		CLNE_RF_COM_DEV, ACT_ZHDJ_TYPE, buff, 2);
}

CLIB_API RS cl_zhdj_dir_set(cl_handle_t slave_handle, u_int8_t dir)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_ZHDJ_DIR, dir);
}

CLIB_API RS cl_dwkj_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	
	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, action, value);
}

CLIB_API RS cl_dwkj_timer_ctrl(cl_handle_t slave_handle, cl_dwkj_timer_t *request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_DWKJ_TIMER, (u_int8_t*)request, sizeof(*request));
}

CLIB_API RS cl_scene_controller_sample_ctrl(cl_handle_t slave_handle, u_int8_t action, u_int32_t value)
{
	CL_CHECK_INIT;
	
	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, action, value);
}

CLIB_API RS cl_scene_controller_set_key(cl_handle_t slave_handle, u_int8_t id, char *name)
{
	u_int8_t buf[64] = {0};
	ucp_scene_controller_key_set_t *request = (ucp_scene_controller_key_set_t *)buf;
	int len;
	
	CL_CHECK_INIT;

	if (id == 0 || id > 4) {
		return RS_INVALID_PARAM;
	}

	len = (int)strlen(name) + 1;

	if (len > 24) {		
		return RS_INVALID_PARAM;
	}

	request->id = id;
	request->len = (u_int8_t)strlen(name) + 1;
	memcpy(request->name, name, request->len);

	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_SC_SET_NAME, (u_int8_t*)request, sizeof(*request) + request->len);
}

//魔乐路数自定义名称
CLIB_API RS cl_dhx_ml_set_key(cl_handle_t slave_handle, u_int8_t id, char *name)
{
	u_int8_t buf[1024] = {0};
	ucp_scene_controller_key_set_t *request = (ucp_scene_controller_key_set_t *)buf;
	int len;
	
	CL_CHECK_INIT;

	if (!name) {
		return RS_INVALID_PARAM;
	}

	len = (int)strlen(name) + 1;

	request->id = id;
	request->len = (u_int8_t)len;
	memcpy(request->name, name, request->len);

	return cl_send_var_data_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_DHX_SET_NAME, (u_int8_t*)request, sizeof(*request) + request->len);	
}

CLIB_API RS cl_rfdev_public_set_shortcuts_onoff(cl_handle_t dev_handle, u_int8_t enable, u_int8_t onoff, u_int32_t time)
{
	char buf[1024];
	cl_shortcuts_onoff_t *pso = (cl_shortcuts_onoff_t *)buf;
	
    CL_CHECK_INIT;

	pso->enable = enable;
	pso->onoff = onoff;
	pso->remain_time = time;
	
    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
			CLNE_RF_COM_DEV, ACT_RF_COM_SHORTCUTS_ONFF_SET, (u_int8_t*)pso, sizeof(*pso));

}

CLIB_API RS cl_rfdev_public_query_shortcuts_onoff(cl_handle_t dev_handle)
{
	
    CL_CHECK_INIT;

    return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle,
    		CLNE_RF_COM_DEV, ACT_RF_COM_SHORTCUTS_ONOFF_QUERY, NULL, 0);
}



CLIB_API RS cl_rf_jq_set_threshold(cl_handle_t slave_handle, u_int8_t type, u_int16_t user_value)
{
	u_int32_t value = 0;
	
	CL_CHECK_INIT;

	value = type | ((user_value << 16) & 0xffff0000);
	
	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_JQ_SET_ALARM_THR, value);
}

CLIB_API RS cl_rf_jq_set_alarm_period(cl_handle_t slave_handle, u_int16_t period)
{
	u_int32_t value = 0;
	
	CL_CHECK_INIT;

	value = (u_int32_t)period;
	
	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_JQ_SET_ALARM_PERIOD, value);
}

CLIB_API RS cl_rf_jq_flush_ch2o(cl_handle_t slave_handle)
{
	u_int32_t value = 0;
	
	CL_CHECK_INIT;
	
	return cl_send_u32_notify(&cl_priv->thread_main, slave_handle, CLNE_RF_COM_DEV, ACT_JQ_FLUSH_CH2O, value);
}


/**************************************网关透传外设控制接口**********************************************************************************/
CLIB_API RS cl_rfdev_set_yl_time(cl_handle_t gw_handle, u_int8_t time)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, gw_handle, CLNE_RFGW_SCM_CTRL, ACT_RF_YL_ALARM_TIME, time);
}

CLIB_API RS cl_rfdev_set_yl_voice(cl_handle_t gw_handle, u_int8_t type, u_int8_t voice)
{
	u_int8_t buf[100];
	
	CL_CHECK_INIT;

	buf[0] = type;
	buf[1] = voice;

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle,
    		CLNE_RFGW_SCM_CTRL, ACT_RF_YL_VOICE, buf, 2);
}

CLIB_API RS cl_rfdev_set_yl_siren_off(cl_handle_t gw_handle)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, gw_handle, CLNE_RFGW_SCM_CTRL, ACT_RF_YL_SIREN_OFF, 0);
}

CLIB_API RS cl_rfdev_set_yl_siren_onoff(cl_handle_t gw_handle, u_int8_t onoff)
{
	CL_CHECK_INIT;

	return cl_send_u8_notify(&cl_priv->thread_main, gw_handle, CLNE_RFGW_SCM_CTRL, ACT_RF_YL_SIREN_OFF, onoff);
}

CLIB_API RS cl_rfdev_up_check(cl_handle_t gw_handle)
{
	u_int8_t data = 0;

	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_UP_CHECK, data);
}

CLIB_API RS cl_rfdev_img_cache_query(cl_handle_t gw_handle)
{
	u_int8_t data = 0;

	CL_CHECK_INIT;
	
	return cl_send_u8_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_IMG_CACHE_QUERY, data);
}

CLIB_API RS cl_rfdev_img_cache_del(cl_handle_t gw_handle, u_int8_t num, u_int8_t *pindex)
{
	int i, len;
	u_int8_t buf[1024];
	
	CL_CHECK_INIT;

	if (!pindex || (0 == num)) {
		return RS_INVALID_PARAM;
	}

	len = 0;
	buf[len++] = num;
	for(i = 0; i < num; i++) {
		buf[len++] = pindex[i];
	}

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_DEV_IMG_CACHE_DEL, buf, len);	
}


CLIB_API RS cl_hpgw_config_appinfo_onoff(cl_handle_t gw_handle, u_int8_t onoff)
{
	int len;
	u_int8_t buf[1024] = {0};
	ucp_hpgw_appinfo_request_t *request = (ucp_hpgw_appinfo_request_t*)buf;
	
	CL_CHECK_INIT;

	request->appinfo = onoff;

	len = sizeof(*request);

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_HPGW_APPINFO, buf, len);	
}

CLIB_API RS cl_hpgw_config_sms(cl_handle_t gw_handle, u_int8_t sms_onoff, u_int8_t lang)
{
	int len;
	u_int8_t buf[1024] = {0};
	ucp_hpgw_hpgw_sms_request_t *request = (ucp_hpgw_hpgw_sms_request_t*)buf;
	
	CL_CHECK_INIT;

	request->sms = sms_onoff;
	request->lang = lang;

	len = sizeof(*request);

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_HPGW_SMS, buf, len);	
}

CLIB_API RS cl_hpgw_config_phone_user(cl_handle_t gw_handle, u_int8_t *name, u_int64_t phone_number)
{
	int len;
	u_int8_t buf[1024] = {0};
	ucp_hpgw_phone_user_request_t *request = (ucp_hpgw_phone_user_request_t*)buf;
	
	CL_CHECK_INIT;

	if (strlen(name) > 16) {
		return RS_INVALID_PARAM;
	}

	memcpy(request->name, name, strlen(name));
	request->phone_number = phone_number;
	

	len = sizeof(*request);

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_HPGW_CONFIG_USER, buf, len);	
}

CLIB_API RS cl_hpgw_del_phone_user(cl_handle_t gw_handle, u_int64_t phone_number)
{
	int len;
	u_int8_t buf[1024] = {0};
	ucp_hpgw_phone_user_del_t *request = (ucp_hpgw_phone_user_del_t*)buf;
	
	CL_CHECK_INIT;

	request->phone_number = phone_number;
	
	len = sizeof(*request);

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_HPGW_DEL_USER, buf, len);	
}


CLIB_API RS cl_hpgw_lamp_ctrl(cl_handle_t gw_handle, cl_rf_lamp_stat_t *stat)
{
	int len;

	CL_CHECK_INIT;

	len = sizeof(*stat);

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_HPGW_LAMP_CTRL, (u_int8_t*)stat, len);	
}

CLIB_API RS cl_rf_com_runtime_query(cl_handle_t slave_handle)
{
	cl_handle_t gw_handle = INVALID_HANDLE;
	slave_t *slave;
	u_int64_t sn = 0;
	
	if(slave_handle == INVALID_HANDLE){
		return RS_INVALID_PARAM;
	}
		
	cl_lock(&cl_priv->mutex);
	slave = lookup_by_handle(HDLT_SLAVE, slave_handle);
	if(slave && slave->user){
		gw_handle = slave->user->handle;
		sn = slave->sn;
	}
	cl_unlock(&cl_priv->mutex);

	if(gw_handle == INVALID_HANDLE) {
		return RS_INVALID_PARAM;
	}
	
	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_RF_RUNTIME_QUERY, (u_int8_t *)&sn, sizeof(sn));	
}

CLIB_API RS cl_rfgw_onekey_ctrl(cl_handle_t gw_handle, cl_rfdev_onekey_ctrl_t *ctrl)
{
	CL_CHECK_INIT;

	return cl_send_var_data_notify(&cl_priv->thread_main, gw_handle, 
		CLNE_RFGW_GATEWAY, ACT_RFGW_ONEKEY, (u_int8_t*)ctrl, sizeof(*ctrl));	
}

