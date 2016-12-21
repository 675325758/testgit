#ifndef	__CL_IA_H__
#define	__CL_IA_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"
#include "cl_smart_appliance.h"

//event
enum {
	IE_BEGIN = 1000,
	/* ����ĳ�����Գɹ� */
	IE_CTRL_OK,
	/* ����ĳ������ʧ�� */
	IE_CTRL_FAULT,

	/* �������������������¼� */
	IE_AIRCLEANER_SET_ONOFF_OK,
	IE_AIRCLEANER_SET_SPEED_OK,
	IE_AIRCLEANER_SET_TIMER_OK,
	IE_AIRCLEANER_SET_ULTRA_OK,
	IE_AIRCLEANER_SET_ANION_OK,
	IE_AIRCLEANER_SET_ONOFF_FAULT,
	IE_AIRCLEANER_SET_SPEED_FAULT,
	IE_AIRCLEANER_SET_TIMER_FAULT,
	IE_AIRCLEANER_SET_ULTRA_FAULT,
	IE_AIRCLEANER_SET_ANION_FAULT,

	/* ����¯ ����  DF-HDW2001RA���������¼� */
	IE_AIRHEATER_SET_ONOFF_OK,
	IE_AIRHEATER_SET_GEAR_OK,
	IE_AIRHEATER_SET_TIME_OK,
	IE_AIRHEATER_SET_MODE_OK,
	IE_AIRHEATER_SET_ONOFF_FAULT,
	IE_AIRHEATER_SET_GEAR_FAULT,
	IE_AIRHEATER_SET_TIME_FAULT,
	IE_AIRHEATER_SET_MODE_FAULT,

	/* ��ˮ�����������¼� */
	IE_WATERHEATER_SET_WORK_OK,
	IE_WATERHEATER_SET_TEMP_OK,
	IE_WATERHEATER_SET_TIMER_OK,
	IE_WATERHEATER_SET_CAPACTITY_OK,
	IE_WATERHEATER_SET_WORK_FAULT,
	IE_WATERHEATER_SET_TEMP_FAULT,
	IE_WATERHEATER_SET_TIMER_FAULT,
	IE_WATERHEATER_SET_CAPACTITY_FAULT,

	/* �յ����������¼� */
	IE_AIRCONDITION_SET_ONOFF_OK,
	IE_AIRCONDITION_SET_MODE_OK,
	IE_AIRCONDITION_SET_TEMP_OK,
	IE_AIRCONDITION_SET_TIMER_OK,
	IE_AIRCONDITION_SET_ONOFF_FAULT,
	IE_AIRCONDITION_SET_MODE_FAULT,
	IE_AIRCONDITION_SET_TEMP_FAULT,
	IE_AIRCONDITION_SET_TIMER_FAULT,

	/* �������������¼� */
	IE_ELECTRICFAN_SET_WORK_OK,
	IE_ELECTRICFAN_SET_GEAR_OK,
	IE_ELECTRICFAN_SET_TIMER_OK,
	IE_ELECTRICFAN_SET_SHAKE_OK,
	IE_ELECTRICFAN_SET_WORK_FAULT,
	IE_ELECTRICFAN_SET_GEAR_FAULT,
	IE_ELECTRICFAN_SET_TIMER_FAULT,
	IE_ELECTRICFAN_SET_SHAKE_FAULT,

	/* �豸���Է����仯 */
	IE_UPDATE_STATUS,

	/* ǰ����ˮ�����������¼� */
	IE_WATERHEATER_A9_SET_WORK_OK,
	IE_WATERHEATER_A9_SET_TEMP_OK,
	IE_WATERHEATER_A9_SET_MODE_OK,
	IE_WATERHEATER_A9_CLEAR_CNT_OK,
	IE_WATERHEATER_A9_SET_FIRE_LEVEL_OK,
	IE_WATERHEATER_A9_SET_WORK_FAULT,
	IE_WATERHEATER_A9_SET_TEMP_FAULT,
	IE_WATERHEATER_A9_SET_MODE_FAULT,
	IE_WATERHEATER_A9_CLEAR_CNT_FAULT,
	IE_WATERHEATER_A9_SET_FIRE_LEVEL_FAULT,



	/* ����¯ ������͡ ���������¼� */
	IE_AIRHEATER_YCYT_SET_TEMP_OK,
	IE_AIRHEATER_YCYT_SET_MODE_OK,
	IE_AIRHEATER_YCYT_SET_GEAR_OK,
	IE_AIRHEATER_YCYT_SET_TIMER_OK,
    IE_AIRHEATER_YCYT_SET_ONOFF_OK,
    IE_AIRHEATER_YCYT_SET_ORDER_TIMER_OK,
    
