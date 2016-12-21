#include "client_lib.h"
#include "cl_priv.h"
#include "md5.h"
#include "cl_notify.h"
#include "cl_thread.h"
#include "cl_user.h"
#include "cl_intelligent_forward.h"
#include "plug_priv.h"
#include "community_priv.h"
#include "ir_lib.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "wait_server.h"
#include "video_record_timer.h"
#include "cmd_misc.h"
#include "smart_appliance_priv.h"
#include "rfgw_priv.h"

#define	HANDLE_QUERY_MASTER 0x11110000
#define	HANDLE_QUERY_SLAVE 0x11220000

extern int timer_query_master(cl_thread_t *t);
extern int timer_query_slave(cl_thread_t *t);
extern void bmp_free(slave_t *slave);
extern void bmp_slave_copy(slave_t *slave, slave_t *src);
extern bool is_led_type(slave_t *slave);
extern void do_passwd_sync_to_server(user_t *user);
extern void tcp_dev_online(user_t *user);

mod_t *mod_alloc()
{
	mod_t *mod;

	mod = cl_calloc(sizeof(mod_t), 1);
	
	return mod;
}


// 申请内存，初始化数据、链表头
slave_t *slave_alloc()
{
	slave_t *slave;

	slave = cl_calloc(sizeof(slave_t), 1);
	if(slave == NULL)
		return NULL;

	slave->ext_type = 0xFF;
	slave->community = cl_calloc(sizeof(slave_cmt_t), 1);
	if(slave->community == NULL){
		cl_free(slave);
		return NULL;
	}
	slave->community->my_notify_id = 1;

	slave->video = video_alloc(slave);
	if(slave->video == NULL){
		cl_free(slave->community);
		cl_free(slave);
		return NULL;
	}

	slave->plug = plug_alloc(slave);
	if(slave->plug == NULL){
		cl_free(slave->community);
		video_free(slave->video);
		cl_free(slave);
		return NULL;
	}

	slave->dev_info.rfdev = rfdev_alloc();
	if(slave->dev_info.rfdev == NULL){
		cl_free(slave->community);
		video_free(slave->video);
		plug_free(slave->plug);
		cl_free(slave);
		return NULL;
	}
	
	STLC_INIT_LIST_HEAD(&slave->mod_list);
	slave->dev_info.release_desc = NULL;
	slave->dev_info.release_url = NULL;
	slave->dev_info.release_date = NULL;

	slave->comm_timer.timer = NULL;

	return slave;
}

void slave_free(slave_t *slave)
{
	mod_t *pos, *n;
    int i;
    
	if (slave == NULL)
		return;
	
	SAFE_FREE(slave->name);
	SAFE_FREE(slave->phone);
    CL_THREAD_TIMER_OFF(slave->t_rf_code_match);
    CL_THREAD_TIMER_OFF(slave->t_rf_code_match_resend);

    CL_THREAD_TIMER_OFF(slave->t_rf_com_timer_query);

    CL_THREAD_TIMER_OFF(slave->t_big_pkt_send);
	CL_THREAD_TIMER_OFF(slave->t_stat_query);
	bmp_free(slave);
	
	stlc_list_for_each_entry_safe(mod_t, pos, n, &slave->mod_list, link) {
		stlc_list_del(&pos->link);
		SAFE_FREE(pos->name);
		cl_free(pos);
	}

	video_free(slave->video);
	plug_free(slave->plug);
	eq_free(slave);
	SAFE_FREE(slave->community);
	SAFE_FREE(slave->dev_info.clients);
	SAFE_FREE(slave->vendor_id);
	SAFE_FREE(slave->dev_info.release_desc);
	SAFE_FREE(slave->dev_info.release_url);
	SAFE_FREE(slave->dev_info.release_date);
	rfdev_free(slave->dev_info.rfdev);
    udp_rf_dev_free_slave_data(slave);
    SAFE_FREE(slave->match_mana.rf_ir_match.matched_code);
    SAFE_FREE(slave->match_mana.ir_cache.ir_detail);
    SAFE_FREE(slave->match_mana.ir_cache.ir_info);
    SAFE_FREE(slave->match_mana.task_info.data);
    
    for (i = 0; i < MAX_RF_B_PKT_RECV_CACHE_NUM ; i++) {
        SAFE_FREE(slave->frag_recv_cache[i].pkt);
        SAFE_FREE(slave->frag_recv_cache[i].mask);
    }
    
    for (i = 0; i < MAX_RF_B_PKT_SEND_CACHE_NUM ; i++) {
        SAFE_FREE(slave->frag_send_cache[i].pkt);
    }
    
	cl_free(slave);
}

// 不释放链表头
void slave_list_free(struct stlc_list_head *slave)
{
	slave_t *pos, *n;
	
	stlc_list_for_each_entry_safe(slave_t, pos, n, slave, link) {
		stlc_list_del(&pos->link);
		slave_free(pos);
	}

	STLC_INIT_LIST_HEAD(slave);
}

