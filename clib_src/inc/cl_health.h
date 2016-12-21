#ifndef	__CL_HEALTH_H__
#define	__CL_HEALTH_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

//event
enum {
	HE_BEGIN = 1100,
	HE_FM_LIST_OK = HE_BEGIN + 1,
	HE_FM_LIST_FAIL = HE_BEGIN + 2,
	HE_FM_CONFIG_OK = HE_BEGIN + 3,
	HE_FM_CONFIG_FAIL = HE_BEGIN + 4,
	HE_MEASURE_QUERY_OK = HE_BEGIN + 5,
	HE_MEASURE_QUERY_FAIL = HE_BEGIN + 6,
	HE_MEASURE_DEL_OK = HE_BEGIN + 7,
	HE_MEASURE_DEL_FAIL = HE_BEGIN + 8,	
	HE_END = HE_BEGIN + 99	
};

/* 健康测量设备类型 */
#define HDT_WEIGTH 0x01  /* 体重计 */
#define HDT_FAT 0x02      /* 脂肪仪 */
#define HDT_BLOOD_PRESSURE 0x03  /* 血压计 */
#define HDT_PEDOMETER 0x04      /* 计步器 */
#define HDT_BLOOD_SUGAR 0x05   /* 血糖仪 */
#define HDT_BLOOD_OXYGN 0x06   /* 血氧仪 */
#define HDT_EAR_TEMP	0x07	/*耳温枪*/

/* 测量参数类型 */
#define HMT_WEIGHT 0x12  /* 实时体重（2Byte 0.0kg）、单位（1Byte：0x00 kg，0x01 lb， 0x02 st）*/
#define HMT_USER 0x13    /* 用户资料（年龄1Byte、身高1Byte、性别（bit7女=1 男=0）&模式（bit6~0）、步距1Byte（0~255cm) ）*/
#define HMT_FAT  0x14    /* 脂肪率(2Byte 0.0%)*/
#define HMT_WATER 0x15   /* 水分（2Byte 0.0%）*/
#define HMT_MUSCLE 0x16  /* 肌肉（2Byte 0.0kg）*/
#define HMT_BONE 0x17    /* 骨骼（2Byte 0.0%）*/
#define HMT_BMI 0x18     /* 身体质量指数（Body Mass Index),BMI(2Byte 0.0)*/
#define HMT_BM 0x19     /* basic metabolism, 基础代谢KCAl（2Byte 1KCAl）*/
#define HMT_M 0x1A      /* metabolism，新陈代谢KCAl（2Byte 1KCAl）*/
#define HMT_VISCERAL_FAT 0x1B  /* 内脏脂肪等级（2Byte）*/
#define HMT_WEIGHT_CF 0x1C /* 确定体重（2Byte 0.0kg）、单位（1Byte：0x00 kg，0x01 lb， 0x02 st）体重确定信号在秤LCD闪烁前发送至主机*/
#define HMT_PRESSURE_H 0x1D  /* 高压值（2Byte 0mmHg）、单位（1Byte：0x00 mmHg，0x01 kPa）*/
#define HMT_PRESSURE_l 0x1E  /* 低压值（2Byte 0mmHg）、单位（1Byte：0x00 mmHg，0x01 kPa）*/
#define HMT_PLUS 0x1F        /* 脉搏数（1Byte ）*/
#define HMT_WALK_CNT 0x20    /* 运动步数,高4Byte（最大步数为99999步）*/
#define HMT_WALK_DISTANCE 0x21    /* 运动距离高4Byte（最大距离为999.99kms）*/
#define HMT_CALORIE 0x22     /* 消耗能量KCAL  4Byte（1 KCAL）*/
#define HMT_BLOOD_SUGAR 0x23  /* 血糖值（2Byte）例如28.5mmol/L 直接发送285   ------血糖仪*/
#define HMT_BLOOD_OXYGN 0x24  /* ---'血氧饱和度（1Byte）                     ------血氧仪*/
#define HMT_BLOOD_PLUS  0x25  /* 脉率（1Byte）                               ------血氧仪*/

#define CL_MAX_NAME 16

typedef struct{
	u_int16_t    bd_year;   /* 出生年，例如1980*/
	u_int16_t    weight;   /* 体重，kg*10*/
	u_int8_t    bd_month;  /* 出生月, 1到12*/
	u_int8_t    height;     /* 身高，cm*/
	u_int8_t    sex;       /* 性别，SX_WOMEN or SX_MAN*/
	u_int8_t    career;    /* 职业，CAREER_NORMAL */
	u_int8_t    step;      /* 步距，cm*/
	u_int8_t    id;        /* 家庭成员id，由服务器生成，手机修改/删除成员时要填写*/
	u_int8_t    action;    /*ACTION_ADD ACTION_XXX*/
	u_int8_t    is_current;  /* true表示当前测量人员*/
	u_int8_t    reserved[4];  /* 保留*/
	u_int8_t    name[CL_MAX_NAME];
}family_t;

