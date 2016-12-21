#include "lan_dev_probe_priv.h"
#include "cl_priv.h"
#include "aes.h"
#include "http.h"
#include "json.h"
#include "phone_user.h"
#include "notify_push_priv.h"
#include "md5.h"
#include "smart_appliance_priv.h"
#include "cl_dns.h"
#include "misc_client.h"
#include "linkage_client.h"
extern void pu_get_dev(user_t *user);
static void pu_try_syn_apns(user_t *user, bool need_syn);

/********************打印调试开关********************打印调试开关***************
*************打印调试开关********************打印调试开关*******************/
#if 0
/*
* 调试级别
* DEBUG_LOCAL_LEVEL = 0 或者不定义就是不开打印
* DEBUG_LOCAL_LEVEL = 1 只开error打印。
* DEBUG_LOCAL_LEVEL = 2 开启error,info打印。
* DEBUG_LOCAL_LEVEL = 3 开启所有打印
*/
#define DEBUG_LOCAL_LEVEL	(3)
#include "cl_log_debug.h"
#endif
 
//#define	DBG_PHONE_USER	1
/////////////////////////////////////////////////////////////////////////////////////////////

void pu_parse_key(user_t *user, char *key_str)
{
	int i;
	u_int8_t plain[16];
	char tmp[3], *endp;
	u_int64_t pn = 0;
	u_int32_t session_id = 0;
	u_int32_t timeout = 0;

	strncpy(user->pu_key, key_str, sizeof(user->pu_key));
	
	tmp[2] = '\0';
	
	for (i = 0; i < 16; i++) {
		tmp[0] = key_str[i*2];
		tmp[1] = key_str[i*2 + 1];
		
		plain[i] = (u_int8_t)strtoul(tmp, &endp, 16);
	}

	dec_block(plain, sizeof(plain), user->pu_preshare_passwd);

	for (i = 0; i < 8; i++) {
		pn = (pn<<8) | plain[i];
	}
	for ( ; i < 12; i++) {
		session_id = (session_id<<8) | plain[i];
	}
	for ( ; i < 16; i++) {
		timeout = (timeout<<8) | plain[i];
	}
	log_info("%s, phone number=%"PRIu64", session id=0x%x, timeout=%u, now=%u, diff=%u\n", 
		user->name, pn, session_id, timeout, get_sec(), timeout - get_sec());

	if (user->pu_from_cache) {
		user->pu_die = 5*60;
	} else {
		user->pu_die = timeout;
	}
}

int pu_get_timeout(user_t *user)
{
	int timeout;

	// 提前60秒重新认证
	timeout = user->pu_die - get_sec() - 60;
	if (timeout <= 0)
		timeout = TIME_CONNECT;
	else
		timeout *= 1000;

	return timeout;
}


/////////////////////////////////////////////////////////////////////////////////////////////

#define	PHONE_USER_PRESHARE_KEY "&d5wgfkj8%@#2gu6"

static void pu_init_preshare_passwd(u_int8_t *pu_preshare_passwd)
{
	int i;
	char pre_pwd[32];

	for (i = 0; i < AES128_EKY_LEN; i++) {
		pre_pwd[i] = PHONE_USER_PRESHARE_KEY[(i+AES128_EKY_LEN-1) % AES128_EKY_LEN];
	}
	pre_pwd[i] = '\0';
	
	hash_passwd(pu_preshare_passwd, pre_pwd);
}

/* 登录密码MD5用共享密钥加密，加密算法AES128，结果长度16bytes,再字符串化：32bytes */
void pu_init_passwd(user_t *user)
{
	int i;
	u_int8_t pwd[AES128_EKY_LEN];

	pu_init_preshare_passwd(user->pu_preshare_passwd);
	memcpy(pwd, user->passwd_md5, AES128_EKY_LEN);
	
	enc_block(pwd, AES128_EKY_LEN, user->pu_preshare_passwd);
	for (i = 0; i < AES128_EKY_LEN; i++) {
		sprintf(&user->pu_passwd[i*2], "%02x", pwd[i]);
	}
}

// 登录密码MD5 并用共享密钥加密，字符串长度32字节
char *pu_enc_passwd(char *pwd, char *out)
{
	int i;
	u_int8_t passwd_md5[16], pu_preshare_passwd[32];

	pu_init_preshare_passwd(pu_preshare_passwd);
	hash_passwd(passwd_md5, pwd);
	
	enc_block(passwd_md5, AES128_EKY_LEN, pu_preshare_passwd);
	for (i = 0; i < AES128_EKY_LEN; i++) {
		sprintf(out + (i*2), "%02x", passwd_md5[i]);
	}

	return out;
}

/////////////////////////////////////////////////////////////////////////////////////////////

RS pu_ignore_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	h = (cln_http_t *)pkt->data;

	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}


/////////////////////////////////////////////////////////////////////////////////////////////

/*
	手机帐号下挂的设备，需要本地缓存，这样局域网才可以登录
	缓存文件的格式为:
	手机帐号md5  缓存内容字节数 [回车换行]
	缓存内容。。。
*/
static RS pu_write_local_cache(user_t *user)
{
	int data_len, count;
	long pos_begin, pos_end;
	char fn[256];
	FILE *fp;
	user_t *dev;
	
	if (cl_priv->dir == NULL) {
		log_err(false, "%s write cache failed: no work directory.\n", user->name);
		return RS_ERROR;
	}

	sprintf(fn, "%s/%s.cache", cl_priv->dir, user->name);
	fn[sizeof(fn) - 1] = '\0';
	if ((fp = fopen(fn, "w+b")) == NULL) {
		log_err(false, "create file %s failed\n", fn);
		return RS_ERROR;
	}

	fprintf(fp, "%s                                  \n", user->passwd_md5_str);
	pos_begin = ftell(fp);

	stlc_list_count(count, &user->dev);

	fprintf(fp, "{\n");
	fprintf(fp, "\t\"result\":	0,\n");
	fprintf(fp, "\t\"errordesc\":	\"\",\n");
	fprintf(fp, "\t\"count\":	%u,\n", count);
	fprintf(fp, "\t\"kvlist\":	[\n");

	count = 0;
	stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
		fprintf(fp, "\t\t%s{\n", count ? "," : "");
		count++;

		fprintf(fp, "\t\t\t\"key\": \"dinf_v2_%s\",\n", dev->name);
		fprintf(fp, "\t\t\t\"value\": \"{\\\"nickname\\\":\\\"%s\\\",\\\"devtype\\\":\\\"%u\\\",\\\"vendorid\\\":\\\"%s\\\",\\\"pwd\\\":\\\"%s\\\",\\\"valid\\\":\\\"1\\\"}\"\n", 
			dev->nickname ? dev->nickname : "", dev->sub_type, dev->vendor_id ? dev->vendor_id : "", dev->passwd_md5_str);

		fprintf(fp, "\t\t}\n");
	}
	
	fprintf(fp, "\t]\n");
	fprintf(fp, "}");
	
	pos_end = ftell(fp);
	data_len = pos_end - pos_begin;
	fseek(fp, pos_begin - 14, SEEK_SET);
	fprintf(fp, "%u", data_len);

	fclose(fp);
	log_info("cache user %s to local success! data length=%u\n", user->name, data_len);
	
	return RS_OK;
}

static RS pu_read_local_cache(user_t *user, cln_http_t *h)
{
	// 要检查密码，密码一样才能恢复
	int data_len;
	char fn[256], line[128], *p;
	FILE *fp;
	long pos_begin, pos_end;
	RS ret = RS_ERROR;
	
	if (cl_priv->dir == NULL) {
		log_err(false, "%s read cache failed: no work directory.\n", user->name);
		return RS_ERROR;
	}

	sprintf(fn, "%s/%s.cache", cl_priv->dir, user->name);
	fn[sizeof(fn) - 1] = '\0';
	if ((fp = fopen(fn, "rb")) == NULL) {
		log_err(false, "open file %s failed\n", fn);
		return RS_ERROR;
	}

	if (fgets(line, sizeof(line), fp) == NULL) {
		log_err(false, "read first line from %s failed.\n", fn);
		goto done;
	}

	if ((p = strchr(line, ' ')) == NULL) {
		log_err(false, "%s bad format: %s.\n", fn, line);
		goto done;
	}

	*p = '\0';
	if (strcasecmp(user->passwd_md5_str, line) != 0) {
		log_err(false, "%s bad password.\n", fn);
		ret = ERR_PASSWD_INVALID;
		goto done;
	}

	p++;
	data_len = atoi(p);
	
	pos_begin = ftell(fp);
	fseek(fp, 0, SEEK_END);
	pos_end = ftell(fp);
	fseek(fp, pos_begin, SEEK_SET);

	log_debug("data_len=%d, pos_begin=%d, pos_end=%d, pos_end - pos_begin=%d\n", data_len, pos_begin, pos_end, pos_end - pos_begin);
	if (data_len > pos_end - pos_begin) {
		log_err(false, "%s bad format: not enought data.\n", fn);
		goto done;
	}

	SAFE_FREE(h->page);
	h->page = cl_malloc(data_len + 1);
	if (fread(h->page, data_len, 1, fp) != 1) {
		log_err(false, "%s read data failed.\n", fn);
		goto done;
	}
	h->page[data_len] = '\0';

	log_debug("read phone user cache success, data_len=%d, data=:\n%s", data_len, h->page);

	ret = RS_OK;
	
done:
	fclose(fp);
	log_info("cache user %s to local success! data length=%u\n", user->name, data_len);
	
	return ret;
}