static bool is_rf_led_lamp_same(slave_t *sa, slave_t *sb)
{
	cl_rf_lamp_t *la, *lb;
	u_int8_t i;

	la = &sa->dev_info.rf_stat.dev_priv_data.lamp_info;
	lb = &sb->dev_info.rf_stat.dev_priv_data.lamp_info;
	if((la->lamp_type != lb->lamp_type) ||(la->remote_count != lb->remote_count))
		return false;
	for(i = 0; i < la->remote_count && i < MAX_DW_LAMP_REMOTE_CNT; i++){
		if(la->r_info[i].remote_id != lb->r_info[i].remote_id)
			return false;
		if(la->r_info[i].key_id != lb->r_info[i].key_id)
			return false;
	}
	return true;
}

static bool is_same(struct stlc_list_head *list_a, struct stlc_list_head *list_b)
{
	slave_t *sa, *sb;
	mod_t *ma, *mb;


	if (stlc_list_empty(list_a) != stlc_list_empty(list_b))
		return false;

	if (stlc_list_empty(list_a) && stlc_list_empty(list_b))
		return true;

	// move slave list B to first
	sb = stlc_list_entry(list_b->next, slave_t, link);
	
	stlc_list_for_each_entry(slave_t, sa, list_a, link) {
		// check empty
		if (&sb->link == list_b)
			return false;

		if (sa->sn != sb->sn
			|| (!IS_SAME_STR(sa->name, sb->name))
			|| sa->sub_type != sb->sub_type
			|| sa->status != sb->status
			|| sa->area_id != sb->area_id)
		{
			return false;
		}

		//dianwei led lamp need compare more info
		if(sa->sub_type == IJ_RFGW && is_led_type(sa)){
			if(is_rf_led_lamp_same(sa, sb) != true)
				return false;
		}

		// check sub module
		mb = stlc_list_entry(sb->mod_list.next, mod_t, link);
		
		stlc_list_for_each_entry(mod_t, ma, &sa->mod_list, link) {
			// check empty
			if (&mb->link == &sb->mod_list)
				return false;

			if (ma->mod_id != mb->mod_id || ( ! IS_SAME_STR(ma->name, mb->name)) || ma->flags != mb->flags)
				return false;
			
			mb = stlc_list_entry(mb->link.next, mod_t, link);
		}
		if (&mb->link != &sb->mod_list)
			return false;
		
		// move slave list B to next
		sb = stlc_list_entry(sb->link.next, slave_t, link);
	}
	if (&sb->link != list_b)
		return false;

	return true;
}


void slave_info_copy(user_t *user, struct stlc_list_head *slave_list)
{
	slave_t *slave, *src;
    int i;
	
	if (slave_list == NULL)
		return;

	stlc_list_for_each_entry(slave_t, slave, slave_list, link) {
		src = slave_lookup_by_sn(user, slave->sn);
		if (src == NULL) {
			slave->handle = handle_create(HDLT_SLAVE);
			/*
				这里要将就上层应用，上层应用独立了对象，这里就要重新申请，否则用slave的
			*/
			if (slave->sub_type == IJ_003 || slave->sub_type == IJ_803) { /* 003摄像头与从设备使用一个句柄 */
				slave->video->handle = slave->handle;
			} else { /* USB摄像头重新一个句柄 */
				slave->video->handle = handle_create(HDLT_VIDEO);
			}
			slave->plug->handle = slave->handle;

			if (slave->has_eq) {
				eq_alloc(slave);
			}
            
            if (slave->sn==user->sn ) {
                
            }
            
		} else {
			slave->handle = src->handle;

			video_free(slave->video);
			slave->video = src->video;
			slave->video->slave = slave;
			src->video = NULL;

			plug_free(slave->plug);
			slave->plug = src->plug;
			slave->plug->slave = slave;
			src->plug = NULL;

			eq_free(slave);
			if ((slave->equipment = src->equipment) != NULL) {
				slave->equipment->slave = slave;
			}
			src->equipment = NULL;
            slave->t_rf_code_match = src->t_rf_code_match;
            slave->t_rf_code_match_resend = src->t_rf_code_match_resend;
			slave->t_rf_com_timer_query = src->t_rf_com_timer_query;
            slave->t_big_pkt_send = src->t_big_pkt_send;
            slave->is_support_la = src->is_support_la;
            slave->has_recv_flag_pkt = src->has_recv_flag_pkt;
            memcpy(&slave->match_mana, &src->match_mana, sizeof(slave->match_mana));

			//清空下
			bmp_slave_copy(slave, src);
			
			memcpy(&slave->dev_info, &src->dev_info, sizeof(dev_info_t));
			// slave_free函数会释放这些指针
			// 拷贝后必须置空来避免重复释放
			src->dev_info.clients = NULL;
			src->dev_info.release_desc = NULL;
			src->dev_info.release_url = NULL;
			src->dev_info.release_date = NULL;
			src->dev_info.rfdev = NULL;
            src->dev_info.rf_stat.work_list = NULL;
            src->t_rf_code_match = NULL;
            src->t_rf_code_match_resend = NULL;
			src->t_rf_com_timer_query = NULL;
            src->t_big_pkt_send = NULL;
            src->match_mana.rf_ir_match.matched_code = NULL;
            src->match_mana.ir_cache.ir_detail = NULL;
            src->match_mana.ir_cache.ir_info = NULL;
            src->match_mana.task_info.data = NULL;

			slave->is_support_public_shortcuts_onoff = src->is_support_public_shortcuts_onoff;
			memcpy((void *)&slave->shortcuts_onoff, (void *)&src->shortcuts_onoff, sizeof(slave->shortcuts_onoff));
			
            if (src->status == BMS_BIND_ONLINE && slave->status != BMS_BIND_ONLINE) {
                slave->is_update_state = false;
            }
            
            //if(src->is_update_state)

			slave->bind_error = src->bind_error;
			slave->other_master_sn = src->other_master_sn;

			//这里将原从设备的定时器copy到新设备里
			memcpy((void *)&slave->comm_timer, (void *)&src->comm_timer, sizeof(slave->comm_timer));
			memset((void *)&src->comm_timer, 0, sizeof(src->comm_timer));
            
            for (i = 0; i < MAX_RF_B_PKT_RECV_CACHE_NUM ; i++) {
                src->frag_recv_cache[i].pkt = NULL;
                src->frag_recv_cache[i].mask = NULL;
            }
            
            for (i = 0; i < MAX_RF_B_PKT_SEND_CACHE_NUM ; i++) {
                src->frag_send_cache[i].pkt = NULL;
            }
		}
	}
}

