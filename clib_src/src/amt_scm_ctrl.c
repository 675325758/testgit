#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "amt_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"

static u_int8_t amt_calc_crc(char* src ,int len)
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

static void _amt_send_set_cmd(smart_air_ctrl_t* ac,amt_fan_priv_data_t* fan,u_int8_t data_type,void* data,int len)
{
	u_int8_t buf[256] = {0};
	amt_pkt_header_t * hdr = (amt_pkt_header_t*)buf;
	amt_pkt_tail_t* tail;
	void* content = (void*)(hdr+1);
	
	if(!data || len <= 0){
		return;
	}	
	
	
	hdr->fix_s_code = AMT_FIX_START_CODE;
	hdr->product_type = fan->product_type;
	hdr->data_type = data_type;
	//只去头尾
	hdr->len = (u_int8_t)((len + sizeof(*hdr) - sizeof(hdr->fix_s_code) + sizeof(u_int8_t))&0xFF);
	
	memcpy(content,data,len);
	tail = (amt_pkt_tail_t *)((char *)content+len);
	tail->fix_end_code1 = tail->fix_end_code2 = AMT_FIX_END_CODE;
	
	tail->crc = amt_calc_crc((char*)&hdr->len,len+sizeof(*hdr)-sizeof(hdr->fix_s_code));
	dump_data(buf,len+sizeof(*hdr)+sizeof(*tail));
	scm_send_single_set_pkt(ac->sac->user->uc_session,buf,len+sizeof(*hdr)+sizeof(*tail));
}

static void _amt_do_a3_cmd(smart_air_ctrl_t* ac,amt_fan_priv_data_t* fan, RS* ret)
{
	amt_53h_info_t info = {0};
	
	memcpy(&info,&fan->cmd_53_info,sizeof(info));
	info.is_on = fan->onoff;
	if(info.is_on){
		if(fan->screen_light){
			info.is_screen_display = 0x3;	
		}else{
			info.is_screen_display = 0x0;
		}
		fan->is_timer_on_valid = false;
	}else{
		info.is_screen_display = 0x0;
		fan->is_timer_off_valid = false;
	}

	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_STATUS,&info,sizeof(info));
}

static void _amt_do_switch_mode_cmd(smart_air_ctrl_t* ac,amt_fan_priv_data_t* fan, RS* ret)
{
	u_int8_t buf[64] = {0};
	amt_54h_info_t* info = (amt_54h_info_t*)buf;
	
	memcpy(info,&fan->mode_info.cmd_54_info,sizeof(*info));
	info->cur_mode = fan->cur_mode;
	//自定义的，是否要加数据
	switch(fan->cur_mode){
		//切换这两个模式，档位为0
		case AMT_FAN_UD_NATURAL:
		case AMT_FAN_SMART:
			info->cur_gear = 0;
			break;
		default:
			info->cur_gear = fan->cur_gear;
			break;
	}

	switch(info->cur_mode){
		case AMT_FAN_UD_NATURAL:
		case AMT_FAN_STAND:
			fan->cur_gear = 16;
			break;
		case AMT_FAN_SLEEP:
		case AMT_FAN_SMART:
		case AMT_FAN_NATURAL:
			fan->cur_gear = 0x2;
			break;

		default:
			break;
	}
	
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_GEAR_INFO,info,sizeof(*info));
}

static void _amt_do_gear_cmd(smart_air_ctrl_t* ac,amt_fan_priv_data_t* fan, RS* ret)
{
	u_int8_t buf[64] = {0};
	amt_54h_info_t* info = (amt_54h_info_t*)buf;
	
	memcpy(info,&fan->mode_info.cmd_54_info,sizeof(*info));
	info->cur_mode = fan->cur_mode;
	info->cur_gear = fan->cur_gear;
	//自定义的，是否要加数据
	switch(info->cur_mode){
		case AMT_FAN_UD_NATURAL:
		case AMT_FAN_SMART:
			return;
			break;
		case AMT_FAN_STAND:
			if(info->cur_gear == 0 || info->cur_gear > AMT_MAX_U_GEAR){
				info->cur_gear = 0x10;
			}
			break;
		case AMT_FAN_SLEEP:
		case AMT_FAN_NATURAL:
			if(info->cur_gear == 0 || info->cur_gear > 3){
				info->cur_gear = 0x2;
			}
			break;

		default:
			return;
			break;
	}
	
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_GEAR_INFO,info,sizeof(*info));
}

