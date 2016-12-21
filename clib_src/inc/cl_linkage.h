/**************************************************************************
**  Copyright (c) 2011 GalaxyWind, Ltd.
**
**  Project: cl_linkage.h
**  File:    cl_linkage.h
**  Author:  liubenlong
**  Date:    12/03/2015
**
**  Purpose:
**    联动外部头文件.
**************************************************************************/


#ifndef CL_LINKAGE_H
#define CL_LINKAGE_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Include files. */
#include "cl_user.h"
/* Macro constant definitions. */
#define LA_USER_MAX		10
#define LA_HOME_MAX		20
#define LA_MEM_MAX		30

/* Type definitions. */
//错误号
enum {
	LA_ERR_OTHER_HOME = 0,//设备添加失败原因，在其他圈子里
	LA_PHONE_ACOUNT_EXIST = 1,//手机账号已经存在，不能创建了
	LA_PHONE_ALL_SESSION_OFFLINE = 2,//所有session都是离线的，不能创建
	LA_PHOME_LOGIN_FAILED_LOGINED = 3,//该手机账号已经登陆过
	LA_PHONE_NOT_EXIST = 4,//该手机账号不存在
	LA_PHONE_CREATE_FAILED_EXIST = 5,//服务器上已有该账号
	LA_PHONE_REPLACE_FAILED = 6,//服务器替换强用户失败
	LA_PHONE_DEFAULT_HOME_SERVER_OFFLINE = 7,//默认圈子服务器离线中
	LA_PHONE_LOGIN_PASSWD_ERR = 8,//登陆密码错误
	LA_HOME_DEL_FAILED_DEF = 9,//删除圈子失败，因为是默认圈子，不可删除
	LA_HOME_REG_SESSION_IS_OFFLINE = 10,//注册失败，服务器离线
	LA_NET_TIMEOUT = 11,	// 因为网络错误导致超时(请检查路由器外网)

	LA_APP_USER_REQ_SHARE_NO_USERID = 12, //pad分享二维码解析错误,没有userid
	LA_APP_USER_REQ_SHARE_USER_ID_ALREAD = 13,//pad分享二维码错误，不能自己分享自己注册
	LA_APP_USER_REQ_SHARE_PARSE_ERROR = 14,//pad分享二维码解析错误
	LA_APP_USER_REQ_SHARE_HOME_NOT_EXIST = 15,//pad分享二维码注册时，找不到要注册的家庭
	LA_APP_USER_REQ_SHRAE_HOME_EXIST = 16,//pad分享二维码注册时，服务器回应该用户已经有该家庭了

	//不支持错误
	LA_NOT_SUPPORT = 17,
	
	LA_APP_VER_IS_TOO_SMALL = 99,//APP版本过低，连接不上服务器
};

