#ifndef	__MD5_H__
#define	__MD5_H__


#ifdef __cplusplus
extern "C" {
#endif 

typedef struct {
  u_int32_t state[4];                                   /* state (ABCD) */
  u_int32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

extern void MD5Init_Sdk(MD5_CTX *ctx);
extern void MD5Update_Sdk(MD5_CTX *ctx, unsigned char *buf, unsigned len);
extern void MD5Final_Sdk(unsigned char digest[16], MD5_CTX *ctx);


#define	MD5Init	MD5Init_Sdk
#define	MD5Update	MD5Update_Sdk
#define	MD5Final	MD5Final_Sdk


#ifdef __cplusplus
}
#endif

#endif