static void _amt_do_config_smart_mode_cmd(smart_air_ctrl_t* ac,cln_common_info_t* cl_info,amt_fan_priv_data_t* fan, RS* ret)
{
	u_int8_t buf[64] = {0};
	amt_54h_info_t* info = (amt_54h_info_t*)buf;
	u_int32_t data = cci_u32_data(cl_info);
	
	memcpy(info,&fan->mode_info.cmd_54_info,sizeof(*info));
	info->cur_mode = AMT_FAN_SMART;
	info->cur_gear = (data >> 16)&0xFF;
	buf[1] = (data >> 24)&0xFF;
	
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_GEAR_INFO,info,sizeof(*info));
}

static void _amt_do_umode_cmd(smart_air_ctrl_t* ac,cln_common_info_t* cl_info,amt_fan_priv_data_t* fan, RS* ret)
{
	u_int8_t buf[64] = {0};
	amt_54h_info_t* info = (amt_54h_info_t*)buf;
	amt_user_define_mode* um;
	u_int8_t interval;
	u_int8_t count,i;
	u_int8_t* p_gear;
	
	memcpy(info,&fan->mode_info.cmd_54_info,sizeof(*info));
	info->cur_mode = AMT_FAN_UD_NATURAL;
	//设置参数，档位置为0xFF
	info->cur_gear = 0xFF;
	
	interval = cl_info->u.u8_data[0];
	count = cl_info->u.u8_data[1];
	p_gear = &cl_info->u.u8_data[2];
	
	if(interval == 0 || interval > 60){
		interval = 5;
	}
	
	um = (amt_user_define_mode*)(info+1);
	for(i = 0 ; (i < count) && (i < MAX_USER_DEFINE_MODE_POINT-1); i++,um++){
		um->group = 0x1;
		um->index = i+1;
		um->total_mode = 0x5;
		um->config_mode = AMT_FAN_UD_NATURAL;
		um->gear = p_gear[i];
		um->time_type = AMT_TIME_TYPE_SECOND;
		um->time = interval;
	}
	
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_GEAR_INFO,info,sizeof(*info)+(MAX_USER_DEFINE_MODE_POINT-1)*sizeof(*um));
}

static void _amt_do_a5_cmd(smart_air_ctrl_t* ac,cln_common_info_t* cl_info,amt_fan_priv_data_t* fan, RS* ret)
{
	char buf[64]= {0};
	amt_machine_timer_t* mt = (amt_machine_timer_t*)buf;
	u_int32_t data;
	u_int8_t onoff;
	u_int16_t time;
	
	data = cci_u32_data(cl_info);
	onoff = !!(data >> 16);
	time = data &0xFFFF;

	log_debug("_amt_do_a5_cmd onoff = %u time=%u\n",onoff,time);
	
	if(onoff){//预约开机
		mt->timer_mode = AMT_TIMER_MODE_ON;
		mt->timer_type = AMT_TIMER_DT_ON_SETTING;
		fan->is_timer_off_valid = false;
		fan->is_timer_on_valid = true;
	}else{
		mt->timer_mode = AMT_TIMER_MODE_OFF;
		mt->timer_type = AMT_TIMER_DT_OFF_SETTING;
		fan->is_timer_off_valid = true;
		fan->is_timer_on_valid = false;
	}
	
	if(time == 0){
		mt->timer_mode = AMT_TIMER_MODE_NO;
		fan->is_timer_off_valid  = fan->is_timer_on_valid = false;
	}else{
		mt->item[0].time_type = AMT_TIME_TYPE_DAY;
		mt->item[1].time_type = AMT_TIME_TYPE_HOUR;
		mt->item[1].time = time / 60;
		mt->item[2].time_type = AMT_TIME_TYPE_MIN;
		mt->item[2].time = time % 60;
		mt->item[3].time_type = AMT_TIME_TYPE_SECOND;
	}
	mt->timer_index = 0x1;
	fan->dev_off_remain_time = time*60;
	//剩余时间信息??
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_TIMER,mt,sizeof(*mt));
	
}

