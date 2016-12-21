#ifndef	__CL_ENV_MON_H__
#define	__CL_ENV_MON_H__


#ifdef __cplusplus
extern "C" {
#endif 

// ������⣬Ŀǰ��Ҫ�����¶ȺͿ���
typedef struct {
	// ���֡��������ֻ��ţ�Ҳ���������к�
	char *name;
	// ���豸�����к�
	u_int64_t sn;

	// ��ȡ������������
	bool get_air_done;
	// ��ȡ��������Ϣ��
	bool get_weather_done;
	// ��ȡ�����ڻ���������
	bool get_suggest_done;
	
	// AQI
	int aqi;
	// pm2.5
	int pm25;
	// ��������: ����
	char *air_quality;
    // ��ȡ������������ʱ��
    char* aqi_last_update_time;
	
	// ���豸���ڵĳ��С��ڲ�ѯ������ʱ��һ�������
	char *city;
	// ��ǰ�¶�
	int temperature;
	// ��ǰʪ��
	int humidity;
	// ��������¶�
	int temp_min;
	// ��������¶�
	int temp_max;
	// ����: ����
	char *weather;
	// ��ȡ��������ʱ��
	char* wear_last_update_time;
	// �¶Ƚ���
	char *suggest_temp;
	// ʪ�Ƚ���
	char *suggest_humidity;
	// ��������
	char *suggest_air;	
} cl_env_mon_t;


typedef struct {
	// ʡ������
	char *name;
	// ��ʡ�ж��ٸ�����
	int city_num;
	// �����б�
	char **city;
} cl_province_t;

typedef struct {
	// �ж��ٸ�ʡ��
	int province_num;
	// ʡ���б�
	cl_province_t **province;
} cl_city_list_t;

typedef struct{
	//�������ڻ�����ȡ�������
	int temp; // �¶�
	int humidity; //ʪ��
	int voc;
	int pm25;
	int pm10;
	int no2;
	int so2;
	int o3;
	int co;
}env_air_t;

/*
	��ȡĳ���豸���ڳ��еĻ�������
*/
CLIB_API cl_env_mon_t *cl_em_get_info(cl_handle_t user_handle);
/*
	�ͷ�cl_em_get_info���ص�����
*/
CLIB_API void cl_em_free_info(cl_env_mon_t *cl);


/*
	��ȡĿǰ֧�ֵĳ����б�
*/
CLIB_API cl_city_list_t *cl_em_get_city_list();
CLIB_API void cl_em_free_city_list(cl_city_list_t *cl);

/*
	�����豸��ǰ���ڵĳ��У�������֧�ֵĳ����б������һ��
*/
CLIB_API RS cl_em_set_city(cl_handle_t user_handle, const char *city);

/*
	��ȡ���ڻ�������
	�������:
		user_handle @ �û����
		env @ ��ǰ���ڻ�������
	�������:
		��
	��ע�¼�:
	UE_ENV_WEATHER_SUGGEST ����cl_em_get_info
*/
CLIB_API RS cl_em_get_suggest(cl_handle_t user_handle, const env_air_t *env);

#ifdef __cplusplus
}
#endif 

#endif


