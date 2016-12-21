#ifndef	__CL_TB_HEATER_PUMP_H__
#define	__CL_TB_HEATER_PUMP_H__

#ifdef __cplusplus
extern "C" {
#endif 


typedef struct{
	/* 从机下控ID */    
	u_int8_t cid;    
	/* 开关机 */    
	u_int8_t onoff;   
	/* 工作模式 */   
	u_int8_t work_mode;   
	/* 温度值 */    
	u_int8_t temp;    
	/* 预留 */    
	u_int8_t reserved[4];
}cl_tb_user_config_t;


typedef struct  {
	 /* 从机下控ID */    
	u_int8_t cid;    
	/* 冷媒回开关 */    
	u_int8_t return_cold_switch;    
	/* 设备安装状态 */    
	u_int16_t facility_state;  
	/* 系统功能选择 */   
	u_int16_t sysfunc;    
	/* 回差温度 */    
	u_int8_t return_diff_temp;    
	/* 制热除霜周期 */    
	u_int8_t heat_defrost_circle;    
	/* 制热进入除霜温度 */   
	u_int8_t start_heat_defrost_temp;    
	/* 制热退出除霜温度 */    
	u_int8_t stop_heat_defrost_temp;    
	/* 制热退出除霜时间 */    
	u_int8_t stop_heat_defrost_time;    
	/* 电加热启动设定值 */    
	u_int8_t eheat_value;    
	/* 背光延时关闭时间 */    
	u_int8_t backlight_delay_time;    
	/*风向模式*/
	u_int8_t fan_mode;
	u_int8_t reserved[2];
}cl_tb_work_config_t;
	

typedef struct  {
	/*从机下控ID*/
	u_int8_t cid;
	u_int8_t mode;   /* 0：手动，1：自动 */
	u_int8_t year;   /* 0 ~ 99 (2000 ~ 2099)*/
	u_int8_t month;  /* 1 ~ 12 */
	u_int8_t mday;   /* 1 ~ 31 */
	u_int8_t hour;   /* 0 ~ 23 */
	u_int8_t minute; /* 0 ~ 59 */
	u_int8_t second; /* 0 ~ 59 */
}cl_tb_rtc_t;

typedef struct {
	/* 从机下控ID */    
	u_int8_t cid;   
	/* 环境温度 */    
	u_int8_t env_temp;    
	/* 水箱下部温度 */    
	u_int8_t tankbottom_temp;    
	/* 水箱上部温度 */   
	u_int8_t tanktop_temp;   
	/* 盘管温度 */  
	u_int8_t coil_temp;   
	/* 排气温度 */  
	u_int8_t air_temp;    
	/* 回气温度 */    
	u_int8_t returnair_temp;  
	/* 出水温度 */   
	u_int8_t outwater_temp;  
	/* 进水温度 */   
	u_int8_t inwater_temp;   
	/* 回水温度 */    
	u_int8_t returnwater_temp;   
	/* 制热量 */    
	u_int16_t heat_capacity;   
	/* 制热时间 */   
	u_int16_t heat_time;   
	/* 耗电量 */   
	u_int16_t consumption_power
	;    /* 节省电量 */   
	u_int16_t saving_power;  
	u_int16_t reserved;
}cl_tb_temp_info_t;

typedef struct {
	/* 从机下控ID */  
	u_int8_t cid;   
	/* 电子阀开度 */  
	u_int8_t valve_expansion; 
	/* 从机设备上电状态 */   
	u_int16_t slave_onoff;   
	/* 设备/故障1 */  
	u_int16_t dev_fault;   
	/* 设备/故障2 */   
	u_int16_t dev_guard;  
	u_int16_t load_state;
	u_int16_t reserved;
}cl_tb_fault_stat;
    
#define TB_ERR_NONE   (0)
    /* 升级成功 */
#define TB_ERR_OK    (127)
    /* 未联网 */
#define TB_ERR_NET   (128)
    /* 镜像下载失败 */
#define TB_ERR_DOWN  (129)
    /* 镜像校验失败 */
#define TB_ERR_IMG   (130)
    /* 以上的错误可以重试, 下面的错误联系厂商处理 */
    /* 升级线控失败 */
#define TB_ERR_MCU0  (131)
    /* 升级电控板失败 */
#define TB_ERR_MCU1  (132)

typedef struct {
	/*从机下控ID*/
	u_int8_t cid;
	/* 机型信息 */
	u_int8_t dev_info;
	/* 模式选择 */
	u_int8_t dev_mode;
	/* 固件版本号 */
	u_int8_t fw_version;
	/* 主控板固件版本号 */
	u_int8_t mb_version;
	u_int8_t svn_version;
    u_int8_t stm_up_stat; //单片机升级状态，见上
	u_int8_t reserved;
}cl_tb_other_info;

typedef struct{
	u_int16_t  bind_state;  
	u_int8_t tb_sn[24]; 
	cl_tb_user_config_t u_config;
	cl_tb_work_config_t w_config;
	cl_tb_temp_info_t temp_info;
	cl_tb_fault_stat fault_stat;
	cl_tb_other_info other_info;	
}cl_tb_info_t;

/*
	功能:
		设置状态
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tb_ctrl_stat(cl_handle_t dev_handle,cl_tb_user_config_t* uconfig);

/*
	功能:
		设置工作参数
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tb_setting_work_param(cl_handle_t dev_handle,cl_tb_work_config_t* wconfig);

/*
	功能:
		刷新温度信息
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tb_refresh_temp_info(cl_handle_t dev_handle);

/*
	功能:
		刷新杂项信息
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tb_refresh_other_info(cl_handle_t dev_handle);

/*
	功能:
		绑定条形序列号
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_tb_bind_dev_bar_code(cl_handle_t dev_handle,u_int8_t* bar_code);

#ifdef __cplusplus
}
#endif 


#endif