static void _amt_do_a6_cmd(smart_air_ctrl_t* ac,amt_fan_priv_data_t* fan, RS* ret)
{
	u_int8_t buf[16] = {0};
	amt_shake_type_t* st = (amt_shake_type_t*)buf;
	amt_shake_angle_t* sa = (amt_shake_angle_t*)(st+1);
	
	st->shake_type = fan->shake_info.shake_type.shake_type;
	sa->max_angle = fan->shake_info.shake_angle.max_angle;
	if(fan->is_shake){
		if(sa->max_angle != 0)
			sa->cur_angle = sa->max_angle;
		else
			sa->cur_angle = 0x9;
	}
	
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_SHAKE,buf,sizeof(*st)+sizeof(*sa));
}

static void _amt_do_a7_cmd(smart_air_ctrl_t* ac,amt_fan_priv_data_t* fan, RS* ret)
{
	u_int8_t other_value = 0;
	
	if(fan->is_anion_on){
		other_value |= BIT(AMT_A7_DB_ANION);
	}	
	if(fan->is_plasma_on){
		other_value |= BIT(AMT_A7_DB_PLASMA);
	}	
	_amt_send_set_cmd(ac,fan,AMT_CTRL_CMD_MASK|AMT_DATA_TYPE_OTHER_INFO,&other_value,sizeof(other_value));
}

bool amt_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	amt_fan_priv_data_t * fan;
	u_int32_t data;


	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	fan = ac->com_udp_dev_info.device_info;
	
	switch(info->action){
	case ACT_AMT_CTRL_ONOFF:
		fan->onoff = !!cci_u8_data(info);
		if(fan->onoff){
			fan->screen_light = 0x1;
		}else{
			fan->screen_light = 0x0;
		}
		_amt_do_a3_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_MODE:
		fan->cur_mode = cci_u8_data(info);
		_amt_do_switch_mode_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_GEAR:
		data = cci_u32_data(info);
		fan->cur_mode = (data >> 24)&0xFF;
		fan->cur_gear = (data >> 16)&0xFF;
		_amt_do_gear_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_SHAKE:
		fan->is_shake = !!cci_u8_data(info);
		_amt_do_a6_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_ANION:
		fan->is_anion_on = !!cci_u8_data(info);
		_amt_do_a7_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_PLASMA:
		fan->is_plasma_on = !!cci_u8_data(info);
		_amt_do_a7_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_SCREEN_LIGHT:
		fan->screen_light = !!cci_u8_data(info);
		_amt_do_a3_cmd(ac,fan,ret);
		break;
	case ACT_AMT_CTRL_U_DEF_MODE:
		_amt_do_umode_cmd(ac,info,fan,ret);
		break;
	case ACT_AMT_CTRL_S_TIMER:
		_amt_do_a5_cmd(ac,info,fan,ret);
		break;
	case ACT_AMT_CTRL_SMART_PARAM:
		_amt_do_config_smart_mode_cmd(ac,info,fan,ret);
		break;
	default:
		res = false;
		break;
	}
	return res;
}

static bool is_packet_valid(amt_pkt_header_t* hdr,u_int16_t pkt_len)
{
	amt_pkt_tail_t * tail;
	u_int8_t crc,dsrc;
	char* pstart;

	dump_data((u_int8_t*)hdr,pkt_len);
	if(!hdr || pkt_len <= sizeof(amt_pkt_header_t)+3){
		log_err(false,"%s err packet pkt_len %u\n",__FUNCTION__,pkt_len);
		return false;
	}
	
	if(hdr->fix_s_code != AMT_FIX_START_CODE){
		log_err(false,"%s err packet start code %X\n",__FUNCTION__,hdr->fix_s_code);
		return false;
	}

	dsrc = hdr->data_type & AMT_DATA_SRC_MASK;
	if( dsrc != DEV_NO_CHANGE_TO_PHONE && dsrc != DEV_HAS_CHANGE_TO_PHONE && dsrc != DEV_PHONE_CTRL_RESPONSE){
		log_err(false,"%s err packet data type %X\n",__FUNCTION__,hdr->data_type);
		return false;
	}

	tail = (amt_pkt_tail_t*)(((char*)(hdr))+(pkt_len-3));
	if(tail->fix_end_code1 != AMT_FIX_END_CODE || tail->fix_end_code2 != AMT_FIX_END_CODE){
		log_err(false,"%s err packet end code code1 = %X code2 = %X\n",__FUNCTION__,tail->fix_end_code1,tail->fix_end_code2);
		return false;
	}
	
	pstart = ((char*)hdr)+sizeof(hdr->fix_s_code);
	crc = amt_calc_crc(pstart,pkt_len - sizeof(*tail)-sizeof(hdr->fix_s_code));
	if(crc !=  tail->crc){
		log_err(false,"%s err packet crc code crc = %x want = %x\n",__FUNCTION__,crc,tail->crc);
		return false;
	}
	
	return true;
}

