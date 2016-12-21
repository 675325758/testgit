#ifndef	__CL_NOTIFY_DEF_H__
#define	__CL_NOTIFY_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif 

enum{
    TNOTIFY_EMERGENCY = 0, /* ����֪ͨ����Ϣ��ʽ�̶�ΪHTTP */
    TNOTIFY_NORMAL = 1,       /* ��֪ͨͨ����Ϣ��ʽ�̶�ΪHTTP */
    TNOTIFY_AD = 2,                /* ��棬��Ϣ��ʽ�̶�ΪHTTP*/
    TNOTIFY_FANS_SYSTEM = 5, /* FansSRV system notify msg */
    TNOTIFY_FANS_BUSSINESS = 6, /* FansSRV bussiness notify msg */
    TNOTIFY_EMERGENCY_V2 = 7,		/* ����֪ͨ������ָ����Ϣ��ʽ */
    TNOTIFY_NORMAL_V2 = 8,		 /* ��֪ͨͨ������ָ����Ϣ��ʽ */
    TNOTIFY_AD_V2 = 9,			/* ��棬����ָ����Ϣ��ʽ */
};

enum{
	FMT_HTTP = 0, /* ��Ϣ��ʽΪhttp */
	FMT_URL = 1, /* ��Ϣ��ʽΪurl */
	FMT_STRING = 2,  /* ��Ϣ��ʽΪ�ַ��� */
};
    
enum  { /*�����豸���Ͷ���*/
    SAFETY_SMOKE_DETECTOR = 0x1,	/* �����Ӧ̽���� */
    SAFETY_GAS_LEAK_DETECTOR = 0x2, /* �����Ӧ̽���� */
    SAFETY_INFRARED_DETECTOR = 0x3,	/* ��Ǻ���̽���� */
    SAFETY_DOOR_SENSOR = 0x4,		/* �Ŵ�̽���� */
    SAFETY_WINDOW_SENSOR = 0x5,		/* Ļ������̽���� */
    SAFETY_INTELLIGENT_LOCK = 0x6,	/* �������� */
    SAFETY_EMERGENCY_BUTTON = 0x7,	/* ������ť */
    SAFETY_003H_MOTION = 100,		/* ����ͷ�ƶ���� */
};

/* ��Ϣȱʡ��Ч�ڣ��� */
#define EXPIRE_DEFAULT 14
/* ��Ϣ���Ч�ڣ��� */
#define EXPIRE_MAX	90
/*����֪ͨ��Ϣ���ĵ���󳤶�*/
#define MAX_NOTIFY_MSG 7800
/*����֪ͨ��ϢժҪ����󳤶�*/
#define MAX_NOTIFY_CONTENT 128

#pragma warning(disable: 4200)
    
#pragma pack(push,1)
    
typedef struct{		/* �����������ݽṹ */
    u_int32_t alarm_time;           /* ��������ʱ�����UTC*/
    u_int16_t alarm_duration;     /* ��������ʱ�䣬��λ��*/
    u_int16_t  alarm_type;		/* ���������ͣ� �� SAFETY_SMOKE_DETECTOR*/
    u_int16_t  alarm_id;			/* ��������ţ�һ����ͥ�����ж������̽�⣬�ñ������ */
    u_int8_t len_name;              /* �����豸���Ƴ���*/
    u_int8_t arlam_name[64];               /* �����豸���ƣ�UTF8�ַ������磺��������̽����*/
    u_int8_t len_msg;		    /* ������Ϣ���ݳ���*/
    u_int8_t alarm_msg[256];                 /* ������Ϣ���ݣ�UTF8�ַ������磺����ð����*/
}alarm_msg_t;

typedef struct{	/* ����ͷ���ݽṹ */
    u_int64_t dev_sn;		/* �豸���к� */
    u_int64_t first_report_id;  /* ������Ϣ��ʼid */
    u_int16_t count;			/* ������Ϣ���� */
    alarm_msg_t *msg;		/* ������Ϣ�����׵�ַ*/
}alarm_msg_list_t;

typedef struct{ 			/* ����֪ͨ��Ϣ����*/
    u_int64_t dev_sn;            /* �豸���к� */
    u_int32_t msg_time;	/* ��Ϣʱ�����UTC*/
    u_int64_t report_id;	/* ��Ϣid */
    u_int16_t msg_type;    /* ��Ϣ���ͣ�TNOTIFY_NORMAL, TNOTIFY_XXX*/
    u_int8_t msg_format; /* ��Ϣ��ʽ��FMT_HTTP��FMT_URL */
    u_int8_t expire;	/* ��Ч�ڣ���λ�� */
    u_int8_t content_len;  /* ��ϢժҪ���� */
    u_int8_t reserved;  /* ����*/
    u_int32_t msg_len;	/* ��Ϣ����*/
    char *msg;			/* ��Ϣ����, msg_typeΪTNOTIFY_URLʱmsgΪURL*/
    char *content;		/*��ϢժҪ��UTF8�ַ���������ΪNULL*/
}notify_msg_t;

typedef struct{
	int count;
	notify_msg_t **list;
}notify_msg_list_t;

typedef struct{                     /* ֪ͨ��Ϣȷ�����ݽṹ*/
    u_int64_t dev_sn;             /* �豸���к� */
    u_int64_t first_report_id; /* ��ʼ��Ϣid*/
    u_int8_t report_count;     /* ��Ϣ����*/
    u_int8_t result;
    u_int8_t reserved[2];
}notify_msg_result_t;

typedef struct{			/* ֪ͨ����������С�������յ��豸��Ϣid���ݽṹ */
    u_int64_t dev_sn;		/* �豸���к�*/
    u_int64_t expect_report_id;	/* �����յ�ָ���豸����Ϣid */
}notify_expect_t;

//һ̨�豸������4��С��
#define MAX_CMT 4

typedef struct{
	u_int64_t cmt_sn;	/*С�����к�*/
	u_int64_t max_report_id; /*С�������Ϣid*/
}cmt_notify_info_t;

typedef struct{
	u_int64_t cmt_sn; /*С�����к�*/
	u_int64_t report_id_begin; /*��ѯ��Ϣ��ʼid��0��ʾ������*/
	u_int64_t report_id_end; /*��ѯ��Ϣ����id��0��ʾ������*/
	u_int8_t is_descending; /*0��ʾ�����ѯ��1��ʾ�����ѯ*/
	u_int8_t query_cnt; /*��ѯ����*/
	u_int8_t reserved[2]; /*����*/
}cmt_notify_query_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 


#endif



