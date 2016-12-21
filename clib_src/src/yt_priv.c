/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: yt_priv.c
**  File:    yt_priv.c
**  Author:  liubenlong
**  Date:    09/06/2015
**
**  Purpose:
**    月兔.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "udp_device_common_priv.h"
#include "cl_yt.h"
#include "yt_priv.h"
/* Macro constant definitions. */
#define MAX_DAY 365

enum {
	YT_FR_GUA = 0,//定频挂机，
	YT_FR_GUI,//定频柜机，	
	YT_FC_GUA,//变频挂机
	YT_FC_GUI,//变频柜机
};

/* Type definitions. */

typedef struct {
	char *name;//内机型号
	u_int16_t code;//内机代码
	u_int16_t cool_power;//制冷功率
	u_int16_t ele_assist_power;//电辅热功率
	u_int16_t hot_power;//辅热功率
}yt_l_type_t;


/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
yt_l_type_t yt_sys_type[] = {
{"KFR-35G/BP2DN1Y-YC1(A3)@", 2351,0,1200,0},
{"KFR-35G/BP2DN1Y-YC1(B3)@", 2352,0,1200,0},
{"KFR-35G/BP2DN1Y-YC(A3)@", 2353,0,1200,0},
{"KFR-35G/BP2DN1Y-YC(B3)@", 2354,0,1200,0},
{"KFR-26G/BP2DN1Y-YC(A3)@", 2355,0,1000,0},
{"KFR-26G/BP2DN1Y-YC(B3)@", 2356,0,1000,0},
{"KFR-35G/BP2DN1Y-VB1(A3)@", 2357,0,1200,0},
{"KFR-35G/BP2DN1Y-VB1(B3)@", 2358,0,1200,0},
{"KFR-35G/BP2DN1Y-VB(A3)@", 2359,0,1200,0},
{"KFR-35G/BP2DN1Y-VB(B3)@", 2360,0,1200,0},
{"KFR-26G/BP2DN1Y-VB(A3)@", 2361,0,1000,0},
{"KFR-26G/BP2DN1Y-VB(B3)@", 2362,0,1000,0},
{"KFR-26G/BP2DN1Y-CA(A3)@", 2363,0,1000,0},
{"KFR-35G/BP2DN1Y-CA(A3)@", 2364,0,1200,0},
{"KFR-51L/BP2DN1Y-BA(A3)@", 2365,0,1800,0},
{"KFR-72L/BP2DN1Y-BA(A3)@", 2366,0,2400,0},
{"KFR-51L/BP2DN1Y-NA(A3)@", 2367,0,1800,0},
{"KFR-72L/BP2DN1Y-NA(A3)@", 2368,0,2400,0},
{"KF-23G/Y-CA(A3)@", 2369,717,0,0},
{"KFR-23G/DY-CA(A3)@", 2370,717,1000,683},
{"KF-25G/Y-CA(A3)@", 2371,760,0,0},
{"KFR-25G/DY-CA(A3)@", 2372,760,1000,757},
{"KF-35G/Y-CA(A3)@", 2373,1064,0,0},
{"KFR-35G/DY-CA(A3)@", 2374,1064,1200,1090},
{"KFR-25G/DY-FT (A3)@", 2375,760,1000,757},
{"KFR-25G/DY-FT (B3)@", 2376,760,1000,757},
{"KFR-35G/DY-FT (A3)@", 2377,1064,1200,1090},
{"KFR-35G/DY-FT (B3)@", 2378,1064,1200,1090},
{"KFR-25G/DY-FT1 (A3)@", 2379,760,1000,757},
{"KFR-25G/DY-FT1 (B3)@", 2380,760,1000,757},
{"KFR-35G/DY-FT1 (A3)@", 2381,1064,1200,1090},
{"KFR-35G/DY-FT1 (B3)@", 2382,1064,1200,1090},
{"KFR-48G/DY-FG (A3)@", 2383,1470,1200,1510},
{"KF-26G/Y-YC(A3)@", 2384,805,0,0},
{"KF-26G/Y-YC(B3)@", 2385,805,0,0},
{"KFR-26G/DY-YC(A3)@", 2386,805,1000,806},
{"KFR-26G/DY-YC(B3)@", 2387,805,1000,806},
{"KF-35G/Y-YC1(A3)@", 2388,1064,0,0},
{"KF-35G/Y-YC1(B3)@", 2389,1064,0,0},
{"KFR-35G/DY-YC1(A3)@", 2390,1064,1200,1090},
{"KFR-35G/DY-YC1(B3)@", 2391,1064,1200,1090},
{"KFR-35G/DY-YC(A3)@", 2392,1064,1200,1090},
{"KFR-35G/DY-YC(B3)@", 2393,1064,1200,1090},
{"KF-26G/Y-VB(A3)@", 2394,805,0,0},
{"KF-26G/Y-VB(B3)@", 2395,805,0,0},
{"KFR-26G/DY-VB(A3)@", 2396,805,1000,806},
{"KFR-26G/DY-VB(B3)@", 2397,805,1000,806},
{"KF-35G/Y-VB1(A3)@", 2398,1064,0,0},
{"KF-35G/Y-VB1(B3)@", 2399,1064,0,0},
{"KFR-35G/DY-VB1(A3)@", 2400,1064,1200,1090},
{"KFR-35G/DY-VB1(B3)@", 2401,1064,1200,1090},
{"KFR-35G/DY-VB(A3)@", 2402,1064,1200,1090},
{"KFR-35G/DY-VB(B3)@", 2403,1064,1200,1090},
{"KFR-51L/DY-BA(A3)@", 2404,1632,1800,1643},
{"KFR-72L/DY-BA(A3)@", 2405,2354,2400,2432},
{"KFR-51L/DY-HD(A3)@", 2406,1632,1800,1643},
{"KFR-72L/DY-HD(A3)@", 2407,2354,2400,2432},
{"KFR-51L/DY-NA(A3)@", 2408,1632,1800,1643},
{"KFR-72L/DY-NA(A3)@", 2409,2354,2400,2432},
};

