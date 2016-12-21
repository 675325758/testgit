#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "client_lib.h"
#include "cl_priv.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "smart_appliance_priv.h"
#include "lc_furnace_priv.h"
#include "md5.h"
#include "eb_priv.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"
#include "lbs.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "udp_device_common_priv.h"
#include "cl_qpcp.h"
#include "qpcp_priv.h"
#include "rfgw_priv.h"
#include "evm_scm_ctrl.h"
#include "yj_heater_scm_ctrl.h"
#include "cl_evm.h"
#include "evm_priv.h"
#include "linkage_client.h"
#include "linkage_priv.h"
#include "lanusers_priv.h"
#include "cl_priv.h"
#include "cl_thread.h"

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

//智能插座v3单品
enum{
    AIR_CM_STOP_MATCH = CL_AIR_CODE_MATCH_STOP,
    AIR_CM_CLOUD_MATCH = CL_AIR_CODE_MATCH_CLOUD,
    AIR_CM_ALL_MATCH = CL_AIR_CODE_MATCH_ALL,
    AIR_CM_MATCH_OK ,
    AIR_CM_MATCH_FAILED,
    AIR_CM_SWITCH_CODE //切换编码
};

enum{
	AIR_CMS_IDLE,/*闲置*/
	AIR_CMS_START_ALL_MATCH,/*启动全匹配*/
	AIR_CMS_START_CLOUD_MATCH,/*开始云匹配，等待回包*/
	AIR_CMS_WAIT_USER_IR_SIGNAL, /*等待按遥控器*/
	AIR_CMS_WAIT_CLOUD_MATCH_RESULT, /*等待云匹配结果*/
	AIR_CMS_WAIT_ALL_MATCH_RESULT/*等待全匹配结果*/
};

#define VERSION2(maj, min, rev)		((maj&0xff) << 24| (min&0xff) << 16| (rev&0xff) <<8)

static void air_code_match_switch_to_status(smart_air_ctrl_t* ac,u_int32_t status);
static bool sa_is_support_attr(smart_appliance_ctrl_t* sac,u_int16_t obj,u_int16_t sub_obj,u_int16_t attr);
void sa_comm_timer_query(ucc_session_t *s);
void comm_timer_next_cal(cl_comm_timer_head_t *pcthd);
extern u_int8_t timer_add_next_day(cl_comm_timer_t *ptimer);
extern void comm_timer_count_cal(cl_comm_timer_head_t *pcth);
extern void la_doname_sync(user_t* puser);
extern void dev_ver_reset_cal(user_t *user);
extern void do_passwd_sync_to_server(user_t *user);
extern void do_dev_la_sync();
extern void do_rfgw_spe_up_callback(user_t *user, u_int16_t error);
RS sa_stm_spe_upgrade_file(user_t *user, u_int8_t num, u_int8_t *psn, uc_spe_upgrade_pre_t *pre_src, u_int8_t force);
bool sa_stm_spe_get_info(char *file, uc_spe_upgrade_pre_t *pre);
RS sa_stm_spe_upgrade_file(user_t *user, u_int8_t num, u_int8_t *psn, uc_spe_upgrade_pre_t *pre_src, u_int8_t force);

typedef struct {
    u_int8_t hour;
    u_int8_t week;
}timer_switch_t;

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

static u_int8_t air_net_wind_2_cl_wind(u_int8_t wind,bool reverse)
{
    if (reverse) {
        if(wind == AC_WIND_3)
            return CL_AC_WIND_LOW;
        if(wind == AC_WIND_1)
            return CL_AC_WIND_HIGH;
        
    }else{
        if(wind == AC_WIND_3)
            return CL_AC_WIND_HIGH;
        
        if(wind == AC_WIND_1)
            return CL_AC_WIND_LOW;
    }
	return wind;
}

static RS sa_quick_query_single_object(ucc_session_t *s,ucp_obj_t* obj)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    ucp_obj_t* uo;
    
    if (!s || !obj ) {
        return RS_ERROR;
    }
    
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+sizeof(ucp_obj_t),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_GET;
    uc->count = 0x1;
    uo = (ucp_obj_t*)(uc+1);
    memcpy(uo, obj, sizeof(*obj));
    uo->param_len = 0;
    
    ucc_request_add(s, pkt);
    
    return RS_OK;
    
}

RS sa_query_obj(ucc_session_t *s,u_int16_t obj,u_int16_t sub_obj, u_int16_t attr)
{
    ucp_obj_t t_obj = {0};
    
    if (!s) {
        return RS_INVALID_PARAM;
    }
    
    t_obj.objct = htons(obj);
    t_obj.sub_objct = htons(sub_obj);
    t_obj.attr = htons(attr);
    
    return sa_quick_query_single_object(s,&t_obj);
}

static bool parse_system_license_active(smart_air_ctrl_t* air_ctrl,ucp_obj_t* obj)
{
	net_active_st_t *actv_st = (net_active_st_t*)(obj+1);
	int param_len = obj->param_len;

	if(param_len < sizeof(net_active_st_t)){
		log_err(true,"Error when parse_system_license_active param_len = %u < %u\n",param_len, sizeof(net_active_st_t));
		return false;
	}

	air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
	air_ctrl->stat.udp_dev_stat.active_st.active_status = ntohl(actv_st->active_status);
	air_ctrl->stat.udp_dev_stat.active_st.active_sn = ntoh_ll(actv_st->active_sn);
	log_info("=======parse_system_license_active: active_status:%u, active_sn:%012llu================\n", 
		air_ctrl->stat.udp_dev_stat.active_st.active_status, air_ctrl->stat.udp_dev_stat.active_st.active_sn);
	
	return true;
}

static bool parse_system_debug_info(smart_air_ctrl_t* air_ctrl,ucp_obj_t* obj)
{

	uc_tlv_t *tlv;
	int remain = obj->param_len;
	int obj_len;
	//光感学习
	int i;
//	light_smp_t *lt;
	u_int32_t temp= 0;
	u_int16_t temp_s= 0;	
//	u_int8_t data[1024];
	light_smp_t lst;
	
	tlv = (uc_tlv_t*)(obj+1);
	while (remain >= sizeof(uc_tlv_t) ) {
		tlv->len = ntohs(tlv->len);
		tlv->type = ntohs(tlv->type);

		obj_len = tlv->len+sizeof(*tlv);
		if(obj_len > remain){
			log_err(true,"Error when parse_system_debug_info obj_len = %u remain_len = %u\n",obj_len,remain);
			break;
		}
		
		remain -= obj_len;
		if(tlv->len == 0){
			tlv = tlv_next(tlv);
			continue;
		}
		
		switch (tlv->type) {
		case DBG_TYPE_CUR:
			if (tlv->len == sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				temp = htonl(temp);		
				air_ctrl->stat.AcCur = temp;
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_cur = temp;
			}
			break;
		case DBG_TYPE_VOL:
			if (tlv->len == sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				temp = htonl(temp);			
				air_ctrl->stat.ACVal = temp;
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_vol = temp;
			}
			break;
		case DBG_TYPE_VOL_AD:
			if (tlv->len == sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				temp = htonl(temp);					
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_vol_ad = temp;
			}
			break;
		case DBG_TYPE_VOl_K:
			if (tlv->len == sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				temp = htonl(temp);					
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_vol_k = temp;
			}
			break;
		case DBG_TYPE_VOL_B:
			if (tlv->len == sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				temp = htonl(temp);					
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_vol_b = temp;
			}
			break;
		case DBG_TYPE_UPTIME:
			if (tlv->len == sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				temp = htonl(temp);
				//debug_info("DBG_TYPE_UPTIME=%u\n", temp);
			}
			break;	
		case DBG_TYPE_LIGHT_STUDY:
			if (tlv->len == sizeof(lst)) {
				memcpy(&lst, (u_int8_t *)(tlv+1), sizeof(lst));
			}
			lst.today = htonl(lst.today);
			lst.yesterday = htonl(lst.yesterday);							
			for(i = 0; i < DAY_HOUR; i++) {
				lst.sample[i] = htonl(lst.sample[i]);
			}
			air_ctrl->stat.LightYes = lst.yesterday;
			air_ctrl->stat.LightNext = lst.today;	

			memcpy((u_int8_t *)&air_ctrl->stat.udp_dev_stat.light_study, (u_int8_t *)&lst, sizeof(light_smp_t));
			break;
		case DBG_TYPE_DEV_SERVER_IP:
			if (tlv->len >= sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				air_ctrl->stat.DevServerIp = htonl(temp);
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_server_ip = air_ctrl->stat.DevServerIp;
			}
			break;
		case DBG_TYPE_SMT_SOFT_VERSION:
			if (tlv->len >= sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				air_ctrl->stat.StmSoftVersion = htonl(temp);
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.smt_soft_ver = air_ctrl->stat.StmSoftVersion;
                air_ctrl->stat.stm_cur_version.major = (air_ctrl->stat.udp_dev_stat.smt_soft_ver>>24)&0xff;
                air_ctrl->stat.stm_cur_version.minor = (air_ctrl->stat.udp_dev_stat.smt_soft_ver>>16)&0xff;
                air_ctrl->stat.stm_cur_version.revise = (air_ctrl->stat.udp_dev_stat.smt_soft_ver>>8)&0xff;
			}
			break;
		case DBG_TYPE_SMT_HARD_VERSION:
			if (tlv->len >= sizeof(temp)) {
				temp = *(u_int32_t *)(tlv+1);
				air_ctrl->stat.StmHardVersion = htonl(temp);
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.smt_hard_ver = air_ctrl->stat.StmHardVersion;
				air_ctrl->air_info.smt_hard_ver = air_ctrl->stat.udp_dev_stat.smt_hard_ver;
			}
			break;	
		case DBG_TYPE_IR_LIB_ID:
			if (tlv->len >= sizeof(temp_s)) {
				temp_s = *(u_int16_t *)(tlv+1);
				air_ctrl->stat.IrId = htons(temp_s);
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.ir_lib_id = air_ctrl->stat.IrId;
				air_ctrl->air_info.id = air_ctrl->stat.IrId;
			}
			break;	

		case DBG_TYPE_SVN:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.devSvnVersion = ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_CUR_AD:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_cur_ad= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_CUR_K:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_cur_k= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_CUR_B:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_cur_b= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_LIGHT_AD:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_light_ad= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_SERVER_DONAME:
			if(tlv->len > 0){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				strncpy(air_ctrl->stat.udp_dev_stat.dev_domain,(char*)(tlv+1),sizeof(air_ctrl->stat.udp_dev_stat.dev_domain)-1);
			}
			break;
		case DBG_TYPE_SERVER_CONNTIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.online= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_CLIENTS:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.dev_cur_phone_num= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_CPU_USAGE:
			if(tlv->len >= sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.cpu = (u_int16_t)ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_MEM_USAGE:
			if(tlv->len >= sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.mem = (u_int16_t)ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_AVG_AD:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.avg_ad= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_MAX_AD:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.max_ad= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_DELAY_POWER_ON_TIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.delay_power_on_time= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_DELAY_POWER_OFF_TIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.delay_power_off_time= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_NO_LOAD_AD:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.no_load_ad= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_COLD_DELAY_PN_TIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.cold_delay_pn_time= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_COLD_DELAY_PF_TIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.cold_delay_pf_time= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_HOT_DELAY_PN_TIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.hot_delay_pn_time= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_HOT_DELAY_PF_TIME:
			if(tlv->len == sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.hot_delay_pf_time= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_STM32_INFO:
			if(tlv->len > 0){
				char* p;
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				p = (char*)(tlv+1);
				p[tlv->len-1] = '\0';
				if(air_ctrl->stat.udp_dev_stat.stm_32_dbg_info != NULL){
					cl_free(air_ctrl->stat.udp_dev_stat.stm_32_dbg_info);
					air_ctrl->stat.udp_dev_stat.stm_32_dbg_info = NULL;
				}
				if(strlen(p) > 0){
					air_ctrl->stat.udp_dev_stat.stm_32_dbg_info = cl_strdup(p);
				}
			}
			break;
		case DBG_TYPE_ADJUST_PRESSED:
			if(tlv->len >= sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.adjust_pressed= ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_PT_STATUS_FLAG:
			if(tlv->len >= sizeof(u_int32_t)){
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.pt_stat_flag = ntohl(*((u_int32_t*)(tlv+1)));
			}

		case DBG_TYPE_TIME_CONN_WIFI:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.wifi_conn_time = ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_TYPE_WIFI_RSSI:
			if (tlv->len >= sizeof(int8_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.wifi_rssi = *((int8_t*)(tlv+1));
			}
			break;
		case DBG_TYPE_WIFI_PHY_MODE:
			if (tlv->len >= sizeof(u_int8_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.wifi_phy_mode = *((u_int8_t*)(tlv+1));
			}
			break;
		case DBG_TYPE_WIFI_VERSION:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				temp = ntohl(*((u_int32_t*)(tlv+1)));
				air_ctrl->stat.udp_dev_stat.wifi_version.major = (temp >> 24) & 0xff;
				air_ctrl->stat.udp_dev_stat.wifi_version.minor = (temp >> 16) & 0xff;
				air_ctrl->stat.udp_dev_stat.wifi_version.revise= (temp >> 8) & 0xff;
				//用来表示wifi信号，模式，版本几个字段是否有效
				air_ctrl->stat.udp_dev_stat.wifi_version.pad = 0x11;
			}
			break;
		case DBG_KERNEL_IMAGE_VERSION:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				temp = ntohl(*((u_int32_t*)(tlv+1)));
				air_ctrl->stat.udp_dev_stat.kernel_image_version.major = (temp >> 24) & 0xff;
				air_ctrl->stat.udp_dev_stat.kernel_image_version.minor = (temp >> 16) & 0xff;
				air_ctrl->stat.udp_dev_stat.kernel_image_version.revise= (temp >> 8) & 0xff;
			}
			break;

		case DBG_KERNEL_IMAGE_SVN:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.kernel_image_svn = ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
		case DBG_USER_IMAGE_VERSION:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				temp = ntohl(*((u_int32_t*)(tlv+1)));
				air_ctrl->stat.udp_dev_stat.user_image_version.major = (temp >> 24) & 0xff;
				air_ctrl->stat.udp_dev_stat.user_image_version.minor = (temp >> 16) & 0xff;
				air_ctrl->stat.udp_dev_stat.user_image_version.revise= (temp >> 8) & 0xff;
			}
			break;
		case DBG_USER_IMAGE_SVN:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.user_image_svn = ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
#if 0
		case DBG_WIRELESS_WAN_IP:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
				air_ctrl->stat.udp_dev_stat.wireless_wan_ip = ntohl(*((u_int32_t*)(tlv+1)));
			}
			break;
#endif			
		case DBG_WIRED_WAN_IP:
			if (tlv->len >= sizeof(u_int32_t)) {
				air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
//				air_ctrl->stat.udp_dev_stat.wired_wan_ip = ntohl(*((u_int32_t*)(tlv+1)));
                air_ctrl->stat.udp_dev_stat.wired_wan_ip = *((u_int32_t*)(tlv+1));
				//air_ctrl->stat.wan.ip =  ntohl(*((u_int32_t*)(tlv+1)));
		//		air_ctrl->stat.wan.ip = *((u_int32_t*)(tlv+1));
				
			}
			break;

		case DBG_TYPE_MAC_ADDR:
			if (tlv->len >= sizeof(air_ctrl->stat.udp_dev_stat.wifi_mac)) {
				memcpy(air_ctrl->stat.udp_dev_stat.wifi_mac, (u_int8_t*)&tlv[1], sizeof(air_ctrl->stat.udp_dev_stat.wifi_mac));
			}
			break;
		 default:
		 	break;
		}

		tlv = tlv_next(tlv);
	}
	return true;
}

static bool air_update_sys_soft_data(smart_air_ctrl_t* air_ctrl,ucp_obj_t* obj)
{
	switch(obj->attr){
		case UCAT_SYS_EVM_INFO:
			if (is_valid_obj_data(obj, sizeof(evm_block_t))) {
				log_debug("get evminfo len %u\n", obj->param_len);

				sys_get_evm_info(air_ctrl, obj);
			}

			break;
			
		case UCAT_SYS_UPTIME:
			if( is_valid_obj_data(obj, sizeof(u_int32_t))){
				air_ctrl->stat.uptime = ntohl(obj_u32_value(obj));
				log_debug("system uptime %d\n",air_ctrl->stat.uptime);
			}
			break;
		case UCAT_SYS_IP:
			if( is_valid_obj_data(obj, sizeof(u_int32_t))){
				 air_ctrl->stat.wan.ip = obj_u32_value(obj);
				air_ctrl->sac->user->dev_ip = air_ctrl->stat.wan.ip;
                
                		log_debug("system ip %u.%u.%u.%u\n",IP_SHOW(air_ctrl->stat.wan.ip));
			}
			break;
		case UCAT_SYS_VERSION:
			if(is_valid_obj_data(obj, sizeof(ucp_version_t))){
				ucp_version_t* uv;
				bool change = false;
				u_int32_t v1,v2;
				uv = (ucp_version_t*)(obj+1);

				v1 = VERSION2(air_ctrl->stat.soft_version.major, air_ctrl->stat.soft_version.minor, air_ctrl->stat.soft_version.revise);
				v2 = VERSION2(uv->major, uv->minor, uv->revise);
				if (v1 != v2) {
					change = true;
				}

				v1 = VERSION2(air_ctrl->stat.upgrade_version.major, air_ctrl->stat.upgrade_version.minor, air_ctrl->stat.upgrade_version.revise);
				if (v1 != v2) {
					change = true;
				}
				
                if (change) {
					air_ctrl->stat.soft_version.major = uv->major;
					air_ctrl->stat.soft_version.minor = uv->minor;
					air_ctrl->stat.soft_version.revise = uv->revise;

					air_ctrl->stat.upgrade_version.major = uv->major;
					air_ctrl->stat.upgrade_version.minor = uv->minor;
					air_ctrl->stat.upgrade_version.revise = uv->revise;

					dev_ver_reset_cal(air_ctrl->sac->user);
                }
				log_debug("soft_version %u.%u.%u change=%u\n",uv->major,uv->minor,uv->revise, change);
			}
			break;
		case UCAT_SYS_SSID:
			SAFE_FREE(air_ctrl->stat.ap_ssid);
			air_ctrl->stat.ap_ssid = cl_strdup(OBJ_VALUE(obj, char*));
			break;
		case UCAT_SYS_PASSWD:
			SAFE_FREE(air_ctrl->stat.ap_passwd);
			air_ctrl->stat.ap_passwd = cl_strdup(OBJ_VALUE(obj, char*));
			break;
		case UCAT_SYS_DEVSTATUS:
			air_ctrl->stat.udp_dev_stat.dev_to_server_stat = obj_u8_value( obj);
			break;
		case UCAT_SYS_COM_DATE:
			strncpy((char*)air_ctrl->stat.udp_dev_stat.dev_img_compile_data,OBJ_VALUE(obj, char*),sizeof(air_ctrl->stat.udp_dev_stat.dev_img_compile_data)-1);
			break;
		case UCAT_SYS_COM_TIME:
			strncpy((char*)air_ctrl->stat.udp_dev_stat.dev_img_compile_time,OBJ_VALUE(obj, char*),sizeof(air_ctrl->stat.udp_dev_stat.dev_img_compile_time)-1);
			break;
		case UCAT_SYS_SYSTIME:
			air_ctrl->stat.udp_dev_stat.dev_cur_time = ntohl(obj_u32_value(obj));
			break;
		case UCAT_SYS_DEBUGINFO:
			 //log_debug("receive system debugInfo packet!\n");
			 parse_system_debug_info(air_ctrl,obj);
			break;
		default:
			return false;
	}
	return true;
}


static bool air_update_system_data(smart_air_ctrl_t* air_ctrl, u_int8_t action, ucp_obj_t* obj)
{
	char buff[1024];

	air_ctrl->stat.udp_dev_stat.is_stat_valid = true;
    switch (obj->sub_objct) {
		case UCSOT_LANUSERS_MANAGE:
			return lanusers_update_ia_data(air_ctrl, action, obj);
			
        case UCSOT_SYS_HOSTNAME:
        {
            //主机名称
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HOSTNAME, UCAT_HOSTNAME_HOSTNAME)) {
                
                SAFE_FREE(air_ctrl->sac->user->nickname);
                air_ctrl->sac->user->nickname= obj_string_dup(obj);
                log_debug("name %s\n",air_ctrl->sac->user->nickname);
                return true;
            }
        }
            break;
        case UCSOT_SYS_SOFTWARE:
        {
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_WIFI_STATE) &&
                is_valid_obj_data(obj, sizeof(u_int8_t))) {
                air_ctrl->com_udp_dev_info.dev_wifi_state = obj_u8_value(obj);
                
                log_debug("dev wifi state =%u\n", air_ctrl->com_udp_dev_info.dev_wifi_state);
                
                return true;
            }
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_WIFI_STATE_S3) &&
                is_valid_obj_data(obj, sizeof(u_int8_t))) {
                air_ctrl->com_udp_dev_info.dev_wifi_state = obj_u8_value(obj);
                
                log_debug("dev wifi state =%u\n", air_ctrl->com_udp_dev_info.dev_wifi_state);
                
                return true;
            }
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_TIMEZONE) &&
                is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->com_udp_dev_info.timezone_valid = true;
				air_ctrl->com_udp_dev_info.timezone = htonl(obj_u32_value(obj));
				air_ctrl->stat.udp_dev_stat.timezone_valid = true;
				air_ctrl->stat.udp_dev_stat.timezone = htonl(obj_u32_value(obj));
                
                log_debug("dev timezone =%u\n", air_ctrl->stat.udp_dev_stat.timezone);
                
                return true;
            }
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_SVN) &&
                is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->stat.svn_num = htonl(obj_u32_value(obj));
                
                log_debug("svn =%u\n",air_ctrl->stat.svn_num);
                
                return true;
            }
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_SSID)) {
				if (air_ctrl->stat.ap_ssid) {
					free(air_ctrl->stat.ap_ssid);
					air_ctrl->stat.ap_ssid = NULL;
				}
				memset(buff, 0, sizeof(buff));
				if (obj->param_len < sizeof(buff)) {
					memcpy(buff, (char *)(obj+1), obj->param_len);
					air_ctrl->stat.ap_ssid = strdup(buff);
				}
				//_debug_memdump("UCAT_SYS_SSID", (char *)(obj+1), obj->param_len);
				//_debug_memdump("buff", buff, obj->param_len);
				log_debug("ap ssid  =%s obj->param_len=%u\n",air_ctrl->stat.ap_ssid, obj->param_len);
				
				return true;
			}		
			
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_PASSWD)) {
				if (air_ctrl->stat.ap_passwd) {
					free(air_ctrl->stat.ap_passwd);
					air_ctrl->stat.ap_passwd = NULL;
				}
				memset(buff, 0, sizeof(buff));
				if (obj->param_len < sizeof(buff)) {
					memcpy(buff, (char *)(obj+1), obj->param_len);
					air_ctrl->stat.ap_passwd = strdup(buff);
				}
				log_debug("ap pwd	=%s\n",air_ctrl->stat.ap_passwd);
				
				return true;
			}
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_COM_DATE)) {
				if (air_ctrl->stat.compile_date) {
					free(air_ctrl->stat.compile_date);
					air_ctrl->stat.compile_date = NULL;
				}
				memset(buff, 0, sizeof(buff));
				if (obj->param_len < sizeof(buff)) {
					memcpy(buff, (char *)(obj+1), obj->param_len);
					air_ctrl->stat.compile_date = strdup(buff);
					strncpy((char*)air_ctrl->stat.udp_dev_stat.dev_img_compile_data,air_ctrl->stat.compile_date,strlen(air_ctrl->stat.compile_date));
				}
				log_debug("compile_date  =%s\n",air_ctrl->stat.compile_date);
				
				return true;
			}

			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_COM_TIME)) {
				if (air_ctrl->stat.compile_time) {
					free(air_ctrl->stat.compile_time);
					air_ctrl->stat.compile_time = NULL;
				}
				memset(buff, 0, sizeof(buff));
				if (obj->param_len < sizeof(buff)) {
					memcpy(buff, (char *)(obj+1), obj->param_len);
					air_ctrl->stat.compile_time = strdup(buff);
					strncpy((char*)air_ctrl->stat.udp_dev_stat.dev_img_compile_time,air_ctrl->stat.compile_time,strlen(air_ctrl->stat.compile_time));
				}
				log_debug("compile_time  =%s\n",air_ctrl->stat.compile_time);
				
				return true;
			}	

			//系统时间
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_SYSTIME) &&
				is_valid_obj_data(obj, sizeof(u_int32_t))) {
				air_ctrl->stat.systime = ntohl(obj_u32_value(obj));
				air_ctrl->stat.udp_dev_stat.dev_cur_time = air_ctrl->stat.systime;
				log_debug("systime	=%u\n",htonl(air_ctrl->stat.systime));
				
				return true;
			}

			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_DEVSTATUS) &&
				is_valid_obj_data(obj, sizeof(u_int8_t))) {
				air_ctrl->stat.dev_status = *(u_int8_t *)(obj+1);
				air_ctrl->stat.udp_dev_stat.dev_to_server_stat = air_ctrl->stat.dev_status;
				log_debug("system devstatus %d\n",air_ctrl->stat.dev_status);
				
				return true;
			}	
			
        	return air_update_sys_soft_data(air_ctrl,obj);
        }
            break;

        case UCSOT_SYS_LICENSE:
        {
            if(is_except_attr(obj,  UCOT_SYSTEM, UCSOT_SYS_LICENSE, UCAT_SYS_LICENSE_ACTIVE))
            {
                 log_debug("receive system license active packet!\n");
                 parse_system_license_active(air_ctrl,obj);
                return true;
            }
        }
	    	break;
        case UCSOT_SYS_UPGRADE:
        {
            //升级包版本
            ucp_version_t* uv;
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_UPGRADE, UCAT_SYS_VERSION) &&
                is_valid_obj_data(obj, sizeof(ucp_version_t))) {
                uv = (ucp_version_t*)(obj+1);
                
                air_ctrl->stat.upgrade_version.major = uv->major;
                air_ctrl->stat.upgrade_version.minor = uv->minor;
                air_ctrl->stat.upgrade_version.revise = uv->revise;
				air_ctrl->sac->user->upgrade_version = VERSION2(uv->major, uv->minor, uv->revise);
                
                log_debug("upgrade_version %u.%u.%u\n",uv->major,uv->minor,uv->revise);
                
                return true;
            }
        }
            break;
        case UCSOT_SYS_HARDWARE:
        {
            ucp_led_t* ul;
			int i;
			u_int16_t len;
			ucp_disk_info_t *disk_net;
			cl_disk_info_t *disk_local;
			ucp_eth_info_t *eth_net;
			cl_eth_info_t *eth_local;
			
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_HARDWARE_DISK_INFO_GET)) {
				disk_local = &air_ctrl->stat.udp_dev_stat.disk_info;
				disk_net = (ucp_disk_info_t *)(obj+1);

				disk_local->valid = true;
				
				len = (u_int16_t)(sizeof(*disk_net) + disk_net->num * sizeof(disk_net->disk_item[0]));
				if (obj->param_len < len) {
					log_err(false, "err param_len=%u len=%u\n", obj->param_len, len);
					return true;
				}
				disk_local->mode = disk_net->mode;
				disk_local->total_capacity = htonl(disk_net->total_capacity);
				disk_local->used_capacity = htonl(disk_net->used_capacity);
				disk_local->num = disk_net->num;
				log_debug("UCAT_HARDWARE_DISK_INFO_GET mode=%u total_capacity=%u used_capacity=%u num=%u\n", 
					disk_local->mode, disk_local->total_capacity, disk_local->used_capacity, 
					disk_local->num);
				for(i = 0; i < disk_local->num && i < DISK_MAX_NUM; i++) {
					disk_local->disk_item[i].temp = (short)htons(disk_net->disk_item[i].temp);
					disk_local->disk_item[i].use_time = htonl(disk_net->disk_item[i].use_time);
					disk_local->disk_item[i].capacity = htonl(disk_net->disk_item[i].capacity);
					memcpy(disk_local->disk_item[i].model, disk_net->disk_item[i].model, DISK_MAX_MODEL_LEN);
					disk_local->disk_item[i].model[DISK_MAX_MODEL_LEN - 1] = 0;
					memcpy(disk_local->disk_item[i].serial, disk_net->disk_item[i].serial, DISK_MAX_MODEL_LEN);
					disk_local->disk_item[i].serial[DISK_MAX_MODEL_LEN - 1] = 0;
					log_debug("i=%d temp=%d user_time=%u capacity=%u model=%s serial=%s\n", 
						i, disk_local->disk_item[i].temp, disk_local->disk_item[i].use_time, 
						disk_local->disk_item[i].capacity, disk_local->disk_item[i].model, 
						disk_local->disk_item[i].serial);
				}
				
				return true;
			}

			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_HARDWARE_ETH_INFO_GET)) {
				eth_local = &air_ctrl->stat.udp_dev_stat.eth_info;
				eth_net = (ucp_eth_info_t *)(obj+1);

				eth_local->valid = true;
				len = (u_int16_t)(sizeof(*eth_net) + eth_net->num * sizeof(eth_net->eth_item[0]));
				if (obj->param_len < len) {
					log_err(false, "err param_len=%u len=%u\n", obj->param_len, len);
					return true;
				}
				eth_local->num = eth_net->num;
				log_debug("eth_local->num =%u\n", eth_local->num);
				for(i = 0; i < eth_net->num && i < ETH_MAX_NUM; i++) {
					eth_local->eth_item[i].index = eth_net->eth_item[i].index;
					eth_local->eth_item[i].rx_rate = htonl(eth_net->eth_item[i].rx_rate);
					eth_local->eth_item[i].tx_rate = htonl(eth_net->eth_item[i].tx_rate);
					eth_local->eth_item[i].ip = htonl(eth_net->eth_item[i].ip);
					strncpy((char *)eth_local->eth_item[i].name, (char *)eth_net->eth_item[i].name, 
						sizeof(eth_local->eth_item[i].name) - 1);
					log_debug("i=%d index=%u rx_rate=%u tx_rate=%u ip=%u.%u.%u.%u name=%s\n", 
						i, eth_local->eth_item[i].index, eth_local->eth_item[i].rx_rate, 
						eth_local->eth_item[i].tx_rate, IP_SHOW(eth_local->eth_item[i].ip), 
						eth_local->eth_item[i].name);
				}
				
				return true;
			}
		
            //LED状态：用户关闭，一定关闭，用户开启，设备可能自动关闭他，都真才是开启
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED) &&
                is_valid_obj_data(obj, sizeof(ucp_led_t))) {
                ul = (ucp_led_t*)(obj+1);
                air_ctrl->air_info.air_led_on_off = ul->user_enable;
                
                log_debug("led device enable %u user_enable %u\n",ul->on_off,ul->user_enable);
                return true;
            }

		
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_SYS_VERSION) &&
                is_valid_obj_data(obj, sizeof(ucp_version_t))) {
		    	ucp_version_t* uv;
                uv = (ucp_version_t*)(obj+1);
                
                air_ctrl->stat.hardware_version.major = uv->major;
                air_ctrl->stat.hardware_version.minor = uv->minor;
                air_ctrl->stat.hardware_version.revise = uv->revise;
                
                log_debug("hard ware %u.%u.%u\n",uv->major,uv->minor,uv->revise);
                
                return true;
            }

		 if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED_COLOR) &&
                is_valid_obj_data(obj, sizeof(ucp_led_color_t))) {
                ucp_led_color_t* ulc = (ucp_led_color_t*)(obj+1);
                air_ctrl->air_info.led_color.air_off_color = ulc->color_of_ac_off;
		   		air_ctrl->air_info.led_color.air_on_color = ulc->color_of_ac_on;
                
                log_debug("led color on %u user_enable  off %u\n",ulc->color_of_ac_on,ulc->color_of_ac_off);
		
                return true;
            }

		if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_HARDWARE_STM_VERSION) &&
                is_valid_obj_data(obj, sizeof(ucp_version_t))) {
		    ucp_version_t* uv;
                uv = (ucp_version_t*)(obj+1);
                
                air_ctrl->stat.stm_cur_version.major = uv->major;
                air_ctrl->stat.stm_cur_version.minor = uv->minor;
                air_ctrl->stat.stm_cur_version.revise = uv->revise;
                
                log_debug("stm current version %u.%u.%u\n",uv->major,uv->minor,uv->revise);
                
                return true;
            }
        }
            break;
        case UCSOT_SYS_VENDOR:
        {
            //厂商ID
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_VENDOR, UCAT_VENDOR_OEM_ID)) {
                SAFE_FREE(air_ctrl->sac->user->vendor_id);
                air_ctrl->sac->user->vendor_id = obj_string_dup(obj);
                
                log_debug("vendor %s \n",air_ctrl->sac->user->vendor_id );
                return true;
            }
        }
            break;
        case UCSOT_SYS_SERVER:
        {
            // 连接服务器的时间
            if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SERVER, UCAT_SERVER_CONNECT_TIME)) {
                air_ctrl->stat.online = ntohl(obj_u32_value(obj));
                log_debug("connect to server %u \n",air_ctrl->stat.online);
                return true;
            }
		
			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SERVER, UCAT_SERVER_DONAME)) {
				if (obj->param_len < sizeof(air_ctrl->stat.server_domainname)) {
					memcpy(air_ctrl->stat.server_domainname, (char *)(obj+1), obj->param_len);
					air_ctrl->stat.server_domainname[obj->param_len] = 0;
				}
				la_doname_sync(air_ctrl->sac->user);
				log_debug("get server domain name=%s\n", air_ctrl->stat.server_domainname);
				return true;
			}	

			if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SERVER, UCAT_SERVER_IP) &&
				is_valid_obj_data(obj, sizeof(u_int32_t))) {
				air_ctrl->stat.server_ip = htonl(obj_u32_value(obj));
				air_ctrl->stat.udp_dev_stat.dispatch_ip = air_ctrl->stat.server_ip;
				log_debug("get server ip=%u.%u.%u.%u\n", IP_SHOW(air_ctrl->stat.server_ip));
				return true;
			}	
        }
            break;
	case UCSOT_SYS_USER:
	{
		 if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_PASSWORD) &&
             is_valid_obj_data(obj, 16)) {
             u_int8_t * md5 = (u_int8_t*)(obj+1);
             mem_dump("dev new password", md5, 16);
             memcpy(air_ctrl->sac->dev_pass_md5, md5, 16);
             return true;
            }
		 
        if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_LOCATION)){
            return on_get_home_location(air_ctrl->sac->user, (void*)obj);
        }
		break;
	}
    case UCSOT_SYS_DEV_SHARE:
        {
            switch (obj->attr) {
                case UCAT_SYS_CODE:
                    if (obj->param_len > 0) {
						user_t* user;
                        SAFE_FREE(air_ctrl->air_info.requested_share_code);
                        air_ctrl->air_info.requested_share_code = (u_int8_t*)cl_strdup((char*)(obj+1));
						user = air_ctrl->sac->user;
        				event_push(user->callback, SAE_SHARE_COEE_GET, user->handle, user->callback_handle);
       					event_cancel_merge(user->handle);
						return true;
                    }
                    break;
                case UCAT_PHONE_LIST:
                {
                    ucp_share_record_t* ur;
                    cl_share_record_t* si;
                    u_int8_t k;
                    
                    SAFE_FREE(air_ctrl->air_info.share_info.records);
                    air_ctrl->air_info.share_info.record_num = 0;
                    air_ctrl->air_info.share_info.record_num = (u_int8_t)obj->param_len/sizeof(ucp_share_record_t);
                    if (air_ctrl->air_info.share_info.record_num > 0  ) {
                        si = cl_calloc(sizeof(cl_share_record_t)*air_ctrl->air_info.share_info.record_num, 1);
						if (!si) {
							return false;
						}
                        air_ctrl->air_info.share_info.records = si;
                        for (ur = (ucp_share_record_t*)(obj+1),k = 0 ; k < air_ctrl->air_info.share_info.record_num; k++,ur++,si++) {
                            si->phone_index = ntohl(ur->phone_index);
                            si->phone_last_operate_time = ntohl(ur->phone_last_operate_time);
                            si->phone_operate_num = ntohl(ur->phone_operate_num);
                            si->phone_share_time = ntohl(ur->phone_share_time);
                            memcpy(si->phone_desc, ur->phone_desc, sizeof(si->phone_desc));
                        }
						return true;
                    }
                }
                    break;
                default:
                    break;
            }
        }
            break;
        default:
            break;
    }
    return false;
}

