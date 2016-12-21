#ifndef EVM_CJTHERMOSTAT_H
#define EVM_CJTHERMOSTAT_H

#include "smart_appliance_priv.h"
#include "udp_scm_direct_ctrl_priv.h"
#include "cl_notify.h"
#include "cl_priv.h"

#include "cl_cjthermostat.h"

#pragma pack(push, 1)

// ����ͷ��
typedef struct {
	u_int8_t syn;
	u_int8_t cmd;
	u_int16_t addr;
} cjthermostat_uart_hdr_t;


typedef struct {
	u_int16_t outtime_hour;	// ���ʱ����Сʱ
	u_int8_t outtime_min;	// ���ʱ��������
	u_int8_t ver;
	u_int8_t is_heat;	// �Ƿ��������� 0 δ���� 1 ����
	u_int8_t week;	// �¿�������
	u_int8_t time;	// ��ǰʱ��0-240,��λΪ6����
	u_int8_t stat;	// 0���޲���1 һ����� 2 ��̨�趨 3 ��������  ��0�����޲�������ָ�¿����ϵ���ӵ������ָ���û���ٶ԰������й������� Ҳ�������Ϊ���޲����� ���UI������û���õ���
	u_int8_t set_temp;	// �趨�¶ȣ�����
	u_int8_t inside_temp;		// ��̽ͷ�¶ȣ��������֡�����
	u_int8_t inside_temp1;	// ��̧ͷ�¶ȣ�С������
	u_int8_t outside_temp;	// ��̧ͷ�¶ȣ�������������
	u_int8_t outside_temp1;	// ��̧ͷ�¶ȣ�С������
	u_int8_t mode;	// ģʽ1 ���� 2 �ֶ�3 ��Ϣ4 �Զ� 5 ��˪ 6 Ԥ��
	u_int8_t power;	// 0 �ػ�1 ������Ļ��ʾ2������Ļ�ر�
	u_int8_t key_lock;	// 0 ������ 1 ������
	u_int8_t fault;	// ���� 0 ���� 1 ����̽ͷ��2����̽ͷ��3����̽ͷ��4��������
	int8_t temp_adjust;	// ��У:-9��9
	u_int8_t set_temp_upper_limit;	// �¶���������
	u_int8_t set_temp_lower_limit;	// �¶���������
	u_int8_t temp_allowance;	// �¿��ݲ�
	u_int8_t defrost_temp;	// ��˪�¶�
	u_int8_t overtemp;	// �����¶�
	u_int8_t overtemp_allowance;	// �����ݲ�
	u_int8_t flag;	// bit 0 �ͺ�:0: S��1:D��  bit 1 Ԥ�ñ�־: 0 סլģʽ1�칫ģʽ
	u_int8_t timer_week;	// �Զ���������bit0-bit7��ʾ����7��1 ~6
	u_int8_t manual_temp;	// �ֶ�ģʽ�¶��趨
} cjthermostat_uart_stat_t;

#pragma pack(pop)


bool cjthermostat_do_update_scm_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool cjthermostat_bulid_priv_dev_info(smart_air_ctrl_t* ac, cl_com_udp_device_data* di);
bool cjthermostat_do_update_device_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
bool cjthermostat_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);

#endif



