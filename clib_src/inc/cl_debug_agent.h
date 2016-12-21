#ifndef	__CL_DEBUG_AGENT_H__
#define	__CL_DEBUG_AGENT_H__



#ifdef __cplusplus
extern "C" {
#endif 
#include "cl_user.h"
#include "client_lib.h"

#define IP_SHOW(IP) 				((IP)>>24)&0xFF, ((IP)>>16)&0xFF, ((IP)>>8)&0xFF, (IP)&0xFF

#define VERTION(maj, min, rev)		((maj&0xff) << 24| (min&0xff) << 16| (rev&0xff) <<8)
#define VERSION_SHOW(v)				(v>>24)&0xff, (v>>16)&0xff, (v>>8)&0xff

#define DEBUG_ERROR     1
#define DEBUG_WARN      2
#define DEBUG_TRACE     2
#define DEBUG_INFO      2
#define DEBUG_DEBUG     3
#define DEBUG_ALL      4

enum {
	CL_DEBUG_BEGIN = 1800,
	CL_DEBUG_CONN_STATUS = CL_DEBUG_BEGIN + 1, //设备连接状态
	CL_DEBUG_PORT_CHANGE = CL_DEBUG_BEGIN + 2, //成功绑定的tcp端口
	CL_DEBUG_IP_CHANGE = CL_DEBUG_BEGIN + 3, //ip变化
	CL_DEBUG_VERSION = CL_DEBUG_BEGIN + 4, //VERSION get
	CL_DEBUG_WIFI_INFO = CL_DEBUG_BEGIN + 5, //wifi info get
	CL_DEBUG_END = CL_DEBUG_BEGIN + 99
};


#define MG_SN_CONN			1
#define MG_SHOW_FLUSH		2
#define MG_DEBUG_LEVEL		3
#define MG_DEBUG_TIME_SET	4
#define MG_LEDCOLOR_SET	5
#define MG_UARTCOMMAND_SET	6



typedef struct cl_debug_info_s{
	u_int32_t ip;
	u_int32_t svn;	
	u_int8_t ssid[32+4];
	u_int8_t passwd[64+4];
	u_int32_t dev_status;	
	u_int32_t dev_uptime;	
	u_int32_t server_uptime;
	u_int8_t domainname[64+4];
	cl_version_t soft_version;
	u_int8_t com_data[32];
	u_int8_t com_time[32];
	u_int32_t systime;
	u_int16_t cpuper;
	u_int16_t memper;
	u_int32_t wan_ip;
	//new add
	u_int32_t DevServerIp;
	u_int32_t StmSoftVersion;
	u_int32_t StmHardVersion;
	u_int32_t LightCur;
	u_int32_t LightYes;
	u_int32_t LightNext;
	u_int16_t IrId;
	u_int32_t AcCur;
	u_int32_t ACVal;
	char vendor[16];
	//power
	int power;
	//devtype
	int ds_type;
	int dev_type;
	int ext_type;
	ad_data_t ad;//adjust
}cl_debug_info_t;





CLIB_API RS cl_debug_agent_init(cl_handle_t dev_handle, int port);
CLIB_API cl_debug_info_t *cl_get_debug_info_fuzhu(cl_handle_t dev_handle);
CLIB_API void cl_get_debug_info_free(cl_debug_info_t *debug_info);
CLIB_API void cl_debug_show_flush(cl_handle_t dev_handle);
CLIB_API void cl_set_debug_level(int level, int value);
CLIB_API void cl_set_debug_time(cl_handle_t dev_handle, struct tm *ptm);


#ifdef __cplusplus
}
#endif 

#endif

