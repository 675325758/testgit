#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"
#include "rfgw_rftt.h"
#include "cmd_misc.h"
#include "uas_client.h"
#include "cl_com_rf_dev.h"


/********************打印调试开关********************打印调试开关***************
*************打印调试开关********************打印调试开关*******************/
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

////////////////////////////////////////////////////////////////
extern bool mem_is_all_zero(u_int8_t *data, int len);
static bool _rf_htllock_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len);
static void _rf_com_detector_instert_alarm_time(cl_alarm_time_t *at, u_int8_t valid, u_int8_t type, u_int32_t time);


extern bool air_ir_get_ir_detail_code(slave_t* slave,u_int8_t* code,int* codelen);
extern void rf_air_ir_ac_status_sync(slave_t *slave);

typedef int (*DIR_CTRL_FUNC_PTR)(slave_t* slave,u_int8_t action,u_int8_t type, u_int8_t value, u_int8_t* pktbuf);

static u_int32_t rf_dorr_manage_get_max_timestamp(cl_door_magnet_info_t* rd);
bool comm_timer_support(u_int8_t ext_type);
bool parse_comm_timer(slave_t *slave, rf_tlv_t* tlv, u_int16_t data_len);
extern cl_slave_data_type _set_rf_dev_d_type(slave_t* slave);
extern void comm_timer_summary_proc(slave_t *slave, u_int8_t* data);
static int htllock_build_tt_uart_send(slave_t* slave, u_int8_t *out, int out_len, u_int8_t mcmd, u_int8_t scmd, u_int8_t *param, u_int8_t param_len);
extern bool udp_rf_dev_bmp_tlv_hook(slave_t* slave,rf_tlv_t* tlv, u_int8_t action);
extern bool is_bmp(rf_tlv_t* tlv);
extern void rf_air_ir_start_db_send(slave_t* slave);
extern void do_rfgw_index_add(slave_t *slave);
extern bool is_led_type(slave_t *slave);
void do_rfgw_index_hllock_add(slave_t *slave);
static void rf_query_comm_timer(slave_t* slave);

#if 1
// 从文件里面尝试读取用户匹配信息

/*
	user_info + xor(4byte)
*/
RS _rf_com_load_user_info(slave_t* slave, int id, u_int8_t *buf, int len)
{
	char path[256] = {0};
	int nread, i, crc = 0, crc1 = 0;
	FILE *fp = NULL;

	sprintf(path, "%s/%"PRIu64"/rf_%d.conf", cl_priv->dir, slave->sn, id);

	if ((fp = fopen(path, "rb")) == NULL) {
		log_err(false, "user info file [%s] not created..\n", path);
		goto err;
	}

	nread = (int)fread((u_int8_t*)buf, 1, len, fp);

	if (len != nread) {
		log_err(false, "need %u, but get %u\n", len, nread);
		goto err;
	}

	// 计算数据校验和
	for (i = 0; i < len; i++) {
		crc ^= buf[i];
	}

	// 再尝试读取4字节校验和
	nread = (int)fread((u_int8_t*)&crc1, 1, sizeof(crc1), fp);

	if (nread != sizeof(crc1)) {
		log_err(false, "have no crc\n");
		goto err;
	}

	fclose(fp);

	if (crc1 != crc) {
		log_err(false, "file crc 0x%x != 0x%x\n", crc1, crc);
		return RS_ERROR;
	}

	log_debug("read %"PRIu64" 's conf#%d done len %d crc 0x%x\n", slave->sn, id, len, crc);

	return RS_OK;
err:
	if (fp) {
		fclose(fp);
	}

	return RS_ERROR;
}

void _rf_com_write_user_info(slave_t *slave, int id, u_int8_t *data, int len)
{
	char path[256] = {0};
	FILE *fp;
	int i, crc = 0;

	sprintf(path, "%s/%"PRIu64"/", cl_priv->dir, slave->sn);
	MKDIR(path, 0777);
	
	sprintf(path, "%s/%"PRIu64"/rf_%d.conf", cl_priv->dir, slave->sn, id);

	if ((fp = fopen(path, "w+b")) == NULL) {
		log_err(true, "fopen path [%s] failed\n", path);
		return;
	}

	len = (int)fwrite(data, 1, len, fp);

	for (i = 0; i < len; i++) {
		crc ^= data[i];
	}

	fwrite((u_int8_t*)&crc, 1, sizeof(crc), fp);

	fclose(fp);

	
	log_debug("write %"PRIu64" 's conf#%d done len %d, crc 0x%x\n", slave->sn, id, len, crc);
	
}
#endif

static u_int8_t rf_dev_calc_xor(u_int8_t *buf, int len)
{
	u_int8_t xor = 0;
	int i;
	
	for (i = 0; i < len; i++) {
		xor ^= buf[i];
	}

	return xor;
}

static void rf_dev_query_tlv_info(slave_t* slave, u_int8_t type, u_int8_t *value, u_int8_t len)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = type;
	tlv->len = len;

	if (len) {
		memcpy((u_int8_t*)&tlv[1], value, len);
	}

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("%"PRIu64" :query rf_dev_query_tlv_info type %u len %u\n", slave->sn, type, len);
}

static u_int8_t air_cl_wind_2_net_wind(u_int8_t wind,bool reverse)
{
    if (reverse) {
        if(wind == CL_AC_WIND_HIGH)
            return AC_WIND_1;
        
        if(wind == CL_AC_WIND_LOW)
            return AC_WIND_3;
        
    }else{
        if(wind == CL_AC_WIND_HIGH)
            return AC_WIND_3;
        
        if(wind == CL_AC_WIND_LOW)
            return AC_WIND_1;
    }
    return wind;
}


/*
	通用的更新历史记录摘要信息
*/
static void rf_update_comm_history_sammary(slave_t *slave, u_int32_t index_current, u_int32_t max_count)
{
	cl_rf_dev_comm_history_info_t *chi = &slave->dev_info.rf_stat.chi;

//	chi->index_invalid = index_invalid;
	chi->index_current  = index_current;
	chi->max_count = max_count;

	log_debug("%"PRIu64": update comm history sammary: current %u count %u\n", 
		slave->sn, index_current, max_count);
	
	event_push(slave->user->callback, SAE_RF_DEV_COMM_HISTORY_SUMMARY, slave->handle, slave->user->callback_handle);
}

/*
	通用的更新历史记录详细信息
*/
static bool rf_update_comm_history_item(slave_t *slave, u_int8_t *data, u_int8_t len)
{
	cl_rf_dev_comm_history_info_t *chi = &slave->dev_info.rf_stat.chi;
	ucp_rf_hislog_info_t *info;
	u_int16_t *index, i, n, remain;

	index = (u_int16_t *)(data);
	*index = ntohs(*index);

	log_debug("old rf_update_comm_history_item len %u\n", len);

	if (len < sizeof(*index)) {
		return false;
	}

	remain = (len - sizeof(*index));
	if ((remain % sizeof(*info)) != 0) {
		return false;
	}

	n = remain / sizeof(*info);

	if (n == 0) {
		return false;
	}
		
	chi->index = *index;

	info = (ucp_rf_hislog_info_t*)&index[1];

	for (i = 0; i < n; i++, info++) {
		//if (info->timestapm == 0) {
		//	break;
		//}

		info->timestapm = ntohl(info->timestapm);
		
		chi->items[i].type = info->type;
		chi->items[i].value = info->value;
		chi->items[i].ex_type = info->ex_type;
		chi->items[i].ex_value = info->ex_value;
		chi->items[i].timestamp = info->timestapm;
		chi->items[i].valid = true;

		log_debug("add history item: type %u value %u ex_type %u ex_value %u time %u\n", 
			chi->items[i].type, chi->items[i].value, chi->items[i].ex_type, chi->items[i].ex_value, chi->items[i].timestamp);
	}

	chi->n = i;
	
	event_push(slave->user->callback, SAE_RF_DEV_COMM_HISTORY_ITEM, slave->handle, slave->user->callback_handle);

	return false;
}



static bool rf_update_comm_history_item_v2(slave_t *slave, u_int8_t *data, u_int8_t len)
{
	cl_rf_dev_comm_history_info_t *chi = &slave->dev_info.rf_stat.chi;
	ucp_rf_hislog_info_v2_t *info;
	u_int16_t *index, i, n, remain;
	u_int8_t *u8ptr, xor;

	index = (u_int16_t *)(data);
	*index = ntohs(*index);

	log_debug("new rf_update_comm_history_item len %u\n", len);

	if (len < sizeof(*index)) {
		return false;
	}

	remain = (len - sizeof(*index));
	if ((remain % sizeof(*info)) != 0) {
		return false;
	}

	n = remain / sizeof(*info);

	if (n == 0) {
		return false;
	}
		
	chi->index = *index;

	info = (ucp_rf_hislog_info_v2_t*)&index[1];

	for (i = 0; i < n; i++, info++) {
		//if (info->timestapm == 0) {
		//	break;
		//}

		u8ptr = (u_int8_t *)info;

		if ((xor = rf_dev_calc_xor(u8ptr, sizeof(*info) - 1)) != info->crc) {
			chi->items[i].valid = false;
		} else {
			chi->items[i].valid = true;
		}

		info->timestapm = ntohl(info->timestapm);
		
		chi->items[i].type = (info->type >> 4) & 0xf;
		chi->items[i].value = info->value;
		chi->items[i].ex_type = info->type & 0xf;
		chi->items[i].ex_value = info->ex_value;
		chi->items[i].timestamp = info->timestapm;
		
		log_debug("add history item: valid %u type %u value %u ex_type %u ex_value %u time %u\n", chi->items[i].valid, chi->items[i].type, chi->items[i].value, chi->items[i].ex_type, chi->items[i].ex_value, chi->items[i].timestamp);
	}

	chi->n = i;
	
	event_push(slave->user->callback, SAE_RF_DEV_COMM_HISTORY_ITEM, slave->handle, slave->user->callback_handle);

	return false;
}


/*
	通用的RF添加报警信息
*/
static bool rf_add_comm_alarm(slave_t *slave, u_int8_t *raw, int len, void (*alarm_update_idx)(slave_t *slave, u_int8_t index_alarm))
{
	ucp_rf_push_alarm_t *pa = (ucp_rf_push_alarm_t *)raw;
	cl_rf_dev_comm_alarm_info_t *cai = &slave->dev_info.rf_stat.cai;
	u_int8_t index_alarm;

	if (len < sizeof(*pa)) {
		log_err(false, "comm alarm len %u invalid\n", len);
		return false;
	}

	pa->id = ntohl(pa->id);
	pa->record_time = ntohl(pa->record_time);
	pa->object = ntohs(pa->object);
	pa->type = ntohs(pa->type);
	pa->value = ntohs(pa->value);
	index_alarm = pa->alarm_index;

	log_debug("[%"PRIu64"] update alarm: id %u record_time %u object %u type %u value %u index_alarm %u\n", 
		slave->sn, pa->id, pa->record_time, pa->object, pa->type, pa->value, index_alarm);


	if (cai->id == pa->id &&
		cai->record_time == pa->record_time &&
		cai->object == pa->object &&
		cai->type == pa->type &&
		cai->value == pa->value)
	{
		log_debug("same alarm, ignore\n");
		return false;
	}


	cai->id = pa->id;
	cai->record_time = pa->record_time;
	cai->object = pa->object;
	cai->type = pa->type;
	cai->value = (u_int32_t)pa->value;
	log_debug("sn=%"PRIu64" xxxx type=%u value=%u record_time=%u\n", 
		slave->sn, cai->type, cai->value ,cai->record_time);

	event_push(slave->user->callback, SAE_RF_DEV_COMM_ALARM_INFO, slave->handle, slave->user->callback_handle);

	if (index_alarm && alarm_update_idx) {
		alarm_update_idx(slave, index_alarm);
	}
	
    return true;
}


// RF 灯
static u_int8_t _rf_lamp_mk_query_pkt(slave_t* slave,u_int8_t* buf)
{
    ucp_rf_led_lamp_t* pkt = (ucp_rf_led_lamp_t*)buf;
    
    memset(pkt, 0, sizeof(*pkt));
    
    pkt->sub_cmd = SC_GET;
    pkt->type = 0;
    
    return sizeof(*pkt);
}

static bool _rf_lamp_update_data(slave_t* slave,u_int8_t* buf,u_int16_t len, bool cache)
{
    ucp_rf_led_lamp_t* pkt = (ucp_rf_led_lamp_t*)buf;
    cl_rfdev_status_t * rs;
    cl_rf_lamp_t* rl;
    u_int8_t m,n;
    u_int16_t org_rgb_l, tmp;
    
    if (!buf || len < sizeof(*pkt) || !slave) {
        return false;
    }

	//判断下缓存
	if (cache && (len >= 32)) {
		if (mem_is_all_zero(&buf[17], 15)) {
			return false;
		}
		pkt = (ucp_rf_led_lamp_t*)&buf[16];
	}
	
  
    rs = &slave->dev_info.rf_stat;
    rl = &rs->dev_priv_data.lamp_info;
    
    rs->d_type = _set_rf_dev_d_type(slave);
    
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
        
        
        if (pkt->W > pkt->C && pkt->W != 0) {
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

	rl->stat.flag = pkt->type;
	
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
    
	
    log_debug("update slave light color R =[%u]  G =[%u]  B =[%u] L=[%u] cold=[%u] action=[%u]\n",
                 rl->stat.R, rl->stat.G, rl->stat.B,
                 rl->stat.L, rl->stat.cold, rl->stat.action);
    
    return true;
    
}

static bool _rf_lamp_proc_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
    cl_rf_lamp_stat_t* src;
    ucp_rf_led_lamp_t* dest;
    char buf[128]={0};
    int len = 0;
	cl_rfdev_status_t * rs;
    cl_rf_lamp_t* rl;

    
    if (!slave) {
        return false;
    }
    
  
    rs = &slave->dev_info.rf_stat;
    rl = &rs->dev_priv_data.lamp_info;
    
    switch (info->action) {
        case ACT_RF_LAMP_SET_COLOR:
        {
            src = cci_pointer_data(info);
            dest = (ucp_rf_led_lamp_t*)buf;
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
            //dest->type = 0x0;
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
			
        }
            break;
            
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    
    if (len > 0) {
        log_debug("set slave light color R =[%u]  G =[%u]  B =[%u] L=[%u] cold=[%u] mod_id=[%u]\n",
                 dest->o_r,dest->o_g,dest->o_b,
                 dest->o_l,dest->o_c,dest->mod_id);
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}
/////////////////////////////////////////////////////////////////
// 夜狼声光报警器
static bool _rf_yllight_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_yllight_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.yllight_info);
	yllight_cache_t *cache = (yllight_cache_t *)param;

	if (len < sizeof(yllight_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	log_debug("%"PRIu64": yllight hislog_index_current %u flag 0x%x\n", slave->sn, cache->hislog_index_current, cache->flagbits);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->hislog_count));

	rd->mode = !!(cache->flagbits & BIT(7));
	rd->is_alarm = !!(cache->flagbits & BIT(0));
	rd->is_dynamic = !!(cache->flagbits & BIT(1));

	rd->alarm_mode = cache->alarm_mode;
	rd->on_time = cache->on_time;
	rd->off_time = ntohs(cache->off_time);
	rd->total_time = ntohs(cache->total_time);	

	log_debug("	mode %u is_alarm %u is_dy %u alarm_mode %u on_time %u off_time %u total %u\n", rd->mode, rd->is_alarm, rd->is_dynamic, rd->alarm_mode, rd->on_time, rd->off_time, rd->total_time);

	if (rd->get_cache == false) {
		rd->get_cache = true;
		rf_dev_query_tlv_info(slave, TLV_YLLIGHT_QUERY, 0, 0);
	}
	
	return true;
}

static void _rf_yllight_update_lamp_stat(cl_rf_lamp_t *rl, ucp_yllight_stat_t *pkt)
{
	u_int8_t m,n;
	u_int16_t org_rgb_l, tmp;
	
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
    
	
    log_debug("update slave light color R =[%u]  G =[%u]  B =[%u] L=[%u] cold=[%u] action=[%u] pwer[%u]\n",
                 rl->stat.R, rl->stat.G, rl->stat.B,
                 rl->stat.L, rl->stat.cold, rl->stat.action, rl->stat.power);
}

static bool _rf_yllight_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    cl_yllight_info_t* yl = &(slave->dev_info.rf_stat.dev_priv_data.yllight_info);
//    ucp_yllight_stat_t *stat;

	log_debug("sn: %"PRIu64" _rf_yllight_update_data type %u\n", slave->sn, tlv->type);

	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);

    switch (tlv->type) {
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);

		case TLV_YLLIGHT_QUERY:
			if (tlv->len < sizeof(ucp_yllight_stat_t)) {
				return false;
			}
			
			_rf_yllight_update_lamp_stat(&yl->lamp_stat, (ucp_yllight_stat_t*)&tlv[1]);
			break;
        default:
            return false;
    }

	log_debug("sn: %"PRIu64" update com detector data done\n", slave->sn);

    return true;
}

static bool _rf_yllight_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0;
	cl_yllight_info_t* yl = &(slave->dev_info.rf_stat.dev_priv_data.yllight_info);
	
    switch (info->action) {
        case ACT_RF_YLLIGHT_MOD:
			{
				u_int8_t request = (u_int8_t)cci_u32_data(info);

				tlv->type = TLV_YLLIGHT_MODE_SET;
				tlv->len = sizeof(request);

				if (request > 1) {
					goto err_out;
				}

				memcpy((u_int8_t*)&tlv[1], &request, sizeof(request));

				if (request != yl->mode) {					
					len = tlv->len + sizeof(*tlv);
					yl->mode = request;
				}
			}
			break;

		case ACT_RF_YLLIGHT_SOUND:
			{
				u_int8_t request = (u_int8_t)cci_u32_data(info);

				if (request > 1) {
					goto err_out;
				}

				memcpy((u_int8_t*)&tlv[1], &request, sizeof(request));

				tlv->type = TLV_YLLIGHT_SOUND;
				tlv->len = sizeof(request);
				len = tlv->len + sizeof(*tlv);
			}
			break;

		case ACT_RF_YLLIGHT_ALARM_CONFIG:
			{
				cl_yllight_alarm_config_t *src = cci_pointer_data(info);
				tlv->type = TLV_YLLIGHT_ALARM_CONFIG;
				tlv->len = sizeof(*src);

				// 本地先修改
				yl->alarm_mode = src->alarm_mode;
				yl->on_time = src->on_time;
				yl->off_time = src->off_time;
				yl->total_time = src->total_time;
				
				src->off_time = ntohs(src->off_time);
				src->total_time = ntohs(src->total_time);

				memcpy(&tlv[1], src, sizeof(*src));

				len = tlv->len + sizeof(*tlv);
			}
			break;

		case ACT_RF_YLLIGHT_LAMP_CTRL:
			{	
				cl_rf_lamp_stat_t* src = cci_pointer_data(info);
				ucp_yllight_stat_t *dest = (ucp_yllight_stat_t *)&tlv[1];
				tlv->type = TLV_YLLIGHT_LAMP_SET;
				tlv->len = sizeof(*dest);

				len = sizeof(*dest) + sizeof(*tlv);
            
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

				if (dest->C == 2)
					dest->C = 1;

				if (dest->W == 2)
					dest->W = 1;
	            
	            dest->mod_id = src->mod_id;
	            dest->power = src->power;
	     //       dest->sub_cmd = SC_SET;
//	            dest->type = 0x0;
	            dest->hwconf = src->ctrl_mode;

				if ( yl->lamp_stat.lamp_type == RL_LT_LAYER ) {
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
				
				memcpy((u_int8_t*)&tlv[1], dest, sizeof(*dest));

				// 先修改内存
				memcpy((u_int8_t*)&yl->lamp_stat, (u_int8_t*)dest, sizeof(*dest));
			}
			break;
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;

err_out:
	*ret = RS_INVALID_PARAM;

	 return  false;
}

/////////////////////////////////////////////////////////////////
// 电王科技灯
static bool _rf_dwkj_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	dwkj_cache_t *cache = (dwkj_cache_t *)param;
    cl_dwkj_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.dwkj_info);
//	cl_dwkj_stat_t *stat;

	if (len < sizeof(dwkj_cache_t)) {
		log_debug("%s err len=%u need %d\n", __FUNCTION__, len, sizeof(dwkj_cache_t));
		return false;
	}
	
	info->stat.onoff = cache->onoff;
	info->stat.vol = ntohs(cache->vol);
	info->stat.current = ntohs(cache->current);
	info->stat.power = ntohs(cache->power);
	info->stat.percent = cache->percent;
	info->stat.degree = ntohl(cache->degree);
	info->stat.error = cache->error;

	log_debug("dwkj cache: onoff %u vol %u current %u power %u percent %u degree %u\n" ,
		info->stat.onoff, info->stat.vol, info->stat.current, info->stat.power, info->stat.percent, info->stat.degree);

	if (info->stat.v_timer != cache->v_timer) {
		log_debug("update v_timer %u => %u\n", info->stat.v_timer, cache->v_timer);
		info->stat.v_timer = cache->v_timer;
		rf_dev_query_tlv_info(slave, RF_DEV_DWKJ_TT_TYPE_QUERY_TIMER_POINT, 0, 0);
	}
	

	log_debug("_rf_dwkj_update_cache_data onoff %u\n", cache->onoff);

	return true;
}

#define ONE_DAY_5_MIN	((60*24)/5)
static bool _rf_dwkj_update_data(slave_t* slave, rf_tlv_t* tlv, u_int8_t action)
{
    cl_dwkj_info_t* info;
	ucp_dwkj_timer_t *timer;
	int i;
	u_int16_t point;
    
    info = &(slave->dev_info.rf_stat.dev_priv_data.dwkj_info);
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    log_debug("_rf_dwkj_update_data &&&&&& tlv->type[%u] tlv->len[%u]\n",tlv->type,tlv->len);
    
    switch (tlv->type) {
		case RF_DEV_DWKJ_TT_TYPE_SET_TIMER_POINT:
			{
				int event = SAE_DWKJ_SET_TIMER_OK;
				u_int8_t *err = (u_int8_t *)rf_tlv_val(tlv);

				log_debug("RF_DEV_DWKJ_TT_TYPE_SET_TIMER_POINT reply\n");

				if (tlv->len < sizeof(*err)) {
					log_err(false, "invalid dwkj set timer reply len %u\n", tlv->len);
					return false;
				}

				if (*err) {
					event = SAE_DWKJ_SET_TIMER_FAILD;
					
					// 还原之前					
					memcpy((u_int8_t*)&info->timer, (u_int8_t*)&info->timer_bk, sizeof(info->timer));
				}

				log_debug("dwkj set timer reply, err %u\n", *err);
				
				event_push(slave->user->callback, event, slave->handle, slave->user->callback_handle);
			}
			
			break;
        case RF_DEV_DWKJ_TT_TYPE_QUERY_TIMER_POINT:
			timer = rf_tlv_val(tlv);

			if (tlv->len < sizeof(*timer)) {
				log_err(false, "invalid timer len %u\n", tlv->len);
				return false;
			}

			info->timer.onoff = timer->onoff;
			info->timer.count = timer->count;

			log_debug("dwkj update timer onoff %u count %u\n", info->timer.onoff, timer->count);
			for (i = 0; i < 12; i++) {
				point = ntohs(timer->point[i]);

				// 需要转换下时区
				info->timer.item[i].point = point & 0x1FF;

#ifdef USE_TIME_MINS
				info->timer.item[i].point =  (info->timer.item[i].point + cl_priv->time_diff / 5) % ONE_DAY_5_MIN;
#else
				info->timer.item[i].point =  (info->timer.item[i].point + cl_priv->timezone * 60 / 5) % ONE_DAY_5_MIN;
#endif
				
				info->timer.item[i].level =  (point >> 9) & 0x7F;

				log_debug("dwkj update timer: point %u level %u [%u]\n", 
					info->timer.item[i].point, info->timer.item[i].level, point);
			}
			

            break;
        default:
            return false;
            break;
    }
    
    return true;
}

static int dwkj_send_timer_request(slave_t *slave, u_int8_t *out, int out_len)
{
	rf_tlv_t *tlv = (rf_tlv_t *)out;
	ucp_dwkj_timer_t *output_timer = (ucp_dwkj_timer_t *)&tlv[1];
	u_int16_t i, point, point1, inv;
	
	cl_dwkj_info_t *priv_info = &slave->dev_info.rf_stat.dev_priv_data.dwkj_info;
	
	tlv->type = RF_DEV_DWKJ_TT_TYPE_SET_TIMER_POINT;
	tlv->len = sizeof(*output_timer);

	output_timer->onoff = priv_info->timer.onoff;
	output_timer->count = priv_info->timer.count;
	
	for (i = 0; i < 12; i++) {
		// 需要转成UTC的时间
		point1 = priv_info->timer.item[i].point;
#ifdef USE_TIME_MINS
		inv = cl_priv->time_diff / 5;
#else
		inv = cl_priv->timezone * 60 / 5;
#endif

		if (point1 >= inv) {
			point1 -= inv;
		} else {
			point1 = point1 + ONE_DAY_5_MIN - inv;
		}
		
		point = (point1 & 0x1FF) | (priv_info->timer.item[i].level << 9 & 0XFE00);

		output_timer->point[i] = ntohs(point);
	}

	return tlv->len + sizeof(*tlv);
}

static int dwkj_build_tt_uart_send(u_int8_t *out, int out_len, u_int8_t cmd, u_int8_t *param, u_int8_t param_len)
{
	rf_tlv_t* tlv = (rf_tlv_t*)out;
	dwkj_uart_hdr_t *hd;
	u_int8_t *tail;

	tlv->type = RF_TT_CMD_RAWDATA;
	tlv->len = sizeof(*hd) + sizeof(*tail) + param_len;
	
	hd = (dwkj_uart_hdr_t *)rf_tlv_val(tlv);

	hd->syn = 0xa8;
	hd->len = tlv->len - 2;
	hd->cmd = cmd;

	if (param_len > 0) {
		memcpy(&hd[1], param, param_len);
	}

	tail = (u_int8_t*)&hd[1] + param_len;
	
	*tail = 0;

	return sizeof(*tlv) + sizeof(*hd) + sizeof(*tail) + param_len;
}

static bool _rf_dwkj_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
   	u_int8_t buf[256] = {0}, param[4] = {0}, cmd;
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0, i = 0;
	cl_dwkj_info_t *priv_info = &slave->dev_info.rf_stat.dev_priv_data.dwkj_info;

    switch (info->action) {
		case ACT_DWKJ_ONOFF:
			{
				priv_info->stat.onoff = (u_int8_t)cci_u32_data(info);
				if (priv_info->stat.onoff == 0) {
					cmd = 2;
				} else {
					cmd = 1;
				}

				len = dwkj_build_tt_uart_send(buf, sizeof(buf), cmd, param, sizeof(param));
			}
			break;

		case ACT_DWKJ_PERCENT:
			{
				param[3] = (u_int8_t)cci_u32_data(info);
				priv_info->stat.percent = param[3];
				
				len = dwkj_build_tt_uart_send(buf, sizeof(buf), 3, param, sizeof(param));
			}
			break;

		case ACT_DWKJ_TIMER:
			{				
				cl_dwkj_timer_t *input_timer = (cl_dwkj_timer_t *)cci_pointer_data(info);

				if (info->data_len < sizeof(*input_timer)) {
					log_err(false, "invalid tlv len %u\n", tlv->len);
					return false;
				}

				// 备份一下
				memcpy((u_int8_t*)&priv_info->timer_bk, (u_int8_t*)&priv_info->timer, sizeof(priv_info->timer));

				memcpy((u_int8_t*)&priv_info->timer, input_timer, sizeof(priv_info->timer));

				len = dwkj_send_timer_request(slave, buf, sizeof(buf));
			}
			break;
			
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave, buf, len);
    }
    
    return true;
}

static bool _rf_dwkj_proc_group_notify(user_t* user,u_int8_t group_id,u_int8_t group_type,
                                      u_int8_t ctrl_type,u_int8_t ctrl_value, RS *ret)
{
    char buf[1024] = {0};
    int t_len, len;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_group_tt_t *up = (net_rfgw_group_tt_t*)(uo + 1);
    u_int8_t* p = (u_int8_t*)(up + 1);
    u_int8_t param[4] = {0};
	
    if (group_id == 0) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
	switch (ctrl_type) {
		case ACT_DWKJ_ONOFF:
			len = dwkj_build_tt_uart_send(p, sizeof(buf), ctrl_value == 0 ? 2 : 1, param, sizeof(param));
			break;
		case ACT_DWKJ_PERCENT:	
			param[3] = ctrl_value;
			len = dwkj_build_tt_uart_send(p, sizeof(buf), 3, param, sizeof(param));
			break;
		default:
			*ret = RS_INVALID_PARAM;
			return false;
	}
    
    up->sub_type = IJ_RFGW;
    up->ext_type = RF_EXT_TYPE_DWKJ;
    up->group_id = group_id;
    up->len = len;
   
    t_len = sizeof(*up) + up->len;
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP_TT, (u_int16_t)sizeof(ucp_obj_t) + t_len);
    sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t) + t_len);
    
    return true;
}


/////////////////////////////////////////////////////////////////
// 门锁
// 更新友泰cache数据
static bool _rf_yt_door_lock_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_door_lock_info_t *pinfo = &(slave->dev_info.rf_stat.dev_priv_data.door_lock_info);
	yt_door_lock_tt_cache_t *cache = (yt_door_lock_tt_cache_t *)param;

	log_debug("_rf_yt_door_lock_update_cache_data\n");
	
	if (len < sizeof(*cache)) {
		return false;
	}

	cache->flagbits = ntohl(cache->flagbits);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	/*
		
		
		0	开关门状态	门关	门开
		1	电池电量报警	电量充足	电量低报警
		2			
		3	是否布防	未布防	已布防
		4	电量显示百分比or格数	百分比	格数
		5			
		6			
		7			
		8	是否已经设置密码	未设置	已设置
		
	*/
	pinfo->stat.is_door_open = !!(cache->flagbits & BIT(0));
	pinfo->stat.is_battary_warn = !!(cache->flagbits & BIT(1));
	//pinfo->stat.is_break_lock = !!(cache->flagbits & BIT(2));
	pinfo->stat.is_guard = !!(cache->flagbits & BIT(3));
	pinfo->stat.is_new_battary_show = !!(cache->flagbits & BIT(4));
	//pinfo->stat.is_look_open = !!(cache->flagbits & BIT(6));
	
	pinfo->stat.has_open_passwd = !!(cache->flagbits & BIT(8));
	//pinfo->stat.has_limit_fault = !!(cache->flagbits & BIT(9));
	//pinfo->stat.has_moto_fault = !!(cache->flagbits & BIT(10));
	//pinfo->stat.has_unlock_timeout = !!(cache->flagbits & BIT(11));

	pinfo->stat.battary = cache->battery;
	if (pinfo->stat.battary < 5) {
		pinfo->stat.battary = 5;
	}

	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->max_hislog_count));

	if (pinfo->stat.alarm_index != cache->v_alarm) {
		pinfo->stat.alarm_index = cache->v_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}
	
	pinfo->stat.has_get_stat = true;

	return true;
}

static void _rf_door_lock_dump_cache(u_int64_t sn, door_lock_tt_cache_t *cache)
{
	time_t t = time(NULL);
	
	log_debug("%s: %"PRIu64" door lock cache: battery %u v_wifilock %u lan_num %u\n", ctime(&t), sn, cache->battery, cache->v_wifilock, cache->lan_num);
}

static bool _rf_door_lock_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_door_lock_info_t *pinfo = &(slave->dev_info.rf_stat.dev_priv_data.door_lock_info);
	door_lock_tt_cache_t *cache = (door_lock_tt_cache_t *)param;
	int i;
	u_int8_t controller_cnt, last_controller_id;
	ucp_rf_door_com_ctrl_t dc;

	log_debug("_rf_door_lock_update_cache_data\n");
	
	if (len < sizeof(*cache)) {
		return false;
	}

	cache->flagbits = ntohl(cache->flagbits);
//	cache->hislog_index_invalid = ntohl(cache->hislog_index_invalid);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	_rf_door_lock_dump_cache(slave->sn, cache);
	/*
		
		bit 说明	清0 置1
		0	开关门状态	门关	门开
		1	电池电量报警	电量充足	电量低报警
		2	是否撬门	未撬门	撬门
		3	是否布防	未布防	已布防
		4			
		5			
		6	开关锁状态	锁关	锁开
		7			
		8			
		9	限位阀故障	正常	有故障
		10	电机故障	正常	有故障
		11	门开超时报警	未超时	超时报警
	*/
	pinfo->stat.is_door_open = !!(cache->flagbits & BIT(0));
	pinfo->stat.is_battary_warn = !!(cache->flagbits & BIT(1));
	pinfo->stat.is_break_lock = !!(cache->flagbits & BIT(2));
	pinfo->stat.is_guard = !!(cache->flagbits & BIT(3));
	pinfo->stat.is_new_battary_show = !!(cache->flagbits & BIT(4));
	pinfo->stat.is_look_open = !!(cache->flagbits & BIT(6));
	pinfo->stat.has_limit_fault = !!(cache->flagbits & BIT(9));
	pinfo->stat.has_moto_fault = !!(cache->flagbits & BIT(10));
	pinfo->stat.has_unlock_timeout = !!(cache->flagbits & BIT(11));

	pinfo->stat.battary = cache->battery;
	if (pinfo->stat.battary < 5) {
		pinfo->stat.battary = 5;
	}
	pinfo->stat.unlock_timeout_enable = cache->open_timeout_en;
	pinfo->stat.unlock_timeout = cache->open_timeout_v;

	if (pinfo->stat.wifilock_index != cache->v_wifilock) {
		pinfo->stat.wifilock_index = cache->v_wifilock;
		rf_dev_query_tlv_info(slave, DOOR_LOCK_PRIV_TYPE_QUERY_WIFI_LOCK, NULL, 0);
	}

	if (pinfo->stat.alarm_index != cache->v_alarm) {
		pinfo->stat.alarm_index = cache->v_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}

	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->max_hislog_count));

	// 遥控器按键按下
	last_controller_id = (cache->controller >> 4) & 0x0f;
	// 在已经获取到一次cache信息情况下，遥控器按键ID有变化，认为当前按下了遥控器
	if (pinfo->stat.has_get_stat == true && pinfo->stat.cid_index != cache->v_cid) {
		log_debug("cid %u => %u, update last controller id %u => %u\n", pinfo->stat.cid_index, cache->v_cid, pinfo->stat.last_controller_id, last_controller_id);
		pinfo->stat.last_controller_id = last_controller_id;
		event_push(slave->user->callback, SAE_RF_DEV_DOOR_LOCK_CONTROLLER, slave->handle, slave->user->callback_handle);
	}
	pinfo->stat.cid_index = cache->v_cid;
	

	// 遥控器信息查询	
	controller_cnt = min(((cache->controller) & 0x0f), 5);
	pinfo->controller_num = controller_cnt;
	log_debug("door lock have %u controller\n", controller_cnt);
	for (i = 0; i < controller_cnt; i++) {
		if (cache->controller_idx[i] != pinfo->stat.controller_index[i]) {
			log_debug("controller %u need query\n", i + 1);
			pinfo->stat.controller_index[i] = cache->controller_idx[i];
			dc.value = i + 1;
			dc.pad = 0;
			rf_dev_query_tlv_info(slave, DOOR_LOCK_PRIV_TYPE_QUERY_CONTROLLER_INFO, (u_int8_t*)&dc, sizeof(dc));
		}
	}

	pinfo->stat.has_get_stat = true;

	return true;
}

