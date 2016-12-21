#include "scene_priv.h"
#include "cl_scene.h"
#include "cl_priv.h"
#include "wait_server.h"

static RS scene_proc_timer_req(int event, cln_scene_t *clns);

static void free_scene_priv(scene_priv_t* sp)
{
    if (!sp) {
        return;
    }
    CL_THREAD_OFF(sp->t_query);
    if (sp->prev_reply) {
        cl_free(sp->prev_reply);
    }
	SAFE_FREE(sp->prev_timer);
    sp->prev_reply = NULL;
    sp->prev_reply_len = 0;
    cl_free(sp);
}

static void free_scene_priv_list(struct stlc_list_head* head)
{
    scene_priv_t* sp,*next;
    stlc_list_for_each_entry_safe(scene_priv_t, sp, next, head, link){
        stlc_list_del_init(&sp->link);
        free_scene_priv(sp);
    }
}

static void free_scene_store_list(struct stlc_list_head* head)
{
    scene_req_store_t* srs,*next;
    stlc_list_for_each_entry_safe(scene_req_store_t, srs, next, head, link){
        stlc_list_del_init(&srs->link);
        cl_free(srs);
    }
}

static void free_scene_ctrl(scene_ctrl_t* sc)
{
    
    if (!sc) {
        return;
    }
    CL_THREAD_OFF(sc->t_query);
    if (sc->prev_reply) {
        cl_free(sc->prev_reply);
    }
    sc->prev_reply = NULL;
    sc->prev_reply_len = 0x0;
    free_scene_priv_list(&sc->scene_list);
    free_scene_store_list(&sc->scene_req_list);
    cl_free(sc);
}

static int timer_user_scene_query(cl_thread_t *t)
{
    pkt_t *pkt;
	user_t *user;
	scene_ctrl_t * sc;
    scene_config_t* sconfig;
    
    user = (user_t*)CL_THREAD_ARG(t);
	sc = user->sc;
    if (!sc) {
        return RS_ERROR;
    }
    
    sc->t_query = NULL;
    if (!sc->query_time_interval) {
        sc->query_time_interval = TIME_SCENE_QUERY;
    }
    
    /*设备不在线，两秒探测一次*/
    if (!user->online) {
        CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_query, timer_user_scene_query, (void *)user, TIME_N_SECOND(2));
        return RS_OK;
    }
    CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_query, timer_user_scene_query, (void *)user,
                       sc->query_time_interval);
    
	USER_BACKGROUND_RETURN_CHECK(user);
    
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = CMD_SCENE_CONFIG;
    sconfig = get_pkt_payload(pkt, scene_config_t);
    sconfig->action = AC_SCENE_QUERY_ALL;
	user_add_pkt(user, pkt);
    
	return 0;
}

static void timer_user_scene_quick_query(user_t* user)
{
    scene_ctrl_t * sc = user->sc;
    if (sc) {
        CL_THREAD_OFF(sc->t_query);
        CL_THREAD_TIMER_ON(&cl_priv->master, sc->t_query, timer_user_scene_query, (void *)user, 0);
    }
}

scene_priv_t* scene_lookup_by_id(scene_ctrl_t* sc,u_int8_t scene_id)
{
    scene_priv_t* sp=NULL;
    
    stlc_list_for_each_entry(scene_priv_t, sp, &sc->scene_list, link){
        if (sp->scene_id == scene_id) {
            return sp;
        }
    }
    return NULL;
}

//static scene_req_store_t* scene_store_lookup_by_time(scene_ctrl_t* sc,u_int32_t create_time)
//{
//    scene_req_store_t* sr;
//    
//    stlc_list_for_each_entry(scene_req_store_t, sr, &sc->scene_req_list, link){
//        if (sr->create_time == create_time) {
//            return sr;
//        }
//    }
//    return NULL;
//}

static scene_req_store_t* scene_store_get_first(scene_ctrl_t* sc)
{
    scene_req_store_t* sr;
    
    if (!stlc_list_empty(&sc->scene_req_list)) {
        sr = stlc_list_first_entry(&sc->scene_req_list, scene_req_store_t, link);
        return sr;
    }
    
    return NULL;
}

static scene_req_store_t* scene_store_lookup_by_handle(scene_ctrl_t* sc,cl_handle_t handle)
{
    scene_req_store_t* sr;
    
    stlc_list_for_each_entry(scene_req_store_t, sr, &sc->scene_req_list, link){
        if (sr->scene_handle == handle) {
            return sr;
        }
    }
    return NULL;
}

static void scene_wait_pkt_timeout_callback(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	scene_priv_t* sp;
    scene_req_store_t* srs;
    user_t * user;
    cl_handle_t t_handle;
    u_int32_t event = 0;
    u_int32_t action = (u_int32_t)w->param;//此参数复用了，添加时为用户句柄
    
    
    sp = lookup_by_handle(HDLT_SCENE, w->obj_handle);
    if (!sp) {
        t_handle = action;
        user = lookup_by_handle(HDLT_USER, t_handle);
        if (user && user->sc) {
            srs = scene_store_lookup_by_handle(user->sc, w->obj_handle);
            if (srs) {//OK,添加失败，估计超时了
                event_push_err(user->callback, SE_SCENE_ADD_FAIL, user->handle, user->callback_handle, ERR_TIMEOUT);
                
                stlc_list_del_init(&srs->link);
                cl_free(srs);
            }
        }
        
        return;
    }
    user = sp->sc->user;
    
    log_err(false, "scene_wait_pkt_timeout_callback, not found user type=%d handle=0x%08x, cmd=%u,action=%d, result=%u\n",w->obj_type, w->obj_handle, w->cmd, action,result);
	

	if (w->cmd == CMD_SCENE_TIMER_Q) {
		switch (action) {
	    case SCENE_TIMER_ADD:
	        event = SE_SCENE_TIMER_ADD_FAIL;
	        break;
	    case SCENE_TIMER_MODIFY:
	        event = SE_SCENE_TIMER_MODIFY_FAIL;
	        break;
	    case SCENE_TIMER_DEL:
	        event = SE_SCENE_TIMER_DEL_FAIL;
	        break;
	    default:
	        log_err(false, "scene_wait_pkt_timeout_callback, unknow cmd=CMD_SCENE_TIMER_Q. result=%d\n", result);
	        break;
		}
	} else {
		switch (action) {
	    case AC_SCENE_MODIFY:
	        event = SE_SCENE_CHANGE_FAIL;
	        break;
	    case AC_SCENE_DELETE:
	        event = SE_SCENE_DEL_FAIL;
	        break;
	    case AC_SCENE_EXEC:
	        event = SE_SCENE_EXEC_FAIL;
	        break;
	        
	    default:
	        log_err(false, "scene_wait_pkt_timeout_callback, unknow cmd=%d. result=%d\n", w->cmd, result);
	        break;
		}
	}
    if (event) {
        event_push_err(user->callback, event, user->handle, user->callback_handle, result);
    }
}

