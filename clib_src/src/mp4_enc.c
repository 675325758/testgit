#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cl_priv.h"
#include "mp4_fmt.h"

#ifdef IOS_COMPILE
#include <sys/time.h>
#endif


#define  SLICE_TYPE_IDR		0x5


static u8 avc1_320_240[] = { /* 113 bytes */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x01, 0x40, 0x00, 0xf0, 0x00, 0x48, 0x00, 0x00, 
		0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0x00, 0x00, 
		0x00, 0x23, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 
		0x40, 0x1f, 0xff, 0xe1, 0x00, 0x0c, 0x67, 0x42, 
		0x40, 0x1f, 0x96, 0x54, 0x0a, 0x0f, 0xd0, 0x0f, 
		0x39, 0xea, 0x01, 0x00, 0x04, 0x68, 0xce, 0x38, 
		0x80
	};

static u8 avc1_640_480[] = { /* 114 bytes */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x02, 0x80, 0x01, 0xe0, 0x00, 0x48, 0x00, 0x00, 
		0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0x00, 0x00, 
		0x00, 0x24, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 
		0x40, 0x1f, 0xff, 0xe1, 0x00, 0x0d, 0x67, 0x42, 
		0x40, 0x1f, 0x96, 0x54, 0x05, 0x01, 0xed, 0x00, 
		0xf3, 0x9e, 0xa0, 0x01, 0x00, 0x04, 0x68, 0xce, 
		0x38, 0x80
	};

static u8 avc1_1280_720[] = { /* 114 bytes */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x05, 0x00, 0x02, 0xd0, 0x00, 0x48, 0x00, 0x00, 
		0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0x00, 0x00, 
		0x00, 0x24, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 
		0x40, 0x1f, 0xff, 0xe1, 0x00, 0x0d, 0x67, 0x42, 
		0x40, 0x1f, 0x96, 0x54, 0x02, 0x80, 0x2d, 0xd0, 
		0x0f, 0x39, 0xea, 0x01, 0x00, 0x04, 0x68, 0xce, 
		0x38, 0x80	
};

typedef struct {
	int width;
	int heght;
	u8 *avc1;
	int avc1_size;
} avc1_wh_map_t;

avc1_wh_map_t avc1_wh_map[] = {
	{320, 240, avc1_320_240, sizeof(avc1_320_240)},
	{640, 480, avc1_640_480, sizeof(avc1_640_480)},
	{1280, 720, avc1_1280_720, sizeof(avc1_1280_720)},
	{0, 0, NULL, 0}
};

static u8 matrix[36] ={
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x40, 0x00, 0x00, 0x00};

static u8 udta_data[] = {
	0x00, 0x00, 0x00, 0x5a, 0x6d, 0x65, 0x74, 0x61, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 
	0x68, 0x64, 0x6c, 0x72, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x6d, 0x64, 0x69, 0x72, 
	0x61, 0x70, 0x70, 0x6c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x2d, 0x69, 0x6c, 0x73, 0x74, 0x00, 0x00, 0x00, 
	0x25, 0xa9, 0x74, 0x6f, 0x6f, 0x00, 0x00, 0x00, 
	0x1d, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x61, 0x76, 
	0x66, 0x35, 0x34, 0x2e, 0x32, 0x39, 0x2e, 0x31, 
	0x30, 0x34, 
};

avc1_wh_map_t *avc1_wh_map_lookup(int w, int h)
{
	int i;
	
	for (i = 0; avc1_wh_map[i].avc1 != NULL; i++) {
		if (avc1_wh_map[i].width == w && avc1_wh_map[i].heght == h)
			return &avc1_wh_map[i];
	}

	return NULL;
}

box_hdr_t *box_alloc(u32 type, int size)
{
	box_hdr_t *box;

	if ((box = calloc(size, 1)) == NULL) {
		printf("Out of memory: type=%s, size=%d\n", i2bn(type), size);
		return NULL;
	}

	box->type = type;
	box->size = htonl(size);

	return box;
}

