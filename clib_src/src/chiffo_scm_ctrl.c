#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "chiffo_scm_ctrl.h"


#if 1
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

#define BCD_2_NUM(bcd) ((bcd&0xF)+((bcd & 0xF0)>>4)*10)

static u_int16_t _chiffo_calc_crc(unsigned char* src ,int len)
{	
	u_int16_t  crc;
	u_int8_t da;
	static u_int16_t  crc_tab[16]={0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	                                      0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,}; //CRC的0x11021余式表    
	                    
	crc=0x0000;  
	while(len--!=0) 
	{
		da=((u_int8_t)(crc/256))/16;  //暂存CRC的高4位   
		crc<<=4;  //CRC左移4位，相当于取CRC的低12位）   
		crc^=crc_tab[da^(*src/16)];  //CRC的高4位和本字节的前半字节相加后查表计算CRC，然后加上上一次CRC的余数    
		da=((u_int8_t)(crc/256))/16;  //暂存CRC的高4位   
		crc<<=4;  //CRC左移4位，相当于取CRC的低12位）     
		crc^=crc_tab[da^(*src&0x0f)];  //CRC的高4位和本字节的后半字节相加后查表计算CRC，然后加上上一次CRC的余数   
		src++; 
	}

	return crc;

}

static void _chiffo_send_set_ctrl_cmd(smart_air_ctrl_t* ac,u_int8_t dtype,void* data,int len)
{
	u_int8_t buf[256] = {0};
	ucp_chiffo_pkt_hdr_t * hdr = (ucp_chiffo_pkt_hdr_t*)buf;
	ucp_chiffo_pkt_tail_t* tail;
	u_int8_t* content = (u_int8_t*)(hdr+1);
	u_int16_t crc;
    u_int16_t t_len;
	
	if(!data || len <= 0){
		return;
	}	
	
	hdr->s_code1 = CH_STM_START_CODE;
	hdr->s_code2 = CH_STM_START_CODE;
	hdr->d_type = dtype;
    
	
	memcpy(content,data,len);
    
    t_len = (u_int16_t)(len + sizeof(*tail) + 1);
    
    hdr->len_low = t_len & 0xFF;
    hdr->len_high = (t_len >> 8) & 0xFF;

	tail = (ucp_chiffo_pkt_tail_t*)(content + len);
	tail->e_code1 = CH_STM_END_CODE;
	tail->e_code2 = CH_STM_END_CODE;
	
	//TODO:确认CRC 计算
	crc = _chiffo_calc_crc((char*)buf, len + sizeof(*hdr));
	tail->crc_high = (crc >> 8) & 0xFF;
	tail->crc_low = crc & 0xFF;

	scm_send_single_set_pkt(ac->sac->user->uc_session,buf,len+sizeof(*hdr)+sizeof(*tail));
}

static void _chiffo_send_short_ctrl_cmd(smart_air_ctrl_t* ac,u_int8_t cmd)
{
	ucp_chiffo_ctrl_key_t ck = {0};

	ck.cmd_bit_len = 0x6;
	ck.cmd_low = cmd;
	
	_chiffo_send_set_ctrl_cmd(ac,CH_DTYPE_PHONE_2_STM,&ck,sizeof(ck));
}

static void _chiffo_send_long_ctrl_cmd(smart_air_ctrl_t* ac,u_int8_t low,u_int8_t high)
{
	ucp_chiffo_ctrl_key_t ck = {0};

	ck.cmd_bit_len = 0xf;
	ck.cmd_low = low;
	ck.cmd_high = high;
	
	_chiffo_send_set_ctrl_cmd(ac,CH_DTYPE_PHONE_2_STM,&ck,sizeof(ck));
}

static void _chiffo_refresh_timer_for_day(smart_air_ctrl_t* ac,u_int8_t day_index)
{
	u_int8_t mask = 0x80;
	
	day_index&=0x7;
	if(day_index == 7){
		day_index = 0x0;
	}

	_chiffo_send_long_ctrl_cmd(ac,day_index|mask,CH_CMD_REFRESH_TIMER);
}