bool slave_replace(user_t *user, struct stlc_list_head *slave_list)
{
	if (is_same(&user->slave, slave_list)) {
		slave_list_free(slave_list);
		return true;
	}

	slave_info_copy(user, slave_list);

	slave_list_free(&user->slave);
	stlc_list_replace_init(slave_list, &user->slave);
    
    if (user->has_area && !user->ac) {
        area_ctrl_alloc(user);
    }
    
    if (user->has_scene && !user->sc) {
        scene_ctrl_alloc(user);
    }
    
	return false;
}

static void parse_mod_remote(slave_t *sp, mod_t *mod)
{
	sp->has_eq = true;
	
	sp->has_rf = !(!(mod->flags & MTF_REMOTE_RF_LEARN));
	sp->has_ir = !(!(mod->flags & MTF_REMOTE_IR_LEARN));
	sp->has_eq_gencode = !(!(mod->flags & MTF_REMOTE_GENCODE));
	sp->has_eq_adjust = !(!(mod->flags & MTF_REMOTE_ADJUST));
	sp->has_eq_width = !(!(mod->flags & MTF_REMOTE_ADJUST));
	sp->has_eq_001e = !(!(mod->flags & MTF_REMOTE_001E));
	sp->has_eq_add_by_json = !(!(mod->flags & MTF_REMOTE_TD));
}

/*
	绑定过程中，中间有几秒钟是显示红色叉叉，就是在获取IP地址的过程，很难看，平滑过渡下
*/
static int check_quick_query(user_t *user, u_int64_t sn, int new_status)
{
	slave_t *slave;

	if ((slave = slave_lookup_by_sn(user, sn)) == NULL) {
		return new_status;
	}
	if (slave->status != new_status && user->quick_query_master > 0) {
		user->quick_query_master = 0;
		CL_THREAD_TIMER_OFF(user->t_timer_query_master);
		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_master, timer_query_master, 
			(void *)user, user->background ? (TIME_QUERY_MASTER*4) : TIME_QUERY_MASTER);
		return slave->status;
	}

	return new_status;
}