static RS pu_try_read_cache(user_t *user, cl_notify_pkt_t *pkt)
{
	RS ret;
	cln_http_t *h;

	h = (cln_http_t *)pkt->data;
	
	if (user->status == CS_ESTABLISH) {
		log_info("It just refresh key, not login.\n");
		return RS_OK;
	} 
	
	if ((ret = pu_read_local_cache(user, h)) != RS_OK) {
		log_err(false, "phone user %s login failed: network error, and no cache, ret=%d.\n", user->name, ret);
		user->last_err = (ret == ERR_PASSWD_INVALID ? ULGE_BAD_PASSWORD : ULGE_NETWORK_ERROR);
		user_set_status(user, CS_LOGIN_ERR);
		return RS_ERROR;
	}
	
	user->pu_from_cache = true;
	user->online = true;
	user_set_status(user, CS_ESTABLISH);

	h->result = RS_OK;
	pu_on_get_dict(pkt);

	return RS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void pu_get_dict(user_t *user)
{
	char url[256], post_data[128];
	
	sprintf(url, "http://%s/cgi-bin/kvdict",DEFAULT_DOMAIN);
	sprintf(post_data, "key=%s", user->pu_key);
	http_post(url, post_data, CLNE_HTTP_PU_GET_DICT, user->handle);
}

bool pu_is_valid_dev_type(int dev_type)
{
	if(cl_priv->app_type == APP_TYPE_AIR_PLUG){
		if(dev_type != IJ_808){
			return false;
		}
	}
	return true;
}

int pu_parse_dict_v2(user_t *user, jvalue_t *dev_list)
{
	jvalue_t *node, *jv;
	int i, count = 0;
	struct stlc_list_head user_list;
	user_t *dev, *dev_next;
	char tmp[3], *endp;
	char *name, pwd[64];
	u_int8_t pwd_md5[16];
	json_t *json;

	STLC_INIT_LIST_HEAD(&user_list);
	tmp[2] = '\0';
	
	for (node = dev_list->value.list; node != NULL; node = node->next) {
		if (node->type != JT_O)
			continue;

		// parse name
		if ((jv = jvalue_lookup_by_str(node, "key")) == NULL || jv->type != JT_S) {
			log_err(false, "no key\n");
			continue;
		}
		if (strlen(jv->value.str) < sizeof(DEV_DICT_KEY_V2) || strncmp(jv->value.str, DEV_DICT_KEY_V2, sizeof(DEV_DICT_KEY_V2)-1) != 0) {
			log_err(false, "ignore key=%s\n", jv->value.str);
			continue;
		}
		name = &jv->value.str[sizeof(DEV_DICT_KEY_V2) - 1];

        
		// parse other
		if ((jv = jvalue_lookup_by_str(node, "value")) == NULL || jv->type != JT_S) {
			log_err(false, "%s no value\n", name);
			continue;
		}
		if ((json = json_parse(jv->value.str, (int)strlen(jv->value.str))) == NULL) {
			continue;
		}
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif

		if ((jv = jvalue_lookup_by_str(json->m_data, "valid")) == NULL || jv->type != JT_S) {
			log_err(false, "no valid\n");
			goto one_done;
		}
		count++;
		if (atoi(jv->value.str) == 0 ) {
			goto one_done;
		}
        

		if ((jv = jvalue_lookup_by_str(json->m_data, "pwd")) == NULL || jv->type != JT_S) {
			log_err(false, "no pwd\n");
			goto one_done;
		}
		
		for (i = 0; i < 16; i++) {
			tmp[0] = jv->value.str[i*2];
			tmp[1] = jv->value.str[i*2 + 1];
			pwd_md5[i] = (u_int8_t)strtoul(tmp, &endp, 16);
		}
        
		fmt_hex(pwd, pwd_md5, 16);
        
		dev = user_create(true, name, pwd,NULL, NULL, 0, user->callback, user->callback_handle, false, false);
		if (dev == NULL) {
			goto one_done;
		}
		//TODO: 添加后devtype 是0xff
		memcpy(dev->passwd_md5, pwd_md5, 16);
		fmt_hex(dev->passwd_md5_str, pwd_md5, 16);

		dev->parent = user;

		if ((jv = jvalue_lookup_by_str(json->m_data, "vendorid")) != NULL || jv->type == JT_S) {
			STR_REPLACE(dev->vendor_id, jv->value.str);
		}

		if ((jv = jvalue_lookup_by_str(json->m_data, "devtype")) != NULL || jv->type == JT_S) {
			if(dev->sub_type == 0 || dev->sub_type == 0xFF){
				dev->sub_type = atoi(jv->value.str);
                ajust_dev_sub_type_by_sn(dev->sn,&dev->sub_type,&dev->ext_type);
                map_test_dev_sub_type(&dev->sub_type,&dev->ext_type);
			}
		}

		log_debug("parse one done: name=%s, pwd=%s, dev_type=%d\n", name, dev->passwd_md5_str, dev->sub_type);

		// add to list
		stlc_list_add_tail(&dev->link, &user_list);
		
one_done:
		json_free(json);
	}

	// TODO: 这里仅仅做简单的处理，把老的删除，添加新的。以后要考虑替换动作
	stlc_list_for_each_entry_safe(user_t, dev, dev_next, &user->dev, link) {
		user_destroy(dev);
	}

	if ( ! stlc_list_empty(&user_list) ) {
		stlc_list_replace(&user_list, &user->dev);
	}

	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);

	return count;
}

