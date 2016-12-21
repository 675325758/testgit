/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_qpcp.h
**  File:    cl_qpcp.h
**  Author:  liubenlong
**  Date:    05/12/2015
**
**  Purpose:
**    ǧ������.
**************************************************************************/


#ifndef CL_QPCP_H
#define CL_QPCP_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
#define 		CL_QPCP_NAME_MAX_LEN		32
    
#define CL_QPCP_CUSTOMIZE_SCENE_MIN_ID 5000 // �Զ����龰��ʼID��

/* Type definitions. */
//����ģʽ
/*
����ģʽ��
����ÿλΪ1��ʾ�������ڽ��У�0��ʾû�н��С�
BIT0��������1Ϊ�Ѿ��ǿ���״̬��0Ϊ������
BIT1����ˮ�����ڼ�ˮ
BIT2����ˮ��������ˮ
BIT3����ˮ�����ڱ���
BIT4�����������ڼ�ˮ
BIT5��������������ˮ
BIT6�����������ڱ���
*/
enum {
	//����
	QPCP_MODE_WAIT = 0,
	//�Զ��ݲ�
	QPCP_MODE_AUTO_TEA,
	//����
	QPCP_MODE_DISINFECT,
	//��ˮ
	QPCP_MODE_HEAT_WATER,
	//�Զ���ˮ
	QPCP_MODE_AUTO_HEAT_WATER,
	//�Զ�����
	QPCP_MODE_AUTO_DISINFECT,
	//��ˮ
	QPCP_MODE_ADD_WATER,
};

//u_int8_t mode; ����ģʽ:1���ֶ� 2: �Զ�
enum {
    //��ˮ
    QPCP_MODE_MANUAL = 0X01,
    //����
    QPCP_MODE_AUTO = 0X02,
};
    
//��������Ϣ
enum {
	QPCP_ERR_NONE = 0X00,
	//����������¶�̽ͷ��·
	QPCP_ERR_TH = 0X01,
	//�¶�̽ͷ��·����
	QPCP_ERR_DL = 0X02,
	//�¶�̽ͷ���²���
	QPCP_ERR_GW = 0X03,
	//��ˮ������ˮ״̬
	QPCP_ERR_NO_WATER = 0X04,
	//��ת��ͷ�ź��쳣
	QPCP_ERR_LTE = 0X05,
};

//����״̬
enum {
	//����
	QPCP_PRODUCT_WAIT = 0X00,
	//������
	QPCP_PRODUCT_DOING = 0X01,
	//�������
	QPCP_PRODUCT_FINISHED = 0X02,
};

//action����
enum {
	//��ˮ
	QPCP_ACTION_BOIL = 0X01,
	//����
	QPCP_ACTION_DISINFECT = 0X02,
};

//��ˮ��ͷ״̬
enum{
	QPCP_WATER_STATE_NONE = 0X0,//ˮ��ͷ����
	QPCP_WATER_STATE_BOIL_ADDING,//��ˮ��ˮ��
	QPCP_WATER_STATE_BOIL_BACKING,//��ˮ��λ��	
	QPCP_WATER_STATE_DISINFECT_ADDING,//������ˮ��
	QPCP_WATER_STATE_DISINFECT_BACKING,//������λ��	
};

//�ֽڶ���
#pragma pack(push, 1)

//�龰ͨ������ cl_qpcp_scene_com_param_t
typedef struct {
	u_int8_t action;	//��ˮ������
	u_int8_t temp;	//�趨�¶�(90-100)
	u_int8_t thermal_temp;	//�����¶ȱ����¶�(40-100)
	u_int8_t thermal_time;	//����ʱ�䱣��ʱ��(1-8 hours)
	u_int8_t time;	//�趨ʱ��ʱ�� (1-240 ����)
	u_int8_t power;	//�趨����(1-9)��ʾ100-900w
	u_int8_t water_time;	//��ˮʱ��(5-30s)
	u_int8_t index; //��ˮ�������ڲ�index
}cl_qpcp_scp_t;

