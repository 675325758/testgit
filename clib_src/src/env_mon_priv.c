/*
	环境检测，目前主要包括温度和PM2.5
*/

#include "cl_priv.h"
#include "env_mon_priv.h"
#include "cl_env_mon.h"
#include "aes.h"
#include "http.h"
#include "json.h"

#define	ENV_DOMAIN	DEFAULT_DOMAIN

static int em_timer(cl_thread_t *t);

#if 0
"sk": {	/*当前实况天气*/
	"temp": "21",	/*当前温度*/
	"wind_direction": "西风",	/*当前风向*/
	"wind_strength": "2级",	/*当前风力*/	
	"humidity": "4%",	/*当前湿度*/
	"time": "14:25"	/*更新时间*/
},
#endif

static bool parse_sk(user_t *user, jvalue_t *node)
{
	jvalue_t *jv;
	int c;
	cl_env_mon_t *em = (cl_env_mon_t *)user->env_mon;
	bool ret = false;

	if ((jv = jvalue_lookup_by_str(node, "temp")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "temperature type=%u\n", jv->type);
		} else {
			c = atoi(jv->value.str);
			if (c != em->temperature) {
				log_info("%s update temperature from %u c to %u c\n", user->name, em->temperature, c);
				em->temperature = c;
				ret = true;
			}
		}

	}
	
	if ((jv = jvalue_lookup_by_str(node, "humidity")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "humidity type=%u\n", jv->type);
		} else {
			c = atoi(jv->value.str);
			if (c != em->humidity) {
				log_info("%s update humidity from %u c to %u c\n", user->name, em->humidity, c);
				em->humidity = c;
				ret = true;
			}
		}

	}
    
    if ((jv = jvalue_lookup_by_str(node, "time")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "time type=%u\n", jv->type);
		} else {
			if (jv->value.str && jv->value.str[0]!='\0') {
                STR_REPLACE(em->wear_last_update_time, jv->value.str);
            }else{
                SAFE_FREE(em->wear_last_update_time);
            }
		}
        
	}

	return ret;
}

#if 0
"today": {
    "city": "天津",
    "date_y": "2014年03月21日",
    "week": "星期五",
    "temperature": "8℃~20℃",	/*今日温度*/
    "weather": "晴转霾",	/*今日天气*/
    "weather_id": {	/*天气唯一标识*/
        "fa": "00",	/*天气标识00：晴*/
        "fb": "53"	/*天气标识53：霾 如果fa不等于fb，说明是组合天气*/
    },
    "wind": "西南风微风",
    "dressing_index": "较冷", /*穿衣指数*/
    "dressing_advice": "建议着大衣、呢外套加毛衣、卫衣等服装。",	/*穿衣建议*/
    "uv_index": "中等",	/*紫外线强度*/
    "comfort_index": "",
    "wash_index": "较适宜",	/*洗车指数*/
    "travel_index": "适宜",	/*旅游指数*/
    "exercise_index": "较适宜",	/*晨练指数*/
    "drying_index": ""
},
#endif