RS pu_on_get_dict(cl_notify_pkt_t *pkt)
{
	int count;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	user_t *user;

	h = (cln_http_t *)pkt->data;

	log_debug("phone get dict: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	if ( ( ! user->pu_from_cache ) && h->result == RS_OK) {
		user->pu_from_cache = false;
	} else {
		if (pu_read_local_cache(user, h) == RS_OK) {
			user->pu_from_cache = true;
		}
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		
		if ((jv = jvalue_lookup_by_str(json->m_data, "kvlist")) != NULL) {
			count = pu_parse_dict_v2(user, jv);
			if (count == 0) {
				if (user->json != NULL) {
					json_free(user->json);
				}
				user->json = json;
				user->dict = json->m_data;
				json = NULL;
				
				pu_get_dev(user);
			} else {
				pu_get_devtype(user);
			}
		}

		if (json != NULL)
			json_free(json);

		if ( ! user->pu_from_cache )
			pu_write_local_cache(user);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////

RS pu_on_put_dict_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;

	h = (cln_http_t *)pkt->data;

	log_debug("phone put dict: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

RS pu_on_dict_add_dev_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	user_t *dev, *user = NULL;
	json_t *json = NULL;
	jvalue_t *jv;

	h = (cln_http_t *)pkt->data;

	log_debug("%s: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		__FUNCTION__, h->handle, h->result, h->url, h->page);
	
	
	cl_lock(&cl_priv->mutex);

	if ((dev = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found dev handle=0x%08x\n", h->handle);
		goto done;
	}
	if (h->page)
		json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			h->result = atoi(jv->value.str);
		}
		
		json_free(json);
	}
	if ((user = dev->parent) != NULL) {
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	}
	event_push_err(dev->callback, UE_PHONE_USER_ADD_DEV_FINISH, dev->handle, dev->callback_handle, h->result);
	if (h->result == 0) {
		if (user != NULL)
			pu_write_local_cache(user);
	} else {
		user_free(dev);
		if (user != NULL) {
			event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
		}
	}

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

RS pu_on_dict_del_dev_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	user_t *dev, *user;

	h = (cln_http_t *)pkt->data;

	log_debug("%s: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		__FUNCTION__, h->handle, h->result, h->url, h->page);
	
	cl_lock(&cl_priv->mutex);

	if ((dev = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found dev handle=0x%08x\n", h->handle);
		goto done;
	}

	if ((user = dev->parent) != NULL) {
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
	}
	event_push_err(dev->callback, UE_PHONE_USER_DEL_DEV_FINISH, dev->handle, dev->callback_handle, h->result);
	if (h->result == 0) {
		user_free(dev);
		pu_write_local_cache(user);
	}

done:	
	cl_unlock(&cl_priv->mutex);

	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

RS pu_on_dict_modify_passwd_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	user_t *user;
	int result = -1;

	h = (cln_http_t *)pkt->data;

	log_debug("pu_on_dict_modify_passwd_notify: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if ((result = atoi(jv->value.str)) != 0) {
				log_err(false, "user %s modify password failed, result=%s\n", user->name, jv->value.str);
			}
		}
		json_free(json);
	}
	
	if (result == 0) {
		user->modify_passwd_flags &= ~MPF_HTTP_DICT;
		if (user->modify_passwd_flags == 0) {
			event_push(user->callback, UE_MODIFY_PASSWD_OK, user->handle, user->callback_handle);
		}
		pu_write_local_cache(user->is_phone_user ? user : user->parent);
	} else {
		SAFE_FREE(user->new_passwd);
		event_push_err(user->callback, UE_MODIFY_PASSWD_FAIL, user->handle, user->callback_handle, result);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

RS pu_put_dict(user_t *user, int event, bool add)
{
	int dev_type = 0;
	char buf[64], key[128], url[256];
	char *value, *post_data;

	if (user->parent == NULL || ! user->parent->is_phone_user) {
		log_err(false, "pu_put_dict failed: user=%s, but parent=%p or not phone user\n", user->name, user->parent);
		return RS_ERROR;
	}

	value = cl_malloc(4000);
	post_data = cl_malloc(4000);

	if (user->sub_type != 0) {
		dev_type = user->sub_type;
	} else	if ( ! stlc_list_empty(&user->slave) ) {
		slave_t *slave;
		slave = stlc_list_first_entry(&user->slave, slave_t, link);
		dev_type = slave->sub_type;
	}

	log_err(false, "pu_put_dict user %s type = %d\n",user->name,dev_type);
	
	// 这里直接用用户输入的，可能是昵称，也可能是SN号
	http_sprintf(key, DEV_DICT_KEY_V2 "%s", user->name);
	// passwd, valid=0|1
	http_sprintf(value, "{\"nickname\":\"%s\",\"devtype\":\"%u\"," \
		"\"vendorid\":\"%s\",\"pwd\":\"%s\",\"valid\":\"%u\"}",
		user->nickname == NULL ? "" : user->nickname, 
		dev_type, user->vendor_id == NULL ? "" : user->vendor_id,
		(user->new_passwd == NULL ? user->passwd_md5_str : fmt_hex(buf, user->new_passwd_md5, 16)), 
		add);

	sprintf(post_data, "key=%s&vvid=%d&kkey=%s&kval=%s", 
		user->parent->pu_key, cl_priv->vvid, key, value);

	sprintf(url, "http://%s/cgi-bin/kvdict", DEFAULT_DOMAIN);
	http_post(url, post_data, event, user->handle);

	cl_free(value);
	cl_free(post_data);

	pu_write_local_cache(user->parent);

	return RS_OK;
}

void pu_save_dev_type(user_t *user)
{
	pu_put_dict(user, CLNE_HTTP_IGNORE, true);
}

void pu_upgrad_dict(user_t *user)
{
	user_t *dev;
	
	stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
		pu_put_dict(dev, CLNE_HTTP_IGNORE, true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void pu_parse_dict_v1(user_t *user, jvalue_t *dev_list)
{
	jvalue_t *node, *jv;
	char *name;
	user_t *dev;
	json_t *json;

	for (node = dev_list->value.list; node != NULL; node = node->next) {
		if (node->type != JT_O)
			continue;

		// parse name
		if ((jv = jvalue_lookup_by_str(node, "key")) == NULL || jv->type != JT_S) {
			log_err(false, "no key\n");
			continue;
		}
		if (strlen(jv->value.str) < sizeof(DEV_DICT_KEY_V1) || strncmp(jv->value.str, DEV_DICT_KEY_V1, sizeof(DEV_DICT_KEY_V1)-1) != 0) {
			log_debug(false, "ignore key=%s\n", jv->value.str);
			continue;
		}
		name = &jv->value.str[sizeof(DEV_DICT_KEY_V1) - 1];

		// parse other
		if ((jv = jvalue_lookup_by_str(node, "value")) == NULL || jv->type != JT_S) {
			log_err(false, "%s no value\n", name);
			continue;
		}
		if ((json = json_parse(jv->value.str, (int)strlen(jv->value.str))) == NULL) {
			continue;
		}
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif

		dev = user_lookup_by_name(user, name);
		if (dev != NULL) {
			if ((jv = jvalue_lookup_by_str(json->m_data, "vendorid")) != NULL || jv->type == JT_S) {
				STR_REPLACE(dev->vendor_id, jv->value.str);
			}
		}
		
		json_free(json);
	}
}

void pu_parse_devs(user_t *user, jvalue_t *dev_list)
{
	jvalue_t *node, *jv;
	int i;
	struct stlc_list_head user_list;
	user_t *dev, *dev_next;
	char tmp[3], *endp;
	char *sn, pwd[64];
	u_int64_t master_sn;
	int dev_type;
	bool online;
	u_int8_t pwd_md5[16];

	STLC_INIT_LIST_HEAD(&user_list);
	tmp[2] = '\0';
	
	for (node = dev_list->value.list; node != NULL; node = node->next) {
		if ((jv = jvalue_lookup_by_str(node, "sn")) == NULL) {
			log_err(false, "no sn\n");
			continue;
		}
		sn = jv->value.str;
		
		if ((jv = jvalue_lookup_by_str(node, "devtype")) == NULL) {
			log_err(false, "no devtype\n");
			continue;
		}
		dev_type = atoi(jv->value.str);

		if ((jv = jvalue_lookup_by_str(node, "mastersn")) == NULL) {
			log_err(false, "no mastersn\n");
			continue;
		}
		master_sn = atoi(jv->value.str);

		if ((jv = jvalue_lookup_by_str(node, "online")) == NULL) {
			log_err(false, "no online\n");
			continue;
		}
		online = ! ( ! atoi(jv->value.str) );

		if ((jv = jvalue_lookup_by_str(node, "pwd")) == NULL) {
			log_err(false, "no pwd\n");
			continue;
		}
		for (i = 0; i < 16; i++) {
			tmp[0] = jv->value.str[i*2];
			tmp[1] = jv->value.str[i*2 + 1];
			pwd_md5[i] = (u_int8_t)strtoul(tmp, &endp, 16);
		}
        
		fmt_hex(pwd, pwd_md5, 16);
        
		dev = user_create(true, sn, pwd, NULL, NULL, 0, user->callback, user->callback_handle, false, false);
		if (dev == NULL) {
			continue;
		}
		//TODO: 添加后devtype 是0xff
		memcpy(dev->passwd_md5, pwd_md5, 16);
		fmt_hex(dev->passwd_md5_str, pwd_md5, 16);
		if(dev->sub_type == 0 || dev->sub_type == 0xFF){
            
			dev->sub_type = dev_type;
            ajust_dev_sub_type_by_sn(dev->sn,&dev->sub_type,&dev->ext_type);
            map_test_dev_sub_type(&dev->sub_type,&dev->ext_type);
		}
		log_debug("pu_parse_devs sn %s subtype = %d\n",sn,dev->sub_type);

		dev->parent = user;

		// add to list
		stlc_list_add_tail(&dev->link, &user_list);
	}

	// TODO: 这里仅仅做简单的处理，把老的删除，添加新的。以后要考虑替换动作
	stlc_list_for_each_entry_safe(user_t, dev, dev_next, &user->dev, link) {
		user_destroy(dev);
	}

	if ( ! stlc_list_empty(&user_list) ) {
		stlc_list_replace(&user_list, &user->dev);
	}

	// 解析之前保存的更多信息
	pu_parse_dict_v1(user, user->dict);
	pu_upgrad_dict(user);

	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
}

RS pu_on_get_dev_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	user_t *user;

	h = (cln_http_t *)pkt->data;

	log_debug("phone get dev list: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER				
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if (atoi(jv->value.str) != 0) {
				log_err(false, "user %s get device list failed, result=%s\n", user->name, jv->value.str);
				goto err;
			}
		}
		if ((jv = jvalue_lookup_by_str(json->m_data, "devlist")) == NULL || jv->type != JT_A) {
			log_err(false, "user %s get devlist failed\n", user->name);
			goto err;
		}
		pu_parse_devs(user, jv);
		
		if ( ! user->pu_from_cache )
			pu_write_local_cache(user);
		
err:		
		json_free(json);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

/* GET: http://www.jiazhang007.com/cgi-bin/userdev?key=xxxx */
void pu_get_dev(user_t *user)
{
	char url[256];

	sprintf(url, "http://%s/cgi-bin/userdev?key=%s", DEFAULT_DOMAIN, user->pu_key);
	
	http_get(url, CLNE_HTTP_PU_GET_DEV, user->handle);
}

#ifdef CONFIG_SN_DICT
extern u_int8_t parse_sn_for_type(u_int64_t sn);
#endif

static void pu_parse_devs_devtype(user_t *user, jvalue_t *dev_list)
{
	jvalue_t *node, *jv;
	user_t *dev;
	char *sn;
	int dev_type;
	bool modify = false;
#ifdef CONFIG_SN_DICT	
	u_int8_t sub_type;
#endif

	for (node = dev_list->value.list; node != NULL; node = node->next) {
		if ((jv = jvalue_lookup_by_str(node, "sn")) == NULL) {
			log_err(false, "no sn\n");
			continue;
		}
		sn = jv->value.str;

		if ((dev = user_lookup_by_name(user, sn)) == NULL) {
			log_err(false, "not found dev sn=%s\n", sn);
			continue;
		}
		
		if ((jv = jvalue_lookup_by_str(node, "devtype")) == NULL) {
			log_err(false, "no devtype\n");
			continue;
		}
#ifdef CONFIG_SN_DICT		
		// 临时修改的
		if ((sub_type = parse_sn_for_type(atoll(sn))) != 0) {
			dev_type = sub_type;
		} else {
#endif
			dev_type = atoi(jv->value.str);
            
            

			if (dev_type != dev->sub_type) {
				log_info("dev %s sub_type modify from %u to %u\n", dev->name, dev->sub_type, dev_type);
				modify = true;
				if(dev->sub_type == 0 || dev->sub_type == 0xFF){
					dev->sub_type = dev_type;
				}
                ajust_dev_sub_type_by_sn(dev->sn,&dev->sub_type,&dev->ext_type);
                map_test_dev_sub_type(&dev->sub_type,&dev->ext_type);
				pu_save_dev_type(dev);
			}
#ifdef CONFIG_SN_DICT
		}
#endif		
		
	}

	if (modify)
		event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
}

RS pu_on_get_devtype_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	user_t *user;

	h = (cln_http_t *)pkt->data;

	log_debug("phone get dev list for devtype: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if (atoi(jv->value.str) != 0) {
				log_err(false, "user %s get device list failed, result=%s\n", user->name, jv->value.str);
				goto err;
			}
		}
		if ((jv = jvalue_lookup_by_str(json->m_data, "devlist")) == NULL || jv->type != JT_A) {
			log_err(false, "user %s get devlist failed\n", user->name);
			goto err;
		}
		pu_parse_devs_devtype(user, jv);
		
err:		
		json_free(json);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

void pu_get_devtype(user_t *user)
{
	char url[256];

	sprintf(url, "http://%s/cgi-bin/userdev?key=%s", DEFAULT_DOMAIN, user->pu_key);
	
	http_get(url, CLNE_HTTP_PU_GET_DEV_TYPE, user->handle);
}

RS pu_on_login_notify(cl_notify_pkt_t *pkt)
{
	int result;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	user_t *user;

	h = (cln_http_t *)pkt->data;

	log_debug("phone login result: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	if (h->result == RS_OK) {
		user->pu_from_cache = false;
	} else {
		pu_try_read_cache(user, pkt);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
		user->tcp_recv_pkts++;
		
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif		
		jv = jvalue_lookup_by_str(json->m_data, "result");
		if (jv != NULL) {
			result = atoi(jv->value.str);
			if (result == 0) {
				log_info("user %s login ok\n", user->name);
				user->online = true;

				// parse key
				jv = jvalue_lookup_by_str(json->m_data, "key");
				if (jv != NULL)
					pu_parse_key(user, jv->value.str);

				if (user->status == CS_ESTABLISH) {
					log_info("It just refresh key, not login.\n");
				} else {
					user_set_status(user, CS_ESTABLISH);
					pu_get_dict(user);
				}
			} else {
				jv = jvalue_lookup_by_str(json->m_data, "errordesc");
				if (jv != NULL) {
					if (strstr(jv->value.str, "wrong user pwd") != NULL || strstr(jv->value.str, "password is wrong") != NULL) {
						user->last_err = ULGE_BAD_PASSWORD;
					} else {
						user->last_err = ULGE_BAD_PHONE_NUM;
					}
					user_set_status(user, CS_LOGIN_ERR);
				}
			}
		}
		
		json_free(json);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

/* GET: http://www.jiazhang007.com/cgi-bin/login?id=x&pwd=x */
void pu_auth(user_t *user)
{
	char url[256];

	sprintf(url, "http://%s/cgi-bin/login?id=%s&pwd=%s&vvid=%d", DEFAULT_DOMAIN,
		user->name, user->pu_passwd, cl_priv->vvid);
	
	http_get(url, CLNE_HTTP_PU_LOGIN, user->handle);
}

/////////////

RS pu_on_add_dev_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;

	h = (cln_http_t *)pkt->data;

	log_debug("phone add device result: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

void pu_on_dev_login(user_t *dev, bool is_ok)
{
	char url[256];
	u_int8_t pwd[16];
	char pwd_str[128];
	
	log_info("pu_on_dev_login, dev=%s, is_ok=%d, last_err=%d\n", dev->name, is_ok, dev->last_err);
	
	dev->is_pud_authing = false;

	if (is_ok) {
		memcpy(pwd, dev->passwd_md5, 16);
		enc_block(pwd, AES128_EKY_LEN, dev->parent->pu_preshare_passwd);
		fmt_hex(pwd_str, pwd, 16);

		sprintf(url, "http://%s/cgi-bin/binddev?key=%s&devsn=%012"PRIu64"&devpwd=%s&bind=1",
			DEFAULT_DOMAIN, dev->parent->pu_key, dev->sn, pwd_str);
		
		http_get(url, CLNE_HTTP_PU_ADD_DEV, dev->handle);
	}
}

/////////////


RS pu_add_dev(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user, *dev;
	char *name, *passwd,*QR_code;

    QR_code = NULL;
	up = (cln_user_t *)pkt->data;
	name = up->data;
    
    if (pkt->type == CLNE_USER_ADD_QR_DEV) {
        passwd = "";
        QR_code = name+up->len_name;
    }else{
        passwd = name + up->len_name;
    }
	
	if (up->callback_handle != NULL) {
		*((cl_handle_t *)up->callback_handle) = INVALID_HANDLE;
	}

	if (cl_is_phone(name)) {
		log_err(false, "add device failed: it's phone number\n");
		return RS_INVALID_PARAM;
	}
	
	cl_lock(&cl_priv->mutex);
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "phone user 0x%08x add device failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if ( ! user->is_phone_user ) {
		log_err(false, "phone user 0x%08x add device failed: not phone user\n", up->user_handle);
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	if (user->status != CS_ESTABLISH) {
		log_err(false, "phone user 0x%08x add device failed: not login(status=%d)\n", up->user_handle, user->status);
		ret = RS_NOT_LOGIN;
		goto done;
	}

	// TODO: POST: http://www.jiazhang007.com/cgi-bin/kvdict?key=xx
	if (user_lookup_by_name(user, name) != NULL) {
		log_err(false, "phone user %s add %s failed: it's exist\n", user->name, name);
		ret = RS_EXIST;
		goto done;
	}
	
	dev = user_create(true, name, passwd, QR_code, NULL, 0, user->callback, user->callback_handle, true, false);
	if (dev == NULL) {
		ret = RS_EXIST;
		goto done;
	}

	if (up->callback_handle != NULL) {
		*((cl_handle_t *)up->callback_handle) = dev->handle;
	}

	dev->parent = user;
	stlc_list_add_tail(&dev->link, &user->dev);

	pu_put_dict(dev, CLNE_HTTP_PU_DICT_ADD_DEV, true);

	// 这里先不通知，等操作服务器完成后再通知
	//event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

//////////////////////


RS pu_on_del_dev_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;

	h = (cln_http_t *)pkt->data;

	log_debug("phone delete device result: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

RS pu_del_dev(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	user_t *user, *phone;
	char url[256];

	up = (cln_user_t *)pkt->data;

	log_debug("pu_del_dev\n");
	
	cl_lock(&cl_priv->mutex);
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, up->user_handle)) == NULL) {
		log_err(false, "phone user 0x%08x delete device failed: not found\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}
	
	if ( user->is_phone_user ) {
		log_err(false, "phone user 0x%08x delete device failed: it's phone user\n", up->user_handle);
		ret = RS_NOT_SUPPORT;
		goto done;
	}

	if ((phone = user->parent) == NULL) {
		log_err(false, "phone user 0x%08x delete device failed: not parent\n", up->user_handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	if (phone->status != CS_ESTABLISH) {
		log_err(false, "phone user 0x%08x add device failed: not login(status=%d)\n", up->user_handle, phone->status);
		ret = RS_NOT_LOGIN;
		goto done;
	}

	pu_put_dict(user, CLNE_HTTP_PU_DICT_DEL_DEV, false);

	sprintf(url, "http://%s/cgi-bin/binddev?key=%s&devsn=%s&devpwd=%s&bind=0",
		DEFAULT_DOMAIN, phone->pu_key, user->name, user->passwd_md5_str);
	
	http_get(url, CLNE_HTTP_PU_DEL_DEV, user->handle);

	// 这里先不删除用户，不发通知，等操作服务器字典OK了再操作
	//user_free(user);
	//event_push(phone->callback, UE_INFO_MODIFY, phone->handle, phone->callback_handle);

done:
	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

///////////////////////////////


RS pu_on_register_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	tmp_obj_t *to;
	json_t *json;
	jvalue_t *jv;
	int result = -1;
    int event = UE_PHONE_USER_REGISTER_FAILED;

	h = (cln_http_t *)pkt->data;

	log_debug("register phone user result: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	to = lookup_by_handle(HDLT_TMP, h->handle);

	if (to == NULL) {
		log_err(false, "not found tmp phone user object handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
                event = UE_PHONE_USER_REGISTER_OK;
            } else {
				log_err(false, "register phone user failed: result=%s\n", jv->value.str);
                if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
                    && jv->type == JT_S
                    && strstr(jv->value.str, "already") != NULL) {
                    event = UE_PHONE_USER_REGISTER_EXISTED;
                }
			}
		}
		
		json_free(json);
	}
	
	event_push(to->callback, event, 0, to->callback_handle);
	tmp_obj_free(to);

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

RS pu_register(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	char url[256];
	tmp_obj_t *to;

	up = (cln_user_t *)pkt->data;

	log_debug("pu_register\n");

	to = tmp_obj_alloc(0, NULL);
	to->callback = up->callback;
	to->callback_handle = up->callback_handle;
	
	sprintf(url, "http://%s/cgi-bin/register?id=%s&vvid=%u", DEFAULT_DOMAIN, up->data, cl_priv->vvid);
	
	http_get(url, CLNE_HTTP_PU_REGISTER, to->handle);
	
	return ret;
}

/////////////////////////////////////////////////

RS pu_on_reset_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	tmp_obj_t *to;
	json_t *json;
	jvalue_t *jv;
	int result = -1;

	h = (cln_http_t *)pkt->data;

	log_debug("reset phone user password result: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	to = lookup_by_handle(HDLT_TMP, h->handle);

	if (to == NULL) {
		log_err(false, "not found tmp phone user object handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if ((result = atoi(jv->value.str)) != 0) {
				log_err(false, "reset phone user password failed: result=%s\n", jv->value.str);
			}
		}
		
		json_free(json);
	}

	if (result == 0) {
		event_push(to->callback, UE_PHONE_USER_RESET_OK, 0, to->callback_handle);
	} else {
		event_push_err(to->callback, UE_PHONE_USER_RESET_FAILED, 0, to->callback_handle, result);
	}
	tmp_obj_free(to);

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

RS pu_reset(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	char url[256];
	tmp_obj_t *to;

	up = (cln_user_t *)pkt->data;

	log_debug("pu_reset\n");

	cl_lock(&cl_priv->mutex);
	
	to = tmp_obj_alloc(0, NULL);
	to->callback = up->callback;
	to->callback_handle = up->callback_handle;

	
	sprintf(url, "http://%s/cgi-bin/register?id=%s&reset=1&vvid=%u", DEFAULT_DOMAIN, up->data, cl_priv->vvid);
	
	http_get(url, CLNE_HTTP_PU_RESET_PASSWD, to->handle);

	cl_unlock(&cl_priv->mutex);
	return ret;
}

// http://www.jiazhang007.com/cgi-bin/register?id=x&pwd=x&random=x

RS pu_on_send_vcode_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	tmp_obj_t *to;
	json_t *json;
	jvalue_t *jv;
	int result = -1;

	h = (cln_http_t *)pkt->data;

	log_debug("send verify code result: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	to = lookup_by_handle(HDLT_TMP, h->handle);

	if (to == NULL) {
		log_err(false, "not found tmp phone user object handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if ((result = atoi(jv->value.str)) != 0) {
				log_err(false, "send verify code failed: result=%s\n", jv->value.str);
			}
		}
		
		json_free(json);
	}

	if (result == 0) {
		event_push(to->callback, UE_PHONE_USER_GOOD_VCODE, 0, to->callback_handle);
	} else {
		event_push_err(to->callback, UE_PHONE_USER_BAD_VCODE, 0, to->callback_handle, result);
	}
	tmp_obj_free(to);

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return RS_OK;
}

// 验证码, verification code
RS pu_send_vcode(cl_notify_pkt_t *pkt)
{
	RS ret = RS_OK;
	cln_user_t *up;
	char *phone, *passwd, *vcode;
	char pwd_enc[64];
	char url[256];
	tmp_obj_t *to;
	
	up = (cln_user_t *)pkt->data;
	phone = up->data;
	passwd = up->data + up->len_name;
	vcode = up->data + up->len_name + up->len_passwd;

	log_debug("pu_send_vcode\n");

	cl_lock(&cl_priv->mutex);

	to = tmp_obj_alloc(0, NULL);
	to->callback = up->callback;
	to->callback_handle = up->callback_handle;

	sprintf(url, "http://%s/cgi-bin/register?id=%s&pwd=%s&random=%s&vvid=%d", DEFAULT_DOMAIN,
		phone, pu_enc_passwd(passwd, pwd_enc), vcode, cl_priv->vvid);
	
	http_get(url, CLNE_HTTP_PU_SEND_VCODE, to->handle);

	cl_unlock(&cl_priv->mutex);
	
	return ret;
}

///////////////////////////////

RS pu_on_modify_passwd_notify(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	user_t *user;
	int result = -1;

	h = (cln_http_t *)pkt->data;

	log_debug("phone modify password notify: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	if ((user = lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "not found user handle=0x%08x\n", h->handle);
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL) {
			if ((result = atoi(jv->value.str)) != 0) {
				log_err(false, "user %s modify password failed, result=%s\n", user->name, jv->value.str);
			}
		}
		json_free(json);
	}
	
	if (result == 0) {
		user->modify_passwd_flags &= ~MPF_HTTP_DEV;
		if (user->is_phone_user) {
			memcpy(user->passwd_md5, user->new_passwd_md5, sizeof(user->passwd_md5));
			SAFE_FREE(user->passwd);
			user->passwd = user->new_passwd;
			user->new_passwd = NULL;
		}
		if (user->modify_passwd_flags == 0) {
			event_push(user->callback, UE_MODIFY_PASSWD_OK, user->handle, user->callback_handle);
		}

		pu_write_local_cache(user->is_phone_user ? user : user->parent);
	} else {
		SAFE_FREE(user->new_passwd);
		event_push_err(user->callback, UE_MODIFY_PASSWD_FAIL, user->handle, user->callback_handle, result);
	}

done:
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	
	return RS_OK;
}

void pu_modify_user_pwd(user_t *user, char *pwd)
{
	char pwd_enc[64];
	char url[256];

	log_debug("pu_modify_user_pwd %s\n", user->name);
	
	sprintf(url, "http://%s/cgi-bin/userinfo?key=%s&name=%s&pwd=%s",DEFAULT_DOMAIN,
		user->pu_key, user->name, pu_enc_passwd(pwd, pwd_enc));
	
	http_get(url, CLNE_HTTP_PU_MODIFY_PASSWD, user->handle);
}

void pu_modify_dev_pwd(user_t *user, char *pwd)
{
	user_t *phone;
	char url[256], pwd_enc[64];

	phone = user->parent;
	
	sprintf(url, "http://%s/cgi-bin/binddev?key=%s&devsn=%012"PRIu64"&devpwd=%s&bind=1", DEFAULT_DOMAIN,
		phone->pu_key, user->sn, pu_enc_passwd(pwd, pwd_enc));
	
	http_get(url, CLNE_HTTP_PU_MODIFY_PASSWD, user->handle);

	pu_put_dict(user, CLNE_HTTP_PU_PUT_DICT_MODIFY_PASSWD, true);
}

#define SHARE_PWD_LEN 16

static void hex_to_asicc(unsigned char *str, int len, unsigned char *out)
{
	int index = 0;
	int i;
	for(i = 0; i < len; i++){
		index = index + sprintf((char*)&out[index], "%02x", str[i]);
	}
}

#pragma pack(push,1)
typedef struct {
	u_int64_t user_id;
	u_int32_t session_id;
	u_int32_t expire_time;
} key_st;
#pragma pack(pop)

void build_udp_user_http_key(user_t *user)
{
	char key[32];
	MD5_CTX mdctx;
	char tmp, prev;
	key_st skey;
	int i;

	strcpy(key, "&d5wgfkj8%@#2gu6");
	prev = key[SHARE_PWD_LEN-1];
	
	for (i = 0; i < SHARE_PWD_LEN; i++) {
	   tmp = key[i];
	   key[i] = prev;
	   prev = tmp;
	}

	MD5Init(&mdctx);
	MD5Update(&mdctx, (u_int8_t*)key, SHARE_PWD_LEN);
	MD5Final((u_int8_t*)key, &mdctx); 

	skey.expire_time = (u_int32_t)time(0) + 3600;
	skey.session_id = skey.expire_time;
	skey.user_id = ntoh_ll(user->sn);
	skey.expire_time = ntohl(skey.expire_time);
	skey.session_id = ntohl(skey.session_id);

	enc_block((unsigned char*)&skey, sizeof(skey), key);
	hex_to_asicc((unsigned char*)&skey, sizeof(skey), user->pu_key);
	
}

void build_udp_sn_http_key(u_int64_t sn, char pu_key[64])
{
	char key[32];
	MD5_CTX mdctx;
	char tmp, prev;
	key_st skey;
	int i;

	strcpy(key, "&d5wgfkj8%@#2gu6");
	prev = key[SHARE_PWD_LEN-1];
	
	for (i = 0; i < SHARE_PWD_LEN; i++) {
	   tmp = key[i];
	   key[i] = prev;
	   prev = tmp;
	}

	MD5Init(&mdctx);
	MD5Update(&mdctx, (u_int8_t*)key, SHARE_PWD_LEN);
	MD5Final((u_int8_t*)key, &mdctx); 

	skey.expire_time = (u_int32_t)time(0) + 3600;
	skey.session_id = skey.expire_time;
	skey.user_id = ntoh_ll(sn);
	skey.expire_time = ntohl(skey.expire_time);
	skey.session_id = ntohl(skey.session_id);

	enc_block((unsigned char*)&skey, sizeof(skey), key);
	hex_to_asicc((unsigned char*)&skey, sizeof(skey), pu_key);
	
}


#ifndef MISC_CLIENT_USER
RS pu_apns_config(user_t *user, cl_notify_pkt_t *pkt)
{
	char url[1024];
	char prefix[512];
	unsigned char token[128];
	int i,n;
	cln_apns_config_t *up;
	smart_appliance_ctrl_t* sac;
	smart_air_ctrl_t* air_ctrl;
	u_int32_t nip = 0;
	char language[64];
	char push_music[256];
	char *dn = NULL;
	ucla_session_t *s;

	log_debug("pu_apns_config...\n");

	if (user->apns_cfg == NULL) {
		user->apns_cfg = cl_calloc(sizeof(cl_apns_config_t), 1);
	}
	if (user->apns_cfg == NULL) {
		return RS_MEMORY_MALLOC_FAIL;
	}

	//  非直连，直接取得服务器IP
	if (user->devserver_ip && !user->direct_ip) {
		nip = user->devserver_ip;
		log_debug("use user's devserver_ip");
	} else if ((sac = user->smart_appliance_ctrl) != NULL && (air_ctrl = (smart_air_ctrl_t* )sac->sub_ctrl)) {
		if (air_ctrl->stat.server_ip) {
			nip = air_ctrl->stat.server_ip;
			log_debug("use stat's server_ip\n");
		}
	}

	// 还没获取到设备连接的服务器
	if (nip == 0) {
		log_info("not get dev 's server's ip\n");
		dn = DFL_CGI_SERVER;
	} else {
		dn = cl_lookup_dn_by_ip(nip);
		if (!dn) {
			log_err(false, "try get ip 0x%x to dns failed\n", nip);
//			return RS_EMPTY;
		}

		log_info("get ip 0x%x 's url [%s]\n", nip, dn);
	}


	//dn = "10.4.0.33";
	

	// 备份原来的订阅音乐和语音
	memcpy(language, user->apns_cfg->language, sizeof(language));
	memcpy(push_music, user->apns_cfg->push_music, sizeof(push_music));
	
	up = (cln_apns_config_t *)&pkt->data[0];
	memcpy(user->apns_cfg, &up->cfg, sizeof(cl_apns_config_t));
	
	for(n = 0, i = 0; i < up->cfg.token_len; i++){
		n = n + sprintf((char*)(&token[n]), "%02x", (u_int8_t)up->cfg.token[i]);
	}
	if (up->cfg.msg_prefix[0])
		http_sprintf(prefix, "%s", up->cfg.msg_prefix);
	else
		prefix[0] = 0;	

	if(user->pu_key[0] == 0 && user->is_udp_ctrl){
		build_udp_user_http_key(user);
	}

#if 0
	if (up->cfg.regid[0]) {
		// 小米推送
		n = sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&token=%s&action=%d&need=%d&iver=%s&prefix=%s&mipush=%s",
					DFL_CGI_SERVER, user->pu_key, up->cfg.regid, up->cfg.action, up->cfg.need_push, up->cfg.phone_ver, prefix, up->cfg.mipush_packname);
		goto done;
	}
#endif	
    if (dn == NULL) {
		if (up->cfg.regid[0] == 0) {
	        n = sprintf(url, "http://%u.%u.%u.%u:880/cgi-bin/apnsconfig?key=%s&cid=%d&token=%s&action=%d&need=%d&iver=%s&prefix=%s",
	                    IP_SHOW(nip), user->pu_key, up->cfg.cert_id, token, up->cfg.action, up->cfg.need_push, up->cfg.phone_ver, prefix);
		} else {			
			// 小米推送
	        n = sprintf(url, "http://%u.%u.%u.%u:880/cgi-bin/apnsconfig?key=%s&token=%s&action=%d&need=%d&iver=%s&prefix=%s&mipush=%s",
	                    IP_SHOW(nip), user->pu_key, up->cfg.regid, up->cfg.action, up->cfg.need_push, up->cfg.phone_ver, prefix, up->cfg.mipush_packname);
		}
    }else{
	    if (up->cfg.regid[0] == 0) {
	        n = sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&cid=%d&token=%s&action=%d&need=%d&iver=%s&prefix=%s",
	                    dn, user->pu_key, up->cfg.cert_id, token, up->cfg.action, up->cfg.need_push, up->cfg.phone_ver, prefix);
	    } else {	    	
	    	// 小米推送
	        n = sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&token=%s&action=%d&need=%d&iver=%s&prefix=%s&mipush=%s",
	                    dn, user->pu_key, up->cfg.regid, up->cfg.action, up->cfg.need_push, up->cfg.phone_ver, prefix, up->cfg.mipush_packname);
	    }
    }
	
//done:
	if (strlen(up->cfg.language)) {
		n += sprintf(&url[n], "&language=%s", up->cfg.language);
	} else {
		memcpy(user->apns_cfg->language, language, sizeof(language));
	}

	if (strlen(up->cfg.push_music)) {
		n += sprintf(&url[n], "&push_music=%s", up->cfg.push_music);
	} else {
		memcpy(user->apns_cfg->push_music, push_music, sizeof(push_music));
	}

	// 联动用户ID也要填上
	if (la_is_valid()) {
		if ((s = ucla_get_any_enstablis_session()) != NULL) {
			n += sprintf(&url[n], "&user_id=%u", s->user_id);
		}
	}

	http_get(url, CLNE_HTTP_ANPS_CONFIG, user->handle);

	log_debug("pu_apns_config url \n%s\n", url);

	return RS_OK;
}
#else
void pu_apns_cb(cl_handle_t handle, u_int16_t type, u_int16_t len, u_int8_t *data)
{
	int event = UE_APNS_CONFIG_FAIL;
	misc_apns_reply_t *reply = (misc_apns_reply_t *)data;
	user_t *user;

	log_debug("pu apns callback: type %u len %u\n", type, len);

	if (type != MCT_APNS) {
		return;
	}

	if (len < sizeof(*reply)) {
		return;
	}

	reply->sn = ntoh_ll(reply->sn);

	user = user_lookup_by_sn(reply->sn);
	if (!user) {
		log_err(false, "not found user %"PRIu64, reply->sn);
		return;
	}

	if (user->apns_cfg == NULL) {
		return;
	}

	if (reply->result) {
		goto done;
	}

	event = UE_APNS_CONFIG;

	user->apns_cfg->need_push = reply->need;
	
	if (reply->language[0]) {
		memcpy(user->apns_cfg->language, reply->language, sizeof(user->apns_cfg->language));
	}
	if (reply->push_music) {
		memcpy(user->apns_cfg->push_music, reply->push_music, sizeof(user->apns_cfg->push_music));
	}

done:
	event_push(user->callback, event, user->handle, user->callback_handle);
}

RS pu_apns_config(user_t *user, cl_notify_pkt_t *pkt)
{
	int i,n;
	cln_apns_config_t *up;
	u_int32_t nip = 0;
	char language[64];
	char push_music[256];
	ucla_session_t *s;
	misc_apns_request_t request = {0};

	log_debug("pu_apns_config...\n");

	if (user->apns_cfg == NULL) {
		user->apns_cfg = cl_calloc(sizeof(cl_apns_config_t), 1);
	}
	if (user->apns_cfg == NULL) {
		return RS_MEMORY_MALLOC_FAIL;
	}

	nip = user_select_ip(user);
	if (nip == 0) {
		return RS_NOT_INIT;
	}
	//nip = ntohl(inet_addr("10.4.0.33"));

	// 备份原来的订阅音乐和语音
	memcpy(language, user->apns_cfg->language, sizeof(language));
	memcpy(push_music, user->apns_cfg->push_music, sizeof(push_music));
	
	up = (cln_apns_config_t *)&pkt->data[0];
	memcpy(user->apns_cfg, &up->cfg, sizeof(cl_apns_config_t));

	request.sn = ntoh_ll(user->sn);
	request.action = up->cfg.action;
	request.need = up->cfg.need_push;
	request.cid = ntohs((u_int16_t)up->cfg.cert_id);
	memcpy(request.iver, up->cfg.phone_ver, 8);

	if (up->cfg.language[0] == 0) {
		memcpy(request.language, language, sizeof(request.language));
	}
	memcpy(request.language, up->cfg.language, sizeof(request.language));

	if (up->cfg.push_music[0] == 0) {
		memcpy(request.push_music, push_music, sizeof(request.push_music));
	}
	memcpy(request.push_music, up->cfg.push_music, sizeof(request.push_music));
	
	for(n = 0, i = 0; i < up->cfg.token_len; i++){
		n = n + sprintf((char*)(&request.token[n]), "%02x", (u_int8_t)up->cfg.token[i]);
	}
	
	if (up->cfg.msg_prefix[0])
		http_sprintf(request.prefix, "%s", up->cfg.msg_prefix);
	else
		request.prefix[0] = 0;	

	// 联动用户ID也要填上
	if (la_is_valid()) {
		s = ucla_get_any_enstablis_session();
		if (s) {
			request.user_id = ntohl(s->user_id);
		}
	}
	
	misc_client_do_request(user->handle, MCT_APNS, sizeof(request), (u_int8_t*)&request, nip, pu_apns_cb);

	log_debug("pu_apns_config url \n%s\n", url);

	return RS_OK;
}
#endif

/**
	根据服务器IP查找这个服务器的同步状态
*/
static apn_server_stat_t *pu_syn_get_server_stat_by_ip(user_t *user, u_int32_t ip)
{
	u_int32_t i;
	cl_apn_syn_t *syn = &user->apns_syn;

	for (i = 0; i < syn->server_num; i++) {
		if (syn->stat[i].ip == ip) {
			return &syn->stat[i];
		}
	}

	return NULL;
}

/**
	进行了订阅同步全球服务器，还没收到任意一个回复
*/
static bool pu_syn_server_stat_had_no_reply(user_t *user)
{
	u_int32_t i;
	cl_apn_syn_t *syn = &user->apns_syn;

	for (i = 0; i < syn->server_num; i++) {
		if (syn->stat[i].valid == true) {
			return false;
		}
	}

	return true;
}

/**
	指定IP地址，去发送配置信息
*/
static RS pu_apns_config_by_ip(user_t *user, u_int32_t ip)
{
	char url[1024];
	char prefix[512];
	unsigned char token[128];
	int i,n;
	cl_apns_config_t *cfg = user->apns_cfg;
	u_int32_t nip;
	u_int8_t *u8ptr = (u_int8_t *)&nip;
	char *dn = NULL;
	ucla_session_t *s = NULL;

	dn = cl_lookup_dn_by_ip(ip);
	if (!dn) {
		log_err(false, "pu_apns_config_by_ip 0x%x failed\n", ip);
//		return RS_ERROR;
	}
	
	for(n = 0, i = 0; i < cfg->token_len; i++){
		n = n + sprintf((char*)(&token[n]), "%02x", (u_int8_t)cfg->token[i]);
	}
	if (cfg->msg_prefix[0])
		http_sprintf(prefix, "%s", cfg->msg_prefix);
	else
		prefix[0] = 0;	

	if(user->pu_key[0] == 0 && user->is_udp_ctrl){
		build_udp_user_http_key(user);
	}

#if 0
	if (cfg->cfg.regid[0]) {
		// 小米推送
		n = sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&token=%s&action=%d&need=%d&iver=%s&prefix=%s&mipush=%s",
					DFL_CGI_SERVER, user->pu_key, cfg->cfg.regid, cfg->cfg.action, cfg->cfg.need_push, cfg->cfg.phone_ver, prefix, cfg->cfg.mipush_packname);
		goto done;
	}
#endif
    if (!dn) {
		if (cfg->regid[0] == 0) {
	        n = sprintf(url, "http://%u.%u.%u.%u:880/cgi-bin/apnsconfig?key=%s&cid=%d&token=%s&action=%d&need=%d&iver=%s&prefix=%s",
	                    IP_SHOW(ip), user->pu_key, cfg->cert_id, token, cfg->action, cfg->need_push, cfg->phone_ver, prefix);
		} else {
			// 小米推送
	        n = sprintf(url, "http://%u.%u.%u.%u:880/cgi-bin/apnsconfig?key=%s&token=%s&action=%d&need=%d&iver=%s&prefix=%s&mipush=%s",
	                    IP_SHOW(nip), user->pu_key, cfg->regid, cfg->action, cfg->need_push, cfg->phone_ver, prefix, cfg->mipush_packname);
	
		}
    }else{
    	if (cfg->regid[0] == 0) {
	        n = sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&cid=%d&token=%s&action=%d&need=%d&iver=%s&prefix=%s",
	                    dn, user->pu_key, cfg->cert_id, token, cfg->action, cfg->need_push, cfg->phone_ver, prefix);
    	} else {
    			    	// 小米推送
	        n = sprintf(url, "http://%s:880/cgi-bin/apnsconfig?key=%s&token=%s&action=%d&need=%d&iver=%s&prefix=%s&mipush=%s",
	                    dn, user->pu_key, cfg->regid, cfg->action, cfg->need_push, cfg->phone_ver, prefix, cfg->mipush_packname);
    	}
    }
	
//done:
	if (strlen(cfg->language)) {
		n += sprintf(&url[n], "&language=%s", cfg->language);
	}

	if (strlen(cfg->push_music)) {
		n += sprintf(&url[n], "&push_music=%s", cfg->push_music);
	}

	// 联动用户ID也要填上
	if (la_is_valid()) {
		if ((s = ucla_get_any_enstablis_session()) != NULL) {
			n += sprintf(&url[n], "&user_id=%u", s->user_id);
		}
	}
	
	http_get(url, CLNE_HTTP_ANPS_CONFIG_V2, user->handle);

	log_debug("syn apns by id: request apns to url\n %s\n", url);
	
	return RS_OK;
}

static int pu_syn_timer(cl_thread_t *t)
{
	u_int32_t i;
	cl_handle_t *handle = CL_THREAD_ARG(t);
	user_t *user;
	cl_apn_syn_t *syn;

	user = lookup_by_handle(HDLT_USER, *handle);
	if (!user) {
		log_debug("not found syn timer's user handle 0x%x\n", (u_int32_t)(*handle));
		goto err_out;
	}

	cl_lock(&cl_priv->mutex);

	syn = &user->apns_syn;

	syn->t_syn = NULL;

	if (++syn->try_count > 2) {
		log_debug("apns syn try count > 2, exit\n");
		goto err_out;
	}

	// 建立N个线程去向每个服务器请求订阅信息
	for (i = 0; i < syn->server_num; i++) {
		if (syn->stat[i].valid == true) {
			continue;
		}
		
		pu_apns_config_by_ip(user, syn->stat[i].ip);
	}

	// 启动一个超时定时器
	CL_THREAD_TIMER_ON(&cl_priv->master, syn->t_syn, pu_syn_timer, handle, TIME_N_SECOND(10));

	cl_unlock(&cl_priv->mutex);
	
	return 0;

err_out:
	if (handle) {
		cl_free(handle);
	}
	return -1;
}

/**
	订阅收到了成功在当前连接的服务器订阅成功的消息
	同步订阅信息到全球服务器
*/
static void pu_try_syn_apns(user_t *user, bool need_syn)
{
	cl_apns_config_t *apns_cfg = user->apns_cfg;
	cl_apn_syn_t *syn = &user->apns_syn;
	u_int32_t i;
	// 因为user可能被删除，所以用handle保险
	cl_handle_t *handle;

	if (apns_cfg == NULL) {
		log_debug("syn apns:apns_cfg is null\n");
		return;
	}

	log_debug("syn apns: try syn apns cfg to global servers\n");

	handle = cl_calloc(1, sizeof(*handle));

	if (!handle) {
		log_debug("syn apns:handle 0x%08x not found\n", *handle);
		return;
	}

	*handle = user->handle;

	cl_lock(&cl_priv->mutex);

	CL_THREAD_TIMER_OFF(syn->t_syn);

	memset(syn, 0x00, sizeof(*syn));

	syn->need_syn = need_syn;

	syn->server_num = cl_priv->num_ip_disp_server;
	
	for (i = 0; i < cl_priv->num_ip_disp_server; i++) {
		syn->stat[i].ip = cl_priv->ip_disp_server[i];
	}

	// 建立N个线程去向每个服务器请求订阅信息
	for (i = 0; i < syn->server_num; i++) {
		pu_apns_config_by_ip(user, syn->stat[i].ip);
	}

	// 启动一个超时定时器，暂时10秒超时
	//CL_THREAD_TIMER_ON(&cl_priv->master, syn->t_syn, pu_syn_timer, handle, TIME_N_SECOND(10));

	cl_unlock(&cl_priv->mutex);
}

//
RS pu_on_apns_config_notify_v2(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	int result = -1;
	RS ret = RS_OK;
	u_int32_t ip = 0;
//	u_int8_t ips[4], *u8ptr;
//	apn_server_stat_t *stat;
	char need_push = -1;
	char language[64] = {0}, push_music[256] = {0};
	u_int32_t event = UE_APNS_CONFIG_FAIL;
	//bool first_reply = false;


	h = (cln_http_t *)pkt->data;

	log_debug("syn apns:phone user apns config answer v2: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
				if ((jv = jvalue_lookup_by_str(json->m_data, "need")) != NULL && jv->type == JT_S){
					need_push = atoi(jv->value.str);
				}
				
				if ((jv = jvalue_lookup_by_str(json->m_data, "language")) != NULL && jv->type == JT_S){
					strncpy(language, jv->value.str, sizeof(user->apns_cfg->language));
				}

				if ((jv = jvalue_lookup_by_str(json->m_data, "push_music")) != NULL && jv->type == JT_S){
					strncpy(push_music, jv->value.str, sizeof(user->apns_cfg->push_music));
				}
			} else {
				log_debug("register phone user failed: result=%s\n", jv->value.str);
				if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
					&& jv->type == JT_S) {
					log_debug("%s result=%d, errordesc=%s\n", __FUNCTION__,  result, jv->value.str);
				}
			}
		}
		json_free(json);
	}

#if 0
	first_reply = pu_syn_server_stat_had_no_reply(user);

	if (result == 0) {
		// 满足2个条件，手机直连，并且还没收到任何一个全球服务器的应答
		if (user->apns_syn.need_syn && first_reply) {
			log_debug("is lan direct, and first reply update need_push to %d\n", need_push);
			
			if (need_push >= 0) {
				user->apns_cfg->need_push = need_push;
			}

			if (strlen(language)) {
				memcpy(user->apns_cfg->language, language, sizeof(user->apns_cfg->language));
			}

			if (strlen(push_music)) {
				memcpy(user->apns_cfg->push_music, push_music, sizeof(user->apns_cfg->push_music));
			}

			// 这里通知订阅成功
			event = UE_APNS_CONFIG;
		}
		
		// 山寨版inet_aton
		if (sscanf(h->url, "http://%u.%u.%u.%u", &ips[0], &ips[1], &ips[2], &ips[3]) != 4) {
			log_debug("syn apns:parse url %s to ips failed\n");
			goto done;
		}

		u8ptr = (u_int8_t*)&ip;
		u8ptr[0] = ips[0];
		u8ptr[1] = ips[1];
		u8ptr[2] = ips[2];
		u8ptr[3] = ips[3];

		// 本地序保存的IP
		ip = ntohl(ip);
		
		stat = pu_syn_get_server_stat_by_ip(user, ip);
		if (stat) {
			stat->valid = true;
			log_debug("syn apns:apns update server %s valid true\n", h->url);
		}
	}

	log_debug("direct_ip %u first_reply %u event %u\n", user->direct_ip, first_reply, event);
	
	// 在SDK直连情况下，从全球服务器收到的一个回复里面获取信息作为结果反馈给上层
	if (user->apns_syn.need_syn && first_reply) {
		event_push(user->callback, event, user->handle, user->callback_handle);
	}
#endif	
done:	
	cl_unlock(&cl_priv->mutex);


	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return ret;
}


RS pu_on_apns_config_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	int result = -1;
	int event = UE_APNS_CONFIG_FAIL;
	RS ret = RS_OK;

	h = (cln_http_t *)pkt->data;

	log_debug("phone user apns config answer: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
	
	
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_debug("%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
				event = UE_APNS_CONFIG;
				if ((jv = jvalue_lookup_by_str(json->m_data, "need")) != NULL && jv->type == JT_S){
					user->apns_cfg->need_push = atoi(jv->value.str);
				}
				// 新增语言和推送音乐
				if ((jv = jvalue_lookup_by_str(json->m_data, "language")) != NULL && jv->type == JT_S){
					strncpy(user->apns_cfg->language, jv->value.str, sizeof(user->apns_cfg->language));
				}

				if ((jv = jvalue_lookup_by_str(json->m_data, "push_music")) != NULL && jv->type == JT_S){
					strncpy(user->apns_cfg->push_music, jv->value.str, sizeof(user->apns_cfg->push_music));
				}
			} else {
				log_debug("register phone user failed: result=%s\n", jv->value.str);
				if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
					&& jv->type == JT_S) {
					log_debug("%s result=%d, errordesc=%s\n", __FUNCTION__,  result, jv->value.str);
				}
			}
		}
		json_free(json);
	}

	event_push(user->callback, event, user->handle, user->callback_handle);

	// 当前服务器订阅成功，想其他服务器推送
	if (event == UE_APNS_CONFIG) {
		log_debug("apns sucess, now try syn to global \n");
		pu_try_syn_apns(user, false);
	}

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return ret;
}