void proc_scene_linkage(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	int len = hdr->param_len;
	net_scene_linkage_t *nslp;

	nslp = get_pkt_payload(pkt, net_scene_linkage_t);

	if (nslp->action != AC_QUERY) {
		int event;

		nslp->err = ntohl(nslp->err);
		switch (nslp->action) {
		case AC_DEL:
			event = nslp->err ? EE_LINKAGE_SCENE_DEL_FAIL : EE_LINKAGE_SCENE_DEL_OK;
			break;
		default: /* AC_MOD */
			event = nslp->err ? EE_LINKAGE_SCENE_MODIFY_FAIL : EE_LINKAGE_SCENE_MODIFY_OK;
			break;
		}
		log_info("%s proc_scene_linkage, action=%u, err=%u, event=%u\n",
			user->name, nslp->action, nslp->err, event);
		event_push(user->callback, event, user->handle, user->callback_handle);
		return;
	}
	
	if (user->prev_scene_linkage_len == len && memcmp(user->prev_scene_linkage, nslp, len) == 0)
		return;

	user->prev_scene_linkage_len = len;
	SAFE_FREE(user->prev_scene_linkage);
	user->prev_scene_linkage = cl_malloc(len);
	memcpy(user->prev_scene_linkage, nslp, len);

	log_info("user %s scene linkage, update (count=%u)\n",
		user->name, nslp->count);
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
}

static int timer_query_alarm_link_scene(cl_thread_t *t)
{
	user_t *user;
	pkt_t *pkt;
	net_scene_linkage_t *nslp;

	user = (user_t *)CL_THREAD_ARG(t);
	user->t_timer_query_link_scene = NULL;
	
    CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_link_scene, 
		timer_query_alarm_link_scene, (void*)user, TIME_N_SECOND(15));
    
    USER_BACKGROUND_RETURN_CHECK(user);

    pkt = pkt_new_v2(CMD_SCENE_LINKAGE, sizeof(net_scene_linkage_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = CMD_SCENE_LINKAGE;
	
    nslp = get_pkt_payload(pkt, net_scene_linkage_t);
	memset(nslp, 0, sizeof(*nslp));
	nslp->action = AC_QUERY;
	nslp->query_type = SCENE_LINKAGE_TYPE_ALARM;

	user_add_pkt(user, pkt);

	return 0;
}

void quick_query_alarm_link_scene(user_t *user)
{
	CL_THREAD_OFF(user->t_timer_query_link_scene);
    CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_query_link_scene, 
		timer_query_alarm_link_scene, (void*)user, 0);
}

