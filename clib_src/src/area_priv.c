#include "area_priv.h"
#include "cl_area.h"
#include "cl_priv.h"
#include "ds_proto.h"
#include "equipment_priv.h" //查询电器句柄
#include "wait_server.h"
#include "cmd_misc.h"


static void free_area_priv(area_priv_t* ap)
{
    if (!ap) {
        return;
    }
    CL_THREAD_OFF(ap->t_query);
    if (ap->prev_reply) {
        cl_free(ap->prev_reply);
    }
    ap->prev_reply = NULL;
    ap->prev_reply_len = 0;
    cl_free(ap);
}

static void free_area_priv_list(struct stlc_list_head* head)
{
    area_priv_t* ap,*next;
    stlc_list_for_each_entry_safe(area_priv_t, ap, next, head, link){
        stlc_list_del_init(&ap->link);
        free_area_priv(ap);
    }
}

static void free_area_store_list(struct stlc_list_head* head)
{
    area_req_store_t* ars,*next;
    stlc_list_for_each_entry_safe(area_req_store_t, ars, next, head, link){
        stlc_list_del_init(&ars->link);
        cl_free(ars);
    }
}

static void free_area_ctrl(area_ctrl_t* ac)
{
    
    if (!ac) {
        return;
    }
    CL_THREAD_OFF(ac->t_query);
    if (ac->prev_reply) {
        cl_free(ac->prev_reply);
    }
    ac->prev_reply = NULL;
    ac->prev_reply_len = 0x0;
    free_area_priv_list(&ac->area_list);
    free_area_store_list(&ac->area_req_list);
    cl_free(ac);
}

static int timer_user_area_query(cl_thread_t *t)
{
    pkt_t *pkt;
	user_t *user;
	area_ctrl_t * ac;
    area_config_t* aconfig;
    
    user = (user_t*)CL_THREAD_ARG(t);
	ac = user->ac;
    if (!ac) {
        return RS_ERROR;
    }
    
    ac->t_query = NULL;
    if (!ac->query_time_interval) {
        ac->query_time_interval = TIME_AREA_QUERY;
    }
    
    /*设备不在线，两秒探测一次*/
    if (!user->online) {
        CL_THREAD_TIMER_ON(&cl_priv->master, ac->t_query, timer_user_area_query, (void *)user, TIME_N_SECOND(2));
        return RS_OK;
    }
    CL_THREAD_TIMER_ON(&cl_priv->master, ac->t_query, timer_user_area_query, (void *)user,
                       ac->query_time_interval);
    
	USER_BACKGROUND_RETURN_CHECK(user);
    
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = CMD_AREA_CONFIG;
    aconfig = get_pkt_payload(pkt, area_config_t);
    aconfig->action = AC_AREA_QUERY_ALL;
	user_add_pkt(user, pkt);
    
	return 0;
}

static void timer_user_area_quick_query(user_t* user)
{
    area_ctrl_t * ac = user->ac;
    if (ac) {
        CL_THREAD_OFF(ac->t_query);
        CL_THREAD_TIMER_ON(&cl_priv->master, ac->t_query, timer_user_area_query, (void *)user, 0);
    }
}

RS area_ctrl_alloc(user_t* user)
{
    area_ctrl_t * ac;
    
    if (user->ac) {
        return RS_OK;
    }
    
    ac = cl_calloc(sizeof(*ac), 1);
    if (!ac) {
        return RS_ERROR;
    }
    user->ac = ac;
    ac->user = user;
    STLC_INIT_LIST_HEAD(&ac->area_list);
    STLC_INIT_LIST_HEAD(&ac->area_req_list);
    timer_user_area_quick_query(user);
    
    return RS_OK;
}

void area_ctrl_free(user_t* user)
{
    area_ctrl_t* ac = user->ac;
    user->ac = NULL;
    free_area_ctrl(ac);
}
/*--------------------------------------------------------------------------------------------*/
static area_priv_t* area_lookup_by_id(area_ctrl_t* ac,u_int8_t area_id)
{
    area_priv_t* ap=NULL;
    
    stlc_list_for_each_entry(area_priv_t, ap, &ac->area_list, link){
        if (ap->area_id == area_id) {
            return ap;
        }
    }
    return NULL;
}

//static area_req_store_t* area_store_lookup_by_time(area_ctrl_t* ac,u_int32_t create_time)
//{
//    area_req_store_t* ar;
//    
//    stlc_list_for_each_entry(area_req_store_t, ar, &ac->area_req_list, link){
//        if (ar->create_time == create_time) {
//            return ar;
//        }
//    }
//    return NULL;
//}

