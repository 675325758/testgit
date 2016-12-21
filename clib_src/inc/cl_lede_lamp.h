#ifndef	__CL_LEDE_LAMP_H__
#define	__CL_LEDE_LAMP_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum{
	ACT_LEDE_CTRL_COLOR, /*����ɫ��*/
	ACT_LEDE_CTRL_COL_TEMP,/*����ɫ��*/
	ACT_LEDE_CTRL_MAX
};

typedef struct{	
	u_int8_t R;			
	/*�� 0~255*/	
	u_int8_t	B;		
	/*��0-255*/
	u_int8_t 	G; 			
	/*��0~255*/	
	u_int8_t	L;			
	/*����0~100*/
	u_int8_t cold;
	/*ɫ��*/
	u_int8_t	power;		
	/*���أ�0Ϊ�أ�1Ϊ��*/
	u_int8_t	mod_id; //�龰ģʽ���
	/*ģʽid*/	
	u_int8_t	action; //0:����RGB 1:����ɫ��
	//��Ϊ
}cl_lede_led_state_t;

#define	LEDE_LED_TIMER_FLAG_ENABLE	0x01

typedef struct {	
	u_int8_t	id;				
	/*��ʱ��id,��Чֵ��1��ʼ*/	
	u_int8_t	flags;			
	/*bit0:enable*/	
	u_int8_t	week_loop;
	/*bit0���������죬bit1��������һ���ȵȡ�Ϊ0����ѭ�������ں��Զ�ɾ��*/	
	u_int8_t	hour;			
	/*Сʱ*/
	u_int8_t	min;			
	/*��*/	
	cl_lede_led_state_t	config;	
	/*��ʱ�����ں�ʹ�ø����ø���led״̬*/
} cl_lede_led_timer_t;

typedef struct {
	u_int8_t valid;	// ���������Ƿ���Ч����Ϊ���Ʋ���������
	/**
		 1ʹ�ܿ���״̬����
               0 ��ʹ���û����ÿ���״̬�� �豸Ĭ������ʾ�ػ�ʱ״̬�� ���ҵ����ǿ�
	*/
	u_int8_t enable;
	/**
		 1 ����ʱ��ʹ��Ĭ�ϵĿ���״̬��  (�豸Ĭ������ʾ�ػ�ʱ״̬�� ���ҵ����ǿ�)
	        2 ����ʱ�������û������ã���ʾ����״̬
	        3 ����ʱ����ʾ�ػ�ʱ���״̬
	*/
	u_int8_t type;
	// �û����õĿ���ʱ��״̬
	cl_lede_led_state_t stat;
} cl_lede_led_on_stat_t;

typedef struct {
	u_int8_t timer_count;
	u_int8_t pad[3];
	cl_lede_led_state_t cur_lamp_stat;
	cl_lede_led_on_stat_t on_stat;	// �û����ÿ���ʱ���״̬
	cl_lede_led_timer_t* timer_info;	
} cl_lede_lamp_info;

/*
	����:
		���Ƶ�
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_lede_ctrl_stat(cl_handle_t dev_handle,cl_lede_led_state_t* uconfig);


/*
	����:
		���Ƶƿ��ƺ�״̬
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_lede_on_state_config(cl_handle_t dev_handle, cl_lede_led_on_stat_t *uconfig);

/*
	����:
		��Ӻ��޸�lede ��ʱ��,�޸�ʱid��Ϊ0
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_lede_ctrl_timer(cl_handle_t dev_handle,cl_lede_led_timer_t* tinfo);

/*
	����:
		ɾ���޸�lede ��ʱ��
	�������:
		@dev_handle: �����ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_lede_delete_timer(cl_handle_t dev_handle,u_int8_t timer_id);
    
///////////////////////////////////////////////////////////////////////////////
//������̨��

//������
enum{
    JL_LAMP_3200 = 0x4,
    JL_LAMP_5400,
    JL_LAMP_ALL_COLOR_TEMP,
    JL_LAMP_ALL_COLOR
};
    
typedef struct {
    u_int8_t color; //ɫ��
    u_int8_t bright; //����
    u_int8_t total_bright; //�����Ȱٷֱ�
    u_int8_t pad;
}cl_jl_3200_lamp_info_t;

typedef struct {
    u_int8_t lamp_type; //������������,�������Ͷ�Ӧ������Ϣ
    u_int8_t on_off;
    cl_jl_3200_lamp_info_t lamp_3200_info;
}cl_jl_lamp_info_t;
    
/*
 ����:
    ���Ƽ��� 3200 ��̨��
 �������:
    @dev_handle: �Ƶľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_jl_ctrl_3200_lamp(cl_handle_t dev_handle,u_int8_t on_off,u_int8_t color,u_int8_t bright);
    
/*
 ����:
	 ���Ƽ��� 3200 ��̨��
 �������:
	 @dev_handle: �Ƶľ��
 �������:
	 ��
 ����:
	 RS_OK: �ɹ�
	 ����: ʧ��
 */
CLIB_API RS cl_jl_ctrl_3200_total_bright_lamp(cl_handle_t dev_handle,u_int8_t on_off,u_int8_t color,u_int8_t bright,u_int8_t total_bright);

#ifdef __cplusplus
}
#endif 


#endif