static void query_scene_timer(user_t *user, u_int8_t scene_id)
{
	pkt_t *pkt;
	net_scene_timer_hd_t *st;

    pkt = pkt_new_v2(CMD_SCENE_TIMER_Q, sizeof(net_scene_timer_hd_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = CMD_SCENE_TIMER_Q;
    st = get_pkt_payload(pkt, net_scene_timer_hd_t);
	memset(st, 0, sizeof(*st));
    st->action = SCENE_TIMER_QUERY;
    st->scene_id = htons(scene_id);
	user_add_pkt(user, pkt);

}
	
static int timer_single_scene_query(cl_thread_t *t)
{
    cl_handle_t scene_handle;
    scene_priv_t* sp;
    scene_config_t* sconfig;
    pkt_t *pkt;
    user_t* user;
    
    scene_handle = (cl_handle_t)CL_THREAD_ARG(t);
    if (!IS_SAME_HANDLE_TYPE(scene_handle, HDLT_SCENE)) {
		log_err(false, "scene_handle=%08x, != HDLT_SCENE\n", scene_handle);
        return -1;
    }
    
    sp = lookup_by_handle(HDLT_SCENE, scene_handle);
    if (!sp) {
		log_err(false, "not found scene = %08x\n", scene_handle);
        return -1;
    }
    sp->t_query = NULL;
    if (!sp->query_time_interval) {
        sp->query_time_interval = TIME_SINGLE_SCENE_QUERY;
    }
    if (sp->sc->user->online) {
        CL_THREAD_TIMER_ON(&cl_priv->master, sp->t_query, timer_single_scene_query,
                           (void*)sp->scene_handle, sp->query_time_interval);
    }else{
        //两秒检查一次
        CL_THREAD_TIMER_ON(&cl_priv->master, sp->t_query, timer_single_scene_query,
                           (void*)sp->scene_handle, TIME_N_SECOND(2));
        return 0;
    }
    user = sp->sc->user;
    
    USER_BACKGROUND_RETURN_CHECK(user);
    
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = CMD_SCENE_CONFIG;
    sconfig = get_pkt_payload(pkt, scene_config_t);
    sconfig->action = AC_SCENE_QUERY_SINGLE;
    sconfig->scene_id = sp->scene_id;
    sconfig->item_num = 0x1;
	user_add_pkt(user, pkt);

	if (user->has_scene_timer) {
		query_scene_timer(user, sp->scene_id);
	}
    
    return 0;
}

static void timer_single_scene_quick_query(scene_priv_t* priv)
{
    if (priv) {
        CL_THREAD_OFF(priv->t_query);
        CL_THREAD_TIMER_ON(&cl_priv->master, priv->t_query, timer_single_scene_query,
                           (void*)priv->scene_handle, 0);
    }
}

static void scene_do_single_replace(scene_ctrl_t* sc,struct stlc_list_head *sr)
{
    scene_priv_t* src,*dest;
    scene_req_store_t* srs;
    
    stlc_list_for_each_entry(scene_priv_t, dest, sr, link){
        src = scene_lookup_by_id(sc, dest->scene_id);
        if (src) {
            dest->t_query = src->t_query;
            dest->scene_handle = src->scene_handle;
            dest->prev_reply = src->prev_reply;
            dest->prev_reply_len = src->prev_reply_len;
			dest->prev_timer = src->prev_timer;
			dest->prev_timer_len = src->prev_timer_len;
            dest->sc = src->sc;
            
            src->t_query = NULL;
            src->scene_handle = 0;
            src->prev_reply_len = 0;
            src->prev_reply = NULL;
			src->prev_timer = NULL;
        }else{
            //处理新添加的情景模式,和区域相同
            //srs = scene_store_lookup_by_time(sc, ntohl(dest->scene_info.create_time));
            srs = scene_store_get_first(sc);
            if (srs) {
                dest->scene_handle = srs->scene_handle;
                stlc_list_del_init(&srs->link);
                cl_free(srs);
            }else{
                dest->scene_handle = handle_create(HDLT_SCENE);
            }
            dest->sc = sc;
        }
        
        if (!dest->t_query) {
            timer_single_scene_quick_query(dest);
        }
    }
    //释放以前的内存
    free_scene_priv_list(&sc->scene_list);
    stlc_list_replace_init(sr, &sc->scene_list);
    
}

static void scene_parse_query_pkt(scene_ctrl_t* sc,scene_config_t* sconf)
{
    struct stlc_list_head head;
    scene_priv_t* sp;
    scene_t* pscene;
    int i = 0;
    
    
    if (sconf->item_num == 0) {
        free_scene_priv_list(&sc->scene_list);
        return;
    }
    
    STLC_INIT_LIST_HEAD(&head);
    
    pscene = sconf->scenes;
    for (i=0; i<sconf->item_num; i++,pscene++) {
        sp = cl_calloc(sizeof(*sp), 1);
        if (!sp) {
            //内存分配失败
            break;
        }
        sp->scene_id = pscene->scene_id;
        memcpy(&sp->scene_info, pscene, sizeof(scene_t));
        stlc_list_add_tail(&sp->link, &head);

		query_scene_timer(sc->user, sp->scene_id);
    }
    
    scene_do_single_replace(sc, &head);
}

static bool scene_do_query_all_tcp_packet(user_t* user,pkt_t* pkt,net_header_t *hdr,scene_config_t* sconf)
{
    scene_ctrl_t* sc = user->sc;
    int param_len = net_param_len(hdr);
    
    if (!sc) {
        return true;
    }
    
    //相同结果
    if (param_len == sc->prev_reply_len &&
        !memcmp(sconf, sc->prev_reply, param_len)) {
        return true;
    }
    
    if (sc->prev_reply) {
        cl_free(sc->prev_reply);
    }
    sc->prev_reply = NULL;
    sc->prev_reply_len = 0;
    
    sc->prev_reply = cl_malloc(param_len);
    if (sc->prev_reply) {
        memcpy(sc->prev_reply, sconf, param_len);
        sc->prev_reply_len = param_len;
        //解析,发包
        scene_parse_query_pkt(sc,sconf);
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    }
    
    return true;
}

static bool scene_do_query_single_tcp_packet(user_t* user,pkt_t* pkt,net_header_t *hdr,scene_config_t* sconf)
{
    scene_priv_t* sp;
    int param_len = net_param_len(hdr);
    bool has_prev_data = false;
    
    if (!user->sc||sconf->err!=ERR_NONE) {
        return true;
    }
    sp = scene_lookup_by_id(user->sc, sconf->scene_id);
    if (!sp) {
        return true;
    }
    
    if (param_len == sp->prev_reply_len &&
        !memcmp(sconf, sp->prev_reply, param_len)) {
        return true;
    }
    
    if (sp->prev_reply) {
        has_prev_data = true;
        cl_free(sp->prev_reply);
    }
    sp->prev_reply = NULL;
    sp->prev_reply_len = 0;
    sp->event_count = 0;
    
    sp->prev_reply = cl_malloc(param_len);
    if (sp->prev_reply) {
        memcpy(sp->prev_reply, sconf, param_len);
        sp->prev_reply_len = param_len;
        sp->event_count = sconf->item_num;
        //解析,发包
        if (!has_prev_data && param_len<=sizeof(scene_config_t)+sizeof(scene_t)) {
            return true;
        }
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    }
    
    return true;
}

static void proc_scene_timer(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	int len = hdr->param_len;
	net_scene_timer_hd_t *st;
	scene_priv_t *priv;

	st = get_pkt_payload(pkt, net_scene_timer_hd_t);
	st->scene_id = ntohs(st->scene_id);

	if (st->action != SCENE_TIMER_QUERY) {
		int event;

		st->errorcode = ntohl(st->errorcode);
		switch (st->action) {
		case SCENE_TIMER_ADD:
			event = st->errorcode ? SE_SCENE_TIMER_ADD_FAIL : SE_SCENE_TIMER_ADD_OK;
			break;
		case SCENE_TIMER_DEL:
			event = st->errorcode ? SE_SCENE_TIMER_DEL_FAIL : SE_SCENE_TIMER_DEL_OK;
			break;
		default:
			event = st->errorcode ? SE_SCENE_TIMER_MODIFY_FAIL : SE_SCENE_TIMER_MODIFY_OK;
			break;
		}
		log_info("%s proc_scene_timer, action=%u, err=%u, event=%u\n",
			user->name, st->action, st->errorcode, event);
		event_push(user->callback, event, user->handle, user->callback_handle);
		return;
	}
	
	if ((priv = scene_lookup_by_id(user->sc, (u_int8_t)st->scene_id)) == NULL) {
		log_err(false, "proc_scene_timer failed: not found scene id=%u\n", st->scene_id);
		return;
	}
	if (priv->prev_timer_len == len && memcmp(priv->prev_timer, st, len) == 0)
		return;

	priv->prev_timer_len = len;
	SAFE_FREE(priv->prev_timer);
	priv->prev_timer = cl_malloc(len);
	memcpy(priv->prev_timer, st, len);

	log_info("user %s scene-id=%u, update timer(count=%u, next_time=%u)\n",
		user->name, priv->scene_id, st->num, st->next_execute_time);
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
}

bool scene_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
    scene_config_t* sconf;
    scene_t* scene;
    int param_len = net_param_len(hdr);
    u_int32_t event = 0;
	wait_t *w;

	if (hdr->command == CMD_SCENE_TIMER_A) {
		proc_scene_timer(user, pkt, hdr);
		return true;
	}
	if (hdr->command == CMD_SCENE_LINKAGE) {
		proc_scene_linkage(user, pkt, hdr);
		return true;
	}
	
    if (hdr->command != CMD_SCENE_CONFIG || param_len < sizeof(*sconf)) {
        return false;
    }
	
  	w = wait_lookup(PKT_HANDLE(pkt));
  
    sconf = get_pkt_payload(pkt, scene_config_t);
    scene = sconf->scenes;
	switch (sconf->action) {
        case AC_SCENE_QUERY_ALL:
            scene_do_query_all_tcp_packet(user, pkt, hdr, sconf);
            break;
        case AC_SCENE_MODIFY:
            if (sconf->err!=ERR_NONE) {
                if (ntohl(sconf->err)==ERR_SCENE_ID_MAX) {
                    event = SE_SCENE_ADD_FAIL;
                }else{
                    event = SE_SCENE_CHANGE_FAIL;
                }
            }else{
                if (user->sc == NULL || param_len < sizeof(*sconf) + sizeof(*scene)) {
					break;
                } else {
					if (w != NULL && (int)w->param == AC_SCENE_MODIFY) {
						event = SE_SCENE_CHANGE_OK;
					}else{
						 event = SE_SCENE_ADD_OK;
	                }
                }
            }
            break;
        case AC_SCENE_DELETE:
            if (sconf->err!=ERR_NONE) {
                event = SE_SCENE_DEL_FAIL;
            }else{
                event = SE_SCENE_DEL_OK;
            }
            break;
        case AC_SCENE_QUERY_SINGLE:
            scene_do_query_single_tcp_packet(user,pkt,hdr,sconf);
            break;
        case AC_SCENE_EXEC:
            if (sconf->err!=ERR_NONE) {
                event = SE_SCENE_EXEC_FAIL;
            }else{
                event = SE_SCENE_EXEC_OK;
            }
            break;
        default:
            return false;
	}
    
    if (event>0) {
        event_push(user->callback, event, user->handle, user->callback_handle);
    }
    
    return true;
}

static int scene_check_eq_valid_key_count(equipment_t* eq,event_t* code_event)
{
    int num =0;
    u_int8_t index;
    
    if (eq->attr.dev_type == REMOTE_TYPE_BD_LAMP||
        eq->attr.dev_type == REMOTE_TYPE_BD_PLUG||
        eq->attr.dev_type == REMOTE_TYPE_BD_ALARM ||
        eq->attr.dev_type == REMOTE_TYPE_BD_DIMMING_LAMP) {
        return 1;
    }
    
    if (eq->attr.dev_type == REMOTE_TYPE_BD_CURTAIN) {
        return 3;
    }
    
    for (index = 0; index<code_event->obj[0].code_obj.key_num; index++) {
        if (rk_lookup(eq, ntohl(code_event->obj[0].code_obj.key_ids[index]))) {
            num++;
        }
    }
    
    return num;
}

static RS scene_proc_add(cln_scene_t* cln_scene)
{
    user_t* user;
    pkt_t* pkt;
    scene_config_t* sconf;
    scene_t* scene;
    scene_ctrl_t* sc;
    scene_req_store_t* srs;
    
    user = lookup_by_handle(HDLT_USER, cln_scene->user_hand);
    if (!user||!user->sc) {
        return RS_INVALID_PARAM;
    }
    
    sc = user->sc;
    srs = cl_calloc(sizeof(scene_req_store_t), 1);
    if (!srs) {
        return RS_MEMORY_MALLOC_FAIL;
    }
    
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t)+sizeof(scene_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    if (!pkt) {
        cl_free(srs);
        return RS_MEMORY_MALLOC_FAIL;
    }
    srs->scene_handle = handle_create(HDLT_SCENE);
    srs->create_time = get_sec();
    
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, srs->scene_handle,CMD_SCENE_CONFIG , (void*)user->handle,
                               scene_wait_pkt_timeout_callback);
    sconf = (scene_config_t*)get_pkt_payload(pkt, scene_config_t);
    scene = sconf->scenes;
    sconf->action = AC_SCENE_MODIFY;
    sconf->item_num = 0x0;
    scene->create_time = htonl(srs->create_time);
    scene->image_id = cln_scene->img_id;
    strncpy((char*)scene->name, (const char*)cln_scene->name, MAX_SCENE_NAME_LENGTH-1);
    
    if (cln_scene->req_hand) {
        *cln_scene->req_hand = srs->scene_handle;
    }
    
    stlc_list_add_tail(&srs->link, &sc->scene_req_list);
    
    user_add_pkt(user, pkt);
    timer_user_scene_quick_query(user);
    
    return RS_OK;
}

