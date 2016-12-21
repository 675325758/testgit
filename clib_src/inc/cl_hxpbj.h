/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: hxpbj
**  File:    cl_hxpbj.h
**  Author:  liubenlong
**  Date:    05/06/2015
**
**  Purpose:
**    ��Ѹ�Ʊڻ�ͷ.
**************************************************************************/


#ifndef CL_HXPBJ_H
#define CL_HXPBJ_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
#define HX_DIY_NAME_MAX		13
#define CL_HXYSH_CUSTOMIZE_SCENE_MIN_ID 101 // �Զ����龰��ʼID��
#define CL_HXYSH_BOILING_TIME 255 //�����Ҫ��ʱ��
    
enum{
	HX_MODE_GZ = 0,	//��֭ģʽ
	HX_MODE_MH,		//�׺�
	HX_MODE_SB,		//ɳ��
	HX_MODE_MZ,		//����
	HX_MODE_ZT,		//����
	HX_MODE_DJ,		//����
	HX_MODE_XT,		//����
	HX_MODE_FR,		//���� 
	HX_MODE_DIY,	//DIY
	HX_MODE_STOP,	//STOP
    HX_MODE_DIY1,   //DIY1
    HX_MODE_DIY2,
    HX_MODE_DIY3,
    HX_MODE_DIY4,
    HX_MODE_DIY5,
    HX_MODE_DIY6,
    HX_MODE_DIY7,
    HX_MODE_DIY8,
    HX_MODE_DIY9,
	HX_MODE_MAX
};

/* Type definitions. */
typedef struct {
	//����ģʽ
	u_int8_t cur_mode;
	//��ǰ�¶�
	u_int8_t cur_tmp;
	//��ǰת��
	u_int8_t cur_speed;
	//������ɱ���
	u_int8_t cur_send_finsh;
    //��ͣ����
    u_int8_t cur_pause;
    //����ʱ��
    u_int32_t work_time;
	//���ͻ�����
	u_int8_t cur_send_err;
	//����״̬
	u_int8_t idle_status;
	//�����¶�
	u_int8_t keep_warm;
	//�Զ�������
	char name[HX_MODE_MAX][HX_DIY_NAME_MAX];
}cl_hx_info;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */

/*
	����:
		����ģʽ
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_hx_ctrl_work_mode(cl_handle_t dev_handle, u_int8_t work_mode);


/*
	����:
		�Զ��������޸ĺ���
	�������:
		@dev_handle: �豸�ľ��
		@id:�Զ���id
		@name:��/0������name�ַ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_hx_ctrl_diy_name(cl_handle_t dev_handle, u_int8_t id, char *name);


/*
	����:
		���finish״̬
	�������:
		@dev_handle: �豸�ľ��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_hx_finish_clear(cl_handle_t dev_handle);
    

////////////////////////////////////////////////////////////////////
//��Ѹ������

/*
 0x00:����״̬
 0x01����������¶�̽ͷ��·
 0x02�¶�̽ͷ��·����
 0x03�¶�̽ͷ���²���
 */
enum{
    HX_YSH_ERR_NONE = 0x0,
    HX_YSH_ERR_NO_POT,
    HX_YSH_ERR_TEMP_DETECT,
    HX_YSH_ERR_HIGH_TEMP
};

//�龰����
enum {
    //ɾ��
    HXYSH_CTRL_SCENE_ACTION_DELETE = 0X01,
    //ֹͣ
    HXYSH_CTRL_SCENE_ACTION_STOP = 0X02,
};

//work_state
enum {
    HXYSH_WORK_STATE_STANDBY = 0X00,     //����
    
    HXYSH_WORK_STATE_HEATING = 0X01,   //������
    HXYSH_WORK_STATE_HEAT_DONE = 0X02, //���Ƚ���
    
    HXYSH_WORK_STATE_KEEPING = 0X03,     //������
    HXYSH_WORK_STATE_KEEP_DONE = 0X04//���½���
};
    
// timer ��������
enum {
    HXYSH_TIMEFACTOR_MCU_TIMER = 2,         //2��
    HXYSH_TIMEFACTOR_WORK_REMAIN_TIME = 5,   //5��
    HXYSH_TIMEFACTOR_HEAT_TIME = 5,   //5��
    HXYSH_TIMEFACTOR_KEEP_TIME = 5   //5��
};
    
typedef struct {
    u_int8_t is_data_valid; //�����Ƿ���Ч
    u_int8_t on_off; //����
    u_int16_t cur_exec_id; //��ǰִ�е��龰������Ǵ�������ԤԼʱ�䲻Ϊ0�����id����ʾԤԼ���龰id
    u_int8_t work_stat; // 0:����: 1. ������ 2:�������
    u_int8_t work_remain_time; //�����ǰ�������У���ʱ���ʾ������ɵĵ���ʱ
    u_int8_t cur_power; //��ǰ���� 1-8 ��λ 100��
    u_int8_t temp; //�¶�
    u_int8_t mcu_timer; //�����ԤԼ����ʱ���ʾ����ʱִ��
    u_int8_t is_hot; //�Ƿ��ڼ�����
    u_int8_t err_no; //����״̬
    u_int16_t wifi_timer_exec_id;
	u_int16_t wifi_timer;
}cl_hx_ysh_stat_info_t;

typedef struct {
    u_int8_t temp;// �¶� 35-100
    u_int8_t time; // ����ʱ�� ��λ 10����
    u_int8_t power; //���ȹ��� 1-8 ����ʾ100-800�ߣ�
    u_int8_t keep_temp; //�����¶� 30-90
    u_int8_t keep_time; //����ʱ��
    u_int8_t pad;
    u_int16_t scene_id; //�龰id;
    u_int32_t create_time;
    char name[32];
}cl_hx_ysh_scene_t;

typedef struct{
    cl_hx_ysh_stat_info_t stat;
    u_int32_t scene_num;
    cl_hx_ysh_scene_t* scene;
}cl_hx_ysh_info_t;

/*
 ����:
    ��������������
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hx_ysh_ctrl_onoff(cl_handle_t dev_handle, u_int8_t on_off);

/*
 ����:
    ����������ִ�л�ɾ���龰
 �������:
    @dev_handle: �豸�ľ��
    @action : 1��ɾ�� : 2:ֹͣ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hx_ysh_ctrl_scene(cl_handle_t dev_handle, u_int8_t aciton, u_int16_t scene_id);

    
/*
 ����:
    ����������ִ�л�ɾ���龰
 �������:
    @dev_handle: �豸�ľ��
    @action : 1��ɾ�� : 2:ֹͣ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hx_ysh_exec_scene(cl_handle_t dev_handle, cl_hx_ysh_scene_t* scene);
/*
 ����:
    �����������޸��龰
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_hx_ysh_modify_scene(cl_handle_t dev_handle, cl_hx_ysh_scene_t* scene);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */



#endif /* CL_HXPBJ_H */

