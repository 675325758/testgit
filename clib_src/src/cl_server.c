#include "client_lib.h"
#include "cl_priv.h"
#include "cl_server.h"
#include "cl_notify.h"
#include "uasc_priv.h"
#include "uas_client.h"


////////////////////////////////////////////
//  server
////////////////////////////////////////////

/*
	功能：
		设置分配服务器的地址。
		不调用本函数，缺省是 www.jiazhang008.com
	参数IN：
		server：可以是IP地址的字符串，也可以是域名。
	返回：
		RS_OK: 成功
		其他：失败
*/
RS cl_disp_server_set(const char *server)
{
	RS ret;

	CL_CHECK_INIT;

	if(server != NULL){
#ifndef MUT_SERVER_ADAPT
		cl_lock(&cl_priv->mutex);
		SAFE_FREE(cl_priv->disp_server);
		cl_priv->disp_server = cl_strdup(server);
		cl_unlock(&cl_priv->mutex);
		ret = cl_send_notify_simple(&cl_priv->thread_dns_resolv, CLNE_RESOLV_DISP_SERVER);
#else
        //自适应多国服务器，不再支持设置dns
        return RS_OK;
#endif
	}else{
		ret = cl_send_notify_simple(&cl_priv->thread_dns_resolv, CLNE_CLEAR_DNS_CACHE);
	}

	

	return ret;
}

/*
	功能：
		获取分配服务器的IP地址
	参数IN：
		ip: 地址数组的首地址
		count: ip数组个数
	参数OUT：
		ip: 存放解析出来的IP地址列表
		count: 解析出来的ip地址个数
	返回：
		RS_OK: 成功
		其他：失败
*/
RS cl_disp_server_get_ip(u_int32_t *ip, u_int32_t *count)
{
	int i;

	CL_CHECK_INIT;

	cl_lock(&cl_priv->mutex);
	
	*count = min(*count, cl_priv->num_ip_disp_server);
	for (i = 0; i < (int)(*count); i++) {
		ip[i] = cl_priv->ip_disp_server[i];
	}
	
	cl_unlock(&cl_priv->mutex);

	return RS_OK;
}

CLIB_API RS cl_start_connect_to_app_server()
{
	CL_CHECK_INIT;
	return cl_send_u8_notify(&cl_priv->thread_main, 0, CLNE_UASC_SERVER_CTRL, ACT_UASC_START_CONNECT,0 );
}

CLIB_API RS cl_get_app_server_info(cl_app_server_connect_info_t* info)
{
	uasc_session_t* s;

	CL_CHECK_INIT;

	if(!info){
		return RS_INVALID_PARAM;
	}
	
	memset(info,0,sizeof(*info));

	cl_lock(&cl_priv->mutex);
	
	if(cl_priv->uasc_session == NULL){
		goto end;
	}
	s =  (uasc_session_t*)cl_priv->uasc_session;
	info->cur_stat = s->status;
	info->server_ip = s->ip;
	info->server_port = s->port;
	if(s->status == UASCS_ESTABLISH){
		info->is_establish = true;
	}
		
end:
	cl_unlock(&cl_priv->mutex);

	return RS_OK;
}

CLIB_API RS cl_push_app_stat_info(cl_app_stat_info_t* stat)
{
	u_int8_t buf[512]={0};

	CL_CHECK_INIT;
	
	if(!stat){
		return RS_INVALID_PARAM;
	}	

	memcpy(buf,stat,sizeof(*stat));

	return cl_send_var_data_notify(&cl_priv->thread_main,0,CLNE_UASC_SERVER_CTRL, ACT_UASC_PUSH_APP_STAT,(u_int8_t*)(&buf[0]),sizeof(buf));
}

