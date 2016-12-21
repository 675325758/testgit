#include "bimar_scm_ctrl.h"
#include "cl_bimar_dev.h"
#include "udp_scm_direct_ctrl_priv.h"

static u_int8_t bimar_calc_crc(char* src ,int len)
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

static u_int8_t c_temp_2_f_temp(int c_temp)
{
    float f_t = (float)c_temp;
    
    f_t = (float)((((c_temp - 10.0)*9.0)/5.0) + 50.0);
    
    return (u_int8_t)(f_t+0.5);
}

static u_int8_t f_temp_2_c_temp(int f_temp)
{
    float c_t = (float)f_temp;
    
    c_t = (float)((((f_temp - 50.0)*5.0)/9.0)+10.0);
    
    return (u_int8_t)(c_t + 0.5);
}


static bool _bimar_send_common_ctrl(user_t* user,smart_air_ctrl_t* ac,cl_bimar_heater_info_t* hi,RS* ret)
{
    char buf[128] = {0};
    int len = 0;
    stm_bimar_head_t* head;
    stm_bimar_tail_t* tail;
    stm_bimar_body_t* body;
    
    head = (stm_bimar_head_t*)buf;
    body = (stm_bimar_body_t*)(head+1);
    tail = (stm_bimar_tail_t*)(body+1);
    
    len = sizeof(*head) + sizeof(*body) + sizeof(*tail);
    
    head->len = sizeof(*body);
    head->start_1 = BIMAR_START_CODE;
    head->start_2 = BIMAR_START_CODE;
    head->cmd = BIMAR_ACT_SET;
    
    //开关
    if (hi->is_on) {
        body->onoff = BIMAR_ON;
    }else{
        body->onoff = BIMAR_OFF;
    }
    
    //摇头
    if (hi->is_shake) {
        body->shake_stat = BIMAR_ON;
    }else{
        body->shake_stat = BIMAR_OFF;
    }
    
    //后续根据其他设备类型再判断
    if (hi->machine_type == BIM_DEV_HC2000L) {
        //暖风机没有自动
        body->auto_valid = BIMAR_UN_SUPPORT_FUNC;
        body->anion_onoff = BIMAR_UN_SUPPORT_FUNC;
        
        //温度单位
        body->temp_unit = hi->temp_unit;
        if (body->temp_unit == BIM_TEMP_UNIT_UNKNOWN || body->temp_unit > BIM_TEMP_UNIT_FAHR) {
            body->temp_unit = BIM_TEMP_UNIT_CELS;
        }
        
        // 功率档位
        body->gear = hi->power_gear;
        if (body->gear > BIM_GEAR_800W_EXT || body->gear == BIM_GEAR_UNKNOWN) {
            body->gear = BIM_GEAR_800W_EXT;
        }
        
        //设置的摄氏温度
        body->set_c_temp = hi->cur_set_cels_temp;
        if (body->set_c_temp < BIM_MIN_C_TEMP) {
            body->set_c_temp = BIM_MIN_C_TEMP;
        }
        
        if (body->set_c_temp > BIM_MAX_C_TEMP) {
            body->set_c_temp = BIM_MAX_C_TEMP;
        }
        
        //设置的华氏温度
        body->set_f_temp = hi->cur_set_fahr_temp;
        if (body->set_f_temp < BIM_MIN_F_TEMP) {
            body->set_f_temp = BIM_MIN_F_TEMP;
        }
        
        if (body->set_f_temp > BIM_MAX_F_TEMP) {
            body->set_f_temp = BIM_MAX_F_TEMP;
        }
        body->f_room_temp = (u_int8_t)hi->fahr_room_temp;
        body->setting_min = hi->cur_remain_min;
       
    }else if(hi->machine_type == BIM_DEV_0505L){
        body->auto_valid = BIMAR_UN_SUPPORT_FUNC;
        body->anion_onoff = BIMAR_UN_SUPPORT_FUNC;
        //不支持
        body->fan_speed = BIMAR_UN_SUPPORT_FUNC;
        body->temp_unit = BIMAR_UN_SUPPORT_FUNC;
        //设置的华氏温度
        body->set_f_temp = BIMAR_UN_SUPPORT_FUNC;
        body->f_room_temp = BIMAR_UN_SUPPORT_FUNC;
        body->setting_min = BIMAR_UN_SUPPORT_FUNC;
        
        // 功率档位
        body->gear = hi->power_gear;
        if (body->gear > BIM_GEAR_HIGH || body->gear == BIM_GEAR_UNKNOWN) {
            body->gear = BIM_GEAR_LOW;
        }
        
        //设置的摄氏温度
        body->set_c_temp = hi->cur_set_cels_temp;
        if (body->set_c_temp < AKM_0505L_MIN_C_TEMP) {
            body->set_c_temp = AKM_0505L_MIN_C_TEMP;
        }
        
        if (body->set_c_temp > AKM_0505L_MAX_C_TEMP) {
            body->set_c_temp = AKM_0505L_MAX_C_TEMP;
        }
        
        
    }else if(hi->machine_type == BIM_DEV_5162L){
        body->auto_valid = BIMAR_UN_SUPPORT_FUNC;
        body->anion_onoff = BIMAR_UN_SUPPORT_FUNC;
        //不支持
        body->fan_speed = BIMAR_UN_SUPPORT_FUNC;
        body->temp_unit = BIMAR_UN_SUPPORT_FUNC;
        //设置的华氏温度
        body->set_f_temp = BIMAR_UN_SUPPORT_FUNC;
        body->f_room_temp = BIMAR_UN_SUPPORT_FUNC;
        body->setting_min = hi->cur_remain_min;
        
        // 功率档位
        body->gear = hi->power_gear;
        if (body->gear > BIM_GEAR_HIGH || body->gear == BIM_GEAR_UNKNOWN) {
            body->gear = BIM_GEAR_LOW;
        }
        
        //设置的摄氏温度
        body->set_c_temp = hi->cur_set_cels_temp;
        if (body->set_c_temp < AKM_5162L_MIN_C_TEMP) {
            body->set_c_temp = AKM_5162L_MIN_C_TEMP;
        }
        
        if (body->set_c_temp > AKM_5162L_MAX_C_TEMP) {
            body->set_c_temp = AKM_5162L_MAX_C_TEMP;
        }
        
        
    }else{
        //负离子
        if (hi->is_anion_enable) {
            body->anion_onoff = BIMAR_ON;
        }else{
            body->anion_onoff = BIMAR_OFF;
        }
        //风速
        body->fan_speed = hi->fan_speed;
        if (body->fan_speed > BIM_FAN_SPEED_HIGH ) {
            body->fan_speed = BIM_FAN_SPEED_HIGH;
        }
        
        //温度单位
        body->temp_unit = hi->temp_unit;
        if (body->temp_unit == BIM_TEMP_UNIT_UNKNOWN || body->temp_unit > BIM_TEMP_UNIT_FAHR) {
            body->temp_unit = BIM_TEMP_UNIT_CELS;
        }
        
        //设置的摄氏温度
        body->set_c_temp = hi->cur_set_cels_temp;
        if (body->set_c_temp < BIM_MIN_C_TEMP) {
            body->set_c_temp = BIM_MIN_C_TEMP;
        }
        
        if (body->set_c_temp > BIM_MAX_C_TEMP) {
            body->set_c_temp = BIM_MAX_C_TEMP;
        }
        
        //设置的华氏温度
        body->set_f_temp = hi->cur_set_fahr_temp;
        if (body->set_f_temp < BIM_MIN_F_TEMP) {
            body->set_f_temp = BIM_MIN_F_TEMP;
        }
        
        if (body->set_f_temp > BIM_MAX_F_TEMP) {
            body->set_f_temp = BIM_MAX_F_TEMP;
        }
        body->f_room_temp = (u_int8_t)hi->fahr_room_temp;

    }
    
    body->timer = hi->cur_set_timer;
    
    body->c_room_temp = (u_int8_t)hi->cels_room_temp;
    body->machine_type = hi->machine_type;
    
    
    //从cmd开始计算
    body->crc = bimar_calc_crc((char*)&head->cmd, 2+sizeof(*body));
    tail->end = BIMAR_END_CODE;
    
    scm_send_single_set_pkt(user->uc_session,buf,len);
    
    return true;
}