static void amt_parse_env_info(amt_fan_priv_data_t* priv_date,u_int8_t* data,u_int16_t len)
{
	amt_env_item_t * item;
	int num = len /sizeof(*item);
	
	if(len == 0 || (len % sizeof(*item) != 0)){
		log_err(false,"%s err env info len = %u\n",__FUNCTION__,len);
		return;
	}
	
	item = (amt_env_item_t*)data;
	for(;num > 0 ; num--,item++){
		switch(item->env_type){
			case AMT_ENV_TEMP:
				priv_date->cur_temp = (int8_t)item->env_val;
				break;
			default:
				break;
		}
	}

}

static void amt_parse_elec_info(amt_fan_priv_data_t* priv_date,u_int8_t* data,u_int16_t len)
{
	amt_elec_item_t * item;
	amt_elec_item_t tmp;
	int num = len /sizeof(*item);
	
	if(len == 0 || (len % sizeof(*item) != 0)){
		log_err(false,"%s err env info len = %u\n",__FUNCTION__,len);
		return;
	}
	
	item = (amt_elec_item_t*)data;
	for(;num > 0 ; num--,item++){
		memcpy(&tmp,item,sizeof(*item));
		switch(tmp.d_type){
			case AMT_A10_CUR_POWER:
				priv_date->cur_power = ntohs(tmp.d_value);
				log_debug("cur power %u \n",priv_date->cur_power);
				break;
			default:
				break;
		}
	}

}

static void amt_parse_timer_info(amt_fan_priv_data_t* priv_date,u_int8_t* data,u_int16_t len)
{
	amt_machine_timer_t * mt = (amt_machine_timer_t*)data;
	int num= len / sizeof(*mt);	

	if((len % sizeof(*mt) !=0) || len == 0){
		log_err(false,"%s err env info len = %u\n",__FUNCTION__,len);
		return;
	}
	
	for(;num > 0 ; num--,mt++){
		switch(mt->timer_type){
			case AMT_TIMER_DT_REMAIN:
				if(mt->timer_mode == AMT_TIMER_MODE_NO){
					priv_date->is_timer_off_valid = priv_date->is_timer_on_valid = true;
				}else if(mt->timer_mode == AMT_TIMER_MODE_OFF){
					priv_date->is_timer_off_valid = true;
					priv_date->is_timer_on_valid = false;
					priv_date->dev_off_remain_time = mt->item[1].time*3600+
											mt->item[2].time*60+
											mt->item[3].time;
				}else if(mt->timer_mode == AMT_TIMER_MODE_ON){
					priv_date->is_timer_off_valid = false;
					priv_date->is_timer_on_valid = true;
					priv_date->dev_on_remain_time = mt->item[1].time*3600+
											mt->item[2].time*60+
											mt->item[3].time;
				}
				break;
			case AMT_TIMER_DT_OFF_SETTING:
				break;
			case AMT_TIMER_DT_ON_SETTING:
				break;
			default:
				break;
		}
	}
		

}

static void amt_parse_mode_info(amt_fan_priv_data_t* priv_date,u_int8_t* data,u_int16_t len)
{
	amt_54h_info_t* ai = (amt_54h_info_t*)data;
	amt_user_define_mode* um;
	unsigned int i;
	
	if(len < sizeof(*ai)){
		log_err(false,"%s err mode info len = %u\n",__FUNCTION__,len);
		return ;
	}
	
	dump_data(data,len);
	memcpy(&priv_date->mode_info.cmd_54_info,ai,sizeof(*ai));
	if(ai->cur_mode == AMT_FAN_UD_NATURAL){
		len-=sizeof(*ai);
		if(len>0 && len / sizeof(amt_user_define_mode) <= MAX_USER_DEFINE_MODE_POINT){
			memset(&priv_date->mode_info.cur_user_define_mode,0,sizeof(priv_date->mode_info.cur_user_define_mode));
			um = (amt_user_define_mode*)(ai+1);
			for(i=0;i<len/sizeof(amt_user_define_mode);i++,um++){
				priv_date->mode_info.cur_user_define_mode[i] = um->gear;
			}
		}
	}	

}