static area_req_store_t* area_store_get_first(area_ctrl_t* ac)
{
    area_req_store_t* ar;
    
    if (!stlc_list_empty(&ac->area_req_list)) {
        ar = stlc_list_first_entry(&ac->area_req_list, area_req_store_t, link);
        return ar;
    }
    
    return NULL;
}

static area_req_store_t* area_store_lookup_by_handle(area_ctrl_t* ac,cl_handle_t handle)
{
    area_req_store_t* ar;
    
    stlc_list_for_each_entry(area_req_store_t, ar, &ac->area_req_list, link){
        if (ar->area_handle == handle) {
            return ar;
        }
    }
    return NULL;
}
//输出单个区域信息
static void area_build_single_object(user_t* user,cl_area_t* area,area_priv_t* ap,area_config_t* aconfig)
{
    u_int8_t num;
    area_t* ai;
    area_obj_t* ao;
    equipment_t* eq;
    slave_t* master,*slave;
    int compled_num = 0;
    int size;
	
    cl_assert(area && ap);
    
    ai = &ap->area_info;
    if (aconfig) {
        ai = aconfig->areas;
    }
    area->area_handle = ap->area_handle;
    strncpy((char*)area->area_name, (const char*)ai->name, MAX_AREA_NAME_LENGTH-1);
    area->create_time = ntohl(ai->create_time);
    area->img_resv = ai->image_id;
    area->item_count = 0;
    area->area_id = ai->area_id;
    
    //用户或用户序列号错误
    if (!user||!user->sn||!aconfig) {
        return;
    }
    //主设备
    master = slave_lookup_by_sn(user,user->sn);
    ai = aconfig->areas;
    ao = (area_obj_t*)(ai+1);
    ap->eq_obj_num = ap->dev_obj_num = 0x0;
    for (num = 0; num < aconfig->item_num; num++) {
        if (ao->obj_type == AREA_OBJ_TYPE_DEV) {
            slave = slave_lookup_by_sn(user, ntoh_ll(ao->obj_sub[0].dev_obj.sn));
            if (slave) {
                area->items[compled_num++] = slave->handle;
            }
            ap->dev_obj_num++;
        }else if(ao->obj_type == AREA_OBJ_TYPE_REMOTE){
            if (master && master->equipment) {
                eq = eq_lookup(master->equipment, ntohs(ao->obj_sub[0].remote_obj.local_id));
                if (eq) {
                    area->items[compled_num++] = eq->handle;
                }
            }
            ap->eq_obj_num++;
        }
        size = (ntohs(ao->obj_data_len)+sizeof(area_obj_t));
        ao = (area_obj_t*)((char*)ao+size);
    }

    area->item_count = compled_num;
}

void area_build_objs(user_t* user,cl_dev_info_t* ui)
{
    area_priv_t* ap;
    cl_area_t *s_area,**areas;
    area_config_t* aconfig;
    int count = 0,complete = 0;
    int size;
    //没有数据
    if (!user||!user->ac||stlc_list_empty(&user->ac->area_list)) {
        return;
    }
    //计算数量
    stlc_list_count(count, &user->ac->area_list);
    areas = cl_calloc(sizeof(cl_area_t*), count);
    if (!areas) {
        return;
    }
    //输出
    stlc_list_for_each_entry(area_priv_t, ap, &user->ac->area_list, link){
        aconfig = (area_config_t*)ap->prev_reply;
        size = sizeof(*s_area);
        if (aconfig) {
            size += sizeof(cl_handle_t)*aconfig->item_num;
        }
        s_area = cl_calloc(size, 1);
        if (!s_area) {
            break;
        }
        area_build_single_object(user,s_area,ap,aconfig);
        areas[complete++]=s_area;
    }
    
    ui->areas = areas;
    ui->num_area = complete;
    
}

static int timer_single_area_query(cl_thread_t *t)
{
    cl_handle_t area_handle;
    area_priv_t* ap;
    area_config_t* aconfig;
    pkt_t *pkt;
    user_t* user;
    
    area_handle = (cl_handle_t)CL_THREAD_ARG(t);
    if (!IS_SAME_HANDLE_TYPE(area_handle, HDLT_AREA)) {
        return -1;
    }
    
    ap = lookup_by_handle(HDLT_AREA, area_handle);
    if (!ap) {
        return -1;
    }
    ap->t_query = NULL;
    if (!ap->query_time_interval) {
        ap->query_time_interval = TIME_SINGLE_AREA_QUERY;
    }
    if (ap->ac->user->online) {
        CL_THREAD_TIMER_ON(&cl_priv->master, ap->t_query, timer_single_area_query,
                           (void*)ap->area_handle, ap->query_time_interval);
    }else{
        //两秒检查一次
        CL_THREAD_TIMER_ON(&cl_priv->master, ap->t_query, timer_single_area_query,
                           (void*)ap->area_handle, TIME_N_SECOND(2));
        return 0;
    }
    user = ap->ac->user;
    
    USER_BACKGROUND_RETURN_CHECK(user);
    
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
	PKT_HANDLE(pkt) = CMD_AREA_CONFIG;
    aconfig = get_pkt_payload(pkt, area_config_t);
    aconfig->action = AC_AREA_QUERY_SINGLE;
    aconfig->area_id = ap->area_id;
    aconfig->item_num = 0x1;
	user_add_pkt(user, pkt);
    
    return 0;
}

