#ifndef	__CL_YJ_HEADER_H
#define	__CL_YJ_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif 


#pragma pack(push,1)

typedef struct {
	// 上层无视
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t cmd;
	u_int8_t dev_type;
	u_int8_t param_len;
	// 上层无视

	// 状态只看这里
	u_int8_t onoff; // 0 off 1 on
	u_int8_t temp_set;// 设置温度5 - 36
	u_int8_t temp_type; // 温度类型 0 摄氏度 1 华氏度
	u_int8_t anion;	// 负离子
	u_int8_t power;// 0 low 1 high
	u_int8_t child_lock; // 童锁 0  关 1 开
	int8_t temp_now;	// 当前室温，可能为负数
	u_int8_t timer;   // 定时器 达到设置时间后自动关机 0-24
	u_int8_t  err_code;	
	u_int8_t process;	//  0---不闪烁  1---较慢闪烁  2---较快闪烁
	// 状态只看这里
	
	// 上层无视
	u_int8_t pad1[4];
	u_int8_t  check_sum;
} cl_yj_heater_info_t;

typedef struct {
	// 上层无视
	u_int8_t syn1;
	u_int8_t syn2;
	u_int8_t cmd;
	u_int8_t pad;
	u_int8_t param_len;
	// 上层无视

	// 状态只看这里
	u_int8_t onoff;	// 0 off 1 on
	u_int8_t temp_set;	// 设置温度5 - 36
	u_int8_t temp_type;	// 温度类型 0 摄氏度 1 华氏度
	u_int8_t anion;	// 负离子1 开
	u_int8_t power;// 功率 0 low 1 high
	u_int8_t child_lock; // 童锁 0  关 1 开
	u_int8_t timer;		// 定时器 达到设置时间后自动关机 0-24
	// 状态只看这里

	u_int8_t pad1[3];
	u_int8_t checksum;
} cl_yj_heater_set_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 



/*
 功能：
 	益佳温控器控制
 输入参数:
 	@handle: 设备句柄
 	@ctrl: 控制参数
 输出参数:
 	无
 返回：
 	RS_OK:
 	其他: 失败
 事件通知:
 
 */
CLIB_API RS cl_yj_heater_set_ctrl(cl_handle_t dev_handle, cl_yj_heater_set_t *ctrl);


#endif
