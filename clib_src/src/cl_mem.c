#include "client_lib.h"
#include "cl_priv.h"
#include "cl_mem.h"
#include "cl_log.h"

#define	MEM_DEBUG_PRINT	0

#define	MAX_ALL_MEM	100
typedef struct {
	cl_mutex_t lock;
	struct stlc_list_head list;
} mem_rec_t;

typedef struct {
	struct stlc_list_head link;
	void *ptr;
	int size;
} mr_node_t;

static mem_rec_t all_mem[MAX_ALL_MEM];
static bool is_mem_init = false;

static void mem_init()
{
	int i;

	if (is_mem_init)
		return;
	is_mem_init = true;

	for (i = 0; i < MAX_ALL_MEM; i++) {
		cl_init_lock(&all_mem[i].lock);
		STLC_INIT_LIST_HEAD(&all_mem[i].list);
	}
}

int mem_check(const char *fn, int line, void *ptr);

static void mem_record(const char *fn, int line, void *old, void *ptr, int size)
{
	int i;
	mr_node_t *node;

	if (old != NULL) {
		mem_check(fn, line, old);
	}

	if (ptr == NULL) {
		log_err(false, "--signal... BAD record ptr=%p, %s line %d\n", ptr, fn, line);
		cl_assert(0);
		return;
	}

	node = calloc(sizeof(mr_node_t), 1);
	node->ptr = ptr;
	node->size = size;
	
	i = (((int)ptr)>>4)%MAX_ALL_MEM;
	
	cl_lock(&all_mem[i].lock);
	stlc_list_add(&node->link, &all_mem[i].list);
	cl_unlock(&all_mem[i].lock);
}

int mem_check(const char *fn, int line, void *ptr)
{
	int i, size = 0;
	bool find = false;
	mr_node_t *node;

	i = (((int)ptr)>>4)%MAX_ALL_MEM;
	
	cl_lock(&all_mem[i].lock);

	stlc_list_for_each_entry(mr_node_t,node,&all_mem[i].list,link) {
		if (node->ptr == ptr) {
			size = node->size;
			stlc_list_del(&node->link);
			free(node);
			find = true;
			break;
		}
	}

	cl_unlock(&all_mem[i].lock);

	if ( ! find ) {
		log_err(false, "--signal... BAD free ptr=%p, %s line %d\n", ptr, fn, line);
		cl_assert(0);
	}

	return size;
}

#define	MAGIC_LEN	16
#define	MAGIC_TOTAL	(MAGIC_LEN * 2)
#define	MAGIC 0xAC7f0D33

void *magic_fill(void *ptr, int size)
{
	u_int32_t *end;
	u_int32_t *begin = (u_int32_t *)ptr;

	*begin++ = MAGIC;
	*begin++ = MAGIC;
	*begin++ = MAGIC;
	*begin++ = MAGIC;

	end = (u_int32_t *)((u_int8_t *)begin + size);
	*end++ = MAGIC;
	*end++ = MAGIC;
	*end++ = MAGIC;
	*end++ = MAGIC;

	return begin;
}

void *magic_check(const char *fn, int line, void *ptr, int size)
{
	u_int32_t *end;
	u_int32_t *begin = (u_int32_t *)ptr;

	end = (u_int32_t *)((u_int8_t *)begin + size);

	begin -= 4;
	if (begin[0] != MAGIC || begin[1] != MAGIC || begin[2] != MAGIC || begin[3] != MAGIC) {
		log_err(false, "--signal... BAD head magic=%p, %s line %d\n", ptr, fn, line);
		cl_assert(0);
	}
	if (end[0] != MAGIC || end[1] != MAGIC || end[2] != MAGIC || end[3] != MAGIC) {
		log_err(false, "--signal... BAD tail magic=%p, %s line %d\n", ptr, fn, line);
		cl_assert(0);
	}

	return begin;
}

void extern_mem_check(const char *fn, int line)
{
	int i;
	mr_node_t *node;

	for (i = 0; i < MAX_ALL_MEM; i++) {
		cl_lock(&all_mem[i].lock);

		stlc_list_for_each_entry(mr_node_t,node,&all_mem[i].list,link) {
			magic_check(fn, line, node->ptr, node->size);
		}

		cl_unlock(&all_mem[i].lock);
	}
}

void *cl_malloc_debug(const char *fn, int line, size_t size)
{
	void *ret;

	mem_init();
	ret = malloc(size + MAGIC_TOTAL);
	ret = magic_fill(ret, (int)size);
	mem_record(fn, line, NULL, ret, (int)size);

#if MEM_DEBUG_PRINT
	log_debug("%s line %d malloc size=%u, ptr=%p\n", fn, line, size, ret);
#endif	

	return ret;
}

void *cl_calloc_debug(const char *fn, int line, size_t size, int n)
{
	void *ret;

	mem_init();
	ret = calloc(size*n+MAGIC_TOTAL, 1);
	ret = magic_fill(ret, (int)size*n);
	mem_record(fn, line, NULL, ret, (int)size*n);

#if MEM_DEBUG_PRINT
	log_debug("%s line %d calloc size=%u, count=%d, ptr=%p\n", fn, line, size, n, ret);
#endif

	return ret;
}

void *cl_realloc_debug(const char *fn, int line, void *ptr, size_t size)
{
	int old_size;
	void *ret;

	mem_init();
	
	ret = cl_malloc_debug(fn, line, size);
	
	old_size = mem_check(fn, line, ptr);
	cl_assert((int)size >= old_size);
	memcpy(ret, ptr, old_size);
	ptr = magic_check(fn, line, ptr, old_size);
	free(ptr);

#if MEM_DEBUG_PRINT
	log_debug("%s line %d realloc size=%u, ptr=%p -> %p\n", fn, line, size, ptr, ret);
#endif

	return ret;
}

char *cl_strdup_debug(const char *fn, int line, const char *str)
{
	char *ret;
	int size;

	mem_init();

	size = (int)strlen(str) + 1;
	ret = cl_malloc_debug(fn, line, size);
	strcpy(ret, str);
	
#if MEM_DEBUG_PRINT
	log_debug("%s line %d strdup size=%u, ptr=%p\n", fn, line, strlen(str), ret);
#endif

	return ret;
}

void cl_free_debug(const char *fn, int line, void *ptr)
{
	int size;

#if MEM_DEBUG_PRINT
	log_debug("%s line %d free ptr=%p\n", fn, line, ptr);
#endif	
	mem_init();
	size = mem_check(fn, line, ptr);
	ptr = magic_check(fn, line, ptr, size);
	free(ptr);
}

void cl_dump_data_debug(const char *fn, int line, void *data, int size)
{
	int i, pos;
	u_int8_t *dp = (u_int8_t *)data;
	char buf[256];
	
	log_debug("================== dump data file=%s, line=%d, data=@%p, size=%u ====================\n",
		fn, line, data, size);

	pos = 0;
	for (i = 0; i < size; i++) {
		pos += sprintf(&buf[pos], " %02x", dp[i]);
		if ((i + 1)%16 == 0) {
			log_debug("<%03d>: %s\n", i-15, buf);
			pos = 0;
		}
	}
	log_debug("%s\n", buf);
}

