/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_qpcp.h
**  File:    cl_qpcp.h
**  Author:  liubenlong
**  Date:    05/12/2015
**
**  Purpose:
**    千帕茶盘.
**************************************************************************/


#ifndef CL_QPCP_H
#define CL_QPCP_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
#define 		CL_QPCP_NAME_MAX_LEN		32
    
#define CL_QPCP_CUSTOMIZE_SCENE_MIN_ID 5000 // 自定义情景起始ID号

/* Type definitions. */
//工作模式
/*
工作模式：
以下每位为1表示动作正在进行，0表示没有进行。
BIT0：开机（1为已经是开机状态，0为待机）
BIT1：烧水壶正在加水
BIT2：烧水壶正在煮水
BIT3：烧水壶正在保温
BIT4：消毒锅正在加水
BIT5：消毒锅正在煮水
BIT6：消毒锅正在保温
*/
enum {
	//待机
	QPCP_MODE_WAIT = 0,
	//自动泡茶
	QPCP_MODE_AUTO_TEA,
	//消毒
	QPCP_MODE_DISINFECT,
	//烧水
	QPCP_MODE_HEAT_WATER,
	//自动烧水
	QPCP_MODE_AUTO_HEAT_WATER,
	//自动消毒
	QPCP_MODE_AUTO_DISINFECT,
	//加水
	QPCP_MODE_ADD_WATER,
};

//u_int8_t mode; 工作模式:1：手动 2: 自动
enum {
    //煮水
    QPCP_MODE_MANUAL = 0X01,
    //消毒
    QPCP_MODE_AUTO = 0X02,
};
    
//错误码信息
enum {
	QPCP_ERR_NONE = 0X00,
	//提壶操作或温度探头开路
	QPCP_ERR_TH = 0X01,
	//温度探头短路操作
	QPCP_ERR_DL = 0X02,
	//温度探头高温操作
	QPCP_ERR_GW = 0X03,
	//煮水壶内无水状态
	QPCP_ERR_NO_WATER = 0X04,
	//旋转龙头信号异常
	QPCP_ERR_LTE = 0X05,
};

//制作状态
enum {
	//待机
	QPCP_PRODUCT_WAIT = 0X00,
	//制作中
	QPCP_PRODUCT_DOING = 0X01,
	//制作完成
	QPCP_PRODUCT_FINISHED = 0X02,
};

//action操作
enum {
	//煮水
	QPCP_ACTION_BOIL = 0X01,
	//消毒
	QPCP_ACTION_DISINFECT = 0X02,
};

//加水龙头状态
enum{
	QPCP_WATER_STATE_NONE = 0X0,//水龙头静置
	QPCP_WATER_STATE_BOIL_ADDING,//煮水加水中
	QPCP_WATER_STATE_BOIL_BACKING,//煮水归位中	
	QPCP_WATER_STATE_DISINFECT_ADDING,//消毒加水中
	QPCP_WATER_STATE_DISINFECT_BACKING,//消毒归位中	
};

//字节对齐
#pragma pack(push, 1)

//情景通用配置 cl_qpcp_scene_com_param_t
typedef struct {
	u_int8_t action;	//煮水或消毒
	u_int8_t temp;	//设定温度(90-100)
	u_int8_t thermal_temp;	//保温温度保温温度(40-100)
	u_int8_t thermal_time;	//保温时间保温时间(1-8 hours)
	u_int8_t time;	//设定时间时间 (1-240 分钟)
	u_int8_t power;	//设定功率(1-9)表示100-900w
	u_int8_t water_time;	//加水时间(5-30s)
	u_int8_t index; //烧水或消毒内部index
}cl_qpcp_scp_t;

//完整情景配置结构
typedef struct {
	u_int16_t scene_id;	//id
	u_int16_t pad;
	cl_qpcp_scp_t com_param;	
	u_int32_t create_time;	//创建时间
	u_int8_t name[CL_QPCP_NAME_MAX_LEN];	//名称
}cl_qpcp_sp_t;

#pragma pack(pop)


//cl_qpcp_scene_hd_t，情景链表查询头部
typedef struct {
	u_int8_t num;
	cl_qpcp_sp_t scene[0];
}cl_qpcp_sh_t;

//当前煮水消毒配置状态
typedef struct {
	u_int8_t temp;	//温度设置(90-100)
	u_int8_t power;	//功率设置(100-900w)
	u_int8_t time;	//时间 (1-240 分钟)
	u_int8_t thermal_temp;	//保温温度(40-100)
	u_int8_t thermal_time;	//保温时间(1-8 hours)
}cl_qpcp_config_t;