/**
	通过SN来进行HTTP订阅以后，把结果通知回来
*/
RS pu_on_apns_config_notify_sn(cl_notify_pkt_t *pkt)
{
	cln_http_t *h;
	json_t *json;
	jvalue_t *jv;
	int result = -1;
	int event = UE_APNS_CONFIG_BY_SN_FAILED;
	RS ret = RS_OK;
	lan_dev_probe_ctrl_t* ldpc = cl_priv->ldpc;

	h = (cln_http_t *)pkt->data;

	log_debug("phone user apns config answer by sn: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);	


	json = json_parse(h->page, (int)strlen(h->page));
	if (json != NULL) {
#ifdef	DBG_PHONE_USER		
		json_dump(json);
#endif
		if ((jv = jvalue_lookup_by_str(json->m_data, "result")) != NULL && jv->type == JT_S) {
			if ((result = atoi(jv->value.str)) == 0) {
				event = UE_APNS_CONFIG_BY_SN_OK;

			} else {
				log_debug("register phone user failed: result=%s\n", jv->value.str);
				if ((jv = jvalue_lookup_by_str(json->m_data, "errordesc")) != NULL
					&& jv->type == JT_S) {
					log_debug("%s result=%d, errordesc=%s\n", __FUNCTION__,  result, jv->value.str);
				}
			}
		}
		json_free(json);
	}

	if (ldpc && ldpc->callback) {
		event_push(ldpc->callback, event, h->handle, ldpc->callback_handle);
	}
	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);

	return ret;
}


