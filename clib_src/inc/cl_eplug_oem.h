#ifndef __CL_EPLUG_OEM_H__
#define __CL_EPLUG_OEM_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    

typedef struct{
    u_int8_t onoff; //开关
    u_int8_t room_temp; //当前室温
    ///////////////////
    u_int8_t range_enable; //室温范围enable
    u_int8_t range_max_temp; //室温高值
    u_int8_t range_min_temp;
    ///////////////////////////
    u_int8_t temp_threshold_enable; //温度阀值开关
    u_int8_t temp_threshold_value; //温度阀值
    //////////////////////////
    u_int8_t off_line_close_enable; //离线关闭功能
    //////////////////////////
    u_int8_t is_support_person_detect; //是否支持人体探测
    u_int8_t person_detect_enable; //人体探测是否开启
}cl_eplug_oem_stat;
    
/*
 功能:
    控制开关
 输入参数:
    @dev_handle: 插座的句柄
    @is_on: 0-关机，1－开机
    输出参数:
 无
    返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_eo_set_onoff(cl_handle_t dev_handle, bool is_on);
    
    
/*
 功能:
    控制恒温范围
 输入参数:
    @dev_handle: 插座的句柄
 
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_eo_set_temp_range(cl_handle_t dev_handle, bool enable,u_int8_t max_temp,u_int8_t min_temp);

/*
 功能:
    控制关闭插座的室温阀值
 输入参数:
    @dev_handle: 插座的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_eo_set_threshold(cl_handle_t dev_handle, bool enable,u_int8_t max_temp);
    
/*
 功能:
    插座离线关机设置
 输入参数:
    @dev_handle: 插座的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_eo_set_off_line_close_enable(cl_handle_t dev_handle, bool enable);

/*
 功能:
    插座人体探测功能
 输入参数:
    @dev_handle: 插座的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_eo_set_person_detect_enable(cl_handle_t dev_handle, bool enable);
    
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_CAR_H */