static bool parse_today(user_t *user, jvalue_t *node)
{
	jvalue_t *jv;
	int c1, c2;
	char *p;
	bool ret = false;
	cl_env_mon_t *em = (cl_env_mon_t *)user->env_mon;

	if ((jv = jvalue_lookup_by_str(node, "city")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "city type=%u\n", jv->type);
		} else 	if (em->city == NULL || strcmp(em->city, jv->value.str) != 0) {
			log_info("%s update city from %s to %s\n", user->name, em->city ? em->city : "", jv->value.str);
			STR_REPLACE(em->city, jv->value.str);
			ret = true;
		}
	}

	if ((jv = jvalue_lookup_by_str(node, "temperature")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "temperature type=%u\n", jv->type);
		} else {
			c1 = atoi(jv->value.str);
			if ((p = strchr(jv->value.str, '~')) != NULL) {
				p++;
				c2 = atoi(p);
				
				if (c1 != em->temp_min || c2 != em->temp_max) {
					log_info("%s update temperature range from %d~%d TO %d~%d\n", 
						user->name, em->temp_min, em->temp_max, c1, c2);
					em->temp_min = c1;
					em->temp_max = c2;
					ret = true;
				}
			} else {
				log_err(false, "Bad temperature = %s\n", jv->value.str);
			}
		}
	}

	if ((jv = jvalue_lookup_by_str(node, "weather")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "weather=%u\n", jv->type);
		} else 	if (em->weather == NULL || strcmp(em->weather, jv->value.str) != 0) {
			log_info("%s update weather from %s to %s\n", user->name, em->weather ? em->weather : "", jv->value.str);
			STR_REPLACE(em->weather, jv->value.str);
			ret = true;
		}
	}

	return ret;
}

RS em_on_get_weather(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *node, *jv;
	user_t *user = NULL;
	bool info_modify = false;
	cl_env_mon_t *em = NULL;

	h = (cln_http_t *)pkt->data;

#ifdef	DBG_ENV_MON
	log_debug("env get weather: handle=0x%08x, result=%d, page len=%u, url=%s\n",
		h->handle, h->result, strlen(h->page), h->url);
	log_debug("\n%s\n", h->page);
#endif

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	user->query_env_count++;
	em = (cl_env_mon_t *)user->env_mon;
	if (h->result != RS_OK) {
		CL_THREAD_OFF(user->t_timer_query_env);
		CL_THREAD_TIMER_ON(MASTER, user->t_timer_query_env, em_timer, (void *)user, TIME_N_SECOND(user->query_env_count));
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_ENV_MON
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if (atoi(jv->value.str) != 0) {
				log_err(false, "user %s get weather, result=%s\n", user->name, jv->value.str);
				goto err;
			}
		} else {
			log_err(false, "user %s get weather failed: no result code\n", user->name);
			goto err;
		}
		if ((jv = jvalue_lookup_by_str(json->m_data, "data")) == NULL) {
			log_err(false, "user %s get weather failed: no data\n", user->name);
			goto err;
		}

		if ((jv = jvalue_lookup_by_str(jv, "result")) != NULL) {
			// 当前温度
			if ((node = jvalue_lookup_by_str(jv, "sk")) != NULL) {
				info_modify |= parse_sk(user, node);
			}
			// 今天温度
			if ((node = jvalue_lookup_by_str(jv, "today")) != NULL && node->type == JT_O) {
				info_modify |= parse_today(user, node);
			}
		}

err:		
		json_free(json);
	}

	em->get_weather_done = true;
	
done:
	if (info_modify && (em != NULL && em->get_air_done && em->get_weather_done))
        event_push(user->callback, UE_ENV_MON_MODIFY, user->handle, user->callback_handle);
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

#if 0
"1": {
    "city": "上方山",
    "AQI": "77",
    "quality": "良",
    "PM2.5Hour": "46μg/m??",
    "PM2.5Day": "46μg/m??",
    "PM10Hour": "104μg/m??",
    "lat": "31.247222",
    "lon": "120.561389"
},
#endif

static bool parse_pm25(user_t *user, jvalue_t *node)
{
	jvalue_t *jv;
	int total = 0, count = 0, v;
	cl_env_mon_t *em = (cl_env_mon_t *)user->env_mon;

	for (node = node->value.list; node != NULL; node = node->next) {
		if ((jv = jvalue_lookup_by_str(node, "PM2.5Hour")) != NULL && jv->type == JT_S) {
			v = atoi(jv->value.str);
			if (v <= 0) {
				log_err(false, "%s bad PM2.5: %s\n", user->name, jv->value.str);
				continue;
			}

			total += v;
			count++;
		}
	}
	if (count == 0) {
		log_err(false, "%s not found PM2.5\n", user->name);
		return false;
	}

	total /= count;
	if (total != em->pm25) {
		log_info("%s PM2.5 update from %u to %u\n", user->name, em->pm25, total);
		em->pm25 = total;
		return true;
	}

	return false;
}