//�����龰���ýṹ
typedef struct {
	u_int16_t scene_id;	//id
	u_int16_t pad;
	cl_qpcp_scp_t com_param;	
	u_int32_t create_time;	//����ʱ��
	u_int8_t name[CL_QPCP_NAME_MAX_LEN];	//����
}cl_qpcp_sp_t;

#pragma pack(pop)


//cl_qpcp_scene_hd_t���龰�����ѯͷ��
typedef struct {
	u_int8_t num;
	cl_qpcp_sp_t scene[0];
}cl_qpcp_sh_t;

//��ǰ��ˮ��������״̬
typedef struct {
	u_int8_t temp;	//�¶�����(90-100)
	u_int8_t power;	//��������(100-900w)
	u_int8_t time;	//ʱ�� (1-240 ����)
	u_int8_t thermal_temp;	//�����¶�(40-100)
	u_int8_t thermal_time;	//����ʱ��(1-8 hours)
}cl_qpcp_config_t;

typedef struct {
	
	cl_qpcp_config_t disinfect;	//��ǰ������������
	cl_qpcp_config_t boil;	//��ǰ��ˮ��������

    u_int8_t cur_onof;	//���ػ�״̬
    u_int8_t cur_mode;	//��ǰ����ģʽ
	u_int8_t cur_water_temp;	//��ǰ��ˮ�¶�
	u_int8_t cur_water_time;	//��ǰ��ˮ������ˮʱ��
    
	u_int8_t cur_water_state;	//��ǰ��ˮ״̬
	u_int8_t cur_error;	//��ǰ������
	u_int8_t cur_remain_water_time;	//��ǰʣ���ˮʱ��
	u_int8_t cur_production_status;	//��ǰ����״̬
    
    u_int16_t disinfect_plan_id; //��ǰ����ִ�е������龰ID
    u_int16_t boil_plan_id; //��ǰ����ִ�е���ˮ�龰ID
    
    u_int8_t disinfect_index; //��ǰ����ִ�е��������
    u_int8_t boil_index; //��ǰ����ִ�е���ˮ���
    u_int8_t mode; //����ģʽ:1���ֶ� 2: �Զ�
    u_int8_t water_warning; //��ˮ����
    
	cl_qpcp_sh_t *pscene;	//����

}cl_qpcp_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	����:
		����
	�������:
		@dev_handle: �豸�ľ��
		@onoff:���أ�0�أ�1��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_qpcp_ctrl_on(cl_handle_t dev_handle, u_int8_t onoff);

