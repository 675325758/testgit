/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: qpcp_priv.c
**  File:    qpcp_priv.c
**  Author:  liubenlong
**  Date:    05/13/2015
**
**  Purpose:
**    千帕茶盘.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_qpcp.h"
#include "qpcp_priv.h"
#include "udp_device_common_priv.h"
/* Macro constant definitions. */


/* Type definitions. */

/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
static void _qpcp_fill_work_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_work_t *pwork);
static void _qpcp_fill_water_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_water_t *pwork);
static void _qpcp_fill_hand_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_scp_t *php);
static void _qpcp_fill_sdel_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_sdel_t *pdel);
static void _qpcp_fill_sadd_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_sp_t *psp);
static void _qpcp_fill_sexe_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_exe_t *pexe);
static void _qpcp_fill_squery_pkt(void *s, cl_qpcp_priv_t* priv,ucp_obj_t* uo);

static void _qpcp_update_data_id_modify(smart_air_ctrl_t* air_ctrl, cl_qpcp_priv_t *priv, cp_id_hd_t *phd ,u_int16_t param_len);
static void _qpcp_update_data_scene_modify(cl_qpcp_priv_t *priv, cl_qpcp_sp_t *psp, u_int16_t param_len);

void *qpcp_priv_init()
{
	cl_qpcp_priv_t *p;

	p = cl_calloc(sizeof(*p), 1);
	if(p){
		STLC_INIT_LIST_HEAD(&p->scene_list);
		// TODO:这里预设一些情景
	}
	return (void*)p;
}

bool _qpcp_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
	int n;	
	cl_qpcp_info_t * bi;
	cl_qpcp_info_t* src;
	cl_qpcp_priv_t *priv;
	cp_scene_node_t *node, *next;
	cl_qpcp_sh_t *psh = NULL;
	cl_qpcp_sp_t *psp = NULL;

	
	if(!di || !ac || !ac->com_udp_dev_info.device_info){
		return false;
	}
	
	priv = ac->com_udp_dev_info.device_info;
	src = &priv->cl_info;
	bi = cl_calloc(sizeof(*bi) , 0x1);
	if(!bi){
		return false;
	}

	di->device_info = bi;
	memcpy(bi, src, sizeof(*bi));
	bi->pscene = NULL;
	n = 0;
	stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, &priv->scene_list, link) {
		n++;
	}

	if (n == 0) {
		return true;
	}

	psh = cl_calloc((n*sizeof(cl_qpcp_sp_t) + sizeof(cl_qpcp_sh_t)), 1);
	if (!psh) {
		return true;
	}

	bi->pscene = psh;
	psh->num = n;
	psp = (cl_qpcp_sp_t *)psh->scene;
	stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, &priv->scene_list, link) {
		psp->scene_id = node->scene.scene_id;
		psp->create_time = node->scene.create_time;
		strncpy((char*)psp->name, (char*)node->scene.name,sizeof(psp->name)-1);
		memcpy(&psp->com_param, &node->scene.com_param, sizeof(psp->com_param));
		psp++;
	}
	
	return true;
}

void _free_qpcp_info(cl_qpcp_info_t* info)
{
	if(!info){
		return;
	}

	if (info->pscene) {
		cl_free(info->pscene);
		info->pscene = NULL;
	}
}

void _free_qp_pot_info(cl_qp_pot_info_t* info)
{
    if (info && info->scenes) {
        cl_free(info->scenes);
        info->scenes = NULL;
    }
}


void _free_qpcp_sdk_info(cl_qpcp_priv_t* priv)
{
	cp_scene_node_t *node, *next;
	
	if(!priv){
		return;
	}

	stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, &priv->scene_list, link) {
		stlc_list_del(&node->link);
		cl_free(node);
	}
}

static void _qpcp_fill_work_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_work_t *pwork)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_WORK,sizeof(*pwork));
	pwork->work = priv->cl_info.cur_onof;
}

static void _qpcp_fill_water_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_water_t *pwork)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_WATER,sizeof(*pwork));
}

static void _qpcp_fill_hand_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_scp_t *php)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_CTRL,sizeof(*php));
}

static void _qpcp_fill_sdel_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_sdel_t *pdel)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_PLAN,sizeof(*pdel));
}

static void _qpcp_fill_sadd_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_sp_t *psp)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_PLAN,sizeof(*psp));
}

static void _qpcp_fill_sexe_pkt(cl_qpcp_priv_t* priv,ucp_obj_t* uo,cl_qpcp_pri_exe_t *pexe)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_PLAN_EXECUTE,sizeof(*pexe));
}

static void _qpcp_fill_squery_pkt(void *s, cl_qpcp_priv_t* priv,ucp_obj_t* uo)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_PLAN_ID,0);
	sa_ctrl_obj_value(s, UCA_GET, false, 0x1, uo, sizeof(*uo));
}

static void _qpcp_fill_queryid_pkt(ucp_obj_t* uo, u_int16_t len)
{
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_PLAN,len);
}

static void scene_check_del(struct stlc_list_head *list, cp_id_hd_t *phd)
{
	int i;
	bool found = false;
	cp_id_modify_t *pim = NULL;
	cp_scene_node_t *node, *next;

	stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, list, link) {
		found = false;
		for(i = 0; i < phd->count; i++) {
			pim = &phd->id_modify[i];
			if (node->scene.scene_id == pim->scene_id) {
				found = true;
				break;
			}		
		}

		//del
		if (!found) {
			stlc_list_del(&node->link);
			cl_free(node);
		}
	}	
}

#if 0
static void scene_check_add(struct stlc_list_head *list, cp_id_hd_t *phd)
{
	int i;
	bool found = false;
	cp_id_modify_t *pim = NULL;
	cp_scene_node_t *node, *next;
	cp_scene_node_t *ptemp;

	for(i = 0; i < phd->count; i++) {
		found = false;
		pim = &phd->id_modify[i];		
		stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, list, link) {
			if (node->scene.scene_id == pim->scene_id){
				found = true;
				break;
			}
		}

		//add
		if (!found) {
			ptemp = cl_calloc(sizeof(cp_scene_node_t), 1);
			if (!ptemp) {
				return;
			}
			STLC_INIT_LIST_HEAD(&ptemp->link);
			stlc_list_add_tail(&ptemp->link, list);
			ptemp->scene.scene_id = pim->scene_id;
			ptemp->modify = pim->modify;
		}
	}
}

#endif
static bool scene_order_check(cp_id_hd_t *phd, u_int16_t param_len)
{
	int i;
	cp_id_modify_t *pim = NULL;

	if (!phd) {
		return false;
	}

	phd->count = htons(phd->count);
//	log_debug("phd->count=%u\n", phd->count);
	if (0 == phd->count) {
		return true;
	}
	
	if (phd->count*sizeof(cp_id_modify_t) < param_len) {
		return false;
	}
	
	for(i = 0; i < phd->count; i++) {
		pim = &phd->id_modify[i];
		pim->scene_id = htons(pim->scene_id);
		pim->modify = htons(pim->modify);
	}

	return true;
}