static void airplug_timer_utc_2_local(ucp_ac_timer_item_t *timer, int zone)
{
#ifdef USE_TIME_MINS	
	int time_min = 0;

	time_min = timer->hour*60 + timer->minute + cl_priv->time_diff + 24*60;
	if (time_min < 24*60) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (time_min >= 48*60) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = (time_min/60)%24;
	timer->minute = time_min%60;
#else	
	int hour;

	hour = timer->hour + 24 + zone;
	if (hour < 24) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (hour >= 48) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = hour%24;
#endif	
}

static u_int8_t airplug_hour_utc_2_local(u_int8_t hour, u_int8_t min, int zone)
{
#ifdef USE_TIME_MINS
	int time = hour*60 + min;

	time += cl_priv->time_diff + 24*60;
	
	hour = (time/60)%24;
#else
	hour = hour + 24 + zone;
	hour = hour%24;
#endif

	return hour;
}

void airplug_timer_local_2_utc(ucp_ac_timer_item_t *timer, int zone)
{	
#ifdef USE_TIME_MINS	
	int time_min = 0;

	time_min = timer->hour*60 + timer->minute - cl_priv->time_diff + 24*60;
	if (time_min < 24*60) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (time_min >= 48*60) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = (time_min/60)%24;
	timer->minute = time_min%60;
#else	
	int hour;
	
	hour = timer->hour + 24 - zone;
	if (hour < 24) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (hour >= 48) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = hour%24;
#endif	
}


static void _com_timer_utc_2_local(timer_switch_t* t)
{
    int hour;
    
    hour = t->hour + 24 + cl_priv->timezone;
    if (hour < 24) {
        t->week = timer_week_right_shift(t->week);
    } else if (hour >= 48) {
        t->week = timer_week_left_shift(t->week);
    }
    t->hour = hour%24;
}

static void _com_timer_local_2_utc(timer_switch_t* t)
{
    int hour;
    
    hour = t->hour + 24 - cl_priv->timezone;
    if (hour < 24) {
        t->week = timer_week_right_shift(t->week);
    } else if (hour >= 48) {
        t->week = timer_week_left_shift(t->week);
    }
    t->hour = hour%24;
}

static void period_timer_utc_2_local(net_period_timer_t *timer, int zone)
{
#ifdef USE_TIME_MINS	
	int time_min = 0;

	time_min = timer->hour*60 + timer->minute + cl_priv->time_diff + 24*60;
	if (time_min < 24*60) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (time_min >= 48*60) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = (time_min/60)%24;
	timer->minute = time_min%60;
#else	
	int hour;

	hour = timer->hour + 24 + zone;
	if (hour < 24) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (hour >= 48) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = hour%24;
#endif	
}

void period_timer_local_2_utc(net_period_timer_t *timer, int zone)
{	
#ifdef USE_TIME_MINS	
	int time_min = 0;

	time_min = timer->hour*60 + timer->minute - cl_priv->time_diff + 24*60;
	if (time_min < 24*60) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (time_min >= 48*60) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = (time_min/60)%24;
	timer->minute = time_min%60;
#else
	int hour;
	
	hour = timer->hour + 24 - zone;
	if (hour < 24) {
		timer->week = timer_week_right_shift(timer->week);
	} else if (hour >= 48) {
		timer->week = timer_week_left_shift(timer->week);
	}
	timer->hour = hour%24;
#endif	
}

static int calc_delta_time(int today_sec, u_int8_t exe_day,u_int8_t exe_hour,u_int8_t exe_min)
{
    int delta = 0;
    if(exe_day >= 1){
        exe_day-=1;
        delta = 86400 - today_sec + exe_day* 86400 + exe_hour*3600 + exe_min*60;
    }else{
        delta = ((int)(exe_hour*3600) + (int)exe_min*60) - today_sec;
    }
    
    return delta;
}

//调整时间
static void ajust_ac_timer_next_day(ucp_ac_timer_t* uat)
{
    time_t now;
    struct tm local_tm = {0};
    int zone,delta_sec,utc_today_sec,local_today_sec;
	int time_min = 0;
    
    //两个都无效，无需矫正
    if (!uat->on_effect && ! uat->off_effect || cl_priv->timezone == 0) {
        return;
    }
    
    //计算当前utc时间
    zone = cl_priv->timezone;
    now = time(NULL);
    
    //utc当天过了多少秒
    utc_today_sec = (int)(now%86400);
#ifdef USE_TIME_MINS
    local_today_sec = (int)((cl_priv->time_diff*60 + now) % 86400);
#else
    local_today_sec = (int)((zone*3600 + now) % 86400);
#endif
    
    
    if (uat->off_effect) {
        //定时器还有多久执行
        delta_sec = calc_delta_time(utc_today_sec,uat->next_off_day,uat->next_off_hour,uat->next_off_min);
        //加上现在时间
        delta_sec+=local_today_sec;
        //看下是否超过今天了
        uat->next_off_day = delta_sec/86400;
    }
    
    if (uat->on_effect) {
        //定时器还有多久执行
        delta_sec = calc_delta_time(utc_today_sec,uat->next_on_day,uat->next_on_hour,uat->next_on_min);
        //加上现在时间
        delta_sec+=local_today_sec;
        //看下是否超过今天了
        uat->next_on_day = delta_sec/86400;
    }

#ifdef USE_TIME_MINS
	//计算本地时分
	//next on
	time_min= uat->next_on_hour*60 + uat->next_on_min;
	time_min += cl_priv->time_diff + 24*60;
	uat->next_on_hour = (time_min/60)%24;
	uat->next_on_min = time_min%60;
	//next off
	time_min= uat->next_off_hour*60 + uat->next_off_min;
	time_min += cl_priv->time_diff + 24*60;
	uat->next_off_hour = (time_min/60)%24;
	uat->next_off_min = time_min%60;
#endif	    
}

//调整时间
static void ajust_perioid_timer_next_day(period_timer_head_t* uat)
{
    time_t now;
    struct tm local_tm = {0};
    int zone,delta_sec,utc_today_sec,local_today_sec;
	int time_min = 0;
    
    //两个都无效，无需矫正
    if (!uat->on_valid && ! uat->off_valid || cl_priv->timezone == 0) {
        return;
    }
    
    //计算当前utc时间
    zone = cl_priv->timezone;
    now = time(NULL);
    
    //utc当天过了多少秒
    utc_today_sec = (int)(now%86400);
    
#ifdef USE_TIME_MINS
    local_today_sec = (int)((cl_priv->time_diff*60 + now) % 86400);
#else
    local_today_sec = (int)((zone*3600 + now) % 86400);
#endif
    
    if (uat->off_valid) {
        //定时器还有多久执行
        delta_sec = calc_delta_time(utc_today_sec,uat->off_day,uat->off_hour,uat->off_min);
        //加上现在时间
        delta_sec+=local_today_sec;
        //看下是否超过今天了
        uat->off_day = delta_sec/86400;
    }
    
    if (uat->on_valid) {
        //定时器还有多久执行
        delta_sec = calc_delta_time(utc_today_sec,uat->on_day,uat->on_hour,uat->on_min);
        //加上现在时间
        delta_sec+=local_today_sec;
        //看下是否超过今天了
        uat->on_day = delta_sec/86400;
    }
#ifdef USE_TIME_MINS
	//计算本地时分
	//next on
	time_min= uat->on_hour*60 + uat->on_min;
	time_min += cl_priv->time_diff + 24*60;
	uat->on_hour = (time_min/60)%24;
	uat->on_min = time_min%60;
	//next off
	time_min= uat->off_hour*60 + uat->off_min;
	time_min += cl_priv->time_diff + 24*60;
	uat->off_hour = (time_min/60)%24;
	uat->off_min = time_min%60;
#endif	
}


//此前已经检查了合法性
void air_net_timer2_cl_timer(ucp_obj_t* obj,cl_air_timer_info_t* cati)
{
    cl_air_timer_t * items,*single;
    int i,count;
    ucp_ac_timer_t* uat = (ucp_ac_timer_t*)(obj+1);
    ucp_ac_timer_item_t* ti = (ucp_ac_timer_item_t*)(uat+1);
    struct tm cur_tm = {0};
    time_t now;
    
    now = time(NULL);
    localtime_r(&now, &cur_tm);
    
    count = (obj->param_len-sizeof(ucp_ac_timer_t))/sizeof(ucp_ac_timer_item_t);
    if (count>0) {
        items = cl_calloc(sizeof(ucp_ac_timer_item_t), count);
        if (!items) {
            return ;
        }
        
        cati->timers = items;
        cati->timer_count = count;
        single = items;
        for (i = 0; i<count; i++,single++,ti++) {
		airplug_timer_utc_2_local(ti,cl_priv->timezone);
            single->enable = ti->enable;
            single->hour = ti->hour;
            single->id = ti->id;
            single->minute = ti->minute;
            single->onoff = (ti->onoff == AC_POWER_ON)?true:false;
            single->week = ti->week;
			single->repeat = ti->repeat;
            log_debug("timer [%u] id[%u] enable[%u] week[%u] hour[%u] minu[%u] onoff[%u] repeat[%u]\n",
                      i,single->id,single->enable,single->week,single->hour,single->minute,single->onoff, single->repeat);
        }
        
    }else{
        cati->timers = NULL;
        cati->timer_count = 0x0;
    }
    
    ajust_ac_timer_next_day(uat);
    
    cati->on_effect = uat->on_effect;
    cati->off_effect = uat->off_effect;
    cati->next_on_day = uat->next_on_day;
    
    if(cati->on_effect){
#ifdef USE_TIME_MINS
		cati->next_on_hour= uat->next_on_hour;
#else
        cati->next_on_hour= airplug_hour_utc_2_local(uat->next_on_hour, uat->next_on_min, cl_priv->timezone);
#endif
   }
    
    cati->next_on_min = uat->next_on_min;
    cati->next_off_day = uat->next_off_day;

   if(cati->off_effect){
#ifdef USE_TIME_MINS
       cati->next_off_hour = uat->next_off_hour;
#else
       cati->next_off_hour = airplug_hour_utc_2_local(uat->next_off_hour, uat->next_off_min, cl_priv->timezone);
#endif
   }
  
    cati->next_off_min = uat->next_off_min;
    
    log_debug("timer on_effect[%u] day[%u] hour[%u] min[%u] off_effect[%u] day[%u] hour[%u] min[%u]\n",
              cati->on_effect,cati->next_on_day,cati->next_on_hour,cati->next_on_min,
              cati->off_effect,cati->next_off_day,cati->next_off_hour,cati->next_off_min
              );
    
}

static int _get_timer_count_from_obj(ucp_obj_t* obj,bool is_ext)
{
    int count  = 0;
    period_timer_head_t* uat = (period_timer_head_t*)(obj+1);
    net_period_timer_t* ti = (net_period_timer_t*)(uat+1);
    int len,t_len;
    
    if (obj->param_len == sizeof(period_timer_head_t)) {
        return 0;
    }
    
    if (! is_ext) {
        count = (obj->param_len-sizeof(period_timer_head_t))/sizeof(net_period_timer_t);
    }else{
        len = obj->param_len - sizeof(period_timer_head_t);
        
        while (len >= sizeof(*ti)) {
            t_len = sizeof(*ti) + (ntohl(ti->ext_data_len));

			if (t_len > len) {
				return 0;
			}
			
            len-=t_len;
            ti = (net_period_timer_t*)(((char*)ti)+t_len);
            count++;
        }
        
    }
    
    return count;
}

void net_period_timer2_cl_period_timer(smart_air_ctrl_t* ac,ucp_obj_t* obj,bool is_ext,cl_air_timer_info_t* cati)
{
    cl_period_timer_t * items,*single;
    int i,count,t_len;
    period_timer_head_t* uat = (period_timer_head_t*)(obj+1);
    net_period_timer_t* ti = (net_period_timer_t*)(uat+1);
    struct tm cur_tm = {0};
    time_t now;
    
    now = time(NULL);
    localtime_r(&now, &cur_tm);
    
    count = _get_timer_count_from_obj(obj,is_ext);
    if (count>0) {
        items = cl_calloc(sizeof(cl_period_timer_t), count);
        if (!items) {
            return ;
        }
        
        cati->period_timers= items;
        cati->timer_count = count;
        single = items;
        for (i = 0; i<count; i++,single++) {
            
		period_timer_utc_2_local(ti,cl_priv->timezone);
            single->enable = ti->enable;
            single->hour = ti->hour;
            single->id = ti->id;
            single->minute = ti->minute;
            single->onoff = (ti->onoff == AC_POWER_ON)?true:false;
            single->week = ti->week;
            single->duration = ntohs(ti->duration);
            
            if (is_ext) {
                ti->ext_data_len = ntohl(ti->ext_data_len);
                udp_updata_ext_peroid_timer_by_ext_data(ac,(void*)(ti+1),ti->ext_data_len,single);
                t_len = sizeof(*ti) + ti->ext_data_len;
                 ti = (net_period_timer_t*)(((char*)ti)+t_len);
            }else{
                ti++;
            }
            log_debug("timer [%u] id[%u] enable[%u] week[%u] hour[%u] minu[%u] onoff[%u] duration[%u]\n",
                      i,single->id,single->enable,single->week,single->hour,single->minute,single->onoff, single->duration);
        }
        
    }else{
        cati->timers = NULL;
        cati->timer_count = 0x0;
    }
    
    ajust_perioid_timer_next_day(uat);
    cati->on_effect = uat->on_valid;
    cati->off_effect = uat->off_valid;
    
    cati->next_on_day = uat->on_day;
   //少于1天时，utc 时间转换到本地时间，
   if(cati->on_effect){
#ifdef USE_TIME_MINS   	
		cati->next_on_hour = uat->on_hour;
#else
       cati->next_on_hour = airplug_hour_utc_2_local(uat->on_hour, uat->on_min, cl_priv->timezone);
#endif
   }
    
    cati->next_on_min = uat->on_min;
    
    cati->next_off_day = uat->off_day;
    
   if(cati->off_effect){
#ifdef USE_TIME_MINS 
		cati->next_off_hour = uat->off_hour;
#else
		cati->next_off_hour = airplug_hour_utc_2_local(uat->off_hour, uat->off_min, cl_priv->timezone);
#endif
   }
    cati->next_off_min = uat->off_min;
    
    log_debug("timer on_effect[%u] day[%u] hour[%u] min[%u] off_effect[%u] day[%u] hour[%u] min[%u]\n",
              cati->on_effect,cati->next_on_day,cati->next_on_hour,cati->next_on_min,
              cati->off_effect,cati->next_off_day,cati->next_off_hour,cati->next_off_min
              );
    
}

static void air_do_code_match_result_parse(smart_air_ctrl_t* air_ctrl,u_int8_t action)
{
    int event = 0;
    //用户未启动match时，timer不启动
    if (!air_ctrl ) {
        return;
    }
    
    switch (air_ctrl->match_stat.action) {
        case AIR_CM_STOP_MATCH:
            if (air_ctrl->match_stat.error == ERR_NONE) {
                event = SAE_CODE_MATCH_STOP_OK;
            }else{
                event = SAE_CODE_MATCH_STOP_FAILED;
            }
            break;
        case AIR_CM_ALL_MATCH://全匹配
            if (air_ctrl->match_stat.error == ERR_NONE) {
                if(air_ctrl->cur_match_step != AIR_CMS_IDLE){
                     air_code_match_switch_to_status(air_ctrl,AIR_CMS_WAIT_CLOUD_MATCH_RESULT);
                }
                
                if (action == UCA_PUSH) {
                    event = SAE_CODE_MATCH_STAT_MODIFY;
                }else{
                    event = SAE_CODE_MATCH_DEV_READY_OK;
                }
            }else{
                air_code_match_switch_to_status(air_ctrl,AIR_CMS_IDLE);
                if (air_ctrl->match_stat.error == ERR_CM_PROCESS_MATCHING) {
                    event = SAE_CODE_MATCH_START_FAILED;
                }else{
                    event = SAE_CODE_MATCH_FAILED;
                }
            }
            break;
        case AIR_CM_MATCH_OK:
        {
            event = SAE_CODE_MATCH_OK;
			air_ctrl->air_info.is_match_code_valid = true;
			air_ctrl->air_info.is_match_code = true;
            air_code_match_switch_to_status(air_ctrl,AIR_CMS_IDLE);
        }
            break;
        case AIR_CM_MATCH_FAILED:
        {
			air_code_match_switch_to_status(air_ctrl,AIR_CMS_IDLE);
            event = SAE_CODE_MATCH_FAILED;
            log_debug("receive Dev error info ! send event to user!\n");
        }
            break;
        case AIR_CM_CLOUD_MATCH: // 云匹配
        {
            if (air_ctrl->match_stat.error == ERR_NONE) {
                if (air_ctrl->match_stat.is_cloud_matching == 0) {
                    event = SAE_CODE_MATCH_DEV_READY_OK;
                    if(air_ctrl->cur_match_step != AIR_CMS_IDLE){
                        air_code_match_switch_to_status(air_ctrl,AIR_CMS_WAIT_USER_IR_SIGNAL);
                    }
                }else{
                    if(air_ctrl->cur_match_step != AIR_CMS_IDLE){
                        air_code_match_switch_to_status(air_ctrl,AIR_CMS_WAIT_CLOUD_MATCH_RESULT);
                    }
                    
                    if (air_ctrl->match_stat.max_step > 0 && air_ctrl->match_stat.cur_step == 0) {
                        event = SAE_CODE_MATCH_DEV_RECV_CODE;	
                    }else{
                        event = SAE_CODE_MATCH_STAT_MODIFY;
                    }
                }
            }else{
                log_debug("receive Dev error info ! send event to user!\n");
                event = SAE_CODE_MATCH_START_FAILED;
                air_code_match_switch_to_status(air_ctrl,AIR_CMS_IDLE);
            }
        }
            break;
            
        default:
            break;
    }
    log_debug("air_do_code_match_result_parse event %u action %u is_code_match %u cur_step %u max_step %u\n",
              event,air_ctrl->match_stat.action,air_ctrl->match_stat.is_cloud_matching,air_ctrl->match_stat.cur_step,
              air_ctrl->match_stat.max_step);
    
    if (event != 0) {
        event_push(air_ctrl->sac->user->callback, event, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);
        event_cancel_merge(air_ctrl->sac->user->handle);
    }
    
}

/*update product test data*/
static bool air_update_pt_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	ucp_pt_get_vol_t *vol;
	ucp_pt_get_cur_t *cur;
	ir_adjust_t *pladjust;

	switch(obj->sub_objct){
		case UCSOT_DEVPRO_IR:
			if (is_except_attr(obj, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_VOLKADKB)) {
				if(obj->param_len >= sizeof(*vol)){
					vol = (ucp_pt_get_vol_t *)(obj + 1);
					air_ctrl->pt_stat.vol_ad = ntohs(vol->ad);
				}
			}else if(is_except_attr(obj, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_CURKB)) {
				if(obj->param_len >= sizeof(*cur)){
					cur = (ucp_pt_get_cur_t *)(obj + 1);
					air_ctrl->pt_stat.cur_ad = ntohs(cur->ad);
					air_ctrl->pt_stat.cur_ad2 = ntohs(cur->ad2);
				}
			} else if (is_except_attr(obj, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_ADJUST)) {
				if(obj->param_len >= sizeof(*pladjust)){
					pladjust = (ir_adjust_t *)(obj + 1);
					air_ctrl->air_info.adkb.ck = htonl(pladjust->cur_k);
					air_ctrl->air_info.adkb.cad = htonl(pladjust->cur_ad);
					air_ctrl->air_info.adkb.cad2 = htonl(pladjust->cur_ad2);
					air_ctrl->air_info.adkb.vk = htonl(pladjust->val_k);
					air_ctrl->air_info.adkb.vb = htonl(pladjust->val_b);
				}
			}
			return true;
		break;
			
		default:
		break;
		
	}
	return false;
}

static bool curve_check(ucp_obj_t* obj)
{
	int len = 0;
	int remain_len = obj->param_len;
	cl_temp_curve_t *pcurve = (cl_temp_curve_t *)(obj+1);

	while(remain_len > (int)sizeof(cl_temp_curve_t) &&
		remain_len >= (int)(sizeof(cl_temp_curve_t) + pcurve->count*sizeof(tmp_curve_t))) {
		remain_len -= (int)(sizeof(cl_temp_curve_t) + pcurve->count*sizeof(tmp_curve_t));
	}

	if (remain_len == 0) {
		return true;
	}

	//log_debug("error len param_len=%u remain_len=%u\n", obj->param_len, remain_len);
	
	return false;
}

void sa_comm_timer_query(ucc_session_t *s)
{
	u_int16_t len;
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	uc_tlv_t *ptlv = (uc_tlv_t *)&uo[1];

	ptlv->type = htons(1);
	ptlv->len = 0;
	
	len = sizeof(uc_tlv_t);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_COM_TIMER_PROC, len);
	sa_ctrl_obj_value(s, UCA_GET, false, 0x1, uo, sizeof(ucp_obj_t) + len);	
}

static void _comm_timer_query(user_t *user, u_int8_t id)
{
	u_int16_t len;
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	uc_tlv_t *ptlv = (uc_tlv_t *)&uo[1];
	u_int8_t *pid = (u_int8_t *)&ptlv[1];

	ptlv->type = htons(2);
	ptlv->len = htons(1);
	*pid = id;

	len = sizeof(uc_tlv_t) + 1;
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_COM_TIMER_PROC, len);
	sa_ctrl_obj_value(user->uc_session, UCA_GET, false, 0x1, uo, sizeof(ucp_obj_t) + len);
}

static bool _comm_update_timer_summary(smart_air_ctrl_t* air_ctrl, uc_tlv_t *ptlv)
{
	int i;
	user_t *user = air_ctrl->sac->user;
	net_dev_timer_summary_ext_t *pnts = (net_dev_timer_summary_ext_t *)&ptlv[1];
	cl_comm_timer_head_t *pcthl = &air_ctrl->com_udp_dev_info.comm_timer_head;
	cl_dev_timer_summary_t *pcts = &air_ctrl->com_udp_dev_info.timer_summary;


	//更新摘要
	pcts->max_timer_count = pnts->max_timer_count;
	//先处理一下，都先只支持32个定时器算了，用不着那么多
	if (pcts->max_timer_count > 32) {
		pcts->max_timer_count = 32;
	}
	pcts->support_type = htons(pnts->support_type);
	pcts->max_data_len = pnts->max_data_len;
	pcts->map = htonl(pnts->map);

	//清除上次定时器
	pcthl->timer_count = 0;
	SAFE_FREE(pcthl->timer);
	log_debug("_comm_update_timer_summary map=%x\n", pcts->map);
	//发送查询报文
	for(i = 0; i < 32; i++) {
		if (pcts->map&BIT(i)) {
			_comm_timer_query(user, i+1);
		}
	}

	return true;
}

cl_comm_timer_t *comm_timer_find_by_id(cl_comm_timer_head_t *pcth, u_int8_t id)
{
	int i;

	if (pcth->timer_count == 0 ||
		!pcth->timer) {
		pcth->timer_count = 0;
		return NULL;
	}

	for(i = 0; i < pcth->timer_count; i++) {
		if (pcth->timer[i].id == id) {
			return &pcth->timer[i];
		}
	}

	return NULL;
}

static bool _comm_update_timer_adq(smart_air_ctrl_t* air_ctrl, uc_tlv_t *ptlv)
{
	user_t *user = air_ctrl->sac->user;
	cl_comm_timer_head_t *pcth = &air_ctrl->com_udp_dev_info.comm_timer_head;
	ucp_comm_timer_t *ptimer = (ucp_comm_timer_t *)&ptlv[1];
	cl_comm_timer_t *pctimer = NULL;
	cl_comm_timer_t *pctimer_tmp = NULL;
	int len = 0;

	if (ptlv->len < sizeof(ucp_comm_timer_t) ||
		(ptlv->len  < sizeof(ucp_comm_timer_t) + ptimer->extened_len)) {
		return false;
	}

	if (ptimer->id== 0) {
		return false;
	}
	pctimer = comm_timer_find_by_id(pcth, ptimer->id);
	if (pctimer) {
		ucp_comm_update_timer_modify_pkt(user,false, 0, ptimer, pctimer);
		comm_timer_next_cal(pcth);
		return true;
	}

	len = (pcth->timer_count + 1)*sizeof(cl_comm_timer_t);	
	pctimer_tmp = cl_calloc(len, 1);
	if (!pctimer_tmp) {
		return false;
	}
	memcpy((void *)pctimer_tmp, (void *)pcth->timer, len - sizeof(cl_comm_timer_t));
	SAFE_FREE(pcth->timer);
	pcth->timer = pctimer_tmp;
	ucp_comm_update_timer_modify_pkt(user, false, 0, ptimer, &pcth->timer[pcth->timer_count]);
	pcth->timer_count++;
	comm_timer_next_cal(pcth);
	pcth->comm_timer_valid = true;

	return true;
}

static bool _comm_update_timer_del_push(smart_air_ctrl_t* air_ctrl, uc_tlv_t *ptlv)
{
	user_t *user = air_ctrl->sac->user;
	cl_comm_timer_head_t *pcth = &air_ctrl->com_udp_dev_info.comm_timer_head;
	cl_comm_timer_t *pctimer = NULL;
	u_int8_t id = *(u_int8_t *)&ptlv[1];
	int len = 0;

	if (ptlv->len == 0) {
		return false;
	}

	pctimer = comm_timer_find_by_id(pcth, id);
	if (pctimer) {
		pctimer->valid = 0;
		comm_timer_next_cal(pcth);
	}	

	return true;
}

static bool _comm_timer_update_proc(smart_air_ctrl_t* air_ctrl, ucp_obj_t* obj)
{
	uc_tlv_t *ptlv = (uc_tlv_t *)&obj[1];
	cl_comm_timer_head_t *pcth = &air_ctrl->com_udp_dev_info.comm_timer_head;
	
	if (obj->param_len < sizeof(uc_tlv_t)) {
		return false;
	}

	ptlv->type = htons(ptlv->type);
	ptlv->len = htons(ptlv->len);
	if (obj->param_len < (sizeof(uc_tlv_t) + ptlv->len)) {
		log_debug("enter %s paralen=%u\n", __FUNCTION__, obj->param_len);
		return false;
	}
	log_debug("enter %s paralen=%u ptlv->type=%u\n", __FUNCTION__, obj->param_len, ptlv->type);	
	switch(ptlv->type) {
	case 1:
		_comm_update_timer_summary(air_ctrl, ptlv);
		break;
	case 2:
		_comm_update_timer_adq(air_ctrl, ptlv);
		break;
	case 3:
		_comm_update_timer_del_push(air_ctrl, ptlv);
		break;
	default:
		break;
	}

	// TODO:计算下定时器实时个数
	comm_timer_count_cal(pcth);

	return true;
}

