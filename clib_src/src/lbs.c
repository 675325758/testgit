#include "client_lib.h"
#include "cl_priv.h"
#include "lbs.h"
#include "math.h"
#include "udp_ctrl.h"
#include "uc_client.h"
#include "smart_appliance_priv.h"
#include "cl_log.h"

/*家所在基站超时时间*/
#define BASE_HOME_EXPIRE (3600*24*30)
/*根据基站回家轨迹失效时间*/
#define BASE_TRACE_EXPIRE (3600*24*30)
/*保存基站信息时间间隔*/
#if 1
#define BASE_UPDATE_INTERVAL (60*30)
#define BASE_TIME_NEAR (60*8)
#define BASE_TIME_MIDDLE (60*10)
#define BASE_TIME_FAR (60*15)
#else
#define BASE_UPDATE_INTERVAL (60*1)
#define BASE_TIME_NEAR (60*2)
#define BASE_TIME_MIDDLE (60*4)
#define BASE_TIME_FAR (60*6)

/**
#define BASE_UPDATE_INTERVAL (60*5)
#define BASE_TIME_NEAR (60*2)
#define BASE_TIME_MIDDLE (60*4)
#define BASE_TIME_FAR (60*6)
*/
#endif 
#define MAX_NEAR_BASE 1

#define STR_HOMEBASE "homebase.txt"
#define STR_NEARBASE "nearbase.txt"
#define STR_FARBASE "farbase.txt"

typedef struct {
	struct stlc_list_head link;
	// 标记时间
	double time;
	// 纬度、经度
	lbs_pos_t pos;
} lbs_mark_t;

typedef struct{
	struct stlc_list_head link;
	u_int32_t time;
	int base;
}fix_base_t;

typedef struct{
	struct stlc_list_head link;
	u_int32_t time;
	u_int32_t time_end;
	int is_in_lan;
	lbs_base_t base;
}track_base_t;

typedef struct {
	// 存储数据的路径
	char dir[CL_MAX_PATH];
	bool has_mark_home;
	lbs_mark_t home; //家的位置
	lbs_mark_t current;//手机当前位置

	bool is_base;
	bool has_mark_base;

	// 最后一次局域网扫描到的时间
	u_int32_t last_lan_detect;
	//局域网扫描到标志
	int is_in_lan;

	// lbs_mark_t
	struct stlc_list_head track;
	// 离家较远的基站集合
	struct stlc_list_head base_far;
	// 离家较近的基站集合
	struct stlc_list_head base_near;
	// 家的基站集合
	struct stlc_list_head base_home;
	// 最近基站轨迹
	struct stlc_list_head base_track;
	int track_count;
	//query device's location count
	u_int8_t query_home_cnt;
	//last execute smart home action
	u_int8_t smart_home_action;
	//last execute smart home time
	u_int32_t smart_home_time;
	int lbs_status;
	int prev_base_status;
	int base_status;
	double recent_speed;
	double recent_distance;
	
} lbs_info_t;

/*
	该文件用来标记安装点。格式: (纬度,经度)
	1407208973.063  72.0876254 36.248569
*/
#define	FN_LBS_HOME	"home.txt"
/*
	该文件保存位置文件的文件列表，一行一个文件名，不含目录
*/
#define	FN_LBS_FLIST	"files.txt"
#define	FN_LBS_FLIST_BAK	"files.bak"

// 记录到家前多久时间内的轨迹，单位秒
#define	MAX_LBS_DURATION (60*15)
// 如果记录的时间很短，认为一直在家里
#define	MIN_LBS_DURATION (60*2)
#define	LBS_LAN_DETECT_DIE	10
//轨迹最少缓存坐标点数量
#define	MIN_TRACK_COUNT 512
// 保存最近多少天的位置
#define	LBS_SAVE_DAY	30


// 多少米认为是很近
static int lbs_near = 500;
static int lbs_far = 2000;
//同一地点误差范围
#define LBS_HOME 200
//提前多久开启空调
//#define LBS_TIMEIN (60*15)

#define LBS_TIMEIN (60*3)
//开启空调多久后没有到家，关闭空调
#define LBS_TIMEOUT (60*15)

//#define LBS_TIMEOUT (60*3)


#define SPEED_MIN 0.27f		//散步速度1.0 km/h
#define SPEED_WALK 1.0f		//步行速度3.6 km/h
#define SPEED_BIKE 5.5f		//自行车速度20 km/h


#ifdef SMART_PUBILC
	#define lbs_log(user, ...)
#else
	#define lbs_log(user, ...) //lbs_save_log_ex(user,__FUNCTION__, __LINE__, __VA_ARGS__)
#endif


/******************************************************************/
void lbs_set_status(user_t *user, int status);
static double pos_diff(lbs_pos_t *a, lbs_pos_t *b);
static double recent_speed(user_t *user);
RS do_set_home_location(user_t *user);
static RS lbs_save(lbs_info_t *info);
void lbs_save_home_base(user_t *user);
void lbs_save_near_base(user_t *user);
void lbs_save_far_base(user_t *user);
void try_update_far_near_base(user_t *user);
void try_update_home_base_ex(user_t *user);
void try_update_home_base(user_t *user);
void lbs_load_home_base(user_t *user);
void lbs_load_near_base(user_t *user);
void lbs_load_far_base(user_t *user);
bool is_in_home_base(user_t *user);
bool is_near_home_base(user_t *user);
bool is_far_home_base(user_t *user);
bool is_learned_base(user_t *user);
int lbs_get_base_status(user_t *user);
RS lbs_user_init(user_t *user);

void lbs_init_into(user_t *user)
{
	if(user->status == UCCS_ESTABLISH)
		query_home_location(user);
	lbs_load_home_base(user);
	lbs_load_near_base(user);
	lbs_load_far_base(user);
}

void lbs_init_out(user_t *user)
{
	
}


void lbs_save_log_ex(user_t *user,const char *functions, int line, const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	FILE *fp;
	char nows[64];	

	time_t tNow =time(NULL);  
	va_list vl;
	int lens = 1024*8;
	
	struct tm* ptm = localtime(&tNow);
	strftime(nows,sizeof(nows),"%F %H:%M:%S ",ptm);  
	
	if(info == NULL){
		return;
	}	
	
	buf = malloc(lens);
	if(buf == NULL)
	{
		return ;
	}
	
    va_start(vl, fmt);
	pos += sprintf(buf + pos, "<%s:%d> ", functions, line);
	pos += vsnprintf(buf + pos, lens-pos, fmt, vl);
    va_end(vl);
	
	sprintf(short_name, "%012"PRIu64"_log.txt", user->sn);
	sprintf(fn, "%s/%s", info->dir, short_name);
	if ((fp = fopen(fn, "a+")) != NULL) {
		fwrite(nows,strlen(nows),1,fp);
		fwrite(buf,pos,1,fp);
		fwrite("\n",1,1,fp);
		fclose(fp);
	}
	free(buf);
	
}

