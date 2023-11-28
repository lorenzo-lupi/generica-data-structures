#ifndef GDS_LINKED_LIST
#define GDS_LINKED_LIST

#ifndef HACK_PTHREADS
#define HACK_PTHREADS 4
#endif

#include <pthread.h>
#include "../utils/errors.h"
#include "../utils/types.h"

typedef struct llist_node_struct{
	void* value;	
	struct llist_node_struct* next;
} llist_node;


typedef struct llist_struct{
	llist_node* head;
	llist_node* tail;
	int length;
} llist;

llist* alloc_llist();

void add_elem_to_llist(llist* l, void* value);

void* remove_elem_from_llist(llist* l, void* value, boolean (*eq) (void*, void*));

boolean contains(llist* l, void* value, boolean (*eq) (void*, void*));

void foreach_llist(llist* list, void (*consumer) (void*));

void foreach_parallel_llist(llist* list, void (*consumer) (void*));

//linear time concat
llist* l_concat_llists(llist* l1, llist* l2);

//constant time concat destroys both l1 and l2
llist* concat_llists(llist* l1, llist* l2);


void destroy_llist(llist* l, void (*value_destroyer) (void*));
#endif