/*
	����:
		��ˮ����
	�������:
		@dev_handle: �豸�ľ��
		@action:��ˮ������,1��ˮ,2����
		@time:��ˮʱ�䣬(5-30s)����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_qpcp_add_water(cl_handle_t dev_handle, u_int8_t action, u_int8_t time);

/*
	����:
		�ֶ���ˮ��������
	�������:
		@dev_handle: �豸�ľ��
		@param:�ֶ���ˮ������������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_qpcp_handle_ctrl(cl_handle_t dev_handle, cl_qpcp_scp_t *param);


/*
	����:
		�龰ɾ������Ϊ��������ɾ������ò�ͬ�ӿ�
	�������:
		@dev_handle: �豸�ľ��
		@id: �龰id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_qpcp_scene_del(cl_handle_t dev_handle, u_int16_t id);

/*
	����:
		�龰�޸�
	�������:
		@dev_handle: �豸�ľ��
		@param: �龰����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_qpcp_scene_modify(cl_handle_t dev_handle, cl_qpcp_sp_t *param);

/*
	����:
		�龰ִ��
	�������:
		@dev_handle: �豸�ľ��
		@id: �龰id
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_qpcp_scene_execute(cl_handle_t dev_handle, u_int16_t id, cl_qpcp_scp_t *param);
    
/*
 ����:
    ����
 �������:
    @dev_handle: �豸�ľ��
    @onoff:���أ�0�أ�1��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_qpcp_reset_fault(cl_handle_t dev_handle);
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ǧ����
    
/**********************************
 ǧ����������
 0���޴���
 1--̽ͷ1��·��������̽ͷ1��·��������ֹͣ��������������ԡ�E1��������
 2--̽ͷ2��·��������̽ͷ2��·��������ֹͣ��������������ԡ�E3��������
 3--̽ͷ1��·��������̽ͷ1��·��������ֹͣ��������������ԡ�E2��������
 4--̽ͷ2��·��������̽ͷ2��·��������ֹͣ��������������ԡ�E4��������
 5--���׳��±�������̽ͷ��⵽�¶ȳ���200��ʱ��������ֹͣ��������������ԡ�E5��������
 6--��ѹ���ͱ���������ѹ����100Vʱ��������ֹͣ��������������ԡ�E7��������
 7--��ѹ���߱���������ѹ����275Vʱ��������ֹͣ��������������ԡ�E8��������
 8--ɢ�����������迪·��������ɢ�����������迪·ʱ��������ֹͣ��������������ԡ�E9��������
 9--ɢ�������������·��������ɢ�������������·ʱ��������ֹͣ��������������ԡ�E10��������
 10--ɢ�����������賬�±�������ɢ�����������賬��ʱ��������ֹͣ��������������ԡ�E11��������
******************************/
enum{
    QPP_STAT_IDLE, //����
    QPP_STAT_COOKING, //���
    QPP_STAT_KEEP_TEMP,//����
    QPP_STAT_WAIT_EXEC_TASK,//��ִ��ԤԼ����
    QPP_STAT_MAX
};
    
enum{
    QPP_MODE_UNKNOWN, //δ֪
    QPP_MODE_RICE,//��
    QPP_MODE_GUREL,//����
    QPP_MODE_SOUP,//����
    QPP_MODE_STEW,//����
    QPP_MODE_BAKING, //����
    QPP_MODE_MAX
};
    
    
typedef struct {
    u_int16_t s_id; //�龰id
    u_int16_t cook_time; //���ʱ��
    u_int8_t hot_degress;//����ǿ�ȣ��ȼ���
    u_int8_t microswitch; //΢�����ش���
    u_int8_t warm_temp; //�����¶�
    u_int8_t cooking_mode; //������� �� QPP_MODE_XXX
    u_int8_t set_action; //1 ���� 0��ȡ��
    u_int8_t rice_degress; //�׷���Ӳ��
}cl_qp_pot_scene_param_t;
    
typedef struct {
    u_int8_t name[CL_QPCP_NAME_MAX_LEN];
    u_int32_t create_time; //����ʱ��
    cl_qp_pot_scene_param_t s_info; //����
}cl_qp_pot_scene_t;
    
typedef struct {
    u_int16_t cur_id;
    u_int16_t cooking_remain_time;
    u_int8_t stat; //״̬���� QPP_STAT_XXX
    u_int8_t cooking_mode; //������� �� QPP_MODE_XXX
    u_int8_t is_complete; //�Ƿ�������
    u_int8_t is_pot_cover_open; //�����Ƿ���
    u_int8_t microswitch; //΢������
    u_int8_t warm_temp; //�����¶�
    u_int8_t pot_temp; //�����¶�
    u_int8_t food_quantity;// ʳ����
    u_int8_t err_num; //������
    u_int8_t scene_count; //�Զ����龰����
    cl_qp_pot_scene_t* scenes; //�Զ����龰
}cl_qp_pot_info_t;

    
/*
 ����:
    ���ƹ�����
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_qp_pot_ctrl(cl_handle_t dev_handle, cl_qp_pot_scene_param_t* param);

/*
 ����:
    ִ���龰
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_qp_pot_exec_scene(cl_handle_t dev_handle, cl_qp_pot_scene_param_t* param);
    

/*
 ����:
    �龰ɾ������Ϊ��������ɾ������ò�ͬ�ӿ�
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_qp_pot_del_scene(cl_handle_t dev_handle, u_int16_t s_id);

/*
 ����:
    �龰�޸�
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
 ����: ʧ��
 */