static void timer_single_area_quick_query(area_priv_t* priv)
{
    if (priv) {
        CL_THREAD_OFF(priv->t_query);
        CL_THREAD_TIMER_ON(&cl_priv->master, priv->t_query, timer_single_area_query, (void*)priv->area_handle, 0);
    }
}

static void area_do_single_replace(area_ctrl_t* ac,struct stlc_list_head *ar)
{
    area_priv_t* src,*dest;
    area_req_store_t* ars;
    
    stlc_list_for_each_entry(area_priv_t, dest, ar, link){
        src = area_lookup_by_id(ac, dest->area_id);
        if (src) {
            dest->t_query = src->t_query;
            dest->area_handle = src->area_handle;
            dest->prev_reply = src->prev_reply;
            dest->prev_reply_len = src->prev_reply_len;
            dest->ac = src->ac;
            
            src->t_query = NULL;
            src->area_handle = 0;
            src->prev_reply_len = 0;
            src->prev_reply = NULL;
        }else{
            //处理新添加的区域,设备端区域处理有bug，只能按序处理了
            
//            ars = area_store_lookup_by_time(ac, ntohl(dest->area_info.create_time));
            ars = area_store_get_first(ac);
            if (ars) {
                dest->area_handle = ars->area_handle;
                stlc_list_del_init(&ars->link);
                cl_free(ars);
            }else{
                dest->area_handle = handle_create(HDLT_AREA);
            }
            dest->ac = ac;
        }
        
        if (!dest->t_query) {
            timer_single_area_quick_query(dest);
        }
    }
    //释放以前的内存
    free_area_priv_list(&ac->area_list);
    stlc_list_replace_init(ar, &ac->area_list);
    
}

static void area_parse_query_pkt(area_ctrl_t* ac,area_config_t* aconf)
{
    struct stlc_list_head head;
    area_priv_t* ap;
    area_t* parea;
    int i = 0;
    
    
    if (aconf->item_num == 0) {
        free_area_priv_list(&ac->area_list);
        return;
    }
    
    STLC_INIT_LIST_HEAD(&head);
    
    parea = aconf->areas;
    for (i=0; i<aconf->item_num; i++,parea++) {
        ap = cl_calloc(sizeof(*ap), 1);
        if (!ap) {
            //内存分配失败
            break;
        }
        ap->area_id = parea->area_id;
        memcpy(&ap->area_info, parea, sizeof(area_t));
        stlc_list_add_tail(&ap->link, &head);
    }
    
    area_do_single_replace(ac, &head);
}

static bool area_do_query_all_tcp_packet(user_t* user,pkt_t* pkt,net_header_t *hdr,area_config_t* aconf)
{
    area_ctrl_t* ac = user->ac;
    int param_len = net_param_len(hdr);
    
    if (!ac) {
        return true;
    }
    
    //相同结果
    if (param_len == ac->prev_reply_len &&
        !memcmp(aconf, ac->prev_reply, param_len)) {
        return true;
    }
    
    if (ac->prev_reply) {
        cl_free(ac->prev_reply);
    }
    ac->prev_reply = NULL;
    ac->prev_reply_len = 0;
    
    ac->prev_reply = cl_malloc(param_len);
    if (ac->prev_reply) {
        memcpy(ac->prev_reply, aconf, param_len);
        ac->prev_reply_len = param_len;
        //解析,发包
        area_parse_query_pkt(ac,aconf);
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    }
    
    return true;
}

