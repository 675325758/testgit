#ifndef	__CL_RC_H__
#define	__CL_RC_H__

#ifdef __cplusplus
extern "C" {
#endif 

    //机顶盒信息
#define MAX_RC_NAME_LEN 16
    
//电视和机顶盒遥控器相关
#define RC_TYPE_TV 0x1
#define RC_TYPE_STB 0x2

/* 电视按键定义 */
/* 开关机   - 0 */
#define TV_KEY_ONOFF (0)
/* 丽音     - 1  */
/* 伴音     - 2  */
/* 制式     - 3  */
/* 睡眠     - 4  */
/* 1        - 5  */
/* 2        - 6  */
/* 3        - 7  */
/* 4        - 8  */
/* 5        - 9  */
/* 6        - 10 */
/* 7        - 11 */
/* 8        - 12 */
/* 9        - 13 */
/* --/-     - 14 */
/* 0        - 15 */
/* 节目交替 - 16 */
/* 交换     - 17 */
/* 画中画   - 18 */
/* 正常     - 19 */
/* 选台     - 20 */
/* 图像     - 21 */
/* CH-      - 22 */
/* CH+      - 23 */
/* 声音     - 24 */
/* 向上     - 25 */
#define TV_KEY_UP (25)
/* 向下     - 26 */
#define TV_KEY_DOWN (26)
/* 向左     - 27 */
#define TV_KEY_LEFT (27)
/* 向右     - 28 */
#define TV_KEY_RIGHT (28)
/* 菜单     - 29 */
/* 屏显     - 30 */
/* TV/AV    - 31 */
#define TV_KEY_TVAV (31)
/* 完成     - 32 */
/* 音量加   - 33 */
#define TV_KEY_VOLUP (33)
/* 音量减   - 34 */
#define TV_KEY_VOLDOWN (34)
/* 频道加   - 35 */
#define TV_KEY_CHUP   (35)
/* 频道减   - 36 */
#define TV_KEY_CHDOWN (36)
/* 静音     - 37 */
#define TV_KEY_MUTE   (37)


/* 机顶盒按键定义 */
/* 1               - 0 */
/* 2               - 1 */
/* 3               - 2 */
/* 4               - 3 */
/* 5               - 4 */
/* 6               - 5 */
/* 7               - 6 */
/* 8               - 7 */
/* 9               - 8 */
/* 0               - 9 */
/* 列表            - 10 */
/* 上一节目        - 11 */
/* 待机            - 12 */
#define STV_KEY_ONOFF (12)
/* 频道+           - 13 */
#define STV_KEY_CHUP  (13)
/* 频道-           - 14 */
#define STV_KEY_CHDOWN (14)
/* 音量+           - 15 */
#define STV_KEY_VOLUP  (15)
/* 音量-           - 16 */
#define STV_KEY_VOLDOWN (16)
/* 上              - 17 */
#define STV_KEY_UP     (17)
/* 下              - 18 */
#define STV_KEY_DOWN   (18)
/* 左              - 19 */
#define STV_KEY_LEFT   (19)
/* 右              - 20 */
#define STV_KEY_RIGHT  (20)
/* 确认            - 21 */
#define STV_KEY_OK     (21)
/* 退出            - 22 */
/* 菜单            - 23 */
#define STV_KEY_MENU  (23)
/* 红              - 24 */
/* 绿              - 25 */
/* 黄              - 26 */
/* 蓝              - 27 */
/* 返回            - 28 */
#define STV_KEY_BACK  (28)
/* 上页            - 29 */
#define STV_KEY_PAGEUP (29)
/* 下页            - 30 */
#define STV_KEY_PAGEDOWN (30)
/* 声道            - 31 */
/* 信息            - 32 */
/* 静音            - 33 */
/* 喜爱            - 34 */
/* 导视            - 35 */
/* 电视            - 36 */
/* 广播            - 37 */
/* 资讯            - 38 */
/* 股票            - 39 */
/* 点播            - 40 */
/* 邮件            - 41 */
/* 游戏            - 42 */
/* 列表            - 43 */
/* 上一节目        - 44 */
/* 设定            - 45 */
/* 主页            - 46 */
#define STV_KEY_HOME   (46)
/* 记录●（Record） - 47 */
/* 停止■           - 48 */
/* A               - 49 */
/* B               - 50 */
/* C               - 51 */
/* D               - 52 */
/* E               - 53 */
/* F               - 54 */
/* 快退            - 55 */
/* 快进            - 56 */
/* 播放/暂停       - 57 */
/* 保留            - 58 */
/* 保留            - 59 */
/* 保留            - 60 */
/* 保留            - 61 */
/* 保留            - 62 */
/* 保留            - 63 */
    
    
typedef struct {
    u_int8_t type; //TV or STB
    u_int8_t rc_id; //遥控器id
    u_int8_t action; // 当前状态 AIR_CODE_XX
    u_int8_t is_cloud_matching; // 是否正在云匹配中，全匹配此字段无效
    u_int8_t cur_step; // 当前进行到第几步
    u_int8_t max_step; //总共多少步
    u_int8_t error; // 匹配出现错误
    u_int8_t flag; //匹配同码错误标等,注:只在匹配过程中有效。
    ////////////////////////////////////////////////
    u_int8_t recommon_key_id; // 推荐的下次按键id
}cl_rc_match_stat_t;

#define MAX_CODE_LEN 256

typedef struct {
    u_int8_t isLearn; //是否正在学习中
    u_int8_t rc_id; // 遥控器id
    u_int8_t type; //TV or STB
    u_int8_t key_id; // 当前学习的 按键id
    u_int8_t code_len; // 编码长度
    u_int8_t code[MAX_CODE_LEN]; //学习到的编码
}cl_rc_key_learn_stat_t;

typedef struct {
    u_int8_t key_id; //按键id
    u_int8_t has_code; //是否有编码
}cl_rc_fixed_key_info;

typedef struct {
    u_int8_t key_id; //按键ID
    u_int8_t has_code; //是否已有编码
    u_int8_t pad[2];
    u_int8_t name[MAX_RC_NAME_LEN]; //按键名称
}cl_rc_user_key_info;

typedef struct {
    u_int8_t d_id; //遥控板 id，也是rc_id
    u_int8_t is_matched; //是否匹配
    u_int8_t fixed_key_num; //固定按键数量
    u_int8_t user_def_key_num; //用户定义按键数量
    u_int16_t matched_ir_id; //已匹配红外库的ID
    u_int8_t name[MAX_RC_NAME_LEN];
    cl_rc_fixed_key_info * fk;
    cl_rc_user_key_info * uk;
}cl_rc_info;

//电视、机顶盒遥控器组合，仅支持一个组合的用这个结构
typedef struct {
    u_int8_t name[MAX_RC_NAME_LEN];
    cl_rc_info tv_info;// 电视遥控板信息
    cl_rc_info stb_info; //机顶盒遥控板信息
}cl_pair_rc_info;
    
typedef struct {
    u_int8_t is_query_key_info;
    u_int8_t pad[3];
    cl_pair_rc_info pair_rc;
    cl_rc_match_stat_t match_stat;
    cl_rc_key_learn_stat_t learn_info;
}priv_rc_manage_info;


#ifdef __cplusplus
}
#endif 

