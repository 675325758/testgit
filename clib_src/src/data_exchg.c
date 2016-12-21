#include "cl_priv.h"
#include "data_exchg.h"

/*
	count����3�������ת��
	protect: �����Ƿ������һ���ɶ���
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
	��ȡһ������д�����ƶ�ָ�룬de_put��ʱ�����ƶ�
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
	ע��: ͬһ�����λ�������ֻ����һ���߳�д
	���û�����ƶ�ָ��
	������ˣ����ƶ�ָ�룬�������˵�ǰ֡
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

			// ���ƶ����������ϱ�д�����ˡ�ʹ�������ݣ�ͨ������de_move_read���ƶ�
			//de->read_pos = (de->read_pos + 1) % de->win;
		}
	}
	
	cl_unlock(&de->lock);

	return db;
}

/*
	�ƶ���ָ��
*/
bool de_move_read(data_exchg_t *de)
{
	bool ret = false;
	
	cl_lock(&de->lock);

	de->read_pos = (de->read_pos + 1) % de->win;
	
	cl_unlock(&de->lock);

	return ret;
}

