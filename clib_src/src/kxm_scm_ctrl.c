#include "kxm_scm_ctrl.h"
#include "cl_yl_thermostat.h"
#include "udp_scm_direct_ctrl_priv.h"

static u_int8_t kxm_calc_crc(u_int8_t* src ,int len)
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


static bool _kxm_proc_common_ctrl(user_t* user,cl_kxm_info_t* k_info,RS* ret)
{
    u_int8_t buf[256] = {0};
    ucp_kxm_smc_head* head;
    u_int8_t* body;
    u_int8_t crc_len = 0x1;
    u_int8_t len = KXM_MIN_BODY_LEN+sizeof(*head);
    
    head = (ucp_kxm_smc_head*)buf;
    body = (u_int8_t*)(head+1);
    //头部
    head->s_code1 = 0x5A;
    head->s_code2 = 0xA5;
    head->dev_index = 0x1;
    head->command = 0x6;
    head->start_addr = 0x0;
    head->d_len = KXM_MIN_BODY_LEN;
    //控制
    //开关 低2bit
    if (k_info->hinfo.on_off) {
        body[51] |= 0x1;
    }
    //模式
    body[52] |= (k_info->hinfo.mode & 0x3);
	body[53] = k_info->hinfo.hot_water_setting_temp;
	body[54] = k_info->hinfo.hot_setting_temp;
	body[55] = k_info->hinfo.cold_setting_temp;
    //时钟
    body[58] = k_info->hinfo.t_hour;
    body[59] = k_info->hinfo.t_min;
    
    memcpy(&body[61], k_info->hinfo.timer, sizeof(k_info->hinfo.timer));
    
    body[KXM_MIN_BODY_LEN] = kxm_calc_crc(buf,len);
    
    scm_send_single_set_pkt(user->uc_session,buf,sizeof(*head)+KXM_MIN_BODY_LEN+crc_len);
    return true;
}

bool kxm_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
    u_int16_t data;
    cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    cl_kxm_info_t* k_info;
    cl_kxm_timer_info_t* ti;
    u_int8_t* p,index;
    
    if (!user->smart_appliance_ctrl) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    k_info = ac->com_udp_dev_info.device_info;
    if (!k_info || !k_info->has_receive_data) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    info = (cln_common_info_t *)&pkt->data[0];
    switch (info->action) {
        case ACT_KXM_CTRL_ONOFF:
            if(k_info->hinfo.on_off == !!cci_u8_data(info)){
                return true;
            }
            k_info->hinfo.on_off = !!cci_u8_data(info);
            break;
        case ACT_KXM_CTRL_MODE:
            data = cci_u16_data(info);
            k_info->hinfo.mode = (data >> 8) & 0xFF;
            switch (k_info->hinfo.mode) {
                case KXM_MODE_HOT_WATER:
                    k_info->hinfo.hot_water_setting_temp = data & 0xFF;
                    break;
                case KXM_MODE_HOT:
                    k_info->hinfo.hot_setting_temp = data & 0xFF;
                    break;
                case KXM_MODE_COLD:
                    k_info->hinfo.cold_setting_temp = data & 0xFF;
                    break;
                    
                default:
                    *ret = RS_INVALID_PARAM;
                    return false;
                    break;
            }
            break;
        case ACT_KXM_CTRL_DEV_TIME:
            data = cci_u16_data(info);
            k_info->hinfo.t_hour = (data >> 8) & 0xFF;
            k_info->hinfo.t_min = data & 0xFF;
            break;
        case ACT_KXM_CTRL_TIMER:
            p = cci_pointer_data(info);
            index = *p;
            if(index >=1 && index <=3){
                memcpy(&k_info->hinfo.timer[index -1], &p[4], sizeof(cl_kxm_timer_info_t));
            }else{
                *ret = RS_INVALID_PARAM;
                return false;
            }
            break;
        case ACT_KXM_CTRL_ALL_TIMER:
            ti = cci_pointer_data(info);
            memcpy(&k_info->hinfo.timer[0], ti, sizeof(k_info->hinfo.timer));
            break;
            
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    
    return _kxm_proc_common_ctrl(user,k_info,ret);
}