static int timer_pu_msg_query(cl_thread_t *t)
{
	user_t *user = (user_t *)CL_THREAD_ARG(t);
	char url[512];

	user->t_timer_pu_msg = NULL;
	if (cl_priv->cleint_type != CID_IOS)
		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_pu_msg, timer_pu_msg_query, (void *)user, TIME_PU_MSG);
	
	sprintf(url, "http://%s/cgi-bin/icepush?key=%s&id=%"PRIu64"&accept=20", DEFAULT_DOMAIN,
		user->pu_key, user->pu_msg_expect_id);
	http_get(url, CLNE_HTTP_MSG_PUSH, user->handle);
	return 0;
}

RS pu_set_notify_expect_id(user_t *user, cln_notify_push_t *param)
{
	user->pu_msg_expect_id = param->expect_report_id;
	CL_THREAD_TIMER_OFF(user->t_timer_pu_msg);
	CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_pu_msg, timer_pu_msg_query, (void *)user, 0);		
	return RS_OK;
}

static RS parse_msg_push_list(user_t *user, jvalue_t *root)
{
	jvalue_t *node, *jv;
	notify_msg_t *msg = NULL;
	user_cmt_event_t *evt = NULL;
	RS ret;
			
	for (node = root->value.list; node != NULL; node = node->next) {
		if ( msg == NULL)
			msg = cl_calloc(sizeof(*msg), 1);
		if (msg == NULL)
			break;
		
		if (node->type != JT_O) {
			continue;
		}

		msg->msg_format = FMT_URL;
		if ((jv = jvalue_lookup_by_str(node, "id")) == NULL || jv->type != JT_S) {
			log_err(false, "%s not found id or bad type\n",__FUNCTION__);
			continue;;
		}
		sscanf(jv->value.str, "%"PRIu64"", &msg->report_id);
		if(msg->report_id >= user->pu_msg_expect_id)
			user->pu_msg_expect_id = msg->report_id + 1;
		
		if ((jv = jvalue_lookup_by_str(node, "title")) == NULL || jv->type != JT_S) {
			log_err(false, "%s not found content or bad type\n",__FUNCTION__);
			continue;;
		}
		msg->content = cl_strdup(jv->value.str);

		if ((jv = jvalue_lookup_by_str(node, "type")) == NULL || jv->type != JT_S) {
			log_err(false, "%s not found type or bad type\n",__FUNCTION__);
			continue;;
		}
		msg->msg_type = atoi(jv->value.str);

		if ((jv = jvalue_lookup_by_str(node, "format")) == NULL || jv->type != JT_S) {
			log_err(false, "%s not found format or bad type\n",__FUNCTION__);
			continue;;
		}
		msg->msg_format = (u_int8_t)atoi(jv->value.str);

		if ((jv = jvalue_lookup_by_str(node, "time")) == NULL || jv->type != JT_S) {
			log_err(false, "%s not found time or bad type\n",__FUNCTION__);
			continue;;
		}
		msg->msg_time = atoi(jv->value.str);

		if ((jv = jvalue_lookup_by_str(node, "context")) == NULL || jv->type != JT_S) {
			log_err(false, "%s not found context or bad type\n",__FUNCTION__);
			continue;;
		}
		msg->msg = cl_strdup(jv->value.str);
		if (msg->msg) {
			msg->msg_len = (u_int32_t)strlen(msg->msg);
		}
		evt = cl_malloc(sizeof(*evt));
		if (evt == NULL)
			break;
	
		evt->msg = (void*)msg;
		evt->event = NE_NOTIFY;
		stlc_list_add_tail(&evt->link, &user->user_cmt->notify_head);

		if (user->np && user->np->callback)
			ret = event_push(user->np->callback, evt->event, user->handle, user->np->callback_handle);
		else
			ret = event_push(user->callback, evt->event, user->handle, user->callback_handle);			
		if (ret != RS_OK) {
			stlc_list_del(&evt->link);
			break;
		}
		evt = NULL;
		msg = NULL;
	}
	
	SAFE_FREE(msg);
	SAFE_FREE(evt);

	return RS_OK;
}

