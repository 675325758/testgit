#ifndef	__MP4_FMT_H__
#define	__MP4_FMT_H__


#include <time.h>
#include "client_lib.h"
#include "cl_sys.h"

#pragma warning(disable:4200) 
#pragma warning(disable:4996) 
#pragma warning(disable:4311) 

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef int RS;
#define	RS_OK	0
#define	RS_ERROR	-1

typedef int BOOL;
#undef TRUE
#undef	FALSE
#define	TRUE	1
#define	FALSE	0
*/

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char       int8_t;
typedef signed short      int16_t;
typedef signed int        int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
//#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))


#pragma pack(push,1)

typedef struct {
	u32 size;
	u32 type;
	u8 data[0];
} box_hdr_t;

typedef struct {
	u32 size;
	u32 type;
} box_hdr2_t;

typedef struct {
	box_hdr2_t hdr;
	u32 major_brand;
	u32 minor_version;
	u32 compatible_brands[0];
} box_ftyp_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 create_time;
	u32 modify_time;
	u32 time_scale;
	u32 duration;
	u16 rate_high;
	u16 rate_low;
	u8 volume_high;
	u8 volume_low;
	u8 reserved[10];
	u8 matrix[36];
	u8 pre_defined[24];
	u32 next_track_id;
} box_mvhd_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 create_time;
	u32 modify_time;
	u32 track_id;
	u8 reserved1[4];
	u32 duration;
	u8 reserved2[8];
	u16 layer;
	u16 alternate_group;
	u8 volume_high;
	u8 volume_low;
	u8 reserved3[2];
	u8 matrix[36];
	u16 width_high;
	u16 width_low;
	u16 height_high;
	u16 height_low;
} box_tkhd_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 pre_defined;	
	/* 
		��vide���� video track
		��soun���� audio track
		��hint���� hint track
	*/
	u32 handler_type;
	u8 reserved[12];
	/* track type name���ԡ�\0����β���ַ��� */
	u8 name[0];
} box_hdlr_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	/* ����ʱ�䣨�����UTCʱ��1904-01-01���������� */
	u32 create_time;
	u32 modify_time;
	u32 time_scale;
	u32 duration;
	/* ý�������롣���λΪ0������15λΪ3���ַ�����ISO 639-2/T��׼�ж��壩 5bit ascii  */
	u16 language;
	u16 pre_defined;
} box_mdhd_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u16 graphics_mode;
	u8 opcolor[6];
} box_vmhd_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u8 balance_high;
	u8 balance_low;
	u16 reserved;
} box_smhd_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	u8 url_urn[0];
} box_dref_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	u8 data[0];
} box_stsd_t;

typedef struct {
	u32 count;
	u32 duration;
} sample_paire_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	sample_paire_t sample[0];
} box_stts_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	// �ؼ�֡��sample��š�����I֡
	u32 sample_number[0];
} box_stss_t;

typedef struct {
	u32 first_chunk;
	u32 samples_per_chunk;
	u32 sample_desc_id;
} sample_to_chunk_t;

/* Sample-to-Chunk */
typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	sample_to_chunk_t stc[0];
} box_stsc_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	/*
		ȫ��sample����Ŀ��������е�sample����ͬ�ĳ��ȣ�����ֶξ������ֵ��
		��������ֶε�ֵ����0����Щ���ȴ���sample size���� 
	*/
	u32 sample_size;
	u32 entry_count;
	u32 sample_size_ar[0];
} box_stsz_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	/*
		�ֽ�ƫ�������ļ���ʼ����ǰchunk��
		��������chunk number��������һ�����
		��һ��trunk���ڶ�����ǵڶ���trunk	
	*/
	u32 chunk_offset[0];	
} box_stco_t;

typedef struct {
	/* duration of this edit segment in units of the movie's time scale */
	u32 track_duration;
	/* starting time within the media of this edit segment (in media timescale units)�� */
	u32 media_time;
	/* relative rate at which to play the media corresponding to this edit segment�� */
	u32 media_rate;
} edit_list_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	u32 entry_count;
	edit_list_t edit_list[0];
} box_elst_t;

#pragma pack(pop)


