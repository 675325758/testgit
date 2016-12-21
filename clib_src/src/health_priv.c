#include "cl_priv.h"
#include "cl_notify.h"
#include "cl_log.h"
#include "wait_server.h"
#include "health_priv.h"
#include "ds_proto.h"

static void fm_to_net(net_fm_t *net, family_t *up)
{
	net->bd_year = ntohs(up->bd_year);
	net->weight = ntohs(up->weight);
	net->bd_month = up->bd_month;
	net->height = up->height;
	net->sex = up->sex;
	net->career = up->career;
	net->step = up->step;
	net->id = up->id;
	net->action = up->action;
	net->is_current = up->is_current;
	memset(net->reserved, 0, sizeof(net->reserved));
	memcpy(net->name, up->name, sizeof(net->name));
}

void fm_to_local(net_fm_t *net, family_t *up)
{
	up->bd_year = ntohs(net->bd_year);
	up->weight = ntohs(net->weight);
	up->bd_month = net->bd_month;
	up->height = net->height;
	up->sex = net->sex;
	up->career = net->career;
	up->step = net->step;
	up->id = net->id;
	up->action = net->action;
	up->is_current = net->is_current;
	memset(up->reserved, 0, sizeof(up->reserved));
	memcpy(up->name, net->name, sizeof(up->name));
}

static RS parse_weigth(measure_t *m, net_messure_t *n)
{	
	u_int16_t w;
	
	if (n->messure_cnt != 1 || n->messure_para[0].mesure_type != MT_WEIGHT_CF) {
		return RS_ERROR;
	}
	w = BUILD_U16(n->messure_para[0].mesure_data[0], n->messure_para[0].mesure_data[1]);
	m->measure_data.weight.weight = w/10.0f;
	return RS_OK;
}

static RS parse_fat(measure_t *m, net_messure_t *n)
{
	u_int8_t i;
	u_int16_t w;
	char f[4] = {1, 1, 1, 1};
	if (n->messure_cnt != 4) {
		return RS_ERROR;
	}
	for (i = 0; i < n->messure_cnt; i++) {
		switch (n->messure_para[i].mesure_type) {
			case MT_FAT:
				w = BUILD_U16(n->messure_para[i].mesure_data[0], n->messure_para[i].mesure_data[1]);
				m->measure_data.fat.fat = w/10.0f;
				f[0] = 0;
				break;
				
			case MT_WATER:
				w = BUILD_U16(n->messure_para[i].mesure_data[0], n->messure_para[i].mesure_data[1]);
				m->measure_data.fat.water = w/10.0f;
				f[1] = 0;
				break;
				
			case MT_MUSCLE:
				w = BUILD_U16(n->messure_para[i].mesure_data[0], n->messure_para[i].mesure_data[1]);
				m->measure_data.fat.muscle = w/10.0f;
				f[2] = 0;
				break;
				
			case MT_VISCERAL_FAT:
				w = BUILD_U16(n->messure_para[i].mesure_data[0], n->messure_para[i].mesure_data[1]);
				m->measure_data.fat.visual_fat = w;
				f[3] = 0;
				break;
				
			default:
				break;
		}
	}
	if (f[0] || f[1] || f[2] || f[3])
		return RS_ERROR;
	return RS_OK;
}

static RS parse_blood_pressure(measure_t *m, net_messure_t *n)
{
	u_int8_t i;
	u_int16_t w;
	char f[3]={1, 1, 1};
	
	if (n->messure_cnt != 3)
		return RS_ERROR;
	for (i = 0; i < n->messure_cnt; i++) {
		switch ( n->messure_para[i].mesure_type) {
			case MT_PRESSURE_H:
				w = BUILD_U16(n->messure_para[i].mesure_data[0], n->messure_para[i].mesure_data[1]);			
				if (n->messure_para[i].mesure_data[2] == 0x01) {
					m->measure_data.blood_pressure.pressure_high = (float)w*7.5f;
				} else {
					m->measure_data.blood_pressure.pressure_high = w;
				}
				f[0] = 0;
				break;
				
			case MT_PRESSURE_l:
				w = BUILD_U16(n->messure_para[i].mesure_data[0], n->messure_para[i].mesure_data[1]);			
				if (n->messure_para[i].mesure_data[2] == 0x01) {
					m->measure_data.blood_pressure.pressure_low = (float)w*7.5f;
				} else {
					m->measure_data.blood_pressure.pressure_low = w;
				}
				f[1] = 0;
				break;
				
			case MT_PLUS:
				m->measure_data.blood_pressure.plus = n->messure_para[i].mesure_data[0];
				f[2] = 0;
				break;
			
			default:
				break;
		}
	}
	if (f[0] || f[1] || f[2])
		return RS_ERROR;
	return RS_OK;
}

