#include "cl_priv.h"
#include "json.h"

/*********************************************************************************/

typedef struct {
	int pos;
	int size;
	void **data;
} __stack_t;

__stack_t *stack_alloc()
{
	__stack_t *stack;

	stack = cl_calloc(sizeof(__stack_t), 1);
	stack->size = 100;
	stack->data = cl_calloc(sizeof(void *), stack->size);

	return stack;
}

void stack_free(__stack_t *stack)
{
	if (stack->pos != 0) {
		log_err(false, "stack_free, top=%u !!!!!!!!!\n", stack->pos);
	}
	
	cl_free(stack->data);
	cl_free(stack);
}

void stack_push(__stack_t *stack, void *data)
{
	if (stack->pos >= stack->size) {
		stack->size *= 2;
		stack->data = cl_realloc(stack->data, sizeof(void *)*stack->size);
	}
	
	stack->data[stack->pos++] = data;
}

void *stack_pop(__stack_t *stack)
{
	if (stack->pos == 0) {
		log_err(false, "stack pop failed: empty");
		return NULL;
	}

	stack->pos--;
	return stack->data[stack->pos];
}

/*********************************************************************************/

typedef enum { 
	JTT_UNKNOW,
	JTT_INTEGER,
	JTT_STRING,
	JTT_BOOLEAN, 
	JTT_ARRAY_B, 
	JTT_ARRAY_E, 
	JTT_OBJECT_B, /* OBJECT begin */
	JTT_OBJECT_E,  /* OBJECT end */
	JTT_VALUE, 
	JTT_VAL, 
	JTT_NEXT, 
	JTT_DONE
} jtokentype_e;


typedef struct {
    jtokentype_e type;
    char *name;
} jtoken_t;


void json_NewNode(jvalue_t ** root);
jtokentype_e json_ValToken(json_t *json, jtoken_t *readToken);
void json_NameObject(jvalue_t *root, const jtoken_t *readToken);
jtokentype_e json_ObjectbToken(json_t *json, jtoken_t *readToken);
jtokentype_e json_ObjecteToken(json_t *json, jtoken_t * readToken);
jtokentype_e json_StringToken(json_t *json, jtoken_t *readToken);
void json_InsertString(jvalue_t *root, const jtoken_t *readToken);
jtokentype_e json_ValueToken(json_t *json, jtoken_t *readToken);
jtokentype_e json_NextToken(json_t *json, jtoken_t *readToken);
jtokentype_e json_ArraybToken(json_t *json, jtoken_t * readToken);
jtokentype_e json_ArrayeToken(json_t *json, jtoken_t * readToken);
jtokentype_e json_DoneToken(json_t *json, jtoken_t * readToken);


jvalue_t *jvalue_alloc()
{
	jvalue_t *v;

	v = cl_calloc(sizeof(jvalue_t), 1);
	v->type = JT_U;
	v->name = cl_strdup("");

	return v;
}

jvalue_t *jvalue_set(jvalue_t *jv, char *newval)
{
	if (jv->type == JT_S) {
		STR_REPLACE(jv->value.str, newval);
	} else {
		log_err(false, "jvalue_set failed: type=%d\n", jv->type);
	}

	return jv;
}

// value is JT_A object node
jvalue_t *jvalue_lookup_by_str(jvalue_t *jv, char *findstr)
{
	jvalue_t *p;
	
	switch( jv->type ){
	case JT_O:
	    if( jv->value.list != NULL){ //object's value                        
	        for (p = jv->value.list; p; p = p->next){
	            if (strcmp(p->name, findstr) == 0){
	                return p;
	            }
	        }
	    }
	    break;
    default:
            break;
	}
	
	log_err(false, "here is not JT_A object,the type is: %d, findstr=%s\n", jv->type, findstr);
	
	return NULL;//not found
}

// value is JT_A array node
jvalue_t * jvalue_lookup_by_idx(jvalue_t *jv, int index)
{
	int i;
	jvalue_t *p;
	
    switch( jv->type )
    {
    case JT_A:
        i = 0;
        p = jv->value.list;
        while ( i < index ) {
            if( p->next ){
                p = p->next;
                i++;
            } else {
                break;
            }
        }
        
        if ( i == index ){
            return p;
        }
            break;
        default:
            break;
    }

	log_err(false, "the array size is to big: index=%d\n", index);

    return jv;
}

char *jvalue_to_string(jvalue_t *jv)
{
    switch( jv->type )
    {
    case JT_S:
        return jv->value.str;
    default:
        break;
    }
	
    return "";
}



void json_IgnoreSpace(json_t *json)
{
	char c;
	
	while ((c = json->json[json->current]) == ' ' || c == '\r' || c == '\n' || c == '\t')
		json->current++;
}