void lbs_init_base_proc(user_t *user)
{
	lbs_info_t *info = (lbs_info_t *)user->lbs;
	
	lbs_log(user," init is_in_lan: %d",info->is_in_lan);	
	if(info->is_in_lan){		
		/*在局域网里，确认在家里*/
		try_update_home_base(user);
		try_update_far_near_base(user);
		info->prev_base_status = BASE_IN_HOME;
		info->base_status = BASE_IN_HOME;
	}else{
		if(is_learned_base(user)){
			lbs_log(user,"is_learned_base");
			info->prev_base_status = BASE_INIT;
			info->base_status = lbs_get_base_status(user);
			lbs_set_status(user, LBS_IDLE);
		}
	}
}

void lbs_init_proc(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;

	if(info->is_base){
		lbs_init_base_proc(user);
		return;
	}
	if(info->has_mark_home){
		/*已经标记家的位置*/
		if(info->is_in_lan){
			/*手机在家里，可以进行修正*/
			if(pos_diff(&info->home.pos, &info->current.pos) > LBS_HOME){
				goto do_mark_home;
			}
		}
		lbs_set_status(user, LBS_IDLE);		
	}else{
		/*还没有标记家的位置*/
		if(info->is_in_lan){
			/*在家里，进行标记并完成初始状态*/
			goto do_mark_home;			
		}
	}
	return;
	
do_mark_home:

	info->home.pos = info->current.pos;
	info->has_mark_home = true;
	do_set_home_location(user);
	lbs_set_status(user, LBS_IDLE);
}

void lbs_idle_into(user_t *user)
{
}

void lbs_idle_out(user_t *user)
{
}

void lbs_idle_base_proc(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	int new_status;
	
	new_status = lbs_get_base_status(user);
	if(info==NULL)
	{
		lbs_log(user,"info is null");
		return;
	}
	
	lbs_log(user,"[is_in_lan: %d, new_status: %d, base_status: %d, prev_base_status: %d]",
	info->is_in_lan,
	new_status,
	info->base_status,info->prev_base_status);
	
	if(info->is_in_lan)
	{
		try_update_far_near_base(user);
		info->prev_base_status = BASE_IN_HOME;
		info->base_status = BASE_IN_HOME;
		return;
	}	
	
	if(new_status == info->base_status)
		return;
	

	lbs_log(user," [new_status=%d, base_status: %d, prev_base_status: %d]",
		new_status,
		info->base_status,info->prev_base_status
		);
	
	if((new_status == BASE_IN_HOME && info->base_status == BASE_NEAR_HOME)
		||(new_status == BASE_IN_HOME && info->prev_base_status == BASE_FAR_HOME))
	{
		lbs_log(user,"base_status: %d,prev_base_status: %d",info->base_status,info->prev_base_status);
	
		/*由远到近，可以开空调了*/
		info->prev_base_status = info->base_status;
		info->base_status = new_status;
		lbs_set_status(user, LBS_GOING_HOME);
		return;
	}else if(new_status == BASE_IN_HOME && info->prev_base_status == BASE_NEAR_HOME)
	{
		try_update_home_base_ex(user);
	}
	info->prev_base_status = info->base_status;
	info->base_status = new_status;
	return;	
}

void lbs_idle_proc(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	double speed, distance;
	if(info->is_base){
		lbs_idle_base_proc(user);
		return;
	}

	lbs_log(user,"idle is_in_lan: %d",info->is_in_lan);
		
	if(info->is_in_lan){
		/*在家里，啥也不用干*/
		return;
	}
	
	distance = pos_diff(&info->home.pos, &info->current.pos);
	lbs_log(user,"distance=%f",distance);
	
	if(distance < lbs_far){
		//在家附近溜达
		return;
	}
	if( (get_sec() - info->last_lan_detect) < LBS_TIMEIN){
		lbs_log(user,"%u %u",get_sec() ,info->last_lan_detect);
		//刚刚还在家，才出门吧
		return;
	}
	speed = recent_speed(user);
	if(speed < SPEED_MIN){
		lbs_log(user,"speed:%d",speed);
		return;
	}
	if(distance/speed < LBS_TIMEIN){
		lbs_set_status(user, LBS_GOING_HOME);
	}
	
}

void lbs_going_home_into(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	lbs_log(user,"do LAC_SMART_ON");

	do_smart_home(user, LAC_SMART_ON);
	event_push(user->callback, SAE_SMART_HOME_ON,  user->handle, user->callback_handle);
	info->smart_home_time = get_sec();

}

void lbs_going_home_out(user_t *user)
{
}

void lbs_going_home_proc(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	u_int32_t now;
	if(info->is_in_lan){
		lbs_set_status(user, LBS_ARRIVE_HOME);
	}else{
		now = get_sec();
		lbs_log(user,"going home timeout[now: %d,smart_home_time: %d",now,info->smart_home_time);
	
		if((now - info->smart_home_time) > LBS_TIMEOUT){
			/*超过一定时间还没有到家*/
			lbs_set_status(user, LBS_LEAVE_HOME);
			event_push(user->callback, SAE_SMART_HOME_CANCEL,  user->handle, user->callback_handle);
		}
	}

	
}

void lbs_arrive_home_into(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	
	lbs_log(user,"do LAC_BACKHOME");
		
	do_smart_home(user, LAC_BACKHOME);
	lbs_save((lbs_info_t*)user->lbs);
	if(info->is_base){
		try_update_home_base_ex(user);
		try_update_far_near_base(user);
	}
}

void lbs_arrive_home_out(user_t *user)
{
}

void lbs_arrive_home_proc(user_t *user)
{
	lbs_set_status(user, LBS_IDLE);
}

void lbs_leave_home_into(user_t *user)
{
	
	lbs_log(user," do LAC_SMART_OFF ");
	do_smart_home(user, LAC_SMART_OFF);
}

void lbs_leave_home_out(user_t *user)
{
}

void lbs_leave_home_proc(user_t *user)
{
	lbs_set_status(user, LBS_IDLE);
}

void lbs_max_into(user_t *user)
{
}

void lbs_max_out(user_t *user)
{
}

void lbs_max_proc(user_t *user)
{
	lbs_set_status(user, LBS_IDLE);
}

