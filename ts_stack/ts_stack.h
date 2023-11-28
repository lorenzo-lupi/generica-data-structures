#ifndef GDS_TS_STACK
#define GDS_TS_STACK

#include <pthread.h>
#include "../utils/errors.h"
#include "../utils/types.h"

typedef struct ts_stack_node_struct{
	void* value;
	struct ts_stack_node_struct* next;
} ts_stack_node;

typedef struct ts_stack_struct{
	ts_stack_node*    head;
	pthread_mutex_t  mutex;
}ts_stack;

ts_stack* alloc_ts_stack();

void* ts_pop(ts_stack* ts_s);

void ts_push(ts_stack* ts_s, void* val);

void destroy_ts_stack(ts_stack* ts_s, void (*destroy_value) (void*));

#endif