static RS box_ftyp_write(mp4_handle_t *h)
{
	int i = 0;
	box_ftyp_t *box;

	box = (box_ftyp_t *)box_alloc(MKTAG('f', 't', 'y', 'p'), 32);
	box->major_brand = MKTAG('i', 's', 'o', 'm');
	box->minor_version = htonl(512);
	box->compatible_brands[i++] = MKTAG('i', 's', 'o', 'm');
	box->compatible_brands[i++] = MKTAG('i', 's', 'o', '2');
	box->compatible_brands[i++] = MKTAG('a', 'v', 'c', '1');
	box->compatible_brands[i++] = MKTAG('m', 'p', '4', '1');

	if (fwrite(box, ntohl(box->hdr.size), 1, h->fp) != 1) {
		printf("%s fwrite failed\n", __FUNCTION__);
		return RS_ERROR;
	}

	return RS_OK;
}

static RS box_free_write(mp4_handle_t *h)
{
	box_hdr_t box;

	box.type = MKTAG('f', 'r', 'e', 'e');
	box.size = htonl(sizeof(box_hdr_t));

	if (fwrite((void *)&box, ntohl(box.size), 1, h->fp) != 1) {
		printf("%s fwrite failed\n", __FUNCTION__);
		return RS_ERROR;
	}

	return RS_OK;
}

static RS mp4_write_head(mp4_handle_t *h)
{
	box_hdr_t box;
	
	if (box_ftyp_write(h) != RS_OK
		|| box_free_write(h) != RS_OK)
		return RS_ERROR;

	box.type = MKTAG('m', 'd', 'a', 't');
	box.size = 0;
	
	h->pos_mdata_size = ftell(h->fp);
	if (fwrite((void *)&box, sizeof(box_hdr_t), 1, h->fp) != 1) {
		printf("%s fwrite failed\n", __FUNCTION__);
		return RS_ERROR;
	}


	return RS_OK;
}

bool is_i_frame(u8 *data, int len)
{
	/* sonix方案压缩的，在第三块 */
	int begin = 24;

	// 头部必然是一个流。如果不是，可能是jpg的，不找了
	if (*(u32 *)data != htonl(0x00000001))
		return false;
	
	data = h264_get_next_block(data + begin, len - begin);
	if (data == NULL)
		return false;
	data += 4;

	if (((*data) & 0x1F) == SLICE_TYPE_IDR)
		return true;

	return false;
}

static RS record_frame_info(mp4_handle_t *h, u8 *data, int len)
{
	int width = 0, height = 0;
	vf_node_t *node;

	
	if (h->vf_node == NULL) {
		if ( ! is_i_frame(data, len)) {
			printf("First frame not I Frame, Drop it\n");
			return RS_ERROR;
		}

		h264_get_pic_size(data, len, &width, &height);

		if (avc1_wh_map_lookup(width, height) == NULL) {
			printf("First frame unknow width=%d, height=%d, Drop it\n", width, height);
			return RS_ERROR;
		}
		h->width = width;
		h->height = height;
		printf("First I frame, width=%d, height=%d\n", width, height);
	}

	if ((node = (vf_node_t *)calloc(sizeof(vf_node_t), 1)) == NULL) {
		printf("%s out of memory\n", __FUNCTION__);
		return RS_ERROR;
	}

	gettimeofday(&node->now, NULL);
	node->size = len;
	node->pos = ftell(h->fp);
	node->is_i_frame = is_i_frame(data, len);

	if (h->vf_node_tail == NULL) {
		h->vf_node_tail = h->vf_node = node;
	} else {
		h->vf_node_tail->next = node;
		h->vf_node_tail = node;
	}

	return RS_OK;
}

void h264_block_2_mp4(u8 *data, int len)
{
	u8 *end = data + len;
	u8 *next;

	for ( ; data != NULL && data < end; data = next) {
		if ((data = h264_get_next_block(data, (int)(end - data))) == NULL)
			break;
		next = h264_get_next_block(data + 4, (int)(end - data - 4));

		*((u32 *)data) = htonl((u32)((next == NULL ? end : next) - data - 4));
	}
}

RS mp4_video_write_frame(mp4_handle_t *h, u8 *data, int len)
{
	if (record_frame_info(h, data, len) != RS_OK)
		return RS_ERROR;

	h264_block_2_mp4(data, len);
	
	if (fwrite(data, len, 1, h->fp) != 1) {
		printf("%s fwrite failed\n", __FUNCTION__);
		return RS_ERROR;
	}

	return RS_OK;
}

static RS update_mdat_size(mp4_handle_t *h)
{
	u32 n;
	
	// update media data size
	n = ftell(h->fp) - h->pos_mdata_size;
	n = htonl(n);
	fseek(h->fp, h->pos_mdata_size, SEEK_SET);
	fwrite(&n, 4, 1, h->fp);
	fseek(h->fp, 0, SEEK_END);

	return RS_OK;
}