static bool do_public_attr(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	switch(obj->attr)
	{
		case UCAT_IA_PUBLIC_PERIOD_TIMER:
			if (obj->param_len >= sizeof(period_timer_head_t) &&
			(obj->param_len-sizeof(period_timer_head_t))%sizeof(net_period_timer_t) == 0) {

				air_ctrl->air_info.air_timer_info.timer_count = 0;
				SAFE_FREE(air_ctrl->air_info.air_timer_info.timers);
				SAFE_FREE(air_ctrl->air_info.air_timer_info.period_timers);
				net_period_timer2_cl_period_timer(air_ctrl,obj,false,&air_ctrl->air_info.air_timer_info);
				log_debug("period timer OK\n");
			}
			return true;
			break;
        case UCAT_IA_PUBLIC_EXT_PERIOD_TIMER:
            if (obj->param_len >= sizeof(period_timer_head_t)){
                air_ctrl->air_info.air_timer_info.timer_count = 0;
                SAFE_FREE(air_ctrl->air_info.air_timer_info.timers);
                SAFE_FREE(air_ctrl->air_info.air_timer_info.period_timers);
                net_period_timer2_cl_period_timer(air_ctrl,obj,true,&air_ctrl->air_info.air_timer_info);
                return true;
            }
            break;
		case UCAT_IA_TMP_CURVE:
			if (obj->param_len >= sizeof(cl_temp_curve_t) && !air_ctrl->air_info.has_utc_temp_curve_data) {
				if (!curve_check(obj)) {
					//log_debug("curve check failed obj->param_len=%u\n", obj->param_len);
					break;
				}
				
				if (air_ctrl->air_info.temp_curve) {
					SAFE_FREE(air_ctrl->air_info.temp_curve);
				}
				
				air_ctrl->air_info.temp_curve = cl_calloc(obj->param_len, 1);
				if (air_ctrl->air_info.temp_curve) {
					air_ctrl->air_info.temp_curve_len = (int)obj->param_len;
					memcpy((void *)air_ctrl->air_info.temp_curve, (void *)(obj+1), obj->param_len);
					return true;
				}
			}
			break;
        case UCAT_IA_TMP_UTC_CURVE:
            if (obj->param_len >= sizeof(cl_temp_curve_t)) {
                timer_switch_t t;
                
                if (!curve_check(obj)) {
                    //log_debug("curve check failed obj->param_len=%u\n", obj->param_len);
                    break;
                }
                
                if (air_ctrl->air_info.temp_curve) {
                    SAFE_FREE(air_ctrl->air_info.temp_curve);
                }
                
                air_ctrl->air_info.temp_curve = cl_calloc(obj->param_len, 1);
                if (air_ctrl->air_info.temp_curve) {
                    air_ctrl->air_info.temp_curve_len = (int)obj->param_len;
                    memcpy((void *)air_ctrl->air_info.temp_curve, (void *)(obj+1), obj->param_len);
                    
                    t.hour = air_ctrl->air_info.temp_curve->begin_hour;
                    t.week = air_ctrl->air_info.temp_curve->week;
                    _com_timer_utc_2_local(&t);
                    air_ctrl->air_info.temp_curve->begin_hour = t.hour;
                    air_ctrl->air_info.temp_curve->week = t.week;
                    
                    t.hour = air_ctrl->air_info.temp_curve->end_hour;
                    _com_timer_utc_2_local(&t);
                    air_ctrl->air_info.temp_curve->end_hour = t.hour;
                    
                    air_ctrl->air_info.has_utc_temp_curve_data = true;
                    //return true;
                }
				// 顺便放在通用里面
				if (air_ctrl->com_udp_dev_info.temp_curve) {
                    SAFE_FREE(air_ctrl->com_udp_dev_info.temp_curve);
                }
                
                air_ctrl->com_udp_dev_info.temp_curve = cl_calloc(obj->param_len, 1);
                if (air_ctrl->com_udp_dev_info.temp_curve) {
                    air_ctrl->com_udp_dev_info.temp_curve_len = (int)obj->param_len;
                    memcpy((void *)air_ctrl->com_udp_dev_info.temp_curve, (void *)(obj+1), obj->param_len);
                    
                    t.hour = air_ctrl->com_udp_dev_info.temp_curve->begin_hour;
                    t.week = air_ctrl->com_udp_dev_info.temp_curve->week;
                    _com_timer_utc_2_local(&t);
                    air_ctrl->com_udp_dev_info.temp_curve->begin_hour = t.hour;
                    air_ctrl->com_udp_dev_info.temp_curve->week = t.week;
                    
                    t.hour = air_ctrl->com_udp_dev_info.temp_curve->end_hour;
                    _com_timer_utc_2_local(&t);
                    air_ctrl->com_udp_dev_info.temp_curve->end_hour = t.hour;
                    
                    air_ctrl->com_udp_dev_info.has_utc_temp_curve_data = true;
                }

				return true;
            }
            break;
		case UCAT_IA_COM_TIMER_PROC:
			return _comm_timer_update_proc(air_ctrl, obj);
			break;

		case UCAT_IA_PUBLIC_UTC_TMP_CTRL:
            if (is_valid_obj_data(obj, sizeof(cl_temp_ac_ctrl_t))) {
                timer_switch_t t;
                cl_temp_ac_ctrl_t* tac = OBJ_VALUE(obj, cl_temp_ac_ctrl_t*);
                
                t.hour = tac->begin_hour;
                t.week = tac->week;
                _com_timer_utc_2_local(&t);
                tac->begin_hour = t.hour;
                tac->week = t.week;
                
                t.hour = tac->end_hour;
                _com_timer_utc_2_local(&t);
                tac->end_hour = t.hour;
                
                air_ctrl->air_info.has_utc_temp_ctrl_data = true;
                memcpy((void *)&air_ctrl->air_info.tac, tac, sizeof(cl_temp_ac_ctrl_t));

				// 随便放在com info里面给其他类型设备用
				air_ctrl->com_udp_dev_info.has_utc_temp_ctrl_data = true;
                memcpy((void *)&air_ctrl->com_udp_dev_info.tac, tac, sizeof(cl_temp_ac_ctrl_t));
                
				return true;
            }
			
            break;
		case UCAT_IA_PUBLIC_SMART_ON:
			{
				if (is_obj_less_than_len(obj, sizeof(u_int8_t))) {
					return false;
				}

				air_ctrl->com_udp_dev_info.smart_on = *(OBJ_VALUE(obj, u_int8_t*));

				return true;
			}
			break;

		case UCAT_IA_PUBLIC_CHILD_LOCK:
			{
				ucp_child_lock_t* uc;
	            if (is_obj_less_than_len(obj, sizeof(ucp_child_lock_t))) {
	                break;
	            }
	            uc = OBJ_VALUE(obj, ucp_child_lock_t*);
	            air_ctrl->com_udp_dev_info.child_lock_value = uc->action;; //CHILD_LOCK_XX

				return true;
			}
			break;

		case UCAT_IA_PUBLIC_TEMP_ALARM:
			{
				ucp_temp_alarm_t* uc;
	            if (is_obj_less_than_len(obj, sizeof(ucp_temp_alarm_t))) {
	                break;
	            }
	            uc = OBJ_VALUE(obj, ucp_temp_alarm_t*);
	            air_ctrl->com_udp_dev_info.temp_alarm_onoff = uc->onoff;
				air_ctrl->com_udp_dev_info.temp_alarm_min = uc->min;
				air_ctrl->com_udp_dev_info.temp_alarm_max = uc->max;

				return true;
			}
			break;

		case UCAT_IA_PUBLIC_SHORTCUT_TIMER:
			{
				ucp_shortcuts_onoff_t *uc;

				if (is_obj_less_than_len(obj, sizeof(ucp_shortcuts_onoff_t))) {
	                break;
	            }

				uc = OBJ_VALUE(obj, ucp_shortcuts_onoff_t*);
				air_ctrl->com_udp_dev_info.shortcuts_onoff.enable = uc->enable;
				air_ctrl->com_udp_dev_info.shortcuts_onoff.onoff = uc->onoff;
				air_ctrl->com_udp_dev_info.shortcuts_onoff.remain_time = ntohs(uc->remain_time);
#ifdef USE_TIME_MINS				
				air_ctrl->com_udp_dev_info.shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->time_diff * 60;
#else
				air_ctrl->com_udp_dev_info.shortcuts_onoff.time = (u_int32_t)time(NULL) + cl_priv->timezone * 3600;
#endif

				return true;				
			}
			break;
		default:
			return udp_update_data_hook(air_ctrl,action,obj);
			break;
	}
	return false;
}

static void air_update_key_info(smart_air_ctrl_t* ac,ucp_key_t* ki)
{
	u_int8_t i ;
	cl_air_key* key;
	
	ac->air_info.key_info.key_num = 0;
	SAFE_FREE(ac->air_info.key_info.keys);

	if(ki->count > 0){
		ac->air_info.key_info.keys = cl_calloc(sizeof(cl_air_key)*ki->count,1);
		if(ac->air_info.key_info.keys != NULL){
			ac->air_info.key_info.key_num = ki->count;
			key = ac->air_info.key_info.keys;
			for(i = 0; i < ki->count;i++,key++){
				key->key_id = ki->items[i].id;
				memcpy(key->name,ki->items[i].name,MAX_KEY_NAME_LEN);
				key->name[MAX_KEY_NAME_LEN-1] = '\0';
				if((ki->items[i].flag & UCP_KEY_FLAG_SUPPORT_LEARN) != 0){
					key->is_support_learn = true;
				}
				if((ki->items[i].flag & UCP_KEY_FLAG_SUPPORT_RENAME) != 0){
					key->is_support_change_name = true;
				}
				if((ki->items[i].flag & UCP_KEY_FLAG_SUPPORT_DEL) != 0){
					key->is_support_delete = true;
				}
				if((ki->items[i].flag & UCP_KEY_FLAG_LEARNED) != 0){
					key->is_learn_code = true;
				}
				if ((ki->items[i].flag & UCP_KEY_FLAG_SNAPSHOT) != 0){
					key->is_snapshot_key = true;
				}
				if ((ki->items[i].flag & UCP_KEY_FLAG_KNOWN) != 0){
					key->is_need_decode = true;
				}

				
			}
		}
	}
}

static bool air_update_key_learn_data(smart_air_ctrl_t* ac,u_int8_t action,ucp_obj_t* obj)
{
	ucp_control_board* ucb;
	ucp_key_t* uki;
	ucp_key_learn_res_t* ukl;
	ucp_key_learn_res_v2_t *ukl2;
	
	switch(obj->attr){
		case UCAT_CONTROL_BOARD:
			ucb = OBJ_VALUE(obj, ucp_control_board*);
			if(is_obj_less_than_len(obj,sizeof(*ucb))){
				return false;
			}
			ac->sac->user->pan_type_for_808 = ucb->current_type;
			ac->air_info.current_pan_type = ucb->current_type;
			
			break;
		case UCAT_CONTROL_KEY:
			uki = OBJ_VALUE(obj, ucp_key_t*);
			if(is_obj_less_than_len(obj,sizeof(*uki))){
				return false;
			}
			air_update_key_info(ac,uki);
			break;
		case UCAT_CONTROL_LEARN:
			
			//学习中的push数据
			ukl = OBJ_VALUE(obj, ucp_key_learn_res_t*);
			ukl->error = ntohs(ukl->error);
			log_debug("push control learn err %u\n", ukl->error);
			if(ukl->error == ERR_NONE){
				
				event_push(ac->sac->user->callback, UE_INFO_MODIFY, ac->sac->user->handle, ac->sac->user->callback_handle);
				event_push(ac->sac->user->callback, SAE_LEARN_KEY_SUCCESSED, ac->sac->user->handle, ac->sac->user->callback_handle);

				if (obj->param_len >= sizeof(*ukl2)) {
					ukl2 = OBJ_VALUE(obj, ucp_key_learn_res_v2_t*);	
					ac->air_info.key_info.stat_valid = 1;
					memcpy((char*)&ac->air_info.key_info.stat, &ukl2->stat, sizeof(ac->air_info.key_info.stat));
					ac->air_info.key_info.stat.temp += AC_TEMP_BASE;
				}
				
				return false;
			}else if(ukl->error == ERR_TIMEOUT){
				event_push(ac->sac->user->callback, SAE_LEARN_KEY_WAIT_TIME_OUT, ac->sac->user->handle, ac->sac->user->callback_handle);
				return false;
			}

			break;
		default:
			return false;
			break;
	}
	return true;
}

static bool air_do_ia_stat(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    u_int32_t index;
   
    switch (obj->attr) {
        case UCAT_STAT_MILI_POWER:
            if (is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->air_info.cur_milli_power = ntohl(*((u_int32_t*)(obj+1)));
                air_ctrl->com_udp_dev_info.cur_milli_power = air_ctrl->air_info.cur_milli_power;
                log_debug("cur power [%u] OK\n",air_ctrl->air_info.cur_milli_power);
                if (air_ctrl->dev_power_timer != NULL) {
                    event_push(air_ctrl->sac->user->callback, SAE_DEV_POWER_NOTIFY, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);
                    event_cancel_merge(air_ctrl->sac->user->handle);
                    return false;
                }
            }
            break;
        case UCAT_STAT_PEAK_TIME:
        {
            cl_peak_time_t* cpt;
            if (is_valid_obj_data(obj, sizeof(cl_peak_time_t))) {
                cpt = (cl_peak_time_t*)(obj+1);
                air_ctrl->air_info.air_elec_stat_info.peak_time.begin_minute = ntohs(cpt->begin_minute);
                air_ctrl->air_info.air_elec_stat_info.peak_time.last_minute = ntohs(cpt->last_minute);
                log_debug("month peak begin[%u] last[%u] time OK\n",
                          air_ctrl->air_info.air_elec_stat_info.peak_time.begin_minute,
                          air_ctrl->air_info.air_elec_stat_info.peak_time.last_minute);
            }
        }
            break;
        case UCAT_STAT_PEAK_PRICE:
            if (is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->air_info.air_elec_stat_info.peak_price = ntohl(*((u_int32_t*)(obj+1)));
                log_debug("month peak price [%u] OK\n",air_ctrl->air_info.air_elec_stat_info.peak_price);
            }
            break;
        case UCAT_STAT_VALLEY_PRICE:
            if (is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->air_info.air_elec_stat_info.valley_price = ntohl(*((u_int32_t*)(obj+1)));
                log_debug("month valley price [%u] OK\n",air_ctrl->air_info.air_elec_stat_info.valley_price);
            }
            break;
        case UCAT_STAT_MONTH_FLAT:
        {
            u_int32_t* value;
            if (obj->param_len > sizeof(u_int32_t) && obj->param_len%sizeof(u_int32_t) == 0) {
                memset(air_ctrl->air_info.air_elec_stat_info.month_normal, 0,
                       sizeof(air_ctrl->air_info.air_elec_stat_info.month_normal));
                value = (u_int32_t*)(obj+1);
                for (index = 0; index < obj->param_len/sizeof(u_int32_t); index++,value++) {
                    air_ctrl->air_info.air_elec_stat_info.month_normal[index] = ntohl(*value);
                    log_debug("month flat index[%u]= [%u]OK\n",index,air_ctrl->air_info.air_elec_stat_info.month_normal[index]);
                }
            }
        }
            break;
        case UCAT_STAT_FLAT_TIME:
        {
            cl_peak_time_t* cpt;
            if (is_valid_obj_data(obj, sizeof(cl_peak_time_t))) {
                cpt = (cl_peak_time_t*)(obj+1);
                air_ctrl->air_info.air_elec_stat_info.flat_time.begin_minute = ntohs(cpt->begin_minute);
                air_ctrl->air_info.air_elec_stat_info.flat_time.last_minute = ntohs(cpt->last_minute);
                log_debug(" flat time begin[%u] last[%u] time OK\n",
                          air_ctrl->air_info.air_elec_stat_info.flat_time.begin_minute,
                          air_ctrl->air_info.air_elec_stat_info.flat_time.last_minute);
            }
        }
            break;
        case UCAT_STAT_VALLEY_TIME:
        {
            cl_peak_time_t* cpt;
            if (is_valid_obj_data(obj, sizeof(cl_peak_time_t))) {
                cpt = (cl_peak_time_t*)(obj+1);
                air_ctrl->air_info.air_elec_stat_info.valley_time.begin_minute = ntohs(cpt->begin_minute);
                air_ctrl->air_info.air_elec_stat_info.valley_time.last_minute = ntohs(cpt->last_minute);
                log_debug(" valley time begin[%u] last[%u] time OK\n",
                          air_ctrl->air_info.air_elec_stat_info.valley_time.begin_minute,
                          air_ctrl->air_info.air_elec_stat_info.valley_time.last_minute);
            }
        }
            break;
        case UCAT_STAT_FLAT_PRICE:
            if (is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->air_info.air_elec_stat_info.flat_price = ntohl(*((u_int32_t*)(obj+1)));
                log_debug("flat  price [%u] OK\n",air_ctrl->air_info.air_elec_stat_info.flat_price);
            }
            break;
        case UCAT_STAT_TOTAL_ELE:
            if (is_valid_obj_data(obj, sizeof(ia_stat_net_t))) {
                ia_stat_net_t* elec_stat = (ia_stat_net_t*)(obj+1);
                air_ctrl->air_info.total_elec.begin_time = ntohl(elec_stat->begin_time);
                air_ctrl->air_info.total_elec.elec = ntohl(elec_stat->ele);
				air_ctrl->com_udp_dev_info.total_elec.begin_time = ntohl(elec_stat->begin_time);
				air_ctrl->com_udp_dev_info.total_elec.elec = ntohl(elec_stat->ele);
                log_debug("Total elec time[%u] elec[%u] OK\n",air_ctrl->air_info.total_elec.begin_time,air_ctrl->air_info.total_elec.elec);
            }
            break;
        case UCAT_STAT_MONTH_PEAK:
            {
                u_int32_t* value;
                if (obj->param_len > sizeof(u_int32_t) && obj->param_len%sizeof(u_int32_t) == 0) {
                    memset(air_ctrl->air_info.air_elec_stat_info.month_peak, 0, sizeof(air_ctrl->air_info.air_elec_stat_info.month_peak));
                    value = (u_int32_t*)(obj+1);
                    for (index = 0; index < obj->param_len/sizeof(u_int32_t); index++,value++) {
                        air_ctrl->air_info.air_elec_stat_info.month_peak[index] = ntohl(*value);
						air_ctrl->com_udp_dev_info.elec_stat_info.month_peak[index] = ntohl(*value);
                        log_debug("month peak index[%u]= [%u]OK\n",index,air_ctrl->air_info.air_elec_stat_info.month_peak[index]);
                    }
                }
            }
            break;
        case UCAT_STAT_MONTH_VALLEY:
        {
            u_int32_t* value;
            if (obj->param_len > sizeof(u_int32_t) && obj->param_len%sizeof(u_int32_t) == 0) {
                memset(air_ctrl->air_info.air_elec_stat_info.month_valley, 0, sizeof(air_ctrl->air_info.air_elec_stat_info.month_valley));
                value = (u_int32_t*)(obj+1);
                for (index = 0; index < obj->param_len/sizeof(u_int32_t); index++,value++) {
                    air_ctrl->air_info.air_elec_stat_info.month_valley[index] = ntohl(*value);
                    log_debug("month valley index[%u]= [%u]OK\n",index,air_ctrl->air_info.air_elec_stat_info.month_valley[index]);
                }
                
            }
        }
        case UCAT_STAT_CUR_POWER:
            if (is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->air_info.cur_power = ntohl(*((u_int32_t*)(obj+1)));
                air_ctrl->com_udp_dev_info.current_power = air_ctrl->air_info.cur_power;
                log_debug("cur power [%u] OK\n",air_ctrl->air_info.cur_power);
                if (air_ctrl->dev_power_timer != NULL) {
                    event_push(air_ctrl->sac->user->callback, SAE_DEV_POWER_NOTIFY, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);
                    event_cancel_merge(air_ctrl->sac->user->handle);
                    return false;
                }
            }
            break;
        case UCAT_STAT_PHASE_ELE:
            if (is_valid_obj_data(obj, sizeof(ia_stat_net_t))) {
                ia_stat_net_t* elec_stat = (ia_stat_net_t*)(obj+1);
                air_ctrl->air_info.period_elec.begin_time = ntohl(elec_stat->begin_time);
                air_ctrl->air_info.period_elec.elec = ntohl(elec_stat->ele);
				air_ctrl->com_udp_dev_info.period_elec.begin_time = ntohl(elec_stat->begin_time);
				air_ctrl->com_udp_dev_info.period_elec.elec = ntohl(elec_stat->ele);
                log_debug("period_elec elec time[%u] elec[%u] OK\n",air_ctrl->air_info.period_elec.begin_time,air_ctrl->air_info.period_elec.elec);
            }
            break;
        case UCAT_STAT_ON_ELE:
            if (is_valid_obj_data(obj, sizeof(ia_stat_net_t))) {
                ia_stat_net_t* elec_stat = (ia_stat_net_t*)(obj+1);
                air_ctrl->air_info.last_on_elec.begin_time = ntohl(elec_stat->begin_time);
                air_ctrl->air_info.last_on_elec.elec = ntohl(elec_stat->ele);
				air_ctrl->com_udp_dev_info.last_on_elec.begin_time = ntohl(elec_stat->begin_time);
				air_ctrl->com_udp_dev_info.last_on_elec.elec = ntohl(elec_stat->ele);
                log_debug("last_on_elec elec time[%u] elec[%u] OK\n",air_ctrl->air_info.last_on_elec.begin_time,air_ctrl->air_info.last_on_elec.elec);
            }
            break;
        default:
            return false;
            break;
    }
    return true;
}

static bool air_do_ia_code(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    switch (obj->attr) {
        case UCAT_CODE_PROCESS:
            if (is_valid_obj_data(obj, sizeof(ucp_ac_code_match_stat_t))) {
                ucp_ac_code_match_stat_t* ms = (ucp_ac_code_match_stat_t*)(obj+1);
                air_ctrl->match_stat.action = ms->action;
                air_ctrl->match_stat.cur_step = ms->cur_step;
                air_ctrl->match_stat.error = ms->err;
                air_ctrl->match_stat.is_cloud_matching = ms->step_type?true:false;
                air_ctrl->match_stat.max_step = ms->step_num;
                air_ctrl->match_stat.flag = ms->flagbits;
                log_debug("code match action[%u] cur_step[%u] error[%u] is_match[%u] max_step[%u]\n",
                          air_ctrl->match_stat.action,
                          air_ctrl->match_stat.cur_step,
                          air_ctrl->match_stat.error,
                          air_ctrl->match_stat.is_cloud_matching,
                          air_ctrl->match_stat.max_step);
                
                air_do_code_match_result_parse(air_ctrl,action);
            }
            break;
        case UCAT_CODE_RC_INFO:
        case UCAT_CODE_RC_KEY:
        case UCAT_CODE_RC_LEARN:
        case UCAT_CODE_RC_CTRL:
        case UCAT_CODE_RC_MATCH:
            return udp_update_data_hook(air_ctrl, action, obj);
            break;
        default:
            return air_update_key_learn_data(air_ctrl,action,obj);
            break;
    }
    
    return true;
  
}

static bool com_do_ajust_update(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    uc_tlv_t *tlv;
    int remain = obj->param_len;
    int obj_len;
    
    tlv = (uc_tlv_t*)(obj+1);
    while (remain >= sizeof(uc_tlv_t) ) {
        tlv->len = ntohs(tlv->len);
        tlv->type = ntohs(tlv->type);
        
        obj_len = tlv->len+sizeof(*tlv);
        if(obj_len > remain){
            log_err(true,"Error when com_do_ajust_update obj_len = %u remain_len = %u\n",obj_len,remain);
            break;
        }
        
        remain -= obj_len;
        if(tlv->len == 0){
            tlv = tlv_next(tlv);
            continue;
        }
        
        switch (tlv->type) {
            case TLV_2_2_13_SAME_ONOFF:
                if(tlv->len >= sizeof(u_int32_t)){
                    air_ctrl->air_info.is_support_param_ajust = true;
                    air_ctrl->air_info.ajust_info.is_same_onoff_code = !!tlv_u8_value(tlv);
                }
                break;
            case TLV_2_2_13_SAME_FAN:
                if(tlv->len >= sizeof(u_int32_t)){
                    air_ctrl->air_info.is_support_param_ajust = true;
                    air_ctrl->air_info.ajust_info.is_same_fan = !!tlv_u8_value(tlv);
                }
                break;
            case TLV_2_2_13_FAN_SPEED_REVERSE:
                if(tlv->len >= sizeof(u_int32_t)){
                    air_ctrl->air_info.is_support_param_ajust = true;
                    
                    air_ctrl->air_info.ajust_info.is_fan_speed_opposite = !! tlv_u8_value(tlv);
                }
                break;
            case TLV_2_2_13_ROOM_TEMP_AJUST:
                if(tlv->len >= sizeof(u_int32_t)){
                    air_ctrl->air_info.is_support_room_temp_ajust =
                    air_ctrl->com_udp_dev_info.is_support_room_temp_ajust = true;
                    
                    air_ctrl->air_info.env_temp_ajust_value =
                    air_ctrl->com_udp_dev_info.env_temp_ajust_value = (int16_t)(*((int8_t*)(tlv+1)));
                }
                break;
            case TLV_2_2_13_ELE_AJUST:
                if(tlv->len >= sizeof(u_int32_t)){
                    air_ctrl->air_info.is_support_elec_ajust =
                    air_ctrl->com_udp_dev_info.is_support_elec_ajust = true;
                    air_ctrl->air_info.elec_ajust_value =
                    air_ctrl->com_udp_dev_info.elec_ajust_value = (int16_t)(*((u_int8_t*)(tlv+1)));
                    
                }
                break;
            default:
                break;
        }
        
        tlv = tlv_next(tlv);
    }
    return true;
}

static void _copy_24h_line_data(cl_24hour_line* cl,ucp_24hour_line* ul)
{
    if (!cl || !ul) {
        return;
    }
    
    cl->is_valid = true;
    cl->num = ul->num;
    memcpy(cl->data, ul->line, sizeof(cl->data));
}

static bool air_do_ia_ac(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    u_int32_t index;
    
    switch (obj->attr) {
        case UCAT_AC_WORK:
        {
            ucp_ac_work_t* aw;
            if (is_valid_obj_data(obj, sizeof(ucp_ac_work_t))) {
                aw = (ucp_ac_work_t*)(obj+1);
                air_ctrl->air_info.air_work_stat.mode = aw->mode;
                air_ctrl->air_info.air_work_stat.onoff = ((aw->onoff == AC_POWER_ON)?true:false);
                air_ctrl->air_info.air_work_stat.wind = aw->wind;
                air_ctrl->air_info.air_work_stat.wind_direct = aw->wind_direct;
                air_ctrl->air_info.air_work_stat.temp = aw->temp + AC_TEMP_BASE;
                
                air_ctrl->work_stat_update_ok = true;
                air_ctrl->air_info.is_work_stat_data_valid = true;
                log_debug("Air work stat mode[%u] onoff[%u] wind[%u] direct[%u] temp[%u] OK\n",
                          aw->mode,aw->onoff,aw->wind,aw->wind_direct,aw->temp);
            }
        }
            break;
        case UCAT_AC_CUR_TEMP:
            if (is_valid_obj_data(obj, sizeof(u_int8_t))) {
                air_ctrl->air_info.room_temp = *((u_int8_t*)(obj+1));
                log_debug("Room temp [%u] OK\n",air_ctrl->air_info.room_temp);
				air_ctrl->com_udp_dev_info.room_temp = air_ctrl->air_info.room_temp;
            }
            break;
        case UCAT_AC_TIMER:
            if(air_ctrl->air_info.is_support_peroid_timer||
               air_ctrl->air_info.is_support_peroid_ext_timer){
                return false;
            }
            if (obj->param_len >= sizeof(ucp_ac_timer_t) &&
                (obj->param_len-sizeof(ucp_ac_timer_t))%sizeof(ucp_ac_timer_item_t) == 0) {
                
                air_ctrl->air_info.air_timer_info.timer_count = 0;
                SAFE_FREE(air_ctrl->air_info.air_timer_info.timers);
                SAFE_FREE(air_ctrl->air_info.air_timer_info.period_timers);
                air_net_timer2_cl_timer(obj,&air_ctrl->air_info.air_timer_info);
                log_debug("Air timer OK\n");
            }
            break;
        case UCAT_STAT_SMART_ON:
            if (is_valid_obj_data(obj, sizeof(smart_on_t))) {
                smart_on_t* so = (smart_on_t*)(obj+1);
                cl_smart_air_on_param_t* ao = &air_ctrl->air_info.smart_on_info;
                
                ao->on = so->on;
                ao->push_on = so->push_on;
                ao->home_on = so->home_on;
                ao->sum_on = so->sum_on;
                ao->sum_tmp = so->sum_tmp;
                ao->win_on = so->win_on;
                ao->win_tmp = so->win_tmp;
                
                air_ctrl->air_info.smart_on_enable = ao->on;
                air_ctrl->air_info.is_smart_on_data_valid = true;
                log_debug("Smart ON CTRL OK\n");
            }
            break;
        case UCAT_STAT_SMART_OFF:
            if (is_valid_obj_data(obj, sizeof(smart_off_t))) {
                smart_off_t* sf = (smart_off_t*)(obj+1);
                cl_smart_air_off_param_t* af = &air_ctrl->air_info.smart_off_info;
                
                af->on = sf->on;
                af->push_on = sf->push_on;
                af->off_time = sf->off_time;
                
                air_ctrl->air_info.smart_off_enable = af->on;
                log_debug("Smart OFF CTRL OK\n");
            }
            break;
        case UCAT_STAT_SMART_SLEEP:
            if (is_valid_obj_data(obj, sizeof(smart_sleep_t))) {
                smart_sleep_t* ss = (smart_sleep_t*)(obj+1);
                cl_smart_air_sleep_param_t* as = &air_ctrl->air_info.smart_sleep_info;
                
                as->on = ss->on;
                
                air_ctrl->air_info.smart_sleep_enable = as->on;
                log_debug("Smart SLEEP CTRL OK\n");
            }
            break;
        case UCAT_AC_MATCH_STAT:
            if (obj->param_len >= sizeof(ucp_ac_code_match_info_t)) {
                ucp_ac_code_match_info_t* ci = (ucp_ac_code_match_info_t*)(obj+1);
                cl_ac_code_item_t * item;
                
                ci->code_num = ntohs(ci->code_num);
                SAFE_FREE(air_ctrl->air_info.last_match_info.items);
                air_ctrl->air_info.last_match_info.code_num = 0;
                air_ctrl->air_info.last_match_info.cur_match_id = ntohs(ci->cur_match_id);
                if(ci->code_num != 0 && is_valid_obj_data(obj,sizeof(ucp_ac_code_match_info_t)+sizeof(ucp_ac_code_item_t)*ci->code_num)){
                    air_ctrl->air_info.last_match_info.items = cl_calloc(sizeof(cl_ac_code_item_t)*ci->code_num,1);
                    if(air_ctrl->air_info.last_match_info.items != NULL){
                        air_ctrl->air_info.last_match_info.code_num = ci->code_num;
                        item = air_ctrl->air_info.last_match_info.items;
                        for(index = 0; index<ci->code_num;index++,item++){
                            item->c_id = ntohs(ci->items[index].c_id);
                            item->is_on = !ci->items[index].is_on;
                            item->mode = ci->items[index].mode;
                            item->temp = ci->items[index].temp;
                            item->fan = air_net_wind_2_cl_wind(ci->items[index].fan,false);
                            item->fan_dir = ci->items[index].fan_dir;
                            item->key = ci->items[index].key;
                        }
                    }
                }
                
                
                log_debug("Smart do UCAT_AC_MATCH_STAT OK\n");
            }
            break;
        case UCAT_IA_ON_USER_SET:
            return com_do_ajust_update(air_ctrl,action,obj);
            break;
        case UCAT_IA_TMP_CTRL:
            if (is_valid_obj_data(obj, sizeof(cl_temp_ac_ctrl_t)) && !air_ctrl->air_info.has_utc_temp_ctrl_data) {
                memcpy((void *)&air_ctrl->air_info.tac, (void *)(obj+1), sizeof(cl_temp_ac_ctrl_t));
            }
            break;
        case UCAT_AC_UTC_TMP_CTRL:
            if (is_valid_obj_data(obj, sizeof(cl_temp_ac_ctrl_t))) {
                timer_switch_t t;
                cl_temp_ac_ctrl_t* tac = OBJ_VALUE(obj, cl_temp_ac_ctrl_t*);
                
                t.hour = tac->begin_hour;
                t.week = tac->week;
                _com_timer_utc_2_local(&t);
                tac->begin_hour = t.hour;
                tac->week = t.week;
                
                t.hour = tac->end_hour;
                _com_timer_utc_2_local(&t);
                tac->end_hour = t.hour;
                
                air_ctrl->air_info.has_utc_temp_ctrl_data = true;
                memcpy((void *)&air_ctrl->air_info.tac, tac, sizeof(cl_temp_ac_ctrl_t));
                
            }
            break;
		case UCAT_AC_MSG_CONFIG:
		case UCAT_AC_MSG_CONFIG_V2:
			{
				cl_ac_msg_config_t *config;

				if (!is_valid_obj_data(obj, sizeof(cl_ac_msg_config_t))) {
					break;
				}

				config = OBJ_VALUE(obj, cl_ac_msg_config_t*);
				memcpy((void *)&air_ctrl->air_info.msg_config, config, sizeof(*config));
				air_ctrl->air_info.is_support_msg_config = obj->attr == UCAT_AC_MSG_CONFIG ? 1 : 2;;
			}
            break;
        case UCAT_IA_CUR_HUMIDITY:
            if (is_valid_obj_data(obj, sizeof(u_int32_t))) {
                air_ctrl->air_info.temp_humidity = obj_u8_value(obj);
				air_ctrl->com_udp_dev_info.temp_humidity = air_ctrl->air_info.temp_humidity;
            }
            break;
        case UCAT_AC_TMP_SAMPLE_CURVE:
            if (is_obj_less_than_len(obj, sizeof(ucp_24hour_line))) {
                break;
            }
            _copy_24h_line_data(&air_ctrl->air_info.room_temp_line,(ucp_24hour_line*)(obj+1));
			
			 memcpy((void *)&air_ctrl->com_udp_dev_info.room_temp_line, (void *)&air_ctrl->air_info.room_temp_line,
			 	sizeof(air_ctrl->com_udp_dev_info.room_temp_line));
            break;
        case UCAT_AC_RH_SAMPLE_CURVE:
            if (is_obj_less_than_len(obj, sizeof(ucp_24hour_line))) {
                break;
            }
             _copy_24h_line_data(&air_ctrl->air_info.humi_line,(ucp_24hour_line*)(obj+1));
			 
			 memcpy((void *)&air_ctrl->com_udp_dev_info.humi_line, (void *)&air_ctrl->air_info.humi_line,
			 	sizeof(air_ctrl->com_udp_dev_info.humi_line));
            break;
        case UCAT_AC_CHILD_LOCK:
        {
            ucp_child_lock_t* uc;
            if (is_obj_less_than_len(obj, sizeof(ucp_child_lock_t))) {
                break;
            }
            uc = OBJ_VALUE(obj, ucp_child_lock_t*);
            air_ctrl->air_info.is_support_child_lock = true;
            air_ctrl->air_info.child_lock_value = uc->action;
        }
		break;

		case UCAT_AC_ATTRI:
			{
				air_ctrl->air_info.is_match_code_valid = true;
				air_ctrl->air_info.is_match_code = *(u_int8_t*)&obj[1];
			}
			break;
		case UCAT_AC_POWERCHECK:
			{
				air_ctrl->air_info.scc_onoff_valid = true;
				air_ctrl->air_info.scc_onoff = *(u_int8_t*)&obj[1];
				air_ctrl->stat.udp_dev_stat.scc_onoff_valid = true;
				air_ctrl->stat.udp_dev_stat.scc_onoff = air_ctrl->air_info.scc_onoff;
			}
			break;
        default:
            return false;
            break;
    }
    
    return true;
}