static RS parse_blood_sugar(measure_t *m, net_messure_t *n)
{	
	u_int16_t w;
	
	if (n->messure_cnt != 1 || n->messure_para[0].mesure_type != MT_BLOOD_SUGAR) {
		return RS_ERROR;
	}
	w = BUILD_U16(n->messure_para[0].mesure_data[0], n->messure_para[0].mesure_data[1]);
	m->measure_data.blood_sugar.sugar = w/10.0f;
	return RS_OK;
}

static RS parse_blood_oxygen(measure_t *m, net_messure_t *n)
{
	u_int8_t i;
	char f[2]={1, 1};
	
	if (n->messure_cnt != 2)
		return RS_ERROR;
	for (i = 0; i < n->messure_cnt; i++) {
		switch ( n->messure_para[i].mesure_type) {
			case MT_BLOOD_OXYGN:
				m->measure_data.blood_oxygen.oxygen = n->messure_para[i].mesure_data[0];
				f[0] = 0;
				break;
				
			case MT_BLOOD_PLUS:
				m->measure_data.blood_oxygen.plus = n->messure_para[i].mesure_data[0];
				f[1] = 0;
				break;
			
			default:
				break;
		}
	}
	if (f[0] || f[1] )
		return RS_ERROR;
	return RS_OK;
}

static RS parse_ear_temp(measure_t *m, net_messure_t *n)
{	
	u_int16_t w;
	
	if (n->messure_cnt != 1 || n->messure_para[0].mesure_type != MT_EAR_TEMP) {
		return RS_ERROR;
	}
	w = BUILD_U16(n->messure_para[0].mesure_data[0], n->messure_para[0].mesure_data[1]);
	m->measure_data.ear_temp.temperature = (float)(w*0.1);	
	return RS_OK;
}

int is_today(u_int32_t ts)
{
	u_int32_t now;
	u_int32_t begin;

	begin = calc_begin_time_of_day(ts);
	now = (u_int32_t)time(NULL);
	if((begin <= now ) &&  (now < (begin + 3600*24-1))){
		return 1;
	}
	return 0;
}

static void parse_pedometer_one(measure_pedometer_t *meter, u_int8_t *data, int day)
{
	/* 服务器保存第一天格式00008c0126 00016d0226 0000000326 0000000426 	
	测量类型:0x26 当天数据
	测量数据:每个4字节
	aa bb cc 0x1[步数]  表示当天总步数为0xaabbcc
	00 aa bb 0x2[卡路里] 表示当天卡路里为0xaabb
	aa bb cc 0x3[有氧步数] 表示当天有氧步数0xaabbcc
	00 aa bb 0x4[有氧卡路里] 表示当天有氧卡路里0xaabb
	*/
	u_int32_t v;
	
	switch(data[3]){
		case 1:
			v = BUILD_U32(0, data[0], data[1], data[2]);
			meter->is_valid[day]++;
			meter->pedometer[day].step = (float)v;
			break;
			
		case 2:
			v = BUILD_U32(0, 0,  data[1], data[2]);
			meter->is_valid[day]++;
			meter->pedometer[day].calorie = (float)v;
			break;
			
		case 3:
			v = BUILD_U32(0, data[0], data[1], data[2]);
			meter->is_valid[day]++;
			meter->pedometer[day].oxygen_step = (float)v;
			break;
			
		case 4:
			v = BUILD_U32(0, 0,  data[1], data[2]);
			meter->is_valid[day]++;
			meter->pedometer[day].oxygen_calorie = (float)v;
			break;
		default:
			break;
	}
}