CLIB_API RS cl_qp_pot_modify_scene(cl_handle_t dev_handle, cl_qp_pot_scene_t *scene);
    
////////////////////////////////////////////////////////////////////
//ǧ���Ʊڻ�

/*
 0x00:����״̬
 0x01����������¶�̽ͷ��·
 0x02�¶�̽ͷ��·����
 0x03�¶�̽ͷ���²���
 */
enum{
    QP_PBJ_ERR_NONE = 0x0,
    QP_PBJ_ERR_NO_POT,
    QP_PBJ_ERR_TEMP_DETECT,
    QP_PBJ_ERR_HIGH_TEMP
};
    
#define QP_MODE_HOT 0x1
#define QP_MODE_MIX 0x2
    
#define MAX_QP_PBJ_STEP 0x9

typedef struct {
    u_int8_t is_data_valid; //�����Ƿ���Ч
    u_int8_t on_off; //����
    u_int16_t cur_exec_id; //��ǰִ�е��龰
    u_int8_t work_stat; // 0:����: 1. ������ 2. �������
    u_int8_t cur_mode; //��ǰģʽ
    u_int8_t mix_power; //���蹦�� 1-9
    u_int8_t temp; //�¶�
    u_int8_t err_no; //����״̬
}cl_qp_pbj_stat_info_t;

typedef struct {
    u_int8_t temp; //�����¶�
    u_int8_t time; //����ʱ�� ��λ������
}cl_qp_pbj_hot_info_t;

typedef struct {
    u_int8_t gear; //���赵λ;
    u_int8_t time; // 1-20 ��λ: ��
    u_int8_t freq; // ������� ��step_countС�ڵ���5ʱ����ΧΪ1-3����step_count����5ʱ����ΧΪ1-10
}cl_qp_pbj_mix_info_t;
    
typedef struct{
    u_int8_t data_type; //����ʹ���������ݣ�2ѡ1  1������  2������
    cl_qp_pbj_mix_info_t m_info;
    cl_qp_pbj_hot_info_t h_info;
}cl_qp_pbj_action_t;

typedef struct {
    u_int16_t scene_id;
    char name[CL_QPCP_NAME_MAX_LEN];
    u_int8_t step_count;
    u_int8_t warm_time;//����ʱ��
    u_int8_t warm_temp;//�����¶�
    cl_qp_pbj_action_t action[MAX_QP_PBJ_STEP];
}cl_qp_pbj_scene_t;
    
typedef struct{
    cl_qp_pbj_stat_info_t stat;
    u_int32_t scene_num;
    cl_qp_pbj_scene_t* scene;
}cl_qp_pbj_info_t;

/*
 ����:
    �����Ʊڻ�����
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_qp_pbj_ctrl_onoff(cl_handle_t dev_handle, u_int8_t on_off);
    
/*
 ����:
    �����Ʊڻ�ִ�л�ɾ���龰
 �������:
    @dev_handle: �豸�ľ��
    @action : 0:ִ�� 1��ɾ�� 2��ֹͣ
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_qp_pbj_ctrl_scene(cl_handle_t dev_handle, u_int8_t aciton, u_int16_t scene_id);
    
/*
 ����:
    �����Ʊڻ��޸��龰
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_qp_pbj_modify_scene(cl_handle_t dev_handle, cl_qp_pbj_scene_t* scene);
    
/*
 ����:
    �������״̬��Ϣ
 �������:
    @dev_handle: �豸�ľ��
 �������:
    ��
 ����:
    RS_OK: �ɹ�
    ����: ʧ��
 */
CLIB_API RS cl_qp_pbj_reset_fault(cl_handle_t dev_handle);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_QPCP_H */