static void _rf_door_lock_instert_his(cl_door_lock_info_t* rd,u_int8_t action , u_int32_t time)
{
    int i ;
    cl_rf_door_history_t* dh,*pd;
    u_int8_t type,value;
    u_int32_t temp_time;
    
    if(!(action & BIT(0)) || time == 0){
        return;
    }
    
    type = (action >>2) & 0x3F;
    value  = (action >>1) & 0x1;
    dh = &rd->his[0];
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (dh->is_valid && dh->time_stamp == time && dh->info_type == type && dh->value == value) {
            return;
        }
    }
    
    dh = &rd->his[0];
    pd = NULL;
    temp_time = 0xFFFFFFFF;
    
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (!dh->is_valid ) {
            dh->is_valid = true;
            dh->info_type = type;
            dh->value  = value;
            dh->time_stamp = time;
            break;
        }
        
        if (dh->is_valid && dh->time_stamp < temp_time) {
            temp_time = dh->time_stamp;
            pd = dh;
        }
    }
    
    if (i >= sizeof(rd->his)/sizeof(cl_rf_door_history_t) && pd != NULL) {
        pd->is_valid = true;
        pd->info_type = type;
        pd->value  = value;
        pd->time_stamp = time;
    }
}

#if 0
static void _rf_door_lock_update_history(cl_door_lock_info_t* rd,ucp_rf_door_history_t* uh)
{
    uh->time_1 = ntohl(uh->time_1);
    uh->time_2 = ntohl(uh->time_2);
    uh->time_3 = ntohl(uh->time_3);
    uh->time_4 = ntohl(uh->time_4);
    
    _rf_door_lock_instert_his(rd,uh->on_off1,uh->time_1);
    _rf_door_lock_instert_his(rd,uh->on_off2,uh->time_2);
    _rf_door_lock_instert_his(rd,uh->on_off3,uh->time_3);
    _rf_door_lock_instert_his(rd,uh->on_off4,uh->time_4);
    
}

static void _rf_door_lock_update_history_1(cl_rf_door_history_t* hs, int n, ucp_rf_hislog_info_t *uh)
{
    int i ;
    cl_rf_door_history_t* dh, *pd;
    u_int8_t type, value, ex_value;
    u_int32_t temp_time, time;
    
    type = uh->type;
    value  = uh->value;
	time = ntohl(uh->timestapm);
	ex_value = uh->ex_value;

	if (time == 0) {
		return;
	}

	log_debug("get door lock new history: type %u value %u time %u ex_type %u ex_value %u\n", 
		type, value, time, ex_type, ex_value);
	
    dh = hs;
    for (i = 0; i < n; i++,dh++) {
        if (dh->is_valid && dh->time_stamp == time && dh->info_type == type && dh->value == value &&
			dh->ex_type == ex_type && dh->ex_value == ex_value) {
            return;
        }
    }
    
    dh = hs;
    pd = NULL;
    temp_time = 0xFFFFFFFF;
    
    for (i = 0; i < n; i++,dh++) {
        if (!dh->is_valid ) {
            dh->is_valid = true;
            dh->info_type = type;
            dh->value  = value;
            dh->time_stamp = time;
			dh->ex_type = ex_type;
			dh->ex_value = ex_value;
            break;
        }
        
        if (dh->is_valid && dh->time_stamp < temp_time) {
            temp_time = dh->time_stamp;
            pd = dh;
        }
    }
    
    if (i >= n && pd != NULL) {
        pd->is_valid = true;
        pd->info_type = type;
        pd->value  = value;
        pd->time_stamp = time;
		pd->ex_type = ex_type;
		pd->ex_value = ex_value;
    }
    
}
#endif

static void door_lock_alarm_update_idx(slave_t *slave, u_int8_t index_alarm)
{
    cl_door_lock_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_lock_info);

	rd->stat.alarm_index = index_alarm;
}


static bool _rf_door_lock_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_rf_door_lock_stat_t* us;
    cl_door_lock_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_lock_info);
    bool alarm_to_app = false;
    ucp_yt_lock_ctrl_resp_t* rsp;
    u_int32_t event = 0;

	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
   	log_debug("_rf_door_lock_update_data tlv->type=%u\n", tlv->type);
		
    switch (tlv->type) {
		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, door_lock_alarm_update_idx);
			
        case UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY:
            if (tlv->len >= sizeof(*us)) {
                us = rf_tlv_val(tlv);
                us->flag_bits = ntohl(us->flag_bits);
				
				rd->stat.unlock_timeout = us->unlock_timeout;
				rd->stat.unlock_timeout_enable = us->unlock_timeout_enable;
				
				log_debug("_rf_door_lock_update_data us->flag_bits=%02x timeout %u enable %u\n", us->flag_bits, rd->stat.unlock_timeout, rd->stat.unlock_timeout_enable);
                //报警
                if (rd->stat.is_guard && !!(us->flag_bits & BIT(3))) {
                    if(!rd->stat.is_door_open && !!(us->flag_bits & BIT(0))){
                        rd->alarm_info.info_type = 0;
                        rd->alarm_info.value = !!(us->flag_bits & BIT(0));
                        rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
                        alarm_to_app = true;
                    }
                    
                    if (!rd->stat.has_break_door && !!(us->flag_bits & BIT(2)) ) {
                        rd->alarm_info.info_type = 0x1;
                        rd->alarm_info.value = !!(us->flag_bits & BIT(2));
                        rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
                        alarm_to_app = true;
                    }
                    
                    if (!rd->stat.is_break_lock && !!(us->flag_bits & BIT(7)) ) {
                        rd->alarm_info.info_type = 0x2;
                        rd->alarm_info.value = !!(us->flag_bits & BIT(7));
                        rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
                        alarm_to_app = true;
                    }

					if (!rd->stat.has_unlock_timeout && !!(us->flag_bits & BIT(11)) ) {
                        rd->alarm_info.info_type = 0x3;
                        rd->alarm_info.value = !!(us->flag_bits & BIT(7));
                        rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
                        alarm_to_app = true;
                    }
                }
                
                rd->stat.battary = us->battery;
				if (rd->stat.battary < 5) {
					rd->stat.battary = 5;
				}
                rd->stat.is_door_open = !!(us->flag_bits & BIT(0));
                rd->stat.is_battary_warn = !!(us->flag_bits & BIT(1));
                rd->stat.has_break_door = !!(us->flag_bits & BIT(2));
                rd->stat.is_guard = !!(us->flag_bits & BIT(3));
                rd->stat.is_look_open = !!(us->flag_bits & BIT(6));
                rd->stat.is_break_lock = !!(us->flag_bits & BIT(7));
                rd->stat.has_open_passwd = !!(us->flag_bits & BIT(8));

				rd->stat.has_limit_fault = !!(us->flag_bits & BIT(9));
				rd->stat.has_moto_fault = !!(us->flag_bits & BIT(10));
				rd->stat.has_unlock_timeout = !!(us->flag_bits & BIT(11));		
            }
            
            break;
#if 0
        case UP_TLV_SMTDLOCK_CMD_PUSH_ONOFFTIME:
            
			if (tlv->len == sizeof(*uh)) {
				uh = rf_tlv_val(tlv);
	            _rf_door_lock_update_history(rd,uh);
			} else if ((tlv->len % sizeof(*rh)) == 0 && (n = tlv->len/sizeof(*rh)) > 0) {
				rh = rf_tlv_val(tlv);
				for (i = 0; i < n; i++) {
					_rf_door_lock_update_history_1(rd->his, sizeof(rd->his)/sizeof(rd->his[0]), rh++);
				}
			}
            break;
#endif			
        case UP_TLV_SMTDLOCK_CMD_PUSH_RTCTRLID:
            break;
        case UP_TLV_SMTDLOCK_CMD_PUSH_RTCTRL_INFO:
            break;
        case UP_TLV_YT_LOCK_CMD_CTRL_LOCK://友泰门锁控制反馈
            if(tlv->len >=sizeof(ucp_yt_lock_ctrl_resp_t)){
                rsp = rf_tlv_val(tlv);				
				log_debug("UP_TLV_YT_LOCK_CMD_CTRL_LOCK action %u err %u\n", rsp->action, rsp->error);
                if (rsp->error == ERR_NONE) {
                    if(rsp->action <= 1){
                        event = SAE_YT_LOCK_CTRL_OK;
                    }else if(rsp->action == 0x2){
                        event = SAE_YT_LOCK_SET_PASSWD_OK;
                    }else{
                        event = SAE_YT_LOCK_CHANGE_PASSWD_OK;
                    }
                }else if(rsp->error == 0x1){
                    event = SAE_YT_LOCK_CTRL_PASSWD_ERR;
                }else if(rsp->error == 0x2){
                    event = SAE_YT_LOCK_PASSWD_ERR;
                }else if(rsp->error == 0x3){
                    event = SAE_YT_LOCK_CTRL_NO_PASSWD;
                }
            }
            if (event != 0) {
                event_push(slave->user->callback, event, slave->handle, slave->user->callback_handle);
            }
            return false;
            break;
		case DOOR_LOCK_PRIV_TYPE_QUERY_WIFI_LOCK:
			{				
				cl_door_lock_wifilock_t *reply = rf_tlv_val(tlv);

				if (tlv->len < sizeof(*reply)) {
					return false;
				}

				memcpy(&rd->wifilock, reply, sizeof(rd->wifilock));

				log_debug("door get wifi lock stat: on_enable %u off_enable %u\n", rd->wifilock.don_enable, rd->wifilock.doff_enable);
			}
			break;

		case RF_TT_CMD_QUERY_HISLOG:
			return rf_update_comm_history_item(slave, rf_tlv_val(tlv), tlv->len);
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);

		case DOOR_LOCK_PRIV_TYPE_QUERY_CONTROLLER_INFO:
			{					
				cl_door_lock_controller_info_t *reply = rf_tlv_val(tlv);
				u_int8_t id_idx;

				if (tlv->len < sizeof(*reply)) {
					return false;
				}

				if (reply->id == 0 || reply->id > 5) {
					log_err(false, "invalid door lock id %u\n", reply->id);
					return false;
				}

				id_idx = reply->id - 1;

				memcpy(&rd->controller[id_idx], reply, sizeof(*reply));

				log_debug("get reply controller: id %u stat 0x%x name[%s]\n", reply->id, reply->state, reply->name);
			}
			
			break;
		
        default:
            return false;
            break;
    }
    
    if (alarm_to_app) {
        event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
        event_push(slave->user->callback, SAE_RF_DEV_ALARM_INFO, slave->handle, slave->user->callback_handle);
        return false;
    }
    
    return true;
}

static u_int8_t _rf_door_lock_stat_query_pkt(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_rf_door_lock_q_t* lq = (ucp_rf_door_lock_q_t*)(tlv+1);
    
    memset(lq, 0, sizeof(*lq));
    lq->type = 0x0;//0xFF;
    tlv->type = UP_TLV_SMTDLOCK_CMD_GET_SUMMARY;
    tlv->len = (u_int8_t)sizeof(*lq);
    
    return tlv->len;
}

static bool _rf_proc_manage_query_his_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
	cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    u_int32_t value;
    int len = 0;
	u_int32_t now = (u_int32_t)time(NULL);
	u_int32_t his_max_timestamp = 0;
    
    tlv->len = sizeof(u_int32_t);
    tlv->type = UP_TLV_SMTDLOCK_CMD_GET_ONOFFTIME;
    value = cci_u32_data(info);

	his_max_timestamp = rf_dorr_manage_get_max_timestamp(rd);
	//如果比历史记录还小就不查询了
	if (value < his_max_timestamp) {
		event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
		return true;
	} else if (value >= his_max_timestamp) {
		value++;
	}

	//判断下发送计数
	if (rd->send_num > 0) {
		log_debug("%s rd->send_num=%u\n", __FUNCTION__, rd->send_num);
		if (now < (rd->send_timeout + 30)) {
			log_debug("rd->send_timeout=%u now=%u *************************\n", rd->send_timeout, now);
			return true;
		}
	}
	
    value = htonl(value);
    *((u_int32_t*)(tlv+1)) = value;
    
    len = sizeof(*tlv) + tlv->len;
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }

	rd->send_num++;
	rd->send_timeout = now;
    
    return true;
}

static bool _rf_proc_query_his_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    u_int32_t value;
    int len = 0;

	//这里做下兼容处理，只有门磁，门锁才优化处理
	switch(slave->ext_type) {
	case RF_EXT_TYPE_DOOR_LOCK:
	case RF_EXT_TYPE_YT_DOOR_LOCK:
	case RF_EXT_TYPE_DOOR_MAGNET:
	case RF_EXT_TYPE_HM_MAGENT:
	case RF_EXT_TYPE_DOOR_MAGNETV2:
	case RF_EXT_TYPE_YLLOCK:
		return _rf_proc_manage_query_his_notify(slave, info, ret);
	default:
		break;
	}
	
    tlv->len = sizeof(u_int32_t);
    tlv->type = UP_TLV_SMTDLOCK_CMD_GET_ONOFFTIME;
    value = cci_u32_data(info);
    value = htonl(value);
    *((u_int32_t*)(tlv+1)) = value;
    
    len = sizeof(*tlv) + tlv->len;
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}


static bool _rf_door_lock_proc_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_rf_door_com_ctrl_t * dc = (ucp_rf_door_com_ctrl_t*)(tlv+1);
    ucp_yt_rf_door_lock_ctrl_t* yc = (ucp_yt_rf_door_lock_ctrl_t*)(tlv+1);
    cl_door_lock_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_lock_info);
    int len = 0,type;
    u_int32_t value;
    u_int32_t old_pass,new_pass;
    u_int8_t onoff;
    u_int64_t u64_value;
    
    switch (info->action) {
        case ACT_RF_DOOR_LOCK_CTRL:
            tlv->len = sizeof(*dc);
            value = cci_u32_data(info);
            type = (value >> 16)&0xFF;
            
            switch (type) {
                case RDL_CTRL_OPEN:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_ONOFF_DOORLOCK;
                    dc->value = !!(value & 0xFF);
                    tlv->len = sizeof(*dc);
                    len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存值
					rd->stat.is_look_open = dc->value; 
                    break;
                case RDL_CTRL_GRARD:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                    dc->value = !!(value & 0xFF);
                    tlv->len = sizeof(*dc);
                    len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存值
					rd->stat.is_guard = dc->value; 
                    break;
                    
                default:
                    len = 0;
                    break;
            }
            break;
        case ACT_RF_DOOR_LOCK_REMOTE_CTRL:
            break;
        case ACT_RF_QUERY_HISTORY:
            return _rf_proc_query_his_notify(slave,info,ret);
            break;
        case ACT_YT_RF_DOOR_LOCK_CTRL:
            tlv->len = sizeof(*yc);
            u64_value = cci_u64_data(info);
            
    
            tlv->type = UP_TLV_YT_LOCK_CMD_CTRL_LOCK;
            onoff = (u_int8_t)((u64_value >> 32) & 0xFF);
            new_pass = (u_int32_t)(u64_value & 0xFFFFFFFF);
            
#if 0
            if (new_pass == 0) {
                *ret = RS_INVALID_PARAM;
                return false;
            }
#endif

            yc->action = !! onoff;
            yc->old_pass = htonl(new_pass);
            
            rd->stat.is_look_open = !!onoff;
            
            len = sizeof(*tlv) + sizeof(*yc);
            break;
        case ACT_YT_RF_DOOR_LOCK_MODIFY_PASSWD:
            tlv->len = sizeof(*yc);
            u64_value = cci_u64_data(info);
            
            tlv->type = UP_TLV_YT_LOCK_CMD_CTRL_LOCK;
            old_pass = (u_int32_t)((u64_value >> 32) & 0xFFFFFFFF);
            new_pass = (u_int32_t)(u64_value & 0xFFFFFFFF);
            
            yc->action = 0x3;
            yc->cur_pass = htonl(new_pass);
            yc->old_pass = htonl(old_pass);
            
            len = sizeof(*tlv) + sizeof(*yc);
            
            break;
		case ACT_YT_RF_DOOR_LOCK_CREATE_PASSWD:
			tlv->len = sizeof(*yc);
            u64_value = cci_u64_data(info);
            
            tlv->type = UP_TLV_YT_LOCK_CMD_CTRL_LOCK;
            old_pass = (u_int32_t)((u64_value >> 32) & 0xFFFFFFFF);
            new_pass = (u_int32_t)(u64_value & 0xFFFFFFFF);
            
            yc->action = 0x2;
            yc->cur_pass = htonl(new_pass);
            yc->old_pass = htonl(old_pass);
			
			len = sizeof(*tlv) + sizeof(*yc);
			break;
		case ACT_RF_DOOR_LOCK_UNLOCK_TIMEOUT:
			{
				ucp_door_unlock_timeout_set_t request;
				u_int16_t value = cci_u16_data(info);
				request.enable = (value >> 8) & 0xff;
				request.timeout = value & 0xff;

				tlv->type = DOOR_LOCK_PRIV_TYPE_SET_UNLOCK_TIMEOUT;
				tlv->len = sizeof(request);
				memcpy(rf_tlv_val(tlv), &request, sizeof(request));

				// 先修改内存值
				rd->stat.unlock_timeout_enable = request.enable;
				rd->stat.unlock_timeout = request.timeout;

				len = tlv->len + sizeof(*tlv);
			}
			
			break;
		case ACT_RF_DOOR_LOCK_WIFI_LOCK:
			{
				ucp_door_wifi_lock_set_t request;
				u_int32_t value = cci_u32_data(info);
				request.type = (value >> 24) & 0xff;
				request.enable = (value >> 16) & 0xff;
				request.starthour = (value >> 8) & 0xff;
				request.endhour = (value) & 0xff;

				tlv->type = DOOR_LOCK_PRIV_TYPE_SET_WIFI_LOCK;
				tlv->len = sizeof(request);
				memcpy(rf_tlv_val(tlv), &request, sizeof(request));

				len = tlv->len + sizeof(*tlv);

				// 先修改内存值
				if (request.type == 1) {
					rd->wifilock.don_enable = request.type;
					rd->wifilock.don_starthour = request.starthour;
					rd->wifilock.don_endhour = request.endhour;
				} else {
					rd->wifilock.doff_enable = request.type;
					rd->wifilock.doff_starthour = request.starthour;
					rd->wifilock.doff_endhour = request.endhour;
				}
				
			}
		case ACT_RF_DOOR_LOCK_SET_CONTROLLER_INFO:
			{
				cl_door_lock_controller_set_t *request = cci_pointer_data(info);
				if (request->id == 0 || request->id > 5) {
					break;
				}
				
				tlv->type = DOOR_LOCK_PRIV_TYPE_SET_CONTROLLER_INFO;
				tlv->len = (u_int8_t)info->data_len;

				memcpy((u_int8_t*)&tlv[1], cci_pointer_data(info), info->data_len);
				len = sizeof(*tlv) + tlv->len;

				// 先修改内存数据
				rd->controller[request->id - 1].id = request->id;
				rd->controller[request->id - 1].state = request->state;

				if (info->data_len == sizeof(*request)) {
					memcpy(rd->controller[request->id - 1].name, request->name, sizeof(rd->controller[request->id - 1].name));
				}
				
			}
			break;

		case ACT_YT_RF_DOOR_LOCK_ASSOCIATE:
			{
				tlv->type = DOOR_LOCK_PRIV_TYPE_ASSOCIATE;
				tlv->len = 0;
				
				len = tlv->len + sizeof(*tlv);
			}
			break;
			
        default:
            *ret =RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}

static bool _rf_door_lock_proc_group_notify(user_t* user,cln_common_info_t *info, RS *ret)
{
    char buf[1024] = {0};
    int t_len,type;
    u_int32_t value = cci_u32_data(info);
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_group_tt_t *up = (net_rfgw_group_tt_t*)(uo+1);
    rf_tlv_t* tlv = (rf_tlv_t*)(up+1);
    ucp_rf_door_com_ctrl_t * dc = (ucp_rf_door_com_ctrl_t*)(tlv+1);
    u_int8_t group = (value >> 24) & 0xFF;
    type = (value >> 16) & 0xFF;
    
    if (group == 0) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    up->sub_type = IJ_RFGW;
    up->ext_type = RF_EXT_TYPE_DOOR_LOCK;
    
    switch (info->action) {
        case ACT_RF_DOOR_LOCK_CTRL:
            switch (type) {
                case RDL_CTRL_OPEN:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_ONOFF_DOORLOCK;
                    dc->value = !!(value & 0xFF);
                    up->group_id = group;
                    tlv->len = sizeof(*dc);
                    up->len = sizeof(*tlv)+sizeof(*dc);
                    break;
                case RDL_CTRL_GRARD:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                    dc->value = !!(value & 0xFF);
                    up->group_id = group;
                    tlv->len = sizeof(*dc);
                    up->len = sizeof(*tlv)+sizeof(*dc);
                    break;
                    
                default:
                    *ret =RS_INVALID_PARAM;
                    return  false;
                    break;
            }
            break;
        default:
            *ret =RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    t_len = sizeof(*up)+up->len;
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP_TT, (u_int16_t)sizeof(ucp_obj_t)+t_len);
    sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+t_len);
    
    return true;
}

//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// 门磁
/*
1.海曼门磁（0x1e，0x27）市场上在售的门磁，还是使用老版本，不做调整
2.stm32 v1门磁（0x1e，0x24），市场上在售的门磁，只有设备端的版本大于等于1.11的才使用新的sdk。 小于的还是使用老的sdk方式
3.stm32 v2门磁（0x1e，0x2b）和友泰门锁（0x1e，0x26）、汇泰龙门锁（0x1e，0x31）都直接使用新的sdk。
*/
static bool slave_old_com(slave_t* slave)
{
	bool old = true;
	u_int32_t ver_slave;
	u_int32_t ver_min;
	ms_version_t *pver = NULL;

	switch(slave->ext_type) {
	case RF_EXT_TYPE_DOOR_MAGNETV2:
	case RF_EXT_TYPE_YT_DOOR_LOCK:
	case RF_EXT_TYPE_HTLLOCK:
		old = false;
		break;
	case RF_EXT_TYPE_DOOR_MAGNET:
		pver = &slave->dev_info.version.upgrade_version;
		ver_slave = BUILD_U32(pver->major, pver->minor, pver->revise, 0);
		ver_min = BUILD_U32(1, 11, 0, 0);
		if (ver_slave >= ver_min) {
			old = false;
		}
		break;
	}

	return old;
}

static void _rf_door_magnet_instert_his(slave_t* slave, cl_door_magnet_info_t* rd,u_int8_t action , u_int32_t time)
{
    int i ;
    cl_rf_door_history_t* dh,*pd;
    u_int8_t type,value;
    u_int32_t temp_time;
    
    if(!(action & BIT(0)) || time == 0){
        return;
    }
    
    type = (action >>2) & 0x3F;
    value  = (action >>1) & 0x1;


	//log_debug("enter %s time=%u\n", __FUNCTION__, time);
	//先不扔一样时间的,这里要做下兼容处理，老版本还是要以前的处理
	if (slave_old_com(slave)) {
	    dh = &rd->his[0];
	    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
	        if (dh->is_valid && dh->time_stamp == time && dh->info_type == type && dh->value == value) {
	            return;
	        }
	    }
	}

    dh = &rd->his[0];
    pd = NULL;
    temp_time = 0xFFFFFFFF;
    
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (!dh->is_valid ) {
            dh->is_valid = true;
            dh->info_type = type;
            dh->value  = value;
            dh->time_stamp = time;
            break;
        }
        
        if (dh->is_valid && dh->time_stamp < temp_time) {
            temp_time = dh->time_stamp;
            pd = dh;
        }
    }
    
    if (i >= sizeof(rd->his)/sizeof(cl_rf_door_history_t) && pd != NULL) {
        pd->is_valid = true;
        pd->info_type = type;
        pd->value  = value;
        pd->time_stamp = time;
    }
}

//更新智能门磁v2的缓存
static u_int32_t rf_dorr_manage_get_max_timestamp(cl_door_magnet_info_t* rd)
{
	int i;
	u_int32_t temp_time;
	cl_rf_door_history_t *dh;

	dh = &rd->his[0];
    temp_time = 0;

    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (!dh->is_valid ) {
            continue;
        }
        
        if (dh->time_stamp > temp_time) {
            temp_time = dh->time_stamp;
        }
    }

	return temp_time;
}

static void his_dump(cl_door_magnet_info_t* rd)
{
	int i;
	cl_rf_door_history_t* dh;

	dh = &rd->his[0];
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (dh->is_valid && dh->info_type == 0) {
			log_debug("index=%u value=%u infotype=%u timestamp=%u ext=%u exv=%u\n", 
				i, dh->value, dh->info_type, dh->time_stamp, dh->ex_type, dh->ex_value);
        }
    }	
	log_debug("\n\n");
}

static void _rf_door_magnet_update_history(slave_t* slave, cl_door_magnet_info_t* rd,ucp_rf_door_history_t* uh)
{	
    uh->time_1 = ntohl(uh->time_1);
    uh->time_2 = ntohl(uh->time_2);
    uh->time_3 = ntohl(uh->time_3);
    uh->time_4 = ntohl(uh->time_4);

	if (uh->time_1 >= rd->max_timestamp) {
		_rf_door_magnet_instert_his(slave, rd,uh->on_off1,uh->time_1);
	}
	if (uh->time_2 >= rd->max_timestamp) {
		_rf_door_magnet_instert_his(slave, rd,uh->on_off2,uh->time_2);
	}
	if (uh->time_3 >= rd->max_timestamp) {
		_rf_door_magnet_instert_his(slave, rd,uh->on_off3,uh->time_3);
	}
	if (uh->time_4 >= rd->max_timestamp) {
		_rf_door_magnet_instert_his(slave, rd,uh->on_off4,uh->time_4);
	}
	
	rd->max_timestamp = rf_dorr_manage_get_max_timestamp(rd);

	if (rd->send_num == 0) {
		return;
	}

	if ((uh->time_1 == 0) || (uh->time_2 == 0) ||
		(uh->time_3 == 0) || (uh->time_4 == 0)) {
		rd->send_num = 0;
		log_debug("the his recv over this time !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		//his_dump(rd);
	} else {
		//再往后延时
		rd->send_timeout = (u_int32_t)time(NULL);
	}

	//test
	//his_dump(rd);
}

static void door_magnet_alarm_update_idx(slave_t *slave, u_int8_t index_alarm)
{
    cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);

	log_debug("magnet alarm update alarm idex to %u\n", index_alarm);

	rd->index_alarm = index_alarm;
}

static bool _rf_door_magnet_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_rf_door_history_t* uh;
    ucp_rf_door_lock_stat_t* us;
    cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
    bool alarm_to_app = false;
    ucp_rf_auto_guard_stat_t* stat;
    
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    switch (tlv->type) {
		case RF_TT_CMD_QUERY_HISLOG:
			return rf_update_comm_history_item(slave, rf_tlv_val(tlv), tlv->len);
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
			
        case UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY:
            us = rf_tlv_val(tlv);
            us->flag_bits = ntohl(us->flag_bits);
            
            if (rd->stat.stat_valid == true && rd->stat.is_guard && !!(us->flag_bits & BIT(3))) {
                if(!rd->stat.is_door_open && !!(us->flag_bits & BIT(0))){
                    rd->alarm_info.info_type = 0;
                    rd->alarm_info.value = !!(us->flag_bits & BIT(0));
                    rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
					log_debug("door managet door open happend..\n");
                    alarm_to_app = true;
                }
                
                if (!rd->stat.is_break_door && !!(us->flag_bits & BIT(2)) ) {
                    rd->alarm_info.info_type = 0x1;
                    rd->alarm_info.value = !!(us->flag_bits & BIT(2));
                    rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
					log_debug("door managet break door happend..\n");
                    alarm_to_app = true;
                }
            }

			if (rd->stat.is_guard && !!(us->flag_bits & BIT(3)) &&
				!rd->stat.is_battary_warn && !!(us->flag_bits & BIT(1))) {
                rd->alarm_info.info_type = 0x4;
                rd->alarm_info.value = !!(us->flag_bits & BIT(1));
                rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
				alarm_to_app = true;
			}
			
            rd->stat.battary = us->battery;
			if (rd->stat.battary < 5) {
				rd->stat.battary = 5;
			}
            rd->stat.is_door_open = !!(us->flag_bits & BIT(0));
           	rd->stat.is_battary_warn = !!(us->flag_bits & BIT(1));
            rd->stat.is_break_door = !!(us->flag_bits & BIT(2));
            rd->stat.is_guard = !!(us->flag_bits & BIT(3));

			rd->stat.stat_valid = true;

			log_debug("dor magnet: is_door_open %u battary_low %u is_break_door %u is_guard %u\n", 
				rd->stat.is_door_open, rd->stat.battary, rd->stat.is_break_door, rd->stat.is_guard);

			log_debug("%"PRIu64" alarm info_type=%u value=%u time_stamp=%u\n", 
				slave->sn, rd->alarm_info.info_type, rd->alarm_info.value, rd->alarm_info.time_stamp);
			
            break;
        case UP_TLV_SMTDLOCK_CMD_PUSH_ONOFFTIME:
            uh = rf_tlv_val(tlv);
			if (tlv->len < sizeof(*uh)) {
				return false;
			}
			
            _rf_door_magnet_update_history(slave, rd,uh);
            break;
        case SMTDLOCK_CMD_SET_AUTO_GUARD:
            stat = rf_tlv_val(tlv);
			if (tlv->len < sizeof(*stat)) {
				return false;
			}
			
            rd->auto_on.type = MODE_AUTO_GUARD_ON;
            rd->auto_on.enable = stat->on_enable;
            rd->auto_on.start_hour = stat->on_start_hour;
            rd->auto_on.end_hour = stat->on_stop_hour;
            
            rd->auto_off.type = MODE_AUTO_GUARD_OFF;
            rd->auto_off.enable = stat->off_enable;
            rd->auto_off.start_hour = stat->off_start_hour;
            rd->auto_off.end_hour = stat->off_stop_hour;

			log_debug("xxx auto on %u auto off %u\n", rd->auto_on.enable, rd->auto_off.enable);
            
            break;
		case RF_TT_CMD_PUSH_ALARM:
			log_debug("enter %s alarm\n", __FUNCTION__);
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, door_magnet_alarm_update_idx);
			break;
        default:
            return false;
            break;
    }
    
    if (alarm_to_app) {
		event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
        event_push(slave->user->callback, SAE_RF_DEV_ALARM_INFO, slave->handle, slave->user->callback_handle);
        return false;
    }
    
    return true;
}

static void manage_set_sync_local(slave_t* slave, cl_rf_auto_guard_info_t* ai)
{
	cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);

	// TODO:暂时就不限制类型了

	if (ai->type == MODE_AUTO_GUARD_ON) {
		rd->auto_on.type = MODE_AUTO_GUARD_ON;
	    rd->auto_on.enable = ai->enable;
	    rd->auto_on.start_hour = ai->start_hour;
	    rd->auto_on.end_hour = ai->end_hour;
	} else {
		rd->auto_off.type = MODE_AUTO_GUARD_OFF;
	    rd->auto_off.enable = ai->enable;
	    rd->auto_off.start_hour = ai->start_hour;
	    rd->auto_off.end_hour = ai->end_hour;
	}
}

static bool _rf_door_magnet_proc_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_rf_door_com_ctrl_t * dc = (ucp_rf_door_com_ctrl_t*)(tlv+1);
    ucp_rf_auto_guard_ctrl_t* ac = (ucp_rf_auto_guard_ctrl_t*)(tlv+1);
    cl_rf_auto_guard_info_t* ai;
	cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
    int len = 0,type;
    u_int32_t value;
    
    switch (info->action) {
        case ACT_RF_COM_DIRECT_CTRL:
            tlv->len = sizeof(*dc);
            value = cci_u32_data(info);
            type = (value >> 16)&0xFF;
            
            switch (type) {
                case RDM_CTRL_OPEN:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_ONOFF_DOORLOCK;
                    dc->value = !!(value & 0xFF);
                    tlv->len = sizeof(*dc);
                    len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存数值
					rd->stat.is_door_open = dc->value;
                    break;
                case RDM_CTRL_GRARD:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                    dc->value = !!(value & 0xFF);
                    tlv->len = sizeof(*dc);
                    len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存数值
					rd->stat.is_guard = dc->value;
                    break;
                    
                default:
                    len = 0;
                    break;
            }
            break;
        case ACT_RF_COM_AUTO_GUARD:
            tlv->len = sizeof(*ac);
            ai = cci_pointer_data(info);
            ac->enable = ai->enable;
            ac->start_hour = ai->start_hour;
            ac->end_hour = ai->end_hour;
            ac->type = ai->type;
            len = sizeof(*tlv)+sizeof(*ac);
			tlv->type = SMTDLOCK_CMD_SET_AUTO_GUARD;
			// TODO:先修改本地数据
			manage_set_sync_local(slave, ai);

			log_debug("@@@ set auto type %u onoff %u\n", ai->type, ai->enable);
            break;
        case ACT_RF_COM_QUERY_HISTORY:
            return _rf_proc_query_his_notify(slave,info,ret);
            break;
        default:
            *ret =RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}

static bool _rf_door_manget_proc_group_notify(user_t* user,u_int8_t group_id,u_int8_t group_type,
                                              u_int8_t ctrl_type,u_int8_t ctrl_value, RS *ret)
{
    char buf[1024] = {0};
    int t_len;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_group_tt_t *up = (net_rfgw_group_tt_t*)(uo+1);
    rf_tlv_t* tlv = (rf_tlv_t*)(up+1);
    ucp_rf_door_com_ctrl_t * dc = (ucp_rf_door_com_ctrl_t*)(tlv+1);
    
    if (group_id == 0) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    up->sub_type = IJ_RFGW;
    up->ext_type = RF_EXT_TYPE_DOOR_MAGNET;
    
    switch (ctrl_type) {
        case RDM_CTRL_OPEN:
            tlv->type = UP_TLV_SMTDLOCK_CMD_SET_ONOFF_DOORLOCK;
            dc->value = !!(ctrl_value);
            up->group_id = group_id;
            tlv->len = sizeof(*dc);
            up->len = sizeof(*tlv)+sizeof(*dc);
            break;
        case RDL_CTRL_GRARD:
            tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
            dc->value = !!(ctrl_value);
            up->group_id = group_id;
            tlv->len = sizeof(*dc);
            up->len = sizeof(*tlv)+sizeof(*dc);
            break;
        default:
            *ret =RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    t_len = sizeof(*up)+up->len;
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP_TT, (u_int16_t)sizeof(ucp_obj_t)+t_len);
    sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+t_len);
    
    return true;
}


//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// 单火线
static void _rf_dhx_save(slave_t *slave)
{	
	cl_dhx_save_conf_t conf;
	cl_dhx_switch_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);

	memcpy((void *)conf.keys, (void *)info->keys, sizeof(conf.keys));
	memcpy((void *)conf.index_key, (void *)info->index_key, sizeof(conf.index_key));
	
	_rf_com_write_user_info(slave, DHX_CONF_ID, (u_int8_t*)&conf, sizeof(conf));

	log_info(" done\n");
}

static bool _rf_dhx_load(slave_t *slave)
{	
	cl_dhx_save_conf_t conf;
	cl_dhx_switch_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);

	if (RS_OK != _rf_com_load_user_info(slave, DHX_CONF_ID, (u_int8_t *)&conf, sizeof(conf))) {
		return false;
	}

	memcpy((void *)info->keys, (void *)conf.keys, sizeof(info->keys));
	memcpy((void *)info->index_key, (void *)conf.index_key, sizeof(info->index_key));

	return true;
}

