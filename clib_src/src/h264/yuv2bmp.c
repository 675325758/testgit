//#include "mp4_fmt.h"
#include "common.h"
#include "yuv2bmp.h"
#ifdef WIN32
#include <windows.h>
#else
#define BI_RGB 0
#endif
#include <string.h>

static int yuv_table_init = 0;
int Gw_U[256], Gw_V[256], Gw_Y1[256], Gw_Y2[256];

void Gw_make_yuv_table()
{
	int i;

	for (i = 0; i < 256; i++)
	{
		Gw_V[i] = 15938*i - 2221300;
		Gw_U[i] = 20238*i - 2771300;
		Gw_Y1[i] = 11644*i;
		Gw_Y2[i] = 19837*i - 311710;
	}
	yuv_table_init = 1;
}

void Gw_init_pic_hdr(bm_t *pic, int fsize, int width, int height, int have_aphal)
{
	memset(pic, 0, sizeof(bm_t));
	fsize += sizeof(bm_t);

	pic->file.bfType[0] = 'B';
	pic->file.bfType[1] = 'M';
	pic->file.bfSize = htoll(fsize);
	pic->file.bfReserved1 = 0;
	pic->file.bfReserved2 = 0;
	pic->file.bfOffBits = htoll(sizeof(bm_t));

	pic->info.biSize = htoll(sizeof(bm_info_hdr_t));
	pic->info.biWidth = htoll(width);
	pic->info.biHeight = htoll(height);
	pic->info.biPlanes = htols(1);
	pic->info.biBitCount = htols(24 + have_aphal*8);
	pic->info.biCompression = htoll(BI_RGB);
	pic->info.biSizeImage = htoll(fsize - sizeof(bm_t));
	pic->info.biXPelsPerMeter = htoll(0);
	pic->info.biYPelsPerMeter = htoll(0);
	pic->info.biClrUsed = htoll(0);
	pic->info.biClrImportant = htoll(0);
}


int Gw_yuv420_2_bmp_180(uint8_t *out_buf, uint8_t *y_buf, uint8_t *u_buf, uint8_t *v_buf, int wrap, int width, int height)
{
	int fsize, bpp;
	int h, w;
	uint8_t *y, *u, *v;
	uint8_t *r, *g, *b;
	bm_t *pic =(bm_t*)out_buf;
	
	if ( ! yuv_table_init ) {
		Gw_make_yuv_table();
		yuv_table_init = 1;
	}

	bpp = 3;

	fsize = sizeof(bm_t) + width*height*bpp;
	Gw_init_pic_hdr(pic, fsize, width, height, 0);

	// 开始逐点转化
	b = &pic->data[0] + width*(height - 1)*bpp;
	g = b + 1;
	r = b + 2;

	for (h = 0; h < height; h++) {
		y = y_buf + h*wrap;
		u = u_buf + (h>>1)*wrap;
		v = v_buf + (h>>1)*wrap;

		for (w = 0; w < width; w += 2) {
			*r = max(0, min(255, (Gw_V[*v] + Gw_Y1[*y])/10000));
			*b = max(0, min(255, (Gw_U[*u] + Gw_Y1[*y])/10000));
			*g = max(0, min(255, (Gw_Y2[*y] - 5094*(*r) - 1942*(*b))/10000));

			r += bpp;
			g += bpp;
			b += bpp;
			y++;

			*r = max(0, min(255, (Gw_V[*v] + Gw_Y1[*y])/10000));
			*b = max(0, min(255, (Gw_U[*u] + Gw_Y1[*y])/10000));
			*g = max(0, min(255, (Gw_Y2[*y] - 5094*(*r) - 1942*(*b))/10000));

			r += bpp;
			g += bpp;
			b += bpp;
			y++;
			u++;
			v++;
		}

		b -= width*2*bpp;
		g = b + 1;
		r = b + 2;
	}

	return fsize;
}

int Gw_yuv420_2_bmp(uint8_t *out_buf, uint8_t *y_buf, uint8_t *u_buf, uint8_t *v_buf, int wrap, int width, int height)
{
	int fsize, bpp;
	int h, w;
	uint8_t *y, *u, *v;
	uint8_t *r, *g, *b;
	bm_t *pic =(bm_t*)out_buf;
	
	if ( ! yuv_table_init ) {
		Gw_make_yuv_table();
		yuv_table_init = 1;
	}

	bpp = 3;

	fsize = sizeof(bm_t) + width*height*bpp;
	Gw_init_pic_hdr(pic, fsize, width, height, 0);

	// 开始逐点转化
	b = &pic->data[0] + (width - 1)*bpp;
	g = b + 1;
	r = b + 2;

	for (h = 0; h < height; h++) {
		y = y_buf + h*wrap;
		u = u_buf + (h>>1)*(wrap);
		v = v_buf + (h>>1)*(wrap);

		for (w = 0; w < width; w += 2) {
			*r = max(0, min(255, (Gw_V[*v] + Gw_Y1[*y])/10000));
			*b = max(0, min(255, (Gw_U[*u] + Gw_Y1[*y])/10000));
			*g = max(0, min(255, (Gw_Y2[*y] - 5094*(*r) - 1942*(*b))/10000));

			r -= bpp;
			g -= bpp;
			b -= bpp;
			y++;

			*r = max(0, min(255, (Gw_V[*v] + Gw_Y1[*y])/10000));
			*b = max(0, min(255, (Gw_U[*u] + Gw_Y1[*y])/10000));
			*g = max(0, min(255, (Gw_Y2[*y] - 5094*(*r) - 1942*(*b))/10000));

			r -= bpp;
			g -= bpp;
			b -= bpp;
			y++;
			u++;
			v++;
		}

		b += width*2*bpp;
		g = b + 1;
		r = b + 2;
	}

	return fsize;
}

