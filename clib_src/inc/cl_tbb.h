/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_tbb.h
**  File:    cl_tbb.h
**  Author:  liubenlong
**  Date:    08/10/2015
**
**  Purpose:
**    �ذ�����.
**************************************************************************/


#ifndef CL_TBB_H
#define CL_TBB_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */


/* Macro constant definitions. */


/*
support_mode:
	Bit0���Զ�    ��0:��   1:�У�
	Bit1������    ��0:��   1:�У�
	Bit2������    ��0:��   1:�У�
	Bit3����ů    ��0:��   1:�У�
*/
/*
pump_info:
	Bit0:1#ѹ������
	Bit1:2#ѹ������
	Bit2::3#ѹ������
	Bit3::4#ѹ������
	Bit4:����ȡ�
	Bit5:���̵���ȡ�
	Bit6:�������� ��
	Bit7:ˮ�á�
	Bit8:ֱ����Ƶˮ�á�
	Bit9:�߷硣
	Bit10:�ͷ� ��
	Bit11:1#��ͨ����
	Bit12:2#��ͨ����
	Bit13:3#��ͨ����
	Bit14:4#��ͨ���� 
	Bit15:ѭ������
*/
/*
setup_on:
���ǣ�0:��/1:�У���
	Bit0:1#��ѹ���ء�
	Bit1:1#��ѹ���ء�
	Bit2:2#��ѹ���ء�
	Bit3:2#��ѹ���ء�
	Bit4:3#��ѹ���ء�
	Bit5:3#��ѹ���ء�
	Bit6:4#��ѹ���ء�
	Bit7:4#��ѹ���ء�
	Bit8:ˮ�����ء�
	Bit9:Ӧ�����ء�
	Bit10:���򿪹ء�
	Bit11:�յ������źſ��ء�
*/
/*
sys_select:
   Ĭ�� 1
   0 ����
   1 �ȱ�
   2 ����
   3 ��ˮ
*/

/*
tmp:
	�����¶��趨ֵ(10~25)
	�����¶��趨ֵ (20~60)
	�Զ��¶��趨ֵ (8~40)	    
*/
//��Ҫ�����
#pragma pack(push, 1)
//Ӳ����Ϣ
/*
sys_type:
	ϵͳ����
	0x01         0 Ϊ ���û�ϵ��    1 ���û�1��
	0x31         3 Ϊ Ӿ�ػ�ϵ��    1 Ӿ�ػ�1��
	0x81         8 Ϊ ���û�ϵ��    1 ���û�1��
ele_band_mcu:
��ذ�MCU�ͺ�
	0x1051    105ΪSTM8S105    1 Ϊ 32K
	0x2071    207ΪSTM8S207    1 Ϊ 32K
	0x2072                     2 Ϊ 64K
	0x2073                     3 Ϊ 128K
ele_band_ver:
	��ذ�̼��汾��Ĭ��0x10 ���̼��汾��ΪV1.0
line_band_mcu:
	�߿���MCU�ͺ�
	0x1051    105ΪSTM8S105    1 Ϊ 32K
	0x2071    207ΪSTM8S207    1 Ϊ 32K
	0x2072                     2 Ϊ 64K
	0x2073                     3 Ϊ 128K
*/
/*
set_on1:
	�豸��װ״̬ 1��
	Bit0��ѭ��ˮ��
	Bit1��ѭ����
	Bit2����ϴ��
	Bit3�������
	Bit4: �ͷ翪��
	Bit5: ֱ����Ƶˮ��
	Bit6: ���̵����
	Bit7����������
	Bit8����ˮ�¶ȴ�����
	Bit9����ˮ�¶ȴ�����
	Bit10����ˮ�¶ȴ�����
	Bit11��ˮ���¶ȴ�����
	Bit12�������¶ȴ�����
	Bit13~ Bit15������
*/

/*
set_on2:
	�豸��װ״̬ 2��
	Bit0��1#�������ͷ�
	Bit1��1#�̹��¶�
	Bit2��1#�����¶�
	Bit3��1#�����¶�
	Bit4��2#�������ͷ�
	Bit5��2#�̹��¶�
	Bit6��2#�����¶�
	Bit6��2#�����¶�
	Bit8��3#�������ͷ�
	Bit9��3#�̹��¶�
	Bit10��3#�����¶�
	Bit11��3#�����¶�
	Bit12��4#�������ͷ�
	Bit13��4#�̹��¶�
	Bit14��4#�����¶�
	Bit15��4#�����¶�
*/