/*
	返回是否相同, true为相同
*/
static bool parse_slave_list(user_t *user, u_int8_t *head_pos, int total)
{
	u_int8_t *p, c;
	int i;
	slave_hdr_t *sh;
	module_hdr_t *mh;
	struct stlc_list_head ar_slave;
	slave_t *sp;
	mod_t *mod;

	STLC_INIT_LIST_HEAD(&ar_slave);

	p = head_pos;

	while (p - head_pos < total && total - (p - head_pos) >= sizeof(slave_hdr_t)) {
		// 解析从设备
		sh = (slave_hdr_t *)p;

		sh->sn = ntoh_ll(sh->sn);
		sh->name_len = ntohs(sh->name_len);
		sh->resv = ntohs(sh->resv);
		
		p += sizeof(slave_hdr_t) + sh->name_len;

		sp = slave_alloc();
		sp->user = user;
		stlc_list_add_tail(&sp->link, &ar_slave);
		
		sp->sn = sh->sn;
		if (sh->status == BMS_BIND_OFFLINE) {
			sp->status = check_quick_query(user, sh->sn, sh->status);
		} else {
			sp->status = sh->status;
		}
		sp->sub_type = sh->dev_type;
		
		c = sh->name[sh->name_len];
		sh->name[sh->name_len] = '\0';
		sp->name = cl_strdup(sh->name == NULL ? "" : sh->name);
		sh->name[sh->name_len] = c;

		sprintf(sp->str_sn, "%012"PRIu64"", sp->sn);

		log_debug("%s(%s) -->\n", sp->str_sn, sp->name);

		// 开始解析该从设备模块
		for (i = 0; i < sh->mod_count && p - head_pos < total; i++) {
			mh = (module_hdr_t *)p;
			mh->mod_id = ntohs(mh->mod_id);
			log_debug("org mode id = %u, flags=%u\n", mh->mod_id, mh->flags);
			p += sizeof(module_hdr_t) + mh->name_len;
			
			mod = mod_alloc();
			stlc_list_add_tail(&mod->link, &sp->mod_list);
			
			mod->mod_id = mh->mod_id;
			mod->flags = mh->flags;
			
			c = mh->name[mh->name_len];
			mh->name[mh->name_len] = '\0';
			mod->name = cl_strdup(mh->name == NULL ? "" : mh->name);
			mh->name[mh->name_len] = c;

			if((mod->mod_id >= MID_MJPG_VIDEO_MIN && mod->mod_id <= MID_MJPG_VIDEO_MAX) ||
				(mod->mod_id >= MID_H264_VIDEO_MIN && mod->mod_id <= MID_H264_VIDEO_MAX))
			{
				sp->has_ptz = true;

				if (mod->flags & MTF_VIDEO_FLIP)
					sp->has_video_flip = true;
				
				if (mod->flags & MTF_VIDEO_RECORD) {
					sp->has_video_record = true;
					// 第一次先快速查询
					vrt_quick_query(sp->video);
				}
				
				if(mod->flags & MTF_VIDEO_DETECT)
					sp->has_video_detect = true;
				if(mod->flags & MTF_VIDEO_CUSTOM_DPI)
					sp->has_video_custom = true;
                
                if (mod->flags & MTF_VIDEO_UN_SUPPORT_PTZ) {
                    sp->has_ptz = false;
                }

			} else if (MID_PLUG_MIN <= mod->mod_id && mod->mod_id <= MID_PLUG_MAX) {
				sp->has_plug = true;
				if (mod->flags & MTF_VC_DETECT) {
					sp->has_current_detect = true;
				}
				if (mod->flags & MTF_PLUG_ELECTRIC_STAT) {
					sp->has_electric_stat = true;
				}

				// 第一次先快速查询
				plug_quick_query(sp->plug);
			} else if (mod->mod_id == MID_REMOTE) {
				parse_mod_remote(sp, mod);
			} else if (mod->mod_id == MID_ALARM) {
				sp->has_alarm = true;
				if (mod->flags & MTF_ALARM_SWITCH) {
					sp->has_alarm_swich = true;
				}
			} else if (mod->mod_id == AREA_FUNC_SUPPORT){
                if (sp->sn == user->sn) {
                    user->has_area = true;
                }
            } else if (mod->mod_id == AREA_ID_SUPPORT){
                sp->area_id = mod->flags;
            } else if (mod->mod_id == MID_SCENE_SUPPORT){
                if (sp->sn == user->sn) {
                    user->has_scene = true;
					if (mod->flags & MFK_SCENE_TIMER)
						user->has_scene_timer = true;
					if (mod->flags & MFK_SCENE_LINKAGE_ALARM) {
						user->has_scene_linkage_alarm = true;
						if (user->t_timer_query_link_scene == NULL)
							quick_query_alarm_link_scene(user);
					}
                }
            } else if (mod->mod_id == MID_REMOTE_BD) {
                user->has_db_rf = true;
                //support double rf must support alarm push/short msg single config 
                sp->has_alarm_swich = true;
            } else if (mod->mod_id == MID_COLOR){
                if (mod->flags & MTF_V4L2_COLOR) {
                    sp->has_v4l2_color_setting = true;
                }
            } else if (mod->mod_id == MID_MOTO_ATTRIBUTE){
                if (mod->flags & MTF_MOTO_ATTRI_SPEED) {
                    sp->has_roll_speed_ctrl = true;
                }
            } else if (mod->mod_id == MID_IA_ID) {
            	if (user->ia) {
					user->ia->ia_sub_type =  mod->flags;
				}
			}

			log_debug("slav sn = %"PRIu64"  mod id=%d, name=%s, flags=0x%x\n", sp->sn, mod->mod_id, mod->name, mod->flags);
		}
	}

	return slave_replace(user, &ar_slave);
}