static void amt_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	amt_pkt_header_t* hdr;
	u_int16_t real_data_len;
	u_int8_t data_type;
	amt_fan_priv_data_t* priv;
	u_int8_t other_info;

	hdr = (amt_pkt_header_t*)pcmd;	
	if(!is_packet_valid(hdr,cmd_len)||!ac->com_udp_dev_info.device_info){
		return;
	}

	priv = ac->com_udp_dev_info.device_info;
	
	real_data_len = cmd_len-sizeof(*hdr)-sizeof(amt_pkt_tail_t);
	data_type = hdr->data_type & AMT_DATA_TYPE_MASK;
	

	if(real_data_len == 0){
		return;
	}
	
	if(priv->product_type == 0){
		priv->product_type = hdr->product_type;
	}
	
	switch(data_type){
		case AMT_DATA_TYPE_PRODUCT_CATEGORY:
			break;
		case AMT_DATA_TYPE_PRODUCT_MODEL:
			{
				memset(priv->product_category,0,sizeof(priv->product_category));
				if(real_data_len < sizeof(priv->product_category)){
					memcpy(priv->product_category,(hdr+1),real_data_len);
				}
			}
			break;
		case AMT_DATA_TYPE_STATUS:
			{
				amt_53h_info_t* src = (amt_53h_info_t*)(hdr+1);
				memcpy(&priv->cmd_53_info,src,sizeof(*src));
			}
			break;
		case AMT_DATA_TYPE_GEAR_INFO:
			amt_parse_mode_info(priv,(u_int8_t*)(hdr+1),real_data_len);
			break;
		case AMT_DATA_TYPE_TIMER:
			amt_parse_timer_info(priv,(u_int8_t*)(hdr+1),real_data_len);
			break;
		case AMT_DATA_TYPE_SHAKE:
			{
				amt_shake_type_t*st = (amt_shake_type_t*)(hdr+1);
				amt_shake_angle_t*sangle = (amt_shake_angle_t*)(st+1);
				if(real_data_len < sizeof(*st)+sizeof(*sangle)){
					return;
				}
				memcpy(&priv->shake_info.shake_type,st,sizeof(*st));
				memcpy(&priv->shake_info.shake_angle,sangle,sizeof(*sangle));
			}
			break;
		case AMT_DATA_TYPE_OTHER_INFO:
			{
				if(real_data_len != sizeof(other_info)){
					return;
				}
				other_info = *((u_int8_t*)(hdr+1));
				priv->is_anion_on = !!(other_info & BIT(AMT_A7_DB_ANION));
				priv->is_plasma_on =   !!(other_info & BIT(AMT_A7_DB_PLASMA));
				
			}
			break;
		case AMT_DATA_TYPE_ENV:
			amt_parse_env_info(priv,(u_int8_t*)(hdr+1),real_data_len);
			break;
		case AMT_DATA_TYPE_DEV_RUN_INFO:
			break;
		case AMT_DATA_TYPE_ELEC_STAT:
			amt_parse_elec_info(priv,(u_int8_t*)(hdr+1),real_data_len);
			break;
		default:
			break;
	}


}

static void amt_analy_data(smart_air_ctrl_t*ac )
{
	amt_fan_priv_data_t* priv;

	if( !ac || !ac->com_udp_dev_info.device_info){
		return ;
	}
	
	priv = ac->com_udp_dev_info.device_info;
	priv->cur_gear = priv->mode_info.cmd_54_info.cur_gear;
	priv->cur_mode = priv->mode_info.cmd_54_info.cur_mode;
	priv->onoff = !!priv->cmd_53_info.is_on;
	priv->screen_light = !!priv->cmd_53_info.is_screen_display;
	priv->is_shake = !! priv->shake_info.shake_angle.cur_angle;
	
}

static bool amt_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
			amt_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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
	
	amt_analy_data(air_ctrl);
	
	return true;
}

bool amt_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = amt_scm_update_tlv_data(air_ctrl,action,obj);
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