static bool _rf_dhx_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	int i;
	dhx_cache_t *cache = (dhx_cache_t *)param;
    cl_dhx_switch_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);
	u_int8_t reallen;
	u_int8_t kid;

	cache->flagbits = ntohl(cache->flagbits);

	reallen = (cache->flagbits >> 24) & 0xff;

	if ((reallen > len) || 
		(reallen < sizeof(dhx_cache_old_t))) {
		log_debug("%s err  realen=%u len=%u need min=%d\n", __FUNCTION__, reallen, len, sizeof(dhx_cache_old_t));
		return false;
	}
	
	if (reallen == sizeof(dhx_cache_old_t)) {
		info->support_time = 0;
		info->support_name_set = 0;
	} else if (reallen > sizeof(dhx_cache_old_t)) {
		info->support_time = 1;
		info->support_name_set = 1;
	}
	
	info->stat.group_num = cache->group_num;
	info->stat.on_off_stat = cache->onoff;

	log_info("sn=%"PRIu64" cache->group_num=%u cache->onoff=%u cache->time_cnum=%u"
		"len=%u realen=%u spt=%u spn=%u\n", 
		slave->sn, cache->group_num, cache->onoff, cache->time_cnum, len, reallen, 
		info->support_time, info->support_name_set);
	
	if (info->support_time) {
		if (info->time_cnum != cache->time_cnum) {
			info->time_cnum = cache->time_cnum;
			//timer query
			rf_query_comm_timer(slave);
		}
	}

	if (!info->support_name_set) {
		return true;
	}
	
	if (!info->init) {
		info->init = true;
		_rf_dhx_load(slave);
	}

	//key name query
	for (i = 0; i < DHX_MAX_NUM; i++) {
		log_info("slave->status=%u info->index_key[%d]=%u cache->index_key[%d]=%u\n", 
			slave->status, i, info->index_key[i], i, cache->index_key[i]);
		if (info->index_key[i] == cache->index_key[i]) {
			continue;
		}

		kid = i + 1;

		log_debug("sc key kid %d index %u => %u, query it\n", kid, info->index_key[i], cache->index_key[i]);

		info->index_key[i] = cache->index_key[i];

		rf_dev_query_tlv_info(slave, RF_TT_DHX_TYPE_QUERY_KEY, &kid, sizeof(kid));		
	}

	return true;
}


static bool _rf_dhx_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_rf_door_lock_stat_t* us;
    cl_dhx_switch_info_t* ds = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);
    ucp_scene_controller_key_push_t *push;
	int len;
	
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    switch (tlv->type) {
        case UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY:
            us = rf_tlv_val(tlv);
            us->flag_bits = ntohl(us->flag_bits);
            ds->stat.group_num = (us->flag_bits >> 2) & 0xF;
            ds->stat.on_off_stat = (us->flag_bits >> 8) & 0xFF;
            break;
		case RF_TT_DHX_TYPE_PUSH_KEY:
			push = (ucp_scene_controller_key_push_t *)rf_tlv_val(tlv);
			
			if (tlv->len < sizeof(*push)) {
				return false;
			}

			len = tlv->len - sizeof(*push);

			if (len > (DHX_MAX_NAME_LEN - 1)) {
				log_err(false, "sc key name len %d invalid\n", len);
				return false;
			}

			if (push->id == 0 || push->id > DHX_MAX_NUM) {
				return false;
			}

			memset((void *)ds->keys[push->id - 1].name, 0, sizeof(ds->keys[push->id - 1].name));
			memcpy((u_int8_t*)&ds->keys[push->id - 1].name, push->name, len);
			ds->keys[push->id - 1].valid = true;
			
			log_debug("sc update key id %u name[%s]\n", push->id, ds->keys[push->id - 1].name);

			_rf_dhx_save(slave);
			break;
        default:
            return false;
            break;
    }
    return true;
}

static bool _rf_dhx_proc_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_rf_door_com_ctrl_t * dc = (ucp_rf_door_com_ctrl_t*)(tlv+1);
    int len = 0,type;
    u_int32_t value;
	ucp_scene_controller_key_set_t *request;
	cl_dhx_switch_info_t* dhx_info = &(slave->dev_info.rf_stat.dev_priv_data.dhx_info);
    
    switch (info->action) {
    case ACT_RF_COM_DIRECT_CTRL:
        tlv->len = sizeof(*dc);
        value = cci_u32_data(info);
        type = (value >> 16)&0xFF;
        
        if (type == DHX_CTRL_ON_OFF) {
            value  =  value & 0xFF;
            tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
            tlv->len = sizeof(u_int32_t);
            len = sizeof(*tlv)+sizeof(u_int32_t);
            memcpy(tlv+1, &value, sizeof(value));
        }
        break;
	case ACT_DHX_SET_NAME:
		request = (ucp_scene_controller_key_set_t *)cci_pointer_data(info);

		if (!dhx_info->support_name_set) {
			*ret = RS_NOT_SUPPORT;
			return false;
		}
		if (request->id == 0 || request->id > DHX_MAX_NUM) {
			*ret = RS_INVALID_PARAM;
			return false;
		}

		len = (int)strlen(request->name) + 1;
		if (len > DHX_MAX_NAME_LEN) {		
			*ret = RS_INVALID_PARAM;
			return false;
		}

		tlv->type = RF_TT_DHX_TYPE_SET_KEY;
		tlv->len = (u_int8_t)info->data_len;

		memcpy((u_int8_t*)&tlv[1], (u_int8_t*)request, info->data_len);
		
		len = tlv->len + sizeof(*tlv);

		log_debug("set id %u name len %u name [%s], total %u\n", request->id, request->len, request->name, info->data_len);

		// 本地先修改
		memset((u_int8_t*)&dhx_info->keys[request->id - 1], 0x00, sizeof(dhx_info->keys[request->id - 1]));
		dhx_info->keys[request->id - 1].valid = true;				
		memcpy(dhx_info->keys[request->id - 1].name, request->name, sizeof(dhx_info->keys[request->id - 1].name));
		break;
    default:
        *ret =RS_INVALID_PARAM;
        return  false;
        break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }else{
        *ret =RS_INVALID_PARAM;
        return  false;
    }
    
    return true;
}

static bool _rf_dhx_proc_group_notify(user_t* user,u_int8_t group_id,u_int8_t group_type,
                                      u_int8_t ctrl_type,u_int8_t ctrl_value, RS *ret)
{
    char buf[1024] = {0};
    int t_len;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_group_tt_t *up = (net_rfgw_group_tt_t*)(uo+1);
    rf_tlv_t* tlv = (rf_tlv_t*)(up+1);
    u_int8_t* p;
    
    if (group_id == 0) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    if (ctrl_type != DHX_CTRL_ON_OFF) {
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    up->sub_type = IJ_RFGW;
    up->ext_type = RF_EXT_TYPE_DHX;
    
    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
    tlv->len = sizeof(u_int32_t);
    p = (u_int8_t*)(tlv+1);
    p+=3;
    memcpy(p, &ctrl_value, sizeof(ctrl_value));
    up->group_id = group_id;
    tlv->len = sizeof(sizeof(u_int32_t));
    up->len = sizeof(*tlv)+sizeof(u_int32_t);

   
    t_len = sizeof(*up)+up->len;
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP_TT, (u_int16_t)sizeof(ucp_obj_t)+t_len);
    sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+t_len);
    
    return true;
}


/////////////////////////////////////////////////////////////////
// 海曼温湿度
// 6秒到10秒，中间间隔200MS
static int _rf_hm_temp_hum_rand_time(void)
{
#define BEGIN_SEC 6
#define END_SEC 10
	int t = TIME_N_SECOND(BEGIN_SEC), n;
	
	srand(get_sec());

	n = rand();

	n = n % ((END_SEC - BEGIN_SEC) * 5);

	return t + TIME_N_MSECOND(n * 2);
}

static int _rf_hm_temp_hum_query_check(cl_thread_t *t)
{
	slave_t *slave = (slave_t *)CL_THREAD_ARG(t);
	cl_hm_temp_hum_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.ht_info);
	
	slave->t_stat_query = NULL;

	log_debug("%"PRIu64" _rf_hm_temp_hum_query_check history valid %u count %d\n", slave->sn, info->is_history_data_valid, info->history_query_count);

	if (info->is_history_data_valid) {
		return 0;
	}

	if (++info->history_query_count > 3) {
		return 0;
	}

	rf_dev_query_tlv_info(slave, RF_TT_CMD_QUERY_CURVE, NULL, 0);

	CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_stat_query, _rf_hm_temp_hum_query_check, slave, _rf_hm_temp_hum_rand_time());

	return 0;
}

static bool _rf_hm_temp_hum_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_hm_temp_hum_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.ht_info);
	hm_temp_hum_cache_t *cache = (hm_temp_hum_cache_t *)param;
	bool new_cache = false;
	int real_len = 0;

	log_debug(" >>>>>>>> [%"PRIu64"] enter hm temp cache\n", slave->sn);

	cache->flag = htonl(cache->flag);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	real_len = (cache->flag >> 24) & 0xff;
	if (((cache->flag >> 24) & 0xff) > 0) {
		log_debug("It's new device rea_len %d\n", real_len);
		if (real_len >= 16) {
			cache->temp100 = ntohs(cache->temp100);
			info->support_temp100 = true;
			info->cur_temp100 = cache->temp100;
		}
	}

	// 更新基本信息
	info->is_low_battary_warn = !!(cache->flag & BIT(1));

	info->battary = cache->abc_battery;
	if (info->battary < 5) {
		info->battary = 5;
	}
	info->cur_temp = cache->temp;
	info->cur_hum = cache->humi;
	

	log_debug("get hm temp %u humi %u index_curve %u\n", 
		info->cur_temp, info->cur_hum, cache->index_curve);

	if (info->is_history_data_valid) {
		info->his_data[0].temp = info->cur_temp;
		info->his_data[0].hum = info->cur_hum;
	}

	// 更新到过去12小时里面去
	if (info->index_curve != cache->index_curve) {
		log_debug("_rf_hm_temp_hum_update_cache_data index_curve query %u => %u\n", info->index_curve, cache->index_curve);
		info->index_curve = cache->index_curve;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_QUERY_CURVE, NULL, 0);

		// 启动定时器查询，怕没查到
		CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_stat_query, _rf_hm_temp_hum_query_check, slave, _rf_hm_temp_hum_rand_time());
	}

	if (info->index_alarm != cache->index_alarm) {		
		info->index_alarm = cache->index_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}

	if (info->stat_valid == false) {
		info->stat_valid = true;
		// 只查询一次软硬件版本
		//rf_dev_query_tlv_info(slave, UP_TLV_RF_DEV_VERSION, NULL, 0);		
	}

	log_debug(" <<<<<< [%"PRIu64"] exit hm temp cache\n", slave->sn);
	
	return true;
}

static void hm_temp_update_idx(slave_t *slave, u_int8_t index_alarm)
{
    cl_hm_temp_hum_info_t* ht = &(slave->dev_info.rf_stat.dev_priv_data.ht_info);
    

	log_debug("hm_temp_update_idx update alarm idex to %u\n", index_alarm);

	ht->index_alarm = index_alarm;
}

static bool _rf_hm_temp_hum_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_hm_temp_hum_t* us;
    cl_hm_temp_hum_info_t* ht = &(slave->dev_info.rf_stat.dev_priv_data.ht_info);
    ucp_hm_temp_hum_his_t* hh;
    int i;
	u_int32_t value = 0;
    
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    switch (tlv->type) {
        case UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY:
			log_debug("[%"PRIu64"] get push sumary, it's old dev\n", slave->sn);
            us = rf_tlv_val(tlv);
            
            if (us->hum > 90) {
                us->hum = 90;
            }
            
            if (us->hum < 10) {
                us->hum = 10;
            }
            
            us->flag_bits = ntohl(us->flag_bits);
            ht->battary = us->battery;
			if (ht->battary < 5) {
				ht->battary = 5;
			}
            ht->cur_temp = us->temp;
            ht->cur_hum = us->hum;
            ht->is_low_battary_warn = !!(us->flag_bits & BIT(1));

			// 如果收到老的状态数据，认为这是一个旧设备，
			// 在第一次的时候发送一个查询曲线命令
			if (ht->stat_valid == false) {
				ht->stat_valid = true;

				log_debug("query old curve \n");
				rf_dev_query_tlv_info(slave, UP_TLV_SMTDLOCK_CMD_GET_ONOFFTIME, (u_int8_t*)&value, sizeof(value));
			}
            break;
        case SENSORFT_CMD_PUSH_SUMMARY:
			log_debug("get old curve\n");
			
            hh = rf_tlv_val(tlv);
            ht->is_history_data_valid = true;
            ht->history_hour = hh->hour;
            for (i = 0 ; i < MAX_HM_HISTORY_NUM; i++) {
                ht->his_data[i].temp = hh->th[i].temp;
                ht->his_data[i].hum = hh->th[i].hum;
            }
            break;

		case RF_TT_CMD_PUSH_CURVE:
			{
				ucp_hm_temp_hum_his_v2_t *hh = (ucp_hm_temp_hum_his_v2_t *)rf_tlv_val(tlv);

				log_debug("[%"PRIu64"] get push curve \n", slave->sn);
				if (tlv->len < sizeof(*hh)) {
					log_err(false, "invalid len %u\n", tlv->len);
					return false;
				}

				if (hh->index != ht->index_curve) {
					log_err(false, "need cur index %u, but get %u\n", ht->index_curve, hh->index);
					return false;
				}

				ht->is_history_data_valid = true;

				for (i = 1 ; i < MAX_HM_HISTORY_NUM; i++) {
	               ht->his_data[i].temp = hh->th[i].temp;
	               ht->his_data[i].hum = hh->th[i].hum;
				   log_debug("add index %d temp %u hum %u\n", i, ht->his_data[i].temp, ht->his_data[i].hum);
           		}

				// 最新数据设备填的0，SDK把cache那获取到的数据拷贝进去
				ht->his_data[0].temp = ht->cur_temp;
				ht->his_data[0].hum = ht->cur_hum;
			}
			break;

		case UP_TLV_RF_DEV_VERSION:
			{
				ucp_scene_controller_version_t *version = rf_tlv_val(tlv);
				if (tlv->len < sizeof(*version)) {
					return false;
				}


				ht->hw_ver = version->hw_ver;
				ht->soft_ver_mar = (version->soft_ver >> 7) & 0x1;
				ht->soft_ver_min = (version->soft_ver >> 4) & 0x7;
				ht->soft_ver_rev = (version->soft_ver >> 7) & 0xF;
				ht->svn = ntohl(version->svn);

				
				log_debug("hm temp hum hw_ver %u svn %u\n", ht->hw_ver, ht->svn);

			}

		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, hm_temp_update_idx);
		
        default:
            return false;
            break;
    }
    return true;
}

static bool _rf_hm_temp_hum_proc_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
	#if 0
    if (info->action == ACT_RF_COM_QUERY_HISTORY) {
        return _rf_proc_query_his_notify(slave,info,ret);
    }
	#endif
    *ret =RS_INVALID_PARAM;
    return  false;
}

/////////////////////////////////////////////////////////////////
// 海曼人体探测设备
static bool _rf_hm_body_detect_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_hm_body_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.hb_info);
	hm_body_detect_cache_t *cache = (hm_body_detect_cache_t *)param;

	if (len < sizeof(hm_body_detect_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	info->is_support_new_history = true;

	cache->flag = htonl(cache->flag);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->hislog_count));
	
	// 更新基本信息
	info->is_low_battary_warn = !!(cache->flag & BIT(0));
	info->is_guard = !!(cache->flag & BIT(3));
	info->is_break = !!(cache->flag & BIT(5));

	info->detectd_num = (u_int32_t)ntohs(cache->alarm_num);
	info->battary = cache->abc_battery;
	if (info->battary < 5) {
		info->battary = 5;
	}
	if (info->index_alarm != cache->index_alarm) {
		log_debug("_rf_com_detector_update_cache_data alarm query\n");
		info->index_alarm = cache->index_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}

	if (info->index_time != cache->index_time) {
		log_debug("_rf_com_detector_update_cache_data time query\n");
		info->index_time = cache->index_time;
		rf_dev_query_tlv_info(slave, UP_TLV_RF_DEV_PUSH_ALARMTIME, NULL, 0);
	}

	if (info->stat_valid == false) {
		info->stat_valid = true;
		// 只查询一次软硬件版本
		rf_dev_query_tlv_info(slave, UP_TLV_RF_DEV_VERSION, NULL, 0);		
	}

	log_debug("%"PRIu64": body dectect update detected num %u index_alarm %u is_defence %u\n", 
		slave->sn, info->detectd_num, info->index_alarm, info->is_guard);

	return true;
}



static void _rf_hm_body_instert_his(cl_hm_body_info_t* rd,u_int8_t action , u_int32_t time)
{
    int i ;
    cl_rf_door_history_t* dh,*pd;
    u_int8_t type,value;
    u_int32_t temp_time;
    
    if(!(action & BIT(0)) || time == 0){
        return;
    }
    
    type = (action >>2) & 0x3F;
    value  = (action >>1) & 0x1;
    dh = &rd->his[0];
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (dh->is_valid && dh->time_stamp == time && dh->info_type == type && dh->value == value) {
            return;
        }
    }
    
    dh = &rd->his[0];
    pd = NULL;
    temp_time = 0xFFFFFFFF;
    
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (!dh->is_valid ) {
            dh->is_valid = true;
            dh->info_type = type;
            dh->value  = value;
            dh->time_stamp = time;
            break;
        }
        
        if (dh->is_valid && dh->time_stamp < temp_time) {
            temp_time = dh->time_stamp;
            pd = dh;
        }
    }
    
    if (i >= sizeof(rd->his)/sizeof(cl_rf_door_history_t) && pd != NULL) {
        pd->is_valid = true;
        pd->info_type = type;
        pd->value  = value;
        pd->time_stamp = time;
    }
}

static void _rf_hm_body_update_history(cl_hm_body_info_t* rd,ucp_rf_door_history_t* uh)
{
    uh->time_1 = ntohl(uh->time_1);
    uh->time_2 = ntohl(uh->time_2);
    uh->time_3 = ntohl(uh->time_3);
    uh->time_4 = ntohl(uh->time_4);
    
    _rf_hm_body_instert_his(rd,uh->on_off1,uh->time_1);
    _rf_hm_body_instert_his(rd,uh->on_off2,uh->time_2);
    _rf_hm_body_instert_his(rd,uh->on_off3,uh->time_3);
    _rf_hm_body_instert_his(rd,uh->on_off4,uh->time_4);
    
}

static void hm_body_detect_update_idx(slave_t *slave, u_int8_t index_alarm)
{
     cl_hm_body_info_t* hb = &(slave->dev_info.rf_stat.dev_priv_data.hb_info);

	log_debug("hm_body_detect_update_idx update alarm idex to %u\n", index_alarm);

	hb->index_alarm = index_alarm;
}

static bool _rf_hm_body_detect_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_hm_body_t* ub;
    cl_hm_body_info_t* hb = &(slave->dev_info.rf_stat.dev_priv_data.hb_info);
    ucp_rf_door_history_t* uh;
    ucp_hm_body_push_t* bp;
    ucp_com_detector_alarm_time_t *at = rf_tlv_val(tlv);
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    switch (tlv->type) {
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
			
		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, hm_body_detect_update_idx);

		case UP_TLV_RF_DEV_VERSION:
			{
				ucp_com_detector_version_t *version = rf_tlv_val(tlv);
				if (tlv->len < sizeof(*version)) {
					return false;
				}

				hb->hw_ver = version->hw_ver;
				hb->soft_ver_mar = (version->soft_ver >> 7) & 0x1;
				hb->soft_ver_min = (version->soft_ver >> 4) & 0x7;
				hb->soft_ver_rev = (version->soft_ver >> 7) & 0xF;
				hb->svn = ntohl(version->svn);
			}
			break;

			
        case UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY:
            ub = rf_tlv_val(tlv);
            ub->flag_bits = ntohl(ub->flag_bits);
            hb->is_low_battary_warn = !!(ub->flag_bits & BIT(1));
            hb->is_guard = !!(ub->flag_bits & BIT(3));
            hb->last_guard_time = ntohl(ub->time);
            hb->detectd_num = ntohl(ub->num);
            hb->battary = ub->battery;
			if (hb->battary < 5) {
				hb->battary = 5;
			}
            break;
        case UP_TLV_SMTDLOCK_CMD_PUSH_ONOFFTIME:
            uh = rf_tlv_val(tlv);
			if (tlv->len < sizeof(*uh)) {
				return false;
			}
			
            _rf_hm_body_update_history(hb,uh);
            break;
        case SENSOR_BODY_DETECT_PUSH:
            bp = rf_tlv_val(tlv);
            bp->flags = ntohl(bp->flags);
            if (bp->flags & BIT(0)) {
                hb->alarm_info.info_type = 0x3;
                hb->alarm_info.value = !!(bp->flags & BIT(0));
                hb->alarm_info.time_stamp = ntohl(bp->time);
                event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
                event_push(slave->user->callback, SAE_RF_DEV_ALARM_INFO, slave->handle, slave->user->callback_handle);
            }
            
            return false;
            break;
		case UP_TLV_RF_DEV_PUSH_ALARMTIME:

			_rf_com_detector_instert_alarm_time(&hb->alarm_time, at->ctl_1 & 1, (at->ctl_1 >> 1) & 0x7, ntohl(at->time_1));
			_rf_com_detector_instert_alarm_time(&hb->alarm_time, at->ctl_2 & 1, (at->ctl_2 >> 1) & 0x7, ntohl(at->time_2));
			_rf_com_detector_instert_alarm_time(&hb->alarm_time, at->ctl_3 & 1, (at->ctl_3 >> 1) & 0x7, ntohl(at->time_3));
			_rf_com_detector_instert_alarm_time(&hb->alarm_time, at->ctl_4 & 1, (at->ctl_4 >> 1) & 0x7, ntohl(at->time_4));
			
			break;
        default:
            return false;
            break;
    }
    return true;
}

static bool _rf_hm_body_detect_proc_notify(slave_t* slave,cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_rf_door_com_ctrl_t * dc = (ucp_rf_door_com_ctrl_t*)(tlv+1);
    cl_hm_body_info_t* hb = &(slave->dev_info.rf_stat.dev_priv_data.hb_info);
    int len = 0,type;
    u_int32_t value;
    
    
    switch (info->action) {
        case ACT_RF_COM_DIRECT_CTRL:
            tlv->len = sizeof(*dc);
            value = cci_u32_data(info);
            type = (value >> 16)&0xFF;
            
            if (type == RDM_HM_CTRL_GUARD) {
                tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                dc->value = !!(value & 0xFF);
				dc->pad = 0xff;
                tlv->len = sizeof(*dc);
                len = sizeof(*tlv)+sizeof(*dc);

				// 先修改内存值，因为查询回来比较慢
				hb->is_guard = dc->value;
            } else if (type == RDM_HM_CTRL_SET_FREQ) {
            	tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                dc->pad = (value & 0xFF);
				dc->value = 0xff;
                tlv->len = sizeof(*dc);
                len = sizeof(*tlv)+sizeof(*dc);

				// 单位是5秒，立即改变
				hb->alarm_time.time[2] = (value & 0xff) * 5;
            }
            break;
        case ACT_RF_COM_QUERY_HISTORY:
            return _rf_proc_query_his_notify(slave,info,ret);
            break;
        
        default:
            *ret =RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }else{
        *ret =RS_INVALID_PARAM;
        return  false;
    }
    
    return true;
}
///////////////////////////////////////////////////////
// 暖气阀
//定时查询暖气阀状态
#define HV_STAT_QUERY_TIME		(30)

static int _rf_hv_stat_query(cl_thread_t *t)
{
	slave_t *slave = (slave_t *)CL_THREAD_ARG(t);

	slave->t_stat_query = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_stat_query, _rf_hv_stat_query, slave, TIME_N_SECOND(HV_STAT_QUERY_TIME));

	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	//如果设备处于后台或者从设备不在线，则不要查询
	if (cl_priv->run_in_background ||
		(slave->status != BMS_BIND_ONLINE)) {
		return 0;
	}
	rf_dev_query_tlv_info(slave, UP_TLV_HV_STAT_QUERY, NULL, 0);

	return 0;
}

static bool _rf_heating_valve_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
    cl_heating_valve_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.hv_info);
	heaing_valve_cache_t *cache = (heaing_valve_cache_t *)param;
	static bool first = true;

	if (len < sizeof(heaing_valve_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	log_debug("sn %"PRIu64" heating valve state index %u time index %u circle index %u quick timer index %u\n", 
		slave->sn, cache->mcu_stat_index, cache->mcu_time_index, cache->mcu_circle_index, cache->quick_timer_index);

	if (rd->cache.mcu_stat_index != cache->mcu_stat_index) {
		log_debug("sn %"PRIu64" _rf_heating_valve_update_cache_data mcu_stat_index query\n", slave->sn);
		rd->cache.mcu_stat_index = cache->mcu_stat_index;
		rf_dev_query_tlv_info(slave, UP_TLV_HV_GET_STATUS, NULL, 0);
	}

	if (rd->cache.mcu_time_index != cache->mcu_time_index) {
		log_debug("sn %"PRIu64" _rf_heating_valve_update_cache_data mcu_time_index query\n", slave->sn);
		rd->cache.mcu_time_index = cache->mcu_time_index;
		rf_dev_query_tlv_info(slave, UP_TLV_HV_GET_TIME, NULL, 0);
	}

	if (rd->cache.mcu_circle_index != cache->mcu_circle_index) {
		log_debug("sn %"PRIu64" _rf_heating_valve_update_cache_data mcu_circle_index query\n", slave->sn);
		rd->cache.mcu_circle_index = cache->mcu_circle_index;
		rf_dev_query_tlv_info(slave, UP_TLV_HV_GET_CIRCLE, NULL, 0);
	}

	if (rd->cache.quick_timer_index != cache->quick_timer_index) {
		log_debug("sn %"PRIu64" _rf_heating_valve_update_cache_data quick_timer_index query\n", slave->sn);
		rd->cache.quick_timer_index = cache->quick_timer_index;
		rf_dev_query_tlv_info(slave, UP_TLV_HV_GET_QUICK_TIMER, NULL, 0);
	}

	//设备自己会push，开始始自己查询一次就行了
	if (first) {
		log_debug("sn %"PRIu64" _rf_heating_valve_update_cache_data com_timer_index query\n", slave->sn);
		first = false;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_GET_TIMER_SUMMARY, NULL, 0);
	}

	//启动定时器定时查询暖气阀状态
	CL_THREAD_OFF(slave->t_stat_query);
	CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_stat_query, _rf_hv_stat_query, slave, TIME_N_SECOND(HV_STAT_QUERY_TIME));

	return false;
}


static void rf_heating_send_uart_cmd(slave_t* slave, u_int8_t cmd, u_int8_t *param, u_int8_t len)
{
	u_int8_t buf[64] = {0};
	rf_tlv_t* tlv = (rf_tlv_t*)buf;
	heating_valve_uart_hd_t *hd;

	tlv->type = UP_TLV_HV_UART_TT;
	tlv->len = sizeof(*hd) + len;

	hd = rf_tlv_val(tlv);
	

	hd->syn1 = 0xff;
	hd->syn2 = 0xaa;
	hd->syn3 = 0x55;
	hd->ver = 1;
	hd->cmd = cmd;
	hd->plen = len;

	memcpy((u_int8_t*)&hd[1], param, len);

	rfgw_send_tt_packet(slave, buf, sizeof(*tlv) + tlv->len);
}

static bool _rf_heating_valve_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
   	u_int8_t buf2[1024] = {0};
	int len;
    rf_tlv_t* tlv = (rf_tlv_t*)buf2;
    //int len = 0,type;
    u_int8_t u8value;
    cl_heating_valve_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.hv_info);

    switch (info->action) {
		// 设置开关机等一个字节可以表述的控制状态
       // case ACT_HEATING_VALVE_ONOFF:
		case ACT_HEATING_VALVE_MODE:
//		case ACT_HEATING_VALVE_ANTI_LIME:
		//case ACT_HEATING_VALVE_FROST_PROTECTION:
		case ACT_HEATING_VALVE_CHILD_LOCK:
		case ACT_HEATING_VALVE_WINDOW:
		//case ACT_HEATING_VALVE_SUMMER_WINTER:

            u8value = cci_u8_data(info);
			rf_heating_send_uart_cmd(slave, info->action, &u8value, sizeof(u8value));

			// 先修改本地值
			if (info->action == ACT_HEATING_VALVE_MODE) {
				rd->stat.mode = u8value;
			} else if (info->action == ACT_HEATING_VALVE_CHILD_LOCK) {
				rd->stat.child_proof = u8value;
			} else if (info->action == ACT_HEATING_VALVE_WINDOW) {
				rd->stat.windowfun = u8value;
			} 

			break;
		case ACT_HEATING_VALVE_DATE:
			{
				cl_heating_valve_param_date_t *pdate = cci_pointer_data(info);

				// 先修改本地
				rd->stat.year = pdate->year;
				rd->stat.month = pdate->month;
				rd->stat.day = pdate->day;
				rd->stat.hour = pdate->hour;
				rd->stat.minute = pdate->minute;
				
				pdate->year = ntohs(pdate->year);

				rf_heating_send_uart_cmd(slave, info->action, (u_int8_t*)pdate, sizeof(*pdate));
			}
			break;

		case ACT_HEATING_VALVE_TEMP:
			{
				cl_heating_valve_param_temp_t *ptemp = cci_pointer_data(info);

				// 先修改本地
				rd->stat.manual_temp = ptemp->manual_temp;
				rd->stat.heat_temp = ptemp->heat_temp;
				rd->stat.economy_temp = ptemp->economy_temp;
				
				ptemp->manual_temp = ntohs(ptemp->manual_temp);
				ptemp->heat_temp = ntohs(ptemp->heat_temp);
				ptemp->economy_temp = ntohs(ptemp->economy_temp);

				rf_heating_send_uart_cmd(slave, info->action, (u_int8_t*)ptemp, sizeof(*ptemp));

				
			}
			break;

		case ACT_HEATING_VALVE_PERIOD:
			{
				cl_heating_valve_param_period_t *request = cci_pointer_data(info);
				if (request->day == 0 || request->day >= 7) {
					return false;
				}
				
				rf_heating_send_uart_cmd(slave, info->action, (u_int8_t*)request, sizeof(*request));

				// 先修改本地数据
				rd->day_period[request->day - 1].eh1 = request->eh1;
				rd->day_period[request->day - 1].hm1 = request->hm1;
				rd->day_period[request->day - 1].eh1 = request->eh1;
				rd->day_period[request->day - 1].em1 = request->em1;
				rd->day_period[request->day - 1].hh2 = request->hh2;
				rd->day_period[request->day - 1].hm2 = request->hm2;
				rd->day_period[request->day - 1].eh2 = request->eh2;
				rd->day_period[request->day - 1].em2 = request->em2;
			}

			break;
		case ACT_RF_COM_SHORTCUTS_ONOFF_QUERY:
			tlv->type = UP_TLV_HV_GET_QUICK_TIMER;
			tlv->len = 0;
			len = sizeof(*tlv);
			rfgw_send_tt_packet(slave, buf2, len);
			break;
		case ACT_RF_COM_SHORTCUTS_ONFF_SET:
			{
				cl_shortcuts_onoff_t *request = cci_pointer_data(info);
				ucp_shortcuts_onoff_t so_net;
				
				memcpy((void *)&slave->shortcuts_onoff, (void *)request, sizeof(*request));
				
				so_net.enable = request->enable;
				so_net.onoff = request->onoff;
				so_net.remain_time = htons((u_int16_t)request->remain_time);

				tlv->type = UP_TLV_HV_SET_QUICK_TIMER;
				tlv->len = sizeof(so_net);
				memcpy(rf_tlv_val(tlv), (void *)&so_net, sizeof(so_net));
				len = sizeof(so_net) + sizeof(*tlv);
				rfgw_send_tt_packet(slave, buf2, len);
			}
			break;
		default:
			return false;
    }
	
    return true;
}

static bool rf_heating_valve_updata_uart(cl_heating_valve_info_t* rd, heating_valve_uart_hd_t *hd, int len)
{
//	cl_heating_valve_stat_t *stat;
	heating_valve_uart_stat_param_t *sp;
	heating_valve_uart_time_t *t;
	
	log_debug("ver %u cmd %u plen %u seq %u checksum 0x%x\n", hd->ver, hd->cmd, hd->plen, hd->seq, hd->checksum);

	// 基本信息
	if (hd->cmd == 1) {
		if (hd->plen != 14 || len != sizeof(*hd) + hd->plen) {
			log_debug("plen != 14 || len %u invalid\n", len);
			return false;
		}

		sp = (heating_valve_uart_stat_param_t*)&hd[1];
		
		sp->current_temp = ntohs(sp->current_temp);
		sp->manual_temp = ntohs(sp->manual_temp);
		sp->heat_temp = ntohs(sp->heat_temp);
		sp->economy_temp = ntohs(sp->economy_temp);

		log_debug("update heating valve state\n");
		
		//
		//rd->stat.onoff = sp->onoff;
		rd->stat.error = sp->error;
		rd->stat.mode = sp->mode;
//		rd->stat.window = sp->window;
		rd->stat.windowfun = sp->windowfun;
		rd->stat.windowopen = sp->windowopen;
		//rd->stat.against = sp->against;
		//rd->stat.frost = sp->frost;
		rd->stat.child_proof = sp->child_proof;
		//rd->stat.summer_winter = sp->summer_winter;
		rd->stat.battery = sp->battery;
		//这里映射一下高低电量
		if (rd->stat.battery == 0) {
			rd->stat.battery = 102;
		} else {
			rd->stat.battery = 103;
		}
		rd->stat.current_temp = sp->current_temp;
		rd->stat.manual_temp = sp->manual_temp;
		rd->stat.heat_temp = sp->heat_temp;
		rd->stat.economy_temp = sp->economy_temp;

		log_debug("battery=%u error %u mode %u window %u against %u  current_temp %u nanual_tmep %u heat_temp %u ectemp %u\n", 
			rd->stat.battery, rd->stat.error, rd->stat.mode, rd->stat.windowfun, rd->stat.against, rd->stat.current_temp, rd->stat.manual_temp, rd->stat.heat_temp, rd->stat.economy_temp);

	} 
	// 下位机时间
	else if (hd->cmd == 3) {
		if (hd->plen != 6 || len != sizeof(*hd) + hd->plen) {
			return false;
		}

		t = (heating_valve_uart_time_t *)&hd[1];

		t->year = ntohs(t->year);

		rd->stat.year = t->year;
		rd->stat.month = t->month;
		rd->stat.day = t->day;
		rd->stat.hour = t->hour;
		rd->stat.minute =  t->minute;

		log_debug("get time: %u-%u-%u: %u:%u:%u\n", rd->stat.year, rd->stat.month, rd->stat.day, rd->stat.hour, rd->stat.minute);
	} else {
		return false;
	}

	return true;
}

static bool _rf_heating_valve_update_data(slave_t* slave, rf_tlv_t* tlv)
{
    cl_heating_valve_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.hv_info);
