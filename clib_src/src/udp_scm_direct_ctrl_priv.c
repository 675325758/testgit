#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "lc_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "amt_scm_ctrl.h"
#include "chiffo_scm_ctrl.h"
#include "bimar_scm_ctrl.h"
#include "zh_jl_lamp_ctrl.h"
#include "kxm_scm_ctrl.h"
#include "sbt_ther_scm_ctrl.h"
#include "yj_heater_scm_ctrl.h"
#include "evm_scm_ctrl.h"
#include "rfgw_scm_ctrl.h"
#include "leis_scm_ctrl.h"
#include "yinsu_scm_ctrl.h"
#include "cl_zhdhx.h"

int htc_get_ext_type_by_tlv(uc_tlv_t* tlv);

bool scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = false;
	//联创的
	if(pkt->type >= CLNE_AH_START && pkt->type <= CLNE_AH_END){
		// call 联创
		return lc_scm_proc_notify(user,pkt,ret);
	}

	switch(pkt->type){
		//前锋
		case CLNE_COMMON_UDP_CHIFFO_FLOOR_HEATER_CTRL:
			break;
		default:
			break;
	}

	//返回失败表示无法处理
	return res;
}

void scm_quick_query_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t stat_objs[] = {{UCOT_IA, UCSOT_IA_TT, UCAT_IA_TT_ALLSTATE, 0}};

	if(!ac)
	   return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
	log_debug("scm_quick_query_info *********\n");
    if (ac->sac->user->sub_type == IJ_KXM_DEVICE) {
        kxm_do_query_hook(ac);
    }
	
    if (ac->sac->user->sub_type == IJ_RFGW) {
        rfgw_do_query_hook(ac);
    }
	
	if(ac->sac->user->sub_type == IJ_AMT){
		amt_send_set_timer_ctrl_cmd(ac);
        if (ac->com_udp_dev_info.is_support_ext_period_timer) {
            log_debug("scm_quick_query_info query public timer info\n");
            stat_objs[0].objct = UCOT_IA;
            stat_objs[0].sub_objct = UCSOT_IA_PUBLIC;
            stat_objs[0].attr = UCAT_IA_PUBLIC_EXT_PERIOD_TIMER;
            sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
        }else if(ac->air_info.is_support_peroid_timer){
            log_debug("scm_quick_query_info query public timer info\n");
            stat_objs[0].objct = UCOT_IA;
            stat_objs[0].sub_objct = UCSOT_IA_PUBLIC;
            stat_objs[0].attr = UCAT_IA_PUBLIC_PERIOD_TIMER;
            sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
        }
	}
    
    
}

bool scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	user_t* user = air_ctrl->sac->user;
	bool ret = false;

	log_debug("scm get UCSOT_IA_TT sub_type %u ext_type %u obj param len %u\n", user->sub_type, user->ext_type, obj->param_len);
	
	switch(user->sub_type){
		case IJ_EVM:
			ret = evm_scm_update_data(air_ctrl, action, obj);
			break;
		case IJ_812: //联创公司
		case IJ_808:
			ret = lc_scm_update_data( air_ctrl,  action,  obj);
			break;
		case IJ_AMT:
			ret = amt_scm_update_data(air_ctrl,action,obj);
			break;
		case IJ_CHIFFO:
            ret = chiffo_scm_update_data(air_ctrl,action,obj);
			break;
		case IJ_LEIS:
			switch (user->ext_type) {
				case ETYPE_LEIS_DEFAULT:	
		            ret = leis_scm_update_data(air_ctrl,action,obj);
					break;
				case ETYPE_LEIS_YINSU:
					ret = yinsu_scm_update_data(air_ctrl,action,obj);
					break;
			}
			break;
		case IJ_JL_STAGE_LAMP:
			switch (user->ext_type) {
	                case ETYPE_JL_STAGE_LAMP:
				ret = zh_jl_lamp_update_data(air_ctrl,action,obj);
	                    break;
	                    
	                default:
	                    break;
	            }
		break;
        case IJ_TEST_DEV:
        {
            switch (user->ext_type) {
                case ETYPE_IJ_TEST_BITMAR:
                    ret = bimar_scm_update_data(air_ctrl, action, obj);
                    break;
                    
                default:
                    break;
            }
        }
            break;
        case IJ_HEATER_DEV:
            switch (user->ext_type) {
                case ETYPE_IJ_HEATER_BIMAR_C2000:
                case ETYPE_IJ_HEATER_AKM_0505L:
                case ETYPE_IJ_HEATER_AKM_5162L:
                    ret = bimar_scm_update_data(air_ctrl, action, obj);
                    break;
                    
                default:
                    break;
            }
            break;
        case IJ_KXM_DEVICE:
        {
            switch (user->ext_type) {
                case ETYPE_IJ_KXM_HOST:
				case EYTYP_IJ_KXM_AC:
                    ret = kxm_scm_update_data(air_ctrl, action, obj);
                    break;
                case ETYPE_IJ_SBT_THER:
                    ret = sbt_ther_scm_update_data(air_ctrl, action, obj);
                    break;
				case ETYPE_IJ_YJ_HEATER:
                    ret = yj_heater_scm_update_data(air_ctrl, action, obj);
                    break;
                default:
                    break;
            }
        }
            break;
		case IJ_RFGW:
			ret = rfgw_scm_update_data(air_ctrl, action, obj);
			break;
		default:
			break;
	}

	return ret;
}


int scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	user_t* user = ctrl->sac->user;
    int event = 0;
	
	switch(user->sub_type){
		case IJ_812: //联创公司
			event = lc_scm_proc_ctrl_result(ctrl,obj, error);
			break;
		default:
            if(error == ERR_NONE){
                event = SAE_COMMON_CTRL_OK;
            }else{
                event = SAE_COMMON_CTRL_FAILED;
            }
			break;
	}

	return event;
}
static void copy_ident_to_user(user_t* user,u_int8_t* ident,int ident_len)
{	
	if(ident == NULL || ident_len<=0){	
		return;
	}

	SAFE_FREE(user->scm_dev_desc);
	user->scm_dev_desc_len = 0;
	user->scm_dev_desc = cl_calloc(ident_len+1, 1);
	if(!user->scm_dev_desc){
		log_err(true,"***********************\ncopy_ident_to_user malloc memory failed!\n***********************\n");
		return;
	}
	user->scm_dev_desc_len = ident_len;
	memcpy(user->scm_dev_desc,ident,ident_len);

	log_info("copy_ident_to_user ok user ident %s\n",user->scm_dev_desc);
	
}

 int scm_get_ext_type_by_ident(u_int8_t dev_sub_type,u_int8_t* ident,int ident_len)
 {
 	int ext_type  = -1;
	
 	switch(dev_sub_type){
		case IJ_812: //联创公司
			ext_type = lc_get_ext_type_by_ident(ident,ident_len);
			break;
		case IJ_AMT:
			ext_type = 0;
            break;
		default:
			break;
	}
	
 	return ext_type;
 }

//根据设备类型处理,被调用函数返回OK,设备才支持直接与单片机通讯
int scm_do_devtype_ident(user_t* user,u_int8_t* ident,int ident_len)
{
	int ext_type;

	if(ident == NULL || ident_len <= 0){
		return RS_ERROR;
	}
	
	copy_ident_to_user(user,ident,ident_len);

	ext_type = scm_get_ext_type_by_ident(user->sub_type,user->scm_dev_desc,user->scm_dev_desc_len);
	
	if(user->sub_type != IJ_812){
		return RS_OK;
	}	

	if(ext_type  >= 0 ){
		user->ext_type = ext_type;
		user->is_dev_support_scm = true;
	}else{
		user->is_dev_support_scm = false;
	}

	return RS_OK;
}

