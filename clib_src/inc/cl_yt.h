/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_yt.h
**  File:    cl_yt.h
**  Author:  liubenlong
**  Date:    08/24/2015
**
**  Purpose:
**    ����.
**************************************************************************/


#ifndef CL_YT_H
#define CL_YT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
//onoff
#define YT_ON	0
#define YT_OFF	1

#define YT_TMP_BASE		16

//mode
enum {
	YT_MODE_AUTO = 0,//�Զ�
	YT_MODE_COOL,	//����
	YT_MODE_AREFACTION,//��ʪ
	YT_MODE_WIND,	//�ͷ�
	YT_MODE_HOT,
	YT_MODE_MAX,
};

enum {
	YT_WIND_SPEED_AUTO = 0,//�Զ�����
	YT_WIND_SPEED_ONE,	//����һ
	YT_WIND_SPEED_TWO,	//���ٶ�
	YT_WIND_SPEED_THREE,	//������
	YT_WIND_SPEED_MAX,
};

enum {
	YT_WIND_DIR_AUTO = 0,//�����Զ�
	YT_WIND_DIR_ONE,	//����һ
	YT_WIND_DIR_TWO,	//�����
	YT_WIND_DIR_THREE,	//������ 
	YT_WIND_DIR_FOUR,	//������
	YT_WIND_DIR_FIVE,	//������
	//YT_WIND_DIR_UPDOWN,	//���°ڷ�
	//YT_WIND_DIR_LEFTRIGHT,//���Ұڷ�
	YT_WIND_DIR_MAX,
};

typedef enum {
	YT_ACTION_ONOFF = 0,//����
	YT_ACTION_MODE,	//ģʽ
	YT_ACTION_TMP,	//�¶�
	YT_ACTION_WINDSPEED,//����
	YT_ACTION_WINDDIR,//����
	YT_ACTION_ELEASSIST,//�縨
	YT_ACTION_SLEEP,//˯��
	YT_ACTION_SWING,//�ڷ�
	YT_ACTION_MAX,
}YT_ACTION_T;

#define YT_TMP_MAX	31
#define YT_TMP_MIN	16
/* Type definitions. */
#pragma pack(push, 1)
typedef struct {
	u_int8_t onoff;
	u_int8_t mode;
	u_int8_t tmp;
	u_int8_t wind_speed;
	u_int8_t wind_dir;
	u_int8_t ele_assist;
	u_int8_t sleep;
	u_int8_t swing;
	u_int8_t action;
	u_int8_t room_tmp;
	u_int8_t extern_tmp;
	u_int8_t pad[5];
}cl_yt_work_t;

typedef struct {
	u_int8_t off_enable;
	u_int8_t pad2;
	u_int16_t off_remain_min;
	u_int8_t on_enable;//�Ƿ�ʹ��
	u_int8_t pad1;
	u_int16_t on_remain_min;//ʣ��ʱ��
}cl_yt_timer_t;

typedef struct {
	u_int16_t index;//���ͱ��±ꡣ=0xffff,��ʾûɨ�������Ҫɨ��
	u_int16_t ele_assist_power;//���ȹ���
	u_int8_t freq_type;//0=��Ƶ��1=��Ƶ��
	u_int8_t cool_type;//0=���䣬1=��ů��
	u_int8_t rl_swing;//0=�����Ұڷ磬1=�����Ұڷ硣
	u_int8_t pad2;
	u_int16_t cool_power;//���书�ʡ�
	u_int16_t hot_power;//���ȹ��ʡ�
	u_int64_t sn;//sn
}cl_yt_ac_type_t;


#pragma pack(pop)

//��ѯ����ʱ���
typedef struct {
	u_int16_t begin_year;
	u_int8_t begin_month;
	u_int8_t begin_day;
	u_int16_t end_year;
	u_int8_t end_month;
	u_int8_t end_day;
}cl_query_ele_time_t;