lbs_proc_t lbs_proc[LBS_MAX+1] = {
	{"INIT", lbs_init_into, lbs_init_out, lbs_init_proc},
	{"IDLE", lbs_idle_into, lbs_idle_out, lbs_idle_proc},
	{"GOING_HOME", lbs_going_home_into, lbs_going_home_out, lbs_going_home_proc},
	{"ARRIVE_HOME", lbs_arrive_home_into, lbs_arrive_home_out, lbs_arrive_home_proc},
	{"LEAVE_HOME", lbs_leave_home_into, lbs_leave_home_out, lbs_leave_home_proc},
	{"MAX", lbs_max_into, lbs_max_out, lbs_max_proc}
};

void lbs_set_status(user_t *user, int status)
{
	lbs_info_t *info = (lbs_info_t *)user->lbs;
	if(status > LBS_MAX)
		return;
	lbs_proc[info->lbs_status].on_out(user);
	info->lbs_status = status;

	lbs_log(user,"status:%d",info->lbs_status);	
	lbs_proc[status].on_into(user);	
}


static void lbs_recover_flist(lbs_info_t *info)
{
	char fn[CL_MAX_PATH], bak[CL_MAX_PATH];
	FILE *fp;

	sprintf(fn, "%s/%s", info->dir, FN_LBS_FLIST);
	if ((fp = fopen(fn, "r")) == NULL) {
		sprintf(bak, "%s/%s", info->dir, FN_LBS_FLIST_BAK);
		if ((fp = fopen(bak, "r")) != NULL) {
			fclose(fp);
			rename(bak, fn);
			log_info("HaHa, recover LBS file list ok, %s -> %s\n", bak, fn);
		}
	} else {
		fclose(fp);
	}
}

static RS lbs_del_old(lbs_info_t *info)
{
	char *p;
	u_int32_t ft;
	char fn[CL_MAX_PATH], fn_bak[CL_MAX_PATH];
	char line[CL_MAX_PATH], fn_del[CL_MAX_PATH];
	FILE *fp, *fp_bak;
	u_int32_t del_time = get_sec() - (LBS_SAVE_DAY*ONE_DAY_SECOND);

	sprintf(fn, "%s/%s", info->dir, FN_LBS_FLIST);
	if ((fp = fopen(fn, "r")) == NULL) {
		log_err(true, "open LBS file list %s failed.\n", fn);
		return RS_ERROR;
	}

	sprintf(fn_bak, "%s/%s", info->dir, FN_LBS_FLIST_BAK);
	if ((fp_bak = fopen(fn_bak, "w+")) == NULL) {
		log_err(true, "open LBS file list bak %s failed.\n", fp_bak);
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		for (p = line; *p != '\0'; p++) {
			if (*p == '\r' || *p == '\n' || *p == ' ' || *p == '\t') {
				*p = '\0';
				p++;
				break;
			}
		}
		
		ft = atoi(p);
		if (ft == 0)
			continue;
		
		if (ft < del_time) {
			sprintf(fn_del, "%s/%s", info->dir, line);
			unlink(fn_del);
		} else if (fp_bak != NULL) {
			fprintf(fp_bak, "%s %u\n", line, ft);
		}
	}

	fclose(fp);
	if (fp_bak != NULL) {
		fclose(fp_bak);
		unlink(fn);
		rename(fn_bak, fn);
	}

	return RS_OK;
}

static RS lbs_save(lbs_info_t *info)
{
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH], fn_flist[CL_MAX_PATH];
	FILE *fp;
	lbs_mark_t *node, *next;
	struct tm tm;
	RS ret = RS_ERROR;
	time_t now = get_sec();

	lbs_del_old(info);
	localtime_r(&now, &tm);
	sprintf(short_name, "%04u_%02u_%02u_%02u_%02u_%02u.txt",
		tm.tm_year + LOCALTIME_YEAR_BASE, tm.tm_mon+1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
	sprintf(fn, "%s/%s", info->dir, short_name);

	// 1. save positions
	if ((fp = fopen(fn, "w+")) == NULL) {
		log_err(true, "create file %s failed\n", fn);
		goto done;
	}

	stlc_list_for_each_entry_safe(lbs_mark_t, node, next, &info->track, link) {
		fprintf(fp, "%lf %.10lf %.10lf\n", node->time, node->pos.latitude, node->pos.longitude);
		
		stlc_list_del_only(&node->link);
		cl_free(node);
	}

	// 最后把home的位置顺便记录下
	fprintf(fp, "HOME: %lf %.10lf %.10lf\n", info->home.time, info->home.pos.latitude, info->home.pos.longitude);

	fclose(fp);

	// 2. save  filename
	sprintf(fn_flist, "%s/%s", info->dir, FN_LBS_FLIST);
	if ((fp = fopen(fn_flist, "a+")) != NULL) {
		fprintf(fp, "%s %u\n", short_name, (u_int32_t)now);
		fclose(fp);
	}

	ret = RS_OK;

done:
	return RS_OK;
}

int try_update_fix_base_one(struct stlc_list_head *head, int base ,u_int32_t now)
{
	fix_base_t *node;
	int ret = 0;
	stlc_list_for_each_entry(fix_base_t, node, head, link){
		if(node->base == base){
			/*保存文件不能太频繁了*/
			//if((now - node->time) >= BASE_UPDATE_INTERVAL){
				node->time = now;				
				ret = 1;			
			//}
			return ret;
		}
	}
	node = cl_calloc(sizeof(*node), 1);
	if(node == NULL)
		return ret;
	ret = 2;
	node->time = now;
	node->base = base;
	stlc_list_add(&node->link, head);
	return ret;	
}

void try_update_home_base_one(user_t *user, track_base_t *base)
{
	int flag=0;
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	
	flag |= try_update_fix_base_one(&info->base_home, base->base.base[0], base->time);
	

	lbs_log(user,"update home base flag=%d,base=%d",
		flag,
		base->base.base[0]);
	
	if(flag)
		lbs_save_home_base(user);
}

void try_update_home_base_ex(user_t *user)
{
	int flag = 0;
	track_base_t *pos; 
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	int count = 0;
	if(stlc_list_empty(&info->base_track)){
		return;
	}
	
	stlc_list_for_each_prev_entry(track_base_t, pos, &info->base_track, link){
		if(count>1){
			break;
		}
		lbs_log(user,"ex count:%d, base: %d",count,pos->base.base[0]);
		flag |= try_update_fix_base_one(&info->base_home, pos->base.base[0], pos->time);
		count ++;
	}
	lbs_log(user,"ex flag: %d",flag);
	if(flag)
		lbs_save_home_base(user);
}