static bool area_do_query_single_tcp_packet(user_t* user,pkt_t* pkt,net_header_t *hdr,area_config_t* aconf)
{
    area_priv_t* ap;
    int param_len = net_param_len(hdr);
    bool has_prev_data = false;
    
    if (!user->ac||aconf->err!=ERR_NONE) {
        return true;
    }
    ap = area_lookup_by_id(user->ac, aconf->area_id);
    if (!ap) {
        return true;
    }
    
    if (param_len == ap->prev_reply_len &&
        !memcmp(aconf, ap->prev_reply, param_len)) {
        return true;
    }
    
    if (ap->prev_reply) {
        has_prev_data = true;
        cl_free(ap->prev_reply);
    }
    ap->prev_reply = NULL;
    ap->prev_reply_len = 0;
    
    ap->prev_reply = cl_malloc(param_len);
    if (ap->prev_reply) {
        memcpy(ap->prev_reply, aconf, param_len);
        ap->prev_reply_len = param_len;
        //解析,发包
        if (!has_prev_data && param_len<=sizeof(area_config_t)+sizeof(area_t)) {
            return true;
        }
        event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    }
    
    return true;
}

static void area_wait_pkt_timeout_callback(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	area_priv_t* ap;
    area_req_store_t* ars;
    user_t * user;
    cl_handle_t t_handle;
    u_int32_t event = 0;
    u_int32_t action = (u_int32_t)w->param;//此参数复用了，添加时为用户句柄
    
    
    ap = lookup_by_handle(HDLT_AREA, w->obj_handle);
    if (!ap) {
        t_handle = action;
        user = lookup_by_handle(HDLT_USER, t_handle);
        if (user && user->ac) {
            ars = area_store_lookup_by_handle(user->ac, w->obj_handle);
            if (ars) {//OK,添加失败，估计超时了
                event_push_err(user->callback, AE_AREA_ADD_FAIL, user->handle, user->callback_handle, ERR_TIMEOUT);
                
                stlc_list_del_init(&ars->link);
                cl_free(ars);
            }
        }
        
        return;
    }
    user = ap->ac->user;
    
    log_err(false, "area_wait_pkt_timeout_callback, not found user type=%d handle=0x%08x, cmd=%u,action=%d, result=%u\n",w->obj_type, w->obj_handle, w->cmd, action,result);
	
    
	switch (action) {
        case AC_AREA_MODIFY:
            event = AE_AREA_CHANGE_FAIL;
            break;
        case AC_AREA_DELETE:
            event = AE_AREA_DEL_FAIL;
            break;
        case AC_AREA_MODIFY_SINGLE:
            event = AE_AREA_CHANGE_FAIL;
            break;
            
        default:
            log_err(false, "area_wait_pkt_timeout_callback, unknow cmd=%d. result=%d\n", w->cmd, result);
            break;		
	}
    if (event) {
        event_push_err(user->callback, event, user->handle, user->callback_handle, ERR_TIMEOUT);
    }
}

