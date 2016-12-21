/* This header is meant to be included before mycrypt.h in projects where
 * you don't want to throw all the defines in a makefile. 
 */

#ifndef MYCRYPT_CUSTOM_H_
#define MYCRYPT_CUSTOM_H_

#ifdef CRYPT
	#error mycrypt_custom.h should be included before mycrypt.h
#endif

/* macros for various libc functions */
#define XMALLOC malloc
#define XREALLOC realloc
#define XCALLOC calloc
#define XFREE free
#define XCLOCK clock
#define XCLOCKS_PER_SEC CLOCKS_PER_SEC

/* Use small code where possible */
#undef SMALL_CODE

/* Enable self-test test vector checking */
#undef LTC_TEST

/* clean the stack of functions which put private information on stack */
//#define CLEAN_STACK

/* disable all file related functions */
//#define NO_FILE

/* various ciphers */
#undef BLOWFISH
#undef RC2
#undef RC5
#undef RC6
#undef SAFERP
#define RIJNDAEL
#undef XTEA
#undef TWOFISH
#undef TWOFISH_TABLES
//#define TWOFISH_ALL_TABLES
//#define TWOFISH_SMALL
#undef DES
#undef CAST5
#undef NOEKEON
#undef SKIPJACK

/* modes of operation */
#undef CFB
#undef OFB
#define ECB
#define CBC
#undef CTR

/* hash functions */
#undef WHIRLPOOL
#undef SHA512
#undef SHA384
#undef SHA256
#undef SHA224
#undef TIGER
#undef SHA1
#undef MD5
#undef MD4
#undef MD2
#undef RIPEMD128
#undef RIPEMD160

/* MAC functions */
#undef HMAC
#undef OMAC
#undef PMAC

/* Encrypt + Authenticate Modes */
#undef EAX_MODE
#undef OCB_MODE

/* Various tidbits of modern neatoness */
#undef BASE64
#undef YARROW
// which descriptor of AES to use? 
// 0 = rijndael_enc 1 = aes_enc, 2 = rijndael [full], 3 = aes [full]
#define YARROW_AES 0
#define SPRNG
#define RC4
#define DEVRANDOM
#define TRY_URANDOM_FIRST

/* Public Key Neatoness */
#undef MRSA
#undef RSA_TIMING                   // enable RSA side channel timing prevention 
#undef MDSA
#undef MDH
#undef MECC
#undef DH768
#undef DH1024
#undef DH1280
#undef DH1536
#undef DH1792
#undef DH2048
#undef DH2560
#undef DH3072
#undef DH4096
#undef ECC160
#undef ECC192
#undef ECC224
#undef ECC256
#undef ECC384
#undef ECC521
#undef MPI

/* PKCS #1 and 5 stuff */
#undef PKCS_1
#undef PKCS_5

#include "mycrypt.h"

#endif