static RS parse_pedometer(measure_t *m, net_messure_t *n)
{	
	u_int8_t i;
	measure_pedometer_t *meter = &m->measure_data.meter;

	//设备上传的记录是从昨天开始的
	m->mtime -= (3600*24);
	memset(meter, 0, sizeof(*meter));	
	if (n->messure_cnt < 4)
		return RS_ERROR;
	
	for (i = 0; i < n->messure_cnt; i++) {
		switch ( n->messure_para[i].mesure_type) {
			case MT_PEDOMETER_1:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 0);
				break;
				
			case MT_PEDOMETER_2:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 1);
				break;

			case MT_PEDOMETER_3:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 2);
				break;
				
			case MT_PEDOMETER_4:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 3);
				break;

			case MT_PEDOMETER_5:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 4);
				break;
				
			case MT_PEDOMETER_6:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 5);
				break;

			case MT_PEDOMETER_7:
				parse_pedometer_one(meter, n->messure_para[i].mesure_data, 6);
				break;

			default:
				break;
		}
	}
	
	for(i = 0; i < MAX_PEDOMETER; i++){
		if(meter->is_valid[i])
			meter->valid_cnt++;
	}
	if(meter->valid_cnt)
		return RS_OK;
	else
		return RS_ERROR;
}

RS parse_measure_data(measure_t *m, net_messure_t *n)
{
	m->mtime = ntohl(n->mtime);
	m->fm_id = n->fm_id;
	m->hdt = n->md_id;
	m->measure_cnt = n->messure_cnt;
	m->reserved = 0;
	switch (m->hdt) {
		case MDT_WEIGTH:
			return parse_weigth(m, n);
			break;
			
		case MDT_FAT:
			return parse_fat(m, n);
			break;
			
		case MDT_BLOOD_PRESSURE:
			return parse_blood_pressure(m, n);
			break;

		case MDT_BLOOD_SUGAR:
			return parse_blood_sugar(m, n);
			break;

		case MDT_BLOOD_OXYGN:
			return parse_blood_oxygen(m, n);
			break;

		case MDT_PEDOMETER:
			return parse_pedometer(m, n);
			break;

		case MDT_EAR_THERMOMETER:
			return parse_ear_temp(m, n);
			break;
			
		default:
			break;
	}
	return RS_ERROR;		
}

void callback_health_request(u_int32_t result, void *none, void *waitp)
{
	wait_t *w = (wait_t *)waitp;
	user_t *user;

	if ((user = (user_t *)lookup_by_handle(w->obj_type, w->obj_handle)) == NULL) {
		log_err(false, "%s, not found obj_type=%d handle=0x%08x, cmd=%u, result=%u\n",
			__FUNCTION__, w->obj_type, w->obj_handle, w->cmd, result);
		return;
	}

	log_debug("%s: %s, handle=0x%08x, result=%d\n", __FUNCTION__, user->name, user->handle, result);

	switch (w->cmd) {
		case CMD_FM_Q:
			if (result == CMD_FAIL) {
				event_push(user->callback, HE_FM_LIST_FAIL, user->handle, user->callback_handle);
			}
			break;

		case CMD_FM_CONFIG_Q:
			if (result == CMD_FAIL) {
				event_push(user->callback, HE_FM_CONFIG_FAIL, user->handle, user->callback_handle);
			}
			break;
			
		case CMD_MESURE_Q:
			if (result == CMD_FAIL) {
				event_push(user->callback, HE_MEASURE_QUERY_FAIL, user->handle, user->callback_handle);
			}
			break;
			
		case CMD_MESURE_DEL:
			if (result == CMD_FAIL) {
				event_push(user->callback, HE_MEASURE_DEL_FAIL, user->handle, user->callback_handle);
			} else {
				event_push(user->callback, HE_MEASURE_DEL_OK, user->handle, user->callback_handle);
			}
			break;

		default:
			log_err(false, "%s, unknow cmd=%d. result=%d\n", __FUNCTION__, w->cmd, result);
			break;		
	}
}