#define QPCP_QUERY_ID_MAX			13
static void scene_check_query(smart_air_ctrl_t* air_ctrl, struct stlc_list_head *list, cp_id_hd_t *phd)
{
	int i, n = 0;
	int len;
	u_int8_t buff[128];
	bool need_query;
	cp_scene_node_t *node, *next;	
	ucp_obj_t* uo = (ucp_obj_t*)buff;
	u_int16_t *pcount = (u_int16_t *)(&uo[1]);
	u_int16_t *pid = &pcount[1];
	cp_id_modify_t *pim = NULL;
	

	memset(buff, 0, sizeof(buff));	

	for(i = 0, n = 0; i < phd->count; i++) {
		need_query = true;
		pim = &phd->id_modify[i];		
		stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, list, link) {
			if (node->scene.scene_id == pim->scene_id &&
				node->modify == pim->modify) {
				need_query = false;
				break;
			}
		}

		if (need_query) {
			pid[n++] = htons(pim->scene_id);
		}

		if (n == QPCP_QUERY_ID_MAX) {
			*pcount = htons(n);
			len = (n+1)*sizeof(u_int16_t);
			_qpcp_fill_queryid_pkt(uo, len);
			sa_ctrl_obj_value(air_ctrl->sac->user->uc_session, UCA_GET, false, 0x1, uo, sizeof(*uo)+len);
			memset(buff, 0, sizeof(buff));
			n = 0;
		}
	}
	
	if (n > 0) {
		//这里处理下，字节四字节对齐
		if (n%2 == 0) {
			*pcount = htons(n);
			n++;
		} else {
			*pcount = htons(n);
		}
		len = (n+1)*sizeof(u_int16_t);
		_qpcp_fill_queryid_pkt(uo, len);
		sa_ctrl_obj_value(air_ctrl->sac->user->uc_session, UCA_GET, false, 0x1, uo, sizeof(*uo)+len);
	}
}

static void _qpcp_update_data_id_modify(smart_air_ctrl_t* air_ctrl, cl_qpcp_priv_t *priv, cp_id_hd_t *phd, u_int16_t param_len)
{
	//orderchek
	if (!scene_order_check(phd, param_len)) {
		return;
	}
	//先做删除操作
	scene_check_del(&priv->scene_list, phd);

	//报文查询id
	scene_check_query(air_ctrl, &priv->scene_list, phd);

	//情景添加，为了modify值,去掉，在更新数据时在添加节点
	//scene_check_add(&priv->scene_list, phd);
}

static void _qpcp_update_data_scene_modify(cl_qpcp_priv_t *priv, cl_qpcp_sp_t *psp, u_int16_t param_len)
{
	int len = 0;
	cp_scene_node_t *node, *next;
	cp_scene_node_t *ptemp = NULL;
	bool found = false;

	while(len + sizeof(*psp) <= param_len) {
		found = false;
		psp->scene_id = htons(psp->scene_id);
		psp->create_time = htonl(psp->create_time);
		stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, &priv->scene_list, link) {
			if (node->scene.scene_id == psp->scene_id) {
				node->scene.create_time = psp->create_time;
				memcpy(node->scene.name, psp->name, sizeof(node->scene.name));
				node->scene.name[sizeof(node->scene.name)-1] = 0;
				memcpy(&node->scene.com_param, &psp->com_param, sizeof(node->scene.com_param));
				found = true;
				break;
			}
		}
		
		//如果没有就添加
		if (!found) {
			ptemp = cl_calloc(sizeof(cp_scene_node_t), 1);
			if (ptemp) {
				STLC_INIT_LIST_HEAD(&ptemp->link);
				stlc_list_add_tail(&ptemp->link, &priv->scene_list);
				ptemp->scene.scene_id = psp->scene_id;
				ptemp->modify = 0;	
				ptemp->scene.create_time = psp->create_time;
				memcpy(ptemp->scene.name, psp->name, sizeof(ptemp->scene.name));
				ptemp->scene.name[sizeof(ptemp->scene.name)-1] = 0;
				memcpy(&ptemp->scene.com_param, &psp->com_param, sizeof(ptemp->scene.com_param));
			}
		}
		
		len += sizeof(*psp);	
		psp++;
	}
}

static u_int16_t _qpcp_get_scene_num(struct stlc_list_head *pscene_list)
{
	u_int16_t num = 0;
	cp_scene_node_t *node, *next;

	stlc_list_for_each_entry_safe(cp_scene_node_t, node, next, pscene_list, link) {
		num++;
	}

	return num;
}