static bool air_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
//    u_int32_t index;
    
    switch (obj->sub_objct) {
        case UCSOT_IA_STAT:
            return air_do_ia_stat(air_ctrl,action,obj);
            break;
        case UCSOT_IA_AC:
            return air_do_ia_ac(air_ctrl, action, obj);
            break;
        case UCSOT_IA_CODE:
            return air_do_ia_code(air_ctrl, action, obj);
            break;
        case UCOT_IA_FHF:
            return ah_update_ia_data(air_ctrl,action,obj); 
            break;
        case UCSOT_IA_HEATER_LCYT:
            return lcyt_update_ia_data(air_ctrl,action,obj);
            break;
        case UCOT_IA_HKAC:
            return hkac_update_ia_data(air_ctrl,action,obj);
            break;
        case UCOT_IA_NBAC:
            return nbac_update_ia_data(air_ctrl,action,obj);
            break;
        case UCOT_IA_GX_LED:
            return lamp_update_ia_data(air_ctrl, action, obj);
            break;
        case UCSOT_IA_EB:
            return eb_update_ia_data(air_ctrl, action, obj);
            break;
        case UCOT_IA_CH_BLANKET:
            return ch_blanket_update_ia_data(air_ctrl, action, obj);
            break;
        case UCSOT_IA_TT:
            return scm_update_data(air_ctrl, action, obj);
            break;
        case UCSOT_IA_PUBLIC:
            return do_public_attr(air_ctrl, action, obj);
            break;
        case UCSOT_IA_RFGW:
            return rfgw_update_ia_data(air_ctrl, action, obj);
            break;
        default:
            return udp_update_data_hook(air_ctrl,action,obj);
            break;
    }
    return false;
}

static void flash_block_order(flash_block_t *pblock)
{
	pblock->flash_addr = htonl(pblock->flash_addr);
	pblock->valid = htonl(pblock->valid);
	pblock->soft_ver = htonl(pblock->soft_ver);
	pblock->svn = htonl(pblock->svn);
	pblock->len = htonl(pblock->len);
	pblock->crc = htonl(pblock->crc);
	pblock->run = htonl(pblock->run);	
}

void do_ucot_upgrade(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	u_int32_t temp;
	u_int8_t stm_valid = 0;
	int i, value;
	ucc_session_t *s = air_ctrl->sac->user->uc_session;
	
	switch(obj->sub_objct) {
		case UCSOT_UPGRADE_FLASH:
            if (is_except_attr(obj, UCOT_UPGRADE, UCSOT_UPGRADE_FLASH, UCAT_FLASH_FLASHINFO) &&
				is_valid_obj_data(obj, sizeof(u_int32_t))) {
				temp = *(u_int32_t *)(obj+1);
                air_ctrl->sac->user->flash_size = htonl(temp);
                log_debug("flash_size %u\n",air_ctrl->sac->user->flash_size);
                return;
            }
            if (is_except_attr(obj, UCOT_UPGRADE, UCSOT_UPGRADE_FLASH, UCAT_FLASH_UPGRADE) &&
				is_valid_obj_data(obj, sizeof(air_ctrl->sac->user->block))) {
				memcpy(air_ctrl->sac->user->block, (obj+1), sizeof(air_ctrl->sac->user->block));
				for(i = 0; i < FLASH_UPGRADE_BLOCK_NUM; i++) {
					flash_block_order(&air_ctrl->sac->user->block[i]);
				}
                return;
            }	

            if (is_except_attr(obj, UCOT_UPGRADE, UCSOT_UPGRADE_FLASH, UCAT_FLASH_ERASE_STM) &&
				is_valid_obj_data(obj, sizeof(stm_valid))) {
				stm_valid = *(u_int8_t *)(obj+1);
				value = stm_valid;
				event_push_err(s->user->callback, UE_DEV_UPGRADE_STM_ERASE, s->user->handle, s->user->callback_handle, value);
				event_cancel_merge(s->user->handle);	
                return;
            }				
			break;	
	}
}


//更新数据，有变化返回true
static bool _air_update_data_with_mib(void* ctrl,u_int8_t action,ucp_obj_t* obj)
{
    smart_air_ctrl_t* air_ctrl;
    
    cl_assert(ctrl != NULL && obj != NULL);
    
    air_ctrl = (smart_air_ctrl_t*)ctrl;
    switch (obj->objct) {
        case UCOT_SYSTEM:
			air_ctrl->com_udp_dev_info.is_system_info_valid = true;
            return air_update_system_data(air_ctrl, action, obj);
            break;
        case UCOT_IA:
            return air_update_ia_data(air_ctrl,action,obj);
            break;
		case UCOT_UPGRADE:
			do_ucot_upgrade(air_ctrl,action,obj);
			break;
        case UCOT_DEVPRO:
            return air_update_pt_data(air_ctrl,action,obj);
            break;
		case UCOT_EVM:
			return evm_update_device_data(air_ctrl, action, obj);
        default:
            break;
    }
    return false;
}

bool air_update_data_with_mib(void* ctrl,u_int8_t action,ucp_obj_t* obj)
{
    smart_air_ctrl_t* air_ctrl;
	bool ret = false;
    
    cl_assert(ctrl != NULL && obj != NULL);
	
    air_ctrl = (smart_air_ctrl_t*)ctrl;
	
    ret = _air_update_data_with_mib(ctrl, action, obj);
	if (air_ctrl && 
		air_ctrl->sac && 
		air_ctrl->sac->user && 
		air_ctrl->sac->user->last_ctrl) {
		return false;
	}

	return ret;
}

static void air_hand_cloud_result(void* ctrl,ucp_obj_t* obj)
{
    smart_air_ctrl_t* air_ctrl;
    user_t* user;
    int event = 0;
	u_int16_t error;
	cl_air_code_match_stat_t *state;
	ucp_key_learn_result_t *r = (ucp_key_learn_result_t *)&obj[1];
    
    cl_assert(ctrl != NULL && obj != NULL);
    
    air_ctrl = (smart_air_ctrl_t*)ctrl;
    user = air_ctrl->sac->user;

	if (obj->sub_objct != UCSOT_IA_CODE) {
		return;
	}

	if (obj->param_len < 2) {
		return;
	}

	error = ntohs(*(u_int16_t*)&obj[1]);
    
	switch (obj->attr) {
        case UCAT_CODE_PROCESS:
            if (obj->param_len < sizeof(*state)) {
				break;
            }
			state = (cl_air_code_match_stat_t *)&obj[1];
			if (state->error == 0) {
				event = SAE_CODE_MATCH_DEV_READY_OK;
			} else {
				event = SAE_CODE_MATCH_START_FAILED;
			}
			break;

		case UCAT_CONTROL_LEARN:
			if (error = ERR_NONE) {
				event = SAE_LEARN_KEY_SUCCESSED;
				log_debug("control learn: learn successed\n");
			} else if (error == ERR_REMOTE_BUSY) {
                event = SAE_LEARN_KEY_DEV_BUSY;
				log_debug("control learn: busy\n");
            } else if(error = 0x1){
                event = SAE_LEARN_KEY_DEV_READY_OK;
				log_debug("control learn: ready\n");
            } else{
                event = SAE_COMMON_CTRL_OK;
            }
			break;
	 }

	if (event != 0) {
        event_push(user->callback, event, user->handle, user->callback_handle);
	}
}


static bool air_hand_ctrl_result(void* ctrl,ucp_obj_t* obj,u_int16_t error)
{
    smart_air_ctrl_t* air_ctrl;
    user_t* user;
    int event = 0;
    
    cl_assert(ctrl != NULL && obj != NULL);
    
    air_ctrl = (smart_air_ctrl_t*)ctrl;
    user = air_ctrl->sac->user;
    
    log_debug("recv ctrl result [%u.%u.%u] err[%u]\n",obj->objct,obj->sub_objct,obj->attr,error);

	udp_proc_ctrl_modify_hook(air_ctrl,obj,error);
	
	// 通用的虚拟机设备的属性报文
	if (obj->objct == UCOT_EVM) {
		if (obj->sub_objct == UCSOT_EVM_STAT) {
			if (error == ERR_NONE) {
          		event = SAE_COMMON_CTRL_OK;
	        } else {
	            event = SAE_COMMON_CTRL_FAILED;
	        }
		}
		
		if (event != 0) {	
	        event_push(user->callback, event, user->handle, user->callback_handle);
	        event_cancel_merge(user->handle);  
		}
		
        return true;
	}
    
    if (obj->objct == UCOT_SYSTEM) {
		if ((user->sub_type == IJ_RFGW) &&
			is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_STM_UPGRADE_PREINFO)) {
			do_rfgw_spe_up_callback(user, error);
		}
		
        if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED)) {
            if (error == ERR_NONE) {
                event = SAE_CTRL_LED_OK;
            }else{
                event = SAE_CTRL_LED_FAILED;
            }
        }
		
        if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_LICENSE, UCAT_SYS_LICENSE_ACTIVE)) {
            if (error == ERR_NONE) {
                event = UE_DEV_ACTIVE_SUCCESS;
            }else{
                event = UE_DEV_ACTIVE_FAILED;
            }
        }		

	if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED_COLOR)) {
            if (error == ERR_NONE) {
                event = SAE_CTRL_LED_COLOR_OK;
            }else{
                event = SAE_CTRL_LED_COLOR_FAILED;
            }
        }
        
        if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_HOSTNAME, UCAT_HOSTNAME_HOSTNAME)) {
            if (error == ERR_NONE) {
                event = SAE_SET_NICK_NAME_OK;
            }else{
                event = SAE_SET_NICK_NAME_FAILED;
            }
        }

        if (is_except_attr(obj, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_PASSWORD)) {
            if (error == ERR_NONE) {
                if(user->new_passwd){
                    memcpy(user->passwd_md5, user->new_passwd_md5, sizeof(user->passwd_md5));
                    user->passwd = user->new_passwd;
                    user->new_passwd = NULL;
                    fmt_hex(user->passwd_md5_str, user->passwd_md5, 16);
                }
				//把修改密码命令交给服务器来做，这样好同步。放在密码修改成功的地方去
				do_passwd_sync_to_server(user);
                user->modify_passwd_flags &= ~MPF_DEVICE;
                event = UE_MODIFY_PASSWD_OK;
            }else{
                SAFE_FREE(user->new_passwd);
                event = UE_MODIFY_PASSWD_FAIL;
            }
        }
        
        if (event != 0) {
            event_push(user->callback, event, user->handle, user->callback_handle);
            event_cancel_merge(user->handle);
        }
        
        return true;
    }
    
    
    
    switch (obj->sub_objct) {
        case UCSOT_IA_STAT:
        {
            //峰值时间段
            if (is_except_attr(obj, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_PEAK_TIME)||
                is_except_attr(obj, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_VALLEY_TIME)||
                is_except_attr(obj, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_FLAT_TIME)) {
                if (error == ERR_NONE) {
                    event = SAE_MODIFY_PERIOD_OK;
                }else{
                    event = SAE_MODIFY_PERIOD_FAILED;
                }
                break;
            }
    
            //峰值电价
            if (is_except_attr(obj, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_PEAK_PRICE)||
                is_except_attr(obj, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_VALLEY_PRICE)||
                is_except_attr(obj, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_FLAT_PRICE)) {
                if (error == ERR_NONE) {
                    event = SAE_MODIFY_PRICE_OK;
                }else{
                    event = SAE_MODIFY_PRICE_FAILED;
                }
                break;
            }
            
        }
            break;
        case UCSOT_IA_AC:
        {
            //空调状态
            if (is_except_attr(obj, UCOT_IA, UCSOT_IA_AC, UCAT_AC_WORK)) {
                if (error == ERR_NONE) {
                    event = SAE_AIR_CTRL_OK;
                }else{
                    if (error == SA_AIR_ERR_NO_MATCH) {
                        event = SAE_AIR_CTRL_NOT_MATCH;
                    }else{
                        event = SAE_AIR_CTRL_FAILED;
                    }
                }
                break;
            }
            
            //定时器
            if (is_except_attr(obj, UCOT_IA, UCSOT_IA_AC, UCAT_AC_TIMER)) {
                if (error == ERR_NONE) {
                    event = SAE_MODIFY_TIMER_OK;
                }else{
                    event = SAE_MODIFY_TIMER_FAILED;
                }
                break;
            }
            
            if (is_except_attr(obj, UCOT_IA, UCSOT_IA_AC, UCAT_STAT_SMART_ON) ||
                is_except_attr(obj, UCOT_IA, UCSOT_IA_AC, UCAT_STAT_SMART_OFF) ||
                is_except_attr(obj, UCOT_IA, UCSOT_IA_AC, UCAT_STAT_SMART_SLEEP)) {
                if (error == ERR_NONE) {
                    event = SAE_SMART_CTRL_OK;
                }else{
                    event = SAE_SMART_CTRL_FAILED;
                }
                break;
            }
            if (error == ERR_NONE) {
                event = SAE_COMMON_CTRL_OK;
            }else{
                event = SAE_COMMON_CTRL_FAILED;
            }
           
        }
            break;
        case UCSOT_IA_CODE:
        {
            switch (obj->attr) {
                case UCAT_CODE_PROCESS:
					log_debug("xdxd error = %hu\n", error);
                    if (error == ERR_NONE) {
                        event = SAE_CODE_MATCH_DEV_READY_OK;
                    }else{
                        event = SAE_CODE_MATCH_START_FAILED;
                    }
                    break;
                case UCAT_CONTROL_KEY:
                    if (error == ERR_NONE || error == 0x1) {
                        event = SAE_COMMON_CTRL_OK;
                    }
                    break;
                case UCAT_CONTROL_LEARN:
                    if (error == ERR_REMOTE_BUSY) {
                        event = SAE_LEARN_KEY_DEV_BUSY;
                    }else if(error = 0x1){
                        event = SAE_LEARN_KEY_DEV_READY_OK;
                    }else{
                        event = SAE_COMMON_CTRL_OK;
                    }
                    break;
                    
                default:
                    event = udp_proc_ctrl_hook(air_ctrl,obj,error);
                    break;
            }
           
        }
            break;
        case UCOT_IA_FHF:
        {
            event = ah_proc_ctrl_result(air_ctrl, obj, error);
        }
            break;
        case UCSOT_IA_HEATER_LCYT:
        {
            event = lcyt_proc_ctrl_result(air_ctrl, obj, error);
        }
            break;
        case UCOT_IA_HKAC:
        {
            event = hkac_proc_ctrl_result(air_ctrl, obj, error);
        }
            break;
        case UCOT_IA_NBAC:
        {
            event = nbac_proc_ctrl_result(air_ctrl, obj, error);
        }
            break;
        case UCOT_IA_GX_LED:
        {
            event = lamp_proc_ctrl_result(air_ctrl, obj, error);
        }
            break;

	case UCSOT_IA_EB:
        event = eb_proc_ctrl_result(air_ctrl, obj, error);
		break;
	case UCOT_IA_CH_BLANKET:
		event = ch_blanket_proc_ctrl_result(air_ctrl, obj, error);
		break;
	case UCSOT_IA_TT:
		event = scm_proc_ctrl_result(air_ctrl, obj, error);
		break;
	case UCSOT_IA_RFGW:
		event = rfgw_proc_ctrl_result(air_ctrl, obj, error);
		break;
	
        default:
            event = udp_proc_ctrl_hook(air_ctrl,obj,error);
            break;
    }
    
    if (event != 0) {
		log_debug("xdxd event = %d\n", event);
        event_push(user->callback, event, user->handle, user->callback_handle);
	  if(air_ctrl->sac->user->is_dev_support_scm){
	  	//单片机透传的，先来个modify
	  	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	  }
        event_cancel_merge(user->handle);
    }

    return true;
}


static RS air_init(smart_appliance_ctrl_t* sac)
{
    smart_air_ctrl_t* air_ctrl;
    
    if (sac->sub_ctrl) {
        return RS_OK;
    }
    
    if (!(air_ctrl = cl_calloc(sizeof(smart_air_ctrl_t), 1))) {
        return RS_ERROR;
    }
    
    air_ctrl->sac = sac;
    sac->sub_ctrl = air_ctrl;
    STLC_INIT_LIST_HEAD(&air_ctrl->pt_sound);
    
    return RS_OK;
}

static void air_free_pt_sound(smart_air_ctrl_t* ac)
{
	sound_link_data_t *sound, *n;

	stlc_list_for_each_entry_safe(sound_link_data_t, sound, n, &ac->pt_sound, link){
		stlc_list_del(&sound->link);
		cl_free(sound);
	}	
}

static void air_free(smart_air_ctrl_t* ac)
{
    if (ac) {
        CL_THREAD_TIMER_OFF(ac->dev_power_timer);
        CL_THREAD_TIMER_OFF(ac->common_timer);
        SAFE_FREE(ac->air_info.air_timer_info.timers);
	  SAFE_FREE(ac->air_info.air_timer_info.period_timers);
        SAFE_FREE(ac->stat.release_desc);
        SAFE_FREE(ac->stat.release_date);
        SAFE_FREE(ac->stat.release_url);
        SAFE_FREE(ac->stat.stm_release_url);
	   SAFE_FREE(ac->air_info.last_match_info.items); 
	SAFE_FREE(ac->air_info.key_info.keys); 
	SAFE_FREE(ac->air_info.temp_curve);
        SAFE_FREE(ac->stat.udp_dev_stat.stm_32_dbg_info);
        SAFE_FREE(ac->air_info.priv_rc.stb_info.uk);
       SAFE_FREE(ac->air_info.priv_rc.stb_info.fk);
        SAFE_FREE(ac->air_info.priv_rc.tv_info.uk);
        SAFE_FREE(ac->air_info.priv_rc.tv_info.fk);
        SAFE_FREE(ac->air_info.share_info.records);
        air_free_pt_sound(ac);
	  	udp_free_sdk_priv_data(&ac->com_udp_dev_info);

        cl_free(ac);
    }
}

//拷贝timer指针
void air_timer_dup(cl_air_timer_info_t* dst,cl_air_timer_info_t* src)
{
    if (!dst || !src) {
        return;
    }
    memcpy(dst, src, sizeof(*dst));
    dst->timer_count = 0x0;
    dst->timers = NULL;
    
    if (src->timer_count == 0 ) {
        return;
    }

    if(src->period_timers != NULL){
    	    dst->period_timers= cl_calloc(sizeof(cl_period_timer_t), src->timer_count);
	    if (!dst->period_timers) {
	        return;
	    }
	    
	    memcpy(dst->period_timers, src->period_timers, sizeof(cl_period_timer_t)*src->timer_count);
    }else if(src->timers != NULL){
	    dst->timers = cl_calloc(sizeof(cl_air_timer_t), src->timer_count);
	    if (!dst->timers) {
	        return;
	    }
	    
	    memcpy(dst->timers, src->timers, sizeof(cl_air_timer_t)*src->timer_count);
    } 
    
    dst->timer_count = src->timer_count;
    
}

static void copy_rc_info(cl_air_info_t* ai,smart_air_ctrl_t* ac)
{
    int len;
    memcpy(&ai->priv_rc, &ac->rc_pm.pair_rc, sizeof(ai->priv_rc));
    
    ai->priv_rc.stb_info.uk = NULL;
    ai->priv_rc.stb_info.fk = NULL;
    ai->priv_rc.stb_info.fixed_key_num = ai->priv_rc.stb_info.user_def_key_num = 0;
    
    if (ac->rc_pm.pair_rc.stb_info.user_def_key_num > 0 && ac->rc_pm.pair_rc.stb_info.uk != NULL) {
        len = sizeof(*ai->priv_rc.stb_info.uk)*ac->rc_pm.pair_rc.stb_info.user_def_key_num ;
        ai->priv_rc.stb_info.uk = cl_calloc( len , 1);
        if (!ai->priv_rc.stb_info.uk) {
            return;
        }
        memcpy(ai->priv_rc.stb_info.uk, ac->rc_pm.pair_rc.stb_info.uk , len);
        ai->priv_rc.stb_info.user_def_key_num = ac->rc_pm.pair_rc.stb_info.user_def_key_num;
    }
    
    if (ac->rc_pm.pair_rc.stb_info.fixed_key_num > 0 && ac->rc_pm.pair_rc.stb_info.fk != NULL) {
        len = sizeof(*ai->priv_rc.stb_info.fk) * ac->rc_pm.pair_rc.stb_info.fixed_key_num ;
        ai->priv_rc.stb_info.fk = cl_calloc( len , 1);
        if (!ai->priv_rc.stb_info.fk) {
            return;
        }
        memcpy(ai->priv_rc.stb_info.fk, ac->rc_pm.pair_rc.stb_info.fk , len);
        ai->priv_rc.stb_info.fixed_key_num = ac->rc_pm.pair_rc.stb_info.fixed_key_num;
    }
    
    ai->priv_rc.tv_info.uk = NULL;
    ai->priv_rc.tv_info.fk = NULL;
    ai->priv_rc.tv_info.fixed_key_num = ai->priv_rc.tv_info.user_def_key_num = 0;
    
   
    if (ac->rc_pm.pair_rc.tv_info.user_def_key_num > 0 && ac->rc_pm.pair_rc.tv_info.uk != NULL) {
        len = sizeof(*ai->priv_rc.tv_info.uk)*ac->rc_pm.pair_rc.tv_info.user_def_key_num ;
        ai->priv_rc.tv_info.uk = cl_calloc( len , 1);
        if (!ai->priv_rc.tv_info.uk) {
            return;
        }
        memcpy(ai->priv_rc.tv_info.uk, ac->rc_pm.pair_rc.tv_info.uk , len);
        ai->priv_rc.tv_info.user_def_key_num = ac->rc_pm.pair_rc.tv_info.user_def_key_num;
    }
    
    if (ac->rc_pm.pair_rc.tv_info.fixed_key_num > 0 && ac->rc_pm.pair_rc.tv_info.fk != NULL) {
        len = sizeof(*ai->priv_rc.tv_info.fk) * ac->rc_pm.pair_rc.tv_info.fixed_key_num ;
        ai->priv_rc.tv_info.fk = cl_calloc( len , 1);
        if (!ai->priv_rc.tv_info.fk) {
            return;
        }
        memcpy(ai->priv_rc.tv_info.fk, ac->rc_pm.pair_rc.tv_info.fk , len);
        ai->priv_rc.tv_info.fixed_key_num = ac->rc_pm.pair_rc.tv_info.fixed_key_num;
    }
    
}

// APP能看见的状态数据
static void air_build_objs(user_t* user,cl_dev_info_t* ui)
{
    smart_appliance_ctrl_t* sma;
    smart_air_ctrl_t* ac;
    cl_air_info_t* ai;
    
    if ( (sma = user->smart_appliance_ctrl) == NULL) {
        return;
    }
    
    if (!(ac = sma->sub_ctrl) || user->sub_type != IJ_808) {
        return;
    }
    
    ai = cl_calloc(sizeof(cl_air_info_t), 1);
    if (!ai) {
        return;
    }
    
//    if (user->name != NULL) {
//        ui->nickname = cl_strdup(user->name);
//    }
    
    memcpy(ai, &ac->air_info, sizeof(cl_air_info_t));
   //特殊处理控制面板类型
    ai->current_pan_type = user->pan_type_for_808;
   
    if(!ai->is_support_switch_pan){
        ai->is_support_switch_pan = !!user->support_set_pan;
    }

    ai->handle = user->handle;
	ai->air_work_stat.wind = air_net_wind_2_cl_wind(ai->air_work_stat.wind,ai->ajust_info.is_fan_speed_opposite);
    if (!ac->work_stat_update_ok) {
        ai->air_work_stat.onoff = false;
    }
    
    ai->share_info.records = NULL;
    if (ai->share_info.record_num > 0 ) {
        ai->share_info.records = cl_calloc(sizeof(cl_share_record_t)*ai->share_info.record_num, 1);
        memcpy(ai->share_info.records , ac->air_info.share_info.records, sizeof(cl_share_record_t)*ai->share_info.record_num);
    }

	if (ac->air_info.requested_share_code) {
		ai->requested_share_code = cl_strdup(ac->air_info.requested_share_code);
	}

    memset(&ai->elec_days_info,0,sizeof(ai->elec_days_info));
	if(ac->com_udp_dev_info.is_suppport_elec_stat){
		ai->elec_days_info.days_count  = 0;
		ai->elec_days_info.elec_data = NULL;
		ai->elec_days_info.is_info_valid = true;
		ai->elec_days_info.nearest_data_time = ac->com_udp_dev_info.elec_days_info.nearest_data_time;
		if(ac->com_udp_dev_info.elec_days_info.days_count > 0){
			ai->elec_days_info.elec_data = cl_calloc(sizeof(u_int16_t)*ac->com_udp_dev_info.elec_days_info.days_count,1);
			if(ai->elec_days_info.elec_data != NULL){
				memcpy(ai->elec_days_info.elec_data,ac->com_udp_dev_info.elec_days_info.elec_data,sizeof(u_int16_t)*ac->com_udp_dev_info.elec_days_info.days_count);
				ai->elec_days_info.days_count  = ac->com_udp_dev_info.elec_days_info.days_count;
			}
		}
	}
    
	memset(&ai->last_match_info,0,sizeof(ai->last_match_info));
	if(ac->air_info.last_match_info.items != NULL && ac->air_info.last_match_info.code_num >0){
		ai->last_match_info.items = cl_calloc(sizeof(cl_ac_code_item_t)*ac->air_info.last_match_info.code_num,1);
		if(ai->last_match_info.items != NULL){
			ai->last_match_info.code_num = ac->air_info.last_match_info.code_num;
			ai->last_match_info.cur_match_id = ac->air_info.last_match_info.cur_match_id;
			memcpy(ai->last_match_info.items,ac->air_info.last_match_info.items,sizeof(cl_ac_code_item_t)*ac->air_info.last_match_info.code_num);
		}
	}
	
	memset(&ai->key_info,0,sizeof(ai->key_info));
	if(ac->air_info.key_info.keys != NULL && ac->air_info.key_info.key_num > 0){
		ai->key_info.keys = cl_calloc(sizeof(cl_air_key)*ac->air_info.key_info.key_num,1);
		if(ai->key_info.keys != NULL){
			ai->key_info.key_num = ac->air_info.key_info.key_num;
			memcpy(ai->key_info.keys,ac->air_info.key_info.keys,sizeof(cl_air_key)*ac->air_info.key_info.key_num);
		}
	}
	ai->key_info.stat_valid = ac->air_info.key_info.stat_valid;
	memcpy((char*)&ai->key_info.stat, (char*)&ac->air_info.key_info.stat, sizeof(ai->key_info.stat));


	//通用温度曲线数据
	ai->is_support_temp_curve = ac->air_info.is_support_temp_curve;
	if (ai->is_support_temp_curve &&
		ac->air_info.temp_curve &&
		ac->air_info.temp_curve_len > 0) {
		ai->temp_curve = cl_calloc(ac->air_info.temp_curve_len, 1);
		if (ai->temp_curve) {
			memcpy((void *)ai->temp_curve, (void *)ac->air_info.temp_curve, ac->air_info.temp_curve_len);
		}
	}
	//根据温度控制空调
	ai->is_support_temp_ac_ctrl = ac->air_info.is_support_temp_ac_ctrl;
	if (ai->is_support_temp_ac_ctrl) {
		memcpy((void *)&ai->tac, (void *)&ac->air_info.tac, sizeof(ai->tac));
	}
    
    ai->env_room_temp_low = (int16_t)(ai->room_temp)- (u_int16_t)(ai->env_temp_ajust_value / 10) - 5;
    ai->env_room_temp_high = ai->env_room_temp_low + 10;

    copy_rc_info(ai,ac);
    
    
//    if (ai->is_support_peroid_ext_timer) {
//        ai->is_support_peroid_timer = false;
//    }
	
    air_timer_dup(&ai->air_timer_info,&ac->air_info.air_timer_info);
    ui->air_info = ai;
}

static void air_free_objs(cl_dev_info_t* ui)
{
    if (!ui || !ui->air_info) {
        return;
    }
   
	 SAFE_FREE(ui->air_info->elec_days_info.elec_data); 
    SAFE_FREE(ui->air_info->air_timer_info.timers);
    SAFE_FREE(ui->air_info->air_timer_info.period_timers);
    SAFE_FREE(ui->air_info);
    
}

static void air_comm_query(ucc_session_t *s)
{
	ucp_obj_t sys_objs[] = {
		{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_TIMEZONE,0},
	};

	sa_query_objects(s, sys_objs, sizeof(sys_objs)/sizeof(ucp_obj_t));
}

static void air_quick_query_ststem_stat_info(smart_air_ctrl_t* ac)
{
    ucc_session_t *s;
    ucp_obj_t sys_objs[] = {
        {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_VERSION,0},
        {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_UPTIME,0},
        {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_IP,0},
	  {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_SSID,0},
	  {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_PASSWD,0},
        {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_VERSION,0},
	 {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_COM_DATE,0},
	  {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_COM_TIME,0},
	  {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_SYSTIME,0},
	  {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_DEVSTATUS,0},
        {UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_CONNECT_TIME,0},
	  {UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_IP,0}
    };
    
    if(!ac)
		return;
	
    s = ac->sac->user->uc_session;
    sa_query_objects(s, sys_objs, sizeof(sys_objs)/sizeof(ucp_obj_t));

	air_comm_query(s);
	
    if(ac->sac->user->sub_type == IJ_SMART_PLUG||
		ac->sac->user->sub_type == IJ_101||
		ac->sac->user->sub_type == IJ_102||
		ac->sac->user->sub_type == IJ_821||
		is_supported_udp_device(ac->sac->user->sub_type,ac->sac->user->ext_type)){
        sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_DEBUGINFO);
    }
    if(ac->sac->user->sub_type == IJ_SMART_PLUG){	
        sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_LICENSE, UCAT_SYS_LICENSE_ACTIVE);
		sa_query_obj(s, UCOT_IA, UCSOT_IA_AC, UCAT_AC_POWERCHECK);
		sa_query_obj(s, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_ADJUST);
    }
	if (ac->com_udp_dev_info.is_support_disk) {
		sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_HARDWARE_DISK_INFO_GET);
	}
	if (ac->com_udp_dev_info.is_support_eth) {
		sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCAT_HARDWARE_ETH_INFO_GET);
	}
}

