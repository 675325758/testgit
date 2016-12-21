/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: xy_priv.c
**  File:    xy_priv.c
**  Author:  liubenlong
**  Date:    07/16/2015
**
**  Purpose:
**    鑫源私有文件.
**************************************************************************/


/* Include files. */
#include "client_lib.h"
#include "cl_smart_appliance.h"
#include "cl_priv.h"
#include "udp_ctrl.h"
#include "smart_appliance_priv.h"
#include "cl_xy.h"
#include "xy_priv.h"
#include "udp_device_common_priv.h"

/* Macro constant definitions. */
//cmd
//状态查询
#define XY_CMD_QUERY			0X01
//时间同步
#define XY_CMD_TIMER_SYNC		0X11
//温控器模式等控制
#define XY_CMD_CTRL				0X12
//温控器后台设置
#define XY_CMD_BACKGROUND_CTRL	0x13
//智能模式设置
#define XY_CMD_SMART_SET		0X14
//无线状态设置
#define XY_CMD_WIFI_STATUS_SET	0X0F
//回复出厂
#define XY_CMD_WIFI_RESET		0X1F

//status位状态值
#define XY_STATUS_BIT_ONOFF			7
#define XY_STATUS_BIT_HEAT			6
#define XY_STATUS_BIT_LOCK			5
#define XY_STATUS_BIT_PROBE			4

/* Type definitions. */

//需要字节对齐
#pragma pack(push, 1)
//通用结构
typedef struct {
	u_int8_t cmd;
	u_int16_t len;
	u_int8_t pad;
	//u_int8_t data;
}ucp_xy_cc_t;

typedef struct {
	u_int8_t start_index;
	u_int8_t end_index;
	u_int8_t week;
	u_int8_t temp;
}ucp_xy_tpda_t;

//time part head
typedef struct {
	u_int8_t sub_mode;
	u_int8_t num;
	u_int16_t pad;
	//ts_xy_tpda_t data;
}ucp_xy_tphd_t;

typedef struct {
	u_int8_t temp;
	u_int8_t pad[3];
}ucp_xy_tmp_t;

//smarthome
typedef struct {
	u_int8_t on;
	u_int8_t pad[3];
}ucp_xy_sh_t;

//命令结构 cmd ctrl data
typedef struct {
	u_int8_t power;
	u_int8_t lock;
	u_int8_t mode;
	u_int8_t temp1;
	u_int8_t temp2;
	u_int8_t hourl;
	u_int8_t hourh;
	u_int8_t pad;
}ucp_xy_ccd_t;

//命令结构 cmd adjust data
typedef struct {
	u_int8_t temp_adj;//温度校正值+20，例如温度校正值为-3，则Adj=17。Adj取值范围11~29
	u_int8_t temp_top;//可设定温度上限（5~85）
	u_int8_t temp_tolr;//温控容差(1-9)
	u_int8_t temp_otemp;//过热保护温度（15~85）
	u_int8_t temp_prottrig;//加热器保护触发时间+1（1~100），例如触发时间5小时，则ProtTrig=6
	u_int8_t temp_protlen;//加热器保护时长（10~90）
	u_int8_t pad[2];
}ucp_xy_cad_t;

//命令结构 cmd smart mode time
typedef struct {
	u_int8_t pad;//为0
	u_int8_t day;
	u_int8_t temp[XY_SMART_WEEK_TIMEPOINT];
}ucp_xy_csmt_t;

//query
typedef struct {
	u_int8_t pad[2];
}ucp_xy_query_t;

