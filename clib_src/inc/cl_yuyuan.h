#ifndef	__CL_YUYUAN_H__
#define	__CL_YUYUAN_H__

#ifdef __cplusplus
extern "C" {
#endif 

/*

错误号	
#define URE_EI_SHORTCUT		20100	// 故障: 开关量输出通道短路 ， 最后一位=通道号，取值范围0~3，21000~21003
			
#define URE_FV_LOC_MAIN_DEV		20201	// 故障: 功能阀(主)定位失败
			
#define URE_FV_LOC_PRE_DEV		20203	// 故障: 功能阀(预)定位失败
			
// task_DrugMotor.c			
#define URE_DM_STATUS		20500	// 故障: 药水泵故障(短路或过热)，最后一位=泵号，取值范围=0~2
			
// task_moni.c			
#define URW_MON_DRUG_MOTOR		10600	// 警告: 药水泵故障
#define URW_MON_WATER_IN_TO		10601	// 警告: 进水流量低或无水持续时间超时
#define URE_MON_SHORTCUT		20600	// 故障: 短路保护
#define URE_MON_FNC_VALVE		20601	// 故障: 功能阀
#define URE_MON_WATER_POS		20602	// 故障: 水位
#define URE_MON_VALVE_IN		20603	// 故障: 进水电磁阀
//#define URE_MON_VALVE_CYC		20604	// 故障: 循环/清洗阀
			
			
// usr_function.c			
#define URE_UF_ACT_FV		20700	// 故障: 执行器设置时功能阀错误
			
#define   URW_LOW_POSITION                		10902	//低水位报警
*/

enum{
    YUYAN_LOOP_OFF = 0x0, // 关
    YUYAN_LOOP_ON = 0x1,  // 开
};
    
enum{
    YUYAN_CLEAN_ACTION_ON = 0x0,  // 开
    YUYAN_CLEAN_ACTION_OFF = 0x1, // 关
};
    
enum{
    YUYAN_WORKSTATE_ON = 0x0, // 开机
    YUYAN_WORKSTATE_STANDBY, // 待机
    YUYAN_WORKSTATE_FILTER,    // 过滤
    YUYAN_WORKSTATE_RECYCLE,   // 循环
    YUYAN_WORKSTATE_CLEAN,     // 清洗
    YUYAN_WORKSTATE_SELFCHECK, // 自检
    YUYAN_WORKSTATE_REBOOTING,  // 正在重启
    YUYAN_WORKSTATE_BACKUP,    //备用
    YUYAN_WORKSTATE_PORJECT    //工程
};

typedef struct {
	// 工作状态，0-待机，1-过滤，2-循环，3-清洗,4-自检，5-正在重启，APP根据该状态的变化来判断上一个工作任务是否完成。注：下位机在收到APP重启命令后，将工作状态置为5即"正在重启"后回复给模组，APP以此可知道净水器是否在重启。
	u_int32_t WORK_STAT;
	// 水位，1-缺，2-低，3-中，4-高
	u_int32_t WATER_LEVEL; 
	// 水位，1-缺，2-低，3-中，4-高
	u_int32_t WATER_BOX; 
	// 用水量对应的脉冲个数,手机端不关心
	u_int32_t WATER_USED_IMPULSE;
	// 进水超时，单位S
	u_int32_t INLET_TIMEOUT;
	// 脉冲个数
	u_int32_t IMPULSE_COUNT;
	// 脉冲检测周期，单位MS
	u_int32_t IMPULSE_PERIOD;
	// 微处理定位延时，单位S
	u_int32_t MCDELAY;
	// 纳米阀定位延时，单位S
	u_int32_t NM_VALVE_DELAY;
	// 功能阀定位超时时间
	u_int32_t FUNC_VALVE_TIMEOUT;

	// 3个部件的转速
	u_int32_t SPEED1;
	u_int32_t SPEED2;
	u_int32_t SPEED3;

	// 循环开关，1-开，0-关
	u_int32_t LOOP_ONOFF;

	// 循环配置，即：在达到循环周期D天H时M分执行TIME分钟循环
	u_int32_t LOOPD;
	u_int32_t LOOPH;
	u_int32_t LOOPM;
	u_int32_t LOOPT; 

	// 微处理自动清洗：天-时-分-反冲时长（MIN）-直洗时长（MIN）
	u_int32_t MCCLEAN_D;
	u_int32_t MCCLEAN_H;
	u_int32_t MCCLEAN_M;
	u_int32_t MCCLEAN_RT;
	u_int32_t MCCLEAN_DT;

	// 纳米自动清洗：天-时-分-反冲时长（MIN）-直洗时长（MIN）
	u_int32_t NMCLEAN_D;
	u_int32_t NMCLEAN_H;
	u_int32_t NMCLEAN_M;
	u_int32_t NMCLEAN_RT;
	u_int32_t NMCLEAN_DT;

	// 错误号，最多存放8个错误，错误号见上面注释URXXX
	u_int32_t ERROR_INFO[8];

	// 工程模式，单片机传过来的数据
	u_int8_t CONFIG[512];
} cl_yuyuan_state_t;

// ATRRI = 1
typedef struct {
	u_int32_t valid;	// had get data ?
	u_int32_t last_write_time;	// UCT
	u_int32_t count;		// 统计的天数，最多365
	u_int16_t data[368];	// 最多有365，写368是为了对齐
} cl_yuyuan_water_history_t;

// ATRRI = 2
typedef struct {
	u_int32_t valid;	// had get data ?
	u_int32_t pwd_len;
	u_int8_t pwd[32];
} cl_yuyuan_pwd_t;

// ATRRI = 3
typedef struct {
	u_int32_t valid;	// had get data ?
	u_int32_t onoff;
	u_int32_t remind_time;	// UTC second
} cl_yuyuan_remind_t;

typedef struct{
	cl_yuyuan_state_t state;
	cl_yuyuan_water_history_t histroy;
	cl_yuyuan_pwd_t pwd;
	cl_yuyuan_remind_t remind;
} cl_yuyuan_info_t;



/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 水箱大小0-大水箱，1-小水箱
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_water_box(cl_handle_t dev_handle,u_int16_t value);

/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 进水超时，单位S，范围0-100
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_inlet_timeout(cl_handle_t dev_handle,u_int16_t value);

/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 设置脉冲个数，
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_impulse_count(cl_handle_t dev_handle,u_int16_t value);


/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 设置脉冲检测周期
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_impulse_period(cl_handle_t dev_handle,u_int16_t value);


/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 微处理定位延时，单位S，默认90，范围0-300
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_mcdelay(cl_handle_t dev_handle,u_int16_t value);

/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 纳米阀定位延时，单位S，默认90，范围0-300
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_nm_valve_delay(cl_handle_t dev_handle,u_int16_t value);


/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 
	输出参数:功能阀定位超时时间，默认120，范围0-300
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_func_valve_timeout(cl_handle_t dev_handle,u_int16_t value);

/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@speed1-3: 3个部件的转速，范围0-100
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_speed(cl_handle_t dev_handle, u_int8_t speed1, u_int8_t speed2, u_int8_t speed3);


/*
	功能:
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 循环开关，1-开，0-关
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_loop_onoff(cl_handle_t dev_handle,u_int16_t value);


/*
	功能:循环时间, 在达到循环周期LPD天LPH时LPM分执行LPT分钟循环
		
	输入参数:
		@dev_handle: 设备的句柄
		@day: 循环天
		@hour: 循环小时
		@min:循环分钟
		@hold_min: 持续分钟
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_loop(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t hold_min);


/*
	功能:微处理自动清洗：天-时-分-反冲时长（MIN）-直洗时长（MIN）
		
	输入参数:
		@dev_handle: 设备的句柄
		@day: 循环天
		@hour: 循环小时
		@rt_min:反冲时长
		@dt_min: 直洗时长
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_mcclean(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t rt_min, u_int8_t dt_min);


/*
	功能:纳米自动清洗：天-时-分-反冲时长（MIN）-直洗时长（MIN）
		
	输入参数:
		@dev_handle: 设备的句柄
		@day: 循环天
		@hour: 循环小时
		@min: 循环分钟
		@rt_min:反冲时长
		@dt_min: 直洗时长
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_nmclean(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t rt_min, u_int8_t dt_min);

/*
	功能:立即开始微处理清洗
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 0 开始 1 结束
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_mincro_clean(cl_handle_t dev_handle, u_int8_t action);


/*
	功能:立即开始纳米清洗
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 0 开始 1 结束
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_nm_clean(cl_handle_t dev_handle, u_int8_t action);


/*
	功能:立即开始自检。自检过程中模组会获取到ERROR，并上报到APP。
		
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_check_self(cl_handle_t dev_handle);


/*
	功能:立即重启净水器
		
	输入参数:
		@dev_handle: 设备的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_reboot_cleanner(cl_handle_t dev_handle);


/*
	功能:工程模式命令，APP上有一个工程模式输入框，输入框内的内容替换掉VALUE，发送给下位机，方便扩充。VALUE的长度限制为50字节内。
		
	输入参数:
		@dev_handle: 设备的句柄
		@cmd: 自定义AT命令
		@cmd_len: 命令长度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_config(cl_handle_t dev_handle, u_int8_t *cmd, u_int8_t cmd_len);

/*
	功能:设置一个密码，密码长度小于50字节
		
	输入参数:
		@dev_handle: 设备的句柄
		@value: 密码
		@value_len: 密码长度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_set_pwd(cl_handle_t dev_handle, u_int8_t *value, u_int8_t value_len);

/*
	功能:设置提醒信息
		
	输入参数:
		@dev_handle: 设备的句柄
		@onoff: 0 关闭提醒 1 开启提醒
		@value_len: 密码长度
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_yuyuan_set_remind(cl_handle_t dev_handle, u_int32_t onoff, u_int32_t remind_time);


#ifdef __cplusplus
}
#endif


#endif