	IE_AIRHEATER_YCYT_SET_TEMP_FAULT,
	IE_AIRHEATER_YCYT_SET_MODE_FAULT,
	IE_AIRHEATER_YCYT_SET_GEAR_FAULT,
	IE_AIRHEATER_YCYT_SET_TIMER_FAULT,
    IE_AIRHEATER_YCYT_SET_ONOFF_FAULT,
    IE_AIRHEATER_YCYT_SET_ORDER_TIMER_FAULT,
    /*����ԡ��*/
	IE_BATHHEATER_SET_WORK_OK,
	IE_BATHHEATER_SET_LIGHT_OK,
	IE_BATHHEATER_SET_ANION_OK,
	IE_BATHHEATER_SET_BREATH_OK,
	IE_BATHHEATER_SET_DRY_OK,
	IE_BATHHEATER_SET_TRONIC_OK,
	IE_BATHHEATER_SET_TIME_OK,
	IE_BATHHEATER_SET_WORK_FAULT,
	IE_BATHHEATER_SET_LIGHT_FAULT,
	IE_BATHHEATER_SET_ANION_FAULT,
	IE_BATHHEATER_SET_BREATH_FAULT,
	IE_BATHHEATER_SET_DRY_FAULT,
	IE_BATHHEATER_SET_TRONIC_FAULT,
	IE_BATHHEATER_SET_TIME_FAULT,

	/* �������������������¼�(����) */
	IE_AIRCLEANER_SET_MODE_OK,
	IE_AIRCLEANER_SET_MODE_FAULT,
	/* ��������������ɱ���¼�(�ϰ�) */
	IE_AIRCLEANER_SET_TERILIZE_OK,
	IE_AIRCLEANER_SET_TERILIZE_FAULT,

	IE_END = IE_BEGIN + 99
};

/**********************************************
	  �յ������ĺ궨��
 **********************************************/

// ����
#define	AC_POWER_ON	0
#define	AC_POWER_OFF	1

// ģʽ
#define	AC_MODE_AUTO	0
#define	AC_MODE_COLD	1
#define	AC_MODE_AREFACTION 2
#define	AC_MODE_WIND	3
#define	AC_MODE_HOT	4

#define	AC_TEMP_BASE	16
#define AC_TEMP_MAX     32

// ����
#define	AC_WIND_AUTO	0
#define	AC_WIND_1	1
#define	AC_WIND_2	2
#define	AC_WIND_3	3

// ����
#define	AC_DIR_AUTO	0
#define	AC_DIR_1	1
#define	AC_DIR_2	2
#define	AC_DIR_3	3

#define	AC_KEY_POWER	0
#define	AC_KEY_MODE     1
#define	AC_KEY_TEMP     2
#define	AC_KEY_WIND     3
#define	AC_KEY_DIR		4


/**************************************************************************************************
	  �����������������Ϣ 
 **************************************************************************************************/

typedef struct {
	 /* ���� 0������; 1: ���� (Ĭ�ϸ߷���) */
	u_int16_t onoff;

	/* ������ 0: �ر�; 1: ���� */
	u_int16_t ultra;

	/* ������ 0: �ر�; 1: ����  */
	u_int16_t anion;

	/* ���� 2: �ͷ���; 3: �з���; 4: �߷��� */
	u_int16_t speed;
	
	/* ��ʱʱ��(����)�� 0��ʾȡ����ʱ������Ϊ���õĶ�ʱ�����������24Сʱ(24*60)
	* ע�⣺
	*	�����ǰ���ڴ���״̬����ʱ��ʾ��ʱ��ʼ����
	*	�����ǰ���ڹ���״̬����ʱ��ʾ��ʱ�رգ��������ģʽ
	*	���ƾ�������ʾ��ʱʣ������������ο�timer_type	
	*/
	u_int16_t timer;

	/* PM2.5 ��ֵ��ӦPM2.5��ֵ */
	u_int16_t pm25;

	/* �¶� ʵ���¶� * 10 */
	u_int16_t temp;

	/* ʪ�� ʵ��ʪ�� */
	u_int16_t rh;

	/* ���� */
	u_int16_t power;

	/*���ƿ�������������ģʽ*/
	u_int16_t work_mode;
	/*���ƿ�����������ʱ������: 1 ��ʱ����2 ��ʱ��*/
	u_int16_t timer_type; 
	/*���ƿ������������趨�Ķ�ʱСʱ*/
	u_int16_t set_hour;	

	/*�ϰس���ɱ�� 0: �ر�; 1: ���� */
	u_int16_t terilize;
	/*�ϰس���ɱ��ʱ��*/
	u_int16_t terilize_minute;
	/*�ϰ�����ʹ����������*/
	u_int32_t rosebox_life;
	cl_air_timer_info_t periodic_timer; //���ڶ�ʱ��
	
	u_int16_t pad;
} cl_ia_aircleaner_info_t;