bool area_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
    area_config_t* aconf;
    area_t* area;
    int param_len = net_param_len(hdr);
    u_int32_t event = 0;
	wait_t *w;

    if (hdr->command != CMD_AREA_CONFIG || param_len < sizeof(*aconf)) {
        return false;
    }

	w = wait_lookup(PKT_HANDLE(pkt));
    
    aconf = get_pkt_payload(pkt, area_config_t);
	switch (aconf->action) {
        case AC_AREA_QUERY_ALL:
            area_do_query_all_tcp_packet(user,pkt,hdr,aconf);
            break;
        case AC_AREA_MODIFY:
            if (aconf->err!=ERR_NONE) {
                if (ntohl(aconf->err)==ERR_AREA_ID_MAX) {
                    event = AE_AREA_ADD_FAIL;
                }else{
                    event = AE_AREA_CHANGE_FAIL;
                }
            }else{
                area = aconf->areas;
                if (!user->ac||param_len<sizeof(*aconf)+sizeof(*area)) {
                    event = AE_AREA_CHANGE_FAIL;
                } else {
                	if (w != NULL && (int)w->param == AC_AREA_MODIFY) {
                 		event = AE_AREA_CHANGE_OK;
	               	} else {
						event = AE_AREA_ADD_OK;
					}
                }
            }
            break;
        case AC_AREA_DELETE:
            if (aconf->err!=ERR_NONE) {
                event = AE_AREA_DEL_FAIL;
            }else{
                event = AE_AREA_DEL_OK;
            }
            break;
        case AC_AREA_QUERY_SINGLE:
            area_do_query_single_tcp_packet(user,pkt,hdr,aconf);
            break;
        case AC_AREA_MODIFY_SINGLE:
            if (aconf->err!=ERR_NONE) {
                event = AE_AREA_CHANGE_FAIL;
            }else{
                event = AE_AREA_CHANGE_OK;
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

static RS area_proc_modify_single(cl_handle_t area_handle,u_int32_t pkt_type,cln_area_t* cln_area)
{
    user_t* user;
    pkt_t* pkt;
    area_priv_t* ap;
    area_config_t* aconf,*aconf_dest;
    area_obj_t * ao,*asrc;
    int size;
    int num_dev,num_eq;
    area_t* area,*area_dest;
    u_int8_t i;
    slave_t* slave,*master;
    equipment_t* eq;
    
    ap = lookup_by_handle(HDLT_AREA, area_handle);
    if (!ap) {
        return RS_INVALID_PARAM;
    }
    //先计算包大小
    user = ap->ac->user;
    num_dev = num_eq =0;
    if (pkt_type == CLNE_AREA_MODIDY_S) {
        for (i=0; i<cln_area->item_count; i++) {
            if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[i],HDLT_EQUIPMENT)) {
                num_eq++;
            }else{
                num_dev++;
            }
        }
        size = num_dev*(sizeof(area_dev_obj_t)+sizeof(area_obj_t))+
        num_eq*(sizeof(area_remote_obj_t)+sizeof(area_obj_t));
    }else{
        if (ap->prev_reply) {
            aconf_dest = (area_config_t*)ap->prev_reply;
            area_dest = aconf_dest->areas;
            asrc = (area_obj_t*)(area_dest+1);
            size = ap->prev_reply_len-(int)((char*)asrc-(char*)aconf_dest);
        }else{
            size = 0;
        }
    }
    
    master = slave_lookup_by_sn(user,user->sn);
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t)+size+sizeof(area_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_AREA, ap->area_handle,CMD_AREA_CONFIG ,
                               (void*)AC_AREA_MODIFY, area_wait_pkt_timeout_callback);
    aconf = (area_config_t*)get_pkt_payload(pkt, area_config_t);
    area = aconf->areas;
    ao = (area_obj_t*)(area+1);
    aconf->area_id = ap->area_id;
    aconf->action = AC_AREA_MODIFY;
    aconf->item_num = 0;
    memcpy(area, &ap->area_info, sizeof(*area));
    //填充报文
    if (pkt_type == CLNE_AREA_MODIDY_S) {
        for (i=0; i<cln_area->item_count; i++) {
            if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[i],HDLT_EQUIPMENT)) {
                if (master && master->equipment) {
                    eq = eq_lookup_by_handle(master->equipment, cln_area->eq_hands[i]);
                    if (eq) {
                        ao->obj_data_len = htons(sizeof(area_remote_obj_t));
                        ao->obj_type = AREA_OBJ_TYPE_REMOTE;
                        ao->obj_sub[0].remote_obj.local_id = htons(eq->attr.local_id);
                        ao = (area_obj_t*)((char*)ao+sizeof(area_obj_t)+sizeof(area_remote_obj_t));
                        aconf->item_num++;
                    }
                }
            }else{
                slave = slave_lookup_by_handle(user, cln_area->eq_hands[i]);
                if (slave) {
                    ao->obj_data_len = htons(sizeof(area_dev_obj_t));
                    ao->obj_type = AREA_OBJ_TYPE_DEV;
                    ao->obj_sub[0].dev_obj.sn = ntoh_ll(slave->sn);
                    ao = (area_obj_t*)((char*)ao+sizeof(area_obj_t)+sizeof(area_dev_obj_t));
                    aconf->item_num++;
                }
            }
        }
    }else{
        if (pkt_type == CLNE_AREA_CHANGE_NAME) {
            strncpy((char*)area->name, (const char*)cln_area->name, MAX_AREA_NAME_LENGTH-1);
            area->name[MAX_AREA_NAME_LENGTH-1]='\0';
        }else if (pkt_type == CLNE_AREA_CHANGE_IMG){
            area->image_id = cln_area->img_id;
        }
        
        if (ap->prev_reply) {
            aconf->item_num = aconf_dest->item_num;
            if (size>0) {
                memcpy(ao, asrc, (size_t)size);
            }
        }
    }
    
    user_add_pkt(user, pkt);
	timer_user_area_quick_query(user);
	timer_single_area_quick_query(ap);
    
    return RS_OK;
}