void try_update_home_base(user_t *user)
{
	track_base_t *pos;	
	int flag = 0;
	int ret = 0, count = 0;
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	
	if(stlc_list_empty(&info->base_track)){
		return;
	}
	
	stlc_list_for_each_prev_entry(track_base_t, pos, &info->base_track, link){

		lbs_log(user,"count=%d, base: %d",	count,pos->base.base[0]);	

		if(count>=2){//学习时，lan的前一个基站也作为home范围(共2个基站)
			break;
		}
		ret = try_update_fix_base_one(&info->base_home, pos->base.base[0], pos->time);
		if(ret)
		{			
			flag |=ret;
			if(ret==2)
			{
				count++;
			}
		}
	}

	lbs_log(user,"save_home_base flag: %d",flag);
	if(flag)
		lbs_save_home_base(user);
}

bool is_in_fix_base(struct stlc_list_head *head, int base)
{
	fix_base_t *node;

	stlc_list_for_each_entry(fix_base_t, node, head, link){
		if(node->base == base){
			return true;
		}
	}
	return false;	
}

void clear_base_track(user_t *user, int  clean_all)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	track_base_t *newer, *pos, *next;
	
	if(stlc_list_empty(&info->base_track))
		return;	
	newer = stlc_list_last_entry(&info->base_track, track_base_t, link);
	stlc_list_for_each_entry_safe(track_base_t, pos, next, &info->base_track, link){
		if(clean_all == 0){
			if(newer == pos)
				break; //保留最新的一个节点
		}
		stlc_list_del(&pos->link);
		cl_free(pos);
	}
}

void try_amend_near_to_home(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	fix_base_t *node, *n;
	int flag = 0;

	stlc_list_for_each_entry_safe(fix_base_t, node, n, &info->base_near, link){
		if(is_in_fix_base(&info->base_home, node->base)){
			stlc_list_del(&node->link);
			cl_free(node);
			flag = 1;
		}
	}
	if(flag){
		lbs_save_near_base(user);
	}	
}
void try_amend_far_to_near(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	fix_base_t *node, *n;
	int flag = 0;

	stlc_list_for_each_entry_safe(fix_base_t, node, n, &info->base_far, link){
		if(is_in_fix_base(&info->base_near, node->base)){
			stlc_list_del(&node->link);
			cl_free(node);
			flag = 1;
		}
	}
	if(flag){
		lbs_save_far_base(user);
	}
}

void try_amend_learned_base(user_t *user)
{
	try_amend_far_to_near(user);
	try_amend_near_to_home(user);
}


void try_update_far_near_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	track_base_t *older, *newer, *pos; 
	int has_near = 0, has_far = 0;
	int near_count = 0;
	int prev_near = 0;


	if(stlc_list_empty(&info->base_track)){
		lbs_log(user,"base_track is null");
		return;
	}
	older = stlc_list_first_entry(&info->base_track, track_base_t, link);
	newer = stlc_list_last_entry(&info->base_track, track_base_t, link);


	try_amend_learned_base(user);

	stlc_list_for_each_prev_entry(track_base_t, pos, &info->base_track, link){	

		lbs_log(user,"base.base[0]: %d",pos->base.base[0]);
		
		if(near_count<MAX_NEAR_BASE)
		{
			
			if(is_in_fix_base(&info->base_home, pos->base.base[0])){
				continue;
			}
			near_count++;
			has_near |= try_update_fix_base_one(&info->base_near, pos->base.base[0], 
				pos->time);
		}else{			
			if(is_in_fix_base(&info->base_home, pos->base.base[0])){
				continue;
			}
			if(is_in_fix_base(&info->base_near, pos->base.base[0])){
				continue;
			}
			has_far |= try_update_fix_base_one(&info->base_far, pos->base.base[0], pos->time);
			
		}
	}
	
	lbs_log(user,"save has_near: %d,has_far: %d ",has_near,has_far);	
	if(has_far)
		lbs_save_far_base(user);
	if(has_near)
		lbs_save_near_base(user);
	clear_base_track(user, 0);
}

/*
是否学习到完整基站信息
*/
bool is_learned_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	if(stlc_list_empty(&info->base_home) 
		||stlc_list_empty(&info->base_near) 
		||stlc_list_empty(&info->base_far) ){
		return false;
	}
	return true;
}


/*
处于家的基站集合中
*/
bool is_in_home_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	track_base_t *node;
	
	if(stlc_list_empty(&info->base_home)){
		return false;
	}	
	node = stlc_list_last_entry(&info->base_track, track_base_t, link);

	if(is_in_fix_base(&info->base_home, node->base.base[0])){
		lbs_log(user,"home base: %d",node->base.base[0]);
		return true;
	}
	
	return false;	
}
/*
处于离家较近的基站集合中
*/
bool is_near_home_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	track_base_t *node;
	int i;
	if(stlc_list_empty(&info->base_track)){
		return false;
	}	
	node = stlc_list_last_entry(&info->base_track, track_base_t, link);
	for(i = 0; i < node->base.count; i++){
		if(is_in_fix_base(&info->base_near, node->base.base[i])){
			lbs_log(user,"near base: %d",node->base.base[i]);
			return true;
		}
	}
	return false;	
}

/*
处于离家较远的基站集合中
*/
bool is_far_home_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	track_base_t *node;
	int i;
	if(stlc_list_empty(&info->base_track)){
		return false;
	}	
	node = stlc_list_last_entry(&info->base_track, track_base_t, link);
	for(i = 0; i < node->base.count; i++){
		if(is_in_fix_base(&info->base_far, node->base.base[i])){
			lbs_log(user,"far base: %d",node->base.base[i]);
			return true;
		}
	}
	return false;	
}

int lbs_get_base_status(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	int base_status = BASE_INIT;

	if(stlc_list_empty(&info->base_track)){
		return base_status;
	}
	
	if(is_in_home_base(user)){
		base_status = BASE_IN_HOME;
	}else if(is_near_home_base(user)){
		base_status = BASE_NEAR_HOME;
	}else if(is_far_home_base(user)){
		base_status = BASE_FAR_HOME;
	}
	lbs_log(user,"base_status: %d",base_status);
	
	return base_status;	
}

/*
保存家的基站集合
*/
void lbs_save_home_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	FILE *fp;
	fix_base_t *node, *next;
	time_t now = get_sec();
	if(info == NULL){
		lbs_log(user,"info is null\n");
		return;
	}

	sprintf(short_name, "%012"PRIu64"_%s", user->sn, STR_HOMEBASE);
	sprintf(fn, "%s/%s", info->dir, short_name);
	if ((fp = fopen(fn, "w+")) == NULL) {
		lbs_log(user,"create file %s failed\n", fn);
		goto done;
	}
	stlc_list_for_each_entry_safe(fix_base_t, node, next, &info->base_home, link) {

		if( (now - node->time) >= BASE_HOME_EXPIRE){
			/*一个月都没有到过的基站就删除了*/
			stlc_list_del_only(&node->link);
			cl_free(node);
		}else{
			fprintf(fp, "%u %d\n", node->time, node->base);
		}
		lbs_log(user,"home base: time: %u, base: %d",node->time,node->base);
	}
	fclose(fp);
	
