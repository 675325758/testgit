#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "lc_scm_ctrl.h"
#include "udp_scm_direct_ctrl_priv.h"

// 5331,支持摇头
const static char* lc_ah_df_ht5331p = "DF-HT5331P";
// 9504r,支持摇头
const static char* lc_ah_df_ht9505r = "DF-HT9505R";
// 5c03p,支持摇头
const static char* lc_ah_df_ht5c05p = "DF-HT5C05P";
// 5325,支持摇头
const static char* lc_ah_df_ht5325p = "DF-HT5325P";
// 5326p,支持摇头
const static char* lc_ah_df_ht5326p = "DF-HT5326P";
// 5250p,支持摇头
const static char* lc_ah_df_ht5250p = "DF-HT5250P";

#define FIXED_FLAG 0xAD

enum{
	LC_SCM_CMD_SET_TT = 0x1, //设置温度和定时，貌似是直接设置
	LC_SCM_CMD_CTRL = 0x2, //通用控制
	LC_SCM_CMD_QUERY_WORK_STAT = 0x3, //查询工作状态
	LC_SCM_CMD_QUERY_CTRL_STAT = 0x4, //查询控制状态
	LC_SCM_CMD_ADD_DEC_CTRL = 0x5, //增减温度和定时
	LC_SCM_CMD_DEV_ATTR = 0x6
};

enum{
	LC_SCM_RETURN_CMD_UNKNOWN = 0x80,
	LC_SCM_RETURN_CMD_SET_TT = 0x81, //设置温度和定时，貌似是直接设置
	LC_SCM_RETURN_CMD_CTRL = 0x82, //通用控制
	LC_SCM_RETURN_CMD_QUERY_WORK_STAT = 0x83, //查询工作状态
	LC_SCM_RETURN_CMD_QUERY_CTRL_STAT = 0x84, //查询控制状态
	LC_SCM_RETURN_CMD_ADD_DEC_CTRL = 0x85, //增减温度和定时
	LC_SCM_RETURN_CMD_DEV_ATTR = 0x86
};
//06/86
#define SUBT_DEV_ATTR 0 //获取属性
//01/81
#define SUBT_SET_TEMP 0x1 //设置温度
#define SUBT_SET_TIMER 0x2 //设置定时器
//02/82
#define SUBT_SET_OFF 		0x0 //关机
#define SUBT_SET_ON 		0x1 //开机
#define SUBT_SET_LOW 		0x2 //抵挡
#define SUBT_SET_HIGH 		0x3 //高档
#define SUBT_SET_SHAKE 		0x4 //摇头
#define SUBT_SET_ECO 		0x5 //ECO
#define SUBT_SET_MODE 		0x6 //模式切换
#define SUBT_SET_ADD_1_STEP 0x7 
#define SUBT_SET_DEC_1_STEP 0x8
//05/85
#define SUBT_SET_ADD_TEMP 0x1 //设置温度增减
#define SUBT_SET_ADD_TIMER 0x2 //设置定时器增减
//03/83
#define SUBT_QUERY_ALL_STAT 0x0
#define SUBT_QUERY_TEMP			0x1 //查询环境温度
#define SUBT_QUERY_HOT1_TEMP 		0x2
#define SUBT_QUERY_HOT2_TEMP 		0x3
#define SUBT_QUERY_REMAIN_TIMER 0x4
#define SUBT_QUERY_POWER 		0x5
#define SUBT_QUERY_SET_TEMP 	0x6
//04/84
#define SUBT_QUERY_ALL_CTRL 0x0 //查询所有控制值

#define SUB_ACTION_ADD 0x1
#define SUB_ACTION_DEC 0x81

#define MAX_SET_TEMP 35
#define MIN_SET_TEMP 5

#define MAX_SET_TIMER 12*60
#define MIN_SET_TIMER 0