static RS reset_video_ip_list(video_t *video, video_ip_list_t *vil, int total)
{
	int i;
	vil_ele_t *e;
	net_detect_t *nd, *pos;

	SAFE_FREE(video->last_video_ip_list);
	video->slect_p2p = -1;
	CL_THREAD_TIMER_OFF(video->t_net_detect);

	stlc_list_free(net_detect_t, &video->net_detect_list, link);

	if (vil == NULL)
		return RS_OK;

	video->last_video_ip_list = (video_ip_list_t *)cl_malloc(total);
	if (video->last_video_ip_list == NULL) {
		log_err(false, "Out of memory\n");
		return RS_ERROR;
	}
	memcpy(video->last_video_ip_list, vil, total);

	// 按优先级排序
	for (i = 0; i < vil->count; i++) {
		e = (vil_ele_t *)((char *)vil->ele + i*vil->ele_size*4);
		
		nd = cl_calloc(sizeof(net_detect_t), 1);
		nd->m_nIp = ntohl(e->ip);
		nd->m_nPort = ntohs(e->port);
		nd->m_nType = e->type;
		nd->m_nPriority = e->priority;

		stlc_list_for_each_entry(net_detect_t, pos, &video->net_detect_list, link) {
			if (nd->m_nPriority < pos->m_nPriority) {
				// insert nd before pos
				stlc_list_add_prev(&nd->link, &pos->link);
				nd = NULL;
				break;
			}
		}
		// 最低优先级，加到尾部
		if (nd != NULL)
			stlc_list_add_tail(&nd->link, &video->net_detect_list);
	}

	i = 0;
	stlc_list_for_each_entry(net_detect_t, pos, &video->net_detect_list, link) {
		log_debug("  %s NetDetect[%d] ip=%u.%u.%u.%u, port=%u, priority=%u, type=%u\n",
			video->slave->str_sn, i++, IP_SHOW(pos->m_nIp), pos->m_nPort, pos->m_nPriority, pos->m_nType);
	}
	
	CL_THREAD_TIMER_ON(&cl_priv->master, video->t_net_detect, timer_net_detect, video, range_rand(0, 100));
	video->pos_next_query = 0;

	return true;
}

RS parse_video_ip_list(slave_t *slave, u_int8_t *head_pos, int total)
{
	video_ip_list_t *vil;
	video_t *video = slave->video;

	vil = (video_ip_list_t *)head_pos;
	log_debug("parse_video_ip_list, sn=%"PRIu64", count=%u, ele_size=%u, total=%d\n",
		slave->sn, vil->count, vil->ele_size*4, total);
	if ((int)vil->count*vil->ele_size*4 + (int)sizeof(video_ip_list_t) > total) {
		log_debug("!!!!! parse_video_ip_list, Bad lenth\n");
		return RS_ERROR;
	}

	if (video->last_video_ip_list != NULL
		&& video->last_video_ip_list->count == vil->count 
		&& video->last_video_ip_list->ele_size == vil->ele_size
		&& memcmp(video->last_video_ip_list, vil, total) == 0)
	{
		return RS_OK;
	}

	log_debug("Reset %s Video IP List\n", slave->str_sn);
	
	return reset_video_ip_list(video, vil, total);
}

static void callback_misc_a(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;

	log_err(false, "callback_misc_a, type=%d handle=0x%08x, cmd=%u, result=%u\n",
		w->obj_type, w->obj_handle, w->cmd, result);
}

static void callback_bind_slave_info(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;

	log_err(false, "query CMD_BIND_SLAVE_INFO failed, type=%d handle=0x%08x, cmd=%u, result=%u\n",
		w->obj_type, w->obj_handle, w->cmd, result);
}

static u_int32_t mem_get_value(u_int8_t *data, int len)
{
	u_int32_t v = 0;
	int i;

	for (i = 0; i < len; i++) {
		v = (v << 8) | data[i];
	}

	return v;
}

static RS parse_ssid_pwd(slave_t *slave, u_int8_t *data, int len)
{
	misc_ssid_pwd_t *sp;

	sp = (misc_ssid_pwd_t *)data;
	
	if (len < sizeof(misc_ssid_pwd_t) || len < (int)(sizeof(misc_ssid_pwd_t) + sp->count*sizeof(misc_ssid_pwd_item_t))) {
		log_err(false, "parse_ssid_pwd %s data len=%d, but need %u at lease\n",
			slave->str_sn, len, sizeof(misc_ssid_pwd_t));
		return RS_ERROR;
	}

	if (sp->count == 0) {
		slave->dev_info.ap_ssid[0] = '\0';
		slave->dev_info.ap_passwd[0] = '\0';
		return RS_OK;
	}

	sp->item[0].ssid_len = ntohs(sp->item[0].ssid_len);
	memcpy(slave->dev_info.ap_ssid, sp->item[0].ssid, sp->item[0].ssid_len);
	slave->dev_info.ap_ssid[sp->item[0].ssid_len] = '\0';

	sp->item[0].psswd_len = ntohs(sp->item[0].psswd_len);
	memcpy(slave->dev_info.ap_passwd, sp->item[0].psswd, sp->item[0].psswd_len);
	slave->dev_info.ap_passwd[sp->item[0].psswd_len] = '\0';

	return RS_OK;
}

static RS parse_ni(slave_t *slave, u_int8_t *data, int len)
{
	int i;
	misc_ni_t *ni;
	misc_ni_item_t *item;
	dev_info_t *di = &slave->dev_info;

	if (len < sizeof(misc_ni_t)) {
		log_err(false, "parse_ni %s data len=%d, but need %u at lease\n",
			slave->str_sn, len, sizeof(misc_ni_t));
		return RS_ERROR;
	}

	ni = (misc_ni_t *)data;
	for (i = 0; i < ni->count; i++) {
		misc_ni_item_t *dst;
		item = (misc_ni_item_t *)BOFP(&ni->item[0], ni->item_len*i);
		if (item->flags & MNIF_WAN) {
			dst = &di->wan;
		} else { 
			dst = &di->lan;
		}
		memcpy(dst, item, sizeof(misc_ni_item_t));
	}

	return RS_OK;
}

