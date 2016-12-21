/* crypto/rand/md_rand.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2001 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#ifdef MD_RAND_DEBUG
# ifndef NDEBUG
#   define NDEBUG
# endif
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "e_os.h"

#include <openssl/rand.h>
#include "rand_lcl.h"

#include <openssl/crypto.h>
#include <openssl/err.h>

#ifdef BN_DEBUG
# define PREDICT
#endif

#define	MD5Init		MD5Init_Rsa
#define	MD5Update	MD5Update_Rsa
#define	MD5Final	MD5Final_Rsa

/* #define PREDICT	1 */

#define STATE_SIZE	1023
static int state_num=0,state_index=0;
static unsigned char state[STATE_SIZE+MD_DIGEST_LENGTH];
static unsigned char md[MD_DIGEST_LENGTH];
static long md_count[2]={0,0};
static double entropy=0;
static int initialized=0;

static unsigned int crypto_lock_rand = 0; /* may be set only when a thread
                                           * holds CRYPTO_LOCK_RAND
                                           * (to prevent double locking) */
/* access to lockin_thread is synchronized by CRYPTO_LOCK_RAND2 */
//static CRYPTO_THREADID locking_threadid; /* valid iff crypto_lock_rand is set */


#ifdef PREDICT
int rand_predictable=0;
#endif

/*
	Added by Richard: copy MD5 implementation from Linux kernel.
*/
#if 0
struct MD5Context {
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
};
#else
#define UINT4	unsigned long
#define UINT2	unsigned short

typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;
#endif


