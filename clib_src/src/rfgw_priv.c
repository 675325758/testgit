/*
	本文件用于实现EB一代相关功能
*/
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_rfgw.h"
#include "rfgw_priv.h"
#include "rfgw_rftt.h"
#include "cmd_misc.h"
#include "cl_user_priv.h"
#include "udp_device_common_priv.h"
#include "uas_client.h"
#include <sys/stat.h>
#include "http.h"
#include "app_pkt_priv.h"
#include "misc_client.h"

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

typedef bool (*timer_type_call_t)(u_int8_t type);

typedef void (*timer_set_call_t)(void *hd, u_int8_t type, u_int8_t days, u_int8_t hours, u_int8_t mins, u_int32_t time_min);



#define COMM_TIMER_DAY_SECONDS	(24*60*60)
#define COMM_TIMER_DAY_MINS		(24*60)
#define COMM_TIMER_WEEK_MINS	(7*24*60)

#define COMM_TIMER_NOWEEK_ONCE	0
#define COMM_TIMER_NOWEEK_DUR	1
#define COMM_TIMER_WEEK_ONCE	2
#define COMM_TIMER_WEEK_DUR		3

#define MAX_BIG_PKT_SEND_TIMEOUT    (5)


extern RS http_get(const char *url, int event, cl_handle_t handle);
extern int sa_stm_upgrade_file_check(user_t *user, char *file);
extern RS sa_stm_upgrade_file(user_t *user, char *file);
static void rfgw_set_tt(user_t *user, u_int8_t *ttd);
static void _rfgw_proc_timer_query(user_t *user, net_dev_timer_t *t);
static void _rfgw_proc_timer_query_next(user_t *user, net_dev_timer_t *t);
extern cl_comm_timer_t *comm_timer_find_by_id(cl_comm_timer_head_t *pcth, u_int8_t id);
static bool rf_slave_save(user_t *user);
extern void slave_info_copy(user_t *user, struct stlc_list_head *slave_list);
cl_slave_data_type _set_rf_dev_d_type(slave_t* slave);
extern bool mem_is_all_zero(u_int8_t *data, int len);
static void rfgw_slave_cache_quick_query(user_t *user,smart_air_ctrl_t* ac);
bool udp_rf_dev_update_cache_date(slave_t* slave, u_int8_t action, u_int8_t* buf, u_int16_t len);
extern void comm_timer_utc_2_local(cl_comm_timer_t *ptimer);
extern void comm_timer_local_2_utc(cl_comm_timer_t *ptimer);
static void _comm_timer_update_timer_rf_proc(user_t *user, cl_comm_timer_t *pctimer, ucp_comm_timer_rf_t *ptimer);
extern void _ucp_comm_update_timer_onoff(ucp_comm_timer_t *ptimer, cl_comm_timer_t *pctimer);
extern u_int8_t timer_add_next_day(cl_comm_timer_t *ptimer);
extern void _rf_com_write_user_info(slave_t *slave, int id, u_int8_t *data, int len);
extern RS _rf_com_load_user_info(slave_t* slave, int id, u_int8_t *buf, int len);

extern void memdump(char* pre,void* dest, u_int32_t len);
static void rfgw_dev_group(user_t *user, cl_dev_group_t *group);
static void comm_timer_slave_save(slave_t *slave);
static void comm_timer_slave_init(slave_t *slave);
static void comm_timer_slave_clean(slave_t *slave);
static void comm_timr_go_query(slave_t *slave, cl_dev_timer_summary_t *pcts);
void bmp_set_status(rf_big_pkt_proc_t *bmpip, u_int8_t status);
static void bmp_do_next(slave_t* slave);
static void bmp_offline_proc(slave_t *slave);
extern void rf_air_ir_start_db_send(slave_t* slave);
extern void rf_air_ir_start_db_timeout(slave_t* slave);
extern void rf_air_ir_send_db_ok(slave_t* slave);
extern void rf_air_ir_recv_timeout(slave_t *slave);
static void do_rfgw_slave_flags_parse(slave_t *slave, rfgw_priv_t *p, u_int8_t flags, bool offline_clean);
static void rfgw_slave_init(slave_t *slave);
extern void do_htllock_tlv_proc(slave_t *slave, uc_tlv_t *ptlv, u_int8_t action);
extern void do_rfgw_index_hllock_add(slave_t *slave);
extern bool sa_stm_spe_get_info(char *file, uc_spe_upgrade_pre_t *pre);
extern RS sa_stm_spe_upgrade_file(user_t *user, u_int8_t num, u_int8_t *psn, uc_spe_upgrade_pre_t *pre_src, u_int8_t force);
extern void _rf_wkair_query_ir_code(slave_t *slave, u_int8_t index);

rfdev_status_t *rfdev_alloc()
{
	rfdev_status_t *p;
	p = cl_calloc(sizeof(*p), 1);
	if(p){
		STLC_INIT_LIST_HEAD(&p->work_list);
	}
	return p;
}

void rfdev_free(rfdev_status_t *p)
{
	rfdev_work_t *pos, *n;
	if(p){
		stlc_list_for_each_entry_safe(rfdev_work_t, pos, n, &p->work_list, link){
			stlc_list_del(&pos->link);
			cl_free(pos);
		}
		cl_free(p);
	}
}

void _cl_rfdev_free(cl_slave_t *slave)
{
    if (!slave) {
        return;
    }
	SAFE_FREE(slave->rfdev.work_list);
    udp_rf_dev_free_cl_data(slave);
}

void _cl_rfgw_free(cl_gw_info_t *info)
{
    int i,j;
    cl_lamp_remote_info_t* lr;
    
	if(!info)
		return;
	SAFE_FREE(info->dev_group);
    
    lr = info->lr_info;
    for (i = 0; i < info->lamp_remote_cnt && lr != NULL; i++,lr++) {
        for (j = 0; j < sizeof(lr->key)/sizeof(cl_lamp_remote_key_info); j++) {
            SAFE_FREE(lr->key[j].slave_handle);
        }
    }
    SAFE_FREE(info->lr_info);

	for(i = 0; i < D_T_MAX; i++) {
		if (info->upgrade_url[i]) {
			SAFE_FREE(info->upgrade_url[i]);
		}
	}
	SAFE_FREE(info->pimg_cache);
}

void *init_rfgw_sdk()
{
	rfgw_priv_t *p;

	p = cl_calloc(sizeof(*p), 1);
	if(p){
		STLC_INIT_LIST_HEAD(&p->new_dev_head);
		STLC_INIT_LIST_HEAD(&p->new_slave_head);
	}
	return (void*)p;
}

void free_rfgw_sdk(void *ptr)
{
	rfgw_priv_t *p = (rfgw_priv_t*)ptr;
	slave_t *slave, *n;	
	sdk_dev_find_t *sdev, *s;
	unsigned int i;

	if(!p)
		return;
	
	stlc_list_for_each_entry_safe(slave_t, slave, n, &p->new_slave_head, link) {
		stlc_list_del(&slave->link);
		slave_free(slave);
	}

	stlc_list_for_each_entry_safe(sdk_dev_find_t, sdev, s, &p->new_dev_head, link){
		stlc_list_del(&sdev->link);
		cl_free(sdev);
	}

	for(i = 0; i < ARRAY_SIZE(p->devgroup); i++){
		SAFE_FREE(p->devgroup[i]);
	}	

	for(i = 0; i < D_T_MAX; i++) {
		if (p->upgrade_url[i]) {
			SAFE_FREE(p->upgrade_url[i]);
		}
	}


	SAFE_FREE(p->plist);
	SAFE_FREE(p->pimg_cache);
}

static bool do_rfgw_join_find(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	net_rfgw_join_find_t *f;
	sdk_dev_find_t *dev;
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;

	f = (net_rfgw_join_find_t*)(obj+1);
	f->sn = ntoh_ll(f->sn);
	dev = cl_calloc(sizeof(*dev), 1);
	dev->cl_dev.sn = f->sn;
	dev->cl_dev.subtype = f->subtype;
	dev->cl_dev.extype = f->extype;

	stlc_list_add_tail(&dev->link, &p->new_dev_head);

	event_push(air_ctrl->sac->user->callback, UE_RFGW_DEV_FIND, air_ctrl->sac->user->handle, air_ctrl->sac->user->callback_handle);
	return false;
}

bool is_valid_name(slave_t *slave)
{
	int i;
	unsigned char *name = (unsigned char*)slave->name;
	
	if(name == NULL)
		return false;
	
	for(i = 0; name[i] != 0; i++){
		if(name[i] != 0xFF)
			break;
	}
	if(i >= 15)
		return false;
	return true;
}

bool is_led_type(slave_t *slave)
{
	switch(slave->ext_type) {
	case RF_EXT_TYPE_LED_LAMP:
	case RF_EXT_TYPE_DWHF:
	case RF_EXT_TYPE_DWYKHF:
	case RF_EXT_TYPE_DWYSTGQ:
		return true;
	}

	if (slave->ext_type >= RF_EXT_TYPE_LAMP_START && slave->ext_type <= RF_EXT_TYPE_LAMP_END) {
		return true;
	}

	return false;
}

void copy_slave_ver(slave_t *slave, rf_stm_ver_t *ver)
{
	u_int32_t vertion = 0;
	ms_dev_version_t *sver = &slave->dev_info.version;

	if (is_led_type(slave)) {
		sver->soft_version.major = ver->app_major;
		sver->soft_version.minor = ver->app_minor;
		sver->soft_version.revise = ver->rf_major;
		sver->soft_version.pad = 0;
		
		sver->upgrade_version = sver->soft_version;
		goto done;
	}
	
	switch(slave->ext_type){
		case RF_EXT_TYPE_LED_LAMP:
		case RF_EXT_TYPE_DWHF:
		case RF_EXT_TYPE_DWYKHF:
		case RF_EXT_TYPE_DWYSTGQ:
			//新唐单片机，升级只有一个映像
			sver->soft_version.major = ver->app_major;
			sver->soft_version.minor = ver->app_minor;
			sver->soft_version.revise = ver->rf_major;
			sver->soft_version.pad = 0;
			
			sver->upgrade_version = sver->soft_version;
			break;
		default:
			//stm32单片机，升级分为基础部分和应用部分
			if (ver->rf_minor&BIT(7)) {
				ver->rf_minor &= 0x7f;
				sver->soft_version.major = (ver->rf_major>>4)&0xf;
				sver->soft_version.minor = ver->rf_major&0xf;
				sver->soft_version.revise = ver->rf_minor;
				sver->soft_version.pad = 0;
			} else {
				sver->soft_version.major = ver->rf_major;
				sver->soft_version.minor = ver->rf_minor;
				sver->soft_version.revise = 0;
				sver->soft_version.pad = 0;
			}

			if (ver->app_minor&BIT(7)) {
				ver->app_minor &= 0x7f;
				sver->upgrade_version.major = (ver->app_major>>4)&0xf;
				sver->upgrade_version.minor = ver->app_major&0xf;
				sver->upgrade_version.revise = ver->app_minor;
				sver->upgrade_version.pad = 0;	
			} else {
				sver->upgrade_version.major = ver->app_major;
				sver->upgrade_version.minor = ver->app_minor;
				sver->upgrade_version.revise = 0;
				sver->upgrade_version.pad = 0;
			}
			break;			
	}

done:	
	vertion = ver->app_major + ver->app_minor + ver->rf_major + ver->rf_minor;
	//log_debug("ver->app_major=%u ver->app_minor=%u ver->rf_major=%u ver->rf_minor=%u\n", 
		//ver->app_major, ver->app_minor, ver->rf_major, ver->rf_minor);
	if ((slave->status == BMS_BIND_ONLINE) && 
		(vertion == 0)) {
		slave->is_upgrade_err = true;
	} else {
		slave->is_upgrade_err = false;
	}
}

void copy_slave_ver2(slave_t *slave, rf_stm_ver2_t *ver)
{
	ms_version_t *sver = &slave->dev_info.rf_stm_ver;

	sver->pad = ver->stm_flag;
	if (sver->pad&BIT(0)) {
		if (ver->stm_minor&BIT(7)) {
			ver->stm_minor &= 0x7f;
			sver->major = (ver->stm_major>>4)&0xf;
			sver->minor = ver->stm_major&0xf;
			sver->revise = ver->stm_minor;
		} else {
			sver->major = ver->stm_major;
			sver->minor = ver->stm_minor;
			sver->revise = 0;
		}
	} else {
		memset((void *)sver, 0, sizeof(*sver));
	}
	
	log_debug("rfver2 sn=%"PRIu64" %u.%u.%u ver->stm_major=%02x ver->stm_minor=%02x\n", 
		slave->sn, sver->major, sver->minor, sver->revise, ver->stm_major, ver->stm_minor);
}

u_int16_t do_rfgw_dev_tlv(slave_t* slave, u_int16_t remain, u_int8_t count, uc_tlv_t *tlv, u_int8_t *has_name, rfgw_priv_t *p)
{
	u_int16_t i=0, ret = 0,j;
    cl_rf_lamp_t* rl;
    u_int32_t * dp,r_id,k_id;
	uc_tlv_t *tlv_bk = NULL;

	*has_name = 0;
	tlv->type = ntohs(tlv->type);
	tlv->len = ntohs(tlv->len);
	while (i < count && remain >= sizeof(uc_tlv_t) && remain >= sizeof(uc_tlv_t) + tlv->len) {				
		switch (tlv->type) {
		case UCT_VENDOR:
			if(tlv->len > 1){
				tlv_val(tlv)[tlv->len - 1] = '\0';
				STR_REPLACE(slave->vendor_id, (const char*)tlv_val(tlv));
			}			
			break;
		case UCT_DEV_NAME:
			if(tlv->len > 1){
				*has_name = 1;
				tlv_val(tlv)[tlv->len - 1] = '\0';
				STR_REPLACE(slave->name, (const char*)tlv_val(tlv));
				if(!is_valid_name(slave)){
					SAFE_FREE(slave->name);
					*has_name = 0;
				}
					
				
			}
			break;
		case UCT_RF_STM_VER:
			if(tlv->len >= sizeof(rf_stm_ver_t)){
				rf_stm_ver_t *ver = (rf_stm_ver_t*)(tlv+1);
				rf_stm_ver2_t *ver2 = (rf_stm_ver2_t *)(ver+1);
				copy_slave_ver(slave, ver);
				if (tlv->len >= (sizeof(rf_stm_ver_t) + sizeof(rf_stm_ver2_t))) {
					copy_slave_ver2(slave, ver2);
				}
				if (p->is_upgrade) {
					slave->is_upgrade_err = false;
				}
			}
			break;
		case UCT_RF_LAMP_REMOTE_ID:
			log_debug("UCT_RF_LAMP_REMOTE_ID sn=%"PRIu64"\n", slave->sn);
			//log_mem_dump("UCT_RF_LAMP_REMOTE_ID", (u_int8_t *)(&tlv[1]), tlv->len);
			if(tlv->len >= sizeof(u_int32_t)*4){
				rl = &slave->dev_info.rf_stat.dev_priv_data.lamp_info;
				rl->remote_count = 0;
				for (dp = (u_int32_t*)(tlv+1),j=0; j < 4; j++,dp++) {
					*dp = ntohl(*dp);
					r_id = ((*dp) & 0xFFFFFFFC) >> 3;
					k_id = (*dp) & 0x7;
					if (r_id != 0 && k_id <= 0x7 ) {
						rl->r_info[rl->remote_count].remote_id = r_id;
						rl->r_info[rl->remote_count].key_id = k_id;
						rl->remote_count++;
					}
				}
				if(tlv->len > sizeof(u_int32_t)*4){
					rl->lamp_type = ((u_int8_t*)(tlv+1))[16];
				}

				if (tlv->len >= 32) {
					if (!mem_is_all_zero(&((u_int8_t *)(tlv+1))[17], 15)) {
						rl->lamp_type = ((u_int8_t*)(tlv+1))[31];	
					}
				}

			}
			break;
		case UCT_SLAVE_SUPPORT:
			if (tlv->len >= 1) {
				slave->dev_info.rf_stat.dev_support = tlv_u8_value(tlv);
			}
			break;
		case UCT_DEVELOPER_ID:
			if (tlv->len >= 32) {
				memcpy(slave->developer_id, tlv_val(tlv), 32);
			}
			break;
		case UCT_RFDEV_NEW_VER:
			slave->is_new_ver = true;
			break;
		default:
			break;
		}
		ret += (sizeof(uc_tlv_t) + tlv->len);
		remain -= (sizeof(uc_tlv_t) + tlv->len);
		i++;

		tlv_bk = tlv;
		
		tlv = tlv_next(tlv);

		tlv_bk->type = ntohs(tlv_bk->type);
		tlv_bk->len = ntohs(tlv_bk->len);
			
		if (i < count && remain >= sizeof(uc_tlv_t)) {
			tlv->type = ntohs(tlv->type);
			tlv->len = ntohs(tlv->len);
		}
	}
	return ret;
}

slave_t * _find_slave_at_list(u_int64_t sn, struct stlc_list_head *head)
{
	slave_t *slave;

	stlc_list_for_each_entry(slave_t, slave, head, link) {
		if(slave->sn == sn)
			return slave;
	}
	return NULL;
}

static void _do_all_slave_quick_query(user_t *user,smart_air_ctrl_t* ac)
{
    u_int8_t buf[2048] = {0};
    u_int16_t pos = 0,n = 0;
    u_int16_t max_pkt_len = 1200;
    u_int8_t cmd_len;
    slave_t *slave;
    ucp_obj_t* uo ;
    net_rfgw_tt_t* nf;
    rf_tlv_t* rt;
    
    if (!user) {
        return;
    }
    
    log_debug("_do_all_slave_quick_query sn [%012"PRIu64"]\n",ac->sac->user->sn);
    //检查在线设备是否更新，没更新的，组织查询报文
    stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
        if (!slave->is_update_state && (slave->status ==  BMS_BIND_ONLINE)) {
            uo = (ucp_obj_t*)&buf[pos];
            nf = (net_rfgw_tt_t*)(uo+1);
            rt = (rf_tlv_t*)(nf+1);
            ////////////////////////////////////
            if (slave->ext_type == RF_EXT_TYPE_LIGHT) {
                nf->data[0] = RFTT_WORK_QUERY;
                cmd_len = 0x1;
            }else{
                // 通用的填充
                cmd_len = udp_rf_dev_mk_raw_stat_query_pkt(slave, nf->data);
                if (cmd_len == 0 ) {
                    //看看是不是tlv形式的
                    cmd_len = udp_rf_dev_mk_stat_query_pkt(slave,rt);
                    if (cmd_len == 0) {
                        continue;
                    }
                }
            }
            
            log_debug("_do_all_slave_quick_query slave sn [%012"PRIu64"] ext_type [%u]\n",slave->sn,slave->ext_type);
            if (ac->is_support_rf_cache) {
            	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT_CACHE, sizeof(*nf)+cmd_len);
			} else {
            	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*nf)+cmd_len);
			}
            nf->sn = ntoh_ll(slave->sn);
            nf->len = ntohs(cmd_len);
            pos += (sizeof(*uo)+sizeof(*nf)+cmd_len);
            n++;
            
            //一个数据包完成
            if (pos >= max_pkt_len) {
                sa_set_obj_value_only(user->uc_session, (u_int8_t)n, buf, pos);
                pos = 0;
                n = 0;
            }
        }
    }
    
    // 不够一个数据包的
    if (n>0 && pos >0) {
        log_debug("query all slave packet!");
        sa_set_obj_value_only(user->uc_session, (u_int8_t)n, buf, pos);
    }
}

static bool rf_slave_save(user_t *user)
{
    char fn[256];
    FILE *fp;
	slave_t *pos, *n;

    if (cl_priv->priv_dir == NULL ||
		!user) {
        return false;
    }

	log_debug("enter rf_slave_save !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    sprintf(fn, "%s/%012"PRIu64"", cl_priv->priv_dir, user->sn);
    MKDIR(fn, 0777);
    
    sprintf(fn, "%s/%012"PRIu64"/rf_slave.data", cl_priv->priv_dir, user->sn);
    if ((fp = fopen(fn, "w")) == NULL) {
        log_err(false, "open file %s failed\n", fn);
        return false;
    }

	stlc_list_for_each_entry_safe(slave_t, pos, n, &user->slave, link) {
		log_debug("slave sn %"PRIu64" stat %u\n", pos->sn, pos->status);
		
		pos->status_valid = true;
		
		// 只缓存绑定过的设备
		if (pos->status != BMS_BIND_ONLINE && pos->status != BMS_BIND_OFFLINE) {
			continue;
		}
		#if 0
		if (BMS_BIND_ONLINE == pos->status) {
			sprintf(fn, "sn=%"PRIu64" status=%u sub=%u ext=%u\n", 
				pos->sn, BMS_LOGINING, pos->sub_type, pos->ext_type);
			fputs(fn, fp);
		}
		#endif
		sprintf(fn, "sn=%"PRIu64" status=%u sub=%u ext=%u id=%u\n", 
				pos->sn, pos->status, pos->sub_type, pos->ext_type, pos->id);
			fputs(fn, fp);
	}	
    
    fclose(fp);
	
	return true;
}

static void rf_slave_sock_sync(slave_t *slave)
{
	if (!slave) {
		return;
	}
	 /* 003摄像头与从设备使用一个句柄 */
	if ((slave->sub_type != IJ_003) &&
		(slave->sub_type != IJ_803)) {
		if (slave->video) {
			CLOSE_SOCK(slave->video->vtry.sock_udp);
		}
	}
}

bool rf_slave_init(user_t *user)
{
	char fn[256];
    FILE *fp;
	slave_t *slave;
	u_int64_t sn;
	u_int32_t id = 0xff;
	u_int32_t status, sub_type, ext_type;
	struct stlc_list_head tmp_head;
	int n = 0;

	STLC_INIT_LIST_HEAD(&tmp_head);
	
    if (cl_priv->priv_dir == NULL ||
		!user) {
        return false;
    }
    
    sprintf(fn, "%s/%012"PRIu64"", cl_priv->priv_dir, user->sn);
    MKDIR(fn, 0777);
    
    sprintf(fn, "%s/%012"PRIu64"/rf_slave.data", cl_priv->priv_dir, user->sn);
    if ((fp = fopen(fn, "r")) == NULL) {
        log_err(false, "open file %s failed\n", fn);
        return false;
    }	

	while(fgets(fn, sizeof(fn), fp)) {
		id = 0xff;
		if (sscanf(fn, "sn=%"PRIu64" status=%u sub=%u ext=%u id=%u", &sn, &status, &sub_type, &ext_type, &id) != 5) {
			id = 0xff;
			if (sscanf(fn, "sn=%"PRIu64" status=%u sub=%u ext=%u", &sn, &status, &sub_type, &ext_type) != 4) {
				continue;
			}
		}
		
		slave = slave_lookup_by_sn(user, sn);
		if (slave) {
			continue;
		}
		slave = slave_alloc();
		if (!slave) {
			break;
		}
		
		slave->need_cache_query = 1;

		slave->sn = sn;
		slave->status = status;
		slave->sub_type = sub_type;
		slave->ext_type = ext_type;
		slave->user = user;
		rf_slave_sock_sync(slave);
		sprintf(slave->str_sn, "%012"PRIu64"", slave->sn);
		slave->name = cl_strdup(slave->str_sn);
		slave->is_udp = true;
		slave->id = (u_int8_t)id;
		slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
		stlc_list_add_tail(&slave->link, &tmp_head);
		log_debug("slave add sn=%"PRIu64" slave->status=%u id=%u!!!!!!!!!!!!!!!!11\n", slave->sn, slave->status, slave->id);
		rfgw_slave_init(slave);
		// TODO:定时器在这里做初始化
		//comm_timer_slave_init(slave);
		n++;
	}

	if (n > 0) {
		slave_replace(user, &tmp_head);
	}

	fclose(fp);

	return true;
}

static bool rf_slave_add(user_t *user, struct stlc_list_head *slave_list)
{
	slave_t *pos, *n, *src;
	bool change = false;
	static bool save_first = false;//总要触发保存一次
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, slave_list, link) {
		src = slave_lookup_by_sn(user, pos->sn);
		if (!src) {
			stlc_list_del(&pos->link);
			stlc_list_add(&pos->link, &user->slave);
			change = true;
			pos->handle = handle_create(HDLT_SLAVE);
			pos->video->handle = handle_create(HDLT_VIDEO);
		} else {
			src->status = pos->status;
			src->dev_info.rf_stat.d_type = pos->dev_info.rf_stat.d_type;
			src->is_update_state = pos->is_update_state;
			src->id = pos->id;
			STR_REPLACE(src->name, pos->name);
		}
	}

	slave_list_free(slave_list);

	if (change || !save_first) {
		save_first = true;
		rf_slave_save(user);
	}

	return false;
}

static void _try_finish_rfgw_list(user_t *user, smart_air_ctrl_t* air_ctrl, u_int8_t total, bool ext)
{
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	slave_t *slave;
	u_int8_t i = 0;

	if (ext) {
		rf_slave_add(user, &p->new_slave_head);
	} else {
		stlc_list_for_each_entry(slave_t, slave, &p->new_slave_head, link) {
			i++;
		}
		
		if(i < total) {
			return;
		}

		//get all rf device, replace current user's slave list and notify up app

	    //log_debug("receive all slave list device! total = %d\n",total);
	    
		if(!slave_replace(user, &p->new_slave_head)){
#ifdef WIN32
#ifdef _DEBUG
			char name[128];
			SAFE_FREE(user->name);
			sprintf(name,"%012"PRIu64"-%hhu", user->sn, i);
			user->name = cl_strdup(name);
#endif
#endif
	        // 更新数据，不需要自己发送UE_MODIFY消息
	//		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
		}

		rf_slave_save(user);
	}
    //触发自动查询状态
    _do_all_slave_quick_query(user,air_ctrl);
}

u_int64_t _get_rf_sn_by_ext(user_t *user, rfgw_priv_t *p)
{
	slave_t *slave;

	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->ext_type == p->cur_query_ext) {
			return slave->sn;
		}
	}

	return 0;
}

cl_slave_data_type _get_rf_dev_index_by_ext(u_int8_t ext_type)
{
    cl_slave_data_type d_type = D_T_UNKNOWN;

	
	if (ext_type >= RF_EXT_TYPE_LAMP_START && ext_type <= RF_EXT_TYPE_LAMP_END) {
		return D_T_LAMP_START + (ext_type - RF_EXT_TYPE_LAMP_START);
	}					
	
    switch (ext_type) {
        case RF_EXT_TYPE_LED_LAMP:
            return D_T_LAMP;
            break;
		case RF_EXT_TYPE_DWHF:
            return D_T_DWHF;
			break;
		case RF_EXT_TYPE_DWYKHF:
			return D_T_DWYKHF;
			break;
		case RF_EXT_TYPE_DOOR_LOCK:
			return D_T_DOOR_LOCK;
            break;
		case RF_EXT_TYPE_DOOR_MAGNET:
			return D_T_DOOR_MAGNET;
            break;
		case RF_EXT_TYPE_DOOR_MAGNETV2:
			return D_T_DOOR_MAGNETV2;
			break;
		case RF_EXT_TYPE_DHX:
			return D_T_DHX_SWITCH;
            break;
		case RF_EXT_TYPE_DHXZH:
			return D_T_DHXZH_SWITCH;
		case RF_EXT_TYPE_DHXCP:
			return D_T_DHXCP_SWITCH;
			
		case RF_EXT_TYPE_YT_DOOR_LOCK:
			return D_T_YT_DOOR_LOCK;
            break;
		case RF_EXT_TYPE_HM_MAGENT:
			return D_T_HM_MAGNET;
            break;
		case RF_EXT_TYPE_HM_BODY_DETECT:
			return D_T_HM_BODY_DETECT;
            break;
		case RF_EXT_TYPE_HM_ENV_DETECT:
			return D_T_HM_TEMP_HM;
            break;
		case RF_EXT_TYPE_KTCZ:
			return D_T_KTCZ;
            break;
		case RF_EXT_TYPE_HEATING_VALVE:
			return D_T_HEATING_VALVE;
            break;
		case RF_EXT_TYPE_GAS:
			return D_T_GAS;
            break;
		case RF_EXT_TYPE_QSJC:
			return D_T_QSJC;
            break;
		case RF_EXT_TYPE_HMCO:
			return D_T_HMCO;
            break;
		case RF_EXT_TYPE_HMYW:
			return D_T_HMYW;
            break;
		case RF_EXT_TYPE_HMQJ:
			return D_T_HMQJ;
			break;
		case RF_EXT_TYPE_HTLLOCK:
			return D_T_HTLLOCK;
            break;
		case RF_EXT_TYPE_DWYK:
			return D_T_DWYK;
			break;
        case RF_EXT_TYPE_WK_AIR:
            return D_T_WK_AIR;
            break;
		case RF_EXT_TYPE_WK_AIR2:
			return D_T_WK_AIR2;
			break;
		case RF_EXT_TYPE_SCENE_CONTROLLER:
			return D_T_SCENE_CONTROOLER;
			break;
		case RS_EXT_TYPE_ZHDJ:
			return D_T_ZHDJ;
			break;
		case RF_EXT_TYPE_DWKJ:
			return D_T_DWKJ;
			break;
		case RF_EXT_TYPE_YLTC:
			return D_T_YLTC;
			break;
		case RF_EXT_TYPE_YLWSD:
			return D_T_YLWSD;
			break;
		case RF_EXT_TYPE_YLSOS:
			return D_T_YLSOS;
			break;
		case RF_EXT_TYPE_YLLOCK:
			return D_T_YLLOCK;
			break;
		case RF_EXT_TYPE_YLLIGHT:
			return D_T_YLLIGHT;
		case RF_EXT_TYPE_DWYSTGQ:
			return D_T_DWYSTGQ;
		case RS_EXT_TYPE_YSD:
			return D_T_YSD;
		case RS_EXT_TYPE_CDQJMB:
			return D_T_CDQJMB;
		case RF_EXT_TYPE_JQ:
			return D_T_JQ;
		case RF_EXT_TYPE_DHXML:
			return D_T_MLDHX;
		case RF_EXT_TYPE_LIGHT_SENSE:
			return D_T_LIGHT_SENSE;
		case RF_EXT_TYPE_LHX:
			return D_T_LHX;
		case RF_EXT_TYPE_WUANS6:
			return D_T_WUANS6;
        default:
			log_err(false, "error exttype=%u\n", ext_type);
            break;
    }
    return d_type;
}

cl_slave_data_type _set_rf_dev_d_type(slave_t* slave)
{
    return _get_rf_dev_index_by_ext(slave->ext_type);
}

static u_int8_t _get_rf_dev_ext(cl_slave_data_type type)
{
	u_int8_t ext = 0;

	if (type >= D_T_LAMP_START && type <= D_T_LAMP_END) {
		return RF_EXT_TYPE_LAMP_START + (type - D_T_LAMP_START);
	}	

	switch(type) {
	case D_T_LAMP:
		return RF_EXT_TYPE_LED_LAMP;
	case D_T_DWHF:
		return RF_EXT_TYPE_DWHF;
	case D_T_DWYKHF:
		return RF_EXT_TYPE_DWYKHF;
	case D_T_DOOR_LOCK:
		return RF_EXT_TYPE_DOOR_LOCK;
	case D_T_DOOR_MAGNET:
		return RF_EXT_TYPE_DOOR_MAGNET;
	case D_T_DOOR_MAGNETV2:
		return RF_EXT_TYPE_DOOR_MAGNETV2;
	case D_T_DHX_SWITCH:
		return RF_EXT_TYPE_DHX;;
	case D_T_DHXZH_SWITCH:
		return RF_EXT_TYPE_DHXZH;
	case D_T_DHXCP_SWITCH:
		return RF_EXT_TYPE_DHXCP;
	case D_T_YT_DOOR_LOCK:
		return RF_EXT_TYPE_YT_DOOR_LOCK;
	case D_T_HM_MAGNET:
		return RF_EXT_TYPE_HM_MAGENT;
	case D_T_HM_BODY_DETECT:
		return RF_EXT_TYPE_HM_BODY_DETECT;
	case D_T_HM_TEMP_HM:
		return RF_EXT_TYPE_HM_ENV_DETECT;
	case D_T_KTCZ:
		return RF_EXT_TYPE_KTCZ;
	case D_T_HEATING_VALVE:
		return RF_EXT_TYPE_HEATING_VALVE;
	case D_T_GAS:
		return RF_EXT_TYPE_GAS;
	case D_T_QSJC:
		return RF_EXT_TYPE_QSJC;
	case D_T_HMCO:
		return RF_EXT_TYPE_HMCO;
	case D_T_HMYW:
		return RF_EXT_TYPE_HMYW;
	case D_T_HMQJ:
		return RF_EXT_TYPE_HMQJ;
	case D_T_HTLLOCK:
		return RF_EXT_TYPE_HTLLOCK;
	case D_T_DWYK:
		return RF_EXT_TYPE_DWYK;
	case D_T_SCENE_CONTROOLER:
		return RF_EXT_TYPE_SCENE_CONTROLLER;
	case D_T_DWKJ:
		return RF_EXT_TYPE_DWKJ;
	case D_T_YLTC:
		return RF_EXT_TYPE_YLTC;
		break;
	case D_T_YLWSD:
		return RF_EXT_TYPE_YLWSD;
		break;
	case D_T_YLSOS:
		return RF_EXT_TYPE_YLSOS;
		break;
	case D_T_YLLOCK:
		return RF_EXT_TYPE_YLLOCK;
	case D_T_ZHDJ:
		return RS_EXT_TYPE_ZHDJ;
	case D_T_YLLIGHT:
		return RF_EXT_TYPE_YLLIGHT;
	case D_T_WK_AIR:
		return RF_EXT_TYPE_WK_AIR;
	case D_T_WK_AIR2:
		return RF_EXT_TYPE_WK_AIR2;
		break;
	case D_T_DWYSTGQ:
		return RF_EXT_TYPE_DWYSTGQ;
	case D_T_YSD:
		return RS_EXT_TYPE_YSD;
	case D_T_CDQJMB:
		return RS_EXT_TYPE_CDQJMB;
	case D_T_JQ:
		return RF_EXT_TYPE_JQ;
	case D_T_MLDHX:
		return RF_EXT_TYPE_DHXML;
	case D_T_LIGHT_SENSE:
		return RF_EXT_TYPE_LIGHT_SENSE;
	case D_T_LHX:
		return RF_EXT_TYPE_LHX;
	case D_T_WUANS6:
		return RF_EXT_TYPE_WUANS6;
	default:
		log_err(false, "_get_rf_dev_ext errtype=%u\n", type);
		break;
	}

	return ext;
}

static RS http_rfdev_upgrade_q(user_t *user, u_int8_t ext_type, u_int64_t sn, u_int8_t index, rfgw_priv_t *p)
{
	char url[1024];

	if (p->min_ver_rf_valid[index]) {
		sprintf(url, "http://%s/cgi-bin/updev?sn=%012"PRIu64"&type=%hhu&subtype=%hhu&exttype=%hhu&vendor=%s&lang=%d&upstm=1&uprfstm=1",
			DEFAULT_DOMAIN, sn, TP_DS007, user->sub_type, ext_type, user->vendor_id, 1);
	} else {
		//加上dev sn，给服务器判断是否是工厂升级用的sn，服务器过滤用
		sprintf(url, "http://%s/cgi-bin/updev?sn=%012"PRIu64"&type=%hhu&subtype=%hhu&exttype=%hhu&vendor=%s&lang=%d&upstm=1&mastersn=%"PRIu64"",
			DEFAULT_DOMAIN, sn, TP_DS007, user->sub_type, ext_type, user->vendor_id, 1, user->sn);
	}

	log_debug("http_rfdev_upgrade_q url=%s\n", url);
	http_get_ext(url, CLNE_HTTP_RFDEV_VERSION, user->handle, index);
	return RS_OK;
}