enum {
	LA_BEGIN = 2100,
	//联动用户圈子成员信息一起获取变动事件,这个时候就该调用cl_la_info_get函数获取总的圈子列表信息
	LA_USER_CHANGED = LA_BEGIN + 1,
	//联动圈子变化 
	LA_HOME_CHANGED = LA_BEGIN + 2,
	//圈子删除失败
	LA_HOME_DEL_FAILED = LA_BEGIN + 3,
	//圈子添加设备错误
	LA_HOME_ADDDEV_FAILED = LA_BEGIN + 4,
	//圈子设备列表有变动，可以用push上来的homehandle，查询圈子设备,如果调用cl_la_home_info_get函数
	LA_HOME_ADDDEV_CHANGED = LA_BEGIN + 5,
	//圈子设备删除失败
	LA_HOME_REMOVEDEV_FAILED = LA_BEGIN + 6,
	//分享注册成功
	LA_HOME_SHARE_REGISTER_SUCCESSED = LA_BEGIN + 7,//事件err_no带的就是home_id
	//分享描述修改失败
	LA_HOME_SHARE_DESC_CHANGE_FAILED = LA_BEGIN + 8,
	//分享删除失败
	LA_HOME_SHARE_DESC_DELETE_FAILED = LA_BEGIN + 9,
	//添加圈子失败
	LA_HOME_CREATE_FAILED = LA_BEGIN + 10,
	//该圈子已经分享注册过了
	LA_HOME_SHARE_REGISTER_ALREADY = LA_BEGIN + 11,
	//规则执行失败
	LA_LINK_CONF_EXEC_FAILED = LA_BEGIN + 12,
	//请求分享码成功
	LA_SHARE_REQ_SUCCESSED = LA_BEGIN + 13,
	//圈子间设备移动失败
	LA_HOME_CONF_DEV_MOVE_FAILED = LA_BEGIN + 14,
	//规则执行成功
	LA_LINK_CONF_EXEC_SUCCESSED = LA_BEGIN + 15,
	//圈子名称修改失败
	LA_HOME_CONF_NAME_MODIFY_FAILED = LA_BEGIN + 16,
	//圈子创建成功
	LA_HOME_CREATE_SUCCESSED = LA_BEGIN + 17,//事件err_no带的就是home_id
	//该分享得来的圈子被删除了
	LA_HOME_SHARE_DELETED_BY_CREATOR = LA_BEGIN + 18,
	//分享注册失败
	LA_HOME_SHARE_REGISTOR_FAILED = LA_BEGIN + 19,
	//手机账号创建失败
	LA_PHONE_CREATE_FAILED = LA_BEGIN + 20,//可以判断errno里的具体错误
	//手机账号创建成功
	LA_PHONE_CREATE_SUCCESS = LA_BEGIN + 21,
	//手机账号登陆失败
	LA_PHONE_LOGIN_FAILED = LA_BEGIN + 22,
	//手机账号登陆成功
	LA_PHONE_LOGIN_SUCCESS = LA_BEGIN + 23,
	//手机账号登出失败
	LA_PHONE_LOGOUT_FAILED = LA_BEGIN + 24,
	//手机账号登出成功
	LA_PHONE_LOGOUT_SUCCESS = LA_BEGIN + 25,
	//手机账号删除失败
	LA_PHONE_DEL_FAILED = LA_BEGIN + 26,
	//手机账号删除成功
	LA_PHONE_DEL_SUCCESS = LA_BEGIN + 27,
	//手机账号切换失败
	LA_PHONE_SWICH_FAILED = LA_BEGIN + 28,
	//手机账号切换成功
	LA_PHONE_SWICH_SUCCESS = LA_BEGIN + 29,
	//手机账号密码修改成功
	LA_PHONE_PASSWD_MODIFY_SUCCESS = LA_BEGIN + 30,
	//手机账号密码修改失败
	LA_PHONE_PASSWD_MODIFY_FAILED = LA_BEGIN + 31,
	//被另一个用户登陆踢掉了
	LA_KICKED_BY_OTHER_USER = LA_BEGIN + 32,
	//分享注册失败，服务器离线
	LA_HOME_SHARE_REGISTOR_FAILED_OFFLINE = LA_BEGIN + 33,
	//请求分享码失败
	LA_SHARE_REQ_FAILED = LA_BEGIN + 34,

	//pad类型请求分享码失败
	LA_USER_SHARE_REQ_FAILED = LA_BEGIN + 35,
	//pad类型请求分享码成功
	LA_USER_SHARE_REQ_SUCCESSED = LA_BEGIN + 36,
	//pad类型分享添加家庭失败
	LA_USER_SHARE_ADD_HOME_FILED = LA_BEGIN + 37,
	//pad类型分享添加家庭成功
	LA_USER_SHARE_ADD_HOME_SUCCESSED = LA_BEGIN + 38,
	//pad类型分享，pad分享二维码放收到成功添加圈子信息
	LA_USER_SHARE_ADD_ONE_HOME_SUCCESSED = LA_BEGIN + 39,