typedef struct {
	
	cl_qpcp_config_t disinfect;	//当前消毒配置数据
	cl_qpcp_config_t boil;	//当前煮水配置数据

    u_int8_t cur_onof;	//开关机状态
    u_int8_t cur_mode;	//当前工作模式
	u_int8_t cur_water_temp;	//当前煮水温度
	u_int8_t cur_water_time;	//当前煮水消毒加水时间
    
	u_int8_t cur_water_state;	//当前加水状态
	u_int8_t cur_error;	//当前错误码
	u_int8_t cur_remain_water_time;	//当前剩余加水时间
	u_int8_t cur_production_status;	//当前制作状态
    
    u_int16_t disinfect_plan_id; //当前正在执行的消毒情景ID
    u_int16_t boil_plan_id; //当前正在执行的煮水情景ID
    
    u_int8_t disinfect_index; //当前正在执行的消毒序号
    u_int8_t boil_index; //当前正在执行的煮水序号
    u_int8_t mode; //工作模式:1：手动 2: 自动
    u_int8_t water_warning; //加水警告
    
	cl_qpcp_sh_t *pscene;	//链表

}cl_qpcp_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	功能:
		开关
	输入参数:
		@dev_handle: 设备的句柄
		@onoff:开关，0关，1开
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_qpcp_ctrl_on(cl_handle_t dev_handle, u_int8_t onoff);

/*
	功能:
		加水操作
	输入参数:
		@dev_handle: 设备的句柄
		@action:煮水或消毒,1煮水,2消毒
		@time:加水时间，(5-30s)秒数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_qpcp_add_water(cl_handle_t dev_handle, u_int8_t action, u_int8_t time);

/*
	功能:
		手动煮水消毒操作
	输入参数:
		@dev_handle: 设备的句柄
		@param:手动煮水消毒操作参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_qpcp_handle_ctrl(cl_handle_t dev_handle, cl_qpcp_scp_t *param);


/*
	功能:
		情景删除，因为参数差别大，删除添加用不同接口
	输入参数:
		@dev_handle: 设备的句柄
		@id: 情景id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_qpcp_scene_del(cl_handle_t dev_handle, u_int16_t id);

/*
	功能:
		情景修改
	输入参数:
		@dev_handle: 设备的句柄
		@param: 情景参数
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_qpcp_scene_modify(cl_handle_t dev_handle, cl_qpcp_sp_t *param);

/*
	功能:
		情景执行
	输入参数:
		@dev_handle: 设备的句柄
		@id: 情景id
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_qpcp_scene_execute(cl_handle_t dev_handle, u_int16_t id, cl_qpcp_scp_t *param);
    
/*
 功能:
    开关
 输入参数:
    @dev_handle: 设备的句柄
    @onoff:开关，0关，1开
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_qpcp_reset_fault(cl_handle_t dev_handle);
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 千帕锅
    
/**********************************
 千帕锅错误码
 0—无错误
 1--探头1开路保护：当探头1开路，报警并停止功率输出。彩屏显“E1”闪亮。
 2--探头2开路保护：当探头2开路，报警并停止功率输出。彩屏显“E3”闪亮。
 3--探头1短路保护：当探头1短路，报警并停止功率输出。彩屏显“E2”闪亮。
 4--探头2短路保护：当探头2短路，报警并停止功率输出。彩屏显“E4”闪亮。
 5--锅底超温保护：当探头检测到温度超过200度时，报警并停止功率输出。彩屏显“E5”闪亮。
 6--电压过低保护：当电压低于100V时，报警并停止功率输出。彩屏显“E7”闪亮。
 7--电压过高保护：当电压高于275V时，报警并停止功率输出。彩屏显“E8”闪亮。
 8--散热器热敏电阻开路保护：当散热器热敏电阻开路时，报警并停止功率输出。彩屏显“E9”闪亮。
 9--散热器热敏电阻短路保护：当散热器热敏电阻短路时，报警并停止功率输出。彩屏显“E10”闪亮。
 10--散热器热敏电阻超温保护：当散热器热敏电阻超温时，报警并停止功率输出。彩屏显“E11”闪亮。
******************************/
enum{
    QPP_STAT_IDLE, //待机
    QPP_STAT_COOKING, //烹饪
    QPP_STAT_KEEP_TEMP,//保温
    QPP_STAT_WAIT_EXEC_TASK,//待执行预约任务
    QPP_STAT_MAX
};
    
enum{
    QPP_MODE_UNKNOWN, //未知
    QPP_MODE_RICE,//煮饭
    QPP_MODE_GUREL,//熬粥
    QPP_MODE_SOUP,//煲汤
    QPP_MODE_STEW,//焖炖
    QPP_MODE_BAKING, //烘培
    QPP_MODE_MAX
};
    
    
typedef struct {
    u_int16_t s_id; //情景id
    u_int16_t cook_time; //烹饪时间
    u_int8_t hot_degress;//加热强度（等级）
    u_int8_t microswitch; //微动开关次数
    u_int8_t warm_temp; //保温温度
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
    u_int8_t set_action; //1 启动 0：取消
    u_int8_t rice_degress; //米饭软硬度
}cl_qp_pot_scene_param_t;
    
