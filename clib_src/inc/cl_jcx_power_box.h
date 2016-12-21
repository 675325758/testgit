#ifndef	__CL_JCX_POWER_BOX_H__
#define	__CL_JCX_POWER_BOX_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define MAX_JCX_CHANNEL  0x10

typedef struct {
	u_int16_t voltage; //电压 单位0.001V
	u_int16_t elec;  //电流 单位0.001A
	int32_t active_power;	// 有功功率 单位0.1W, 有符号数，APP 或JNI转换
	int32_t reactive_power; // 无功功能 单位 0.1 乏尔,有符号数，APP 或JNI转换
	int16_t power_factor;	// 功率因素，单位0.001  , 有符号数，APP 或JNI转换
	u_int16_t frequency; //频率
	u_int32_t active_degree;	// 有效电度 单位千瓦时
	u_int32_t reactive_degree;	// 无效电度 单位千乏尔时
	u_int32_t jcx_sn;				// 序列号
	u_int16_t jcx_soft_ver;			// 软件版本
	u_int16_t jcx_hardware_ver;		// 硬件版本
	u_int8_t channel_num; //通道数
	u_int8_t pad;
	u_int16_t channelstat; //通道通段状态
	char* channel_names[MAX_JCX_CHANNEL]; //通道名称
} cl_jcx_power_box_info;

#ifdef __cplusplus
}
#endif 


#endif

