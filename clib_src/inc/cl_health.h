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

/* ���������豸���� */
#define HDT_WEIGTH 0x01  /* ���ؼ� */
#define HDT_FAT 0x02      /* ֬���� */
#define HDT_BLOOD_PRESSURE 0x03  /* Ѫѹ�� */
#define HDT_PEDOMETER 0x04      /* �Ʋ��� */
#define HDT_BLOOD_SUGAR 0x05   /* Ѫ���� */
#define HDT_BLOOD_OXYGN 0x06   /* Ѫ���� */
#define HDT_EAR_TEMP	0x07	/*����ǹ*/

/* ������������ */
#define HMT_WEIGHT 0x12  /* ʵʱ���أ�2Byte 0.0kg������λ��1Byte��0x00 kg��0x01 lb�� 0x02 st��*/
#define HMT_USER 0x13    /* �û����ϣ�����1Byte�����1Byte���Ա�bit7Ů=1 ��=0��&ģʽ��bit6~0��������1Byte��0~255cm) ��*/
#define HMT_FAT  0x14    /* ֬����(2Byte 0.0%)*/
#define HMT_WATER 0x15   /* ˮ�֣�2Byte 0.0%��*/
#define HMT_MUSCLE 0x16  /* ���⣨2Byte 0.0kg��*/
#define HMT_BONE 0x17    /* ������2Byte 0.0%��*/
#define HMT_BMI 0x18     /* ��������ָ����Body Mass Index),BMI(2Byte 0.0)*/
#define HMT_BM 0x19     /* basic metabolism, ������лKCAl��2Byte 1KCAl��*/
#define HMT_M 0x1A      /* metabolism���³´�лKCAl��2Byte 1KCAl��*/
#define HMT_VISCERAL_FAT 0x1B  /* ����֬���ȼ���2Byte��*/
#define HMT_WEIGHT_CF 0x1C /* ȷ�����أ�2Byte 0.0kg������λ��1Byte��0x00 kg��0x01 lb�� 0x02 st������ȷ���ź��ڳ�LCD��˸ǰ����������*/
#define HMT_PRESSURE_H 0x1D  /* ��ѹֵ��2Byte 0mmHg������λ��1Byte��0x00 mmHg��0x01 kPa��*/
#define HMT_PRESSURE_l 0x1E  /* ��ѹֵ��2Byte 0mmHg������λ��1Byte��0x00 mmHg��0x01 kPa��*/
#define HMT_PLUS 0x1F        /* ��������1Byte ��*/
#define HMT_WALK_CNT 0x20    /* �˶�����,��4Byte�������Ϊ99999����*/
#define HMT_WALK_DISTANCE 0x21    /* �˶������4Byte��������Ϊ999.99kms��*/
#define HMT_CALORIE 0x22     /* ��������KCAL  4Byte��1 KCAL��*/
#define HMT_BLOOD_SUGAR 0x23  /* Ѫ��ֵ��2Byte������28.5mmol/L ֱ�ӷ���285   ------Ѫ����*/
#define HMT_BLOOD_OXYGN 0x24  /* ---'Ѫ�����Ͷȣ�1Byte��                     ------Ѫ����*/
#define HMT_BLOOD_PLUS  0x25  /* ���ʣ�1Byte��                               ------Ѫ����*/

#define CL_MAX_NAME 16

typedef struct{
	u_int16_t    bd_year;   /* �����꣬����1980*/
	u_int16_t    weight;   /* ���أ�kg*10*/
	u_int8_t    bd_month;  /* ������, 1��12*/
	u_int8_t    height;     /* ��ߣ�cm*/
	u_int8_t    sex;       /* �Ա�SX_WOMEN or SX_MAN*/
	u_int8_t    career;    /* ְҵ��CAREER_NORMAL */
	u_int8_t    step;      /* ���࣬cm*/
	u_int8_t    id;        /* ��ͥ��Աid���ɷ��������ɣ��ֻ��޸�/ɾ����ԱʱҪ��д*/
	u_int8_t    action;    /*ACTION_ADD ACTION_XXX*/
	u_int8_t    is_current;  /* true��ʾ��ǰ������Ա*/
	u_int8_t    reserved[4];  /* ����*/
	u_int8_t    name[CL_MAX_NAME];
}family_t;