static RS scene_proc_del(cl_handle_t s_handle)
{
    user_t* user;
    pkt_t* pkt;
    scene_priv_t* sp;
    scene_config_t* sconf;
    
    sp = lookup_by_handle(HDLT_SCENE, s_handle);
    if (!sp) {
        return RS_INVALID_PARAM;
    }
    
    user = sp->sc->user;
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, sp->scene_handle,CMD_SCENE_CONFIG , (void*)AC_SCENE_DELETE,
                               scene_wait_pkt_timeout_callback);
    sconf = (scene_config_t*)get_pkt_payload(pkt, scene_config_t);
    sconf->scene_id = sp->scene_id;
    sconf->action = AC_SCENE_DELETE;
    user_add_pkt(user, pkt);
    timer_user_scene_quick_query(user);
    
    return RS_OK;
}

static int scene_get_cl_event_valid_key_count(equipment_t* eq,cl_scene_event_t* cse)
{
    int num = 0;
    u_int32_t pos;
    
    if (eq->attr.dev_type == REMOTE_TYPE_BD_LAMP||
        eq->attr.dev_type == REMOTE_TYPE_BD_PLUG||
        eq->attr.dev_type == REMOTE_TYPE_BD_ALARM ||
        eq->attr.dev_type == REMOTE_TYPE_BD_DIMMING_LAMP) {
        return 1;
    }
    
    if (eq->attr.dev_type == REMOTE_TYPE_BD_CURTAIN) {
        return 3;
    }
    
    for (pos = 0; pos<cse->action_num; pos++) {
        if (rk_lookup(eq, cse->ac_values[pos])!=NULL) {
            num++;
        }
    }
    
    return num;
}
/*
 使用用户的事件填充报文
 返回数据长度
 */