	//标签相关
	LA_LABEL_ADD_SUCCESS = LA_BEGIN + 40,
	LA_LABEL_ADD_FAILED = LA_BEGIN + 41,
	LA_LABEL_DEL_SUCCESS = LA_BEGIN + 42,
	LA_LABEL_DEL_FAILED = LA_BEGIN + 43,
	LA_LABEL_BIND_SUCCESS = LA_BEGIN + 44,
	LA_LABEL_BIND_FAILED = LA_BEGIN + 45,
	

	//服务器推送消息提示事件
	LA_SERVER_PUSH_MSG = LA_BEGIN + 50,
	//连接服务器失败事件
	LA_SERVER_LINK_FAILED = LA_BEGIN + 51,
	//能力文件变化事件
	LA_CAP_FILE_CHANGED = LA_BEGIN + 52,
	//rsa请求app处理事件
	LA_RSA_REQ = LA_BEGIN + 53,
	//手机账号密码错误
	LA_PHONE_PASSWD_ERR = LA_BEGIN + 54,

	//s3专用
	//查询失败
	LA_S3_QUERY_RULE_FAILED = LA_BEGIN + 55,
	//查询成功
	LA_S3_QUERY_RULE_SUCCESSD = LA_BEGIN + 56,
	//查询最新修改时间失败
	LA_S3_QUERY_RULE_LAST_MODIFY_TIME_FAILED = LA_BEGIN + 57,
	//查询最新修改时间成功
	LA_S3_QUERY_RULE_LAST_MODIFY_TIMER_SUCCESSD = LA_BEGIN + 58,

	//快捷按键添加
	LA_SHORTCUT_ADD_SUCCESSD = LA_BEGIN + 59,
	LA_SHORTUT_ADD_FAILED = LA_BEGIN + 60,
	LA_SHORTCUT_DEL_SUCCESSD = LA_BEGIN + 61,
	LA_SHORTCUT_DEL_FAILED = LA_BEGIN + 62,

	// 字典相关
	LA_DICT_MODIFY = LA_BEGIN + 63,	//字典有更新
	LA_DICT_SET_SUCCESS = LA_BEGIN + 64,	//字典添加成功
	LA_DICT_SET_FAILED = LA_BEGIN + 65,	//字典添加失败
	LA_DICT_DEL_SUCCESS = LA_BEGIN + 66,	//字典删除成功

	LA_SHORTCUT_MOD_SUCCESSD = LA_BEGIN + 67,
	LA_SHORTCUT_MOD_FAILED = LA_BEGIN + 68,

	//日志相关
	LA_LOG_RULE_EXCUTE = LA_BEGIN + 69,//联动规则执行日志查询成功。收到该事件后，从errno里取出home_id用来获取日志
	
	LA_LOG_HOME_DEV_CHANGE = LA_BEGIN + 70,//设备移出移入家庭日志查询成功。收到该事件后，从errno里取出home_id用来获取日志

	// 字典相关
	LA_DICT_DEL_FAILED = LA_BEGIN + 71,	//字典删除失败
	LA_DICT_QUERY_FAILED = LA_BEGIN + 72,	// 字典查询失败

	//规则执行日志事件，服务器主动推送了变化 日志最大index下来，上传事件时err_no = max_index
	LA_LOG_RULE_EXCUTE_MAX_INDEX = LA_BEGIN + 73,

	LA_WIDGET_KEY_DONAME_CHANGED = LA_BEGIN + 74,//密钥或者域名变化了，app需要去获取同步保存

	LA_END = 2399,
};

enum {
	CAP_FILE_TYPE_NONE = 0,
	CAP_FILE_TYPE_COM,//普通能力文件
	CAP_FILE_TYPE_CUSTOM,//自定义能力文件
};