done:
	return;
}

void lbs_save_near_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	FILE *fp;
	fix_base_t *node, *next;
	time_t now = get_sec();
	if(info == NULL){
		lbs_log(user,"info is null");
		return;
	}
	
	sprintf(short_name, "%012"PRIu64"_%s", user->sn, STR_NEARBASE);
	sprintf(fn, "%s/%s", info->dir, short_name);

	if ((fp = fopen(fn, "w+")) == NULL) {
		lbs_log(user,"create file %s failed",fn);		
		goto done;
	}
	
	stlc_list_for_each_entry_safe(fix_base_t, node, next, &info->base_near, link) {

		if( (now - node->time) >= BASE_HOME_EXPIRE){
			/*一个月都没有到过的基站就删除了*/
			stlc_list_del_only(&node->link);
			cl_free(node);
		}else{		
			lbs_log(user,"near base: time:%u, %d",node->time, node->base);
			fprintf(fp, "%u %d\n", node->time, node->base);
		}
		
	}
	fclose(fp);
	
done:
	return;
}

void lbs_save_far_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	FILE *fp;
	fix_base_t *node, *next;
	time_t now = get_sec();
	if(info == NULL){	
		lbs_log(user,"info is null");
		return;
	}
	sprintf(short_name, "%012"PRIu64"_%s", user->sn, STR_FARBASE);
	sprintf(fn, "%s/%s", info->dir, short_name);

	if ((fp = fopen(fn, "w+")) == NULL) {		
		lbs_log(user,"create file %s failed",fn);		
		goto done;
	}

	stlc_list_for_each_entry_safe(fix_base_t, node, next, &info->base_far, link) {
		if( (now - node->time) >= BASE_HOME_EXPIRE){
			/*一个月都没有到过的基站就删除了*/
			stlc_list_del_only(&node->link);
			cl_free(node);
		}else{
			lbs_log(user,"far base: time: %u,%d",node->time, node->base);
			fprintf(fp, "%u %d\n", node->time, node->base);
		}
		
	}
	fclose(fp);
	
done:
	return;
}

/*
加载家的基站集合
*/
void lbs_load_home_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	char buf[1024];
	FILE *fp;
	fix_base_t *node, base;
	time_t now = get_sec();
	if(info == NULL){
		lbs_log(user,"info is null");
		return;
	}

	sprintf(short_name, "%012"PRIu64"_%s", user->sn, STR_HOMEBASE);
	sprintf(fn, "%s/%s", info->dir, short_name);
	if ((fp = fopen(fn, "r")) == NULL) {
		log_err(true, "open file %s failed\n", fn);
		goto done;
	}
	while(1){
		if(fgets(buf, sizeof(buf), fp) == NULL){
			break;
		}
		if(sscanf(buf, "%u %d", &base.time, &base.base) != 2){
			continue;
		}
			
		if( (now - base.time) >= BASE_HOME_EXPIRE){
			continue;
		}
		node = cl_calloc(sizeof(*node), 1);
		if(node == NULL){
			break;
		}
		node->time = base.time;
		node->base = base.base;
		stlc_list_add_tail(&node->link, &info->base_home);		
	}
	if(!stlc_list_empty(&info->base_home)){
		info->has_mark_base = true;
	}else{
		info->has_mark_base = false;
	}

	fclose(fp);

done:
	return;
}

void lbs_load_near_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	char buf[1024];
	FILE *fp;
	fix_base_t *node, base;
	time_t now = get_sec();

	if(info == NULL)
		return;

	sprintf(short_name, "%012"PRIu64"_%s", user->sn, STR_NEARBASE);
	sprintf(fn, "%s/%s", info->dir, short_name);

	if ((fp = fopen(fn, "r")) == NULL) {
		log_err(true, "open file %s failed\n", fn);
		goto done;
	}

	while(1){
		if(fgets(buf, sizeof(buf), fp) == NULL)
			break;
		if(sscanf(buf, "%u %d", &base.time, &base.base) != 2)
			continue;
		if( (now - base.time) >= BASE_HOME_EXPIRE){
			continue;
		}
		node = cl_calloc(sizeof(*node), 1);
		if(node == NULL)
			break;
		node->time = base.time;
		node->base = base.base;
		stlc_list_add_tail(&node->link, &info->base_near);		
	}

	fclose(fp);

done:
	return;
}

void lbs_load_far_base(user_t *user)
{
	lbs_info_t *info = (lbs_info_t*)user->lbs;	
	char fn[CL_MAX_PATH], short_name[CL_MAX_PATH];
	char buf[1024];
	FILE *fp;
	fix_base_t *node, base;
	time_t now = get_sec();

	if(info == NULL)
		return;

	sprintf(short_name, "%012"PRIu64"_%s", user->sn, STR_FARBASE);
	sprintf(fn, "%s/%s", info->dir, short_name);

	if ((fp = fopen(fn, "r")) == NULL) {
		log_err(true, "open file %s failed\n", fn);
		goto done;
	}

	while(1){
		if(fgets(buf, sizeof(buf), fp) == NULL)
			break;
		if(sscanf(buf, "%u %d", &base.time, &base.base) != 2)
			continue;
		if( (now - base.time) >= BASE_HOME_EXPIRE){
			continue;
		}
		node = cl_calloc(sizeof(*node), 1);
		if(node == NULL)
			break;
		node->time = base.time;
		node->base = base.base;
		stlc_list_add_tail(&node->link, &info->base_far);		
	}

	fclose(fp);

done:
	return;
}
/*
	该文件用来标记安装点。格式: (纬度,经度)
	1407208973.063  72.0876254 36.248569
*/
RS lbs_mark_home(user_t *user, lbs_info_t *info, lbs_pos_t *pos)
{
	char fn[CL_MAX_PATH];
	FILE *fp;
	RS ret = RS_ERROR;

	info = (lbs_info_t *)user->lbs;
	sprintf(fn, "%s/%s", info->dir, FN_LBS_HOME);

	info->has_mark_home = true;
	info->home.time = get_dtime();
	info->home.pos.latitude = pos->latitude;
	info->home.pos.longitude = pos->longitude;
	
	if ((fp = fopen(fn, "w+")) == NULL) {
		log_err(true, "create file %s failed\n", fn);
		goto done;
	}

	fprintf(fp, "%lf %.10lf %.10lf\n", info->home.time, info->home.pos.latitude, info->home.pos.longitude);

	fclose(fp);
	
	ret = RS_OK;

done:
	return ret;
}