jtokentype_e json_ReadToken(json_t *json, jtoken_t *readToken)
{
    if ( json->current < json->size ){
        json_IgnoreSpace(json);

        switch( json->json[json->current] ){
        case '"':
            json_StringToken(json, readToken);
            return JTT_STRING;
        case '[':
            json_ArraybToken(json, readToken);
            return JTT_ARRAY_B;
        case ']':
            json_ArrayeToken(json, readToken);
            return JTT_ARRAY_E;
        case '{':
            json_ObjectbToken(json, readToken);
            return JTT_OBJECT_B;
        case '}':
            json_ObjecteToken(json, readToken);
            return JTT_OBJECT_E;
        case ':':
            json_ValueToken(json, readToken);
            return JTT_VALUE;
        case ',':
            json_NextToken(json, readToken);
            return JTT_NEXT;
        case ';':
            json_DoneToken(json, readToken);
            return JTT_DONE;
        default:
            json_ValToken(json, readToken);
            return JTT_VAL;
            break;
        }    
    }
	
	return JTT_UNKNOW;    
}

void json_InsertObject(jvalue_t **root)
{
	(*root)->type = JT_O;
	json_NewNode(root);
}

void json_EndObject(jvalue_t *root)
{
	root->next = NULL;
}

void json_InsertArray(jvalue_t **root)
{
	(*root)->type = JT_A;
	json_NewNode(root);
}

void json_EndArray(jvalue_t *root)
{
	root->next = NULL;
}       

void json_NewNode(jvalue_t ** root)
{
    (*root)->value.list = jvalue_alloc();
    *root = (*root)->value.list;
}

void json_NextNode(jvalue_t ** root)
{
    (*root)->next = jvalue_alloc();
    *root = (*root)->next;
}

#if 0
char *jit_map[] = { 
	"JTT_UNKNOW",
	"JTT_INTEGER",
	"JTT_STRING",
	"JTT_BOOLEAN", 
	"JTT_ARRAY_B", 
	"JTT_ARRAY_E", 
	"JTT_OBJECT_B", /* OBJECT begin */
	"JTT_OBJECT_E",  /* OBJECT end */
	"JTT_VALUE", 
	"JTT_VAL", 
	"JTT_NEXT", 
	"JTT_DONE"
};
#endif

bool json_Analyze(json_t *json)
{
    jtoken_t readToken;
    jvalue_t * currentNode = json->m_data;
    __stack_t *stack;
    jtokentype_e readType;

	memset(&readToken, 0, sizeof(readToken));
	stack = stack_alloc();
	
    while ( (readType = json_ReadToken(json, &readToken))!=JTT_UNKNOW ){//JTT_UNKNOW will out here
		//log_debug("readType=%s | %s, token=\"%s\"\n", jit_map[readType], jit_map[readToken.type], readToken.name);    
        switch( readType ){

        case JTT_OBJECT_B:
            stack_push(stack, currentNode);
            json_InsertObject(&currentNode);//new JT_A value node 
            break;
        case JTT_OBJECT_E:
            json_EndObject(currentNode);
            currentNode = stack_pop(stack);
            break;
        case JTT_STRING:
            json_InsertString(currentNode, &readToken);
            break;
        case JTT_ARRAY_B:
            stack_push(stack, currentNode);
            json_InsertArray(&currentNode); //new JT_A value node
            break;
        case JTT_ARRAY_E:
            json_EndArray(currentNode);
            currentNode = stack_pop(stack);
            break;
        case JTT_VAL:
			json_InsertString(currentNode, &readToken);
			//json_NameObject(currentNode, &readToken);
            break;
        case JTT_NEXT:
            json_NextNode(&currentNode);
            break;
        case JTT_DONE:
            currentNode->next = NULL;
            goto done;
        case JTT_VALUE:
            break;
        case JTT_INTEGER:
        case JTT_BOOLEAN:
            break;
        default:
                break;
        }

    }

done:
	stack_free(stack);

	SAFE_FREE(readToken.name);

    return true;
}

#if 1
jtokentype_e json_ValToken(json_t *json, jtoken_t *readToken)
{
	char *p, c;

    json_IgnoreSpace(json);

	p = strchr(json->json + json->current, ',');
	if (p == NULL) {
		for (p = json->json + json->current; *p; p++) {
			if (strchr("[]{}\"\r\n:;", *p) != NULL)
				break;
		}
	}
	
	c = *p;
	*p = '\0';
	STR_REPLACE(readToken->name, json->json + json->current);
	*p = c;
	readToken->type = JTT_VALUE;
	json->current = (int)(p - json->json);
	
	// del last space
	for (p = readToken->name + strlen(readToken->name) - 1; p >= readToken->name && *p == ' '; p--)
		;
	*(p + 1) = '\0';

    return JTT_STRING;
}
#else
jtokentype_e json_ValToken(json_t *json, jtoken_t *readToken)
{
	char *p;

	if (strncmp(json->json + json->current, "var", 3) != 0) {
        log_err(false, "var define error: %s\n", json->json + json->current);
    }
    json->current += 3;

    json_IgnoreSpace(json);

	p = strchr(json->json + json->current, '=');
	if (p == NULL) {
		log_err(false, "var define error, no =: %s\n", json->json + json->current);
		readToken->type = JTT_VAL;
		STR_REPLACE(readToken->name, "");
	} else {
		*p = '\0';
		STR_REPLACE(readToken->name, json->json + json->current);
		*p = '=';
		readToken->type = JTT_VAL;

		json->current = (int)(p - json->json + 1);
	}
	
	// del last space
	for (p = readToken->name + strlen(readToken->name) - 1; p >= readToken->name && *p == ' '; p--)
		;
	*(p + 1) = '\0';

    return JTT_VAL;
}
#endif

