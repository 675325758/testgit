#ifndef	__LAN_DEV_PROBE_PRIV_H__
#define	__LAN_DEV_PROBE_PRIV_H__

#include "cl_lan_dev_probe.h"
#include "cl_priv.h"

//������������
#define	CFGPT_RESULT	0
#define	CFGPT_DISCOVERY  1
#define	CFGPT_I_HERE  2
#define	CFGPT_AUTH		3
#define	CFGPT_KEEPLIVE	4
#define	CFGPT_SET_CONFIG	5
#define	CFGPT_GET_CONFIG	6
#define	CFGPT_GET_INFO	7
#define	CFGPT_SET_INFO	8
#define	CFGPT_FRAG		9
#define	CFGPT_UPGRADE_REQ	10
#define	CFGPT_UPGRADE_ACK	11
#define	CFGPT_SWITCH_QUERY	100
#define	CFGPT_SWITCH_STATUS	101
#define	CFGPT_SWITCH_SET		102


#define	SN_BROCAST	0

#define	IJCFG_PORT_DEV	8818 //�豸�����Ķ˿�
#define	IJCFG_PORT_APP	8819 //�ͻ��˼����˿�
#define DEV_SERVER_PORT 1181 //������ֱ�ӵ�¼�˿�

//ע��: �����flags��־��Ҫ���豸��ͳһ������ɾ����˫��ȷ��
/* CFGPF_XXX*/
#define	CFGPF_SLAVE_SUPPORT     0x01
#define	CFGPF_MASTER_SUPPORT    0x02
#define	CFGPF_INDPD_SUPPORT     0x04 //����ģʽ
// ��������
#define	CFGPF_UPGRADING		0x08
//�����Ƿ������ukey�ı��
#define CFGPF_UKEY_PLUGIN    0x10
// ֧��UDP����
#define	CFGPF_UDP_CTRL	0x20
//��ʾ�Ƿ��û�û�޸Ĺ�wan������
#define CFGPF_USER_NO_MODIFY	0x40


#define NO_PASSWD "no password"
#define HAS_PASSWD "password1 %s"

#define MODE_MASTER "master"
#define MODE_INDEPEND "independence"

//�����������������ַ�����ʽ
#define CFGPF_PARAM_FMT \
"!\n\
misc\n\
 mode %s\n\
!\n\
wan\n\
 router_mode\n\
 type_line 2\n\
 type dhcp\n\
!\n\
wifi\n\
 enable\n\
 wifi mode 1\n\
!\n\
ap_client\n\
 ssid1 %s\n\
 %s\n\
!\n\
end\n"

#pragma pack(push,1)

/*Device Probe statue enum*/
enum{
	PS_INIT,
	PS_AUTHING,
	PS_AUTHED,
	PS_CONFIGING,
	PS_CONFIGED,
};

typedef struct cfg_pkt_dhr_s {
    u_int64_t src_sn;   //Դsn,ɨ��ʱ�����������
    u_int64_t dst_sn;   //Ŀ��sn
    u_int16_t cmd;      //�������ͣ�CFGPT_XXX
    u_int16_t param_len;//��������
    u_int16_t req_id;   //����ID
    u_int16_t flags;    //CFGPF_xxx
    u_int8_t data[0];
} cfg_pkt_dhr_t;

typedef struct cfg_pkt_i_here_s {
    u_int8_t type;          //�豸����,TP_XXX
    u_int8_t sub_type;      //�豸������,U_XXX
    u_int16_t config_count; //���ü�����ÿ�ı�һ�����þͼ�1
    u_int8_t auth[16];      //��֤�����
    u_int16_t sys_mode;     //�豸����ģʽ
    u_int8_t ext_type;
    u_int8_t pad;          //����
} cfg_pkt_i_here_t;

typedef struct {
	u_int16_t cfg_cnt;
	u_int8_t enc;
	u_int8_t pad;
	u_int8_t checksum[16];
	u_int8_t config[0];
}cfg_pkt_set_config_t;

typedef struct {
	u_int16_t req_id;
	u_int16_t err_code;
} cfg_pkt_result_t;

#pragma pack(pop)

/*�洢�豸���ֱ���*/
typedef struct {
    struct stlc_list_head link;
    cl_handle_t handle;
    cl_thread_t* time_out;
    u_int8_t type;
    u_int8_t sub_type;
    u_int8_t ext_type;
	u_int8_t developer_id[32];	// ������ID
    u_int8_t real_ext_type;
    u_int16_t dev_run_mode;
    u_int64_t dev_sn; /*���к�*/
    u_int32_t peer_ip;/*ip*/
    u_int32_t recv_time;/*�յ����ĵ�ʱ��*/
    u_int32_t sm_success_time;//һ�����óɹ������
    u_int8_t evm_is_upgrading;
    u_int8_t auth[16];    
    cl_thread_t *t_timer;
    char md5_pwd[16];
    char *ssid;
    char *wifi_pwd;
    u_int8_t probe_status;
    u_int8_t retry;
    u_int16_t req_id;
    u_int16_t flags;
	cl_la_lan_info_t la_info;//����������ɨ������
}dev_probe_info_t;

typedef struct {
	struct stlc_list_head link;
	// Ŀ�Ĺ㲥��ַ�������ֽ���
	u_int32_t bcast_addr;
} brcdst_t;

typedef struct _lan_dev_probe_ctrl_s{
    struct stlc_list_head dev_info_list;
    cl_callback_t callback;
    void *  callback_handle;
    SOCKET udp_socket;
    u_int64_t random_sn;
    u_int64_t ap_dest_sn;
    cl_thread_t *t_probe;
    cl_thread_t *t_read;
    pkt_t*      packet;
    cl_mutex_t mutex;
    u_int8_t probe_times;
    u_int8_t is_probe_enable;//�Ƿ���ɨ��״̬
    u_int8_t notify_user_times;
	// brcdst_t
	struct stlc_list_head bcast_lst;
	// ���һ�λ�ȡ�㲥��ַ��ʱ�䣬ÿ�����������»�ȡ������̫Ƶ��
	u_int32_t last_get_bcast;
}lan_dev_probe_ctrl_t;

//
extern u_int32_t get_dev_lan_ipaddr(u_int64_t sn);
extern bool lan_dev_is_need_udp_login(u_int64_t sn);
extern void delete_lan_dev_by_sn(u_int64_t sn);
extern bool lan_dev_probe_init(cl_priv_t* cp);
extern void lan_dev_probe_exit(cl_priv_t* cp);
extern bool lan_dev_probe_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern void lan_phone_send_config_ok_to_app(u_int64_t wan_sn);
extern void lan_dev_proc_wifi_switch(int new_net_type);

#endif