static void dw_upgrade_nofity_proc(user_t *user, smart_air_ctrl_t* air_ctrl)
{
	static bool dw_upgrade_notify = false;
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;

	if (p->upgrade_status[D_T_DWYK] == UP_STATUS_NONE) {
		dw_upgrade_notify = false;
	}
	if ((!dw_upgrade_notify) && 
		(p->upgrade_status[D_T_DWYK] == UP_STATUS_NEED_UPGRADE)) {
		dw_upgrade_notify = true;
        event_push(user->callback, UE_DWYK_NEED_UPGRADE, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		log_debug("_slave_upgrade_check !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}
}

static void slave_upgrade_parse(user_t *user, misc_upgrade_query_reply_t *pr, u_int16_t len)
{
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl = NULL;
	u_int32_t vnew, vold;
	rfgw_priv_t *p;
	bool upgrade_ready = false;
	int index;
	static cl_version_t newest_version;
	slave_t *slave;
	char *tmp;
	char *url;
	u_int16_t dst_len = 0;

	pr->sn = ntoh_ll(pr->sn);
	pr->url_ver_len = ntohl(pr->url_ver_len);
	pr->url_stm_ver_len = ntohl(pr->url_stm_ver_len);
	pr->url_rfstm_ver_len = ntohl(pr->url_rfstm_ver_len);

	dst_len = (u_int16_t)(pr->url_ver_len + pr->url_stm_ver_len + pr->url_rfstm_ver_len + sizeof(*pr));
	tmp = (char *)pr;
	log_err(false, "len=%u url_ver_len=%u url_stm_ver_len=%u url_rfstm_ver_len=%u dst_len=%u\n", 
		len, pr->url_ver_len, pr->url_stm_ver_len, pr->url_rfstm_ver_len, dst_len);
	//len check
	if (len < dst_len) {
		return;
	}

	slave = slave_lookup_by_sn(user, pr->sn);
	if (!slave) {
		log_err(false, "err slave sn=%llu\n", pr->sn);
		return;
	}
	index = _get_rf_dev_index_by_ext(slave->ext_type);
	if (index == D_T_UNKNOWN) {
		log_err(false, "err ext_type=%u index=%u\n", slave->ext_type, index);
		return;
	}
	index %= D_T_MAX;
	if ((sac = user->smart_appliance_ctrl) == NULL || !sac->sub_ctrl) {
		return;
    }
	air_ctrl = sac->sub_ctrl;

	p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	p->upgrade_query[index] = 0;
	p->server_queryed[index] = 1;
	
	memset((void *)&newest_version, 0, sizeof(newest_version));
	//判断是否app通讯镜像需要升级
	SAFE_FREE(p->upgrade_url[index]);
	if (pr->url_stm_ver_len) {
		url = cl_calloc((pr->url_stm_ver_len + 10), 1);
		if (!url) {
			log_err(true, "calloc failed\n");
			return;
		}
		memcpy(url, &tmp[sizeof(*pr) + pr->url_ver_len], pr->url_stm_ver_len);
		p->upgrade_url[index] = url;
		vnew = BUILD_U32(pr->stm_ver.major, pr->stm_ver.minor, pr->stm_ver.revise, 0);
		p->server_ver[index] = vnew;
		vold = p->min_ver[index];
		log_debug("ver index=%u vold=%08x vnew-=%08x upgradestatus=%u\n", 
		index, vold, vnew, p->upgrade_status[index]);
		if((vnew > vold)){
			p->upgrade_query[index] = UP_QUERYED;
			p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
			upgrade_ready = true;
			log_debug("app need upgrade index=%u url=%s\n", index, p->upgrade_url[index]);
			goto done;
		} else {
			p->upgrade_status[index] = 0;
		}
	}

	//判断是否rf需要升级
	memset((void *)&newest_version, 0, sizeof(newest_version));
	SAFE_FREE(p->upgrade_url[index]);
	if (pr->url_ver_len) {
		url = cl_calloc((pr->url_ver_len + 10), 1);
		if (!url) {
			log_err(true, "calloc failed\n");
			return;
		}
		memcpy(url, &tmp[sizeof(*pr)], pr->url_ver_len);
		p->upgrade_url[index] = url;
		vnew = BUILD_U32(pr->ver.major, pr->ver.minor, pr->ver.revise, 0);
		p->server_ver_rf[index] = vnew;
		vold = p->min_ver_rf[index];
		log_debug("rf index=%u vold=%08x vnew-=%08x upgradestatus=%u\n", 
			index, vold, vnew, p->upgrade_status[index]);
		if((vnew > vold)){
			p->upgrade_query[index] = UP_QUERYED;
			p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
			upgrade_ready = true;
			log_debug("rf need upgrade index=%u url=%s\n", index, p->upgrade_url[index]);
			goto done;
		} else {
			p->upgrade_status[index] = 0;
		}
	}

	//判断是否rf下单片机需要升级
	memset((void *)&newest_version, 0, sizeof(newest_version));
	SAFE_FREE(p->upgrade_url[index]);
	if (pr->url_rfstm_ver_len) {
		url = cl_calloc((pr->url_rfstm_ver_len + 10), 1);
		if (!url) {
			log_err(true, "calloc failed\n");
			return;
		}
		memcpy(url, &tmp[sizeof(*pr) + pr->url_ver_len + pr->url_stm_ver_len], pr->url_rfstm_ver_len);
		p->upgrade_url[index] = url;
		vnew = BUILD_U32(pr->rfstm_ver.major, pr->rfstm_ver.minor, pr->rfstm_ver.revise, 0);
		p->server_ver_rf_stm[index] = vnew;
		vold = p->min_ver_rf_stm[index];
		log_debug("rf index=%u vold=%08x vnew-=%08x upgradestatus=%u\n", 
			index, vold, vnew, p->upgrade_status[index]);
		if(vnew > vold){
			p->upgrade_query[index] = UP_QUERYED;
			p->upgrade_status[index] = UP_STATUS_NEED_UPGRADE;
			upgrade_ready = true;
			log_debug("rf need upgrade index=%u url=%s\n", index, p->upgrade_url[index]);
		} else {
			p->upgrade_status[index] = 0;
		}
	}
	
done:	
	if (upgrade_ready) {
		event_push(user->callback, UE_DEV_UPGRADE_READY, user->handle, user->callback_handle);
	}	

	//这里处理一下，电微遥控器升级要尽快，要给个事件提示下
	if (air_ctrl) {
		dw_upgrade_nofity_proc(user, air_ctrl);
	}
}

static void slave_upgrade_cb(cl_handle_t user_handle, u_int16_t type, u_int16_t len, u_int8_t *data)
{
	user_t *user;
	misc_upgrade_query_reply_t *pr = (misc_upgrade_query_reply_t *)data;
	
	if (MCT_UPGRADE != type) {
		log_err(false, "err type=%u\n", type);
		return;
	}
	if (len < sizeof(*pr)) {
		log_err(false, "err len=%u\n", len);
		return;
	}
	if (pr->result != 0) {
		log_err(false, "err result=%u\n", pr->result);
		return;
	}

	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, user_handle);
		goto done;
	}

	slave_upgrade_parse(user, pr, len);
	
done:
	cl_unlock(&cl_priv->mutex);
}

static void do_slave_upgrade_build(user_t *user, smart_air_ctrl_t* air_ctrl, u_int64_t slave_sn, int index, int ext_type)
{
	misc_upgrade_query_t query;
	slave_t *slave;
	u_int32_t ip = user_select_ip(user);
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;

	if (ip == 0) {
		log_err(false, "err ip = 0\n");
		return;
	}
	slave = slave_lookup_by_sn(user, slave_sn);
	if (!slave) {
		log_err(false, "err slave sn="PRIu64"\n", slave_sn);
		return;
	}

	memset((void *)&query, 0, sizeof(query));
	query.type = TP_DS007;
	query.sub_type = user->sub_type;
	query.ext_type = ext_type;
	query.lang = 1;

	if (user->vendor_id) {
		strcpy((char *)query.vendor, user->vendor_id);
	}
	memcpy(query.developer_id, slave->developer_id, sizeof(query.developer_id));
	query.mastersn = ntoh_ll(user->sn);
	query.sn = ntoh_ll(slave_sn);
	query.upstm = 1;
	if (p->min_ver_rf_valid[index]) {
		query.uprfstm = 1;
	}
	
	misc_client_do_request(user->handle, MCT_UPGRADE, sizeof(query), (u_int8_t *)&query, ip, slave_upgrade_cb);
}

void _slave_upgrade_check(user_t *user, smart_air_ctrl_t* air_ctrl)
{
	int i;
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	u_int64_t sn;
	
	for(i = 0; i < D_T_MAX; i++) {
		if (p->upgrade_query[i] == UP_QUERY_NEED) {
			//p->cur_query_index = i;
			p->upgrade_query[i] = 0;
			p->cur_query_ext = _get_rf_dev_ext(i);
			sn = _get_rf_sn_by_ext(user, p);
			log_debug("	D_TYPE %u need query upgrade info sn %"PRIu64" ext_type %u\n", i, sn, p->cur_query_ext);
#ifdef MISC_CLIENT_USER			
			do_slave_upgrade_build(user, air_ctrl, sn, i, p->cur_query_ext);
#else
			http_rfdev_upgrade_q(user, p->cur_query_ext, sn, i, p);
#endif
		}
	}

	//这里处理一下，电微遥控器升级要尽快，要给个事件提示下
	dw_upgrade_nofity_proc(user, air_ctrl);
}

void _slave_min_ver_sync(user_t *user, smart_air_ctrl_t* air_ctrl)
{
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	slave_t *slave;
	u_int32_t type;
	ms_dev_version_t *sver;
	ms_version_t *rf_stmver;
	u_int32_t ver = 0;
	u_int32_t gw_ver = 0;
	int i;
	cl_version_t *pver = NULL;
	u_int32_t type_valid[D_T_MAX];//判断个类型是否有效

	//清除一下数据
	memset((void *)type_valid, 0, sizeof(type_valid));
	memset((void *)p->min_ver, 0, sizeof(p->min_ver));
	memset((void *)p->max_ver, 0, sizeof(p->max_ver));
	memset((void *)p->min_ver_rf, 0, sizeof(p->min_ver_rf));
	memset((void *)p->min_ver_rf_stm, 0, sizeof(p->min_ver_rf_stm));
	memset((void *)p->min_ver_rf_valid, 0, sizeof(p->min_ver_rf_valid));

	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->dev_info.rf_stat.d_type == D_T_UNKNOWN) {
			continue;
		}
		//不是绑定在线的从设备就不需要处理
		if (slave->status != BMS_BIND_ONLINE) {
			continue;
		}
		type = slave->dev_info.rf_stat.d_type;
		sver = &slave->dev_info.version;
		rf_stmver = &slave->dev_info.rf_stm_ver;
		//app ver min统计
		ver = BUILD_U32(sver->upgrade_version.major, sver->upgrade_version.minor, sver->upgrade_version.revise, 0);
		//从设备门磁兼容下1.4版本
		if ((type == D_T_DOOR_MAGNET) && (ver < BUILD_U32(1, 4, 0, 0))) {
			continue;
		}
		log_debug("%s app_ver=%u.%u.%u rf_ver=%u.%u.%u type=%u rf_stmver->flag=%02x\n", 
			slave->name, sver->upgrade_version.major, sver->upgrade_version.minor, 
			sver->upgrade_version.revise, sver->soft_version.major, sver->soft_version.minor, 
			sver->soft_version.revise, slave->dev_info.rf_stat.d_type, rf_stmver->revise);
		//第一次要赋值....
		if (type_valid[type] == 0) {
			p->min_ver[type] = ver;
		}
		if (p->min_ver[type] > ver) {
			p->min_ver[type] = ver;
			log_debug("min_ver up=%08x\n", ver);
		}
		//app max ver统计
		if (p->max_ver[type] < ver) {
			p->max_ver[type] = ver;
		}
		//rf ver统计		
		ver = BUILD_U32(sver->soft_version.major, sver->soft_version.minor, sver->soft_version.revise, 0);
		if (type_valid[type] == 0) {
			p->min_ver_rf[type] = ver;
		}
		if (p->min_ver_rf[type] > ver) {
			p->min_ver_rf[type] = ver;
			log_debug("minrf_ver up=%08x\n", ver);
		}
		//rf stm
		//先判断单片机是否有效
		if (rf_stmver->pad&BIT(0)) {
			ver = BUILD_U32(rf_stmver->major, rf_stmver->minor, rf_stmver->revise, 0);
			if (type_valid[type] == 0) {
				p->min_ver_rf_stm[type] = ver;
			}
			if (p->min_ver_rf_stm[type] > ver) {
				p->min_ver_rf_stm[type] = ver;
				log_debug("minrf_stm_ver up=%08x\n", ver);
			}
			p->min_ver_rf_valid[type] = 1;
		}
		type_valid[type] = 1;
	}
	/*
	这里的升级限制改为网关版本小于1.0.29就不升级了
	*/
	pver = &air_ctrl->stat.soft_version;
	ver = BUILD_U32(1, 0, 29, 0);
	gw_ver = BUILD_U32(pver->major, pver->minor, pver->revise, 0);
	if (gw_ver < ver) {
		//log_debug("gw_ver=%08x ver=%08x\n", gw_ver, ver);
		memset((void *)p->upgrade_query, 0, sizeof(p->upgrade_query));
		return;
	}
	//做一些处理纠正，如升级成功后设备第一次上传上来的版本号还是以前的，后面又正常了，这样不应该报需要升级
	for(i = 0; i < D_T_MAX; i++) {
		if (0 == type_valid[i]) {
			p->upgrade_query[i] = 0;
			p->upgrade_status[i] = 0;
			continue;
		}
		log_debug("check d_type %u\n", i);
		if (p->server_queryed[i]) {
			if ((p->min_ver[i] < p->server_ver[i]) ||
				(p->min_ver_rf[i] < p->server_ver_rf[i]) ||
				(p->min_ver_rf_valid[i] && (p->min_ver_rf_stm[i] < p->server_ver_rf_stm[i]))) {
				//判断一下，如果没得到需要升级的信息就从到服务器去查询
				if (p->upgrade_status[i] != UP_STATUS_NEED_UPGRADE) {
					p->upgrade_query[i] = UP_QUERY_NEED;
				}
			} else {
				p->upgrade_query[i] = 0;
				p->upgrade_status[i] = 0;
			}
		} else {
			log_debug("need query\n");
			p->upgrade_query[i] = UP_QUERY_NEED;
		}
	}	
}

static void comm_timer_online_query(user_t *user, slave_t *slave)
{
	net_dev_timer_t t;

	t.sn = slave->sn;
	log_debug("enter %s\n", __FUNCTION__);
	_rfgw_proc_timer_query(user , &t);
	//_rfgw_proc_timer_query_next(user , &t);
}

static void rfgw_slave_init(slave_t *slave)
{
	if (!slave) {
		return;
	}

	switch(slave->ext_type) {
	case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_WK_AIR2:
		slave->dev_info.rf_stat.dev_priv_data.wk_air_info.stat.ir_id = 0xffff;
		break;
	default:
		break;
	}
}

static bool do_rfgw_list(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj, bool ext)
{
	u_int8_t i , has_name;
	u_int16_t remain; u_int16_t tlv_len;
	uc_tlv_t *tlv;
	slave_t *slave;
	net_rfgw_list_t *list;
	net_rfgw_dev_t *dev;
	user_t *user = air_ctrl->sac->user;
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
    slave_t *old_slave;
	u_int8_t id = 0;
	bool is_new = false;

	log_debug("enter %s len=%u ext=%08x action%u\n", __FUNCTION__, obj->param_len, ext, action);
	if(action != UCA_PUSH &&
		action != UCA_GET)
		return false;

	if (air_ctrl->is_support_rf_cache) {
		log_debug("now the gw is new and drop do_rfgw_list packet\n");
		return false;
	}
	
	if(obj->param_len < sizeof(*list)) {
		return false;
	}
	
	list = (net_rfgw_list_t*)(obj+1);
	
	if(list->count == 0 && list->total == 0){
		_try_finish_rfgw_list(user, air_ctrl, 0, ext);
		return true;
	}
	remain = obj->param_len - sizeof(*list);

	//加点调试
	
	dev = (net_rfgw_dev_t*)(list+1);
	for( i = 0; i < list->count && remain >= sizeof(*dev); i++){
		has_name = 0;
		dev->sn = ntoh_ll(dev->sn);
		if (ext) {
			id = (u_int8_t)(dev->sn>>56)&0xff;
			dev->sn &= 0x0000ffffffffffff;
		}
		is_new = false;
		slave = _find_slave_at_list(dev->sn, &p->new_slave_head);
		if(slave == NULL){
			slave = slave_alloc();
			if(slave == NULL)
				return false;
            log_debug("new slave  sn [%012"PRIu64"] ext_type [%u]\n",dev->sn,dev->extype);
			stlc_list_add_tail(&slave->link, &p->new_slave_head);
			is_new = true;
		}
        slave->id = id;
		slave->user = user;
		slave->sn = dev->sn;
		sprintf(slave->str_sn, "%012"PRIu64"", slave->sn);
		slave->sub_type = dev->subtype;
		slave->ext_type = dev->extype;
		slave->is_udp = true;
		slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
		rf_slave_sock_sync(slave);
		if (is_new) {
			rfgw_slave_init(slave);
		}
		//电威灯第一次不去查询所有灯状态，灯网关优化内存，资源够用再查询
		if(slave->sub_type == IJ_RFGW && slave->ext_type == RF_EXT_TYPE_LED_LAMP) {
			slave->is_update_state = true;
		}
		if(slave->sub_type == IJ_RFGW && is_led_type(slave)) {
			slave->is_update_state = true;
		}

		log_debug("dvesn=%"PRIu64" flags=%02x\n", dev->sn, dev->flags);
        do_rfgw_slave_flags_parse(slave, p, dev->flags, false);

		if ((slave->status == BMS_BIND_ONLINE)&&
			(slave->dev_info.rf_stat.d_type != D_T_UNKNOWN) && 
			(slave->dev_info.rf_stat.d_type < D_T_MAX) &&
			(p->upgrade_query[slave->dev_info.rf_stat.d_type] == 0)) {
			p->upgrade_query[slave->dev_info.rf_stat.d_type] = UP_QUERY_NEED;
		}
		
		log_debug("after sn=%"PRIu64" !!!dev->flags=%u!!!slave->ext_type=%u! slave->status=%u!!1\n", 
			dev->sn, dev->flags, slave->ext_type, slave->status);		
		remain -= sizeof(*dev);
		tlv_len = 0;

		old_slave = _find_slave_at_list(dev->sn, &user->slave);
		
		if(dev->tlv_count && remain >= sizeof(*tlv)){
			tlv_len = do_rfgw_dev_tlv(slave, remain, dev->tlv_count, (uc_tlv_t*)dev->tlvdata, &has_name, p);

			// 全局的SLAVE也一起刷新
			if (old_slave) {
				do_rfgw_dev_tlv(old_slave, remain, dev->tlv_count, (uc_tlv_t*)dev->tlvdata, &has_name, p);
			}
			
			remain -= tlv_len;
		}
        
        old_slave = _find_slave_at_list(dev->sn, &user->slave);
		if(has_name == 0){
			if(old_slave && old_slave->name){
				STR_REPLACE(slave->name, old_slave->name);
				log_debug("11old slave sn %012"PRIu64" name : \"%s\"\r\n", dev->sn, old_slave->name);
			}else {
				if(slave->name == NULL){
					slave->name = cl_strdup(slave->str_sn);
				}
			}
		}
#ifdef WIN32
#ifdef _DEBUG
	if(slave->ext_type ==  RF_EXT_TYPE_LED_LAMP){
		ms_dev_version_t *sver = &slave->dev_info.version;
		if(has_name){
			sprintf(slave->str_sn, "%s-%hhu.%hhu.%hhu", slave->name,
			sver->soft_version.major ,
			sver->soft_version.minor ,
			sver->soft_version.revise);
		}else{
			sprintf(slave->str_sn, "%012"PRIu64"-%hhu.%hhu.%hhu", slave->sn,
			sver->soft_version.major ,
			sver->soft_version.minor ,
			sver->soft_version.revise);
		}
		STR_REPLACE(slave->name , slave->str_sn);
	}
	
#endif
#endif
        //fu'gai
        if ((old_slave != NULL) && is_led_type(slave)) {
            memcpy(&old_slave->dev_info.rf_stat.dev_priv_data.lamp_info.r_info, &slave->dev_info.rf_stat.dev_priv_data.lamp_info.r_info,
                   sizeof(old_slave->dev_info.rf_stat.dev_priv_data.lamp_info.r_info));
            old_slave->dev_info.rf_stat.dev_priv_data.lamp_info.remote_count = slave->dev_info.rf_stat.dev_priv_data.lamp_info.remote_count;
        }
        
		dev = (net_rfgw_dev_t*)(((u_int8_t*)(dev+1)) + tlv_len);
	}

	log_info("HELLO [%s:%d] begin try finish rfgw list total %u\n", __FILE__, __LINE__, list->total);
	_try_finish_rfgw_list(user, air_ctrl, list->total, ext);
	log_info("HELLO [%s:%d] end _try_finish_rfgw_list\n", __FILE__, __LINE__);

	//选出最小版本号
	_slave_min_ver_sync(user, air_ctrl);
	// TODO:在这里开启定时器查询所有类型升级版本
	_slave_upgrade_check(user, air_ctrl);
	return true;	
}

static void rfgw_id_sn_sync(u_int8_t id, u_int64_t sn, struct stlc_list_head *head)
{
	slave_t *slave, *n;

	stlc_list_for_each_entry_safe(slave_t, slave, n, head, link) {
		if (id != slave->id) {
			continue;
		}

		if (sn != slave->sn) {
			stlc_list_del(&slave->link);
			slave_free(slave);
		}
	}
}

static void rfgw_any_alram_clean(slave_t *slave)
{
	cl_com_detector_info_t* rd = &(slave->dev_info.rf_stat.dev_priv_data.cd_info);

	switch(slave->ext_type) {
	case RF_EXT_TYPE_GAS:
	case RF_EXT_TYPE_QSJC:
	case RF_EXT_TYPE_HMCO:
	case RF_EXT_TYPE_HMYW:
	case RF_EXT_TYPE_HMQJ:
	case RF_EXT_TYPE_YLSOS:
		rd->stat.is_alarm = 0;
		rd->stat.is_low_battery = 0;
		rd->stat.is_defence = 0;
		rd->stat.is_pause_alarm = 0;
		break;
	}
}

static void do_rfgw_slave_flags_parse(slave_t *slave, rfgw_priv_t *p, u_int8_t flags, bool offline_clean)
{
	if (!slave) {
		return;
	}
	if(flags & RFDEV_FLAG_ONLINE){
		//从设备上线时查询下定时器
		if (slave->status != BMS_BIND_ONLINE) {
			slave->need_cache_query = 1;
			log_debug("do_rfgw_list_with_id sn=%"PRIu64"\n", slave->sn);
		}
	    slave->status = BMS_BIND_ONLINE;
	}else if (flags & RFDEV_FLAG_UPGRADING) {
		slave->status = BMS_BIND_ONLINE;
		if (slave->dev_info.rf_stat.d_type != D_T_UNKNOWN) {
			if (p) {
				p->upgrade_status[slave->dev_info.rf_stat.d_type] = UP_STATUS_UPGRADING;
			}
		}
	}else if(!(flags & RFDEV_FLAG_NEW)){
		if(flags & RFDEV_FLAG_BINDING){
	        slave->status = BMS_BINDING;
		} else {
	    	slave->status = BMS_BIND_OFFLINE;
			if (offline_clean) {
				//app要求，在从设备离线时清楚某些数据
				rfgw_any_alram_clean(slave);
				//从设备离线，还是清除下红外发送数据这些吧
				bmp_offline_proc(slave);
			}
		}
	}else{
	    if(flags & RFDEV_FLAG_BINDING){
	        slave->status = BMS_BINDING;
	    }else{
	        slave->status = BMS_UNBIND;
	    }
	}	
	if (flags&RFDEV_FLAG_DBC) {
		slave->is_support_dbc = true;
	} else {
		slave->is_support_dbc = false;
	}	
	slave->has_recv_flag_pkt = 1;
	if (flags & RFDEV_FLAG_LINKAGE) {
		slave->is_support_la = 1;
	} 

#ifdef ELE_STAT_CHECK
	if (slave->status == BMS_BIND_ONLINE) {
		//上线清零
		slave->ele_stat.offline_time = 0;
		slave->ele_stat.has_onlined = true;
	} else if (slave->status == BMS_BIND_OFFLINE) {
		if (slave->ele_stat.has_onlined) {
			slave->ele_stat.offline_time = get_sec();
		}
	}
#endif
}

static void do_del_all_rf_slave(user_t *user)
{
	slave_t *slave, *n;
	bool del = false;

	if (!user) {
		return;
	}
	
	stlc_list_for_each_entry_safe(slave_t, slave, n, &user->slave, link) {
		stlc_list_del(&slave->link);
		slave_free(slave);
		del = true;
	}

	if (del) {
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	}
}

static void httlock_slave_user_summary_query(slave_t *slave)
{
	u_int8_t buf[128];
	uc_tlv_t *ptlv = (uc_tlv_t *)buf;
	httlock_bid_user_query_v2_t *pq = (httlock_bid_user_query_v2_t *)&ptlv[1];

	if (slave->is_queryed_bd_summary &&
		(slave->user->establish_num == 0)) {
		return;
	}
	
	log_debug("httlock_slave_user_summary_query query\n");
	memset(buf, 0, sizeof(buf));
	ptlv->type = htons(GW_HTL_BD_QUERY);
	ptlv->len = htons(sizeof(*pq));
	pq->index_type = HTLLOCK_USER_QUERY_SUMMARY;
	pq->sn = ntoh_ll(slave->sn);
	
	slave->is_queryed_bd_summary = true;
	slave->user->establish_num = 0;
	
	do_comm_gw_big_data_send(slave, UCA_GET, buf, (int)(sizeof(*ptlv) + sizeof(*pq)));
}

static void rfgw_slave_big_data_query(user_t *user)
{
	slave_t *slave, *slaven;
	
	stlc_list_for_each_entry_safe(slave_t , slave, slaven, &user->slave, link) {
		switch(slave->ext_type) {
		case RF_EXT_TYPE_HTLLOCK:
			httlock_slave_user_summary_query(slave);
			break;
		default:
			break;
		}
	}
}

#if 1
static bool do_rfgw_list_with_id(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	u_int8_t i , has_name;
	u_int16_t remain; u_int16_t tlv_len;
	uc_tlv_t *tlv;
	slave_t *slave;
	net_rfgw_list_t *list;
	net_rfgw_dev_t *dev;
	user_t *user = air_ctrl->sac->user;
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	u_int8_t id = 0;
	bool is_new = false;

	if(action != UCA_PUSH &&
		action != UCA_GET) {
		return false;
	}
	
	if(obj->param_len < sizeof(*list)) {
		return false;
	}
	
	list = (net_rfgw_list_t*)(obj+1);

	log_debug("enter %s list->count=%u!!!!!!list->total=%u!!!!!11\n", 
		__FUNCTION__, list->count, list->total);	
	remain = obj->param_len - sizeof(*list);
	//加点调试
	dev = (net_rfgw_dev_t*)(list+1);
	for( i = 0; i < list->count && remain >= sizeof(*dev); i++){
		has_name = 0;
		dev->sn = ntoh_ll(dev->sn);
		id = (u_int8_t)(dev->sn>>56)&0xff;
		dev->sn &= 0x0000ffffffffffff;
		//这里要处理下，有可能id对应sn有变化，如果变化需要把以前对应的sn删除
		rfgw_id_sn_sync(id, dev->sn, &user->slave);
		slave = _find_slave_at_list(dev->sn, &user->slave);
		log_debug("enter %s id=%u dev->sn=%"PRIu64"\n", __FUNCTION__, id, dev->sn);
		is_new = false;
		if(slave == NULL){
			slave = slave_alloc();
			if(slave == NULL)
				return false;
			slave->handle = handle_create(HDLT_SLAVE);
			slave->video->handle = handle_create(HDLT_VIDEO);
			stlc_list_add_tail(&slave->link, &user->slave);
			//第一次查询下tt1
			slave->need_cache_query = 1;
            log_debug("new slave  sn [%012"PRIu64"] ext_type [%u] slave->need_cache_query=%u\n",
				dev->sn,dev->extype, slave->need_cache_query);
			is_new = true;
		}
        slave->id = id;
		slave->user = user;
		slave->sn = dev->sn;
		sprintf(slave->str_sn, "%012"PRIu64"", slave->sn);
		slave->sub_type = dev->subtype;
		slave->ext_type = dev->extype;
		slave->is_udp = true;
		slave->dev_info.rf_stat.d_type = _set_rf_dev_d_type(slave);
		rf_slave_sock_sync(slave);
		//电威灯第一次不去查询所有灯状态，灯网关优化内存，资源够用再查询
		if(slave->sub_type == IJ_RFGW && is_led_type(slave)) {
			slave->is_update_state = true;
		}
		log_debug("do_rfgw_list_with_id sn=%"PRIu64" flags=%02x slave->statu=%u\n", slave->sn, dev->flags, slave->status);

		if (is_new) {
			rfgw_slave_init(slave);
		}
        do_rfgw_slave_flags_parse(slave, p, dev->flags, false);

		if ((slave->status == BMS_BIND_ONLINE)&&
			(slave->dev_info.rf_stat.d_type != D_T_UNKNOWN) && 
			(slave->dev_info.rf_stat.d_type < D_T_MAX) &&
			(p->upgrade_query[slave->dev_info.rf_stat.d_type] == 0)) {
			p->upgrade_query[slave->dev_info.rf_stat.d_type] = UP_QUERY_NEED;
		}

		//log_debug("xxx sn=%"PRIu64" !!!dev->flags=%u!!!slave->ext_type=%u! slave->status=%u!!1slave->need_cache_query=%u\n", 
			//slave->sn, dev->flags, slave->ext_type, slave->status, slave->need_cache_query);		
		remain -= sizeof(*dev);
		tlv_len = 0;
		
		if(dev->tlv_count && remain >= sizeof(*tlv)){
			tlv_len = do_rfgw_dev_tlv(slave, remain, dev->tlv_count, (uc_tlv_t*)dev->tlvdata, &has_name, p);	
			remain -= tlv_len;
		}
        
		if(has_name == 0){
			SAFE_FREE(slave->name);
			slave->name = cl_strdup(slave->str_sn);
		}
#ifdef WIN32
#ifdef _DEBUG
	if(slave->ext_type ==  RF_EXT_TYPE_LED_LAMP){
		ms_dev_version_t *sver = &slave->dev_info.version;
		if(has_name){
			sprintf(slave->str_sn, "%s-%hhu.%hhu.%hhu", slave->name,
			sver->soft_version.major ,
			sver->soft_version.minor ,
			sver->soft_version.revise);
		}else{
			sprintf(slave->str_sn, "%012"PRIu64"-%hhu.%hhu.%hhu", slave->sn,
			sver->soft_version.major ,
			sver->soft_version.minor ,
			sver->soft_version.revise);
		}
		STR_REPLACE(slave->name , slave->str_sn);
	}
	
#endif
#endif
        
		dev = (net_rfgw_dev_t*)(((u_int8_t*)(dev+1)) + tlv_len);
	}

	if (user->query_num > list->count) {
		user->query_num -= list->count;
	} else {
		user->query_num = 0;
	}
	
	if (user->query_num == 0) {
		rf_slave_save(user);
		//选出最小版本号,查询完后再去判断是否升级
		_slave_min_ver_sync(user, air_ctrl);
		// TODO:在这里开启定时器查询所有类型升级版本
		_slave_upgrade_check(user, air_ctrl);
	}

	if (air_ctrl->is_support_rf_cache) {
		rfgw_slave_cache_quick_query(air_ctrl->sac->user, air_ctrl);
	}
	//大量用户数据查询
	if (user->is_support_batch_user) {
		rfgw_slave_big_data_query(air_ctrl->sac->user);
	}
	//选出最小版本号
	//_slave_min_ver_sync(user, air_ctrl);
	// TODO:在这里开启定时器查询所有类型升级版本
	//_slave_upgrade_check(user, air_ctrl);
	
	return true;	
}
#endif

static bool do_rftt_work(slave_t *slave, u_int8_t *data, u_int16_t len)
{
	u_int32_t delta;
	if(len < 2)
		return false;
	if(slave->dev_info.rfdev->is_ctrl){
		delta = get_msec() - slave->dev_info.rfdev->ctrl_msec;
		slave->dev_info.rfdev->ctrl_ok++;
		slave->dev_info.rfdev->is_ctrl = 0;
		slave->dev_info.rfdev->ctrl_total += delta;
		if(slave->dev_info.rfdev->ctrl_max < delta)
			slave->dev_info.rfdev->ctrl_max = delta;
		if(slave->dev_info.rfdev->ctrl_min == 0 || slave->dev_info.rfdev->ctrl_min > delta)
			slave->dev_info.rfdev->ctrl_min = delta;
	}
	if(len > 6){
		rfdev_work_t *wk;
		wk = cl_calloc(sizeof(*wk), 1);
		if(wk){
			wk->work = data[1];
			wk->time = ntohl(*(u_int32_t*)&data[2]);
			stlc_list_add_tail(&wk->link, &slave->dev_info.rfdev->work_list);
			log_info("rfdev slave %p %012"PRIu64" work_list %p\r\n", slave, slave->sn, &slave->dev_info.rfdev->work_list);
			if((wk->time+(60*3)) >= time(NULL))
				slave->dev_info.rfdev->work = data[1];
			return true;
		}
	}else{
		if(slave->dev_info.rfdev->work != data[1]){
			slave->dev_info.rfdev->work = data[1];
			return true;
		}
	}
	return false;
}

static bool do_rftt_dbg_info(slave_t *slave, u_int8_t *data, u_int16_t len)
{
	rfdev_dbg_t *dbg;
	if(len < (sizeof(rfdev_dbg_t) + 1)){
		return false;
	}
	dbg = (rfdev_dbg_t *)&data[1];
	slave->dev_info.uptime = ntohl(dbg->uptime);
	slave->dev_info.online = ntohl(dbg->linktime);
	slave->dev_info.query_uptime = get_sec( );
	slave->dev_info.query_online = get_sec( );
	slave->dev_info.rfdev->rfrx = ntohl(dbg->rfrx);
	slave->dev_info.rfdev->linkretry = ntohs(dbg->linkretry);
	return true;
}

static bool _do_light_update(slave_t *slave, net_rfgw_tt_t *tt)
{
    bool res = false;
    
    if(tt->len < 1)
        return false;
    
    switch(tt->data[0]){
        case RFTT_WORK_ACK:
            res = do_rftt_work(slave, tt->data, tt->len);
            break;
        case RFTT_DBG_INFO:
            res = do_rftt_dbg_info(slave, tt->data, tt->len);
            break;
        default:
            break;
    }
    return  res;
}

static u_int8_t _comm_timer_type(cl_comm_timer_t *ptimer)
{
	if (!(ptimer->week_cal&BIT(7))) {
		if (ptimer->duration) {
			return COMM_TIMER_NOWEEK_DUR;
		} else {
			return COMM_TIMER_NOWEEK_ONCE;
		}
	} else {
		if (ptimer->duration) {
			return COMM_TIMER_WEEK_DUR;
		} else {
			return COMM_TIMER_WEEK_ONCE;
		}
	}
}

/******************************************新版下次定时器计算方法*********************************************************************************/
static void _comm_timer_dur_ext(bool noweek, cl_comm_timer_t *ptimer, u_int32_t now, struct tm *ptm, 
	cl_comm_timer_head_t *pcthd, timer_set_call_t start_call, timer_set_call_t finish_call)
{
	int i,j,n;
	struct tm cur_tm;
	u_int32_t time_begin = 0;
	u_int32_t time_finish = 0;
	u_int8_t diff_days = 0;
	u_int32_t value;
	u_int32_t time_base = 0;
	u_int32_t time_diff = ptimer->duration*60;

	cur_tm = *ptm;
	cur_tm.tm_hour = ptimer->hour;
	cur_tm.tm_min = ptimer->min;

	time_base = (u_int32_t)mktime(&cur_tm);
	
	for (i = cur_tm.tm_wday, diff_days = 0; diff_days <= 7; i++, diff_days++) {
		i %= 7;
		if (!(ptimer->week_cal&BIT(i))) {
			continue;
		}
		
		time_begin = time_base + diff_days*COMM_TIMER_DAY_SECONDS;
		time_finish = time_base + time_diff + diff_days*COMM_TIMER_DAY_SECONDS;
		
		if (now < time_begin) {
			start_call(pcthd, ptimer->type, diff_days, ptimer->hour, ptimer->min, time_begin);
			
			value = ptimer->hour*60 + ptimer->min + ptimer->duration;
			if (value/60 > 24) {
				diff_days++;
			}
			finish_call(pcthd, ptimer->type, diff_days, (value/60)%24, value%60, time_finish);
			//周期性定时器，可以一找，直到找到
			break;
		} else if (now < time_finish){
			//落在中间
			if (!noweek) {
				//这里挺麻烦的，要生成下次开始时间，当天是没搞了，得从下一天开始
				for(j = i+1, n = 1; n <= 7; n++, j++) {
					j %= 7;
					if (ptimer->week_cal&BIT(j)) {
						break;
					}
				}

				time_begin = time_base + (diff_days + n)*COMM_TIMER_DAY_SECONDS;
				start_call(pcthd, ptimer->type, diff_days + n, ptimer->hour, ptimer->min, time_begin);
			}
			
			value = ptimer->hour*60 + ptimer->min + ptimer->duration;
			if (value/60 > 24) {
				diff_days++;
			}
			finish_call(pcthd, ptimer->type, diff_days, (value/60)%24, value%60, time_finish);
			//周期性定时器，可以一找，直到找到
			break;
		}
		
		if (noweek) {
			//因为是一次性单点定时器，只可能是一周某一天有效，可以一次后退出
			break;
		}
	}	
}