typedef struct {
	u_int16_t type;//CAP_FILE_TYPE_COM ....
	//设备能力文件上次修改时间
	u_int32_t last_cap_time;
	u_int16_t cap_num;//能力文件个数
	u_int8_t **cap_array;//能力文件url数组
}cap_file_type_t;

typedef struct {
	u_int8_t file_type_num;//下面数组个数
	cap_file_type_t *pfile_type;
}cap_file_t;

typedef struct {
	u_int8_t user_num;//以后服务器可能有多个，现在只有一个，先放这儿，
	u_int32_t user_id[10];//根据user_num来取user_id

	u_int8_t phone_num;//手机账号个数,大于0才有效
	u_int8_t cur_phone;//当前手机账号
	char **phone_name;//手机账号名

	u_int8_t language;// 1-中文，2-英文
/*
	//设备能力文件上次修改时间
	u_int32_t last_cap_time;
	u_int16_t cap_num;//能力文件个数
	u_int8_t **cap_array;//能力文件url数组
*/
	cap_file_t cap_file;

	//是否支持标签操作，有可能服务器没升级等原因
	bool support_label;
	
	u_int8_t home_num;
	cl_user_t **home;
}cl_la_info_t;

//服务器消息上报结构
typedef struct {
	u_int16_t type;//消息类型
	char *cn_msg;
	char *en_msg;
}cl_la_notify_msg_t;

#define LA_USE_RSA

//rsa加密数据结构
typedef void (*cl_rsa_callback)(int priv_len, u_int8_t *priv, int in_len, u_int8_t *in, u_int8_t *out);

typedef struct {
	cl_rsa_callback dec;
	cl_rsa_callback enc;
}cln_la_rsa_t;

//s3专用
typedef struct {
	u_int32_t home_id;
	u_int16_t rule_num;//添加的规则
	u_int32_t last_modify_time;//该家庭上次规则最新修改时间
	cl_rule_desc_t *rule_array;//添加的规则字符串
}cl_home_rule_info_t;


//日志相关
//规则执行变化日志
typedef struct {
	u_int32_t index;

	u_int8_t is_test;//是否是试一试执行规则。
	u_int8_t is_timer;//是否有timer。
	
	u_int16_t monitor_count;
	u_int64_t *pmonitor_sn;
	
	u_int16_t action_count;
	u_int64_t *paction_sn;

	u_int32_t ruld_id;
	u_int32_t time_stamp;
}home_log_rule_info_t;

typedef struct {
	u_int32_t home_id;
	u_int32_t max_index;
	
	u_int16_t log_count;
	home_log_rule_info_t *plog;
}cl_home_log_rule_change_t;

//家庭设备变化日志
#define HOME_MEM_CHANGE_ACTION_OUT 	0
#define HOME_MEM_CHANGE_ACTION_IN 	1

typedef struct {
	u_int64_t sn;
	u_int8_t action;
	u_int8_t resaon;
	u_int8_t username[APP_USER_UUID_NAME_LEN];
	u_int32_t user_id;
	u_int32_t time_stamp;
}home_log_member_info_t;

typedef struct {
	u_int32_t home_id;
	
	u_int16_t log_count;
	home_log_member_info_t *plog;
}cl_home_log_member_change_t;



//密钥域名获取
#define WIDGET_KEY_MAX_LEN (32)
typedef struct {
	u_int8_t key[WIDGET_KEY_MAX_LEN];
	char doname[64];
}cl_widget_info_t;

/* External function declarations. */


/* Macro API definitions. */


/* Global variable declarations. */
/*
功能: 
    获取sdk的所有联动信息    
 返回:
    cl_la_info_t
    其他: 失败
 */
CLIB_API cl_la_info_t *cl_la_info_get();
CLIB_API RS cl_la_info_free(cl_la_info_t * info);
/*
	功能描述:
	家庭创建，这里没想好，创建时该挂在哪个服务器上，先随便吧
	参数:
	@name, 家庭名
*/
CLIB_API RS cl_la_home_create(cl_handle_t *homehandle, char *name);

