#ifndef	__CL_VIDEO_H__
#define	__CL_VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "client_lib.h"


// video event
enum {
	VE_BEGIN = 200,
	// ��Ƶ���ӽ��������յ�ͼ����Ҫ��֪�Ƿ�֧������
	VE_ESTABLISH = VE_BEGIN + 1,
	// ��ȡ��һ��ͼƬ
	VE_GET_PICTURE = VE_BEGIN + 2,
	// ��ȡ��һ������Ƭ��
	VE_GET_SOUND = VE_BEGIN + 3,
	// ��ȡ����Ȩ�ɹ�
	VE_TALK_REQ_SUCCESS = VE_BEGIN + 4,

	VE_REC_TIMER_SET_OK = VE_BEGIN + 5,
	VE_REC_TIMER_SET_FAILED = VE_BEGIN + 6,
	VE_REC_TIMER_MODIFY = VE_BEGIN + 7,
    //������Ƶ���Ͷȳɹ�
    VE_SET_V4L2_OK = VE_BEGIN + 8,
    //������̨ת�ٳɹ�
    VE_SET_ROLL_SPEED_OK = VE_BEGIN + 9,
	
	// ��Ƶ����
	VE_ERROR = VE_BEGIN + 50,
	// ��ȡ����Ȩʧ��
	VE_TALK_REQ_FAILED,
    //������Ƶ���Ͷ�ʧ��
    VE_SET_V4L2_FAILED,
    //������̨ת��ʧ��
    VE_SET_ROLL_SPEED_FAILED,
    //��ȡ¼���б�ɹ�
    VE_VTAP_GET_LIST_OK,
    //��ȡ¼���б�ʧ��
    VE_VTAP_GET_LIST_FAILED,
    VE_VTAP_SET_POS_FAILED,
    //��Ƶ�鿴���ִ���
    VE_VTAP_CHECK_ERR,
	VE_END = VE_BEGIN + 99
};


/*
	USB����ͷ�����Ϣ
*/
typedef struct {
	cl_obj_t obj;
} cl_usb_video_t;



/*
	���ܣ�
		��ʼ��Ƶ����
	����IN��
		slave: Ҫ�ۿ���Ƶ�Ĵ��豸��ָ��
		callback���ص�����
	����OUT��
		��
	���أ�
		VIDEO_XXX
*/
CLIB_API RS cl_video_start(cl_handle_t video_handle, cl_callback_t callback, void *handle);

CLIB_API RS cl_video_stop(cl_handle_t handle);

/*
	���ܣ�
		��ȡ����Ƶʱ�����һ֡ͼƬ��H264���ص���BMP��ʽ�ģ�MJPG�ķ��ص���JPG��ʽ�ġ�
*/
CLIB_API RS cl_video_get_picture(cl_handle_t handle, void **pic, u_int32_t *size);

/**************************************************************************************************/

typedef struct {
	void *data;
	u_int32_t len;
	// ͨ������һ��Ϊ1
	u_int8_t channels;
	// λ��һ��Ϊ16λ
	u_int8_t bits;
	/* ������ */
	u_int16_t samples;
	// �Ƿ����ظ�������
	u_int32_t repeat;
} cl_sound_data_t;

/*
	ȡ����һ������
*/
CLIB_API RS cl_audio_get_sound(cl_handle_t handle, cl_sound_data_t *sd);

/*
	��������
*/
CLIB_API RS cl_audio_put_sound(cl_handle_t handle, cl_sound_data_t *sd);

/**************************************************************************************************/

/*
	������Ȩ
*/
CLIB_API RS cl_audio_request_speek(cl_handle_t handle);
/*
	�ͷŷ���Ȩ
*/
CLIB_API RS cl_audio_release_speek(cl_handle_t handle);

/**************************************************************************************************/