/*
T1i、Tf低四位：室温整数和小数。
T2i、Tf高四位：地温整数和小数。
T1i和T2i为补码形式。例如室温-2.3℃，地温-0.7℃，T1i=0xFE，T2i=0x80，Tf=0x73
TempSet：当前目标温度
Status：当前状态
Bit7：1-开机 2-关机
Bit6：0-未加热 1-正在加热
Bit5：1-按键开放 2-按键锁定
Bit4：0-正常 1-探头故障
Bit3~Bit0：保留
Mode：当前工作模式。1-恒温 2-智能 3-休假
Temp1：恒温模式的目标温度
Temp2：休假模式的目标温度
HHoursL：剩余休假小时数的低字节
HhoursH：剩余休假小时数的高字节。例如休假300小时，则HhoursL=0x2C，HhoursH=0x01。
Adj：温度校正值+20，例如温度校正值为-3，则Adj=17。Adj取值范围11~29
Top：可设定温度上限（5~85）
Tolr：温控容差（1~9）。
Otemp：过热保护温度（15~85）
ProtTrig：加热器保护触发时间+1（1~100），例如触发时间5小时，则ProtTrig=6
ProtLen：加热器保护时长（10~90）
T1_0 ~ T1_47：周一的智能模式设定温度，每半小时一个温度值。
T7_0 ~ T7_47：周日的智能模式设定温度，每半小时一个温度值。
*/
//命令结构，cmd ctrl update data
typedef struct {
	u_int8_t result;
	u_int8_t pad;
	u_int8_t t1i;
	u_int8_t t2i;
	
	u_int8_t tf;
	u_int8_t tempset;
	u_int8_t status;
	u_int8_t mode;
	
	u_int8_t temp1;
	u_int8_t temp2;
	u_int8_t hourl;
	u_int8_t hourh;
	
	u_int8_t adj;
	u_int8_t top;
	u_int8_t tolr;
	u_int8_t otemp;
	
	u_int8_t prottrig;
	u_int8_t protlen;

	u_int8_t pad2[8];
	u_int8_t temp[XY_SMART_WEEK_MAX][XY_SMART_WEEK_TIMEPOINT];
	
	u_int8_t pad3[2];
}ucp_xy_ccud_t;

#pragma pack(pop)