static u32 cal_duration(struct timeval *begin, struct timeval *end, int timescal)
{
	double t;

	t = (end->tv_sec + (double)end->tv_usec/1000000) - (begin->tv_sec + (double)begin->tv_usec/1000000);

	return (u32)(t*timescal);
}

/*
	moov
		mvhd
		trak
			tkhd
			edts
				elst
			mdia
				mdhd
				hdlr
				minf
					vmhd
					dinf
						dref
							url
					stbl
						stsd
							avc1
						stts
						stss
						stsc
						stsz
						stco
		udta
*/
static RS build_moov(mp4_handle_t *h, box_hdr_t *moov, int num_frame, int num_chunk)
{
	int i;
	vf_node_t *node;
	u8 *end = NULL;
	box_mvhd_t *mvhd;
	int timescale = 1000;
	double t;
	avc1_wh_map_t *awm;
	box_hdr_t *trak;
	box_tkhd_t *tkhd;
	box_hdr_t *edts;
	box_elst_t *elst;
	box_hdr_t *mdia;
	box_mdhd_t *mdhd;
	box_hdlr_t *hdlr;
	box_hdr_t *minf;
	box_vmhd_t *vmhd;
	box_hdr_t *dinf;
	box_dref_t *dref;
	box_hdr_t *url;
	box_hdr_t *stbl;
	box_hdr_t *avc1;
	box_stsd_t *stsd;
	box_stts_t *stts;
	box_stss_t *stss;
	box_stsc_t *stsc;
	box_stsz_t *stsz;
	box_stco_t *stco;
	box_hdr_t *udta;
	int count;

	awm = avc1_wh_map_lookup(h->width, h->height);

	/*
		build mvhd
	*/
	mvhd = (box_mvhd_t *)(((u8 *)moov) + sizeof(box_hdr_t));
	mvhd->hdr.type = MKTAG('m', 'v', 'h', 'd');
	mvhd->hdr.size = htonl(sizeof(box_mvhd_t));
	mvhd->time_scale = htonl(timescale);
	// 计算持续时间。最后一帧取所有帧的平均值
	t = (h->vf_node_tail->now.tv_sec + (double)h->vf_node_tail->now.tv_usec/1000000) 
		- (h->vf_node->now.tv_sec + (double)h->vf_node->now.tv_usec/1000000);
	t = t/(num_frame-1) * num_frame;
	mvhd->duration = htonl((u32)(t*timescale));
	mvhd->rate_high = htons(1);
	mvhd->rate_low = htons(0);
	mvhd->volume_high = 1;
	mvhd->volume_low = 0;
	memcpy(mvhd->matrix, matrix, sizeof(mvhd->matrix));
	// 先只有视频流一个，没有音频流
	mvhd->next_track_id = htonl(2);

	/*
		build track
	*/
	
	trak = (box_hdr_t *)(((u8 *)mvhd) + sizeof(box_mvhd_t));
	trak->type = MKTAG('t', 'r', 'a', 'k');

	/*
		build tkhd
	*/
	
	tkhd = (box_tkhd_t *)(((u8 *)trak) + sizeof(box_hdr_t));
	tkhd->hdr.type = MKTAG('t', 'k', 'h', 'd');
	tkhd->hdr.size = htonl(sizeof(box_tkhd_t));
	tkhd->track_id = htonl(1);
	memcpy(tkhd->matrix, matrix, sizeof(tkhd->matrix));
	tkhd->width_high = htons(h->width);
	tkhd->height_high = htons(h->height);

	/*
		edts
	*/
	
	edts = (box_hdr_t *)(((u8 *)tkhd) + ntohl(tkhd->hdr.size));
	edts->type = MKTAG('e', 'd', 't', 's');
	edts->size = htonl(sizeof(box_hdr_t) + sizeof(box_elst_t) + sizeof(edit_list_t)*1);

	
	elst = (box_elst_t *)edts->data;
	elst->hdr.type = MKTAG('e', 'l', 's', 't');
	elst->hdr.size = htonl(sizeof(box_elst_t) + sizeof(edit_list_t)*1);
	elst->entry_count = htonl(1);
	elst->edit_list[0].track_duration = mvhd->duration;
	elst->edit_list[0].media_time = 0;
	elst->edit_list[0].media_rate = htonl(65536);

	/*
		mdia [mdhd hdlr minf]
	*/
	
	mdia = (box_hdr_t *)(((u8 *)edts) + ntohl(edts->size));
	mdia->type = MKTAG('m', 'd', 'i', 'a');

	
	mdhd = (box_mdhd_t *)mdia->data;
	mdhd->hdr.type = MKTAG('m', 'd', 'h', 'd');
	mdhd->hdr.size = htonl(sizeof(box_mdhd_t));
	mdhd->time_scale = htonl(timescale);
	mdhd->duration = mvhd->duration;
	mdhd->language = htons(0x55c4); /* und */

	
	hdlr = (box_hdlr_t *)(mdhd + 1);
	hdlr->hdr.type = MKTAG('h', 'd', 'l', 'r');
	hdlr->handler_type = MKTAG('v', 'i', 'd', 'e');
	strcpy((char *)hdlr->name, "VideoHandler");
	hdlr->hdr.size = htonl((u32)(sizeof(box_hdlr_t) + strlen((char *)hdlr->name) + 1));

	/* minf [vmhd dinf stbl] */
	
	minf = (box_hdr_t *)((u8 *)hdlr + ntohl(hdlr->hdr.size));
	minf->type = MKTAG('m', 'i', 'n', 'f');

	
	vmhd = (box_vmhd_t *)minf->data;
	vmhd->hdr.type = MKTAG('v', 'm', 'h', 'd');
	vmhd->hdr.size = htonl(sizeof(box_vmhd_t));

	/* dinf [dref [url ]] */
	
	dinf = (box_hdr_t *)((u8 *)(vmhd + 1));
	dinf->type = MKTAG('d', 'i', 'n', 'f');
	dinf->size = htonl(36);

	
	dref = (box_dref_t *)dinf->data;
	dref->hdr.type = MKTAG('d', 'r', 'e', 'f');
	dref->hdr.size = htonl(28);
	dref->entry_count = htonl(1);

	
	url = (box_hdr_t *)dref->url_urn;
	url->type = MKTAG('u', 'r', 'l', ' ');
	url->size = htonl(12);
	*(u32 *)url->data = htonl(1);

	/* stbl [stsd stts stss stsc stsz stco] */
	
	stbl = (box_hdr_t *)(((u8 *)dinf) + ntohl(dinf->size));
	stbl->type = MKTAG('s', 't', 'b', 'l');

	
	stsd = (box_stsd_t *)stbl->data;
	stsd->hdr.type = MKTAG('s', 't', 's', 'd');
	stsd->entry_count = htonl(1);

	
	avc1 = (box_hdr_t *)stsd->data;
	avc1->type = MKTAG('a', 'v', 'c', '1');
	stsd->hdr.size = htonl(sizeof(box_stsd_t) + sizeof(box_hdr_t) + awm->avc1_size);
	avc1->size = htonl(sizeof(box_hdr_t) + awm->avc1_size);
	memcpy(avc1->data, awm->avc1, awm->avc1_size);

	
	

	stts = (box_stts_t *)((u8 *)stsd + ntohl(stsd->hdr.size));
	stts->hdr.type = MKTAG('s', 't', 't', 's');
	SAFE_FREE(h->video_duration);
	h->video_duration = malloc(4*num_frame);
	for (i = 0, node = h->vf_node; i < (num_frame - 1); i++, node = node->next) {
		h->video_duration[i] = cal_duration(&node->now, &node->next->now, timescale);
	}
	h->video_duration[i] = ntohl(mvhd->duration)/num_frame;
	for (i = 0, count = 1; i < num_frame; i++) {
		if (i == (num_frame - 1) || h->video_duration[i] != h->video_duration[i + 1]) {
			stts->sample[stts->entry_count].count = htonl(count);
			stts->sample[stts->entry_count].duration = htonl(h->video_duration[i]);
			stts->entry_count++;

			count = 1;
		} else {
			count++;
		}
	}
	stts->hdr.size = htonl(sizeof(box_stts_t) + sizeof(sample_paire_t)*stts->entry_count);
	stts->entry_count = htonl(stts->entry_count);

	

	stss = (box_stss_t *)((u8 *)stts + ntohl(stts->hdr.size));
	stss->hdr.type = MKTAG('s', 't', 's', 's');
	for (node = h->vf_node, count = 1; node != NULL; node = node->next, count++) {
		if (node->is_i_frame) {
			stss->sample_number[stss->entry_count++] = htonl(count);
		}
	}
	stss->hdr.size = htonl(sizeof(box_stss_t) + sizeof(u32)*stss->entry_count);
	stss->entry_count = htonl(stss->entry_count);

	
	stsc = (box_stsc_t *)((u8 *)stss + ntohl(stss->hdr.size));
	stsc->hdr.type = MKTAG('s', 't', 's', 'c');
	stsc->hdr.size = htonl(28);
	stsc->entry_count = htonl(1);
	stsc->stc[0].first_chunk = htonl(1);
	stsc->stc[0].samples_per_chunk = htonl(1);
	stsc->stc[0].sample_desc_id = htonl(1);

	
	stsz = (box_stsz_t *)((u8 *)stsc + ntohl(stsc->hdr.size));
	stsz->hdr.type = MKTAG('s', 't', 's', 'z');
	stsz->hdr.size = htonl(sizeof(box_stsz_t) + 4*num_frame);
	stsz->sample_size = htonl(0);
	stsz->entry_count = htonl(num_frame);
	for (i = 0, node = h->vf_node; node != NULL; i++, node = node->next) {
		stsz->sample_size_ar[i] = htonl(node->size);
	}

	
	stco = (box_stco_t *)((u8 *)stsz + ntohl(stsz->hdr.size));
	stco->hdr.type = MKTAG('s', 't', 'c', 'o');
	stco->hdr.size = htonl(sizeof(box_stco_t) + 4*num_frame);
	stco->entry_count = htonl(num_frame);
	for (i = 0, node = h->vf_node; node != NULL; i++, node = node->next) {
		stco->chunk_offset[i] = htonl(node->pos);
	}

	/* udta */
	
	udta = (box_hdr_t *)((u8 *)stco + ntohl(stco->hdr.size));
	udta->type = MKTAG('u', 'd', 't', 'a');
	udta->size = htonl(sizeof(box_hdr_t) + sizeof(udta_data));
	memcpy(udta->data, udta_data, sizeof(udta_data));

	/* end */
	end = (u8 *)stco + ntohl(stco->hdr.size);
	
	stbl->size = htonl((u32)(end - (u8 *)stbl));
	minf->size = htonl((u32)(end - (u8 *)minf));
	mdia->size = htonl((u32)(end - (u8 *)mdia));
	trak->size = htonl((u32)(end - (u8 *)trak));

	end = (u8 *)udta + ntohl(udta->size);
	moov->size = htonl((u32)(end - (u8 *)moov));

	return RS_OK;
}


