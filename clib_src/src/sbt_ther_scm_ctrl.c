#include "sbt_ther_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_yl_thermostat.h"

static u_int8_t sbt_calc_crc(u_int8_t* src ,int len)
{
    int i,total;
    
    if(!src||len<=0){
        return 0;
    }
    
    total = 0;
    
    for(i=0;i<len;i++){
        total += src[i];
    }
    
    return total & 0xFF;
}

static u_int8_t sbt_ther_proc_stat(cl_sbt_ther_info_t* sb,void* app_data,u_int8_t* dest)
{
    ucp_sbt_scm_stat_t* stat = (ucp_sbt_scm_stat_t*)dest;
    cl_sbt_work_ctrl_t* wc = (cl_sbt_work_ctrl_t*)app_data;
    
    sb->onoff = !!wc->onoff;
    sb->mode = wc->mode;
    sb->temp = wc->temp;
    sb->fan_speed = wc->fan_speed;
    
    stat->onoff = sb->onoff;
    stat->mode = sb->mode;
    stat->fan_speed = sb->fan_speed;
    stat->temp = sb->temp;
    
    return sizeof(*stat);
}

static u_int8_t sbt_ther_proc_func(cl_sbt_ther_info_t* sb,void* app_data,u_int8_t* dest)
{
    ucp_sbt_scm_func_t* sf = (ucp_sbt_scm_func_t*)dest;
    cl_sbt_func_setting_t* fs = (cl_sbt_func_setting_t*)app_data;
    
    sb->auto_mode = sf->auto_mode = !!fs->auto_mode;
    sb->is_low_temp_guard = sf->is_low_temp_guard = !!fs->is_low_temp_guard;
    sb->low_temp = sf->low_temp = fs->low_temp;
    sf->max_temp = sb->max_temp = fs->max_temp;
    sf->min_temp = sb->min_temp = fs->min_temp;
    sf->return_temp = sb->return_temp = fs->return_temp;
    sf->temp_adjust = sb->temp_adjust = fs->temp_adjust;
    sf->valve_mode = sb->valve_mode = fs->valve_mode;
    
    return sizeof(*sf);
}

static u_int8_t sbt_ther_proc_time_adjust(cl_sbt_ther_info_t* sb,void* app_data,u_int8_t* dest)
{
    ucp_sbt_scm_time_adjust_t* ta = (ucp_sbt_scm_time_adjust_t*)dest;
    cl_sbt_time_adjust_t* st = (cl_sbt_time_adjust_t*)app_data;
    
    ta->scm_hour = st->scm_hour;
    ta->scm_min = st->scm_min;
    ta->scm_sec = st->scm_sec;
    ta->scm_weekday = st->scm_weekday;
    
    return sizeof(*ta);
}

static u_int8_t sbt_ther_proc_smart_config(cl_sbt_ther_info_t* sb,void* app_data,u_int8_t* dest)
{
    cl_smart_smart_ctrl_t * css = (cl_smart_smart_ctrl_t*)app_data;
    cl_smart_smart_ctrl_t * dss = (cl_smart_smart_ctrl_t*)dest;
    
    
    memcpy(&sb->smart_info, css, sizeof(*css));
    memcpy(dss, css, sizeof(*css));
    
    return sizeof(*css);
}

bool sbt_ther_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
    ucp_sbt_scm_head_t* head;
    cl_sbt_ther_info_t* sb;
    cl_sbt_func_setting_t fs;
    u_int8_t buffer[256] = {0};
    u_int8_t len = 0;
	
	if(!user || !user->smart_appliance_ctrl){
		*ret = RS_OFFLINE;
		return false;
	}

    head = (ucp_sbt_scm_head_t*)buffer;
    ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    info = (cln_common_info_t *)&pkt->data[0];
    
    if (!ac || !(sb = ac->com_udp_dev_info.device_info) || !sb->is_data_valid) {
        *ret = RS_INVALID_PARAM;
        return false;
    }

    switch (info->action) {
        case ACT_SBT_CTRL_STAT:
            len = sbt_ther_proc_stat(sb,cci_pointer_data(info),(u_int8_t*)(head+1));
            head->command = SBT_SCM_CMD_STAT;
            break;
        case ACT_SBT_SETTINT_PARAM:
            len = sbt_ther_proc_func(sb,cci_pointer_data(info),(u_int8_t*)(head+1));
            head->command = SBT_SCM_CMD_FUNC_PARAM;
            break;
        case ACT_SBT_SMART_MODE_ENABLE:
            fs.auto_mode = !!cci_u8_data(info);
            fs.is_low_temp_guard = sb->is_low_temp_guard;
            fs.low_temp = sb->low_temp;
            fs.max_temp = sb->max_temp;
            fs.min_temp = sb->min_temp;
            fs.return_temp = sb->return_temp;
            fs.temp_adjust = sb->temp_adjust;
            fs.valve_mode = sb->valve_mode;
            len = sbt_ther_proc_func(sb,&fs,(u_int8_t*)(head+1));
            head->command = SBT_SCM_CMD_FUNC_PARAM;
            break;
        case ACT_SBT_SMART_MODE_PARAM:
            len = sbt_ther_proc_smart_config(sb,cci_pointer_data(info),(u_int8_t*)(head+1));
            head->command = SBT_SCM_CMD_AUTO_PARAM;
            break;
        case ACT_SBT_AJUST_TIME:
            len = sbt_ther_proc_time_adjust(sb,cci_pointer_data(info),(u_int8_t*)(head+1));
            head->command = SBT_SCM_CMD_TIME_ADJUST;
            break;
        default:
            break;
    }
   
    if (len == 0) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    head->start_code1 = SBT_START_CODE1;
    head->start_code2 = SBT_START_CODE2;
    head->data_len = len + 0x3;
    buffer[sizeof(*head)+len] = sbt_calc_crc(buffer, len+sizeof(*head));
	
	scm_send_single_set_pkt(user->uc_session,buffer,len+sizeof(*head)+1);
    
    return true;
}

