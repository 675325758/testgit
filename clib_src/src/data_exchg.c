#include "cl_priv.h"
#include "data_exchg.h"

/*
	count最少3个才玩得转。
	protect: 保护是否保留最后一个可读的
*/
data_exchg_t *de_alloc(int count, int size, bool protect)
{
	int i;
	data_buf_t *db;
	data_exchg_t *de;

	de = cl_calloc(sizeof(data_exchg_t), 1);
	cl_init_lock(&de->lock);
	de->protect = protect;

	de->win = count;
	de->data = cl_calloc(sizeof(void *)*count, 1);
	for (i = 0; i < count; i++) {
		db = cl_malloc(size + sizeof(data_buf_t));
		db->read_count = 0;
		db->buf_size = size;
		db->data_size = 0;
		de->data[i] = db;
	}

	return de;
}

void de_free(data_exchg_t *de)
{
	int i;

	if (de == NULL)
		return;
	
	for (i = 0; i < de->win; i++) {
		cl_free(de->data[i]);
	}
	cl_free(de->data);
	cl_free(de);
}

bool de_is_empty(data_exchg_t *de)
{
	return (de->read_pos == de->write_pos);
}

bool de_is_full(data_exchg_t *de)
{
	return (((de->write_pos + 1)%de->win) == de->read_pos);
}

/*
	获取一个拿来写，不移动指针，de_put的时候再移动
*/
data_buf_t *de_get_write(data_exchg_t *de, int len)
{
	data_buf_t *db;
	
	cl_lock(&de->lock);

	db = de->data[de->write_pos];
	if (len > db->buf_size) {
		db = cl_malloc(len + sizeof(data_buf_t));
		db->buf_size = len;
		db->data_size = 0;

		cl_free(de->data[de->write_pos]);
		de->data[de->write_pos] = db;	
	}
	
	cl_unlock(&de->lock);

	return db;
}

/*
	注意: 同一个环形缓冲区，只能有一个线程写
	如果没满，移动指针
	如果满了，不移动指针，即丢弃了当前帧
*/
bool de_move_write(data_exchg_t *de)
{
	bool ret = false;
	
	cl_lock(&de->lock);

	if ( ! de_is_full(de) ) {
		de->write_pos = (de->write_pos + 1)%de->win;
		ret = true;
	}
	
	cl_unlock(&de->lock);

	return ret;
}

data_buf_t *de_get_read(data_exchg_t *de)
{
	data_buf_t *db = NULL;

	if (de == NULL)
		return NULL;
	
	cl_lock(&de->lock);

	if ( ! de_is_empty(de) ) {
		db = de->data[de->read_pos];
		if (de->protect) {
			if (db->read_count > 0) {
				if (((de->read_pos + 1) % de->win) != de->write_pos) {
					de->read_pos = (de->read_pos + 1) % de->win;
					db->read_count = 1;
				} else {
					db->read_count++;
				}
			} else {
				db->read_count = 1;
			}
		} else {

			// 别移动，否则马上被写覆盖了。使用完数据，通过调用de_move_read来移动
			//de->read_pos = (de->read_pos + 1) % de->win;
		}
	}
	
	cl_unlock(&de->lock);

	return db;
}

/*
	移动读指针
*/
bool de_move_read(data_exchg_t *de)
{
	bool ret = false;
	
	cl_lock(&de->lock);

	de->read_pos = (de->read_pos + 1) % de->win;
	
	cl_unlock(&de->lock);

	return ret;
}