typedef struct{
	int count;
	family_t *list;
}family_list_t;

/* 查询测量结果条件数据结构 */
typedef struct{
	u_int32_t    begin_time;  /* 测量开始时间，0表示不限制*/
	u_int32_t    end_time;   /* 测量结束时，0表示不限制*/
	u_int8_t    fm_id;       /* 测量人员id，0表示不限制*/
	u_int8_t    hdt;         /* 测量类型，HDT_XXX，0表示不限制*/
	u_int8_t    count;       /* 查询测量条数，0表示不限制*/
	u_int8_t    reserved[5];  /* 保留*/
}measure_query_t;

typedef struct{
	u_int32_t    mtime;   /*测量时间戳*/
	u_int8_t    fm_id;   /*被测量家庭成员id*/
	u_int8_t    hdt;     /*测量设备类型*/
	u_int8_t    measure_cnt;     /*测量参数个数*/
	u_int8_t    reserved;      /*保留*/
}measure_del_t;

/* 体重仪测量数据 */
typedef struct{
	float weight;
}measure_weight_t;

/* 脂肪仪测量数据 */
typedef struct{
	float fat;
	float water;
	float muscle;
	float visual_fat;
}measure_fat_t;

/* 血压仪测量数据 */
typedef struct{
	float pressure_high; 
	float pressure_low;
	float plus;
}measure_blood_pressure_t;

/* 血糖仪测量数据 */
typedef struct{
	float sugar; 
}measure_blood_sugar_t;

/* 血氧仪测量数据 */
typedef struct{
	float oxygen;
	float plus;
}measure_blood_oxygen_t;

/* 计步器测量数据 */
typedef struct{
	float step;	//步数
	float calorie;	//卡路里
	float oxygen_step;	//有氧步数
	float oxygen_calorie;	//有氧卡路里
}one_pedometer_t;

#define MAX_PEDOMETER 7
typedef struct{
	/* 计步器每条记录最多有7天数据 */
	/* is_valid表示从测量时间倒推每天记录是否有效 */
	u_int8_t is_valid[MAX_PEDOMETER];
	u_int8_t valid_cnt;	//有效天数
	one_pedometer_t pedometer[MAX_PEDOMETER];
}measure_pedometer_t;;

/* 耳温枪测量数据 */
typedef struct{
	float temperature;
}measure_ear_temp_t;

typedef union{
	measure_weight_t weight;
	measure_fat_t fat;
	measure_blood_pressure_t blood_pressure;
	measure_blood_sugar_t blood_sugar;
	measure_blood_oxygen_t blood_oxygen;
	measure_ear_temp_t ear_temp;
	measure_pedometer_t meter;
}measure_data_t;

typedef struct{
	u_int32_t    mtime;   /*测量时间戳*/
	u_int8_t    fm_id;   /*被测量家庭成员id*/
	u_int8_t    hdt;     /*测量设备类型*/
	u_int8_t    measure_cnt;     /*测量参数个数*/
	u_int8_t    reserved;      /*保留*/
	measure_data_t measure_data;
}measure_t;

typedef struct{
	int count;
	measure_t *list;
}measure_list_t;
/*
	功能：
		查询健康家庭成员列表
	输入参数:
		@handle: 设备句柄
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	回调事件: HE_FM_LIST_OK , 查询成功, 调用cl_family_list_get获取列表
			  	   HE_FM_LIST_FAIL , 查询失败
	
*/
CLIB_API RS cl_family_list_query(cl_handle_t handle);
CLIB_API family_list_t *cl_family_list_get(cl_handle_t handle);
CLIB_API void cl_family_list_free(family_list_t *fl);

/*
	功能：
		配置健康家庭成员, 一次只能配置一个
	输入参数:
		@handle: 设备句柄
		@fm: 家庭成员信息
	输出参数:
		无
	返回：
		RS_OK: 成功
		其他: 失败
	回调事件: HE_FM_CONFIG_OK HE_FM_CONFIG_FAIL
*/
CLIB_API RS cl_family_config(cl_handle_t handle, family_t *fm);

CLIB_API RS cl_measure_query(cl_handle_t handle, measure_query_t *query);
CLIB_API measure_list_t *cl_measure_list_get(cl_handle_t handle);
CLIB_API void cl_measure_list_free(measure_list_t *ml);

CLIB_API RS cl_measure_delete(cl_handle_t handle, measure_del_t *del);

#ifdef __cplusplus
}
#endif 


#endif