/*
	����:
		���ؿ���������
	�������:
		@dev_handle:  �����������ľ��
		@onoff:  1,����0����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_onoff(cl_handle_t dev_handle, bool onoff);




/*
	����:
		���ÿ�������������
	�������:
		@dev_handle:  �����������ľ��
		@speed:  1: �ͷ���; 2: �з���; 3: �߷���; 4: ���߷���(����)
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_speed(cl_handle_t dev_handle, u_int8_t speed);





/*
	����:
		���ÿ�����������ʱ��
	ע�⣺
����		�����ǰ���ڴ���״̬����ʱ��ʾ��ʱ��ʼ����
		�����ǰ���ڹ���״̬����ʱ��ʾ��ʱ�رգ��������ģʽ
	�������:
		@dev_handle:  �����������ľ��
		@min:  0ȡ����ʱ������Ϊ���õĶ�ʱ�����������24Сʱ(24*60)
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_timer(cl_handle_t dev_handle, u_int32_t min);




/*
	����:
		���ÿ����������������߹���
	�������:
		@dev_handle:  �����������ľ��
		@onoff:  0: �ر�; 1: ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_ultra(cl_handle_t dev_handle, bool onoff);






/*
	����:
		���ÿ����������ĸ����ӹ���
	�������:
		@dev_handle:  �����������ľ��
		@onoff:  0: �ر�; 1: ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_anion(cl_handle_t dev_handle, bool onoff);


/*
	����:
		���ÿ����������Ĺ���ģʽ
		���ƿ�������������
	�������:
		@dev_handle:  �����������ľ��
		@mode: 1: �Զ� 2: �ֶ� 3: ˯��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
	����:
		��ѯ���������������й���״̬
		Ŀǰ��udpЭ��֧��
	�������:
		@dev_handle:  �����������ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_query_all(cl_handle_t dev_handle);

/*
	����:
		���ÿ����������ĳ���ɱ������
		�ϰؿ�������������
	�������:
		@dev_handle:  �����������ľ��
		@is_on:  0: �ر� 1: ����
		@minute: ����ɱ��ʱ�䣬֧��10���ӡ�30���ӡ�60����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircleaner_set_terilize(cl_handle_t dev_handle, u_int8_t is_on, u_int8_t minute);

/*
 ����:
    ������ڶ�ʱ��
 �������:
    @time_info: ��ʱ����Ϣ id�����ڣ���Ϊ�޸�
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ia_aircleaner_add_periodic_timer(cl_handle_t dev_handle, cl_air_timer_t* time_info);

/*
 ����:
    ɾ�����ڶ�ʱ��
 �������:
    @timer_id : ��ʱ��ID
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ia_aircleaner_del_periodic_timer(cl_handle_t dev_handle, u_int8_t timer_id);


/**************************************************************************************************
	  ����¯/ů����������Ϣ ����  DF-HDW2001RA
 **************************************************************************************************/

typedef struct {
	/* ͨ�ϵ���� 0��ʾ�ػ��� 1��ʾ���� */
	u_int16_t onoff;
	
	/* ��λ 0: �� 1:�� 2:�� 3:�� */
	u_int16_t gear;
	
	/* ԤԼ��ʱ 
	*	0: ȡ��
	*	60:1h      120:2h      180:3h
	*	240:4h     300:5h      360:6h
	*	420:7h     480:8h      540:9h
	*	600:10h    660:11h     720:12h
	*	780:13h    840:14h     900:15h
	*	ע�⣺
	*	1.��ǰΪ�ػ�״̬����ʾʲôʱ�򿪻�
	*
	*	2.��ǰΪ����״̬����ʾʲôʱ��ػ�
	*
	*	��ԤԼʱ�仹ʣ�¶��ٷ��ӡ�ע��
	*	3.ֻ����ԤԼʱ�䲻Ϊ0�������Ч
	*	4.��ǰΪ�ػ�״̬����ʾʲôʱ�򿪻�
	*	5.��ǰΪ����״̬����ʾʲôʱ��ػ�
	*	ԤԼʱ�䵽��ִ����Ӧ��/�ػ�������ͬʱ��ԤԼ��ʱ״̬��Ϊȡ��
	*/
	u_int16_t time;

	/* ���� 0: �� 1:˯�� 2:ʡ�� 3:���� 4:���� */
	u_int16_t mode;

	/* ���� */
	u_int16_t power;

	/* ��ǰ�¶� */
	u_int16_t temp;
} cl_ia_airheater_info_t;