/*
	功能描述:
	家庭删除
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
*/
CLIB_API RS cl_la_home_del(cl_handle_t homehandle, u_int32_t home_id);


/*
	功能描述:
	家庭成员添加
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@num,添加的设备个数
	@*sn,添加的设备sn数组
*/
CLIB_API RS cl_la_home_adddev(cl_handle_t homehandle, u_int32_t home_id, u_int8_t num, u_int64_t *sn);

/*
	功能描述:
	家庭成员删除
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@num,添加的设备个数
	@*sn,添加的设备sn数组
	@flag: bit 0
			为0表示需要恢复出厂 ；为1表示不需要恢复出厂
*/
CLIB_API RS cl_la_home_removedev(cl_handle_t homehandle, u_int32_t home_id, u_int8_t num, u_int64_t *sn, u_int8_t flag);
/*
	功能描述:
	联动配置模板查询
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
*/
CLIB_API RS cl_la_config_template_query(cl_handle_t homehandle, u_int32_t home_id);
/*
	功能描述:
	联动配置规则查询
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@rule_id, 规则id
*/
//  查询一个规则
CLIB_API RS cl_la_config_rule_query(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id);
/*
	功能描述:
	联动配置规则添加
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@rule_id, 规则id
	@rule, 规则数据
*/
// 添加一个规则
CLIB_API RS cl_la_config_rule_add(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id, char *rule, u_int8_t enable);
/*
	功能描述:
	联动配置规则删除
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@rule_id, 规则id
*/
// 删除一个规则
CLIB_API RS cl_la_config_rule_del(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id);
/*
	功能描述:
	联动事件触发
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@rule_id, 规则id
*/
// 触发事件
CLIB_API RS cl_la_config_event(cl_handle_t homehandle, u_int8_t event, u_int32_t home_id, u_int32_t rule_id);
/*
	功能描述:
	家庭分享请求
	参数:
	@homehandle,家庭handle
	@home_id, 家庭id
*/
/*
	家庭成员分享、成员管理
**/
// App向服务器申请家庭的分享码，服务器确定其有效期，默认10分钟。
CLIB_API RS cl_la_home_share_create(cl_handle_t homehandle, u_int32_t home_id);
// 注册
/*
	功能描述:
	家庭分享注册
	参数:
	@homehandle,家庭handle
	@home_id, 家庭id
	@share,二维码生成的共享码字符串
	@desc,描述
*/
CLIB_API RS cl_la_home_share_register(cl_handle_t homehandle, u_int32_t home_id, u_int8_t *share, u_int8_t *desc);
// 查询分享
/*
	功能描述:
	家庭分享查询
	参数:
	@homehandle,家庭handle
	@home_id, 家庭id
*/
CLIB_API RS cl_la_home_share_query(cl_handle_t homehandle, u_int32_t home_id);
/*
	功能描述:
	家庭分享编辑修改
	参数:
	@homehandle,家庭handle
	@home_id, 家庭id
	@desc, 描述
*/
// 分享编辑
CLIB_API RS cl_la_home_share_edit(cl_handle_t homehandle, u_int32_t home_id, u_int32_t user_id, u_int8_t role_id, u_int8_t *desc);
/*
	功能描述:
	家庭分享编辑修改
	参数:
	@homehandle,用户handle
	@home_id, 家庭id
	@user_id, 用户id
*/
// 删除分享
CLIB_API RS cl_la_home_share_del(cl_handle_t homehandle, u_int32_t home_id, u_int32_t user_id);

/*
	功能描述:
	app设置，要求sdk自动登陆的sn设备
	参数:
	@homehandle,用户handle
	@sn,若干个sn的数组
	@num,num个sn
*/
// 删除分享
CLIB_API RS cl_la_sdk_login_set(cl_handle_t homehandle, u_int64_t *sn, u_int16_t num);

