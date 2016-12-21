#ifndef	__SMART_APPLIANCE_PRIV_H__
#define	__SMART_APPLIANCE_PRIV_H__

#include "cl_priv.h"
#include "client_lib.h"
#include "udp_ctrl.h"
#include "uc_client.h"
#include "cl_smart_appliance.h"
#include "cl_lc_furnace.h"
#include "cl_lamp.h"
#include "cl_ia.h"
#include "cl_common_udp_device.h"
#include "cl_evm.h"
#include "cl_lanusers.h"

enum{
    SA_ACT_UNKNOWN,
    SA_ACT_CODE_MATCH_START,
    SA_ACT_CODE_MATCH_STOP,
    SA_ACT_SMART_POWER_OFF,
    SA_ACT_SMART_POWER_ON,
    SA_ACT_SMART_SLEEP,
    SA_ACT_STOP_CUR_POWER,
    SA_ACT_SMART_POWER_OFF_DETAIL,
    SA_ACT_SMART_POWER_ON_DETAIL,
    SA_ACT_SMART_SLEEP_DETAIL,
    SA_ACT_RESET_IR_CODE,
/////////////////////////ѧϰ�������
    SA_ACT_SET_PAN_TYPE,
    SA_ACT_REFRESH_KEY_INFO,
    SA_ACT_SET_KEY_INFO,
    SA_ACT_SET_KEY_INFO_V2,
    SA_ACT_DELETE_KEY,
    SA_ACT_START_LEARN_KEY,
    SA_ACT_STOP_LEARN_KEY,
    SA_ACT_CTRL_KEY,
    SA_ACT_MAX
};

#define	MAGIC1	"tAIl"
#define	MAGIC2	"TaiL"

//rf��Ƭ��magic
#define	MAGIC1_RF	"sAIl"
#define	MAGIC2_RF	"SaiL"

#define SA_AIR_ERR_NO_MATCH  90

typedef struct {
    user_t* user;
    ucp_obj_t *all_mib;
	// smart_air_ctrl_t*
    void* sub_ctrl;
    u_int8_t dev_pass_md5[16];
}smart_appliance_ctrl_t;

typedef struct{
	struct stlc_list_head link;
	int len;
	u_int8_t data[0];
}sound_link_data_t;

typedef struct {
    u_int8_t work_stat_update_ok; /*����״̬�Ƿ�ˢ�¹�*/
    u_int8_t pad[3];
    smart_appliance_ctrl_t* sac;
    cl_air_info_t air_info; /*�豸״̬*/
    cl_ah_info_t ah_info; /*����ů���״̬(û��ʱ��ͷ��뿪)*/ 
    cl_dev_stat_t stat; /*ϵͳ״̬*/
	evm_block_t evm_block;
    cl_pt_stat_t pt_stat; /*����״̬*/
    cl_air_code_match_stat_t match_stat;
    cl_thread_t* match_timer_out; //����ƥ�䳬ʱ
    cl_thread_t* rc_match_timer;
    cl_thread_t* dev_power_timer; //�����豸������ʱ
    cl_thread_t* common_timer; //�豸��Ҫ�Ķ�ʱ��
    u_int32_t common_time;
    u_int32_t power_time_interval;
    u_int32_t last_get_power_time;
    u_int32_t cur_match_step;
	u_int8_t smart_home_enable;//���ܿ��أ���ֻ�ǿյ�����
	u_int8_t is_support_rf_cache;//�Ƿ�֧��rf���ػ���
    union {
		cl_ia_airheater_ycyt_info_t lcyt_info;
		cl_ia_aircleaner_info_t hkac_info;//���ơ��ϰؿ���������
		cl_ia_gx_led_info_t	gxled_info; //��ѶLED�����
		cl_ia_eb_info_t eb_info; /* E�� */
		cl_ia_ch_blanket_info_t ch_blanket_info; 
    }u;
    priv_rc_manage_info rc_pm;
   cl_com_udp_device_data com_udp_dev_info;
    u_int16_t ir_len;
    u_int8_t ir_buf[1024];
    struct stlc_list_head pt_sound; /*�豸�ɼ�����*/
}smart_air_ctrl_t;

#define ia_lcyt_info u.lcyt_info

#pragma pack(push,1)

typedef struct smart_off_s {
	//���ܹع��ܿ���
	u_int8_t on;
	//���Ϳ���
	u_int8_t push_on;
	// 1Сʱ��û�ˣ��Զ��ػ�(�ɸ�ʱ��0.5-3Сʱ����0.5СʱΪ��λ������ΧΪ��������Ĭ��Ϊ��)
	u_int8_t off_time;
	u_int8_t pad[9];
}smart_off_t;

typedef struct smart_on_s {
	//���ܿ���
	u_int8_t on;
	//���Ϳ���
	u_int8_t push_on;
	//�����¶ȸ����������俪��
	u_int8_t sum_on ;
	//�����¶�����,Ĭ�ϣ���
	u_int8_t sum_tmp;
	//�����¶ȸ���xx�������ȿ���
	u_int8_t win_on;
	//����xx�ȣ�Ĭ�ϣ���
	u_int8_t win_tmp;
	//���ҿ����ǵ���ǰ�����ӿ��յ��Ŀ���
	u_int8_t home_on;
	u_int8_t pad[9];
}smart_on_t;

typedef struct smart_sleep_s{
	u_int8_t on;
	u_int8_t pad[3];
}smart_sleep_t;

#define IMAGE_MAGIC 0x0D6AF943
#define IMAGE_BLOCK_SZ 1300
#define IMAGE_TAIL_ALIGN	128