/*
	����:
		���ؿ���¯
	�������:
		@dev_handle:  ����¯�ľ��
		@onoff:  1,����0����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_set_onoff(cl_handle_t dev_handle, bool onoff);



/*
	����:
		����¯��λ����
	�������:
		@dev_handle:  ����¯�ľ��
		@gear:  1:�� 2:�� 3:��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_set_gear(cl_handle_t dev_handle, u_int16_t gear);



/*
	����:
		����¯ԤԼʱ������
	�������:
		@dev_handle:  ����¯�ľ��
		@time: 
			*	0: ȡ��
			*	60:1h      120:2h      180:3h
			*	240:4h     300:5h      360:6h
			*	420:7h     480:8h      540:9h
			*	600:10h    660:11h     720:12h
			*	780:13h    840:14h     900:15h
			*	ע�⣺
			*	1.��ǰΪ�ػ�״̬����ʾʲôʱ�򿪻�
			*
			*	2.��ǰΪ����״̬����ʾʲôʱ��ػ�
			*
			*	��ԤԼʱ�仹ʣ�¶��ٷ��ӡ�ע��
			*	3.ֻ����ԤԼʱ�䲻Ϊ0�������Ч
			*	4.��ǰΪ�ػ�״̬����ʾʲôʱ�򿪻�
			*	5.��ǰΪ����״̬����ʾʲôʱ��ػ�
			*	ԤԼʱ�䵽��ִ����Ӧ��/�ػ�������ͬʱ��ԤԼ��ʱ״̬��Ϊȡ��
	
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_set_time(cl_handle_t dev_handle, u_int16_t time);


/*
	����:
		����¯ģʽ����
	�������:
		@dev_handle:  ����¯�ľ��
		@mode:  1:˯�� 2:ʡ�� 3:���� 4:����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_set_mode(cl_handle_t dev_handle, u_int16_t mode);


/**************************************************************************************************
	  ����¯/ů����������Ϣ ������͡
 **************************************************************************************************/


typedef struct {
	/* �����¶� 1-35 */
	u_int16_t set_temp;

	/* ��ǰ�¶� 0-51 */
	u_int16_t cur_temp;

	/* 1:˯�� 2:ʡ�� 3:���� 4:���� 5:�¿� */
	u_int16_t mode;
	
	/* 1:�ر� 2:�͵� 3:�е� 4:�ߵ� */
	u_int16_t gear;
	
	/* ԤԼ��ʱʱ�� 
	*	����ʱ��ȡ60����(Сʱ)��������
	*	��ѯ������ʱ�䣬��ʾִ�е���ʱ��
	*/
	u_int16_t time; //��ʱ�ػ�ʣ��ʱ��

	u_int16_t onoff;
    
    	u_int16_t time_on;/*ԤԼ����ʣ��ʱ��*/

	u_int16_t time_set_off; /*���õĶ�ùػ�*/
	u_int16_t time_set_on; /*���õĶ�ÿ���*/
} cl_ia_airheater_ycyt_info_t;




/*
	����:
		���ÿ���¯/ů��������¶�
	�������:
		@dev_handle:  ��ˮ���ľ��
		@temp:  ��ΧΪ1-35
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_ycyt_set_temp(cl_handle_t dev_handle, u_int16_t temp);




/*
	����:
		�������ÿ���¯/ů�������ģʽ
	�������:
		@dev_handle:  ��ˮ���ľ��
		@mode:  	 1:˯�� 2:ʡ�� 3:���� 4:���� 5:�¿�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_ycyt_set_mode(cl_handle_t dev_handle, u_int16_t mode);




/*
	����:
		�������ÿ���¯/ů���������λ
	�������:
		@dev_handle:  ��ˮ���ľ��
		@gear:  	  1:�ر� 2:�͵� 3:�е� 4:�ߵ�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_ycyt_set_gear(cl_handle_t dev_handle, u_int16_t gear);





/*
	����:
		�������ÿ���¯/ů���������λ
	�������:
		@dev_handle:  ��ˮ���ľ��
		@time:  	  ��λСʱ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_airheater_ycyt_set_timer(cl_handle_t dev_handle, u_int16_t time);
    
/*
 ����:
  ���ÿ������߹ر���͡
 �������:
 @dev_handle:  ��ˮ���ľ��
 @on_off:  	  ����/�ر�
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ia_airheater_ycyt_set_onoff(cl_handle_t dev_handle, bool onoff);

/*
 ����:
 ������͡ԤԼ����ʱ��
 �������:
 @dev_handle:  ��ˮ���ľ��
 @time:  	  ��λСʱ
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ia_airheater_ycyt_set_order_timer(cl_handle_t dev_handle, u_int16_t time);

/*
 ����:
ˢ����͡��ʱʱ�� �͵���ʱʱ��
 @dev_handle:  ��ˮ���ľ��
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_ia_airheater_ycyt_refresh_timer(cl_handle_t dev_handle);


/**************************************************************************************************
	  ��ˮ���������Ϣ 
 **************************************************************************************************/

