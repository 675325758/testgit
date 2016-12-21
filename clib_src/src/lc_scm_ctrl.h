#ifndef	__LC_SCM_CTRL_H__
#define	__LC_SCM_CTRL_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "cl_smart_appliance.h"

//Bit0��0�ػ���1����
#define FHF_LC_UART_CMD_FLAG_STATE (0x1 << 0)
//Bit1��0�͵���1�ߵ�
#define FHF_LC_UART_CMD_FLAG_GEAR (0x1 << 1)
//Bit2��0��ҡͷ��1ҡͷ
#define FHF_LC_UART_CMD_FLAG_SHAKE_HEAD (0x1 << 2)
//Bit3��0����ECO��1��EC0ģʽ
#define FHF_LC_UART_CMD_FLAG_ECO (0x1 << 3)
//Bit4��0�����岻����,�ڹ��±���״̬��1�������ڹ���
#define FHF_LC_UART_CMD_FLAG_HEATWORK (0x1 << 4)
//���ڷ����������ֵ
#define FHF_UART_SEND_LIST_MAX 25
//���ڷ�������������Դ���
#define FHF_UART_SEND_TRY_MAX 3
//���ڷ�����ʱʱ��,��λ��
#define FHF_UART_SEND_TIMEOUT 1
//���ڷ�����ʱʱ��,��λ����
#define FHF_UART_SEND_TIMEOUT_MS	500
//״̬ˢ�¶�ʱ��ʱ��,��λ��
#define FHF_STATE_REFRESH_TIME 3

enum {
	FHF_LC_OPERATION_SUB = 0,
	FHF_LC_OPERATION_ADD = 1
};

enum {
	//�������ͣ��¶��趨ֵ
	FHF_LC_UART_CMD_TYPE_SET_TEMP = 0x1,
	//�������ͣ���ʱ�趨ֵ
	FHF_LC_UART_CMD_TYPE_SET_TIMER = 0x2
};

enum {
	//��ȡ�����������ͻ�����
	FHF_LC_UART_CMD_TYPE_GET_ALL = 0x0,
	//��ȡ�����������ͻ����¶�ֵ
	FHF_LC_UART_CMD_TYPE_GET_ENV_TEMP = 0x1,
	//��ȡ�����������ͷ�����1�¶�ֵ
	FHF_LC_UART_CMD_TYPE_GET_HEATING_UNIT1_TEMP = 0x2,
	//��ȡ�����������ͷ�����2�¶�ֵ
	FHF_LC_UART_CMD_TYPE_GET_HEATING_UNIT2_TEMP = 0x3,
	//��ȡ������������ʣ�ඨʱʱ��
	FHF_LC_UART_CMD_TYPE_GET_REMAIN_MINUTE = 0x4,
	//��ȡ�����������͵�ǰ���ĵ���ֵ
	FHF_LC_UART_CMD_TYPE_GET_POWER = 0x5,
	//��ȡ�����������������¶�ֵ
	FHF_LC_UART_CMD_TYPE_GET_SET_TEMP = 0x6
};

enum {
	//���ò�������
	FHF_LC_UART_CMD_SET_PARAM = 0x1,
	//������������
	FHF_LC_UART_CMD_SET_PRESS_KEY = 0x2,
	//��ȡ��������
	FHF_LC_UART_CMD_GET_PARAM = 0x3,
	//��ȡ����״̬����
	FHF_LC_UART_CMD_GET_STATE = 0x4,
	//������������
	FHF_LC_UART_CMD_ADD_OR_SUB_PARAM = 0x5,
	//��ȡ��Ʒ���к�����
	FHF_LC_UART_CMD_GET_SN = 0x6,
	//�������ݰ���������
	FHF_LC_UART_CMD_ERROR_REPLY = 0x80,
	//���ò��������Ӧ������
	FHF_LC_UART_CMD_SET_PARAM_REPLY = 0x81,
	//����������Ӧ������
	FHF_LC_UART_CMD_SET_PRESS_KEY_REPLY = 0x82,
	//��ȡ������Ӧ������
	FHF_LC_UART_CMD_GET_PARAM_REPLY = 0x83,
	//��ȡ����״̬��Ӧ������
	FHF_LC_UART_CMD_GET_STATE_REPLY = 0x84,
	//����������Ӧ������
	FHF_LC_UART_CMD_ADD_OR_SUB_PARAM_REPLY = 0x85,
	//��ȡ��Ʒ���кŵ�Ӧ������
	FHF_LC_UART_CMD_GET_SN_REPLY = 0x86,
	//�����
	FHF_LC_UART_CMD_ACTIVE = 0x87
};