/* Local function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
//static void xy_mem_dump(char *name, void *ptr, int len);
static void _xy_sync_sm_cur_tmp(cl_xy_info_t *pxy);

void _free_xy_info(cl_xy_info_t* info)
{

}

static int _xy_buff_build(u_int8_t *buff, int max_len,  u_int8_t cmd, u_int8_t *pdata, u_int16_t len)
{
	ucp_xy_cc_t *pcc = (ucp_xy_cc_t *)buff;

	if (len + sizeof(ucp_xy_cc_t) > (u_int32_t)max_len) {
		log_err(false, "error len=%u max_len=%u\n", len + sizeof(ucp_xy_cc_t), max_len);
		return 0;
	}

	pcc->cmd = cmd;
	pcc->len = htons(len);

	if (len > 0) {
		memcpy((u_int8_t *)(&pcc[1]), pdata, len);
	}

	return sizeof(ucp_xy_cc_t) + len;
}

static int _xy_ctrl_onoff(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t onoff, cl_xy_info_t *pxy)
{
	int len = 0;
	ucp_xy_ccd_t ccd;

	memset((void *)&ccd, 0, sizeof(ccd));
	
	ccd.power = (onoff == XY_STATUS_OFF)?XY_UART_STATUS_OFF:XY_UART_STATUS_ON;

	pxy->onoff = onoff;
	
	len = _xy_buff_build(pdata, max_len, XY_CMD_CTRL, (u_int8_t *)&ccd, sizeof(ccd));
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);

	return len;
}

static int _xy_ctrl_temp(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t temp, cl_xy_info_t *pxy)
{
	int len;
	ucp_xy_ccd_t ccd;

	memset((void *)&ccd, 0, sizeof(ccd));

	//ccd.mode = XY_MODE_CONSTTEMP;
#if 0	
	if (pxy->mode == XY_MODE_HOLIDAY) {
		ccd.temp2 = temp;
	} else {
		ccd.temp1 = temp;
	}
#else
	ccd.temp1 = temp;
#endif

	pxy->cons_temp = temp;
	pxy->cur_dst_temp = temp;
	
	len = _xy_buff_build(pdata, max_len, XY_CMD_CTRL, (u_int8_t *)&ccd, sizeof(ccd));
	
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);

	return len;
}

static int _xy_ctrl_mode(ucc_session_t *s, ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t mode, cl_xy_info_t *pxy)
{
	int len;
	ucp_xy_ccd_t ccd;

	memset((void *)&ccd, 0, sizeof(ccd));
	
	ccd.mode = mode;

	pxy->mode = mode;
#if 0	
	if (mode == XY_MODE_HOLIDAY) {
		ccd.temp2 = 5;
	}
#endif	
	
	len = _xy_buff_build(pdata, max_len, XY_CMD_CTRL, (u_int8_t *)&ccd, sizeof(ccd));

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);

	if (mode == XY_MODE_SMART) {
		s->drop_update_num = XY_DROP_UPDATE_NUM;
	}

	return len;
}

static int _xy_ctrl_time(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int16_t time, cl_xy_info_t *pxy)
{
	int len;
	ucp_xy_ccd_t ccd;

	memset((void *)&ccd, 0, sizeof(ccd));
	
	ccd.hourl = time&0xff;
	ccd.hourh = (time&0xff00)>>8;

	pxy->remain_time = time;

	len = _xy_buff_build(pdata, max_len, XY_CMD_CTRL, (u_int8_t *)&ccd, sizeof(ccd));

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);

	return len;
}

static int _xy_ctrl_lock(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t lock, cl_xy_info_t *pxy)
{
	int len;
	ucp_xy_ccd_t ccd;

	memset((void *)&ccd, 0, sizeof(ccd));
	
	ccd.lock = (lock == XY_LOCK_OFF)?XY_UART_LOCK_OFF:XY_UART_LOCK_ON;

	pxy->lock = lock;
	
	len = _xy_buff_build(pdata, max_len, XY_CMD_CTRL, (u_int8_t *)&ccd, sizeof(ccd));

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);

	return len;
}

static int _xy_ctrl_et(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t temp, cl_xy_info_t *pxy)
{
	int len;
	ucp_xy_tmp_t et;

	memset((void *)&et, 0, sizeof(et));
	
	et.temp = temp;

	pxy->extern_temp = temp;
	//log_debug("pxy->extern_temp=%u\n", pxy->extern_temp);
	//len = _xy_buff_build(pdata, max_len, XY_CMD_CTRL, (u_int8_t *)&et, sizeof(et));
	len = sizeof(et);
	memcpy(pdata, (u_int8_t *)&et, len);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_EX_TMP,len);

	return len;
}

static int _xy_ctrl_smarthome_onoff(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t temp, cl_xy_info_t *pxy, smart_air_ctrl_t* ac)
{
	int len;
	ucp_xy_sh_t sh;

	memset((void *)&sh, 0, sizeof(sh));
	
	sh.on = temp;

	ac->smart_home_enable = temp;
	pxy->smarthome_onoff = temp;
	len = sizeof(sh);
	memcpy(pdata, (u_int8_t *)&sh, len);

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_SMART_HOME,len);

	return len;
}

static int _xy_config_sm_stm(ucc_session_t *s, u_int8_t mode, cl_xy_info_t *pxy)
{
	int i, len, j, index, num;
	u_int8_t buf[1024];
	cl_xy_tp_t *pxytp = NULL;
	ucp_xy_csmt_t csmt;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pdata = (u_int8_t *)(&uo[1]);
	u_int32_t max_len = sizeof(buf) - sizeof(ucp_obj_t);
	cl_xy_smartmode_info_t *psti = (cl_xy_smartmode_info_t *)&pxy->smart_mode;
	

	/*一周时间配置报文发送给单片机*/
	for(i = 0; i < XY_SMART_WEEK_MAX; i++) {
		//log_debug("sizeof(pxsi->smart_temp[i])=%u\n", sizeof(pxy->timepoint[]));
		memset((void *)&csmt, 0, sizeof(csmt));
		csmt.day |= BIT(i);
		for(j = 0, index = 0; j < XY_SMART_DAY_TIMEPOINT_MAX; j++) {
			pxytp = &psti->timepoint[mode][i][j];
			if (!pxytp->valid) {
				continue;
			}

			num = pxytp->end_index - pxytp->start_index;
			memset((void *)&csmt.temp[index], pxytp->temp, num);
			index += num;
		}
		len = _xy_buff_build(pdata, max_len, XY_CMD_SMART_SET, (u_int8_t *)&csmt, sizeof(csmt));
		fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);
		sa_set_obj_value_only(s, 0x1, buf, sizeof(*uo)+len);
		//xy_mem_dump("smsend:", &csmt, sizeof(csmt));
	}
	
	return 0;
}