static void _chiffo_proc_setting_timer(smart_air_ctrl_t* ac,chiffo_priv_data_t* pv,cln_common_info_t *info,RS *ret)
{
	u_int8_t day_index;
	cl_chiffo_one_day_timer_t* dt;
	u_int8_t day_cmd = CH_DTYPE_WEEK_7_TIMER;
	ucp_chiffo_continu_cmd_item_t* ci,*p_enable,*tmp;
	u_int8_t buf[128] = {0},tbuf[32] = {0};
	int i ;
	int len = CHIFFO_TIMER_SECTION_PER_DAY+3;
	u_int16_t crc;
	
	day_index = info->u.u8_data[0] & 0x7;
	dt = (cl_chiffo_one_day_timer_t*)(&info->u.u8_data[1]);
	ci = (ucp_chiffo_continu_cmd_item_t*)buf;
	tmp = ci;

	if(day_index >= CHIFFO_TIMER_DAYS_CNT){
		day_index = 0;
	}

	day_cmd+=day_index;

	ci->cmd = CH_CMD_CONTINU_TYPE;
	ci->data = day_cmd;
	ci++;
	
	ci->cmd = CH_CMD_CONTINU_COUNT;
	ci->data = 0xB;
	ci++;

	ci->cmd = CH_CMD_CONTINU_DATA;
	ci->data = 0;
	p_enable = ci;
	ci++;

	for(i = 0;i<CHIFFO_TIMER_SECTION_PER_DAY;i++,ci++){
		ci->cmd = CH_CMD_CONTINU_DATA;
		ci->data = dt->items[i].temp;
		if(dt->items[i].is_enable){
			p_enable->data = (p_enable->data | BIT(i));
		}
	}
	
	memcpy(&pv->heater_info.timer_info[day_index],dt,sizeof(*dt));

	for(i=0;i<len;i++,tmp++){
		tbuf[i] = tmp->data;
	}

	crc = _chiffo_calc_crc((char*)tbuf,len);

	ci->cmd = CH_CMD_CONTINU_DATA;
	ci->data = crc &0xFF;
	ci ++;

	ci->cmd = CH_CMD_CONTINU_DATA;
	ci->data = (crc >> 8) &0xFF ;
	
	_chiffo_send_set_ctrl_cmd(ac,CH_DTYPE_PHONE_CONTINUE_DATA,buf,sizeof(ucp_chiffo_continu_cmd_item_t)*0xD);
	
}

static void _chiffo_proc_sync_clock(smart_air_ctrl_t* ac,chiffo_priv_data_t* pv,cln_common_info_t *info,RS *ret)
{
	ucp_chiffo_continu_cmd_item_t* ci, *tmp;
	u_int8_t buf[128] = {0}, value[8] = {0};
	int i ;
	int len = sizeof(value) + 4;	// 2 head + value + 2 crc
	u_int16_t crc;

	if (info->data_len != 8) {
		return;
	}

	memcpy(value, cci_pointer_data(info), 8);
	
	ci = (ucp_chiffo_continu_cmd_item_t*)buf;
	tmp = ci;

	ci->cmd = CH_CMD_CONTINU_TYPE;
	ci->data = CH_DTYPE_REAL_TIME;
	ci++;
	
	ci->cmd = CH_CMD_CONTINU_COUNT;
	ci->data = sizeof(value) + 2;
	ci++;

	for (i = 0; i < sizeof(value); i++, ci++) {
		ci->cmd = CH_CMD_CONTINU_DATA;
		ci->data = value[i];
	}

	// 校验和
	crc = _chiffo_calc_crc((char*)value, sizeof(value));

	ci->cmd = CH_CMD_CONTINU_DATA;
	ci->data = crc & 0xFF;
	ci++;

	ci->cmd = CH_CMD_CONTINU_DATA;
	ci->data = (crc >> 8) &0xFF ;
	
	_chiffo_send_set_ctrl_cmd(ac,CH_DTYPE_PHONE_CONTINUE_DATA, buf, sizeof(ucp_chiffo_continu_cmd_item_t) * len);
	
}