//    cl_heating_valve_stat_t *stat;
	heating_valve_uart_hd_t *hd;
	ucp_shortcuts_onoff_t *pso;

	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);

	log_debug("_rf_heating_valve_update_data tlv type %u\n", tlv->type);
    switch (tlv->type) {
        case UP_TLV_HV_UART_TT:
			log_debug("UP_TLV_HV_UART_TT len %u\n", tlv->len);
			
			hd = rf_tlv_val(tlv);			
			rf_heating_valve_updata_uart(rd, hd, tlv->len);
			
			break;

		case UP_TLV_HV_PERIOD:
			{ 
				int seq, index, widx, slice_len;
				u_int8_t *ptr;
				bool last_one;
				
				heating_valve_period_slice_hd_t *hd = rf_tlv_val(tlv);

				last_one = !!(hd->slice & 0x80);
				index = hd->slice & 0xf;
				seq = (hd->slice >> 4) & 0x7;
				widx = hd->slice_idx;

				
				log_debug("get period slice: slice %u seq %u last_one %u widx %u\n", index, seq, last_one, widx);

				if (tlv->len <= sizeof(*hd)) {
					log_err(false, "invalid tlv len %u\n", tlv->len);
				}

				if (seq != rd->dpc.seq) {
					log_debug("seq updata from %u => %u\n", rd->dpc.seq, seq);
					memset(&rd->dpc, 0x00, sizeof(rd->dpc));
					rd->dpc.seq = seq;
				}
				
				slice_len = tlv->len - sizeof(*hd);

				if (widx + slice_len > sizeof(rd->dpc.day_period)) {
					log_debug(false, "invalid widx %u slice_len %u\n", widx, slice_len);
					return false;
				}
				
				if (last_one) {
					rd->dpc.total_count = index + 1;					
					log_debug("updata total count %u\n", rd->dpc.total_count);
				}

				ptr = (u_int8_t*)&rd->dpc.day_period[0];
				memcpy(ptr + widx, (u_int8_t*)&hd[1], slice_len);
				rd->dpc.recv_count++;
				
				log_debug("updata bytes from %u to %u\n", widx, widx + slice_len);

				if (rd->dpc.total_count && rd->dpc.recv_count == rd->dpc.total_count) {
					log_debug("good, total count %u all received\n", rd->dpc.total_count);
					memcpy(rd->day_period, rd->dpc.day_period, sizeof(rd->day_period));
					memset(&rd->dpc, 0x00, sizeof(rd->dpc));
				} else {
					return false;
				}
			}
			break;
      	case UP_TLV_HV_GET_QUICK_TIMER:
			pso = rf_tlv_val(tlv);
			slave->shortcuts_onoff.enable = pso->enable;
			slave->shortcuts_onoff.onoff = pso->onoff;
			slave->shortcuts_onoff.remain_time = htons(pso->remain_time);
#ifdef USE_TIME_MINS
			slave->shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->time_diff * 60;
#else
			slave->shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->timezone * 3600;
#endif
			break;
        default:
            return false;
            break;
    }

    return true;
}

////////////////////////////////////////////////////////////////
// 凯特插座
static u_int8_t _rf_ktcz_stat_query_pkt(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_rf_door_lock_q_t* lq = (ucp_rf_door_lock_q_t*)(tlv+1);
    
    tlv->type = UP_TLV_KTCZ_GET_ONOFF;
    tlv->len = 0;
    
    return tlv->len;
}

static void rf_ktcz_send_cmd(slave_t* slave, u_int8_t type, u_int8_t *param, u_int8_t len)
{
	u_int8_t buf[64] = {0};
	rf_tlv_t* tlv = (rf_tlv_t*)buf;


	tlv->type = type;
	tlv->len = len;

	memcpy(rf_tlv_val(tlv), param, len);

	rfgw_send_tt_packet(slave, buf, len + sizeof(rf_tlv_t));
}

static bool _rf_ktcz_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    cl_ktcz_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.kt_info);
//    cl_ktcz_stat_t *stat;

	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    switch (tlv->type) {
        case UP_TLV_KTCZ_PUSH_ONOFF:
			if (tlv->len < sizeof(u_int8_t)) {
				break;
			}

			rd->stat.onoff = *(u_int8_t*)rf_tlv_val(tlv);

			log_debug("update ktcz onoff %u\n", rd->stat.onoff);

			break;
        default:
            return false;
    }

    return true;
}

static bool _rf_ktcz_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    int len = 0;
    u_int8_t u8value;

    switch (info->action) {
		case ACT_KTCZ_ONOFF:
			u8value = cci_u8_data(info);

			rf_ktcz_send_cmd(slave, UP_TLV_KTCZ_SET_ONOFF, &u8value, sizeof(u8value));

			log_debug("ktcz send onoff %u\n", u8value); 
			break;
		default:
			return false;
    }

	return true;
}

////////////////////////////////////////////////////////////////
// 通用探测器:气感水感探测等

static bool _rf_com_detector_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_com_detector_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cd_info);
	com_detector_cache_t *cache = (com_detector_cache_t *)param;

	if (len < sizeof(com_detector_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	rd->stat.is_support_new_history = true;

	cache->flag = htonl(cache->flag);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->hislog_count));
	
	// 更新基本信息
	rd->stat.is_alarm = !!(cache->flag & BIT(0));
	rd->stat.is_low_battery = !!(cache->flag & BIT(1));
	rd->stat.is_alarm_once = !(!!(cache->flag & BIT(2)));
	rd->stat.is_defence = !!(cache->flag & BIT(3));
	rd->stat.is_pause_alarm = !!(cache->flag & BIT(4));

	rd->stat.abc_battery = cache->abc_battery;
	
	if (rd->stat.index_alarm != cache->index_alarm) {
		log_debug("sn %"PRIu64" _rf_com_detector_update_cache_data alarm query\n", slave->sn);
		rd->stat.index_alarm = cache->index_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}

	if (rd->stat.index_time != cache->index_time) {
		log_debug("_rf_com_detector_update_cache_data time query\n");
		rd->stat.index_time = cache->index_time;
		rf_dev_query_tlv_info(slave, UP_TLV_RF_DEV_PUSH_ALARMTIME, NULL, 0);
	}

	if (rd->stat.stat_valid == false) {
		rd->stat.stat_valid = true;
		// 只查询一次软硬件版本
		rf_dev_query_tlv_info(slave, UP_TLV_RF_DEV_VERSION, NULL, 0);		
	}

	return true;
}


static void rf_com_detector_on_copy(slave_t* slave)
{
    cl_com_detector_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cd_info);

	// 如果还没有获取到电量信息，默认电量正常
	if (rd->stat.abc_battery == 0) {
		rd->stat.abc_battery = 102;
	}
}


static void rf_com_detector_send_cmd(slave_t* slave, u_int8_t type, u_int8_t *param, u_int8_t len)
{
	u_int8_t buf[64] = {0};
	rf_tlv_t* tlv = (rf_tlv_t*)buf;


	tlv->type = type;
	tlv->len = sizeof(*tlv) + len;


	memcpy(rf_tlv_val(tlv), param, len);

	rfgw_send_tt_packet(slave, buf, tlv->len);
}

static void _rf_com_detector_instert_alarm_time(cl_alarm_time_t *at, u_int8_t valid, u_int8_t type, u_int32_t time)
{
	if (!valid) {
		return;
	}

	if (type > 2) {
		return;
	}

	at->time[type] = time;

	log_debug("[%"PRIu64"]: update alarm time%d %u\n", type, time);
}

static void _rf_com_detector_instert_his(cl_com_detector_info_t* rd,u_int8_t action , u_int32_t time)
{
    int i ;
    cl_com_detector_his_t* dh,*pd;
    u_int8_t type,value;
    u_int32_t temp_time;
    
    if (!(action & BIT(0)) || time == 0){
        return;
    }
    
    type = (action >>2) & 0x3F;
    value  = (action >>1) & 0x1;
    dh = &rd->his[0];
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (dh->is_valid && dh->time_stamp == time && dh->info_type == type && dh->value == value) {
            return;
        }
    }
    
    dh = &rd->his[0];
    pd = NULL;
    temp_time = 0xFFFFFFFF;
    
    for (i = 0; i < sizeof(rd->his)/sizeof(cl_rf_door_history_t); i++,dh++) {
        if (!dh->is_valid ) {
            dh->is_valid = true;
            dh->info_type = type;
            dh->value  = value;
            dh->time_stamp = time;
            break;
        }
        
        if (dh->is_valid && dh->time_stamp < temp_time) {
            temp_time = dh->time_stamp;
            pd = dh;
        }
    }
    
    if (i >= sizeof(rd->his)/sizeof(cl_rf_door_history_t) && pd != NULL) {
        pd->is_valid = true;
        pd->info_type = type;
        pd->value  = value;
        pd->time_stamp = time;
    }
}

static void _rf_com_detector_update_history(cl_com_detector_info_t* rd, ucp_com_detector_history_t* uh)
{
    uh->time_1 = ntohl(uh->time_1);
    uh->time_2 = ntohl(uh->time_2);
    uh->time_3 = ntohl(uh->time_3);
    uh->time_4 = ntohl(uh->time_4);
    
    _rf_com_detector_instert_his(rd, uh->on_off1, uh->time_1);
    _rf_com_detector_instert_his(rd, uh->on_off2, uh->time_2);
    _rf_com_detector_instert_his(rd, uh->on_off3, uh->time_3);
    _rf_com_detector_instert_his(rd, uh->on_off4, uh->time_4);
   
}

static void com_detector_update_idx(slave_t *slave, u_int8_t index_alarm)
{
    cl_com_detector_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cd_info);

	log_debug("com_detector_update_idx update alarm idex to %u\n", index_alarm);

	rd->stat.index_alarm = index_alarm;
}

static bool _rf_com_detector_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    cl_com_detector_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cd_info);
    ucp_com_detector_stat_t *stat;
	ucp_com_detector_history_t *uh;
	ucp_com_detector_alarm_time_t *at = rf_tlv_val(tlv);
	bool need_alarm = false;
	u_int8_t battery_before = rd->stat.is_low_battery;

	log_debug("sn: %"PRIu64" update com detector data\n", slave->sn);

#if 0
	switch (slave->ext_type) {
		case RF_EXT_TYPE_GAS:
			slave->dev_info.rf_stat.d_type = D_T_GAS;
			break;
		case RF_EXT_TYPE_QSJC:
			slave->dev_info.rf_stat.d_type = D_T_QSJC;
			break;
		case RF_EXT_TYPE_HMCO:
			slave->dev_info.rf_stat.d_type = D_T_HMCO;
			break;
		case RF_EXT_TYPE_HMYW:
			slave->dev_info.rf_stat.d_type = D_T_HMYW;
			break;
		case RF_EXT_TYPE_HMQJ:
			slave->dev_info.rf_stat.d_type = D_T_HMQJ;
			break;
			break;
		default:
			return false;
	}
#endif
	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);

    switch (tlv->type) {
		case RF_TT_CMD_QUERY_HISLOG:
			return rf_update_comm_history_item(slave, rf_tlv_val(tlv), tlv->len);
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
			
		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, com_detector_update_idx);
			
        case UP_TLV_COM_DETECTOR_PUSH_SUMMARY:
			stat = rf_tlv_val(tlv);

			stat->flagbits = ntohl(stat->flagbits);

			rd->stat.hw_ver = stat->hw_ver;
			rd->stat.soft_ver_mar = (stat->soft_ver >> 7) & 0x1;
			rd->stat.soft_ver_min = (stat->soft_ver >> 4) & 0x7;
			rd->stat.soft_ver_rev = (stat->soft_ver >> 7) & 0xF;
			
			rd->stat.abc_battery = stat->abc_battery;
			rd->stat.is_alarm = (stat->flagbits) & 0x1;
			rd->stat.is_low_battery = (stat->flagbits >> 1) & 0x1;
			rd->stat.is_defence = (stat->flagbits >> 3) & 0x1;
			rd->stat.is_pause_alarm = (stat->flagbits >> 4) & 0x1;
			rd->stat.is_alarm_once = (stat->flagbits >> 5) & 0x1;
			
#if 1
			// 布防状态下才推送报警EVENT
			if (rd->stat.is_defence && slave->dev_info.rf_stat.d_type != D_T_HMQJ) {
				if (rd->stat.is_alarm) {
					need_alarm = true;
					rd->alarm_info.value = 1;
					rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
					rd->alarm_info.info_type = CD_ALARM_INFO_TYPE_DEV;
				} else if (battery_before == 0 && rd->stat.is_low_battery) {
					need_alarm = true;
					rd->alarm_info.value = 1;
					rd->alarm_info.time_stamp = (u_int32_t)time(NULL);
					rd->alarm_info.info_type = CD_ALARM_INFO_TYPE_BATTERY;
				}

				// 在免扰模式下，不报警
				if (rd->alarm_time.time[0] != 0 && (u_int32_t)time(NULL) < rd->alarm_time.time[0]) {
					need_alarm = false;
				}

				if (need_alarm) {					
					event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
					event_push(slave->user->callback, SAE_RF_DEV_ALARM_INFO, slave->handle, slave->user->callback_handle);
				}
			}
#endif 			
			break;
			
		case UP_TLV_COM_DETECTOR_PUSH_ONOFFTIME:
			uh = rf_tlv_val(tlv);
			if (tlv->len < sizeof(*uh)) {
				return false;
			}
			
            _rf_com_detector_update_history(rd,uh);
			
			break;

		case UP_TLV_RF_DEV_PUSH_ALARMTIME:
			_rf_com_detector_instert_alarm_time(&rd->alarm_time, at->ctl_1 & 1, (at->ctl_1 >> 1) & 0x7, ntohl(at->time_1));
			_rf_com_detector_instert_alarm_time(&rd->alarm_time, at->ctl_2 & 1, (at->ctl_2 >> 1) & 0x7, ntohl(at->time_2));
			_rf_com_detector_instert_alarm_time(&rd->alarm_time, at->ctl_3 & 1, (at->ctl_3 >> 1) & 0x7, ntohl(at->time_3));
			_rf_com_detector_instert_alarm_time(&rd->alarm_time, at->ctl_4 & 1, (at->ctl_4 >> 1) & 0x7, ntohl(at->time_4));
			break;

		case UP_TLV_RF_DEV_VERSION:
			{
				ucp_com_detector_version_t *version = rf_tlv_val(tlv);
				if (tlv->len < sizeof(*version)) {
					return false;
				}

				rd->stat.hw_ver = version->hw_ver;
				rd->stat.soft_ver_mar = (version->soft_ver >> 7) & 0x1;
				rd->stat.soft_ver_min = (version->soft_ver >> 4) & 0x7;
				rd->stat.soft_ver_rev = (version->soft_ver >> 7) & 0xF;
				rd->stat.svn = ntohl(version->svn);
			}
			break;
			
        default:
            return false;
    }

	log_debug("sn: %"PRIu64" update com detector data done\n", slave->sn);

    return true;
}

static bool _rf_com_detector_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0,type;
    u_int32_t value;
	ucp_rf_com_detector_ctrl_t *dc = (ucp_rf_com_detector_ctrl_t *)&tlv[1];
    cl_com_detector_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cd_info);
	
    switch (info->action) {
        case ACT_RF_COM_DIRECT_CTRL:
            tlv->len = sizeof(*dc);
            value = cci_u32_data(info);
            type = (value >> 16) & 0xFF;
            
            switch (type) {
                case ACT_COM_DETECTOR_CTRL_DEFENSE:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                    tlv->len = sizeof(*dc);

					dc->value = !!(value & 0xFF);
					dc->pad = 0xff;
					memcpy(rf_tlv_val(tlv), dc, sizeof(*dc));

					len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存值
					rd->stat.is_defence = dc->value;
                    break;

				case ACT_COM_DETECTOR_CTRL_REPORT_FREQ:
                    tlv->type = UP_TLV_SMTDLOCK_CMD_SET_DEFENSE;
                    tlv->len = sizeof(*dc);

					dc->value = 0xff;
					dc->pad = (value & 0xFF);
					memcpy(rf_tlv_val(tlv), dc, sizeof(*dc));
					
                    len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存值
					rd->alarm_time.time[2] = 5 * dc->pad;
                    break;

				case ACT_COM_DETECTOR_CTRL_ALARM_TYPE:
					tlv->type = UP_TLV_RF_DEV_SET_ALARM_TYPE;
                    tlv->len = sizeof(*dc);

					dc->value = (u_int8_t)value;
					memcpy(rf_tlv_val(tlv), dc, sizeof(*dc));
					
                    len = sizeof(*tlv)+sizeof(*dc);

					// 先修改内存值
					rd->stat.is_alarm_once = dc->value;
					break;

				case ACT_COM_DETECTOR_CTRL_ALARM_DEMO:
					tlv->type = UP_TLV_RF_DEV_ALARM_DEMO;
                    tlv->len = sizeof(*dc);

					dc->value = 1;
					memcpy(rf_tlv_val(tlv), dc, sizeof(*dc));
					
                    len = sizeof(*tlv)+sizeof(*dc);
					break;
					
                default:
                    len = 0;
                    break;
            }
            break;
        case ACT_RF_COM_QUERY_HISTORY:
            return _rf_proc_query_his_notify(slave, info, ret);

		case ACT_RF_COM_ALARM_TIME:
			{
				ucp_rf_alarm_time_t *request = cci_pointer_data(info);

				// 先修改内存值
				if ((request->ctrl & 1) == 1) {
					rd->alarm_time.time[0] = request->time;
					log_debug("sn: %"PRIu64" com detect: set alarm time0 %u\n", slave->sn, request->time);
				} else {
					rd->alarm_time.time[1] = request->time;
				}
				
				request->time = ntohl(request->time);

				tlv->type = UP_TLV_RF_DEV_SET_ALARMTIME;
                tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));

				len = sizeof(*tlv)+sizeof(*request);				
			}
			break;

		case ACT_RF_COM_ALARM_CLC:
			{
				ucp_rf_alarm_clc_t request;

				memset(&request, 0x00, sizeof(request));

				request.alarm_clc = 1;

				tlv->type = UP_TLV_RF_DEV_SET_ALARMCLC;
                tlv->len = sizeof(request);
				memcpy(rf_tlv_val(tlv), (u_int8_t*)&request, sizeof(request));

				len = sizeof(*tlv) + sizeof(request);	
			}
			break;
			
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////
static void do_jq_his_query(slave_t* slave)
{
	int i, len;
	u_int8_t query_index = 0;
	u_int8_t buf[128];
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	u_int8_t *pvalue = (u_int8_t *)&tlv[1];
	cl_jq_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.jq_info);

	for(i = 0; i < rd->n_item; i++) {
		if (rd->query_map[i] == 0) {
			continue;
		}
		query_index = i;
		break;
	}

	if (i >= rd->n_item) {
		return;
	}

	log_info("queru index=%u\n", query_index + 1);
	tlv->type = UP_TLV_JQ_GET_CH2O_HISTORY;
	tlv->len = 1;
	*pvalue = query_index + 1;
	
	len = sizeof(*tlv) + tlv->len;
	
	rfgw_send_tt_packet(slave,buf,len);
}

// 甲醛传感器
static bool _rf_jq_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_jq_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.jq_info);
	jq_cache_t *cache = (jq_cache_t *)param;

	if (len < sizeof(jq_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}
	rd->stat.valid = 1;

	cache->cur_ch2o = ntohs(cache->cur_ch2o);
	cache->thr_ch2o = ntohs(cache->thr_ch2o);
	cache->alarm_period = ntohs(cache->alarm_period);
	
	cache->hislog_count = ntohs(cache->hislog_count);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->hislog_count));
	
	// 更新基本信息
	rd->stat.cur_ch2o = cache->cur_ch2o;
	rd->stat.battery = cache->battery;
	if (rd->stat.battery < 5) {
		rd->stat.battery = 5;
	}
	rd->stat.std = cache->std;
	rd->stat.thr_ch2o = cache->thr_ch2o;
	rd->stat.period = cache->alarm_period;

	if (rd->stat.v_alarm != cache->v_alarm) {
		rd->stat.v_alarm = cache->v_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}

	log_debug("%s rd->v_ch2o_history=%u v_ch2o_history=%u n_ch2o_history=%u\n", __FUNCTION__, rd->v_ch2o_history, cache->v_ch2o_history, cache->n_ch2o_history);
	//平均浓度查询
	if (rd->v_ch2o_history != cache->v_ch2o_history) {
		//这里如果一次性查询几个不知道会不会丢包，还是一个一个查询吧
		rd->v_ch2o_history = cache->v_ch2o_history;
		rd->n_item = (int)cache->n_ch2o_history;
		if (rd->n_item > MAX_JQ_HISTORY_ITEM_NUM) {
			rd->n_item = MAX_JQ_HISTORY_ITEM_NUM;
		}
		memset((void *)rd->query_map, 0, sizeof(rd->query_map));
		memset((void *)rd->query_map, 1, rd->n_item);
		do_jq_his_query(slave);
	}
	
	return true;
}

static void jq_alarm_update_idx(slave_t *slave, u_int8_t index_alarm)
{
    cl_jq_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.jq_info);

	rd->stat.v_alarm = index_alarm;
}

static bool rf_update_his(slave_t *slave, u_int8_t *data, u_int8_t len)
{
	int i,n;
	u_int8_t index = 0;
	bool ret = false;
	ucp_rf_jq_his_item_t *pitem;
	ucp_rf_jq_his_query_t *pjhq = (ucp_rf_jq_his_query_t *)data;
	cl_jq_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.jq_info);

	if (len < sizeof(*pjhq)) {
		log_info("error len=%u\n", len);
		return false;
	}

	if (pjhq->v_history_index != rd->v_ch2o_history) {
		log_info("need requery  pjhq->v_history_index=%u rd->v_ch2o_history=%u\n", 
			pjhq->v_history_index, rd->v_ch2o_history);
		//这里如果一次性查询几个不知道会不会丢包，还是一个一个查询吧
		rd->v_ch2o_history = pjhq->v_history_index;
		memset((void *)rd->query_map, 0, sizeof(rd->query_map));
		memset((void *)rd->query_map, 1, rd->n_item);
		do_jq_his_query(slave);
		return false;
	}

	log_info("rd->v_ch2o_history=%u pjhq->v_history_index=%u\n", rd->v_ch2o_history, pjhq->v_history_index);
	
	rd->v_ch2o_history = pjhq->v_history_index;
	n = (len - sizeof(*pjhq))/sizeof(*pitem);
	if (n == 0) {
		return false;
	}

	log_debug("enter rf_update_his\n");
	for(i = 0; i < n; i++) {
		pitem = &pjhq->item[i];
		pitem->timestamp = htonl(pitem->timestamp);
		pitem->ch20 = htons(pitem->ch20);
		//check
		if (pjhq->hislog_index) {
			index = pjhq->hislog_index + i - 1;
		} else {
			index = pjhq->hislog_index + i;
		}
		if (index >= MAX_JQ_HISTORY_ITEM_NUM) {
			log_debug("%s error pjhq->hislog_index=%u\n", __FUNCTION__, index);
			continue;
		}


		rd->items[index].ch2o = pitem->ch20;
		rd->items[index].time = pitem->timestamp;
		rd->query_map[index] = 0;
		ret = true;
		log_debug("index=%d pjhq->ch20=%u time=%u\n", index, pitem->ch20, pitem->timestamp);
	}

	if (ret) {
		do_jq_his_query(slave);
	}

	return ret;
}

static bool _rf_jq_update_data(slave_t* slave,rf_tlv_t* tlv)
{
    cl_jq_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.jq_info);

	log_debug("sn: %"PRIu64" _rf_jq_update_data data type=%u len=%u\n", slave->sn, tlv->type, tlv->len);

	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);

    switch (tlv->type) {
		case RF_TT_CMD_QUERY_HISLOG:
			return rf_update_comm_history_item(slave, rf_tlv_val(tlv), tlv->len);
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
			
		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, jq_alarm_update_idx);
		case UP_TLV_JQ_GET_CH2O_HISTORY:
			return rf_update_his(slave, rf_tlv_val(tlv), tlv->len);
			break;
        default:
            return false;
    }

	log_debug("sn: %"PRIu64" update jq detector data done\n", slave->sn);

    return true;
}

static bool _rf_jq_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0;
    u_int32_t value;
    cl_jq_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.jq_info);
	
    switch (info->action) {
        case ACT_JQ_SET_ALARM_THR:
			{
				ucp_rf_jq_set_thr_t request;
				
				value = cci_u32_data(info);

				request.std = value & 0xff;
				request.thr = ntohs((value >> 16) & 0xffff);

				tlv->type = UP_TLV_JQ_SET_ALARM_THR;
				tlv->len = sizeof(request);
				memcpy((u_int8_t*)(tlv + 1), &request, tlv->len);

				len = tlv->len + sizeof(*tlv);

				//
				rd->stat.std = value & 0xff;
				rd->stat.thr_ch2o = (value >> 16) & 0xffff;
			}
			break;
			
		case ACT_JQ_SET_ALARM_PERIOD:
			{
				u_int16_t request;
				
 				request = (u_int16_t)cci_u32_data(info);
				request = ntohs(request);

				tlv->type = UP_TLV_JQ_SET_ALARM_PERIOD;
				tlv->len = sizeof(request);
				memcpy((u_int8_t*)(tlv + 1), &request, tlv->len);

				len = tlv->len + sizeof(*tlv);

				//
				rd->stat.period = (u_int16_t)cci_u32_data(info);
			}
			break;

		case ACT_JQ_FLUSH_CH2O:
			{
				tlv->type = UP_TLV_JQ_FLUSH_CH2O;
				tlv->len = 0;

				len = tlv->len + sizeof(*tlv);
			}
			break;
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}

////////////////////////////////////////////////////////////////////////
//智能门磁v2查询
static void rf_door_manage_query_his(slave_t* slave, u_int32_t new_timestamp)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	u_int32_t now = (u_int32_t)time(NULL);
	cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);

	//如果比历史记录还小就不查询了
	new_timestamp++;

	//判断下发送计数
	if (rd->send_num > 0) {
		log_debug("%s rd->send_num=%u\n", __FUNCTION__, rd->send_num);
		if (now < (rd->send_timeout + 10)) {
			log_debug("rd->send_timeout=%u now=%u !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n", rd->send_timeout, now);
			return;
		}
	}

	rd->send_num++;
	rd->send_timeout = now;

	tlv->type = UP_TLV_SMTDLOCK_CMD_GET_ONOFFTIME;
	tlv->len = sizeof(u_int32_t);
	
	*(u_int32_t *)rf_tlv_val(tlv) = htonl(new_timestamp);;
	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query rf_door_manage_query_his new_timestamp=%u\n", new_timestamp);
}

static void rf_door_manage_query_autodefense(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = UP_TLV_SMTDLOCK_CMD_GET_AUTO_DEFENSE;
	tlv->len = 0;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query rf_door_manage_query_autodefense\n");
}

static void rf_door_manage_query_alarm(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = RF_TT_CMD_PUSH_ALARM;
	tlv->len = 2;

	*(u_int8_t*)rf_tlv_val(tlv) = 1;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query rf_door_manage_query_alarm\n");
}

static bool _rf_door_magnetv2_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_door_magnet_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.door_magnet_info);
	door_manage_v2_cache_t *cache = (door_manage_v2_cache_t *)param;
	u_int32_t max_timestamp = 0;
	bool modify = false;
	u_int8_t value;
	
	if (len < sizeof(door_manage_v2_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	// 支持TT CACHE的设备，认为支持新的历史记录查询
	rd->stat.is_support_new_history = true;

	cache->flag = htonl(cache->flag);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->hislog_count));
	
	log_debug("v2 flag=%08x abc=%u an=%u na=%u al=%u la=%u !!!!!!!!!!!!!!\n", 
		cache->flag, cache->abc_battery, cache->index_alarm, cache->index_autodefense, 
		rd->index_alarm, rd->index_autodefense);

	log_debug("rd->stat.is_door_open=%u rd->stat.is_break_door=%u rd->stat.is_battary_warn=%u\n", 
		rd->stat.is_door_open, rd->stat.is_break_door, rd->stat.is_battary_warn);
    if (rd->stat.stat_valid == true && !!(cache->flag & BIT(3))) {
        if(!rd->stat.is_door_open && !!(cache->flag & BIT(0))){
			log_debug("is_door_open !!!!!!!!!!!!!!1\n");
        }
        
        if (!rd->stat.is_break_door && !!(cache->flag & BIT(2)) ) {
			log_debug("is_break_door !!!!!!!!!!!!!!1\n");
        }
    }

	//低电量报警
	if (!!(cache->flag & BIT(3)) &&
		!rd->stat.is_battary_warn && !!(cache->flag & BIT(1))) {
		log_debug("is_battary_warn !!!!!!!!!!!!!!1\n");
	}
	
	if (rd->stat.battary != cache->abc_battery) {
		rd->stat.battary = cache->abc_battery;
		if (rd->stat.battary < 5) {
			rd->stat.battary = 5;
		}
		modify = true;
	}
	
	value = !!(cache->flag&BIT(0));
	if (rd->stat.is_door_open != value) {
		rd->stat.is_door_open = value;
		modify = true;
	}

	value = !!(cache->flag&BIT(1));
	if (rd->stat.is_battary_warn != value) {
		rd->stat.is_battary_warn = value;
		modify = true;
	}

	value = !!(cache->flag&BIT(2));
	if (rd->stat.is_break_door != value) {
		rd->stat.is_break_door = value;
		modify = true;
	}	

	value = !!(cache->flag&BIT(3));
	if (rd->stat.is_guard != value) {
		rd->stat.is_guard = value;
		modify = true;
	}	

#if 0
	if (rd->index_hislog != cache->index_hislog) {
		log_debug("rf_door_manage_query_his on=%u nn=%u\n", 
			rd->index_hislog, cache->index_hislog);
		rd->index_hislog = cache->index_hislog;
		max_timestamp = rf_dorr_manage_get_max_timestamp(rd);
		rf_door_manage_query_his(slave, max_timestamp);
	}
#else
	if (rd->index_alarm != cache->index_alarm) {
		log_debug("_rf_door_magnetv2_update_cache_data alarm query %u => %u\n", rd->index_alarm, cache->index_alarm);
		rd->index_alarm = cache->index_alarm;
			
		rf_door_manage_query_alarm(slave);
	}
