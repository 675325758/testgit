#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "yinsu_scm_ctrl.h"

#if 0
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif


static void _yinsu_send_set_cmd(smart_air_ctrl_t* ac, u_int8_t cmd, u_int8_t *data, u_int16_t len)
{
	u_int8_t buf[512] = {0};
	
	ucp_yinsu_pkt_hdr_t *hd = (ucp_yinsu_pkt_hdr_t *)buf;
	hd->syn1 = 0xff;
	hd->syn2 = 0xaa;
	hd->cmd = cmd;
	hd->param_len = ntohs(len);

	memcpy((u_int8_t*)&hd[1], data, len);
	
	scm_send_single_set_pkt(ac->sac->user->uc_session, buf, sizeof(*hd) + len);
}

static bool yinsu_do_lamp_ctrl(smart_air_ctrl_t* ac, cl_rf_lamp_stat_t *src)
{
	u_int8_t buf[512] = {0};
	yinsu_uart_lamp_ctrl_param_t *dest;
	int len;
	cl_yinsu_info_t *pv = (cl_yinsu_info_t *)ac->com_udp_dev_info.device_info;
	cl_rf_lamp_t *rl = &(pv->lamp_stat);
	
    dest = (yinsu_uart_lamp_ctrl_param_t*)buf;
    len = sizeof(*dest);
    
    dest->o_wc_l = src->action;
    dest->o_r = src->R;
    dest->o_g = src->G;
    dest->o_b = src->B;
    dest->o_l = src->L;
    dest->o_c = src->cold;

	dest->type = src->flag;
    
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

	if (dest->C == 2)
		dest->C = 1;

	if (dest->W == 2)
		dest->W = 1;
    
    dest->mod_id = src->mod_id;
    dest->power = src->power;
    dest->sub_cmd = SC_SET;
   // dest->type = 0x0;
    dest->hwconf = src->ctrl_mode;
	
	if ( rl->lamp_type == RL_LT_LAYER ) {
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

	_yinsu_send_set_cmd(ac, 2, buf, sizeof(*dest));

	return true;
}

bool yinsu_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_yinsu_info_t *pv;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = ac->com_udp_dev_info.device_info;
	
	switch (info->action){
		case ACT_YINSU_CTRL_LAMP:
			yinsu_do_lamp_ctrl(ac, (cl_rf_lamp_stat_t *)cci_pointer_data(info));
			break;
			
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	
	return res;
}


static bool _yinsu_do_update(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	ucp_yinsu_pkt_hdr_t* hdr = (ucp_yinsu_pkt_hdr_t*)pcmd;
	yinsu_uart_lamp_ctrl_param_t* pkt = (yinsu_uart_lamp_ctrl_param_t*)&hdr[1];
	cl_yinsu_info_t *pv = (cl_yinsu_info_t *)ac->com_udp_dev_info.device_info;
	cl_rf_lamp_t *rl = &(pv->lamp_stat);
	u_int8_t m,n;
	u_int16_t org_rgb_l, tmp;

	log_debug("yinsu get cmd_len %u\n", cmd_len);

	if (cmd_len < sizeof(*hdr)) {
		log_err(false, "invalid cmd len\n");
		return false;
	}

	hdr->param_len = ntohs(hdr->param_len);

	if (hdr->param_len < sizeof(*pkt)) {
		log_err(false, "invalid cmd param_len %u\n", hdr->param_len);
		return false;
	}

#if 0
{
	cl_rf_lamp_stat_t sat;
	yinsu_do_lamp_ctrl(ac, &sat);
}
#endif


	if (hdr->cmd != 3) {
		log_err(false, "not yinsu push stat\n");
		return false;
	}


	
	
    if (pkt->o_l == 0xFF) {
        //遥控器控制灯，需要转换
        m = max(pkt->R, max(pkt->G, pkt->B));
        n = min(pkt->R, min(pkt->G, pkt->B));
        
        org_rgb_l = (m + n)*100/256;
        if(org_rgb_l > 100)
              org_rgb_l = 100;
        
        rl->stat.L = (u_int8_t)org_rgb_l;
        if(org_rgb_l > 0) {
            tmp = pkt->R *100 /org_rgb_l;
			if(tmp > 255)
				tmp = 255;
			rl->stat.R = (u_int8_t)tmp;

			tmp = pkt->G *100 /org_rgb_l;
			if(tmp > 255)
				tmp = 255;
			rl->stat.G = (u_int8_t)tmp;

			tmp = pkt->B *100 /org_rgb_l;
			if(tmp > 255)
				tmp = 255;
			rl->stat.B = (u_int8_t)tmp;

        } else {
            rl->stat.R = 128;
            rl->stat.G = 128;
            rl->stat.B = 128;
			if (pkt->R || pkt->G || pkt->B)
				rl->stat.L = 1;
				
        }

		if ( rl->lamp_type == RL_LT_LAYER ) {
			rl->stat.R = pkt->o_r;
			
			if (pkt->B > pkt->G) {
	            rl->stat.L = (u_int8_t)((int)pkt->B * 100/255.0);
	            rl->stat.B = (u_int8_t)((int)pkt->G * 50.0/ (int)pkt->B);
	        } else {
	            if(pkt->G > 0) {
	                rl->stat.L = (u_int8_t)((int)pkt->G*100/255.0);
	                rl->stat.B = 100 - (u_int8_t)((int)pkt->B*50.0/pkt->G);
	            } else {
	                rl->stat.L = 0;
	                rl->stat.B = 50;
	            }
	        }
			if (rl->stat.L == 0) {
				if (pkt->G || pkt->B)
					rl->stat.L = 1;
			}
				
			if (rl->stat.L > 100)
				rl->stat.L = 100;
			if (rl->stat.B > 100)
				rl->stat.B = 100;
		}
        
        
        if (pkt->W > pkt->C) {
            rl->stat.action = (u_int8_t)((int)pkt->W * 100/255.0);
            rl->stat.cold = (u_int8_t)((int)pkt->C * 50.0/ (int)pkt->W);
        } else {
            if(pkt->C > 0) {
                rl->stat.action = (u_int8_t)((int)pkt->C*100/255.0);
                rl->stat.cold = 100 - (u_int8_t)((int)pkt->W*50.0/pkt->C);
            } else {
                rl->stat.action = 0;
                rl->stat.cold = 50;
            }
        }

		if (rl->stat.action == 0) {
			if (pkt->W || pkt->C)
				rl->stat.action = 1;
		}
		
		if (rl->stat.action > 100)
			rl->stat.action = 100;
		if (rl->stat.cold> 100)
			rl->stat.cold = 100;
    
    }else{
        rl->stat.R = pkt->o_r;
        rl->stat.G = pkt->o_g;
        rl->stat.B = pkt->o_b;
        rl->stat.L = pkt->o_l;
        rl->stat.cold = pkt->o_c;
        rl->stat.action = pkt->o_wc_l;
    }
    
    rl->stat.mod_id = pkt->mod_id;
    rl->stat.power = pkt->power;
    rl->lamp_type = pkt->hwconf;
    if(rl->lamp_type >= RL_LT_MAX){
        rl->lamp_type = RL_LT_WC_ONLY;
    }
    
    rl->is_support_color_temp = true;
    rl->is_support_rgb = true;
    
    if (rl->lamp_type == RL_LT_WC_ONLY) {
        rl->is_support_rgb = false;
    }

	return true;
}

static bool _yinsu_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
			_yinsu_do_update(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool yinsu_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _yinsu_scm_update_tlv_data(air_ctrl,action,obj);
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

bool yinsu_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	cl_yinsu_info_t* priv;
	cl_yinsu_info_t* info;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	priv = (cl_yinsu_info_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if(!info){
		return false;
	}

	memcpy(info, priv, sizeof(*info));

	di->device_info = info;

	return true;
}


CLIB_API RS cl_yinsu_lamp_ctrl(cl_handle_t dev_handle, cl_rf_lamp_stat_t* request)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main, dev_handle, CLNE_COMMON_UDP_YINSU, ACT_YINSU_CTRL_LAMP, (u_int8_t*)request, sizeof(*request));
}

