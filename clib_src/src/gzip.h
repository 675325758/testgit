/* gzip.h -- public header file for gzip/gunzip

   Copyright (C) 2010 Gilles Buloz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _GZIP_H_
#define _GZIP_H_
#if 0
#include <ctype.h>
#include <sys/types.h> /* for off_t */
#include <stdio.h>  /* for EOF (and fprintf if messages/debug enabled) */
#include <string.h> /* for memset, memcmp, memcpy */
#include <stdlib.h> /* for malloc */
#include <fcntl.h>  /* for open */
#include "client_lib.h"
#else
#include "cl_priv.h"
#include "cl_sys.h"
#endif

/*

#define COMPRESS_TEST_MAX 	(1024*1024)
static void _do_compress_test(char *file1, char *file2, char *file3)
{
	u_int8_t *pbuf_in = NULL;
	u_int8_t *pbuf_out = NULL;
	u_int32_t buf_in_len = 0;
	u_int32_t buf_out_len = 0;
	FILE *file_in = NULL;
	FILE *file_zip = NULL;
	FILE *file_unzip = NULL;
	int rc = 0;
	
	pbuf_in = cl_calloc(COMPRESS_TEST_MAX, 1);
	if (!pbuf_in) {
		log_err(false, "calloc failed \n");
		goto end;
	}
	pbuf_out = cl_calloc(COMPRESS_TEST_MAX, 1);
	if (!pbuf_out) {
		log_err(false, "calloc failed \n");
		goto end;
	}

	file_in = fopen(file1, "rb");
	if (!file_in) {
		log_err(false, "fopen failed file1=%s\n", file1);
		goto end;
	}
	file_zip = fopen(file2, "wb");
	if (!file_zip) {
		log_err(false, "fopen failed file2=%s\n", file2);
		goto end;
	}
	file_unzip = fopen(file3, "wb");
	if (!file_unzip) {
		log_err(false, "fopen failed file3=%s\n", file3);
		goto end;
	}

	buf_in_len = (u_int32_t)fread(pbuf_in, 1, COMPRESS_TEST_MAX, file_in);
	fclose(file_in);
	file_in = NULL;
	rc = sdk_comm_zip(pbuf_in, buf_in_len, pbuf_out, &buf_out_len);
	if (rc != 0) {
		log_debug("sdk_comm_zip failed rc=%d\n", rc);
		goto end;
	}
	//memdumpone("pbuf_out", pbuf_out, buf_out_len);
	if (buf_out_len) {
		fwrite(pbuf_out, 1, buf_out_len, file_zip);
		fclose(file_zip);
		file_zip = NULL;
	}

	memset(pbuf_in, 0, COMPRESS_TEST_MAX);
	buf_in_len = buf_out_len;
	memcpy(pbuf_in, pbuf_out, buf_in_len);
	memset(pbuf_out, 0, COMPRESS_TEST_MAX);
	buf_out_len = 0;
	
	rc = sdk_comm_unzip(pbuf_in, buf_in_len, pbuf_out, &buf_out_len);
	if (rc != 0) {
		log_debug("sdk_comm_unzip failed rc=%d\n", rc);
		goto end;
	}
	if (buf_out_len) {
		fwrite(pbuf_out, 1, buf_out_len, file_unzip);
		fclose(file_unzip);
		file_unzip = NULL;
	}

end:
	SAFE_FREE(pbuf_in);
	SAFE_FREE(pbuf_out);
	if (file_in) {
		fclose(file_in);
	}
	if (file_zip) {
		fclose(file_zip);
	}
	if (file_unzip) {
		fclose(file_unzip);
	}
}

static void do_compress_test()
{
	int i;
	char fn1[UCLA_FILE_PATH_LEN];
	char fn2[UCLA_FILE_PATH_LEN];
	char fn3[UCLA_FILE_PATH_LEN];

	log_debug("enter %s time=%u\n", __FUNCTION__, get_msec());
	sprintf(fn1, "%s/zip/%s", cl_priv->priv_dir, "json.txt");
	for(i = 0; i < 100; i++)  {
		sprintf(fn2, "%s/zip/%s%d.zip", cl_priv->priv_dir, "0zip", i);
		sprintf(fn3, "%s/zip/%s%d.txt", cl_priv->priv_dir, "0unzip", i);
		_do_compress_test(fn1 ,fn2, fn3);
	}
	log_debug("exit %s time=%u\n", __FUNCTION__, get_msec());
}
*/


//sdk需要接口
extern int sdk_comm_zip(u_int8_t *pin_data, u_int32_t in_data_len, u_int8_t *p_out_data, u_int32_t *pout_data_len);
extern int sdk_comm_unzip(u_int8_t *pin_data, u_int32_t in_data_len, u_int8_t *p_out_data, u_int32_t *pout_data_len);

#endif /* _GZIP_H_ */
