#include "zh_jl_lamp_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_lede_lamp.h"


bool zh_jl_lamp_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	u_int32_t data;
	cln_common_info_t *info;
	ucp_zh_jl_3200_pkt zh_3200 = {0};
	smart_air_ctrl_t* ac;
	cl_jl_lamp_info_t* jl_info;
	
	if(!user || !user->smart_appliance_ctrl){
		*ret = RS_OFFLINE;
		return false;
	}

    ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    jl_info = ac->com_udp_dev_info.device_info;
    info = (cln_common_info_t *)&pkt->data[0];
    switch (info->action) {
        case ACT_JL_3200_CTRL:
        case ACT_JL_3200_AL_CTRL:
            data = cci_u32_data(info);
            jl_info->on_off = !!((data >> 24) &0xFF);
            jl_info->lamp_3200_info.color = (data >> 16) &0xFF;
            jl_info->lamp_3200_info.bright = (data >> 8) &0xFF;
            if (info->action == ACT_JL_3200_AL_CTRL) {
                jl_info->lamp_3200_info.total_bright = data & 0xFF;
            }
            break;
        default:
			*ret = RS_INVALID_PARAM;
		return false;
            break;
    }
   
	zh_3200.start_code = 0xAA;
	zh_3200.on_off |= 0x10;
    zh_3200.total_bright = jl_info->lamp_3200_info.total_bright;
	if(jl_info->on_off){
		zh_3200.on_off |= 0x1;
		zh_3200.color = jl_info->lamp_3200_info.color;
		zh_3200.bright = jl_info->lamp_3200_info.bright;
	}else{
		zh_3200.on_off |= 0x1;
		zh_3200.color = jl_info->lamp_3200_info.color;
		zh_3200.bright = 0;
	}
	
	scm_send_single_set_pkt(user->uc_session,&zh_3200,sizeof(zh_3200));
    
    return true;
}

static void zh_jl_lamp_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	cl_jl_lamp_info_t* jl_info = ac->com_udp_dev_info.device_info;
	ucp_zh_jl_3200_pkt* zp = (ucp_zh_jl_3200_pkt*)pcmd;

	if(!jl_info || !zp || cmd_len < sizeof(*zp)){
	    return;
	}

	if(zp->start_code != 0xAA){
		return;
	}

	switch(ac->sac->user->ext_type){
		case ETYPE_JL_STAGE_LAMP:
			jl_info->lamp_type = JL_LAMP_3200;
			break;
		default:
			return;
			break;
	}
	
	jl_info->lamp_3200_info.bright = zp->bright;
	jl_info->lamp_3200_info.color = zp->color;
    jl_info->lamp_3200_info.total_bright = zp->total_bright;
	jl_info->on_off = zp->on_off;
	

}

static bool zh_jl_lamp_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
                zh_jl_lamp_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool zh_jl_lamp_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    switch (obj->attr) {
        case UCAT_IA_TT_ALLSTATE:
            ret = zh_jl_lamp_update_tlv_data(air_ctrl,action,obj);
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


int zh_jl_lamp_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
{
	u_int8_t value;
    int ext_type = 0;
    tlv_dev_ident_t* ti;
    
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
        switch (value) {
            case JL_LAMP_3200:
                ext_type = ETYPE_JL_STAGE_LAMP;
                break;
                
            default:
                break;
        }
    }
    
    return ext_type;
}