bool amt_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	amt_fan_priv_data_t * src = NULL;
	cl_amt_fan_device_t * dest;
	int i;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	src = ac->com_udp_dev_info.device_info;
	dest = cl_calloc(sizeof(*dest) , 0x1);
	if(!dest){
		return false;
	}
	
	dest->cur_gear = src->cur_gear;
	dest->cur_mode = src->cur_mode;
	dest->onoff = !!src->onoff;
	dest->screen_light = !!src->screen_light;
	dest->is_shake = !!src->is_shake;
	dest->cur_temp = src->cur_temp;
	dest->is_timer_off_valid = src->is_timer_off_valid;
	dest->is_timer_on_valid = src->is_timer_on_valid;
	dest->dev_off_remain_time = src->dev_off_remain_time;
	dest->dev_on_remain_time = src->dev_on_remain_time;
	dest->is_anion_on = !!src->is_anion_on;
	dest->is_plasma_on = !!src->is_plasma_on;
	dest->cur_power = (u_int8_t)src->cur_power;
	memcpy(&dest->cur_user_define_mode,&src->mode_info.cur_user_define_mode,sizeof(dest->cur_user_define_mode));

	if(dest->cur_mode == AMT_FAN_STAND){
		if(dest->cur_gear == 0  || dest->cur_gear > AMT_MAX_U_GEAR){
			dest->cur_gear = 16;
		}
	}else if(dest->cur_mode == AMT_FAN_NATURAL || dest->cur_mode == AMT_FAN_SLEEP||dest->cur_mode == AMT_FAN_SMART){
		if(dest->cur_gear == 0 || dest->cur_gear > 3){
			dest->cur_gear = 0x2;
		}
	}else if( dest->cur_mode == AMT_FAN_UD_NATURAL){
		dest->cur_gear = 0;
	}

	for(i=0;i<MAX_USER_DEFINE_MODE_POINT;i++){
		if(dest->cur_user_define_mode[i] == 0)
			dest->cur_user_define_mode[i] = 16;
	}
	
	
	di->device_info = dest;

	return true;
}

int amt_get_ext_type_by_tlv(uc_tlv_t* tlv)
{
	int ext_type = 0;
	tlv_dev_ident_t* ti;

	if(tlv->type == UCT_SCM_FACTORY_DATA && tlv->len == sizeof(tlv_dev_ident_t)){
		ti = (tlv_dev_ident_t*)(tlv+1);
		switch(ti->ident){
			case AMT_PRODUCT_TYPE_FAN:
				ext_type = ETYPE_IJ_AMT_FAN;
				break;
			default:
				break;
		}
	}
	
	return ext_type;
}

bool amt_send_set_timer_ctrl_cmd(smart_air_ctrl_t* ac)
{
	char buf[256] = {0};
	
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t on,off;
    amt_pkt_header_t * hdr = (amt_pkt_header_t*)(uo+1);
    u_int8_t * value = (u_int8_t*)(hdr+1);
    amt_pkt_tail_t* tail = (amt_pkt_tail_t*)(value+1);
    int dest_pkt_len = sizeof(ucp_obj_t)+sizeof(*value)+sizeof(*hdr)+sizeof(*tail);
    
    hdr->fix_s_code = AMT_FIX_START_CODE;
    hdr->data_type = AMT_DATA_TYPE_STATUS|DEV_PHONE_CTRL_RESPONSE;
    hdr->len = 0x5;
    tail->fix_end_code1 = tail->fix_end_code2 = AMT_FIX_END_CODE;
	
	switch(ac->sac->user->ext_type){
		case ETYPE_IJ_AMT_FAN:
            hdr->product_type = AMT_PRODUCT_TYPE_FAN;
			on = 0xC9;
			off = 0xC8;
			break;
		default:
			return false;
	}
    
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TT, UCAT_IA_TT_ON_CMD,(u_int16_t)(dest_pkt_len-sizeof(*uo)));
	*value = on;
    tail->crc = amt_calc_crc((char*)&hdr->len,sizeof(*hdr)-sizeof(hdr->fix_s_code)+sizeof(*value));
	sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf,dest_pkt_len);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TT, UCAT_IA_TT_OFF_CMD,(u_int16_t)(dest_pkt_len-sizeof(*uo)));
	*value = off;
    tail->crc = amt_calc_crc((char*)&hdr->len,sizeof(*hdr)-sizeof(hdr->fix_s_code)+sizeof(*value));
	sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf,dest_pkt_len);
	
	return true;	
}


