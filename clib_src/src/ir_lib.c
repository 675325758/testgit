/*
	红外库编码管理
*/
#include "cl_priv.h"
#include "ir_lib.h"
#include "stdarg.h"
#include "wget.h"
#include "json.h"


#define URL_INFR_V2_GET_VERSION "http://"DEFAULT_DOMAIN"/cgi-bin/ircode"
#define URL_INFR_V2_GET_CODE "http://"DEFAULT_DOMAIN"/cgi-bin/ircode?version=0"


typedef struct {
	u_int32_t id;
	char *cn;
	char *en;
} cl_eq_brand_t;

typedef struct {
	u_int32_t class_id;
	char *cn;
	char *en;

	u_int32_t num_brand;
	cl_eq_brand_t *brand;
} cl_eq_one_class_t;

typedef struct {
	cl_eq_one_class_t tv;
	cl_eq_one_class_t itv;
	cl_eq_one_class_t air_condition;
} cl_eq_all_class_t;

/*
[DEBUG] 6444:214: @00BCF400, name=, type=array, value=, next=@00000000, list=@025F6688
[DEBUG] 6444:214:     @025F6688, name=, type=object, value=, next=@025FBB40, list=@025F6718
[DEBUG] 6444:214:         @025F6718, name=class_id, type=string, value=1, next=@025F6838, list=@025F67A8
[DEBUG] 6444:214:         @025F6838, name=cn, type=string, value=鐢佃, next=@025F6958, list=@025F6910
[DEBUG] 6444:214:         @025F6958, name=en, type=string, value=TV, next=@025F69E8, list=@025F69A8
[DEBUG] 6444:229:         @025F69E8, name=brand_list, type=array, value=, next=@00000000, list=@025F6A80
[DEBUG] 6444:229:             @025F6A80, name=, type=object, value=, next=@025F8420, list=@025F6B10
[DEBUG] 6444:229:                 @025F6B10, name=id, type=string, value=1, next=@025F6BE0, list=@025F6BA0
[DEBUG] 6444:229:                 @025F6BE0, name=cn, type=string, value=闀胯櫣, next=@025F8348, list=@025F8300
[DEBUG] 6444:229:                 @025F8348, name=en, type=string, value=ChangHong, next=@00000000, list=@025F83D8
[DEBUG] 6444:229:             @025F8420, name=, type=object, value=, next=@025F8730, list=@025F84B0
[DEBUG] 6444:229:                 @025F84B0, name=id, type=string, value=3, next=@025F8580, list=@025F8540
[DEBUG] 6444:229:                 @025F8580, name=cn, type=string, value=搴蜂匠, next=@025F8658, list=@025F8610
[DEBUG] 6444:229:                 @025F8658, name=en, type=string, value=Konka, next=@00000000, list=@025F86E8
[DEBUG] 6444:260:     @025FBB40, name=, type=object, value=, next=@025FCB20, list=@025FBBD0
[DEBUG] 6444:260:         @025FBBD0, name=class_id, type=string, value=2, next=@025FBCA8, list=@025FBC68
[DEBUG] 6444:260:         @025FBCA8, name=cn, type=string, value=鏈洪《鐩? next=@025FBD80, list=@025FBD38
[DEBUG] 6444:260:         @025FBD80, name=en, type=string, value=ITV, next=@025FBE50, list=@025FBE10
[DEBUG] 6444:260:         @025FBE50, name=brand_list, type=array, value=, next=@00000000, list=@025FBEE8
[DEBUG] 6444:260:             @025FBEE8, name=, type=object, value=, next=@025FC1F8, list=@025FBF78
[DEBUG] 6444:260:                 @025FBF78, name=id, type=string, value=39, next=@025FC048, list=@025FC008
[DEBUG] 6444:260:                 @025FC048, name=cn, type=string, value=鍗庝负, next=@025FC120, list=@025FC0D8
[DEBUG] 6444:260:                 @025FC120, name=en, type=string, value=huawei, next=@00000000, list=@025FC1B0
[DEBUG] 6444:276:     @025FCB20, name=, type=object, value=, next=@00000000, list=@025FCBB0
[DEBUG] 6444:276:         @025FCBB0, name=class_id, type=string, value=3, next=@025FCC88, list=@025FCC48
[DEBUG] 6444:276:         @025FCC88, name=cn, type=string, value=绌鸿皟, next=@025FCD60, list=@025FCD18
[DEBUG] 6444:276:         @025FCD60, name=en, type=string, value=AIR-CONDITION, next=@025FCE40, list=@025FCDF0
[DEBUG] 6444:276:         @025FCE40, name=brand_list, type=array, value=, next=@00000000, list=@025FCED8
[DEBUG] 6444:276:             @025FCED8, name=, type=object, value=, next=@025FD1E8, list=@025FCF68
[DEBUG] 6444:276:                 @025FCF68, name=id, type=string, value=1, next=@025FD038, list=@025FCFF8
[DEBUG] 6444:276:                 @025FD038, name=cn, type=string, value=闀胯櫣, next=@025FD110, list=@025FD0C8
[DEBUG] 6444:276:                 @025FD110, name=en, type=string, value=ChangHong, next=@00000000, list=@025FD1A0
*/
cl_eq_all_class_t *json_to_fac_lib(json_t *json)
{
	int num;
	cl_eq_all_class_t *eac;
	cl_eq_one_class_t *oc;
	jvalue_t *jv_class, *jv_brand, *jv;

	eac = cl_calloc(sizeof(cl_eq_all_class_t), 1);
	eac->tv.class_id = 1;
	eac->itv.class_id = 2;
	eac->air_condition.class_id = 3;
	if (json->m_data == NULL || json->m_data->type != JT_A)
		goto done;

	for (jv_class = json->m_data->value.list; jv_class != NULL; jv_class = jv_class->next) {
		jv = jvalue_lookup_by_str(jv_class, "class_id");
		if (jv == NULL || jv->type != JT_S)
			continue;
		switch (atoi(jv->value.str)) {
		case 1:
			oc = &eac->tv;
			break;
		case 2:
			oc = &eac->itv;
			break;
		case 3:
			oc = &eac->air_condition;
			break;
		default:
			log_err(false, "unknow class value %s, ignore it\n", jv->value.str);
			continue;
		}

		jv = jvalue_lookup_by_str(jv_class, "cn");
		if (jv == NULL || jv->type == JT_S) {
			oc->cn = cl_strdup(jv->value.str);
		}
		
		jv = jvalue_lookup_by_str(jv_class, "en");
		if (jv == NULL || jv->type == JT_S) {
			oc->en = cl_strdup(jv->value.str);
		}
			
		jv_brand = jvalue_lookup_by_str(jv_class, "brand_list");
		if (jv_brand == NULL || jv_brand->type != JT_A) {
			log_err(false, "not found brand_list\n");
			continue;
		}

		// 计算个数
		for (num = 0, jv = jv_brand->value.list; jv != NULL; jv = jv->next) {
			if (jv->type != JT_O)
				continue;
			num++;
		}
		oc->num_brand = num;
		oc->brand = cl_calloc(sizeof(cl_eq_brand_t), num);
		// 赋值
		for (num = 0, jv_brand = jv_brand->value.list; jv_brand != NULL; jv_brand = jv_brand->next) {
			if (jv_brand->type != JT_O)
				continue;
			
			jv = jvalue_lookup_by_str(jv_brand, "id");
			if (jv == NULL || jv->type == JT_S) {
				oc->brand[num].id = atoi(jv->value.str);
			}
			
			jv = jvalue_lookup_by_str(jv_brand, "cn");
			if (jv == NULL || jv->type == JT_S) {
				oc->brand[num].cn = cl_strdup(jv->value.str);
			}
			
			jv = jvalue_lookup_by_str(jv_brand, "en");
			if (jv == NULL || jv->type == JT_S) {
				oc->brand[num].en = cl_strdup(jv->value.str);
			}

			num++;
		}
	}

done:
	return eac;
}