/*
	功能描述:
	app清楚配置
	参数:
	@homehandle,用户handle
	@sn,若干个sn的数组
	@num,num个sn
*/
// 删除分享
CLIB_API void cl_la_conf_clean();
/*
	功能描述:
	联动配置规则执行
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@rule_id, 规则id
*/
// 删除一个规则
CLIB_API RS cl_la_config_rule_exec(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id);

// 查询分享
/*
	功能描述:
	设置当前家庭id
	参数:
	@homehandle,家庭handle
	@home_id, 家庭id
*/
CLIB_API RS cl_la_cur_homeid_set(u_int32_t home_id);

/*
	功能描述:
	获取当前圈子id
	参数:
	无
	返回:
	消息数据
*/
CLIB_API u_int32_t cl_la_cur_homeid_get();

/*
	功能描述:
	家庭间设备移动接口
	参数:
	@homehandle,目的家庭handle
	@src_home_id，移出设备的家庭id
	@dst_home_id，移进设备的家庭id
	@num,移动的设备个数
	@*sn,移动的设备sn数组
*/
CLIB_API RS cl_la_home_movedev(cl_handle_t homehandle, u_int32_t src_home_id, u_int32_t dst_home_id, u_int8_t num, u_int64_t *sn);

/*
	功能描述:
	联动配置修改成是否使能
	参数:
	@homehandle,家庭handle
	@home_id，家庭id
	@rule_id, 规则id
	@enable, 是否使能，1使能
*/
// 删除一个规则
CLIB_API RS cl_la_config_rule_modify(cl_handle_t homehandle, u_int32_t home_id, u_int32_t rule_id, u_int8_t enable);

/*
	功能描述:
	修改家庭名称
	参数:
	@home_id,修改的家庭id
	@name, 家庭名
*/
CLIB_API RS cl_la_home_name_modify(cl_handle_t homehandle, u_int32_t home_id, char *name);

/**************************************************手机账号操作接口**********************************************************************************/
/*
	功能描述:
	创建手机用户
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API RS cl_la_phone_create(char *name, char *passwd);

/*
	功能描述:
	登陆手机用户
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API RS cl_la_phone_login(const char *name, const char *passwd);


/*
	功能描述:
	当前被踢去睡眠的session重新登陆
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API RS cl_la_phone_relogin();

/*
	功能描述:
	登出手机用户
	参数:
	@name,用户名
*/
CLIB_API RS cl_la_phone_logout(char *name);

/*
	功能描述:
	删除手机用户，会修改服务器
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API RS cl_la_phone_del(char *name);
/*
	功能描述:
	切换手机用户
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API RS cl_la_phone_swich(char *name);
/*
	功能描述:
	手机用户密码修改
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API RS cl_la_phone_passwd_modify(char *name, char *passwd);

/*
	功能描述:
	手机用户密码比较
	参数:
	@name,用户名
	@passwd, 密码
*/
CLIB_API bool cl_la_phone_is_same_passwd(char *name, char *passwd);

//服务器消息通知
/*
	功能描述:
	收到 LA_SERVER_PUSH_MSG 事件后调用获取消息数据
	参数:
	无
	返回:
	消息数据
*/
CLIB_API cl_la_notify_msg_t *cl_la_get_notify_msg();

/*
	功能描述:
	是否消息
	参数:
	无
	返回:
	消息数据
*/
CLIB_API void cl_la_free_notify_msg(cl_la_notify_msg_t *msg);

/*
	功能描述:
	设置语言类型
	参数:
	lang@:1-中文，2-英文
*/
CLIB_API RS cl_la_cur_lang_set(u_int8_t lang);

/*
	功能描述:
	获取语言类型
	参数:
	lang@:1-中文，2-英文
*/
CLIB_API RS cl_la_cur_lang_get(u_int8_t *lang);

/*
	功能描述:
	获取设备能力文件查询接口
	参数:
	lang@:1-中文，2-英文
*/
CLIB_API RS cl_la_cap_file_query();