typedef struct {
    u_int8_t name[CL_QPCP_NAME_MAX_LEN];
    u_int32_t create_time; //创建时间
    cl_qp_pot_scene_param_t s_info; //参数
}cl_qp_pot_scene_t;
    
typedef struct {
    u_int16_t cur_id;
    u_int16_t cooking_remain_time;
    u_int8_t stat; //状态，见 QPP_STAT_XXX
    u_int8_t cooking_mode; //烹饪类型 见 QPP_MODE_XXX
    u_int8_t is_complete; //是否烹饪完成
    u_int8_t is_pot_cover_open; //锅盖是否开启
    u_int8_t microswitch; //微动开关
    u_int8_t warm_temp; //保温温度
    u_int8_t pot_temp; //锅内温度
    u_int8_t food_quantity;// 食物量
    u_int8_t err_num; //错误码
    u_int8_t scene_count; //自定义情景个数
    cl_qp_pot_scene_t* scenes; //自定义情景
}cl_qp_pot_info_t;

    
/*
 功能:
    控制锅启动
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_qp_pot_ctrl(cl_handle_t dev_handle, cl_qp_pot_scene_param_t* param);

/*
 功能:
    执行情景
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_qp_pot_exec_scene(cl_handle_t dev_handle, cl_qp_pot_scene_param_t* param);
    

/*
 功能:
    情景删除，因为参数差别大，删除添加用不同接口
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_qp_pot_del_scene(cl_handle_t dev_handle, u_int16_t s_id);

/*
 功能:
    情景修改
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_qp_pot_modify_scene(cl_handle_t dev_handle, cl_qp_pot_scene_t *scene);
    
////////////////////////////////////////////////////////////////////
//千帕破壁机

/*
 0x00:正常状态
 0x01提壶操作或温度探头开路
 0x02温度探头短路操作
 0x03温度探头高温操作
 */
enum{
    QP_PBJ_ERR_NONE = 0x0,
    QP_PBJ_ERR_NO_POT,
    QP_PBJ_ERR_TEMP_DETECT,
    QP_PBJ_ERR_HIGH_TEMP
};
    
#define QP_MODE_HOT 0x1
#define QP_MODE_MIX 0x2
    
#define MAX_QP_PBJ_STEP 0x9

typedef struct {
    u_int8_t is_data_valid; //数据是否有效
    u_int8_t on_off; //开关
    u_int16_t cur_exec_id; //当前执行的情景
    u_int8_t work_stat; // 0:待机: 1. 制作中 2. 制作完成
    u_int8_t cur_mode; //当前模式
    u_int8_t mix_power; //搅拌功率 1-9
    u_int8_t temp; //温度
    u_int8_t err_no; //错误状态
}cl_qp_pbj_stat_info_t;

typedef struct {
    u_int8_t temp; //加热温度
    u_int8_t time; //加热时间 单位：分钟
}cl_qp_pbj_hot_info_t;

typedef struct {
    u_int8_t gear; //搅拌档位;
    u_int8_t time; // 1-20 单位: 秒
    u_int8_t freq; // 搅拌次数 当step_count小于等于5时，范围为1-3，当step_count大于5时，范围为1-10
}cl_qp_pbj_mix_info_t;
    
typedef struct{
    u_int8_t data_type; //表明使用哪类数据，2选1  1：加热  2：搅拌
    cl_qp_pbj_mix_info_t m_info;
    cl_qp_pbj_hot_info_t h_info;
}cl_qp_pbj_action_t;

typedef struct {
    u_int16_t scene_id;
    char name[CL_QPCP_NAME_MAX_LEN];
    u_int8_t step_count;
    u_int8_t warm_time;//保温时间
    u_int8_t warm_temp;//报文温度
    cl_qp_pbj_action_t action[MAX_QP_PBJ_STEP];
}cl_qp_pbj_scene_t;
    
typedef struct{
    cl_qp_pbj_stat_info_t stat;
    u_int32_t scene_num;
    cl_qp_pbj_scene_t* scene;
}cl_qp_pbj_info_t;

/*
 功能:
    控制破壁机启动
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_qp_pbj_ctrl_onoff(cl_handle_t dev_handle, u_int8_t on_off);
    
/*
 功能:
    控制破壁机执行或删除情景
 输入参数:
    @dev_handle: 设备的句柄
    @action : 0:执行 1：删除 2：停止
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_qp_pbj_ctrl_scene(cl_handle_t dev_handle, u_int8_t aciton, u_int16_t scene_id);
    
/*
 功能:
    控制破壁机修改情景
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_qp_pbj_modify_scene(cl_handle_t dev_handle, cl_qp_pbj_scene_t* scene);
    
/*
 功能:
    清除错误状态信息
 输入参数:
    @dev_handle: 设备的句柄
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_qp_pbj_reset_fault(cl_handle_t dev_handle);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_QPCP_H */