#endif	

	if (rd->index_autodefense != cache->index_autodefense) {
		rd->index_autodefense = cache->index_autodefense;
		rf_door_manage_query_autodefense(slave);
	}

	rd->stat.stat_valid = true;

	return modify;
}
/////////////////////////////////////////////////////////////////////////
// 光照感应
static bool _rf_light_sense_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_light_sense_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.ls);
	light_sense_cache_t *cache = (light_sense_cache_t *)param;
	u_int32_t flag = 0;

	log_debug("enter %s\n", __FUNCTION__);
	if (len < sizeof(light_sense_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	cache->flags = ntohl(cache->flags);
	cache->light_val = ntohs(cache->light_val);

	info->stat.battery = cache->battery;
	if (info->stat.battery < 5) {
		info->stat.battery = 5;
	}
	info->stat.light_level = cache->light_level;
	info->stat.light_val = cache->light_val;

	return true;
}

/////////////////////////////////////////////////////////////////////////
//橙灯情景面板
static void _rf_cdqjmb_conf_query(slave_t* slave, u_int8_t index)
{
	char buf[1024] = {0};
	rf_tlv_t *prtlv = (rf_tlv_t *)buf;
	u_int8_t *pvalue = (u_int8_t *)&prtlv[1];

	prtlv->type = UP_TLV_CDQJMB_NAME_QUERY;
	prtlv->len = 1;
	*pvalue = index;

	log_debug("%s _rf_cdqjmb_conf_query UP_TLV_CDQJMB_NAME_QUERY to sn=%"PRIu64" index %u\n", 
		slave->user->name, slave->sn, index);
	rfgw_send_tt_packet(slave, buf, prtlv->len + sizeof(*prtlv));
}

static bool _rf_cdqjmb_valve_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_cdqjmb_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cdqjmb_info);
	cdqjmb_cache_t *cache = (cdqjmb_cache_t *)param;
	u_int32_t max_timestamp = 0;
	u_int32_t flag = 0;
	cl_cdqjmb_info_t rdc;
	int i;

	log_debug("enter %s\n", __FUNCTION__);
	if (len < sizeof(cdqjmb_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	if (!rd->is_valid &&
		(RS_OK == _rf_com_load_user_info(slave, CDQJMB_CONF_ID, (u_int8_t *)&rdc, sizeof(rdc)))) {
		memcpy((void *)rd, (void *)&rdc, sizeof(*rd));
	}

	rd->is_valid = true;
	cache->flag_len = htonl(cache->flag_len);
	cache->hislog_cur_index = ntohl(cache->hislog_cur_index);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_cur_index, (u_int32_t)(cache->hislog_count));
	//parse
	len = cache->flag_len&0xff;
	flag = (cache->flag_len>>8)&0xffffff;

	rd->flag = flag;

	//memdumpone("cache", cache, sizeof(*cache));
	//联动规则数据
	rd->ice_rule_maxnum = cache->ice_rule_maxnum;
	rd->ice_rule_curnum = cache->ice_rule_curnum;
	rd->key_num = cache->key_num;
	log_debug("%s keynum=%u\n", __FUNCTION__, rd->key_num);
	if (rd->key_num > CDQJMB_KEY_MAXNUM) {
		rd->key_num = CDQJMB_KEY_MAXNUM;
	}
	//按键配置解析
	for(i = 0; i < rd->key_num; i++) {
		log_debug("%s i=%d lc=%u rc=%u\n", __FUNCTION__, i, rd->key_changed[i], cache->key_changed[i]);
		if (rd->key_changed[i] != cache->key_changed[i]) {
			rd->key_changed[i] = cache->key_changed[i];
			// query;
			_rf_cdqjmb_conf_query(slave, (u_int8_t)i+1);
		}
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////
//空调贴
static void rf_wkair_query_shortcuts(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = UP_TLV_WKAIR_SHORTCUTS_QUERY;
	tlv->len = 0;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query %s type=%u\n", __FUNCTION__, tlv->type);
}

static void rf_wkair_query_status(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = UP_TLV_SMTDLOCK_CMD_GET_SUMMARY;
	tlv->len = 0;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query %s\n", __FUNCTION__);
}

static void rf_query_comm_timer(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = RF_TT_CMD_GET_TIMER_SUMMARY;
	tlv->len = 0;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query %s\n", __FUNCTION__);
}

static void rf_wkair_query_ir(slave_t* slave)
{
	log_debug("query %s\n", __FUNCTION__);
	rf_air_ir_start_db_send(slave);	
	rf_slave_recv_big_pkt_v2(slave, RF_BIG_PKT_IR_COLLECTION, rbmp_callback);
}

//空调贴控制有些空调需要知道地址
#define RF_WKAIR_ADDR_MIN (1)
#define RF_WKAIR_ADDR_MAX (16)

void _rf_wkair_query_ir_code(slave_t *slave, u_int8_t index)
{		
    cl_wk_air_info_t* ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);
	u_int8_t data[256] = {0};
	u_int16_t data_len = 0;
	rf_tlv_t *tlv = (rf_tlv_t *)data;
	wk_air_ir_code_get_t *request = (wk_air_ir_code_get_t *)&tlv[1];
	u_int16_t ir_id = ai->stat.ir_id;

	log_debug("wkair query ir code : ir_id %u sync_num %u offset %u\n", ir_id, ai->sync_num, index);

	if (ir_id == 0) {
		log_err(false, "not get ir id\n");
		return;
	}

	tlv->type = UP_TLV_WKAIR_GET_IR_CODE;
	tlv->len = sizeof(*request);

	request->offset = index;
	request->ir_id = ntohs(ir_id);
	request->sync_num = ai->sync_num;

	data_len = sizeof(*tlv) + sizeof(*request);

	rfgw_send_tt_packet(slave, data, data_len);
}

static bool _rf_wkair_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	wk_air_cache_t *pwac = (wk_air_cache_t *)param;
    cl_wk_air_info_t* ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);
	priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
	u_int8_t value;
	bool changed = false;
	
	if (len < sizeof(wk_air_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	log_debug("enter _rf_wkair_update_cache_data\n");
	log_debug("osh=%u ost=%u oco=%u nsh=%u nst=%u nco=%u "
		"slave->ir_num=%u pwac->ir_num=%u pwac->onoff=%u sync_num %u ir_sync_ctrl %u\n", 
		ai->shortcuts_count, ai->status_count, ai->comm_timer_count, 
		pwac->shortcuts_count, pwac->status_count, pwac->comm_timer_count, 
		slave->ir_num, pwac->ir_num, pwac->onoff, pwac->sync_num, pwac->ir_sync_ctrl);

	if (pwac->addr == 0xff) {
		ai->stat.support_ir_sync_ctrl = true;
	}

	//处理下非法数据
    if (pwac->temp < 16) {
        pwac->temp = 16;
    }
    if (pwac->temp > 32) {
        pwac->temp = 32;
    }
	if (pwac->mode > AC_MODE_HOT) {
		pwac->mode = AC_MODE_HOT;
	}
	if (pwac->wind> AC_WIND_3) {
		pwac->wind = AC_WIND_3;
	}
	if (pwac->wind_direct> AC_DIR_3) {
		pwac->wind_direct = AC_DIR_3;
	}
	if (pwac->addr < RF_WKAIR_ADDR_MIN) {
		pwac->addr = RF_WKAIR_ADDR_MIN;
	}
	if (pwac->addr > RF_WKAIR_ADDR_MAX) {
		pwac->addr = RF_WKAIR_ADDR_MAX;
	}

    ai->stat.battary = pwac->battary;
	if (ai->stat.battary < 5) {
		ai->stat.battary = 5;
	}
    ai->stat.room_humi = pwac->room_humi;
    ai->stat.room_temp = (int8_t)pwac->room_temp;

    // save stat
    if (cache->onoff != pwac->onoff) {
		cache->onoff = ai->stat.onoff = !!pwac->onoff;
		changed = true;
	}

	if (ai->stat.addr != pwac->addr) {
		ai->stat.addr= pwac->addr;
		changed = true;
	}

	if (cache->mode != pwac->mode) {
    	cache->mode = ai->stat.mode = pwac->mode;
		changed = true;
	}
	if (cache->temp != pwac->temp) {
    	cache->temp = ai->stat.temp = pwac->temp;
		changed = true;
	}
	if (cache->wind != pwac->wind) {
    	cache->wind = ai->stat.wind = pwac->wind;
		changed = true;
	}
	if (cache->wind_direct != pwac->wind_direct) {
    	cache->wind_direct = ai->stat.wind_direct = pwac->wind_direct;
		changed = true;
	}

	if (ai->shortcuts_count != pwac->shortcuts_count) {
		ai->shortcuts_count = pwac->shortcuts_count;
		slave->is_support_public_shortcuts_onoff = true;
		rf_wkair_query_shortcuts(slave);
	}
	
	if (ai->status_count != pwac->status_count) {
		ai->status_count = pwac->status_count;
		rf_wkair_query_status(slave);
	}
	
	if (ai->comm_timer_count != pwac->comm_timer_count) {
		ai->comm_timer_count = pwac->comm_timer_count;
		rf_query_comm_timer(slave);
	}

	if ((slave->ir_num > 0) && 
		(slave->ir_num == pwac->ir_num)) {
		slave->ir_num = 0;
		rf_wkair_query_ir(slave);
	}

	if (ai->sync_num != pwac->sync_num) {
		log_debug("sync num %u -> %u\n", ai->sync_num, pwac->sync_num);
		ai->sync_num = pwac->sync_num;

		if (ai->stat.ir_id == cache->cur_dev_ir_id) {
			_rf_wkair_query_ir_code(slave, 0);
		}
	}

	// 通过前面addr 为0xff判断这个悟空支持同步协议
	if (ai->stat.support_ir_sync_ctrl == 0) {
		goto done;
	}

	if (pwac->ir_sync_ctrl != 0) {
		if (pwac->ir_sync_ctrl == 1) {
			value = 1;
		} else {
			value = 0;
		}

		if (ai->stat.ir_sync_ctrl != value) {
			ai->stat.ir_sync_ctrl = value;
			changed = true;
		}
	}

done:	
	return changed;
}

/////////////////////////////////////////////////////////////////////////
//智皇电机
static bool _rf_zhdj_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	zhdj_cache_t *pzc = (zhdj_cache_t *)param;
    cl_zhdj_info_t* zi = &(slave->dev_info.rf_stat.dev_priv_data.zhdj_info);

	// 为了参展，智皇MACBEE强制支持联动
	slave->is_support_la = true;

	if (len < sizeof(zhdj_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	zi->magic = htonl(pzc->magic);
	zi->index = pzc->index;
	if (pzc->percent > 100) {
		pzc->percent = 100;
	}
	zi->percent = 100 - pzc->percent;
	//zi->status = pzc->status;
	zi->status = pzc->status;
	zi->type = pzc->type;
	zi->support_dir = pzc->support_dir;
	zi->dir = pzc->dir;


	//智皇电机这里有点奇怪，在cache里放的定时器摘要,处理下吧,且判断一下，只支持3个定时器
	if (comm_timer_support(slave->ext_type)) {
		comm_timer_summary_proc(slave, &pzc->max_timer_count);
	}	

	return true;
}


////////////////////////////////////////////////////////////////
// 汇泰龙锁

static void rf_htllock_query_user_manage(slave_t* slave, u_int16_t index)
{
	u_int8_t buf[256] = {0};
	htllock_tt_user_manage_get_info_t *request;
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = HTLLOCK_USER_MANAGE;
	tlv->len = sizeof(*request);

	request = (htllock_tt_user_manage_get_info_t *)&tlv[1];
	request->op = HTLLOCK_USER_MANAGE_OPT_TYPE_QUERY;
	request->get_userindex = ntohs(index);

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query rf_htllock_query_user_manage index %u\n", index);
}

static void rf_htllock_query_notice_info(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = HTLLOCK_INFO_NOTICE;
	tlv->len = 2;

	*(u_int8_t*)rf_tlv_val(tlv) = HTLLOCK_INFO_NOTICE_OPT_TYPE_QUERY;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query rf_htllock_query_notice_info\n");
}

static void rf_htllock_query_alarm(slave_t* slave)
{
	u_int8_t buf[256] = {0};
	rf_tlv_t *tlv = (rf_tlv_t *)buf;

	tlv->type = RF_TT_CMD_PUSH_ALARM;
	tlv->len = 2;

	*(u_int8_t*)rf_tlv_val(tlv) = 1;

	rfgw_send_tt_packet(slave, buf, tlv->len + sizeof(*tlv));

	log_debug("query rf_htllock_query_alarm\n");
}

// 从文件里面尝试读取用户匹配信息
static RS _rf_htllock_load_user_info(slave_t* slave)
{
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	htllock_user_info_flash_t flash;
	char path[256] = {0}, buf[512] = {0};
	int len;
	FILE *fp = NULL;

	sprintf(path, "%s/%"PRIu64"/htllock_user_info.info", cl_priv->dir, slave->sn);

	if ((fp = fopen(path, "rb")) == NULL) {
		log_err(false, "user info file [%s] not created..\n", path);
		goto def;
	}

	len = (int)fread((u_int8_t*)&flash, 1, sizeof(flash), fp);
	if (len != sizeof(flash)) {
		log_err(false, "read len %d invalid != sizeof(flash) %d\n", len, sizeof(flash));
		goto def;
	}

	if (flash.magic != USER_INFO_FLASH_MAGIC) {
		log_err(false, "invalid magic 0x%x\n", flash.magic);
		goto def;
	}

	memcpy(hl_info->user_manage, flash.user_manage, sizeof(hl_info->user_manage));
	hl_info->index_user = flash.index_user;

	log_debug("htllock get user info successed, index user %u\n", hl_info->index_user);

	fclose(fp);

	return RS_OK;
def:
	if (fp) {
		fclose(fp);
	}
	
	log_debug("no user info, use default\n");

	hl_info->index_user = 0;
	hl_info->user_count = 0;
	hl_info->user_write_index = 0;

	return RS_ERROR;
}

static void _rf_htllock_write_user_info(slave_t *slave)
{
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	htllock_user_info_flash_t flash;
	char path[256] = {0};
	FILE *fp;
	int len;

	flash.magic = USER_INFO_FLASH_MAGIC;
	flash.index_user = hl_info->index_user;

	memcpy((u_int8_t*)(flash.user_manage), (u_int8_t*)(hl_info->user_manage), sizeof(flash.user_manage));

	sprintf(path, "%s/%"PRIu64"", cl_priv->dir, slave->sn);
	MKDIR(path, 0777);
	
	sprintf(path, "%s/%"PRIu64"/htllock_user_info.info", cl_priv->dir, slave->sn);

	if ((fp = fopen(path, "w+b")) == NULL) {
		log_err(true, "fopen path [%s] failed\n", path);
		return;
	}

	len = (int)fwrite((u_int8_t*)&flash, 1, sizeof(flash), fp);

	log_debug("_rf_htllock_write_user_info ret len %d path[%s]\n", len, path);

	fclose(fp);
}

// 更新cache数据
static bool _rf_htllock_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	htllock_tt_cache_t *cache = (htllock_tt_cache_t *)param;
	u_int32_t onoff;
	u_int8_t buf[64] = {0};
	int buf_len;
	bool modify = false;

	if (len < sizeof(*cache)) {
		return false;
	}

	cache->devstat_bits = ntohl(cache->devstat_bits);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);
	cache->index_user = ntohs(cache->index_user);
	cache->user_count = ntohs(cache->user_count);

	log_debug("%s htllock update cache: index_user %u user_count %u\n", 
		__FUNCTION__, cache->index_user, cache->user_count);

	onoff = cache->devstat_bits & 1;
	if (hl_info->lock_stat.onoff != onoff) {
		hl_info->lock_stat.onoff = onoff;
		modify = true;
	}

	if (hl_info->get_cache == false) {
		hl_info->get_cache = true;
		
		// 1、第一次情况下，先从文件里面尝试读取
		if (_rf_htllock_load_user_info(slave) == RS_OK) {
			modify = true;
		}

		// 顺便查询下音量和语言
		buf_len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0x03, 0x01, 0, 0);
		rfgw_send_tt_packet(slave, buf, buf_len);
	}

	// 2、判断和之前获取到的用户信息变化数是否一致，不一致需要重新获取
	//这里处理下，如果网关支持大量用户查询，就不跑这个tt查询了
	log_info("sn=%"PRIu64" is_support_batch_user=%u hl_info->index_user=%u cache->index_user=%u\n", 
	slave->sn, slave->user->is_support_batch_user, hl_info->index_user, cache->index_user);
	if (!(slave->user && slave->user->is_support_batch_user) && 
		(hl_info->index_user != cache->index_user)) {
		log_debug("local index user %u != cache index user %u query it\n", 
			hl_info->index_user, cache->index_user);
		hl_info->index_user = cache->index_user;
		hl_info->user_count = cache->user_count;
		
		rf_htllock_query_user_manage(slave, 1);
	}
	
	/*
		 获取历史记录信息
	*/
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)cache->hislog_count);
	
#if 0
	// 查看变化，发送对应的查询命令
	if (cache->index_user != hl_info->cache.index_user) {		
		log_debug("update htl index_user %u => %u\n", hl_info->cache.index_user, cache->index_user);
		hl_info->cache.index_user = cache->index_user;
		rf_htllock_query_user_manage(slave);
	}
#endif

	if (cache->index_notice != hl_info->cache.index_notice) {		
		log_debug("update htl index_notice %u => %u\n", hl_info->cache.index_notice, cache->index_notice);
		hl_info->cache.index_notice = cache->index_notice;		
		rf_htllock_query_notice_info(slave);
	}

	log_debug("updata htllock cache: index alarm %u\n", hl_info->cache.index_alarm);
	if (cache->index_alarm != hl_info->cache.index_alarm) {		
		log_debug("htllock update htl index_alarm %u => %u\n", hl_info->cache.index_alarm, cache->index_alarm);
		hl_info->cache.index_alarm = cache->index_alarm;		
		rf_htllock_query_alarm(slave);
	}
	
	return modify;
}

static htllock_user_manage_stat_t * rf_htllock_user_get_by_index(cl_htllock_info_t *hl_info, u_int16_t index)
{
	int i;

	for (i = 0; i < HTLLOCK_MAX_USER_NUM; i++) {
		if (hl_info->user_manage[i].index == index) {
			return &hl_info->user_manage[i];
		}
	}

	return NULL;
}

static htllock_user_manage_stat_t * rf_htllock_user_get_by_createid(cl_htllock_info_t *hl_info, u_int16_t create_id)
{
	int i;

	for (i = 0; i < HTLLOCK_MAX_USER_NUM; i++) {
		if (hl_info->user_manage[i].create_id == create_id) {
			return &hl_info->user_manage[i];
		}
	}

	return NULL;
}


/**
	
*/
static bool rf_htllock_user_add(cl_htllock_info_t *hl_info, htllock_tt_user_manage_push_t *ump)
{
	int i;
	int smallerid = HTLLOCK_MAX_USER_NUM;

	if (ump->create_id == 0) {
		return false;
	}

	for (i = 0; i < HTLLOCK_MAX_USER_NUM; i++) {
		if (hl_info->user_manage[i].create_id) {
			if (hl_info->user_manage[i].create_id == ump->create_id) {
				break;
			}
		}else{
			if(smallerid == HTLLOCK_MAX_USER_NUM){
				smallerid = i;
			}
		}
	}
	
	if(i == HTLLOCK_MAX_USER_NUM){
		if(smallerid == HTLLOCK_MAX_USER_NUM){
			log_err(false, "not found user space\n");
			return false;
		}else{
			i = smallerid;
		}
	}


	hl_info->user_manage[i].is_close_stat_reminder = ump->flagbit & 1;
	hl_info->user_manage[i].index = ump->index;
	hl_info->user_manage[i].pindex = ump->pindex;
	hl_info->user_manage[i].create_id = ump->create_id;
	hl_info->user_manage[i].pic_id = ump->pic_id;

	log_debug("add user[%d] is_close %uindex %u pindex %u create_id %u pic_id %u\n", 
		i, hl_info->user_manage[i].is_close_stat_reminder, hl_info->user_manage[i].index, hl_info->user_manage[i].pindex, 
		hl_info->user_manage[i].create_id, hl_info->user_manage[i].pic_id);

	memcpy(hl_info->user_manage[i].name, ump->name, sizeof(hl_info->user_manage[i].name));

	return true;
}

/**
	因为用户信息是分片传输，这里是添加一片在缓存。筹齐以后再更新用户
*/
#if 0
static bool rf_htllock_user_add_one_slice(cl_htllock_info_t *hl_info, htllock_tt_user_manage_push_t *ump)
{
	int i;
	bool found = false, last_slice = false;

	ump->slice = ntohs(ump->slice);
	ump->index = ntohs(ump->index);
	ump->pindex = ntohs(ump->pindex);
	ump->create_id = ntohs(ump->create_id);
	last_slice = !!(ump->slice & BIT(15));
	ump->slice = ump->slice & 0x7f;

	log_debug("user slice %u last_slice %u\n", ump->slice, last_slice);
	log_debug("user: index %u pindex %u create_id %u\n", ump->index, ump->pindex, ump->create_id);

	if (ump->slice == 0) {
		hl_info->user_slice_count = 0;
	} else {
		if (ump->slice >= HTLLOCK_MAX_USER_NUM) {
			log_err(false, "invalid slice %u\n", ump->slice);
			return false;
		}
		
		if (hl_info->user_slice_count != ump->slice) {
			log_err(false, "now user slice count %u, but pkt slice %u\n", hl_info->user_slice_count, ump->slice);
			return false;
		}
	}

	memcpy(&hl_info->user_slice[hl_info->user_slice_count++], ump, sizeof(*ump));

	// 替换用户列表
	if (last_slice == true) {
		memset(hl_info->user_manage, 0x00, sizeof(hl_info->user_manage));
		
		for (i = 0; i < hl_info->user_slice_count; i++) {
			rf_htllock_user_add(hl_info, &hl_info->user_slice[i]);
		}
	}

	return true;
}

#endif
static bool rf_htllock_user_add_one_slice(slave_t *slave, cl_htllock_info_t *hl_info, htllock_tt_user_manage_push_t *ump)
{
	int i;
	bool found = false, last_slice = false;

	ump->index_user = ntohs(ump->index_user);
	ump->get_useridex = ntohs(ump->get_useridex);
	ump->index = ntohs(ump->index);
	ump->pindex = ntohs(ump->pindex);
	ump->create_id = ntohs(ump->create_id);

	log_debug("htllock get user slice [%u], index_user %u\n", ump->get_useridex, ump->index_user);

	// 和最新cache里面的index不一样，需要重新查询
	if (hl_info->index_user != ump->index_user) {
		log_err(false, "index user change %u => %u, need query user again\n", hl_info->index_user, ump->index_user);
		rf_htllock_query_user_manage(slave, 1);
		return false;
	}

	// 第一片数据
	if (ump->get_useridex == 1) {
		hl_info->user_write_index = 0;
		log_debug("htllock first user...\n");
		memset((u_int8_t*)hl_info->user_slice, 0x00, sizeof(hl_info->user_slice));
	} 
	
	log_debug("htllock user write index %u\n", hl_info->user_write_index);

	if (ump->get_useridex != hl_info->user_write_index + 1) {
		log_debug("ump->get_useridex %u != hl_info->user_write_index %u + 1\n", 
			ump->get_useridex, hl_info->user_write_index + 1);
		rf_htllock_query_user_manage(slave, hl_info->user_write_index + 1);
		return false;
	}

	memcpy((u_int8_t*)&hl_info->user_slice[hl_info->user_write_index++], (u_int8_t*)ump, sizeof(*ump));

	// 还没接收完成
	if (hl_info->user_count > ump->get_useridex) {
		log_debug("htllock user not finish.., query user index %u\n", hl_info->user_write_index + 1);
		rf_htllock_query_user_manage(slave, hl_info->user_write_index + 1);
		return false;
		
	}

	log_debug("htllock: good, get all user info ,count %u\n", hl_info->user_count);
	
	memset(hl_info->user_manage, 0x00, sizeof(hl_info->user_manage));
	
	for (i = 0; i < hl_info->user_count; i++) {
		rf_htllock_user_add(hl_info, &hl_info->user_slice[i]);
	}

	// 写文件
	_rf_htllock_write_user_info(slave);

	return true;
}
//操作命令
#define HTL_LOCK_OP_ADD 		(1)
#define HTL_LOCK_OP_DEL 		(2)//删除一个
#define HTL_LOCK_OP_DEL_TYPE 	(3)//删除一类
#define HTL_LOCK_OP_DEL_ALL		(4)//都删除

#define HTL_LOCK_USER_ADMIN		(0x2000)

#define HTL_TYPE_GET(id) ((id >> 12)&0x0f)
static void do_htllock_user_batch_query_from_index(slave_t *slave, u_int16_t index)
{
	u_int8_t buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	httlock_bid_user_query_v2_t *pq = (httlock_bid_user_query_v2_t *)&ptlv[1];

	log_info("sn=%"PRIu64" query index=%u\n", slave->sn, index);
	memset(buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_QUERY);
	ptlv->len = htons(sizeof(*pq));
	pq->index_type = HTLLOCK_USER_QUERY_BATCH;
	pq->sn = ntoh_ll(slave->sn);
	pq->get_index = htons(index);
	
	do_comm_gw_big_data_send(slave, UCA_GET, buf, (int)(sizeof(*ptlv) + sizeof(*pq)));
}

static void do_htllock_summary_proc(slave_t *slave, httlock_bid_cache_hd_v2_t *phd, u_int16_t len)
{
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	
	if (hl_info->index_user != phd->index_userconf) {
		hl_info->index_user = phd->index_userconf;

		hl_info->user_count = phd->all_usernum;
		log_info("phd->all_usernum=%u phd->index_userconf=%u\n", phd->all_usernum, phd->index_userconf);
		//query all
		do_htllock_user_batch_query_from_index(slave, 1);
	}
}

static void do_htllock_one_proc(slave_t *slave, httlock_bid_cache_hd_v2_t *phd, u_int16_t len)
{
	int n;
	htllock_user_manage_stat_t *pms = NULL;
	httlock_bid_cache_user_item_v2_t *pit = phd->item;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	n = (len - sizeof(*phd))/sizeof(*pit);

	if (n == 0) {
		log_info("n==0!!!!!!!!!!!!!!!\n");
		return;
	}

	pit->id_index = ntohs(pit->id_index);
	pit->pid_index = ntohs(pit->pid_index);
	pit->create_id = ntohs(pit->create_id);

	pms = rf_htllock_user_get_by_index(hl_info, pit->id_index);
	if (!pms) {
		log_info("error id_index=%u\n", pit->id_index);
		return;
	}
	
	pms->is_close_stat_reminder = pit->flagbit & 1;
	pms->index = pit->id_index;
	pms->pindex = pit->pid_index;
	pms->create_id = pit->create_id;
	pms->pic_id = pit->pic_id;
	memcpy(pms->name ,pit->name, sizeof(pms->name));
	// 写文件
	_rf_htllock_write_user_info(slave);
	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void do_htllock_batch_proc(slave_t *slave, httlock_bid_cache_hd_v2_t *phd, u_int16_t len)
{
	int i,n;
	httlock_bid_cache_user_item_v2_t *pit = phd->item;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	n = (len - sizeof(*phd))/sizeof(*pit);

	if (n == 0) {
		log_info("n==0!!!!!!!!!!!!!!!\n");
		return;
	}

	//这里先判断一下，如果个数不同也要查询一下
	if (hl_info->user_count != phd->all_usernum) {
		hl_info->user_count = phd->all_usernum;
		log_info("phd->all_usernum=%u phd->index_userconf=%u\n", phd->all_usernum, phd->index_userconf);
		//query all
		do_htllock_user_batch_query_from_index(slave, 1);
		return;
	}

	//第一个
	if (phd->tmp_index == 1) {
		hl_info->user_write_index = 0;
		log_debug("htllock first user...\n");
		memset((u_int8_t*)hl_info->user_slice, 0x00, sizeof(hl_info->user_slice));
	}

	log_debug("htllock user write index %u n=%u hl_info->user_count=%u\n", hl_info->user_write_index, n, hl_info->user_count);
	if ((hl_info->user_write_index + n) > HTLLOCK_MAX_USER_NUM) {
		log_info("error hl_info->user_write_index + n=%u\n", hl_info->user_write_index + n);
		return;
	}
	
	for(i = 0; i < n; i++) {
		hl_info->user_slice[hl_info->user_write_index+i].index = ntohs(pit[i].id_index);
		hl_info->user_slice[hl_info->user_write_index+i].pindex = ntohs(pit[i].pid_index);
		hl_info->user_slice[hl_info->user_write_index+i].create_id = ntohs(pit[i].create_id);
		hl_info->user_slice[hl_info->user_write_index+i].pic_id = pit[i].pic_id;
		hl_info->user_slice[hl_info->user_write_index+i].flagbit = pit[i].flagbit;
		memcpy(hl_info->user_slice[hl_info->user_write_index+i].name, pit[i].name, sizeof(pit[i].name)); 
	}

	hl_info->user_write_index += i;
	if (hl_info->user_write_index < hl_info->user_count) {
		do_htllock_user_batch_query_from_index(slave,hl_info->user_write_index + 1);
		return;
	}

	memset(hl_info->user_manage, 0x00, sizeof(hl_info->user_manage));
	for(i = 0; i < hl_info->user_count; i++) {
		hl_info->user_manage[i].is_close_stat_reminder = hl_info->user_slice[i].flagbit & 1;
		hl_info->user_manage[i].index = hl_info->user_slice[i].index;
		hl_info->user_manage[i].pindex = hl_info->user_slice[i].pindex;
		hl_info->user_manage[i].create_id = hl_info->user_slice[i].create_id;
		hl_info->user_manage[i].pic_id = hl_info->user_slice[i].pic_id;
		memcpy(hl_info->user_manage[i].name ,hl_info->user_slice[i].name, sizeof(hl_info->user_manage[i].name));
		log_info("i=%d createid=%u pindex=%u index=%u name=%s type=%u\n", 
			i, hl_info->user_manage[i].create_id, hl_info->user_manage[i].pindex, 
			hl_info->user_manage[i].index, hl_info->user_manage[i].name, HTL_TYPE_GET(hl_info->user_manage[i].create_id));
	}

	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void do_htllock_bd_query_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	httlock_bid_cache_hd_v2_t *phd = (httlock_bid_cache_hd_v2_t *)tlv_val(ptlv);

	if (ptlv->len < sizeof(*phd)) {
		log_debug("%s %d error len=%u\n", __FUNCTION__, __LINE__);
		return;
	}
	
	phd->index_userconf = htons(phd->index_userconf);
	phd->all_usernum = htons(phd->all_usernum);
	phd->tmp_index = htons(phd->tmp_index);
	
	switch(phd->index_type) {
	case HTLLOCK_USER_QUERY_BATCH:
		do_htllock_batch_proc(slave, phd, ptlv->len);
		break;
	case HTLLOCK_USER_QUERY_ONE:
		do_htllock_one_proc(slave, phd, ptlv->len);
		break;
	case HTLLOCK_USER_QUERY_SUMMARY:
		do_htllock_summary_proc(slave, phd, ptlv->len);
		break;
	default:
		break;
	}
}

static void do_htllock_bd_name_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	htllock_user_manage_stat_t *pit= NULL;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	htllock_tt_user_manage_set_name_v2_t *pname = (htllock_tt_user_manage_set_name_v2_t *)tlv_val(ptlv);

	if (ptlv->len < (sizeof(*pname))) {
		log_info("error tlv len=%u\n", ptlv->len);
		return;
	}
	pname->index = ntohs(pname->index);

	pit = rf_htllock_user_get_by_index(hl_info, pname->index);
	if (!pit) {
		log_info("err index=%u\n", pname->index);
		return;
	}

	memcpy(pit->name, pname->name, sizeof(pit->name));
	do_rfgw_index_hllock_add(slave);
	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void do_htllock_bd_pid_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	htllock_user_manage_stat_t *pit= NULL;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	httlock_bid_user_picid_set_v2_t *pids = (httlock_bid_user_picid_set_v2_t *)tlv_val(ptlv);

	if (ptlv->len < (sizeof(*pids))) {
		log_info("error tlv len=%u\n", ptlv->len);
		return;
	}
	pids->id_index = ntohs(pids->id_index);

	pit = rf_htllock_user_get_by_index(hl_info, pids->id_index);
	if (!pit) {
		log_info("err index=%u\n", pids->id_index);
		return;
	}

	pit->pic_id = pids->pic_id;
	do_rfgw_index_hllock_add(slave);
	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void do_htllock_bd_link_add_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	htllock_user_manage_stat_t *pit= NULL;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	httlock_bid_user_bind_v2_t *pbind = (httlock_bid_user_bind_v2_t *)tlv_val(ptlv);

	if (ptlv->len < (sizeof(*pbind))) {
		log_info("error tlv len=%u\n", ptlv->len);
		return;
	}
	pbind->id_index = ntohs(pbind->id_index);
	pbind->cindex = ntohs(pbind->cindex);

	pit = rf_htllock_user_get_by_index(hl_info, pbind->cindex);
	if (!pit) {
		log_info("err pbind->cindex=%u\n", pbind->cindex);
		return;
	}

	pit->pindex = pbind->id_index;
	do_rfgw_index_hllock_add(slave);
	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void do_htllock_bd_link_del_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	htllock_user_manage_stat_t *pit= NULL;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	httlock_bid_user_bind_v2_t *pbind = (httlock_bid_user_bind_v2_t *)tlv_val(ptlv);

	if (ptlv->len < (sizeof(*pbind))) {
		log_info("error tlv len=%u\n", ptlv->len);
		return;
	}
	pbind->id_index = ntohs(pbind->id_index);
	pbind->cindex = ntohs(pbind->cindex);

	pit = rf_htllock_user_get_by_index(hl_info, pbind->cindex);
	if (!pit) {
		log_info("err pbind->cindex=%u\n", pbind->cindex);
		return;
	}

	pit->pindex = pbind->cindex;
	do_rfgw_index_hllock_add(slave);
	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void rf_htllock_user_sync(slave_t *slave, u_int16_t pindex)
{
	int i;
	
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	if (pindex == 0) {
		return;
	}

	for(i = 0; i < hl_info->user_count; i++) {
		if (hl_info->user_manage[i].pindex == pindex) {
			hl_info->user_manage[i].pindex = hl_info->user_manage[i].index;
		}
	}
}

static void do_htloock_bd_adddel_push_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	int i, j, n;
	bool need_copy = false;
	htllock_user_manage_stat_t *pit= NULL;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	httlock_bid_user_adddel_v2_t *padp = (httlock_bid_user_adddel_v2_t *)tlv_val(ptlv);
	htllock_user_manage_stat_t userlocl[HTLLOCK_MAX_USER_NUM]; // 用户管理信息	

	if (ptlv->len < (sizeof(*padp))) {
		log_info("error tlv len=%u\n", ptlv->len);
		return;
	}

	padp->create_id = ntohs(padp->create_id);
	padp->id_index = ntohs(padp->id_index);

	log_info("padp->create_id=%u padp->id_index=%u op=%u\n", padp->create_id, padp->id_index, padp->op);
	switch(padp->op) {
	case HTL_LOCK_OP_ADD:
		pit = rf_htllock_user_get_by_index(hl_info, padp->id_index);
		if (pit) {
			log_info("error index=%u is exsit already\n", padp->id_index);
			return;
		}
		if (hl_info->user_count >= HTLLOCK_MAX_USER_NUM) {
			log_info("the user num is max hl_info->user_count=%u HTLLOCK_MAX_USER_NUM=%u\n", 
				hl_info->user_count, HTLLOCK_MAX_USER_NUM);
			return;
		}
		hl_info->user_manage[hl_info->user_count].index = padp->id_index;
		hl_info->user_manage[hl_info->user_count].pindex = padp->id_index;
		hl_info->user_manage[hl_info->user_count].pic_id = 0;
		hl_info->user_manage[hl_info->user_count].create_id = padp->create_id;
		memset(hl_info->user_manage[hl_info->user_count].name, 0, sizeof(hl_info->user_manage[hl_info->user_count].name));
		hl_info->user_count++;
		break;
	case HTL_LOCK_OP_DEL:
		pit = rf_htllock_user_get_by_createid(hl_info, padp->create_id);
		if (!pit) {
			log_info("error create_id=%u is not exsit\n", padp->create_id);
			return;
		}
		memset((void *)userlocl, 0, sizeof(userlocl));
		n = hl_info->user_count;
		for(i = 0, j = 0; i < n; i++) {
			if (hl_info->user_manage[i].create_id == padp->create_id) {
				hl_info->user_count--;
				continue;
			}
			need_copy = true;
			memcpy((void *)&userlocl[j++], (void *)&hl_info->user_manage[i], sizeof(htllock_user_manage_stat_t));
		}
		if (need_copy) {
			memcpy((void *)hl_info->user_manage, (void *)userlocl, sizeof(userlocl));
			//同步一下,如果删除的是父节点，还要处理下子节点
			rf_htllock_user_sync(slave, padp->id_index);
		}
		break;
	case HTL_LOCK_OP_DEL_TYPE:
		pit = rf_htllock_user_get_by_createid(hl_info, padp->create_id);
		if (!pit) {
			log_info("error create_id=%u is not exsit\n", padp->create_id);
			return;
		}
		log_info("del createid=%u type=%u\n", padp->create_id, HTL_TYPE_GET(padp->create_id));
		memset((void *)userlocl, 0, sizeof(userlocl));
		//打标记，同步
		for(i = 0; i < hl_info->user_count; i++) {
			//管理员不删除
			if (HTL_LOCK_USER_ADMIN == hl_info->user_manage[i].create_id) {
				log_info("i=%d createid=%u is admin no del\n", i, hl_info->user_manage[i].create_id);
				continue;
			}
			log_info("i=%d id1=%u id2=%u type1=%u type2=%u\n", 
				i, hl_info->user_manage[i].create_id, padp->create_id, 
				HTL_TYPE_GET(hl_info->user_manage[i].create_id), HTL_TYPE_GET(padp->create_id));
			if (HTL_TYPE_GET(hl_info->user_manage[i].create_id) == HTL_TYPE_GET(padp->create_id)) {
				rf_htllock_user_sync(slave, hl_info->user_manage[i].index);
				hl_info->user_manage[i].create_id = 0;
				need_copy = true;
				continue;
			}
		}
		//删除
		for(i = 0, j = 0, n = 0; i < hl_info->user_count; i++) {
			if (hl_info->user_manage[i].create_id == 0) {
				n++;
				continue;
			}
			memcpy((void *)&userlocl[j++], (void *)&hl_info->user_manage[i], sizeof(htllock_user_manage_stat_t));
		}
		if (need_copy) {
			hl_info->user_count -= n;
			memcpy((void *)hl_info->user_manage, (void *)userlocl, sizeof(userlocl));
			//同步一下,如果删除的是父节点，还要处理下子节点
			rf_htllock_user_sync(slave, padp->id_index);
			for(i = 0; i < hl_info->user_count; i++) {
				log_info("save pindex=%u index=%u creatdid=%u type=%u\n", 
					hl_info->user_manage[i].index, hl_info->user_manage[i].pindex,
					hl_info->user_manage[i].create_id, HTL_TYPE_GET(hl_info->user_manage[i].create_id));
			}
		}
		break;
	case HTL_LOCK_OP_DEL_ALL:
		memset((void *)userlocl, 0, sizeof(userlocl));
		n = hl_info->user_count;
		for(i = 0, j = 0; i < n; i++) {
			//管理员不删除
			if (HTL_LOCK_USER_ADMIN != hl_info->user_manage[i].create_id) {
				hl_info->user_count--;
				continue;
			}
			hl_info->user_manage[i].pindex = hl_info->user_manage[i].index;
			memcpy((void *)&userlocl[j++], (void *)&hl_info->user_manage[i], sizeof(htllock_user_manage_stat_t));
		}
		memcpy((void *)hl_info->user_manage, (void *)userlocl, sizeof(userlocl));
		break;
	default:
		log_info("error op=%u\n", padp->op);
		return;
	}

	do_rfgw_index_hllock_add(slave);
	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

static void do_htloock_bd_notice_proc(slave_t *slave, uc_tlv_t *ptlv)
{
	htllock_user_manage_stat_t *pit= NULL;
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	httlock_bid_notice_v2_t *pnotice = (httlock_bid_notice_v2_t *)tlv_val(ptlv);

	if (ptlv->len < (sizeof(*pnotice))) {
		log_info("error tlv len=%u\n", ptlv->len);
		return;
	}
	pnotice->id_index = ntohs(pnotice->id_index);

	pit = rf_htllock_user_get_by_index(hl_info, pnotice->id_index);
	if (!pit) {
		log_info("err pnotice->id_index=%u\n", pnotice->id_index);
		return;
	}

	pit->is_close_stat_reminder = pnotice->isclose;
	do_rfgw_index_hllock_add(slave);
	// 写文件
	_rf_htllock_write_user_info(slave);

	//push个modify
	event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
}

void do_htllock_tlv_proc(slave_t *slave, uc_tlv_t *ptlv, u_int8_t action)
{
	log_info("type=%u\n", ptlv->type);
	switch(ptlv->type) {
	case GW_HTL_BD_QUERY:
		do_htllock_bd_query_proc(slave, ptlv);
		break;
	case GW_HTL_BD_NAME_SET:
		do_htllock_bd_name_proc(slave, ptlv);
		break;
	case GW_HTL_BD_PID_SET:
		do_htllock_bd_pid_proc(slave, ptlv);
		break;
	case GW_HTL_BD_ADD_LINK:
		do_htllock_bd_link_add_proc(slave, ptlv);
		break;
	case GW_HTL_BD_DEL_LINK:
		do_htllock_bd_link_del_proc(slave, ptlv);
		break;
	case GW_HTL_BD_ADDDEL_PUSH:
		do_htloock_bd_adddel_push_proc(slave, ptlv);
		break;
	case GW_HTL_BD_NOTICE:
		do_htloock_bd_notice_proc(slave, ptlv);
		break;
	default:
		break;
	}
}

void do_rfgw_index_hllock_add(slave_t *slave)
{
	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	hl_info->index_user++;

	_rf_htllock_write_user_info(slave);
}

void do_rfgw_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;
	user_t *user = ctrl->sac->user;
	uc_tlv_t *ptlv = (uc_tlv_t *)(((u_int16_t *)&obj[1]) + 1);//过滤掉ret
	slave_t *slave = NULL;
	u_int64_t sn = 0;
	
    if (!is_except_attr(obj, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_BIGDATA_CMD)) {
		return;
    }

	ptlv->type = ntohs(ptlv->type);
	ptlv->len = ntohs(ptlv->len);
	sn = ntoh_ll(*(u_int64_t *)tlv_val(ptlv));
	log_debug("do_rfgw_ctrl_result type=%u error=%u sn=%"PRIu64"\n", ptlv->type, error, sn);
	switch(ptlv->type) {
	case GW_HTL_BD_NAME_SET:
		event = (error == 0)?SAE_HTLLOCK_SET_NAME_OK:SAE_HTLLOCK_SET_NAME_FAILED;
		break;
	case GW_HTL_BD_PID_SET:
		event = (error == 0)?SAE_HTLLOCK_SET_PIC_OK:SAE_HTLLOCK_SET_PIC_FAILED;
		break;
	case GW_HTL_BD_ADD_LINK:
		event = (error == 0)?SAE_HTLLOCK_SET_BIND_OK:SAE_HTLLOCK_SET_BIND_FAILED;
		break;
	case GW_HTL_BD_DEL_LINK:
		event = (error == 0)?SAE_HTLLOCK_SET_UNBIND_OK:SAE_HTLLOCK_SET_UNBIND_FAILED;
		break;
	case GW_HTL_BD_NOTICE:
		event = (error == 0)?SAE_HTLLOCK_SET_REMINDER_ONOFF_OK:SAE_HTLLOCK_SET_REMINDER_ONOFF_FAILED;
		break;
	default:
		return;
	}

	if (error == 0) {
		//操作成功后，内部计算递增一下，避免重复查询
		slave = slave_lookup_by_sn(ctrl->sac->user, sn);
		if (slave) {
			do_rfgw_index_add(slave);
		}
	}

	if (event) {
        event_push(user->callback, event, user->handle, user->callback_handle);
	}
}