typedef struct {
	/* ����״̬ 
	*  ���ڲ�ѯ��0��ʾ�����У�1������
	*  ���ڿ��ƣ�0��ʾ����ˮ��ֹͣ���� 1 ��ʾ����ˮ����ʼ����(����) 
	*/
	u_int16_t work;
	
	/* ����ˮ�� ��ΧΪ35-75����ʾ�������õ����϶� */
	u_int16_t temp_set;

	/* ��ǰˮ�� ʵ�ʵ����϶� */
	u_int16_t temp_current;

	/* ԤԼʱ��(����) 
	* 0ȡ����ʱ����ʾ���õĶ�ʱ�����������Ϊ24Сʱ(24*60)
	* ע��
	*    �������Ҫ�󣬴����ʱ�������60�ı�����Ҳ����Сʱ��
	*���������ǰ���ڴ���״̬����ʱ��ʾ��ʱ��ʼ����
	*    �����ǰ���ڹ���״̬����ʱ��ʾ��ʱ�رգ��������ģʽ
	*/
	u_int16_t timer;
	
	/* ���� 1��ʾ�뵨�� 2��ʾȫ�� */
	u_int16_t capactity;

	/* ���� 100WΪ��λ */
	u_int16_t power;
} cl_ia_waterheater_info_t;






/*
	����:
		������ˮ���Ƿ���
	�������:
		@dev_handle:  ��ˮ���ľ��
		@work:  0��ʾ����ˮ��ֹͣ���� 1 ��ʾ����ˮ����ʼ����(����) 
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_set_work(cl_handle_t dev_handle, bool work);




/*
	����:
		������ˮ�������¶�
	�������:
		@dev_handle:  ��ˮ���ľ��
		@temp:  ��ΧΪ35-75
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_set_temp(cl_handle_t dev_handle, u_int16_t temp);



/*
	����:
		������ˮ����ԤԼʱ��
	�������:
		@dev_handle:  ��ˮ���ľ��
		@timer:  	* 0ȡ����ʱ����ʾ���õĶ�ʱ�����������Ϊ24Сʱ(24*60)
				* ע��
				*    �������Ҫ�󣬴����ʱ�������60�ı�����Ҳ����Сʱ��
				*���������ǰ���ڴ���״̬����ʱ��ʾ��ʱ��ʼ����
				*    �����ǰ���ڹ���״̬����ʱ��ʾ��ʱ�رգ��������ģʽ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_set_timer(cl_handle_t dev_handle, u_int16_t timer);


/*
	����:
		������ˮ������
	�������:
		@dev_handle:  ��ˮ���ľ��
		@capactity:  1��ʾ�뵨�� 2��ʾȫ�� 
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_set_capactity(cl_handle_t dev_handle, u_int16_t capactity);


/**************************************************************************************************
	  ǰ����ˮ���������Ϣ 
 **************************************************************************************************/
typedef struct {
	/* ����ˮ�� ��ΧΪ35-65����ʾ�������õ����϶� */
	u_int16_t temp_set;

	/* ��ǰˮ�� ʵ�ʵ����϶� */
	u_int16_t temp_current;

	/* ����: 1 �Զ� 2 ԡ�� 3 ϴ�� 4 ϴ�� 5 ϴ�� */
	u_int16_t mode;

	/* ����״̬ 
	* bit 0: ����Ƿ���(0 �ر� 1 ����)    
	* bit 1: ˮ���Ƿ���(0 �ر� 1 ����)
	* bit 2: �Ƿ���ȼ��(0 �� 1 ��)   
	* bit 3: �Ƿ�����Ϊ������(0 �� 1 ��)   
	* bit 4: �Ƿ���С(0 �� 1 ��)
	* bit 5: �Ƿ�Ϊ������(0 �� 1 ��)
	* bit 6: �Ƿ�T1(0 �� 1 ��)
	* bit 7: �Ƿ�T2(0 �� 1 ��)
	*/
	u_int16_t work;

	/* ȼ�շֶ�
	*  0����ȼ��
	*  1����ȼ��
	*  2����ȼ��
	*  3��ȫȼ��
	*/
	u_int16_t fire_level;
	
	/* �ۼ�ˮ���� ��λ:L */
	u_int16_t count;

	/* ������ 0-6 */
	u_int16_t gas;
} cl_ia_waterheater_a9_info_t;