static void _comm_timer_once_ext(bool noweek, cl_comm_timer_t *ptimer, u_int32_t now, struct tm *ptm, 
	cl_comm_timer_head_t *pcthd, timer_set_call_t start_call, timer_set_call_t finish_call)
{
	int i;
	struct tm cur_tm;
	u_int32_t time_begin = 0;
	u_int8_t diff_days = 0;
	u_int32_t time_base = 0;

	cur_tm = *ptm;
	
	cur_tm.tm_hour = ptimer->hour;
	cur_tm.tm_min = ptimer->min;

	time_base = (u_int32_t)mktime(&cur_tm);
	
	for (i = cur_tm.tm_wday, diff_days = 0; diff_days <= 7; i++, diff_days++) {
		i %= 7;
		if (!(ptimer->week_cal&BIT(i))) {
			continue;
		}
		
		time_begin = time_base + diff_days*COMM_TIMER_DAY_SECONDS;

		if (now < time_begin) {
			start_call(pcthd, ptimer->type, diff_days, ptimer->hour, ptimer->min, time_begin);
			//周期性定时器，可以一找，直到找到
			break;			
		}
		if (noweek) {
			//因为是一次性单点定时器，只可能是一周某一天有效，可以一次后退出
			break;
		}
	}	
}

static void comm_timer_noweek_once_ext(cl_comm_timer_t *ptimer, u_int32_t now, struct tm *ptm, 
	cl_comm_timer_head_t *pcthd, timer_set_call_t start_call, timer_set_call_t finish_call)
{
	_comm_timer_once_ext(true, ptimer, now, ptm, pcthd, start_call, finish_call);
}

static void comm_timer_week_once_ext(cl_comm_timer_t *ptimer, u_int32_t now, struct tm *ptm, 
	cl_comm_timer_head_t *pcthd, timer_set_call_t start_call, timer_set_call_t finish_call)
{
	_comm_timer_once_ext(false, ptimer, now, ptm, pcthd, start_call, finish_call);
}

static void comm_timer_noweek_dur_ext(cl_comm_timer_t *ptimer, u_int32_t now, struct tm *ptm, 
	cl_comm_timer_head_t *pcthd, timer_set_call_t start_call, timer_set_call_t finish_call)
{
	_comm_timer_dur_ext(true, ptimer, now, ptm, pcthd, start_call, finish_call);
}

static void comm_timer_week_dur_ext(cl_comm_timer_t *ptimer, u_int32_t now, struct tm *ptm, 
	cl_comm_timer_head_t *pcthd, timer_set_call_t start_call, timer_set_call_t finish_call)
{
	_comm_timer_dur_ext(false, ptimer, now, ptm, pcthd, start_call, finish_call);
}


void comm_timer_cal_init(cl_comm_timer_head_t *pcthd)
{
	pcthd->min_time_start = 0xffffffff;
	pcthd->min_time_finish = 0xffffffff;	
}

void comm_timer_cal_sync(cl_comm_timer_head_t *pcthd)
{
	if (pcthd->min_time_start != 0xffffffff) {
		if (pcthd->min_time > pcthd->min_time_start) {
			pcthd->min_time = pcthd->min_time_start;
		}
		if (pcthd->min_time == 0) {
			pcthd->min_time = pcthd->min_time_start;
		}
	}
	
	if (pcthd->min_time_finish != 0xffffffff) {
		if (pcthd->min_time > pcthd->min_time_finish) {
			pcthd->min_time = pcthd->min_time_finish;
		}
		if (pcthd->min_time == 0) {
			pcthd->min_time = pcthd->min_time_start;
		}
	}	
}

void _comm_timer_next_cal(cl_comm_timer_head_t *pcthd, timer_type_call_t type_select_call, 
	timer_set_call_t start_call, timer_set_call_t finish_call, struct tm *ptm, u_int32_t now)
{
	int i;
	cl_comm_timer_t *ptimer = NULL;

	comm_timer_cal_init(pcthd);
	
	for(i = 0; i < pcthd->timer_count; i++) {
		ptimer = &pcthd->timer[i];
		if ((ptimer->valid == 0) ||
			(ptimer->enable == 0)) {
			continue;
		}
		
		if (!type_select_call(ptimer->type)) {
			continue;
		}
		
		switch(_comm_timer_type(ptimer)) {
		case COMM_TIMER_NOWEEK_ONCE:
			//非周期一次性定时器
			comm_timer_noweek_once_ext(ptimer, now, ptm, pcthd, start_call, finish_call);
			break;
		case COMM_TIMER_NOWEEK_DUR:
			comm_timer_noweek_dur_ext(ptimer, now, ptm, pcthd, start_call, finish_call);
			break;
		case COMM_TIMER_WEEK_ONCE:
			comm_timer_week_once_ext(ptimer, now, ptm, pcthd, start_call, finish_call);
			break;
		case COMM_TIMER_WEEK_DUR:
			comm_timer_week_dur_ext(ptimer, now, ptm, pcthd, start_call, finish_call);
			break;
		default:
			break;
		}
	}

	comm_timer_cal_sync(pcthd);
}

//定时器start时的下次动作执行过滤函数，如本函数就是过滤开关定时器的
void onoff_start_set(cl_comm_timer_head_t *pcthd, u_int8_t type, u_int8_t days, 
	u_int8_t hours, u_int8_t mins, u_int32_t time_min)
{
	bool start = true;
	next_exec_time_t *pnext = &pcthd->next_on;
	
	switch(type) {
	case UT_TYPE_ON:
	case UT_TYPE_PERIOD_ONOFF:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		//不是最小的不需要
		if (time_min >= pcthd->min_time_start) {
			return;
		}
		pcthd->min_time_start = time_min;
		pnext = &pcthd->next_on;
		break;
	case UT_TYPE_OFF:
		//不是最小的不需要
		if (time_min >= pcthd->min_time_finish) {
			return;
		}
		pcthd->min_time_finish = time_min;
		pnext = &pcthd->next_off;
		start = false;
		break;
	default:
		return;
		break;
	}

	pnext->next_valid = 1;
	pnext->next_day = days;
	pnext->next_hour = hours;
	pnext->next_min = mins;	
}

//定时器finish时的下次动作执行过滤函数，如果本函数就是过滤开关定时器的
void onoff_finish_set(cl_comm_timer_head_t *pcthd, u_int8_t type, u_int8_t days, 
	u_int8_t hours, u_int8_t mins, u_int32_t time_min)
{
	next_exec_time_t *pnext = &pcthd->next_off;
	
	switch(type) {
	case UT_TYPE_PERIOD_ONOFF:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
	case UT_TYPE_OFF:
		//不是最小的不需要
		if (time_min >= pcthd->min_time_finish) {
			return;
		}
		pcthd->min_time_finish = time_min;
		pnext = &pcthd->next_off;
		break;
	default:
		return;
		break;
	}

	pnext->next_valid = 1;
	pnext->next_day = days;
	pnext->next_hour = hours;
	pnext->next_min = mins;
}

//类型过滤函数，只计算想要的类型，如，本函数是过滤开关定时器的。
bool timer_type_onoff_select(u_int8_t type)
{
	bool ret = false;

	switch(type) {
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		ret = true;
		break;
	default:
		break;
	}
	
	return ret;
}

void comm_timer_next_cal_onoff(cl_comm_timer_head_t *pcthd, struct tm * ptm, u_int32_t now)
{
	memset((void *)&pcthd->next_on, 0, sizeof(pcthd->next_on));
	memset((void *)&pcthd->next_off, 0, sizeof(pcthd->next_off));

	_comm_timer_next_cal(pcthd, timer_type_onoff_select, (timer_set_call_t)onoff_start_set, (timer_set_call_t)onoff_finish_set, ptm, now);
}

bool timer_type_contmp_select(u_int8_t type)
{
	bool ret = false;

	switch(type) {
	case UT_TYPE_COMM_TYPE_CONST_TMP:
		ret = true;
		break;
	default:
		break;
	}
	
	return ret;
}

void contmp_start_set(cl_comm_timer_head_t *pcthd, u_int8_t type, u_int8_t days, 
	u_int8_t hours, u_int8_t mins, u_int32_t time_min)
{
	next_exec_time_t *pnext = &pcthd->next_temp_start;
	
	switch(type) {
	case UT_TYPE_COMM_TYPE_CONST_TMP:
		//不是最小的不需要
		if (time_min >= pcthd->min_time_start) {
			return;
		}
		pcthd->min_time_start = time_min;
		pnext = &pcthd->next_temp_start;
		break;
	default:
		return;
		break;
	}

	pnext->next_valid = 1;
	pnext->next_day = days;
	pnext->next_hour = hours;
	pnext->next_min = mins;	
}

void contmp_finish_set(cl_comm_timer_head_t *pcthd, u_int8_t type, u_int8_t days, 
	u_int8_t hours, u_int8_t mins, u_int32_t time_min)
{
	next_exec_time_t *pnext = &pcthd->next_temp_finish;
	
	switch(type) {
	case UT_TYPE_COMM_TYPE_CONST_TMP:
		//不是最小的不需要
		if (time_min >= pcthd->min_time_finish) {
			return;
		}
		pcthd->min_time_finish = time_min;
		pnext = &pcthd->next_temp_finish;
		break;
	default:
		return;
		break;
	}

	pnext->next_valid = 1;
	pnext->next_day = days;
	pnext->next_hour = hours;
	pnext->next_min = mins;
}

void comm_timer_next_cal_const_tmp(cl_comm_timer_head_t *pcthd, struct tm *ptm, u_int32_t now)
{
	memset((void *)&pcthd->next_temp_start, 0, sizeof(pcthd->next_temp_start));
	memset((void *)&pcthd->next_temp_start, 0, sizeof(pcthd->next_temp_start));

	_comm_timer_next_cal(pcthd, timer_type_contmp_select, (timer_set_call_t)contmp_start_set, (timer_set_call_t)contmp_finish_set, ptm, now);
}

void comm_timer_next_cal_ext(cl_comm_timer_head_t *pcthd)
{
 	struct tm cur_tm = {0};
	struct tm cur_tm2 = {0};
	time_t now_time;

    now_time = time(NULL);
    localtime_r(&now_time, &cur_tm);

	pcthd->min_time = 0;
	//下次开关机执行定时器计算, 在开关定时器看来min_time_start表示下次开时间，min_time_finish表示下次关时间。。。。。
	memcpy((void *)&cur_tm2, (void *)&cur_tm, sizeof(cur_tm2));
	comm_timer_next_cal_onoff(pcthd, &cur_tm2, (u_int32_t)now_time);

	//下次温度曲线设置定时器计算
	memcpy((void *)&cur_tm2, (void *)&cur_tm, sizeof(cur_tm2));
	comm_timer_next_cal_const_tmp(pcthd, &cur_tm2, (u_int32_t)now_time);

	
	// TODO:等等等等等等等等等等等等等等等等等等等等等等等等等等等
}

void comm_timer_next_cal(cl_comm_timer_head_t *pcthd)
{
	comm_timer_next_cal_ext(pcthd);
}

//定时器类型映射
int timer_type_map(u_int8_t type)
{
	int type_ret = 0;

	switch(type) {
	case UT_TYPE_ON:
	case UT_TYPE_OFF:
	case UT_TYPE_PERIOD_ONOFF:
	case UT_TYPE_ADVANCE_TIMER:
	case UT_TYPE_ON_ADVANCE_TIMER:
		type_ret = UT_TIMER_TYPE_ONOFF;
		break;
	case UT_TYPE_COMM_TYPE_CONST_TMP:
		type_ret = UT_TIMER_TYPE_TMP;
		break;
	default:
		break;
	}
	
	return type_ret;
}

static bool timer_ext_type_map_slave_is_same(cl_comm_timer_head_t *pcthd, cl_comm_timer_t *timer_local, cl_comm_timer_t *ptimer)
{
	bool is_same = true;

	switch(pcthd->ext_type) {
	case RF_EXT_TYPE_DHXML:
	case RF_EXT_TYPE_DHXZH:
	case RF_EXT_TYPE_DHXCP:
		//这里注意，mask bit为1表示 无效，为0才表示有效，需要取反一下。
		//再次注意，发现app上层一直让mask的最高位有效的，不知道为什么，这里先处理一下，反正用不了这么多路
		if (!(((~timer_local->extended_data_u.dhxml_timer.on_off_stat)&0x7fff0000) &
			((~ptimer->extended_data_u.dhxml_timer.on_off_stat)&0x7fff0000))) {
			is_same = false;
		}
		break;
	default:
		break;
	}

	return is_same;
}

static bool timer_ext_type_map_master_is_same(cl_comm_timer_head_t *pcthd, cl_comm_timer_t *timer_local, cl_comm_timer_t *ptimer)
{
	return true;
}

static bool timer_ext_type_is_same(cl_comm_timer_head_t *pcthd, cl_comm_timer_t *timer_local, cl_comm_timer_t *ptimer)
{
	bool is_same = true;

	if (pcthd->is_slave) {
		is_same = timer_ext_type_map_slave_is_same(pcthd, timer_local, ptimer);
	} else {
		is_same = timer_ext_type_map_master_is_same(pcthd, timer_local, ptimer);
	}

	return is_same;
}

u_int8_t mem_get_first_true_value(u_int8_t *pdata, int len)
{
	int i;

	for(i = 0; i < len; i++) {
		if (pdata[i]) {
			return pdata[i];
		}
	}

	return 0;
}

typedef struct {
	u_int16_t begin_time;
	u_int16_t end_time;
}tc_day_map_t;

typedef struct {
	tc_day_map_t day[7];
}tc_week_map_t;

static void _timer_map_init(tc_week_map_t *map, cl_comm_timer_t *timer)
{
	int i;

	for(i = 0; i < 7; i++) {
		if ((timer->week_cal & BIT(i)) == 0) {
			continue;
		}
		map->day[i].begin_time = i*ONE_DAY_MINS + timer->hour*ONE_HOUR_MINS + timer->min;
		map->day[i].end_time = map->day[i].begin_time + timer->duration;
	}
}

bool _timer_is_conflict(cl_comm_timer_t *src, cl_comm_timer_t *dst)
{
	int i, j;
	tc_week_map_t src_map;
	tc_week_map_t dst_map;

	memset((void *)&src_map, 0xff, sizeof(src_map));
	memset((void *)&dst_map, 0xff, sizeof(dst_map));
	_timer_map_init(&src_map, src);
	_timer_map_init(&dst_map, dst);

	//check
	for(i = 0; i < 7; i++) {
		if (src_map.day[i].begin_time == 0xffff) {
			continue;
		}
		for(j = 0; j < 7; j++) {
			if ((src_map.day[i].begin_time >= dst_map.day[j].begin_time) && 
				(src_map.day[i].begin_time <= dst_map.day[j].end_time)) {
				log_debug("week i=%u j=%u src->week_cal=%2x dst->week_cal=%2x conflict src_begintime=%u dst_begintime=%u dst_endtime=%u\n", i, j,
					src->week_cal, dst->week_cal,
					src_map.day[i].begin_time, dst_map.day[j].begin_time, 
					dst_map.day[j].end_time);
				return true;
			}
			if ((src_map.day[i].end_time >= dst_map.day[j].begin_time) && 
				(src_map.day[i].end_time <= dst_map.day[j].end_time)) {
				log_debug("week i=%u j=%u src->week_cal=%2x dst->week_cal=%2x conflict src_begintime=%u dst_begintime=%u dst_endtime=%u\n", i, j,
					src->week_cal, dst->week_cal,
					src_map.day[i].end_time, dst_map.day[j].begin_time, 
					dst_map.day[j].end_time);
				return true;
			}
		}
	}

	return false;
}
#if 1
//计算定时器时间冲突的函数
bool comm_timer_is_conflict(user_t *user, cl_comm_timer_head_t *pcthd, cl_comm_timer_t *timer)
{
	int i;
	int type;
	cl_comm_timer_t *ptimer = NULL;
	cl_comm_timer_t timer_local;
	cl_comm_timer_t *ptimer_local = &timer_local;

	//合法检查一下纠正一下
	if (timer->hour > 23) {
		timer->hour = 23;
	}
	if (timer->min > 59) {
		timer->min = 59;
	}
	//没有定时器直接返回false
	if (pcthd->real_count == 0) {
		return false;
	}
	//只有自己一个定时器时修改自己，直接返回false
	if ((pcthd->real_count == 1) &&
		(timer->id != 0)) {
		return false;
	}
	log_debug("pcthd->timer_count=%u\n", pcthd->real_count);
	memcpy((void *)ptimer_local, (void *)timer, sizeof(*ptimer_local));
	//这里专门处理下，rf定时器需要
	if (ptimer_local->week) {
		ptimer_local->week_cal = ptimer_local->week|BIT(7);
	} else {
	//如果week==0，赋值
		ptimer_local->week_cal = timer_add_next_day(ptimer_local);
	}
	type = timer_type_map(ptimer_local->type);
	for(i = 0; i < pcthd->timer_count; i++) {
		ptimer = &pcthd->timer[i];
		if (!ptimer->valid) {
			continue;
		}
		//如修改或者使能某个过时的定时器
		if (ptimer->id == ptimer_local->id) {
			continue;
		}
		//有些设备要单独比较下，比如单火线扩展数据路数不一样时间是可以冲突 的
		if (!timer_ext_type_is_same(pcthd, ptimer_local, ptimer)) {
			continue;
		}
		//不是一类的定时器可以不管时间冲突
		if (timer_type_map(ptimer->type) != type) {
			continue;
		}
		//未使能的定时器也加入计算，毕竟没有删除
		if (_timer_is_conflict(ptimer_local, ptimer)) {
			event_push_err(user->callback, COMMON_UE_DEV_COMM_TIMER_TIME_CONFLICT, user->handle, 
				user->callback_handle, 0);
        		event_cancel_merge(user->handle);
			return true;
		}
	}	
	
	
	return false;
}
#else
//计算定时器时间冲突的函数
bool comm_timer_is_conflict(user_t *user, cl_comm_timer_head_t *pcthd, cl_comm_timer_t *timer)
{
	int i,j;
	bool conflict = false;
	int type;
	u_int8_t value;
	u_int8_t *map = NULL;
	cl_comm_timer_t *ptimer = NULL;
	cl_comm_timer_t timer_local;
	cl_comm_timer_t *ptimer_local = &timer_local;
	u_int32_t time_begin;
	u_int32_t time_start;
	u_int32_t time_end;
	u_int32_t time_diff;
	u_int8_t ret_id;

	//合法检查一下纠正一下
	if (timer->hour > 23) {
		timer->hour = 23;
	}

	if (timer->min > 59) {
		timer->min = 59;
	}

	//没有定时器直接返回false
	if (pcthd->real_count == 0) {
		return false;
	}

	//只有自己一个定时器时修改自己，直接返回false
	if ((pcthd->real_count == 1) &&
		(timer->id != 0)) {
		return false;
	}

	log_debug("pcthd->timer_count=%u\n", pcthd->real_count);
	map = cl_calloc(COMM_TIMER_WEEK_MINS, 1);
	if (!map) {
		return true;
	}

	memcpy((void *)ptimer_local, (void *)timer, sizeof(*ptimer_local));
	//这里专门处理下，rf定时器需要
	if (ptimer_local->week) {
		ptimer_local->week_cal = ptimer_local->week|BIT(7);
	} else {
	//如果week==0，赋值
		ptimer_local->week_cal = timer_add_next_day(ptimer_local);
	}

	type = timer_type_map(ptimer_local->type);
	for(i = 0; i < pcthd->timer_count; i++) {
		ptimer = &pcthd->timer[i];

		if (!ptimer->valid) {
			continue;
		}
		//如修改或者使能某个过时的定时器
		if (ptimer->id == ptimer_local->id) {
			continue;
		}

		//有些设备要单独比较下，比如单火线扩展数据路数不一样时间是可以冲突 的
		if (!timer_ext_type_is_same(pcthd, ptimer_local, ptimer)) {
			continue;
		}

		//不是一类的定时器可以不管时间冲突
		if (timer_type_map(ptimer->type) != type) {
			continue;
		}

		time_begin = ptimer->hour*60 + ptimer->min;
		value = ptimer->id;

		//未使能的定时器也加入计算，毕竟没有删除
		for(j = 0; j < 7; j++) {
			if (!(ptimer->week_cal&BIT(j))) {
				continue;
			}
			
			switch(_comm_timer_type(ptimer)) {
			case COMM_TIMER_NOWEEK_ONCE:
			case COMM_TIMER_WEEK_ONCE:
				time_start = time_begin + j*COMM_TIMER_DAY_MINS;
				map[time_start] = value;
				break;
			case COMM_TIMER_NOWEEK_DUR:
			case COMM_TIMER_WEEK_DUR:
				time_start = time_begin + j*COMM_TIMER_DAY_MINS;
				time_end = time_start + ptimer->duration;
				if (time_end < COMM_TIMER_WEEK_MINS) {
					memset((void *)&map[time_start], value, ptimer->duration);
				} else {
					time_diff = time_end - COMM_TIMER_WEEK_MINS;
					if (time_diff > COMM_TIMER_WEEK_MINS) {
						time_diff = COMM_TIMER_WEEK_MINS;
					}
					memset((void *)&map[time_start], value, COMM_TIMER_WEEK_MINS - time_start);
					memset((void *)&map[0], value, time_diff);
				}
				break;
			default:
				break;
			}
		}
	}	
	
	//判断是不是冲突的
	time_begin = ptimer_local->hour*60 + ptimer_local->min;
	for(j = 0; j < 7; j++) {
		if (!(ptimer_local->week_cal&BIT(j))) {
			continue;
		}
		switch(_comm_timer_type(ptimer_local)) {
		case COMM_TIMER_NOWEEK_ONCE:
		case COMM_TIMER_WEEK_ONCE:
			time_start = time_begin + j*COMM_TIMER_DAY_MINS;
			if (map[time_start]) {
				conflict = true;
				// TODO:通知
				event_push_err(user->callback, COMMON_UE_DEV_COMM_TIMER_TIME_CONFLICT, user->handle, 
				user->callback_handle, (int)map[time_start]);
        		event_cancel_merge(user->handle);
				goto done;
			}
			break;
		case COMM_TIMER_NOWEEK_DUR:
		case COMM_TIMER_WEEK_DUR:
			time_start = time_begin + j*COMM_TIMER_DAY_MINS;
			time_end = time_start + ptimer_local->duration;
			if (time_end < COMM_TIMER_WEEK_MINS) {
				ret_id = mem_get_first_true_value(&map[time_start], ptimer_local->duration);
			} else {
				time_diff = time_end - COMM_TIMER_WEEK_MINS;
				if (time_diff > COMM_TIMER_WEEK_MINS) {
					time_diff = COMM_TIMER_WEEK_MINS;
				}
				ret_id = mem_get_first_true_value(&map[time_start], COMM_TIMER_WEEK_MINS - time_start);
				if (ret_id == 0) {
					ret_id = mem_get_first_true_value((void *)&map[0], time_diff);
				}
			}

			if (ret_id != 0) {
				conflict = true;
				// TODO:通知
				event_push_err(user->callback, COMMON_UE_DEV_COMM_TIMER_TIME_CONFLICT, user->handle, 
				user->callback_handle, (int)ret_id);
        		event_cancel_merge(user->handle);
				goto done;
			}
			break;
		default:
			break;
		}
	}

done:	
	if (map) {
		cl_free(map);
	}	
	
	return conflict;
}
#endif

int do_comm_query(cl_thread_t *t)
{
	slave_t *slave = CL_THREAD_ARG(t);
	cl_dev_timer_summary_t *pcts = &slave->timer_summary;

	slave->t_rf_com_timer_query = NULL;

	log_debug("enter do_comm_query sn=%"PRIu64"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", slave->sn);
	comm_timr_go_query(slave, pcts);

	return 0;
}

static void comm_query_timer_reset(slave_t *slave)
{
	CL_THREAD_OFF(slave->t_rf_com_timer_query);
	CL_THREAD_TIMER_ON(MASTER, slave->t_rf_com_timer_query, do_comm_query, (void *)slave, TIME_N_SECOND(3));
}

static void comm_query_timer_cancel(slave_t *slave)
{
	CL_THREAD_OFF(slave->t_rf_com_timer_query);
}

static void comm_timr_go_query(slave_t *slave, cl_dev_timer_summary_t *pcts)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);
	u_int8_t *pid = (u_int8_t *)(&prtlv[1]);
	int i;

	if (!slave || !pcts) {
		log_debug("there is no sn=%"PRIu64"\n", pcts->sn);
		return;
	}

	log_debug("comm_timr_go_query map=%02x querymap=%02x status=%u\n", 
		pcts->map, pcts->need_query, slave->status);
	
	prtlv->type = RF_TT_CMD_GET_TIMER;
	prtlv->len = 1;
	
	prt->sn = slave->sn;
	prt->len = sizeof(rf_tlv_t) + 1;
	
	//查询
	for(i = 0; i < pcts->max_timer_count; i++) {
		if (pcts->need_query&BIT(i)) {
			*pid = i+1;
			log_debug("comm_timr_go_query query id=%u\n", *pid);
			rfgw_set_tt(slave->user, (u_int8_t *)buf);
			break;
		}
	}	

	//查询下次定时器时间
	if (pcts->need_query == 0) {
		comm_timer_next_cal(&slave->comm_timer);
		comm_query_timer_cancel(slave);
	} else {
		comm_query_timer_reset(slave);
	}
}

void comm_timer_count_cal(cl_comm_timer_head_t *pcth)
{
	int i;

	pcth->real_count = 0;
	
	if (pcth->timer_count == 0 ||
		!pcth->timer) {
		pcth->timer_count = 0;
	}

	for(i = 0; i < pcth->timer_count; i++) {
		if (pcth->timer[i].valid) {
			pcth->real_count++;
		}
	}
}

void comm_timer_summary_proc(slave_t *slave, u_int8_t* data)
{
	int i, len;
	user_t *user = slave->user;
	net_dev_timer_summary_ext_t *pnts = (net_dev_timer_summary_ext_t *)data;
	cl_comm_timer_head_t *pcthl = &slave->comm_timer;
	cl_dev_timer_summary_t *pcts = &slave->timer_summary;
	u_int8_t *psta_count = (u_int8_t *)&pnts[1];
	cl_comm_timer_t *pctimer = NULL;

	//更新摘要
	//log_debug("enter %s\n", __FUNCTION__);
	pcts->max_timer_count = pnts->max_timer_count;
	//先处理一下，都先只支持32个定时器算了，用不着那么多
	if (pcts->max_timer_count > 32) {
		pcts->max_timer_count = 32;
	}
	pcts->support_type = htons(pnts->support_type);
	pcts->max_data_len = pnts->max_data_len;
	pcts->map = htonl(pnts->map);

	len = pcts->max_timer_count*sizeof(u_int16_t);
	if (pcts->stat_count == NULL) {
		pcts->stat_count = cl_calloc(len, 1);
		if(pcts->stat_count) {
			memset((void *)pcts->stat_count, 0xff, len);
		}
	}

	if (!pcts->stat_count) {
		log_debug("calloc failed !!!!!!!!!!!!!!!!!\n");
		return;
	}

	//pcts->need_query = 0;
	//判断是否需要查询
	for(i = 0; (i < 32) && (i < pcts->max_timer_count); i++) {
		if (pcts->map&BIT(i)) {
			if ((pcts->stat_count[i] == 0xffff) ||
				(pcts->stat_count[i] != (u_int16_t)psta_count[i])) {
				pcts->need_query |= pcts->map&BIT(i);
				pcts->stat_count[i] = (u_int16_t)psta_count[i];
			}
		} else {
			//del
			pctimer = comm_timer_find_by_id(pcthl, i+1);
			if (pctimer) {
				pctimer->valid = 0;
				comm_timer_next_cal(&slave->comm_timer);
				pcts->stat_count[i] = 0xffff;
				// TODO:保存下从设备定时器数据   删除后需要保存
				//comm_timer_slave_save(slave);
			}
			pcts->need_query &= ~(BIT(i));
		}
	}

	pcts->sn = slave->sn;
	//查询
	if (pcts->need_query) {
		comm_timr_go_query(slave, pcts);
	}

	//计算下实时定时器个数
	comm_timer_count_cal(pcthl);
}

static void comm_timer_slave_save(slave_t *slave)
{
	int i,j;
	u_int8_t buf[1024*2];
	cl_comm_timer_head_t *pcth = NULL;
	cl_dev_timer_summary_t *pcts = NULL;
	rf_comm_timer_save_t *psave = (rf_comm_timer_save_t *)buf;

	if (!slave) {
		return;
	}
	pcts = &slave->timer_summary;
	pcth = &slave->comm_timer;
	
	if (!pcts->stat_count || 
		(pcts->max_timer_count == 0)) {
		return;
	}

	if (sizeof(*psave) > sizeof(buf)) {
		return;
	}

	psave->max_timer_count = pcts->max_timer_count;
	psave->max_type_count = pcts->max_type_count;
	psave->max_data_len = pcts->max_data_len;
	psave->support_type = pcts->support_type;
	psave->map = pcts->map;

	for(i = 0; i < 32 && i < pcts->max_timer_count; i++) {
		psave->stat_count[i] = pcts->stat_count[i];
	}

	psave->timer_count = pcth->timer_count;
	psave->real_count = pcth->real_count;

	for(i = 0, j = 0; i < pcth->timer_count; i++) {
		if (pcth->timer[i].valid) {
			memcpy((void *)&psave->timer[j], (void *)&pcth->timer[i], sizeof(psave->timer[j]));
			j++;
		}
	}

	//save
	_rf_com_write_user_info(slave, COMM_TIMER_CONFI_ID, (u_int8_t *)psave, sizeof(*psave));
}

static void comm_timer_slave_init(slave_t *slave)
{
	u_int8_t buf[1024*2];
	cl_comm_timer_head_t *pcth = NULL;
	cl_dev_timer_summary_t *pcts = NULL;
	rf_comm_timer_save_t *psave = (rf_comm_timer_save_t *)buf;

	if (!slave) {
		return;
	}
	pcts = &slave->timer_summary;
	pcth = &slave->comm_timer;

	if (RS_OK != _rf_com_load_user_info(slave, COMM_TIMER_CONFI_ID, (u_int8_t *)psave, sizeof(*psave))) {
		log_debug("comm_timer_slave_init sn=%"PRIu64" failed\n", slave->sn);
		return;
	}

	pcts->max_timer_count = psave->max_timer_count;
	pcts->max_type_count = psave->max_type_count;
	pcts->max_data_len = psave->max_data_len;
	pcts->support_type = psave->support_type;
	pcts->map = psave->map;

	SAFE_FREE(pcts->stat_count);
	pcts->stat_count = cl_calloc(sizeof(psave->stat_count), 1);
	if (!pcts->stat_count) {
		pcts->max_timer_count = 0;
		return;
	}

	memcpy((void *)pcts->stat_count, (void *)psave->stat_count, sizeof(psave->stat_count));

	pcth->timer_count = psave->timer_count;
	pcth->real_count = psave->real_count;
	SAFE_FREE(pcth->timer);
	pcth->timer = cl_calloc(sizeof(psave->timer), 1);
	if (!pcth->timer) {
		pcts->max_timer_count = 0;
		return;
	}

	memcpy((void *)pcth->timer, (void *)psave->timer, sizeof(psave->timer));
}

static void comm_timer_slave_clean(slave_t *slave)
{
	
}

static void comm_timer_summary_proc_ktcz(slave_t *slave, rf_tlv_t* tlv)
{
	int i, len;
	user_t *user = slave->user;
	net_dev_timer_summary_t *pnts = (net_dev_timer_summary_t *)rf_tlv_val(tlv);
	cl_comm_timer_head_t *pcthl = &slave->comm_timer;
	cl_dev_timer_summary_t *pcts = &slave->timer_summary;
	u_int8_t *psta_count = (u_int8_t *)&pnts[1];
	cl_comm_timer_t *pctimer = NULL;


	if (pcts->max_timer_count && 
		(pcts->max_timer_count != pnts->max_timer_count)) {
		log_debug("max_timer_count changed last=%u now=%u !!!!!!!!!!!!!!!!!\n", 
			pcts->max_timer_count, pnts->max_timer_count);
		return;		
	}
	//更新摘要
	pcts->reserve = pnts->reserve;
	pcts->max_timer_count = pnts->max_timer_count;
	pcts->max_type_count = pnts->max_type_count;
	pcts->max_data_len = pnts->max_data_len;
	pcts->map = htonl(pnts->map);

	
	len = pcts->max_timer_count*sizeof(u_int16_t);
	if (pcts->stat_count == NULL) {
		pcts->stat_count = cl_calloc(len, 1);
		if(pcts->stat_count) {
			memset((void *)pcts->stat_count, 0xff, len);
		}
	}

	if (!pcts->stat_count) {
		log_debug("calloc failed !!!!!!!!!!!!!!!!!\n");
		return;
	}

	//pcts->need_query = 0;
	//判断是否需要查询
	for(i = 0; (i < 32) && (i < pcts->max_timer_count); i++) {
		if (pcts->map&BIT(i)) {
			if ((pcts->stat_count[i] == 0xffff) ||
				(pcts->stat_count[i] != (u_int16_t)psta_count[i])) {
				pcts->need_query |= pcts->map&BIT(i);
			}
			log_debug("pcts->stat_count[%d]=%u psta_count[%d]=%u\n", 
				i, pcts->stat_count[i], i, psta_count[i]);
		} else {
			//del
			pctimer = comm_timer_find_by_id(pcthl, i+1);
			if (pctimer) {
				pctimer->valid = 0;
				comm_timer_next_cal(&slave->comm_timer);
			}
			pcts->need_query &= ~(BIT(i));
		}
		pcts->stat_count[i] = (u_int16_t)psta_count[i];
	}

	pcts->sn = slave->sn;
	//查询
	if (pcts->need_query) {
		comm_timr_go_query(slave, pcts);
	}
}

static void comm_timer_query_rightnow(slave_t *slave, u_int8_t id)
{
	cl_dev_timer_summary_t *pcts = &slave->timer_summary;

	if (id > pcts->max_timer_count) {
		return;
	}

	pcts->need_query |= BIT((id-1));
	comm_timr_go_query(slave, pcts);
}