#ifndef	QUALITY_SMART
	// ����(jpeg), ����(h264)
	#define	QUALITY_SMART	0
	// ����(jpeg)������(h264)
	#define	QUALITY_SMOOTH	1
	// ����(jpeg)������(h264)
	#define	QUALITY_CLEAR	2
	//720p(h264)
	#define QUALITY_720P 3
#endif


typedef struct {
	// QUALITY_xxx
	u_int16_t quality;
	u_int16_t width;
	u_int16_t height;
	u_int16_t fps;
} cl_video_quality_t;

/*
	���ܣ�
		���ÿ���Ƶʱ�ĵ�ǰ����
	����IN��
		slave: ��ش��豸
		qulity: ��������QUALITY_xxx
		width: ��Ƶ������ء��Ϸ�����0��320��640��1280��0��ʾ���豸�˸���qulityѡ��
		height: ��Ƶ�߶����ء��Ϸ�����0��240��480��720��0��ʾ���豸�˸���qulityѡ��
		fps: ֡�ʣ��Ϸ���Ϊ0��5��10��15��20��25��30��0��ʾ���豸�˸���qulityѡ��
	����OUT��
		��
	���أ�
		RS_OK: �ɹ�
		������ʧ��		
*/
CLIB_API RS cl_video_set_quality(cl_handle_t handle, cl_video_quality_t *q);

/**************************************************************************************************/

/*
	����:
		ͼ��ת
	����IN:
		@slave_handle: Ҫ��ת������ͷ
	����OUT��
		��
	���أ�
		RS_OK: �ɹ�
		������ʧ��		
*/
CLIB_API RS cl_video_flip(cl_handle_t handle);

/**************************************************************************************************/

typedef enum {
	ptz_roll_stop = 0,
	ptz_roll_left = 1,
	ptz_roll_right = 2,
	ptz_roll_up = 3,
	ptz_roll_down = 4
} ptz_roll_t;
	
/*
	����:
		��ת��̨
	����:
		@left_right: 
			ptz_roll_stop: ֹͣ����ת��
			ptz_roll_right: ��ת
			ptz_roll_left: ��ת
		@up_down:
			ptz_roll_stop: ֹͣ����ת��
			ptz_roll_down: ��ת
			ptz_roll_up: ��ת
*/
CLIB_API RS cl_video_ptz_roll(cl_handle_t handle, ptz_roll_t left_right, ptz_roll_t up_down);

/*
	����:
		��ʼ��ת��̨
	����:
		@left_right: 
			ptz_roll_stop: ֹͣ����ת��
			ptz_roll_right: ��ת
			ptz_roll_left: ��ת
		@up_down:
			ptz_roll_stop: ֹͣ����ת��
			ptz_roll_down: ��ת
			ptz_roll_up: ��ת
*/
CLIB_API RS cl_video_ptz_roll_start(cl_handle_t handle, ptz_roll_t left_right, ptz_roll_t up_down);

/*
	����:
		ֹͣ��ת��̨����cl_video_ptz_roll_start����ʹ��
	����:
	
*/
CLIB_API RS cl_video_ptz_roll_stop(cl_handle_t handle);


/**************************************************************************************************/
    
typedef struct {
    int32_t brightness_val;//����
    u_int32_t contrast_val; //�Աȶ�
    u_int32_t saturation_val; //���Ͷ�
    u_int32_t gain_val; //����
}cl_video_saturation_t;

typedef struct {
	/* ���� */
	u_int8_t is_custom_quality; /* �Ƿ����Զ���Ļ��� */
	u_int8_t quality; /* ���ʵȼ�: QUALITY_XXX */
	u_int16_t width;
	u_int16_t height;
	u_int16_t fps;
	// �ۿ���Ƶ�Ŀͻ�����
	u_int16_t client_count;
	u_int8_t pad[2];
	/* ���� */
	u_int32_t bitrate;
    /*��̨ת��*/
    u_int8_t roll_speed;
    /* ��Ƶ���Ͷ� */
    cl_video_saturation_t vs_stat;
	/* ���� */
	u_int8_t resv[128];
} cl_video_stat_t;
    