static int _xy_ctrl_query(ucc_session_t *s)
{
	int len;
	u_int8_t buf[1024];
	ucp_xy_query_t query;	
//	cl_xy_tp_t *pxytp = NULL;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pdata = (u_int8_t *)(&uo[1]);
	u_int32_t max_len = sizeof(buf) - sizeof(ucp_obj_t);
	
	memset((void *)&query, 0, sizeof(query));
	
	len = _xy_buff_build(pdata, max_len, XY_CMD_QUERY, (u_int8_t *)&query, sizeof(query));

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);
	sa_set_obj_value(s, 0x1, buf, sizeof(*uo)+len);
	
    return 0;
}

static int _xy_config_sm_dev(ucc_session_t *s, u_int8_t *psrc_data, cl_xy_info_t *pxy)
{
	int len;
	u_int8_t buf[1024];
//	cl_xy_tp_t *pxtp = NULL;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	cl_xy_smartmode_info_t *psend = (cl_xy_smartmode_info_t *)(&uo[1]);
	cl_xy_smartmode_info_t *pxsi = (cl_xy_smartmode_info_t *)psrc_data;

	if (pxsi->sub_mode.mode >= XY_SMARTHOME_SUB_MODE_NUM) {
		return 0;
	}
	
	//memcpy((void *)&pxy->smart_mode, (void *)pxsi, sizeof(cl_xy_smartmode_info_t));
	len = sizeof(cl_xy_smartmode_info_t);
	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_TIME,len);
	memcpy((void *)psend, (void *)pxsi, sizeof(cl_xy_smartmode_info_t));
	sa_set_obj_value(s, 0x1, buf, sizeof(*uo)+len);

	//修改本地当前温度值
	_xy_sync_sm_cur_tmp(pxy);

	return 0;
}

static int _xy_ctrl_smart_mode(ucc_session_t *s, u_int8_t mode, cl_xy_info_t *pxy)
{
	if (mode >= XY_SMARTHOME_SUB_MODE_NUM) {
		return 0;
	}
	
	if (pxy->smart_mode.sub_mode.mode != mode) {
		//save config
		pxy->smart_mode.sub_mode.mode = mode;
		_xy_config_sm_dev(s, (u_int8_t *)&pxy->smart_mode, pxy);
	} else {
		//修改本地当前温度值
		_xy_sync_sm_cur_tmp(pxy);
	}

	return 0;
}

static void _xy_sync_sm_cur_tmp(cl_xy_info_t *pxy)
{
#if 0	
	int i, index;
	struct tm tm;
	time_t now;
	u_int32_t tmp_hour;
	u_int8_t tmp;
	cl_xy_tp_t *pxt = NULL;
	
	now = get_sec();
	localtime_r(&now, &tm);

	index = (pxy->smart_mode.sub_mode.mode)*(tm.tm_wday);
	pxt = &(pxy->smart_mode.timepoint[index]);

	tmp = pxt->temp;
	tmp_hour = tm.tm_hour*2;
	for(i = 0; i < XY_SMART_DAY_TIMEPOINT_MAX; i++) {
		if (!pxt[i].valid) {
			continue;
		}
		if (pxt[i].start_index <= tmp_hour &&
			pxt[i].end_index >= tmp_hour) {
			tmp = pxt[i].temp;
			break;
		}
	}

	pxy->cur_dst_temp = tmp;
#else
	return;
#endif
}

static int _xy_config_sm(ucc_session_t *s, u_int8_t *psrc_data, cl_xy_info_t *pxy)
{
	cl_xy_smartmode_info_t *pxsi = (cl_xy_smartmode_info_t *)psrc_data;

	if (memcmp((void *)&pxy->smart_mode, (void *)pxsi, sizeof(cl_xy_smartmode_info_t)) == 0) {
		//修改本地当前温度值
		_xy_sync_sm_cur_tmp(pxy);
		return 0;
	}
	
	memcpy((void *)&pxy->smart_mode, (void *)pxsi, sizeof(cl_xy_smartmode_info_t));
	//配置保存报文发送给设备保存
	_xy_config_sm_dev(s, psrc_data, pxy);
	
	s->drop_update_num = XY_DROP_UPDATE_NUM;

    return 0;
}