bool chiffo_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	chiffo_priv_data_t* pv;

	info = (cln_common_info_t *)&pkt->data[0];
	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "air_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pv = ac->com_udp_dev_info.device_info;
	
	switch(info->action){
		case ACT_CHIFFO_CTRL_ONOFF:
			pv->heater_info.is_on = cci_u8_data(info);
			if(pv->heater_info.is_on){
				_chiffo_send_short_ctrl_cmd(ac,CH_CMD_FH_ON);
			}else{
				_chiffo_send_short_ctrl_cmd(ac,CH_CMD_FH_OFF);
			}
			break;
        case ACT_CHIFFO_ADD_DEC_WATER_TEMP:
            if (!!cci_u8_data(info)) {
                _chiffo_send_short_ctrl_cmd(ac,CH_CMD_W_TEMP_ADD);
            }else{
                _chiffo_send_short_ctrl_cmd(ac,CH_CMD_W_TEMP_DEC);
            }
            break;
        case ACT_CHIFFO_ADD_DEC_HEATER_TEMP:
            if (!!cci_u8_data(info)) {
                _chiffo_send_short_ctrl_cmd(ac,CH_CMD_H_TEMP_ADD);
            }else{
                _chiffo_send_short_ctrl_cmd(ac,CH_CMD_H_TEMP_DEC);
            }
            break;
		case ACT_CHIFFO_CTRL_WATER_TEMP:
			pv->heater_info.water_setting_temp = cci_u8_data(info);

			//if (ac->sac->user->ext_type == ETYPE_IJ_CHIFFO_FlOOR_HEATER) {
				_chiffo_send_long_ctrl_cmd(ac,pv->heater_info.water_setting_temp,CH_CMD_WATER_SET_TEMP);
			//} else {
			//	_chiffo_send_long_ctrl_cmd(ac, pv->heater_info.water_setting_temp, 0x3b);
			//}
			break;
		case ACT_CHIFFO_CTRL_HEATER_TEMP:
			pv->heater_info.heater_setting_temp = cci_u8_data(info);
			_chiffo_send_long_ctrl_cmd(ac,pv->heater_info.heater_setting_temp,CH_CMD_HEATER_SET_TEMP);
			break;
		case ACT_CHIFFO_SET_MODE:
			pv->heater_info.is_heater_mode_on = cci_u8_data(info);
			if(pv->heater_info.is_heater_mode_on){
				_chiffo_send_short_ctrl_cmd(ac,CH_CMD_FH_H_ON);
			}else{
				_chiffo_send_short_ctrl_cmd(ac,CH_CMD_FH_H_OFF);
			}
			break;
		case ACT_CHIFFO_SET_TIMER:
			_chiffo_proc_setting_timer(ac,pv,info,ret);
			break;
		case ACT_CHIFFO_SET_CLOCK:
			_chiffo_proc_sync_clock(ac, pv, info, ret);
			break;
		case ACT_CHIFFO_REFRESH_TIMER:
			_chiffo_refresh_timer_for_day(ac,cci_u8_data(info));
			break;
		case ACT_CHIFFO_SET_SCENE:
			switch (cci_u8_data(info)) {
				case SCENE_AUTO:
					_chiffo_send_short_ctrl_cmd(ac,	CH_CMD_MODE_AUTO);
					break;
				case SCENE_BATH:
					_chiffo_send_short_ctrl_cmd(ac,	CH_CMD_MODE_BATH);
					break;
				case SCENE_DISHES:
					_chiffo_send_short_ctrl_cmd(ac,	CH_CMD_MODE_DISHES);
					break;
				case SCENE_VEGETABLES:
					_chiffo_send_short_ctrl_cmd(ac,	CH_CMD_MODE_VEGETABLES);
					break;
				case SCENE_CLOTHES:
					_chiffo_send_short_ctrl_cmd(ac,	CH_CMD_MODE_CLOTHES);
					break;

				default:
					*ret = RS_INVALID_PARAM;
					return false;
			}

			
			pv->heater_info.scene = cci_u8_data(info);
			break;
		case ACT_CHIFFO_SET_LOOP:
			log_debug("	chiffo set loop now %u, want %u\n", pv->heater_info.loop_type, cci_u8_data(info));
			switch (cci_u8_data(info)) {
				case 0:
					if (pv->heater_info.loop_type == 1) {
						_chiffo_send_short_ctrl_cmd(ac, CH_CMD_SINGLE_LOOP);
					} else if (pv->heater_info.loop_type == 2) {
						_chiffo_send_short_ctrl_cmd(ac, CH_CMD_SMART_LOOP);
					}
					break;
				case 1:
					if (pv->heater_info.loop_type != 1) {
						log_debug("	send single loop\n");
						_chiffo_send_short_ctrl_cmd(ac, CH_CMD_SINGLE_LOOP);
					}
					break;
				case 2:
					if (pv->heater_info.loop_type != 2) {
						log_debug("	send smart loop\n");
					_chiffo_send_short_ctrl_cmd(ac, CH_CMD_SMART_LOOP);
					}
					break;
				default:
					*ret = RS_INVALID_PARAM;
					return false;
			}

			pv->heater_info.loop_type = cci_u8_data(info);

			break;
		case ACT_CHIFFO_SET_WATER_CAP_UP_OR_DOWN:
			if (pv->heater_info.scene != SCENE_BATH) {
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}

			
			log_debug("	%s the water capacity\n", !!cci_u8_data(info) ? "UP" : "DOWN");
			
			
			if (!!cci_u8_data(info)) {
                _chiffo_send_short_ctrl_cmd(ac,CH_CMD_WATER_CAP_UP);
            } else {
                _chiffo_send_short_ctrl_cmd(ac,CH_CMD_WATER_CAP_DOWN);
            }
			
			break;
		case ACT_CHIFFO_SET_WATER_CAP:
			if (pv->heater_info.scene != SCENE_BATH) {
				log_err(false, "not in bath scene\n");
				*ret = RS_INVALID_PARAM;
				res = false;
				break;
			}
			pv->heater_info.water_capacity = cci_u8_data(info) * 10;			
			_chiffo_send_long_ctrl_cmd(ac, cci_u8_data(info), CH_CMD_WATER_CAP);
			log_debug("	set water_capacity to %u\n", pv->heater_info.water_capacity);
			break;
			
		default:
			*ret = RS_INVALID_PARAM;
			res = false;
			break;
	}
	return res;
}