static u_int8_t lc_calc_crc(char* src ,int len)
{	
	u_int8_t dest;
	int i;
	
	if(!src||len<=0){
		return 0;
	}

	dest = *src;

	for(i=1;i<len;i++){
		dest ^= src[i];
	}
	
	return dest;
}

//返回封包长度，失败为0
static int mk_simple_scm_cmd(u_int8_t* dest ,u_int8_t cmd,u_int8_t data_sub_type)
{
	lc_ucd_pkt_request_head_t*  hdr = (lc_ucd_pkt_request_head_t*)dest;
	u_int8_t* crc = (u_int8_t*)(hdr+1);

	hdr->len = sizeof(lc_ucd_pkt_request_head_t)+1;
	hdr->flag = FIXED_FLAG;
	hdr->cmd = cmd;
	hdr->data_sub_type = data_sub_type;
	
	*crc = lc_calc_crc((char *)dest,hdr->len-1);

	log_debug("mk_simple_scm_cmd cmd = %u,data_sub_type = %u\n",cmd,data_sub_type);
	mem_dump("mk_simple_scm_cmd ",hdr,hdr->len);
	
	return hdr->len;
}

static int send_simple_scm_cmd(user_t* user,u_int8_t action,u_int8_t cmd,u_int8_t data_sub_type)
{
	char pkt_buff[256] = {0};
	int len;

	len = mk_simple_scm_cmd((u_int8_t *)pkt_buff,cmd,data_sub_type);

	if(action == UCA_SET){
		return scm_send_single_set_pkt(user->uc_session,pkt_buff,len);
	}else if(action == UCA_GET){
		return scm_send_single_get_pkt(user->uc_session,pkt_buff,len);
	}
	
	return -1;
}

static int mk_add_dec_scm_cmd(u_int8_t* dest ,u_int8_t cmd,u_int8_t data_sub_type,bool isadd)
{
	lc_ucd_pkt_request_head_t*  hdr = (lc_ucd_pkt_request_head_t*)dest;
	u_int8_t* action = (u_int8_t*)(hdr+1);
	u_int8_t* crc = action+1;

	hdr->len = sizeof(lc_ucd_pkt_request_head_t)+2;
	hdr->flag = FIXED_FLAG;
	hdr->cmd = cmd;
	hdr->data_sub_type = data_sub_type;
	*action = isadd?SUB_ACTION_ADD:SUB_ACTION_DEC;
	*crc = lc_calc_crc((char *)dest,hdr->len-1);
	
	return hdr->len;
}

static int send_add_dec_scm_cmd(user_t* user,u_int8_t action,u_int8_t cmd,u_int8_t data_sub_type,bool isadd)
{
	char pkt_buff[256] = {0};
	int len;

	len = mk_add_dec_scm_cmd((u_int8_t *)pkt_buff,cmd,data_sub_type,isadd);

	if(action == UCA_SET){
		return scm_send_single_set_pkt(user->uc_session,pkt_buff,len);
	}else if(action == UCA_GET){
		return scm_send_single_get_pkt(user->uc_session,pkt_buff,len);
	}
	
	return -1;
}


static void ah_scm_proc_set_temp_ctrl(smart_air_ctrl_t* ac, u_int8_t is_add, RS* ret)
{
	send_add_dec_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_ADD_DEC_CTRL,SUBT_SET_ADD_TEMP,is_add);
}

static void ah_scm_proc_set_timer_ctrl(smart_air_ctrl_t* ac, u_int8_t is_add, RS* ret)
{
	send_add_dec_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_ADD_DEC_CTRL,SUBT_SET_ADD_TIMER,is_add);
}