enum {
	//��������ػ�
	FHF_LC_UART_SET_COMMAND_SHUTDOWN = 0x0,
	//���������
	FHF_LC_UART_SET_COMMAND_STARTING_UP = 0x1,
	//��������͵�
	FHF_LC_UART_SET_COMMAND_GEAR_LOW = 0x2,
	//��������ߵ�
	FHF_LC_UART_SET_COMMAND_GEAR_HIGH = 0x3,
	//��������ҡͷ
	FHF_LC_UART_SET_COMMAND_SHAKE_HEAD = 0x4,
	//��������ECO
	FHF_LC_UART_SET_COMMAND_ECO = 0x5,
	//������������
	FHF_LC_UART_SET_COMMAND_SETTING = 0x6,
	//������������
	FHF_LC_UART_SET_COMMAND_ADD = 0x7,
	//�����������
	FHF_LC_UART_SET_COMMAND_SUB = 0x8,
	//�������λ�л�
	FHF_LC_UART_SET_COMMAND_SWITCH_GEAR = 0x9,
	//����������л�
	FHF_LC_UART_SET_COMMAND_SWITCH_ON_OFF = 0xA
};

enum {
	//����״̬�����Է�������
	FHF_LC_UART_STATE_READY = 0x0,
	//����״̬���ȴ��ذ���ʱ���ڷ���
	FHF_LC_UART_STATE_WORKING = 0x1
};

/* Type definitions. */
#pragma pack(push,1)

typedef struct lc_ucd_pkt_request_head_s{
	u_int8_t len;
	u_int8_t flag;
	u_int8_t cmd;
	u_int8_t data_sub_type;
	u_int8_t data[0];
}lc_ucd_pkt_request_head_t;

typedef struct lc_ucd_pkt_response_head_s{
	u_int8_t len;
	u_int8_t flag;
	u_int8_t cmd;
	u_int8_t data_sub_type;
	u_int8_t err_no;
	u_int8_t data[0];
}lc_ucd_pkt_response_head_t;

//83 00
typedef struct {
	u_int16_t pad;
	//�����¶�
	u_int8_t env_temp;
	//������1�¶�ֵ
	u_int8_t heating_unit1_temp;
	//������2�¶�ֵ
	u_int8_t heating_unit2_temp;
	//ʣ�ඨʱʱ��
	u_int8_t remain_hour;
	//��ǰ���ĵ���ֵ
	u_int32_t power_low;
	u_int32_t power_high;
	//�����¶�
	u_int8_t set_temp;
}fhf_uart_get_param_data_t;

typedef struct {
	u_int32_t power_low;
	u_int32_t power_high;
}fhf_uart_get_power_t;
//84 00
typedef struct {
	/*��ǰ�Ĺ���״̬:
	*Bit0��0�ػ�,1����
	*Bit1��0�͵�,1�ߵ�
	*Bit2��0��ҡͷ,1ҡͷ
	*Bit3��0����ECO,1��EC0ģʽ*/
	u_int8_t state;
	/*����״̬,0����������,��0�������,���Ͼ��嶨������
	*Bit0���㵹����
	*Bit1�������¶ȹ��߱���
	*Bit2���������¶ȹ��߱���
	*Bit3������ִ�д���*/
	u_int8_t fault_state;
}fhf_uart_get_state_data_t;

#pragma pack(pop)

extern bool lc_scm_proc_notify(user_t* user,cl_notify_pkt_t *pkt, RS *ret);
extern bool lc_scm_update_data(smart_air_ctrl_t* air_ctrl,u_int8_t action,ucp_obj_t* obj);
extern int lc_scm_proc_ctrl_result(smart_air_ctrl_t* ctrl,ucp_obj_t* obj,u_int16_t error);
extern int lc_get_ext_type_by_ident(u_int8_t* ident,int ident_len);

#endif