static bool rf_htllock_info_notice_add(cl_htllock_info_t *hl_info, u_int8_t sbit)
{
	int idx = 0;
	
	u_int8_t type;	// HTLLOCK_INFO_NOTICE_TYPE_XXX 报警类型
	u_int8_t support_remind;	// 支持通知提醒
	u_int8_t support_trouble_free;	// 支持在家免打扰
	u_int8_t support_msg_remind;	// 支持短信提醒
	u_int8_t support_tel_remind;	//  支持电话提醒

	type = (sbit) & 0xf;
	support_remind = !!(sbit & BIT(4));
	support_trouble_free = !!(sbit & BIT(5));
	support_msg_remind = !!(sbit & BIT(6));
	support_tel_remind = !!(sbit & BIT(7));

	log_debug(" notice add: type %u support_remind %u\n", type, support_remind);

	if (type == 0 || type > HTLLOCK_INFO_NOTICE_TYPE_MANDLE) {
		return false;
	}

	idx = type - 1;

	hl_info->info_notice[idx].type = type;
	hl_info->info_notice[idx].support_remind = support_remind;
	hl_info->info_notice[idx].support_trouble_free = support_trouble_free;
	hl_info->info_notice[idx].support_msg_remind = support_msg_remind;
	hl_info->info_notice[idx].support_tel_remind = support_tel_remind;	
	
	return true;
}

static bool rf_htllock_parse_raw_stat(slave_t* slave, u_int8_t *raw, int len)
{
	htllock_uart_hdr_t *hd = (htllock_uart_hdr_t *)raw;
	htllock_uart_cmd_stat_reply_t *stat = (htllock_uart_cmd_stat_reply_t *)&hd[1];
   	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	log_debug("htllock get stat uart len %d\n", len);

	if (len < sizeof(*hd) + sizeof(*raw) + 2) {
		return false;
	}

	hl_info->lock_stat.vol = stat->vol;
	hl_info->lock_stat.lang = stat->lang;
	hl_info->lock_stat.battery = stat->battery;
	if (hl_info->lock_stat.battery < 5) {
		hl_info->lock_stat.battery = 5;
	}

	log_debug("update vol %u lang %u battery %u\n", stat->vol, stat->lang, stat->battery);

	return true;
}

static bool rf_htllock_parse_pin(slave_t* slave, u_int8_t *raw, int len)
{
	htllock_uart_hdr_t *hd = (htllock_uart_hdr_t *)raw;
	htllock_tt_set_pin_t *stat = (htllock_tt_set_pin_t *)&hd[1];
   	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	log_debug("htllock get pin reply len %d\n", len);

	if (len < sizeof(*hd) + sizeof(*stat) + 2) {
		return false;
	}

	memcpy(&hl_info->last_pin, stat, sizeof(hl_info->last_pin));

	event_push(slave->user->callback, SAE_HTLLOCK_PIN_REPLY, slave->handle, slave->user->callback_handle);

	return true;
}


/**
	处理透传数据，目前暂时只处理认证的
*/
static bool rf_htllock_parse_raw_data(slave_t* slave, u_int8_t *raw, int len)
{
	htllock_uart_hdr_t *hd = (htllock_uart_hdr_t *)raw;
	u_int8_t *err;
	int event = 0;

	log_debug("htllock get raw mcmd 0x%x scmd 0x%x\n", hd->mcmd, hd->scmd);

	//event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);

	if ((hd->mcmd == 0x03 && hd->scmd == 0x01) || (hd->mcmd == 0x03 && hd->scmd == 0x06)) {
		return rf_htllock_parse_raw_stat(slave, raw, len);
	}

	// 设置临时PIN
	if (hd->mcmd == 0x02 && hd->scmd == 0x03) {
		if (hd->len < 10) {
			return false;
		}

		err = (u_int8_t *)&hd[1] + 2;
		if (*err == 0) {
			log_debug("SAE_HTLLOCK_SET_PIN_OK\n");
			event = SAE_HTLLOCK_SET_PIN_OK;
		} else {
			
			log_debug("SAE_HTLLOCK_SET_PIN_FAILED\n");
			event = SAE_HTLLOCK_SET_PIN_FAILED;
		}
		
		goto done;
	}

	if (hd->mcmd == 0x2 && hd->scmd == 0x5) {
		return rf_htllock_parse_pin(slave, raw, len);
	}

	if (hd->mcmd == 0x02 && hd->scmd == 0x06) {
		event = SAE_HTLLOCK_UNLOCK_OK;
		log_debug("SAE_HTLLOCK_UNLOCK_OK\n");
		goto done;
	}

	if (hd->mcmd != 0xf0 || hd->scmd != 0x00) {
		return false;
	}

	err = (u_int8_t *)&hd[1];

	if (*err == 0) {
		event = SAE_HTLLOCK_ADMIN_LOGIN_OK;
	} else {
		event = SAE_HTLLOCK_ADMIN_LOGIN_FAILED;
	}

done:
	if (event) {
        event_push(slave->user->callback, event, slave->handle, slave->user->callback_handle);
	}

	return false;
}

static bool rf_htllock_update_history(cl_htllock_info_t *hf, htllock_tt_info_notice_get_history_reply_t *reply)
{
	int i;
	u_int32_t temp_time = 0xffffffff;
	httlock_history_t *hh, newh;

	reply->id = ntohs(reply->id);
	reply->timestamp = ntohl(reply->timestamp);
	

	newh.is_valid = reply->info & 1;
	newh.value = (reply->info >> 1) & 1;
	newh.create_id = reply->id;
	newh.info_type = 0;
	newh.time_stamp = reply->timestamp;

	newh.info_type = (reply->info >> 2) & 0x3f;
	#if 0
	for (i = 2; i < 8; i++) {
		if (reply->info & BIT(i)) {
			newh.info_type = i - 1;
			break;
		}
	}
	#endif

	if (newh.info_type == 0) {
		log_err(false, "can not found info type\n");
		return false;
	}

	// 先检测重复
	for (i = 0; i < HTLLOCK_MAX_NOTICE_TYPE_NUM; i++) {
		if (memcmp(&hf->history[i], &newh, sizeof(httlock_history_t)) == 0) {
			return false;
		}
	}

	for (i = 0; i < HTLLOCK_MAX_NOTICE_TYPE_NUM; i++) {
		if (hf->history[i].is_valid == false) {
			hh = &hf->history[i];
			break;
		}

		// 找个最久远的时间替换掉
		if (hf->history[i].time_stamp < temp_time) {
			hh = &hf->history[i];
			temp_time = hf->history[i].time_stamp;
		}
	}

	if (hh) {
		log_debug("htllock update history: type %u time %u create_id %u\n", newh.info_type, newh.time_stamp, newh.create_id);
		memcpy(hh, &newh, sizeof(newh));
		return true;
	}

	return false;
}

static void htllock_alarm_update_idx(slave_t *slave, u_int8_t index_alarm)
{
   	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);

	log_debug("htllock alarm update alarm idex to %u\n", index_alarm);

	hl_info->cache.index_alarm = index_alarm;
}

static bool _rf_htllock_update_data(slave_t* slave, rf_tlv_t* tlv, u_int8_t action)
{
	u_int8_t buf[256] = {0}, i;
   	cl_htllock_info_t *hl_info = &(slave->dev_info.rf_stat.dev_priv_data.hl_info);
	bool need_alarm = false;
	int event = 0;
	

	u_int8_t *op = (u_int8_t *)rf_tlv_val(tlv);
	htllock_tt_set_reply_t *set_reply = (htllock_tt_set_reply_t *)rf_tlv_val(tlv);
	htllock_tt_user_manage_push_t *ump = (htllock_tt_user_manage_push_t *)rf_tlv_val(tlv);
	htllock_tt_info_notice_push_t *inp = (htllock_tt_info_notice_push_t *)rf_tlv_val(tlv);
	htllock_tt_info_notice_set_reply_t *ins = (htllock_tt_info_notice_set_reply_t *)rf_tlv_val(tlv);
	htllock_tt_info_notice_get_history_reply_t *hr = (htllock_tt_info_notice_get_history_reply_t *)rf_tlv_val(tlv);
	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);

	// 更新数据来自PUSH
	if (action != UCA_PUSH) {
		log_err(false, "htllock updata data must be push\n");
		return false;
	}

	log_debug("_rf_htllock_update_data type 0x%x\n", tlv->type);

    switch (tlv->type) {
		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, htllock_alarm_update_idx);
			
		case HTLLOCK_RAW_DATA:
			return rf_htllock_parse_raw_data(slave, rf_tlv_val(tlv), tlv->len);

		case RF_TT_CMD_QUERY_HISLOG:
			return rf_update_comm_history_item(slave, rf_tlv_val(tlv), tlv->len);
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
			
        case HTLLOCK_USER_MANAGE:
			// 查询用户属性的回报文
			if (*op == HTLLOCK_USER_MANAGE_OPT_TYPE_QUERY) {

				if (tlv->len < sizeof(*ump)) {
					return false;
				}
#if 0
				if (hl_info->cache.index_user != ump->index_user) {
					
					log_debug("cache index user %u but reply idx %u\n", hl_info->cache.index_user, ump->index_user);
					rf_htllock_query_user_manage(slave);
					return false;
				}

#endif
							
				return rf_htllock_user_add_one_slice(slave, hl_info, ump);
				
			} else if (*op == HTLLOCK_USER_MANAGE_OPT_TYPE_SET_NAME) {
				if (set_reply->err == 0) {
					event = SAE_HTLLOCK_SET_NAME_OK;
				} else {
					event = SAE_HTLLOCK_SET_NAME_FAILED;
				}
			} else if (*op == HTLLOCK_USER_MANAGE_OPT_TYPE_SET_PIC) {
				if (set_reply->err == 0) {
					event = SAE_HTLLOCK_SET_PIC_OK;
				} else {
					event = SAE_HTLLOCK_SET_PIC_FAILED;
				}
			} else if (*op == HTLLOCK_USER_MANAGE_OPT_TYPE_SET_BIND) {
				if (set_reply->err == 0) {
					event = SAE_HTLLOCK_SET_BIND_OK;
				} else {
					event = SAE_HTLLOCK_SET_BIND_FAILED;
				}
			} else if (*op == HTLLOCK_USER_MANAGE_OPT_TYPE_SET_UNBIND) {
				if (set_reply->err == 0) {
					event = SAE_HTLLOCK_SET_UNBIND_OK;
				} else {
					event = SAE_HTLLOCK_SET_UNBIND_FAILED;
				}
			} else if (*op == HTLLOCK_USER_MANAGE_OPT_TYPE_SET_REMIND_ONOFF) {
				if (set_reply->err == 0) {
					event = SAE_HTLLOCK_SET_REMINDER_ONOFF_OK;
				} else {
					event = SAE_HTLLOCK_SET_REMINDER_ONOFF_FAILED;
				}
			}
			
			break;

		case HTLLOCK_INFO_NOTICE:
			// 查询信息提醒的回复报文
			if (*op == HTLLOCK_INFO_NOTICE_OPT_TYPE_QUERY) {
				if (tlv->len < sizeof(*inp)) {
					return false;
				}

				if (hl_info->cache.index_notice != inp->index_notice) {
					log_debug("index_notice change %u => %u\n", hl_info->cache.index_notice, inp->index_notice);
					rf_htllock_query_notice_info(slave);
					return false;
				}

				for (i = 0; i < sizeof(inp->sbit); i++) {
					rf_htllock_info_notice_add(hl_info, inp->sbit[i]);
				}
				
				return true;
			} else if (*op == HTLLOCK_INFO_NOTICE_OPT_TYPE_SET) {
				if (tlv->len < sizeof(*ins)) {
					return false;
				}

				if (ins->err == 0) {
					event = SAE_HTLLOCK_SET_INFO_NOTICE_OK;
				} else {
					event = SAE_HTLLOCK_SET_INFO_NOTICE_FAILED;
				}

				rf_htllock_info_notice_add(hl_info, ins->sbit_tmp);
			} else if (*op == HTLLOCK_INFO_NOTICE_OPT_TYPE_GET_HISTORY) {
				if (tlv->len < sizeof(*hr)) {
					return false;
				}
				
				rf_htllock_update_history(hl_info, hr);
			}
			break;
			
        default:
            return false;
    }

	if (event) {
        event_push(slave->user->callback, event, slave->handle, slave->user->callback_handle);
	}

    return true;
}

static u_int8_t htllock_calc_checksum(htllock_uart_hdr_t *hd)
{
	u_int8_t i;
	u_int8_t checksum = 0;
	u_int8_t *p = (u_int8_t *)(hd);

	for (i = 1; i < hd->len - sizeof(htllock_uart_tail_t); i++) {
		checksum ^= p[i];
	}

	return checksum;
}

static int htllock_build_tt_uart_send(slave_t* slave, u_int8_t *out, int out_len, u_int8_t mcmd, u_int8_t scmd, u_int8_t *param, u_int8_t param_len)
{
	rf_tlv_t* tlv = (rf_tlv_t*)out;
	htllock_uart_hdr_t *hd;
	htllock_uart_tail_t *tail;

	tlv->type = RF_TT_CMD_RAWDATA;
	tlv->len = sizeof(*hd) + sizeof(*tail) + param_len;
	
	hd = (htllock_uart_hdr_t *)rf_tlv_val(tlv);

	hd->start = 0xfc;
	hd->pid = 0;
	hd->len = sizeof(*hd) + sizeof(*tail) + param_len;

	hd->mcmd = mcmd;
	hd->scmd = scmd;

	if (param_len > 0) {
		memcpy(&hd[1], param, param_len);
	}

	tail = (htllock_uart_tail_t *)(((u_int8_t*)&hd[1]) + param_len);
	
	tail->end = 0xfe;
	tail->checksum = htllock_calc_checksum(hd);

	return sizeof(*tlv) + sizeof(*hd) + sizeof(*tail) + param_len;;
}

static void do_htllock_user_name_set(slave_t *slave, htllock_tt_user_manage_set_name_t *request)
{
	int len = 0;
	char buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	htllock_tt_user_manage_set_name_v2_t *pname= (htllock_tt_user_manage_set_name_v2_t *)&ptlv[1];
	
	memset((void *)buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_NAME_SET);
	ptlv->len = htons(sizeof(*pname));
	
	pname->sn = ntoh_ll(slave->sn);
	pname->index = htons(request->index);
	memcpy((void *)pname->name, request->name, sizeof(pname->name));

	len = sizeof(*ptlv) + sizeof(*pname);
	do_comm_gw_big_data_send(slave, UCA_SET, (u_int8_t *)buf, len);
}

static void do_htllock_user_pic_set(slave_t *slave, htllock_tt_user_manage_set_pic_t *request)
{
	int len = 0;
	char buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	httlock_bid_user_picid_set_v2_t *ppic= (httlock_bid_user_picid_set_v2_t *)&ptlv[1];
	
	memset((void *)buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_PID_SET);
	ptlv->len = htons(sizeof(*ppic));
	
	ppic->sn = ntoh_ll(slave->sn);
	ppic->id_index = htons(request->index);
	ppic->pic_id = request->pic_id;

	len = sizeof(*ptlv) + sizeof(*ppic);
	do_comm_gw_big_data_send(slave, UCA_SET, (u_int8_t *)buf, len);
}

static void do_htllock_user_bind(slave_t *slave, htllock_tt_user_manage_set_bind_t *request)
{
	int len = 0;
	char buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	httlock_bid_user_bind_v2_t *pbind= (httlock_bid_user_bind_v2_t *)&ptlv[1];
	
	memset((void *)buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_ADD_LINK);
	ptlv->len = htons(sizeof(*pbind));
	
	pbind->sn = ntoh_ll(slave->sn);
	pbind->id_index = htons(request->index);
	pbind->cindex = htons(request->cindex);

	len = sizeof(*ptlv) + sizeof(*pbind);
	do_comm_gw_big_data_send(slave, UCA_SET, (u_int8_t *)buf, len);
}

static void do_htllock_user_unbind(slave_t *slave, htllock_tt_user_manage_set_unbind_t *request)
{
	int len = 0;
	char buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	httlock_bid_user_bind_v2_t *pbind= (httlock_bid_user_bind_v2_t *)&ptlv[1];
	
	memset((void *)buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_DEL_LINK);
	ptlv->len = htons(sizeof(*pbind));
	
	pbind->sn = ntoh_ll(slave->sn);
	pbind->id_index = htons(request->index);
	pbind->cindex = htons(request->cindex);

	len = sizeof(*ptlv) + sizeof(*pbind);
	do_comm_gw_big_data_send(slave, UCA_SET, (u_int8_t *)buf, len);
}

static void do_htllock_user_notice(slave_t *slave, htllock_tt_user_manage_set_remind_onoff_t *request)
{
	int len = 0;
	char buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	httlock_bid_notice_v2_t *pnotice= (httlock_bid_notice_v2_t *)&ptlv[1];
	
	memset((void *)buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_NOTICE);
	ptlv->len = htons(sizeof(*pnotice));
	
	pnotice->sn = ntoh_ll(slave->sn);
	pnotice->id_index = htons(request->index);
	pnotice->isclose = request->isclose;

	len = sizeof(*ptlv) + sizeof(*pnotice);
	do_comm_gw_big_data_send(slave, UCA_SET, (u_int8_t *)buf, len);
}

static bool _rf_htllock_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
   	u_int8_t buf[256] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0, i = 0;
	cl_htllock_info_t *hl_info = &slave->dev_info.rf_stat.dev_priv_data.hl_info;
	htllock_user_manage_stat_t *user = NULL;

    switch (info->action) {
		case ACT_HTLLOCK_ADMIN_LOGIN:
			{
				htllock_tt_admin_login_t *request = cci_pointer_data(info);
				request->id = ntohs(request->id);

				len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0xf0, 0x00, (u_int8_t*)request, sizeof(*request));
			}
			break;

		case ACT_HTLLOCK_SET_VOL:
			{
				u_int8_t vol = (u_int8_t)cci_u32_data(info);
				len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0x01, 0x08, &vol, sizeof(vol));

				log_debug("htllock set vol %u before %u\n", vol, hl_info->lock_stat.vol);

				hl_info->lock_stat.vol = vol;

			}
			break;

		case ACT_HTLLOCK_SET_LANG:
			{
				u_int8_t lang = (u_int8_t)cci_u32_data(info);
				len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0x01, 0x09, &lang, sizeof(lang));

				hl_info->lock_stat.lang = lang;
			}
			break;

		case ACT_HTLLOCK_LOCAL_OPEN:
			{
				
				u_int8_t value = 0;
				len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0x02, 0x06, &value, sizeof(value));
			}
			break;

		case ACT_HTLLOCK_SET_PIN:
			{
				htllock_tt_set_pin_t *request =  cci_pointer_data(info);
				u_int8_t p[2];

				log_debug("set pin time %u\n", request->time);

				p[0] = (request->time & 0xff);
				p[1] = (request->time >> 8) & 0xff;
				
				memcpy(request, &p, sizeof(p));

				

				len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0x02, 0x03, (u_int8_t*)request, sizeof(*request));
			}
			break;

		case ACT_HTLLOCK_QUERY_PIN:
			{
				len = htllock_build_tt_uart_send(slave, buf, sizeof(buf), 0x02, 0x05, NULL, 0);
			}
			break;
			
        case ACT_HTLLOCK_USER_MANAGE_SET_NAME:
			{
				htllock_tt_user_manage_set_name_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_USER_MANAGE_OPT_TYPE_SET_NAME;

				// 因为从对方返回PUSH数据太久了，先本地更新
				if ((user = rf_htllock_user_get_by_index(hl_info, request->index)) != NULL) {
					memcpy(user->name, request->name, sizeof(user->name));
				}

				//这里判断一下，如果网关是支持大量用户版本，则发给网关
				if (slave->user && slave->user->is_support_batch_user) {
					do_htllock_user_name_set(slave, request);
					return true;
				}
				
				request->index = ntohs(request->index);

				tlv->type = HTLLOCK_USER_MANAGE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);				
			}            
            break;
			
		case ACT_HTLLOCK_USER_MANAGE_SET_PIC:
			{
				htllock_tt_user_manage_set_pic_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_USER_MANAGE_OPT_TYPE_SET_PIC;

				// 因为从对方返回PUSH数据太久了，先本地更新
				if ((user = rf_htllock_user_get_by_index(hl_info, request->index)) != NULL) {
					user->pic_id = request->pic_id;
				}

				//这里判断一下，如果网关是支持大量用户版本，则发给网关
				if (slave->user && slave->user->is_support_batch_user) {
					do_htllock_user_pic_set(slave, request);
					return true;
				}

				request->index = ntohs(request->index);

				tlv->type = HTLLOCK_USER_MANAGE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);
			}
			break;
			
		case ACT_HTLLOCK_USER_MANAGE_SET_BIND:
			{
				htllock_tt_user_manage_set_bind_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_USER_MANAGE_OPT_TYPE_SET_BIND;

				// 因为从对方返回PUSH数据太久了，先本地更新
				if ((user = rf_htllock_user_get_by_index(hl_info, request->cindex)) != NULL) {
					user->pindex = request->index;
				}

				//这里判断一下，如果网关是支持大量用户版本，则发给网关
				if (slave->user && slave->user->is_support_batch_user) {
					do_htllock_user_bind(slave, request);
					return true;
				}
				
				request->index = ntohs(request->index);
				request->cindex = ntohs(request->cindex);

				tlv->type = HTLLOCK_USER_MANAGE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);
			}
			break;
		case ACT_HTLLOCK_USER_MANAGE_SET_UNBIND:
			{
				htllock_tt_user_manage_set_unbind_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_USER_MANAGE_OPT_TYPE_SET_UNBIND;

				// 因为从对方返回PUSH数据太久了，先本地更新
				for (i = 0; i < HTLLOCK_MAX_USER_NUM; i++) {
					if (hl_info->user_manage[i].index == request->cindex) {
						hl_info->user_manage[i].pindex = request->cindex;
					}
				}
				//这里判断一下，如果网关是支持大量用户版本，则发给网关
				if (slave->user && slave->user->is_support_batch_user) {
					do_htllock_user_unbind(slave, request);
					return true;
				}
				request->index = ntohs(request->index);
				request->cindex = ntohs(request->cindex);

				tlv->type = HTLLOCK_USER_MANAGE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);
			}
			break;
		case ACT_HTLLOCK_USER_MANAGE_SET_REMIND_ONOFF:
			{
				htllock_tt_user_manage_set_remind_onoff_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_USER_MANAGE_OPT_TYPE_SET_REMIND_ONOFF;

				log_debug("htllock set remind onoff: is_close %u user index %u\n", request->isclose, request->index);

				// 因为从对方返回PUSH数据太久了，先本地更新
				if ((user = rf_htllock_user_get_by_index(hl_info, request->index)) != NULL) {
					log_debug("htllock updata local reminder to %u\n", request->isclose);
					user->is_close_stat_reminder = request->isclose;
				}

				//这里判断一下，如果网关是支持大量用户版本，则发给网关
				if (slave->user && slave->user->is_support_batch_user) {
					do_htllock_user_notice(slave, request);
					return true;
				}
				
				request->index = ntohs(request->index);

				tlv->type = HTLLOCK_USER_MANAGE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);

				
			}
			break;
		case ACT_HTLLOCK_SET_INTICE_INFO:
			{
				htllock_tt_info_notice_set_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_INFO_NOTICE_OPT_TYPE_SET;
			
				tlv->type = HTLLOCK_INFO_NOTICE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);
				rf_htllock_info_notice_add(hl_info, request->sbit_temp);
			}
			break;

		case ACT_HTLLOCK_GET_HISTORY:
			{
				htllock_tt_info_notice_get_history_t *request = cci_pointer_data(info);
				request->op = HTLLOCK_INFO_NOTICE_OPT_TYPE_GET_HISTORY;

#ifdef USE_TIME_MINS
				request->timestamp = request->timestamp - cl_priv->time_diff * 60;
#else
				request->timestamp = request->timestamp - cl_priv->timezone * 3600;
#endif
				request->timestamp = ntohl(request->timestamp);
			
				tlv->type = HTLLOCK_INFO_NOTICE;
				tlv->len = sizeof(*request);
				memcpy(rf_tlv_val(tlv), request, sizeof(*request));
			
				len = sizeof(*tlv) + sizeof(*request);
			}
			break;
			
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave, buf, len);
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////
//智皇电机
static bool _rf_zhdj_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
   	u_int8_t buf[1024] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0;
	cl_zhdj_info_t *zi = &slave->dev_info.rf_stat.dev_priv_data.zhdj_info;

    switch (info->action) {
	case ACT_ZHDJ_STATUS_SET:
		{
			u_int8_t *value = NULL;
			u_int8_t status = 0;
			
			tlv->type = UP_TLV_ZHDJ_STATUS_SET;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			status = cci_u8_data(info);
			*value = status;
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_ZHDJ_LOCATION_SET:
		{
			u_int8_t *value = NULL;
			u_int8_t pos = cci_u8_data(info);

			//check
			if (pos > 100) {
				*ret = RS_INVALID_PARAM;
				return true;
			}
			pos = 100 - pos;
			
			tlv->type = UP_TLV_ZHDJ_LOCATION_SET;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			*value = pos;
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_ZHDJ_LOCATION_QUERY:
		{
			tlv->type = UP_TLV_ZHDJ_LOCATION_GET;
			tlv->len = 0;
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_ZHDJ_BIND:
		{
			zhdj_bind_t *bind_n;
			zhdj_bind_t *bind_l;
			
			tlv->type = UP_TLV_ZHDJ_BIND;
			tlv->len = sizeof(*bind_n);
			bind_n = rf_tlv_val(tlv);
			bind_l = cci_pointer_data(info);
			memcpy((void *)bind_n, (void *)bind_l, sizeof(*bind_n));
			bind_n->magic = htonl(bind_n->magic);
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_ZHDJ_TYPE:
		{
			u_int8_t *value = NULL;
			u_int8_t *pdata = NULL;
			
			tlv->type = UP_TLV_ZHDJ_TYPE;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			pdata = cci_pointer_data(info);
			value[0] = pdata[0];
			value[1] = pdata[1];
			
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_ZHDJ_DIR:
		{
			u_int8_t *value = NULL;
			
			tlv->type = UP_TLV_ZHDJ_DIR;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			value[0] = cci_u8_data(info);;
			
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	default:
		break;
	}

    if (len > 0) {
		log_debug("%s action=%u\n", __FUNCTION__, info->action);
        rfgw_send_tt_packet(slave, buf, len);
    }

	return true;
}
////////////////////////////////////////////////////////////////
//// 情景遥控器
static void _rf_scene_controller_save(slave_t *slave)
{	
	cl_scene_controller_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.sc_info);
	dwkj_flash_key_t keys[4];
	int i;

	for (i = 0; i < 4; i++) {
		keys[i].id = i + 1;
		keys[i].idx = info->index_key[i];
		memcpy(keys[i].name, info->keys[i].name, sizeof(keys[i].name));
		keys[i].valid = info->keys[i].valid;
	}

	_rf_com_write_user_info(slave, DWKJ_CONF_ID_NAME, (u_int8_t*)keys, sizeof(keys));

	log_debug("_rf_scene_controller_save done\n");
}

static bool _rf_scene_controller_load(slave_t *slave)
{	
	cl_scene_controller_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.sc_info);
	dwkj_flash_key_t keys[4];
	int i;

	if (_rf_com_load_user_info(slave, DWKJ_CONF_ID_NAME, (u_int8_t*)keys, sizeof(keys)) != RS_OK) {
		log_err(false, "load sc config failed\n");
		return RS_ERROR;
	}

	for (i = 0; i < 4; i++) {
		keys[i].id = i + 1;

		info->index_key[i] = keys[i].idx;
		
		memcpy(info->keys[i].name, keys[i].name, sizeof(info->keys[i].name));
		info->keys[i].valid = keys[i].valid;

		log_debug("get key id %d, name[%s] index %u valid %u\n", i + 1, info->keys[i].name, info->index_key[i], keys[i].valid);
	}

	log_debug("load sc key done\n");

	return true;
}

static bool _rf_scene_controller_update_cache_data(slave_t* slave, u_int8_t action, u_int8_t *param, u_int16_t len)
{
	cl_scene_controller_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.sc_info);
	scene_controller_cache_t *cache = (scene_controller_cache_t *)param;
	int i;
	u_int8_t kid;

	if (len < sizeof(scene_controller_cache_t)) {
		log_debug("%s err len=%u\n", __FUNCTION__, len);
		return false;
	}

	cache->flag = htonl(cache->flag);
	cache->hislog_index_current = ntohl(cache->hislog_index_current);

	// 更新历史记录数据
	rf_update_comm_history_sammary(slave, cache->hislog_index_current, (u_int32_t)(cache->hislog_count));
	
	// 更新基本信息
	info->is_loss = !!(cache->flag & BIT(0));
	info->is_low_battery = !!(cache->flag & BIT(1));

	info->abc_battery = cache->abc_battery;
	if (info->abc_battery < 5) {
		info->abc_battery = 5;
	}
	
	if (info->index_alarm != cache->index_alarm) {
		log_debug("_rf_scene_controller_update_cache_data alarm query\n");
		info->index_alarm = cache->index_alarm;
		rf_dev_query_tlv_info(slave, RF_TT_CMD_PUSH_ALARM, NULL, 0);
	}

	if (info->stat_valid == false) {
		info->stat_valid = true;
		// 只查询一次软硬件版本
		rf_dev_query_tlv_info(slave, UP_TLV_RF_DEV_VERSION, NULL, 0);		

		// 先获取文件保存的按键信息
		_rf_scene_controller_load(slave);
	}

	for (i = 0; i < 4; i++) {
		if (info->index_key[i] == cache->index_key[i]) {
			continue;
		}

		kid = i + 1;

		log_debug("sc key kid %d index %u => %u, query it\n", kid, info->index_key[i], cache->index_key[i]);

		info->index_key[i] = cache->index_key[i];

		rf_dev_query_tlv_info(slave, RF_TT_SC_TYPE_QUERY_KEY, &kid, sizeof(kid));		
	}
	
	

	return true;
}

static void scene_controller_update_idx(slave_t *slave, u_int8_t index_alarm)
{
	cl_scene_controller_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.sc_info);

	log_debug("scene_controller_update_idx to %u\n", index_alarm);

	info->index_alarm = index_alarm;
}


static bool _rf_scene_controller_update_data(slave_t* slave,rf_tlv_t* tlv)
{
	cl_scene_controller_info_t* info = &(slave->dev_info.rf_stat.dev_priv_data.sc_info);

	log_debug("sn: %"PRIu64" _rf_scene_controller_update_data tlv type %u\n", slave->sn, tlv->type);

	
	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);


    switch (tlv->type) {
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
			
		case RF_TT_CMD_PUSH_ALARM:
			return rf_add_comm_alarm(slave, rf_tlv_val(tlv), tlv->len, scene_controller_update_idx);
			
		case UP_TLV_RF_DEV_VERSION:
			{
				ucp_scene_controller_version_t *version = rf_tlv_val(tlv);
				if (tlv->len < sizeof(*version)) {
					return false;
				}


				info->hw_ver = version->hw_ver;
				info->soft_ver_mar = (version->soft_ver >> 7) & 0x1;
				info->soft_ver_min = (version->soft_ver >> 4) & 0x7;
				info->soft_ver_rev = (version->soft_ver >> 7) & 0xF;
				info->svn = ntohl(version->svn);

				
				log_debug("scene controller hw_ver %u svn %u\n", info->hw_ver, info->svn);
			}
			break;

		case RF_TT_SC_TYPE_PUSH_KEY:
			{
				ucp_scene_controller_key_push_t *push = (ucp_scene_controller_key_push_t *)rf_tlv_val(tlv);
				int len;
				
				if (tlv->len < sizeof(*push)) {
					return false;
				}

				len = tlv->len - sizeof(*push);

				if (len > 31) {
					log_err(false, "sc key name len %d invalid\n", len);
					return false;
				}

				if (push->id == 0 || push->id > 4) {
					return false;
				}
				memset((void *)info->keys[push->id - 1].name, 0, sizeof(info->keys[push->id - 1].name));
				memcpy((u_int8_t*)&info->keys[push->id - 1].name, push->name, len);
				info->keys[push->id - 1].valid = true;
				
				log_debug("sc update key id %u name[%s]\n", push->id, info->keys[push->id - 1].name);

				_rf_scene_controller_save(slave);
			}
			break;
			
        default:
            return false;
    }

    return true;
}