#if 0
"citynow": {
    "city": "suzhou",
    "AQI": "77",
    "quality": "良",
    "date": "2014-05-09 14:00"
},
#endif

static bool parse_air_citynow(user_t *user, jvalue_t *node)
{
	jvalue_t *jv;
	int n;
	bool ret = false;
	cl_env_mon_t *em = (cl_env_mon_t *)user->env_mon;

	if ((jv = jvalue_lookup_by_str(node, "AQI")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "AQI type=%u\n", jv->type);
		} else {
			n = atoi(jv->value.str);
			if (em->aqi != n) {
				log_info("%s update AQI from %u to %u\n", user->name, em->aqi, n);
				em->aqi = n;
				ret = true;
			}
		}
	}

	if ((jv = jvalue_lookup_by_str(node, "quality")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "quality type=%u\n", jv->type);
		} else {
			if (em->air_quality == NULL || strcmp(em->air_quality, jv->value.str) != 0) {
				log_info("%s update air qulity from %s to %s\n", 
					user->name, em->air_quality ? em->air_quality : "", jv->value.str);
				STR_REPLACE(em->air_quality, jv->value.str);
				ret = true;
			}
		}
	}
    
    if ((jv = jvalue_lookup_by_str(node, "date")) != NULL) {
		if (jv->type != JT_S) {
			log_err(false, "date type=%u\n", jv->type);
		} else {
            if (jv->value.str && jv->value.str[0]!='\0') {
                STR_REPLACE(em->aqi_last_update_time, jv->value.str);
            }else{
                SAFE_FREE(em->aqi_last_update_time);
            }
		}
	}

	return ret;
}

RS em_on_get_pm25(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *node, *jv;
	user_t *user = NULL;
	bool info_modify = false;
	cl_env_mon_t *em = NULL;

	h = (cln_http_t *)pkt->data;

#ifdef	DBG_ENV_MON
	log_debug("env get PM2.5: handle=0x%08x, result=%d, page len=%u, url=%s\n",
		h->handle, h->result, strlen(h->page), h->url);
	log_debug("\n%s\n", h->page);
#endif

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	user->query_env_count++;
	em = (cl_env_mon_t *)user->env_mon;
	if (h->result != RS_OK) {
		CL_THREAD_OFF(user->t_timer_query_env);
		CL_THREAD_TIMER_ON(MASTER, user->t_timer_query_env, em_timer, (void *)user, TIME_N_SECOND(user->query_env_count));
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_ENV_MON
		json_dump(json);
#endif

		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if (atoi(jv->value.str) != 0) {
				log_err(false, "user %s get PM2.5, result=%s\n", user->name, jv->value.str);
				goto err;
			}
		} else {
			log_err(false, "user %s get PM2.5 failed: no result code\n", user->name);
			goto err;
		}
		if ((jv = jvalue_lookup_by_str(json->m_data, "data")) == NULL) {
			log_err(false, "user %s get PM2.5 failed: no data\n", user->name);
			goto err;
		}

		if ((jv = jvalue_lookup_by_str(jv, "result")) != NULL) {
			// 当前质量
			if ((node = jvalue_lookup_by_str(jv->value.list, "citynow")) != NULL && node->type == JT_O) {
				info_modify |= parse_air_citynow(user, node);
			}

			// 最近PM2.5
			if ((node = jvalue_lookup_by_str(jv->value.list, "lastMoniData")) != NULL && node->type == JT_O) {
				info_modify |= parse_pm25(user, node);
			}
		}

err:		
		json_free(json);
	}

	em->get_air_done = true;
	
