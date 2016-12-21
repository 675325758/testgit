#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cl_priv.h"
#include "mp4_fmt.h"

typedef struct {
	char *short_name;
	char *long_name;
} name_map_t;

static name_map_t name_map[] = {
	{"ftyp", "file type and compatibility"},
	{"mvhd", "movie header"},
	{"trak", "track or stream"},
	{"tkhd", "Track header"},
	{"edts", "edit list container"},
	{"elst", "an edit list"},
	{"mdia", "media information in a track"},
	{"mdhd", "media header"},
	{"hdlr", "handler, declares the media (handler) type"},
	{"minf", "media information container"},
	{"vmhd", "video media header"},
	{"dinf", "data information box, container"},
	{"dref", "data reference"},
	{"url ", "a URL"},
	{"stbl", "sample table"},
	{"stsd", "sample descriptions"},
	{"stts", "time-to-sample"},
	{"stss", "sync sample table"},
	{"stsc", "sample-to-chunk"},
	{"stsz", "sample sizes"},
	{"stco", "chunk offset"},
	{"udta", "user-data"},
	{"avc1", "I don't know"}, /* avcN */
	{"mp4a", "I don't know"},
	{"free", "free space"},
	{"mdat", "media data"},
	{"moov", "Movie"},
	{"smhd", "sound media heade"},
	{NULL, NULL}
};

/*****************************************************
	外部接口
 *****************************************************/
 
const char *i2bn(int n)
{
	static char buf[8];

	*(u32 *)buf = n;
	buf[4] = '\0';

	return buf;

}

const char *long_name(const char *short_name)
{
	int i;

	for (i = 0; name_map[i].short_name != NULL; i++) {
		if (strcmp(short_name, name_map[i].short_name) == 0)
			return name_map[i].long_name;
	}

	return "Not Found";
}


typedef int (* func_t)();

typedef struct {
	u32 type;
	func_t func;
} box_parse_map_t;

/****************************************************************************************/

static RS box_parse(mp4_handle_t *h, box_hdr_t *box);

#define	BOX_CHECK_SIZE(size, min_size) \
	do { \
		if ((size) < (min_size)) { \
			return RS_ERROR; \
		} \
	} while (0)

#define	CHECK_ENTRY_SIZE(type, entry_type) \
	do { \
		if (box->entry_count*sizeof(entry_type) + sizeof(type) > box->hdr.size) { \
			return RS_ERROR; \
		} \
	} while (0)
	
#define	BOX_SAFE_SET(a, b)	\
	do { \
		if ((a) != NULL) { \
			free(a); \
		} \
		a = (b); \
	} while (0)

static RS box_container_parse(mp4_handle_t *h, box_hdr_t *box);
		
static RS box_ftyp_parse(mp4_handle_t *h, box_ftyp_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_ftyp_t));

	box->minor_version = ntohl(box->minor_version);

	BOX_SAFE_SET(h->boxes.ftyp, box);

	return RS_OK;
}

static RS box_trak_parse(mp4_handle_t *h, box_hdr_t *box)
{
	h->boxes.cur_track_id++;
	if (h->boxes.cur_track_id > h->boxes.max_track_id) {
		printf("Out of track id, max=%d\n", (int)h->boxes.max_track_id);
		return RS_ERROR;
	}

	return box_container_parse(h, box);
}
	
static RS box_mvhd_parse(mp4_handle_t *h, box_mvhd_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_mvhd_t));

	box->create_time = ntohl(box->create_time);
	box->modify_time = ntohl(box->modify_time);
	box->time_scale = ntohl(box->time_scale);
	box->duration = ntohl(box->duration);
	box->rate_high = ntohs(box->rate_high);
	box->rate_low = ntohs(box->rate_low);
	box->next_track_id = ntohl(box->next_track_id);

	h->boxes.max_track_id = box->next_track_id;
	SAFE_FREE(h->boxes.tracks);
	h->boxes.tracks = calloc(sizeof(cont_track_t), h->boxes.max_track_id + 1);
	if (h->boxes.tracks == NULL) {
		printf("Out of memory: trak id=%d\n", (int)box->next_track_id);
		return RS_ERROR;
	}
	
	BOX_SAFE_SET(h->boxes.mvhd, box);

	return RS_OK;
}