static bool _bimar_proc_common_ctrl(user_t* user,u_int8_t action,u_int8_t value,RS* ret)
{
    smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    cl_bimar_heater_info_t* hi = ac->com_udp_dev_info.device_info;
    
    if (!hi || !hi->is_data_valid) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    switch (action) {
        case BIM_CTRL_TYPE_ONOFF:
            hi->is_on = !!value;
            if(!hi->is_on){
                hi->cur_set_timer = 0;
                hi->cur_remain_min = 0;
                
                if (hi->machine_type == BIM_DEV_0505L||
                    hi->machine_type == BIM_DEV_5162L) {
                    hi->is_shake  = false;
                    hi->power_gear = BIM_GEAR_LOW;
                }
            }
            
            break;
        case BIM_CTRL_TYPE_ANION:
            hi->is_anion_enable = !!value;
            break;
        case BIM_CTRL_TYPE_SHAKE:
            hi->is_shake = !!value;
            break;
        case BIM_CTRL_TYPE_TEMP_UNIT:
            hi->temp_unit = value;
            break;
        case BIM_CTRL_TYPE_GEAR:
            hi->power_gear = value;
            break;
        case BIM_CTRL_TYPE_CELS_TEMP:
            hi->cur_set_cels_temp = value;
            hi->cur_set_fahr_temp = c_temp_2_f_temp(value);
            break;
        case BIM_CTRL_TYPE_FAHR_TEMP:
            hi->cur_set_fahr_temp = value;
            hi->cur_set_cels_temp = f_temp_2_c_temp(value);
            break;
        case BIM_CTRL_TYPE_TIMER:
            hi->cur_set_timer = value;
            hi->cur_remain_min = 0;
            break;
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    return _bimar_send_common_ctrl(user,ac,hi,ret);
}

bool bimar_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
    u_int8_t value;
    u_int8_t action;
    u_int16_t data;
    cln_common_info_t *info;
    
    info = (cln_common_info_t *)&pkt->data[0];
    switch (info->action) {
        case ACT_BIMAR_COMMON_CTRL:
            data = cci_u16_data(info);
            action = (data >> 8 ) &0xff;
            value = data &0xff;
            return _bimar_proc_common_ctrl(user,action,value,ret);
            break;
            
        default:
            break;
    }
   
    
    return false;
}

