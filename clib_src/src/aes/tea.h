#ifndef __RF_TEA_ALGO__
#define __RF_TEA_ALGO__

#define XXTEA_BLOCK_SIZE 8

typedef struct{
	unsigned int k[4];
}tea_key_t;

/*
setup tea key
parameters description:
k: tea internal encrypt/decrypt key
key: 16 bytes key
*/
void tea_key(tea_key_t *k, unsigned char *key);

/*
rf protocol tea key setup
parameters description:
k: tea internal encrypt/decrypt key
gw_rand: rf gateway random, host byte order
dev_rand: rf device random, host byte order
psk: 8 bytes preshare key
*/
void tea_setup_rf(tea_key_t *k, unsigned int gw_rand, unsigned int dev_rand, unsigned char *psk);

/*
rf auth protocol digest calc
parameters description:
gw_rand: rf gateway random, host byte order
dev_rand: rf device random, host byte order
psk: 8 bytes preshare key
k:tea session key
digest: 16 bytes auth digest
*/
void tea_digest(unsigned char *digest, tea_key_t *k, unsigned int gw_rand, unsigned int dev_rand, unsigned char *psk);

/*
parameters description:
out: 8 bytes cipher
data: 8 bytes plaintext
key: tea key
*/
void tea_enc(unsigned char *out, unsigned char *data, tea_key_t *k);
/*
parameters description:
out: 8 bytes plaintext to be output
data: 8 bytes cipher to be decrypted
key: tea key
*/
void tea_dec(unsigned char *out, unsigned char *data, tea_key_t *k);
void tea_test(void);
void tea_enc_block(unsigned char *data, int len, unsigned char *passwd);
void tea_dec_block(unsigned char *data, int len, unsigned char *passwd);
#endif

