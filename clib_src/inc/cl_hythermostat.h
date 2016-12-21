#ifndef CL_HYTHERMOSTAT_H
#define CL_HYTHERMOSTAT_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

// cl_hythermostat_ctrl的action 
enum {
	ACT_HYTHERMOSTAT_ONOFF = 1,// 开关机1 开 0 关闭  (通用)
	ACT_HYTHERMOSTAT_TEMP,// 0.1℃  50~600 (通用)
	ACT_HYTHERMOSTAT_MODE,// 空调模式 1：制冷；2：制热；3：送风 // 水暖模式 1：水制冷；2：水采暖；
    ACT_HYTHERMOSTAT_WIND,// 风量档位 0: 停风；1：低风；2：中风；3：强风；(仅空调用)

    ACT_HYTHERMOSTAT_RHFUN,//结露侦测功能 0: 关闭； 1: 打开；(仅空调用)
    ACT_HYTHERMOSTAT_RHVAL,//结露侦测阈值 1%  10~90 (仅空调用)
    
};

// cl_hythermostat
typedef struct {
    u_int16_t mcuver;   //OR  下位机软件版本
    u_int16_t type;	    //OR  设备类型，0-空调机型，1-水暖机型
    u_int16_t temp;	    //OR  当前室内温度 0.1摄氏度
    u_int16_t valve;    //OR  空调：0-阀关闭，1-阀开启；水暖：0-执行器关，1-执行器开；
    u_int16_t onoff;	//WR 开关机1 开 0 关闭 (通用)========
    u_int16_t settemp;  //WR 设置温度 0.1℃  50~600 (通用)=========
    u_int16_t mode;	    //WR 空调模式 1：制冷；2：制热；3：送风 // 水暖模式 1：水制冷；2：水采暖；============
    u_int16_t wind;  	//WR 风量档位 0: 停风；1：低风；2：中风；3：强风；(仅空调用)=========
    u_int16_t RHfun;	//WR 结露侦测功能 0: 关闭； 1: 打开；(仅空调用)=====
    u_int16_t RHval;    //WR 结露侦测阈值 1%  10~90 (仅空调用)========
    u_int16_t RHstate;  //OR 结露侦测状态 0: 未报警；1: 处于报警状态

} cl_hythermostat_stat_t;


typedef struct {
	cl_hythermostat_stat_t stat;

} cl_hythermostat_info_t;



/*
	功能: 对温控器设置报文
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_HYTHERMOSTAT_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_hythermostat_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);



#ifdef __cplusplus
}
#endif 


#endif