static void comm_timer_add_timer_proc(slave_t *slave, rf_tlv_t* tlv)
{
	user_t *user = slave->user;
	u_int8_t *pdata = (u_int8_t *)rf_tlv_val(tlv);
	int event = 0;

	if (tlv->len < 1) {
		return;
	}
	log_debug("comm_timer_add_timer_proc *pdata=%u\n", *pdata);
	if (tlv->type == RF_TT_CMD_ADD_TIMER) {
		if (*pdata == 0) {
			event = COMMON_UE_DEV_COMM_TIMER_ADD_FAILED;
		} else {
			//添加成功后里面就查询
			comm_timer_query_rightnow(slave, *pdata);
		}
	} else {
		if (*pdata == 0) {
			event = COMMON_UE_DEV_COMM_TIMER_DEL_FAILED;
		}
	}
	
	if (*(u_int8_t *)tlv->value == 0) {
        event_push(user->callback, event, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
	}
}

static void comm_timer_update_timer_proc_ktcz(slave_t *slave, rf_tlv_t* tlv)
{
	user_t *user = slave->user;
	cl_comm_timer_head_t *pcth = &slave->comm_timer;
	ucp_comm_timer_rf_t *ptimer = (ucp_comm_timer_rf_t *)rf_tlv_val(tlv);
	cl_comm_timer_t *pctimer = NULL;
	cl_dev_timer_summary_t *pcts = &slave->timer_summary;
	int len = 0;

	//log_mem_dump("timer recv", ptimer, tlv->len);
	if ((sizeof(ucp_comm_timer_rf_t) + ptimer->extened_len) > tlv->len) {
		log_debug("tlv->len=%u totallen=%u\n", tlv->len, (sizeof(ucp_comm_timer_rf_t) + ptimer->extened_len));
		return;
	}

	if (ptimer->id == 0) {
		log_debug("the timer id=0 is not correct\n");
		return;
	}

	pctimer = comm_timer_find_by_id(pcth, ptimer->id);
	if (pctimer) {
		_comm_timer_update_timer_rf_proc(user, pctimer, ptimer);
		pcts->need_query &= ~(BIT((ptimer->id-1)));
		comm_timr_go_query(slave, pcts);
		return;
	}

	len = (pcth->timer_count + 1)*sizeof(cl_comm_timer_t);
	pctimer = cl_calloc(len, 1);
	if (!pctimer) {
		return;
	}

	memcpy((void *)pctimer, (void *)pcth->timer, len - sizeof(cl_comm_timer_t));
	SAFE_FREE(pcth->timer);
	pcth->timer = pctimer;
	_comm_timer_update_timer_rf_proc(user, &pcth->timer[pcth->timer_count], ptimer);
	pcth->timer_count++;
	pcts->need_query &= ~(BIT((ptimer->id-1)));
	comm_timr_go_query(slave, pcts);

	pcth->comm_timer_valid = true;
}

void _ucp_comm_update_timer_rf_onoff(ucp_comm_timer_rf_t *ptimer, cl_comm_timer_t *pctimer)
{
	u_int8_t *pdata = (u_int8_t *)&ptimer[1];
	
	if (ptimer->extened_len == 0) {
		pctimer->type = 0;
		return;
	}

	switch(ptimer->type) {
	case UT_DEV_TYPE_ONOFF:
		if (*pdata == 1) {
			if (ptimer->duration) {
				pctimer->type = UT_TYPE_PERIOD_ONOFF;
			} else {
				pctimer->type = UT_TYPE_ON;
			}
		} else if (*pdata == 0){
			pctimer->type = UT_TYPE_OFF;
		}
		break;
	default:
		pctimer->type = 0;
		break;
	}
}


void ucp_comm_update_timer_rf_modify_pkt(user_t *user, ucp_comm_timer_rf_t *ptimer, cl_comm_timer_t *pctimer)
{	
	if (!user || !pctimer || !ptimer) {
		return ;
	}

	pctimer->id = ptimer->id;
	pctimer->enable = ptimer->enable;
	pctimer->week = ptimer->week;
	pctimer->hour = ptimer->hour;
	pctimer->min = ptimer->min;
	pctimer->duration = htons(ptimer->duration);
	pctimer->extened_len = ptimer->extened_len;
	pctimer->valid = 1;
	comm_timer_utc_2_local(pctimer);
	pctimer->week_cal = pctimer->week;
	if (pctimer->week&BIT(7)) {
		pctimer->week &= ~(BIT(7));
	} else {
		pctimer->week = 0;
	}
	
	switch(ptimer->type) {
	case UT_DEV_TYPE_ONOFF:
		_ucp_comm_update_timer_rf_onoff(ptimer, pctimer);
		break;
	default:
		log_debug("error !!!!!!!!!!!!!!!!!!!!!!!!1type=%u\n ", ptimer->type);
		pctimer->type = ptimer->type;
		break;
	}	


	log_debug("update id=%u enable=%u\n", pctimer->id, pctimer->enable);
}

static void _comm_timer_update_timer_rf_proc(user_t *user, cl_comm_timer_t *pctimer, ucp_comm_timer_rf_t *ptimer)
{
	ucp_comm_update_timer_rf_modify_pkt(user, ptimer, pctimer);
}

static void _comm_timer_update_timer_proc(user_t *user, bool slave, u_int64_t slave_sn, cl_comm_timer_t *pctimer, ucp_comm_timer_t *ptimer)
{
	ucp_comm_update_timer_modify_pkt(user, slave, slave_sn, ptimer, pctimer);
}


static void comm_timer_update_timer_proc(slave_t *slave, rf_tlv_t* tlv)
{
	user_t *user = slave->user;
	cl_comm_timer_head_t *pcth = &slave->comm_timer;
	ucp_comm_timer_t *ptimer = (ucp_comm_timer_t *)rf_tlv_val(tlv);
	cl_comm_timer_t *pctimer = NULL;
	cl_dev_timer_summary_t *pcts = &slave->timer_summary;
	int len = 0;

	//log_mem_dump("timer recv", ptimer, tlv->len);
	if ((sizeof(ucp_comm_timer_t) + ptimer->extened_len) > tlv->len) {
		log_debug("tlv->len=%u totallen=%u\n", tlv->len, (sizeof(ucp_comm_timer_t) + ptimer->extened_len));
		return;
	}

	log_debug("comm_timer_update_timer_proc ptimer->id=%u\n", ptimer->id);
	//这里限制下定时器id范围，
	if (ptimer->id == 0 ||
		ptimer->id > 32) {
		log_debug("the timer id=0 is not correct\n");
		return;
	}

	pctimer = comm_timer_find_by_id(pcth, ptimer->id);
	if (pctimer) {
		_comm_timer_update_timer_proc(user, true, slave->sn, pctimer, ptimer);
		pcts->need_query &= ~(BIT((ptimer->id-1)));
		comm_timr_go_query(slave, pcts);
		return;
	}

	len = (pcth->timer_count + 1)*sizeof(cl_comm_timer_t);
	pctimer = cl_calloc(len, 1);
	if (!pctimer) {
		return;
	}

	memcpy((void *)pctimer, (void *)pcth->timer, len - sizeof(cl_comm_timer_t));
	SAFE_FREE(pcth->timer);
	pcth->timer = pctimer;
	_comm_timer_update_timer_proc(user, true, slave->sn, &pcth->timer[pcth->timer_count], ptimer);
	pcth->timer_count++;
	pcts->need_query &= ~(BIT((ptimer->id-1)));
	comm_timr_go_query(slave, pcts);
	pcth->comm_timer_valid = true;
}

static void comm_timer_del_up_proc(slave_t *slave, rf_tlv_t* tlv)
{
	cl_comm_timer_head_t *pcth = &slave->comm_timer;
	cl_comm_timer_t *pctimer = NULL;
	u_int8_t id = *(u_int8_t *)rf_tlv_val(tlv);

	if (tlv->len == 0) {
		return;
	}

	pctimer = comm_timer_find_by_id(pcth, id);
	if (pctimer) {
		pctimer->valid = 0;
	}
}

static void comm_timer_dump(slave_t *slave)
{
	int i;
	cl_comm_timer_head_t *pthd = &slave->comm_timer;

	log_debug("timer_dump:\n");
	for(i = 0; i < pthd->timer_count; i++) {
		log_debug("timer=%d id=%u enale=%u type=%u hour=%u min=%u week=%u duration=%u exlen=%u\n", 
			i, pthd->timer[i].id, pthd->timer[i].enable, pthd->timer[i].type, pthd->timer[i].hour, 
			pthd->timer[i].min ,pthd->timer[i].week, pthd->timer[i].duration, pthd->timer[i].extened_len);
	}
}

//做凯特插座兼容
static void comm_timer_summary_proc_ext(slave_t *slave, rf_tlv_t* tlv)
{
	if (slave->ext_type == RF_EXT_TYPE_KTCZ) {
		comm_timer_summary_proc_ktcz(slave, tlv);
	} else {
		comm_timer_summary_proc(slave, rf_tlv_val(tlv));
	}	
}

static void comm_timer_update_timer_proc_exit(slave_t *slave, rf_tlv_t* tlv)
{
	if (slave->ext_type == RF_EXT_TYPE_KTCZ) {
		comm_timer_update_timer_proc_ktcz(slave, tlv);
	} else {
		comm_timer_update_timer_proc(slave, tlv);
	}	
}

bool parse_comm_timer(slave_t *slave, rf_tlv_t* tlv, u_int16_t data_len)
{
	int remain_len = (int)data_len;
	bool has_timer = false;

	//log_debug("enter %s data_len=%u\n", __FUNCTION__, data_len);
	//log_debug("type=%u len=%u\n", tlv->type, tlv->len);

	//长度校验
	if (data_len < sizeof(*tlv)) {
		log_debug("parse_comm_timer error data_len=%u\n", data_len);
		return false;
	}
	
	switch(tlv->type) {
	case RF_TT_CMD_GET_TIMER_SUMMARY:
		//查询摘要
		comm_timer_summary_proc_ext(slave, tlv);
		has_timer = true;
		break;
	case RF_TT_CMD_ADD_TIMER:
		//是否添加成功
		comm_timer_add_timer_proc(slave, tlv);
		has_timer = true;
		break;
	case RF_TT_CMD_DEL_TIMER_RET:
		//是否删除成功
		comm_timer_add_timer_proc(slave, tlv);
		has_timer = true;
		break;
	case RF_TT_CMD_GET_NEXT_TIMERS:
		//下次执行定时器查询
		//comm_timer_next_timer_proc(slave, tlv);
		has_timer = true;
		break;
	case RF_TT_CMD_GET_TIMER:
	case RF_TT_CMD_TIMER_UPDATE_PUSH:
		comm_timer_update_timer_proc_exit(slave, tlv);
		// TODO:计算下实时定时器个数
		comm_timer_count_cal(&slave->comm_timer);
		has_timer = true;
		// TODO:保存下从设备定时器数据   删除后需要保存
		//comm_timer_slave_save(slave);
		break;
	case RF_TT_CMD_TIMER_DEL_PUSH:
		comm_timer_del_up_proc(slave, tlv);
		has_timer = true;
		//删除定时器push
		break;
	default:
		break;
	}

	return has_timer;
}

bool comm_timer_support(u_int8_t ext_type)
{
	bool support = false;
	
	switch(ext_type) {
	case RF_EXT_TYPE_KTCZ:
	case RF_EXT_TYPE_WK_AIR:
	case RF_EXT_TYPE_WK_AIR2:
	case RF_EXT_TYPE_HEATING_VALVE:
	case RS_EXT_TYPE_ZHDJ:
	case RF_EXT_TYPE_DHXML:
	case RF_EXT_TYPE_DHXZH:
	case RF_EXT_TYPE_DHXCP:
	case RF_EXT_TYPE_LHX:
		support = true;
		break;
	default:
		break;
	}

	return support;
}

static bool parse_rfgw_tt(slave_t *slave, net_rfgw_tt_t *tt, u_int8_t action)
{
	rfdev_status_t  *info = (rfdev_status_t *)slave->dev_info.rfdev;
	bool rv = false;
	rf_tlv_t *tlv = (rf_tlv_t *)tt->data;

	
	if(info == NULL){
		info = (void*)cl_calloc(sizeof(*info), 1);
		if(info == NULL)
			return false;
		slave->dev_info.rfdev = info;
	}


	// 提供接口二次开发用
	if (tlv->type == RF_TT_CMD_RAWDATA) {
		app_proc_pkt_from_macbee_dev(slave->sn, tlv->value, tlv->len);
	}

	//处理下公共透传的定时tlv
	if (comm_timer_support(slave->ext_type) && 
		parse_comm_timer(slave, (rf_tlv_t*)(tt->data), tt->len)) {
		return true;
	}	
    //log_debug("parse_rfgw_tt slave sn[%012"PRIu64"] type[%u] action[%u]\n",slave->sn,slave->ext_type, action);
    slave->is_update_state = true;

	if (is_led_type(slave)) {
		rv = udp_rf_dev_update_raw_date(slave, tt->data, tt->len, false);
		return rv;
	}
	
    switch (slave->ext_type) {
        case RF_EXT_TYPE_LIGHT:
            rv =  _do_light_update(slave,tt);
            break;
        case RF_EXT_TYPE_LED_LAMP:
		case RF_EXT_TYPE_DWHF:
		case RF_EXT_TYPE_DWYKHF:
		case RF_EXT_TYPE_DWYSTGQ:
            rv = udp_rf_dev_update_raw_date(slave, tt->data, tt->len, false);
            break;		
        default:
        	rv = udp_rf_dev_update_date(slave, (rf_tlv_t*)(tt->data), action);
            break;
    }
	
	return rv;
		
}

static bool do_rfgw_tt(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	net_rfgw_tt_t *tt = (net_rfgw_tt_t*)(obj+1);
	user_t *user = air_ctrl->sac->user;
	slave_t *slave;

	if(obj->param_len < (sizeof(*tt))) {
		return false;
	}

	tt->sn = ntoh_ll(tt->sn);
	tt->len = ntohs(tt->len);
	if(obj->param_len < (sizeof(*tt) + tt->len)) {
		return false;
	}
	
	slave = slave_lookup_by_sn(user, tt->sn);
	if(slave == NULL)
		return false;

	if(parse_rfgw_tt(slave, tt, action)){
        event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
	}
    
	return false;
}

static bool parse_rfgw_tt_cache(slave_t *slave, net_rfgw_tt_t *tt, u_int8_t action)
{
	rfdev_status_t  *info = (rfdev_status_t *)slave->dev_info.rfdev;
	bool rv = false;
	
	if(info == NULL){
		info = (void*)cl_calloc(sizeof(*info), 1);
		if(info == NULL)
			return false;
		slave->dev_info.rfdev = info;
	}
	
    log_debug("parse_rfgw_tt_cache slave sn[%012"PRIu64"] type[%u] len %d\n",slave->sn,slave->ext_type, tt->len);

	app_proc_pkt_from_macbee_dev_cache(slave->sn, tt->data, tt->len);

    slave->is_update_state = true;

	if (is_led_type(slave)) {
		rv = udp_rf_dev_update_raw_date(slave, tt->data, tt->len, true);
		return rv;
	}
	
    switch (slave->ext_type) {
        case RF_EXT_TYPE_LIGHT:
            break;
        case RF_EXT_TYPE_LED_LAMP:
		case RF_EXT_TYPE_DWHF:
		case RF_EXT_TYPE_DWYKHF:
		case RF_EXT_TYPE_DWYSTGQ:
			rv = udp_rf_dev_update_raw_date(slave, tt->data, tt->len, true);
            break;
		default:
			rv = udp_rf_dev_update_cache_date(slave, action, tt->data, tt->len);
			break;
    }
	
	return rv;
		
}


static bool do_rfgw_tt_cache(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	net_rfgw_tt_t *tt = (net_rfgw_tt_t*)(obj+1);
	user_t *user = air_ctrl->sac->user;
	slave_t *slave;

	//log_mem_dump("do_rfgw_tt_cache", (u_int8_t *)(obj+1), obj->param_len);	

	log_debug("enter %s paramlen=%u sn=%"PRIu64" len=%u\n", __FUNCTION__, obj->param_len, tt->sn, tt->len);
	if(obj->param_len < (sizeof(*tt))) {
		//log_debug("do_rfgw_tt_cache error len=%u tt->len=%u\n", obj->param_len, tt->len);
		return false;
	}

	tt->sn = ntoh_ll(tt->sn);
	tt->len = ntohs(tt->len);
	
	if(obj->param_len < (sizeof(*tt) + tt->len)) {
		log_debug("do_rfgw_tt_cache error len=%u tt->len=%u\n", obj->param_len, tt->len);
		return false;
	}
	
	slave = slave_lookup_by_sn(user, tt->sn);
	if(slave == NULL) {
		log_debug("not foundj sn=%"PRIu64"\n", tt->sn);
		return false;
	}

	if(parse_rfgw_tt_cache(slave, tt, action)){
        event_push(slave->user->callback, UE_INFO_MODIFY, slave->handle, slave->user->callback_handle);
	}
    
	return false;
}


static u_int8_t count_dev_group(rfgw_priv_t *p)
{
	u_int32_t i = ARRAY_SIZE(p->devgroup);
	u_int8_t n = 0;
	for(i = 0; i < ARRAY_SIZE(p->devgroup); i++){
		if(p->devgroup[i])
			n++;
	}
	return n;	
}

static bool do_rfgw_dev_name(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	net_dev_name_t *t = (net_dev_name_t*)(obj+1);
	user_t *user = air_ctrl->sac->user;
	slave_t *slave;

	t->sn = ntoh_ll(t->sn);	
	if(obj->param_len < sizeof(*t))
		return false;
	
	slave = slave_lookup_by_sn(user, t->sn);
	if(slave == NULL)
		return false;

	t->name[sizeof(t->name)-1] = 0;
	STR_REPLACE(slave->name, (char*)t->name);
	
	return true;
	
}

static net_dev_group_t* _find_cl_gp_by_pkts(net_dev_group_t *ng,u_int8_t count,u_int8_t g_id)
{
    u_int8_t i;
    
    for (i = 0; i< count; i++,ng++) {
        if (ng->group_id == g_id) {
            return ng;
        }
    }
    return NULL;
}

static void _sync_group_by_pkt(rfgw_priv_t* gw,net_dev_group_t *ng,u_int8_t count)
{
    int m_c = (int)(sizeof(gw->devgroup)/sizeof(cl_dev_group_t *));
    int i;
    
    
    for (i = 0; i < m_c; i++) {
        if (gw->devgroup[i] != NULL && _find_cl_gp_by_pkts(ng,count,gw->devgroup[i]->group_id) == NULL) {
            cl_free(gw->devgroup[i]);
            gw->devgroup[i] = NULL;
        }
    }
}

static bool do_rfgw_dev_group(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	net_dev_group_t *src = (net_dev_group_t *)(obj+1);
	cl_dev_group_t *dst;
	u_int8_t i;
    u_int8_t count = (u_int8_t)(obj->param_len/sizeof(*src));
	cl_dev_group_t querg;

    _sync_group_by_pkt(p,src,count);
    if(count == 0){
        return true;
    }
    
    
    for (i = 0; i< count; i++,src++) {
        
        if (src->group_id >= 32) {
            log_err(false,"error group id [%u]from device\n",src->group_id);
            continue;
        }
        
        if (src->dev_count >128) {
            log_err(false,"error group dev count; group id [%u] dev_count[%u]\n",src->group_id,src->dev_count);
            continue;
        }
        
        if(p->devgroup[src->group_id] == NULL){
            p->devgroup[src->group_id] = cl_calloc(sizeof(*dst), 1);
        }
        if(p->devgroup[src->group_id] == NULL){
            continue;
        }

        dst = p->devgroup[src->group_id];
		
		
        dst->group_id = src->group_id;
        dst->group_type = src->group_type;
        dst->dev_cnt = src->dev_count;
        memcpy(dst->name, src->name, RF_MAX_NAME);

		memcpy((void *)&querg, (void *)dst, sizeof(querg));
    	querg.reserved = ACTION_QUERY;
		rfgw_dev_group(air_ctrl->sac->user, &querg);
    }

	
	return true;
}

static bool do_rfgw_dev_group_member(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
    net_dev_group_t *src = (net_dev_group_t *)(obj+1);
    cl_dev_group_t *dst;
    u_int8_t i;
    
    if(obj->param_len < (sizeof(*src) + src->dev_count*sizeof(u_int64_t)))
        return false;
    
    if (src->group_id >= 32) {
        log_err(false,"error group id [%u]from device\n",src->group_id);
        return false;
    }
    
    if (src->dev_count >128) {
        log_err(false,"error group dev count; group id [%u] dev_count[%u]\n",src->group_id,src->dev_count);
        return false;
    }
    
    if(p->devgroup[src->group_id] == NULL){
        p->devgroup[src->group_id] = cl_calloc(sizeof(*dst), 1);
    }
    
    if(p->devgroup[src->group_id] == NULL){
        return false;
    }
    
    dst = p->devgroup[src->group_id];
    dst->group_id = src->group_id;
    dst->group_type = src->group_type;
    dst->dev_cnt = src->dev_count;
    dst->query_dev = true;
    memcpy(dst->name, src->name, RF_MAX_NAME);
    for( i = 0; i < src->dev_count; i++){
        dst->dev_sn[i] = ntoh_ll(src->sn[i]);
    }	
    
    return true;
}

static void do_rfgw_upgrade_check(user_t *user, rfgw_priv_t *p)
{
	if (!user->rf_need_up) {
		return;
	}
	user->rf_need_up = false;
	//如果在升级中,就报错
	if (p->is_upgrade) {
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		return;
	}
	
	//镜像头部校验
	if (sa_stm_upgrade_file_check(user, user->rf_up_filepath) != RS_OK) {
		log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
		event_push(user->callback, UE_RFGW_DEV_UPGRDE_ERR, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		return;
	}

	if (user->is_support_spe_up) {
		uc_spe_upgrade_pre_t pre;
		memset((void *)&pre, 0, sizeof(pre));
		if (sa_stm_spe_get_info(user->rf_up_filepath, &pre)) {
			sa_stm_spe_upgrade_file(user, 0, NULL, &pre, 1);
		}
	}
	
	if (sa_stm_upgrade_file(user, user->rf_up_filepath) != RS_OK) {
		log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
		event_push(user->callback, UE_RFGW_DEV_UPGRDE_ERR, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		return;
	}

	p->is_upgrade = 1;

/*
	if (sa_dev_upgrade_file(user, user->rf_up_filepath) != RS_OK) {
			log_debug("enter UE_RFGW_DEV_UPGRDE_ERR222\n");
        event_push(user->callback, UE_RFGW_DEV_UPGRDE_ERR, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		return;	
	}
*/

	if (user->rf_up_type > 0 && 
		user->rf_up_type < D_T_MAX) {
		p->upgrade_status[user->rf_up_type] = UP_STATUS_UPGRADING;
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
        event_cancel_merge(user->handle);
		return;	
	}
}

static void do_rfgw_dev_support_parse(user_t *user, u_int8_t support)
{
	//BIT0，是否支持直接获取子设备的批量用户
	if (support&BIT(0)) {
		user->is_support_batch_user = true;
	}
}

static bool do_rfgw_dev_param(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
    net_dev_param_t *src = (net_dev_param_t *)(obj+1);
	user_t *user = air_ctrl->sac->user;

    if(obj->param_len < sizeof(net_dev_param_t)) {

        return false;
    }

	if (p->commpat != src->commpat) {
		p->commpat = src->commpat;
	}

	if (p->channel != src->channel) {
		p->channel = src->channel;
	}

	do_rfgw_dev_support_parse(user, src->support);

	log_debug("do_rfgw_dev_param src->is_upgrade=%u\n", src->is_upgrade);
	if (p->is_upgrade != src->is_upgrade) {
		p->is_upgrade = src->is_upgrade;
		//如果升级成功就清除标志
		user->rf_up_type = 0;
	}

	//判断下是否需要升级
	do_rfgw_upgrade_check(user, p);
	
    return true;
}

static void _rfgw_list_digest_query(smart_air_ctrl_t* air_ctrl, u_int8_t *pid_src, u_int8_t num)
{
	int i;
	u_int16_t len;
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_list_t *list = (net_rfgw_list_t*)(uo+1);
	net_rfgw_dev_t *pnrde = (net_rfgw_dev_t*)&list[1];
	user_t *user = air_ctrl->sac->user;

	memset(buf, 0, sizeof(buf));
	list->count = (u_int8_t)num;
	list->total = 0;

	for(i = 0; i < num; i++) {
		pnrde[i].sn = pid_src[i];
		pnrde[i].sn = pnrde[i].sn<<56;
		pnrde[i].sn = ntoh_ll(pnrde[i].sn);
	}
	len = sizeof(net_rfgw_list_t) + sizeof(net_rfgw_dev_t)*num;
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_LIST_WITH_ID, sizeof(ucp_obj_t)+len);
	sa_ctrl_obj_value(user->uc_session, UCA_GET, false, 0x1, uo, sizeof(ucp_obj_t)+len);
	log_debug("enter %s query num=%u len=%u !!!!\n", __FUNCTION__, num, len);	
}

static void rfgw_slave_del(user_t* user, u_int8_t id)
{
	slave_t *pos, *n;
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, &user->slave, link) {
		if (pos->id == id) {
			log_debug("rfgw_slave_del del sn=%"PRIu64" \n", pos->sn);
			stlc_list_del(&pos->link);
			slave_free(pos);
			rf_slave_save(user);
			break;
		}
	}
}

static void rfgw_slave_dbc_sync(user_t* user, u_int8_t id, u_int8_t support)
{
	slave_t *pos, *n;
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, &user->slave, link) {
		if (pos->id == id) {
			pos->is_support_dbc = support;
			break;
		}
	}
}

static void rfgw_slave_flag_up(user_t* user, u_int8_t id, u_int8_t flags)
{
	slave_t *slave, *n;
	
	stlc_list_for_each_entry_safe(slave_t, slave, n, &user->slave, link) {
		if (slave->id == id) {
	   		do_rfgw_slave_flags_parse(slave, NULL, flags, true);
			break;
		}
	}
}

static void rfgw_slave_cache_query_set(user_t* user, u_int8_t id)
{
	slave_t *slave, *n;
	bool found = false;
	
	stlc_list_for_each_entry_safe(slave_t, slave, n, &user->slave, link) {
		if (slave->id == id) {
	   		slave->need_cache_query = 1;
			found = true;
			break;
		}
	}

	if (!found) {
		log_debug("not found id=%u\n", id);
	}
}

static void rfgw_slave_cache_quick_query(user_t *user,smart_air_ctrl_t* ac)
{
    u_int8_t buf[2048] = {0};
    u_int16_t pos = 0,n = 0;
    u_int16_t max_pkt_len = 600;
    u_int8_t cmd_len;
    slave_t *slave;
    ucp_obj_t* uo ;
    net_rfgw_tt_t* nf;
    rf_tlv_t* rt;
    
    if (!user) {
        return;
    }
    
    //log_debug("rfgw_slave_cache_quick_query sn [%012"PRIu64"]\n",ac->sac->user->sn);
    //检查在线设备是否更新，没更新的，组织查询报文
    stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
        if ((slave->status ==  BMS_BIND_ONLINE) &&
			(slave->need_cache_query)) {
			slave->need_cache_query = 0;
            uo = (ucp_obj_t*)&buf[pos];
            nf = (net_rfgw_tt_t*)(uo+1);
            rt = (rf_tlv_t*)(nf+1);
            ////////////////////////////////////
            if (slave->ext_type == RF_EXT_TYPE_LIGHT) {
                nf->data[0] = RFTT_WORK_QUERY;
                cmd_len = 0x1;
            }else{
                // 通用的填充
                cmd_len = udp_rf_dev_mk_raw_stat_query_pkt(slave, nf->data);
                if (cmd_len == 0 ) {
                    //看看是不是tlv形式的
                    cmd_len = udp_rf_dev_mk_stat_query_pkt(slave,rt);
                    if (cmd_len == 0) {
                        continue;
                    }
                }
            }
            
            //log_debug("xx rfgw_slave_cache_quick_query slave sn [%012"PRIu64"] ext_type [%u] ac->is_support_rf_cache=%u\n",
				//slave->sn,slave->ext_type, ac->is_support_rf_cache);
            if (ac->is_support_rf_cache) {
            	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT_CACHE, sizeof(*nf)+cmd_len);
			} else {
            	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*nf)+cmd_len);
			}
            nf->sn = ntoh_ll(slave->sn);
            nf->len = ntohs(cmd_len);
            pos += (sizeof(*uo)+sizeof(*nf)+cmd_len);
            n++;
            
            //一个数据包完成
            if (pos >= max_pkt_len) {
                //sa_set_obj_value_only(user->uc_session, (u_int8_t)n, buf, pos);
				sa_ctrl_obj_value(user->uc_session,UCA_GET,false, (u_int8_t)n, buf, pos);
                pos = 0;
                n = 0;
            }
        }
    }
    
    // 不够一个数据包的
    if (n>0 && pos >0) {
        //log_debug("query all slave packet! *************************************** \n");
        //sa_set_obj_value_only(user->uc_session, (u_int8_t)n, buf, pos);
		sa_ctrl_obj_value(user->uc_session,UCA_GET,false, (u_int8_t)n, buf, pos);
    }
}

static void rfgw_slave_status_update(user_t* user, int id, u_int8_t flags)
{
	slave_t *slave, *n;
	bool found = false;
	
	stlc_list_for_each_entry_safe(slave_t, slave, n, &user->slave, link) {
		if (slave->id == id) {
			found = true;
			break;
		}
	}	

	if (!found) {
		return;
	}

	//log_debug("rfgw_slave_status_update sn=%"PRIu64" flags=%02x slave->status=%u\n", slave->sn, flags, slave->status);
   do_rfgw_slave_flags_parse(slave, NULL, flags, false);
}

static void do_rf_slave_all_flag(user_t *user, u_int8_t flag)
{
	slave_t *pos, *n;

	if (!user) {
		return;
	}
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, &user->slave, link) {
		pos->flag = flag;
	}	
}

static void do_rf_slave_flag(user_t *user, u_int8_t id, u_int8_t flag)
{
	slave_t *pos, *n;

	if (!user) {
		return;
	}
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, &user->slave, link) {
		if (pos->id != id) {
			continue;
		}

		pos->flag = flag;
		break;
	}	
}

static void do_del_rf_slave_by_flag(user_t *user, u_int8_t flag)
{
	slave_t *pos, *n;
	bool need_save = false;

	if (!user) {
		return;
	}
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, &user->slave, link) {
		if (pos->flag != flag) {
			continue;
		}
		log_debug("do_del_rf_slave_by_flag del sn=%"PRIu64" \n", pos->sn);
		stlc_list_del(&pos->link);
		slave_free(pos);
		need_save = true;
	}

	if (need_save) {
		rf_slave_save(user);
	}
}

static void rfgw_list_digest_query(smart_air_ctrl_t* air_ctrl, net_dev_list_digest_t *pndl_n,
	net_dev_list_digest_t *pndl_l, int num)
{
	u_int8_t id_array[10];
	int i;
	u_int8_t n;
	u_int8_t query_num = 0;

	//给所有从设备打上标记
	do_rf_slave_all_flag(air_ctrl->sac->user, 1);
	log_debug("enter %s  sn=%"PRIu64" need query num=%u\n", __FUNCTION__, air_ctrl->sac->user->sn, num);
	// TODO:这里要保证下第一次查询，要所有都查询，但要知道哪些id是有效的
	for(i = 0, n = 0; i < num; i++) {
		if (pndl_n->dev[i].flag&RFDEV_FLAG_INVALID) {
			rfgw_slave_del(air_ctrl->sac->user, i);
			continue;
		}
		log_debug("i=%u flag=%02x\n", i, pndl_n->dev[i].flag);
		if (pndl_n->dev[i].flag&RFDEV_FLAG_DBC) {
			rfgw_slave_dbc_sync(air_ctrl->sac->user, i, 1);
		} else {
			rfgw_slave_dbc_sync(air_ctrl->sac->user, i, 0);
		}
		//清除标记
		do_rf_slave_flag(air_ctrl->sac->user, i, 0);
		//更新从设备状态
		//log_debug("rfgw_list_digest_query id=%u flag=%02x\n", i, pndl_n->dev[i].flag);
		rfgw_slave_status_update(air_ctrl->sac->user, i, pndl_n->dev[i].flag);

		//cache查询
		//log_mem_dump("cache dump", (u_int8_t *)&pndl_n->dev[i], sizeof(net_dev_list_info_t));
		log_debug("rfgw_list_digest_query id=%d need cache cache_changed_num=%u\n", i, pndl_n->dev[i].cache_changed_num);
		if ((air_ctrl->is_support_rf_cache) && 
			(pndl_n->dev[i].cache_changed_num != pndl_l->dev[i].cache_changed_num)) {
			//log_debug("rfgw_list_digest_query id=%d need cache cache_changed_num=%u\n", i, pndl_n->dev[i].cache_changed_num);
			rfgw_slave_cache_query_set(air_ctrl->sac->user, i);
		}

		log_debug("i=%u n_chanum=%u l_changnu=%u n=%u\n", i, pndl_n->dev[i].changed_num, pndl_l->dev[i].changed_num, n);
		if (pndl_n->dev[i].changed_num != pndl_l->dev[i].changed_num) {
			//log_debug("n_chanum=%u l_changnu=%u n=%u\n", pndl_n->dev[i].changed_num, pndl_l->dev[i].changed_num, n);
			id_array[n++] = i;
		}

		//更新状态
		if (pndl_n->dev[i].flag != pndl_l->dev[i].flag) {
			rfgw_slave_flag_up(air_ctrl->sac->user, i, pndl_n->dev[i].flag);
		}		
		
		if (n >= 10) {
			_rfgw_list_digest_query(air_ctrl, id_array, n);
			query_num += n;
			n = 0;
		}
	}

	if (n > 0) {
		_rfgw_list_digest_query(air_ctrl, id_array, n);
		query_num += n;
		n = 0;
	}

	if (query_num == 0) {
		rf_slave_save(air_ctrl->sac->user);
	}

	air_ctrl->sac->user->query_num = query_num;

	if (air_ctrl->is_support_rf_cache) {
		//log_debug("rfgw_list_digest_query need go rf cache query\n");
		rfgw_slave_cache_quick_query(air_ctrl->sac->user, air_ctrl);
	}

	do_del_rf_slave_by_flag(air_ctrl->sac->user, 1);
}

static void rfgw_list_digest_response(smart_air_ctrl_t* air_ctrl, u_int32_t seq)
{
    pkt_t *pkt;
    ucp_ctrl_t* uc;
    ucc_session_t* s = air_ctrl->sac->user->uc_session;
    ucp_obj_t* obj;
    u_int32_t *pseq;
	int n = 0;
	RS ret = RS_OK;
	
    pkt = uc_pkt_new(s,CMD_UDP_CTRL, sizeof(ucp_ctrl_t) + sizeof(ucp_obj_t) + sizeof(*pseq)
                     ,true, s->select_enc?true:false, 0x01, s->client_sid, s->device_sid, s->peer_request_id);
	uc = get_ucp_payload(pkt, ucp_ctrl_t);
    obj = (ucp_obj_t*)(uc+1);
	pseq = (u_int32_t*)(obj+1);

	uc->action = UCA_PUSH;
    uc->count = 1;
	
	fill_net_ucp_obj(obj, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_LIST_DIGEST_ACK, sizeof(*pseq));
	*pseq = htonl(seq);

	//log_mem_dump("rfgw_list_digest_response", pkt->data, pkt->total);	
    ret = ucc_response_send(s,pkt);
    //n = uc_send_pkt_raw(s->sock, s->ip, s->port, pkt);
	//pkt_free(pkt);
    //log_debug("rfgw_list_digest_response seq=%08x ret=%d \n", seq, ret);

}

static bool do_rfgw_dev_list_digest(smart_air_ctrl_t* air_ctrl, u_int8_t action, ucp_obj_t* obj)
{
    rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	net_dev_list_digest_t *pndld = (net_dev_list_digest_t *)&obj[1];
	int n = 0;

	if (obj->param_len < sizeof(net_dev_list_digest_t)) {
		return false;
	}

	pndld->seq = htonl(pndld->seq);
	//log_debug("enter %s len=%u seq=%08x\n", __FUNCTION__, obj->param_len, pndld->seq);
	if (p->list_len != obj->param_len) {
		SAFE_FREE(p->plist);
		p->list_len = 0;
		p->plist = cl_calloc(obj->param_len, 1);
		if (!p->plist) {
			return false;
		} else {
			p->list_len = obj->param_len;
			//初始化下区别
			p->plist->seq = 0xffffffff;
		}
	}

	if (!p->plist) {
		return false;
	}

	n = (obj->param_len - sizeof(net_dev_list_digest_t))/sizeof(net_dev_list_info_t);
	//先直接返回个确认
	rfgw_list_digest_response(air_ctrl, pndld->seq);
	rfgw_list_digest_query(air_ctrl, pndld, p->plist, n);
	memcpy((void *)p->plist, (void *)pndld, obj->param_len);

	//尽快触发查询
	//_slave_min_ver_sync(air_ctrl->sac->user, air_ctrl);
	// TODO:在这里开启定时器查询所有类型升级版本
	//_slave_upgrade_check(air_ctrl->sac->user, air_ctrl);

	return true;
}

