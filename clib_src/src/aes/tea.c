#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbool.h>
//#include <stdint.h>
#include "client_lib.h"
#include "tea.h"
//#include "dbg_io.h"
//#include "misc.h"

#define TEA_DELTA 0x9E3779B9
#define TEA_SUM 0xC6EF3720
#define TEA_ROUND 32

static unsigned int u8_to_u32(unsigned char _u8[4])
{
	return ((unsigned int)_u8[0]<<24)|((unsigned int)_u8[1]<<16)|((unsigned int)_u8[2]<<8)|(unsigned int)_u8[3];
}

static void u32_to_u8(unsigned char _u8[4], unsigned int _u32) 
{
	(_u8)[0]= ((_u32)>>24)&0xFF;\
	(_u8)[1]= ((_u32)>>16)&0xFF;\
	(_u8)[2]= ((_u32)>>8)&0xFF;\
	(_u8)[3]= (_u32)&0xFF;\
}

#define bytes_xor8(a,b) \
do{\
	(a)[0]=(a)[0]^(b)[1];\
	(a)[1]=(a)[1]^(b)[3];\
	(a)[2]=(a)[2]^(b)[5];\
	(a)[3]=(a)[3]^(b)[7];\
	(a)[4]=(a)[4]^(b)[0];\
	(a)[5]=(a)[5]^(b)[2];\
	(a)[6]=(a)[6]^(b)[4];\
	(a)[7]=(a)[7]^(b)[6];\
}while(0)
void tea_key(tea_key_t *k, unsigned char *key)
{
	k->k[0]=u8_to_u32(key);
	k->k[1]=u8_to_u32(key+4);
	k->k[2]=u8_to_u32(key+8);
	k->k[3]=u8_to_u32(key+12);
}

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

void xxtea_enc(u_int32_t *v, int n, u_int32_t const key[4])
{
	u_int32_t y, z, sum;
	unsigned p, rounds, e;
	if (n <= 0) 
		return;
	
	rounds = 6 + 52/n;
	sum = 0;
	z = v[n-1];
	do {
		sum += DELTA;
		e = (sum >> 2) & 3;
		for (p=0; p<(u_int32_t)n-1; p++) {
			y = v[p+1]; 
			z = v[p] += MX;
		}
		y = v[0];
		z = v[n-1] += MX;
	} while (--rounds);	
}

void tea_enc(unsigned char *out, unsigned char *data, tea_key_t *k)
{
	u_int32_t x[2];
	x[0]=u8_to_u32(data);
	x[1]=u8_to_u32(data+4);
	xxtea_enc(x, 2, k->k);
	u32_to_u8(out, x[0]);
	u32_to_u8(&out[4], x[1]);
}

void tea_enc_block(unsigned char *data, int len, unsigned char *passwd)
{
	int i = 0, n = len >>2;
	u_int32_t *v = (u_int32_t *)data;
	u_int32_t tmp;
	tea_key_t key;
	
	tea_key(&key, passwd);
	for(i = 0; i < n; i++){
		v[i] = u8_to_u32(&data[i*4]);
	}
	xxtea_enc(v, i, key.k);	
	for(i = 0; i < n; i++){
		tmp = v[i];
		u32_to_u8(&data[i*4], tmp);
	}
}

void xxtea_dec(u_int32_t *v, int n, u_int32_t const key[4])
{
	u_int32_t y, z, sum;
	unsigned p, rounds, e;
	if (n <= 0) 
		return;
	
	rounds = 6 + 52/n;
	sum = rounds*DELTA;
	y = v[0];
	do {
		e = (sum >> 2) & 3;
		for (p=n-1; p>0; p--) {
			z = v[p-1];
			y = v[p] -= MX;
		}
		z = v[n-1];
		y = v[0] -= MX;
		sum -= DELTA;
	} while (--rounds);
}
void tea_dec(unsigned char *out, unsigned char *data, tea_key_t *k)
{
	u_int32_t x[2];
	x[0]=u8_to_u32(data);
	x[1]=u8_to_u32(data+4);
	xxtea_dec(x, 2, k->k);
	u32_to_u8(out, x[0]);
	u32_to_u8(&out[4], x[1]);
}

void tea_dec_block(unsigned char *data, int len, unsigned char *passwd)
{
	int i = 0, n = len >> 2;
	u_int32_t *v = (u_int32_t *)data;
	u_int32_t tmp;
	tea_key_t key;
	
	tea_key(&key, passwd);
	for(i = 0; i < n; i++){
		v[i] = u8_to_u32(&data[i*4]);
	}
	xxtea_dec(v, i, key.k);	
	for(i = 0; i < n; i++){
		tmp = v[i];
		u32_to_u8(&data[i*4], tmp);
	}
}
#if 0
void tea_test(void)
{
	unsigned char plain[] = {0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88};
	unsigned char cipher[8], out[8];
	unsigned char key[]={0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88};
	tea_key_t k;

	tea_key(&k, key);	
	tea_enc(cipher, plain, &k);
	rf_printf("cipher: %02x%02x%02x%02x%02x%02x%02x%02x\n", cipher[0], cipher[1], cipher[2], cipher[3], cipher[4], cipher[5], cipher[6], cipher[7]) ;
	tea_dec(out, cipher, &k);
	rf_printf("plain : %02x%02x%02x%02x%02x%02x%02x%02x\n", out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]) ;
}
#endif
#if 0
int main(int argc, char *argv[])
{
	tea_test(); 
	return 0;
}
#endif
