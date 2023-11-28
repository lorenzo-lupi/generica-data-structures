#include "hash_t.h"

struct ua_parallel_arg{
	hash_t* h_t;
	int index;
	int len;
	void* (*function) (void*);
	boolean free_old_val;
};

static inline hash_t_entry* alloc_hash_t_entry(void* key, void* value);
static inline void resize_hash_t(hash_t* h_t);
static inline boolean hash_t_entries_list_contains(llist* entries_list,  boolean (*equals_key) (void*, void*), void* key);
static inline void add_entry(hash_t* h_t, hash_t_entry* entry);

static void* update_all_thread_function(void* arg);
// PUBLIC FUNCTIONs

hash_t* alloc_hash_t(int (*hash_function) (void*)){
	hash_t* n_h_t = malloc(sizeof(hash_t));
	if(n_h_t == NULL)
		terminate_w_error("in alloc_hash_t","can't allocate mem");

	n_h_t -> table = malloc(sizeof(llist*) * INIT_SIZE);
	if(n_h_t -> table == NULL)
		terminate_w_error("in alloc_hash_t", "can't allocate mem");

	for(int i = 0; i < INIT_SIZE; i++) 
		n_h_t -> table[i] = NULL;
	
	n_h_t -> size = INIT_SIZE;
	n_h_t -> length = 0;
	n_h_t -> hash_function = hash_function;
	
	return n_h_t;
}


boolean add_elem_to_hash_t(hash_t* h_t, 
		boolean (*equals_key) (void*, void*), 
		void* key, void* value){

	if(h_t == NULL)
		terminate_w_error("in add_elem_to_hash_t", "the hash_t can't be null");
	if(key == NULL)
		terminate_w_error("in add_elem_to_hash_t", "the key can't be null");

	if((h_t -> length + 1) / (h_t -> size) >= MAX_LOAD)
		resize_hash_t(h_t);

	int hash_val = ((*(h_t -> hash_function)) (key)) % (h_t -> size);

	if(h_t -> table[hash_val] == NULL)
		h_t -> table[hash_val] = alloc_llist();
	else
		if(hash_t_entries_list_contains(h_t -> table[hash_val], equals_key, key)) 
			return FALSE;

	add_elem_to_llist(h_t -> table[hash_val], alloc_hash_t_entry(key, value));
	h_t -> length ++;

	return TRUE;
}

void* get_from_hash_t(hash_t* h_t, boolean (*equals_key) (void*, void*), void* key){
	if(h_t == NULL)
		terminate_w_error("in get_from_hash_t ", "the hash_t can't be null");
	if(key == NULL)
		terminate_w_error("in get_from_hash_t ", "the key can't be null");

	int hash_val = ((*(h_t -> hash_function)) (key)) % (h_t -> size);

	if(h_t -> table[hash_val] == NULL)
		return NULL;

	llist_node* ptr = (h_t -> table[hash_val]) -> head;
	while(ptr != NULL){
		if(equals_key(((hash_t_entry*)(ptr -> value)) -> key, key))
			return ((hash_t_entry*)(ptr -> value)) -> value;
		ptr = ptr -> next;
	}
	return NULL;
}


int update_value_hasht(hash_t* h_t, 
		boolean (*equals_key) (void*, void*), 
		void* key, 
		void* new_value,
		boolean free_old_val){
	if(h_t == NULL)
		terminate_w_error("in get_from_hash_t ", "the hash_t can't be null");
	if(key == NULL)
		terminate_w_error("in get_from_hash_t ", "the key can't be null");

	int hash_val = ((*(h_t -> hash_function)) (key)) % (h_t -> size);

	if(h_t -> table[hash_val] == NULL)
		return FALSE;

	llist_node* ptr = (h_t -> table[hash_val]) -> head;
	while(ptr != NULL){
		if(equals_key(((hash_t_entry*)(ptr -> value)) -> key, key)){
			free(((hash_t_entry*)(ptr -> value)) -> value);
			if(free_old_val) ((hash_t_entry*)(ptr -> value)) -> value = new_value;
			return TRUE;
		}
		ptr = ptr -> next;
	}
	return FALSE;
}

void update_all_values_hasht(hash_t* h_t, void* function(void*), boolean free_old_val){
	if(h_t == NULL)
		terminate_w_error("in update_all_values_hasht ", "the hash_t can't be null");
	for(int i = 0; i < h_t -> size; i++)
		if(h_t -> table[i] != NULL){
			llist_node* ptr = h_t -> table[i] -> head;
			while(ptr != NULL){

				void* old_val = ((hash_t_entry*)(ptr -> value)) -> value;
				((hash_t_entry*)(ptr -> value)) -> value = function(old_val);

				if(free_old_val) free(old_val);
				ptr = ptr -> next;
			}
		}
}