bool do_comm_gw_big_data_send(slave_t *slave, u_int8_t action, u_int8_t *pdata, int len)
{
	char buf[1024] = {0};
	ucp_obj_t *uo = (ucp_obj_t *)buf;
	u_int8_t *pvalue = (u_int8_t *)&uo[1];
	user_t *user = slave->user;

	if (!user || len > 900) {
		log_debug("error %s %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", __FUNCTION__, __LINE__);
		return false;
	}
	
	memset(buf, 0, sizeof(buf));
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_BIGDATA_CMD, (u_int16_t)len);
	if (len > 0) {
		memcpy(pvalue, pdata, len);
	}
	sa_ctrl_obj_value(user->uc_session, action, false, 0x1, uo, sizeof(ucp_obj_t)+len);
	log_info("send UCAT_IA_RFGW_BIGDATA_CMD=%u len=%u action=%u\n", UCAT_IA_RFGW_BIGDATA_CMD , len, action);
	
	return true;
}

static bool do_rfgw_big_data_proc(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	int remain_len = 0;
	slave_t *slave;
	u_int64_t sn;
	uc_tlv_t *ptlv = (uc_tlv_t *)&obj[1];

	remain_len = obj->param_len;

	ptlv->type = htons(ptlv->type);
	ptlv->len = htons(ptlv->len);
	log_debug("do_rfgw_big_data_proc len=%d ptlv->type=%u ptlv->len=%u\n", 
		remain_len, ptlv->type, ptlv->len);

	while(remain_len >= (int)(sizeof(*ptlv) + ptlv->len)) {
		if (ptlv->len < sizeof(u_int64_t)) {
			log_debug("%s %d err len=%u\n", __FUNCTION__, __LINE__, ptlv->len);
			return true;
		}

		memcpy((void *)&sn, tlv_val(ptlv), sizeof(u_int64_t));
		sn = ntoh_ll(sn);

		log_info("sn=%"PRIu64"\n", sn);
		slave = slave_lookup_by_sn(air_ctrl->sac->user, sn);
		if (!slave) {
			log_debug("%s %d slave is null\n" ,__FUNCTION__, __LINE__);
			return true;
		}

		log_debug("do_rfgw_big_data_proc sn=%"PRIu64"\n", slave->sn);
		if (slave->ext_type == RF_EXT_TYPE_HTLLOCK) {
			do_htllock_tlv_proc(slave, ptlv, action);
		}

		remain_len -= (int)(sizeof(*ptlv) + ptlv->len);
		ptlv = (uc_tlv_t *)((u_int8_t *)tlv_val(ptlv) + ptlv->len);
		if (remain_len > (int)sizeof(*ptlv)) {
			ptlv->type = htons(ptlv->type);
			ptlv->len = htons(ptlv->len);
		}
	}

	return true;
}

static bool do_rfgw_img_proc(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	int i, num;
	user_t *user = air_ctrl->sac->user;
	rfgw_priv_t *p = (rfgw_priv_t*)air_ctrl->com_udp_dev_info.device_info;
	cl_rfdev_img_cache_query_t*pq = NULL;
	cl_rfdev_img_cache_info_t *pinfo = NULL;

	log_debug("enter %s\n", __FUNCTION__);
	if (obj->param_len < sizeof(*pq)) {
		log_err(false, "err len=%u\n", obj->param_len);
		return false;
	}
	num = (int)((obj->param_len - sizeof(*pq))/sizeof(cl_rfdev_img_cache_info_t));
	pq = (cl_rfdev_img_cache_query_t *)&obj[1];

	if (pq->img_num != num) {
		log_err(false, "cal num=%u pq->img_num=%u\n", num, pq->img_num);
		return false;
	}

	p->img_cache_num = 0;
	SAFE_FREE(p->pimg_cache);
	p->pimg_cache = cl_calloc((num * sizeof(cl_rfdev_img_cache_info_t)), 1);
	if (!p->pimg_cache) {
		return false;
	}

	for(i = 0; i < num; i++) {
		memcpy((void *)&p->pimg_cache[i], (void *)&pq->cache[i], sizeof(cl_rfdev_img_cache_info_t));
		pinfo = &p->pimg_cache[i];
		log_debug("cache[%d] sub=%u ext=%u index=%u majvor=%u minor=%u imatype=%u action=%u\n", 
			i, pinfo->sub_type, pinfo->ext_type, pinfo->index, pinfo->major_ver, 
			pinfo->minor_ver, pinfo->img_type, pinfo->up_action);
	}

	p->img_cache_num = num;
	
	return true;
}

void do_rfgw_index_add(slave_t *slave)
{	
	if (slave->ext_type != RF_EXT_TYPE_HTLLOCK) {
		return;
	}

	do_rfgw_index_hllock_add(slave);	
}

static bool hpgw_do_lamp_ctrl(cl_hpgw_info_t *hpgw_info, cl_rf_lamp_stat_t *src, u_int8_t *out, int *out_len)
{
	hpgw_lamp_ctrl_param_t *dest;
	cl_rf_lamp_t *rl = &(hpgw_info->lamp_stat);
	
    dest = (hpgw_lamp_ctrl_param_t*)out;
    *out_len = sizeof(*dest);
    
    dest->o_wc_l = src->action;
    dest->o_r = src->R;
    dest->o_g = src->G;
    dest->o_b = src->B;
    dest->o_l = src->L;
    dest->o_c = src->cold;

	//dest->type = src->flag;
    
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
    //dest->sub_cmd = SC_SET;
   // dest->type = 0x0;
    dest->hwconf = src->ctrl_mode;
#if 0
	
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
#endif
	return RS_OK;
}

static bool hpgw_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	rfgw_priv_t *priv_info;
	cl_hpgw_info_t *hpgw_info;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	u_int8_t *pdata = NULL;
	u_int8_t dest_pkt_len = 0;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];
	if(!user->smart_appliance_ctrl) {
		log_err(false, "%s error handle %08x\n", __FUNCTION__, info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "%s error handle %08x\n", __FUNCTION__, info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	priv_info = ac->com_udp_dev_info.device_info;

	hpgw_info = (cl_hpgw_info_t *)&priv_info;

	switch(info->action){
	case ACT_RFGW_HPGW_APPINFO:
		{
			ucp_hpgw_appinfo_request_t *request = cci_pointer_data(info);
			int dest_pkt_len = sizeof(*request);			
				
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HPGW, UCAT_IA_HPGW_APPINFO, dest_pkt_len);
			memcpy(uo + 1, request, dest_pkt_len);
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
	case ACT_RFGW_HPGW_SMS:
		{
			ucp_hpgw_hpgw_sms_request_t *request = cci_pointer_data(info);
			int dest_pkt_len = sizeof(*request);			
				
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HPGW, UCAT_IA_HPGW_SMS, dest_pkt_len);
			memcpy(uo + 1, request, dest_pkt_len);
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_RFGW_HPGW_CONFIG_USER:
		{
			ucp_hpgw_phone_user_request_t *request = cci_pointer_data(info);
			int dest_pkt_len = sizeof(*request);	

			request->phone_number = ntoh_ll(request->phone_number);
				
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HPGW, UCAT_IA_HPGW_ADDUSER, dest_pkt_len);
			memcpy(uo + 1, request, dest_pkt_len);
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;
		
	case ACT_RFGW_HPGW_DEL_USER:
		{
			ucp_hpgw_phone_user_del_t *request = cci_pointer_data(info);
			int dest_pkt_len = sizeof(*request);	

			request->phone_number = ntoh_ll(request->phone_number);
				
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HPGW, UCAT_IA_HPGW_DELSUER, dest_pkt_len);
			memcpy(uo + 1, request, dest_pkt_len);
	        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	case ACT_RFGW_HPGW_LAMP_CTRL:
		{
			int dest_pkt_len = 0;

			hpgw_do_lamp_ctrl(hpgw_info, cci_pointer_data(info), (u_int8_t*)&uo[1], &dest_pkt_len);
			fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HPGW, UCAT_IA_HPGW_LAMP, dest_pkt_len);
			sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);
		}
		break;

	default:
		break;
	}

	return true;
}

static bool hpgw_update_lamp_stat( cl_hpgw_info_t *hpgw_info, u_int8_t *data, int data_len)
{
	hpgw_lamp_ctrl_param_t* pkt = (hpgw_lamp_ctrl_param_t *)data;
	cl_rf_lamp_t *rl = &hpgw_info->lamp_stat;
	u_int8_t m,n;
	u_int16_t org_rgb_l, tmp;


	log_debug("	hpgw lamp R %u G %u B %u power %u\n", pkt->R, pkt->G, pkt->B, pkt->power);
	
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



bool hpgw_update_data(smart_air_ctrl_t* ac, u_int8_t action, ucp_obj_t* obj)
{
	rfgw_priv_t *priv_info;
	cl_hpgw_info_t *hpgw_info;

	priv_info = (rfgw_priv_t *)(ac->com_udp_dev_info.device_info);

	if (!priv_info) {
		log_err(false, "not initd %s\n", __FUNCTION__);
		return false;
	}

	hpgw_info = (cl_hpgw_info_t *)&priv_info->hpinfo;

	log_debug("hpgw update data attr %u param len %u\n", obj->attr, obj->param_len);
	
    switch (obj->attr) {
    	case UCAT_IA_HPGW_ALARM_CONFIG:
			{				
				int plen, i;
				ucp_hpgw_info_t *src = OBJ_VALUE(obj, ucp_hpgw_info_t *);
				
		
				if (is_obj_less_than_len(obj, sizeof(*src))) {
					return false;
				}

				if (src->phone_user_num > 16) {
					return false;
				}
			
				plen = sizeof(*src) + src->phone_user_num * sizeof(ucp_hpgw_phone_user_t);
				
				if (is_obj_less_than_len(obj, plen)) {
					return false;
				}

				hpgw_info->support_appinfo = src->support_appinfo;
				hpgw_info->support_sms = src->support_sms;
				hpgw_info->sms_lang= src->sms_lang;
				hpgw_info->phone_user_num = src->phone_user_num;

				log_debug("	support_appinfo %u support_sms %u lang %u user num %u\n", 
					hpgw_info->support_appinfo, hpgw_info->support_sms, hpgw_info->sms_lang, hpgw_info->phone_user_num);
				
				for (i = 0; i < src->phone_user_num; i++) {
					memcpy(&hpgw_info->users[i].phome_number, &src->users[i].phome_number, sizeof(u_int64_t));
					hpgw_info->users[i].phome_number = ntoh_ll(hpgw_info->users[i].phome_number);
					memcpy(hpgw_info->users[i].name, src->users[i].name, sizeof(hpgw_info->users[i].name));
						log_debug("	user[%s] number %"PRIu64"\n", hpgw_info->users[i].name, hpgw_info->users[i].phome_number);
				}
				
			}
			break;

		case UCAT_IA_HPGW_APPINFO:
			{
				ucp_hpgw_appinfo_request_t *value = OBJ_VALUE(obj, ucp_hpgw_appinfo_request_t *);
				int plen = sizeof(*value);
				
				if (is_obj_less_than_len(obj, plen)) {
					return false;
				}

				hpgw_info->support_appinfo = value->appinfo;

				log_debug("	update appinfo to %u\n", hpgw_info->support_appinfo);
			}
			break;

		case UCAT_IA_HPGW_SMS:
			{
				ucp_hpgw_hpgw_sms_request_t *value = OBJ_VALUE(obj, ucp_hpgw_hpgw_sms_request_t *);
				int plen = sizeof(*value);
				
				if (is_obj_less_than_len(obj, plen)) {
					return false;
				}

				hpgw_info->support_sms= value->sms;
				hpgw_info->sms_lang = value->lang;

				log_debug("	update sms onoff %u lang %u\n", hpgw_info->support_sms, hpgw_info->sms_lang);
			}
			break;

		case UCAT_IA_HPGW_ADDUSER:
			{
				ucp_hpgw_phone_user_request_t *value = OBJ_VALUE(obj, ucp_hpgw_phone_user_request_t *);
				int plen = sizeof(*value), i;
				
				if (is_obj_less_than_len(obj, plen)) {
					return false;
				}

				value->phone_number = ntoh_ll(value->phone_number);

				log_debug("	add or modify user[%s] number %"PRIu64"\n", value->name, value->phone_number);

				for (i = 0; i < hpgw_info->phone_user_num; i++) {
					if (hpgw_info->users[i].phome_number == value->phone_number) {
						log_debug("found same phone_number, modify\n");
						break;
					}
				}

				if (i >= ARRAY_SIZE(hpgw_info->users)) {
					log_err(false, "no buffer left to add user\n");
					return false;
				}

				if (i >= hpgw_info->phone_user_num) {
					hpgw_info->phone_user_num++;
				}

				hpgw_info->users[i].phome_number = value->phone_number;
				memcpy(hpgw_info->users[i].name, value->name, sizeof(hpgw_info->users[i].name));

				log_debug(" user[%s] number %"PRIu64"\n", hpgw_info->users[i].name, hpgw_info->users[i].phome_number);
			}
			break;

		case UCAT_IA_HPGW_DELSUER:
			{
				
				ucp_hpgw_phone_user_del_t *value = OBJ_VALUE(obj, ucp_hpgw_phone_user_del_t *);
				int plen = sizeof(*value), i;
				
				if (is_obj_less_than_len(obj, plen)) {
					return false;
				}

				value->phone_number = ntoh_ll(value->phone_number);

				log_debug(" del user[%s] number %u\n", value->name, value->phone_number);

				for (i = 0; i < hpgw_info->phone_user_num; i++) {
					if (hpgw_info->users[i].phome_number == value->phone_number) {
						log_debug("	found idx %d has phone_user, del\n", i);
						// 去掉i位置的数据
						memcpy(&hpgw_info->users[i], &hpgw_info->users[i + 1], hpgw_info->phone_user_num - i - 1);
						hpgw_info->phone_user_num--;
						break;
					}
				}
			}

		case UCAT_IA_HPGW_LAMP:
			hpgw_update_lamp_stat(hpgw_info, OBJ_VALUE(obj, u_int8_t *), obj->param_len);
			break;
		default:
			break;
    }

	return true;
}

static bool rfgw_onekey_proc(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	cln_common_info_t *info;
	cl_rfdev_onekey_ctrl_t *request;
	int dest_pkt_len;
	char buf[1024] = {0};
	ucp_obj_t* uo = NULL;
	u_int8_t *pdata = NULL;
	
	memset(buf, 0, sizeof(buf));	
	uo = (ucp_obj_t*)buf;
	info = (cln_common_info_t *)&pkt->data[0];

	request = cci_pointer_data(info);
	dest_pkt_len = 2;	

	switch (request->type) {
		case OKT_SMART_ALARM:
			{
				cl_rfdev_onekey_smart_alarm_t *ctrl = (cl_rfdev_onekey_smart_alarm_t *)&request->ctrl;
				ctrl->gw_sn = ntoh_ll(ctrl->gw_sn);

				dest_pkt_len += sizeof(*ctrl);
			}
			break;
		case OKT_SET_DEFENSE:
			{
				cl_rfdev_onekey_set_defense_t *ctrl = (cl_rfdev_onekey_set_defense_t *)&request->ctrl;
				ctrl->pad = 0xff;

				dest_pkt_len += sizeof(*ctrl);
			}
			break;
		case OKT_ALARM_MODE:
			{
				cl_rfdev_onekey_set_alarm_mode_t *ctrl = (cl_rfdev_onekey_set_alarm_mode_t *)&request->ctrl;
				ctrl->off_time = ntohs(ctrl->off_time);
				ctrl->total_time = ntohs(ctrl->total_time);

				dest_pkt_len += sizeof(*ctrl);
			}
			break;
		case OKT_ONOFF:
			{
				cl_rfdev_onekey_set_onoff_t *ctrl = (cl_rfdev_onekey_set_onoff_t *)&request->ctrl;

				dest_pkt_len += sizeof(*ctrl);
			}
			break;

		default:
			*ret = RS_INVALID_PARAM;
			return RS_ERROR;
	}
		
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_ONEKEY, dest_pkt_len);
	memcpy(uo + 1, request, dest_pkt_len);
    sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, buf, sizeof(*uo) + dest_pkt_len);


	return true;
}


/*
	处理设备端过来的通知
*/
bool rfgw_update_ia_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;

	if (air_ctrl == NULL || obj == NULL)
		return ret;

	//log_debug("rfgw_update_ia_data attr %u\n", obj->attr);
	
	switch (obj->attr) {
	case UCAT_IA_RFGW_JOIN_FIND:
		//ret = eb_update_work(air_ctrl, action, obj);
		ret = do_rfgw_join_find(air_ctrl, action, obj);
		break;

	case UCAT_IA_RFGW_LIST:
		ret = do_rfgw_list(air_ctrl, action, obj, false);
		break;

	case UCAT_IA_RFGW_TT_CACHE:
		ret = do_rfgw_tt_cache(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_TT:
		ret = do_rfgw_tt(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_DEV_NAME:
		ret = do_rfgw_dev_name(air_ctrl, action, obj);
		break;

	case UCAT_IA_RFGW_DEV_GROUP:
		ret = do_rfgw_dev_group(air_ctrl, action, obj);
		break;

	case UCAT_IA_RFGW_DEV_GROUP_MEMBER:
		ret = do_rfgw_dev_group_member(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_PARAM:
		ret = do_rfgw_dev_param(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_LIST_DIGEST:
		ret = do_rfgw_dev_list_digest(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_LIST_WITH_ID:
		ret = do_rfgw_list_with_id(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_BIGDATA_CMD:
		ret = do_rfgw_big_data_proc(air_ctrl, action, obj);
		break;
	case UCAT_IA_RFGW_IMG:
		ret = do_rfgw_img_proc(air_ctrl, action, obj);
		break;
	default:
		break;
	}
	
	return ret;
}

static void rfgw_join(user_t *user, u_int16_t timeout)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_join_t* w = (net_rfgw_join_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_JOIN, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	w->timeout = ntohs(timeout);
	
	sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+sizeof(*w));
}

static void rfgw_query_devlist(user_t *user)
{
	smart_air_ctrl_t* ac = NULL;
	ucp_obj_t objs[] = {
		{UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP, 0},
		{UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_PARAM, 0},
		};
	ucp_obj_t objs_list[] = {
		{UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_LIST, 0},
		};
	ucp_obj_t objs_list_cache[] = {
		{UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_LIST_DIGEST, 0},
		};

	if (user->smart_appliance_ctrl) {
		ac = ((smart_appliance_ctrl_t *)user->smart_appliance_ctrl)->sub_ctrl;
	}
	
    //log_debug("login sucessed! query dev list!\n");
	sa_query_objects(user->uc_session, &objs[0], (int)ARRAY_SIZE(objs));
	if (ac && ac->is_support_rf_cache) {
		//log_debug("ac->is_support_rf_cache query UCAT_IA_RFGW_LIST_DIGEST\n");
		sa_query_objects(user->uc_session, &objs_list_cache[0], (int)ARRAY_SIZE(objs_list_cache));
		//rf_slave_init(user);
	} else {
		sa_query_objects(user->uc_session, &objs_list[0], (int)ARRAY_SIZE(objs_list));
	}
}

static void rfgw_join_action(user_t *user, sdk_join_act_t *act)
{
	char buf[64] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_join_action_t* w = (net_rfgw_join_action_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_JOIN_ACTION, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	w->sn = ntoh_ll(act->dev_sn);
	w->action = !!act->accept;
	
	sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+sizeof(*w));
}

static void rfgw_group(user_t *user, sdk_group_t *g)
{
	user_t *gw;
	char buf[256] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_group_t * w = (net_rfgw_group_t*)(uo+1);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_GROUP, sizeof(ucp_obj_t)+sizeof(*w));
	memset(w, 0, sizeof(*w));
	memcpy(w->psk, g->psk, PSK_LEN);
	
	for(w->gw_cnt = 0;w->gw_cnt < g->gw_cnt; ){
		gw = lookup_by_handle(HDLT_USER, g->gw_handle[w->gw_cnt]);
		if(gw == NULL)
			continue;
		w->gw_sn[w->gw_cnt++] = ntoh_ll(gw->sn);
	}
	
	sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+sizeof(*w)+w->gw_cnt*sizeof(u_int64_t));

	
}

static void rfgw_set_tt(user_t *user, u_int8_t *ttd)
{
	smart_air_ctrl_t* ac = NULL;
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_tt_t *w = (net_rfgw_tt_t*)(uo+1);
	net_rfgw_tt_t *up = (net_rfgw_tt_t*)ttd;
	
	if (user->smart_appliance_ctrl) {
		ac = ((smart_appliance_ctrl_t *)user->smart_appliance_ctrl)->sub_ctrl;
	}
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(ucp_obj_t)+sizeof(*w)+up->len);
	w->sn = ntoh_ll(up->sn);
	w->len = ntohs(up->len);
	memcpy(w->data, up->data, up->len);
	//log_mem_dump("rfgw_set_tt", buf, sizeof(ucp_obj_t)+sizeof(*w)+up->len);
	sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+sizeof(*w)+up->len);	
}

static void rfgw_set_tt_batch(user_t *user, batch_sn_handle_t *batch)
{
	u_int8_t rgb, i, n;
	char buf[1024] = {0};
	int len = 0;
	slave_t *slave;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_tt_t *w = (net_rfgw_tt_t*)(uo+1);

	rgb = batch->rgb;
	i = 0; n = 0;
	for(i = 0; i < batch->cnt; i++){
		slave = slave_lookup_by_handle(user, batch->handle[i]);
		if(slave == NULL)
			continue;

		if(slave->dev_info.rfdev->is_ctrl){
			slave->dev_info.rfdev->ctrl_fail++;
		}
		slave->dev_info.rfdev->is_ctrl = 1;
		slave->dev_info.rfdev->ctrl_msec = get_msec();
		uo = (ucp_obj_t*)&buf[len];
		w = (net_rfgw_tt_t*)(uo+1);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*w)+2);
		w->sn = ntoh_ll(slave->sn);
		w->len = ntohs(2);
		w->data[0] = RFTT_WORK_REQ;
		w->data[1] = rgb;
		n++;
		len += (sizeof(*uo)+sizeof(*w)+2);
	}
	if(n > 0)
		sa_set_obj_value_only(user->uc_session, n, buf, len);
}


void rfgw_send_tt_query_packet(slave_t* slave,void* param ,u_int16_t len)
{
    char buf[2048] = {0};
    int total = 0;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_tt_t *w = (net_rfgw_tt_t*)(uo+1);
    
    if (len == 0 || len > 1024) {
        return;
    }
    
    uo = (ucp_obj_t*)&buf[0];
    w = (net_rfgw_tt_t*)(uo+1);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*w)+len);
    
    w->sn = ntoh_ll(slave->sn);
    w->len = ntohs(len);
    
    memcpy(w->data, param, len);
    
    total = sizeof(*uo)+sizeof(*w)+len;
    
    sa_ctrl_obj_value(slave->user->uc_session, UCA_GET, false, 0x1, buf, total);
    
}


void rfgw_send_tt_packet(slave_t* slave,void* param ,u_int16_t len)
{
    char buf[2048] = {0};
    int total = 0;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_tt_t *w = (net_rfgw_tt_t*)(uo+1);
    
    if (len == 0 || len > 1024) {
        return;
    }
    
    uo = (ucp_obj_t*)&buf[0];
    w = (net_rfgw_tt_t*)(uo+1);
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*w)+len);
    
    w->sn = ntoh_ll(slave->sn);
    w->len = ntohs(len);
    
    memcpy(w->data, param, len);
    
    total = sizeof(*uo)+sizeof(*w)+len;
    
    sa_set_obj_value_only(slave->user->uc_session, 0x1, buf, total);
    
}

static u_int8_t calc_rf_big_pkt_crc(u_int8_t* data , int len)
{
    u_int8_t dest = 0;
    int i;
    
    if (!data || len <= 0) {
        return dest;
    }

    for (i = 0; i < len ; i++  ) {
        dest += data[i];
    }
    
    return dest;
}

static void rf_slave_send_big_pkt_ret(slave_t* slave,u_int8_t d_type,u_int8_t seq,u_int8_t ret)
{
    ucp_rf_big_pkt_hdr_t* ph;
    ucp_rf_big_pkt_err_t* pe;
    rf_tlv_t* tlv;
    
    u_int8_t buf[64] = {0};
    
    
    if (!slave ) {
        return ;
    }
    
    tlv = (rf_tlv_t*)buf;
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    pe = (ucp_rf_big_pkt_err_t*)(ph+1);
    
    tlv->type = UP_TLV_BIG_DATA_RET;
    tlv->len = sizeof(*ph)+sizeof(*pe);
    
    ph->d_type = d_type;
    ph->pkt_seq = seq;
    pe->error = ret;
    
    rfgw_send_tt_packet(slave,buf,(u_int16_t)(tlv->len+sizeof(*tlv)));
    
}

static void rf_slve_send_frame(slave_t* slave,rf_b_pkt_send_cache_t* cache,u_int8_t index)
{
    u_int8_t buf[128]={0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_rf_big_pkt_content_t* pc;
    ucp_rf_big_pkt_req_t* pr;
    ucp_rf_big_pkt_hdr_t* ph;
    int len;
    
    if (index >= cache->frame_num) {
        // 是否要发错误包呢
        return;
    }
    
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    pc = (ucp_rf_big_pkt_content_t*)(ph+1);
    pr = (ucp_rf_big_pkt_req_t*)(ph+1);
    
    if (index == cache->frame_num-1) {
        len = cache->pkt_len % cache->byte_per_frame;
        if (len == 0) {
            len = cache->byte_per_frame;
        }
    }else{
        len = cache->byte_per_frame;
    }
    

    tlv->len = (u_int8_t)(len + sizeof(*pc) + sizeof(*ph));
    tlv->type = UP_TLV_BIG_DATA_RESPONSE;
    ph->d_type = cache->data_type;
    ph->pkt_seq = slave->send_seq;
    pc->crc = 0;
    pc->index = index;
    
    memcpy((void*)(pc+1), cache->pkt+cache->byte_per_frame*index, len);
    
    //计算crc;
    pc->crc = calc_rf_big_pkt_crc((u_int8_t *)(pc+1),len);
    rfgw_send_tt_packet(slave,buf,(u_int16_t)(tlv->len+sizeof(*tlv)));

}

static rf_b_pkt_send_cache_t* lookup_send_pkt_by_seq(slave_t* slave, u_int8_t seq)
{
    int i;
    
    for (i = 0; i < MAX_RF_B_PKT_SEND_CACHE_NUM; i++) {
        if (slave->frag_send_cache[i].seq == seq) {
            return &slave->frag_send_cache[i];
        }
    }
    
    return NULL;
}

static rf_b_pkt_defrag_t* lookup_frag_by_seq(slave_t* slave , u_int8_t seq)
{
    int i;
    
    for (i = 0; i < MAX_RF_B_PKT_RECV_CACHE_NUM; i++) {
        if (slave->frag_recv_cache[i].seq == seq) {
            return &slave->frag_recv_cache[i];
        }
    }
    
    return NULL;
}


static int air_send_big_pkt_req_timer(cl_thread_t *t)
{
    slave_t* slave;
    cl_handle_t handle;
    bool need_restart_timer = false;
    int i = 0;
    rf_b_pkt_send_cache_t* cache;
    u_int32_t now = (u_int32_t)time(NULL);
    
    handle = (cl_handle_t)CL_THREAD_ARG(t);
    slave = lookup_by_handle(HDLT_SLAVE, handle);
    if (!slave) {
        return 0;
    }
    
    slave->t_big_pkt_send = NULL;
    
    for (i = 0; i < MAX_RF_B_PKT_SEND_CACHE_NUM; i++) {
        cache = &slave->frag_send_cache[i];
        if (!cache->req_pkt_len || !cache->req_send_time||  (now - cache->req_send_time) > MAX_BIG_PKT_SEND_TIMEOUT ) {
            continue;
        }
        
        if ((now - cache->req_send_time) >= TIME_PER_SECOND*2) {
            rfgw_send_tt_packet(slave,cache->send_req_cache,cache->req_pkt_len);
            cache->has_send_pkt ++;
            if (cache->has_send_pkt >=3 ) {
                cache->req_send_time = 0;
                continue;
            }
        }
        //发送报文
        need_restart_timer = true;
    }
    
    
    if (need_restart_timer) {
        CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_big_pkt_send, air_send_big_pkt_req_timer, (void *)(slave->handle),
                           TIME_PER_SECOND);
    }
    
    return 0;
}

void rf_slave_send_big_pkt(slave_t* slave,u_int8_t data_type,u_int8_t* param ,u_int16_t len)
{
    rf_b_pkt_send_cache_t* cache;
    u_int8_t* pkt;
    ucp_rf_big_pkt_content_t* pc;
    ucp_rf_big_pkt_req_t* pr;
    ucp_rf_big_pkt_hdr_t* ph;
    u_int8_t buf[128]={0};
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    int frame_num ;
    u_int8_t frame_size = RF_B_PKT_FRAME_SIZE , dlen;
    int pkt_len;
    
    //一个包能发完的就算了
    if(!param || len == 0 || !slave){
        return;
    }
    
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    pc = (ucp_rf_big_pkt_content_t*)(ph+1);
    pr = (ucp_rf_big_pkt_req_t*)(ph+1);
    
    //随便分配一个序号
    ++slave->send_seq;
    // 发送大数据请求;
    tlv->type = UP_TLV_BIG_DATA_SEND_REQUEST;
    tlv->len = sizeof(*pr) + sizeof(*ph);
    ph->d_type = data_type;
    ph->pkt_seq = slave->send_seq;
    pr->frame_size = frame_size;
    pr->pkt_len = htons(len);
    
    pkt_len = tlv->len+sizeof(*tlv);
    rfgw_send_tt_packet(slave,buf,pkt_len);
    
    // 计算分片数量
    dlen = len % RF_B_PKT_FRAME_SIZE;
    frame_num = len/RF_B_PKT_FRAME_SIZE;
    if (dlen != 0) {
        frame_num++;
    }else{
        dlen = RF_B_PKT_FRAME_SIZE;
    }
    
    //发送真实数据
//    tlv->type = UP_TLV_BIG_DATA_RESPONSE;
//    for (i = 0; i < frame_num; i++) {
//        if (i == (frame_num - 1)) {
//            pkt_len = dlen;
//        }else{
//            pkt_len = RF_B_PKT_FRAME_SIZE;
//        }
//        
//        pkt_len += (sizeof(*pc) + sizeof(*ph));
//        tlv->len = pkt_len;
//        ph->d_type = data_type;
//        ph->pkt_seq = slave->send_seq;
//        pc->crc = 0;
//        pc->index = i;
//        
//        memcpy((void*)(pc+1), param+i*RF_B_PKT_FRAME_SIZE, pkt_len);
//        
//        //计算crc;
//        pc->crc = calc_rf_big_pkt_crc((u_int8_t *)(pc+1),pkt_len-sizeof(*ph)-sizeof(*pc));
//        pkt_len+=sizeof(*tlv);
//        
//        rfgw_send_tt_packet(slave,buf,pkt_len);
//    }
//    
//    if (len <= RF_B_PKT_FRAME_SIZE) {
//        return;
//    }
    
    // 缓存报文
    pkt = cl_calloc(len, 1);
    if (!pkt) {
        return;
    }
    
    cache = &slave->frag_send_cache[slave->send_pos%MAX_RF_B_PKT_SEND_CACHE_NUM];
    slave->send_pos++;
    SAFE_FREE(cache->pkt);
    memset(cache, 0, sizeof(*cache));
    
    cache->pkt_len = len;
    cache->byte_per_frame = RF_B_PKT_FRAME_SIZE;
    cache->frame_num = frame_num;
    cache->seq = slave->send_seq;
    cache->data_type = data_type;
    
    //缓存发送请求
    if (pkt_len <= sizeof(cache->send_req_cache)) {
        memcpy(cache->send_req_cache, buf, pkt_len);
        cache->req_pkt_len = pkt_len;
        cache->req_send_time = (u_int32_t)time(NULL);
        cache->has_send_pkt = 0x1;
        
        rfgw_send_tt_packet(slave,cache->send_req_cache,cache->req_pkt_len);
        
        CL_THREAD_TIMER_OFF(slave->t_big_pkt_send);
        CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_big_pkt_send, air_send_big_pkt_req_timer, (void *)(slave->handle),
                           TIME_PER_SECOND);
    }
    
    memcpy(pkt, param, len);
    cache->pkt = pkt;
    
    
    
}

/**************************************************大报文处理**************************************************************************/
static u_int8_t bmp_checksum(u_int8_t *pdata, u_int8_t len)
{
	int i;
	u_int8_t check_sum = 0;
	
	if (!pdata) {
		return 0;
	}

	for(i = 0; i < len; i++) {
		check_sum += pdata[i];
	}

	return check_sum;
}

void bmp_free(slave_t *slave)
{
	int i;

	if (!slave) {
		return;
	}

	for(i = 0; i < RF_BIG_PKT_IR_MAX; i++) {
		CL_THREAD_OFF(slave->rbpi_proc[i].t_time);
		CL_THREAD_OFF(slave->rbpi_proc[i].t_timeout);
		SAFE_FREE(slave->rbpi_proc[i].send_pkt);
		SAFE_FREE(slave->rbpi_proc[i].recv_pkt);
	}

	for(i = 0; i < BMP_MAX_CACHE; i++) {
		SAFE_FREE(slave->cache[i].pdata);
	}
}

static void bmp_offline_proc(slave_t *slave)
{
	bmp_free(slave);
	memset((void *)slave->rbpi_proc, 0, sizeof(slave->rbpi_proc));
	memset((void *)slave->cache, 0, sizeof(slave->cache));
}

bool is_bmp(rf_tlv_t* tlv)
{
    bool res = false;
    
    switch (tlv->type) {
        case UP_TLV_BIG_DATA_GET:
        case UP_TLV_BIG_DATA_SEND_REQUEST:
		case UP_TLV_BIG_DATA_RESPONSE:
        case UP_TLV_BIG_DATA_RET:
            res = true;
            break;
            
        default:
            break;
    }
    
    return res;
}

bool udp_rf_dev_bmp_tlv_hook(slave_t* slave,rf_tlv_t* tlv, u_int8_t action)
{
    bool res = false;
    u_int8_t status = RBPI_IDLE;
	u_int8_t *pdata = tlv->value;
	
   	// TODO:解析tlv
   	switch(tlv->type) {
	case UP_TLV_BIG_DATA_SEND_REQUEST:
		status = RBPI_REQEST;
		break;
	case UP_TLV_BIG_DATA_GET:
	case UP_TLV_BIG_DATA_RESPONSE:
		status = RBPI_PROC;
		break;
	case UP_TLV_BIG_DATA_RET:
		status = RBPI_RET;
		break;
	default:
		return false;
		break;
	}

	if (bmp_proc[status].proc_pkt) {
		bmp_proc[status].proc_pkt(slave, tlv, action, status);
	}
	
    return true;
}