static RS box_hdlr_parse(mp4_handle_t *h, box_hdlr_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_hdlr_t));

	box->pre_defined = ntohl(box->pre_defined);

	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].hdlr, box);

	if (box->handler_type == MKTAG('v', 'i', 'd', 'e')) {
		h->boxes.video = &h->boxes.tracks[h->boxes.cur_track_id];
	} else 	if (box->handler_type == MKTAG('s', 'o', 'u', 'n')) {
		h->boxes.audio = &h->boxes.tracks[h->boxes.cur_track_id];
	}

	return RS_OK;	
}

static RS box_mdhd_parse(mp4_handle_t *h, box_mdhd_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_mdhd_t));
	
	box->create_time = ntohl(box->create_time);
	box->modify_time = ntohl(box->modify_time);
	box->time_scale = ntohl(box->time_scale);
	box->duration = ntohl(box->duration);
	box->language = ntohs(box->language);
	box->pre_defined = ntohs(box->pre_defined);
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].mdhd, box);

	return RS_OK;
}

static RS box_tkhd_parse(mp4_handle_t *h, box_tkhd_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_tkhd_t));

	box->create_time = ntohl(box->create_time);
	box->modify_time = ntohl(box->modify_time);
	box->track_id = ntohl(box->track_id);
	box->duration = ntohl(box->duration);
	box->layer = ntohs(box->layer);
	box->alternate_group = ntohs(box->alternate_group);
	box->width_high = ntohs(box->width_high);
	box->width_low = ntohs(box->width_low);
	box->height_high = ntohs(box->height_high);
	box->height_low = ntohs(box->height_low);
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].tkhd, box);

	return RS_OK;
}

static RS box_vmhd_parse(mp4_handle_t *h, box_vmhd_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_vmhd_t));

	box->graphics_mode = ntohs(box->graphics_mode);

	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].vmhd, box);

	return RS_OK;
}

static RS box_smhd_parse(mp4_handle_t *h, box_smhd_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_smhd_t));

	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].smhd, box);

	return RS_OK;
}

static RS box_dref_parse(mp4_handle_t *h, box_dref_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_dref_t));

	box->entry_count = ntohl(box->entry_count);
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].dref, box);

	return RS_OK;
}

static RS box_stsd_parse(mp4_handle_t *h, box_stsd_t *box)
{
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_stsd_t));

	box->entry_count = ntohl(box->entry_count);
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].stsd, box);

	return RS_OK;
}

static RS box_stts_parse(mp4_handle_t *h, box_stts_t *box)
{
	u32 i;

	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_stts_t));

	box->entry_count = ntohl(box->entry_count);
	CHECK_ENTRY_SIZE(box_stts_t, sample_paire_t);
	
	for (i = 0; i < box->entry_count; i++) {
		box->sample[i].count = ntohl(box->sample[i].count);
		box->sample[i].duration = ntohl(box->sample[i].duration);
	}
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].stts, box);

	return RS_OK;
}

static RS box_stss_parse(mp4_handle_t *h, box_stss_t *box)
{
	u32 i;
	
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_stss_t));

	box->entry_count = ntohl(box->entry_count);
	CHECK_ENTRY_SIZE(box_stss_t, u32);

	for (i = 0; i < box->entry_count; i++) {
		box->sample_number[i] = ntohl(box->sample_number[i]);
	}
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].stss, box);

	return RS_OK;
}

