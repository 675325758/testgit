#ifndef	__RFGW_RFTT_H__
#define	__RFGW_RFTT_H__

#ifndef	BIT
#define	BIT(n)	(1<<(n))
#endif

/*ͨ��ָ��*/
enum{
    RF_CT_UNKNOWN = 0x0,
    RF_CT_WORK_STAT = 0x1, //�豸״̬
    RF_CT_DBG_INFO = 0x2,//������Ϣ����������ʱ���
    RF_CT_CTRL_ON_OFF = 0x3,//���ƿ������رգ�֧�ּ򵥿��Ƶ��豸��֧�֣������
    RF_CT_CTRL_GUARD = 0x4,//��������,�������豸֧��
};


#pragma pack(push,1)
/////////////////2.4G RF transparent translation protocol////////////////////
/*
����2.4G RF ͨ���غ����32�ֽڣ���Ҫ��Լʹ��
RF ���ĳ�����֪��͸������Ͳ���������
͸���������һ��ֱ�Ӹ�����
*/

//���ƹ���״̬���󣬲���1�ֽڣ���3λ�ֱ��ʾRGB��
#define RFTT_WORK_REQ 1
//���ƹ���״̬��Ӧ������1�ֽڣ���3λ�ֱ��ʾRGB��
#define RFTT_WORK_ACK 2
//RF�豸������Ϣ������rfdev_dbg_t
#define RFTT_DBG_INFO 3
//��ѯ����״̬���޲������豸��ӦRFTT_WORK_ACK
#define RFTT_WORK_QUERY (4)

//RGB��ɫ��״̬λ
#define RGB_WORK_R BIT(0)
#define RGB_WORK_G BIT(1)
#define RGB_WORK_B BIT(2)
//�Ŵ�״̬����λ��1��ʾ���ţ�0��ʾ����
#define MC_WORK_OPEN BIT(3)

//RF�豸������Ϣ
typedef struct{
	u_int32_t rfrx;//�յ���RF֡����
	u_int32_t uptime;//������ʱ����
	u_int32_t linktime;//���ӵ����غ�ʱ����
	u_int16_t linkretry;//���ӵ����ش���
}rfdev_dbg_t;


#pragma pack(pop)

#endif