static RS area_proc_add_3(cl_handle_t user_handle, u_int32_t pkt_type, cln_area_t* cln_area)
{
    user_t* user;
    pkt_t* pkt;
    area_config_t* aconf;
    area_obj_t * ao;
    int size;
    int num_dev,num_eq;
    area_t* area;
    u_int8_t i;
    slave_t* slave,*master;
    equipment_t *eq;
    area_req_store_t* ars;
	
    user = lookup_by_handle(HDLT_USER, user_handle);
    if (user == NULL || user->ac == NULL) {
        return RS_INVALID_PARAM;
    }

    ars = cl_calloc(sizeof(area_req_store_t), 1);
    if (!ars) {
        return RS_MEMORY_MALLOC_FAIL;
    }
    stlc_list_add_tail(&ars->link, &user->ac->area_req_list);
	
    //先计算包大小
    num_dev = num_eq =0;

    for (i=0; i<cln_area->item_count; i++) {
        if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[i],HDLT_EQUIPMENT)) {
            num_eq++;
        }else{
            num_dev++;
        }
    }
    size = num_dev*(sizeof(area_dev_obj_t)+sizeof(area_obj_t))+
    num_eq*(sizeof(area_remote_obj_t)+sizeof(area_obj_t));
    
    master = slave_lookup_by_sn(user,user->sn);
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t)+size+sizeof(area_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);

    ars->area_handle = handle_create(HDLT_AREA);
    ars->create_time = get_sec();

    PKT_HANDLE(pkt) = wait_add(HDLT_AREA, ars->area_handle,CMD_AREA_CONFIG , (void*)user->handle,
                               area_wait_pkt_timeout_callback);
	
    aconf = (area_config_t*)get_pkt_payload(pkt, area_config_t);
    area = aconf->areas;
    ao = (area_obj_t*)(area+1);
    aconf->area_id = 0;
    aconf->action = AC_AREA_MODIFY;
    aconf->item_num = 0;
    area->create_time = htonl(ars->create_time);
	
    //填充报文
    for (i = 0; i < cln_area->item_count; i++) {
        if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[i],HDLT_EQUIPMENT)) {
            if (master && master->equipment) {
                eq = eq_lookup_by_handle(master->equipment, cln_area->eq_hands[i]);
                if (eq) {
                    ao->obj_data_len = htons(sizeof(area_remote_obj_t));
                    ao->obj_type = AREA_OBJ_TYPE_REMOTE;
                    ao->obj_sub[0].remote_obj.local_id = htons(eq->attr.local_id);
                    ao = (area_obj_t*)((char*)ao+sizeof(area_obj_t)+sizeof(area_remote_obj_t));
                    aconf->item_num++;
                }
            }
        }else{
            slave = slave_lookup_by_handle(user, cln_area->eq_hands[i]);
            if (slave) {
                ao->obj_data_len = htons(sizeof(area_dev_obj_t));
                ao->obj_type = AREA_OBJ_TYPE_DEV;
                ao->obj_sub[0].dev_obj.sn = ntoh_ll(slave->sn);
                ao = (area_obj_t*)((char*)ao+sizeof(area_obj_t)+sizeof(area_dev_obj_t));
                aconf->item_num++;
            }
        }
    }

    strncpy((char*)area->name, (const char*)cln_area->name, MAX_AREA_NAME_LENGTH-1);
    area->name[MAX_AREA_NAME_LENGTH-1]='\0';

    area->image_id = cln_area->img_id;
    
    user_add_pkt(user, pkt);
	timer_user_area_quick_query(user);

    if (cln_area->req_hand) {
        *cln_area->req_hand = ars->area_handle;
    }
    
    return RS_OK;
}

static RS area_proc_modify(cl_handle_t area_handle, u_int32_t pkt_type, cln_area_t* cln_area)
{
    user_t* user;
    pkt_t* pkt;
    area_priv_t* ap;
    area_config_t* aconf;
    area_obj_t * ao;
    int size;
    int num_dev,num_eq;
    area_t* area;
    u_int8_t i;
    slave_t* slave,*master;
    equipment_t *eq;
    
    ap = lookup_by_handle(HDLT_AREA, area_handle);
    if (!ap) {
        return RS_INVALID_PARAM;
    }
    //先计算包大小
    user = ap->ac->user;
    num_dev = num_eq =0;

    for (i=0; i<cln_area->item_count; i++) {
        if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[i],HDLT_EQUIPMENT)) {
            num_eq++;
        }else{
            num_dev++;
        }
    }
    size = num_dev*(sizeof(area_dev_obj_t)+sizeof(area_obj_t))+
    num_eq*(sizeof(area_remote_obj_t)+sizeof(area_obj_t));
    
    master = slave_lookup_by_sn(user,user->sn);
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t)+size+sizeof(area_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_AREA, ap->area_handle,CMD_AREA_CONFIG ,
                               (void*)AC_AREA_MODIFY, area_wait_pkt_timeout_callback);
    aconf = (area_config_t*)get_pkt_payload(pkt, area_config_t);
    area = aconf->areas;
    ao = (area_obj_t*)(area+1);
    aconf->area_id = ap->area_id;
    aconf->action = AC_AREA_MODIFY;
    aconf->item_num = 0;
    memcpy(area, &ap->area_info, sizeof(*area));
    //填充报文
    for (i = 0; i < cln_area->item_count; i++) {
        if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[i],HDLT_EQUIPMENT)) {
            if (master && master->equipment) {
                eq = eq_lookup_by_handle(master->equipment, cln_area->eq_hands[i]);
                if (eq) {
                    ao->obj_data_len = htons(sizeof(area_remote_obj_t));
                    ao->obj_type = AREA_OBJ_TYPE_REMOTE;
                    ao->obj_sub[0].remote_obj.local_id = htons(eq->attr.local_id);
                    ao = (area_obj_t*)((char*)ao+sizeof(area_obj_t)+sizeof(area_remote_obj_t));
                    aconf->item_num++;
                }
            }
        }else{
            slave = slave_lookup_by_handle(user, cln_area->eq_hands[i]);
            if (slave) {
                ao->obj_data_len = htons(sizeof(area_dev_obj_t));
                ao->obj_type = AREA_OBJ_TYPE_DEV;
                ao->obj_sub[0].dev_obj.sn = ntoh_ll(slave->sn);
                ao = (area_obj_t*)((char*)ao+sizeof(area_obj_t)+sizeof(area_dev_obj_t));
                aconf->item_num++;
            }
        }
    }

    strncpy((char*)area->name, (const char*)cln_area->name, MAX_AREA_NAME_LENGTH-1);
    area->name[MAX_AREA_NAME_LENGTH-1]='\0';

    area->image_id = cln_area->img_id;
    
    user_add_pkt(user, pkt);
	timer_user_area_quick_query(user);
	timer_single_area_quick_query(ap);
    
    return RS_OK;
}