static void air_quick_query_pt_stat_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
	ucp_obj_t pt_objs[] = {
		{UCOT_DEVPRO,UCSOT_DEVPRO_IR,UCAT_IR_VOLKADKB,0},
		{UCOT_DEVPRO,UCSOT_DEVPRO_IR,UCAT_IR_CURKB,0},
		{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_DEBUGINFO,0},
	};
	
	if(!ac)
		return;

	s = ac->sac->user->uc_session;
	sa_query_objects(s, pt_objs, sizeof(pt_objs)/sizeof(ucp_obj_t));
}


static void air_quick_query_system_info(smart_air_ctrl_t* ac)
{
    ucc_session_t *s;
    ucp_obj_t sys_objs[] = {{UCOT_SYSTEM,UCSOT_SYS_HOSTNAME,UCAT_HOSTNAME_HOSTNAME,0},
                            {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_VERSION,0},
                            {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_UPTIME,0},
                            {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_IP,0},
                            {UCOT_SYSTEM,UCSOT_SYS_UPGRADE,UCAT_SYS_VERSION,0},
                            {UCOT_SYSTEM,UCSOT_SYS_HARDWARE,UCATT_HARDWARE_LED,0},
                            {UCOT_SYSTEM,UCSOT_SYS_VENDOR,UCAT_VENDOR_OEM_ID,0},
                            {UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_CONNECT_TIME,0},
                            {UCOT_SYSTEM,UCSOT_SYS_USER,UCAT_SYS_PASSWORD,0},
				 		    {UCOT_SYSTEM,UCSOT_SYS_HARDWARE,UCAT_SYS_VERSION,0},
							{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_COM_DATE,0},
							{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_COM_TIME,0},
							{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_SVN,0},
							{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_SSID,0},
							{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_PASSWD,0},
							{UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_IP,0},
							{UCOT_UPGRADE,UCSOT_UPGRADE_FLASH,UCAT_FLASH_UPGRADE,0},
                            {UCOT_UPGRADE,UCSOT_UPGRADE_FLASH,UCAT_FLASH_FLASHINFO,0},
							};

    if(!ac)
		return;
	
	s = ac->sac->user->uc_session;
	if(ac->sac->user->sub_type != IJ_812 && ac->sac->user->sub_type != IJ_816){
		sa_query_objects(s, sys_objs, sizeof(sys_objs)/sizeof(ucp_obj_t));
	}else{
		sa_query_objects(s, sys_objs, (sizeof(sys_objs)/sizeof(ucp_obj_t))-1);
	}
	if(ac->air_info.is_support_led_color){
		sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED_COLOR);
	}

	
	// 支持用户上下线管理
	if (ac->com_udp_dev_info.support_lanusers_manage) {
		log_debug("  query support_lanusers_manage all attri\n");
		sa_query_obj(s, UCOT_SYSTEM, UCSOT_LANUSERS_MANAGE, 0xffff);
	}

	// 支持WIFI连接状态显示
	if (ac->com_udp_dev_info.is_support_dev_wifi_state) {
		log_debug("  query is_support_dev_wifi_state \n");
		if (ac->sac->user->sub_type == IJ_RFGW && 
			ac->sac->user->ext_type == ETYPE_IJ_RFGW_S3) {
			sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_WIFI_STATE_S3);
		} else {
			sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_WIFI_STATE);
		}
	}
	
	//单独获取系统状态信息
   //
}

static cl_dev_stat_t* air_get_dev_stat_info(smart_air_ctrl_t* ac)
{
    cl_dev_stat_t* ds = cl_calloc(sizeof(*ds), 1);
    
    if (!ac || !ds) {
        return NULL;
    }
    memcpy(ds, &ac->stat, sizeof(*ds));
    
    ds->handle = ac->sac->user->handle;
    ds->sn = ac->sac->user->sn;
    ds->clients = NULL;
	memcpy(&ds->udp_dev_stat, &ac->stat.udp_dev_stat, sizeof(ds->udp_dev_stat));
	
    ds->udp_dev_stat.stm_32_dbg_info = ac->stat.udp_dev_stat.stm_32_dbg_info?cl_strdup(ac->stat.udp_dev_stat.stm_32_dbg_info):NULL;
    ds->release_date = ac->stat.release_date?cl_strdup(ac->stat.release_date):NULL;
    ds->release_url = ac->stat.release_url?cl_strdup(ac->stat.release_url):NULL;
    ds->stm_release_url = ac->stat.stm_release_url?cl_strdup(ac->stat.stm_release_url):NULL;
    ds->release_desc = ac->stat.release_desc?cl_strdup(ac->stat.release_desc):NULL;
    
    ds->ap_passwd = ac->stat.ap_passwd?cl_strdup(ac->stat.ap_passwd):NULL;
    ds->ap_ssid = ac->stat.ap_ssid?cl_strdup(ac->stat.ap_ssid):NULL;
    air_quick_query_ststem_stat_info(ac);
    
    return ds;
}

static void air_quick_query_ia_info(smart_air_ctrl_t* ac)
{
    ucc_session_t *s;
    ucp_obj_t ia_stat_objs[] = {{UCOT_IA,UCSOT_IA_STAT,0xFFFF,0}};
    ucp_obj_t ia_ac_objs[] = {{UCOT_IA,UCSOT_IA_AC,0xFFFF,0}};

   if(!ac)
       return;
    
    s = ac->sac->user->uc_session;
    log_debug("query ia object\n");

    sa_query_objects(s, ia_stat_objs, sizeof(ia_stat_objs)/sizeof(ucp_obj_t));
    sa_query_objects(s, ia_ac_objs, sizeof(ia_ac_objs)/sizeof(ucp_obj_t));

   if(ac->air_info.is_support_switch_pan){
	 sa_query_obj(s, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_BOARD);
	 sa_query_obj(s, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY);
   }

	if (ac->air_info.is_support_key_learn){
	   sa_query_obj(s, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY);
	}
    
    if(ac->sac->user->sub_type == IJ_SMART_PLUG ||
	ac->sac->user->sub_type == IJ_101 ||
	ac->sac->user->sub_type == IJ_102){
        SAFE_FREE(ac->air_info.last_match_info.items);
        ac->air_info.last_match_info.code_num = 0;
        ac->air_info.last_match_info.cur_match_id = 0;
        
		if(ac->com_udp_dev_info.is_suppport_elec_stat){
		    sa_query_obj(s, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_DAYS_STAT);
		}
        sa_query_obj(s, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_DEBUGINFO);
    }
    if (ac->air_info.is_support_peroid_ext_timer) {
        sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_EXT_PERIOD_TIMER);
    }else if(ac->air_info.is_support_peroid_timer){
        log_debug("air_quick_query_ia_info query public info\n");
        sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_PERIOD_TIMER);
    }

    if (ac->air_info.is_support_utc_temp_curve) {
        sa_query_obj(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_TMP_UTC_CURVE);
    }else if (ac->air_info.is_support_temp_curve) {
		sa_query_obj(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_PUBLIC,UCAT_IA_TMP_CURVE);
	}
    
    if (ac->air_info.is_support_utc_temp_ac_ctrl) {
        sa_query_obj(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_AC,UCAT_AC_UTC_TMP_CTRL);
    }else if (ac->air_info.is_support_temp_ac_ctrl) {
		sa_query_obj(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_AC,UCAT_IA_TMP_CTRL);
    }
}

static void _quick_query_rc_info(smart_air_ctrl_t* ac)
{
    sa_query_obj(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_CODE,UCAT_CODE_RC_INFO);
    sa_query_obj(ac->sac->user->uc_session,UCOT_IA,UCSOT_IA_CODE,UCAT_CODE_RC_KEY);
}

static void quick_query_public_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s = ac->sac->user->uc_session;

	if (!ac) {
		return;
	}
	
	if (ac->com_udp_dev_info.is_support_public_utc_temp_ac_ctrl) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_UTC_TMP_CTRL);
	}

	if (ac->com_udp_dev_info.is_support_public_child_lock) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_CHILD_LOCK);
	}

	if (ac->com_udp_dev_info.is_support_public_temp_alarm) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_TEMP_ALARM);
	}

	if (ac->com_udp_dev_info.is_support_public_smart_on) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_SMART_ON);
	}

	if (ac->com_udp_dev_info.is_support_public_shortcuts_onoff) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_SHORTCUT_TIMER);
	}
	
	if (ac->com_udp_dev_info.is_support_dev_history) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_HISTORY_INFO);
	}

	if (ac->com_udp_dev_info.is_support_boot_temp) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_BOOT_TEMP);
	}
	
	if (ac->com_udp_dev_info.is_support_wan_config) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_WAN_CONFIG);
	}	

	if (ac->com_udp_dev_info.is_support_dhcp_server) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_DHCP_SERVER);
	}

	if (ac->com_udp_dev_info.is_support_ap_config) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_AP_CONFIG);
	}	

	if (ac->com_udp_dev_info.is_support_repeat) {
		sa_query_obj(s, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_REPEATER);
	}	

	
}

static void air_quick_query_all_info(smart_appliance_ctrl_t*sac)
{
    smart_air_ctrl_t* ac = sac->sub_ctrl;

	// 公共部分
	quick_query_public_info(sac->sub_ctrl);


	//避免被透传标记后不查询其他命令
	if (sac->user->sub_type == IJ_RFGW) {
		rfgw_quick_query_info(sac->sub_ctrl);
	}
	
	if(sac->user->is_dev_support_scm){
		 air_quick_query_system_info(sac->sub_ctrl);
		 scm_quick_query_info(sac->sub_ctrl);
		 return;
	}

    switch (sac->user->sub_type) {
        case IJ_SMART_PLUG://智能插座
        {
            air_quick_query_system_info(sac->sub_ctrl);
            air_quick_query_ia_info(sac->sub_ctrl);
        }
            break;

        case IJ_812://联创暖风机
        {
            air_quick_query_system_info(sac->sub_ctrl);
            ah_quick_query_ia_info(sac->sub_ctrl);
        }
            break;
        case IJ_816://联创油汀
        {
            air_quick_query_system_info(sac->sub_ctrl);
            lcyt_quick_query_info(sac->sub_ctrl);
        }
            break;
        case IJ_813:
        {
            air_quick_query_system_info(sac->sub_ctrl);
            hkac_quick_query_info(sac->sub_ctrl);
        }
            break;
        case IJ_820:
        {
            air_quick_query_system_info(sac->sub_ctrl);
            lamp_quick_query_info(sac->sub_ctrl);
        }
            break;
        case IJ_101:
        case IJ_102:
            air_quick_query_system_info(sac->sub_ctrl);
            eb_quick_query_info(sac->sub_ctrl);
            break;
        case IJ_821:
        {
            air_quick_query_system_info(sac->sub_ctrl);
            ch_blanket_quick_query_info(sac->sub_ctrl);
            break;
        }
        case IJ_RFGW:
            air_quick_query_system_info(sac->sub_ctrl);
            rfgw_quick_query_info(sac->sub_ctrl);
            break;

        default:
            if(is_supported_udp_device(sac->user->sub_type,sac->user->ext_type)){
                air_quick_query_system_info(sac->sub_ctrl);
                udp_quick_query_info_hook(sac->sub_ctrl);
            }
            break;
    }
    if (ac && ac->com_udp_dev_info.is_support_rc) {
        _quick_query_rc_info(ac);
    }

	// SDK需要知道设备连接的哪个服务器，通过SERVER_IP得到
	if (is_supported_udp_device(ac->sac->user->sub_type,ac->sac->user->ext_type)) {
        sa_query_obj(sac->user->uc_session, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SERVER_IP);
	}

	// 公共部分
	//quick_query_public_info(sac->sub_ctrl);
}

/////////////////////////////////////////////////////////////////////////////////
// 处理APP数据设置

static bool air_proc_stat_ctrl(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_ac_work_t* aw = (ucp_ac_work_t*)(uo+1);
    
    if (sai->stat.temp > 32) {
        sai->stat.temp = 32;
    }
    
    if (sai->stat.temp < AC_TEMP_BASE) {
        sai->stat.temp = AC_TEMP_BASE;
    }
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_WORK, sizeof(*aw));
    aw->onoff = sai->stat.onoff?AC_POWER_ON:AC_POWER_OFF;
    
    aw->mode = sai->stat.mode;
    aw->temp = sai->stat.temp - AC_TEMP_BASE;
    aw->wind_direct = sai->stat.wind_direct;
    aw->wind = sai->stat.wind;
    //ACTION 没用，利用来存储一下key值
    aw->key = sai->action &0xff;
//    aw->old_key_value = sai->old_key;
    //反转风力
//    if (ac->air_info.ajust_info.is_fan_speed_opposite) {
//        if (aw->wind == AC_WIND_3) {
//            aw->wind = AC_WIND_1;
//        }else if(aw->wind == AC_WIND_1){
//            aw->wind = AC_WIND_3;
//        }
//    }
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*aw));
    
    return true;
}

static bool air_proc_led_ctrl(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_led_t* led = (ucp_led_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED, sizeof(*led));
    led->user_enable = sai->led_on_off;
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*led));
    
    return true;
}

static bool air_proc_led_color_ctrl(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_led_color_t* ledc = (ucp_led_color_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_HARDWARE, UCATT_HARDWARE_LED_COLOR, sizeof(*ledc));
    ledc->color_of_ac_on = sai->share_struct[0];
    ledc->color_of_ac_off = sai->share_struct[1];
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*ledc));
    
    return true;
}

static bool air_proc_trans_send(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[2048] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int8_t *tb = (u_int8_t*)(uo+1);    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TT, UCAT_IA_TT_UARTCMD, sai->trans_data.trans_len);
    memcpy(tb, sai->trans_data.trans_buf, sai->trans_data.trans_len);
    
    sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sai->trans_data.trans_len);
    
    return true;
}

static bool air_proc_restore_factory(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_led_color_t* ledc = (ucp_led_color_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TT, UCAT_IA_TT_RESET, 0);
    
    sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo));
    
    return true;
}

static bool air_proc_pt_set_adkb(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[512] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_pt_set_cur_t* cur = (ucp_pt_set_cur_t*)(uo+1);
    ucp_obj_t* uov = (ucp_obj_t*)(cur+1);
    ucp_pt_set_vol_t* vol = (ucp_pt_set_vol_t*)(uov+1);
    cl_air_pt_adkb_t *v = (cl_air_pt_adkb_t *)sai->share_struct;

    cur->k = ntohl(v->ck);
    cur->ad= ntohs((u_int16_t)v->cad);
    cur->ad2= ntohs((u_int16_t)v->cad2);
    vol->k = ntohl(v->vk);
    vol->b = ntohl(v->vb);

    fill_net_ucp_obj(uo, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_CURKB, sizeof(*cur));
    fill_net_ucp_obj(uov, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_VOLKADKB, sizeof(*vol));
    sa_set_obj_value_only(ac->sac->user->uc_session, 0x2, buf, 
		sizeof(*uo)+sizeof(*cur)+sizeof(*uov)+sizeof(*vol));
    
    return true;
}

static bool air_proc_pt_set_adkb_ext(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	ir_adjust_t *pladjust = NULL;
	ucc_session_t *s = ac->sac->user->uc_session;
	cl_air_pt_adkb_t *v = (cl_air_pt_adkb_t *)sai->share_struct;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + sizeof(ir_adjust_t),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return false;
	}
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_ADJUST, sizeof(ir_adjust_t));
    pladjust = (ir_adjust_t *)(uo+1);
	pladjust->cur_k = htonl(v->ck);
	pladjust->cur_ad = htons((u_int16_t)v->cad);
	pladjust->cur_ad2 = htons((u_int16_t)v->cad2);	
	pladjust->val_k = htonl(v->vk);
	pladjust->val_b = htonl(v->vb);

	ucc_request_add(s, pkt);
    
    return true;
}

static bool air_proc_pt_set_adj(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	char buf[128] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_plug_pt_adkb_t* w = (cl_plug_pt_adkb_t *)(uo + 1);
	cl_plug_pt_adkb_t *v = (cl_plug_pt_adkb_t *)sai->share_struct;
	
	memset(w, 0, sizeof(*w));
	w->freq_current = htonl(v->freq_current);
	w->k_current = htonl(v->k_current);
	w->freq_voltage = htonl(v->freq_voltage);
	w->k_voltage = htonl(v->k_voltage);
	fill_net_ucp_obj(uo, UCOT_DEVPRO, UCSOT_DEVPRO_IR, UCAT_IR_ADJ, sizeof(*w));
   	 sa_set_obj_value_only(ac->sac->user->uc_session, 0x01, buf,   sizeof(ucp_obj_t)+sizeof(*w));	
	printf("air_proc_pt_set_adj: freq_current:%u, k_current:%u, freq_voltage:%u, k_voltage:%u\n", 
		v->freq_current, v->k_current, v->freq_voltage, v->k_voltage);
    return true;
}

static bool air_proc_pt_send_lcc(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai, RS* ret)
{
    char buf[512] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;

    memcpy((uo+1), sai->trans_data.trans_buf, sai->trans_data.trans_len);
	
    fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_LICENSE, UCAT_SYS_LICENSE_ACTIVE, sai->trans_data.trans_len);
    sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sai->trans_data.trans_len);
    log_info("air_proc_pt_send_lcc\n");
    
    return true;
}

static bool air_proc_peak_period(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_stat_peak_time_t* period = (ucp_stat_peak_time_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_PEAK_TIME, sizeof(*period));
    period->begin_minute = htons(sai->begin_time);
    period->last_minute = htons(sai->last_time);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*period));
    
    return true;
}

static bool air_proc_valley_period(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_stat_peak_time_t* period = (ucp_stat_peak_time_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_VALLEY_TIME, sizeof(*period));
    period->begin_minute = htons(sai->begin_time);
    period->last_minute = htons(sai->last_time);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*period));
    
    return true;
}

static bool air_proc_peak_price(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int32_t* price = (u_int32_t*)(uo+1);
    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_PEAK_PRICE, sizeof(*price));
    *price = htonl(sai->temp_value);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*price));
    
    return true;
}

static bool air_proc_valley_price(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int32_t* price = (u_int32_t*)(uo+1);
    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_VALLEY_PRICE, sizeof(*price));
    *price = htonl(sai->temp_value);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*price));
    
    return true;
}

static bool air_proc_flat_price(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int32_t* price = (u_int32_t*)(uo+1);
    
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_FLAT_PRICE, sizeof(*price));
    *price = htonl(sai->temp_value);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*price));
    
    return true;
}

static bool air_proc_timer(u_int32_t pkt_type,smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_ac_timer_item_t* timer = (ucp_ac_timer_item_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_TIMER, sizeof(*timer));
    timer->id = sai->timer_info.id;
    timer->enable = sai->timer_info.enable;
    timer->hour = sai->timer_info.hour;
    timer->minute = sai->timer_info.minute;
    timer->onoff = sai->timer_info.onoff?AC_POWER_ON:AC_POWER_OFF;
    timer->week = sai->timer_info.week;

	airplug_timer_local_2_utc(timer,cl_priv->timezone);
    
    log_debug("Proc timer id[%u] enable[%u] week[%u] hour[%u] minu[%u] onoff[%u]\n",
              timer->id,timer->enable,timer->week,timer->hour,timer->minute,timer->onoff);
    
    if (pkt_type == CLNE_SA_AIR_MODIFY_TIMER) {
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,true,0x1, buf, sizeof(*uo)+sizeof(*timer));
    }else if(pkt_type == CLNE_SA_AIR_DEL_TIMER){
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,true,0x1, buf, sizeof(*uo)+sizeof(*timer));
    }
    
    return true;
}

static bool air_proc_period_timer(u_int32_t pkt_type,smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_period_timer_t* timer = (net_period_timer_t*)(uo+1);
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_PERIOD_TIMER, sizeof(*timer));
    timer->id = sai->timer_info.id;
    timer->enable = sai->timer_info.enable;
    timer->hour = sai->timer_info.hour;
    timer->minute = sai->timer_info.minute;
    timer->onoff = sai->timer_info.onoff?AC_POWER_ON:AC_POWER_OFF;
    timer->week = sai->timer_info.week;
    timer->duration = ntohs(sai->timer_info.duration);
    

	period_timer_local_2_utc(timer,cl_priv->timezone);
    
    log_debug("Proc period timer id[%u] enable[%u] week[%u] hour[%u] minu[%u] onoff[%u]\n",
              timer->id,timer->enable,timer->week,timer->hour,timer->minute,timer->onoff);
    
    if (pkt_type == CLNE_SA_MODIFY_PERIOD_TIMER) {
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,true,0x1, buf, sizeof(*uo)+sizeof(*timer));
    }else if(pkt_type == CLNE_SA_PERIOD_DEL_TIMER){
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,true,0x1, buf, sizeof(*uo)+sizeof(*timer));
    }
    
    return true;
}

static int air_timer_code_match(cl_thread_t *t)
{
	smart_air_ctrl_t* ac;
	int event = 0;
	
    ac = (smart_air_ctrl_t*)CL_THREAD_ARG(t);
    if (!ac) {
        return 0;
    }
	
    ac->match_timer_out = NULL;
	memset(&ac->match_stat, 0x0 ,sizeof(ac->match_stat));
    
	if(ac->cur_match_step != AIR_CMS_IDLE){
		air_code_match_switch_to_status(ac,AIR_CMS_IDLE);
		ac->match_stat.error = ERR_CM_TO_DEV_TIME_OUT;
        log_debug("air_timer_code_match ! send event to user!\n");
		event_push(ac->sac->user->callback, SAE_CODE_MATCH_FAILED, ac->sac->user->handle, ac->sac->user->callback_handle);
        event_cancel_merge(ac->sac->user->handle);
	}

	return 0;
}

static void air_rest_code_match_timer(smart_air_ctrl_t* ac,u_int32_t sec_time_out)
{
	CL_THREAD_TIMER_OFF(ac->match_timer_out);

	if(sec_time_out > 0){
		CL_THREAD_TIMER_ON(&cl_priv->master, ac->match_timer_out, air_timer_code_match, (void*)ac,
                           sec_time_out*TIME_PER_SECOND);
	}
}

static void air_code_match_switch_to_status(smart_air_ctrl_t* ac,u_int32_t status)
{
	air_rest_code_match_timer(ac,0);
	
	switch(status){
		case AIR_CMS_START_ALL_MATCH:
			air_rest_code_match_timer(ac,3);
			break;
		case AIR_CMS_START_CLOUD_MATCH:
			air_rest_code_match_timer(ac,3);
			break;
		case AIR_CMS_WAIT_ALL_MATCH_RESULT:
			air_rest_code_match_timer(ac,60);
			break;
		case AIR_CMS_WAIT_CLOUD_MATCH_RESULT:
			air_rest_code_match_timer(ac,60);
			break;
		case AIR_CMS_WAIT_USER_IR_SIGNAL:
			air_rest_code_match_timer(ac,3*60+10);
			break;
		default:
			status = AIR_CMS_IDLE;
			break;
	}
	
	ac->cur_match_step = status ;
	
}

static bool air_proc_code_match(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_ac_code_match_ctrl_t* uacc = (ucp_ac_code_match_ctrl_t*)(uo+1);
    
    switch (sai->action) {
        case SA_ACT_CODE_MATCH_START:
        {
		
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_PROCESS, sizeof(*uacc));
            uacc->action = sai->code_match.mode;
            uacc->time_out = sai->code_match.time_out;
		if(sai->code_match.mode == AIR_CM_ALL_MATCH){
			air_code_match_switch_to_status(ac,AIR_CMS_START_ALL_MATCH);
		}else{
			air_code_match_switch_to_status(ac,AIR_CMS_START_CLOUD_MATCH);
		}
            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+sizeof(*uacc));
        }
            break;
        case SA_ACT_CODE_MATCH_STOP:
        {
            air_code_match_switch_to_status(ac,AIR_CMS_IDLE);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_PROCESS, sizeof(*uacc));
            uacc->action = CL_AIR_CODE_MATCH_STOP;
            
            sa_ctrl_obj_value(ac->sac->user->uc_session,UCA_GET,false, 0x1, buf, sizeof(*uo)+sizeof(*uacc));
        }
            break;
        case SA_ACT_RESET_IR_CODE:
        {
        
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_PROCESS, sizeof(*uacc));
            uacc->action = AIR_CM_SWITCH_CODE;
            uacc->new_ir_code_id = sai->temp_value & 0xFFFF;
            uacc->new_ir_code_id = ntohs(uacc->new_ir_code_id);
            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+sizeof(*uacc));
            air_code_match_switch_to_status(ac,AIR_CMS_WAIT_CLOUD_MATCH_RESULT);
        }
            break;
        default:
            break;
    }
    
    return true;
    
}

static bool air_proc_smart_ctrl(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    char buf[128] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    
    switch (sai->action) {
        case SA_ACT_SMART_POWER_ON:
        case SA_ACT_SMART_POWER_ON_DETAIL:
        {
            smart_on_t* dst_smart_on = (smart_on_t*)(uo+1);
            cl_smart_air_on_param_t* cop = (cl_smart_air_on_param_t*)sai->share_struct;
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_STAT_SMART_ON, sizeof(*dst_smart_on));
            
            if (sai->action == SA_ACT_SMART_POWER_ON) {
                cop = &ac->air_info.smart_on_info;
                dst_smart_on->on = !!sai->temp_value;
            }else{
                dst_smart_on->on = cop->on;
            }
            
            dst_smart_on->home_on = cop->home_on;
            dst_smart_on->push_on = cop->push_on;
            dst_smart_on->sum_on = cop->sum_on;
            dst_smart_on->sum_tmp = cop->sum_tmp;
            dst_smart_on->win_on = cop->win_on;
            dst_smart_on->win_tmp = cop->win_tmp;
            
        }
            break;
        case SA_ACT_SMART_POWER_OFF:
        case SA_ACT_SMART_POWER_OFF_DETAIL:
        {
            smart_off_t* dst_smart_off = (smart_off_t*)(uo+1);
            cl_smart_air_off_param_t* cfp = (cl_smart_air_off_param_t*)sai->share_struct;
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_STAT_SMART_OFF, sizeof(*dst_smart_off));
            
            if (sai->action == SA_ACT_SMART_POWER_OFF) {
                cfp = &ac->air_info.smart_off_info;
                dst_smart_off->on = !!sai->temp_value;
            }else{
                dst_smart_off->on = cfp->on;
            }
            
            dst_smart_off->push_on = cfp->push_on;
            dst_smart_off->off_time = cfp->off_time;
        }
            break;
        case SA_ACT_SMART_SLEEP:
        case SA_ACT_SMART_SLEEP_DETAIL:
        {
            smart_sleep_t* dst_smart_sleep = (smart_sleep_t*)(uo+1);
            cl_smart_air_sleep_param_t* csp = (cl_smart_air_sleep_param_t*)sai->share_struct;
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_STAT_SMART_SLEEP, sizeof(*dst_smart_sleep));
            if (sai->action == SA_ACT_SMART_SLEEP) {
                csp = &ac->air_info.smart_sleep_info;
                dst_smart_sleep->on = !!sai->temp_value;
            }else{
                dst_smart_sleep->on = csp->on;
            }
        }
            break;
        default:
            return false;
            break;
    }
    
    //参数长度字节序已转换，转回来做包长
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+ntohs(uo->param_len));

    return true;
}

static int air_timer_cur_power_query(cl_thread_t *t)
{
    smart_air_ctrl_t* ac;
    ucp_obj_t obj[2] = {{UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_CUR_POWER, 0},{UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_MILI_POWER, 0}};
    ucc_session_t *s;
    
    ac = (smart_air_ctrl_t*)CL_THREAD_ARG(t);
    if (!ac) {
        return 0;
    }
    s = ac->sac->user->uc_session;
    
    ac->dev_power_timer = NULL;
    if (ac->power_time_interval > 0) {
        CL_THREAD_TIMER_ON(&cl_priv->master, ac->dev_power_timer, air_timer_cur_power_query, (void*)ac,
                           ac->power_time_interval*TIME_PER_SECOND);
    }
    
    sa_query_objects(s, &obj[0],sizeof(obj)/sizeof(ucp_obj_t));
    
    return 0;
}

static void air_quick_cur_power(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
    if (!ac || !sai) {
        return;
    }
    
    CL_THREAD_TIMER_OFF(ac->dev_power_timer);
    
    if (sai->action == SA_ACT_STOP_CUR_POWER) {
        return;
    }
    
    ac->power_time_interval = sai->temp_value;
    if (ac->power_time_interval == 0) {
        ac->power_time_interval = 1;
    }else if(ac->power_time_interval >200){
        ac->power_time_interval = 200;
    }
    
    ac->last_get_power_time = get_sec();
    
    
    CL_THREAD_TIMER_ON(&cl_priv->master, ac->dev_power_timer, air_timer_cur_power_query, (void*)ac, 0);
}

static void air_query_elec_info(smart_air_ctrl_t* ac)
{
	ucc_session_t *s;
    ucp_obj_t stat_objs[] = {{UCOT_IA,UCSOT_IA_STAT,0xFFFF,0}};

    if(!ac)
       return;
    
    s = ac->sac->user->uc_session;

    sa_query_objects(s, stat_objs, sizeof(stat_objs)/sizeof(ucp_obj_t));
}

static void air_clear_stat_info(smart_air_ctrl_t* ac,u_int32_t type)
{
	 char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ia_stat_net_t* isn = (ia_stat_net_t*)(uo+1);
   u_int16_t attr = UCAT_STAT_PHASE_ELE;

	if(type == ELEC_CLEAR_TOTAL){
		attr = UCAT_STAT_TOTAL_ELE;
	}else if(type == ELEC_CLEAR_LAST_ON){
	      attr = UCAT_STAT_ON_ELE;
	}
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, attr, sizeof(*isn));
    isn->begin_time = (u_int32_t)time(NULL);
    isn->begin_time = htonl(isn->begin_time);
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*isn));
}


static bool air_porc_key_request(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_key_item_t*key = (ucp_key_item_t*)(uo+1);
	ucp_snapshort_key_item_t *snapshort_key = (ucp_snapshort_key_item_t*)(uo+1);
	ucp_key_ctrl_t* kc = (ucp_key_ctrl_t*)(uo+1);
	int len = 0;

	switch(sai->action){
		case SA_ACT_SET_KEY_INFO:
			len = sizeof(*key);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY, len);
			key->id = sai->temp_value & 0xFF;
			strncpy((char*)key->name,sai->share_struct,sizeof(key->name)-1);
			
			sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
			break;
		case SA_ACT_SET_KEY_INFO_V2:			
			len = sizeof(*snapshort_key);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY, len);
			snapshort_key->id = sai->temp_value & 0xFF;
			snapshort_key->flag = (u_int8_t)sai->begin_time;
			strncpy((char*)snapshort_key->name,sai->share_struct,sizeof(snapshort_key->name)-1);
			memcpy(snapshort_key->stat, (char*)&sai->stat, sizeof(sai->stat));
			sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
			break;
		case SA_ACT_DELETE_KEY:
			len = sizeof(ucp_key_ctrl_t);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY, len);
			kc->key_id = sai->temp_value & 0xFF;
			
			sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE, true, 0x1, uo, sizeof(*uo)+len);
			break;
		case SA_ACT_CTRL_KEY:
			len = sizeof(ucp_key_ctrl_t);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY_CODE, len);
			kc->key_id = sai->temp_value & 0xFF;
			
			sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
			break;
		default:
	        *ret = RS_INVALID_PARAM;
	        return false;
			break;
	}

	return true;
}

