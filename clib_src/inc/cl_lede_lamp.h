#ifndef	__CL_LEDE_LAMP_H__
#define	__CL_LEDE_LAMP_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum{
	ACT_LEDE_CTRL_COLOR, /*控制色盘*/
	ACT_LEDE_CTRL_COL_TEMP,/*控制色温*/
	ACT_LEDE_CTRL_MAX
};

typedef struct{	
	u_int8_t R;			
	/*红 0~255*/	
	u_int8_t	B;		
	/*绿0-255*/
	u_int8_t 	G; 			
	/*蓝0~255*/	
	u_int8_t	L;			
	/*亮度0~100*/
	u_int8_t cold;
	/*色温*/
	u_int8_t	power;		
	/*开关，0为关，1为开*/
	u_int8_t	mod_id; //情景模式编号
	/*模式id*/	
	u_int8_t	action; //0:设置RGB 1:设置色温
	//行为
}cl_lede_led_state_t;

#define	LEDE_LED_TIMER_FLAG_ENABLE	0x01

typedef struct {	
	u_int8_t	id;				
	/*定时器id,有效值从1开始*/	
	u_int8_t	flags;			
	/*bit0:enable*/	
	u_int8_t	week_loop;
	/*bit0代表星期天，bit1代表星期一，等等。为0代表不循环，到期后自动删除*/	
	u_int8_t	hour;			
	/*小时*/
	u_int8_t	min;			
	/*分*/	
	cl_lede_led_state_t	config;	
	/*定时器到期后，使用该设置更新led状态*/
} cl_lede_led_timer_t;

typedef struct {
	u_int8_t valid;	// 下面数据是否有效，作为控制参数不用填
	/**
		 1使能开灯状态配置
               0 不使能用户配置开机状态， 设备默认是显示关机时状态， 并且灯总是开
	*/
	u_int8_t enable;
	/**
		 1 开机时，使用默认的开灯状态，  (设备默认是显示关机时状态， 并且灯总是开)
	        2 开机时，根据用户的设置，显示开灯状态
	        3 开机时，显示关机时候的状态
	*/
	u_int8_t type;
	// 用户配置的开机时候状态
	cl_lede_led_state_t stat;
} cl_lede_led_on_stat_t;

typedef struct {
	u_int8_t timer_count;
	u_int8_t pad[3];
	cl_lede_led_state_t cur_lamp_stat;
	cl_lede_led_on_stat_t on_stat;	// 用户配置开机时候的状态
	cl_lede_led_timer_t* timer_info;	
} cl_lede_lamp_info;

/*
	功能:
		控制灯
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_lede_ctrl_stat(cl_handle_t dev_handle,cl_lede_led_state_t* uconfig);


/*
	功能:
		控制灯开灯后状态
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_lede_on_state_config(cl_handle_t dev_handle, cl_lede_led_on_stat_t *uconfig);

/*
	功能:
		添加和修改lede 定时器,修改时id不为0
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_lede_ctrl_timer(cl_handle_t dev_handle,cl_lede_led_timer_t* tinfo);

/*
	功能:
		删除修改lede 定时器
	输入参数:
		@dev_handle: 插座的句柄
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_lede_delete_timer(cl_handle_t dev_handle,u_int8_t timer_id);
    
///////////////////////////////////////////////////////////////////////////////
//集利舞台灯

//灯类型
enum{
    JL_LAMP_3200 = 0x4,
    JL_LAMP_5400,
    JL_LAMP_ALL_COLOR_TEMP,
    JL_LAMP_ALL_COLOR
};
    
typedef struct {
    u_int8_t color; //色度
    u_int8_t bright; //亮度
    u_int8_t total_bright; //总亮度百分比
    u_int8_t pad;
}cl_jl_3200_lamp_info_t;

typedef struct {
    u_int8_t lamp_type; //见集利灯类型,根据类型对应数据信息
    u_int8_t on_off;
    cl_jl_3200_lamp_info_t lamp_3200_info;
}cl_jl_lamp_info_t;
    
/*
 功能:
    控制集利 3200 舞台灯
 输入参数:
    @dev_handle: 灯的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_jl_ctrl_3200_lamp(cl_handle_t dev_handle,u_int8_t on_off,u_int8_t color,u_int8_t bright);
    
/*
 功能:
	 控制集利 3200 舞台灯
 输入参数:
	 @dev_handle: 灯的句柄
 输出参数:
	 无
 返回:
	 RS_OK: 成功
	 其他: 失败
 */
CLIB_API RS cl_jl_ctrl_3200_total_bright_lamp(cl_handle_t dev_handle,u_int8_t on_off,u_int8_t color,u_int8_t bright,u_int8_t total_bright);

#ifdef __cplusplus
}
#endif 


#endif