bool _qpcp_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_qpcp_priv_t *pqp;
	char buf[256] = {0};
	u_int8_t *ptmp = NULL;
	cl_qpcp_scp_t *pscp_tmp = NULL;
	u_int16_t *p16_temp = NULL;
	ucp_obj_t* uo = (ucp_obj_t*)buf;	
	cl_qpcp_pri_work_t *pwork = (cl_qpcp_pri_work_t *)(&uo[1]);	
	cl_qpcp_pri_water_t *pwater = (cl_qpcp_pri_water_t *)(&uo[1]);
	cl_qpcp_pri_sdel_t *psdel = (cl_qpcp_pri_sdel_t *)(&uo[1]);	
	cl_qpcp_scp_t *php = (cl_qpcp_scp_t *)(&uo[1]);	
	cl_qpcp_sp_t *psp = (cl_qpcp_sp_t *)(&uo[1]);
	cl_qpcp_pri_exe_t *pexe = (cl_qpcp_pri_exe_t *)(&uo[1]);
    usp_qpcp_reset_pkt_t* qr = (usp_qpcp_reset_pkt_t* )(uo+1);
	
	int len = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ptmp = (u_int8_t *)info->u.u8_data;
	
	if(!user->smart_appliance_ctrl) {
		log_err(false, "_hx_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "_qpcp_proc_notify error handle %08x\n",info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}
	
	pqp = ac->com_udp_dev_info.device_info;

	switch(info->action){
	case ACT_QPCP_CTRL_ONOFF:
		len = sizeof(*pwork);
		pqp->cl_info.cur_onof = !!cci_u8_data(info);
		_qpcp_fill_work_pkt(pqp, uo, pwork);
		break;
	case ACT_QPCP_CTRL_ADD_WATER:
		len = 0;
		//这里不知道加水后应该是什么模式，先不修改，看真实返回时什么模式
		//pqp->cl_info.cur_mode = QPCP_MODE_ADD_WATER;
		pqp->cl_info.cur_water_time = ptmp[1];
		pwater->action = ptmp[0];
		pwater->water_time = ptmp[1];
		_qpcp_fill_water_pkt(pqp, uo, pwater);
		sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo)+sizeof(*pwater));
		break;
	case ACT_QPCP_CTRL_HANDLE_CTRL:
		//要么手动烧水，要么手动消毒
		len = 0;
		memcpy(php, ptmp, sizeof(*php));
		//改变内存值
		if (php->action == QPCP_ACTION_BOIL) {
			pqp->cl_info.boil.temp = php->temp;
			pqp->cl_info.boil.power = php->power;
			pqp->cl_info.boil.time = php->time;
			pqp->cl_info.boil.thermal_temp = php->thermal_temp;
			pqp->cl_info.boil.thermal_time = php->thermal_time;
			//pqp->cl_info.cur_mode = QPCP_MODE_HEAT_WATER;
		} else {
			pqp->cl_info.disinfect.temp = php->temp;
			pqp->cl_info.disinfect.power = php->power;
			pqp->cl_info.disinfect.time = php->time;
			pqp->cl_info.disinfect.thermal_temp = php->thermal_temp;
			pqp->cl_info.disinfect.thermal_time = php->thermal_time;		
			//pqp->cl_info.cur_mode = QPCP_MODE_DISINFECT;
		}
		pqp->cl_info.cur_water_time = php->water_time;
		_qpcp_fill_hand_pkt(pqp, uo, php);
		sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo)+sizeof(*php));
		break;
	case ACT_QPCP_CTRL_SCENE_DEL:
		len = 0;
		psdel->id = htons(cci_u16_data(info));
		_qpcp_fill_sdel_pkt(pqp, uo, psdel);
		sa_ctrl_obj_value(user->uc_session, UCA_DELETE, false, 0x1, uo, sizeof(*uo)+sizeof(*psdel));
		//接着查询情景
		_qpcp_fill_squery_pkt(user->uc_session, pqp, uo);
		break;
	case ACT_QPCP_CTRL_SCENE_MODIFY:
		len = 0;
		memcpy(psp, ptmp, sizeof(*psp));
		//这里判断一下茶盘是否个数超出限制
		if (psp->scene_id == 0 &&
			_qpcp_get_scene_num(&pqp->scene_list) >= TEA_TRAY_QP_PLAN_MAX) {
			event_push(user->callback, SAE_SCENE_ID_MAX, user->handle, user->callback_handle);
        	event_cancel_merge(user->handle);
			return RS_OK;
		}
		psp->scene_id = htons(psp->scene_id);
		psp->create_time = htonl(psp->create_time);
		_qpcp_fill_sadd_pkt(pqp, uo, psp);
		sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo)+sizeof(*psp));
		//接着查询情景
		_qpcp_fill_squery_pkt(user->uc_session, pqp, uo);
		break;		
	case ACT_QPCP_CTRL_SCENE_EXECUTE:
		len = 0;
		p16_temp = (u_int16_t *)ptmp;
		pscp_tmp= (cl_qpcp_scp_t *)(&p16_temp[1]);
		pexe->id = htons(*p16_temp);
		memcpy(&pexe->scp, pscp_tmp, sizeof(*pscp_tmp));
		_qpcp_fill_sexe_pkt(pqp, uo, pexe);
		sa_ctrl_obj_value(user->uc_session, UCA_SET, false, 0x1, uo, sizeof(*uo)+sizeof(*pexe));
		break;
    case ACT_QPCP_CTRL_RESET_FAULT:
        len = 0;
        qr->is_rest = true;
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_TEA_TRAY_QP, UCAT_IA_TEA_TRAY_QP_RESET_FAULT,sizeof(*qr));
        sa_set_obj_value_only(user->uc_session, 0x1, buf, sizeof(*uo)+sizeof(*qr));
        break;
	default:
		*ret = RS_INVALID_PARAM;
		break;		
	}

	if(len > 0){
		sa_set_obj_value(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	}
		
	return res;
}

static void _qpcp_update_data_work(cl_qpcp_info_t *info, cl_cp_wg_t *pwg)
{
	cl_qpcp_config_t *pconfig = NULL;

	info->cur_onof = pwg->work;
	info->cur_mode = pwg->mode;
	info->cur_water_temp = pwg->water_temp;
	info->cur_water_time = pwg->water_time;
	info->cur_water_state = pwg->water_state;
	info->cur_error = pwg->error;
	info->cur_remain_water_time = pwg->remain_water_time;
	info->cur_production_status = pwg->product_status;
    info->boil_plan_id = ntohs(pwg->boil_plan_id);
    info->disinfect_plan_id = ntohs(pwg->disinfect_plan_id);
    info->boil_index = pwg->boil_index;
    info->disinfect_index = pwg->disinfect_index;
    info->mode = pwg->work_mode;
    info->water_warning = pwg->water_warning;

#if 0
	if (pwg->mode == QPCP_MODE_DISINFECT ||
		pwg->mode == QPCP_MODE_AUTO_DISINFECT) {
		pconfig = &info->disinfect;
		pconfig->power = pwg->disinfect_power;
		pconfig->temp = pwg->disinfect_temp;
		pconfig->thermal_temp = pwg->disinfect_thermal_temp;
		pconfig->thermal_time = pwg->disinfect_thermal_time;
		pconfig->time = pwg->disinfect_time;
	} else if (pwg->mode == QPCP_MODE_HEAT_WATER ||
		pwg->mode == QPCP_MODE_AUTO_HEAT_WATER ) {
		pconfig = &info->boil;
		pconfig->power = pwg->boil_power;
		pconfig->temp = pwg->boil_temp;
		pconfig->thermal_temp = pwg->boil_thermal_temp;
		pconfig->thermal_time = pwg->boil_thermal_time;
		pconfig->time = pwg->boil_time;
	}
#else
		pconfig = &info->disinfect;
		pconfig->power = pwg->disinfect_power;
		pconfig->temp = pwg->disinfect_temp;
		pconfig->thermal_temp = pwg->disinfect_thermal_temp;
		pconfig->thermal_time = pwg->disinfect_thermal_time;
		pconfig->time = pwg->disinfect_time;
		pconfig = &info->boil;
		pconfig->power = pwg->boil_power;
		pconfig->temp = pwg->boil_temp;
		pconfig->thermal_temp = pwg->boil_thermal_temp;
		pconfig->thermal_time = pwg->boil_thermal_time;
		pconfig->time = pwg->boil_time;
#endif
}

bool _qpcp_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	cl_qpcp_priv_t *priv = air_ctrl->com_udp_dev_info.device_info; 
	cl_qpcp_info_t *info = &priv->cl_info;
	cl_cp_wg_t *pwg;
	cp_id_hd_t *phd;
	cl_qpcp_sp_t *psp;

	if( !priv || !obj ){
		return false;
	}

	air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
	case UCAT_IA_TEA_TRAY_QP_WORK:
		if(!is_valid_obj_data(obj,sizeof(*pwg))){
			return false;
		}

		pwg = OBJ_VALUE(obj, cl_cp_wg_t*);
		_qpcp_update_data_work(info, pwg);
		break;
	case UCAT_IA_TEA_TRAY_QP_PLAN_ID:
		if(is_obj_less_than_len(obj,sizeof(*phd))){
			return false;
		}
		phd = OBJ_VALUE(obj, cp_id_hd_t*);
		_qpcp_update_data_id_modify(air_ctrl, priv, phd, obj->param_len - sizeof(*phd));
		//这里是为了接收到id时不modify.
		return false;
		//break;			
	case UCAT_IA_TEA_TRAY_QP_PLAN:
		if(is_obj_less_than_len(obj,sizeof(*psp))){
			return false;
		}

		psp = OBJ_VALUE(obj, cl_qpcp_sp_t*);
		_qpcp_update_data_scene_modify(priv, psp, obj->param_len);
		break;		
	default:
		return false;
		break;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//千帕锅
