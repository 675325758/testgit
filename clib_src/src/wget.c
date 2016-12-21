#include "cl_priv.h"
#include "wget.h"
#ifdef __GNUC__
#include <netdb.h>
#include <ctype.h>
#endif
#include "ioext.h"
#include "cl_dns.h"


#define	MAX_HTTP_DATA	(1024*1024)
#define	HTTP_CONNECT_TIMEOUT	5
#define	HTTP_RW_TIMEOUT	15

typedef struct {
	char *host;
	int port;
	char *path;
	char *user;
} host_info_t;
	
typedef struct {
	SOCKET sock;

	// 是否异常需要终止
	bool is_die;
	
	char *url;
	host_info_t host;
	bool is_post;
	char *post_data;

	// 当前读到哪里
	int buf_idx;
	// 接收缓冲区大小
	int buf_size;
	// 接收缓冲区
	u_int8_t *buf;
} wget_t;


static void parse_url(char *url, host_info_t *h)
{
	char *cp, *sp, *up, *pp;

	h->port = 80;
	h->host = url;
	
	if (strncmp(url, "http://", 7) == 0) {
		h->host = url + 7;
	}

	sp = strchr(h->host, '/');
	if (sp) {
		*sp++ = '\0';
		h->path = sp;
	} else
		h->path = cl_strdup("");

	up = strrchr(h->host, '@');
	if (up != NULL) {
		h->user = h->host;
		*up++ = '\0';
		h->host = up;
	} else
		h->user = NULL;

	pp = h->host;

	cp = strchr(pp, ':');
	if (cp != NULL) {
		*cp++ = '\0';
		h->port = atoi(cp);
	}

//	log_debug("parse url: host=%s, path=%s, user=%s, port=%u\n", h->host, h->path, h->user, h->port);
}

void wget_free(wget_t *wg)
{
	CLOSE_SOCK(wg->sock);
	
	SAFE_FREE(wg->url);
	SAFE_FREE(wg->post_data);
	
	SAFE_FREE(wg->buf);
	cl_free(wg);
}

static RS wget_gets(wget_t *wg, char *buf, int blen)
{
	int idx, n, need;

	blen -= 4;
	
	for (idx = 0, need = 2; idx < blen; idx += n) {
		n = recv_tmo(wg->sock, buf + idx, need, HTTP_RW_TIMEOUT);
		if (n <= 0) {
			log_err(true, "wget_gets read=%d\n", n);
			return RS_ERROR;
		}
		
		if (need == 1) {
			idx++;
			break;
		}
		// 0x0d 0x0a
		if (*(buf + idx) == 0x0d) {
			idx += 2;
			break;
		}
		if (*(buf + idx + 1) == 0x0d)
			need = 1;
	}
	buf[idx] = '\0';
//	log_debug("wget_gets(%d): %s\n", idx, buf);

	return RS_OK;
}

char *gethdr(wget_t *wg, char *buf, int bufsiz, int *istrunc)
{
	char *s, *hdrval;
	u_int8_t c;

	*istrunc = 0;

	/* retrieve header line */
	if (wget_gets(wg, buf, bufsiz) != RS_OK)
		return NULL;

	/* see if we are at the end of the headers */
	for (s = buf ; *s == '\r' ; ++s)
		;
	if (s[0] == '\n')
		return NULL;

	/* convert the header name to lower case */
	for (s = buf ; isalnum(*s) || *s == '-' ; ++s)
		*s = tolower(*s);

	/* verify we are at the end of the header name */
	if (*s != ':') {
		log_err(false, "bad header line: %s", buf);
		wg->is_die = true;
		return NULL;
	}

	/* locate the start of the header value */
	for (*s++ = '\0' ; *s == ' ' || *s == '\t' ; ++s)
		;
	hdrval = s;

	/* locate the end of header */
	while (*s != '\0' && *s != '\r' && *s != '\n')
		++s;

	/* end of header found */
	if (*s != '\0') {
		*s = '\0';
		return hdrval;
	}

	/* Rats!  The buffer isn't big enough to hold the entire header value. */
	while (recv_tmo(wg->sock, &c, 1, HTTP_RW_TIMEOUT) == 1 && c != '\n') {
	}
	*istrunc = 1;
	return hdrval;
}


