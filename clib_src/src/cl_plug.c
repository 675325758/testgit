#include "cl_priv.h"
#include "cl_plug.h"
#include "plug_priv.h"

static void plug_timer_utc_2_local(cl_plug_timer_t *net_plug_timer, int zone)
{
	int hour;

	hour = net_plug_timer->hour + 24 + zone;
	if (hour < 24) {
		net_plug_timer->week = timer_week_right_shift(net_plug_timer->week);
	} else if (hour >= 48) {
		net_plug_timer->week = timer_week_left_shift(net_plug_timer->week);
	}
	net_plug_timer->hour = hour%24;
}

static void plug_timer_local_2_utc(cl_plug_timer_t *net_plug_timer, int zone)
{	
	int hour;
	
	hour = net_plug_timer->hour + 24 - zone;
	if (hour < 24) {
		net_plug_timer->week = timer_week_right_shift(net_plug_timer->week);
	} else if (hour >= 48) {
		net_plug_timer->week = timer_week_left_shift(net_plug_timer->week);
	}
	net_plug_timer->hour = hour%24;
}

#define	BIG_NUM	88888888

static int cal_next_time(cl_plug_timer_t *t, int wday, int hour, int minute)
{
	int i, n;
	
	if ( ! t->enable )
		return BIG_NUM;

	// 如果是一次性规则，做下特殊处理
	if (t->week == 0) {
		n = (t->hour - hour)*60 + (t->minute - minute);
		if (n < 0)
			n += (24*60);
		return n;
	}

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
	log_err(false, "@@@@@@@@@: cal_next_time error: plug timer: week=0x%02x, hour=%u, minute=%u, now: wday=%u, hour=%u, minute=%u\n",
		t->week, t->hour, t->minute, wday, hour, minute);
	return BIG_NUM;
}