#define	MP4OF_SIMPLE	0x0001
#define	MP4OF_WRITE	0x0002

typedef struct {
	// Track header
	box_tkhd_t *tkhd;
	// an edit list
	box_elst_t *elst;
	// media header
	box_mdhd_t *mdhd;
	// handler, declares the media (handler) type
	box_hdlr_t *hdlr;
	// video media header
	box_vmhd_t *vmhd;
	// sound media heade
	box_smhd_t *smhd;
	// data reference
	box_dref_t *dref;
	// sample descriptions
	box_stsd_t *stsd;
	// time-to-sample
	box_stts_t *stts;
	// sync sample table
	box_stss_t *stss;
	// sample-to-chunk
	box_stsc_t *stsc;
	// sample sizes
	box_stsz_t *stsz;
	// chunk offset
	box_stco_t *stco;
} cont_track_t;

typedef struct vf_node_s {
	struct vf_node_s *next;
	// �����ֽ�
	u32 size;
	// �ļ��е�λ��
	u32 pos;
	bool is_i_frame;
	struct timeval now;
} vf_node_t;

typedef struct {
	// file type and compatibility
	box_ftyp_t *ftyp;
	// movie header
	box_mvhd_t *mvhd;
	// video �� audioָ������tracks������
	cont_track_t *video;
	cont_track_t *audio;
	// ��ǰ�����ĸ����
	int cur_track_id;
	// �����ID
	int max_track_id;
	// ����
	cont_track_t *tracks;
} mp4_boxes_t;

typedef struct {
	FILE *fp;
	int open_flags;
	char *filename;
	// ����һ֡һ֡��������
	int next_frame;
	mp4_boxes_t boxes; 
	int num_video_duration;
	// ÿ����Ƶ֡���Ŷ೤ʱ�䣬��λ��timescale
	u32 *video_duration;

	// һЩ�������ļ��е�λ��
	u32 pos_mdata_size;
	vf_node_t *vf_node;
	vf_node_t *vf_node_tail;
	int width;
	int height;
} mp4_handle_t;


typedef struct {
	// OUT. ��֡���Ŷ೤ʱ�䣬��λ��timescale
	u32 duration;
	// IN. ��������С
	u32 buf_size;
	// IN/OUT. ������
	u8 *buf;
} mp4_video_frame_t;

/******************************************************************/

u8 *h264_get_next_block(u8 *data, int len);
int h264_get_pic_size(u8 *data, int len, int *width, int *height);

/******************************************************************/

extern const char *i2bn(int n);
extern const char *long_name(const char *short_name);

/*
	����:
		fn: �ļ���
		flags: MP4OF_XXX
	����:
		NULL: ʧ��
		����: �ɹ�
*/
extern mp4_handle_t *mp4_open(const char *fn, int flags);
extern RS mp4_close(mp4_handle_t *h);

/*
	����: ��ǰ����ʲôpts����Ϊ�����I֡��ʼ����������λ�ò�һ�����û����õ�λ��
*/	
extern u64 mp4_seek(mp4_handle_t *h, u64 pts);

//����¼��ʱ�䳤�ȣ���λ����
extern double mp4_get_duration(mp4_handle_t *h);
/*
	������Ƶ֡pts��ʱ������
	> 0 �ɹ�
	== 0 ʧ��
*/
extern int mp4_get_video_timescale(mp4_handle_t *h);

/*
	��ȡ��Ƶ֡��Ƕ����ֽ�һ֡
*/
extern int mp4_get_max_video_frame_size(mp4_handle_t *h);

/*
	��ȡ��һ֡��Ƶ��
	����ֵ:
	> 0: �ɹ���֡���ݴ�С
	= 0: ����
	< 0: ʧ��
*/
extern int mp4_video_read_frame(mp4_handle_t *h, mp4_video_frame_t *vf);
extern RS mp4_flush(mp4_handle_t *h);
extern mp4_handle_t *mp4_create(const char *fn, int flags);
extern RS mp4_video_write_frame(mp4_handle_t *h, u8 *data, int len);
extern bool is_i_frame(u8 *data, int len);
extern void mp4_frame_2_h264(u8 *data, int len);


#ifdef __cplusplus
}
#endif


#endif