bool qp_pot_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
    cl_qpcp_priv_t *priv;
    cl_qp_pot_info_t* qp;
    int len,count;
    qp_pot_scene_node_t *node, *next;
    cl_qp_pot_scene_t* ts;
    
    if(!di || !ac || !ac->com_udp_dev_info.device_info){
        return false;
    }
    
    priv = ac->com_udp_dev_info.device_info;
    qp = cl_calloc(sizeof(*qp), 1);
    if (!qp) {
        return false;
    }
    
    memcpy(qp, &priv->qp_pot_info, sizeof(*qp));
    qp->scenes = NULL;
    qp->scene_count = 0;
    
    stlc_list_count(count, &priv->scene_list);
    
    if (count >0 ) {
        len = sizeof(cl_qp_pot_scene_t)*count;
        qp->scenes = cl_calloc( len , 1);
        if (qp->scenes != NULL) {
            qp->scene_count = count;
            ts = qp->scenes;
            
            stlc_list_for_each_entry_safe(qp_pot_scene_node_t, node, next, &priv->scene_list, link) {
                memcpy(ts, &node->scene, sizeof(*ts));
                ts++;
            }
        }
    }
    
    di->device_info = qp;
    
    return true;
}

bool qp_pot_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    bool res = true;
    cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    cl_qpcp_priv_t *pqp;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    int len = 0;
    u_int16_t s_id,*dest;
    cl_qp_pot_scene_param_t* sp;
    cl_qp_pot_scene_t* sc;
    ucp_qp_pot_stat_setting_t * uss;
    ucp_qp_pot_scene_t * usc;
    
    info = (cln_common_info_t *)&pkt->data[0];
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac ||!ac->com_udp_dev_info.device_info) {
        log_err(false, "qp_pot_proc_notify error handle %08x\n",info->handle);
        *ret = RS_INVALID_PARAM;
        return false;
    }

   
    pqp = ac->com_udp_dev_info.device_info;
    
    switch(info->action){
        case ACT_QP_POT_CTRL:
        case ACT_QP_POT_EXEC_SCENE:
            uss = (ucp_qp_pot_stat_setting_t*)(uo+1);
            usc = (ucp_qp_pot_scene_t*)(uo+1);
            sp = cci_pointer_data(info);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_POT, UCAT_IA_QP_POT_SET_PROC,sizeof(*uss));
            len = sizeof(*uss);
            
            uss->action = sp->set_action;
            uss->cook_id = htons(sp->s_id);
            uss->cook_time = htons(sp->cook_time);
            uss->cooking_mode = sp->cooking_mode;
            uss->hot_degress = sp->hot_degress;
            uss->microswitch = sp->microswitch;
            uss->warm_temp = sp->warm_temp;
            if (info->action == ACT_QP_POT_EXEC_SCENE) {
                usc->rice_degress = sp->rice_degress;
            }
            
            break;
        case ACT_QP_POT_DEL_SCENE:
            s_id = cci_u16_data(info);
            len = sizeof(u_int32_t);
            dest = (u_int16_t*)(uo+1);
            *dest = htons(s_id);
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_POT, UCAT_IA_QP_POT_SCENE_CTRL,len);
            return sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,false,0x1, buf, sizeof(*uo)+len);
            break;
        case ACT_QP_POT_MODIFY_SCENE:
            usc = (ucp_qp_pot_scene_t*)(uo+1);
            sc = cci_pointer_data(info);
            len = sizeof(*usc);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_POT, UCAT_IA_QP_POT_SCENE_CTRL,len);
            
            usc->cook_time = htons(sc->s_info.cook_time);
            usc->cook_id = htons(sc->s_info.s_id);
            usc->cooking_mode = sc->s_info.cooking_mode;
            usc->hot_degress = sc->s_info.hot_degress;
            usc->microswitch = sc->s_info.microswitch;
            usc->warm_temp = sc->s_info.warm_temp;
            usc->rice_degress = sc->s_info.rice_degress;
            memcpy(usc->name, sc->name, sizeof(usc->name)-1);
            if (!sc->create_time) {
                sc->create_time = (u_int32_t)time(NULL);
            }
            usc->create_time = htonl(sc->create_time);
            
            break;
         default:
            *ret = RS_INVALID_PARAM;
            break;		
    }
    
    if(len > 0){
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,false,0x1, buf, sizeof(*uo)+len);
    }
    
    return res;
}

enum{
    ACT_QP_SCENE_ADD = 0,
    ACT_QP_SCENE_MODIFY,
    ACT_QP_SCENE_DELETE
};

static qp_pot_scene_node_t* _qp_pot_find_scene_by_id(cl_qpcp_priv_t *priv,u_int16_t s_id)
{
    qp_pot_scene_node_t *node, *next;
    
    stlc_list_for_each_entry_safe(qp_pot_scene_node_t, node, next, &priv->scene_list, link) {
        if (node->scene.s_info.s_id == s_id) {
            return node;
        }
    }
    return NULL;
}

static void _qp_pot_delete_scene_by_scene(qp_pot_scene_node_t* node)
{
    if (node) {
        stlc_list_del(&node->link);
        cl_free(node);
    }
}

static void _qp_pot_do_scene(cl_qpcp_priv_t *priv,bool is_list,ucp_qp_pot_scene_t* scene)
{
    u_int16_t s_id  = ntohs(scene->cook_id);
    qp_pot_scene_node_t* node,*new_node;
    u_int8_t action = scene->action;
    
    if (is_list) {
        action = ACT_QP_SCENE_ADD;
    }
    
    
    node = _qp_pot_find_scene_by_id(priv , s_id);
    if (action == ACT_QP_SCENE_DELETE) {
        _qp_pot_delete_scene_by_scene(node);
        return;
    }
    
    if (node) {
        new_node = node;
    }else{
        new_node = cl_calloc(sizeof(*node), 1);
        if (!new_node) {
            return;
        }
    }
    
    new_node->scene.create_time = ntohl(scene->create_time);
    memcpy(new_node->scene.name, scene->name, sizeof(node->scene.name)-1);
    new_node->scene.s_info.cook_time = ntohs(scene->cook_time);
    new_node->scene.s_info.cooking_mode = scene->cooking_mode;
    new_node->scene.s_info.hot_degress = scene->hot_degress;
    new_node->scene.s_info.microswitch = scene->microswitch;
    new_node->scene.s_info.s_id = ntohs(scene->cook_id);
    new_node->scene.s_info.warm_temp = scene->warm_temp;
    new_node->scene.s_info.rice_degress = scene->rice_degress;
    
    //
    if (!node) {
        stlc_list_add_tail(&new_node->link, &priv->scene_list);
    }
}