static RS parse_dev_client(slave_t *slave, u_int8_t *data, int len)
{
	misc_client_t *ni;
	dev_info_t *di = &slave->dev_info;

	if (len < sizeof(misc_client_t)) {
		log_err(false, "parse_dev_client %s data len=%d, but need %u at lease\n",
			slave->str_sn, len, sizeof(misc_client_t));
		return RS_ERROR;
	}

	ni = (misc_client_t *)data;
	if (ni->count > 0) {
		if (ni->item_len < sizeof(misc_client_item_t)) {
			log_err(false, "parse_dev_client %s bad item_len=%u. we need %u at lease\n",
				slave->str_sn, ni->item_len, sizeof(misc_client_item_t));
			return RS_ERROR;
		}
		if (len < ni->count*ni->item_len) {
			log_err(false, "parse_dev_client %s bad len=%u. we need %ux%u=%u at lease\n",
				slave->str_sn, len, ni->item_len, ni->count, ni->count*ni->item_len);
			return RS_ERROR;
		}
	}
	
	MEM_REPLACE(di->clients, data, len);

	return RS_OK;
}

RS do_misc_a(user_t *user, pkt_t *pkt)
{
	u_int8_t *p, *head_pos;
	int total;
	misc_hdr_t *mh;
	bool is_same = true;
	net_header_t *hdr;
	slave_t *slave = NULL;
	plug_t *plug = NULL;
	bool notify_plug_base = false;
	bool notify_plug_on_off = false;
	bool notify_online = false, online;
	wait_t *w;

	hdr = (net_header_t *)(pkt->data);
	total = ((net_header_t *)(pkt->data))->param_len;

	head_pos = p = get_pkt_payload(pkt, u_int8_t);

	log_debug("MISC pkt, version=%u, hlen=%u, handle0x%08x\n", hdr->ver, hdr->hlen<<2, hdr->handle);
	if ((w = wait_lookup(hdr->handle)) != NULL) {
		slave = slave_lookup_by_handle(user, w->obj_handle);
		wait_del(hdr->handle);
		if (slave == NULL) {
			log_err(false, "do_misc_a failed: slave 0x%08x is deleted\n", hdr->handle);
			return RS_ERROR;
		}
		plug = slave->plug;
		log_debug("slave=%s, %s\n", slave->str_sn, slave->name);
	} else 	if (hdr->ver >= 2) {
		net_header_v2_t *hv2;
		hv2 = (net_header_v2_t *)hdr;
		slave = slave_lookup_by_sn(user, hv2->sn);
		if (slave == NULL) {
			log_err(false, "do_misc_a, slave=NULL!!!\n");
			return RS_ERROR;
		}
		plug = slave->plug;
		log_debug("do_misc_a, slave=%p, sn=%012"PRIu64", handle=%09u\n", slave, hv2->sn, PKT_HANDLE(pkt));
	}

	while (p - head_pos < total && total - (p - head_pos) >= sizeof(misc_hdr_t)) {
		mh = (misc_hdr_t *)p;
		mh->type = ntohs(mh->type);
		mh->len = ntohs(mh->len);
		p += sizeof(misc_hdr_t) + mh->len;
		
		log_debug("MISC type=0x%x, len=%u\n", mh->type, mh->len);
		if (slave != NULL) log_debug("    clients=%p\n", slave->dev_info.clients);
		
		switch (mh->type) {
		case MT_DEV_ONLINE:
			online = ! ( ! *(u_int32_t *)&mh->data[0]);
			if (user->online != online) {
				user->online = online;
				if (user->online) {
					//tcp设备修改密码登陆后，需要同步密码
					if (user->maybe_need_pd_sync) {
						do_passwd_sync_to_server(user);
					}
					user->maybe_need_pd_sync = false;
					tcp_dev_online(user);
				}
				event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
				event_push(user->callback, user->online ? UE_LOGIN_ONLINE : UE_LOGIN_OFFLINE, user->handle, user->callback_handle);
			}
			notify_online = true;
			event_cancel_merge(user->handle);
			break;
			
		case MT_SLAVE_LIST:
			if ( ! parse_slave_list(user, &mh->data[0], mh->len) ) {
				event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);

				CL_THREAD_OFF(user->t_timer_query_slave);
				CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_slave, timer_query_slave, (void *)user, 1);
			}
			break;

		case MT_VENDOR_ID:
			if (mh->len > 0) {
				mh->data[mh->len - 1] = '\0';
				if (slave && slave->sn != user->sn ) {
					STR_REPLACE(slave->vendor_id, mh->data);
					log_debug("%012"PRIu64" vendor id=%s\n", slave->sn, slave->vendor_id);
				} else {
					STR_REPLACE(user->vendor_id, mh->data);
					log_debug("%s vendor id=%s\n", user->name, user->vendor_id);
				}
			}
			break;

		case MT_VENDOR_URL:
			if (mh->len > 0) {
				mh->data[mh->len - 1] = '\0';
				STR_REPLACE(user->vendor_url, mh->data);
				log_debug("%s vendor url=%s\n", user->name, user->vendor_url);
			}
			break;
			
		case MT_DEV_VERSION:
			if (slave != NULL && mh->len >= sizeof(ms_dev_version_t)) {
				memcpy(&slave->dev_info.version, mh->data, sizeof(ms_dev_version_t));
				if (slave->dev_info.version.soft_version.pad) {
					slave->ext_type = slave->dev_info.version.upgrade_version.pad;
				} else {
					slave->ext_type = 0xFF;
				}
			}
			break;
			
		case MT_DEV_RUN_TIME:
			if (slave != NULL && mh->len >= sizeof(u_int32_t)) {
				slave->dev_info.uptime = ntohl(*(u_int32_t *)mh->data);
				slave->dev_info.query_uptime = get_sec( );
			}
			break;
			
		case MT_DEV_ONLINE_TIME:
			if (slave != NULL && mh->len >= sizeof(u_int32_t)) {
				slave->dev_info.online = ntohl(*(u_int32_t *)mh->data);
				slave->dev_info.query_online = get_sec( );
			}
			break;
			
		case MT_DEV_CONNECT_INTERNET_TIME:
			if (slave != NULL && mh->len >= sizeof(u_int32_t)) {
				slave->dev_info.conn_internet = ntohl(*(u_int32_t *)mh->data);
				slave->dev_info.query_conn_internet = get_sec( );
			}
			break;
			
		case MT_VIDEO_IP_LIST:
			if (slave != NULL)
				parse_video_ip_list(slave, &mh->data[0], mh->len);
			break;

		case MT_PLUG_AC:
			if (plug != NULL) {
				plug->ac = mh->data[0];
				notify_plug_base = true;
			}
			break;

		case MT_PLUG_V:
			if (plug != NULL) {
				plug->v = mh->data[0];
				notify_plug_base = true;
			}
			break;

		case MT_PLUG_T:
			if (plug != NULL) {
				plug->t = mh->data[0];
				notify_plug_base = true;
			}
			break;

		case MT_PLUG_ON:
			if (plug != NULL && plug->on != mh->data[0]) {
				plug->on = mh->data[0];
				notify_plug_on_off = true;
			}
			break;
		case MT_DEV_CPU:
			slave->dev_info.cpu = (u_int16_t)mem_get_value(mh->data, mh->len);
			break;
		case MT_DEV_MEM:
			slave->dev_info.mem = (u_int16_t)mem_get_value(mh->data, mh->len);
			break;
		case MT_DEV_SSIDPW:
			parse_ssid_pwd(slave, mh->data, mh->len);
			break;
		case MT_DEV_NI:
			parse_ni(slave, mh->data, mh->len);
			break;
		case MT_DEV_CLIENT:
			parse_dev_client(slave, mh->data, mh->len);
			break;
		case MT_DEV_GET_TMEP:
			user->if_info.temp = (int16_t)mem_get_value(mh->data, mh->len);
			log_debug("temp:%d\n", user->if_info.temp);
			event_push(user->callback, IF_QUERY_OK, user->handle, user->callback_handle);
			break;
		case MT_DEV_GET_RH:
			user->if_info.rh = (u_int8_t)mem_get_value(mh->data, mh->len);
			log_debug("rh:%d\n", user->if_info.rh);
			event_push(user->callback, IF_QUERY_OK, user->handle, user->callback_handle);
			break;
		case MT_DEV_GET_PM25:
			user->if_info.pm25 = (u_int16_t)mem_get_value(mh->data, mh->len);
			log_debug("pm25:%d\n", user->if_info.pm25);
			event_push(user->callback, IF_QUERY_OK, user->handle, user->callback_handle);

			break;
		case MT_DEV_GET_VOC:
			user->if_info.voc = (u_int16_t)mem_get_value(mh->data, mh->len);
			log_debug("voc:%d\n", user->if_info.voc);
			event_push(user->callback, IF_QUERY_OK, user->handle, user->callback_handle);
			break;
		
		}
	}

	if ( ! notify_online ) {
		if (total == 0 && hdr->handle == HANDLE_QUERY_MASTER && user->online) {
			user->online = false;
			event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
			event_push(user->callback, UE_LOGIN_OFFLINE, user->handle, user->callback_handle);
			event_cancel_merge(user->handle);
		} else if (total > 0 && hdr->handle == HANDLE_QUERY_MASTER && (! user->online )) {
			user->online = true;
			event_push(user->callback, UE_INFO_MODIFY, (user->parent ? user->parent->handle : user->handle), user->callback_handle);
			event_push(user->callback, UE_LOGIN_ONLINE, user->handle, user->callback_handle);
			event_cancel_merge(user->handle);
		} 
	}

	if (notify_plug_base && plug->callback != NULL) {
		event_push(plug->callback, PE_QUERY, slave->plug->handle, plug->callback_handle);
	}
	if (notify_plug_on_off && plug->callback != NULL) {
		event_push(plug->callback, plug->on ? PE_ON : PE_OFF, slave->plug->handle, plug->callback_handle);
	}

	return RS_OK;
}