typedef struct {
    u_int32_t begin_time;
    u_int32_t duration;
}cl_vtap_t;

typedef struct {
    u_int32_t total_num;
    cl_vtap_t vtap[0];
}cl_vtap_list_t;
/*
	���ܣ�
		��ȡ����Ƶʱ�ĵ�ǰͳ��
*/
CLIB_API RS cl_video_get_stat(cl_handle_t handle, cl_video_stat_t *st);

/**************************************************************************************************
	  ��ʱ���������Ϣ 
 **************************************************************************************************/


typedef struct {
	u_int8_t id;		/* ����ID */
	u_int8_t enable;	/* ʹ��/���� ������ */
	u_int8_t is_once;	/* ��һ���Թ����������Թ���һ���Թ������wday��hours��minute, �����Թ������location_time */
	u_int8_t wday;		/* bit 0-6λ��Ӧ�����쵽������ */
	u_int8_t hour;		/* Сʱ 0-23 */
	u_int8_t minute;	/* ���� 0-59 */
	u_int16_t duration;/* �������(����) */
	u_int32_t location_time; /* һ���Թ���Ĵ���ʱ�� */
	char *name;		/* ��������, UTF-8��ʽ */
} cl_vrt_item_t;


// 0: ����
#define	REC_STA_INIT	0
// 1:����ʧ�ܻ���tf��δ�ҵ�
#define	REC_STA_ERROR1	1
// 2:video����δ����
#define	REC_STA_ERROR2	2
// 3:׼������ffmpeg
#define	REC_STA_READY	3
// 4:ffmpeg�Ѿ�����
#define	REC_STA_RUN	4

typedef struct {
	// ��Ƶ�Ƿ����ɹ���ֻ�н����ɹ��������is_h264��has_audio��has_audio_speek��׼ȷ����������
	bool has_establish;
	// �Ƿ�֧����̨
	bool has_ptz;
	// ��H26���뻹��MJPG��
	bool is_h264;
	// �Ƿ�֧������
	bool has_audio;
	// �Ƿ�֧�ֶԽ�
	bool has_audio_speek;

	// �Ƿ�ʹ��¼����
	u_int8_t record_enable;
	// ¼��״̬: REC_STA_XXX
	u_int8_t record_status;
	u_int8_t pad[1];
	
	// ��ʱ¼������ж�����
	u_int32_t num_timer;
	// ��ʱ¼�����ָ������
	cl_vrt_item_t **timer;
} cl_video_info_t;

/*
	����:
		¼����ܿ���
	�������:
		@slave_handle: ң�ز����ľ��
		@on: 1��ʾʹ��¼�����0��ʾ��ֹ¼�����
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_video_rec_timer_turn_on(cl_handle_t slave_handle, bool on);

/*
	����:
		��Ӷ�ʱ��ʱ¼�����
	�������:
		@slave_handle: ң�ز����ľ��
		@timer: Ҫ��ӵĶ�ʱ¼�����
			timer->name����ΪUTF-8��ʽ������С��64�ֽڡ�
		@tz: ʱ������λΪСʱ�����綫����Ϊ8
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_video_rec_timer_add(cl_handle_t slave_handle, cl_vrt_item_t *timer, int32_t tz);

/*
	����:
		�޸Ķ�ʱ¼�����
	�������:
		@slave_handle: ң�ز����ľ��
		@timer: Ҫ�޸ĵĶ�ʱ¼�����
			timer->name����ΪUTF-8��ʽ������С��64�ֽڡ�
		@tz: ʱ������λΪСʱ�����綫����Ϊ8
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_video_rec_timer_modify(cl_handle_t slave_handle, cl_vrt_item_t *timer, int32_t tz);

/*
	����:
		ɾ����ʱ¼�����
	�������:
		@slave_handle: ң�ز����ľ��
		@id: Ҫɾ���Ĺ����id
		@tz: ʱ������λΪСʱ�����綫����Ϊ8
	�������:
		��
	����:
		RS_OK: �ɹ�
		����: ʧ��
*/
CLIB_API RS cl_video_rec_timer_del(cl_handle_t slave_handle, int id);