static u_int8_t* bmp_decompress_ir_code(u_int8_t *pkt, u_int16_t pkt_len, u_int8_t* code_buf,u_int16_t* code_len)
{
    int i,len;
    u_int16_t tmp;
    u_int16_t* ptr = (u_int16_t*)code_buf;
    
    if (!pkt || !pkt_len || !ptr) {
        return NULL;
    }
    
    for (i = 0; i < pkt_len; ) {
        if (!pkt[i]) {
            return NULL;
        }
        
        if ((i+1) < pkt_len && pkt[i+1] != 0) {
            
            tmp = ((u_int16_t)pkt[i])*25;
            if (i < 5) {
                log_info("decompress_ir_code i = %u tmp = %u htons(tmp) = %u\n",i,tmp,htons(tmp));
            }
            i++;
        }else{
       		if (i + 1 == pkt_len) {
				tmp = ((u_int16_t)pkt[i])*25;
				i++;
			} else {
	            tmp = ((u_int16_t)pkt[i] + (((u_int16_t)pkt[i+2])<<8))*25;
	            i+=3;
			}
        }
        //log_debug("bmp_decompress_ir_code i=%d index=%d pkt=%u tmp=%u\n", i, (ptr - code_buf), pkt[i], tmp);
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


static void rbmp_recv_proc(slave_t *slave, u_int8_t data_type, u_int8_t *pdata, u_int16_t data_len)
{
	priv_rf_ir_match_t* mi;
    u_int8_t code_buf[2048],err = 0;
    ucp_obj_t* uo;
    u_int8_t* dest;
    u_int16_t len = 0,d_len;
    u_int16_t* num;
	
	switch(data_type) {
	case RF_BIG_PKT_IR_COLLECTION:
		mi = &slave->match_mana.rf_ir_match;    
        log_debug("rbmp_recv_proc recv all packet len %u cur_stat %u\n", data_len, mi->cur_stat);
		if (mi->cur_stat == MS_WK_DEV_WAIT_SING ) {
                
                if (!bmp_decompress_ir_code(pdata, data_len,code_buf,&len) || !len || (len&0x1) != 0) {
					log_debug("bmp_decompress_ir_code failed !!!!!!!!!!!!!!!!!!!\n");
                    mi->error = err = MS_WK_ERR_DEV_TRANS_IR;
                    break;
                }

                
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
                //memdump("recv codebuf", code_buf, len);
                log_debug("rbmp_recv_proc after decompress len = %u num[%u] ntohs num[%u] d_len=%u\n",len,*num,
                          ntohs(*num),d_len);
                
                
                SAFE_FREE(mi->matched_code);
                mi->code_len = 0;
                
                mi->matched_code = (u_int8_t*)uo;
               	mi->code_len = d_len+sizeof(*uo);
                //memdump("rbmp_recv_proc decompress", &uo[1], d_len);
                air_ir_code_match_set_status(slave,MS_WK_SERVER_MATCH);
                air_ir_start_to_server_match(slave);
            }
		break;
	case RF_BIG_PKT_IR_TIMER:
		break;
	default:
		break;
	}

    if (err != 0) {
        event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
        air_ir_code_match_set_status(slave,MS_WK_IDLE);
    }	
}

void rf_air_ir_ac_status_sync(slave_t *slave)
{
	char buf[512];
	rf_tlv_t* tlv = (rf_tlv_t*)buf;
	u_int8_t *pvalue = (u_int8_t *)tlv->value;
	priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;

	tlv->type = UP_TLV_WKAIR_STATUS_SYNC;
	tlv->len = 6;
	pvalue[0] = cache->onoff;
	pvalue[1] = cache->mode;
	pvalue[2] = cache->temp;
	pvalue[3] = cache->wind;
	pvalue[4] = cache->wind_direct;
	pvalue[5] = cache->key_id;

	log_debug("	air sync stat: %u %u %u %u %u %u to dev\n", pvalue[0], pvalue[1], pvalue[2], pvalue[3], pvalue[4], pvalue[5]);

	rfgw_send_tt_packet(slave,buf,sizeof(*tlv)+tlv->len);
}

void rbmp_callback_send(slave_t *slave, rf_big_pkt_proc_t *bmp, u_int8_t errcode)
{
	if (!slave || !bmp) {
		log_debug("rbmp_callback_recv err\n");
		return;
	}

	switch(bmp->data_type) {
	case RF_BIG_PKT_IR_DB:
		if (errcode != BMP_ERR_NONE) {
			rf_air_ir_start_db_timeout(slave);
			log_debug("data_type=%u rbmp_callback err=%u is not ok\n", bmp->data_type, errcode);
		} else {
			rf_air_ir_send_db_ok(slave);
			//成功后同步下空调状态给设备
			rf_air_ir_ac_status_sync(slave);
		}
		break;
	default:
		break;
	}
}

void rbmp_callback_recv(slave_t *slave, rf_big_pkt_proc_t *bmp, u_int8_t errcode)
{
	if (!slave || !bmp) {
		log_debug("rbmp_callback_recv err\n");
		return;
	}

	//判断一下，如果是接收红外数据失败，则直接报匹配失败
	if ((errcode != ERR_NONE) && 
		(bmp->data_type == RF_BIG_PKT_IR_COLLECTION)) {
		rf_air_ir_recv_timeout(slave);
		return;
	}

	if (!bmp->recv_pkt ||
		!bmp->recv_total ||
		(bmp->recv_index != bmp->recv_total)) {
		log_debug("rbmp_callback_recv maybe st is wrong\n");
		//判断一下，如果是接收红外数据失败，则直接报匹配失败
		if ((errcode != ERR_NONE) && 
			(bmp->data_type == RF_BIG_PKT_IR_COLLECTION)) {
			rf_air_ir_recv_timeout(slave);
			return;
		}
		return;
	}
	
	rbmp_recv_proc(slave, bmp->data_type, bmp->recv_pkt, bmp->recv_total);
}

void rbmp_callback(slave_t *slave, u_int8_t data_type, u_int8_t errcode)
{
	rf_big_pkt_proc_t *bmp = NULL;
		
	if (!slave ||
		(data_type >= RF_BIG_PKT_IR_MAX)) {
		log_debug("err data_type=%u\n", data_type);
		return;
	}

	bmp = &slave->rbpi_proc[data_type];

	if (bmp->is_send) {
		rbmp_callback_send(slave, bmp, errcode);
	} else {
		rbmp_callback_recv(slave, bmp, errcode);
	}
}

//状态机
static void bmp_err_report(rf_big_pkt_proc_t *bmp, u_int8_t err)
{
	if (!bmp) {
		return;
	}

	if (bmp->finish) {
		bmp->finish(bmp->slave, bmp->data_type, err);
	}

	bmp_set_status(bmp, RBPI_IDLE);
}

static int bmp_comm_tiemr(cl_thread_t *t)
{
	rf_big_pkt_proc_t *bmp = CL_THREAD_ARG(t);

	bmp->t_time = NULL;
	CL_THREAD_TIMER_ON(MASTER, bmp->t_time, bmp_comm_tiemr, (void *)bmp, TIME_N_SECOND(BMP_TIMER));

	if (bmp_proc[bmp->status].timer) {
		bmp_proc[bmp->status].timer(bmp);
	}

	return 0;	
}

static void bmp_reset_timer(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}

	CL_THREAD_OFF(bmp->t_time);
	CL_THREAD_TIMER_ON(MASTER, bmp->t_time, bmp_comm_tiemr, (void *)bmp, TIME_N_SECOND(BMP_TIMER));
}

static int bmp_comm_tiemr_die(cl_thread_t *t)
{
	rf_big_pkt_proc_t *bmp = CL_THREAD_ARG(t);

	bmp->t_timeout = NULL;

	log_debug("data_type=%u goto die timeout\n", bmp->data_type);

	bmp_err_report(bmp, BMP_ERR_TIMEOUT);
	
	return 0;
}

static void bmp_reset_timer_die(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}

	CL_THREAD_OFF(bmp->t_timeout);
	CL_THREAD_TIMER_ON(MASTER, bmp->t_timeout, bmp_comm_tiemr_die, (void *)bmp, TIME_N_SECOND(BMP_TIMER_DIE));
}

static void bmp_timer_off(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}

	CL_THREAD_OFF(bmp->t_time);
	CL_THREAD_OFF(bmp->t_timeout);	
}

static void bmp_all_clean(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}

	CL_THREAD_OFF(bmp->t_time);
	CL_THREAD_OFF(bmp->t_timeout);
	SAFE_FREE(bmp->send_pkt);
	SAFE_FREE(bmp->recv_pkt);
	memset((void *)bmp, 0, sizeof(*bmp));
}

//idle
static void bmp_idle_indo(rf_big_pkt_proc_t *bmp)
{
	slave_t *slave = NULL;

	if (!bmp) {
		return;
	}
	slave = bmp->slave;
	bmp_all_clean(bmp);

	//看看缓存还有没有
	bmp_do_next(slave);
}

//req
static void bmp_req_send(rf_big_pkt_proc_t *bmp, bool is_send)
{
	u_int8_t buf[128];
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	ucp_wk_air_req_t *req = (ucp_wk_air_req_t *)&tlv[1];

	tlv->type = UP_TLV_BIG_DATA_SEND_REQUEST;
	if (is_send) {
		tlv->len = 4;
		req->action = BMP_ACTION_SET;
		req->sub_type = bmp->data_type;
		req->total_len = htons(bmp->send_total);
		log_debug("bmp_req_send bmp->send_total=%u\n", bmp->send_total);
	} else {
		tlv->len = 2;
		req->action = BMP_ACTION_GET;
		req->sub_type = bmp->data_type;
	}

	rfgw_send_tt_packet(bmp->slave, buf, (sizeof(*tlv) + tlv->len));
}

static void bmp_req_indo(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}
	
	bmp_timer_off(bmp);
	//timer...
	bmp_reset_timer(bmp);
	//retset timerdie
	bmp_reset_timer_die(bmp);

	bmp_req_send(bmp, bmp->is_send);
}

static void bmp_req_proc(slave_t* slave, rf_tlv_t* tlv, u_int8_t action, u_int8_t status)
{
	rf_big_pkt_proc_t *bmp = NULL;
	ucp_wk_air_req_r_t *req = NULL;
	
	if (!slave || !tlv) {
		return;
	}

	if (tlv->len < sizeof(*req)) {
		log_err(false, "bmp_req_proc err tlvlen=%u\n", tlv->len);
		return;
	}

	req = (ucp_wk_air_req_r_t *)&tlv[1];
	if (req->sub_type >= RF_BIG_PKT_IR_MAX) {
		log_err(false, "err subtype=%u\n", req->sub_type);
		return;
	}
	
	bmp = &slave->rbpi_proc[req->sub_type];
	if (bmp->status != status) {
		log_debug("bmp_req_proc status is not match %u %u\n", bmp->status, status);
		return;
	}

	log_debug("bmp_req_proc action=%u sub_type=%u ret=%u total_len=%u\n", 
		req->action, req->sub_type, req->ret, htons(req->total_len));
	//不允许传送
	if (req->ret) {
		bmp_err_report(bmp, (req->action == BMP_ACTION_GET)?BMP_ERR_NO_RECV:BMP_ERR_NO_SEND);
		return;
	}

	req->total_len = htons(req->total_len);
	if (req->action == BMP_ACTION_GET && req->total_len == 0) {
		log_debug("req->total_len == 0\n");
		return;
	}

	if (req->action == BMP_ACTION_GET) {
		log_debug("bmp_req_proc need goto get total_len=%u\n", req->total_len);
		bmp->recv_index = 0;
		SAFE_FREE(bmp->recv_pkt);
		bmp->recv_pkt = cl_calloc(req->total_len, 1);
		if (!bmp->recv_pkt) {
			bmp_err_report(bmp, BMP_ERR_MALLOC_FAILED);
			return;
		}
		bmp->recv_total = req->total_len;
	}

	bmp_set_status(bmp, RBPI_PROC);
}

static void bmp_req_timer(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}

	bmp_req_send(bmp, bmp->is_send);
}

//rw
static void bmp_rw_send(rf_big_pkt_proc_t *bmp, bool is_send)
{
	u_int8_t buf[128];
	u_int8_t send_len = 0;
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	ucp_wk_air_rw_t *prw = (ucp_wk_air_rw_t *)&tlv[1];

	if (!is_send) {
		tlv->type = UP_TLV_BIG_DATA_GET;
		tlv->len = 4;
		prw->sub_type = bmp->data_type;
		prw->offset = htons(bmp->recv_index);
		prw->rw_len = BMP_RW_DATA_LEN;
		log_debug("bmp_rw_send req read index=%u\n", bmp->recv_index);
		rfgw_send_tt_packet(bmp->slave, buf, (tlv->len+ sizeof(*tlv)));
		return;
	}

	if (!bmp->send_pkt) {
		log_err(false, "bmp_rw_send bmp->send_pkt is null\n");
		return;
	}

	if (bmp->send_index + BMP_RW_DATA_LEN <= bmp->send_total) {
		send_len= BMP_RW_DATA_LEN;
	} else {
		send_len = bmp->send_total - bmp->send_index;
	}
	bmp->cur_send_len = send_len;
	
	tlv->type = UP_TLV_BIG_DATA_RESPONSE;
	tlv->len = send_len + sizeof(*prw);
	prw->sub_type = bmp->data_type;
	prw->offset = htons(bmp->send_index);
	prw->rw_len = send_len;
	memcpy(prw->data, &bmp->send_pkt[bmp->send_index], send_len);
	//memdump("bmp_rw_send", prw->data, send_len);
	prw->checksum = bmp_checksum(prw->data, send_len);
	log_debug("bmp_rw_send rea send index=%u checksum=%u\n", bmp->send_index, prw->checksum);
	rfgw_send_tt_packet(bmp->slave, buf, (tlv->len+ sizeof(*tlv)));
}

static void bmp_rw_indo(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}
	
	bmp_timer_off(bmp);
	//timer...
	bmp_reset_timer(bmp);
	//retset timerdie
	bmp_reset_timer_die(bmp);

	bmp_rw_send(bmp, bmp->is_send);
}

static void bmp_rw_proc(slave_t* slave, rf_tlv_t* tlv,u_int8_t action, u_int8_t status)
{
	rf_big_pkt_proc_t *bmp = NULL;
	ucp_wk_air_rw_r_t *rw = NULL;
	u_int8_t check_sum = 0;
	u_int8_t data_len = 0;
	
	if (!slave || !tlv) {
		return;
	}

	rw = (ucp_wk_air_rw_r_t *)&tlv[1];
	if (rw->sub_type >= RF_BIG_PKT_IR_MAX) {
		log_err(false, "bmp_rw_proc err subtype=%u\n", rw->sub_type);
		return;
	}
	
	bmp = &slave->rbpi_proc[rw->sub_type];
	if (bmp->status != status) {
		log_debug("bmp_rw_proc status is not match status_local=%u status_get=%u rw->sub_type=%u\n", 
			bmp->status, status, rw->sub_type);
		return;
	}

	//是否操作失败
	if (rw->ret) {
		log_debug("bmp_rw_proc ctrl failed ret=1 is_send=%u\n", bmp->is_send);
		bmp_err_report(bmp, (tlv->type == UP_TLV_BIG_DATA_RESPONSE)?BMP_ERR_SEND_FAILED:BMP_ERR_RECV_FAILED);
		return;
	}

	if (tlv->type == UP_TLV_BIG_DATA_RESPONSE) {
		bmp->send_index += bmp->cur_send_len;
		if (bmp->send_index >= bmp->send_total) {
			log_debug("bmp_rw_proc send over total=%u\n", bmp->send_total);
			bmp_set_status(bmp, RBPI_RET);
			return;
		}
		bmp_rw_send(bmp, bmp->is_send);
		return;
	}

	if (!bmp->recv_pkt) {
		log_debug("bmp_rw_proc recv_pkt is nuLL\n");
		return;
	}

	if (tlv->len < sizeof(*rw)) {
		log_debug("err len =%u\n", tlv->len);
		return;
	}
	
	rw->offset = htons(rw->offset);
	log_debug("bmp_rw_proc read sub=%u offset=%u checksum=%u tlvlen=%u\n", 
		rw->sub_type, rw->offset, rw->check_sum, tlv->len);
	data_len = tlv->len - sizeof(*rw);
	check_sum = bmp_checksum(rw->data, data_len);
	log_debug("bmp_rw_proc checksum %u %u and data_len=%u tlv->len=%u sizeof(rw)=%u\n", 
		check_sum, rw->check_sum, data_len, tlv->len, sizeof(*rw));
	if (check_sum != rw->check_sum) {
		log_debug("bmp_rw_proc check_sum err\n");
		return;
	}

	log_debug("bmp_rw_proc recv_index=%u data_len=%u recv_total=%u data_type=%u\n", 
		bmp->recv_index, data_len, bmp->recv_total, bmp->data_type);
	if (bmp->recv_index + data_len > bmp->recv_total) {
		log_debug("bmp_rw_proc err len\n");
		return;
	}
	memcpy(&bmp->recv_pkt[bmp->recv_index], rw->data, data_len);
	bmp->recv_index += data_len;
	if (bmp->recv_index >= bmp->recv_total) {
		log_debug("%s recv over total=%u\n", __FUNCTION__, bmp->recv_total);
		bmp_set_status(bmp, RBPI_RET);
		return;
	}
	bmp_rw_send(bmp, bmp->is_send);

	//读写耗费时间比较长，成功一个读写阶段后可以重置定时器
	bmp_reset_timer(bmp);
	bmp_reset_timer_die(bmp);
}

static void bmp_rw_timer(rf_big_pkt_proc_t *bmp)
{
	bmp_rw_send(bmp, bmp->is_send);
}

//ret
static void bmp_ret_send(rf_big_pkt_proc_t *bmp, bool is_send)
{
	u_int8_t buf[128];
	rf_tlv_t *tlv = (rf_tlv_t *)buf;
	ucp_wk_air_ret_t *pret = (ucp_wk_air_ret_t *)&tlv[1];

	tlv->type = UP_TLV_BIG_DATA_RET;
	tlv->len = 3;
	pret->action = is_send?BMP_ACTION_SET:BMP_ACTION_GET;
	pret->sub_type = bmp->data_type;
	pret->checksum = bmp_checksum(bmp->send_pkt, (u_int8_t)bmp->send_total);

	rfgw_send_tt_packet(bmp->slave, buf, (tlv->len+ sizeof(*tlv)));
}

static void bmp_ret_indo(rf_big_pkt_proc_t *bmp)
{
	if (!bmp) {
		return;
	}
	
	bmp_timer_off(bmp);
	//timer...
	bmp_reset_timer(bmp);
	//retset timerdie
	bmp_reset_timer_die(bmp);

	bmp_ret_send(bmp, bmp->is_send);
}

static void bmp_ret_proc(slave_t* slave, rf_tlv_t* tlv,u_int8_t action, u_int8_t status)
{
	rf_big_pkt_proc_t *bmp = NULL;
	ucp_wk_air_ret_r_t *ret = NULL;
	
	if (!slave || !tlv) {
		return;
	}

	ret = (ucp_wk_air_ret_r_t *)&tlv[1];
	if (ret->sub_type >= RF_BIG_PKT_IR_MAX) {
		log_err(false, "err subtype=%u\n", ret->sub_type);
		return;
	}
	
	bmp = &slave->rbpi_proc[ret->sub_type];
	log_debug("bmp_ret_proc sub_type=%u ret=%u\n", ret->sub_type, ret->ret);
	if (bmp->status != status) {
		log_debug("%s status is not match %u %u\n", __FUNCTION__, bmp->status, status);
		return;
	}
	
	//是否操作失败
	if (ret->ret) {
		bmp_err_report(bmp, (ret->action == BMP_ACTION_SET)?BMP_ERR_SEND_FAILED:BMP_ERR_RECV_FAILED);
		return;
	}
	
	bmp_err_report(bmp, BMP_ERR_NONE);
}

static void bmp_ret_timer(rf_big_pkt_proc_t *bmp)
{
	bmp_ret_send(bmp, bmp->is_send);
}

bmp_proc_t bmp_proc[RBPI_MAX] = {
	{"BMP_IDLE", bmp_idle_indo, NULL, NULL, NULL},
	{"BMP_REQ", bmp_req_indo, NULL, (bmp_func_t)bmp_req_proc, bmp_req_timer},
	{"BMP_RW", bmp_rw_indo, NULL, (bmp_func_t)bmp_rw_proc, bmp_rw_timer},
	{"BMP_RET", bmp_ret_indo, NULL, (bmp_func_t)bmp_ret_proc, bmp_ret_timer},
};

void bmp_set_status(rf_big_pkt_proc_t *bmpip, u_int8_t status)
{
	u_int8_t pre_status = 0;
	
	if (!bmpip ||
		(status > RBPI_RET)) {
		return;
	}

	pre_status = bmpip->status;
	log_info("data_type=%u  bmp_set_status modify status from %u to %u\n", bmpip->data_type, pre_status, status);
	bmpip->status = status;
	
	if (bmp_proc[pre_status].on_out) {
		bmp_proc[pre_status].on_out(bmpip);
	}
	
	if (bmp_proc[pre_status].on_into) {
		bmp_proc[status].on_into(bmpip);
	}
}

bool rbpi_type_valid(u_int8_t data_type)
{
	if (data_type >= RF_BIG_PKT_IR_MAX) {
		return false;
	}
	
	return true;
}

bool rbmp_busy(slave_t *slave)
{
	int i;
	
	if (!slave) {
		return false;
	}
	
	for(i = 0; i < RF_BIG_PKT_IR_MAX; i++) {
		if (slave->rbpi_proc[i].status != RBPI_IDLE) {
			return true;
		}
	}

	return false;
}

void bmp_slave_copy(slave_t *slave, slave_t *src)
{
	int i;

	//slave info
	memcpy((void *)slave->rbpi_proc, (void *)src->rbpi_proc, sizeof(slave->rbpi_proc));
	memset((void *)src->rbpi_proc, 0, sizeof(src->rbpi_proc));
	for(i = 0; i < RF_BIG_PKT_IR_MAX; i++) {
		slave->rbpi_proc[i].slave = slave;
	}

	//cache
	memcpy((void *)slave->cache, (void *)src->cache, sizeof(slave->cache));
	memset((void *)src->cache, 0, sizeof(slave->cache));
	for(i = 0; i < BMP_MAX_CACHE; i++) {
		slave->cache[i].slave = slave;
	}
}

static bool bmp_busy_list_add(slave_t* slave,u_int8_t data_type,u_int8_t* param ,u_int16_t len, rbpi_callback finish_func, bool is_send)
{
	int i;
	rbmp_cache_t *bc = NULL;

	log_debug("enter bmp_busy_list_add\n");
	if (!rbpi_type_valid(data_type)) {
		return false;
	}
	
	if (!slave ||
		(is_send && (!param || (len == 0)))) {
		return false;
	}

	//get idle one
	for(i = 0; i < BMP_MAX_CACHE; i++) {
		bc = &slave->cache[i];
		if (!bc->busy) {
			break;
		}
	}
	if (i == BMP_MAX_CACHE) {
		return false;
	}

	SAFE_FREE(bc->pdata);
	bc->slave = slave;
	bc->data_type = data_type;
	bc->is_send = is_send;
	bc->finish = finish_func;
	bc->len = len;
	if (is_send) {
		bc->pdata = cl_calloc(len, 1);
		if (!bc->pdata) {
			return false;
		}
		memcpy(bc->pdata, param, len);
	}
	bc->busy = true;

	return true;
}

static void bmp_do_next(slave_t* slave)
{
	int i;
	rbmp_cache_t *bc = NULL;
	rf_big_pkt_proc_t *bmpip = NULL;

	if (!slave) {
		log_debug("bmp_do_next err slave is null\n");
		return;
	}
	//找到有值的
	for(i = 0; i < BMP_MAX_CACHE; i++) {
		bc = &slave->cache[i];
		if (!bc->busy) {
			continue;
		}
		if (!rbpi_type_valid(bc->data_type)) {
			memset((void *)bc, 0, sizeof(*bc));
			continue;
		}
		if (bc->is_send && ((bc->len == 0) || !bc->pdata)) {
			memset((void *)bc, 0, sizeof(*bc));
			continue;
		}
		break;
	}
	if (i == BMP_MAX_CACHE) {
		return;
	}

	bmpip = &slave->rbpi_proc[bc->data_type];
	bmpip->is_send = bc->is_send;
	bmpip->data_type = bc->data_type;
	bmpip->slave = slave;
	bmpip->finish = bc->finish;
	if (bc->is_send) {
		bmpip->send_index = 0;
		bmpip->send_total = bc->len;
		SAFE_FREE(bmpip->send_pkt);
		bmpip->send_pkt = bc->pdata;
	} else {
		bmpip->recv_index = 0;
		bmpip->recv_total = 0;
		SAFE_FREE(bmpip->recv_pkt);
	}
	
	memset((void *)bc, 0, sizeof(*bc));
	//开启状态机
	bmp_set_status(bmpip, RBPI_REQEST);
}

bool rf_slave_send_big_pkt_v2(slave_t* slave,u_int8_t data_type,u_int8_t* param ,u_int16_t len, rbpi_callback finish_func)
{
	rf_big_pkt_proc_t *bmpip = NULL;
	u_int8_t *pdata = NULL;
	
	if (!slave ||
		!param ||
		(len == 0)) {
		return false;
	}

	if (!rbpi_type_valid(data_type)) {
		return false;
	}
	
	if (rbmp_busy(slave)) {
		return bmp_busy_list_add(slave, data_type, param, len, finish_func, true);
	}
	
	pdata = cl_calloc(len, 1);
	if (!pdata) {
		return false;
	}

	memcpy(pdata, param, len);
	bmpip = &slave->rbpi_proc[data_type];
	bmpip->is_send = true;
	bmpip->data_type = data_type;
	bmpip->slave = slave;
	bmpip->finish = finish_func;
	bmpip->send_index = 0;
	bmpip->send_total = len;
	SAFE_FREE(bmpip->send_pkt);
	bmpip->send_pkt = pdata;
	//开启状态机
	bmp_set_status(bmpip, RBPI_REQEST);

	return true;
}

bool rf_slave_recv_big_pkt_v2(slave_t* slave, u_int8_t data_type, rbpi_callback finish_func)
{
	rf_big_pkt_proc_t *bmpip = NULL;
	
	if (!slave) {
		return false;
	}

	if (!rbpi_type_valid(data_type)) {
		return false;
	}
	
	if (rbmp_busy(slave)) {
		return bmp_busy_list_add(slave, data_type, NULL, 0, finish_func, false);
	}

	bmpip = &slave->rbpi_proc[data_type];
	bmpip->is_send = false;
	bmpip->data_type = data_type;
	bmpip->slave = slave;
	bmpip->finish = finish_func;
	bmpip->recv_index = 0;
	bmpip->recv_total = 0;
	SAFE_FREE(bmpip->recv_pkt);
	
	bmp_set_status(bmpip, RBPI_REQEST);
	
	return true;
}

/*********************************************************************************************************************************/

static void add_frag_to_cache(rf_b_pkt_defrag_t* dest,ucp_rf_big_pkt_hdr_t* ph,ucp_rf_big_pkt_content_t* pc,u_int8_t t_len)
{
    u_int8_t crc;
    
    if (!dest || !ph || !pc || !t_len) {
        return;
    }
    
    //判断集片缓存是否有效,数据包是否已经收到过,
    if ( !dest->mask || !dest->pkt || pc->index >= dest->frame_num || dest->mask[pc->index]!=0 ||
        dest->data_type != ph->d_type || dest->byte_per_frame < t_len) {
        return;
    }
    
    crc = calc_rf_big_pkt_crc((u_int8_t *)(pc+1), t_len);
    if (crc != pc->crc) {//校验错误
        return;
    }
    
    memcpy(&dest->pkt[pc->index*RF_B_PKT_FRAME_SIZE], pc+1, t_len);
    dest->mask[pc->index] = true;
    dest->recv_frame++;
}

static void rf_slave_proc_big_pkt_send_fail(slave_t* slave,ucp_rf_big_pkt_hdr_t* ph,ucp_rf_big_pkt_err_t* pe)
{
    if (!ph || !pe || !pe->error) {
        return;
    }
    
    switch (ph->d_type) {
        //手机传数据给设备失败
        case BD_IR_CT_IR_DOWNLOAD:
            if(slave->match_mana.rf_ir_match.cur_stat != MS_WK_IDLE){
                air_ir_code_match_set_status(slave, MS_WK_IDLE);
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_IR_TO_DEVICE_TIMEOUT;
                event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
            }
            break;
        //控制失败
        case BD_IR_CT_IR_CTRL:
            break;
            
        default:
            break;
    }
}

void rf_slave_request_remain_big_pkt(slave_t* slave,rf_b_pkt_defrag_t* frag)
{
    int i;
    u_int8_t buf[64] = {0};
    ucp_rf_big_pkt_hdr_t* ph;
    u_int8_t* pIndex;
    
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    pIndex = (u_int8_t*)(ph+1);
    
    if (frag->recv_frame >= frag->frame_num){
        return;
    }
    
    memset(buf, 0, sizeof(buf));
    
    tlv->type = UP_TLV_BIG_DATA_GET;
    ph->d_type = frag->data_type;
    ph->pkt_seq = frag->seq;
    tlv->len = sizeof(*ph);
    
    for (i = 0; i < frag->frame_num; i++) {
        if (!frag->mask[i]) {
            *pIndex++ = (u_int8_t)i;
            tlv->len++;
            
            if (tlv->len >= RF_B_PKT_FRAME_SIZE ) {
                rfgw_send_tt_packet(slave,buf,(u_int16_t)(tlv->len+sizeof(*tlv)));
                
                tlv->type = UP_TLV_BIG_DATA_GET;
                ph->d_type = frag->data_type;
                ph->pkt_seq = frag->seq;
                tlv->len = sizeof(*ph);
                
                pIndex = (u_int8_t*)(ph+1);
                
            }
        }
    }
    
    if (tlv->len > sizeof(*ph) ) {
        rfgw_send_tt_packet(slave,buf,(u_int16_t)(tlv->len+sizeof(*tlv)));
    }
    
}

bool rf_slave_do_big_pkt_ret(slave_t* slave,rf_tlv_t* tlv)
{
    ucp_rf_big_pkt_hdr_t* ph;
    ucp_rf_big_pkt_err_t* pe;
    rf_b_pkt_send_cache_t* cache;
    rf_b_pkt_defrag_t* frag;
    int i;
    
    if (!slave || !tlv) {
        return false;
    }
    
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    pe = (ucp_rf_big_pkt_err_t*)(ph+1);
    cache = lookup_send_pkt_by_seq(slave,ph->pkt_seq);
    
    switch (pe->error) {
        case BP_ERR_NONE:
            //收到了响应，发送剩余报文,可以统一出来
            if (cache != NULL) {
                cache->req_send_time = 0;
            }
            
            // 发送所有报文
            if (cache->frame_num > 0) {
                for (i = 0 ; i < cache->frame_num; i++) {
                    rf_slve_send_frame(slave,cache,i);
                }

				#if 0
                for (i = 0 ; i < 2 ; i++) {
                    rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_SEND_PKT_OK);
                }
				#endif
            }
            
            break;
        case BP_ERR_DEV_BUSY:
        case BP_ERR_PKT_TOO_BIG:
            //出错了，不再发送请求，一个序列报文只处理一次
            if (cache != NULL && cache->req_send_time != 0) {
                cache->req_send_time = 0;
                //处理不同数据类型的错误
                rf_slave_proc_big_pkt_send_fail(slave,ph,pe);
            }
            break;
        case BP_RECV_PKT_OK:
            break;
        case BP_SEND_PKT_OK:
			log_debug("do send bit pkt ok seq %u\n", ph->pkt_seq);
            frag = lookup_frag_by_seq(slave, ph->pkt_seq);
            if (frag != NULL) {
                if (frag->recv_frame == frag->frame_num) {
                    rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_RECV_PKT_OK);
                }else if(frag->mask != NULL){
                    rf_slave_request_remain_big_pkt(slave,frag);
                }
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

rf_b_pkt_defrag_t* rf_slave_recv_big_pkt(slave_t* slave, rf_tlv_t* tlv)
{
    rf_b_pkt_defrag_t* dest = NULL;
    ucp_rf_big_pkt_content_t* pc;
    ucp_rf_big_pkt_req_t* pr;
    ucp_rf_big_pkt_hdr_t* ph;
    u_int8_t frame_num;
    
    if (!slave || !tlv) {
        return NULL;
    }
    
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    pc = (ucp_rf_big_pkt_content_t*)(ph+1);
    pr = (ucp_rf_big_pkt_req_t*)(ph+1);
    
    
    
    switch (tlv->type) {
        case UP_TLV_BIG_DATA_SEND_REQUEST:
            dest = lookup_frag_by_seq(slave,ph->pkt_seq); // 如果找到，清空它
            if (!dest) {
                //没找到，分配一个最久没使用的
                dest = &slave->frag_recv_cache[slave->recv_pos % MAX_RF_B_PKT_RECV_CACHE_NUM];
                slave->recv_pos++;
            }else{
                if (dest->pkt_len == ntohs(pr->pkt_len)) {
                    rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_ERR_NONE);
                    return NULL;
                }
            }
            
            
            
            SAFE_FREE(dest->pkt);
            SAFE_FREE(dest->mask);
            memset(dest, 0, sizeof(*dest));
            
            pr->pkt_len = ntohs(pr->pkt_len);
            frame_num = pr->pkt_len/pr->frame_size;
            
            if ((pr->pkt_len % pr->frame_size) != 0) {
                frame_num++;
            }
            
            log_debug("recv packet request pkt len[%u] frame_size[%u] frame_num %u\n",pr->pkt_len,pr->frame_size,frame_num);
            
            dest->mask = cl_calloc(sizeof(u_int8_t), frame_num);
            if (!dest->mask) {
                rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_ERR_PKT_TOO_BIG);
                return NULL;
            }
            
            dest->pkt = cl_calloc(sizeof(u_int8_t), pr->pkt_len);
            if (!dest->pkt) {
                rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_ERR_PKT_TOO_BIG);
                return NULL;
            }
            
            dest->frame_num = frame_num;
            dest->pkt_len = pr->pkt_len;
            dest->byte_per_frame = pr->frame_size;
            dest->seq = ph->pkt_seq;
            dest->data_type = ph->d_type;
            
            rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_ERR_NONE);
            
            break;
        case UP_TLV_BIG_DATA_RESPONSE:
            //数据是否有效
            log_debug("recv packet seq[%u] index [%u]\n",ph->pkt_seq,pc->index);
            if (tlv->len <= sizeof(*pc) + sizeof(*ph)) {
				log_err(false, "tlv len %u invalid\n", tlv->len);
                break;
            }
            
            //查找序号是否有效
            dest = lookup_frag_by_seq(slave,ph->pkt_seq);
            if (!dest) {
				log_err(false, "lookup_frag_by_seq failed\n");
                break;
            }
            
            add_frag_to_cache(dest,ph,pc,tlv->len - sizeof(*pc) - sizeof(*ph));
            
            break;
            
        default:
            return NULL;
            break;
    }

	if (dest) {
		log_debug("dest %p: dest->recv_frame %u dest->frame_num %u dest->is_proc_pkt %u\n", 
			dest, dest->recv_frame, dest->frame_num, dest->is_proc_pkt);
	}
    
    if (dest && (dest->recv_frame == dest->frame_num)  && (!dest->is_proc_pkt)) {
        rf_slave_send_big_pkt_ret(slave,ph->d_type,ph->pkt_seq,BP_RECV_PKT_OK);
        return dest;
    }
    
    return NULL;
}

void rf_slave_proc_frame_get(slave_t* slave, rf_tlv_t* tlv)
{
    rf_b_pkt_send_cache_t* cache;
    ucp_rf_big_pkt_hdr_t* ph;
    u_int8_t* pindex,tmp;
    int num,i;
    
    if (!slave || !tlv || tlv->len <= sizeof(*ph)) {
        return;
    }
    
    if (tlv->type != UP_TLV_BIG_DATA_GET) {
        return;
    }
    
    ph = (ucp_rf_big_pkt_hdr_t*)(tlv+1);
    
    cache = lookup_send_pkt_by_seq(slave,ph->pkt_seq);
    if (!cache || cache->data_type != ph->d_type || cache->seq != ph->pkt_seq) {
        return;
    }
    
    num = (tlv->len - sizeof(*ph))/sizeof(u_int8_t);
    pindex = (u_int8_t*)(ph+1);
    for (i = 0; i < num; i++,pindex++) {
        tmp = *pindex;
        rf_slve_send_frame(slave,cache,tmp);
    }

}

static int air_ir_code_match_timer(cl_thread_t *t)
{
    slave_t* slave;
    cl_handle_t handle;
    u_int8_t cur_stat;
    
    handle = (cl_handle_t)CL_THREAD_ARG(t);
    slave = lookup_by_handle(HDLT_SLAVE, handle);
    if (!slave) {
        return 0;
    }
    
    slave->t_rf_code_match = NULL;
    cur_stat = slave->match_mana.rf_ir_match.cur_stat;
    if (cur_stat != MS_WK_IDLE) {
        switch (cur_stat) {
            case MS_WK_WAIT_DEV_START:
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_WAIT_DEV_RESP;
                break;
            case MS_WK_DEV_WAIT_SING:
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_DEV_WAIT_IR_TIME_OUT;
                break;
            case MS_WK_SERVER_MATCH:
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_SERVER_RESP;
                break;
            case MS_WK_PHONE_DOWN_SING:
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                break;
            case MS_WK_NOTIFY_DEV_CODE_ID:
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_IR_TO_DEVICE_TIMEOUT;
                break;
            default:
                slave->match_mana.rf_ir_match.error = MS_WK_ERR_WAIT_DEV_RESP;
                uasc_remove_wait_pkt_by_ident(cl_priv->uasc_session, slave->sn);
                break;
        }
        air_ir_code_match_set_status(slave, MS_WK_IDLE);
        event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
    }
   
    return 0;
}

static void air_ir_reset_code_match_timer(slave_t* slave,u_int32_t sec_time_out)
{
    CL_THREAD_TIMER_OFF(slave->t_rf_code_match);
    
    if(sec_time_out > 0){
        CL_THREAD_TIMER_ON(&cl_priv->master, slave->t_rf_code_match, air_ir_code_match_timer, (void *)(slave->handle),
                           sec_time_out*TIME_PER_SECOND);
    }
}