static void ah_scm_proc_work_ctrl(smart_air_ctrl_t* ac, int pkt_type, RS* ret)
{
	
	switch(pkt_type){
		case CLNE_AH_POWER:
			if(ac->ah_info.ah_work_stat.is_on){
				send_simple_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_CTRL,SUBT_SET_ON);
			}else{
				send_simple_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_CTRL,SUBT_SET_OFF);
			}
			break;
		case CLNE_AH_SHAKE:
			send_simple_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_CTRL,SUBT_SET_SHAKE);
			break;
		case CLNE_AH_ECO:
			send_simple_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_CTRL,SUBT_SET_ECO);
			break;
		case CLNE_AH_MODE:
			if(ac->ah_info.ah_work_stat.heat_mode == AH_HEAT_MODE_LOW){
				send_simple_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_CTRL,SUBT_SET_LOW);
			}else{
				send_simple_scm_cmd(ac->sac->user,UCA_SET,LC_SCM_CMD_CTRL,SUBT_SET_HIGH);
			}
			break;
		default:
		{
			break;
		}
	}
}


static void ah_scm_proc_refresh_power(smart_air_ctrl_t* ac, int pkt_type, RS* ret)
{
	send_simple_scm_cmd(ac->sac->user,UCA_GET,LC_SCM_CMD_QUERY_WORK_STAT,SUBT_QUERY_POWER);
}

static int ah_test_timer(cl_thread_t *t)
{
    smart_air_ctrl_t* ac;
    RS ret;
    static u_int32_t tt = 0;
    
    ac = (smart_air_ctrl_t*)CL_THREAD_ARG(t);
    if (!ac) {
        return 0;
    }
    ac->common_timer = NULL;
    
    if((((tt++)/8)%2) == 0){
        ah_scm_proc_set_timer_ctrl(ac,true,&ret);
    }else{
        ah_scm_proc_set_timer_ctrl(ac,false,&ret);
    }
    
    CL_THREAD_TIMER_ON(&cl_priv->master, ac->common_timer, ah_test_timer, (void*)ac,
                       ac->common_time*100);
    return 0;
}

static void ah_scm_proc_test(smart_air_ctrl_t* ac,u_int8_t time_out, RS* ret)
{
    CL_THREAD_TIMER_OFF(ac->common_timer);
    
    if (time_out == 0) {
        return;
    }
    
    ac->common_time = time_out;
    CL_THREAD_TIMER_ON(&cl_priv->master, ac->common_timer, ah_test_timer, (void*)ac,
                       0);
}

bool lc_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	smart_air_ctrl_t* ac;
	cln_sa_ah_info_t* sai;
	
  
	sai = (cln_sa_ah_info_t*)pkt->data;
	*ret = RS_OK;
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	
	switch(pkt->type){
		case CLNE_AH_TEMP: /* 5-35*/
			if(sai->action_param > 0){//增加
				if(ac->ah_info.ah_work_stat.set_temp < MAX_SET_TEMP){
					ac->ah_info.ah_work_stat.set_temp++;
				}
			}else{
				if(ac->ah_info.ah_work_stat.set_temp > MIN_SET_TEMP){
					ac->ah_info.ah_work_stat.set_temp--;
				}
			}
			ah_scm_proc_set_temp_ctrl(ac, sai->action_param, ret);
			break;
			
		case CLNE_AH_TIMER: /*0-12*/
			if(sai->action_param > 0){//增加
				if(ac->ah_info.ah_work_stat.remain_minute < MAX_SET_TIMER){
					ac->ah_info.ah_work_stat.remain_minute += 60;
				}
			}else{
				if(ac->ah_info.ah_work_stat.set_temp > MIN_SET_TIMER){
					ac->ah_info.ah_work_stat.remain_minute-=60;
				}
			}
			ah_scm_proc_set_timer_ctrl(ac, sai->action_param, ret);
			break;

		case CLNE_AH_POWER:
			ac->ah_info.ah_work_stat.is_on = sai->action_param;		
			ah_scm_proc_work_ctrl(ac,pkt->type, ret);
			break;

			
		case CLNE_AH_SHAKE:
			ac->ah_info.ah_work_stat.is_shake = sai->action_param;		
			ah_scm_proc_work_ctrl(ac,pkt->type, ret);
			break;
			
		case CLNE_AH_ECO:
			ac->ah_info.ah_work_stat.is_eco = sai->action_param;		
			ah_scm_proc_work_ctrl(ac, pkt->type,ret);
			break;
			
		case CLNE_AH_MODE:
			ac->ah_info.ah_work_stat.heat_mode = sai->action_param;		
			ah_scm_proc_work_ctrl(ac, pkt->type,ret);
			break;
		case CLNE_AH_REFRESH_POWER:
			ah_scm_proc_refresh_power(ac,pkt->type,ret);
			break;
        case CLNE_AH_TEST:
            ah_scm_proc_test(ac,sai->action_param,ret);
            break;
			
		default:
            *ret = RS_ERROR;
			break; 
	}
	return true;
}

