/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_xy.h
**  File:    cl_xy.h
**  Author:  liubenlong
**  Date:    07/16/2015
**
**  Purpose:
**    ��Դ��Ŀ�ⲿͷ�ļ�.
**************************************************************************/


#ifndef CL_XY_H
#define CL_XY_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */
#define XY_SMART_WEEK_MAX			7	//һ��7��
#define XY_SMART_WEEK_TIMEPOINT		48	//һ��ֳ�48��ʱ��㣬��Сʱһ����

#define XY_SMART_DAY_TIMEPOINT_MAX	6	//һ����Է�Ϊ���6��ʱ���

#define XY_SMARTHOME_SUB_MODE_NUM	3	//����ģʽ��ģʽ����

//��������
#define XY_DROP_UPDATE_NUM			1

/* Type definitions. */
//mode
enum {
	XY_MODE_CONSTTEMP = 1,//����ģʽ
	XY_MODE_SMART = 2,//����ģʽ
	XY_MODE_HOLIDAY = 3,//�ݼ�ģʽ
};

//����ģʽ�µ���ģʽ
enum {
	XY_SSM_ONE_SERVEN = 0X0,// 1*7
	XY_SSM_FIVE_TWO,		// 5+2
	XY_SSM_FIVE_ONE_ONE,	// 5+1+1
};

//���ػ�
enum {
	XY_STATUS_OFF = 0x0,
	XY_STATUS_ON,
};

//�������ÿ���
enum {
	XY_UART_STATUS_ON = 0X1,
	XY_UART_STATUS_OFF,
};

//����״̬
enum {
	XY_HEAT_OFF = 0x0,
	XY_HEAT_ON,
};

//����״̬
enum {
	XY_LOCK_OFF = 0x0,
	XY_LOCK_ON,
};

//������������
enum {
	XY_UART_LOCK_OFF = 0x1,
	XY_UART_LOCK_ON,
};

//̽ͷ״̬
enum {
	XY_PROBE_OK = 0X0,
	XY_PROBE_ERR,
};


//ʱ��� 
#pragma pack(push, 1)
typedef struct {
	u_int8_t valid;//(0/1)
	u_int8_t start_index;//(0~48)
	u_int8_t end_index;//(0~48)
	u_int8_t temp;
}cl_xy_tp_t;

//mode
typedef struct {
	u_int8_t mode;//XY_SSM_ONE_SERVEN , XY_SSM_FIVE_TWO, XY_SSM_FIVE_ONE_ONE
	u_int8_t pad[3];
}cl_xy_st_mode_t;

typedef struct {
	//����ģʽ���ò���,�����ֶ����ȡ��һ��7*48���ڵ���¶�����
	cl_xy_st_mode_t sub_mode;
	cl_xy_tp_t timepoint[XY_SMARTHOME_SUB_MODE_NUM][XY_SMART_WEEK_MAX][XY_SMART_DAY_TIMEPOINT_MAX];//һ��ʱ��η�������
}cl_xy_smartmode_info_t;

typedef struct {
	//����ģʽ���ò���,�����ֶ����ȡ��һ��7*48���ڵ���¶�����
	cl_xy_st_mode_t sub_mode;
	//һ��ʱ��η�������,��һά���飬jni������
	cl_xy_tp_t timepoint[XY_SMARTHOME_SUB_MODE_NUM*XY_SMART_WEEK_MAX*XY_SMART_DAY_TIMEPOINT_MAX];
}cl_xy_smartmode_com_info_t;

#pragma pack(pop)


typedef struct {
	u_int8_t temp_adj;//�¶�У��ֵ+20�������¶�У��ֵΪ-3����Adj=17��Adjȡֵ��Χ11~29
	u_int8_t temp_top;//���趨�¶����ޣ�5~85��
	u_int8_t temp_tolr;//�¿��ݲ�(1-9)
	u_int8_t temp_otemp;//���ȱ����¶ȣ�15~85��
	u_int8_t temp_prottrig;//��������������ʱ��+1��1~100�������紥��ʱ��5Сʱ����ProtTrig=6
	u_int8_t temp_protlen;//����������ʱ����10~90��
}cl_xy_adjust_t;