/*
	����:
		������ˮ��ȼ�շֶ�
	�������:
		@dev_handle:  ��ˮ���ľ��
		@level:  	0����ȼ��
	   			1����ȼ��
	   			2����ȼ��
	   			3��ȫȼ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_a9_set_fire_level(cl_handle_t dev_handle, u_int16_t level);


/*
	����:
		�����ˮ��ˮ������¼
	�������:
		@dev_handle:  ��ˮ���ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_a9_clear_cnt(cl_handle_t dev_handle);


/*
	����:
		������ˮ������ģʽ
	�������:
		@dev_handle:  ��ˮ���ľ��
		@mode:  	 1 �Զ� 2 ԡ�� 3 ϴ�� 4 ϴ�� 5 ϴ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_a9_set_mode(cl_handle_t dev_handle, u_int16_t mode);





/*
	����:
		������ˮ�������¶�
	�������:
		@dev_handle:  ��ˮ���ľ��
		@temp:  ��ΧΪ35-65
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_waterheater_a9_set_temp(cl_handle_t dev_handle, u_int16_t temp);









/**************************************************************************************************
	  �յ��������Ϣ 
 **************************************************************************************************/

typedef struct {
	/* ͨ�ϵ���� 0��ʾ�ػ��� 1��ʾ���� */
	u_int16_t onoff;

	/* ģʽ 0 �Զ� 1 ���� 2 ���� 3 ��ʪ 4 ͨ�� */
	u_int16_t mode;

	/* �¶� 16-32 ��ʾ��Ӧ�¶� */
	u_int16_t temp;

	/* ���� */
	u_int16_t power;

	/* ��ʱ */
	u_int16_t timer;

	/* ��ǰ�¶� */
	u_int16_t cur_temp;
} cl_ia_aircondition_info_t;



/*
	����:
		���ÿյ����ء�ģʽ���¶�
	�������:
		@dev_handle:  �յ��ľ��
		@onoff:  0��ʾ�ػ��� 1��ʾ����
		@mode:  0 �Զ� 1 ���� 2 ���� 3 ��ʪ 4 ͨ��
		@temp:  16-32 ��ʾ��Ӧ�¶�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircondition_set(cl_handle_t dev_handle, bool onoff, u_int16_t mode, u_int16_t temp);



/*
	����:
		���ÿյ���ʱ��
	�������:
		@dev_handle:  �յ��ľ��
		@timer:  0��ʾȡ����ʱ������Ϊ���õĶ�ʱ�����������(24 * 60)����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_aircondition_set_timer(cl_handle_t dev_handle, u_int16_t timer);



/**************************************************************************************************
	  ���ȵ������Ϣ 
 **************************************************************************************************/

typedef struct {
	/* ����״̬: 0 ���� 1 ���� */
	u_int16_t work;

	/* ���� 1 ˯�� 2 ������ 3 ������ 4 ������*/
	u_int16_t gear;

	/* ���ö�ʱ��������Ϊ��λ */
	u_int16_t timer;

	/* ���Ȱ�ͷ 0 �ر� 1���� */
	u_int16_t shake;

	/* ���� */
	u_int16_t power;
	
	u_int16_t pad;
} cl_ia_electricfan_info_t;