static int cl_event_2_packet(user_t* user, cln_scene_t* cln_scene,
                             void* buffer,int buf_len,int* event_num)
{
    event_t* event;
    cl_scene_event_t* cse;
    int len = 0,num,k,j;
    u_int8_t pos;
    slave_t* slave,*master;
    equipment_t* eq;
    
    memset(buffer, 0, buf_len);
    if (event_num) {
        *event_num = 0;
    }
    if (!cln_scene->events||cln_scene->item_count == 0) {
        return 0;
    }
    
    master = slave_lookup_by_sn(user, user->sn);
    event = (event_t*)buffer;
    j=0;
    for (pos = 0; pos<cln_scene->item_count; pos++) {
        cse = cln_scene->events[pos];
        if (IS_SAME_HANDLE_TYPE(cse->obj_handle,HDLT_EQUIPMENT)) {
            if (master && master->equipment) {
                /*先找电器*/
                eq = eq_lookup_by_handle(master->equipment, cse->obj_handle);
                if (eq) {
                    event->obj_type = htons(OBJ_TYPE_CODE);
                    strncpy((char*)event->name, (const char*)cse->event_name, MAX_EVENT_NAME_LENGTH-1);
                    //检查有效按键个数,需要考虑是否是虚拟的报警设备
                    if (isAlarmDevice(eq)) {
                        event->obj_data_size = htons(sizeof(code_obj_t));
                        event->obj[0].code_obj.key_num = 0x1;
                        event->obj[0].code_obj.local_id = htons(eq->attr.local_id);
                        if (cse->ac_values[0] == CEA_ON) {
                            event->obj[0].code_obj.key_ids[0] = htonl(SCENE_DEV_ACTION_ON);
                        }else{
                            event->obj[0].code_obj.key_ids[0] = htonl(SCENE_DEV_ACTION_OFF);
                        }
                        if (cse->ev_type == CVE_ALARM_SWITCH) {
                            num = sizeof(event_t)+ntohs(event->obj_data_size);
                            event = (event_t*)((char*)event+num);
                            len+=num;
                            j++;
                        }
                    }else{
                        num = scene_get_cl_event_valid_key_count(eq,cse);
                        if (num>0) {
                            event->obj_data_size = htons((u_int16_t)(sizeof(code_obj_t)+(num-1)*sizeof(u_int32_t)));
                            event->obj[0].code_obj.key_num = num;
                            event->obj[0].code_obj.local_id = htons(eq->attr.local_id);
                            
                            for (k=0,num = 0 ; num<(int)cse->action_num; num++) {
                                event->obj[0].code_obj.key_ids[k++] = htonl(cse->ac_values[num]);
                            }
            
                            if (cse->ev_type != CVE_ALARM_SWITCH) {
                                num = sizeof(event_t)+ntohs(event->obj_data_size);
                                event = (event_t*)((char*)event+num);
                                len+=num;
                                j++;
                            }
                        }
                    }
                    
                }
            }
        }else{
            slave = slave_lookup_by_handle(user, cse->obj_handle);
            if (slave) {
                event->obj_type = htons(OBJ_TYPE_SWITCH);
                event->obj[0].switch_obj.mode_id = 0x0;/*先清0*/
                strncpy((char*)event->name, (const char*)cse->event_name, MAX_EVENT_NAME_LENGTH-1);
                event->obj_data_size = htons(sizeof(switch_obj_t));
                event->obj[0].switch_obj.sn = ntoh_ll(slave->sn);
                if (cse->ev_type == CVE_003_MOTION) {
                    if (slave->sub_type == IJ_003 || slave->sub_type == IJ_803) {
                        event->obj[0].switch_obj.mode_id = htons(MID_H264_VIDEO_MIN);
                        event->obj[0].switch_obj.flag = MTF_VIDEO_DETECT;
                        event->obj[0].switch_obj.action = (cse->ac_values[0] == CEA_ON)?SCENE_DEV_ACTION_ON:SCENE_DEV_ACTION_OFF;
                    }
                }else if (cse->ev_type == CVE_003_RECORD){
                    if (slave->sub_type == IJ_003 || slave->sub_type == IJ_803) {
                        event->obj[0].switch_obj.mode_id = htons(MID_H264_VIDEO_MIN);
                        event->obj[0].switch_obj.flag = MTF_VIDEO_RECORD;
                        event->obj[0].switch_obj.action = (cse->ac_values[0] == CEA_ON)?SCENE_DEV_ACTION_ON:SCENE_DEV_ACTION_OFF;
                    }
                } else if (cse->ev_type == CVE_008_CTRL) {
                    if (slave->sub_type == IJ_008) {
                        event->obj[0].switch_obj.mode_id = htons(MID_PLUG_MIN);
                        event->obj[0].switch_obj.action = (cse->ac_values[0] == CEA_ON)?SCENE_DEV_ACTION_ON:SCENE_DEV_ACTION_OFF;
                    }
                }
                
                if (event->obj[0].switch_obj.mode_id) {
                    num = sizeof(event_t)+ntohs(event->obj_data_size);
                    event = (event_t*)((char*)event+num);
                    len+=num;
                    j++;
                }
                
            }
        }
    }
    
    if (event_num) {
        *event_num = j;
    }
    
    
    return len;
}

static RS scene_proc_modify_single(cl_handle_t s_handle,u_int32_t pkt_type,cln_scene_t* cln_scene)
{
    user_t* user;
    pkt_t* pkt;
    scene_priv_t* sp;
    scene_config_t* sconf;
    int size = 0,len,count=0;
    scene_t* scene;
    char buffer[2048];
    
    sp = lookup_by_handle(HDLT_SCENE, s_handle);
    if (!sp) {
        return RS_INVALID_PARAM;
    }
    user = sp->sc->user;
    //先计算包大小
    if (pkt_type == CLNE_SCENE_MODIDY_EVENT) {
        //修改事件的需要重新组包
        size = cl_event_2_packet(user,cln_scene,buffer,sizeof(buffer),&count);
    } else {
        //不修改事件的需要检测以前是否配置了事件
        len = sizeof(scene_config_t)+sizeof(scene_t);
        if (sp->prev_reply && sp->prev_reply_len > len) {
            size = sp->prev_reply_len-len;
            memcpy(buffer, (char *)sp->prev_reply+len, size);
            count = sp->event_count;
        }
    }
    
    if (size<0) {
        return RS_INVALID_PARAM;
    }
    
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t)+size+sizeof(scene_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, sp->scene_handle,CMD_SCENE_CONFIG ,
                               (void*)AC_SCENE_MODIFY, scene_wait_pkt_timeout_callback);
    sconf = (scene_config_t*)get_pkt_payload(pkt, scene_config_t);
    scene = sconf->scenes;
    sconf->action = AC_SCENE_MODIFY;
    sconf->scene_id = sp->scene_id;
    memcpy(sconf->scenes, &sp->scene_info, sizeof(*scene));
    sconf->item_num = count;
    
    //处理非修改事件函数
    if (pkt_type == CLNE_SCENE_CHANGE_IMG) {
        scene->image_id = cln_scene->img_id;
    }else if (pkt_type == CLNE_SCENE_CHANGE_NAME){
        strncpy((char*)scene->name, (const char*)cln_scene->name, MAX_SCENE_NAME_LENGTH-1);
    }
    
    if (size>0) {
        memcpy(scene+1, buffer, size);
    }
   
    user_add_pkt(user, pkt);
    timer_single_scene_quick_query(sp);
    
    return RS_OK;
}

static RS scene_proc_add_3(cl_handle_t user_handle, u_int32_t pkt_type, cln_scene_t* cln_scene)
{
    user_t* user;
    pkt_t* pkt;
    scene_config_t* sconf;
    int size = 0, count=0;
    scene_t* scene;
    char buffer[2048];
    scene_req_store_t* srs;
    
    user = lookup_by_handle(HDLT_USER, cln_scene->user_hand);
    if (user == NULL || user->sc == NULL) {
        return RS_INVALID_PARAM;
    }

    //先计算包大小
    //修改事件的需要重新组包
    size = cl_event_2_packet(user,cln_scene,buffer,sizeof(buffer),&count);
    
    if (size < 0) {
        return RS_INVALID_PARAM;
    }
	
    srs = cl_calloc(sizeof(scene_req_store_t), 1);
    if (!srs) {
        return RS_MEMORY_MALLOC_FAIL;
    }
    srs->scene_handle = handle_create(HDLT_SCENE);
    srs->create_time = get_sec();
    stlc_list_add_tail(&srs->link, &user->sc->scene_req_list);
    if (cln_scene->req_hand) {
        *cln_scene->req_hand = srs->scene_handle;
    }	
    
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t)+size+sizeof(scene_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY,user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, srs->scene_handle, CMD_SCENE_CONFIG ,
                               (void*)user->handle, scene_wait_pkt_timeout_callback);

	sconf = (scene_config_t*)get_pkt_payload(pkt, scene_config_t);
    scene = sconf->scenes;
    sconf->action = AC_SCENE_MODIFY;
    sconf->scene_id = 0;
    sconf->item_num = count;
    
    scene->image_id = cln_scene->img_id;
	scene->create_time = htonl(srs->create_time);
    strncpy((char*)scene->name, (const char*)cln_scene->name, MAX_SCENE_NAME_LENGTH-1);
    
    if (size>0) {
        memcpy(scene+1, buffer, size);
    }
   
    user_add_pkt(user, pkt);
    timer_user_scene_quick_query(user);
    
    return RS_OK;
}