/*
	功能描述:
	获取设备自定义能力文件查询接口
	参数:
	lang@:1-中文，2-英文
*/
CLIB_API RS cl_la_cap_custom_file_query();

/*
	功能描述:
	rsa初始化函数，退出函数注册
	参数:
	@rsa_enc:rsa加密函数
	@rsa_dec:rsa解密函数
*/
CLIB_API RS cl_la_rsa_callback_set(cl_rsa_callback rsa_enc, cl_rsa_callback rsa_dec);


// App向服务器申请家庭的分享码，服务器确定其有效期，默认10分钟。为了pad添加，反向添加
CLIB_API RS cl_la_app_share_create();
// 注册
/*
	功能描述:
	家庭分享注册
	参数:
	@home_id, 家庭id
	@share,二维码生成的共享码字符串
	@desc,描述
*/
CLIB_API RS cl_la_app_share_register(u_int32_t home_id, char *share, char *desc);

/*
*当app收到 分享成功事件LA_USER_SHARE_REQ_SUCCESSED 后，调用这个接口获取share字符串
*/
CLIB_API char *cl_la_app_get_share();


//标签操作
/*
	功能描述:
	添加修改标签,将多个设备加入到对应标签中，或创建的标签中。
	参数:
	@home_id, 家庭id
	@label_id,标签id，0表示创建，其他表示修改
	@name:标签名称，最大63字节
	@sn_num:标签包含的sn个数,可以为0, 0-n个。
	@*sn:指向添加的sn数组
*/
CLIB_API RS cl_la_label_add_modify(u_int32_t home_id, u_int16_t label_id, char *name, u_int16_t sn_num, u_int64_t *sn);

/*
	功能描述:
	删除标签
	参数:
	@home_id, 家庭id
	@label_id,标签id
*/
CLIB_API RS cl_la_label_del(u_int32_t home_id, u_int16_t label_id);

/*
	功能描述:
	将设备添加个多个标签中
	参数:
	@home_id, 家庭id
	@sn，要添加到其他标签中的sn
	@label_id_num:要添加的标签数,不允许为0
	@*lable_id:要添加的标签数组
*/
CLIB_API RS cl_la_bind_devs_to_label(u_int32_t home_id, u_int64_t sn, u_int16_t label_id_num, u_int16_t *label_id);

// 字典

/*
*	功能描述:
*	主动查询家庭下的某个字典(从网络取)，查询后有LA_DICT_MODIFY事件
*	参数:
*	@home_id,欲获取字典的homeid
	@key: 字典
	@key_len: 字典长度
*/
CLIB_API RS cl_la_dict_query(u_int32_t home_id, u_int8_t *key, u_int16_t key_len);

/*
*	功能描述:
*	设置家庭下的某个字典，有LA_DICT_SET_SUCCESS和LA_DICT_SET_FAILED事件
*	参数:
*	@home_id,欲获取字典的homeid
	@key: 字典
	@key_len: 字典长度
	@value: 字典数值
	@value_len: 字典数据长度
*/
CLIB_API RS cl_la_dict_set(u_int32_t home_id, u_int8_t *key, u_int8_t *value, u_int16_t key_len, u_int16_t value_len);

/*
*	功能描述:
*	设置家庭下的某个字典
*	参数:
*	@home_id,欲获取字典的homeid，有LA_DICT_DEL_SUCCESS事件
	@key: 字典
	@key_len: 字典长度
	@value: 字典数值
	@value_len: 字典数据长度
*/
CLIB_API RS cl_la_dict_del(u_int32_t home_id, u_int8_t *key, u_int16_t key_len);

//快捷按键定义

/*
	功能描述:
	快捷按键，规则关联创建
	参数:
	@home_id, 家庭id
	@index, 1-6,6个自定义快捷键
	@name,自定义名称
	@rule_id,规则id
*/
CLIB_API RS cl_la_shortcut_rule_bind(u_int32_t home_id, u_int8_t index, u_int8_t enabel, char *name, u_int32_t rule_id, char *rule);