static RS area_proc_single_modify(cl_handle_t area_handle,cln_area_t* cln_area)
{
    user_t* user;
    pkt_t* pkt;
    area_config_t* aconf;
    area_priv_t* ap, *prev_ap;
    area_obj_t* ao;
    equipment_t* eq = NULL;
    slave_t* slave = NULL,*master;
    int len = 0;
	u_int8_t area_id = 0;

    ap = lookup_by_handle(HDLT_AREA, cln_area->area_hand);
    if (!ap) {
        return RS_INVALID_PARAM;
    }
    user = ap->ac->user;
    if (IS_SAME_HANDLE_TYPE(cln_area->eq_hands[0],HDLT_EQUIPMENT)) {
        master = slave_lookup_by_sn(user, user->sn);
        if (!master||!master->equipment) {
            return RS_INVALID_PARAM;
        }
        eq = eq_lookup_by_handle(master->equipment, cln_area->eq_hands[0]);
        if (!eq) {
           return RS_INVALID_PARAM;
        }
        len = sizeof(area_remote_obj_t);
    }else{
        slave = slave_lookup_by_handle(user, cln_area->eq_hands[0]);
        if (!slave) {
            return RS_INVALID_PARAM;
        }
        len = sizeof(area_dev_obj_t);
    }
    len+=sizeof(area_obj_t);
    
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t)+len, NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    if (!pkt) {
        return RS_MEMORY_MALLOC_FAIL;
    }
    PKT_HANDLE(pkt) = wait_add(HDLT_AREA, ap->area_handle,CMD_AREA_CONFIG , (void*)AC_AREA_MODIFY_SINGLE, area_wait_pkt_timeout_callback);
    
    aconf = (area_config_t*)get_pkt_payload(pkt, area_config_t);
    ao = (area_obj_t*)(aconf+1);
    aconf->area_id = ap->area_id;
    aconf->action = AC_AREA_MODIFY_SINGLE;
    aconf->item_num = 0x1;
    
    if (eq) {
		area_id = eq->attr.area_id;
        ao->obj_data_len = htons(sizeof(area_remote_obj_t));
        ao->obj_type = AREA_OBJ_TYPE_REMOTE;
        ao->obj_sub[0].remote_obj.local_id = htons(eq->attr.local_id);
    } else if (slave) {
    	area_id = slave->area_id;
        ao->obj_data_len = htons(sizeof(area_dev_obj_t));
        ao->obj_type = AREA_OBJ_TYPE_DEV;
        ao->obj_sub[0].dev_obj.sn = ntoh_ll(slave->sn);
    }
    
    user_add_pkt(user, pkt);
	log_debug("area_proc_single_modify, modify to area id=%u\n", ap->area_id);

	// 快速查询
	if ((prev_ap = area_lookup_by_id(user->ac, area_id)) != NULL) {
    	timer_single_area_quick_query(prev_ap);
	}
    timer_single_area_quick_query(ap);
	
    if (eq) {
        eq_force_update_all_info(eq->ec);
    }else if(slave){
        CL_THREAD_OFF(slave->user->t_timer_query_master);
		CL_THREAD_TIMER_ON(&cl_priv->master, slave->user->t_timer_query_master, timer_query_master, (void *)slave->user, 1);
    }
    
    return RS_OK;
}