int timer_query_master(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);
	int i;
	pkt_t *pkt;
	net_bind_info_ctl_t *bic;
	u_int16_t *req;
	u_int16_t req_list[] = {MT_SLAVE_LIST, MT_DEV_ONLINE, MT_VENDOR_ID, MT_VENDOR_URL};

	user->t_timer_query_master = NULL;
	if (user->quick_query_master > 0) {
		user->quick_query_master--;
		i = TIME_N_SECOND(user->quick_query_time);
	} else {
		i = user->background ? (TIME_QUERY_MASTER*4) : TIME_QUERY_MASTER;
	}
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_master, timer_query_master, (void *)user, i);
    
    USER_BACKGROUND_RETURN_CHECK(user);
    
	user_get_nickname(user);
	
	pkt = pkt_new(CMD_MISC_Q, sizeof(req_list), user->ds_type);
	PKT_HANDLE(pkt) = HANDLE_QUERY_MASTER;
	req = get_pkt_payload(pkt, u_int16_t);

	for (i = 0; i < sizeof(req_list)/sizeof(u_int16_t); i++, req++) {
		*req = htons(req_list[i]);
	}
	
	user_add_pkt(user, pkt);

	// CMD_BIND_SLAVE_INFO, 查询绑定信息
	pkt = pkt_new_v2(CMD_BIND_SLAVE_INFO, sizeof(net_bind_info_ctl_t), NHF_TRANSPARENT|NHF_WAIT_REPLY, user->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, user->handle, CMD_BIND_SLAVE_INFO, NULL, callback_bind_slave_info);
	bic = get_pkt_payload(pkt, net_bind_info_ctl_t);
	bic->action = BIND_INFO_Q;
	user_add_pkt(user, pkt);
	
	return 0;	
}