/*
		������ʾ����B2		������ʾ����B12����Ƶ����		������ʾ����B13����Ƶ����		������ʾ����B14����Ƶ����	������ʾ����B18����Ƶ����

bit0		���¹���			����EE����					�⻷����					����AC��������ͣ��		�������̹��ȱ���ͣ��

bit1		���̹���			����EE����					���̹���					ѹ�������������ͣ��		�����⻷���ͱ���ͣ��

bit2		��������			�����ͨ�Ź���				��������					����AC��ѹ����ͣ��		�����⻷���߱���ͣ��

bit3		�������			��������������ͨ�Ź���		����						ֱ��ĸ�ߵ�ѹ����ͣ��		����

bit4		����				ѹ���������쳣				����						IPMģ���¶ȹ�			����

bit5		����				ѹ����ʧ������				����						�����¶ȹ��߱���ͣ��		����

bit6		����				IPMģ�����				����						�������̷����ᱣ��ͣ��	����
	
bit7		����				����						����						�������̹��ȱ���ͣ��		����

*/
typedef struct {
	//����������
	u_int8_t onoff;//0:�رգ�1:��
	u_int8_t mode;//0���Զ� 1������ 2����ʪ 3���ͷ� 4������
	u_int8_t tmp;//16��31,�����¶�
	u_int8_t wind_speed;//0���Զ���1������1��2������2��3������3
	u_int8_t wind_dir;//0���Զ���1������1��2������2��3������3��4������4��5 = ����5��6 = ���°ڷ�
	u_int8_t ele_assist;//�縨,1=����0=��
	u_int8_t sleep;//˯��,1=����0=��
	u_int8_t swing;//�ڷ�,0=�ڷ�رգ� 1 = ���Ұڷ�
	u_int8_t room_tmp;//����-30~99��
	u_int8_t extern_tmp;//�����¶�-30~99��
	u_int8_t sn_err;//0������1��ʾ��������ʵ�ʻ��Ͳ�ƥ��

	//��ʱ��
	cl_yt_timer_t timer;


	//����ʾ����
	u_int8_t compressor_onoff;//ѹ�������أ�1=����0=��
	u_int8_t extern_wind_onoff;//�������أ�0=��, 1=�ͷ磬2=�߷�
	u_int8_t four_valve;//��ͨ�����أ�1=����0=��
	u_int8_t ele_hot;//�����ȿ��أ�1=����0=��
	u_int16_t compressor_work_hours;//ѹ���������ۼ�ʱ�䣬��λСʱ
	u_int8_t down_reboot;//�����������ܣ�1=����0=��
	u_int8_t wind_real_spee;//���ʵ��ת�٣���ʾʱֵ*10����λת
	u_int8_t inner_tmp;//�����¶ȣ�-30~99��
	u_int8_t dc_busway_val;//ֱ��ĸ�ߵ�ѹ,��Ƶ������ʾʱ��ֵ+200����λ����
	u_int8_t extern_ac_val;//����ac��ѹ,��Ƶ������ʾʱ, ֵ+100����λ����
	u_int8_t extern_ac_cur;//����ac��������Ƶ������ʾʱ, ֵ/10������С�����һλ����λ����
	u_int8_t compressor_cur;//ѹ�������������Ƶ������ʾʱ��ֵ/10������С�����һλ����λ����
	u_int8_t compressor_freq;//��Ƶ��, ѹ������תƵ�ʣ�0~255HZ
	u_int8_t outside_tmp;//������Ƶ��, �����¶�,-30~99��
	u_int8_t exhaust_tmp;//��Ƶ��, �����¶ȣ�-30~127��
	u_int8_t ipm_tmp;//��Ƶ��, ipm�¶ȣ�-30~127��
	u_int8_t heat_defrost;// 1 = ���ȳ�˪״̬��0 = ���ȷǳ�˪״̬
	u_int8_t sys_type;//0=��Ƶ�һ���1=��Ƶ�����2=��Ƶ�һ�,3=��Ƶ���
	u_int8_t in_fans_gears;//ʵ�ʷ��ٵ�λ��0=ֹͣ��1=΢�磬2=�͵���3=�е���4=�ߵ�
	u_int16_t assit_work_hours;//ѹ���������ۼ�ʱ�䣬��λСʱ



	//���ϱ���״̬
	u_int8_t fault_b2;//������ʾ����B2
	u_int8_t fault_b12;//������ʾ����B15,ʵ����b15������jni�Ķ���ֻ��ӡ�����
	u_int8_t protect_b13;//������ʾ����B16
	u_int8_t protect_b14;//������ʾ����B17 ,λ��ʾ��������ͼһ
	u_int8_t protect_b18;//������ʾ����18

	//����ͳ��
	u_int32_t ele_total;//���ܲ��ã��ȷ����
	
	cl_query_ele_time_t ele_time;//�׶ε���ʱ��Ρ�
	u_int32_t ele_phase;//�׶ε�������λ��0.1��

	//���ڻ��͵�һЩ����
	cl_yt_ac_type_t ac_info;
	char name[30];//����

	//����ֶ���sdk�õģ�app jni���Բ��ܣ���Ҫ��Ϊ�˷���sdk����
	cl_yt_work_t work;
	bool ac_info_valid;
	bool stat_info_valid;
	bool work_info_valid;
}cl_yt_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	����:
		�������������
	�������:
		@dev_handle: �豸�ľ��
		@action:������Ϊ
		@value:
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yt_ctrl(cl_handle_t dev_handle, YT_ACTION_T action, u_int8_t value);

/*
	����:
		��ʱ������
	�������:
		@dev_handle: �豸�ľ��
		@on_remain_min:����ʣ��ʱ������
		@off_remain_min:�ػ�ʣ��ʱ������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yt_timer_config(cl_handle_t dev_handle, cl_yt_timer_t *ptimer);

/*
	����:
		������ɨ��
	�������:
		@dev_handle: �豸�ľ��
		@sn:����������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yt_scan_sn(cl_handle_t dev_handle, char *sn);


/*
	����:
		��ѯ����������ѡ��ʱ��κ���øú���������ᷢmodify����ʱ��infoȡ������ʾ
	�������:
		@dev_handle: �豸�ľ��
		@time:��ѯ������ʱ���
	�������:
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yt_query_ele(cl_handle_t dev_handle, cl_query_ele_time_t *time);






#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_YT_H */