static RS wget_recv(wget_t *wg)
{
	int status, n;
	char buf[1024];
	char *s, *endp;
	int chunked = 0;			/* chunked transfer encoding */
	int filesize = 0;		/* content-length of the file */
	int got_clen = 0;			/* got content-length: from server	*/

	if(wget_gets(wg, buf, sizeof(buf)) != RS_OK)
		return RS_ERROR;

	for (s = buf ; *s != '\0' && !isspace(*s) ; ++s)
	;
	for ( ; isspace(*s) ; ++s)
	;
	status = atoi(s);
	if (status != 200) {
		log_err(false, "server returned error %d: %s", atoi(s), buf);
		return RS_ERROR;
	}
	/*
	 * Retrieve HTTP headers.
	 */
	while ((s = gethdr(wg, buf, sizeof(buf),  &n)) != NULL) {
		if (strcasecmp(buf, "content-length") == 0) {
			unsigned long value;
			if ((value = strtoul(s, &endp, 0)) > MAX_HTTP_DATA) {
				log_err(false, "content-length %s is garbage", s);
				return RS_ERROR;
			}
			filesize = value;
			got_clen = 1;
			continue;
		}
		if (strcasecmp(buf, "transfer-encoding") == 0) {
			if (strcasecmp(s, "chunked") == 0) {
				chunked = got_clen = 1;
			} else {
				log_err(false, "server wants to do %s transfer encoding", s);
				return RS_ERROR;
			}
		}
	}

	if (wg->is_die)
		return RS_ERROR;

	/*
	 * Retrieve file
	 */
	if (chunked) {
		wget_gets(wg, buf, sizeof(buf));
		filesize = strtol(buf, &endp, 16);
	}

	if (wg->is_die)
		return RS_ERROR;
	
	do {
		while ((filesize > 0 || !got_clen) && (n = recv_tmo(wg->sock, buf, ((chunked || got_clen) && (filesize < sizeof(buf)) ? filesize : sizeof(buf)), HTTP_RW_TIMEOUT)) > 0) {
			if (wg->buf_size - wg->buf_idx <= n) {
				wg->buf_size *= 2;
				wg->buf = cl_realloc(wg->buf, wg->buf_size);
			}
			memcpy(wg->buf + wg->buf_idx, buf, n);
			wg->buf_idx += n;
			
			if (got_clen) {
				filesize -= n;
			}
		}

		if (chunked) {
			if (wget_gets(wg, buf, sizeof(buf)) != RS_OK
				|| wget_gets(wg, buf, sizeof(buf)) != RS_OK)
				break;
			filesize = strtol(buf, &endp, 16);
			if (filesize==0) {
				chunked = 0; /* all done! */
			}
		}

		if (n == 0 && wg->is_die) {
			log_err(true, "network read error\n");
		}
	} while (chunked);

	return RS_OK;
}

static RS wget_printf(wget_t *wg, const char *fmt, ...)
{
	int i, n;
	va_list vl;
	char buf[1024];
	
    va_start(vl, fmt);
	n = vsprintf(buf, fmt, vl);
    va_end(vl);

	if ((i = send_tmo(wg->sock, buf, n, HTTP_RW_TIMEOUT)) != n) {
		log_err(true, "wget_printf failed: n=%d, but ret=%d\n", n, i);
		return RS_ERROR;
	}

	return RS_OK;
}

static RS wget_send_request(wget_t *wg)
{
	if (wget_printf(wg, "GET /%s HTTP/1.1\r\n", wg->host.path) != RS_OK
		|| wget_printf(wg, "Host: %s\r\nUser-Agent: Wget\r\n", wg->host.host) != RS_OK
		|| wget_printf(wg, "Connection: close\r\n") != RS_OK
		|| wget_printf(wg, "\r\n") != RS_OK
		)
		return RS_ERROR;

	return RS_OK;
}