/*
	功能描述:
	快捷按键，修改。
	参数:
	@index, 1-6,6个自定义快捷键
	@name,自定义名称
*/
CLIB_API RS cl_la_shortcut_modify(u_int32_t home_id, u_int8_t index, char *name);

/*
	功能描述:
	快捷开关，规则关联
	参数:
	@home_id, 家庭id
	@index, 1-6,6个自定义快捷键
*/
CLIB_API RS cl_la_shortcut_del(u_int32_t home_id, u_int8_t index);


//s3网关专用接口
/*
*	功能描述:
*	通过homeid查询规则列表
*	参数:
*	@home_id,欲查询homeid
*/
CLIB_API RS cl_la_home_rule_query(u_int32_t home_id);

/*
*	功能描述:
*	通过homeid获取sdk内的规则列表
*	参数:
*	@home_id,欲获取规则的homeid
*/
CLIB_API cl_home_rule_info_t *cl_la_home_rule_get(u_int32_t home_id);

/*
*	功能描述:
*	释放
*	参数:
*/
CLIB_API RS cl_la_home_rule_free(cl_home_rule_info_t *pinfo);

/*
*	功能描述:
*	查询家庭下规则的最新修改时间
*	参数:
*	@home_id,欲查询的homeid
*/
CLIB_API RS cl_la_home_rule_query_last_modify_time(u_int32_t home_id);

/*
*	功能描述:
*	pad需要的临时性执行规则
*	参数:
*	@home_id,
*	@rule, 需要执行的规则
*/
CLIB_API RS cl_la_home_rule_excute(u_int32_t home_id, char *rule);


/*
*	功能描述:
*	查询家庭规则执行日志
*	参数:
*	@home_id,
*	@index:日志index >=0
*	@count:查询日志条数
*/
CLIB_API RS cl_la_home_rules_logs_query(u_int32_t home_id, u_int32_t index, u_int32_t count);

/*
*	功能描述:
*	获取家庭规则执行日志
*	参数:
*	@home_id,
*/
CLIB_API cl_home_log_rule_change_t *cl_la_home_rules_logs_get(u_int32_t home_id);

/*
*	功能描述:
*	获取家庭规则执行日志
*	参数:
*	@home_id,
*/
CLIB_API RS cl_la_home_rules_logs_free(cl_home_log_rule_change_t *pinfo);

/*
*	功能描述:
*	查询家庭规设备变动情况
*	参数:
*	@home_id,
*/
CLIB_API RS cl_la_home_members_logs_query(u_int32_t home_id);

/*
*	功能描述:
*	获取家庭规设备变动情况
*	参数:
*	@home_id,
*/
CLIB_API cl_home_log_member_change_t *cl_la_home_members_logs_get(u_int32_t home_id);

/*
*	功能描述:
*	获取家庭规设备变动情况
*	参数:
*	@home_id,
*/
CLIB_API RS cl_la_home_members_logs_free(cl_home_log_member_change_t *pinfo);

/*
*	功能描述:
*	widget获取密钥查询，在https查询报密钥超时的情况下调用，可能只有安卓需要调用，因为安桌会在后台常驻，程序重新连接
*	服务器后会自动获取一个新的，所以ios可能不需要查询
*	参数:
*	@home_id,
*/
CLIB_API RS cl_la_widget_key_query();

/*
*	功能描述:
*	在收到密钥变动事件后，调用该函数获取密钥域名信息
*	参数:
*/
CLIB_API cl_widget_info_t *cl_la_widget_info_get();

/*
*	功能描述:
*	释放数据
*	参数:
*/
CLIB_API RS cl_la_widget_info_free(cl_widget_info_t *pinfo);




#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CL_LINKAGE_H */