#if 0
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	(w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x)

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void
MD5Transform(unsigned int buf[4], unsigned int const in[16])
{
	unsigned int a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

/*
void *memmove(void *dest, const void *src, size_t count)
{
	if (dest < src) {
		return memcpy(dest, src, count);
	} else {
		char *p = dest + count;
		const char *s = src + count;
		while (count--)
			*--p = *--s;
	}
	return dest;
}
*/
/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Init(struct MD5Context *ctx)
{
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;

	ctx->bits[0] = 0;
	ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len)
{
	unsigned int t;

	/* Update bitcount */

	t = ctx->bits[0];
	if ((ctx->bits[0] = t + ((unsigned int) len << 3)) < t)
		ctx->bits[1]++;	/* Carry from low to high */
	ctx->bits[1] += len >> 29;

	t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

	/* Handle any leading odd-sized chunks */

	if (t) {
		unsigned char *p = (unsigned char *) ctx->in + t;

		t = 64 - t;
		if (len < t) {
			memmove(p, buf, len);
			return;
		}
		memmove(p, buf, t);
		//byteReverse(ctx->in, 16);
		MD5Transform(ctx->buf, (unsigned int *) ctx->in);
		buf += t;
		len -= t;
	}
	/* Process data in 64-byte chunks */

	while (len >= 64) {
		memmove(ctx->in, buf, 64);
		//byteReverse(ctx->in, 16);
		MD5Transform(ctx->buf, (unsigned int *) ctx->in);
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */

	memmove(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
	unsigned int count;
	unsigned char *p;

	/* Compute number of bytes mod 64 */
	count = (ctx->bits[0] >> 3) & 0x3F;

	/* Set the first char of padding to 0x80.  This is safe since there is
	   always at least one byte free */
	p = ctx->in + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		memset(p, 0, count);
	//	byteReverse(ctx->in, 16);
		MD5Transform(ctx->buf, (unsigned int *) ctx->in);

		/* Now fill the next block with 56 bytes */
		memset(ctx->in, 0, 56);
	} else {
		/* Pad block to 56 bytes */
		memset(p, 0, count - 8);
	}
//	byteReverse(ctx->in, 14);

	/* Append length in bits and transform */
	((unsigned int *) ctx->in)[14] = ctx->bits[0];
	((unsigned int *) ctx->in)[15] = ctx->bits[1];

	MD5Transform(ctx->buf, (unsigned int *) ctx->in);
//	byteReverse((unsigned char *) ctx->buf, 4);
	memmove(digest, ctx->buf, 16);
	memset(ctx, 0, sizeof(*ctx));	/* In case it's sensitive */
}

#else
/* Constants for MD5Transform routine.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform(UINT4 [4], unsigned char [64]);
static void Encode(unsigned char *, UINT4 *, unsigned int);
static void Decode(UINT4 *, unsigned char *, unsigned int);
//static void MD5_memcpy(POINTER, POINTER, unsigned int);
//static void MD5_memset(POINTER, int, unsigned int);

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init_Rsa(MD5_CTX *context)
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update_Rsa(MD5_CTX *context,unsigned char *input,unsigned int inputLen)
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 memcpy(&context->buffer[index], input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  memcpy(&context->buffer[index], &input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final_Rsa (unsigned char digest[16],MD5_CTX *context)
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);
  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  memset(context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (state, block)
UINT4 state[4];
unsigned char block[64];
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
*/
  memset(x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (output, input, len)
unsigned char *output;
UINT4 *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (output, input, len)
UINT4 *output;
unsigned char *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

static void MD5(unsigned char digest[16],unsigned char *input,unsigned int inputlen)
{
	MD5_CTX		context;

	MD5Init(&context);
	MD5Update(&context,input,inputlen);
	MD5Final (digest,&context);
}

#endif


const char RAND_version[]="RAND" OPENSSL_VERSION_PTEXT;

static void ssleay_rand_cleanup(void);
static void ssleay_rand_seed(const void *buf, int num);
static void ssleay_rand_add(const void *buf, int num, double add_entropy);
static int ssleay_rand_bytes(unsigned char *buf, int num);
static int ssleay_rand_pseudo_bytes(unsigned char *buf, int num);
static int ssleay_rand_status(void);

RAND_METHOD rand_ssleay_meth={
	ssleay_rand_seed,
	ssleay_rand_bytes,
	ssleay_rand_cleanup,
	ssleay_rand_add,
	ssleay_rand_pseudo_bytes,
	ssleay_rand_status
	}; 

RAND_METHOD *RAND_SSLeay(void)
	{
	return(&rand_ssleay_meth);
	}

static void ssleay_rand_cleanup(void)
	{
	OPENSSL_cleanse(state,sizeof(state));
	state_num=0;
	state_index=0;
	OPENSSL_cleanse(md,MD_DIGEST_LENGTH);
	md_count[0]=0;
	md_count[1]=0;
	entropy=0;
	initialized=0;
	}

static void ssleay_rand_add(const void *buf, int num, double add)
	{
	int i,j,k,st_idx;
	long md_c[2];
	unsigned char local_md[MD_DIGEST_LENGTH];
//	EVP_MD_CTX m;
	int do_not_lock;
//	struct MD5Context md5ctx;
    MD5_CTX md5ctx;
	/* Modified by Richard 20111024: Removing crypto_lock, change EVP_MD_xxx ro MD5xxx */

	/*
	 * (Based on the rand(3) manpage)
	 *
	 * The input is chopped up into units of 20 bytes (or less for
	 * the last block).  Each of these blocks is run through the hash
	 * function as follows:  The data passed to the hash function
	 * is the current 'md', the same number of bytes from the 'state'
	 * (the location determined by in incremented looping index) as
	 * the current 'block', the new key data 'block', and 'count'
	 * (which is incremented after each use).
	 * The result of this is kept in 'md' and also xored into the
	 * 'state' at the same locations that were used as input into the
         * hash function.
	 */

	/* check if we already have the lock */

#if 0
	if (crypto_lock_rand)
		{
		CRYPTO_THREADID cur;
		CRYPTO_THREADID_current(&cur);
		CRYPTO_r_lock(CRYPTO_LOCK_RAND2);
		do_not_lock = !CRYPTO_THREADID_cmp(&locking_threadid, &cur);
		CRYPTO_r_unlock(CRYPTO_LOCK_RAND2);
		}
	else
#endif
		do_not_lock = 0;

//	if (!do_not_lock) CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	st_idx=state_index;

	/* use our own copies of the counters so that even
	 * if a concurrent thread seeds with exactly the
	 * same data and uses the same subarray there's _some_
	 * difference */
	md_c[0] = md_count[0];
	md_c[1] = md_count[1];

	memcpy(local_md, md, sizeof md);

	/* state_index <= state_num <= STATE_SIZE */
	state_index += num;
	if (state_index >= STATE_SIZE)
		{
		state_index%=STATE_SIZE;
		state_num=STATE_SIZE;
		}
	else if (state_num < STATE_SIZE)	
		{
		if (state_index > state_num)
			state_num=state_index;
		}
	/* state_index <= state_num <= STATE_SIZE */

	/* state[st_idx], ..., state[(st_idx + num - 1) % STATE_SIZE]
	 * are what we will use now, but other threads may use them
	 * as well */

	md_count[1] += (num / MD_DIGEST_LENGTH) + (num % MD_DIGEST_LENGTH > 0);

//	if (!do_not_lock) CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	MD5Init(&md5ctx);
	for (i=0; i<num; i+=MD_DIGEST_LENGTH)
		{
		j=(num-i);
		j=(j > MD_DIGEST_LENGTH)?MD_DIGEST_LENGTH:j;

//		MD_Update(&m,local_md,MD_DIGEST_LENGTH);
		MD5Update(&md5ctx, local_md, MD_DIGEST_LENGTH);
		k=(st_idx+j)-STATE_SIZE;
		if (k > 0)
			{
			//MD_Update(&m,&(state[st_idx]),j-k);
			MD5Update(&md5ctx, &(state[st_idx]),j-k);
			//MD_Update(&m,&(state[0]),k);
			MD5Update(&md5ctx, &(state[0]),k);
			}
		else
			//MD_Update(&m,&(state[st_idx]),j);
			MD5Update(&md5ctx, &(state[st_idx]),j);

		/* DO NOT REMOVE THE FOLLOWING CALL TO MD_Update()! */
		//MD_Update(&m,buf,j);
		MD5Update(&md5ctx, (unsigned char *)buf, j);
		/* We know that line may cause programs such as
		   purify and valgrind to complain about use of
		   uninitialized data.  The problem is not, it's
		   with the caller.  Removing that line will make
		   sure you get really bad randomness and thereby
		   other problems such as very insecure keys. */

		//MD_Update(&m,(unsigned char *)&(md_c[0]),sizeof(md_c));
		MD5Update(&md5ctx, (unsigned char *)&(md_c[0]),sizeof(md_c));
		//MD_Final(&m,local_md);
		MD5Final(local_md, &md5ctx);
		md_c[1]++;

		buf=(const char *)buf + j;

		for (k=0; k<j; k++)
			{
			/* Parallel threads may interfere with this,
			 * but always each byte of the new state is
			 * the XOR of some previous value of its
			 * and local_md (itermediate values may be lost).
			 * Alway using locking could hurt performance more
			 * than necessary given that conflicts occur only
			 * when the total seeding is longer than the random
			 * state. */
			state[st_idx++]^=local_md[k];
			if (st_idx >= STATE_SIZE)
				st_idx=0;
			}
		}
//	EVP_MD_CTX_cleanup(&m);

//	if (!do_not_lock) CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	/* Don't just copy back local_md into md -- this could mean that
	 * other thread's seeding remains without effect (except for
	 * the incremented counter).  By XORing it we keep at least as
	 * much entropy as fits into md. */
	for (k = 0; k < (int)sizeof(md); k++)
		{
		md[k] ^= local_md[k];
		}
	if (entropy < ENTROPY_NEEDED) /* stop counting when we have enough */
	    entropy += add;
//	if (!do_not_lock) CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
	
#if !defined(OPENSSL_THREADS) && !defined(OPENSSL_SYS_WIN32)
	assert(md_c[1] == md_count[1]);
#endif

	}

static void ssleay_rand_seed(const void *buf, int num)
	{
	ssleay_rand_add(buf, num, (double)num);
	}

static int ssleay_rand_bytes(unsigned char *buf, int num)
	{
	static volatile int stirred_pool = 0;
	int i,j,k,st_num,st_idx;
	int num_ceil;
	int ok;
	long md_c[2];
	unsigned char local_md[MD_DIGEST_LENGTH];
//	struct MD5Context m;
    MD5_CTX m;
	int do_stir_pool = 0;

	/* Modified by Richard 20111024: Removing crypto_lock, change EVP_MD_xxx ro MD5xxx */

#ifdef PREDICT
	if (rand_predictable)
		{
		static unsigned char val=0;

		for (i=0; i<num; i++)
			buf[i]=val++;
		return(1);
		}
#endif

	if (num <= 0)
		return 1;

	MD5Init(&m);
	/* round upwards to multiple of MD_DIGEST_LENGTH/2 */
	num_ceil = (1 + (num-1)/(MD_DIGEST_LENGTH/2)) * (MD_DIGEST_LENGTH/2);

	/*
	 * (Based on the rand(3) manpage:)
	 *
	 * For each group of 10 bytes (or less), we do the following:
	 *
	 * Input into the hash function the local 'md' (which is initialized from
	 * the global 'md' before any bytes are generated), the bytes that are to
	 * be overwritten by the random bytes, and bytes from the 'state'
	 * (incrementing looping index). From this digest output (which is kept
	 * in 'md'), the top (up to) 10 bytes are returned to the caller and the
	 * bottom 10 bytes are xored into the 'state'.
	 * 
	 * Finally, after we have finished 'num' random bytes for the
	 * caller, 'count' (which is incremented) and the local and global 'md'
	 * are fed into the hash function and the results are kept in the
	 * global 'md'.
	 */

//	CRYPTO_w_lock(CRYPTO_LOCK_RAND);

	/* prevent ssleay_rand_bytes() from trying to obtain the lock again */
//	CRYPTO_w_lock(CRYPTO_LOCK_RAND2);
//	CRYPTO_THREADID_current(&locking_threadid);
//	CRYPTO_w_unlock(CRYPTO_LOCK_RAND2);
	crypto_lock_rand = 1;

	if (!initialized)
		{
		//RAND_poll();
		initialized = 1;
		}
	
	if (!stirred_pool)
		do_stir_pool = 1;
	
	ok = (entropy >= ENTROPY_NEEDED);
	if (!ok)
		{
		/* If the PRNG state is not yet unpredictable, then seeing
		 * the PRNG output may help attackers to determine the new
		 * state; thus we have to decrease the entropy estimate.
		 * Once we've had enough initial seeding we don't bother to
		 * adjust the entropy count, though, because we're not ambitious
		 * to provide *information-theoretic* randomness.
		 *
		 * NOTE: This approach fails if the program forks before
		 * we have enough entropy. Entropy should be collected
		 * in a separate input pool and be transferred to the
		 * output pool only when the entropy limit has been reached.
		 */
		entropy -= num;
		if (entropy < 0)
			entropy = 0;
		}

	if (do_stir_pool)
		{
		/* In the output function only half of 'md' remains secret,
		 * so we better make sure that the required entropy gets
		 * 'evenly distributed' through 'state', our randomness pool.
		 * The input function (ssleay_rand_add) chains all of 'md',
		 * which makes it more suitable for this purpose.
		 */

		int n = STATE_SIZE; /* so that the complete pool gets accessed */
		while (n > 0)
			{
#define DUMMY_SEED "...................." /* at least MD_DIGEST_LENGTH */
			/* Note that the seed does not matter, it's just that
			 * ssleay_rand_add expects to have something to hash. */
			ssleay_rand_add(DUMMY_SEED, MD_DIGEST_LENGTH, 0.0);
			n -= MD_DIGEST_LENGTH;
			}
		if (ok)
			stirred_pool = 1;
		}

	st_idx=state_index;
	st_num=state_num;
	md_c[0] = md_count[0];
	md_c[1] = md_count[1];
	memcpy(local_md, md, sizeof md);

	state_index+=num_ceil;
	if (state_index > state_num)
		state_index %= state_num;

	/* state[st_idx], ..., state[(st_idx + num_ceil - 1) % st_num]
	 * are now ours (but other threads may use them too) */

	md_count[0] += 1;

	/* before unlocking, we must clear 'crypto_lock_rand' */
	crypto_lock_rand = 0;
//	CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	while (num > 0)
		{
		/* num_ceil -= MD_DIGEST_LENGTH/2 */
		j=(num >= MD_DIGEST_LENGTH/2)?MD_DIGEST_LENGTH/2:num;
		num-=j;
		MD5Init(&m);
		MD5Update(&m,local_md,MD_DIGEST_LENGTH);
		MD5Update(&m,(unsigned char *)&(md_c[0]),sizeof(md_c));

#ifndef PURIFY /* purify complains */
		/* The following line uses the supplied buffer as a small
		 * source of entropy: since this buffer is often uninitialised
		 * it may cause programs such as purify or valgrind to
		 * complain. So for those builds it is not used: the removal
		 * of such a small source of entropy has negligible impact on
		 * security.
		 */
		MD5Update(&m,buf,j);
#endif

		k=(st_idx+MD_DIGEST_LENGTH/2)-st_num;
		if (k > 0)
			{
			MD5Update(&m,&(state[st_idx]),MD_DIGEST_LENGTH/2-k);
			MD5Update(&m,&(state[0]),k);
			}
		else
			MD5Update(&m,&(state[st_idx]),MD_DIGEST_LENGTH/2);
		MD5Final(local_md, &m);

		for (i=0; i<MD_DIGEST_LENGTH/2; i++)
			{
			state[st_idx++]^=local_md[i]; /* may compete with other threads */
			if (st_idx >= st_num)
				st_idx=0;
			if (i < j)
				*(buf++)=local_md[i+MD_DIGEST_LENGTH/2];
			}
		}

	MD5Init(&m);
	MD5Update(&m,(unsigned char *)&(md_c[0]),sizeof(md_c));
	MD5Update(&m,local_md,MD_DIGEST_LENGTH);
	MD5Update(&m,md,MD_DIGEST_LENGTH);
	MD5Final(md, &m);

	return(1);

	}

/* pseudo-random bytes that are guaranteed to be unique but not
   unpredictable */
static int ssleay_rand_pseudo_bytes(unsigned char *buf, int num) 
	{
	int ret;
	unsigned long err;

	ret = RAND_bytes(buf, num);
	if (ret == 0)
		{
		err = ERR_peek_error();
		if (ERR_GET_LIB(err) == ERR_LIB_RAND &&
		    ERR_GET_REASON(err) == RAND_R_PRNG_NOT_SEEDED)
			ERR_clear_error();
		}
	return (ret);
	}

static int ssleay_rand_status(void)
	{
	int ret;
#if 0
	CRYPTO_THREADID cur;
	int do_not_lock;

	/* Modified by Richard 20111024   */
	CRYPTO_THREADID_current(&cur);
	/* check if we already have the lock
	 * (could happen if a RAND_poll() implementation calls RAND_status()) */
	if (crypto_lock_rand)
		{
		CRYPTO_r_lock(CRYPTO_LOCK_RAND2);
		do_not_lock = !CRYPTO_THREADID_cmp(&locking_threadid, &cur);
		CRYPTO_r_unlock(CRYPTO_LOCK_RAND2);
		}
	else
		do_not_lock = 0;
	
	if (!do_not_lock)
		{
		CRYPTO_w_lock(CRYPTO_LOCK_RAND);
		
		/* prevent ssleay_rand_bytes() from trying to obtain the lock again */
		CRYPTO_w_lock(CRYPTO_LOCK_RAND2);
		CRYPTO_THREADID_cpy(&locking_threadid, &cur);
		CRYPTO_w_unlock(CRYPTO_LOCK_RAND2);
		crypto_lock_rand = 1;
		}
	
	if (!initialized)
		{
		RAND_poll();
		initialized = 1;
		}

	ret = entropy >= ENTROPY_NEEDED;

	if (!do_not_lock)
		{
		/* before unlocking, we must clear 'crypto_lock_rand' */
		crypto_lock_rand = 0;
		
		CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
		}
#endif	
	ret = 1;
	return ret;
	}
