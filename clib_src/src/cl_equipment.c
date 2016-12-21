/*
	本文件实现电器、报警等外设，与用户程序的接口
*/
#include "cl_priv.h"
#include "cl_equipment.h"
#include "equipment_priv.h"

//obj中priv由用户释放
void cl_eq_free(cl_equipment_t* eq);
void cl_key_info_free(cl_key_t* ki);

static bool is_phone_number(char* phone)
{
    int i, len;

	if (phone == NULL)
		return false;
	
	len = (int)strlen(phone);
	
    if (len > 15 || len < 4) {
        return false;
    }

   for (i = 0; i< len; i++) {
        if (!(phone[i]>='0' && phone[i]<='9')) {
            return false;
        }
    }
    
    return true;
}

CLIB_API RS cl_eq_add(cl_handle_t slave_handle,cl_handle_t* eq_handle,cl_equipment_add_info_t* info)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!info||!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
    info->name[sizeof(info->name)-1]=0x0;
    if (strlen(info->name)<=0) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ADD, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
    e->slave_handle = slave_handle;
    e->s_001e_handle = info->eq_001e_handle;
    e->area_handle = info->area_handle;
    e->add_handle = eq_handle;
    e->eq_type = info->dev_type;
    e->group_num = info->group_num;
    e->is_more_ctrl = info->is_more_ctrl;
    memcpy(e->name, info->name, sizeof(e->name));
    e->name[sizeof(e->name)-1]=0x0;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_add_by_json(cl_handle_t slave_handle,cl_handle_t* eq_handle, const char *json)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (eq_handle == NULL || json == NULL) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ADD_BY_JSON, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
    e->slave_handle = slave_handle;
    e->add_handle = eq_handle;
	e->json = (char *)json;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);

	return RS_OK;
}

CLIB_API RS cl_eq_del(cl_handle_t eq_handle)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle) {
        return RS_INVALID_PARAM;
    }

	pkt = cl_notify_pkt_new(1024, CLNE_EQ_DEL, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
    e->eq_handle = eq_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_modify_name(cl_handle_t eq_handle, const char* new_name)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!new_name||!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_CH_NAME, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    strncpy(e->name, new_name, sizeof(e->name)-1);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_modify_area(cl_handle_t eq_handle, cl_handle_t area_handle)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!area_handle || !eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_CH_AREA, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    e->area_handle = area_handle;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_set_alarm_assist_001e(cl_handle_t eq_handle,cl_handle_t slave_handle)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ALARM_SET_BIND_001E, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    e->slave_handle = slave_handle;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_set_alarm_phones(cl_handle_t eq_handle,char** phonelist,u_int8_t phone_count)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    int i;
    
	CL_CHECK_INIT;
    
    if (!eq_handle ||(phone_count>0 && phonelist == NULL)) {
        return RS_INVALID_PARAM;
    }
    
    if (phone_count>0) {
        for (i=0; i<phone_count; i++) {
            if (!is_phone_number(phonelist[i])) {
                return RS_INVALID_PARAM;
            }
        }
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ALARM_PHONE, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    e->phonelist = phonelist;
    e->numofphone = phone_count;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

#if 0
RS cl_user_set_alarm_phones(cl_handle_t user_handle,char** phonelist,u_int8_t phone_count)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle ||(phone_count>0 && phonelist == NULL)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_USER_ALARM_PHONE_LIST, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->user_handle = user_handle;
    e->phonelist = phonelist;
    e->numofphone = phone_count;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}
#endif

CLIB_API RS cl_user_add_alarm_phone(cl_handle_t user_handle, const char* phone)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle ||!is_phone_number((char *)phone)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_USER_ADD_ALARM_PHONE, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->user_handle = user_handle;
    strcpy(e->name, phone);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}



CLIB_API RS cl_user_del_alarm_phone(cl_handle_t user_handle, const char* phone)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle ||!is_phone_number((char *)phone)) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_USER_DEL_ALARM_PHONE, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->user_handle = user_handle;
    strcpy(e->name, phone);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_bind_soundlight(cl_handle_t eq_handle, bool on, cl_handle_t *soundline, u_int8_t soundline_count)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (soundline_count > 0 && soundline == NULL) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_BIND_SOUNDLIGHT, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    e->soundline = soundline;
    e->soundline_num = soundline_count;
	e->soundline_on = on;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_set_db_dimming_value(cl_handle_t eq_handle,u_int8_t value)
{
    cl_notify_pkt_t *pkt;
	cln_key_info_t *k;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
    if (value > MAX_DIMMING_VALUE) {
        value = MAX_DIMMING_VALUE;
    }
    
//    if ((value%STEP_FOR_DIMMING)!=0) {
//        value = ((value+STEP_FOR_DIMMING-1)/STEP_FOR_DIMMING)*STEP_FOR_DIMMING;
//    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_SET_DIMMING_LAMP, CLNPF_ACK);
	k = (cln_key_info_t *)&pkt->data[0];
    k->eq_handle = eq_handle;
    k->db_dimm_value = value;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}
