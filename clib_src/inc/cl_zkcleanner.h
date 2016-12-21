#ifndef CL_ZKCLEANNER_H
#define CL_ZKCLEANNER_H

// 

#ifdef __cplusplus
extern "C" {
#endif 

// cl_zkcleanner_ctrl的action 
enum {
	ACT_ZKCLEANNER_ONOFF = 1,// 开关机1 开 0 关闭
	ACT_ZKCLEANNER_MODE,// 模式 1：自动；2：新风；3：循环?
	ACT_ZKCLEANNER_WIND,// 风量档位1：低速；2：中速；3：高速；
	ACT_ZKCLEANNER_ANTIBIOSIS,// 抗菌0：关闭；1：打开；
	ACT_ZKCLEANNER_FRESH, // 清新0：关闭；1：打开；
	ACT_ZKCLEANNER_MAINTAIN,// 保养标志0：无；1：清除保养标志；
	ACT_ZKCLEANNER_ONTIME,// 开机时间(新修改，value 低16位表示开机时间，高16位如果不为0，表示关机时间)
	ACT_ZKCLEANNER_OFFTIME,// 关机时间
	/*
		 查询一天测量数据，查询成功会收到事件SAE_ZKCLEANNER_DAY_DATA
		 然后读取cl_zkcleanner_info_t里面day_data内容
	*/
	ACT_ZKCLEANNER_QUERY_ONE_DAY_DATA, 
	/*
		 查询最近一月测量数据，查询成功会收到事件SAE_ZKCLEANNER_MONTH_DATA
		 然后读取cl_zkcleanner_info_t里面month_data内容
	*/
	//ACT_ZKCLEANNER_QUERY_ONE_MONTH_DATA, 
};

typedef struct {
	u_int8_t valid;	// 下面数据是否有效
	u_int8_t onoff;	// 开关机1 开 0 关闭
	u_int8_t mode;	// 模式 1：自动；2：新风；3：循环；
	u_int8_t wind;	// 风量档位1：低速；2：中速；3：高速；
	u_int8_t antibiosis;	// 抗菌0：关闭；1：打开；
	u_int8_t fresh;	// 清新0：关闭；1：打开；
	u_int8_t maintain; // 保养标志0：无；1：清除保养标志；
	u_int16_t ontime;	// 开机时间
	u_int16_t offtime;	// 关机时间

	u_int8_t type;	// 型号
	u_int8_t temp;	// 当前温度
	u_int16_t pm25; // 当前PM2.5  单位1ug/m3
	u_int16_t co2;	// 当前CO2浓度，单位1ppm
	u_int16_t hcho;	// 当前甲醛浓度单位1ugm3
	u_int16_t voc;	// 当前VOC浓度，单位1ppm
	u_int8_t aqi;	// 空气质量1：优；2：良；3：差；
	u_int16_t uptime;	// 设备运行时间单位小时0-1440

	// 新加的标志
	u_int8_t on_timer_changed;	// 开机 定时器是否改变
	u_int8_t off_timer_changed;	// 关机定时器是否改变
	u_int8_t ver;	// 版本号，最新是1
} cl_zkcleanner_stat_t;


// 具体的数据 PM2.5和CO2
typedef struct {
	u_int16_t pm25;
	u_int16_t co2;
} cl_zkcleanner_date_t;

typedef struct {
	u_int32_t time;	// 得到该数据的时间戳 UTC
	cl_zkcleanner_date_t items[24];
} cl_zkcleanner_day_data_t;

typedef struct {
	u_int32_t time; // 得到该数据的时间戳 UTC
	cl_zkcleanner_date_t items[31];
} cl_zkcleanner_month_data_t;


typedef struct {
	cl_zkcleanner_stat_t stat;

	/*
		一天24小时的数据，如果中间有停电，或者没有采集的数据都填写0。
	*/
	cl_zkcleanner_day_data_t day_data;

	/*
		记录一个月的数据，从当天往前记录，如果没有记录填零。
		如：第一条是11月15日的，第二条就是11月14日数据，
	*/
	cl_zkcleanner_month_data_t month_data;
} cl_zkcleanner_info_t;

/*
	功能: 对净化器设置报文
		
	输入参数:
		@dev_handle: 设备的句柄
		@action: 控制类型ACT_ZKCLEANNER_XXX，和value配合使用
			
	输出参数:
		无
	返回:
		RS_OK: 成功
		其他: 失败
*/
CLIB_API RS cl_zkcleanner_ctrl(cl_handle_t dev_handle, u_int8_t action, u_int32_t value);

#ifdef __cplusplus
}
#endif 


#endif