static void* update_all_thread_function(void* arg){

        struct ua_parallel_arg* conv_arg = (struct ua_parallel_arg*) arg;
	int index   = conv_arg -> index; 
	int len     = conv_arg -> len;
	hash_t* h_t = conv_arg -> h_t;
	void* (*function) (void *) = conv_arg -> function;
	boolean free_old_val = conv_arg -> free_old_val;
	
	for(int i = index; i < index+len; i++)
		if(h_t -> table[i] != NULL){
			llist_node* ptr = h_t -> table[i] -> head;
			while(ptr != NULL){
				void* old_val = ((hash_t_entry*)(ptr -> value)) -> value;
				((hash_t_entry*)(ptr -> value)) -> value = (*function) (old_val);
				if(free_old_val) free(old_val);
				ptr = ptr -> next;
			}
		}
	return NULL;
}

void update_all_values_hasht_parallel(hash_t* h_t, void* function(void*), boolean free_old_val){
	if(h_t == NULL)
		terminate_w_error("in update_all_values_hasht_parallel", "null arguments");

	void* thread_res;
	struct ua_parallel_arg args[HACK_PTHREADS];
	int len = h_t -> size / HACK_PTHREADS;

	for(int i = 0; i < HACK_PTHREADS; i++){
		args[i].h_t = h_t;
		args[i].len = len;
		args[i].index = len * i;
		args[i].function = function;
		args[i].free_old_val = free_old_val;
	}

	pthread_t threads_[HACK_PTHREADS];
	int status[HACK_PTHREADS];

	
	for(int i = 0; i < HACK_PTHREADS; i++)
		status[i] =  pthread_create(&threads_[i], NULL, update_all_thread_function, &args[i]);
	
	
	for(int i = 0; i < HACK_PTHREADS; i++)
		if(status[i])
			terminate_w_error("In foreach_parallel:", "can't create pthread");


	for(int i = 0; i < HACK_PTHREADS; i++)
		status[i] =  pthread_join(threads_[i], &thread_res);
	
	for(int i = 0; i < HACK_PTHREADS; i++)
		if(status[i])
			terminate_w_error("In foreach_parallel:", "error in joining threads");
	
}

void destroy_hash_t(hash_t* h_t, void (*destroy_value) (void*), void (*destroy_key) (void*)){
	if(h_t == NULL) return;
	for(int i = 0; i < h_t -> size; i++){
		if(h_t -> table[i] != NULL){
			llist_node* ptr = h_t -> table[i] -> head;
			llist_node* buff;
			while(ptr != NULL){
				buff = ptr -> next;
				hash_t_entry* entry = ptr -> value; 
				(*destroy_value) (entry -> value);
				(*destroy_key) (entry -> key);
				free(ptr);
				free(entry);
				ptr = buff;
			}
			free(h_t -> table[i]);
		}
	}
	free(h_t -> table);
	free(h_t);
}

// PRIVATE FUNCTIONs
//
//i'd love if map functions could easily exist in C
static inline boolean hash_t_entries_list_contains(llist* entries_list,  
		boolean (*equals_key) (void*, void*), 
		void* key) {

	llist_node* ptr = entries_list -> head;
	while(ptr != NULL){
		if(equals_key(((hash_t_entry*)(ptr -> value)) -> key, key)) return TRUE;
		ptr = ptr -> next;
	}
	return FALSE;
}


static inline hash_t_entry* alloc_hash_t_entry(void* key, void* value){
	hash_t_entry* n_e = malloc(sizeof(hash_t_entry)); 
	n_e -> key = key;
	n_e -> value = value;
	return n_e;
}

static inline llist* concat_all_sub_lists(hash_t* h_t){
	llist* all_lists = NULL;
	for(int i = 0; i < h_t -> size; i++)
		if(h_t -> table[i] != NULL){
			//i'm also freeing h_t -> table[i]
			all_lists = concat_llists(h_t -> table[i], all_lists);
			h_t -> table[i] = NULL;
		}
	return all_lists;
}

//i'm assuming entry already was in hash_t!!
static inline void add_entry(hash_t* h_t, hash_t_entry* entry){
	int hash_val = ((*(h_t -> hash_function)) (entry -> key)) % (h_t -> size);
	if(h_t -> table[hash_val] == NULL)
		h_t -> table[hash_val] = alloc_llist();
	add_elem_to_llist(h_t -> table[hash_val], entry);
}

static inline void resize_hash_t(hash_t* h_t){
	llist* all_lists = concat_all_sub_lists(h_t);
	h_t -> size += REALLOC_SIZE;
	h_t -> table = realloc(h_t -> table, sizeof(llist*) * (h_t -> size));

	for(int i = (h_t -> size - REALLOC_SIZE); i < h_t -> size; i++){
		h_t -> table[i] = NULL;
	}

	llist_node* ptr = all_lists -> head;
	llist_node* buff = NULL;
	for(int i = 0; i < all_lists -> length; i++){
		buff = ptr -> next;
		add_entry(h_t, ptr -> value);
		free(ptr);
		ptr = buff;
	}

	free(all_lists); 
}