static RS box_stsc_parse(mp4_handle_t *h, box_stsc_t *box)
{
	u32 i;
	
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_stsc_t));

	box->entry_count = ntohl(box->entry_count);
	CHECK_ENTRY_SIZE(box_stsc_t, sample_to_chunk_t);

	for (i = 0; i < box->entry_count; i++) {
		box->stc[i].first_chunk = ntohl(box->stc[i].first_chunk);
		box->stc[i].samples_per_chunk = ntohl(box->stc[i].samples_per_chunk);
		box->stc[i].sample_desc_id = ntohl(box->stc[i].sample_desc_id);
	}
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].stsc, box);

	return RS_OK;
}

static RS box_stsz_parse(mp4_handle_t *h, box_stsz_t *box)
{
	u32 i;
	
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_stsz_t));

	box->sample_size = ntohl(box->sample_size);
	box->entry_count = ntohl(box->entry_count);

	if (box->sample_size == 0) {
		CHECK_ENTRY_SIZE(box_stsz_t, u32);

		for (i = 0; i < box->entry_count; i++) {
			box->sample_size_ar[i] = ntohl(box->sample_size_ar[i]);
		}
	}
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].stsz, box);

	return RS_OK;
}

static RS box_stco_parse(mp4_handle_t *h, box_stco_t *box)
{
	u32 i;
	
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_stsz_t));

	box->entry_count = ntohl(box->entry_count);
	CHECK_ENTRY_SIZE(box_stco_t, u32);
	
	for (i = 0; i < box->entry_count; i++) {
		box->chunk_offset[i] = ntohl(box->chunk_offset[i]);
	}
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].stco, box);

	return RS_OK;
}

static RS box_elst_parse(mp4_handle_t *h, box_elst_t *box)
{
	u32 i;
	
	BOX_CHECK_SIZE(box->hdr.size, sizeof(box_elst_t));

	box->entry_count = ntohl(box->entry_count);
	CHECK_ENTRY_SIZE(box_elst_t, edit_list_t);

	for (i = 0; i < box->entry_count; i++) {
		box->edit_list[i].track_duration = ntohl(box->edit_list[i].track_duration);
		box->edit_list[i].media_time = ntohl(box->edit_list[i].media_time);
		box->edit_list[i].media_rate = ntohl(box->edit_list[i].media_rate);
	}
	
	BOX_SAFE_SET(h->boxes.tracks[h->boxes.cur_track_id].elst, box);

	return RS_OK;
}
	
/*
	有些BOX是容器性质，里面又有很多小BOX
*/
static RS box_container_parse(mp4_handle_t *h, box_hdr_t *box)
{
	RS ret = RS_ERROR;
	u8 *data;
	int size, ofs;
	box_hdr_t *sub, *nb = NULL;

	data = box->data;
	size = box->size - sizeof(box_hdr_t);
	
	for (ofs = 0; ofs < size; ofs += sub->size) {
		sub = (box_hdr_t *)(data + ofs);
		sub->size = ntohl(sub->size);
		if (sub->size <= 1) {
			printf("BAD BOX, type=%s, size=%u\n", i2bn(sub->type), sub->size);
			goto done;
		}
		if (sub->size + ofs > box->size) {
			printf("BAD BOX, type=%s, size=%u\n", i2bn(sub->type), sub->size);
			goto done;
		}

		if ((nb = malloc(sub->size)) == NULL)
			goto done;
		memcpy(nb, sub, sub->size);
		if (box_parse(h, nb) != RS_OK) {
			printf("Parse BOX %s size=%d failed\n", i2bn(sub->type), (int)sub->size);
			free(nb);
			goto done;
		}

	}

	ret = RS_OK;
	
done:
	if (ret == RS_OK)
		free(box);
	
	return ret;
}

/****************************************************************************************/

