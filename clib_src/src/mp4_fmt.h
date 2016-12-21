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
		“vide”— video track
		“soun”— audio track
		“hint”— hint track
	*/
	u32 handler_type;
	u8 reserved[12];
	/* track type name，以‘\0’结尾的字符串 */
	u8 name[0];
} box_hdlr_t;

typedef struct {
	box_hdr2_t hdr;
	u8 version;
	u8 flags[3];
	/* 创建时间（相对于UTC时间1904-01-01零点的秒数） */
	u32 create_time;
	u32 modify_time;
	u32 time_scale;
	u32 duration;
	/* 媒体语言码。最高位为0，后面15位为3个字符（见ISO 639-2/T标准中定义） 5bit ascii  */
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
	// 关键帧的sample序号。比如I帧
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
		全部sample的数目。如果所有的sample有相同的长度，这个字段就是这个值。
		否则，这个字段的值就是0。那些长度存在sample size表中 
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
		字节偏移量从文件开始到当前chunk。
		这个表根据chunk number索引，第一项就是
		第一个trunk，第二项就是第二个trunk	
	*/
	u32 chunk_offset[0];	
} box_stco_t;

typedef struct {
	/* duration of this edit segment in units of the movie's time scale */
	u32 track_duration;
	/* starting time within the media of this edit segment (in media timescale units)。 */
	u32 media_time;
	/* relative rate at which to play the media corresponding to this edit segment。 */
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
	// 多少字节
	u32 size;
	// 文件中的位置
	u32 pos;
	bool is_i_frame;
	struct timeval now;
} vf_node_t;

typedef struct {
	// file type and compatibility
	box_ftyp_t *ftyp;
	// movie header
	box_mvhd_t *mvhd;
	// video 和 audio指向后面的tracks数组中
	cont_track_t *video;
	cont_track_t *audio;
	// 当前解析哪个轨道
	int cur_track_id;
	// 最大轨道ID
	int max_track_id;
	// 数组
	cont_track_t *tracks;
} mp4_boxes_t;

typedef struct {
	FILE *fp;
	int open_flags;
	char *filename;
	// 用于一帧一帧读出数据
	int next_frame;
	mp4_boxes_t boxes; 
	int num_video_duration;
	// 每个视频帧播放多长时间，单位是timescale
	u32 *video_duration;

	// 一些东西在文件中的位置
	u32 pos_mdata_size;
	vf_node_t *vf_node;
	vf_node_t *vf_node_tail;
	int width;
	int height;
} mp4_handle_t;


typedef struct {
	// OUT. 该帧播放多长时间，单位是timescale
	u32 duration;
	// IN. 缓冲区大小
	u32 buf_size;
	// IN/OUT. 缓冲区
	u8 *buf;
} mp4_video_frame_t;

/******************************************************************/

u8 *h264_get_next_block(u8 *data, int len);
int h264_get_pic_size(u8 *data, int len, int *width, int *height);

/******************************************************************/

extern const char *i2bn(int n);
extern const char *long_name(const char *short_name);

/*
	参数:
		fn: 文件名
		flags: MP4OF_XXX
	返回:
		NULL: 失败
		其他: 成功
*/
extern mp4_handle_t *mp4_open(const char *fn, int flags);
extern RS mp4_close(mp4_handle_t *h);

/*
	返回: 当前跳到什么pts。因为必须从I帧开始，所以跳的位置不一定是用户设置的位置
*/	
extern u64 mp4_seek(mp4_handle_t *h, u64 pts);

//返回录像时间长度，单位：秒
extern double mp4_get_duration(mp4_handle_t *h);
/*
	返回视频帧pts的时间因子
	> 0 成功
	== 0 失败
*/
extern int mp4_get_video_timescale(mp4_handle_t *h);

/*
	获取视频帧最长是多少字节一帧
*/
extern int mp4_get_max_video_frame_size(mp4_handle_t *h);

/*
	读取下一帧视频。
	返回值:
	> 0: 成功，帧数据大小
	= 0: 结束
	< 0: 失败
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

