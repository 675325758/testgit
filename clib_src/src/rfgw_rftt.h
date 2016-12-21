#ifndef	__RFGW_RFTT_H__
#define	__RFGW_RFTT_H__

#ifndef	BIT
#define	BIT(n)	(1<<(n))
#endif

/*通用指令*/
enum{
    RF_CT_UNKNOWN = 0x0,
    RF_CT_WORK_STAT = 0x1, //设备状态
    RF_CT_DBG_INFO = 0x2,//调试信息，包括在线时间等
    RF_CT_CTRL_ON_OFF = 0x3,//控制开启、关闭，支持简单控制的设备才支持，比如灯
    RF_CT_CTRL_GUARD = 0x4,//布防撤防,报警类设备支持
};


#pragma pack(push,1)
/////////////////2.4G RF transparent translation protocol////////////////////
/*
由于2.4G RF 通信载荷最多32字节，需要节约使用
RF 报文长度已知，透传命令就不带长度了
透传命令后面一般直接跟参数
*/

//控制工作状态请求，参数1字节，低3位分别表示RGB灯
#define RFTT_WORK_REQ 1
//控制工作状态响应，参数1字节，低3位分别表示RGB灯
#define RFTT_WORK_ACK 2
//RF设备调试信息，参数rfdev_dbg_t
#define RFTT_DBG_INFO 3
//查询工作状态，无参数，设备回应RFTT_WORK_ACK
#define RFTT_WORK_QUERY (4)

//RGB三色灯状态位
#define RGB_WORK_R BIT(0)
#define RGB_WORK_G BIT(1)
#define RGB_WORK_B BIT(2)
//门磁状态比特位，1表示开门，0表示关门
#define MC_WORK_OPEN BIT(3)

//RF设备调试信息
typedef struct{
	u_int32_t rfrx;//收到的RF帧个数
	u_int32_t uptime;//启动后时间秒
	u_int32_t linktime;//连接到网关后时间秒
	u_int16_t linkretry;//连接到网关次数
}rfdev_dbg_t;


#pragma pack(pop)

#endif