static void do_air_ir_start_resend(slave_t *slave)
{
	int len = 0;
	u_int8_t buf[100];
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_air_ir_learn_ctrl_t* irc = (ucp_air_ir_learn_ctrl_t*)(tlv+1);

	irc->action = ACT_START_IR_LEARN;
	irc->ir_num = slave->ir_num;
	irc->timeout = AIR_IR_CODE_LEARN_TIMEOUT;
	tlv->type = UP_TLV_AIR_IR_LEARN_CTRL;
	tlv->len = sizeof(*irc);
	len = tlv->len + sizeof(*tlv);

    rfgw_send_tt_packet(slave,buf,len);
}

static int air_ir_code_match_resend_timer(cl_thread_t *t)
{
    slave_t* slave;
    
    slave = (slave_t *)CL_THREAD_ARG(t);
	if (!slave) {
		return 0;
	}

	log_debug("air_ir_code_match_resend_timer\n");
	slave->t_rf_code_match_resend = NULL;
	CL_THREAD_TIMER_ON(MASTER, slave->t_rf_code_match_resend, air_ir_code_match_resend_timer, (void *)slave,
	                       TIME_N_SECOND(AIR_IR_RESEND_TIME));

	do_air_ir_start_resend(slave);

	return 0;
}

void air_ir_code_match_resend_timer_init(slave_t *slave)
{
	if (!slave) {
		return;
	}

	CL_THREAD_TIMER_OFF(slave->t_rf_code_match_resend);
	CL_THREAD_TIMER_ON(MASTER, slave->t_rf_code_match_resend, air_ir_code_match_resend_timer, (void *)slave,
	                       TIME_N_SECOND(AIR_IR_RESEND_TIME));
}

void air_ir_code_match_resend_timer_exit(slave_t *slave)
{
	if (!slave) {
		return;
	}

	CL_THREAD_TIMER_OFF(slave->t_rf_code_match_resend);
}

void air_ir_code_match_set_status(slave_t* slave,u_int32_t status)
{
    int event  = 0;
    
    if (slave->match_mana.rf_ir_match.cur_stat == status) {
        return;
    }

	air_ir_code_match_resend_timer_exit(slave);
	log_debug("air_ir_code_match_set_status cur_status=%u status=%u ir_num=%u\n", 
		slave->match_mana.rf_ir_match.cur_stat, status, slave->ir_num);
    switch (status) {
        case MS_WK_WAIT_DEV_START:
            SAFE_FREE(slave->match_mana.rf_ir_match.matched_code);
            memset(&slave->match_mana.rf_ir_match, 0, sizeof(slave->match_mana.rf_ir_match));
            air_ir_reset_code_match_timer(slave, 30);
            air_ir_code_match_resend_timer_init(slave);
            break;
        case MS_WK_DEV_WAIT_SING:
            air_ir_reset_code_match_timer(slave, 180);
            break;
        case MS_WK_SERVER_MATCH:
            air_ir_reset_code_match_timer(slave, 10);
            event = SAE_CODE_MATCH_DEV_RECV_CODE;
            break;
        case MS_WK_PHONE_DOWN_SING:
            air_ir_reset_code_match_timer(slave, 20);
            event = SAE_CODE_MATCH_START_DOWNLOAD_CODE;
            break;
        case MS_WK_NOTIFY_DEV_CODE_ID:
            air_ir_reset_code_match_timer(slave, 40);
            event = SAE_CODE_MATCH_START_NOTIFY_DEV;
            break;
        default:
            slave->match_mana.ir_cache.is_downloading = false;
            air_ir_reset_code_match_timer(slave,0);
            break;
    }
    
    slave->match_mana.rf_ir_match.cur_stat = status;
    
    if (event != 0) {
        event_push(slave->user->callback, event, slave->handle, slave->user->callback_handle);
    }
    
}

static int _get_file_len(char* fname)
{
    struct stat st = {0};
    
    
    if (!fname || stat(fname, &st) != 0) {
        return -1;
    }
    
    return (int)st.st_size;
}

static u_int8_t* read_file_from_pos_to_end(char* fname, int pos ,int* dest_len)
{
    FILE* file;
    int len;
    u_int8_t* dest = NULL;
    
    if (!fname || pos < 0 ) {
        return NULL;
    }
    
    file = fopen(fname, "rb");
    if (!file) {
		log_err(false, "[%s] fopen failed\n", fname);
        return dest;
    }
    
    if(fseek(file, 0, SEEK_END) != 0){
		log_err(false, "[%s] fseek failed\n", fname);
        goto end;
    }
    
    len = (int)ftell(file);
    if (len <= pos || len <= 0) {
		log_err(false, "[%s] len %u pos %u\n", fname, len, pos);
        goto end;
    }
    
    len-=pos;
    
    dest = cl_malloc(len);
    if(!dest){
        goto end;
    }
    
    if(fseek(file, pos, SEEK_SET) != 0){
		log_err(false, "fseek\n");
        goto end;
    }
    
    if(fread(dest, len, 1, file) <= 0){
		log_err(false, "[%s] fread <= 0\n", fname);
        cl_free(dest);
        dest = NULL;
    }else{
        if (dest_len) {
            *dest_len = len;
        }
    }
    
end:
    fclose(file);
    return dest;
    
}

int air_ir_mk_index(slave_t* slave)
{
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    u_int8_t onoff,mode,temp,wind,wind_direct,key_id;
    
    if ( cache->is_ir_info_valid && (cache->d_ir_id == cache->cur_dev_ir_id)) {
        
#if 0
        //临时这样写
        //应该从服务器匹配的数据中获取到匹配的编码是什么状态，更新到本地，然后再取
        cache->onoff = true;
        cache->mode = AC_MODE_AUTO;
        cache->wind_direct = AC_DIR_AUTO;
        cache->wind = AC_WIND_AUTO;
        cache->temp = 16;
        cache->key_id = AC_KEY_POWER;
#else
        if (cache->temp < 16) {
            cache->temp = 16;
        }
        
        if (cache->temp > 32) {
            cache->temp = 32;
        }
#endif
        
        onoff = cache->onoff?AC_POWER_ON:AC_POWER_OFF;
        mode = cache->mode;
        temp = cache->temp - 16;
        wind = cache->wind;
        wind_direct = cache->wind_direct;
        key_id = cache->key_id;
        
        if (cache->d_item_num == 15000 ) {
            return onoff*7500 + mode*1500 + temp*100 +
            wind*25 + wind_direct*5 + key_id;
        }else if(cache->d_item_num == 3000){
            return onoff*1500 + mode*300 + temp*20 +
            wind*5 + wind_direct;
        }
    }
    
    return -1;
}

bool air_ir_get_ir_detail_code(slave_t* slave,u_int8_t* code,int* codelen)
{
    int index;
    int total,offset,len,use,clen,bits;
    u_int8_t value;
    priv_air_ir_stat_cache* cache ;
    
    if (!slave || !code) {
        return false;
    }
    
    cache = &slave->match_mana.ir_cache;
    index = air_ir_mk_index(slave);
    
    if (index < 0 ) {
        return false;
    }
    
    
    total = index * cache->d_ir_bit_num;
    offset = total >> 3;
    use = total & 0x7;
    len = ( use + cache->d_ir_bit_num + 7)/8;
    clen = 0;
    
    if (cache->ir_detail_len < len + offset) {
        return false;
    }
    
    for ( bits = cache->d_ir_bit_num ; bits > 0; offset++) {
        value = cache->ir_detail[offset] >> use;
        if (use != 0 && offset < cache->ir_detail_len) {
            value |= (cache->ir_detail[offset+1]  << (8-use));
        }
        
        if (bits > 8) {
            bits-=8;
        }else{
            value <<= (8-bits);
            value >>= (8-bits);
            bits =0;
        }
        
        code[clen++] = value;
    }
    
    if (codelen) {
        *codelen = clen;
    }
    
    return true;
}

bool air_rf_ir_send_signal(slave_t* slave)
{
    char buf[512];
    int len = 0;
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    rf_tlv_t* tlv = (rf_tlv_t*)buf;
    ucp_wk_air_ir_ctrl* ic = (ucp_wk_air_ir_ctrl*)(tlv+1);
    
    if(!cache->is_ir_info_valid ){
		log_debug("enter %s %d is_ir_info_valid=%u cache=%p sn=%"PRIu64"\n", __FUNCTION__, __LINE__, cache->is_ir_info_valid, cache, slave->sn);
        return false;
    }
    
    if(!cache->is_ir_info_valid ){
		log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
        return false;
    }
    
    
    if (!air_ir_get_ir_detail_code(slave, ic->code, &len)||len <= 0) {
		log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
        return false;
    }
    
    ic->fan = cache->wind;
    ic->fan_dir = cache->wind_direct;
    ic->key_id = cache->key_id;
    ic->mode = cache->mode;
    ic->onoff = cache->onoff;
    ic->temp = cache->temp;
    
    
    log_debug(" fan %u fan_dir %u key_id %u mode %u onoff %u temp %u \n",ic->fan,ic->fan_dir,ic->key_id,ic->mode,
              ic->onoff,ic->temp);
    if (len < 20) {	
        tlv->type = UP_TLV_AIR_IR_CTRL;
        tlv->len = (u_int8_t)(len + sizeof(*ic));
        rfgw_send_tt_packet(slave,buf,sizeof(*tlv)+tlv->len);
    }else{
#ifdef USE_BMP_TEST
		if (!rf_slave_send_big_pkt_v2(slave, RF_BIG_PKT_IR_CTRL, (u_int8_t*)&buf[2], (u_int16_t)(len+sizeof(*ic)), rbmp_callback)) {
			log_debug("%s rf_slave_send_big_pkt_v2 failed\n", __FUNCTION__);
		}
#else
        rf_slave_send_big_pkt(slave,BD_IR_CT_IR_CTRL,(u_int8_t*)&buf[2],(u_int16_t)(len+sizeof(*ic)));
#endif    
    }
    
    return true;
}

static bool init_ir_info(slave_t* slave,u_int8_t* src)
{
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
	cl_wk_air_info_t *ai = &slave->dev_info.rf_stat.dev_priv_data.wk_air_info;
    u_int8_t* p;
    u_int8_t item_len;
    u_int16_t value;
    u_int32_t u32Value;
    char fname[256];
   
    log_debug("enter %s %d \n", __FUNCTION__, __LINE__);
    if (src) {
        p = src;
    }else{
        if (!cache->ir_info || cache->ir_info_len < 140) {
			log_err(false, "ir_info %p, info_len %u\n", cache->ir_info, cache->ir_info_len);
            return false;
        }
    	p = cache->ir_info;
    }
    
    p+=2;
    cache->d_ir_id = *((u_int16_t*)p);
    cache->d_ir_id = ntohs(cache->d_ir_id);

    p+=2;
    
    p = p + (*p);
    p+=1;
    
    p = p + (*p);
    p+=1;
    
    
    cache->d_item_num = *((u_int16_t*)p);
    cache->d_item_num = ntohs(cache->d_item_num);
    
    p+=2;
    
    item_len = *p;
	cache->item_len = item_len;
    p+=1;
    
    p+=(item_len*2);
    
    p+=16; //md5
    
    //p+=130;
    cache->ext_tlv_len = ntohs(*((u_int16_t*)p));
	p += 2;
	memcpy(cache->ext_tlv, p, sizeof(cache->ext_tlv));
	p += 128;

	// 关键数据掩码
	memcpy(cache->bytes_map, p, item_len);
	p += item_len;
	
	// 公共掩码
	memcpy(cache->mask_data, p, item_len);
	p += item_len;

	// 公共数据
    memcpy(cache->common_data, p, item_len);
    p += item_len;
	
   // p+=(item_len*3);
    
    value = *((u_int16_t*)p);
    value = ntohs(value);
    cache->d_ir_bit_num = value & 0xFF;
    
    p+=2;
    u32Value = *((u_int32_t*)p);
    u32Value = ntohl(u32Value);
    if (src) {
        cache->download_total_len = u32Value;
    }else{
        sprintf(fname, "%s/airir/%u.detail",cl_priv->dir,cache->d_ir_id);
        if (_get_file_len(fname) != u32Value) {
			log_err(false, "file len %u , u32value %u file[%s]\n", _get_file_len(fname), u32Value, fname);
            return false;
        }
        cache->cur_dev_ir_id = cache->d_ir_id;
		ai->stat.ir_id = cache->d_ir_id;
        cache->is_ir_info_valid = true;
		log_debug("enter %s %d is_ir_info_valid=%u cache=%p sn=%"PRIu64"\n", __FUNCTION__, __LINE__, cache->is_ir_info_valid, cache, slave->sn);
    }

	p += sizeof(u32Value);

	// 
	cache->level_count2 = *p;
	p += 1;

	cache->delta = *p;
    
    return true;
}

static bool ir_info_to_i8_dev_info(slave_t* slave, u_int8_t* dest , int* dest_len)
{
    ucp_air_ir_info_t* uai;
    priv_air_ir_stat_cache* cache;
    ucp_air_ir_server_match_info_t* si;
    u_int8_t* p;
    u_int8_t item_len;
    u_int8_t* tmp;
    
    if (!slave || !dest || !dest_len) {
        return false;
    }
    
    cache = &slave->match_mana.ir_cache;
    if (!cache->is_ir_info_valid || !cache->ir_info) {
        return false;
    }

	log_debug("ir_info_to_i8_dev_info cache->d_ir_id=%u\n", cache->d_ir_id);
    uai = (ucp_air_ir_info_t*)dest;
    uai->ir_id = htons(cache->d_ir_id); 
    uai->wk_stat_len = 0x6;
    si = (ucp_air_ir_server_match_info_t*)(uai+1);
    si->key_id = cache->key_id;
    si->mode = cache->mode;
    si->onoff = cache->onoff;
    si->temp = cache->temp;
    si->wind = cache->wind;
    si->wind_direct = cache->wind_direct;
    
    tmp = (u_int8_t*)(si+1);
    
    
    p = cache->ir_info;
    p+=4;
    p = p + (*p);
    p+=1;
    
    uai->mode_len = *p;
    memcpy(tmp, p+1, uai->mode_len);
    
    tmp+=uai->mode_len;
    p = p + (*p);
    p+=1;
    
    p+=2;
    
    item_len = *p;
    uai->data_mode_len = item_len;
    p+=1;
    
    p+=(item_len*2);
    
    p+=16; //md5
    
    p+=130;
    
    p+=item_len;
    
    memcpy(tmp, p+item_len, item_len);
    tmp+= item_len;
    
    memcpy(tmp, p, item_len);
    
    *dest_len = sizeof(*uai) + item_len*2 + uai->mode_len + uai->wk_stat_len;
    
    log_debug("i8 info ir_id %u  mode_len %u data_mode_len %u\n",uai->ir_id,uai->mode_len,uai->data_mode_len);
    
    return true;
}

static bool load_ir_to_memory(slave_t* slave,u_int16_t new_ir_id)
{
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    char fname[256];
    
    SAFE_FREE(cache->ir_info);
    SAFE_FREE(cache->ir_detail);

    sprintf(fname, "%s/airir/%u.info",cl_priv->dir,new_ir_id);
    cache->ir_info = read_file_from_pos_to_end(fname, 0,&cache->ir_info_len);
    if(!cache->ir_info || !init_ir_info(slave,NULL) ){
		log_err(false, "load ir to memory failed\n");
        SAFE_FREE(cache->ir_info);
        return false;
    }
    
    sprintf(fname, "%s/airir/%u.detail",cl_priv->dir,new_ir_id);
    cache->ir_detail = read_file_from_pos_to_end(fname, 0,&cache->ir_detail_len);
    if(! cache->ir_detail || cache->ir_detail_len <= 0x10){
		log_err(false, "load ir to memory failed\n");
        SAFE_FREE(cache->ir_info);
        SAFE_FREE(cache->ir_detail);
        return false;
    }
    
    return true;
}

static bool is_file_exist(char* fname)
{
    if (!fname) {
        return false;
    }
    
    return !cl_access(fname, 0);
}

static void try_create_ir_dir()
{
    char dir[256];
    
    sprintf(dir, "%s/airir/",cl_priv->dir);
    
    log_debug("try_create_ir_dir %s\n",dir);
    
    if (!is_file_exist(dir)) {
        MKDIR(dir, 0777);
    }
}

static bool save_ir_data_to_file(u_int64_t sn,u_int8_t* data,u_int16_t d_len)
{
    char dir[256];
    FILE* file;
    
    if (!data || d_len <=0) {
        return false;
    }
    
    try_create_ir_dir();
    
    sprintf(dir, "%s/airir/%"PRIu64".match_stat",cl_priv->dir,sn);
    file = fopen(dir, "wb");
    if (!file) {
        return false;
    }
    
    fwrite(data, d_len, 1, file);
    fclose(file);
    
    return true;
}

static bool save_ir_info_to_file(u_int16_t ir_id,u_int8_t* data,u_int16_t d_len)
{
    char dir[256];
    FILE* file;
    
    if (!data || d_len <=0) {
        return false;
    }
    
    try_create_ir_dir();
    
    sprintf(dir, "%s/airir/%u.info",cl_priv->dir,ir_id);
    file = fopen(dir, "wb");
    if (!file) {
        return false;
    }
    
    fwrite(data, d_len, 1, file);
    fclose(file);
    
    return true;
}


static void _trunc_ir_detail_file(u_int16_t ir_id)
{
    char fname[256];
    FILE* file;
    
    try_create_ir_dir();
    
    sprintf(fname, "%s/airir/%u.detail",cl_priv->dir,ir_id);
    
    file = fopen(fname, "wb");
    if (!file) {
        return ;
    }

    fclose(file);
    
    return ;
}

static int _append_data_to_detail(u_int16_t ir_id,u_int8_t* data, int d_len)
{
    char fname[256];
    FILE* file;
    int pos;
    
    try_create_ir_dir();
    
    
    if (!data || d_len <= 0) {
        return -1;
    }
    
    sprintf(fname, "%s/airir/%u.detail",cl_priv->dir,ir_id);
    
    file = fopen(fname, "ab");
    if (!file) {
        return -1;
    }
    
    fwrite(data, d_len, 1, file);
    
    pos = (int)ftell(file);
    
    fclose(file);
    
    return pos;
}

static void ucp_ir_code_hdr_order(ucp_ir_code_data2_t* hdr)
{
    hdr->ir_id = ntohs(hdr->ir_id);
    hdr->len = ntohs(hdr->len);
    hdr->pos = ntohl(hdr->pos);
}

static int udp_rf_do_download_ir(slave_t* slave,u_int8_t* data, int d_len)
{
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache;
    char fname[256];
    int f_len;
    u_int32_t next_pos = 0;
    ucp_ir_code_data2_t* hdr;
    u_int8_t buf[64];
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    ucp_ir_code_dl_req_t* req = (ucp_ir_code_dl_req_t*)(uo+1);
    pkt_t* pkt;
    
    
    sprintf(fname, "%s/airir/%u.detail",cl_priv->dir,cache->download_id);
    
    if (data) {
        if (d_len < sizeof(*hdr)) {
            log_err(false,"udp_rf_do_download_ir dlen %u hdr len %u\n",d_len,sizeof(*hdr));
            return -1;
        }
        
        hdr = (ucp_ir_code_data2_t*)data;
        ucp_ir_code_hdr_order(hdr);
        if (hdr->ir_id != cache->download_id) {
            log_err(false,"udp_rf_do_download_ir ir_id %u down_id %u\n",hdr->ir_id,cache->download_id);
            return -1;
        }
        
        f_len = _get_file_len(fname);
        if (f_len != hdr->pos) {
            log_err(false,"udp_rf_do_download_ir flen %u hdr->pos %u\n",f_len,hdr->pos);
            return -1;
        }
        
        d_len-=sizeof(*hdr);
        f_len = _append_data_to_detail(hdr->ir_id,hdr->data,d_len);
        if (f_len < 0 || f_len != hdr->pos + d_len) {
            log_err(false,"udp_rf_do_download_ir flen %u expect len %u\n",f_len,hdr->pos+d_len);
            return -1;
        }
        
        next_pos = f_len;
        
    }else{
        //启动下载
        f_len = _get_file_len(fname);
        if (f_len < 0) {
            //期望断点续传
            if(next_pos != 0 ){
                return -1;
            }
        }else{
            if(next_pos !=  f_len){
                log_err(false,"udp_rf_do_download_ir next_pos %d f_len %d \n",next_pos, f_len);
                return -1;
            }
        }
    }
    
    if (next_pos >= cache->download_total_len) {
        log_err(false,"udp_rf_do_download_ir download ok %u %u \n",next_pos,cache->download_total_len);
        //下载完成了
        return next_pos;
    }
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_DATA2, sizeof(*req));
    req->ir_id = htons(cache->download_id);
    req->pos = htonl(next_pos);
    if ((cache->download_total_len - next_pos) >= IR_CODE_BYTE_PER_PKT ) {
        req->len = IR_CODE_BYTE_PER_PKT;
    }else{
        req->len = cache->download_total_len - next_pos;
    }
    req->len = ntohs(req->len);
    
    pkt = uasc_mk_ctrl_pkt(slave->sn, UCA_GET, 0x1, buf, sizeof(*uo)+sizeof(*req));
    if (!pkt) {
        return -1;
    }
    uasc_wait_request_add(cl_priv->uasc_session, pkt, slave->sn);
    
    return next_pos;
}

static bool try_send_if_info_to_dev(slave_t* slave);

static void proc_match_info(slave_t* slave,priv_air_ir_stat_cache* cache,u_int8_t* match_pkt, int pkt_len)
{
    u_int16_t* ptr = (u_int16_t*)match_pkt;
    u_int16_t num;
    u_int8_t* data;
    
    if (!slave || !cache || !match_pkt) {
        return;
    }
    
    if(*ptr == 0){
        return;
    }
    
    num = ntohs(*ptr);
    
    if ((int)((num +7)*sizeof(u_int16_t)) > pkt_len) {
        return;
    }
    
    ptr++;
    ptr+=(num+3);
    
    data = (u_int8_t*)ptr;
    cache->onoff = *data++;
    cache->mode = *data++;
    cache->temp = *data++;
    cache->wind = *data++;
    cache->wind_direct = *data++;
    cache->key_id = *data++;
    
    //处理一下学习到的数据
    cache->onoff = !(!!cache->onoff);	// 本地认为0是关机 1是开机
    cache->temp += 16;
}

void ir_ql_sync_other(u_int64_t sn, u_int16_t ir_id)
{
	ir_ql_t *p, *n;
	user_t *puser;
	slave_t *pslave;

	log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
	stlc_list_for_each_entry_safe(ir_ql_t, p, n, &cl_priv->ir_query_list, link) {
		puser = user_lookup_by_sn(p->user_sn);
		if (!puser) {
			log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
			continue;
		}
		
		pslave = slave_lookup_by_sn(puser, p->slave_sn);
		if (!pslave) {
			log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
			continue;
		}

		if (pslave->sn == sn) {
			log_debug("enter %s %d\n", __FUNCTION__, __LINE__);
			continue;
		}

		if (p->ir_id != ir_id) {
			log_debug("enter %s %d p->ir_id=%u ir_id=%u\n", __FUNCTION__, __LINE__, p->ir_id, ir_id);
			continue;
		}

		if (load_ir_to_memory(pslave, ir_id) && init_ir_info(pslave, NULL)) {
			log_debug("%s sn=%"PRIu64" init id=%u success\n", __FUNCTION__, pslave->sn, ir_id);
		} else {
			log_debug("%s sn=%"PRIu64" init id=%u failed\n", __FUNCTION__, pslave->sn, ir_id);
		}

		stlc_list_del(&p->link);
		cl_free(p);
	}
}

bool udp_rf_update_server_ia_code_subobj(slave_t* slave,u_int8_t action,ucp_obj_t* obj)
{
    bool ret = false;
    priv_rf_ir_match_t* mi = &slave->match_mana.rf_ir_match;
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache ;
    u_int16_t* ptr,num;
    u_int8_t err = 0;
    int len;
    
    switch (obj->attr) {
        case UCAT_CODE_MATCH:
            if (mi->cur_stat != MS_WK_SERVER_MATCH) {
                log_debug("recv code match state err stat = %u\n", mi->cur_stat);
				err = MS_WK_ERR_SERVER_IR;
                break;
            }
            
            ptr = OBJ_VALUE(obj, u_int16_t*);
            
            proc_match_info(slave,cache,(u_int8_t*)(obj+1),obj->param_len);
            num = ntohs(*ptr);
            if (num == 0) {
                log_debug("recv code match irid = %u\n", num);
                err = MS_WK_ERR_SERVER_IR;
                break;
            }
            save_ir_data_to_file(slave->sn, (u_int8_t*)(obj+1), obj->param_len);
            ptr++;
            if (*ptr == 0) {
                log_debug("recv code match err irid = %u\n", ntohs(*ptr));
                err = MS_WK_ERR_SERVER_IR;
                break;
            }
            
            //保证匹配可以必须进行
            cache->is_ir_info_valid = false;
			log_debug("enter %s %d is_ir_info_valid=%u cache=%p sn=%"PRIu64"\n", __FUNCTION__, __LINE__, cache->is_ir_info_valid, cache, slave->sn);
            _trunc_ir_detail_file(ntohs(*ptr));
            
            air_ir_code_match_set_status(slave, MS_WK_PHONE_DOWN_SING);
            reset_ir_info(slave,ntohs(*ptr));
            
            break;
        case UCAT_CODE_INFO:
            ptr = OBJ_VALUE(obj, u_int16_t*);
            err = *((u_int8_t*)ptr);
            
            if (err != 0 ) {
                err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                log_err(false,"recv code info irid error = %u\n", err);
                break;
            }
            
            ptr++;
            num = ntohs(*ptr);
            
            if (cache->is_downloading ) {
                err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                log_debug("recv code info irid  already downloading\n");
                break;
            }
            
            save_ir_info_to_file(num,(u_int8_t*)(obj+1), obj->param_len);
            
            cache->download_total_len = 0;
            //获取红外信息
            if (!init_ir_info(slave, (u_int8_t*)(obj+1))) {
                err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                log_debug("init ir_info faild\n");
                break;
            }
            
            //检查是否是相同id
            if (cache->download_id != cache->d_ir_id || !cache->download_id || !cache->download_total_len) {
                err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                log_debug("proc ir id  cache->download_id %u  cache->d_ir_id %u cache->download_total_len %u\n", cache->download_id,cache->d_ir_id,cache->download_total_len);
                break;
            }
            
            //清空文件
            _trunc_ir_detail_file(cache->download_id);
            
            if (udp_rf_do_download_ir(slave,NULL,0) < 0) {
                err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                log_debug("proc xxxx ir id  cache->download_id %u  cache->d_ir_id %u cache->download_total_len %u\n", cache->download_id,cache->download_id,cache->download_total_len);
                break;
            }
            
            log_debug("proc xxxx ir id  cache->download_id %u  cache->d_ir_id %u cache->download_total_len %u\n", cache->download_id,cache->download_id,cache->download_total_len);
            
            
            cache->is_downloading = true;
            
            break;
        case UCAT_CODE_DATA2:
            if (!cache->is_downloading ) {
                log_err(false,"udp_rf_do_download_ir UCAT_CODE_DATA2 not in downloading\n");
                break;
            }

			//国外服务器下载编码需要的时间比较长，这里收到数据后重置下状态机
			air_ir_reset_code_match_timer(slave, 20);
            len = udp_rf_do_download_ir(slave,(u_int8_t*)(obj+1), obj->param_len);
			log_debug("enter %s %d  len=%d totaolen=%u sn=%"PRIu64"\n", __FUNCTION__, __LINE__, len, cache->download_total_len, slave->sn);
            if (len < 0) {
                err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
            }else if(len >= (int)(cache->download_total_len)){
                if (!load_ir_to_memory(slave, slave->match_mana.ir_cache.download_id) ) {
                    err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                    log_err(false,"udp_rf_do_download_ir load ir to momery down id =  %u\n",slave->match_mana.ir_cache.download_id);
                    break;
                }
                log_debug("enter %s %d \n", __FUNCTION__, __LINE__);
                if ( !init_ir_info(slave, NULL)) {
                    err = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
                    log_err(false,"udp_rf_do_download_ir UCAT_CODE_DATA2 init ir failed\n");
                    break;
                }
                
                //在进行匹配操作
                if (mi->cur_stat != MS_WK_IDLE) {
                    air_ir_code_match_set_status(slave, MS_WK_NOTIFY_DEV_CODE_ID);
                    try_send_if_info_to_dev(slave);
                } else {
     				// 红外下载完成
     				_rf_wkair_query_ir_code(slave, 0);
                }

				//这里将所有需要初始化相同irid的从设备处理下
				ir_ql_sync_other(slave->sn, slave->match_mana.ir_cache.download_id);
            }
            
            break;
            
        default:
            break;
    }
    
    if (err != 0 && mi->cur_stat != MS_WK_IDLE) {
        mi->error = err;
        event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
        air_ir_code_match_set_status(slave, MS_WK_IDLE);
    }
    
    return ret;
}

static void start_download_ir_file(slave_t* slave,u_int16_t new_ir_id)
{
    u_int8_t buf[64];
    ucp_obj_t * uo;
    pkt_t* pkt;
    u_int16_t * ptr;
    priv_rf_ir_match_t* mi;
    
    uo = (ucp_obj_t*)buf;
    ptr = (u_int16_t*)(uo+1);
    
    log_debug("start_download_ir_file\n");
    
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_CODE, UCAT_CODE_INFO, sizeof(*ptr));
    *ptr = htons(new_ir_id);
    
    pkt = uasc_mk_ctrl_pkt(slave->sn, UCA_GET, 0x1, buf, sizeof(*uo)+sizeof(*ptr));
    
    if (pkt) {
        uasc_wait_request_add(cl_priv->uasc_session, pkt, slave->sn);
    }else{
        mi = &slave->match_mana.rf_ir_match;
        if (mi->cur_stat == MS_WK_PHONE_DOWN_SING) {
            mi->error = MS_WK_ERR_SERVER_DOWNLOAD_CODE;
            event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
            air_ir_code_match_set_status(slave, MS_WK_IDLE);
        }
    }
    
}

void ir_ql_add(u_int64_t user_sn, u_int64_t slave_sn, u_int16_t ir_id)
{
	ir_ql_t *p;

	p = cl_calloc(sizeof(*p), 1);
	if (!p) {
		return;
	}

	STLC_INIT_LIST_HEAD(&p->link);
	stlc_list_add_tail(&p->link, &cl_priv->ir_query_list);

	p->user_sn = user_sn;
	p->slave_sn = slave_sn;
	p->ir_id = ir_id;
	log_debug("enter %s usersn=%"PRIu64" slavesn=%"PRIu64" id=%u\n", __FUNCTION__, user_sn, slave_sn, ir_id);
}

ir_ql_t *ir_ql_find_by_id(u_int16_t ir_id)
{
	ir_ql_t *p, *n;

	stlc_list_for_each_entry_safe(ir_ql_t, p, n, &cl_priv->ir_query_list, link) {
		if (ir_id == p->ir_id) {
			return p;
		}
	}

	return NULL;
}

void ir_ql_init()
{
	STLC_INIT_LIST_HEAD(&cl_priv->ir_query_list);
}

void ir_ql_exit()
{
	ir_ql_t *p, *n;

	stlc_list_for_each_entry_safe(ir_ql_t, p, n, &cl_priv->ir_query_list, link) {
		stlc_list_del(&p->link);
		cl_free(p);
	}
}

bool reset_ir_info(slave_t* slave,u_int16_t new_ir_id)
{
    priv_air_ir_stat_cache* cache = &slave->match_mana.ir_cache ;
    
    if (cache->is_ir_info_valid && cache->d_ir_id == new_ir_id) {
         log_debug("reset_ir_onfo xxxx d_ir_id %u\n",new_ir_id);
        return true;
    }
    
    log_debug("reset_ir_onfo new_ir_id %u\n",new_ir_id);
    
    cache->is_ir_info_valid = false;
	log_debug("enter %s %d is_ir_info_valid=%u cache=%p sn=%"PRIu64"\n", __FUNCTION__, __LINE__, cache->is_ir_info_valid, cache, slave->sn);
    
    if (!load_ir_to_memory(slave, new_ir_id)) {
        if (cache->is_downloading && cache->download_id == new_ir_id) {
            return false;
        }
		
        cache->download_id = new_ir_id;
        cache->is_downloading = false;
		
		//这里判断一下，id是否已经在查询了，如果已经在查询了就不必查询了
		if (ir_ql_find_by_id(new_ir_id)) {
			ir_ql_add(slave->user->sn, slave->sn, new_ir_id);
			log_debug("sn=%"PRIu64" there is someone query id=%u already now\n", slave->sn, new_ir_id);
			return false;
		}
		ir_ql_add(slave->user->sn, slave->sn, new_ir_id);
        start_download_ir_file(slave, new_ir_id);
		
        return false;
    }
    
    if(!init_ir_info(slave,NULL)){
        return false;
    }
    
    return true;
}

static bool try_send_if_info_to_dev(slave_t* slave)
{
    u_int8_t buf[1024], other_info[2] = {0};
    int dest_len = 0;
	rf_tlv_t *tlv;
    
    if (!ir_info_to_i8_dev_info(slave, buf, &dest_len) || dest_len <= 0) {
        if(slave->match_mana.rf_ir_match.cur_stat !=  MS_WK_IDLE){
            air_ir_code_match_set_status(slave,MS_WK_IDLE);
            event_push(slave->user->callback, SAE_CODE_MATCH_FAILED, slave->handle, slave->user->callback_handle);
            slave->match_mana.rf_ir_match.error = MS_WK_ERR_IR_TO_DEVICE_TIMEOUT;
        }
        return false;
    }
    
    log_debug("rf_slave_send_big_pkt send CTRL result ! len = %d\n",dest_len);
#ifdef USE_BMP_TEST
		if ((dest_len + (int)slave->match_mana.ir_cache.ext_tlv_len) <= (int)sizeof(buf)) {
			memcpy((void *)&buf[dest_len], (void *)slave->match_mana.ir_cache.ext_tlv, 
			slave->match_mana.ir_cache.ext_tlv_len);
			dest_len += (int)slave->match_mana.ir_cache.ext_tlv_len;

			// 把DELTA和第二长度也传过去
			tlv = (rf_tlv_t *)&buf[dest_len];
			tlv->type = 10;
			tlv->len = 2;
			
			other_info[0] = slave->match_mana.ir_cache.delta;
			other_info[1] = slave->match_mana.ir_cache.level_count2;
			memcpy((u_int8_t*)&tlv[1], other_info, 2);
			dest_len += sizeof(*tlv) + tlv->len;
			
			if (!rf_slave_send_big_pkt_v2(slave, RF_BIG_PKT_IR_DB, buf, dest_len, rbmp_callback)) {
				log_debug("%s rf_slave_send_big_pkt_v2 failed\n", __FUNCTION__);
			} 
		}
#else
        rf_slave_send_big_pkt(slave,BD_IR_CT_IR_DOWNLOAD,buf,dest_len);
#endif 	
    
    air_ir_code_match_set_status(slave,MS_WK_NOTIFY_DEV_CODE_ID);
    
    return true;
}

void air_ir_start_to_server_match(slave_t* slave)
{
    priv_rf_ir_match_t* m = &slave->match_mana.rf_ir_match;
    pkt_t* pkt;
    
    if (!m->matched_code || !m->code_len || !cl_priv->uasc_session) {
        log_err(false, "air_ir_start_to_server_match error ******** matched_code %p code_len %u uasc_session %p\n", 
			m->matched_code, m->code_len, cl_priv->uasc_session);
        return;
    }
    
    pkt = uasc_mk_ctrl_pkt(slave->sn,UCA_GET, 0x1,m->matched_code,m->code_len);
    uasc_wait_request_add(cl_priv->uasc_session, pkt, slave->sn);

	log_debug("send to app server state %u\n", ((uasc_session_t*)(cl_priv->uasc_session))->status);
}

