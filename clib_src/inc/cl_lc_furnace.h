#ifndef	__CL_LC_FURNACE_H__
#define	__CL_LC_FURNACE_H__

/*
ah : air heater
����ů���DF-HT5313P
�����ṩ6621ģ�飬ͨ��ʹ��udpЭ��
*/

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"

//event of air heater
enum {
	AHE_BEGIN = 1400,
	AHE_CTRL_OK = AHE_BEGIN + 1, //���Ƴɹ���������Դ��ҡͷ����λ��ECO
	AHE_CTRL_FAIL = AHE_BEGIN + 2, //����ʧ��
	AHE_CTRL_TEMP_OK  = AHE_BEGIN + 3, // �����¶ȳɹ�
	AHE_CTRL_TEMP_FAIL  = AHE_BEGIN + 4, // �����¶�ʧ��
	AHE_CTRL_TIMER_OK  = AHE_BEGIN + 5, // ���ö�ʱ�ɹ�
	AHE_CTRL_TIMER_FAIL  = AHE_BEGIN + 6, // ���ö�ʱʧ��
	AHE_END = AHE_BEGIN + 99
};

#define AH_HEAT_MODE_LOW 0x2
#define AH_HEAT_MODE_HIGH 0x3

//ů�������״̬
typedef struct{
	// true��ʾ��Դ��
	u_int8_t is_on;
	//  AH_HEAT_MODE_XX ů�����λ: �͵����ߵ�
	u_int8_t heat_mode; 
	//true��ʾҡͷ
	u_int8_t is_shake; 
	//true��ʾ������ECOģʽ
	u_int8_t is_eco;
	//ture��ʾ��������ڹ���״̬�����ڼ���
	u_int8_t is_heating;
	
	//�㵹����
	u_int8_t is_topple_protect;
	// �����¶ȹ��߱���
	u_int8_t is_temp_high_protect;
	//�������¶ȹ��߱���
	u_int8_t is_furnace_high_protect;
	//ů����������
	u_int8_t is_furnace_error;

	//�趨���¶�
	u_int8_t set_temp;
	//�����¶�ֵ
	u_int8_t room_temp;
	//������1�¶�ֵ
	u_int8_t thermode_1_temp;
	//������2�¶�ֵ
	u_int8_t thermode_2_temp;	
	//ʣ�ඨʱʱ�䣬��λ����
	u_int16_t remain_minute;
	//������ȥ��Сʱ
	u_int8_t set_hour;
	u_int8_t timer_type;
	//��ǰ���ĵ���ֵ
	u_int32_t epower;		
}cl_ah_work_stat_t;

     
    
//����ů�����Ϣ
typedef struct _cl_ah_info_s{
	cl_handle_t handle; //�豸���
	cl_ah_work_stat_t ah_work_stat;
}cl_ah_info_t;


/*
 ����:
    ���Ƶ�Դ����
 �������:
    @is_on,  true:ON  false:OFF
 �������:
 ��
 ����:
 RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_ah_ctrl_power(cl_handle_t dev_handle, u_int8_t is_on);
    
/*
 ����:
    ���Ƽ��ȵ�λ
 �������:
    @mode: AH_HEAT_MODE_LOW AH_HEAT_MODE_HIGH
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_ah_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
 ����:
    �����¶�
 �������:
    @is_add, true: �����¶� false: �����¶�
 �������:
 ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_ah_ctrl_temp(cl_handle_t dev_handle, u_int8_t is_add);
    
/*
����:
    ���ƶ�ʱ��
�������:
    @is_add: true: ���Ӷ�ʱ�� false: ���ٶ�ʱ��
�������:
    ��
����:
 RS_OK: �ɹ�
����: ʧ��
*/
CLIB_API RS cl_sa_ah_ctrl_timer(cl_handle_t dev_handle, u_int8_t is_add);

/*
 ����:
    ����ECO
 �������:
    @is_eco: true: ����ECOģʽ false: �ر�ECOģʽ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_ah_ctrl_eco(cl_handle_t dev_handle, u_int8_t is_eco);

/*
 ����:
    ����ҡͷ
 �������:
    @@is_shake: true: ����ҡͷ false: �ر�ҡͷ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_sa_ah_ctrl_shake(cl_handle_t dev_handle, u_int8_t is_shake);

CLIB_API void cl_sa_ah_refresh_power_and_timer(cl_handle_t dev_handle);
    
/*
 ����:
    ������֤�ýӿ�
    ���ڶ�ʱ����ʱ������ָ��
 �������:
    @@timeout: ��ʱʱ�� 0��ֹͣ 1-10 ��ʾ100-1000����ʱ����
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_sa_ah_ctrl_test(cl_handle_t dev_handle, u_int8_t timeout);

    
#ifdef __cplusplus
}
#endif 

#endif