int scm_do_dispatch_tlv(user_t* user,uc_tlv_t* tlv)
{
	if(!user || !tlv || tlv->len == 0){
		return 0;
	}
	
    switch(tlv->type){
        case UCT_SCM_DEV_TYPE:
            scm_do_devtype_ident(user,(u_int8_t*)(tlv+1),tlv->len);
            break;
		case UCT_DEVICE_NEW_DEV_EXT_TYPE:
			evm_set_type_by_tlv(&user->sub_type, &user->ext_type, tlv);		
			user->type_fixed = true;
			break;
        case UCT_SCM_FACTORY_DATA:
			user->type_fixed = true;
			if (user->sub_type == IJ_ZHDHX) {
				smart_appliance_ctrl_t *sac = NULL;
				smart_air_ctrl_t *sa = NULL;
				cl_zhdhx_info_t *priv_info;
				u_int8_t type = *(u_int8_t *)(tlv+1);

				if (tlv->len > 0 && 
					(sa_init(user) == RS_OK)) {
					sac = user->smart_appliance_ctrl;
					if (sac && sac->sub_ctrl) {
						sa = sac->sub_ctrl;
						priv_info = (cl_zhdhx_info_t *)(sa->com_udp_dev_info.device_info);
						if (priv_info) {
							priv_info->dhx_type = type;
							log_debug("sn=%llu type=%u\n", user->sn, type);
						}
					}
				}
			} else {
           		user->ext_type = scm_get_ext_type_by_tlv(user->sub_type,user->ext_type,tlv) & 0xFF;
			}
            break;
	 case UCT_808_PAN_TYPE:
		if(tlv->len > 0 ){
			user->pan_type_for_808 = *((char*)(tlv+1));
			user->support_set_pan = true;
		}
		break;
	 case UCT_DEVELOPER_ID:
	 	if (tlv->len >= 32) {
			memcpy(user->developer_id, tlv_val(tlv), 32);
	 	}
	 	break;
     default:
            break;
    }
		
	return RS_OK;
}

static u_int8_t  normal_dev_get_ext_type_by_tlv(u_int8_t sub_type,uc_tlv_t* tlv)
{
    u_int8_t value = 0;
    u_int8_t ext_type = 0;
    tlv_dev_ident_t* ti;
    
    
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
        
    }
    
    switch (sub_type) {
        case IJ_JS_MICWAVE:
            switch (value) {
                case 0x1:
                    ext_type = ETYPE_IJ_JS_MICWARE;
                    break;
                case 0x2:
                    ext_type = ETYPE_IJ_JS_MIC_BARBECUE;
                    break;
                case 0x3:
                    ext_type = ETYPE_IJ_JS_ONLY_MIC;
                    break;
                    
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    
    return ext_type;
}

int scm_get_ext_type_by_tlv(u_int8_t dev_sub_type,u_int8_t real_ext_type,uc_tlv_t* tlv)
{
	int ext_type = 0;

	if (!tlv) {
	    return 0;
	}

	switch(tlv->type){
	    case UCT_SCM_FACTORY_DATA:
        {
            switch (dev_sub_type) {
                case IJ_AMT:
                    ext_type = amt_get_ext_type_by_tlv(tlv);
                    break;
                case IJ_CHIFFO:
                    ext_type = chiffo_get_ext_type_by_tlv(tlv);
                    break;
                case IJ_TEST_DEV:
                {
                    switch (real_ext_type) {
                        case ETYPE_IJ_TEST_BITMAR:
                            ext_type = bimar_get_ext_type_by_tlv(dev_sub_type,tlv);
                            break;
                            
                        default:
                            break;
                    }
                }
                    break;
                case IJ_HEATER_DEV:
                    switch (real_ext_type) {
                        case ETYPE_IJ_HEATER_BIMAR_C2000:
                            ext_type = bimar_get_ext_type_by_tlv(dev_sub_type,tlv);
                            break;
                            
                        default:
                            break;
                    }
                    break;
				case IJ_824:
					ext_type = htc_get_ext_type_by_tlv(tlv);
					break;
                case IJ_KXM_DEVICE:
                    if (real_ext_type == ETYPE_IJ_KXM_HOST || real_ext_type == EYTYP_IJ_KXM_AC) {
                        ext_type = kxm_get_ext_type_by_tlv(dev_sub_type, tlv);
                    }else if(real_ext_type == ETYPE_IJ_SBT_THER){
                        ext_type = sbt_ther_scm_get_ext_type_by_tlv(dev_sub_type, tlv);
                    }

					if (real_ext_type == ETYPE_IJ_YJ_HEATER) {
						ext_type = yj_heater_scm_get_ext_type_by_tlv(dev_sub_type, tlv);
					}
                    
                    break;
                case IJ_JS_MICWAVE:
                    ext_type = normal_dev_get_ext_type_by_tlv(dev_sub_type, tlv);
                    break;
				case IJ_RFGW:
					ext_type = rfgw_get_ext_type_by_tlv(dev_sub_type, tlv);
					break;
                default:
                    break;
            }
        }
	        break;
	    default:
	        break;
	}

    return ext_type;
}

bool scm_send_single_set_pkt(ucc_session_t *s,void* content,int content_len)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
	ucp_obj_t* obj;
	uc_tlv_t* tlv;
    
    if (!s || !content ||content_len <= 0) {
        return RS_ERROR;
    }
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+content_len+sizeof(ucp_obj_t)+sizeof(uc_tlv_t),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
    uc = get_ucp_payload(pkt, ucp_ctrl_t);
    obj = (ucp_obj_t*)(uc+1);
    tlv = (uc_tlv_t*)(obj+1);
	
    uc->action = UCA_SET;
    uc->count = 0x1;
	
	obj->objct = htons(UCOT_IA);
	obj->sub_objct= htons(UCSOT_IA_TT);
	obj->attr= htons(UCAT_IA_TT_CMD);
	obj->param_len= htons((u_int16_t)(sizeof(*tlv)+content_len));

	tlv->type = htons(0x1);
	tlv->len = htons(content_len);
	
    memcpy((void*)(tlv+1), content, content_len);
    
    ucc_request_add(s, pkt);    
       
    return true;
}