static box_parse_map_t box_parse_map[] = {
	{MKTAG('f', 't', 'y', 'p'), box_ftyp_parse},
	{MKTAG('m', 'v', 'h', 'd'), box_mvhd_parse},
	{MKTAG('t', 'r', 'a', 'k'), box_trak_parse},
	{MKTAG('t', 'k', 'h', 'd'), box_tkhd_parse},
	{MKTAG('h', 'd', 'l', 'r'), box_hdlr_parse},
	{MKTAG('m', 'd', 'h', 'd'), box_mdhd_parse},
	{MKTAG('v', 'm', 'h', 'd'), box_vmhd_parse},
	{MKTAG('s', 'm', 'h', 'd'), box_smhd_parse},
	{MKTAG('d', 'r', 'e', 'f'), box_dref_parse},
	{MKTAG('s', 't', 's', 'd'), box_stsd_parse},
	{MKTAG('s', 't', 't', 's'), box_stts_parse},
	{MKTAG('s', 't', 's', 's'), box_stss_parse},
	{MKTAG('s', 't', 's', 'c'), box_stsc_parse},
	{MKTAG('s', 't', 's', 'z'), box_stsz_parse},
	{MKTAG('s', 't', 'c', 'o'), box_stco_parse},
	{MKTAG('e', 'l', 's', 't'), box_elst_parse},
	{MKTAG('m', 'o', 'o', 'v'), box_container_parse},
	{MKTAG('t', 'r', 'a', 'k'), box_container_parse},
	{MKTAG('m', 'd', 'i', 'a'), box_container_parse},
	{MKTAG('m', 'i', 'n', 'f'), box_container_parse},
	{MKTAG('d', 'i', 'n', 'f'), box_container_parse},
	{MKTAG('s', 't', 'b', 'l'), box_container_parse},
	{MKTAG('e', 'd', 't', 's'), box_container_parse},
	{0, NULL}
};

static box_parse_map_t *box_parse_map_lookup(u32 type)
{
	int i;

	for (i = 0; box_parse_map[i].type != 0; i++) {
		if (box_parse_map[i].type == type)
			return &box_parse_map[i];
	}

	return NULL;

}

static RS box_parse(mp4_handle_t *h, box_hdr_t *box)
{
	box_parse_map_t *bpm;

	if ((bpm = box_parse_map_lookup(box->type)) != NULL) {
		return bpm->func(h, box);
	}

	printf("Ignore box %s size=%d\n", i2bn(box->type), (int)box->size);
	free(box);

	return RS_OK;
}

/****************************************************************************************/

static RS box_read_and_parse(mp4_handle_t *h)
{
	int n;
	long pos = 0;
	box_hdr_t box, *nb = NULL;
	box_parse_map_t *bpm = NULL;
	
	if (fread(&box, 1, sizeof(box_hdr_t), h->fp) < sizeof(box_hdr_t))
		return RS_ERROR;
	box.size = ntohl(box.size);
	if (box.size == 1) {
		printf("too big file, type=%s, box size = 1!!!!\n", i2bn(box.type));
		return RS_ERROR;
	}

	n = box.size - sizeof(box_hdr_t);
	pos = ftell(h->fp) + n;
	
	if ((bpm = box_parse_map_lookup(box.type)) == NULL)
		goto skip_parse;

	if ((nb = malloc(box.size)) == NULL) {
		printf("Out of memory: box type=%s, size=%u\n", i2bn(box.type), box.size);
		return RS_ERROR;
	}

	nb->type = box.type;
	nb->size = box.size;	
	
	if (n != 0) {
		if (fread(nb->data, 1, n, h->fp) != n) {
			printf("not enought data for BOX %s, say %d\n", i2bn(box.type), (int)n);
			free(nb);
			return RS_ERROR;
		}
	}

skip_parse:
	fseek(h->fp, pos, SEEK_SET);
	
	if (bpm != NULL && nb != NULL) {
		if (bpm->func(h, nb) != RS_OK) {
			printf("Parse Box %s failed\n", i2bn(box.type));
			free(nb);
			return RS_ERROR;
		}
	}
	
	return RS_OK;
}