static void _chiffo_floor_heater_do_update(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	u_int8_t stat,value,index;
	chiffo_priv_data_t* pv;
	cl_chiffo_one_day_timer_t* di;
	u_int8_t * ptemp;
	char show_hex[512] = {0};

	if(cmd_len < CH_STM_EXCEPT_PKT_LEN || !ac || !pcmd || !ac->com_udp_dev_info.device_info){
		return;
	}
    for (index = 0; index < 8; index++) {
        pcmd[index]^=0xFF;
    }
    
	mem_dump("_chiffo_floor_heater_do_update",pcmd,cmd_len);

	pv = (chiffo_priv_data_t*)ac->com_udp_dev_info.device_info;

	fmt_hex(show_hex, pcmd, cmd_len);
	log_debug("[%s]\n", show_hex);
	
    stat = pcmd[0];
    pv->disp_fault = (((stat >> 3) & 0x7) == 0x4)?true:false;
	//第2字节
	stat = pcmd[1];	
	pv->heater_info.is_fire_working = !! (stat & BIT(B2_BIT_SHIFT_FIRE));
	pv->heater_info.is_fan_working = !! (stat & BIT(B2_BIT_SHIFT_FAN));
	pv->heater_info.is_water_working = !! (stat & BIT(B2_BIT_SHIFT_WATER));
    
	//第3字节
	stat = pcmd[2];

	value = stat & TBIT_MASK;
	if (value == 1) {
		pv->heater_info.scene = SCENE_AUTO;
	} else if (value == 2) {
		pv->heater_info.scene = SCENE_BATH;
	} else if (value == 3) {
		pv->heater_info.scene = SCENE_DISHES;
	} else if (value == 4) {
		pv->heater_info.scene = SCENE_VEGETABLES;
	} else if (value == 5) {
		pv->heater_info.scene = SCENE_CLOTHES;
	}

	// 第4字节
	stat = pcmd[3];
	if (((stat >> 6) & DBIT_MASK) == 3) {
		pv->heater_info.loop_type = 0;
	} else if (((stat >> 6) & DBIT_MASK) == 1) {
		pv->heater_info.loop_type = 1;
	} else if (((stat >> 6) & DBIT_MASK) == 2) {
		pv->heater_info.loop_type = 2;
	} else {
		pv->heater_info.loop_type = 0;
	}

	log_debug("	loop_type %u by 0x%x\n", pv->heater_info.loop_type, ((stat >> 6) & DBIT_MASK));



	log_debug("	scene src 0x%x scene is %u\n", value, pv->heater_info.scene);
	
	value = (stat >> B3_BIT_SHIFT_HEATER_FUNC) & DBIT_MASK;
	if(value == CH_HS_FUNC_ON){
		pv->heater_info.is_heater_mode_on = true;
	}else{
		pv->heater_info.is_heater_mode_on = false;
	}
	//第4字节
	stat = pcmd[3];
	pv->heater_info.is_floor_heat_working =  !(stat & BIT(B4_BIT_SHIFT_HEATER_MODE));
	pv->heater_info.is_radiator_working = !pv->heater_info.is_floor_heat_working;
	pv->heater_info.is_pump_working = !!(stat & BIT(B4_BIT_SHIFT_PUMP));
	pv->heater_info.is_on = !(stat & BIT(B4_BIT_SHIFT_ON_OFF));

	// 第六个字节 b0-b1 信号强度 b2-b3 无线连接情况
	stat = pcmd[5];
	pv->heater_info.rssi = (stat & DBIT_MASK) + 1;
	log_debug("wifi rssi %u byte[4] 0x%x\n", pv->heater_info.rssi, pcmd[5]);

	// 第七个字节
	value = pcmd[6] & 0xf;
	pv->heater_info.need_show_water_capacity = 0;
	
	if (value == 0x9) {
		pv->heater_info.need_show_water_capacity = 1;
	}
	
	
	//第8字节数据指示
	stat = pcmd[7];
	if((stat & 0x1F) == 0 ){//水压数据
		pv->heater_info.cur_water_pressure = BCD_2_NUM(pcmd[13]);
	}else if( ((stat >> 3) & DBIT_MASK) == 1){// 星期数据
		value = stat & TBIT_MASK;
		if(value >= CHIFFO_TIMER_DAYS_CNT){
			return;
		}
		di = &pv->heater_info.timer_info[value];
		ptemp = &pcmd[14];
		for(index = 0 ; index < CHIFFO_TIMER_SECTION_PER_DAY ; index++,ptemp++){
			di->items[index].is_enable = !!(pcmd[13] & BIT(index));
			di->items[index].temp = *ptemp;
		}
	}
	
	pv->heater_info.water_setting_temp = BCD_2_NUM(pcmd[9]);//设置出水温度
	pv->heater_info.water_current_temp = BCD_2_NUM(pcmd[10]); //出水温度

	pv->heater_info.heater_setting_temp = BCD_2_NUM(pcmd[11]);//设置出水温度
	pv->heater_info.heater_current_temp = BCD_2_NUM(pcmd[12]);//出水温度

	log_debug("	set out water temp %u now water temp %u\n", pv->heater_info.water_setting_temp, pv->heater_info.water_current_temp);


	// 14 15字节是浴缸模式水量
	if (ac->sac->user->ext_type == ETYPE_IJ_CHIFFO_WATER_HEATER) {
		pv->heater_info.water_capacity = BCD_2_NUM(pcmd[13]) + BCD_2_NUM(pcmd[14]) * 100;
		log_debug("	water_capacity is %u\n", pv->heater_info.water_capacity);
	}
}

