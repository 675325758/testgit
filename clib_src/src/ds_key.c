#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "cl_priv.h"
#include "cl_user.h"
#include "cl_notify.h"
#include "openssl/md5.h"
#include "client_lib.h"
#include "ds_key.h"

RSA *parse_cert(u_int8_t *cert, ds_cert_h_t *hd, RSA *rsa_ca)
{
	ds_cert_h_t *h = (ds_cert_h_t*)cert;
	unsigned char *pkey;
	unsigned char *psig;
	unsigned char md5[128], digst[128];
	int digst_len;
	RSA *rsa_public;
	MD5_CTX ctx;
	
	memcpy(hd, h, sizeof(*h));
	hd->global_id = ntohl(h->global_id);
	hd->publickey_len = ntohs(h->publickey_len);
	hd->signature_len = ntohs(h->signature_len);

	pkey = &cert[sizeof(ds_cert_h_t)];
	psig = &pkey[hd->publickey_len];
	if(rsa_ca){
		digst_len = RSA_public_decrypt(hd->signature_len, psig, digst, rsa_ca, RSA_PKCS1_PADDING);
		if(digst_len == -1)
			return NULL;
		MD5_Init(&ctx);
		MD5_Update(&ctx, cert, sizeof(ds_cert_h_t)+hd->publickey_len);
		MD5_Final(md5, &ctx);
		if(memcmp(md5, digst, 16)!=0)
			return NULL;
	}
	rsa_public =  a2i_RSAPublicKey(pkey, hd->publickey_len);
	return rsa_public;
}

void free_licence(ds_key_t *key)
{
	if(key){
		if(key->buf){
			cl_free(key->buf);
			key->buf = NULL;
		}
		if(key->rsa_private){			
			RSA_free(key->rsa_private);
			key->rsa_private = NULL;
		}
		if(key->rsa_ca_public){
			RSA_free(key->rsa_ca_public);
			key->rsa_ca_public = NULL;
		}
		if(key->rsa_public){
			RSA_free(key->rsa_public);	
			key->rsa_public = NULL;
		}
		cl_free(key);
	}
}

ds_key_t *parse_licence(const u_int8_t *license, int *len)
{
	ds_key_t* key = NULL;
	ds_license_h_t *h;
	int index, real_len;

	if(*len < sizeof(*h))
		goto errorout;
	h = (ds_license_h_t*)license;
	if(h->ver != KEY_VER1)
		goto errorout;
	
	key = cl_malloc(sizeof(*key));
	if(key == NULL)
		goto errorout;
	
	memset(key, 0, sizeof(*key));
	memcpy(&key->license_hd, h, sizeof(*h));
	key->license_hd.privatekey_len = ntohs(h->privatekey_len);
	key->license_hd.cert_len = ntohs(h->cert_len);
	key->license_hd.cacert_len = ntohs(h->cacert_len);

	real_len = (sizeof(ds_license_h_t) + key->license_hd.privatekey_len
			+key->license_hd.cert_len +  key->license_hd.cacert_len);
	
	if( real_len > *len)
		goto errorout;
	*len = real_len;

	key->buf = cl_malloc(real_len);
	if(key->buf == NULL)
		goto errorout;
	memcpy(key->buf, license, real_len);
	
	index = sizeof(ds_license_h_t);
	key->privatekey = &key->buf[index];
	index += key->license_hd.privatekey_len;
	key->cert = &key->buf[index];
	index += key->license_hd.cert_len;
	key->ca_cert = &key->buf[index];

	key->rsa_private = a2i_RSAPrivateKey(key->privatekey, key->license_hd.privatekey_len);
	if(key->rsa_private == NULL)
		goto errorout;
	key->rsa_ca_public = parse_cert(key->ca_cert, &key->ca_cert_hd, NULL);
	if(key->rsa_ca_public == NULL)
		goto errorout;
	key->rsa_public = parse_cert(key->cert, &key->cert_hd, key->rsa_ca_public);
	if(key->rsa_public == NULL)
		goto errorout;		
	
	return key;

errorout:	
	if(key)
		free_licence(key);
	return NULL;
}


int read_licence(const char * filename, u_int8_t *license)
{
	FILE *fp = NULL;
	file_hdr_t fileh;

	fp = cl_fopen(filename, "rb");
	if(fp == NULL)
		goto errorout;

	if(fread(&fileh, sizeof(file_hdr_t), 1, fp) != 1)
		goto errorout;
		
	fileh.ds_type = ntohs(fileh.ds_type);
	fileh.licence_num = ntohs(fileh.licence_num);
	fileh.is_active = ntohs(fileh.is_active);
	fileh.licence_len = ntohs(fileh.licence_len);
	if(fileh.licence_num != 1 || fileh.is_active != 0 )
		goto errorout;

	if(fileh.licence_len > MAX_LICENCE)
		goto errorout;
	
	if(fread((void*)license,fileh.licence_len, 1, fp) != 1)
		goto errorout;	

	fclose(fp);
	return (int)fileh.licence_len;
	
errorout:
	if(fp)
		fclose(fp);
	return 0;	
}


