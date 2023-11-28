#ifndef GDS_HASH_T
#define GDS_HASH_T

#ifndef HACK_PTHREADS
#define HACK_PTHREADS 4
#endif

#include <pthread.h>
#include "../linked_list/linked_list.h"
#include "../utils/errors.h"
#include "../utils/types.h"

#define INIT_SIZE 512
#define REALLOC_SIZE 1024
#define MAX_LOAD 0.90 


typedef struct hash_t_entry_struct{
	void* key;
	void* value;
} hash_t_entry;


typedef struct hash_t_struct{

	llist** table;
	int size;
	int length;
	int (*hash_function) (void*);

} hash_t;

hash_t* alloc_hash_t(int (*hash_function) (void*));

boolean add_elem_to_hash_t(hash_t* h_t, boolean (*equals_key) (void*, void*), void* key, void* value);

void* get_from_hash_t(hash_t* h_t, boolean (*equals_key) (void*, void*), void* key);

int update_value_hasht(hash_t* h_t, boolean (*equals_key) (void*, void*), void* key, void* new_value, boolean free_old_val);

void update_all_values_hasht(hash_t* h_t, void* function(void*), boolean free_old_val);

void update_all_values_hasht_parallel(hash_t* h_t, void* function(void*), boolean free_old_val);

void destroy_hash_t(hash_t* h_t, void (*destroy_key) (void*), void (*destroy_value) (void*));


#endif