RS mp4_flush(mp4_handle_t *h)
{
	int size;
	box_hdr_t *moov;
	vf_node_t *node;
	int num_chunk, num_frame;

	if (h->vf_node == NULL) {
		printf("%s failed: no video data\n", __FUNCTION__);
		return RS_ERROR;
	}
	
	if (update_mdat_size(h) != RS_OK)	
		return RS_ERROR;

	for (node = h->vf_node, num_chunk = 0, num_frame = 0; node != NULL; node = node->next) {
		if (num_frame == 0 || node->is_i_frame) {
			num_chunk++;
		}
		num_frame++;
	}
	// stts-8*frame, stsz-4*frame, stco-4*frame, stss-4*chunk
	size = num_frame*16 + num_chunk*4 + 20*1024;
	if ((moov = box_alloc(MKTAG('m', 'o', 'o', 'v'), size)) == NULL)
		return RS_ERROR;

	build_moov(h, moov, num_frame, num_chunk);
	if (fwrite(moov, ntohl(moov->size), 1, h->fp) != 1) {
		printf("%s fwrite failed\n", __FUNCTION__);
		free(moov);
		return RS_ERROR;
	}

	free(moov);

	return RS_OK;
}

mp4_handle_t *mp4_create(const char *fn, int flags)
{
	mp4_handle_t *h;

	if ((h = calloc(sizeof(mp4_handle_t), 1)) == NULL)
		return NULL;

	h->filename = cl_strdup(fn);
	h->open_flags = flags | MP4OF_WRITE;
	
	if ((h->fp = fopen(fn, "w+b")) == NULL)
		goto err;

	mp4_write_head(h);

	return h;
	
err:
	mp4_close(h);
	return NULL;
}


