
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#include "mp4_fmt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "dsputil.h"
#include "h264.h"
#include "yuv2bmp.h"
#include "mini264lib.h"

#ifdef IOS_COMPILE
#include "osdep.h"
#endif


#define M264_FLAG 0x19199191

typedef struct{
	int m264flag;
	void *me;
	int frame;
	int angle;
	int opened;
	AVCodecContext *c;
	AVFrame *picture;
}m264_t;

#define IS_M264_CTX(ctx) ((ctx) && ((ctx)->me == (ctx)) && ((ctx)->m264flag ==0x19199191) )

/*Gw_m264_init
initial mini h264 decode library
called only once at startup
*/
void Gw_m264_init()
{
	Gw_avcodec_init();
	Gw_make_yuv_table();	
}
void Gw_m264_clean()
{
	Gw_decode_free_vlc();
	Gw_destroy_avlock();
}
/*Gw_m264_new
create mini h264 decode instance
===parameter===
angle -- output bitmap rotate angle
-----------------
return value: mini h264 context if success, else NULL
*/
void *Gw_m264_new(int angle)
{
	extern AVCodec Gw_h264_decoder;
	AVCodec *codec = &Gw_h264_decoder;
	m264_t *m264 = NULL;

	m264 = malloc(sizeof(*m264));
	if(m264 == NULL)
		goto error;
	memset(m264, 0, sizeof(*m264));
	m264->m264flag = M264_FLAG;	
	m264->me = m264;
	m264->angle = angle;

	m264->c = Gw_avcodec_alloc_context();	
	m264->picture= Gw_avcodec_alloc_frame();
	if(m264->c == NULL || m264->picture == NULL)
		goto error;


	if(codec->capabilities&CODEC_CAP_TRUNCATED)
		m264->c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

	/* For some codecs, such as msmpeg4 and mpeg4, width and height
       	MUST be initialized there because this information is not
		available in the bitstream. */
	/* open it */
	  
	if (Gw_avcodec_open(m264->c, codec) < 0) {		
		goto error;
	}
	{   
	H264Context *h = (H264Context *)m264->c->priv_data;
	MpegEncContext *s = &h->s;
	s->dsp.idct_permutation_type =1;
	Gw_dsputil_init(&s->dsp, m264->c);
	}
	m264->opened = 1;


	return (void*)m264;

error:
	Gw_m264_del(m264);	
	return NULL;
}

/*Gw_m264_del
destroy mini h264 decode instance
===parameter===
ctx -- mini h264 instance, created by Gw_m264_new
*/
void Gw_m264_del(void *ctx)
{
	m264_t *m264 = (m264_t *)ctx;
	if(!IS_M264_CTX(m264))
		return;
	
	if(m264->c){
		if(m264->opened)
			Gw_avcodec_close(m264->c);
		Gw_av_free(m264->c);
	}
	if(m264->picture)
		Gw_av_free(m264->picture);
	free(m264);
}

/*Gw_m264_deocde
decode h264 frame
===parameter===
out -- output buffer
in  -- input buffer
in_size -- input buffer length
ctx -- mini h264 instance, created by Gw_m264_new
-----------------
return value: 
    > 0  , got a bitmap and return value is the length of bitmap
    ==0 ,  got no bitmap
    < 0  , decode failed
*/
int Gw_m264_deocde(uint8_t *out, const uint8_t *in, int in_size, void *ctx)
{
	int size = in_size;
	int len, got_picture,bmplen = 0;
	uint8_t *inbuf_ptr = (uint8_t*)in;
	m264_t *m264 = (m264_t *)ctx;

	if(!IS_M264_CTX(m264))
		return -1;
	
	while(size){
		len = Gw_avcodec_decode_video(m264->c, m264->picture, &got_picture,inbuf_ptr, size);
		if (len < 0) {
			//fprintf(stderr, "Error while decoding frame %d\n", frame);
			return -1;
		}
		if (got_picture) {
			//printf("saving frame %3d\n", frame);
			//fflush(stdout);

			/* the picture is allocated by the decoder. no need to free it */
			if(m264->angle)              
				bmplen = Gw_yuv420_2_bmp_180(out, 
					m264->picture->data[0], m264->picture->data[1], m264->picture->data[2], 
					m264->picture->linesize[0], m264->c->width, m264->c->height);
			else
				bmplen = Gw_yuv420_2_bmp(out, 
					m264->picture->data[0], m264->picture->data[1], m264->picture->data[2], 
					m264->picture->linesize[0], m264->c->width, m264->c->height);
		}
		size -= len;
		inbuf_ptr += len;
	}
	
	return bmplen;
}