/*
	scene_handle为0表示删除联动
*/
CLIB_API RS cl_eq_linkage_scene_set(cl_handle_t eq_handle, cl_handle_t scene_handle)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_LINKAGE_SCENE_SET, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    e->scene_handle = scene_handle;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_linkage_scene_del(cl_handle_t eq_handle, cl_handle_t scene_handle)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_LINKAGE_SCENE_DEL, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    e->scene_handle = scene_handle;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_enable_alarm(cl_handle_t eq_handle,bool enable)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ALARM_ENABLE, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
	e->enableAlarm = enable;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_enable_alarm_push(cl_handle_t eq_handle,bool enable)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ALARM_PUSH_ENABLE, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
	e->enableAlarm = enable;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_enable_alarm_sms(cl_handle_t eq_handle,bool enable)
{
    cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_ALARM_SMS_ENABLE, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
	e->enableAlarm = enable;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_set_alarm_msg(cl_handle_t eq_handle, const char* msg)
{
    cl_notify_pkt_t *pkt;
	cln_key_info_t *k;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle||!msg||strlen(msg)<=0) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EKEY_CH_NAME, CLNPF_ACK);
	k = (cln_key_info_t *)&pkt->data[0];
    k->eq_handle = eq_handle;
    k->key_id = KEYID_ALARM_KEY;
    strncpy(k->key_name, msg, sizeof(k->key_name)-1);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

void cl_eq_free(cl_equipment_t* eq)
{
    int i;
    if (eq) {
        if (eq->alarm_info) {
            cl_alarm_info_free(eq->alarm_info);
            eq->alarm_info = NULL;
        }
        for (i=0; i<eq->key_count; i++) {
            if (eq->keys[i]) {
                cl_key_info_free(eq->keys[i]);
            }
        }
        SAFE_FREE(eq->keys);
        SAFE_FREE(eq->obj.name);
        cl_free(eq);
    }
}

CLIB_API void cl_alarm_info_free(cl_alarm_info_t* ai)
{
    int i=0;
    
    if (ai) {
        SAFE_FREE(ai->alarm_msg);
        for (i=0; i<ai->phone_num; i++) {
            SAFE_FREE(ai->phone_list[i]);
        }
        SAFE_FREE(ai->phone_list);
		SAFE_FREE(ai->soundline);
        cl_free(ai);
    }
}

/***************************************************************************************************/
// 电器按键管理
CLIB_API RS cl_key_add(cl_handle_t eq_handle,u_int32_t key_id, const char* key_name)
{
    cl_notify_pkt_t *pkt;
	cln_key_info_t *k;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!key_id||!eq_handle||!key_name||strlen(key_name)<=0) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EKEY_ADD, CLNPF_ACK);
	k = (cln_key_info_t *)&pkt->data[0];
    k->eq_handle = eq_handle;
    k->key_id = key_id;
    strncpy(k->key_name, key_name, sizeof(k->key_name)-1);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_key_del(cl_handle_t eq_handle,u_int32_t key_id)
{
    cl_notify_pkt_t *pkt;
	cln_key_info_t *k;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle||!key_id) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EKEY_DEL, CLNPF_ACK);
	k = (cln_key_info_t *)&pkt->data[0];
    k->eq_handle = eq_handle;
    k->key_id = key_id;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_key_modify_name(cl_handle_t eq_handle,u_int32_t key_id, const char* new_name)
{
    cl_notify_pkt_t *pkt;
	cln_key_info_t *k;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!key_id||!eq_handle||!new_name||strlen(new_name)<=0) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EKEY_CH_NAME, CLNPF_ACK);
	k = (cln_key_info_t *)&pkt->data[0];
    k->eq_handle = eq_handle;
    k->key_id = key_id;
    strncpy(k->key_name, new_name, sizeof(k->key_name)-1);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_key_send_ctrl_singal(cl_handle_t eq_handle,u_int32_t key_id)
{
    cl_notify_pkt_t *pkt;
	cln_key_info_t *k;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!eq_handle||!key_id) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_EKEY_CTRL, CLNPF_ACK);
	k = (cln_key_info_t *)&pkt->data[0];
    k->eq_handle = eq_handle;
    k->key_id = key_id;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

void cl_key_info_free(cl_key_t* ki)
{
    if (ki) {
        SAFE_FREE(ki->obj.name);
        cl_free(ki);
    }
}

