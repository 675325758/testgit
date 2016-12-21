#ifndef	__AES_H__
#define	__AES_H__

#include "mycrypt.h"


#ifdef __cplusplus
extern "C" {
#endif

#define	AES128_EKY_LEN	16
extern void enc_block(unsigned char *data, int len, unsigned char *passwd);
extern void dec_block(unsigned char *data, int len, unsigned char *passwd);
extern void dec_block2(unsigned char *dst, unsigned char *src, int len, unsigned char *passwd);

#ifdef __cplusplus
}
#endif


#endif