void _free_qp_pot_sdk_info(cl_qpcp_priv_t* priv)
{
    qp_pot_scene_node_t *node, *next;
    
    if(!priv){
        return;
    }
    
    stlc_list_for_each_entry_safe(qp_pot_scene_node_t, node, next, &priv->scene_list, link) {
        stlc_list_del(&node->link);
        cl_free(node);
    }
}

static bool _qp_pot_find_id_in_list(ucp_qp_pot_scene_list_t* list,u_int16_t s_id)
{
    u_int16_t index;
    s_id = htons(s_id);
    
    for (index = 0; index < list->count; index++) {
        if (list->ids[index] == s_id) {
            return true;
        }
    }
    
    return false;
}

void _qp_pot_query_all_scene_by_pkt(smart_air_ctrl_t* ac,cl_qpcp_priv_t *priv,ucp_qp_pot_scene_list_t* list)
{
    qp_pot_scene_node_t *node, *next;
    u_int16_t max_cnt,index,count,len;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int16_t* p_data;
    
    if (!list) {
        return;
    }
    
    list->count = ntohs(list->count);
    if (list->count == 0) {
        _free_qp_pot_sdk_info(priv);
        return;
    }
    
    // drop deleted item
    stlc_list_for_each_entry_safe(qp_pot_scene_node_t, node, next, &priv->scene_list, link) {
        if (! _qp_pot_find_id_in_list(list,node->scene.s_info.s_id)) {
            stlc_list_del(&node->link);
            cl_free(node);
        }
    }
    
    // query
    max_cnt = 1200 / sizeof(ucp_qp_pot_scene_t);
    count = index = 0;
    p_data = (u_int16_t*)(uo+1);
    
    for (; index < list->count; index++,count++) {
        p_data[count+1] = list->ids[index];
        if (count >= max_cnt) {
            len = (count+1)*sizeof(u_int16_t);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_POT, UCAT_IA_QP_POT_SCENE_CTRL,len);
            p_data[0] = htons(count);
            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+len);
            count = 0;
        }
    }
    
    if(count >0 ){
        len = (count+1)*sizeof(u_int16_t);
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_POT, UCAT_IA_QP_POT_SCENE_CTRL,len);
        p_data[0] = htons(count);
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+len);
    }
    
}

bool qp_pot_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    cl_qpcp_priv_t *priv = air_ctrl->com_udp_dev_info.device_info;
    cl_qp_pot_info_t* pot_info = &priv->qp_pot_info;
    ucp_qp_pot_stat_t* qs;
    ucp_qp_pot_scene_t* qsc;
    ucp_qp_pot_scene_list_t* ql;
    int i,count;
    
    if( !priv || !obj ){
        return false;
    }
    
    air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;
    
    
    switch(obj->attr){
        case UCAT_IA_QP_POT_SCENE_ID_LIST:
            ql = OBJ_VALUE(obj, ucp_qp_pot_scene_list_t*);
            _qp_pot_query_all_scene_by_pkt(air_ctrl,priv,ql);
            break;
        case UCAT_IA_QP_POT_SCENE_MODIDY_PUSH:
            qsc = OBJ_VALUE(obj, ucp_qp_pot_scene_t*);
            count = obj->param_len/sizeof(*qsc);
            for (i = 0; i< count; i++,qsc++) {
                _qp_pot_do_scene(priv,false,qsc);
            }
            break;
        case UCAT_IA_QP_POT_CURR_STAT:
            qs  = OBJ_VALUE(obj, ucp_qp_pot_stat_t*);
            
            pot_info->cooking_mode = qs->cooking_mode;
            pot_info->cooking_remain_time = ntohs(qs->cook_time);
            pot_info->cur_id = ntohs(qs->cook_id);
            pot_info->err_num = qs->err_code;
            pot_info->food_quantity = qs->food_degress;
            pot_info->is_complete = !!(qs->pot_flag & BIT(0));
            pot_info->is_pot_cover_open = !!(qs->pot_flag & BIT(1));
            pot_info->microswitch = !!(qs->pot_flag & BIT(2));
            pot_info->pot_temp = qs->pot_temp;
            pot_info->stat = qs->work_stat;
            pot_info->warm_temp = qs->warm_temp;
            
            
            break;
        case UCAT_IA_QP_POT_SCENE_CTRL:
            qsc = OBJ_VALUE(obj, ucp_qp_pot_scene_t*);
            count = obj->param_len/sizeof(*qsc);
            for (i = 0; i< count; i++,qsc++) {
                _qp_pot_do_scene(priv,true,qsc);
            }
            
            break;
        default:
            return false;
            break;
    }
    
    return true;
}
//////////////////////////////////////////
//千帕破壁机

bool qp_pbj_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
    cl_qpcp_priv_t *priv;
    cl_qp_pbj_info_t* qp;
    int len,count;
    qp_pbj_scene_node_t *node, *next;
    cl_qp_pbj_scene_t* ts;
    
    if(!di || !ac || !ac->com_udp_dev_info.device_info){
        return false;
    }
    
    priv = ac->com_udp_dev_info.device_info;
    qp = cl_calloc(sizeof(*qp), 1);
    if (!qp) {
        return false;
    }
    
    memcpy(qp, &priv->qp_pbj_info, sizeof(*qp));
    qp->scene = NULL;
    qp->scene_num = 0;
    
    stlc_list_count(count, &priv->scene_list);
    
    if (count >0 ) {
        len = sizeof(cl_qp_pbj_scene_t)*count;
        qp->scene = cl_calloc( len , 1);
        if (qp->scene != NULL) {
            qp->scene_num = count;
            ts = qp->scene;
            
            stlc_list_for_each_entry_safe(qp_pbj_scene_node_t, node, next, &priv->scene_list, link) {
                memcpy(ts, &node->scene, sizeof(*ts));
                ts++;
            }
        }
    }
    
    di->device_info = qp;
    
    return true;
}