static RS scene_proc_modify(cl_handle_t s_handle, u_int32_t pkt_type, cln_scene_t* cln_scene)
{
    user_t* user;
    pkt_t* pkt;
    scene_priv_t* sp;
    scene_config_t* sconf;
    int size = 0, count=0;
    scene_t* scene;
    char buffer[2048];
    
    sp = lookup_by_handle(HDLT_SCENE, s_handle);
    if (!sp) {
        return RS_INVALID_PARAM;
    }
    user = sp->sc->user;
    //先计算包大小
    //修改事件的需要重新组包
    size = cl_event_2_packet(user,cln_scene,buffer,sizeof(buffer),&count);
    
    if (size < 0) {
        return RS_INVALID_PARAM;
    }
    
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t)+size+sizeof(scene_t), 
			NHF_TRANSPARENT|NHF_WAIT_REPLY,user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, sp->scene_handle,CMD_SCENE_CONFIG ,
                               (void*)AC_SCENE_MODIFY, scene_wait_pkt_timeout_callback);
    sconf = (scene_config_t*)get_pkt_payload(pkt, scene_config_t);
    scene = sconf->scenes;
    sconf->action = AC_SCENE_MODIFY;
    sconf->scene_id = sp->scene_id;
    memcpy(sconf->scenes, &sp->scene_info, sizeof(*scene));
    sconf->item_num = count;
    
    scene->image_id = cln_scene->img_id;
    strncpy((char*)scene->name, (const char*)cln_scene->name, MAX_SCENE_NAME_LENGTH-1);
    
    if (size>0) {
        memcpy(scene+1, buffer, size);
    }
   
    user_add_pkt(user, pkt);
    timer_single_scene_quick_query(sp);
    
    return RS_OK;
}

static void scene_force_set_last(user_t *user, cl_handle_t s_handle)
{
    scene_priv_t* sp;

	// 先偷偷强制修改本地的。设备需要一两秒才能改过来
    stlc_list_for_each_entry(scene_priv_t, sp, &user->sc->scene_list, link){
		if (sp->scene_handle == s_handle) {
			sp->scene_info.flag |= FLAG_SCENE_LAST_ID;
		} else {
			sp->scene_info.flag &= ~FLAG_SCENE_LAST_ID;
		}
    }

	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);

	// 慢查询，避免冲掉本地暂时修改的
    if (user->sc != NULL) {
        CL_THREAD_OFF(user->sc->t_query);
        CL_THREAD_TIMER_ON(&cl_priv->master, user->sc->t_query, timer_user_scene_query, (void *)user, TIME_SCENE_QUERY);
    }
}

RS scene_proc_exec(cl_handle_t s_handle)
{
    user_t* user;
    pkt_t* pkt;
    scene_priv_t* sp;
    scene_config_t* sconf;
    
    sp = lookup_by_handle(HDLT_SCENE, s_handle);
    if (!sp) {
        return RS_INVALID_PARAM;
    }
    
    user = sp->sc->user;
    pkt = pkt_new_v2(CMD_SCENE_CONFIG, sizeof(scene_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, sp->scene_handle,CMD_SCENE_CONFIG , (void*)AC_SCENE_EXEC,
                               scene_wait_pkt_timeout_callback);
    sconf = (scene_config_t*)get_pkt_payload(pkt, scene_config_t);
    sconf->scene_id = sp->scene_id;
    sconf->action = AC_SCENE_EXEC;
    user_add_pkt(user, pkt);

	scene_force_set_last(user, s_handle);
    //timer_user_scene_quick_query(user);
    
    return RS_OK;

}

bool scene_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_scene_t* cln_scene = (cln_scene_t*)pkt->data;
    
	if (pkt->type <= CLNE_SCENE_START || pkt->type >= CLNE_SCENE_END) {
		return false;
	}
    
    switch (pkt->type) {
        case CLNE_SCENE_ADD:
            *ret = scene_proc_add(cln_scene);
            break;
        case CLNE_SCENE_DEL:
            *ret = scene_proc_del(cln_scene->scene_hand);
            break;
        case CLNE_SCENE_CHANGE_NAME:
        case CLNE_SCENE_CHANGE_IMG:
        case CLNE_SCENE_MODIDY_EVENT:
            *ret = scene_proc_modify_single(cln_scene->scene_hand,pkt->type,cln_scene);
            break;
		case CLNE_SCENE_MODIDY:
			if (cln_scene->scene_hand != 0) {
	            *ret = scene_proc_modify(cln_scene->scene_hand,pkt->type,cln_scene);
			} else {
	            *ret = scene_proc_add_3(cln_scene->user_hand,pkt->type,cln_scene);
			}
			break;
        case CLNE_SCENE_EXEC:
            *ret = scene_proc_exec(cln_scene->scene_hand);
            break;

		case CLNE_SCENE_TIMER_ADD:
		case CLNE_SCENE_TIMER_MODIFY:
		case CLNE_SCENE_TIMER_DEL:
			*ret = scene_proc_timer_req(pkt->type, cln_scene);
			break;
            
        default:
            return false;
            break;
    }
    
	return true;
}

RS scene_ctrl_alloc(user_t* user)
{
    scene_ctrl_t * sc;
    
    if (user->sc) {
        return RS_OK;
    }
    
    sc = cl_calloc(sizeof(*sc), 1);
    if (!sc) {
        return RS_ERROR;
    }
    user->sc = sc;
    sc->user = user;
    STLC_INIT_LIST_HEAD(&sc->scene_list);
    STLC_INIT_LIST_HEAD(&sc->scene_req_list);
    
    timer_user_scene_quick_query(user);
    
    return RS_OK;
}

void scene_ctrl_free(user_t* user)
{
    scene_ctrl_t* sc = user->sc;
    user->sc = NULL;
    free_scene_ctrl(sc);
}