/*
功能:
    修改遥控器组合名字
输入参数:
    @key
输出参数:
    无
返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_change_name(cl_handle_t dev_handle, u_int8_t* name);

/*
 功能:
    开始匹配编码，函数成功后，根据事件处理
 输入参数:
    @rc_id 电视或机顶盒id
    @time_out 单位：秒
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_start_match(cl_handle_t dev_handle,u_int8_t rc_id,u_int16_t timeout);

/*
 功能:
    多键匹配的第二次匹配
 输入参数:
    @rc_id 电视或机顶盒id
    @time_out 单位：秒
 输出参数:
    无
 返回:
    RS_OK: 成功
 其他: 失败
 */
CLIB_API RS cl_rc_start_next_key_match(cl_handle_t dev_handle,u_int8_t rc_id,u_int16_t timeout,u_int8_t key_id);

/*
 功能:
    停止匹配编码
 输入参数:
    @rc_id 电视或机顶盒id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_stop_match(cl_handle_t dev_handle,u_int8_t rc_id);

/*
 功能:
    获取编码匹配状态
 输入参数:
    @key
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */

CLIB_API RS cl_rc_get_match_stat(cl_handle_t dev_handle,cl_rc_match_stat_t* stat);

/*
 功能:
    获取按键学习编码状态
 输入参数:
    @key
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_get_learn_stat(cl_handle_t dev_handle,cl_rc_key_learn_stat_t* stat);

/*
 功能:
    发送控制指令
 输入参数:
    @rc_id 电视或机顶盒id
    @key_id 按键id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_ctrl_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 功能:
    快速发送电视和机顶盒开关控制指令
    方便用户一键开启、关闭电视和机顶盒
 输入参数:
    @key
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_quick_ctrl_onoff(cl_handle_t dev_handle,u_int8_t tv_rc_id,u_int8_t stb_rc_id);

/*
 功能:
    开始学习编码
 输入参数:
    @rc_id 电视或机顶盒id
    @key_id 按键ID
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_start_learn(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 功能:
    停止学习编码
 输入参数:
    @rc_id 电视或机顶盒id
    @key_id 按键ID
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_stop_learn(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 功能:
    删除按键
 输入参数:
    @rc_id 电视或机顶盒id
    @key_id 按键id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_delete_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id);

/*
 功能:
    添加/修改按键
 输入参数:
    @rc_id 电视或机顶盒id
    @key_id 按键id
 输出参数:
    无
 返回:
    RS_OK: 成功
    其他: 失败
 */
CLIB_API RS cl_rc_modify_key(cl_handle_t dev_handle,u_int8_t rc_id,u_int8_t key_id,char* name);


#endif