static void rfgw_dev_work_query(user_t *user, batch_sn_handle_t *batch)
{
    u_int8_t buf[2048] = {0};
    u_int16_t pos = 0,n = 0;
    u_int16_t max_pkt_len = 1200;
    u_int8_t cmd_len;
    slave_t *slave;
    ucp_obj_t* uo ;
    net_rfgw_tt_t* nf;
    rf_tlv_t* rt;
    u_int8_t i;
	smart_air_ctrl_t* ac = NULL;
    
    if (!user) {
        return;
    }
	
	if (user->smart_appliance_ctrl) {
		ac = ((smart_appliance_ctrl_t *)user->smart_appliance_ctrl)->sub_ctrl;
	}
	
    log_debug("%s sn [%012"PRIu64"]\n",__FUNCTION__, user->sn);
    //检查在线设备是否更新，没更新的，组织查询报文
    for(i = 0; i < batch->cnt; i++) {
	slave = slave_lookup_by_handle(user, batch->handle[i]);
		if(slave == NULL) {
			continue;	
		}
		if (slave->is_new_ver) {
			continue;
		}
        if (slave->status ==  BMS_BIND_ONLINE) {
            uo = (ucp_obj_t*)&buf[pos];
            nf = (net_rfgw_tt_t*)(uo+1);
            rt = (rf_tlv_t*)(nf+1);
            ////////////////////////////////////
            if (slave->ext_type == RF_EXT_TYPE_LIGHT) {
                nf->data[0] = RFTT_WORK_QUERY;
                cmd_len = 0x1;
            }else{
                // 通用的填充
                cmd_len = udp_rf_dev_mk_raw_stat_query_pkt(slave, nf->data);
                if (cmd_len == 0 ) {
					// 除了电威灯以外，其他设备都不需要发送透传查询命令
					continue;
					#if 0
                    //看看是不是tlv形式的
                    cmd_len = udp_rf_dev_mk_stat_query_pkt(slave,rt);
                    if (cmd_len == 0) {
                        continue;
                    }
					#endif
                }
            }
            
            log_debug("%s slave sn [%012"PRIu64"] ext_type [%u]\n", __FUNCTION__, slave->sn,slave->ext_type);
	
			//if (ac->is_support_rf_cache) {
            //	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT_CACHE, sizeof(*nf)+cmd_len);
			//} else {
            	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*nf)+cmd_len);
			//}
			
            //fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*nf)+cmd_len);
            nf->sn = ntoh_ll(slave->sn);
            nf->len = ntohs(cmd_len);
            pos += (sizeof(*uo)+sizeof(*nf)+cmd_len);
            n++;
            
            //一个数据包完成
            if (pos >= max_pkt_len) {
                sa_set_obj_value_only(user->uc_session, (u_int8_t)n, buf, pos);
                //sa_ctrl_obj_value(user->uc_session,UCA_GET,false, (u_int8_t)n, buf, pos);
                pos = 0;
                n = 0;
            }
        }
    }
    
    // 不够一个数据包的
    if (n>0 && pos >0) {
        log_debug("%s query all slave packet!", __FUNCTION__);
        sa_set_obj_value_only(user->uc_session, (u_int8_t)n, buf, pos);
        //sa_ctrl_obj_value(user->uc_session,UCA_GET,false, (u_int8_t)n, buf, pos);
    }
}
#if 0
{
	u_int8_t rgb, i, n;
	char buf[1024] = {0};
	int len = 0;
	slave_t *slave;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_tt_t *w = (net_rfgw_tt_t*)(uo+1);

	rgb = batch->rgb;
	i = 0; n = 0;
	for(i = 0; i < batch->cnt; i++){
		slave = slave_lookup_by_handle(user, batch->handle[i]);
		if(slave == NULL)
			continue;

		if(slave->dev_info.rfdev->is_ctrl){
			slave->dev_info.rfdev->ctrl_fail++;
		}
		slave->dev_info.rfdev->is_ctrl = 1;
		slave->dev_info.rfdev->ctrl_msec = get_msec();
		uo = (ucp_obj_t*)&buf[len];
		w = (net_rfgw_tt_t*)(uo+1);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_TT, sizeof(*w)+1);
		w->sn = ntoh_ll(slave->sn);
		w->len = ntohs(1);
		w->data[0] = RFTT_WORK_QUERY;
		n++;
		len += (sizeof(*uo)+sizeof(*w)+1);
	}
	if(n > 0)
		sa_set_obj_value_only(user->uc_session, n, buf, len);
}

#endif

static void rfgw_dev_delete(user_t *user, batch_sn_handle_t *batch)
{
	u_int8_t i, n;
	char buf[1024] = {0};
	int len = 0;
	slave_t *slave;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_list_t *list = (net_rfgw_list_t*)(uo+1);
	net_rfgw_dev_t *dev = (net_rfgw_dev_t*)(list+1);

	i = 0; n = 0; len = sizeof(*list);
	for(i = 0; i < batch->cnt; i++){
		slave = slave_lookup_by_handle(user, batch->handle[i]);
		if(slave == NULL)
			continue;
		
		dev->sn = ntoh_ll(slave->sn);
		dev->subtype = slave->sub_type;
		dev->extype = slave->ext_type;
		dev->tlv_count = 0;
		n++; dev++;
		len += sizeof(*dev);
	}
	if(n > 0){
		list->count = n;
		list->total = n;
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_LIST, len);
		sa_ctrl_obj_value(user->uc_session, UCA_DELETE, false, 1, buf, sizeof(*uo) + len);
	}
}

static void rfgw_dev_delete_all(user_t *user)
{
	u_int8_t n;
	char buf[1024] = {0};
	int len = 0;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_rfgw_list_t *list = (net_rfgw_list_t*)(uo+1);
	net_rfgw_dev_t *dev = (net_rfgw_dev_t*)(list+1);

	memset(buf, 0, sizeof(buf));
	n = 1; 
	len = sizeof(*list) + sizeof(*dev);
	
	list->count = n;
	list->total = n;
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_LIST, len);
	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, false, 1, buf, sizeof(*uo) + len);
}


static void rfgw_dev_group(user_t *user, cl_dev_group_t *group)
{
	char buf[1500] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_dev_group_t *g = (net_dev_group_t *)(uo+1);
	u_int16_t len = sizeof(*g);
	u_int8_t action;
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	rfgw_priv_t *sdkinfo;

	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || user->sub_type != IJ_RFGW) {
		return;
	}

	sdkinfo = ac->com_udp_dev_info.device_info;
	if(sdkinfo &&sdkinfo->devgroup[group->group_id]){
		sdkinfo->devgroup[group->group_id]->query_dev++;
		if(sdkinfo->devgroup[group->group_id]->query_dev == 0)
			sdkinfo->devgroup[group->group_id]->query_dev = 1;
	}

	
	memset(g, 0, sizeof(*g));
	g->group_id = group->group_id;
	g->group_type = group->group_type;
	switch(group->reserved){
		case ACTION_QUERY:
			action = UCA_GET;	
			break;
		case ACTION_ADD:
			action = UCA_SET;
			memcpy(g->name, group->name, RF_MAX_NAME);
			for(g->dev_count = 0; g->dev_count < group->dev_cnt; g->dev_count++){
				g->sn[g->dev_count] = ntoh_ll(group->dev_sn[g->dev_count]);
				len += sizeof(u_int64_t);
			}
			break;
		case ACTION_DEL:
			action = UCA_DELETE;
			break;
		default:
			return;
			break;
	}

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP_MEMBER, len);
	sa_ctrl_obj_value(user->uc_session, action, false, 1, buf, sizeof(*uo) + len);
    
    if(group->reserved == ACTION_ADD || group->reserved == ACTION_DEL){
        sa_query_obj(user->uc_session,UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP);
    }
}

static void rfgw_dev_dbc(user_t *user, ucp_rfgw_dbc_hd_t *phd)
{
	char buf[1024*4] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	ucp_rfgw_dbc_hd_t *phdn = (ucp_rfgw_dbc_hd_t *)(uo+1);
	u_int16_t len = sizeof(*phdn) + phd->num*sizeof(u_int64_t);

	memcpy((void *)phdn, (void *)phd, len);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DBC, len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 1, buf, sizeof(*uo) + len);
}

static void rfgw_set_dev_name(user_t *user, net_dev_name_t *t)
{
	char buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_dev_name_t *nt;
	
	uo = (ucp_obj_t*)buf;
	nt = (net_dev_name_t*)(uo+1);
	nt->sn = ntoh_ll(t->sn);
	memcpy(nt->name, t->name, sizeof(t->name));
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_NAME, sizeof(*nt));
		
	sa_set_obj_value_only(user->uc_session, 1, buf, sizeof(*uo)+sizeof(*nt));
	sa_ctrl_obj_value(user->uc_session, UCA_GET, 0, 1, buf, sizeof(*uo)+sizeof(*nt));
}

int ucp_comm_fill_timer_rf_modify_pkt(user_t *user, ucp_comm_timer_rf_t *puct, cl_comm_timer_t *ptimer)
{
	int len = 0;
	u_int8_t *pdata = (u_int8_t *)&puct[1];
	
	if (!user || !puct || !ptimer) {
		return 0;
	}

	//这里专门处理下，rf定时器需要
	if (ptimer->week) {
		ptimer->week |= BIT(7);
	} else {
	//如果week==0，赋值
		ptimer->week = timer_add_next_day(ptimer);
	}

	comm_timer_local_2_utc(ptimer);
	puct->id = ptimer->id;
	puct->enable = ptimer->enable;
	puct->duration = htons(ptimer->duration);
	puct->hour = ptimer->hour;
	puct->min = ptimer->min;
	puct->week = ptimer->week;

	switch(ptimer->type) {
	case UT_TYPE_ON:
	case UT_TYPE_PERIOD_ONOFF:
		puct->type = UT_DEV_TYPE_ONOFF;
		len = puct->extened_len = 1;
		*pdata = 1;
		break;
	case UT_TYPE_OFF:
		puct->type = UT_DEV_TYPE_ONOFF;
		len = puct->extened_len = 1;
		*pdata = 0;
		break;
	default:
		log_debug("error !!!!!!!!!!!!!!!!!!!!!!!!ptimer->type=%u\n ", ptimer->type);
		len = puct->extened_len = 0;
		return 0;
		break;
	}

	log_debug("%s id=%u enable=%u\n", __FUNCTION__, ptimer->id, ptimer->enable);

	return (len + sizeof(*puct));	
}

static void _rfgw_proc_timer_add_rf(user_t *user, net_dev_timer_t *t)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);
	ucp_comm_timer_rf_t *puct = (ucp_comm_timer_rf_t *)(&prtlv[1]);
	u_int8_t len = 0;

	len = ucp_comm_fill_timer_rf_modify_pkt(user, puct, &t->timer);
	if (len == 0) {
		log_debug("_rfgw_proc_timer_add error !!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
		return;
	}
	
	prtlv->type = RF_TT_CMD_ADD_TIMER;
	prtlv->len = len;
	
	prt->sn = t->sn;
	prt->len = len + sizeof(rf_tlv_t);

	log_debug("_rfgw_proc_timer_add data_len=%u\n", len);
	//log_mem_dump("_rfgw_proc_timer_add", buf, sizeof(rf_tlv_t)+len+sizeof(net_rfgw_tt_t));
	rfgw_set_tt(user, (u_int8_t *)buf);
	
}

static void _rfgw_proc_timer_add(user_t *user, net_dev_timer_t *t)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);
	ucp_comm_timer_t *puct = (ucp_comm_timer_t *)(&prtlv[1]);
	u_int8_t len = 0;
	slave_t *slave;

	len = ucp_comm_fill_timer_modify_pkt(user, true, t->sn, puct, &t->timer);
	if (len == 0) {
		log_debug("_rfgw_proc_timer_add error !!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
		return;
	}

	prtlv->type = RF_TT_CMD_ADD_TIMER;
	prtlv->len = len;
	
	prt->sn = t->sn;
	prt->len = len + sizeof(rf_tlv_t);

	slave = _find_slave_at_list(t->sn, &user->slave);
	if (!slave) {
		log_debug("not found slave sn=%"PRIu64"\n", t->sn);
		return;
	}

	log_debug("_rfgw_proc_timer_add data_len=%u\n", prt->len);
	//log_mem_dump("_rfgw_proc_timer_add", buf, sizeof(rf_tlv_t)+len+sizeof(net_rfgw_tt_t));
    if (prt->len < 20) {
       rfgw_set_tt(user, (u_int8_t *)buf);
    }else{
#ifdef USE_BMP_TEST
		if (!rf_slave_send_big_pkt_v2(slave, RF_BIG_PKT_IR_TIMER, (u_int8_t *)prtlv, prt->len, rbmp_callback)) {
			log_debug("rf_slave_send_big_pkt_v2 failed\n");
		}
#else
        rf_slave_send_big_pkt(slave, BD_COMM_TIMER_CTRL, (u_int8_t *)prtlv, prt->len);
#endif
    }
}

static void _rfgw_proc_timer_add_ext(user_t *user, net_dev_timer_t *t)
{
	slave_t *slave;
	cl_comm_timer_head_t *pcthd;

	log_debug("enter _rfgw_proc_timer_add_ext\n");
	slave = _find_slave_at_list(t->sn, &user->slave);
	if (!slave) {
		log_debug("_rfgw_proc_timer_add_ext not found %"PRIu64"\n", t->sn);
		return;
	}

	pcthd = &slave->comm_timer;
	pcthd->is_slave = true;
	pcthd->sub_type = slave->sub_type;
	pcthd->ext_type = slave->ext_type;
	//判断下时间冲突
	if (comm_timer_is_conflict(user, pcthd, &t->timer)) {
		log_debug("%s timer conflict\n", __FUNCTION__);
		return;
	}

	if (slave->ext_type == RF_EXT_TYPE_KTCZ) {
		_rfgw_proc_timer_add_rf(user, t);
	} else {
		_rfgw_proc_timer_add(user, t);
	}
}

static void _rfgw_proc_timer_del(user_t *user, net_dev_timer_t *t)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);
	u_int8_t *pid = (u_int8_t *)(&prtlv[1]);
	u_int8_t len = 1;

	*pid = t->timer.id;
	
	prtlv->type = RF_TT_CMD_DEL_TIMER_RET;
	prtlv->len = len;
	
	prt->sn = t->sn;
	prt->len = len + sizeof(rf_tlv_t);
		
	rfgw_set_tt(user, (u_int8_t *)buf);
}

static void _rfgw_proc_timer_query(user_t *user, net_dev_timer_t *t)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);

	prtlv->type = RF_TT_CMD_GET_TIMER_SUMMARY;
	prtlv->len = 0;
	
	prt->sn = t->sn;
	prt->len = sizeof(rf_tlv_t);

	log_debug("%s _rfgw_proc_timer_query RF_TT_CMD_GET_TIMER_SUMMARY to sn=%"PRIu64"\n", user->name, t->sn);
	rfgw_set_tt(user, (u_int8_t *)buf);
}

static void _rfgw_proc_runtime_query(user_t *user, cln_common_info_t *info)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);
	u_int64_t slave_sn = 0;
	u_int8_t *value = NULL;

	value = cci_pointer_data(info);
	memcpy((void *)&slave_sn, value, sizeof(slave_sn));

	prtlv->type = RF_TT_CMD_GET_RUN_TIME;
	prtlv->len = 0;
	
	prt->sn = slave_sn;
	prt->len = sizeof(rf_tlv_t);

	log_debug("%s _rfgw_proc_runtime_query RF_TT_CMD_GET_RUN_TIME to sn=%"PRIu64"\n", user->name, slave_sn);
	rfgw_set_tt(user, (u_int8_t *)buf);
}

static void _rfgw_proc_timer_query_next(user_t *user, net_dev_timer_t *t)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);

	prtlv->type = RF_TT_CMD_GET_NEXT_TIMERS;
	prtlv->len = 0;
	
	prt->sn = t->sn;
	prt->len = sizeof(rf_tlv_t);
		
	rfgw_set_tt(user, (u_int8_t *)buf);
}

static void _rfgw_proc_comm_history_query(user_t *user, net_dev_comm_history_query_t *request)
{
	char buf[1024] = {0};
	net_rfgw_tt_t *prt = (net_rfgw_tt_t *)buf;
	rf_tlv_t *prtlv = (rf_tlv_t *)(&prt[1]);
	u_int32_t *index = (u_int32_t *)&prtlv[1];

	prtlv->type = RF_TT_CMD_QUERY_HISLOG;
	prtlv->len = sizeof(*index);
	*index = ntohl(request->index);
	
	prt->sn = request->sn;
	prt->len = sizeof(rf_tlv_t) + sizeof(index);

	log_debug("%s _rfgw_proc_comm_history_query RF_TT_CMD_QUERY_HISLOG to sn=%"PRIu64" index %u\n", user->name, request->sn, *index);
	rfgw_set_tt(user, (u_int8_t *)buf);
}


static void _rfgw_proc_group_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
    char buf[1024] = {0};
    int t_len;
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    net_rfgw_group_tt_t *up = cci_pointer_data(info);
    
    up->sub_type = IJ_RFGW;
    up->ext_type = RF_EXT_TYPE_LED_LAMP;
    
    t_len = sizeof(*up)+up->len;
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_DEV_GROUP_TT, (u_int16_t)sizeof(ucp_obj_t)+t_len);
    memcpy(uo+1,cci_pointer_data(info),t_len);
    
    sa_set_obj_value_only(user->uc_session, 0x1, uo, sizeof(ucp_obj_t)+t_len);
}

static void _rfgw_proc_upgrade_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
    char buf[1024] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    rfgw_upgrade_t *up = cci_pointer_data(info);
    
    //这里先把要升级的路径保存下来，去查询下设备是否可以升级
    SAFE_FREE(user->rf_up_filepath);
	user->rf_up_type = up->type;
	user->rf_up_filepath = cl_strdup(up->filepath);
	user->rf_need_up = true;
	
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_PARAM, 0);
    
    sa_ctrl_obj_value(user->uc_session, UCA_GET, 0, 1, buf, sizeof(*uo));
}

static void _rfgw_proc_commpat_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
	int len = 0;
    char buf[1024] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
	net_dev_param_t *pndp = (net_dev_param_t*)(&uo[1]);
    u_int8_t *pdata = cci_pointer_data(info);
    
    memset(buf, 0, sizeof(buf));
	len = sizeof(net_dev_param_t);
	pndp->commpat = pdata[0];
	pndp->channel = pdata[1];
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_PARAM, len);
	sa_ctrl_obj_value(user->uc_session, UCA_SET, 1, 1, buf, sizeof(*uo)+len);
}

static void _rfgw_proc_up_query_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
    char buf[1024] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
	
    fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_PARAM, 0);
	sa_ctrl_obj_value(user->uc_session, UCA_GET, 0, 1, buf, sizeof(*uo));
}

static void _rfgw_proc_up_check_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
	char url[1024];

	sprintf(url, "http://%s/cgi-bin/updev?sn=0&mastersn=%"PRIu64"&type=%hhu&subtype=0&exttype=0&vendor=%s&lang=%d&upstm=1",
		DEFAULT_DOMAIN, user->sn, TP_DS007, user->vendor_id, 1);

	log_debug("_rfgw_proc_up_check_ctrl url=%s\n", url);
	http_get(url, CLNE_HTTP_RFDEV_UP_CHECK, user->handle);
}

void _rfgw_img_cache_query(ucc_session_t *s)
{
	u_int8_t buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;

	log_debug("enter %s\n", __FUNCTION__);	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_IMG, 0);
	sa_ctrl_obj_value(s, UCA_GET, 0, 1, buf, sizeof(*uo));	
}

static void _rfgw_proc_img_cache_query_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
	if (!user->is_support_spe_up) {
		log_debug("sn=%"PRIu64" user->is_support_spe_up=0\n", user->sn);
		*ret = RS_NOT_SUPPORT;
		return;
	}
	_rfgw_img_cache_query(user->uc_session);
}

static void _rfgw_proc_img_cache_del_ctrl(user_t* user, cln_common_info_t *info,RS* ret)
{
	int i;
	u_int8_t num = 0;
	u_int8_t buf[1024] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pindex = (u_int8_t *)&uo[1];
	u_int8_t *pdata = cci_pointer_data(info);;

	if (!user->is_support_spe_up) {
		*ret = RS_NOT_SUPPORT;
		return;
	}

	num = pdata[0];
	pdata++;
	
	if (0 == num) {
		*ret = RS_INVALID_PARAM;
		return;
	}
	for(i = 0; i < num; i++) {
		pindex[i] = pdata[i];
	}

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_RFGW, UCAT_IA_RFGW_IMG, num);
	sa_ctrl_obj_value(user->uc_session, UCA_DELETE, true, 1, buf, sizeof(*uo) + num);
}

/*
	处理APP下来的用户请求
*/
bool rfgw_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
    batch_sn_handle_t* bh;
    cl_dev_group_t* dg;

	info = (cln_common_info_t *)&pkt->data[0];
    
	switch(info->action){
	case ACT_RFGW_JOIN:
		rfgw_join(user, info->u.u16_data[0]);
		break;
	case ACT_RFGW_JOIN_ACTION:
		{
			sdk_join_act_t *sdk_act = (sdk_join_act_t*)&info->u.u8_data[0];
			rfgw_join_action(user, sdk_act);
		}
		break;
	case ACT_RFGW_GROUP:
		{
			sdk_group_t *g = (sdk_group_t*)&info->u.u8_data[0];
			rfgw_group(user, g);						
		}
		break;
	case ACT_RFGW_TT:
		rfgw_set_tt(user, &info->u.u8_data[0]);
		break;
	case ACT_RFGW_TT_BATCH:
            bh = cci_pointer_data(info);
		rfgw_set_tt_batch(user, bh);
		break;
	case ACT_RFGW_WORK_QUERY:
        //自动查询状态
        rfgw_dev_work_query(user, (batch_sn_handle_t *)cci_pointer_data(info));
		break;
	case ACT_RFGW_DEVLIST:
        //自动查询列表
		rfgw_query_devlist(user);
		break;

	case ACT_RFGW_DEV_DEL:
        bh = cci_pointer_data(info);
		rfgw_dev_delete(user, bh);
		break;
	case ACT_RFGW_DEV_DEL_ALL:
		rfgw_dev_delete_all(user);
		break;
	case ACT_RFGW_DEV_NAME:
		{
			net_dev_name_t *t = (net_dev_name_t*)&info->u.u8_data[0];
			rfgw_set_dev_name(user , t);
		}
		break;
	case ACT_RFGW_DEV_GROUP:
        dg = cci_pointer_data(info);
		rfgw_dev_group(user, dg);
		break;

	case ACT_RFGW_DEV_GROUP_MEMBER:
        dg = cci_pointer_data(info);
        rfgw_dev_group(user, dg);
		break;

	case ACT_RFGW_DEV_GROUP_TT:
        _rfgw_proc_group_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_UPGRADE:
		_rfgw_proc_upgrade_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_COMMPAT:
		_rfgw_proc_commpat_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_UP_QUERY:
		_rfgw_proc_up_query_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_UP_CHECK:
		_rfgw_proc_up_check_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_IMG_CACHE_QUERY:
		_rfgw_proc_img_cache_query_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_IMG_CACHE_DEL:
		_rfgw_proc_img_cache_del_ctrl(user,info,ret);
		break;
	case ACT_RFGW_DEV_TIMER_DEL:
		{
			net_dev_timer_t *t = (net_dev_timer_t*)&info->u.u8_data[0];
			_rfgw_proc_timer_del(user , t);
		}
		break;
	case ACT_RFGW_DEV_TIMER_ADD:
		{
			net_dev_timer_t *t = (net_dev_timer_t*)&info->u.u8_data[0];
			_rfgw_proc_timer_add_ext(user , t);
		}
		break;
	case ACT_RFGW_DEV_TIMER_QUERY:
		{
			net_dev_timer_t *t = (net_dev_timer_t*)&info->u.u8_data[0];
			_rfgw_proc_timer_query(user , t);
			//_rfgw_proc_timer_query_next(user , t);
		}
		break;
	case ACT_RFGW_DEV_COMM_HISTORY_QUERY:
		{
			net_dev_comm_history_query_t *request = cci_pointer_data(info);
			_rfgw_proc_comm_history_query(user, request);
		}
		break;
	case ACT_RFGW_DEV_DEFENSE_BATCH_CONFIG:
		{
			int i;
			ucp_rfgw_dbc_hd_t *phd = cci_pointer_data(info);
			for(i = 0; i < phd->num; i++) {
				phd->sn[i] = ntoh_ll(phd->sn[i]);
			}
			rfgw_dev_dbc(user, phd);
		}
		break;

	case ACT_RFGW_HPGW_APPINFO:
	case ACT_RFGW_HPGW_SMS:
	case ACT_RFGW_HPGW_CONFIG_USER:
	case ACT_RFGW_HPGW_DEL_USER:
	case ACT_RFGW_HPGW_LAMP_CTRL:
		hpgw_proc_notify(user, pkt, ret);
		break;
	case ACT_RFGW_RF_RUNTIME_QUERY:
		_rfgw_proc_runtime_query(user, info);
		break;
	case ACT_RFGW_ONEKEY:
		rfgw_onekey_proc(user, pkt, ret);
		break;
	default:
		res = false;
		break;
	}
	return res;
}

void build_dev_group(cl_gw_info_t *info, rfgw_priv_t *sdkinfo)
{
	u_int32_t i;
	u_int8_t n = 0;
	info->dev_group_cnt = count_dev_group(sdkinfo);
	if(info->dev_group_cnt == 0){
		info->dev_group = NULL;
		return;
	}
	
	info->dev_group = cl_calloc(sizeof(cl_dev_group_t), info->dev_group_cnt);
	if(info->dev_group == NULL){
		info->dev_group_cnt = 0 ;
		return;
	}

	for(i = 0; i < ARRAY_SIZE(sdkinfo->devgroup); i++){
		if(sdkinfo->devgroup[i] == NULL){
			continue;
		}
		memcpy(&info->dev_group[n], sdkinfo->devgroup[i], sizeof(cl_dev_group_t));
        log_debug("group id [%u] name[%s] type[%u] count[%u]\n",info->dev_group[n].group_id,info->dev_group[n].name,
                  info->dev_group[n].group_type,info->dev_group[n].dev_cnt);
        n++;
	}	
	
}

static cl_lamp_remote_info_t*  _find_remote_info_by_id(cl_lamp_remote_info_t* lri,u_int32_t cur_cnt,u_int32_t r_id)
{
    cl_lamp_remote_info_t* p = lri;
    u_int32_t i;
    
    if (!p || !cur_cnt) {
        return NULL;
    }
    
    for (i = 0; i < cur_cnt; i++,p++) {
        if (p->r_id == r_id) {
            return p;
        }
    }
    
    return NULL;
}

static void rfgw_build_remote(user_t* user,cl_gw_info_t *info)
{
    u_int32_t lr_cnt = 0;
    cl_lamp_remote_info_t lri[MAX_SLAVE_PER_GW* MAX_DW_LAMP_REMOTE_CNT] = {0};
    slave_t *slave;
    cl_rf_lamp_t * crl;
    u_int8_t i,j;
    cl_lamp_remote_info_t* lrp;
    cl_lamp_remote_key_info* ki;
    
    stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {

		if (!is_led_type(slave)) {
			continue;
		}
		
        crl = &slave->dev_info.rf_stat.dev_priv_data.lamp_info;
        for (i = 0; i < crl->remote_count; i++) {
            if (crl->r_info[i].remote_id != 0) {
                lrp = _find_remote_info_by_id(lri,lr_cnt,crl->r_info[i].remote_id);
                if (!lrp) {
                    lrp = &lri[lr_cnt++];
                    lrp->r_id = crl->r_info[i].remote_id;
                    lrp->lamp_type = RL_LT_MAX;
                }
                if (crl->r_info[i].key_id < MAX_DW_LAMP_KEY_ID) {
                    ki = &lrp->key[crl->r_info[i].key_id&0x3];
                    ki->slave_count++;
                    
                    //calc lamp_type for remote key
                    if (ki->lamp_type >= RL_LT_MAX) {
                        ki->lamp_type = crl->lamp_type;
                    }else{
                        if (ki->lamp_type == RL_LT_WC_AND_RGB) {
                            if (ki->lamp_type != RL_LT_WC_AND_RGB) {
                                ki->lamp_type = crl->lamp_type;
                            }
                        }else if(ki->lamp_type == RL_LT_WC_OR_RGB){
                            if (ki->lamp_type == RL_LT_WC_ONLY) {
                                ki->lamp_type = RL_LT_WC_ONLY;
                            }
                        }
                    }
                    // calc lamp_type for remote
                    if (lrp->lamp_type >= RL_LT_MAX) {
                        lrp->lamp_type = crl->lamp_type;
                    }else{
                        if (lrp->lamp_type == RL_LT_WC_AND_RGB) {
                            if (crl->lamp_type != RL_LT_WC_AND_RGB) {
                                lrp->lamp_type = crl->lamp_type;
                            }
                        }else if(lrp->lamp_type == RL_LT_WC_OR_RGB){
                            if (crl->lamp_type == RL_LT_WC_ONLY) {
                                lrp->lamp_type = RL_LT_WC_ONLY;
                            }
                        }
                    }
                    
                }
            }
        }
    }
    
    if (lr_cnt == 0) {
        return;
    }
    
    info->lr_info = cl_calloc(sizeof(*lrp)*lr_cnt, 1);
    if (!info->lr_info) {
        return;
    }
    
    info->lamp_remote_cnt = lr_cnt;
    memcpy(info->lr_info, lri, sizeof(*lrp)*lr_cnt);
    lrp = info->lr_info;
    for (i = 0; i < lr_cnt; i++,lrp++) {
        for (j = 0; j < sizeof(lrp->key)/sizeof(cl_lamp_remote_key_info); j++) {
            if (lrp->key[j].slave_count > 0) {
                lrp->key[j].slave_handle = cl_calloc(sizeof(u_int32_t)*lrp->key[j].slave_count, 1);
            }
        }
    }
    
    stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
        if (!is_led_type(slave)) {
			continue;
		}        crl = &slave->dev_info.rf_stat.dev_priv_data.lamp_info;
        for (i = 0; i < crl->remote_count; i++) {
            if (crl->r_info[i].remote_id != 0) {
                
                lrp = _find_remote_info_by_id(info->lr_info,lr_cnt,crl->r_info[i].remote_id);
                if (!lrp) {
                    continue;
                }
                
                if (crl->r_info[i].key_id < MAX_DW_LAMP_KEY_ID) {
                    lrp->key[crl->r_info[i].key_id&0x3].slave_handle[lrp->key[crl->r_info[i].key_id&0x3].real_count ++] = slave->handle;
                }
            }
        }
    }
    
    
}

static void rfgw_upgrade_build(cl_gw_info_t *info, rfgw_priv_t *sdkinfo)
{
	int i;

	memcpy(info->upgrade_status, sdkinfo->upgrade_status, sizeof(info->upgrade_status));

	for(i = 0; i < D_T_MAX; i++) {
		if (sdkinfo->upgrade_url[i]) {
			info->upgrade_url[i] = cl_strdup(sdkinfo->upgrade_url[i]);
		} else {
			info->upgrade_url[i]  = cl_strdup("");
		}
	}
}

static void rfgw_comm_build(cl_gw_info_t *info, rfgw_priv_t *sdkinfo)
{
	int len = 0;
	
	info->commpat = sdkinfo->commpat;
	info->channel = sdkinfo->channel;
	info->is_upgrade = sdkinfo->is_upgrade;
	if (sdkinfo->img_cache_num) {
		len = sdkinfo->img_cache_num * sizeof(cl_rfdev_img_cache_info_t);
		info->pimg_cache = cl_calloc(len, 1);
		if (info->pimg_cache) {
			memcpy((void *)info->pimg_cache, (void *)sdkinfo->pimg_cache, len);
			info->img_cache_num = sdkinfo->img_cache_num;
		}
	}
}

static void rfgw_scm_build(cl_gw_info_t *info, rfgw_priv_t *sdkinfo)
{
	memcpy((void *)&info->rfdef_scm_dev, (void *)&sdkinfo->rfdef_scm_dev, sizeof(info->rfdef_scm_dev));
}

static void rfgw_hpinfo_build(cl_gw_info_t *info, rfgw_priv_t *sdkinfo)
{
	memcpy((void *)&info->hpinfo, (void *)&sdkinfo->hpinfo, sizeof(info->hpinfo));
	info->support_hpinfo = sdkinfo->support_hpinfo;
}


static void rfgw_upgrade_status_sync(user_t *user, rfgw_priv_t *sdkinfo)
{
	int i;

	if (!user || !sdkinfo) {
		return;
	}
	
	if (user->rf_up_type == 0) {
		for(i = 0; i < D_T_MAX; i++) {
			if (sdkinfo->upgrade_status[i] == UP_STATUS_UPGRADING) {
				sdkinfo->upgrade_status[i] = 0;
			}
		}
	}
}

// APP能看见的状态数据
void rfgw_build_objs(user_t* user, cl_dev_info_t* ui)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
	cl_gw_info_t *info;
	rfgw_priv_t *sdkinfo;

	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return;
	}

	if (!(ac = sma->sub_ctrl) || user->sub_type != IJ_RFGW) {
		return;
	}
	
	// 仅处理网关相关的数据，如果有的话
	sdkinfo = ac->com_udp_dev_info.device_info;
	if(sdkinfo == NULL) {
		return;
	}

	//同步一下升级状态
	rfgw_upgrade_status_sync(user, sdkinfo);
	
	info = cl_calloc(sizeof(*info), 1);
	if(info == NULL)
		return;
	build_dev_group(info, sdkinfo);
    rfgw_build_remote(user,info);
	rfgw_upgrade_build(info, sdkinfo);
	rfgw_comm_build(info, sdkinfo);
	rfgw_scm_build(info, sdkinfo);
	rfgw_hpinfo_build(info, sdkinfo);
	ui->com_udp_info->device_info = info;

}

int rfgw_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error)
{
	int event = 0;

	log_info("%s, attr=%u, error=%d\n", __FUNCTION__, obj->attr, error);
    if(obj->attr != UCAT_IA_RFGW_TT){
        return 0;
    }
    // 使用通用的，仅表示指令到达网关
    if(error != ERR_NONE)
    {
        event = SAE_COMMON_CTRL_FAILED;
    }
	
	return event;
}


void rfgw_quick_query_info(smart_air_ctrl_t* ac)
{
	// 查询设备列表
	rfgw_query_devlist(ac->sac->user);
    log_debug("rfgw_quick_query_info start query info sn[%012"PRIu64"]\n",ac->sac->user->sn);
	if (ac->sac->user->is_support_spe_up) {
		_rfgw_img_cache_query(ac->sac->user->uc_session);
	}

	// 花瓶网关查询
	if (ac->com_udp_dev_info.device_info && ((rfgw_priv_t*)ac->com_udp_dev_info.device_info)->support_hpinfo == true) {
		sa_query_obj(ac->sac->user->uc_session, UCOT_IA, UCSOT_IA_HPGW, ALL_SUB_ATTR);
	}
}

//处理网关离线事件
void rfgw_proc_gw_offline_hook(user_t *user)
{
    slave_t *slave;
    
    if (!user) {
        return;
    }
   
    //所有设备都离线
    stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
        //设置为未更新数据
        slave->is_update_state = false;
        
        if (slave->is_udp) {
            //如果是绑定的，则设置为离线
            if (slave->status != BMS_UNBIND) {
                slave->status = BMS_BIND_OFFLINE;
            }
        }
    }
}

bool rfgw_proc_notify_hook(cl_notify_pkt_t *pkt, RS *ret)
{
    user_t * user = NULL;
    slave_t * slave = NULL;
    cln_common_info_t *info;
    
    info = (cln_common_info_t *)&pkt->data[0];
    
    if (pkt->type == CLNE_RFGW_GATEWAY) {
        //给网关的指令
        user = lookup_by_handle(HDLT_USER, info->handle);
        if (!user|| !user->is_udp_ctrl || !user->smart_appliance_ctrl ) {
            log_err(false, "rfgw_proc_notify_hook error user handle %08x\n",info->handle);
            *ret = RS_INVALID_PARAM;
            return false;
        }
        
        if(!user->online){
            *ret = RS_OFFLINE;
            return false;
        }
        
        return rfgw_proc_notify(user, pkt, ret);
    }

	
    // 给从设备的指令
    slave = lookup_by_handle(HDLT_SLAVE, info->handle);
    //?? 是否需要检测主设备是否是RF网关呢?
    if (!slave ) {
        user = lookup_by_handle(HDLT_USER, info->handle);
        if (!user) {
            *ret = RS_INVALID_PARAM;
            return false;
        }
        return udp_rf_dev_group_proc_notify(user,pkt,ret);
    }
    
    //检测主设备是否在线
    if(!slave->user || !slave->user->online){
        *ret = RS_OFFLINE;
        return false;
    }
    
    return udp_rf_dev_proc_notify(slave,pkt,ret);
}