static void sort_plug_timer(cl_plug_info_t *pi)
{
	int i, k;
	time_t now;
	struct tm *tm;
	int wday, hour, minute;
	int *diff, *idx;
	bool modify;

	diff = cl_calloc(sizeof(int), pi->num_timer);
	idx = cl_calloc(sizeof(int), pi->num_timer);
	
	now = get_sec();
	tm = localtime(&now);
	// Day of week (0 – 6; Sunday = 0).
	wday = tm->tm_wday;
	hour = tm->tm_hour;
	minute = tm->tm_min;

	// 计算与当前时间的差值
	for (i = 0; i < (int)pi->num_timer; i++) {
		diff[i] = cal_next_time(pi->timer[i], wday, hour, minute);
	}

	// 从小到大排序
	for (i = 0; i < (int)pi->num_timer; i++) {
		idx[i] = i;
	}
	for (i = 0; i < (int)pi->num_timer; i++) {
		modify = false;
		for (k = 0; k < (int)pi->num_timer - 1; k++) {
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
	for (i = 0; i < (int)pi->num_timer; i++) {
		if (diff[idx[i]] == BIG_NUM)
			break;

		pi->timer[idx[i]]->sort = i + 1;
	}
	log_debug("----------------------------------\n");
	for (i = 0; i < (int)pi->num_timer; i++) {
		log_debug("plug timer[%d] %s: sort=%d\n", i, pi->timer[i]->name, pi->timer[i]->sort);
	}

	cl_free(diff);
	cl_free(idx);
}


/*
	功能:
		开始定时轮询某个遥控插座的状态
	输入参数:
		@slave_handle: 遥控插座的句柄
		@seconds: 多少秒查询一次
		@callback: 回调函数
		@handle: 回调参数，给调用者自己使用，SDK不关心该参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_query_start(cl_handle_t slave_handle, u_int32_t seconds, cl_callback_t callback, void *handle)
{
	cl_notify_pkt_t *pkt;
	cln_plug_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_PLUG_QUERY_START, CLNPF_ACK);
	
	v = (cln_plug_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->callback = callback;
	v->callback_handle = handle;
	v->query_seconds = seconds;
	
	pkt->param_len = sizeof(cln_plug_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}


/*
	功能:
		停止定时轮询某个遥控插座的状态
	输入参数:
		@slave_handle: 遥控插座的句柄
	输出参数:
		无
	返回:
		无
*/
CLIB_API RS cl_plug_query_stop(cl_handle_t slave_handle)
{
	cl_notify_pkt_t *pkt;
	cln_plug_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_PLUG_QUERY_STOP, CLNPF_ACK);
	
	v = (cln_plug_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	
	pkt->param_len = sizeof(cln_plug_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

/*
	功能:
		开始定时轮询某个遥控插座的状态
	输入参数:
		@slave_handle: 遥控插座的句柄
		@index: 控制哪个插孔。0表示所有
		@on: 1表示通电，0表示断电
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_plug_turn_on(cl_handle_t slave_handle, u_int32_t index, bool on)
{
	cl_notify_pkt_t *pkt;
	cln_plug_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_PLUG_SET_ON, CLNPF_ACK);
	
	v = (cln_plug_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->index = index;
	v->on = on;
	
	pkt->param_len = sizeof(cln_plug_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_plug_timer_set(cl_handle_t slave_handle, u_int32_t index, cl_plug_timer_t *plug_timer, int32_t tz)
{
	cl_notify_pkt_t *pkt;
	cln_plug_t *v;
	RS ret;

	CL_CHECK_INIT;

	plug_timer_local_2_utc(plug_timer, tz);

	pkt = cl_notify_pkt_new(1024, CLNE_PLUG_SET_TIMER, CLNPF_ACK);
	
	v = (cln_plug_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->index = index;
	v->timer = plug_timer;
	
	pkt->param_len = sizeof(cln_plug_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_plug_clear_electric_stat(cl_handle_t slave_handle, u_int32_t index)
{
	cl_notify_pkt_t *pkt;
	cln_plug_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_PLUG_ELECTRIC_STAT_CLS, CLNPF_ACK);
	
	v = (cln_plug_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->index = index;
	
	pkt->param_len = sizeof(cln_plug_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API RS cl_plug_timer_del(cl_handle_t slave_handle, u_int32_t index, int id)
{
	cl_notify_pkt_t *pkt;
	cln_plug_t *v;
	RS ret;

	CL_CHECK_INIT;

	pkt = cl_notify_pkt_new(1024, CLNE_PLUG_DEL_TIMER, CLNPF_ACK);
	
	v = (cln_plug_t *)&pkt->data[0];
	v->slave_halde = slave_handle;
	v->index = index;
	v->id = id;
	
	pkt->param_len = sizeof(cln_plug_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	return ret;
}

CLIB_API cl_plug_info_t *cl_plug_get_info(cl_handle_t slave_handle, u_int32_t index, int32_t tz)
{
	int i, next_action;
	slave_t *slave;
	plug_t *plug;
	net_plug_timer_t *npt;
	cl_plug_info_t *pi = NULL;
	
	if (cl_priv == NULL) {
		log_err(false, "client libary is not init now. please call cl_init first!!!!\n");
		return NULL; 
	}
	
	cl_lock(&cl_priv->mutex);

	if ((slave = lookup_by_handle(HDLT_SLAVE, slave_handle)) == NULL) {
		log_err(false, "cl_plug_get_info: lookup slave handle=0x%08x failed\n", slave_handle);
		goto done;
	}
	plug = slave->plug;
	
	pi = cl_calloc(sizeof(cl_plug_info_t), 1);
	pi->is_on = plug->on;
	pi->current = plug->ac;
	pi->voltage = plug->v;
	pi->temperature = plug->t;

	pi->electric_section = plug->electric_stat_section;
	pi->electric_total = plug->electric_stat_total;
	pi->section_begin_time = plug->section_time;

	if (plug->timer == NULL)
		goto done;
	pi->num_timer = plug->timer->num;
	pi->timer = cl_calloc(sizeof(void *)*pi->num_timer, 1);
	for (i = 0; i < (int)pi->num_timer; i++) {
		pi->timer[i] = cl_calloc(sizeof(cl_plug_timer_t), 1);
		npt = &plug->timer->plug_timer[i];
		
		pi->timer[i]->id = npt->id;
		pi->timer[i]->hour = npt->hours;
		pi->timer[i]->minute = npt->minute;
		pi->timer[i]->week = npt->week;
		pi->timer[i]->enable = npt->enable;
		pi->timer[i]->last = ntohs(npt->last);
		pi->timer[i]->name = cl_strdup((char*)npt->name);
		
		plug_timer_utc_2_local(pi->timer[i], tz);
	}

	next_action = ntohs(plug->timer->next_action);
	if (next_action & PTCF_EFFECT) {
		pi->next_effect = true;
		pi->next_on = ! (!(next_action & PTCF_ON));
		pi->next_minute = next_action & 0x3FFF;
	}
	sort_plug_timer(pi);
	
done:
	cl_unlock(&cl_priv->mutex);

	return pi;
}

CLIB_API void cl_plug_free_info(cl_plug_info_t *info)
{
	int i;
	
	if (info == NULL)
		return;
	
	for (i = 0; i < (int)info->num_timer; i++) {
		SAFE_FREE(info->timer[i]->name);
		SAFE_FREE(info->timer[i]);
	}
	
	SAFE_FREE(info->timer);
	cl_free(info);
}