static bool _rf_scene_controller_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
    char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0;
	cl_scene_controller_info_t* priv_info = &(slave->dev_info.rf_stat.dev_priv_data.sc_info);
	
    switch (info->action) {
		case ACT_SC_SET_LOSS:
			{
				u_int8_t value, cmd;

				value = (u_int8_t)cci_u32_data(info);

				if (value == 1) {
					cmd = 1;
					priv_info->is_loss = 1;
				} else {
					cmd = 2;
					priv_info->is_loss = 0;
				}

				tlv->type = RF_TT_SC_TYPE_SET_LOSS;
				tlv->len = sizeof(cmd);
				memcpy((u_int8_t*)&tlv[1], (u_int8_t*)&cmd, tlv->len);
				
				len = tlv->len + sizeof(*tlv);
			}
			break;
        case ACT_SC_SET_NAME:
			{
				ucp_scene_controller_key_set_t *request = (ucp_scene_controller_key_set_t *)cci_pointer_data(info);

				if (request->id == 0 || request->id > 4) {
					*ret = RS_INVALID_PARAM;
					return false;
				}

				len = (int)strlen(request->name) + 1;

				if (len > 24) {		
					*ret = RS_INVALID_PARAM;
					return false;
				}

				tlv->type = RF_TT_SC_TYPE_SET_KEY;
				tlv->len = (u_int8_t)info->data_len;

				memcpy((u_int8_t*)&tlv[1], (u_int8_t*)request, info->data_len);
				
				len = tlv->len + sizeof(*tlv);

				log_debug("set id %u name len %u name [%s], total %u\n", request->id, request->len, request->name, info->data_len);

				// 本地先修改
				memset((u_int8_t*)&priv_info->keys[request->id - 1], 0x00, sizeof(priv_info->keys[request->id - 1]));
				priv_info->keys[request->id - 1].valid = true;				
				memcpy(priv_info->keys[request->id - 1].name, request->name, sizeof(priv_info->keys[request->id - 1].name));
				
			}
			break;
			
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////
// 橙灯情景面板
static bool _rf_cdqjmb_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
	char buf[128] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0;
	cdqjmb_set_name_t *pname = NULL;
	cl_cdqjmb_info_t* ci = &(slave->dev_info.rf_stat.dev_priv_data.cdqjmb_info);

	switch (info->action) {
		case ACT_CDQJMB_NAME_SET:
			{
				pname = cci_pointer_data(info);

				if ((pname->index == 0) ||
					(pname->index > CDQJMB_KEY_MAXNUM)) {
					*ret = RS_INVALID_PARAM;
           			 return  false;
				}
				if (pname->len > CDQJMB_NAME_MAXLEN) {
					*ret = RS_INVALID_PARAM;
           			 return  false;
				}

				tlv->type = UP_TLV_CDQJMB_NAME_SET;
				tlv->len = sizeof(*pname) + pname->len;
				memcpy((u_int8_t*)&tlv[1], (u_int8_t*)pname, tlv->len);
				
				len = tlv->len + sizeof(*tlv);
			}
			break;
        default:
            *ret = RS_INVALID_PARAM;
            return  false;
            break;
    }
    
    if (len > 0) {
        rfgw_send_tt_packet(slave,buf,len);
    }
	
	return true;
}

/////////////////////////////////////////////////////////////////
//// 空调贴
static int rf_wk_air_code_fill(slave_t* slave, u_int8_t onoff, u_int8_t *code)
{
	int len = 0;
	priv_air_ir_stat_cache* cache;
	cl_wk_air_work_stat_t *wk_air = &slave->dev_info.rf_stat.dev_priv_data.wk_air_info.stat;
	
	cache = &slave->match_mana.ir_cache;
    if(!cache->is_ir_info_valid){
        log_debug("cache->is_ir_info_valid is false\n");
		return 0;
    }

	cache->onoff = onoff;
	cache->key_id = AC_KEY_POWER;

    if (!air_ir_get_ir_detail_code(slave, code, &len)||len <= 0) {
		log_debug("get ir code failed\n");
        return 0;
    }	

	return len;
}

void slave_cache_back(priv_air_ir_stat_cache* cache)
{
	if (!cache) {
		return;
	}

	cache->back_onoff = cache->onoff;
	cache->back_mode = cache->mode;
	cache->back_temp = cache->temp;
	cache->back_wind = cache->wind;
	cache->back_wind_direct = cache->wind_direct;
	cache->back_key_id = cache->key_id;
}

void slave_cache_revert(priv_air_ir_stat_cache* cache)
{
	if (!cache) {
		return;
	}

	cache->onoff = cache->back_onoff;
	cache->mode = cache->back_mode;
	cache->temp = cache->back_temp;
	cache->wind = cache->back_wind;
	cache->wind_direct = cache->back_wind_direct;
	cache->key_id = cache->back_key_id;
}

