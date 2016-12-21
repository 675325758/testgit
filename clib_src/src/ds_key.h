#ifndef __DS_KEY_HEADER__
#define __DS_KEY_HEADER__
#include "client_lib.h"
#include <openssl/rsa.h>

#define MAX_VENDER 16
#define MAX_SN	16
#define LICENCE_FILE "dslicense"
#define MAX_LICENCE (16*1024)
#define RSA_BITS 1024
#define RSA_SIZE (RSA_BITS/8)
#define SKEY_BITS 256
#define SKEY_SIZE (SKEY_BITS/8)
#define DEF_VENDOR "GALAXYWIND"

#define KEY_VER1 1

/*cert header structure*/
#pragma pack(push, 1)
typedef struct{
	u_int8_t ver;
	u_int8_t rsa_type;
	u_int8_t ds_type;
	u_int8_t reserved;
	u_int8_t mac[6];
	u_int8_t sn[MAX_SN];
	u_int8_t vendor[MAX_VENDER];
	u_int32_t global_id;
	u_int16_t publickey_len;
	u_int16_t signature_len;
	/*the coming data is public key + signature*/
}ds_cert_h_t;

/*license file header structure*/
typedef struct{
	u_int8_t ver;
	u_int8_t rsa_type;
	u_int16_t privatekey_len;
	u_int16_t cert_len;
	u_int16_t cacert_len;
	/*the coming data is private key + Cert + CaCert*/
}ds_license_h_t;

typedef struct{
	ds_license_h_t license_hd;
	ds_cert_h_t cert_hd;
	ds_cert_h_t ca_cert_hd;
	u_int8_t *privatekey;
	u_int8_t *cert;
	u_int8_t *ca_cert;
	u_int8_t *buf;

	RSA *rsa_private;
	RSA *rsa_public;
	RSA *rsa_ca_public;
}ds_key_t;

/*
对于DS007设备的licence文件是通过激活程序将证书发送到设备端，
为了便于激活程序的批量操作，licence文件可以存放多个证书，格式如下：

名称			长度			说明
DS_Type			2bytes			设备型号（目前只有DS007一种型号：TP_IPLUS）
Licence_Num		2bytes			Licence数量
Is_Active			2bytes			Licence是否已被用于激活（0：未激活，1：已激活）
Licence_Len		2bytes			Licence长度

给小区的授权文件由DS007_make_licences.exe程序生成，
每个授权文件只包含一个licence。
*/
typedef struct{
	u_int16_t ds_type;
	u_int16_t licence_num;
	u_int16_t is_active;
	u_int16_t licence_len;
	u_int8_t licence_data[0];	
}file_hdr_t;
#pragma pack(pop)
#ifdef __cplusplus
extern "C" {
#endif 

RSA *parse_cert(u_int8_t *cert, ds_cert_h_t *hd, RSA *rsa_ca);
void free_licence(ds_key_t *key);
ds_key_t *parse_licence(const u_int8_t *license, int *len);
int read_licence(const char * filename, u_int8_t *license);

#ifdef __cplusplus
}
#endif 


#endif