RS pu_on_msg_push_notify(cl_notify_pkt_t *pkt)
{
	user_t *user;
	cln_http_t *h;
	json_t *json = NULL;
	jvalue_t *jv;
//	notify_msg_t *up_msg = NULL;
//	user_cmt_event_t *evt = NULL;
	int result = -1;
	int count = 0, finished = 1;
	RS ret = RS_OK;

	h = (cln_http_t *)pkt->data;

	log_debug("phone user msg push answer: handle=0x%08x, result=%d, url=\n%s\n, page=\n%s\n",
		h->handle, h->result, h->url, h->page);

	cl_lock(&cl_priv->mutex);
		
	if ((user = (user_t *)lookup_by_handle(HDLT_USER, h->handle)) == NULL) {
		log_err(false, "%s 0x%08x failed: not found\n", __FUNCTION__, h->handle);
		ret = RS_NOT_FOUND;
		goto done;
	}

	json = json_parse(h->page, (int)strlen(h->page));
	if (json == NULL)
		goto done;
	
#ifdef	DBG_PHONE_USER		
	json_dump(json);
#endif
	jv = jvalue_lookup_by_str(json->m_data, "result");
	if (jv  == NULL || jv->type != JT_S)
		goto done;
	result = atoi(jv->value.str);
	if (result != 0) {
		goto done;
	}

	jv = jvalue_lookup_by_str(json->m_data, "count");
	if (jv == NULL || jv->type != JT_S)
		goto done;
	count = atoi(jv->value.str);
	if (count == 0) {
		goto done;
	}

	jv = jvalue_lookup_by_str(json->m_data, "finished");
	if (jv && jv->type == JT_S)
		finished = atoi(jv->value.str);
	jv = jvalue_lookup_by_str(json->m_data, "pushlist");
	if (jv == NULL)
		goto done;
	
	result = parse_msg_push_list(user, jv);
	
	json_free(json);
	json = NULL;
	if(!finished){
		CL_THREAD_TIMER_OFF(user->t_timer_pu_msg);
		CL_THREAD_TIMER_ON(&cl_priv->master, user->t_timer_pu_msg, timer_pu_msg_query, (void *)user, 30);
	}

done:	
	cl_unlock(&cl_priv->mutex);
	
	SAFE_FREE(h->url);
	SAFE_FREE(h->page);
	if (json)
		json_free(json);

	return ret;
}