void lbs_on_in_lan_event(user_t *user)
{
}

void lbs_on_out_lan_event(user_t *user)
{
}

void lbs_update_lan_detect(user_t *user, int is_in_lan)
{
	lbs_info_t *info = (lbs_info_t *)user->lbs;

	if(user->lbs == NULL){
		lbs_user_init(user);
	}
	if ((info = (lbs_info_t *)user->lbs) == NULL)
		return;

	if(is_in_lan)
		info->last_lan_detect = get_sec();
	
	lbs_log(user,"update is_in_lan:[%d,%d]",is_in_lan, info->is_in_lan);  
	
	if(info->is_in_lan == is_in_lan)
		return;
	
	info->is_in_lan = is_in_lan;
	if(is_in_lan){
		if(info->lbs_status == LBS_GOING_HOME){
			do_smart_home(user, LAC_BACKHOME);
			info->lbs_status = LBS_ARRIVE_HOME;
		}
	}else{
	}
}

static double recent_speed(user_t *user)
{
	double speed = 0, distance = 0, times = 0;
	double tm_hou, tm_qian;
	int point = 0;	
	lbs_mark_t *qian = NULL, *hou = NULL;
	lbs_info_t *info = (lbs_info_t *)user->lbs;
	
	stlc_list_for_each_prev_entry(lbs_mark_t, qian, &info->track, link) {
		point++;
		if(hou == NULL){
			hou = qian;
			tm_hou = qian->time;
			continue;
		}
		distance += pos_diff(&qian->pos, &hou->pos);
		times += (hou->time - qian->time);
		tm_qian = qian->time;
		hou = qian;
		//计算最近4分钟的平均速度
		if((tm_hou - tm_qian) >= 4*60){
			if(times){
				speed = distance / times;
			}
			break;
		}
	}
	return speed;	
}

static double pos_diff(lbs_pos_t *a, lbs_pos_t *b)
{
	double pi = 3.1415926535;
	double x, y, diff;

	// 同一经线上那么两点间的距离=纬度差*111km
	y = (a->latitude - b->latitude)*111000;
	
	// 同一纬线上那么两点间的距离=经度差*111km* cos纬度
	x = (a->longitude - b->longitude)*111000 * cos(a->latitude/180*pi);

	diff = sqrt(x*x + y*y);

	return diff;
}

static bool is_home(user_t *user, lbs_info_t *info, lbs_pos_t *pos)
{
	u_int32_t now = get_sec();
	
	if (now - info->last_lan_detect < LBS_LAN_DETECT_DIE) {
		if (  info->has_mark_home ) {
			// 考虑从一个家庭拿到另外一个家庭，需要更新家的位置
			// 判断依据是距离跟以前标记的离的比较远(>=2km)
			if (pos_diff(pos, &info->home.pos) >= lbs_far) {
				lbs_mark_home(user, info, pos);
			}
		} else {
			lbs_mark_home(user, info, pos);
		}

		return true;
	}

	if ( ! info->has_mark_home )
		return false;

	if (pos_diff(&info->home.pos, pos) < lbs_near) {
		return true;
	}

	return false;
}

RS lbs_user_init(user_t *user)
{
	char fn[CL_MAX_PATH];
	lbs_info_t *info;
	FILE *fp;

	if(user->lbs)
		return RS_OK;
	
	if (cl_priv->dir == NULL) {
		log_err(false, "LBS init failed: dir is NULL\n");
		return RS_ERROR;
	}
	
	info = (lbs_info_t *)cl_calloc(sizeof(lbs_info_t), 1);
	user->lbs = info;

	#if 1
	info->current.pos.is_east = 1;
	info->current.pos.is_north = 1;
	info->current.pos.latitude = 57.0;
	info->current.pos.longitude =35.0;
	#endif
	
	sprintf(info->dir, "%s/%012"PRIu64"", cl_priv->dir, user->sn);
	MKDIR(info->dir, 0777);
	STLC_INIT_LIST_HEAD(&info->track);	
	STLC_INIT_LIST_HEAD(&info->base_home);
	STLC_INIT_LIST_HEAD(&info->base_near);
	STLC_INIT_LIST_HEAD(&info->base_far);
	STLC_INIT_LIST_HEAD(&info->base_track);

	lbs_recover_flist(info);

	// read back information
	sprintf(fn, "%s/%s", info->dir, FN_LBS_HOME);
	if ((fp = fopen(fn, "r")) != NULL) {
		if (fscanf(fp, "%lf %lf %lf\n", &info->home.time, &info->home.pos.latitude, &info->home.pos.longitude) == 3) {
			info->has_mark_home = true;
		}

		fclose(fp);
	}
	lbs_set_status(user, LBS_INIT);

	return RS_OK;
}

void lbs_user_free(user_t *user)
{
	lbs_info_t *info;
	lbs_mark_t *node, *next;
	
	info = (lbs_info_t *)user->lbs;
	if (info == NULL)
		return;

	stlc_list_for_each_entry_safe(lbs_mark_t, node, next, &info->track, link) {
		stlc_list_del_only(&node->link);
		cl_free(node);
	}

	cl_free(info);
	user->lbs = NULL;
}

/*
	文件名: time.txt, 如1407208973.txt, 格式(纬度,经度)
	1407208973.063  72.0876254 36.248569
	1407208974.921  72.0876269 36.248591
*/
static void lbs_mark_one(user_t *user, lbs_pos_t *pos, u_int32_t now)
{
	lbs_info_t *info;
	lbs_mark_t *node, *next;

	if (user->lbs == NULL) {
		lbs_user_init(user);
	}

	info = (lbs_info_t *)user->lbs;
	info->current.pos = *pos;
	info->is_base = false;
#if 0
	if (is_home(user, info, pos)) {
		if ( ! stlc_list_empty(&info->track) )
			lbs_save(info);
		do_smart_home(user, LAC_SMART_ON);
		return;
	}
#endif	
	node = cl_calloc(sizeof(lbs_mark_t), 1);
	node->time = get_dtime();
	node->pos = *pos;
	stlc_list_add_tail(&node->link, &info->track);
	if(++info->track_count < MIN_TRACK_COUNT){
		goto done;
	}

	stlc_list_for_each_entry_safe(lbs_mark_t, node, next, &info->track, link) {
		if (now - node->time > MAX_LBS_DURATION) {
			stlc_list_del_only(&node->link);
			cl_free(node);
			if(--info->track_count < MIN_TRACK_COUNT){
				break;
			}
		}
	}
done:
	lbs_proc[info->lbs_status].proc_pos(user);
}

