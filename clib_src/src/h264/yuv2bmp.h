#ifndef __YUV2BMP_HEADER__
#define __YUV2BMP_HEADER__ 
#ifdef WIN32
#include "stdint_win32.h"
#else
#include <stdint.h>
#endif
#pragma pack(push,1)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
typedef struct {
	uint8_t    bfType[2];
	uint32_t   bfSize;
	uint16_t    bfReserved1;
	uint16_t    bfReserved2;
	uint32_t   bfOffBits;
} bm_file_hdr_t;

typedef struct {
        uint32_t	biSize;
        int	biWidth;
        int	biHeight;
        uint16_t	biPlanes;
        uint16_t	biBitCount;
        uint32_t	biCompression;
        uint32_t	biSizeImage;
        int	biXPelsPerMeter;
        int	biYPelsPerMeter;
        uint32_t	biClrUsed;
        uint32_t	biClrImportant;
} bm_info_hdr_t;

typedef struct {
	bm_file_hdr_t file;
	bm_info_hdr_t info;
	uint8_t data[0];
} bm_t;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif 
void Gw_make_yuv_table();
int Gw_yuv420_2_bmp_180(uint8_t *out_buf, uint8_t *y_buf, uint8_t *u_buf, uint8_t *v_buf, int wrap, int width, int height);
int Gw_yuv420_2_bmp(uint8_t *out_buf, uint8_t *y_buf, uint8_t *u_buf, uint8_t *v_buf, int wrap, int width, int height);
#if 1
#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint32_t htoll(uint32_t n)
{
	return n;

}

static inline uint16_t htols(uint16_t n)
{
	return n;

}

#elif __BYTE_ORDER == __BIG_ENDIAN

static inline uint32_t htoll(uint32_t n)
{
	n = ((n >> 24) & 0xFF | (n >> 8) & 0xFF00 | (n & 0xFF00) << 8 | (n & 0xFF) << 24);
	return n;

}

static inline uint16_t htols(uint16_t n)
{
	n = (n >> 8) & 0xFF | (n & 0xFF) << 8
	return n;

}

#else
# error "Please fix <bits/endian.h>"
#endif
#endif

#ifdef __cplusplus
}
#endif 

#endif
