#include "yj_heater_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_yj_heater.h"


static u_int8_t yj_calc_crc(u_int8_t* src ,int len)
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


bool yj_heater_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
    cl_yj_heater_set_t* priv_info, *recv_info;
    u_int8_t buffer[256] = {0};
	
	if(!user || !user->smart_appliance_ctrl){
		*ret = RS_OFFLINE;
		return false;
	}

    ac = ((smart_appliance_ctrl_t*)(user->smart_appliance_ctrl))->sub_ctrl;
    info = (cln_common_info_t *)&pkt->data[0];
    
    if (!ac || !(priv_info = ac->com_udp_dev_info.device_info)) {
        *ret = RS_INVALID_PARAM;
        return false;
    }

    switch (info->action) {
        case ACT_YJ_HEATER_CTRL:
			recv_info = cci_pointer_data(info);

			recv_info->syn1 = YJ_START_CODE1;
			recv_info->syn2 = YJ_START_CODE2;	
			recv_info->cmd = 1;
			recv_info->param_len = sizeof(*recv_info) - 6;	// 出去syn1 syn2 cmd pad len checksum

			// pad 必须清空
			recv_info->pad = 0;
			recv_info->pad1[0] = 0;
			recv_info->pad1[1] = 0;
			recv_info->pad1[2] = 0;
			
			recv_info->checksum = yj_calc_crc((u_int8_t*)recv_info, sizeof(*recv_info) - 1);

			scm_send_single_set_pkt(user->uc_session, recv_info, sizeof(*recv_info));
			
            break;
        default:
			return false;
    }
   
    return true;
}

static void yj_heater_scm_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{

	cl_yj_heater_info_t* priv_info = ac->com_udp_dev_info.device_info;
	cl_yj_heater_info_t* recv_info = (cl_yj_heater_info_t* )pcmd;
	u_int8_t checksum;
    
	if (!priv_info  || cmd_len < sizeof(*recv_info)){
        log_err(false,"yj_heater_scm_do_parse_scm_command err command len %u\n", cmd_len);
	    return;
	}


	
    if (recv_info->syn1 !=  YJ_START_CODE1 || recv_info->syn2 !=  YJ_START_CODE2) {
        log_err(false, "err start code  %02X %02X\n", recv_info->syn1, recv_info->syn2);
        return;
    }

	checksum = yj_calc_crc((u_int8_t*)recv_info, sizeof(*recv_info) - 1);
	if (checksum != recv_info->check_sum) {
		log_err(false, "real checksum 0x%x != pack checksum 0x%x\n", checksum, recv_info->check_sum);
		return;
	}
    
    memcpy((u_int8_t*)priv_info, (u_int8_t*)recv_info, sizeof(*priv_info));
}

static bool yj_heater_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
                yj_heater_scm_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool yj_heater_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    switch (obj->attr) {
        case UCAT_IA_TT_ALLSTATE:
            ret = yj_heater_scm_update_tlv_data(air_ctrl,action,obj);
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


int yj_heater_scm_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
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