void json_NameObject(jvalue_t *root, const jtoken_t *readToken)
{
	STR_REPLACE(root->name, readToken->name);
    root->type = JT_O;
}

jtokentype_e json_ObjectbToken(json_t *json, jtoken_t *readToken)
{
    json->current++;
    readToken->type = JTT_OBJECT_B;
    return JTT_OBJECT_B;
}
jtokentype_e json_ObjecteToken(json_t *json, jtoken_t * readToken)
{
    readToken->type = JTT_OBJECT_E;
    json->current++;
    return JTT_OBJECT_E;
}
jtokentype_e json_StringToken(json_t *json, jtoken_t *readToken)
{
	bool has_esc = false;
	char *src, *dst;
	
    json->current++;

	for (src = dst = json->json + json->current; *src; src++) {
		if (*src == '\\') {
			has_esc = true;
		} else {
			if (*src == '"' && ! has_esc)
				break;
			
			has_esc = false;
			*dst++ = *src;
		}
	}

	*dst = '\0';

	if (src == '\0') {
		log_err(false, "get JTT_STRING token failed, str=%s\n", json->json + json->current);
		STR_REPLACE(readToken->name, json->json + json->current);
		return JTT_STRING;
	}

	STR_REPLACE(readToken->name, json->json + json->current);
	*dst = '"';

    if( readToken->type != JTT_VALUE ){ //judge the JTT_STRING is value or name
        readToken->type = JTT_STRING;
    }

	json->current = (int)(src - json->json + 1);
	
    return JTT_STRING;
}

void json_InsertString(jvalue_t *root, const jtoken_t *readToken)
{
     if( readToken->type != JTT_VALUE ){
         STR_REPLACE(root->name, readToken->name);
     }else{
         STR_REPLACE(root->value.str, readToken->name);
		 root->type = JT_S;
     }
}

jtokentype_e json_ValueToken(json_t *json, jtoken_t *readToken)
{
    json->current++;
    readToken->type = JTT_VALUE;
    return JTT_VALUE;
}

jtokentype_e json_NextToken(json_t *json, jtoken_t *readToken)
{
    readToken->type = JTT_NEXT;
    json->current++;
    return JTT_NEXT;
}

jtokentype_e json_ArraybToken(json_t *json, jtoken_t * readToken)
{
    json->current++;
    readToken->type = JTT_ARRAY_B;
    return JTT_ARRAY_B;
}

jtokentype_e json_ArrayeToken(json_t *json, jtoken_t * readToken)
{
    json->current++;
    readToken->type = JTT_ARRAY_E;
    return JTT_ARRAY_E;
}
jtokentype_e json_DoneToken(json_t *json, jtoken_t * readToken)
{
    readToken->type = JTT_DONE;
    json->current++;
    return JTT_DONE;
}

/*********************************************************************************************/

json_t *json_parse(const char *json_str, int size)
{
	json_t *json;

	json = cl_calloc(sizeof(json_t), 1);
	json->size = size;
	json->json = cl_malloc(size + 1);
	memcpy(json->json, json_str, size);
	json->json[size] = '\0';
	json->m_data = jvalue_alloc();

	json_Analyze(json);

	return json;
}

char *jt_str_map[] = {
	"unknow",
	"integer",
	"string",
	"boolean",
	"array",
	"object"
};

void jvalue_dump(jvalue_t *jv, int deep)
{
	char *space;
	jvalue_t *next;

	if (jv == NULL)
		return;
	
	space = cl_calloc(deep*4 + 1, 1);
	memset(space, ' ', deep*4);
	
	log_debug("%s@%p, name=%s, type=%s, value=%s, next=@%p, list=@%p\n",
		space, jv, jv->name, jt_str_map[jv->type], jvalue_to_string(jv),
		jv->next, jv->value.list);
	
	switch (jv->type) {
	case JT_A:
	case JT_O:
		for (next = jv->value.list; next != NULL; next = next->next)
			jvalue_dump(next, deep + 1);
		break;
    default:
            break;
	}

	cl_free(space);
}

void json_dump(json_t *json)
{
	jvalue_dump(json->m_data, 0);
}

// 深度优先遍历来释放
void jvalue_free(jvalue_t *jv)
{
	jvalue_t *node, *next;
	
	if (jv == NULL)
		return;

	if (jv->type == JT_A || jv->type == JT_O) {
		for (node = jv->value.list; node != NULL; node = next) {
			next = node->next;
			jvalue_free(node);
		}
	} else if (jv->type == JT_S) {
		SAFE_FREE(jv->value.str);
	}

	SAFE_FREE(jv->name);

	cl_free(jv);
}

void json_free(json_t *json)
{
	jvalue_free(json->m_data);
	SAFE_FREE(json->json);
	SAFE_FREE(json);
}