/***************************************************************************************************/
// 电器按键学习
RS cl_set_key_learn_callback(cl_handle_t user_handle,cl_callback_t callback,void* callback_param)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle||!callback) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_SET_CALLBACK, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
    kl->callback = callback;
    kl->callback_handle = callback_param;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_key_learn_start(cl_handle_t user_handle,cl_handle_t eq_handle,u_int32_t key_id,KL_LEARN_MODE_T learn_mode)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle||!eq_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_START_LEARN, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
    kl->eq_handle = eq_handle;
    kl->key_id = key_id;
    kl->learn_mode = learn_mode;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_kl_try_ctrl(cl_handle_t user_handle)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_TRY, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_kl_gen_code(cl_handle_t user_handle)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_GEN_CODE, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_key_learn_stop(cl_handle_t user_handle)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_STOP_LEARN, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_kl_ajust_code(cl_handle_t user_handle,int ajust_value)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_AJUST_CODE, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
	kl->ajust_value = ajust_value;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API RS cl_kl_set_plus_width(cl_handle_t user_handle,bool is_narrow)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_SET_PW, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
	kl->ajust_pw_value = is_narrow?0:1;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}

#if 0
RS cl_kl_query_plus_width(cl_handle_t user_handle)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_GET_PW, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}
#endif

CLIB_API RS cl_kl_save_learn_code(cl_handle_t user_handle)
{
    cl_notify_pkt_t *pkt;
	cln_key_learn_t *kl;
	RS ret;
    
	CL_CHECK_INIT;
    
    if (!user_handle) {
        return RS_INVALID_PARAM;
    }
    
	pkt = cl_notify_pkt_new(1024, CLNE_KL_SAVE_CODE, CLNPF_ACK);
	kl = (cln_key_learn_t *)&pkt->data[0];
    kl->user_handle = user_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
    
    return ret;
}

CLIB_API cl_key_learn_t* cl_key_learn_get_stat(cl_handle_t user_handle)
{
    cl_key_learn_t* ki;
    remote_key_learn_t * kl;

    user_t * user;
    
    if (!user_handle) {
        return NULL;
    }
    
    cl_lock(&cl_priv->mutex);
    user = lookup_by_handle(HDLT_USER, user_handle);
    if (!user||!user->kl) {
        cl_unlock(&cl_priv->mutex);
        return NULL;
    }
    
    ki = cl_calloc(sizeof(cl_key_learn_t), 1);
    if (!ki) {
        return ki;
    }
    
    
    kl = user->kl;
    ki->eq_handle = kl->eq_handle;
    ki->is_support_ajust_code = kl->is_support_ajust;
    ki->max_ajust_value = kl->ajust_range;
    ki->min_ajust_value = -kl->ajust_range;
    ki->key_id = kl->key_id;
    ki->last_error = kl->last_err;
    ki->remain_time = kl->learn_remain_time;
    ki->is_narrow_plus_width = kl->ajust_current_value?false:true;
    
    cl_unlock(&cl_priv->mutex);
    
    return ki;
}

CLIB_API void cl_kl_stat_free(cl_key_learn_t* stat)
{
    SAFE_FREE(stat);
}

CLIB_API RS cl_user_scan_db_rf(cl_handle_t user_handle)
{
	cl_notify_pkt_t *pkt;
	cl_handle_t *p;
	RS ret;
	
	CL_CHECK_INIT;
	
	pkt = cl_notify_pkt_new(1024, CLNE_EQ_DB_RF_SCAN, CLNPF_ACK);
	p = (cl_handle_t *)&pkt->data[0];
	*p = user_handle;
	pkt->param_len = sizeof(cl_handle_t);
	
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
	cl_notify_pkt_free(pkt);
	
	return ret;
}

CLIB_API RS cl_eq_refresh(cl_handle_t eq_handle)
{
	cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
	if (!eq_handle) {
		return RS_INVALID_PARAM;
	}

	pkt = cl_notify_pkt_new(1024, CLNE_EQ_REFRESH, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}

CLIB_API RS cl_eq_rf_repeater_set(cl_handle_t eq_handle, u_int8_t repeater_on)
{
	cl_notify_pkt_t *pkt;
	cln_equipment_t *e;
	RS ret;
    
	CL_CHECK_INIT;
    
	if (!eq_handle) {
		return RS_INVALID_PARAM;
	}

	pkt = cl_notify_pkt_new(1024, CLNE_EQ_RF_REPEATER_SET, CLNPF_ACK);
	e = (cln_equipment_t *)&pkt->data[0];
	e->eq_handle = eq_handle;
	e->rf_repeater_on = repeater_on;
    
	ret = cl_send_notify(&cl_priv->thread_main, pkt);
    
	cl_notify_pkt_free(pkt);
    
	return ret;
}
/***************************************************************************************************/