static void bimar_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
    cl_bimar_heater_info_t* hi = ac->com_udp_dev_info.device_info;
    stm_bimar_head_t* head;
    stm_bimar_tail_t* tail;
    stm_bimar_body_t* body;
    
    if(!hi || cmd_len < sizeof(*head)+sizeof(*tail)+sizeof(*body)){
        return;
    }
    
    
    head = (stm_bimar_head_t*)pcmd;
    body = (stm_bimar_body_t*)(head+1);
    
    if (head->start_1 != BIMAR_RSP_S_CODE || head->start_2 != BIMAR_RSP_S_CODE) {
        return;
    }
    
    if (head->len < sizeof(*body)) {
        return;
    }
    
    hi->is_on = (body->onoff == BIMAR_ON)?true:false;
    hi->is_shake = (body->shake_stat == BIMAR_ON)?true:false;
    hi->is_anion_enable = (body->anion_onoff == BIMAR_ON)?true:false;
    hi->fan_speed = body->fan_speed;
    hi->temp_unit = body->temp_unit;
    hi->power_gear = body->gear;
    hi->cur_set_cels_temp = body->set_c_temp;
    hi->cur_set_fahr_temp = body->set_f_temp;
    hi->cur_set_timer = body->timer;
    hi->cels_room_temp = body->c_room_temp;
    hi->fahr_room_temp = body->f_room_temp;
    hi->cur_remain_min = body->setting_min;
    hi->machine_type = body->machine_type;
    hi->is_data_valid = true;
    hi->loc_update_gnu_time = (u_int32_t)time(NULL);

	if (hi->old_set_cels_temp != 0 && hi->cur_set_cels_temp != hi->old_set_cels_temp) {
		hi->scm_run_mode = AUCMA_RUN_MODE_TEMP;
	}
	if (hi->old_gear != 0 && hi->power_gear != hi->old_gear) {
		hi->scm_run_mode = AUCMA_RUN_MODE_GEAR;
	}
	hi->old_set_cels_temp = hi->cur_set_cels_temp;
	hi->old_gear = hi->power_gear;
}

static bool bimar_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
                bimar_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool bimar_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    switch (obj->attr) {
        case UCAT_IA_TT_ALLSTATE:
            ret = bimar_scm_update_tlv_data(air_ctrl,action,obj);
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

int bimar_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
{
    u_int8_t value;
    int ext_type = 0;
    tlv_dev_ident_t* ti;
    
    if (sub_type == IJ_TEST_DEV) {
        return ETYPE_IJ_TEST_BITMAR;
    }
    
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
        switch (value) {
            case BIM_DEV_HC2000L:
                ext_type = ETYPE_IJ_HEATER_BIMAR_C2000;
                break;
            case BIM_DEV_0505L:
                ext_type = ETYPE_IJ_HEATER_AKM_0505L;
                break;
            case BIM_DEV_5162L:
                ext_type = ETYPE_IJ_HEATER_AKM_5162L;
                break;
                
            default:
                break;
        }
    }
    
    return ext_type;
}