done:
    cl_unlock(&cl_priv->mutex);
    
	if (info_modify && (em != NULL && em->get_air_done && em->get_weather_done))
        event_push(user->callback, UE_ENV_MON_MODIFY, user->handle, user->callback_handle);
	
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

static int em_timer(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);
	char url[256];

	user->t_timer_query_env = NULL;
	// 下次半个小时以后
	CL_THREAD_TIMER_ON(MASTER, user->t_timer_query_env, em_timer, (void *)user, ONE_HOUR_SECOND*1000/2);
    
    USER_BACKGROUND_RETURN_CHECK(user);
    
    if (user->is_phone_user) {
		sprintf(url, "http://%s/cgi-bin/weather?action=weather&phone=%s", ENV_DOMAIN, user->name);
		http_get(url, CLNE_HTTP_ENV_GET_WEATHER, user->handle);

		sprintf(url, "http://%s/cgi-bin/weather?action=pm&phone=%s", ENV_DOMAIN, user->name);
		http_get(url, CLNE_HTTP_ENV_GET_PM25, user->handle);
	} else {
		sprintf(url, "http://%s/cgi-bin/weather?action=weather&phone=123&sn=%012"PRIu64"", ENV_DOMAIN, user->sn);
		http_get(url, CLNE_HTTP_ENV_GET_WEATHER, user->handle);

		sprintf(url, "http://%s/cgi-bin/weather?action=pm&phone=123&sn=%012"PRIu64"", ENV_DOMAIN, user->sn);
		http_get(url, CLNE_HTTP_ENV_GET_PM25, user->handle);
	}

	return 0;
}

RS em_on_set_city(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	user_t *user;

	h = (cln_http_t *)pkt->data;

	log_debug("env set city: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	// 重新查询该设备的天气
	CL_THREAD_OFF(user->t_timer_query_env);
	em_start(user);
	
done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

/*
{
	"result":0, 
	"errordesc":"查询成功", 
	"data":[
		{"province":"四川","citylist":["成都","绵阳","南充"]},
		{"province":"直辖市","citylist":["北京","上海","重庆","香港","澳门"]},
		{"province":"云南","citylist":["昆明","大理","丽江"]}
	]
}
*/

static RS parse_city_list(jvalue_t *root)
{
	int prov_count, city_count;
	jvalue_t *node, *jv, *jc;
	cl_province_t *prov;
	cl_city_list_t *city_list;

	// 计算个数
	for (node = root->value.list, prov_count = 0; node != NULL; node = node->next) {
		if (node->type != JT_O) {
			continue;
		}
		prov_count++;
	}
	if (prov_count == 0) {
		log_err(false, "parse_city_list failed: no city\n");
		return RS_ERROR;
	}

	city_list = cl_calloc(sizeof(cl_city_list_t), 1);
	city_list->province = cl_calloc(sizeof(cl_province_t *), prov_count);
	
	for (node = root->value.list, prov_count = 0; node != NULL; node = node->next) {
		if (node->type != JT_O) {
			continue;
		}
		
		if ((jv = jvalue_lookup_by_str(node, "province")) == NULL || jv->type != JT_S) {
			log_err(false, "parse_city_list not found province or bad type\n");
			continue;;
		}

		prov = cl_calloc(sizeof(cl_province_t), 1);
		city_list->province[prov_count++] = prov;

		prov->name = cl_strdup(jv->value.str);
		
		if ((jv = jvalue_lookup_by_str(node, "citylist")) == NULL || jv->type != JT_A) {
			log_err(false, "parse_city_list not found citylist or bad type\n");
			continue;;
		}
		// 算该省份城市个数
		for (jc = jv->value.list, city_count = 0; jc != NULL; jc = jc->next, city_count++) 
			;
		prov->city = cl_calloc(sizeof(char *), city_count);
		for (jc = jv->value.list, city_count = 0; jc != NULL; jc = jc->next) {
			prov->city[city_count++] = cl_strdup(jc->name);
		}
		prov->city_num = city_count;
	}

	city_list->province_num = prov_count;

	CL_LOCK;
	cl_em_free_city_list(cl_priv->city_list);
	cl_priv->city_list = city_list;
	CL_UNLOCK;

	return RS_OK;
}

RS em_on_get_city_list(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;

	h = (cln_http_t *)pkt->data;

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_ENV_MON
		json_dump(json);
#endif		
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if (atoi(jv->value.str) != 0) {
				log_err(false, "get city list, result=%s\n", jv->value.str);
				goto err;
			}
		} else {
			log_err(false, "get city list: no result code\n");
			goto err;
		}
		if ((jv = jvalue_lookup_by_str(json->m_data, "data")) == NULL) {
			log_err(false, "get city list failed: no data\n");
			goto err;
		}

		if (jv->type != JT_A) {
			log_err(false, "get city list failed: type=%d\n", jv->type);
			goto err;
		}
		parse_city_list(jv);

err:		
		json_free(json);
	}
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	cl_priv->query_city_list_now = false;
	
	return RS_OK;
}

