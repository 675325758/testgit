#ifndef	__CL_YUYUAN_H__
#define	__CL_YUYUAN_H__

#ifdef __cplusplus
extern "C" {
#endif 

/*

�����	
#define URE_EI_SHORTCUT		20100	// ����: ���������ͨ����· �� ���һλ=ͨ���ţ�ȡֵ��Χ0~3��21000~21003
			
#define URE_FV_LOC_MAIN_DEV		20201	// ����: ���ܷ�(��)��λʧ��
			
#define URE_FV_LOC_PRE_DEV		20203	// ����: ���ܷ�(Ԥ)��λʧ��
			
// task_DrugMotor.c			
#define URE_DM_STATUS		20500	// ����: ҩˮ�ù���(��·�����)�����һλ=�úţ�ȡֵ��Χ=0~2
			
// task_moni.c			
#define URW_MON_DRUG_MOTOR		10600	// ����: ҩˮ�ù���
#define URW_MON_WATER_IN_TO		10601	// ����: ��ˮ�����ͻ���ˮ����ʱ�䳬ʱ
#define URE_MON_SHORTCUT		20600	// ����: ��·����
#define URE_MON_FNC_VALVE		20601	// ����: ���ܷ�
#define URE_MON_WATER_POS		20602	// ����: ˮλ
#define URE_MON_VALVE_IN		20603	// ����: ��ˮ��ŷ�
//#define URE_MON_VALVE_CYC		20604	// ����: ѭ��/��ϴ��
			
			
// usr_function.c			
#define URE_UF_ACT_FV		20700	// ����: ִ��������ʱ���ܷ�����
			
#define   URW_LOW_POSITION                		10902	//��ˮλ����
*/

enum{
    YUYAN_LOOP_OFF = 0x0, // ��
    YUYAN_LOOP_ON = 0x1,  // ��
};
    
enum{
    YUYAN_CLEAN_ACTION_ON = 0x0,  // ��
    YUYAN_CLEAN_ACTION_OFF = 0x1, // ��
};
    
enum{
    YUYAN_WORKSTATE_ON = 0x0, // ����
    YUYAN_WORKSTATE_STANDBY, // ����
    YUYAN_WORKSTATE_FILTER,    // ����
    YUYAN_WORKSTATE_RECYCLE,   // ѭ��
    YUYAN_WORKSTATE_CLEAN,     // ��ϴ
    YUYAN_WORKSTATE_SELFCHECK, // �Լ�
    YUYAN_WORKSTATE_REBOOTING,  // ��������
    YUYAN_WORKSTATE_BACKUP,    //����
    YUYAN_WORKSTATE_PORJECT    //����
};

typedef struct {
	// ����״̬��0-������1-���ˣ�2-ѭ����3-��ϴ,4-�Լ죬5-����������APP���ݸ�״̬�ı仯���ж���һ�����������Ƿ���ɡ�ע����λ�����յ�APP��������󣬽�����״̬��Ϊ5��"��������"��ظ���ģ�飬APP�Դ˿�֪����ˮ���Ƿ���������
	u_int32_t WORK_STAT;
	// ˮλ��1-ȱ��2-�ͣ�3-�У�4-��
	u_int32_t WATER_LEVEL; 
	// ˮλ��1-ȱ��2-�ͣ�3-�У�4-��
	u_int32_t WATER_BOX; 
	// ��ˮ����Ӧ���������,�ֻ��˲�����
	u_int32_t WATER_USED_IMPULSE;
	// ��ˮ��ʱ����λS
	u_int32_t INLET_TIMEOUT;
	// �������
	u_int32_t IMPULSE_COUNT;
	// ���������ڣ���λMS
	u_int32_t IMPULSE_PERIOD;
	// ΢����λ��ʱ����λS
	u_int32_t MCDELAY;
	// ���׷���λ��ʱ����λS
	u_int32_t NM_VALVE_DELAY;
	// ���ܷ���λ��ʱʱ��
	u_int32_t FUNC_VALVE_TIMEOUT;

	// 3��������ת��
	u_int32_t SPEED1;
	u_int32_t SPEED2;
	u_int32_t SPEED3;

	// ѭ�����أ�1-����0-��
	u_int32_t LOOP_ONOFF;

	// ѭ�����ã������ڴﵽѭ������D��HʱM��ִ��TIME����ѭ��
	u_int32_t LOOPD;
	u_int32_t LOOPH;
	u_int32_t LOOPM;
	u_int32_t LOOPT; 

	// ΢�����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
	u_int32_t MCCLEAN_D;
	u_int32_t MCCLEAN_H;
	u_int32_t MCCLEAN_M;
	u_int32_t MCCLEAN_RT;
	u_int32_t MCCLEAN_DT;

	// �����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
	u_int32_t NMCLEAN_D;
	u_int32_t NMCLEAN_H;
	u_int32_t NMCLEAN_M;
	u_int32_t NMCLEAN_RT;
	u_int32_t NMCLEAN_DT;

	// ����ţ������8�����󣬴���ż�����ע��URXXX
	u_int32_t ERROR_INFO[8];

	// ����ģʽ����Ƭ��������������
	u_int8_t CONFIG[512];
} cl_yuyuan_state_t;

// ATRRI = 1
typedef struct {
	u_int32_t valid;	// had get data ?
	u_int32_t last_write_time;	// UCT
	u_int32_t count;		// ͳ�Ƶ����������365
	u_int16_t data[368];	// �����365��д368��Ϊ�˶���
} cl_yuyuan_water_history_t;

// ATRRI = 2
typedef struct {
	u_int32_t valid;	// had get data ?
	u_int32_t pwd_len;
	u_int8_t pwd[32];
} cl_yuyuan_pwd_t;

// ATRRI = 3
typedef struct {
	u_int32_t valid;	// had get data ?
	u_int32_t onoff;
	u_int32_t remind_time;	// UTC second
} cl_yuyuan_remind_t;

typedef struct{
	cl_yuyuan_state_t state;
	cl_yuyuan_water_history_t histroy;
	cl_yuyuan_pwd_t pwd;
	cl_yuyuan_remind_t remind;
} cl_yuyuan_info_t;



/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ˮ���С0-��ˮ�䣬1-Сˮ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_water_box(cl_handle_t dev_handle,u_int16_t value);

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ��ˮ��ʱ����λS����Χ0-100
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_inlet_timeout(cl_handle_t dev_handle,u_int16_t value);

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: �������������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_impulse_count(cl_handle_t dev_handle,u_int16_t value);


/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ��������������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_impulse_period(cl_handle_t dev_handle,u_int16_t value);


/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ΢����λ��ʱ����λS��Ĭ��90����Χ0-300
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_mcdelay(cl_handle_t dev_handle,u_int16_t value);

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ���׷���λ��ʱ����λS��Ĭ��90����Χ0-300
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_nm_valve_delay(cl_handle_t dev_handle,u_int16_t value);


/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: 
	�������:���ܷ���λ��ʱʱ�䣬Ĭ��120����Χ0-300
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_func_valve_timeout(cl_handle_t dev_handle,u_int16_t value);

/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@speed1-3: 3��������ת�٣���Χ0-100
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_speed(cl_handle_t dev_handle, u_int8_t speed1, u_int8_t speed2, u_int8_t speed3);


/*
	����:
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ѭ�����أ�1-����0-��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_loop_onoff(cl_handle_t dev_handle,u_int16_t value);


/*
	����:ѭ��ʱ��, �ڴﵽѭ������LPD��LPHʱLPM��ִ��LPT����ѭ��
		
	�������:
		@dev_handle: �豸�ľ��
		@day: ѭ����
		@hour: ѭ��Сʱ
		@min:ѭ������
		@hold_min: ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_loop(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t hold_min);


/*
	����:΢�����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
		
	�������:
		@dev_handle: �豸�ľ��
		@day: ѭ����
		@hour: ѭ��Сʱ
		@rt_min:����ʱ��
		@dt_min: ֱϴʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_mcclean(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t rt_min, u_int8_t dt_min);


/*
	����:�����Զ���ϴ����-ʱ-��-����ʱ����MIN��-ֱϴʱ����MIN��
		
	�������:
		@dev_handle: �豸�ľ��
		@day: ѭ����
		@hour: ѭ��Сʱ
		@min: ѭ������
		@rt_min:����ʱ��
		@dt_min: ֱϴʱ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_nmclean(cl_handle_t dev_handle,u_int8_t day, u_int8_t hour, u_int8_t min, u_int8_t rt_min, u_int8_t dt_min);

/*
	����:������ʼ΢������ϴ
		
	�������:
		@dev_handle: �豸�ľ��
		@action: 0 ��ʼ 1 ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_mincro_clean(cl_handle_t dev_handle, u_int8_t action);


/*
	����:������ʼ������ϴ
		
	�������:
		@dev_handle: �豸�ľ��
		@action: 0 ��ʼ 1 ����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_nm_clean(cl_handle_t dev_handle, u_int8_t action);


/*
	����:������ʼ�Լ졣�Լ������ģ����ȡ��ERROR�����ϱ���APP��
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_check_self(cl_handle_t dev_handle);


/*
	����:����������ˮ��
		
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_reboot_cleanner(cl_handle_t dev_handle);


/*
	����:����ģʽ���APP����һ������ģʽ�����������ڵ������滻��VALUE�����͸���λ�����������䡣VALUE�ĳ�������Ϊ50�ֽ��ڡ�
		
	�������:
		@dev_handle: �豸�ľ��
		@cmd: �Զ���AT����
		@cmd_len: �����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_config(cl_handle_t dev_handle, u_int8_t *cmd, u_int8_t cmd_len);

/*
	����:����һ�����룬���볤��С��50�ֽ�
		
	�������:
		@dev_handle: �豸�ľ��
		@value: ����
		@value_len: ���볤��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_set_pwd(cl_handle_t dev_handle, u_int8_t *value, u_int8_t value_len);

/*
	����:����������Ϣ
		
	�������:
		@dev_handle: �豸�ľ��
		@onoff: 0 �ر����� 1 ��������
		@value_len: ���볤��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_yuyuan_set_remind(cl_handle_t dev_handle, u_int32_t onoff, u_int32_t remind_time);


#ifdef __cplusplus
}
#endif


#endif