CLIB_API RS lbs_mark(lbs_pos_t *pos)
{
	u_int32_t now;
	user_t *user, *dev;

	now = get_sec();
	
	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == 0)
					continue;

				lbs_mark_one(dev, pos, now);
			}
		} else {
			if (user->sn == 0)
				continue;

			lbs_mark_one(user, pos, now);
		}
	}

	cl_unlock(&cl_priv->mutex);

	return RS_OK;
}

void fill_one_info(user_t *user, lbs_gdb_list_t *gdb)
{
	lbs_info_t *info = (lbs_info_t *)user->lbs;
	lbs_gdb_info_t *one = &gdb->list[gdb->count];

	if(info == NULL)
		return;
	one->sn = user->sn;
	one->distance = info->recent_distance;
	one->speed = info->recent_speed;
	one->status = info->lbs_status;
	one->is_in_lan = info->is_in_lan;
	one->has_mark_home = info->has_mark_home;	
	gdb->count++;
}

CLIB_API void lbs_get_info(lbs_gdb_list_t *info)
{
	user_t *user, *dev;

	info->count = 0;
	
	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if(info->count >= 10)
			break;
		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == 0)
					continue;
				fill_one_info(dev, info);
			}
		} else {
			if (user->sn == 0)
				continue;
			fill_one_info(user, info);
		}
	}

	cl_unlock(&cl_priv->mutex);

}

// 设置位置精度，单位米
CLIB_API void lbs_set_precision(int precision)
{
	lbs_near = precision;
}


#define	ERR_BASE	999900

/* 纬度距离 */
CLIB_API double lbs_get_latitude_diff()
{
	user_t *user, *dev = NULL;
	double diff = -1;
	lbs_info_t *info;
	lbs_mark_t *node;

	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == 0)
					continue;
				break;
			}
		} else {
			if (user->sn == 0)
				continue;

			dev = user;
			break;
		}
	}

	if (dev == NULL) {
		diff = ERR_BASE + 1;
		goto done;
	}

	if ((info = (lbs_info_t *)dev->lbs) == NULL) {
		diff = ERR_BASE + 2;
		goto done;
	}

	if ( ! info->has_mark_home) {
		diff = ERR_BASE + 3;
		goto done;
	}
	
	if (stlc_list_empty(&info->track)) {
		diff = ERR_BASE + 4;
		goto done;
	}

	node = stlc_list_entry(info->track.prev, lbs_mark_t, link);
	
	// 同一经线上那么两点间的距离=纬度差*111km
	diff = (info->home.pos.latitude - node->pos.latitude)*111000;

done:	
	cl_unlock(&cl_priv->mutex);

	return diff;
}

/* 经度距离 */
CLIB_API double lbs_get_longitude_diff()
{
	user_t *user, *dev = NULL;
	double diff = ERR_BASE;
	double pi = 3.1415926535;
	lbs_info_t *info;
	lbs_mark_t *node;

	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == 0)
					continue;
				break;
			}
		} else {
			if (user->sn == 0)
				continue;

			dev = user;
			break;
		}
	}

	if (dev == NULL) {
		diff = ERR_BASE + 1;
		goto done;
	}

	if ((info = (lbs_info_t *)dev->lbs) == NULL) {
		diff = ERR_BASE + 2;
		goto done;
	}

	if ( ! info->has_mark_home) {
		diff = ERR_BASE + 3;
		goto done;
	}
	
	if (stlc_list_empty(&info->track)) {
		diff = ERR_BASE + 4;
		goto done;
	}
	node = stlc_list_entry(info->track.prev, lbs_mark_t, link);
	
	// 同一经线上那么两点间的距离=纬度差*111km
	diff = (info->home.pos.longitude - node->pos.longitude)*111000 * cos(node->pos.latitude/180*pi);

done:	
	cl_unlock(&cl_priv->mutex);

	return diff;
}

CLIB_API RS lbs_mark_home_test()
{
	user_t *user, *dev;
	lbs_info_t *info;
	
	cl_lock(&cl_priv->mutex);

	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == 0)
					continue;

				lbs_user_init(dev);
				if ((info = (lbs_info_t *)dev->lbs) != NULL)
					info->last_lan_detect = get_sec();
			}
		} else {
			if (user->sn == 0)
				continue;

			lbs_user_init(user);
			if ((info = (lbs_info_t *)user->lbs) != NULL)
				info->has_mark_home = false;
				info->last_lan_detect = get_sec();
		}
	}

	cl_unlock(&cl_priv->mutex);

	return RS_OK;
}

RS query_home_location(user_t *user)
{
	ucc_session_t *s = (ucc_session_t*)user->uc_session;
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	char buf[512] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	sa_net_location_t* st = (sa_net_location_t*)(uo+1);
	
	if (s == NULL || info == NULL)
		return RS_ERROR;
	
	if(info->home.pos.latitude && info->home.pos.longitude)
		return RS_OK;
//	if(info->query_cnt > 1)
//		return RS_OK;
//	info->query_cnt++;
	st->action = LAC_GET;	
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_LOCATION, sizeof(ucp_obj_t)+sizeof(*st));
	sa_ctrl_obj_value(s, UCA_GET, false, 1, uo, sizeof(*uo)+sizeof(*st));
	return RS_OK;
}

RS do_smart_home(user_t *user, u_int8_t action)
{
	ucc_session_t *s = (ucc_session_t*)user->uc_session;
	lbs_info_t *info = (lbs_info_t*)user->lbs;
	char buf[512] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	sa_net_location_t* st = (sa_net_location_t*)(uo+1);
	
	if (s == NULL || info == NULL)
		return RS_ERROR;
	
	st->is_east = info->current.pos.is_east;
	st->is_north = info->current.pos.is_north;
	st->action = action;
	st->pad = 0;
	st->distance = 0;
	st->speed = ntohs(info->current.pos.speed);
	sprintf(st->latitude, "%f", info->current.pos.latitude);
	sprintf(st->longitude, "%f", info->current.pos.longitude);
	
	lbs_log(user,"%s,%s, action=%d",
		st->latitude,
		st->longitude,
		action);
	
	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_LOCATION, sizeof(ucp_obj_t)+sizeof(*st));
	sa_ctrl_obj_value(s, UCA_SET, false, 1, uo, sizeof(*uo)+sizeof(*st));
	
	return RS_OK;
}