static void _chiffo_do_update(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
	ucp_chiffo_pkt_hdr_t* hdr;
	ucp_chiffo_pkt_tail_t* tail;

	if(cmd_len < sizeof(*hdr) + sizeof(*tail)){
		return;
	}
	
	hdr = (ucp_chiffo_pkt_hdr_t*)pcmd;
	tail = (ucp_chiffo_pkt_tail_t*)(pcmd+cmd_len-sizeof(*tail));
	
	if(hdr->s_code1 != CH_STM_START_CODE || hdr->s_code2 != CH_STM_START_CODE || 
		tail->e_code1 != CH_STM_END_CODE || tail->e_code2 != CH_STM_END_CODE){
		return;
	}

	pcmd+=sizeof(*hdr);
	cmd_len -=(sizeof(*hdr) + sizeof(*tail));
	
	switch(ac->sac->user->ext_type){
		case ETYPE_IJ_CHIFFO_FlOOR_HEATER:
		case ETYPE_IJ_CHIFFO_WATER_HEATER:
			_chiffo_floor_heater_do_update(ac,pcmd,cmd_len);
			break;
		default:
			break;
	}
}

static void _chiffo_do_fixed_data_update(smart_air_ctrl_t* ac,u_int8_t* pcmd, u_int16_t cmd_len)
{
    chiffo_priv_data_t* pv;
    
    if (cmd_len != CH_ALTER_DT_MAX || !ac->com_udp_dev_info.device_info) {
        return;
    }
    mem_dump("_chiffo_do_fixed_data_update",pcmd,cmd_len);
    
    pv = (chiffo_priv_data_t*)(ac->com_udp_dev_info.device_info);
    pv->heater_info.fault_code = BCD_2_NUM(pcmd[CH_ALTER_DT_FAULT]);

	pv->heater_info.hour = pcmd[CH_ALTER_DT_HOUR] & 0x1f;
	pv->heater_info.min = pcmd[CH_ALTER_DT_MIN] & 0x3f;
    
    log_debug("receive chiffo data len = %u hour %u min %u\n",cmd_len, pv->heater_info.hour, pv->heater_info.min);
}