static RS build_video_duration(mp4_handle_t *h)
{
	int count;
	box_stts_t *stts;
	u32 i, k;
	
	if (h->boxes.video == NULL || h->boxes.video->stts == NULL)
		return RS_OK;

	stts = h->boxes.video->stts;
	for (i = 0, count = 0; i < stts->entry_count; i++) {
		count += stts->sample[i].count;
	}

	SAFE_FREE(h->video_duration);
	h->video_duration = malloc(4*count);
	if (h->video_duration == NULL) {
		printf("Out of memory: frame count=%d\n", (int)count);
		return RS_ERROR;
	}

	h->num_video_duration = count;
	for (i = 0, count = 0; i < stts->entry_count; i++) {
		for (k = 0; k < stts->sample[i].count; k++) {
			h->video_duration[count++] = stts->sample[i].duration;
		}
	}

	return RS_OK;
}

mp4_handle_t *mp4_open_read(const char *fn, int flags)
{
	mp4_handle_t *h;


	if ((h = calloc(sizeof(mp4_handle_t), 1)) == NULL)
		return NULL;

	h->filename = cl_strdup(fn);
	h->open_flags = flags;
	
	if ((h->fp = fopen(fn, "rb")) == NULL)
		goto err;

	while (box_read_and_parse(h) == RS_OK) {
	}

	if (build_video_duration(h) != RS_OK)
		goto err;

	return h;
	
err:
	mp4_close(h);
	return NULL;
}

mp4_handle_t *mp4_open(const char *fn, int flags)
{
	if (flags & MP4OF_WRITE)
		return mp4_create(fn, flags);

	return mp4_open_read(fn, flags);
}

RS mp4_close(mp4_handle_t *h)
{
	RS ret = RS_OK;
	int i;
	vf_node_t *node, *next;
	
	mp4_boxes_t *bs;
	
	if (h == NULL)
		return RS_ERROR;

	if (h->open_flags & MP4OF_WRITE) {
		if (mp4_flush(h) != RS_OK) {
			ret = RS_ERROR;
		}
	}
	
	if (h->fp != NULL) {
		fclose(h->fp);
	}
	// 保存失败，删除错误的不完整的文件
	if (ret == RS_ERROR) {
		remove(h->filename);
	}
	
	SAFE_FREE(h->filename);

	bs = &h->boxes;
	SAFE_FREE(bs->ftyp);
	SAFE_FREE(bs->mvhd);
	SAFE_FREE(h->video_duration);

	if (bs->tracks != NULL) {
		for (i = 0; i <= bs->max_track_id; i++) {
			SAFE_FREE(bs->tracks[i].tkhd);
			SAFE_FREE(bs->tracks[i].elst);
			SAFE_FREE(bs->tracks[i].mdhd);
			SAFE_FREE(bs->tracks[i].hdlr);
			SAFE_FREE(bs->tracks[i].vmhd);
			SAFE_FREE(bs->tracks[i].smhd);
			SAFE_FREE(bs->tracks[i].dref);
			SAFE_FREE(bs->tracks[i].stsd);
			SAFE_FREE(bs->tracks[i].stts);
			SAFE_FREE(bs->tracks[i].stss);
			SAFE_FREE(bs->tracks[i].stsc);
			SAFE_FREE(bs->tracks[i].stsz);
			SAFE_FREE(bs->tracks[i].stco);
		}
		free(bs->tracks);
	}

	for (node = h->vf_node; node != NULL; node = next) {
		next = node->next;
		free(node);
	}
	h->vf_node = h->vf_node_tail = NULL;

	free(h);

	return ret;
}

// 返回: 当前跳到什么pts。因为必须从I帧开始，所以跳的位置不一定是用户设置的位置
u64 mp4_seek(mp4_handle_t *h, u64 seek_pts)
{
	u32 i, fn, real_fn = 0;
	u64 pts, real_pts = 0;
	box_stss_t *stss;

	if (h->boxes.video == NULL || h->boxes.video->stss == NULL || h->video_duration == NULL) {
		printf("mp4 seek failed: some data lost.\n");
		return 0;
	}

	stss = h->boxes.video->stss;

	for (i = 0, fn = 0, pts = 0; i < stss->entry_count && pts < seek_pts; i++) {
		// 记住关键帧开始位置
		real_pts = pts;
		real_fn = fn;
		
		for (; fn < stss->sample_number[i] && pts < seek_pts; fn++) {
			pts += h->video_duration[fn];
		}
	}

	// 内部以0为起始，文件中是以1为起始的
	if (real_fn > 0) {
		real_fn--;
		real_pts -= h->video_duration[real_fn];
	}

	h->next_frame = real_fn;

	return real_pts;
}