typedef struct {
	u_int16_t sys_type;//��Ƭ������
	u_int16_t ele_band_mcu;//��ذ�mcu�ͺ�
	u_int16_t ele_band_ver;//��ذ�汾��
	u_int16_t line_band_mcu;//�߿ذ�mcu�ͺ�
	u_int16_t line_band_ver;//�߿ذ�汾��
	u_int16_t pad;
}cl_tbb_hd_ver_t;
//��˪��������
typedef struct {
	u_int16_t in_defrost_time;//���Ƚ����˪ʱ������(10~90min)
	u_int16_t in_defrost_tmp;//���Ƚ����˪�¶�( -30~0��)
	u_int16_t out_defrost_tmp;// �����˳���˪�¶�����(2~30��)
	u_int16_t out_defrost_time;//�����˳���˪ʱ������(1~12min)
	u_int16_t env_tmp;//��˪�����¶�����
	u_int16_t dif;//��˪�̹��¶Ⱥͻ����¶Ȳ�ֵ
	u_int16_t defrost_continue_time;//��˪��������ʱ�� 
	u_int16_t pad;
}cl_tbb_defrost_t;

typedef struct {
	u_int16_t dst_tmp_pro;//Ŀ����ȶ�( -15~15��)
	u_int16_t cool_out_water;//�����ˮ���䱣��ֵ(3~20��)
	u_int16_t heat_out_water;//���ȳ�ˮ���ȱ���ֵ(20~90��)
	u_int16_t in_out_tmp;//����ˮ�²����ֵ(5~20��)
	u_int16_t pump_begin_time;//ˮ����ǰ����ʱ��(5~99 s)
	u_int16_t pump_delay_time;//ˮ����ʱ�ر�ʱ��(5~99 s)
	u_int16_t wind_ordor_tmp;//����ת���¶�(5~40��)
	u_int16_t env_tmp;//�����¶ȷ���ֵ(0~15��)
	u_int16_t in_water_tmp;//��ˮ�¶ȷ���ֵ(2~14��)
	u_int16_t pad;
}cl_tbb_protect_t;
/*
work:
	BIT1�����޵��籣�� ��0���ޣ�~1���У�����
	Bit3��ˮ�ù�����ʽ��0����ͨ��1�����⣩��
	Bit8���������ͷ����ڷ�ʽ��0���ֶ���1���Զ�����
	Bit9����ͨ��������0������~1�����򣩣���
	Bit10�����϶�/���϶�ת�� ��0�����϶ȣ� 1�����϶ȣ� Ĭ��0
	Bit15���ָ�����Ĭ��ֵ��0���أ�1��������
*/
typedef struct {
	u_int8_t sys_num;//ϵͳ����(1-4)
	u_int8_t sys_select;//����ѡ��
	u_int16_t work;
	u_int16_t heat_pump_diff;//�ȱ������ز�ֵ(1~20��)
	u_int16_t cool_diff;//����ز��¶�
	u_int16_t hot_diff;//��ů�ز��¶�
	u_int16_t pad;
}cl_tbb_misc_t;

typedef struct {
	u_int16_t ele_cycle;//�������ͷ�����������
	u_int16_t hand_ctrl_step;//�ֶ����ڵ������ͷ����� (10~50)
	u_int16_t cool_ele_valve;//����������ͷ�����(10~50)
	u_int16_t limit_day;//��ʱ����(0~999)
}cl_tbb_eev_t;

typedef struct {
	u_int16_t cool_tmp;//�����ˮ�¶��趨ֵ 
	u_int16_t heat_tmp;//���Ȼ�ˮ�¶��趨ֵ   
	u_int16_t auto_tmp;// �Զ���ˮ�¶��趨ֵ    
	u_int16_t pad;
}cl_tbb_tmp_t;

typedef struct {
	cl_tbb_defrost_t defrost;//��˪����
	cl_tbb_misc_t misc;
	cl_tbb_protect_t protect;//��������
	cl_tbb_eev_t eev;
	cl_tbb_tmp_t auto_tmp;//
	u_int16_t bottom_ele_heat_tmp;//���̵���������¶�(0~20��)
	u_int16_t pad;
}cl_tbb_config_set_t;
//���û����
typedef struct {
	u_int16_t scroll_tmp;//�̹��¶�
	u_int16_t out_air_tmp;//�����¶�
	u_int16_t back_air_tmp;//�����¶�
}cl_tbb_scroll_t;