static void lc_do_parse_scm_cmd_81(smart_air_ctrl_t* ac,lc_ucd_pkt_response_head_t* hdr,int rel_data_len)
{	
	switch(hdr->data_sub_type){
		case SUBT_SET_TEMP:
			break;
		case SUBT_SET_TIMER:
			break;
		default:
			break;
	}
}

static void lc_do_parse_scm_cmd_82(smart_air_ctrl_t* ac,lc_ucd_pkt_response_head_t* hdr,int rel_data_len)
{
	switch(hdr->data_sub_type){
		case SUBT_SET_OFF:
			break;
		case SUBT_SET_ON:
			break;
		case SUBT_SET_LOW:
			break;
		case SUBT_SET_HIGH:
			break;
		case SUBT_SET_SHAKE:
			break;
		case SUBT_SET_ECO:
			break;
		case SUBT_SET_MODE:
			break;
		case SUBT_SET_ADD_1_STEP:
			break;
		case SUBT_SET_DEC_1_STEP:
			break;
		default:
			break;
	}
}

static void lc_do_parse_scm_cmd_83(smart_air_ctrl_t* ac,lc_ucd_pkt_response_head_t* hdr,int rel_data_len)
{
	fhf_uart_get_param_data_t data;
	fhf_uart_get_power_t power;
	switch(hdr->data_sub_type){
		case SUBT_QUERY_ALL_STAT:
		{
			if(rel_data_len < sizeof(data)){
				return;
			}
			//头部有5个字节，数据字节未对其，特殊处理一下
			memcpy(&data,(void*)(hdr+1),sizeof(data));
			ac->ah_info.ah_work_stat.room_temp = data.env_temp;
			ac->ah_info.ah_work_stat.thermode_1_temp = data.heating_unit1_temp;
			ac->ah_info.ah_work_stat.thermode_2_temp = data.heating_unit2_temp;
			ac->ah_info.ah_work_stat.set_temp = data.set_temp;
			ac->ah_info.ah_work_stat.epower = (u_int32_t)(data.power_low * (1700.0 / 3600.0) + data.power_high * (2500.0 / 3600.0));
			ac->ah_info.ah_work_stat.remain_minute = data.remain_hour*60;
			//设置时间和类型如何获取
			log_debug("lc_do_parse_scm_cmd_83 evn_temp = %u heating_temp1 = %u heating_temp2 = %u set_temp = %u remain_hour = %u pow_low=%u pow_high=%u \n",
			data.env_temp,data.heating_unit1_temp,data.heating_unit2_temp,data.set_temp,data.remain_hour,data.power_low,data.power_high);
		}
			break;
		case SUBT_QUERY_TEMP:
			break;
		case SUBT_QUERY_HOT1_TEMP:
			break;
		case SUBT_QUERY_HOT2_TEMP:
			break;
		case SUBT_QUERY_REMAIN_TIMER:
			break;
		case SUBT_QUERY_POWER:
		{
			if(rel_data_len < sizeof(power)){
				return;
			}

			memcpy(&power,(void*)(hdr+1),sizeof(power));
			ac->ah_info.ah_work_stat.epower = (u_int32_t)(power.power_low * (1700.0 / 3600.0) + power.power_high * (2500.0 / 3600.0));
		}
			break;
		case SUBT_QUERY_SET_TEMP:
			break;
		default:
			break;
	}
}

