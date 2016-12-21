
#ifndef	__BUFFER_H__
#define	__BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define	DFL_VIDEO_TCP_BUFF_SIZE	(64*1024)
#define	MAX_VIDEO_TCP_BUFF_SIZE	(100*1024*1024)


typedef struct {
	char *buf;
	// 缓冲区大小
	int buf_size;
	// 报文大小
	int pkt_size;
	int read_pos;
	int write_pos;
} buff_t;


extern buff_t *bf_alloc(int size);
extern void bf_free(buff_t *bf);

extern bool bf_expand(buff_t *bf, int size);
extern bool bf_init(buff_t *bf, int size);
extern int bf_get_write_free(buff_t *bf);
extern char *bf_get_write_pos(buff_t *bf);
extern int bf_get_read_len(buff_t *bf);
extern char *bf_get_read_pos(buff_t *bf);
extern void bf_check_read_write_pos(buff_t *bf);	
extern void bf_reset(buff_t *bf);


#ifdef __cplusplus
}
#endif 

#endif