bool qp_pbj_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    cl_qpcp_priv_t *pqp;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    int len = 0,i;
    u_int16_t s_id,act;
    cl_qp_pbj_scene_t* cs;
    ucp_qp_pbj_scene_t* us;
    u_int32_t t_value;
    
    
    
    info = (cln_common_info_t *)&pkt->data[0];
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac ||!ac->com_udp_dev_info.device_info) {
        log_err(false, "qp_pot_proc_notify error handle %08x\n",info->handle);
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    
    pqp = ac->com_udp_dev_info.device_info;
    switch (info->action) {
        case ACT_QP_PBJ_CTRL_ONOFF:
            pqp->qp_pbj_info.stat.on_off = !!cci_u8_data(info);
            len = sizeof(u_int32_t);
            
            *((u_int8_t*)(uo+1)) = pqp->qp_pbj_info.stat.on_off;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_WORK_STAT,len);
            break;
        case ACT_QP_PBJ_CTRL_EXEC_SCENE:
            
            t_value = cci_u32_data(info);
            s_id = t_value & 0xFFFF;
            s_id = htons(s_id);
            len = sizeof(u_int32_t);
            act = (t_value >> 16) & 0xFF;
            
            memcpy(uo+1, &s_id, sizeof(s_id));
            if (act == 0x1) {//
                fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_EDIT_SCENE,len);
                sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,false,0x1, buf, sizeof(*uo)+len);
                return true;
            }else{ //执行
                if (act == 0x2) {
                    memset(uo+1, 0, sizeof(u_int32_t));
                }
                fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_EXEC_SCENE,len);
            }
            break;
        case ACT_QP_PBJ_CTRL_MODIFY_SCENE:
            us = (ucp_qp_pbj_scene_t*)(uo+1);
            cs = cci_pointer_data(info);
            
            us->scene_id = htons(cs->scene_id);
            memcpy(us->name, cs->name, sizeof(cs->name)-1);
            us->create_time = 0;
            us->warm_temp = cs->warm_temp;
            us->warm_time = cs->warm_time;
            us->step_count = cs->step_count;
            us->len = sizeof(ucp_qp_pbj_scene_action_t)*cs->step_count;
            
            for (i = 0; i < cs->step_count; i++) {
                us->action[i].action = cs->action[i].data_type;
                if (cs->action[i].data_type == QP_MODE_HOT) {
                    us->action[i].p1 = cs->action[i].h_info.temp;
                    us->action[i].p2 = cs->action[i].h_info.time;
                }else{
                    us->action[i].p1 = cs->action[i].m_info.gear;
                    us->action[i].p2 = cs->action[i].m_info.time;
                    us->action[i].p3 = cs->action[i].m_info.freq;
                }
            }
            
            len = sizeof(*us)+ sizeof(ucp_qp_pbj_scene_action_t)*us->step_count;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_EDIT_SCENE,len);
            break;
        case ACT_QP_PBJ_CTRL_FAULT_STAT:
            len = sizeof(u_int32_t);
            
            *((u_int8_t*)(uo+1)) = 0x1;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_CLEAR_FAULT,len);
            break;
            
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    
    if(len > 0){
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,false,0x1, buf, sizeof(*uo)+len);
    }
    
    return true;
}

static bool _qp_pbj_find_id_in_list(ucp_qp_pbj_scene_list_t* list,u_int16_t s_id)
{
    u_int16_t index;
    s_id = htons(s_id);
    
    for (index = 0; index < list->count; index++) {
        if (list->ids[index] == s_id) {
            return true;
        }
    }
    
    return false;
}

static void _qp_pbj_query_all_scene_by_pkt(smart_air_ctrl_t* ac,cl_qpcp_priv_t *priv,ucp_qp_pbj_scene_list_t* list)
{
    qp_pbj_scene_node_t *node, *next;
    u_int16_t max_cnt,index,count,len;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int16_t* p_data;
    
    if (!list) {
        return;
    }
    
    list->count = ntohs(list->count);
    if (list->count == 0) {
        _free_qp_pot_sdk_info(priv);
        return;
    }
    
    // drop deleted item
    stlc_list_for_each_entry_safe(qp_pbj_scene_node_t, node, next, &priv->scene_list, link) {
        if (! _qp_pbj_find_id_in_list(list,node->scene.scene_id)) {
            stlc_list_del(&node->link);
            cl_free(node);
        }
    }
    
    // query
    max_cnt = 1200 / (sizeof(ucp_qp_pbj_scene_t)+sizeof(ucp_qp_pbj_scene_action_t)*9);
    count = index = 0;
    p_data = (u_int16_t*)(uo+1);
    
    for (; index < list->count; index++,count++) {
        p_data[count+1] = list->ids[index];
        if (count >= max_cnt) {
            len = (count+1)*sizeof(u_int16_t);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_EDIT_SCENE,len);
            p_data[0] = htons(count);
            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+len);
            count = 0;
        }
    }
    
    if(count >0 ){
        len = (count+1)*sizeof(u_int16_t);
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_QP_PBJ, UCAT_IA_QP_PBJ_EDIT_SCENE,len);
        p_data[0] = htons(count);
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+len);
    }
    
}

static qp_pbj_scene_node_t* _qp_pbj_find_scene_by_id(cl_qpcp_priv_t *priv,u_int16_t s_id)
{
    qp_pbj_scene_node_t *node, *next;
    
    stlc_list_for_each_entry_safe(qp_pbj_scene_node_t, node, next, &priv->scene_list, link) {
        if (node->scene.scene_id == s_id) {
            return node;
        }
    }
    return NULL;
}

static void _qp_pbj_delete_scene_by_scene(qp_pbj_scene_node_t* node)
{
    if (node) {
        stlc_list_del(&node->link);
        cl_free(node);
    }
}


static void _qp_pbj_do_scene(cl_qpcp_priv_t *priv,bool is_list,ucp_qp_pbj_scene_t* scene)
{
    u_int16_t s_id  = ntohs(scene->scene_id);
    qp_pbj_scene_node_t* node,*new_node;
    u_int8_t action = scene->act;
    int i;
    
    if (is_list) {
        action = ACT_QP_SCENE_ADD;
    }else{
        if (action == 0x1) {
            action = ACT_QP_SCENE_ADD;
        }else if(action == 0x2){
            action = ACT_QP_SCENE_DELETE;
        }else{
            action = ACT_QP_SCENE_MODIFY;
        }
    }
    
    
    node = _qp_pbj_find_scene_by_id(priv , s_id);
    if (action == ACT_QP_SCENE_DELETE) {
        _qp_pbj_delete_scene_by_scene(node);
        return;
    }
    
    if (node) {
        new_node = node;
    }else{
        new_node = cl_calloc(sizeof(*node), 1);
        if (!new_node) {
            return;
        }
    }
    
    //
    new_node->scene.scene_id = ntohs(scene->scene_id);
    new_node->scene.step_count = scene->step_count;
    new_node->scene.warm_temp = scene->warm_temp;
    new_node->scene.warm_time = scene->warm_time;
    memcpy(new_node->scene.name, scene->name, sizeof(new_node->scene.name)-1);
    for (i = 0; i< scene->step_count; i++) {
        new_node->scene.action[i].data_type = scene->action[i].action;
        if(scene->action[i].action == QP_MODE_HOT){
            new_node->scene.action[i].h_info.temp = scene->action[i].p1;
            new_node->scene.action[i].h_info.time = scene->action[i].p2;
        }else{
            new_node->scene.action[i].m_info.gear = scene->action[i].p1;
            new_node->scene.action[i].m_info.time = scene->action[i].p2;
            new_node->scene.action[i].m_info.freq = scene->action[i].p3;
        }
    }
    
    if (!node) {
        stlc_list_add_tail(&new_node->link, &priv->scene_list);
    }
}