/*
	����:
		��ȡ������ͷ�����ж�ʱ��
	�������:
		@slave_handle: ң�ز����ľ��
		@tz: ʱ������λΪСʱ�����綫����Ϊ8
	�������:
		��
	����:
		NULL: ʧ��
		����: �ɹ�, ���ص� cl_plug_info_t * ָ����ڴ�飬��Ҫ��cl_plug_free_info�����ͷ�
*/
CLIB_API cl_video_info_t *cl_video_info_get(cl_handle_t slave_handle, int32_t tz);

/*
	����:
		�ͷ� cl_video_info_get() �������ص��ڴ��
	�������:
		@info: cl_video_info_get() �������ص��ڴ��
	�������:
		��
	����:
		��
*/
CLIB_API void cl_video_info_free(cl_video_info_t *info);
    
/*
    ����:
        ������Ƶ���Ͷ�
    �������:
        @vs: ��Ƶ���ͶȲ�������
     �������:
     ��
     ����:
     ��
*/
CLIB_API RS cl_video_set_saturation(cl_handle_t slave_handle,cl_video_saturation_t* vs);
    
/*
 ����:
    ������Ƶ��̨ת���ٶ�
 �������:
    @speed:��Ƶ��̨ת�٣�0-100
 �������:
    ��
 ����:
    ��
 */
CLIB_API RS cl_video_set_roll_speed(cl_handle_t slave_handle,u_int8_t speed);

/*
 ����:
    ��ѯ��Ƶ�ж���¼��(ֻ֧�ְ����ȡ¼��
 �������:
    @begin_time ��ʼʱ��,0��ʾ����
 �������:
    ��
 ����:
    ��
 */
CLIB_API RS cl_query_vtap_list(cl_handle_t slave_handle,u_int32_t begin_time);
/*
 ���ܣ�
    ��ȡ�ϴ�cl_query_vtap_list�Ľ��
 ����IN��
    slave_handle:¼���豸�����
 ����OUT��
    ��
 ���أ�
    cl_vtap_list_t����Ƶ�б�
 */
CLIB_API cl_vtap_list_t* cl_get_vtap_list_data(cl_handle_t slave_handle);

/*
 ���ܣ�
    �ͷ�cl_get_vtap_list_data��ȡ�������ݡ�
 ����IN��
    list:cl_get_vtap_list_data��ȡ�������ݡ�
 ����OUT��
    ��
 ���أ�
    ��
 */
CLIB_API void cl_free_vtap_list_data(cl_vtap_list_t* list);
    
/*
 ���ܣ�
    ��ʼ��¼��
 ����IN��
    begin_time: ��ʼʱ��
    slave: Ҫ�ۿ���Ƶ�Ĵ��豸��ָ��
    callback���ص�����
 ����OUT��
    ��
 ���أ�
    VIDEO_XXX
 */
CLIB_API RS cl_vtap_start(cl_handle_t slave_handle,u_int32_t begin_time,
                          cl_callback_t callback, void *handle);
/*
 ���ܣ�
    ֹͣ��¼��
 ����IN��
    handle:¼���豸�����
 ����OUT��
    ��
 ���أ�
    VIDEO_XXX
 */
CLIB_API RS cl_vtap_stop(cl_handle_t handle);
    
/*
 ���ܣ�
 ��ȡ����Ƶʱ�����һ֡ͼƬ��H264���ص���BMP��ʽ�ģ�MJPG�ķ��ص���JPG��ʽ�ġ�pic_time�Ǹ�ͼ���ʱ��
 */
CLIB_API RS cl_vtap_get_picture(cl_handle_t handle, u_int32_t* pic_time,void **pic, u_int32_t *size);


#ifdef __cplusplus
}
#endif 

#endif