RS do_set_home_location(user_t *user)
{
	ucc_session_t *s = (ucc_session_t*)user->uc_session;
	lbs_info_t *info;
	char buf[512] = {0};
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	sa_net_location_t* st = (sa_net_location_t*)(uo+1);
	
	if (s == NULL)
		return RS_ERROR;
		
	if (user->lbs == NULL)
		return RS_ERROR;
	
	info = (lbs_info_t*)user->lbs;
		
	
	st->is_east = info->current.pos.is_east;
	st->is_north = info->current.pos.is_north;
	st->action = LAC_PUT;	
	st->pad = 0;
	st->distance = 0;;
	st->speed = ntohs(info->current.pos.speed);
	sprintf(st->latitude, "%f", info->current.pos.latitude);
	sprintf(st->longitude, "%f", info->current.pos.longitude);

	fill_net_ucp_obj(uo, UCOT_SYSTEM, UCSOT_SYS_USER, UCAT_SYS_LOCATION, sizeof(ucp_obj_t)+sizeof(*st));
	sa_ctrl_obj_value(s, UCA_SET, false, 1, uo, sizeof(*uo)+sizeof(*st));
	return RS_OK;		
}

bool on_get_home_location(user_t *user, void *uo)
{
	ucp_obj_t *obj = (ucp_obj_t*)uo;
	sa_net_location_t* st = (sa_net_location_t*)(obj+1);
	lbs_info_t *info = (lbs_info_t *)user->lbs;
	lbs_pos_t pos = {0, 0, 0, 0, 0, 0.0f, 0.0f};
	
	if(user->lbs == NULL)
		return false;
	
	if(obj->param_len < (sizeof(*st)))
		return false;

	if(st->latitude[0] && st->longitude[0]){
		info->home.pos.latitude = atof(st->latitude);
		info->home.pos.longitude = atof(st->longitude);
		info->home.pos.is_east = st->is_east;
		info->home.pos.is_north = st->is_north;		
		if(info->home.pos.latitude && info->home.pos.longitude){
			info->has_mark_home = true;			
		}
	}	
	
	return false;	
}

int diff_base(lbs_base_t *a, lbs_base_t *b)
{
	int i, j;
	int same = 0;
	
	for(i = 0; i < a->count; i++){
		for(j = 0; j < b->count; j++){
			if(a->base[i] == b->base[j]){
				same++;
			}
		}
	}
	return same;
}

static bool _is_smart_home_on_enable(user_t *user)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
    
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return false;
	}
    
	if (!(ac = sma->sub_ctrl) || user->sub_type != IJ_808) {
		return false;
	}
    
 	if(ac->air_info.smart_on_enable)
		return true;

	if (ac->com_udp_dev_info.is_support_public_smart_on) {
		return true;
	}
	
	return false;
}

static bool _is_smart_home_on_enable2(user_t *user)
{
	smart_appliance_ctrl_t* sma;
	smart_air_ctrl_t* ac;
    
	if ( (sma = user->smart_appliance_ctrl) == NULL) {
		return false;
	}
    
	if (!(ac = sma->sub_ctrl)) {
		return false;
	}

	if (((user->sub_type == IJ_TEST_DEV && 
		user->ext_type == ETYPE_IJ_TEST_XY) || 
		(user->sub_type == IJ_KXM_DEVICE && 
		user->ext_type == ETYPE_IJ_XY_THERMOSTAT)) &&
		ac->smart_home_enable) {
		return true;
	}
    
	return false;
}

static bool is_smart_home_on_enable(user_t *user)
{
	bool ret = false;
	
	switch(user->sub_type) {
	case IJ_TEST_DEV:
		ret = _is_smart_home_on_enable2(user);
		break;
	default:
		ret = _is_smart_home_on_enable(user);	
		break;
	}

	return ret;
}

static void lbs_mark_base_one(user_t *user, lbs_base_t *base, u_int32_t now)
{
	lbs_info_t *info;
	track_base_t *node, *next;
	
	if(!is_smart_home_on_enable(user)){
		lbs_log(user," is smart disabled");
		return;
	}
	
	if (user->lbs == NULL) {
		lbs_log(user," user lbs is null");
		lbs_user_init(user);
	}
	info = (lbs_info_t *)user->lbs;
	info->is_base = true;
	
	if(!stlc_list_empty(&info->base_track)){
		node = stlc_list_last_entry(&info->base_track, track_base_t, link);
		if(node->base.base[0] == base->base[0]){
			/*还在同一个基站*/
			node->time_end = now;
			lbs_log(user,"base=%d,time:%u,time_end:%u,is_in_lan:[%d, %d]",
				base->base[0],
				node->time,
				node->time_end,
				node->is_in_lan,
				info->is_in_lan
			);
			if(node->is_in_lan == info->is_in_lan)
				return;
			/*局域网扫描状态有变化*/
			node->is_in_lan = info->is_in_lan;
			goto done;
		}
	}

	
	node = cl_calloc(sizeof(*node), 1);
	node->time = now;
	node->time_end = now;
	node->base = *base;
	node->is_in_lan = info->is_in_lan;	
	
	stlc_list_add_tail(&node->link, &info->base_track);
	if(++info->track_count < MIN_TRACK_COUNT){
		goto done;
	}
	stlc_list_for_each_entry_safe(track_base_t, node, next, &info->base_track, link) {
		if (now - node->time > MAX_LBS_DURATION) {
			stlc_list_del_only(&node->link);
			cl_free(node);
			if(--info->track_count < MIN_TRACK_COUNT){
				break;
			}
		}
	}
	
done:
	lbs_log(user," is_in_lan=%d,base=%d,status:%d,time:%u,time_end:%u",
		info->is_in_lan,
		base->base[0],
		info->lbs_status,
		node->time,
		node->time_end
	);
	
	lbs_proc[info->lbs_status].proc_pos(user);
}
CLIB_API RS lbs_mark_base(lbs_base_t *base)
{
	u_int32_t now;
	user_t *user, *dev;

	if((base->count <= 0) || (base->base[0] == 0))
		return RS_INVALID_PARAM;
	
	now = get_sec();
	cl_lock(&cl_priv->mutex);
	
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == 0)
					continue;
				lbs_mark_base_one(dev, base, now);
			}
		} else {
			if (user->sn == 0)
				continue;
			lbs_mark_base_one(user, base, now);
		}
	}

	cl_unlock(&cl_priv->mutex);

	return RS_OK;
}

CLIB_API RS lbs_going_home_on(cl_handle_t handle)
{
	user_t *user;
	RS ret = RS_OK;
	lbs_info_t *info;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, handle)) == NULL) {
		ret = RS_NOT_FOUND;
		goto done;
	}
	if (user->lbs == NULL) {
		lbs_user_init(user);
	}
	info = (lbs_info_t *)user->lbs;
	ret = do_smart_home(user, LAC_SMART_ON);
	if(ret == RS_OK)
		info->lbs_status = LBS_GOING_HOME;
		
	
done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}