static void lc_do_parse_scm_cmd_84(smart_air_ctrl_t* ac,lc_ucd_pkt_response_head_t* hdr,int rel_data_len)
{
	fhf_uart_get_state_data_t* data;
	if(hdr->data_sub_type != SUBT_QUERY_ALL_CTRL || rel_data_len < sizeof(fhf_uart_get_state_data_t)){
		return;
	}

	data = (fhf_uart_get_state_data_t*)(hdr+1);

	log_debug("lc_do_parse_scm_cmd_84 stat = %02x falut_stat = %02x\n",data->state,data->fault_state);
	
	ac->ah_info.ah_work_stat.is_on = data->state & BIT(0);
	ac->ah_info.ah_work_stat.heat_mode= ((data->state & BIT(1))==0)?AH_HEAT_MODE_LOW:AH_HEAT_MODE_HIGH;
	ac->ah_info.ah_work_stat.is_shake = data->state & BIT(2);
	ac->ah_info.ah_work_stat.is_eco = data->state & BIT(3);
	//is_heating false 表示正在工作
	ac->ah_info.ah_work_stat.is_heating = !(data->state & BIT(4));
	ac->ah_info.ah_work_stat.is_topple_protect = data->fault_state & BIT(0);
	ac->ah_info.ah_work_stat.is_temp_high_protect= data->fault_state & BIT(1);
	ac->ah_info.ah_work_stat.is_furnace_high_protect= data->fault_state & BIT(2);
	ac->ah_info.ah_work_stat.is_furnace_error= data->fault_state & BIT(3);
	ac->work_stat_update_ok = true;
	
	
}

static void lc_do_parse_scm_cmd_85(smart_air_ctrl_t* ac,lc_ucd_pkt_response_head_t* hdr,int rel_data_len)
{
	switch(hdr->data_sub_type){
		case SUBT_SET_ADD_TEMP:
			break;
		case SUBT_SET_ADD_TIMER:
			break;
		default:
			break;
	}
}

static void lc_do_parse_scm_cmd_86(smart_air_ctrl_t* ac,lc_ucd_pkt_response_head_t* hdr,int rel_data_len)
{
	if(hdr->data_sub_type == SUBT_DEV_ATTR){
		
	}
}


static void lc_do_parse_scm_command(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	lc_ucd_pkt_response_head_t* hdr;

	//检查数据和校验和
	if(cmd_len < sizeof(*hdr)+1){
		log_err(false, "lc_do_parse_scm_command length error %u\n",cmd_len);
		return;
	}

	hdr = (lc_ucd_pkt_response_head_t*)pcmd;
	log_debug("lc_do_parse_scm_command dump info cmd=%02x sub_cmd=%02x errno = %u\n",hdr->cmd,hdr->data_sub_type,hdr->err_no);
	if(hdr->err_no != ERR_NONE){
		log_err(false, "lc_do_parse_scm_command cmd=%02x sub_cmd=%02x errno = %u\n",hdr->cmd,hdr->data_sub_type,hdr->err_no);
		return;
	}

	cmd_len-=(sizeof(*hdr)+1);
	
	switch(hdr->cmd){
		case LC_SCM_RETURN_CMD_SET_TT:
			lc_do_parse_scm_cmd_81(ac,hdr,cmd_len);
			break;
		case LC_SCM_RETURN_CMD_CTRL:
			lc_do_parse_scm_cmd_82(ac,hdr,cmd_len);
			break;
		case LC_SCM_RETURN_CMD_QUERY_WORK_STAT:
			lc_do_parse_scm_cmd_83(ac,hdr,cmd_len);
			break;
		case LC_SCM_RETURN_CMD_QUERY_CTRL_STAT:
			lc_do_parse_scm_cmd_84(ac,hdr,cmd_len);
			break;
		case LC_SCM_RETURN_CMD_ADD_DEC_CTRL:
			lc_do_parse_scm_cmd_85(ac,hdr,cmd_len);
			break;
		case LC_SCM_RETURN_CMD_DEV_ATTR:
			lc_do_parse_scm_cmd_86(ac,hdr,cmd_len);
			break;
		default:
			break;
	}

	
}