RS em_get_city_list()
{
	char url[256];

	cl_priv->query_city_list_now = true;
	
	sprintf(url, "http://%s/cgi-bin/weather?action=get_city_list", ENV_DOMAIN);
	// 用户不会这么快修改城市，可以稍做延时
	http_get(url, CLNE_HTTP_ENV_GET_CITY_LIST, 300);

	return 0;
}
	
RS em_start(user_t *user)
{
	CL_THREAD_TIMER_ON(MASTER, user->t_timer_query_env, em_timer, (void *)user, range_rand(0, 1000));

	log_debug("env monitor start for user %s (sn=%012"PRIu64")\n", user->name, user->sn);

	if (cl_priv->city_list == NULL && (!cl_priv->query_city_list_now))
		em_get_city_list();

	return RS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

/*
	获取某个设备所在城市的环境参数
*/
CLIB_API cl_env_mon_t *cl_em_get_info(cl_handle_t user_handle)
{
	cl_env_mon_t *dst = NULL, *src;
	user_t *user;

	CL_CHECK_INIT_RP;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_em_get_info: lookup user handle=0x%08x failed\n", user_handle);
		goto done;
	}
	dst = (cl_env_mon_t *)cl_calloc(sizeof(cl_env_mon_t), 1);
	src = (cl_env_mon_t *)user->env_mon;

	dst->name = cl_strdup(user->name ? user->name : "");
	dst->sn = user->sn;
	dst->get_air_done = src->get_air_done;
	dst->get_weather_done = src->get_weather_done;
	dst->get_suggest_done = src->get_suggest_done;
	dst->city = cl_strdup(src->city ? src->city : "");
	dst->aqi = src->aqi;
	dst->pm25 = src->pm25;
	dst->air_quality = cl_strdup(src->air_quality ? src->air_quality : "");
	dst->temperature = src->temperature;
	dst->humidity = src->humidity;
	dst->temp_min = src->temp_min;
	dst->temp_max = src->temp_max;
	dst->weather = cl_strdup(src->weather ? src->weather : "");
	dst->wear_last_update_time = cl_strdup(src->wear_last_update_time?src->wear_last_update_time:"");
	dst->aqi_last_update_time = cl_strdup(src->aqi_last_update_time?src->aqi_last_update_time:"");

	if(user->sub_type == IJ_805){
		dst->suggest_temp = cl_strdup(src->suggest_temp?src->suggest_temp:"");
		dst->suggest_humidity= cl_strdup(src->suggest_humidity?src->suggest_humidity:"");
		dst->suggest_air = cl_strdup(src->suggest_air?src->suggest_air:"");
	}else{
		dst->suggest_temp = NULL;
		dst->suggest_humidity = NULL;
		dst->suggest_air = NULL;
	}
	
done:
	cl_unlock(&cl_priv->mutex);
	
	return dst;
}