static bool air_porc_pan_type(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_pan_ctrl_t*pc = (ucp_pan_ctrl_t*)(uo+1);
	int len = sizeof(*pc);
	pc->pan_type = sai->temp_value & 0xFF;

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_BOARD, len);
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	
	return true;
}

static bool air_porc_kl_request(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_key_learn_t*kl = (ucp_key_learn_t*)(uo+1);
	int len = sizeof(*kl);

	kl->id = sai->temp_value & 0xFF;
	switch(sai->action){
		case SA_ACT_START_LEARN_KEY:
			kl->action = 0;
			break;
		case SA_ACT_STOP_LEARN_KEY:
			kl->action = 0x1;
			break;
		default:
	        *ret = RS_INVALID_PARAM;
	        return false;
			break;
	}
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_LEARN, len);
	sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);

	return true;
}

static bool air_proc_key_learn_request(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	if(!ac->air_info.is_support_switch_pan && !ac->air_info.is_support_key_learn){
		    log_err(false, "air_proc_key_learn_request not support switch pan or key learn handle %08x\n",sai->dev_handle);
	        *ret = RS_NOT_SUPPORT;
	        return false;
	}
	
	switch(sai->action){
		case SA_ACT_SET_PAN_TYPE:
			air_porc_pan_type(ac,sai,ret);
			break;
		case SA_ACT_REFRESH_KEY_INFO:
			sa_query_obj(ac->sac->user->uc_session, UCOT_IA, UCSOT_IA_CODE, UCAT_CONTROL_KEY);
			break;
		case SA_ACT_SET_KEY_INFO:
		case SA_ACT_SET_KEY_INFO_V2:
		case SA_ACT_DELETE_KEY:
		case SA_ACT_CTRL_KEY:
			air_porc_key_request(ac,sai,ret);
			break;
		case SA_ACT_START_LEARN_KEY:
		case SA_ACT_STOP_LEARN_KEY:
			air_porc_kl_request(ac,sai,ret);
			break;
		default:
			log_err(false, "air_proc_key_learn_request invalid action[%u] not support switch pan ! handle %08x\n",sai->action,sai->dev_handle);
	        *ret = RS_INVALID_PARAM;
	        return false;
			break;
	}

	return true;
}

static void air_proc_isc_seton(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	
	char buf[256] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
    uc_tlv_t* tlv = (uc_tlv_t*)(uo+1);
    int len = sizeof(*tlv) + sizeof(u_int32_t);

	memset(buf, 0, sizeof(buf));
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_ON_USER_SET, len);
    tlv->type = htons(TLV_2_2_13_SAME_ONOFF);
    tlv->len = htons(sizeof(u_int32_t));
    
    ((u_int8_t*)(tlv+1))[1] = !!sai->stat.onoff;
    
    ac->air_info.air_work_stat.onoff = !sai->stat.onoff;
    
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	*ret = RS_OK;
}

static void air_proc_temp_curve(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	int len = 0;
	char buf[1024*2] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
    cl_temp_curve_t* tc;
    timer_switch_t t;

	memset(buf, 0, sizeof(buf));
	len = sai->trans_data.trans_len;
	
    if (ac->air_info.is_support_utc_temp_curve) {
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_TMP_UTC_CURVE, len);
        memcpy((void *)(uo+1), (void *)sai->trans_data.trans_buf, len);
        
        tc = (cl_temp_curve_t*)(uo+1);
        
        t.hour = tc->begin_hour;
        t.week = tc->week;
        _com_timer_local_2_utc(&t);
        tc->begin_hour = t.hour;
        tc->week = t.week;
        
        t.hour = tc->end_hour;
        _com_timer_local_2_utc(&t);
        tc->end_hour = t.hour;
        
        
    }else{
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_TMP_CURVE, len);
        memcpy((void *)(uo+1), (void *)sai->trans_data.trans_buf, len);
    }
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	*ret = RS_OK;
	//立马更新本地数据
	ac->air_info.temp_curve_len = 0;
	if (ac->air_info.temp_curve) {
		SAFE_FREE(ac->air_info.temp_curve);
	}
    
	ac->air_info.temp_curve = cl_calloc(len, 1);
	if (ac->air_info.temp_curve) {
		memcpy((void *)ac->air_info.temp_curve, (void *)sai->trans_data.trans_buf, len);
		ac->air_info.temp_curve_len = len;
	}
    
	if (ac->sac) {
    	event_push(ac->sac->user->callback, UE_INFO_MODIFY, ac->sac->user->handle, ac->sac->user->callback_handle);
    	event_cancel_merge(ac->sac->user->handle);
	}
}

static void air_proc_msg_config(smart_air_ctrl_t* ac, cln_sa_air_info_t* sai, RS* ret)
{
	int len = 0;
	char buf[1024*2] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;

	len = sai->trans_data.trans_len;
	memcpy((u_int8_t*)&uo[1], sai->trans_data.trans_buf, len);

	if (ac->air_info.is_support_msg_config == 1) {	
	    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_MSG_CONFIG, len);
	} else {
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_MSG_CONFIG_V2, len);
	}

	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
}


static void air_proc_set_temp_ctrl(smart_air_ctrl_t* ac,cln_sa_air_info_t* sai,RS* ret)
{
	int len = 0;
	char buf[256] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
    timer_switch_t t;
    cl_temp_ac_ctrl_t* tc;

	memset(buf, 0, sizeof(buf));
	len = sai->trans_data.trans_len;
	
	// 优先处理PUBLIC的
	if (ac->com_udp_dev_info.is_support_public_utc_temp_ac_ctrl) {
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_UTC_TMP_CTRL, len);
        memcpy((void *)(uo+1), (void *)sai->trans_data.trans_buf, len);
        
        tc = (cl_temp_ac_ctrl_t*)(uo+1);
        t.hour = tc->begin_hour;
        t.week = tc->week;
        _com_timer_local_2_utc(&t);
        tc->begin_hour = t.hour;
        tc->week = t.week;
        
        t.hour = tc->end_hour;
        _com_timer_local_2_utc(&t);
        tc->end_hour = t.hour;

		return;
	}
	
    if (ac->air_info.is_support_utc_temp_ac_ctrl) {
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_UTC_TMP_CTRL, len);
        memcpy((void *)(uo+1), (void *)sai->trans_data.trans_buf, len);
        
        tc = (cl_temp_ac_ctrl_t*)(uo+1);
        t.hour = tc->begin_hour;
        t.week = tc->week;
        _com_timer_local_2_utc(&t);
        tc->begin_hour = t.hour;
        tc->week = t.week;
        
        t.hour = tc->end_hour;
        _com_timer_local_2_utc(&t);
        tc->end_hour = t.hour;
    }else{
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_TMP_CTRL, len);
        memcpy((void *)(uo+1), (void *)sai->trans_data.trans_buf, len);
    }
	sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	*ret = RS_OK;
	//立马更新本地数据
	if (sizeof(ac->air_info.tac) == len) {
		memcpy((void *)&ac->air_info.tac, (void *)sai->trans_data.trans_buf, len);
	}
	if (ac->sac) {
    	event_push(ac->sac->user->callback, UE_INFO_MODIFY, ac->sac->user->handle, ac->sac->user->callback_handle);
    	event_cancel_merge(ac->sac->user->handle);
	}
}

static bool air_proc_fan_adjust(smart_air_ctrl_t* ac,u_int8_t onoff)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    uc_tlv_t* tlv = (uc_tlv_t*)(uo+1);
    int len;
    
    len = sizeof(*tlv)+sizeof(u_int32_t);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_ON_USER_SET, len);
    tlv->type = htons(TLV_2_2_13_SAME_FAN);
    tlv->len = htons(sizeof(u_int32_t));
    
    ((u_int8_t*)(tlv+1))[1] = !!onoff;
    
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
    
    ac->air_info.air_work_stat.wind_direct = !ac->air_info.air_work_stat.wind_direct ;
    
    return true;
}

static bool air_proc_fan_speed_adjust(smart_air_ctrl_t* ac,u_int8_t is_opp)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    uc_tlv_t* tlv = (uc_tlv_t*)(uo+1);
    int len;
    user_t* user = ac->sac->user;
    
    if ((ac->air_info.ajust_info.is_fan_speed_opposite && is_opp) ||
        (!ac->air_info.ajust_info.is_fan_speed_opposite && !is_opp )) {
        return true;
    }
    
    
    len = sizeof(*tlv)+sizeof(u_int32_t);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_IA_ON_USER_SET, len);
    tlv->type = htons(TLV_2_2_13_FAN_SPEED_REVERSE);
    tlv->len = htons(sizeof(u_int32_t));
    
    *((u_int8_t*)(tlv+1)) = !!is_opp;
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
    
    //反转风速，通知上层
    ac->air_info.ajust_info.is_fan_speed_opposite = is_opp;
    event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    
    return true;
}

static bool air_proc_chilid_lock(smart_air_ctrl_t* ac,u_int8_t lock_stat)
{
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_child_lock_t* uc = (ucp_child_lock_t*)(uo+1);
    user_t* user = ac->sac->user;

    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_CHILD_LOCK, sizeof(*uc));
    uc->action = lock_stat;
    ac->air_info.child_lock_value = lock_stat;
    
    sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*uc));
    
    return true;
}

static bool air_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_sa_air_info_t* sai;
    user_t* user;
    smart_air_ctrl_t* ac;
    
    sai = (cln_sa_air_info_t*)pkt->data;
    user = lookup_by_handle(HDLT_USER, sai->dev_handle);
    
    if (!user || user->sub_type != IJ_SMART_PLUG || !user->smart_appliance_ctrl ) {
        log_err(false, "air_proc_notify error handle %08x\n",sai->dev_handle);
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac) {
        log_err(false, "air_proc_notify error handle %08x\n",sai->dev_handle);
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    switch (pkt->type) {
        case CLNE_SA_AIR_CTRL_STAT:
            return air_proc_stat_ctrl(ac,sai,ret);
            break;
        case CLNE_SA_AIR_CTRL_POWER:
        {
            sai->stat.temp = ac->air_info.air_work_stat.temp ;
            sai->stat.mode = ac->air_info.air_work_stat.mode;
            sai->stat.wind = ac->air_info.air_work_stat.wind;
            sai->stat.wind_direct = ac->air_info.air_work_stat.wind_direct;
            ac->air_info.air_work_stat.onoff = sai->stat.onoff ;
            sai->action = AC_KEY_POWER;
            return air_proc_stat_ctrl(ac, sai, ret);
        }
            break;
        case CLNE_SA_AIR_CTRL_MODE:
        {
            sai->stat.temp = ac->air_info.air_work_stat.temp ;
            sai->stat.onoff = ac->air_info.air_work_stat.onoff;
            sai->stat.wind = ac->air_info.air_work_stat.wind;
            sai->stat.wind_direct = ac->air_info.air_work_stat.wind_direct;
            ac->air_info.air_work_stat.mode = sai->stat.mode;
            sai->action = AC_KEY_MODE;
            return air_proc_stat_ctrl(ac, sai, ret);
        }
            break;
        case CLNE_SA_AIR_CTRL_WIND:
        {
            sai->stat.temp = ac->air_info.air_work_stat.temp;
            sai->stat.mode = ac->air_info.air_work_stat.mode;
            sai->stat.onoff = ac->air_info.air_work_stat.onoff;
            sai->stat.wind_direct = ac->air_info.air_work_stat.wind_direct;
            ac->air_info.air_work_stat.wind = sai->stat.wind;
            sai->stat.wind = air_cl_wind_2_net_wind(sai->stat.wind,ac->air_info.ajust_info.is_fan_speed_opposite);
            
            sai->action = AC_KEY_WIND;
            return air_proc_stat_ctrl(ac, sai, ret);
        }
            break;
        case CLNE_SA_AIR_CTRL_TEMP:
        {
            sai->stat.onoff = ac->air_info.air_work_stat.onoff;
            sai->stat.mode = ac->air_info.air_work_stat.mode;
            sai->stat.wind = ac->air_info.air_work_stat.wind;
            sai->stat.wind_direct = ac->air_info.air_work_stat.wind_direct;
            ac->air_info.air_work_stat.temp = sai->stat.temp;
            sai->action = AC_KEY_TEMP;
            return air_proc_stat_ctrl(ac, sai, ret);
        }
            break;
        case CLNE_SA_AIR_CTRL_DIRECT:
        {
            sai->stat.temp = ac->air_info.air_work_stat.temp ;
            sai->stat.mode = ac->air_info.air_work_stat.mode;
            sai->stat.onoff = ac->air_info.air_work_stat.onoff;
            sai->stat.wind = ac->air_info.air_work_stat.wind;
            ac->air_info.air_work_stat.wind_direct = sai->stat.wind_direct ;
            sai->action = AC_KEY_DIR;
            return air_proc_stat_ctrl(ac, sai, ret);
        }
            break;
        case CLNE_SA_AIR_CTRL_OLD_AIR:
            return air_proc_stat_ctrl(ac, sai, ret);
            break;
        case CLNE_SA_AIR_CTRL_LED:
            air_proc_led_ctrl(ac, sai, ret);
            break;
        case CLNE_SA_AIR_PEAK_PERIOD:
            air_proc_peak_period(ac, sai, ret);
            break;
        case CLNE_SA_AIR_PEAK_PRICE:
            air_proc_peak_price(ac, sai, ret);
            break;
        case CLNE_SA_AIR_VALLEY_PERIOD:
            air_proc_valley_period(ac, sai, ret);
            break;
        case CLNE_SA_AIR_VALLEY_PRICE:
            air_proc_valley_price(ac, sai, ret);
            break;
        case CLNE_SA_AIR_FLAT_PRICE:
            air_proc_flat_price(ac, sai, ret);
            break;
        case CLNE_SA_AIR_MODIFY_TIMER:
        case CLNE_SA_AIR_DEL_TIMER:
            air_proc_timer(pkt->type, ac, sai, ret);
            break;
        case CLNE_SA_MODIFY_PERIOD_TIMER:
        case CLNE_SA_PERIOD_DEL_TIMER:
            air_proc_period_timer(pkt->type, ac, sai, ret);
            break;
        case CLNE_SA_CODE_MATCH_CTRL:
            air_proc_code_match(ac, sai, ret);
            break;
        case CLNE_SA_SMART_CTRL:
            air_proc_smart_ctrl(ac, sai, ret);
            break;
        case CLNE_SA_AIR_POWER_START:
            air_quick_cur_power(ac,sai,ret);
            break;
        case CLNE_SA_AIR_REFRESH_TIMER:
            if (ac->air_info.is_support_peroid_ext_timer) {
                sa_query_obj(user->uc_session,UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_EXT_PERIOD_TIMER);
            }else if(ac->air_info.is_support_peroid_timer){
                sa_query_obj(user->uc_session,UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_PERIOD_TIMER);
            }else{
                sa_query_obj(user->uc_session,UCOT_IA, UCSOT_IA_AC, UCAT_AC_TIMER);
            }
            break;
        case CLNE_SA_AIR_REFRESH_ELEC:
            air_query_elec_info(ac);
            break;
        case CLNE_SA_AIR_CLEAR_ELEC:
            air_clear_stat_info(ac,sai->action);
            break;
        case CLNE_SA_AIR_LED_COLOR:
            air_proc_led_color_ctrl(ac,sai,ret);
            break;
        case CLNE_SA_TRANS_SEND:
            air_proc_trans_send(ac, sai, ret);
            break;
        case CLNE_SA_RESTORE_FACTORY:
            air_proc_restore_factory(ac, sai, ret);
            break;
        case CLNE_SA_PT_SET_ADKB:
            air_proc_pt_set_adkb(ac, sai, ret);
            break;
		case CLNE_SA_PT_SET_ADJ_EXT:
			air_proc_pt_set_adkb_ext(ac, sai, ret);
			break;
        case CLNE_SA_PT_SET_ADJ:
            air_proc_pt_set_adj(ac, sai, ret);
            break;		
        case CLNE_SA_SEND_LICENSE:
            air_proc_pt_send_lcc(ac, sai, ret);
            break;	
        case CLNE_SA_KEY_LEARN:
            air_proc_key_learn_request(ac,sai,ret);
            break;
        case CLNE_SA_ISC_SETON:
            air_proc_isc_seton(ac,sai,ret);
            break;
        case CLNE_SA_TEMP_CURVE:
            air_proc_temp_curve(ac,sai,ret);
            break;
        case CLNE_SA_SET_TEMP_CTRL:
            air_proc_set_temp_ctrl(ac,sai,ret);
            break;
        case CLNE_SA_AJUST_FAN_SPEED:
            air_proc_fan_speed_adjust(ac,!!sai->temp_value);
            break;
        case CLNE_SA_AJUST_FAN:
            air_proc_fan_adjust(ac,!!sai->temp_value);
            break;
        case CLNE_SA_SET_CHILD_LOCK:
            air_proc_chilid_lock(ac,sai->temp_value&0xff);
            break;
		case CLNE_SA_SET_I8_MSG_CONFIG:
			air_proc_msg_config(ac, sai, ret);
			break;
        default:
            return false;
            break;
    }
    return true;
}

static void air_do_uc_notify(user_t* user , ucph_t* hdr)
{
    ucp_notify_head_t* head = (ucp_notify_head_t*)(hdr+1);
    
    switch (head->action) {
        case 0://数据有变化
            
            break;
            
        default:
            break;
    }
    
}

////////////////////////////////////////////////////////
//智能设备统一处理函数
void sa_query_debuginfo(ucc_session_t *s)
{
    ucp_obj_t obj[] = {{UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_DONAME,0},
					   {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_DEBUGINFO,0},
					   	{UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_VERSION,0},};	
					   
	sa_query_objects(s, obj, 0x3);
}

static bool comm_cal_the_timer(cl_comm_timer_head_t *pcth, u_int32_t now)
{
	if (pcth->timer_count == 0 ||
		pcth->min_time == 0) {
		return false;
	}

	if (now < pcth->min_time + 5) {
		return false;
	}

	comm_timer_next_cal(pcth);

	return true;
}

static void comm_timer_timer_cal(user_t *user)
{
	u_int32_t now;
	cl_comm_timer_head_t *pcth;
	slave_t *slave, *n;
	bool need_modify = false;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;

	now = (u_int32_t)time(NULL);
	if (!user->smart_appliance_ctrl) {
		return;
	}
	
	sac = user->smart_appliance_ctrl;
	if (!sac->sub_ctrl) {
		return;
	}
	
	air_ctrl = sac->sub_ctrl;
	pcth = &air_ctrl->com_udp_dev_info.comm_timer_head;
	if (comm_cal_the_timer(pcth, now)) {
		need_modify = true;
	}
	stlc_list_for_each_entry_safe(slave_t, slave, n, &user->slave, link) {
		if (comm_cal_the_timer(&slave->comm_timer, now)) {
			need_modify = true;
		}
	}

	if (need_modify) {
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
        //event_cancel_merge(user->handle);
	}	
}

static void phone_backgroud_notify_2_dev(user_t *user)
{
    char buf[1024] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_phone_status_t* uc = (ucp_phone_status_t*)(uo+1);

	memset((void *)buf, 0, sizeof(buf));
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_BACKGROUND, sizeof(*uc));
	uc->is_front = !user->last_background_status;
    
    sa_set_obj_value_only(user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*uc));
}

static void comm_timer_background_notify(user_t *user)
{
	if (!user->is_support_background_set) {
		return;
	}
	if (cl_priv->run_in_background == 
		user->last_background_status) {
		return;
	}
	user->last_background_status = cl_priv->run_in_background;

	phone_backgroud_notify_2_dev(user);
}

static int comm_timer_proc(cl_thread_t *t)
{
	user_t *user;
    
    user = (user_t*)CL_THREAD_ARG(t);
	user->t_timer_comm = NULL;

	//计算下次定时器时间
	comm_timer_timer_cal(user);

	//处理app后台通知
	comm_timer_background_notify(user);

	CL_THREAD_TIMER_ON(MASTER, user->t_timer_comm, comm_timer_proc, (void *)user, TIME_N_SECOND(10));
	
	return 0;
}

static void sa_comm_timer_start(user_t *user)
{
	CL_THREAD_OFF(user->t_timer_comm);
    CL_THREAD_TIMER_ON(MASTER, user->t_timer_comm, comm_timer_proc, (void *)user, TIME_N_SECOND(10));
}

static void sa_quick_query_all_mib(smart_appliance_ctrl_t* sac)
{
    ucp_obj_t obj = {UCOT_SYSTEM, UCSOT_SYS_SUPPORT, UCAT_SUPPO_RT_SUPPORT, 0};
    ucc_session_t *s = sac->user->uc_session;
    
    log_debug("query All Mib object\n");
    sa_query_objects(s, &obj, 0x1);
	sa_comm_timer_query(s);
	sa_query_debuginfo(s);
	sa_comm_timer_start(sac->user);
}

static void sa_quick_query_uptime_mib(smart_appliance_ctrl_t* sac)
{
    ucp_obj_t obj[] = {{UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_CONNECT_TIME,0},
					   {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_UPTIME,0},
					   {UCOT_IA,UCSOT_IA_STAT,UCAT_STAT_CUR_POWER,0},
					   {UCOT_DEVPRO,UCSOT_DEVPRO_IR,UCAT_IR_CURKB,0},
					   {UCOT_SYSTEM,UCSOT_SYS_SERVER,UCAT_SERVER_IP,0},
					   {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_SYSTIME,0},
					   {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_DEBUGINFO,0},					   	
					   {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_DEVSTATUS,0}};
					   
    ucc_session_t *s = sac->user->uc_session;

    sa_query_objects(s, obj, sizeof(obj)/sizeof(ucp_obj_t));
}

void sa_force_refresh_uptime(user_t* user)
{
    if (!user || !user->smart_appliance_ctrl) {
        return;
    }

    sa_quick_query_uptime_mib(user->smart_appliance_ctrl);
}


void sa_systime_set(user_t* user, struct tm *ptm)
{
    pkt_t *pkt;
    ucp_obj_t* uo;	
    ucp_ctrl_t* uc;
	uc_time_t sys_time;
	ucc_session_t *s = NULL;
	uc_time_t *psys_time = NULL;	
	ucp_obj_t obj = {UCOT_SYSTEM,UCSOT_SYS_SOFTWARE,UCAT_SYS_SET_SYSTIME,0};

	
    if (!ptm || !user || !user->smart_appliance_ctrl) {
        return;
    }

	sys_time.year = ptm->tm_year + LOCALTIME_YEAR_BASE;
	sys_time.year = htons(sys_time.year);
	sys_time.month = ptm->tm_mon + 1;
	sys_time.day = ptm->tm_mday;
	sys_time.hour = ptm->tm_hour;
	sys_time.minute = ptm->tm_min;
	sys_time.second = ptm->tm_sec;
	sys_time.timezone = 0;

	s = ((smart_appliance_ctrl_t *)user->smart_appliance_ctrl)->user->uc_session;
    pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+sizeof(ucp_obj_t) + 
sizeof(uc_time_t),
                     true, s->select_enc?true:false, 0, s->client_sid, s->
device_sid, s->my_request_id);
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
    uc->count = 1;
    uo = (ucp_obj_t*)(uc+1);	
    fill_net_ucp_obj(uo, obj.objct, obj.sub_objct, obj.attr, sizeof(uc_time_t)
);
	psys_time = (uc_time_t*)(uo + 1);
	memcpy((void *)psys_time, (void *)&sys_time, sizeof(uc_time_t));
	
    ucc_request_add(s, pkt);
}


static void sa_do_action_when_estab_ok(user_t* user)
{
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* ac;

	if(is_supported_udp_device(user->sub_type,user->ext_type)){
		sac = user->smart_appliance_ctrl;
		if(!sac->sub_ctrl){
			return;
		}
		ac = sac->sub_ctrl;
		ac->com_udp_dev_info.is_all_data_update = 0;
		if(user->direct_ip == 0){
			ac->com_udp_dev_info.is_lan_connect = 2;
		}else{
			ac->com_udp_dev_info.is_lan_connect = 0x1;
		}
	}

}

void sa_force_refresh_data(user_t* user)
{
    if (!user || !user->smart_appliance_ctrl) {
        return;
    }
    sa_do_action_when_estab_ok(user);
    sa_quick_query_all_mib(user->smart_appliance_ctrl);
}

void sa_do_action_when_estab_out(user_t* user)
{
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* ac;

	if (!user || !user->smart_appliance_ctrl) {
		return;
	}

	if(is_supported_udp_device(user->sub_type,user->ext_type)){
		sac = user->smart_appliance_ctrl;
		if(!sac->sub_ctrl){
			return;
		}
		ac = sac->sub_ctrl;
		ac->com_udp_dev_info.is_all_data_update = 0;
		ac->com_udp_dev_info.is_lan_connect = 0;
	}
    if (user->sub_type == IJ_RFGW) {
        rfgw_proc_gw_offline_hook(user);
    }
}

static bool sa_is_support_attr(smart_appliance_ctrl_t* sac,u_int16_t obj,u_int16_t sub_obj,u_int16_t attr)
{
	ucp_obj_t* pobj = sac->all_mib;
	uc_sys_support_obj_t* usobj;
	unsigned int i;

	if(pobj == NULL){
		return false;
	}

	 usobj = (uc_sys_support_obj_t*)(pobj+1);
        for (i = 0; i < pobj->param_len/sizeof(*usobj) ; i++) {
            if(usobj->obj == obj && usobj->sub_obj == sub_obj && usobj->attr == attr){
			return true;
            	}
        }

	return false;
}

//根据设备支持的，设置属性
static void sa_set_support_flag(smart_air_ctrl_t* ac, uc_sys_support_obj_t* so)
{
	if(!ac){
		return;
	}
    
    log_debug("set attr flag  obj=[%u] sub_obj=[%u] attr=[%u]\n",so->obj,so->sub_obj,so->attr);
	if (ac->sac && ac->sac->user) {
		nd_login_debug(ac->sac->user->uc_session, "set attr flag  obj=[%u] sub_obj=[%u] attr=[%u]\n",so->obj,so->sub_obj,so->attr);
	}

	switch(so->obj){
		case UCOT_SYSTEM:
		{
			if(so->sub_obj == UCSOT_SYS_HARDWARE && so->attr == UCATT_HARDWARE_LED_COLOR){
				ac->air_info.is_support_led_color = true;
			}
		}
			break;
		case UCOT_IA:
		{
			log_debug("support ia: sub_obj %u attri %u\n", so->sub_obj, so->attr);
			
			if(so->sub_obj == UCSOT_IA_PUBLIC && so->attr == UCAT_IA_PUBLIC_PERIOD_TIMER){
				log_debug("receive system is_support_peroid_timer packet!\n");
				ac->air_info.is_support_peroid_timer = true;
			} else if(so->sub_obj == UCSOT_IA_CODE && so->attr == UCAT_CONTROL_BOARD){
				ac->air_info.is_support_switch_pan = true;
			} else if(so->sub_obj == UCSOT_IA_CODE && so->attr == UCAT_CONTROL_KEY){
				ac->air_info.is_support_key_learn = true;
			} else if(so->sub_obj == UCSOT_IA_CODE && so->attr == UCAT_CODE_LEARN_SNAPSHOT){
				ac->air_info.is_support_learn_snapshort = true;
			} else if (so->sub_obj == UCSOT_IA_RFGW && so->attr == UCAT_IA_RFGW_LIST_DIGEST) {
				log_debug("support tt cache\n");
				ac->is_support_rf_cache = true;
			} else if (so->sub_obj == UCSOT_IA_RFGW && so->attr == UCAT_IA_RFGW_DBC) {
				log_debug("support dbc\n");
				if (ac->sac && ac->sac->user) {
					ac->sac->user->is_support_dbc = true;
				}
			} else if (so->sub_obj == UCSOT_IA_HPGW && so->attr == UCAT_IA_HPGW_ALARM_CONFIG) {
				if (ac->com_udp_dev_info.device_info) {
					((rfgw_priv_t*)ac->com_udp_dev_info.device_info)->support_hpinfo = true;
				}
			}
		}
		
		default:
			break;
	}
	udp_set_support_flag_hook(ac,so);
	
}

static void sa_set_support_flag_done(smart_air_ctrl_t* ac)
{
	ac->com_udp_dev_info.flag_stat_update_ok = true;
}

/**
	一些根据SN做的特殊处理
*/
static void sa_flag_speical(smart_air_ctrl_t *ac)
{
	user_t *user = ac->sac->user;
	u_int64_t sn = user->sn;
		
	// 2016/05/24  针对一批磨具上不插孔的悟空，去掉电量统计
	if (sn >= 808080000100 && sn <= 808080006420) {
		ac->com_udp_dev_info.is_suppport_elec_stat = false;
	}
}

//预处理或截取mib，返回true截取，否则只查看
static bool sa_pre_hand_reply_mib(user_t* user,ucp_obj_t* obj)
{
    smart_appliance_ctrl_t* sac = user->smart_appliance_ctrl;
	smart_air_ctrl_t* ac = sac->sub_ctrl;
    uc_sys_support_obj_t* usobj;
    u_int32_t i,len;
    ucp_obj_t* t_obj;
	u_int8_t back_la = 0;
    //统一处理系统所有支持的
    if (is_except_attr(obj,UCOT_SYSTEM,UCSOT_SYS_SUPPORT,UCAT_SUPPO_RT_SUPPORT)) {
        if ( obj->param_len/sizeof(*usobj) == 0 ||
            obj->param_len % sizeof(*usobj) != 0) {
            log_err(false,"sa_pre_hand_reply_mib system support param_len_err %u\n", obj->param_len);
            return true;
        }
		
        usobj = (uc_sys_support_obj_t*)(obj+1);
		
		ac->com_udp_dev_info.has_recv_flag_pkt = true;
		back_la = ac->com_udp_dev_info.is_support_la;
		ac->com_udp_dev_info.is_support_la = 0;
		
        for (i = 0; i < obj->param_len/sizeof(*usobj) ; i++,usobj++) {
            usobj->obj = ntohs(usobj->obj);
            usobj->sub_obj = ntohs(usobj->sub_obj);
            usobj->attr = ntohs(usobj->attr);
            if(sac->sub_ctrl != NULL){
                sa_set_support_flag(sac->sub_ctrl,usobj);
            }
        }
        
        len = sizeof(*obj)+sizeof(uc_sys_support_obj_t)*(obj->param_len/sizeof(*usobj));
        t_obj = cl_malloc(len);
        if (t_obj) {
            memcpy(t_obj, obj, len);
            SAFE_FREE(sac->all_mib);
            sac->all_mib = t_obj;
        }
        
        air_quick_query_all_info(sac);
		//获取支持属性后，可以检查是否支持联动属性变化了
		if (back_la != ac->com_udp_dev_info.is_support_la) {
			user->is_dev_la_changed = true;
			do_dev_la_sync();
		}
		//有可能在错误密码登陆设备重登陆后密码变化了,那不支持联动的设备就需要同步下密码了
		if (user->maybe_need_pd_sync && !ac->com_udp_dev_info.is_support_la) {
			do_passwd_sync_to_server(user);
		}
		user->maybe_need_pd_sync = false;
		
        return true;
    }


	// 表示支持的标志都打完了
	if (sac->sub_ctrl != NULL){
        sa_set_support_flag_done(sac->sub_ctrl);
    }
    
    return false;
}

static void sa_send_push_response(user_t* user,ucp_ctrl_t* suc)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    ucc_session_t* s = user->uc_session;
    ucp_obj_t* obj;
    ucp_obj_t* srcobj = (ucp_obj_t*)(suc+1);
    
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t) + sizeof(ucp_obj_t)
                     ,false, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->peer_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
    obj = (ucp_obj_t*)(uc+1);
	uc->action = UCA_PUSH;
    if (suc->count > 0 ) {
        uc->count = 1;
        memcpy(obj, suc+1, sizeof(*obj));
        obj->param_len = 0x0;
    }else{
        uc->count = 0;
    }
    
    log_info("send PUSH PACKET to peer obj count [%u]  to us[%u] rid 0x%x\n",uc->count,suc->count, s->peer_request_id);
    ucc_response_send(s,pkt);
}