static RS area_proc_add(cln_area_t* cln_area)
{
    user_t* user;
    pkt_t* pkt;
    area_config_t* aconf;
    area_t* area;
    area_ctrl_t* ac;
    area_req_store_t* ars;
    
    user = lookup_by_handle(HDLT_USER, cln_area->user_hand);
    if (!user||!user->ac) {
        return RS_INVALID_PARAM;
    }
    
    ac = user->ac;
    ars = cl_calloc(sizeof(area_req_store_t), 1);
    if (!ars) {
        return RS_MEMORY_MALLOC_FAIL;
    }
    
    
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t)+sizeof(area_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                     user->sn, user->ds_type);
    if (!pkt) {
        cl_free(ars);
        return RS_MEMORY_MALLOC_FAIL;
    }
    ars->area_handle = handle_create(HDLT_AREA);
    ars->create_time = get_sec();
    
    PKT_HANDLE(pkt) = wait_add(HDLT_AREA, ars->area_handle,CMD_AREA_CONFIG , (void*)user->handle,
                               area_wait_pkt_timeout_callback);
    aconf = (area_config_t*)get_pkt_payload(pkt, area_config_t);
    area = aconf->areas;
    aconf->action = AC_AREA_MODIFY;
    aconf->item_num = 0x0;
    area->create_time = htonl(ars->create_time);
    area->image_id = cln_area->img_id;
    strncpy((char*)area->name, (const char*)cln_area->name, MAX_AREA_NAME_LENGTH-1);
    
    if (cln_area->req_hand) {
        *cln_area->req_hand = ars->area_handle;
    }
    
    stlc_list_add_tail(&ars->link, &ac->area_req_list);
    
    user_add_pkt(user, pkt);
    timer_user_area_quick_query(user);
    
    return RS_OK;
}

static RS area_porc_del(cl_handle_t area_handle)
{
    user_t* user;
    pkt_t* pkt;
    area_priv_t* ap;
    area_config_t* aconf;
    
    ap = lookup_by_handle(HDLT_AREA, area_handle);
    if (!ap) {
        return RS_INVALID_PARAM;
    }
    
    user = ap->ac->user;
    pkt = pkt_new_v2(CMD_AREA_CONFIG, sizeof(area_config_t), NHF_TRANSPARENT|NHF_WAIT_REPLY,
                    user->sn, user->ds_type);
    PKT_HANDLE(pkt) = wait_add(HDLT_AREA, ap->area_handle,CMD_AREA_CONFIG , (void*)AC_AREA_DELETE,
                               area_wait_pkt_timeout_callback);
    aconf = (area_config_t*)get_pkt_payload(pkt, area_config_t);
    aconf->area_id = ap->area_id;
    aconf->action = AC_AREA_DELETE;
    user_add_pkt(user, pkt);
    timer_user_area_quick_query(user);
    
    return RS_OK;
}

bool area_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
    cln_area_t* cln_area = (cln_area_t*)pkt->data;
    
	if (pkt->type <= CLNE_AREA_START || pkt->type >= CLNE_AREA_END) {
		return false;
	}
    
    switch (pkt->type) {
        case CLNE_AREA_ADD:
            *ret = area_proc_add(cln_area);
            break;
        case CLNE_AREA_DEL:
            *ret = area_porc_del(cln_area->area_hand);
            break;
        case CLNE_AREA_CHANGE_NAME:
        case CLNE_AREA_MODIDY_S:
        case CLNE_AREA_CHANGE_IMG:
            *ret = area_proc_modify_single(cln_area->area_hand, pkt->type,cln_area);
            break;
		case CLNE_AREA_MODIFY:
			if (cln_area->area_hand != 0) {
	            *ret = area_proc_modify(cln_area->area_hand, pkt->type, cln_area);
			} else {
	            *ret = area_proc_add_3(cln_area->user_hand, pkt->type, cln_area);
			}
			break;
        case CLNE_AREA_CHANGE_EQ:
            *ret = area_proc_single_modify(cln_area->area_hand, cln_area);
            break;
            
        default:
            return false;
            break;
    }
    
	return true;
}

cl_handle_t area_get_handle_by_id(user_t* user,u_int8_t area_id)
{
    cl_handle_t handle = 0;
    area_priv_t* ap;
    
    if ( area_id == 0 ||!user->ac ) {
        return handle;
    }
    
    ap = area_lookup_by_id(user->ac, area_id);
    if (ap) {
        return ap->area_handle;
    }
    
    return handle;
}