bool qp_pbj_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    cl_qpcp_priv_t *priv = air_ctrl->com_udp_dev_info.device_info;
    cl_qp_pbj_info_t* pbj = &priv->qp_pbj_info;
    ucp_qp_pbj_scene_t* qs;
    ucp_qp_pbj_stat_t*  qt;
    int pos;
    u_int8_t* p;
    
    if( !priv || !obj ){
        return false;
    }
    
    air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;
    switch(obj->attr){
        case UCAT_IA_QP_PBJ_WORK_STAT:
            qt = OBJ_VALUE(obj, ucp_qp_pbj_stat_t*);
            pbj->stat.on_off = qt->on_off;
            pbj->stat.cur_exec_id = ntohs(qt->cur_exe_id);
            pbj->stat.cur_mode = qt->mode;
            pbj->stat.err_no = qt->err_num;
            pbj->stat.is_data_valid = true;
            pbj->stat.mix_power = qt->mix_power;
            pbj->stat.temp = qt->temp;
            pbj->stat.work_stat = qt->stat;
            break;
        case UCAT_IA_QP_PBJ_EDIT_SCENE:
            p = OBJ_VALUE(obj, u_int8_t*);
            pos = 0;
            qs = (ucp_qp_pbj_scene_t*)(p+pos);
            
            if (action == UCA_PUSH) {
                _qp_pbj_do_scene(priv,false,qs);
            }else{
                for (pos = 0; pos < obj->param_len; ) {
                    qs = (ucp_qp_pbj_scene_t*)(p+pos);
                    _qp_pbj_do_scene(priv,true,qs);
                    pos = pos + sizeof(*qs) + qs->step_count * sizeof(ucp_qp_pbj_scene_action_t);
                }
            }
            break;
        case UCAT_IA_QP_PBJ_SCENE_LIST:
            _qp_pbj_query_all_scene_by_pkt(air_ctrl,priv,(ucp_qp_pbj_scene_list_t*)(obj+1));
            break;
        default:
            return false;
            break;
    }
    
    return true;

}

void _free_qp_pbj_sdk_info(cl_qpcp_priv_t* priv)
{
    qp_pbj_scene_node_t *node, *next;
    
    if(!priv){
        return;
    }
    
    stlc_list_for_each_entry_safe(qp_pbj_scene_node_t, node, next, &priv->scene_list, link) {
        stlc_list_del(&node->link);
        cl_free(node);
    }
}

void _free_qp_pbj_info(cl_qp_pbj_info_t* info)
{
    if (info && info->scene) {
        cl_free(info->scene);
        info->scene = NULL;
    }
}

////////////////////////////////////////////////////////////////////////
// 海迅养生壶

bool hx_ysh_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di)
{
    cl_qpcp_priv_t *priv;
    cl_hx_ysh_info_t* hy;
    int len,count;
    hx_ysh_scene_node_t *node, *next;
    cl_hx_ysh_scene_t* ts;
    
    if(!di || !ac || !ac->com_udp_dev_info.device_info){
        return false;
    }
    
    priv = ac->com_udp_dev_info.device_info;
    hy = cl_calloc(sizeof(*hy), 1);
    if (!hy) {
        return false;
    }
    
    memcpy(hy, &priv->hx_ysh_info, sizeof(*hy));
    hy->scene = NULL;
    hy->scene_num = 0;
    
    stlc_list_count(count, &priv->scene_list);
    
    if (count >0 ) {
        len = sizeof(cl_hx_ysh_scene_t)*count;
        hy->scene = cl_calloc( len , 1);
        if (hy->scene != NULL) {
            hy->scene_num = count;
            ts = hy->scene;
            
            stlc_list_for_each_entry_safe(hx_ysh_scene_node_t, node, next, &priv->scene_list, link) {
                memcpy(ts, &node->scene, sizeof(*ts));
                ts++;
            }
        }
    }
    
    di->device_info = hy;
    
    return true;
}

bool hx_ysh_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
    cln_common_info_t *info;
    smart_air_ctrl_t* ac;
    cl_qpcp_priv_t *pqp;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    int len = 0;
    u_int16_t s_id;
    cl_hx_ysh_scene_t* cs;
    ucp_hx_ysh_scene_t* us;
    u_int32_t t_value;
    u_int8_t action;
    bool need_query = false;
    
    info = (cln_common_info_t *)&pkt->data[0];
    ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
    if (!ac ||!ac->com_udp_dev_info.device_info) {
        log_err(false, "qp_pot_proc_notify error handle %08x\n",info->handle);
        *ret = RS_INVALID_PARAM;
        return false;
    }
    
    
    pqp = ac->com_udp_dev_info.device_info;
    switch (info->action) {
        case ACT_HX_YSH_CTRL_ONOFF:
            pqp->qp_pbj_info.stat.on_off = !!cci_u8_data(info);
            len = sizeof(u_int32_t);
            
            *((u_int8_t*)(uo+1)) = pqp->qp_pbj_info.stat.on_off;
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_WORK_STAT,len);
            need_query = true;
            break;
        case ACT_HX_YSH_CTRL_EXEC_SCENE:
            cs = cci_pointer_data(info);
            len = sizeof(ucp_hx_ysh_scene_t);
            us = (ucp_hx_ysh_scene_t*)(uo+1);
            us->keep_temp = cs->keep_temp;
            us->keep_time = cs->keep_time;
            us->power = cs->power;
            us->scene_id = htons(cs->scene_id);
            us->temp = cs->temp;
            us->work_time = cs->time;
            
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_EXEC_SCENE,len);
            break;
        case ACT_HX_YSH_CTRL_DEL_OR_STOP_SCENE:
        
            t_value = cci_u32_data(info);
            s_id = t_value & 0xFFFF;
            s_id = htons(s_id);
            len = sizeof(u_int32_t);
            action = (t_value >> 16) &0xFF;
            
            memcpy(uo+1, &s_id, sizeof(s_id));
            if (action == 0x1) {//
                fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_EDIT_SCENE,len);
                sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_DELETE,false,0x1, buf, sizeof(*uo)+len);
                return true;
            }else{ //执行
                if (action == 0x2) {
                    memset(uo+1, 0, sizeof(s_id));
                }
                fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_EXEC_SCENE,len);
            }
            break;
        case ACT_HX_YSH_CTRL_MODIFY_SCENE:
            us = (ucp_hx_ysh_scene_t*)(uo+1);
            cs = cci_pointer_data(info);
            
            us->scene_id = htons(cs->scene_id);
            memcpy(us->name, cs->name, sizeof(cs->name)-1);
            if (cs->create_time == 0) {
                cs->create_time = (u_int32_t)time(NULL);
            }
            us->create_time = htonl(cs->create_time);
			us->temp = cs->temp;
            us->keep_temp = cs->keep_temp;
            us->keep_time = cs->keep_time;
            us->power = cs->power;
            us->work_time = cs->time;
            
            len = sizeof(*us);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_EDIT_SCENE,len);
            break;
        default:
            *ret = RS_INVALID_PARAM;
            return false;
            break;
    }
    
    if(len > 0){
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_SET,need_query,0x1, buf, sizeof(*uo)+len);
    }
    
    return true;
}

static bool _hx_ysh_find_id_in_list(ucp_qp_pbj_scene_list_t* list,u_int16_t s_id)
{
    u_int16_t index;
    s_id = htons(s_id);
    
    for (index = 0; index < list->count; index++) {
        if (list->ids[index] == s_id) {
            return true;
        }
    }
    
    return false;
}

