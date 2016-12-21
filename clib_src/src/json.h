#ifndef	 __JSON_H__
#define	__JSON_H__

#include "client_lib.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef enum {
	JT_U, /* unknow */
	JT_I, /* integer, not use now */
	JT_S, /* string */
	JT_B, /* JTT_BOOLEAN, not use now */
	JT_A, /* array */
	JT_O /* object */
} jtype_e;



struct jvalue_s;
typedef struct jvalue_s jvalue_t;


typedef union {
    char * str;
    jvalue_t * list;
} jvalue_u;

typedef struct jvalue_s {
	// JT_XXX
    jtype_e type;
    char *name;
    jvalue_u  value;
    jvalue_t * next;
} jvalue_t;


typedef struct {
	jvalue_t *m_data;
	char *json;
	int size;
	int current;
} json_t;

extern jvalue_t *jvalue_lookup_by_str(jvalue_t *jv, char *findstr);

extern json_t *json_parse(const char *json_str, int size);
extern void json_free(json_t *json);
extern void json_dump(json_t *json);

#ifdef __cplusplus
}
#endif 

#endif