static bool lc_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
			lc_do_parse_scm_command(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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
static bool tt_update_ir_notify(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	user_t *user = air_ctrl->sac->user;
	u_int8_t *buf = (u_int8_t*)(obj+1);
	
	air_ctrl->ir_len = obj->param_len;
	memcpy(air_ctrl->ir_buf, buf, air_ctrl->ir_len<=sizeof(air_ctrl->ir_buf)?air_ctrl->ir_len:sizeof(air_ctrl->ir_buf));
	event_push(user->callback, SAE_TT_IR, user->handle, user->callback_handle);
	return true;
}

static bool tt_update_ir_sound_notify(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	user_t *user = air_ctrl->sac->user;
	sound_link_data_t *sound;

	sound = cl_calloc(sizeof(*sound)+obj->param_len+128, 1);
	if(sound){
		sound->len = obj->param_len;
		memcpy(sound->data, (obj+1), obj->param_len);
		stlc_list_add_tail(&sound->link, &air_ctrl->pt_sound);
		event_push(user->callback, SAE_TT_SOUND, user->handle, user->callback_handle);
	}
	
	return true;
}

bool lc_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = lc_scm_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMD:
		//	ret = lc_scm_update_tlv_data(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_CMDRET:
			break;
		case UCAT_IA_TT_IR_NOTIFY:
			ret = tt_update_ir_notify(air_ctrl,action,obj);
			break;
		case UCAT_IA_TT_IR_SOUND_NOTIFY:
			ret = tt_update_ir_sound_notify(air_ctrl,action,obj);
			break;

		default:
			break;
	}
	
	return ret;
}

int lc_scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	
	int event = 0;
	
	switch(obj->attr){
		case UCAT_FHF_WORK:
			if(error == ERR_NONE)
				event = AHE_CTRL_OK;
			else
				event = AHE_CTRL_FAIL;			
			break;
			
		case UCAT_FHF_SET_TEMP:
			if(error == ERR_NONE)
				event = AHE_CTRL_TEMP_OK;
			else
				event = AHE_CTRL_TEMP_FAIL;
			break;
			
		case UCAT_FHF_SET_TIMER:
			if(error == ERR_NONE)
				event = AHE_CTRL_TIMER_OK;
			else
				event = AHE_CTRL_TIMER_FAIL;
			break;
			
		default:
			break;
	}
	
	return event;
}

int lc_get_ext_type_by_ident(u_int8_t* ident,int ident_len)
{
	int ret = -1;
	if(memcmp(ident,lc_ah_df_ht5331p,strlen(lc_ah_df_ht5331p)) == 0){
		return ETYPE_IJ_812_AH_5331P;
	}

	if(memcmp(ident,lc_ah_df_ht9505r,strlen(lc_ah_df_ht9505r)) == 0){
		return ETYPE_IJ_812_AH_9505R;
	}

	if(memcmp(ident,lc_ah_df_ht5c05p,strlen(lc_ah_df_ht5c05p)) == 0){
		return ETYPE_IJ_812_AH_HT5C05P;
	}

	if(memcmp(ident,lc_ah_df_ht5325p,strlen(lc_ah_df_ht5325p)) == 0){
		return ETYPE_IJ_812_HT_5325P;
	}

	if(memcmp(ident,lc_ah_df_ht5326p,strlen(lc_ah_df_ht5326p)) == 0){
		return ETYPE_IJ_812_HT_5326P;
	}

	if(memcmp(ident,lc_ah_df_ht5250p,strlen(lc_ah_df_ht5250p)) == 0){
		return ETYPE_IJ_812_HT_5250P;
	}
	
	return ret;
}