#if 0
typedef struct {
	// ���Լ��������0x0D6AF943
	u_int32_t magic;
	
	// md5У���
	u_int8_t checksum[16];
	
	// TP_DS007 = 5
	u_int8_t ds_type;
	u_int8_t dev_type;
	u_int8_t ext_type;
	u_int8_t pad;
	
	char oem[16];
	// Ԥ������0
	u_int8_t resv[64];
} image_hdr_t;
#else
typedef struct {
	// ���Լ��������0x0D6AF943
	u_int32_t magic;
	
	// md5У���
	u_int8_t checksum[16];
	
	// TP_DS007 = 5
	u_int8_t ds_type;
	u_int8_t dev_type;
	u_int8_t ext_type;
	u_int8_t pad;
	
	char oem[16];
	// Ԥ������0
	//u_int8_t resv[64];
	ucp_version_t soft_ver;
	u_int8_t developer_id[16];	// 32�ֽ��ַ���ת16����
	u_int8_t resv[44];
} image_hdr_t;

#endif
typedef struct{
	u_int16_t total;
	u_int16_t current;
}uc_upgrade_block_t;

enum{
	STM_SPE_UP_OK = 0,
	STM_SPE_UP_NEED_IMG = 1,
	STM_SPE_UP_ING = 2,
	STM_SPE_UP_SN_ERR = 3,
	STM_SPE_UP_PARAM_INVALID = 4,
};

typedef struct {
	u_int8_t sn_num;
	u_int8_t reserve[3];
	
	u_int8_t dev_type;
	u_int8_t ext_type;
	u_int8_t major_ver;
	u_int8_t minor_ver;

	u_int8_t ima_type;
	u_int8_t up_action;
	u_int8_t pad[2];

	u_int64_t sn[0];
}uc_spe_upgrade_pre_t;

typedef struct {
	u_int8_t magic1[4];
	u_int8_t size[2];
	u_int8_t crc[2];
	u_int8_t magic2[4];
} tail_t;

typedef struct {
	u_int8_t magic1[4];
	u_int8_t size[2];
	u_int8_t crc[2];
	u_int8_t magic2[2];
	u_int8_t size_h[2];
} tail2_t;

typedef struct {
	u_int8_t magic1[4];	
	u_int8_t dev_type; //�豸���ͣ�
	u_int8_t ext_type; //�豸��չ���ͣ�
	u_int8_t major_ver; //���汾��,����
	u_int8_t minor_ver; //�ΰ汾��,����
	u_int8_t image_type;  //�̼����ͣ����Ρ�1-app��2-rf
	u_int8_t data_crc[2];
    u_int8_t pad[1];
	u_int8_t magic2[4];
} rf_tail_t;

#pragma pack(pop)

extern bool sa_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern bool misc_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern bool la_proc_notify(cl_notify_pkt_t *pkt, RS *ret);

extern RS sa_init(user_t *user);
extern void sa_free(user_t *user);
extern void sa_build_objs(user_t* user,cl_dev_info_t* ui);
extern cl_dev_stat_t* sa_get_dev_stat_info(user_t* user);
extern cl_pt_stat_t* sa_get_pt_stat_info(user_t* user);
extern RS sa_user_modify_nick_name(user_t* user,char* name,u_int32_t name_len);
extern RS sa_user_modify_passwd(user_t* user,char* pwd_md5);
extern RS sa_user_reboot_device(user_t* user);
extern RS sa_user_telnet_device(user_t* user, cln_user_t *up);

extern bool sa_do_uc_request_pkt(user_t* user,ucph_t* hdr);
extern bool sa_do_uc_reply_pkt(user_t* user,ucph_t* hdr);
extern void sa_force_refresh_data(user_t* user);
extern void sa_do_action_when_estab_out(user_t* user);
extern RS sa_query_objects(ucc_session_t *s,ucp_obj_t* objs,int count);
extern RS sa_set_obj_value(ucc_session_t *s,u_int8_t obj_count,void* content,int content_len);
extern RS sa_ctrl_obj_value(ucc_session_t *s,u_int8_t action,bool need_query,u_int8_t obj_count,void* content,int content_len);
extern RS sa_set_obj_value_only(ucc_session_t *s,u_int8_t obj_count,void* content,int content_len);
extern RS sa_dev_upgrade_cli(cl_notify_pkt_t *cln_pkt);
extern void air_net_timer2_cl_timer(ucp_obj_t* obj,cl_air_timer_info_t* cati);
extern void airplug_timer_local_2_utc(ucp_ac_timer_item_t *timer, int zone);
extern void period_timer_local_2_utc(net_period_timer_t *timer, int zone);
extern void air_timer_dup(cl_air_timer_info_t* dst,cl_air_timer_info_t* src);
extern RS sa_dev_stm_upgrade_cli(cl_notify_pkt_t *cln_pkt);
extern RS sa_dev_stm_upgrade_spe_cli(cl_notify_pkt_t *cln_pkt);
extern RS sa_dev_active(cl_notify_pkt_t *cln_pkt);
extern RS sa_query_obj(ucc_session_t *s,u_int16_t obj,u_int16_t sub_obj, u_int16_t attr);
extern RS sa_dev_upgrade_file(user_t *user, char *file);
extern RS sa_dev_upgrade_file_no_head(user_t *user, char *file);
extern RS sa_dev_upgrade_cli_no_head(cl_notify_pkt_t *cln_pkt);
extern RS sa_dev_upgrade_erase(cl_notify_pkt_t *cln_pkt);
extern RS sa_dev_upgrade_upgrade(cl_notify_pkt_t *cln_pkt);
extern RS sa_check_upgrade_file_no_head(user_t *user, char *file, int len, struct stlc_list_head *pktlst);
extern void comm_timer_build(user_t* user,cl_dev_info_t* ui);

#endif