#if 0
static void _kxm_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
    ucp_kxm_smc_head* head;
    u_int8_t* body;
    cl_kxm_info_t* ki = ac->com_udp_dev_info.device_info;
    cl_kxm_pump_stat_info_t* pump;
    u_int16_t pos,i,offset;
    u_int16_t crc_len = 0x1;
    
    
    offset = 0;
    if (!pcmd || !ki) {
        return;
    }
    
    while (cmd_len >= sizeof(*head)+KXM_MIN_BODY_LEN) {
        head = (ucp_kxm_smc_head*)(pcmd+offset);
        
        if (cmd_len < sizeof(*head)+KXM_MIN_BODY_LEN || head->d_len > cmd_len-sizeof(*head) ) {
            log_err(false, "_kxm_do_parse_scm_command data length error! len = %d\n",cmd_len);
            return;
        }
        
        if (head->dev_index > MAX_KXM_SUB_PUMP_NUM || head->dev_index == 0) {
            log_err(false, "_kxm_do_parse_scm_command dev addr error! addr = %d\n",head->dev_index);
            return;
        }
        
        //1-8
        head->dev_index-=1;
        pump = &ki->pump[head->dev_index];
        
        if (head->dev_index == 0x0) {
            memcpy(ki->sdk_priv_data, pcmd, sizeof(*head)+KXM_MIN_BODY_LEN+crc_len);
        }
        
        
        body = (u_int8_t*)(head+1);
        pump->is_online = !! (body[0] & BIT(head->dev_index));
        pump->machine_type = !!(body[1] & BIT(7));
        pump->back_water_temp = body[3];
        pump->water_temp = body[4];
        pump->env_temp = body[5];
        pump->water_box_temp = body[6];
        pump->in_water_temp = body[7];
        pump->out_water_temp = body[8];
        pos = 9;
        for (i = 0; i < MAX_KXM_SUB_SYSTEM; i++) {
            pump->sub_system_info[i].scoll_temp = body[pos+i];
            pump->sub_system_info[i].inhale_temp = body[pos+4+i];
            pump->sub_system_info[i].exhaust_temp = body[pos+8+i];
        }
        
        pump->water_pos = body[31];
        pump->run_stat = body[32];
        pos = 33;
        for (i = 0; i < MAX_KXM_SUB_SYSTEM; i++) {
            pump->sub_system_info[i].exv_value = body[pos+i];
            pump->sub_system_info[i].compress_stat = !!(body[pos+4] & BIT(i));
            pump->sub_system_info[i].spray_stat = !!(body[pos+4] & BIT(i+4));
        }
        
        pump->is_fan_high = !!(body[38] & BIT(0));
        pump->is_fan_low = !!(body[38] & BIT(1));
        pump->cir_pump_run = !!(body[38] & BIT(2));
        pump->back_pump_run = !!(body[38] & BIT(3));
        pump->in_water_pump_run = !!(body[38] & BIT(4));
        pump->water_pump_run = !!(body[38] & BIT(5));
        pump->is_elec_hot_run = !!(body[38] & BIT(6));
        pump->sw_dir_tap_run = !!(body[38] & BIT(7));
        
        pump->sensor_back_water_fault = !!(body[41] & BIT(0));
        pump->sensor_water_fault = !!(body[41] & BIT(1));
        pump->sensor_env_fault = !!(body[41] & BIT(2));
        pump->sensor_water_box_fault = !!(body[41] & BIT(3));
        pump->sensor_in_water_fault = !!(body[41] & BIT(4));
        pump->sensor_out_water_fault = !!(body[41] & BIT(5));
        
        pos = 44;
        for (i = 0; i < MAX_KXM_SUB_SYSTEM; i++) {
            pump->sub_system_info[i].is_low_press_fault = !!(body[pos] & BIT(i));
            pump->sub_system_info[i].is_high_press_fault = !!(body[pos] & BIT(i+4));
            pump->sub_system_info[i].is_over_curr_fault = !!(body[pos+1] & BIT(i));
            pump->sub_system_info[i].is_exhault_fault = !!(body[pos+1] & BIT(i+4));
        }
        
        pump->is_out_water_temp_low_fault = !!(body[46] & BIT(0));
        pump->is_out_water_temp_high_fault = !!(body[46] & BIT(1));
        pump->is_in_out_temp_big_fault = !!(body[46] & BIT(2));
        
        pump->is_anti_phase_fault = !!(body[47] & BIT(0));
        pump->is_no_phase_L2_fault = !!(body[47] & BIT(1));
        pump->is_no_phase_L3_fault = !!(body[47] & BIT(2));
        pump->is_ctrl_comu_fault = !!(body[47] & BIT(3));
        
        if (head->dev_index == 0) {
            ki->hinfo.on_off = !!(body[51] & BIT(0));
            ki->hinfo.mode = body[52] & 0x3;
            ki->hinfo.hot_water_setting_temp = body[53];
            ki->hinfo.hot_setting_temp = body[54];
            ki->hinfo.cold_setting_temp = body[55];
            ki->hinfo.t_hour = body[58];
            ki->hinfo.t_min = body[59];
            
            pos = 61;
            for (i = 0; i < MAX_KXM_TIMER_CNT; i++) {
                ki->hinfo.timer[i].on_hour = body[pos+i*4];
                ki->hinfo.timer[i].on_min= body[pos+i*4+1];
                ki->hinfo.timer[i].off_hour = body[pos+i*4+2];
                ki->hinfo.timer[i].off_min = body[pos+i*4+3];
            }
            
        }
        
        cmd_len = cmd_len - (sizeof(*head)+KXM_MIN_BODY_LEN +crc_len);
        offset += (sizeof(*head)+KXM_MIN_BODY_LEN +crc_len);
    }
    
    ki->is_data_valid = true;
    ki->has_receive_data = true;
    
    
}
#else
static void _kxm_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
    ucp_kxm_smc_head* head;
    u_int8_t* body;
    cl_kxm_info_t* ki = ac->com_udp_dev_info.device_info;
    cl_kxm_pump_stat_info_t* pump;
    u_int16_t pos,i,offset;
    u_int16_t crc_len = 0x1;
    
    
    offset = 0;
    if (!pcmd || !ki) {
        return;
    }

	// 先清空全部机组信息
	memset((u_int8_t*)&ki->pump, 0x00, sizeof(ki->pump));
	memset((u_int8_t*)&ki->hinfo, 0x00, sizeof(ki->hinfo));
    
    while (cmd_len >= sizeof(*head)+KXM_MIN_BODY_LEN) {
        head = (ucp_kxm_smc_head*)(pcmd+offset);

		if (head->s_code1 != 0xA5 || head->s_code2 != 0x5A) {
			log_err(false, "invalid syn code 0x%x 0x%x\n", head->s_code1, head->s_code2);
			return;
		}
		
		log_debug("remain %u offset %u d_len %u\n", cmd_len, offset, head->d_len);
        
        if (cmd_len < sizeof(*head)+head->d_len + crc_len) {
            log_err(false, "_kxm_do_parse_scm_command data length error! remain len = %d dlen %u\n",cmd_len, head->d_len);
            return;
        }
        
        if (head->dev_index > MAX_KXM_SUB_PUMP_NUM || head->dev_index == 0) {
            log_err(false, "_kxm_do_parse_scm_command dev addr error! addr = %d\n",head->dev_index);
            return;
        }
        
        //1-8
        head->dev_index-=1;
        pump = &ki->pump[head->dev_index];
        
        if (head->dev_index == 0x0) {
            memcpy(ki->sdk_priv_data, pcmd, sizeof(*head)+KXM_MIN_BODY_LEN+crc_len);
        }
        
        
        body = (u_int8_t*)(head+1);
        pump->is_online = !! (body[0] & BIT(head->dev_index));
        pump->machine_type = !!(body[1] & BIT(7));
        pump->back_water_temp = body[3];
        pump->water_temp = body[4];
        pump->env_temp = body[5];
        pump->water_box_temp = body[6];
        pump->in_water_temp = body[7];
        pump->out_water_temp = body[8];
        pos = 9;
        for (i = 0; i < MAX_KXM_SUB_SYSTEM; i++) {
            pump->sub_system_info[i].scoll_temp = body[pos+i];
            pump->sub_system_info[i].inhale_temp = body[pos+4+i];
            pump->sub_system_info[i].exhaust_temp = body[pos+8+i];
        }
        
        pump->water_pos = body[31];
        pump->run_stat = body[32];
        pos = 33;
        for (i = 0; i < MAX_KXM_SUB_SYSTEM; i++) {
            pump->sub_system_info[i].exv_value = body[pos+i];
            pump->sub_system_info[i].compress_stat = !!(body[pos+4] & BIT(i));
            pump->sub_system_info[i].spray_stat = !!(body[pos+4] & BIT(i+4));
        }
        
        pump->is_fan_high = !!(body[38] & BIT(0));
        pump->is_fan_low = !!(body[38] & BIT(1));
        pump->cir_pump_run = !!(body[38] & BIT(2));
        pump->back_pump_run = !!(body[38] & BIT(3));
        pump->in_water_pump_run = !!(body[38] & BIT(4));
        pump->water_pump_run = !!(body[38] & BIT(5));
        pump->is_elec_hot_run = !!(body[38] & BIT(6));
        pump->sw_dir_tap_run = !!(body[38] & BIT(7));
        
        pump->sensor_back_water_fault = !!(body[41] & BIT(0));
        pump->sensor_water_fault = !!(body[41] & BIT(1));
        pump->sensor_env_fault = !!(body[41] & BIT(2));
        pump->sensor_water_box_fault = !!(body[41] & BIT(3));
        pump->sensor_in_water_fault = !!(body[41] & BIT(4));
        pump->sensor_out_water_fault = !!(body[41] & BIT(5));
        
        pos = 44;
        for (i = 0; i < MAX_KXM_SUB_SYSTEM; i++) {
            pump->sub_system_info[i].is_low_press_fault = !!(body[pos] & BIT(i));
            pump->sub_system_info[i].is_high_press_fault = !!(body[pos] & BIT(i+4));
            pump->sub_system_info[i].is_over_curr_fault = !!(body[pos+1] & BIT(i));
            pump->sub_system_info[i].is_exhault_fault = !!(body[pos+1] & BIT(i+4));
        }
        
        pump->is_out_water_temp_low_fault = !!(body[46] & BIT(0));
        pump->is_out_water_temp_high_fault = !!(body[46] & BIT(1));
        pump->is_in_out_temp_big_fault = !!(body[46] & BIT(2));
        
        pump->is_anti_phase_fault = !!(body[47] & BIT(0));
        pump->is_no_phase_L2_fault = !!(body[47] & BIT(1));
        pump->is_no_phase_L3_fault = !!(body[47] & BIT(2));
        pump->is_ctrl_comu_fault = !!(body[47] & BIT(3));
        
        if (head->dev_index == 0) {
            ki->hinfo.on_off = !!(body[51] & BIT(0));
            ki->hinfo.mode = body[52] & 0x3;
            ki->hinfo.hot_water_setting_temp = body[53];
            ki->hinfo.hot_setting_temp = body[54];
            ki->hinfo.cold_setting_temp = body[55];
            ki->hinfo.t_hour = body[58];
            ki->hinfo.t_min = body[59];
            
            pos = 61;
            for (i = 0; i < MAX_KXM_TIMER_CNT; i++) {
                ki->hinfo.timer[i].on_hour = body[pos+i*4];
                ki->hinfo.timer[i].on_min= body[pos+i*4+1];
                ki->hinfo.timer[i].off_hour = body[pos+i*4+2];
                ki->hinfo.timer[i].off_min = body[pos+i*4+3];
            }
            
        }
        
        cmd_len = cmd_len - (sizeof(*head)+ head->d_len +crc_len);
        offset += (sizeof(*head)+ head->d_len +crc_len);
    }
    
    ki->is_data_valid = true;
    ki->has_receive_data = true;
    
    
}
#endif
static bool _kxm_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
                _kxm_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool kxm_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    switch (obj->attr) {
        case UCAT_IA_TT_ALLSTATE:
            ret = _kxm_scm_update_tlv_data(air_ctrl,action,obj);
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

void kxm_do_query_hook(smart_air_ctrl_t* ac)
{
    cl_kxm_info_t* ki;
    
    if (!ac || (ac->sac->user->ext_type != ETYPE_IJ_KXM_HOST && 
		ac->sac->user->ext_type != EYTYP_IJ_KXM_AC)) {
        return;
    }
    
    ki = ac->com_udp_dev_info.device_info;
    ki->has_receive_data = false;
}

int kxm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
{
    u_int8_t value;
    int ext_type = 0;
    tlv_dev_ident_t* ti;
    
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
        switch (value) {
            case 0x1:
                ext_type = ETYPE_IJ_KXM_HOST;
                break;
            case 0x6:
				ext_type = EYTYP_IJ_KXM_AC;
				break;
            default:
                break;
        }
    }
    
    return ext_type;
}