RS helth_proc_family_q(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_family_t *up;
	pkt_t *pkt;

	up = (cln_family_t *)cln_pkt->data;	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, up->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new(CMD_FM_Q, 0, user->ds_type);
	if(pkt == NULL){
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, up->handle, CMD_FM_Q, NULL, callback_health_request);
	user_add_pkt(user, pkt);
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

RS helth_proc_family_config(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_family_t *up;
	net_fm_t *fm;
	pkt_t *pkt;

	up = (cln_family_t *)cln_pkt->data;	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, up->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new(CMD_FM_CONFIG_Q, sizeof(net_fm_t), user->ds_type);
	if(pkt == NULL){
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	fm = get_pkt_payload(pkt, net_fm_t);
	fm_to_net(fm, &up->fm);
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, up->handle, CMD_FM_CONFIG_Q, NULL, callback_health_request);
	user_add_pkt(user, pkt);
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

RS helth_proc_messure_q(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_measure_query_t *up;
	net_messure_q_t *net;
	pkt_t *pkt;

	up = (cln_measure_query_t *)cln_pkt->data;	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, up->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pkt = pkt_new(CMD_MESURE_Q, sizeof(net_messure_q_t), user->ds_type);
	if(pkt == NULL){
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	net = get_pkt_payload(pkt, net_messure_q_t);
	net->begin_time = ntohl(up->q.begin_time);
	net->end_time = ntohl(up->q.end_time);
	net->fm_id = up->q.fm_id;
	net->mdt = up->q.hdt;
	net->count = up->q.count;
	memset(net->reserved, 0, sizeof(net->reserved));
	
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, up->handle, CMD_MESURE_Q, NULL, callback_health_request);
	user_add_pkt(user, pkt);
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

RS helth_proc_messure_del(cl_notify_pkt_t *cln_pkt)
{
	RS ret = RS_OK;
	user_t *user;
	cln_measure_del_t *up;
	net_messure_t *net;
	pkt_t *pkt;

	up = (cln_measure_del_t *)cln_pkt->data;	
	cl_lock(&cl_priv->mutex);

	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, up->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		ret = RS_NOT_LOGIN;
		goto done;
	}

	if(up->del_all){
		pkt = pkt_new(CMD_MESURE_DEL, 0, user->ds_type);
	}else{
		pkt = pkt_new(CMD_MESURE_DEL, sizeof(net_messure_t), user->ds_type);
	}
	if(pkt == NULL){
		ret = RS_MEMORY_MALLOC_FAIL;
		goto done;
	}
	if(!up->del_all){
		net = get_pkt_payload(pkt, net_messure_t);
		net->mtime = ntohl(up->del.mtime);
		net->fm_id = up->del.fm_id;
		net->md_id = up->del.hdt;
		net->messure_cnt = up->del.measure_cnt;
		net->reserved = 0;
	}
	PKT_HANDLE(pkt) = wait_add(HDLT_USER, up->handle, CMD_MESURE_DEL, NULL, callback_health_request);
	user_add_pkt(user, pkt);
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}

bool health_proc_notify(cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;

	switch(pkt->type){
		case CLNE_FAMILY_LIST_Q:
			*ret = helth_proc_family_q(pkt);
			break;
			
		case CLNE_FAMILY_CONFIG:
			*ret = helth_proc_family_config(pkt);
			break;
			
		case CLNE_MESSURE_Q:
			*ret = helth_proc_messure_q(pkt);
			break;
			
		case CLNE_MESSURE_DEL:
			*ret = helth_proc_messure_del(pkt);
			break;
			
		default:
			res =false;
			break;
	}

	return res;
}

/*server response*/
void helth_tcp_fm_a(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	net_fm_t *fm;
	int cnt = 0;
//	int remain = hdr->param_len;
	user_health_t  *uh = (user_health_t *)user->health;

	if (uh == NULL)
		return ;
	
	cnt = hdr->param_len/sizeof(*fm);
	if (uh->fl) {
		cl_family_list_free(uh->fl);
	}	
	uh->fl = cl_malloc(sizeof(family_list_t));
	if(uh->fl == NULL){
		return;
	}
	uh->fl->count = 0;
	uh->fl->list = cl_calloc(sizeof(family_t), cnt);
	if (uh->fl->list == NULL) {
		SAFE_FREE(uh->fl);
		return;
	}
	
	fm = get_pkt_payload(pkt, net_fm_t);
	for (uh->fl->count = 0; uh->fl->count < cnt; uh->fl->count++) {
		fm_to_local(fm, &uh->fl->list[uh->fl->count]);
		fm++;
	}

	event_push(user->callback, HE_FM_LIST_OK,  user->handle, user->callback_handle);
	

}
void helth_tcp_fm_config_a(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	net_fm_t *fm;
	user_health_t *uh = user->health;

	if (uh == NULL)
		return;
	if (hdr->param_len < sizeof(*fm))
		return;
	fm = get_pkt_payload(pkt, net_fm_t);
	fm_to_local(fm, &uh->config_a);
	event_push(user->callback, HE_FM_CONFIG_OK,  user->handle, user->callback_handle);
}

void helth_tcp_measure_a(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	net_messure_t *mh;
	messure_para_t *md;
	int offset = 0, cnt = 0, remain = hdr->param_len, len;
	u_int8_t *payload;
	measure_list_t *ml = NULL;
	list_link_t *lk = NULL;
	user_health_t *uh = (user_health_t*)user->health;

	if (uh == NULL)
		return;
	
	payload = get_pkt_payload(pkt, u_int8_t);
	while (remain >= sizeof(*mh)) {
		mh = (net_messure_t*)&payload[offset];
		len = (int)(sizeof(*mh) + (mh->messure_cnt * sizeof(*md)));
		if (remain < len)
			break;
		cnt++;
		remain -= len;
		offset += len;
	}

	if (cnt == 0)
		return;
	ml = cl_malloc(sizeof(*ml));
	lk = cl_malloc(sizeof(*lk));
	if(ml == NULL || lk == NULL)
		goto errorout;	
	ml->list = cl_malloc(sizeof(measure_t) * cnt);
	if(ml->list == NULL)
		goto errorout;
	
	remain = hdr->param_len; offset = 0; cnt = 0;
	while (remain >= sizeof(*mh)) {
		mh = (net_messure_t*)&payload[offset];
		len = (int)(sizeof(*mh) + (mh->messure_cnt * sizeof(*md)));
		if (remain < len)
			break;		
		if(parse_measure_data(&ml->list[cnt], mh) == RS_OK)
			cnt++;
		remain -= len;
		offset += len;
	}
	ml->count = cnt;
	lk->p = ml;
	stlc_list_add_tail(&lk->node, &uh->measure_hd);
	event_push(user->callback, HE_MEASURE_QUERY_OK,  user->handle, user->callback_handle);
	return;

errorout:
	if(ml){
		SAFE_FREE(ml->list);
		cl_free(ml);
	}
	SAFE_FREE(lk);
}

bool health_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr)
{
	bool res = true;
	switch (hdr->command) {
		case CMD_FM_A:
			helth_tcp_fm_a(user, pkt, hdr);
			break;
			
		case CMD_FM_CONFIG_A:
			helth_tcp_fm_config_a(user, pkt, hdr);
			break;
			
		case CMD_MESURE_A:
			helth_tcp_measure_a(user, pkt, hdr);
			break;
			
		default:
			res = false;
			break;
	}

	return res;
}

user_health_t *user_health_init()
{
	user_health_t *h;
	h = cl_malloc(sizeof(*h));
	if (h) {
		memset(h, 0, sizeof(*h));
		STLC_INIT_LIST_HEAD(&h->measure_hd);		
	}
	return h;
}
void user_health_clean(user_health_t* h)
{
	list_link_t *pos, *n;
	if (!h)
		return;
	
	if (h->fl) {
		cl_family_list_free(h->fl);
	}
	stlc_list_for_each_entry_safe(list_link_t, pos, n, &h->measure_hd, node){
		stlc_list_del(&pos->node);
		cl_measure_list_free((measure_list_t *)pos->p);
		cl_free(pos);
	}
	
	cl_free(h);
}