/*
	����:
		���Ʒ����Ƿ���
	�������:
		@dev_handle:  ���ȵľ��
		@work:  0 ���� 1 ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_electricfan_set_work(cl_handle_t dev_handle, bool work);


/*
	����:
		���Ʒ��ȵķ���
	�������:
		@dev_handle:  ���ȵľ��
		@gear:  ���� 1: ˯�ߵ� 2: �ͷ絵 3: �з絵 4: ǿ�絵
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_electricfan_set_gear(cl_handle_t dev_handle, u_int16_t gear);


/*
	����:
		���Ʒ��ȵĶ�ʱ��
	�������:
		@dev_handle:  ���ȵľ��
		@timer:  0ȡ����ʱ������Ϊ���õĶ�ʱ�����������7.5Сʱ��30���ӵı���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_electricfan_set_timer(cl_handle_t dev_handle, u_int16_t timer);



/*
	����:
		���Ʒ��Ȱ�ͷ
	�������:
		@dev_handle:  ���ȵľ��
		@shake:  ���Ȱ�ͷ 0 �ر� 1����

	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_electricfan_set_shake(cl_handle_t dev_handle, u_int16_t shake);

/////////////////////////////////////////////////////////////
//  ԡ������

typedef struct{
	u_int16_t power_on_off; // �������ر�
	u_int16_t anion_on_off; // ������
	u_int16_t light_on_off; //����
	u_int16_t breath_on_off; //����
	u_int16_t dry_on_off; //����
	u_int16_t tronic_on_off; //��ů
	u_int16_t next_time; //�´ο�ʱ��
}cl_ia_bath_heater_info_t;

/*
	����:
		����ԡ�ԵĹ���
	�������:
		@dev_handle:  ԡ�Եľ��
		@work: �Ƿ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_work(cl_handle_t dev_handle, u_int16_t work);

/*
	����:
		���Ƹ�����
	�������:
		@dev_handle:  ԡ�Եľ��
		@anion: �Ƿ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_anion(cl_handle_t dev_handle, u_int16_t anion);

/*
	����:
		���Ƶƹ�
	�������:
		@dev_handle:  ԡ�Եľ��
		@light: �Ƿ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_light(cl_handle_t dev_handle, u_int16_t light);

/*
	����:
		���ƻ���
	�������:
		@dev_handle:  ԡ�Եľ��
		@breath: �����Ƿ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_breath(cl_handle_t dev_handle, u_int16_t breath);

/*
	����:
		���Ƹ���ģʽ
	�������:
		@dev_handle:  ԡ�Եľ��
		@dry: �����Ƿ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_dry(cl_handle_t dev_handle, u_int16_t dry);

/*
	����:
		���Ʒ�ů
	�������:
		@dev_handle:  ԡ�Եľ��
		@tronic: 0 �رշ�ů 1.��ů����2��ůǿ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_tronic(cl_handle_t dev_handle, u_int16_t tronic);

/*
	����:
		���ƶ�ʱ��
	�������:
		@dev_handle:  ԡ�Եľ��
		@timer:  0ȡ����ʱ������Ϊ���õĶ�ʱ������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_ia_bath_heater_set_timer(cl_handle_t dev_handle, u_int16_t timer);

/* ���ܼҾ����� �յ� */ 
#define IA_AIRCONDITION 1
/* ���ܼҾ����� ��ˮ�� */ 
#define IA_WATERHEATER 2
/* ���ܼҾ����� ����¯ */ 
#define IA_AIRHEATER 3
/* ���ܼҾ����� ���������� */ 
#define IA_AIRCLEANER 4
/* ���ܼҾ����� ���� */ 
#define IA_ELECTRICFAN 5
/* ���ܼҾ����� ����ԡ�� */ 
#define IA_BATHHEATER_AUPU 6

#define IA_UDP_AIR_HEATER 0x12
#define IA_UDP_AIR_CLEAR 0x13 // UDP��͡


#define IA_EXT_TYPE_LCYT_OIL 0x1 
#define IA_EXT_TYPE_813_HK 0x1 
#define IA_EXT_TYPE_813_NB 0x2 




/* �յ������� */
#define IA_AIRCONDITION_SUBTYPE_PROTO  0
/* ��ˮ�������� */
#define IA_WATERHEATER_SUBTYPE_PROTO  0
#define IA_WATERHEATER_SUBTYPE_A9	  1
/* ����¯������ */
#define IA_AIRHEATER_SUBTYPE_PROTO  0
#define IA_AIRHEATER_SUBTYPE_YCYT	  1

/* ���������������� */
#define IA_AIRCLEANER_SUBTYPE_PROTO  0
/* ���������� */
#define IA_ELECTRICFAN_SUBTYPE_PROTO  0

//��ѶLED�����
enum {	
	GX_LED_STATUS_OFF = 0,	 	 // �ص�	
	GX_LED_STATUS_ON = 1,	  	 // ����	
	GX_LED_STATUS_FULL_LIGHT = 2,// ȫ��
	GX_LED_STATUS_WHITE = 3,	 // �׹�	
	GX_LED_STATUS_WARM = 4, 	 // ů��	
	GX_LED_STATUS_MIX = 5,  	 // ���	
	GX_LED_STATUS_AUTO = 6, 	 // �Զ�	
	GX_LED_STATUS_SAVE = 7, 	 // ����	
	GX_LED_STATUS_MODEL1 = 8, 	 // MODEL1	
	GX_LED_STATUS_MODEL2 = 9, 	 // MODEL2	
	GX_LED_STATUS_MAX
};