static void event_action_2_cl_event_action(cl_scene_event_t* cl_event,event_t* event)
{
    switch_obj_t* so;
    code_obj_t* co;
    u_int16_t m_id;
    u_int32_t action;
    bool valid = false;
    
    if (ntohs(event->obj_type) == OBJ_TYPE_CODE) {
        co = &event->obj[0].code_obj;
        action = ntohl(co->key_ids[0]);
        cl_event->ev_type = CVE_ALARM_SWITCH;
        
        if (action == SCENE_DEV_ACTION_OFF) {
            cl_event->ac_values[0] = CEA_OFF;
        }else{
            cl_event->ac_values[0] = CEA_ON;
        }
    }else if (ntohs(event->obj_type) == OBJ_TYPE_SWITCH) {
        so= &event->obj[0].switch_obj;
        m_id = ntohs(so->mode_id);
        
        if (m_id>=MID_PLUG_MIN && m_id<=MID_PLUG_MAX) {//排插
            cl_event->ev_type = CVE_008_CTRL;
            valid = true;
        }else if (m_id>=MID_H264_VIDEO_MIN && m_id<=MID_H264_VIDEO_MAX) {
            if (so->flag == MTF_VIDEO_RECORD) {//录像
                cl_event->ev_type = CVE_003_RECORD;
                valid = true;
            }else if (so->flag == MTF_VIDEO_DETECT) {//移动侦测
                cl_event->ev_type = CVE_003_MOTION;
                valid = true;
            }
        }
        
        if (valid) {
            if (so->action == SCENE_DEV_ACTION_OFF) {
                cl_event->ac_values[0] = CEA_OFF;
            }else {
                cl_event->ac_values[0] = CEA_ON;
            }
        }
    }
}

void scene_free_timer(cl_scene_t* scene)
{
	int i;
	
	if (scene->timer == NULL)
		return;

	for (i = 0; i < scene->timer_num; i++) {
		SAFE_FREE(scene->timer[i]->name);
		SAFE_FREE(scene->timer[i]);
	}

	cl_free(scene->timer);
}

#define	BIG_NUM	88888888

static int cal_next_time(cl_scene_timer_t *t, int wday, int hour, int minute)
{
	int i, n;
	
	if ( ! t->enable )
		return BIG_NUM;

	for (n = 0; n <= 7; n++) {
		i = (wday + n)%7;
		// 看该天是否置标志
		if ((t->week & (1<<i)) == 0)
			continue;
		// 如果是今天，看是否已经过时了
		if (n == 0 && (hour*60+minute) > (t->hour*60+t->minute))
			continue;

		return n*24*60 + (t->hour - hour)*60 + (t->minute - minute);
	}

	// 不可能这样
	log_err(false, "@@@@@@@@@: cal_next_time error: scene timer: week=0x%02x, hour=%u, minute=%u, now: wday=%u, hour=%u, minute=%u\n",
		t->week, t->hour, t->minute, wday, hour, minute);
	return BIG_NUM;
}

static void sort_scene_timer(cl_scene_t *scene)
{
	int i, k;
	time_t now;
	struct tm *tm;
	int wday, hour, minute;
	int *diff, *idx;
	bool modify;

	diff = cl_calloc(sizeof(int), scene->timer_num);
	idx = cl_calloc(sizeof(int), scene->timer_num);
	
	now = get_sec();
	tm = localtime(&now);
	// Day of week (0 – 6; Sunday = 0).
	wday = tm->tm_wday;
	hour = tm->tm_hour;
	minute = tm->tm_min;

	// 计算与当前时间的差值
	for (i = 0; i < (int)scene->timer_num; i++) {
		diff[i] = cal_next_time(scene->timer[i], wday, hour, minute);
	}

	// 从小到大排序
	for (i = 0; i < (int)scene->timer_num; i++) {
		idx[i] = i;
	}
	for (i = 0; i < (int)scene->timer_num; i++) {
		modify = false;
		for (k = 0; k < (int)scene->timer_num - 1; k++) {
			if (diff[idx[k]] > diff[idx[k + 1]]) {
				int t;
				t= idx[k];
				idx[k] = idx[k + 1];
				idx[k + 1] = t;
				modify = true;
			}
		}
		if ( ! modify )
			break;
	}
	for (i = 0; i < (int)scene->timer_num; i++) {
		if (diff[idx[i]] == BIG_NUM)
			break;

		scene->timer[idx[i]]->sort = i + 1;
	}

	cl_free(diff);
	cl_free(idx);
}

static void scene_timer_utc_2_local(cl_scene_timer_t *timer)
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
	int zone = cl_priv->timezone;
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

static void scene_timer_local_2_utc(cl_scene_timer_t *timer)
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
	int zone = cl_priv->timezone;
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

static void scene_build_timer(cl_scene_t* scene, scene_priv_t* sp)
{
	int i;
	net_scene_timer_hd_t *st = (net_scene_timer_hd_t *)sp->prev_timer;
	
	if (sp->prev_timer_len == 0 || sp->prev_timer == NULL)
		return;

	scene->next_time = ntohs(st->next_execute_time);
	scene->timer_num = st->num;
	scene->timer = (cl_scene_timer_t **)cl_calloc(sizeof(cl_scene_timer_t *), st->num);

	for (i = 0; i < st->num; i++) {
		net_scene_timer_t *src;
		cl_scene_timer_t *dst;

		src = &st->timer[i];
		
		dst = cl_calloc(sizeof(cl_scene_timer_t), 1);
		dst->id = src->id;
		dst->hour = src->hour;
		dst->minute = src->minute;
		dst->week = src->week;
		dst->enable = src->enable;
		dst->name = cl_strdup((const char*)src->name);

		scene_timer_utc_2_local(dst);

		scene->timer[i] = dst;
	}

	sort_scene_timer(scene);
}

