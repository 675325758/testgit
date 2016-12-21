#include "client_lib.h"
#include "cl_priv.h"

#include "buffer.h"


buff_t *bf_alloc(int size)
{
	buff_t *bf;

	bf = cl_calloc(sizeof(buff_t), 1);
	bf_init(bf, size);

	return bf;
}

void bf_free(buff_t *bf)
{
	if (bf == NULL)
		return;
	
	SAFE_FREE(bf->buf);
	cl_free(bf);
}

bool bf_expand(buff_t *bf, int size)
{
	char *p;
	
	if (size > MAX_VIDEO_TCP_BUFF_SIZE)
		return false;

	if ((p = cl_realloc(bf->buf, size)) == NULL)
		return false;

	bf->buf = p;
	bf->buf_size = size;

	return true;
}

bool bf_init(buff_t *bf, int size)
{
	bf->read_pos = 0;
	bf->write_pos = 0;

	if (size <= bf->buf_size)
		return true;
	
	SAFE_FREE(bf->buf);
	bf->buf_size = 0;

	bf->buf = cl_malloc(size);
	if (bf->buf == NULL)
		return false;

	bf->buf_size = size;

	return true;
}

void bf_reset(buff_t *bf)
{
	bf->read_pos = bf->write_pos = bf->pkt_size = 0;
}

int bf_get_write_free(buff_t *bf)
{
	return bf->buf_size - bf->write_pos;
}

char *bf_get_write_pos(buff_t *bf)
{
	return bf->buf + bf->write_pos;
}

int bf_get_read_len(buff_t *bf)
{
	return bf->write_pos - bf->read_pos;
}

char *bf_get_read_pos(buff_t *bf)
{
	return bf->buf + bf->read_pos;
}

void bf_check_read_write_pos(buff_t *bf)
{
	if (bf->read_pos == bf->write_pos)
		bf->read_pos = bf->write_pos = 0;
}