static RS post_send_request(wget_t *wg)
{
    //in_addr_t* addr;
    struct sockaddr_in addr;



    if (wg->host.host != NULL && ((addr.sin_addr.s_addr = inet_addr(wg->host.host))!= 0)) {
        if (wget_printf(wg, "POST /%s HTTP/1.1\r\n", wg->host.path) != RS_OK
            || wget_printf(wg, "Host: www.jiazhang008.com\r\nUser-Agent: Wget\r\n") != RS_OK
            || wget_printf(wg, "Connection: close\r\n") != RS_OK
            || wget_printf(wg, "Content-Type: application/x-www-form-urlencoded\r\n") != RS_OK
            || wget_printf(wg, "Content-Length: %u\r\n", strlen(wg->post_data)) != RS_OK
            || wget_printf(wg, "\r\n") != RS_OK
            || wget_printf(wg, "%s\r\n", wg->post_data) != RS_OK
            || wget_printf(wg, "\r\n") != RS_OK
            )
            return RS_ERROR;
    }else{
        if (wget_printf(wg, "POST /%s HTTP/1.1\r\n", wg->host.path) != RS_OK
            || wget_printf(wg, "Host: %s\r\nUser-Agent: Wget\r\n", wg->host.host) != RS_OK
            || wget_printf(wg, "Connection: close\r\n") != RS_OK
            || wget_printf(wg, "Content-Type: application/x-www-form-urlencoded\r\n") != RS_OK
            || wget_printf(wg, "Content-Length: %u\r\n", strlen(wg->post_data)) != RS_OK
            || wget_printf(wg, "\r\n") != RS_OK
            || wget_printf(wg, "%s\r\n", wg->post_data) != RS_OK
            || wget_printf(wg, "\r\n") != RS_OK
            )
            return RS_ERROR;
    }
    
	

	return RS_OK;
}

static RS wget_connect(wget_t *wg)
{
	u_int32_t ipp[1024];
	SOCKET sock = INVALID_SOCKET;
	int i, count;

	count = my_gethostbyname(wg->host.host, ipp, sizeof(ipp));
	if (count <= 0) {
		log_err(true, "my_gethostbyname failed for %s\n", wg->host.host);
		return RS_ERROR;
	}

	for (i = 0; i < count; i++) {
		if ((sock = connect_tmo(ipp[i], wg->host.port, HTTP_CONNECT_TIMEOUT)) != INVALID_SOCKET) {
			log_info("connect tcp to %u.%u.%u.%u port %u success!\n", IP_SHOW(ipp[i]), wg->host.port);
			break;
		}
		log_err(false, "connect tcp to %u.%u.%u.%u port %u failed! try next\n", IP_SHOW(ipp[i]), wg->host.port);
	}

	if (sock == INVALID_SOCKET)
		return RS_ERROR;
	
	wg->sock = sock;

	return RS_OK;
}

RS wget(char *url, char **ret_buf, int *ret_len, bool is_post, char *post_data)
{
	wget_t *wg;

	if (cl_priv->is_pt_mode) {
		*ret_buf = NULL;
		*ret_len = 0;
		return RS_ERROR;
	}

	wg = cl_calloc(sizeof(wget_t), 1);
	wg->url = cl_strdup(url);
	parse_url(wg->url, &wg->host);

	wg->buf_size = 1024*32;
	wg->buf = cl_malloc(wg->buf_size);
	wg->is_post = is_post;
	wg->post_data = cl_strdup(post_data == NULL ? "" : post_data);

	if (wget_connect(wg) != RS_OK) {
		goto err;
	}
	if (is_post) {
		if (post_send_request(wg) != RS_OK)
			goto err;
	} else {
		if (wget_send_request(wg) != RS_OK)
			goto err;
	}
	if (wget_recv(wg) != RS_OK)
		goto err;

	wg->buf[wg->buf_idx] = '\0';

	*ret_buf = wg->buf;
	*ret_len = wg->buf_idx;

	wg->buf = NULL;
	wget_free(wg);
	
	return RS_OK;

err:
	wget_free(wg);
	*ret_buf = NULL;
	*ret_len = 0;
	return RS_ERROR;
}




