#ifndef	__CL_ENV_MON_H__
#define	__CL_ENV_MON_H__


#ifdef __cplusplus
extern "C" {
#endif 

// 环境监测，目前主要包括温度和空气
typedef struct {
	// 名字。可能是手机号，也可能是序列号
	char *name;
	// 该设备的序列号
	u_int64_t sn;

	// 获取到空气质量了
	bool get_air_done;
	// 获取到天气信息了
	bool get_weather_done;
	// 获取到室内环境建议了
	bool get_suggest_done;
	
	// AQI
	int aqi;
	// pm2.5
	int pm25;
	// 空气质量: 良，
	char *air_quality;
    // 获取到空气质量的时间
    char* aqi_last_update_time;
	
	// 该设备所在的城市。在查询天气的时候一起回来的
	char *city;
	// 当前温度
	int temperature;
	// 当前湿度
	int humidity;
	// 当日最低温度
	int temp_min;
	// 当日最高温度
	int temp_max;
	// 天气: 多云
	char *weather;
	// 获取到天气的时间
	char* wear_last_update_time;
	// 温度建议
	char *suggest_temp;
	// 湿度建议
	char *suggest_humidity;
	// 空气建议
	char *suggest_air;	
} cl_env_mon_t;


typedef struct {
	// 省份名字
	char *name;
	// 该省有多少个城市
	int city_num;
	// 城市列表
	char **city;
} cl_province_t;

typedef struct {
	// 有多少个省份
	int province_num;
	// 省份列表
	cl_province_t **province;
} cl_city_list_t;

typedef struct{
	//根据室内环境获取建议参数
	int temp; // 温度
	int humidity; //湿度
	int voc;
	int pm25;
	int pm10;
	int no2;
	int so2;
	int o3;
	int co;
}env_air_t;

/*
	获取某个设备所在城市的环境参数
*/
CLIB_API cl_env_mon_t *cl_em_get_info(cl_handle_t user_handle);
/*
	释放cl_em_get_info返回的数据
*/
CLIB_API void cl_em_free_info(cl_env_mon_t *cl);


/*
	获取目前支持的城市列表
*/
CLIB_API cl_city_list_t *cl_em_get_city_list();
CLIB_API void cl_em_free_city_list(cl_city_list_t *cl);

/*
	设置设备当前所在的城市，必须是支持的城市列表里面的一个
*/
CLIB_API RS cl_em_set_city(cl_handle_t user_handle, const char *city);

/*
	获取室内环境建议
	输入参数:
		user_handle @ 用户句柄
		env @ 当前室内环境参数
	输出参数:
		无
	关注事件:
	UE_ENV_WEATHER_SUGGEST 调用cl_em_get_info
*/
CLIB_API RS cl_em_get_suggest(cl_handle_t user_handle, const env_air_t *env);

#ifdef __cplusplus
}
#endif 

#endif


