#ifndef	__CL_TB_HEATER_PUMP_H__
#define	__CL_TB_HEATER_PUMP_H__

#ifdef __cplusplus
extern "C" {
#endif 


typedef struct{
	/* �ӻ��¿�ID */    
	u_int8_t cid;    
	/* ���ػ� */    
	u_int8_t onoff;   
	/* ����ģʽ */   
	u_int8_t work_mode;   
	/* �¶�ֵ */    
	u_int8_t temp;    
	/* Ԥ�� */    
	u_int8_t reserved[4];
}cl_tb_user_config_t;


typedef struct  {
	 /* �ӻ��¿�ID */    
	u_int8_t cid;    
	/* ��ý�ؿ��� */    
	u_int8_t return_cold_switch;    
	/* �豸��װ״̬ */    
	u_int16_t facility_state;  
	/* ϵͳ����ѡ�� */   
	u_int16_t sysfunc;    
	/* �ز��¶� */    
	u_int8_t return_diff_temp;    
	/* ���ȳ�˪���� */    
	u_int8_t heat_defrost_circle;    
	/* ���Ƚ����˪�¶� */   
	u_int8_t start_heat_defrost_temp;    
	/* �����˳���˪�¶� */    
	u_int8_t stop_heat_defrost_temp;    
	/* �����˳���˪ʱ�� */    
	u_int8_t stop_heat_defrost_time;    
	/* ����������趨ֵ */    
	u_int8_t eheat_value;    
	/* ������ʱ�ر�ʱ�� */    
	u_int8_t backlight_delay_time;    
	/*����ģʽ*/
	u_int8_t fan_mode;
	u_int8_t reserved[2];
}cl_tb_work_config_t;
	

typedef struct  {
	/*�ӻ��¿�ID*/
	u_int8_t cid;
	u_int8_t mode;   /* 0���ֶ���1���Զ� */
	u_int8_t year;   /* 0 ~ 99 (2000 ~ 2099)*/
	u_int8_t month;  /* 1 ~ 12 */
	u_int8_t mday;   /* 1 ~ 31 */
	u_int8_t hour;   /* 0 ~ 23 */
	u_int8_t minute; /* 0 ~ 59 */
	u_int8_t second; /* 0 ~ 59 */
}cl_tb_rtc_t;

typedef struct {
	/* �ӻ��¿�ID */    
	u_int8_t cid;   
	/* �����¶� */    
	u_int8_t env_temp;    
	/* ˮ���²��¶� */    
	u_int8_t tankbottom_temp;    
	/* ˮ���ϲ��¶� */   
	u_int8_t tanktop_temp;   
	/* �̹��¶� */  
	u_int8_t coil_temp;   
	/* �����¶� */  
	u_int8_t air_temp;    
	/* �����¶� */    
	u_int8_t returnair_temp;  
	/* ��ˮ�¶� */   
	u_int8_t outwater_temp;  
	/* ��ˮ�¶� */   
	u_int8_t inwater_temp;   
	/* ��ˮ�¶� */    
	u_int8_t returnwater_temp;   
	/* ������ */    
	u_int16_t heat_capacity;   
	/* ����ʱ�� */   
	u_int16_t heat_time;   
	/* �ĵ��� */   
	u_int16_t consumption_power
	;    /* ��ʡ���� */   
	u_int16_t saving_power;  
	u_int16_t reserved;
}cl_tb_temp_info_t;

typedef struct {
	/* �ӻ��¿�ID */  
	u_int8_t cid;   
	/* ���ӷ����� */  
	u_int8_t valve_expansion; 
	/* �ӻ��豸�ϵ�״̬ */   
	u_int16_t slave_onoff;   
	/* �豸/����1 */  
	u_int16_t dev_fault;   
	/* �豸/����2 */   
	u_int16_t dev_guard;  
	u_int16_t load_state;
	u_int16_t reserved;
}cl_tb_fault_stat;
    
#define TB_ERR_NONE   (0)
    /* �����ɹ� */
#define TB_ERR_OK    (127)
    /* δ���� */
#define TB_ERR_NET   (128)
    /* ��������ʧ�� */
#define TB_ERR_DOWN  (129)
    /* ����У��ʧ�� */
#define TB_ERR_IMG   (130)
    /* ���ϵĴ����������, ����Ĵ�����ϵ���̴��� */
    /* �����߿�ʧ�� */
#define TB_ERR_MCU0  (131)
    /* ������ذ�ʧ�� */
#define TB_ERR_MCU1  (132)

typedef struct {
	/*�ӻ��¿�ID*/
	u_int8_t cid;
	/* ������Ϣ */
	u_int8_t dev_info;
	/* ģʽѡ�� */
	u_int8_t dev_mode;
	/* �̼��汾�� */
	u_int8_t fw_version;
	/* ���ذ�̼��汾�� */
	u_int8_t mb_version;
	u_int8_t svn_version;
    u_int8_t stm_up_stat; //��Ƭ������״̬������
	u_int8_t reserved;
}cl_tb_other_info;

typedef struct{
	u_int16_t  bind_state;  
	u_int8_t tb_sn[24]; 
	cl_tb_user_config_t u_config;
	cl_tb_work_config_t w_config;
	cl_tb_temp_info_t temp_info;
	cl_tb_fault_stat fault_stat;
	cl_tb_other_info other_info;	
}cl_tb_info_t;

/*
	����:
		����״̬
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tb_ctrl_stat(cl_handle_t dev_handle,cl_tb_user_config_t* uconfig);

/*
	����:
		���ù�������
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tb_setting_work_param(cl_handle_t dev_handle,cl_tb_work_config_t* wconfig);

/*
	����:
		ˢ���¶���Ϣ
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tb_refresh_temp_info(cl_handle_t dev_handle);

/*
	����:
		ˢ��������Ϣ
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tb_refresh_other_info(cl_handle_t dev_handle);

/*
	����:
		���������к�
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tb_bind_dev_bar_code(cl_handle_t dev_handle,u_int8_t* bar_code);

#ifdef __cplusplus
}
#endif 


#endif

