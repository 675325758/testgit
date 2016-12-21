#include "cl_priv.h"
#include "wget.h"

#if 0
#ifdef log_debug
#undef log_debug
#define	log_debug(...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef log_err
#undef log_err
#define	log_err(print_err, ...) _log_err(__FILE__, __LINE__, print_err, __VA_ARGS__)
#endif

#ifdef log_info
#undef log_info
#define	log_info(...) _log_info(__FILE__, __LINE__, __VA_ARGS__)
#endif
#endif


typedef struct {
	char *url;
	/* 空为get，不空为post */
	char *post_data;
	int event;
	cl_handle_t handle;
	cl_thread_info_t *ti;
	u_int8_t index;
} http_param_t;


char *http_sprintf(char *buf, char *fmt, ...)
{
	int pos = 0;
	unsigned char *tmp_buf, *p;
	va_list vl;
	char dc[] = "0123456789ABCDEF";

	tmp_buf = cl_malloc(4096);
	
    va_start(vl, fmt);
	vsprintf((char*)tmp_buf, fmt, vl);
    va_end(vl);

	for (p = tmp_buf, pos = 0; *p != '\0'; p++) {
		if (('0' <= *p && *p <= '9') || ('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z')) {
			buf[pos++] = *p;
			continue;
		}
		buf[pos++] = '%';
		buf[pos++] = dc[(*p & 0xF0) >> 4];
		buf[pos++] = dc[*p & 0xF];
	}
	buf[pos] = '\0';

	cl_free(tmp_buf);

	return buf;
}

RS null_proc_notify(struct cl_notify_pkt_s *pkt)
{
	log_err(false, "ignore wget_proc_notify\n");
	
	return RS_ERROR;
}

u_int32_t http_get_thread(void *param)
{
	RS ret;
	int len = 0;
	char *page = NULL;
	http_param_t hp;
	cl_notify_pkt_t *pkt;
	cln_http_t *h;


	hp = *((http_param_t *)param);
	hp.url = cl_strdup(hp.url);
	if (hp.post_data != NULL)
		hp.post_data = cl_strdup(hp.post_data);
	hp.ti->proc_notify = null_proc_notify;

#ifdef SUPPORT_TRAFFIC_STAT
    HTTP_PKT_STAT(false, ((u_int32_t)strlen(hp.url)));
#endif

	log_debug("http_get_thread [%s]\n", hp.url);

	ret = wget(hp.url, &page, &len, (hp.post_data != NULL), hp.post_data);

#ifdef SUPPORT_TRAFFIC_STAT
    HTTP_PKT_STAT(true, len);
#endif
	// 通知其他线程
	pkt = cl_notify_pkt_new(4096, hp.event, 0);
	h = (cln_http_t *)&pkt->data[0];
	h->handle = hp.handle;
	h->result = ret;
	h->url = hp.url; /* 目标线程来释放 */
	hp.url = NULL;
	if (page == NULL) {
		h->page = cl_strdup("");
	} else {
		h->page = page; /* 目标线程来释放 */
	}
	
	pkt->param_len = sizeof(cln_http_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	SAFE_FREE(hp.post_data);

	return RS_OK;
}

RS http_get(const char *url, int event, cl_handle_t handle)
{
	http_param_t hp;
	cl_thread_info_t ti;

	log_debug("http_get: %s\n", url);
	memset(&ti, 0, sizeof(cl_thread_info_t));
	memset(&hp, 0, sizeof(http_param_t));
	
	hp.url = (char *)url;
	hp.event = event;
	hp.handle = handle;
	hp.ti = &ti;
	
	cl_create_thread(&ti, "http_get", http_get_thread, (void *)&hp);
	cl_destroy_thread(&ti);

	return RS_OK;
}

u_int32_t http_get_thread_ext(void *param)
{
	RS ret;
	int len = 0;
	char *page = NULL;
	http_param_t hp;
	cl_notify_pkt_t *pkt;
	cln_http_t *h;


	hp = *((http_param_t *)param);
	hp.url = cl_strdup(hp.url);
	if (hp.post_data != NULL)
		hp.post_data = cl_strdup(hp.post_data);
	hp.ti->proc_notify = null_proc_notify;

#ifdef SUPPORT_TRAFFIC_STAT
    HTTP_PKT_STAT(false, ((u_int32_t)strlen(hp.url)));
#endif

	log_debug("http_get_thread [%s]\n", hp.url);

	ret = wget(hp.url, &page, &len, (hp.post_data != NULL), hp.post_data);

#ifdef SUPPORT_TRAFFIC_STAT
    HTTP_PKT_STAT(true, len);
#endif
	// 通知其他线程
	pkt = cl_notify_pkt_new(4096, hp.event, 0);
	h = (cln_http_t *)&pkt->data[0];
	h->handle = hp.handle;
	h->result = ret;
	h->url = hp.url; /* 目标线程来释放 */
	h->index = hp.index;
	hp.url = NULL;
	if (page == NULL) {
		h->page = cl_strdup("");
	} else {
		h->page = page; /* 目标线程来释放 */
	}
	
	pkt->param_len = sizeof(cln_http_t);

	ret = cl_send_notify(&cl_priv->thread_main, pkt);

	cl_notify_pkt_free(pkt);

	SAFE_FREE(hp.post_data);

	return RS_OK;
}

RS http_get_ext(const char *url, int event, cl_handle_t handle, u_int8_t index)
{
	http_param_t hp;
	cl_thread_info_t ti;

	log_debug("http_get: %s\n", url);
	memset(&ti, 0, sizeof(cl_thread_info_t));
	memset(&hp, 0, sizeof(http_param_t));
	
	hp.url = (char *)url;
	hp.event = event;
	hp.handle = handle;
	hp.ti = &ti;
	hp.index = index;
	
	cl_create_thread(&ti, "http_get", http_get_thread_ext, (void *)&hp);
	cl_destroy_thread(&ti);

	return RS_OK;
}

RS http_post(const char *url, const char *post_data, int event, cl_handle_t handle)
{
	http_param_t hp;
	cl_thread_info_t ti;

	log_debug("http_post: %s, post_data=%s\n", url, post_data == NULL ? "" : post_data);
	memset(&ti, 0, sizeof(cl_thread_info_t));
	
	hp.url = (char *)url;
	hp.post_data = (char *)post_data;
	hp.event = event;
	hp.handle = handle;
	hp.ti = &ti;
	
	cl_create_thread(&ti, "http_post", http_get_thread, (void *)&hp);
	cl_destroy_thread(&ti);

	return RS_OK;
}