static RS scene_proc_timer_req(int event, cln_scene_t *clns)
{
    user_t* user;
    pkt_t* pkt;
    scene_priv_t *sp;
	net_scene_timer_hd_t *st;
	int ste;
	net_scene_timer_t *nst;
	cl_scene_timer_t *cst = (cl_scene_timer_t *)clns->timer;
    
    sp = lookup_by_handle(HDLT_SCENE, clns->scene_hand);
    if (!sp) {
		log_err(false, "scene_proc_timer_req, event=%d, handle=0x%08x, not found\n",
			event, clns->scene_hand);
        return RS_INVALID_PARAM;
    }

	switch (event) {
	case CLNE_SCENE_TIMER_ADD:
		ste = SCENE_TIMER_ADD;
		break;
	case CLNE_SCENE_TIMER_MODIFY:
		ste = SCENE_TIMER_MODIFY;
		break;
	case CLNE_SCENE_TIMER_DEL:
		ste = SCENE_TIMER_DEL;
		break;
	default:
		return RS_NOT_SUPPORT;
	}
    
    user = sp->sc->user;
    pkt = pkt_new_v2(CMD_SCENE_TIMER_Q, sizeof(net_scene_timer_hd_t) + sizeof(net_scene_timer_t),
					NHF_TRANSPARENT|NHF_WAIT_REPLY,
                    user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_SCENE, sp->scene_handle, CMD_SCENE_TIMER_Q, (void*)ste,
                               scene_wait_pkt_timeout_callback);
    st = (net_scene_timer_hd_t*)get_pkt_payload(pkt, net_scene_timer_hd_t);
	st->version = 0;
    st->action = ste;
    st->scene_id = htons(sp->scene_id);
	st->num = 1;
	
	scene_timer_local_2_utc(cst);
	
	nst = &st->timer[0];
	nst->id = cst->id;
	nst->enable = cst->enable;
	nst->hour = cst->hour;
	nst->minute = cst->minute;
	nst->week = cst->week;
	strncpy((char*)nst->name, cst->name == NULL ? "" : (char*)cst->name, sizeof(nst->name));
	nst->name[sizeof(nst->name) - 1] = '\0';

	
	log_info("scene_proc_timer_req, event=%d, ste=%u, scene_id=%u, timer id=%u, name=%s\n",
		event, ste, sp->scene_id, nst->id, nst->name);
	
    user_add_pkt(user, pkt);
	
	query_scene_timer(user, sp->scene_id);
    
    return RS_OK;
}

///////////////////////////////////

static void scene_build_single_object(user_t* user,cl_scene_t* scene,
				scene_priv_t* sp, scene_config_t* sconfig)
{
    u_int8_t num;
    scene_t* si;
    event_t* eo;
    equipment_t* eq;
    cl_scene_event_t* cl_event;
    slave_t* master,*slave;
    int compled_num = 0;
    int size,key_num;
    
    cl_assert(scene && sp);
    
    si = &sp->scene_info;
    scene->scene_handle = sp->scene_handle;
    strncpy((char*)scene->scene_name, (const char*)si->name, MAX_SCENE_NAME_LENGTH-1);
    scene->create_time = ntohl(si->create_time);
    scene->img_resv = si->image_id;
    scene->event_count = 0;
    scene->scene_id = si->scene_id;
	if (si->flag & FLAG_SCENE_LAST_ID) {
		scene->last_executed = true;
	}
    
    //用户或用户序列号错误或者无数据
    if (!user||!user->sn||!sconfig) {
        return;
    }
    //主设备
    master = slave_lookup_by_sn(user,user->sn);
    si = sconfig->scenes;
    eo = (event_t*)(si+1);
    
    if (sconfig->item_num>0x1) {
        int cc =0;
        cc++;
    }
    
    for (num = 0; num < sconfig->item_num; num++) {
        
        if (ntohs(eo->obj_type) == OBJ_TYPE_SWITCH) {
            slave = slave_lookup_by_sn(user, ntoh_ll(eo->obj[0].switch_obj.sn));
            if (slave) {
                cl_event = cl_calloc(sizeof(cl_scene_event_t), 1);
                if (!cl_event) {
                    break;
                }
                cl_event->enent_type = EM_ET_DEVICE_FUNC;
                cl_event->obj_handle = slave->handle;
                strncpy(cl_event->event_name, (const char*)eo->name, MAX_EVENT_NAME_LENGTH-1);
                cl_event->action_num = 0x1;
                
                event_action_2_cl_event_action(cl_event,eo);
                
                scene->events[compled_num++] = cl_event;
            }
        }else if(ntohs(eo->obj_type) == OBJ_TYPE_CODE){
            if (master && master->equipment) {
                eq = eq_lookup(master->equipment, ntohs(eo->obj[0].code_obj.local_id));
                if (eq) {
                    if ( isAlarmDevice(eq) ) {
                        // 报警设备
                        cl_event = cl_calloc(sizeof(cl_scene_event_t), 1);
                        if (!cl_event) {
                            break;
                        }
                        cl_event->enent_type = EM_ET_DEVICE_FUNC;
                        cl_event->obj_handle = eq->handle;
                        strncpy(cl_event->event_name, (const char*)eo->name, MAX_EVENT_NAME_LENGTH-1);
                        cl_event->action_num = 0x1;
                        cl_event->ev_type = CVE_ALARM_SWITCH;
                        event_action_2_cl_event_action(cl_event,eo);
                        
                        scene->events[compled_num++] = cl_event;
                    }else{
                        // 一般电器
                        key_num = scene_check_eq_valid_key_count(eq,eo);
                        if (key_num>0) {//必须有有效的key
                            cl_event = cl_calloc(sizeof(cl_scene_event_t)+(key_num-1)*sizeof(u_int32_t), 1);
                            if (!cl_event) {
                                break;
                            }
                            //拷贝数据
                            cl_event->enent_type = EM_ET_EQUIPMENT_KEY;
                            cl_event->obj_handle = eq->handle;
                            strncpy(cl_event->event_name, (const char*)eo->name, MAX_EVENT_NAME_LENGTH-1);
                            cl_event->action_num = key_num;
                            for (key_num = 0; key_num<(int)cl_event->action_num; key_num++) {
                                cl_event->ac_values[key_num] = ntohl(eo->obj[0].code_obj.key_ids[key_num]);
                            }
                            
                            scene->events[compled_num++] = cl_event;
                        }
                    }
                }
            }
        }

        size = (ntohs(eo->obj_data_size)+sizeof(event_t));
        eo = (event_t*)((char*)eo+size);
    }
    
    scene->event_count = compled_num;

	scene_build_timer(scene, sp);
}

void scene_build_objs(user_t* user,cl_dev_info_t* ui)
{
    scene_priv_t* sp;
    cl_scene_t *s_scene,**scenes;
    scene_config_t* sconfig;
    int count = 0,complete = 0;
    int size;
    //没有数据
    if (!user||!user->sc||stlc_list_empty(&user->sc->scene_list)) {
        return;
    }
    //计算数量
    stlc_list_count(count, &user->sc->scene_list);
    scenes = cl_calloc(sizeof(cl_scene_t*), count);
    if (!scenes) {
        return;
    }
    //输出
    stlc_list_for_each_entry(scene_priv_t, sp, &user->sc->scene_list, link){
        sconfig = (scene_config_t*)sp->prev_reply;
        size = sizeof(*s_scene);
        if (sconfig) {
            size += sizeof(cl_scene_event_t*)*sconfig->item_num;
        }
        s_scene = cl_calloc(size, 1);
        if (!s_scene) {
            break;
        }
        scene_build_single_object(user,s_scene,sp,sconfig);
        scenes[complete++]=s_scene;
    }
    
    ui->scenes = scenes;
    ui->num_scene = complete;
}