typedef struct{
	int count;
	family_t *list;
}family_list_t;

/* ��ѯ��������������ݽṹ */
typedef struct{
	u_int32_t    begin_time;  /* ������ʼʱ�䣬0��ʾ������*/
	u_int32_t    end_time;   /* ��������ʱ��0��ʾ������*/
	u_int8_t    fm_id;       /* ������Աid��0��ʾ������*/
	u_int8_t    hdt;         /* �������ͣ�HDT_XXX��0��ʾ������*/
	u_int8_t    count;       /* ��ѯ����������0��ʾ������*/
	u_int8_t    reserved[5];  /* ����*/
}measure_query_t;

typedef struct{
	u_int32_t    mtime;   /*����ʱ���*/
	u_int8_t    fm_id;   /*��������ͥ��Աid*/
	u_int8_t    hdt;     /*�����豸����*/
	u_int8_t    measure_cnt;     /*������������*/
	u_int8_t    reserved;      /*����*/
}measure_del_t;

/* �����ǲ������� */
typedef struct{
	float weight;
}measure_weight_t;

/* ֬���ǲ������� */
typedef struct{
	float fat;
	float water;
	float muscle;
	float visual_fat;
}measure_fat_t;

/* Ѫѹ�ǲ������� */
typedef struct{
	float pressure_high; 
	float pressure_low;
	float plus;
}measure_blood_pressure_t;

/* Ѫ���ǲ������� */
typedef struct{
	float sugar; 
}measure_blood_sugar_t;

/* Ѫ���ǲ������� */
typedef struct{
	float oxygen;
	float plus;
}measure_blood_oxygen_t;

/* �Ʋ����������� */
typedef struct{
	float step;	//����
	float calorie;	//��·��
	float oxygen_step;	//��������
	float oxygen_calorie;	//������·��
}one_pedometer_t;

#define MAX_PEDOMETER 7
typedef struct{
	/* �Ʋ���ÿ����¼�����7������ */
	/* is_valid��ʾ�Ӳ���ʱ�䵹��ÿ���¼�Ƿ���Ч */
	u_int8_t is_valid[MAX_PEDOMETER];
	u_int8_t valid_cnt;	//��Ч����
	one_pedometer_t pedometer[MAX_PEDOMETER];
}measure_pedometer_t;;

/* ����ǹ�������� */
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
	u_int32_t    mtime;   /*����ʱ���*/
	u_int8_t    fm_id;   /*��������ͥ��Աid*/
	u_int8_t    hdt;     /*�����豸����*/
	u_int8_t    measure_cnt;     /*������������*/
	u_int8_t    reserved;      /*����*/
	measure_data_t measure_data;
}measure_t;

typedef struct{
	int count;
	measure_t *list;
}measure_list_t;
/*
	���ܣ�
		��ѯ������ͥ��Ա�б�
	�������:
		@handle: �豸���
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�ص��¼�: HE_FM_LIST_OK , ��ѯ�ɹ�, ����cl_family_list_get��ȡ�б�
			  	   HE_FM_LIST_FAIL , ��ѯʧ��
	
*/
CLIB_API RS cl_family_list_query(cl_handle_t handle);
CLIB_API family_list_t *cl_family_list_get(cl_handle_t handle);
CLIB_API void cl_family_list_free(family_list_t *fl);

/*
	���ܣ�
		���ý�����ͥ��Ա, һ��ֻ������һ��
	�������:
		@handle: �豸���
		@fm: ��ͥ��Ա��Ϣ
	�������:
		��
	���أ�
		RS_OK: �ɹ�
		����: ʧ��
	�ص��¼�: HE_FM_CONFIG_OK HE_FM_CONFIG_FAIL
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