/*
	释放cl_em_get_info返回的数据
*/
CLIB_API void cl_em_free_info(cl_env_mon_t *cl)
{
	SAFE_FREE(cl->name);
	SAFE_FREE(cl->city);
	SAFE_FREE(cl->air_quality);
	SAFE_FREE(cl->weather);
	SAFE_FREE(cl->aqi_last_update_time);
	SAFE_FREE(cl->wear_last_update_time);
	SAFE_FREE(cl->suggest_temp);
	SAFE_FREE(cl->suggest_humidity);
	SAFE_FREE(cl->suggest_air);
	SAFE_FREE(cl);
}


/*
	设置设备所在的城市
	设置成功后，立即触发一次查询
*/
CLIB_API RS cl_em_set_city(cl_handle_t user_handle, const char *city)
{
	user_t *user;
	RS ret = RS_ERROR;
	char url[256];

	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_em_set_city: lookup user handle=0x%08x failed\n", user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	if (user->is_phone_user) {
		sprintf(url, "http://%s/cgi-bin/weather?action=set_city&phone=%s&city=%s", 
			ENV_DOMAIN, user->name, city);
	} else {
		if (user->sn == 0) {
			log_err(false, "cl_em_set_city: user %s unknow sn\n", user->name);
			ret = RS_NOT_SUPPORT;
			goto done;
		}

		sprintf(url, "http://%s/cgi-bin/weather?action=set_city&sn=%012"PRIu64"&city=%s", 
			ENV_DOMAIN, user->sn, city);
	}
	http_get(url, CLNE_HTTP_ENV_SET_CITY, user->handle);

	ret = RS_OK;
	
done:
	cl_unlock(&cl_priv->mutex);

	return ret;
}
static RS parse_env_suggest(cl_env_mon_t *em, jvalue_t *node)
{
	jvalue_t *jv;
//	{"result":0, "errordesc":"OK", "data":{"temp":"","humidity":"","voc":"","pm25":"","pm10":"","no2":"","so2":"","o3":"","co":""}

	SAFE_FREE(em->suggest_temp);
	SAFE_FREE(em->suggest_humidity);
	SAFE_FREE(em->suggest_air);
	
	if ((jv = jvalue_lookup_by_str(node, "temp")) != NULL) {
		if (jv->type == JT_S) {
			em->suggest_temp = cl_strdup(jv->value.str);
		} 
	}

	if ((jv = jvalue_lookup_by_str(node, "humidity")) != NULL) {
		if (jv->type == JT_S) {
			em->suggest_humidity= cl_strdup(jv->value.str);
		} 
	}

	if ((jv = jvalue_lookup_by_str(node, "pm25")) != NULL) {
		if (jv->type == JT_S) {
			em->suggest_air = cl_strdup(jv->value.str);
		} 
	}
	
	if(em->suggest_temp && em->suggest_humidity && em->suggest_air){
		em->get_suggest_done  = true;
		return RS_OK;
	}else{
		em->get_suggest_done  = false;
		return RS_ERROR;
	}
}

RS em_on_get_suggest(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json = NULL;
	jvalue_t *jv;
	user_t *user;
	cl_env_mon_t *em;
	RS ret = RS_ERROR;

	h = (cln_http_t *)pkt->data;
	
	cl_lock(&cl_priv->mutex);
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	em = (cl_env_mon_t *)user->env_mon;
	if(em == NULL)
		goto done;
	em->get_suggest_done = false;
	
	json = json_parse(h->page, (int)strlen(h->page));
	if (json == NULL) 
		goto done;
#ifdef	DBG_ENV_MON
	json_dump(json);
#endif
	if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
		if (atoi(jv->value.str) != 0) {
			log_err(false, "%s, result=%s\n", __FUNCTION__, jv->value.str);
			goto done;
		}
	} else {
		log_err(false, "%s: no result code\n", __FUNCTION__);
		goto done;
	}

	if ((jv = jvalue_lookup_by_str(json->m_data, "data")) == NULL) {
		log_err(false, "%s failed: no data\n", __FUNCTION__);
		goto done;
	}

	if (jv->type != JT_O) {
		log_err(false, "%s failed: type=%d\n", __FUNCTION__, jv->type);
		goto done;
	}
	ret = parse_env_suggest(em, jv);
	