static bool sa_rid_check(user_t* user, u_int8_t rid)
{
	ucc_session_t *s = user->uc_session;

	//不等于鑫源温控器就直接返回真
	if ((user->sub_type != IJ_TEST_DEV ||
		user->ext_type != ETYPE_IJ_TEST_XY) && 
		(user->sub_type != IJ_838_YT) &&
		(user->sub_type != IJ_EVM) &&
		(user->sub_type != IJ_RFGW) &&
		(user->sub_type != IJ_INDIACAR) &&
		((user->sub_type != IJ_KXM_DEVICE) ||
		((user->ext_type != ETYPE_IJ_GALAXYWIND_THERMOSTAT) &&
		(user->ext_type != ETYPE_IJ_GALAXYWIND_THERMOSTAT_WEB)))) 
	{
		return true;
	}

	if (s->last_rid == rid) {
		return false;
	}

	s->last_rid = rid;

	return true;
}

static void sa_parse_ctrl_pkt(user_t* user,ucph_t* hdr)
{
    ucp_ctrl_t* uc;
    ucp_obj_t* obj;
    int total = hdr->param_len;
    int index;
    bool modify = false;
    smart_appliance_ctrl_t* sac;
    u_int16_t error;
	bool is_valid_dev = false;

    
    //不是智能终端
    if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
        return;
    }
    
//    mem_dump("sa ctrl packet ************** ", hdr, sizeof(*hdr)+total);
    
    uc = get_net_ucp_payload(hdr, ucp_ctrl_t);
    log_debug("sa_parse_ctrl_pkt last_ctrl=%u actin[%u] count[%u] request[%u] \n", user->last_ctrl, uc->action,
           uc->count,hdr->request);
    //先回响应
    if (uc->action == UCA_PUSH && hdr->request) {
		//这里判断一下，如果是rfgw网关push数据就别回包，后面自己回
		if (hdr->flags != 0x01) {
        	sa_send_push_response(user,uc);
		}
    }
    
    if (!uc->count) {
        return;
    }

	if (hdr->flags != 0x01) {
		if (!sa_rid_check(user, hdr->request_id)) {
			return;
		}
	}
	
	if(cl_priv->is_pt_mode == 0){//产测模式下不用判断设备类型
		switch (user->sub_type) {
			case IJ_SMART_PLUG:
			case IJ_812:
			case IJ_816:
			case IJ_813:
			case IJ_820:
			case IJ_101:
			case IJ_102:
			case IJ_821:
				is_valid_dev = true;
			        break;
			default:
				if(!is_supported_udp_device(user->sub_type,user->ext_type)){
			     		 return;
				}
				is_valid_dev = true;
			     break;
		}
	}
		
    total-=sizeof(*uc);
    obj = (ucp_obj_t*)(uc+1);
    
    for (index = 0; index < uc->count && total >= sizeof(ucp_obj_t); index++) {
        ucp_obj_order(obj);
        
        if (!is_valid_obj(obj,total)) {
            break;
        }
        log_debug("********* sa handle object[%u] sub_obj[%u] attr[%u] param_len[%u], action[%hhu]\n",obj->objct,obj->sub_objct,obj->attr,obj->param_len, uc->action);
       
        //统一预处理
        total-=(sizeof(ucp_obj_t)+obj->param_len);
        if (sa_pre_hand_reply_mib(user,obj)) {
            obj = (ucp_obj_t*)((char*)(obj+1)+obj->param_len);
            continue;
        }

	 if (uc->action == UCA_GET ||
            uc->action == UCA_PUSH) {
            if(air_update_data_with_mib(sac->sub_ctrl,uc->action,obj)){
                modify = true;
            }
        }else if(uc->action == UCA_SET){
        	// 云匹配SET的回复报文是一个结构体，特殊处理下
        	air_hand_cloud_result(sac->sub_ctrl, obj);

			//这里加上一些网关控制结果提示
			if (user->is_support_batch_user && 
				(user->sub_type == IJ_RFGW)) {
				do_rfgw_ctrl_result(sac->sub_ctrl, obj, ntohs(*((u_int16_t*)(obj+1))));
			}

            if (obj->param_len != sizeof(u_int16_t)) {
                break;
            }
			error = ntohs(*((u_int16_t*)(obj+1)));
            air_hand_ctrl_result(sac->sub_ctrl, obj,error);
			
        }
        
        obj = (ucp_obj_t*)((char*)(obj+1)+obj->param_len);
    }
    
    if (modify) {
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
		//现在modify上传太多了，app受不了，打开消息归并功能，modify归并每次延时200毫秒，最多2s就push
        //event_cancel_merge(user->handle);
    }
}

bool sa_do_uc_request_pkt(user_t* user,ucph_t* hdr)
{
    if (!user || !hdr) {
        return true;
    }
    
    switch (hdr->command) {
        case CMD_UDP_NOTIFY:
            break;
        case CMD_UDP_BIND_PHONE:
            break;
        case CMD_UDP_CTRL:
            sa_parse_ctrl_pkt(user,hdr);
            break;
        default:
            log_debug("%s ignore reqeust pkt cmd=%u\n", user->name, hdr->command);
            break;
	}
    
    return true;
}

static void sa_send_notify_response(user_t* user)
{
    pkt_t *pkt;
    ucc_session_t *s = user->uc_session;
    
    if (!s ) {
        return;
    }
    pkt = uc_pkt_new(s,CMD_UDP_NOTIFY, false,true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->peer_request_id);
    
    ucc_response_send(s, pkt);
}

static void sa_do_uc_notify(user_t* user,ucph_t* hdr)
{
    //不管三七二十一，先发个响应.
    if (hdr->request) {
        sa_send_notify_response(user);
    }
    
    switch (user->sub_type) {
        case IJ_808:
            air_do_uc_notify(user, hdr);
            break;
            
        default:
            break;
    }
    
}

bool sa_do_uc_reply_pkt(user_t* user,ucph_t* hdr)
{
    if (!user || !hdr) {
        return true;
    }
    
    switch (hdr->command) {
        case CMD_UDP_NOTIFY:
            sa_do_uc_notify(user,hdr);
            break;
        case CMD_UDP_BIND_PHONE:
            break;
        case CMD_UDP_CTRL:
            sa_parse_ctrl_pkt(user,hdr);
            break;
        default:
            log_debug("%s ignore reqeust pkt cmd=%u\n", user->name, hdr->command);
            break;
	}
    
    return true;
}

bool sa_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	if (pkt->type < CLNE_SA_START || pkt->type > CLNE_SA_END) {
		return udp_proc_notify_hook(pkt,ret);
	}
	
	return air_proc_notify(pkt, ret);
}

static void _comm_test_timer(user_t *user, cl_comm_timer_t *pctimer)
{
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	cl_comm_timer_head_t *pcth ;
	cl_comm_timer_t *pctimer_tmp;
	int len;
	static int num = 1;

	
	sac = user->smart_appliance_ctrl;
	air_ctrl = sac->sub_ctrl;
	pctimer->id = num++;
	pctimer->valid = 1;	


	//这里专门处理下，rf定时器需要
	if (pctimer->week) {
		pctimer->week_cal = pctimer->week|BIT(7);
	} else {
	//如果week==0，赋值
		pctimer->week_cal = timer_add_next_day(pctimer);
	}

	log_debug("pctimer->week=%02x pctimer->week_cal=%02x\n", 
		pctimer->week, pctimer->week_cal);
	pcth = &air_ctrl->com_udp_dev_info.comm_timer_head;

	len = (pcth->timer_count + 1)*sizeof(cl_comm_timer_t);	
	pctimer_tmp = cl_calloc(len, 1);
	if (!pctimer_tmp) {
		return;
	}
	memcpy((void *)pctimer_tmp, (void *)pcth->timer, len - sizeof(cl_comm_timer_t));
	SAFE_FREE(pcth->timer);
	pcth->timer = pctimer_tmp;
	memcpy(&pcth->timer[pcth->timer_count], pctimer, sizeof(*pctimer));
	pcth->timer_count++;	
	comm_timer_next_cal(pcth);
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    //event_cancel_merge(user->handle);
}


static void _comm_timer_add_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_comm_timer_t *pctimer = (cl_comm_timer_t *)pdata;
	ucp_comm_timer_t *pntimer = (ucp_comm_timer_t *)&uo[1];
	u_int16_t len = 0;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	cl_comm_timer_head_t *pcthd;

#if 0
	_comm_test_timer(user, pctimer);
#else
	if (type == CLNE_COMM_TIMER_ADD) {
		//定时器冲突判断
		sac = user->smart_appliance_ctrl;
		air_ctrl = sac->sub_ctrl;	
		pcthd = &air_ctrl->com_udp_dev_info.comm_timer_head;

		pcthd->is_slave = false;
		pcthd->sub_type = user->sub_type;
		pcthd->ext_type = user->ext_type;
		
		if (comm_timer_is_conflict(user, pcthd, pctimer)) {
			log_debug("%s timer conflict\n", __FUNCTION__);
			return;
		}
		
		len = ucp_comm_fill_timer_modify_pkt(user, false, 0, pntimer, pctimer);
		if (len == 0) {
			log_debug("_comm_timer_add_proc error !!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
			return;
		}
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_COM_TIMER_PROC, len);
		sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + len);
	} else {
		pntimer->id = pctimer->id;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_COM_TIMER_PROC, 4);
		sa_ctrl_obj_value(user->uc_session, UCA_DELETE, false, 0x1, uo, sizeof(*uo) + 4);
	}
#endif	
}

static void _comm_smart_on_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *value = (u_int8_t*)&uo[1];
	
	*value = pdata[0];
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_SMART_ON, 4);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + 4);
}

static void _comm_child_lock_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_child_lock_t *lock = (ucp_child_lock_t *)&uo[1];

	lock->action = pdata[0];
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_CHILD_LOCK, sizeof(*lock));
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(*lock));
}

static void _comm_boot_temp_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	u_int16_t *value = (u_int16_t *)pdata;
	ucp_obj_t* uo = (ucp_obj_t*)buf;	
	u_int8_t *set = (u_int8_t *)&uo[1];

	set[0] = (*value) & 0xff;
	set[1] = ((*value) >> 8) & 0xff;
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_BOOT_TEMP, 4);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + 4);
}

static void _comm_wan_config_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_wan_config_item_t *request = (cl_wan_config_item_t *)&uo[1];

	if (data_len < sizeof(*request)) {
		return;
	}

	memcpy(request, pdata, data_len);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_WAN_CONFIG, data_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo) + data_len);
}

static void _comm_dhcp_config_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_dhcp_server_config_t *request = (cl_dhcp_server_config_t *)&uo[1];

	if (data_len < sizeof(*request)) {
		return;
	}

	memcpy(request, pdata, data_len);
	
	request->time = ntohl(request->time);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_DHCP_SERVER, data_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo) + data_len);
}

static void _comm_ap_config_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_ap_config_t *request = (cl_ap_config_t *)&uo[1];

	if (data_len < sizeof(*request)) {
		return;
	}

	memcpy(request, pdata, data_len);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_AP_CONFIG, data_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo) + data_len);
}

static void _comm_repeat_config_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *request = (u_int8_t *)&uo[1];

	if (data_len < sizeof(*request)) {
		return;
	}

	memcpy(request, pdata, data_len);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_REPEATER, data_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, true, 0x1, uo, sizeof(*uo) + data_len);
}

static void _comm_temp_alarm_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_temp_alarm_t *ta = (ucp_temp_alarm_t *)&uo[1];

	if (data_len < sizeof(*ta)) {
		return;
	}

	memcpy(ta, pdata, sizeof(*ta));
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_TEMP_ALARM, sizeof(*ta));
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(*ta));
}

static void _comm_history_query_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_dev_history_item_hd_t *in= (cl_dev_history_item_hd_t *)pdata;
	ucp_dev_comm_history_request_t *request = (ucp_dev_comm_history_request_t *)&uo[1];

	if (data_len < sizeof(*in)) {
		return;
	}

	request->index = ntohl(in->index);
	request->num = ntohl(in->num);
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_HISTORY_ITEM, sizeof(*request));
	sa_ctrl_obj_value(user->uc_session, UCA_GET, false, 0x1, uo, sizeof(*uo) + sizeof(*request));
}

static void _comm_temp_ctrl_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;	
    timer_switch_t t;
	cl_temp_ac_ctrl_t *request = (cl_temp_ac_ctrl_t *)pdata, *tc;
	
	memcpy((u_int8_t*)&uo[1], request, sizeof(*request));

	tc = (cl_temp_ac_ctrl_t*)(uo+1);
    t.hour = tc->begin_hour;
    t.week = tc->week;
    _com_timer_local_2_utc(&t);
    tc->begin_hour = t.hour;
    tc->week = t.week;
    
    t.hour = tc->end_hour;
    _com_timer_local_2_utc(&t);
    tc->end_hour = t.hour;
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_UTC_TMP_CTRL, sizeof(*request));
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(*request));
}

static void _comm_temp_curve_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	timer_switch_t t;
	cl_temp_curve_t *request = (cl_temp_curve_t *)pdata, *tc;
	
	memcpy((u_int8_t*)&uo[1], request, data_len);

	tc = (cl_temp_curve_t*)(uo+1);
        
    t.hour = tc->begin_hour;
    t.week = tc->week;
    _com_timer_local_2_utc(&t);
    tc->begin_hour = t.hour;
    tc->week = t.week;
    
    t.hour = tc->end_hour;
    _com_timer_local_2_utc(&t);
    tc->end_hour = t.hour;
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_TMP_UTC_CURVE, data_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + data_len);
}

static RS _comm_shortcuts_onoff_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_shortcuts_onoff_t *request = (ucp_shortcuts_onoff_t *)pdata;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;

	sac = user->smart_appliance_ctrl;
	air_ctrl = (smart_air_ctrl_t*)sac->sub_ctrl;

#if 1	
	//这里要判断一下，没有红外编码学习 悟空的话要判断一下是否匹配编码
	if (user->sub_type == IJ_SMART_PLUG) {
		if (!air_ctrl->air_info.is_match_code_valid) {
			return RS_IR_SYNING;
		}
		if (!air_ctrl->air_info.is_match_code) {
			return RS_IR_HAVE_NO_MATCH;
		}
	}
#endif	
	
	request->remain_time = ntohs(request->remain_time);

	memcpy((u_int8_t*)&uo[1], request, sizeof(*request));
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_SHORTCUT_TIMER, sizeof(*request));
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + sizeof(*request));

	return RS_OK;
}

static bool file_24hour_line_fill(u_int8_t *pdata, int len, char *file)
{
	char buff[1024];
	int index = 0;
	FILE *fp = NULL;
	int value = 0;

	fp = fopen(file, "r");
	if (!fp) {
		return false;
	}

	while(fgets(buff, sizeof(buff), fp)) {
		if (sscanf(buff, "%d=%d", &index, &value)== 2) {
			if (index >= len) {
				continue;
			}

			pdata[index] = (u_int8_t)value;
		}
	}

	if (fp) {
		fclose(fp);
	}

	return true;
}

static RS _comm_tmp_24hour_line_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pcurve = (u_int8_t *)&uo[1];
	int send_len = 24*6;

	if (!file_24hour_line_fill(pcurve, send_len, (char *)pdata)) {
		return RS_ERROR;
	}
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_TMP_SAMPLE_CURVE, send_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + send_len);

	return RS_OK;
}

static RS _comm_humi_24hour_line_proc(user_t *user, u_int8_t *pdata, u_int32_t data_len, u_int32_t type)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pcurve = (u_int8_t *)&uo[1];
	int send_len = 24*6;
	
	if (!file_24hour_line_fill(pcurve, send_len, (char *)pdata)) {
		return RS_ERROR;
	}
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_RH_SAMPLE_CURVE, send_len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo) + send_len);

	return RS_OK;
}

static pkt_t *sa_dev_set_irid_pkt(ucc_session_t *s, u_int32_t id)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	int len = sizeof(ucp_code_process_request_t);
	ucp_code_process_request_t *pcpr;

	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return NULL;
	}
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->count = 1;
	uc->reserved = 0;
	uc->action = UCA_SET;

	uo = (ucp_obj_t *)(uc+1);
	pcpr = (ucp_code_process_request_t *)&uo[1];
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_PROCESS,len);
	pcpr->action = CODE_PROCESS_ACTION_SET;
	pcpr->id = htons((u_int16_t)id);
	
	return pkt;
}

static RS sa_dev_set_irid(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	pkt_t *pkt = NULL;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	pkt = sa_dev_set_irid_pkt(user->uc_session, up->ip);
	if (pkt) {
		ucc_request_add(user->uc_session, pkt);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static pkt_t *sa_dev_set_timezone_pkt(ucc_session_t *s, u_int32_t timezone)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	int len;
	u_int8_t *pcpr;
	smart_air_ctrl_t* air_ctrl;
	smart_appliance_ctrl_t* sac;

	sac = s->user->smart_appliance_ctrl;
	if (!sac) {
		return NULL;
	}

	air_ctrl = sac->sub_ctrl;
	if (!air_ctrl) {
		return NULL;
	}

	len = sizeof(timezone);
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return NULL;
	}
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->count = 1;
	uc->reserved = 0;
	uc->action = UCA_SET;

	uo = (ucp_obj_t *)(uc+1);
	pcpr = (u_int8_t *)&uo[1];
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_TIMEZONE,len);
	timezone = htonl(timezone);
	memcpy(pcpr, (u_int8_t *)&timezone, len);;
	
	return pkt;
}

static RS sa_dev_set_timezone(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	pkt_t *pkt = NULL;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	pkt = sa_dev_set_timezone_pkt(user->uc_session, up->ip);
	if (pkt) {
		ucc_request_add(user->uc_session, pkt);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static void sa_dev_set_scc_onoff_pkt(ucc_session_t *s, u_int32_t onoff)
{
	u_int8_t buf[1024] = {0};
	int len = sizeof(u_int8_t);
	ucp_obj_t* uo = (ucp_obj_t *)buf;
	u_int8_t *pcpr = (u_int8_t *)&uo[1];
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_AC, UCAT_AC_POWERCHECK,len);
	*pcpr = (u_int8_t)onoff;
	
	sa_ctrl_obj_value(s, UCA_SET, true, 1, buf, sizeof(*uo) + len);
}

static RS sa_dev_set_scc_onoff(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	sa_dev_set_scc_onoff_pkt(user->uc_session, up->ip);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static pkt_t *sa_dev_stm_erase_pkt(ucc_session_t *s, u_int32_t onoff)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return NULL;
	}
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->count = 1;
	uc->reserved = 0;
	uc->action = UCA_SET;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_UPGRADE, UCSOT_UPGRADE_FLASH, UCAT_FLASH_ERASE_STM,0);
	
	return pkt;
}

static RS sa_dev_stm_erase(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	pkt_t *pkt = NULL;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	pkt = sa_dev_stm_erase_pkt(user->uc_session, up->ip);
	if (pkt) {
		ucc_request_add(user->uc_session, pkt);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static int uc_put_tlv(u_int8_t *buf, int size, int *pos, u_int16_t type, u_int16_t len, u_int8_t *data)
{
	u_int16_t *ptr;

	if (*pos + (int)DBG_TLV_HD_LEN + len > size) {
		log_debug("error len size=%d pos=%d\n", size, *pos);
		return -1;
	}
	
	ptr = (u_int16_t *)(buf + (*pos));
	*ptr = htons(type);
	ptr++;

	*ptr = htons(len);
	ptr++;
	if (len > 0) {	
		memcpy(ptr, data, len);
	}

	*pos = (*pos) + DBG_TLV_HD_LEN + len;

	return 0;
}


static void sa_dev_debug_proc(ucc_session_t *s, cln_user_t *up)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	char buff[MAX_UC_PKT_SIZE];
	int buff_len = 0;
	cl_air_debug_info_set_t *pinfo = (cl_air_debug_info_set_t *)up->data;

	uc_put_tlv(buff, MAX_UC_PKT_SIZE, &buff_len, up->len, sizeof(*pinfo), (u_int8_t *)pinfo);
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc) + sizeof(*uo) + buff_len,
                     true, true, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return;
	}
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_DEBUGINFO, buff_len);

	memcpy((void *)(uo+1), buff, buff_len);

	ucc_request_add(s, pkt);
}

static RS sa_dev_debug_set(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	sa_dev_debug_proc(user->uc_session, up);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

static u_int32_t sa_day_ele_get_time()
{
	struct tm lt;
	time_t now;

	time(&now);
	gmtime_r(&now, &lt);

	lt.tm_hour = 0;
	lt.tm_min = 0;
	lt.tm_sec = 0;

	return (u_int32_t)mktime(&lt);
}

static int sa_day_ele_build(ucc_session_t *s, char *filename, char *buff)
{
	FILE *fp = NULL;
	char buf_line[128];
	int index;
	u_int32_t value;
	ucp_elec_days_stat* ds = (ucp_elec_days_stat *)buff;
    	
	if (!s || !filename) {
		return 0;
	}
	fp = fopen(filename, "r");
	if (!fp) {
		return 0;
	}

	ds->nearest_data_time = ntohl(sa_day_ele_get_time());
	ds->days_count = htons(365);
	
	while(fgets(buf_line, sizeof(buf_line), fp)) {
		if (buf_line[0]== '#') {
			continue;
		}
		if (sscanf(buf_line, "%d=%u", &index, &value) == 2 &&
			index < 365) {
			ds->elec_data[index] = htons((u_int16_t)value);
		}
	}

	if (fp) {
		fclose(fp);
		fp = NULL;
	}

	return (int)(sizeof(*ds)+365*sizeof(u_int16_t));
}

void sa_dev_day_ele_import_pkt(ucc_session_t *s, char *filename)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	char buff[MAX_UC_PKT_SIZE];
	int buff_len = 0;

	memset(buff, 0, sizeof(buff));
	buff_len = sa_day_ele_build(s, filename, buff);
	if (buff_len <= 0) {
		return;
	}
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + buff_len,
                     true, true, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return;
	}
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	//fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_STAT, UCAT_STAT_DAYS_STAT, buff_len);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_COMMON_STAT, UCAT_STAT_DAYS_STAT, buff_len);

	memcpy((void *)(uo+1), buff, buff_len);
	sa_set_obj_value(s, 0x1, uo, sizeof(uo) + buff_len);
}

static RS sa_dev_days_ele_import(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	sa_dev_day_ele_import_pkt(user->uc_session, up->data);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}


bool _misc_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	user_t *user;
	cln_misc_info_t* smi;
  
	smi = (cln_misc_info_t*)pkt->data;
	
	
	user = lookup_by_handle(HDLT_USER, smi->dev_handle);
	if (!user || !user->smart_appliance_ctrl ) {
		log_err(false, "_misc_proc_notify error handle %08x\n",smi->dev_handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	*ret = RS_OK;
	switch(pkt->type){
		case CLNE_COMM_TIMER_ADD:
		case CLNE_COMM_TIMER_DEL:
			_comm_timer_add_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_SMART_ON:
			_comm_smart_on_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_CHILD_LOCK:
			_comm_child_lock_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_BOOT_TEMP:
			_comm_boot_temp_proc(user, smi->data, smi->data_len, pkt->type);
			break;		
		case CLNE_COMM_WAN_CONFIG:
			_comm_wan_config_proc(user, smi->data, smi->data_len, pkt->type);
			break;	
		case CLNE_COMM_DHCP_CONFIG:
			_comm_dhcp_config_proc(user, smi->data, smi->data_len, pkt->type);
			break;	
		case CLNE_COMM_AP_CONFIG:
			_comm_ap_config_proc(user, smi->data, smi->data_len, pkt->type);
			break;	
		case CLNE_COMM_REPEAT_CONFIG:
			_comm_repeat_config_proc(user, smi->data, smi->data_len, pkt->type);
			break;	
		case CLNE_COMM_TEMP_ALARM:
			_comm_temp_alarm_proc(user, smi->data, smi->data_len, pkt->type);
			break;			
		case CLNE_COMM_SHORTCUTS_ONOFF:
			*ret = _comm_shortcuts_onoff_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_TEMP_CTRL:
			_comm_temp_ctrl_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_TEMP_CURVE:
			_comm_temp_curve_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_HISTORY_QUERY:
			_comm_history_query_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_TMP_24HOUR_LINE_IMPORT:
			*ret = _comm_tmp_24hour_line_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_HUMI_24HOUR_LINE_IMPORT:
			*ret = _comm_humi_24hour_line_proc(user, smi->data, smi->data_len, pkt->type);
			break;
		case CLNE_COMM_IRID_SET:
			*ret = sa_dev_set_irid(pkt);
			break;
		case CLNE_COMM_TIMEZONE_SET:
			*ret = sa_dev_set_timezone(pkt);
			break;
		case CLNE_COMM_SCC_ONOFF_SET:
			*ret = sa_dev_set_scc_onoff(pkt);
			break;
		case CLNE_COMM_STM_ERASE:
			*ret = sa_dev_stm_erase(pkt);
			break;
		case CLNE_COMM_DEBUG_CONF:
			*ret = sa_dev_debug_set(pkt);
			break;
		case CLNE_COMM_DAYS_ELE_IMPORT:
			*ret = sa_dev_days_ele_import(pkt);
			break;
		default:
            *ret = RS_ERROR;
			break; 
	}
	return true;
}


bool misc_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    if (pkt->type < CLNE_MISC_START || pkt->type > CLNE_MISC_END) {
		return udp_proc_notify_hook(pkt,ret);
	}
    
    return _misc_proc_notify(pkt, ret);
    
}

bool la_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    if (pkt->type < CLNE_LA_START || pkt->type > CLNE_LA_END) {
		return false;
	}
    
    return _la_proc_notify(pkt->data, ret);
    
}

RS sa_init(user_t *user)
{
    smart_appliance_ctrl_t* sac;
    RS ret = RS_ERROR;
    smart_air_ctrl_t* ac;
    
    if (!user || user->is_phone_user || !user->sub_type ) {
        return RS_OK;
    }

    sac = user->smart_appliance_ctrl;
    if(sac && sac->sub_ctrl)
		return RS_OK;
	
    if(!sac){
        sac = cl_calloc(sizeof(*sac), 1);
    }
    
    if (!sac) {
        return RS_ERROR;
    }
    sac->user = user;
    
    switch (user->sub_type) {
        case IJ_SMART_PLUG://智能插座
        case IJ_812:
        case IJ_816:
        case IJ_813:
        case IJ_820:
        case IJ_101:
        case IJ_102:
        case IJ_821:	
        case IJ_RFGW:
            ret = air_init(sac);
            break;
        default:
            break;
    }

   if(ret!= RS_OK && is_supported_udp_device(user->sub_type,user->ext_type)){
   	ret = air_init(sac);
   }
    
    if (ret != RS_OK) {
        cl_free(sac);
        return ret;
    }

    if(is_supported_udp_device(user->sub_type,user->ext_type)){
        if(!udp_init_ac_hook(user,sac->sub_ctrl)){
            air_free(sac->sub_ctrl);
            cl_free(sac);
            return RS_MEMORY_MALLOC_FAIL;
        }
    }

    if (user->sub_type == IJ_SMART_PLUG && user->ext_type == ETYPE_IJ_SMP_H_ENH_16A) {
        ac = (smart_air_ctrl_t* )sac->sub_ctrl;
        ac->com_udp_dev_info.is_support_rc = true;
    }

	if (user->sub_type == IJ_SMART_PLUG && user->ext_type == ETYPE_IJ_SMP_H_ENH_16A_NO_INPUT) {
        ac = (smart_air_ctrl_t* )sac->sub_ctrl;
        ac->com_udp_dev_info.is_support_rc = true;
    }	
    
    user->smart_appliance_ctrl = sac;
    
    return RS_OK;
}

void sa_free(user_t *user)
{
    smart_appliance_ctrl_t* sac;
    if (!user || user->is_phone_user || !user->smart_appliance_ctrl) {
        return ;
    }
    
    sac = (smart_appliance_ctrl_t*)user->smart_appliance_ctrl;

    if( is_supported_udp_device(user->sub_type,user->ext_type)){
        air_free(sac->sub_ctrl);
    }else{
        switch (user->sub_type) {
            case IJ_SMART_PLUG://智能插座
            case IJ_812:
            case IJ_816:
            case IJ_813:
            case IJ_820:
                air_free(sac->sub_ctrl);
                break;
            default:
                break;
        }
    }

    SAFE_FREE(sac->all_mib);
    SAFE_FREE(user->smart_appliance_ctrl);
}

void comm_timer_build(user_t* user,cl_dev_info_t* ui)
{
	int i, j, len, n;
	cl_comm_timer_t *ptimer = NULL;
	cl_comm_timer_head_t *pcthd_sdk = NULL;
	cl_comm_timer_head_t *pcthd_app = NULL;
	cl_dev_timer_summary_t *pcts_sdk = NULL;
	cl_dev_timer_summary_t *pcts_app = NULL;
    smart_appliance_ctrl_t* sac = NULL;
	smart_air_ctrl_t *ac = NULL;
	
    if (!user || user->is_phone_user || !user->smart_appliance_ctrl) {
        return;
    }
	sac = (smart_appliance_ctrl_t*)user->smart_appliance_ctrl;
	ac = sac->sub_ctrl;
	if (!ac) {
		return;
	}
	if (!ui->com_udp_info) {
		return;
	}
	pcthd_sdk = &ac->com_udp_dev_info.comm_timer_head;
	pcthd_app = &ui->com_udp_info->comm_timer_head;
	pcts_sdk = &ac->com_udp_dev_info.timer_summary;
	pcts_app = &ui->com_udp_info->timer_summary;
	pcts_app->stat_count = NULL;

	memcpy((void *)pcts_app, (void *)pcts_sdk, sizeof(*pcts_app));
	memcpy((void *)pcthd_app, (void *)pcthd_sdk, sizeof(*pcthd_app));
	pcthd_app->timer_count = 0;
	pcthd_app->timer = NULL;

	//判断有效个数
	for(i = 0, n = 0; i < pcthd_sdk->timer_count; i++) {
		if (pcthd_sdk->timer[i].valid) {
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
	
	pcthd_app->timer = ptimer;
	pcthd_app->timer_count = n;

	for(i = 0, j = 0; i < pcthd_sdk->timer_count && j < n; i++) {
		if (pcthd_sdk->timer[i].valid) {
			memcpy((void *)&pcthd_app->timer[j], (void *)&pcthd_sdk->timer[i], sizeof(cl_comm_timer_t));
			j++;
		}
	}
}
void sa_build_objs(user_t* user,cl_dev_info_t* ui)
{
    ucc_session_t* s = user->uc_session;
    smart_appliance_ctrl_t* sma;
    smart_air_ctrl_t* ac;
    
    
    if (!ui) {
        return;
    }
    
    if (s && s->has_share_key) {
        if ( (sma = user->smart_appliance_ctrl) == NULL) {
            return;
        }
        
        if (!(ac = sma->sub_ctrl) ) {
            return;
        }
        
        ac->air_info.share_info.is_share_data_valid = true;
        ac->air_info.share_info.is_super_user = (s->phone_index == SUPER_PHONE_INDEX)?true:false;
        ac->air_info.share_info.cur_phone_index = s->phone_index;
        ac->air_info.share_info.v1_remain_days = s->v1_remain_days;
    }
    
    switch (user->sub_type) {
        case IJ_SMART_PLUG://智能插座
            air_build_objs(user, ui);
            break;
        case IJ_812:
            ah_build_objs(user, ui);
            break;
        case IJ_101:
            eb_build_objs(user, ui);
            break;
        case IJ_RFGW:
            //rfgw_build_objs(user, ui);
            break;
        default:
            break;
    }
	
   if( is_supported_udp_device(user->sub_type,user->ext_type)){
   	udp_build_objs_hook(user,ui);
   }
  
}


cl_dev_stat_t* sa_get_dev_stat_info(user_t* user)
{
    smart_appliance_ctrl_t* sac;
    if (!user || user->is_phone_user  || !user->smart_appliance_ctrl) {
        return NULL;
    }
    
    sac = (smart_appliance_ctrl_t*)user->smart_appliance_ctrl;
    if (!sac->sub_ctrl) {
        return NULL;
    }
    
    return air_get_dev_stat_info(sac->sub_ctrl);
}

cl_pt_stat_t* sa_get_pt_stat_info(user_t* user)
{
    smart_appliance_ctrl_t* sac;
    smart_air_ctrl_t *ac;
    cl_pt_stat_t* pt = NULL;

    if (!user || user->is_phone_user  || !user->smart_appliance_ctrl) {
        return NULL;
    }
    
    sac = (smart_appliance_ctrl_t*)user->smart_appliance_ctrl;
    if (!sac->sub_ctrl) {
        return NULL;
    }
    pt = cl_calloc(sizeof(*pt), 1);
    if(pt == NULL)
        return NULL;
    ac = (smart_air_ctrl_t*)sac->sub_ctrl;
    memcpy(pt, &ac->pt_stat, sizeof(*pt));
    air_quick_query_pt_stat_info(ac);
    return pt;
}

RS sa_user_modify_passwd(user_t* user,char* pwd_md5)
{
	 char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    char* dest_name = (char*)(uo+1);
    smart_appliance_ctrl_t* sac;
    smart_air_ctrl_t* ac;
    int pwd_len = 16;
    
    if (!(sac = user->smart_appliance_ctrl) || !pwd_md5 ) {
        return RS_INVALID_PARAM;
    }
    
    mem_dump("want modify password to\n",pwd_md5, 16);
	
    if (sac->sub_ctrl ) {
        ac = sac->sub_ctrl;
        fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_PASSWORD,pwd_len);
        memcpy(dest_name, pwd_md5, pwd_len);
        
        sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+pwd_len);
        
        return RS_OK;
    }
    
    return RS_INVALID_PARAM;
}