RS scm_send_single_get_pkt(ucc_session_t *s,void* content,int content_len)
{
     pkt_t *pkt;
    ucp_ctrl_t* uc;
	ucp_obj_t* obj;
	uc_tlv_t* tlv;
    
    if (!s || !content ||content_len <= 0) {
        return RS_ERROR;
    }
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+content_len+sizeof(ucp_obj_t)+sizeof(uc_tlv_t),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
    uc = get_ucp_payload(pkt, ucp_ctrl_t);
   obj = (ucp_obj_t*)(uc+1);
    tlv = (uc_tlv_t*)(obj+1);
	
    uc->action = UCA_GET;
    uc->count = 0x1;
	
	obj->objct = htons(UCOT_IA);
	obj->sub_objct= htons(UCSOT_IA_TT);
	obj->attr= htons(UCAT_IA_TT_CMD);
	obj->param_len= htons((u_int16_t)(sizeof(*tlv)+content_len));

	tlv->type = htons(0x1);
	tlv->len = htons(content_len);
	
    memcpy((void*)(tlv+1), content, content_len);
    
    ucc_request_add(s, pkt);    
       
    return RS_OK;
}


////////////////////////////////不想再添加个私有文件，先放这里吧//////////////////////////////
#define HTC_SYS_TYPE_HOME			(0X0)
#define HTC_SYS_TYPE_YCJ			(0X3)
#define HTC_SYS_TYPE_BUSINESS		(0X8)
int htc_get_ext_type_by_tlv(uc_tlv_t* tlv)
{
    u_int8_t value;
    int ext_type = ETYPE_IJ_824_HTC_BUSINESS;
    tlv_dev_ident_t* ti;
    
    
    if (tlv->len >= sizeof(*ti)) {
        ti = (tlv_dev_ident_t*)(tlv+1);
        value = ti->ident;
        switch (value) {
            case HTC_SYS_TYPE_HOME:
                ext_type = ETYPE_IJ_824_HTC;
                break;
            case HTC_SYS_TYPE_YCJ:
                ext_type = TYPE_IJ_824_YCJ;
                break;
            case HTC_SYS_TYPE_BUSINESS:
                ext_type = ETYPE_IJ_824_HTC_BUSINESS;
                break;
            default:
                break;
        }
    }
    
    return ext_type;
}