static void dump_one_fac(cl_eq_one_class_t *oc)
{
	int i;
	
	log_debug("id=%d, en=%s, cn=%s, num brand=%d\n", oc->class_id, oc->en, oc->cn, oc->num_brand);
	for (i = 0; i < (int)oc->num_brand; i++) {
		log_debug("    [%d]: id=%d, en=%s, cn=%s\n", i, oc->brand[i].id, oc->brand[i].en, oc->brand[i].cn);
	}
}

void fac_lib_dump(cl_eq_all_class_t *eac)
{
	dump_one_fac(&eac->tv);
	dump_one_fac(&eac->itv);
	dump_one_fac(&eac->air_condition);
}

void fac_lib_free(cl_eq_all_class_t *eac)
{
	SAFE_FREE(eac->tv.brand);
	SAFE_FREE(eac->itv.brand);
	SAFE_FREE(eac->air_condition.brand);
	cl_free(eac);
}





typedef struct {
	char *url;
	cl_thread_info_t *ti;
} wget_param_t;

RS wget_proc_notify(struct cl_notify_pkt_s *pkt)
{
	log_err(false, "ignore wget_proc_notify\n");
	
	return RS_ERROR;
}

static void wget_notify(char *json)
{
	ir_lib_t *il = cl_priv->ir_lib;

	cl_lock(&cl_priv->mutex);

	if (il->json != NULL)
		json_free(il->json);
	il->json = json;
	
	cl_unlock(&cl_priv->mutex);
}

int wget_thread(void *param)
{
	RS ret = RS_ERROR;
	char *url;
	char *json_str = NULL;
	json_t *json;
	int len;
	wget_param_t *wp = (wget_param_t *)param;

	url = cl_strdup(wp->url);
	wp->ti->proc_notify = wget_proc_notify;

	if (wget(url, &json_str, &len, false, NULL) != RS_OK)
		goto done;
	
	log_debug("IR Lib(%u):\n%s\n", len, json_str);
	log_debug("************************ parse json_str ******************************\n\n");

	json = json_parse(json_str, len);
	if (json != NULL) {
		cl_eq_all_class_t *eac;

		eac = json_to_fac_lib(json);
		fac_lib_dump(eac);
		fac_lib_free(eac);
		
		//json_dump(json);
		json_free(json);
	}

	ret = RS_OK;
	
done:
	// 通知其他线程
	wget_notify(json_str);

	cl_free(url);
	SAFE_FREE(json_str);
	
	return RS_OK;
}

void check_query_ir_url(user_t *user)
{
	u_int32_t now;
	ir_lib_t *il = cl_priv->ir_lib;
	wget_param_t wp;
	cl_thread_info_t ti;

	now = get_sec();
	if (now < il->next_check)
		return;
	if (il->json == NULL) {
		il->next_check = now + TIME_QUERY_IR_LIB_QUICK;
	} else {
		il->next_check = now + TIME_QUERY_IR_LIB_SLOW;
	}

	memset(&wp, 0, sizeof(wp));
	memset(&ti, 0, sizeof(ti));
	
	wp.url = URL_INFR_V2_GET_CODE;
	wp.ti = &ti;
	cl_create_thread(&ti, "wget", wget_thread, (void *)&wp);
	CLOSE_SOCK(ti.sock_notify);
}