#define 	YT_SYS_TYPE_NUM			(sizeof(yt_sys_type)/sizeof(yt_sys_type[0]))



u_int8_t yt_get_rls(u_int16_t code)
{
	if (code >= 2365 &&
		code <= 2368) {
		return 1;
	}
	
	if (code >= 2404 &&
		code <= 2405) {
		return 1;
	}

	if (code >= 2408 &&
		code <= 2409) {
		return 1;
	}


	return 0;
}

bool yt_get_ac_info(u_int8_t *psn, cl_yt_ac_type_t *ptmp)
{
	int i;
	u_int64_t sn = 0;
	u_int16_t code = 0;
	yt_l_type_t *plt = NULL;
	bool found = false;

	sn = atoll(psn);
	code = (u_int16_t)(sn/10000000000000);

	for(i = 0; i < YT_SYS_TYPE_NUM; i++) {
		plt = &yt_sys_type[i];
		if (plt->code == code) {
			found = true;
			break;
		}
	}

	if (!found) {
		return false;
	}

	ptmp->index = i;
	ptmp->freq_type = (plt->cool_power == 0)?1:0;
	ptmp->cool_type = (plt->ele_assist_power != 0)?1:0;;
	ptmp->rl_swing = yt_get_rls(code);
	ptmp->cool_power = plt->cool_power;
	ptmp->hot_power = plt->hot_power;
	ptmp->ele_assist_power = plt->ele_assist_power;
	ptmp->sn = sn;

	return true;
}

void ac_type_order(cl_yt_ac_type_t *ptmp)
{
	ptmp->index = htons(ptmp->index);
	ptmp->cool_power = htons(ptmp->cool_power);
	ptmp->hot_power = htons(ptmp->hot_power);
	ptmp->ele_assist_power = htons(ptmp->ele_assist_power);
	ptmp->sn = ntoh_ll(ptmp->sn);
}

char *ac_type_get_name(u_int16_t index)
{
	char *def = "KFR-35G/DY-CA(A3)@";
	
	if (index < YT_SYS_TYPE_NUM) {
		return yt_sys_type[index].name;
	} else {
		return def;
	}
}

u_int8_t ac_sys_type_get(u_int16_t index)
{
	yt_l_type_t *plt = NULL;

	if (index < YT_SYS_TYPE_NUM) {
		plt = &yt_sys_type[index];
	} else {
		plt = &yt_sys_type[18];
	}

	if (plt->cool_power) {
		if (plt->ele_assist_power >= 1800) {
			return YT_FR_GUI;
		}

		return YT_FR_GUA;
	}

	if (plt->ele_assist_power >= 1800) {
		return YT_FC_GUI;
	}

	return YT_FC_GUA;
}

bool _yt_sn(user_t *user, u_int8_t *ptmp, RS *ret)
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_yt_info_t* pyti = (cl_yt_info_t*)ac->com_udp_dev_info.device_info;
	char buf[1024];
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_yt_ac_type_t *ptmp_net = (cl_yt_ac_type_t *)(&uo[1]);
	cl_yt_ac_type_t *ptmp_local = &pyti->ac_info;
	cl_yt_ac_type_t tmp;
	int len = 0;
	
	if (!ac || !pyti) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	memcpy((void *)&tmp, (void *)ptmp_local, sizeof(tmp));
	if (!yt_get_ac_info(ptmp, &tmp)) {
		*ret = RS_INVALID_PARAM;
		return false;
	}

	if (memcmp((void *)ptmp_local, (void *)&tmp, sizeof(tmp))) {
		if (tmp.cool_type) {
			pyti->sn_err = 0;
		} else {
			if (pyti->ac_info_valid && 
				pyti->stat_info_valid && 
				pyti->work_info_valid) {
				//单冷情况下不能有制热模式，或者自动模式下四通阀开启
				if ((pyti->mode == YT_MODE_HOT) || 
					((pyti->mode == YT_MODE_AUTO) && 
					pyti->four_valve)) {
					pyti->sn_err = 1;
				}
			}
		}
		
		strcpy(pyti->name, yt_sys_type[tmp.index].name);
		pyti->sys_type = ac_sys_type_get(tmp.index);
		memcpy((void *)ptmp_local, (void *)&tmp, sizeof(tmp));
		ac_type_order(&tmp);
		memcpy((void *)ptmp_net, (void *)&tmp, sizeof(tmp));
		len = sizeof(tmp);
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_YUETU, UCAT_IA_YUETU_SN,len);
		sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET, true, 0x1, buf, sizeof(*uo)+len);
		//控制后立马更新数据
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	   	event_cancel_merge(user->handle);
	}

	*ret = RS_OK;
	
	return true;	
}