static void _hx_ysh_query_all_scene_by_pkt(smart_air_ctrl_t* ac,cl_qpcp_priv_t *priv,ucp_qp_pbj_scene_list_t* list)
{
    hx_ysh_scene_node_t *node, *next;
    u_int16_t max_cnt,index,count,len;
    char buf[256] = {0};
    ucp_obj_t* uo = (ucp_obj_t*)buf;
    u_int16_t* p_data;
    
    if (!list) {
        return;
    }
    
    list->count = ntohs(list->count);
    if (list->count == 0) {
        _free_qp_pot_sdk_info(priv);
        return;
    }
    
    // drop deleted item
    stlc_list_for_each_entry_safe(hx_ysh_scene_node_t, node, next, &priv->scene_list, link) {
        if (! _hx_ysh_find_id_in_list(list,node->scene.scene_id)) {
            stlc_list_del(&node->link);
            cl_free(node);
        }
    }
    
    // query
    max_cnt = 1200 / sizeof(ucp_hx_ysh_scene_t);
    count = index = 0;
    p_data = (u_int16_t*)(uo+1);
    
    for (; index < list->count; index++,count++) {
        p_data[count+1] = list->ids[index];
        if (count >= max_cnt) {
            len = (count+1)*sizeof(u_int16_t);
            fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_EDIT_SCENE,len);
            p_data[0] = htons(count);
            sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+len);
            count = 0;
        }
    }
    
    if(count >0 ){
        len = (count+1)*sizeof(u_int16_t);
        fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_HX_YS_POT, UCAT_IA_HX_YS_POT_EDIT_SCENE,len);
        p_data[0] = htons(count);
        sa_ctrl_obj_value(ac->sac->user->uc_session, UCA_GET,false,0x1, buf, sizeof(*uo)+len);
    }
    
}

static hx_ysh_scene_node_t* _hx_ysh_find_scene_by_id(cl_qpcp_priv_t *priv,u_int16_t s_id)
{
    hx_ysh_scene_node_t *node, *next;
    
    stlc_list_for_each_entry_safe(hx_ysh_scene_node_t, node, next, &priv->scene_list, link) {
        if (node->scene.scene_id == s_id) {
            return node;
        }
    }
    return NULL;
}

static void _hx_ysh_delete_scene_by_scene(hx_ysh_scene_node_t* node)
{
    if (node) {
        stlc_list_del(&node->link);
        cl_free(node);
    }
}

static void _hx_ysh_do_scene(cl_qpcp_priv_t *priv,bool is_list,ucp_hx_ysh_scene_t* scene)
{
    u_int16_t s_id  = ntohs(scene->scene_id);
    hx_ysh_scene_node_t* node,*new_node;
    u_int8_t action = scene->act;
    
    if (is_list) {
        action = ACT_QP_SCENE_ADD;
    }else{
        if (action == 0x1) {
            action = ACT_QP_SCENE_ADD;
        }else if(action == 0x2){
            action = ACT_QP_SCENE_DELETE;
        }else{
            action = ACT_QP_SCENE_MODIFY;
        }
    }
    
    
    node = _hx_ysh_find_scene_by_id(priv , s_id);
    if (action == ACT_QP_SCENE_DELETE) {
        _hx_ysh_delete_scene_by_scene(node);
        return;
    }
    
    if (node) {
        new_node = node;
    }else{
        new_node = cl_calloc(sizeof(*node), 1);
        if (!new_node) {
            return;
        }
    }
    
    //
    new_node->scene.scene_id = ntohs(scene->scene_id);
    new_node->scene.keep_temp = scene->keep_temp;
    new_node->scene.keep_time = scene->keep_time;
    memcpy(new_node->scene.name, scene->name, sizeof(new_node->scene.name)-1);
    new_node->scene.power = scene->power;
    new_node->scene.temp = scene->temp;
    new_node->scene.time = scene->work_time;
    new_node->scene.create_time = ntohl(new_node->scene.create_time);

    if (!node) {
        stlc_list_add_tail(&new_node->link, &priv->scene_list);
    }
}


bool hx_ysh_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
    cl_qpcp_priv_t *priv = air_ctrl->com_udp_dev_info.device_info;
    cl_hx_ysh_info_t* ysh = &priv->hx_ysh_info;
    ucp_hx_ysh_scene_t* qs;
    ucp_hx_ysh_stat_t*  qt;
    u_int16_t pos;
    
    if( !priv || !obj ){
        return false;
    }
    
    air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;
    
    switch(obj->attr){
        case UCAT_IA_HX_YS_POT_WORK_STAT:
            qt = OBJ_VALUE(obj, ucp_hx_ysh_stat_t*);
            
            ysh->stat.is_data_valid = true;
            ysh->stat.cur_exec_id = qt->cur_exe_id;
            ysh->stat.cur_power = qt->power;
            ysh->stat.err_no = qt->err_num;
            ysh->stat.mcu_timer = qt->mcu_timer;
            ysh->stat.on_off = !!(qt->flag & BIT(0));
            ysh->stat.is_hot = !!(qt->flag & BIT(1));
            ysh->stat.temp = qt->temp;
            ysh->stat.work_stat = qt->work_stat;
            ysh->stat.work_remain_time = qt->work_remain_time;
            ysh->stat.wifi_timer_exec_id = qt->wifi_timer_exec_id;
			ysh->stat.wifi_timer = ntohs(qt->wifi_timer);
            break;
        case UCAT_IA_HX_YS_POT_EDIT_SCENE:
            if (!is_obj_less_than_len(obj, sizeof(ucp_hx_ysh_scene_t))) {
                qs = (ucp_hx_ysh_scene_t*)(obj+1);
                
                if (action == UCA_PUSH) {
                    _hx_ysh_do_scene(priv,false,qs);
                }else{
                    for (pos = 0; pos < obj->param_len/sizeof(ucp_hx_ysh_scene_t); pos++,qs++) {
                        _hx_ysh_do_scene(priv,true,qs);
                    }
                }
            }
           
            break;
        case UCAT_IA_HX_YS_POT_SCENE_LIST:
            if (!is_obj_less_than_len(obj, sizeof(ucp_qp_pbj_scene_list_t))) {
                _hx_ysh_query_all_scene_by_pkt(air_ctrl,priv,(ucp_qp_pbj_scene_list_t*)(obj+1));
            }
            break;
        default:
            return false;
            break;
    }
    
    return true;
}

void _free_hx_ysh_sdk_info(cl_qpcp_priv_t* priv)
{
    hx_ysh_scene_node_t *node, *next;
    
    if(!priv){
        return;
    }
    
    stlc_list_for_each_entry_safe(hx_ysh_scene_node_t, node, next, &priv->scene_list, link) {
        stlc_list_del(&node->link);
        cl_free(node);
    }
}

void _free_hx_ysh_info(cl_hx_ysh_info_t* info)
{
    if (info && info->scene) {
        cl_free(info->scene);
        info->scene = NULL;
    }
}