static int _xy_ctrl_adjust(ucp_obj_t* uo, u_int8_t *pdata, u_int32_t max_len, u_int8_t *psrc_data, cl_xy_info_t *pxy)
{
	int len;
	ucp_xy_cad_t adj;
	cl_xy_adjust_t *pxya = (cl_xy_adjust_t *)psrc_data;

	memset((void *)&adj, 0, sizeof(adj));
	
	adj.temp_adj = pxya->temp_adj;
	adj.temp_top = pxya->temp_top;
	adj.temp_tolr = pxya->temp_tolr;
	adj.temp_otemp = pxya->temp_otemp;
	adj.temp_prottrig = pxya->temp_prottrig;
	adj.temp_protlen = pxya->temp_protlen;

	memcpy((void *)&pxy->adjust, pxya, sizeof(cl_xy_adjust_t));
	
	len = _xy_buff_build(pdata, max_len, XY_CMD_BACKGROUND_CTRL, (u_int8_t *)&adj, sizeof(adj));

	fill_net_ucp_obj(uo, UCOT_IA, UCSOT_IA_THERMOSTAT_XY, UCAT_IA_THERMOSTAT_CTRL,len);

	return len;
}

bool xy_proc_notify(user_t *user, cl_notify_pkt_t *pkt, RS *ret)
{
	bool res = true;
	cln_common_info_t *info;
	smart_air_ctrl_t* ac;
	cl_xy_info_t *pxy;
	char buf[1024] = {0};
	u_int8_t *ptmp = NULL;
	u_int8_t tmp = 0;
	u_int16_t tmp16 = 0;
	ucc_session_t *s = NULL;
	ucp_obj_t* uo = (ucp_obj_t*)buf;
	u_int8_t *pdata = (u_int8_t *)(&uo[1]);	
	u_int32_t max_data_len = sizeof(buf) - sizeof(ucp_obj_t);

	
	int len = 0;

	info = (cln_common_info_t *)&pkt->data[0];
	ptmp = (u_int8_t *)info->u.u8_data;
	
	if(!user->smart_appliance_ctrl) {
		log_err(false, "%s error handle %08x\n", __FUNCTION__, info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	ac = ((smart_appliance_ctrl_t*)user->smart_appliance_ctrl)->sub_ctrl;
	if (!ac ||!ac->com_udp_dev_info.device_info) {
		log_err(false, "%s error handle %08x\n", __FUNCTION__, info->handle);
		*ret = RS_INVALID_PARAM;
		return false;
	}

	s = ac->sac->user->uc_session;
	pxy = ac->com_udp_dev_info.device_info;

	tmp = cci_u8_data(info);
	tmp16 = cci_u16_data(info);
	ptmp = cci_pointer_data(info);
	pxy->last_cmd = 0;
	switch(info->action){
	case ACT_XY_CTRL_ONOFF:
		pxy->last_cmd = XY_CMD_CTRL;
		len = _xy_ctrl_onoff(uo, pdata, max_data_len, tmp, pxy);
		break;
	case ACT_XY_CTRL_TEMP:
		pxy->last_cmd = XY_CMD_CTRL;
		len = _xy_ctrl_temp(uo, pdata, max_data_len, tmp, pxy);
		break;
	case ACT_XY_CTRL_MODE:
		pxy->last_cmd = XY_CMD_CTRL;
		len = _xy_ctrl_mode(s, uo, pdata, max_data_len, tmp, pxy);
		break;
	case ACT_XY_CTRL_TIME:
		pxy->last_cmd = XY_CMD_CTRL;
		len = _xy_ctrl_time(uo, pdata, max_data_len, tmp16, pxy);
		break;
	case ACT_XY_CTRL_ADJUST:
		pxy->last_cmd = XY_CMD_BACKGROUND_CTRL;
		len = _xy_ctrl_adjust(uo, pdata, max_data_len, ptmp, pxy);
		break;
	case ACT_XY_CTRL_LOCK_ONOFF:
		pxy->last_cmd = XY_CMD_CTRL;
		len = _xy_ctrl_lock(uo, pdata, max_data_len, tmp, pxy);
		break;
	case ACT_XY_CONFIG_SMART_MODE:
		len = _xy_config_sm(s, ptmp, pxy);
		break;
	case ACT_XY_CTRL_EXTERN_TEMP:
		len = _xy_ctrl_et(uo, pdata, max_data_len, tmp, pxy);
		break;
	case ACT_XY_CTRL_SMARTHOME_ONOFF:
		len = _xy_ctrl_smarthome_onoff(uo, pdata, max_data_len, tmp, pxy, ac);
		break;
	case ACT_XY_CTRL_SMART_MODE:
		len = _xy_ctrl_smart_mode(s, tmp, pxy);
		break;
	default:
		*ret = RS_INVALID_PARAM;
		break;		
	}

	//控制后立马更新数据
	event_push(user->callback, UE_INFO_MODIFY, user->handle, user->callback_handle);
    event_cancel_merge(user->handle);

	if(len > 0){
		sa_set_obj_value_only(ac->sac->user->uc_session, 0x1, buf, sizeof(*uo)+len);
	}
		
	return res;
}

#if 0
#define	LOG_BUF_SIZE	(16*1024)
static void _log_test(const char *fmt, ...)
{
	int pos = 0;
	char *buf;
	char file[1024];
	FILE *fp = NULL;
	va_list vl;

	if (!cl_priv->dir) {
		return;
	}

	sprintf(file, "%s/xy_debug.txt", cl_priv->dir);
	fp = fopen(file, "a+");
	if (!fp) {
		return;
	}
	
	buf = malloc(LOG_BUF_SIZE+64);
	
    va_start(vl, fmt);
	pos += vsnprintf(buf + pos, LOG_BUF_SIZE-pos, fmt, vl);
	va_end(vl);

	fwrite(buf, pos, 1, fp);

	fclose(fp);
	free(buf);
}

void memdump_test(char* pre,void* dest, u_int32_t len)
{
    u_int8_t* p = dest;
    u_int32_t i;
	int index = 0;
	char buff[1024*10];
    
    if (!p) {
        return;
    }
    
    if (pre) {
        index += sprintf(buff+index, "%s:\n",pre);
    }
    
    for (i = 0; i<len; i++) {
        if (i>0 && i%16 == 0) {
           index += sprintf(buff + index, "\n");
        }
        
        index += sprintf(buff+index, "%02x ",p[i]);
    }
    
    _log_test("%s\n", buff);
    
}

static void xy_dump(ucc_session_t *s, cl_xy_info_t *info, ucp_xy_ccud_t *pccud)
{
	time_t now = time(NULL);
	
	if (info->cur_dst_temp != 0 &&
		info->cons_temp != 0 &&
		info->holiday_temp != 0) {
		//return;
	}

	_log_test("\ntime=%s\n", ctime(&now));
	_log_test("onoff=%u mode=%u root_temp=%u di_temp=%u cur_dst_temp=%u\n", 
		info->onoff, info->mode, info->root_temp, info->di_temp, info->cur_dst_temp);
	_log_test("heat=%u lock=%u err=%u cons_temp=%u holiday_temp=%u\n", 
		info->heat, info->lock, info->err, info->cons_temp, info->holiday_temp);
	_log_test("remain_time=%u last_cmd=%u extern_temp=%u probe_err=%u\n", 
		info->remain_time, info->last_cmd, info->extern_temp, info->probe_err);

	memdump_test("xy_dump", pccud, sizeof(*pccud));
}
#endif

static bool _xy_update_data_push(ucc_session_t *s, cl_xy_info_t *info, ucp_obj_t* obj)
{
	int len;
	bool ret = false;
	ucp_xy_cc_t *pcc = NULL;
	ucp_xy_ccud_t *pccud = NULL;
	u_int16_t temp16 = 0;
	u_int8_t temp8 = 0;
	cl_xy_adjust_t adjust;

	if (obj->param_len < sizeof(ucp_xy_cc_t)) {
		log_debug("%s %d error len=%u sile=%u\n", 
			__FUNCTION__, __LINE__, obj->param_len, sizeof(ucp_xy_cc_t));
		return false;
	}

	pcc = (ucp_xy_cc_t *)(&obj[1]);
	pccud = (ucp_xy_ccud_t *)(&pcc[1]);

	pcc->len = htons(pcc->len);

	len = pcc->len + sizeof(ucp_xy_cc_t);
	if (len > obj->param_len) {
		log_debug("%s %d error len=%u obj->param_len=%u\n", 
			__FUNCTION__, __LINE__, len, obj->param_len);
		return false;
	}

	if (pcc->len < sizeof(ucp_xy_ccud_t)) {
		log_debug("%s %d error pcc->len=%u sizeof(ucp_xy_ccud_t)=%u\n", 
			__FUNCTION__, __LINE__, pcc->len, sizeof(ucp_xy_ccud_t));
		return false;
	}

	//查询命令时不判断结果
	if (pcc->cmd != XY_CMD_QUERY) {
		if (info->err != pccud->result) {
			ret = true;
			info->err = pccud->result;
		}
		//log_debug("cmd excute error=%u\n", pccud->result);
	}

	s->droped_num = 0;
	//获取室温
	temp16 = (u_int16_t)(((char)pccud->t1i)*10 + (char)(pccud->tf&0xf));
	if ((temp16) != (info->root_temp)) {
		info->root_temp = temp16;
		ret = true;
	}
	
	//获取地温
	temp16 = (u_int16_t)(((char)pccud->t2i)*10 + (char)((pccud->tf>>4)&0xf));
	if (temp16 != info->di_temp) {
		info->di_temp = temp16;
		ret = true;
	}
	
	if (info->cur_dst_temp != pccud->tempset) {
		info->cur_dst_temp = pccud->tempset;
		ret = true;
	}
	//status
	//开关状态
	if (pccud->status & BIT(XY_STATUS_BIT_ONOFF)) {
		temp8 = XY_STATUS_ON;
	} else {
		temp8 = XY_STATUS_OFF;
	}
	
	if (temp8 != info->onoff) {
		info->onoff = temp8;
		ret = true;
	}
	
	//加热状态
	if (pccud->status & BIT(XY_STATUS_BIT_HEAT)) {
		temp8 = XY_HEAT_ON;
	} else {
		temp8 = XY_HEAT_OFF;
	}

	if (temp8 != info->heat) {
		info->heat = temp8;
		ret = true;
	}
	
	//锁定状态
	if (pccud->status & BIT(XY_STATUS_BIT_LOCK)) {
		temp8 = XY_LOCK_ON;
	} else {
		temp8 = XY_LOCK_OFF;
	}
	if (temp8 != info->lock) {
		info->lock = temp8;
		ret = true;
	}

	// 探头状态
	if (pccud->status & BIT(XY_STATUS_BIT_PROBE)) {
		temp8 = XY_PROBE_ERR;
	} else {
		temp8 = XY_PROBE_OK;
	}
	if (temp8 != info->probe_err) {
		info->probe_err = temp8;
		ret = true;
	}
	
	//mode
	if (pccud->mode <= XY_MODE_HOLIDAY &&
		pccud->mode >= XY_MODE_CONSTTEMP) {
		if (info->mode != pccud->mode) {
			info->mode = pccud->mode;
			ret = true;
		}
	}

	//恒温模式的目标温度
	if (info->cons_temp != pccud->temp1) {
		info->cons_temp = pccud->temp1;
		ret = true;
	}

	//休假模式的目标温度
	if (info->holiday_temp != pccud->temp2) {
		info->holiday_temp = pccud->temp2;
		ret = true;
	}

	//剩余休假小时数
#if 0	
	info->remain_time = pccud->hourl;
	info->remain_time |= (pccud->hourh<<8)&0xff00;
#else
	temp16 = pccud->hourh;
	temp16 |= (pccud->hourl<<8)&0xff00;
	if (temp16 != info->remain_time) {
		info->remain_time = temp16;
		ret = true;
	}
#endif

	//修正参数
	adjust.temp_adj = pccud->adj;
	adjust.temp_top = pccud->top;
	adjust.temp_otemp = pccud->otemp;
	adjust.temp_protlen = pccud->protlen;
	adjust.temp_prottrig = pccud->prottrig;
	adjust.temp_tolr = pccud->tolr;
	if (memcmp((void *)&adjust, (void *)&info->adjust, sizeof(adjust))) {
		memcpy((void *)&info->adjust, (void *)&adjust, sizeof(adjust));
		ret = true;
	}
	//xy_dump(s, info, pccud);
	
    return ret;
}

#if 0
static void xy_mem_dump(char *name, void *ptr, int len)
{
	int index, i;
	char *tmp = ptr;
	char test_buff[10*1024];

	index = 0;
	index += sprintf(test_buff, "smart_temp test recv !!!!!!!!!!!!!!!!!!!!!!\n\n");
	
	for(i = 0; i < len; i++) {
		if (i%16 == 0) {
			index += sprintf(test_buff+index, "\n");
		}
		index += sprintf(test_buff+index, "%02x ", tmp[i]);
	}
	log_debug("name=%s:\n%s\n", name, test_buff);
}

#endif

static bool _xy_update_data_time(cl_xy_info_t *info, ucp_obj_t* obj)
{
//	cl_xy_tp_t *ptp = NULL;
//	ucp_xy_tphd_t *ptphd = NULL;
//	ucp_xy_tpda_t *ptpda = NULL;
	cl_xy_smartmode_info_t *psmart_mode;

	if (obj->param_len < sizeof(cl_xy_smartmode_info_t)) {
		log_debug("%s %d error len=%u sile=%u\n", 
			__FUNCTION__, __LINE__, obj->param_len, sizeof(cl_xy_smartmode_info_t));
		return false;
	}

	psmart_mode = (cl_xy_smartmode_info_t *)(obj+1);

	//memdump("_xy_update_data_time", psmart_mode, sizeof(*psmart_mode));
	if (memcmp((void *)psmart_mode, (void *)&info->smart_mode, sizeof(cl_xy_smartmode_info_t))) {
		memcpy((void *)&info->smart_mode, (void *)psmart_mode, sizeof(cl_xy_smartmode_info_t));
		return true;
	}

	return false;
}

bool _xy_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj)
{
	bool ret = false;
	ucp_xy_tmp_t *pet;
	ucc_session_t *s = NULL;
	cl_xy_info_t *info = air_ctrl->com_udp_dev_info.device_info; 

	if( !info || !obj ){
		return false;
	}

	if (air_ctrl->sac && air_ctrl->sac->user) {
		s = air_ctrl->sac->user->uc_session;
	}

	air_ctrl->com_udp_dev_info.is_stat_info_valid  = true;

	switch(obj->attr){
	case UCAT_IA_THERMOSTAT_PUSH:
		ret = _xy_update_data_push(s, info, obj);
		break;
	case UCAT_IA_THERMOSTAT_TIME:
		ret = _xy_update_data_time(info, obj);
		break;
	case UCAT_IA_THERMOSTAT_EX_TMP:
		if(!is_valid_obj_data(obj,sizeof(*pet))){
			return false;
		}
		pet = (ucp_xy_tmp_t *)(&obj[1]);
		if (info->extern_temp != pet->temp) {
			info->extern_temp = pet->temp;
			ret = true;
		}
		break;
	case UCAT_IA_THERMOSTAT_SMART_HOME:
		if(!is_valid_obj_data(obj,sizeof(*pet))){
			return false;
		}
		pet = (ucp_xy_tmp_t *)(&obj[1]);
		if (air_ctrl->smart_home_enable != pet->temp) {
			air_ctrl->smart_home_enable = pet->temp;
			info->smarthome_onoff = pet->temp;
			ret = true;
		}
		break;
	default:
		break;
	}
	
	return ret;
}