done:
	cl_unlock(&cl_priv->mutex);
	if(json)
		json_free(json);
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	if(ret == RS_OK)
		event_push(user->callback, UE_ENV_WEATHER_SUGGEST, user->handle, user->callback_handle);	
	
	return RS_OK;
}

CLIB_API RS cl_em_get_suggest(cl_handle_t user_handle, const env_air_t *env)
{
	user_t *user;
	RS ret = RS_ERROR;
	char url[512];

	CL_CHECK_INIT;
	
	cl_lock(&cl_priv->mutex);

	if ((user = lookup_by_handle(HDLT_USER, user_handle)) == NULL) {
		log_err(false, "cl_em_set_city: lookup user handle=0x%08x failed\n", user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	/*
	http://www.jiazhang007.com/cgi-bin/health_suggest?temp=35&humidity=90&voc=500&pm25=125&pm10=0&no2=0&so2=0&o3=0&co=0
	返回：
	{"result":0, "errordesc":"OK", "data":{"temp":"","humidity":"","voc":"","pm25":"","pm10":"","no2":"","so2":"","o3":"","co":""}
	{"result":1, "errordesc":"(失败原因)"}
	*/
	sprintf(url, "http://%s/cgi-bin/health_suggest?temp=%d&humidity=%d&voc=%d&pm25=%d&pm10=%d&no2=%d&so2=%d&o3=%d&co=%d",
		ENV_DOMAIN, env->temp, env->humidity, env->voc, env->pm25, env->pm10, env->no2, env->so2, env->o3, env->co);
	http_get(url, CLNE_HTTP_ENV_GET_SUGGEST, user->handle);
	ret = RS_OK;
	
done:
	cl_unlock(&cl_priv->mutex);
	return ret;
}

CLIB_API cl_city_list_t *cl_em_get_city_list()
{
	int i, k;
	cl_city_list_t *src, *dst = NULL;
	
	CL_CHECK_INIT_RP;
	
	CL_LOCK;

	if ((src = (cl_city_list_t *)cl_priv->city_list) == NULL)
		goto done;

	dst = (cl_city_list_t *)cl_calloc(sizeof(cl_city_list_t), 1);
	dst->province_num = src->province_num;
	dst->province = cl_calloc(sizeof(void *), src->province_num);

	for (i = 0; i < src->province_num; i++) {
		dst->province[i] = cl_calloc(sizeof(cl_province_t), 1);
		dst->province[i]->name = cl_strdup(src->province[i]->name);
		dst->province[i]->city_num = src->province[i]->city_num;
		dst->province[i]->city = cl_calloc(sizeof(void *), src->province[i]->city_num);
		
		for (k = 0; k < src->province[i]->city_num; k++) {
			dst->province[i]->city[k] = cl_strdup(src->province[i]->city[k]);
		}
	}

done:	
	CL_UNLOCK;

	return dst;
}

CLIB_API void cl_em_free_city_list(cl_city_list_t *cl)
{
	int i, k;
	cl_province_t *pro;

	if (cl == NULL)
		return;
	
	for (i = 0; i < cl->province_num; i++) {
		pro = cl->province[i];
		if(!pro)
			continue;
		for (k = 0; k < pro->city_num; k++) {
			SAFE_FREE(pro->city[k]);
		}
		SAFE_FREE(pro->name);
		SAFE_FREE(pro->city);
		cl_free(pro);
	}
	SAFE_FREE(cl->province);
	SAFE_FREE(cl);
}