static u_int16_t get_yday(u_int16_t year, u_int16_t month, u_int16_t day)
{
	int i;
	u_int16_t index = 0;
	u_int8_t month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((year % 4 == 0 && year % 100 != 0) || year % 400== 0) {
		month_day[1] = 29;
	} else {
		month_day[1] = 28;
	}

	for(i = 0; i < month && i < 12; i++) {
		index += month_day[i];
	}

	return (index + day)%MAX_DAY;
}

static bool _yt_ele_sync(cl_elec_days_stat_info *pdsi, cl_yt_info_t* pyti)
{
	u_int16_t day1[MAX_DAY];
	struct tm lm;
	time_t last_time = 0;
	u_int16_t index_cur = 0;
	u_int16_t index_begin = 0;
	u_int16_t index_end = 0;
	u_int32_t ele_phase= 0;
	
	if (pdsi->days_count > MAX_DAY ||
		!pdsi->elec_data) {
		return false;
	}

	last_time = pdsi->nearest_data_time;
	localtime_r(&last_time, &lm);
	memset(day1, 0, sizeof(day1));
	memcpy((void *)day1, (void *)pdsi->elec_data, sizeof(u_int16_t)*pdsi->days_count);

	index_cur = get_yday(lm.tm_year + LOCALTIME_YEAR_BASE, lm.tm_mon + 1, lm.tm_mday);
	index_begin = get_yday(pyti->ele_time.begin_year, pyti->ele_time.begin_month, pyti->ele_time.begin_day);
	index_end = get_yday(pyti->ele_time.end_year, pyti->ele_time.end_month, pyti->ele_time.end_day);

	//映射到day1下标上
	//begin
	index_begin = (index_cur - index_begin + MAX_DAY)%MAX_DAY;
	index_end = (index_cur - index_end + MAX_DAY)%MAX_DAY;

	while(index_begin != index_end) {
		ele_phase += day1[index_begin];
		index_begin = (index_begin - 1)%MAX_DAY;
	}

	pyti->ele_phase = ele_phase/10;

	return true;
}

bool _yt_query_phase_ele(user_t *user, u_int8_t *ptmp, RS *ret) 
{
	smart_air_ctrl_t* ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	cl_yt_info_t* pyti = (cl_yt_info_t*)ac->com_udp_dev_info.device_info;
	cl_query_ele_time_t *plocal = &pyti->ele_time;
	cl_query_ele_time_t *pnet = (cl_query_ele_time_t *)ptmp;

	if (!ac) {
		*ret = RS_ERROR;
		return false;
	}

	*ret = RS_OK;
	if (memcmp((void *)plocal, (void *)pnet, sizeof(*plocal)) == 0) {
		return true;
	}

	memcpy((void *)plocal, (void *)pnet, sizeof(*plocal));
#if 0	
	if (pyti->ele_phase == 0) {
		pyti->ele_phase = 1000;
	} else {
		pyti->ele_phase += 10;
	}
#else
	if (!_yt_ele_sync(&ac->air_info.elec_days_info, pyti)) {
		*ret = RS_ERROR;
		return false;
	}
#endif

	//控制后立马更新数据
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    event_cancel_merge(user->handle);

	return true;
}
CLIB_API RS cl_yt_scan_sn(cl_handle_t dev_handle, char *sn)
{
	u_int8_t buff[100];
	
	CL_CHECK_INIT;

	if (sn) {
		strncpy(buff, sn, sizeof(buff));
	}
	
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_YT,
		ACT_YT_SN,(u_int8_t *)buff,sizeof(buff));
}

CLIB_API RS cl_yt_query_ele(cl_handle_t dev_handle, cl_query_ele_time_t *time)
{
	CL_CHECK_INIT;
	
	return cl_send_var_data_notify(&cl_priv->thread_main,dev_handle,CLNE_COMMON_UDP_YT,
		ACT_YT_QUERY_ELE,(u_int8_t *)time,sizeof(*time));
}