double mp4_get_duration(mp4_handle_t *h)
{
	if (h->boxes.mvhd == NULL)
		return 0.0;

	return (double)h->boxes.mvhd->duration/h->boxes.mvhd->time_scale;
}

/*
	返回视频帧pts的时间因子
	> 0 成功
	== 0 失败
*/
int mp4_get_video_timescale(mp4_handle_t *h)
{
	if (h->boxes.video == NULL || h->boxes.video->mdhd == NULL)
		return 0;

	return h->boxes.video->mdhd->time_scale;
}

/*
	获取视频帧最长是多少字节一帧
*/
int mp4_get_max_video_frame_size(mp4_handle_t *h)
{
	u32 size = 0;
	u32 i;
	box_stsz_t *stsz;

	if (h->boxes.video == NULL || h->boxes.video->stsz == NULL)
		return 0;
	stsz = h->boxes.video->stsz;

	for (i = 0; i < stsz->entry_count; i++) {
		if (stsz->sample_size_ar[i] > size)
			size = stsz->sample_size_ar[i];
	}

	return size;
}



/*
	读取下一帧视频。
	返回值:
	> 0: 成功，帧数据大小
	= 0: 结束
	< 0: 失败
*/
int mp4_video_read_frame(mp4_handle_t *h, mp4_video_frame_t *vf)
{
	size_t pos, n;
	box_stsz_t *stsz;
	box_stco_t *stco;
	
	if (h->boxes.video == NULL || h->boxes.video->stco == NULL || h->boxes.video->stsz == NULL)
		return 0;
	if (h->next_frame >= h->num_video_duration)
		return 0;

	stsz = h->boxes.video->stsz;
	stco = h->boxes.video->stco;
	if (h->next_frame >= (int)stsz->entry_count || h->next_frame >= (int)stco->entry_count) {
		printf("%s next_frame=%d, but stsz->entry_count=%d, stco->entry_count=%d\n",
			__FUNCTION__, (int)h->next_frame, (int)stsz->entry_count, (int)stco->entry_count);
		return 0;
	}
	if (stsz->sample_size_ar[h->next_frame] > vf->buf_size) {
		printf("%s failed: frame size=%d, but buffer size=%d\n", __FUNCTION__,
			(int)stsz->sample_size_ar[h->next_frame],(int) vf->buf_size);
		return -(int)stsz->sample_size_ar[h->next_frame];
	}

	if (fseek(h->fp, stco->chunk_offset[h->next_frame], SEEK_SET) != 0) {
		printf("%s fseek to %d failed\n", __FUNCTION__, (int)stco->chunk_offset[h->next_frame]);
		return -1;
	}
	for (pos = 0; pos < stsz->sample_size_ar[h->next_frame]; pos += n) {
		n = fread(vf->buf + pos, 1, stsz->sample_size_ar[h->next_frame] - pos, h->fp);
		if (n == 0) {
			printf("%s fread failed: not enought data(need %d, now %d)\n",
				__FUNCTION__, (int)stsz->sample_size_ar[h->next_frame], (int)pos);
			return -2;
		}
	}

	vf->duration = h->video_duration[h->next_frame];

	h->next_frame++;
	
	return (int)pos;
}

void mp4_frame_2_h264(u8 *data, int len)
{
	u32 *psize, size;

	psize = (u32 *)data;
	data += len;

	for ( ; (u32)psize < (u32)data; psize = (u32 *)((u8 *)psize + size + 4)) {
		size = ntohl(*psize);
		*psize = htonl(0x00000001);
	}
}