typedef struct {
	u_int8_t led_status; // LED״̬�� ȡֵ����,�� GX_LED_STATUS_ON	
	u_int8_t brightness;		 // ����ֵ	
	u_int8_t warmness;		 // ů��ֵ	
	u_int8_t red;		 // RED	
	u_int8_t green;      // GREEN	
	u_int8_t blue;       // BLUE	
	u_int8_t night_status;// ҹ��״̬ 0-�رգ�1-����	
	u_int8_t reserved;	 // ����
} cl_ia_gx_led_info_t;

/*
	E��
*/
typedef struct{
	u_int8_t is_support_elec_info;
	u_int8_t pad[3];
	u_int32_t current_power; // ��ǰ����
	u_int32_t current_mil_power; // ��ǰ����
	cl_elec_stat_info_t elec_stat_info; //����ͳ����Ϣ
	cl_air_elec_item_info total_elec; //�ܵ���
	cl_air_elec_item_info period_elec; //�׶ε���
	cl_air_elec_item_info last_on_elec; //�ϴο�������
	cl_elec_days_stat_info elec_days_info; //����ͳ��֮365��ͳ������
}cl_common_elec_info;

typedef struct {
	u_int8_t on_off_valid;	// ���ػ�״̬�Ƿ���Ч
	// �������ر�
	u_int8_t on_off; 
	//�Ƿ�֧��ʱ��ζ�ʱ��
	u_int8_t is_support_period_timer;
    //�Ƿ�֧�ֿ���LED
    u_int8_t is_support_ctrl_led;
    //led�Ƿ���
    u_int8_t led_onoff;
	// ��ʱ��
	cl_air_timer_info_t timer;
	//����ͳ����ع���
	cl_common_elec_info elec_info;
} cl_ia_eb_info_t;

/*
*  �ʺ����̺
*/
#define MAX_CURVE_DATA_NUM 48
typedef struct {
	u_int8_t work_stat; //����
	u_int8_t set_temperature;//�û������¶�
	u_int8_t current_temperature; //������ǰ�¶�
	u_int8_t off_timer; //�ֹ���ʱʣ��ʱ��
	u_int8_t curve_enable; //�����Ƿ�����
	u_int8_t curve_week; //������������
	u_int16_t curve_time_interval;//���߼��ʱ������, 60����һ��Сʱ
	u_int16_t curve_next_work_time;//�´ο������߹ر�ʱ��
	u_int8_t curve_data_len;
	u_int8_t work_mode; //����ģʽ
	u_int8_t curve_data[MAX_CURVE_DATA_NUM];
}cl_ia_ch_area_info_t;

typedef struct {
	cl_ia_ch_area_info_t left_area_info;
	cl_ia_ch_area_info_t right_area_info;
}cl_ia_ch_blanket_info_t;


/*
* ��ȡcl_ia_info_t��Ϣ����Ҫ����cl_user_get_dev_info�ӿڷ���cl_dev_info_t*��Ȼ���ٴ�cl_dev_info_t����ȡcl_ia_info_t��
*/
typedef struct _cl_ia_info_s_{
	/* ���ܼҾ����� */ 
	u_int8_t ia_type;
	/* ���ܼҾ������� */
	u_int8_t ia_sub_type;
	u_int8_t pad[2];
	/* ��������ѡ�ͬ��� */
	union {
		/* ���������� */ 
		cl_ia_aircleaner_info_t  *aircleaner_info;
		/* ����¯ */ 
		cl_ia_airheater_info_t	 *airheater_info;
		/* ����¯ ycyt */ 
		cl_ia_airheater_ycyt_info_t	 *airheater_ycyt_info;
		/* ��ˮ�� */ 
		cl_ia_waterheater_info_t *waterheater_info;
		/* ǰ����ˮ�� */
		cl_ia_waterheater_a9_info_t *waterheater_a9_info;
		/* �յ� */ 
		cl_ia_aircondition_info_t *aircondition_info;
		/* ���� */ 
		cl_ia_electricfan_info_t  *electricfan_info;
		/*ԡ��*/
		cl_ia_bath_heater_info_t* bath_heater_info;
		/* ��Ѷ����� */
		cl_ia_gx_led_info_t *gx_led_info;
		/* E�� */
		cl_ia_eb_info_t *eb;
		//�ʺ����̺
		cl_ia_ch_blanket_info_t* ch_blanket;
		
		void *ptr;
	} u;
} cl_ia_info_t;


#ifdef __cplusplus
}
#endif 

#endif

