
#ifndef	__DATA_EXCHG_H__
#define	__DATA_EXCHG_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct {
	// 被读了多少次
	int read_count;
	int buf_size;
	int data_size;
	u_int8_t data[0];
} data_buf_t;

typedef struct {
	cl_mutex_t lock;

	bool protect;
	int win;
	int read_pos;
	int write_pos;
	// 使用者具体用
	int priv;
	// 环形缓冲区
	data_buf_t **data;
} data_exchg_t;

extern data_exchg_t *de_alloc(int count, int size, bool protect);
extern void de_free(data_exchg_t *de);
extern bool de_is_empty(data_exchg_t *de);
extern bool de_is_full(data_exchg_t *de);
extern data_buf_t *de_get_write(data_exchg_t *de, int len);
extern bool de_move_write(data_exchg_t *de);
extern data_buf_t *de_get_read(data_exchg_t *de);
extern bool de_move_read(data_exchg_t *de);



#ifdef __cplusplus
}
#endif 

#endif