int timer_query_slave(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);
	int i;
	pkt_t *pkt;
	u_int16_t *req;
	slave_t *slave;
	u_int16_t req_list[] = {MT_VIDEO_IP_LIST, MT_DEV_VERSION, MT_VENDOR_ID, MT_DEV_RUN_TIME, MT_DEV_ONLINE_TIME, MT_DEV_CONNECT_INTERNET_TIME};

	log_debug("query slave timer\n");
	
	user->t_timer_query_slave = NULL;
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_slave, timer_query_slave, (void *)user, TIME_QUERY_SLAVE);
    
    USER_BACKGROUND_RETURN_CHECK(user);
	/*
		应该每个从设备都发送哈
	*/
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		// 如果不是绑定在线
		if (slave->status != BMS_BIND_ONLINE) {
			// 离线，查询次数清0，便于下次查询更多信息
			slave->query_count = 0;
			continue;
		}

		pkt = pkt_new_v2(CMD_MISC_Q, sizeof(req_list), NHF_TRANSPARENT|NHF_WAIT_REPLY, slave->sn, user->ds_type);
		PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_MISC_Q, NULL, callback_misc_a);
		req = get_pkt_payload(pkt, u_int16_t);

		for (i = 0; i < sizeof(req_list)/sizeof(u_int16_t); i++, req++) {
			*req = htons(req_list[i]);
		}
		
		user_add_pkt(user, pkt);
		log_debug("query slave %s, %012"PRIu64".\n", slave->str_sn, slave->sn);

		// 每5分钟查一次，另外第一次也查一下
		if ((slave->query_count%5) == 0) {
			query_slave_stat(slave);
		}
		slave->query_count++;
	}

	return 0;	
}

RS query_slave_stat(slave_t *slave)
{
	user_t *user = slave->user;
	int i;
	pkt_t *pkt;
	u_int16_t *req;
	u_int16_t req_list[] = {MT_DEV_RUN_TIME, MT_DEV_ONLINE_TIME, MT_DEV_CONNECT_INTERNET_TIME, 
					MT_DEV_CPU, MT_DEV_MEM, MT_DEV_SSIDPW, MT_DEV_CLIENT, MT_DEV_NI};

	log_debug("query slave stat.\n");
	
	// 如果不是绑定在线
	if (slave->status != BMS_BIND_ONLINE)
		RS_ERROR;

	pkt = pkt_new_v2(CMD_MISC_Q, sizeof(req_list), NHF_TRANSPARENT|NHF_WAIT_REPLY, slave->sn, user->ds_type);
	PKT_HANDLE(pkt) = wait_add(HDLT_SLAVE, slave->handle, CMD_MISC_Q, NULL, callback_misc_a);
	req = get_pkt_payload(pkt, u_int16_t);

	for (i = 0; i < sizeof(req_list)/sizeof(u_int16_t); i++, req++) {
		*req = htons(req_list[i]);
	}
	
	user_add_pkt(user, pkt);

	log_debug("query slave %s statics, %012"PRIu64".\n", slave->str_sn, slave->sn);

	return RS_OK;	
}