typedef struct {
	//״̬����
	u_int8_t onoff;//0:�رգ�1:����
	u_int8_t mode;// 1:���£�2:���ܣ�3:�ݼ�
	u_int16_t root_temp;//���£���λ0.1��
	u_int16_t di_temp;//���£��������¶�
	u_int8_t cur_dst_temp;//��ǰĿ���¶�
	u_int8_t heat;//0:δ���ȣ�1:����
	u_int8_t lock;//0:δ������1:����
	u_int8_t err;//0-ʧ�� 1-�ɹ� 2-��������ȡֵ��Χ
	u_int8_t cons_temp;//����ģʽĿ���¶�
	u_int8_t holiday_temp;//�ݼ�ģʽĿ���¶�
	u_int16_t remain_time;//��8λ��ʾСʱ����8λ��ʾ����

	//�ϴη�������������˷��ص����ݸ���
	u_int8_t last_cmd;

	//�����������¶�
	u_int8_t extern_temp;

	//̽ͷ����
	u_int8_t probe_err;

	//���ܻؼҿ���
	u_int8_t smarthome_onoff;

	//��̨��������
	cl_xy_adjust_t adjust;//У������
	
	//��������ģʽ�����ȡ�
	cl_xy_smartmode_com_info_t smart_mode;
}cl_xy_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
	����:
		����
	�������:
		@dev_handle: �豸�ľ��
		@onoff:���أ�1��, 0��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_onoff(cl_handle_t dev_handle, u_int8_t onoff);

/*
	����:
		����ģʽ�µ��¶�����
	�������:
		@dev_handle: �豸�ľ��
		@temp:�¶�ֵ������������ʽ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_temp(cl_handle_t dev_handle, u_int8_t temp);

/*
	����:
		������������
	�������:
		@dev_handle: �豸�ľ��
		@onoff:���أ�1��, 0��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_lock_onoff(cl_handle_t dev_handle, u_int8_t onoff);

/*
	����:
		ģʽ����
	�������:
		@dev_handle: �豸�ľ��
		@mode:1-���� 2-���� 3-�ݼ�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
	����:
		�ݼ�ģʽʱ����������
	�������:
		@dev_handle: �豸�ľ��
		@time:99*24Сʱ
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_time(cl_handle_t dev_handle, u_int16_t time);


/*
	����:
		����ģʽ��������
	�������:
		@dev_handle: �豸�ľ��
		@st_info:����ģʽ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_config_smart_mode(cl_handle_t dev_handle, cl_xy_smartmode_com_info_t *pst_info);

/*
	����:
		��̨У������
	�������:
		@dev_handle: �豸�ľ��
		@st_info:����ģʽ��������
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_adjust(cl_handle_t dev_handle, cl_xy_adjust_t *padjust);

/*
	����:
		�ⲿ�¶�����
	�������:
		@dev_handle: �豸�ľ��
		@temp:�������������¶�
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_extern_temp(cl_handle_t dev_handle, u_int8_t temp);

/*
	����:
		���ܻؼҿ���
	�������:
		@dev_handle: �豸�ľ��
		@onoff:0�أ�1��
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_smarthome_onoff(cl_handle_t dev_handle, u_int8_t onoff);

/*
	����:
		������ģʽ����
	�������:
		@dev_handle: �豸�ľ��
		@mode:
			//����ģʽ�µ���ģʽ
			enum {
				XY_SSM_ONE_SERVEN = 0X0,// 1*7
				XY_SSM_FIVE_TWO,		// 5+2
				XY_SSM_FIVE_ONE_ONE,	// 5+1+1
			};		
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_xy_ctrl_smarthome_mode(cl_handle_t dev_handle, u_int8_t mode);




#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_XY_H */