static void sbt_ther_scm_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	cl_sbt_ther_info_t* sb = ac->com_udp_dev_info.device_info;
    ucp_sbt_scm_head_t* head;
    ucp_sbt_scm_info_t* ssi;
    
	if(!sb  || cmd_len < sizeof(*head)+sizeof(*ssi)){
        log_err(false,"sbt_ther_scm_do_parse_scm_command err command len %u\n",cmd_len);
	    return;
	}

    head = (ucp_sbt_scm_head_t*)(pcmd);
    ssi = (ucp_sbt_scm_info_t*)(head+1);
	
    if (head->start_code1 != SBT_START_CODE1 || head->start_code2 != SBT_START_CODE2) {
        log_err(false,"sbt_ther_scm_do_parse_scm_command err start code  %02X %02X\n",head->start_code1,head->start_code2);
        return;
    }
    
    sb->auto_mode = ssi->auto_mode;
    sb->broad_cast_type = ssi->broad_cast_type;
    sb->fan_speed = ssi->fan_speed;
    sb->is_data_valid = true;
    sb->is_low_temp_guard = !!ssi->is_low_temp_guard;
    sb->lock_screen = !!ssi->lock_screen;
    sb->low_temp = ssi->low_temp;
    sb->max_sesor_temp = ssi->max_sesor_temp;
    sb->max_temp = ssi->max_temp;
    sb->min_temp = ssi->min_temp;
    sb->mode = ssi->mode;
    sb->no_paid_mode = !!ssi->no_paid_mode;
    sb->onoff = !!ssi->onoff;
    sb->return_temp = ssi->return_temp;
    sb->room_temp = (u_int16_t)ssi->room_temp_integer*10+ssi->room_temp_decimal;
    sb->scm_hour = ssi->scm_hour;
    sb->scm_min = ssi->scm_min;
    sb->scm_sec = ssi->scm_sec;
    sb->scm_weekday = ssi->scm_weekday;
    sb->sesor_type = ssi->sesor_type;
    memcpy(&sb->smart_info, &ssi->smart_info, sizeof(ssi->smart_info));
    sb->temp = ssi->temp;
    sb->temp_adjust = ssi->temp_adjust;
    sb->valve_mode = ssi->valve_mode;

}

static bool sbt_ther_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    uc_tlv_t* tlv;
    int remain = obj->param_len;
    
    if(remain < sizeof(*tlv)){
        return false;
    }
    
    tlv = (uc_tlv_t*)(obj+1);
    tlv->type = ntohs(tlv->type);
    tlv->len = ntohs(tlv->len);
    while (remain >= sizeof(uc_tlv_t) && (u_int32_t)remain >= sizeof(uc_tlv_t) + tlv->len) {
        remain -= (sizeof(uc_tlv_t) + tlv->len);
        
        switch (tlv->type) {
            case TLV_TYPE_SCM_COMMAND:
                sbt_ther_scm_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
                break;
            default:
                break;
        }
        
        tlv = tlv_next(tlv);
        if (remain >= sizeof(uc_tlv_t)) {
            tlv->type = ntohs(tlv->type);
            tlv->len = ntohs(tlv->len);
        }
    }
    
    
    return true;
}

bool sbt_ther_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    switch (obj->attr) {
        case UCAT_IA_TT_ALLSTATE:
            ret = sbt_ther_scm_update_tlv_data(air_ctrl,action,obj);
            break;
        case UCAT_IA_TT_CMD:
            break;
        case UCAT_IA_TT_CMDRET:
            break;
        default:
            break;
    }
    
    return ret;
}


int sbt_ther_scm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
{
	u_int8_t value;
    int ext_type = 0;
    tlv_dev_ident_t* ti;
    
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
        switch (value) {
            case 0:
                break;
                
            default:
                break;
        }
    }
    
    return ext_type;
}