RS sa_user_reboot_device(user_t* user)
{
    smart_appliance_ctrl_t* sac;
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int8_t* reboot = (u_int8_t*)(uo+1);
    smart_air_ctrl_t* ac;
    
    if (!user || !user->is_udp_ctrl) {
        return RS_INVALID_PARAM;
    }
    
    if (!user->online) {
        return RS_NOT_LOGIN;
    }
    
    if (!(sac = user->smart_appliance_ctrl) || ! (ac = sac->sub_ctrl) ) {
        return RS_INVALID_PARAM;
    }
    
    fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_REBOOT, sizeof(*reboot));
    *reboot = 0x1;
    
    sa_ctrl_obj_value(user->uc_session,UCA_SET,false, 0x1, buf, sizeof(*uo)+sizeof(*reboot));
    
    return RS_OK;
}

RS sa_user_telnet_device(user_t* user, cln_user_t *up)
{
    smart_appliance_ctrl_t* sac;
    char buf[64] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
	remote_vty_t *nb = (remote_vty_t *)(uo+1);
    smart_air_ctrl_t* ac;
    
    if (!user || !user->is_udp_ctrl) {
        return RS_INVALID_PARAM;
    }
    
    if (!user->online) {
        return RS_NOT_LOGIN;
    }
    
    if (!(sac = user->smart_appliance_ctrl) || ! (ac = sac->sub_ctrl) ) {
        return RS_INVALID_PARAM;
    }
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_PUBLIC, UCAT_IA_PUBLIC_TELNET, sizeof(*nb));
    nb->ip = htonl(up->ip);
	nb->port = htons(up->port);
    
    sa_ctrl_obj_value(user->uc_session,UCA_SET,false, 0x1, buf, sizeof(*uo)+sizeof(*nb));
    
    return RS_OK;
}


RS sa_user_modify_nick_name(user_t* user,char* name,u_int32_t name_len)
{
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    char* dest_name = (char*)(uo+1);
    smart_appliance_ctrl_t* sac;
    smart_air_ctrl_t* ac;
    
    if (!(sac = user->smart_appliance_ctrl)) {
        return RS_INVALID_PARAM;
    }
    
    if (sac->sub_ctrl ) {
        ac = sac->sub_ctrl;
        
        fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_HOSTNAME, UCAT_HOSTNAME_HOSTNAME, name_len+1);
        strncpy(dest_name, name, name_len);
        
        sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+name_len+1);
        
        return RS_OK;
    }
    
    return RS_INVALID_PARAM;
}

RS sa_query_objects(ucc_session_t *s,ucp_obj_t* objs,int count)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    ucp_obj_t* uo;
    int i;
   
    if (!s || !objs ||count <= 0) {
        return RS_ERROR;
    }
    
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+sizeof(ucp_obj_t)*count,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_GET;
    uc->count = count;
    uo = (ucp_obj_t*)(uc+1);
    for (i = 0; i < count; i++,uo++) {
        fill_net_ucp_obj(uo, objs[i].objct, objs[i].sub_objct, objs[i].attr, 0);
    }
    
    ucc_request_add(s, pkt);
    
    return RS_OK;
}

RS sa_ctrl_obj_value(ucc_session_t *s,u_int8_t action,bool need_query,u_int8_t obj_count,void* content,int content_len)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    
    if (!s || !content ||content_len <= 0) {
        return RS_ERROR;
    }
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+content_len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = action;
    uc->count = obj_count;
    memcpy((void*)(uc+1), content, content_len);
    
    ucc_request_add(s, pkt);
    
    if (need_query) {
        sa_quick_query_single_object(s, content);
    }
    
    return RS_OK;
}

/*
sa_set_obj_value
set带查询
*/
RS sa_set_obj_value(ucc_session_t *s,u_int8_t obj_count,void* content,int content_len)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    
    if (!s || !content ||content_len <= 0) {
        return RS_ERROR;
    }
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+content_len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
    uc->count = obj_count;
    memcpy((void*)(uc+1), content, content_len);
    
    ucc_request_add(s, pkt);
    
    sa_quick_query_single_object(s,content);
    
    return RS_OK;
}

/*
sa_set_obj_value_only
set不带查询
*/
RS sa_set_obj_value_only(ucc_session_t *s,u_int8_t obj_count,void* content,int content_len)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    
    if (!s || !content ||content_len <= 0) {
        return RS_ERROR;
    }
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t)+content_len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
    uc = get_ucp_payload(pkt, ucp_ctrl_t);
    uc->action = UCA_SET;
    uc->count = obj_count;
    memcpy((void*)(uc+1), content, content_len);
    
    ucc_request_add(s, pkt);    
       
    return RS_OK;
}

RS sa_get_file_len(char *file, int *len)
{
	struct stat st;
	if(stat(file, &st) != 0)
		return RS_INVALID_PARAM;
	*len = (int)st.st_size;	
	return RS_OK;
}

pkt_t *sa_new_upgrade_pkt(ucc_session_t *s, uc_upgrade_block_t **ppub, int block_sz, u_int16_t block_idx)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	uc_upgrade_block_t *ub;
	
	pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo)+sizeof(*ub)+block_sz,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id+(u_int8_t)block_idx);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_UPGRADE, (short)(sizeof(*ub) + block_sz));
	ub = (uc_upgrade_block_t*)(uo+1);
	*ppub = ub;
	return pkt;
}

static RS sa_hex2str(u_int8_t *hex, int hex_len, char *str, int *str_len)
{
	int i, len = 0;
	
	if (*str_len < 2 * hex_len) {
		return RS_ERROR;
	}
	*str_len = 2 * hex_len;

	for (i = 0; i < hex_len; i++) {
		len += sprintf(str + len, "%02X", hex[i]);
	}

	return RS_OK;
}

RS ckeck_image_hdr(user_t *user, image_hdr_t *hdr)
{
	char developer_id[32] = {0};
	int did_len = 32;
	if(hdr->magic != ntohl(IMAGE_MAGIC)){
		return RS_INVALID_PARAM;
	}
	if(hdr->ds_type != TP_DS007
		|| (hdr->dev_type != user->sub_type && hdr->dev_type!=user->real_sub_type)
		|| (hdr->ext_type != user->ext_type && hdr->ext_type!= user->real_ext_type)){
		log_err(false, "image type check fail: %hhu.%hhu.%huu , device is %hhu.%hhu.%hhu  device real is %hhu.%hhu.%hhu\n",
			hdr->ds_type, hdr->dev_type, hdr->ext_type, 
			TP_DS007, user->sub_type, user->ext_type,
                TP_DS007, user->real_sub_type, user->real_sub_type);
		return RS_INVALID_PARAM;
	}
	if(user->vendor_id == NULL || strcmp(hdr->oem, user->vendor_id) !=0 ){
		log_err(false, "image oem check fail: %s , device is %s\n",
			hdr->oem, user->vendor_id ? user->vendor_id: "NULL");
		return RS_INVALID_PARAM;
	}

	if (user->developer_id[0]) {
		if (sa_hex2str(hdr->developer_id, sizeof(hdr->developer_id), developer_id, &did_len) == RS_OK) {
			if (strncmp(developer_id, user->developer_id, sizeof(developer_id))) {
				log_err(false, "image devloper_id check failed: image's [%s] user's [%s]\n", developer_id, user->developer_id);
				return RS_INVALID_PARAM;
			}
		}
	}
	
	return RS_OK;
}

RS sa_check_upgrade_file(user_t *user, char *file, int len, struct stlc_list_head *pktlst)
{
	FILE *fp;
	int n;
	MD5_CTX ctx;
	pkt_t *pkt, *next;
	RS ret = RS_INVALID_PARAM;
	unsigned char digest[16];
	unsigned short total, current;
	uc_upgrade_block_t *ub;
	ucc_session_t *s = user->uc_session;
	char *data;
		
	image_hdr_t hdr;
	fp = fopen(file, "rb");
	if(fp==NULL){
		return RS_INVALID_PARAM;
	}


	if(fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)){
		goto done;
	}
	if(ckeck_image_hdr(user, &hdr) != RS_OK)
		goto done;


	MD5Init(&ctx);	
	total = 1 + len/IMAGE_BLOCK_SZ;
	if(len%IMAGE_BLOCK_SZ)
		total++;
	for(current = 1; current <= (total-1); current++){
		if(len >= IMAGE_BLOCK_SZ){
			n = IMAGE_BLOCK_SZ;
			len -= IMAGE_BLOCK_SZ;
		}else{
			n = len;
			len = 0;
		}
		
		pkt = sa_new_upgrade_pkt(s, &ub, n, current-1);
		if(pkt == NULL)
			goto done;
		pkt->up_total = total;
		pkt->up_current = current;
		ub->total = ntohs(total);
		ub->current = ntohs(current);
		data = (char*)(ub+1);
		if(fread(data, 1, n, fp) != n)
			goto done;
		MD5Update(&ctx, (unsigned char *)data, (unsigned int)n);
		stlc_list_add_tail(&pkt->link, pktlst);
	}	
	MD5Final(digest, &ctx);	

	if(memcmp(digest, hdr.checksum, 16) != 0){
		log_err(false, "image checksum fail\n");
		goto done;
	}

	
	pkt = sa_new_upgrade_pkt(s, &ub, 16, current-1);
	if(pkt == NULL)
		goto done;
	pkt->up_total = total;
	pkt->up_current = total;
	ub->total = ntohs(total);
	ub->current = ntohs(current++);
	data = (char*)(ub+1);
	memcpy(data, digest, 16);
	stlc_list_add_tail(&pkt->link, pktlst);
	ret = RS_OK;
	
done:
	fclose(fp);
	if(ret != RS_OK){
		stlc_list_for_each_entry_safe(pkt_t, pkt, next, pktlst, link){
			stlc_list_del(&pkt->link);
			pkt_free(pkt);
		}
	}
	return ret;
}

RS sa_dev_upgrade_file(user_t *user, char *file)
{
	int len;
	pkt_t *pkt, *next;
	ucc_session_t *s = user->uc_session;
	struct stlc_list_head pktlst;

	STLC_INIT_LIST_HEAD(&pktlst);
	if (sa_get_file_len(file, &len) != RS_OK)
		return RS_INVALID_PARAM;
	if (len < (int)sizeof(image_hdr_t))
		return RS_INVALID_PARAM;
	
	len -= (int)sizeof(image_hdr_t);
	if (sa_check_upgrade_file(user, file, len, &pktlst) != RS_OK)
		return RS_INVALID_PARAM;
	s->up_total = 0;
	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &pktlst, link){
		stlc_list_del(&pkt->link);
		ucc_request_add(s, pkt);
		s->up_total++;
	}
	log_debug("add %hu upgrade pkt\n", s->up_total);
	
	return RS_OK;
}

RS sa_dev_upgrade_cli(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	up->data[up->len_name] = 0;
	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}
	ret = sa_dev_upgrade_file(user, up->data);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}


RS sa_check_upgrade_file_no_head(user_t *user, char *file, int len, struct 
stlc_list_head *pktlst)
{
	FILE *fp;
	int n;
	MD5_CTX ctx;
	pkt_t *pkt, *next;
	RS ret = RS_INVALID_PARAM;
	unsigned char digest[16];
	unsigned short total, current;
	uc_upgrade_block_t *ub;
	ucc_session_t *s = user->uc_session;
	char *data;
		
//	image_hdr_t hdr;
	fp = fopen(file, "rb");
	if(fp==NULL){
		return RS_INVALID_PARAM;
	}
#if 0
	if(fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)){
		goto done;
	}
	if(ckeck_image_hdr(user, &hdr) != RS_OK)
		goto done;
#endif
	MD5Init(&ctx);	
	total = 1 + len/IMAGE_BLOCK_SZ;
	if(len%IMAGE_BLOCK_SZ)
		total++;
	for(current = 1; current <= (total-1); current++){
		if(len >= IMAGE_BLOCK_SZ){
			n = IMAGE_BLOCK_SZ;
			len -= IMAGE_BLOCK_SZ;
		}else{
			n = len;
			len = 0;
		}
		
		pkt = sa_new_upgrade_pkt(s, &ub, n, current-1);
		if(pkt == NULL)
			goto done;
		pkt->up_total = total;
		pkt->up_current = current;
		ub->total = ntohs(total);
		ub->current = ntohs(current);
		data = (char*)(ub+1);
		if(fread(data, 1, n, fp) != n)
			goto done;
		MD5Update(&ctx, (unsigned char *)data, (unsigned int)n);
		stlc_list_add_tail(&pkt->link, pktlst);
	}	
	MD5Final(digest, &ctx);	
#if 0
	if(memcmp(digest, hdr.checksum, 16) != 0){
		log_err(false, "image checksum fail\n");
		goto done;
	}
#endif
	pkt = sa_new_upgrade_pkt(s, &ub, 16, current-1);
	if(pkt == NULL)
		goto done;
	pkt->up_total = total;
	pkt->up_current = total;
	ub->total = ntohs(total);
	ub->current = ntohs(current++);
	data = (char*)(ub+1);
	memcpy(data, digest, 16);
	stlc_list_add_tail(&pkt->link, pktlst);
	ret = RS_OK;
	
done:
	fclose(fp);
	if(ret != RS_OK){
		stlc_list_for_each_entry_safe(pkt_t, pkt, next, pktlst, link){
			stlc_list_del(&pkt->link);
			pkt_free(pkt);
		}
	}
	return ret;
}


RS sa_dev_upgrade_file_no_head(user_t *user, char *file)
{
	int len;
	pkt_t *pkt, *next;
	ucc_session_t *s = user->uc_session;
	struct stlc_list_head pktlst;



	STLC_INIT_LIST_HEAD(&pktlst);
	if (sa_get_file_len(file, &len) != RS_OK) {
		return -11;
			return RS_INVALID_PARAM;
		}
	if (len < (int)sizeof(image_hdr_t)) {
		return -12;
			return RS_INVALID_PARAM;
		}
	
	//len -= (int)sizeof(image_hdr_t);
	if (sa_check_upgrade_file_no_head(user, file, len, &pktlst) != RS_OK) {
		return -13;
		//return RS_INVALID_PARAM;
		}
	s->up_total = 0;
	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &pktlst, link){
		stlc_list_del(&pkt->link);
		ucc_request_add(s, pkt);
		s->up_total++;
	}
	log_debug("add %hu upgrade pkt\n", s->up_total);
	
	return RS_OK;
}

RS sa_dev_upgrade_cli_no_head(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	up->data[up->len_name] = 0;
	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}


	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}
	ret = sa_dev_upgrade_file_no_head(user, up->data);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

pkt_t *sa_dev_upgrade_flash_erase_pkt(ucc_session_t *s, u_int8_t num)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	u_int8_t *pdata = NULL;
	
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + sizeof(num),
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_UPGRADE, UCSOT_UPGRADE_FLASH, UCAT_FLASH_ERASE, sizeof(num));
	pdata = (u_int8_t *)(uo+1);
	*pdata = num;
	log_debug("sa_dev_upgrade_flash_erase_pkt num=%u\n", num);
	
	return pkt;
}

RS sa_dev_upgrade_erase(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	pkt_t *pkt = NULL;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}
	log_debug("sa_dev_upgrade_erase num=%u\n", up->num);
	pkt = sa_dev_upgrade_flash_erase_pkt(user->uc_session, up->num);
	if (pkt) {
		log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);		
		ucc_request_add(user->uc_session, pkt);
	} 

done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}

static u_int16_t StmCrc(u_int16_t CRC, u_int8_t *pData, u_int16_t sLen)
{  
   u_int16_t i = 0;
   
   while (sLen--)  //len是所要计算的长度
   {
       CRC = CRC^(int)(*pData++) << 8; //    
       for (i=8; i!=0; i--) 
       {
           if (CRC & 0x8000)   
               CRC = CRC << 1 ^ 0x1021;    
           else
               CRC = CRC << 1;
       }    
   }

   return CRC;
}

#define STM_READ_BUFF_MAX 		(1024)

int sa_stm_upgrade_file_check(user_t *user, char *file)
{
	int ret = ERROR_IMAGE;
	tail_t *ptail = NULL;
	tail2_t *ptail2 = NULL;
	char buff[STM_READ_BUFF_MAX];
	FILE *fp = NULL;
	struct stat stat0;
	int image_size = 0;
	int file_size = 0;
	u_int16_t crc = 0;
	int read_max = 0;
	int num = 0;
	u_int32_t size = 0;

	log_debug("enter %s %d file=%s\n", __FUNCTION__, __LINE__, file);
	if (stat(file, &stat0) != 0) {
		log_err(true, "stat failed\n");
		return ret;
	}

	file_size = stat0.st_size;
	if (file_size < sizeof(tail_t)) {
		log_err(false, "error file_size=%u\n", file_size);
		return ret;
	}

	image_size = file_size - sizeof(tail_t);

	fp = fopen(file, "rb");
	if (!fp) {
		log_err(true, "fopen failed\n");
		return ret;
	}

	log_debug("file=%s\n", file);
	log_debug("image_size=%d file_size=%d\n", image_size, file_size);
	while(image_size > 0) {
		if (image_size > STM_READ_BUFF_MAX) {
			read_max = STM_READ_BUFF_MAX;
		} else {
			read_max = image_size;
		}
		num = (int)fread(buff, read_max, 1, fp);
		if (num != 1) {
			log_err(true, "fread failed image_size=%d read_max=%d num=%d\n", 
				image_size, read_max, num);
			goto end;
		} 
		crc = StmCrc(crc, buff, read_max);
		image_size -= read_max;
	}
	
	if (fread(buff, sizeof(tail_t), 1, fp) != 1) {
		log_err(true, "fread tail failed\n");
		goto end;
	}
	ptail = (tail_t *)buff;
	ptail2 = (tail2_t *)buff;

	if ((memcmp(ptail2->magic1, MAGIC1_RF, 4) == 0) && (memcmp(ptail2->magic2, MAGIC2_RF, 2) == 0)) {
		user->is_rfstm_up = true;
	} else if ((memcmp(ptail->magic1, MAGIC1, 4) == 0) && (memcmp(ptail->magic2, MAGIC2, 4) == 0)) {
		user->is_rfstm_up = false;
	} else {
		log_err(false, "error magic");
		log_debug("magic1=%02x,%02x,%02x,%02x\n", 
			ptail->magic1[0], ptail->magic1[1], ptail->magic1[2], ptail->magic1[3]);
		log_debug("magic2=%02x,%02x,%02x,%02x\n", 
			ptail->magic2[0], ptail->magic2[1], ptail->magic2[2], ptail->magic2[3]);		
		goto end;
	}
	
	if (ptail->crc[0] != ((crc >> 8)&0xFF) ||
		ptail->crc[1] != (crc&0xFF)) {
		log_err(false, "crc error netcrc=%u , calcrc=%u\n", 
			((ptail->crc[0] << 8) | ptail->crc[1]), crc);
		goto end;
	}

	if (user->is_rfstm_up) {
		size = ptail2->size_h[0]<<24 | ptail2->size_h[1]<<16 | ptail2->size[0]<<8 | ptail2->size[1];
	} else {
		size = ptail->size[0]<<8 | ptail->size[1];
	}

	if (size != (file_size - sizeof(tail_t))) {
		log_err(false, "len error size=%u recv_len=%u\n", size, file_size);
		goto end;
	}

	ret = 0;
	
end:
	if (fp) {
		fclose(fp);
		fp  = NULL;
	}

	return ret;	
}

pkt_t *sa_stm_new_upgrade_pkt(ucc_session_t *s, uc_upgrade_block_t **ppub, int block_sz, u_int32_t block_index)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	uc_upgrade_block_t *ub;
	
	pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo)+sizeof(*ub)+block_sz,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id + block_index);
	if(pkt == NULL)
		return NULL;
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	if (s->user && s->user->is_rfstm_up) {
		fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_STM2_UPGRADE, (short)(sizeof(*ub) + block_sz));
	} else {
		fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_STM_UPGRADE, (short)(sizeof(*ub) + block_sz));
	}
	ub = (uc_upgrade_block_t*)(uo+1);
	*ppub = ub;
	return pkt;
}


RS sa_stm_check_upgrade_file(user_t *user, char *file, int len, struct stlc_list_head *pktlst)
{
	FILE *fp;
	int n;
	MD5_CTX ctx;
	pkt_t *pkt, *next;
	RS ret = RS_INVALID_PARAM;
	unsigned char digest[16];
	unsigned short total, current;
	uc_upgrade_block_t *ub;
	ucc_session_t *s = user->uc_session;
	char *data;

	fp = fopen(file, "rb");
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);	
	if(fp==NULL){
		return RS_INVALID_PARAM;
	}

	MD5Init(&ctx);	
	total = 1 + len/IMAGE_BLOCK_SZ;
	if(len%IMAGE_BLOCK_SZ) {
		total++;
	}
	for(current = 1; current <= (total-1); current++){
		if(len >= IMAGE_BLOCK_SZ){
			n = IMAGE_BLOCK_SZ;
			len -= IMAGE_BLOCK_SZ;
			if(len > 0 && len < IMAGE_TAIL_ALIGN){
				//最后一片要包含完整的尾部信息
				//否则设备端处理会出错
				n = n - (IMAGE_TAIL_ALIGN - len);
				len = IMAGE_TAIL_ALIGN;
			}	
		}else{
			n = len;
			len = 0;
		}
		
		pkt = sa_stm_new_upgrade_pkt(s, &ub, n, current - 1);
		if(pkt == NULL) {
			goto done;
		}
		pkt->up_total = total;
		pkt->up_current = current;
		ub->total = ntohs(total);
		ub->current = ntohs(current);
		data = (char*)(ub+1);
		if(fread(data, 1, n, fp) != n) {
			goto done;
		}
		MD5Update(&ctx, (unsigned char *)data, (unsigned int)n);
		stlc_list_add_tail(&pkt->link, pktlst);
	}	
	
	MD5Final(digest, &ctx);
	pkt = sa_stm_new_upgrade_pkt(s, &ub, 16, current - 1);
	if(pkt == NULL) {
		goto done;
	}
	pkt->up_total = total;
	pkt->up_current = current;
	ub->total = ntohs(total);
	ub->current = ntohs(current++);
	data = (char*)(ub+1);
	memcpy(data, digest, 16);
	stlc_list_add_tail(&pkt->link, pktlst);
	ret = RS_OK;

done:
	fclose(fp);
	if(ret != RS_OK){
		stlc_list_for_each_entry_safe(pkt_t, pkt, next, pktlst, link){
			stlc_list_del(&pkt->link);
			pkt_free(pkt);			
		}
	}
	return ret;		
}

extern RS ucc_request_add2(ucc_session_t *s, pkt_t *pkt);
RS sa_stm_upgrade_file(user_t *user, char *file)
{
	int len;	
	pkt_t *pkt, *next;
	ucc_session_t *s = user->uc_session;
	struct stlc_list_head pktlst;

	STLC_INIT_LIST_HEAD(&pktlst);
	if (sa_get_file_len(file, &len) != RS_OK) {
		return RS_INVALID_PARAM;
	}
	if (len < (int)sizeof(image_hdr_t)) {
		return RS_INVALID_PARAM;
	}
	log_debug("enter len=%d %s %d \n", len, __FUNCTION__, __LINE__);
	//len -= (int)sizeof(image_hdr_t);
	if (sa_stm_check_upgrade_file(user, file, len, &pktlst) != RS_OK) {
		return RS_INVALID_PARAM;		
	}
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);	
	s->up_total = 0;
	stlc_list_for_each_entry_safe(pkt_t, pkt, next, &pktlst, link){
		stlc_list_del(&pkt->link);
		ucc_request_add(s, pkt);
		s->up_total++;
	}
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	
	return RS_OK;
}


RS sa_dev_stm_upgrade_cli(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	up->data[up->len_name] = 0;
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	//镜像头部校验
	if ((ret = sa_stm_upgrade_file_check(user, up->data)) != 0) {
		log_err(false, "enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
		goto done;
	}

	if (user->is_support_spe_up) {
		uc_spe_upgrade_pre_t pre;
		memset((void *)&pre, 0, sizeof(pre));
		if (sa_stm_spe_get_info(up->data, &pre)) {
			sa_stm_spe_upgrade_file(user, 0, NULL, &pre, 1);
		}
	}
	SAFE_FREE(user->stm_spe_up_file);
	ret = sa_stm_upgrade_file(user, up->data);

done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}

RS sa_stm_spe_upgrade_file(user_t *user, u_int8_t num, u_int8_t *psn, uc_spe_upgrade_pre_t *pre_src, u_int8_t force)
{
	int i;
	int len;
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	uc_spe_upgrade_pre_t *pre;
	ucc_session_t *s = user->uc_session;

	log_debug("enter %s num=%u s->my_request_id=%u\n", 
		__FUNCTION__, num, s->my_request_id);
	len = sizeof(*uc) + sizeof(*uo) + sizeof(*pre) + num*sizeof(u_int64_t);
	pkt = uc_pkt_new(s, CMD_UDP_CTRL, len, true, s->select_enc?true:false, 0, 
		s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return RS_ERROR;
	}	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->action = UCA_SET;
	uc->count = 1;
	uc->reserved = 0;

	uo = (ucp_obj_t *)(uc+1);
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_SOFTWARE, UCAT_SYS_STM_UPGRADE_PREINFO, 
		(u_int16_t)(sizeof(*pre) + num*sizeof(u_int64_t)));
	pre = (uc_spe_upgrade_pre_t *)(uo+1);
	memcpy((void *)pre, (void *)pre_src, sizeof(*pre));
	pre->sn_num = num;
	pre->up_action = force;
	if (num) {
		memcpy((void *)pre->sn, (void *)psn, num*sizeof(u_int64_t));
	}
	for(i = 0; i < num; i++) {
		log_debug("enter %s sn[%d]=%llu\n", __FUNCTION__, i, pre->sn[i]);
		pre->sn[i] = ntoh_ll(pre->sn[i]);
	}

	ucc_request_add(s, pkt);
	log_debug("exit %s\n", __FUNCTION__);
	return RS_OK;
}

//指定升级

static void do_rfgw_spe_up(user_t *user)
{
	RS ret = RS_ERROR;

	if (!user->stm_spe_up_file) {
		return;
	}
	
	cl_lock(&cl_priv->mutex);
	ret = sa_stm_upgrade_file(user, user->stm_spe_up_file);
	cl_unlock(&cl_priv->mutex);

	if (ret != RS_OK) {
		log_err(false, "do_rfgw_spe_up stm upgrade failed\n");
	}
	SAFE_FREE(user->stm_spe_up_file);
}

void do_rfgw_spe_up_callback(user_t *user, u_int16_t error)
{
	int event = 0;
	bool need_up = false;

	log_debug("do_rfgw_spe_up_callback error=%u\n", error);
	switch(error) {
	case STM_SPE_UP_OK:
		break;
	case STM_SPE_UP_NEED_IMG:
		do_rfgw_spe_up(user);
		need_up = true;
		break;
	case STM_SPE_UP_ING:
	case STM_SPE_UP_SN_ERR:
	case STM_SPE_UP_PARAM_INVALID:
		break;
	default:
		break;
	}

	if (!need_up) {
		event_push_err(user->callback, UE_RFGW_NOT_ALLOW_UPGRADE, user->handle, user->callback_handle, error);
        event_cancel_merge(user->handle);	
	}
}

bool sa_stm_spe_get_info(char *file, uc_spe_upgrade_pre_t *pre)
{
	FILE *fp = NULL;
	int len = 0;
	u_int8_t buf[1024];
	rf_tail_t *prt = (rf_tail_t *)buf;
	bool ret = false;

	log_debug("enter %s file=%s\n", __FUNCTION__, file);
	fp = fopen(file, "rb");
	if (!fp) {
		return false;
	}
	len = (int)(sizeof(rf_tail_t) + sizeof(tail_t));
	if (fseek(fp, -len, SEEK_END) < 0) {
		log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
		goto done;
	}
	if (fread(buf, sizeof(*prt), 1, fp) != 1) {
		log_debug("enter err %s %d len=%u sizeof(*prt)=%u\n", 
			__FUNCTION__, __LINE__, len, sizeof(*prt));
		goto done;
	}
	pre->dev_type = prt->dev_type;
	pre->ext_type = prt->ext_type;
	pre->major_ver = prt->major_ver;
	pre->minor_ver = prt->minor_ver;
	pre->ima_type = prt->image_type;
	log_debug("enter %s get dt=%u et=%u mav=%u miv=%u imt=%u\n", 
		__FUNCTION__, pre->dev_type, pre->ext_type, pre->major_ver, 
		pre->minor_ver, pre->ima_type);
	ret = true;

done:
	fclose(fp);

	return ret;
}

RS sa_dev_stm_upgrade_spe_cli(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	u_int8_t num = 0;
	u_int8_t *psn = NULL;
	uc_spe_upgrade_pre_t pre;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	num = up->data[up->len_name];
	psn = &up->data[up->len_name + 1];

	log_debug("enter %s %d file=%s num=%u\n", __FUNCTION__, __LINE__, up->data, num);	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	if (!user->is_support_spe_up) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	//镜像头部校验
	if ((ret = sa_stm_upgrade_file_check(user, up->data)) != 0) {
		log_err(false, "enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
		goto done;
	}

	SAFE_FREE(user->stm_spe_up_file);
	user->stm_spe_up_file = cl_strdup(up->data);

	memset((void *)&pre, 0, sizeof(pre));
	if (sa_stm_spe_get_info(up->data, &pre)) {
		ret = sa_stm_spe_upgrade_file(user, num, psn, &pre, up->b_proc);
	}

done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}


pkt_t *sa_dev_active_pkt(ucc_session_t *s, u_int8_t *pbuff, int len)
{
	pkt_t *pkt;
	ucp_ctrl_t* uc;
	ucp_obj_t* uo;
	u_int8_t *pdata;

	pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(*uc)+sizeof(*uo) + len,
                     true, s->select_enc?true:false, 0, s->client_sid, s->device_sid, s->my_request_id);
	if(pkt == NULL) {
		return NULL;
	}
	
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
	uc->count = 1;
	uc->reserved = 0;
	uc->action = UCA_SET;

	uo = (ucp_obj_t *)(uc+1);
	pdata = (u_int8_t *)&uo[1];
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_LICENSE,  UCAT_SYS_LICENSE_ACTIVE, len);
	memcpy(pdata, pbuff, len);
	
	return pkt;
}

RS sa_dev_active(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user;
	pkt_t *pkt = NULL;
	
	up = (cln_user_t *)&cln_pkt->data[0];
	
	cl_lock(&cl_priv->mutex);
	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "%s handle = 0x%08x failed: not found\n", __FUNCTION__, up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if (!user->is_udp_ctrl) {
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	pkt = sa_dev_active_pkt(user->uc_session, up->data, up->len);
	if (pkt) {
		log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);		
		ucc_request_add(user->uc_session, pkt);
	} 

done:
	cl_unlock(&cl_priv->mutex);
	log_debug("enter %s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	
	return ret;
}



