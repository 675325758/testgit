#ifndef	__EQUIPMENT_PRIV_H__
#define	__EQUIPMENT_PRIV_H__

#include "cl_equipment.h"

#ifdef __cplusplus
extern "C" {
#endif 

#pragma pack(push, 1)

// 这个本来是在net_proto.h中，可是vc不允许在0数组中保护0数组，哎
typedef struct{
	u_int32_t err;
	u_int8_t action; // 0:查询 1: 添加 2: 修改 3:删除
	u_int8_t version;
	u_int16_t count;	// remote_attri_t 结构体个数
	//remote_atrri_t ctrldev[0];	// remote_attri_t 结构体
}net_remote_t;

#pragma pack(pop)

enum  {
    AC_KEYCODE_LEARN = 0x0, /*开始学习*/
    AC_KEYCODE_ADD,  /*添加编码*/
    AC_KEYCODE_DELETE, /*删除编码*/
    AC_KEYCODE_MODIFY, /*修改编码*/
    AC_KEYCODE_TRY,    /*尝试*/
    AC_KEYCODE_STUDYOK,  /*学习成功,设备推送报文*/
    AC_KEYCODE_STUDYSTOP, /*停止学习*/
    AC_KEYCODE_GEN_CODE,       /*产生对码*/
    AC_KEYCODE_PROBE,     /*探测是否可以微调*/
    AC_KEYCODE_ADJUST,    /*微调*/
    AC_KEYCODE_PLUS_WIDTH_AJUST,  /*脉宽调整*/
};
    
enum  { /*REMOTE_TYPE_DIY自定义电器的子类型定义*/
    RDEVSUBTYPE_CUSTOM = 0,  /*自定义RF电器*/
    RDEVSUBTYPE_RF_TO_INFR_TV = 0x1, /*红外转发控制电视*/
    RDEVSUBTYPE_RF_TO_INFR_TVBOX = 0x2,/*红外转发控制机顶盒*/
    RDEVSUBTYPE_RF_TO_INFR_AIRCONDITION = 0x3,/*红外转发控制空调*/
    RDEVSUBTYPE_RF_TO_INFR_CUSTOM =0x4/*红转自定义*/
};

#define ALARM_PHONE_LENGTH (16)

#define KEYID_ALARM_MSG_MASK (0x40000000)
#define KEYID_ALARM_KEY ((0x40000000)|0x1)
    
#define STATEID_OUTLET_GROUPNUM (16)/*有多少路*/
#define STATEID_OUTLET_FUCTION  (15)/*单开单关还是反转*/
    
#define STATEID_CURTAIN_TYPE    (16)/*百叶窗还是左右推拉窗*/
#define STATEID_CURTAIN_STATE   (15)/*当前状态*/

#define STATEID_DB_RF_CURTAIN_TIME (15) /*双向窗帘全开或全关时间*/
#define STATEID_DB_RF_CURTAIN_POSTION (12) /*双向窗帘当前位置*/
#define STSTEID_DB_RF_CTRL (11) /*双向灯控制开还是关*/
    
#define STATEID_LIGHT_GROUPNUM  (16)/*有多少路*/
#define STATEID_LIGHT_FUCTION    (15)/*单开单关还是反转*/
#define STATEID_LIGHT_STATE     (12)/*灯当前状态*/
#define STATEID_RF_REPEATER    (17)/*RF中续*/
#define STATEID_DB_DIMMING      (14)/*调光灯*/

#define STATEID_FUNC_AJUST_LIGHT     BIT(0)  /*置1表示调光*/
#define STATEID_FUNC_BTN_ONOFF_CTRL BIT(1)  /*置1单开单关*/

#define IS_KEY_STATE_ON(keyid, state)  ((BIT((keyid))>>1) & (state))

enum{//action of double rf equipment
	AC_RF2_BIND = 1,
	AC_RF2_UNBIND = 2,
	AC_RF2_REPEATER_ON = 3,
	AC_RF2_REPEATER_OFF = 4,
};

typedef struct {
    struct stlc_list_head link;
    cl_handle_t handle;
}handle_backup_t;

typedef struct _equipment_ctrl_s {
	// 回指指针
	slave_t *slave;
	//云匹配完查询
	cl_thread_t *t_cm_query;
	// 查询定时器
	cl_thread_t *t_query;

	int prev_reply_len;
	// 缓存上一次查询的结果，用来判断本次查询结果是否改变
	void *prev_reply;

	// 电器链表, equipment_t
	struct stlc_list_head eq_list;
    
    struct stlc_list_head eq_new_eq_hand_list;

} equipment_ctrl_t;

typedef struct {
	struct stlc_list_head link;
	cl_handle_t handle;
    equipment_ctrl_t* ec;
    
    /*按键列表*/
    struct stlc_list_head keylist;
    cl_thread_t *t_key_query;
	 /* 查询电话信息和关联的声光报警器 */
    cl_thread_t *t_alarm_query;
    u_int32_t nbp_len;
    net_alarm_bind_phone_t* nbp;

	// 该报警器关联的声光报警器
	u_int32_t soundlight_len;
	// 保存的数据里面，list的字节序未转化，其他的已经转化好了
    net_remote_config_soundlight *soundlight;

    int prev_reply_len;
	// 缓存上一次按键查询的结果，用来判断本次查询结果是否改变
	void *prev_reply;
	u_int8_t match_id_num;	// 云空调匹配到多少个编码，最多4
	u_int16_t match_id[4];	//最多4个匹配到的ID号
	remote_atrri_v2_t attr;
} equipment_t;
    
typedef struct {
    struct stlc_list_head link;
    /*回指电器指针*/
    equipment_t *eq;
    remote_key_attri_t key_attr;
}remote_key_t;

/*复用按键和报警学习*/
typedef struct remote_key_learn_s{
    u_int32_t key_id; /*学习的key句柄*/
    cl_handle_t eq_handle; /*电器句柄*/
    u_int8_t learn_mode; /*学习模式，是否是对码*/
    u_int8_t learn_stat; /*当前学习状态*/
    u_int8_t learn_remain_time; /*倒计时剩余时间*/
    u_int8_t is_support_plw; /*是否支持脉宽调整*/
    u_int8_t is_support_ajust; /*是否支持信号微调*/
    int last_err;
    u_int16_t ajust_pw_max;
    u_int16_t ajust_current_value;
    
    cl_callback_t callback;  /*回调函数*/
	void *callback_handle; /*回调参数*/
    
    u_int16_t learn_code_len; /*学习到的编码长度*/
    u_int16_t learn_code_type; /*学习到的编码类型*/
    u_int8_t* learn_code; /*学习到的编码*/
    
    u_int16_t host_gen_code_len; /*主机产生的编码长度*/
    u_int16_t host_gen_code_type; /*主机产生的编码类型*/
    u_int8_t* host_gen_code;  /*主机产生的编码*/
    
    u_int16_t ajust_code_len;  /*微调产生的编码长度*/
    u_int16_t ajust_code_type; /*微调产生的编码类型*/
    u_int8_t* ajust_code;      /*微调产生的编码*/
    int ajust_range;     /*微调范围*/
    
    u_int16_t base_code_len;  /*微调时用的基准编码长度*/
    u_int16_t base_code_type; /*微调时用的基准编码类型*/
    u_int8_t* base_code;      /*微调时用的基准编码*/
    
    cl_thread_t *t_learn;
}remote_key_learn_t;
    
typedef struct _alarm_phone_assist_s{
    user_t* user; //回指指针
    cl_thread_t* t_phone_query; /*查询phone列表*/
    u_int32_t nacp_len;
    net_alarm_config_phone_t* nacp; /*phone列表*/
}alarm_phone_assist_t;

bool isAlarmDevice(equipment_t* eq);
// 返回; BOOL: 处理了该报文. false: 需要其他模块继续处理处理
extern bool eq_proc_tcp(user_t *user, pkt_t *pkt, net_header_t *hdr);
extern bool eq_proc_notify(cl_notify_pkt_t *pkt, RS *ret);
extern RS eq_alloc(slave_t *slave);
extern void eq_free(slave_t *slave);
extern void eq_free_objs(cl_equipment_t *eq);
extern void eq_build_objs(cl_dev_info_t *ui, slave_t *slave, int *idx_eq);
extern RS key_learn_alloc(user_t* user);
extern void key_learn_free(user_t* user);
extern RS alarm_phone_alloc(user_t* user);
extern void alarm_phone_free(user_t* user);
extern equipment_t *eq_lookup(equipment_ctrl_t *ec, u_int16_t id);
extern equipment_t *eq_lookup_by_handle(equipment_ctrl_t *ec, cl_handle_t eq_handle);
extern remote_key_t *rk_lookup(equipment_t *eq, u_int32_t id);
extern void eq_force_update_all_info(equipment_ctrl_t *ec);



#ifdef __cplusplus
}
#endif 

#endif