static bool _rf_wk_air_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret)
{
   	u_int8_t buf[1024] = {0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int len = 0;
	u_int8_t data_type = RF_BIG_PKT_IR_CTRL;
	priv_air_ir_stat_cache* cache;
	cl_wk_air_work_stat_t *wk_air = &slave->dev_info.rf_stat.dev_priv_data.wk_air_info.stat;

	//clean
	memset(buf, 0, sizeof(buf));
	
    switch (info->action) {
	case ACT_RF_COM_SHORTCUTS_ONOFF_QUERY:
		tlv->type = UP_TLV_WKAIR_SHORTCUTS_QUERY;
		tlv->len = 0;
		len = sizeof(*tlv);
		break;
	case ACT_RF_COM_SHORTCUTS_ONFF_SET:
		{
			// TODO:为了设备端处理方便，这里用通用定时器结构来传送快捷定时器
#if 1		
			cl_shortcuts_onoff_t *request = cci_pointer_data(info);
			ucp_comm_timer_t *puct = (ucp_comm_timer_t *)rf_tlv_val(tlv);
			ucp_comm_wkair_ext_t *padvance = (ucp_comm_wkair_ext_t *)puct->data;
			cache = &slave->match_mana.ir_cache;

			if(!cache->is_ir_info_valid){
			    log_debug("cache->is_ir_info_valid is false\n");
				return 0;
			}
			padvance->mode = wk_air->mode;
			padvance->tmp = wk_air->temp;
			padvance->win = wk_air->wind;
			padvance->dir = wk_air->wind_direct;
			padvance->key = AC_KEY_POWER;

			slave_cache_back(cache);
			cache->mode = padvance->mode;
			cache->temp = padvance->tmp;
			cache->wind = padvance->win;
			cache->wind_direct = padvance->dir;
			cache->key_id = padvance->key;
			
			memcpy((void *)&slave->shortcuts_onoff, (void *)request, sizeof(*request));
			puct->id = 100;
			puct->enable = request->enable;
			puct->start_time = 0;
			puct->duration = htons((u_int16_t)request->remain_time);
			
			cache->onoff = padvance->onoff = request->onoff;
			tlv->type = UP_TLV_WKAIR_SHORTCUTS_SET;
			len = rf_wk_air_code_fill(slave, request->onoff, padvance->code);
			slave_cache_revert(cache);
			if (len == 0) {
			    log_debug("rf_wk_air_code_fill code==0\n");
				return 0;
			}
			tlv->len = sizeof(*puct) + sizeof(*padvance) + (u_int8_t)len;
			len = sizeof(*tlv) + tlv->len;
			data_type = RF_BIG_PKT_IR_TIMER;
#else
			cl_shortcuts_onoff_t *request = cci_pointer_data(info);
			ucp_shortcuts_onoff_t so_net;
			
			memcpy((void *)&slave->shortcuts_onoff, (void *)request, sizeof(*request));
			
			so_net.enable = request->enable;
			so_net.onoff = request->onoff;
			so_net.remain_time = htons((u_int16_t)request->remain_time);
			tlv->type = UP_TLV_WKAIR_SHORTCUTS_SET;
			memcpy(rf_tlv_val(tlv), (void *)&so_net, sizeof(so_net));
			len = rf_wk_air_code_fill(slave, so_net.onoff, &buf[sizeof(*tlv) + sizeof(so_net)]);
			tlv->len = sizeof(so_net) + (u_int8_t)len;
			len = sizeof(so_net) + sizeof(*tlv) + len;
#endif
		}
		break;
	case ACT_RF_COM_TMP_ADJUST:
		{
			u_int8_t *value = NULL;
			
			tlv->type = UP_TLV_WKAIR_TEMP_ADJUST;
			tlv->len = 2;
			value = rf_tlv_val(tlv);
			*value = cci_u8_data(info);
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_RF_COM_LED_MODE:
		{
			u_int8_t *value = NULL;
			
			tlv->type = UP_TLV_WKAIR_LED_MODE;
			tlv->len = 2;
			value = rf_tlv_val(tlv);
			*value = cci_u8_data(info);
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_DIR:
		{
			u_int8_t *value = NULL;
			u_int8_t *pdata = NULL;

			pdata = cci_pointer_data(info);
			
			tlv->type = UP_TLV_WKAIR_DIR;
			tlv->len = *pdata;
			value = rf_tlv_val(tlv);
			memcpy((void *)value, (void *)&pdata[1], tlv->len);
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_IR_SET:
		{
			u_int16_t *id = cci_pointer_data(info);

			log_debug("mini wukong set new id %u\n", *id);

			reset_ir_info(slave, *id);

			slave->match_mana.rf_ir_match.cur_stat = MS_WK_PHONE_DOWN_SING;
		}
		break;
	case ACT_WKAIR_CHECK:
		{
			u_int8_t *value = NULL;
			u_int8_t *pdata = NULL;
			u_int8_t *pcode = NULL;
			ucp_wk_air_ir_ctrl* ic = NULL;
			priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
			
			pdata = cci_pointer_data(info);
			
			tlv->type = UP_TLV_WKAIR_CHECK;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			memcpy((void *)value, (void *)pdata, tlv->len);
			ic = (ucp_wk_air_ir_ctrl *)&value[tlv->len];
			pcode = ic->code;
			len = rf_wk_air_code_fill(slave, 1, pcode);
			ic->onoff = cache->onoff;
			ic->mode = cache->mode;
			ic->temp = cache->temp;
			ic->fan = cache->wind;
			ic->fan_dir = cache->wind_direct;
			ic->key_id = cache->key_id;
			log_debug("UP_TLV_WKAIR_CHECK get codelen=%u\n", len);
			tlv->len += (u_int8_t)(len + sizeof(*ic));
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_DIR_ADJUST:
		{
			u_int8_t *value = NULL;
			u_int8_t *pdata = NULL;
			u_int8_t *pcode = NULL;

			pdata = cci_pointer_data(info);
			
			tlv->type = UP_TLV_WKAIR_ADJUST;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			memcpy((void *)value, (void *)pdata, tlv->len);
			pcode = &value[tlv->len];
			// TODO:这里改成制冷，原因是制热可能要预热时间，导致方向校正不生效
			slave->match_mana.ir_cache.mode = AC_MODE_COLD;
			slave->match_mana.ir_cache.wind = AC_WIND_3;
			
			len = rf_wk_air_code_fill(slave, 1, pcode);
			tlv->len += len;
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_SHOCK_ADJUST:
		{
			u_int8_t *value = NULL;
			u_int8_t *data = NULL;
			
			tlv->type = UP_TLV_WKAIR_ADJUST;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			data = cci_pointer_data(info);
			value[0] = data[0];
			value[1] = data[1];
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_SHOCK_QUERY:
		{
			tlv->type = UP_TLV_WKAIR_SHOCK_QUERY;
			tlv->len = 0;
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_IR_QUERY:
		{
			tlv->type = UP_TLV_WKAIR_IR_QUERY;
			tlv->len = 0;
			len = sizeof(*tlv) + tlv->len;
		}
		break;
	case ACT_WKAIR_ADDR_SET:
		{
			u_int8_t *value = NULL;
			u_int8_t *data = NULL;
			
			tlv->type = UP_TLV_WKAIR_ADDR_SET;
			tlv->len = 4;
			value = rf_tlv_val(tlv);
			data = cci_pointer_data(info);
			value[0] = data[0];
			len = sizeof(*tlv) + tlv->len;
		}
		break;

	case ACT_WKAIR_IR_SYNC:
		{
			u_int8_t *value = NULL;
			u_int8_t onoff;
			
			tlv->type = UP_TLV_WKAIR_SYNC_CTRL;
			tlv->len = 2;
			value = rf_tlv_val(tlv);
			onoff = cci_u8_data(info);
			value[0] = onoff == 1 ? 1 : 2;
			len = sizeof(*tlv) + tlv->len;

			wk_air->ir_sync_ctrl = onoff;
		}
	default:
		break;
	}

    if (len == 0) {
		return true;
    }

    if (len < 20) {
       log_debug("%s action=%u\n", __FUNCTION__, info->action);
        rfgw_send_tt_packet(slave, buf, len);
    }else{
	    log_debug("%s action=%u big_pkt\n", __FUNCTION__, info->action);
#ifdef USE_BMP_TEST
		if (!rf_slave_send_big_pkt_v2(slave, data_type, buf, len, rbmp_callback)) {
			log_debug("%s rf_slave_send_big_pkt_v2 failed\n", __FUNCTION__);
		}
#else
        rf_slave_send_big_pkt(slave, BD_IR_CT_IR_CTRL, buf, len);
#endif 
    }	

	return true;
}

static bool _rf_wk_air_update_big_pkt(slave_t* slave, rf_b_pkt_defrag_t* frame)
{
    return true;
}

static RS _rf_wk_air_read_encode_data(slave_t *slave, u_int16_t index, u_int8_t *encode_data, int *data_len)
{
	int bytes_idx;
	u_int8_t data[512] = {0};
	int tmp, used;
	priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    cl_wk_air_info_t* ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);

	if (!cache->ir_detail) {
		log_err(false, "no ir_detail\n");
		return RS_ERROR;
	}

	// 当前IDX对应压缩数据偏移位置
	tmp = (index * cache->d_ir_bit_num);
	bytes_idx = tmp/8;
	used = (tmp % 8);
	
	*data_len = ((cache->d_ir_bit_num + used) % 8) == 0 ? ((cache->d_ir_bit_num + used)/8) : (((cache->d_ir_bit_num + used)/8) + 1) ;

	if (bytes_idx + *data_len >= cache->ir_detail_len) {
		return RS_ERROR;
	}

	memcpy(encode_data, cache->ir_detail + bytes_idx, *data_len);

	return RS_OK;
}

static void _rf_wk_air_decode_data(slave_t *slave, u_int16_t index, u_int8_t *encode_data, int encode_data_len, u_int8_t *decode_data, int *decode_data_len)
{	
	priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    cl_wk_air_info_t* ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);
	int i, j, bit_idx;

	// 还原
	memcpy(decode_data, cache->common_data, cache->item_len);

	bit_idx = (cache->d_ir_bit_num* index) % 8;

	for (i = 0; i < cache->item_len; i++) {
		if (cache->mask_data[i] == 0xff) {
			continue;
		}

		for (j = 0; j < 8; j++) {
			if (((cache->mask_data[i] >> j) & 1) == 0) {
				decode_data[i] |= ((encode_data[bit_idx/8] >> (bit_idx % 8)) & 1) << j;
				bit_idx++;
			}
		}
	}

	*decode_data_len = cache->item_len;
}


static RS _rf_wk_air_read_code_data(slave_t *slave, u_int16_t index, u_int8_t *data, int *data_len)
{
	u_int8_t encode_data[512];
	int encode_data_len = sizeof(encode_data);

	if (_rf_wk_air_read_encode_data(slave, index, encode_data, &encode_data_len) != RS_OK) {
		return RS_ERROR;
	}

	_rf_wk_air_decode_data(slave, index, encode_data, encode_data_len, data, data_len);

	return RS_OK;
}

/**
	比较两组byte数据的相似百分比
*/
static int _rf_wk_air_bytes_cmp(u_int8_t *s1, int s1_len, u_int8_t *s2, int s2_len, u_int8_t *bytes_map, int bm_len)
{
    int i, j, ret, match = 0, n = 0;

    for (i = 0; i < s1_len && i < s2_len && i < bm_len; i++) {
		s1[i] = s1[i] & bytes_map[i];
		s2[i] = s2[i] & bytes_map[i];

        ret = s1[i] ^ s2[i];

        for (j = 0; j < 8; j++) {
            if ((((ret) >> j) & 0x1) == 0)
                match++;
            n++;
        }
    }

   return match;
}

static void _rf_wk_air_decoder_get_status(int entry_num, int idx, u_int8_t *cOnoff, u_int8_t *cMode, u_int8_t *cTemp, u_int8_t *cWind, u_int8_t *cWinddir, u_int8_t *key)
{
	// 不同的类型解析方法不一样

	// 条目数为3000的计算方法
	// idx = cOnoff * 1500 + cMode * 300 + cTemp * 20 + cWind * 5 + cWinddir;
	if (entry_num == 3000) {
		//DEBUG("entry num == 3000\n");

		*cOnoff = idx/1500;
		//ERROR("cOnoff %u\n", *cOnoff);
	    idx %= 1500;

	    *cMode = (idx)/300;
		//ERROR("cMode %u\n", *cMode);
	    idx %= 300;

	    *cTemp = (idx)/20;
		//ERROR("cTemp %u\n", *cTemp);
	    idx %= 20;

	    *cWind = (idx)/5;
		//ERROR("cWind %u\n", *cWind);
	    idx %= 5;

	    *cWinddir = idx%5;
		//ERROR("cWinddir %u\n", *cWinddir);

		*key = 0;
	}
	// 15000条目 indx = cOnoff * 7500 + cMode * 1500 + cTemp * 100 + cWind * 25 + cWinddir * 5 + cKey
	else if (entry_num == 15000) {
		//DEBUG("entry num == 15000\n");
		*cOnoff = idx/7500;
	    idx %= 7500;

	    *cMode = (idx)/1500;
	    idx %= 1500;

	    *cTemp = (idx)/100;
	    idx %= 100;

	    *cWind = (idx)/25;
	    idx %= 25;

	    *cWinddir = idx/5;
		idx %= 5;

		*key = idx;
	} else {
		log_err(false, "get status error, lib entry_num=%u\n", entry_num);
		return;
	}

	log_debug("decoder get status: onoff %u mode %u temp %u wind %u winddir %u key %u\n", *cOnoff, *cMode, (*cTemp) + 16, *cWind, *cWinddir, *key);
}

static void _rf_wk_air_dump_mem(char *msg, u_int8_t *data, int len)
{
	u_int8_t buf[512] = {0};
	int idx = 0, i;

	for (i = 0; i < len; i++) {
		idx += sprintf(&buf[idx], "%02X ", data[i]);
	}

	log_debug("%s:\n%s\n", msg, buf);
}

static RS _rf_wk_air_get_idx(u_int16_t entry_num, u_int8_t cOnoff, u_int8_t cMode, u_int8_t cTemp, u_int8_t cWind, u_int8_t cWinddir, u_int8_t key, u_int16_t *idx)
{
	if (entry_num == 15000) {
		*idx = cOnoff * 7500 + cMode * 1500 + cTemp * 100 + cWind * 25 + cWinddir * 5 + key;
	}
	else if (entry_num == 3000) {
		*idx = cOnoff * 1500 + cMode * 300 + cTemp * 20 + cWind * 5 + cWinddir;
	}
	else {
		return RS_ERROR;
	}

	return RS_OK;
}


static bool _rf_wk_air_ir_onoff_same(slave_t *slave)
{
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
	u_int8_t ondata[512], offdata[512];
	int ndata;
	u_int16_t idx = 0;

	_rf_wk_air_get_idx(cache->d_item_num, AC_POWER_ON, AC_MODE_AUTO, AC_TEMP_BASE, AC_WIND_AUTO, AC_DIR_AUTO, AC_KEY_POWER, &idx);
	ndata = sizeof(ondata);
	_rf_wk_air_read_code_data(slave, idx, ondata, &ndata);
	_rf_wk_air_dump_mem("on", ondata, ndata);

	_rf_wk_air_get_idx(cache->d_item_num, AC_POWER_OFF, AC_MODE_AUTO, AC_TEMP_BASE, AC_WIND_AUTO, AC_DIR_AUTO, AC_KEY_POWER, &idx);
	ndata = sizeof(ondata);
	_rf_wk_air_read_code_data(slave, idx, offdata, &ndata);
	_rf_wk_air_dump_mem("off", offdata, ndata);

	if (memcmp(ondata, offdata, ndata)== 0) {
		return true;
	}

	return false;
}

static void _rf_wk_air_syn_stat(slave_t *slave, u_int8_t *code, int code_len)
{	
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
	int i;
	u_int8_t data[512], tmp_work[6];
	int ndata, best_count = 0, best_idx = 0, ret, match_count = cache->item_len * 8;;
	

	for (i = 0; i < cache->d_item_num; i++) {
		ndata = sizeof(data);
		if (_rf_wk_air_read_code_data(slave, i, data, &ndata) != RS_OK) {
			return;
		}

		ret = _rf_wk_air_bytes_cmp(code, code_len, data, ndata, cache->bytes_map, cache->item_len);
		if (ret > best_count) {
			best_idx = i;
			// 全匹配
			if (ret == match_count) {
				log_debug("good  match, i=%d / %d\n", i, match_count);
				goto done;
			}
		}
	}
done:	
	// 解析出索引对应的数值
	_rf_wk_air_decoder_get_status(cache->d_item_num, best_idx, &tmp_work[0], &tmp_work[1], &tmp_work[2], &tmp_work[3], &tmp_work[4], &tmp_work[5]);

	log_debug("	best match idx %d\n", best_idx);

	ndata = sizeof(data);
	if (_rf_wk_air_read_code_data(slave, best_idx, data, &ndata) != RS_OK) {
		return;
	}
	_rf_wk_air_dump_mem("best", data, ndata);

	if (_rf_wk_air_ir_onoff_same(slave) == true) {
		cache->onoff = !cache->onoff;
	} else {
		cache->onoff = tmp_work[0] == 0 ? 1 : 0;
	}

	cache->mode = tmp_work[1];
	cache->temp = tmp_work[2] + AC_TEMP_BASE;
	cache->wind = tmp_work[3];
	cache->wind_direct = tmp_work[4];
	cache->key_id = tmp_work[5];

}


static void _rf_wk_air_do_ir_code(slave_t *slave, wk_air_ir_code_get_reply_t *reply, int total_len)
{	
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    cl_wk_air_info_t* ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);
	int data_len = total_len - sizeof(*reply), wlen;
	
	log_debug("wkair get ir code reply: ret %u sync num %u offset %u data_len %d current lib item len %u local code widx %u\n", reply->ret, reply->sync_num, reply->offset, data_len, cache->item_len, ai->code_widx);

	if (reply->ret) {
		// 查询第二片，会返回处理中，这里特殊处理下
		if (!(reply->ret == 1 && reply->offset > 0)) {
			goto err;
		}
	}

	if (reply->sync_num != ai->sync_num) {
		log_err(false, "sync num %u != local sync num %u\n", reply->sync_num, ai->sync_num);
		goto err;
	}

	if (data_len <= 0) {
		goto err;
	}

	if (reply->offset != ai->code_widx) {
		log_err(false, "ignore offset %u, need %u\n", reply->offset, ai->code_widx);
		goto err;
	}

	wlen = data_len + ai->code_widx >= cache->item_len ? cache->item_len - ai->code_widx : data_len;
	memcpy(&ai->code[ai->code_widx], reply->data, wlen);

	log_debug("	wkair ir code write from %u -> %u total item len %u\n", ai->code_widx, ai->code_widx + wlen, cache->item_len);

	ai->code_widx += wlen;
	
	if (ai->code_widx < cache->item_len) {
		_rf_wkair_query_ir_code(slave, ai->code_widx);
		return;
	}

	_rf_wk_air_dump_mem("ir code get done", ai->code, cache->item_len);

	// 同步状态
	_rf_wk_air_syn_stat(slave, ai->code, cache->item_len);

	// 最新状态到设备
	rf_air_ir_ac_status_sync(slave);

err:
	// 重新开始
	ai->code_widx = 0;
}

static bool _rf_wk_air_update_data(slave_t* slave, rf_tlv_t* tlv, u_int8_t action)
{
    ucp_wk_air_stat_t* stat;
    cl_wk_air_info_t* ai;
	ucp_comm_timer_t *puct;
	ucp_comm_wkair_ext_t *padvance;
    priv_air_ir_stat_cache* cache;
	u_int8_t *pdata = NULL;
    
    ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    cache = &slave->match_mana.ir_cache;
    
    log_debug("_rf_wk_air_update_data &&&&&& tlv->type[%u] tlv->len[%u]\n",tlv->type,tlv->len);
    
    switch (tlv->type) {
        case UP_TLV_SMTDLOCK_CMD_PUSH_SUMMARY:
            stat = rf_tlv_val(tlv);

            log_debug("before on off %u mode %u temp %u wind %u dir %u ai->stat.ir_id=%u "
				"cache->cur_dev_ir_id=%u room_temp=%u tmp_adjus=%u tlvlen=%u\n",
				stat->onoff,stat->mode,stat->temp,stat->fan, stat->fan_dir, 
				ai->stat.ir_id ,stat->ir_id, stat->room_temp, stat->tmp_adjust, tlv->len);            
			//处理下非法数据
	        if (stat->temp < 16) {
	            stat->temp = 16;
	        }
	        if (stat->temp > 32) {
	            stat->temp = 32;
	        }
			if (stat->mode > AC_MODE_HOT) {
				stat->mode = AC_MODE_HOT;
			}
			if (stat->fan > AC_WIND_3) {
				stat->fan = AC_WIND_3;
			}
			if (stat->fan_dir > AC_DIR_3) {
				stat->fan_dir = AC_DIR_3;
			}
			if (stat->ir_id == 0xffff) {
				stat->ir_id = 0;
			}

            ai->stat.battary = stat->battary;
			if (ai->stat.battary < 5) {
				ai->stat.battary = 5;
			}
            ai->stat.ir_id = ntohs(stat->ir_id);
            ai->stat.room_humi = stat->room_humi;
            ai->stat.room_temp = stat->room_temp;
            ai->stat.charge = stat->charge;
            ai->stat.tmp_adjust = ((int8_t)stat->tmp_adjust)/10;
            ai->stat.led_mode = stat->led_mode;

            // save stat
            cache->onoff = ai->stat.onoff = !!stat->onoff;
            cache->mode = ai->stat.mode = stat->mode;
            cache->temp = ai->stat.temp = stat->temp;
            cache->wind = ai->stat.wind = stat->fan;
            cache->wind_direct = ai->stat.wind_direct = stat->fan_dir;
            
            log_debug("after on off %u mode %u temp %u wind %u dir %u ai->stat.ir_id=%u "
				"cache->cur_dev_ir_id=%u room_temp=%u tmp_adjus=%u tlvlen=%u\n",
				stat->onoff,stat->mode,stat->temp,stat->fan, stat->fan_dir, 
				ai->stat.ir_id ,stat->ir_id, stat->room_temp, stat->tmp_adjust, tlv->len); 

			// 第一次收到红外ID，判断下如果本地有这个一样的ID，就可以开始请求获取编码来同步了
			if (ai->ir_valid == 0) {
				if (ai->stat.ir_id != 0 && ai->stat.ir_id == cache->cur_dev_ir_id) {
					log_debug(" first get ir id %u and eq local ir id, try sync \n", ai->stat.ir_id);
					_rf_wkair_query_ir_code(slave, 0);
					//xxx_start(slave);
				}
			}
			
			ai->ir_valid = 1;
            if (ai->stat.ir_id != 0 && ai->stat.ir_id != cache->cur_dev_ir_id) {
                // ready update ir_id;
                if(slave->match_mana.rf_ir_match.cur_stat == MS_WK_IDLE){
                	reset_ir_info(slave,ai->stat.ir_id);
                }
                
            }
            break;
      	case UP_TLV_WKAIR_SHORTCUTS_QUERY:
			{
				u_int32_t time_tmp = 0;
				puct = rf_tlv_val(tlv);
				padvance = (ucp_comm_wkair_ext_t *)puct->data;
				slave->shortcuts_onoff.enable = puct->enable;
				slave->shortcuts_onoff.onoff = padvance->onoff;
				time_tmp = htons(puct->duration);
				if (slave->shortcuts_onoff.remain_time < (u_int32_t)time_tmp) {
					slave->shortcuts_onoff.remain_time = (u_int32_t)time_tmp;
				}
#ifdef USE_TIME_MINS
				slave->shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->time_diff * 60;
#else
				slave->shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->timezone * 3600;
#endif
				log_debug("sn=%"PRIu64" get shortcutse=%u o=%u t=%u rt=%u pso->remain_time=%u\n", 
					slave->sn, slave->shortcuts_onoff.enable, slave->shortcuts_onoff.onoff, 
					slave->shortcuts_onoff.time, slave->shortcuts_onoff.remain_time, slave->shortcuts_onoff.remain_time);
      		}
			break;
		case UP_TLV_WKAIR_GET_IR_CODE:
			_rf_wk_air_do_ir_code(slave, (wk_air_ir_code_get_reply_t *)rf_tlv_val(tlv), tlv->len);
			break;
		//case UP_TLV_WKAIR_CHECK:
		//	pdata = rf_tlv_val(tlv);
		//	event_push_err(slave->user->callback, UE_WKAIR_CHECK_STATUS, slave->handle, slave->user->callback_handle, (int)*pdata);
		//	break;
		//case UP_TLV_WKAIR_ADJUST:
		//	pdata = rf_tlv_val(tlv);
		//	event_push_err(slave->user->callback, UE_WKAIR_AUTO_SHOCK_CHECK, slave->handle, slave->user->callback_handle, (int)*pdata);
		//	break;
		case UP_TLV_WKAIR_SHOCK_QUERY:
			pdata = rf_tlv_val(tlv);
			event_push_err(slave->user->callback, UE_WKAIR_AUTO_SHOCK_CHECK, slave->handle, slave->user->callback_handle, (int)*pdata);
			break;
		case UP_TLV_WKAIR_IR_QUERY:
			pdata = rf_tlv_val(tlv);
			event_push_err(slave->user->callback, UE_WKAIR_CHECK_STATUS, slave->handle, slave->user->callback_handle, (int)*pdata);
			break;
        default:
            return false;
            break;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////
//橙灯情景面板
static bool _rf_cdqjmb_update_data(slave_t* slave, rf_tlv_t* tlv)
{
	cl_cdqjmb_info_t *ci = NULL;
	u_int8_t *pdata = NULL;
	u_int8_t num = 0;
	u_int8_t len = 0;
	u_int8_t *pvalue = NULL;

	ci = &(slave->dev_info.rf_stat.dev_priv_data.cdqjmb_info);
	slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);

	log_debug("%s &&&&&& tlv->type[%u] tlv->len[%u]\n", 
		__FUNCTION__, tlv->type, tlv->len);

    switch (tlv->type) {
        case UP_TLV_CDQJMB_NAME_PUSH:
			pdata = rf_tlv_val(tlv);
			//check key num
			num = pdata[0];
			len = pdata[1];
			pvalue = &pdata[2];
			if ((num == 0) ||
				(num > CDQJMB_KEY_MAXNUM)) {
				log_debug("%s %"PRIu64" err keynum=%u\n", __FUNCTION__, slave->sn, num);
				return false;
			}
			//check len
			if (len > CDQJMB_NAME_MAXLEN) {
				log_debug("%s %"PRIu64" err len=%u\n", __FUNCTION__, slave->sn, len);
				return false;
			}
			memset((void *)ci->key_conf[num - 1].name, 0, sizeof(ci->key_conf[num - 1].name));
			if (len > 0) {
				memcpy((void *)ci->key_conf[num - 1].name, (void *)pvalue, len);
			}
			// TODO: 保存一下
			_rf_com_write_user_info(slave, CDQJMB_CONF_ID, (u_int8_t*)ci, sizeof(*ci));
			log_debug("slave sn=%"PRIu64" key=%u name=%s\n", slave->sn, num-1, ci->key_conf[num - 1].name);
            break;
		case RF_TT_CMD_QUERY_HISLOG_V2:
			return rf_update_comm_history_item_v2(slave, rf_tlv_val(tlv), tlv->len);
        default:
            return false;
            break;
    }
	
	return true;
}

/////////////////////////////////////////////////////////////////
//智皇电机状态更新
static bool _rf_zhdj_update_data(slave_t* slave, rf_tlv_t* tlv, u_int8_t action)
{
    cl_zhdj_info_t* zi;
	u_int8_t *pdata;
    
    zi = &(slave->dev_info.rf_stat.dev_priv_data.zhdj_info);
    slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
    
    log_debug("_rf_zhdj_update_data &&&&&& tlv->type[%u] tlv->len[%u]\n",tlv->type,tlv->len);
    
    switch (tlv->type) {
        case UP_TLV_ZHDJ_LOCATION_GET:
			pdata = rf_tlv_val(tlv);
			if (*pdata > 100) {
				zi->percent = 0;
			} else {
				zi->percent = 100 - *pdata;
			}
            break;
		case RF_TT_CMD_GET_RUN_TIME:
			slave->run_time = htonl(*(u_int32_t *)rf_tlv_val(tlv));
			log_debug("sn=%"PRIu64" runtime=%u\n", slave->sn, slave->run_time);
			break;
        default:
            return false;
            break;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////
//终端设备更新状态

static bool _rf_com_ctrl_proc_notify(slave_t* slave, cln_common_info_t *info, RS *ret, DIR_CTRL_FUNC_PTR func)
{
    u_int8_t buf[512] = {0};  // 512够用了
    int len = 0,type;
    u_int32_t value;
	
    if (!func) {
        *ret =RS_INVALID_PARAM;
        return  false;
    }
    
    switch (info->action) {
        case ACT_RF_COM_DIRECT_CTRL:
            value = cci_u32_data(info);
            type = (value >> 16)&0xFF;
            len = func(slave,info->action, type&0xFF , value&0xFF , buf);
            break;
        case ACT_RF_COM_AIR_IR_CTRL:
            value = cci_u32_data(info);
            type = (value >> 8) & 0xFF;
            len = func(slave,info->action, type&0xFF, value&0xFF,buf);
            break;
        default:
            break;
    }
    
    if (len >=  0) {
        if (len > 0) {
            rfgw_send_tt_packet(slave,buf,len);
        }
        return true;
    }
    
    *ret = RS_INVALID_PARAM;
    return  false;
} 

static bool rf_air_ir_do_tlv(slave_t* slave,rf_tlv_t* tlv,u_int8_t action)
{
    ucp_air_ir_learn_resp_t* ir_res;
    int event = 0;
    
    if (slave->match_mana.rf_ir_match.cur_stat == MS_WK_IDLE) {
		log_debug("rf_air_ir_do_tlv MS_WK_IDLE\n");
        return false;
    }
    
    log_debug("rf_air_ir_do_tlv type %u \n",tlv->type);
    
    switch (tlv->type) {
        case UP_TLV_AIR_IR_LEARN_RET:
            ir_res = rf_tlv_val(tlv);
            slave->match_mana.rf_ir_match.error = ERR_NONE;
            
            log_debug("rf_air_ir_do_tlv proc ir learn  result action[%u] err[%u] matchstatus=%u\n",
				ir_res->action,ir_res->error, slave->match_mana.rf_ir_match.cur_stat);
            if (ir_res->action == ACT_START_IR_LEARN) {
				//当另一个手机在学习的时候，发学习指令返回的错误；
                if ((slave->match_mana.rf_ir_match.cur_stat != MS_WK_WAIT_DEV_START) &&
                    (slave->match_mana.rf_ir_match.cur_stat !=  MS_WK_DEV_WAIT_SING)) {
                    break;
                }
				//防止第一丢包，第二次变成err，但因为上次是自己操作的，可以直接进入下一阶段
                if ((ir_res->error == ERR_NONE) || (ir_res->ir_num == slave->ir_num)) {
                    // TO wait signal
                    air_ir_code_match_set_status(slave, MS_WK_DEV_WAIT_SING);
                    event = SAE_CODE_MATCH_DEV_READY_OK;
                }else{
                    air_ir_code_match_set_status(slave, MS_WK_IDLE);
                    event = SAE_CODE_MATCH_START_FAILED;
                    slave->match_mana.rf_ir_match.error = MS_WK_ERR_DEV_BUSY;
                }
            }else if(ir_res->action == ACT_STOP_IR_LEARN){
            	//当另一个手机在学习的时候，发停止学习指令返回的错误；
                //already reset stat when call stop
                if ((ir_res->error == ERR_NONE) ||(ir_res->ir_num == slave->ir_num)) {
                    event = SAE_CODE_MATCH_STOP_OK;
                }else{
                    slave->match_mana.rf_ir_match.error = MS_WK_ERR_DEV_BUSY;
                    event = SAE_CODE_MATCH_STOP_FAILED;
                }
            }else if(ir_res->action == ACT_DEV_WAIT_IR_SINGAL){
                // may be our had timout
                //设备等待按遥控器超时后恢复，(如果不做超时，则手机退出，永远不能再学习)，为了版本兼容性
                //这里不限制了，可以在app上返回第二次学习直接进入学习状态
				if ((ir_res->error == ERR_NONE) && (slave->match_mana.rf_ir_match.cur_stat == MS_WK_WAIT_DEV_START)) {
					air_ir_code_match_set_status(slave, MS_WK_DEV_WAIT_SING);
					event = SAE_CODE_MATCH_DEV_READY_OK;
					goto done;
				}
				
                if (slave->match_mana.rf_ir_match.cur_stat != MS_WK_DEV_WAIT_SING) {
                    break;
                }

                if (ir_res->error != ERR_NONE && slave->match_mana.rf_ir_match.cur_stat != MS_WK_IDLE) {
                    
                    event = SAE_CODE_MATCH_FAILED;
                    slave->match_mana.rf_ir_match.error = MS_WK_ERR_DEV_WAIT_IR_TIME_OUT;
                    air_ir_code_match_set_status(slave, MS_WK_IDLE);
                }
            }else if(ir_res->action == ACT_DEV_DATABASE_RECV){
            	//上传编码给设备成功了，但编码是全0，无效，不能控制的
                if (slave->match_mana.rf_ir_match.cur_stat != MS_WK_NOTIFY_DEV_CODE_ID) {
                    break;
                }
                air_ir_code_match_set_status(slave, MS_WK_IDLE);
                if (ir_res->error == ERR_NONE) {
                    event = SAE_CODE_MATCH_OK;
                }else{
                    event = SAE_CODE_MATCH_FAILED;
                    slave->match_mana.rf_ir_match.error = MS_WK_ERR_IR_TO_DEVICE_TIMEOUT;
                }
            }else if(ir_res->action == ACT_DEV_RECV_IR){
            	//开始发送红外编码库时就可以置上
                if (ir_res->error == ERR_NONE) {
                    event = SAE_CODE_MATCH_START_RECV_IR_CODE;
                }
            }
            break;
        default:
            return false;
            break;
    }

done:    
    if (event != 0) {
        event_push(slave->user->callback, event, slave->handle, slave->user->callback_handle);
    }
    
    return false;
}

//发送学习成功
void rf_air_ir_start_db_send(slave_t* slave)
{
	u_int8_t buf[20];
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	ucp_air_ir_learn_resp_t* ir_res = rf_tlv_val(tlv);

	tlv->type = UP_TLV_AIR_IR_LEARN_RET;
	tlv->len = sizeof(*ir_res);
	ir_res->action = ACT_DEV_RECV_IR;
	ir_res->error = ERR_NONE;

	rf_air_ir_do_tlv(slave, tlv, 0);
}

//数据库传送超时
void rf_air_ir_start_db_timeout(slave_t* slave)
{
	u_int8_t buf[20];
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	ucp_air_ir_learn_resp_t* ir_res = rf_tlv_val(tlv);

	tlv->type = UP_TLV_AIR_IR_LEARN_RET;
	tlv->len = sizeof(*ir_res);
	ir_res->action = ACT_DEV_DATABASE_RECV;
	ir_res->error = ERR_SYSTEM;

	rf_air_ir_do_tlv(slave, tlv, 0);
}

//发送数据库给设备成功
void rf_air_ir_send_db_ok(slave_t* slave)
{
	u_int8_t buf[20];
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	ucp_air_ir_learn_resp_t* ir_res = rf_tlv_val(tlv);

	tlv->type = UP_TLV_AIR_IR_LEARN_RET;
	tlv->len = sizeof(*ir_res);
	ir_res->action = ACT_DEV_DATABASE_RECV;
	ir_res->error = ERR_NONE;

	rf_air_ir_do_tlv(slave, tlv, 0);
}

//接收设备数据超时，直接报匹配编码无效啥的
void rf_air_ir_recv_timeout(slave_t *slave)
{
	if (!slave) {
		return;
	}

	slave->match_mana.rf_ir_match.error = MS_WK_ERR_SERVER_IR;
	event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
	air_ir_code_match_set_status(slave, MS_WK_IDLE);
}

static u_int8_t* decompress_ir_code(rf_b_pkt_defrag_t* frame, u_int8_t* code_buf,u_int16_t* code_len)
{
    int i,len;
    u_int16_t tmp;
    u_int16_t* ptr = (u_int16_t*)code_buf;
    
    if (!frame->pkt || !frame->pkt_len || !ptr) {
        return NULL;
    }
    
    
    for (i = 0; i < frame->pkt_len; ) {
        if (!frame->pkt[i]) {
            return NULL;
        }
        
        if ((i+1) < frame->pkt_len && frame->pkt[i+1] != 0) {
            
            tmp = ((u_int16_t)frame->pkt[i])*25;
            if (i < 5) {
                log_info("decompress_ir_code i = %u tmp = %u htons(tmp) = %u\n",i,tmp,htons(tmp));
            }
            i++;
        }else{
       		if (i + 1 == frame->pkt_len) {
				tmp = ((u_int16_t)frame->pkt[i])*25;
				i++;
			} else {
	            tmp = ((u_int16_t)frame->pkt[i] + (((u_int16_t)frame->pkt[i+2])<<8))*25;
	            i+=3;
			}
        }
        
        *ptr = htons(tmp);
        ptr++;
    }
    
    len = (int)((u_int8_t*)ptr - code_buf);
    if (len <= 0) {
        return NULL;
    }

    
    if (code_len) {
        *code_len = len;
    }
    
    return code_buf;
}

static bool air_ir_do_big_pkt(slave_t* slave, rf_b_pkt_defrag_t* frame)
{
    priv_rf_ir_match_t* mi;
    u_int8_t code_buf[2048],err = 0;
    ucp_obj_t* uo;
    u_int8_t* dest;
    u_int16_t len = 0,d_len;
    u_int16_t* num;
    
    switch (frame->data_type) {
        case BD_IR_CT_IR_UPLOAD:
            mi = &slave->match_mana.rf_ir_match;
            
            log_debug("air_ir_do_big_pkt recv all packet len %u cur_stat %u\n",frame->pkt_len, mi->cur_stat);

            if (mi->cur_stat == MS_WK_DEV_WAIT_SING ) {
                
                //memdump("air_ir_do_big_pkt pre", frame->pkt, frame->pkt_len);
                
                if (!decompress_ir_code(frame,code_buf,&len) || !len || (len&0x1) != 0) {
                    mi->error = err = MS_WK_ERR_DEV_TRANS_IR;
                    break;
                }
                
                //memdump("air_ir_do_big_pkt after", code_buf, len);

                
                d_len = len+sizeof(*num);
                
                uo = cl_calloc(d_len + sizeof(*uo), 1);
                if (!uo) {
                    mi->error = err = MS_WK_ERR_DEV_TRANS_IR;
                    break;
                }
                
                
                fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_MATCH, d_len);
                
                num = (u_int16_t*)(uo+1);
                *num = htons(len/2);
                dest = (u_int8_t*)(num+1);
                memcpy(dest, code_buf, len);
                
                log_debug("air_ir_do_big_pkt after decompress len = %u num[%u] ntohs num[%u] d_len=%u\n",len,*num,
                          ntohs(*num),d_len);
                
                
                SAFE_FREE(mi->matched_code);
                mi->code_len = 0;
                
                mi->matched_code = (u_int8_t*)uo;
               	mi->code_len = d_len+sizeof(*uo);
                
                air_ir_code_match_set_status(slave,MS_WK_SERVER_MATCH);
                air_ir_start_to_server_match(slave);
            }
            break;
            
        default:
            break;
    }
    
    if (err != 0) {
        event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
        air_ir_code_match_set_status(slave,MS_WK_IDLE);
    }
    
    
    return false;
}

static bool is_tlv_need_common_doing(rf_tlv_t* tlv)
{
    bool res = false;
    
    switch (tlv->type) {
        case UP_TLV_AIR_IR_LEARN_RET:
        case UP_TLV_BIG_DATA_GET:
        case UP_TLV_BIG_DATA_SEND_REQUEST:
        case UP_TLV_BIG_DATA_RET:
            res = true;
            break;
            
        default:
            break;
    }
    
    return res;
}



static bool udp_rf_dev_common_tlv_hook(slave_t* slave,rf_tlv_t* tlv, u_int8_t action)
{
    bool res = false;
    
    switch (tlv->type) {
        case UP_TLV_AIR_IR_LEARN_RET:
            res = rf_air_ir_do_tlv(slave,tlv,action);
            break;
        case UP_TLV_BIG_DATA_GET:
            // don't need notify app
            rf_slave_proc_frame_get(slave, tlv);
            break;
        case UP_TLV_BIG_DATA_SEND_REQUEST:
            rf_slave_recv_big_pkt(slave,tlv);
            break;
        case UP_TLV_BIG_DATA_RET:
            rf_slave_do_big_pkt_ret(slave,tlv);
            break;
            
        default:
            break;
    }
    
    return res;
}

static bool udp_rf_do_big_packet(slave_t* slave,rf_b_pkt_defrag_t* frame)
{
    bool res = false;

	log_debug("udp_rf_do_big_packet\n");
    
    if (!frame->pkt) {
        goto end;
    }

	log_debug("udp_rf_do_big_packet type %u\n", frame->data_type);
    
    if(frame->data_type == BD_IR_CT_IR_UPLOAD)
    {
        res = air_ir_do_big_pkt(slave,frame);
        goto end;
    }

	//处理下公共透传的定时tlv
	if (comm_timer_support(slave->ext_type) && 
		parse_comm_timer(slave, (rf_tlv_t*)(frame->pkt), frame->pkt_len)) {
		return true;
	}	
    
    switch (slave->ext_type) {
        case RF_EXT_TYPE_WK_AIR:
		case RF_EXT_TYPE_WK_AIR2:
            res = _rf_wk_air_update_big_pkt(slave,frame);
            break;
            
        default:
            break;
    }
    
end:
	log_debug("frame %p is proced\n", frame);
    frame->is_proc_pkt = true;
    return res;
}


static bool udp_rf_update_server_ia_obj(slave_t* slave,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    
    switch (obj->sub_objct) {
        case UCSOT_IA_CODE:
            ret = udp_rf_update_server_ia_code_subobj(slave,action,obj);
            break;
            
        default:
            break;
    }
    
    return ret;
}

static bool udp_rf_update_server_obj_data(slave_t* slave,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    
    log_debug("udp_rf_update_server_obj_data obj %u sobj %u attr %u len %u\n",obj->objct,obj->sub_objct,
              obj->attr,obj->param_len);
    
    switch (obj->objct) {
        case UCOT_IA:
            ret = udp_rf_update_server_ia_obj(slave,action,obj);
            break;
            
        default:
            break;
    }
    return ret;
}

bool udp_rf_dev_do_server_uc(slave_t* slave,ucp_ctrl_t* uc,int total)
{
    int index;
    ucp_obj_t* obj;
    
    if (uc->count == 0) {
        log_err(false, "uasc udp ctrl :count == 0\n");
        return false;
    }
    
    total -= sizeof(*uc);
    obj = (ucp_obj_t*)(uc + 1);
    
    for (index = 0; index < uc->count && total >= sizeof(ucp_obj_t); index++) {
        ucp_obj_order(obj);
        
        udp_rf_update_server_obj_data(slave, uc->action, obj);
        
        obj = (ucp_obj_t*)((char*)(obj+1) + obj->param_len);
    }

    return true;
}

static bool is_ir_learn_ret(rf_tlv_t* tlv)
{
    bool res = false;
    
    switch (tlv->type) {
        case UP_TLV_AIR_IR_LEARN_RET:
            res = true;
            break;
            
        default:
            break;
    }
    
    return res;	
}

bool udp_rf_dev_update_date(slave_t* slave,rf_tlv_t* tlv, u_int8_t action)
{    
#ifdef USE_BMP_TEST
    bool res = false;

	log_debug("udp_rf_dev_update_date s_etype=%u sn=%"PRIu64" type=%u \n", 
		slave->ext_type, slave->sn, tlv->type);
	if (is_bmp(tlv)){
        return udp_rf_dev_bmp_tlv_hook(slave,tlv,action);
    }

	if (is_ir_learn_ret(tlv)) {
		return udp_rf_dev_common_tlv_hook(slave,tlv,action);
	}
#else
    bool res = false;
    rf_b_pkt_defrag_t* frame;

	log_debug("udp_rf_dev_update_date s_etype=%u sn=%"PRIu64" type=%u \n", 
		slave->ext_type, slave->sn, tlv->type);	
    if (is_tlv_need_common_doing(tlv)){
        return udp_rf_dev_common_tlv_hook(slave,tlv,action);
    }
    
    if (tlv->type == UP_TLV_BIG_DATA_RESPONSE)
    {
        frame = rf_slave_recv_big_pkt(slave,tlv);
        if (frame != NULL && !frame->is_proc_pkt) {
            res = udp_rf_do_big_packet(slave,frame);
            memset(frame, 0, sizeof(*frame));
        }
        return res;
    }
#endif
    
    switch (slave->ext_type) {
        case RF_EXT_TYPE_DOOR_LOCK:
            res = _rf_door_lock_update_data(slave,tlv);
            break;
        case RF_EXT_TYPE_DOOR_MAGNET:
		case RF_EXT_TYPE_DOOR_MAGNETV2:
        case RF_EXT_TYPE_HM_MAGENT:
		case RF_EXT_TYPE_YLLOCK:
            res = _rf_door_magnet_update_data(slave, tlv);
            break;
        case RF_EXT_TYPE_DHX:
		case RF_EXT_TYPE_DHXZH:
		case RF_EXT_TYPE_DHXCP:
		case RF_EXT_TYPE_DHXML:
		case RF_EXT_TYPE_LHX:
            res = _rf_dhx_update_data(slave, tlv);
            break;
        case RF_EXT_TYPE_YT_DOOR_LOCK:
			res = _rf_door_lock_update_data(slave,tlv);
            break;
		case RF_EXT_TYPE_KTCZ:
			res = _rf_ktcz_update_data(slave, tlv);
			break;
		case RF_EXT_TYPE_HEATING_VALVE:			
			res = _rf_heating_valve_update_data(slave, tlv);
			break;
        case RF_EXT_TYPE_HM_BODY_DETECT:
		case RF_EXT_TYPE_YLTC:
		case RF_EXT_TYPE_WUANS6:
            res = _rf_hm_body_detect_update_data(slave, tlv);
            break;
		case RF_EXT_TYPE_YLLIGHT:
			res = _rf_yllight_update_data(slave, tlv);
			break;
        case RF_EXT_TYPE_HM_ENV_DETECT:
		case RF_EXT_TYPE_YLWSD:
            res = _rf_hm_temp_hum_update_data(slave, tlv);
            break;
		case RF_EXT_TYPE_GAS:
		case RF_EXT_TYPE_QSJC:
		case RF_EXT_TYPE_HMCO:
		case RF_EXT_TYPE_HMYW:
		case RF_EXT_TYPE_HMQJ:
		case RF_EXT_TYPE_YLSOS:
			res = _rf_com_detector_update_data(slave, tlv);
			break;
		case RF_EXT_TYPE_HTLLOCK:
			res = _rf_htllock_update_data(slave, tlv, action);
			break;
        case RF_EXT_TYPE_WK_AIR:
		case RF_EXT_TYPE_WK_AIR2:
            res = _rf_wk_air_update_data(slave,tlv,action);
            break;
		case RS_EXT_TYPE_ZHDJ:
            res = _rf_zhdj_update_data(slave,tlv,action);
			break;
		case RF_EXT_TYPE_DWKJ:
			res = _rf_dwkj_update_data(slave,tlv,action);
			break;
			
		case RF_EXT_TYPE_SCENE_CONTROLLER:
			res = _rf_scene_controller_update_data(slave, tlv);
			break;
		case RS_EXT_TYPE_CDQJMB:
			res = _rf_cdqjmb_update_data(slave, tlv);
			break;
		case RF_EXT_TYPE_JQ:
			res = _rf_jq_update_data(slave, tlv);
			break;
        default:
            break;
    }
    return res;
}

bool udp_rf_dev_update_raw_date(slave_t* slave,u_int8_t* buf,u_int16_t len, bool cache)
{
    bool res = false;

	if (is_led_type(slave)) {
		return _rf_lamp_update_data(slave,buf,len, cache);
	}
    
    switch (slave->ext_type) {
        case RF_EXT_TYPE_LED_LAMP:
		case RF_EXT_TYPE_DWHF:
		case RF_EXT_TYPE_DWYKHF:
		case RF_EXT_TYPE_DWYSTGQ:
            res = _rf_lamp_update_data(slave,buf,len, cache);
            break;
            
        default:
            break;
    }
    return res;
}


bool udp_rf_dev_update_cache_date(slave_t* slave, u_int8_t action, u_int8_t* buf, u_int16_t len)
{
    bool res = false;

	log_debug("[%"PRIu64"] update cache date, ext_type 0x%x len %u\n", slave->sn, slave->ext_type, len);
    
    switch (slave->ext_type) {
        case RF_EXT_TYPE_DOOR_MAGNET:

		case RF_EXT_TYPE_KTCZ:
            break;
			
		case RF_EXT_TYPE_HM_BODY_DETECT:
		case RF_EXT_TYPE_YLTC:
		case RF_EXT_TYPE_WUANS6:
			res = _rf_hm_body_detect_update_cache_data(slave, action, buf, len);
			break;

		case RF_EXT_TYPE_HM_ENV_DETECT:
		case RF_EXT_TYPE_YLWSD:
			res = _rf_hm_temp_hum_update_cache_data(slave, action, buf, len);
			break;

        case RF_EXT_TYPE_DHX:
		case RF_EXT_TYPE_DHXZH:
		case RF_EXT_TYPE_DHXCP:
		case RF_EXT_TYPE_DHXML:
		case RF_EXT_TYPE_LHX:
			res = _rf_dhx_update_cache_data(slave, action, buf, len);
			break;
		
		case RF_EXT_TYPE_GAS:
		case RF_EXT_TYPE_QSJC:
		case RF_EXT_TYPE_HMCO:
		case RF_EXT_TYPE_HMYW:
		case RF_EXT_TYPE_HMQJ:
		case RF_EXT_TYPE_YLSOS:
			res = _rf_com_detector_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_YLLIGHT:
			res = _rf_yllight_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_YT_DOOR_LOCK:		
			res = _rf_yt_door_lock_update_cache_data(slave, action, buf, len);
			break;
			
		case RF_EXT_TYPE_DOOR_LOCK:		
			res = _rf_door_lock_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_DOOR_MAGNETV2:
		case RF_EXT_TYPE_HM_MAGENT:
		case RF_EXT_TYPE_YLLOCK:
			res = _rf_door_magnetv2_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_HTLLOCK:
			res = _rf_htllock_update_cache_data(slave, action, buf, len);
            break;
		case RF_EXT_TYPE_WK_AIR:
		case RF_EXT_TYPE_WK_AIR2:
			res = _rf_wkair_update_cache_data(slave, action, buf, len);
			break;
		case RS_EXT_TYPE_ZHDJ:
			res = _rf_zhdj_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_DWKJ:
			res = _rf_dwkj_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_SCENE_CONTROLLER:
			res = _rf_scene_controller_update_cache_data(slave, action, buf, len);
			break;

		case RF_EXT_TYPE_HEATING_VALVE:
			res = _rf_heating_valve_update_cache_data(slave, action, buf, len);
			break;
		case RS_EXT_TYPE_CDQJMB:
			res = _rf_cdqjmb_valve_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_JQ:
			res = _rf_jq_update_cache_data(slave, action, buf, len);
			break;
		case RF_EXT_TYPE_LIGHT_SENSE:
			res = _rf_light_sense_update_cache_data(slave, action, buf, len);
			break;
        default:
            break;
    }
    return res;
}

//填充查询报文
u_int8_t udp_rf_dev_mk_stat_query_pkt(slave_t* slave,rf_tlv_t* tlv)
{
    u_int8_t len = 0;
    
    switch (slave->ext_type) {
        case RF_EXT_TYPE_DOOR_LOCK:
        case RF_EXT_TYPE_DOOR_MAGNET:
		case RF_EXT_TYPE_DOOR_MAGNETV2:
		case RF_EXT_TYPE_DHX:
		case RF_EXT_TYPE_DHXZH:
		case RF_EXT_TYPE_DHXCP:
        case RF_EXT_TYPE_YT_DOOR_LOCK:
        case RF_EXT_TYPE_HM_MAGENT:
        case RF_EXT_TYPE_HM_BODY_DETECT:
		case RF_EXT_TYPE_WUANS6:
		case RF_EXT_TYPE_YLTC:
		case RF_EXT_TYPE_YLLOCK:
        case RF_EXT_TYPE_HM_ENV_DETECT:
		case RF_EXT_TYPE_GAS:
   		case RF_EXT_TYPE_QSJC:
		case RF_EXT_TYPE_HMCO:
		case RF_EXT_TYPE_HMYW:
		case RF_EXT_TYPE_HMQJ:
		case RF_EXT_TYPE_HTLLOCK:
		case RF_EXT_TYPE_HEATING_VALVE:
		case RF_EXT_TYPE_WK_AIR:
		case RF_EXT_TYPE_WK_AIR2:
		case RS_EXT_TYPE_ZHDJ:
		case RF_EXT_TYPE_YLWSD:
		case RF_EXT_TYPE_YLSOS:
		case RF_EXT_TYPE_YLLIGHT:
		case RF_EXT_TYPE_DHXML:
		case RF_EXT_TYPE_LHX:
            len = _rf_door_lock_stat_query_pkt(slave,tlv);
            break;
     	case RF_EXT_TYPE_KTCZ:
			len = _rf_ktcz_stat_query_pkt(slave,tlv);
			break;
        default:
            len = _rf_door_lock_stat_query_pkt(slave,tlv);
    }
    
    return len + sizeof(*tlv);
}

u_int8_t udp_rf_dev_mk_raw_stat_query_pkt(slave_t* slave,u_int8_t* buf)
{
    u_int8_t len = 0;

	if (is_led_type(slave)) {
		return _rf_lamp_mk_query_pkt(slave,buf);
	}
    
    switch (slave->ext_type) {
        case RF_EXT_TYPE_LED_LAMP:
        case RF_EXT_TYPE_DWHF:
		case RF_EXT_TYPE_DWYKHF:
		case RF_EXT_TYPE_DWYSTGQ:
            len = _rf_lamp_mk_query_pkt(slave,buf);
            break;
        
        default:
            break;
    }
    
    return len;
}

//释放APP层slave结构数据
void udp_rf_dev_free_cl_data(cl_slave_t* slave)
{
	//释放定时器
	SAFE_FREE(slave->comm_timer.timer);
	SAFE_FREE(slave->timer_summary.stat_count);		
}

//释放slave结构数据
void udp_rf_dev_free_slave_data(slave_t* slave)
{
    //释放通用数据
    SAFE_FREE(slave->dev_info.rf_stat.work_list);
	//释放定时器
	SAFE_FREE(slave->comm_timer.timer);
	SAFE_FREE(slave->timer_summary.stat_count);		
}

static int _rf_air_ir_proc_notify(slave_t* slave, u_int8_t action,u_int8_t type, u_int8_t value, u_int8_t* pktbuf)
{
    int len = 0;
    rf_tlv_t* tlv = (rf_tlv_t*)pktbuf;
    ucp_air_ir_learn_ctrl_t* irc = (ucp_air_ir_learn_ctrl_t*)(tlv+1);
    priv_air_ir_stat_cache* cache;
	cl_wk_air_info_t* ai;
    bool res = false;

    //不支持其他通用控制
    if (action != ACT_RF_COM_AIR_IR_CTRL || !pktbuf) {
        return -1;
    }
    
    cache = &slave->match_mana.ir_cache;
    ai = &(slave->dev_info.rf_stat.dev_priv_data.wk_air_info);

	if (ai->ir_valid == 0) {
		return 0;
	}
	
    switch (type) {
        case CT_WK_MODE:
            cache->key_id = AC_KEY_MODE;
            cache->mode = value;
            if(cache->mode > AC_MODE_HOT){
                cache->mode = AC_MODE_HOT;
            }
            res = air_rf_ir_send_signal(slave);
            break;
        case CT_WK_ONOFF:		
            cache->key_id = AC_KEY_POWER;
            cache->onoff = !!value;
            res = air_rf_ir_send_signal(slave);
            break;
        case CT_WK_TEMP:
            cache->key_id = AC_KEY_TEMP;
            cache->temp = value;
            if (cache->temp > 32) {
                cache->temp = 32;
            }
            
            if (cache->temp < 16) {
                cache->temp = 16;
            }
            
            res = air_rf_ir_send_signal(slave);
            
            break;
        case CT_WK_WIND:
            cache->key_id = AC_KEY_WIND;
            value = air_cl_wind_2_net_wind(value,false);
            cache->wind = value;
            if (cache->wind > AC_WIND_3) {
                cache->wind =  AC_WIND_3;
            }
            res = air_rf_ir_send_signal(slave);
            break;
        case CT_WK_WIND_DIR:
            cache->key_id = AC_KEY_DIR;
            cache->wind_direct = value;
            if (cache->wind_direct > AC_DIR_3) {
                cache->wind_direct = AC_DIR_3;
            }
            res = air_rf_ir_send_signal(slave);
            break;
        case CT_WK_START_MATCH:
			if (slave->ir_num == 0) {
				srand((u_int32_t)time(NULL));
				slave->ir_num = rand()%0x100;
			}
            irc->action = ACT_START_IR_LEARN;
			irc->ir_num = slave->ir_num;
			//这里按强哥说的，兼容以前的008，timeout下发0，设备默认3分钟
			//irc->timeout = AIR_IR_CODE_LEARN_TIMEOUT;
			irc->timeout = 0;
            tlv->type = UP_TLV_AIR_IR_LEARN_CTRL;
            tlv->len = sizeof(*irc);
            len = tlv->len + sizeof(*tlv);
			ir_ql_exit();
            uasc_try_connect_to_server(cl_priv->uasc_session);
            air_ir_code_match_set_status(slave, MS_WK_WAIT_DEV_START);
            res = true;
            break;
        case CT_WK_STOP_MATCH:
            irc->action = ACT_STOP_IR_LEARN;
            tlv->type = UP_TLV_AIR_IR_LEARN_CTRL;
            tlv->len = sizeof(*irc);
            len = tlv->len + sizeof(*tlv);
            
            air_ir_code_match_set_status(slave, MS_WK_IDLE);
            res = true;
            break;
            
        default:
            break;
    }
    
    if (!res) {
        len = -1;
    }
    
    return len;
}

// 处理APP接口
bool udp_rf_dev_proc_notify(slave_t* slave,cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    bool res = false;
    
    info = (cln_common_info_t *)&pkt->data[0];
    
    switch (pkt->type) {
        case CLNE_RF_LAMP:
            res = _rf_lamp_proc_notify(slave,info,ret);
            break;
        case CLNE_RF_DOOR_LOCK:
            res = _rf_door_lock_proc_notify(slave, info, ret);
            break;
        case CLNE_RF_COM_DEV:
            
            switch (slave->ext_type) {
                case RF_EXT_TYPE_YT_DOOR_LOCK:
                case RF_EXT_TYPE_DOOR_LOCK:
                   res = _rf_proc_query_his_notify(slave, info, ret);
				   break;
                case RF_EXT_TYPE_DOOR_MAGNET:
				case RF_EXT_TYPE_DOOR_MAGNETV2:
                case RF_EXT_TYPE_HM_MAGENT:
				case RF_EXT_TYPE_YLLOCK:
                    res =  _rf_door_magnet_proc_notify(slave, info, ret);
					break;
                case RF_EXT_TYPE_DHX:
				case RF_EXT_TYPE_DHXZH:
				case RF_EXT_TYPE_DHXCP:
				case RF_EXT_TYPE_DHXML:
				case RF_EXT_TYPE_LHX:
                    res = _rf_dhx_proc_notify(slave, info, ret);
					break;
                case RF_EXT_TYPE_HM_ENV_DETECT:
				case RF_EXT_TYPE_YLWSD:
                	res = _rf_hm_temp_hum_proc_notify(slave, info, ret);
					break;
                case RF_EXT_TYPE_HM_BODY_DETECT:
				case RF_EXT_TYPE_YLTC:
				case RF_EXT_TYPE_WUANS6:
					res = _rf_hm_body_detect_proc_notify(slave, info, ret);
                    break;
				case RF_EXT_TYPE_HEATING_VALVE:
					res = _rf_heating_valve_proc_notify(slave, info, ret);
					break;
				case RF_EXT_TYPE_GAS:
				case RF_EXT_TYPE_QSJC:
				case RF_EXT_TYPE_HMCO:
				case RF_EXT_TYPE_HMYW:
				case RF_EXT_TYPE_HMQJ:
				case RF_EXT_TYPE_YLSOS:
					res = _rf_com_detector_proc_notify(slave, info, ret);
					break;
				case RF_EXT_TYPE_YLLIGHT:
					res = _rf_yllight_proc_notify(slave, info, ret);
					break;
				case RF_EXT_TYPE_KTCZ:
					res = _rf_ktcz_proc_notify(slave, info, ret);
					break;
				case RF_EXT_TYPE_HTLLOCK:
					res = _rf_htllock_proc_notify(slave, info, ret);
                    break;
                case RF_EXT_TYPE_WK_AIR:
				case RF_EXT_TYPE_WK_AIR2:
		            if(info->action == ACT_RF_COM_AIR_IR_CTRL){
		                res = _rf_com_ctrl_proc_notify(slave, info, ret,_rf_air_ir_proc_notify);
		                break;
		            }
                    res = _rf_wk_air_proc_notify(slave, info, ret);
                    break;
				case RS_EXT_TYPE_ZHDJ:
					res = _rf_zhdj_proc_notify(slave, info, ret);
					break;
				case RF_EXT_TYPE_DWKJ:
					res = _rf_dwkj_proc_notify(slave, info, ret);
					break;

				case RF_EXT_TYPE_SCENE_CONTROLLER:
					res = _rf_scene_controller_proc_notify(slave, info, ret);
					break;
				case RS_EXT_TYPE_CDQJMB:
					res = _rf_cdqjmb_proc_notify(slave, info, ret);
					break;
				case RF_EXT_TYPE_JQ:
					res = _rf_jq_proc_notify(slave, info, ret);
					break;

						
                default:
                    break;
            }
            break;
        default:
            *ret = RS_INVALID_PARAM;
            break;
    }
    return res;
}

static bool _udp_rf_com_dev_group_proc_notify(user_t* user,cln_common_info_t *info, RS *ret)
{
    u_int32_t data = cci_u32_data(info);
    u_int8_t group = (data >> 24) & 0xFF;
    u_int8_t type = (data >> 16) & 0xFF;
    u_int8_t group_type = (data >> 8) & 0xFF;
    u_int8_t value = data & 0xFF;
    
    //switch (type) {
	switch (info->action) {
        case ACT_RF_COM_DIRECT_CTRL:
            switch (group_type) {
                case RF_GPT_DOOR_MAGNET:
                    _rf_door_manget_proc_group_notify(user,group,group_type,type,value,ret);
                    break;
                case RF_GPT_DHX_SWITCH:
                    _rf_dhx_proc_group_notify(user,group,group_type,type,value,ret);
                    break;
                case RF_GFT_HM_MAGNET://海曼门磁组
                    break;
				case RF_GFT_DWKJ_LAMP:
					_rf_dwkj_proc_group_notify(user,group,group_type,type,value,ret);
					break;
                    
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    return true;
}

bool udp_rf_dev_group_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    bool res = false;
    
    info = (cln_common_info_t *)&pkt->data[0];
    
    switch (pkt->type) {
        case CLNE_RF_DOOR_LOCK:
            res = _rf_door_lock_proc_group_notify(user,info,ret);
            break;
        case CLNE_RF_COM_DEV:
            res = _udp_rf_com_dev_group_proc_notify(user,info,ret);
            break;
        default:
            *ret = RS_INVALID_PARAM;
            break;
    }
    return res;
}

static void udp_rf_comm_timer_copy(slave_t* slave,cl_slave_t* info)
{
	int i, j, len, n;
	cl_comm_timer_t *ptimer = NULL;

	memcpy((void *)&info->timer_summary, (void *)&slave->timer_summary, sizeof(info->timer_summary));
	memcpy((void *)&info->comm_timer, (void *)&slave->comm_timer, sizeof(info->comm_timer));
	info->comm_timer.timer = NULL;
	info->comm_timer.timer_count = 0;
	info->timer_summary.stat_count = NULL;

	//判断有效个数
	for(i = 0, n = 0; i < slave->comm_timer.timer_count; i++) {
		if (slave->comm_timer.timer[i].valid) {
			n++;
		}
	}
	if (n == 0) {
		return;
	}

	len = n*sizeof(cl_comm_timer_t);
	ptimer = cl_calloc(len, 1);
	if (!ptimer) {
		return;
	}
	
	info->comm_timer.timer = ptimer;
	info->comm_timer.timer_count = n;

	for(i = 0, j = 0; i < slave->comm_timer.timer_count && j < n; i++) {
		if (slave->comm_timer.timer[i].valid) {
			memcpy((void *)&info->comm_timer.timer[j], (void *)&slave->comm_timer.timer[i], sizeof(cl_comm_timer_t));
			j++;
		}
	}
}

// 处理APP接口
bool udp_rf_dev_bulid_slave(slave_t* slave,cl_slave_t* info)
{
	//定时器copy
	udp_rf_comm_timer_copy(slave, info);

	switch (slave->ext_type) {
		case RF_EXT_TYPE_DHX:
		case RF_EXT_TYPE_DHXZH:
		case RF_EXT_TYPE_DHXCP:
		case RF_EXT_TYPE_GAS:
    	case RF_EXT_TYPE_QSJC:
        case RF_EXT_TYPE_HMCO:
 		case RF_EXT_TYPE_HMYW:
		case RF_EXT_TYPE_HMQJ:
		case RF_EXT_TYPE_YLSOS:
		case RF_EXT_TYPE_DHXML:
		case RF_EXT_TYPE_LHX:
			rf_com_detector_on_copy(slave);
			break;
        
        default:
            break;
	}
	
    return true;
}