typedef struct {
	u_int16_t water_box_tmp;//ˮ���¶ȣ� -300~990(0.1��)��
	u_int16_t in_water_tmp;//��ˮ�¶ȣ� -300~990(0.1��)��
	u_int16_t out_water_tmp;//��ˮ�¶ȣ� -300~990(0.1��)��
	u_int16_t env_tmp;//�����¶ȣ� -300~990(0.1��)��
	u_int16_t back_water_tmp;//��ˮ�¶ȣ� -300~990(0.1��)��
	u_int16_t support_mode;//֧��ģʽ
	cl_tbb_scroll_t scroll_tmp[4];
	u_int16_t ele_valve[4];//�������ͷ�����,�������ͷ�����, �������ͷ�����, �������ͷ�����
	u_int16_t pump_info;//�ȱù�������(
	u_int16_t set_on;//����װ����
	u_int16_t set_on1;//��װ1
	u_int16_t set_on2;//��װ2
	u_int16_t slave_status;
	u_int16_t fault1;// 30033
	u_int16_t fault2;// 30034
	u_int16_t fault3;// 30035
	u_int16_t sys_run_days;
	u_int16_t pad16;
	u_int32_t pad32;
}cl_tbb_status_t;

typedef struct{
	/* �豸״̬ */  
	u_int16_t dev_state;   
	/* ��״̬ */  
	u_int16_t  bind_state;  
	/* �ذ�SN��17�ַ� */ 
	u_int8_t tb_sn[24]; 
	u_int8_t reserved[8];
}cl_tbb_bindinfo_t;

/*
upgrade_state:
//��������
#define TBB_ERR_NONE   (0)
	//�����ɹ� 
#define TBB_ERR_OK    (127)
	δ����
#define TBB_ERR_NET   (128)
	��������ʧ��
#define TBB_ERR_DOWN  (129)
	����У��ʧ��
#define TBB_ERR_IMG   (130)
	���ϵĴ����������, ����Ĵ�����ϵ���̴���
	�����߿�ʧ��
#define TBB_ERR_MCU0  (131)
	������ذ�ʧ��
#define TBB_ERR_MCU1  (132)
*/
typedef struct {
    u_int32_t upgradeing;//������
    u_int32_t upgrade_role;//������ɫ��2=>mcu0,3=>mcu1
    u_int32_t upgrade_state;//����״̬
    u_int32_t up_state;//�����ٷֱ�
}cl_stm_upgrade_info_t;

#pragma pack(pop)

typedef struct {
	//ֻ��ʾ����
	u_int8_t on;
	u_int8_t mode;//ģʽ����support_mode ����
	u_int16_t tmp;//�����¶�

	cl_stm_upgrade_info_t upgrade_info;//������Ϣ
	cl_tbb_status_t status;
	cl_tbb_hd_ver_t hd_ver;//��Ƭ��Ӳ����Ϣ����Щ����Ӳ����Ϣ��ʾ
	cl_tbb_bindinfo_t bindinfo;//����Ϣ
	//����������
	cl_tbb_config_set_t config;
}cl_tbb_info_t;

/* Type definitions. */


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
CLIB_API RS cl_tbb_ctrl_onoff(cl_handle_t dev_handle, u_int8_t onoff);
/*
	����:
		ģʽ����
	�������:
		@dev_handle: �豸�ľ��
		@mode:(1-4)
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tbb_ctrl_mode(cl_handle_t dev_handle, u_int8_t mode);

/*
	����:
		�¶�����
	�������:
		@dev_handle: �豸�ľ��
		@tmp:����� 
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tbb_ctrl_tmp(cl_handle_t dev_handle, u_int16_t tmp);


/*
	����:
		��˪����
	�������:
		@dev_handle: �豸�ľ��
		@pconfig:���ò���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tbb_config(cl_handle_t dev_handle, cl_tbb_config_set_t *pconfig);

/*
	����:
		sn��
	�������:
		@dev_handle: �豸�ľ��
		@bar_code:sn�ַ���
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_tbb_bind_dev_bar_code(cl_handle_t dev_handle,u_int8_t* bar_code);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_TBB_H */