static bool _chiffo_scm_update_tlv_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
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
			_chiffo_do_update(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
			break;
        	case TLV_TYPE_SCM_OTHER:
            		_chiffo_do_fixed_data_update(air_ctrl,(u_int8_t*)(tlv+1),tlv->len);
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

bool chiffo_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	switch (obj->attr) {
		case UCAT_IA_TT_ALLSTATE:
			ret = _chiffo_scm_update_tlv_data(air_ctrl,action,obj);
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

bool chiffo_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	chiffo_priv_data_t* priv;
	cl_chiffo_floor_heater_info_t* info;
	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	priv = (chiffo_priv_data_t*)ac->com_udp_dev_info.device_info;
	info = cl_calloc(sizeof(*info),1);
	if(!info){
		return false;
	}

	memcpy(info,&priv->heater_info,sizeof(*info));
//    if (!priv->disp_fault) {
//        info->fault_code = 0x0;
//    }

	di->device_info = info;

	return true;
}

int chiffo_get_ext_type_by_tlv(uc_tlv_t* tlv)
{
	int ext_type = ETYPE_IJ_CHIFFO_FlOOR_HEATER;
	tlv_dev_ident_t* ti;

	if(tlv->type == UCT_SCM_FACTORY_DATA && tlv->len == sizeof(tlv_dev_ident_t)){
		ti = (tlv_dev_ident_t*)(tlv+1);
		switch(ti->ident){
			case 0:
				ext_type = ETYPE_IJ_CHIFFO_FlOOR_HEATER;
				break;
			case 20:
				ext_type = ETYPE_IJ_CHIFFO_WATER_HEATER;
				break;
			default:
				break;
		}
	}

	return ext_type;	
}

